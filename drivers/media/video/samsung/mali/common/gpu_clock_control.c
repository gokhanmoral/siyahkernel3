/*
 * gpu_clock_control.c -- a clock control interface for the sgs2
 *
 *  Copyright (C) 2011 Michael Wodkins
 *  twitter - @xdanetarchy
 *  XDA-developers - netarchy
 *  modified by gokhanmoral
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of the GNU General Public License as published by the
 *  Free Software Foundation;
 *
 */

#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#include "gpu_clock_control.h"

#define GPU_MAX_CLOCK 450
#define GPU_MIN_CLOCK 10

typedef struct mali_dvfs_tableTag{
    unsigned int clock;
    unsigned int freq;
    unsigned int vol;
}mali_dvfs_table;
typedef struct mali_dvfs_thresholdTag{
	unsigned int downthreshold;
	unsigned int upthreshold;
}mali_dvfs_threshold_table;
extern mali_dvfs_table mali_dvfs[3];
extern mali_dvfs_threshold_table mali_dvfs_threshold[3];

typedef struct mali_dvfs_staycount{
	unsigned int staycount;
}mali_dvfs_staycount_table;

extern mali_dvfs_staycount_table mali_dvfs_staycount[3];
		
static ssize_t gpu_clock_show(struct device *dev, struct device_attribute *attr, char *buf) {
	return sprintf(buf, "Step0: %d\nStep1: %d\nStep2: %d\n"
						"Threshold0-1/up-down: %d%% %d%%\n"
						"Threshold1-2/up-down: %d%% %d%%\n",
		mali_dvfs[0].clock, mali_dvfs[1].clock, mali_dvfs[2].clock,
		mali_dvfs_threshold[0].upthreshold*100/255, 
		mali_dvfs_threshold[1].downthreshold*100/255,
		mali_dvfs_threshold[1].upthreshold*100/255,
		mali_dvfs_threshold[2].downthreshold*100/255
		);
}

unsigned int g[4];

static ssize_t gpu_clock_store(struct device *dev, struct device_attribute *attr, const char *buf,
									size_t count) {
	unsigned int ret = -EINVAL;
	int i = 0;

	if ( (ret=sscanf(buf, "%d%% %d%% %d%% %d%%", &g[0], &g[1], &g[2], &g[3])) == 4 ) i=1;
	else if ( (ret=sscanf(buf, "%d%% %d%%", &g[0], &g[1])) == 2 ) i=1;
	
	if(i) {
		if(g[1]<0 || g[0]>100) return -EINVAL;
		mali_dvfs_threshold[0].upthreshold = ((int)((255*g[0])/100));
		mali_dvfs_threshold[1].downthreshold = ((int)((255*g[1])/100));
		if(ret==4) {
			if(g[3]<0 || g[2]>100) return -EINVAL;
			mali_dvfs_threshold[1].upthreshold = ((int)((255*g[2])/100));
			mali_dvfs_threshold[2].downthreshold = ((int)((255*g[3])/100));
		}		
	} 
	else {
	  if ( (ret=sscanf(buf, "%d %d %d", &g[0], &g[1], &g[2]))!=3 )
	    if( (ret=sscanf(buf, "%d %d", &g[0], &g[1]))!=2 )
			return -EINVAL;
		/* safety floor and ceiling - netarchy */
		for( i = 0; i < 3; i++ ) {
			if (g[i] < GPU_MIN_CLOCK) {
				g[i] = GPU_MIN_CLOCK;
			}
			else if (g[i] > GPU_MAX_CLOCK) {
				g[i] = GPU_MAX_CLOCK;
			}
			if(ret==3 || i<2) mali_dvfs[i].clock=g[i];
		}
	}
	//if we set the second freq higher than the first by using voltage control app
	//...disable the third step
	if(mali_dvfs[2].clock < mali_dvfs[1].clock)
	{
		mali_dvfs_threshold[1].upthreshold = ((int)((255*120)/100));
	}
	return count;	
}

static ssize_t gpu_staycount_show(struct device *dev, struct device_attribute *attr, char *buf) {
	return sprintf(buf, "%d %d %d\n", 
	mali_dvfs_staycount[0].staycount,
	mali_dvfs_staycount[1].staycount,
	mali_dvfs_staycount[2].staycount
	);
}

unsigned int g[4];

static ssize_t gpu_staycount_store(struct device *dev, struct device_attribute *attr, const char *buf,
									size_t count) {
	unsigned int ret = -EINVAL;
	int i = 0, i1, i2, i3;

    if ( (ret=sscanf(buf, "%d %d %d", &i1, &i2, &i3))!=3 )
		return -EINVAL;
	mali_dvfs_staycount[0].staycount = i1;
	mali_dvfs_staycount[1].staycount = i2;
	mali_dvfs_staycount[2].staycount = i3;
	return count;	
}

static DEVICE_ATTR(gpu_control, S_IRUGO | S_IWUGO, gpu_clock_show, gpu_clock_store);
static DEVICE_ATTR(gpu_staycount, S_IRUGO | S_IWUGO, gpu_staycount_show, gpu_staycount_store);

static struct attribute *gpu_clock_control_attributes[] = {
	&dev_attr_gpu_control.attr,
	&dev_attr_gpu_staycount.attr,
	NULL
};

static struct attribute_group gpu_clock_control_group = {
	.attrs = gpu_clock_control_attributes,
};

static struct miscdevice gpu_clock_control_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "gpu_clock_control",
};

void gpu_clock_control_start()
{
	printk("Initializing gpu clock control interface\n");
	
	misc_register(&gpu_clock_control_device);
	if (sysfs_create_group(&gpu_clock_control_device.this_device->kobj,
				&gpu_clock_control_group) < 0) {
		printk("%s sysfs_create_group failed\n", __FUNCTION__);
		pr_err("Unable to create group for %s\n", gpu_clock_control_device.name);
	}
}
