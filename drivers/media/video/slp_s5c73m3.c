/*
 * driver for LSI S5C73M3 (ISP for 8MP Camera)
 *
 * Copyright (c) 2011, Samsung Electronics. All rights reserved
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
#include <linux/vmalloc.h>
#include <linux/firmware.h>
#include <linux/videodev2.h>

#include <plat/gpio-cfg.h>
#include <linux/gpio.h>

#ifdef CONFIG_BUSFREQ_OPP
#include <mach/dev.h>
#endif

#ifdef CONFIG_VIDEO_SAMSUNG_V4L2
#include <linux/videodev2_samsung.h>
#endif

#include <linux/regulator/machine.h>

#include <media/s5c73m3_platform.h>
#ifdef CONFIG_VIDEO_SLP_S5C73M3
#include "slp_s5c73m3.h"
#else
#include "s5c73m3.h"
#endif

#define S5C73M3_DRIVER_NAME	"S5C73M3"

extern struct class *camera_class; /*sys/class/camera*/
struct device *s5c73m3_dev; /*sys/class/camera/rear*/
struct v4l2_subdev *sd_internal;

#ifdef CONFIG_BUSFREQ_OPP
static struct device *bus_dev;
#endif

/*#define S5C73M3_FROM_BOOTING*/
#define SEPERATE_RESOLUTION_SETTING

#ifdef CONFIG_VIDEO_SLP_S5C73M3
#ifdef CONFIG_GPIO_MIDAS_02_BD
#define S5C73M3_FW_PATH		"/lib/firmware/SlimISP_OA.bin"
#define S5C73M3_FW_REQUEST_PATH		"SlimISP_OA.bin"
#else
#define S5C73M3_FW_PATH		"/lib/firmware/SlimISP_OB.bin"
#define S5C73M3_FW_REQUEST_PATH		"SlimISP_OB.bin"
#endif
#define S5C73M3_FW_OD_PATH		"/lib/firmware/SlimISP_OD.bin"
#define S5C73M3_FW_SC_PATH		"/lib/firmware/SlimISP_SC.bin"
#define S5C73M3_FW_SD_PATH		"/lib/firmware/SlimISP_SD.bin"
#else
#ifdef CONFIG_MACH_MIDAS_02_BD
#define S5C73M3_FW_PATH		"/sdcard/SlimISP_OA.bin"
#define S5C73M3_FW_REQUEST_PATH		"SlimISP_OA.bin"
#else
#define S5C73M3_FW_PATH		"/sdcard/SlimISP_OB.bin"
#define S5C73M3_FW_REQUEST_PATH		"SlimISP_OB.bin"
#endif
#define S5C73M3_FW_OD_PATH		"/sdcard/SlimISP_OD.bin"
#define S5C73M3_FW_SC_PATH		"/sdcard/SlimISP_SC.bin"
#define S5C73M3_FW_SD_PATH		"/sdcard/SlimISP_SD.bin"
#endif

#define S5C73M3_FW_REQUEST_OD_PATH		"SlimISP_OD.bin"
#define S5C73M3_FW_REQUEST_SC_PATH		"SlimISP_SC.bin"
#define S5C73M3_FW_REQUEST_SD_PATH		"SlimISP_SD.bin"

#define S5C73M3_FW_VER_LEN		6
#define S5C73M3_FW_VER_FILE_CUR	0x60

#define S5C73M3_FLASH_BASE_ADDR	0x10000000
#define S5C73M3_INT_RAM_BASE_ADDR	0x68000000

#define S5C73M3_I2C_RETRY		5
#define S5C73M3_I2C_VERIFY		100
#define S5C73M3_ISP_TIMEOUT	3000
#define S5C73M3_ISP_AFB_TIMEOUT	15000 /* FIXME */
#define S5C73M3_ISP_ESD_TIMEOUT	1000

#define S5C73M3_JPEG_MAXSIZE	0x800000
#define S5C73M3_YUV_MAXSIZE	0x3F4800 /*FHD*/
#define S5C73M3_POINTER_MAXSIZE	0x10E0 /*FHD*/

#define S5C73M3_DEF_APEX_DEN	100

#define CHECK_ERR(x)	if ((x) < 0) { \
				cam_err("i2c failed, err %d\n", x); \
				return x; \
			}
static const struct s5c73m3_frmsizeenum preview_frmsizes[] = {
	{ S5C73M3_PREVIEW_QVGA,	320,	240,	0x01 },
	{ S5C73M3_PREVIEW_CIF,	352,	288,	0x0E },
	{ S5C73M3_PREVIEW_VGA,	640,	480,	0x02 },
	{ S5C73M3_PREVIEW_880X720,	880,	720,	0x03 },
	{ S5C73M3_PREVIEW_960X720,	960,	720,	0x04 },
	{ S5C73M3_PREVIEW_1056X704,	1056,	704,	0x0F },
	{ S5C73M3_PREVIEW_1184X666,	1184,	666,	0x05 },
	{ S5C73M3_PREVIEW_720P,	1280,	720,	0x06 },
	{ S5C73M3_PREVIEW_1080P,	1920,	1080,	0x0A},
	{ S5C73M3_VDIS_1080P,	2304,	1296,	0x0C},
};

static const struct s5c73m3_frmsizeenum capture_frmsizes[] = {
	{ S5C73M3_CAPTURE_VGA,	640,	480,	0x10 },
	{ S5C73M3_CAPTURE_HD,	1280,	720,	0x40 },
	{ S5C73M3_CAPTURE_2MP,	1600,	1200,	0x70 },
	{ S5C73M3_CAPTURE_W2MP,	2048,	1152,	0x80 },
	{ S5C73M3_CAPTURE_3MP,	2048,	1536,	0x90 },
	{ S5C73M3_CAPTURE_W6MP,	3264,	1836,	0xE0 },
	{ S5C73M3_CAPTURE_8MP,	3264,	2448,	0xF0 },
};

static struct s5c73m3_control s5c73m3_ctrls[] = {
	/* Flash */
	{
		.id = V4L2_CID_CAMERA_FLASH_MODE,
		.minimum = FLASH_MODE_OFF,
		.maximum = FLASH_MODE_MAX - 1,
		.step = 1,
		.value = FLASH_MODE_OFF,
		.default_value = FLASH_MODE_OFF,
	},

	/* Exposure & Scenemode stuff(ISO, Metering, Saturation, etc) */
	{
		.id = V4L2_CID_CAMERA_ISO,
		.minimum = ISO_AUTO,
		.maximum = ISO_800,
		.step = 1,
		.value = ISO_AUTO,
		.default_value = ISO_AUTO,
	}, {
		.id = V4L2_CID_EXPOSURE,
		.minimum = EV_MINUS_4,
		.maximum = EV_MAX - 1,
		.step = 1,
		.value = EV_DEFAULT,
		.default_value = EV_DEFAULT,
	}, {
		.id = V4L2_CID_SATURATION,
		.minimum = SATURATION_MINUS_2,
		.maximum = SATURATION_MAX - 1,
		.step = 1,
		.value = SATURATION_DEFAULT,
		.default_value = SATURATION_DEFAULT,
	}, {
		.id = V4L2_CID_SHARPNESS,
		.minimum = SHARPNESS_MINUS_2,
		.maximum = SHARPNESS_MAX - 1,
		.step = 1,
		.value = SHARPNESS_DEFAULT,
		.default_value = SHARPNESS_DEFAULT,
	}, {
		.id = V4L2_CID_CAMERA_METERING,
		.minimum = METERING_MATRIX,
		.maximum = METERING_MAX - 1,
		.step = 1,
		.value = METERING_MATRIX,
		.default_value = METERING_MATRIX,
	}, {
		.id = V4L2_CID_WHITE_BALANCE_PRESET,
		.minimum = WHITE_BALANCE_AUTO,
		.maximum = WHITE_BALANCE_MAX - 1,
		.step = 1,
		.value = WHITE_BALANCE_AUTO,
		.default_value = WHITE_BALANCE_AUTO,
	}, {
		.id = V4L2_CID_COLORFX,
		.minimum = V4L2_COLORFX_NONE,
		.maximum = V4L2_COLORFX_VIVID,
		.step = 1,
		.value = V4L2_COLORFX_NONE,
		.default_value = V4L2_COLORFX_NONE,
	}, {
		.id = V4L2_CID_CAMERA_SCENE_MODE,
		.minimum = SCENE_MODE_NONE,
		.maximum = SCENE_MODE_MAX - 1,
		.step = 1,
		.value = SCENE_MODE_NONE,
		.default_value = SCENE_MODE_MAX,
	},

	/* Zoom */
	{
		.id = V4L2_CID_ZOOM_ABSOLUTE,
		.minimum = ZOOM_LEVEL_0,
		.maximum = ZOOM_LEVEL_MAX - 1,
		.step = 1,
		.value = ZOOM_LEVEL_0,
		.default_value = ZOOM_LEVEL_0,
	},

	/* Focus */
	{
		.id = V4L2_CID_FOCUS_AUTO_RECTANGLE_LEFT,
		.minimum = 0,
		.maximum = 4000,	/* FIXME */
		.step = 1,
		.value = 0,
		.default_value = 0,
	}, {
		.id = V4L2_CID_FOCUS_AUTO_RECTANGLE_TOP,
		.minimum = 0,
		.maximum = 3000,	/* FIXME */
		.step = 1,
		.value = 0,
		.default_value = 0,
	}, {
		.id = V4L2_CID_FOCUS_AUTO_RECTANGLE_WIDTH,
		.minimum = 0,
		.maximum = 4000,	/* FIXME */
		.step = 1,
		.value = 0,
		.default_value = 0,
	}, {
		.id = V4L2_CID_FOCUS_AUTO_RECTANGLE_HEIGHT,
		.minimum = 0,
		.maximum = 3000,	/* FIXME */
		.step = 1,
		.value = 0,
		.default_value = 0,
	},
#if 1
	{
		.id = V4L2_CID_FOCUS_AUTO_MODE,
		.minimum = V4L2_FOCUS_AUTO_NORMAL,
		.maximum = V4L2_FOCUS_AUTO_RECTANGLE,
		.step = 1,
		.value = V4L2_FOCUS_AUTO_NORMAL,
		.default_value = V4L2_FOCUS_AUTO_NORMAL,
	}, {
		.id = V4L2_CID_CAMERA_SET_AUTO_FOCUS,
		.minimum = 0,
		.maximum = 1,
		.step = 1,
		.value = 0,
		.default_value = 0,
	},
#endif

	/* Capture */
	{
		.id = V4L2_CID_CAM_JPEG_QUALITY,
		.minimum = 1,
		.maximum = 100,
		.step = 1,
		.value = 100,
		.default_value = 100,
	},
};

static int s5c73m3_s_stream_sensor(struct v4l2_subdev *sd, int onoff);
static int s5c73m3_set_touch_auto_focus(struct v4l2_subdev *sd);
static int s5c73m3_SPI_booting(struct v4l2_subdev *sd);

static inline struct s5c73m3_state *to_state(struct v4l2_subdev *sd)
{
	return container_of(sd, struct s5c73m3_state, sd);
}

static int s5c73m3_i2c_write(struct v4l2_subdev *sd,
	unsigned short addr, unsigned short data)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg msg;
	unsigned char buf[4];
	int i, err;

	if (!client->adapter)
		return -ENODEV;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = sizeof(buf);
	msg.buf = buf;

	buf[0] = addr >> 8;
	buf[1] = addr & 0xff;
	buf[2] = data >> 8;
	buf[3] = data & 0xff;

	cam_i2c_dbg("addr %#x, data %#x\n", addr, data);

	for (i = S5C73M3_I2C_RETRY; i; i--) {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			break;
		msleep(20);
	}

	return err;
}

static int s5c73m3_i2c_write_block(struct v4l2_subdev *sd,
	const u32 regs[], int size)
{
	int i, err = 0;

	for (i = 0; i < size; i++) {
		err = s5c73m3_i2c_write(sd, (regs[i]>>16), regs[i]);
		CHECK_ERR(err);
	}

	return err;
}

static int s5c73m3_i2c_read(struct v4l2_subdev *sd,
	unsigned short addr, unsigned short *data)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct i2c_msg msg;
	unsigned char buf[2];
	int i, err;

	if (!client->adapter)
		return -ENODEV;

	msg.addr = client->addr;
	msg.flags = 0;
	msg.len = sizeof(buf);
	msg.buf = buf;

	buf[0] = addr >> 8;
	buf[1] = addr & 0xff;

	for (i = S5C73M3_I2C_RETRY; i; i--) {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			break;
		msleep(20);
	}

	if (err != 1) {
		cam_err("addr %#x\n", addr);
		return err;
	}

	msg.flags = I2C_M_RD;

	for (i = S5C73M3_I2C_RETRY; i; i--) {
		err = i2c_transfer(client->adapter, &msg, 1);
		if (err == 1)
			break;
		msleep(20);
	}

	if (err != 1) {
		cam_err("addr %#x\n", addr);
		return err;
	}

	*data = ((buf[0] << 8) | buf[1]);

	return err;
}

static int s5c73m3_write(struct v4l2_subdev *sd,
	unsigned short addr1, unsigned short addr2, unsigned short data)
{
	int err;

	err = s5c73m3_i2c_write(sd, 0x0050, addr1);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0054, addr2);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, data);
	CHECK_ERR(err);

	return err;
}

static int s5c73m3_read(struct v4l2_subdev *sd,
	unsigned short addr1, unsigned short addr2, unsigned short *data)
{
	int err;

	err = s5c73m3_i2c_write(sd, 0xfcfc, 0x3310);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0058, addr1);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x005C, addr2);
	CHECK_ERR(err);

	err = s5c73m3_i2c_read(sd, 0x0F14, data);
	CHECK_ERR(err);

	return err;
}

static int s5c73m3_i2c_check_status(struct v4l2_subdev *sd)
{
	int err = 0;
	int index = 0;
	u16 status = 0;

	do {
		err = s5c73m3_read(sd, 0x0009, 0x5080, &status);
		if (status == 0xffff)
			break;

		index++;
		msleep(20);
	} while (index < 50);

	cam_dbg("index : %d, status : %#x\n", index, status);

	return err;
}

static int s5c73m3_writeb(struct v4l2_subdev *sd,
	unsigned short addr, unsigned short data)
{
	int err;
	s5c73m3_i2c_check_status(sd);

	/*err = s5c73m3_i2c_write(sd, 0xfcfc, 0x3310);*/
	/*CHECK_ERR(err);*/

	err = s5c73m3_i2c_write(sd, 0x0050, 0x0009);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0054, 0x5000);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, addr);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, data);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0054, 0x5080);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0001);
	CHECK_ERR(err);

	return err;
}

static int s5c73m3_set_mode(struct v4l2_subdev *sd)
{
#ifdef SEPERATE_RESOLUTION_SETTING
	struct s5c73m3_state *state = to_state(sd);
	struct v4l2_control ctrl;
	int err;
	cam_trace("E\n");

	if (state->format_mode != V4L2_PIX_FMT_MODE_CAPTURE) {
		err = s5c73m3_writeb(sd, S5C73M3_IMG_OUTPUT,
			S5C73M3_YUV_OUTPUT);
		CHECK_ERR(err);

		cam_dbg("yuv ouput mode\n");
	} else {
		err = s5c73m3_writeb(sd, S5C73M3_IMG_OUTPUT,
			S5C73M3_INTERLEAVED_OUTPUT);
		CHECK_ERR(err);

		cam_dbg("interleaved ouput mode\n");
	}

	cam_trace("X\n");
#endif
	return 0;
}

/*
 * v4l2_subdev_core_ops
 */
static int s5c73m3_queryctrl(struct v4l2_subdev *sd, struct v4l2_queryctrl *qc)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(s5c73m3_ctrls); i++) {
		if (qc->id == s5c73m3_ctrls[i].id) {
			qc->maximum = s5c73m3_ctrls[i].maximum;
			qc->minimum = s5c73m3_ctrls[i].minimum;
			qc->step = s5c73m3_ctrls[i].step;
			qc->default_value = s5c73m3_ctrls[i].default_value;
			return 0;
		}
	}

	return -EINVAL;
}

#ifdef CONFIG_TARGET_LOCALE_KOR
static int s5c73m3_set_antibanding(struct v4l2_subdev *sd, int val)
{
	return 0;
}
#endif

static int s5c73m3_set_af_softlanding(struct v4l2_subdev *sd)
{
	return 0;
}

static int s5c73m3_dump_fw(struct v4l2_subdev *sd)
{
	return 0;
}

static int s5c73m3_get_sensor_fw_version(struct v4l2_subdev *sd)
{
	struct s5c73m3_state *state = to_state(sd);
	u16 read_val;
	u16 sensor_fw;
	u16 af_cal_data;
	int i;
	int err = 0;

	cam_trace("E\n");

	/*ARM go*/
	err = s5c73m3_write(sd, 0x3000, 0x0004, 0xFFFF);
	CHECK_ERR(err);

	udelay(400);

	/*Check boot done*/
	for (i = 0; i < 3; i++) {
		err = s5c73m3_read(sd, 0x3010, 0x0010, &read_val);
		CHECK_ERR(err);

		if (read_val == 0x0C)
			break;

		udelay(100);
	}

	if (read_val != 0x0C) {
		cam_err("boot fail, read_val %#x\n", read_val);
		return -1;
	}

	/*P,M,S and Boot Mode*/
	err = s5c73m3_write(sd, 0x3010, 0x0014, 0x2146);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3010, 0x0010, 0x230C);
	CHECK_ERR(err);

	udelay(200);

	/*Check SPI ready*/
	for (i = 0; i < 300; i++) {
		err = s5c73m3_read(sd, 0x3010, 0x0010, &read_val);
		CHECK_ERR(err);

		if (read_val == 0x230E)
			break;

		udelay(100);
	}

	if (read_val != 0x230E) {
		cam_err("SPI not ready, read_val %#x\n", read_val);
		return -1;
	}

	/*ARM reset*/
	err = s5c73m3_write(sd, 0x3000, 0x0004, 0xFFFD);
	CHECK_ERR(err);

	/*remap*/
	err = s5c73m3_write(sd, 0x3010, 0x00A4, 0x0183);
	CHECK_ERR(err);

	/*ARM go again*/
	err = s5c73m3_write(sd, 0x3000, 0x0004, 0xFFFF);
	CHECK_ERR(err);

	for (i = 0; i < 3; i++) {
		err = s5c73m3_read(sd, 0x0000, 0x0060+i*2, &sensor_fw);
		CHECK_ERR(err);
		state->sensor_fw[i*2] = sensor_fw&0x00ff;
		state->sensor_fw[i*2+1] = (sensor_fw&0xff00)>>8;
	}
	state->sensor_fw[i*2+2] = ' ';

	cam_dbg("Sensor_version = %s\n", state->sensor_fw);
/*
	err = s5c73m3_i2c_check_status(sd);
	CHECK_ERR(err);
	err = s5c73m3_read(sd, 0x0009, 0x3000, &af_cal_data);
	CHECK_ERR(err);
	cam_trace("Data...... = 0x%x\n", af_cal_data);
	err = s5c73m3_read(sd, 0x0009, 0x590E, &af_cal_data);
	CHECK_ERR(err);
	cam_trace("AF CAL. = 0x%x\n", af_cal_data);
*/
	cam_trace("X\n");

	return err;
}


static int s5c73m3_get_phone_fw_version(struct v4l2_subdev *sd)
{
	struct device *dev = sd->v4l2_dev->dev;
	struct s5c73m3_state *state = to_state(sd);
	const struct firmware *fw;
	u8 *buf = NULL;
	int err = 0;
	int txSize;

	struct file *fp;
	mm_segment_t old_fs;
	long fsize, nread;
	int fw_requested = 1;

	buf = vmalloc(S5C73M3_FW_VER_LEN+1);
	if (!buf) {
		cam_err("failed to allocate memory\n");
		err = -ENOMEM;
		goto out;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fp = filp_open(S5C73M3_FW_PATH, O_RDONLY, 0);
	if (IS_ERR(fp)) {
		cam_trace("failed to open %s, err %ld\n",
			S5C73M3_FW_PATH, PTR_ERR(fp));
		goto request_fw;
	}

	fw_requested = 0;
	err = vfs_llseek(fp, S5C73M3_FW_VER_FILE_CUR, SEEK_SET);
	if (err < 0) {
		cam_warn("failed to fseek, %d\n", err);
		goto out;
	}

	nread = vfs_read(fp, (char __user *)buf,
		S5C73M3_FW_VER_LEN,
		&fp->f_pos);

	if (nread != S5C73M3_FW_VER_LEN) {
		cam_err("failed to read firmware file, %ld Bytes\n", nread);
		err = -EIO;
		goto out;
	}

request_fw:
	if (fw_requested) {
		set_fs(old_fs);

		err = request_firmware(&fw, S5C73M3_FW_REQUEST_PATH, dev);
		if (err != 0) {
			cam_err("request_firmware falied\n");
			err = -EINVAL;
			goto out;
		}
		memcpy(buf, (u8 *)&fw->data[S5C73M3_FW_VER_FILE_CUR],
			S5C73M3_FW_VER_LEN);

	}

	memcpy(state->phone_fw, buf, S5C73M3_FW_VER_LEN);
	state->phone_fw[S5C73M3_FW_VER_LEN+1] = ' ';
	cam_dbg("Phone_version = %s\n", state->phone_fw);

	cam_dbg("end\n");

out:
	if (!fw_requested) {
		vfree(buf);

		filp_close(fp, current->files);
		set_fs(old_fs);
	} else {
		release_firmware(fw);
	}

	return err;
}

static int s5c73m3_check_fw(struct v4l2_subdev *sd)
{
	struct s5c73m3_state *state = to_state(sd);
	int err, ret;

	cam_trace("E\n");

	err = s5c73m3_get_sensor_fw_version(sd);
	err = s5c73m3_get_phone_fw_version(sd);

	if (state->phone_fw[0] == 'S' || state->phone_fw[0] == 'O') {
		err = state->pdata->is_isp_reset();
		CHECK_ERR(err);

		err = s5c73m3_SPI_booting(sd);
		CHECK_ERR(err);

		err = s5c73m3_writeb(sd, 0x0E06,
			0x0000);
		CHECK_ERR(err);
	}
	cam_trace("X\n");
	return 0;
}

static int s5c73m3_set_sensor_mode(struct v4l2_subdev *sd, int val)
{
	struct s5c73m3_state *state = to_state(sd);
	int err;
	cam_dbg("E, value %d\n", val);

retry:
	switch (val) {
	case SENSOR_CAMERA:
		err = s5c73m3_writeb(sd, S5C73M3_AE_MODE,
			S5C73M3_AUTO_MODE_AE_SET);
		CHECK_ERR(err);
		break;

	case SENSOR_MOVIE:
		err = s5c73m3_writeb(sd, S5C73M3_AE_MODE,
			S5C73M3_FIXED_30FPS);
		CHECK_ERR(err);
		break;

	default:
		cam_warn("invalid value, %d\n", val);
		val = SENSOR_CAMERA;
		goto retry;
	}
	state->sensor_mode = val;

	cam_trace("X\n");
	return 0;
}

static int s5c73m3_set_flash(struct v4l2_subdev *sd, int val, int recording)
{
	struct s5c73m3_state *state = to_state(sd);
	int err;
	cam_dbg("E, value %d\n", val);

retry:
	switch (val) {
	case FLASH_MODE_OFF:
		err = s5c73m3_writeb(sd, S5C73M3_FLASH_MODE,
			S5C73M3_FLASH_MODE_OFF);
		CHECK_ERR(err);
		err = s5c73m3_writeb(sd, S5C73M3_FLASH_TORCH,
			S5C73M3_FLASH_TORCH_OFF);
		CHECK_ERR(err);
		break;

	case FLASH_MODE_AUTO:
		err = s5c73m3_writeb(sd, S5C73M3_FLASH_TORCH,
			S5C73M3_FLASH_TORCH_OFF);
		CHECK_ERR(err);
		err = s5c73m3_writeb(sd, S5C73M3_FLASH_MODE,
			S5C73M3_FLASH_MODE_AUTO);
		CHECK_ERR(err);
		break;

	case FLASH_MODE_ON:
		err = s5c73m3_writeb(sd, S5C73M3_FLASH_TORCH,
			S5C73M3_FLASH_TORCH_OFF);
		CHECK_ERR(err);
		err = s5c73m3_writeb(sd, S5C73M3_FLASH_MODE,
			S5C73M3_FLASH_MODE_ON);
		CHECK_ERR(err);
		break;

	case FLASH_MODE_TORCH:
		err = s5c73m3_writeb(sd, S5C73M3_FLASH_MODE,
			S5C73M3_FLASH_MODE_OFF);
		CHECK_ERR(err);
		err = s5c73m3_writeb(sd, S5C73M3_FLASH_TORCH,
			S5C73M3_FLASH_TORCH_ON);
		CHECK_ERR(err);
		break;

	default:
		cam_warn("invalid value, %d\n", val);
		val = FLASH_MODE_OFF;
		goto retry;
	}
	state->flash_mode = val;

	cam_trace("X\n");
	return 0;
}

static int s5c73m3_set_iso(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	struct s5c73m3_state *state = to_state(sd);
	cam_dbg("E, value %d\n", ctrl->value);

retry:
	switch (ctrl->value) {
	case ISO_AUTO:
		err = s5c73m3_writeb(sd, S5C73M3_ISO,
			S5C73M3_ISO_AUTO);
		CHECK_ERR(err);
		break;

	case ISO_50:
	case ISO_100:
		err = s5c73m3_writeb(sd, S5C73M3_ISO,
			S5C73M3_ISO_100);
		CHECK_ERR(err);
		break;

	case ISO_200:
		err = s5c73m3_writeb(sd, S5C73M3_ISO,
			S5C73M3_ISO_200);
		CHECK_ERR(err);
		break;

	case ISO_400:
		err = s5c73m3_writeb(sd, S5C73M3_ISO,
			S5C73M3_ISO_400);
		CHECK_ERR(err);
		break;

	case ISO_800:
		err = s5c73m3_writeb(sd, S5C73M3_ISO,
			S5C73M3_ISO_800);
		CHECK_ERR(err);
		break;

	default:
		cam_warn("invalid value, %d\n", ctrl->value);
		ctrl->value = ISO_AUTO;
		goto retry;
	}

	state->exif.iso = ctrl->value;

	cam_trace("X\n");
	return 0;
}

static int s5c73m3_set_metering(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_dbg("E, value %d\n", val);

retry:
	switch (val) {
	case METERING_CENTER:
		err = s5c73m3_writeb(sd, S5C73M3_METER,
			S5C73M3_METER_CENTER);
		CHECK_ERR(err);
		break;

	case METERING_SPOT:
		err = s5c73m3_writeb(sd, S5C73M3_METER,
			S5C73M3_METER_SPOT);
		CHECK_ERR(err);
		break;

	case METERING_MATRIX:
		err = s5c73m3_writeb(sd, S5C73M3_METER,
			S5C73M3_METER_AVERAGE);
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

static int s5c73m3_set_exposure(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	int err;
	cam_dbg("E, value %d\n", ctrl->value);

	if (ctrl->value < 0 || ctrl->value > 8) {
		cam_warn("invalid value, %d\n", ctrl->value);
		ctrl->value = 0;
	}
	err = s5c73m3_writeb(sd, S5C73M3_EV,
		ctrl->value);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int s5c73m3_set_whitebalance(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_dbg("E, value %d\n", val);

retry:
	switch (val) {
	case WHITE_BALANCE_AUTO:
		err = s5c73m3_writeb(sd, S5C73M3_AWB_MODE,
			S5C73M3_AWB_MODE_AUTO);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_SUNNY:
		err = s5c73m3_writeb(sd, S5C73M3_AWB_MODE,
			S5C73M3_AWB_MODE_DAYLIGHT);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_CLOUDY:
		err = s5c73m3_writeb(sd, S5C73M3_AWB_MODE,
			S5C73M3_AWB_MODE_CLOUDY);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_TUNGSTEN:
		err = s5c73m3_writeb(sd, S5C73M3_AWB_MODE,
			S5C73M3_AWB_MODE_INCANDESCENT);
		CHECK_ERR(err);
		break;

	case WHITE_BALANCE_FLUORESCENT:
		err = s5c73m3_writeb(sd, S5C73M3_AWB_MODE,
			S5C73M3_AWB_MODE_FLUORESCENT1);
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

static int s5c73m3_set_sharpness(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	return 0;
}

static int s5c73m3_set_saturation(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	return 0;
}

static int s5c73m3_set_scene_mode(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_dbg("E, value %d\n", val);

retry:
	switch (val) {
	case SCENE_MODE_NONE:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_NONE);
		CHECK_ERR(err);
		break;

	case SCENE_MODE_PORTRAIT:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_PORTRAIT);
		CHECK_ERR(err);
		break;

	case SCENE_MODE_LANDSCAPE:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_LANDSCAPE);
		CHECK_ERR(err);
		break;

	case SCENE_MODE_SPORTS:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_SPORTS);
		CHECK_ERR(err);
		break;

	case SCENE_MODE_PARTY_INDOOR:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_INDOOR);
		CHECK_ERR(err);
		break;

	case SCENE_MODE_BEACH_SNOW:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_BEACH);
		CHECK_ERR(err);
		break;

	case SCENE_MODE_SUNSET:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_SUNSET);
		CHECK_ERR(err);
		break;

	case SCENE_MODE_DUSK_DAWN:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_DAWN);
		CHECK_ERR(err);
		break;

	case SCENE_MODE_FALL_COLOR:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_FALL);
		CHECK_ERR(err);
		break;

	case SCENE_MODE_NIGHTSHOT:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_NIGHT);
		CHECK_ERR(err);
		break;

	case SCENE_MODE_BACK_LIGHT:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_AGAINSTLIGHT);
		CHECK_ERR(err);
		break;

	case SCENE_MODE_FIREWORKS:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_FIRE);
		CHECK_ERR(err);
		break;

	case SCENE_MODE_TEXT:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_TEXT);
		CHECK_ERR(err);
		break;

	case SCENE_MODE_CANDLE_LIGHT:
		err = s5c73m3_writeb(sd, S5C73M3_SCENE_MODE,
			S5C73M3_SCENE_MODE_CANDLE);
		CHECK_ERR(err);
		break;

	default:
		cam_warn("invalid value, %d\n", val);
		val = SCENE_MODE_NONE;
		goto retry;
	}

	cam_trace("X\n");
	return 0;
}

static int s5c73m3_set_effect(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_dbg("E, value %d\n", val);

retry:
	switch (val) {
	case IMAGE_EFFECT_NONE:
		err = s5c73m3_writeb(sd, S5C73M3_IMAGE_EFFECT,
			S5C73M3_IMAGE_EFFECT_NONE);
		CHECK_ERR(err);
		break;

	case IMAGE_EFFECT_SEPIA:
		err = s5c73m3_writeb(sd, S5C73M3_IMAGE_EFFECT,
			S5C73M3_IMAGE_EFFECT_SEPIA);
		CHECK_ERR(err);
		break;

	case IMAGE_EFFECT_BNW:
		err = s5c73m3_writeb(sd, S5C73M3_IMAGE_EFFECT,
			S5C73M3_IMAGE_EFFECT_MONO);
		CHECK_ERR(err);
		break;

	case IMAGE_EFFECT_NEGATIVE:
		err = s5c73m3_writeb(sd, S5C73M3_IMAGE_EFFECT,
			S5C73M3_IMAGE_EFFECT_NEGATIVE);
		CHECK_ERR(err);
		break;

	case IMAGE_EFFECT_AQUA:
		err = s5c73m3_writeb(sd, S5C73M3_IMAGE_EFFECT,
			S5C73M3_IMAGE_EFFECT_AQUA);
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

static int s5c73m3_set_wdr(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_dbg("E, value %d\n", val);

retry:
	switch (val) {
	case WDR_OFF:
		err = s5c73m3_writeb(sd, S5C73M3_WDR,
			S5C73M3_WDR_OFF);
		CHECK_ERR(err);
		break;

	case WDR_ON:
		err = s5c73m3_writeb(sd, S5C73M3_WDR,
			S5C73M3_WDR_ON);
		CHECK_ERR(err);
		break;

	default:
		cam_warn("invalid value, %d\n", val);
		val = WDR_OFF;
		goto retry;
	}

	cam_trace("X\n");
	return 0;
}

static int s5c73m3_set_antishake(struct v4l2_subdev *sd, int val)
{
	int err = 0;
	if (val) {
		err = s5c73m3_writeb(sd, S5C73M3_AE_MODE,
			S5C73M3_FIXED_30FPS);
		CHECK_ERR(err);
	} else {
		err = s5c73m3_writeb(sd, S5C73M3_AE_MODE,
			S5C73M3_AUTO_MODE_AE_SET);
		CHECK_ERR(err);
	}
	return err;
}

static int s5c73m3_set_face_beauty(struct v4l2_subdev *sd, int val)
{
	return 0;
}

static int s5c73m3_set_ae_lock(struct v4l2_subdev *sd, int val)
{
	int err;

	if (val)
		err = s5c73m3_writeb(sd, S5C73M3_AE_CON, S5C73M3_AE_STOP);
	else
		err = s5c73m3_writeb(sd, S5C73M3_AE_CON, S5C73M3_AE_START);

	CHECK_ERR(err);

	return err;
}

static int s5c73m3_set_awb_lock(struct v4l2_subdev *sd, int val)
{
	int err;

	if (val)
		err = s5c73m3_writeb(sd, S5C73M3_AWB_CON, S5C73M3_AWB_STOP);
	else
		err = s5c73m3_writeb(sd, S5C73M3_AWB_CON, S5C73M3_AWB_START);

	CHECK_ERR(err);

	return err;
}

static int s5c73m3_set_ae_awb_lock(struct v4l2_subdev *sd, int val)
{
	return 0;
}

static int s5c73m3_set_af(struct v4l2_subdev *sd, int val)
{
	struct s5c73m3_state *state = to_state(sd);
	u16 isneed_flash = false;
	int err = 0;

	cam_info("%s, mode %#x\n", val ? "start" : "stop", state->focus.mode);

	if (val) {
		if (state->flash_mode == FLASH_MODE_ON)
			isneed_flash = true;
		else if (state->flash_mode == FLASH_MODE_AUTO) {
			err = s5c73m3_read(sd, 0x0009,
				S5C73M3_AE_ISNEEDFLASH | 0x5000, &isneed_flash);
			CHECK_ERR(err);
		} else if (state->flash_mode == FLASH_MODE_OFF)
			isneed_flash = false;
		else {
			isneed_flash = false;
			cam_err("Unsupported flash mode!!\n");
		}

		cam_dbg("isneed flash???? %s\n",
				isneed_flash ? "true" : "false");
		if (isneed_flash) {
			err = s5c73m3_writeb(sd, S5C73M3_STILL_PRE_FLASH
					, S5C73M3_STILL_PRE_FLASH_FIRE);
			state->isflash = S5C73M3_ISNEED_FLASH_ON;
			cam_dbg("pre flash!!!\n");
		} else {
			state->isflash = S5C73M3_ISNEED_FLASH_OFF;
			cam_dbg("pre flash did not fired!!!\n");
		}

		if (state->focus.mode == V4L2_FOCUS_AUTO_RECTANGLE)
			err = s5c73m3_set_touch_auto_focus(sd);
		else
			err = s5c73m3_writeb(sd, S5C73M3_AF_CON,
				S5C73M3_AF_CON_START);

		state->focus.focusing = true;
		state->focus.status = 1;
	} else {
		err = s5c73m3_writeb(sd, S5C73M3_STILL_MAIN_FLASH
				, S5C73M3_STILL_MAIN_FLASH_CANCEL);
		err = s5c73m3_writeb(sd, S5C73M3_AF_CON,
			S5C73M3_AF_CON_STOP);
		state->isflash = S5C73M3_ISNEED_FLASH_UNDEFINED;
		state->focus.status = 0;
	}

	CHECK_ERR(err);

	cam_info("X\n");
	return 0;
}

static int s5c73m3_get_af(struct v4l2_subdev *sd)
{
	struct s5c73m3_state *state = to_state(sd);
	int err = 0;
	u16 af_status = S5C73M3_AF_STATUS_UNFOCUSED;
	/*u16 temp_status = 0;*/

	err = s5c73m3_read(sd, 0x0009, S5C73M3_AF_STATUS, &af_status);
	CHECK_ERR(err);

	/*err = s5c73m3_read(sd, 0x0009, 0x5840, &temp_status);*/

	switch (af_status) {
	case S5C73M3_AF_STATUS_FOCUSING:
	case S5C73M3_CAF_STATUS_FOCUSING:
	case S5C73M3_CAF_STATUS_FIND_SEARCHING_DIR:
	case S5C73M3_CAF_STATUS_INITIALIZE:
		err = -EBUSY;
		break;

	case S5C73M3_AF_STATUS_FOCUSED:
	case S5C73M3_CAF_STATUS_FOCUSED:
		err = 0;
		break;

	case S5C73M3_AF_STATUS_UNFOCUSED:
	default:
		err = -EAGAIN;
		break;
	}
	state->focus.status = af_status;

	cam_info("af_status = %d\n", err);

/*	cam_dbg("af_status = %d, frame_cnt = %d\n",
		state->focus.status, temp_status);*/
	return err;
}


static int s5c73m3_set_af_mode(struct v4l2_subdev *sd, int val)
{
	struct s5c73m3_state *state = to_state(sd);
	int err;
	cam_dbg("E, value %d\n", val);

retry:
	switch (val) {
	case V4L2_FOCUS_AUTO_NORMAL:
		state->focus.mode = val;

		err = s5c73m3_writeb(sd, S5C73M3_AF_MODE,
			S5C73M3_AF_MODE_NORMAL);
		CHECK_ERR(err);

#ifndef SEPERATE_RESOLUTION_SETTING
		if (state->format_mode == V4L2_PIX_FMT_MODE_CAPTURE) {
			S5C73M3_INTERLEAVED_PREVIEW[11] =
				(S5C73M3_INTERLEAVED_PREVIEW[11] & 0xFFFFFFF0)
				| S5C73M3_AF_MODE_NORMAL;
		} else {
			S5C73M3_YUV_PREVIEW[11] =
				(S5C73M3_YUV_PREVIEW[11] & 0xFFFFFFF0)
				| S5C73M3_AF_MODE_NORMAL;
		}
#endif
		break;

	case V4L2_FOCUS_AUTO_MACRO:
		state->focus.mode = val;

		err = s5c73m3_writeb(sd, S5C73M3_AF_MODE,
			S5C73M3_AF_MODE_MACRO);
		CHECK_ERR(err);

#ifndef SEPERATE_RESOLUTION_SETTING
		if (state->format_mode == V4L2_PIX_FMT_MODE_CAPTURE) {
			S5C73M3_INTERLEAVED_PREVIEW[11] =
				(S5C73M3_INTERLEAVED_PREVIEW[11] & 0xFFFFFFF0)
				| S5C73M3_AF_MODE_MACRO;
		} else {
			S5C73M3_YUV_PREVIEW[11] =
				(S5C73M3_YUV_PREVIEW[11] & 0xFFFFFFF0)
				| S5C73M3_AF_MODE_MACRO;
		}
#endif
		break;

	case V4L2_FOCUS_AUTO_CONTINUOUS:
		state->focus.mode = val;
#if 0
		state->isflash = S5C73M3_ISNEED_FLASH_UNDEFINED;
#endif

	err = s5c73m3_writeb(sd, S5C73M3_AF_MODE,
	S5C73M3_AF_MODE_PREVIEW_CAF_START);
	CHECK_ERR(err);

#ifndef SEPERATE_RESOLUTION_SETTING
		if (state->format_mode == V4L2_PIX_FMT_MODE_CAPTURE) {
			S5C73M3_INTERLEAVED_PREVIEW[11] =
				(S5C73M3_INTERLEAVED_PREVIEW[11] & 0xFFFFFFF0)
				| S5C73M3_AF_MODE_PREVIEW_CAF_START;
		} else {
			S5C73M3_YUV_PREVIEW[11] =
				(S5C73M3_YUV_PREVIEW[11] & 0xFFFFFFF0)
				| S5C73M3_AF_MODE_PREVIEW_CAF_START;
		}

#endif
		break;

	case V4L2_FOCUS_AUTO_RECTANGLE:
		state->focus.mode = val;
		break;

#if 0
	case FOCUS_MODE_CONTINOUS_VIDEO:
		state->focus.mode = val;

		err = s5c73m3_writeb(sd, S5C73M3_AF_MODE,
			S5C73M3_AF_MODE_MOVIE_CAF_START);
		CHECK_ERR(err);

#ifndef SEPERATE_RESOLUTION_SETTING
		if (state->format_mode == V4L2_PIX_FMT_MODE_CAPTURE) {
			S5C73M3_INTERLEAVED_PREVIEW[11] =
				(S5C73M3_INTERLEAVED_PREVIEW[11] & 0xFFFFFFF0)
				| S5C73M3_AF_MODE_MOVIE_CAF_START;
		} else {
			S5C73M3_YUV_PREVIEW[11] =
				(S5C73M3_YUV_PREVIEW[11] & 0xFFFFFFF0)
				| S5C73M3_AF_MODE_MOVIE_CAF_START;
		}
#endif
		break;
*/
	case FOCUS_MODE_FACEDETECT:
		state->focus.mode = val;

/*		err = s5c73m3_writeb(sd, S5C73M3_AF_FACE_DET,
			S5C73M3_AF_FACE_DET_ON);
		CHECK_ERR(err);*/
		break;

	case FOCUS_MODE_INFINITY:
		state->focus.mode = val;

/*		err = s5c73m3_writeb(sd, S5C73M3_AWB_MODE,
			S5C73M3_AWB_MODE_FLUORESCENT);
		CHECK_ERR(err);*/
		break;

	case FOCUS_MODE_TOUCH:
		state->focus.mode = val;
		break;
#endif
	default:
		cam_warn("invalid value, %d\n", val);
		val = FOCUS_MODE_AUTO;
		goto retry;
	}

	state->focus.mode = val;

	cam_trace("X\n");
	return 0;
}

static int s5c73m3_set_touch_auto_focus(struct v4l2_subdev *sd)
{
	struct s5c73m3_state *state = to_state(sd);
	int err;

	cam_dbg("s5c73m3_set_touch_auto_focus\n");
	cam_dbg("Touch Position(%d,%d)\n",
		state->focus.pos_x, state->focus.pos_y);
	cam_dbg("Preview Size(%d,%d)\n",
		state->preview->width, state->preview->height);

	err = s5c73m3_i2c_write(sd, 0xfcfc, 0x3310);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0050, 0x0009);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0054, S5C73M3_AF_TOUCH_POSITION);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, state->focus.pos_x);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, state->focus.pos_y);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, state->preview->width);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, state->preview->height);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0050, 0x0009);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0054, 0x5000);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0E0A);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0000);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0054, 0x5080);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0001);
	CHECK_ERR(err);

	return 0;
}

static int s5c73m3_set_zoom(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	int err;
	cam_dbg("E, value %d\n", ctrl->value);

retry:
	if (ctrl->value < 0 || ctrl->value > 30) {
		cam_warn("invalid value, %d\n", ctrl->value);
		ctrl->value = 0;
		goto retry;
	}

	err = s5c73m3_writeb(sd, S5C73M3_ZOOM_STEP,
			ctrl->value);
	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int s5c73m3_set_jpeg_quality(struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	int val = ctrl->value, err;
	cam_dbg("E, value %d\n", val);

	if (val <= 65)		/* Normal */
		err = s5c73m3_writeb(sd, S5C73M3_IMAGE_QUALITY,
			S5C73M3_IMAGE_QUALITY_NORMAL);
	else if (val <= 75)	/* Fine */
		err = s5c73m3_writeb(sd, S5C73M3_IMAGE_QUALITY,
			S5C73M3_IMAGE_QUALITY_FINE);
	else			/* Superfine */
		err = s5c73m3_writeb(sd, S5C73M3_IMAGE_QUALITY,
			S5C73M3_IMAGE_QUALITY_SUPERFINE);

	CHECK_ERR(err);

	cam_trace("X\n");
	return 0;
}

static int s5c73m3_get_exif(struct v4l2_subdev *sd)
{
	return 0;
}

static int s5c73m3_aeawb_lock_unlock(struct v4l2_subdev *sd, int val)
{
	struct s5c73m3_state *state = to_state(sd);
	int err = 0;
	int ae_lock = val & 0x1;
	int awb_lock = (val & 0x2) >> 1;
	int ae_lock_changed =
		~(ae_lock & state->ae_lock) & (ae_lock | state->ae_lock);
	int awb_lock_changed =
		~(awb_lock & state->awb_lock) & (awb_lock | state->awb_lock);

	if (ae_lock_changed) {
		cam_dbg("ae lock - %s\n", ae_lock ? "true" : "false");
		err = s5c73m3_writeb(sd, S5C73M3_AE_CON,
				ae_lock ? S5C73M3_AE_STOP : S5C73M3_AE_START);
		CHECK_ERR(err);
		state->ae_lock = ae_lock;
	}
	if (awb_lock_changed) {
		cam_dbg("awb lock - %s\n", awb_lock ? "true" : "false");
		err = s5c73m3_writeb(sd, S5C73M3_AWB_CON,
			awb_lock ? S5C73M3_AWB_STOP : S5C73M3_AWB_START);
		CHECK_ERR(err);
		state->awb_lock = awb_lock;
	}

	return 0;
}
static int s5c73m3_start_capture(struct v4l2_subdev *sd, int val)
{
	return 0;
}

static int s5c73m3_change_auto_braket_mode(struct v4l2_subdev *sd)
{
	struct s5c73m3_state *state = to_state(sd);
	int err = 0;
	int index = 0;
	u16 stream_status = 0;

	err = s5c73m3_i2c_write(sd, 0xfcfc, 0x3310);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0050, 0x0009);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0054, 0x5000);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0902);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0008);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x091A);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0002);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0B10);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x8000 |
			state->capture->reg_val |
			state->preview->reg_val);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0054, 0x5080);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0003);
	CHECK_ERR(err);

	do {
		err = s5c73m3_read(sd, 0x0009, 0x5080, &stream_status);
		if (stream_status == 0xffff)
			break;

		index++;
		msleep(20);
	} while (index < 30);

	return err;
}

static int s5c73m3_set_hdr(struct v4l2_subdev *sd, int val)
{
	struct s5c73m3_state *state = to_state(sd);
	int err = 0;
	state->hdr_mode = val;

	if (val) {
		/* change PictureSize & frame rate */
		err = s5c73m3_change_auto_braket_mode(sd);
		CHECK_ERR(err);
		/* Sensor_Off */
		err = s5c73m3_s_stream_sensor(sd, 0);
		CHECK_ERR(err);
		err = s5c73m3_writeb(sd, S5C73M3_AE_AUTO_BRAKET,
				S5C73M3_AE_AUTO_BRAKET_EV10);
		CHECK_ERR(err);
	}
	return err;
}

static int s5c73m3_check_dataline(struct v4l2_subdev *sd, int val)
{
	return 0;
}

static int s5c73m3_check_esd(struct v4l2_subdev *sd)
{
	return 0;
}

static int s5c73m3_set_face_zoom(struct v4l2_subdev *sd, int val)
{
	struct s5c73m3_state *state = to_state(sd);
	int err;

	cam_dbg("s5c73m3_set_face_zoom\n");
	cam_dbg("Touch Position(%d,%d)\n",
		state->focus.pos_x, state->focus.pos_y);
	cam_dbg("Preview Size(%d,%d)\n",
		state->preview->width, state->preview->height);

	err = s5c73m3_i2c_write(sd, 0xfcfc, 0x3310);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0050, 0x0009);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0054, S5C73M3_AF_TOUCH_POSITION);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, state->focus.pos_x);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, state->focus.pos_y);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, state->preview->width);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, state->preview->height);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0050, 0x0009);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0054, 0x5000);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, S5C73M3_AF_FACE_ZOOM);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, val); /*0:reset, 1:Start*/
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0054, 0x5080);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0001);
	CHECK_ERR(err);

	return 0;
}


static int s5c73m3_set_face_detection(struct v4l2_subdev *sd, int val)
{
	int err;
	cam_dbg("E, value %d\n", val);

retry:
	switch (val) {
	case FACE_DETECTION_ON:
		err = s5c73m3_writeb(sd, S5C73M3_FACE_DET,
			S5C73M3_FACE_DET_ON);
		CHECK_ERR(err);

		err = s5c73m3_writeb(sd, S5C73M3_AF_MODE,
		S5C73M3_AF_MODE_PREVIEW_CAF_START);
		CHECK_ERR(err);

		break;

	case FACE_DETECTION_OFF:
		err = s5c73m3_writeb(sd, S5C73M3_FACE_DET,
			S5C73M3_FACE_DET_OFF);
		CHECK_ERR(err);
		break;

	default:
		cam_warn("invalid value, %d\n", val);
		val = FACE_DETECTION_OFF;
		goto retry;
	}

	cam_trace("X\n");
	return 0;

}

static int s5c73m3_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct s5c73m3_state *state = to_state(sd);
	int err = 0;

	if (unlikely(state->isp.bad_fw && ctrl->id != V4L2_CID_CAM_UPDATE_FW)) {
		cam_err("\"Unknown\" state, please update F/W");
		return -ENOSYS;
	}

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_FACE_DETECTION:
		err = s5c73m3_set_face_detection(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FACE_ZOOM:
		err = s5c73m3_set_face_zoom(sd, ctrl->value);
		break;

	case V4L2_CID_CAM_UPDATE_FW:
		if (ctrl->value == FW_MODE_DUMP)
			err = s5c73m3_dump_fw(sd);
		else
			err = s5c73m3_check_fw(sd);
		break;

	case V4L2_CID_CAMERA_SENSOR_MODE:
		err = s5c73m3_set_sensor_mode(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_FLASH_MODE:
		err = s5c73m3_set_flash(sd, ctrl->value, 0);
		break;

	case V4L2_CID_CAMERA_ISO:
		err = s5c73m3_set_iso(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_METERING:
		if (state->sensor_mode == SENSOR_CAMERA)
			err = s5c73m3_set_metering(sd, ctrl->value);
		break;

	case V4L2_CID_EXPOSURE:
		err = s5c73m3_set_exposure(sd, ctrl);
		break;

	case V4L2_CID_WHITE_BALANCE_PRESET:
		err = s5c73m3_set_whitebalance(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_SCENE_MODE:
		err = s5c73m3_set_scene_mode(sd, ctrl->value);
		break;

	case V4L2_CID_COLORFX:
		err = s5c73m3_set_effect(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_WDR:
		err = s5c73m3_set_wdr(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_ANTI_SHAKE:
		err = s5c73m3_set_antishake(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_BEAUTY_SHOT:
		err = s5c73m3_set_face_beauty(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_DEFAULT_FOCUS_POSITION:
		/*err = s5c73m3_set_af_mode(sd, state->focus.mode);*/
		err = 0;
		break;

	case V4L2_CID_FOCUS_AUTO_MODE:
		/*state->focus.mode = ctrl->value;*/
		err = s5c73m3_set_af_mode(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_SET_AUTO_FOCUS:
		/* Focus off */
		if (!ctrl->value) {
			if (state->focus.focusing != true)
				break;

			err = s5c73m3_set_af(sd, 0);
			if (!err)
				state->focus.focusing = false;
			break;
		}

		/* Focus on */
		if (state->focus.focusing == false) {
			err = s5c73m3_set_af(sd, 1);
			if (!err && !(is_focus(V4L2_FOCUS_AUTO_CONTINUOUS)))
				return -EBUSY;
		} else {
			err = s5c73m3_get_af(sd);
			if (err)
				return err;
		}
		break;

	case V4L2_CID_FOCUS_AUTO_RECTANGLE_LEFT:
		state->focus.pos_x = ctrl->value;
		break;

	case V4L2_CID_FOCUS_AUTO_RECTANGLE_TOP:
		state->focus.pos_y = ctrl->value;
		break;

	case V4L2_CID_CAMERA_ZOOM:
		err = s5c73m3_set_zoom(sd, ctrl);
		break;

	case V4L2_CID_CAM_JPEG_QUALITY:
		err = s5c73m3_set_jpeg_quality(sd, ctrl);
		break;

	case V4L2_CID_CAMERA_CAPTURE:
		err = s5c73m3_start_capture(sd, ctrl->value);
		break;

	case V4L2_CID_CAMERA_HDR:
		state->hdr_mode = ctrl->value;
		err = 0;
		break;

	case V4L2_CID_CAMERA_VT_MODE:
		state->vt_mode = ctrl->value;
		break;

	case V4L2_CID_CAMERA_CHECK_DATALINE:
		state->check_dataline = ctrl->value;
		break;

	case V4L2_CID_CAMERA_CHECK_ESD:
		err = s5c73m3_check_esd(sd);
		break;

	case V4L2_CID_CAMERA_JPEG_RESOLUTION:
		state->jpeg_width = (u32)ctrl->value >> 16;
		state->jpeg_height = (u32)ctrl->value & 0x0FFFF;
		break;
	case V4L2_CID_CAMERA_AEAWB_LOCK_UNLOCK:
		err = s5c73m3_aeawb_lock_unlock(sd, ctrl->value);
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

static int s5c73m3_g_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct s5c73m3_state *state = to_state(sd);
	int err = 0;

	switch (ctrl->id) {
	case V4L2_CID_CAMERA_AUTO_FOCUS_RESULT:
		err = s5c73m3_get_af(sd);
		break;

	case V4L2_CID_CAM_JPEG_MEMSIZE:
		ctrl->value = S5C73M3_JPEG_MAXSIZE +
			S5C73M3_YUV_MAXSIZE + S5C73M3_POINTER_MAXSIZE;
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

	case V4L2_CID_CAMERA_ISO:
		ctrl->value = state->exif.iso;
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


static int s5c73m3_g_ext_ctrl(struct v4l2_subdev *sd,
	struct v4l2_ext_control *ctrl)
{
	struct s5c73m3_state *state = to_state(sd);
	int err = 0;

	switch (ctrl->id) {
	case V4L2_CID_CAM_SENSOR_FW_VER:
		strcpy(ctrl->string, state->exif.unique_id);
		break;

	default:
		cam_err("no such control id %d\n",
			ctrl->id - V4L2_CID_CAMERA_CLASS_BASE);
		/*err = -ENOIOCTLCMD*/
		err = 0;
		break;
	}

	if (err < 0 && err != -ENOIOCTLCMD)
		cam_err("failed, id %d\n",
			ctrl->id - V4L2_CID_CAMERA_CLASS_BASE);

	return err;
}

static int s5c73m3_g_ext_ctrls(struct v4l2_subdev *sd,
	struct v4l2_ext_controls *ctrls)
{
	struct v4l2_ext_control *ctrl = ctrls->controls;
	int i, err = 0;

	for (i = 0; i < ctrls->count; i++, ctrl++) {
		err = s5c73m3_g_ext_ctrl(sd, ctrl);
		if (err) {
			ctrls->error_idx = i;
			break;
		}
	}
	return err;
}


static int s5c73m3_program_fw(struct v4l2_subdev *sd,
	u8 *buf, u32 addr, u32 unit, u32 count, u8 id)
{
	return 0;
}

#ifndef CONFIG_VIDEO_S5C73M3_SPI
int s5c73m3_spi_write(const u8 *addr, const int len, const int txSize)
{ return 0; }
#endif

static int s5c73m3_load_fw(struct v4l2_subdev *sd)
{
	struct device *dev = sd->v4l2_dev->dev;
	struct s5c73m3_state *state = to_state(sd);
	const struct firmware *fw;
	u8 *buf = NULL;
	int err, txSize;

	struct file *fp;
	mm_segment_t old_fs;
	long fsize, nread;
	int fw_requested = 1;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	if ((state->sensor_fw[0] == 'O') && (state->sensor_fw[1] == 'D'))
		fp = filp_open(S5C73M3_FW_OD_PATH, O_RDONLY, 0);
	else if ((state->sensor_fw[0] == 'O') && (state->sensor_fw[1] == 'C'))
		fp = filp_open(S5C73M3_FW_PATH, O_RDONLY, 0);
	else if ((state->sensor_fw[0] == 'S') && (state->sensor_fw[1] == 'D'))
		fp = filp_open(S5C73M3_FW_SD_PATH, O_RDONLY, 0);
	else if ((state->sensor_fw[0] == 'S') && (state->sensor_fw[1] == 'C'))
		fp = filp_open(S5C73M3_FW_SC_PATH, O_RDONLY, 0);
	else
		fp = filp_open(S5C73M3_FW_PATH, O_RDONLY, 0);

	if (IS_ERR(fp)) {
		cam_trace("failed to open %s, err %ld\n",
			S5C73M3_FW_PATH, PTR_ERR(fp));
		goto request_fw;
	}

	fw_requested = 0;
	fsize = fp->f_path.dentry->d_inode->i_size;

	buf = vmalloc(fsize);
	if (!buf) {
		cam_err("failed to allocate memory\n");
		err = -ENOMEM;
		goto out;
	}

	nread = vfs_read(fp, (char __user *)buf, fsize, &fp->f_pos);
	if (nread != fsize) {
		cam_err("failed to read firmware file, %ld Bytes\n", nread);
		err = -EIO;
		goto out;
	}

request_fw:
	if (fw_requested) {
		set_fs(old_fs);

		if ((state->sensor_fw[0] == 'O')
			&& (state->sensor_fw[1] == 'D'))
			err = request_firmware(&fw,
				S5C73M3_FW_REQUEST_OD_PATH,
				dev);
		else if ((state->sensor_fw[0] == 'O')
			&& (state->sensor_fw[1] == 'C'))
			err = request_firmware(&fw,
				S5C73M3_FW_REQUEST_PATH,
				dev);
		else if ((state->sensor_fw[0] == 'S')
			&& (state->sensor_fw[1] == 'D'))
			err = request_firmware(&fw,
				S5C73M3_FW_REQUEST_SD_PATH,
				dev);
		else if ((state->sensor_fw[0] == 'S')
			&& (state->sensor_fw[1] == 'C'))
			err = request_firmware(&fw,
				S5C73M3_FW_REQUEST_SC_PATH,
				dev);
		else
			err = request_firmware(&fw,
				S5C73M3_FW_REQUEST_PATH,
				dev);

		if (err != 0) {
			cam_err("request_firmware falied\n");
			err = -EINVAL;
			goto out;
		}

		cam_dbg("start, size %d Bytes\n", fw->size);
		buf = (u8 *)fw->data;
		fsize = fw->size;
	}

	if (fw_requested)
		txSize = 60*1024; /*60KB*/
	else
		txSize = 64; /*64 byte for non-DMA mode*/

	err = s5c73m3_spi_write(buf, fsize, txSize);
	if (err < 0) {
		cam_err("s5c73m3_spi_write falied\n");
		goto out;
	}

	cam_dbg("end\n");

out:
	if (!fw_requested) {
		vfree(buf);

		filp_close(fp, current->files);
		set_fs(old_fs);
	} else {
		release_firmware(fw);
	}

	return err;
}

/*
 * v4l2_subdev_video_ops
 */
static const struct s5c73m3_frmsizeenum *s5c73m3_get_frmsize
	(const struct s5c73m3_frmsizeenum *frmsizes, int num_entries, int index)
{
	int i;

	for (i = 0; i < num_entries; i++) {
		if (frmsizes[i].index == index)
			return &frmsizes[i];
	}

	return NULL;
}

static int s5c73m3_set_frmsize(struct v4l2_subdev *sd)
{
	struct s5c73m3_state *state = to_state(sd);
	struct v4l2_control ctrl;
	int err ;
	cam_trace("E\n");

#ifdef SEPERATE_RESOLUTION_SETTING
	if (state->format_mode != V4L2_PIX_FMT_MODE_CAPTURE) {
		err = s5c73m3_writeb(sd, S5C73M3_CHG_MODE,
			S5C73M3_YUV_MODE | state->preview->reg_val);
		CHECK_ERR(err);

		cam_dbg("yuv frame size %dx%d\n",
			state->preview->width, state->preview->height);
	} else {
		err = s5c73m3_writeb(sd, S5C73M3_CHG_MODE,
			S5C73M3_INTERLEAVED_MODE
			| state->capture->reg_val | state->preview->reg_val);
		CHECK_ERR(err);

		cam_dbg("interleaved yuv size %dx%d\n",
			state->preview->width, state->preview->height);

		cam_dbg("interleaved jpeg size %dx%d\n",
			state->capture->width, state->capture->height);
	}
#else
	if (state->format_mode != V4L2_PIX_FMT_MODE_CAPTURE) {
		S5C73M3_YUV_PREVIEW[9] = (S5C73M3_YUV_PREVIEW[9] & 0xFFFFFFF0)
			| state->preview->reg_val;

		cam_dbg("yuv frame size %dx%d\n",
			state->preview->width, state->preview->height);
	} else {
		S5C73M3_INTERLEAVED_PREVIEW[9] =
			(S5C73M3_INTERLEAVED_PREVIEW[9] & 0xFFFFFF00)
			| state->capture->reg_val | state->preview->reg_val;

		cam_dbg("interleaved yuv size %dx%d\n",
			state->preview->width, state->preview->height);

		cam_dbg("interleaved jpeg size %dx%d\n",
			state->capture->width, state->capture->height);
	}
#endif

	if (state->zoom) {
		ctrl.id = V4L2_CID_CAMERA_ZOOM;
		ctrl.value = state->zoom;
		s5c73m3_set_zoom(sd, &ctrl);
	}

	cam_trace("X\n");
	return 0;
}

static int s5c73m3_s_fmt(struct v4l2_subdev *sd,
	struct v4l2_mbus_framefmt *ffmt)
{
	struct s5c73m3_state *state = to_state(sd);
	const struct s5c73m3_frmsizeenum **frmsize;
	const struct s5c73m3_frmsizeenum **capfrmsize;

	u32 width = ffmt->width;
	u32 height = ffmt->height;
	u32 tmp_width;
	u32 old_index, old_index_cap;
	int i, num_entries;
	cam_trace("E\n");

	if (unlikely(state->isp.bad_fw)) {
		cam_err("\"Unknown\" state, please update F/W");
		return -ENOSYS;
	}
	if (ffmt->width < ffmt->height) {
		tmp_width = ffmt->height;
		height = ffmt->width;
		width = tmp_width;
	}

	if (ffmt->colorspace == V4L2_COLORSPACE_JPEG)
		state->format_mode = V4L2_PIX_FMT_MODE_CAPTURE;
	else
		state->format_mode = V4L2_PIX_FMT_MODE_PREVIEW;

	s5c73m3_set_mode(sd);

	/*set frame size for preview(yuv)*/
	frmsize = &state->preview;
	old_index = *frmsize ? (*frmsize)->index : -1;
	*frmsize = NULL;

	num_entries = ARRAY_SIZE(preview_frmsizes);
	for (i = 0; i < num_entries; i++) {
		if (width == preview_frmsizes[i].width &&
			height == preview_frmsizes[i].height) {
			*frmsize = &preview_frmsizes[i];
			break;
		}
	}

	if (*frmsize == NULL) {
		cam_warn("invalid yuv frame size %dx%d\n", width, height);
		if (state->hdr_mode) {
			num_entries = ARRAY_SIZE(capture_frmsizes);
			for (i = 0; i < num_entries; i++) {
				if (width == capture_frmsizes[i].width &&
					height == capture_frmsizes[i].height) {
					*frmsize = &capture_frmsizes[i];
					break;
				}
			}
		} else {
			*frmsize = s5c73m3_get_frmsize(preview_frmsizes,
					num_entries,
					S5C73M3_PREVIEW_960X720);
		}
	}

	/*set frame size for capture(jpeg)*/
	/*it's meaningful for interleaved mode*/
	capfrmsize = &state->capture;
	old_index_cap = *capfrmsize ? (*capfrmsize)->index : -1;
	*capfrmsize = NULL;

	width = state->jpeg_width;
	height = state->jpeg_height;

	num_entries = ARRAY_SIZE(capture_frmsizes);
	for (i = 0; i < num_entries; i++) {
		if (width == capture_frmsizes[i].width &&
			height == capture_frmsizes[i].height) {
			*capfrmsize = &capture_frmsizes[i];
			break;
		}
	}

	if (*capfrmsize == NULL) {
		cam_warn("invalid jpeg frame size %dx%d\n", width, height);
		*capfrmsize = s5c73m3_get_frmsize(capture_frmsizes, num_entries,
				S5C73M3_CAPTURE_VGA);
	}

	cam_dbg("yuv %dx%d\n", (*frmsize)->width, (*frmsize)->height);
	cam_dbg("jpeg %dx%d\n", (*capfrmsize)->width, (*capfrmsize)->height);
	if (state->stream_enable) {
		if (ffmt->colorspace == V4L2_COLORSPACE_JPEG) {
			if ((old_index != (*frmsize)->index)
				|| (old_index_cap != (*capfrmsize)->index))
				s5c73m3_set_frmsize(sd);
		} else {
			if (old_index != (*frmsize)->index)
				s5c73m3_set_frmsize(sd);
		}
	} else
		s5c73m3_set_frmsize(sd);

	cam_trace("X\n");
	return 0;
}

static int s5c73m3_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *a)
{
	struct s5c73m3_state *state = to_state(sd);

	cam_info("g_parm: FPS %d\n", state->fps);
	if (state->fps == 0) {
		a->parm.capture.timeperframe.numerator = state->fps;
		a->parm.capture.timeperframe.denominator = 1;
	} else {
		a->parm.capture.timeperframe.numerator = 1;
		a->parm.capture.timeperframe.denominator = state->fps;
	}

	return 0;
}

static int s5c73m3_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *a)
{
	struct s5c73m3_state *state = to_state(sd);
	u32 denom = a->parm.capture.timeperframe.denominator;
	u32 numer = a->parm.capture.timeperframe.numerator;
	u32 fps, idx = 0;

	if (denom >= 1 && numer == 1) {
		fps = denom / numer;

		/*
		 * supported fps : 8(7.5), 10, 12, 15, 20, 21, 22, 23, 24, 30
		 */
		if (fps > 0 && fps <= 9) {
			fps = 8;  state->fps_reg = 0x06;
		} else if (fps > 9 && fps <= 11) {
			fps = 10; state->fps_reg = 0x05;
		} else if (fps > 10 && fps <= 14) {
			fps = 12; state->fps_reg = 0x04;
		} else if (fps > 14 && fps <= 18) {
			fps = 15; state->fps_reg = 0x03;
		} else if (fps > 18 && fps <= 20) {
			fps = 20; state->fps_reg = 0x08;
		} else if (fps > 20 && fps <= 21) {
			fps = 21; state->fps_reg = 0x09;
		} else if (fps > 21 && fps <= 22) {
			fps = 22; state->fps_reg = 0x0A;
		} else if (fps > 22 && fps <= 23) {
			fps = 23; state->fps_reg = 0x0B;
		} else if (fps > 23 && fps <= 27) {
			fps = 24; state->fps_reg = 0x07;
		} else {
			fps = 30; state->fps_reg = 0x02;
		}

	} else if (denom == 1 && numer == 0) {
		fps = 0;
	} else
		return -EINVAL;		/* Wrong fps */

	if (state->fps != fps) {
		state->fps = fps;
		state->need_setfps = true;
	}

	cam_info("s_parm: denom %d, numer %d, FPS idx %08x\n",
			denom, numer, idx);

	return 0;
}

static int s5c73m3_enum_framesizes(struct v4l2_subdev *sd,
	struct v4l2_frmsizeenum *fsize)
{
	struct s5c73m3_state *state = to_state(sd);

	/*
	* The camera interface should read this value, this is the resolution
	* at which the sensor would provide framedata to the camera i/f
	* In case of image capture,
	* this returns the default camera resolution (VGA)
	*/
	if (state->preview == NULL)
		return -EINVAL;

	fsize->type = V4L2_FRMSIZE_TYPE_DISCRETE;
	fsize->discrete.width = state->preview->width;
	fsize->discrete.height = state->preview->height;

	return 0;
}

static int s5c73m3_s_stream_sensor(struct v4l2_subdev *sd, int onoff)
{
	int err = 0;
	int index = 0;
	u16 stream_status = 0;

	cam_info("%s::::onoff=%d\n", __func__, onoff);
	err = s5c73m3_writeb(sd, S5C73M3_SENSOR_STREAMING,
		onoff ? S5C73M3_SENSOR_STREAMING_ON :
		S5C73M3_SENSOR_STREAMING_OFF);
	CHECK_ERR(err);

	do {
		err = s5c73m3_read(sd, 0x0009, 0x5080, &stream_status);
		if (stream_status == 0xffff)
			break;

		index++;
		msleep(20);
	} while (index < 30);

	return err;
}

static int s5c73m3_s_stream_preview(struct v4l2_subdev *sd, int enable)
{
	int err = 0;

#ifndef SEPERATE_RESOLUTION_SETTING
	if (enable) {
		err = s5c73m3_i2c_write_block(sd, S5C73M3_YUV_PREVIEW,
			sizeof(S5C73M3_YUV_PREVIEW)/
			sizeof(S5C73M3_YUV_PREVIEW[0]));
		CHECK_ERR(err);
	}
#endif

	return err;
}

static int s5c73m3_s_stream_capture(struct v4l2_subdev *sd, int enable)
{
	int err = 0;

#ifndef SEPERATE_RESOLUTION_SETTING
	if (enable) {
		err = s5c73m3_i2c_write_block(sd, S5C73M3_INTERLEAVED_PREVIEW,
			sizeof(S5C73M3_INTERLEAVED_PREVIEW)/
			sizeof(S5C73M3_INTERLEAVED_PREVIEW[0]));
		CHECK_ERR(err);
	}
#endif

	return err;
}

static int s5c73m3_s_stream_hdr(struct v4l2_subdev *sd, int enable)
{
	struct s5c73m3_state *state = to_state(sd);
	int err = 0;
	cam_info("s_stream_hdr\n");

	err = s5c73m3_i2c_write(sd, 0x0050, 0x0009);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0054, 0x5000);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0902);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0008);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x091A);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0002);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0B10);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x8000 |
			state->capture->reg_val |
			state->preview->reg_val);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x090A);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0001);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0B14);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0200);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0054, 0x5080);
	CHECK_ERR(err);

	err = s5c73m3_i2c_write(sd, 0x0F14, 0x0005);
	CHECK_ERR(err);


#if 0
	/* change PictureSize & frame rate */
	err = s5c73m3_change_auto_braket_mode(sd);
	CHECK_ERR(err);
	err = s5c73m3_s_stream_sensor(sd, 1);
	CHECK_ERR(err);
	err = s5c73m3_writeb(sd, S5C73M3_AE_AUTO_BRAKET,
			S5C73M3_AE_AUTO_BRAKET_EV10);
	CHECK_ERR(err);
#endif
	return 0;
}

static int s5c73m3_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct s5c73m3_state *state = to_state(sd);
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
#ifndef SEPERATE_RESOLUTION_SETTING
			if (enable == STREAM_MODE_CAM_ON) {
				err = s5c73m3_s_stream_capture(sd,
					enable == STREAM_MODE_CAM_ON);
			} else {
				err = s5c73m3_writeb(sd,
					S5C73M3_SENSOR_STREAMING,
					S5C73M3_SENSOR_STREAMING_OFF);
			}
#else
			s5c73m3_s_stream_sensor(sd, enable);
#if 0
			if (enable == STREAM_MODE_CAM_ON &&
			state->focus.mode == FOCUS_MODE_CONTINOUS_VIDEO) {
				s5c73m3_set_af_mode(sd,
					FOCUS_MODE_CONTINOUS_VIDEO);
			}
#endif
#endif
			break;
		default:
			cam_info("preview %s",
				enable == STREAM_MODE_CAM_ON ? "on" : "off");
#ifndef SEPERATE_RESOLUTION_SETTING
			if (enable == STREAM_MODE_CAM_ON) {
				if (state->hdr_mode) {
					err = s5c73m3_s_stream_hdr(sd, enable);
				} else {
					err = s5c73m3_s_stream_preview(sd,
						enable == STREAM_MODE_CAM_ON);
				}
			} else {
				err = s5c73m3_writeb(sd,
					S5C73M3_SENSOR_STREAMING,
					S5C73M3_SENSOR_STREAMING_OFF);
			}
#else
			if (state->hdr_mode) {
				err = s5c73m3_s_stream_hdr(sd, enable);
			} else {
				err = s5c73m3_s_stream_sensor(sd, enable);
#if 0
				if (enable == STREAM_MODE_CAM_ON &&
				state->focus.mode ==
				FOCUS_MODE_CONTINOUS_VIDEO) {
					s5c73m3_set_af_mode(sd,
						FOCUS_MODE_CONTINOUS_VIDEO);
				}
#endif
			}
#endif
			break;
		}
		break;

	case STREAM_MODE_MOVIE_ON:
		if (state->flash_mode != FLASH_MODE_OFF)
			err = s5c73m3_set_flash(sd, state->flash_mode, 1);

		break;

	case STREAM_MODE_MOVIE_OFF:

		s5c73m3_set_flash(sd, FLASH_MODE_OFF, 1);
		break;

	default:
		cam_err("invalid stream option, %d\n", enable);
		break;
	}

#if 0
		err = s5c73m3_writeb(sd, S5C73M3_AF_CAL, 0);
		CHECK_ERR(err);
#endif
	state->stream_enable = enable;

	cam_trace("X\n");
	return 0;
}

static int s5c73m3_check_version(struct v4l2_subdev *sd)
{
	return 0;
}

static int s5c73m3_init_param(struct v4l2_subdev *sd)
{
	return 0;
}

static int s5c73m3_FROM_booting(struct v4l2_subdev *sd)
{
	u16 read_val;
	int i, err;

	cam_trace("E\n");

	/*ARM go*/
	err = s5c73m3_write(sd, 0x3000, 0x0004, 0xFFFF);
	CHECK_ERR(err);

	udelay(400);

	/*Check boot done*/
	for (i = 0; i < 4; i++) {
		err = s5c73m3_read(sd, 0x3010, 0x0010, &read_val);
		CHECK_ERR(err);

		if (read_val == 0x0C)
			break;

		udelay(100);
	}

	if (read_val != 0x0C) {
		cam_err("boot fail, read_val %#x\n", read_val);
		return -1;
	}

       /*P,M,S and Boot Mode*/
	err = s5c73m3_write(sd, 0x3100, 0x010C, 0x0044);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3100, 0x0108, 0x000D);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3100, 0x0304, 0x0001);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x0001, 0x0000, 0x5800);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x0001, 0x0002, 0x0002);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3100, 0x0000, 0x0001);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3010, 0x0014, 0x1B85);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3010, 0x0010, 0x230C);
	CHECK_ERR(err);

	mdelay(300);

	/*Check binary read done*/
	for (i = 0; i < 3; i++) {
		err = s5c73m3_read(sd, 0x3010, 0x0010, &read_val);
		CHECK_ERR(err);

		if (read_val == 0x230E)
			break;

		udelay(100);
	}

	if (read_val != 0x230E) {
		cam_err("binary read fail, read_val %#x\n", read_val);
		return -1;
	}

	/*ARM reset*/
	err = s5c73m3_write(sd, 0x3000, 0x0004, 0xFFFD);
	CHECK_ERR(err);

	/*remap*/
	err = s5c73m3_write(sd, 0x3010, 0x00A4, 0x0183);
	CHECK_ERR(err);

	/*ARM go again*/
	err = s5c73m3_write(sd, 0x3000, 0x0004, 0xFFFF);
	CHECK_ERR(err);

	cam_trace("X\n");

	return 0;
}

static int s5c73m3_SPI_booting(struct v4l2_subdev *sd)
{
	u16 read_val;
	int i, err;

	cam_trace("E\n");

	/*ARM go*/
	err = s5c73m3_write(sd, 0x3000, 0x0004, 0xFFFF);
	CHECK_ERR(err);

	udelay(400);

	/*Check boot done*/
	for (i = 0; i < 3; i++) {
		err = s5c73m3_read(sd, 0x3010, 0x0010, &read_val);
		CHECK_ERR(err);

		if (read_val == 0x0C)
			break;

		udelay(100);
	}

	if (read_val != 0x0C) {
		cam_err("boot fail, read_val %#x\n", read_val);
		return -1;
	}

       /*P,M,S and Boot Mode*/
	err = s5c73m3_write(sd, 0x3010, 0x0014, 0x2146);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3010, 0x0010, 0x210C);
	CHECK_ERR(err);

	udelay(200);

	/*Check SPI ready*/
	for (i = 0; i < 3; i++) {
		err = s5c73m3_read(sd, 0x3010, 0x0010, &read_val);
		CHECK_ERR(err);

		if (read_val == 0x210D)
			break;

		udelay(100);
	}

	if (read_val != 0x210D) {
		cam_err("SPI not ready, read_val %#x\n", read_val);
		return -1;
	}

	/*download fw by SPI*/
	s5c73m3_load_fw(sd);

	/*ARM reset*/
	err = s5c73m3_write(sd, 0x3000, 0x0004, 0xFFFD);
	CHECK_ERR(err);

	/*remap*/
	err = s5c73m3_write(sd, 0x3010, 0x00A4, 0x0183);
	CHECK_ERR(err);

	/*ARM go again*/
	err = s5c73m3_write(sd, 0x3000, 0x0004, 0xFFFF);
	CHECK_ERR(err);

	cam_trace("X\n");

	return 0;
}

static int s5c73m3_read_vdd_core(struct v4l2_subdev *sd)
{
	struct s5c73m3_state *state = to_state(sd);
	u16 read_val;
	int err;

	cam_trace("E\n");

	/*Initialize OTP Controller*/
#if 0
	err = s5c73m3_i2c_write_block(sd, S5C73M3_OTP_CONTROL,
		sizeof(S5C73M3_OTP_CONTROL)/
		sizeof(S5C73M3_OTP_CONTROL[0]));
	CHECK_ERR(err);
#else
	err = s5c73m3_write(sd, 0x3800, 0xA004, 0x0000);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3800, 0xA000, 0x0004);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3800, 0xA0D8, 0x0000);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3800, 0xA0DC, 0x0004);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3800, 0xA0C4, 0x4000);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3800, 0xA0D4, 0x0015);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3800, 0xA000, 0x0001);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3800, 0xA0B4, 0x9F90);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3800, 0xA09C, 0x9A95);
	CHECK_ERR(err);
#endif

	/*Page Select*/
#if 0
	err = s5c73m3_i2c_write_block(sd, S5C73M3_OTP_PAGE,
		sizeof(S5C73M3_OTP_PAGE)/
		sizeof(S5C73M3_OTP_PAGE[0]));
	CHECK_ERR(err);
#else
	err = s5c73m3_write(sd, 0x3800, 0xA0C4, 0x4800);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3800, 0xA0C4, 0x4400);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3800, 0xA0C4, 0x4200);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3800, 0xA004, 0x00C0);
	CHECK_ERR(err);
	err = s5c73m3_write(sd, 0x3800, 0xA000, 0x0001);
	CHECK_ERR(err);
#endif

#if 0 /*read_val should be 0x7383*/
	err = s5c73m3_read(sd, 0x0000, 0x131C, &read_val);
	CHECK_ERR(err);

	cam_dbg("read_val %#x\n", read_val);
#endif

	/*Read Data*/
	err = s5c73m3_read(sd, 0x3800, 0xA034, &read_val);
	CHECK_ERR(err);

	cam_dbg("read_val %#x\n", read_val);

	/*Read Data End*/
	err = s5c73m3_write(sd, 0x3800, 0xA000, 0x0000);
	CHECK_ERR(err);

	if (read_val & 0x200)
		state->pdata->set_vdd_core(1150000);
	else if (read_val & 0x800)
		state->pdata->set_vdd_core(1100000);
	else if (read_val & 0x2000)
		state->pdata->set_vdd_core(1050000);
	else if (read_val & 0x8000)
		state->pdata->set_vdd_core(1000000);
	else
		state->pdata->set_vdd_core(1150000);

	cam_trace("X\n");

	return 0;
}


static int s5c73m3_init(struct v4l2_subdev *sd, u32 val)
{
	struct s5c73m3_state *state = to_state(sd);
	int err;

	sd_internal = sd;

	/* Default state values */
	state->isp.bad_fw = 0;

	state->preview = NULL;
	state->capture = NULL;

	state->format_mode = V4L2_PIX_FMT_MODE_PREVIEW;
	state->sensor_mode = SENSOR_CAMERA;
	state->flash_mode = FLASH_MODE_OFF;
	state->beauty_mode = 0;
	state->focus.mode = FOCUS_MODE_CONTINOUS_PICTURE;
	state->focus.touch = 0;

	state->fps = 0;			/* auto */

	memset(&state->focus, 0, sizeof(state->focus));

#ifndef CONFIG_MACH_MIDAS_02_BD
	if (!state->pdata->is_vdd_core_set())
		s5c73m3_read_vdd_core(sd);
#endif

#ifdef S5C73M3_FROM_BOOTING
	err = s5c73m3_FROM_booting(sd);
#else
	err = s5c73m3_check_fw(sd);
	/*err = s5c73m3_SPI_booting(sd);*/
#endif
	CHECK_ERR(err);

	s5c73m3_init_param(sd);

	return 0;
}

static const struct v4l2_subdev_core_ops s5c73m3_core_ops = {
	.init = s5c73m3_init,		/* initializing API */
	.load_fw = s5c73m3_load_fw,
	.queryctrl = s5c73m3_queryctrl,
	.g_ctrl = s5c73m3_g_ctrl,
	.s_ctrl = s5c73m3_s_ctrl,
	.g_ext_ctrls = s5c73m3_g_ext_ctrls,
};

static const struct v4l2_subdev_video_ops s5c73m3_video_ops = {
	.s_mbus_fmt = s5c73m3_s_fmt,
	.g_parm = s5c73m3_g_parm,
	.s_parm = s5c73m3_s_parm,
	.enum_framesizes = s5c73m3_enum_framesizes,
	.s_stream = s5c73m3_s_stream,
};

static const struct v4l2_subdev_ops s5c73m3_ops = {
	.core = &s5c73m3_core_ops,
	.video = &s5c73m3_video_ops,
};

static ssize_t s5c73m3_camera_rear_camtype_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct s5c73m3_state *state = dev_get_drvdata(dev);
	char type[25];

	if (state->sensor_fw[1] == 'C') {
		strcpy(type, "SLSI_S5K3H7_S5C73M3");
	} else if (state->sensor_fw[1] == 'D') {
		strcpy(type, "SONY_IMX157_S5C73M3");
	} else {
		cam_warn("cannot find the matched camera type\n");
		strcpy(type, "UNKOWN CameraType");
	}

	return sprintf(buf, "%s\n", type);
}

static ssize_t s5c73m3_camera_rear_camfw_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct s5c73m3_state *state = dev_get_drvdata(dev);

	return sprintf(buf, "%s %s\n", state->sensor_fw, state->phone_fw);
}

static ssize_t s5c73m3_camera_rear_flash(struct device *dev,
	struct device_attribute *attr, const char *buf,
	size_t count)
{
	int err;

	if (buf[0] == '0')
		err = s5c73m3_writeb(sd_internal, S5C73M3_FLASH_TORCH,
			S5C73M3_FLASH_TORCH_OFF);
	else
		err = s5c73m3_writeb(sd_internal, S5C73M3_FLASH_TORCH,
			S5C73M3_FLASH_TORCH_ON);

	CHECK_ERR(err);

	return count;
}

static DEVICE_ATTR(rear_camtype, S_IRUGO,
		s5c73m3_camera_rear_camtype_show, NULL);
static DEVICE_ATTR(rear_camfw, S_IRUGO, s5c73m3_camera_rear_camfw_show, NULL);
static DEVICE_ATTR(rear_flash, S_IWUSR | S_IWGRP, NULL,
		s5c73m3_camera_rear_flash);

/*
 * s5c73m3_probe
 * Fetching platform data is being done with s_config subdev call.
 * In probe routine, we just register subdev device
 */
static int __devinit s5c73m3_probe(struct i2c_client *client,
			 const struct i2c_device_id *id)
{
	struct s5c73m3_state *state;
	struct v4l2_subdev *sd;

	state = kzalloc(sizeof(struct s5c73m3_state), GFP_KERNEL);
	if (state == NULL)
		return -ENOMEM;

	sd = &state->sd;
	strcpy(sd->name, S5C73M3_DRIVER_NAME);

	state->pdata = client->dev.platform_data;

	/* Registering subdev */
	v4l2_i2c_subdev_init(sd, client, &s5c73m3_ops);

#ifdef CAM_DEBUG
	state->dbg_level = CAM_DEBUG | CAM_TRACE;
#endif

#ifdef CONFIG_BUSFREQ_OPP
	/* lock bus frequency */
	dev_lock(bus_dev, s5c73m3_dev, 160160);
#endif

	if (s5c73m3_dev)
		dev_set_drvdata(s5c73m3_dev, state);

	printk(KERN_DEBUG "%s\n", __func__);

	return 0;
}

static int __devexit s5c73m3_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct s5c73m3_state *state = to_state(sd);

	if (s5c73m3_set_af_softlanding(sd) < 0)
		cam_err("failed to set soft landing\n");
	v4l2_device_unregister_subdev(sd);

#ifdef CONFIG_BUSFREQ_OPP
	/* Unlock bus frequency */
	dev_unlock(bus_dev, s5c73m3_dev);
#endif

	kfree(state->fw_version);
	kfree(state);

	return 0;
}

static const struct i2c_device_id s5c73m3_id[] = {
	{ S5C73M3_DRIVER_NAME, 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, s5c73m3_id);

static struct i2c_driver s5c73m3_i2c_driver = {
	.driver = {
		.name	= S5C73M3_DRIVER_NAME,
	},
	.probe		= s5c73m3_probe,
	.remove		= __devexit_p(s5c73m3_remove),
	.id_table	= s5c73m3_id,
};

static int __init s5c73m3_mod_init(void)
{
#ifdef CONFIG_BUSFREQ_OPP
	/* To lock bus frequency in OPP mode */
	bus_dev = dev_get("exynos-busfreq");
#endif

	if (!s5c73m3_dev) {
		s5c73m3_dev = device_create(camera_class,
				NULL, 0, NULL, "rear");
		if (IS_ERR(s5c73m3_dev)) {
			cam_warn("failed to create device!\n");
			return 0;
		}

		if (device_create_file(s5c73m3_dev, &dev_attr_rear_camtype)
				< 0) {
			cam_warn("failed to create device file, %s\n",
					dev_attr_rear_camtype.attr.name);
		}

		if (device_create_file(s5c73m3_dev, &dev_attr_rear_camfw) < 0) {
			cam_warn("failed to create device file, %s\n",
					dev_attr_rear_camfw.attr.name);
		}

		if (device_create_file(s5c73m3_dev, &dev_attr_rear_flash) < 0) {
			cam_warn("failed to create device file, %s\n",
					dev_attr_rear_flash.attr.name);
		}
	}

	return i2c_add_driver(&s5c73m3_i2c_driver);
}

static void __exit s5c73m3_mod_exit(void)
{
	i2c_del_driver(&s5c73m3_i2c_driver);
}
module_init(s5c73m3_mod_init);
module_exit(s5c73m3_mod_exit);


MODULE_DESCRIPTION("driver for LSI S5C73M3");
MODULE_LICENSE("GPL");
