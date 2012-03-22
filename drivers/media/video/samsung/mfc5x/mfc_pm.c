/*
 * linux/drivers/media/video/samsung/mfc5x/mfc_pm.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * Power management module for Samsung MFC (Multi Function Codec - FIMV) driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/err.h>
#include <linux/clk.h>
#include <linux/delay.h>
#ifdef CONFIG_PM_RUNTIME
#include <linux/pm_runtime.h>
#endif

#include <plat/clock.h>
#include <plat/s5p-mfc.h>
#include <plat/cpu.h>

#include <mach/regs-pmu.h>

#include <asm/io.h>

#include "mfc_dev.h"
#include "mfc_log.h"

#define MFC_PARENT_CLK_NAME	"mout_mfc0"
#define MFC_CLKNAME		"sclk_mfc"
#define MFC_GATE_CLK_NAME	"mfc"

#undef CLK_DEBUG

static struct mfc_pm *pm;

#ifdef CLK_DEBUG
atomic_t clk_ref;
#endif

#ifdef CONFIG_CPU_FREQ
#include <linux/cpufreq.h>

#define MFC0_BUS_CLK_NAME	"aclk_gdl"
#define MFC1_BUS_CLK_NAME	"aclk_gdr"

static struct clk *bus_clk;
static unsigned int prev_bus_rate;

static int mfc_cpufreq_transition(struct notifier_block *nb,
					unsigned long val, void *data)
{
	unsigned long bus_rate;

	if (val == CPUFREQ_PRECHANGE)
		prev_bus_rate = clk_get_rate(bus_clk);

	if (val == CPUFREQ_POSTCHANGE) {
		bus_rate = clk_get_rate(bus_clk);

		if (bus_rate != prev_bus_rate) {
			mfc_dbg("MFC freq pre: %lu\n",
				clk_get_rate(pm->op_clk));
			if (clk_set_rate(pm->op_clk, bus_rate)) {
				printk(KERN_ERR "%s rate change failed: %lu\n",
						pm->op_clk->name, bus_rate);
				return -EINVAL;
			}
			mfc_dbg("MFC freq post: %lu\n",
				clk_get_rate(pm->op_clk));
		}
	}

	return 0;
}

static inline int mfc_cpufreq_register(void)
{
	int ret;
	unsigned long rate;

	bus_clk = clk_get(pm->device, MFC0_BUS_CLK_NAME);

	if (IS_ERR(bus_clk)) {
		printk(KERN_ERR "failed to get bus clock\n");
		ret = -ENOENT;
		goto err_bus_clk;
	}

	prev_bus_rate = clk_get_rate(bus_clk);

	rate = clk_get_rate(pm->clock);

	if (rate != prev_bus_rate) {
		if (clk_set_rate(pm->op_clk, prev_bus_rate)) {
			printk(KERN_ERR "%s rate change failed: %u\n",
					pm->op_clk->name, prev_bus_rate);
			ret = -EINVAL;
			goto err_bus_clk;
		}
	}

	pm->freq_transition.notifier_call = mfc_cpufreq_transition;

	return cpufreq_register_notifier(&pm->freq_transition,
					 CPUFREQ_TRANSITION_NOTIFIER);
err_bus_clk:
	return ret;
}

static inline void mfc_cpufreq_deregister(void)
{
	clk_put(bus_clk);

	prev_bus_rate = 0;

	cpufreq_unregister_notifier(&pm->freq_transition,
				    CPUFREQ_TRANSITION_NOTIFIER);
}

#else
static inline int mfc_cpufreq_register(void)
{

	return 0;
}

static inline void mfc_cpufreq_deregister(void)
{
}
#endif /* CONFIG_CPU_FREQ */

int mfc_init_pm(struct mfc_dev *mfcdev)
{
	struct clk *parent, *sclk;
	int ret = 0;

	pm = &mfcdev->pm;

	parent = clk_get(mfcdev->device, MFC_PARENT_CLK_NAME);
	if (IS_ERR(parent)) {
		printk(KERN_ERR "failed to get parent clock\n");
		ret = -ENOENT;
		goto err_gp_clk;
	}

	sclk = clk_get(mfcdev->device, MFC_CLKNAME);
	if (IS_ERR(sclk)) {
		printk(KERN_ERR "failed to get source clock\n");
		ret = -ENOENT;
		goto err_gs_clk;
	}

	ret = clk_set_parent(sclk, parent);
	if (ret) {
		printk(KERN_ERR "unable to set parent %s of clock %s\n",
				parent->name, sclk->name);
		goto err_sp_clk;
	}

	/* FIXME: clock name & rate have to move to machine code */
	ret = clk_set_rate(sclk, mfc_clk_rate);
	if (ret) {
		printk(KERN_ERR "%s rate change failed: %u\n", sclk->name, 200 * 1000000);
		goto err_ss_clk;
	}

	/* clock for gating */
	pm->clock = clk_get(mfcdev->device, MFC_GATE_CLK_NAME);
	if (IS_ERR(pm->clock)) {
		printk(KERN_ERR "failed to get clock-gating control\n");
		ret = -ENOENT;
		goto err_gg_clk;
	}

	atomic_set(&pm->power, 0);

#if defined(CONFIG_PM_RUNTIME) || defined(CONFIG_CPU_FREQ)
	pm->device = mfcdev->device;
#endif

#ifdef CONFIG_PM_RUNTIME
	pm_runtime_enable(pm->device);
#endif

#ifdef CLK_DEBUG
	atomic_set(&clk_ref, 0);
#endif

#ifdef CONFIG_CPU_FREQ
	pm->op_clk = sclk;
	ret = mfc_cpufreq_register();
	if (ret < 0) {
		dev_err(mfcdev->device, "Failed to register cpufreq\n");
		goto err_cpufreq;
	}
#endif

	return 0;

#ifdef CONFIG_CPU_FREQ
err_cpufreq:
	clk_put(pm->clock);
#endif
err_gg_clk:
err_ss_clk:
err_sp_clk:
	clk_put(sclk);
err_gs_clk:
	clk_put(parent);
err_gp_clk:
	return ret;
}

void mfc_final_pm(struct mfc_dev *mfcdev)
{
	clk_put(pm->clock);

#ifdef CONFIG_PM_RUNTIME
	pm_runtime_disable(pm->device);
#endif
}

int mfc_clock_on(void)
{
#ifdef CLK_DEBUG
	atomic_inc(&clk_ref);
	mfc_dbg("+ %d", atomic_read(&clk_ref));
#endif

#ifdef CONFIG_PM_RUNTIME
	return clk_enable(pm->clock);
#else
	return clk_enable(pm->clock);
#endif
}

void mfc_clock_off(void)
{
#ifdef CLK_DEBUG
	atomic_dec(&clk_ref);
	mfc_dbg("- %d", atomic_read(&clk_ref));
#endif

#ifdef CONFIG_PM_RUNTIME
	clk_disable(pm->clock);
#else
	clk_disable(pm->clock);
#endif
}

int mfc_power_on(void)
{
#ifdef CONFIG_PM_RUNTIME
#ifdef MFC_NO_POWER_GATING
	if (soc_is_exynos4212() || soc_is_exynos4412())
		return 0;
	else
#endif
		return pm_runtime_get_sync(pm->device);
#else
	atomic_set(&pm->power, 1);

	return 0;
#endif
}

int mfc_power_off(void)
{
#ifdef CONFIG_PM_RUNTIME
#ifdef MFC_NO_POWER_GATING
	if (soc_is_exynos4212() || soc_is_exynos4412())
		return 0;
	else
#endif
		return pm_runtime_put_sync(pm->device);
#else
	atomic_set(&pm->power, 0);

	return 0;
#endif
}

bool mfc_power_chk(void)
{
	mfc_dbg("%s", atomic_read(&pm->power) ? "on" : "off");

	return atomic_read(&pm->power) ? true : false;
}

void mfc_pd_enable(void)
{
	u32 timeout;

	__raw_writel(S5P_INT_LOCAL_PWR_EN, S5P_PMU_MFC_CONF);

	/* Wait max 1ms */
	timeout = 10;
	while ((__raw_readl(S5P_PMU_MFC_CONF + 0x4) & S5P_INT_LOCAL_PWR_EN)
		!= S5P_INT_LOCAL_PWR_EN) {
		if (timeout == 0) {
			printk(KERN_ERR "Power domain MFC enable failed.\n");
			break;
		}

		timeout--;

		udelay(100);
	}
}

