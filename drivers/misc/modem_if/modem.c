/* linux/drivers/modem/modem.c
 *
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2010 Samsung Electronics.
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
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/wakelock.h>

#include <linux/platform_data/modem.h>
#include "modem_prj.h"
#include "modem_variation.h"
#include "modem_utils.h"

#define FMT_WAKE_TIME   (HZ/2)
#define RAW_WAKE_TIME   (HZ*6)

static struct modem_ctl *create_modemctl_device(struct platform_device *pdev)
{
	int ret = 0;
	struct modem_data *pdata;
	struct modem_ctl *modemctl;
	struct device *dev = &pdev->dev;

	/* create modem control device */
	modemctl = kzalloc(sizeof(struct modem_ctl), GFP_KERNEL);
	if (!modemctl)
		return NULL;

	modemctl->dev = dev;
	modemctl->phone_state = STATE_OFFLINE;

	pdata = pdev->dev.platform_data;
	modemctl->mdm_data = pdata;
	modemctl->name = pdata->name;

	/* initialize link device list */
	INIT_LIST_HEAD(&modemctl->link_dev_list);

	/* init modemctl device for getting modemctl operations */
	ret = call_modem_init_func(modemctl, pdata);
	if (ret) {
		kfree(modemctl);
		return NULL;
	}

	pr_info("[MIF] <%s> %s is created!!!\n", __func__, pdata->name);
	return modemctl;
}

static struct io_device *create_io_device(struct modem_io_t *io_t,
		struct modem_ctl *modemctl, struct modem_data *pdata)
{
	int ret = 0;
	struct io_device *iod = NULL;

	iod = kzalloc(sizeof(struct io_device), GFP_KERNEL);
	if (!iod) {
		pr_err("[MIF/E] <%s> iod == NULL\n", __func__);
		return NULL;
	}

	INIT_LIST_HEAD(&iod->list);
	iod->name = io_t->name;
	iod->id = io_t->id;
	iod->format = io_t->format;
	iod->io_typ = io_t->io_type;
	iod->link_types = io_t->links;
	iod->net_typ = pdata->modem_net;
	iod->use_handover = pdata->use_handover;

	/* link between io device and modem control */
	iod->mc = modemctl;
	if (iod->format == IPC_FMT)
		modemctl->iod = iod;
	if (iod->format == IPC_BOOT) {
		modemctl->bootd = iod;
		pr_info("[MIF] <%s> Bood device = %s\n", __func__, iod->name);
	}

	/* register misc device or net device */
	ret = init_io_device(iod);
	if (ret) {
		kfree(iod);
		pr_err("[MIF/E] <%s> init_io_device fail (%d)\n",
			__func__, ret);
		return NULL;
	}

	pr_debug("[MIF] <%s> %s is created!!!\n", __func__, iod->name);
	return iod;
}

static int attach_devices(struct modem_ctl *mc, struct io_device *iod,
		struct io_raw_devices *io_raw_devs,
		struct list_head *list_of_io_devs, enum modem_link tx_link)
{
	struct link_device *ld;

	/* add iod to ld->list_of_io_devices */
	if (iod->format != IPC_RAW)
		list_add(&iod->list, list_of_io_devs);

	/* find link type for this io device */
	list_for_each_entry(ld, &mc->link_dev_list, list) {
		if (LINKTYPE(ld->link_type) & iod->link_types) {
			/* The count 1 bits of iod->link_types is count
			 * of link devices of this iod.
			 * If use one link device,
			 * or, 2+ link devices and this link is tx_link,
			 * set iod->link with ld
			 */
			if ((countbits(iod->link_types) <= 1) ||
					(tx_link == ld->link_type)) {
				pr_debug("[MIF] set iod(%s)->link(%s)\n",
						iod->name, ld->name);

				iod->link = ld;

				if (iod->format == IPC_RAW) {
					int ch = iod->id & 0x1F;
					io_raw_devs->raw_devices[ch] = iod;
					io_raw_devs->num_of_raw_devs++;
				}
			}
		}
	}

	/* if use rx dynamic switch, set tx_link at modem_io_t of
	 * board-*-modems.c
	 */
	if (!iod->link) {
		pr_err("[MIF/E] <%s> iod(%s)->link == NULL\n",
			__func__, iod->name);
		BUG();
	}

	if (iod->format == IPC_MULTI_RAW)
		iod->private_data = io_raw_devs;

	if (iod->format == IPC_FMT ||
			iod->format == IPC_MULTI_RAW) {
		wake_lock_init(&iod->wakelock,
				WAKE_LOCK_SUSPEND, iod->name);
		if (iod->format == IPC_FMT)
			iod->waketime = FMT_WAKE_TIME;
		if (iod->format == IPC_MULTI_RAW)
			iod->waketime = RAW_WAKE_TIME;
	}

	return 0;
}

static int __devinit modem_probe(struct platform_device *pdev)
{
	int i;
	struct modem_data *pdata = pdev->dev.platform_data;
	struct modem_ctl *modemctl;
	struct io_device *iod[pdata->num_iodevs];
	struct link_device *ld;
	struct io_raw_devices *io_raw_devs = NULL;
	struct list_head *list_of_io_devs = NULL;

	memset(iod, 0, sizeof(iod));

	modemctl = create_modemctl_device(pdev);
	if (!modemctl) {
		pr_err("[MIF/E] <%s> modemctl == NULL\n", __func__);
		goto err_free_modemctl;
	}

	list_of_io_devs = kzalloc(sizeof(struct list_head), GFP_KERNEL);
	if (!list_of_io_devs) {
		pr_err("[MIF/E] <%s> list_of_io_devs == NULL\n", __func__);
		goto err_free_modemctl;
	}
	INIT_LIST_HEAD(list_of_io_devs);

	/* create link device */
	/* support multi-link device */
	for (i = 0; i < LINKDEV_MAX ; i++) {
		/* find matching link type */
		if (pdata->link_types & LINKTYPE(i)) {
			ld = call_link_init_func(pdev, i);
			if (ld) {
				pr_err("[MIF] %s: link created: %s\n",
						__func__, ld->name);
				ld->link_type = i;
				ld->list_of_io_devices = list_of_io_devs;
				list_add(&ld->list, &modemctl->link_dev_list);
			} else
				goto err_free_modemctl;
		}
	}

	io_raw_devs = kzalloc(sizeof(struct io_raw_devices), GFP_KERNEL);
	if (!io_raw_devs) {
		pr_err("[MIF/E] <%s> io_raw_devs == NULL\n", __func__);
		goto err_free_modemctl;
	}

	/* create io deivces and connect to modemctl device */
	for (i = 0; i < pdata->num_iodevs; i++) {
		iod[i] = create_io_device(&pdata->iodevs[i], modemctl, pdata);
		if (!iod[i]) {
			pr_err("[MIF/E] <%s> iod[%d] == NULL\n",
				__func__, i);
			goto err_free_modemctl;
		}

		attach_devices(modemctl, iod[i], io_raw_devs,
				list_of_io_devs, pdata->iodevs[i].tx_link);
	}

	platform_set_drvdata(pdev, modemctl);

	pr_info("[MIF] <%s> Complete!!!\n", __func__);
	return 0;

err_free_modemctl:
	for (i = 0; i < pdata->num_iodevs; i++)
		if (iod[i] != NULL)
			kfree(iod[i]);

	if (io_raw_devs != NULL)
		kfree(io_raw_devs);

	if (list_of_io_devs != NULL)
		kfree(list_of_io_devs);

	if (modemctl != NULL)
		kfree(modemctl);

	return -ENOMEM;
}

static void modem_shutdown(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct modem_ctl *mc = dev_get_drvdata(dev);
	mc->ops.modem_off(mc);
}

static int modem_suspend(struct device *pdev)
{
#ifndef CONFIG_LINK_DEVICE_HSIC
	struct modem_ctl *mc = dev_get_drvdata(pdev);

	if (mc->gpio_pda_active)
		gpio_set_value(mc->gpio_pda_active, 0);
#endif

	return 0;
}

static int modem_resume(struct device *pdev)
{
#ifndef CONFIG_LINK_DEVICE_HSIC
	struct modem_ctl *mc = dev_get_drvdata(pdev);

	if (mc->gpio_pda_active)
		gpio_set_value(mc->gpio_pda_active, 1);
#endif

	return 0;
}

static const struct dev_pm_ops modem_pm_ops = {
	.suspend    = modem_suspend,
	.resume     = modem_resume,
};

static struct platform_driver modem_driver = {
	.probe = modem_probe,
	.shutdown = modem_shutdown,
	.driver = {
		.name = "modem_if",
		.pm   = &modem_pm_ops,
	},
};

static int __init modem_init(void)
{
	return platform_driver_register(&modem_driver);
}

module_init(modem_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Samsung Modem Interface Driver");
