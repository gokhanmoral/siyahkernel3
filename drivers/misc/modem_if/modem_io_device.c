/* /linux/drivers/misc/modem_if/modem_io_device.c
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
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/if_arp.h>
#include <linux/ip.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>
#include <linux/device.h>

#include <linux/platform_data/modem.h>
#ifdef CONFIG_LINK_DEVICE_C2C
#include <linux/platform_data/c2c.h>
#endif
#include "modem_prj.h"
#include "modem_utils.h"

#define HDLC_START		0x7F
#define HDLC_END		0x7E
#define SIZE_OF_HDLC_START	1
#define SIZE_OF_HDLC_END	1
#define MAX_RXDATA_SIZE		0x1000	/* 4 * 1024 */
#define MAX_MULTI_FMT_SIZE	0x4000	/* 16 * 1024 */
#define MAX_LINK_PADDING_SIZE	3

static const char hdlc_start[1] = { HDLC_START };
static const char hdlc_end[1] = { HDLC_END };

struct fmt_hdr {
	u16 len;
	u8 control;
} __packed;

struct raw_hdr {
	u32 len;
	u8 channel;
	u8 control;
} __packed;

struct rfs_hdr {
	u32 len;
	u8 cmd;
	u8 id;
} __packed;

static int rx_iodev_skb(struct sk_buff *skb);

static ssize_t show_waketime(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	unsigned int msec;
	char *p = buf;
	struct miscdevice *miscdev = dev_get_drvdata(dev);
	struct io_device *iod = container_of(miscdev, struct io_device,
			miscdev);

	msec = jiffies_to_msecs(iod->waketime);

	p += sprintf(buf, "raw waketime : %ums\n", msec);

	return p - buf;
}

static ssize_t store_waketime(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long msec;
	int ret;
	struct miscdevice *miscdev = dev_get_drvdata(dev);
	struct io_device *iod = container_of(miscdev, struct io_device,
			miscdev);

	ret = strict_strtoul(buf, 10, &msec);
	if (ret)
		return count;

	iod->waketime = msecs_to_jiffies(msec);

	return count;
}

static struct device_attribute attr_waketime =
	__ATTR(waketime, S_IRUGO | S_IWUSR, show_waketime, store_waketime);

static int get_header_size(struct io_device *iod)
{
	switch (iod->format) {
	case IPC_FMT:
		return sizeof(struct fmt_hdr);

	case IPC_RAW:
	case IPC_MULTI_RAW:
		return sizeof(struct raw_hdr);

	case IPC_RFS:
		return sizeof(struct rfs_hdr);

	case IPC_BOOT:
		/* minimum size for transaction align */
		return 4;

	case IPC_RAMDUMP:
	default:
		return 0;
	}
}

static int get_hdlc_size(struct io_device *iod, char *buf)
{
	struct fmt_hdr *fmt_header;
	struct raw_hdr *raw_header;
	struct rfs_hdr *rfs_header;

	pr_debug("[IOD] buf : %02x %02x %02x (%d)\n", *buf, *(buf + 1),
				*(buf + 2), __LINE__);

	switch (iod->format) {
	case IPC_FMT:
		fmt_header = (struct fmt_hdr *)buf;
		return fmt_header->len;
	case IPC_RAW:
	case IPC_MULTI_RAW:
		raw_header = (struct raw_hdr *)buf;
		return raw_header->len;
	case IPC_RFS:
		rfs_header = (struct rfs_hdr *)buf;
		return rfs_header->len;
	default:
		break;
	}
	return 0;
}

static void *get_header(struct io_device *iod, size_t count,
			char *frame_header_buf)
{
	struct fmt_hdr *fmt_h;
	struct raw_hdr *raw_h;
	struct rfs_hdr *rfs_h;

	switch (iod->format) {
	case IPC_FMT:
		fmt_h = (struct fmt_hdr *)frame_header_buf;

		fmt_h->len = count + sizeof(struct fmt_hdr);
		fmt_h->control = 0;

		return (void *)frame_header_buf;

	case IPC_RAW:
	case IPC_MULTI_RAW:
		raw_h = (struct raw_hdr *)frame_header_buf;

		raw_h->len = count + sizeof(struct raw_hdr);
		raw_h->channel = iod->id & 0x1F;
		raw_h->control = 0;

		return (void *)frame_header_buf;

	case IPC_RFS:
		rfs_h = (struct rfs_hdr *)frame_header_buf;

		rfs_h->len = count + sizeof(struct raw_hdr);
		rfs_h->id = iod->id;

		return (void *)frame_header_buf;

	default:
		return 0;
	}
}

static inline int calc_padding_size(struct io_device *iod,
	struct link_device *ld, unsigned len)
{
	if (likely(ld->aligned))
		return (4 - (len & 0x3)) & 0x3;
	else
		return 0;
}

static inline int rx_hdlc_head_start_check(char *buf)
{
	/* check hdlc head and return size of start byte */
	return (buf[0] == HDLC_START) ? SIZE_OF_HDLC_START : -EBADMSG;
}

static inline int rx_hdlc_tail_check(char *buf)
{
	/* check hdlc tail and return size of tail byte */
	return (buf[0] == HDLC_END) ? SIZE_OF_HDLC_END : -EBADMSG;
}

/* remove hdlc header and store IPC header */
static int rx_hdlc_head_check(struct io_device *iod, struct link_device *ld,
						char *buf, unsigned rest)
{
	struct header_data *hdr = &fragdata(iod, ld)->h_data;
	int head_size = get_header_size(iod);
	int done_len = 0;
	int len = 0;

	/* first frame, remove start header 7F */
	if (!hdr->start) {
		len = rx_hdlc_head_start_check(buf);
		if (len < 0) {
			pr_err("[IOD] Wrong HDLC start: 0x%x\n", *buf);
			return len; /*Wrong hdlc start*/
		}

		pr_debug("[IOD] check len : %d, rest : %d (%d)\n", len,
					rest, __LINE__);

		/* set the start flag of current packet */
		hdr->start = HDLC_START;
		hdr->len = 0;

		/* debug print */
		switch (iod->format) {
		case IPC_FMT:
		case IPC_RAW:
		case IPC_MULTI_RAW:
		case IPC_RFS:
			/* TODO: print buf...  */
			break;

		case IPC_CMD:
		case IPC_BOOT:
		case IPC_RAMDUMP:
		default:
			break;
		}

		buf += len;
		done_len += len;
		rest -= len; /* rest, call by value */
	}

	pr_debug("[IOD] check len : %d, rest : %d (%d)\n", len, rest, __LINE__);

	/* store the HDLC header to iod priv */
	if (hdr->len < head_size) {
		len = min(rest, head_size - hdr->len);
		memcpy(hdr->hdr + hdr->len, buf, len);
		hdr->len += len;
		done_len += len;
	}

	pr_debug("[IOD] check done_len : %d, rest : %d (%d)\n", done_len,
				rest, __LINE__);
	return done_len;
}

/* alloc skb and copy data to skb */
static int rx_hdlc_data_check(struct io_device *iod, struct link_device *ld,
						char *buf, unsigned rest)
{
	struct header_data *hdr = &fragdata(iod, ld)->h_data;
	struct sk_buff *skb = fragdata(iod, ld)->skb_recv;
	int head_size = get_header_size(iod);
	int data_size = get_hdlc_size(iod, hdr->hdr) - head_size;
	int alloc_size = min(data_size, MAX_RXDATA_SIZE);
	int len = 0;
	int done_len = 0;
	int rest_len = data_size - hdr->frag_len;
	struct sk_buff *skb_new = NULL;

	pr_debug("[IOD] head_size : %d, data_size : %d (%d)\n", head_size,
				data_size, __LINE__);

	/* first payload data - alloc skb */
	if (!skb) {
		switch (iod->format) {
		case IPC_RFS:
			alloc_size = min_t(int, data_size, rest) + head_size;
			alloc_size = min(alloc_size, MAX_RXDATA_SIZE);
			skb = rx_alloc_skb(alloc_size, GFP_ATOMIC, iod, ld);
			if (unlikely(!skb))
				return -ENOMEM;

			/* copy the RFS haeder to skb->data */
			memcpy(skb_put(skb, head_size), hdr->hdr, head_size);
			break;

		case IPC_MULTI_RAW:
			if (iod->use_handover)
				skb = rx_alloc_skb(alloc_size +
					sizeof(struct ethhdr), GFP_ATOMIC,
					iod, ld);
			else
				skb = rx_alloc_skb(alloc_size, GFP_ATOMIC,
					iod, ld);

			if (unlikely(!skb))
				return -ENOMEM;

			if (iod->use_handover)
				skb_reserve(skb, sizeof(struct ethhdr));
			break;

		default:
			skb = rx_alloc_skb(alloc_size, GFP_ATOMIC, iod, ld);
			if (unlikely(!skb))
				return -ENOMEM;
			break;
		}
		fragdata(iod, ld)->skb_recv = skb;
	}

	/* if recv packet size is larger than user space */
	while ((rest_len > MAX_RXDATA_SIZE) && (rest > 0)) {
		len = MAX_RXDATA_SIZE - skb->len;
		len = min_t(int, len, rest);
		len = min(len, rest_len);
		memcpy(skb_put(skb, len), buf, len);
		buf += len;
		done_len += len;
		rest -= len;
		rest_len -= len;

		if (!rest_len)
			break;

		rx_iodev_skb(fragdata(iod, ld)->skb_recv);
		fragdata(iod, ld)->skb_recv =  NULL;

		alloc_size = min(rest_len, MAX_RXDATA_SIZE);
		skb_new = rx_alloc_skb(alloc_size, GFP_ATOMIC, iod, ld);
		if (unlikely(!skb_new))
			return -ENOMEM;
		skb = fragdata(iod, ld)->skb_recv = skb_new;
	}

	/* copy data to skb */
	len = min(rest, alloc_size - skb->len);
	len = min(len, rest_len);
	pr_debug("[IOD] rest : %d, alloc_size : %d , len : %d (%d)\n",
				rest, alloc_size, skb->len, __LINE__);

	memcpy(skb_put(skb, len), buf, len);
	done_len += len;
	hdr->frag_len += done_len;

	return done_len;
}

static int rx_multi_fmt_frame(struct sk_buff *rx_skb)
{
	struct io_device *iod = skbpriv(rx_skb)->iod;
	struct link_device *ld = skbpriv(rx_skb)->ld;
	struct fmt_hdr *fh =
		(struct fmt_hdr *)fragdata(iod, ld)->h_data.hdr;
	unsigned int    id = fh->control & 0x7F;
	struct sk_buff *skb = iod->skb[id];
	unsigned char  *data = fragdata(iod, ld)->skb_recv->data;
	unsigned int    rcvd = fragdata(iod, ld)->skb_recv->len;

	if (!skb) {
		/* If there has been no multiple frame with this ID */
		if (!(fh->control & 0x80)) {
			/* It is a single frame because the "more" bit is 0. */
#if 0
			pr_err("\n[IOD] <%s:%s> Rx FMT frame (len %d)\n",
				__func__, iod->name, rcvd);
			print_sipc4_fmt_frame(data);
			pr_err("\n");
#endif
			skb_queue_tail(&iod->sk_rx_q,
					fragdata(iod, ld)->skb_recv);
			pr_debug("[IOD] wake up wq of %s\n", iod->name);
			wake_up(&iod->wq);
			return 0;
		} else {
			struct sipc4_fmt_hdr *ifh = NULL;
			skb = rx_alloc_skb(MAX_MULTI_FMT_SIZE, GFP_ATOMIC,
					iod, ld);
			if (!skb) {
				pr_err("[IOD/E] <%s> alloc_skb fail\n",
					__func__);
				return -ENOMEM;
			}
			iod->skb[id] = skb;

			ifh = (struct sipc4_fmt_hdr *)data;
			pr_err("[IOD] <%s> Start multi-frame (ID %d, len %d)",
				__func__, id, ifh->len);
		}
	}

	/* Start multi-frame processing */

	memcpy(skb_put(skb, rcvd), data, rcvd);
	dev_kfree_skb_any(fragdata(iod, ld)->skb_recv);

	if (fh->control & 0x80) {
		/* The last frame has not arrived yet. */
		pr_err("[IOD] <%s> Receiving (ID %d, %d bytes)\n",
			__func__, id, skb->len);
	} else {
		/* It is the last frame because the "more" bit is 0. */
		pr_err("[IOD] <%s> The Last (ID %d, %d bytes received)\n",
			__func__, id, skb->len);
#if 0
		pr_err("\n[IOD] <%s:%s> Rx FMT frame (len %d)\n",
			__func__, iod->name, skb->len);
		print_sipc4_fmt_frame(skb->data);
		pr_err("\n");
#endif
		skb_queue_tail(&iod->sk_rx_q, skb);
		iod->skb[id] = NULL;
		pr_info("[IOD] wake up wq of %s\n", iod->name);
		wake_up(&iod->wq);
	}

	return 0;
}

static int rx_iodev_skb_raw(struct sk_buff *skb)
{
	int err;
	struct io_device *iod = skbpriv(skb)->real_iod;
	struct link_device *ld = skbpriv(skb)->ld;
	struct net_device *ndev;
	struct iphdr *ip_header;
	struct ethhdr *ehdr;
	const char source[ETH_ALEN] = SOURCE_MAC_ADDR;

	switch (iod->io_typ) {
	case IODEV_MISC:
		skb_queue_tail(&iod->sk_rx_q, skb);
		wake_up(&iod->wq);
		return 0;

	case IODEV_NET:
		ndev = iod->ndev;
		if (!ndev) {
			pr_err("[IOD] <%s:%s> iod->ndev == NULL",
				__func__, iod->name);
			return -EINVAL;
		}

		skb->dev = ndev;
		ndev->stats.rx_packets++;
		ndev->stats.rx_bytes += skb->len;

		/* check the version of IP */
		ip_header = (struct iphdr *)skb->data;
		if (ip_header->version == IP6VERSION)
			skb->protocol = htons(ETH_P_IPV6);
		else
			skb->protocol = htons(ETH_P_IP);

		if (iod->use_handover) {
			skb_push(skb, sizeof(struct ethhdr));
			ehdr = (void *)skb->data;
			memcpy(ehdr->h_dest, ndev->dev_addr, ETH_ALEN);
			memcpy(ehdr->h_source, source, ETH_ALEN);
			ehdr->h_proto = skb->protocol;
			skb->ip_summed = CHECKSUM_UNNECESSARY;
			skb_reset_mac_header(skb);

			skb_pull(skb, sizeof(struct ethhdr));
		}

		if (in_irq())
			err = netif_rx(skb);
		else
			err = netif_rx_ni(skb);

		if (err != NET_RX_SUCCESS)
			dev_err(&ndev->dev, "rx error: %d\n", err);
		return err;

	default:
		pr_err("[IOD] wrong io_type : %d\n", iod->io_typ);
		return -EINVAL;
	}
}

static void rx_iodev_work(struct work_struct *work)
{
	int ret;
	struct sk_buff *skb;
	struct io_device *iod = container_of(work, struct io_device,
				rx_work.work);

	while ((skb = skb_dequeue(&iod->sk_rx_q)) != NULL) {
		ret = rx_iodev_skb_raw(skb);
		if (ret < 0) {
			pr_err("[IOD/E] <%s:%s> rx_iodev_skb_raw err = %d",
				__func__, iod->name, ret);
			dev_kfree_skb_any(skb);
		} else if (ret == NET_RX_DROP) {
			pr_err("[IOD/E] <%s:%s> ret == NET_RX_DROP\n",
				__func__, iod->name);
			schedule_delayed_work(&iod->rx_work,
						msecs_to_jiffies(100));
			break;
		}
	}
}

static int rx_multipdp(struct sk_buff *skb)
{
	u8 ch;
	struct io_device *iod = skbpriv(skb)->iod;
	struct link_device *ld = skbpriv(skb)->ld;
	struct raw_hdr *raw_header =
		(struct raw_hdr *)fragdata(iod, ld)->h_data.hdr;
	struct io_raw_devices *io_raw_devs = NULL;
	struct io_device *real_iod = NULL;

	ch = raw_header->channel;
	io_raw_devs = (struct io_raw_devices *)iod->private_data;
	real_iod = io_raw_devs->raw_devices[ch];
	if (!real_iod) {
		pr_err("[IOD] %s: wrong channel %d\n", __func__, ch);
		return -1;
	}

	skbpriv(skb)->real_iod = real_iod;
	skb_queue_tail(&iod->sk_rx_q, skb);
	pr_debug("sk_rx_qlen:%d\n", iod->sk_rx_q.qlen);

	schedule_delayed_work(&iod->rx_work, 0);
	return 0;
}

/* de-mux function draft */
static int rx_iodev_skb(struct sk_buff *skb)
{
	struct io_device *iod = skbpriv(skb)->iod;

	switch (iod->format) {
	case IPC_MULTI_RAW:
		return rx_multipdp(skb);

	case IPC_FMT:
		return rx_multi_fmt_frame(skb);

	case IPC_RFS:
	default:
		skb_queue_tail(&iod->sk_rx_q, skb);
		pr_debug("[IOD] wake up wq of %s\n", iod->name);
		wake_up(&iod->wq);
		return 0;
	}
}

static int rx_hdlc_packet(struct io_device *iod, struct link_device *ld,
		const char *data, unsigned recv_size)
{
	int rest = (int)recv_size;
	char *buf = (char *)data;
	int err = 0;
	int len = 0;
	unsigned rcvd = 0;

	if (rest <= 0)
		goto exit;

	pr_debug("[IOD] <%s> RX_SIZE = %d, ld: %s\n", __func__, rest, ld->name);

	if (fragdata(iod, ld)->h_data.frag_len) {
		/*
		  If the fragdata(iod, ld)->h_data.frag_len field is
		  not zero, there is a HDLC frame that is waiting for more data
		  or HDLC_END in the skb (fragdata(iod, ld)->skb_recv).
		  In this case, rx_hdlc_head_check() must be skipped.
		*/
		goto data_check;
	}

next_frame:
	err = len = rx_hdlc_head_check(iod, ld, buf, rest);
	if (err < 0)
		goto exit;
	pr_debug("[IOD] check len : %d, rest : %d (%d)\n", len, rest,
				__LINE__);

	buf += len;
	rest -= len;
	if (rest <= 0)
		goto exit;

data_check:
	/*
	  If the return value of rx_hdlc_data_check() is zero, there remains
	  only HDLC_END that will be received.
	*/
	err = len = rx_hdlc_data_check(iod, ld, buf, rest);
	if (err < 0)
		goto exit;
	pr_debug("[IOD] check len : %d, rest : %d (%d)\n", len, rest,
				__LINE__);

	buf += len;
	rest -= len;

	if (!rest && fragdata(iod, ld)->h_data.frag_len) {
		/*
		  Data is being received and more data or HDLC_END does not
		  arrive yet, but there is no more data in the buffer. More
		  data may come within the next frame from the link device.
		*/
		return 0;
	} else if (rest <= 0)
		goto exit;

	/* At this point, one HDLC frame except HDLC_END has been received. */

	err = len = rx_hdlc_tail_check(buf);
	if (err < 0) {
		pr_err("[IOD] Wrong HDLC end: 0x%02X\n", *buf);
		goto exit;
	}
	pr_debug("[IOD] check len : %d, rest : %d (%d)\n", len, rest,
				__LINE__);
	buf += len;
	rest -= len;

	/* At this point, one complete HDLC frame has been received. */

	/*
	  The padding size is applied for the next HDLC frame. Zero will be
	  returned by calc_padding_size() if the link device does not require
	  4-byte aligned access.
	*/
	rcvd = get_hdlc_size(iod, fragdata(iod, ld)->h_data.hdr) +
	       (SIZE_OF_HDLC_START + SIZE_OF_HDLC_END);
	len = calc_padding_size(iod, ld, rcvd);
	buf += len;
	rest -= len;
	if (rest < 0)
		goto exit;

	err = rx_iodev_skb(fragdata(iod, ld)->skb_recv);
	if (err < 0)
		goto exit;

	/* initialize header & skb */
	fragdata(iod, ld)->skb_recv = NULL;
	memset(&fragdata(iod, ld)->h_data, 0x00,
			sizeof(struct header_data));

	if (rest)
		goto next_frame;

exit:
	/* free buffers. mipi-hsi re-use recv buf */

	if (rest < 0)
		err = -ERANGE;

	if (err < 0 && fragdata(iod, ld)->skb_recv) {
		dev_kfree_skb_any(fragdata(iod, ld)->skb_recv);
		fragdata(iod, ld)->skb_recv = NULL;

		/* clear headers */
		memset(&fragdata(iod, ld)->h_data, 0x00,
				sizeof(struct header_data));
	}

	return err;
}

static int rx_rfs_packet(struct io_device *iod, struct link_device *ld,
					const char *data, unsigned size)
{
	int err = 0;
	int pad = 0;
	int rcvd = 0;
	struct sk_buff *skb;

	if (data[0] != HDLC_START) {
		pr_err("[IOD] <%s> Dropping RFS packet ... "
		       "size = %d, start = %02X %02X %02X %02X\n",
			__func__, size,
			data[0], data[1], data[2], data[3]);
		return -EINVAL;
	}

	if (data[size-1] != HDLC_END) {
		for (pad = 1; pad < 4; pad++)
			if (data[(size-1)-pad] == HDLC_END)
				break;

		if (pad >= 4) {
			char *b = (char *)data;
			unsigned sz = size;
			pr_err("[IOD/E] <%s> size %d, No END_FLAG!!!\n",
				__func__, size);
			pr_err("[IOD/E] <%s> end = %02X %02X %02X %02X\n",
				__func__, b[sz-4], b[sz-3], b[sz-2], b[sz-1]);
			return -EINVAL;
		} else {
			pr_info("[IOD] <%s> padding = %d\n", __func__, pad);
		}
	}

	skb = rx_alloc_skb(size, GFP_ATOMIC, iod, ld);
	if (unlikely(!skb)) {
		pr_err("[IOD] <%s> alloc_skb fail\n", __func__);
		return -ENOMEM;
	}

	/* copy the RFS haeder to skb->data */
	rcvd = size - sizeof(hdlc_start) - sizeof(hdlc_end) - pad;
	memcpy(skb_put(skb, rcvd), ((char *)data + sizeof(hdlc_start)), rcvd);

	fragdata(iod, ld)->skb_recv = skb;
	err = rx_iodev_skb(fragdata(iod, ld)->skb_recv);

	return err;
}

/* called from link device when a packet arrives for this io device */
static int io_dev_recv_data_from_link_dev(struct io_device *iod,
		struct link_device *ld, const char *data, unsigned int len)
{
	struct sk_buff *skb;
	int err;

	switch (iod->format) {
	case IPC_RFS:
#ifdef CONFIG_IPC_CMC22x_OLD_RFS
		err = rx_rfs_packet(iod, ld, data, len);
		return err;
#endif

	case IPC_FMT:
	case IPC_RAW:
	case IPC_MULTI_RAW:
		if (iod->waketime)
			wake_lock_timeout(&iod->wakelock, iod->waketime);
		err = rx_hdlc_packet(iod, ld, data, len);
		if (err < 0)
			pr_err("[IOD] fail process HDLC frame\n");
		return err;

	case IPC_CMD:
		/* TODO- handle flow control command from CP */
		return 0;

	case IPC_BOOT:
	case IPC_RAMDUMP:
		/* save packet to sk_buff */
		skb = rx_alloc_skb(len, GFP_ATOMIC, iod, ld);
		if (!skb) {
			pr_err("[IOD] fail alloc skb (%d)\n", __LINE__);
			return -ENOMEM;
		}

		pr_debug("[IOD] boot len : %d\n", len);

		memcpy(skb_put(skb, len), data, len);
		skb_queue_tail(&iod->sk_rx_q, skb);
		pr_debug("[IOD] skb len : %d\n", skb->len);

		wake_up(&iod->wq);
		return len;

	default:
		return -EINVAL;
	}
}

/* inform the IO device that the modem is now online or offline or
 * crashing or whatever...
 */
static void io_dev_modem_state_changed(struct io_device *iod,
			enum modem_state state)
{
	iod->mc->phone_state = state;
	pr_err("[IOD] <%s> modem state changed. (iod: %s, state: %d)\n",
		__func__, iod->name, state);

	if ((state == STATE_CRASH_RESET) || (state == STATE_CRASH_EXIT)
		|| (state == STATE_NV_REBUILDING))
		wake_up(&iod->wq);
}

static int misc_open(struct inode *inode, struct file *filp)
{
	struct io_device *iod = to_io_device(filp->private_data);
	filp->private_data = (void *)iod;

	pr_err("[IOD] <%s> iod = %s\n", __func__, iod->name);
	if (iod->link->init_comm)
		return iod->link->init_comm(iod->link, iod);

	return 0;
}

static int misc_release(struct inode *inode, struct file *filp)
{
	struct io_device *iod = (struct io_device *)filp->private_data;

	pr_err("[IOD] <%s> iod = %s\n", __func__, iod->name);

	if (iod->link->terminate_comm)
		iod->link->terminate_comm(iod->link, iod);

	return 0;
}

static unsigned int misc_poll(struct file *filp, struct poll_table_struct *wait)
{
	struct io_device *iod = (struct io_device *)filp->private_data;

	poll_wait(filp, &iod->wq, wait);

	if ((!skb_queue_empty(&iod->sk_rx_q))
				&& (iod->mc->phone_state != STATE_OFFLINE))
		return POLLIN | POLLRDNORM;
	else if ((iod->mc->phone_state == STATE_CRASH_RESET) ||
			(iod->mc->phone_state == STATE_CRASH_EXIT) ||
			(iod->mc->phone_state == STATE_NV_REBUILDING))
		return POLLHUP;
	else
		return 0;
}

static long misc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int i;
	int p_state;
	struct io_device *iod = (struct io_device *)filp->private_data;
	struct io_raw_devices *io_raw_devs;

	pr_debug("[IOD] <%s> cmd = 0x%x\n", __func__, cmd);

	switch (cmd) {
	case IOCTL_MODEM_ON:
		pr_debug("[IOD] misc_ioctl : IOCTL_MODEM_ON\n");
		return iod->mc->ops.modem_on(iod->mc);

	case IOCTL_MODEM_OFF:
		pr_debug("[IOD] misc_ioctl : IOCTL_MODEM_OFF\n");
		return iod->mc->ops.modem_off(iod->mc);

	case IOCTL_MODEM_RESET:
		pr_debug("[IOD] misc_ioctl : IOCTL_MODEM_RESET\n");
		return iod->mc->ops.modem_reset(iod->mc);

	case IOCTL_MODEM_BOOT_ON:
		pr_debug("[IOD] misc_ioctl : IOCTL_MODEM_BOOT_ON\n");
		return iod->mc->ops.modem_boot_on(iod->mc);

	case IOCTL_MODEM_BOOT_OFF:
		pr_debug("[IOD] misc_ioctl : IOCTL_MODEM_BOOT_OFF\n");
		return iod->mc->ops.modem_boot_off(iod->mc);

	/* TODO - will remove this command after ril updated */
	case IOCTL_MODEM_START:
		pr_debug("[IOD] misc_ioctl : IOCTL_MODEM_START\n");
		return 0;

	case IOCTL_MODEM_STATUS:
		pr_debug("[IOD] misc_ioctl : IOCTL_MODEM_STATUS\n");

		p_state = iod->mc->phone_state;
		if ((p_state == STATE_CRASH_RESET) ||
			(p_state == STATE_CRASH_EXIT)) {
			pr_err("[IOD] send err state : %d\n", p_state);
		/*	iod->mc->phone_state = STATE_OFFLINE; */
		} else if (p_state == STATE_NV_REBUILDING) {
			pr_info("[IOD] send nv rebuild state : %d\n",
				p_state);
			iod->mc->phone_state = STATE_ONLINE;
		}
		return p_state;

	case IOCTL_MODEM_PROTOCOL_SUSPEND:
		pr_info("[IOD] misc_ioctl : IOCTL_MODEM_PROTOCOL_SUSPEND\n");

		if (iod->format != IPC_MULTI_RAW)
			return -EINVAL;
		io_raw_devs = (struct io_raw_devices *)iod->private_data;

		for (i = 0; i < MAX_RAW_DEVS; i++) {
			if (io_raw_devs->raw_devices[i] &&
			(io_raw_devs->raw_devices[i]->io_typ == IODEV_NET)) {
				netif_stop_queue(
					io_raw_devs->raw_devices[i]->ndev);
				pr_info("[IOD] netif stopped : %s\n",
					io_raw_devs->raw_devices[i]->name);
			}
		}
		return 0;

	case IOCTL_MODEM_PROTOCOL_RESUME:
		pr_info("[IOD] misc_ioctl : IOCTL_MODEM_PROTOCOL_RESUME\n");

		if (iod->format != IPC_MULTI_RAW)
			return -EINVAL;
		io_raw_devs = (struct io_raw_devices *)iod->private_data;

		for (i = 0; i < MAX_RAW_DEVS; i++) {
			if (io_raw_devs->raw_devices[i] &&
			(io_raw_devs->raw_devices[i]->io_typ == IODEV_NET)) {
				netif_wake_queue(
					io_raw_devs->raw_devices[i]->ndev);
				pr_info("[IOD] netif woke : %s\n",
					io_raw_devs->raw_devices[i]->name);
			}
		}
		return 0;

	case IOCTL_MODEM_DUMP_START:
		pr_err("[IOD] misc_ioctl : IOCTL_MODEM_DUMP_START\n");
		return iod->link->dump_start(iod->link, iod);

	case IOCTL_MODEM_DUMP_UPDATE:
		pr_debug("[IOD] misc_ioctl : IOCTL_MODEM_DUMP_UPDATE\n");
		return iod->link->dump_update(iod->link, iod, arg);

	case IOCTL_MODEM_FORCE_CRASH_EXIT:
		pr_debug("[IOD] misc_ioctl : IOCTL_MODEM_FORCE_CRASH_EXIT\n");
		if (iod->mc->ops.modem_force_crash_exit)
			return iod->mc->ops.modem_force_crash_exit(iod->mc);
		return -EINVAL;

	case IOCTL_MODEM_CP_UPLOAD:
		pr_err("[IOD] misc_ioctl : IOCTL_MODEM_CP_UPLOAD\n");
		panic("CP Crash");
		return 0;

	case IOCTL_MODEM_DUMP_RESET:
		pr_err("[IOD] misc_ioctl : IOCTL_MODEM_DUMP_RESET\n");
		return iod->mc->ops.modem_dump_reset(iod->mc);

	default:
		 /* If you need to handle the ioctl for specific link device,
		  * then assign the link ioctl handler to iod->link->ioctl
		  * It will be call for specific link ioctl */
		if (iod->link->ioctl)
			return iod->link->ioctl(iod->link, iod, cmd, arg);

		pr_err("[IOD] misc_ioctl : ioctl 0x%X is not defined.\n", cmd);
		return -EINVAL;
	}
	return 0;
}

static ssize_t misc_write(struct file *filp, const char __user * buf,
			size_t count, loff_t *ppos)
{
	struct io_device *iod = (struct io_device *)filp->private_data;
	int frame_len = 0;
	char frame_header_buf[sizeof(struct raw_hdr)];
	struct sk_buff *skb;
	int err;
	size_t tx_size;

	/* TODO - check here flow control for only raw data */

	frame_len = SIZE_OF_HDLC_START +
		    get_header_size(iod) +
		    count +
		    SIZE_OF_HDLC_END;
	if (likely(iod->link->aligned))
		frame_len += MAX_LINK_PADDING_SIZE;

	skb = alloc_skb(frame_len, GFP_KERNEL);
	if (!skb) {
		pr_err("[IOD] fail alloc skb (%d)\n", __LINE__);
		return -ENOMEM;
	}

	switch (iod->format) {
	case IPC_BOOT:
	case IPC_RAMDUMP:
		if (copy_from_user(skb_put(skb, count), buf, count) != 0) {
			dev_kfree_skb_any(skb);
			return -EFAULT;
		}
		break;

	case IPC_RFS:
		memcpy(skb_put(skb, SIZE_OF_HDLC_START), hdlc_start,
				SIZE_OF_HDLC_START);
		if (copy_from_user(skb_put(skb, count), buf, count) != 0) {
			dev_kfree_skb_any(skb);
			return -EFAULT;
		}
		memcpy(skb_put(skb, SIZE_OF_HDLC_END), hdlc_end,
					SIZE_OF_HDLC_END);
		break;

	default:
		memcpy(skb_put(skb, SIZE_OF_HDLC_START), hdlc_start,
				SIZE_OF_HDLC_START);
		memcpy(skb_put(skb, get_header_size(iod)),
			get_header(iod, count, frame_header_buf),
			get_header_size(iod));
		if (copy_from_user(skb_put(skb, count), buf, count) != 0) {
			dev_kfree_skb_any(skb);
			return -EFAULT;
		}
		memcpy(skb_put(skb, SIZE_OF_HDLC_END), hdlc_end,
					SIZE_OF_HDLC_END);
		break;
	}

	skb_put(skb, calc_padding_size(iod, iod->link, skb->len));
#if 0
	if (iod->format == IPC_FMT) {
		pr_err("\n[LNK] <%s:%s> Tx HDLC FMT frame (len %d)\n",
			__func__, iod->name, skb->len);
		print_sipc4_hdlc_fmt_frame(skb->data);
		pr_err("\n");
	}
#endif
#if 0
	if (iod->format == IPC_RAW) {
		pr_err("\n[LNK] <%s:%s> Tx HDLC RAW frame (len %d)\n",
			__func__, iod->name, skb->len);
		mif_print_data(skb->data, (skb->len < 64 ? skb->len : 64));
		pr_err("\n");
	}
#endif
#if 0
	if (iod->format == IPC_RFS) {
		pr_err("\n[LNK] <%s:%s> Tx HDLC RFS frame (len %d)\n",
			__func__, iod->name, skb->len);
		mif_print_data(skb->data, (skb->len < 64 ? skb->len : 64));
		pr_err("\n");
	}
#endif

	/* send data with sk_buff, link device will put sk_buff
	 * into the specific sk_buff_q and run work-q to send data
	 */
	tx_size = skb->len;
	err = iod->link->send(iod->link, iod, skb);
	if (err < 0)
		return err;

	if (err != tx_size)
		pr_err("mif: %s: WARNNING: wrong tx size: %s, format=%d "
			"count=%d, tx_size=%d, return_size=%d", __func__,
			iod->name, iod->format, count, tx_size, err);

	return count;
}

static ssize_t misc_read(struct file *filp, char *buf, size_t count,
			loff_t *f_pos)
{
	struct io_device *iod = (struct io_device *)filp->private_data;
	struct sk_buff *skb;
	int pktsize = 0;

	skb = skb_dequeue(&iod->sk_rx_q);
	if (!skb) {
		pr_err("[IOD] no data from sk_rx_q\n");
		return 0;
	}

	if (skb->len > count) {
		pr_err("[IOD] skb len is too big = %d,%d!(%d)\n",
				count, skb->len, __LINE__);
		dev_kfree_skb_any(skb);
		return -EFAULT;
	}
	pr_debug("[IOD] skb len : %d\n", skb->len);

	pktsize = skb->len;
	if (copy_to_user(buf, skb->data, pktsize) != 0) {
		dev_kfree_skb_any(skb);
		return -EFAULT;
	}
	dev_kfree_skb_any(skb);

	pr_debug("[IOD] copy to user : %d\n", pktsize);

	return pktsize;
}

#ifdef CONFIG_LINK_DEVICE_C2C
static int misc_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int r = 0;
	unsigned long size = 0;
	unsigned long pfn = 0;
	unsigned long offset = 0;
	struct io_device *iod = (struct io_device *)filp->private_data;

	if (!vma)
		return -EFAULT;

	size = vma->vm_end - vma->vm_start;
	offset = vma->vm_pgoff << PAGE_SHIFT;
	if (offset + size > (C2C_CP_RGN_SIZE + C2C_SH_RGN_SIZE)) {
		iod_err(iod, "offset + size > C2C_CP_RGN_SIZE\n");
		return -EINVAL;
	}

	/* Set the noncacheable property to the region */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
	vma->vm_flags |= VM_RESERVED | VM_IO;

	pfn = __phys_to_pfn(C2C_CP_RGN_ADDR + offset);
	r = remap_pfn_range(vma, vma->vm_start, pfn, size, vma->vm_page_prot);
	if (r) {
		iod_err(iod, "Failed in remap_pfn_range()!!!\n");
		return -EAGAIN;
	}

	pr_err("[IOD] <%s> VA = 0x%08lx, offset = 0x%lx, size = %lu\n",
		__func__, vma->vm_start, offset, size);

	return 0;
}
#endif

static const struct file_operations misc_io_fops = {
	.owner = THIS_MODULE,
	.open = misc_open,
	.release = misc_release,
	.poll = misc_poll,
	.unlocked_ioctl = misc_ioctl,
	.write = misc_write,
	.read = misc_read,
#ifdef CONFIG_LINK_DEVICE_C2C
	.mmap = misc_mmap,
#endif
};

static int vnet_open(struct net_device *ndev)
{
	netif_start_queue(ndev);
	return 0;
}

static int vnet_stop(struct net_device *ndev)
{
	netif_stop_queue(ndev);
	return 0;
}

static int vnet_xmit(struct sk_buff *skb, struct net_device *ndev)
{
	int ret;
	int skb_pull_bytes = 0;
	int headroom, tailroom;
	struct raw_hdr hd;
	struct sk_buff *skb_new;
	struct vnet *vnet = netdev_priv(ndev);
	struct io_device *iod = vnet->iod;

	/* umts doesn't need to discard ethernet header */
	if (iod->use_handover) {
		if (iod->id >= PSD_DATA_CHID_BEGIN &&
			iod->id <= PSD_DATA_CHID_END)
			skb_pull_bytes = sizeof(struct ethhdr);
	}

	hd.len = skb->len + sizeof(hd);
	hd.control = 0;
	hd.channel = iod->id & 0x1F;

	headroom = sizeof(hd) + sizeof(hdlc_start) + skb_pull_bytes;
	tailroom = sizeof(hdlc_end);
	if (likely(iod->link->aligned))
		tailroom += MAX_LINK_PADDING_SIZE;
	if (skb_headroom(skb) < headroom || skb_tailroom(skb) < tailroom) {
		skb_new = skb_copy_expand(skb, headroom, tailroom, GFP_ATOMIC);
		/* skb_copy_expand success or not, free old skb from caller */
		dev_kfree_skb_any(skb);
		if (!skb_new)
			return -ENOMEM;
	} else
		skb_new = skb;

	if (skb_pull_bytes)
		skb_pull(skb_new, skb_pull_bytes);

	memcpy(skb_push(skb_new, sizeof(hd)), &hd, sizeof(hd));
	memcpy(skb_push(skb_new, sizeof(hdlc_start)), hdlc_start,
				sizeof(hdlc_start));
	memcpy(skb_put(skb_new, sizeof(hdlc_end)), hdlc_end, sizeof(hdlc_end));
	skb_put(skb_new, calc_padding_size(iod, iod->link,  skb_new->len));

	ret = iod->link->send(iod->link, iod, skb_new);
	if (ret < 0) {
		netif_stop_queue(ndev);
		dev_kfree_skb_any(skb_new);
		return NETDEV_TX_BUSY;
	}

	ndev->stats.tx_packets++;
	ndev->stats.tx_bytes += skb->len;

	return NETDEV_TX_OK;
}

static struct net_device_ops vnet_ops = {
	.ndo_open = vnet_open,
	.ndo_stop = vnet_stop,
	.ndo_start_xmit = vnet_xmit,
};

static void vnet_setup(struct net_device *ndev)
{
	ndev->netdev_ops = &vnet_ops;
	ndev->type = ARPHRD_PPP;
	ndev->flags = IFF_POINTOPOINT | IFF_NOARP | IFF_MULTICAST;
	ndev->addr_len = 0;
	ndev->hard_header_len = 0;
	ndev->tx_queue_len = 1000;
	ndev->mtu = ETH_DATA_LEN;
	ndev->watchdog_timeo = 5 * HZ;
}

static void vnet_setup_ether(struct net_device *ndev)
{
	ndev->netdev_ops = &vnet_ops;
	ndev->type = ARPHRD_ETHER;
	ndev->flags = IFF_POINTOPOINT | IFF_NOARP | IFF_MULTICAST | IFF_SLAVE;
	ndev->addr_len = ETH_ALEN;
	random_ether_addr(ndev->dev_addr);
	ndev->hard_header_len = 0;
	ndev->tx_queue_len = 1000;
	ndev->mtu = ETH_DATA_LEN;
	ndev->watchdog_timeo = 5 * HZ;
}

int init_io_device(struct io_device *iod)
{
	int ret = 0;
	struct vnet *vnet;

	/* Get modem state from modem control device */
	iod->modem_state_changed = io_dev_modem_state_changed;

	/* Get data from link device */
	iod->recv = io_dev_recv_data_from_link_dev;

	INIT_LIST_HEAD(&iod->list);

	/* Register misc or net device */
	switch (iod->io_typ) {
	case IODEV_MISC:
		init_waitqueue_head(&iod->wq);
		skb_queue_head_init(&iod->sk_rx_q);
		INIT_DELAYED_WORK(&iod->rx_work, rx_iodev_work);

		iod->miscdev.minor = MISC_DYNAMIC_MINOR;
		iod->miscdev.name = iod->name;
		iod->miscdev.fops = &misc_io_fops;

		ret = misc_register(&iod->miscdev);
		if (ret)
			pr_err("failed to register misc io device : %s\n",
						iod->name);

		break;

	case IODEV_NET:
		skb_queue_head_init(&iod->sk_rx_q);
		if (iod->use_handover)
			iod->ndev = alloc_netdev(0, iod->name,
						vnet_setup_ether);
		else
			iod->ndev = alloc_netdev(0, iod->name, vnet_setup);

		if (!iod->ndev) {
			pr_err("failed to alloc netdev\n");
			return -ENOMEM;
		}

		ret = register_netdev(iod->ndev);
		if (ret)
			free_netdev(iod->ndev);

		pr_debug("%s: %d(iod:0x%p)\n", __func__, __LINE__, iod);
		vnet = netdev_priv(iod->ndev);
		pr_debug("%s: %d(vnet:0x%p)\n", __func__, __LINE__, vnet);
		vnet->iod = iod;

		break;

	case IODEV_DUMMY:
		skb_queue_head_init(&iod->sk_rx_q);
		INIT_DELAYED_WORK(&iod->rx_work, rx_iodev_work);

		iod->miscdev.minor = MISC_DYNAMIC_MINOR;
		iod->miscdev.name = iod->name;
		iod->miscdev.fops = &misc_io_fops;

		ret = misc_register(&iod->miscdev);
		if (ret)
			pr_err("failed to register misc io device : %s\n",
						iod->name);
		ret = device_create_file(iod->miscdev.this_device,
				&attr_waketime);
		if (ret)
			pr_err("failed to create sysfs file : %s\n", iod->name);

		break;

	default:
		pr_err("wrong io_type : %d\n", iod->io_typ);
		return -EINVAL;
	}

	pr_debug("[IOD] %s(%d) : init_io_device() done : %d\n",
				iod->name, iod->io_typ, ret);
	return ret;
}
