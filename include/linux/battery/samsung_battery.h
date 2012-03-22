/*
 * Copyright (C) 2010 Samsung Electronics, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */

#ifndef __MACH_SAMSUNG_BATTERY_H
#define __MACH_SAMSUNG_BATTERY_H __FILE__

#include <linux/android_alarm.h>

struct battery_info {
	struct device				*dev;
	struct samsung_battery_platform_data	*pdata;
#if defined(CONFIG_S3C_ADC)
	struct s3c_adc_client			*adc_client;
#endif

	bool use_sub_charger;
	char *fuelgauge_name;
	char *charger_name;
	char *sub_charger_name;

	/* android common power supply */
	struct power_supply psy_bat;
	struct power_supply psy_usb;
	struct power_supply psy_ac;

	/* charger, fuelgauge psy depends on machine */
	struct power_supply *psy_charger;
	struct power_supply *psy_fuelgauge;
	struct power_supply *psy_sub_charger;

	struct work_struct monitor_work;
	unsigned int monitor_mode;
	unsigned int monitor_interval;
	struct work_struct error_work;

	struct wake_lock charge_wake_lock;
	struct wake_lock monitor_wake_lock;
	struct wake_lock emer_wake_lock;

	unsigned int charge_real_state;
	unsigned int charge_virt_state;
	unsigned int charge_type;
	unsigned int charge_current;

	unsigned int battery_health;
	unsigned int battery_present;
	unsigned int battery_vcell;
	unsigned int battery_vfocv;
	unsigned int battery_soc;
	unsigned int battery_raw_soc;
	int battery_temp;
	int battery_temp_adc;

	unsigned int cable_type;

	/* For SAMSUNG charge spec */
	unsigned int vf_state;
	unsigned int temperature_state;
	unsigned int overheated_state;
	unsigned int freezed_state;
	unsigned int full_charged_state;
	unsigned int abstimer_state;
	unsigned int recharge_phase;
	unsigned int recharge_start;
	unsigned int lpm_state;
	unsigned int siop_state;
	unsigned int siop_charge_current;
	unsigned int health_state;
	unsigned int led_state;

	unsigned int charge_start_time;
	struct alarm	alarm;
	bool		slow_poll;
	ktime_t		last_poll;

	struct proc_dir_entry *entry;

	/* For debugging */
	unsigned int battery_test_mode;
	unsigned int battery_error_test;
};

/* jig state */
extern bool is_jig_attached;

/*
 * Use for charger
 */
enum charger_state {
	CHARGER_STATE_ENABLE	= 0,
	CHARGER_STATE_DISABLE	= 1
};

/*
 * Use for fuelgauge
 */
enum voltage_type {
	VOLTAGE_TYPE_VCELL	= 0,
	VOLTAGE_TYPE_VFOCV	= 1,
};

enum soc_type {
	SOC_TYPE_ADJUSTED	= 0,
	SOC_TYPE_RAW		= 1,
};

/*
 * Use for battery
 */
#define CHARGER_AC_CURRENT	1000
#define CHARGER_AC_CURRENT_S2PLUS	650
#define CHARGER_USB_CURRENT	475
#define CHARGER_CDP_CURRENT	1000
#define CHARGER_WPC_CURRENT	650
#define CHARGER_OFF_CURRENT	0
#define CHARGER_KEEP_CURRENT	-1

/* VF error check */
#define VF_CHECK_COUNT		10
#define VF_CHECK_DELAY		1000
#define RESET_SOC_DIFF_TH	100000

enum {
	CHARGE_DISABLE = 0,
	CHARGE_ENABLE,
};

enum {
	JIG_OFF = 0,
	JIG_ON,
};

/* temperature source */
enum {
	TEMPER_FUELGAUGE = 0,
	TEMPER_AP_ADC,
	TEMPER_EXT_ADC,

	TEMPER_UNKNOWN,
};

/* siop state */
enum {
	SIOP_DEACTIVE = 0,
	SIOP_ACTIVE,
};

/* monitoring mode */
enum {
	MONITOR_CHNG = 0,
	MONITOR_CHNG_SUSP,
	MONITOR_NORM,
	MONITOR_NORM_SUSP,
	MONITOR_EMER,
};

/* Temperature from adc */
#if defined(CONFIG_STMPE811_ADC)
#define BATTERY_TEMPER_CH 7
u16 stmpe811_get_adc_data(u8 channel);
int stmpe811_get_adc_value(u8 channel);
#endif

/* LED control */
enum led_state {
	BATT_LED_CHARGING = 0,
	BATT_LED_DISCHARGING,
	BATT_LED_NOT_CHARGING,
	BATT_LED_FULL,
};

enum led_color {
	BATT_LED_RED = 0,
	BATT_LED_GREEN,
	BATT_LED_BLUE,
};

enum led_pattern {
	BATT_LED_PATT_OFF = 0,
	BATT_LED_PATT_CHG,
	BATT_LED_PATT_NOT_CHG,
};

#if defined(CONFIG_LEDS_AN30259A)
extern void an30259a_start_led_pattern(int mode);
extern void an30259a_led(int RGB, u8 brightness);
#else
static inline void an30259a_start_led_pattern(int mode) {};
static inline void an30259a_led(int RGB, u8 brightness) {};
#endif

/**
 * struct sec_bat_plaform_data - init data for sec batter driver
 * @fuel_gauge_name: power supply name of fuel gauge
 * @charger_name: power supply name of charger
 */
struct samsung_battery_platform_data {
	char *charger_name;
	char *fuelgauge_name;
	char *sub_charger_name;
	bool use_sub_charger;

	/* battery voltage design */
	unsigned int voltage_max;
	unsigned int voltage_min;

	/* variable monitoring interval */
	unsigned int chng_interval;
	unsigned int chng_susp_interval;
	unsigned int norm_interval;
	unsigned int norm_susp_interval;
	unsigned int emer_interval;

	/* Recharge sceanario */
	unsigned int recharge_voltage;
	unsigned int abstimer_charge_duration;
	unsigned int abstimer_recharge_duration;

	/* Temperature scenario */
	int overheat_stop_temp;
	int overheat_recovery_temp;
	int freeze_stop_temp;
	int freeze_recovery_temp;

	/* Temperature source 0: fuelgauge, 1: ap adc, 2: ex. adc */
	int temper_src;
	int temper_ch;
#ifdef CONFIG_S3C_ADC
	int (*covert_adc) (int, int);
#endif

	/* suspend in charging */
	bool suspend_chging;

	/* support led indicator */
	bool led_indicator;
};

#endif /* __MACH_SAMSUNG_BATTERY_H */
