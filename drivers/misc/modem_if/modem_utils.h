/*
 * Copyright (C) 2011 Samsung Electronics.
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

#ifndef __MODEM_UTILS_H__
#define __MODEM_UTILS_H__

#define MAX_SKB_LOG_LEN ((size_t)16)
#define RAW_DEV(rdevs, i) (((struct io_raw_devices *)rdevs)->raw_devices[i])

/**
 * raw_devs_for_each - iterate raw devices of multi raw device
 * @iod:	struct io_device *iod
 * @index:	int index
 * @multiraw:	struct io_device *multiraw
 */
#define raw_devs_for_each(multiraw, index, iod) \
	for (index = 0; iod = RAW_DEV(multiraw->private_data, index), \
		index < MAX_RAW_DEVS; index++) \
		if (iod)

/**
 * io_devs_for_each - iterate io devices of list_of_io_devices
 * @iod:	struct io_device *iod
 * @ld:		struct link_device *ld
 */
#define io_devs_for_each(iod, ld) \
	list_for_each_entry(iod, (ld)->list_of_io_devices, list) \
		if (iod->link_types & LINKTYPE((ld)->link_type))


static inline struct io_device *find_iodev(struct link_device *ld,
		enum dev_format format)
{
	struct io_device *iod;

	io_devs_for_each(iod, ld) {
		if (iod->format == format)
			return iod;
	}
	return NULL;
}

/** countbits - count number of 1 bits as fastest way
 * @n: number
 */
static inline unsigned int countbits(unsigned int n)
{
	unsigned int i;
	for (i = 0; n != 0; i++)
		n &= (n - 1);
	return i;
}

/* print buffer as hex string */
int pr_buffer(const char *tag, const char *data, size_t data_len,
							size_t max_len);

/* print a sk_buff as hex string */
static inline int pr_skb(const char *tag, struct sk_buff *skb)
{
	return pr_buffer(tag, skb->data, skb->len, MAX_SKB_LOG_LEN);
}

/* flow control CMD from CP, it use in serial devices */
int link_rx_flowctl_cmd(struct link_device *ld, const char *data, size_t len);

void mif_print_data(char *buf, int len);
void print_sipc4_hdlc_fmt_frame(const u8 *psrc);
void print_sipc4_fmt_frame(const u8 *psrc);

#endif/*__MODEM_UTILS_H__*/
