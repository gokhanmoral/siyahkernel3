/* linux/arch/arm/mach-exynos/stand-hotplug.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS - Dynamic CPU hotpluging
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
#include <linux/gpio.h>
#include <linux/cpufreq.h>
#include <linux/device.h>       //for second_core by tegrak
#include <linux/miscdevice.h>   //for second_core by tegrak

#include <plat/map-base.h>
#include <plat/gpio-cfg.h>
#include <plat/s5p-clock.h>
#include <plat/clock.h>

#include <mach/regs-gpio.h>
#include <mach/regs-irq.h>

#if defined(CONFIG_MACH_P10)
#define TRANS_LOAD_H0 5
#define TRANS_LOAD_L1 2
#define TRANS_LOAD_H1 100

#define BOOT_DELAY	30
#define CHECK_DELAY_ON	(.5*HZ * 8)
#define CHECK_DELAY_OFF	(.5*HZ)

#endif

#if defined(CONFIG_MACH_U1) || defined(CONFIG_MACH_PX) || \
	defined(CONFIG_MACH_TRATS)
#define TRANS_LOAD_H0 30
#define TRANS_LOAD_L1 20
#define TRANS_LOAD_H1 100

#define BOOT_DELAY	60
#define CHECK_DELAY_ON	(.5*HZ * 4)
#define CHECK_DELAY_OFF	(.5*HZ)
#endif

#if defined(CONFIG_MACH_MIDAS) || defined(CONFIG_MACH_SMDK4X12) \
	|| defined(CONFIG_MACH_SLP_PQ)
#define TRANS_LOAD_H0 20
#define TRANS_LOAD_L1 10
#define TRANS_LOAD_H1 35
#define TRANS_LOAD_L2 15
#define TRANS_LOAD_H2 45
#define TRANS_LOAD_L3 20

#define BOOT_DELAY	60

#if defined(CONFIG_MACH_SLP_PQ)
#define CHECK_DELAY_ON	(.3*HZ * 4)
#define CHECK_DELAY_OFF	(.3*HZ)
#else
#define CHECK_DELAY_ON	(.5*HZ * 4)
#define CHECK_DELAY_OFF	(.5*HZ)
#endif
#endif

#define TRANS_RQ 2
#define TRANS_LOAD_RQ 20

#define CPU_OFF 0
#define CPU_ON  1

#define HOTPLUG_UNLOCKED 0
#define HOTPLUG_LOCKED 1
#define PM_HOTPLUG_DEBUG 0
#define NUM_CPUS num_possible_cpus()
#define CPULOAD_TABLE (NR_CPUS + 1)

#define DBG_PRINT(fmt, ...)\
	if(PM_HOTPLUG_DEBUG)			\
		printk(KERN_INFO pr_fmt(fmt), ##__VA_ARGS__)

static struct workqueue_struct *hotplug_wq;
static struct delayed_work hotplug_work;

static unsigned int max_performance;
static unsigned int freq_min = -1UL;
module_param_named(freq_min, freq_min, uint, 0644);

static unsigned int hotpluging_rate = CHECK_DELAY_OFF;
module_param_named(rate, hotpluging_rate, uint, 0644);
static unsigned int user_lock;
module_param_named(lock, user_lock, uint, 0644);
static unsigned int trans_rq= TRANS_RQ;
module_param_named(min_rq, trans_rq, uint, 0644);
static unsigned int trans_load_rq = TRANS_LOAD_RQ;
module_param_named(load_rq, trans_load_rq, uint, 0644);

static unsigned int trans_load_h0 = TRANS_LOAD_H0;
module_param_named(load_h0, trans_load_h0, uint, 0644);
static unsigned int trans_load_l1 = TRANS_LOAD_L1;
module_param_named(load_l1, trans_load_l1, uint, 0644);
static unsigned int trans_load_h1 = TRANS_LOAD_H1;
module_param_named(load_h1, trans_load_h1, uint, 0644);

#if (NR_CPUS > 2)
static unsigned int trans_load_l2 = TRANS_LOAD_L2;
module_param_named(load_l2, trans_load_l2, uint, 0644);
static unsigned int trans_load_h2 = TRANS_LOAD_H2;
module_param_named(load_h2, trans_load_h2, uint, 0644);
static unsigned int trans_load_l3 = TRANS_LOAD_L3;
module_param_named(load_l3, trans_load_l3, uint, 0644);
#endif

enum flag{
	HOTPLUG_NOP,
	HOTPLUG_IN,
	HOTPLUG_OUT
};

struct cpu_time_info {
	cputime64_t prev_cpu_idle;
	cputime64_t prev_cpu_wall;
	unsigned int load;
};

struct cpu_hotplug_info {
	unsigned long nr_running;
	pid_t tgid;
};

static DEFINE_PER_CPU(struct cpu_time_info, hotplug_cpu_time);

static bool standhotplug_enabled = true;

/* mutex can be used since hotplug_timer does not run in
   timer(softirq) context but in process context */
static DEFINE_MUTEX(hotplug_lock);
/* Second core values by tegrak */
#define SECOND_CORE_VERSION (1)
int second_core_on = 1;
int hotplug_on = 1;

bool hotplug_out_chk(unsigned int nr_online_cpu, unsigned int threshold_up,
		unsigned int avg_load, unsigned int cur_freq)
{
#if defined(CONFIG_MACH_P10)
	return ((nr_online_cpu > 1) &&
		(avg_load < threshold_up &&
		cur_freq <= freq_min));
#else
	return ((nr_online_cpu > 1) &&
		(avg_load < threshold_up ||
		cur_freq <= freq_min));
#endif
}

static inline enum flag
standalone_hotplug(unsigned int load, unsigned long nr_rq_min, unsigned int cpu_rq_min)
{
	unsigned int cur_freq;
	unsigned int nr_online_cpu;
	unsigned int avg_load;
	/*load threshold*/
	unsigned int threshold[CPULOAD_TABLE][2] = {
		{0, trans_load_h0},
		{trans_load_l1, trans_load_h1},
#if (NR_CPUS > 2)
		{trans_load_l2, trans_load_h2},
		{trans_load_l3, 100},
#endif
		{0, 0}
	};

	static void __iomem *clk_fimc;
	unsigned char fimc_stat;

	cur_freq = clk_get_rate(clk_get(NULL, "armclk")) / 1000;

	nr_online_cpu = num_online_cpus();

	avg_load = (unsigned int)((cur_freq * load) / max_performance);

	clk_fimc = ioremap(0x10020000, SZ_4K);
	fimc_stat = __raw_readl(clk_fimc + 0x0920);
	iounmap(clk_fimc);

	if ((fimc_stat>>4 & 0x1) == 1)
		return HOTPLUG_IN;

	if (hotplug_out_chk(nr_online_cpu, threshold[nr_online_cpu - 1][0],
			    avg_load, cur_freq)) {
		return HOTPLUG_OUT;
		/* If total nr_running is less than cpu(on-state) number, hotplug do not hotplug-in */
	} else if (nr_running() > nr_online_cpu &&
		   avg_load > threshold[nr_online_cpu - 1][1] && cur_freq > freq_min) {

		return HOTPLUG_IN;
#if defined(CONFIG_MACH_P10)
#else
	} else if (nr_online_cpu > 1 && nr_rq_min < trans_rq) {

		struct cpu_time_info *tmp_info;

		tmp_info = &per_cpu(hotplug_cpu_time, cpu_rq_min);
		/*If CPU(cpu_rq_min) load is less than trans_load_rq, hotplug-out*/
		if (tmp_info->load < trans_load_rq)
			return HOTPLUG_OUT;
#endif
	}

	return HOTPLUG_NOP;
}

static void hotplug_timer(struct work_struct *work)
{
	struct cpu_hotplug_info tmp_hotplug_info[4];
	int i;
	unsigned int load = 0;
	unsigned int cpu_rq_min=0;
	unsigned long nr_rq_min = -1UL;
	unsigned int select_off_cpu = 0;
	enum flag flag_hotplug;

	mutex_lock(&hotplug_lock);

	if(!standhotplug_enabled) {
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

#ifdef CONFIG_TARGET_LOCALE_P2TMO_TEMP
		/*For once Divide-by-Zero issue*/
		if (wall_time == 0)
			wall_time++;
#endif
		tmp_info->load = 100 * (wall_time - idle_time) / wall_time;

		load += tmp_info->load;
		/*find minimum runqueue length*/
		tmp_hotplug_info[i].nr_running = get_cpu_nr_running(i);

		if (i && nr_rq_min > tmp_hotplug_info[i].nr_running) {
			nr_rq_min = tmp_hotplug_info[i].nr_running;

			cpu_rq_min = i;
		}
	}

	for (i = NUM_CPUS - 1; i > 0; --i) {
		if (cpu_online(i) == 0) {
			select_off_cpu = i;
			break;
		}
	}

	/*standallone hotplug*/
	flag_hotplug = standalone_hotplug(load, nr_rq_min, cpu_rq_min);

	/*do not ever hotplug out CPU 0*/
	if((cpu_rq_min == 0) && (flag_hotplug == HOTPLUG_OUT))
		goto no_hotplug;

	/*cpu hotplug*/
	if (flag_hotplug == HOTPLUG_IN && cpu_online(select_off_cpu) == CPU_OFF) {
		DBG_PRINT("cpu%d turning on!\n", select_off_cpu);
		cpu_up(select_off_cpu);
		DBG_PRINT("cpu%d on\n", select_off_cpu);
		hotpluging_rate = CHECK_DELAY_ON;
	} else if (flag_hotplug == HOTPLUG_OUT && cpu_online(cpu_rq_min) == CPU_ON) {
		DBG_PRINT("cpu%d turnning off!\n", cpu_rq_min);
		cpu_down(cpu_rq_min);
		DBG_PRINT("cpu%d off!\n", cpu_rq_min);
		hotpluging_rate = CHECK_DELAY_OFF;
	} 

no_hotplug:
	//printk("hotplug_timer done.\n");

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
		hotpluging_rate = CHECK_DELAY_ON;
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


static int __init exynos4_pm_hotplug_init(void)
{
	int ret;
	unsigned int i;
	unsigned int freq;
	unsigned int freq_max = 0;
	struct cpufreq_frequency_table *table;

	printk(KERN_INFO "EXYNOS4 PM-hotplug init function\n");
	//hotplug_wq = create_workqueue("dynamic hotplug");
	hotplug_wq = alloc_workqueue("dynamic hotplug", 0, 0);
	if (!hotplug_wq) {
		printk(KERN_ERR "Creation of hotplug work failed\n");
		return -EFAULT;
	}

	INIT_DELAYED_WORK(&hotplug_work, hotplug_timer);

	queue_delayed_work_on(0, hotplug_wq, &hotplug_work, BOOT_DELAY * HZ);
#ifdef CONFIG_CPU_FREQ
	table = cpufreq_frequency_get_table(0);

	for (i = 0; table[i].frequency != CPUFREQ_TABLE_END; i++) {
		freq = table[i].frequency;

		if (freq != CPUFREQ_ENTRY_INVALID && freq > freq_max)
			freq_max = freq;
		else if (freq != CPUFREQ_ENTRY_INVALID && freq_min > freq)
			freq_min = freq;
	}
	/*get max frequence*/
	max_performance = freq_max * NUM_CPUS;
#else
	max_performance = clk_get_rate(clk_get(NULL, "armclk")) / 1000 * NUM_CPUS;
	freq_min = clk_get_rate(clk_get(NULL, "armclk")) / 1000;
#endif
	register_pm_notifier(&exynos4_pm_hotplug_notifier);
	register_reboot_notifier(&hotplug_reboot_notifier);
	
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

	return 0;
}

late_initcall(exynos4_pm_hotplug_init);

static struct platform_device exynos4_pm_hotplug_device = {
	.name = "exynos4-dynamic-cpu-hotplug",
	.id = -1,
};

static int standhotplug_cpufreq_policy_notifier_call(struct notifier_block *this,
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
			if(standhotplug_enabled)
			{
				DBG_PRINT("Stand-hotplug is disabled: governor=%s\n",
								policy->governor->name);
				mutex_lock(&hotplug_lock);
				standhotplug_enabled = false;
				mutex_unlock(&hotplug_lock);
			}
		} 
		else
		{
			if(!standhotplug_enabled)
			{
				DBG_PRINT("Stand-hotplug is enabled: governor=%s\n",
								policy->governor->name);
				mutex_lock(&hotplug_lock);
				standhotplug_enabled = true;
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
static struct notifier_block standhotplug_cpufreq_policy_notifier = {
	.notifier_call = standhotplug_cpufreq_policy_notifier_call,
};



static int __init exynos4_pm_hotplug_device_init(void)
{
	int ret;

#if defined(CONFIG_CPU_FREQ_DEFAULT_GOV_PEGASUSQ) || \
	defined(CONFIG_CPU_FREQ_DEFAULT_GOV_HOTPLUG) || \
	defined(CONFIG_CPU_FREQ_DEFAULT_GOV_LULZACTIVEQ)
	standhotplug_enabled = 0;
#else
	standhotplug_enabled = 1;
#endif
	ret = platform_device_register(&exynos4_pm_hotplug_device);

	if (ret) {
		printk(KERN_ERR "failed at(%d)\n", __LINE__);
		return ret;
	}

	printk(KERN_INFO "exynos4_pm_hotplug_device_init: %d\n", ret);

	cpufreq_register_notifier(&standhotplug_cpufreq_policy_notifier,
						CPUFREQ_POLICY_NOTIFIER);
	return ret;
}

late_initcall(exynos4_pm_hotplug_device_init);
