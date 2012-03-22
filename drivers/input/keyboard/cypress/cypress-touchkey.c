/*
 * Driver for keys on GPIO lines capable of generating interrupts.
 *
 * Copyright 2005 Phil Blundell
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>
#include <asm/gpio.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/earlysuspend.h>
#include <asm/io.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include "u1-cypress-gpio.h"

#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio.h>

#include "issp_extern.h"
#ifdef CONFIG_TOUCHSCREEN_ATMEL_MXT540E
#include <linux/i2c/mxt540e.h>
#else
#include <linux/i2c/mxt224_u1.h>
#endif

/*
touchkey register
*/
#define KEYCODE_REG 0x00
#define FIRMWARE_VERSION 0x01
#define TOUCHKEY_MODULE_VERSION 0x02
#define TOUCHKEY_ADDRESS	0x20

#define UPDOWN_EVENT_BIT 0x08
#define KEYCODE_BIT 0x07

#define I2C_M_WR 0		/* for i2c */

#define DEVICE_NAME "sec_touchkey"
#define TOUCH_FIRMWARE_V04  0x04
#define TOUCH_FIRMWARE_V07  0x07
#define DOOSUNGTECH_TOUCH_V1_2  0x0C

#if defined(CONFIG_MACH_Q1_BD)
#define TK_FIRMWARE_VER  0x12
#define TK_MODULE_VER    0x11
#elif defined(CONFIG_MACH_C1_NA_USCC_REV05)
#define TK_FIRMWARE_VER  0x0E
#define TK_MODULE_VER    0x08
#else
#define TK_FIRMWARE_VER	 0x04
#define TK_MODULE_VER    0x00
#endif

#if defined(CONFIG_TARGET_LOCALE_NAATT_TEMP)
/* Temp Fix NAGSM_SEL_ANDROID_MOHAMMAD_ANSARI_20111224*/
#define CONFIG_TARGET_LOCALE_NAATT
#endif

#if defined(CONFIG_TARGET_LOCALE_NAATT)
static int touchkey_keycode[5] = { 0,
	KEY_MENU, KEY_ENTER, KEY_BACK, KEY_END };
#elif defined(CONFIG_TARGET_LOCALE_NA)
static int touchkey_keycode[5] = { NULL,
	KEY_SEARCH, KEY_BACK, KEY_HOME, KEY_MENU };
#else
static int touchkey_keycode[3] = { 0, KEY_MENU, KEY_BACK };
#endif
static const int touchkey_count = sizeof(touchkey_keycode) / sizeof(int);

#if defined(CONFIG_TARGET_LOCALE_NAATT)\
	|| defined(CONFIG_TARGET_LOCALE_NA)\
	|| defined(CONFIG_MACH_Q1_BD)

static u8 home_sensitivity;
static u8 search_sensitivity;
static u16 raw_data0;
static u16 raw_data1;
static u16 raw_data2;
static u16 raw_data3;
static u8 idac0;
static u8 idac1;
static u8 idac2;
static u8 idac3;
static u8 touchkey_threshold;

static int touchkey_autocalibration(void);
#endif
static int get_touchkey_module_version(void);

static u8 menu_sensitivity;
static u8 back_sensitivity;

static int touchkey_enable;
static bool touchkey_probe = true;

struct device *sec_touchkey;

struct i2c_touchkey_driver {
	struct i2c_client *client;
	struct input_dev *input_dev;
	struct early_suspend early_suspend;
};
struct i2c_touchkey_driver *touchkey_driver;
struct work_struct touchkey_work;
struct workqueue_struct *touchkey_wq;

struct work_struct touch_update_work;
struct delayed_work touch_resume_work;

#ifdef WHY_DO_WE_NEED_THIS
static void __iomem *gpio_pend_mask_mem;
#define		INT_PEND_BASE	0xE0200A54
#endif

static const struct i2c_device_id sec_touchkey_id[] = {
	{"sec_touchkey", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, sec_touchkey_id);

static void init_hw(void);
static int i2c_touchkey_probe(struct i2c_client *client,
			      const struct i2c_device_id *id);

extern int get_touchkey_firmware(char *version);
static int touchkey_led_status;
static int touchled_cmd_reversed;

struct i2c_driver touchkey_i2c_driver = {
	.driver = {
		   .name = "sec_touchkey_driver",
		   },
	.id_table = sec_touchkey_id,
	.probe = i2c_touchkey_probe,
};

static int touchkey_debug_count;
static char touchkey_debug[104];
static int touch_version;
static int module_version;
#ifdef CONFIG_TARGET_LOCALE_NA
static int store_module_version;
#endif

static int touchkey_update_status;

int touchkey_led_ldo_on(bool on)
{
	struct regulator *regulator;

#if defined(CONFIG_MACH_S2PLUS)
	if (on) {
		gpio_direction_output(GPIO_3_TOUCH_EN, 1);
	} else {
		gpio_direction_output(GPIO_3_TOUCH_EN, 0);
	}
#else
	if (on) {
		regulator = regulator_get(NULL, "touch_led");
		if (IS_ERR(regulator))
			return 0;
		regulator_enable(regulator);
		regulator_put(regulator);
	} else {
		regulator = regulator_get(NULL, "touch_led");
		if (IS_ERR(regulator))
			return 0;
		if (regulator_is_enabled(regulator))
			regulator_force_disable(regulator);
		regulator_put(regulator);
	}
#endif
	return 0;
}

int touchkey_ldo_on(bool on)
{
	struct regulator *regulator;

#if defined(CONFIG_MACH_S2PLUS)
	if (on) {
		regulator = regulator_get(NULL, "3_touch_1.8v");
		if (IS_ERR(regulator))
			return 0;
		regulator_enable(regulator);
		regulator_put(regulator);
	} else {
		regulator = regulator_get(NULL, "3_touch_1.8v");
		if (IS_ERR(regulator))
			return 0;
		if (regulator_is_enabled(regulator))
			regulator_force_disable(regulator);
		regulator_put(regulator);
	}
#else
	if (on) {
		regulator = regulator_get(NULL, "touch");
		if (IS_ERR(regulator))
			return 0;
		regulator_enable(regulator);
		regulator_put(regulator);
	} else {
		regulator = regulator_get(NULL, "touch");
		if (IS_ERR(regulator))
			return 0;
		if (regulator_is_enabled(regulator))
			regulator_force_disable(regulator);
		regulator_put(regulator);
	}
#endif

	return 1;
}

static void change_touch_key_led_voltage(int vol_mv)
{
	struct regulator *tled_regulator;

	tled_regulator = regulator_get(NULL, "touch_led");
	if (IS_ERR(tled_regulator)) {
		pr_err("%s: failed to get resource %s\n", __func__,
		       "touch_led");
		return;
	}
	regulator_set_voltage(tled_regulator, vol_mv * 1000, vol_mv * 1000);
	regulator_put(tled_regulator);
}

static ssize_t brightness_control(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t size)
{
	int data;

	if (sscanf(buf, "%d\n", &data) == 1) {
		printk(KERN_ERR "[TouchKey] touch_led_brightness: %d\n", data);
		change_touch_key_led_voltage(data);
	} else {
		printk(KERN_ERR "[TouchKey] touch_led_brightness Error\n");
	}

	return size;
}

static void set_touchkey_debug(char value)
{
	if (touchkey_debug_count == 100)
		touchkey_debug_count = 0;

	touchkey_debug[touchkey_debug_count] = value;
	touchkey_debug_count++;
}

static int i2c_touchkey_read(u8 reg, u8 *val, unsigned int len)
{
	int err = 0;
	int retry = 2;
	struct i2c_msg msg[1];

	if ((touchkey_driver == NULL) || !(touchkey_enable == 1)
	    || !touchkey_probe) {
		printk(KERN_ERR "[TouchKey] touchkey is not enabled. %d\n",
		       __LINE__);
		return -ENODEV;
	}

	while (retry--) {
		msg->addr = touchkey_driver->client->addr;
		msg->flags = I2C_M_RD;
		msg->len = len;
		msg->buf = val;
		err = i2c_transfer(touchkey_driver->client->adapter, msg, 1);

		if (err >= 0)
			return 0;
		printk(KERN_ERR "[TouchKey] %s %d i2c transfer error\n",
		       __func__, __LINE__);
		mdelay(10);
	}
	return err;

}

static int i2c_touchkey_write(u8 *val, unsigned int len)
{
	int err = 0;
	struct i2c_msg msg[1];
	int retry = 2;

	if ((touchkey_driver == NULL) || !(touchkey_enable == 1)
	    || !touchkey_probe) {
		printk(KERN_ERR "[TouchKey] touchkey is not enabled. %d\n",
		       __LINE__);
		return -ENODEV;
	}

	while (retry--) {
		msg->addr = touchkey_driver->client->addr;
		msg->flags = I2C_M_WR;
		msg->len = len;
		msg->buf = val;
		err = i2c_transfer(touchkey_driver->client->adapter, msg, 1);

		if (err >= 0)
			return 0;

		printk(KERN_DEBUG "[TouchKey] %s %d i2c transfer error\n",
		       __func__, __LINE__);
		mdelay(10);
	}
	return err;
}

#if defined(CONFIG_TARGET_LOCALE_NAATT) \
|| defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_MACH_Q1_BD)
static int touchkey_autocalibration(void)
{
	u8 data[6] = { 0, };
	int count = 0;
	int ret = 0;
	unsigned short retry = 0;

	while (retry < 3) {
		ret = i2c_touchkey_read(KEYCODE_REG, data, 4);
		if (ret < 0) {
			printk(KERN_ERR "[TouchKey]i2c read fail.\n");
			return ret;
		}
		printk(KERN_DEBUG
		       "[TouchKey] %s : data[0]=%x data[1]=%x data[2]=%x data[3]=%x\n",
		       __func__, data[0], data[1], data[2], data[3]);

		/* Send autocal Command */
		data[0] = 0x50;
		data[3] = 0x01;

		count = i2c_touchkey_write(data, 4);

		msleep(100);

		/* Check autocal status */
		ret = i2c_touchkey_read(KEYCODE_REG, data, 6);

		if ((data[5] & 0x80)) {
			printk(KERN_DEBUG "[Touchkey] autocal Enabled\n");
			break;
		} else
			printk(KERN_DEBUG
			       "[Touchkey] autocal disabled, retry %d\n",
			       retry);

		retry = retry + 1;
	}

	if (retry == 3)
		printk(KERN_DEBUG "[Touchkey] autocal failed\n");

	return count;
}
#endif

#ifdef CONFIG_TARGET_LOCALE_NAATT
static ssize_t set_touchkey_autocal_testmode(struct device *dev,
					     struct device_attribute *attr,
					     const char *buf, size_t size)
{
	int count = 0;
	u8 set_data;
	int on_off;

	if (sscanf(buf, "%d\n", &on_off) == 1) {
		printk(KERN_ERR "[TouchKey] Test Mode : %d\n", on_off);

		if (on_off == 1) {
			set_data = 0x40;
			count = i2c_touchkey_write(&set_data, 1);
		} else {
			touchkey_ldo_on(0);
			msleep(50);
			touchkey_ldo_on(1);
			msleep(50);
			init_hw();
			msleep(50);
#if defined(CONFIG_TARGET_LOCALE_NAATT) \
|| defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_MACH_Q1_BD)
			touchkey_autocalibration();
#endif
		}
	} else {
		printk(KERN_ERR "[TouchKey] touch_led_brightness Error\n");
	}

	return count;
}
#endif

#if defined(CONFIG_TARGET_LOCALE_NAATT) \
|| defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_MACH_Q1_BD)
static ssize_t touchkey_raw_data0_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	u8 data[26] = { 0, };
	int ret;

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 26);
#if defined(CONFIG_TARGET_LOCALE_NA)
	printk(KERN_DEBUG "called %s data[18] =%d,data[19] = %d\n", __func__,
	       data[18], data[19]);
	raw_data0 = ((0x00FF & data[18]) << 8) | data[19];
#elif defined(CONFIG_MACH_Q1_BD)
	printk(KERN_DEBUG "called %s data[16] =%d,data[17] = %d\n", __func__,
	       data[16], data[17]);
	raw_data0 = ((0x00FF & data[14]) << 8) | data[15];
#else
	printk(KERN_DEBUG "called %s data[18] =%d,data[19] = %d\n", __func__,
	       data[10], data[11]);
	raw_data0 = ((0x00FF & data[10]) << 8) | data[11];
#endif
	return sprintf(buf, "%d\n", raw_data0);
}

static ssize_t touchkey_raw_data1_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	u8 data[26] = { 0, };
	int ret;

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 26);
#if defined(CONFIG_TARGET_LOCALE_NA)
	printk(KERN_DEBUG "called %s data[20] =%d,data[21] = %d\n", __func__,
	       data[20], data[21]);
	raw_data1 = ((0x00FF & data[20]) << 8) | data[21];
#elif defined(CONFIG_MACH_Q1_BD)
	printk(KERN_DEBUG "called %s data[14] =%d,data[15] = %d\n", __func__,
	       data[14], data[15]);
	raw_data1 = ((0x00FF & data[16]) << 8) | data[17];
#else
	printk(KERN_DEBUG "called %s data[20] =%d,data[21] = %d\n", __func__,
	       data[12], data[13]);
	raw_data1 = ((0x00FF & data[12]) << 8) | data[13];
#endif				/* CONFIG_TARGET_LOCALE_NA */
	return sprintf(buf, "%d\n", raw_data1);
}

static ssize_t touchkey_raw_data2_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	u8 data[26] = { 0, };
	int ret;

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 26);
#if defined(CONFIG_TARGET_LOCALE_NA)
	printk(KERN_DEBUG "called %s data[22] =%d,data[23] = %d\n", __func__,
	       data[22], data[23]);
	raw_data2 = ((0x00FF & data[22]) << 8) | data[23];
#else
	printk(KERN_DEBUG "called %s data[22] =%d,data[23] = %d\n", __func__,
	       data[14], data[15]);
	raw_data2 = ((0x00FF & data[14]) << 8) | data[15];
#endif				/* CONFIG_TARGET_LOCALE_NA */
	return sprintf(buf, "%d\n", raw_data2);
}

static ssize_t touchkey_raw_data3_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	u8 data[26] = { 0, };
	int ret;

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 26);
#if defined(CONFIG_TARGET_LOCALE_NA)
	printk(KERN_DEBUG "called %s data[24] =%d,data[25] = %d\n", __func__,
	       data[24], data[25]);
	raw_data3 = ((0x00FF & data[24]) << 8) | data[25];
#else
	printk(KERN_DEBUG "called %s data[24] =%d,data[25] = %d\n", __func__,
	       data[16], data[17]);
	raw_data3 = ((0x00FF & data[16]) << 8) | data[17];
#endif				/* CONFIG_TARGET_LOCALE_NA */
	return sprintf(buf, "%d\n", raw_data3);
}

static ssize_t touchkey_idac0_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	u8 data[10];
	int ret;
#ifdef CONFIG_TARGET_LOCALE_NA
	if (store_module_version < 8)
		return 0;
#endif

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
	printk(KERN_DEBUG "called %s data[6] =%d\n", __func__, data[6]);
	idac0 = data[6];
	return sprintf(buf, "%d\n", idac0);
}

static ssize_t touchkey_idac1_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	u8 data[10];
	int ret;
#ifdef CONFIG_TARGET_LOCALE_NA
	if (store_module_version < 8)
		return 0;
#endif

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
	printk(KERN_DEBUG "called %s data[7] = %d\n", __func__, data[7]);
	idac1 = data[7];
	return sprintf(buf, "%d\n", idac1);
}

static ssize_t touchkey_idac2_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	u8 data[10];
	int ret;
#ifdef CONFIG_TARGET_LOCALE_NA
	if (store_module_version < 8)
		return 0;
#endif

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
	printk(KERN_DEBUG "called %s data[8] =%d\n", __func__, data[8]);
	idac2 = data[8];
	return sprintf(buf, "%d\n", idac2);
}

static ssize_t touchkey_idac3_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	u8 data[10];
	int ret;
#ifdef CONFIG_TARGET_LOCALE_NA
	if (store_module_version < 8)
		return 0;
#endif

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
	printk(KERN_DEBUG "called %s data[9] = %d\n", __func__, data[9]);
	idac3 = data[9];
	return sprintf(buf, "%d\n", idac3);
}

static ssize_t touchkey_threshold_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	u8 data[10];
	int ret;

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
	printk(KERN_DEBUG "called %s data[4] = %d\n", __func__, data[4]);
	touchkey_threshold = data[4];
	return sprintf(buf, "%d\n", touchkey_threshold);
}
#endif

#if defined(CONFIG_MACH_C1_NA_SPR_EPIC2_REV00) \
	|| defined(CONFIG_MACH_Q1_BD) \
	|| defined(CONFIG_MACH_C1_NA_USCC_REV05) \
	|| defined(CONFIG_TARGET_LOCALE_NA)
void touchkey_firmware_update(void)
{
	char data[3];
	int retry = 3;
	int ret = 0;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 3);
	if (ret < 0) {
		printk(KERN_DEBUG
		       "[TouchKey] i2c read fail. do not excute firm update.\n");
		return;
	}

	touch_version = data[1];
	module_version = data[2];

#ifdef CONFIG_MACH_C1_NA_SPR_EPIC2_REV00
	if (system_rev > 6) {
		printk(KERN_DEBUG "[TouchKey] not firmup hw(system_rev=%d)\n",
		       system_rev);
		return;
	}
#endif

	if ((touch_version < TK_FIRMWARE_VER) &&
	    (module_version == TK_MODULE_VER)) {
		printk(KERN_DEBUG "[TouchKey] firmware auto update excute\n");
		disable_irq(IRQ_TOUCH_INT);
		touchkey_update_status = 1;

		while (retry--) {
			if (ISSP_main() == 0) {
				printk(KERN_DEBUG
				       "[TouchKey]firmware update succeeded\n");
				touchkey_update_status = 0;
				break;
			}
			msleep(100);
			printk(KERN_DEBUG
			       "[TouchKey] firmware update failed. retry\n");
		}
		if (retry <= 0) {
			touchkey_ldo_on(0);
			touchkey_update_status = -1;
			printk(KERN_DEBUG
			       "[TouchKey] firmware update failed.\n");
			msleep(300);
		}
		enable_irq(IRQ_TOUCH_INT);
		init_hw();
	} else {
		printk(KERN_DEBUG
		       "[TouchKey] firmware auto update do not excute\n");
		printk(KERN_DEBUG
		       "[TouchKey] firmware_ver(banary=%d, current=%d)\n",
		       TK_FIRMWARE_VER, touch_version);
		printk(KERN_DEBUG
		       "[TouchKey] module_ver(banary=%d, current=%d)\n",
		       TK_MODULE_VER, module_version);
		return;
	}
	msleep(100);
	i2c_touchkey_read(KEYCODE_REG, data, 3);
	touch_version = data[1];
	module_version = data[2];
	printk(KERN_DEBUG "[TouchKey] firm ver = %d, module ver = %d\n",
	       touch_version, module_version);
}
#else
void touchkey_firmware_update(void)
{
	char data[3];
	int retry;
	int ret = 0;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 3);
	if (ret < 0) {
		printk(KERN_DEBUG
		       "[TouchKey] i2c read fail. do not excute firm update.\n");
		return;
	}

	printk(KERN_ERR "%s F/W version: 0x%x, Module version:0x%x\n", __func__,
	       data[1], data[2]);
	retry = 3;

	touch_version = data[1];
	module_version = data[2];

	if (touch_version < 0x0A) {
		touchkey_update_status = 1;
		while (retry--) {
			if (ISSP_main() == 0) {
				printk(KERN_ERR
				       "[TOUCHKEY]Touchkey_update succeeded\n");
				touchkey_update_status = 0;
				break;
			}
			printk(KERN_ERR "touchkey_update failed...retry...\n");
		}
		if (retry <= 0) {
			touchkey_ldo_on(0);
			touchkey_update_status = -1;
			msleep(300);
		}

		init_hw();
	} else {
		if (touch_version >= 0x0A) {
			printk(KERN_ERR
			       "[TouchKey] Not F/W update. Cypess touch-key F/W version is latest\n");
		} else {
			printk(KERN_ERR
			       "[TouchKey] Not F/W update. Cypess touch-key version(module or F/W) is not valid\n");
		}
	}
}
#endif

#ifndef TEST_JIG_MODE
void touchkey_work_func(struct work_struct *p)
{
	u8 data[3];
	int ret;
	int retry = 10;
	int keycode_type = 0;
	int pressed;

	set_touchkey_debug('a');

	retry = 3;
	while (retry--) {
		ret = i2c_touchkey_read(KEYCODE_REG, data, 3);
		if (!ret)
			break;
		else {
			printk(KERN_DEBUG
			       "[TouchKey] i2c read failed, ret:%d, retry: %d\n",
			       ret, retry);
			continue;
		}
	}
	if (ret < 0) {
		enable_irq(IRQ_TOUCH_INT);
		return;
	}
	set_touchkey_debug(data[0]);

	keycode_type = (data[0] & KEYCODE_BIT);
	pressed = !(data[0] & UPDOWN_EVENT_BIT);

	if (keycode_type <= 0 || keycode_type >= touchkey_count) {
		printk(KERN_DEBUG "[Touchkey] keycode_type err\n");
		enable_irq(IRQ_TOUCH_INT);
		return;
	}

	if (pressed)
		set_touchkey_debug('P');

	if (get_tsp_status() && pressed)
		printk(KERN_DEBUG "[TouchKey] touchkey pressed but don't send event because touch is pressed.\n");
	else {
		input_report_key(touchkey_driver->input_dev,
				 touchkey_keycode[keycode_type], pressed);
		input_sync(touchkey_driver->input_dev);
		/* printk(KERN_DEBUG "[TouchKey] keycode:%d pressed:%d\n",
		   touchkey_keycode[keycode_index], pressed); */
	}
	set_touchkey_debug('A');
	enable_irq(IRQ_TOUCH_INT);
}
#else
void touchkey_work_func(struct work_struct *p)
{
	u8 data[18];
	int ret;
	int retry = 10;
	int keycode_type = 0;
	int pressed;

#if 0
	if (gpio_get_value(_3_GPIO_TOUCH_INT)) {
		printk(KERN_DEBUG "[TouchKey] Unknown state.\n", __func__);
		enable_irq(IRQ_TOUCH_INT);
		return;
	}
#endif

	set_touchkey_debug('a');

#ifdef CONFIG_CPU_FREQ
	/* set_dvfs_target_level(LEV_800MHZ); */
#endif

	retry = 3;
	while (retry--) {
#if defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_MACH_Q1_BD)
		ret = i2c_touchkey_read(KEYCODE_REG, data, 18);
#else
		ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
#endif
		if (!ret)
			break;
		else {
			printk(KERN_DEBUG
			       "[TouchKey] i2c read failed, ret:%d, retry: %d\n",
			       ret, retry);
			continue;
		}
	}
	if (ret < 0) {
		enable_irq(IRQ_TOUCH_INT);
		return;
	}
#if defined(CONFIG_TARGET_LOCALE_NA)
#if defined(CONFIG_MACH_C1_NA_SPR_EPIC2_REV00)
	menu_sensitivity = data[11];
	home_sensitivity = data[13];
	search_sensitivity = data[15];
	back_sensitivity = data[17];
#else
	if (store_module_version >= 8) {
		menu_sensitivity = data[17];
		home_sensitivity = data[15];
		search_sensitivity = data[11];
		back_sensitivity = data[13];
	} else {
		menu_sensitivity = data[6];
		home_sensitivity = data[7];
		search_sensitivity = data[8];
		back_sensitivity = data[9];
	}
#endif
#elif defined(CONFIG_MACH_Q1_BD)
	menu_sensitivity = data[13];
	back_sensitivity = data[11];
#else
	menu_sensitivity = data[7];
	back_sensitivity = data[9];
#endif				/* CONFIG_TARGET_LOCALE_NA  */

	set_touchkey_debug(data[0]);

	keycode_type = (data[0] & KEYCODE_BIT);
	pressed = !(data[0] & UPDOWN_EVENT_BIT);

	if (keycode_type <= 0 || keycode_type >= touchkey_count) {
		printk(KERN_DEBUG "[Touchkey] keycode_type err\n");
		enable_irq(IRQ_TOUCH_INT);
		return;
	}

	if (pressed)
		set_touchkey_debug('P');

	if (get_tsp_status() && pressed)
		printk(KERN_DEBUG "[TouchKey] touchkey pressed"
		       " but don't send event because touch is pressed.\n");
	else {
		input_report_key(touchkey_driver->input_dev,
				 touchkey_keycode[keycode_type], pressed);
		input_sync(touchkey_driver->input_dev);
		/* printk(KERN_DEBUG "[TouchKey] keycode:%d pressed:%d\n",
		   touchkey_keycode[keycode_index], pressed); */
	}

	if (keycode_type == 1)
		printk(KERN_DEBUG "search key sensitivity = %d\n",
		       search_sensitivity);
	if (keycode_type == 2)
		printk(KERN_DEBUG "back key sensitivity = %d\n",
		       back_sensitivity);
#ifdef CONFIG_TARGET_LOCALE_NA
	if (keycode_type == 3)
		printk(KERN_DEBUG "home key sensitivity = %d\n",
		       home_sensitivity);
	if (keycode_type == 4)
		printk(KERN_DEBUG "menu key sensitivity = %d\n",
		       menu_sensitivity);
#endif

#ifdef WHY_DO_WE_NEED_THIS
	/* clear interrupt */
	if (readl(gpio_pend_mask_mem) & (0x1 << 1)) {
		writel(readl(gpio_pend_mask_mem) | (0x1 << 1),
		       gpio_pend_mask_mem);
	}
#endif
	set_touchkey_debug('A');
	enable_irq(IRQ_TOUCH_INT);
}
#endif

static irqreturn_t touchkey_interrupt(int irq, void *dummy)
{
	set_touchkey_debug('I');
	disable_irq_nosync(IRQ_TOUCH_INT);
	queue_work(touchkey_wq, &touchkey_work);

	return IRQ_HANDLED;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static int sec_touchkey_early_suspend(struct early_suspend *h)
{
	int ret;
	int i;

	disable_irq(IRQ_TOUCH_INT);
	ret = cancel_work_sync(&touchkey_work);
	if (ret) {
		printk(KERN_DEBUG "[Touchkey] enable_irq ret=%d\n", ret);
		enable_irq(IRQ_TOUCH_INT);
	}

	/* release keys */
	for (i = 1; i < touchkey_count; ++i) {
		input_report_key(touchkey_driver->input_dev,
				 touchkey_keycode[i], 0);
	}

	touchkey_enable = 0;
	set_touchkey_debug('S');
	printk(KERN_DEBUG "[TouchKey] sec_touchkey_early_suspend\n");
	if (touchkey_enable < 0) {
		printk(KERN_DEBUG "[TouchKey] ---%s---touchkey_enable: %d\n",
		       __func__, touchkey_enable);
		return 0;
	}

	gpio_direction_input(_3_GPIO_TOUCH_INT);
#if 0
	gpio_direction_output(_3_GPIO_TOUCH_EN, 0);
	gpio_direction_output(_3_TOUCH_SDA_28V, 0);
	gpio_direction_output(_3_TOUCH_SCL_28V, 0);
	s3c_gpio_setpull(_3_GPIO_TOUCH_INT, S3C_GPIO_PULL_DOWN);
#endif

	/* disable ldo18 */
	touchkey_led_ldo_on(0);

	/* disable ldo11 */
	touchkey_ldo_on(0);

	return 0;
}

static int sec_touchkey_late_resume(struct early_suspend *h)
{
#ifdef TEST_JIG_MODE
	unsigned char get_touch = 0x40;
#endif

	set_touchkey_debug('R');
	printk(KERN_DEBUG "[TouchKey] sec_touchkey_late_resume\n");

	/* enable ldo11 */
	touchkey_ldo_on(1);

	if (touchkey_enable < 0) {
		printk(KERN_DEBUG "[TouchKey] ---%s---touchkey_enable: %d\n",
		       __func__, touchkey_enable);
		return 0;
	}
	gpio_direction_output(_3_GPIO_TOUCH_EN, 1);
	gpio_direction_output(_3_TOUCH_SDA_28V, 1);
	gpio_direction_output(_3_TOUCH_SCL_28V, 1);

	gpio_direction_output(_3_GPIO_TOUCH_INT, 1);
	irq_set_irq_type(IRQ_TOUCH_INT, IRQF_TRIGGER_FALLING);
	s3c_gpio_cfgpin(_3_GPIO_TOUCH_INT, _3_GPIO_TOUCH_INT_AF);
	s3c_gpio_setpull(_3_GPIO_TOUCH_INT, S3C_GPIO_PULL_NONE);
	msleep(50);
	touchkey_led_ldo_on(1);

#ifdef WHY_DO_WE_NEED_THIS
	/* clear interrupt */
	if (readl(gpio_pend_mask_mem) & (0x1 << 1)) {
		writel(readl(gpio_pend_mask_mem) | (0x1 << 1),
		       gpio_pend_mask_mem);
	}
#endif

	touchkey_enable = 1;

#if defined(CONFIG_TARGET_LOCALE_NAATT) \
|| defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_MACH_Q1_BD)
#if defined(CONFIG_TARGET_LOCALE_NA)
	if (store_module_version >= 8) {
#endif
		msleep(50);
		touchkey_autocalibration();
		msleep(200);
#if defined(CONFIG_TARGET_LOCALE_NA)
	}
#endif				/* CONFIG_TARGET_LOCALE_NA */
#endif

	if (touchled_cmd_reversed) {
		touchled_cmd_reversed = 0;
		i2c_touchkey_write((u8 *) &touchkey_led_status, 1);
		printk(KERN_DEBUG "LED returned on\n");
	}
#ifdef TEST_JIG_MODE
	i2c_touchkey_write(&get_touch, 1);
#endif

	enable_irq(IRQ_TOUCH_INT);

	return 0;
}
#endif

static int i2c_touchkey_probe(struct i2c_client *client,
			      const struct i2c_device_id *id)
{
	struct device *dev = &client->dev;
	struct input_dev *input_dev;
	int err = 0;
	unsigned char data;
	int i;
	int module_version;

	printk(KERN_DEBUG "[TouchKey] i2c_touchkey_probe\n");

	touchkey_driver =
	    kzalloc(sizeof(struct i2c_touchkey_driver), GFP_KERNEL);
	if (touchkey_driver == NULL) {
		dev_err(dev, "failed to create our state\n");
		return -ENOMEM;
	}

	touchkey_driver->client = client;
	touchkey_driver->client->irq = IRQ_TOUCH_INT;
	strlcpy(touchkey_driver->client->name, "sec_touchkey", I2C_NAME_SIZE);

	input_dev = input_allocate_device();

	if (!input_dev)
		return -ENOMEM;

	touchkey_driver->input_dev = input_dev;

	input_dev->name = DEVICE_NAME;
	input_dev->phys = "sec_touchkey/input0";
	input_dev->id.bustype = BUS_HOST;

	set_bit(EV_SYN, input_dev->evbit);
	set_bit(EV_LED, input_dev->evbit);
	set_bit(LED_MISC, input_dev->ledbit);
	set_bit(EV_KEY, input_dev->evbit);

	for (i = 1; i < touchkey_count; i++)
		set_bit(touchkey_keycode[i], input_dev->keybit);

	err = input_register_device(input_dev);
	if (err) {
		input_free_device(input_dev);
		return err;
	}
#ifdef WHY_DO_WE_NEED_THIS
	gpio_pend_mask_mem = ioremap(INT_PEND_BASE, 0x10);
#endif

#if defined(CONFIG_MACH_S2PLUS)
	gpio_request(GPIO_3_TOUCH_EN, "gpio_3_touch_en");
#endif

	/* enable ldo18 */
	touchkey_ldo_on(1);

	msleep(50);

	touchkey_enable = 1;
	data = 1;

	module_version = get_touchkey_module_version();
	if (module_version < 0) {
		printk(KERN_ERR "[TouchKey] Probe fail\n");
		input_unregister_device(input_dev);
		touchkey_probe = false;
		return -ENODEV;
	}

#ifdef CONFIG_TARGET_LOCALE_NA
	store_module_version = module_version;
#endif				/* CONFIG_TARGET_LOCALE_NA */
	if (request_irq
	    (IRQ_TOUCH_INT, touchkey_interrupt, IRQF_TRIGGER_FALLING,
	     DEVICE_NAME, NULL)) {
		printk(KERN_ERR "[TouchKey] %s Can't allocate irq ..\n",
		       __func__);
		return -EBUSY;
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	touchkey_driver->early_suspend.suspend =
		(void *)sec_touchkey_early_suspend;
	touchkey_driver->early_suspend.resume =
		(void *)sec_touchkey_late_resume;
	register_early_suspend(&touchkey_driver->early_suspend);
#endif

	touchkey_led_ldo_on(1);

#if defined(CONFIG_MACH_C1_NA_SPR_EPIC2_REV00)\
	|| defined(CONFIG_MACH_Q1_BD) \
	|| defined(CONFIG_MACH_C1_NA_USCC_REV05)

	touchkey_firmware_update();
#endif

#if defined(CONFIG_TARGET_LOCALE_NAATT) \
|| defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_MACH_Q1_BD)
	/*touchkey_firmware_update(); */
#if defined(CONFIG_TARGET_LOCALE_NA)
	if (store_module_version >= 8) {
#endif
		msleep(100);
		err = touchkey_autocalibration();
		if (err < 0) {
			printk(KERN_ERR
			       "[TouchKey] probe autocalibration fail\n");
			return err;
		}
#if defined(CONFIG_TARGET_LOCALE_NA)
	}
#endif				/* CONFIG_TARGET_LOCALE_NA */
#endif
	set_touchkey_debug('K');
	return 0;
}

static void init_hw(void)
{
	gpio_direction_output(_3_GPIO_TOUCH_EN, 1);
	msleep(200);
	s3c_gpio_setpull(_3_GPIO_TOUCH_INT, S3C_GPIO_PULL_NONE);
	irq_set_irq_type(IRQ_TOUCH_INT, IRQF_TRIGGER_FALLING);
	s3c_gpio_cfgpin(_3_GPIO_TOUCH_INT, _3_GPIO_TOUCH_INT_AF);
}

static int get_touchkey_module_version()
{
	char data[3] = { 0, };
	int ret = 0;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 3);
	if (ret < 0) {
		printk(KERN_ERR "[TouchKey] module version read fail\n");
		return ret;
	} else {
		printk(KERN_DEBUG "[TouchKey] Module Version: %d\n", data[2]);
		return data[2];
	}
}

int touchkey_update_open(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t touchkey_update_read(struct file *filp, char *buf, size_t count,
			     loff_t *f_pos)
{
	char data[3] = { 0, };

	get_touchkey_firmware(data);
	put_user(data[1], buf);

	return 1;
}

int touchkey_update_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t touch_version_read(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	char data[3] = { 0, };
	int count;

	init_hw();
	i2c_touchkey_read(KEYCODE_REG, data, 3);

	count = sprintf(buf, "0x%x\n", data[1]);

	printk(KERN_DEBUG "[TouchKey] touch_version_read 0x%x\n", data[1]);
	printk(KERN_DEBUG "[TouchKey] module_version_read 0x%x\n", data[2]);

	return count;
}

static ssize_t touch_version_write(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t size)
{
	printk(KERN_DEBUG "[TouchKey] input data --> %s\n", buf);

	return size;
}

void touchkey_update_func(struct work_struct *p)
{
	int retry = 10;
#if defined(CONFIG_TARGET_LOCALE_NAATT)
	char data[3];
	i2c_touchkey_read(KEYCODE_REG, data, 3);
	printk(KERN_DEBUG "[%s] F/W version: 0x%x, Module version:0x%x\n",
	       __func__, data[1], data[2]);
#endif
	touchkey_update_status = 1;
	printk(KERN_DEBUG "[TouchKey] %s start\n", __func__);
	touchkey_enable = 0;
	while (retry--) {
		if (ISSP_main() == 0) {
			printk(KERN_DEBUG
			       "[TouchKey] touchkey_update succeeded\n");
			init_hw();
			enable_irq(IRQ_TOUCH_INT);
			touchkey_enable = 1;
#if defined(CONFIG_MACH_Q1_BD)
			touchkey_autocalibration();
#else
#if defined(CONFIG_TARGET_LOCALE_NA)
			if (store_module_version >= 8)
				touchkey_autocalibration();
#endif
#endif
			touchkey_update_status = 0;
			return;
		}
#if defined(CONFIG_TARGET_LOCALE_NAATT) \
|| defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_MACH_Q1_BD)
		touchkey_ldo_on(0);
		msleep(300);
		init_hw();
#endif
	}

	touchkey_update_status = -1;
	printk(KERN_DEBUG "[TouchKey] touchkey_update failed\n");
	return;
}

static ssize_t touch_update_write(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t size)
{
#ifdef CONFIG_TARGET_LOCALE_NA
	if (store_module_version < 8) {
		printk(KERN_DEBUG
		       "[TouchKey] Skipping f/w update : module_version =%d\n",
		       store_module_version);
		touchkey_update_status = 0;
		return 1;
	} else {
#endif				/* CONFIG_TARGET_LOCALE_NA */
		printk(KERN_DEBUG "[TouchKey] touchkey firmware update\n");

		if (*buf == 'S') {
			disable_irq(IRQ_TOUCH_INT);
			INIT_WORK(&touch_update_work, touchkey_update_func);
			queue_work(touchkey_wq, &touch_update_work);
		}
		return size;
#ifdef CONFIG_TARGET_LOCALE_NA
	}
#endif				/* CONFIG_TARGET_LOCALE_NA */
}

static ssize_t touch_update_read(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	int count = 0;

	printk(KERN_DEBUG
	       "[TouchKey] touch_update_read: touchkey_update_status %d\n",
	       touchkey_update_status);

	if (touchkey_update_status == 0)
		count = sprintf(buf, "PASS\n");
	else if (touchkey_update_status == 1)
		count = sprintf(buf, "Downloading\n");
	else if (touchkey_update_status == -1)
		count = sprintf(buf, "Fail\n");

	return count;
}

static ssize_t touch_led_control(struct device *dev,
				 struct device_attribute *attr, const char *buf,
				 size_t size)
{
	int data;
	int errnum;

	if (sscanf(buf, "%d\n", &data) == 1) {
#if defined(CONFIG_MACH_Q1_BD)
		if (data == 1)
			data = 0x10;
		else if (data == 2)
			data = 0x20;
#elif defined(CONFIG_TARGET_LOCALE_NA)
		if (store_module_version >= 8) {
			if (data == 1)
				data = 0x10;
			else if (data == 2)
				data = 0x20;
		}
#endif
		errnum = i2c_touchkey_write((u8 *) &data, 1);
		if (errnum == -ENODEV)
			touchled_cmd_reversed = 1;

		touchkey_led_status = data;
	} else {
		printk(KERN_DEBUG "[TouchKey] touch_led_control Error\n");
	}

	return size;
}

static ssize_t touchkey_enable_disable(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t size)
{
	return size;
}

#if defined(CONFIG_TARGET_LOCALE_NAATT) || defined(CONFIG_TARGET_LOCALE_NA)
static ssize_t touchkey_menu_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	u8 data[18] = { 0, };
	int ret;

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 18);
#ifdef CONFIG_TARGET_LOCALE_NA
	if (store_module_version < 8) {
		printk(KERN_DEBUG "called %s data[12] =%d,data[13] = %d\n",
		       __func__, data[12], data[13]);
		menu_sensitivity = ((0x00FF & data[12]) << 8) | data[13];
	} else {
		printk(KERN_DEBUG "called %s data[17] =%d\n", __func__,
		       data[17]);
		menu_sensitivity = data[17];
	}
#else
	printk(KERN_DEBUG "called %s data[10] =%d,data[11] = %d\n", __func__,
	       data[10], data[11]);
	menu_sensitivity = ((0x00FF & data[10]) << 8) | data[11];
#endif				/* CONFIG_TARGET_LOCALE_NA */
	return sprintf(buf, "%d\n", menu_sensitivity);
}

static ssize_t touchkey_home_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	u8 data[18] = { 0, };
	int ret;

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 18);
#ifdef CONFIG_TARGET_LOCALE_NA
	if (store_module_version < 8) {
		printk(KERN_DEBUG "called %s data[10] =%d,data[11] = %d\n",
		       __func__, data[10], data[11]);
		home_sensitivity = ((0x00FF & data[10]) << 8) | data[11];
	} else {
		printk(KERN_DEBUG "called %s data[15] =%d\n", __func__,
		       data[15]);
		home_sensitivity = data[15];
	}
#else
	printk(KERN_DEBUG "called %s data[12] =%d,data[13] = %d\n", __func__,
	       data[12], data[13]);
	home_sensitivity = ((0x00FF & data[12]) << 8) | data[13];
#endif				/* CONFIG_TARGET_LOCALE_NA */
	return sprintf(buf, "%d\n", home_sensitivity);
}

static ssize_t touchkey_back_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
	u8 data[18] = { 0, };
	int ret;

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 18);
#ifdef CONFIG_TARGET_LOCALE_NA
	if (store_module_version < 8) {
		printk(KERN_DEBUG "called %s data[8] =%d,data[9] = %d\n",
		       __func__, data[8], data[9]);
		back_sensitivity = ((0x00FF & data[8]) << 8) | data[9];
	} else {
		printk(KERN_DEBUG "called %s data[13] =%d\n", __func__,
		       data[13]);
		back_sensitivity = data[13];
	}
#else
	printk(KERN_DEBUG "called %s data[14] =%d,data[15] = %d\n", __func__,
	       data[14], data[15]);
	back_sensitivity = ((0x00FF & data[14]) << 8) | data[15];
#endif				/* CONFIG_TARGET_LOCALE_NA */
	return sprintf(buf, "%d\n", back_sensitivity);
}

static ssize_t touchkey_search_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	u8 data[18] = { 0, };
	int ret;

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 18);
#ifdef CONFIG_TARGET_LOCALE_NA
	if (store_module_version < 8) {
		printk(KERN_DEBUG "called %s data[6] =%d,data[7] = %d\n",
		       __func__, data[6], data[7]);
		search_sensitivity = ((0x00FF & data[6]) << 8) | data[7];
	} else {
		printk(KERN_DEBUG "called %s data[11] =%d\n", __func__,
		       data[11]);
		search_sensitivity = data[11];
	}
#else
	printk(KERN_DEBUG "called %s data[16] =%d,data[17] = %d\n", __func__,
	       data[16], data[17]);
	search_sensitivity = ((0x00FF & data[16]) << 8) | data[17];
#endif				/* CONFIG_TARGET_LOCALE_NA */
	return sprintf(buf, "%d\n", search_sensitivity);
}
#else
static ssize_t touchkey_menu_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
#if defined(CONFIG_MACH_Q1_BD)
	u8 data[14] = { 0, };
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 14);

	printk(KERN_DEBUG "called %s data[13] =%d\n", __func__, data[13]);
	menu_sensitivity = data[13];
#else
	u8 data[10];
	int ret;

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
	menu_sensitivity = data[7];
#endif
	return sprintf(buf, "%d\n", menu_sensitivity);
}

static ssize_t touchkey_back_show(struct device *dev,
				  struct device_attribute *attr, char *buf)
{
#if defined(CONFIG_MACH_Q1_BD)
	u8 data[14] = { 0, };
	int ret;

	ret = i2c_touchkey_read(KEYCODE_REG, data, 14);

	printk(KERN_DEBUG "called %s data[11] =%d\n", __func__, data[11]);
	back_sensitivity = data[11];
#else
	u8 data[10];
	int ret;

	printk(KERN_DEBUG "called %s\n", __func__);
	ret = i2c_touchkey_read(KEYCODE_REG, data, 10);
	back_sensitivity = data[9];
#endif
	return sprintf(buf, "%d\n", back_sensitivity);
}
#endif

#if defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_MACH_Q1_BD)
static ssize_t autocalibration_enable(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t size)
{
	int data;

	sscanf(buf, "%d\n", &data);

	if (data == 1)
		touchkey_autocalibration();

	return size;
}

static ssize_t autocalibration_status(struct device *dev,
				      struct device_attribute *attr, char *buf)
{
	u8 data[6];
	int ret;

	printk(KERN_DEBUG "called %s\n", __func__);

	ret = i2c_touchkey_read(KEYCODE_REG, data, 6);
	if ((data[5] & 0x80))
		return sprintf(buf, "Enabled\n");
	else
		return sprintf(buf, "Disabled\n");

}
#endif				/* CONFIG_TARGET_LOCALE_NA */

static ssize_t touch_sensitivity_control(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t size)
{
	unsigned char data = 0x40;
	i2c_touchkey_write(&data, 1);
	return size;
}

static ssize_t set_touchkey_firm_version_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
	return sprintf(buf, "0x%x\n", TK_FIRMWARE_VER);
}

static ssize_t set_touchkey_update_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	/* TO DO IT */
	int count = 0;
	int retry = 3;
	touchkey_update_status = 1;

#ifdef TEST_JIG_MODE
	unsigned char get_touch = 0x40;
#endif

	while (retry--) {
		if (ISSP_main() == 0) {
			printk(KERN_ERR
			       "[TOUCHKEY]Touchkey_update succeeded\n");
			touchkey_update_status = 0;
			count = 1;
			break;
		}
		printk(KERN_ERR "touchkey_update failed... retry...\n");
	}
	if (retry <= 0) {
		/* disable ldo11 */
		touchkey_ldo_on(0);
		msleep(300);
		count = 0;
		printk(KERN_ERR "[TOUCHKEY]Touchkey_update fail\n");
		touchkey_update_status = -1;
		return count;
	}

	init_hw();		/* after update, re initalize. */

#ifdef TEST_JIG_MODE
	i2c_touchkey_write(&get_touch, 1);
#endif

	return count;

}

static ssize_t set_touchkey_firm_version_read_show(struct device *dev,
						   struct device_attribute
						   *attr, char *buf)
{
	char data[3] = { 0, };
	int count;

	init_hw();
	/*if (get_touchkey_firmware(data) != 0) { */
	i2c_touchkey_read(KEYCODE_REG, data, 3);
	/*} */
	count = sprintf(buf, "0x%x\n", data[1]);

	printk(KERN_DEBUG "[TouchKey] touch_version_read 0x%x\n", data[1]);
	printk(KERN_DEBUG "[TouchKey] module_version_read 0x%x\n", data[2]);
	return count;
}

static ssize_t set_touchkey_firm_status_show(struct device *dev,
					     struct device_attribute *attr,
					     char *buf)
{
	int count = 0;

	printk(KERN_DEBUG
	       "[TouchKey] touch_update_read: touchkey_update_status %d\n",
	       touchkey_update_status);

	if (touchkey_update_status == 0)
		count = sprintf(buf, "PASS\n");
	else if (touchkey_update_status == 1)
		count = sprintf(buf, "Downloading\n");
	else if (touchkey_update_status == -1)
		count = sprintf(buf, "Fail\n");

	return count;
}

static DEVICE_ATTR(recommended_version, S_IRUGO | S_IWUSR | S_IWGRP,
		   touch_version_read, touch_version_write);
static DEVICE_ATTR(updated_version, S_IRUGO | S_IWUSR | S_IWGRP,
		   touch_update_read, touch_update_write);
static DEVICE_ATTR(brightness, S_IRUGO | S_IWUSR | S_IWGRP, NULL,
		   touch_led_control);
static DEVICE_ATTR(enable_disable, S_IRUGO | S_IWUSR | S_IWGRP, NULL,
		   touchkey_enable_disable);
static DEVICE_ATTR(touchkey_menu, S_IRUGO | S_IWUSR | S_IWGRP,
		   touchkey_menu_show, NULL);
static DEVICE_ATTR(touchkey_back, S_IRUGO | S_IWUSR | S_IWGRP,
		   touchkey_back_show, NULL);
#if defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_TARGET_LOCALE_NAATT)
static DEVICE_ATTR(touchkey_home, S_IRUGO, touchkey_home_show, NULL);
static DEVICE_ATTR(touchkey_search, S_IRUGO, touchkey_search_show, NULL);
#endif				/* CONFIG_TARGET_LOCALE_NA  */
static DEVICE_ATTR(touch_sensitivity, S_IRUGO | S_IWUSR | S_IWGRP, NULL,
		   touch_sensitivity_control);
/*20110223N1 firmware sync*/
static DEVICE_ATTR(touchkey_firm_update, S_IRUGO | S_IWUSR | S_IWGRP,
	set_touchkey_update_show, NULL);/* firmware update */
static DEVICE_ATTR(touchkey_firm_update_status, S_IRUGO | S_IWUSR | S_IWGRP,
	set_touchkey_firm_status_show, NULL);/* firmware update status */
static DEVICE_ATTR(touchkey_firm_version_phone, S_IRUGO | S_IWUSR | S_IWGRP,
	set_touchkey_firm_version_show, NULL);/* PHONE */
static DEVICE_ATTR(touchkey_firm_version_panel, S_IRUGO | S_IWUSR | S_IWGRP,
		   set_touchkey_firm_version_read_show, NULL);
/*PART*/
/*end N1 firmware sync*/
static DEVICE_ATTR(touchkey_brightness, S_IRUGO | S_IWUSR | S_IWGRP, NULL,
		   brightness_control);

#if defined(CONFIG_TARGET_LOCALE_NAATT)
static DEVICE_ATTR(touchkey_autocal_start, S_IRUGO | S_IWUSR | S_IWGRP, NULL,
		   set_touchkey_autocal_testmode);
#endif

#if defined(CONFIG_TARGET_LOCALE_NAATT) \
|| defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_MACH_Q1_BD)
static DEVICE_ATTR(touchkey_raw_data0, S_IRUGO, touchkey_raw_data0_show, NULL);
static DEVICE_ATTR(touchkey_raw_data1, S_IRUGO, touchkey_raw_data1_show, NULL);
static DEVICE_ATTR(touchkey_raw_data2, S_IRUGO, touchkey_raw_data2_show, NULL);
static DEVICE_ATTR(touchkey_raw_data3, S_IRUGO, touchkey_raw_data3_show, NULL);
static DEVICE_ATTR(touchkey_idac0, S_IRUGO, touchkey_idac0_show, NULL);
static DEVICE_ATTR(touchkey_idac1, S_IRUGO, touchkey_idac1_show, NULL);
static DEVICE_ATTR(touchkey_idac2, S_IRUGO, touchkey_idac2_show, NULL);
static DEVICE_ATTR(touchkey_idac3, S_IRUGO, touchkey_idac3_show, NULL);
static DEVICE_ATTR(touchkey_threshold, S_IRUGO, touchkey_threshold_show, NULL);
#endif

#if defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_MACH_Q1_BD)
static DEVICE_ATTR(autocal_enable, S_IRUGO | S_IWUSR | S_IWGRP, NULL,
		   autocalibration_enable);
static DEVICE_ATTR(autocal_stat, S_IRUGO | S_IWUSR | S_IWGRP,
		   autocalibration_status, NULL);
#endif				/* CONFIG_TARGET_LOCALE_NA */
static int __init touchkey_init(void)
{
	int ret = 0;

#ifdef TEST_JIG_MODE
	unsigned char get_touch = 0x40;
#endif

	sec_touchkey = device_create(sec_class, NULL, 0, NULL, "sec_touchkey");

	if (IS_ERR(sec_touchkey))
		printk(KERN_ERR "Failed to create device(sec_touchkey)!\n");

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_firm_update) <
	    0) {
		printk(KERN_ERR "Failed to create device file(%s)!\n",
		       dev_attr_touchkey_firm_update.attr.name);
	}
	if (device_create_file
	    (sec_touchkey, &dev_attr_touchkey_firm_update_status) < 0) {
		printk(KERN_ERR "Failed to create device file(%s)!\n",
		       dev_attr_touchkey_firm_update_status.attr.name);
	}
	if (device_create_file
	    (sec_touchkey, &dev_attr_touchkey_firm_version_phone) < 0) {
		printk(KERN_ERR "Failed to create device file(%s)!\n",
		       dev_attr_touchkey_firm_version_phone.attr.name);
	}
	if (device_create_file
	    (sec_touchkey, &dev_attr_touchkey_firm_version_panel) < 0) {
		printk(KERN_ERR "Failed to create device file(%s)!\n",
		       dev_attr_touchkey_firm_version_panel.attr.name);
	}
	if (device_create_file(sec_touchkey,
		&dev_attr_touchkey_brightness) < 0) {
		printk(KERN_ERR "Failed to create device file(%s)!\n",
		       dev_attr_touchkey_brightness.attr.name);
	}
#if defined(CONFIG_TARGET_LOCALE_NAATT)
	if (device_create_file(sec_touchkey,
		&dev_attr_touchkey_autocal_start) <
	    0) {
		printk(KERN_ERR "Failed to create device file(%s)!\n",
		       dev_attr_touchkey_brightness.attr.name);
	}
#endif

	if (device_create_file(sec_touchkey,
		&dev_attr_recommended_version) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_recommended_version.attr.name);
	}

	if (device_create_file(sec_touchkey,
		&dev_attr_updated_version) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_updated_version.attr.name);
	}

	if (device_create_file(sec_touchkey,
		&dev_attr_brightness) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_brightness.attr.name);
	}

	if (device_create_file(sec_touchkey,
		&dev_attr_enable_disable) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_enable_disable.attr.name);
	}

	if (device_create_file(sec_touchkey,
		&dev_attr_touchkey_menu) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touchkey_menu.attr.name);
	}

	if (device_create_file(sec_touchkey,
		&dev_attr_touchkey_back) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touchkey_back.attr.name);
	}
#if defined(CONFIG_TARGET_LOCALE_NAATT) \
|| defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_MACH_Q1_BD)
	if (device_create_file(sec_touchkey,
		&dev_attr_touchkey_raw_data0) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touchkey_raw_data0.attr.name);
	}

	if (device_create_file(sec_touchkey,
		&dev_attr_touchkey_raw_data1) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touchkey_raw_data1.attr.name);
	}

	if (device_create_file(sec_touchkey,
		&dev_attr_touchkey_raw_data2) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touchkey_raw_data2.attr.name);
	}

	if (device_create_file(sec_touchkey,
		&dev_attr_touchkey_raw_data3) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touchkey_raw_data3.attr.name);
	}

	if (device_create_file(sec_touchkey,
		&dev_attr_touchkey_idac0) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touchkey_idac0.attr.name);
	}

	if (device_create_file(sec_touchkey,
		&dev_attr_touchkey_idac1) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touchkey_idac1.attr.name);
	}

	if (device_create_file(sec_touchkey,
		&dev_attr_touchkey_idac2) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touchkey_idac2.attr.name);
	}

	if (device_create_file(sec_touchkey,
		&dev_attr_touchkey_idac3) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touchkey_idac3.attr.name);
	}

	if (device_create_file(sec_touchkey,
		&dev_attr_touchkey_threshold) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touchkey_threshold.attr.name);
	}
#endif

#if defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_TARGET_LOCALE_NAATT)
	if (device_create_file(sec_touchkey, &dev_attr_touchkey_home) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touchkey_home.attr.name);
	}

	if (device_create_file(sec_touchkey, &dev_attr_touchkey_search) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touchkey_search.attr.name);
	}
#endif				/* CONFIG_TARGET_LOCALE_NA  */

#if defined(CONFIG_TARGET_LOCALE_NA) || defined(CONFIG_MACH_Q1_BD)
	if (device_create_file(sec_touchkey, &dev_attr_autocal_enable) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_autocal_enable.attr.name);
	}

	if (device_create_file(sec_touchkey, &dev_attr_autocal_stat) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_autocal_stat.attr.name);
	}
#endif				/* CONFIG_TARGET_LOCALE_NA */

	if (device_create_file(sec_touchkey,
		&dev_attr_touch_sensitivity) < 0) {
		pr_err("Failed to create device file(%s)!\n",
		       dev_attr_touch_sensitivity.attr.name);
	}
	touchkey_wq = create_singlethread_workqueue("sec_touchkey_wq");
	if (!touchkey_wq)
		return -ENOMEM;

	INIT_WORK(&touchkey_work, touchkey_work_func);

	init_hw();

	ret = i2c_add_driver(&touchkey_i2c_driver);

	if (ret) {
		printk(KERN_ERR
	       "[TouchKey] registration failed, module not inserted.ret= %d\n",
	       ret);
	}
#ifdef TEST_JIG_MODE
	i2c_touchkey_write(&get_touch, 1);
#endif
	return ret;

}

static void __exit touchkey_exit(void)
{
	printk(KERN_DEBUG "[TouchKey] %s\n", __func__);
	i2c_del_driver(&touchkey_i2c_driver);

	if (touchkey_wq)
		destroy_workqueue(touchkey_wq);

#ifndef CONFIG_MACH_Q1_BD
	gpio_free(_3_TOUCH_SDA_28V);
	gpio_free(_3_TOUCH_SCL_28V);
	gpio_free(_3_GPIO_TOUCH_EN);
#endif
	gpio_free(_3_GPIO_TOUCH_INT);
}

late_initcall(touchkey_init);
module_exit(touchkey_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("@@@");
MODULE_DESCRIPTION("touch keypad");
