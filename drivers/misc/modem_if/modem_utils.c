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

#include <linux/netdevice.h>
#include <linux/platform_data/modem.h>
#include <linux/platform_device.h>
#include <linux/skbuff.h>

#include "modem_prj.h"
#include "modem_utils.h"

#define CMD_SUSPEND	((unsigned short)(0x00CA))
#define CMD_RESUME	((unsigned short)(0x00CB))

/* dump2hex
 * dump data to hex as fast as possible.
 * the length of @buf must be greater than "@len * 3"
 * it need 3 bytes per one data byte to print.
 */
static inline int dump2hex(char *buf, const char *data, size_t len)
{
	static const char *hex = "0123456789abcdef";
	char *dest = buf;
	int i;

	for (i = 0; i < len; i++) {
		*dest++ = hex[(data[i] >> 4) & 0xf];
		*dest++ = hex[data[i] & 0xf];
		*dest++ = ' ';
	}
	if (likely(len > 0))
		dest--; /* last space will be overwrited with null */

	*dest = '\0';

	return dest - buf;
}

/* print buffer as hex string */
int pr_buffer(const char *tag, const char *data, size_t data_len,
							size_t max_len)
{
	size_t len = min(data_len, max_len);
	unsigned char hexstr[len ? len * 3 : 1]; /* 1 <= sizeof <= max_len*3 */
	dump2hex(hexstr, data, len);
	return printk(KERN_DEBUG "[MIF] %s(%u): %s%s\n", tag, data_len, hexstr,
			len == data_len ? "" : " ...");
}

/* flow control CMD from CP, it use in serial devices */
int link_rx_flowctl_cmd(struct link_device *ld, const char *data, size_t len)
{
	unsigned short *cmd, *end = (unsigned short *)(data + len);
	struct io_device *iod = NULL, *multi_raw_iod;
	int i;

	pr_debug("[MODEM_IF] flow control cmd: size=%d\n", len);

	multi_raw_iod = find_iodev(ld, IPC_MULTI_RAW);
	if (!multi_raw_iod || !multi_raw_iod->private_data) {
		pr_err("[MODEM_IF] %s: no multi raw device\n", __func__);
		return -ENODEV;
	}

	for (cmd = (unsigned short *)data; cmd < end; cmd++) {
		char dev_ids[MAX_RAW_DEVS * 3 + 1] = ""; /* 3bytes per id */
		char *str = dev_ids;

		switch (*cmd) {
		case CMD_SUSPEND:
			raw_devs_for_each(multi_raw_iod, i, iod) {
				if (iod->io_typ == IODEV_NET && iod->ndev)
					netif_stop_queue(iod->ndev);

				str += sprintf(str, " %02x", iod->id);
			}
			ld->raw_tx_suspended = true;
			pr_info("[MODEM_IF] flowctl CMD_SUSPEND(%04X):%s\n",
					*cmd, dev_ids);
			break;

		case CMD_RESUME:
			raw_devs_for_each(multi_raw_iod, i, iod) {
				if (iod->io_typ == IODEV_NET && iod->ndev)
					netif_wake_queue(iod->ndev);

				str += sprintf(str, " %02x", iod->id);
			}
			ld->raw_tx_suspended = false;
			complete_all(&ld->raw_tx_resumed_by_cp);
			pr_info("[MODEM_IF] flowctl CMD_RESUME(%04X):%s\n",
					*cmd, dev_ids);
			break;

		default:
			pr_err("[MODEM_IF] flowctl BAD CMD: %04X\n", *cmd);
			break;
		}
	}

	return 0;
}

void mif_print_data(char *buf, int len)
{
	int   words = len >> 4;
	int   residue = len - (words << 4);
	int   i = 0;
	char *b = NULL;
	char  last[80];
	char  tb[8];

	/* Make the last line, if ((len % 16) > 0) */
	if (residue > 0) {
		memset(last, 0, sizeof(last));
		memset(tb, 0, sizeof(tb));
		b = buf + (words << 4);

		sprintf(last, "%04X: ", (words << 4));
		for (i = 0; i < residue; i++) {
			sprintf(tb, "%02x ", b[i]);
			strcat(last, tb);
			if ((i & 0x3) == 0x3) {
				sprintf(tb, " ");
				strcat(last, tb);
			}
		}
	}

	for (i = 0; i < words; i++) {
		b = buf + (i << 4);
		printk(KERN_INFO "%04X: "
			"%02x %02x %02x %02x  %02x %02x %02x %02x  "
			"%02x %02x %02x %02x  %02x %02x %02x %02x\n",
			(i << 4),
			b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7],
			b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
	}

	/* Print the last line */
	if (residue > 0)
		printk(KERN_INFO "%s\n", last);
}

void print_sipc4_hdlc_fmt_frame(const u8 *psrc)
{
	u8  *hf = NULL;					/* HDLC Frame	*/
	struct sipc4_hdlc_fmt_hdr *hh = NULL;		/* HDLC Header	*/
	struct sipc4_fmt_hdr      *ih = NULL;		/* IPC Header	*/
	u16  hh_len = sizeof(struct sipc4_hdlc_fmt_hdr);
	u16  ih_len = sizeof(struct sipc4_fmt_hdr);
	u8  *data = NULL;
	int  data_len = 0;

	/* Actual HDLC header starts from after START flag (0x7F) */
	hf = (u8 *)(psrc + 1);

	/* Point HDLC header and IPC header */
	hh = (struct sipc4_hdlc_fmt_hdr *)(hf);
	ih = (struct sipc4_fmt_hdr *)(hf + hh_len);

	/* Point IPC data */
	data     = hf + (hh_len + ih_len);
	data_len = hh->len - (hh_len + ih_len);

	pr_err("--------------------HDLC & FMT HEADER----------------------\n");

	pr_err("HDLC len = %d, HDLC control = 0x%02x\n", hh->len, hh->control);

	pr_err("(M)0x%02X, (S)0x%02X, (T)0x%02X, mseq:%d, aseq:%d, len:%d\n",
		ih->main_cmd, ih->sub_cmd, ih->cmd_type,
		ih->msg_seq, ih->ack_seq, ih->len);

	pr_err("-----------------------IPC FMT DATA------------------------\n");

	if (data_len > 0) {
		if (data_len > 64)
			data_len = 64;
		mif_print_data(data, data_len);
	}

	pr_err("-----------------------------------------------------------\n");
}

void print_sipc4_fmt_frame(const u8 *psrc)
{
	struct sipc4_fmt_hdr *ih = (struct sipc4_fmt_hdr *)psrc;
	u16  ih_len = sizeof(struct sipc4_fmt_hdr);
	u8  *data = NULL;
	int  data_len = 0;

	/* Point IPC data */
	data     = (u8 *)(psrc + ih_len);
	data_len = ih->len - ih_len;

	pr_err("----------------------IPC FMT HEADER-----------------------\n");

	pr_err("(M)0x%02X, (S)0x%02X, (T)0x%02X, mseq:%d, aseq:%d, len:%d\n",
		ih->main_cmd, ih->sub_cmd, ih->cmd_type,
		ih->msg_seq, ih->ack_seq, ih->len);

	pr_err("-----------------------IPC FMT DATA------------------------\n");

	if (data_len > 0)
		mif_print_data(data, data_len);

	pr_err("-----------------------------------------------------------\n");
}

