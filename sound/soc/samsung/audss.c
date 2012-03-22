/* sound/soc/samsung/audss.c
 *
 * ALSA SoC Audio Layer - Samsung Audio Subsystem driver
 *
 * Copyright (c) 2010 Samsung Electronics Co. Ltd.
 *	Lakkyung Jung <lakkyung.jung@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <sound/soc.h>
#include <sound/pcm_params.h>

#include <plat/audio.h>
#include <plat/clock.h>
#include <mach/map.h>
#include <mach/regs-audss.h>

#include "audss.h"

static struct audss_runtime_data {
	struct clk *mout_audss;
	struct clk *dout_srp;
	struct clk *rclk;
	struct clk *srp_clk;

	u32	clk_src_rate;
	u32	suspend_audss_clksrc;
	u32	suspend_audss_clkdiv;
	u32	suspend_audss_clkgate;

	bool	clk_enabled;
	bool	reg_saved;
} audss;

static char *rclksrc[] = {
	[0] = "busclk",
	[1] = "i2sclk",
};

/* Lock for cross i/f checks */
static DEFINE_SPINLOCK(lock);

int audss_set_clk_div(u32 mode)
{
	u32 srp_clk_rate = 0;
	u32 rclk_rate = 0;
	u32 rclk_shift = 0;
	u32 ret = -1;

#ifdef CONFIG_SND_SAMSUNG_I2S_MASTER
	pr_debug("%s: Do not set div reg in i2s master mode\n", __func__);
	return 0;
#endif

	srp_clk_rate = clk_get_rate(audss.srp_clk);
	if (!srp_clk_rate) {
		pr_err("%s: Can't get current clk_rate %d\n",
			__func__, srp_clk_rate);
		return ret;
	}

	pr_debug("%s: Current src clock %d\n", __func__, srp_clk_rate);

	if (mode == AUDSS_ACTIVE) {
		switch (srp_clk_rate) {
		case 180000000:
		case 180633600:
			rclk_shift = 2; /* div = 4 */
			break;
		case 96000000:
			rclk_shift = 1; /* div = 2 */
			break;
		}
	} else {
		switch (srp_clk_rate) {
		case 180000000:
		case 180633600:
			rclk_shift = 4; /* div = 16 */
			break;
		case 96000000:
			rclk_shift = 3; /* div = 8 */
			break;
		}
	}
	pr_debug("%s: rclk shift [%d]\n", __func__, rclk_shift);

	rclk_rate = srp_clk_rate >> rclk_shift;
	if (rclk_rate != clk_get_rate(audss.rclk)) {
		clk_set_rate(audss.rclk, rclk_rate);
		pr_debug("%s: I2S_CLK[%ld]\n",
			__func__, clk_get_rate(audss.rclk));
	}

	pr_debug("%s: AUDSS DIV REG[0x%x]\n",
		__func__, readl(S5P_CLKDIV_AUDSS));

	return 0;
}

void audss_reg_save_restore(int cmd)
{
	if (!cmd) {
		if (audss.reg_saved)
			goto exit_func;

		audss.suspend_audss_clksrc = readl(S5P_CLKSRC_AUDSS);
		audss.suspend_audss_clkdiv = readl(S5P_CLKDIV_AUDSS);
		audss.suspend_audss_clkgate = readl(S5P_CLKGATE_AUDSS);
		audss.reg_saved = true;
	} else {
		if (!audss.reg_saved)
			goto exit_func;

		writel(audss.suspend_audss_clksrc, S5P_CLKSRC_AUDSS);
		writel(audss.suspend_audss_clkdiv, S5P_CLKDIV_AUDSS);
		writel(audss.suspend_audss_clkgate, S5P_CLKGATE_AUDSS);
		audss.reg_saved = false;
	}

exit_func:
	return;
}

void audss_clk_enable(bool enable)
{
	unsigned long flags;

	pr_debug("%s: state %d\n", __func__, enable);
	spin_lock_irqsave(&lock, flags);

	if (enable) {
		if (audss.clk_enabled) {
			pr_debug("%s: Already enabled audss clk %d\n",
					__func__, audss.clk_enabled);
			goto exit_func;
		}

		clk_enable(audss.rclk);
		clk_enable(audss.srp_clk);

		audss_reg_save_restore(AUDSS_REG_RESTORE);
		audss_set_clk_div(AUDSS_ACTIVE);
		audss.clk_enabled = true;


		pr_info("%s:ON : CLKSRC[0x%x], CLKGATE[0x%x]\n", __func__,
				readl(S5P_CLKSRC_AUDSS),
				readl(S5P_CLKGATE_AUDSS));

	} else {
		if (!audss.clk_enabled) {
			pr_debug("%s: Already disabled audss clk %d\n",
					__func__, audss.clk_enabled);
			goto exit_func;
		}

		audss_set_clk_div(AUDSS_INACTIVE);
		audss_reg_save_restore(AUDSS_REG_SAVE);

		clk_disable(audss.rclk);
		clk_disable(audss.srp_clk);
		audss.clk_enabled = false;

		pr_info("%s:OFF: CLKSRC[0x%x], CLKGATE[0x%x]\n", __func__,
				readl(S5P_CLKSRC_AUDSS),
				readl(S5P_CLKGATE_AUDSS));
	}

exit_func:
	spin_unlock_irqrestore(&lock, flags);

	return;
}

static __devinit int audss_init(void)
{
	int ret = 0;

	audss.mout_audss = clk_get(NULL, "mout_audss");
	if (IS_ERR(audss.mout_audss)) {
		pr_err("%s: failed to get mout audss\n", __func__);
		ret = PTR_ERR(audss.mout_audss);
		return ret;
	}

	audss.dout_srp = clk_get(NULL, "dout_srp");
	if (IS_ERR(audss.dout_srp)) {
		pr_err("%s: failed to get dout_srp\n", __func__);
		ret = PTR_ERR(audss.dout_srp);
		goto err1;
	}

	audss.rclk = clk_get(NULL, rclksrc[BUSCLK]);
	if (IS_ERR(audss.rclk)) {
		pr_err("%s: failed to get i2s root clk\n", __func__);
		ret = PTR_ERR(audss.rclk);
		goto err2;
	}

	audss.srp_clk = clk_get(NULL, "srpclk");
	if (IS_ERR(audss.srp_clk)) {
		pr_err("%s:failed to get srp_clk\n", __func__);
		ret = PTR_ERR(audss.srp_clk);
		goto err3;
	}

	if (!strcmp(audss.rclk->name, "i2sclk")) {
		if (clk_set_parent(audss.rclk, audss.mout_audss)) {
			pr_err("unable to set parent %s of clock %s.\n",
				audss.mout_audss->name, audss.rclk->name);
			goto err4;
		}
	}

	ret = audss_set_clk_div(AUDSS_INACTIVE);
	if (ret < 0) {
		pr_err("%s:failed to set clock rate\n", __func__);
		goto err4;
	}

	audss.clk_enabled = false;
	audss.reg_saved = false;

	return ret;
err4:
	clk_put(audss.srp_clk);
err3:
	clk_put(audss.rclk);
err2:
	clk_put(audss.dout_srp);
err1:
	clk_put(audss.mout_audss);

	return ret;
}

static __devexit int audss_deinit(void)
{
	clk_put(audss.rclk);
	audss.rclk = NULL;

	if (audss.srp_clk) {
		clk_put(audss.srp_clk);
		audss.srp_clk = NULL;
	}

	return 0;
}

static char banner[] __initdata = "Samsung Audio Subsystem Driver, (c) 2011 Samsung Electronics";

static int __init samsung_audss_init(void)
{
	int ret = 0;

	pr_info("%s\n", banner);

	ret = audss_init();
	if (ret < 0)
		pr_err("%s:failed to init audss clock\n", __func__);

	return ret;

}
module_init(samsung_audss_init);

static void __exit samsung_audss_exit(void)
{
	audss_deinit();
}
module_exit(samsung_audss_exit);

/* Module information */
MODULE_AUTHOR("Lakkyung Jung, <lakkyung.jung@samsung.com>");
MODULE_DESCRIPTION("Samsung Audio subsystem Interface");
MODULE_ALIAS("platform:samsung-audss");
MODULE_LICENSE("GPL");
