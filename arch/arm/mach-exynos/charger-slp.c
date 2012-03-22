/*
 * linux/arch/arm/mach-exynos/charger-slp.c
 * COPYRIGHT(C) 2011
 * MyungJoo Ham <myungjoo.ham@samsung.com>
 *
 * Charger Support with Charger-Manager Framework
 *
 */

#include <asm/io.h>

#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/power/charger-manager.h>

#include <plat/adc.h>
#include <plat/pm.h>

#include <mach/regs-pmu.h>
#include <mach/irqs.h>

#include "board-mobile.h"

#define S5P_WAKEUP_STAT_WKSRC_MASK	0x000ffe3f

/* Temperatures in milli-centigrade */
#define SECBATTSPEC_TEMP_HIGH		(65 * 1000)
#define SECBATTSPEC_TEMP_HIGH_REC	(43 * 1000)
#define SECBATTSPEC_TEMP_LOW		(-5 * 1000)
#define SECBATTSPEC_TEMP_LOW_REC	(0 * 1000)

/* TODO actually read temperature from ADC */
static int __read_thermistor_mC(void)
{
	return 25000; /* 25 mili-Centigrade */
}


enum temp_stat { TEMP_OK = 0, TEMP_HOT = 1, TEMP_COLD = -1 };

static int midas_thermistor_ck(int *mC)
{
	static enum temp_stat state = TEMP_OK;

	*mC = __read_thermistor_mC();
	switch (state) {
	case TEMP_OK:
		if (*mC >= SECBATTSPEC_TEMP_HIGH)
			state = TEMP_HOT;
		else if (*mC <= SECBATTSPEC_TEMP_LOW)
			state = TEMP_COLD;
		break;
	case TEMP_HOT:
		if (*mC <= SECBATTSPEC_TEMP_LOW)
			state = TEMP_COLD;
		else if (*mC < SECBATTSPEC_TEMP_HIGH_REC)
			state = TEMP_OK;
		break;
	case TEMP_COLD:
		if (*mC >= SECBATTSPEC_TEMP_HIGH)
			state = TEMP_HOT;
		else if (*mC > SECBATTSPEC_TEMP_LOW_REC)
			state = TEMP_OK;
	default:
		pr_err("%s has invalid state %d\n", __func__, state);
	}

	return state;
}

static bool s3c_wksrc_rtc_alarm(void)
{
	u32 reg = s3c_suspend_wakeup_stat & S5P_WAKEUP_STAT_WKSRC_MASK;

	if ((reg & S5P_WAKEUP_STAT_RTCALARM) &&
	    !(reg & ~S5P_WAKEUP_STAT_RTCALARM))
		return true; /* yes, it is */

	return false;
}

static char *midas_charger_stats[] = {
#if defined(CONFIG_BATTERY_MAX77693_CHARGER)
	"max77693-charger",
#endif
	NULL };

static struct regulator_bulk_data midas_chargers[] = {
	{ .supply = "vinchg1", },
};

static struct charger_desc midas_charger_desc = {
	.psy_name		= "battery",
	.polling_interval_ms	= 30000,
	.polling_mode		= CM_POLL_EXTERNAL_POWER_ONLY,
	.fullbatt_vchkdrop_ms	= 30000,
	.fullbatt_vchkdrop_uV	= 50000,
	.fullbatt_uV		= 4200000,
	.battery_present	= CM_CHARGER_STAT,
	.psy_charger_stat	= midas_charger_stats,
	.charger_regulators	= midas_chargers,
	.num_charger_regulators	= ARRAY_SIZE(midas_chargers),
	.psy_fuel_gauge		= "max17047-fuelgauge",
	.is_temperature_error	= midas_thermistor_ck,
	.measure_ambient_temp	= true,
	.measure_battery_temp	= false,
	.soc_margin		= 0,
};

struct charger_global_desc midas_charger_g_desc = {
	.rtc = "rtc0",
	.is_rtc_only_wakeup_reason = s3c_wksrc_rtc_alarm,
	.assume_timer_stops_in_suspend	= false,
};

struct platform_device midas_charger_manager = {
	.name			= "charger-manager",
	.dev			= {
		.platform_data = &midas_charger_desc,
	},
};
