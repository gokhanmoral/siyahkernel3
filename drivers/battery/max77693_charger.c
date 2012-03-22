/*
 * max77693_charger.c
 *
 * Copyright (C) 2011 Samsung Electronics
 * SangYoung Son <hello.son@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/battery/samsung_battery.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/mfd/max77693.h>
#include <linux/mfd/max77693-private.h>
#include <linux/power/charger-manager.h>
#ifdef CONFIG_USB_HOST_NOTIFY
#include <linux/host_notify.h>
#include <plat/devs.h>
#endif

/* MAX77693 Registers(defined @max77693-private.h) */

/* MAX77693_CHG_REG_CHG_INT */
#define MAX77693_BYP_I			(1 << 0)
#define MAX77693_THM_I			(1 << 2)
#define MAX77693_BAT_I			(1 << 3)
#define MAX77693_CHG_I			(1 << 4)
#define MAX77693_CHGIN_I		(1 << 6)

/* MAX77693_CHG_REG_CHG_INT_MASK */
#define MAX77693_BYP_IM			(1 << 0)
#define MAX77693_THM_IM			(1 << 2)
#define MAX77693_BAT_IM			(1 << 3)
#define MAX77693_CHG_IM			(1 << 4)
#define MAX77693_CHGIN_IM		(1 << 6)

/* MAX77693_CHG_REG_CHG_INT_OK */
#define MAX77693_BYP_OK			0x01
#define MAX77693_BYP_OK_SHIFT		0
#define MAX77693_THM_OK			0x04
#define MAX77693_THM_OK_SHIFT		2
#define MAX77693_BAT_OK			0x08
#define MAX77693_BAT_OK_SHIFT		3
#define MAX77693_CHG_OK			0x10
#define MAX77693_CHG_OK_SHIFT		4
#define MAX77693_CHGIN_OK		0x40
#define MAX77693_CHGIN_OK_SHIFT		6
#define MAX77693_DETBAT			0x80
#define MAX77693_DETBAT_SHIFT		7

/* MAX77693_CHG_REG_CHG_DTLS_00 */
#define MAX77693_THM_DTLS		0x07
#define MAX77693_THM_DTLS_SHIFT		0
#define MAX77693_CHGIN_DTLS		0x60
#define MAX77693_CHGIN_DTLS_SHIFT	5

/* MAX77693_CHG_REG_CHG_DTLS_01 */
#define MAX77693_CHG_DTLS		0x0F
#define MAX77693_CHG_DTLS_SHIFT		0
#define MAX77693_BAT_DTLS		0x70
#define MAX77693_BAT_DTLS_SHIFT		4

/* MAX77693_CHG_REG_CHG_DTLS_02 */
#define MAX77693_BYP_DTLS		0x0F
#define MAX77693_BYP_DTLS_SHIFT		0
#define MAX77693_BYP_DTLS0	0x1
#define MAX77693_BYP_DTLS1	0x2
#define MAX77693_BYP_DTLS2	0x4
#define MAX77693_BYP_DTLS3	0x8

/* MAX77693_CHG_REG_CHG_CNFG_00 */
#define MAX77693_MODE_DEFAULT	0x04
#define MAX77693_MODE_CHG_ON	0x05
#define MAX77693_MODE_OTG_ON	0x0A

/* MAX77693_CHG_REG_CHG_CNFG_02 */
#define MAX77693_CHG_CC		0x3F

/* MAX77693_CHG_REG_CHG_CNFG_09 */
#define MAX77693_CHG_CHGIN_LIM	0x7F

/* MAX77693_MUIC_REG_STATUS2 */
#define MAX77693_CHGDETRUN		0x08
#define MAX77693_CHGDETRUN_SHIFT	3
#define MAX77693_CHGTYPE		0x07
#define MAX77693_CHGTYPE_SHIFT		0

struct max77693_charger_data {
	struct max77693_dev	*max77693;

	struct power_supply	charger;

	struct delayed_work	update_work;

	unsigned int	charging_state;
	unsigned int	charging_type;
	unsigned int	battery_state;
	unsigned int	battery_present;
	unsigned int	cable_type;
	unsigned int	charging_current;
	unsigned int	vbus_state;

	int		irq_bypass;
	int		irq_therm;
	int		irq_battery;
	int		irq_charge;
	int		irq_chargin;

	/* software regulation */
	bool		soft_reg_state;
	unsigned int	soft_reg_current;

#ifdef CONFIG_BATTERY_WPC_CHARGER
	int		wpc_gpio;
	int		wpc_irq;
	int		wpc_state;
#endif

	/* Will be move to pdata */
	unsigned int	usb_charge_current;
	unsigned int	ac_charge_current;
	unsigned int	misc_charge_current;
	unsigned int	recharge_voltage;

	struct max77693_charger_platform_data	*charger_pdata;

	int		irq;
	u8		irq_reg;
};

static void max77693_charger_test_read(struct max77693_charger_data *chg_data)
{
	struct i2c_client *i2c = chg_data->max77693->i2c;
	u8 reg_data;
	u32 reg_addr;
	pr_info("%s\n", __func__);

	for (reg_addr = 0xB0; reg_addr <= 0xC5; reg_addr++) {
		max77693_read_reg(i2c, reg_addr, &reg_data);
		pr_info("max77693 charger 0x%02x(0x%02x)\n",
					reg_addr, reg_data);
	}
}

static int max77693_get_battery_present(struct max77693_charger_data *chg_data)
{
	struct i2c_client *i2c = chg_data->max77693->i2c;
	u8 reg_data;
	pr_debug("%s\n", __func__);

	max77693_read_reg(i2c, MAX77693_CHG_REG_CHG_INT_OK, &reg_data);
	pr_debug("%s: CHG_INT_OK(0x%02x)\n", __func__,
								reg_data);
	reg_data = ((reg_data & MAX77693_DETBAT) >> MAX77693_DETBAT_SHIFT);

	return !reg_data;
}

static int max77693_get_vbus_state(struct max77693_charger_data *chg_data)
{
	struct i2c_client *i2c = chg_data->max77693->i2c;
	int state;
	u8 reg_data;
	pr_debug("%s\n", __func__);

	max77693_read_reg(i2c, MAX77693_CHG_REG_CHG_DTLS_00, &reg_data);
	reg_data = ((reg_data & MAX77693_CHGIN_DTLS) >>
				MAX77693_CHGIN_DTLS_SHIFT);
	pr_debug("%s: CHGIN_DTLS(0x%02x)\n", __func__, reg_data);

	switch (reg_data) {
	case 0x00:
		state = POWER_SUPPLY_VBUS_UVLO;
		break;
	case 0x01:
		state = POWER_SUPPLY_VBUS_WEAK;
		break;
	case 0x02:
		state = POWER_SUPPLY_VBUS_OVLO;
		break;
	case 0x03:
		state = POWER_SUPPLY_VBUS_GOOD;
		break;
	default:
		state = POWER_SUPPLY_VBUS_UNKNOWN;
		break;
	}

	chg_data->vbus_state = state;
	return state;
}

static int max77693_get_battery_state(struct max77693_charger_data *chg_data)
{
	struct i2c_client *i2c = chg_data->max77693->i2c;
	int state;
	int vbus_state;
	u8 reg_data;
	pr_debug("%s\n", __func__);

	max77693_read_reg(i2c, MAX77693_CHG_REG_CHG_DTLS_01, &reg_data);
	reg_data = ((reg_data & MAX77693_BAT_DTLS) >> MAX77693_BAT_DTLS_SHIFT);
	pr_debug("%s: BAT_DTLS(0x%02x)\n", __func__, reg_data);

	switch (reg_data) {
	case 0x03:
	case 0x04:
		state = POWER_SUPPLY_HEALTH_GOOD;
		/* VBUS OVP state return battery OVP state */
		vbus_state = max77693_get_vbus_state(chg_data);
		if (vbus_state == POWER_SUPPLY_VBUS_OVLO) {
			pr_info("%s: vbus ovp\n", __func__);
			state = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
		} else
			pr_debug("%s: vbus not ovp\n", __func__);
		break;
	case 0x02:
		state = POWER_SUPPLY_HEALTH_DEAD;
		break;
	case 0x05:
		state = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
		break;
	default:
		state = POWER_SUPPLY_HEALTH_UNKNOWN;
		break;
	}

	chg_data->battery_state = state;
	return state;
}

static int max77693_get_charging_type(struct max77693_charger_data *chg_data)
{
	struct i2c_client *i2c = chg_data->max77693->i2c;
	int state;
	u8 reg_data;
	pr_debug("%s\n", __func__);

	max77693_read_reg(i2c, MAX77693_CHG_REG_CHG_DTLS_01, &reg_data);
	reg_data = ((reg_data & MAX77693_CHG_DTLS) >> MAX77693_CHG_DTLS_SHIFT);
	pr_debug("%s: CHG_DTLS(0x%02x)\n", __func__, reg_data);

	switch (reg_data) {
	case 0x0:
		state = POWER_SUPPLY_CHARGE_TYPE_TRICKLE;
		break;
	case 0x1:
	case 0x2:
	case 0x3:
		state = POWER_SUPPLY_CHARGE_TYPE_FAST;
		break;
	case 0x4:
	case 0x8:
	case 0xA:
	case 0xB:
		state = POWER_SUPPLY_CHARGE_TYPE_NONE;
		break;
	default:
		state = POWER_SUPPLY_CHARGE_TYPE_UNKNOWN;
		break;
	}

	chg_data->charging_type = state;
	return state;
}

int max77693_get_charging_current(struct max77693_charger_data *chg_data)
{
	struct i2c_client *i2c = chg_data->max77693->i2c;
	u8 reg_data;
	int get_current = 0;
	pr_debug("%s\n", __func__);

	max77693_read_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_02, &reg_data);
	pr_debug("%s: CHG_CNFG_02(0x%02x)\n", __func__, reg_data);

	reg_data &= MAX77693_CHG_CC;
	get_current = chg_data->charging_current = reg_data * 333 / 10;

	pr_debug("%s: Get charging current as %dmA.\n", __func__, get_current);
	return get_current;
}

void max77693_set_charging_current(struct max77693_charger_data *chg_data,
						unsigned int set_current)
{
	struct i2c_client *i2c = chg_data->max77693->i2c;
	u8 reg_data;
	pr_debug("%s: Set charging current as %dmA.\n", __func__, set_current);

	if (set_current == CHARGER_OFF_CURRENT) {
		pr_debug("%s: exit soft regulation loop\n", __func__);
		chg_data->soft_reg_state = false;
	}

	max77693_read_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_02, &reg_data);

	reg_data &= ~MAX77693_CHG_CC;
	reg_data |= ((set_current * 3 / 100) << 0);
	pr_debug("%s: reg_data(0x%02x)\n", __func__, reg_data);

	max77693_write_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_02, reg_data);

	/* Set input current limit */
	if (chg_data->soft_reg_state) {
		pr_info("%s: now in soft regulation loop: %d\n", __func__,
						chg_data->soft_reg_current);
		reg_data = (chg_data->soft_reg_current / 20);
	} else
		reg_data = (set_current / 20);
	pr_debug("%s: reg_data(0x%02x)\n", __func__, reg_data);
	max77693_write_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_09, reg_data);
}

static int max77693_get_charging_state(struct max77693_charger_data *chg_data)
{
	struct i2c_client *i2c = chg_data->max77693->i2c;
	int state;
	u8 reg_data;
	pr_debug("%s\n", __func__);

	max77693_read_reg(i2c, MAX77693_CHG_REG_CHG_DTLS_01, &reg_data);
	reg_data = ((reg_data & MAX77693_CHG_DTLS) >> MAX77693_CHG_DTLS_SHIFT);
	pr_debug("%s: CHG_DTLS(0x%02x)\n", __func__, reg_data);

	switch (reg_data) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
		state = POWER_SUPPLY_STATUS_CHARGING;
		break;
	case 0x4:
		state = POWER_SUPPLY_STATUS_FULL;
		break;
	case 0x5:
	case 0x6:
	case 0x7:
		state = POWER_SUPPLY_STATUS_NOT_CHARGING;
		break;
	case 0x8:
	case 0xA:
	case 0xB:
		state = POWER_SUPPLY_STATUS_DISCHARGING;
		break;
	default:
		state = POWER_SUPPLY_STATUS_UNKNOWN;
		break;
	}

	chg_data->charging_state = state;
	return state;
}

static void max77693_set_charging_state(struct max77693_charger_data *chg_data,
							int enable)
{
	struct i2c_client *i2c = chg_data->max77693->i2c;
	u8 reg_data;
	pr_debug("%s: enable=%d\n", __func__, enable);

	max77693_read_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_00, &reg_data);

	if (enable)
		reg_data |= 0x01;
	else
		reg_data &= ~0x01;

	pr_debug("%s: reg_data(0x%02x)\n", __func__, reg_data);
	max77693_write_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_00, reg_data);
}

static int max77693_get_cable_type(struct max77693_charger_data *chg_data)
{
	int state;
	u8 reg_data;
	u8 adc1k;
	int chgdetrun;
	int retry_cnt = 0;
	pr_debug("%s\n", __func__);

	do {
		max77693_read_reg(chg_data->max77693->muic,
				  MAX77693_MUIC_REG_STATUS2, &reg_data);
		pr_debug("%s: MAX77693_MUIC_REG_STATUS2(0x%02x)\n", __func__,
			 reg_data);
		chgdetrun = ((reg_data & MAX77693_CHGDETRUN) >>
			     MAX77693_CHGDETRUN_SHIFT);

		if (chgdetrun == 0x1) {
			retry_cnt++;
			pr_info("%s: Charger detection running(0x%02x), "
				"retry(%d)\n", __func__, reg_data, retry_cnt);
			msleep(100);
		}
	} while ((chgdetrun == 0x1) && (retry_cnt <= 10));

	reg_data &= MAX77693_CHGTYPE;	/* MUIC ChgTyp */

	switch (reg_data) {
	case 0x0:		/* Noting attached */
		state = POWER_SUPPLY_TYPE_BATTERY;
#ifdef CONFIG_BATTERY_WPC_CHARGER
	chg_data->wpc_state = !gpio_get_value(chg_data->wpc_gpio);
	if (chg_data->wpc_state == 1)
		state = POWER_SUPPLY_TYPE_WIRELESS;
	pr_debug("%s: wpc charger is %s state\n", __func__,
		(chg_data->wpc_state ? "enabled" : "disabled"));
#endif
		break;
	case 0x1:		/* USB cabled */
	case 0x4:		/* Apple 500mA charger */
		state = POWER_SUPPLY_TYPE_USB;
		break;
	case 0x2:		/* Charging downstream port */
		state = POWER_SUPPLY_TYPE_USB_CDP;
		break;
	case 0x3:		/* Dedicated charger(up to 1.5A) */
	case 0x5:		/* Apple 1A or 2A charger */
	case 0x6:		/* Special charger */
		state = POWER_SUPPLY_TYPE_MAINS;
		break;
	default:
		state = POWER_SUPPLY_TYPE_BATTERY;
		break;
	}

	/* WORKAROUND: If OTG enabled, skip detecting charger cable */
	/* So mhl cable does not have adc ID that below condition   */
	/* can`t include adc1k mask */
	max77693_read_reg(chg_data->max77693->muic,
			  MAX77693_MUIC_REG_STATUS1, &reg_data);
	pr_debug("%s: MAX77693_MUIC_REG_STATUS1(0x%02x)\n", __func__,
		 reg_data);
	adc1k = reg_data & (0x1 << 7); /* STATUS1_ADC1K_MASK */
	reg_data &= 0x1F;
	if (reg_data == 0x00 && !adc1k) {
		pr_info("%s: otg enabled(0x%02x)\n", __func__,
			 reg_data);
		state = POWER_SUPPLY_TYPE_BATTERY;
	}

	chg_data->cable_type = state;
	return state;
}

static void max77693_charger_reg_init(struct max77693_charger_data *chg_data)
{
	struct i2c_client *i2c = chg_data->max77693->i2c;
	u8 reg_data;
	pr_debug("%s\n", __func__);

	/* unlock charger setting protect */
	reg_data = (0x03 << 2);
	max77693_write_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_06, reg_data);

	/*
	 * fast charge timer 10hrs
	 * restart threshold disable
	 * pre-qual charge enable(default)
	 */
	reg_data = (0x04 << 0) | (0x03 << 4);
	max77693_write_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_01, reg_data);

	/*
	 * charge current 466mA(default)
	 * otg current limit 900mA
	 */
	reg_data = (1 << 7);
	max77693_write_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_02, reg_data);

	/*
	 * top off current 100mA
	 * top off timer 0min
	 */
	if (chg_data->max77693->pmic_rev == MAX77693_REV_PASS1)
		reg_data = (0x03 << 0);	/* 125mA */
	else
		reg_data = (0x03 << 0);	/* 175mA */
	reg_data |= (0x00 << 3);
	max77693_write_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_03, reg_data);

	/*
	 * cv voltage 4.2V or 4.35V
	 * MINVSYS 3.6V(default)
	 */
	if ((system_rev != 3) && (system_rev >= 1))
		reg_data = (0x1D << 0);
	else
		reg_data = (0x16 << 0);
	pr_info("%s: battery cv voltage %s, (sysrev %d)\n", __func__,
		((reg_data == (0x1D << 0)) ? "4.35V" : "4.2V"), system_rev);
	max77693_write_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_04, reg_data);

	/* VBYPSET 5V */
	reg_data = 0x50;
	max77693_write_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_11, reg_data);

	max77693_charger_test_read(chg_data);
}

static void max77693_soft_regulation(struct max77693_charger_data *chg_data)
{
	struct i2c_client *i2c = chg_data->max77693->i2c;
	u8 reg_data;
	pr_info("%s\n", __func__);

	/* enable soft regulation loop */
	chg_data->soft_reg_state = true;

	max77693_read_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_09, &reg_data);
	reg_data &= MAX77693_CHG_CHGIN_LIM;
	chg_data->soft_reg_current = reg_data * 20;
	chg_data->soft_reg_current -= 100;
	if (chg_data->soft_reg_current < 0)
		chg_data->soft_reg_current = 0;
	pr_info("%s: %dmA to %dmA\n", __func__,
			reg_data * 20, chg_data->soft_reg_current);

	reg_data = (chg_data->soft_reg_current / 20);
	pr_debug("%s: reg_data(0x%02x)\n", __func__, reg_data);
	max77693_write_reg(i2c, MAX77693_CHG_REG_CHG_CNFG_09, reg_data);
}

static void max77693_update_work(struct work_struct *work)
{
	struct max77693_charger_data *chg_data = container_of(work,
						struct max77693_charger_data,
						update_work.work);
	struct power_supply *battery_psy = power_supply_get_by_name("battery");
	union power_supply_propval value;
	int vbus_state;

	if (!battery_psy) {
		pr_err("%s: fail to get battery power supply\n", __func__);
		return;
	}

#if defined(CONFIG_CHARGER_MANAGER)
	/* Notify charger-manager */
	enum cm_event_types type;

	/* only consider battery in/out and external power in/out */
	/* It seems that charger interrupt does not work at all */
	switch (chg_data->irq - chg_data->max77693->irq_base) {
	case MAX77693_CHG_IRQ_BAT_I:
		type = max77693_get_battery_present(chg_data) ?
			CM_EVENT_BATT_IN : CM_EVENT_BATT_OUT;
		cm_notify_event(&chg_data->charger, type, NULL);
		break;
	case MAX77693_CHG_IRQ_CHGIN_I:
		cm_notify_event(&chg_data->charger,
			CM_EVENT_EXT_PWR_IN_OUT, NULL);
		break;
	default:
		break;
	}
#else
	switch (chg_data->irq - chg_data->max77693->irq_base) {
	case MAX77693_CHG_IRQ_CHGIN_I:
		vbus_state = max77693_get_vbus_state(chg_data);
		if (vbus_state == POWER_SUPPLY_VBUS_WEAK) {
			pr_info("%s: vbus weak\n", __func__);
			max77693_soft_regulation(chg_data);
		} else
			pr_debug("%s: vbus not weak\n", __func__);
		break;
	default:
		break;

	}

	battery_psy->set_property(battery_psy,
				POWER_SUPPLY_PROP_STATUS,
				&value);
#endif
}

/* Support property from charger */
static enum power_supply_property max77693_charger_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_CHARGE_TYPE,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_CURRENT_NOW
};

static int max77693_charger_get_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    union power_supply_propval *val)
{
	struct max77693_charger_data *chg_data = container_of(psy,
						  struct max77693_charger_data,
						  charger);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		val->intval = max77693_get_charging_state(chg_data);
		break;
	case POWER_SUPPLY_PROP_CHARGE_TYPE:
		val->intval = max77693_get_charging_type(chg_data);
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = max77693_get_battery_state(chg_data);
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = max77693_get_battery_present(chg_data);
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = max77693_get_cable_type(chg_data);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = max77693_get_charging_current(chg_data);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int max77693_charger_set_property(struct power_supply *psy,
			    enum power_supply_property psp,
			    const union power_supply_propval *val)
{
	struct max77693_charger_data *chg_data = container_of(psy,
						  struct max77693_charger_data,
						  charger);


	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		max77693_set_charging_state(chg_data, val->intval);
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		max77693_set_charging_current(chg_data, val->intval);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static irqreturn_t max77693_bypass_irq(int irq, void *data)
{
	struct max77693_charger_data *chg_data = data;
	u8 int_ok, dtls_02;
	u8 byp_dtls;
#ifdef CONFIG_USB_HOST_NOTIFY
	struct host_notifier_platform_data *host_noti_pdata =
			host_notifier_device.dev.platform_data;
#endif
	pr_info("%s: irq(%d)\n", __func__, irq);

	max77693_read_reg(chg_data->max77693->i2c,
				MAX77693_CHG_REG_CHG_INT_OK,
				&int_ok);

	max77693_read_reg(chg_data->max77693->i2c,
				MAX77693_CHG_REG_CHG_DTLS_02,
				&dtls_02);

	byp_dtls = ((dtls_02 & MAX77693_BYP_DTLS) >>
				MAX77693_BYP_DTLS_SHIFT);
	pr_info("%s: INT_OK(0x%02x), BYP_DTLS(0x%02x)\n",
				__func__, int_ok, byp_dtls);

	switch (byp_dtls) {
	case 0x0:
		pr_info("%s: bypass node is okay\n", __func__);
		break;
	case 0x1:
		pr_err("%s: bypass overcurrent limit\n", __func__);
#ifdef CONFIG_USB_HOST_NOTIFY
		host_state_notify(&host_noti_pdata->ndev,
					NOTIFY_HOST_OVERCURRENT);
#endif
		max77693_write_reg(chg_data->max77693->i2c,
					MAX77693_CHG_REG_CHG_CNFG_00,
					MAX77693_MODE_DEFAULT);
		break;
	case 0x8:
		pr_err("%s: chgin regulation loop is active\n", __func__);
		break;
	default:
		pr_info("%s: bypass reserved\n", __func__);
		break;
	}

	schedule_delayed_work(&chg_data->update_work, msecs_to_jiffies(100));

	return IRQ_HANDLED;
}

static irqreturn_t max77693_charger_irq(int irq, void *data)
{
	struct max77693_charger_data *chg_data = data;
	u8 int_ok, dtls_00, dtls_01;
	u8 thm_dtls, chgin_dtls, chg_dtls, bat_dtls;
	pr_info("%s: irq(%d)\n", __func__, irq);

	max77693_read_reg(chg_data->max77693->i2c,
				MAX77693_CHG_REG_CHG_INT_OK,
				&int_ok);

	max77693_read_reg(chg_data->max77693->i2c,
				MAX77693_CHG_REG_CHG_DTLS_00,
				&dtls_00);

	max77693_read_reg(chg_data->max77693->i2c,
				MAX77693_CHG_REG_CHG_DTLS_01,
				&dtls_01);

	thm_dtls = ((dtls_00 & MAX77693_THM_DTLS) >>
				MAX77693_THM_DTLS_SHIFT);
	chgin_dtls = ((dtls_00 & MAX77693_CHGIN_DTLS) >>
				MAX77693_CHGIN_DTLS_SHIFT);
	chg_dtls = ((dtls_01 & MAX77693_CHG_DTLS) >>
				MAX77693_CHG_DTLS_SHIFT);
	bat_dtls = ((dtls_01 & MAX77693_BAT_DTLS) >>
				MAX77693_BAT_DTLS_SHIFT);

	pr_info("%s: INT_OK(0x%x), THM(0x%x), CHGIN(0x%x), CHG(0x%x), BAT(0x%x)\n",
		__func__, int_ok, thm_dtls, chgin_dtls, chg_dtls, bat_dtls);

	chg_data->irq = irq;

	schedule_delayed_work(&chg_data->update_work, msecs_to_jiffies(100));

	return IRQ_HANDLED;
}

static void max77693_charger_initialize(struct max77693_charger_data *chg_data)
{
	struct max77693_charger_platform_data *charger_pdata =
					chg_data->charger_pdata;
	struct i2c_client *i2c = chg_data->max77693->i2c;
	int i;

	for (i = 0; i < charger_pdata->num_init_data; i++)
		max77693_write_reg(i2c, charger_pdata->init_data[i].addr,
				charger_pdata->init_data[i].data);
}

static __devinit int max77693_charger_probe(struct platform_device *pdev)
{
	struct max77693_charger_data *chg_data;
	struct max77693_dev *max77693 = dev_get_drvdata(pdev->dev.parent);
	struct max77693_platform_data *pdata = dev_get_platdata(max77693->dev);
	int ret;

	pr_info("%s: charger init start\n", __func__);

	chg_data = kzalloc(sizeof(struct max77693_charger_data), GFP_KERNEL);
	if (!chg_data)
		return -ENOMEM;

	platform_set_drvdata(pdev, chg_data);
	chg_data->max77693 = max77693;

	chg_data->charger_pdata = pdata->charger_data;
	if (!pdata->charger_data->init_data)
		max77693_charger_reg_init(chg_data);
	else
		max77693_charger_initialize(chg_data);

	chg_data->irq_bypass = max77693->irq_base + MAX77693_CHG_IRQ_BYP_I;
	chg_data->irq_therm = max77693->irq_base + MAX77693_CHG_IRQ_THM_I;
	chg_data->irq_battery = max77693->irq_base + MAX77693_CHG_IRQ_BAT_I;
	chg_data->irq_charge = max77693->irq_base + MAX77693_CHG_IRQ_CHG_I;
	chg_data->irq_chargin = max77693->irq_base + MAX77693_CHG_IRQ_CHGIN_I;

	chg_data->charger.name = "max77693-charger",
	chg_data->charger.type = POWER_SUPPLY_TYPE_BATTERY,
	chg_data->charger.properties = max77693_charger_props,
	chg_data->charger.num_properties = ARRAY_SIZE(max77693_charger_props),
	chg_data->charger.get_property = max77693_charger_get_property,
	chg_data->charger.set_property = max77693_charger_set_property,

	ret = power_supply_register(&pdev->dev, &chg_data->charger);
	if (ret) {
		pr_err("%s: failed: power supply register\n", __func__);
		goto err_kfree;
	}

	INIT_DELAYED_WORK_DEFERRABLE(&chg_data->update_work,
					max77693_update_work);

	ret = request_threaded_irq(chg_data->irq_bypass, NULL,
			max77693_bypass_irq, 0, "bypass-irq", chg_data);
	if (ret < 0)
		pr_err("%s: fail to request bypass IRQ: %d: %d\n",
				__func__, chg_data->irq_bypass, ret);

	ret = request_threaded_irq(chg_data->irq_battery, NULL,
			max77693_charger_irq, 0, "battery-irq", chg_data);
	if (ret < 0)
		pr_err("%s: fail to request battery IRQ: %d: %d\n",
				__func__, chg_data->irq_battery, ret);

	ret = request_threaded_irq(chg_data->irq_charge, NULL,
			max77693_charger_irq, 0, "charge-irq", chg_data);
	if (ret < 0)
		pr_err("%s: fail to request charge IRQ: %d: %d\n",
				__func__, chg_data->irq_charge, ret);

	ret = request_threaded_irq(chg_data->irq_chargin, NULL,
			max77693_charger_irq, 0, "chargin-irq", chg_data);
	if (ret < 0)
		pr_err("%s: fail to request chargin IRQ: %d: %d\n",
				__func__, chg_data->irq_chargin, ret);

#ifdef CONFIG_BATTERY_WPC_CHARGER
	if (!chg_data->charger_pdata->wpc_irq_gpio) {
		pr_err("%s: no irq gpio, do not support wpc\n", __func__);
		goto wpc_init_fail;
	}

	chg_data->wpc_gpio = chg_data->charger_pdata->wpc_irq_gpio;
	chg_data->wpc_irq = gpio_to_irq(chg_data->wpc_gpio);
	ret = gpio_request(chg_data->wpc_gpio, "wpc_charger-irq");
	if (ret < 0) {
		pr_err("%s: failed requesting gpio %d\n", __func__,
				chg_data->wpc_gpio);
		goto wpc_init_fail;
	}
	gpio_direction_input(chg_data->wpc_gpio);
	gpio_free(chg_data->wpc_gpio);

	ret = request_irq(chg_data->wpc_irq, max77693_charger_irq,
			IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING |
			IRQF_ONESHOT,
			"wpc-int", chg_data);
	if (ret)
		pr_err("Can NOT request irq 'WPC_INT' %d",
		       chg_data->wpc_irq);

	chg_data->wpc_state = !gpio_get_value(chg_data->wpc_gpio);
	pr_info("wpc charger initialized(%s state)\n",
		(chg_data->wpc_state ? "enabled" : "disabled"));
wpc_init_fail:
#endif

	pr_info("%s: charger init complete\n", __func__);

	return 0;

err_kfree:
	kfree(chg_data);
	return ret;
}

static int __devexit max77693_charger_remove(struct platform_device *pdev)
{
	struct max77693_charger_data *chg_data = platform_get_drvdata(pdev);

	power_supply_unregister(&chg_data->charger);
	kfree(chg_data);

	return 0;
}

/*
 * WORKAROUND:
 * Several interrupts occur while charging through TA.
 * Suspended state cannot be maintained by the interrupts.
 */
#ifdef CONFIG_SLP
static u8 saved_int_mask;
static int max77693_charger_suspend(struct device *dev)
{
	struct max77693_dev *max77693 = dev_get_drvdata(dev->parent);
	u8 int_mask;

	/* Save the masking value */
	max77693_read_reg(max77693->i2c,
			MAX77693_CHG_REG_CHG_INT_MASK,
			&saved_int_mask);

	/* Mask all the interrupts related to charger */
	int_mask = 0xff;
	max77693_write_reg(max77693->i2c,
			MAX77693_CHG_REG_CHG_INT_MASK,
			int_mask);
	return 0;
}

static int max77693_charger_resume(struct device *dev)
{
	struct max77693_dev *max77693 = dev_get_drvdata(dev->parent);

	/* Restore the saved masking value */
	max77693_write_reg(max77693->i2c,
			MAX77693_CHG_REG_CHG_INT_MASK,
			saved_int_mask);
	return 0;
}

static SIMPLE_DEV_PM_OPS(max77693_charger_pm_ops, max77693_charger_suspend,
			max77693_charger_resume);
#endif

static struct platform_driver max77693_charger_driver = {
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "max77693-charger",
#ifdef CONFIG_SLP
		.pm	= &max77693_charger_pm_ops,
#endif
	},
	.probe		= max77693_charger_probe,
	.remove		= __devexit_p(max77693_charger_remove),
};

static int __init max77693_charger_init(void)
{
	return platform_driver_register(&max77693_charger_driver);
}

static void __exit max77693_charger_exit(void)
{
	platform_driver_unregister(&max77693_charger_driver);
}

module_init(max77693_charger_init);
module_exit(max77693_charger_exit);

MODULE_AUTHOR("SangYoung Son <hello.son@samsung.com>");
MODULE_DESCRIPTION("max77693 Charger driver");
MODULE_LICENSE("GPL");
