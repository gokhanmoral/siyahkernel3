/*
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
#include <linux/kallsyms.h>
#include <linux/platform_data/modem.h>

#include "modem_prj.h"
#include "modem_link_device_dpram.h"
#include "modem_utils.h"

static struct dpram_rxb *rxbq_create_pool(unsigned size, int count)
{
	struct dpram_rxb *rxb;
	u8 *buff;
	int i;

	rxb = kzalloc(sizeof(struct dpram_rxb) * count, GFP_KERNEL);
	if (!rxb) {
		mif_info("ERR! kzalloc rxb fail\n");
		return NULL;
	}

	buff = kzalloc((size * count), GFP_KERNEL|GFP_DMA);
	if (!buff) {
		mif_info("ERR! kzalloc buff fail\n");
		kfree(rxb);
		return NULL;
	}

	for (i = 0; i < count; i++) {
		rxb[i].buff = buff;
		rxb[i].size = size;
		buff += size;
	}

	return rxb;
}

static inline unsigned rxbq_get_page_size(unsigned len)
{
	return ((len + PAGE_SIZE - 1) >> PAGE_SHIFT) << PAGE_SHIFT;
}

static inline bool rxbq_empty(struct dpram_rxb_queue *rxbq)
{
	return (rxbq->in == rxbq->out) ? true : false;
}

static inline int rxbq_free_size(struct dpram_rxb_queue *rxbq)
{
	int in = rxbq->in;
	int out = rxbq->out;
	int qsize = rxbq->size;
	return (in < out) ? (out - in - 1) : (qsize + out - in - 1);
}

static inline struct dpram_rxb *rxbq_get_free_rxb(struct dpram_rxb_queue *rxbq)
{
	struct dpram_rxb *rxb = NULL;

	if (likely(rxbq_free_size(rxbq) > 0)) {
		rxb = &rxbq->rxb[rxbq->in];
		rxbq->in++;
		if (rxbq->in >= rxbq->size)
			rxbq->in -= rxbq->size;
		rxb->data = rxb->buff;
	}

	return rxb;
}

static inline int rxbq_size(struct dpram_rxb_queue *rxbq)
{
	int in = rxbq->in;
	int out = rxbq->out;
	int qsize = rxbq->size;
	return (in >= out) ? (in - out) : (qsize - out + in);
}

static inline struct dpram_rxb *rxbq_get_data_rxb(struct dpram_rxb_queue *rxbq)
{
	struct dpram_rxb *rxb = NULL;

	if (likely(!rxbq_empty(rxbq))) {
		rxb = &rxbq->rxb[rxbq->out];
		rxbq->out++;
		if (rxbq->out >= rxbq->size)
			rxbq->out -= rxbq->size;
	}

	return rxb;
}

static inline u8 *rxb_put(struct dpram_rxb *rxb, unsigned len)
{
	rxb->len = len;
	return rxb->data;
}

static inline void rxb_clear(struct dpram_rxb *rxb)
{
	rxb->data = NULL;
	rxb->len = 0;
}

static int dpram_register_isr(unsigned irq, irqreturn_t (*isr)(int, void*),
				unsigned long flag, const char *name,
				struct dpram_link_device *dpld)
{
	int ret = 0;

	ret = request_irq(irq, isr, flag, name, dpld);
	if (ret) {
		mif_info("%s: ERR! request_irq fail (err %d)\n", name, ret);
		return ret;
	}

	ret = enable_irq_wake(irq);
	if (ret)
		mif_info("%s: ERR! enable_irq_wake fail (err %d)\n", name, ret);

	mif_info("%s (#%d) handler registered\n", name, irq);

	return 0;
}

static inline void clear_intr(struct dpram_link_device *dpld)
{
	if (dpld->dpctl->clear_intr)
		dpld->dpctl->clear_intr();
}

static inline u16 recv_intr(struct dpram_link_device *dpld)
{
	if (dpld->dpctl->recv_intr)
		return dpld->dpctl->recv_intr();
	else
		return ioread16(dpld->mbx2ap);
}

static inline void send_intr(struct dpram_link_device *dpld, u16 mask)
{
	if (dpld->dpctl->send_intr)
		dpld->dpctl->send_intr(mask);
	else
		iowrite16(mask, dpld->mbx2cp);
}

static inline u16 get_magic(struct dpram_link_device *dpld)
{
	return ioread16(dpld->magic);
}

static inline void set_magic(struct dpram_link_device *dpld, u16 value)
{
	iowrite16(value, dpld->magic);
}

static inline u16 get_access(struct dpram_link_device *dpld)
{
	return ioread16(dpld->access);
}

static inline void set_access(struct dpram_link_device *dpld, u16 value)
{
	iowrite16(value, dpld->access);
}

static inline u32 get_tx_head(struct dpram_link_device *dpld, int id)
{
	return ioread16(dpld->dev[id]->txq.head);
}

static inline u32 get_tx_tail(struct dpram_link_device *dpld, int id)
{
	return ioread16(dpld->dev[id]->txq.tail);
}

static inline void set_tx_head(struct dpram_link_device *dpld, int id, u32 head)
{
	int cnt = 100;
	u32 val = 0;

	iowrite16((u16)head, dpld->dev[id]->txq.head);

	do {
		/* Check head value written */
		val = ioread16(dpld->dev[id]->txq.head);
		if (likely(val == head))
			break;

		mif_info("ERR: txq.head(%d) != head(%d)\n", val, head);

		/* Write head value again */
		iowrite16((u16)head, dpld->dev[id]->txq.head);
	} while (cnt--);
}

static inline void set_tx_tail(struct dpram_link_device *dpld, int id, u32 tail)
{
	int cnt = 100;
	u32 val = 0;

	iowrite16((u16)tail, dpld->dev[id]->txq.tail);

	do {
		/* Check tail value written */
		val = ioread16(dpld->dev[id]->txq.tail);
		if (likely(val == tail))
			break;

		mif_info("ERR: txq.tail(%d) != tail(%d)\n", val, tail);

		/* Write tail value again */
		iowrite16((u16)tail, dpld->dev[id]->txq.tail);
	} while (cnt--);
}

static inline u8 *get_tx_buff(struct dpram_link_device *dpld, int id)
{
	return dpld->dev[id]->txq.buff;
}

static inline u32 get_tx_buff_size(struct dpram_link_device *dpld, int id)
{
	return dpld->dev[id]->txq.size;
}

static inline u32 get_rx_head(struct dpram_link_device *dpld, int id)
{
	return ioread16(dpld->dev[id]->rxq.head);
}

static inline u32 get_rx_tail(struct dpram_link_device *dpld, int id)
{
	return ioread16(dpld->dev[id]->rxq.tail);
}

static inline void set_rx_head(struct dpram_link_device *dpld, int id, u32 head)
{
	int cnt = 100;
	u32 val = 0;

	iowrite16((u16)head, dpld->dev[id]->rxq.head);

	do {
		/* Check head value written */
		val = ioread16(dpld->dev[id]->rxq.head);
		if (val == head)
			break;

		mif_info("ERR: rxq.head(%d) != head(%d)\n", val, head);

		/* Write head value again */
		iowrite16((u16)head, dpld->dev[id]->rxq.head);
	} while (cnt--);
}

static inline void set_rx_tail(struct dpram_link_device *dpld, int id, u32 tail)
{
	int cnt = 100;
	u32 val = 0;

	iowrite16((u16)tail, dpld->dev[id]->rxq.tail);

	do {
		/* Check tail value written */
		val = ioread16(dpld->dev[id]->rxq.tail);
		if (val == tail)
			break;

		mif_info("ERR: rxq.tail(%d) != tail(%d)\n", val, tail);

		/* Write tail value again */
		iowrite16((u16)tail, dpld->dev[id]->rxq.tail);
	} while (cnt--);
}

static inline u8 *get_rx_buff(struct dpram_link_device *dpld, int id)
{
	return dpld->dev[id]->rxq.buff;
}

static inline u32 get_rx_buff_size(struct dpram_link_device *dpld, int id)
{
	return dpld->dev[id]->rxq.size;
}

static inline u16 get_mask_req_ack(struct dpram_link_device *dpld, int id)
{
	return dpld->dev[id]->mask_req_ack;
}

static inline u16 get_mask_res_ack(struct dpram_link_device *dpld, int id)
{
	return dpld->dev[id]->mask_res_ack;
}

static inline u16 get_mask_send(struct dpram_link_device *dpld, int id)
{
	return dpld->dev[id]->mask_send;
}

/*
** CAUTION : dpram_allow_sleep() MUST be invoked after dpram_wake_up() success
*/
static int dpram_wake_up(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;

	set_sromc_access(true);

	if (!dpld->dpctl->wakeup)
		return 0;

	if (dpld->dpctl->wakeup() < 0) {
		mif_info("%s: ERR! <%pF> DPRAM wakeup fail\n",
			ld->name, __builtin_return_address(0));
		set_sromc_access(false);
		return -EACCES;
	}

	atomic_inc(&dpld->accessing);
	return 0;
}

static void dpram_allow_sleep(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;

	if (!dpld->dpctl->sleep) {
		set_sromc_access(false);
		return;
	}

	if (atomic_dec_return(&dpld->accessing) <= 0) {
		dpld->dpctl->sleep();
		atomic_set(&dpld->accessing, 0);
		set_sromc_access(false);
		mif_debug("%s: DPRAM sleep possible\n", ld->name);
	}
}

static int dpram_check_access(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;
	int i;
	u16 magic = get_magic(dpld);
	u16 access = get_access(dpld);

	if (likely(magic == DPRAM_MAGIC_CODE && access == 1))
		return 0;

	for (i = 1; i <= 10; i++) {
		mif_info("%s: ERR! magic:%X access:%X -> retry:%d\n",
			ld->name, magic, access, i);
		mdelay(1);

		magic = get_magic(dpld);
		access = get_access(dpld);
		if (likely(magic == DPRAM_MAGIC_CODE && access == 1))
			return 0;
	}

	mif_info("%s: !CRISIS! magic:%X access:%X\n", ld->name, magic, access);
	return -EACCES;
}

static bool dpram_ipc_active(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;

	/* Check DPRAM mode */
	if (ld->mode != LINK_MODE_IPC) {
		mif_info("%s: ERR! <%pF> ld->mode != LINK_MODE_IPC\n",
			ld->name, __builtin_return_address(0));
		return false;
	}

	if (dpram_check_access(dpld) < 0) {
		mif_info("%s: ERR! <%pF> dpram_check_access fail\n",
			ld->name, __builtin_return_address(0));
		return false;
	}

	return true;
}

static inline bool dpram_circ_valid(u32 size, u32 in, u32 out)
{
	if (in >= size)
		return false;

	if (out >= size)
		return false;

	return true;
}

/* get free space in the TXQ as well as in & out pointers */
static inline int get_txq_space(struct dpram_link_device *dpld, int dev,
		u32 qsize, u32 *in, u32 *out)
{
	struct link_device *ld = &dpld->ld;

	*in = get_tx_head(dpld, dev);
	*out = get_tx_tail(dpld, dev);

	if (!dpram_circ_valid(qsize, *in, *out)) {
		mif_info("%s: ERR! <%pF> "
			"%s_TXQ invalid (size:%d in:%d out:%d)\n",
			ld->name, __builtin_return_address(0),
			get_dev_name(dev), qsize, *in, *out);
		set_tx_head(dpld, dev, 0);
		set_tx_tail(dpld, dev, 0);
		*in = 0;
		*out = 0;
		return -EINVAL;
	}

	return (*in < *out) ? (*out - *in - 1) : (qsize + *out - *in - 1);
}

static void set_dpram_map(struct dpram_link_device *dpld,
	struct mif_irq_map *map)
{
	map->magic = get_magic(dpld);
	map->access = get_access(dpld);

	map->fmt_tx_in = get_tx_head(dpld, IPC_FMT);
	map->fmt_tx_out = get_tx_tail(dpld, IPC_FMT);
	map->fmt_rx_in = get_rx_head(dpld, IPC_FMT);
	map->fmt_rx_out = get_rx_tail(dpld, IPC_FMT);
	map->raw_tx_in = get_tx_head(dpld, IPC_RAW);
	map->raw_tx_out = get_tx_tail(dpld, IPC_RAW);
	map->raw_rx_in = get_rx_head(dpld, IPC_RAW);
	map->raw_rx_out = get_rx_tail(dpld, IPC_RAW);

	map->cp2ap = recv_intr(dpld);
}

static void dpram_ipc_write(struct dpram_link_device *dpld, int dev,
		u32 qsize, u32 in, u32 out, struct sk_buff *skb)
{
	struct link_device *ld = &dpld->ld;
	u8 __iomem *dst = get_tx_buff(dpld, dev);
	u8 *src = skb->data;
	u32 len = skb->len;
	struct mif_irq_map map;

	/* check queue status */
	mif_debug("%s: {FMT %u %u %u %u} {RAW %u %u %u %u} ...\n", ld->name,
		get_tx_head(dpld, IPC_FMT), get_tx_tail(dpld, IPC_FMT),
		get_rx_head(dpld, IPC_FMT), get_rx_tail(dpld, IPC_FMT),
		get_tx_head(dpld, IPC_RAW), get_tx_tail(dpld, IPC_RAW),
		get_rx_head(dpld, IPC_RAW), get_rx_tail(dpld, IPC_RAW));

	if (dev == IPC_FMT) {
		set_dpram_map(dpld, &map);
		mif_irq_log(ld->mc->msd, map, "ipc_write", sizeof("ipc_write"));
		mif_ipc_log(MIF_IPC_AP2CP, ld->mc->msd, skb->data, skb->len);
/*
		mif_info("%s: [M:0x%X|A:%u] F[TI:%u TO:%u | RI:%u RO:%u] "
			"R[TI:%u TO:%u | RI:%u RO:%u]\n",
			ld->name, get_magic(dpld), get_access(dpld),
			get_tx_head(dpld, IPC_FMT), get_tx_tail(dpld, IPC_FMT),
			get_rx_head(dpld, IPC_FMT), get_rx_tail(dpld, IPC_FMT),
			get_tx_head(dpld, IPC_RAW), get_tx_tail(dpld, IPC_RAW),
			get_rx_head(dpld, IPC_RAW), get_rx_tail(dpld, IPC_RAW));
		pr_ipc(__func__, src, (len > 16 ? 16 : len));
*/
	}

	if (in < out) {
		/* +++++++++ in ---------- out ++++++++++ */
		memcpy((dst + in), src, len);
	} else {
		/* ------ out +++++++++++ in ------------ */
		u32 space = qsize - in;

		/* 1) in -> buffer end */
		memcpy((dst + in), src, ((len > space) ? space : len));

		/* 2) buffer start -> out */
		if (len > space)
			memcpy(dst, (src + space), (len - space));
	}

	/* update new in pointer */
	in += len;
	if (in >= qsize)
		in -= qsize;
	set_tx_head(dpld, dev, in);
}

static int dpram_try_ipc_tx(struct dpram_link_device *dpld, int dev)
{
	struct link_device *ld = &dpld->ld;
	struct sk_buff_head *txq = ld->skb_txq[dev];
	struct sk_buff *skb;
	u32 qsize = get_tx_buff_size(dpld, dev);
	u32 in;
	u32 out;
	int space;
	int copied = 0;
	u16 mask = 0;

	while (1) {
		skb = skb_dequeue(txq);
		if (unlikely(!skb))
			break;

		space = get_txq_space(dpld, dev, qsize, &in, &out);
		if (unlikely(space < 0)) {
			skb_queue_head(txq, skb);
			return -ENOSPC;
		}

		if (unlikely(space < skb->len)) {
			atomic_set(&dpld->res_required[dev], 1);
			skb_queue_head(txq, skb);
			mask = get_mask_req_ack(dpld, dev);
			mif_info("%s: %s "
				"qsize[%u] in[%u] out[%u] free[%u] < len[%u]\n",
				ld->name, get_dev_name(dev),
				qsize, in, out, space, skb->len);
			break;
		}

		/* TX if there is enough room in the queue
		*/
		mif_debug("%s: %s "
			"qsize[%u] in[%u] out[%u] free[%u] >= len[%u]\n",
			ld->name, get_dev_name(dev),
			qsize, in, out, space, skb->len);

		dpram_ipc_write(dpld, dev, qsize, in, out, skb);

		copied += skb->len;

		dev_kfree_skb_any(skb);
	}

	if (mask)
		return -ENOSPC;
	else
		return copied;
}

static void dpram_trigger_crash(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;
	struct io_device *iod;
	int i;

	for (i = 0; i < dpld->max_ipc_dev; i++) {
		mif_info("%s: purging %s_skb_txq\b", ld->name, get_dev_name(i));
		skb_queue_purge(ld->skb_txq[i]);
	}

	iod = link_get_iod_with_format(ld, IPC_FMT);
	iod->modem_state_changed(iod, STATE_CRASH_EXIT);

	iod = link_get_iod_with_format(ld, IPC_BOOT);
	iod->modem_state_changed(iod, STATE_CRASH_EXIT);

	iod = link_get_iod_with_channel(ld, PS_DATA_CH_0);
	if (iod)
		iodevs_for_each(iod->msd, iodev_netif_stop, 0);
}

static int dpram_trigger_force_cp_crash(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;
	int ret;
	int cnt = 5000;

	ld->mode = LINK_MODE_ULOAD;
	mif_info("%s\n", ld->name);

	dpram_wake_up(dpld);

	send_intr(dpld, INT_CMD(INT_CMD_CRASH_EXIT));

	while (cnt--) {
		ret = try_wait_for_completion(&dpld->crash_start_complete);
		if (ret)
			break;
		udelay(1000);
	}

	if (!ret) {
		mif_info("%s: ERR! No CRASH_EXIT ACK from CP\n", ld->name);
		dpram_trigger_crash(dpld);
	}

	return 0;
}

static void dpram_ipc_rx_task(unsigned long data)
{
	struct link_device *ld;
	struct dpram_link_device *dpld;
	struct dpram_rxb *rxb;
	struct io_device *iod;
	u32 qlen;
	int i;

	dpld = (struct dpram_link_device *)data;
	ld = &dpld->ld;

	for (i = 0; i < dpld->max_ipc_dev; i++) {
		if (i == IPC_RAW)
			iod = link_get_iod_with_format(ld, IPC_MULTI_RAW);
		else
			iod = link_get_iod_with_format(ld, i);

		qlen = rxbq_size(&dpld->rxbq[i]);
		/*
		if (qlen > 0 && i == IPC_FMT) {
			mif_info("%s: [M:0x%X|A:%u] "
				"F[TI:%u TO:%u | RI:%u RO:%u] "
				"R[TI:%u TO:%u | RI:%u RO:%u]\n",
				ld->name,
				dpld->logbuff.magic, dpld->logbuff.access,
				dpld->logbuff.fmt_tx_in,
				dpld->logbuff.fmt_tx_out,
				dpld->logbuff.fmt_rx_in,
				dpld->logbuff.fmt_rx_out,
				dpld->logbuff.raw_tx_in,
				dpld->logbuff.raw_tx_out,
				dpld->logbuff.raw_rx_in,
				dpld->logbuff.raw_rx_out);
		}
		*/

		while (qlen > 0) {
			rxb = rxbq_get_data_rxb(&dpld->rxbq[i]);
			iod->recv(iod, ld, rxb->data, rxb->len);
			rxb_clear(rxb);
			qlen--;
		}
	}
}

static void dpram_ipc_read(struct dpram_link_device *dpld, int dev, u8 *dst,
	u8 __iomem *src, u32 out, u32 len, u32 qsize)
{
	if ((out + len) <= qsize) {
		/* ----- (out)         (in) ----- */
		/* -----   7f 00 00 7e      ----- */
		memcpy(dst, (src + out), len);
	} else {
		/*       (in) ----------- (out)   */
		/* 00 7e      -----------   7f 00 */
		unsigned len1 = qsize - out;

		/* 1) out -> buffer end */
		memcpy(dst, (src + out), len1);

		/* 2) buffer start -> in */
		dst += len1;
		memcpy(dst, src, (len - len1));
	}
}

static void log_dpram_irq(struct dpram_link_device *dpld, u16 int2ap)
{
	struct sk_buff *skb;
	struct mif_event_buff *evtb;
	struct dpram_irq_buff *irqb;
	struct link_device *ld = &dpld->ld;

	skb = alloc_skb(MAX_MIF_EVT_BUFF_SIZE, GFP_ATOMIC);
	if (!skb)
		return;

	evtb = (struct mif_event_buff *)skb_put(skb, MAX_MIF_EVT_BUFF_SIZE);
	memset(evtb, 0, MAX_MIF_EVT_BUFF_SIZE);

	do_gettimeofday(&evtb->tv);
	evtb->evt = MIF_IRQ_EVT;

	strncpy(evtb->mc, ld->mc->name, MAX_MIF_NAME_LEN);
	strncpy(evtb->ld, ld->name, MAX_MIF_NAME_LEN);
	evtb->link_type = ld->link_type;

	irqb = &evtb->dpram_irqb;

	irqb->magic = get_magic(dpld);
	irqb->access = get_access(dpld);

	irqb->qsp[IPC_FMT].txq.in = get_tx_head(dpld, IPC_FMT);
	irqb->qsp[IPC_FMT].txq.out = get_tx_tail(dpld, IPC_FMT);
	irqb->qsp[IPC_FMT].rxq.in = get_rx_head(dpld, IPC_FMT);
	irqb->qsp[IPC_FMT].rxq.out = get_rx_tail(dpld, IPC_FMT);

	irqb->qsp[IPC_RAW].txq.in = get_tx_head(dpld, IPC_RAW);
	irqb->qsp[IPC_RAW].txq.out = get_tx_tail(dpld, IPC_RAW);
	irqb->qsp[IPC_RAW].rxq.in = get_rx_head(dpld, IPC_RAW);
	irqb->qsp[IPC_RAW].rxq.out = get_rx_tail(dpld, IPC_RAW);

	irqb->int2ap = int2ap;

	evtb->rcvd = sizeof(struct dpram_irq_buff);
	evtb->len = sizeof(struct dpram_irq_buff);

#if 0
	mif_irq_log(ld->mc, skb);
	mif_flush_logs(ld->mc);
#endif
}

/*
  ret < 0  : error
  ret == 0 : no data
  ret > 0  : valid data
*/
static int dpram_ipc_recv_data(struct dpram_link_device *dpld, int dev,
				u16 non_cmd)
{
	struct link_device *ld = &dpld->ld;
	struct dpram_rxb *rxb;
	u8 __iomem *src = get_rx_buff(dpld, dev);
	u32 qsize = get_rx_buff_size(dpld, dev);
	u32 in = get_rx_head(dpld, dev);
	u32 out = get_rx_tail(dpld, dev);
	u32 rcvd = 0;
	struct mif_irq_map map;

	if (in == out)
		return 0;

	if (dev == IPC_FMT) {
		set_dpram_map(dpld, &map);
		mif_irq_log(ld->mc->msd, map, "ipc_recv", sizeof("ipc_recv"));
		/*
		dpld->logbuff.magic = get_magic(dpld);
		dpld->logbuff.access = get_access(dpld);

		dpld->logbuff.fmt_tx_in = get_tx_head(dpld, IPC_FMT);
		dpld->logbuff.fmt_tx_out = get_tx_tail(dpld, IPC_FMT);
		dpld->logbuff.fmt_rx_in = get_rx_head(dpld, IPC_FMT);
		dpld->logbuff.fmt_rx_out = get_rx_tail(dpld, IPC_FMT);

		dpld->logbuff.raw_tx_in = get_tx_head(dpld, IPC_RAW);
		dpld->logbuff.raw_tx_out = get_tx_tail(dpld, IPC_RAW);
		dpld->logbuff.raw_rx_in = get_rx_head(dpld, IPC_RAW);
		dpld->logbuff.raw_rx_out = get_rx_tail(dpld, IPC_RAW);
		*/
	}

	/* Get data length in DPRAM*/
	rcvd = (in > out) ? (in - out) : (qsize - out + in);

	mif_debug("%s: %s qsize[%u] in[%u] out[%u] rcvd[%u]\n",
		ld->name, get_dev_name(dev), qsize, in, out, rcvd);

	/* Check each queue */
	if (!dpram_circ_valid(qsize, in, out)) {
		mif_info("%s: ERR! %s_RXQ invalid (size:%d in:%d out:%d)\n",
			ld->name, get_dev_name(dev), qsize, in, out);
		set_rx_head(dpld, dev, 0);
		set_rx_tail(dpld, dev, 0);
		return -EINVAL;
	}

	/* Allocate an rxb */
	rxb = rxbq_get_free_rxb(&dpld->rxbq[dev]);
	if (!rxb) {
		mif_info("%s: ERR! %s rxbq_get_free_rxb fail\n",
			ld->name, get_dev_name(dev));
		return -ENOMEM;
	}

	/* Read data from each DPRAM buffer */
	dpram_ipc_read(dpld, dev, rxb_put(rxb, rcvd), src, out, rcvd, qsize);

	/* Calculate and set new out */
	out += rcvd;
	if (out >= qsize)
		out -= qsize;
	set_rx_tail(dpld, dev, out);

	return rcvd;
}

static void dpram_purge_rx_circ(struct dpram_link_device *dpld, int dev)
{
	u32 in = get_rx_head(dpld, dev);
	set_rx_tail(dpld, dev, in);
}

static void non_command_handler(struct dpram_link_device *dpld, u16 non_cmd)
{
	struct link_device *ld = &dpld->ld;
	int i;
	int ret = 0;
	u32 in;
	u32 out;
	u16 mask = 0;
	u16 req_mask = 0;
	u16 tx_mask = 0;

	if (!dpram_ipc_active(dpld))
		return;

	/* Read data from DPRAM */
	for (i = 0; i < dpld->max_ipc_dev; i++) {
		ret = dpram_ipc_recv_data(dpld, i, non_cmd);
		if (ret < 0)
			dpram_purge_rx_circ(dpld, i);

		/* Check and process REQ_ACK (at this time, in == out) */
		if (non_cmd & get_mask_req_ack(dpld, i)) {
			mif_debug("%s: send %s_RES_ACK\n",
				ld->name, get_dev_name(i));
			mask = get_mask_res_ack(dpld, i);
			send_intr(dpld, INT_NON_CMD(mask));
		}
	}

	/* Schedule soft IRQ for RX */
	tasklet_hi_schedule(&dpld->rx_tsk);

	/* Try TX via DPRAM */
	for (i = 0; i < dpld->max_ipc_dev; i++) {
		if (atomic_read(&dpld->res_required[i]) > 0) {
			in = get_tx_head(dpld, i);
			out = get_tx_tail(dpld, i);
			if (likely(in == out)) {
				ret = dpram_try_ipc_tx(dpld, i);
				if (ret > 0) {
					atomic_set(&dpld->res_required[i], 0);
					tx_mask |= get_mask_send(dpld, i);
				} else {
					req_mask |= get_mask_req_ack(dpld, i);
				}
			} else {
				req_mask |= get_mask_req_ack(dpld, i);
			}
		}
	}

	if (req_mask || tx_mask) {
		tx_mask |= req_mask;
		send_intr(dpld, INT_NON_CMD(tx_mask));
		mif_debug("%s: send intr 0x%04X\n", ld->name, tx_mask);
	}
}

static int dpram_init_ipc(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;
	int i;

	if (ld->mode == LINK_MODE_IPC &&
	    get_magic(dpld) == DPRAM_MAGIC_CODE &&
	    get_access(dpld) == 1)
		mif_info("%s: IPC already initialized\n", ld->name);

	/* Clear pointers in every circular queue */
	for (i = 0; i < dpld->max_ipc_dev; i++) {
		set_tx_head(dpld, i, 0);
		set_tx_tail(dpld, i, 0);
		set_rx_head(dpld, i, 0);
		set_rx_tail(dpld, i, 0);
	}

	/* Enable IPC */
	set_magic(dpld, DPRAM_MAGIC_CODE);
	set_access(dpld, 1);
	if (get_magic(dpld) != DPRAM_MAGIC_CODE || get_access(dpld) != 1)
		return -EACCES;

	ld->mode = LINK_MODE_IPC;

	for (i = 0; i < dpld->max_ipc_dev; i++) {
		spin_lock_init(&dpld->tx_lock[i]);
		atomic_set(&dpld->res_required[i], 0);
	}

	atomic_set(&dpld->accessing, 0);

	return 0;
}

static void cmd_req_active_handler(struct dpram_link_device *dpld)
{
	send_intr(dpld, INT_CMD(INT_CMD_RES_ACTIVE));
}

static void cmd_crash_reset_handler(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;
	struct io_device *iod = NULL;

	ld->mode = LINK_MODE_ULOAD;
	mif_info("%s: Recv 0xC7 (CRASH_RESET)\n", ld->name);

	iod = link_get_iod_with_format(ld, IPC_FMT);
	iod->modem_state_changed(iod, STATE_CRASH_RESET);

	iod = link_get_iod_with_format(ld, IPC_BOOT);
	iod->modem_state_changed(iod, STATE_CRASH_RESET);
}

static void cmd_crash_exit_handler(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;

	ld->mode = LINK_MODE_ULOAD;
	mif_info("%s: Recv 0xC9 (CRASH_EXIT)\n", ld->name);

	dpram_wake_up(dpld);

	complete_all(&dpld->crash_start_complete);

	if (dpld->ext_op && dpld->ext_op->log_disp)
		dpld->ext_op->log_disp(dpld);

	dpram_trigger_crash(dpld);
}

static void cmd_phone_start_handler(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;
	struct io_device *iod = NULL;

	mif_info("%s: Recv 0xC8 (CP_START)\n", ld->name);

	dpram_init_ipc(dpld);

	iod = link_get_iod_with_format(ld, IPC_FMT);
	if (!iod) {
		mif_info("%s: ERR! no iod\n", ld->name);
		return;
	}

	if (dpld->ext_op && dpld->ext_op->cp_start_handler)
		dpld->ext_op->cp_start_handler(dpld);

	if (ld->mc->phone_state != STATE_ONLINE) {
		mif_info("%s: phone_state: %d -> ONLINE\n",
			ld->name, ld->mc->phone_state);
		iod->modem_state_changed(iod, STATE_ONLINE);
	}

	mif_info("%s: Send 0xC2 (INIT_END)\n", ld->name);
	send_intr(dpld, INT_CMD(INT_CMD_INIT_END));
}

static void command_handler(struct dpram_link_device *dpld, u16 cmd)
{
	struct link_device *ld = &dpld->ld;

	switch (INT_CMD_MASK(cmd)) {
	case INT_CMD_REQ_ACTIVE:
		cmd_req_active_handler(dpld);
		break;

	case INT_CMD_CRASH_RESET:
		dpld->dpram_init_status = DPRAM_INIT_STATE_NONE;
		cmd_crash_reset_handler(dpld);
		break;

	case INT_CMD_CRASH_EXIT:
		dpld->dpram_init_status = DPRAM_INIT_STATE_NONE;
		cmd_crash_exit_handler(dpld);
		break;

	case INT_CMD_PHONE_START:
		dpld->dpram_init_status = DPRAM_INIT_STATE_READY;
		cmd_phone_start_handler(dpld);
		complete_all(&dpld->dpram_init_cmd);
		break;

	case INT_CMD_NV_REBUILDING:
		mif_info("%s: NV_REBUILDING\n", ld->name);
		break;

	case INT_CMD_PIF_INIT_DONE:
		complete_all(&dpld->modem_pif_init_done);
		break;

	case INT_CMD_SILENT_NV_REBUILDING:
		mif_info("%s: SILENT_NV_REBUILDING\n", ld->name);
		break;

	case INT_CMD_NORMAL_PWR_OFF:
		/*ToDo:*/
		/*kernel_sec_set_cp_ack()*/;
		break;

	case INT_CMD_REQ_TIME_SYNC:
	case INT_CMD_CP_DEEP_SLEEP:
	case INT_CMD_EMER_DOWN:
		break;

	default:
		mif_info("%s: unknown command 0x%04X\n", ld->name, cmd);
	}
}

static void ext_command_handler(struct dpram_link_device *dpld, u16 cmd)
{
	struct link_device *ld = &dpld->ld;
	u16 resp;

	switch (EXT_CMD_MASK(cmd)) {
	case EXT_CMD_SET_SPEED_LOW:
		if (dpld->dpctl->setup_speed) {
			dpld->dpctl->setup_speed(DPRAM_SPEED_LOW);
			resp = INT_EXT_CMD(EXT_CMD_SET_SPEED_LOW);
			send_intr(dpld, resp);
		}
		break;

	case EXT_CMD_SET_SPEED_MID:
		if (dpld->dpctl->setup_speed) {
			dpld->dpctl->setup_speed(DPRAM_SPEED_MID);
			resp = INT_EXT_CMD(EXT_CMD_SET_SPEED_MID);
			send_intr(dpld, resp);
		}
		break;

	case EXT_CMD_SET_SPEED_HIGH:
		if (dpld->dpctl->setup_speed) {
			dpld->dpctl->setup_speed(DPRAM_SPEED_HIGH);
			resp = INT_EXT_CMD(EXT_CMD_SET_SPEED_HIGH);
			send_intr(dpld, resp);
		}
		break;

	default:
		mif_info("%s: unknown command 0x%04X\n", ld->name, cmd);
		break;
	}
}

static void udl_command_handler(struct dpram_link_device *dpld, u16 cmd)
{
	struct link_device *ld = &dpld->ld;

	if (cmd & UDL_RESULT_FAIL) {
		mif_info("%s: ERR! Command failed: %04x\n", ld->name, cmd);
		return;
	}

	switch (UDL_CMD_MASK(cmd)) {
	case UDL_CMD_RECEIVE_READY:
		mif_debug("%s: Send CP-->AP RECEIVE_READY\n", ld->name);
		send_intr(dpld, CMD_IMG_START_REQ);
		break;
	default:
		complete_all(&dpld->udl_cmd_complete);
	}
}

static irqreturn_t dpram_irq_handler(int irq, void *data)
{
	struct dpram_link_device *dpld = (struct dpram_link_device *)data;
	struct link_device *ld = (struct link_device *)&dpld->ld;
	u16 int2ap = 0;

	if (!ld->mc || ld->mc->phone_state == STATE_OFFLINE)
		return IRQ_HANDLED;

	if (dpram_wake_up(dpld) < 0)
		return IRQ_HANDLED;

	int2ap = recv_intr(dpld);

	clear_intr(dpld);

	if (int2ap == INT_POWERSAFE_FAIL) {
		mif_info("%s: int2ap == INT_POWERSAFE_FAIL\n", ld->name);
		goto exit_isr;
	} else if (int2ap == 0x1234 || int2ap == 0xDBAB || int2ap == 0xABCD) {
		if (dpld->ext_op && dpld->ext_op->dload_cmd_handler) {
			dpld->ext_op->dload_cmd_handler(dpld, int2ap);
			goto exit_isr;
		}
	}

	if (UDL_CMD_VALID(int2ap))
		udl_command_handler(dpld, int2ap);
	else if (EXT_INT_VALID(int2ap) && EXT_CMD_VALID(int2ap))
		ext_command_handler(dpld, int2ap);
	else if (INT_CMD_VALID(int2ap))
		command_handler(dpld, int2ap);
	else if (INT_VALID(int2ap))
		non_command_handler(dpld, int2ap);
	else
		mif_info("%s: ERR! invalid intr 0x%04X\n", ld->name, int2ap);

exit_isr:
	dpram_allow_sleep(dpld);
	return IRQ_HANDLED;
}

static void dpram_send_ipc(struct link_device *ld, int dev,
			struct io_device *iod, struct sk_buff *skb)
{
	struct dpram_link_device *dpld = to_dpram_link_device(ld);
	struct sk_buff_head *txq;
	unsigned long int flags;
	int ret;
	u16 mask;

	if (iod->io_typ == IODEV_NET) {
		u8 *pkt = skb->data + get_sipc5_hdr_size(skb->data);
		if (unlikely(is_dns_packet(pkt))) {
			u8 str[64];
			snprintf(str, 64, "%s: %s: DNS", __func__, ld->name);
			pr_ipc(str, pkt, 20);
		}
	}

	if (unlikely(dev >= dpld->max_ipc_dev)) {
		mif_info("%s: ERR! dev %d >= max_ipc_dev(%s)\n",
			ld->name, dev, get_dev_name(dpld->max_ipc_dev));
		return;
	}

	if (dpram_wake_up(dpld) < 0) {
		dpram_trigger_force_cp_crash(dpld);
		return;
	}

	if (!dpram_ipc_active(dpld))
		goto exit;

	txq = ld->skb_txq[dev];
	if (txq->qlen > 1024) {
		mif_debug("%s: %s txq->qlen %d > 1024\n",
			ld->name, get_dev_name(dev), txq->qlen);
	}

	skb_queue_tail(txq, skb);

	if (atomic_read(&dpld->res_required[dev]) > 0) {
		mif_debug("%s: %s_TXQ is full\n", ld->name, get_dev_name(dev));
		goto exit;
	}

	spin_lock_irqsave(&dpld->tx_lock[dev], flags);
	ret = dpram_try_ipc_tx(dpld, dev);
	if (ret > 0) {
		mask = get_mask_send(dpld, dev);
		send_intr(dpld, INT_NON_CMD(mask));
	} else {
		mask = get_mask_req_ack(dpld, dev);
		send_intr(dpld, INT_NON_CMD(mask));
		mif_info("%s: Send REQ_ACK 0x%04X\n", ld->name, mask);
	}
	spin_unlock_irqrestore(&dpld->tx_lock[dev], flags);

exit:
	dpram_allow_sleep(dpld);
}

static int dpram_send_binary(struct link_device *ld, struct sk_buff *skb)
{
	struct dpram_link_device *dpld = to_dpram_link_device(ld);

	if (dpld->ext_op && dpld->ext_op->download_skb)
		return dpld->ext_op->download_skb(dpld, skb);
	else
		return -ENODEV;
}

static int dpram_send(struct link_device *ld, struct io_device *iod,
		struct sk_buff *skb)
{
	enum dev_format fmt = iod->format;
	int len = skb->len;

	switch (fmt) {
	case IPC_FMT:
	case IPC_RAW:
	case IPC_RFS:
		if (likely(ld->mc->phone_state == STATE_ONLINE)) {
			dpram_send_ipc(ld, fmt, iod, skb);
		} else {
			mif_info("%s: phone_state != STATE_ONLINE\n", ld->name);
			dev_kfree_skb_any(skb);
		}
		return len;

	case IPC_BOOT:
		return dpram_send_binary(ld, skb);

	default:
		mif_info("%s: ERR! no TXQ for %s\n", ld->name, iod->name);
		dev_kfree_skb_any(skb);
		return -ENODEV;
	}
}

static int dpram_set_dload_magic(struct link_device *ld, struct io_device *iod)
{
	struct dpram_link_device *dpld = to_dpram_link_device(ld);

	ld->mode = LINK_MODE_DLOAD;

	iowrite32(DP_MAGIC_DMDL, dpld->dl_map.magic);

	return 0;
}

static int dpram_force_dump(struct link_device *ld, struct io_device *iod)
{
	struct dpram_link_device *dpld = to_dpram_link_device(ld);
	dpram_trigger_force_cp_crash(dpld);
	return 0;
}

static void dpram_dump_memory(struct link_device *ld, char *buff)
{
	struct dpram_link_device *dpld = to_dpram_link_device(ld);
	u8 __iomem *base = dpld->dpctl->dp_base;
	u32 size = dpld->dpctl->dp_size;

	dpram_wake_up(dpld);
	memcpy(buff, base, size);
}

static int dpram_dump_start(struct link_device *ld, struct io_device *iod)
{
	struct dpram_link_device *dpld = to_dpram_link_device(ld);

	if (dpld->ext_op && dpld->ext_op->dump_start)
		return dpld->ext_op->dump_start(dpld);
	else
		return -ENODEV;
}

static int dpram_dump_update(struct link_device *ld, struct io_device *iod,
		unsigned long arg)
{
	struct dpram_link_device *dpld = to_dpram_link_device(ld);

	if (dpld->ext_op && dpld->ext_op->dump_update)
		return dpld->ext_op->dump_update(dpld, (void *)arg);
	else
		return -ENODEV;
}

static int dpram_ioctl(struct link_device *ld, struct io_device *iod,
		unsigned int cmd, unsigned long arg)
{
	int err = -EFAULT;
	struct dpram_link_device *dpld = to_dpram_link_device(ld);

	mif_info("%s: cmd 0x%08X\n", ld->name, cmd);

	switch (cmd) {
	case IOCTL_DPRAM_SEND_BOOT:
		if (dpld->ext_op && dpld->ext_op->download_boot) {
			err = dpld->ext_op->download_boot(dpld, (void *)arg);
			if (err < 0) {
				mif_info("%s: ERR! download_boot fail\n",
					ld->name);
				goto exit;
			}
		}
		break;

	case IOCTL_DPRAM_PHONE_POWON:
		if (dpld->ext_op && dpld->ext_op->prepare_download) {
			err = dpld->ext_op->prepare_download(dpld);
			if (err < 0) {
				mif_info("%s: ERR! prepare_download fail\n",
					ld->name);
				goto exit;
			}
		}
		break;

	case IOCTL_DPRAM_PHONEIMG_LOAD:
		if (dpld->ext_op && dpld->ext_op->download_bin) {
			err = dpld->ext_op->download_bin(dpld, (void *)arg);
			if (err < 0) {
				mif_info("%s: ERR! download_bin fail\n",
					ld->name);
				goto exit;
			}
		}
		break;

	case IOCTL_DPRAM_NVDATA_LOAD:
		if (dpld->ext_op && dpld->ext_op->download_nv) {
			err = dpld->ext_op->download_nv(dpld, (void *)arg);
			if (err < 0) {
				mif_info("%s: ERR! download_nv fail\n",
					ld->name);
				goto exit;
			}
		}
		break;

	case IOCTL_DPRAM_PHONE_BOOTSTART:
		if (dpld->ext_op && dpld->ext_op->cp_boot_start) {
			err = dpld->ext_op->cp_boot_start(dpld);
			if (err < 0) {
				mif_info("%s: ERR! cp_boot_start fail\n",
					ld->name);
				goto exit;
			}
		}
		if (dpld->ext_op && dpld->ext_op->cp_boot_post_process) {
			err = dpld->ext_op->cp_boot_post_process(dpld);
			if (err < 0) {
				mif_info("%s: ERR! cp_boot_post_process fail\n",
					ld->name);
				goto exit;
			}
		}
		break;

	case IOCTL_DPRAM_PHONE_UPLOAD_STEP1:
		disable_irq_nosync(dpld->irq);

		if (dpld->ext_op && dpld->ext_op->upload_step1) {
			err = dpld->ext_op->upload_step1(dpld);
			if (err < 0) {
				clear_intr(dpld);
				enable_irq(dpld->irq);
				mif_info("%s: ERR! upload_step1 fail\n",
					ld->name);
				goto exit;
			}
		}
		break;

	case IOCTL_DPRAM_PHONE_UPLOAD_STEP2:
		if (dpld->ext_op->upload_step2) {
			err = dpld->ext_op->upload_step2(dpld, (void *)arg);
			if (err < 0) {
				clear_intr(dpld);
				enable_irq(dpld->irq);
				mif_info("%s: ERR! upload_step2 fail\n",
					ld->name);
				goto exit;
			}
		}
		break;

	case IOCTL_DPRAM_INIT_STATUS:
		mif_debug("%s: get dpram init status\n", ld->name);
		return dpld->dpram_init_status;

	case IOCTL_MODEM_DL_START:
		err = dpram_set_dload_magic(ld, iod);
		if (err < 0) {
			mif_info("%s: ERR! dpram_set_dload_magic fail\n",
				ld->name);
			goto exit;
		}

	default:
		break;
	}

	return 0;

exit:
	return err;
}

static void dpram_remap_std_16k_region(struct dpram_link_device *dpld)
{
	struct dpram_ipc_16k_map *dpram_map;
	struct dpram_ipc_device *dev;

	dpram_map = (struct dpram_ipc_16k_map *)dpld->dp_base;

	/* magic code and access enable fields */
	dpld->ipc_map.magic = (u16 __iomem *)&dpram_map->magic;
	dpld->ipc_map.access = (u16 __iomem *)&dpram_map->access;

	/* FMT */
	dev = &dpld->ipc_map.dev[IPC_FMT];

	strcpy(dev->name, "FMT");
	dev->id = IPC_FMT;

	dev->txq.head = (u16 __iomem *)&dpram_map->fmt_tx_head;
	dev->txq.tail = (u16 __iomem *)&dpram_map->fmt_tx_tail;
	dev->txq.buff = (u8 __iomem *)&dpram_map->fmt_tx_buff[0];
	dev->txq.size = DP_16K_FMT_TX_BUFF_SZ;

	dev->rxq.head = (u16 __iomem *)&dpram_map->fmt_rx_head;
	dev->rxq.tail = (u16 __iomem *)&dpram_map->fmt_rx_tail;
	dev->rxq.buff = (u8 __iomem *)&dpram_map->fmt_rx_buff[0];
	dev->rxq.size = DP_16K_FMT_RX_BUFF_SZ;

	dev->mask_req_ack = INT_MASK_REQ_ACK_F;
	dev->mask_res_ack = INT_MASK_RES_ACK_F;
	dev->mask_send    = INT_MASK_SEND_F;

	/* RAW */
	dev = &dpld->ipc_map.dev[IPC_RAW];

	strcpy(dev->name, "RAW");
	dev->id = IPC_RAW;

	dev->txq.head = (u16 __iomem *)&dpram_map->raw_tx_head;
	dev->txq.tail = (u16 __iomem *)&dpram_map->raw_tx_tail;
	dev->txq.buff = (u8 __iomem *)&dpram_map->raw_tx_buff[0];
	dev->txq.size = DP_16K_RAW_TX_BUFF_SZ;

	dev->rxq.head = (u16 __iomem *)&dpram_map->raw_rx_head;
	dev->rxq.tail = (u16 __iomem *)&dpram_map->raw_rx_tail;
	dev->rxq.buff = (u8 __iomem *)&dpram_map->raw_rx_buff[0];
	dev->rxq.size = DP_16K_RAW_RX_BUFF_SZ;

	dev->mask_req_ack = INT_MASK_REQ_ACK_R;
	dev->mask_res_ack = INT_MASK_RES_ACK_R;
	dev->mask_send    = INT_MASK_SEND_R;

	/* interrupt ports */
	dpld->ipc_map.mbx_cp2ap = (u16 __iomem *)&dpram_map->mbx_cp2ap;
	dpld->ipc_map.mbx_ap2cp = (u16 __iomem *)&dpram_map->mbx_ap2cp;
}

static int dpram_table_init(struct dpram_link_device *dpld)
{
	struct link_device *ld = &dpld->ld;
	u8 __iomem *dp_base;
	int i;

	if (!dpld->dp_base) {
		mif_info("%s: ERR! dpld->dp_base == NULL\n", ld->name);
		return -EINVAL;
	}
	dp_base = dpld->dp_base;

	/* Map for IPC */
	if (dpld->dpctl->ipc_map) {
		memcpy(&dpld->ipc_map, dpld->dpctl->ipc_map,
			sizeof(struct dpram_ipc_map));
	} else {
		if (dpld->dp_size == DPRAM_SIZE_16KB)
			dpram_remap_std_16k_region(dpld);
		else
			return -EINVAL;
	}

	dpld->magic = dpld->ipc_map.magic;
	dpld->access = dpld->ipc_map.access;
	for (i = 0; i < dpld->max_ipc_dev; i++)
		dpld->dev[i] = &dpld->ipc_map.dev[i];
	dpld->mbx2ap = dpld->ipc_map.mbx_cp2ap;
	dpld->mbx2cp = dpld->ipc_map.mbx_ap2cp;

	/* Map for booting */
	if (dpld->ext_op && dpld->ext_op->init_boot_map) {
		dpld->ext_op->init_boot_map(dpld);
	} else {
		dpld->bt_map.magic = (u32 *)(dp_base);
		dpld->bt_map.buff = (u8 *)(dp_base + DP_BOOT_BUFF_OFFSET);
		dpld->bt_map.size = dpld->dp_size - 8;
	}

	/* Map for download (FOTA, UDL, etc.) */
	if (dpld->ext_op && dpld->ext_op->init_dl_map) {
		dpld->ext_op->init_dl_map(dpld);
	} else {
		dpld->dl_map.magic = (u32 *)(dp_base);
		dpld->dl_map.buff = (u8 *)(dp_base + DP_DLOAD_BUFF_OFFSET);
	}

	/* Map for upload mode */
	if (dpld->ext_op && dpld->ext_op->init_ul_map) {
		dpld->ext_op->init_ul_map(dpld);
	} else {
		dpld->ul_map.magic = (u32 *)(dp_base);
		dpld->ul_map.buff = (u8 *)(dp_base + DP_ULOAD_BUFF_OFFSET);
	}

	return 0;
}

static void dpram_setup_common_op(struct dpram_link_device *dpld)
{
	dpld->clear_intr = clear_intr;
	dpld->recv_intr = recv_intr;
	dpld->send_intr = send_intr;
	dpld->get_magic = get_magic;
	dpld->set_magic = set_magic;
	dpld->get_access = get_access;
	dpld->set_access = set_access;
	dpld->get_tx_head = get_tx_head;
	dpld->get_tx_tail = get_tx_tail;
	dpld->set_tx_head = set_tx_head;
	dpld->set_tx_tail = set_tx_tail;
	dpld->get_tx_buff = get_tx_buff;
	dpld->get_tx_buff_size = get_tx_buff_size;
	dpld->get_rx_head = get_rx_head;
	dpld->get_rx_tail = get_rx_tail;
	dpld->set_rx_head = set_rx_head;
	dpld->set_rx_tail = set_rx_tail;
	dpld->get_rx_buff = get_rx_buff;
	dpld->get_rx_buff_size = get_rx_buff_size;
	dpld->get_mask_req_ack = get_mask_req_ack;
	dpld->get_mask_res_ack = get_mask_res_ack;
	dpld->get_mask_send = get_mask_send;
}

static int dpram_link_init(struct link_device *ld, struct io_device *iod)
{
	return 0;
}

static void dpram_link_terminate(struct link_device *ld, struct io_device *iod)
{
	return;
}

struct link_device *dpram_create_link_device(struct platform_device *pdev)
{
	struct modem_data *mdm_data = NULL;
	struct dpram_link_device *dpld = NULL;
	struct link_device *ld = NULL;
	struct resource *res = NULL;
	resource_size_t res_size;
	struct modemlink_dpram_control *dpctl = NULL;
	unsigned long task_data;
	int ret = 0;
	int i = 0;
	int bsize;
	int qsize;

	/* Get the platform data */
	mdm_data = (struct modem_data *)pdev->dev.platform_data;
	if (!mdm_data) {
		mif_info("ERR! mdm_data == NULL\n");
		goto err;
	}
	mif_info("modem = %s\n", mdm_data->name);
	mif_info("link device = %s\n", mdm_data->link_name);

	if (!mdm_data->dpram_ctl) {
		mif_info("ERR! mdm_data->dpram_ctl == NULL\n");
		goto err;
	}
	dpctl = mdm_data->dpram_ctl;

	/* Alloc DPRAM link device structure */
	dpld = kzalloc(sizeof(struct dpram_link_device), GFP_KERNEL);
	if (!dpld) {
		mif_info("ERR! kzalloc dpld fail\n");
		goto err;
	}
	ld = &dpld->ld;
	task_data = (unsigned long)dpld;

	/* Retrieve modem data and DPRAM control data from the modem data */
	ld->mdm_data = mdm_data;
	ld->name = mdm_data->link_name;
	ld->ipc_version = mdm_data->ipc_version;

	/* Retrieve the most basic data for IPC from the modem data */
	dpld->dpctl = dpctl;
	dpld->dp_type = dpctl->dp_type;

	if (mdm_data->ipc_version < SIPC_VER_50) {
		if (!dpctl->max_ipc_dev) {
			mif_info("ERR! no max_ipc_dev\n");
			goto err;
		}

		ld->aligned = dpctl->aligned;
		dpld->max_ipc_dev = dpctl->max_ipc_dev;
	} else {
		ld->aligned = 1;
		dpld->max_ipc_dev = MAX_SIPC5_DEV;
	}

	/* Set attributes as a link device */
	ld->init_comm = dpram_link_init;
	ld->terminate_comm = dpram_link_terminate;
	ld->send = dpram_send;
	ld->force_dump = dpram_force_dump;
	ld->dump_start = dpram_dump_start;
	ld->dump_update = dpram_dump_update;
	ld->ioctl = dpram_ioctl;

	INIT_LIST_HEAD(&ld->list);

	skb_queue_head_init(&ld->sk_fmt_tx_q);
	skb_queue_head_init(&ld->sk_raw_tx_q);
	skb_queue_head_init(&ld->sk_rfs_tx_q);
	ld->skb_txq[IPC_FMT] = &ld->sk_fmt_tx_q;
	ld->skb_txq[IPC_RAW] = &ld->sk_raw_tx_q;
	ld->skb_txq[IPC_RFS] = &ld->sk_rfs_tx_q;

	/* Set up function pointers */
	dpram_setup_common_op(dpld);
	dpld->dpram_dump = dpram_dump_memory;
	dpld->ext_op = dpram_get_ext_op(mdm_data->modem_type);

	/* Retrieve DPRAM resource */
	if (!dpctl->dp_base) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (!res) {
			mif_info("%s: ERR! platform_get_resource fail\n",
				ld->name);
			goto err;
		}
		res_size = resource_size(res);

		dpctl->dp_base = ioremap_nocache(res->start, res_size);
		dpctl->dp_size = res_size;
	}
	dpld->dp_base = dpctl->dp_base;
	dpld->dp_size = dpctl->dp_size;

	mif_info("%s: dp_type %d, aligned %d, dp_base 0x%08X, dp_size %d\n",
		ld->name, dpld->dp_type, ld->aligned, (int)dpld->dp_base,
		dpld->dp_size);

	/* Initialize DPRAM map (physical map -> logical map) */
	ret = dpram_table_init(dpld);
	if (ret < 0) {
		mif_info("%s: ERR! dpram_table_init fail (err %d)\n",
			ld->name, ret);
		goto err;
	}

	/* Disable IPC */
	set_magic(dpld, 0);
	set_access(dpld, 0);
	dpld->dpram_init_status = DPRAM_INIT_STATE_NONE;

	/* Initialize locks, completions, and bottom halves */
	snprintf(dpld->wlock_name, DP_MAX_NAME_LEN, "%s_wlock", ld->name);
	wake_lock_init(&dpld->wlock, WAKE_LOCK_SUSPEND, dpld->wlock_name);

	init_completion(&dpld->dpram_init_cmd);
	init_completion(&dpld->modem_pif_init_done);
	init_completion(&dpld->udl_start_complete);
	init_completion(&dpld->udl_cmd_complete);
	init_completion(&dpld->crash_start_complete);
	init_completion(&dpld->dump_start_complete);
	init_completion(&dpld->dump_recv_done);

	tasklet_init(&dpld->rx_tsk, dpram_ipc_rx_task, task_data);
	if (dpld->ext_op && dpld->ext_op->dl_task)
		tasklet_init(&dpld->dl_tsk, dpld->ext_op->dl_task, task_data);

	/* Prepare RXB queue */
	qsize = DPRAM_MAX_RXBQ_SIZE;
	for (i = 0; i < dpld->max_ipc_dev; i++) {
		bsize = rxbq_get_page_size(get_rx_buff_size(dpld, i));
		dpld->rxbq[i].size = qsize;
		dpld->rxbq[i].in = 0;
		dpld->rxbq[i].out = 0;
		dpld->rxbq[i].rxb = rxbq_create_pool(bsize, qsize);
		if (!dpld->rxbq[i].rxb) {
			mif_info("%s: ERR! %s rxbq_create_pool fail\n",
				ld->name, get_dev_name(i));
			goto err;
		}
		mif_info("%s: %s rxbq_pool created (bsize:%d, qsize:%d)\n",
			ld->name, get_dev_name(i), bsize, qsize);
	}

	/* Prepare a multi-purpose miscellaneous buffer */
	dpld->buff = kzalloc(dpld->dp_size, GFP_KERNEL);
	if (!dpld->buff) {
		mif_info("%s: ERR! kzalloc dpld->buff fail\n", ld->name);
		goto err;
	}

	/* Retrieve DPRAM IRQ GPIO# */
	dpld->gpio_dpram_int = mdm_data->gpio_dpram_int;

	/* Retrieve DPRAM IRQ# */
	if (!dpctl->dpram_irq) {
		dpctl->dpram_irq = platform_get_irq_byname(pdev, "dpram_irq");
		if (dpctl->dpram_irq < 0) {
			mif_info("%s: ERR! platform_get_irq_byname fail\n",
				ld->name);
			goto err;
		}
	}
	dpld->irq = dpctl->dpram_irq;

	/* Retrieve DPRAM IRQ flags */
	if (!dpctl->dpram_irq_flags)
		dpctl->dpram_irq_flags = (IRQF_NO_SUSPEND | IRQF_TRIGGER_LOW);
	dpld->irq_flags = dpctl->dpram_irq_flags;

	/* Register DPRAM interrupt handler */
	snprintf(dpld->irq_name, DP_MAX_NAME_LEN, "%s_irq", ld->name);
	ret = dpram_register_isr(dpld->irq, dpram_irq_handler, dpld->irq_flags,
				dpld->irq_name, dpld);
	if (ret)
		goto err;

	return ld;

err:
	if (dpld) {
		if (dpld->buff)
			kfree(dpld->buff);
		kfree(dpld);
	}

	return NULL;
}

