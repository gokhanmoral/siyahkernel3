/*
 * linux/arch/arm/mach-exynos/include/mach/midas-tsp.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __MIDAS_TSP_H
#define __MIDAS_TSP_H __FILE__

#include <linux/melfas_ts.h>

extern int melfas_power(int on);
void melfas_set_touch_i2c(void);
void melfas_set_touch_i2c_to_gpio(void);
void midas_tsp_set_platdata(struct melfas_tsi_platform_data *pdata);
void midas_tsp_init(void);


#ifdef CONFIG_CPU_FREQ_GOV_ONDEMAND_FLEXRATE
extern void flexrate_request(void *data);
#else
#define flexrate_request	NULL
#endif

#endif /* __MIDAS_TSP_H */
