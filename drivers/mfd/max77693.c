/*
 * max77693.c - mfd core driver for the Maxim 77693
 *
 * Copyright (C) 2011 Samsung Electronics
 * SangYoung Son <hello.son@smasung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This driver is based on max8997.c
 */

#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/mfd/core.h>
#include <linux/mfd/max77693.h>
#include <linux/mfd/max77693-private.h>
#include <linux/regulator/machine.h>

#define I2C_ADDR_PMIC	(0xCC >> 1)	/* Charger, Flash LED */
#define I2C_ADDR_MUIC	(0x4A >> 1)
#define I2C_ADDR_HAPTIC	(0x90 >> 1)

static struct mfd_cell max77693_devs[] = {
	{ .name = "max77693-charger", },
	{ .name = "max77693-led", },
	{ .name = "max77693-muic", },
	{ .name = "max77693-safeout", },
	{ .name = "max77693-haptic", },
};

int max77693_read_reg(struct i2c_client *i2c, u8 reg, u8 *dest)
{
	struct max77693_dev *max77693 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77693->iolock);
	ret = i2c_smbus_read_byte_data(i2c, reg);
	mutex_unlock(&max77693->iolock);
	if (ret < 0)
		return ret;

	ret &= 0xff;
	*dest = ret;
	return 0;
}
EXPORT_SYMBOL_GPL(max77693_read_reg);

int max77693_bulk_read(struct i2c_client *i2c, u8 reg, int count, u8 *buf)
{
	struct max77693_dev *max77693 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77693->iolock);
	ret = i2c_smbus_read_i2c_block_data(i2c, reg, count, buf);
	mutex_unlock(&max77693->iolock);
	if (ret < 0)
		return ret;

	return 0;
}
EXPORT_SYMBOL_GPL(max77693_bulk_read);

int max77693_write_reg(struct i2c_client *i2c, u8 reg, u8 value)
{
	struct max77693_dev *max77693 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77693->iolock);
	ret = i2c_smbus_write_byte_data(i2c, reg, value);
	mutex_unlock(&max77693->iolock);
	return ret;
}
EXPORT_SYMBOL_GPL(max77693_write_reg);

int max77693_bulk_write(struct i2c_client *i2c, u8 reg, int count, u8 *buf)
{
	struct max77693_dev *max77693 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77693->iolock);
	ret = i2c_smbus_write_i2c_block_data(i2c, reg, count, buf);
	mutex_unlock(&max77693->iolock);
	if (ret < 0)
		return ret;

	return 0;
}
EXPORT_SYMBOL_GPL(max77693_bulk_write);

int max77693_update_reg(struct i2c_client *i2c, u8 reg, u8 val, u8 mask)
{
	struct max77693_dev *max77693 = i2c_get_clientdata(i2c);
	int ret;

	mutex_lock(&max77693->iolock);
	ret = i2c_smbus_read_byte_data(i2c, reg);
	if (ret >= 0) {
		u8 old_val = ret & 0xff;
		u8 new_val = (val & mask) | (old_val & (~mask));
		ret = i2c_smbus_write_byte_data(i2c, reg, new_val);
	}
	mutex_unlock(&max77693->iolock);
	return ret;
}
EXPORT_SYMBOL_GPL(max77693_update_reg);

static int max77693_i2c_probe(struct i2c_client *i2c,
			      const struct i2c_device_id *id)
{
	struct max77693_dev *max77693;
	struct max77693_platform_data *pdata = i2c->dev.platform_data;
	u8 reg_data;
	int ret = 0;

	max77693 = kzalloc(sizeof(struct max77693_dev), GFP_KERNEL);
	if (max77693 == NULL)
		return -ENOMEM;

	i2c_set_clientdata(i2c, max77693);
	max77693->dev = &i2c->dev;
	max77693->i2c = i2c;
	max77693->irq = i2c->irq;
	max77693->type = id->driver_data;
	if (pdata) {
		max77693->irq_base = pdata->irq_base;
		max77693->irq_gpio = pdata->irq_gpio;
		max77693->wakeup = pdata->wakeup;
	} else
		goto err;

	mutex_init(&max77693->iolock);

	if (max77693_read_reg(i2c, MAX77693_PMIC_REG_PMIC_ID2, &reg_data) < 0) {
		dev_err(max77693->dev,
			"device not found on this channel (this is not an error)\n");
		ret = -ENODEV;
		goto err;
	} else {
		/* print rev */
		max77693->pmic_rev = (reg_data & 0x7);
		max77693->pmic_ver = ((reg_data & 0xF8) >> 0x3);
		pr_info("%s: device found: rev.0x%x, ver.0x%x\n", __func__,
				max77693->pmic_rev, max77693->pmic_ver);
	}

	max77693->muic = i2c_new_dummy(i2c->adapter, I2C_ADDR_MUIC);
	i2c_set_clientdata(max77693->muic, max77693);

	max77693->haptic = i2c_new_dummy(i2c->adapter, I2C_ADDR_HAPTIC);
	i2c_set_clientdata(max77693->haptic, max77693);

	ret = max77693_irq_init(max77693);
	if (ret < 0)
		goto err_mfd;

	ret = mfd_add_devices(max77693->dev, -1, max77693_devs,
			ARRAY_SIZE(max77693_devs), NULL, 0);
	if (ret < 0)
		goto err_mfd;

	return ret;

err_mfd:
	mfd_remove_devices(max77693->dev);
	i2c_unregister_device(max77693->muic);
	i2c_unregister_device(max77693->haptic);
err:
	kfree(max77693);
	return ret;
}

static int max77693_i2c_remove(struct i2c_client *i2c)
{
	struct max77693_dev *max77693 = i2c_get_clientdata(i2c);

	mfd_remove_devices(max77693->dev);
	i2c_unregister_device(max77693->muic);
	i2c_unregister_device(max77693->haptic);
	kfree(max77693);

	return 0;
}

static const struct i2c_device_id max77693_i2c_id[] = {
	{ "max77693", TYPE_MAX77693 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, max77693_i2c_id);

#ifdef CONFIG_PM
static int max77693_suspend(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct max77693_dev *max77693 = i2c_get_clientdata(i2c);

	if (max77693->wakeup)
		enable_irq_wake(max77693->irq);

	disable_irq(max77693->irq);

	return 0;
}

static int max77693_resume(struct device *dev)
{
	struct i2c_client *i2c = container_of(dev, struct i2c_client, dev);
	struct max77693_dev *max77693 = i2c_get_clientdata(i2c);

	if (max77693->wakeup)
		disable_irq_wake(max77693->irq);

	enable_irq(max77693->irq);

	return 0;
}
#else
#define max77693_suspend	NULL
#define max77693_resume		NULL
#endif /* CONFIG_PM */

const struct dev_pm_ops max77693_pm = {
	.suspend = max77693_suspend,
	.resume = max77693_resume,
};

static struct i2c_driver max77693_i2c_driver = {
	.driver = {
		.name = "max77693",
		.owner = THIS_MODULE,
		.pm = &max77693_pm,
	},
	.probe = max77693_i2c_probe,
	.remove = max77693_i2c_remove,
	.id_table = max77693_i2c_id,
};

static int __init max77693_i2c_init(void)
{
	return i2c_add_driver(&max77693_i2c_driver);
}
/* init early so consumer devices can complete system boot */
subsys_initcall(max77693_i2c_init);

static void __exit max77693_i2c_exit(void)
{
	i2c_del_driver(&max77693_i2c_driver);
}
module_exit(max77693_i2c_exit);

MODULE_DESCRIPTION("MAXIM 77693 multi-function core driver");
MODULE_AUTHOR("SangYoung, Son <hello.son@samsung.com>");
MODULE_LICENSE("GPL");
