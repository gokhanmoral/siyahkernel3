/*
 *  drivers/extcon/extcon_gpio.c
 *
 *  Single-state GPIO extcon driver based on extcon class
 *
 * Copyright (C) 2008 Google, Inc.
 * Author: Mike Lockwood <lockwood@android.com>
 *
 * Modified by MyungJoo Ham <myungjoo.ham@samsung.com> to support extcon
 * (originally switch class is supported)
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/extcon.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>

struct gpio_extcon_data {
	struct extcon_dev edev;
	unsigned gpio;
	const char *state_on;
	const char *state_off;
	int irq;
	struct work_struct work;
};

static void gpio_extcon_work(struct work_struct *work)
{
	int state;
	struct gpio_extcon_data	*data =
		container_of(work, struct gpio_extcon_data, work);

	state = gpio_get_value(data->gpio);
	extcon_set_state(&data->edev, state);
}

static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
	struct gpio_extcon_data *extcon_data =
	    (struct gpio_extcon_data *)dev_id;

	schedule_work(&extcon_data->work);
	return IRQ_HANDLED;
}

static ssize_t extcon_gpio_print_state(struct extcon_dev *edev, char *buf)
{
	struct gpio_extcon_data	*extcon_data =
		container_of(edev, struct gpio_extcon_data, edev);
	const char *state;
	if (extcon_get_state(edev))
		state = extcon_data->state_on;
	else
		state = extcon_data->state_off;

	if (state)
		return sprintf(buf, "%s\n", state);
	return -1;
}

static int gpio_extcon_probe(struct platform_device *pdev)
{
	struct gpio_extcon_platform_data *pdata = pdev->dev.platform_data;
	struct gpio_extcon_data *extcon_data;
	int ret = 0;

	if (!pdata)
		return -EBUSY;

	extcon_data = kzalloc(sizeof(struct gpio_extcon_data), GFP_KERNEL);
	if (!extcon_data)
		return -ENOMEM;

	extcon_data->edev.name = pdata->name;
	extcon_data->gpio = pdata->gpio;
	extcon_data->state_on = pdata->state_on;
	extcon_data->state_off = pdata->state_off;
	extcon_data->edev.print_state = extcon_gpio_print_state;

	ret = extcon_dev_register(&extcon_data->edev, &pdev->dev);
	if (ret < 0)
		goto err_extcon_dev_register;

	ret = gpio_request(extcon_data->gpio, pdev->name);
	if (ret < 0)
		goto err_request_gpio;

	ret = gpio_direction_input(extcon_data->gpio);
	if (ret < 0)
		goto err_set_gpio_input;

	INIT_WORK(&extcon_data->work, gpio_extcon_work);

	extcon_data->irq = gpio_to_irq(extcon_data->gpio);
	if (extcon_data->irq < 0) {
		ret = extcon_data->irq;
		goto err_detect_irq_num_failed;
	}

	ret = request_irq(extcon_data->irq, gpio_irq_handler,
			  IRQF_TRIGGER_LOW, pdev->name, extcon_data);
	if (ret < 0)
		goto err_request_irq;

	/* Perform initial detection */
	gpio_extcon_work(&extcon_data->work);

	return 0;

err_request_irq:
err_detect_irq_num_failed:
err_set_gpio_input:
	gpio_free(extcon_data->gpio);
err_request_gpio:
	extcon_dev_unregister(&extcon_data->edev);
err_extcon_dev_register:
	kfree(extcon_data);

	return ret;
}

static int __devexit gpio_extcon_remove(struct platform_device *pdev)
{
	struct gpio_extcon_data *extcon_data = platform_get_drvdata(pdev);

	cancel_work_sync(&extcon_data->work);
	gpio_free(extcon_data->gpio);
	extcon_dev_unregister(&extcon_data->edev);
	kfree(extcon_data);

	return 0;
}

static struct platform_driver gpio_extcon_driver = {
	.probe		= gpio_extcon_probe,
	.remove		= __devexit_p(gpio_extcon_remove),
	.driver		= {
		.name	= "extcon-gpio",
		.owner	= THIS_MODULE,
	},
};

static int __init gpio_extcon_init(void)
{
	return platform_driver_register(&gpio_extcon_driver);
}

static void __exit gpio_extcon_exit(void)
{
	platform_driver_unregister(&gpio_extcon_driver);
}

module_init(gpio_extcon_init);
module_exit(gpio_extcon_exit);

MODULE_AUTHOR("Mike Lockwood <lockwood@android.com>");
MODULE_DESCRIPTION("GPIO extcon driver");
MODULE_LICENSE("GPL");
