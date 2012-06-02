/*
 * Copyright (C) 2011 Google, Inc.
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

#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/wakelock.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/if_arp.h>
#include <linux/platform_device.h>

#include <linux/platform_data/modem.h>
#include "modem_prj.h"
#include "modem_link_device_dpram.h"
#include "modem_utils.h"

static int memcmp16_to_io(const void __iomem *to, void *from, int size)
{
	int count = size >> 1;
	int i = 0;
	u16 *d = (u16 *)to;
	u16 *s = (u16 *)from;
	u16 d1 = 0;
	u16 s1 = 0;
	int diff = 0;

	for (i = 0; i < count; i++) {
		d1 = ioread16(d);
		s1 = *s;
		if (d1 != s1) {
			diff++;
			pr_err("[LNK/E] <%s> d[%d] 0x%04X != s[%d] 0x%04X\n",
				__func__, i, d1, i, s1);
		}
		d++;
		s++;
	}

	return diff;
}

static int dpram_test_memory(char *dp_name, u8 __iomem *start, u32 size)
{
	int i = 0;
	u8 __iomem *dst = NULL;
	u16 val = 0;

	pr_err("[LNK] <%s:%s> start = 0x%08X, size = %d\n",
		__func__, dp_name, (int)start, size);

	dst = start;
	for (i = 0; i < (size >> 1); i++) {
		iowrite16((i & 0xFFFF), dst);
		dst += 2;
	}

	dst = start;
	for (i = 0; i < (size >> 1); i++) {
		val = ioread16(dst);
		if (val != (i & 0xFFFF)) {
			pr_err("[LNK/E] <%s:%s> dst[%d] 0x%04X != 0x%04X\n",
				__func__, dp_name, i, val, (i & 0xFFFF));
			return -1;
		}
		dst += 2;
	}

	dst = start;
	for (i = 0; i < (size >> 1); i++) {
		iowrite16(0x00FF, dst);
		dst += 2;
	}

	dst = start;
	for (i = 0; i < (size >> 1); i++) {
		val = ioread16(dst);
		if (val != 0x00FF) {
			pr_err("[LNK/E] <%s:%s> dst[%d] 0x%04X != 0x00FF\n",
				__func__, dp_name, i, val);
			return -1;
		}
		dst += 2;
	}

	dst = start;
	for (i = 0; i < (size >> 1); i++) {
		iowrite16(0x0FF0, dst);
		dst += 2;
	}

	dst = start;
	for (i = 0; i < (size >> 1); i++) {
		val = ioread16(dst);
		if (val != 0x0FF0) {
			pr_err("[LNK/E] <%s:%s> dst[%d] 0x%04X != 0x0FF0\n",
				__func__, dp_name, i, val);
			return -1;
		}
		dst += 2;
	}

	dst = start;
	for (i = 0; i < (size >> 1); i++) {
		iowrite16(0xFF00, dst);
		dst += 2;
	}

	dst = start;
	for (i = 0; i < (size >> 1); i++) {
		val = ioread16(dst);
		if (val != 0xFF00) {
			pr_err("[LNK/E] <%s:%s> dst[%d] 0x%04X != 0xFF00\n",
				__func__, dp_name, i, val);
			return -1;
		}
		dst += 2;
	}

	dst = start;
	for (i = 0; i < (size >> 1); i++) {
		iowrite16(0, dst);
		dst += 2;
	}

	dst = start;
	for (i = 0; i < (size >> 1); i++) {
		val = ioread16(dst);
		if (val != 0) {
			pr_err("[LNK/E] <%s:%s> dst[%d] 0x%04X != 0\n",
				__func__, dp_name, i, val);
			return -1;
		}
		dst += 2;
	}

	pr_err("[LNK] <%s:%s> PASS!!!\n", __func__, dp_name);
	return 0;
}

static int dpram_register_isr
(
	unsigned irq,
	irqreturn_t (*isr)(int, void*),
	unsigned long flag,
	const char *name,
	struct link_device *ld
)
{
	int ret = 0;

	ret = request_irq(irq, isr, flag, name, ld);
	if (ret) {
		pr_err("[LNK/E] <%s> request_irq fail (%d)\n",
			__func__, ret);
		return ret;
	}

	ret = enable_irq_wake(irq);
	if (ret)
		pr_err("[LNK/E] <%s> enable_irq_wake fail (%d)\n",
			__func__, ret);

	pr_err("[LNK] <%s> IRQ#%d handler is registered.\n", __func__, irq);

	return 0;
}

static void dpram_register_timer
(
	struct dpram_link_device *dpld,
	struct timer_list *timer,
	unsigned long expire,
	void (*function)(unsigned long)
)
{
	init_timer(timer);
	timer->expires = get_jiffies_64() + expire;
	timer->function = function;
	timer->data = (unsigned long)dpld;
	add_timer(timer);
}

static inline bool dpram_circ_valid(int size, u32 head, u32 tail)
{
	if (head >= size) {
		pr_err("[LNK/E] head(%d) >= size(%d)\n", head, size);
		return false;
	}
	if (tail >= size) {
		pr_err("[LNK/E] tail(%d) >= size(%d)\n", tail, size);
		return false;
	}
	return true;
}

static struct sk_buff *dpram_read
(
	struct dpram_link_device *dpld,
	int dev_id,
	u32 head,
	u32 tail,
	u32 buff_size
)
{
	struct link_device *ld = &dpld->ld;
	struct modemlink_dpram_control *dpctl = dpld->dpctl;
	struct sk_buff *skb = NULL;
	u8 __iomem *src = NULL;
	u8  *buff = NULL;
	int  len = 0;
	int  len1 = 0, len2 = 0;

	/* Get data length in DPRAM*/
	len = (head > tail) ? (head - tail) : (buff_size - tail + head);

#if 0
	if (dev_id == IPC_FMT)
		pr_err("[LNK] <%s:%s> FMT, buff=%d, head=%d, tail=%d, len=%d\n",
			__func__, ld->name, buff_size, head, tail, len);
#endif
#if 0
	if (dev_id == IPC_RAW)
		pr_err("[LNK] <%s:%s> RAW, buff=%d, head=%d, tail=%d, len=%d\n",
			__func__, ld->name, buff_size, head, tail, len);
#endif
#if 0
	if (dev_id == IPC_RFS)
		pr_err("[LNK] <%s:%s> RFS, buff=%d, head=%d, tail=%d, len=%d\n",
			__func__, ld->name, buff_size, head, tail, len);
#endif

	skb = alloc_skb(len, GFP_ATOMIC);
	if (!skb) {
		pr_err("[LNK/E] <%s:%s> alloc_skb fail\n", __func__, ld->name);
		return NULL;
	}

	if (head > tail) {
		/* ----- (tail) 7f 00 00 7e (head) ----- */
		buff = skb_put(skb, len);
		src  = dpctl->get_rx_buff(dev_id) + tail;
		memcpy(buff, src, len);
	} else {
		/* 00 7e (head) ----------- (tail) 7f 00 */
		/* 1) tail -> buffer end */
		buff = skb_put(skb, len);
		src  = dpctl->get_rx_buff(dev_id) + tail;
		len1 = buff_size - tail;
		memcpy(buff, src, len1);

		/* 2) buffer start -> head */
		if (len > len1) {
			buff = buff + len1;
			src  = dpctl->get_rx_buff(dev_id);
			len2 = len - len1;
			memcpy(buff, src, len2);
		}
	}

	/* new tail */
	tail = (u32)((tail + len) % buff_size);
	dpctl->set_rx_tail(dev_id, tail);

	return skb;
}

static void dpram_rx_work(struct work_struct *work)
{
	struct link_device *ld = NULL;
	struct dpram_link_device *dpld = NULL;
	struct sk_buff *skb = NULL;
	struct io_device *iod = NULL;
	int  i = 0;

	dpld = container_of(work, struct dpram_link_device, rx_work);
	if (!dpld)
		pr_err("[LNK/E] <%s> dpld == NULL\n", __func__);
	ld = &dpld->ld;

	for (i = 0; i < dpld->max_ipc_dev; i++) {
		iod = dpld->iod[i];
		while ((skb = skb_dequeue(&dpld->skb_rxq[i])) != NULL) {
			if (iod->recv(iod, ld, skb->data, skb->len) < 0)
				pr_err("[LNK/E] <%s:%s> recv fail\n",
					__func__, ld->name);
			dev_kfree_skb_any(skb);
		}
	}
}

static int dpram_write
(
	struct dpram_link_device *dpld,
	int dev_id,
	const char *buff,
	unsigned size
)
{
	struct link_device *ld = &dpld->ld;
	struct modemlink_dpram_control *dpctl = dpld->dpctl;
	u8 __iomem *dst = NULL;
	u8         *src = NULL;
	unsigned    len = 0;
	u32  head = 0;
	u32  tail = 0;
	u32  qsize = 0;
	int  space = 0;
	u16  mask = 0;

	if (unlikely(!size)) {
		pr_err("[LNK/E] <%s:%s> dev_id = %d, size == 0\n",
			__func__, ld->name, dev_id);
		return -EINVAL;
	}

	if (dpctl->wakeup) {
		/* Wakeup DPRAM, then check magic and access */
		if (dpctl->wakeup())
			return -EINVAL;
	} else {
		/* Check magic and access */
		head = dpctl->get_magic();
		tail = dpctl->get_access();
		if (head != DPRAM_MAGIC_CODE || tail != 1) {
			pr_err("\n[LNK/E] <%s:%s> magic=0x%X, access=0x%X\n\n",
				__func__, ld->name, head, tail);
			return -EINVAL;
		}
	}

#if 0
	if (dev_id == IPC_FMT)
		pr_err("[LNK] <%s:%s> FMT, size = %d\n",
			__func__, ld->name, size);
#endif
#if 0
	if (dev_id == IPC_RAW)
		pr_err("[LNK] <%s:%s> RAW, size = %d\n",
			__func__, ld->name, size);
#endif
#if 0
	if (dev_id == IPC_RFS)
		pr_err("[LNK] <%s:%s> RFS, size = %d\n",
			__func__, ld->name, size);
#endif

	head = dpctl->get_tx_head(dev_id);
	tail = dpctl->get_tx_tail(dev_id);
	qsize = dpctl->get_tx_buff_size(dev_id);

	if (!dpram_circ_valid(qsize, head, tail)) {
		pr_err("[LNK/E] <%s:%s> invalid circular buffer\n",
			__func__, ld->name);
		dpctl->set_tx_head(dev_id, 0);
		dpctl->set_tx_tail(dev_id, 0);
		len = -EINVAL;
		goto exit;
	}

	space = (head < tail) ? (tail - head - 1) : (qsize + tail - head - 1);
	if (size > space) {
		pr_info("[LNK] <%s:%s> NO_SPC in TXQ[%d] "
			"(qsize[%d] free[%d] head[%d] tail[%d] vs. size[%d])\n",
			__func__, ld->name,
			dev_id, qsize, space, head, tail, size);
		mask = dpctl->get_mask_req_ack(dev_id);
		dpctl->send_intr(INT_NON_CMD(mask));
		len = -EAGAIN;
		goto exit;
	}

#if 0
	if (dev_id == IPC_FMT) {
		pr_err("\n[LNK] <%s:%s> Tx HDLC FMT frame (len %d)\n",
			__func__, ld->name, size);
		print_sipc4_hdlc_fmt_frame(buff);
		pr_err("\n");
	}
#endif
#if 0
	if (dev_id == IPC_RAW) {
		pr_err("\n[LNK] <%s:%s> Tx HDLC RAW frame (len %d)\n",
			__func__, ld->name, size);
		mif_print_data((char *)buff, (size < 64 ? size : 64));
		pr_err("\n");
	}
#endif
#if 0
	if (dev_id == IPC_RFS) {
		pr_err("\n[LNK] <%s:%s> Tx HDLC RFS frame (len %d)\n",
			__func__, ld->name, size);
		mif_print_data((char *)buff, (size < 64 ? size : 64));
		pr_err("\n");
	}
#endif

	if (head < tail) {
		/* +++++++++ head ---------- tail ++++++++++ */
		dst = dpctl->get_tx_buff(dev_id) + head;
		src = (u8 *)buff;
		len = size;
		memcpy(dst, src, len);
	} else {
		/* ------ tail +++++++++++ head ------------ */
		space = qsize - head;

		dst = dpctl->get_tx_buff(dev_id) + head;
		src = (u8 *)buff;
		len = (size > space) ? space : size;

		/* 1) head -> buffer end */
		memcpy(dst, src, len);

		/* 2) buffer start -> tail */
		if (size > space) {
			dst = dpctl->get_tx_buff(dev_id);
			src = (u8 *)(buff + space);
			len = (size - space);
			memcpy(dst, src, len);
		}
	}

	/* Update new head */
	head = (u32)((head + size) % qsize);
	dpctl->set_tx_head(dev_id, head);

	/* Send interrupt to CP */
	mask = dpctl->get_mask_send(dev_id);
	dpctl->send_intr(INT_NON_CMD(mask));

exit:
	if (dpctl->sleep)
		dpctl->sleep();
	return len;
}

static void dpram_tx_work(struct work_struct *work)
{
	struct link_device *ld = NULL;
	struct dpram_link_device *dpld = NULL;
	struct sk_buff *skb = NULL;
	bool reschedule = false;
	int  ret = 0;
	int  i = 0;

	ld = container_of(work, struct link_device, tx_delayed_work.work);
	dpld = to_dpram_link_device(ld);

	for (i = 0; i < dpld->max_ipc_dev; i++) {
		while ((skb = skb_dequeue(ld->skb_txq[i]))) {
			if (dpld->mode == DPRAM_LINK_MODE_IPC) {
				ret = dpram_write(dpld, i, skb->data, skb->len);
				if (ret < 0) {
					skb_queue_head(ld->skb_txq[i], skb);
					reschedule = true;
					break;
				}
			} else {
				pr_err("[LNK] <%s> dpld->mode != IPC_MODE\n",
					__func__);
			}

			dev_kfree_skb_any(skb);
		}
	}

	if (reschedule)
		schedule_delayed_work(
			&ld->tx_delayed_work, msecs_to_jiffies(10));
}

static void non_command_handler(struct dpram_link_device *dpld, u16 non_cmd)
{
	struct sk_buff *skb = NULL;
	struct link_device *ld = &dpld->ld;
	struct modemlink_dpram_control *dpctl = dpld->dpctl;
	int i = 0;
	u32 head = 0;
	u32 tail = 0;
	u32 size = 0;
	u16 magic = 0;
	u16 access = 0;
	u16 mask = 0;

	/* Check DPRAM mode */
	if (dpld->mode != DPRAM_LINK_MODE_IPC) {
		pr_err("[LNK/E] <%s:%s> dpld->mode != DPRAM_LINK_MODE_IPC",
			__func__, ld->name);
		return;
	}

	/* Check magic and access */
	magic = dpctl->get_magic();
	access = dpctl->get_access();
	if (magic != DPRAM_MAGIC_CODE || access != 1) {
		pr_err("[LNK/E] <%s:%s> magic=0x%X, access=0x%X\n",
			__func__, ld->name, magic, access);
		return;
	}

	/* Read data from DPRAM */
	for (i = 0; i < dpld->max_ipc_dev; i++) {
		head = dpctl->get_rx_head(i);
		tail = dpctl->get_rx_tail(i);
		if (head != tail) {
			/* Check each queue */
			size = dpctl->get_rx_buff_size(i);
			if (!dpram_circ_valid(size, head, tail)) {
				pr_err("[LNK/E] <%s:%s> RXQ[%d] invalid!!\n",
					__func__, ld->name, i);
				dpctl->set_rx_head(i, 0);
				dpctl->set_rx_tail(i, 0);
			}

			/* Read data from each queue */
			skb = dpram_read(dpld, i, head, tail, size);
			if (skb)
				skb_queue_tail(&dpld->skb_rxq[i], skb);
			else
				pr_err("[LNK/E] <%s:%s> read[%d] fail\n",
					__func__, ld->name, i);
		}

		/* Check and process REQ_ACK */
		if (non_cmd & dpctl->get_mask_req_ack(i)) {
			mask = dpctl->get_mask_res_ack(i);
			dpctl->send_intr(INT_NON_CMD(mask));
		}
	}

	schedule_work(&dpld->rx_work);
}

static int dpram_init_ipc(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;
	struct modemlink_dpram_control *dpctl = dpld->dpctl;
	int i = 0;
	u16 magic = 0;
	u16 access = 0;

	magic = dpctl->get_magic();
	access = dpctl->get_access();

	if ((dpld->mode == DPRAM_LINK_MODE_IPC) &&
	    (magic == DPRAM_MAGIC_CODE && access == 1)) {
		pr_err("[LNK] <%s:%s> IPC is already initialized!\n",
			__func__, ld->name);
		return 0;
	}

	getnstimeofday(&dpld->ts_base);

	/* Make aliases to each IO device */
	for (i = 0; i < MAX_DEV_FORMAT; i++)
		dpld->iod[i] = find_iodev(ld, i);
	dpld->iod[IPC_RAW] = dpld->iod[IPC_MULTI_RAW];

	/* List up the IO devices connected to each IPC channel */
	for (i = 0; i < MAX_DEV_FORMAT; i++) {
		if (dpld->iod[i])
			pr_err("[LNK] <%s:%s> dpld->iod[%d]->name = %s\n",
				__func__, ld->name, i, dpld->iod[i]->name);
		else
			pr_err("[LNK] <%s:%s> No dpld->iod[%d]\n",
				__func__, ld->name, i);
	}

	/* Clear pointers in every circular queue */
	for (i = 0; i < dpld->max_ipc_dev; i++) {
		dpctl->set_tx_head(i, 0);
		dpctl->set_tx_tail(i, 0);
		dpctl->set_rx_head(i, 0);
		dpctl->set_rx_tail(i, 0);
	}

	/* Enable IPC */
	dpctl->set_magic(DPRAM_MAGIC_CODE);
	dpctl->set_access(1);

	magic = dpctl->get_magic();
	if (magic != DPRAM_MAGIC_CODE) {
		pr_err("[LNK/E] <%s:%s> magic code (0x%04X) != 0x%04X\n",
			__func__, ld->name, magic, DPRAM_MAGIC_CODE);
		return -1;
	}

	access = dpctl->get_access();
	if (access != 1) {
		pr_err("[LNK/E] <%s:%s> access enable (0x%04X) != 1\n",
			__func__, ld->name, access);
		return -1;
	}

	dpld->mode = DPRAM_LINK_MODE_IPC;

	return 0;
}

static void cmd_req_active_handler(struct dpram_link_device *dpld)
{
	dpld->dpctl->send_intr(INT_CMD(INT_CMD_RES_ACTIVE));
}

static void cmd_error_display_handler(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;
	struct io_device *iod = NULL;

	pr_err("[LNK] <%s:%s> Receive 0xC9 (Error Display)\n",
		__func__, ld->name);

	if (ld->mdm_data->modem_type == QC_MDM6600) {
		if (dpld->dpctl->log_disp)
			dpld->dpctl->log_disp(dpld->dpctl);
	}

	iod = find_iodev(ld, IPC_FMT);
	iod->modem_state_changed(iod, STATE_CRASH_EXIT);
}

static void cmd_phone_start_handler(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;
	struct io_device *iod = NULL;

	pr_err("[LNK] <%s:%s> Receive 0xC8\n", __func__, ld->name);

	dpram_init_ipc(dpld);

	iod = find_iodev(ld, IPC_FMT);
	if (!iod) {
		pr_err("[LNK/E] <%s:%s> iod == NULL\n", __func__, ld->name);
		return;
	}

	if (ld->mdm_data->modem_type == SEC_CMC221) {
		if (iod->mc->phone_state != STATE_ONLINE) {
			pr_err("[LNK] <%s:%s> phone_state: %d -> ONLINE\n",
				__func__, ld->name, iod->mc->phone_state);
			iod->modem_state_changed(iod, STATE_ONLINE);
		}
	} else if (ld->mdm_data->modem_type == QC_MDM6600) {
		if (dpld->dpctl->phone_boot_start_handler)
			dpld->dpctl->phone_boot_start_handler(dpld->dpctl);
	} else {
		/* Send INIT_END code to CP */
		pr_err("[LNK] <%s:%s> Send 0xC2 (INIT_END)\n",
			__func__, ld->name);
		dpld->dpctl->send_intr(INT_CMD(INT_CMD_INIT_END));
	}
}

static void command_handler(struct dpram_link_device *dpld, u16 cmd)
{
	struct link_device *ld = &dpld->ld;

	switch (INT_CMD_MASK(cmd)) {
	case INT_CMD_REQ_ACTIVE:
		cmd_req_active_handler(dpld);
		break;

	case INT_CMD_ERR_DISPLAY:
		dpld->dpram_init_status = DPRAM_INIT_STATE_NONE;
		cmd_error_display_handler(dpld);
		break;

	case INT_CMD_PHONE_START:
		dpld->dpram_init_status = DPRAM_INIT_STATE_READY;
		cmd_phone_start_handler(dpld);
		complete_all(&dpld->dpram_init_cmd);
		break;

	case INT_CMD_NV_REBUILDING:
		pr_err("[LNK] NV_REBUILDING\n");
		break;

	case INT_CMD_PIF_INIT_DONE:
		complete_all(&dpld->modem_pif_init_done);
		break;

	case INT_CMD_SILENT_NV_REBUILDING:
		pr_err("[LNK] SILENT_NV_REBUILDING\n");
		break;

	case INT_CMD_NORMAL_POWER_OFF:
		/*ToDo:*/
		/*kernel_sec_set_cp_ack()*/;
		break;

	case INT_CMD_REQ_TIME_SYNC:
	case INT_CMD_PHONE_DEEP_SLEEP:
	case INT_CMD_EMER_DOWN:
		break;

	default:
		pr_err("[LNK/E] <%s:%s> Unknown command.. 0x%04X\n",
			__func__, ld->name, cmd);
	}
}

static void cmc22x_idpram_enable_ipc(struct dpram_link_device *dpld)
{
	dpld->dpctl->set_magic(DPRAM_MAGIC_CODE);
	dpld->dpctl->set_access(1);
	enable_irq(dpld->irq);
}

static void cmc22x_idpram_disable_ipc(struct dpram_link_device *dpld)
{
	disable_irq_nosync(dpld->irq);
	dpld->dpctl->set_magic(0);
	dpld->dpctl->set_access(0);

	if (dpld->dpctl->setup_speed)
		dpld->dpctl->setup_speed(DPRAM_SPEED_LOW);
}

static int cmc22x_idpram_wait_response(struct dpram_link_device *dpld, u32 resp)
{
	int count = 100;
	u32 rcvd = 0;

	if (resp == CMC22x_CP_REQ_NV_DATA) {
		while (1) {
			rcvd = ioread32(dpld->bt_map.resp);
			if (rcvd == resp)
				break;

			if (count-- < 0) {
				pr_err("[LNK/E] <%s> No resp (0x%08X)!!!\n",
					__func__, resp);
				return -EAGAIN;
			}
		}
	} else {
		while (1) {
			rcvd = dpld->dpctl->recv_msg();

			if (rcvd == resp)
				break;

			if (resp == CMC22x_CP_RECV_NV_END &&
			    rcvd == CMC22x_CP_CAL_BAD) {
				pr_err("[LNK/E] <%s> CMC22x_CP_CAL_BAD!!!\n",
					__func__);
				break;
			}

			if (count-- < 0) {
				pr_err("[LNK/E] <%s> No resp (0x%04X)!!!\n",
					__func__, resp);
				return -EAGAIN;
			}
		}
	}

	return rcvd;
}

static int cmc22x_idpram_send_boot(struct link_device *ld, unsigned long arg)
{
	int err = 0;
	int cnt = 0;
	struct dpram_link_device *dpld = to_dpram_link_device(ld);
	u8 __iomem *bt_buff = dpld->bt_map.buff;
	struct dpram_boot_img cp_img;
	u8 *img_buff = NULL;

	dpld->mode = DPRAM_LINK_MODE_BOOT;
	cmc22x_idpram_disable_ipc(dpld);

	/* Test memory... After testing, memory is cleared. */
	if (dpram_test_memory(ld->name, bt_buff, dpld->bt_map.size) < 0) {
		pr_err("\n[LNK/E] <%s> dpram_test_memory fail!\n\n", __func__);
		dpld->mode = DPRAM_LINK_MODE_INVALID;
		return -EIO;
	}

	/* Get information about the boot image */
	err = copy_from_user((struct dpram_boot_img *)&cp_img,
			     (void *)arg,
			     sizeof(struct dpram_boot_img));
	pr_err("[LNK] <%s> CP image addr = 0x%08X, size = 0x%X\n",
		__func__, (int)cp_img.addr, cp_img.size);

	/* Alloc a buffer for the boot image */
	img_buff = kzalloc(dpld->bt_map.size, GFP_KERNEL);
	if (!img_buff) {
		pr_err("[LNK/E] <%s> kzalloc fail\n", __func__);
		dpld->mode = DPRAM_LINK_MODE_INVALID;
		return -ENOMEM;
	}

	/* Copy boot image from the user space to the image buffer */
	err = copy_from_user(img_buff, cp_img.addr, cp_img.size);

	/* Copy boot image to DPRAM and verify it */
	memcpy(bt_buff, img_buff, cp_img.size);
	if (memcmp16_to_io(bt_buff, img_buff, cp_img.size)) {
		pr_err("\n[LNK/E] <%s> Boot may be broken!!!\n\n", __func__);
		goto err;
	}

	dpld->dpctl->reset();
	udelay(1000);

	if (cp_img.mode == CMC22x_BOOT_MODE_NORMAL) {
		pr_err("[LNK] <%s> CMC22x_BOOT_MODE_NORMAL\n", __func__);
		iowrite32(CMC22x_AP_BOOT_DOWN_DONE, dpld->bt_map.req);

		/* Wait for CMC22x_CP_REQ_MAIN_BIN from CP for 1 second */
		while (ioread32(dpld->bt_map.resp) != CMC22x_CP_REQ_MAIN_BIN) {
			cnt++;
			msleep_interruptible(10);
			if (cnt > 100) {
				pr_err("\n[LNK/E] <%s> No CP_REQ_MAIN_BIN!!!\n",
					__func__);
				goto err;
			}
		}
	} else {
		pr_err("[LNK] <%s> CMC22x_BOOT_MODE_DUMP\n", __func__);
	}

	kfree(img_buff);

	pr_err("[LNK] <%s> Boot send done.\n", __func__);

	if (dpld->dpctl->setup_speed)
		dpld->dpctl->setup_speed(DPRAM_SPEED_HIGH);

	return 0;

err:
	dpld->mode = DPRAM_LINK_MODE_INVALID;
	kfree(img_buff);

	pr_err("[LNK/E] <%s> Boot send fail!!!\n\n", __func__);
	return -EIO;
}

static int cmc22x_idpram_send_main(struct link_device *ld, struct sk_buff *skb)
{
	int err = 0;
	struct dpram_link_device *dpld = to_dpram_link_device(ld);
	struct dpram_boot_frame *bf = (struct dpram_boot_frame *)skb->data;
	u8 __iomem *buff = (dpld->bt_map.buff + bf->offset);

	if ((bf->offset + bf->len) > dpld->bt_map.size) {
		pr_err("[LNK/E] <%s> Out of DPRAM boundary\n", __func__);
		err = -EINVAL;
		goto exit;
	}

	if (bf->len) {
		memcpy(buff, bf->data, bf->len);
#if 0
		if (memcmp16_to_io(buff, bf->data, bf->len)) {
			pr_err("\n[LNK/E] <%s> Binary may be broken!!!\n\n",
				__func__);
			goto exit;
		}
#endif
	}

	if (bf->request)
		dpld->dpctl->send_msg((u16)bf->request);

	if (bf->response) {
		err = cmc22x_idpram_wait_response(dpld, bf->response);
		if (err < 0)
			pr_err("[LNK/E] <%s> wait_response fail (%d)\n",
				__func__, err);
	}

	if (bf->request == CMC22x_CAL_NV_DOWN_END) {
		pr_err("[LNK] <%s> CMC22x_CAL_NV_DOWN_END\n", __func__);
		cmc22x_idpram_enable_ipc(dpld);
	}

exit:
	dev_kfree_skb_any(skb);
	return err;
}

static void cmc22x_idpram_wait_dump(unsigned long arg)
{
	struct dpram_link_device *dpld = (struct dpram_link_device *)arg;
	u16 msg;

	if (!dpld) {
		pr_err("[LNK/E] <%s> dpld == NULL\n", __func__);
		return;
	}

	msg = dpld->dpctl->recv_msg();

	if (msg == CMC22x_CP_DUMP_END) {
		complete_all(&dpld->dump_recv_done);
		return;
	}

	if (((dpld->dump_rcvd & 0x1) == 0) && (msg == CMC22x_1ST_BUFF_FULL)) {
		complete_all(&dpld->dump_recv_done);
		return;
	}

	if (((dpld->dump_rcvd & 0x1) == 1) && (msg == CMC22x_2ND_BUFF_FULL)) {
		complete_all(&dpld->dump_recv_done);
		return;
	}

	dpram_register_timer(dpld,
			     &dpld->dump_timer,
			     CMC22x_DUMP_WAIT_TIMEOVER,
			     cmc22x_idpram_wait_dump);
}

static int cmc22x_idpram_upload
(
	struct dpram_link_device *dpld,
	struct dpram_dump_arg *dumparg
)
{
	int ret = 0;
	u8 __iomem *src;
	int buff_size = CMC22x_DUMP_BUFF_SIZE;

	if ((dpld->dump_rcvd & 0x1) == 0)
		dpld->dpctl->send_msg(CMC22x_1ST_BUFF_READY);
	else
		dpld->dpctl->send_msg(CMC22x_2ND_BUFF_READY);

	init_completion(&dpld->dump_recv_done);

	dpram_register_timer(dpld,
			     &dpld->dump_timer,
			     CMC22x_DUMP_WAIT_TIMEOVER,
			     cmc22x_idpram_wait_dump);

	ret = wait_for_completion_interruptible_timeout(
			&dpld->dump_recv_done, DUMP_TIMEOUT);
	if (!ret) {
		pr_err("[LNK/E] CP didn't send dump data!!!\n");
		goto err_out;
	}

	if (dpld->dpctl->recv_msg() == CMC22x_CP_DUMP_END) {
		pr_err("[LNK] <%s> CMC22x_CP_DUMP_END\n", __func__);
		wake_unlock(&dpld->dpram_wake_lock);
		return 0;
	}

	if ((dpld->dump_rcvd & 0x1) == 0)
		src = dpld->ul_map.buff;
	else
		src = dpld->ul_map.buff + CMC22x_DUMP_BUFF_SIZE;

	memcpy(dpld->buff, src, buff_size);

	ret = copy_to_user(dumparg->buff, dpld->buff, buff_size);
	if (ret < 0) {
		pr_err("[LNK/E] <%s> copy_to_user fail\n", __func__);
		goto err_out;
	}

	dpld->dump_rcvd++;

	return buff_size;

err_out:
	pr_err("[LNK/E] <%s> error out!!!\n", __func__);

	wake_unlock(&dpld->dpram_wake_lock);
	return -EIO;
}

static int cmc22x_idpram_force_dump(struct dpram_link_device *dpld)
{
	pr_err("[LNK] <%s>\n", __func__);
	dpld->dpctl->send_intr(INT_CMD(INT_CMD_ERR_DISPLAY));
	return 0;
}

static int dpram_upload
(
	struct dpram_link_device *dpld,
	struct dpram_firmware *uploaddata
)
{
	struct ul_header header;
	u8 *dest;
	u8 *buff = vmalloc(DP_DEFAULT_DUMP_LEN);
	u16 plen = 0;
	u32 tlen = 0;
	int ret;
	int region = 0;

	pr_err("[LNK] <%s>\n", __func__);

	ret = wait_for_completion_interruptible_timeout(
			&dpld->dump_start_complete,
			DUMP_START_TIMEOUT);
	if (!ret) {
		pr_err("[LNK/E] <%s> No UPLOAD_START!!!\n", __func__);
		goto err_out;
	}

	wake_lock(&dpld->dpram_wake_lock);

	memset(buff, 0, DP_DEFAULT_DUMP_LEN);

	dpld->dpctl->send_intr(CMD_IMG_SEND_REQ);
	pr_debug("[LNK] write CMD_IMG_SEND_REQ(0x9400)\n");

	while (1) {
		init_completion(&dpld->dump_recv_done);
		ret = wait_for_completion_interruptible_timeout(
				&dpld->dump_recv_done, DUMP_TIMEOUT);
		if (!ret) {
			pr_err("[LNK/E] No DATA_SEND_DONE_RESP!!!\n");
			goto err_out;
		}

		dest = (u8 *)dpld->ul_map.buff;

		header.bop = *(u8 *)(dest);
		header.total_frame = *(u16 *)(dest + 1);
		header.curr_frame = *(u16 *)(dest + 3);
		header.len = *(u16 *)(dest + 5);

		pr_debug("total frame:%d, current frame:%d, data len:%d\n",
			header.total_frame, header.curr_frame,
			header.len);

		dest += DP_DUMP_HEADER_SIZE;
		plen = min_t(u16, header.len, DP_DEFAULT_DUMP_LEN);

		memcpy(buff, dest, plen);
		dest += plen;

		ret = copy_to_user(uploaddata->firmware + tlen,	buff,  plen);
		if (ret < 0) {
			pr_err("[LNK/E] <%s> copy_to_user fail\n", __func__);
			goto err_out;
		}

		tlen += plen;

		if (header.total_frame == header.curr_frame) {
			if (region) {
				uploaddata->is_delta = tlen - uploaddata->size;
				dpld->dpctl->send_intr(CMD_UL_RECEIVE_RESP);
				break;
			} else {
				uploaddata->size = tlen;
				region = 1;
			}
		}
		dpld->dpctl->send_intr(CMD_UL_RECEIVE_RESP);
	}

	pr_debug("1st dump region data size=%d\n", uploaddata->size);
	pr_debug("2st dump region data size=%d\n", uploaddata->is_delta);

	init_completion(&dpld->gota_send_done);
	ret = wait_for_completion_interruptible_timeout(
			&dpld->gota_send_done, DUMP_TIMEOUT);
	if (!ret) {
		pr_err("[LNK/E] CP didn't send SEND_DONE_RESP\n");
		goto err_out;
	}

	dpld->dpctl->send_intr(CMD_UL_RECEIVE_DONE_RESP);
	pr_debug("[LNK] write CMD_UL_RECEIVE_DONE_RESP(0x9801)\n");

	iowrite32(0, dpld->ul_map.magic); /*clear magic code */

	wake_unlock(&dpld->dpram_wake_lock);

	vfree(buff);
	return 0;

err_out:
	vfree(buff);
	iowrite32(0, dpld->ul_map.magic);
	pr_err("[LNK/E] <%s> CDMA dump error out\n", __func__);
	wake_unlock(&dpld->dpram_wake_lock);
	return -EIO;
}

static int dpram_download
(
	struct dpram_link_device *dpld,
	const char *buf,
	int len
)
{
	struct dpram_ota_header header;
	u16 nframes;
	u16 curframe = 1;
	u16 plen;
	u8 *dest;
	int ret;

	nframes = DIV_ROUND_UP(len, DP_DEFAULT_WRITE_LEN);

	pr_debug("[GOTA] download len = %d\n", len);

	header.start_index = START_INDEX;
	header.nframes = nframes;

	while (len > 0) {
		plen = min_t(int, len, DP_DEFAULT_WRITE_LEN);
		dest = dpld->dl_map.buff;

		pr_debug("[GOTA] Start write frame %d/%d\n", curframe, nframes);

		header.curframe = curframe;
		header.len = plen;

		memcpy(dest, &header, sizeof(header));
		dest += sizeof(header);

		memcpy(dest, buf, plen);
		dest += plen;
		buf += plen;
		len -= plen;

		iowrite8(END_INDEX, dest+3);

		init_completion(&dpld->gota_send_done);

		if (curframe == 1) {
			ret = wait_for_completion_interruptible_timeout(
				&dpld->gota_start_complete,
				GOTA_TIMEOUT);
			if (!ret) {
				pr_err("[GOTA/E] No DOWNLOAD_START!!!\n");
				return -ENXIO;
			}
		}

		dpld->dpctl->send_intr(CMD_IMG_SEND_REQ);
		ret = wait_for_completion_interruptible_timeout(
				&dpld->gota_send_done, GOTA_SEND_TIMEOUT);
		if (!ret) {
			pr_err("[GOTA/E] No SEND_DONE_RESP!!!\n");
			return -ENXIO;
		}

		curframe++;
	}

	dpld->dpctl->send_intr(CMD_DL_SEND_DONE_REQ);
	ret = wait_for_completion_interruptible_timeout(
			&dpld->gota_update_done, GOTA_TIMEOUT);
	if (!ret) {
		pr_err("[GOTA/E] No UPDATE_DONE_NOTIFICATION!!!\n");
		return -ENXIO;
	}

	return 0;
}

static int dpram_process_modem_update
(
	struct dpram_link_device *dpld,
	struct dpram_firmware *pfw
)
{
	int ret = 0;
	char *buff = vmalloc(pfw->size);

	pr_debug("[GOTA] modem size =[%d]\n", pfw->size);

	if (!buff)
		return -ENOMEM;

	ret = copy_from_user(buff, pfw->firmware, pfw->size);
	if (ret < 0) {
		pr_err("[GOTA/E] <%s> copy_from_user fail\n", __func__);
		goto out;
	}

	ret = dpram_download(dpld, buff, pfw->size);
	if (ret < 0)
		pr_err("[GOTA/E] <%s> firmware write fail\n", __func__);

out:
	vfree(buff);
	return ret;
}

static void gota_cmd_handler(struct dpram_link_device *dpld, u16 cmd)
{
	if (cmd & GOTA_RESULT_FAIL) {
		pr_err("[GOTA/E] <%s> Command failed: %04x\n", __func__, cmd);
		return;
	}

	switch (GOTA_CMD_MASK(cmd)) {
	case GOTA_CMD_RECEIVE_READY:
		pr_debug("[GOTA] Send CP-->AP RECEIVE_READY\n");
		dpld->dpctl->send_intr(CMD_DL_START_REQ);
		break;

	case GOTA_CMD_DOWNLOAD_START_RESP:
		pr_debug("[GOTA] Send CP-->AP DOWNLOAD_START_RESP\n");
		complete_all(&dpld->gota_start_complete);
		break;

	case GOTA_CMD_SEND_DONE_RESP:
		pr_debug("[GOTA] Send CP-->AP SEND_DONE_RESP\n");
		complete_all(&dpld->gota_send_done);
		break;

	case GOTA_CMD_UPDATE_DONE:
		pr_debug("[GOTA] Send CP-->AP UPDATE_DONE\n");
		complete_all(&dpld->gota_update_done);
		break;

	case GOTA_CMD_IMAGE_SEND_RESP:
		pr_debug("[GOTA] Send CP-->AP IMAGE_SEND_RESP\n");
		complete_all(&dpld->gota_recv_done);
		break;

	default:
		pr_err("[GOTA/E] <%s> Unknown command.. %x\n", __func__, cmd);
	}
}

static irqreturn_t dpram_irq_handler(int irq, void *p_ld)
{
	struct link_device *ld = (struct link_device *)p_ld;
	struct dpram_link_device *dpld = to_dpram_link_device(ld);
	u16 int2ap = 0;

	disable_irq_nosync(irq);

	int2ap = dpld->dpctl->recv_intr();

#if 0
	getnstimeofday(&dpld->ts_now);
	pr_err("\n[LNK] <%s:%s> {%ld.%09ld} int2ap = 0x%04X\n",
		__func__, ld->name,
		(long)(dpld->ts_now.tv_sec - dpld->ts_base.tv_sec),
		(long)(dpld->ts_now.tv_nsec),
		int2ap);
#endif

	if (int2ap == INT_POWERSAFE_FAIL) {
		pr_err("[LNK] <%s> int2ap == INT_POWERSAFE_FAIL\n", __func__);
		goto exit_irq;
	}

	if (ld->mdm_data->modem_type == QC_MDM6600) {
		if ((int2ap == 0x1234)|(int2ap == 0xDBAB)|(int2ap == 0xABCD)) {
			if (dpld->dpctl->dload_cmd_hdlr)
				dpld->dpctl->dload_cmd_hdlr(dpld->dpctl,
									int2ap);
			goto exit_irq;
		}
	}

	if (GOTA_CMD_VALID(int2ap))
		gota_cmd_handler(dpld, int2ap);
	else if (INT_CMD_VALID(int2ap))
		command_handler(dpld, int2ap);
	else if (INT_VALID(int2ap))
		non_command_handler(dpld, int2ap);
	else
		pr_err("[LNK/E] <%s> Invalid interrupt (0x%04X)\n",
			__func__, int2ap);

exit_irq:
	dpld->dpctl->clear_intr();
	enable_irq(irq);
	return IRQ_HANDLED;
}

static int dpram_send_binary(struct link_device *ld, struct sk_buff *skb)
{
	int err = 0;

	if (ld->mdm_data->modem_type == SEC_CMC221)
		err = cmc22x_idpram_send_main(ld, skb);

	return err;
}

static int dpram_send(struct link_device *ld, struct io_device *iod,
		      struct sk_buff *skb)
{
	int len = skb->len;

	switch (iod->format) {
	case IPC_BOOT:
		return dpram_send_binary(ld, skb);

	case IPC_FMT:
		skb_queue_tail(&ld->sk_fmt_tx_q, skb);
		break;

	case IPC_RAW:
		skb_queue_tail(&ld->sk_raw_tx_q, skb);
		break;

	case IPC_RFS:
		skb_queue_tail(&ld->sk_rfs_tx_q, skb);
		break;

	default:
		dev_kfree_skb_any(skb);
		return 0;
	}

	schedule_delayed_work(&ld->tx_delayed_work, 0);

	return len;
}

static int dpram_set_dl_magic(struct link_device *ld, struct io_device *iod)
{
	struct dpram_link_device *dpld = to_dpram_link_device(ld);

	dpld->mode = DPRAM_LINK_MODE_DLOAD;

	iowrite32(DP_MAGIC_DMDL, dpld->dl_map.magic);

	return 0;
}

static int dpram_force_dump(struct link_device *ld, struct io_device *iod)
{
	struct dpram_link_device *dpld = to_dpram_link_device(ld);
	if (ld->mdm_data->modem_type == SEC_CMC221)
		return cmc22x_idpram_force_dump(dpld);
	return 0;
}

static int dpram_set_ul_magic(struct link_device *ld, struct io_device *iod)
{
	struct dpram_link_device *dpld = to_dpram_link_device(ld);
	u8 *dest = dpld->ul_map.buff;

	dpld->mode = DPRAM_LINK_MODE_ULOAD;

	if (ld->mdm_data->modem_type == SEC_CMC221) {
		wake_lock(&dpld->dpram_wake_lock);
		dpld->dump_rcvd = 0;
		iowrite32(CMC22x_CP_DUMP_MAGIC, dpld->ul_map.magic);
	} else {
		iowrite32(DP_MAGIC_UMDL, dpld->ul_map.magic);

		iowrite8((u8)START_INDEX, dest + 0);
		iowrite8((u8)0x1, dest + 1);
		iowrite8((u8)0x1, dest + 2);
		iowrite8((u8)0x0, dest + 3);
		iowrite8((u8)END_INDEX, dest + 4);

		dpld->dpctl->send_intr(CMD_DL_START_REQ);
	}

	init_completion(&dpld->dump_start_complete);

	return 0;
}

static int dpram_modem_update(struct link_device *ld, struct io_device *iod,
							unsigned long arg)
{
	int ret;
	struct dpram_link_device *dpld = to_dpram_link_device(ld);
	struct dpram_firmware fw;

	pr_debug("[GOTA] <%s>\n", __func__);

	ret = copy_from_user(&fw, (void __user *)arg, sizeof(fw));
	if (ret  < 0) {
		pr_err("[GOTA/E] <%s> copy_from_user fail\n", __func__);
		return ret;
	}

	return dpram_process_modem_update(dpld, &fw);
}

static int dpram_dump_update
(
	struct link_device *ld,
	struct io_device *iod,
	unsigned long arg
)
{
	struct dpram_link_device *dpld = to_dpram_link_device(ld);
	struct dpram_firmware *fw = (struct dpram_firmware *)arg ;
	struct dpram_dump_arg *dump = (struct dpram_dump_arg *)arg;

	pr_debug("[LNK] <%s>\n", __func__);

	if (ld->mdm_data->modem_type == SEC_CMC221)
		return cmc22x_idpram_upload(dpld, dump);
	else
		return dpram_upload(dpld, fw);
}

static int dpram_link_ioctl
(
	struct link_device *ld,
	struct io_device *iod,
	unsigned int cmd,
	unsigned long arg
)
{
	int err = -EFAULT;
	struct dpram_link_device *dpld = to_dpram_link_device(ld);

	pr_debug("[LNK] <%s> cmd = 0x%08X\n", __func__, cmd);

	switch (cmd) {
	case IOCTL_DPRAM_SEND_BOOT:
		err = cmc22x_idpram_send_boot(ld, arg);
		if (err < 0) {
			lnk_err(dpld, "dpram boot send fail\n");
			goto exit;
		}
		break;

	case IOCTL_DPRAM_PHONE_POWON:
		if (dpld->dpctl->cpimage_load_prepare) {
			err = dpld->dpctl->cpimage_load_prepare(dpld->dpctl);
			if (err < 0) {
				lnk_err(dpld, "cpimage_load_prepare fail\n");
				goto exit;
			}
		}
		break;

	case IOCTL_DPRAM_PHONEIMG_LOAD:
		if (dpld->dpctl->cpimage_load) {
			err = dpld->dpctl->cpimage_load(
					(void *)arg, dpld->dpctl);
			if (err < 0) {
				lnk_err(dpld, "cpimage_load fail\n");
				goto exit;
			}
		}
		break;

	case IOCTL_DPRAM_NVDATA_LOAD:
		if (dpld->dpctl->nvdata_load) {
			err = dpld->dpctl->nvdata_load(
					(void *)arg, dpld->dpctl);
			if (err < 0) {
				lnk_err(dpld, "nvdata_load fail\n");
				goto exit;
			}
		}
		break;

	case IOCTL_DPRAM_PHONE_BOOTSTART:
		if (dpld->dpctl->phone_boot_start) {
			err = dpld->dpctl->phone_boot_start(dpld->dpctl);
			if (err < 0) {
				lnk_err(dpld, "phone_boot_start fail\n");
				goto exit;
			}
		}
		if (dpld->dpctl->phone_boot_start_post_process) {
			err = dpld->dpctl->phone_boot_start_post_process();
			if (err < 0) {
				lnk_err(dpld, "phone_boot_start_post_process fail\n");
				goto exit;
			}
		}
		break;

	case IOCTL_DPRAM_PHONE_UPLOAD_STEP1:
		disable_irq_nosync(dpld->irq);

		if (dpld->dpctl->cpupload_step1) {
			err = dpld->dpctl->cpupload_step1(dpld->dpctl);
			if (err < 0) {
				dpld->dpctl->clear_intr();
				enable_irq(dpld->irq);
				lnk_err(dpld, "cpupload_step1 fail\n");
				goto exit;
			}
		}
		break;

	case IOCTL_DPRAM_PHONE_UPLOAD_STEP2:
		if (dpld->dpctl->cpupload_step2) {
			err = dpld->dpctl->cpupload_step2(
					(void *)arg, dpld->dpctl);
			if (err < 0) {
				lnk_err(dpld, "cpupload_step2 fail\n");
				goto exit;
			}
		}
		break;
	case IOCTL_DPRAM_INIT_STATUS:
		lnk_dbg(dpld, "get dpram init status\n");
		return dpld->dpram_init_status;
	default:
		break;
	}

	return 0;

exit:
	return err;
}

static void dpram_table_init(struct dpram_link_device *dpld)
{
	struct link_device *ld = NULL;
	u8 __iomem *dp_base = NULL;

	if (!dpld) {
		pr_err("[LNK/E] <%s> dpld == NULL\n", __func__);
		return;
	}

	if (!dpld->dp_base) {
		pr_err("[LNK/E] <%s> dpld->dp_base == NULL\n", __func__);
		return;
	}

	ld = &dpld->ld;
	dp_base = dpld->dp_base;

	/* Map for booting */
	if (ld->mdm_data->modem_type == SEC_CMC221) {
		dpld->bt_map.buff = (u8 *)(dp_base);
		dpld->bt_map.req  = (u32 *)(dp_base + DP_BOOT_REQ_OFFSET);
		dpld->bt_map.resp = (u32 *)(dp_base + DP_BOOT_RESP_OFFSET);
		dpld->bt_map.size = dpld->dp_size;
	} else if (ld->mdm_data->modem_type == QC_MDM6600) {
		if (dpld->dpctl->bt_map_init)
			dpld->dpctl->bt_map_init(dpld->dpctl);
	} else {
		dpld->bt_map.buff = (u8 *)(dp_base);
		dpld->bt_map.req  = (u32 *)(dp_base + DP_BOOT_REQ_OFFSET);
		dpld->bt_map.resp = (u32 *)(dp_base + DP_BOOT_RESP_OFFSET);
		dpld->bt_map.size = dpld->dp_size - 4;
	}

	/* Map for download (FOTA, GOTA, etc.) */
	if (ld->mdm_data->modem_type == SEC_CMC221) {
		dpld->dl_map.magic = (u32 *)(dp_base);
		dpld->dl_map.buff  = (u8 *)(dp_base + DP_DLOAD_BUFF_OFFSET);
	}

	/* Map for upload mode */
	dpld->ul_map.magic = (u32 *)(dp_base);
	if (ld->mdm_data->modem_type == SEC_CMC221)
		dpld->ul_map.buff = (u8 *)(dp_base);
	else
		dpld->ul_map.buff = (u8 *)(dp_base + DP_ULOAD_BUFF_OFFSET);
}

struct link_device *dpram_create_link_device(struct platform_device *pdev)
{
	struct dpram_link_device *dpld = NULL;
	struct link_device *ld = NULL;
	struct modem_data *pdata = NULL;
	int ret = 0;
	int i = 0;

	/* Get the platform data */
	pdata = (struct modem_data *)pdev->dev.platform_data;
	if (!pdata) {
		pr_err("[LNK/E] <%s> pdata == NULL\n", __func__);
		goto err;
	}
	if (!pdata->dpram_ctl) {
		pr_err("[LNK/E] <%s> pdata->dpram_ctl == NULL\n", __func__);
		goto err;
	}
	pr_err("[LNK] <%s> link device = %s\n", __func__, pdata->link_name);
	pr_err("[LNK] <%s> modem = %s\n", __func__, pdata->name);

	/* Alloc DPRAM link device structure */
	dpld = kzalloc(sizeof(struct dpram_link_device), GFP_KERNEL);
	if (!dpld) {
		pr_err("[LNK/E] <%s> Failed to kzalloc()\n", __func__);
		goto err;
	}
	ld = &dpld->ld;

	/* Extract modem data and DPRAM control data from the platform data */
	ld->mdm_data = pdata;
	ld->name     = pdata->link_name;
	dpld->dpctl  = pdata->dpram_ctl;

	/* Set attributes as a link device */
	ld->aligned = dpld->dpctl->aligned;
	if (ld->aligned)
		pr_err("[LNK] <%s> Aligned access is required!!!\n", __func__);

	ld->send         = dpram_send;
	ld->gota_start   = dpram_set_dl_magic;
	ld->force_dump   = dpram_force_dump;
	ld->dump_start   = dpram_set_ul_magic;
	ld->modem_update = dpram_modem_update;
	ld->dump_update  = dpram_dump_update;
	ld->ioctl        = dpram_link_ioctl;

	INIT_LIST_HEAD(&ld->list);

	skb_queue_head_init(&ld->sk_fmt_tx_q);
	skb_queue_head_init(&ld->sk_raw_tx_q);
	skb_queue_head_init(&ld->sk_rfs_tx_q);
	ld->skb_txq[IPC_FMT] = &ld->sk_fmt_tx_q;
	ld->skb_txq[IPC_RAW] = &ld->sk_raw_tx_q;
	ld->skb_txq[IPC_RFS] = &ld->sk_rfs_tx_q;

	INIT_DELAYED_WORK(&ld->tx_delayed_work, dpram_tx_work);

	/* Set attributes as a dpram link device */
	dpld->dp_base     = dpld->dpctl->dp_base;
	dpld->dp_size     = dpld->dpctl->dp_size;
	dpld->max_ipc_dev = dpld->dpctl->max_ipc_dev;

	dpld->irq = dpld->dpctl->dpram_irq;
	if (dpld->irq < 0) {
		pr_err("[LNK/E] <%s> Failed to get IRQ\n", __func__);
		goto err;
	}
	pr_err("[LNK] <%s> DPRAM IRQ# = %d\n", __func__, dpld->irq);

	wake_lock_init(&dpld->dpram_wake_lock,
		       WAKE_LOCK_SUSPEND,
		       dpld->dpctl->dpram_wlock_name);

	init_completion(&dpld->dpram_init_cmd);
	init_completion(&dpld->modem_pif_init_done);
	init_completion(&dpld->gota_start_complete);
	init_completion(&dpld->gota_send_done);
	init_completion(&dpld->gota_recv_done);
	init_completion(&dpld->gota_update_done);
	init_completion(&dpld->dump_start_complete);
	init_completion(&dpld->dump_recv_done);

	INIT_WORK(&dpld->rx_work, dpram_rx_work);
	for (i = 0; i < dpld->max_ipc_dev; i++)
		skb_queue_head_init(&dpld->skb_rxq[i]);

	/* Initialize DPRAM map (physical map -> logical map) */
	dpram_table_init(dpld);

	if (ld->mdm_data->modem_type == QC_MDM6600) {
		if (dpld->dpctl->load_init)
			dpld->dpctl->load_init(dpld->dpctl);
	}

	/* Disable IPC */
	dpld->dpctl->set_magic(0);
	dpld->dpctl->set_access(0);
	dpld->dpram_init_status = DPRAM_INIT_STATE_NONE;

	/* Prepare a clean buffer for DPRAM access */
	dpld->buff = kzalloc(dpld->dp_size, GFP_KERNEL);
	if (!dpld->buff) {
		pr_err("[LNK/E] <%s> Failed to alloc dpld->buff\n", __func__);
		goto err;
	}

	/* Register DPRAM interrupt handler */
	ret = dpram_register_isr(dpld->irq,
				 dpram_irq_handler,
				 dpld->dpctl->dpram_irq_flags,
				 dpld->dpctl->dpram_irq_name,
				 ld);
	if (ret)
		goto err;

	return ld;

err:
	kfree(dpld->buff);
	kfree(dpld);
	return NULL;
}

