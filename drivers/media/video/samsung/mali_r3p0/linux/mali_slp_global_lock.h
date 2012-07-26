/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __SLP_GLOBAL_LOCK_H__
#define __SLP_GLOBAL_LOCK_H__

#include <linux/ioctl.h>

static char sgl_dev_name[] = "slp_global_lock";

#define SGL_IOC_BASE				0x32

struct sgl_attribute {
	unsigned int key;
	unsigned int timeout_ms;
};

struct sgl_user_data {
	unsigned int key;
	unsigned int data1;
	unsigned int data2;
	unsigned int locked;
};

typedef enum {
	_SGL_INIT_LOCK = 1,
	_SGL_DESTROY_LOCK,
	_SGL_LOCK_LOCK,
	_SGL_UNLOCK_LOCK,
	_SGL_SET_DATA,
	_SGL_GET_DATA,
} _sgl_ioctls;

#define SGL_IOC_INIT_LOCK			_IOW(SGL_IOC_BASE, _SGL_INIT_LOCK, struct sgl_attribute *)
#define SGL_IOC_DESTROY_LOCK		_IOW(SGL_IOC_BASE, _SGL_DESTROY_LOCK, unsigned int)
#define SGL_IOC_LOCK_LOCK			_IOW(SGL_IOC_BASE, _SGL_LOCK_LOCK, unsigned int)
#define SGL_IOC_UNLOCK_LOCK			_IOW(SGL_IOC_BASE, _SGL_UNLOCK_LOCK, unsigned int)
#define SGL_IOC_SET_DATA			_IOW(SGL_IOC_BASE, _SGL_SET_DATA, struct sgl_user_data *)
#define SGL_IOC_GET_DATA			_IOW(SGL_IOC_BASE, _SGL_GET_DATA, struct sgl_user_data *)

#endif /* __SLP_GLOBAL_LOCK_H__ */
