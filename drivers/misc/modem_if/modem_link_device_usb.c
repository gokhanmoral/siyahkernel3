/*
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

#define DEBUG

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <linux/gpio.h>
#include <linux/if_arp.h>
#include <linux/platform_device.h>
#include <linux/suspend.h>

#include <linux/platform_data/modem.h>
#include "modem_prj.h"
#include "modem_link_device_usb.h"
#include "modem_utils.h"

#define URB_COUNT	4

static irqreturn_t usb_resume_irq(int irq, void *data);
static int usb_tx_urb_with_skb(struct usb_link_device *usb_ld,
		struct sk_buff *skb, struct if_usb_devdata *pipe_data);

static void
usb_free_urbs(struct usb_link_device *usb_ld, struct if_usb_devdata *pipe)
{
	struct usb_device *usbdev = usb_ld->usbdev;
	struct urb *urb;

	while ((urb = usb_get_from_anchor(&pipe->urbs))) {
		usb_poison_urb(urb);
		usb_free_coherent(usbdev, pipe->rx_buf_size,
				urb->transfer_buffer, urb->transfer_dma);
		urb->transfer_buffer = NULL;
		usb_put_urb(urb);
		usb_free_urb(urb);
	}
}

static int start_ipc(struct link_device *ld)
{
	struct sk_buff *skb;
	char data[1] = {'a'};
	int err;
	struct usb_link_device *usb_ld = to_usb_link_device(ld);
	struct if_usb_devdata *pipe_data = &usb_ld->devdata[IF_USB_FMT_EP];

	if (usb_ld->link_pm_data->hub_handshake_done) {
		lnk_err(usb_ld, "Aleady send start ipc, skip start ipc\n");
		err = 0;
		goto exit;
	}

	if (!usb_ld->if_usb_connected) {
		lnk_err(usb_ld, "HSIC/USB not connected, skip start ipc\n");
		err = -ENODEV;
		goto exit;
	}

	if (usb_ld->if_usb_initstates == INIT_IPC_START_DONE) {
		lnk_dbg(usb_ld, "aleady IPC started\n");
		err = 0;
		goto exit;
	}

	lnk_info(usb_ld, "send 'a'\n");

	skb = alloc_skb(16, GFP_ATOMIC);
	if (unlikely(!skb))
		return -ENOMEM;
	memcpy(skb_put(skb, 1), data, 1);

	err = usb_tx_urb_with_skb(usb_ld, skb, pipe_data);
	if (err < 0) {
		lnk_err(usb_ld, "usb_tx_urb fail\n");
		goto exit;
	}
	usb_ld->link_pm_data->hub_handshake_done = true;
	usb_ld->if_usb_initstates = INIT_IPC_START_DONE;
exit:
	return err;
}

static int usb_init_communication(struct link_device *ld,
			struct io_device *iod)
{
	int err = 0;
	switch (iod->format) {
	case IPC_BOOT:
		ld->com_state = COM_BOOT;
		skb_queue_purge(&ld->sk_fmt_tx_q);
		break;

	case IPC_RAMDUMP:
		ld->com_state = COM_CRASH;
		break;

	case IPC_FMT:
		err = start_ipc(ld);
		break;

	case IPC_RFS:
	case IPC_RAW:

	default:
		ld->com_state = COM_ONLINE;
		break;
	}

	iod_dbg(iod, "com_state = %d\n", ld->com_state);
	return err;
}

static void usb_terminate_communication(
			struct link_device *ld, struct io_device *iod)
{
	iod_dbg(iod, "com_state = %d\n", ld->com_state);
}

static int usb_rx_submit(struct if_usb_devdata *pipe, struct urb *urb,
	gfp_t gfp_flags)
{
	int ret;

	usb_anchor_urb(urb, &pipe->reading);
	ret = usb_submit_urb(urb, gfp_flags);
	if (ret) {
		usb_unanchor_urb(urb);
		usb_anchor_urb(urb, &pipe->urbs);
		pr_err("%s: submit urb fail with ret (%d)\n", __func__, ret);
	}

	usb_mark_last_busy(urb->dev);
	return ret;
}

static void usb_rx_complete(struct urb *urb)
{
	struct if_usb_devdata *pipe_data = urb->context;
	struct usb_link_device *usb_ld = usb_get_intfdata(pipe_data->data_intf);
	struct io_device *iod;
	int iod_format = IPC_FMT;
	int ret;

	switch (urb->status) {
	case 0:
	case -ENOENT:
		if (!urb->actual_length)
			goto re_submit;
		/* call iod recv */
		/* how we can distinguish boot ch with fmt ch ?? */
		switch (pipe_data->format) {
		case IF_USB_FMT_EP:
			iod_format = IPC_FMT;
			pr_buffer("rx", (char *)urb->transfer_buffer,
					(size_t)urb->actual_length, 16);
			break;
		case IF_USB_RAW_EP:
			iod_format = IPC_MULTI_RAW;
			break;
		case IF_USB_RFS_EP:
			iod_format = IPC_RFS;
			break;
		default:
			break;
		}

		io_devs_for_each(iod, &usb_ld->ld) {
			/* during boot stage fmt end point */
			/* shared with boot io device */
			/* when we use fmt device only, at boot and ipc exchange
				it can be reduced to 1 device */
			if (iod_format == IPC_FMT &&
				usb_ld->ld.com_state == COM_BOOT)
				iod_format = IPC_BOOT;
			if (iod_format == IPC_FMT &&
				usb_ld->ld.com_state == COM_CRASH)
				iod_format = IPC_RAMDUMP;

			if (iod->format == iod_format) {
				ret = iod->recv(iod,
						&usb_ld->ld,
						(char *)urb->transfer_buffer,
						urb->actual_length);
				if (ret < 0)
					lnk_err(usb_ld,
						"io device recv error :%d\n",
						ret);
				break;
			}
		}
re_submit:
		if (urb->status || atomic_read(&usb_ld->suspend_count))
			break;
		usb_rx_submit(pipe_data, urb, GFP_ATOMIC);
		return;
	case -ESHUTDOWN:
	case -EPROTO:
		break;
	case -EOVERFLOW:
		lnk_err(usb_ld, "RX overflow\n");
		break;
	default:
		lnk_err(usb_ld, "RX complete Status (%d)\n", urb->status);
		break;
	}

	usb_anchor_urb(urb, &pipe_data->urbs);
}

static int usb_send(struct link_device *ld, struct io_device *iod,
			struct sk_buff *skb)
{
	struct sk_buff_head *txq;
	size_t tx_size;

	if (iod->format == IPC_RAW)
		txq = &ld->sk_raw_tx_q;
	else
		txq = &ld->sk_fmt_tx_q;

	/* store the tx size before run the tx_delayed_work*/
	tx_size = skb->len;

	/* save io device into cb area */
	skbpriv(skb)->iod = iod;
	/* en queue skb data */
	skb_queue_tail(txq, skb);

	queue_delayed_work(ld->tx_wq, &ld->tx_delayed_work, 0);

	return tx_size;
}

static void usb_tx_complete(struct urb *urb)
{
	int ret = 0;
	struct sk_buff *skb = urb->context;

	switch (urb->status) {
	case 0:
		break;
	default:
		pr_err("%s:TX error (%d)\n", __func__, urb->status);
	}

	usb_mark_last_busy(urb->dev);
	ret = pm_runtime_put_autosuspend(&urb->dev->dev);
	if (ret < 0)
		pr_debug("%s pm_runtime_put_autosuspend failed : ret(%d)\n",
			__func__, ret);
	usb_free_urb(urb);
	dev_kfree_skb_any(skb);
}

static void if_usb_force_disconnect(struct work_struct *work)
{
	struct usb_link_device *usb_ld =
		container_of(work, struct usb_link_device, disconnect_work);
	struct usb_device *udev = usb_ld->usbdev;

	pm_runtime_get_sync(&udev->dev);
	if (udev->state != USB_STATE_NOTATTACHED) {
		/* TODO: check below symbol from proxima...
		usb_force_disconnect(udev); */
		pr_info("force disconnect by modem not responding!!\n");
	}
	pm_runtime_put_autosuspend(&udev->dev);
}

static void
usb_change_modem_state(struct usb_link_device *usb_ld, enum modem_state state)
{
	struct io_device *iod;

	io_devs_for_each(iod, &usb_ld->ld) {
		if (iod->format == IPC_FMT) {
			iod->modem_state_changed(iod, state);
			return;
		}
	}
}

static int usb_tx_urb_with_skb(struct usb_link_device *usb_ld,
		struct sk_buff *skb, struct if_usb_devdata *pipe_data)
{
	int ret, cnt = 0;
	struct urb *urb;
	struct usb_device *usbdev = usb_ld->usbdev;
	unsigned long flags;

	if (!usbdev || (usbdev->state == USB_STATE_NOTATTACHED) ||
			usb_ld->host_wake_timeout_flag)
		return -ENODEV;

	pm_runtime_get_noresume(&usbdev->dev);

	if (usbdev->dev.power.runtime_status == RPM_SUSPENDED ||
		usbdev->dev.power.runtime_status == RPM_SUSPENDING) {
		usb_ld->resume_status = AP_INITIATED_RESUME;
		SET_SLAVE_WAKEUP(usb_ld->pdata, 1);

		while (!wait_event_interruptible_timeout(usb_ld->l2_wait,
				usbdev->dev.power.runtime_status == RPM_ACTIVE
				|| pipe_data->disconnected,
				HOST_WAKEUP_TIMEOUT_JIFFIES)) {

			if (cnt == MAX_RETRY) {
				pr_err("host wakeup timeout !!\n");
				SET_SLAVE_WAKEUP(usb_ld->pdata, 0);
				pm_runtime_put_autosuspend(&usbdev->dev);
				schedule_work(&usb_ld->disconnect_work);
				usb_ld->host_wake_timeout_flag = 1;
				return -1;
			}
			pr_err("host wakeup timeout ! retry..\n");
			SET_SLAVE_WAKEUP(usb_ld->pdata, 0);
			udelay(100);
			SET_SLAVE_WAKEUP(usb_ld->pdata, 1);
			cnt++;
		}

		if (pipe_data->disconnected) {
			SET_SLAVE_WAKEUP(usb_ld->pdata, 0);
			pm_runtime_put_autosuspend(&usbdev->dev);
			return -ENODEV;
		}

		pr_debug("wait_q done (runtime_status=%d)\n",
				usbdev->dev.power.runtime_status);
	}

	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) {
		pr_err("%s alloc urb error\n", __func__);
		if (pm_runtime_put_autosuspend(&usbdev->dev) < 0)
			pr_debug("pm_runtime_put_autosuspend fail\n");
		return -ENOMEM;
	}

	urb->transfer_flags = URB_ZERO_PACKET;
	usb_fill_bulk_urb(urb, usbdev, pipe_data->tx_pipe, skb->data,
			skb->len, usb_tx_complete, (void *)skb);

	spin_lock_irqsave(&usb_ld->lock, flags);
	if (atomic_read(&usb_ld->suspend_count)) {
		/* transmission will be done in resume */
		usb_anchor_urb(urb, &usb_ld->deferred);
		usb_put_urb(urb);
		pr_debug("%s: anchor urb (0x%p)\n", __func__, urb);
		spin_unlock_irqrestore(&usb_ld->lock, flags);
		return 0;
	}
	spin_unlock_irqrestore(&usb_ld->lock, flags);

	ret = usb_submit_urb(urb, GFP_KERNEL);
	if (ret < 0) {
		pr_err("%s usb_submit_urb with ret(%d)\n", __func__, ret);
		if (pm_runtime_put_autosuspend(&usbdev->dev) < 0)
			pr_debug("pm_runtime_put_autosuspend fail\n");
	}
	return ret;
}

static void link_pm_hub_work(struct work_struct *work)
{
	int err;
	struct link_pm_data *pm_data =
		container_of(work, struct link_pm_data, link_pm_hub.work);
	struct device *rhub = pm_data->root_hub;/* fix runtime pm get/put pair*/

	if (pm_data->hub_status == HUB_STATE_ACTIVE)
		return;

	if (!pm_data->port_enable) {
		pr_err("mif: hub power func not assinged\n");
		return;
	}
	wake_lock(&pm_data->hub_lock);

	/* If kernel if suspend, wait the ehci resume */
	if (pm_data->dpm_suspending) {
		pr_info("%s: dpm_suspending\n", __func__);
		schedule_delayed_work(&pm_data->link_pm_hub,
						msecs_to_jiffies(500));
		goto exit;
	}

	switch (pm_data->hub_status) {
	case HUB_STATE_OFF:
		pm_data->hub_status = HUB_STATE_RESUMMING;
		mif_trace("hub off->on\n");

		/* skip 1st time before first probe */
		if (rhub && pm_data->root_hub)
			pm_runtime_get_sync(pm_data->root_hub);
		err = pm_data->port_enable(2, 1);
		if (err < 0) {
			lnk_err(pm_data->usb_ld,
					"hub on fail err=%d\n", err);
			err = pm_data->port_enable(2, 0);
			if (err < 0)
				lnk_err(pm_data->usb_ld,
					"hub off fail err=%d\n", err);
			pm_data->hub_status = HUB_STATE_OFF;
			if (rhub && pm_data->root_hub)
				pm_runtime_put_sync(pm_data->root_hub);
			goto exit;
		}
		/* resume root hub */
		schedule_delayed_work(&pm_data->link_pm_hub,
						msecs_to_jiffies(100));
		break;
	case HUB_STATE_RESUMMING:
		if (pm_data->hub_on_retry_cnt++ > 50) {
			pm_data->hub_on_retry_cnt = 0;
			pm_data->hub_status = HUB_STATE_OFF;
			if (rhub && pm_data->root_hub)
				pm_runtime_put_sync(pm_data->root_hub);
		}
		mif_trace("hub resumming\n");
		schedule_delayed_work(&pm_data->link_pm_hub,
						msecs_to_jiffies(200));
		break;
	case HUB_STATE_PREACTIVE:
		mif_trace("hub active\n");
		pm_data->hub_on_retry_cnt = 0;
		wake_unlock(&pm_data->hub_lock);
		pm_data->hub_status = HUB_STATE_ACTIVE;
		complete(&pm_data->hub_active);
		if (rhub && pm_data->root_hub)
			pm_runtime_put_sync(pm_data->root_hub);
		break;
	}
exit:
	return;
}


static int link_pm_hub_standby(struct link_pm_data *pm_data)
{
	struct usb_link_device *usb_ld = pm_data->usb_ld;
	int err = 0;

	lnk_info(usb_ld, "wait hub standby\n");

	if (!pm_data->port_enable) {
		lnk_err(usb_ld, "hub power func not assinged\n");
		return -ENODEV;
	}

	err = pm_data->port_enable(2, 0);
	if (err < 0)
		lnk_err(pm_data->usb_ld, "hub off fail err=%d\n", err);

	pm_data->hub_status = HUB_STATE_OFF;
	return err;
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

	/*TODO: check the PHONE ACTIVE STATES */
	/* because tx data wait until hub on with wait_for_complettion, it
	 should queue to single_threaded work queue */
	if (pm_data->hub_status != HUB_STATE_ACTIVE) {
		INIT_COMPLETION(pm_data->hub_active);
		SET_SLAVE_WAKEUP(usb_ld->pdata, 1);
		ret = wait_for_completion_timeout(&pm_data->hub_active,
			msecs_to_jiffies(2000));
		if (!ret) { /*timeout*/
			pr_err("%s: hub on timeout - retry\n", __func__);
			SET_SLAVE_WAKEUP(usb_ld->pdata, 0);
			queue_delayed_work(ld->tx_wq, &ld->tx_delayed_work, 0);
			return;
		}
	}

	while (ld->sk_fmt_tx_q.qlen || ld->sk_raw_tx_q.qlen) {
		/* send skb from fmt_txq and raw_txq,
		 * one by one for fair flow control */
		skb = skb_dequeue(&ld->sk_fmt_tx_q);
		if (skb) {
			iod = skbpriv(skb)->iod;
			switch (iod->format) {
			case IPC_BOOT:
			case IPC_RAMDUMP:
			case IPC_FMT:
				/* boot device uses same intf with fmt*/
				pipe_data = &usb_ld->devdata[IF_USB_FMT_EP];
				break;
			case IPC_RFS:
				pipe_data = &usb_ld->devdata[IF_USB_RFS_EP];
				break;
			default:
				/* wrong packet for fmt tx q , drop it */
				dev_kfree_skb_any(skb);
				continue;
			}

			ret = usb_tx_urb_with_skb(usb_ld, skb, pipe_data);
			if (ret < 0) {
				iod_err(iod, "usb_tx_urb_with_skb, ret(%d)\n",
					ret);
				skb_queue_head(&ld->sk_fmt_tx_q, skb);
				return;
			}
		}

		skb = skb_dequeue(&ld->sk_raw_tx_q);
		if (skb) {
			pipe_data = &usb_ld->devdata[IF_USB_RAW_EP];
			ret = usb_tx_urb_with_skb(usb_ld, skb, pipe_data);
			if (ret < 0) {
				pr_err(
				"%s usb_tx_urb_with_skb for raw, ret(%d)\n",
						__func__, ret);
				skb_queue_head(&ld->sk_raw_tx_q, skb);
				return;
			}
		}
	}
}

static long link_pm_ioctl(struct file *file, unsigned int cmd,
						unsigned long arg)
{
	int value, err = 0;
	struct link_pm_data *pm_data = file->private_data;

	lnk_info(pm_data->usb_ld, "cmd: 0x%08x\n", cmd);

	switch (cmd) {
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
	case IOCTL_LINK_PORT_ON:
		/* ignore cp host wakeup irq, set the hub_init_lock when AP try
		 CP off and release hub_init_lock when CP boot done */
		pm_data->hub_init_lock = 0;
		if (pm_data->port_enable) {
			err = pm_data->port_enable(2, 1);
			if (err < 0) {
				lnk_err(pm_data->usb_ld,
					 "hub on fail err=%d\n", err);
				goto exit;
			}
			pm_data->hub_status = HUB_STATE_RESUMMING;
		}
		break;
	case IOCTL_LINK_PORT_OFF:
		err = link_pm_hub_standby(pm_data);
		if (err < 0) {
			lnk_err(pm_data->usb_ld, "usb3503 active fail\n");
			goto exit;
		}
		pm_data->hub_init_lock = 1;
		pm_data->hub_handshake_done = 0;

		break;
	default:
		break;
	}
exit:
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
		link_pm_hub_standby(pm_data);
		return NOTIFY_OK;
	case PM_POST_SUSPEND:
		pm_data->dpm_suspending = false;
		return NOTIFY_OK;
	}
	return NOTIFY_DONE;
}

static int if_usb_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct usb_link_device *usb_ld = usb_get_intfdata(intf);
	int i;

	if (atomic_inc_return(&usb_ld->suspend_count) == IF_USB_DEVNUM_MAX) {
		lnk_dbg(usb_ld, "L2\n");

		for (i = 0; i < IF_USB_DEVNUM_MAX; i++)
			usb_kill_anchored_urbs(&usb_ld->devdata[i].reading);

		wake_unlock(&usb_ld->susplock);
	}

	return 0;
}

static void runtime_pm_work(struct work_struct *work)
{
	struct usb_link_device *usb_ld = container_of(work,
		struct usb_link_device, runtime_pm_work.work);
	int ret;

	ret = pm_request_autosuspend(&usb_ld->usbdev->dev);

	if (ret == -EAGAIN || ret == 1)
		queue_delayed_work(system_nrt_wq, &usb_ld->runtime_pm_work,
							msecs_to_jiffies(50));
}

static void wait_enumeration_work(struct work_struct *work)
{
	struct usb_link_device *usb_ld = container_of(work,
		struct usb_link_device, wait_enumeration.work);
	if (usb_ld->if_usb_connected == 0) {
		pr_err("USB disconnected and not enumerated for long time\n");
		usb_change_modem_state(usb_ld, STATE_CRASH_EXIT);
	}
}

static int if_usb_resume(struct usb_interface *intf)
{
	int i, ret;
	struct sk_buff *skb;
	struct usb_link_device *usb_ld = usb_get_intfdata(intf);
	struct if_usb_devdata *pipe;
	struct urb *urb;

	spin_lock_irq(&usb_ld->lock);
	if (!atomic_dec_return(&usb_ld->suspend_count)) {
		spin_unlock_irq(&usb_ld->lock);

		pr_debug("%s\n", __func__);
		wake_lock(&usb_ld->susplock);

		/* HACK: Runtime pm does not allow requesting autosuspend from
		 * resume callback, delayed it after resume */
		queue_delayed_work(system_nrt_wq, &usb_ld->runtime_pm_work,
							msecs_to_jiffies(50));

		for (i = 0; i < IF_USB_DEVNUM_MAX; i++) {
			pipe = &usb_ld->devdata[i];
			while ((urb = usb_get_from_anchor(&pipe->urbs))) {
				ret = usb_rx_submit(pipe, urb, GFP_KERNEL);
				if (ret < 0) {
					usb_put_urb(urb);
					pr_err(
					"%s: usb_rx_submit error with (%d)\n",
						__func__, ret);
					return ret;
				}
				usb_put_urb(urb);
			}
		}

		while ((urb = usb_get_from_anchor(&usb_ld->deferred))) {
			pr_debug("%s: got urb (0x%p) from anchor & resubmit\n",
					__func__, urb);
			ret = usb_submit_urb(urb, GFP_KERNEL);
			if (ret < 0) {
				pr_err("%s: resubmit failed\n", __func__);
				skb = urb->context;
				dev_kfree_skb_any(skb);
				usb_free_urb(urb);
				if (pm_runtime_put_autosuspend(
					&usb_ld->usbdev->dev) < 0)
					pr_debug(
					"pm_runtime_put_autosuspend fail\n");
			}
		}
		SET_SLAVE_WAKEUP(usb_ld->pdata, 1);
		udelay(100);
		SET_SLAVE_WAKEUP(usb_ld->pdata, 0);
		return 0;
	}

	spin_unlock_irq(&usb_ld->lock);
	return 0;
}

static int if_usb_reset_resume(struct usb_interface *intf)
{
	int ret;

	pr_debug("%s\n", __func__);
	ret = if_usb_resume(intf);
	return ret;
}

static struct usb_device_id if_usb_ids[] = {
	{ USB_DEVICE(0x04e8, 0x6999), /* CMC221 LTE Modem */
	/*.driver_info = 0,*/
	},
	{ } /* terminating entry */
};
MODULE_DEVICE_TABLE(usb, if_usb_ids);

static struct usb_driver if_usb_driver;
static void if_usb_disconnect(struct usb_interface *intf)
{
	struct usb_link_device *usb_ld  = usb_get_intfdata(intf);
	struct usb_device *usbdev = usb_ld->usbdev;
	int dev_id = intf->altsetting->desc.bInterfaceNumber;
	struct if_usb_devdata *pipe_data = &usb_ld->devdata[dev_id];

	usb_set_intfdata(intf, NULL);

	pipe_data->disconnected = 1;
	smp_wmb();

	wake_up(&usb_ld->l2_wait);

	usb_ld->if_usb_connected = 0;
	usb_ld->flow_suspend = 1;

	dev_dbg(&usbdev->dev, "%s\n", __func__);
	usb_ld->dev_count--;
	usb_driver_release_interface(&if_usb_driver, pipe_data->data_intf);

	usb_kill_anchored_urbs(&pipe_data->reading);
	usb_free_urbs(usb_ld, pipe_data);

	if (usb_ld->dev_count == 0) {
		cancel_delayed_work_sync(&usb_ld->runtime_pm_work);
		cancel_delayed_work_sync(&usb_ld->ld.tx_delayed_work);
		usb_put_dev(usbdev);
		usb_ld->usbdev = NULL;
	}
}

static int __devinit if_usb_probe(struct usb_interface *intf,
					const struct usb_device_id *id)
{
	struct usb_host_interface *data_desc;
	struct usb_link_device *usb_ld =
			(struct usb_link_device *)id->driver_info;
	struct link_device *ld = &usb_ld->ld;
	struct usb_interface *data_intf;
	struct usb_device *usbdev = interface_to_usbdev(intf);
	struct device *dev, *ehci_dev;
	struct if_usb_devdata *pipe;
	struct urb *urb;
	int i;
	int j;
	int dev_id;
	int err;

	/* To detect usb device order probed */
	dev_id = intf->cur_altsetting->desc.bInterfaceNumber;

	if (dev_id >= IF_USB_DEVNUM_MAX) {
		dev_err(&intf->dev, "Device id %d cannot support\n",
								dev_id);
		return -EINVAL;
	}

	if (!usb_ld) {
		dev_err(&intf->dev,
		"if_usb device doesn't be allocated\n");
		err = ENOMEM;
		goto out;
	}

	lnk_info(usb_ld, "probe dev_id=%d usb_device_id(0x%p), usb_ld (0x%p)\n",
				dev_id, id, usb_ld);

	usb_ld->usbdev = usbdev;

	usb_get_dev(usbdev);

	for (i = 0; i < IF_USB_DEVNUM_MAX; i++) {
		data_intf = usb_ifnum_to_if(usbdev, i);

		/* remap endpoint of RAW to no.1 for LTE modem */
		if (i == 0)
			pipe = &usb_ld->devdata[1];
		else if (i == 1)
			pipe = &usb_ld->devdata[0];
		else
			pipe = &usb_ld->devdata[i];

		pipe->disconnected = 0;
		pipe->data_intf = data_intf;
		data_desc = data_intf->cur_altsetting;

		/* Endpoints */
		if (usb_pipein(data_desc->endpoint[0].desc.bEndpointAddress)) {
			pipe->rx_pipe = usb_rcvbulkpipe(usbdev,
				data_desc->endpoint[0].desc.bEndpointAddress);
			pipe->tx_pipe = usb_sndbulkpipe(usbdev,
				data_desc->endpoint[1].desc.bEndpointAddress);
			pipe->rx_buf_size = 1024*4;
		} else {
			pipe->rx_pipe = usb_rcvbulkpipe(usbdev,
				data_desc->endpoint[1].desc.bEndpointAddress);
			pipe->tx_pipe = usb_sndbulkpipe(usbdev,
				data_desc->endpoint[0].desc.bEndpointAddress);
			pipe->rx_buf_size = 1024*4;
		}

		if (i == 0) {
			dev_info(&usbdev->dev, "USB IF USB device found\n");
		} else {
			err = usb_driver_claim_interface(&if_usb_driver,
					data_intf, usb_ld);
			if (err < 0) {
				pr_err("%s - failed to cliam usb interface\n",
						__func__);
				goto out;
			}
		}

		usb_set_intfdata(data_intf, usb_ld);
		usb_ld->dev_count++;
		pm_suspend_ignore_children(&data_intf->dev, true);

		for (j = 0; j < URB_COUNT; j++) {
			urb = usb_alloc_urb(0, GFP_KERNEL);
			if (!urb) {
				pr_err("%s: alloc urb fail\n", __func__);
				err = -ENOMEM;
				goto out2;
			}

			urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
			urb->transfer_buffer = usb_alloc_coherent(usbdev,
				pipe->rx_buf_size, GFP_KERNEL,
				&urb->transfer_dma);
			if (!urb->transfer_buffer) {
				pr_err(
				"%s: Failed to allocate transfer buffer\n",
					__func__);
				usb_free_urb(urb);
				err = -ENOMEM;
				goto out2;
			}

			usb_fill_bulk_urb(urb, usbdev, pipe->rx_pipe,
				urb->transfer_buffer, pipe->rx_buf_size,
				usb_rx_complete, pipe);
			usb_anchor_urb(urb, &pipe->urbs);
		}
	}

	/* temporary call reset_resume */
	atomic_set(&usb_ld->suspend_count, 1);
	if_usb_reset_resume(data_intf);
	atomic_set(&usb_ld->suspend_count, 0);

	SET_HOST_ACTIVE(usb_ld->pdata, 1);
	usb_ld->host_wake_timeout_flag = 0;

	if (gpio_get_value(usb_ld->pdata->gpio_phone_active)) {
		pm_runtime_set_autosuspend_delay(
				&usbdev->dev, AUTOSUSPEND_DELAY_MS);
		dev = &usb_ld->usbdev->dev;
		if (dev->parent) {
			dev_dbg(&usbdev->dev, "if_usb Runtime PM Start!!\n");
			usb_enable_autosuspend(usb_ld->usbdev);

			/* s5p-ehci runtime pm allow - usb phy suspend mode
			     svnet2->usb3503->root hub->ehci*/
			ehci_dev = dev->parent->parent->parent;
			lnk_dbg(usb_ld, "ehci device = %s, %s\n",
				dev_driver_string(ehci_dev),
				dev_name(ehci_dev));
			pm_runtime_allow(ehci_dev);

			usb_ld->link_pm_data->hub_status =
				(usb_ld->link_pm_data->root_hub) ?
				HUB_STATE_PREACTIVE : HUB_STATE_ACTIVE;

			usb_ld->link_pm_data->root_hub = dev->parent->parent;
		}

		usb_ld->flow_suspend = 0;
		/* Queue work if skbs were pending before a disconnect/probe */
		if (ld->sk_fmt_tx_q.qlen || ld->sk_raw_tx_q.qlen)
			queue_delayed_work(ld->tx_wq, &ld->tx_delayed_work, 0);

		usb_ld->if_usb_connected = 1;
		/*USB3503*/
		lnk_dbg(usb_ld, "hub active complete\n");

		usb_change_modem_state(usb_ld, STATE_ONLINE);
	} else {
		usb_change_modem_state(usb_ld, STATE_LOADER_DONE);
	}

	return 0;

out2:
	usb_ld->dev_count--;
	for (i = 0; i < IF_USB_DEVNUM_MAX; i++)
		usb_free_urbs(usb_ld, &usb_ld->devdata[i]);
out:
	usb_set_intfdata(intf, NULL);
	return err;
}

static irqreturn_t usb_resume_irq(int irq, void *data)
{
	int ret;
	struct usb_link_device *usb_ld = data;
	int val;
	struct device *dev;

	val = gpio_get_value(usb_ld->pdata->gpio_host_wakeup);
	irq_set_irq_type(irq, val ? IRQF_TRIGGER_LOW : IRQF_TRIGGER_HIGH);
	wake_lock_timeout(&usb_ld->gpiolock, 100);

	pr_err(LOG_TAG "< H-WUP %d\n", val);

	if (usb_ld->link_pm_data->hub_status == HUB_STATE_RESUMMING
		|| usb_ld->link_pm_data->hub_init_lock)
		return IRQ_HANDLED;

	if (usb_ld->link_pm_data->hub_status == HUB_STATE_OFF) {
		schedule_delayed_work(&usb_ld->link_pm_data->link_pm_hub, 0);
		return IRQ_HANDLED;
	}

	if (!usb_ld->if_usb_connected) {
		pr_err("mif: if not connected\n");
		return IRQ_HANDLED;
	}

	if (val) {
		dev = &usb_ld->usbdev->dev;
		pr_info("%s: runtime status=%d\n", __func__,
				dev->power.runtime_status);
		/* if usb3503 was on, usb_if was resumed by probe */
		if (dev->power.runtime_status == RPM_ACTIVE ||
			dev->power.runtime_status == RPM_RESUMING)
			return IRQ_HANDLED;

		device_lock(dev);
		if (dev->power.is_prepared || dev->power.is_suspended) {
			pm_runtime_get_noresume(dev);
			ret = 0;
		} else {
			ret = pm_runtime_get_sync(dev);
		}
		device_unlock(dev);
		if (ret < 0) {
			pr_err("%s pm_runtime_get fail (%d)\n", __func__, ret);
			return IRQ_HANDLED;
		}
	} else {
		if (!usb_ld->if_usb_connected) {
			pr_err("mif: if not connected\n");
			return IRQ_HANDLED;
		}

		if (usb_ld->resume_status == AP_INITIATED_RESUME)
			wake_up(&usb_ld->l2_wait);
		usb_ld->resume_status = CP_INITIATED_RESUME;
		pm_runtime_mark_last_busy(&usb_ld->usbdev->dev);
		pm_runtime_put_autosuspend(&usb_ld->usbdev->dev);
	}

	return IRQ_HANDLED;
}

static int if_usb_init(struct usb_link_device *usb_ld)
{
	int ret;
	int i;
	struct if_usb_devdata *pipe;

	/* give it to probe, or global variable needed */
	if_usb_ids[0].driver_info = (unsigned long)usb_ld;

	for (i = 0; i < IF_USB_DEVNUM_MAX; i++) {
		pipe = &usb_ld->devdata[i];
		pipe->format = i;
		pipe->disconnected = 1;
		init_usb_anchor(&pipe->urbs);
		init_usb_anchor(&pipe->reading);
	}

	init_waitqueue_head(&usb_ld->l2_wait);
	init_usb_anchor(&usb_ld->deferred);

	ret = usb_register(&if_usb_driver);
	if (ret) {
		pr_err("usb_register_driver() fail : %d\n", ret);
		return ret;
	}

	return 0;
}

static int usb_link_pm_init(struct usb_link_device *usb_ld, void *data)
{
	int err;
	int irq;
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
	pm_data->gpio_link_hostwake = pm_pdata->gpio_link_hostwake;
	pm_data->gpio_link_slavewake = pm_pdata->gpio_link_slavewake;
	pm_data->link_reconnect = pm_pdata->link_reconnect;
	pm_data->port_enable = pm_pdata->port_enable;

	pm_data->usb_ld = usb_ld;
	pm_data->link_pm_active = false;
	usb_ld->link_pm_data = pm_data;

	pm_data->miscdev.minor = MISC_DYNAMIC_MINOR;
	pm_data->miscdev.name = "link_pm";
	pm_data->miscdev.fops = &link_pm_fops;

	err = misc_register(&pm_data->miscdev);
	if (err < 0) {
		pr_err("%s:fail to register pm device(%d)\n", __func__, err);
		goto err_misc_register;
	}

	pm_data->hub_init_lock = 1;
	irq = gpio_to_irq(usb_ld->pdata->gpio_host_wakeup);
	err = request_threaded_irq(irq, NULL, usb_resume_irq,
		IRQF_TRIGGER_HIGH | IRQF_ONESHOT, "modem_usb_wake", usb_ld);
	if (err) {
		pr_err("Failed to allocate an interrupt(%d)\n", irq);
		goto err_request_irq;
	}
	enable_irq_wake(irq);

	init_completion(&pm_data->hub_active);
	pm_data->hub_status = HUB_STATE_OFF;
	pm_pdata->p_hub_status = &pm_data->hub_status;
	pm_data->hub_handshake_done = 0;
	pm_data->root_hub = NULL;
	wake_lock_init(&pm_data->hub_lock, WAKE_LOCK_SUSPEND,
		"modem_hub_enum_lock");
	INIT_DELAYED_WORK(&pm_data->link_pm_hub, link_pm_hub_work);
	pm_data->pm_notifier.notifier_call = link_pm_notifier_event;
	register_pm_notifier(&pm_data->pm_notifier);

	return 0;

err_request_irq:
	misc_deregister(&pm_data->miscdev);
err_misc_register:
	kfree(pm_data);
	return err;
}

struct link_device *usb_create_link_device(void *data)
{
	int ret;
	struct modem_data *pdata;
	struct platform_device *pdev = (struct platform_device *)data;
	struct usb_link_device *usb_ld;
	struct link_device *ld;

	pdata = pdev->dev.platform_data;

	usb_ld = kzalloc(sizeof(struct usb_link_device), GFP_KERNEL);
	if (!usb_ld)
		return NULL;

	INIT_LIST_HEAD(&usb_ld->ld.list);
	skb_queue_head_init(&usb_ld->ld.sk_fmt_tx_q);
	skb_queue_head_init(&usb_ld->ld.sk_raw_tx_q);
	spin_lock_init(&usb_ld->lock);

	ld = &usb_ld->ld;
	usb_ld->pdata = pdata;


	ld->name = "usb";
	ld->init_comm = usb_init_communication;
	ld->terminate_comm = usb_terminate_communication;
	ld->send = usb_send;
	ld->com_state = COM_NONE;

	/*ld->tx_wq = create_singlethread_workqueue("usb_tx_wq");*/
	ld->tx_wq = alloc_workqueue("usb_tx_wq",
		WQ_HIGHPRI | WQ_UNBOUND | WQ_RESCUER, 1);

	if (!ld->tx_wq) {
		pr_err("fail to create work Q.\n");
		return NULL;
	}

	usb_ld->pdata->irq_host_wakeup = platform_get_irq(pdev, 1);
	wake_lock_init(&usb_ld->gpiolock, WAKE_LOCK_SUSPEND,
		"modem_usb_gpio_wake");
	wake_lock_init(&usb_ld->susplock, WAKE_LOCK_SUSPEND,
		"modem_usb_suspend_block");

	INIT_DELAYED_WORK(&ld->tx_delayed_work, usb_tx_work);
	INIT_DELAYED_WORK(&usb_ld->runtime_pm_work, runtime_pm_work);
	INIT_DELAYED_WORK(&usb_ld->wait_enumeration, wait_enumeration_work);
	INIT_WORK(&usb_ld->disconnect_work, if_usb_force_disconnect);

	/* create link pm device */
	ret = usb_link_pm_init(usb_ld, data);
	if (ret)
		goto err;

	ret = if_usb_init(usb_ld);
	if (ret)
		goto err;

	return ld;
err:
	kfree(usb_ld);
	return NULL;
}

static struct usb_driver if_usb_driver = {
	.name =		"if_usb_driver",
	.probe =	if_usb_probe,
	.disconnect =	if_usb_disconnect,
	.id_table =	if_usb_ids,
	.suspend =	if_usb_suspend,
	.resume =	if_usb_resume,
	.reset_resume =	if_usb_reset_resume,
	.supports_autosuspend = 1,
};

static void __exit if_usb_exit(void)
{
	usb_deregister(&if_usb_driver);
}

static int lte_wake_resume(struct device *pdev)
{
	struct modem_data *pdata = pdev->platform_data;
	int val;


	val = gpio_get_value(pdata->gpio_host_wakeup);
	if (!val) {
		pr_debug("%s: > S-WUP 1\n", __func__);
		gpio_set_value(pdata->gpio_slave_wakeup, 1);
	}


	return 0;
}

static const struct dev_pm_ops lte_wake_pm_ops = {
	.resume     = lte_wake_resume,
};

static struct platform_driver lte_wake_driver = {
	.driver = {
		.name = "modem_lte_wake",
		.pm   = &lte_wake_pm_ops,
	},
};

static int __init lte_wake_init(void)
{
	return platform_driver_register(&lte_wake_driver);
}
module_init(lte_wake_init);
