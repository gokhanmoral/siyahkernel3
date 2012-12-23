/* linux/arch/arm/mach-s5pv310/pm-hotplug.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * S5PV310 - Dynamic CPU hotpluging
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/serial_core.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/cpu.h>
#include <linux/percpu.h>
#include <linux/ktime.h>
#include <linux/tick.h>
#include <linux/kernel_stat.h>
#include <linux/sched.h>
#include <linux/suspend.h>
#include <linux/reboot.h>
#include <linux/earlysuspend.h>

#include <plat/map-base.h>
#include <plat/gpio-cfg.h>

#include <mach/regs-gpio.h>
#include <mach/regs-irq.h>
#include <linux/gpio.h>
#include <linux/cpufreq.h>

#include <linux/device.h> 		//for second_core by tegrak
#include <linux/miscdevice.h> 	//for second_core by tegrak

#define CPUMON 1

#define CHECK_DELAY	(.5*HZ)
#define TRANS_LOAD_L	20
#define TRANS_LOAD_H	(TRANS_LOAD_L*3)
#define TRANS_LOAD_L_SCREEN_OFF 35
#define TRANS_LOAD_H_SCREEN_OFF 90

#define HOTPLUG_UNLOCKED 0
#define HOTPLUG_LOCKED 1

static struct workqueue_struct *hotplug_wq;

static struct delayed_work hotplug_work;

static unsigned int hotpluging_rate = CHECK_DELAY;
module_param_named(rate, hotpluging_rate, uint, 0644);
static unsigned int user_lock;
module_param_named(lock, user_lock, uint, 0644);
static unsigned int trans_load_l = TRANS_LOAD_L;
module_param_named(loadl, trans_load_l, uint, 0644);
static unsigned int trans_load_h = TRANS_LOAD_H;
module_param_named(loadh, trans_load_h, uint, 0644);
static unsigned int trans_load_l_off = TRANS_LOAD_L_SCREEN_OFF;
module_param_named(loadl_scroff, trans_load_l_off, uint, 0644);
static unsigned int trans_load_h_off = TRANS_LOAD_H_SCREEN_OFF;
module_param_named(loadh_scroff, trans_load_h_off, uint, 0644);
static unsigned int trans_load_l_inuse = TRANS_LOAD_L;
static unsigned int trans_load_h_inuse = TRANS_LOAD_H;

struct cpu_time_info {
	cputime64_t prev_cpu_idle;
	cputime64_t prev_cpu_wall;
	unsigned int load;
};

static DEFINE_PER_CPU(struct cpu_time_info, hotplug_cpu_time);

static bool screen_off;

/* mutex can be used since hotplug_timer does not run in
   timer(softirq) context but in process context */
static DEFINE_MUTEX(hotplug_lock);
static bool pm_hotplug_enabled = true;

/* Second core values by tegrak */
#define SECOND_CORE_VERSION (1)
int second_core_on = 1;
int hotplug_on = 1;

static void hotplug_timer(struct work_struct *work)
{
	unsigned int i, avg_load = 0, load = 0;
	unsigned int cur_freq;

	mutex_lock(&hotplug_lock);
	if(!pm_hotplug_enabled) {
		printk(KERN_INFO "pm-hotplug: disable cpu auto-hotplug\n");
		goto off_hotplug;
	}

	// exit if we turned off dynamic hotplug by tegrak
	// cancel the timer
	if (!hotplug_on) {
		if (!second_core_on && cpu_online(1) == 1)
			cpu_down(1);
		goto off_hotplug;
	}

	if (user_lock == 1)
		goto no_hotplug;

	for_each_online_cpu(i) {
		struct cpu_time_info *tmp_info;
		cputime64_t cur_wall_time, cur_idle_time;
		unsigned int idle_time, wall_time;

		tmp_info = &per_cpu(hotplug_cpu_time, i);

		cur_idle_time = get_cpu_idle_time_us(i, &cur_wall_time);

		idle_time = (unsigned int)cputime64_sub(cur_idle_time,
						tmp_info->prev_cpu_idle);
		tmp_info->prev_cpu_idle = cur_idle_time;

		wall_time = (unsigned int)cputime64_sub(cur_wall_time,
						tmp_info->prev_cpu_wall);
		tmp_info->prev_cpu_wall = cur_wall_time;

		if (wall_time < idle_time)
			goto no_hotplug;

		tmp_info->load = 100 * (wall_time - idle_time) / wall_time;

		load += tmp_info->load;
	}

	avg_load = load / num_online_cpus();

	cur_freq = cpufreq_get(0);

	if (((avg_load < trans_load_l_inuse) || (cur_freq <= 200 * 1000)) &&
	    (cpu_online(1) == 1)) {
		printk(KERN_INFO "cpu1 turning off!\n");
		cpu_down(1);
#if CPUMON
		printk(KERN_ERR "CPUMON D %d\n", avg_load);
#endif
		printk(KERN_INFO "cpu1 off end!\n");
		hotpluging_rate = CHECK_DELAY;
	} else if (((avg_load > trans_load_h_inuse) && (cur_freq > 200 * 1000)) &&
		   (cpu_online(1) == 0)) {
		printk(KERN_INFO "cpu1 turning on!\n");
		cpu_up(1);
#if CPUMON
		printk(KERN_ERR "CPUMON U %d\n", avg_load);
#endif
		printk(KERN_INFO "cpu1 on end!\n");
		hotpluging_rate = CHECK_DELAY * 4;
	}
 no_hotplug:

	queue_delayed_work_on(0, hotplug_wq, &hotplug_work, hotpluging_rate);

off_hotplug:
	mutex_unlock(&hotplug_lock);
}

static int exynos4_pm_hotplug_notifier_event(struct notifier_block *this,
					     unsigned long event, void *ptr)
{
	static unsigned user_lock_saved;

	switch (event) {
	case PM_SUSPEND_PREPARE:
		mutex_lock(&hotplug_lock);
		user_lock_saved = user_lock;
		user_lock = 1;
		pr_info("%s: saving pm_hotplug lock %x\n",
			__func__, user_lock_saved);
		mutex_unlock(&hotplug_lock);
		return NOTIFY_OK;
	case PM_POST_RESTORE:
	case PM_POST_SUSPEND:
		mutex_lock(&hotplug_lock);
		pr_info("%s: restoring pm_hotplug lock %x\n",
			__func__, user_lock_saved);
		user_lock = user_lock_saved;
		mutex_unlock(&hotplug_lock);
		return NOTIFY_OK;
	}
	return NOTIFY_DONE;
}

static struct notifier_block exynos4_pm_hotplug_notifier = {
	.notifier_call = exynos4_pm_hotplug_notifier_event,
};

static int hotplug_reboot_notifier_call(struct notifier_block *this,
					unsigned long code, void *_cmd)
{
	mutex_lock(&hotplug_lock);
	pr_err("%s: disabling pm hotplug\n", __func__);
	user_lock = 1;
	mutex_unlock(&hotplug_lock);

	return NOTIFY_DONE;
}

static struct notifier_block hotplug_reboot_notifier = {
	.notifier_call = hotplug_reboot_notifier_call,
};

static void hotplug_early_suspend(struct early_suspend *handler)
{
	mutex_lock(&hotplug_lock);
	screen_off = true;
	trans_load_l_inuse = trans_load_l_off;
	trans_load_h_inuse = trans_load_h_off;
	mutex_unlock(&hotplug_lock);
}

static void hotplug_late_resume(struct early_suspend *handler)
{
	printk(KERN_INFO "pm-hotplug: enable cpu auto-hotplug\n");

	mutex_lock(&hotplug_lock);
	screen_off = false;
	trans_load_l_inuse = trans_load_l;
	trans_load_h_inuse = trans_load_h;
	//cpu_up(1); //when the screen is on, activate the second cpu no matter what the load is
	//queue_delayed_work_on(0, hotplug_wq, &hotplug_work, hotpluging_rate);
	mutex_unlock(&hotplug_lock);
}

static struct early_suspend hotplug_early_suspend_notifier = {
	.suspend = hotplug_early_suspend,
	.resume = hotplug_late_resume,
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
};

/****************************************
 * DEVICE ATTRIBUTES FUNCTION by tegrak
****************************************/
#define declare_show(filename) \
	static ssize_t show_##filename(struct device *dev, struct device_attribute *attr, char *buf)

#define declare_store(filename) \
	static ssize_t store_##filename(\
		struct device *dev, struct device_attribute *attr, const char *buf, size_t size)

/****************************************
 * second_core attributes function by tegrak
 ****************************************/
declare_show(version) {
	return sprintf(buf, "%u\n", SECOND_CORE_VERSION);
}

declare_show(author) {
	return sprintf(buf, "Tegrak\n");
}

declare_show(hotplug_on) {
	return sprintf(buf, "%s\n", (hotplug_on) ? ("on") : ("off"));
}

declare_store(hotplug_on) {	
	mutex_lock(&hotplug_lock);
	
	if (user_lock) {
		goto finish;
	}
	
	if (!hotplug_on && strcmp(buf, "on\n") == 0) {
		hotplug_on = 1;
		// restart worker thread.
		hotpluging_rate = CHECK_DELAY;
		queue_delayed_work_on(0, hotplug_wq, &hotplug_work, hotpluging_rate);
		printk("second_core: hotplug is on!\n");
	}
	else if (hotplug_on && strcmp(buf, "off\n") == 0) {
		hotplug_on = 0;
		second_core_on = 1;
		if (cpu_online(1) == 0) {
			cpu_up(1);
		}
		printk("second_core: hotplug is off!\n");
	}
	
finish:
	mutex_unlock(&hotplug_lock);
	return size;
}

declare_show(second_core_on) {
	return sprintf(buf, "%s\n", (second_core_on) ? ("on") : ("off"));
}

declare_store(second_core_on) {
	mutex_lock(&hotplug_lock);
	
	if (hotplug_on || user_lock) {
		goto finish;
	}
	
	if (!second_core_on && strcmp(buf, "on\n") == 0) {
		second_core_on = 1;
		if (cpu_online(1) == 0) {
			cpu_up(1);
		}
		printk("second_core: 2nd core is always on!\n");
	}
	else if (second_core_on && strcmp(buf, "off\n") == 0) {
		second_core_on = 0;
		if (cpu_online(1) == 1) {
			cpu_down(1);
		}
		printk("second_core: 2nd core is always off!\n");
	}
	
finish:
	mutex_unlock(&hotplug_lock);
	return size;
}

/****************************************
 * DEVICE ATTRIBUTE by tegrak
 ****************************************/
#define declare_attr_rw(filename, perm) \
	static DEVICE_ATTR(filename, perm, show_##filename, store_##filename)
#define declare_attr_ro(filename, perm) \
	static DEVICE_ATTR(filename, perm, show_##filename, NULL)
#define declare_attr_wo(filename, perm) \
	static DEVICE_ATTR(filename, perm, NULL, store_##filename)

declare_attr_ro(version, 0444);
declare_attr_ro(author, 0444);
declare_attr_rw(hotplug_on, 0666);
declare_attr_rw(second_core_on, 0666);

static struct attribute *second_core_attributes[] = {
	&dev_attr_hotplug_on.attr, 
	&dev_attr_second_core_on.attr,
	&dev_attr_version.attr,
	&dev_attr_author.attr,
	NULL
};

static struct attribute_group second_core_group = {
		.attrs  = second_core_attributes,
};

static struct miscdevice second_core_device = {
		.minor = MISC_DYNAMIC_MINOR,
		.name = "second_core",
};

static int pm_hotplug_cpufreq_policy_notifier_call(struct notifier_block *this,
				unsigned long code, void *data)
{
	struct cpufreq_policy *policy = data;

	switch (code) {
	case CPUFREQ_ADJUST:
		if (
			(!strnicmp(policy->governor->name, "pegasusq", CPUFREQ_NAME_LEN)) ||
			(!strnicmp(policy->governor->name, "lulzactiveq", CPUFREQ_NAME_LEN)) ||
			(!strnicmp(policy->governor->name, "hotplug", CPUFREQ_NAME_LEN)) ||
			(!strnicmp(policy->governor->name, "assplug", CPUFREQ_NAME_LEN))
			) 
		{
			if(pm_hotplug_enabled)
			{
				pr_info("Pm-hotplug is disabled: governor=%s\n",
								policy->governor->name);
				mutex_lock(&hotplug_lock);
				pm_hotplug_enabled = false;
				mutex_unlock(&hotplug_lock);
			}
		} 
		else
		{
			if(!pm_hotplug_enabled)
			{
				pr_info("Pm-hotplug is enabled: governor=%s\n",
								policy->governor->name);
				mutex_lock(&hotplug_lock);
				pm_hotplug_enabled = true;
				queue_delayed_work_on(0, hotplug_wq, &hotplug_work, hotpluging_rate);
				mutex_unlock(&hotplug_lock);
			}
		}
		break;
	case CPUFREQ_INCOMPATIBLE:
	case CPUFREQ_NOTIFY:
	default:
		break;
	}

	return NOTIFY_DONE;
}
static struct notifier_block pm_hotplug_cpufreq_policy_notifier = {
	.notifier_call = pm_hotplug_cpufreq_policy_notifier_call,
};

static int __init exynos4_pm_hotplug_init(void)
{
	int ret;
	printk(KERN_INFO "EXYNOS4 PM-hotplug init function\n");
	/* hotplug_wq = create_workqueue("dynamic hotplug"); */
	hotplug_wq = alloc_workqueue("dynamic hotplug", 0, 0);
	if (!hotplug_wq) {
		printk(KERN_ERR "Creation of hotplug work failed\n");
		return -EFAULT;
	}

#if defined(CONFIG_CPU_FREQ_DEFAULT_GOV_PEGASUSQ) || \
	defined(CONFIG_CPU_FREQ_DEFAULT_GOV_HOTPLUG) || \
	defined(CONFIG_CPU_FREQ_DEFAULT_GOV_LULZACTIVEQ)
	pm_hotplug_enabled = 0;
#else
	pm_hotplug_enabled = 1;
#endif

	INIT_DELAYED_WORK(&hotplug_work, hotplug_timer);

	queue_delayed_work_on(0, hotplug_wq, &hotplug_work, 60 * HZ);

	register_pm_notifier(&exynos4_pm_hotplug_notifier);
	register_reboot_notifier(&hotplug_reboot_notifier);
	register_early_suspend(&hotplug_early_suspend_notifier);

	// register second_core device by tegrak
	ret = misc_register(&second_core_device);
	if (ret) {
	   printk(KERN_ERR "failed at(%d)\n", __LINE__);
		return ret;
	}

	ret = sysfs_create_group(&second_core_device.this_device->kobj, &second_core_group);
	if (ret)
	{
		printk(KERN_ERR "failed at(%d)\n", __LINE__);
		return ret;
	}

	cpufreq_register_notifier(&pm_hotplug_cpufreq_policy_notifier,
						CPUFREQ_POLICY_NOTIFIER);
	return 0;
}

late_initcall(exynos4_pm_hotplug_init);

static struct platform_device exynos4_pm_hotplug_device = {
	.name = "exynos4-dynamic-cpu-hotplug",
	.id = -1,
};

static int __init exynos4_pm_hotplug_device_init(void)
{
	int ret;

	ret = platform_device_register(&exynos4_pm_hotplug_device);

	if (ret) {
		printk(KERN_ERR "failed at(%d)\n", __LINE__);
		return ret;
	}

	printk(KERN_INFO "exynos4_pm_hotplug_device_init: %d\n", ret);

	return ret;
}

late_initcall(exynos4_pm_hotplug_device_init);
