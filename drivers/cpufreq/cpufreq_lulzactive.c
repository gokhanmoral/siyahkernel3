/*
 * drivers/cpufreq/cpufreq_lulzactive.c
 *
 * Copyright (C) 2010 Google, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author: Mike Chan (mike@android.com)
 * Edited: Tegrak (luciferanna@gmail.com)
 *
 * Driver values in /sys/devices/system/cpu/cpufreq/lulzactive
 * 
 */

#include <linux/cpu.h>
#include <linux/cpumask.h>
#include <linux/cpufreq.h>
#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/tick.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/earlysuspend.h>
#include <asm/cputime.h>
#include <linux/suspend.h>

#define LULZACTIVE_VERSION	(2)
#define LULZACTIVE_AUTHOR	"tegrak"

// if you changed some codes for optimization, just write your name here.
#define LULZACTIVE_TUNER "siyah"

static atomic_t active_count = ATOMIC_INIT(0);

struct cpufreq_lulzactive_cpuinfo {
	struct timer_list cpu_timer;
	int timer_idlecancel;
	u64 time_in_idle;
	u64 idle_exit_time;
	u64 timer_run_time;
	int idling;
	u64 freq_change_time;
	u64 freq_change_time_in_idle;
	struct cpufreq_policy *policy;
	struct cpufreq_frequency_table *freq_table;
	struct cpufreq_frequency_table lulzfreq_table[32];
	unsigned int lulzfreq_table_size;
	unsigned int target_freq;
	int governor_enabled;
};

static DEFINE_PER_CPU(struct cpufreq_lulzactive_cpuinfo, cpuinfo);

/* Workqueues handle frequency scaling */
static struct task_struct *up_task;
static struct workqueue_struct *down_wq;
static struct work_struct freq_scale_down_work;
static cpumask_t up_cpumask;
static spinlock_t up_cpumask_lock;
static cpumask_t down_cpumask;
static spinlock_t down_cpumask_lock;
static struct mutex set_speed_lock;

/*
 * The minimum amount of time to spend at a frequency before we can step up.
 */
#define DEFAULT_UP_SAMPLE_TIME 20 * USEC_PER_MSEC
static unsigned long up_sample_time;

/*
 * The minimum amount of time to spend at a frequency before we can step down.
 */
#define DEFAULT_DOWN_SAMPLE_TIME 40 * USEC_PER_MSEC
static unsigned long down_sample_time;

/*
 * CPU freq will be increased if measured load > inc_cpu_load;
 */
#define DEFAULT_INC_CPU_LOAD 60
static unsigned long inc_cpu_load;

/*
 * CPU freq will be decreased if measured load < dec_cpu_load;
 * not implemented yet.
 */
#define DEFAULT_DEC_CPU_LOAD 30
static unsigned long dec_cpu_load;

/*
 * Increasing frequency table index
 * zero disables and causes to always jump straight to max frequency.
 */
#define DEFAULT_PUMP_UP_STEP 0
static unsigned long pump_up_step;

/*
 * Decreasing frequency table index
 * zero disables and will calculate frequency according to load heuristic.
 */
#define DEFAULT_PUMP_DOWN_STEP 0
static unsigned long pump_down_step;

/*
 * Use minimum frequency while suspended.
 */
static unsigned int early_suspended;

#define SCREEN_OFF_LOWEST_STEP 		(0xffffffff)
#define DEFAULT_SCREEN_OFF_MIN_STEP	(SCREEN_OFF_LOWEST_STEP)
static unsigned long screen_off_min_step;

static int cpufreq_governor_lulzactive(struct cpufreq_policy *policy,
		unsigned int event);

#ifndef CONFIG_CPU_FREQ_DEFAULT_GOV_LULZACTIVE
static
#endif
struct cpufreq_governor cpufreq_gov_lulzactive = {
	.name = "lulzactive",
	.governor = cpufreq_governor_lulzactive,
	.max_transition_latency = 10000000,
	.owner = THIS_MODULE,
};

static unsigned int get_lulzfreq_table_size(struct cpufreq_lulzactive_cpuinfo *pcpu) {
	unsigned int size = 0, i;
	for (i = 0; (pcpu->freq_table[i].frequency != CPUFREQ_TABLE_END); i++) {
		unsigned int freq = pcpu->freq_table[i].frequency;
		if (freq == CPUFREQ_ENTRY_INVALID) continue;
		pcpu->lulzfreq_table[size].index = i; //in case we need it later -gm
		pcpu->lulzfreq_table[size].frequency = freq;
		size++;
	}
	pcpu->lulzfreq_table[size].index = 0;
	pcpu->lulzfreq_table[size].frequency = CPUFREQ_TABLE_END;
	return size;
}

static inline void fix_screen_off_min_step(struct cpufreq_lulzactive_cpuinfo *pcpu) {
	if (pcpu->lulzfreq_table_size <= 0) {
		screen_off_min_step = 0;
		return;
	}
	
	if (DEFAULT_SCREEN_OFF_MIN_STEP == screen_off_min_step) 
		for(screen_off_min_step=0;
		pcpu->lulzfreq_table[screen_off_min_step].frequency != 500000;
		screen_off_min_step++);
	
	if (screen_off_min_step >= pcpu->lulzfreq_table_size)
		for(screen_off_min_step=0;
		pcpu->lulzfreq_table[screen_off_min_step].frequency != 500000;
		screen_off_min_step++);
}

static inline unsigned int adjust_screen_off_freq(
	struct cpufreq_lulzactive_cpuinfo *pcpu, unsigned int freq) {
	
	if (early_suspended && freq > pcpu->lulzfreq_table[screen_off_min_step].frequency) {		
		freq = pcpu->lulzfreq_table[screen_off_min_step].frequency;
		pcpu->target_freq = pcpu->policy->cur;
		
		if (freq > pcpu->policy->max)
			freq = pcpu->policy->max;
		if (freq < pcpu->policy->min)
			freq = pcpu->policy->min;
	}
	
	return freq;
}

static void cpufreq_lulzactive_timer(unsigned long data)
{
	unsigned int delta_idle;
	unsigned int delta_time;
	int cpu_load;
	int load_since_change;
	u64 time_in_idle;
	u64 idle_exit_time;
	struct cpufreq_lulzactive_cpuinfo *pcpu =
		&per_cpu(cpuinfo, data);
	u64 now_idle;
	unsigned int new_freq;
	unsigned int index;
	unsigned long flags;
	int ret;

	smp_rmb();

	if (!pcpu->governor_enabled)
		goto exit;

	/*
	 * Once pcpu->timer_run_time is updated to >= pcpu->idle_exit_time,
	 * this lets idle exit know the current idle time sample has
	 * been processed, and idle exit can generate a new sample and
	 * re-arm the timer.  This prevents a concurrent idle
	 * exit on that CPU from writing a new set of info at the same time
	 * the timer function runs (the timer function can't use that info
	 * until more time passes).
	 */
	time_in_idle = pcpu->time_in_idle;
	idle_exit_time = pcpu->idle_exit_time;
	now_idle = get_cpu_idle_time_us(data, &pcpu->timer_run_time);
	smp_wmb();

	/* If we raced with cancelling a timer, skip. */
	if (!idle_exit_time)
		goto exit;

	delta_idle = (unsigned int) cputime64_sub(now_idle, time_in_idle);
	delta_time = (unsigned int) cputime64_sub(pcpu->timer_run_time,
						  idle_exit_time);

	/*
	 * If timer ran less than 1ms after short-term sample started, retry.
	 */
	if (delta_time < 1000)
		goto rearm;

	if (delta_idle > delta_time)
		cpu_load = 0;
	else
		cpu_load = 100 * (delta_time - delta_idle) / delta_time;

	delta_idle = (unsigned int) cputime64_sub(now_idle,
						 pcpu->freq_change_time_in_idle);
	delta_time = (unsigned int) cputime64_sub(pcpu->timer_run_time,
						  pcpu->freq_change_time);

	if ((delta_time == 0) || (delta_idle > delta_time))
		load_since_change = 0;
	else
		load_since_change =
			100 * (delta_time - delta_idle) / delta_time;

	/*
	 * Choose greater of short-term load (since last idle timer
	 * started or timer function re-armed itself) or long-term load
	 * (since last frequency change).
	 */
	if (load_since_change > cpu_load)
		cpu_load = load_since_change;
	
	/*
	 * START lulzactive algorithm section
	 */
	if (cpu_load >= inc_cpu_load) {
		if (pump_up_step && pcpu->policy->cur < pcpu->policy->max) {
			ret = cpufreq_frequency_table_target(
				pcpu->policy, pcpu->lulzfreq_table,
				pcpu->policy->cur, CPUFREQ_RELATION_H,
				&index);
			if (ret < 0) {
				goto rearm;
			}
			
			// apply pump_up_step by tegrak
			index -= pump_up_step;
			if (index < 0)
				index = 0;
			
			new_freq = pcpu->lulzfreq_table[index].frequency;
		}
		else {
			new_freq = pcpu->policy->max;
		}
	}
	else {		
		if (pump_down_step) {
			ret = cpufreq_frequency_table_target(
				pcpu->policy, pcpu->lulzfreq_table,
				pcpu->policy->cur, CPUFREQ_RELATION_H,
				&index);
			if (ret < 0) {
				goto rearm;
			}
			
			// apply pump_down_step by tegrak
			index += pump_down_step;
			if (index >= pcpu->lulzfreq_table_size) {
				index = pcpu->lulzfreq_table_size - 1;
			}
			
			new_freq = (pcpu->policy->cur > pcpu->policy->min) ? 
				(pcpu->lulzfreq_table[index].frequency) :
				(pcpu->policy->min);
		}
		else {
			new_freq = pcpu->policy->max * cpu_load / 100;
			ret = cpufreq_frequency_table_target(
				pcpu->policy, pcpu->lulzfreq_table,
				new_freq, CPUFREQ_RELATION_H,
				&index);
			if (ret < 0) {
				goto rearm;
			}
			new_freq = pcpu->lulzfreq_table[index].frequency;
		}		
	}
	
	// adjust freq when screen off
	new_freq = adjust_screen_off_freq(pcpu, new_freq);
	
	if (pcpu->target_freq == new_freq)
		goto rearm_if_notmax;

	/*
	 * Do not scale down unless we have been at this frequency for the
	 * minimum sample time.
	 */
	if (new_freq < pcpu->target_freq) {
		if (cputime64_sub(pcpu->timer_run_time, pcpu->freq_change_time)
		    < down_sample_time)
			goto rearm;
	}
	else {
		if (cputime64_sub(pcpu->timer_run_time, pcpu->freq_change_time) <
		    up_sample_time) {
			/* don't reset timer */
			goto rearm;
		}
	}

	if (new_freq < pcpu->target_freq) {
		pcpu->target_freq = new_freq;
		spin_lock_irqsave(&down_cpumask_lock, flags);
		cpumask_set_cpu(data, &down_cpumask);
		spin_unlock_irqrestore(&down_cpumask_lock, flags);
		queue_work(down_wq, &freq_scale_down_work);
	} else {
		pcpu->target_freq = new_freq;
		spin_lock_irqsave(&up_cpumask_lock, flags);
		cpumask_set_cpu(data, &up_cpumask);
		spin_unlock_irqrestore(&up_cpumask_lock, flags);
		wake_up_process(up_task);
	}

rearm_if_notmax:
	/*
	 * Already set max speed and don't see a need to change that,
	 * wait until next idle to re-evaluate, don't need timer.
	 */
	if (pcpu->target_freq == pcpu->policy->max)
		goto exit;

rearm:
	if (!timer_pending(&pcpu->cpu_timer)) {
		/*
		 * If already at min: if that CPU is idle, don't set timer.
		 * Else cancel the timer if that CPU goes idle.  We don't
		 * need to re-evaluate speed until the next idle exit.
		 */
		if (pcpu->target_freq == pcpu->policy->min) {
			smp_rmb();

			if (pcpu->idling)
				goto exit;

			pcpu->timer_idlecancel = 1;
		}

		pcpu->time_in_idle = get_cpu_idle_time_us(
			data, &pcpu->idle_exit_time);
		mod_timer(&pcpu->cpu_timer,
			  jiffies + 4);
	}

exit:
	return;
}

static void cpufreq_lulzactive_idle_start(void)
{
	struct cpufreq_lulzactive_cpuinfo *pcpu =
		&per_cpu(cpuinfo, smp_processor_id());
	int pending;

	if (!pcpu->governor_enabled)
		return;

	pcpu->idling = 1;
	smp_wmb();
	pending = timer_pending(&pcpu->cpu_timer);

	if (pcpu->target_freq != pcpu->policy->min) {
#ifdef CONFIG_SMP
		/*
		 * Entering idle while not at lowest speed.  On some
		 * platforms this can hold the other CPU(s) at that speed
		 * even though the CPU is idle. Set a timer to re-evaluate
		 * speed so this idle CPU doesn't hold the other CPUs above
		 * min indefinitely.  This should probably be a quirk of
		 * the CPUFreq driver.
		 */
		if (!pending) {
			pcpu->time_in_idle = get_cpu_idle_time_us(
				smp_processor_id(), &pcpu->idle_exit_time);
			pcpu->timer_idlecancel = 0;
			mod_timer(&pcpu->cpu_timer,
				  jiffies + 4);
		}
#endif
	} else {
		/*
		 * If at min speed and entering idle after load has
		 * already been evaluated, and a timer has been set just in
		 * case the CPU suddenly goes busy, cancel that timer.  The
		 * CPU didn't go busy; we'll recheck things upon idle exit.
		 */
		if (pending && pcpu->timer_idlecancel) {
			del_timer(&pcpu->cpu_timer);
			/*
			 * Ensure last timer run time is after current idle
			 * sample start time, so next idle exit will always
			 * start a new idle sampling period.
			 */
			pcpu->idle_exit_time = 0;
			pcpu->timer_idlecancel = 0;
		}
	}

}

static void cpufreq_lulzactive_idle_end(void)
{
	struct cpufreq_lulzactive_cpuinfo *pcpu =
		&per_cpu(cpuinfo, smp_processor_id());

	pcpu->idling = 0;
	smp_wmb();

	/*
	 * Arm the timer for 1-2 ticks later if not already, and if the timer
	 * function has already processed the previous load sampling
	 * interval.  (If the timer is not pending but has not processed
	 * the previous interval, it is probably racing with us on another
	 * CPU.  Let it compute load based on the previous sample and then
	 * re-arm the timer for another interval when it's done, rather
	 * than updating the interval start time to be "now", which doesn't
	 * give the timer function enough time to make a decision on this
	 * run.)
	 */
	if (timer_pending(&pcpu->cpu_timer) == 0 &&
	    pcpu->timer_run_time >= pcpu->idle_exit_time &&
	    pcpu->governor_enabled) {
		pcpu->time_in_idle =
			get_cpu_idle_time_us(smp_processor_id(),
					     &pcpu->idle_exit_time);
		pcpu->timer_idlecancel = 0;
		mod_timer(&pcpu->cpu_timer,
			  jiffies + 4);
	}

}

static int cpufreq_lulzactive_up_task(void *data)
{
	unsigned int cpu;
	cpumask_t tmp_mask;
	unsigned long flags;
	struct cpufreq_lulzactive_cpuinfo *pcpu;


	while (1) {
		set_current_state(TASK_INTERRUPTIBLE);
		spin_lock_irqsave(&up_cpumask_lock, flags);

		if (cpumask_empty(&up_cpumask)) {
			spin_unlock_irqrestore(&up_cpumask_lock, flags);
			schedule();

			if (kthread_should_stop())
				break;

			spin_lock_irqsave(&up_cpumask_lock, flags);
		}

		set_current_state(TASK_RUNNING);
		tmp_mask = up_cpumask;
		cpumask_clear(&up_cpumask);
		spin_unlock_irqrestore(&up_cpumask_lock, flags);

		for_each_cpu(cpu, &tmp_mask) {
			unsigned int j;
			unsigned int max_freq = 0;

			pcpu = &per_cpu(cpuinfo, cpu);
			smp_rmb();

			if (!pcpu->governor_enabled)
				continue;

			mutex_lock(&set_speed_lock);

			for_each_cpu(j, pcpu->policy->cpus) {
				struct cpufreq_lulzactive_cpuinfo *pjcpu =
					&per_cpu(cpuinfo, j);

				if (pjcpu->target_freq > max_freq)
					max_freq = pjcpu->target_freq;
			}

			if (max_freq != pcpu->policy->cur)
				__cpufreq_driver_target(pcpu->policy,
							max_freq,
							CPUFREQ_RELATION_H);
			mutex_unlock(&set_speed_lock);
		}
	}

	return 0;
}

static void cpufreq_lulzactive_freq_down(struct work_struct *work)
{
	unsigned int cpu;
	cpumask_t tmp_mask;
	unsigned long flags;
	struct cpufreq_lulzactive_cpuinfo *pcpu;

	spin_lock_irqsave(&down_cpumask_lock, flags);
	tmp_mask = down_cpumask;
	cpumask_clear(&down_cpumask);
	spin_unlock_irqrestore(&down_cpumask_lock, flags);

	for_each_cpu(cpu, &tmp_mask) {
		unsigned int j;
		unsigned int max_freq = 0;

		pcpu = &per_cpu(cpuinfo, cpu);
		smp_rmb();

		if (!pcpu->governor_enabled)
			continue;

		mutex_lock(&set_speed_lock);

		for_each_cpu(j, pcpu->policy->cpus) {
			struct cpufreq_lulzactive_cpuinfo *pjcpu =
				&per_cpu(cpuinfo, j);

			if (pjcpu->target_freq > max_freq)
				max_freq = pjcpu->target_freq;
		}

		if (max_freq != pcpu->policy->cur)
			__cpufreq_driver_target(pcpu->policy, max_freq,
						CPUFREQ_RELATION_H);

		mutex_unlock(&set_speed_lock);
	}
}

// inc_cpu_load
static ssize_t show_inc_cpu_load(struct kobject *kobj,
				     struct attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", inc_cpu_load);
}

static ssize_t store_inc_cpu_load(struct kobject *kobj,
			struct attribute *attr, const char *buf, size_t count)
{
	if(strict_strtoul(buf, 0, &inc_cpu_load)==-EINVAL) return -EINVAL;
	
	if (inc_cpu_load > 100) {
		inc_cpu_load = 100;
	}
	else if (inc_cpu_load < 10) {
		inc_cpu_load = 10;
	}
	return count;
}

static struct global_attr inc_cpu_load_attr = __ATTR(inc_cpu_load, 0666,
		show_inc_cpu_load, store_inc_cpu_load);

// down_sample_time
static ssize_t show_down_sample_time(struct kobject *kobj,
				struct attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", down_sample_time);
}

static ssize_t store_down_sample_time(struct kobject *kobj,
			struct attribute *attr, const char *buf, size_t count)
{
	if(strict_strtoul(buf, 0, &down_sample_time)==-EINVAL) return -EINVAL;
	return count;
}

static struct global_attr down_sample_time_attr = __ATTR(down_sample_time, 0666,
		show_down_sample_time, store_down_sample_time);

// up_sample_time
static ssize_t show_up_sample_time(struct kobject *kobj,
				struct attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", up_sample_time);
}

static ssize_t store_up_sample_time(struct kobject *kobj,
			struct attribute *attr, const char *buf, size_t count)
{
	if(strict_strtoul(buf, 0, &up_sample_time)==-EINVAL) return -EINVAL;
	return count;
}

static struct global_attr up_sample_time_attr = __ATTR(up_sample_time, 0666,
		show_up_sample_time, store_up_sample_time);

// debug_mode
static ssize_t show_debug_mode(struct kobject *kobj,
				     struct attribute *attr, char *buf)
{
	return sprintf(buf, "0\n");
}

static ssize_t store_debug_mode(struct kobject *kobj,
			struct attribute *attr, const char *buf, size_t count)
{
	return count;
}

static struct global_attr debug_mode_attr = __ATTR(debug_mode, 0666,
		show_debug_mode, store_debug_mode);

// pump_up_step
static ssize_t show_pump_up_step(struct kobject *kobj,
				     struct attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", pump_up_step);
}

static ssize_t store_pump_up_step(struct kobject *kobj,
			struct attribute *attr, const char *buf, size_t count)
{
	if(strict_strtoul(buf, 0, &pump_up_step)==-EINVAL) return -EINVAL;
	return count;
}

static struct global_attr pump_up_step_attr = __ATTR(pump_up_step, 0666,
		show_pump_up_step, store_pump_up_step);

// pump_down_step
static ssize_t show_pump_down_step(struct kobject *kobj,
				     struct attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", pump_down_step);
}

static ssize_t store_pump_down_step(struct kobject *kobj,
			struct attribute *attr, const char *buf, size_t count)
{
	struct cpufreq_lulzactive_cpuinfo *pcpu;
	
	if(strict_strtoul(buf, 0, &pump_down_step)==-EINVAL) return -EINVAL;
	
	pcpu = &per_cpu(cpuinfo, 0);
	// fix out of bound
	if (pcpu->lulzfreq_table_size <= pump_down_step) {
		pump_down_step = pcpu->lulzfreq_table_size - 1;
	}
	return count;
}

static struct global_attr pump_down_step_attr = __ATTR(pump_down_step, 0666,
		show_pump_down_step, store_pump_down_step);

// screen_off_min_step
static ssize_t show_screen_off_min_step(struct kobject *kobj,
				     struct attribute *attr, char *buf)
{
	struct cpufreq_lulzactive_cpuinfo *pcpu;
	
	pcpu = &per_cpu(cpuinfo, 0);
	fix_screen_off_min_step(pcpu);
	
	return sprintf(buf, "%lu\n", screen_off_min_step);
}

static ssize_t store_screen_off_min_step(struct kobject *kobj,
			struct attribute *attr, const char *buf, size_t count)
{
	struct cpufreq_lulzactive_cpuinfo *pcpu;
	
	if(strict_strtoul(buf, 0, &screen_off_min_step)==-EINVAL) return -EINVAL;
	
	pcpu = &per_cpu(cpuinfo, 0);
	fix_screen_off_min_step(pcpu);
	
	return count;
}

static struct global_attr screen_off_min_step_attr = __ATTR(screen_off_min_step, 0666,
		show_screen_off_min_step, store_screen_off_min_step);

// author
static ssize_t show_author(struct kobject *kobj,
				     struct attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", LULZACTIVE_AUTHOR);
}

static struct global_attr author_attr = __ATTR(author, 0444,
		show_author, NULL);

// tuner
static ssize_t show_tuner(struct kobject *kobj,
				     struct attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", LULZACTIVE_TUNER);
}

static struct global_attr tuner_attr = __ATTR(tuner, 0444,
		show_tuner, NULL);

// version
static ssize_t show_version(struct kobject *kobj,
				     struct attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", LULZACTIVE_VERSION);
}

static struct global_attr version_attr = __ATTR(version, 0444,
		show_version, NULL);

// freq_table
static ssize_t show_freq_table(struct kobject *kobj,
				     struct attribute *attr, char *buf)
{
	struct cpufreq_lulzactive_cpuinfo *pcpu;
	char temp[64];
	int i;
	
	pcpu = &per_cpu(cpuinfo, 0);
	
	for (i = 0; i < pcpu->lulzfreq_table_size; i++) {
		sprintf(temp, "%u\n", pcpu->lulzfreq_table[i].frequency);
		strcat(buf, temp);
	}
	
	return strlen(buf);
}

static struct global_attr freq_table_attr = __ATTR(freq_table, 0444,
		show_freq_table, NULL);

static struct attribute *lulzactive_attributes[] = {
	&inc_cpu_load_attr.attr,
	&up_sample_time_attr.attr,
	&down_sample_time_attr.attr,
	&pump_up_step_attr.attr,
	&pump_down_step_attr.attr,
	&screen_off_min_step_attr.attr,
	&debug_mode_attr.attr,
	&author_attr.attr,
	&tuner_attr.attr,
	&version_attr.attr,
	&freq_table_attr.attr,
	NULL,
};

void start_lulzactive(void);
void stop_lulzactive(void);
		
static struct attribute_group lulzactive_attr_group = {
	.attrs = lulzactive_attributes,
	.name = "lulzactive",
};

static int cpufreq_governor_lulzactive(struct cpufreq_policy *policy,
		unsigned int event)
{
	int rc;
	unsigned int j;
	struct cpufreq_lulzactive_cpuinfo *pcpu;
	struct cpufreq_frequency_table *freq_table;

	switch (event) {
	case CPUFREQ_GOV_START:
		if (!cpu_online(policy->cpu))
			return -EINVAL;

		freq_table =
			cpufreq_frequency_get_table(policy->cpu);

		for_each_cpu(j, policy->cpus) {
			pcpu = &per_cpu(cpuinfo, j);
			pcpu->policy = policy;
			pcpu->target_freq = policy->cur;
			pcpu->freq_table = freq_table;
			pcpu->freq_change_time_in_idle =
				get_cpu_idle_time_us(j,
						     &pcpu->freq_change_time);
			pcpu->governor_enabled = 1;
			smp_wmb();
			pcpu->lulzfreq_table_size = get_lulzfreq_table_size(pcpu);

			// fix invalid screen_off_min_step
			fix_screen_off_min_step(pcpu);
		}

		/*
		 * Do not register the idle hook and create sysfs
		 * entries if we have already done so.
		 */
		if (atomic_inc_return(&active_count) > 1)
			return 0;
		start_lulzactive();

		rc = sysfs_create_group(cpufreq_global_kobject,
				&lulzactive_attr_group);
		if (rc)
			return rc;

		break;

	case CPUFREQ_GOV_STOP:
		for_each_cpu(j, policy->cpus) {
			pcpu = &per_cpu(cpuinfo, j);
			pcpu->governor_enabled = 0;
			smp_wmb();
			del_timer_sync(&pcpu->cpu_timer);

			/*
			 * Reset idle exit time since we may cancel the timer
			 * before it can run after the last idle exit time,
			 * to avoid tripping the check in idle exit for a timer
			 * that is trying to run.
			 */
			pcpu->idle_exit_time = 0;
		}

		flush_work(&freq_scale_down_work);
		if (atomic_dec_return(&active_count) > 0)
			return 0;

		sysfs_remove_group(cpufreq_global_kobject,
				&lulzactive_attr_group);
		stop_lulzactive();
		break;

	case CPUFREQ_GOV_LIMITS:
		if (policy->max < policy->cur)
			__cpufreq_driver_target(policy,
					policy->max, CPUFREQ_RELATION_H);
		else if (policy->min > policy->cur)
			__cpufreq_driver_target(policy,
					policy->min, CPUFREQ_RELATION_L);
		break;
	}
	return 0;
}

static int cpufreq_lulzactive_idle_notifier(struct notifier_block *nb,
					     unsigned long val,
					     void *data)
{
	switch (val) {
	case IDLE_START:
		cpufreq_lulzactive_idle_start();
		break;
	case IDLE_END:
		cpufreq_lulzactive_idle_end();
		break;
	}

	return 0;
}

static struct notifier_block cpufreq_lulzactive_idle_nb = {
	.notifier_call = cpufreq_lulzactive_idle_notifier,
};

static void lulzactive_early_suspend(struct early_suspend *handler) {
	early_suspended = 1;
}

static void lulzactive_late_resume(struct early_suspend *handler) {
	early_suspended = 0;
}

static struct early_suspend lulzactive_power_suspend = {
	.suspend = lulzactive_early_suspend,
	.resume = lulzactive_late_resume,
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 1,
};

void start_lulzactive(void)
{
	//it is more appropriate to start the up_task thread after starting the governor -gm
	unsigned int i, index500, index800;
	struct cpufreq_lulzactive_cpuinfo *pcpu;
	struct sched_param param = { .sched_priority = MAX_RT_PRIO-1 };

	if( pump_up_step == 0 )
	{
		pcpu = &per_cpu(cpuinfo, 0);
		cpufreq_frequency_table_target(
				pcpu->policy, pcpu->lulzfreq_table,
				500000, CPUFREQ_RELATION_H,
				&index500);
		cpufreq_frequency_table_target(
				pcpu->policy, pcpu->lulzfreq_table,
				800000, CPUFREQ_RELATION_H,
				&index800);
		for(i=index800;i<index500;i++)
		{
		  if(pcpu->lulzfreq_table[i].frequency==CPUFREQ_ENTRY_INVALID) continue;
		  pump_up_step++;
		}
	}
	if( pump_down_step == 0 )
	{
		pump_down_step = pump_up_step;
	}	

	up_task = kthread_create(cpufreq_lulzactive_up_task, NULL,
				 "klulzactiveup");

	sched_setscheduler_nocheck(up_task, SCHED_FIFO, &param);
	get_task_struct(up_task);

	idle_notifier_register(&cpufreq_lulzactive_idle_nb);
	register_early_suspend(&lulzactive_power_suspend);
}

void stop_lulzactive(void)
{
	//cleanup the thread after stopping the governor -gm
	kthread_stop(up_task);
	put_task_struct(up_task);

	idle_notifier_unregister(&cpufreq_lulzactive_idle_nb);
	unregister_early_suspend(&lulzactive_power_suspend);
	pump_up_step = DEFAULT_PUMP_UP_STEP;
	pump_down_step = DEFAULT_PUMP_DOWN_STEP;
}

static int __init cpufreq_lulzactive_init(void)
{
	unsigned int i;
	struct cpufreq_lulzactive_cpuinfo *pcpu;
	up_sample_time = DEFAULT_UP_SAMPLE_TIME;
	down_sample_time = DEFAULT_DOWN_SAMPLE_TIME;
	inc_cpu_load = DEFAULT_INC_CPU_LOAD;
	dec_cpu_load = DEFAULT_DEC_CPU_LOAD;
	pump_up_step = DEFAULT_PUMP_UP_STEP;
	pump_down_step = DEFAULT_PUMP_DOWN_STEP;
	early_suspended = 0;
	screen_off_min_step = DEFAULT_SCREEN_OFF_MIN_STEP;


	/* Initalize per-cpu timers */
	for_each_possible_cpu(i) {
		pcpu = &per_cpu(cpuinfo, i);
		init_timer(&pcpu->cpu_timer);
		pcpu->cpu_timer.function = cpufreq_lulzactive_timer;
		pcpu->cpu_timer.data = i;
	}

	/* No rescuer thread, bind to CPU queuing the work for possibly
	   warm cache (probably doesn't matter much). */
	down_wq = alloc_workqueue("knteractive_down", 0, 1);

	if (!down_wq)
		goto err_freeuptask;

	INIT_WORK(&freq_scale_down_work,
		  cpufreq_lulzactive_freq_down);

	spin_lock_init(&up_cpumask_lock);
	spin_lock_init(&down_cpumask_lock);
	mutex_init(&set_speed_lock);

	return cpufreq_register_governor(&cpufreq_gov_lulzactive);

err_freeuptask:
	put_task_struct(up_task);
	return -ENOMEM;
}

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_LULZACTIVE
fs_initcall(cpufreq_lulzactive_init);
#else
module_init(cpufreq_lulzactive_init);
#endif

static void __exit cpufreq_lulzactive_exit(void)
{
	cpufreq_unregister_governor(&cpufreq_gov_lulzactive);
	kthread_stop(up_task);
	put_task_struct(up_task);
	destroy_workqueue(down_wq);
}

module_exit(cpufreq_lulzactive_exit);

MODULE_AUTHOR("Tegrak <luciferanna@gmail.com>");
MODULE_DESCRIPTION("'lulzactive' - improved interactive governor inspired by smartass");
MODULE_LICENSE("GPL");
