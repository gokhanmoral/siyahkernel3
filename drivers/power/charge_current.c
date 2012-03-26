/*
 * charge_current.c -- charge current control interface for the sgs2
 *
 *  Copyright (C) 2011 Gokhan Moral
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of the GNU General Public License as published by the
 *  Free Software Foundation;
 *
 */

#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#define MAX_CURRENT_AC   950
#define MAX_CURRENT_MISC 950
#define MAX_CURRENT_USB  950
int charge_current_ac = 650;
int charge_current_misc = 550;
int charge_current_usb = 450;


static ssize_t charge_current_show(struct device *dev, struct device_attribute *attr, char *buf) {
	return sprintf(buf, "AC: %d\nMisc: %d\nUSB: %d\n",
		charge_current_ac, charge_current_misc, charge_current_usb);
}

static ssize_t charge_current_store(struct device *dev, struct device_attribute *attr, const char *buf,
									size_t count) {
	unsigned int ret = -EINVAL;
	int temp1, temp2, temp3;
	ret = sscanf(buf, "%d %d %d", &temp1, &temp2, &temp3);
	if (ret != 3) {
		return -EINVAL;
	}
	else {
		charge_current_ac = (temp1<MAX_CURRENT_AC) ? temp1 : MAX_CURRENT_AC;
		charge_current_misc = (temp2<MAX_CURRENT_MISC) ? temp2 : MAX_CURRENT_MISC;
		charge_current_usb = (temp3<MAX_CURRENT_USB) ? temp3 : MAX_CURRENT_USB;
	}
	return count;	
}

static DEVICE_ATTR(charge_current, S_IRUGO | S_IWUGO, charge_current_show, charge_current_store);

static struct attribute *charge_current_attributes[] = {
	&dev_attr_charge_current.attr,
	NULL
};

static struct attribute_group charge_current_group = {
	.attrs = charge_current_attributes,
};

static struct miscdevice charge_current_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "charge_current",
};

void charge_current_start(void)
{
	printk("Initializing charge current control interface\n");
	
	misc_register(&charge_current_device);
	if (sysfs_create_group(&charge_current_device.this_device->kobj,
				&charge_current_group) < 0) {
		printk("%s sysfs_create_group failed\n", __FUNCTION__);
		pr_err("Unable to create group for %s\n", charge_current_device.name);
	}
}

