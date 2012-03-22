/*
 * Copyright (C) 2011 Samsung Electronics Co. Ltd.
 *  Hyuk Kang <hyuk78.kang@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/kthread.h>
#include <linux/wakelock.h>
#include <linux/host_notify.h>

struct  host_notifier_info {
	struct host_notifier_platform_data *pdata;
	struct task_struct *th;
	wait_queue_head_t	delay_wait;
	int	thread_remove;

	struct wake_lock	wlock;
};

static struct host_notifier_info ninfo;

static int currentlimit_thread(void *data)
{
	struct host_notifier_info *ninfo = data;
	struct host_notify_dev *ndev = &ninfo->pdata->ndev;
	int gpio = ninfo->pdata->gpio;
	int prev = ndev->booster;
	int ret = 0;

	pr_info("host_notifier usbhostd: start %d\n", prev);

	while (!kthread_should_stop()) {
		wait_event_interruptible_timeout(ninfo->delay_wait,
				ninfo->thread_remove, 1 * HZ);

		ret = gpio_get_value(gpio);
		if (prev != ret) {
			pr_info("host_notifier usbhostd: gpio %d = %s\n",
					gpio, ret ? "HIGH" : "LOW");
			ndev->booster = ret ?
				NOTIFY_POWER_ON : NOTIFY_POWER_OFF;
			prev = ret;

			if (!ret && ndev->mode == NOTIFY_HOST_MODE) {
				host_state_notify(ndev,
						NOTIFY_HOST_OVERCURRENT);
				pr_err("host_notifier usbhostd: overcurrent\n");
				break;
			}
		}
	}

	ninfo->thread_remove = 1;

	pr_info("host_notifier usbhostd: exit %d\n", ret);
	return 0;
}

static int start_usbhostd_thread(void)
{
	if (!ninfo.th) {
		pr_info("host_notifier: start usbhostd thread\n");

		init_waitqueue_head(&ninfo.delay_wait);
		ninfo.thread_remove = 0;
		ninfo.th = kthread_run(currentlimit_thread,
				&ninfo, "usbhostd");

		if (IS_ERR(ninfo.th)) {
			pr_err("host_notifier: Unable to start usbhostd\n");
			ninfo.th = NULL;
			ninfo.thread_remove = 1;
			return -1;
		}
		host_state_notify(&ninfo.pdata->ndev, NOTIFY_HOST_ADD);
		wake_lock(&ninfo.wlock);

	} else
		pr_info("host_notifier: usbhostd already started!\n");

	return 0;
}

static int stop_usbhostd_thread(void)
{
	if (ninfo.th) {
		pr_info("host_notifier: stop thread\n");

		if (!ninfo.thread_remove)
			kthread_stop(ninfo.th);

		ninfo.th = NULL;
		host_state_notify(&ninfo.pdata->ndev, NOTIFY_HOST_REMOVE);
		wake_unlock(&ninfo.wlock);
	} else
		pr_info("host_notifier: no thread\n");

	return 0;
}

static int start_usbhostd_notify(void)
{
	pr_info("host_notifier: start usbhostd notify\n");

	host_state_notify(&ninfo.pdata->ndev, NOTIFY_HOST_ADD);
	wake_lock(&ninfo.wlock);

	return 0;
}

static int stop_usbhostd_notify(void)
{
	pr_info("host_notifier: stop usbhostd notify\n");

	host_state_notify(&ninfo.pdata->ndev, NOTIFY_HOST_REMOVE);
	wake_unlock(&ninfo.wlock);

	return 0;
}

static void host_notifier_booster(int enable)
{
	pr_info("host_notifier: booster %s\n", enable ? "ON" : "OFF");

	ninfo.pdata->booster(enable);

	if (ninfo.pdata->thread_enable) {
		if (enable)
			start_usbhostd_thread();
		else
			stop_usbhostd_thread();
	}
}

static int host_notifier_probe(struct platform_device *pdev)
{
	int ret = 0;

	dev_info(&pdev->dev, "notifier_prove\n");

	ninfo.pdata = pdev->dev.platform_data;
	if (!ninfo.pdata)
		return -ENODEV;

	if (ninfo.pdata->thread_enable) {
		ret = gpio_request(ninfo.pdata->gpio, "host_notifier");
		if (ret) {
			dev_err(&pdev->dev, "failed to request %d\n",
				ninfo.pdata->gpio);
			return -EPERM;
		}
		gpio_direction_input(ninfo.pdata->gpio);
		dev_info(&pdev->dev, "gpio = %d\n", ninfo.pdata->gpio);

		ninfo.pdata->ndev.set_booster = host_notifier_booster;
		ninfo.pdata->usbhostd_start = start_usbhostd_thread;
		ninfo.pdata->usbhostd_stop = stop_usbhostd_thread;
	} else {
		ninfo.pdata->ndev.set_booster = host_notifier_booster;
		ninfo.pdata->usbhostd_start = start_usbhostd_notify;
		ninfo.pdata->usbhostd_stop = stop_usbhostd_notify;
	}

	ret = host_notify_dev_register(&ninfo.pdata->ndev);
	if (ret < 0) {
		dev_err(&pdev->dev, "Failed to host_notify_dev_register\n");
		return ret;
	}
	wake_lock_init(&ninfo.wlock, WAKE_LOCK_SUSPEND, "hostd");

	return 0;
}

static int host_notifier_remove(struct platform_device *pdev)
{
	/* gpio_free(ninfo.pdata->gpio); */
	host_notify_dev_unregister(&ninfo.pdata->ndev);
	wake_lock_destroy(&ninfo.wlock);
	return 0;
}

static struct platform_driver host_notifier_driver = {
	.probe		= host_notifier_probe,
	.remove		= host_notifier_remove,
	.driver		= {
		.name	= "host_notifier",
		.owner	= THIS_MODULE,
	},
};


static int __init host_notifier_init(void)
{
	return platform_driver_register(&host_notifier_driver);
}

static void __init host_notifier_exit(void)
{
	platform_driver_unregister(&host_notifier_driver);
}

module_init(host_notifier_init);
module_exit(host_notifier_exit);

MODULE_AUTHOR("Hyuk Kang <hyuk78.kang@samsung.com>");
MODULE_DESCRIPTION("USB Host notifier");
MODULE_LICENSE("GPL");
