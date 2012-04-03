/*
 * Samsung Exynos4 SoC series FIMC-IS slave interface driver
 *
 * main platform driver interface
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd
 * Contact: Younghwan Joo, <yhwan.joo@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/clk.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/memory.h>
#include <linux/regulator/consumer.h>
#include <linux/pm_runtime.h>

#include <linux/videodev2.h>
#include <linux/videodev2_samsung.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mem2mem.h>

#include <linux/cma.h>
#include <asm/cacheflush.h>
#include <asm/pgtable.h>
#include <linux/firmware.h>
#include <linux/dma-mapping.h>
#include <media/videobuf2-core.h>

#include "fimc-is-core.h"
#include "fimc-is-regs.h"
#include "fimc-is-param.h"
#include "fimc-is-cmd.h"
#include "fimc-is-err.h"

extern struct class *camera_class;
struct device *s5k6a3_dev; /*sys/class/camera/front*/

struct fimc_is_dev *to_fimc_is_dev(struct v4l2_subdev *sdev)
{
	return container_of(sdev, struct fimc_is_dev, sd);
}

static irqreturn_t fimc_is_irq_handler1(int irq, void *dev_id)
{
	struct fimc_is_dev *dev = dev_id;

	/* Read ISSR10 ~ ISSR15 */
	dev->i2h_cmd.cmd = readl(dev->regs + ISSR10);

	switch (dev->i2h_cmd.cmd) {
	case IHC_GET_SENSOR_NUMBER:
		dbg("IHC_GET_SENSOR_NUMBER\n");
		fimc_is_hw_get_param(dev, 1);
		dev->fw.ver = dev->i2h_cmd.arg[0];
		fimc_is_hw_wait_intmsr0_intmsd0(dev);
		fimc_is_hw_set_sensor_num(dev);
		break;
	case IHC_SET_SHOT_MARK:
		fimc_is_hw_get_param(dev, 3);
		break;
	case IHC_SET_FACE_MARK:
		fimc_is_hw_get_param(dev, 2);
		break;
	case IHC_FRAME_DONE:
		fimc_is_hw_get_param(dev, 2);
		break;
	case IHC_NOT_READY:
		break;
	case IHC_AA_DONE:
		fimc_is_hw_get_param(dev, 3);
		break;
	case ISR_DONE:
		fimc_is_hw_get_param(dev, 3);
		break;
	case ISR_NDONE:
		fimc_is_hw_get_param(dev, 4);
		break;
	}
	/* Just clear the interrupt pending bits. */
	fimc_is_fw_clear_irq1(dev);

	switch (dev->i2h_cmd.cmd) {
	case IHC_GET_SENSOR_NUMBER:
		fimc_is_hw_set_intgr0_gd0(dev);
		set_bit(IS_ST_FW_LOADED, &dev->state);
		break;
	case IHC_SET_SHOT_MARK:
		break;
	case IHC_SET_FACE_MARK:
		dev->fd_header.count = dev->i2h_cmd.arg[0];
		dev->fd_header.index = dev->i2h_cmd.arg[1];
		break;
	case IHC_FRAME_DONE:
		break;
	case IHC_AA_DONE:
		err("AA_DONE - %d, %d, %d\n", dev->i2h_cmd.arg[0],
			dev->i2h_cmd.arg[1], dev->i2h_cmd.arg[2]);
		switch (dev->i2h_cmd.arg[0]) {
		/* SEARCH: Occurs when search is requested at continuous AF */
		case 2:
			break;
		/* INFOCUS: Occurs when focus is found. */
		case 3:
			if (dev->af.af_state == FIMC_IS_AF_RUNNING)
				dev->af.af_state = FIMC_IS_AF_LOCK;
			dev->af.af_lock_state = 0x2;
			break;
		/* OUTOFFOCUS: Occurs when focus is not found. */
		case 4:
			if (dev->af.af_state == FIMC_IS_AF_RUNNING)
				dev->af.af_state = FIMC_IS_AF_LOCK;
			dev->af.af_lock_state = 0x1;
			break;
		}
		break;
	case IHC_NOT_READY:
		err("Init Sequnce Error- IS will be turned off!!");
		break;
	case ISR_DONE:
		dbg("ISR_DONE - %d\n", dev->i2h_cmd.arg[0]);
		switch (dev->i2h_cmd.arg[0]) {
		case HIC_PREVIEW_STILL:
		case HIC_PREVIEW_VIDEO:
		case HIC_CAPTURE_STILL:
		case HIC_CAPTURE_VIDEO:
			set_bit(IS_ST_CHANGE_MODE, &dev->state);
			/* Get CAC margin */
			dev->sensor.offset_x = dev->i2h_cmd.arg[1];
			dev->sensor.offset_y = dev->i2h_cmd.arg[2];
			break;
		case HIC_STREAM_ON:
			clear_bit(IS_ST_STREAM_OFF, &dev->state);
			set_bit(IS_ST_STREAM_ON, &dev->state);
			break;
		case HIC_STREAM_OFF:
			clear_bit(IS_ST_STREAM_ON, &dev->state);
			set_bit(IS_ST_STREAM_OFF, &dev->state);
			break;
		case HIC_SET_PARAMETER:
			dev->p_region_index1 = 0;
			dev->p_region_index2 = 0;
			atomic_set(&dev->p_region_num, 0);
			set_bit(IS_ST_BLOCK_CMD_CLEARED, &dev->state);

			if (dev->af.af_state == FIMC_IS_AF_SETCONFIG)
				dev->af.af_state = FIMC_IS_AF_RUNNING;
			else if (dev->af.af_state == FIMC_IS_AF_ABORT)
				dev->af.af_state = FIMC_IS_AF_IDLE;
			break;
		case HIC_GET_PARAMETER:
			break;
		case HIC_SET_TUNE:
			break;
		case HIC_GET_STATUS:
			break;
		case HIC_OPEN_SENSOR:
			set_bit(IS_ST_OPEN_SENSOR, &dev->state);
			break;
		case HIC_CLOSE_SENSOR:
			clear_bit(IS_ST_OPEN_SENSOR, &dev->state);
			dev->sensor.id = 0;
			break;
		case HIC_MSG_TEST:
			dbg("Config MSG level was done\n");
			break;
		case HIC_POWER_DOWN:
			set_bit(IS_PWR_SUB_IP_POWER_OFF, &dev->power);
			break;
		case HIC_GET_SET_FILE_ADDR:
			dev->setfile.base = dev->i2h_cmd.arg[1];
			set_bit(IS_ST_SETFILE_LOADED, &dev->state);
			break;
		case HIC_LOAD_SET_FILE:
			set_bit(IS_ST_SETFILE_LOADED, &dev->state);
			break;
		}
		break;
	case ISR_NDONE:
		err("ISR_NDONE - %d: 0x%08x\n", dev->i2h_cmd.arg[0],
			dev->i2h_cmd.arg[1]);
		fimc_is_print_err_number(dev->i2h_cmd.arg[1]);
		switch (dev->i2h_cmd.arg[1]) {
		case IS_ERROR_SET_PARAMETER:
			fimc_is_mem_cache_inv((void *)dev->is_p_region,
				IS_PARAM_SIZE);
			fimc_is_param_err_checker(dev);
			break;
		}
		break;
	}
	wake_up(&dev->irq_queue1);
	return IRQ_HANDLED;
}

static ssize_t s5k6a3_camera_front_camtype_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char type[] = "SLSI_S5K6A3_FIMC_IS";

	return sprintf(buf, "%s\n", type);
}

static ssize_t s5k6a3_camera_front_camfw_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char fw_sd[7];
	char fw_ori[7];
	struct file *fp_sd;
	struct file *fp_ori;

	mm_segment_t old_fs;
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp_ori = filp_open("/vendor/firmware/fimc_is_fw.bin", O_RDONLY, 0);

	if (IS_ERR(fp_ori))
		return sprintf(buf, "%s\n", "Error!!!");

	vfs_llseek(fp_ori, -7, SEEK_END);
	vfs_read(fp_ori, (char __user *)fw_ori, 7, &fp_ori->f_pos);
	fw_ori[6] = '\0';
	filp_close(fp_ori, current->files);

	fp_sd = filp_open("/sdcard/fimc_is_fw.bin", O_RDONLY, 0);

	if (IS_ERR(fp_sd))
		return sprintf(buf, "%s\n", fw_ori);
	else {
		vfs_llseek(fp_sd, -7, SEEK_END);
		vfs_read(fp_sd, (char __user *)fw_sd, 7, &fp_sd->f_pos);
		fw_sd[6] = '\0';
		filp_close(fp_sd, current->files);
	}
	set_fs(old_fs);
	return sprintf(buf, "%s %s\n", fw_ori, fw_sd);

}

static DEVICE_ATTR(front_camtype, S_IRUGO,
		s5k6a3_camera_front_camtype_show, NULL);
static DEVICE_ATTR(front_camfw, S_IRUGO, s5k6a3_camera_front_camfw_show, NULL);

static int fimc_is_probe(struct platform_device *pdev)
{
	struct exynos4_platform_fimc_is *pdata;
	struct resource *mem_res;
	struct resource *regs_res;
	struct fimc_is_dev *dev;
	int ret = -ENODEV;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev) {
		dev_err(&pdev->dev, "Not enough memory for FIMC-IS device.\n");
		return -ENOMEM;
	}
	mutex_init(&dev->lock);
	spin_lock_init(&dev->slock);
	init_waitqueue_head(&dev->irq_queue1);

	dev->pdev = pdev;
	if (!dev->pdev) {
		dev_err(&pdev->dev, "No platform data specified\n");
		goto p_err_info;
	}

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev, "Platform data not set\n");
		goto p_err_info;
	}
	dev->pdata = pdata;
	/*
	 * I/O remap
	*/
	mem_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem_res) {
		dev_err(&pdev->dev, "Failed to get io memory region\n");
		ret = -ENOENT;
		goto p_err_info;
	}

	regs_res = request_mem_region(mem_res->start,
		resource_size(mem_res), pdev->name);
	if (!regs_res) {
		dev_err(&pdev->dev, "Failed to request io memory region\n");
		ret = -ENOENT;
		goto p_err_info;
	}
	dev->regs_res = regs_res;

	dev->regs = ioremap(mem_res->start, resource_size(mem_res));
	if (!dev->regs) {
		dev_err(&pdev->dev, "Failed to remap io region\n");
		ret = -ENXIO;
		goto p_err_req_region;
	}

	/*
	 * initialize IRQ , FIMC-IS IRQ : ISP[0] -> SPI[90] , ISP[1] -> SPI[95]
	*/
	dev->irq1 = platform_get_irq(pdev, 0);
	if (dev->irq1 < 0) {
		ret = dev->irq1;
		dev_err(&pdev->dev, "Failed to get irq\n");
		goto p_err_get_irq;
	}

	ret = request_irq(dev->irq1, fimc_is_irq_handler1,
		IRQF_DISABLED, dev_name(&pdev->dev), dev);
	if (ret) {
		dev_err(&pdev->dev, "failed to allocate irq (%d)\n", ret);
		goto p_err_req_irq;
	}

	/*
	 * initialize memory manager
	*/
	ret = fimc_is_init_mem_mgr(dev);
	if (ret) {
		dev_err(&pdev->dev,
			"failed to fimc_is_init_mem_mgr (%d)\n", ret);
		goto p_err_init_mem;
	}
	dbg("Parameter region = 0x%08x\n", (unsigned int)dev->is_p_region);

	/* Init v4l2 sub device */
	v4l2_subdev_init(&dev->sd, &fimc_is_subdev_ops);
	dev->sd.owner = THIS_MODULE;
	strcpy(dev->sd.name, MODULE_NAME);
	v4l2_set_subdevdata(&dev->sd, pdev);

	platform_set_drvdata(pdev, &dev->sd);

	pm_runtime_enable(&pdev->dev);

#ifdef CONFIG_BUSFREQ_OPP
	/* To lock bus frequency in OPP mode */
	dev->bus_dev = dev_get("exynos-busfreq");
#endif
	dev->power = 0;
	dev->state = 0;
	dev->sensor_num = FIMC_IS_SENSOR_NUM;
	dev->sensor.id = 0;
	dev->p_region_index1 = 0;
	dev->p_region_index2 = 0;
	atomic_set(&dev->p_region_num, 0);
	set_bit(IS_ST_IDLE, &dev->state);
	set_bit(IS_PWR_ST_POWEROFF, &dev->power);
	dev->af.af_state = FIMC_IS_AF_IDLE;

	s5k6a3_dev = device_create(camera_class, NULL, 0, NULL, "front");
	if (IS_ERR(s5k6a3_dev)) {
		printk(KERN_ERR "failed to create device!\n");
	} else {
		if (device_create_file(s5k6a3_dev, &dev_attr_front_camtype)
				< 0) {
			printk(KERN_ERR "failed to create device file, %s\n",
				dev_attr_front_camtype.attr.name);
		}
		if (device_create_file(s5k6a3_dev, &dev_attr_front_camfw) < 0) {
			printk(KERN_ERR "failed to create device file, %s\n",
				dev_attr_front_camfw.attr.name);
		}
	}
	dbg("FIMC-IS probe completed\n");
	return 0;

p_err_init_mem:
	free_irq(dev->irq1, dev);
p_err_req_irq:
p_err_get_irq:
	iounmap(dev->regs);
p_err_req_region:
	release_mem_region(regs_res->start, resource_size(regs_res));
p_err_info:
	dev_err(&dev->pdev->dev, "failed to install\n");
	kfree(dev);
	return ret;
}

static int fimc_is_remove(struct platform_device *pdev)
{
	struct v4l2_subdev *sd = platform_get_drvdata(pdev);
	struct fimc_is_dev *dev = to_fimc_is_dev(sd);
#if defined(CONFIG_VIDEOBUF2_ION)
	fimc_is_mem_init_mem_cleanup(dev->alloc_ctx);
#endif
	kfree(dev);
	return 0;
}

static int fimc_is_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct v4l2_subdev *sd = platform_get_drvdata(pdev);
	struct fimc_is_dev *is_dev = to_fimc_is_dev(sd);

	printk(KERN_INFO "FIMC_IS suspend\n");
	mutex_lock(&is_dev->lock);
	if (!test_bit(IS_PWR_ST_POWEROFF, &is_dev->power)) {
		err("not power off state\n");
		fimc_is_s_power(sd, false);
	}
	mutex_unlock(&is_dev->lock);
	printk(KERN_INFO "FIMC_IS suspend end\n");
	return 0;
}

static int fimc_is_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct v4l2_subdev *sd = platform_get_drvdata(pdev);
	struct fimc_is_dev *is_dev = to_fimc_is_dev(sd);

	printk(KERN_INFO "FIMC_IS resume\n");
	mutex_lock(&is_dev->lock);
	mutex_unlock(&is_dev->lock);
	printk(KERN_INFO "FIMC_IS resume end\n");
	return 0;
}

static int fimc_is_runtime_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct v4l2_subdev *sd = platform_get_drvdata(pdev);
	struct fimc_is_dev *is_dev = to_fimc_is_dev(sd);

	printk(KERN_INFO "FIMC_IS runtime suspend\n");
	mutex_lock(&is_dev->lock);
	if (is_dev->pdata->clk_off) {
		is_dev->pdata->clk_off(pdev);
	} else {
		printk(KERN_ERR "#### failed to Clock OFF ####\n");
		return -EINVAL;
	}
#ifdef CONFIG_BUSFREQ_OPP
	/* Unlock bus frequency */
	dev_unlock(is_dev->bus_dev, dev);
#endif
#if defined(CONFIG_VIDEOBUF2_ION)
	if (is_dev->alloc_ctx)
		fimc_is_mem_suspend(is_dev->alloc_ctx);
#endif
	clear_bit(IS_PWR_ST_POWERON, &is_dev->power);
	set_bit(IS_PWR_ST_POWEROFF, &is_dev->power);
	mutex_unlock(&is_dev->lock);
	printk(KERN_INFO "FIMC_IS runtime suspend end\n");
	return 0;
}

static int fimc_is_runtime_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct v4l2_subdev *sd = platform_get_drvdata(pdev);
	struct fimc_is_dev *is_dev = to_fimc_is_dev(sd);

	printk(KERN_INFO "FIMC_IS runtime resume\n");
	mutex_lock(&is_dev->lock);
	if (is_dev->pdata->clk_cfg) {
		is_dev->pdata->clk_cfg(pdev);
	} else {
		printk(KERN_ERR "#### failed to Clock CONFIG ####\n");
		return -EINVAL;
	}
	if (is_dev->pdata->clk_on) {
		is_dev->pdata->clk_on(pdev);
	} else {
		printk(KERN_ERR "#### failed to Clock On ####\n");
		return -EINVAL;
	}
#ifdef CONFIG_BUSFREQ_OPP
	/* lock bus frequency */
	dev_lock(is_dev->bus_dev, dev, BUS_LOCK_FREQ_L0);
#endif
	is_dev->frame_count = 0;
#if defined(CONFIG_VIDEOBUF2_ION)
	if (is_dev->alloc_ctx)
		fimc_is_mem_resume(is_dev->alloc_ctx);
#endif
	clear_bit(IS_PWR_ST_POWEROFF, &is_dev->power);
	clear_bit(IS_PWR_SUB_IP_POWER_OFF, &is_dev->power);
	set_bit(IS_PWR_ST_POWERON, &is_dev->power);
	mutex_unlock(&is_dev->lock);
	printk(KERN_INFO "FIMC_IS runtime resume end\n");
	return 0;
}

static const struct dev_pm_ops fimc_is_pm_ops = {
	.suspend	 = fimc_is_suspend,
	.resume		 = fimc_is_resume,
	.runtime_suspend = fimc_is_runtime_suspend,
	.runtime_resume	 = fimc_is_runtime_resume,
};

static struct platform_driver fimc_is_driver = {
	.probe		= fimc_is_probe,
	.remove		= fimc_is_remove,
	.driver		= {
		.name	= MODULE_NAME,
		.owner	= THIS_MODULE,
		.pm	= &fimc_is_pm_ops,
	},
};

static int __init fimc_is_init(void)
{
	int ret;
	ret = platform_driver_register(&fimc_is_driver);
	if (ret)
		err("platform_driver_register failed: %d\n", ret);
	return ret;
}

static void __exit fimc_is_exit(void)
{
	platform_driver_unregister(&fimc_is_driver);
}

module_init(fimc_is_init);
module_exit(fimc_is_exit);

MODULE_AUTHOR("Younghwan Joo, <yhwan.joo@samsung.com>");
MODULE_DESCRIPTION("Exynos4 series FIMC-IS slave driver");
MODULE_LICENSE("GPL");
