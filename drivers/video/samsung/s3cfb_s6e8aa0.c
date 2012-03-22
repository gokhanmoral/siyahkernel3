/* linux/drivers/video/samsung/s3cfb_s6e8aa0.c
 *
 * MIPI-DSI based AMS529HA01 AMOLED lcd panel driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/ctype.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>
#include <linux/backlight.h>
#include <linux/lcd.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-dsim.h>
#include <mach/dsim.h>
#include <mach/mipi_ddi.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include "s5p-dsim.h"
#include "s3cfb.h"
#include "s6e8aa0_gamma_l.h"

#if defined(CONFIG_MACH_Q1_BD)
#include "s6e8aa0_gamma_q1.h"
#define SMART_DIMMING
#elif defined(CONFIG_MACH_M0) || defined(CONFIG_MACH_C1) ||\
	defined(CONFIG_MACH_C1VZW) || defined(CONFIG_MACH_SLP_PQ) ||\
	defined(CONFIG_MACH_SLP_PQ_LTE) || defined(CONFIG_MACH_M3) ||\
	defined(CONFIG_MACH_C1CTC) || defined(CONFIG_MACH_M0_CHNOPEN) ||\
	defined(CONFIG_MACH_M0_CMCC)
#include "s6e8aa0_gamma_c1m0.h"
#define SMART_DIMMING
#else
#include "s6e8aa0_gamma_midas.h"
#define SMART_DIMMING
#endif

#ifdef SMART_DIMMING
#include "smart_dimming.h"
#endif

#define POWER_IS_ON(pwr)	((pwr) <= FB_BLANK_NORMAL)

#define MIN_BRIGHTNESS	0
#define MAX_BRIGHTNESS	255
#if defined(CONFIG_MACH_Q1_BD)
#define MAX_GAMMA		290
#define DEFUALT_GAMMA		150
#define DEFAULT_BRIGHTNESS	120
#define DEFAULT_GAMMA_LEVEL	GAMMA_150CD
#else
#define MAX_GAMMA		300
#define DEFUALT_GAMMA		160
#define DEFAULT_BRIGHTNESS	125
#define DEFAULT_GAMMA_LEVEL	GAMMA_160CD
#endif

#define LDI_ID_REG			0xD1
#define LDI_ID_LEN			3

#ifdef SMART_DIMMING
#define	PANEL_A1_M3			0xA1
#define	PANEL_A2_M3			0xA2
#define	PANEL_A1_SM2			0x12

#define LDI_MTP_LENGTH		24
#define LDI_MTP_ADDR			0xD3

#define ELVSS_OFFSET_MAX		0x00
#define ELVSS_OFFSET_2		0x08
#if defined(CONFIG_MACH_Q1_BD)
#define ELVSS_OFFSET_1		0x0D
#else
#define ELVSS_OFFSET_1		ELVSS_OFFSET_2
#endif
#define ELVSS_OFFSET_MIN		0x12

#define DYNAMIC_ELVSS_MIN_VALUE	0x81
#define DYNAMIC_ELVSS_MAX_VALUE	0x9F

#define ELVSS_MODE0_MIN_VOLTAGE	62
#define ELVSS_MODE1_MIN_VOLTAGE	52

struct str_elvss {
	u8 reference;
	u8 limit;
};
#endif

#if defined(CONFIG_MACH_C1CTC) || defined(CONFIG_MACH_M0_CHNOPEN) ||\
	defined(CONFIG_MACH_M0_CMCC)
unsigned int lcdtype;
EXPORT_SYMBOL(lcdtype);
#endif

struct lcd_info {
	unsigned int			bl;
	unsigned int			acl_enable;
	unsigned int			cur_acl;
	unsigned int			current_bl;

	unsigned int			ldi_enable;
	unsigned int			power;
	struct mutex			lock;
	struct mutex			bl_lock;

	struct device			*dev;
	struct lcd_device		*ld;
	struct backlight_device		*bd;
	struct lcd_platform_data	*lcd_pd;
	struct early_suspend		early_suspend;

	unsigned char			id[LDI_ID_LEN];

	unsigned char			**gamma_table;
	unsigned char			**elvss_table;

#ifdef SMART_DIMMING
	unsigned int			support_elvss;
	unsigned int			aid;

	struct str_smart_dim		smart;
	struct str_elvss		elvss;
#endif
	unsigned int			irq;
	unsigned int			connected;
};

static struct mipi_ddi_platform_data *ddi_pd;

static const unsigned int candela_table[GAMMA_MAX] = {
	 30,  40,  50,  60,  70,  80,  90, 100, 110, 120,
	130, 140, 150, 160, 170, 180, 190, 200, 210, 220,
	230, 240, 250, 260, 270, 280, MAX_GAMMA
};

#if defined(GPIO_OLED_DET)
struct delayed_work hs_clk_re_try;
unsigned int count_dsim;

static void hs_clk_re_try_work(struct work_struct *work)
{
	int read_oled_det;

	read_oled_det = gpio_get_value(GPIO_OLED_DET);

	printk(KERN_INFO "%s, %d, %d\n", __func__, count_dsim, read_oled_det);

	if (read_oled_det == 0) {
		if (count_dsim < 10) {
			schedule_delayed_work(&hs_clk_re_try, HZ/8);
			count_dsim++;
			set_dsim_hs_clk_toggle_count(15);
		} else
			s5p_dsim_frame_done_interrupt_enable(0);
	} else
		s5p_dsim_frame_done_interrupt_enable(0);
}

static irqreturn_t oled_det_int(int irq, void *dev_id)
{
	printk(KERN_INFO "[DSIM] %s\n", __func__);

	schedule_delayed_work(&hs_clk_re_try, HZ/16);

	count_dsim = 0;

	return IRQ_HANDLED;
}
#endif

static int s6e8ax0_write(struct lcd_info *lcd, const unsigned char *seq, int len)
{
	int size;
	const unsigned char *wbuf;

	if (!lcd->connected)
		return 0;

	mutex_lock(&lcd->lock);

	size = len;
	wbuf = seq;

	if (size == 1)
		ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_WR_NO_PARA, wbuf[0], 0);
	else if (size == 2)
		ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_WR_1_PARA, wbuf[0], wbuf[1]);
	else
		ddi_pd->cmd_write(ddi_pd->dsim_base, DCS_LONG_WR, (unsigned int)wbuf, size);

	mutex_unlock(&lcd->lock);

	return 0;
}

static int _s6e8ax0_read(struct lcd_info *lcd, const u8 addr, u16 count, u8 *buf)
{
	int ret = 0;

	if (!lcd->connected)
		return ret;

	mutex_lock(&lcd->lock);

	if (ddi_pd->cmd_read)
		ret = ddi_pd->cmd_read(ddi_pd->dsim_base, addr, count, buf);

	mutex_unlock(&lcd->lock);

	return ret;
}

static int s6e8ax0_set_link(void *pd, unsigned int dsim_base,
	unsigned char (*cmd_write) (unsigned int dsim_base, unsigned int data0,
	    unsigned int data1, unsigned int data2),
	int (*cmd_read) (u32 reg_base, u8 addr, u16 count, u8 *buf))
{
	struct mipi_ddi_platform_data *temp_pd = NULL;

	temp_pd = (struct mipi_ddi_platform_data *) pd;
	if (temp_pd == NULL) {
		printk(KERN_ERR "mipi_ddi_platform_data is null.\n");
		return -EPERM;
	}

	ddi_pd = temp_pd;

	ddi_pd->dsim_base = dsim_base;

	if (cmd_write)
		ddi_pd->cmd_write = cmd_write;
	else
		printk(KERN_WARNING "cmd_write function is null.\n");

	if (cmd_read)
		ddi_pd->cmd_read = cmd_read;
	else
		printk(KERN_WARNING "cmd_read function is null.\n");

	return 0;
}

static int s6e8ax0_read(struct lcd_info *lcd, const u8 addr, u16 count, u8 *buf, u8 retry_cnt)
{
	int ret = 0;

read_retry:
	ret = _s6e8ax0_read(lcd, addr, count, buf);
	if (!ret) {
		if (retry_cnt) {
			printk(KERN_WARNING "[WARN:LCD] %s : retry cnt : %d\n", __func__, retry_cnt);
			retry_cnt--;
			goto read_retry;
		} else
			printk(KERN_ERR "[ERROR:LCD] %s : 0x%02x read failed\n", __func__, addr);
	}

	return ret;
}

static int get_backlight_level_from_brightness(int brightness)
{
	int backlightlevel;

	/* brightness setting from platform is from 0 to 255
	 * But in this driver, brightness is only supported from 0 to 24 */

	switch (brightness) {
	case 0:
		backlightlevel = GAMMA_30CD;
		break;
	case 1 ... 29:
		backlightlevel = GAMMA_30CD;
		break;
	case 30 ... 34:
		backlightlevel = GAMMA_40CD;
		break;
	case 35 ... 39:
		backlightlevel = GAMMA_50CD;
		break;
	case 40 ... 44:
		backlightlevel = GAMMA_60CD;
		break;
	case 45 ... 49:
		backlightlevel = GAMMA_70CD;
		break;
	case 50 ... 54:
		backlightlevel = GAMMA_80CD;
		break;
	case 55 ... 64:
		backlightlevel = GAMMA_90CD;
		break;
	case 65 ... 74:
		backlightlevel = GAMMA_100CD;
		break;
	case 75 ... 83:
		backlightlevel = GAMMA_110CD;
		break;
	case 84 ... 93:
		backlightlevel = GAMMA_120CD;
		break;
	case 94 ... 103:
		backlightlevel = GAMMA_130CD;
		break;
	case 104 ... 113:
		backlightlevel = GAMMA_140CD;
		break;
	case 114 ... 122:
		backlightlevel = GAMMA_150CD;
		break;
	case 123 ... 132:
		backlightlevel = GAMMA_160CD;
		break;
	case 133 ... 142:
		backlightlevel = GAMMA_170CD;
		break;
	case 143 ... 152:
		backlightlevel = GAMMA_180CD;
		break;
	case 153 ... 162:
		backlightlevel = GAMMA_190CD;
		break;
	case 163 ... 171:
		backlightlevel = GAMMA_200CD;
		break;
	case 172 ... 181:
		backlightlevel = GAMMA_210CD;
		break;
	case 182 ... 191:
		backlightlevel = GAMMA_220CD;
		break;
	case 192 ... 201:
		backlightlevel = GAMMA_230CD;
		break;
	case 202 ... 210:
		backlightlevel = GAMMA_240CD;
		break;
	case 211 ... 220:
		backlightlevel = GAMMA_250CD;
		break;
	case 221 ... 230:
		backlightlevel = GAMMA_260CD;
		break;
	case 231 ... 240:
		backlightlevel = GAMMA_270CD;
		break;
	case 241 ... 250:
		backlightlevel = GAMMA_280CD;
		break;
	case 251 ... 255:
		backlightlevel = GAMMA_290CD;	/* or GAMMA_300CD */
		break;
	default:
		backlightlevel = DEFAULT_GAMMA_LEVEL;
		break;
	}
	return backlightlevel;
}

static int s6e8ax0_gamma_ctl(struct lcd_info *lcd)
{
	s6e8ax0_write(lcd, lcd->gamma_table[lcd->bl], GAMMA_PARAM_SIZE);

	/* Gamma Set Update */
	s6e8ax0_write(lcd, SEQ_GAMMA_UPDATE, ARRAY_SIZE(SEQ_GAMMA_UPDATE));

	return 0;
}

#if defined(CONFIG_MACH_Q1_BD)
static int s6e8ax0_set_acl(struct lcd_info *lcd)
{
	int ret = 0;

	if (lcd->acl_enable) {
		if (lcd->cur_acl == 0) {
			if (lcd->bl == 0 || lcd->bl == 1) {
				s6e8ax0_write(lcd, SEQ_ACL_OFF, ARRAY_SIZE(SEQ_ACL_OFF));
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d, acl_off\n", __func__, lcd->cur_acl);
			} else
				s6e8ax0_write(lcd, SEQ_ACL_ON, ARRAY_SIZE(SEQ_ACL_ON));
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d, acl_on\n", __func__, lcd->cur_acl);
		}
		switch (lcd->bl) {
		case 0 ... 1: /* 30cd ~ 40cd */
			if (lcd->cur_acl != 0) {
				s6e8ax0_write(lcd, SEQ_ACL_OFF, ARRAY_SIZE(SEQ_ACL_OFF));
				lcd->cur_acl = 0;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 2: /* 50cd */
			if (lcd->cur_acl != 200) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[ACL_STATUS_20P], ACL_PARAM_SIZE);
				lcd->cur_acl = 200;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 3: /* 60cd */
			if (lcd->cur_acl != 330) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[ACL_STATUS_33P], ACL_PARAM_SIZE);
				lcd->cur_acl = 330;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 4: /* 70cd */
			if (lcd->cur_acl != 430) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[ACL_STATUS_43P], ACL_PARAM_SIZE);
				lcd->cur_acl = 430;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 5: /* 80cd */
			if (lcd->cur_acl != 450) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[ACL_STATUS_45P_80CD], ACL_PARAM_SIZE);
				lcd->cur_acl = 450;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 6 ... 12: /* 90cd ~ 150cd */
			if (lcd->cur_acl != 451) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[ACL_STATUS_45P], ACL_PARAM_SIZE);
				lcd->cur_acl = 451;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 13: /* 160cd */
			if (lcd->cur_acl != 460) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[ACL_STATUS_46P_160CD], ACL_PARAM_SIZE);
				lcd->cur_acl = 460;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 14 ... 22: /* 170cd ~ 250cd */
			if (lcd->cur_acl != 461) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[ACL_STATUS_46P], ACL_PARAM_SIZE);
				lcd->cur_acl = 461;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 23: /* 260cd */
			if (lcd->cur_acl != 470) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[ACL_STATUS_47P], ACL_PARAM_SIZE);
				lcd->cur_acl = 470;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 24: /* 270cd */
			if (lcd->cur_acl != 480) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[ACL_STATUS_48P], ACL_PARAM_SIZE);
				lcd->cur_acl = 480;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 25: /* 280cd */
			if (lcd->cur_acl != 490) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[ACL_STATUS_49P], ACL_PARAM_SIZE);
				lcd->cur_acl = 490;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 26: /* 290cd */
			if (lcd->cur_acl != 550) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[ACL_STATUS_50P], ACL_PARAM_SIZE);
				lcd->cur_acl = 550;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		default: /* 290cd */
			if (lcd->cur_acl != 550) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[ACL_STATUS_50P], ACL_PARAM_SIZE);
				lcd->cur_acl = 550;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		}
	} else {
		s6e8ax0_write(lcd, SEQ_ACL_OFF, ARRAY_SIZE(SEQ_ACL_OFF));
		lcd->cur_acl = 0;
		dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d, acl_off\n", __func__, lcd->cur_acl);
	}

	if (ret) {
		ret = -EPERM;
		goto acl_err;
	}

acl_err:
	return ret;
}
#else
static int s6e8ax0_set_acl(struct lcd_info *lcd)
{
	if (lcd->acl_enable) {
		if (lcd->cur_acl == 0) {
			if (lcd->bl == 0 || lcd->bl == 1) {
				s6e8ax0_write(lcd, SEQ_ACL_OFF, ARRAY_SIZE(SEQ_ACL_OFF));
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d, acl_off\n", __func__, lcd->cur_acl);
			} else
				s6e8ax0_write(lcd, SEQ_ACL_ON, ARRAY_SIZE(SEQ_ACL_ON));
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d, acl_on\n", __func__, lcd->cur_acl);
		}
		switch (lcd->bl) {
		case 0 ... 1: /* 30cd ~ 40cd - 0%*/
			if (lcd->cur_acl != 0) {
				s6e8ax0_write(lcd, SEQ_ACL_OFF, ARRAY_SIZE(SEQ_ACL_OFF));
				lcd->cur_acl = 0;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 2: /* 50cd - 20%*/
			if (lcd->cur_acl != 20) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[1], ACL_PARAM_SIZE);
				lcd->cur_acl = 20;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 3: /* 60cd - 33%*/
			if (lcd->cur_acl != 33) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[2], ACL_PARAM_SIZE);
				lcd->cur_acl = 33;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case 4 ... 22: /*70cd ~ 250cd - 40%*/
			if (lcd->cur_acl != 40) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[3], ACL_PARAM_SIZE);
				lcd->cur_acl = 40;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		default:
			if (lcd->cur_acl != 50) {
				s6e8ax0_write(lcd, ACL_CUTOFF_TABLE[7], ACL_PARAM_SIZE);
				lcd->cur_acl = 50;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		}
	} else {
		s6e8ax0_write(lcd, SEQ_ACL_OFF, ARRAY_SIZE(SEQ_ACL_OFF));
		lcd->cur_acl = 0;
		dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d, acl_off\n", __func__, lcd->cur_acl);
	}

	return 0;
}
#endif

static int s6e8ax0_set_elvss(struct lcd_info *lcd)
{
	int ret = 0;
	u32 candela = candela_table[lcd->bl];

	switch (candela) {
	case 0 ... 100:
		ret = s6e8ax0_write(lcd, lcd->elvss_table[0], ELVSS_PARAM_SIZE);
		break;
	case 101 ... 160:
		ret = s6e8ax0_write(lcd, lcd->elvss_table[1], ELVSS_PARAM_SIZE);
		break;
	case 161 ... 200:
		ret = s6e8ax0_write(lcd, lcd->elvss_table[2], ELVSS_PARAM_SIZE);
		break;
	case 201 ... 300:
		ret = s6e8ax0_write(lcd, lcd->elvss_table[3], ELVSS_PARAM_SIZE);
		break;
	default:
		break;
	}

	dev_dbg(&lcd->ld->dev, "level  = %d\n", lcd->bl);

	if (ret) {
		ret = -EPERM;
		goto elvss_err;
	}

elvss_err:
	return ret;
}

#ifdef SMART_DIMMING
static u8 get_elvss_offset(u32 elvss_level)
{
	u8 offset = 0;

	switch (elvss_level) {
	case 0:
		offset = ELVSS_OFFSET_MIN;
		break;
	case 1:
		offset = ELVSS_OFFSET_1;
		break;
	case 2:
		offset = ELVSS_OFFSET_2;
		break;
	case 3:
		offset = ELVSS_OFFSET_MAX;
		break;
	default:
		offset = ELVSS_OFFSET_MAX;
		break;
	}
	return offset;
}

static u8 get_elvss_value(struct lcd_info *lcd, u8 elvss_level)
{
	u8 ref = 0;
	u8 offset;

	if (lcd->elvss.limit == 0x00)
		ref = (lcd->elvss.reference | 0x80);
	else if (lcd->elvss.limit == 0x01)
		ref = (lcd->elvss.reference + 0x40);
	else {
		printk(KERN_ERR "[ERROR:ELVSS]:%s undefined elvss limit value :%x\n", __func__, lcd->elvss.limit);
		return 0;
	}

	offset = get_elvss_offset(elvss_level);
	ref += offset;

	if (ref < DYNAMIC_ELVSS_MIN_VALUE)
		ref = DYNAMIC_ELVSS_MIN_VALUE;
	else if (ref > DYNAMIC_ELVSS_MAX_VALUE)
		ref = DYNAMIC_ELVSS_MAX_VALUE;

	return ref;
}

static int init_elvss_table(struct lcd_info *lcd)
{
	int i, ret = 0;

	lcd->elvss_table = kzalloc(ELVSS_STATUS_MAX * sizeof(u8 *), GFP_KERNEL);

	if (IS_ERR_OR_NULL(lcd->elvss_table)) {
		pr_err("failed to allocate elvss table\n");
		ret = -ENOMEM;
		goto err_alloc_elvss_table;
	}

	for (i = 0; i < ELVSS_STATUS_MAX; i++) {
		lcd->elvss_table[i] = kzalloc(ELVSS_PARAM_SIZE * sizeof(u8), GFP_KERNEL);
		if (IS_ERR_OR_NULL(lcd->elvss_table[i])) {
			pr_err("failed to allocate elvss\n");
			ret = -ENOMEM;
			goto err_alloc_elvss;
		}
		lcd->elvss_table[i][0] = 0xB1;
		lcd->elvss_table[i][1] = 0x04;
		lcd->elvss_table[i][2] = get_elvss_value(lcd, i);
	}

	return 0;

err_alloc_elvss:
	while (i > 0) {
		kfree(lcd->elvss_table[i-1]);
		i--;
	}
err_alloc_elvss_table:
	kfree(lcd->elvss_table);

	return ret;
}

static int init_gamma_table(struct lcd_info *lcd)
{
	int i, ret = 0;

	lcd->gamma_table = kzalloc(GAMMA_MAX * sizeof(u8 *), GFP_KERNEL);
	if (IS_ERR_OR_NULL(lcd->gamma_table)) {
		pr_err("failed to allocate gamma table\n");
		ret = -ENOMEM;
		goto err_alloc_gamma_table;
	}

	for (i = 0; i < GAMMA_MAX; i++) {
		lcd->gamma_table[i] = kzalloc(GAMMA_PARAM_SIZE * sizeof(u8), GFP_KERNEL);
		if (IS_ERR_OR_NULL(lcd->gamma_table[i])) {
			pr_err("failed to allocate gamma\n");
			ret = -ENOMEM;
			goto err_alloc_gamma;
		}
		lcd->gamma_table[i][0] = 0xFA;
		lcd->gamma_table[i][1] = 0x01;
		calc_gamma_table(&lcd->smart, candela_table[i]-1, lcd->gamma_table[i]+2);
	}
#if 0
	for (i = 0; i < GAMMA_MAX; i++) {
		for (j = 0; j < GAMMA_PARAM_SIZE; j++)
			printk("0x%02x, ", lcd->gamma_table[i][j]);
		printk("\n");
	}
#endif
	return 0;

err_alloc_gamma:
	while (i > 0) {
		kfree(lcd->gamma_table[i-1]);
		i--;
	}
err_alloc_gamma_table:
	kfree(lcd->gamma_table);

	return ret;
}
#endif

static int update_brightness(struct lcd_info *lcd, u8 force)
{
	int ret;
	u32 brightness;

	mutex_lock(&lcd->bl_lock);

	brightness = lcd->bd->props.brightness;

	lcd->bl = get_backlight_level_from_brightness(brightness);

	if ((force) || ((lcd->ldi_enable) && (lcd->current_bl != lcd->bl))) {

		ret = s6e8ax0_gamma_ctl(lcd);

		ret = s6e8ax0_set_acl(lcd);

		ret = s6e8ax0_set_elvss(lcd);

		lcd->current_bl = lcd->bl;

		dev_info(&lcd->ld->dev, "brightness=%d, bl=%d\n", brightness, lcd->bl);
	}

	mutex_unlock(&lcd->bl_lock);

	return 0;
}

static int s6e8ax0_ldi_init(struct lcd_info *lcd)
{
	int ret = 0;

#if defined(CONFIG_MACH_Q1_BD)
	s6e8ax0_write(lcd, SEQ_APPLY_LEVEL_2_KEY, ARRAY_SIZE(SEQ_APPLY_LEVEL_2_KEY));
	msleep(20);
	s6e8ax0_write(lcd, SEQ_SLEEP_OUT, ARRAY_SIZE(SEQ_SLEEP_OUT));
	msleep(5);
	s6e8ax0_write(lcd, SEQ_PANEL_CONDITION_SET, ARRAY_SIZE(SEQ_PANEL_CONDITION_SET));
	s6e8ax0_write(lcd, SEQ_DISPLAY_CONDITION_SET, ARRAY_SIZE(SEQ_DISPLAY_CONDITION_SET));
	s6e8ax0_gamma_ctl(lcd);
	s6e8ax0_write(lcd, SEQ_ETC_SOURCE_CONTROL, ARRAY_SIZE(SEQ_ETC_SOURCE_CONTROL));
	s6e8ax0_write(lcd, SEQ_ETC_PENTILE_CONTROL, ARRAY_SIZE(SEQ_ETC_PENTILE_CONTROL));
	s6e8ax0_write(lcd, SEQ_ETC_POWER_CONTROL, ARRAY_SIZE(SEQ_ETC_POWER_CONTROL));
	s6e8ax0_write(lcd, SEQ_ELVSS_NVM_SETTING, ARRAY_SIZE(SEQ_ELVSS_NVM_SETTING));
	s6e8ax0_write(lcd, SEQ_ELVSS_CONTROL, ARRAY_SIZE(SEQ_ELVSS_CONTROL));
#else
	s6e8ax0_write(lcd, SEQ_APPLY_LEVEL_2,\
	ARRAY_SIZE(SEQ_APPLY_LEVEL_2));
	s6e8ax0_write(lcd, SEQ_APPLY_MTP_KEY_ENABLE,\
		ARRAY_SIZE(SEQ_APPLY_MTP_KEY_ENABLE));
	s6e8ax0_write(lcd, SEQ_SLEEP_OUT, \
		ARRAY_SIZE(SEQ_SLEEP_OUT));
	msleep(5);
	if (lcd->id[1] == 0xae)
		s6e8ax0_write(lcd, SEQ_PANEL_CONDITION_SET_480MBPS_46,\
		ARRAY_SIZE(SEQ_PANEL_CONDITION_SET_480MBPS_46));
	else if (lcd->id[1] == 0x20) /* 4.8" HD for M0/C1*/
		s6e8ax0_write(lcd, SEQ_PANEL_CONDITION_SET_500MBPS,\
		ARRAY_SIZE(SEQ_PANEL_CONDITION_SET_500MBPS));
	else
		s6e8ax0_write(lcd, SEQ_PANEL_CONDITION_SET_500MBPS_46,\
		ARRAY_SIZE(SEQ_PANEL_CONDITION_SET_500MBPS_46));
	s6e8ax0_write(lcd, SEQ_DISPLAY_CONDITION_SET, \
		ARRAY_SIZE(SEQ_DISPLAY_CONDITION_SET));
	s6e8ax0_gamma_ctl(lcd);
	s6e8ax0_write(lcd, SEQ_ETC_SOURCE_CONTROL, \
		ARRAY_SIZE(SEQ_ETC_SOURCE_CONTROL));
	if (lcd->id[1] == 0x20) { /* 4.8" HD for M0/C1*/
		s6e8ax0_write(lcd, SEQ_ETC_PENTILE_CONTROL, \
			ARRAY_SIZE(SEQ_ETC_PENTILE_CONTROL));
		s6e8ax0_write(lcd, SEQ_ETC_NVM_SETTING, \
			ARRAY_SIZE(SEQ_ETC_NVM_SETTING));
		s6e8ax0_write(lcd, SEQ_ETC_POWER_CONTROL, \
			ARRAY_SIZE(SEQ_ETC_POWER_CONTROL));
	} else {
		s6e8ax0_write(lcd, SEQ_ETC_PENTILE_CONTROL_46, \
			ARRAY_SIZE(SEQ_ETC_PENTILE_CONTROL_46));
		s6e8ax0_write(lcd, SEQ_ETC_NVM_SETTING_46, \
			ARRAY_SIZE(SEQ_ETC_NVM_SETTING_46));
		s6e8ax0_write(lcd, SEQ_ETC_POWER_CONTROL_46, \
			ARRAY_SIZE(SEQ_ETC_POWER_CONTROL_46));
	}
	s6e8ax0_write(lcd, SEQ_ELVSS_CONTROL,\
		ARRAY_SIZE(SEQ_ELVSS_CONTROL));
#endif

	return ret;
}

static int s6e8ax0_ldi_enable(struct lcd_info *lcd)
{
	int ret = 0;

	s6e8ax0_write(lcd, SEQ_DISPLAY_ON, ARRAY_SIZE(SEQ_DISPLAY_ON));

	return ret;
}

static int s6e8ax0_ldi_disable(struct lcd_info *lcd)
{
	int ret = 0;

	s6e8ax0_write(lcd, SEQ_DISPLAY_OFF, ARRAY_SIZE(SEQ_DISPLAY_OFF));
	s6e8ax0_write(lcd, SEQ_STANDBY_ON, ARRAY_SIZE(SEQ_STANDBY_ON));

	return ret;
}

static int s6e8ax0_power_on(struct lcd_info *lcd)
{
	int ret = 0;
	struct lcd_platform_data *pd = NULL;
	pd = lcd->lcd_pd;

	/* dev_info(&lcd->ld->dev, "%s\n", __func__); */

	ret = s6e8ax0_ldi_init(lcd);
	if (ret) {
		dev_err(&lcd->ld->dev, "failed to initialize ldi.\n");
		goto err;
	}

	msleep(120);

	ret = s6e8ax0_ldi_enable(lcd);
	if (ret) {
		dev_err(&lcd->ld->dev, "failed to enable ldi.\n");
		goto err;
	}

	lcd->ldi_enable = 1;

	update_brightness(lcd, 1);
err:
	return ret;
}

static int s6e8ax0_power_off(struct lcd_info *lcd)
{
	int ret = 0;

	dev_info(&lcd->ld->dev, "%s\n", __func__);

	lcd->ldi_enable = 0;

	ret = s6e8ax0_ldi_disable(lcd);

	msleep(120);

	return ret;
}

static int s6e8ax0_power(struct lcd_info *lcd, int power)
{
	int ret = 0;

	if (POWER_IS_ON(power) && !POWER_IS_ON(lcd->power))
		ret = s6e8ax0_power_on(lcd);
	else if (!POWER_IS_ON(power) && POWER_IS_ON(lcd->power))
		ret = s6e8ax0_power_off(lcd);

	if (!ret)
		lcd->power = power;

	return ret;
}

static int s6e8ax0_set_power(struct lcd_device *ld, int power)
{
	struct lcd_info *lcd = lcd_get_data(ld);

	if (power != FB_BLANK_UNBLANK && power != FB_BLANK_POWERDOWN &&
		power != FB_BLANK_NORMAL) {
		dev_err(&lcd->ld->dev, "power value should be 0, 1 or 4.\n");
		return -EINVAL;
	}

	return s6e8ax0_power(lcd, power);
}

static int s6e8ax0_get_power(struct lcd_device *ld)
{
	struct lcd_info *lcd = lcd_get_data(ld);

	return lcd->power;
}

static int s6e8ax0_set_brightness(struct backlight_device *bd)
{
	int ret = 0;
	int brightness = bd->props.brightness;
	struct lcd_info *lcd = bl_get_data(bd);

	/* dev_info(&lcd->ld->dev, "%s: brightness=%d\n", __func__, brightness); */

	if (brightness < MIN_BRIGHTNESS ||
		brightness > bd->props.max_brightness) {
		dev_err(&bd->dev, "lcd brightness should be %d to %d. now %d\n",
			MIN_BRIGHTNESS, MAX_BRIGHTNESS, brightness);
		return -EINVAL;
	}

	if (lcd->ldi_enable) {
		ret = update_brightness(lcd, 0);
		if (ret < 0) {
			dev_err(lcd->dev, "err in %s\n", __func__);
			return -EINVAL;
		}
	}

	return ret;
}

static int s6e8ax0_get_brightness(struct backlight_device *bd)
{
	return bd->props.brightness;
}

static struct lcd_ops s6e8ax0_lcd_ops = {
	.set_power = s6e8ax0_set_power,
	.get_power = s6e8ax0_get_power,
};

static const struct backlight_ops s6e8ax0_backlight_ops  = {
	.get_brightness = s6e8ax0_get_brightness,
	.update_status = s6e8ax0_set_brightness,
};

static ssize_t lcd_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char temp[15];
#if defined(CONFIG_MACH_Q1_BD)
	sprintf(temp, "SMD_AMS529HA01\n");
#else
	sprintf(temp, "SMD_AMS480GYXX\n");
#endif
	strcat(buf, temp);
	return strlen(buf);
}

static DEVICE_ATTR(lcd_type, 0444, lcd_type_show, NULL);

static ssize_t power_reduce_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	char temp[3];

	sprintf(temp, "%d\n", lcd->acl_enable);
	strcpy(buf, temp);

	return strlen(buf);
}

static ssize_t power_reduce_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int value;
	int rc;

	rc = strict_strtoul(buf, (unsigned int)0, (unsigned long *)&value);
	if (rc < 0)
		return rc;
	else {
		if (lcd->acl_enable != value) {
			dev_info(dev, "%s - %d, %d\n", __func__, lcd->acl_enable, value);
			lcd->acl_enable = value;
			if (lcd->ldi_enable)
				s6e8ax0_set_acl(lcd);
		}
	}
	return size;
}

static DEVICE_ATTR(power_reduce, 0664, power_reduce_show, power_reduce_store);

static ssize_t gamma_table_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int i, j;

	for (i = 0; i < GAMMA_MAX; i++) {
		for (j = 0; j < GAMMA_PARAM_SIZE; j++)
			printk("0x%02x, ", lcd->gamma_table[i][j]);
		printk("\n");
	}

	return strlen(buf);
}
static DEVICE_ATTR(gamma_table, 0444, gamma_table_show, NULL);

#ifdef CONFIG_HAS_EARLYSUSPEND
struct lcd_info *g_lcd;

void s6e8ax0_early_suspend(void)
{
	struct lcd_info *lcd = g_lcd;

	dev_info(&lcd->ld->dev, "+%s\n", __func__);
#if defined(GPIO_OLED_DET)
	disable_irq(lcd->irq);
	gpio_request(GPIO_OLED_DET, "OLED_DET");
	s3c_gpio_cfgpin(GPIO_OLED_DET, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_OLED_DET, S3C_GPIO_PULL_NONE);
	gpio_direction_output(GPIO_OLED_DET, GPIO_LEVEL_LOW);
	gpio_free(GPIO_OLED_DET);
#endif
	s6e8ax0_power(lcd, FB_BLANK_POWERDOWN);
	dev_info(&lcd->ld->dev, "-%s\n", __func__);

	return ;
}

void s6e8ax0_late_resume(void)
{
	struct lcd_info *lcd = g_lcd;

	dev_info(&lcd->ld->dev, "+%s\n", __func__);
	s6e8ax0_power(lcd, FB_BLANK_UNBLANK);
#if defined(GPIO_OLED_DET)
	s3c_gpio_cfgpin(GPIO_OLED_DET, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(GPIO_OLED_DET, S3C_GPIO_PULL_NONE);
	enable_irq(lcd->irq);
#endif
	dev_info(&lcd->ld->dev, "-%s\n", __func__);

	set_dsim_lcd_enabled();

	return ;
}
#endif

static void s6e8ax0_read_id(struct lcd_info *lcd, u8 *buf)
{
	int ret = 0;

	ret = s6e8ax0_read(lcd, LDI_ID_REG, LDI_ID_LEN, buf, 3);
	if (!ret) {
		lcd->connected = 0;
		dev_info(&lcd->ld->dev, "panel is not connected well\n");
	}
}

#ifdef SMART_DIMMING
static int s6e8ax0_read_mtp(struct lcd_info *lcd, u8 *mtp_data)
{
	int ret;

	ret = s6e8ax0_read(lcd, LDI_MTP_ADDR, LDI_MTP_LENGTH, mtp_data, 0);

	return ret;
}

#if defined(CONFIG_MACH_Q1_BD)
static void s6e8aa0_check_id(struct lcd_info *lcd, u8 *idbuf)
{
	u32 i;

	if (idbuf[2] == 0x33) {
		lcd->support_elvss = 0;
		printk(KERN_INFO "ID-3 is 0xff does not support dynamic elvss\n");
	} else {
		lcd->support_elvss = 1;
		lcd->elvss.limit = (idbuf[2] & 0xc0) >> 6;
		lcd->elvss.reference = idbuf[2] & 0x3f;
		printk(KERN_INFO "ID-3 is 0x%x support dynamic elvss\n", idbuf[2]);
		printk(KERN_INFO "Dynamic ELVSS Information\n");
		printk(KERN_INFO "limit    : %02x\n", lcd->elvss.limit);
	}

	for (i = 0; i < LDI_ID_LEN; i++)
		lcd->smart.panelid[i] = idbuf[i];
}
#else
static void s6e8aa0_check_id(struct lcd_info *lcd, u8 *idbuf)
{
	int i;

	for (i = 0; i < LDI_ID_LEN; i++)
		lcd->smart.panelid[i] = idbuf[i];

	lcd->aid = lcd->smart.panelid[2] & 0xe0 >> 5;

#if defined(CONFIG_MACH_C1CTC) || defined(CONFIG_MACH_M0_CHNOPEN) ||\
	defined(CONFIG_MACH_M0_CMCC)
	lcdtype = lcd->smart.panelid[1];
#endif

	if (idbuf[0] == PANEL_A1_SM2) {
		lcd->support_elvss = 1;
		lcd->aid = lcd->smart.panelid[2] & 0xe0 >> 5;
		lcd->elvss.reference = lcd->smart.panelid[2] & 0x3f;

		printk(KERN_DEBUG "Dynamic ELVSS Information\n");
		printk(KERN_DEBUG "Refrence : %02x , lcd->aid= %02x\n", lcd->elvss.reference, lcd->aid);
	} else if ((idbuf[0] == PANEL_A1_M3) || (idbuf[0] == PANEL_A2_M3)) {
		lcd->support_elvss = 0;
		printk(KERN_DEBUG "ID-3 is 0xff does not support dynamic elvss\n");
	} else {
		lcd->support_elvss = 0;
		printk(KERN_DEBUG "No valid panel id\n");
	}
}
#endif
#endif

static int s6e8ax0_probe(struct device *dev)
{
	int ret = 0;
	struct lcd_info *lcd;
#ifdef SMART_DIMMING
	u8 mtp_data[LDI_MTP_LENGTH] = {0,};
#endif

	lcd = kzalloc(sizeof(struct lcd_info), GFP_KERNEL);
	if (!lcd) {
		pr_err("failed to allocate for lcd\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	g_lcd = lcd;

	lcd->ld = lcd_device_register("panel", dev, lcd, &s6e8ax0_lcd_ops);
	if (IS_ERR(lcd->ld)) {
		pr_err("failed to register lcd device\n");
		ret = PTR_ERR(lcd->ld);
		goto out_free_lcd;
	}

	lcd->bd = backlight_device_register("panel", dev, lcd, &s6e8ax0_backlight_ops, NULL);
	if (IS_ERR(lcd->bd)) {
		pr_err("failed to register backlight device\n");
		ret = PTR_ERR(lcd->bd);
		goto out_free_backlight;
	}

	lcd->dev = dev;
	lcd->bd->props.max_brightness = MAX_BRIGHTNESS;
	lcd->bd->props.brightness = DEFAULT_BRIGHTNESS;
	lcd->bl = DEFAULT_GAMMA_LEVEL;
	lcd->current_bl = lcd->bl;

	lcd->acl_enable = 0;
	lcd->cur_acl = 0;

	lcd->power = FB_BLANK_UNBLANK;
	lcd->ldi_enable = 1;
	lcd->connected = 1;

	ret = device_create_file(&lcd->ld->dev, &dev_attr_power_reduce);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&lcd->ld->dev, &dev_attr_lcd_type);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	ret = device_create_file(&lcd->ld->dev, &dev_attr_gamma_table);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries, %d\n", __LINE__);

	dev_set_drvdata(dev, lcd);

	mutex_init(&lcd->lock);
	mutex_init(&lcd->bl_lock);

	s6e8ax0_read_id(lcd, lcd->id);

	dev_info(&lcd->ld->dev, "ID: %x, %x, %x\n", lcd->id[0], lcd->id[1], lcd->id[2]);

	dev_info(&lcd->ld->dev, "s6e8aa0 lcd panel driver has been probed.\n");

	lcd->gamma_table = (unsigned char **)gamma22_table_l;
	lcd->elvss_table = (unsigned char **)ELVSS_TABLE;

#ifdef SMART_DIMMING
	s6e8aa0_check_id(lcd, lcd->id);

	init_table_info(&lcd->smart);

	ret = s6e8ax0_read_mtp(lcd, mtp_data);
	if (!ret) {
		printk(KERN_ERR "[LCD:ERROR] : %s read mtp failed\n", __func__);
		/*return -EPERM;*/
	}

	calc_voltage_table(&lcd->smart, mtp_data);

	if (lcd->connected) {
		ret = init_gamma_table(lcd);
		ret += init_elvss_table(lcd);

		if (ret) {
			lcd->gamma_table = (unsigned char **)gamma22_table_l;
			lcd->elvss_table = (unsigned char **)ELVSS_TABLE;
		}
	}

	update_brightness(lcd, 1);
#endif

#if defined(GPIO_OLED_DET)
	if (lcd->connected) {
		INIT_DELAYED_WORK(&hs_clk_re_try, hs_clk_re_try_work);

		lcd->irq = gpio_to_irq(GPIO_OLED_DET);

		s3c_gpio_cfgpin(GPIO_OLED_DET, S3C_GPIO_SFN(0xf));
		s3c_gpio_setpull(GPIO_OLED_DET, S3C_GPIO_PULL_NONE);
		if (request_irq(lcd->irq, oled_det_int,
			IRQF_TRIGGER_FALLING, "esd_detection", 0))
			pr_err("failed to reqeust irq. %d\n", lcd->irq);
	}
#endif
	return 0;

out_free_backlight:
	lcd_device_unregister(lcd->ld);
	kfree(lcd);
	return ret;

out_free_lcd:
	kfree(lcd);
	return ret;

err_alloc:
	return ret;
}

static int __devexit s6e8ax0_remove(struct device *dev)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	s6e8ax0_power(lcd, FB_BLANK_POWERDOWN);
	lcd_device_unregister(lcd->ld);
	backlight_device_unregister(lcd->bd);
	kfree(lcd);

	return 0;
}

/* Power down all displays on reboot, poweroff or halt. */
static void s6e8ax0_shutdown(struct device *dev)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);

	dev_info(&lcd->ld->dev, "%s\n", __func__);

	s6e8ax0_power(lcd, FB_BLANK_POWERDOWN);
}

static struct mipi_lcd_driver s6e8ax0_mipi_driver = {
	.name = "s6e8aa0",
	.set_link		= s6e8ax0_set_link,
	.probe			= s6e8ax0_probe,
	.remove			= __devexit_p(s6e8ax0_remove),
	.shutdown		= s6e8ax0_shutdown,
};

static int s6e8ax0_init(void)
{
	return s5p_dsim_register_lcd_driver(&s6e8ax0_mipi_driver);
}

static void s6e8ax0_exit(void)
{
	return;
}

module_init(s6e8ax0_init);
module_exit(s6e8ax0_exit);

MODULE_DESCRIPTION("MIPI-DSI S6E8AA0:AMS529HA01 (800x1280) Panel Driver");
MODULE_LICENSE("GPL");
