/* /linux/drivers/misc/modem_if/modem_modemctl_device_cmc221.c
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

#define DEBUG

#include <linux/init.h>

#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>

#include <linux/platform_data/modem.h>
#include "modem_prj.h"
#include "modem_link_device_usb.h"


static void mc_state_fsm(struct modem_ctl *mc)
{
	int cp_reset  = gpio_get_value(mc->gpio_cp_reset);
	int cp_active = gpio_get_value(mc->gpio_phone_active);
	int old_state = mc->phone_state;
	int new_state = mc->phone_state;

	pr_err("[CMC] <%s> old_state = %d, cp_reset = %d, cp_active = %d\n",
		__func__, old_state, cp_reset, cp_active);

	if (cp_active) {
		if (old_state == STATE_CRASH_EXIT) {
			mdm_info(mc, "LTE DUMP END!!!\n");
			pr_err("[CMC] <%s> LTE DUMP END!!!\n", __func__);
			pr_err("[CMC] <%s> new_state = OFFLINE\n", __func__);
		} else if (old_state == STATE_BOOTING) {
			new_state = STATE_ONLINE;
			pr_err("[CMC] <%s> new_state = ONLINE\n", __func__);
		} else {
			pr_err("[CMC] <%s> Don't care!!!\n", __func__);
		}
	} else {
		if (old_state == STATE_ONLINE) {
			new_state = STATE_CRASH_EXIT;
			mdm_info(mc, "LTE CRASHED!!!\n");
			pr_err("[CMC] <%s> LTE CRASHED!!!\n", __func__);
			pr_err("[CMC] <%s> new_state = CRASH_EXIT\n",
				__func__);
		} else {
			pr_err("[CMC] <%s> Don't care!!!\n", __func__);
		}
	}

	if (old_state != new_state)
		mc->iod->modem_state_changed(mc->iod, new_state);
}

static void mc_work(struct work_struct *work_arg)
{
	struct modem_ctl *mc = NULL;

	mc = container_of(work_arg, struct modem_ctl, dwork.work);

	mc_state_fsm(mc);
}

static irqreturn_t phone_active_handler(int irq, void *arg)
{
	struct modem_ctl *mc = (struct modem_ctl *)arg;
	int cp_reset = gpio_get_value(mc->gpio_cp_reset);

	if (cp_reset)
		schedule_delayed_work(&mc->dwork, 1);

	return IRQ_HANDLED;
}

static int cmc221_on(struct modem_ctl *mc)
{
	mdm_info(mc, "on\n");
	pr_err("[CMC] <%s> device = %s\n", __func__, mc->bootd->name);

	gpio_set_value(mc->gpio_cp_on, 0);
	if (mc->gpio_cp_off)
		gpio_set_value(mc->gpio_cp_off, 1);
	gpio_set_value(mc->gpio_cp_reset, 0);

	msleep(500);

	gpio_set_value(mc->gpio_cp_on, 1);
	if (mc->gpio_cp_off)
		gpio_set_value(mc->gpio_cp_off, 0);

	msleep(100);

	gpio_set_value(mc->gpio_cp_reset, 1);

	msleep(300);

	mc->phone_state = STATE_OFFLINE;

	return 0;
}

static int cmc221_off(struct modem_ctl *mc)
{
	mdm_info(mc, "off\n");
	pr_err("[CMC] <%s> device = %s\n", __func__, mc->bootd->name);

	gpio_set_value(mc->gpio_cp_on, 0);

	msleep(100);

	if (mc->gpio_cp_off)
		gpio_set_value(mc->gpio_cp_off, 1);
	gpio_set_value(mc->gpio_cp_reset, 0);

	mc->phone_state = STATE_OFFLINE;

	return 0;
}

static int cmc221_force_crash_exit(struct modem_ctl *mc)
{
	mdm_info(mc, "crash_exit\n");
	pr_err("[CMC] <%s> device = %s\n", __func__, mc->iod->name);

	/* Make DUMP start */
	mc->iod->link->force_dump(mc->iod->link, mc->iod);

	msleep_interruptible(1000);

	mc->iod->modem_state_changed(mc->iod, STATE_CRASH_EXIT);

	return 0;
}

static int cmc221_dump_reset(struct modem_ctl *mc)
{
	mdm_info(mc, "dump_reset\n");
	pr_err("[CMC] <%s> device = %s\n", __func__, mc->iod->name);

	gpio_set_value(mc->gpio_host_active, 0);
	gpio_set_value(mc->gpio_cp_reset, 0);

	udelay(200);

	gpio_set_value(mc->gpio_cp_reset, 1);

	msleep(300);

	return 0;
}

static int cmc221_reset(struct modem_ctl *mc)
{
	mdm_info(mc, "reset\n");

	if (cmc221_off(mc))
		return -ENXIO;

	msleep(100);

	if (cmc221_on(mc))
		return -ENXIO;

	return 0;
}

static int cmc221_boot_on(struct modem_ctl *mc)
{
	mdm_dbg(mc, "\n");
	pr_err("[CMC] <%s>\n", __func__);

	pr_err("[CMC] <%s> phone_state = STATE_BOOTING\n", __func__);
	mc->iod->modem_state_changed(mc->iod, STATE_BOOTING);

	return 0;
}

static int cmc221_boot_off(struct modem_ctl *mc)
{
	mdm_dbg(mc, "\n");
	pr_err("[CMC] <%s>\n", __func__);
	return 0;
}

static void cmc221_get_ops(struct modem_ctl *mc)
{
	mc->ops.modem_on = cmc221_on;
	mc->ops.modem_off = cmc221_off;
	mc->ops.modem_reset = cmc221_reset;
	mc->ops.modem_boot_on = cmc221_boot_on;
	mc->ops.modem_boot_off = cmc221_boot_off;
	mc->ops.modem_force_crash_exit = cmc221_force_crash_exit;
	mc->ops.modem_dump_reset = cmc221_dump_reset;
}

int cmc221_init_modemctl_device(struct modem_ctl *mc, struct modem_data *pdata)
{
	int ret = 0;
	int irq = 0;
	unsigned long flag = 0;
	struct platform_device *pdev = NULL;

	mc->gpio_cp_on        = pdata->gpio_cp_on;
	mc->gpio_cp_off       = pdata->gpio_cp_off;
	mc->gpio_cp_reset     = pdata->gpio_cp_reset;
	mc->gpio_phone_active = pdata->gpio_phone_active;
#if 0	/*TODO: check the GPIO map*/
	mc->gpio_pda_active   = pdata->gpio_pda_active;
	mc->gpio_cp_dump_int  = pdata->gpio_cp_dump_int;
	mc->gpio_flm_uart_sel = pdata->gpio_flm_uart_sel;
	mc->gpio_slave_wakeup = pdata->gpio_slave_wakeup;
	mc->gpio_host_active  = pdata->gpio_host_active;
	mc->gpio_host_wakeup  = pdata->gpio_host_wakeup;
#endif

	if (!mc->gpio_cp_on || !mc->gpio_cp_reset || !mc->gpio_phone_active) {
		mdm_err(mc, "no GPIO data\n");
		return -ENXIO;
	}

	gpio_set_value(mc->gpio_cp_reset, 0);
	gpio_set_value(mc->gpio_cp_on, 0);
	if (mc->gpio_cp_off)
		gpio_set_value(mc->gpio_cp_off, 1);

	cmc221_get_ops(mc);
	dev_set_drvdata(mc->dev, mc);
	INIT_DELAYED_WORK(&mc->dwork, mc_work);

	pdev = to_platform_device(mc->dev);
	mc->irq_phone_active = platform_get_irq_byname(pdev, "cp_active_irq");
	if (!mc->irq_phone_active) {
		mdm_err(mc, "get irq fail\n");
		return -1;
	}

	irq = mc->irq_phone_active;
	pr_err("[CMC] <%s> PHONE_ACTIVE IRQ# = %d\n", __func__, irq);

	flag = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
	/* flag |= IRQF_NO_SUSPEND; */
	ret = request_irq(irq, phone_active_handler, flag, "cmc_active", mc);
	if (ret) {
		pr_err("[CMC] <%s> request_irq fail (%d)\n", __func__, ret);
		return ret;
	}

	ret = enable_irq_wake(irq);
	if (ret)
		pr_err("[CMC] <%s> enable_irq_wake fail (%d)\n", __func__, ret);

	pr_err("[CMC] <%s> IRQ#%d handler is registered.\n", __func__, irq);

	return 0;
}
