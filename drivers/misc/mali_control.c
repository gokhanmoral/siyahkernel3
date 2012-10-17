/* drivers/misc/mali_control.c
 *
 * based on Michael Wodkins's gpu_control implementation on S2
 * a little modified to have clock and threshold controls by gokhanmoral
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of the GNU General Public License as published by the
 *  Free Software Foundation;
 */

#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/kallsyms.h>

#define GPU_MAX_CLOCK 700
#define GPU_MIN_CLOCK 10

#define MIN_VOLTAGE_GPU  600000
#define MAX_VOLTAGE_GPU 1400000

typedef struct mali_dvfs_tableTag{
    unsigned int clock;
    unsigned int freq;
    unsigned int vol;
}mali_dvfs_table;

typedef struct mali_dvfs_thresholdTag{
	unsigned int downthreshold;
	unsigned int upthreshold;
} mali_dvfs_threshold_table;

typedef struct mali_runtime_resumeTag{
	int clk;
	int vol;
} mali_runtime_resume_table;

typedef struct mali_dvfs_staycount{
	unsigned int staycount;
}mali_dvfs_staycount_table;

mali_runtime_resume_table *gm_mali_runtime_resume;
mali_dvfs_table *gm_mali_dvfs;
mali_dvfs_table *gm_mali_dvfs_all;
mali_dvfs_threshold_table *gm_mali_dvfs_threshold;
mali_dvfs_staycount_table *gm_mali_dvfs_staycount;
int *gm_mali_dvfs_control;


static ssize_t gpu_clock_show(struct device *dev, struct device_attribute *attr, char *buf) {
	return sprintf(buf, "Step0: %d\nStep1: %d\nStep2: %d\nStep3: %d\nStep4: %d\n",
		gm_mali_dvfs[0].clock, gm_mali_dvfs[1].clock, gm_mali_dvfs[2].clock, gm_mali_dvfs[3].clock, gm_mali_dvfs[4].clock
		);
}


static ssize_t gpu_clock_store(struct device *dev, struct device_attribute *attr, const char *buf,
									size_t count) {
	unsigned int ret = -EINVAL;
	int i = 0;
	unsigned int g[8];

	if ( (ret=sscanf(buf, "%d %d %d %d %d", &g[0], &g[1], &g[2], &g[3], &g[4]))!=5 )
			return -EINVAL;
	/* safety floor and ceiling - netarchy */
	for( i = 0; i < 5; i++ ) {
		if (g[i] < GPU_MIN_CLOCK) {
			g[i] = GPU_MIN_CLOCK;
		}
		else if (g[i] > GPU_MAX_CLOCK) {
			g[i] = GPU_MAX_CLOCK;
		}
		gm_mali_dvfs_all[i].clock=g[i];
	}
	*gm_mali_dvfs_control = 999;
	return count;	
}

static ssize_t gpu_staycount_show(struct device *dev, struct device_attribute *attr, char *buf) {
	return sprintf(buf, "%d %d %d %d %d\n", 
	gm_mali_dvfs_staycount[0].staycount,
	gm_mali_dvfs_staycount[1].staycount,
	gm_mali_dvfs_staycount[2].staycount,
	gm_mali_dvfs_staycount[3].staycount,
	gm_mali_dvfs_staycount[4].staycount
	);
}

static ssize_t gpu_staycount_store(struct device *dev, struct device_attribute *attr, const char *buf,
									size_t count) {
	unsigned int ret = -EINVAL;
	int i1, i2, i3, i4, i5;

    if ( (ret=sscanf(buf, "%d %d %d %d %d", &i1, &i2, &i3, &i4, &i5))!=5 )
		return -EINVAL;
	gm_mali_dvfs_staycount[0].staycount = i1;
	gm_mali_dvfs_staycount[1].staycount = i2;
	gm_mali_dvfs_staycount[2].staycount = i3;
	gm_mali_dvfs_staycount[3].staycount = i4;
	gm_mali_dvfs_staycount[4].staycount = i5;
	return count;	
}

static ssize_t gpu_voltage_show(struct device *dev, struct device_attribute *attr, char *buf) {
	return sprintf(buf, "Step1: %d\nStep2: %d\nStep3: %d\nStep4: %d\nStep5: %d\n",
			gm_mali_dvfs[0].vol, gm_mali_dvfs[1].vol,gm_mali_dvfs[2].vol,gm_mali_dvfs[3].vol,gm_mali_dvfs[4].vol);
}

static ssize_t gpu_voltage_store(struct device *dev, struct device_attribute *attr, const char *buf,
									size_t count) {
	unsigned int ret = -EINVAL;
	int i = 0;
	unsigned int gv[5];

	ret = sscanf(buf, "%d %d %d %d %d", &gv[0], &gv[1], &gv[2], &gv[3], &gv[4]);
	if(ret!=5) 
	{
		return -EINVAL;
	}
	
    /* safety floor and ceiling - netarchy */
    for( i = 0; i < 5; i++ ) {
        if (gv[i] < MIN_VOLTAGE_GPU) {
            gv[i] = MIN_VOLTAGE_GPU;
        }
        else if (gv[i] > MAX_VOLTAGE_GPU) {
            gv[i] = MAX_VOLTAGE_GPU;
    	}
		gm_mali_dvfs[i].vol=gv[i];
    }
//	gm_mali_runtime_resume.vol = gm_mali_dvfs[1].vol;
	return count;	
}

static DEVICE_ATTR(voltage_control, S_IRUGO | S_IWUGO, gpu_voltage_show, gpu_voltage_store);
static DEVICE_ATTR(clock_control, S_IRUGO | S_IWUGO, gpu_clock_show, gpu_clock_store);
static DEVICE_ATTR(staycount_control, S_IRUGO | S_IWUGO, gpu_staycount_show, gpu_staycount_store);

static struct attribute *malicontrol_attributes[] = {
	&dev_attr_voltage_control.attr,
	&dev_attr_clock_control.attr,
	&dev_attr_staycount_control.attr,
	NULL
};

static struct attribute_group malicontrol_group = {
	.attrs = malicontrol_attributes,
};

static struct miscdevice malicontrol_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mali_control",
};

int register_mali_control(void)
{
    int ret;
    pr_info("%s misc_register(%s)\n", __FUNCTION__, malicontrol_device.name);
	gm_mali_dvfs = (mali_dvfs_table *)kallsyms_lookup_name("mali_dvfs");
	gm_mali_dvfs_all = (mali_dvfs_table *)kallsyms_lookup_name("mali_dvfs_all");
	gm_mali_dvfs_threshold = (mali_dvfs_threshold_table *)kallsyms_lookup_name("mali_dvfs_threshold");
	gm_mali_dvfs_staycount = (mali_dvfs_staycount_table *)kallsyms_lookup_name("mali_dvfs_staycount");
	gm_mali_dvfs_control = (int *)kallsyms_lookup_name("mali_dvfs_control");
    ret = misc_register(&malicontrol_device);
    if (ret) 
	{
	    pr_err("%s misc_register(%s) fail\n", __FUNCTION__, malicontrol_device.name);
	    return 1;
	}
    if (sysfs_create_group(&malicontrol_device.this_device->kobj, &malicontrol_group) < 0) 
	{
	    pr_err("%s sysfs_create_group fail\n", __FUNCTION__);
	    pr_err("Failed to create sysfs group for device (%s)!\n", malicontrol_device.name);
	}

    return 0;
}

static int __init malicontrol_init(void)
{
#ifdef MODULE
	return register_mali_control();
#else
	return 0;
#endif
}

static void __exit malicontrol_exit(void)
{
	sysfs_remove_group(&malicontrol_device.this_device->kobj, &malicontrol_group);
	misc_deregister(&malicontrol_device);
}

module_init( malicontrol_init );
module_exit( malicontrol_exit );

MODULE_AUTHOR("Gokhan Moral <gm@alumni.bilkent.edu.tr>");
MODULE_DESCRIPTION("Mali over/under clocking/volting module");
MODULE_LICENSE("GPL");
