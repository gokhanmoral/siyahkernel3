/* linux/arch/arm/mach-exynos/busfreq_opp_5250.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS5 - BUS clock frequency scaling support with OPP
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/regulator/consumer.h>
#include <linux/sysfs.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/cpu.h>
#include <linux/ktime.h>
#include <linux/tick.h>
#include <linux/kernel_stat.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/opp.h>
#include <linux/clk.h>
#include <mach/busfreq.h>

#include <asm/mach-types.h>

#include <mach/ppmu.h>
#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/gpio.h>
#include <mach/regs-mem.h>
#include <mach/dev.h>
#include <mach/asv.h>

#include <plat/map-s5p.h>
#include <plat/gpio-cfg.h>
#include <plat/cpu.h>
#include <plat/clock.h>

#define UP_THRESHOLD		30
#define DMC_MAX_THRESHOLD	42
#define PPMU_THRESHOLD		5
#define IDLE_THRESHOLD		4
#define UP_CPU_THRESHOLD	11
#define MAX_CPU_THRESHOLD	20
#define CPU_SLOPE_SIZE		7

/* To save/restore DMC_PAUSE_CTRL register */
static unsigned int dmc_pause_ctrl;

enum busfreq_level_idx {
	LV_0,
	LV_1,
	LV_2,
	LV_3,
	LV_4,
	LV_5,
	LV_END
};

static struct busfreq_table *exynos5_busfreq_table;

static struct busfreq_table exynos5_busfreq_table_for800[] = {
	{LV_0, 800000, 1000000, 0, 0, 0},
	{LV_1, 400000, 1000000, 0, 0, 0},
	{LV_2, 267000, 1000000, 0, 0, 0},
	{LV_3, 200000, 1000000, 0, 0, 0},
	{LV_4, 100000, 1000000, 0, 0, 0},
	{LV_5, 100000, 1000000, 0, 0, 0},
};

static struct busfreq_table exynos5_busfreq_table_for667[] = {
	{LV_0, 667000, 1000000, 0, 0, 0},
	{LV_1, 334000, 1000000, 0, 0, 0},
	{LV_2, 222000, 1000000, 0, 0, 0},
	{LV_3, 167000, 1000000, 0, 0, 0},
	{LV_4, 133000, 1000000, 0, 0, 0},
	{LV_5, 133000, 1000000, 0, 0, 0},
};

static struct busfreq_table exynos5_busfreq_table_for533[] = {
	{LV_0, 533000, 1000000, 0, 0, 0},
	{LV_1, 267000, 1000000, 0, 0, 0},
	{LV_2, 178000, 1000000, 0, 0, 0},
	{LV_3, 133000, 1000000, 0, 0, 0},
	{LV_4, 107000, 1000000, 0, 0, 0},
	{LV_5, 67000, 1000000, 0, 0, 0},
};

static struct busfreq_table exynos5_busfreq_table_for400[] = {
	{LV_0, 400000, 1000000, 0, 0, 0},
	{LV_1, 267000, 1000000, 0, 0, 0},
	{LV_2, 200000, 1000000, 0, 0, 0},
	{LV_3, 160000, 1000000, 0, 0, 0},
	{LV_4, 130000, 1000000, 0, 0, 0},
	{LV_5, 100000, 1000000, 0, 0, 0},
};
#define ASV_GROUP	9
static unsigned int asv_group_index;

static unsigned int exynos5_asv_volt[ASV_GROUP][LV_END] = {
	/* 800      400      267      200      160      100 */
	{1200000, 1200000, 1200000, 1200000, 1200000, 1200000}, /* ASV0 */
	{1200000, 1200000, 1200000, 1200000, 1200000, 1200000}, /* ASV1 */
	{1200000, 1200000, 1200000, 1200000, 1200000, 1200000}, /* ASV2 */
	{1200000, 1200000, 1200000, 1200000, 1200000, 1200000}, /* ASV3 */
	{1200000, 1200000, 1200000, 1200000, 1200000, 1200000}, /* ASV4 */
	{1200000, 1200000, 1200000, 1200000, 1200000, 1200000}, /* ASV5 */
	{1200000, 1200000, 1200000, 1200000, 1200000, 1200000}, /* ASV6 */
	{1200000, 1200000, 1200000, 1200000, 1200000, 1200000}, /* ASV7 */
	{1200000, 1200000, 1200000, 1200000, 1200000, 1200000}, /* ASV8 */
};

static unsigned int exynos5250_int_volt[ASV_GROUP][LV_END] = {
	/* 400      400      267      200      160      100 */
	{1150000, 1150000, 1150000, 1150000, 1150000, 11500000}, /* ASV0 */
	{1150000, 1150000, 1150000, 1150000, 1150000, 11500000}, /* ASV1 */
	{1150000, 1150000, 1150000, 1150000, 1150000, 11500000}, /* ASV2 */
	{1150000, 1150000, 1150000, 1150000, 1150000, 11500000}, /* ASV3 */
	{1150000, 1150000, 1150000, 1150000, 1150000, 11500000}, /* ASV4 */
	{1150000, 1150000, 1150000, 1150000, 1150000, 11500000}, /* ASV5 */
	{1150000, 1150000, 1150000, 1150000, 1150000, 11500000}, /* ASV6 */
	{1150000, 1150000, 1150000, 1150000, 1150000, 11500000}, /* ASV7 */
	{1150000, 1150000, 1150000, 1150000, 1150000, 11500000}, /* ASV8 */
};

/* For CMU_LEX */
static unsigned int clkdiv_lex[LV_END][2] = {
	/*
	 * Clock divider value for following
	 * { DIVATCLK_LEX, DIVPCLK_LEX }
	 */

	/* ATCLK_LEX L0 : 200MHz */
	{0, 1},

	/* ATCLK_LEX L1 : 200MHz */
	{0, 1},

	/* ATCLK_LEX L2 : 166MHz */
	{0, 1},

	/* ATCLK_LEX L3 : 133MHz */
	{0, 1},

	/* ATCLK_LEX L4 : 114MHz */
	{0, 1},

	/* ATCLK_LEX L5 : 100MHz */
	{0, 1},
};

/* For CMU_R0X */
static unsigned int clkdiv_r0x[LV_END][1] = {
	/*
	 * Clock divider value for following
	 * { DIVPCLK_R0X }
	 */

	/* ACLK_PR0X L0 : 133MHz */
	{1},

	/* ACLK_PR0X L1 : 133MHz */
	{1},

	/* ACLK_DR0X L2 : 100MHz */
	{1},

	/* ACLK_PR0X L3 : 80MHz */
	{1},

	/* ACLK_PR0X L4 : 67MHz */
	{1},

	/* ACLK_PR0X L5 : 50MHz */
	{1},
};

/* For CMU_R1X */
static unsigned int clkdiv_r1x[LV_END][1] = {
	/*
	 * Clock divider value for following
	 * { DIVPCLK_R1X }
	 */

	/* ACLK_PR1X L0 : 133MHz */
	{1},

	/* ACLK_PR1X L1 : 133MHz */
	{1},

	/* ACLK_DR1X L2 : 100MHz */
	{1},

	/* ACLK_PR1X L3 : 80MHz */
	{1},

	/* ACLK_PR1X L4 : 67MHz */
	{1},

	/* ACLK_PR1X L5 : 50MHz */
	{1},
};

/* For CMU_TOP */
static unsigned int clkdiv_top[LV_END][10] = {
	/*
	 * Clock divider value for following
	 * { DIVACLK400_ISP, DIVACLK400_IOP, DIVACLK266, DIVACLK_200, DIVACLK_66_PRE,
	 DIVACLK_66, DIVACLK_333, DIVACLK_166, DIVACLK_300_DISP1, DIVACLK300_GSCL }
	 */

	/* ACLK_400_ISP L0 : 400MHz */
	{1, 1, 2, 3, 1, 5, 0, 1, 2, 2},

	/* ACLK_400_ISP L1 : 400MHz */
	{1, 1, 2, 3, 1, 5, 0, 1, 2, 2},

	/* ACLK_400_ISP L2 : 267MHz */
	{2, 3, 3, 4, 1, 5, 1, 2, 2, 2},

	/* ACLK_400_ISP L3 : 200MHz */
	{3, 3, 4, 5, 1, 5, 2, 3, 2, 2},

	/* ACLK_400_ISP L4 : 160MHz */
	{4, 4, 5, 6, 1, 5, 3, 4, 5, 5},

	/* ACLK_400_ISP L5 : 100MHz */
	{7, 7, 7, 7, 1, 5, 7, 7, 7, 7},
};

/* For CMU_CDREX */
static unsigned int __maybe_unused clkdiv_cdrex_for800[LV_END][9] = {
	/*
	 * Clock divider value for following
	 * { DIVMCLK_DPHY, DIVMCLK_CDREX2, DIVACLK_CDREX, DIVMCLK_CDREX,
		DIVPCLK_CDREX, DIVC2C, DIVC2C_ACLK, DIVMCLK_EFPHY, DIVACLK_EFCON }
	 */

	/* MCLK_CDREX L0: 800MHz */
	{0, 0, 1, 0, 5, 1, 1, 4, 1},

	/* MCLK_CDREX L1: 400MHz */
	{0, 1, 1, 1, 5, 2, 1, 5, 1},

	/* MCLK_CDREX L2: 267MHz */
	{0, 2, 1, 2, 5, 3, 1, 6, 1},

	/* MCLK_CDREX L3: 200MHz */
	{0, 3, 1, 3, 5, 4, 1, 7, 1},

	/* MCLK_CDREX L4: 100MHz */
	{0, 7, 1, 5, 7, 7, 1, 15, 1},

	/* MCLK_CDREX L5: 100MHz */
	{0, 7, 1, 5, 7, 7, 1, 15, 1},
};

static unsigned int __maybe_unused clkdiv_cdrex_for667[LV_END][9] = {
	/*
	 * Clock divider value for following
	 * { DIVMCLK_DPHY, DIVMCLK_CDREX2, DIVACLK_CDREX, DIVMCLK_CDREX,
		DIVPCLK_CDREX, DIVC2C, DIVC2C_ACLK, DIVMCLK_EFPHY, DIVACLK_EFCON }
	 */

	/* MCLK_CDREX L0: 667MHz */
	{0, 0, 1, 0, 4, 1, 1, 4, 1},

	/* MCLK_CDREX L1: 334MHz */
	{0, 1, 1, 1, 4, 2, 1, 5, 1},

	/* MCLK_CDREX L2: 222MHz */
	{0, 2, 1, 2, 4, 3, 1, 6, 1},

	/* MCLK_CDREX L3: 167MHz */
	{0, 3, 1, 3, 4, 4, 1, 7, 1},

	/* MCLK_CDREX L4: 133MHz */
	{0, 4, 1, 4, 4, 5, 1, 8, 1},

	/* MCLK_CDREX L5: 133MHz */
	{0, 4, 1, 4, 4, 5, 1, 8, 1},
};

static unsigned int clkdiv_cdrex_for533[LV_END][9] = {
	/*
	 * Clock divider value for following
	 * { DIVMCLK_DPHY, DIVMCLK_CDREX2, DIVACLK_CDREX, DIVMCLK_CDREX,
		DIVPCLK_CDREX, DIVC2C, DIVC2C_ACLK, DIVMCLK_EFPHY, DIVACLK_EFCON }
	 */

	/* MCLK_CDREX L0: 533MHz */
	{0, 0, 1, 0, 3, 1, 1, 4, 1},

	/* MCLK_CDREX L1: 267MHz */
	{0, 1, 1, 1, 3, 2, 1, 5, 1},

	/* MCLK_CDREX L2: 178MHz */
	{0, 2, 1, 2, 3, 3, 1, 6, 1},

	/* MCLK_CDREX L3: 133MHz */
	{0, 3, 1, 3, 3, 4, 1, 7, 1},

	/* MCLK_CDREX L4: 107MHz */
	{0, 4, 1, 4, 3, 5, 1, 8, 1},

	/* MCLK_CDREX L5: 67MHz */
	{0, 7, 1, 3, 7, 7, 1, 15, 1},
};

static unsigned int __maybe_unused clkdiv_cdrex_for400[LV_END][9] = {
	/*
	 * Clock divider value for following
	 * { DIVMCLK_DPHY, DIVMCLK_CDREX2, DIVACLK_CDREX, DIVMCLK_CDREX,
		DIVPCLK_CDREX, DIVC2C, DIVC2C_ACLK, DIVMCLK_EFPHY, DIVACLK_EFCON }
	 */

	/* MCLK_CDREX L0: 400MHz */
	{1, 1, 1, 0, 5, 1, 1, 4, 1},

	/* MCLK_CDREX L1: 267MHz */
	{1, 2, 1, 2, 2, 2, 1, 5, 1},

	/* MCLK_CDREX L2: 200MHz */
	{1, 3, 1, 3, 2, 3, 1, 6, 1},

	/* MCLK_CDREX L3: 160MHz */
	{1, 4, 1, 4, 2, 4, 1, 7, 1},

	/* MCLK_CDREX L4: 133MHz */
	{1, 5, 1, 5, 2, 5, 1, 8, 1},

	/* MCLK_CDREX L5: 100MHz */
	{1, 7, 1, 2, 7, 7, 1, 15, 1},
};

static unsigned int (*clkdiv_cdrex)[9];

static void exynos5250_set_bus_volt(void)
{
	unsigned int i;

	asv_group_index = 0;

	if (asv_group_index == 0xff)
		asv_group_index = 0;

	printk(KERN_INFO "DVFS : VDD_INT Voltage table set with %d Group\n", asv_group_index);

	for (i = 0 ; i < LV_END ; i++)
		exynos5_busfreq_table[i].volt =
			exynos5_asv_volt[asv_group_index][i];

	return;
}

unsigned int exynos5250_target(unsigned int div_index)
{
	unsigned int tmp;
	unsigned int tmp2;

	/* Change Divider - CDREX */
	tmp = __raw_readl(EXYNOS5_CLKDIV_CDREX);

	tmp &= ~(EXYNOS5_CLKDIV_CDREX_MCLK_DPHY_MASK |
		EXYNOS5_CLKDIV_CDREX_MCLK_CDREX2_MASK |
		EXYNOS5_CLKDIV_CDREX_ACLK_CDREX_MASK |
		EXYNOS5_CLKDIV_CDREX_MCLK_CDREX_MASK |
		EXYNOS5_CLKDIV_CDREX_PCLK_CDREX_MASK |
		EXYNOS5_CLKDIV_CDREX_ACLK_CLK400_MASK |
		EXYNOS5_CLKDIV_CDREX_ACLK_C2C200_MASK |
		EXYNOS5_CLKDIV_CDREX_ACLK_EFCON_MASK);

	tmp |= ((clkdiv_cdrex[div_index][0] << EXYNOS5_CLKDIV_CDREX_MCLK_DPHY_SHIFT) |
		(clkdiv_cdrex[div_index][1] << EXYNOS5_CLKDIV_CDREX_MCLK_CDREX2_SHIFT) |
		(clkdiv_cdrex[div_index][2] << EXYNOS5_CLKDIV_CDREX_ACLK_CDREX_SHIFT) |
		(clkdiv_cdrex[div_index][3] << EXYNOS5_CLKDIV_CDREX_MCLK_CDREX_SHIFT) |
		(clkdiv_cdrex[div_index][4] << EXYNOS5_CLKDIV_CDREX_PCLK_CDREX_SHIFT) |
		(clkdiv_cdrex[div_index][5] << EXYNOS5_CLKDIV_CDREX_ACLK_CLK400_SHIFT) |
		(clkdiv_cdrex[div_index][6] << EXYNOS5_CLKDIV_CDREX_ACLK_C2C200_SHIFT) |
		(clkdiv_cdrex[div_index][8] << EXYNOS5_CLKDIV_CDREX_ACLK_EFCON_SHIFT));

	__raw_writel(tmp, EXYNOS5_CLKDIV_CDREX);

	do {
		tmp = __raw_readl(EXYNOS5_CLKDIV_STAT_CDREX);
	} while (tmp & 0x11111111);

	tmp = __raw_readl(EXYNOS5_CLKDIV_CDREX2);

	tmp &= ~EXYNOS5_CLKDIV_CDREX2_MCLK_EFPHY_MASK;

	tmp |= clkdiv_cdrex[div_index][7] << EXYNOS5_CLKDIV_CDREX2_MCLK_EFPHY_SHIFT;

	__raw_writel(tmp, EXYNOS5_CLKDIV_CDREX2);

	do {
		tmp = __raw_readl(EXYNOS5_CLKDIV_STAT_CDREX2);
	} while (tmp & 0x1);

	/* Change Divider - TOP */
	tmp = __raw_readl(EXYNOS5_CLKDIV_TOP0);

	tmp &= ~(EXYNOS5_CLKDIV_TOP0_ACLK266_MASK |
		EXYNOS5_CLKDIV_TOP0_ACLK200_MASK |
		EXYNOS5_CLKDIV_TOP0_ACLK66_MASK |
		EXYNOS5_CLKDIV_TOP0_ACLK333_MASK |
		EXYNOS5_CLKDIV_TOP0_ACLK166_MASK |
		EXYNOS5_CLKDIV_TOP0_ACLK300_DISP1_MASK);

	tmp |= ((clkdiv_top[div_index][2] << EXYNOS5_CLKDIV_TOP0_ACLK266_SHIFT) |
		(clkdiv_top[div_index][3] << EXYNOS5_CLKDIV_TOP0_ACLK200_SHIFT) |
		(clkdiv_top[div_index][5] << EXYNOS5_CLKDIV_TOP0_ACLK66_SHIFT) |
		(clkdiv_top[div_index][6] << EXYNOS5_CLKDIV_TOP0_ACLK333_SHIFT) |
		(clkdiv_top[div_index][7] << EXYNOS5_CLKDIV_TOP0_ACLK166_SHIFT) |
		(clkdiv_top[div_index][8] << EXYNOS5_CLKDIV_TOP0_ACLK300_DISP1_SHIFT));

	__raw_writel(tmp, EXYNOS5_CLKDIV_TOP0);

	do {
		tmp = __raw_readl(EXYNOS5_CLKDIV_STAT_TOP0);
	} while (tmp & 0x151101);

	tmp = __raw_readl(EXYNOS5_CLKDIV_TOP1);

	tmp &= ~(EXYNOS5_CLKDIV_TOP1_ACLK400_ISP_MASK |
		EXYNOS5_CLKDIV_TOP1_ACLK400_IOP_MASK |
		EXYNOS5_CLKDIV_TOP1_ACLK66_PRE_MASK |
		EXYNOS5_CLKDIV_TOP1_ACLK300_GSCL_MASK);

	tmp |= ((clkdiv_top[div_index][0] << EXYNOS5_CLKDIV_TOP1_ACLK400_ISP_SHIFT) |
		(clkdiv_top[div_index][1] << EXYNOS5_CLKDIV_TOP1_ACLK400_IOP_SHIFT) |
		(clkdiv_top[div_index][2] << EXYNOS5_CLKDIV_TOP1_ACLK66_PRE_SHIFT) |
		(clkdiv_top[div_index][9] << EXYNOS5_CLKDIV_TOP1_ACLK300_GSCL_SHIFT));

	__raw_writel(tmp, EXYNOS5_CLKDIV_TOP1);

	do {
		tmp = __raw_readl(EXYNOS5_CLKDIV_STAT_TOP1);
		tmp2 = __raw_readl(EXYNOS5_CLKDIV_STAT_TOP0);
	} while ((tmp & 0x1110000) && (tmp2 & 0x80000));

	/* Change Divider - LEX */
	tmp = __raw_readl(EXYNOS5_CLKDIV_LEX);

	tmp &= ~(EXYNOS5_CLKDIV_LEX_ATCLK_LEX_MASK | EXYNOS5_CLKDIV_LEX_PCLK_LEX_MASK);

	tmp |= ((clkdiv_lex[div_index][0] << EXYNOS5_CLKDIV_LEX_ATCLK_LEX_SHIFT) |
		(clkdiv_lex[div_index][1] << EXYNOS5_CLKDIV_LEX_PCLK_LEX_SHIFT));

	__raw_writel(tmp, EXYNOS5_CLKDIV_LEX);

	do {
		tmp = __raw_readl(EXYNOS5_CLKDIV_STAT_LEX);
	} while (tmp & 0x110);

	/* Change Divider - R0X */
	tmp = __raw_readl(EXYNOS5_CLKDIV_R0X);

	tmp &= ~EXYNOS5_CLKDIV_R0X_PCLK_R0X_MASK;

	tmp |= (clkdiv_r0x[div_index][0] << EXYNOS5_CLKDIV_R0X_PCLK_R0X_SHIFT);

	__raw_writel(tmp, EXYNOS5_CLKDIV_R0X);

	do {
		tmp = __raw_readl(EXYNOS5_CLKDIV_STAT_R0X);
	} while (tmp & 0x10);

	/* Change Divider - R1X */
	tmp = __raw_readl(EXYNOS5_CLKDIV_R1X);

	tmp &= ~EXYNOS5_CLKDIV_R1X_PCLK_R1X_MASK;

	tmp |= (clkdiv_r1x[div_index][0] << EXYNOS5_CLKDIV_R0X_PCLK_R0X_SHIFT);

	__raw_writel(tmp, EXYNOS5_CLKDIV_R1X);

	do {
		tmp = __raw_readl(EXYNOS5_CLKDIV_STAT_R1X);
	} while (tmp & 0x10);

	return div_index;
}

unsigned int exynos5250_get_table_index(struct opp *opp)
{
	unsigned int index;

	for (index = LV_0; index < LV_END; index++)
		if (opp_get_freq(opp) == exynos5_busfreq_table[index].mem_clk)
			break;

	return index;
}

void exynos5250_suspend(void)
{
	/* Nothing to do */
}

void exynos5250_resume(void)
{
	__raw_writel(dmc_pause_ctrl, EXYNOS5_DMC_PAUSE_CTRL);
}

unsigned int exynos5250_get_int_volt(unsigned long index)
{
	return exynos5250_int_volt[asv_group_index][index];
}

struct opp *exynos5250_monitor(struct busfreq_data *data)
{
	struct opp *opp = data->curr_opp;
	int i;
	unsigned int cpu_load_average = 0;
	unsigned int dmc_c_load_average = 0;
	unsigned int dmc_l_load_average = 0;
	unsigned int dmc_r1_load_average = 0;
	unsigned int dmc_load_average;
	unsigned long cpufreq = 0;
	unsigned long lockfreq;
	unsigned long dmcfreq;
	unsigned long newfreq;
	unsigned long currfreq = opp_get_freq(data->curr_opp) / 1000;
	unsigned long maxfreq = opp_get_freq(data->max_opp) / 1000;
	unsigned long cpu_load;
	unsigned long dmc_load;
	unsigned long dmc_c_load;
	unsigned long dmc_r1_load;
	unsigned long dmc_l_load;

	ppmu_update(data->dev, 3);

	/* Convert from base xxx to base maxfreq */
	cpu_load = div64_u64(ppmu_load[PPMU_CPU] * currfreq, maxfreq);
	dmc_c_load = div64_u64(ppmu_load[PPMU_DDR_C] * currfreq, maxfreq);
	dmc_r1_load = div64_u64(ppmu_load[PPMU_DDR_R1] * currfreq, maxfreq);
	dmc_l_load = div64_u64(ppmu_load[PPMU_DDR_L] * currfreq, maxfreq);

	data->load_history[PPMU_CPU][data->index] = cpu_load;
	data->load_history[PPMU_DDR_C][data->index] = dmc_c_load;
	data->load_history[PPMU_DDR_R1][data->index] = dmc_r1_load;
	data->load_history[PPMU_DDR_L][data->index++] = dmc_l_load;

	if (data->index >= LOAD_HISTORY_SIZE)
		data->index = 0;

	for (i = 0; i < LOAD_HISTORY_SIZE; i++) {
		cpu_load_average += data->load_history[PPMU_CPU][i];
		dmc_c_load_average += data->load_history[PPMU_DDR_C][i];
		dmc_r1_load_average += data->load_history[PPMU_DDR_R1][i];
		dmc_l_load_average += data->load_history[PPMU_DDR_L][i];
	}

	/* Calculate average Load */
	cpu_load_average /= LOAD_HISTORY_SIZE;
	dmc_c_load_average /= LOAD_HISTORY_SIZE;
	dmc_r1_load_average /= LOAD_HISTORY_SIZE;
	dmc_l_load_average /= LOAD_HISTORY_SIZE;

	if (dmc_c_load >= dmc_r1_load) {
		dmc_load = dmc_c_load;
		dmc_load_average = dmc_c_load_average;
	} else {
		dmc_load = dmc_r1_load;
		dmc_load_average = dmc_r1_load_average;
	}

	if (dmc_l_load >= dmc_load) {
		dmc_load = dmc_l_load;
		dmc_load_average = dmc_l_load_average;
	}

	if (dmc_load >= DMC_MAX_THRESHOLD) {
		dmcfreq = opp_get_freq(data->max_opp);
	} else if (dmc_load < IDLE_THRESHOLD) {
		if (dmc_load_average < IDLE_THRESHOLD)
			opp = step_down(data, 1);
		else
			opp = data->curr_opp;
		dmcfreq = opp_get_freq(opp);
	} else {
		if (dmc_load < dmc_load_average) {
			dmc_load = dmc_load_average;
			if (dmc_load >= DMC_MAX_THRESHOLD)
				dmc_load = DMC_MAX_THRESHOLD;
		}
		dmcfreq = div64_u64(maxfreq * dmc_load * 1000, DMC_MAX_THRESHOLD);
	}

	lockfreq = dev_max_freq(data->dev);

	newfreq = max3(lockfreq, dmcfreq, cpufreq);

	opp = opp_find_freq_ceil(data->dev, &newfreq);

	return opp;
}

int exynos5250_init(struct device *dev, struct busfreq_data *data)
{
	unsigned int i;
	unsigned long maxfreq = ULONG_MAX;
	unsigned long minfreq = 0;
	unsigned long cdrexfreq;
	struct clk *clk;
	int ret;

	/* Enable pause function for DREX2 DVFS */
	dmc_pause_ctrl = __raw_readl(EXYNOS5_DMC_PAUSE_CTRL);
	dmc_pause_ctrl |= DMC_PAUSE_ENABLE;
	__raw_writel(dmc_pause_ctrl, EXYNOS5_DMC_PAUSE_CTRL);

	clk = clk_get(NULL, "mout_cdrex");
	if (IS_ERR(clk)) {
		dev_err(dev, "Fail to get mclk_cdrex clock");
		ret = PTR_ERR(clk);
		return ret;
	}
	cdrexfreq = clk_get_rate(clk) / 1000;

	clk_put(clk);

	if (cdrexfreq == 800000) {
		clkdiv_cdrex = clkdiv_cdrex_for800;
		exynos5_busfreq_table = exynos5_busfreq_table_for800;
	} else if (cdrexfreq == 667000) {
		clkdiv_cdrex = clkdiv_cdrex_for667;
		exynos5_busfreq_table = exynos5_busfreq_table_for667;
	} else if (cdrexfreq == 533000) {
		clkdiv_cdrex = clkdiv_cdrex_for533;
		exynos5_busfreq_table = exynos5_busfreq_table_for533;
	} else if (cdrexfreq == 400000) {
		clkdiv_cdrex = clkdiv_cdrex_for400;
		exynos5_busfreq_table = exynos5_busfreq_table_for400;
	} else {
		dev_err(dev, "Don't support cdrex table\n");
		return -EINVAL;
	}

	exynos5250_set_bus_volt();

	for (i = 0; i < LV_END; i++) {
		ret = opp_add(dev, exynos5_busfreq_table[i].mem_clk,
				exynos5_busfreq_table[i].volt);
		if (ret) {
			dev_err(dev, "Fail to add opp entries.\n");
			return ret;
		}
	}

	opp_disable(dev, 107000);
	opp_disable(dev, 67000);

	data->table = exynos5_busfreq_table;
	data->table_size = LV_END;

	/* Find max frequency */
	data->max_opp = opp_find_freq_floor(dev, &maxfreq);
	data->min_opp = opp_find_freq_ceil(dev, &minfreq);
	data->curr_opp = opp_find_freq_ceil(dev, &cdrexfreq);

	data->vdd_int = regulator_get(NULL, "vdd_int");
	if (IS_ERR(data->vdd_int)) {
		pr_err("failed to get resource %s\n", "vdd_int");
		return -ENODEV;
	}

	data->vdd_mif = regulator_get(NULL, "vdd_mif");
	if (IS_ERR(data->vdd_mif)) {
		pr_err("failed to get resource %s\n", "vdd_mif");
		regulator_put(data->vdd_int);
		return -ENODEV;
	}

	return 0;
}
