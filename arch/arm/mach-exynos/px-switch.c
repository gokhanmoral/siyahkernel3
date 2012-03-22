#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/semaphore.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <plat/gpio-cfg.h>
#include <mach/gpio.h>
#include <mach/usb_switch.h>

static struct device *sec_switch_dev;

enum usb_path_t current_path = USB_PATH_NONE;

static struct semaphore usb_switch_sem;

static ssize_t show_usb_sel(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	/* 1 for AP, 0 for CP */
	return sprintf(buf, "%d", current_path & USB_PATH_CP ? 0 : 1);
}

static ssize_t store_usb_sel(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	int ret, usb_sel;
	ret = sscanf(buf, "%d", &usb_sel);

	if (ret != 1)
		return -EINVAL;

	pr_err("%s: %d\n", __func__, usb_sel);
	/* 1 for AP, 0 for CP */
	if (usb_sel == 1)
		usb_switch_clr_path(USB_PATH_CP);
	else if (usb_sel == 0)
		usb_switch_set_path(USB_PATH_CP);

	return count;
}

static DEVICE_ATTR(usb_sel, 0644, show_usb_sel, store_usb_sel);

static ssize_t show_uart_sel(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
#ifdef CONFIG_MACH_P8LTE
	/* 2 for LTE, 1 for AP, 0 for CP */
	int val_sel1, val_sel2;
	val_sel1 = gpio_get_value(GPIO_UART_SEL1);
	val_sel2 = gpio_get_value(GPIO_UART_SEL2);
	return sprintf(buf, "%d", val_sel1 << (1 - val_sel2));
#else
	/* 1 for AP, 0 for CP */
	return sprintf(buf, "%d", gpio_get_value(GPIO_UART_SEL));
#endif
}

static ssize_t store_uart_sel(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	int ret, uart_sel;
	ret = sscanf(buf, "%d", &uart_sel);

	if (ret != 1)
		return -EINVAL;

	pr_err("%s: %d\n", __func__, uart_sel);

#ifdef CONFIG_MACH_P8LTE
	/* 2 for LTE, 1 for AP, 0 for CP */
	int set_val1, set_val2;

	set_val1 = (uart_sel > 0) ? 1 : 0;
	set_val2 = uart_sel & 0x0001;

	gpio_set_value(GPIO_UART_SEL1, set_val1);
	gpio_set_value(GPIO_UART_SEL2, set_val2);

#else
	/* 1 for AP, 0 for CP */
	if (uart_sel == 1)
		gpio_set_value(GPIO_UART_SEL, 1);
	else if (uart_sel == 0)
		gpio_set_value(GPIO_UART_SEL, 0);
#endif

	return count;
}

static DEVICE_ATTR(uart_sel, 0644, show_uart_sel, store_uart_sel);

static void pmic_safeout2(int onoff)
{
	struct regulator *regulator;

	regulator = regulator_get(NULL, "safeout2");
	BUG_ON(IS_ERR_OR_NULL(regulator));

	if (onoff) {
		if (!regulator_is_enabled(regulator)) {
			regulator_enable(regulator);
		} else {
			pr_err("%s: onoff:%d No change in safeout2\n",
			       __func__, onoff);
		}
	} else {
		if (regulator_is_enabled(regulator)) {
			regulator_force_disable(regulator);
		} else {
			pr_err("%s: onoff:%d No change in safeout2\n",
			       __func__, onoff);
		}
	}

	regulator_put(regulator);
}

static void usb_apply_path(enum usb_path_t path)
{
	pr_err("%s: current gpio before changing : sel1:%d sel2:%d sel3:%d\n",
	       __func__, gpio_get_value(GPIO_USB_SEL1),
	       gpio_get_value(GPIO_USB_SEL2), gpio_get_value(GPIO_USB_SEL3));
	pr_err("%s: target path %x\n", __func__, path);

	/* following checks are ordered according to priority */
	if (path & USB_PATH_ADCCHECK) {
		gpio_set_value(GPIO_USB_SEL1, 0);
		gpio_set_value(GPIO_USB_SEL2, 1);
		/* don't care SEL3 */
#if defined(CONFIG_MACH_P8LTE)  || defined(CONFIG_MACH_P8)
		gpio_set_value(GPIO_USB_SEL3, 1);
#endif
		goto out_nochange;
	}
	if (path & USB_PATH_TA) {
		gpio_set_value(GPIO_USB_SEL1, 1);
		gpio_set_value(GPIO_USB_SEL2, 0);
		/* don't care SEL3 */
		goto out_ap;
	}
	if (path & USB_PATH_CP) {
		pr_err("DEBUG: set USB path to CP\n");
		gpio_set_value(GPIO_USB_SEL1, 0);
		gpio_set_value(GPIO_USB_SEL2, 0);
		/* don't care SEL3 */
#ifdef CONFIG_MACH_P8LTE
		gpio_set_value(GPIO_USB_SEL3, 1);
#endif
		mdelay(3);
		goto out_cp;
	}
	if (path & USB_PATH_OTG) {
		gpio_set_value(GPIO_USB_SEL1, 1);
		/* don't care SEL2 */
		gpio_set_value(GPIO_USB_SEL3, 1);
		goto out_ap;
	}
	if (path & USB_PATH_HOST) {
#ifndef CONFIG_MACH_P8LTE
		gpio_set_value(GPIO_USB_SEL1, 1);
#endif
		/* don't care SEL2 */
		gpio_set_value(GPIO_USB_SEL3, 0);
		goto out_ap;
	}

	/* default */
	gpio_set_value(GPIO_USB_SEL1, 1);
#ifdef CONFIG_MACH_P8LTE
	gpio_set_value(GPIO_USB_SEL2, 1);
#else
	gpio_set_value(GPIO_USB_SEL2, 0);
#endif
	gpio_set_value(GPIO_USB_SEL3, 1);

out_ap:
	pr_err("%s: %x safeout2 off\n", __func__, path);
	pmic_safeout2(0);
	return;

out_cp:
	pr_err("%s: %x safeout2 on\n", __func__, path);
	pmic_safeout2(1);
	return;

out_nochange:
	pr_err("%s: %x safeout2 no change\n", __func__, path);
	return;
}

/*
  Typical usage of usb switch:

  usb_switch_lock();  (alternatively from hard/soft irq context)
  ( or usb_switch_trylock() )
  ...
  usb_switch_set_path(USB_PATH_ADCCHECK);
  ...
  usb_switch_set_path(USB_PATH_TA);
  ...
  usb_switch_unlock(); (this restores previous usb switch settings)
*/
void usb_switch_set_path(enum usb_path_t path)
{
	pr_err("%s: %x current_path before changing\n", __func__, current_path);
	current_path |= path;
	usb_apply_path(current_path);
}

void usb_switch_clr_path(enum usb_path_t path)
{
	pr_err("%s: %x current_path before changing\n", __func__, current_path);
	current_path &= ~path;
	usb_apply_path(current_path);
}

int usb_switch_lock(void)
{
	return down_interruptible(&usb_switch_sem);
}

int usb_switch_trylock(void)
{
	return down_trylock(&usb_switch_sem);
}

void usb_switch_unlock(void)
{
	up(&usb_switch_sem);
}

static int __init usb_switch_init(void)
{
	int ret;

	gpio_request(GPIO_USB_SEL1, "GPIO_USB_SEL1");
	gpio_request(GPIO_USB_SEL2, "GPIO_USB_SEL2");
	gpio_request(GPIO_USB_SEL3, "GPIO_USB_SEL3");
#ifdef CONFIG_MACH_P8LTE
	gpio_request(GPIO_UART_SEL1, "GPIO_UART_SEL1");
	gpio_request(GPIO_UART_SEL2, "GPIO_UART_SEL2");
#else
	gpio_request(GPIO_UART_SEL, "GPIO_UART_SEL");
#endif

	gpio_export(GPIO_USB_SEL1, 1);
	gpio_export(GPIO_USB_SEL2, 1);
	gpio_export(GPIO_USB_SEL3, 1);
#ifdef CONFIG_MACH_P8LTE
	gpio_export(GPIO_UART_SEL1, 1);
	gpio_export(GPIO_UART_SEL2, 1);
#else
	gpio_export(GPIO_UART_SEL, 1);
#endif

	BUG_ON(!sec_class);
	sec_switch_dev = device_create(sec_class, NULL, 0, NULL, "sec_switch");

	BUG_ON(!sec_switch_dev);
	gpio_export_link(sec_switch_dev, "GPIO_USB_SEL1", GPIO_USB_SEL1);
	gpio_export_link(sec_switch_dev, "GPIO_USB_SEL2", GPIO_USB_SEL2);
	gpio_export_link(sec_switch_dev, "GPIO_USB_SEL3", GPIO_USB_SEL3);
#ifdef CONFIG_MACH_P8LTE
	gpio_export_link(sec_switch_dev, "GPIO_UART_SEL1", GPIO_UART_SEL1);
	gpio_export_link(sec_switch_dev, "GPIO_UART_SEL2", GPIO_UART_SEL2);
#else
	gpio_export_link(sec_switch_dev, "GPIO_UART_SEL", GPIO_UART_SEL);
#endif

	/*init_MUTEX(&usb_switch_sem);*/
	sema_init(&usb_switch_sem, 1);

	if (!gpio_get_value(GPIO_USB_SEL1))
		usb_switch_set_path(USB_PATH_CP);

	ret = device_create_file(sec_switch_dev, &dev_attr_usb_sel);
	BUG_ON(ret);

	ret = device_create_file(sec_switch_dev, &dev_attr_uart_sel);
	BUG_ON(ret);

	return 0;
}

device_initcall(usb_switch_init);
