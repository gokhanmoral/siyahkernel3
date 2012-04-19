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

#define CPUMON 0

#define CHECK_DELAY	HZ >> 1
#define TRANS_LOAD_L	30
#define TRANS_LOAD_H    80
#define TRANS_LOAD_L_SCREEN_OFF 80
#define TRANS_LOAD_H_SCREEN_OFF 1000 //just to make sure that cpu1 is off to help didle

#define CPU1_ON_FREQ    800000

#define HOTPLUG_UNLOCKED 0
#define HOTPLUG_LOCKED 1

static struct workqueue_struct *hotplug_wq;

static struct delayed_work hotplug_work;

static unsigned int hotpluging_rate = CHECK_DELAY;
static unsigned int check_rate = CHECK_DELAY;
module_param_named(rate, check_rate, uint, 0644);
static unsigned int check_rate_cpuon = CHECK_DELAY << 1;
module_param_named(rate_cpuon, check_rate_cpuon, uint, 0644);
static unsigned int check_rate_scroff = CHECK_DELAY << 2;
module_param_named(rate_scroff, check_rate_scroff, uint, 0644);
static unsigned int freq_cpu1on = CPU1_ON_FREQ;
module_param_named(freq_cpu1on, freq_cpu1on, uint, 0644);
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

static void hotplug_timer(struct work_struct *work)
{
	unsigned int i, avg_load = 0, load = 0;
	unsigned int cur_freq;

	mutex_lock(&hotplug_lock);

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

	if ( ( (avg_load < (screen_off ? trans_load_l_off : trans_load_l)) ||
			(cur_freq < freq_cpu1on )
			)
		&& cpu_online(1) ) 
	{
		printk("cpu1 turning off!\n");
		cpu_down(1);
#if CPUMON
		printk(KERN_ERR "CPUMON D %d\n", avg_load);
#endif
		printk("cpu1 off end!\n");
		if(!screen_off) hotpluging_rate = check_rate;
		else hotpluging_rate = check_rate_scroff;
	} else if ( (   (avg_load > (screen_off ? trans_load_h_off : trans_load_h)) &&
			(cur_freq >= freq_cpu1on )
			) 
		&& !cpu_online(1) ) 
	{
		printk("cpu1 turning on!\n");
		cpu_up(1);
#if CPUMON
		printk(KERN_ERR "CPUMON U %d\n", avg_load);
#endif
		printk("cpu1 on end!\n");
		hotpluging_rate = check_rate_cpuon;
	}
 no_hotplug:

	queue_delayed_work_on(0, hotplug_wq, &hotplug_work, hotpluging_rate);

	mutex_unlock(&hotplug_lock);
}

static int s5pv310_pm_hotplug_notifier_event(struct notifier_block *this,
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

static struct notifier_block s5pv310_pm_hotplug_notifier = {
	.notifier_call = s5pv310_pm_hotplug_notifier_event,
};

static void hotplug_early_suspend(struct early_suspend *handler)
{
	mutex_lock(&hotplug_lock);
	screen_off = true;
	hotpluging_rate = check_rate_scroff;
	mutex_unlock(&hotplug_lock);
}

static void hotplug_late_resume(struct early_suspend *handler)
{
	printk(KERN_INFO "pm-hotplug: enable cpu auto-hotplug\n");

	mutex_lock(&hotplug_lock);
	screen_off = false;
	hotpluging_rate = check_rate;
	//cpu_up(1); //when the screen is on, activate the second cpu no matter what the load is
	queue_delayed_work_on(0, hotplug_wq, &hotplug_work, hotpluging_rate);
	mutex_unlock(&hotplug_lock);
}

static struct early_suspend hotplug_early_suspend_notifier = {
	.suspend = hotplug_early_suspend,
	.resume = hotplug_late_resume,
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
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

static int __init s5pv310_pm_hotplug_init(void)
{
	printk(KERN_INFO "SMDKV310 PM-hotplug init function\n");
	hotplug_wq = create_singlethread_workqueue("dynamic hotplug");
	if (!hotplug_wq) {
		printk(KERN_ERR "Creation of hotplug work failed\n");
		return -EFAULT;
	}

	INIT_DELAYED_WORK_DEFERRABLE(&hotplug_work, hotplug_timer);

	queue_delayed_work_on(0, hotplug_wq, &hotplug_work, 60 * HZ);

	register_pm_notifier(&s5pv310_pm_hotplug_notifier);
	register_reboot_notifier(&hotplug_reboot_notifier);
	register_early_suspend(&hotplug_early_suspend_notifier);

	return 0;
}

late_initcall(s5pv310_pm_hotplug_init);

static struct platform_device s5pv310_pm_hotplug_device = {
	.name = "s5pv310-dynamic-cpu-hotplug",
	.id = -1,
};

static int __init s5pv310_pm_hotplug_device_init(void)
{
	int ret;

	ret = platform_device_register(&s5pv310_pm_hotplug_device);

	if (ret) {
		printk(KERN_ERR "failed at(%d)\n", __LINE__);
		return ret;
	}

	printk(KERN_INFO "s5pv310_pm_hotplug_device_init: %d\n", ret);

	return ret;
}

late_initcall(s5pv310_pm_hotplug_device_init);
