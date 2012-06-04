/* /linux/drivers/new_modem_if/link_dev_usb.c
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
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <linux/gpio.h>
#include <linux/if_arp.h>
#include <linux/usb.h>
#include <linux/usb/cdc.h>
#include <linux/pm_runtime.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/wakelock.h>
#include <linux/suspend.h>
#include <linux/version.h>

#include <linux/platform_data/modem.h>
#include "modem_prj.h"
#include "modem_link_device_hsic.h"
#include "modem_utils.h"

static struct modem_ctl *if_usb_get_modemctl(struct link_pm_data *pm_data);
static int link_pm_runtime_get_active(struct link_pm_data *pm_data);
static int usb_tx_urb_with_skb(struct usb_device *usbdev, struct sk_buff *skb,
					struct if_usb_devdata *pipe_data);
#ifdef FOR_TEGRA
#define ehci_vendor_txfilltuning tegra_ehci_txfilltuning
#else
#define ehci_vendor_txfilltuning()
#endif
static void usb_rx_complete(struct urb *urb);

#if 1
static void usb_set_autosuspend_delay(struct usb_device *usbdev, int delay)
{
	pm_runtime_set_autosuspend_delay(&usbdev->dev, delay);
}
#else
static void usb_set_autosuspend_delay(struct usb_device *usbdev, int delay)
{
	usbdev->autosuspend_delay = msecs_to_jiffies(delay);
}
#endif

static int start_ipc(struct link_device *ld)
{
	struct sk_buff *skb;
	char data[1] = {'a'};
	int err;
	struct usb_link_device *usb_ld = to_usb_link_device(ld);
	struct if_usb_devdata *pipe_data = &usb_ld->devdata[IF_USB_FMT_EP];

	if (!usb_ld->if_usb_connected) {
		pr_err("[MODEM_IF] HSIC not connected, skip start ipc\n");
		err = -ENODEV;
		goto exit;
	}

	pr_err("[MODEM_IF] send 'a'\n");

	skb = alloc_skb(16, GFP_ATOMIC);
	if (unlikely(!skb))
		return -ENOMEM;
	memcpy(skb_put(skb, 1), data, 1);

	err = usb_tx_urb_with_skb(usb_ld->usbdev, skb, pipe_data);
	if (err < 0) {
		pr_err("usb_tx_urb fail - %s\n", __func__);
		dev_kfree_skb_any(skb);
		goto exit;
	}
exit:
	return err;
}

static void stop_ipc(struct link_device *ld)
{
	ld->com_state = COM_NONE;
}

static int usb_init_communication(struct link_device *ld,
			struct io_device *iod)
{
	/* Send IPC Start ASCII 'a' */
	if (iod->id == 0x1)
		return start_ipc(ld);

	return 0;
}

static void usb_terminate_communication(struct link_device *ld,
			struct io_device *iod)
{
	if (iod->id != 0x1 || iod->format != IPC_FMT)
		return;

	if (iod->mc->phone_state == STATE_CRASH_RESET ||
			iod->mc->phone_state == STATE_CRASH_EXIT)
		stop_ipc(ld);
}

static int usb_rx_submit(struct usb_link_device *usb_ld,
					struct if_usb_devdata *pipe_data,
					gfp_t gfp_flags)
{
	int ret;
	struct urb *urb;

	if (pipe_data->disconnected)
		return 0;

	ehci_vendor_txfilltuning();

	urb = pipe_data->urb;

	urb->transfer_flags = 0;
	usb_fill_bulk_urb(urb, pipe_data->usbdev,
				pipe_data->rx_pipe, pipe_data->rx_buf,
				pipe_data->rx_buf_size, usb_rx_complete,
				(void *)pipe_data);

	usb_mark_last_busy(usb_ld->usbdev);
	ret = usb_submit_urb(urb, gfp_flags);
	if (ret)
		pr_err("%s: submit urb fail with ret (%d)\n", __func__, ret);

	return ret;
}

static void usb_rx_complete(struct urb *urb)
{
	struct if_usb_devdata *pipe_data = urb->context;
	struct usb_link_device *usb_ld = pipe_data->usb_ld;
	struct io_device *iod;
	int iod_format;
	int ret;

	usb_mark_last_busy(usb_ld->usbdev);

	switch (urb->status) {
	case -ENOENT:
		/* case for 'link pm suspended but rx data had remained' */
	case 0:
		if (!urb->actual_length)
			goto rx_submit;

		/* call iod recv */
		/* how we can distinguish boot ch with fmt ch ?? */
		switch (pipe_data->format) {
		case IF_USB_FMT_EP:
			iod_format = IPC_FMT;
			pr_buffer("IPC-RX", (char *)urb->transfer_buffer,
				(size_t)urb->actual_length, MAX_SKB_LOG_LEN);
			break;
		case IF_USB_RAW_EP:
			iod_format = IPC_MULTI_RAW;
			break;
		case IF_USB_RFS_EP:
			iod_format = IPC_RFS;
			break;
		case IF_USB_CMD_EP:
			iod_format = IPC_CMD;
			break;
		default:
			iod_format = -1;
			break;
		}

		/* flow control CMD by CP, not use io device */
		if (unlikely(iod_format == IPC_CMD)) {
			ret = link_rx_flowctl_cmd(&usb_ld->ld,
					(char *)urb->transfer_buffer,
					urb->actual_length);
			if (ret < 0)
				pr_err("%s: no multi raw device (%d)\n",
						__func__, ret);
			goto rx_submit;
		}

		io_devs_for_each(iod, &usb_ld->ld) {
			/* during boot stage fmt end point */
			/* shared with boot io device */
			/* when we use fmt device only for boot and ipc,
				it can be reduced to 1 device */
			if (iod_format == IPC_FMT &&
				unlikely(iod->mc->phone_state == STATE_BOOTING))
				iod_format = IPC_BOOT;

			if (iod->format == iod_format) {
				ret = iod->recv(iod,
						&usb_ld->ld,
						(char *)urb->transfer_buffer,
						urb->actual_length);
				if (ret < 0)
					pr_err("%s: io device recv error (%d)\n"
						, __func__, ret);
				break;
			}
		}
rx_submit:
		if (urb->status == 0) {
			usb_mark_last_busy(usb_ld->usbdev);
			usb_rx_submit(usb_ld, pipe_data, GFP_ATOMIC);
		}
		break;
	case -ECONNRESET:
	case -ESHUTDOWN:
		pr_err("%s: RX complete Status(%d)\n", __func__, urb->status);
		break;
	case -EOVERFLOW:
		pr_err("%s: RX overflow\n", __func__);
		break;
	case -EILSEQ:
		break;
	default:
		break;
	}
}

static int usb_send(struct link_device *ld, struct io_device *iod,
			struct sk_buff *skb)
{
	struct sk_buff_head *txq;
	size_t tx_size;

	switch (iod->format) {
	case IPC_RAW:
		txq = &ld->sk_raw_tx_q;

		if (unlikely(ld->raw_tx_suspended)) {
			pr_err("[MODEM_IF] %s: wait RESUME CMD...\n", __func__);
			INIT_COMPLETION(ld->raw_tx_resumed_by_cp);
			wait_for_completion(&ld->raw_tx_resumed_by_cp);
			pr_err("[MODEM_IF] %s: resumed done.\n", __func__);
		}
		break;
	case IPC_BOOT:
	case IPC_FMT:
	case IPC_RFS:
	default:
		txq = &ld->sk_fmt_tx_q;
		break;
	}
	/* store the tx size before run the tx_delayed_work*/
	tx_size = skb->len;

	/* drop packet, when link is not online */
	if (ld->com_state == COM_BOOT && iod->format != IPC_BOOT) {
		dev_kfree_skb_any(skb);
		pr_err("[MODEM_IF] %s: %s: drop packet, size=%d, com_state=%d\n",
				__func__, iod->name, skb->len, ld->com_state);
		return -ENODEV;
	}

	/* save io device */
	skbpriv(skb)->iod = iod;
	/* en queue skb data */
	skb_queue_tail(txq, skb);

	if (!work_pending(&ld->tx_delayed_work.work))
		queue_delayed_work(ld->tx_wq, &ld->tx_delayed_work, 0);

	return tx_size;
}

static void usb_tx_complete(struct urb *urb)
{
	struct sk_buff *skb = urb->context;

	switch (urb->status) {
	case 0:
		break;
	case -ENOENT:
	case -ECONNRESET:
	case -ESHUTDOWN:
	default:
		pr_err("%s:TX error (%d)\n", __func__, urb->status);
	}

	dev_kfree_skb_any(skb);
	usb_mark_last_busy(urb->dev);
	usb_free_urb(urb);
}

/* Even if usb_tx_urb_with_skb is failed, does not release the skb to retry */
static int usb_tx_urb_with_skb(struct usb_device *usbdev, struct sk_buff *skb,
					struct if_usb_devdata *pipe_data)
{
	int ret;
	struct urb *urb;

	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) {
		pr_err("%s alloc urb error\n", __func__);
		return -ENOMEM;
	}
#if 0
	int i;
	for (i = 0; i < skb->len; i++) {
		if (i > 16)
			break;
		pr_err("[0x%02x]", *(skb->data + i));
	}
#endif
	usb_mark_last_busy(usbdev);
	urb->transfer_flags = URB_ZERO_PACKET;
	usb_fill_bulk_urb(urb, pipe_data->usbdev, pipe_data->tx_pipe, skb->data,
			skb->len, usb_tx_complete, (void *)skb);

	usb_mark_last_busy(usbdev);
	ret = usb_submit_urb(urb, GFP_KERNEL);
	if (ret < 0) {
		pr_err("%s usb_submit_urb with ret(%d)\n",
				__func__, ret);
		usb_free_urb(urb);
		return ret;
	}

	return 0;
}

static void usb_tx_work(struct work_struct *work)
{
	int ret = 0;
	struct link_device *ld =
		container_of(work, struct link_device, tx_delayed_work.work);
	struct usb_link_device *usb_ld = to_usb_link_device(ld);
	struct io_device *iod;
	struct sk_buff *skb;
	struct if_usb_devdata *pipe_data;
	struct link_pm_data *pm_data = usb_ld->link_pm_data;
	struct device *dev;

	dev = &usb_ld->usbdev->dev;

	while (ld->sk_fmt_tx_q.qlen || ld->sk_raw_tx_q.qlen) {
		/* request and check usb runtime pm first */
		ret = link_pm_runtime_get_active(pm_data);
		if (ret < 0) {
			if (ret == -ENODEV) {
				skb_queue_purge(&ld->sk_fmt_tx_q);
				skb_queue_purge(&ld->sk_raw_tx_q);
			} else
				queue_delayed_work(ld->tx_wq,
				&ld->tx_delayed_work, msecs_to_jiffies(20));
			return;
		}
		usb_mark_last_busy(usb_ld->usbdev);
		pm_runtime_get_sync(dev);

		/* send skb from fmt_txq and raw_txq,*/
		/* one by one for fair flow control */
		skb = skb_dequeue(&ld->sk_fmt_tx_q);
		if (skb) {
			iod = skbpriv(skb)->iod;
			switch (iod->format) {
			case IPC_BOOT:
			case IPC_FMT:
				/* boot device uses same intf with fmt*/
				pipe_data = &usb_ld->devdata[IF_USB_FMT_EP];
				break;
			case IPC_RFS:
				pipe_data = &usb_ld->devdata[IF_USB_RFS_EP];
				break;
			default:
				/* wrong packet for fmt tx q , drop it */
				pipe_data =  NULL;
				break;
			}

			if (!pipe_data) {
				dev_kfree_skb_any(skb);
				pm_runtime_put(dev);
				continue;
			}

			if (iod->format == IPC_FMT)
				pr_skb("IPC-TX", skb);

			usb_mark_last_busy(usb_ld->usbdev);
			ret = usb_tx_urb_with_skb(usb_ld->usbdev,
						skb,
						pipe_data);
			pr_debug("TX[F]\n");
			if (ret < 0) {
				if (ret == -ENODEV) {
					dev_kfree_skb_any(skb);
					skb_queue_purge(&ld->sk_fmt_tx_q);
					skb_queue_purge(&ld->sk_raw_tx_q);
					/* when if disconnected, runtime call
					 * for 'dev pointer' makes bugs, dev
					 * has already broken through
					 * disconnection, so do not call
					 * runtime_put here */
					return;
				}
				pr_err("%s usb_tx_urb_with_skb for iod(%d)\n",
						__func__, iod->format);
				skb_queue_head(&ld->sk_fmt_tx_q, skb);
				queue_delayed_work(ld->tx_wq,
						&ld->tx_delayed_work,
						msecs_to_jiffies(20));
				pm_runtime_put(dev);
				return;
			}
		}

		usb_mark_last_busy(usb_ld->usbdev);
		skb = skb_dequeue(&ld->sk_raw_tx_q);
		if (skb) {
			iod = skbpriv(skb)->iod;
			switch (iod->format) {
			case IPC_RAW:
				pipe_data = &usb_ld->devdata[IF_USB_RAW_EP];
				break;
			default:
				/* wrong packet for raw tx q , drop it */
				pipe_data =  NULL;
				break;
			}

			if (!pipe_data) {
				dev_kfree_skb_any(skb);
				pm_runtime_put(dev);
				continue;
			}

			usb_mark_last_busy(usb_ld->usbdev);
			ret = usb_tx_urb_with_skb(usb_ld->usbdev,
							skb, pipe_data);
			pr_debug("TX[R]\n");
			if (ret < 0) {
				if (ret == -ENODEV) {
					dev_kfree_skb_any(skb);
					skb_queue_purge(&ld->sk_fmt_tx_q);
					skb_queue_purge(&ld->sk_raw_tx_q);
					/* when if disconnected, runtime call
					 * for 'dev pointer' makes bugs, dev
					 * has already broken through
					 * disconnection, so do not call
					 * runtime_put here */
					return;
				}
				pr_err("%s usb_tx_urb_with_skb for iod(%d)\n",
						__func__, iod->format);
				skb_queue_head(&ld->sk_raw_tx_q, skb);
				queue_delayed_work(ld->tx_wq,
						&ld->tx_delayed_work,
						msecs_to_jiffies(20));
				pm_runtime_put(dev);
				return;
			}
		}
		pm_runtime_put(dev);
		usb_mark_last_busy(usb_ld->usbdev);
	}
}

/*
#ifdef CONFIG_LINK_PM
*/

static int link_pm_runtime_get_active(struct link_pm_data *pm_data)
{
	int ret;
	struct usb_link_device *usb_ld = pm_data->usb_ld;
	struct device *dev = &usb_ld->usbdev->dev;

	if (!usb_ld->if_usb_connected || usb_ld->ld.com_state == COM_NONE)
		return -ENODEV;

	if (pm_data->dpm_suspending) {
		pr_err("[MIF] Kernel in suspending try get_active later\n");
		return -EAGAIN;
	}

	if (dev->power.runtime_status == RPM_ACTIVE) {
		pm_data->resume_retry_cnt = 0;
		return 0;
	}

	if (!pm_data->resume_requested) {
		pr_debug("[MIF] QW PM\n");
		queue_delayed_work(pm_data->wq, &pm_data->link_pm_work, 0);
	}
	pr_debug("[MIF] Wait pm\n");
	INIT_COMPLETION(pm_data->active_done);
	ret = wait_for_completion_timeout(&pm_data->active_done,
						msecs_to_jiffies(500));
	/* check link state, after completion timeout */
	if (usb_ld->ld.com_state == COM_NONE)
		return -ENODEV;

	if (dev->power.runtime_status != RPM_ACTIVE) {
		pr_info("[MIF] link_active (%d) retry\n",
						dev->power.runtime_status);
		return -EAGAIN;
	}
	pr_debug("[MIF] link_active success(%d)\n", ret);
	return 0;
}

static void link_pm_runtime_start(struct work_struct *work)
{
	struct link_pm_data *pm_data =
		container_of(work, struct link_pm_data, link_pm_start.work);
	struct usb_device *usbdev = pm_data->usb_ld->usbdev;
	struct device *dev, *ppdev;

	if (!pm_data->usb_ld->if_usb_connected
		|| pm_data->usb_ld->ld.com_state == COM_NONE) {
		pr_debug("mif: %s: disconnect status, ignore\n", __func__);
		return;
	}

	dev = &pm_data->usb_ld->usbdev->dev;

	/* wait interface driver resumming */
	if (dev->power.runtime_status == RPM_SUSPENDED) {
		pr_info("mif: %s: suspended yet, delayed work\n", __func__);
		queue_delayed_work(pm_data->wq, &pm_data->link_pm_start,
			msecs_to_jiffies(20));
		return;
	}

	if (pm_data->usb_ld->usbdev && dev->parent) {
		pr_info("%s: rpm_status: %d\n", __func__,
			dev->power.runtime_status);
		usb_set_autosuspend_delay(usbdev, 200);
		ppdev = dev->parent->parent;
		pm_runtime_allow(dev);
		pm_runtime_allow(ppdev);/*ehci*/
		pm_data->link_pm_active = true;
		pm_data->resume_requested = false;
		pm_data->dpm_suspending = false;
		pm_data->link_reconnect_cnt = 2;
		pm_data->resume_retry_cnt = 0;
	}
}

static void link_pm_change_modem_state(struct link_pm_data *pm_data,
						enum modem_state state)
{
	struct modem_ctl *mc = if_usb_get_modemctl(pm_data);

	if (!mc->iod || pm_data->usb_ld->ld.com_state != COM_ONLINE)
		return;

	pr_err("%s: set modem state %d\n", __func__, state);
	mc->iod->modem_state_changed(mc->iod, state);
}

static void link_pm_reconnect_work(struct work_struct *work)
{
	struct link_pm_data *pm_data =
		container_of(work, struct link_pm_data,
					link_reconnect_work.work);
	struct modem_ctl *mc = if_usb_get_modemctl(pm_data);

	if (!mc || pm_data->usb_ld->if_usb_connected)
		return;

	if (pm_data->usb_ld->ld.com_state != COM_ONLINE)
		return;

	if (pm_data->link_reconnect_cnt--) {
		if (mc->phone_state == STATE_ONLINE &&
						!pm_data->link_reconnect())
			/* try reconnect and check */
			schedule_delayed_work(&pm_data->link_reconnect_work,
							msecs_to_jiffies(500));
		else	/* under cp crash or reset, just return */
			return;
	} else {
		/* try to recover cp */
		pr_err("%s: recover connection: silent reset\n", __func__);
		link_pm_change_modem_state(pm_data, STATE_CRASH_RESET);
	}
}

static inline int link_pm_slave_wake(struct link_pm_data *pm_data)
{
	int spin = 20;

	/* do not wake slave first, when host is not in active */
	if (!gpio_get_value(pm_data->gpio_link_active))
		return spin;

	/* when slave device is in sleep, wake up slave cpu first */
	if (gpio_get_value(pm_data->gpio_link_hostwake)
				!= HOSTWAKE_TRIGLEVEL) {
		if (gpio_get_value(pm_data->gpio_link_slavewake)) {
			gpio_set_value(pm_data->gpio_link_slavewake, 0);
			pr_info("[MIF] gpio [SWK] set [0]\n");
			mdelay(5);
		}
		gpio_set_value(pm_data->gpio_link_slavewake, 1);
		pr_info("[MIF] gpio [SWK] set [1]\n");
		mdelay(5);

		/* wait host wake signal*/
		while (spin-- && gpio_get_value(pm_data->gpio_link_hostwake) !=
							HOSTWAKE_TRIGLEVEL)
			mdelay(5);
	}
	/* runtime pm goes to active */
	if (!gpio_get_value(pm_data->gpio_link_active)) {
		pr_info("[MIF] gpio [H ACTV : %d] set 1\n",
				gpio_get_value(pm_data->gpio_link_active));
		gpio_set_value(pm_data->gpio_link_active, 1);
	}
	return spin;
}

static void link_pm_runtime_work(struct work_struct *work)
{
	int ret;
	struct link_pm_data *pm_data =
		container_of(work, struct link_pm_data, link_pm_work.work);
	struct device *dev = &pm_data->usb_ld->usbdev->dev;

	if (!pm_data->usb_ld->if_usb_connected || pm_data->dpm_suspending)
		return;

	if (pm_data->usb_ld->ld.com_state == COM_NONE)
		return;

	pr_debug("%s: for dev 0x%p : current %d\n", __func__, dev,
						dev->power.runtime_status);

	switch (dev->power.runtime_status) {
	case RPM_ACTIVE:
		pm_data->resume_retry_cnt = 0;
		pm_data->resume_requested = false;
		complete(&pm_data->active_done);

		return;
	case RPM_SUSPENDED:
		if (pm_data->resume_requested)
			break;
		pm_data->resume_requested = true;
		wake_lock(&pm_data->rpm_wake);
		ret = link_pm_slave_wake(pm_data);
		if (ret < 0) {
			pr_err("%s: slave wake fail\n", __func__);
			wake_unlock(&pm_data->rpm_wake);
			break;
		}

		ret = pm_runtime_resume(dev);
		if (ret < 0) {
			pr_err("%s: resume error(%d)\n", __func__, ret);
			/* force to go runtime idle before retry resume */
			if (dev->power.timer_expires == 0 &&
						!dev->power.request_pending) {
				pr_info("%s:run time idle\n", __func__);
				pm_runtime_idle(dev);
			}
		}
		wake_unlock(&pm_data->rpm_wake);
		break;
	default:
		break;
	}
	pm_data->resume_requested = false;

	/* check until runtime_status goes to active */
	/* attemp 10 times, or re-establish modem-link */
	/* if pm_runtime_resume run properly, rpm status must be in ACTIVE */
	if (dev->power.runtime_status == RPM_ACTIVE) {
		pm_data->resume_retry_cnt = 0;
		complete(&pm_data->active_done);
	} else if (pm_data->resume_retry_cnt++ > 10)
		link_pm_change_modem_state(pm_data, STATE_CRASH_RESET);
	else
		queue_delayed_work(pm_data->wq, &pm_data->link_pm_work,
							msecs_to_jiffies(20));
}

static irqreturn_t link_pm_irq_handler(int irq, void *data)
{
	int value;
	struct link_pm_data *pm_data = data;

	if (!pm_data->link_pm_active)
		return IRQ_HANDLED;

	/* host wake up HIGH */
	/*
		resume usb runtime pm start
	*/
	/* host wake up LOW */
	/*
		slave usb enumeration end,
		host can send usb packet after
		runtime pm status changes to ACTIVE
	*/
	value = gpio_get_value(pm_data->gpio_link_hostwake);
	pr_err("[MIF] gpio [HWK] get [%d]\n", value);
	/*
	* igonore host wakeup interrupt at suspending kernel
	*/
	if (pm_data->dpm_suspending) {
		pr_err("[MIF] ignore request by suspending\n");
		return IRQ_HANDLED;
	}

	if (value == HOSTWAKE_TRIGLEVEL) {
		/* move to slave wake function */
		/* runtime pm goes to active */
		/*
		if (gpio_get_value(pm_data->gpio_link_active)) {
			pr_err("[MIF] gpio [H ACTV : %d] set 1\n",
				gpio_get_value(pm_data->gpio_link_active));
			gpio_set_value(pm_data->gpio_link_active, 1);
		}
		*/
		queue_delayed_work(pm_data->wq, &pm_data->link_pm_work, 0);
	} else {
		/* notification of enumeration process from slave device
		 * But it does not mean whole connection is in resume, so do not
		 * notify resume completion here.

		if (pm_data->link_pm_active && !pm_data->active_done.done)
			complete(&pm_data->active_done);
		*/
		/* clear slave cpu wake up pin */
		gpio_set_value(pm_data->gpio_link_slavewake, 0);
		pr_info("[MIF] gpio [SWK] set [0]\n");
	}
	return IRQ_HANDLED;
}

static long link_pm_ioctl(struct file *file, unsigned int cmd,
						unsigned long arg)
{
	int value;
	struct link_pm_data *pm_data = file->private_data;

	pr_info("%s:%x\n", __func__, cmd);

	switch (cmd) {
	case IOCTL_LINK_CONTROL_ENABLE:
		if (copy_from_user(&value, (const void __user *)arg,
							sizeof(int)))
			return -EFAULT;
		if (pm_data->link_ldo_enable)
			pm_data->link_ldo_enable(!!value);
		if (pm_data->gpio_link_enable)
			gpio_set_value(pm_data->gpio_link_enable, value);
		break;
	case IOCTL_LINK_CONTROL_ACTIVE:
		if (copy_from_user(&value, (const void __user *)arg,
							sizeof(int)))
			return -EFAULT;
		gpio_set_value(pm_data->gpio_link_active, value);
		break;
	case IOCTL_LINK_GET_HOSTWAKE:
		return !gpio_get_value(pm_data->gpio_link_hostwake);
	case IOCTL_LINK_CONNECTED:
		return pm_data->usb_ld->if_usb_connected;
	case IOCTL_LINK_SET_BIAS_CLEAR:
		if (copy_from_user(&value, (const void __user *)arg,
							sizeof(int)))
			return -EFAULT;
		if (value) {
			gpio_direction_output(pm_data->gpio_link_slavewake, 0);
			gpio_direction_output(pm_data->gpio_link_hostwake, 0);
		} else {
			gpio_direction_output(pm_data->gpio_link_slavewake, 0);
			gpio_direction_input(pm_data->gpio_link_hostwake);
			irq_set_irq_type(pm_data->irq_link_hostwake,
				IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING);
		}
	default:
		break;
	}

	return 0;
}

static int link_pm_open(struct inode *inode, struct file *file)
{
	struct link_pm_data *pm_data =
		(struct link_pm_data *)file->private_data;
	file->private_data = (void *)pm_data;
	return 0;
}

static int link_pm_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static const struct file_operations link_pm_fops = {
	.owner = THIS_MODULE,
	.open = link_pm_open,
	.release = link_pm_release,
	.unlocked_ioctl = link_pm_ioctl,
};

static int link_pm_notifier_event(struct notifier_block *this,
				unsigned long event, void *ptr)
{
	struct link_pm_data *pm_data =
			container_of(this, struct link_pm_data,	pm_notifier);

	switch (event) {
	case PM_SUSPEND_PREPARE:
		pm_data->dpm_suspending = true;
		pr_info("%s : dpm suspending set to true\n", __func__);
		return NOTIFY_OK;
	case PM_POST_SUSPEND:
		pm_data->dpm_suspending = false;
		pr_info("%s : dpm suspending set to false\n", __func__);
		return NOTIFY_OK;
	}
	return NOTIFY_DONE;
}

static struct modem_ctl *if_usb_get_modemctl(struct link_pm_data *pm_data)
{
	struct io_device *iod;

	io_devs_for_each(iod, &pm_data->usb_ld->ld) {
		if (iod->format == IPC_FMT)
			break;
	}
	if (!iod) {
		pr_err("%s : no iodevice for modem control\n", __func__);
		return NULL;
	}

	return iod->mc;
}

static int if_usb_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct if_usb_devdata *devdata = usb_get_intfdata(intf);
	struct link_pm_data *pm_data = devdata->usb_ld->link_pm_data;
	if (!devdata->disconnected && devdata->state == STATE_RESUMED) {
		usb_kill_urb(devdata->urb);
		devdata->state = STATE_SUSPENDED;
	}

	devdata->usb_ld->suspended++;

	if (devdata->usb_ld->suspended == LINKPM_DEV_NUM) {
		pr_info("[if_usb_suspended]\n");
		wake_unlock(&pm_data->l2_wake);
	}
	return 0;
}

static int if_usb_resume(struct usb_interface *intf)
{
	int ret;
	struct if_usb_devdata *devdata = usb_get_intfdata(intf);
	struct link_pm_data *pm_data = devdata->usb_ld->link_pm_data;

	if (!devdata->disconnected && devdata->state == STATE_SUSPENDED) {
		ret = usb_rx_submit(devdata->usb_ld, devdata, GFP_ATOMIC);
		if (ret < 0) {
			pr_err("%s: usb_rx_submit error with (%d)\n",
					__func__, ret);
			return ret;
		}
		devdata->state = STATE_RESUMED;
	}

	devdata->usb_ld->suspended--;
	if (!devdata->usb_ld->suspended) {
		pr_info("[if_usb_resumed]\n");
		wake_lock(&pm_data->l2_wake);
	}

	return 0;
}

static int if_usb_reset_resume(struct usb_interface *intf)
{
	int ret;
	struct if_usb_devdata *devdata = usb_get_intfdata(intf);
	struct link_pm_data *pm_data = devdata->usb_ld->link_pm_data;

	ret = if_usb_resume(intf);
	/*
	 * for runtime suspend, kick runtime pm at L3 -> L0 reset resume
	*/
	if (!devdata->usb_ld->suspended)
		queue_delayed_work(pm_data->wq, &pm_data->link_pm_start, 0);
	return ret;
}

static void if_usb_disconnect(struct usb_interface *intf)
{
	struct if_usb_devdata *devdata = usb_get_intfdata(intf);
	struct link_pm_data *pm_data = devdata->usb_ld->link_pm_data;
	struct device *dev, *ppdev;

	pr_info("%s\n", __func__);

	if (devdata->disconnected)
		return;

	usb_driver_release_interface(to_usb_driver(intf->dev.driver), intf);

	usb_kill_urb(devdata->urb);

	dev = &devdata->usb_ld->usbdev->dev;
	ppdev = dev->parent->parent;
	pm_runtime_forbid(ppdev); /*ehci*/

	pr_info("%s put dev 0x%p\n", __func__, devdata->usbdev);
	usb_put_dev(devdata->usbdev);

	devdata->data_intf = NULL;
	/* if possible, merge below 2 variables */
	devdata->disconnected = 1;
	devdata->state = STATE_SUSPENDED;

	devdata->usb_ld->if_usb_connected = 0;
	devdata->usb_ld->suspended = 0;
	wake_lock(&pm_data->boot_wake);

	usb_set_intfdata(intf, NULL);

	/* cancel runtime start delayed works */
	cancel_delayed_work_sync(&pm_data->link_pm_start);

	/* if reconnect function exist , try reconnect without reset modem
	 * reconnect function checks modem is under crash or not, so we don't
	 * need check crash state here. reconnect work checks and determine
	 * further works
	 */
	if (!pm_data->link_reconnect)
		return;

	if (devdata->usb_ld->ld.com_state != COM_ONLINE) {
		cancel_delayed_work(&pm_data->link_reconnect_work);
		return;
	} else
		schedule_delayed_work(&pm_data->link_reconnect_work,
							msecs_to_jiffies(200));
	return;
}

static int if_usb_set_pipe(struct usb_link_device *usb_ld,
			const struct usb_host_interface *desc, int pipe)
{
	if (pipe < 0 || pipe >= IF_USB_DEVNUM_MAX) {
		pr_err("%s:undefined endpoint, exceed max\n", __func__);
		return -EINVAL;
	}

	pr_err("%s: set %d\n", __func__, pipe);

	if ((usb_pipein(desc->endpoint[0].desc.bEndpointAddress)) &&
	    (usb_pipeout(desc->endpoint[1].desc.bEndpointAddress))) {
		usb_ld->devdata[pipe].rx_pipe = usb_rcvbulkpipe(usb_ld->usbdev,
				desc->endpoint[0].desc.bEndpointAddress);
		usb_ld->devdata[pipe].tx_pipe = usb_sndbulkpipe(usb_ld->usbdev,
				desc->endpoint[1].desc.bEndpointAddress);
	} else if ((usb_pipeout(desc->endpoint[0].desc.bEndpointAddress)) &&
		   (usb_pipein(desc->endpoint[1].desc.bEndpointAddress))) {
		usb_ld->devdata[pipe].rx_pipe = usb_rcvbulkpipe(usb_ld->usbdev,
				desc->endpoint[1].desc.bEndpointAddress);
		usb_ld->devdata[pipe].tx_pipe = usb_sndbulkpipe(usb_ld->usbdev,
				desc->endpoint[0].desc.bEndpointAddress);
	} else {
		pr_err("%s:undefined endpoint\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int __devinit if_usb_probe(struct usb_interface *intf,
					const struct usb_device_id *id)
{
	int err;
	int pipe;
	const struct usb_cdc_union_desc *union_hdr;
	const struct usb_host_interface *data_desc;
	unsigned char *buf = intf->altsetting->extra;
	int buflen = intf->altsetting->extralen;
	struct usb_interface *data_intf;
	struct usb_device *usbdev = interface_to_usbdev(intf);
	struct usb_driver *usbdrv = to_usb_driver(intf->dev.driver);
	struct usb_id_info *info = (struct usb_id_info *)id->driver_info;
	struct usb_link_device *usb_ld = info->usb_ld;

	pr_err("%s:  usbdev = 0x%p\n", __func__, usbdev);

	usb_ld->usbdev = usbdev;
	pm_runtime_forbid(&usbdev->dev);
	usb_ld->link_pm_data->link_pm_active = false;

	union_hdr = NULL;
	/* for WMC-ACM compatibility, WMC-ACM use an end-point for control msg*/
	if (intf->altsetting->desc.bInterfaceSubClass != USB_CDC_SUBCLASS_ACM) {
		pr_err("%s:ignore Non ACM end-point\n", __func__);
		return -EINVAL;
	}

	if (!buflen) {
		if (intf->cur_altsetting->endpoint->extralen &&
				    intf->cur_altsetting->endpoint->extra) {
			buflen = intf->cur_altsetting->endpoint->extralen;
			buf = intf->cur_altsetting->endpoint->extra;
		} else {
			pr_err("%s:Zero len descriptor reference\n", __func__);
			return -EINVAL;
		}
	}

	while (buflen > 0) {
		if (buf[1] == USB_DT_CS_INTERFACE) {
			switch (buf[2]) {
			case USB_CDC_UNION_TYPE:
				if (union_hdr)
					break;
				union_hdr = (struct usb_cdc_union_desc *)buf;
				break;
			default:
				break;
			}
		}
		buf += buf[0];
		buflen -= buf[0];
	}

	if (!union_hdr) {
		pr_err("%s:USB CDC is not union type\n", __func__);
		return -EINVAL;
	}

	data_intf = usb_ifnum_to_if(usbdev, union_hdr->bSlaveInterface0);
	if (!data_intf) {
		pr_err("%s:data_inferface is NULL\n", __func__);
		return -ENODEV;
	}

	data_desc = data_intf->altsetting;
	if (!data_desc) {
		pr_err("%s:data_desc is NULL\n", __func__);
		return -ENODEV;
	}

	switch (info->intf_id) {
	case BOOT_DOWN:
		pipe = IF_USB_BOOT_EP;
		usb_ld->ld.com_state = COM_BOOT;
		break;
	case IPC_CHANNEL:
		pipe = intf->altsetting->desc.bInterfaceNumber / 2;
		usb_ld->ld.com_state = COM_ONLINE;
		break;
	default:
		pipe = -1;
		break;
	}

	if (if_usb_set_pipe(usb_ld, data_desc, pipe) < 0)
		return -EINVAL;

	usb_ld->devdata[pipe].usbdev = usb_get_dev(usbdev);
	pr_err("%s:  devdata usbdev = 0x%p\n", __func__,
		usb_ld->devdata[pipe].usbdev);
	usb_ld->devdata[pipe].usb_ld = usb_ld;
	usb_ld->devdata[pipe].data_intf = data_intf;
	usb_ld->devdata[pipe].format = pipe;
	usb_ld->devdata[pipe].disconnected = 0;
	usb_ld->devdata[pipe].state = STATE_RESUMED;

	usb_ld->if_usb_connected = 1;
	usb_ld->suspended = 0;

	err = usb_driver_claim_interface(usbdrv, data_intf,
		(void *)&usb_ld->devdata[pipe]);
	if (err < 0) {
		pr_err("%s:usb_driver_claim() failed\n", __func__);
		return err;
	}

	pm_suspend_ignore_children(&usbdev->dev, true);

	usb_set_intfdata(intf, (void *)&usb_ld->devdata[pipe]);

	/* rx start for this endpoint */
	usb_rx_submit(usb_ld, &usb_ld->devdata[pipe], GFP_KERNEL);

	if (info->intf_id == IPC_CHANNEL &&
		!(work_pending(&usb_ld->link_pm_data->link_pm_start.work))) {
		queue_delayed_work(usb_ld->link_pm_data->wq,
				&usb_ld->link_pm_data->link_pm_start,
				msecs_to_jiffies(10000));
		wake_lock(&usb_ld->link_pm_data->l2_wake);
		wake_unlock(&usb_ld->link_pm_data->boot_wake);
	}

	pr_info("[USB-IPC] %s successfully done\n", __func__);

	return 0;
}

static void if_usb_free_pipe_data(struct usb_link_device *usb_ld)
{
	int i;
	for (i = 0; i < IF_USB_DEVNUM_MAX; i++) {
		kfree(usb_ld->devdata[i].rx_buf);
		usb_kill_urb(usb_ld->devdata[i].urb);
	}
}

static struct usb_id_info hsic_boot_down_info = {
	.intf_id = BOOT_DOWN,
};
static struct usb_id_info hsic_channel_info = {
	.intf_id = IPC_CHANNEL,
};

static struct usb_device_id if_usb_ids[] = {
	{USB_DEVICE(IMC_BOOT_VID, IMC_BOOT_PID),
	.driver_info = (unsigned long)&hsic_boot_down_info,},
	{USB_DEVICE(IMC_MAIN_VID, IMC_MAIN_PID),
	.driver_info = (unsigned long)&hsic_channel_info,},
	{USB_DEVICE(STE_BOOT_VID, STE_BOOT_PID),
	.driver_info = (unsigned long)&hsic_boot_down_info,},
	{USB_DEVICE(STE_MAIN_VID, STE_MAIN_PID),
	.driver_info = (unsigned long)&hsic_channel_info,},
	{}
};
MODULE_DEVICE_TABLE(usb, if_usb_ids);

static struct usb_driver if_usb_driver = {
	.name =		"cdc_modem",
	.probe =		if_usb_probe,
	.disconnect =	if_usb_disconnect,
	.id_table =	if_usb_ids,
	.suspend =	if_usb_suspend,
	.resume =	if_usb_resume,
	.reset_resume =	if_usb_reset_resume,
	.supports_autosuspend = 1,
};

static int if_usb_init(struct link_device *ld)
{
	int ret;
	int i;
	struct usb_link_device *usb_ld = to_usb_link_device(ld);
	struct if_usb_devdata *pipe_data;
	struct usb_id_info *id_info;

	/* to connect usb link device with usb interface driver */
	for (i = 0; i < ARRAY_SIZE(if_usb_ids); i++) {
		id_info = (struct usb_id_info *)if_usb_ids[i].driver_info;
		if (id_info)
			id_info->usb_ld = usb_ld;
	}

	ret = usb_register(&if_usb_driver);
	if (ret) {
		pr_err("[USB-IPC] usb_register_driver() fail : %d\n", ret);
		return ret;
	}

	/* allocate rx buffer for usb receive */
	for (i = 0; i < IF_USB_DEVNUM_MAX; i++) {
		pipe_data = &usb_ld->devdata[i];
		pipe_data->format = i;
		pipe_data->rx_buf_size = 16 * 1024;

		pipe_data->rx_buf = kmalloc(pipe_data->rx_buf_size,
						GFP_DMA | GFP_KERNEL);
		if (!pipe_data->rx_buf) {
			if_usb_free_pipe_data(usb_ld);
			ret = -ENOMEM;
			break;
		}

		pipe_data->urb = usb_alloc_urb(0, GFP_KERNEL);
		if (!pipe_data->urb) {
			pr_err("%s: alloc urb fail\n", __func__);
			if_usb_free_pipe_data(usb_ld);
			return -ENOMEM;
		}
	}

	pr_info("[USB-IPC] if_usb_init() done : %d, usb_ld (0x%p)\n",
								ret, usb_ld);
	return ret;
}

static int usb_link_pm_init(struct usb_link_device *usb_ld, void *data)
{
	int r;
	struct platform_device *pdev = (struct platform_device *)data;
	struct modem_data *pdata =
			(struct modem_data *)pdev->dev.platform_data;
	struct modemlink_pm_data *pm_pdata = pdata->link_pm_data;
	struct link_pm_data *pm_data =
			kzalloc(sizeof(struct link_pm_data), GFP_KERNEL);
	if (!pm_data) {
		pr_err("%s: link_pm_data is NULL\n", __func__);
		return -ENOMEM;
	}
	/* get link pm data from modemcontrol's platform data */
	pm_data->gpio_link_active = pm_pdata->gpio_link_active;
	pm_data->gpio_link_enable = pm_pdata->gpio_link_enable;
	pm_data->gpio_link_hostwake = pm_pdata->gpio_link_hostwake;
	pm_data->gpio_link_slavewake = pm_pdata->gpio_link_slavewake;
	pm_data->irq_link_hostwake = gpio_to_irq(pm_data->gpio_link_hostwake);
	pm_data->link_ldo_enable = pm_pdata->link_ldo_enable;
	pm_data->link_reconnect = pm_pdata->link_reconnect;

	pm_data->usb_ld = usb_ld;
	pm_data->link_pm_active = false;
	usb_ld->link_pm_data = pm_data;

	pm_data->miscdev.minor = MISC_DYNAMIC_MINOR;
	pm_data->miscdev.name = "link_pm";
	pm_data->miscdev.fops = &link_pm_fops;

	r = misc_register(&pm_data->miscdev);
	if (r < 0) {
		pr_err("%s:fail to register pm device(%d)\n", __func__, r);
		goto err_misc_register;
	}

	r = request_irq(pm_data->irq_link_hostwake, link_pm_irq_handler,
		IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
		"hostwake", (void *)pm_data);
	if (r) {
		pr_err("%s:fail to request irq(%d)\n", __func__, r);
		goto err_request_irq;
	}

	r = enable_irq_wake(pm_data->irq_link_hostwake);
	if (r) {
		pr_err("%s: failed to enable_irq_wake:%d\n", __func__, r);
		goto err_set_wake_irq;
	}

	/* create work queue & init work for runtime pm */
	pm_data->wq = create_singlethread_workqueue("linkpmd");
	if (!pm_data->wq) {
		pr_err("%s:fail to create wq\n", __func__);
		goto err_create_wq;
	}

	pm_data->pm_notifier.notifier_call = link_pm_notifier_event;
	register_pm_notifier(&pm_data->pm_notifier);

	init_completion(&pm_data->active_done);
	INIT_DELAYED_WORK(&pm_data->link_pm_work, link_pm_runtime_work);
	INIT_DELAYED_WORK(&pm_data->link_pm_start, link_pm_runtime_start);
	INIT_DELAYED_WORK(&pm_data->link_reconnect_work,
						link_pm_reconnect_work);
	wake_lock_init(&pm_data->l2_wake, WAKE_LOCK_SUSPEND, "l2_hsic");
	wake_lock_init(&pm_data->boot_wake, WAKE_LOCK_SUSPEND, "boot_hsic");
	wake_lock_init(&pm_data->rpm_wake, WAKE_LOCK_SUSPEND, "rpm_hsic");

	return 0;

err_create_wq:
	disable_irq_wake(pm_data->irq_link_hostwake);
err_set_wake_irq:
	free_irq(pm_data->irq_link_hostwake, (void *)pm_data);
err_request_irq:
	misc_deregister(&pm_data->miscdev);
err_misc_register:
	kfree(pm_data);
	return r;
}

struct link_device *hsic_create_link_device(void *data)
{
	int ret;
	struct usb_link_device *usb_ld;
	struct link_device *ld;

	usb_ld = kzalloc(sizeof(struct usb_link_device), GFP_KERNEL);
	if (!usb_ld)
		return NULL;

	INIT_LIST_HEAD(&usb_ld->ld.list);
	skb_queue_head_init(&usb_ld->ld.sk_fmt_tx_q);
	skb_queue_head_init(&usb_ld->ld.sk_raw_tx_q);

	ld = &usb_ld->ld;

	ld->name = "usb";
	ld->init_comm = usb_init_communication;
	ld->terminate_comm = usb_terminate_communication;
	ld->send = usb_send;
	ld->com_state = COM_NONE;
	ld->raw_tx_suspended = false;
	init_completion(&ld->raw_tx_resumed_by_cp);

	ld->tx_wq = create_singlethread_workqueue("usb_tx_wq");
	if (!ld->tx_wq) {
		pr_err("[USB-IPC] fail to create work Q.\n");
		goto err;
	}

	INIT_DELAYED_WORK(&ld->tx_delayed_work, usb_tx_work);

	/* create link pm device */
	ret = usb_link_pm_init(usb_ld, data);
	if (ret)
		goto err;

	ret = if_usb_init(ld);
	if (ret)
		goto err;

	pr_info("[MODEM_IF] %s : create_link_device DONE\n", usb_ld->ld.name);
	return (void *)ld;
err:
	kfree(usb_ld);
	return NULL;
}

static void __exit if_usb_exit(void)
{
	usb_deregister(&if_usb_driver);
}
