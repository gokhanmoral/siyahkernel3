/* drivers/misc/m0_mods.c
 *
 * European S3 (i9300) specific hacks by gokhanmoral
 * almost no error control. expect panic if you use it on other devices/kernels
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/cpufreq.h>
#include <mach/cpufreq.h>
#include <mach/busfreq.h>
#include <linux/kallsyms.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>

unsigned int max_current_idx = L2;
unsigned int min_current_idx = L14;

static struct exynos_dvfs_info *gm_exynos_info;
static struct cpufreq_driver *gm_exynos_driver;
static int (*old_exynos_cpufreq_cpu_init)(struct cpufreq_policy *policy);
static void (*gm_exynos_cpufreq_upper_limit_free)(unsigned int nId);
static void (*gm_exynos_cpufreq_upper_limit)(unsigned int nId,
	enum cpufreq_level_index cpufreq_level);

static int gm_exynos_cpufreq_cpu_init(struct cpufreq_policy *policy)
{
	int ret;
	ret = (*old_exynos_cpufreq_cpu_init)(policy);
	/* set safe default min and max speeds - netarchy */
	policy->max = gm_exynos_info->freq_table[max_current_idx].frequency;
	policy->min = gm_exynos_info->freq_table[min_current_idx].frequency;
	return ret;
}

static int m0_cpufreq_policy_notifier_call(struct notifier_block *this,
				unsigned long code, void *data)
{
	struct cpufreq_policy *policy = data;
	enum cpufreq_level_index level;

	switch (code) {
	case CPUFREQ_ADJUST:
		exynos_cpufreq_get_level(policy->max, &level);
		if(level!=-EINVAL) max_current_idx = level;
		exynos_cpufreq_get_level(policy->min, &level);
		if(level!=-EINVAL) min_current_idx = level;
		(*gm_exynos_cpufreq_upper_limit_free)(DVFS_LOCK_ID_USER);
		(*gm_exynos_cpufreq_upper_limit)(DVFS_LOCK_ID_USER, max_current_idx);
		break;
	case CPUFREQ_INCOMPATIBLE:
	case CPUFREQ_NOTIFY:
	default:
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block m0_exynos_cpufreq_policy_notifier = {
	.notifier_call = m0_cpufreq_policy_notifier_call,
};

void enable_overclocked_frequencies(void)
{
	struct cpufreq_policy *policy = cpufreq_cpu_get(0);
	struct cpufreq_frequency_table *gm_exynos4x12_freq_table;
	void (*gm_cpufreq_stats_free_table)(unsigned int cpu);
	void (*gm_cpufreq_stats_create_table_cpu)(unsigned int cpu);
	int (*gm_cpufreq_stats_create_table)(struct cpufreq_policy *policy,
		struct cpufreq_frequency_table *table);
	void (*gm_cpufreq_stats_free_sysfs)(unsigned int cpu);
	static unsigned int *gm_exynos4x12_volt_table;
	unsigned int j = 0;
	enum cpufreq_level_index level;
	struct attribute_group *gm_stats_attr_group;

	gm_exynos4x12_freq_table = (struct cpufreq_frequency_table 	*)
			kallsyms_lookup_name("exynos4x12_freq_table");
	gm_exynos4x12_volt_table = (unsigned int *)
			kallsyms_lookup_name("exynos4x12_volt_table");
	gm_stats_attr_group = (struct attribute_group *)
			kallsyms_lookup_name("stats_attr_group");

	gm_cpufreq_stats_free_table = (void (*)(unsigned int))
			kallsyms_lookup_name("cpufreq_stats_free_table");
	gm_cpufreq_stats_free_sysfs = (void (*)(unsigned int))
			kallsyms_lookup_name("cpufreq_stats_free_sysfs");
	gm_cpufreq_stats_create_table = (int (*)(struct cpufreq_policy *, struct cpufreq_frequency_table *))
			kallsyms_lookup_name("cpufreq_stats_create_table");
	gm_cpufreq_stats_create_table_cpu = (void (*)(unsigned int))
			kallsyms_lookup_name("cpufreq_stats_create_table_cpu");

	if(gm_exynos4x12_freq_table[1].frequency == CPUFREQ_ENTRY_INVALID)
	{
		gm_exynos4x12_freq_table[1].frequency =
				gm_exynos4x12_freq_table[2].frequency + 100000;
		gm_exynos4x12_volt_table[1] = gm_exynos4x12_volt_table[2] + 50000;
	}
	if(gm_exynos4x12_freq_table[0].frequency == CPUFREQ_ENTRY_INVALID)
	{
		gm_exynos4x12_freq_table[0].frequency =
				gm_exynos4x12_freq_table[1].frequency + 100000;
		gm_exynos4x12_volt_table[0] = gm_exynos4x12_volt_table[1] + 50000;
	}
	gm_exynos_info->max_support_idx = 0;
	policy->cpuinfo.max_freq = 1600000;
	exynos_cpufreq_get_level(policy->min, &level);
	min_current_idx = level;
	exynos_cpufreq_get_level(policy->max, &level);
	max_current_idx = level;
	(*gm_exynos_cpufreq_upper_limit_free)(DVFS_LOCK_ID_USER);
	(*gm_exynos_cpufreq_upper_limit)(DVFS_LOCK_ID_USER, level);

	sysfs_remove_group(&policy->kobj, gm_stats_attr_group);
	for_each_cpu(j, policy->cpus) {
		(*gm_cpufreq_stats_free_table)(j);
	}
	(*gm_cpufreq_stats_create_table)(policy, gm_exynos4x12_freq_table);
}

void hijack_exynos_driver_init(void)
{
	gm_exynos_driver = (struct cpufreq_driver *)
			kallsyms_lookup_name("exynos_driver");
	gm_exynos_cpufreq_upper_limit_free = (void (*)(unsigned int))
			kallsyms_lookup_name("exynos_cpufreq_upper_limit_free");
	gm_exynos_cpufreq_upper_limit = (void (*)(unsigned int, enum cpufreq_level_index))
			kallsyms_lookup_name("exynos_cpufreq_upper_limit");
	old_exynos_cpufreq_cpu_init = gm_exynos_driver->init;
	gm_exynos_driver->init = gm_exynos_cpufreq_cpu_init;
}

void adjust_regulator_constraints(void)
{
	struct list_head *gm_regulator_list;
	struct regulator_dev *rdev;

	gm_regulator_list = (struct list_head *)kallsyms_lookup_name("regulator_list");
	list_for_each_entry(rdev, gm_regulator_list, list) {
		mutex_lock(&rdev->mutex);
		if(rdev->constraints && rdev->constraints->name)
		{
			if(strcmp(rdev->constraints->name, "vdd_g3d range") == 0)
			{
				rdev->constraints->min_uV = 600000;
				rdev->constraints->max_uV = 1400000;
			}
			if(strcmp(rdev->constraints->name, "vdd_arm range") == 0)
			{
				rdev->constraints->min_uV = 600000;
				rdev->constraints->max_uV = 1500000;
			}
		}
		mutex_unlock(&rdev->mutex);
	}
}

void release_exynos_driver_init(void)
{
	gm_exynos_driver->init = old_exynos_cpufreq_cpu_init;
}

struct device **gm_sec_touchscreen;

static ssize_t m0mods_touch_boost_level_write(struct device * dev, struct device_attribute * attr, const char * buf, size_t size)
{
	int lvl;
	int * gm_touch_boost_level;
	gm_touch_boost_level = (int*)(dev_get_drvdata(*gm_sec_touchscreen)) + 71;
    if (sscanf(buf, "%d", &lvl) == 1)
	{
		*gm_touch_boost_level = lvl;
	}
    return size;
}

static ssize_t m0mods_touch_boost_level_read(struct device * dev, struct device_attribute * attr, char * buf)
{
	int * gm_touch_boost_level;
	gm_touch_boost_level = (int*)(dev_get_drvdata(*gm_sec_touchscreen)) + 71;
    return sprintf(buf, "%d\n", *gm_touch_boost_level);
}

static DEVICE_ATTR(touch_boost_level, S_IRUGO | S_IWUGO, m0mods_touch_boost_level_read, m0mods_touch_boost_level_write);

static struct attribute *m0mods_attributes[] = 
    {
	&dev_attr_touch_boost_level.attr,
	NULL
    };

static struct attribute_group m0mods_group = 
    {
	.attrs  = m0mods_attributes,
    };

static struct miscdevice m0mods_device = 
    {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "m0mods",
    };

void create_m0mods_misc_device(void)
{
	int ret;
	
	gm_sec_touchscreen = (
		(struct device**)kallsyms_lookup_name("sec_touchscreen")
		);

    ret = misc_register(&m0mods_device);
    if (ret) return;
    if (sysfs_create_group(&m0mods_device.this_device->kobj, &m0mods_group) < 0) 
		return;
}

static int __init m0mods_init(void)
{
	if(kallsyms_lookup_name("rom_feature_set") == 0)
	{
		gm_exynos_info = *(
			(struct exynos_dvfs_info **)kallsyms_lookup_name("exynos_info")
			);
		cpufreq_register_notifier(&m0_exynos_cpufreq_policy_notifier,
						CPUFREQ_POLICY_NOTIFIER);
		adjust_regulator_constraints();
		hijack_exynos_driver_init();
		enable_overclocked_frequencies();
	}
	create_m0mods_misc_device();
    return 0;
}

static void __exit m0mods_exit(void)
{
	return;
}

module_init( m0mods_init );
module_exit( m0mods_exit );

MODULE_AUTHOR("Gokhan Moral <gm@alumni.bilkent.edu.tr>");
MODULE_DESCRIPTION("European i9300 specific hacks");
MODULE_LICENSE("GPL");
