/* linux/drivers/video/samsung/s3cfb_s6c1372.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * S6F1202A : 7" WSVGA Landscape LCD module driver
 * S6C1372 : 7.3" WXGA Landscape LCD module driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/lcd.h>
#include <plat/gpio-cfg.h>
#include "s3cfb.h"


struct s6c1372_lcd {
	struct device			*dev;
	struct lcd_device		*ld;
};

static int lvds_lcd_set_power(struct lcd_device *ld, int power)
{
	return 0;
}

static struct lcd_ops s6c1372_ops = {
	.set_power = lvds_lcd_set_power,
};

extern unsigned int lcdtype;

static ssize_t lcdtype_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char temp[15];
#if defined(CONFIG_FB_S5P_S6F1202A)
	/*For P2*/
	if (lcdtype == 0)
		sprintf(temp, "HYDIS_NT51008\n");
	else if (lcdtype == 2)
		sprintf(temp, "BOE_NT51008\n");
	else
		sprintf(temp, "SMD_S6F1202A02\n");
#else
	/*For p4*/
		sprintf(temp, "SMD_S6C1372\n");
#endif
	strcat(buf, temp);
	return strlen(buf);
}
static DEVICE_ATTR(lcd_type, 0664, lcdtype_show, NULL);

static int __init s6c1372_probe(struct platform_device *pdev)
{
	struct s6c1372_lcd *lcd;
	int ret = 0;

	lcd = kzalloc(sizeof(struct s6c1372_lcd), GFP_KERNEL);
	if (!lcd) {
		pr_err("failed to allocate for lcd\n");
		ret = -ENOMEM;
		goto err_alloc;
	}

	lcd->ld = lcd_device_register("panel", &pdev->dev, lcd, &s6c1372_ops);

	if (IS_ERR(lcd->ld)) {
		pr_err("failed to register lcd device\n");
		ret = PTR_ERR(lcd->ld);
		goto out_free_lcd;
	}
	ret = device_create_file(&lcd->ld->dev, &dev_attr_lcd_type);
	if (ret < 0)
		dev_err(&lcd->ld->dev, "failed to add sysfs entries\n");

	dev_set_drvdata(&pdev->dev, lcd);

	dev_info(&lcd->ld->dev, "lcd panel driver has been probed.\n");

	return  0;

out_free_lcd:
	kfree(lcd);
	return ret;
err_alloc:
	return ret;
}

static int __devexit s6c1372_remove(struct platform_device *pdev)
{
	struct s6c1372_lcd *lcd = dev_get_drvdata(&pdev->dev);

	lcd_device_unregister(lcd->ld);
	kfree(lcd);

	return 0;
}

static struct platform_driver s6c1372_driver = {
	.driver = {
		.name	= "s6c1372",
		.owner	= THIS_MODULE,
	},
	.probe		= s6c1372_probe,
	.remove		= __exit_p(s6c1372_remove),
	.suspend	= NULL,
	.resume		= NULL,
};

static int __init s6c1372_init(void)
{
	return  platform_driver_register(&s6c1372_driver);
}

static void __exit s6c1372_exit(void)
{
	platform_driver_unregister(&s6c1372_driver);
}

module_init(s6c1372_init);
module_exit(s6c1372_exit);

MODULE_AUTHOR("SAMSUNG");
MODULE_DESCRIPTION("S6C1372 LCD driver");
MODULE_LICENSE("GPL");

