/*
 *  midas_wm1811.c
 *
 *  Copyright (c) 2011 Samsung Electronics Co. Ltd
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 */
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/input.h>

#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/jack.h>

#include <mach/regs-clock.h>

#include <linux/mfd/wm8994/registers.h>
#include <linux/mfd/wm8994/pdata.h>

#include "i2s.h"
#include "s3c-i2s-v2.h"
#include "../codecs/wm8994.h"

/*
#define MANAGE_MCLK1
*/

/* SMDK has a 16.934MHZ crystal attached to WM8994 */
#define SMDK_WM8994_OSC_FREQ	16934400
#define WM8994_DAI_AIF1		0
#define WM8994_DAI_AIF2		1
#define WM8994_DAI_AIF3		2

#define WM1811_JACKDET_MODE_NONE  0x0000
#define WM1811_JACKDET_MODE_JACK  0x0100
#define WM1811_JACKDET_MODE_MIC   0x0080
#define WM1811_JACKDET_MODE_AUDIO 0x0180

#define WM1811_JACKDET_BTN0	0x04
#define WM1811_JACKDET_BTN1	0x10
#define WM1811_JACKDET_BTN2	0x08


static const struct wm8958_micd_rate midas_det_rates[] = {
	{ 32768,       true,  0, 1 },
	{ 32768,       false, 0, 1 },
	{ 44100 * 256, true,  8, 8 },
	{ 44100 * 256, false, 8, 8 },
};

/* To support PBA function test */
static struct class *jack_class;
static struct device *jack_dev;

static bool midas_fll1_active;
static enum snd_soc_bias_level midas_card_bias_level;
struct mutex midas_lock;

static void midas_set_mclk(int on)
{
	u32 val;
	u32 __iomem *xusbxti_sys_pwr;
	u32 __iomem *pmu_debug;

	static int ipwron = -1;

	if (ipwron == on)
		return;

	ipwron = on;

	xusbxti_sys_pwr = ioremap(0x10021280, 4);
	pmu_debug = ioremap(0x10020A00, 4);

	if (on) {
		val = readl(xusbxti_sys_pwr);
		val |= 0x0001;			/* SYS_PWR_CFG is enabled */
		writel(val, xusbxti_sys_pwr);

		val = readl(pmu_debug);
		val &= ~(0x000F << 8);
		val |= 0x0009 << 8;		/* Selected XUSBXTI */
		val &= ~(0x0001);		/* CLKOUT is enabled */
		writel(val, pmu_debug);
	} else {
		val = readl(xusbxti_sys_pwr);
		val &= ~(0x0001);		/* SYS_PWR_CFG is disabled */
		writel(val, xusbxti_sys_pwr);

		val = readl(pmu_debug);
		val |= 0x0001;			/* CLKOUT is disabled */
		writel(val, pmu_debug);
	}

	iounmap(xusbxti_sys_pwr);
	iounmap(pmu_debug);

	mdelay(10);
}

static void midas_gpio_init(void)
{
	int err;

	/* Main Microphone BIAS */
	err = gpio_request(GPIO_MIC_BIAS_EN, "GPF1");
	if (err) {
		pr_err(KERN_ERR "MIC_BIAS_EN GPIO set error!\n");
		return;
	}
	gpio_direction_output(GPIO_MIC_BIAS_EN, 1);
	gpio_set_value(GPIO_MIC_BIAS_EN, 0);
	gpio_free(GPIO_MIC_BIAS_EN);

	/* Sub Microphone BIAS */
	err = gpio_request(GPIO_SUB_MIC_BIAS_EN, "GPF2");
	if (err) {
		pr_err(KERN_ERR "SUB_MIC_BIAS_EN GPIO set error!\n");
		return;
	}
	gpio_direction_output(GPIO_SUB_MIC_BIAS_EN, 1);
	gpio_set_value(GPIO_SUB_MIC_BIAS_EN, 0);
	gpio_free(GPIO_SUB_MIC_BIAS_EN);
}

static int midas_ext_micbias(struct snd_soc_dapm_widget *w,
			     struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;

	dev_dbg(codec->dev, "%s event is %02X", w->name, event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		gpio_set_value(GPIO_MIC_BIAS_EN, 1);
		break;
	case SND_SOC_DAPM_POST_PMD:
		gpio_set_value(GPIO_MIC_BIAS_EN, 0);
		break;
	}

#if 0
	gpio_set_value(GPIO_MIC_BIAS_EN, SND_SOC_DAPM_EVENT_ON(event));
#endif

	return 0;
}

static int midas_ext_submicbias(struct snd_soc_dapm_widget *w,
				struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;

	dev_dbg(codec->dev, "%s event is %02X", w->name, event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		gpio_set_value(GPIO_SUB_MIC_BIAS_EN, 1);
		break;
	case SND_SOC_DAPM_POST_PMD:
		gpio_set_value(GPIO_SUB_MIC_BIAS_EN, 0);
		break;
	}

#if 0
	gpio_set_value(GPIO_SUB_MIC_BIAS_EN, SND_SOC_DAPM_EVENT_ON(event));
#endif

	return 0;
}

static int midas_ext_spkmode(struct snd_soc_dapm_widget *w,
			     struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = w->codec;
	int ret = 0;

	ret = snd_soc_update_bits(codec, WM8994_SPKOUT_MIXERS,
				  WM8994_SPKMIXR_TO_SPKOUTL_MASK,
				  WM8994_SPKMIXR_TO_SPKOUTL);

	return ret;
}

static void midas_micd_set_rate(struct snd_soc_codec *codec)
{
	struct wm8994_priv *wm8994 = snd_soc_codec_get_drvdata(codec);
	int best, i, sysclk, val;
	bool idle;
	const struct wm8958_micd_rate *rates;
	int num_rates;

	idle = !wm8994->jack_mic;

	sysclk = snd_soc_read(codec, WM8994_CLOCKING_1);
	if (sysclk & WM8994_SYSCLK_SRC)
		sysclk = wm8994->aifclk[1];
	else
		sysclk = wm8994->aifclk[0];

	if (wm8994->jackdet) {
		rates = midas_det_rates;
		num_rates = ARRAY_SIZE(midas_det_rates);
	}

	best = 0;
	for (i = 0; i < num_rates; i++) {
		if (rates[i].idle != idle)
			continue;
		if (abs(rates[i].sysclk - sysclk) <
		    abs(rates[best].sysclk - sysclk))
			best = i;
		else if (rates[best].idle != idle)
			best = i;
	}

	val = rates[best].start << WM8958_MICD_BIAS_STARTTIME_SHIFT
		| rates[best].rate << WM8958_MICD_RATE_SHIFT;

	snd_soc_update_bits(codec, WM8958_MIC_DETECT_1,
			    WM8958_MICD_BIAS_STARTTIME_MASK |
			    WM8958_MICD_RATE_MASK, val);
}

static void midas_start_fll1(struct snd_soc_dai *aif1_dai)
{
	int ret;
	if (midas_fll1_active)
		return;

	dev_info(aif1_dai->dev, "Moving to audio clocking settings\n");

	/* Switch AIF1 to MCLK2 while we bring stuff up */
	ret = snd_soc_dai_set_sysclk(aif1_dai,
				     WM8994_SYSCLK_MCLK2,
				     32768,
				     SND_SOC_CLOCK_IN);
	if (ret < 0)
		dev_err(aif1_dai->dev, "Unable to switch to MCLK2: %d\n", ret);

	/* Start the 24MHz clock to provide a high frequency reference to
	 * provide a high frequency reference for the FLL, giving improved
	 * performance.
	 */
	midas_set_mclk(3);	/* forced enable MCLK */

	/* Switch the FLL */
	ret = snd_soc_dai_set_pll(aif1_dai, WM8994_FLL1, WM8994_FLL_SRC_MCLK1,
					 24000000, 44100 * 256);
	if (ret < 0)
		dev_err(aif1_dai->dev, "Unable to start FLL1: %d\n", ret);

#ifdef MANAGE_MCLK1
	/* Now the FLL is running we can stop the reference clock, the
	 * FLL will maintain frequency with no reference so this saves
	 * power from the reference clock.
	 */
	midas_set_mclk(0);
#endif

	/* Then switch AIF1CLK to it */
	ret = snd_soc_dai_set_sysclk(aif1_dai,
					WM8994_SYSCLK_FLL1,
					44100 * 256,
					SND_SOC_CLOCK_IN);
	if (ret < 0)
		dev_err(aif1_dai->dev, "Unable to switch to FLL1: %d\n", ret);

	midas_micd_set_rate(aif1_dai->codec);

	midas_fll1_active = true;
}

static void midas_micdet(u16 status, void *data)
{
	struct snd_soc_codec *codec = data;
	struct wm8994_priv *wm8994 = snd_soc_codec_get_drvdata(codec);
	int report;

	/* Either nothing present or just starting detection */
	if (!(status & WM8958_MICD_STS)) {
		if (!wm8994->jackdet) {
			/* If nothing present then clear our statuses */
			dev_dbg(codec->dev, "Detected open circuit\n");
			wm8994->jack_mic = false;
			wm8994->mic_detecting = true;

			midas_micd_set_rate(codec);

			snd_soc_jack_report(wm8994->micdet[0].jack, 0,
					    wm8994->btn_mask |
					     SND_JACK_HEADSET);
		}
		return;
	}

	/* If the measurement is showing a high impedence we've got a
	 * microphone.
	 */
	if (wm8994->mic_detecting && (status & 0x400)) {
		dev_info(codec->dev, "Detected microphone\n");

		wm8994->mic_detecting = false;
		wm8994->jack_mic = true;

		midas_micd_set_rate(codec);

		snd_soc_jack_report(wm8994->micdet[0].jack, SND_JACK_HEADSET,
				    SND_JACK_HEADSET);
	}

	if (wm8994->mic_detecting && status & 0x4) {
		dev_info(codec->dev, "Detected headphone\n");
		wm8994->mic_detecting = false;

		midas_micd_set_rate(codec);

		snd_soc_jack_report(wm8994->micdet[0].jack, SND_JACK_HEADPHONE,
				    SND_JACK_HEADSET);

		/* If we have jackdet that will detect removal */
		if (wm8994->jackdet) {
			snd_soc_update_bits(codec, WM8958_MIC_DETECT_1,
					    WM8958_MICD_ENA, 0);

			if (wm8994->active_refcount) {
				snd_soc_update_bits(codec, WM8994_ANTIPOP_2,
					WM1811_JACKDET_MODE_MASK,
					WM1811_JACKDET_MODE_AUDIO);
			} else {
				snd_soc_update_bits(codec, WM8994_ANTIPOP_2,
						WM1811_JACKDET_MODE_MASK,
						WM1811_JACKDET_MODE_JACK);
			}
		}
	}

	/* Report short circuit as a button */
	if (wm8994->jack_mic) {
		report = 0;
		if (status & WM1811_JACKDET_BTN0)
			report |= SND_JACK_BTN_0;

		if (status & WM1811_JACKDET_BTN1)
			report |= SND_JACK_BTN_1;

		if (status & WM1811_JACKDET_BTN2)
			report |= SND_JACK_BTN_2;

		dev_dbg(codec->dev, "Detected Button: %08x (%08X)\n",
			report, status);

		snd_soc_jack_report(wm8994->micdet[0].jack, report,
				    wm8994->btn_mask);
	}
}

#ifdef CONFIG_SND_SAMSUNG_I2S_MASTER
static int set_epll_rate(unsigned long rate)
{
	struct clk *fout_epll;

	fout_epll = clk_get(NULL, "fout_epll");
	if (IS_ERR(fout_epll)) {
		printk(KERN_ERR "%s: failed to get fout_epll\n", __func__);
		return -ENOENT;
	}

	if (rate == clk_get_rate(fout_epll))
		goto out;

	clk_set_rate(fout_epll, rate);
out:
	clk_put(fout_epll);

	return 0;
}
#endif /* CONFIG_SND_SAMSUNG_I2S_MASTER */

#ifndef CONFIG_SND_SAMSUNG_I2S_MASTER
static int midas_wm1811_aif1_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	unsigned int pll_out;
	int ret;

	/* AIF1CLK should be >=3MHz for optimal performance */
	if (params_rate(params) == 8000 || params_rate(params) == 11025)
		pll_out = params_rate(params) * 512;
	else
		pll_out = params_rate(params) * 256;

	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S
					| SND_SOC_DAIFMT_NB_NF
					| SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0)
		return ret;

	/* Set the cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S
					| SND_SOC_DAIFMT_NB_NF
					| SND_SOC_DAIFMT_CBM_CFM);
	if (ret < 0)
		return ret;

#if 0
	ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_FLL1,
					pll_out, SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_sysclk(cpu_dai, SAMSUNG_I2S_OPCLK,
					0, MOD_OPCLK_PCLK);
	if (ret < 0)
		return ret;
#else
	midas_start_fll1(codec_dai);
#endif

	if (ret < 0)
		return ret;

	return 0;
}
#else /* CONFIG_SND_SAMSUNG_I2S_MASTER */
static int midas_wm1811_aif1_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	int bfs, psr, rfs, ret;
	unsigned long rclk;

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_U24:
	case SNDRV_PCM_FORMAT_S24:
		bfs = 48;
		break;
	case SNDRV_PCM_FORMAT_U16_LE:
	case SNDRV_PCM_FORMAT_S16_LE:
		bfs = 32;
		break;
	default:
		return -EINVAL;
	}

	switch (params_rate(params)) {
	case 16000:
	case 22050:
	case 24000:
	case 32000:
	case 44100:
	case 48000:
	case 88200:
	case 96000:
		if (bfs == 48)
			rfs = 384;
		else
			rfs = 256;
		break;
	case 64000:
		rfs = 384;
		break;
	case 8000:
	case 11025:
	case 12000:
		if (bfs == 48)
			rfs = 768;
		else
			rfs = 512;
		break;
	default:
		return -EINVAL;
	}

	rclk = params_rate(params) * rfs;

	switch (rclk) {
	case 4096000:
	case 5644800:
	case 6144000:
	case 8467200:
	case 9216000:
		psr = 8;
		break;
	case 8192000:
	case 11289600:
	case 12288000:
	case 16934400:
	case 18432000:
		psr = 4;
		break;
	case 22579200:
	case 24576000:
	case 33868800:
	case 36864000:
		psr = 2;
		break;
	case 67737600:
	case 73728000:
		psr = 1;
		break;
	default:
		printk(KERN_INFO "Not yet supported!\n");
		return -EINVAL;
	}

	set_epll_rate(rclk * psr);

	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S
					| SND_SOC_DAIFMT_NB_NF
					| SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S
					| SND_SOC_DAIFMT_NB_NF
					| SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_MCLK1,
					rclk, SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_sysclk(cpu_dai, SAMSUNG_I2S_CDCLK,
					0, SND_SOC_CLOCK_OUT);
	if (ret < 0)
		return ret;

	ret = snd_soc_dai_set_clkdiv(cpu_dai, SAMSUNG_I2S_DIV_BCLK, bfs);
	if (ret < 0)
		return ret;

	return 0;
}
#endif /* CONFIG_SND_SAMSUNG_I2S_MASTER */

/*
 * Midas WM1811 DAI operations.
 */
static struct snd_soc_ops midas_wm1811_aif1_ops = {
	.hw_params = midas_wm1811_aif1_hw_params,
};

static int midas_wm1811_aif2_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	int ret;
	int prate;
	int bclk;

	prate = params_rate(params);
	switch (params_rate(params)) {
	case 8000:
	case 16000:
	       break;
	default:
		dev_warn(codec_dai->dev, "Unsupported LRCLK %d, falling back to 8000Hz\n",
				(int)params_rate(params));
		prate = 8000;
	}

#ifdef CONFIG_LTE_MODEM_CMC221
	/* Set the codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_DSP_A
					| SND_SOC_DAIFMT_IB_NF
					| SND_SOC_DAIFMT_CBS_CFS);
#else
	/* Set the codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S
					| SND_SOC_DAIFMT_NB_NF
					| SND_SOC_DAIFMT_CBS_CFS);
#endif

	if (ret < 0)
		return ret;

#ifdef CONFIG_LTE_MODEM_CMC221
	bclk = 2048000;
#else
	switch (prate) {
	case 8000:
		bclk = 256000;
		break;
	case 16000:
		bclk = 512000;
		break;
	default:
		return -EINVAL;
	}
#endif

	ret = snd_soc_dai_set_pll(codec_dai, WM8994_FLL2,
					WM8994_FLL_SRC_BCLK,
					bclk, prate * 256);
	if (ret < 0)
		dev_err(codec_dai->dev, "Unable to configure FLL2: %d\n", ret);

	ret = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_FLL2,
					prate * 256, SND_SOC_CLOCK_IN);
	if (ret < 0)
		dev_err(codec_dai->dev, "Unable to switch to FLL2: %d\n", ret);

	return 0;
}

static struct snd_soc_ops midas_wm1811_aif2_ops = {
	.hw_params = midas_wm1811_aif2_hw_params,
};

static int midas_wm1811_aif3_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	pr_err("%s: enter\n", __func__);
	return 0;
}

static struct snd_soc_ops midas_wm1811_aif3_ops = {
	.hw_params = midas_wm1811_aif3_hw_params,
};

static const struct snd_kcontrol_new midas_controls[] = {
	SOC_DAPM_PIN_SWITCH("HP"),
	SOC_DAPM_PIN_SWITCH("SPK"),
	SOC_DAPM_PIN_SWITCH("RCV"),
	SOC_DAPM_PIN_SWITCH("FM In"),
	SOC_DAPM_PIN_SWITCH("LINE"),
};

const struct snd_soc_dapm_widget midas_dapm_widgets[] = {
	SND_SOC_DAPM_HP("HP", NULL),
	SND_SOC_DAPM_SPK("SPK", midas_ext_spkmode),
	SND_SOC_DAPM_SPK("RCV", NULL),
	SND_SOC_DAPM_LINE("LINE", NULL),

	SND_SOC_DAPM_MIC("Headset Mic", NULL),
	SND_SOC_DAPM_MIC("Main Mic", midas_ext_micbias),
	SND_SOC_DAPM_MIC("Sub Mic", midas_ext_submicbias),
	SND_SOC_DAPM_LINE("FM In", NULL),

	SND_SOC_DAPM_INPUT("S5P RP"),
};

const struct snd_soc_dapm_route midas_dapm_routes[] = {
	{ "HP", NULL, "HPOUT1L" },
	{ "HP", NULL, "HPOUT1R" },

	{ "SPK", NULL, "SPKOUTLN" },
	{ "SPK", NULL, "SPKOUTLP" },
	{ "SPK", NULL, "SPKOUTRN" },
	{ "SPK", NULL, "SPKOUTRP" },

	{ "RCV", NULL, "HPOUT2N" },
	{ "RCV", NULL, "HPOUT2P" },

	{ "LINE", NULL, "LINEOUT2N" },
	{ "LINE", NULL, "LINEOUT2P" },

	{ "IN1LP", NULL, "Main Mic" },
	{ "IN1LN", NULL, "Main Mic" },

	{ "IN1RP", NULL, "Sub Mic" },
	{ "IN1RN", NULL, "Sub Mic" },

	{ "IN2LP:VXRN", NULL, "MICBIAS2" },
	{ "MICBIAS2", NULL, "Headset Mic" },

	{ "AIF1DAC1L", NULL, "S5P RP" },
	{ "AIF1DAC1R", NULL, "S5P RP" },

	{ "IN2RN", NULL, "FM In" },
	{ "IN2RP:VXRP", NULL, "FM In" },
};

struct wm1811_machine_priv {
	struct snd_soc_jack jack;
	struct snd_soc_codec *codec;
	struct delayed_work mic_work;
};

static void wm1811_mic_work(struct work_struct *work)
{
	int report = 0;
	struct wm1811_machine_priv *wm1811;
	struct snd_soc_codec *codec;
	int status;

	wm1811 = container_of(work, struct wm1811_machine_priv,
							mic_work.work);
	codec = wm1811->codec;

	status = snd_soc_read(codec, WM8958_MIC_DETECT_3);
	if (status < 0) {
		dev_err(codec->dev, "Failed to read mic detect status: %d\n",
				status);
		return;
	}

	/* If nothing present then clear our statuses */
	if (!(status & WM8958_MICD_STS))
		goto done;

	report = SND_JACK_HEADSET;

	/* Everything else is buttons; just assign slots */
	if (status & WM1811_JACKDET_BTN0)
		report |= SND_JACK_BTN_0;
	if (status & WM1811_JACKDET_BTN1)
		report |= SND_JACK_BTN_1;
	if (status & WM1811_JACKDET_BTN2)
		report |= SND_JACK_BTN_2;

	if (report & SND_JACK_MICROPHONE)
		dev_crit(codec->dev, "Reporting microphone\n");
	if (report & SND_JACK_HEADPHONE)
		dev_crit(codec->dev, "Reporting headphone\n");
	if (report & SND_JACK_BTN_0)
		dev_crit(codec->dev, "Reporting button 0\n");
	if (report & SND_JACK_BTN_1)
		dev_crit(codec->dev, "Reporting button 1\n");
	if (report & SND_JACK_BTN_2)
		dev_crit(codec->dev, "Reporting button 2\n");

done:
	if (!report)
		dev_crit(codec->dev, "Reporting open circuit\n");

	snd_soc_jack_report(&wm1811->jack, report,
				SND_JACK_BTN_0 | SND_JACK_BTN_1 |
				SND_JACK_BTN_2 | SND_JACK_HEADSET);
}

static struct snd_soc_dai_driver midas_ext_dai[] = {
	{
		.name = "midas.cp",
		.playback = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 16000,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.capture = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 16000,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
	},
	{
		.name = "midas.bt",
		.playback = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 16000,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
		.capture = {
			.channels_min = 1,
			.channels_max = 2,
			.rate_min = 8000,
			.rate_max = 16000,
			.rates = SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000,
			.formats = SNDRV_PCM_FMTBIT_S16_LE,
		},
	},
};

static ssize_t earjack_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct snd_soc_codec *codec = dev_get_drvdata(dev);

	int status;
	int report = 0;

	status = snd_soc_read(codec, WM8958_MIC_DETECT_3);

	if (status < 0) {
		dev_err(codec->dev, "Failed to read mic detect status: %d\n",
				status);
		goto done;
	}

	/* If nothing present then clear our statuses */
	if (!(status & WM8958_MICD_STS))
		goto done;

	report = 1;

done:
	return sprintf(buf, "%d\n", report);
}

static ssize_t earjack_state_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	pr_info("%s : operate nothing\n", __func__);

	return size;
}

static ssize_t earjack_key_state_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct snd_soc_codec *codec = dev_get_drvdata(dev);

	int status;
	int report = 0;

	status = snd_soc_read(codec, WM8958_MIC_DETECT_3);

	if (status < 0) {
		dev_err(codec->dev, "Failed to read mic detect status: %d\n",
				status);
		goto done;
	}

	/* If nothing present then clear our statuses */
	if (!(status & WM8958_MICD_STS))
		goto done;

	if (status & WM1811_JACKDET_BTN0)
		report = 1;

done:
	return sprintf(buf, "%d\n", report);
}

static ssize_t earjack_key_state_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	pr_info("%s : operate nothing\n", __func__);

	return size;
}

static ssize_t earjack_select_jack_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	pr_info("%s : operate nothing\n", __func__);

	return 0;
}

static ssize_t earjack_select_jack_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct snd_soc_codec *codec = dev_get_drvdata(dev);
	struct wm8994_priv *wm8994 = snd_soc_codec_get_drvdata(codec);

	dev_info(codec->dev, "Forced detect microphone\n");

	wm8994->mic_detecting = false;
	wm8994->jack_mic = true;

	midas_micd_set_rate(codec);

	if ((!size) || (buf[0] != '1')) {
		snd_soc_jack_report(wm8994->micdet[0].jack, 0,
				SND_JACK_HEADSET);
	}

	snd_soc_jack_report(wm8994->micdet[0].jack, SND_JACK_HEADSET,
				SND_JACK_HEADSET);

	return size;
}

static DEVICE_ATTR(select_jack, S_IRUGO | S_IWUSR | S_IWGRP,
		   earjack_select_jack_show, earjack_select_jack_store);

static DEVICE_ATTR(key_state, S_IRUGO | S_IWUSR | S_IWGRP,
		   earjack_key_state_show, earjack_key_state_store);

static DEVICE_ATTR(state, S_IRUGO | S_IWUSR | S_IWGRP,
		   earjack_state_show, earjack_state_store);

static int midas_wm1811_init_paiftx(struct snd_soc_pcm_runtime *rtd)
{

	struct wm1811_machine_priv *wm1811;
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dai *aif1_dai = rtd->codec_dai;
	int ret;

#ifndef MANAGE_MCLK1
	midas_set_mclk(1);
#endif

	ret = snd_soc_add_controls(codec, midas_controls,
					ARRAY_SIZE(midas_controls));

	ret = snd_soc_dapm_new_controls(&codec->dapm, midas_dapm_widgets,
					   ARRAY_SIZE(midas_dapm_widgets));
	if (ret != 0)
		dev_err(codec->dev, "Failed to add DAPM widgets: %d\n", ret);

	ret = snd_soc_dapm_add_routes(&codec->dapm, midas_dapm_routes,
					   ARRAY_SIZE(midas_dapm_routes));
	if (ret != 0)
		dev_err(codec->dev, "Failed to add DAPM routes: %d\n", ret);

	ret = snd_soc_dai_set_sysclk(aif1_dai, WM8994_SYSCLK_MCLK2,
				     32768, SND_SOC_CLOCK_IN);
	if (ret < 0)
		dev_err(codec->dev, "Failed to boot clocking\n");

	/* Force AIF1CLK on as it will be master for jack detection */
	ret = snd_soc_dapm_force_enable_pin(&codec->dapm, "AIF1CLK");
	if (ret < 0)
		dev_err(codec->dev, "Failed to enable AIF1CLK: %d\n", ret);

	ret = snd_soc_dapm_disable_pin(&codec->dapm, "S5P RP");
	if (ret < 0)
		dev_err(codec->dev, "Failed to disable S5P RP: %d\n", ret);

	wm1811 = kmalloc(sizeof *wm1811, GFP_KERNEL);
	if (!wm1811) {
		dev_err(codec->dev, "Failed to allocate memory!");
		return -ENOMEM;
	}

	snd_soc_dapm_ignore_suspend(&codec->dapm, "RCV");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "SPK");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "HP");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "Headset Mic");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "Sub Mic");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "Main Mic");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "AIF1DACDAT");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "AIF2DACDAT");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "AIF3DACDAT");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "AIF1ADCDAT");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "AIF2ADCDAT");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "AIF3ADCDAT");
	snd_soc_dapm_ignore_suspend(&codec->dapm, "FM In");

	wm1811->codec = codec;

	INIT_DELAYED_WORK(&wm1811->mic_work, wm1811_mic_work);

	ret = snd_soc_jack_new(codec, "Midas Jack",
				SND_JACK_HEADSET | SND_JACK_BTN_0 |
				SND_JACK_BTN_1 | SND_JACK_BTN_2,
				&wm1811->jack);

	if (ret < 0)
		dev_err(codec->dev, "Failed to create jack: %d\n", ret);

	ret = snd_jack_set_key(wm1811->jack.jack, SND_JACK_BTN_0, KEY_MEDIA);
	if (ret < 0)
		dev_err(codec->dev, "Failed to set KEY_MEDIA: %d\n", ret);

	ret = snd_jack_set_key(wm1811->jack.jack, SND_JACK_BTN_1,
							KEY_VOLUMEDOWN);
	if (ret < 0)
		dev_err(codec->dev, "Failed to set KEY_VOLUMEUP: %d\n", ret);

	ret = snd_jack_set_key(wm1811->jack.jack, SND_JACK_BTN_2,
							KEY_VOLUMEUP);
	if (ret < 0)
		dev_err(codec->dev, "Failed to set KEY_VOLUMEDOWN: %d\n", ret);

	ret = wm8958_mic_detect(codec, &wm1811->jack, midas_micdet, codec);
	if (ret < 0)
		dev_err(codec->dev, "Failed start detection: %d\n", ret);


	/* To support PBA function test */
	jack_class = class_create(THIS_MODULE, "audio");

	if (IS_ERR(jack_class))
		pr_err("Failed to create class\n");

	jack_dev = device_create(jack_class, NULL, 0, codec, "earjack");

	if (device_create_file(jack_dev, &dev_attr_select_jack) < 0)
		pr_err("Failed to create device file (%s)!\n",
			dev_attr_select_jack.attr.name);

	if (device_create_file(jack_dev, &dev_attr_key_state) < 0)
		pr_err("Failed to create device file (%s)!\n",
			dev_attr_key_state.attr.name);

	if (device_create_file(jack_dev, &dev_attr_state) < 0)
		pr_err("Failed to create device file (%s)!\n",
			dev_attr_state.attr.name);

	mutex_init(&midas_lock);

	return snd_soc_dapm_sync(&codec->dapm);

}

static struct snd_soc_dai_link midas_dai[] = {
	{ /* Sec_Fifo DAI i/f */
		.name = "Sec_FIFO TX",
		.stream_name = "Sec_Dai",
		.cpu_dai_name = "samsung-i2s.4",
		.codec_dai_name = "wm8994-aif1",
#ifndef CONFIG_SND_SOC_SAMSUNG_USE_DMA_WRAPPER
		.platform_name = "samsung-audio-idma",
#else
		.platform_name = "samsung-audio",
#endif
		.codec_name = "wm8994-codec",
		.init = midas_wm1811_init_paiftx,
		.ops = &midas_wm1811_aif1_ops,
	},
	{
		.name = "Midas_WM1811 Voice",
		.stream_name = "Voice Tx/Rx",
		.cpu_dai_name = "midas.cp",
		.codec_dai_name = "wm8994-aif2",
		.platform_name = "snd-soc-dummy",
		.codec_name = "wm8994-codec",
		.ops = &midas_wm1811_aif2_ops,
		.ignore_suspend = 1,
	},
	{
		.name = "Midas_WM1811 BT",
		.stream_name = "BT Tx/Rx",
		.cpu_dai_name = "midas.bt",
		.codec_dai_name = "wm8994-aif3",
		.platform_name = "snd-soc-dummy",
		.codec_name = "wm8994-codec",
		.ops = &midas_wm1811_aif3_ops,
		.ignore_suspend = 1,
	},
	{ /* Primary DAI i/f */
		.name = "WM8994 AIF1",
		.stream_name = "Pri_Dai",
		.cpu_dai_name = "samsung-i2s.0",
		.codec_dai_name = "wm8994-aif1",
		.platform_name = "samsung-audio",
		.codec_name = "wm8994-codec",
		.ops = &midas_wm1811_aif1_ops,
	},
};

static int midas_card_suspend(struct snd_soc_card *card)
{
	midas_set_mclk(0);

	return 0;
}

static int midas_card_resume(struct snd_soc_card *card)
{
	midas_set_mclk(1);

	return 0;
}

static int midas_set_bias_level(struct snd_soc_card *card,
				enum snd_soc_bias_level level)
{
	mutex_lock(&midas_lock);
	switch (level) {
	case SND_SOC_BIAS_PREPARE:
		if (midas_card_bias_level == SND_SOC_BIAS_STANDBY)
			midas_start_fll1(card->rtd[0].codec_dai);
		break;

	default:
		break;
	}
	mutex_unlock(&midas_lock);

	return 0;
}

static int midas_set_bias_level_post(struct snd_soc_card *card,
					 enum snd_soc_bias_level level)
{
	struct snd_soc_codec *codec = card->rtd->codec;
	struct snd_soc_dai *aif1_dai = card->rtd[0].codec_dai;
	struct snd_soc_dai *aif2_dai = card->rtd[1].codec_dai;
	int ret;

	mutex_lock(&midas_lock);
	switch (level) {
	case SND_SOC_BIAS_STANDBY:

		/* When going idle stop FLL1 and revert to using MCLK2
		 * directly for minimum power consumptin for accessory
		 * detection.
		 */
		if (midas_card_bias_level == SND_SOC_BIAS_PREPARE) {
			dev_info(aif1_dai->dev, "Moving to STANDBY\n");

			ret = snd_soc_dai_set_sysclk(aif2_dai,
							WM8994_SYSCLK_MCLK2,
							32768,
							SND_SOC_CLOCK_IN);
			if (ret < 0)
				dev_err(codec->dev, "Failed to switch to MCLK2\n");

			ret = snd_soc_dai_set_pll(aif2_dai, WM8994_FLL2,
							0, 0, 0);

			if (ret < 0)
				dev_err(codec->dev,
					"Failed to change FLL2\n");

			ret = snd_soc_dai_set_sysclk(aif1_dai,
						     WM8994_SYSCLK_MCLK2,
						     32768,
						     SND_SOC_CLOCK_IN);
			if (ret < 0)
				dev_err(codec->dev,
					"Failed to switch to MCLK2\n");

			ret = snd_soc_dai_set_pll(aif1_dai, WM8994_FLL1,
						  0, 0, 0);
			if (ret < 0)
				dev_err(codec->dev,
					"Failed to stop FLL1\n");


			midas_fll1_active = false;

			midas_micd_set_rate(codec);
			midas_set_mclk(0);
		}

		break;
	default:
		break;
	}

#if 0 /* TODO */
	codec->dapm.bias_level = level;
#else
	midas_card_bias_level = level;
#endif
	mutex_unlock(&midas_lock);

	return 0;
}

static struct snd_soc_card midas = {
	.name = "Midas_WM1811",
	.dai_link = midas_dai,

	/* If you want to use sec_fifo device,
	 * changes the num_link = 2 or ARRAY_SIZE(midas_dai). */
	.num_links = ARRAY_SIZE(midas_dai),

	.set_bias_level = midas_set_bias_level,
	.set_bias_level_post = midas_set_bias_level_post,

	.suspend_post = midas_card_suspend,
	.resume_pre = midas_card_resume
};

static struct platform_device *midas_snd_device;

static int __init midas_audio_init(void)
{
	int ret;

	midas_snd_device = platform_device_alloc("soc-audio", -1);
	if (!midas_snd_device)
		return -ENOMEM;

	ret = snd_soc_register_dais(&midas_snd_device->dev,
					midas_ext_dai, ARRAY_SIZE(midas_ext_dai));
	if (ret != 0)
		pr_err("Failed to register external DAIs: %d\n", ret);

	platform_set_drvdata(midas_snd_device, &midas);

	ret = platform_device_add(midas_snd_device);
	if (ret)
		platform_device_put(midas_snd_device);

	midas_gpio_init();

	return ret;
}
module_init(midas_audio_init);

static void __exit midas_audio_exit(void)
{
	platform_device_unregister(midas_snd_device);
}
module_exit(midas_audio_exit);

MODULE_AUTHOR("JS. Park <aitdark.park@samsung.com>");
MODULE_DESCRIPTION("ALSA SoC Midas WM1811");
MODULE_LICENSE("GPL");


