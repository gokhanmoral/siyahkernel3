#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/sensor/gp2a.h>
#include <linux/sensor/lsm330dlc_accel.h>
#include <linux/sensor/lsm330dlc_gyro.h>
#include <linux/sensor/ak8975.h>
#include <linux/sensor/lps331ap.h>
#include <linux/sensor/cm36651.h>
#include <linux/sensor/cm3663.h>

#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>
#include <mach/gpio.h>
#include "midas.h"

#ifdef CONFIG_SENSORS_LSM330DLC
static struct i2c_board_info i2c_devs1[] __initdata = {
	{
		I2C_BOARD_INFO("lsm330dlc_accel", (0x32 >> 1)),
	},
	{
		I2C_BOARD_INFO("lsm330dlc_gyro", (0xD6 >> 1)),
	},
};

static void lsm331dlc_gpio_init(void)
{
	int ret = gpio_request(GPIO_GYRO_INT, "lsm330dlc_gyro_irq");

	printk(KERN_INFO "%s\n", __func__);

	if (ret)
		printk(KERN_ERR "Failed to request gpio lsm330dlc_gyro_irq\n");

	ret = gpio_request(GPIO_GYRO_DE, "lsm330dlc_gyro_data_enable");

	if (ret)
		printk(KERN_ERR
		       "Failed to request gpio lsm330dlc_gyro_data_enable\n");

	ret = gpio_request(GPIO_ACC_INT, "lsm330dlc_accel_irq");

	if (ret)
		printk(KERN_ERR "Failed to request gpio lsm330dlc_accel_irq\n");

	/* Accelerometer sensor interrupt pin initialization */
	s3c_gpio_cfgpin(GPIO_ACC_INT, S3C_GPIO_INPUT);
	gpio_set_value(GPIO_ACC_INT, 2);
	s3c_gpio_setpull(GPIO_ACC_INT, S3C_GPIO_PULL_NONE);
	s5p_gpio_set_drvstr(GPIO_ACC_INT, S5P_GPIO_DRVSTR_LV1);
	i2c_devs1[0].irq = gpio_to_irq(GPIO_ACC_INT);

	/* Gyro sensor interrupt pin initialization */
#if defined(CONFIG_MACH_C1) || defined(CONFIG_MACH_C1VZW) ||\
	defined(CONFIG_MACH_JENGA) || defined(CONFIG_MACH_S2PLUS) || \
	defined(CONFIG_MACH_M0) || defined(CONFIG_MACH_M3) || \
	defined(CONFIG_MACH_C1CTC)
	s5p_register_gpio_interrupt(GPIO_GYRO_INT);
	s3c_gpio_cfgpin(GPIO_GYRO_INT, S3C_GPIO_SFN(0xF));
#else /* For Midas */
	s3c_gpio_cfgpin(GPIO_GYRO_INT, S3C_GPIO_INPUT);
#endif
	gpio_set_value(GPIO_GYRO_INT, 2);
	s3c_gpio_setpull(GPIO_GYRO_INT, S3C_GPIO_PULL_DOWN);
	s5p_gpio_set_drvstr(GPIO_GYRO_INT, S5P_GPIO_DRVSTR_LV1);
	i2c_devs1[1].irq = -1;/*gpio_to_irq(GPIO_GYRO_INT);*/

	/* Gyro sensor data enable pin initialization */
	s3c_gpio_cfgpin(GPIO_GYRO_DE, S3C_GPIO_OUTPUT);
	gpio_set_value(GPIO_GYRO_DE, 0);
	s3c_gpio_setpull(GPIO_GYRO_DE, S3C_GPIO_PULL_DOWN);
	s5p_gpio_set_drvstr(GPIO_GYRO_DE, S5P_GPIO_DRVSTR_LV1);
}
#endif

#if defined(CONFIG_SENSORS_GP2A) || defined(CONFIG_SENSORS_CM36651) || \
	defined(CONFIG_SENSORS_CM3663)
static int proximity_leda_on(bool onoff)
{
	printk(KERN_INFO "%s, onoff = %d\n", __func__, onoff);

	gpio_set_value(GPIO_PS_ALS_EN, onoff);

	return 0;
}

static struct cm36651_platform_data cm36651_pdata = {
	.cm36651_led_on = proximity_leda_on,
	.irq = GPIO_PS_ALS_INT,
};

static struct cm3663_platform_data cm3663_pdata = {
	.proximity_power = proximity_leda_on,
};

static struct i2c_board_info i2c_devs9_emul[] __initdata = {
	{
		I2C_BOARD_INFO("gp2a", (0x72 >> 1)),
	},
	{
		I2C_BOARD_INFO("cm36651", (0x30 >> 1)),
		.platform_data = &cm36651_pdata,
	},
	{
		I2C_BOARD_INFO("cm3663", (0x20)),
		.irq = GPIO_PS_ALS_INT,
		.platform_data = &cm3663_pdata,
	}
};


static struct gp2a_platform_data gp2a_pdata = {
	.gp2a_led_on	= proximity_leda_on,
	.p_out = GPIO_PS_ALS_INT,
};

static struct platform_device opt_gp2a = {
	.name = "gp2a-opt",
	.id = -1,
	.dev = {
		.platform_data = &gp2a_pdata,
	},
};

static void optical_gpio_init(void)
{
	int ret = gpio_request(GPIO_PS_ALS_EN, "optical_power_supply_on");

	printk(KERN_INFO "%s\n", __func__);

	if (ret)
		printk(KERN_ERR "Failed to request gpio optical power supply.\n");

	/* configuring for gp2a gpio for LEDA power */
	s3c_gpio_cfgpin(GPIO_PS_ALS_EN, S3C_GPIO_OUTPUT);
	gpio_set_value(GPIO_PS_ALS_EN, 0);
	s3c_gpio_setpull(GPIO_PS_ALS_EN, S3C_GPIO_PULL_NONE);
}
#endif

#ifdef CONFIG_SENSORS_AK8975C
static struct akm8975_platform_data akm8975_pdata = {
	.gpio_data_ready_int = GPIO_MSENSOR_INT,
};

static struct i2c_board_info i2c_devs10_emul[] __initdata = {
	{
		I2C_BOARD_INFO("ak8975", 0x0C),
		.platform_data = &akm8975_pdata,
	},
};

#if defined(CONFIG_MACH_C1) || defined(CONFIG_MACH_C1VZW) || \
	defined(CONFIG_MACH_JENGA) || defined(CONFIG_MACH_S2PLUS) || \
	defined(CONFIG_MACH_M0) || defined(CONFIG_MACH_M3) || \
	defined(CONFIG_MACH_C1CTC)
static void ak8975c_gpio_init(void)
{
	int ret = gpio_request(GPIO_MSENSOR_INT, "gpio_akm_int");

	printk(KERN_INFO "%s\n", __func__);

	if (ret)
		printk(KERN_ERR "Failed to request gpio akm_int.\n");

	s5p_register_gpio_interrupt(GPIO_MSENSOR_INT);
	s3c_gpio_setpull(GPIO_MSENSOR_INT, S3C_GPIO_PULL_DOWN);
	s3c_gpio_cfgpin(GPIO_MSENSOR_INT, S3C_GPIO_SFN(0xF));
	i2c_devs10_emul[0].irq = gpio_to_irq(GPIO_MSENSOR_INT);
}
#endif
#endif

#ifdef CONFIG_SENSORS_LPS331
static void lps331_gpio_init(void)
{
	int ret = gpio_request(GPIO_BARO_INT, "lps331_irq");

	printk(KERN_INFO "%s\n", __func__);

	if (ret)
		printk(KERN_ERR "Failed to request gpio lps331_irq\n");

	s3c_gpio_cfgpin(GPIO_BARO_INT, S3C_GPIO_INPUT);
	gpio_set_value(GPIO_BARO_INT, 2);
	s3c_gpio_setpull(GPIO_BARO_INT, S3C_GPIO_PULL_NONE);
	s5p_gpio_set_drvstr(GPIO_BARO_INT, S5P_GPIO_DRVSTR_LV1);
}

static struct lps331ap_platform_data lps331ap_pdata = {
	.irq =	GPIO_BARO_INT,
};

static struct i2c_board_info i2c_devs11_emul[] __initdata = {
	{
		I2C_BOARD_INFO("lps331ap", 0x5D),
		.platform_data = &lps331ap_pdata,
	},
};
#endif

static int __init midas_sensor_init(void)
{
	int ret = 0;
#ifdef CONFIG_SENSORS_LSM330DLC
	/* LSM330DLC (Gyro & Accelerometer Sensor) */
	lsm331dlc_gpio_init();
	i2c_add_devices(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));
#endif

	/* Optical Sensor */
#if defined(CONFIG_SENSORS_GP2A) || defined(CONFIG_SENSORS_CM36651) || \
	defined(CONFIG_SENSORS_CM3663)
	optical_gpio_init();
	i2c_add_devices(9, i2c_devs9_emul, ARRAY_SIZE(i2c_devs9_emul));
#endif

#ifdef CONFIG_SENSORS_AK8975C
#if defined(CONFIG_MACH_C1) || defined(CONFIG_MACH_C1VZW) || \
	defined(CONFIG_MACH_JENGA) || defined(CONFIG_MACH_S2PLUS) || \
	defined(CONFIG_MACH_M0) || defined(CONFIG_MACH_M3) || \
	defined(CONFIG_MACH_C1CTC)
	ak8975c_gpio_init();
#endif
	i2c_add_devices(10, i2c_devs10_emul, ARRAY_SIZE(i2c_devs10_emul));
#endif

#ifdef CONFIG_SENSORS_LPS331
	lps331_gpio_init();
	i2c_add_devices(11, i2c_devs11_emul, ARRAY_SIZE(i2c_devs11_emul));
#endif

#if defined(CONFIG_SENSORS_GP2A) || defined(CONFIG_SENSORS_CM36651)
	ret = platform_device_register(&opt_gp2a);
	if (ret < 0)
		printk(KERN_ERR "failed to register opt_gp2a\n");
#endif

	return ret;
}
module_init(midas_sensor_init);
