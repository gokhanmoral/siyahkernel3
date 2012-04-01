/* linux/arch/arm/mach-exynos/cpufreq.c
 *
 * Copyright (c) 2010-2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * EXYNOS - CPU frequency scaling support for EXYNOS series
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/regulator/consumer.h>
#include <linux/cpufreq.h>
#include <linux/suspend.h>
#include <linux/reboot.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-mem.h>
#include <mach/cpufreq.h>
#include <mach/asv.h>

#include <plat/clock.h>
#include <plat/pm.h>
#include <plat/cpu.h>

struct exynos_dvfs_info *exynos_info;

static struct regulator *arm_regulator;
static struct cpufreq_freqs freqs;

static bool exynos_cpufreq_disable;
static bool exynos_cpufreq_lock_disable;
static bool exynos_cpufreq_init_done;
static DEFINE_MUTEX(set_freq_lock);
static DEFINE_MUTEX(set_cpu_freq_lock);

unsigned int g_cpufreq_limit_id;
unsigned int g_cpufreq_limit_val[DVFS_LOCK_ID_END];
unsigned int g_cpufreq_limit_level;

unsigned int g_cpufreq_lock_id;
unsigned int g_cpufreq_lock_val[DVFS_LOCK_ID_END];
unsigned int g_cpufreq_lock_level;

int exynos_verify_speed(struct cpufreq_policy *policy)
{
	return cpufreq_frequency_table_verify(policy,
					      exynos_info->freq_table);
}

unsigned int exynos_getspeed(unsigned int cpu)
{
	return clk_get_rate(exynos_info->cpu_clk) / 1000;
}

static unsigned int exynos_get_safe_armvolt(unsigned int old_index, unsigned int new_index)
{
	unsigned int safe_arm_volt = 0;
	struct cpufreq_frequency_table *freq_table = exynos_info->freq_table;
	unsigned int *volt_table = exynos_info->volt_table;

	/*
	 * ARM clock source will be changed APLL to MPLL temporary
	 * To support this level, need to control regulator for
	 * reguired voltage level
	 */

	if (exynos_info->need_apll_change != NULL) {
		if (exynos_info->need_apll_change(old_index, new_index) &&
			(freq_table[new_index].frequency < exynos_info->mpll_freq_khz) &&
			(freq_table[old_index].frequency < exynos_info->mpll_freq_khz)) {
				safe_arm_volt = volt_table[exynos_info->pll_safe_idx];
			}

	}

	return safe_arm_volt;
}

unsigned int smooth_target = L2;
unsigned int smooth_offset = 2;
unsigned int smooth_step = 2;
static int exynos_target(struct cpufreq_policy *policy,
			  unsigned int target_freq,
			  unsigned int relation)
{
	unsigned int index, old_index;
	unsigned int arm_volt, safe_arm_volt = 0;
	int ret = 0;
	struct cpufreq_frequency_table *freq_table = exynos_info->freq_table;
	unsigned int *volt_table = exynos_info->volt_table;

	mutex_lock(&set_freq_lock);

	if (exynos_cpufreq_disable)
		goto out;

	freqs.old = exynos_getspeed(policy->cpu);

	if(policy->max < freqs.old || policy->min > freqs.old)
	{
		struct cpufreq_policy policytemp;
		memcpy(&policytemp, policy, sizeof(struct cpufreq_policy));
		policytemp.max = freqs.old;
		policytemp.min = freqs.old;
		if (cpufreq_frequency_table_target(&policytemp, freq_table,
						   freqs.old, relation, &old_index)) {
			ret = -EINVAL;
			goto out;
		}
	} else
	if (cpufreq_frequency_table_target(policy, freq_table,
					   freqs.old, relation, &old_index)) {
		ret = -EINVAL;
		goto out;
	}

	if (cpufreq_frequency_table_target(policy, freq_table,
					   target_freq, relation, &index)) {
		ret = -EINVAL;
		goto out;
	}

/* prevent freqs going above max policy - originally by netarchy */
	if(exynos_info->max_current_idx > index)
		index = exynos_info->max_current_idx;

	/* Need to set performance limitation */
	if (!exynos_cpufreq_lock_disable && (index > g_cpufreq_lock_level))
		index = g_cpufreq_lock_level;

	if (index < g_cpufreq_limit_level)
		index = g_cpufreq_limit_level;

#if defined(CONFIG_CPU_EXYNOS4210)
	/* Do NOT step up max arm clock directly to reduce power consumption */
	//reach 1200MHz step by step starting from 800MHz -gm
	if(index <= smooth_target && index < old_index)
	{
		index = max(index,min(smooth_target + smooth_offset, old_index - smooth_step));
	}
#endif

	freqs.new = freq_table[index].frequency;
	freqs.cpu = policy->cpu;

	safe_arm_volt = exynos_get_safe_armvolt(old_index, index);

	arm_volt = volt_table[index];

	cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);

	/* When the new frequency is higher than current frequency */
	if ((freqs.new > freqs.old) && !safe_arm_volt) {
		/* Firstly, voltage up to increase frequency */
		regulator_set_voltage(arm_regulator, arm_volt,
				     arm_volt + 25000);
	}

	if (safe_arm_volt)
		regulator_set_voltage(arm_regulator, safe_arm_volt,
				     safe_arm_volt + 25000);
	if (freqs.new != freqs.old)
		exynos_info->set_freq(old_index, index);

	cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);

	/* When the new frequency is lower than current frequency */
	if ((freqs.new < freqs.old) ||
	   ((freqs.new > freqs.old) && safe_arm_volt)) {
		/* down the voltage after frequency change */
		regulator_set_voltage(arm_regulator, arm_volt,
				     arm_volt + 25000);
	}

out:
	mutex_unlock(&set_freq_lock);

	return ret;
}

int exynos_cpufreq_get_level(unsigned int freq, unsigned int *level)
{
	struct cpufreq_frequency_table *table;
	unsigned int i;

	if (!exynos_cpufreq_init_done)
		return -EINVAL;

	table = cpufreq_frequency_get_table(0);
	if (!table) {
		pr_err("%s: Failed to get the cpufreq table\n", __func__);
		return -EINVAL;
	}

	for (i = exynos_info->max_support_idx;
		(table[i].frequency != CPUFREQ_TABLE_END); i++) {
		if (table[i].frequency == freq) {
			*level = i;
			return 0;
		}
	}

	pr_err("%s: %u KHz is an unsupported cpufreq\n", __func__, freq);

	return -EINVAL;
}
EXPORT_SYMBOL_GPL(exynos_cpufreq_get_level);

atomic_t exynos_cpufreq_lock_count;

int exynos_cpufreq_lock(unsigned int nId,
			 enum cpufreq_level_index cpufreq_level)
{
	int ret = 0, i, old_idx = 0;
	unsigned int freq_old, freq_new, arm_volt, safe_arm_volt;
	unsigned int *volt_table;
	struct cpufreq_policy *policy;
	struct cpufreq_frequency_table *freq_table;

	if (!exynos_cpufreq_init_done)
		return -EPERM;

	if (!exynos_info)
		return -EPERM;

	policy = cpufreq_cpu_get(0);
	if (!policy)
		return -EPERM;

	volt_table = exynos_info->volt_table;
	freq_table = exynos_info->freq_table;

	if (g_cpufreq_lock_id & (1 << nId)) {
		printk(KERN_ERR "%s:Device [%d] already locked cpufreq\n",
				__func__,  nId);
		return 0;
	}

	volt_table = exynos_info->volt_table;
	policy = cpufreq_cpu_get(0);
	freq_table = exynos_info->freq_table;

	//do not lock to higher than L2 -gm
	cpufreq_level = max(L2, (int)cpufreq_level);

	mutex_lock(&set_cpu_freq_lock);
	g_cpufreq_lock_id |= (1 << nId);
	g_cpufreq_lock_val[nId] = cpufreq_level;

	/* If the requested cpufreq is higher than current min frequency */
	if (cpufreq_level < g_cpufreq_lock_level)
		g_cpufreq_lock_level = cpufreq_level;

	mutex_unlock(&set_cpu_freq_lock);

	if ((g_cpufreq_lock_level < g_cpufreq_limit_level)
				&& (nId != DVFS_LOCK_ID_PM))
		return 0;

	/* Do not setting cpufreq lock frequency
	 * because current governor doesn't support dvfs level lock
	 * except DVFS_LOCK_ID_PM */
	if (exynos_cpufreq_lock_disable && (nId != DVFS_LOCK_ID_PM))
		return 0;

	/* If current frequency is lower than requested freq,
	 * it needs to update
	 */
	mutex_lock(&set_freq_lock);
	freq_old = policy->cur;
	freq_new = freq_table[cpufreq_level].frequency;
	if (freq_old < freq_new) {
		/* Find out current level index */
		for (i = 0 ; i <= exynos_info->min_support_idx;  i++) {
			if (freq_old == freq_table[i].frequency) {
				old_idx = freq_table[i].index;
				break;
			} else if (i == exynos_info->min_support_idx) {
				printk(KERN_ERR "%s: Level not found\n",
					__func__);
				mutex_unlock(&set_freq_lock);
				return -EINVAL;
			} else {
				continue;
			}
		}
		freqs.old = freq_old;
		freqs.new = freq_new;
		cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);

		/* get the voltage value */
		safe_arm_volt = exynos_get_safe_armvolt(old_idx, cpufreq_level);
		if (safe_arm_volt)
			regulator_set_voltage(arm_regulator, safe_arm_volt,
					     safe_arm_volt + 25000);

		arm_volt = volt_table[cpufreq_level];
		regulator_set_voltage(arm_regulator, arm_volt,
				     arm_volt + 25000);

		exynos_info->set_freq(old_idx, cpufreq_level);

		cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
	}
	mutex_unlock(&set_freq_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(exynos_cpufreq_lock);

void exynos_cpufreq_lock_free(unsigned int nId)
{
	unsigned int i;

	if (!exynos_cpufreq_init_done)
		return;

	mutex_lock(&set_cpu_freq_lock);
	g_cpufreq_lock_id &= ~(1 << nId);
	g_cpufreq_lock_val[nId] = exynos_info->min_support_idx;
	g_cpufreq_lock_level = exynos_info->min_support_idx;
	if (g_cpufreq_lock_id) {
		for (i = 0; i < DVFS_LOCK_ID_END; i++) {
			if (g_cpufreq_lock_val[i] < g_cpufreq_lock_level)
				g_cpufreq_lock_level = g_cpufreq_lock_val[i];
		}
	}
	mutex_unlock(&set_cpu_freq_lock);
}
EXPORT_SYMBOL_GPL(exynos_cpufreq_lock_free);

int exynos_cpufreq_upper_limit(unsigned int nId,
				enum cpufreq_level_index cpufreq_level)
{
	int ret = 0, old_idx = 0, i;
	unsigned int freq_old, freq_new, arm_volt, safe_arm_volt;
	unsigned int *volt_table;
	struct cpufreq_policy *policy;
	struct cpufreq_frequency_table *freq_table;

	if (!exynos_cpufreq_init_done)
		return -EPERM;

	if (!exynos_info)
		return -EPERM;

	policy = cpufreq_cpu_get(0);
	if (!policy)
		return -EPERM;

	volt_table = exynos_info->volt_table;
	freq_table = exynos_info->freq_table;

	if (g_cpufreq_limit_id & (1 << nId)) {
		pr_err("[CPUFREQ]This device [%d] already limited cpufreq\n", nId);
		return 0;
	}

	volt_table = exynos_info->volt_table;
	policy = cpufreq_cpu_get(0);
	freq_table = exynos_info->freq_table;

	mutex_lock(&set_cpu_freq_lock);
	g_cpufreq_limit_id |= (1 << nId);
	g_cpufreq_limit_val[nId] = cpufreq_level;

	/* If the requested limit level is lower than current value */
	if (cpufreq_level > g_cpufreq_limit_level)
		g_cpufreq_limit_level = cpufreq_level;

	mutex_unlock(&set_cpu_freq_lock);

	mutex_lock(&set_freq_lock);
	/* If cur frequency is higher than limit freq, it needs to update */
	freq_old = policy->cur;
	freq_new = freq_table[cpufreq_level].frequency;
	if (freq_old > freq_new) {
		/* Find out current level index */
		for (i = 0; i <= exynos_info->min_support_idx; i++) {
			if (freq_old == freq_table[i].frequency) {
				old_idx = freq_table[i].index;
				break;
			} else if (i == exynos_info->min_support_idx) {
				printk(KERN_ERR "%s: Level is not found\n", __func__);
				mutex_unlock(&set_freq_lock);

				return -EINVAL;
			} else {
				continue;
			}
		}
		freqs.old = freq_old;
		freqs.new = freq_new;

		cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);

		exynos_info->set_freq(old_idx, cpufreq_level);

		safe_arm_volt = exynos_get_safe_armvolt(old_idx, cpufreq_level);
		if (safe_arm_volt)
			regulator_set_voltage(arm_regulator, safe_arm_volt,
					     safe_arm_volt + 25000);

		arm_volt = volt_table[cpufreq_level];
		regulator_set_voltage(arm_regulator, arm_volt, arm_volt + 25000);

		cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
	}

	mutex_unlock(&set_freq_lock);

	return ret;
}

void exynos_cpufreq_upper_limit_free(unsigned int nId)
{
	unsigned int i;

	if (!exynos_cpufreq_init_done)
		return;

	mutex_lock(&set_cpu_freq_lock);
	g_cpufreq_limit_id &= ~(1 << nId);
	g_cpufreq_limit_val[nId] = exynos_info->max_support_idx;
	g_cpufreq_limit_level = exynos_info->max_support_idx;

	if (g_cpufreq_limit_id) {
		for (i = 0; i < DVFS_LOCK_ID_END; i++) {
			if (g_cpufreq_limit_val[i] > g_cpufreq_limit_level)
				g_cpufreq_limit_level = g_cpufreq_limit_val[i];
		}
	}
	mutex_unlock(&set_cpu_freq_lock);
}

#ifdef CONFIG_PM
static int exynos_cpufreq_suspend(struct cpufreq_policy *policy)
{
	return 0;
}

static int exynos_cpufreq_resume(struct cpufreq_policy *policy)
{
	return 0;
}
#endif

static int exynos_cpufreq_notifier_event(struct notifier_block *this,
		unsigned long event, void *ptr)
{
	int ret = 0;
	unsigned int safe_arm_volt, arm_volt;
	unsigned int *volt_table;
	enum cpufreq_level_index level;
	struct cpufreq_policy *policy = ptr;

	volt_table = exynos_info->volt_table;

	switch (event) {
	case PM_SUSPEND_PREPARE:
		ret = exynos_cpufreq_lock(DVFS_LOCK_ID_PM,
					   exynos_info->pm_lock_idx);
		if (ret < 0)
			return NOTIFY_BAD;
#if defined(CONFIG_CPU_EXYNOS4210)
		ret = exynos_cpufreq_upper_limit(DVFS_LOCK_ID_PM,
						exynos_info->pm_lock_idx);
		if (ret < 0)
			return NOTIFY_BAD;
#endif

		exynos_cpufreq_disable = true;

		pr_debug("PM_SUSPEND_PREPARE for CPUFREQ\n");
		return NOTIFY_OK;
	case PM_POST_RESTORE:
	case PM_POST_SUSPEND:
		pr_debug("PM_POST_SUSPEND for CPUFREQ: %d\n", ret);
		exynos_cpufreq_lock_free(DVFS_LOCK_ID_PM);
#if defined(CONFIG_CPU_EXYNOS4210)
		exynos_cpufreq_upper_limit_free(DVFS_LOCK_ID_PM);
#endif
		// In case of using performance governor,
		// max level should be used after sleep and wakeup
		if (exynos_cpufreq_lock_disable) {
			mutex_lock(&set_freq_lock);
			exynos_cpufreq_get_level(policy->max, &level);

			/* get the voltage value */
			safe_arm_volt = exynos_get_safe_armvolt(exynos_info->pm_lock_idx, level);
			if (safe_arm_volt)
				regulator_set_voltage(arm_regulator, safe_arm_volt,
					safe_arm_volt + 25000);

			arm_volt = volt_table[level];
			regulator_set_voltage(arm_regulator, arm_volt,
				arm_volt + 25000);

			exynos_info->set_freq(exynos_info->pm_lock_idx, level);

			mutex_unlock(&set_freq_lock);
		}
		exynos_cpufreq_disable = false;

		return NOTIFY_OK;
	}
	return NOTIFY_DONE;
}

static struct notifier_block exynos_cpufreq_notifier = {
	.notifier_call = exynos_cpufreq_notifier_event,
	.priority = INT_MIN, /* done last */
};

static int exynos_cpufreq_policy_notifier_call(struct notifier_block *this,
				unsigned long code, void *data)
{
	struct cpufreq_policy *policy = data;
	enum cpufreq_level_index level;

	switch (code) {
	case CPUFREQ_ADJUST:
		if ((!strnicmp(policy->governor->name, "powersave", CPUFREQ_NAME_LEN))
		|| (!strnicmp(policy->governor->name, "performance", CPUFREQ_NAME_LEN))
		|| (!strnicmp(policy->governor->name, "userspace", CPUFREQ_NAME_LEN))) {
			printk(KERN_DEBUG "cpufreq governor is changed to %s\n",
							policy->governor->name);
			exynos_cpufreq_lock_disable = true;
		} else
			exynos_cpufreq_lock_disable = false;
		break;
	case CPUFREQ_INCOMPATIBLE:
		break;
	case CPUFREQ_NOTIFY:
		exynos_cpufreq_get_level(policy->max, &level);
		if(level!=-EINVAL) exynos_info->max_current_idx = level;
		exynos_cpufreq_get_level(policy->min, &level);
		if(level!=-EINVAL) exynos_info->min_current_idx = level;
	default:
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block exynos_cpufreq_policy_notifier = {
	.notifier_call = exynos_cpufreq_policy_notifier_call,
};


static int exynos_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
	int ret;

	policy->cur = policy->min = policy->max = exynos_getspeed(policy->cpu);

	cpufreq_frequency_table_get_attr(exynos_info->freq_table, policy->cpu);

	/* set the transition latency value */
	policy->cpuinfo.transition_latency = 100000;

	/*
	 * EXYNOS4 multi-core processors has 2 cores
	 * that the frequency cannot be set independently.
	 * Each cpu is bound to the same speed.
	 * So the affected cpu is all of the cpus.
	 */
	if (num_online_cpus() == 1) {
		cpumask_copy(policy->related_cpus, cpu_possible_mask);
		cpumask_copy(policy->cpus, cpu_online_mask);
	} else {
		cpumask_setall(policy->cpus);
	}

	ret = cpufreq_frequency_table_cpuinfo(policy, exynos_info->freq_table);
	/* set safe default min and max speeds - netarchy */
	policy->max = exynos_info->freq_table[exynos_info->max_current_idx].frequency;
	policy->min = exynos_info->freq_table[exynos_info->min_current_idx].frequency;
	return ret;
}

/* Make sure we have the scaling_available_freqs sysfs file */
static struct freq_attr *exynos_cpufreq_attr[] = {
        &cpufreq_freq_attr_scaling_available_freqs,
        NULL,
}; 

static int exynos_cpufreq_reboot_notifier_call(struct notifier_block *this,
				   unsigned long code, void *_cmd)
{
	int ret = 0;

	ret = exynos_cpufreq_lock(DVFS_LOCK_ID_PM, exynos_info->pm_lock_idx);
	if (ret < 0)
		return NOTIFY_BAD;

	printk(KERN_INFO "REBOOT Notifier for CPUFREQ\n");
	return NOTIFY_DONE;
}

static struct notifier_block exynos_cpufreq_reboot_notifier = {
	.notifier_call = exynos_cpufreq_reboot_notifier_call,
};

static struct cpufreq_driver exynos_driver = {
	.flags		= CPUFREQ_STICKY,
	.verify		= exynos_verify_speed,
	.target		= exynos_target,
	.get		= exynos_getspeed,
	.init		= exynos_cpufreq_cpu_init,
	.name		= "exynos_cpufreq",
	.attr		= exynos_cpufreq_attr,
#ifdef CONFIG_PM
	.suspend	= exynos_cpufreq_suspend,
	.resume		= exynos_cpufreq_resume,
#endif
};

static int __init exynos_cpufreq_init(void)
{
	int ret = -EINVAL;
	int i;

	exynos_info = kzalloc(sizeof(struct exynos_dvfs_info), GFP_KERNEL);
	if (!exynos_info)
		return -ENOMEM;

	if (soc_is_exynos4210())
		ret = exynos4210_cpufreq_init(exynos_info);
	else if (soc_is_exynos4212() || soc_is_exynos4412())
		ret = exynos4x12_cpufreq_init(exynos_info);
	else if (soc_is_exynos5250())
		ret = exynos5250_cpufreq_init(exynos_info);
	else
		pr_err("%s: CPU type not found\n", __func__);

	if (ret)
		goto err_vdd_arm;

	if (exynos_info->set_freq == NULL) {
		printk(KERN_ERR "%s: No set_freq function (ERR)\n",
				__func__);
		goto err_vdd_arm;
	}

	arm_regulator = regulator_get(NULL, "vdd_arm");
	if (IS_ERR(arm_regulator)) {
		printk(KERN_ERR "failed to get resource %s\n", "vdd_arm");
		goto err_vdd_arm;
	}

	exynos_cpufreq_disable = false;

	register_pm_notifier(&exynos_cpufreq_notifier);
	register_reboot_notifier(&exynos_cpufreq_reboot_notifier);
	cpufreq_register_notifier(&exynos_cpufreq_policy_notifier,
						CPUFREQ_POLICY_NOTIFIER);

	exynos_cpufreq_init_done = true;

	for (i = 0; i < DVFS_LOCK_ID_END; i++) {
		g_cpufreq_lock_val[i] = exynos_info->min_support_idx;
		g_cpufreq_limit_val[i] = exynos_info->max_support_idx;
	}

	g_cpufreq_lock_level = exynos_info->min_support_idx;
	g_cpufreq_limit_level = exynos_info->max_support_idx;

	if (cpufreq_register_driver(&exynos_driver)) {
		pr_err("failed to register cpufreq driver\n");
		goto err_cpufreq;
	}

	return 0;
err_cpufreq:
	unregister_reboot_notifier(&exynos_cpufreq_reboot_notifier);
	unregister_pm_notifier(&exynos_cpufreq_notifier);

	if (!IS_ERR(arm_regulator))
		regulator_put(arm_regulator);
err_vdd_arm:
	kfree(exynos_info);
	pr_debug("%s: failed initialization\n", __func__);
	return -EINVAL;
}
late_initcall(exynos_cpufreq_init);


ssize_t show_UV_mV_table(struct cpufreq_policy *policy, char *buf) {
int i, len = 0;
if (buf)
{
for (i = exynos_info->max_support_idx; i<=exynos_info->min_support_idx; i++)
{
if(exynos_info->freq_table[i].frequency==CPUFREQ_ENTRY_INVALID) continue;
len += sprintf(buf + len, "%dmhz: %d mV\n", exynos_info->freq_table[i].frequency/1000,exynos_info->volt_table[i]/1000);
}
}
return len;
}


#define VREF_SEL     1	/* 0: 0.625V (50mV step), 1: 0.3125V (25mV step). */
#define V_STEP       (25 * (2 - VREF_SEL)) /* Minimum voltage step size. */
#define VREG_DATA    (VREG_CONFIG | (VREF_SEL << 5))
#define VREG_CONFIG  (BIT(7) | BIT(6)) /* Enable VREG, pull-down if disabled. */
/* Cause a compile error if the voltage is not a multiple of the step size. */
#define MV(mv)      ((mv) / (!((mv) % V_STEP)))

ssize_t acpuclk_get_vdd_levels_str(char *buf)
{
int i, len = 0;
if (buf)
{
for (i = exynos_info->max_support_idx; i<=exynos_info->min_support_idx; i++)
{
if(exynos_info->freq_table[i].frequency==CPUFREQ_ENTRY_INVALID) continue;
len += sprintf(buf + len, "%8u: %4d\n", exynos_info->freq_table[i].frequency, exynos_info->volt_table[i]);
}
}
return len;
}

void acpuclk_set_vdd(unsigned int khz, unsigned int vdd)
{
int i;
unsigned int new_vdd;
vdd = vdd / V_STEP * V_STEP;
for (i = exynos_info->max_support_idx; i<=exynos_info->min_support_idx; i++)
{
if(exynos_info->freq_table[i].frequency==CPUFREQ_ENTRY_INVALID) continue;
if (khz == 0)
new_vdd = min(max((unsigned int)(exynos_info->volt_table[i] + vdd), (unsigned int)CPU_UV_MV_MIN), (unsigned int)CPU_UV_MV_MAX);
else if (exynos_info->freq_table[i].frequency == khz)
new_vdd = min(max((unsigned int)vdd, (unsigned int)CPU_UV_MV_MIN), (unsigned int)CPU_UV_MV_MAX);
else continue;

exynos_info->volt_table[i] = new_vdd;
}
}

ssize_t store_UV_mV_table(struct cpufreq_policy *policy,
                                      const char *buf, size_t count) {

      unsigned int ret = -EINVAL;
      int i = 0;
	  int u[8];
      ret = sscanf(buf, "%d %d %d %d %d %d %d %d", &u[0], &u[1], &u[2], &u[3], &u[4], &u[5], &u[6], &u[7]);
	  if(ret != 8) {
	      ret = sscanf(buf, "%d %d %d %d %d %d %d", &u[0], &u[1], &u[2], &u[3], &u[4], &u[5], &u[6]);
		  if(ret != 7) {
		      ret = sscanf(buf, "%d %d %d %d %d %d", &u[1], &u[2], &u[3], &u[4], &u[5], &u[6]);
			  if( ret != 6) return -EINVAL;
		  }
	  }
		for( i = 0; i < 8; i++ )
		{
			if (u[i] > CPU_UV_MV_MAX / 1000)
			{
				u[i] = CPU_UV_MV_MAX / 1000;
			}
			else if (u[i] < CPU_UV_MV_MIN / 1000)
			{
				u[i] = CPU_UV_MV_MIN / 1000;
			}
		}
		if(ret >= 7) exynos_info->volt_table[0] = u[0] * 1000;
		exynos_info->volt_table[1] = u[1] * 1000; 
		exynos_info->volt_table[2] = u[2] * 1000;
		exynos_info->volt_table[3] = u[3] * 1000; 
		exynos_info->volt_table[4] = u[4] * 1000;
		exynos_info->volt_table[5] = u[5] * 1000; 
		exynos_info->volt_table[6] = u[6] * 1000;
		if(ret == 8) exynos_info->volt_table[7] = u[7] * 1000;
		return count;
}
extern unsigned int smooth_step;
ssize_t show_smooth_step(struct cpufreq_policy *policy, char *buf) {
      return sprintf(buf, "%d\n", smooth_step);
}
ssize_t store_smooth_step(struct cpufreq_policy *policy,
                                      const char *buf, size_t count) {
	unsigned int ret = -EINVAL, level;
	ret = sscanf(buf, "%d", &level);
	if(ret!=1) return -EINVAL;
	if(level<0 || level>4) return -EINVAL;
	smooth_step = level;
	return count;
}
extern unsigned int smooth_target;
ssize_t show_smooth_target(struct cpufreq_policy *policy, char *buf) {
      return sprintf(buf, "%d\n", smooth_target);
}
ssize_t store_smooth_target(struct cpufreq_policy *policy,
                                      const char *buf, size_t count) {
	unsigned int ret = -EINVAL, level;
	ret = sscanf(buf, "%d", &level);
	if(ret!=1) return -EINVAL;
	if(level<0 || level>7) return -EINVAL;
	smooth_target = level;
	return count;
}
extern unsigned int smooth_offset;
ssize_t show_smooth_offset(struct cpufreq_policy *policy, char *buf) {
      return sprintf(buf, "%d\n", smooth_offset);
}
ssize_t store_smooth_offset(struct cpufreq_policy *policy,
                                      const char *buf, size_t count) {
	unsigned int ret = -EINVAL, level;
	ret = sscanf(buf, "%d", &level);
	if(ret!=1) return -EINVAL;
	if(level<0 || level>4) return -EINVAL;
	smooth_offset = level;
	return count;
}
