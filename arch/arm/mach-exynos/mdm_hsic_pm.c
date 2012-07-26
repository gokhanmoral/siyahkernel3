/* linux/arch/arm/mach-xxxx/mdm_hsic_pm.c
 * Copyright (C) 2010 Samsung Electronics. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/usb.h>
#include <linux/pm_runtime.h>
#include <plat/gpio-cfg.h>
#include <linux/mdm_hsic_pm.h>
#include <linux/suspend.h>
#include <linux/wakelock.h>
/**
 * TODO:
 * pm notifier register
 *
 * think the way to use notifier to register or unregister device
 *
 * disconnect also can be notified
 *
 * block request under kernel power off seq.
 *
 * in suspend function if busy has set, return
 *
 */

/**
 * struct mdm_hsic_pm_data - hsic pm platform driver private data
 *	provide data and information for pm state transition
 *
 * @name:		name of this pm driver
 * @udev:		usb driver for resuming device from device request
 * @intf_cnt:		count of registered interface driver
 * @block_request:	block and ignore requested resume interrupt
 * @state_busy:		it is not determined to use, it can be replaced to
 *			rpm status check
 * @pm_notifier:	notifier block to control block_request variable
 *			block_reqeust set to true at PM_SUSPEND_PREPARE and
 *			release at PM_POST_RESUME
 *
 */
struct mdm_hsic_pm_data {
	struct list_head list;
	char name[32];

	struct usb_device *udev;
	int intf_cnt;

	/* control variables */
	struct notifier_block pm_notifier;
	bool block_request;
	bool state_busy;

	/* gpio-s and irq */
	int gpio_host_ready;
	int gpio_device_ready;
	int gpio_host_wake;
	int irq;

	/* wakelock for L0 - L2 */
	struct wake_lock l2_wake;

	/* workqueue, work for delayed autosuspend */
	struct workqueue_struct *wq;
	struct delayed_work auto_rpm_start_work;
	struct delayed_work request_resume_work;
};

/**
 * hsic pm device list for multiple modem support
 */
static LIST_HEAD(hsic_pm_dev_list);

static void print_pm_dev_info(struct mdm_hsic_pm_data *pm_data)
{
	pr_info("pm device\n\tname = %s\n"
		"\tudev = 0x%p\n"
		"\tintf_cnt = %d\n"
		"\tblock_request = %s\n",
		pm_data->name,
		pm_data->udev,
		pm_data->intf_cnt,
		pm_data->block_request ? "true" : "false");
}

static struct mdm_hsic_pm_data *get_pm_data_by_dev_name(const char *name)
{
	struct mdm_hsic_pm_data *pm_data;

	if (list_empty(&hsic_pm_dev_list)) {
		pr_err("%s:there's no dev on pm dev list\n", __func__);
		return NULL;
	};

	/* get device from list */
	list_for_each_entry(pm_data, &hsic_pm_dev_list, list) {
		if (!strncmp(pm_data->name, name, strlen(name)))
			break;
		pm_data = NULL;
	}

	return pm_data;
}

void request_active_lock_set(const char *name)
{
	struct mdm_hsic_pm_data *pm_data = get_pm_data_by_dev_name(name);
	pr_info("%s\n", __func__);
	if (pm_data)
		wake_lock(&pm_data->l2_wake);
}
void request_active_lock_release(const char *name)
{
	struct mdm_hsic_pm_data *pm_data = get_pm_data_by_dev_name(name);
	pr_info("%s\n", __func__);
	if (pm_data)
		wake_unlock(&pm_data->l2_wake);
}

void set_host_stat(const char *name, enum pwr_stat status)
{
	/* find pm device from list by name */
	struct mdm_hsic_pm_data *pm_data = get_pm_data_by_dev_name(name);

	if (!pm_data) {
		pr_err("%s:no pm device(%s) exist\n", __func__, name);
		return;
	}

	if (pm_data->gpio_host_ready) {
		pr_info("dev rdy val = %d\n",
				gpio_get_value(pm_data->gpio_device_ready));
		pr_info("%s:set host port power status to [%d]\n",
							__func__, status);
		gpio_set_value(pm_data->gpio_host_ready, status);
	}
}

#define DEV_POWER_WAIT_SPIN	10
#define DEV_POWER_WAIT_MS	20
int wait_dev_pwr_stat(const char *name, enum pwr_stat status)
{
	int spin;
	/* find pm device from list by name */
	struct mdm_hsic_pm_data *pm_data = get_pm_data_by_dev_name(name);

	if (!pm_data) {
		pr_err("%s:no pm device(%s) exist\n", __func__, name);
		return -ENODEV;
	}

	pr_info("%s:[%s]...\n", __func__, status ? "PWR ON" : "PWR OFF");

	if (pm_data->gpio_device_ready) {
		for (spin = 0; spin < DEV_POWER_WAIT_SPIN ; spin++) {
			if (gpio_get_value(pm_data->gpio_device_ready) ==
								status) {
				pr_info("%s:get dev [%s]\n", __func__,
						status ? "PWR ON" : "PWR OFF");
				break;
			}
			msleep(DEV_POWER_WAIT_MS);
			pr_info("dev rdy val = %d\n",
				gpio_get_value(pm_data->gpio_device_ready));
		}
	}

	if (gpio_get_value(pm_data->gpio_device_ready) == status)
		pr_info(" done\n");
	else
		return -EBUSY;
	/*
	#if 0
	if (gpio_get_value(pm_data->gpio_device_ready) != status)
		panic("HSIC L3 power seq\n");
	#else
	while (gpio_get_value(pm_data->gpio_device_ready) != status) {
		pr_info("loop\n");
		msleep(1000);
	};
	#endif
	*/
	return 0;
}

/**
 * check suspended state for L3 drive
 * if not, L3 blocked and stay at L2 / L0 state
 */
int check_udev_suspend_allowed(const char *name)
{
	struct mdm_hsic_pm_data *pm_data = get_pm_data_by_dev_name(name);
	struct device *dev;

	if (!pm_data) {
		pr_err("%s:no pm device(%s) exist\n", __func__, name);
		return -ENODEV;
	}
	if (!pm_data->intf_cnt || !pm_data->udev)
		return -ENODEV;

	dev = &pm_data->udev->dev;

	pr_info("%s:state_busy = %d, suspended = %d(rpmstat = %d:dpth:%d),"
		" suspended_child = %d\n", __func__, pm_data->state_busy,
		pm_runtime_suspended(dev), dev->power.runtime_status,
		dev->power.disable_depth, pm_children_suspended(dev));

	if (pm_data->state_busy)
		return -EBUSY;

	return pm_runtime_suspended(dev) && pm_children_suspended(dev);
}

int register_udev_to_pm_dev(const char *name, struct usb_device *udev)
{
	/* find pm device from list by name */
	struct mdm_hsic_pm_data *pm_data = get_pm_data_by_dev_name(name);

	if (!pm_data) {
		pr_err("%s:no pm device(%s) exist for udev(0x%p)\n",
					__func__, name, udev);
		return -ENODEV;
	}

	print_pm_dev_info(pm_data);

	if (!pm_data->intf_cnt) {
		pr_info("%s: registering new udev(0x%p) to %s\n", __func__,
							udev, pm_data->name);
		pm_data->udev = udev;
		usb_disable_autosuspend(udev);
	} else if (pm_data->udev && pm_data->udev != udev) {
		pr_err("%s:udev mismatching: pm_data->udev(0x%p), udev(0x%p)\n",
		__func__, pm_data->udev, udev);
		return -EINVAL;
	}

	pm_data->intf_cnt++;
	pr_info("%s:udev(0x%p) successfully registerd to %s, intf count = %d\n",
			__func__, udev, pm_data->name, pm_data->intf_cnt);

	queue_delayed_work(pm_data->wq, &pm_data->auto_rpm_start_work,
						msecs_to_jiffies(10000));
	return 0;
}

void unregister_udev_from_pm_dev(const char *name, struct usb_device *udev)
{
	/* find pm device from list by name */
	struct mdm_hsic_pm_data *pm_data = get_pm_data_by_dev_name(name);

	if (!pm_data) {
		pr_err("%s:no pm device(%s) exist for udev(0x%p)\n",
					__func__, name, udev);
		return;
	}

	if (pm_data->udev && pm_data->udev != udev) {
		pr_err("%s:udev mismatching: pm_data->udev(0x%p), udev(0x%p)\n",
		__func__, pm_data->udev, udev);
		return;
	}

	pm_data->intf_cnt--;
	pr_info("%s:udev(0x%p) unregistered from %s, intf count = %d\n",
			__func__, udev, pm_data->name, pm_data->intf_cnt);

	if (!pm_data->intf_cnt) {
		pr_info("%s: all intf device unregistered from %s\n",
						__func__, pm_data->name);
		pm_data->udev = NULL;
	}
}

static void mdm_hsic_rpm_check(struct work_struct *work)
{
	struct mdm_hsic_pm_data *pm_data =
			container_of(work, struct mdm_hsic_pm_data,
					request_resume_work.work);
	struct device *dev;

	pr_info("%s\n", __func__);

	if (!pm_data->udev)
		return;

	dev = &pm_data->udev->dev;

	if (pm_runtime_resume(dev) < 0)
		queue_delayed_work(pm_data->wq, &pm_data->request_resume_work,
							msecs_to_jiffies(20));

	if (pm_runtime_suspended(dev))
		queue_delayed_work(pm_data->wq, &pm_data->request_resume_work,
							msecs_to_jiffies(20));
};

static void mdm_hsic_rpm_start(struct work_struct *work)
{
	struct mdm_hsic_pm_data *pm_data =
			container_of(work, struct mdm_hsic_pm_data,
					auto_rpm_start_work.work);
	struct device *dev;

	pr_info("%s\n", __func__);

	if (!pm_data->intf_cnt || !pm_data->udev)
		return;

	dev = &pm_data->udev->dev;
	pm_runtime_set_autosuspend_delay(dev, 200);
	pm_runtime_allow(dev);
}

static ssize_t store_runtime(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t count)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct mdm_hsic_pm_data *pm_data = platform_get_drvdata(pdev);
	int value;

	if (sscanf(buf, "%d", &value) != 1)
		return -EINVAL;

	if (!pm_data || !pm_data->intf_cnt || !pm_data->udev)
		return -ENXIO;

	if (value == 1) {
		pr_info("%s: request runtime resume\n", __func__);
		if (pm_request_resume(&pm_data->udev->dev) < 0)
			pr_err("%s: unable to add pm work for rpm\n", __func__);
	}

	return count;
}
static DEVICE_ATTR(runtime, 0664, NULL, store_runtime);

static int mdm_hsic_pm_notify_event(struct notifier_block *this,
				unsigned long event, void *ptr)
{
	struct mdm_hsic_pm_data *pm_data =
		container_of(this, struct mdm_hsic_pm_data, pm_notifier);

	switch (event) {
	case PM_SUSPEND_PREPARE:
		/* to catch blocked resume req */
		pm_data->state_busy = false;
		pm_data->block_request = true;
		pr_info("%s: block request\n", __func__);
		return NOTIFY_OK;
	case PM_POST_SUSPEND:
		pm_data->block_request = false;
		pr_info("%s: unblock request\n", __func__);
		/**
		 * cover L2 -> L3 broken and resume req blocked case :
		 * force resume request for the lost request
		 */
		pm_request_resume(&pm_data->udev->dev);

		return NOTIFY_OK;
	}
	return NOTIFY_DONE;
}

#define HSIC_RESUME_TRIGGER_LEVEL	1
static irqreturn_t mdm_hsic_irq_handler(int irq, void *data)
{
	int irq_level;
	struct mdm_hsic_pm_data *pm_data = data;

	if (!pm_data || !pm_data->intf_cnt || !pm_data->udev)
		return IRQ_HANDLED;

	/**
	 * host wake up handler, takes both edge
	 * in rising, isr triggers L2 ->  L0 resume
	 */

	irq_level = gpio_get_value(pm_data->gpio_host_wake);
	pr_info("%s: detect %s edge\n", __func__,
					irq_level ? "Rising" : "Falling");

	if (irq_level != HSIC_RESUME_TRIGGER_LEVEL)
		return IRQ_HANDLED;

	if (pm_data->block_request) {
		pr_info("%s: request blocked by kernel suspending\n", __func__);
		pm_data->state_busy = true;
		return IRQ_HANDLED;
	}

	if (pm_request_resume(&pm_data->udev->dev) < 0)
		pr_err("%s: unable to add pm work for rpm\n", __func__);
	/* check runtime pm runs in Active state, after 100ms */
	queue_delayed_work(pm_data->wq, &pm_data->request_resume_work,
							msecs_to_jiffies(200));
	return IRQ_HANDLED;
}

static int mdm_hsic_pm_gpio_init(struct mdm_hsic_pm_data *pm_data,
						struct platform_device *pdev)
{
	int ret;
	struct resource *res;

	/* get gpio from platform data */

	/* host ready gpio */
	res = platform_get_resource_byname(pdev, IORESOURCE_IO,
					"AP2MDM_HSIC_ACTIVE");
	if (res)
		pm_data->gpio_host_ready = res->start;

	if (pm_data->gpio_host_ready) {
		ret = gpio_request(pm_data->gpio_host_ready, "host_rdy");
		if (ret < 0)
			return ret;
		gpio_direction_output(pm_data->gpio_host_ready, 1);
	} else
		return -ENXIO;

	/* device ready gpio */
	res = platform_get_resource_byname(pdev, IORESOURCE_IO,
					"MDM2AP_DEVICE_PWR_ACTIVE");
	if (res)
		pm_data->gpio_device_ready = res->start;
	if (pm_data->gpio_device_ready) {
		ret = gpio_request(pm_data->gpio_device_ready, "device_rdy");
		if (ret < 0)
			return ret;
		gpio_direction_input(pm_data->gpio_device_ready);
		s3c_gpio_cfgpin(pm_data->gpio_device_ready, S3C_GPIO_INPUT);
	} else
		return -ENXIO;

	/* host wake gpio */
	res = platform_get_resource_byname(pdev, IORESOURCE_IO,
					"MDM2AP_RESUME_REQ");
	if (res)
		pm_data->gpio_host_wake = res->start;
	if (pm_data->gpio_host_wake) {
		ret = gpio_request(pm_data->gpio_host_wake, "host_wake");
		if (ret < 0)
			return ret;
		gpio_direction_input(pm_data->gpio_host_wake);
		s3c_gpio_cfgpin(pm_data->gpio_host_wake, S3C_GPIO_SFN(0xF));
	} else
		return -ENXIO;

	if (pm_data->gpio_host_wake)
		pm_data->irq = gpio_to_irq(pm_data->gpio_host_wake);

	if (!pm_data->irq) {
		pr_err("fail to get host wake irq\n");
		return -ENXIO;
	}

	return 0;
}

static void mdm_hsic_pm_gpio_free(struct mdm_hsic_pm_data *pm_data)
{
	if (pm_data->gpio_host_ready)
		gpio_free(pm_data->gpio_host_ready);

	if (pm_data->gpio_device_ready)
		gpio_free(pm_data->gpio_device_ready);

	if (pm_data->gpio_host_wake)
		gpio_free(pm_data->gpio_host_wake);
}

static int mdm_hsic_pm_probe(struct platform_device *pdev)
{
	int ret;
	struct mdm_hsic_pm_data *pm_data;

	pr_info("%s for %s\n", __func__, pdev->name);

	pm_data = kzalloc(sizeof(struct mdm_hsic_pm_data), GFP_KERNEL);
	if (!pm_data) {
		pr_err("%s: fail to alloc pm_data\n", __func__);
		return -ENOMEM;
	}

	/* initial value */
	memcpy(pm_data->name, pdev->name, strlen(pdev->name));

	ret = mdm_hsic_pm_gpio_init(pm_data, pdev);
	if (ret < 0)
		goto err_gpio_init_fail;

	/* request irq for host wake interrupt */
	ret = request_irq(pm_data->irq, mdm_hsic_irq_handler,
		IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING | IRQF_DISABLED,
		pm_data->name, (void *)pm_data);
	if (ret < 0) {
		pr_err("%s: fail to request irq(%d)\n", __func__, ret);
		goto err_request_irq;
	}

	ret = enable_irq_wake(pm_data->irq);
	if (ret < 0) {
		pr_err("%s: fail to set wake irq(%d)\n", __func__, ret);
		goto err_set_wake_irq;
	}

	pm_data->wq = create_singlethread_workqueue("hsicrpmd");
	if (!pm_data->wq) {
		pr_err("%s: fail to create wq\n", __func__);
		goto err_create_wq;
	}

	INIT_DELAYED_WORK(&pm_data->auto_rpm_start_work, mdm_hsic_rpm_start);
	INIT_DELAYED_WORK(&pm_data->request_resume_work, mdm_hsic_rpm_check);
	/* register notifier call */
	pm_data->pm_notifier.notifier_call = mdm_hsic_pm_notify_event;
	register_pm_notifier(&pm_data->pm_notifier);

	ret = device_create_file(&pdev->dev, &dev_attr_runtime);
	if (ret < 0) {
		pr_err("%s: fail to create sys file(%d)\n", __func__, ret);
		goto err_create_sys_file;
	}

	wake_lock_init(&pm_data->l2_wake, WAKE_LOCK_SUSPEND, pm_data->name);

	print_pm_dev_info(pm_data);
	list_add(&pm_data->list, &hsic_pm_dev_list);
	platform_set_drvdata(pdev, pm_data);
	pr_info("%s for %s has done\n", __func__, pdev->name);

	return 0;

err_create_sys_file:
	destroy_workqueue(pm_data->wq);
err_create_wq:
	disable_irq_wake(pm_data->irq);
err_set_wake_irq:
	free_irq(pm_data->irq, (void *)pm_data);
err_request_irq:
err_gpio_init_fail:
	mdm_hsic_pm_gpio_free(pm_data);
	kfree(pm_data);
	return -ENXIO;
}

static struct platform_driver mdm_pm_driver = {
	.probe = mdm_hsic_pm_probe,
	.driver = {
		.name = "mdm_hsic_pm0",
		.owner = THIS_MODULE,
	},
};

static int __init mdm_hsic_pm_init(void)
{
	return platform_driver_register(&mdm_pm_driver);
}

static void __exit mdm_hsic_pm_exit(void)
{
	platform_driver_unregister(&mdm_pm_driver);
}

late_initcall(mdm_hsic_pm_init);
