/*
 * LD9040 AMOLED LCD panel driver.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <linux/wait.h>
#include <linux/fb.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/lcd.h>
#include <linux/backlight.h>
#include <linux/ld9040.h>

#include <linux/device.h> 
#include <linux/miscdevice.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#define SLEEPMSEC		0x1000
#define ENDDEF			0x2000
#define	DEFMASK		0xFF00
#define COMMAND_ONLY		0xFE
#define DATA_ONLY		0xFF

#define MIN_BRIGHTNESS	0
#define MAX_BRIGHTNESS	255
#define POWER_IS_ON(pwr)	((pwr) <= FB_BLANK_NORMAL)

/* #define SMART_DIMMING */

#ifdef SMART_DIMMING
#include "ld9042_panel.h"
#include "smart_dimming_ld9042.h"

#define spidelay(nsecs)	do {} while (0)

#define GAMMA_PARAM_LEN	21

#define LD9040_ID3		0x11
#define LDI_ID_REG		0xDA
#define LDI_ID_LEN		3
#define LDI_MTP_REG		0xD6
#define LDI_MTP_LEN		18

#define ELVSS_OFFSET_MIN	0x0D
#define ELVSS_OFFSET_1	0x0C
#define ELVSS_OFFSET_2	0x09
#define ELVSS_OFFSET_MAX	0x00
#define ELVSS_LIMIT		0x29
#endif

#define MIN_GAMMA_ADJUST -50
#define MAX_GAMMA_ADJUST  50
#define GAMMA_DATA_SIZE   (48*sizeof(unsigned short))

extern struct ld9040_panel_data u1_panel_data;
extern struct ld9040_panel_data u1_panel_data_a2;
extern struct ld9040_panel_data u1_panel_data_m2;

struct lcd_info  {
	struct device			*dev;
	struct spi_device		*spi;
	unsigned int			power;
	unsigned int			gamma_mode;
	unsigned int			current_gamma_mode;
	unsigned int			current_bl;
	unsigned int			bl;
	unsigned int			auto_brightness;
	unsigned int			ldi_enable;
	unsigned int			acl_enable;
	unsigned int			cur_acl;
	unsigned int			user_lcdtype;
	signed int			user_gamma_adjust;
	signed int			current_user_gamma_adjust;
	struct mutex			lock;
	struct mutex			bl_lock;
	struct lcd_device		*ld;
	struct backlight_device		*bd;
	struct lcd_platform_data	*lcd_pd;
	struct early_suspend		early_suspend;

#ifdef SMART_DIMMING
	unsigned char			id[3];
	struct str_smart_dim		smart;
#endif
};

enum {
	GAMMA_30CD,
	GAMMA_40CD,
	GAMMA_70CD,
	GAMMA_90CD,
	GAMMA_100CD,
	GAMMA_110CD,
	GAMMA_120CD,
	GAMMA_130CD,
	GAMMA_140CD,
	GAMMA_150CD,
	GAMMA_160CD,
	GAMMA_170CD,
	GAMMA_180CD,
	GAMMA_190CD,
	GAMMA_200CD,
	GAMMA_210CD,
	GAMMA_220CD,
	GAMMA_230CD,
	GAMMA_240CD,
	GAMMA_250CD,
	GAMMA_300CD,
	GAMMA_MAX
};

static const unsigned int candela_table[GAMMA_MAX] = {
	 30,  40,  70,  90, 100, 110, 120, 130, 140, 150,
	160, 170, 180, 190, 200, 210, 220, 230, 240, 250,
	300
};

static signed short gamma_adjust_map[] = { 3, 3, 3, 3, 0, 1, 0, 1, 5, 2, 4, 0, 1, 0, 3, 3, 3, 2, 0, 1 };

static int ld9040_spi_write_byte(struct lcd_info *lcd, int addr, int data)
{
	u16 buf[1];
	struct spi_message msg;

	struct spi_transfer xfer = {
		.len		= 2,
		.tx_buf		= buf,
	};

	buf[0] = (addr << 8) | data;

	spi_message_init(&msg);
	spi_message_add_tail(&xfer, &msg);

	return spi_sync(lcd->spi, &msg);
}

#ifdef SMART_DIMMING
static int ld9040_spi_read(struct lcd_info *lcd, unsigned int addr,
	unsigned char *buf, unsigned int len, unsigned int dummy_bit)
{
	struct ld9040_panel_data *pdata = lcd->lcd_pd->pdata;
	struct spi_ops *ops = pdata->ops;
	unsigned int bits;

	int i;
	int j;

	bits = lcd->spi->bits_per_word - 1;

	mutex_lock(&lcd->lock);

	ops->setcs(0);
	spidelay(0);

	for (j = bits; j >= 0; j--) {
		ops->setsck(0);
		spidelay(0);

		ops->setmosi((addr >> j) & 1);
		spidelay(0);

		ops->setsck(1);
		spidelay(0);
	}

	ops->setmosi2miso(1);	/* SDI as input */
	spidelay(0);

	for (j = 0; j < dummy_bit; j++) {
		ops->setsck(0);
		spidelay(0);

		ops->setsck(1);
		spidelay(0);

		ops->getmiso();
		spidelay(0);
	}

	for (i = 0; i < len; i++) {
		for (j = bits - 1; j >= 0; j--) {
			ops->setsck(0);
			spidelay(0);

			ops->setsck(1);
			spidelay(0);

			buf[i] |= (unsigned char)(ops->getmiso() << j);
			spidelay(0);
		}
		/* printk(KERN_INFO "0x%x, %d\n", buf[i], buf[i]); */
	}

	ops->setcs(1);
	spidelay(0);

	ops->setmosi2miso(0);	/* SDI as output */
	spidelay(0);

	mutex_unlock(&lcd->lock);

	return 0;
}

static int ld9040_read_id(struct lcd_info *lcd, unsigned int addr)
{
	unsigned char buf[1] = {0};

	ld9040_spi_read(lcd, addr, buf, 1, 8);

	return *buf;
}

static int spi_read_multi_byte(struct lcd_info *lcd,
	unsigned int addr, unsigned char *buf, unsigned int len)
{
	if (len == 1)
		ld9040_spi_read(lcd, addr, buf, len, 8);
	else
		ld9040_spi_read(lcd, addr, buf, len, 1);

	return 0;
}

static void ld9042_init_smart_dimming_table_22(struct lcd_info *lcd)
{
	unsigned int i, j;
	unsigned char gamma_22[GAMMA_PARAM_LEN] = {0,};

	for (i = 0; i < GAMMA_MAX; i++) {
		calc_gamma_table_22(&lcd->smart, candela_table[i], gamma_22);
		for (j = 0; j < GAMMA_PARAM_LEN; j++)
			ld9042_22gamma_table[i][j*2+1] = gamma_22[j];
	}
}

static void ld9042_init_smart_dimming_table_19(struct lcd_info *lcd)
{
	unsigned int i, j;
	unsigned char gamma_19[GAMMA_PARAM_LEN] = {0,};

	for (i = 0; i < GAMMA_MAX; i++) {
		calc_gamma_table_19(&lcd->smart, candela_table[i], gamma_19);
		for (j = 0; j < GAMMA_PARAM_LEN; j++)
			ld9042_19gamma_table[i][j*2+1] = gamma_19[j];
	}
}

static void ld9042_init_smart_elvss_table(struct lcd_info *lcd)
{
	unsigned int i, j;
	unsigned char elvss, b2;

	elvss = lcd->id[2] & (~(BIT(6) | BIT(7)));

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			b2 = elvss + ELVSS_TABLE[i][j*2+1];
			b2 = (b2 > ELVSS_LIMIT) ? ELVSS_LIMIT : b2;
			ELVSS_TABLE[i][j*2+1] = b2;
		}
	}

}
#endif

static int ld9040_spi_write(struct lcd_info *lcd,
	unsigned char address, unsigned char command)
{
	int ret = 0;

	if (address != DATA_ONLY)
		ret = ld9040_spi_write_byte(lcd, 0x0, address);
	if (command != COMMAND_ONLY)
		ret = ld9040_spi_write_byte(lcd, 0x1, command);

	return ret;
}

static int ld9040_panel_send_sequence(struct lcd_info *lcd,
	const unsigned short *seq)
{
	int ret = 0, i = 0;
	const unsigned short *wbuf;

	mutex_lock(&lcd->lock);

	wbuf = seq;

	while ((wbuf[i] & DEFMASK) != ENDDEF) {
		if ((wbuf[i] & DEFMASK) != SLEEPMSEC) {
			ret = ld9040_spi_write(lcd, wbuf[i], wbuf[i+1]);
			if (ret)
				break;
		} else if ((wbuf[i] & DEFMASK) == SLEEPMSEC)
			msleep(wbuf[i+1]);
		i += 2;
	}

	mutex_unlock(&lcd->lock);

	return ret;
}

#define MAX_BL 255
#define MAX_GAMMA 20

int min_gamma = 0, max_gamma = 20, min_bl = 40;
static int get_backlight_level_from_brightness(unsigned int bl)
{
	int gamma_value =0;
	int gamma_val_x10 =0;

	if(bl >= min_bl){
		gamma_val_x10 = 10 *(max_gamma-1-min_gamma)*bl/(MAX_BL-min_bl) 
		    + (10 - 10*(max_gamma-1-min_gamma)*(min_bl)/(MAX_BL-min_bl));
		gamma_value=(gamma_val_x10 +5)/10 + min_gamma;
	}else{
		gamma_value =min_gamma;
	}
	if(gamma_value > MAX_GAMMA) gamma_value = MAX_GAMMA;
	else if(gamma_value < min_gamma) gamma_value = min_gamma;
	return gamma_value;
}

#define declare_show(filename) \
	static ssize_t show_##filename(struct device *dev, struct device_attribute *attr, char *buf)

#define declare_store(filename) \
	static ssize_t store_##filename(\
		struct device *dev, struct device_attribute *attr, const char *buf, size_t size)

declare_show(author) {
	return sprintf(buf, "Siyah\n");
}

declare_show(min_gamma) {
	return sprintf(buf, "%d\n", min_gamma);
}
declare_show(max_gamma) {
	return sprintf(buf, "%d\n", max_gamma);
}
declare_show(min_bl) {
	return sprintf(buf, "%d\n", min_bl);
}

declare_store(min_gamma) {	
	int val;
	if(sscanf(buf,"%d",&val)==1) {
		if(val>23) val=23;
		else if(val<0) val=0;
		min_gamma = val;
	}
	return size;
}
declare_store(max_gamma) {	
	int val;
	if(sscanf(buf,"%d",&val)==1) {
		if(val>24) val=24;
		else if(val<0) val=0;
		max_gamma = val;
	}
	return size;
}
declare_store(min_bl) {	
	int val;
	if(sscanf(buf,"%d",&val)==1) {
		if(val>200) val=200;
		else if(val<0) val=0;
		min_bl = val;
	}
	return size;
}

#define declare_attr_rw(filename, perm) \
	static DEVICE_ATTR(filename, perm, show_##filename, store_##filename)
#define declare_attr_ro(filename, perm) \
	static DEVICE_ATTR(filename, perm, show_##filename, NULL)
#define declare_attr_wo(filename, perm) \
	static DEVICE_ATTR(filename, perm, NULL, store_##filename)

declare_attr_ro(author, 0444);
declare_attr_rw(min_gamma, 0666);
declare_attr_rw(max_gamma, 0666);
declare_attr_rw(min_bl, 0666);

static struct attribute *brightness_curve_attributes[] = {
	&dev_attr_min_gamma.attr,
	&dev_attr_max_gamma.attr,
	&dev_attr_min_bl.attr,
	&dev_attr_author.attr,
	NULL
};

static struct attribute_group brightness_curve_group = {
		.attrs  = brightness_curve_attributes,
};

static struct miscdevice brightness_curve_device = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = "brightness_curve",
};
static int ld9040_gamma_ctl(struct lcd_info *lcd)
{
	int ret = 0, i;
	const unsigned short *gamma;
	unsigned short gamma_adjust[100];
	struct ld9040_panel_data *pdata = lcd->lcd_pd->pdata;

	if (lcd->gamma_mode)
		gamma = pdata->gamma19_table[lcd->bl];
	else
		gamma = pdata->gamma22_table[lcd->bl];

	if (lcd->user_gamma_adjust != 0) {
		//printk("*** : debug gamma size=%d, *gamma_adjust_map size=%d\n", GAMMA_DATA_SIZE, ARRAY_SIZE(gamma_adjust_map));
		memcpy(gamma_adjust, gamma, GAMMA_DATA_SIZE);
		for (i = 0; i < ARRAY_SIZE(gamma_adjust_map); i++) {
			if (gamma_adjust_map[i] != 0) {
				signed short value = gamma_adjust[(i+1)*2+1];
				value += lcd->user_gamma_adjust / gamma_adjust_map[i];
				if (value > 0xFF) value = 0xFF;
				else if (value < 0) value = 0x00;
				gamma_adjust[(i+1)*2+1] = (unsigned short)value;
			}
		}
		ret = ld9040_panel_send_sequence(lcd, gamma_adjust);
	} else {
		ret = ld9040_panel_send_sequence(lcd, gamma);
	}

	if (ret) {
		ret = -EPERM;
		goto gamma_err;
	}

	lcd->current_gamma_mode = lcd->gamma_mode;
gamma_err:
	return ret;
}

static int ld9040_set_elvss(struct lcd_info *lcd)
{
	int ret = 0;
	struct ld9040_panel_data *pdata = lcd->lcd_pd->pdata;

	if (lcd->acl_enable) {
		switch (lcd->bl) {
		case GAMMA_30CD ... GAMMA_200CD: /* 30cd ~ 200cd */
			ret = ld9040_panel_send_sequence(lcd, pdata->elvss_table[0]);
			break;
		case GAMMA_210CD ... GAMMA_300CD: /* 210cd ~ 300cd */
			ret = ld9040_panel_send_sequence(lcd, pdata->elvss_table[1]);
			break;
		default:
			break;
		}
	} else {
		switch (lcd->bl) {
		case GAMMA_30CD ... GAMMA_100CD: /* 30cd ~ 100cd */
			ret = ld9040_panel_send_sequence(lcd, pdata->elvss_table[0]);
			break;
		case GAMMA_110CD ... GAMMA_160CD: /* 110cd ~ 160cd */
			ret = ld9040_panel_send_sequence(lcd, pdata->elvss_table[1]);
			break;
		case GAMMA_170CD ... GAMMA_200CD: /* 170cd ~ 200cd */
			ret = ld9040_panel_send_sequence(lcd, pdata->elvss_table[2]);
			break;
		case GAMMA_210CD ... GAMMA_300CD: /* 210cd ~ 300cd */
			ret = ld9040_panel_send_sequence(lcd, pdata->elvss_table[3]);
			break;
		default:
			break;
		}
	}

	dev_dbg(&lcd->ld->dev, "level  = %d\n", lcd->bl);

	if (ret) {
		ret = -EPERM;
		goto elvss_err;
	}

elvss_err:
	return ret;
}

static int ld9040_set_acl(struct lcd_info *lcd)
{
	int ret = 0;
	struct ld9040_panel_data *pdata = lcd->lcd_pd->pdata;

	if (lcd->acl_enable) {
		if (lcd->cur_acl == 0) {
			if (lcd->bl == 0 || lcd->bl == 1) {
				ret = ld9040_panel_send_sequence(lcd, pdata->acl_table[0]);
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d, acl_off\n", __func__, lcd->cur_acl);
			} else {
				ret = ld9040_panel_send_sequence(lcd, pdata->acl_on);
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d, acl_on\n", __func__, lcd->cur_acl);
			}
		}
		switch (lcd->bl) {
		case GAMMA_30CD ... GAMMA_40CD: /* 30cd ~ 40cd */
			if (lcd->cur_acl != 0) {
				ret = ld9040_panel_send_sequence(lcd, pdata->acl_table[0]);
				lcd->cur_acl = 0;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		case GAMMA_70CD ... GAMMA_250CD: /* 70cd ~ 250cd */
			if (lcd->cur_acl != 40) {
				ret = ld9040_panel_send_sequence(lcd, pdata->acl_table[1]);
				lcd->cur_acl = 40;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		default:
			if (lcd->cur_acl != 50) {
				ret = ld9040_panel_send_sequence(lcd, pdata->acl_table[2]);
				lcd->cur_acl = 50;
				dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
			}
			break;
		}
	} else {
		ret = ld9040_panel_send_sequence(lcd, pdata->acl_table[0]);
		lcd->cur_acl = 0;
		dev_dbg(&lcd->ld->dev, "%s : cur_acl=%d\n", __func__, lcd->cur_acl);
	}

	if (ret) {
		ret = -EPERM;
		goto acl_err;
	}

acl_err:
	return ret;
}

static int ld9040_ldi_init(struct lcd_info *lcd)
{
	int ret, i;
	struct ld9040_panel_data *pdata = lcd->lcd_pd->pdata;
	const unsigned short *init_seq[] = {
		pdata->seq_user_set,
		pdata->seq_displayctl_set,
		pdata->seq_gtcon_set,
		pdata->seq_panelcondition_set,
		pdata->acl_table[0],
		pdata->sleep_out,
		pdata->elvss_on,
		pdata->seq_pwrctl_set,
		pdata->gamma22_table[0],
	};
	for (i = 0; i < ARRAY_SIZE(init_seq); i++) {
		ret = ld9040_panel_send_sequence(lcd, init_seq[i]);
		if (ret)
			break;
	}

	return ret;
}

static int ld9040_ldi_enable(struct lcd_info *lcd)
{
	int ret = 0;
	struct ld9040_panel_data *pdata = lcd->lcd_pd->pdata;

	ret = ld9040_panel_send_sequence(lcd, pdata->display_on);

	return ret;
}

static int ld9040_ldi_disable(struct lcd_info *lcd)
{
	int ret;
	struct ld9040_panel_data *pdata = lcd->lcd_pd->pdata;

	lcd->ldi_enable = 0;
	ret = ld9040_panel_send_sequence(lcd, pdata->display_off);
	ret = ld9040_panel_send_sequence(lcd, pdata->sleep_in);

	return ret;
}

static int update_brightness(struct lcd_info *lcd, u8 force)
{
	int ret = 0, brightness;
	struct ld9040_panel_data *pdata = lcd->lcd_pd->pdata;

	mutex_lock(&lcd->bl_lock);

	brightness = lcd->bd->props.brightness;

	if (unlikely(!lcd->auto_brightness && brightness > 250))
		brightness = 250;

	lcd->bl = get_backlight_level_from_brightness(brightness);

	if ((force) || ((lcd->ldi_enable) && (lcd->current_bl != lcd->bl))) {

		ret = ld9040_gamma_ctl(lcd);

		ret |= ld9040_set_acl(lcd);

		ret |= ld9040_set_elvss(lcd);

		lcd->current_bl = lcd->bl;

		dev_info(&lcd->ld->dev, "id=%d brightness=%d, bl=%d, candela=%d\n", pdata->lcdtype, brightness, lcd->bl, candela_table[lcd->bl]);
	}

	mutex_unlock(&lcd->bl_lock);

	return ret;
}


static int ld9040_power_on(struct lcd_info *lcd)
{
	int ret = 0;
	struct lcd_platform_data *pd = NULL;
	pd = lcd->lcd_pd;

	dev_info(&lcd->ld->dev, "%s\n", __func__);

	if (!pd) {
		dev_err(&lcd->ld->dev, "platform data is NULL.\n");
		return -EFAULT;
	}

	if (!pd->power_on) {
		dev_err(&lcd->ld->dev, "power_on is NULL.\n");
		return -EFAULT;
	} else {
		pd->power_on(lcd->ld, 1);
		msleep(pd->power_on_delay);
	}

	if (!pd->reset) {
		dev_err(&lcd->ld->dev, "reset is NULL.\n");
		return -EFAULT;
	} else {
		pd->reset(lcd->ld);
		msleep(pd->reset_delay);
	}

	ret = ld9040_ldi_init(lcd);

	if (ret) {
		dev_err(&lcd->ld->dev, "failed to initialize ldi.\n");
		goto err;
	}

	ret = ld9040_ldi_enable(lcd);
	if (ret) {
		dev_err(&lcd->ld->dev, "failed to enable ldi.\n");
		goto err;
	}

	lcd->ldi_enable = 1;

	update_brightness(lcd, 1);

err:
	return ret;
}

static int ld9040_power_off(struct lcd_info *lcd)
{
	int ret = 0;
	struct lcd_platform_data *pd = NULL;

	dev_info(&lcd->ld->dev, "%s\n", __func__);

	pd = lcd->lcd_pd;
	if (!pd) {
		dev_err(&lcd->ld->dev, "platform data is NULL.\n");
		return -EFAULT;
	}

	ret = ld9040_ldi_disable(lcd);
	if (ret) {
		dev_err(&lcd->ld->dev, "lcd setting failed.\n");
		ret = -EIO;
		goto err;
	}

	if (!pd->gpio_cfg_earlysuspend) {
		dev_err(&lcd->ld->dev, "gpio_cfg_earlysuspend is NULL.\n");
		ret = -EFAULT;
		goto err;
	} else
		pd->gpio_cfg_earlysuspend(lcd->ld);

	if (!pd->power_on) {
		dev_err(&lcd->ld->dev, "power_on is NULL.\n");
		ret = -EFAULT;
		goto err;
	} else {
		msleep(pd->power_off_delay);
		pd->power_on(lcd->ld, 0);
	}

err:
	return ret;
}

static int ld9040_power(struct lcd_info *lcd, int power)
{
	int ret = 0;

	if (POWER_IS_ON(power) && !POWER_IS_ON(lcd->power))
		ret = ld9040_power_on(lcd);
	else if (!POWER_IS_ON(power) && POWER_IS_ON(lcd->power))
		ret = ld9040_power_off(lcd);

	if (!ret)
		lcd->power = power;

	return ret;
}

static int ld9040_set_power(struct lcd_device *ld, int power)
{
	struct lcd_info *lcd = lcd_get_data(ld);

	if (power != FB_BLANK_UNBLANK && power != FB_BLANK_POWERDOWN &&
		power != FB_BLANK_NORMAL) {
		dev_err(&lcd->ld->dev, "power value should be 0, 1 or 4.\n");
		return -EINVAL;
	}

	return ld9040_power(lcd, power);
}

static int ld9040_get_power(struct lcd_device *ld)
{
	struct lcd_info *lcd = lcd_get_data(ld);

	return lcd->power;
}

static int ld9040_get_brightness(struct backlight_device *bd)
{
	return bd->props.brightness;
}

static int ld9040_set_brightness(struct backlight_device *bd)
{
	int ret = 0, brightness = bd->props.brightness;
	struct lcd_info *lcd = bl_get_data(bd);

	if (brightness < MIN_BRIGHTNESS ||
		brightness > bd->props.max_brightness) {
		dev_err(&bd->dev, "lcd brightness should be %d to %d. now %d\n",
			MIN_BRIGHTNESS, MAX_BRIGHTNESS, brightness);
		return -EINVAL;
	}

	if (lcd->ldi_enable) {
		ret = update_brightness(lcd, 0);
		if (ret < 0)
			return -EINVAL;
	}

	return ret;
}

static struct lcd_ops ld9040_lcd_ops = {
	.set_power = ld9040_set_power,
	.get_power = ld9040_get_power,
};

static const struct backlight_ops ld9040_backlight_ops  = {
	.get_brightness = ld9040_get_brightness,
	.update_status = ld9040_set_brightness,
};

static ssize_t gamma_table_show(struct device *dev, struct
device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	struct ld9040_panel_data *pdata = lcd->lcd_pd->pdata;
	const unsigned short *wbuf;
	int i, j;

	for (i = 0; i < GAMMA_MAX; i++) {
		wbuf = pdata->gamma22_table[i];
		j = 1;
		while ((wbuf[j] & DEFMASK) != ENDDEF) {
			if ((wbuf[j] & DEFMASK) != SLEEPMSEC) {
				if (wbuf[j] != DATA_ONLY)
					printk("0x%02x, ", wbuf[j]);
			}
			j++;
		}
		printk("\n");
	}

	for (i = 0; i < GAMMA_MAX; i++) {
		wbuf = pdata->gamma19_table[i];
		j = 1;
		while ((wbuf[j] & DEFMASK) != ENDDEF) {
			if ((wbuf[j] & DEFMASK) != SLEEPMSEC) {
				if (wbuf[j] != DATA_ONLY)
					printk("0x%02x, ", wbuf[j]);
			}
			j++;
		}
		printk("\n");
	}

#ifdef SMART_DIMMING
	for (i = 0; i < 4; i++) {
		wbuf = pdata->elvss_table[i];
		j = 1;
		while ((wbuf[j] & DEFMASK) != ENDDEF) {
			if ((wbuf[j] & DEFMASK) != SLEEPMSEC) {
				if (wbuf[j] != DATA_ONLY)
					printk("0x%02x, ", wbuf[j]);
			}
			j++;
		}
		printk("\n");
	}
#endif

	return strlen(buf);
}
static DEVICE_ATTR(gamma_table, 0444, gamma_table_show, NULL);

static ssize_t acl_set_show(struct device *dev, struct
device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	char temp[3];

	sprintf(temp, "%d\n", lcd->acl_enable);
	strcpy(buf, temp);

	return strlen(buf);
}
static ssize_t acl_set_store(struct device *dev, struct
device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int value;
	int rc;

	rc = strict_strtoul(buf, (unsigned int)0, (unsigned long *)&value);
	if (rc < 0)
		return rc;
	else {
		if (lcd->acl_enable != value) {
			dev_info(&lcd->ld->dev, "%s - %d, %d\n", __func__, lcd->acl_enable, value);
			mutex_lock(&lcd->bl_lock);
			lcd->acl_enable = value;
			if (lcd->ldi_enable)
				ld9040_set_acl(lcd);
			mutex_unlock(&lcd->bl_lock);
		}
	}
	return size;
}

static DEVICE_ATTR(power_reduce, 0664,
		acl_set_show, acl_set_store);

static ssize_t lcdtype_show(struct device *dev, struct
device_attribute *attr, char *buf)
{

	char temp[15];
	sprintf(temp, "SMD_AMS427G03\n");
	strcat(buf, temp);
	return strlen(buf);
}

static DEVICE_ATTR(lcd_type, 0664,
		lcdtype_show, NULL);

static ssize_t octa_lcdtype_show(struct device *dev, struct
device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	struct ld9040_panel_data *pdata = lcd->lcd_pd->pdata;
	char temp[15];

	switch (pdata->lcdtype) {
	case LCDTYPE_SM2_A1:
		sprintf(temp, "OCTA : SM2 (A1 line)\n");
		strcat(buf, temp);
		break;
	case LCDTYPE_SM2_A2:
		sprintf(temp, "OCTA : SM2 (A2 line)\n");
		strcat(buf, temp);
		break;
	case LCDTYPE_M2:
		sprintf(temp, "OCTA : M2\n");
		strcat(buf, temp);
		break;
	default:
		sprintf(temp, "error\n");
		strcat(buf, temp);
		dev_info(&lcd->ld->dev, "read octa lcd type failed.\n");
		break;
	}
	return strlen(buf);

}

static DEVICE_ATTR(octa_lcdtype, 0664,
		octa_lcdtype_show, NULL);

static ssize_t ld9040_sysfs_show_gamma_mode(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	char temp[10];

	switch (lcd->gamma_mode) {
	case 0:
		sprintf(temp, "2.2 mode\n");
		strcat(buf, temp);
		break;
	case 1:
		sprintf(temp, "1.9 mode\n");
		strcat(buf, temp);
		break;
	default:
		dev_info(&lcd->ld->dev, "gamma mode should be 0:2.2, 1:1.9)\n");
		break;
	}

	return strlen(buf);
}

static ssize_t ld9040_sysfs_store_gamma_mode(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t len)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int value;
	int rc;

	rc = strict_strtoul(buf, 0, (unsigned long *)&value);

	if (rc < 0)
		return rc;

	if (value > 1) {
		dev_err(dev, "there are only 2 types of gamma mode(0:2.2, 1:1.9)\n");
		return len;
	} else
		dev_info(dev, "%s :: gamma_mode=%d\n", __func__, value);

	if (lcd->ldi_enable) {
		if ((lcd->current_bl == lcd->bl) && (lcd->current_gamma_mode == value))
			dev_err(&lcd->ld->dev, "gamma_mode & brightness are same\n");
		else {
			mutex_lock(&lcd->bl_lock);
			lcd->gamma_mode = value;
			ld9040_gamma_ctl(lcd);
			mutex_unlock(&lcd->bl_lock);
		}
	}
	return len;
}

static DEVICE_ATTR(gamma_mode, 0664,
		ld9040_sysfs_show_gamma_mode, ld9040_sysfs_store_gamma_mode);

static ssize_t auto_brightness_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	char temp[3];

	sprintf(temp, "%d\n", lcd->auto_brightness);
	strcpy(buf, temp);

	return strlen(buf);
}

static ssize_t auto_brightness_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int value;
	int rc;

	rc = strict_strtoul(buf, (unsigned int)0, (unsigned long *)&value);
	if (rc < 0)
		return rc;
	else {
		if (lcd->auto_brightness != value) {
			dev_info(dev, "%s - %d, %d\n", __func__, lcd->auto_brightness, value);
			mutex_lock(&lcd->bl_lock);
			lcd->auto_brightness = value;
			mutex_unlock(&lcd->bl_lock);
			if (lcd->ldi_enable)
				update_brightness(lcd, 0);
		}
	}
	return size;
}

static DEVICE_ATTR(auto_brightness, 0644, auto_brightness_show, auto_brightness_store);

static ssize_t ld9040_sysfs_show_user_lcdtype(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	char temp[30];
	switch (lcd->user_lcdtype) {
	case LCDTYPE_SM2_A1:
		sprintf(temp, "SM2 (A1 line)\n");
		strcat(buf, temp);
		break;
	case LCDTYPE_M2:
		sprintf(temp, "M2\n");
		strcat(buf, temp);
		break;
	case LCDTYPE_SM2_A2:
		sprintf(temp, "SM2 (A2 line)\n");
		strcat(buf, temp);
		break;
	default:
		sprintf(temp, "error\n");
		strcat(buf, temp);
		dev_info(dev, "read octa lcd type failed. \n");
		break;
	}
	return strlen(buf);
}

static ssize_t ld9040_sysfs_store_user_lcdtype(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t len)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int rc;
	unsigned int lcdtype, cur_lcdtype;

	rc = strict_strtoul(buf, 0, (unsigned long *)&lcdtype);

	if (rc < 0)
		return rc;

	if (lcd->user_lcdtype == lcdtype)
		return len;

	if (lcdtype == LCDTYPE_M2) {
		lcd->lcd_pd->pdata = &u1_panel_data_m2;
	} else if (lcdtype == LCDTYPE_SM2_A1) {
		lcd->lcd_pd->pdata = &u1_panel_data;
	} else if (lcdtype == LCDTYPE_SM2_A2) {
		lcd->lcd_pd->pdata = &u1_panel_data_a2;
	} else {
		dev_err(dev, "lcdtype mode(0:M2, 1:SM2_A1, 2:SM2_A2)\n");
		return len;
	}

	cur_lcdtype = lcd->user_lcdtype;
	lcd->user_lcdtype = lcdtype;
	dev_info(dev, "%s :: change user_lcdtype=%d\n", __FUNCTION__, lcd->user_lcdtype);

	if (lcd->ldi_enable) {
		if (update_brightness(lcd, 1) != 0) {
			lcd->user_lcdtype = cur_lcdtype;
		}
	}

	return len;
}

static DEVICE_ATTR(user_lcdtype, 0664,
		ld9040_sysfs_show_user_lcdtype, ld9040_sysfs_store_user_lcdtype);


static ssize_t ld9040_sysfs_show_user_gamma_adjust(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	return sprintf(buf, "%d\n", lcd->user_gamma_adjust);
}

static ssize_t ld9040_sysfs_store_user_gamma_adjust(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t len)
{
	struct lcd_info *lcd = dev_get_drvdata(dev);
	int rc;

	rc = strict_strtol(buf, 0, (long *)&lcd->user_gamma_adjust);

	if (rc < 0)
		return rc;

	if ((lcd->user_gamma_adjust < MIN_GAMMA_ADJUST)
	||  (lcd->user_gamma_adjust > MAX_GAMMA_ADJUST)) {
		dev_err(dev, "invalid user_gamma_adjust value. -%d < value < %d\n", MIN_GAMMA_ADJUST, MAX_GAMMA_ADJUST);
		lcd->user_gamma_adjust = 0;
	} else {
		dev_info(dev, "%s :: user_gamma_adjust=%d\n", __FUNCTION__, lcd->user_gamma_adjust);
	}

	if (lcd->ldi_enable)
	{
		if(lcd->current_user_gamma_adjust == lcd->user_gamma_adjust)
			printk("there is no gamma shift & brightness changed\n");	
		else
		{
			dev_info(lcd->dev, "user_gamma_adjust update : (id=%d) bl=%d\n", lcd->user_lcdtype, lcd->bl);
			if (update_brightness(lcd, 1) == 0) {\
				lcd->current_user_gamma_adjust = lcd->user_gamma_adjust;
			} else {
				lcd->user_gamma_adjust = lcd->current_user_gamma_adjust;
			}
		}
	}
	return len;
}

static DEVICE_ATTR(user_gamma_adjust, 0664,
		ld9040_sysfs_show_user_gamma_adjust, ld9040_sysfs_store_user_gamma_adjust);

static ssize_t ld9040_sysfs_show_user_gamma_adjust_table(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
						gamma_adjust_map[0],
						gamma_adjust_map[1],
						gamma_adjust_map[2],
						gamma_adjust_map[3],
						gamma_adjust_map[4],
						gamma_adjust_map[5],
						gamma_adjust_map[6],
						gamma_adjust_map[7],
						gamma_adjust_map[8],
						gamma_adjust_map[9],
						gamma_adjust_map[10],
						gamma_adjust_map[11],
						gamma_adjust_map[12],
						gamma_adjust_map[13],
						gamma_adjust_map[14],
						gamma_adjust_map[15],
						gamma_adjust_map[16],
						gamma_adjust_map[17],
						gamma_adjust_map[18],
						gamma_adjust_map[19]);
}

static ssize_t ld9040_sysfs_store_user_gamma_adjust_table(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t len)
{
	int i, j, k;
	long v = 0;
	char temp[50];

	j = k = 0;
	for (i = 0; i < len; i++) {
		if (buf[i] != ' ') {
			temp[j++] = buf[i];
		} else {
			temp[j] = '\0';
			j = 0;
			if (strict_strtol(temp, 0, &v)) {
				printk("user_gamma_adjust_table update : gamma_adjust_map[%d] = %d", k, (int)v);
				gamma_adjust_map[k++] = (int)v;
			}
		}
	}
	temp[j] = '\0';
	if (strict_strtol(temp, 0, &v)) {
		printk("user_gamma_adjust_table update : gamma_adjust_map[%d] = %d", k, (int)v);
		gamma_adjust_map[k++] = (int)v;
	}

	return len;
}

static DEVICE_ATTR(user_gamma_adjust_table, 0664,
		ld9040_sysfs_show_user_gamma_adjust_table, ld9040_sysfs_store_user_gamma_adjust_table);

#if defined(CONFIG_PM)
#ifdef CONFIG_HAS_EARLYSUSPEND
void ld9040_early_suspend(struct early_suspend *h)
{
	struct lcd_info *lcd = container_of(h, struct lcd_info ,
								early_suspend);
	dev_info(&lcd->ld->dev, "+%s\n", __func__);
	ld9040_power(lcd, FB_BLANK_POWERDOWN);
	dev_info(&lcd->ld->dev, "-%s\n", __func__);

	return ;
}

void ld9040_late_resume(struct early_suspend *h)
{
	struct lcd_info *lcd = container_of(h, struct lcd_info ,
								early_suspend);
	dev_info(&lcd->ld->dev, "+%s\n", __func__);
	ld9040_power(lcd, FB_BLANK_UNBLANK);
	dev_info(&lcd->ld->dev, "-%s\n", __func__);

	return ;
}
#endif
#endif

static int ld9040_probe(struct spi_device *spi)
{
	int ret = 0;
	struct lcd_info *lcd;
	struct ld9040_panel_data *pdata;
#ifdef SMART_DIMMING
	unsigned int i;
	u8 mtp_data[LDI_MTP_LEN] = {0,};
#endif

	lcd = kzalloc(sizeof(struct lcd_info), GFP_KERNEL);
	if (!lcd) {
		pr_err("failed to allocate for lcd\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	/* ld9040 lcd panel uses 3-wire 9bits SPI Mode. */
	spi->bits_per_word = 9;

	ret = spi_setup(spi);
	if (ret < 0) {
		dev_err(&spi->dev, "spi setup failed.\n");
		goto out_free_lcd;
	}

	lcd->spi = spi;
	lcd->dev = &spi->dev;

	lcd->lcd_pd = (struct lcd_platform_data *)spi->dev.platform_data;
	if (!lcd->lcd_pd) {
		dev_err(&spi->dev, "platform data is NULL.\n");
		goto out_free_lcd;
	}

	pdata = lcd->lcd_pd->pdata;
	if (IS_ERR_OR_NULL(pdata)) {
		dev_err(&spi->dev, "panel data is NULL.\n");
		goto out_free_lcd;
	}

	lcd->ld = lcd_device_register("panel", &spi->dev,
		lcd, &ld9040_lcd_ops);
	if (IS_ERR(lcd->ld)) {
		ret = PTR_ERR(lcd->ld);
		goto out_free_lcd;
	}

	lcd->bd = backlight_device_register("panel", &spi->dev,
		lcd, &ld9040_backlight_ops, NULL);
	if (IS_ERR(lcd->bd)) {
		ret = PTR_ERR(lcd->bd);
		goto out_free_backlight;
	}

	lcd->bd->props.max_brightness = MAX_BRIGHTNESS;
	lcd->bd->props.brightness = candela_table[GAMMA_160CD];
	lcd->bl = GAMMA_160CD;
	lcd->current_bl = lcd->bl;
	lcd->gamma_mode = 0;
	lcd->current_gamma_mode = 0;
	lcd->user_gamma_adjust = 0;
	lcd->current_user_gamma_adjust = 0;
	lcd->user_lcdtype = pdata->lcdtype;
	lcd->acl_enable = 0;
	lcd->cur_acl = 0;

	lcd->auto_brightness = 1;

	ret = device_create_file(&lcd->ld->dev, &dev_attr_gamma_mode);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries\n");

	ret = device_create_file(&lcd->ld->dev, &dev_attr_power_reduce);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries\n");

	ret = device_create_file(&lcd->ld->dev, &dev_attr_lcd_type);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries\n");

	ret = device_create_file(&lcd->ld->dev, &dev_attr_octa_lcdtype);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries\n");

	ret = device_create_file(&lcd->ld->dev, &dev_attr_gamma_table);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries\n");

	ret = device_create_file(&lcd->bd->dev, &dev_attr_auto_brightness);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries\n");

	ret = device_create_file(&lcd->ld->dev, &dev_attr_user_lcdtype);
	if (ret < 0)
		dev_err(&(spi->dev), "failed to add sysfs entries\n");

	ret = device_create_file(&lcd->ld->dev, &dev_attr_user_gamma_adjust);
	if (ret < 0)
		dev_err(&(spi->dev), "failed to add sysfs entries\n");

	ret = device_create_file(&lcd->ld->dev, &dev_attr_user_gamma_adjust_table);
	if (ret < 0)
		dev_err(&(spi->dev), "failed to add sysfs entries\n");

	mutex_init(&lcd->lock);
	mutex_init(&lcd->bl_lock);

	/*
	 * if lcd panel was on from bootloader like u-boot then
	 * do not lcd on.
	 */
	if (!lcd->lcd_pd->lcd_enabled) {
		/*
		 * if lcd panel was off from bootloader then
		 * current lcd status is powerdown and then
		 * it enables lcd panel.
		 */
		lcd->power = FB_BLANK_POWERDOWN;

		ld9040_power(lcd, FB_BLANK_UNBLANK);
	} else {
		lcd->power = FB_BLANK_UNBLANK;
		lcd->ldi_enable = 1;
	}

	dev_set_drvdata(&spi->dev, lcd);

#ifdef CONFIG_HAS_EARLYSUSPEND
	lcd->early_suspend.suspend = ld9040_early_suspend;
	lcd->early_suspend.resume = ld9040_late_resume;
	lcd->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB - 1;
	register_early_suspend(&lcd->early_suspend);
#endif

	if (pdata->lcdtype == LCDTYPE_M2)
		dev_info(&lcd->ld->dev, "%s : lcdtype : M2\n", __func__);
	else if (pdata->lcdtype == LCDTYPE_SM2_A1)
		dev_info(&lcd->ld->dev, "%s : lcdtype : SM2_A1\n", __func__);
	else if (pdata->lcdtype == LCDTYPE_SM2_A2)
		dev_info(&lcd->ld->dev, "%s : lcdtype : SM2_A2\n", __func__);
	else	/* UNKNOWN */
		dev_info(&lcd->ld->dev, "%s : lcdtype : Unknown(SM2_A1)\n", __func__);

	dev_info(&lcd->ld->dev, "ld9040 panel driver has been probed.\n");

#ifdef SMART_DIMMING
	for (i = 0; i < LDI_ID_LEN; i++) {
		lcd->id[i] = ld9040_read_id(lcd, LDI_ID_REG + i);
		lcd->smart.panelid[i] = lcd->id[i];
	}

	if (lcd->id[2] == LD9040_ID3)
		return 0;

	/* prepare initial data to operate smart dimming */

	printk(KERN_INFO "id: %x, %x, %x", lcd->id[0], lcd->id[1], lcd->id[2]);

	init_table_info_22(&lcd->smart);
	init_table_info_19(&lcd->smart);

	spi_read_multi_byte(lcd, LDI_MTP_REG, mtp_data, LDI_MTP_LEN);

	calc_voltage_table(&lcd->smart, mtp_data);

#if 0
	for (i = 0; i < LDI_MTP_LEN; i++)
		printk(KERN_INFO "%d\n", mtp_data[i]);
#endif

	ld9042_init_smart_dimming_table_22(lcd);
	ld9042_init_smart_dimming_table_19(lcd);
	ld9042_init_smart_elvss_table(lcd);

	pdata->elvss_table = (const unsigned short **)ELVSS_TABLE;
	pdata->gamma19_table = (const unsigned short **)ld9042_19gamma_table;
	pdata->gamma22_table = (const unsigned short **)ld9042_22gamma_table;

	update_brightness(lcd, 1);
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

static int __devexit ld9040_remove(struct spi_device *spi)
{
	struct lcd_info *lcd = dev_get_drvdata(&spi->dev);

	ld9040_power(lcd, FB_BLANK_POWERDOWN);
	lcd_device_unregister(lcd->ld);
	backlight_device_unregister(lcd->bd);
	kfree(lcd);

	return 0;
}

static struct spi_driver ld9040_driver = {
	.driver = {
		.name	= "ld9040",
		.bus	= &spi_bus_type,
		.owner	= THIS_MODULE,
	},
	.probe		= ld9040_probe,
	.remove		= __devexit_p(ld9040_remove),
};

static int __init ld9040_init(void)
{
	int ret;
	
	ret = misc_register(&brightness_curve_device);
	if (ret) {
	   printk(KERN_ERR "failed at(%d)\n", __LINE__);
	}

	ret = sysfs_create_group(&brightness_curve_device.this_device->kobj, 
			&brightness_curve_group);
	if (ret)
	{
		printk(KERN_ERR "failed at(%d)\n", __LINE__);
	}

	return spi_register_driver(&ld9040_driver);
}

static void __exit ld9040_exit(void)
{
	spi_unregister_driver(&ld9040_driver);
}

module_init(ld9040_init);
module_exit(ld9040_exit);

MODULE_DESCRIPTION("LD9040 AMOLED LCD Driver");
MODULE_LICENSE("GPL");

