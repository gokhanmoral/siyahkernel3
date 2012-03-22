/*
 *  STMicroelectronics lsm330dlc_accel acceleration sensor driver
 *
 *  Copyright (C) 2011 Samsung Electronics Co.Ltd
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/sensor/lsm330dlc_accel.h>
#include <linux/sensor/sensors_core.h>

/* For debugging */
#if 1
#define accel_dbgmsg(str, args...)\
	pr_info("%s: " str, __func__, ##args)
#else
#define accel_dbgmsg(str, args...)\
	pr_debug("%s: " str, __func__, ##args)
#endif

#undef LSM330DLC_ACCEL_LOGGING
#undef DEBUG_ODR

/* The default settings when sensor is on is for all 3 axis to be enabled
 * and output data rate set to 400Hz.  Output is via a ioctl read call.
 */
#define DEFAULT_POWER_ON_SETTING (ODR400 | ENABLE_ALL_AXES)

#ifdef USES_MOVEMETNT_RECOGNITION
#define DEFAULT_CTRL3_SETTING 0x40 /* INT1_A enable */
#define DEFAULT_INTERRUPT_SETTING 0x0A /* XH,YH : enable */
#define FACTORY_INTERRUPT_SETTING 0x2A /* XH,YH, ZH : enable */
#define MOVEMENT_THRESHOLD 0x19 /* 16 *25 = 400mg => 25 = 0x19 */
enum {
	OFF = 0,
	ON = 1
};
#define ACC_INTERRUPT_FILE_PATH	"/data/log/acc_int"
#endif

#define ACC_DEV_NAME "accelerometer"

#define CALIBRATION_FILE_PATH	"/efs/calibration_data"
#define CAL_DATA_AMOUNT	20

static const struct odr_delay {
	u8 odr; /* odr reg setting */
	s64 delay_ns; /* odr in ns */
} odr_delay_table[] = {
	{ ODR1344,     744047LL }, /* 1344Hz */
	{  ODR400,    2500000LL }, /*  400Hz */
	{  ODR200,    5000000LL }, /*  200Hz */
	{  ODR100,   10000000LL }, /*  100Hz */
	{   ODR50,   20000000LL }, /*   50Hz */
	{   ODR25,   40000000LL }, /*   25Hz */
	{   ODR10,  100000000LL }, /*   10Hz */
	{    ODR1, 1000000000LL }, /*    1Hz */
};

struct lsm330dlc_accel_data {
	struct i2c_client *client;
	struct miscdevice lsm330dlc_accel_device;
	struct mutex read_lock;
	struct mutex write_lock;
	struct lsm330dlc_acc cal_data;
	struct device *dev;
	u8 ctrl_reg1_shadow;
	atomic_t opened; /* opened implies enabled */
	#ifdef USES_MOVEMETNT_RECOGNITION
	int movement_recog_flag;
	unsigned char interrupt_state;
	#endif
};

 /* Read X,Y and Z-axis acceleration raw data */
static int lsm330dlc_accel_read_raw_xyz(struct lsm330dlc_accel_data *data,
				struct lsm330dlc_acc *acc)
{
	int err;
	s8 reg = OUT_X_L | AC; /* read from OUT_X_L to OUT_Z_H by auto-inc */
	u8 acc_data[6];

	err = i2c_smbus_read_i2c_block_data(data->client, reg,
					    sizeof(acc_data), acc_data);
	if (err != sizeof(acc_data)) {
		pr_err("%s : failed to read 6 bytes for getting x/y/z\n",
		       __func__);
		return -EIO;
	}

	acc->x = (acc_data[1] << 8) | acc_data[0];
	acc->y = (acc_data[3] << 8) | acc_data[2];
	acc->z = (acc_data[5] << 8) | acc_data[4];

	acc->x = acc->x >> 4;
	acc->y = acc->y >> 4;
	acc->z = acc->z >> 4;

	return 0;
}

static int lsm330dlc_accel_read_xyz(struct lsm330dlc_accel_data *data,
				struct lsm330dlc_acc *acc)
{
	int err = 0;

	mutex_lock(&data->read_lock);
	err = lsm330dlc_accel_read_raw_xyz(data, acc);
	mutex_unlock(&data->read_lock);
	if (err < 0) {
		pr_err("%s: lsm330dlc_accel_read_xyz() failed\n", __func__);
		return err;
	}

	acc->x -= data->cal_data.x;
	acc->y -= data->cal_data.y;
	acc->z -= data->cal_data.z;

	return err;
}

static int lsm330dlc_accel_open_calibration(struct lsm330dlc_accel_data *data)
{
	struct file *cal_filp = NULL;
	int err = 0;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(CALIBRATION_FILE_PATH, O_RDONLY, 0666);
	if (IS_ERR(cal_filp)) {
		err = PTR_ERR(cal_filp);
		if (err != -ENOENT)
			pr_err("%s: Can't open calibration file\n", __func__);
		set_fs(old_fs);
		return err;
	}

	err = cal_filp->f_op->read(cal_filp,
		(char *)&data->cal_data, 3 * sizeof(s16), &cal_filp->f_pos);
	if (err != 3 * sizeof(s16)) {
		pr_err("%s: Can't read the cal data from file\n", __func__);
		err = -EIO;
	}

	accel_dbgmsg("(%u,%u,%u)\n",
		data->cal_data.x, data->cal_data.y, data->cal_data.z);

	filp_close(cal_filp, current->files);
	set_fs(old_fs);

	return err;
}

static int lsm330dlc_accel_do_calibrate(struct device *dev, bool do_calib)
{
	struct lsm330dlc_accel_data *acc_data = dev_get_drvdata(dev);
	struct lsm330dlc_acc data = { 0, };
	struct file *cal_filp = NULL;
	int sum[3] = { 0, };
	int err = 0;
	int i;
	mm_segment_t old_fs;

	if (do_calib) {
		for (i = 0; i < CAL_DATA_AMOUNT; i++) {
			mutex_lock(&acc_data->read_lock);
			err = lsm330dlc_accel_read_raw_xyz(acc_data, &data);
			mutex_unlock(&acc_data->read_lock);
			if (err < 0) {
				pr_err("%s: lsm330dlc_accel_read_raw_xyz() "
					"failed in the %dth loop\n",
					__func__, i);
				return err;
			}

			sum[0] += data.x;
			sum[1] += data.y;
			sum[2] += data.z;
		}

		acc_data->cal_data.x = sum[0] / CAL_DATA_AMOUNT;
		acc_data->cal_data.y = sum[1] / CAL_DATA_AMOUNT;
		acc_data->cal_data.z = (sum[2] / CAL_DATA_AMOUNT) - 1024;
	} else {
		acc_data->cal_data.x = 0;
		acc_data->cal_data.y = 0;
		acc_data->cal_data.z = 0;
	}

	printk(KERN_INFO "%s: cal data (%d,%d,%d)\n", __func__,
			acc_data->cal_data.x, acc_data->cal_data.y
			, acc_data->cal_data.z);

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	cal_filp = filp_open(CALIBRATION_FILE_PATH,
			O_CREAT | O_TRUNC | O_WRONLY, 0666);
	if (IS_ERR(cal_filp)) {
		pr_err("%s: Can't open calibration file\n", __func__);
		set_fs(old_fs);
		err = PTR_ERR(cal_filp);
		return err;
	}

	err = cal_filp->f_op->write(cal_filp,
		(char *)&acc_data->cal_data, 3 * sizeof(s16), &cal_filp->f_pos);
	if (err != 3 * sizeof(s16)) {
		pr_err("%s: Can't write the cal data to file\n", __func__);
		err = -EIO;
	}

	filp_close(cal_filp, current->files);
	set_fs(old_fs);

	return err;
}

/*  open command for lsm330dlc_accel device file  */
static int lsm330dlc_accel_open(struct inode *inode, struct file *file)
{
	int err = 0;
	struct lsm330dlc_accel_data *data = container_of(file->private_data,
						struct lsm330dlc_accel_data,
						lsm330dlc_accel_device);

	accel_dbgmsg("is called.\n");
	mutex_lock(&data->write_lock);
	if (atomic_read(&data->opened) == 0) {
		err = lsm330dlc_accel_open_calibration(data);
		if (err < 0 && err != -ENOENT)
			pr_err("%s: lsm330dlc_accel_open_calibration() failed\n",
				__func__);
		data->ctrl_reg1_shadow = DEFAULT_POWER_ON_SETTING;
		err = i2c_smbus_write_byte_data(data->client, CTRL_REG1,
						DEFAULT_POWER_ON_SETTING);
		if (err)
			pr_err("%s: i2c write ctrl_reg1 failed\n", __func__);

		err = i2c_smbus_write_byte_data(data->client, CTRL_REG4,
						CTRL_REG4_HR);
		if (err)
			pr_err("%s: i2c write ctrl_reg4 failed\n", __func__);
	}

	atomic_add(1, &data->opened);
	mutex_unlock(&data->write_lock);

	return err;
}

/*  release command for lsm330dlc_accel device file */
static int lsm330dlc_accel_close(struct inode *inode, struct file *file)
{
	int err = 0;
	struct lsm330dlc_accel_data *data = container_of(file->private_data,
						struct lsm330dlc_accel_data,
						lsm330dlc_accel_device);

	accel_dbgmsg("is called.\n");
	mutex_lock(&data->write_lock);
	atomic_sub(1, &data->opened);

#ifdef USES_MOVEMETNT_RECOGNITION
	if (data->movement_recog_flag == ON) {
		accel_dbgmsg("LOW_PWR_MODE.\n");
		err = i2c_smbus_write_byte_data(data->client,
						CTRL_REG1, LOW_PWR_MODE);
		if (atomic_read(&data->opened) == 0)
			data->ctrl_reg1_shadow = PM_OFF;
	} else if (atomic_read(&data->opened) == 0) {
#else
	if (atomic_read(&data->opened) == 0) {
#endif
		err = i2c_smbus_write_byte_data(data->client, CTRL_REG1,
							PM_OFF);
		data->ctrl_reg1_shadow = PM_OFF;
	}
	mutex_unlock(&data->write_lock);

	return err;
}

static s64 lsm330dlc_accel_get_delay(struct lsm330dlc_accel_data *data)
{
	int i;
	u8 odr;
	s64 delay = -1;

	odr = data->ctrl_reg1_shadow & ODR_MASK;
	for (i = 0; i < ARRAY_SIZE(odr_delay_table); i++) {
		if (odr == odr_delay_table[i].odr) {
			delay = odr_delay_table[i].delay_ns;
			break;
		}
	}
	return delay;
}

static int lsm330dlc_accel_set_delay(struct lsm330dlc_accel_data *data\
	, s64 delay_ns)
{
	int odr_value = ODR1;
	int res = 0;
	int i;
	/* round to the nearest delay that is less than
	 * the requested value (next highest freq)
	 */

	accel_dbgmsg("passed %lldns\n", delay_ns);
	for (i = 0; i < ARRAY_SIZE(odr_delay_table); i++) {
		if (delay_ns < odr_delay_table[i].delay_ns)
			break;
	}
	if (i > 0)
		i--;
	accel_dbgmsg("matched rate %lldns, odr = 0x%x\n",
			odr_delay_table[i].delay_ns,
			odr_delay_table[i].odr);
	odr_value = odr_delay_table[i].odr;
	delay_ns = odr_delay_table[i].delay_ns;
	mutex_lock(&data->write_lock);
	accel_dbgmsg("old = %lldns, new = %lldns\n",
		     lsm330dlc_accel_get_delay(data), delay_ns);
	if (odr_value != (data->ctrl_reg1_shadow & ODR_MASK)) {
		u8 ctrl = (data->ctrl_reg1_shadow & ~ODR_MASK);
		ctrl |= odr_value;
		data->ctrl_reg1_shadow = ctrl;
		res = i2c_smbus_write_byte_data(data->client, CTRL_REG1, ctrl);
		accel_dbgmsg("writing odr value 0x%x\n", odr_value);
	}
	mutex_unlock(&data->write_lock);
	return res;
}

/*  ioctl command for lsm330dlc_accel device file */
static long lsm330dlc_accel_ioctl(struct file *file,
		       unsigned int cmd, unsigned long arg)
{
	int err = 0;
	struct lsm330dlc_accel_data *accel_data =
		container_of(file->private_data, struct lsm330dlc_accel_data,
			lsm330dlc_accel_device);
	struct lsm330dlc_acc data;
	s64 delay_ns;

	/* cmd mapping */
	switch (cmd) {
	case LSM330DLC_ACCEL_IOCTL_SET_DELAY:
		if (copy_from_user(&delay_ns, (void __user *)arg,
					sizeof(delay_ns)))
			return -EFAULT;
		err = lsm330dlc_accel_set_delay(accel_data, delay_ns);

		break;
	case LSM330DLC_ACCEL_IOCTL_GET_DELAY:
		delay_ns = lsm330dlc_accel_get_delay(accel_data);
		if (put_user(delay_ns, (s64 __user *)arg))
			return -EFAULT;
		break;
	case LSM330DLC_ACCEL_IOCTL_READ_XYZ:
		err = lsm330dlc_accel_read_xyz(accel_data, &data);
		#ifdef LSM330DLC_ACCEL_LOGGING
		printk(KERN_INFO "%s: raw x = %d, y = %d, z = %d\n",\
			__func__, data.x, data.y, data.z);
		#endif
		if (err)
			break;
		if (copy_to_user((void __user *)arg, &data, sizeof(data)))
			return -EFAULT;
		break;
	default:
		err = -EINVAL;
		break;
	}

	return err;
}

static int lsm330dlc_accel_suspend(struct device *dev)
{
	int res = 0;
	struct lsm330dlc_accel_data *data = dev_get_drvdata(dev);

#ifdef USES_MOVEMETNT_RECOGNITION
	if (data->movement_recog_flag == ON) {
		accel_dbgmsg("LOW_PWR_MODE.\n");
		res = i2c_smbus_write_byte_data(data->client,
						CTRL_REG1, LOW_PWR_MODE);
	} else if (atomic_read(&data->opened) > 0) {
#else
	if (atomic_read(&data->opened) > 0) {
#endif
		accel_dbgmsg("PM_OFF.\n");
		res = i2c_smbus_write_byte_data(data->client,
						CTRL_REG1, PM_OFF);
	}

	return res;
}

static int lsm330dlc_accel_resume(struct device *dev)
{
	int res = 0;
	struct lsm330dlc_accel_data *data = dev_get_drvdata(dev);

	if (atomic_read(&data->opened) > 0) {
		accel_dbgmsg("ctrl_reg1_shadow = %d.\n"
			, data->ctrl_reg1_shadow);
		res = i2c_smbus_write_byte_data(data->client, CTRL_REG1,
						data->ctrl_reg1_shadow);
	}

	return res;
}

static const struct dev_pm_ops lsm330dlc_accel_pm_ops = {
	.suspend = lsm330dlc_accel_suspend,
	.resume = lsm330dlc_accel_resume,
};

static const struct file_operations lsm330dlc_accel_fops = {
	.owner = THIS_MODULE,
	.open = lsm330dlc_accel_open,
	.release = lsm330dlc_accel_close,
	.unlocked_ioctl = lsm330dlc_accel_ioctl,
};

static ssize_t lsm330dlc_accel_fs_read(struct device *dev,
			struct device_attribute *attr, char *buf)
{
	struct lsm330dlc_accel_data *accel_data = dev_get_drvdata(dev);
	struct lsm330dlc_acc data = { 0, };
	int err = 0;
	int on;

	on = atomic_read(&accel_data->opened);
	if (on == 0) {
		mutex_lock(&accel_data->write_lock);
		err = i2c_smbus_write_byte_data(accel_data->client, CTRL_REG1,
						DEFAULT_POWER_ON_SETTING);
	}

	if (err < 0) {
		pr_err("%s: i2c write ctrl_reg1 failed\n", __func__);
		return err;
	}

	err = lsm330dlc_accel_read_xyz(accel_data, &data);
	if (err < 0) {
		pr_err("%s: lsm330dlc_accel_read_xyz failed\n", __func__);
		return err;
	}

	if (on == 0) {
		err = i2c_smbus_write_byte_data(accel_data->client, CTRL_REG1,
								PM_OFF);
		mutex_unlock(&accel_data->write_lock);
		if (err)
			pr_err("%s: i2c write ctrl_reg1 failed\n", __func__);
	}

	return sprintf(buf, "%d,%d,%d\n", data.x, data.y, data.z);
}

static ssize_t lsm330dlc_accel_calibration_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	int err;
	struct lsm330dlc_accel_data *data = dev_get_drvdata(dev);

	err = lsm330dlc_accel_open_calibration(data);
	if (err < 0)
		pr_err("%s: lsm330dlc_accel_open_calibration() failed\n"\
		, __func__);

	if (!data->cal_data.x && !data->cal_data.y && !data->cal_data.z)
		err = -1;

	return sprintf(buf, "%d %d %d %d\n",
		err, data->cal_data.x, data->cal_data.y, data->cal_data.z);
}

static ssize_t lsm330dlc_accel_calibration_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	bool do_calib;
	int err;

	if (sysfs_streq(buf, "1"))
		do_calib = true;
	else if (sysfs_streq(buf, "0"))
		do_calib = false;
	else {
		pr_debug("%s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}

	err = lsm330dlc_accel_do_calibrate(dev, do_calib);
	if (err < 0) {
		pr_err("%s: lsm330dlc_accel_do_calibrate() failed\n", __func__);
		return err;
	}

	return count;
}

#ifdef USES_MOVEMETNT_RECOGNITION
static ssize_t lsm330dlc_accel_reactive_alert_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t count)
{
	int onoff = OFF, err = 0;
	struct lsm330dlc_accel_data *data = dev_get_drvdata(dev);
	static bool is_first = true, factory_test;
	mm_segment_t old_fs;
	struct file *acc_int = NULL;

	if (is_first == true) {
		is_first = false;
		old_fs = get_fs();
		set_fs(KERNEL_DS);

		acc_int = filp_open(ACC_INTERRUPT_FILE_PATH, O_CREAT, 0666);

		if (IS_ERR(acc_int)) {
			pr_err("%s: Can't create interrupt file\n", __func__);
			set_fs(old_fs);
			err = PTR_ERR(acc_int);
			return err;
		}

		accel_dbgmsg("Success creating acc interrupt file .\n");
		filp_close(acc_int, current->files);
		set_fs(old_fs);
	}

	if (sysfs_streq(buf, "1"))
		onoff = ON;
	else if (sysfs_streq(buf, "0"))
		onoff = OFF;
	else if (sysfs_streq(buf, "2")) {
		onoff = ON;
		factory_test = true;
		accel_dbgmsg("factory_test = %d\n", factory_test);
	} else {
		pr_err("%s: invalid value %d\n", __func__, *buf);
		return -EINVAL;
	}

	if (onoff == ON && data->movement_recog_flag == OFF) {
		accel_dbgmsg("reactive alert is on.\n");

		if (atomic_read(&data->opened) == 0) {
			/* if off state, turn on the device.*/
			err = i2c_smbus_write_byte_data(data->client, CTRL_REG1,
				LOW_PWR_MODE);
			if (err)
				pr_err("%s: i2c write ctrl_reg3 failed\n",
					__func__);
		}

		enable_irq(data->client->irq);

		if (device_may_wakeup(&data->client->dev))
			enable_irq_wake(data->client->irq);

		if (factory_test == true) { /* To enable Z axis */
			err = i2c_smbus_write_byte_data(data->client, INT1_CFG,
				FACTORY_INTERRUPT_SETTING);
			if (err)
				pr_err("%s: i2c write int1_cfg failed\n",
					__func__);
		}

		/* INT1_A enable */
		err = i2c_smbus_write_byte_data(data->client, CTRL_REG3,
			DEFAULT_CTRL3_SETTING);
		if (err)
			pr_err("%s: i2c write ctrl_reg3 failed\n", __func__);

		data->movement_recog_flag = ON;
		data->interrupt_state = 0; /* Init interrupt state.*/
	} else if (onoff == OFF && data->movement_recog_flag == ON) {
		accel_dbgmsg("reactive alert is off.\n");

		/* INT1_A disable */
		err = i2c_smbus_write_byte_data(data->client, CTRL_REG3,
			PM_OFF);
		if (err)
			pr_err("%s: i2c write ctrl_reg3 failed\n", __func__);

		if (device_may_wakeup(&data->client->dev))
			disable_irq_wake(data->client->irq);

		disable_irq_nosync(data->client->irq);

		/* return the power state */
		err = i2c_smbus_write_byte_data(data->client, CTRL_REG1,
			data->ctrl_reg1_shadow);
		if (err)
			pr_err("%s: i2c write ctrl_reg3 failed\n", __func__);

		if (factory_test == true) {
			err = i2c_smbus_write_byte_data(data->client, INT1_CFG,
				DEFAULT_INTERRUPT_SETTING);
			factory_test = false;
			if (err)
				pr_err("%s: i2c write int1_cfg failed\n",
					__func__);
		}
		data->movement_recog_flag = OFF;
		data->interrupt_state = 0; /* Init interrupt state.*/
	}

	return count;
}

static ssize_t lsm330dlc_accel_reactive_alert_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct lsm330dlc_accel_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", data->interrupt_state);
}
#endif

#ifdef DEBUG_ODR
static ssize_t lsm330dlc_accel_odr_read(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct lsm330dlc_accel_data *data = dev_get_drvdata(dev);
	struct i2c_msg msg[2];
	u8 odr_data, fs_data, ctrl3_data, int_data, int_ths, int_du;
	u8 reg_buf;
	int err;

	msg[0].addr = data->client->addr;
	msg[0].buf = &reg_buf;
	msg[0].flags = 0;
	msg[0].len = 1;

	/* read output data rate */
	reg_buf = CTRL_REG1;
	msg[1].addr = data->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &odr_data;

	err = i2c_transfer(data->client->adapter, msg, 2);
	if (err != 2)
		return (err < 0) ? err : -EIO;

	/* read full scale setting */
	reg_buf = CTRL_REG4;
	msg[1].addr = data->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &fs_data;

	err = i2c_transfer(data->client->adapter, msg, 2);
	if (err != 2)
		return (err < 0) ? err : -EIO;

	/* read INT1 on */
	reg_buf = CTRL_REG3;
	msg[1].addr = data->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &ctrl3_data;

	err = i2c_transfer(data->client->adapter, msg, 2);
	if (err != 2)
		return (err < 0) ? err : -EIO;

	/* read int1_cfg setting */
	reg_buf = INT1_CFG;
	msg[1].addr = data->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &int_data;

	err = i2c_transfer(data->client->adapter, msg, 2);
	if (err != 2)
		return (err < 0) ? err : -EIO;

	/* read int1_ths setting */
	reg_buf = INT1_THS;
	msg[1].addr = data->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &int_ths;

	err = i2c_transfer(data->client->adapter, msg, 2);
	if (err != 2)
		return (err < 0) ? err : -EIO;

	/* read int1_duration setting */
	reg_buf = INT1_DURATION;
	msg[1].addr = data->client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &int_du;

	err = i2c_transfer(data->client->adapter, msg, 2);
	if (err != 2)
		return (err < 0) ? err : -EIO;

	return sprintf(buf\
, "odr:0x%x,fs:0x%x,REG3:0x%x,int_cfg:0x%x, int_ths:0x%x, int_du:0x%x\n"\
		, odr_data, fs_data, ctrl3_data, int_data, int_ths, int_du);
}
#endif

static DEVICE_ATTR(calibration, 0664,
		   lsm330dlc_accel_calibration_show
		   , lsm330dlc_accel_calibration_store);
static DEVICE_ATTR(raw_data, 0664, lsm330dlc_accel_fs_read, NULL);
#ifdef USES_MOVEMETNT_RECOGNITION
static DEVICE_ATTR(reactive_alert, 0664,
	lsm330dlc_accel_reactive_alert_show,
	lsm330dlc_accel_reactive_alert_store);
#endif

#ifdef DEBUG_ODR
static DEVICE_ATTR(odr, 0664, lsm330dlc_accel_odr_read, NULL);
#endif

void lsm330dlc_accel_shutdown(struct i2c_client *client)
{
	int res = 0;
	struct lsm330dlc_accel_data *data = i2c_get_clientdata(client);

	accel_dbgmsg("is called.\n");
	res = i2c_smbus_write_byte_data(data->client,
					CTRL_REG1, PM_OFF);
	if (res < 0)
		pr_err("%s: pm_off failed %d\n", __func__, res);
}

#ifdef USES_MOVEMETNT_RECOGNITION
static irqreturn_t lsm330dlc_accel_interrupt_thread(int irq\
	, void *lsm330dlc_accel_data_p)
{
	struct file *acc_int = NULL;
	struct lsm330dlc_accel_data *data = lsm330dlc_accel_data_p;
	int err = 0;
	mm_segment_t old_fs;
	char dummy_data = 1;

	/* INT1_A disable */
	err = i2c_smbus_write_byte_data(data->client, CTRL_REG3,
		PM_OFF);
	if (err)
		pr_err("%s: i2c write ctrl_reg3 failed\n", __func__);

	data->interrupt_state = 1;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	acc_int = filp_open(ACC_INTERRUPT_FILE_PATH, O_WRONLY, 0666);
	if (IS_ERR(acc_int)) {
		err = PTR_ERR(acc_int);
		pr_err("%s: Can't open ACC_INTERRUPT file(err = %d)\n",
			__func__, err);
		set_fs(old_fs);
		return IRQ_HANDLED;
	}
	err = acc_int->f_op->write(acc_int,
		(char *)&dummy_data, sizeof(dummy_data), &acc_int->f_pos);
	if (err != sizeof(dummy_data))
		pr_err("%s: Can't write to the interrupt file(err = %d)\n",
			__func__, err);

	filp_close(acc_int, current->files);
	set_fs(old_fs);

	accel_dbgmsg("lsm330dlc_accel irq is handled.\n");

	return IRQ_HANDLED;
}
#endif

static int lsm330dlc_accel_probe(struct i2c_client *client,
		       const struct i2c_device_id *id)
{
	struct lsm330dlc_accel_data *accel_data;
	int err;

	if (!i2c_check_functionality(client->adapter,
				     I2C_FUNC_SMBUS_WRITE_BYTE_DATA |
				     I2C_FUNC_SMBUS_READ_I2C_BLOCK)) {
		pr_err("%s: i2c functionality check failed!\n", __func__);
		err = -ENODEV;
		goto exit;
	}

	accel_data = kzalloc(sizeof(struct lsm330dlc_accel_data), GFP_KERNEL);
	if (accel_data == NULL) {
		dev_err(&client->dev,
				"failed to allocate memory for module data\n");
		err = -ENOMEM;
		goto exit;
	}

	accel_data->client = client;
	i2c_set_clientdata(client, accel_data);

	mutex_init(&accel_data->read_lock);
	mutex_init(&accel_data->write_lock);
	atomic_set(&accel_data->opened, 0);

	/* sensor HAL expects to find /dev/accelerometer */
	accel_data->lsm330dlc_accel_device.minor = MISC_DYNAMIC_MINOR;
	accel_data->lsm330dlc_accel_device.name = ACC_DEV_NAME;
	accel_data->lsm330dlc_accel_device.fops = &lsm330dlc_accel_fops;

	err = misc_register(&accel_data->lsm330dlc_accel_device);
	if (err) {
		pr_err("%s: misc_register failed\n", __FILE__);
		goto err_misc_register;
	}

#ifdef USES_MOVEMETNT_RECOGNITION
	accel_data->movement_recog_flag = OFF;

	err = i2c_smbus_write_byte_data(accel_data->client, INT1_THS
		, MOVEMENT_THRESHOLD);
	if (err)
		pr_err("%s: i2c write int1_ths failed\n", __func__);

	/* (2/odr) ms */
	err = i2c_smbus_write_byte_data(accel_data->client, INT1_DURATION
		, 0x02);
	if (err)
		pr_err("%s: i2c write int1_duration failed\n"
		, __func__);

	err = i2c_smbus_write_byte_data(accel_data->client, INT1_CFG,
		DEFAULT_INTERRUPT_SETTING);
	if (err)
		pr_err("%s: i2c write int1_cfg failed\n", __func__);

	err = request_threaded_irq(accel_data->client->irq, NULL,
		lsm330dlc_accel_interrupt_thread\
		, IRQF_TRIGGER_RISING | IRQF_ONESHOT,\
			"lsm330dlc_accel", accel_data);
	if (err < 0) {
		pr_err("%s: can't allocate irq.\n", __func__);
		goto err_request_irq;
	}

	disable_irq(accel_data->client->irq);
	device_init_wakeup(&accel_data->client->dev, 1);
#endif

	/* creating device for test & calibration */
	accel_data->dev = sensors_classdev_register("accelerometer_sensor");
	if (IS_ERR(accel_data->dev)) {
		pr_err("%s: class create failed(accelerometer_sensor)\n",
			__func__);
		err = PTR_ERR(accel_data->dev);
		goto err_acc_device_create;
	}

	err = device_create_file(accel_data->dev, &dev_attr_raw_data);
	if (err < 0) {
		pr_err("%s: Failed to create device file(%s)\n",
				__func__, dev_attr_raw_data.attr.name);
		goto err_acc_device_create_file;
	}

	err = device_create_file(accel_data->dev, &dev_attr_calibration);
	if (err < 0) {
		pr_err("%s: Failed to create device file(%s)\n",
				__func__, dev_attr_calibration.attr.name);
		goto err_cal_device_create_file;
	}

#ifdef USES_MOVEMETNT_RECOGNITION
	err = device_create_file(accel_data->dev, &dev_attr_reactive_alert);
	if (err < 0) {
		pr_err("%s: Failed to create device file(%s)\n",
				__func__, dev_attr_reactive_alert.attr.name);
		goto err_reactive_device_create_file;
	}
#endif

#ifdef DEBUG_ODR
	err = device_create_file(accel_data->dev, &dev_attr_odr);
		if (err < 0) {
			pr_err("%s: Failed to create device file(%s)\n",
					__func__, dev_attr_odr.attr.name);
			goto err_odr_device_create_file;
		}
#endif

	dev_set_drvdata(accel_data->dev, accel_data);

	return 0;

#ifdef DEBUG_ODR
err_odr_device_create_file:
#ifdef USES_MOVEMETNT_RECOGNITION
	device_remove_file(accel_data->dev, &dev_attr_reactive_alert);
#else
	device_remove_file(accel_data->dev, &dev_attr_calibration);
#endif
#endif
#ifdef USES_MOVEMETNT_RECOGNITION
err_reactive_device_create_file:
	device_remove_file(accel_data->dev, &dev_attr_calibration);
#endif
err_cal_device_create_file:
	device_remove_file(accel_data->dev, &dev_attr_raw_data);
err_acc_device_create_file:
	sensors_classdev_unregister(accel_data->dev);
err_acc_device_create:
#ifdef USES_MOVEMETNT_RECOGNITION
	free_irq(accel_data->client->irq, accel_data);
err_request_irq:
#endif
	misc_deregister(&accel_data->lsm330dlc_accel_device);
err_misc_register:
	mutex_destroy(&accel_data->read_lock);
	mutex_destroy(&accel_data->write_lock);
	kfree(accel_data);
exit:
	return err;
}

static int lsm330dlc_accel_remove(struct i2c_client *client)
{
	struct lsm330dlc_accel_data *accel_data = i2c_get_clientdata(client);

#ifdef USES_MOVEMETNT_RECOGNITION
	device_remove_file(accel_data->dev, &dev_attr_reactive_alert);
#endif
	device_remove_file(accel_data->dev, &dev_attr_calibration);
	device_remove_file(accel_data->dev, &dev_attr_raw_data);
	sensors_classdev_unregister(accel_data->dev);

#ifdef USES_MOVEMETNT_RECOGNITION
	device_init_wakeup(&accel_data->client->dev, 0);
	free_irq(accel_data->client->irq, accel_data);
#endif
	misc_deregister(&accel_data->lsm330dlc_accel_device);
	mutex_destroy(&accel_data->read_lock);
	mutex_destroy(&accel_data->write_lock);
	kfree(accel_data);

	return 0;
}

static const struct i2c_device_id lsm330dlc_accel_id[] = {
	{ "lsm330dlc_accel", 0 }, /* lsm330dlc_accel */
	{ }
};
MODULE_DEVICE_TABLE(i2c, lsm330dlc_accel_id);

static struct i2c_driver lsm330dlc_accel_driver = {
	.probe = lsm330dlc_accel_probe,
	.shutdown = lsm330dlc_accel_shutdown,
	.remove = __devexit_p(lsm330dlc_accel_remove),
	.id_table = lsm330dlc_accel_id,
	.driver = {
		.pm = &lsm330dlc_accel_pm_ops,
		.owner = THIS_MODULE,
		.name = "lsm330dlc_accel", /* lsm330dlc_accel */
	},
};

static int __init lsm330dlc_accel_init(void)
{
	return i2c_add_driver(&lsm330dlc_accel_driver);
}

static void __exit lsm330dlc_accel_exit(void)
{
	i2c_del_driver(&lsm330dlc_accel_driver);
}

module_init(lsm330dlc_accel_init);
module_exit(lsm330dlc_accel_exit);

MODULE_DESCRIPTION("LSM330DLC accelerometer driver");
MODULE_AUTHOR("Samsung Electronics");
MODULE_LICENSE("GPL");
