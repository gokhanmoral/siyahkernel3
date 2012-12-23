/* linux/drivers/video/samsung/mdnie_tuning.c
 *
 * Register interface file for Samsung mDNIe driver
 *
 * Copyright (c) 2011 Samsung Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/ctype.h>
#include <linux/uaccess.h>

#include "s3cfb.h"
#include "mdnie.h"

static int parse_text(char *src, int len, u16 *mdnie_data)
{
	int i, count, ret;
	int index = 0;
	char *str_line[100];
	char *sstart;
	char *c;
	unsigned int data1, data2;

	c = src;
	count = 0;
	sstart = c;

	for (i = 0; i < len; i++, c++) {
		char a = *c;
		if (a == '\r' || a == '\n') {
			if (c > sstart) {
				str_line[count] = sstart;
				count++;
			}
			*c = '\0';
			sstart = c+1;
		}
	}

	if (c > sstart) {
		str_line[count] = sstart;
		count++;
	}

	/* printk(KERN_INFO "----------------------------- Total number of lines:%d\n", count); */

	for (i = 0; i < count; i++) {
		/* printk(KERN_INFO "line:%d, [start]%s[end]\n", i, str_line[i]); */
		ret = sscanf(str_line[i], "0x%x, 0x%x\n", &data1, &data2);
		/* printk(KERN_INFO "Result => [0x%2x 0x%4x] %s\n", data1, data2, (ret == 2) ? "Ok" : "Not available"); */
		if (ret == 2) {
			mdnie_data[index++] = (unsigned short)data1;
			mdnie_data[index++] = (unsigned short)data2;
		}
	}

	mdnie_data[index] = END_SEQ;

	return index;
}

int mdnie_txtbuf_to_parsing(char const *pFilepath, u16 *buf, u16 size)
{
	struct file *filp;
	char	*dp;
	long	l;
	loff_t  pos;
	int     ret, num;
	mm_segment_t fs;

	fs = get_fs();
	set_fs(get_ds());

	printk(KERN_INFO "%s:", pFilepath);

	if (!pFilepath) {
		printk(KERN_ERR "Error : mdnie_txtbuf_to_parsing has invalid filepath.\n");
		goto parse_err;
	}

	filp = filp_open(pFilepath, O_RDONLY, 0);

	if (IS_ERR(filp)) {
		printk(KERN_ERR "file open error:%d\n", (s32)filp);
		goto parse_err;
	}

	l = filp->f_path.dentry->d_inode->i_size;
	dp = kmalloc(l, GFP_KERNEL);
	if (dp == NULL) {
		printk(KERN_INFO "Out of Memory!\n");
		filp_close(filp, current->files);
		goto parse_err;
	}
	pos = 0;
	memset(dp, 0, l);
	ret = vfs_read(filp, (char __user *)dp, l, &pos);

	if (ret != l) {
		printk(KERN_INFO "ret = %d, l = %ld, %s\n", ret, l, pFilepath);
		l = (l > ret) ? ret : l;
		if (size < l) {
			kfree(dp);
			filp_close(filp, current->files);
			goto parse_err;
		}
	}

	filp_close(filp, current->files);
	set_fs(fs);
	num = parse_text(dp, l, buf);
	if (!num) {
		printk(KERN_ERR "Nothing to parse!\n");
		kfree(dp);
		goto parse_err;
	}

	kfree(dp);

	num = num / 2;
	return num;

parse_err:
	return -EPERM;
}

