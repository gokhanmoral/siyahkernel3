/*
 *  linux/arch/arm/mach-exynos/p11-gpio.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS - GPIO setting in set board
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/serial_core.h>
#include <plat/devs.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-serial.h>
#include <mach/gpio-midas.h>
#include <plat/cpu.h>
#include <mach/pmu.h>

struct gpio_init_data {
	uint num;
	uint cfg;
	uint val;
	uint pud;
	uint drv;
};

extern int s3c_gpio_slp_cfgpin(unsigned int pin, unsigned int config);
extern int s3c_gpio_slp_setpull_updown(unsigned int pin, unsigned int config);

/* this is sample code for p11 board */
static struct gpio_init_data p11_init_gpios[] = {

};

/* this table only for p11 board */
static unsigned int exynos5_sleep_gpio_table[][3] = {
	{EXYNOS5_GPH1(1),  S3C_GPIO_SLP_PREV, S3C_GPIO_PULL_NONE},

	/* BUCK4_SEL(Q) / NC(D) */
	{EXYNOS5_GPV0(0),  S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE},
	/* BUCK3_SEL(Q) / NC(D) */
	{EXYNOS5_GPV0(1),  S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE},
	/* BUCK2_SEL(Q) / NC(D) */
	{EXYNOS5_GPV0(4),  S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE},
	/* PMIC_DVS3(Q) / NC(D) */
	{EXYNOS5_GPV0(5),  S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE},
	/* PMIC_DVS2(Q) / NC(D) */
	{EXYNOS5_GPV0(6),  S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE},
	/* PMIC_DVS1(Q) / NC(D) */
	{EXYNOS5_GPV0(7),  S3C_GPIO_SLP_OUT0, S3C_GPIO_PULL_NONE},
};

static void config_sleep_gpio_table(int array_size,
				    unsigned int (*gpio_table)[3])
{
	u32 i, gpio;

	for (i = 0; i < array_size; i++) {
		gpio = gpio_table[i][0];
		s3c_gpio_slp_cfgpin(gpio, gpio_table[i][1]);
		s3c_gpio_slp_setpull_updown(gpio, gpio_table[i][2]);
	}
}

/* To save power consumption, gpio pin set before enterling sleep */
void p11_config_sleep_gpio_table(void)
{
	config_sleep_gpio_table(ARRAY_SIZE(exynos5_sleep_gpio_table),
			exynos5_sleep_gpio_table);
}

/* Intialize gpio set in p11 board */
void p11_config_gpio_table(void)
{

	printk(KERN_DEBUG "%s\n", __func__);

}
