/*
 * driver for Fusitju M9MO LS 8MP camera
 *
 * Copyright (c) 2010, Samsung Electronics. All rights reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <linux/i2c.h>
#include <linux/init.h>
#include <media/v4l2-device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/vmalloc.h>
#include <linux/firmware.h>
#include <linux/videodev2.h>

#ifdef CONFIG_VIDEO_SAMSUNG_V4L2
#include <linux/videodev2_exynos_camera.h>
#include <linux/videodev2_exynos_media.h>
#endif

#include <linux/regulator/machine.h>

#include <media/m9mo_platform.h>
#include "m9mo.h"

#define M9MO_DRIVER_NAME	"M9MO"

extern struct class *camera_class;
struct device *m9mo_dev;

#if 0
#define M9MO_FW_PATH		"/data/RS_M9MO.bin"
#define FW_INFO_PATH		"/data/FW_INFO.bin"
#endif

#define M9MO_FW_PATH		"/sdcard/RS_M9MO.bin"

#define M9MO_FW_REQ_PATH	"RS_M9MO.bin"
#define M9MO_EVT31_FW_REQ_PATH	"RS_M9MO_EVT3.1.bin"

#define FW_INFO_PATH		"/sdcard/FW_INFO.bin"


#define M9MO_FW_DUMP_PATH	"/data/RS_M9LS_dump.bin"
#define M9MO_FACTORY_CSV_PATH	"/data/FACTORY_CSV_RAW.bin"

#define M9MOTB_FW_PATH "RS_M9LS_TB.bin" /* TECHWIN - SONY */
/* #define M9MOON_FW_PATH "RS_M9LS_ON.bin" */ /* FIBEROPTICS - SONY */
/* #define M9MOOM_FW_PATH "RS_M9LS_OM.bin" */ /* FIBEROPTICS - S.LSI */
#if defined(CONFIG_MACH_U1_KOR_LGT)
#define M9MOSB_FW_PATH "RS_M9LS_SB.bin" /* ELECTRO-MECHANICS - SONY */
#endif
/* #define M9MOSC_FW_PATH "RS_M9LS_SC.bin" */ /* ELECTRO-MECHANICS - S.LSI */
/* #define M9MOCB_FW_PATH "RS_M9LS_CB.bin" */ /* CAMSYS - SONY */
#if defined(CONFIG_TARGET_LOCALE_NA)
/* #define M9MOOE_FW_PATH "RS_M9LS_OE.bin" */ /* FIBEROPTICS - SONY */
#endif
#if defined(CONFIG_MACH_Q1_BD)
#define M9MOOO_FW_PATH "RS_M9LS_OO.bin" /* FIBEROPTICS - SONY */
#endif

#if 0
#define M9MO_FW_VER_LEN		22
#define M9MO_FW_VER_FILE_CUR	0x16FF00
#else
#define M9MO_FW_VER_LEN		2
#define M9MO_SEN_FW_VER_LEN	30
#define M9MO_FW_VER_FILE_CUR	0x1FF080
#define M9MO_FW_VER_NUM		0x000018
#endif

#define M9MO_FLASH_BASE_ADDR	0x00000000

#define M9MO_FLASH_READ_BASE_ADDR	0x000000

#define M9MO_FLASH_BASE_ADDR_1	0x001FF000

u32 M9MO_FLASH_FACTORY_OIS[] = {0x27E031A2, 0x27E031C7};
u32 M9MO_FLASH_FACTORY_VIB[] = {0x27E031C8, 0x27E031D1};
u32 M9MO_FLASH_FACTORY_GYRO[] = {0x27E031D2, 0x27E031D7};
u32 M9MO_FLASH_FACTORY_TELE_RESOL[] = {0x27E03298, 0x27E0329F};
u32 M9MO_FLASH_FACTORY_WIDE_RESOL[] = {0x27E032A0, 0x27E032A7};
u32 M9MO_FLASH_FACTORY_AF_FCS[] = {0x27E0323A, 0x27E03275};
u32 M9MO_FLASH_FACTORY_PUNT[] = {0x27E031D8, 0x27E03239};

u32 M9MO_FLASH_FACTORY_BACKLASH[] = {0x27E03276, 0x27E03279};

u32 M9MO_FLASH_FACTORY_AF_LED[] = {0x27E032A8, 0x27E032AD};
u32 M9MO_FLASH_FACTORY_IRIS[] = {0x27E030E8, 0x27E03107};
u32 M9MO_FLASH_FACTORY_LIVEVIEW[] = {0x27E03108, 0x27E0310F};
u32 M9MO_FLASH_FACTORY_GAIN_CAPTURE[] = {0x27E03110, 0x27E03117};
u32 M9MO_FLASH_FACTORY_SH_CLOSE[] = {0x27E0327A, 0x27E03297};
u32 M9MO_FLASH_FACTORY_FLASH_CHECK[] = {0x27E032AE, 0x27E032B0};
u32 M9MO_FLASH_FACTORY_WB_ADJ[] = {0x27E03000, 0x27E03027};

u32 M9MO_FLASH_FACTORY_RESULT = 0x27E03128;

#define M9MO_INT_RAM_BASE_ADDR	0x01100000

#define M9MO_I2C_RETRY		5
#define M9MO_I2C_VERIFY		100
#define M9MO_ISP_TIMEOUT	5000  /* timeout delay for m9mo 3000->5000 */
#define M9MO_ISP_AFB_TIMEOUT	15000 /* FIXME */
#define M9MO_ISP_ESD_TIMEOUT	1000

#define M9MO_JPEG_MAXSIZE	0x990000 /* 10M 4K align */
#define M9MO_THUMB_MAXSIZE	0x0
#define M9MO_POST_MAXSIZE	0x0

#define M9MO_DEF_APEX_DEN	100

#define m9mo_readb(sd, g, b, v)		m9mo_read(__LINE__, sd, 1, g, b, v)
#define m9mo_readw(sd, g, b, v)		m9mo_read(__LINE__, sd, 2, g, b, v)
#define m9mo_readl(sd, g, b, v)		m9mo_read(__LINE__, sd, 4, g, b, v)

#define m9mo_writeb(sd, g, b, v)	m9mo_write(__LINE__, sd, 1, g, b, v)
#define m9mo_writew(sd, g, b, v)	m9mo_write(__LINE__, sd, 2, g, b, v)
#define m9mo_writel(sd, g, b, v)	m9mo_write(__LINE__, sd, 4, g, b, v)

#define CHECK_ERR(x)	if ((x) < 0) { \
				cam_err("i2c failed, err %d\n", x); \
				return x; \
			}

#define NELEMS(array) (sizeof(array) / sizeof(array[0]))

#if 0
#define FAST_CAPTURE
#endif

static const struct m9mo_frmsizeenum preview_frmsizes[] = {
	{ M9MO_PREVIEW_QCIF,	176,	144,	0x05 },	/* 176 x 144 */
	{ M9MO_PREVIEW_QVGA,	320,	240,	0x09 },
	{ M9MO_PREVIEW_VGA,	640,	480,	0x17 },
	{ M9MO_PREVIEW_D1,	720,	480,	0x33 }, /* High speed */
	{ M9MO_PREVIEW_WVGA,	800,	480,	0x1A },
	{ M9MO_PREVIEW_960_720,	960,	720,	0x34 },
	{ M9MO_PREVIEW_720P,	1280,	720,	0x21 },
	{ M9MO_PREVIEW_1080P,	1920,	1080,	0x28 },
	{ M9MO_PREVIEW_HDR,	3264,	2448,	0x27 },
	{ M9MO_PREVIEW_720P_60FPS,	1280, 720,	   0x25 },
	{ M9MO_PREVIEW_VGA_60FPS,	640, 480,	   0x2F },
};

static const struct m9mo_frmsizeenum dual_preview_frmsizes[] = {
	{ M9MO_PREVIEW_DUAL_1080P,	1920,	1080,	0x2C },
	{ M9MO_PREVIEW_DUAL_720P,	1280,	720,	0x2D },
	{ M9MO_PREVIEW_DUAL_VGA,	640,	480,	0x2E },
};

static const struct m9mo_frmsizeenum capture_frmsizes[] = {
	{ M9MO_CAPTURE_1MP,	1024,	768,	0x0F },
	{ M9MO_CAPTURE_2MPW,	1920,	1080,	0x19 },
	{ M9MO_CAPTURE_3MP,	1984,	1488,	0x2F },
	{ M9MO_CAPTURE_4MP,	2304,	1728,	0x1E },
	{ M9MO_CAPTURE_5MP,	2592,	1944,	0x20 },
	{ M9MO_CAPTURE_8MP,	3264,	2448,	0x25 },
	{ M9MO_CAPTURE_10MP,	3648,	2736,	0x30 },
	{ M9MO_CAPTURE_12MPW,	4608,	2592,	0x31 },
	{ M9MO_CAPTURE_14MP,	4608,	3072,	0x32 },
	{ M9MO_CAPTURE_16MP,	4608,	3456,	0x33 },
	/* for Postview size */
	{ M9MO_CAPTURE_POSTWVGA,	800,	480,	0x09 },
	{ M9MO_CAPTURE_POSTVGA,		640,	480,	0x08 },
	{ M9MO_CAPTURE_POSTWHD,		1280,	720,	0x0F },
	{ M9MO_CAPTURE_POSTHD,		960,	720,	0x13 },
};

static struct m9mo_control m9mo_ctrls[] = {
	{
		.id = V4L2_CID_CAMERA_ISO,
		.minimum = ISO_AUTO,
		.maximum = ISO_3200,
		.step = 1,
		.value = ISO_AUTO,
		.default_value = ISO_AUTO,
	}, {
		.id = V4L2_CID_CAMERA_BRIGHTNESS,
		.minimum = EXPOSURE_MINUS_6,
		.maximum = EXPOSURE_PLUS_6,
		.step = 1,
		.value = EXPOSURE_DEFAULT,
		.default_value = EXPOSURE_DEFAULT,
	}, {
		.id = V4L2_CID_CAMERA_SATURATION,
		.minimum = SATURATION_MINUS_2,
		.maximum = SATURATION_MAX - 1,
		.step = 1,
		.value = SATURATION_DEFAULT,
		.default_value = SATURATION_DEFAULT,
	}, {
		.id = V4L2_CID_CAMERA_SHARPNESS,
		.minimum = SHARPNESS_MINUS_2,
		.maximum = SHARPNESS_MAX - 1,
		.step = 1,
		.value = SHARPNESS_DEFAULT,
		.default_value = SHARPNESS_DEFAULT,
	}, {
		.id = V4L2_CID_CAMERA_ZOOM,
		.minimum = ZOOM_LEVEL_0,
		.maximum = ZOOM_LEVEL_MAX - 1,
		.step = 1,
		.value = ZOOM_LEVEL_0,
		.default_value = ZOOM_LEVEL_0,
	}, {
		.id = V4L2_CID_CAM_JPEG_QUALITY,
		.minimum = 1,
		.maximum = 100,
		.step = 1,
		.value = 100,
		.default_value = 100,
	}, {
		.id = V4L2_CID_CAMERA_ANTI_BANDING,
		.minimum = ANTI_BANDING_AUTO,
		.maximum = ANTI_BANDING_OFF,
		.step = 1,
		.value = ANTI_BANDING_50HZ,
		.default_value = ANTI_BANDING_50HZ,
	},
};

static u8 sysfs_sensor_fw[30] = {0,};
static u8 sysfs_phone_fw[5] = {0,};
static u8 sysfs_sensor_type[15] = {0,};

static inline struct m9mo_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct m9mo_state, sd);
}

static int m9mo_read(int _line, struct v4l2_subdev *sd,
	u8 len, u8 category, u8 byte, int *val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg msg;
	unsigned char data[5];
	unsigned char recv_data[len + 1];
	int i, err = 0;

	if (!client->adapter)
		return -ENODEV;

	if (len != 0x01 && len != 0x02 && len != 0x04)
		return -EINVAL;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = sizeof(data);
	msg.buf = data;

	/* high byte goes out first */
	data[0] = msg.len;
	data[1] = 0x01;			/* Read category parameters */
	data[2] = category;
	data[3] = byte;
	data[4] = len;

	for (i = M9MO_I2C_RETRY; i; i--) {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			break;
		msleep(20);
	}

	if (err != 1) {
		cam_err("category %#x, byte %#x\n", category, byte);
		return err;
	}

	msg.flags = I2C_M_RD;
	msg.len = sizeof(recv_data);
	msg.buf = recv_data;
	for (i = M9MO_I2C_RETRY; i; i--) {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			break;
		msleep(20);
	}

	if (err != 1) {
		cam_err("category %#x, byte %#x\n", category, byte);
		return err;
	}

	if (recv_data[0] != sizeof(recv_data))
		cam_i2c_dbg("expected length %d, but return length %d\n",
				 sizeof(recv_data), recv_data[0]);

	if (len == 0x01)
		*val = recv_data[1];
	else if (len == 0x02)
		*val = recv_data[1] << 8 | recv_data[2];
	else
		*val = recv_data[1] << 24 | recv_data[2] << 16 |
				recv_data[3] << 8 | recv_data[4];

	cam_i2c_dbg("m9mo Read [ %4d ] %#02x, byte %#x, value %#x\n",
					_line, category, byte, *val);

	return err;
}

static int m9mo_write(int _line, struct v4l2_subdev *sd,
	u8 len, u8 category, u8 byte, int val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg msg;
	unsigned char data[len + 4];
	int i, err;

	if (!client->adapter)
		return -ENODEV;

	if (len != 0x01 && len != 0x02 && len != 0x04)
		return -EINVAL;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = sizeof(data);
	msg.buf = data;

	data[0] = msg.len;
	data[1] = 0x02;			/* Write category parameters */
	data[2] = category;
	data[3] = byte;
	if (len == 0x01) {
		data[4] = val & 0xFF;
	} else if (len == 0x02) {
		data[4] = (val >> 8) & 0xFF;
		data[5] = val & 0xFF;
	} else {
		data[4] = (val >> 24) & 0xFF;
		data[5] = (val >> 16) & 0xFF;
		data[6] = (val >> 8) & 0xFF;
		data[7] = val & 0xFF;
	}

	cam_i2c_dbg("m9mo Write [ %4d ] %#x, byte %#x, value %#x\n",
		_line, category, byte, val);

	for (i = M9MO_I2C_RETRY; i; i--) {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			break;
		msleep(20);
	}

	return err;
}
static int m9mo_mem_dump(struct v4l2_subdev *sd, u16 len, u32 addr, u8 *val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg msg;
	unsigned char data[8];
	unsigned char recv_data[len + 3];
	int i, err = 0;

	if (!client->adapter)
		return -ENODEV;

	if (len <= 0)
		return -EINVAL;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = sizeof(data);
	msg.buf = data;

	/* high byte goes out first */
	data[0] = 0x00;
	data[1] = 0x03;
	data[2] = 0x18;
	data[3] = (addr >> 16) & 0xFF;
	data[4] = (addr >> 8) & 0xFF;
	data[5] = addr & 0xFF;
	data[6] = (len >> 8) & 0xFF;
	data[7] = len & 0xFF;

	for (i = M9MO_I2C_RETRY; i; i--) {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			break;
		msleep(20);
	}

	if (err != 1)
		return err;

	msg.flags = I2C_M_RD;
	msg.len = sizeof(recv_data);
	msg.buf = recv_data;
	for (i = M9MO_I2C_RETRY; i; i--) {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			break;
		msleep(20);
	}

	if (err != 1)
		return err;

	if (len != (recv_data[1] << 8 | recv_data[2]))
		cam_i2c_dbg("expected length %d, but return length %d\n",
			len, recv_data[1] << 8 | recv_data[2]);

	memcpy(val, recv_data + 3, len);

	cam_i2c_dbg("address %#x, length %d\n", addr, len);
	return err;
}
static int m9mo_mem_read(struct v4l2_subdev *sd, u16 len, u32 addr, u8 *val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg msg;
	unsigned char data[8];
	unsigned char recv_data[len + 3];
	int i, err = 0;

	if (!client->adapter)
		return -ENODEV;

	if (len <= 0)
		return -EINVAL;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = sizeof(data);
	msg.buf = data;

	/* high byte goes out first */
	data[0] = 0x00;
	data[1] = 0x03;
	data[2] = (addr >> 24) & 0xFF;
	data[3] = (addr >> 16) & 0xFF;
	data[4] = (addr >> 8) & 0xFF;
	data[5] = addr & 0xFF;
	data[6] = (len >> 8) & 0xFF;
	data[7] = len & 0xFF;

	for (i = M9MO_I2C_RETRY; i; i--) {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			break;
		msleep(20);
	}

	if (err != 1)
		return err;

	msg.flags = I2C_M_RD;
	msg.len = sizeof(recv_data);
	msg.buf = recv_data;
	for (i = M9MO_I2C_RETRY; i; i--) {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			break;
		msleep(20);
	}

	if (err != 1)
		return err;

	if (len != (recv_data[1] << 8 | recv_data[2]))
		cam_i2c_dbg("expected length %d, but return length %d\n",
			len, recv_data[1] << 8 | recv_data[2]);

	memcpy(val, recv_data + 3, len);

	cam_i2c_dbg("address %#x, length %d\n", addr, len);
	return err;
}

static int m9mo_mem_write(struct v4l2_subdev *sd, u8 cmd,
		u16 len, u32 addr, u8 *val)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg msg;
	unsigned char data[len + 8];
	int i, err = 0;

	if (!client->adapter)
		return -ENODEV;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = sizeof(data);
	msg.buf = data;

	/* high byte goes out first */
	data[0] = 0x00;
	data[1] = cmd;
	data[2] = (addr >> 24) & 0xFF;
	data[3] = (addr >> 16) & 0xFF;
	data[4] = (addr >> 8) & 0xFF;
	data[5] = addr & 0xFF;
	data[6] = (len >> 8) & 0xFF;
	data[7] = len & 0xFF;
	memcpy(data + 2 + sizeof(addr) + sizeof(len), val, len);

	cam_i2c_dbg("address %#x, length %d\n", addr, len);

	for (i = M9MO_I2C_RETRY; i; i--) {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			break;
		msleep(20);
	}

	return err;
}

static irqreturn_t m9mo_isp_isr(int irq, void *dev_id)
{
	struct v4l2_subdev *sd = (struct v4l2_subdev *)dev_id;
	struct m9mo_state *state = to_state(sd);

	cam_dbg("**************** interrupt ****************\n");
	state->isp.issued = 1;
	wake_up_interruptible(&state->isp.wait);

	return IRQ_HANDLED;
}

static u32 m9mo_wait_interrupt(struct v4l2_subdev *sd,
	unsigned int timeout)
{
	struct m9mo_state *state = to_state(sd);
	cam_trace("E\n");

	if (wait_event_interruptible_timeout(state->isp.wait,
		state->isp.issued == 1,
		msecs_to_jiffies(timeout)) == 0) {
		cam_err("timeout ~~~~~~~~~~~~~~~~~~~~~\n");
		return 0;
	}

	state->isp.issued = 0;

	m9mo_readw(sd, M9MO_CATEGORY_SYS,
		M9MO_SYS_INT_FACTOR, &state->isp.int_factor);
	cam_err("state->isp.int_factor = %x\n", state->isp.int_factor);
	cam_trace("X\n");
	return state->isp.int_factor;
}

static int m9mo_wait_framesync(struct v4l2_subdev *sd)
{
	int i, frame_sync_count = 0;
	u32 int_factor;
	s32 read_val = 0;
	struct m9mo_state *state = to_state(sd);

	 if (state->running_capture_mode == RUNNING_MODE_CONTINUOUS) {
		cam_dbg("Start Continuous capture");
		frame_sync_count = 9;
	} else if (state->running_capture_mode == RUNNING_MODE_BRACKET
	       || state->running_capture_mode == RUNNING_MODE_HDR) {
		cam_dbg("Start AutoBracket(AEB) or HDR capture");
		frame_sync_count = 3;
	} else if (state->running_capture_mode == RUNNING_MODE_BLINK) {
		cam_dbg("Start FaceDetect EyeBlink capture");
		frame_sync_count = 3;
	}

	/* Clear Interrupt factor */
	for (i = frame_sync_count; i; i--) {
		int_factor = m9mo_wait_interrupt(sd,
				M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_FRAME_SYNC)) {
			cam_warn("M9MO_INT_FRAME_SYNC isn't issued, %#x\n",
					int_factor);
			return -ETIMEDOUT;
		}
		m9mo_readb(sd,
				M9MO_CATEGORY_SYS,
				M9MO_SYS_FRAMESYNC_CNT,
				&read_val);
		cam_dbg("Frame interrupt M9MO_INT_FRAME_SYNC cnt[%d]\n",
				read_val);
	}

	return 0;
}

static int m9mo_set_mode(struct v4l2_subdev *sd, u32 mode)
{
	int i, err;
	u32 old_mode, val;
	u32 int_factor;

	cam_trace("E\n");

	err = m9mo_readb(sd, M9MO_CATEGORY_SYS, M9MO_SYS_MODE, &old_mode);

	if (err < 0)
		return err;

	if (old_mode == mode) {
		cam_dbg("%#x -> %#x\n", old_mode, mode);
		return old_mode;
	}

	cam_dbg("%#x -> %#x\n", old_mode, mode);

	switch (old_mode) {
	case M9MO_SYSINIT_MODE:
		cam_warn("sensor is initializing\n");
		err = -EBUSY;
		break;

	case M9MO_PARMSET_MODE:
		if (mode == M9MO_STILLCAP_MODE) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_SYS,
				M9MO_SYS_MODE, M9MO_MONITOR_MODE);
			if (err < 0)
				break;
			for (i = M9MO_I2C_VERIFY; i; i--) {
				err = m9mo_readb(sd, M9MO_CATEGORY_SYS,
					M9MO_SYS_MODE, &val);
				if (val == M9MO_MONITOR_MODE)
					break;
				msleep(20);
			}
		}
	case M9MO_MONITOR_MODE:
	case M9MO_STILLCAP_MODE:
		err = m9mo_writeb(sd, M9MO_CATEGORY_SYS, M9MO_SYS_MODE, mode);
		break;

	default:
		cam_warn("current mode is unknown, %d\n", old_mode);
		err = 0;/* -EINVAL; */
	}

	if (err < 0)
		return err;

	if (mode == M9MO_STILLCAP_MODE) {
		m9mo_wait_framesync(sd);

		/* Clear Interrupt factor */
		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_CAPTURE)) {
			cam_warn("M9MO_INT_CAPTURE isn't issued, %#x\n",
					int_factor);
			return -ETIMEDOUT;
		}
	}

	for (i = M9MO_I2C_VERIFY; i; i--) {
		err = m9mo_readb(sd, M9MO_CATEGORY_SYS, M9MO_SYS_MODE, &val);
		if (val == mode)
			break;
		msleep(20);
	}

	cam_trace("X\n");
	return old_mode;
}

static int m9mo_set_capture_mode(struct v4l2_subdev *sd, int val)
{
	int err, capture_val, shutter_val;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E\n");

	state->running_capture_mode = val;

	err = m9mo_readb(sd, M9MO_CATEGORY_ADJST,
			M9MO_ADJST_SHUTTER_MODE, &shutter_val);
	CHECK_ERR(err);

	err = m9mo_readb(sd, M9MO_CATEGORY_CAPCTRL,
			M9MO_CAPCTRL_CAP_MODE, &capture_val);
	CHECK_ERR(err);

	switch (state->running_capture_mode) {
	case RUNNING_MODE_CONTINUOUS:
		if (shutter_val != 0x00) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
					M9MO_ADJST_SHUTTER_MODE, 0x00);
			CHECK_ERR(err);
		}

		if (capture_val != M9MO_CAP_MODE_MULTI_CAPTURE) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
					M9MO_CAPCTRL_CAP_MODE,
					M9MO_CAP_MODE_MULTI_CAPTURE);
			CHECK_ERR(err);
		}
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
				M9MO_CAPCTRL_CAP_FRM_COUNT, 0x09);
		CHECK_ERR(err);
		break;

	case RUNNING_MODE_BRACKET:
		cam_trace("~~~~~~ AutoBracket ~~~~~~\n");
		if (shutter_val != 0x00) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
					M9MO_ADJST_SHUTTER_MODE, 0x00);
			CHECK_ERR(err);
		}

		if (capture_val != M9MO_CAP_MODE_BRACKET_CAPTURE) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
					M9MO_CAPCTRL_CAP_MODE,
					M9MO_CAP_MODE_BRACKET_CAPTURE);
			CHECK_ERR(err);
		}
		break;

	case RUNNING_MODE_HDR:
		cam_trace("~~~~~~ HDRmode capture ~~~~~~\n");
		if (shutter_val != 0x00) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
					M9MO_ADJST_SHUTTER_MODE, 0x00);
			CHECK_ERR(err);
		}

		if (capture_val != M9MO_CAP_MODE_BRACKET_CAPTURE) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
					M9MO_CAPCTRL_CAP_MODE,
					M9MO_CAP_MODE_BRACKET_CAPTURE);
			CHECK_ERR(err);
		}
		break;

	case RUNNING_MODE_BLINK:
		cam_trace("~~~~~~ EyeBlink capture ~~~~~~\n");
		if (shutter_val != 0x00) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
					M9MO_ADJST_SHUTTER_MODE, 0x00);
			CHECK_ERR(err);
		}

		if (capture_val != M9MO_CAP_MODE_BLINK_CAPTURE) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
					M9MO_CAPCTRL_CAP_MODE,
					M9MO_CAP_MODE_BLINK_CAPTURE);
			CHECK_ERR(err);
			/* Set frame rate (0x00, 12 fps) */
			/* err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
					M9MO_CAPCTRL_CAP_FRM_INTERVAL, 0x00);
			CHECK_ERR(err); */
		}
		break;

	case RUNNING_MODE_SINGLE:
	default:
		cam_trace("~~~~~~ Single capture ~~~~~~\n");
		if (shutter_val != 0x01) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
					M9MO_ADJST_SHUTTER_MODE, 0x01);
			CHECK_ERR(err);
		}

		if (capture_val != M9MO_CAP_MODE_SINGLE_CAPTURE) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
					M9MO_CAPCTRL_CAP_MODE, 0x00);
			CHECK_ERR(err);
		}
		break;
	}

	cam_trace("X\n");
	return state->running_capture_mode;
}


/*
 * v4l2_subdev_core_ops
 */
static int m9mo_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(m9mo_ctrls); i++) {
		if (qc->id == m9mo_ctrls[i].id) {
			qc->maximum = m9mo_ctrls[i].maximum;
			qc->minimum = m9mo_ctrls[i].minimum;
			qc->step = m9mo_ctrls[i].step;
			qc->default_value = m9mo_ctrls[i].default_value;
			return 0;
		}
	}

	return -EINVAL;
}
static int m9mo_get_af_result(struct v4l2_subdev *sd,
		struct v4l2_control *ctrl);

static int m9mo_get_scene_mode(struct v4l2_subdev *sd,
		struct v4l2_control *ctrl)
{
	int err;

	err = m9mo_readb(sd, M9MO_CATEGORY_NEW,
			M9MO_NEW_DETECT_SCENE, &ctrl->value);

	cam_trace("mode : %d\n", ctrl->value);

	return ctrl->value;
}

static int m9mo_get_scene_sub_mode(struct v4l2_subdev *sd,
		struct v4l2_control *ctrl)
{
	int err;

	err = m9mo_readb(sd, M9MO_CATEGORY_NEW, 0x0C, &ctrl->value);

	return ctrl->value;
}

static int m9mo_get_zoom_level(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	int err;
	int curr_zoom_info;

	err = m9mo_readb(sd, M9MO_CATEGORY_LENS,
		M9MO_LENS_ZOOM_LEVEL_INFO, &curr_zoom_info);
	CHECK_ERR(err);

	ctrl->value = state->zoom_status = curr_zoom_info;

	cam_trace(": zoom_level %d\n", curr_zoom_info);
	return 0;
}

static int m9mo_get_zoom_status(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	int err;
	int zoom_status;

	err = m9mo_readb(sd, M9MO_CATEGORY_LENS,
		M9MO_LENS_ZOOM_STATUS, &zoom_status);
	CHECK_ERR(err);

	ctrl->value = zoom_status;

	cam_trace(": zoom_status %d\n", zoom_status);
	return 0;
}

static int m9mo_get_object_tracking(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	int err, ot_ready;
	int cnt = 30;

	cam_trace("E\n");

	err = m9mo_readb(sd, M9MO_CATEGORY_OT,
		M9MO_OT_TRACKING_STATUS, &state->ot_status);
	CHECK_ERR(err);

	if (state->ot_status != OBJECT_TRACKING_STATUS_SUCCESS) {
		cam_err("Object Tracking Fail  status: %d\n", state->ot_status);
		return 0;
	}

	err = m9mo_readb(sd, M9MO_CATEGORY_OT,
		M9MO_OT_INFO_READY, &ot_ready);
	CHECK_ERR(err);
	while (ot_ready && cnt) {
		mdelay(10);
		err = m9mo_readb(sd, M9MO_CATEGORY_OT,
			M9MO_OT_INFO_READY, &ot_ready);
		CHECK_ERR(err);
		cnt--;
	}

	if (state->ot_status != OBJECT_TRACKING_STATUS_SUCCESS) {
		cam_err("Object Tracking Not ready\n");
		return 0;
	}

	err = m9mo_readw(sd, M9MO_CATEGORY_OT,
		M9MO_OT_TRACKING_X_LOCATION,
		&state->ot_x_loc);
	CHECK_ERR(err);
	err = m9mo_readw(sd, M9MO_CATEGORY_OT,
		M9MO_OT_TRACKING_Y_LOCATION,
		&state->ot_y_loc);
	CHECK_ERR(err);
	err = m9mo_readw(sd, M9MO_CATEGORY_OT,
		M9MO_OT_TRACKING_FRAME_WIDTH,
		&state->ot_width);
	CHECK_ERR(err);
	err = m9mo_readw(sd, M9MO_CATEGORY_OT,
		M9MO_OT_TRACKING_FRAME_HEIGHT,
		&state->ot_height);
	CHECK_ERR(err);
	cam_dbg("OT pos x: %d, y: %d, w: %d, h: %d\n",
		state->ot_x_loc, state->ot_y_loc,
		state->ot_width, state->ot_height);

	cam_trace("X\n");
	return 0;
}

static int m9mo_get_av(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	int value, err;

	err = m9mo_readl(sd, M9MO_CATEGORY_AE,
		M9MO_AE_NOW_AV, &value);
	CHECK_ERR(err);

	ctrl->value = value/65536;
	state->AV = ctrl->value;

	cam_trace("X %d\n", ctrl->value);

	return ctrl->value;
}

static int m9mo_get_tv(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	int value, err;

	err = m9mo_readl(sd, M9MO_CATEGORY_AE,
		M9MO_AE_NOW_TV, &value);
	CHECK_ERR(err);

	ctrl->value = value/65536;
	state->TV = ctrl->value;

	cam_trace("X %d\n", ctrl->value);

	return ctrl->value;
}

static int m9mo_get_sv(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	int value, err;

	err = m9mo_readl(sd, M9MO_CATEGORY_AE,
		M9MO_AE_NOW_SV, &value);
	CHECK_ERR(err);

	ctrl->value = value/65536;
	state->SV = ctrl->value;

	cam_trace("X %d\n", ctrl->value);

	return ctrl->value;
}

static int m9mo_get_ev(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	int value, err;

	state->EV = state->AV + state->TV;

	cam_trace("X %d\n", state->EV);

	return state->EV;
}

static int m9mo_get_WBcustomX(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	int value, err;

	err = m9mo_readl(sd, M9MO_CATEGORY_WB,
		M9MO_WB_CUSTOM_X, &value);
	CHECK_ERR(err);

	state->wb_custom_x = value;

	cam_trace("X %d\n", value);

	return value;
}

static int m9mo_get_WBcustomY(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	int value, err;

	err = m9mo_readl(sd, M9MO_CATEGORY_WB,
		M9MO_WB_CUSTOM_Y, &value);
	CHECK_ERR(err);

	state->wb_custom_y = value;

	cam_trace("X %d\n", value);

	return value;
}

static int m9mo_get_factory_FW_info(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	int err = 0;
	u32 read_val1, read_val2;
	u32 ver = 0;

	m9mo_readb(sd, M9MO_CATEGORY_SYS,
			0x02, &read_val1);
	CHECK_ERR(err);

	m9mo_readb(sd, M9MO_CATEGORY_SYS,
		0x03, &read_val2);
	CHECK_ERR(err);

	ver = 0;
	ver = (read_val1 << 8) | (read_val2);

	cam_trace("m9mo_get_factory_FW : 0x%x\n", ver);
	ctrl->value = ver;

	return 0;
}

static int m9mo_get_factory_OIS_info(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	int err;
	u32 read_val1, read_val2, read_val3;
	u32 ver = 0;

	err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x15, 0x10);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x16, 0x70);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x14, 0x0);
	CHECK_ERR(err);


	err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x15, 0x10);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x16, 0xF0);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x14, 0x0);
	CHECK_ERR(err);


	m9mo_readb(sd, M9MO_CATEGORY_SYS,
			0x21, &read_val1);
	CHECK_ERR(err);

	m9mo_readb(sd, M9MO_CATEGORY_SYS,
			0x22, &read_val1);
	CHECK_ERR(err);

	m9mo_readb(sd, M9MO_CATEGORY_SYS,
			0x23, &read_val1);
	CHECK_ERR(err);

	ver = 0;
	ver = (read_val1 << 16) | (read_val2 << 8) | (read_val3);

	cam_trace("m9mo_get_factory_FW : 0x%x\n", ver);
	ctrl->value = ver;

	return 0;
}

static int m9mo_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	int err = 0;

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_AUTO_FOCUS_RESULT:
		m9mo_get_af_result(sd, ctrl);
		break;

	case V4L2_CID_CAM_JPEG_MEMSIZE:
		ctrl->value = M9MO_JPEG_MAXSIZE +
			M9MO_THUMB_MAXSIZE + M9MO_POST_MAXSIZE;
		break;

	case V4L2_CID_CAM_JPEG_MAIN_SIZE:
		ctrl->value = state->jpeg.main_size;
		break;

	case V4L2_CID_CAM_JPEG_MAIN_OFFSET:
		ctrl->value = state->jpeg.main_offset;
		break;

	case V4L2_CID_CAM_JPEG_THUMB_SIZE:
		ctrl->value = state->jpeg.thumb_size;
		break;

	case V4L2_CID_CAM_JPEG_THUMB_OFFSET:
		ctrl->value = state->jpeg.thumb_offset;
		break;

	case V4L2_CID_CAM_JPEG_POSTVIEW_OFFSET:
		ctrl->value = state->jpeg.postview_offset;
		break;

	case V4L2_CID_CAMERA_EXIF_FLASH:
		ctrl->value = state->exif.flash;
		break;

	case V4L2_CID_CAMERA_EXIF_ISO:
		ctrl->value = state->exif.iso;
		break;

	case V4L2_CID_CAMERA_EXIF_TV:
		ctrl->value = state->exif.tv;
		break;

	case V4L2_CID_CAMERA_EXIF_BV:
		ctrl->value = state->exif.bv;
		break;

	case V4L2_CID_CAMERA_EXIF_EBV:
		ctrl->value = state->exif.ebv;
		break;

	case V4L2_CID_CAMERA_FD_EYE_BLINK_RESULT:
		ctrl->value = state->fd_eyeblink_cap;
		break;

	case V4L2_CID_CAMERA_RED_EYE_FIX_RESULT:
		ctrl->value = state->fd_red_eye_status;
		break;

	case V4L2_CID_CAMERA_SCENE_MODE:
		err = m9mo_get_scene_mode(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_SCENE_SUB_MODE:
		err = m9mo_get_scene_sub_mode(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_FACTORY_DOWN_RESULT:
		ctrl->value = state->factory_down_check;
		break;

	case V4L2_CID_CAMERA_FACTORY_END_RESULT:
		ctrl->value = state->factory_end_check;
		break;

	case V4L2_CID_CAMERA_ZOOM:
		err = m9mo_get_zoom_level(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_OPTICAL_ZOOM_CTRL:
		err = m9mo_get_zoom_status(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_OBJ_TRACKING_STATUS:
		err = m9mo_get_object_tracking(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_AV:
		ctrl->value = m9mo_get_av(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_TV:
		ctrl->value = m9mo_get_tv(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_SV:
		ctrl->value = m9mo_get_sv(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_EV:
		ctrl->value = m9mo_get_ev(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_WB_CUSTOM_X:
		ctrl->value = m9mo_get_WBcustomX(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_WB_CUSTOM_Y:
		ctrl->value = m9mo_get_WBcustomY(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_FACTORY_ISP_FW_CHECK:
		err = m9mo_get_factory_FW_info(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_FACTORY_OIS_VER_CHECK:
		err = m9mo_get_factory_OIS_info(sd, ctrl);
		break;

	default:
		cam_err("no such control id %d\n",
				ctrl->id - V4L2_CID_PRIVATE_BASE);
		/*err = -ENOIOCTLCMD*/
		err = 0;
		break;
	}

	if (err < 0 && err != -ENOIOCTLCMD)
		cam_err("failed, id %d\n", ctrl->id - V4L2_CID_PRIVATE_BASE);

	return err;
}

static int m9mo_set_antibanding(struct v4l2_subdev *sd,
		struct v4l2_control *ctrl)
{
	struct v4l2_queryctrl qc = {0,};
	struct m9mo_state *state = to_state(sd);
	int val = ctrl->value, err;
	u32 antibanding[] = {0x00, 0x01, 0x02, 0x03};

	if (state->anti_banding == val)
		return 0;

	cam_dbg("E, value %d\n", val);

	qc.id = ctrl->id;
	m9mo_queryctrl(sd, &qc);

	if (val < qc.minimum || val > qc.maximum) {
		cam_warn("invalied value, %d\n", val);
		val = qc.default_value;
	}

	val -= qc.minimum;

	err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
					M9MO_AE_FLICKER, antibanding[val]);
	CHECK_ERR(err);

	state->anti_banding = val;

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_lens_off(struct v4l2_subdev *sd)
{
	struct m9mo_state *state = to_state(sd);
	u32 int_factor = 0;
	int err = 0;

	cam_trace("E\n");

	if (unlikely(state->isp.bad_fw)) {
		cam_err("\"Unknown\" state, please update F/W");
		return -ENOSYS;
	}

	err = m9mo_set_mode(sd, M9MO_MONITOR_MODE);
	if (err <= 0) {
		cam_err("failed to set mode\n");
		return err;
	}

	err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
		0x01, 0x00);

	int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);

	if (!(int_factor & M9MO_INT_LENS_INIT)) {
		cam_err("M9MO_INT_LENS_INIT isn't issued, %#x\n",
				int_factor);
		return -ETIMEDOUT;
	}

	cam_trace("X\n");
	return err;
}

static int m9mo_dump_fw(struct v4l2_subdev *sd)
{
	struct file *fp;
	mm_segment_t old_fs;
	u8 *buf/*, val*/;
	u32 addr, unit, count, intram_unit = 0x1000;
	int i, /*j,*/ err;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(M9MO_FW_DUMP_PATH,
		O_WRONLY|O_CREAT|O_TRUNC, S_IRUGO|S_IWUGO|S_IXUSR);
	if (IS_ERR(fp)) {
		cam_err("failed to open %s, err %ld\n",
			M9MO_FW_DUMP_PATH, PTR_ERR(fp));
		err = -ENOENT;
		goto file_out;
	}

	buf = kmalloc(intram_unit, GFP_KERNEL);
	if (!buf) {
		cam_err("failed to allocate memory\n");
		err = -ENOMEM;
		goto out;
	}

	cam_dbg("start, file path %s\n", M9MO_FW_DUMP_PATH);


/*
	val = 0x7E;
	err = m9mo_mem_write(sd, 0x04, sizeof(val), 0x50000308, &val);
	if (err < 0) {
		cam_err("failed to write memory\n");
		goto out;
	}
*/


	err = m9mo_mem_write(sd, 0x04, SZ_64,
				0x90001200 , buf_port_seting0);
			CHECK_ERR(err);
			mdelay(10);

	err = m9mo_mem_write(sd, 0x04, SZ_64,
				0x90001000 , buf_port_seting1);
			CHECK_ERR(err);
			mdelay(10);

	err = m9mo_mem_write(sd, 0x04, SZ_64,
				0x90001100 , buf_port_seting2);
			CHECK_ERR(err);
			mdelay(10);

	err = m9mo_writel(sd, M9MO_CATEGORY_FLASH,
				0x1C, 0x0247036D);

	err = m9mo_writeb(sd, M9MO_CATEGORY_FLASH,
				0x57, 01);

	CHECK_ERR(err);


	addr = M9MO_FLASH_READ_BASE_ADDR;
	unit = SZ_4K;
	count = 1024;
	for (i = 0; i < count; i++) {

			err = m9mo_mem_dump(sd,
				unit, addr + (i * unit), buf);
			cam_err("dump ~~ %d\n", i);
			if (err < 0) {
				cam_err("i2c falied, err %d\n", err);
				goto out;
			}
			vfs_write(fp, buf, unit, &fp->f_pos);
	}
/*
	addr = M9MO_FLASH_BASE_ADDR + SZ_64K * count;
	unit = SZ_8K;
	count = 4;
	for (i = 0; i < count; i++) {
		for (j = 0; j < unit; j += intram_unit) {
			err = m9mo_mem_read(sd,
				intram_unit, addr + (i * unit) + j, buf);
			if (err < 0) {
				cam_err("i2c falied, err %d\n", err);
				goto out;
			}
			vfs_write(fp, buf, intram_unit, &fp->f_pos);
		}
	}
*/
	cam_dbg("end\n");

out:
	kfree(buf);
	if (!IS_ERR(fp))
		filp_close(fp, current->files);
file_out:
	set_fs(old_fs);

	return err;
}

static int m9mo_get_sensor_fw_version(struct v4l2_subdev *sd)
{
	struct m9mo_state *state = to_state(sd);
	int err;
	int fw_ver = 0x00;
	int awb_ver = 0x00;
	int af_ver = 0x00;
	int ois_ver = 0x00;
	int parm_ver = 0x00;

	cam_err("E\n");

	/* read F/W version */
	err = m9mo_readw(sd, M9MO_CATEGORY_SYS,
		M9MO_SYS_VER_FW, &fw_ver);
	CHECK_ERR(err);

	err = m9mo_readw(sd, M9MO_CATEGORY_SYS,
		M9MO_SYS_VER_AWB, &awb_ver);
	CHECK_ERR(err);

	err = m9mo_readw(sd, M9MO_CATEGORY_SYS,
		M9MO_SYS_VER_PARAM, &parm_ver);
	CHECK_ERR(err);

#if 0
	err = m9mo_readw(sd, M9MO_CATEGORY_SYS,
		M9MO_SYS_VER_AF, &af_ver);
	CHECK_ERR(err);

	err = m9mo_readw(sd, M9MO_CATEGORY_SYS,
		M9MO_SYS_VER_OIS, &ois_ver);
	CHECK_ERR(err);
#endif

	cam_info("f/w version = %x\n", fw_ver);
	cam_info("awb version = %x\n", awb_ver);
	cam_info("af version = %x\n", af_ver);
	cam_info("ois version = %x\n", ois_ver);
	cam_info("parm version = %x\n", parm_ver);

	sprintf(state->sensor_ver, "%x %x %x %x %x",
			fw_ver, awb_ver, af_ver, ois_ver, parm_ver);
	memcpy(sysfs_sensor_fw, state->sensor_ver,
			sizeof(state->sensor_ver));

	cam_info("sensor fw : %s\n", sysfs_sensor_fw);
	return 0;
}

static int m9mo_get_phone_fw_version(struct v4l2_subdev *sd)
{
	struct m9mo_state *state = to_state(sd);
	struct device *dev = sd->v4l2_dev->dev;
	const struct firmware *fw;
	int err = 0;

	struct file *fp;
	mm_segment_t old_fs;
	long nread;
	int fw_requested = 1;
	int ver_tmp;

	cam_info("E\n");

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(M9MO_FW_PATH, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		cam_trace("failed to open %s, err %ld\n", M9MO_FW_PATH,
			  PTR_ERR(fp));
		goto request_fw;
	} else {
		cam_info("FW File(phone) opened.\n");
	}

	fw_requested = 0;

	err = vfs_llseek(fp, M9MO_FW_VER_NUM, SEEK_SET);
	if (err < 0) {
		cam_warn("failed to fseek, %d\n", err);
		goto out;
	}

	nread = vfs_read(fp, (char __user *)state->phone_ver,
			M9MO_FW_VER_LEN, &fp->f_pos);
	if (nread != M9MO_FW_VER_LEN) {
		cam_err("failed to read firmware file, %ld Bytes\n", nread);
		err = -EIO;
		goto out;
	}

request_fw:
	if (fw_requested) {
		set_fs(old_fs);

#if 0
		m9mo_get_sensor_fw_version(sd, sensor_ver);

		if (sensor_ver[0] == 'T' && sensor_ver[1] == 'B') {
			err = request_firmware(&fw, M9MOTB_FW_PATH, dev);
#if defined(CONFIG_MACH_Q1_BD)
		} else if (sensor_ver[0] == 'O' && sensor_ver[1] == 'O') {
			err = request_firmware(&fw, M9MOOO_FW_PATH, dev);
#endif
#if defined(CONFIG_MACH_U1_KOR_LGT)
		} else if (sensor_ver[0] == 'S' && sensor_ver[1] == 'B') {
			err = request_firmware(&fw, M9MOSB_FW_PATH, dev);
#endif
		} else {
			cam_warn("cannot find the matched F/W file\n");
#if defined(CONFIG_MACH_Q1_BD)
			err = request_firmware(&fw, M9MOOO_FW_PATH, dev);
#elif defined(CONFIG_MACH_U1_KOR_LGT)
			err = request_firmware(&fw, M9MOSB_FW_PATH, dev);
#else
			err = request_firmware(&fw, M9MOTB_FW_PATH, dev);
#endif
		}
#else
		cam_info("Firmware Path = %s\n", M9MO_FW_REQ_PATH);
		err = request_firmware(&fw, M9MO_FW_REQ_PATH, dev);
#endif

		if (err != 0) {
			cam_err("request_firmware falied\n");
			err = -EINVAL;
			goto out;
		}
#if 0
		sprintf(state->phone_ver, "%x%x",
				(u32)&fw->data[M9MO_FW_VER_NUM],
				(u32)&fw->data[M9MO_FW_VER_NUM + 1]);
#endif
		cam_info("%s: fw->data[0] = %x, fw->data[1] = %x\n", __func__,
				(int)fw->data[M9MO_FW_VER_NUM],
				(int)fw->data[M9MO_FW_VER_NUM + 1]);
		ver_tmp = (int)fw->data[M9MO_FW_VER_NUM] * 16 * 16;
		ver_tmp += (int)fw->data[M9MO_FW_VER_NUM + 1];
		cam_info("ver_tmp = %x\n", ver_tmp);
		sprintf(state->phone_ver, "%x", ver_tmp);
		memcpy(sysfs_phone_fw, state->phone_ver,
				sizeof(state->phone_ver));

	}
out:
	if (!fw_requested) {
		filp_close(fp, current->files);
		set_fs(old_fs);
	} else {
		release_firmware(fw);
	}

	cam_dbg("phone ver : %s\n", sysfs_phone_fw);
	return 0;
}

static int m9mo_check_fw(struct v4l2_subdev *sd)
{
	struct m9mo_state *state = to_state(sd);
	int af_cal_h = 0, af_cal_l = 0;
	int rg_cal_h = 0, rg_cal_l = 0;
	int bg_cal_h = 0, bg_cal_l = 0;
	int update_count = 0;
	u32 int_factor;
	int err;

	cam_trace("E\n");

	/* F/W version */
	m9mo_get_phone_fw_version(sd);

#if 0
	if (state->isp.bad_fw)
		goto out;
#endif

	m9mo_get_sensor_fw_version(sd);

	goto out;

	err = m9mo_writeb(sd, M9MO_CATEGORY_FLASH, M9MO_FLASH_CAM_START, 0x01);
	CHECK_ERR(err);

	int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
	if (!(int_factor & M9MO_INT_MODE)) {
		cam_err("firmware was erased?\n");
		return -ETIMEDOUT;
	}

	err = m9mo_readb(sd, M9MO_CATEGORY_LENS, M9MO_LENS_AF_CAL, &af_cal_l);
	CHECK_ERR(err);

	err = m9mo_readb(sd, M9MO_CATEGORY_ADJST,
			M9MO_ADJST_AWB_RG_H, &rg_cal_h);
	CHECK_ERR(err);
	err = m9mo_readb(sd, M9MO_CATEGORY_ADJST,
			M9MO_ADJST_AWB_RG_L, &rg_cal_l);
	CHECK_ERR(err);

	err = m9mo_readb(sd, M9MO_CATEGORY_ADJST,
			M9MO_ADJST_AWB_BG_H, &bg_cal_h);
	CHECK_ERR(err);
	err = m9mo_readb(sd, M9MO_CATEGORY_ADJST,
			M9MO_ADJST_AWB_BG_L, &bg_cal_l);
	CHECK_ERR(err);

out:
#if 0
	if (!state->fw_version) {
		state->fw_version = kzalloc(50, GFP_KERNEL);
		if (!state->fw_version) {
			cam_err("no memory for F/W version\n");
			return -ENOMEM;
		}
	}
#endif

#if 0
	sprintf(state->fw_version, "%s %s %d %x %x %x %x %x %x",
		sensor_ver, phone_ver, update_count,
		af_cal_h, af_cal_l, rg_cal_h, rg_cal_l, bg_cal_h, bg_cal_l);
#endif
	cam_info("phone ver = %s, sensor_ver = %s\n",
			sysfs_phone_fw, sysfs_sensor_fw);

	cam_trace("X\n");
	return 0;
}


static int m9mo_make_CSV_rawdata(struct v4l2_subdev *sd,
	u32 *address, bool bAddResult)
{
	struct file *fp;
	mm_segment_t old_fs;
	u8 *buf/*, val*/;
	u32 addr, unit, count, intram_unit = 0x1000;
	int i, /*j,*/ err;
	struct m9mo_state *state = to_state(sd);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(M9MO_FACTORY_CSV_PATH,
		O_WRONLY|O_CREAT|O_TRUNC, S_IRUGO|S_IWUGO|S_IXUSR);
	if (IS_ERR(fp)) {
		cam_err("failed to open %s, err %ld\n",
			M9MO_FACTORY_CSV_PATH, PTR_ERR(fp));
		err = -ENOENT;
		goto file_out;
	}

	buf = kmalloc(intram_unit, GFP_KERNEL);
	if (!buf) {
		cam_err("failed to allocate memory\n");
		err = -ENOMEM;
		goto out;
	}

	cam_dbg("start, file path %s\n", M9MO_FACTORY_CSV_PATH);

	addr = address[0];
	unit = address[1]-address[0]+1;

	cam_trace("m9mo_make_CSV_rawdata() addr[0x%0x] size=%d\n",
		addr, unit);

	err = m9mo_mem_read(sd, unit, addr, buf);
	if (err < 0) {
		cam_err("i2c falied, err %d\n", err);
		goto out;
	}

/*"Result27E03128(0:OK, 1:NG)"Bit 0 : IRIS
Bit 1 : Liveview Gain
Bit 2 : ShutterClose
Bit 3 : CaptureGain
Bit 5 : DefectPixel*/
	if (bAddResult) {
		m9mo_mem_read(sd, 0x2,
			M9MO_FLASH_FACTORY_RESULT, buf+unit);
		unit += 2;
	}

	cam_trace("m9mo_make_CSV_rawdata() size=%d  result=%x\n",
		unit, *(buf+unit));

	vfs_write(fp, buf, unit, &fp->f_pos);
	mdelay(10);

out:
	kfree(buf);
	if (!IS_ERR(fp))
		filp_close(fp, current->files);
file_out:
	set_fs(old_fs);

	return err;
}

static int m9mo_make_CSV_rawdata_direct(struct v4l2_subdev *sd, int nkind)
{
	struct file *fp;
	mm_segment_t old_fs;
	u8 *buf;
	int val;
	u32 addr, unit, count, intram_unit = 0x1000;
	int i, j, err, start, end;

	struct m9mo_state *state = to_state(sd);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(M9MO_FACTORY_CSV_PATH,
		O_WRONLY|O_CREAT|O_TRUNC, S_IRUGO|S_IWUGO|S_IXUSR);
	if (IS_ERR(fp)) {
		cam_err("failed to open %s, err %ld\n",
			M9MO_FACTORY_CSV_PATH, PTR_ERR(fp));
		err = -ENOENT;
		goto file_out;
	}

	buf = kmalloc(intram_unit, GFP_KERNEL);
	if (!buf) {
		cam_err("failed to allocate memory\n");
		err = -ENOMEM;
		goto out;
	}

	if (V4L2_CID_CAMERA_FACTORY_DEFECTPIXEL) {
		cam_dbg("start, file path %s\n", M9MO_FACTORY_CSV_PATH);

		start = 0x69;
		end = 0x86;
		unit = end-start + 1;

		for (i = start; i <= end; i++) {
			err = m9mo_readb(sd, M9MO_CATEGORY_MON, i, &val);
			CHECK_ERR(err);

			buf[i-start] = (u8)val;
		}
	}
	vfs_write(fp, buf, unit, &fp->f_pos);

out:
	kfree(buf);

	if (!IS_ERR(fp))
		filp_close(fp, current->files);

file_out:
	set_fs(old_fs);

	return err;
}

#ifdef FAST_CAPTURE
static int m9mo_set_fast_capture(struct v4l2_subdev *sd)
{
	struct m9mo_state *state = to_state(sd);
	int err;
	cam_info("E\n");

	err = m9mo_set_mode(sd, M9MO_STILLCAP_MODE);
	if (err < 0) {
		cam_err("Mode change is failed to STILLCAP for fast capture\n");
		return err;
	} else {
		cam_info("Fast capture is issued. mode change start.\n");
	}
	return 0;
}
#endif

static int m9mo_set_sensor_mode(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err;
	cam_dbg("E, value %d\n", val);

	err = m9mo_set_mode(sd, M9MO_PARMSET_MODE);
	CHECK_ERR(err);

	state->sensor_mode = val;

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_FillIn_step(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E, value %d\n", val);

	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
			M9MO_CAPPARM_STROBE_EVC, val);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_flash(struct v4l2_subdev *sd, int val, int force)
{
	struct m9mo_state *state = to_state(sd);
	int strobe_en = 0;
	int err;
	cam_dbg("E, value %d\n", val);

	if (!force)
		state->flash_mode = val;

	/* movie flash mode should be set when recording is started */
	if (state->sensor_mode == SENSOR_MOVIE && !state->recording)
		return 0;

retry:
	switch (val) {
	case FLASH_MODE_OFF:
		strobe_en = 0;
		break;

	case FLASH_MODE_AUTO:
		strobe_en = 2;
		break;

	case FLASH_MODE_ON:
		strobe_en = 1;
		break;

	case FLASH_MODE_RED_EYE:
		strobe_en = 1;
		break;

	case FLASH_MODE_FILL_IN:
		strobe_en = 1;
		break;

	case FLASH_MODE_SLOW_SYNC:
		strobe_en = 3;
		break;

	case FLASH_MODE_RED_EYE_FIX:
		strobe_en = 2;
		err = m9mo_writeb(sd, M9MO_CATEGORY_FD,
				M9MO_FD_RED_EYE, 0x01);
		CHECK_ERR(err);
		break;

	default:
		cam_warn("invalid value, %d\n", val);
		val = FLASH_MODE_OFF;
		goto retry;
	}

	if (val !=  FLASH_MODE_RED_EYE_FIX) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_FD,
				M9MO_FD_RED_EYE, 0x00);
		CHECK_ERR(err);
	}

	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
			M9MO_CAPPARM_STROBE_EN, strobe_en);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_iso(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	struct v4l2_queryctrl qc = {0,};
	int val = ctrl->value, err;
	u32 iso[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};

	if (state->scene_mode != SCENE_MODE_NONE) {
		/* sensor will set internally */
		return 0;
	}

	cam_dbg("E, value %d\n", val);

	qc.id = ctrl->id;
	m9mo_queryctrl(sd, &qc);

	if (val < qc.minimum || val > qc.maximum) {
		cam_warn("invalied value, %d\n", val);
		val = qc.default_value;
	}

	val -= qc.minimum;

	switch (val) {
	case 0:
		state->iso = 0;
		break;

	case 1:
		state->iso = 50;
		break;

	case 2:
		state->iso = 100;
		break;

	case 3:
		state->iso = 200;
		break;

	case 4:
		state->iso = 400;
		break;

	case 5:
		state->iso = 800;
		break;

	case 6:
		state->iso = 1600;
		break;

	case 7:
		state->iso = 3200;
		break;

	default:
		break;
	}

	err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_ISOSEL, iso[val]);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_metering(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_dbg("E, value %d\n", val);

retry:
	switch (val) {
	case METERING_CENTER:
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_MODE, 0x03);
		CHECK_ERR(err);
		break;
	case METERING_SPOT:
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_MODE, 0x05);
		CHECK_ERR(err);
		break;
	case METERING_MATRIX:
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_MODE, 0x01);
		CHECK_ERR(err);
		break;
	default:
		cam_warn("invalid value, %d\n", val);
		val = METERING_CENTER;
		goto retry;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_exposure(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	struct v4l2_queryctrl qc = {0,};
	int val = ctrl->value, err;
	/*
	   -2.0, -1.7, -1.3, -1.0 -0.7 -0.3
	   0
	   +0.3 +0.7 +1.0 +1.3 +1.7 +2.0
	*/
	u32 exposure[] = {0x0A, 0x0D, 0x11, 0x14, 0x17, 0x1B,
		0x1E,
		0x21, 0x25, 0x28, 0x2B, 0x2F, 0x32};
	cam_dbg("E, value %d\n", val);

	qc.id = ctrl->id;
	m9mo_queryctrl(sd, &qc);

	if (val < qc.minimum || val > qc.maximum) {
		cam_warn("invalied value, %d\n", val);
		val = qc.default_value;
	}

	val -= qc.minimum;

	err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
		M9MO_AE_INDEX, exposure[val]);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_whitebalance(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_dbg("E, value %d\n", val);

retry:
	switch (val) {
	case WHITE_BALANCE_AUTO:
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MODE, 0x01);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MANUAL, 0x01);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_SUNNY:
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MODE, 0x02);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MANUAL, 0x04);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_CLOUDY:
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MODE, 0x02);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MANUAL, 0x05);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_TUNGSTEN:
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MODE, 0x02);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MANUAL, 0x01);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_FLUORESCENT:
	case WHITE_BALANCE_FLUORESCENT_H:
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MODE, 0x02);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MANUAL, 0x02);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_FLUORESCENT_L:
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MODE, 0x02);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MANUAL, 0x03);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_K:
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MODE, 0x02);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MANUAL, 0x0A);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_INCANDESCENT:
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MODE, 0x02);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MANUAL, 0x01);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_PROHIBITION:
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MODE, 0x02);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MANUAL, 0x00);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_HORIZON:
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MODE, 0x02);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MANUAL, 0x07);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_LEDLIGHT:
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MODE, 0x02);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MANUAL, 0x09);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_CUSTOM:
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MODE, 0x02);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
			M9MO_WB_AWB_MANUAL, 0x08);
		CHECK_ERR(err);
		break;

	default:
		cam_warn("invalid value, %d\n", val);
		val = WHITE_BALANCE_AUTO;
		goto retry;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_sharpness(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct v4l2_queryctrl qc = {0,};
	int val = ctrl->value, err;
	u32 sharpness[] = {0x03, 0x04, 0x05, 0x06, 0x07};
	cam_dbg("E, value %d\n", val);

	qc.id = ctrl->id;
	m9mo_queryctrl(sd, &qc);

	if (val < qc.minimum || val > qc.maximum) {
		cam_warn("invalied value, %d\n", val);
		val = qc.default_value;
	}

	val -= qc.minimum;

	err = m9mo_writeb(sd, M9MO_CATEGORY_MON,
		M9MO_MON_EDGE_LVL, sharpness[val]);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_saturation(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	struct v4l2_queryctrl qc = {0,};
	int val = ctrl->value, err;
	u32 saturation[] = {0x01, 0x02, 0x03, 0x04, 0x05};
	cam_dbg("E, value %d\n", val);

	qc.id = ctrl->id;
	m9mo_queryctrl(sd, &qc);

	if (val < qc.minimum || val > qc.maximum) {
		cam_warn("invalied value, %d\n", val);
		val = qc.default_value;
	}

	val -= qc.minimum;

	err = m9mo_writeb(sd, M9MO_CATEGORY_MON,
		M9MO_MON_CHROMA_LVL, saturation[val]);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_scene_mode(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	struct v4l2_control ctrl;
	int evp, sharpness, saturation;
	int err;
	cam_dbg("E, value %d\n", val);

	sharpness = SHARPNESS_DEFAULT;
	saturation = CONTRAST_DEFAULT;

retry:
	switch (val) {
	case SCENE_MODE_NONE:
		evp = 0x00;
		break;

	case SCENE_MODE_PORTRAIT:
		evp = 0x01;
		sharpness = SHARPNESS_MINUS_1;
		break;

	case SCENE_MODE_LANDSCAPE:
		evp = 0x02;
		sharpness = SHARPNESS_PLUS_1;
		saturation = SATURATION_PLUS_1;
		break;

	case SCENE_MODE_SPORTS:
		evp = 0x03;
		break;

	case SCENE_MODE_PARTY_INDOOR:
		evp = 0x04;
		saturation = SATURATION_PLUS_1;
		break;

	case SCENE_MODE_BEACH_SNOW:
		evp = 0x05;
		saturation = SATURATION_PLUS_1;
		break;

	case SCENE_MODE_SUNSET:
		evp = 0x06;
		break;

	case SCENE_MODE_DUSK_DAWN:
		evp = 0x07;
		break;

	case SCENE_MODE_FALL_COLOR:
		evp = 0x08;
		saturation = SATURATION_PLUS_2;
		break;

	case SCENE_MODE_NIGHTSHOT:
		evp = 0x09;
		break;

	case SCENE_MODE_BACK_LIGHT:
		evp = 0x0A;
		break;

	case SCENE_MODE_FIREWORKS:
		evp = 0x0B;
		break;

	case SCENE_MODE_TEXT:
		evp = 0x0C;
		sharpness = SHARPNESS_PLUS_2;
		break;

	case SCENE_MODE_CANDLE_LIGHT:
		evp = 0x0D;
		break;

	default:
		cam_warn("invalid value, %d\n", val);
		val = SCENE_MODE_NONE;
		goto retry;
	}

	/* EV-P */
	err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_EP_MODE_MON, evp);
	CHECK_ERR(err);
	err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_EP_MODE_CAP, evp);
	CHECK_ERR(err);

	/* Chroma Saturation */
	ctrl.id = V4L2_CID_CAMERA_SATURATION;
	ctrl.value = saturation;
	m9mo_set_saturation(sd, &ctrl);

	/* Sharpness */
	ctrl.id = V4L2_CID_CAMERA_SHARPNESS;
	ctrl.value = sharpness;
	m9mo_set_sharpness(sd, &ctrl);

	/* Emotional Color */
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
		M9MO_CAPPARM_MCC_MODE, val == SCENE_MODE_NONE ? 0x01 : 0x00);
	CHECK_ERR(err);

	state->scene_mode = val;

	cam_trace("X\n");
	return 0;
}


static int m9mo_set_effect_color(struct v4l2_subdev *sd, int val)
{
	u32 int_factor;
	int on, old_mode, cb = 0, cr = 0;
	int err;

	err = m9mo_readb(sd, M9MO_CATEGORY_PARM, M9MO_PARM_EFFECT, &on);
	CHECK_ERR(err);
	if (on)	{
		old_mode = m9mo_set_mode(sd, M9MO_PARMSET_MODE);
		CHECK_ERR(old_mode);

		err = m9mo_writeb(sd, M9MO_CATEGORY_PARM, M9MO_PARM_EFFECT, 0);
		CHECK_ERR(err);

		if (old_mode == M9MO_MONITOR_MODE) {
			err = m9mo_set_mode(sd, old_mode);
			CHECK_ERR(err);

			int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
			if (!(int_factor & M9MO_INT_MODE)) {
				cam_err("M9MO_INT_MODE isn't issued, %#x\n",
					int_factor);
				return -ETIMEDOUT;
			}
			CHECK_ERR(err);
		}
	}

	switch (val) {
	case IMAGE_EFFECT_NONE:
		break;

	case IMAGE_EFFECT_SEPIA:
		cb = 0xD8;
		cr = 0x18;
		break;

	case IMAGE_EFFECT_BNW:
		cb = 0x00;
		cr = 0x00;
		break;
	}

	err = m9mo_writeb(sd, M9MO_CATEGORY_MON,
		M9MO_MON_COLOR_EFFECT, val == IMAGE_EFFECT_NONE ? 0x00 : 0x01);
		CHECK_ERR(err);

	if (val != IMAGE_EFFECT_NONE) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_MON, M9MO_MON_CFIXB, cb);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_MON, M9MO_MON_CFIXR, cr);
		CHECK_ERR(err);
	}

	return 0;
}

static int m9mo_set_effect_gamma(struct v4l2_subdev *sd, s32 val)
{
	u32 int_factor;
	int on, effect, old_mode;
	int err;

	err = m9mo_readb(sd, M9MO_CATEGORY_MON, M9MO_MON_COLOR_EFFECT, &on);
	CHECK_ERR(err);
	if (on) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_MON,
			M9MO_MON_COLOR_EFFECT, 0);
		CHECK_ERR(err);
	}

	switch (val) {
	case IMAGE_EFFECT_NEGATIVE:
		effect = 0x01;
		break;

	case IMAGE_EFFECT_AQUA:
		effect = 0x08;
		break;
	}

	old_mode = m9mo_set_mode(sd, M9MO_PARMSET_MODE);
	CHECK_ERR(old_mode);

	err = m9mo_writeb(sd, M9MO_CATEGORY_PARM, M9MO_PARM_EFFECT, effect);
	CHECK_ERR(err);

	if (old_mode == M9MO_MONITOR_MODE) {
		err = m9mo_set_mode(sd, old_mode);
		CHECK_ERR(err);

		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_MODE)) {
			cam_err("M9MO_INT_MODE isn't issued, %#x\n",
				int_factor);
			return -ETIMEDOUT;
		}
		CHECK_ERR(err);
	}

	return err;
}

static int m9mo_set_effect(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_dbg("E, value %d\n", val);

retry:
	switch (val) {
	case IMAGE_EFFECT_NONE:
	case IMAGE_EFFECT_BNW:
	case IMAGE_EFFECT_SEPIA:
		err = m9mo_set_effect_color(sd, val);
		CHECK_ERR(err);
		break;

	case IMAGE_EFFECT_AQUA:
	case IMAGE_EFFECT_NEGATIVE:
		err = m9mo_set_effect_gamma(sd, val);
		CHECK_ERR(err);
		break;

	default:
		cam_warn("invalid value, %d\n", val);
		val = IMAGE_EFFECT_NONE;
		goto retry;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_wdr(struct v4l2_subdev *sd, int val)
{
	int contrast, wdr, err;

	cam_dbg("%s\n", val ? "on" : "off");

	contrast = (val == 1 ? 0x09 : 0x05);
	wdr = (val == 1 ? 0x01 : 0x00);

	err = m9mo_writeb(sd, M9MO_CATEGORY_MON,
			M9MO_MON_TONE_CTRL, contrast);
		CHECK_ERR(err);
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
			M9MO_CAPPARM_WDR_EN, wdr);
		CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_antishake(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int ahs, err;

	if (state->scene_mode != SCENE_MODE_NONE) {
		cam_warn("Should not be set with scene mode");
		return 0;
	}

	cam_dbg("%s\n", val ? "on" : "off");

	ahs = (val == 1 ? 0x0E : 0x00);

	err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_EP_MODE_MON, ahs);
		CHECK_ERR(err);
	err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_EP_MODE_CAP, ahs);
		CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_face_beauty(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err;

	cam_dbg("%s\n", val ? "on" : "off");

	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
		M9MO_CAPPARM_AFB_CAP_EN, val ? 0x01 : 0x00);
	CHECK_ERR(err);

	state->face_beauty = val;

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_lock(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err;

	cam_trace("%s\n", val ? "on" : "off");

	err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_LOCK, val);
	CHECK_ERR(err);
	err = m9mo_writeb(sd, M9MO_CATEGORY_WB, M9MO_AWB_LOCK, val);
	CHECK_ERR(err);

	state->focus.lock = val;

	cam_trace("X\n");
	return 0;
}

static unsigned int m9mo_set_cal_rect_pos(struct v4l2_subdev *sd,
	unsigned int pos_val)
{
	struct m9mo_state *state = to_state(sd);
	unsigned int set_val;

	if (pos_val <= 40)
		set_val = 40;
	else if (pos_val > (state->preview->width - 40))
		set_val = state->preview->width - 40;
	else
		set_val = pos_val;

	return set_val;
}

static int m9mo_set_object_tracking(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err = 0;
	unsigned int set_x, set_y;

	if (state->focus_area_mode != V4L2_FOCUS_AREA_SMART_TOUCH)
		return 0;

	err = m9mo_writeb(sd, M9MO_CATEGORY_OT,
		M9MO_OT_TRACKING_CTL, 0x10);
	CHECK_ERR(err);

	cam_trace("E val : %d\n", val);

	if (val == OT_START) {
		set_x = m9mo_set_cal_rect_pos(sd, state->focus.pos_x);
		set_y = m9mo_set_cal_rect_pos(sd, state->focus.pos_y);

		cam_dbg("idx[%d] w[%d] h[%d]", state->preview->index,
			state->preview->width, state->preview->height);
		cam_dbg("pos_x[%d] pos_y[%d] x[%d] y[%d]",
			state->focus.pos_x, state->focus.pos_y,
			set_x, set_y);

		err = m9mo_writeb(sd, M9MO_CATEGORY_OT,
			M9MO_OT_FRAME_WIDTH, 0x02);
		CHECK_ERR(err);
		err = m9mo_writew(sd, M9MO_CATEGORY_OT,
			M9MO_OT_X_START_LOCATION,
			set_x - 40);
		CHECK_ERR(err);
		err = m9mo_writew(sd, M9MO_CATEGORY_OT,
			M9MO_OT_Y_START_LOCATION,
			set_y - 40);
		CHECK_ERR(err);
		err = m9mo_writew(sd, M9MO_CATEGORY_OT,
			M9MO_OT_X_END_LOCATION,
			set_x + 40);
		CHECK_ERR(err);
		err = m9mo_writew(sd, M9MO_CATEGORY_OT,
			M9MO_OT_Y_END_LOCATION,
			set_y + 40);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_OT,
			M9MO_OT_TRACKING_CTL, 0x11);
		CHECK_ERR(err);
	}

	return 0;
}

static int m9mo_get_af_result(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	int status, err;
	static int get_cnt;

	cam_trace("E, cnt: %d, status: 0x%x\n", get_cnt, state->focus.status);

	get_cnt++;

	err = m9mo_readw(sd, M9MO_CATEGORY_LENS,
		M9MO_LENS_AF_RESULT, &status);
	CHECK_ERR(err);

	if (status) {
		cam_dbg("~~~ success !!!~~~\n");
		get_cnt = 0;
	} else if (get_cnt >= 50) {
		get_cnt = 0;
	} else if (state->focus.status & 0x8000) {
		get_cnt = 0;
		status = 0x2000;
	} else {
		status = 0x1000;
	}

	state->focus.status = status;
	ctrl->value = state->focus.status;

	/* Set AF Window mode to Cener */
	if ((status != 0x1000)
		&& state->focus_area_mode == V4L2_FOCUS_AREA_SMART_TOUCH) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			M9MO_LENS_AF_WINDOW_MODE, 0x00);
		CHECK_ERR(err);
	}

	cam_dbg("X, value 0x%04x\n", ctrl->value);

	return ctrl->value;
}

static int m9mo_set_image_stabilizer_OIS(struct v4l2_subdev *sd, int val)
{
	int err, int_factor, set_ois, int_en;
	int wait_int_ois = 0;

	cam_trace("E: mode %d\n", val);

retry:
	switch (val) {
	case V4L2_IS_OIS_NONE:
		cam_warn("OIS_NONE and OIS End");
		return 0;

	case V4L2_IS_OIS_MOVIE:
		set_ois = 0x01;
		wait_int_ois = 1;
		break;

	case V4L2_IS_OIS_STILL:
		set_ois = 0x02;
		wait_int_ois = 0;
		break;

	case V4L2_IS_OIS_MULTI:
		set_ois = 0x03;
		wait_int_ois = 0;
		break;

	case V4L2_IS_OIS_VSS:
		set_ois = 0x04;
		wait_int_ois = 1;
		break;

	default:
		cam_warn("invalid value, %d", val);
		val = V4L2_IS_OIS_STILL;
		goto retry;
	}

	if (wait_int_ois) {
		err = m9mo_readw(sd, M9MO_CATEGORY_SYS,
			M9MO_SYS_INT_EN, &int_en);
		CHECK_ERR(err);

		/* enable OIS_SET interrupt */
		int_en |= M9MO_INT_OIS_SET;

		err = m9mo_writew(sd, M9MO_CATEGORY_SYS,
			M9MO_SYS_INT_EN, int_en);
		CHECK_ERR(err);

		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW, 0x18, set_ois);
		CHECK_ERR(err);

		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_OIS_SET)) {
			cam_err("M9MO_INT_OIS_SET isn't issued, %#x\n",
				int_factor);
			return -ETIMEDOUT;
		}
		CHECK_ERR(err);

		/* enable OIS_SET interrupt */
		int_en &= ~M9MO_INT_OIS_SET;

		err = m9mo_writew(sd, M9MO_CATEGORY_SYS,
			M9MO_SYS_INT_EN, int_en);
		CHECK_ERR(err);
	} else {
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW, 0x18, set_ois);
		CHECK_ERR(err);
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_af(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err = 0;

	cam_info("%s, mode %d\n", val ? "start" : "stop", state->focus.mode);

	state->focus.start = val;

	if (val == 1) {
		state->focus.status &= ~0x8000;
		if (state->focus.mode == FOCUS_MODE_CONTINOUS) {
			/* Start Continuous AF */
			err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
					M9MO_LENS_AF_START_STOP, 0x01);
			CHECK_ERR(err);
		} else if (state->facedetect_mode) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
				M9MO_LENS_AF_START_STOP, 0x00);
			CHECK_ERR(err);
		} else {
			/* Single AF Start */
			err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
					M9MO_LENS_AF_START_STOP, 0x00);
			CHECK_ERR(err);
		}
	} else {
		state->focus.status |= 0x8000;
		if (state->focus.mode == FOCUS_MODE_CONTINOUS) {
			/* Stop Continuous AF */
			err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
					M9MO_LENS_AF_START_STOP, 0x02);
			CHECK_ERR(err);
		} else if (state->facedetect_mode) {
			/* Stop Continuous AF */
			err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
					M9MO_LENS_AF_START_STOP, 0x02);
			CHECK_ERR(err);
			err = m9mo_writeb(sd, M9MO_CATEGORY_FD,
				M9MO_FD_CTL, 0x11);
			CHECK_ERR(err);
		}
	}

	cam_dbg("X\n");
	return err;
}

static int m9mo_set_af_sensor_mode(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	u32 cancel;
	int err;
	int af_mode, af_window, af_range;

	cam_dbg("E, value %d\n", val);

	cancel = val & FOCUS_MODE_DEFAULT;
	val &= 0xFF;
	af_range = state->focus_range;

retry:
	switch (val) {
	case FOCUS_MODE_AUTO:
		af_mode = 0x00;
		af_window = state->focus_area_mode;
		break;

	case FOCUS_MODE_MULTI:
		af_mode = 0x00;
		af_window = 0x01;
		break;

	case FOCUS_MODE_CONTINOUS:
		af_mode = 0x01;
		af_range = 0x02;
		af_window = 0x00;
		cancel = 0;
		break;

	case FOCUS_MODE_FACEDETECT:
		af_mode = 0x01;
		af_window = 0x02;
		break;

	case FOCUS_MODE_TOUCH:
		af_mode = 0x00;
		af_window = 0x02;
		cancel = 0;
		break;

	case FOCUS_MODE_MACRO:
		af_mode = 0x00;
		af_range = 0x01;
		af_window = 0x00;
		break;

	case FOCUS_MODE_MANUAL:
		af_mode = 0x02;
		af_window = state->focus_area_mode;
		af_range = 0x02;
		cancel = 0;
		break;

	default:
		cam_warn("invalid value, %d", val);
		val = FOCUS_MODE_AUTO;
		goto retry;
	}

	if (cancel) {
		state->focus.status |= 0x8000;
		m9mo_set_af(sd, 0);
		m9mo_set_lock(sd, 0);
	}

	state->focus.mode = val;

	if (state->focus.mode != FOCUS_MODE_TOUCH) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			M9MO_LENS_AF_MODE, af_mode);
		CHECK_ERR(err);
	}

	err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
		M9MO_LENS_AF_SCAN_RANGE, af_range);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
		M9MO_LENS_AF_WINDOW_MODE, af_window);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_af_mode(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	u32 cancel;

	cam_dbg("E, value %d\n", val);

	cancel = val & FOCUS_MODE_DEFAULT;
	val &= 0xFF;

retry:
	switch (val) {
	case FOCUS_MODE_AUTO:
		state->focus_range = 0;
		break;

	case FOCUS_MODE_MACRO:
		state->focus_range = 1;
		break;

	case FOCUS_MODE_MANUAL:
		state->focus.mode = FOCUS_MODE_MANUAL;
		break;

	case FOCUS_MODE_CONTINOUS:
		state->focus.mode = FOCUS_MODE_CONTINOUS;
		break;

	default:
		cam_warn("invalid value, %d", val);
		val = FOCUS_MODE_AUTO;
		goto retry;
	}

	if (cancel)
		state->focus.mode |= cancel;

	m9mo_set_af_sensor_mode(sd, state->focus.mode);
	cam_trace("X\n");
	return 0;
}

static int m9mo_set_focus_area_mode(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

retry:
	switch (val) {
	case V4L2_FOCUS_AREA_CENTER:
		m9mo_set_object_tracking(sd, 0);
		m9mo_set_af_sensor_mode(sd, FOCUS_MODE_AUTO);
		break;

	case V4L2_FOCUS_AREA_MULTI:
		m9mo_set_object_tracking(sd, 0);
		m9mo_set_af_sensor_mode(sd, FOCUS_MODE_MULTI);
		break;

	case V4L2_FOCUS_AREA_SMART_TOUCH:
		state->focus_range = 0;
		break;

	default:
		cam_warn("invalid value, %d", val);
		val = V4L2_FOCUS_AREA_CENTER;
		goto retry;
	}

	state->focus_area_mode = val;

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_touch_auto_focus(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err = 0;
	cam_info("%s\n", val ? "start" : "stop");

	if (val) {
		m9mo_set_af_sensor_mode(sd, FOCUS_MODE_TOUCH);

		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
				M9MO_LENS_AF_TOUCH_POSX, state->focus.pos_x);
		CHECK_ERR(err);
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
				M9MO_LENS_AF_TOUCH_POSY, state->focus.pos_y);
		CHECK_ERR(err);
	}

	cam_trace("X\n");
	return err;
}

static int m9mo_set_zoom(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	struct v4l2_queryctrl qc = {0,};
	int val = ctrl->value, err;
	int opti_val, digi_val;
	int opti_max = 15;
	int optical_zoom_val[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		11, 12, 13, 14, 15};
	int zoom_val[] = { 1, 2, 3, 5, 6, 7, 9, 10, 11, 13,
		14, 15, 17, 18, 19, 20, 21, 22, 24, 25,
		26, 28, 29, 30, 31, 32, 34, 35, 36, 38, 39};
	cam_trace("E, value %d\n", val);

	qc.id = ctrl->id;
	m9mo_queryctrl(sd, &qc);

	if (val < qc.minimum || val > qc.maximum) {
		cam_warn("invalied value, %d\n", val);
		val = qc.default_value;
	}

	if (val <= opti_max) {
		opti_val = val;
		digi_val = 0;
	} else {
		opti_val = opti_max;
		digi_val = val - opti_max;
	}

	err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
		M9MO_LENS_AF_ZOOM_LEVEL, optical_zoom_val[opti_val]);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_MON,
		M9MO_MON_ZOOM, zoom_val[digi_val]);
	CHECK_ERR(err);

	state->zoom = val;

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_zoom_ctrl(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E, value %d\n", val);

	if (val == V4L2_OPTICAL_ZOOM_STOP) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			M9MO_LENS_AF_ZOOM_CTRL, val);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			M9MO_LENS_ZOOM_SET_INFO, 0);
		CHECK_ERR(err);
	} else if (val == V4L2_OPTICAL_ZOOM_PINCH) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			M9MO_LENS_ZOOM_SET_INFO, 0);
		CHECK_ERR(err);
	} else if (val == V4L2_OPTICAL_ZOOM_KEY) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			M9MO_LENS_ZOOM_SET_INFO, 1);
		CHECK_ERR(err);
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_jpeg_quality(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	struct v4l2_queryctrl qc = {0,};
	int val = ctrl->value, ratio, err;
	cam_dbg("E, value %d\n", val);

	qc.id = ctrl->id;
	m9mo_queryctrl(sd, &qc);

	if (val < qc.minimum || val > qc.maximum) {
		cam_warn("invalied value, %d\n", val);
		val = qc.default_value;
	}

	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
		M9MO_CAPPARM_JPEG_RATIO, 0x62);
	CHECK_ERR(err);

#if 0	/* m9mo */
	if (val <= 65)		/* Normal */
		ratio = 0x0A;
	else if (val <= 75)	/* Fine */
		ratio = 0x05;
	else			/* Superfine */
#endif
		ratio = 0x00;

	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
		M9MO_CAPPARM_JPEG_RATIO_OFS, ratio);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_get_exif(struct v4l2_subdev *sd)
{
	struct m9mo_state *state = to_state(sd);
	/* standard values */
	u16 iso_std_values[] = { 10, 12, 16, 20, 25, 32, 40, 50, 64, 80,
		100, 125, 160, 200, 250, 320, 400, 500, 640, 800,
		1000, 1250, 1600, 2000, 2500, 3200, 4000, 5000, 6400, 8000};
	/* quantization table */
	u16 iso_qtable[] = { 11, 14, 17, 22, 28, 35, 44, 56, 71, 89,
		112, 141, 178, 224, 282, 356, 449, 565, 712, 890,
		1122, 1414, 1782, 2245, 2828, 3564, 4490, 5657, 7127, 8909};
	int num, den, i, err;

	/* exposure time */
	err = m9mo_readl(sd, M9MO_CATEGORY_EXIF,
		M9MO_EXIF_EXPTIME_NUM, &num);
	CHECK_ERR(err);
	err = m9mo_readl(sd, M9MO_CATEGORY_EXIF,
		M9MO_EXIF_EXPTIME_DEN, &den);
	CHECK_ERR(err);
	state->exif.exptime = (u32)num*1000/den;

	/* flash */
	err = m9mo_readw(sd, M9MO_CATEGORY_EXIF, M9MO_EXIF_FLASH, &num);
	CHECK_ERR(err);
	state->exif.flash = (u16)num;

	/* iso */
	err = m9mo_readw(sd, M9MO_CATEGORY_EXIF, M9MO_EXIF_ISO, &num);
	CHECK_ERR(err);
	for (i = 0; i < NELEMS(iso_qtable); i++) {
		if (num <= iso_qtable[i]) {
			state->exif.iso = iso_std_values[i];
			break;
		}
	}

	/* shutter speed */
	err = m9mo_readl(sd, M9MO_CATEGORY_EXIF, M9MO_EXIF_TV_NUM, &num);
	CHECK_ERR(err);
	err = m9mo_readl(sd, M9MO_CATEGORY_EXIF, M9MO_EXIF_TV_DEN, &den);
	CHECK_ERR(err);
	state->exif.tv = num*M9MO_DEF_APEX_DEN/den;

	/* brightness */
	err = m9mo_readl(sd, M9MO_CATEGORY_EXIF, M9MO_EXIF_BV_NUM, &num);
	CHECK_ERR(err);
	err = m9mo_readl(sd, M9MO_CATEGORY_EXIF, M9MO_EXIF_BV_DEN, &den);
	CHECK_ERR(err);
	state->exif.bv = num*M9MO_DEF_APEX_DEN/den;

	/* exposure */
	err = m9mo_readl(sd, M9MO_CATEGORY_EXIF, M9MO_EXIF_EBV_NUM, &num);
	CHECK_ERR(err);
	err = m9mo_readl(sd, M9MO_CATEGORY_EXIF, M9MO_EXIF_EBV_DEN, &den);
	CHECK_ERR(err);
	state->exif.ebv = num*M9MO_DEF_APEX_DEN/den;

	return err;
}

static int m9mo_get_fd_eye_blink_result(struct v4l2_subdev *sd)
{
	struct m9mo_state *state = to_state(sd);
	int err;
	s32 val_no = 1, val_level = 0;

	/* EyeBlink error check FRAME No, Level */
	err = m9mo_readb(sd, M9MO_CATEGORY_FD,
			M9MO_FD_BLINK_FRAMENO, &val_no);
	CHECK_ERR(err);
	if (val_no < 1 || val_no > 3) {
		val_no = 1;
		cam_warn("Read Error FD_BLINK_FRAMENO [0x%x]\n", val_no);
	}
	err = m9mo_readb(sd, M9MO_CATEGORY_FD,
			M9MO_FD_BLINK_LEVEL_1+val_no-1, &val_level);
	CHECK_ERR(err);

	if (val_level >= 0x05)
		state->fd_eyeblink_cap = 1;
	else
		state->fd_eyeblink_cap = 0;
	cam_dbg("blink no[%d] level[0x%x]\n", val_no, val_level);

	return err;
}

static int m9mo_get_red_eye_fix_result(struct v4l2_subdev *sd)
{
	struct m9mo_state *state = to_state(sd);
	int err;
	s32 red_eye_status;

	if (state->flash_mode != FLASH_MODE_RED_EYE_FIX)
		return 0;

	err = m9mo_readb(sd, M9MO_CATEGORY_FD,
			M9MO_FD_RED_DET_STATUS, &red_eye_status);
	CHECK_ERR(err);

	state->fd_red_eye_status = red_eye_status;

	cam_dbg("red eye status [0x%x]\n", red_eye_status);

	return err;
}

static int m9mo_start_postview_capture(struct v4l2_subdev *sd, int frame_num)
{
	struct m9mo_state *state = to_state(sd);
	int err, int_factor;
	cam_trace("E\n");

	if (state->running_capture_mode == RUNNING_MODE_CONTINUOUS) {
		/* Select image number of frame */
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
				M9MO_CAPCTRL_FRM_PRV_SEL, frame_num);

		/* Clear Interrupt factor */
		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_CAPTURE)) {
			cam_warn("M9MO_INT_CAPTURE isn't issued on frame select, %#x\n",
					int_factor);
			return -ETIMEDOUT;
		}
	} else if (state->running_capture_mode == RUNNING_MODE_BRACKET) {
		/* Select image number of frame */
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
				M9MO_CAPCTRL_FRM_PRV_SEL, frame_num);

		/* Clear Interrupt factor */
		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_CAPTURE)) {
			cam_warn("M9MO_INT_CAPTURE isn't issued on frame select, %#x\n",
					int_factor);
			return -ETIMEDOUT;
		}
	} else if (state->running_capture_mode == RUNNING_MODE_HDR) {
		cam_warn("HDR have no PostView\n");
		return 0;
	} else if (state->running_capture_mode == RUNNING_MODE_BLINK) {
		/* Select image number of frame */
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
				M9MO_CAPCTRL_FRM_PRV_SEL, 0xFF);

		/* Clear Interrupt factor */
		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_CAPTURE)) {
			cam_warn("M9MO_INT_CAPTURE isn't issued on frame select, %#x\n",
					int_factor);
			return -ETIMEDOUT;
		}
	} else {
		/* Select image number of frame */
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
				M9MO_CAPCTRL_FRM_PRV_SEL, 0x01);
	}
	CHECK_ERR(err);

	/* Set YUV out for Preview */
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
			M9MO_CAPPARM_YUVOUT_PREVIEW, 0x00);
	CHECK_ERR(err);

	/* Set Preview Image size */
	if (FRM_RATIO(state->capture) == CAM_FRMRATIO_WVGA) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
				M9MO_CAPPARM_PREVIEW_IMG_SIZE, 0x0F);
		CHECK_ERR(err);
	} else if (FRM_RATIO(state->capture) == CAM_FRMRATIO_VGA) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
				M9MO_CAPPARM_PREVIEW_IMG_SIZE, 0x13);
		CHECK_ERR(err);
	}

	/* Get Preview data */
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
			M9MO_CAPCTRL_TRANSFER, 0x02);
	CHECK_ERR(err);

	/* Clear Interrupt factor */
	int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
	if (!(int_factor & M9MO_INT_CAPTURE)) {
		cam_warn("M9MO_INT_CAPTURE isn't issued on transfer, %#x\n",
				int_factor);
		return -ETIMEDOUT;
	}

/*
	err = m9mo_readl(sd, M9MO_CATEGORY_CAPCTRL, M9MO_CAPCTRL_IMG_SIZE,
				&state->jpeg.main_size);
	CHECK_ERR(err);

	err = m9mo_readl(sd, M9MO_CATEGORY_CAPCTRL, M9MO_CAPCTRL_THUMB_SIZE,
				&state->jpeg.thumb_size);
	CHECK_ERR(err);

	state->jpeg.main_offset = 0;
	state->jpeg.thumb_offset = M9MO_JPEG_MAXSIZE;
	state->jpeg.postview_offset = M9MO_JPEG_MAXSIZE + M9MO_THUMB_MAXSIZE;

	m9mo_get_exif(sd);
*/
	cam_trace("X\n");
	return err;
}

static int m9mo_start_YUV_capture(struct v4l2_subdev *sd, int frame_num)
{
	struct m9mo_state *state = to_state(sd);
	int err, int_factor;
	cam_trace("E\n");

	/* Select image number of frame */
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
	M9MO_CAPCTRL_FRM_SEL, frame_num);
	CHECK_ERR(err);

	/* Clear Interrupt factor */
	int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
	if (!(int_factor & M9MO_INT_CAPTURE)) {
		cam_warn("M9MO_INT_CAPTURE isn't issued on frame select, %#x\n",
				int_factor);
		return -ETIMEDOUT;
	}

	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
			M9MO_CAPPARM_YUVOUT_MAIN, 0x00);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
		M9MO_CAPPARM_MAIN_IMG_SIZE, state->capture->reg_val);
	CHECK_ERR(err);
	cam_trace("Select image size [ width %d, height : %d ]\n",
			state->capture->width, state->capture->height);

	/* Get main YUV data */
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
			M9MO_CAPCTRL_TRANSFER, 0x01);
	CHECK_ERR(err);

	/* Clear Interrupt factor */
	int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
	if (!(int_factor & M9MO_INT_CAPTURE)) {
		cam_warn("M9MO_INT_CAPTURE isn't issued on transfer, %#x\n",
				int_factor);
		return -ETIMEDOUT;
	}

	err = m9mo_readl(sd, M9MO_CATEGORY_CAPCTRL, M9MO_CAPCTRL_IMG_SIZE,
				&state->jpeg.main_size);
	CHECK_ERR(err);
/*
	err = m9mo_readl(sd, M9MO_CATEGORY_CAPCTRL, M9MO_CAPCTRL_THUMB_SIZE,
				&state->jpeg.thumb_size);
	CHECK_ERR(err);

	state->jpeg.main_offset = 0;
	state->jpeg.thumb_offset = M9MO_JPEG_MAXSIZE;
	state->jpeg.postview_offset = M9MO_JPEG_MAXSIZE + M9MO_THUMB_MAXSIZE;

	m9mo_get_exif(sd);
*/
	cam_trace("X\n");
	return err;
}

static int m9mo_start_capture(struct v4l2_subdev *sd, int frame_num)
{
	struct m9mo_state *state = to_state(sd);
	int err, int_factor;
	cam_trace("E\n");

	if (state->running_capture_mode == RUNNING_MODE_CONTINUOUS) {
		/* Select image number of frame */
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
				M9MO_CAPCTRL_FRM_SEL, frame_num);
		CHECK_ERR(err);

		/* Clear Interrupt factor */
		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_CAPTURE)) {
			cam_warn("M9MO_INT_CAPTURE isn't issued on frame select, %#x\n",
					int_factor);
			return -ETIMEDOUT;
		}
	} else if (state->running_capture_mode == RUNNING_MODE_BRACKET) {
		/* Select image number of frame */
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
				M9MO_CAPCTRL_FRM_SEL, frame_num);
		CHECK_ERR(err);

		/* Clear Interrupt factor */
		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_CAPTURE)) {
			cam_warn("M9MO_INT_CAPTURE isn't issued on frame select, %#x\n",
					int_factor);
			return -ETIMEDOUT;
		}
	} else if (state->running_capture_mode == RUNNING_MODE_BLINK) {
		/* Select image number of frame */
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
				M9MO_CAPCTRL_FRM_SEL, 0xFF);
		CHECK_ERR(err);

		/* Clear Interrupt factor */
		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_CAPTURE)) {
			cam_warn("M9MO_INT_CAPTURE isn't issued on frame select, %#x\n",
					int_factor);
			return -ETIMEDOUT;
		}

		err = m9mo_get_fd_eye_blink_result(sd);
		CHECK_ERR(err);
	} else {
		/* Select image number of frame */
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
				M9MO_CAPCTRL_FRM_SEL, 0x01);
		CHECK_ERR(err);
	}

	m9mo_get_red_eye_fix_result(sd);

#if 0
	/* Set main image JPEG fime max size */
	err = m9mo_writel(sd, M9MO_CATEGORY_CAPPARM,
			M9MO_CAPPARM_JPEG_SIZE_MAX, 0x01000000);
	CHECK_ERR(err);

	/* Set main image JPEG fime min size */
	err = m9mo_writel(sd, M9MO_CATEGORY_CAPPARM,
			M9MO_CAPPARM_JPEG_SIZE_MIN, 0x00100000);
	CHECK_ERR(err);
#endif

	/* Select main image format */
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
			M9MO_CAPPARM_YUVOUT_MAIN, 0x01);
	CHECK_ERR(err);

#if 0
	/* Select main image size */
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
			M9MO_CAPPARM_MAIN_IMG_SIZE, 0x31);
	CHECK_ERR(err);
#endif

	/* Get main JPEG data */
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
			M9MO_CAPCTRL_TRANSFER, 0x01);

	/* Clear Interrupt factor */
	int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
	if (!(int_factor & M9MO_INT_CAPTURE)) {
		cam_warn("M9MO_INT_CAPTURE isn't issued on transfer, %#x\n",
				int_factor);
		return -ETIMEDOUT;
	}

	err = m9mo_readl(sd, M9MO_CATEGORY_CAPCTRL, M9MO_CAPCTRL_IMG_SIZE,
				&state->jpeg.main_size);
	CHECK_ERR(err);

/*
	err = m9mo_readl(sd, M9MO_CATEGORY_CAPCTRL, M9MO_CAPCTRL_THUMB_SIZE,
				&state->jpeg.thumb_size);
	CHECK_ERR(err);
*/
	state->jpeg.main_offset = 0;
	state->jpeg.thumb_offset = M9MO_JPEG_MAXSIZE;
	state->jpeg.postview_offset = M9MO_JPEG_MAXSIZE + M9MO_THUMB_MAXSIZE;

	m9mo_get_exif(sd);

	cam_trace("X\n");
	return err;
}

/*static int m9mo_set_hdr(struct v4l2_subdev *sd, int val)
{
	cam_trace("E val : %d\n", val);
	cam_trace("X\n");
	return 0;
}*/


static int m9mo_set_facedetect(struct v4l2_subdev *sd, int val)
{
	int err;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	state->facedetect_mode = val;

	switch (state->facedetect_mode) {
	case FACE_DETECTION_NORMAL:
		cam_dbg("~~~~~~ face detect on ~~~~~~ val : %d\n", val);
		err = m9mo_writeb(sd, M9MO_CATEGORY_FD, M9MO_FD_SIZE, 0x04);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_FD, M9MO_FD_MAX, 0x07);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_FD, M9MO_FD_CTL, 0x11);
		CHECK_ERR(err);

#if 0	/* AF */
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
				M9MO_LENS_AF_SCAN_RANGE, 0x02);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
				M9MO_LENS_AF_WINDOW_MODE, 0x02);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
				M9MO_LENS_AF_MODE, 0x00);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
				M9MO_LENS_AF_START_STOP, 0x00);
		CHECK_ERR(err);
#endif
		break;

	case FACE_DETECTION_SMILE_SHOT:
		cam_dbg("~~~~~~ fd smile shot ~~~~~~ val : %d\n", val);
		break;

	case FACE_DETECTION_BLINK:
		cam_dbg("~~~~~~ fd eye blink ~~~~~~ val : %d\n", val);
		break;

	case FACE_DETECTION_OFF:
	default:
		cam_dbg("~~~~~~ face detect off ~~~~~~ val : %d\n", val);
		err = m9mo_writeb(sd, M9MO_CATEGORY_FD, M9MO_FD_CTL, 0x00);
		CHECK_ERR(err);
		break;
	}
	cam_trace("X\n");
	return 0;
}

static int m9mo_set_bracket(struct v4l2_subdev *sd, int val)
{
	cam_trace("E val : %d\n", val);

	switch (val) {
	case BRACKET_MODE_OFF:
		cam_dbg("~~~~~~ bracket off ~~~~~~ val : %d\n", val);
		break;

	case BRACKET_MODE_AEB:
		cam_dbg("~~~~~~ bracket aeb on ~~~~~~ val : %d\n", val);
		break;

	case BRACKET_MODE_WBB:
		cam_dbg("~~~~~~ bracket wbb on ~~~~~~ val : %d\n", val);
		break;

	default:
		cam_err("~~~~ TBD ~~~~ val : %d", val);
		break;
	}
	cam_trace("X\n");
	return 0;
}

static int m9mo_set_bracket_aeb(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_trace("E val : %d\n", val);

	switch (val) {
	case BRACKET_AEB_VALUE1:
		cam_trace("~~~~~~ AEB value1 ~~~~~~ val : %d\n", val);
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
				M9MO_AE_AUTO_BRACKET_EV, 0x1E); /* EV 0.3 */
		break;

	case BRACKET_AEB_VALUE2:
		cam_trace("~~~~~~ AEB value2 ~~~~~~ val : %d\n", val);
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
				M9MO_AE_AUTO_BRACKET_EV, 0x3C); /* EV 0.6 */
		break;

	case BRACKET_AEB_VALUE3:
		cam_trace("~~~~~~ AEB value3 ~~~~~~ val : %d\n", val);
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
				M9MO_AE_AUTO_BRACKET_EV, 0x64); /* EV 1.0 */
		break;

	case BRACKET_AEB_VALUE4:
		cam_trace("~~~~~~ AEB value4 ~~~~~~ val : %d\n", val);
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
				M9MO_AE_AUTO_BRACKET_EV, 0x82); /* EV 1.3 */
		break;

	case BRACKET_AEB_VALUE5:
		cam_trace("~~~~~~ AEB value5 ~~~~~~ val : %d\n", val);
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
				M9MO_AE_AUTO_BRACKET_EV, 0xA0); /* EV 1.6 */
		break;

	case BRACKET_AEB_VALUE6:
		cam_trace("~~~~~~ AEB value6 ~~~~~~ val : %d\n", val);
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
				M9MO_AE_AUTO_BRACKET_EV, 0xC8); /* EV 2.0 */
		break;

	default:
		cam_err("~~~~ TBD ~~~~ val : %d", val);
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
				M9MO_AE_AUTO_BRACKET_EV, 0x64); /* Ev 1.0 */
		break;
	}
	cam_trace("X\n");
	return 0;
}

static int m9mo_set_bracket_wbb(struct v4l2_subdev *sd, int val)
{
	cam_trace("E val : %d\n", val);

	switch (val) {
	case BRACKET_WBB_VALUE1:
		cam_trace("~~~~~~ WBB value1 ~~~~~~ val : %d\n", val);
		break;

	case BRACKET_WBB_VALUE2:
		cam_trace("~~~~~~ WBB value2 ~~~~~~ val : %d\n", val);
		break;

	case BRACKET_WBB_VALUE3:
		cam_trace("~~~~~~ WBB value3 ~~~~~~ val : %d\n", val);
		break;

	case BRACKET_WBB_VALUE4:
		cam_trace("~~~~~~ WBB value4 ~~~~~~ val : %d\n", val);
		break;

	case BRACKET_WBB_VALUE5:
		cam_trace("~~~~~~ WBB value5 ~~~~~~ val : %d\n", val);
		break;

	case BRACKET_WBB_VALUE6:
		cam_trace("~~~~~~ WBB value6 ~~~~~~ val : %d\n", val);
		break;

	default:
		cam_err("~~~~ TBD ~~~~ val : %d", val);
		break;
	}
	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_cam_sys_mode(struct v4l2_subdev *sd, int val)
{
	int err, old_mode;
	u32 int_factor;

	struct m9mo_state *state = to_state(sd);
	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_SYSMODE_CAPTURE:
		cam_trace("~ FACTORY_SYSMODE_CAPTURE ~\n");

		err = m9mo_writeb(sd, M9MO_CATEGORY_SYS,
				M9MO_SYS_MODE, M9MO_STILLCAP_MODE);
		CHECK_ERR(err);

		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_CAPTURE)) {
			cam_warn("M9MO_INT_CAPTURE isn't issued, %#x\n",
					int_factor);
			return -ETIMEDOUT;
		}
		break;

	case FACTORY_SYSMODE_MONITOR:
		cam_trace("~ FACTORY_SYSMODE_MONITOR ~\n");

		err = m9mo_writeb(sd, M9MO_CATEGORY_SYS,
				M9MO_SYS_MODE, M9MO_MONITOR_MODE);
		CHECK_ERR(err);

		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor)) {
			cam_warn("M9MO_INT_CAPTURE isn't issued, %#x\n",
					int_factor);
			return -ETIMEDOUT;
		}
		break;

	case FACTORY_SYSMODE_PARAM:
		cam_trace("~ FACTORY_SYSMODE_PARAM ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_SYS,
				M9MO_SYS_MODE, M9MO_PARMSET_MODE);
		CHECK_ERR(err);
		break;

	default:
		cam_trace("~ FACTORY_SYSMODE_DEFAULT ~\n");
		break;
	}
	cam_trace("X\n");
	return 0;

}

static int m9mo_set_fps(struct v4l2_subdev *sd, int val)
{
	int err, old_mode;
	u32 int_factor;

	struct m9mo_state *state = to_state(sd);
	cam_trace("E val : %d\n", val);

	if (state->preview == NULL) {
		state->fps = val;
		cam_trace("~~~~~~ return ~~~~~~\n");
		return 0;
	}

	if (val == state->fps) {
		cam_info("same fps. skip\n");
		return 0;
	}
	if (val <= 0 || val > 120) {
		cam_err("invalid frame rate %d\n", val);
		val = 0; /* set to auto(default) */
	}

	cam_info("set AE EP to %d\n", val);

	switch (val) {
	case 120:
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
				M9MO_AE_EP_MODE_MON, 0x1C);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
				M9MO_AE_EP_MODE_CAP, 0x1C);
		CHECK_ERR(err);
		cam_trace("~~~~~~ 120 fps ~~~~~~\n");
		break;

	case 60:
		cam_trace("~~~~~~ 60 fps ~~~~~~\n");
		cam_info("Temporarilly set AE EP to 60\n");

		old_mode = m9mo_set_mode(sd, M9MO_PARMSET_MODE);
		CHECK_ERR(old_mode);

		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
				M9MO_AE_EP_MODE_MON, 0x1C);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
				M9MO_AE_EP_MODE_CAP, 0x1C);
		CHECK_ERR(err);

		if (state->preview->height == 480) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
			M9MO_PARM_MON_SIZE, 0x2F);
			CHECK_ERR(err);
		} else if (state->preview->height == 720) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
			M9MO_PARM_MON_SIZE, 0x21); /* FIXME */
			CHECK_ERR(err);
		}

		if (old_mode == M9MO_MONITOR_MODE) {
			err = m9mo_set_mode(sd, old_mode);
			CHECK_ERR(err);

			int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
			if (!(int_factor & M9MO_INT_MODE)) {
				cam_err("M9MO_INT_MODE isn't issued, %#x\n",
					int_factor);
				return -ETIMEDOUT;
			}
			CHECK_ERR(err);
		}
		break;

	default:
		cam_trace("~~~~~~ default : auto fps ~~~~~~\n");

		old_mode = m9mo_set_mode(sd, M9MO_PARMSET_MODE);
		CHECK_ERR(old_mode);

		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
				M9MO_AE_EP_MODE_MON, 0x00);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
				M9MO_AE_EP_MODE_CAP, 0x00);
		CHECK_ERR(err);

		if (state->preview->width == 640 &&
			state->preview->height == 480) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
				M9MO_PARM_MON_SIZE, 0x17);
			CHECK_ERR(err);
		} else if (state->preview->height == 720) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
				M9MO_PARM_MON_SIZE, 0x21);
			CHECK_ERR(err);
		}

		if (old_mode == M9MO_MONITOR_MODE) {
			err = m9mo_set_mode(sd, old_mode);
			CHECK_ERR(err);

			int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
			if (!(int_factor & M9MO_INT_MODE)) {
				cam_err("M9MO_INT_MODE isn't issued, %#x\n",
					int_factor);
				return -ETIMEDOUT;
			}
			CHECK_ERR(err);
		}
		break;
	}
	cam_trace("X\n");
	return 0;
}

static int m9mo_set_camera_mode(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);

	switch (val) {
	case MODE_A:
		cam_trace("~ MODE_A ~\n");
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			0x34, 0x01);
		CHECK_ERR(err);

		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			0x35, 0x01);
		CHECK_ERR(err);

		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			0x0A, 0x0C);
		CHECK_ERR(err);

		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			0x0B, 0x0C);
		CHECK_ERR(err);
		break;

	case MODE_M:
		cam_trace("~ MODE_M ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			0x34, 0x07);
		CHECK_ERR(err);
		break;

	case MODE_S:
		cam_trace("~ MODE_S ~\n");
		break;

	case MODE_PROGRAM:
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			0x34, 0x00);
		CHECK_ERR(err);
		cam_trace("~ MODE_PROGRAM ~\n");
		break;

	case MODE_SMART_AUTO:
		cam_trace("~ MODE_SMART_AUTO ~\n");
		break;

	default:
		cam_err("~ m9mo_set_factory_OIS ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_LDC(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_dbg("%s\n", val ? "on" : "off");

	if (val == 1) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
			0x1B, 0x01);
		CHECK_ERR(err);
	} else {
		err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
			0x1B, 0x00);
		CHECK_ERR(err);
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_LSC(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_dbg("%s\n", val ? "on" : "off");

	if (val == 1) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
			0x07, 0x01);
		CHECK_ERR(err);
	} else {
		err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
			0x07, 0x00);
		CHECK_ERR(err);
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_aperture_preview(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_trace("E val : %d\n", val);
	err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			0x3D, 0x28);
	CHECK_ERR(err);
	cam_trace("X\n");
	return 0;
}

static int m9mo_set_aperture_capture(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_trace("E val : %d\n", val);
	err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			0x36, 0x28);
	CHECK_ERR(err);
	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_OIS(struct v4l2_subdev *sd, int val)
{
	int err;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_OIS_RETURN_TO_CENTER:
		cam_trace("~ FACTORY_OIS_RETURN_TO_CENTER   ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x15, 0x30);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x16, 0x11);
		CHECK_ERR(err);
		break;

	case FACTORY_OIS_RUN:
		cam_trace("~ FACTORY_OIS_RUN ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x14, 0x00);
		CHECK_ERR(err);
		break;

	case FACTORY_OIS_START:
		cam_trace("~ FACTORY_OIS_START ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x20, 0x01);
		CHECK_ERR(err);
		break;

	case FACTORY_OIS_STOP:
		cam_trace("~ FACTORY_OIS_STOP ~\n");
		break;

	case FACTORY_OIS_MODE_ON:
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x10, 0x01);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_OIS_MODE_ON ~\n");
		break;

	case FACTORY_OIS_MODE_OFF:
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x10, 0x00);
		cam_trace("~ FACTORY_OIS_MODE_OFF ~\n");
		break;
	case FACTORY_OIS_LOG:
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x19, 0x01);
		CHECK_ERR(err);
		msleep(40);
		err = m9mo_make_CSV_rawdata(sd, M9MO_FLASH_FACTORY_OIS, false);
		CHECK_ERR(err);
		cam_trace("~FACTORY_OIS_LOG ~\n");
		break;

	default:
		cam_err("~ m9mo_set_factory_OIS ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_OIS_shift(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_trace("E val : 0x%x\n", val);
	err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			0x15, val);
	CHECK_ERR(err);
	err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			0x14, 0);
	CHECK_ERR(err);
	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_punt(struct v4l2_subdev *sd, int val)
{
	int err;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_PUNT_RANGE_START:
		cam_trace("~ FACTORY_PUNT_RANGE_START ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0E, 0x00);
		CHECK_ERR(err);
		break;

	case FACTORY_PUNT_RANGE_STOP:
		cam_trace("~ FACTORY_PUNT_RANGE_STOP ~\n");
		break;

	case FACTORY_PUNT_SHORT_SCAN_DATA:
		cam_trace("~ FACTORY_PUNT_SHORT_SCAN_DATA ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, 0x00);
		CHECK_ERR(err);
		break;

	case FACTORY_PUNT_SHORT_SCAN_START:
		cam_trace("~ FACTORY_PUNT_SHORT_SCAN_START ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0E, 0x01);
		CHECK_ERR(err);
		break;

	case FACTORY_PUNT_SHORT_SCAN_STOP:
		cam_trace("~ FACTORY_PUNT_SHORT_SCAN_STOP ~\n");
		break;

	case FACTORY_PUNT_LONG_SCAN_DATA:
		cam_trace("~ FACTORY_PUNT_LONG_SCAN_DATA ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, 0x00);
		CHECK_ERR(err);
		break;

	case FACTORY_PUNT_LONG_SCAN_START:
		cam_trace("~ FACTORY_PUNT_LONG_SCAN_START ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0E, 0x02);
		CHECK_ERR(err);
		break;

	case FACTORY_PUNT_LONG_SCAN_STOP:
		cam_trace("~FACTORY_PUNT_LONG_SCAN_STOP ~\n");
		break;

	case FACTORY_PUNT_LOG:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0E, 0x04);
		CHECK_ERR(err);
		msleep(40);
		err = m9mo_make_CSV_rawdata(sd, M9MO_FLASH_FACTORY_PUNT, false);
		CHECK_ERR(err);
		cam_trace("~FACTORY_PUNT_LOG ~\n");
		break;

	case FACTORY_PUNT_SET_RANGE_DATA:
		cam_trace("~FACTORY_PUNT_SET_RANGE_DATA ~\n");
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x17, (unsigned short)(state->f_punt_data.min));
		CHECK_ERR(err);

		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x19, (unsigned short)(state->f_punt_data.max));
		CHECK_ERR(err);

		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, (unsigned char)(state->f_punt_data.num));
		CHECK_ERR(err);

		cam_trace("~ FACTORY_PUNT_RANGE_START ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0E, 0x00);
		CHECK_ERR(err);
		break;

	case FACTORY_PUNT_EEP_WRITE:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0E, 0x05);
		CHECK_ERR(err);
		break;

	default:
		cam_err("~ m9mo_set_factory_punt ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_zoom(struct v4l2_subdev *sd, int val)
{
	int err;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_ZOOM_MOVE_STEP:
		cam_trace("~ FACTORY_ZOOM_MOVE_STEP ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0F, 0x00);
		CHECK_ERR(err);
		break;

	case FACTORY_ZOOM_RANGE_CHECK_START:
		cam_trace("~ FACTORY_ZOOM_RANGE_CHECK_START ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0F, 0x05);
		CHECK_ERR(err);
		break;

	case FACTORY_ZOOM_RANGE_CHECK_STOP:
		cam_trace("~ FACTORY_ZOOM_RANGE_CHECK_STOP ~\n");
		break;

	case FACTORY_ZOOM_SLOPE_CHECK_START:
		cam_trace("~ FACTORY_ZOOM_SLOPE_CHECK_START ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0E, 0x03);
		CHECK_ERR(err);
		break;

	case FACTORY_ZOOM_SLOPE_CHECK_STOP:
		cam_trace("~ FACTORY_ZOOM_SLOPE_CHECK_STOP ~\n");
		break;

	case FACTORY_ZOOM_SET_RANGE_CHECK_DATA:
		cam_trace("~ FACTORY_ZOOM_SET_RANGE_CHECK_DATA ~\n");
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x18, (unsigned short)(state->f_zoom_data.range_min));
		CHECK_ERR(err);

		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x1A, (unsigned short)(state->f_zoom_data.range_max));
		CHECK_ERR(err);
		break;

	case FACTORY_ZOOM_SET_SLOPE_CHECK_DATA:
		cam_trace("~ FACTORY_ZOOM_SET_SLOPE_CHECK_DATA ~\n");
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x18, (unsigned short)(state->f_zoom_data.slope_min));
		CHECK_ERR(err);

		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x1A, (unsigned short)(state->f_zoom_data.slope_max));
		CHECK_ERR(err);
		break;

	case FACTORY_ZOOM_STEP_TELE:
		cam_trace("~ FACTORY_ZOOM_STEP_TELE ~\n");
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x1A, 0x0F);
		CHECK_ERR(err);
		break;

	case FACTORY_ZOOM_STEP_WIDE:
		cam_trace("~ FACTORY_ZOOM_STEP_WIDE ~\n");
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x1A, 0x00);
		CHECK_ERR(err);
		break;

	default:
		cam_err("~ m9mo_set_factory_zoom ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_zoom_step(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);
#if 1
	if (val >= 0 && val < 16) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1A, val);
		CHECK_ERR(err);
		msleep(500);
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, 0x00);
		CHECK_ERR(err);
		msleep(500);
	}
	cam_trace("~ FACTORY_ZOOM_MOVE_STEP ~\n");
	err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
		0x0F, 0x00);
	CHECK_ERR(err);
	msleep(500);
#else
	err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
		0x06, val);
	CHECK_ERR(err);
	msleep(500);
#endif
	return 0;
}

static int m9mo_set_factory_fail_stop(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_FAIL_STOP_ON:
		cam_trace("~ FACTORY_FAIL_STOP_ON ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, 0x01);
		CHECK_ERR(err);
		break;

	case FACTORY_FAIL_STOP_OFF:
		cam_trace("~ FACTORY_FAIL_STOP_OFF ~\n");
		break;

	case FACTORY_FAIL_STOP_RUN:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0D, 0x0C);
		CHECK_ERR(err);

		cam_trace("~ FACTORY_FAIL_STOP_RUN ~\n");
		break;

	case FACTORY_FAIL_STOP_STOP:
		cam_trace("~ FACTORY_FAIL_STOP_STOP ~\n");
		break;

	default:
		cam_err("~ m9mo_set_factory_fail_stop ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_nodefocus(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_NODEFOCUSYES_ON:
		cam_trace("~ FACTORY_NODEFOCUSYES_ON ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, 0x01);
		CHECK_ERR(err);
		break;

	case FACTORY_NODEFOCUSYES_OFF:
		cam_trace("~ FACTORY_NODEFOCUSYES_OFF ~\n");
		break;

	case FACTORY_NODEFOCUSYES_RUN:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0D, 0x09);
		CHECK_ERR(err);

		cam_trace("~ FACTORY_NODEFOCUSYES_RUN ~\n");
		break;

	case FACTORY_NODEFOCUSYES_STOP:
		cam_trace("~ FACTORY_NODEFOCUSYES_STOP ~\n");
		break;

	default:
		cam_err("~ m9mo_set_factory_defocus ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_interpolation(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_INTERPOLATION_USE:
		cam_trace("~ FACTORY_INTERPOLATION_USE ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0E, 0x0A);
		CHECK_ERR(err);
		break;

	case FACTORY_INTERPOLATION_RELEASE:
		cam_trace("~ FACTORY_INTERPOLATION_RELEASE ~\n");
		break;

	default:
		cam_err("~ m9mo_set_factory_interpolation ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_common(struct v4l2_subdev *sd, int val)
{
	int err, down_check = 1, end_check = 0;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_FIRMWARE_DOWNLOAD:
		cam_trace("~ FACTORY_FIRMWARE_DOWNLOAD ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x11, 0x00);
		CHECK_ERR(err);
		break;

	case FACTORY_DOWNLOAD_CHECK:
		err = m9mo_readb(sd, M9MO_CATEGORY_NEW,
			0x11, &down_check);
		CHECK_ERR(err);
		state->factory_down_check = down_check;
		cam_trace("~ FACTORY_DOWNLOAD_CHECK ~\n");
		break;

	case FACTORY_END_CHECK:
		err = m9mo_readb(sd, M9MO_CATEGORY_LENS,
			0x40, &end_check);
		CHECK_ERR(err);
		state->factory_end_check = end_check;
		cam_trace("~ FACTORY_END_CHECK ~\n");
		break;

	case FACTORY_COMMON_SET_FOCUS_ZONE_MACRO:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x07, 0x02);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_COMMON_SET_FOCUS_ZONE_MACRO ~\n");
		break;

	default:
		cam_err("~ m9mo_set_factory_common ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_vib(struct v4l2_subdev *sd, int val)
{
	int err;
	struct m9mo_state *state = to_state(sd);

	cam_trace("m9mo_set_factory_vib E val : %d\n", val);

	switch (val) {
	case FACTORY_VIB_START:
		cam_trace("~ FACTORY_VIB_START ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x20, 0x02);
		CHECK_ERR(err);
		break;

	case FACTORY_VIB_STOP:
		cam_trace("~ FACTORY_VIB_STOP ~\n");
		break;

	case FACTORY_VIB_LOG:
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x19, 0x02);
		CHECK_ERR(err);
		msleep(40);
		err = m9mo_make_CSV_rawdata(sd, M9MO_FLASH_FACTORY_VIB, false);
		CHECK_ERR(err);
		cam_trace("~FACTORY_VIB_LOG ~\n");
		break;

	default:
		cam_err("~m9mo_set_factory_vib~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_gyro(struct v4l2_subdev *sd, int val)
{
	int err;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_GYRO_START:
		cam_trace("~ FACTORY_GYRO_START ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x20, 0x03);
		CHECK_ERR(err);
		break;

	case FACTORY_GYRO_STOP:
		cam_trace("~ FACTORY_GYRO_STOP ~\n");
		break;

	case FACTORY_GYRO_LOG:
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
			0x19, 0x03);
		CHECK_ERR(err);
		msleep(40);
		err = m9mo_make_CSV_rawdata(sd, M9MO_FLASH_FACTORY_GYRO, false);
		CHECK_ERR(err);
		cam_trace("~FACTORY_PUNT_LOG ~\n");
		break;
	default:
		cam_err("~ m9mo_set_factory_gyro ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_backlash(struct v4l2_subdev *sd, int val)
{
	int err;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_BACKLASH_INPUT:
		cam_trace("~ FACTORY_BACKLASH_INPUT ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0A, 0x01);
		CHECK_ERR(err);
		break;

	case FACTORY_BACKLASH_MAX_THR:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0A, 0x00);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_BACKLASH_MAX_THR ~\n");
		break;

	case FACTORY_BACKLASH_WIDE_RUN:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0A, 0x03);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_BACKLASH_WIDE_RUN ~\n");
		break;

	case FACTORY_BACKLASH_LOG:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0A, 0x05);
		CHECK_ERR(err);
		msleep(40);
		err = m9mo_make_CSV_rawdata(sd,
			M9MO_FLASH_FACTORY_BACKLASH, false);
		CHECK_ERR(err);
		cam_trace("~FACTORY_BACKLASH_LOG ~\n");
		break;

	default:
		cam_err("~ m9mo_set_factory_backlash ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_backlash_count(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);

	err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
		0x1B, val);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_af(struct v4l2_subdev *sd, int val)
{
	int err, end_check = 0;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_AF_LOCK_ON_SET:
		cam_trace("~ FACTORY_AF_LOCK_ON_SET ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, 0x01);
		CHECK_ERR(err);
		break;

	case FACTORY_AF_LOCK_OFF_SET:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, 0x00);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_AF_LOCK_OFF_SET ~\n");
		break;

	case FACTORY_AF_MOVE:
		cam_trace("~ FACTORY_AF_MOVE ~\n");
		break;

	case FACTORY_AF_STEP_LOG:
		if (state->factory_test_num == 106) {
			cam_trace("~ FACTORY_AF_STEP_LOG WIDE ~\n");
			err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
				0x0D, 0x1A);
			CHECK_ERR(err);
			msleep(40);
			err = m9mo_make_CSV_rawdata(sd,
				M9MO_FLASH_FACTORY_WIDE_RESOL, false);
			CHECK_ERR(err);
		} else if (state->factory_test_num == 107) {
			cam_trace("~ FACTORY_AF_STEP_LOG TELE ~\n");
			err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
				0x0D, 0x19);
			CHECK_ERR(err);
			msleep(40);
			err = m9mo_make_CSV_rawdata(sd,
				M9MO_FLASH_FACTORY_TELE_RESOL, false);
			CHECK_ERR(err);
		} else {
			cam_trace("~ FACTORY NUMBER ERROR ~\n");
		}
		break;

	case FACTORY_AF_LOCK_START:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0D, 0x01);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_AF_LOCK_START ~\n");
		break;

	case FACTORY_AF_LOCK_STOP:
		cam_trace("~ FACTORY_AF_LOCK_STOP ~\n");
		break;

	case FACTORY_AF_FOCUS_LOG:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0D, 0x0B);
		CHECK_ERR(err);
		msleep(40);
		err = m9mo_make_CSV_rawdata(sd,
			M9MO_FLASH_FACTORY_AF_FCS, false);
		CHECK_ERR(err);

		cam_trace("~ FACTORY_AF_FOCUS_LOG ~\n");
		break;

	case FACTORY_AF_INT_SET:
		cam_trace("~ FACTORY_AF_INT_SET ~\n");
		break;

	case FACTORY_AF_STEP_SAVE:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0D, 0x0A);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_AF_SETP_SAVE ~\n");
		break;

	case FACTORY_AF_SCAN_LIMIT_START:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0D, 0x06);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_AF_SCAN_LIMIT_START ~\n");
		break;

	case FACTORY_AF_SCAN_LIMIT_STOP:
		cam_trace("~ FACTORY_AF_SCAN_LIMIT_STOP ~\n");
		break;

	case FACTORY_AF_SCAN_RANGE_START:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0D, 0x07);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_AF_SCAN_RANGE_START ~\n");
		break;

	case FACTORY_AF_SCAN_RANGE_STOP:
		cam_trace("~ FACTORY_AF_SCAN_RANGE_STOP ~\n");
		break;

	case FACTORY_AF_LED_END_CHECK:
		cam_trace("~ FACTORY_AF_LED_END_CHECK ~\n");
		err = m9mo_readb(sd, M9MO_CATEGORY_LENS,
			0x40, &end_check);
		CHECK_ERR(err);
		state->factory_end_check = end_check;
		break;
	case FACTORY_AF_LED_LOG:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x4F, 0x02);
		CHECK_ERR(err);
		msleep(40);
		err = m9mo_make_CSV_rawdata(sd,
			M9MO_FLASH_FACTORY_AF_LED, false);
		CHECK_ERR(err);

		cam_trace("~ FACTORY_AF_LED_LOG ~\n");
		break;

	default:
		cam_err("~ m9mo_set_factory_af ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_af_step(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);

	err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
		0x1A, val);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_af_position(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);

	err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0B, val);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_defocus(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_DEFOCUS_RUN:
		cam_trace("~ FACTORY_DEFOCUS_RUN ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
				0x0D, 0x18);
		CHECK_ERR(err);
		break;

	case FACTORY_DEFOCUS_STOP:
		cam_trace("~ FACTORY_DEFOCUS_STOP ~\n");
		break;

	default:
		cam_err("~ m9mo_set_factory_defocus ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_defocus_wide(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);

	err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1A, val);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_defocus_tele(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);

	err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, val);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_resol_cap(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_CAP_COMP_ON:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, 0x01);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_CAP_COMP_ON ~\n");
		break;

	case FACTORY_CAP_COMP_OFF:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, 0x00);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_CAP_COMP_OFF ~\n");
		break;

	case FACTORY_CAP_COMP_START:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0D, 0x04);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_CAP_COMP_START ~\n");
		break;

	case FACTORY_CAP_COMP_STOP:
		cam_trace("~ FACTORY_CAP_COMP_STOP ~\n");
		break;

	case FACTORY_CAP_BARREL_ON:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, 0x01);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_CAP_BARREL_ON ~\n");
		break;

	case FACTORY_CAP_BARREL_OFF:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, 0x00);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_CAP_BARREL_OFF ~\n");
		break;

	case FACTORY_CAP_BARREL_START:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0D, 0x05);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_CAP_BARREL_START ~\n");
		break;

	case FACTORY_CAP_BARREL_STOP:
		cam_trace("~ FACTORY_CAP_BARREL_STOP ~\n");
		break;

	default:
		cam_err("~ m9mo_set_factory_resol_cap ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_af_zone(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_AFZONE_NORMAL:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x07, 0x00);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_AFZONE_NORMAL ~\n");
		break;

	case FACTORY_AFZONE_MACRO:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x07, 0x01);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_AFZONE_MACRO ~\n");
		break;

	case FACTORY_AFZONE_AUTO:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x07, 0x02);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_AFZONE_AUTO ~\n");
		break;

	default:
		cam_err("~ m9mo_set_factory_resol_cap ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_af_lens(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_AFLENS_OPEN:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x00, 0x00);
		CHECK_ERR(err);

		cam_trace("~ FACTORY_AFLENS_OPEN ~\n");
		break;

	case FACTORY_AFLENS_CLOSE:
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x01, 0x00);
		CHECK_ERR(err);
		cam_trace("~ FACTORY_AFLENS_CLOSE ~\n");
		break;

	default:
		cam_err("~ m9mo_set_factory_resol_cap ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_adj_iris(struct v4l2_subdev *sd, int val)
{
	int err, end_check = 0;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_ADJ_IRIS_RUN:
		cam_trace("~ FACTORY_ADJ_IRIS_RUN ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x53, 0x01);
		CHECK_ERR(err);
		break;

	case FACTORY_ADJ_IRIS_STOP:
		cam_trace("~ FACTORY_ADJ_IRIS_STOP ~\n");
		break;

	case FACTORY_ADJ_IRIS_END_CHECK:
		cam_trace("~ FACTORY_ADJ_IRIS_END_CHECK ~\n");
		err = m9mo_readb(sd, M9MO_CATEGORY_ADJST,
			0x53, &end_check);
		CHECK_ERR(err);
		state->factory_end_check = end_check;
		cam_trace("adj_iris::FACTORY_ADJ_IRIS_END_CHECK=%d\n",
			end_check);
		break;

	case FACTORY_ADJ_IRIS_LOG:
		err = m9mo_make_CSV_rawdata(sd,
			M9MO_FLASH_FACTORY_IRIS, true);
		CHECK_ERR(err);
		break;

	default:
		cam_err("~ m9mo_set_factory_adj_iris ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_sh_close(struct v4l2_subdev *sd, int val)
{
	int err, end_check = 0;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_SH_CLOSE_RUN:
		cam_trace("~ FACTORY_SH_CLOSE_RUN ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x53, 0x05);
		CHECK_ERR(err);
		break;

	case FACTORY_SH_CLOSE_STOP:
		cam_trace("~ FACTORY_SH_CLOSE_STOP ~\n");
		break;

	case FACTORY_SH_CLOSE_END_CHECK:
		cam_trace("~ FACTORY_SH_CLOSE_END_CHECK ~\n");
		err = m9mo_readb(sd, M9MO_CATEGORY_ADJST,
			0x53, &end_check);
		CHECK_ERR(err);
		state->factory_end_check = end_check;
		break;

	case FACTORY_SH_CLOSE_LOG:
		cam_trace("~ FACTORY_SH_CLOSE_LOG ~\n");
		err = m9mo_make_CSV_rawdata(sd,
			M9MO_FLASH_FACTORY_SH_CLOSE, true);
		CHECK_ERR(err);
		break;

	default:
		cam_err("~ m9mo_set_factory_adj_iris ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_adj_gain_liveview(struct v4l2_subdev *sd, int val)
{
	int err, end_check = 0;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_ADJ_GAIN_LIVEVIEW_RUN:
		cam_trace("~ FACTORY_ADJ_GAIN_LIVEVIEW_RUN ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x53, 0x03);
		CHECK_ERR(err);
		break;

	case FACTORY_ADJ_GAIN_LIVEVIEW_STOP:
		cam_trace("~ FACTORY_ADJ_GAIN_LIVEVIEW_STOP ~\n");
		break;

	case FACTORY_ADJ_GAIN_LIVEVIEW_END_CHECK:
		cam_trace("~ FACTORY_ADJ_GAIN_LIVEVIEW_END_CHECK ~\n");
		err = m9mo_readb(sd, M9MO_CATEGORY_ADJST,
			0x53, &end_check);
		CHECK_ERR(err);
		state->factory_end_check = end_check;
		break;

	case FACTORY_ADJ_GAIN_LIVEVIEW_LOG:
		cam_trace("~ FACTORY_ADJ_GAIN_LIVEVIEW_END_CHECK ~\n");
		err = m9mo_make_CSV_rawdata(sd,
			M9MO_FLASH_FACTORY_LIVEVIEW, true);
		CHECK_ERR(err);
		break;

	default:
		cam_err("~ m9mo_set_factory_adj_iris ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_flicker(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_FLICKER_AUTO:
		cam_trace("~ FACTORY_FLICKER_AUTO ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_FLICKER, 0x00);
		CHECK_ERR(err);
		break;

	case FACTORY_FLICKER_50HZ:
		cam_trace("~ FACTORY_FLICKER_50HZ ~\n");
		CHECK_ERR(err);
		break;

	case FACTORY_FLICKER_60HZ:
		cam_trace("~ FACTORY_FLICKER_60HZ ~\n");
		break;

	case FACTORY_FLICKER_50_60:
		cam_trace("~ FACTORY_FLICKER_50_60 ~\n");
		break;

	case FACTORY_FLICKER_OFF:
		cam_trace("~ FACTORY_FLICKER_OFF ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_FLICKER, 0x04);
		CHECK_ERR(err);
		break;

	default:
		cam_err("~ m9mo_set_factory_adj_iris ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_capture_gain(struct v4l2_subdev *sd, int val)
{
	int err, end_check = 0;
	struct m9mo_state *state = to_state(sd);
	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_CAPTURE_GAIN_RUN:
		cam_trace("~ FACTORY_CAPTURE_GAIN_RUN ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x53, 0x07);
		CHECK_ERR(err);
		break;

	case FACTORY_CAPTURE_GAIN_STOP:
		cam_trace("~ FACTORY_CAPTURE_GAIN_STOP ~\n");
		break;

	case FACTORY_CAPTURE_GAIN_END_CHECK:
		cam_trace("~ FACTORY_CAPTURE_GAIN_END_CHECK ~\n");
		err = m9mo_readb(sd, M9MO_CATEGORY_ADJST,
			0x53, &end_check);
		CHECK_ERR(err);
		state->factory_end_check = end_check;
		break;

	case FACTORY_CAPTURE_GAIN_LOG:
		err = m9mo_make_CSV_rawdata(sd,
			M9MO_FLASH_FACTORY_GAIN_CAPTURE, true);
		CHECK_ERR(err);
		cam_trace("~FACTORY_OIS_LOG ~\n");
		break;

	default:
		cam_err("~ m9mo_set_factory_adj_iris ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_image_stabilizer_mode(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err, old_mode, int_factor;
	int cnt = 30;
	s32 ois_stability = 1;
	cam_trace("E: mode %d\n", val);

	old_mode = m9mo_set_mode(sd, M9MO_PARMSET_MODE);
	CHECK_ERR(old_mode);

retry:
	switch (val) {
	case V4L2_IMAGE_STABILIZER_OFF:
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW, 0x11, 0x04);
		CHECK_ERR(err);

		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW, 0x1A, 0x01);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW, 0x11, 0x01);
		CHECK_ERR(err);

		err = m9mo_readb(sd, M9MO_CATEGORY_NEW,
				0x1A, &ois_stability);
		CHECK_ERR(err);
		while (ois_stability && cnt) {
			mdelay(20);
			err = m9mo_readb(sd, M9MO_CATEGORY_NEW,
					0x1A, &ois_stability);
			CHECK_ERR(err);
			cnt--;
		}
		break;

	case V4L2_IMAGE_STABILIZER_OIS:
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW, 0x1A, 0x01);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW, 0x11, 0x02);
		CHECK_ERR(err);

		err = m9mo_readb(sd, M9MO_CATEGORY_NEW,
				0x1A, &ois_stability);
		CHECK_ERR(err);
		while (ois_stability && cnt) {
			mdelay(20);
			err = m9mo_readb(sd, M9MO_CATEGORY_NEW,
					0x1A, &ois_stability);
			CHECK_ERR(err);
			cnt--;
		}
		break;

	case V4L2_IMAGE_STABILIZER_DUALIS:
		/*break;*/

	default:
		val = V4L2_IMAGE_STABILIZER_OFF;
		goto retry;
		break;
	}

	if (old_mode == M9MO_MONITOR_MODE) {
		err = m9mo_set_mode(sd, old_mode);
		CHECK_ERR(err);

		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_MODE)) {
			cam_err("M9MO_INT_MODE isn't issued, %#x\n",
				int_factor);
			return -ETIMEDOUT;
		}
		CHECK_ERR(err);
	}

	state->image_stabilizer_mode = val;

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_capture_ctrl(struct v4l2_subdev *sd, int val)
{
	int err, end_check = 0;
	struct m9mo_state *state = to_state(sd);
	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_STILL_CAP_NORMAL:
		cam_trace("~ FACTORY_STILL_CAP_NORMAL ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x53, 0x07);
		CHECK_ERR(err);
		break;

	case FACTORY_STILL_CAP_DUALCAP:
		cam_trace("~ FACTORY_STILL_CAP_DUALCAP ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
			M9MO_CAPCTRL_CAP_MODE, 0x05);
		CHECK_ERR(err);
		break;

	case FACTORY_DUAL_CAP_ON:
		cam_trace("~ FACTORY_DUAL_CAP_ON ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
			M9MO_CAPCTRL_START_DUALCAP, 0x01);
		CHECK_ERR(err);
		state->factory_end_check = end_check;
		break;

	case FACTORY_DUAL_CAP_OFF:
		cam_trace("~ FACTORY_DUAL_CAP_OFF ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
			M9MO_CAPCTRL_START_DUALCAP, 0x00);
		CHECK_ERR(err);
		break;

	default:
		cam_err("~ m9mo_set_factory_capture_ctrl ~ val : %d", val);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_flash(struct v4l2_subdev *sd, int val)
{
	int err;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_FLASH_STROBE_CHECK_ON:
		cam_trace("~ FACTORY_FLASH_STROBE_CHECK_ON ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x70, 0x01);
		CHECK_ERR(err);
		break;

	case FACTORY_FLASH_STROBE_CHECK_OFF:
		cam_trace("~ FACTORY_FLASH_STROBE_CHECK_OFF ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x70, 0x00);
		CHECK_ERR(err);
		break;

	case FACTORY_FLASH_CHARGE:
		cam_trace("~ FACTORY_FLASH_CHARGE ~\n");
		break;

	case FACTORY_FLASH_LOG:
		err = m9mo_make_CSV_rawdata(sd,
				M9MO_FLASH_FACTORY_FLASH_CHECK, false);
		cam_trace("~ FACTORY_FLASH_LOG ~\n");
		break;

	default:
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_wb(struct v4l2_subdev *sd, int val)
{
	int err, end_check = 0;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_WB_INDOOR_RUN:
		cam_trace("~ FACTORY_WB_INDOOR_RUN ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x14, 0x01);
		CHECK_ERR(err);
		break;

	case FACTORY_WB_OUTDOOR_RUN:
		cam_trace("~ FACTORY_WB_OUTDOOR_RUN ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x14, 0x11);
		CHECK_ERR(err);
		break;

	case FACTORY_WB_INDOOR_END_CHECK:
	case FACTORY_WB_OUTDOOR_END_CHECK:
		cam_trace("~ FACTORY_WB_END_CHECK ~\n");
		err = m9mo_readb(sd, M9MO_CATEGORY_ADJST,
			0x40, &end_check);
		CHECK_ERR(err);
		state->factory_end_check = end_check;
		break;

	case FACTORY_WB_LOG:
		err = m9mo_make_CSV_rawdata(sd,
				M9MO_FLASH_FACTORY_WB_ADJ, false);
		cam_trace("~ FACTORY_WB_LOG ~\n");
		break;

	default:
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_factory_defectpixel(struct v4l2_subdev *sd, int val)
{
	int err;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val : %d\n", val);

	switch (val) {
	case FACTORY_DEFECTPIXEL_SCENARIO_6:
		cam_trace("~ FACTORY_DEFECTPIXEL_SCENARIO_6 ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_MON,
			0x5C, 0x06);
		CHECK_ERR(err);
		break;

	case FACTORY_DEFECTPIXEL_RUN:
		cam_trace("~ FACTORY_DEFECTPIXEL_RUN ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_MON,
			0x5C, 0x07);
		CHECK_ERR(err);
		break;

	case FACTORY_DEFECTPIXEL_END_CHECK:
		cam_trace("~ FACTORY_DEFECTPIXEL_END_CHECK ~\n");
		break;

	case FACTORY_DEFECTPIXEL_LOG:
		m9mo_make_CSV_rawdata_direct(sd,
			V4L2_CID_CAMERA_FACTORY_DEFECTPIXEL);
		cam_trace("~ FACTORY_DEFECTPIXEL_LOG ~\n");
		break;

	default:
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_aeawblock(struct v4l2_subdev *sd, int val)
{
	int err;

	cam_err("%d\n", val);
	switch (val) {
	case AE_UNLOCK_AWB_UNLOCK:
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_LOCK, 0);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB, M9MO_AWB_LOCK, 0);
		CHECK_ERR(err);
		break;

	case AE_LOCK_AWB_UNLOCK:
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_LOCK, 1);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB, M9MO_AWB_LOCK, 0);
		CHECK_ERR(err);
		break;

	case AE_UNLOCK_AWB_LOCK:
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_LOCK, 0);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB, M9MO_AWB_LOCK, 1);
		CHECK_ERR(err);
		break;

	case AE_LOCK_AWB_LOCK:
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_LOCK, 1);
		CHECK_ERR(err);
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB, M9MO_AWB_LOCK, 1);
		CHECK_ERR(err);
		break;
	}
	cam_err("X\n");
	return 0;
}

static int m9mo_set_GBAM(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err = 0;

	cam_trace("E\n");

	if (val) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
				M9MO_WB_GBAM_MODE, 0x01);
		CHECK_ERR(err);

		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
				M9MO_WB_G_VALUE, state->wb_g_value);
		CHECK_ERR(err);

		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
				M9MO_WB_B_VALUE, state->wb_b_value);
		CHECK_ERR(err);

		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
				M9MO_WB_A_VALUE, state->wb_a_value);
		CHECK_ERR(err);

		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
				M9MO_WB_M_VALUE, state->wb_m_value);
		CHECK_ERR(err);
	} else {
		err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
				M9MO_WB_GBAM_MODE, 0x00);
		CHECK_ERR(err);
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_K(struct v4l2_subdev *sd, int val)
{
	int err = 0;

	cam_trace("E %02X\n", val);

	err = m9mo_writeb(sd, M9MO_CATEGORY_WB,
		M9MO_WB_K_VALUE, val);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_dual_capture_mode(struct v4l2_subdev *sd, int val)
{
	int err = 0;
	struct m9mo_state *state = to_state(sd);

	cam_trace("E val = %d\n", val);

	if (val == state->vss_mode) {
		cam_err("same vss_mode\n");
		return err;
	}

	switch (val) {
	case 0:
		err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
				M9MO_PARM_VSS_MODE, 0x00);
		CHECK_ERR(err);
		break;
	case 1:
		/* 4M Video Snap Shot */
		err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
				M9MO_PARM_VSS_MODE, 0x01);
		CHECK_ERR(err);
		break;
	default:
		val = 0;
		break;
	}

	state->vss_mode = val;
	cam_trace("X\n");
	return err;
}

static int m9mo_start_set_dual_capture(struct v4l2_subdev *sd, int val)
{
	int err, int_factor;

	cam_trace("E\n");

	/* Start video snap shot */
	err = m9mo_writeb(sd, M9MO_CATEGORY_MON,
		M9MO_MON_START_VIDEO_SNAP_SHOT, 0x01);
	CHECK_ERR(err);

	/* Clear Interrupt factor */
	int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
	if (!(int_factor & M9MO_INT_FRAME_SYNC)) {
		cam_err("M9MO_INT_FRAME_SYNC isn't issued, %#x\n", int_factor);
		return -ETIMEDOUT;
	}

	cam_trace("X\n");
	return err;
}

static int m9mo_start_dual_postview(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err, int_factor;
	cam_trace("E\n");

	/* Change to Parameter Mode */
	err = m9mo_set_mode(sd, M9MO_PARMSET_MODE);
	CHECK_ERR(err);

	/* Select image number of frame Preview image */
	err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
		M9MO_PARM_SEL_FRAME_VIDEO_SNAP, val);
	CHECK_ERR(err);

	/* Select main image format */
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
		M9MO_CAPPARM_YUVOUT_PREVIEW, 0x00);
	CHECK_ERR(err);

	/* Select preview image size */
	if (FRM_RATIO(state->preview) == CAM_FRMRATIO_VGA) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
		M9MO_CAPPARM_PREVIEW_IMG_SIZE, 0x08);
		CHECK_ERR(err);
	} else if (FRM_RATIO(state->preview) == CAM_FRMRATIO_HD) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
		M9MO_CAPPARM_PREVIEW_IMG_SIZE, 0x0F);
		CHECK_ERR(err);
	}

	/* Get Video Snap Shot data */
	err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
		M9MO_PARM_VIDEO_SNAP_IMG_TRANSFER_START, 0x02);
	CHECK_ERR(err);

	/* Clear Interrupt factor */
	int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
	if (!(int_factor & M9MO_INT_FRAME_SYNC)) {
		cam_err("M9MO_INT_FRAME_SYNC isn't issued, %#x\n", int_factor);
		return -ETIMEDOUT;
	}

	cam_trace("X\n");
	return err;
}

static int m9mo_start_dual_capture(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err, int_factor;
	cam_trace("E\n");

	/* Select main image size */
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
		M9MO_CAPPARM_MAIN_IMG_SIZE, 0x1E);
	CHECK_ERR(err);

	/* Select main image format */
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
		M9MO_CAPPARM_YUVOUT_MAIN, 0x01);
	CHECK_ERR(err);

	/* Select image number of frame For Video Snap Shot image */
	err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
		M9MO_PARM_SEL_FRAME_VIDEO_SNAP, val);
	CHECK_ERR(err);

	/* Get Video Snap Shot data */
	err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
		M9MO_PARM_VIDEO_SNAP_IMG_TRANSFER_START, 0x01);
	CHECK_ERR(err);

	/* Clear Interrupt factor */
	int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
	if (!(int_factor & M9MO_INT_FRAME_SYNC)) {
		cam_err("M9MO_INT_FRAME_SYNC isn't issued, %#x\n", int_factor);
		return -ETIMEDOUT;
	}

	/* Get main image JPEG size */
	err = m9mo_readl(sd, M9MO_CATEGORY_CAPCTRL,
			M9MO_CAPCTRL_IMG_SIZE, &state->jpeg.main_size);
	CHECK_ERR(err);
	cam_trace("~~~~~~ main_size : 0x%x ~~~~~~\n", state->jpeg.main_size);
#if 1
	state->jpeg.main_offset = 0;
	state->jpeg.thumb_offset = M9MO_JPEG_MAXSIZE;
	state->jpeg.postview_offset = M9MO_JPEG_MAXSIZE + M9MO_THUMB_MAXSIZE;

	/* Read Exif information */
	m9mo_get_exif(sd);
#endif

	/* Changes to Video Snap Shot Normal mode */
	err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
		M9MO_PARM_VSS_MODE, 0x00);
	CHECK_ERR(err);

	err = m9mo_set_mode(sd, M9MO_MONITOR_MODE);
	CHECK_ERR(err);

	int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
	if (!(int_factor & M9MO_INT_MODE)) {
		cam_err("M9MO_INT_MODE isn't issued, %#x\n",
			int_factor);
		return -ETIMEDOUT;
	}
	CHECK_ERR(err);

	cam_trace("X\n");
	return err;
}

static int m9mo_set_PASM_mode(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err = 0;

	cam_dbg("E, value %d\n", val);

	switch (val) {
	case MODE_SMART_AUTO:
		/* Set Still Mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW, 0x08, 0x01);
		CHECK_ERR(err);

		/* Set HISTOGRAM ON */
		err = m9mo_writeb(sd, M9MO_CATEGORY_MON, 0x58, 0x01);
		CHECK_ERR(err);

		/* SMART AUTO CAP */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_MODE_CAP, 0x10);
		CHECK_ERR(err);

		break;

	case MODE_PROGRAM:
		/* Set Monitor EV program mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EP_MODE_MON, state->EV);
		CHECK_ERR(err);

		/* Set Still Capture EV program mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EP_MODE_CAP, state->EV);
		CHECK_ERR(err);

		/* Still Capture EVP Set Parameter Mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_MODE_CAP, 0x00);
		CHECK_ERR(err);
#if 0
		/* Monitor EVP Set Parameter Mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_MODE_MON, 0x00);
		CHECK_ERR(err);
		break;
#endif
	case MODE_A:
		/* Set Monitor EV program mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EP_MODE_MON, state->EV);
		CHECK_ERR(err);

		/* Set Still Capture EV program mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EP_MODE_CAP, state->EV);
		CHECK_ERR(err);

		/* Still Capture EVP Set Parameter Mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_MODE_CAP, 0x01);
		CHECK_ERR(err);
#if 0
		/* Monitor EVP Set Parameter Mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_MODE_MON, 0x01);
		CHECK_ERR(err);
#endif
		/* Set Still Capture F-Number Value */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_F_NUMBER, state->f_number);
		CHECK_ERR(err);

		/* Set Monitor F-Number Value */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_F_NUMBER_MON, state->f_number);
		CHECK_ERR(err);
		break;

	case MODE_S:
		/* Set Monitor EV program mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EP_MODE_MON, state->EV);
		CHECK_ERR(err);

		/* Set Still Capture EV program mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EP_MODE_CAP, state->EV);
		CHECK_ERR(err);

		/* Still Capture EVP Set Parameter Mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_MODE_CAP, 0x02);
		CHECK_ERR(err);
#if 0
		/* Monitor EVP Set Parameter Mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_MODE_MON, 0x02);
		CHECK_ERR(err);
#endif
		/* Set Capture Shutter Speed Time */
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_SS_NUMERATOR, state->numerator);
		CHECK_ERR(err);
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_SS_DENOMINATOR, state->denominator);
		CHECK_ERR(err);

		/* Set Monitor Shutter Speed Time */
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_SS_NUMERATOR_MON, state->numerator);
		CHECK_ERR(err);
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_SS_DENOMINATOR_MON, state->denominator);
		CHECK_ERR(err);
		break;

	case MODE_M:
		/* Still Capture EVP Set Parameter Mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_MODE_CAP, 0x07);
		CHECK_ERR(err);
#if 0
		/* Monitor EVP Set Parameter Mode */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_MODE_MON, 0x07);
		CHECK_ERR(err);
#endif
		/* Set Still Capture F-Number Value */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_F_NUMBER, state->f_number);
		CHECK_ERR(err);

		/* Set Monitor F-Number Value */
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_F_NUMBER_MON, state->f_number);
		CHECK_ERR(err);

		/* Set Capture Shutter Speed Time */
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_SS_NUMERATOR, state->numerator);
		CHECK_ERR(err);
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_SS_DENOMINATOR, state->denominator);
		CHECK_ERR(err);

		/* Set Monitor Shutter Speed Time */
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_SS_NUMERATOR_MON, state->numerator);
		CHECK_ERR(err);
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_SS_DENOMINATOR_MON, state->denominator);
		CHECK_ERR(err);

		/* Set Still Capture ISO Value */
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_ISO_VALUE, state->iso);
		CHECK_ERR(err);

		/* Set Monitor ISO Value */
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			M9MO_AE_EV_PRG_ISO_VALUE_MON, state->iso);
		CHECK_ERR(err);
		break;

	default:
		break;
	}

	cam_trace("X\n");

	return 0;
}

static int m9mo_set_shutter_speed(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err = 0;
	int numerator, denominator;
	cam_dbg("E, value %d\n", val);

	switch (val) {
	case 1:
		numerator = 16;
		denominator = 1;
		break;

	case 2:
		numerator = 12;
		denominator = 1;
		break;

	case 3:
		numerator = 8;
		denominator = 1;
		break;

	case 4:
		numerator = 6;
		denominator = 1;
		break;

	case 5:
		numerator = 4;
		denominator = 1;
		break;

	case 6:
		numerator = 3;
		denominator = 1;
		break;

	case 7:
		numerator = 2;
		denominator = 1;
		break;

	case 8:
		numerator = 15;
		denominator = 10;
		break;

	case 9:
		numerator = 1;
		denominator = 1;
		break;
	/* 0.7 */
	case 10:
		numerator = 7;
		denominator = 10;
		break;

	case 11:
		numerator = 5;
		denominator = 10;
		break;

	case 12:
		numerator = 1;
		denominator = 3;
		break;

	case 13:
		numerator = 1;
		denominator = 4;
		break;

	case 14:
		numerator = 1;
		denominator = 6;
		break;

	case 15:
		numerator = 1;
		denominator = 8;
		break;

	case 16:
		numerator = 1;
		denominator = 15;
		break;

	case 17:
		numerator = 1;
		denominator = 20;
		break;

	case 18:
		numerator = 1;
		denominator = 30;
		break;

	case 19:
		numerator = 1;
		denominator = 45;
		break;

	case 20:
		numerator = 1;
		denominator = 60;
		break;

	case 21:
		numerator = 1;
		denominator = 90;
		break;

	case 22:
		numerator = 1;
		denominator = 125;
		break;

	case 23:
		numerator = 1;
		denominator = 180;
		break;

	case 24:
		numerator = 1;
		denominator = 250;
		break;

	case 25:
		numerator = 1;
		denominator = 350;
		break;

	case 26:
		numerator = 1;
		denominator = 500;
		break;

	case 27:
		numerator = 1;
		denominator = 750;
		break;

	case 28:
		numerator = 1;
		denominator = 1000;
		break;

	case 29:
		numerator = 1;
		denominator = 1500;
		break;

	case 30:
		numerator = 1;
		denominator = 2000;
		break;

	default:
		break;
	}

	state->numerator = numerator;
	state->denominator = denominator;

	/* Set Capture Shutter Speed Time */
	err = m9mo_writew(sd, M9MO_CATEGORY_AE,
		M9MO_AE_EV_PRG_SS_NUMERATOR, numerator);
	CHECK_ERR(err);
	err = m9mo_writew(sd, M9MO_CATEGORY_AE,
		M9MO_AE_EV_PRG_SS_DENOMINATOR, denominator);
	CHECK_ERR(err);

	/* Set Monitor Shutter Speed Time */
	err = m9mo_writew(sd, M9MO_CATEGORY_AE,
		M9MO_AE_EV_PRG_SS_NUMERATOR_MON, numerator);
	CHECK_ERR(err);
	err = m9mo_writew(sd, M9MO_CATEGORY_AE,
		M9MO_AE_EV_PRG_SS_DENOMINATOR_MON, denominator);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_f_number(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err = 0;
	int f_number;

	cam_dbg("E, value %d\n", val);

	switch (val) {
	case 28:
		f_number = 28;
		break;

	case 32:
		f_number = 32;
		break;

	case 35:
		f_number = 35;
		break;

	case 40:
		f_number = 40;
		break;

	case 45:
		f_number = 45;
		break;

	case 50:
		f_number = 50;
		break;

	case 56:
		f_number = 56;
		break;

	case 63:
		f_number = 63;
		break;

	case 71:
		f_number = 71;
		break;

	case 80:
		f_number = 80;
		break;

	case 90:
		f_number = 90;
		break;

	case 100:
		f_number = 160; /* 0xA0 */
		break;

	case 110:
		f_number = 176; /* 0xB0 */
		break;

	default:
		break;
	}

	state->f_number = f_number;

	/* Set Still Capture F-Number Value */
	err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
		M9MO_AE_EV_PRG_F_NUMBER, f_number);
	CHECK_ERR(err);

	/* Set Monitor F-Number Value */
	err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
		M9MO_AE_EV_PRG_F_NUMBER_MON, f_number);
	CHECK_ERR(err);


	cam_trace("X\n");
	return 0;
}

static int m9mo_set_wb_custom(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int err = 0;

	cam_dbg("E\n");

	err = m9mo_writel(sd, M9MO_CATEGORY_WB,
		M9MO_WB_CUSTOM_X, state->wb_custom_x);
	CHECK_ERR(err);

	err = m9mo_writel(sd, M9MO_CATEGORY_WB,
		M9MO_WB_CUSTOM_Y, state->wb_custom_y);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_set_smart_auto_s1_push(struct v4l2_subdev *sd, int val)
{
	struct m9mo_state *state = to_state(sd);
	int int_factor, err;

	cam_dbg("E\n");

	if (val == 1) {
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW, 0x09, 0x02);
		CHECK_ERR(err);

		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_CAPTURE)) {
			cam_err("M9MO_INT_CAPTURE isn't issued, %#x\n",
				int_factor);
			return -ETIMEDOUT;
		}
		CHECK_ERR(err);
	} else {
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW, 0x09, 0x03);
		CHECK_ERR(err);
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_check_dataline(struct v4l2_subdev *sd, int val)
{
	int err = 0;

	cam_dbg("E, value %d\n", val);

	err = m9mo_writeb(sd, M9MO_CATEGORY_TEST,
		M9MO_TEST_OUTPUT_YCO_TEST_DATA, val ? 0x01 : 0x00);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int m9mo_check_esd(struct v4l2_subdev *sd)
{
	s32 val = 0;
	int err = 0;

	struct m9mo_state *state = to_state(sd);

	if (state->factory_test_num != 0) {
		cam_dbg("factory test mode !!! ignore esd check\n");
		return 0;
	}

	/* check ISP */
	err = m9mo_readb(sd, M9MO_CATEGORY_TEST, M9MO_TEST_ISP_PROCESS, &val);
	CHECK_ERR(err);
	cam_dbg("progress %#x\n", val);

	if (val != 0x80) {
		goto esd_occur;
	} else {
		m9mo_wait_interrupt(sd, M9MO_ISP_ESD_TIMEOUT);

		err = m9mo_readb(sd, M9MO_CATEGORY_SYS, M9MO_SYS_ESD_INT, &val);
		CHECK_ERR(err);

		if (val & M9MO_INT_ESD)
			goto esd_occur;
	}

	cam_warn("ESD is not detected\n");
	return 0;

esd_occur:
	cam_warn("ESD shock is detected\n");
	return -EIO;
}

static int m9mo_g_ext_ctrl(struct v4l2_subdev *sd,
		struct v4l2_ext_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	int err = 0;

	switch (ctrl->id) {
	case V4L2_CID_CAM_SENSOR_FW_VER:
		strcpy(ctrl->string, state->exif.unique_id);
		break;

	default:
		cam_err("no such control id %d\n",
				ctrl->id - V4L2_CID_CAMERA_CLASS_BASE);
		/*err = -ENOIOCTLCMD*/
		/*err = 0;*/
		break;
	}

	/* FIXME
	 * if (err < 0 && err != -ENOIOCTLCMD)
	 *	cam_err("failed, id %d\n",
		ctrl->id - V4L2_CID_CAMERA_CLASS_BASE);
	 */

	return err;
}

static int m9mo_g_ext_ctrls(struct v4l2_subdev *sd,
		struct v4l2_ext_controls *ctrls)
{
	struct v4l2_ext_control *ctrl = ctrls->controls;
	int i, err = 0;

	for (i = 0; i < ctrls->count; i++, ctrl++) {
		err = m9mo_g_ext_ctrl(sd, ctrl);
		if (err) {
			ctrls->error_idx = i;
			break;
		}
	}
	return err;
}

static int m9mo_check_manufacturer_id(struct v4l2_subdev *sd)
{
	int i, err;
	u8 id;
	u32 addr[] = {0x1000AAAA, 0x10005554, 0x1000AAAA};
	u8 val[3][2] = {
		[0] = {0x00, 0xAA},
		[1] = {0x00, 0x55},
		[2] = {0x00, 0x90},
	};
	u8 reset[] = {0x00, 0xF0};

	/* set manufacturer's ID read-mode */
	for (i = 0; i < 3; i++) {
		err = m9mo_mem_write(sd, 0x06, 2, addr[i], val[i]);
		CHECK_ERR(err);
	}

	/* read manufacturer's ID */
	err = m9mo_mem_read(sd, sizeof(id), 0x10000001, &id);
	CHECK_ERR(err);

	/* reset manufacturer's ID read-mode */
	err = m9mo_mem_write(sd, 0x06, sizeof(reset), 0x10000000, reset);
	CHECK_ERR(err);

	cam_dbg("%#x\n", id);

	return id;
}

static int m9mo_program_fw(struct v4l2_subdev *sd,
	u8 *buf, u32 addr, u32 unit, u32 count)
{
	u32 val;
	int i, err = 0;
	int erase = 0x01;
	int test_count = 0;
	int retries = 0;

	for (i = 0; i < unit*count; i += unit) {
		/* Set Flash ROM memory address */
		err = m9mo_writel(sd, M9MO_CATEGORY_FLASH,
			M9MO_FLASH_ADDR, addr + i);
		CHECK_ERR(err);

		/* Erase FLASH ROM entire memory */
		err = m9mo_writeb(sd, M9MO_CATEGORY_FLASH,
			M9MO_FLASH_ERASE, erase);
		CHECK_ERR(err);
		/* Response while sector-erase is operating */
		retries = 0;
		do {
			mdelay(30);
			err = m9mo_readb(sd, M9MO_CATEGORY_FLASH,
				M9MO_FLASH_ERASE, &val);
			CHECK_ERR(err);
		} while (val == erase && retries++ < M9MO_I2C_VERIFY);

		if (val != 0) {
			cam_err("failed to erase sector\n");
			return -1;
		}

		/* Set FLASH ROM programming size */
		err = m9mo_writew(sd, M9MO_CATEGORY_FLASH,
				M9MO_FLASH_BYTE, unit);
		CHECK_ERR(err);

		err = m9mo_mem_write(sd, 0x04, unit,
				M9MO_INT_RAM_BASE_ADDR, buf + i);
			CHECK_ERR(err);
		cam_err("fw Send = %x count = %d\n", i, test_count++);

	/* Start Programming */
		err = m9mo_writeb(sd, M9MO_CATEGORY_FLASH, M9MO_FLASH_WR, 0x01);
		CHECK_ERR(err);

		/* Confirm programming has been completed */
		retries = 0;
		do {
			mdelay(30);
			err = m9mo_readb(sd, M9MO_CATEGORY_FLASH,
				M9MO_FLASH_WR, &val);
			CHECK_ERR(err);
		} while (val && retries++ < M9MO_I2C_VERIFY);

		if (val != 0) {
			cam_err("failed to program~~~~\n");
			return -1;
		}
	}
	cam_err("m9mo_program_fw out ~~~~~~~~~~~\n");
	return 0;
}

static int m9mo_load_fw_main(struct v4l2_subdev *sd)
{
	struct m9mo_state *state = to_state(sd);
	struct device *dev = sd->v4l2_dev->dev;
	const struct firmware *fw = NULL;
	u8 *buf_m9mo = NULL;
	unsigned int count = 0;
	/*int offset;*/
	int err = 0;

	struct file *fp;
	mm_segment_t old_fs;
	long fsize, nread;
	int fw_requested = 1;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(M9MO_FW_PATH, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		cam_trace("failed to open %s, err %ld\n",
			M9MO_FW_PATH, PTR_ERR(fp));
		goto request_fw;
	}

	fw_requested = 0;
	fsize = fp->f_path.dentry->d_inode->i_size;
	count = fsize / SZ_4K;

	cam_err("start, file path %s, size %ld Bytes, count %d\n",
		M9MO_FW_PATH, fsize, count);

	buf_m9mo = vmalloc(fsize);
	if (!buf_m9mo) {
		cam_err("failed to allocate memory\n");
		err = -ENOMEM;
		goto out;
	}

	nread = vfs_read(fp, (char __user *)buf_m9mo, fsize, &fp->f_pos);
	if (nread != fsize) {
		cam_err("failed to read firmware file, %ld Bytes\n", nread);
		err = -EIO;
		goto out;
	}

	filp_close(fp, current->files);

request_fw:
	if (fw_requested) {
		set_fs(old_fs);
		if (system_rev > 1) {
			cam_info("Firmware Path = %s\n",
					M9MO_EVT31_FW_REQ_PATH);
			err = request_firmware(&fw,
					M9MO_EVT31_FW_REQ_PATH, dev);
		} else {
			cam_info("Firmware Path = %s\n", M9MO_FW_REQ_PATH);
			err = request_firmware(&fw, M9MO_FW_REQ_PATH, dev);
		}


		if (err != 0) {
			cam_err("request_firmware failed\n");
			err = -EINVAL;
			goto out;
		}
		count = fw->size / SZ_4K;
		cam_err("start, size %d Bytes  count = %d\n", fw->size, count);
		buf_m9mo = (u8 *)fw->data;
	}

	err = m9mo_mem_write(sd, 0x04, SZ_64,
			0x90001200 , buf_port_seting0);
	CHECK_ERR(err);
	mdelay(10);

	err = m9mo_mem_write(sd, 0x04, SZ_64,
			0x90001000 , buf_port_seting1);
	CHECK_ERR(err);
	mdelay(10);

	err = m9mo_mem_write(sd, 0x04, SZ_64,
			0x90001100 , buf_port_seting2);
	CHECK_ERR(err);
	mdelay(10);

	err = m9mo_writel(sd, M9MO_CATEGORY_FLASH,
			0x1C, 0x0247036D);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_FLASH,
			0x4A, 0x01);
	CHECK_ERR(err);
	mdelay(10);

	/* program FLASH ROM */
	err = m9mo_program_fw(sd, buf_m9mo, M9MO_FLASH_BASE_ADDR, SZ_4K, count);
	if (err < 0)
		goto out;


#if 0
	offset = SZ_64K * 31;
	if (id == 0x01) {
		err = m9mo_program_fw(sd, buf + offset,
				M9MO_FLASH_BASE_ADDR + offset, SZ_8K, 4, id);
	} else {
		err = m9mo_program_fw(sd, buf + offset,
				M9MO_FLASH_BASE_ADDR + offset, SZ_4K, 8, id);
	}
#endif
	cam_err("end\n");
	state->isp.bad_fw = 0;

out:
	if (!fw_requested) {
		vfree(buf_m9mo);

		filp_close(fp, current->files);
		set_fs(old_fs);
	} else {
		release_firmware(fw);
	}

	return err;
}

static int m9mo_load_fw_info(struct v4l2_subdev *sd)
{
	const struct firmware *fw = NULL;
	u8 *buf_m9mo = NULL;
	/*int offset;*/
	int err = 0;

	struct file *fp;
	mm_segment_t old_fs;
	long fsize, nread;
	int fw_requested = 1;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(FW_INFO_PATH, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		cam_trace("failed to open %s, err %ld\n",
			M9MO_FW_PATH, PTR_ERR(fp));
	}
	fsize = fp->f_path.dentry->d_inode->i_size;

	cam_err("start, file path %s, size %ld Bytes\n", FW_INFO_PATH, fsize);

	buf_m9mo = vmalloc(fsize);
	if (!buf_m9mo) {
		cam_err("failed to allocate memory\n");
		err = -ENOMEM;
		goto out;
	}

	nread = vfs_read(fp, (char __user *)buf_m9mo, fsize, &fp->f_pos);
	if (nread != fsize) {
		cam_err("failed to read firmware file, %ld Bytes\n", nread);
		err = -EIO;
		goto out;
	}

	err = m9mo_mem_write(sd, 0x04, SZ_64,
				0x90001200 , buf_port_seting0);
			CHECK_ERR(err);
			mdelay(10);

	err = m9mo_mem_write(sd, 0x04, SZ_64,
				0x90001000 , buf_port_seting1);
			CHECK_ERR(err);
			mdelay(10);

	err = m9mo_mem_write(sd, 0x04, SZ_64,
				0x90001100 , buf_port_seting2);
			CHECK_ERR(err);
			mdelay(10);

	err = m9mo_writel(sd, M9MO_CATEGORY_FLASH,
				0x1C, 0x0247036D);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_FLASH,
				0x4A, 0x01);
	CHECK_ERR(err);
			mdelay(10);

	/* program FLSH ROM */
	err = m9mo_program_fw(sd, buf_m9mo, M9MO_FLASH_BASE_ADDR_1, SZ_4K, 1);
	if (err < 0)
		goto out;


#if 0
	offset = SZ_64K * 31;
	if (id == 0x01) {
		err = m9mo_program_fw(sd, buf + offset,
				M9MO_FLASH_BASE_ADDR + offset, SZ_8K, 4, id);
	} else {
		err = m9mo_program_fw(sd, buf + offset,
				M9MO_FLASH_BASE_ADDR + offset, SZ_4K, 8, id);
	}
#endif
	cam_err("end\n");

out:
	if (!fw_requested) {
		vfree(buf_m9mo);

		filp_close(fp, current->files);
		set_fs(old_fs);
	} else {
		release_firmware(fw);
	}

	return err;
}



static int m9mo_load_fw(struct v4l2_subdev *sd)
{
	struct device *dev = sd->v4l2_dev->dev;
	const struct firmware *fw = NULL;
	u8 sensor_ver[M9MO_FW_VER_LEN] = {0, };
	u8 *buf_m9mo = NULL, *buf_fw_info = NULL;
	/*int offset;*/
	int err = 0;

	struct file *fp;
	mm_segment_t old_fs;
	long fsize, nread;
	int fw_requested = 1;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(M9MO_FW_PATH, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		cam_err("failed to open %s, err %ld\n",
			M9MO_FW_PATH, PTR_ERR(fp));
		goto request_fw;
	}

	fw_requested = 0;
	fsize = fp->f_path.dentry->d_inode->i_size;

	cam_err("start, file path %s, size %ld Bytes\n", M9MO_FW_PATH, fsize);

	buf_m9mo = vmalloc(fsize);
	if (!buf_m9mo) {
		cam_err("failed to allocate memory\n");
		err = -ENOMEM;
		goto out;
	}

	nread = vfs_read(fp, (char __user *)buf_m9mo, fsize, &fp->f_pos);
	if (nread != fsize) {
		cam_err("failed to read firmware file, %ld Bytes\n", nread);
		err = -EIO;
		goto out;
	}

	filp_close(fp, current->files);

	fp = filp_open(FW_INFO_PATH, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		cam_trace("failed to open %s, err %ld\n",
			FW_INFO_PATH, PTR_ERR(fp));
		goto request_fw;
	}

	fw_requested = 0;
	fsize = fp->f_path.dentry->d_inode->i_size;

	cam_err("start, file path %s, size %ld Bytes\n", FW_INFO_PATH, fsize);

	buf_fw_info = vmalloc(fsize);
	if (!buf_fw_info) {
		cam_err("failed to allocate memory\n");
		err = -ENOMEM;
		goto out;
	}

	nread = vfs_read(fp, (char __user *)buf_fw_info, fsize, &fp->f_pos);
	if (nread != fsize) {
		cam_err("failed to read firmware file, %ld Bytes\n", nread);
		err = -EIO;
		goto out;
	}


request_fw:
	if (fw_requested) {
		set_fs(old_fs);

	m9mo_get_sensor_fw_version(sd);

	if (sensor_ver[0] == 'T' && sensor_ver[1] == 'B') {
		err = request_firmware(&fw, M9MOTB_FW_PATH, dev);
#if defined(CONFIG_MACH_Q1_BD)
	} else if (sensor_ver[0] == 'O' && sensor_ver[1] == 'O') {
		err = request_firmware(&fw, M9MOOO_FW_PATH, dev);
#endif
#if defined(CONFIG_MACH_U1_KOR_LGT)
	} else if (sensor_ver[0] == 'S' && sensor_ver[1] == 'B') {
		err = request_firmware(&fw, M9MOSB_FW_PATH, dev);
#endif
	} else {
		cam_err("cannot find the matched F/W file\n");
		err = -EINVAL;
	}

	if (err != 0) {
		cam_err("request_firmware falied\n");
			err = -EINVAL;
			goto out;
	}
		cam_dbg("start, size %d Bytes\n", fw->size);
		buf_m9mo = (u8 *)fw->data;
	}


	err = m9mo_mem_write(sd, 0x04, SZ_64,
				0x90001200 , buf_port_seting0);
			CHECK_ERR(err);
			mdelay(10);

	err = m9mo_mem_write(sd, 0x04, SZ_64,
				0x90001000 , buf_port_seting1);
			CHECK_ERR(err);
			mdelay(10);

	err = m9mo_mem_write(sd, 0x04, SZ_64,
				0x90001100 , buf_port_seting2);
			CHECK_ERR(err);
			mdelay(10);

	err = m9mo_writel(sd, M9MO_CATEGORY_FLASH,
				0x1C, 0x0247036D);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_FLASH,
				0x4A, 0x01);
	CHECK_ERR(err);
			mdelay(10);

	/* program FLSH ROM */
	err = m9mo_program_fw(sd, buf_m9mo, M9MO_FLASH_BASE_ADDR, SZ_4K, 504);
	if (err < 0)
		goto out;

	err = m9mo_program_fw(sd, buf_fw_info,
			M9MO_FLASH_BASE_ADDR_1, SZ_4K, 1);
	if (err < 0)
		goto out;

#if 0
	offset = SZ_64K * 31;
	if (id == 0x01) {
		err = m9mo_program_fw(sd, buf + offset,
				M9MO_FLASH_BASE_ADDR + offset, SZ_8K, 4, id);
	} else {
		err = m9mo_program_fw(sd, buf + offset,
				M9MO_FLASH_BASE_ADDR + offset, SZ_4K, 8, id);
	}
#endif
	cam_err("end\n");

out:
	if (!fw_requested) {
		vfree(buf_m9mo);
		vfree(buf_fw_info);

		filp_close(fp, current->files);
		set_fs(old_fs);
	} else {
		release_firmware(fw);
	}

	return err;
}


static int m9mo_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct m9mo_state *state = to_state(sd);
	int err = 0;
	s16 temp;

	cam_trace(" id %d, value %d\n",
		ctrl->id - V4L2_CID_PRIVATE_BASE, ctrl->value);

	if (unlikely(state->isp.bad_fw && ctrl->id != V4L2_CID_CAM_UPDATE_FW)) {
		cam_err("\"Unknown\" state, please update F/W");
		return -ENOSYS;
	}

	switch (ctrl->id) {
	case V4L2_CID_CAM_UPDATE_FW:
		if (ctrl->value == FW_MODE_DUMP)
			err = m9mo_dump_fw(sd);
		else
			err = m9mo_check_fw(sd);
		break;

	case V4L2_CID_CAMERA_SENSOR_MODE:
#ifdef FAST_CAPTURE
		err = m9mo_set_fast_capture(sd);
#else
		err = m9mo_set_sensor_mode(sd, ctrl->value);
#endif
		break;

	case V4L2_CID_CAMERA_FLASH_MODE:
		err = m9mo_set_flash(sd, ctrl->value, 0);
		break;

	case V4L2_CID_CAMERA_ISO:
		err = m9mo_set_iso(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_METERING:
		if (state->sensor_mode == SENSOR_CAMERA)
			err = m9mo_set_metering(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_BRIGHTNESS:
		err = m9mo_set_exposure(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_WHITE_BALANCE:
		err = m9mo_set_whitebalance(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_SCENE_MODE:
		err = m9mo_set_scene_mode(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_EFFECT:
		err = m9mo_set_effect(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_WDR:
		err = m9mo_set_wdr(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_ANTI_SHAKE:
		err = m9mo_set_antishake(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_BEAUTY_SHOT:
		err = m9mo_set_face_beauty(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FOCUS_MODE:
		err = m9mo_set_af_mode(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
		err = m9mo_set_af(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_OBJECT_POSITION_X:
		state->focus.pos_x = ctrl->value;
		#if 0
		/* FIXME - It should be fixed on F/W (touch AF offset) */
		if (state->preview != NULL) {
			if (state->exif.unique_id[0] == 'T') {
				if (state->preview->index == M9MO_PREVIEW_VGA)
					state->focus.pos_x -= 40;
				else if (state->preview->index ==
						M9MO_PREVIEW_WVGA)
					state->focus.pos_x -= 50;
			}
		}
		#endif
		break;

	case V4L2_CID_CAMERA_OBJECT_POSITION_Y:
		state->focus.pos_y = ctrl->value;
		#if 0
		/* FIXME - It should be fixed on F/W (touch AF offset) */
		if (state->preview != NULL) {
			if (state->preview->index == M9MO_PREVIEW_VGA) {
				if (state->exif.unique_id[0] == 'T')
					state->focus.pos_y -= 50;
			} else if (state->preview->index == M9MO_PREVIEW_WVGA) {
				if (state->exif.unique_id[0] == 'T')
					state->focus.pos_y -= 2;
				else
					state->focus.pos_y += 60;
			}
		}
		#endif
		break;

	case V4L2_CID_CAMERA_TOUCH_AF_START_STOP:
		err = m9mo_set_touch_auto_focus(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_ZOOM:
		err = m9mo_set_zoom(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_OPTICAL_ZOOM_CTRL:
		err = m9mo_set_zoom_ctrl(sd, ctrl->value);
		break;

	case V4L2_CID_CAM_JPEG_QUALITY:
		err = m9mo_set_jpeg_quality(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_CAPTURE:
		err = m9mo_start_capture(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_YUV_CAPTURE:
		err = m9mo_start_YUV_capture(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_POSTVIEW_CAPTURE:
		err = m9mo_start_postview_capture(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_CAPTURE_MODE:
		err = m9mo_set_capture_mode(sd, ctrl->value);
		break;

	/*case V4L2_CID_CAMERA_HDR:
		err = m9mo_set_hdr(sd, ctrl->value);
		break;*/

	case V4L2_CID_CAMERA_VT_MODE:
		state->vt_mode = ctrl->value;
		break;

	case V4L2_CID_CAMERA_CHECK_DATALINE:
		state->check_dataline = ctrl->value;
		break;

	case V4L2_CID_CAMERA_ANTI_BANDING:
		err = m9mo_set_antibanding(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_CHECK_ESD:
		err = m9mo_check_esd(sd);
		break;

	case V4L2_CID_CAMERA_AEAWB_LOCK_UNLOCK:
		err = m9mo_set_aeawblock(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACE_DETECTION:
		err = m9mo_set_facedetect(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_BRACKET:
		err = m9mo_set_bracket(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_BRACKET_AEB:
		err = m9mo_set_bracket_aeb(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_BRACKET_WBB:
		err = m9mo_set_bracket_wbb(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_IMAGE_STABILIZER:
		err = m9mo_set_image_stabilizer_mode(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_IS_OIS_MODE:
		err = m9mo_set_image_stabilizer_OIS(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FOCUS_AREA_MODE:
		err = m9mo_set_focus_area_mode(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_OBJ_TRACKING_START_STOP:
		err = m9mo_set_object_tracking(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FRAME_RATE:
		err = m9mo_set_fps(sd, ctrl->value);
		state->fps = ctrl->value;
		break;

	case V4L2_CID_CAMERA_LDC:
		err = m9mo_set_LDC(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_LSC:
		err = m9mo_set_LSC(sd, ctrl->value);
		break;

	case V4L2_CID_CAM_APERTURE_PREVIEW:
		err = m9mo_set_aperture_preview(sd, ctrl->value);
		break;

	case V4L2_CID_CAM_APERTURE_CAPTURE:
		err = m9mo_set_aperture_capture(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_OIS:
		err = m9mo_set_factory_OIS(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_OIS_SHIFT:
		err = m9mo_set_factory_OIS_shift(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_PUNT:
		err = m9mo_set_factory_punt(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_PUNT_SHORT_SCAN_DATA:
		cam_trace("FACTORY_PUNT_SHORT_SCAN_DATA : 0x%x\n", ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, (unsigned char)(ctrl->value));
		CHECK_ERR(err);

		cam_trace("~ FACTORY_PUNT_SHORT_SCAN_START ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0E, 0x01);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_PUNT_LONG_SCAN_DATA:
		cam_trace("FACTORY_PUNT_LONG_SCAN_DATA : 0x%x\n", ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x1B, (unsigned char)(ctrl->value));
		CHECK_ERR(err);

		cam_trace("~ FACTORY_PUNT_LONG_SCAN_START ~\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x0E, 0x02);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_ZOOM:
		err = m9mo_set_factory_zoom(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_ZOOM_STEP:
		err = m9mo_set_factory_zoom_step(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_PUNT_RANGE_DATA_MIN:
		state->f_punt_data.min = ctrl->value;
		break;

	case V4L2_CID_CAMERA_FACTORY_PUNT_RANGE_DATA_MAX:
		state->f_punt_data.max = ctrl->value;
		break;

	case V4L2_CID_CAMERA_FACTORY_PUNT_RANGE_DATA_NUM:
		state->f_punt_data.num = ctrl->value;
		break;

	case V4L2_CID_CAMERA_FACTORY_FAIL_STOP:
		err = m9mo_set_factory_fail_stop(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_NODEFOCUS:
		err = m9mo_set_factory_nodefocus(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_INTERPOLATION:
		err = m9mo_set_factory_interpolation(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_COMMON:
		err = m9mo_set_factory_common(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_VIB:
		err = m9mo_set_factory_vib(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_GYRO:
		err = m9mo_set_factory_gyro(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_BACKLASH:
		err = m9mo_set_factory_backlash(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_BACKLASH_COUNT:
		err = m9mo_set_factory_backlash_count(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_BACKLASH_MAXTHRESHOLD:
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x1A, ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_ZOOM_RANGE_CHECK_DATA_MIN:
		state->f_zoom_data.range_min = ctrl->value;
		break;

	case V4L2_CID_CAMERA_FACTORY_ZOOM_RANGE_CHECK_DATA_MAX:
		state->f_zoom_data.range_max = ctrl->value;
		break;

	case V4L2_CID_CAMERA_FACTORY_ZOOM_SLOPE_CHECK_DATA_MIN:
		state->f_zoom_data.slope_min = ctrl->value;
		break;

	case V4L2_CID_CAMERA_FACTORY_ZOOM_SLOPE_CHECK_DATA_MAX:
		state->f_zoom_data.slope_max = ctrl->value;
		break;

	case V4L2_CID_CAMERA_FACTORY_AF:
		err = m9mo_set_factory_af(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_AF_STEP_SET:
		err = m9mo_set_factory_af_step(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_AF_POSITION:
		err = m9mo_set_factory_af_position(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_DEFOCUS:
		err = m9mo_set_factory_defocus(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_DEFOCUS_WIDE:
		err = m9mo_set_factory_defocus_wide(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_DEFOCUS_TELE:
		err = m9mo_set_factory_defocus_tele(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_RESOL_CAP:
		err = m9mo_set_factory_resol_cap(sd, ctrl->value);
		break;
	case V4L2_CID_CAMERA_SET_G_VALUE:
		state->wb_g_value = ctrl->value;
		break;

	case V4L2_CID_CAMERA_SET_B_VALUE:
		state->wb_b_value = ctrl->value;
		break;

	case V4L2_CID_CAMERA_SET_A_VALUE:
		state->wb_a_value = ctrl->value;
		break;

	case V4L2_CID_CAMERA_SET_M_VALUE:
		state->wb_m_value = ctrl->value;
		break;

	case V4L2_CID_CAMERA_SET_GBAM:
		err = m9mo_set_GBAM(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_SET_K_VALUE:
		err = m9mo_set_K(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_SET_FILLIN_STEP:
		err = m9mo_set_FillIn_step(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_OIS_RANGE_DATA_X_MIN:
		cam_trace("==========Range X min Data : 0x%x, %d\n",
			(short)ctrl->value, (short)ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_NEW,
			0x21, (short)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_VIB_RANGE_DATA_X_MIN:
	case V4L2_CID_CAMERA_FACTORY_GYRO_RANGE_DATA_X_MIN:
		cam_trace("==========Range X min Data : 0x%x, %d\n",
			(u16)ctrl->value, (u16)ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_NEW,
			0x21, (short)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_OIS_RANGE_DATA_X_MAX:
		cam_trace("==========Range X max Data : 0x%x, %d\n",
			(short)ctrl->value, (short)ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_NEW,
			0x23, (short)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_VIB_RANGE_DATA_X_MAX:
	case V4L2_CID_CAMERA_FACTORY_GYRO_RANGE_DATA_X_MAX:
		cam_trace("==========Range X max Data : 0x%x, %d\n",
			(u16)ctrl->value, (u16)ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_NEW,
			0x23, (u16)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_OIS_RANGE_DATA_Y_MIN:
		err = m9mo_writew(sd, M9MO_CATEGORY_NEW,
			0x25, (short)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_VIB_RANGE_DATA_Y_MIN:
	case V4L2_CID_CAMERA_FACTORY_GYRO_RANGE_DATA_Y_MIN:
		err = m9mo_writew(sd, M9MO_CATEGORY_NEW,
			0x25, (u16)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_OIS_RANGE_DATA_Y_MAX:
		err = m9mo_writew(sd, M9MO_CATEGORY_NEW,
			0x27, (short)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_VIB_RANGE_DATA_Y_MAX:
	case V4L2_CID_CAMERA_FACTORY_GYRO_RANGE_DATA_Y_MAX:
		err = m9mo_writew(sd, M9MO_CATEGORY_NEW,
			0x27, (u16)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_OIS_RANGE_DATA_X_GAIN:
		err = m9mo_writew(sd, M9MO_CATEGORY_NEW,
			0x29, (short)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_VIB_RANGE_DATA_PEAK_X:
		err = m9mo_writew(sd, M9MO_CATEGORY_NEW,
			0x29, (u16)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_OIS_RANGE_DATA_PEAK_X:
		err = m9mo_writew(sd, M9MO_CATEGORY_NEW,
			0x2B, (short)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_VIB_RANGE_DATA_PEAK_Y:
		err = m9mo_writew(sd, M9MO_CATEGORY_NEW,
			0x2B, (u16)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_OIS_RANGE_DATA_PEAK_Y:
		err = m9mo_writew(sd, M9MO_CATEGORY_NEW,
			0x2D, (u16)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_TEST_NUMBER:
		cam_trace("==========FACTORY_TEST_NUMBER : 0x%x\n",
			ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x41, ctrl->value);
		state->factory_test_num = ctrl->value;
		CHECK_ERR(err);
		break;
/*
	case V4L2_CID_CAMERA_MODE:
		err = m9mo_set_camera_mode(sd, ctrl->value);
		break;
*/
	case V4L2_CID_CAMERA_DUAL_POSTVIEW:
		err = m9mo_start_dual_postview(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_DUAL_CAPTURE:
		err = m9mo_start_dual_capture(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_SET_DUAL_CAPTURE:
		err = m9mo_start_set_dual_capture(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_DUAL_CAPTURE_MODE:
		err = m9mo_set_dual_capture_mode(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_AF_SCAN_LIMIT_MIN:
	case V4L2_CID_CAMERA_FACTORY_AF_SCAN_RANGE_MIN:
		temp = (short)((ctrl->value) & 0x0000FFFF);
		cam_trace("==========Range    min Data : 0x%x, %d\n",
			temp, temp);
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x18, temp);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_AF_SCAN_LIMIT_MAX:
	case V4L2_CID_CAMERA_FACTORY_AF_SCAN_RANGE_MAX:
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x1A, (u16)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_AF_LENS:
		err = m9mo_set_factory_af_lens(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_AF_ZONE:
		err = m9mo_set_factory_af_zone(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_LV_TARGET:
		cam_trace("FACTORY_LV_TARGET : 0x%x\n", ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x52, ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_ADJ_IRIS:
		err = m9mo_set_factory_adj_iris(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_ADJ_IRIS_RANGE_MIN:
		cam_trace("FACTORY_ADJ_IRIS_RANGE_MIN : 0x%x\n", ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x54, (unsigned char)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_ADJ_IRIS_RANGE_MAX:
		cam_trace("FACTORY_ADJ_IRIS_RANGE_MAX : 0x%x\n", ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x55, (unsigned char)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_ADJ_GAIN_LIVEVIEW:
		err = m9mo_set_factory_adj_gain_liveview(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_ADJ_GAIN_LIVEVIEW_RANGE_MIN:
		cam_trace("FACTORY_ADJ_GAIN_LIVEVIEW_RANGE_MIN : 0x%x\n",
				ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_ADJST,
			0x56, (u16)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_ADJ_GAIN_LIVEVIEW_RANGE_MAX:
		cam_trace("FACTORY_ADJ_GAIN_LIVEVIEW_RANGE_MAX : 0x%x\n",
				ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_ADJST,
			0x58, (u16)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_SH_CLOSE:
		err = m9mo_set_factory_sh_close(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_SH_CLOSE_IRIS_NUM:
		cam_trace("FACTORY_SH_CLOSE_IRIS_NUM : 0x%x\n", ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x51, ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_SH_CLOSE_SET_IRIS:
		cam_trace("FACTORY_SH_CLOSE_SET_IRIS : 0x%x\n", ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x6E, ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_SH_CLOSE_RANGE:
		cam_trace("FACTORY_SH_CLOSE_RANGE : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_ADJST,
			0x5A, (u16)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_SH_CLOSE_ISO:
		cam_trace("FACTORY_SH_CLOSE_ISO : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			0x3B, ctrl->value);
		CHECK_ERR(err);
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			0x42, ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_SH_CLOSE_SPEEDTIME_X:
		cam_trace("FACTORY_SH_CLOSE_SPEEDTIME_X : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			0x37, (u16)(ctrl->value));
		CHECK_ERR(err);
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			0x3E, (u16)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_SH_CLOSE_SPEEDTIME_Y:
		cam_trace("FACTORY_SH_CLOSE_SPEEDTIME_Y : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			0x39, (u16)(ctrl->value));
		CHECK_ERR(err);
		err = m9mo_writew(sd, M9MO_CATEGORY_AE,
			0x40, (u16)(ctrl->value));
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_FLICKER:
		err = m9mo_set_factory_flicker(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_CAPTURE_GAIN:
		err = m9mo_set_factory_capture_gain(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_CAPTURE_GAIN_RANGE_MIN:
		cam_trace("FACTORY_CAPTURE_GAIN_RANGE_MIN : 0x%x\n",
				ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_ADJST,
			0x56, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_CAPTURE_GAIN_RANGE_MAX:
		cam_trace("FACTORY_CAPTURE_GAIN_RANGE_MAX : 0x%x\n",
				ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_ADJST,
			0x58, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_LSC_TABLE:
		cam_trace("FACTORY_LSC_TABLE : 0x%x\n", ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x30, ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_LSC_REFERENCE:
		cam_trace("FACTORY_LSC_REFERENCE : 0x%x\n", ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x31, ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_CAPTURE_CTRL:
		err = m9mo_set_factory_capture_ctrl(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_AE_TARGET:
		cam_trace("V4L2_CID_CAMERA_FACTORY_AE_TARGET : 0x%x\n",
				ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
			0x02, (unsigned char)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_FLASH:
		err = m9mo_set_factory_flash(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_FLASH_RANGE_X:
		cam_trace("FLASH_RANGE_X : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_ADJST,
			0x71, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_FLASH_RANGE_Y:
		cam_trace("FLASH_RANGE_Y : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_ADJST,
			0x73, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_WB:
		err = m9mo_set_factory_wb(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_WB_IN_RG_VALUE:
		cam_trace("WB_IN_RG_VALUE : 0x%x\n",
				ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_ADJST,
			0x27, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_WB_IN_BG_VALUE:
		cam_trace("WB_IN_BG_VALUE : 0x%x\n",
				ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_ADJST,
			0x29, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_WB_OUT_RG_VALUE:
		cam_trace("WB_OUT_RG_VALUE : 0x%x\n",
				ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_ADJST,
			0x2B, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_WB_OUT_BG_VALUE:
		cam_trace("WB_OUT_RG_VALUE : 0x%x\n",
				ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_ADJST,
			0x2D, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_WB_RANGE_FLASH_WRITE:
		err = m9mo_writeb(sd, M9MO_CATEGORY_ADJST,
			0x26, 0x01);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_AFLED_RANGE_DATA_START_X:
		cam_trace("AFLED_RANGE_DATA_START_X : 0x%x\n",
				ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x43, (unsigned char)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_AFLED_RANGE_DATA_END_X:
		cam_trace("AFLED_RANGE_DATA_END_X : 0x%x\n",
				ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x44, (unsigned char)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_AFLED_RANGE_DATA_START_Y:
		cam_trace("AFLED_RANGE_DATA_START_Y : 0x%x\n",
				ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x45, (unsigned char)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_AFLED_RANGE_DATA_END_Y:
		cam_trace("AFLED_RANGE_DATA_END_Y : 0x%x\n",
				ctrl->value);
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x46, (unsigned char)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_AF_LED_TIME:
		cam_trace("AF_LED_TIME : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x4B, ctrl->value);
		CHECK_ERR(err);

		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x4D, 0x01);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_AF_DIFF_CHECK_MIN:
		cam_trace("AF_DIFF_CHECK_MIN : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x18, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_AF_DIFF_CHECK_MAX:
		cam_trace("AF_DIFF_CHECK_MAX : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x1A, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_DEFECTPIXEL:
		err = m9mo_set_factory_defectpixel(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACTORY_DFPX_NLV_CAP:
		cam_trace("DFPX_NLV_CAP : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_PARM,
			0x70, (u16) ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_DFPX_NLV_DR0:
		cam_trace("DFPX_NLV_DR0 : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_PARM,
			0x76, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_DFPX_NLV_DR1:
		cam_trace("DFPX_NLV_D1 : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_PARM,
			0x72, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_DFPX_NLV_DR2:
		cam_trace("DFPX_NLV_DR2 : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_PARM,
			0x78, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_DFPX_NLV_DR_HS:
		cam_trace("DFPX_NLV_DR_HS : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_PARM,
			0x74, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_AF_LED_LV_MIN:
		cam_trace("AF_LED_LV_MIN : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x47, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_AF_LED_LV_MAX:
		cam_trace("AF_LED_LV_MIN : 0x%x\n", ctrl->value);
		err = m9mo_writew(sd, M9MO_CATEGORY_LENS,
			0x49, (u16)ctrl->value);
		CHECK_ERR(err);
		break;

	case V4L2_CID_CAMERA_FACTORY_CAM_SYS_MODE:
		err = m9mo_set_factory_cam_sys_mode(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_PASM_MODE:
		err = m9mo_set_PASM_mode(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_SHUTTER_SPEED:
		err = m9mo_set_shutter_speed(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_F_NUMBER:
		err = m9mo_set_f_number(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_WB_CUSTOM_X:
		state->wb_custom_x = ctrl->value;
		break;

	case V4L2_CID_CAMERA_WB_CUSTOM_Y:
		state->wb_custom_y = ctrl->value;
		break;

	case V4L2_CID_CAMERA_WB_CUSTOM_VALUE:
		err = m9mo_set_wb_custom(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_SMART_SCENE_DETECT:
		if (ctrl->value == 1) {
			state->smart_scene_detect_mode = 1;
			err = m9mo_writeb(sd, M9MO_CATEGORY_NEW, 0x0A, 0x01);
			CHECK_ERR(err);
		} else {
			state->smart_scene_detect_mode = 0;
		}
		break;

	case V4L2_CID_CAMERA_SMART_MOVIE_RECORDING:
		if (state->smart_scene_detect_mode == 1) {
			if (ctrl->value == 1) {
				err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
					0x0A, 0x02);
				CHECK_ERR(err);
			} else {
				err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
					0x0A, 0x01);
				CHECK_ERR(err);
			}
		}
		break;

	case V4L2_CID_CAMERA_SMART_AUTO_S1_PUSH:
		err = m9mo_set_smart_auto_s1_push(sd, ctrl->value);
		break;

	default:
		cam_err("no such control id %d, value %d\n",
				ctrl->id - V4L2_CID_PRIVATE_BASE, ctrl->value);
		/*err = -ENOIOCTLCMD;*/
		err = 0;
		break;
	}

	if (err < 0 && err != -ENOIOCTLCMD)
		cam_err("failed, id %d, value %d\n",
				ctrl->id - V4L2_CID_PRIVATE_BASE, ctrl->value);
	return err;
}


/*
 * v4l2_subdev_video_ops
 */
static const struct m9mo_frmsizeenum *m9mo_get_frmsize
	(const struct m9mo_frmsizeenum *frmsizes, int num_entries, int index)
{
	int i;

	for (i = 0; i < num_entries; i++) {
		if (frmsizes[i].index == index)
			return &frmsizes[i];
	}

	return NULL;
}

static int m9mo_set_frmsize(struct v4l2_subdev *sd)
{
	struct m9mo_state *state = to_state(sd);
	struct v4l2_control ctrl;
	int err;
	cam_trace("E\n");

	if (state->format_mode == V4L2_PIX_FMT_MODE_PREVIEW) {
		err = m9mo_set_mode(sd, M9MO_PARMSET_MODE);
		CHECK_ERR(err);

		if (state->fps == 60) {
			if (state->preview->height == 480) {
				err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
					M9MO_PARM_MON_SIZE, 0x2F);
				CHECK_ERR(err);
			} else if (state->preview->height == 720) {
				err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
					M9MO_PARM_MON_SIZE, 0x25);
				CHECK_ERR(err);
			}
		} else {
			err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
				M9MO_PARM_MON_SIZE, state->preview->reg_val);
			CHECK_ERR(err);
		}

		if (state->zoom) {
			/* Zoom position returns to 1
			   when the monitor size is changed. */
			ctrl.id = V4L2_CID_CAMERA_ZOOM;
			ctrl.value = state->zoom;
			m9mo_set_zoom(sd, &ctrl);
		}

		cam_err("preview frame size %dx%d\n",
			state->preview->width, state->preview->height);
	} else {
		if (state->pixelformat == V4L2_COLORSPACE_JPEG) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
					M9MO_CAPPARM_MAIN_IMG_SIZE,
					state->capture->reg_val);
			CHECK_ERR(err);
			cam_info("capture frame size %dx%d\n",
			state->capture->width, state->capture->height);
		} else {
			err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
					M9MO_CAPPARM_PREVIEW_IMG_SIZE,
					state->capture->reg_val);
			CHECK_ERR(err);
			cam_info("capture frame size %dx%d\n",
					state->capture->width,
					state->capture->height);
		}
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_s_fmt(struct v4l2_subdev *sd, struct v4l2_mbus_framefmt *ffmt)
{
	struct m9mo_state *state = to_state(sd);
	const struct m9mo_frmsizeenum **frmsize;

	u32 width = ffmt->width;
	u32 height = ffmt->height;
	u32 old_index;
	int i, num_entries;
	cam_trace("E\n");

	if (unlikely(state->isp.bad_fw)) {
		cam_err("\"Unknown\" state, please update F/W");
		return -ENOSYS;
	}

	state->format_mode = ffmt->field;
	state->pixelformat = ffmt->colorspace;

	frmsize = state->format_mode == V4L2_PIX_FMT_MODE_PREVIEW ?
		&state->preview : &state->capture;

	old_index = *frmsize ? (*frmsize)->index : -1;
	*frmsize = NULL;

	if (state->format_mode == V4L2_PIX_FMT_MODE_PREVIEW) {
		if (state->vss_mode) {
			num_entries = ARRAY_SIZE(dual_preview_frmsizes);
			for (i = 0; i < num_entries; i++) {
				if (width == dual_preview_frmsizes[i].width &&
				height == dual_preview_frmsizes[i].height) {
					*frmsize = &dual_preview_frmsizes[i];
					break;
				}
			}
		} else {
			num_entries = ARRAY_SIZE(preview_frmsizes);
			for (i = 0; i < num_entries; i++) {
				if (width == preview_frmsizes[i].width &&
					height == preview_frmsizes[i].height) {
					*frmsize = &preview_frmsizes[i];
					break;
				}
			}
		}
	} else {
		num_entries = ARRAY_SIZE(capture_frmsizes);
		for (i = 0; i < num_entries; i++) {
			if (width == capture_frmsizes[i].width &&
				height == capture_frmsizes[i].height) {
				*frmsize = &capture_frmsizes[i];
				break;
			}
		}
	}

	if (*frmsize == NULL) {
		cam_warn("invalid frame size %dx%d\n", width, height);
		*frmsize = state->format_mode == V4L2_PIX_FMT_MODE_PREVIEW ?
			state->vss_mode ?
			m9mo_get_frmsize(dual_preview_frmsizes, num_entries,
				M9MO_PREVIEW_DUAL_VGA) :
			m9mo_get_frmsize(preview_frmsizes, num_entries,
				M9MO_PREVIEW_VGA) :
			m9mo_get_frmsize(capture_frmsizes, num_entries,
				M9MO_CAPTURE_3MP);
	}

	cam_err("%dx%d\n", (*frmsize)->width, (*frmsize)->height);
	m9mo_set_frmsize(sd);

	cam_trace("X\n");
	return 0;
}

static int m9mo_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *a)
{
	struct m9mo_state *state = to_state(sd);

	a->parm.capture.timeperframe.numerator = 1;
	a->parm.capture.timeperframe.denominator = state->fps;

	return 0;
}

static int m9mo_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *a)
{
	struct m9mo_state *state = to_state(sd);
	/*int err;*/

	u32 fps = a->parm.capture.timeperframe.denominator /
					a->parm.capture.timeperframe.numerator;

	if (unlikely(state->isp.bad_fw)) {
		cam_err("\"Unknown\" state, please update F/W");
		return -ENOSYS;
	}

	if (fps != state->fps) {
		if (fps <= 0 || fps > 120) {
			cam_err("invalid frame rate %d\n", fps);
			fps = 0; /* set to auto(default) */
		}
	}

	cam_info("%s: X, fps = %d\n", __func__, fps);
	return 0;
}

static int m9mo_enum_framesizes(struct v4l2_subdev *sd,
	struct v4l2_frmsizeenum *fsize)
{
	struct m9mo_state *state = to_state(sd);

	/*
	* The camera interface should read this value, this is the resolution
	* at which the sensor would provide framedata to the camera i/f
	* In case of image capture,
	* this returns the default camera resolution (VGA)
	*/
	if (state->format_mode == V4L2_PIX_FMT_MODE_PREVIEW) {
		if (state->preview == NULL
				/* FIXME || state->preview->index < 0 */)
			return -EINVAL;

		fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
		fsize->discrete.width = state->preview->width;
		fsize->discrete.height = state->preview->height;
	} else {
		if (state->capture == NULL
				/* FIXME || state->capture->index < 0 */)
			return -EINVAL;

		fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
		fsize->discrete.width = state->capture->width;
		fsize->discrete.height = state->capture->height;
	}

	return 0;
}

static int m9mo_s_stream_preview(struct v4l2_subdev *sd, int enable)
{
	struct m9mo_state *state = to_state(sd);
	u32 old_mode, int_factor, value;
	int err;

	if (enable) {
		if (state->vt_mode) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
					M9MO_AE_EP_MODE_MON, 0x11);
			CHECK_ERR(err);
		}

#if 0
		if (state->preview->width == 720 &&
				state->preview->height == 480) {
			err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
					M9MO_AE_EP_MODE_MON, 0x1C);
			CHECK_ERR(err);
			err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
					M9MO_AE_EP_MODE_CAP, 0x1C);
			CHECK_ERR(err);
		} else {
			err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
					M9MO_AE_EP_MODE_MON, 0x00);
			CHECK_ERR(err);
			err = m9mo_writeb(sd, M9MO_CATEGORY_AE,
					M9MO_AE_EP_MODE_CAP, 0x00);
			CHECK_ERR(err);
		}
#endif

		old_mode = m9mo_set_mode(sd, M9MO_MONITOR_MODE);
		if (old_mode <= 0) {
			cam_err("failed to set mode\n");
			return old_mode;
		}

		if (old_mode != M9MO_MONITOR_MODE) {
			int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
			if (!(int_factor & M9MO_INT_MODE)) {
				cam_err("M9MO_INT_MODE isn't issued, %#x\n",
					int_factor);
				return -ETIMEDOUT;
			}
		}
		err = m9mo_writeb(sd, M9MO_CATEGORY_TEST,
			0x34, 0x00);
		err = m9mo_writeb(sd, M9MO_CATEGORY_TEST,
			0x35, 0x02);
		err = m9mo_writeb(sd, M9MO_CATEGORY_TEST,
			0x33, 0x01);
		msleep(20);
		err = m9mo_readb(sd, M9MO_CATEGORY_TEST,
			0x36, &value);
		cam_err("******** sensor version = %x *********\n", value);

		m9mo_set_lock(sd, 0);

#if 0
		cam_err("******** LENS ON *********\n");
		err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			0x04, 0x0a);
#endif

#if 0
		if (state->check_dataline) {
			err = m9mo_check_dataline(sd, state->check_dataline);
			CHECK_ERR(err);
		}
#endif
	} else {
	}

	return 0;
}

static int m9mo_s_stream_capture(struct v4l2_subdev *sd, int enable)
{
	/*u32 int_factor;*/
	int err;

#ifndef FAST_CAPTURE
	if (enable) {
		err = m9mo_set_mode(sd, M9MO_STILLCAP_MODE);
		if (err <= 0) {
			cam_err("failed to set mode\n");
			return err;
		}
/*
		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_SOUND)) {
			cam_err("M9MO_INT_SOUND isn't issued, %#x\n",
				int_factor);
			return -ETIMEDOUT;
		}
*/
	}
#endif
	return 0;
}

static int m9mo_s_stream_hdr(struct v4l2_subdev *sd, int enable)
{
	struct m9mo_state *state = to_state(sd);
	int int_en, int_factor, i, err;

	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
		M9MO_CAPCTRL_CAP_MODE, enable ? 0x06 : 0x00);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
		M9MO_CAPPARM_YUVOUT_MAIN, enable ? 0x00 : 0x21);
		CHECK_ERR(err);

	err = m9mo_readw(sd, M9MO_CATEGORY_SYS, M9MO_SYS_INT_EN, &int_en);
		CHECK_ERR(err);

	if (enable)
		int_en |= M9MO_INT_FRAME_SYNC;
	else
		int_en &= ~M9MO_INT_FRAME_SYNC;

	err = m9mo_writew(sd, M9MO_CATEGORY_SYS, M9MO_SYS_INT_EN, int_en);
		CHECK_ERR(err);

	if (enable) {
		err = m9mo_set_mode(sd, M9MO_STILLCAP_MODE);
		if (err <= 0) {
			cam_err("failed to set mode\n");
			return err;
		}

		/* convert raw to jpeg by the image data processing and
		   store memory on ISP and
		   receive preview jpeg image from ISP */
		for (i = 0; i < 3; i++) {
			int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
			if (!(int_factor & M9MO_INT_FRAME_SYNC)) {
				cam_err("M9MO_INT_FRAME_SYNC isn't issued, %#x\n",
					int_factor);
				return -ETIMEDOUT;
			}
		}

		/* stop ring-buffer */
		if (!(state->isp.int_factor & M9MO_INT_CAPTURE)) {
			/* FIXME - M9MO_INT_FRAME_SYNC interrupt
			   should be issued just three times */
			for (i = 0; i < 9; i++) {
				int_factor = m9mo_wait_interrupt(sd,
						M9MO_ISP_TIMEOUT);
				if (int_factor & M9MO_INT_CAPTURE)
					break;

				cam_err("M9MO_INT_CAPTURE isn't issued, %#x\n",
						int_factor);
			}
		}
	} else {
	}
	return 0;
}

static int m9mo_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct m9mo_state *state = to_state(sd);
	int err;

	cam_trace("E\n");

	if (unlikely(state->isp.bad_fw)) {
		cam_err("\"Unknown\" state, please update F/W");
		return -ENOSYS;
	}

	switch (enable) {
	case STREAM_MODE_CAM_ON:
	case STREAM_MODE_CAM_OFF:
		switch (state->format_mode) {
		case V4L2_PIX_FMT_MODE_CAPTURE:
			cam_info("capture %s",
				enable == STREAM_MODE_CAM_ON ? "on" : "off");
			err = m9mo_s_stream_capture(sd,
					enable == STREAM_MODE_CAM_ON);
			break;
		case V4L2_PIX_FMT_MODE_HDR:
			err = m9mo_s_stream_hdr(sd,
					enable == STREAM_MODE_CAM_ON);
			break;
		default:
			cam_err("preview %s",
				enable == STREAM_MODE_CAM_ON ? "on" : "off");
			err = m9mo_s_stream_preview(sd,
					enable == STREAM_MODE_CAM_ON);
			break;
		}
		break;

	case STREAM_MODE_MOVIE_ON:
		state->recording = 1;
		if (state->flash_mode != FLASH_MODE_OFF)
			err = m9mo_set_flash(sd, state->flash_mode, 1);

		if (state->preview->index == M9MO_PREVIEW_720P ||
				state->preview->index == M9MO_PREVIEW_1080P)
			err = m9mo_set_af(sd, 1);
		break;

	case STREAM_MODE_MOVIE_OFF:
		if (state->preview->index == M9MO_PREVIEW_720P ||
				state->preview->index == M9MO_PREVIEW_1080P)
			err = m9mo_set_af(sd, 0);

		m9mo_set_flash(sd, FLASH_MODE_OFF, 1);

		state->recording = 0;
		break;

	default:
		cam_err("invalid stream option, %d\n", enable);
		break;
	}

	cam_trace("X\n");
	return 0;
}

static int m9mo_check_version(struct v4l2_subdev *sd)
{
	struct m9mo_state *state = to_state(sd);
	int i, val;

	for (i = 0; i < 6; i++) {
		m9mo_readb(sd, M9MO_CATEGORY_SYS, M9MO_SYS_USER_VER, &val);
		state->exif.unique_id[i] = (char)val;
	}
	state->exif.unique_id[i] = '\0';

	cam_info("*************************************\n");
	cam_info("F/W Version: %s\n", state->exif.unique_id);
	cam_dbg("Binary Released: %s %s\n", __DATE__, __TIME__);
	cam_info("*************************************\n");

	return 0;
}

static int m9mo_init_param(struct v4l2_subdev *sd)
{
	int err;
	cam_trace("E\n");

	err = m9mo_writew(sd, M9MO_CATEGORY_SYS, M9MO_SYS_INT_EN,
			M9MO_INT_MODE | M9MO_INT_CAPTURE | M9MO_INT_FRAME_SYNC
			/* | M9MO_INT_SOUND*/);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_PARM,
			M9MO_PARM_OUT_SEL, 0x02);
	CHECK_ERR(err);

	/* Capture */
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPPARM,
			M9MO_CAPPARM_YUVOUT_MAIN, 0x01);
	CHECK_ERR(err);

#if 0
	err = m9mo_writel(sd, M9MO_CATEGORY_CAPPARM,
		M9MO_CAPPARM_THUMB_JPEG_MAX, M9MO_THUMB_MAXSIZE);
	CHECK_ERR(err);

	/* Face detect */
	err = m9mo_writeb(sd, M9MO_CATEGORY_FD, M9MO_FD_SIZE, 0x01);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_FD, M9MO_FD_MAX, 0x0B);
	CHECK_ERR(err);

	/* HDR */
	err = m9mo_writeb(sd, M9MO_CATEGORY_CAPCTRL,
			M9MO_CAPCTRL_CAP_FRM_COUNT, 0x03);
	CHECK_ERR(err);

	err = m9mo_writeb(sd, M9MO_CATEGORY_AE, M9MO_AE_AUTO_BRACKET_EV, 0x64);
	CHECK_ERR(err);
#endif
	cam_trace("X\n");
	return 0;
}

static int m9mo_ois_init(struct v4l2_subdev *sd)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	const struct m9mo_platform_data *pdata = client->dev.platform_data;
	struct m9mo_state *state = to_state(sd);
	u32 int_factor, int_en, err, ois_result;
	int try_cnt = 2;

	err = m9mo_readw(sd, M9MO_CATEGORY_SYS, M9MO_SYS_INT_EN, &int_en);
	CHECK_ERR(err);

	/* enable OIS_INIT interrupt */
	int_en |= M9MO_INT_OIS_INIT;
	/* enable LENS_INIT interrupt */
	int_en |= M9MO_INT_LENS_INIT;

	err = m9mo_writew(sd, M9MO_CATEGORY_SYS, M9MO_SYS_INT_EN, int_en);
	CHECK_ERR(err);

	do {
		/* SambaZ PLL enable */
		pdata->config_sambaz(1);

		/* OIS on set */
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
				0x10, 0x01);
		CHECK_ERR(err);

		/* OIS F/W download, boot */
		err = m9mo_writeb(sd, M9MO_CATEGORY_NEW,
				0x11, 0x00);

		int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
		if (!(int_factor & M9MO_INT_OIS_INIT)) {
			cam_err("OIS interrupt not issued\n");
			state->isp.bad_fw = 1;
			return -ENOSYS;
		}
		cam_info("OIS init complete\n");

		/* Read OIS result */
		m9mo_readb(sd, M9MO_CATEGORY_NEW, 0x17, &ois_result);
		cam_info("ois result = %d", ois_result);
		if (ois_result != 0x02)
			try_cnt--;
		else
			try_cnt = 0;
	} while (try_cnt);

	/* Lens boot */
	err = m9mo_writeb(sd, M9MO_CATEGORY_LENS,
			M9MO_LENS_AF_INITIAL, 0x00);
	int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
	if (!(int_factor & M9MO_INT_LENS_INIT)) {
		cam_err("M9MO_INT_LENS_INIT isn't issued, %#x\n",
				int_factor);
		return -ETIMEDOUT;
	}

	return err;
}

static int m9mo_init(struct v4l2_subdev *sd, u32 val)
{
	struct m9mo_state *state = to_state(sd);
	u32 int_factor;
	/*u32 value;*/
	int err;

	/* Default state values */
	state->preview = NULL;
	state->capture = NULL;

	state->format_mode = V4L2_PIX_FMT_MODE_PREVIEW;
	state->sensor_mode = SENSOR_CAMERA;
	state->flash_mode = FLASH_MODE_OFF;
	state->scene_mode = SCENE_MODE_NONE;

	state->face_beauty = 0;

	state->fps = 0;			/* auto */

	state->isp.bad_fw = 0;
	state->isp.issued = 0;

	state->vss_mode = 0x00;
	state->focus_area_mode = V4L2_FOCUS_AREA_CENTER;

	memset(&state->focus, 0, sizeof(state->focus));

	/* Test force update FW */
#if 0
	err = m9mo_load_fw_main(sd);
	msleep(1000);
#endif
	if (system_rev > 0) {
		err = m9mo_writel(sd, M9MO_CATEGORY_FLASH,
				0x0C, 0x27c00020);
	}

	/* start camera program(parallel FLASH ROM) */
	cam_info("write 0x0f, 0x12~~~\n");
	err = m9mo_writeb(sd, M9MO_CATEGORY_FLASH,
			M9MO_FLASH_CAM_START, 0x01);
	CHECK_ERR(err);

	int_factor = m9mo_wait_interrupt(sd, M9MO_ISP_TIMEOUT);
	if (!(int_factor & M9MO_INT_MODE)) {
		cam_err("firmware was erased?\n");
		state->isp.bad_fw = 1;
		return -ENOSYS;
	}
	cam_info("ISP boot complete\n");

#if 0
	cam_err("read 0x00, 0x1c~~~\n");

	m9mo_readb(sd, M9MO_CATEGORY_SYS,
		M9MO_SYS_INT_FACTOR, &state->isp.int_factor);
	cam_err("state->isp.int_factor = %x\n", state->isp.int_factor);

	m9mo_readb(sd, 0x01,
		0x01, &value);
	cam_err("value = %x\n", value);
#endif

	/* check up F/W version */
	err = m9mo_check_fw(sd);

	m9mo_init_param(sd);
	m9mo_ois_init(sd);

#if 0
	cam_info("Sysfs full version of phone = %s\n", sysfs_phone_fw);
	cam_info("Sysfs full version of sensor = %s\n", sysfs_sensor_fw);
#endif
	cam_info("Lens boot complete - M9MO init complete\n");

	return 0;
}

static const struct v4l2_subdev_core_ops m9mo_core_ops = {
	.init = m9mo_init,		/* initializing API */
	.load_fw = m9mo_load_fw_main,
	.queryctrl = m9mo_queryctrl,
	.g_ctrl = m9mo_g_ctrl,
	.s_ctrl = m9mo_s_ctrl,
	.g_ext_ctrls = m9mo_g_ext_ctrls,
};

static const struct v4l2_subdev_video_ops m9mo_video_ops = {
	.s_mbus_fmt = m9mo_s_fmt,
	.g_parm = m9mo_g_parm,
	.s_parm = m9mo_s_parm,
	.enum_framesizes = m9mo_enum_framesizes,
	.s_stream = m9mo_s_stream,
};

static const struct v4l2_subdev_ops m9mo_ops = {
	.core = &m9mo_core_ops,
	.video = &m9mo_video_ops,
};

static ssize_t m9mo_camera_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n", "16M External ISP");
}

static ssize_t m9mo_camera_fw_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s %s\n", sysfs_phone_fw, sysfs_sensor_fw);
}

static DEVICE_ATTR(rear_camtype, S_IRUGO, m9mo_camera_type_show, NULL);
static DEVICE_ATTR(rear_camfw, S_IRUGO, m9mo_camera_fw_show, NULL);

/*
 * m9mo_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
static int __devinit m9mo_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct m9mo_state *state;
	struct v4l2_subdev *sd;

	const struct m9mo_platform_data *pdata = client->dev.platform_data;
	int err = 0;

	state = kzalloc(sizeof(struct m9mo_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;

	sd = &state->sd;
	strcpy(sd->name, M9MO_DRIVER_NAME);

	/* Registering subdev */
	v4l2_i2c_subdev_init(sd, client, &m9mo_ops);

#ifdef CAM_DEBUG
	state->dbg_level = CAM_TRACE | CAM_DEBUG;
#endif

	/* wait queue initialize */
	init_waitqueue_head(&state->isp.wait);

	if (pdata->config_isp_irq)
		pdata->config_isp_irq();

	err = request_irq(pdata->irq,
		m9mo_isp_isr, IRQF_TRIGGER_RISING, "m9mo isp", sd);
	if (err) {
		cam_err("failed to request irq ~~~~~~~~~~~~~\n");
		return err;
	}
	state->isp.irq = pdata->irq;
	state->isp.issued = 0;

	cam_dbg("%s\n", __func__);

	return 0;
}

static int __devexit m9mo_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct m9mo_state *state = to_state(sd);
	/*int err;*/

	if (m9mo_set_lens_off(sd) < 0)
		cam_err("failed to set m9mo_set_lens_off~~~~~\n");

	if (state->isp.irq > 0)
		free_irq(state->isp.irq, sd);

	v4l2_device_unregister_subdev(sd);

#if 0
	kfree(state->fw_version);
#endif
	kfree(state);

	return 0;
}

static const struct i2c_device_id m9mo_id[] = {
	{ M9MO_DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, m9mo_id);

static struct i2c_driver m9mo_i2c_driver = {
	.driver = {
		.name	= M9MO_DRIVER_NAME,
	},
	.probe		= m9mo_probe,
	.remove		= __devexit_p(m9mo_remove),
	.id_table	= m9mo_id,
};

static int __init m9mo_mod_init(void)
{
	if (!m9mo_dev) {
		m9mo_dev =
		device_create(camera_class, NULL, 0, NULL, "rear");
		if (IS_ERR(m9mo_dev)) {
			cam_err("failed to create device m9mo_dev!\n");
			return 0;
		}
		if (device_create_file
		(m9mo_dev, &dev_attr_rear_camtype) < 0) {
			cam_err("failed to create device file, %s\n",
			dev_attr_rear_camtype.attr.name);
		}
		if (device_create_file
		(m9mo_dev, &dev_attr_rear_camfw) < 0) {
			cam_err("failed to create device file, %s\n",
			dev_attr_rear_camfw.attr.name);
		}
	}
	return i2c_add_driver(&m9mo_i2c_driver);
}

static void __exit m9mo_mod_exit(void)
{
	i2c_del_driver(&m9mo_i2c_driver);
}
module_init(m9mo_mod_init);
module_exit(m9mo_mod_exit);

MODULE_DESCRIPTION("driver for Fusitju M9MO LS 16MP camera");
MODULE_LICENSE("GPL");
