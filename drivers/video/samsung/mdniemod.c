/* drivers/misc/mdniemod.c
 *
 * mdnie interfaces by gokhanmoral
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/device.h>
#include <linux/kallsyms.h>
#include <linux/miscdevice.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif
#if defined(CONFIG_CPU_EXYNOS4210)
#if defined(CONFIG_FB_S5P_LD9040) || defined(CONFIG_FB_S5P_NT35560)
#include "mdnie_table_u1.h"
#endif
#else
#include "mdnie_table_c1m0.h"
#endif

static void force_update_mdnie(void)
{
	struct mdnie_info *gm_g_mdnie;
	void (*gm_set_mdnie_value)(struct mdnie_info *mdnie, u8 force);
	gm_g_mdnie = *((void **)kallsyms_lookup_name("g_mdnie"));
	gm_set_mdnie_value = (void (*)(struct mdnie_info*, u8))
			kallsyms_lookup_name("set_mdnie_value");
	(*gm_set_mdnie_value)(gm_g_mdnie, 1);
}

#define MDNIE_STORE(name) \
static ssize_t show_##name(struct device *dev, \
		struct device_attribute *attr, \
		char *buf) \
{ \
	int i; \
	unsigned short *gm_##name = (unsigned short *)(kallsyms_lookup_name(#name)); \
	for (i = 2; i <= sizeof(name); i+=2) \
	{ \
		if(gm_##name[i] == END_SEQ) break; \
		sprintf(buf, "%s0x%X 0x%X\n", buf, gm_##name[i], gm_##name[i+1]); \
	} \
	return sprintf(buf, "%s", buf); \
} \
static ssize_t store_##name(struct device *dev, \
				  struct device_attribute *attr, \
				  const char *buf, size_t size) \
{ \
	short unsigned int reg = 0; \
	short unsigned int val = 0; \
	int unsigned bytes_read = 0; \
	int i; \
	unsigned short *gm_##name = (unsigned short *)(kallsyms_lookup_name(#name)); \
	while (sscanf(buf, "%hx %hx%n", &reg, &val, &bytes_read) == 2) { \
		buf += bytes_read; \
		for(i = 2; i<= sizeof(name); i+=2) { \
			if(gm_##name[i] == END_SEQ) break; \
			if(gm_##name[i] == reg) { \
				gm_##name[i+1] = val; \
				force_update_mdnie(); \
				break; \
			} \
		} \
	} \
	return size; \
} \
static DEVICE_ATTR(name, S_IRUGO | S_IWUGO, show_##name, store_##name);


MDNIE_STORE(tune_dynamic_gallery);
MDNIE_STORE(tune_dynamic_ui);
MDNIE_STORE(tune_dynamic_video);
MDNIE_STORE(tune_dynamic_vt);
MDNIE_STORE(tune_movie_gallery);
MDNIE_STORE(tune_movie_ui);
MDNIE_STORE(tune_movie_video);
MDNIE_STORE(tune_movie_vt);
MDNIE_STORE(tune_standard_gallery);
MDNIE_STORE(tune_standard_ui);
MDNIE_STORE(tune_standard_video);
MDNIE_STORE(tune_standard_vt);
MDNIE_STORE(tune_natural_gallery);
MDNIE_STORE(tune_natural_ui);
MDNIE_STORE(tune_natural_video);
MDNIE_STORE(tune_natural_vt);
MDNIE_STORE(tune_camera);
MDNIE_STORE(tune_camera_outdoor);
MDNIE_STORE(tune_cold);
MDNIE_STORE(tune_cold_outdoor);
MDNIE_STORE(tune_normal_outdoor);
MDNIE_STORE(tune_warm);
MDNIE_STORE(tune_warm_outdoor);

#define MDNIE_ATTR(name) &dev_attr_##name.attr,

static struct attribute *mdnie_attributes[] = {
MDNIE_ATTR(tune_dynamic_gallery)
MDNIE_ATTR(tune_dynamic_ui)
MDNIE_ATTR(tune_dynamic_video)
MDNIE_ATTR(tune_dynamic_vt)
MDNIE_ATTR(tune_movie_gallery)
MDNIE_ATTR(tune_movie_ui)
MDNIE_ATTR(tune_movie_video)
MDNIE_ATTR(tune_movie_vt)
MDNIE_ATTR(tune_standard_gallery)
MDNIE_ATTR(tune_standard_ui)
MDNIE_ATTR(tune_standard_video)
MDNIE_ATTR(tune_standard_vt)
MDNIE_ATTR(tune_natural_gallery)
MDNIE_ATTR(tune_natural_ui)
MDNIE_ATTR(tune_natural_video)
MDNIE_ATTR(tune_natural_vt)
MDNIE_ATTR(tune_camera)
MDNIE_ATTR(tune_camera_outdoor)
MDNIE_ATTR(tune_cold)
MDNIE_ATTR(tune_cold_outdoor)
MDNIE_ATTR(tune_normal_outdoor)
MDNIE_ATTR(tune_warm)
MDNIE_ATTR(tune_warm_outdoor)
	NULL
};

static struct attribute_group mdnie_group = {
	.attrs = mdnie_attributes,
};

static struct miscdevice mdnie_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mdnie",
};


static int __init mdniemod_init(void)
{
	int ret;
    ret = misc_register(&mdnie_device);
    if (ret) 
	{
	    pr_err("%s misc_register(%s) fail\n", __FUNCTION__, mdnie_device.name);
	    return 1;
	}
    if (sysfs_create_group(&mdnie_device.this_device->kobj, &mdnie_group) < 0) 
	{
	    pr_err("%s sysfs_create_group fail\n", __FUNCTION__);
	    pr_err("Failed to create sysfs group for device (%s)!\n", mdnie_device.name);
	}
	return 0;
}

static void __exit mdniemod_exit(void)
{
	sysfs_remove_group(&mdnie_device.this_device->kobj, &mdnie_group);
	misc_deregister(&mdnie_device);	
	return;
}

module_init( mdniemod_init );
module_exit( mdniemod_exit );

MODULE_AUTHOR("Gokhan Moral <gm@alumni.bilkent.edu.tr>");
MODULE_DESCRIPTION("mdnie sharpness tweaks by hardcore");
MODULE_LICENSE("GPL");
