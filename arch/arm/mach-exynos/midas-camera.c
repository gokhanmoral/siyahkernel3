/*
 * camera class init
 */

#include <linux/gpio.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/i2c.h>

#include <linux/regulator/machine.h>

#include <plat/devs.h>
#include <plat/csis.h>
#include <plat/pd.h>
#include <plat/gpio-cfg.h>

#include <media/exynos_flite.h>

#if defined(CONFIG_VIDEO_S5C73M3) || defined(CONFIG_VIDEO_SLP_S5C73M3)
#include <media/s5c73m3_platform.h>
#endif

#if defined(CONFIG_VIDEO_M5MO)
#include <mach/regs-gpio.h>
#include <media/m5mo_platform.h>
#endif

#ifdef CONFIG_EXYNOS4_CONTENT_PATH_PROTECTION
#include <mach/secmem.h>
#endif

struct class *camera_class;

static int __init camera_class_init(void)
{
	camera_class = class_create(THIS_MODULE, "camera");

	return 0;
}

subsys_initcall(camera_class_init);

#if defined(CONFIG_VIDEO_FIMC)
/*
 * External camera reset
 * Because the most of cameras take i2c bus signal, so that
 * you have to reset at the boot time for other i2c slave devices.
 * This function also called at fimc_init_camera()
 * Do optimization for cameras on your platform.
 */
int s3c_csis_power(int enable)
{
	struct regulator *regulator;
	int ret = 0;

	/* mipi_1.1v ,mipi_1.8v are always powered-on.
	 * If they are off, we then power them on.
	 */
	if (enable) {
		/* VMIPI_1.0V */
		regulator = regulator_get(NULL, "vmipi_1.0v");
		if (IS_ERR(regulator))
			goto error_out;
		if (!regulator_is_enabled(regulator)) {
			printk(KERN_WARNING "%s: vmipi_1.1v is off. so ON\n",
			       __func__);
			ret = regulator_enable(regulator);
		}
		regulator_put(regulator);

		/* VMIPI_1.8V */
		regulator = regulator_get(NULL, "vmipi_1.8v");
		if (IS_ERR(regulator))
			goto error_out;
		if (!regulator_is_enabled(regulator)) {
			printk(KERN_WARNING "%s: vmipi_1.8v is off. so ON\n",
			       __func__);
			ret = regulator_enable(regulator);
		}
		regulator_put(regulator);
		printk(KERN_WARNING "%s: vmipi_1.0v and vmipi_1.8v were ON\n",
		       __func__);
	}

	return 0;

error_out:
	printk(KERN_ERR "%s: ERROR: failed to check mipi-power\n", __func__);
	return 0;
}

static int s5k6a3_power(int enable)
{
	int ret;
	struct regulator *regulator;
	int err;

	printk(KERN_ERR "%s %s\n", __func__, enable ? "on" : "down");

	if (enable) {
		/* CAM_SENSOR_A2.8V */
#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
		err = gpio_request(GPIO_CAM_IO_EN, "GPY6");
#else
		err = gpio_request(GPIO_CAM_IO_EN, "GPM0");
#endif
		if (err)
			printk(KERN_ERR "#### failed to request GPIO_CAM_IO_EN ####\n");

		ret = gpio_direction_output(GPIO_CAM_IO_EN, 1);
		gpio_free(GPIO_CAM_IO_EN);
		udelay(50);

		/* Camera MCLK */
#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
		err = gpio_request(GPIO_VTCAM_MCLK, "GPJ1");
#else
		err = gpio_request(GPIO_VTCAM_MCLK, "GPM2");
#endif
		if (err)
			printk(KERN_ERR "#### failed to request GPM2_2 ####\n");

#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
		s3c_gpio_cfgpin(GPIO_VTCAM_MCLK, S3C_GPIO_SFN(3));
#else
		s3c_gpio_cfgpin(GPIO_VTCAM_MCLK, S3C_GPIO_SFN(2));
#endif
		s3c_gpio_setpull(GPIO_VTCAM_MCLK, S3C_GPIO_PULL_NONE);
		gpio_free(GPIO_VTCAM_MCLK);

		/* VT_CAM_1.8V */
		regulator = regulator_get(NULL, "vt_cam_1.8v");
		if (IS_ERR(regulator)) {
			printk(KERN_INFO
			       "smdk4212_cam1_reset regulator get err\n");
			goto out;
		}
		if (!regulator_is_enabled(regulator)) {
			printk(KERN_WARNING "%s: vt_cam_1.8v is off. so ON\n",
			       __func__);
			ret = regulator_enable(regulator);
		}
		regulator_put(regulator);

		/* Camera reset */
		err = gpio_request(GPIO_CAM_VT_nRST, "GPM1");
		if (err)
			printk(KERN_ERR "#### failed to request GPIO_CAM_VT_nRST ####\n");

		s3c_gpio_setpull(GPIO_CAM_VT_nRST, S3C_GPIO_PULL_NONE);
		gpio_direction_output(GPIO_CAM_VT_nRST, 0);
		gpio_direction_output(GPIO_CAM_VT_nRST, 1);
		gpio_free(GPIO_CAM_VT_nRST);

		s3c_csis_power(1);
	} else {
		/* Camera reset */
		err = gpio_request(GPIO_CAM_VT_nRST, "GPM1");
		if (err)
			printk(KERN_ERR "#### failed to request GPIO_CAM_VT_nRST ####\n");

		s3c_gpio_setpull(GPIO_CAM_VT_nRST, S3C_GPIO_PULL_NONE);
		gpio_direction_output(GPIO_CAM_VT_nRST, 0);
		gpio_free(GPIO_CAM_VT_nRST);

		/* VT_CAM_1.8V */
		regulator = regulator_get(NULL, "vt_cam_1.8v");
		if (IS_ERR(regulator)) {
			printk(KERN_INFO
			       "smdk4212_cam1_reset regulator get err\n");
			goto out;
		}
		if (regulator_is_enabled(regulator)) {
			printk(KERN_WARNING "%s: vt_cam_1.8v is on. so OFF\n",
			       __func__);
			ret = regulator_disable(regulator);
		}
		regulator_put(regulator);

		/* Camera MCLK */
#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
		err = gpio_request(GPIO_VTCAM_MCLK, "GPM2");
#else
		err = gpio_request(GPIO_VTCAM_MCLK, "GPJ1");
#endif
		if (err)
			printk(KERN_ERR "#### failed to request GPM2_2 ####\n");

		s3c_gpio_cfgpin(GPIO_VTCAM_MCLK, S3C_GPIO_INPUT);
		s3c_gpio_setpull(GPIO_VTCAM_MCLK, S3C_GPIO_PULL_DOWN);
		gpio_free(GPIO_VTCAM_MCLK);

		/* CAM_SENSOR_A2.8V */
#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
		err = gpio_request(GPIO_CAM_IO_EN, "GPY6");
#else
		err = gpio_request(GPIO_CAM_IO_EN, "GPM0");
#endif
		if (err)
			printk(KERN_ERR "#### failed to request GPIO_CAM_IO_EN ####\n");

		ret = gpio_direction_output(GPIO_CAM_IO_EN, 0);
		gpio_free(GPIO_CAM_IO_EN);

		s3c_csis_power(0);
	}

out:
	return 0;
}

#ifdef WRITEBACK_ENABLED
static int get_i2c_busnum_writeback(void)
{
	return 0;
}

static struct i2c_board_info writeback_i2c_info = {
	I2C_BOARD_INFO("WriteBack", 0x0),
};

static struct s3c_platform_camera writeback = {
	.id		= CAMERA_WB,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.get_i2c_busnum = get_i2c_busnum_writeback,
	.info		= &writeback_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_YUV444,
	.line_length	= 800,
	.width		= 480,
	.height		= 800,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 480,
		.height	= 800,
	},

	.initialized	= 0,
};
#endif

#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS
#ifdef CONFIG_VIDEO_S5K6A3
static struct s3c_platform_camera s5k6a3 = {
	.id		= CAMERA_CSI_D,
#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
	.clk_name	= "sclk_cam1",
#else
	.clk_name	= "sclk_cam0",
#endif
	.cam_power	= s5k6a3_power,
	.type		= CAM_TYPE_MIPI,
	.fmt		= MIPI_CSI_RAW10,
	.order422	= CAM_ORDER422_8BIT_YCBYCR,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.line_length	= 1920,
	.width		= 1920,
	.height		= 1080,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 1920,
		.height	= 1080,
	},
	.srclk_name	= "xusbxti",
	.clk_rate	= 24000000,
	.mipi_lanes	= 1,
	.mipi_settle	= 12,
	.mipi_align	= 24,

	.initialized	= 0,
	.flite_id	= FLITE_IDX_B,
	.use_isp	= true,
	.sensor_index	= 102,
};
#endif
#endif

#if defined(CONFIG_VIDEO_S5C73M3) || defined(CONFIG_VIDEO_SLP_S5C73M3)

#define CAM_CHECK_ERR_RET(x, msg)					\
	if (unlikely((x) < 0)) {					\
		printk(KERN_ERR "\nfail to %s: err = %d\n", msg, x);	\
		return x;						\
	}
#define CAM_CHECK_ERR(x, msg)						\
	if (unlikely((x) < 0)) {					\
		printk(KERN_ERR "\nfail to %s: err = %d\n", msg, x);	\
	}

#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
static int vddCore = 1200000;
#else
static int vddCore = 1150000;
#endif
static bool isVddCoreSet;
static void s5c73m3_set_vdd_core(int level)
{
	vddCore = level;
	isVddCoreSet = true;
	printk(KERN_ERR "%s : %d\n", __func__, vddCore);
}

static bool s5c73m3_is_vdd_core_set(void)
{
	return isVddCoreSet;
}

static int s5c73m3_is_isp_reset(void)
{
	int ret = 0;

#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
		ret = gpio_request(GPIO_ISP_RESET, "GPY3");
#else
		ret = gpio_request(GPIO_ISP_RESET, "GPF1");
#endif
	if (ret) {
		printk(KERN_ERR "faile to request gpio(GPIO_ISP_RESET)\n");
		return ret;
	}

	/* ISP_RESET */
	ret = gpio_direction_output(GPIO_ISP_RESET, 0);
	CAM_CHECK_ERR_RET(ret, "output GPIO_ISP_RESET");
	udelay(10);	/* 200 cycle */
	ret = gpio_direction_output(GPIO_ISP_RESET, 1);
	CAM_CHECK_ERR_RET(ret, "output GPIO_ISP_RESET");
	udelay(10);	/* 200 cycle */

	gpio_free(GPIO_ISP_RESET);

	return ret;
}

static int s5c73m3_gpio_request(void)
{
	int ret = 0;

#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
	ret = gpio_request(GPIO_ISP_STANDBY, "GPY5");
#else
	ret = gpio_request(GPIO_ISP_STANDBY, "GPM0");
#endif
	if (ret) {
		printk(KERN_ERR "faile to request gpio(GPIO_ISP_STANDBY)\n");
		return ret;
	}

#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
	ret = gpio_request(GPIO_ISP_RESET, "GPY3");
#else
	ret = gpio_request(GPIO_ISP_RESET, "GPF1");
#endif
	if (ret) {
		printk(KERN_ERR "faile to request gpio(GPIO_ISP_RESET)\n");
		return ret;
	}

	/* SENSOR_A2.8V */
#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
	ret = gpio_request(GPIO_CAM_IO_EN, "GPY6");
#else
	ret = gpio_request(GPIO_CAM_IO_EN, "GPM0");
#endif
	if (ret) {
		printk(KERN_ERR "fail to request gpio(GPIO_CAM_IO_EN)\n");
		return ret;
	}

#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
	ret = gpio_request(GPIO_CAM_AF_EN, "GPY6");
#else
	ret = gpio_request(GPIO_CAM_AF_EN, "GPM0");
#endif
	if (ret) {
		printk(KERN_ERR "faile to request gpio(GPIO_CAM_AF_EN)\n");
		return ret;
	}

#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
	ret = gpio_request(GPIO_ISP_CORE_EN, "GPY6");
#else
	ret = gpio_request(GPIO_ISP_CORE_EN, "GPM0");
#endif
	if (ret) {
		printk(KERN_ERR "fail to request gpio(GPIO_ISP_CORE_EN)\n");
		return ret;
	}

	return ret;
}

static int s5c73m3_power_on(void)
{
	struct regulator *regulator;
	int ret = 0;

	printk(KERN_DEBUG "%s: in\n", __func__);
	printk(KERN_DEBUG "vddCore : %d\n", vddCore);

	s5c73m3_gpio_request();

	/* CAM_ISP_CORE_1.2V */
	ret = gpio_direction_output(GPIO_ISP_CORE_EN, 1);
	CAM_CHECK_ERR_RET(ret, "output GPIO_ISP_CORE_EN");

	regulator = regulator_get(NULL, "cam_isp_core_1.2v");
	if (IS_ERR(regulator))
		return -ENODEV;
	regulator_set_voltage(regulator, vddCore, vddCore);
	ret = regulator_enable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "enable cam_isp_core_1.2v");

	/* CAM_SENSOR_A2.8V */
	ret = gpio_direction_output(GPIO_CAM_IO_EN, 1);
	CAM_CHECK_ERR_RET(ret, "output IO_EN");

	/* CAM_SENSOR_CORE_1.2V */
	regulator = regulator_get(NULL, "cam_sensor_core_1.2v");
	if (IS_ERR(regulator))
		return -ENODEV;
	ret = regulator_enable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "enable cam_sensor_core_1.2v");
       /* delay is needed : pmu control is slower than gpio control*/
	mdelay(8);

	/* MCLK */
	ret = s3c_gpio_cfgpin(GPIO_CAM_MCLK, S3C_GPIO_SFN(2));
	CAM_CHECK_ERR_RET(ret, "cfg mclk");
	s3c_gpio_setpull(GPIO_CAM_MCLK, S3C_GPIO_PULL_NONE);

	/* CAM_AF_2.8V */
	ret = gpio_direction_output(GPIO_CAM_AF_EN, 1);
	CAM_CHECK_ERR_RET(ret, "output GPIO_CAM_AF_EN");
	udelay(2000);

#if 0
	/* VT_CORE_1.8V */
	regulator = regulator_get(NULL, "vt_cam_1.8v");
	if (IS_ERR(regulator))
		return -ENODEV;
	ret = regulator_enable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "enable vt_cam_1.8v");
#endif

	/* CAM_ISP_SENSOR_1.8V */
	regulator = regulator_get(NULL, "cam_isp_sensor_1.8v");
	if (IS_ERR(regulator))
		return -ENODEV;
	ret = regulator_enable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "enable cam_isp_sensor_1.8v");

	/* CAM_ISP_MIPI_1.2V */
	regulator = regulator_get(NULL, "cam_isp_mipi_1.2v");
	if (IS_ERR(regulator))
		return -ENODEV;
	ret = regulator_enable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "enable cam_isp_mipi_1.2v");

	/* ISP_STANDBY */
	ret = gpio_direction_output(GPIO_ISP_STANDBY, 1);
	CAM_CHECK_ERR_RET(ret, "output GPIO_ISP_STANDBY");
	udelay(100);		/* 2000 cycle */

	/* ISP_RESET */
	ret = gpio_direction_output(GPIO_ISP_RESET, 1);
	CAM_CHECK_ERR_RET(ret, "output GPIO_ISP_RESET");
	udelay(10);		/* 200 cycle */

	/* EVT0 : set VDD_INT as 1.3V when entering camera */
#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
	regulator = regulator_get(NULL, "vdd_int");
	if (IS_ERR(regulator))
		return -ENODEV;
	regulator_set_voltage(regulator, 1300000, 1300000);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "set vdd_int as 1.3V");
#else
	/*M0, C1 REV00*/
	if (system_rev == 0x03) {
		regulator = regulator_get(NULL, "vdd_int");
		if (IS_ERR(regulator))
			return -ENODEV;
		regulator_set_voltage(regulator, 1300000, 1300000);
		regulator_put(regulator);
		CAM_CHECK_ERR_RET(ret, "set vdd_int as 1.3V");
	}
#endif

	gpio_free(GPIO_ISP_STANDBY);
	gpio_free(GPIO_ISP_RESET);
	gpio_free(GPIO_CAM_IO_EN);
	gpio_free(GPIO_CAM_AF_EN);
	gpio_free(GPIO_ISP_CORE_EN);

	return ret;
}

static int s5c73m3_power_down(void)
{
	struct regulator *regulator;
	int ret = 0;

	printk(KERN_DEBUG "%s: in\n", __func__);

	s5c73m3_gpio_request();

	/* ISP_STANDBY */
	ret = gpio_direction_output(GPIO_ISP_STANDBY, 0);
	CAM_CHECK_ERR_RET(ret, "output GPIO_ISP_STANDBY");
	udelay(2);		/* 40 cycle */

	/* ISP_RESET */
	ret = gpio_direction_output(GPIO_ISP_RESET, 0);
	CAM_CHECK_ERR_RET(ret, "output GPIO_ISP_RESET");

	/* CAM_AF_2.8V */
	ret = gpio_direction_output(GPIO_CAM_AF_EN, 0);
	CAM_CHECK_ERR_RET(ret, "output GPIO_CAM_AF_EN");

	/* CAM_ISP_MIPI_1.2V */
	regulator = regulator_get(NULL, "cam_isp_mipi_1.2v");
	if (IS_ERR(regulator))
		return -ENODEV;
	if (regulator_is_enabled(regulator))
		ret = regulator_force_disable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR(ret, "disable cam_isp_mipi_1.2v");
	udelay(10);		/* 200 cycle */

	/* CAM_ISP_SENSOR_1.8V */
	regulator = regulator_get(NULL, "cam_isp_sensor_1.8v");
	if (IS_ERR(regulator))
		return -ENODEV;
	if (regulator_is_enabled(regulator))
		ret = regulator_force_disable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR(ret, "disable cam_isp_sensor_1.8v");

#if 0
	/* VT_CORE_1.8V */
	regulator = regulator_get(NULL, "vt_cam_1.8v");
	if (IS_ERR(regulator))
		return -ENODEV;
	if (regulator_is_enabled(regulator))
		ret = regulator_force_disable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR(ret, "disable vt_cam_1.8v");
#endif

	/* MCLK */
	ret = s3c_gpio_cfgpin(GPIO_CAM_MCLK, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_CAM_MCLK, S3C_GPIO_PULL_DOWN);
	CAM_CHECK_ERR(ret, "cfg mclk");

	/* CAM_SENSOR_CORE_1.2V */
	regulator = regulator_get(NULL, "cam_sensor_core_1.2v");
	if (IS_ERR(regulator))
		return -ENODEV;
	if (regulator_is_enabled(regulator))
		ret = regulator_force_disable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR(ret, "disable cam_sensor_core_1.2v");

	/* CAM_SENSOR_A2.8V */
	ret = gpio_direction_output(GPIO_CAM_IO_EN, 0);
	CAM_CHECK_ERR_RET(ret, "output GPIO_CAM_IO_EN");

	/* CAM_ISP_CORE_1.2V */
	regulator = regulator_get(NULL, "cam_isp_core_1.2v");
	if (IS_ERR(regulator))
		return -ENODEV;
	if (regulator_is_enabled(regulator))
		ret = regulator_force_disable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR(ret, "disable cam_isp_core_1.2v");

	ret = gpio_direction_output(GPIO_ISP_CORE_EN, 0);
	CAM_CHECK_ERR_RET(ret, "output GPIO_CAM_ISP_CORE_EN");

	/* EVT0 : set VDD_INT as 1.0V when exiting camera */
#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
	regulator = regulator_get(NULL, "vdd_int");
	if (IS_ERR(regulator))
		return -ENODEV;
	regulator_set_voltage(regulator, 1000000, 1000000);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "set vdd_int as 1.0V");
#else
	/*M0, C1 REV00*/
	if (system_rev == 0x03) {
		regulator = regulator_get(NULL, "vdd_int");
		if (IS_ERR(regulator))
			return -ENODEV;
		regulator_set_voltage(regulator, 1000000, 1000000);
		regulator_put(regulator);
		CAM_CHECK_ERR_RET(ret, "set vdd_int as 1.0V");
	}
#endif

	gpio_free(GPIO_ISP_STANDBY);
	gpio_free(GPIO_ISP_RESET);
	gpio_free(GPIO_CAM_IO_EN);
	gpio_free(GPIO_CAM_AF_EN);
	gpio_free(GPIO_ISP_CORE_EN);

	return ret;
}

static int s5c73m3_power(int enable)
{
	int ret = 0;

	printk(KERN_ERR "%s %s\n", __func__, enable ? "on" : "down");

	if (enable) {
		ret = s5c73m3_power_on();

		if (unlikely(ret))
			goto error_out;
	} else
		ret = s5c73m3_power_down();

	ret = s3c_csis_power(enable);

error_out:
	return ret;
}

static int s5c73m3_get_i2c_busnum(void)
{
#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
	return 0;
#else
	if (system_rev == 0x03) /*M0, M1 REV00*/
		return 18;
	else
		return 0;
#endif
}

static struct s5c73m3_platform_data s5c73m3_plat = {
	.default_width = 640,	/* 1920 */
	.default_height = 480,	/* 1080 */
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 24000000,
	.is_mipi = 1,
	.set_vdd_core = s5c73m3_set_vdd_core,
	.is_vdd_core_set = s5c73m3_is_vdd_core_set,
	.is_isp_reset = s5c73m3_is_isp_reset,
};

static struct i2c_board_info s5c73m3_i2c_info = {
#if defined(CONFIG_MACH_MIDAS_02_BD) || defined(CONFIG_GPIO_MIDAS_02_BD)
	I2C_BOARD_INFO("S5C73M3", 0x5A >> 1),
#else
	I2C_BOARD_INFO("S5C73M3", 0x78 >> 1),
#endif
	.platform_data = &s5c73m3_plat,
};

static struct s3c_platform_camera s5c73m3 = {
	.id = CAMERA_CSI_C,
	.clk_name = "sclk_cam0",
	.get_i2c_busnum = s5c73m3_get_i2c_busnum,
	.cam_power = s5c73m3_power,
	.type = CAM_TYPE_MIPI,
	.fmt = MIPI_CSI_YCBCR422_8BIT,
	.order422 = CAM_ORDER422_8BIT_YCBYCR,
	.info = &s5c73m3_i2c_info,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.srclk_name = "xusbxti",	/* "mout_mpll" */
	.clk_rate = 24000000,	/* 48000000 */
	.line_length = 1920,
	.width = 640,
	.height = 480,
	.window = {
		.left = 0,
		.top = 0,
		.width = 640,
		.height = 480,
	},

	.mipi_lanes = 4,
	.mipi_settle = 12,
	.mipi_align = 32,

	/* Polarity */
	.inv_pclk = 1,
	.inv_vsync = 1,
	.inv_href = 0,
	.inv_hsync = 0,
	.reset_camera = 0,
	.initialized = 0,
};
#endif

#ifdef CONFIG_VIDEO_M5MO
#define CAM_CHECK_ERR_RET(x, msg)	\
	if (unlikely((x) < 0)) { \
		printk(KERN_ERR "\nfail to %s: err = %d\n", msg, x); \
		return x; \
	}
#define CAM_CHECK_ERR(x, msg)	\
		if (unlikely((x) < 0)) { \
			printk(KERN_ERR "\nfail to %s: err = %d\n", msg, x); \
		}

static int m5mo_get_i2c_busnum(void)
{
#ifdef CONFIG_VIDEO_M5MO_USE_SWI2C
	return 25;
#else
	return 0;
#endif
}

static int m5mo_power_on(void)
{
	struct regulator *regulator;
	int ret = 0;

	printk(KERN_DEBUG "%s: in\n", __func__);

	ret = gpio_request(GPIO_CAM_VT_nSTBY, "GPL2");
	if (ret) {
		printk(KERN_ERR "faile to request gpio(GPIO_CAM_VGA_nSTBY)\n");
		return ret;
	}
	ret = gpio_request(GPIO_CAM_VT_nRST, "GPL2");
	if (ret) {
		printk(KERN_ERR "faile to request gpio(GPIO_CAM_VGA_nRST)\n");
		return ret;
	}
	ret = gpio_request(GPIO_ISP_CORE_EN, "GPM0");
	if (ret) {
		printk(KERN_ERR "fail to request gpio(CAM_SENSOR_CORE)\n");
		return ret;
	}
	ret = gpio_request(GPIO_ISP_RESET, "GPY3");
	if (ret) {
		printk(KERN_ERR "faile to request gpio(GPIO_ISP_RESET)\n");
		return ret;
	}
	/* CAM_VT_nSTBY low */
	ret = gpio_direction_output(GPIO_CAM_VT_nSTBY, 0);
	CAM_CHECK_ERR_RET(ret, "output VT_nSTBY");

	/* CAM_VT_nRST	low */
	gpio_direction_output(GPIO_CAM_VT_nRST, 0);
	CAM_CHECK_ERR_RET(ret, "output VT_nRST");
	udelay(10);

	/* CAM_ISP_CORE_1.2V */
	ret = gpio_direction_output(GPIO_ISP_CORE_EN, 1);
	CAM_CHECK_ERR_RET(ret, "output GPIO_ISP_CORE_EN");
	/* No delay */
	/* CAM_SENSOR_CORE_1.2V */
	regulator = regulator_get(NULL, "cam_isp_core_1.2v");
	if (IS_ERR(regulator))
		return -ENODEV;
	ret = regulator_enable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "enable cam_isp_core_1.2v");
	udelay(10);
	regulator = regulator_get(NULL, "cam_sensor_core_1.2v");
	if (IS_ERR(regulator)) {
		CAM_CHECK_ERR_RET(ret, "output Err cam_sensor_core_1.2v");
		return -ENODEV;
	}
	ret = regulator_enable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "enable cam_sensor_core_1.2v");
	udelay(10);

	/* CAM_SENSOR_A2.8V */
	regulator = regulator_get(NULL, "cam_sensor_a2.8v");
	if (IS_ERR(regulator)) {
		CAM_CHECK_ERR_RET(ret, "output Err cam_sensor_a2.8v");
		return -ENODEV;
	}
	ret = regulator_enable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "enable cam_sensor_a2.8v");
	/* it takes about 100us at least during level transition.*/
	udelay(160); /* 130us -> 160us */
	/* VT_CAM_DVDD_1.8V */
	regulator = regulator_get(NULL, "vt_cam_dvdd_1.8v");
	if (IS_ERR(regulator))
		return -ENODEV;
	ret = regulator_enable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "enable vt_cam_dvdd_1.8v");
	udelay(10);

	/* CAM_AF_2.8V */
	regulator = regulator_get(NULL, "cam_af_2.8v");
	if (IS_ERR(regulator))
		return -ENODEV;
	ret = regulator_enable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR(ret, "output cam_af_2.8v");
	mdelay(7);

	/* VT_CAM_1.8V */
	regulator = regulator_get(NULL, "vt_cam_1.8v");
	if (IS_ERR(regulator)) {
		CAM_CHECK_ERR_RET(ret, "output Err vt_cam_1.8v");
		return -ENODEV;
	}
	ret = regulator_enable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "enable vt_cam_1.8v");
	udelay(20);

	/* CAM_ISP_1.8V */
	regulator = regulator_get(NULL, "cam_isp_1.8v");
	if (IS_ERR(regulator)) {
		CAM_CHECK_ERR_RET(ret, "output Err cam_isp_1.8v");
		return -ENODEV;
	}
	ret = regulator_enable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "enable cam_isp_1.8v");
	udelay(120); /* at least */

	/* CAM_ISP_SEN_IO_1.8V */
	regulator = regulator_get(NULL, "cam_isp_sensor_1.8v");
	if (IS_ERR(regulator)) {
		CAM_CHECK_ERR_RET(ret, "output Err cam_isp_sensor_1.8v");
		return -ENODEV;
	}
	ret = regulator_enable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR_RET(ret, "enable cam_isp_sensor_1.8v");
	udelay(30);

	/* MCLK */
	ret = s3c_gpio_cfgpin(GPIO_CAM_MCLK, S3C_GPIO_SFN(2));
	CAM_CHECK_ERR_RET(ret, "cfg mclk");
	s3c_gpio_setpull(GPIO_CAM_MCLK, S3C_GPIO_PULL_NONE);
	udelay(70);
	/* ISP_RESET */
	ret = gpio_direction_output(GPIO_ISP_RESET, 1);
	CAM_CHECK_ERR_RET(ret, "output reset");
	mdelay(4);

	gpio_free(GPIO_CAM_VT_nSTBY);
	gpio_free(GPIO_CAM_VT_nRST);
	gpio_free(GPIO_ISP_CORE_EN);
	gpio_free(GPIO_ISP_RESET);

	return ret;
}

static int m5mo_power_down(void)
{
	struct regulator *regulator;
	int ret = 0;

	printk(KERN_DEBUG "%s: in\n", __func__);

	ret = gpio_request(GPIO_CAM_VT_nSTBY, "GPL2");
	if (ret) {
		printk(KERN_ERR "faile to request gpio(GPIO_CAM_VGA_nSTBY)\n");
		return ret;
	}
	ret = gpio_request(GPIO_CAM_VT_nRST, "GPL2");
	if (ret) {
		printk(KERN_ERR "faile to request gpio(GPIO_CAM_VGA_nRST)\n");
		return ret;
	}
	ret = gpio_request(GPIO_ISP_CORE_EN, "GPM0");
	if (ret) {
		printk(KERN_ERR "fail to request gpio(CAM_SENSOR_CORE)\n");
		return ret;
	}
	ret = gpio_request(GPIO_ISP_RESET, "GPY3");
	if (ret) {
		printk(KERN_ERR "faile to request gpio(GPIO_ISP_RESET)\n");
		return ret;
	}

	/* s3c_i2c0_force_stop(); */

	mdelay(3);

	/* ISP_RESET */
	ret = gpio_direction_output(GPIO_ISP_RESET, 0);
	CAM_CHECK_ERR(ret, "output reset");
	mdelay(2);

	/* MCLK */
	ret = s3c_gpio_cfgpin(GPIO_CAM_MCLK, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_CAM_MCLK, S3C_GPIO_PULL_DOWN);
	CAM_CHECK_ERR(ret, "cfg mclk");
	udelay(20);

	/* CAM_AF_2.8V */
	regulator = regulator_get(NULL, "cam_af_2.8v");
	if (IS_ERR(regulator))
		return -ENODEV;
	if (regulator_is_enabled(regulator))
		ret = regulator_force_disable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR(ret, "disable cam_af_2.8v");

	/* CAM_ISP_SEN_IO_1.8V */
	regulator = regulator_get(NULL, "cam_isp_sensor_1.8v");
	if (IS_ERR(regulator))
		return -ENODEV;
	if (regulator_is_enabled(regulator))
		ret = regulator_force_disable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR(ret, "disable, cam_isp_sensor_1.8v");
	udelay(10);

	/* CAM_ISP_1.8V */
	regulator = regulator_get(NULL, "cam_isp_1.8v");
	if (IS_ERR(regulator))
		return -ENODEV;
	if (regulator_is_enabled(regulator))
		ret = regulator_force_disable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR(ret, "disable cam_isp_1.8v");
	udelay(500); /* 100us -> 500us */

	/* VT_CAM_1.8V */
	regulator = regulator_get(NULL, "vt_cam_1.8v");
	if (IS_ERR(regulator))
		return -ENODEV;
	if (regulator_is_enabled(regulator))
		ret = regulator_disable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR(ret, "disable vt_cam_1.8v");
	udelay(250); /* 10us -> 250us */

	/* VT_CAM_DVDD_1.8V */
	regulator = regulator_get(NULL, "vt_cam_dvdd_1.8v");
	if (IS_ERR(regulator))
		return -ENODEV;
	if (regulator_is_enabled(regulator))
		ret = regulator_disable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR(ret, "disable vt_cam_dvdd_1.8v");
	udelay(300); /*10 -> 300 us */

	/* CAM_SENSOR_A2.8V */
	regulator = regulator_get(NULL, "cam_sensor_a2.8v");
	if (IS_ERR(regulator))
		return -ENODEV;
	if (regulator_is_enabled(regulator))
		ret = regulator_force_disable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR(ret, "disable cam_sensor_a2.8v");
	udelay(800);

	/* CAM_SENSOR_CORE_1.2V */
	regulator = regulator_get(NULL, "cam_sensor_core_1.2v");
	if (IS_ERR(regulator))
		return -ENODEV;
	if (regulator_is_enabled(regulator))
		ret = regulator_force_disable(regulator);
	regulator_put(regulator);
	CAM_CHECK_ERR(ret, "disable cam_sensor_core_1.2v");
	udelay(5);

	/* CAM_ISP_CORE_1.2V */
	ret = gpio_direction_output(GPIO_ISP_CORE_EN, 0);
	CAM_CHECK_ERR(ret, "output ISP_CORE");

	gpio_free(GPIO_CAM_VT_nSTBY);
	gpio_free(GPIO_CAM_VT_nRST);
	gpio_free(GPIO_ISP_CORE_EN);
	gpio_free(GPIO_ISP_RESET);

	return ret;
}

static int m5mo_flash_power(int enable)
{
/* TODO */
	return 0;
}

static int m5mo_power(int enable)
{
	int ret = 0;

	printk(KERN_ERR "%s %s\n", __func__, enable ? "on" : "down");
	if (enable) {
		ret = m5mo_power_on();
		if (unlikely(ret))
			goto error_out;
	} else
		ret = m5mo_power_down();

	ret = s3c_csis_power(enable);
	m5mo_flash_power(enable);

error_out:
	return ret;
}

static int m5mo_config_isp_irq(void)
{
	s3c_gpio_cfgpin(GPIO_ISP_INT, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(GPIO_ISP_INT, S3C_GPIO_PULL_NONE);
	return 0;
}

static struct m5mo_platform_data m5mo_plat = {
	.default_width = 640, /* 1920 */
	.default_height = 480, /* 1080 */
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 24000000,
	.is_mipi = 1,
	.config_isp_irq = m5mo_config_isp_irq,
	.irq = IRQ_EINT(24),
};

static struct i2c_board_info  m5mo_i2c_info = {
	I2C_BOARD_INFO("M5MO", 0x1F),
	.platform_data = &m5mo_plat,
};

static struct s3c_platform_camera m5mo = {
	.id		= CAMERA_CSI_C,
	.clk_name	= "sclk_cam0",
	.get_i2c_busnum	= m5mo_get_i2c_busnum,
	.cam_power	= m5mo_power, /*smdkv310_mipi_cam0_reset,*/
	.type		= CAM_TYPE_MIPI,
	.fmt		= ITU_601_YCBCR422_8BIT, /*MIPI_CSI_YCBCR422_8BIT*/
	.order422	= CAM_ORDER422_8BIT_CBYCRY,

	.info		= &m5mo_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "xusbxti", /* "mout_mpll" */
	.clk_rate	= 24000000, /* 48000000 */
	.line_length	= 1920,
	.width		= 640,
	.height		= 480,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 640,
		.height	= 480,
	},

	.mipi_lanes	= 2,
	.mipi_settle	= 12,
	.mipi_align	= 32,

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,
	.reset_camera	= 0,
	.initialized	= 0,
};
#endif /* #ifdef CONFIG_VIDEO_M5MO */

/* Interface setting */
static struct s3c_platform_fimc fimc_plat = {
	.default_cam = CAMERA_CSI_D,
#ifdef WRITEBACK_ENABLED
	.default_cam = CAMERA_WB,
#endif
	.camera = {
#if defined(CONFIG_VIDEO_S5C73M3) || defined(CONFIG_VIDEO_SLP_S5C73M3)
		&s5c73m3,
#endif
#ifdef CONFIG_VIDEO_S5K6A3
		&s5k6a3,
#endif
#if defined(CONFIG_VIDEO_M5MO)
		&m5mo,
#endif
#ifdef WRITEBACK_ENABLED
		&writeback,
#endif
	},
	.hw_ver		= 0x51,
};
#endif /* CONFIG_VIDEO_FIMC */

#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
static void __set_flite_camera_config(struct exynos_platform_flite *data,
			u32 active_index, u32 max_cam)
{
	data->active_cam_index = active_index;
	data->num_clients = max_cam;
}

static void __init smdk4x12_set_camera_flite_platdata(void)
{
	int flite0_cam_index = 0;
	int flite1_cam_index = 0;
#ifdef CONFIG_VIDEO_S5K6A3
	exynos_flite1_default_data.cam[flite1_cam_index++] = &s5k6a3;
#endif
	__set_flite_camera_config(&exynos_flite0_default_data, 0, flite0_cam_index);
	__set_flite_camera_config(&exynos_flite1_default_data, 0, flite1_cam_index);
}
#endif

void __init midas_camera_init(void)
{
#ifdef CONFIG_VIDEO_FIMC
	s3c_fimc0_set_platdata(&fimc_plat);
	s3c_fimc1_set_platdata(&fimc_plat);
	s3c_fimc2_set_platdata(NULL);
	s3c_fimc3_set_platdata(NULL);
#ifdef CONFIG_EXYNOS_DEV_PD
	s3c_device_fimc0.dev.parent = &exynos4_device_pd[PD_CAM].dev;
	s3c_device_fimc1.dev.parent = &exynos4_device_pd[PD_CAM].dev;
	s3c_device_fimc2.dev.parent = &exynos4_device_pd[PD_CAM].dev;
	s3c_device_fimc3.dev.parent = &exynos4_device_pd[PD_CAM].dev;
#ifdef CONFIG_EXYNOS4_CONTENT_PATH_PROTECTION
	secmem.parent = &exynos4_device_pd[PD_CAM].dev;
#endif
#endif
#ifdef CONFIG_VIDEO_FIMC_MIPI
	s3c_csis0_set_platdata(NULL);
	s3c_csis1_set_platdata(NULL);
#ifdef CONFIG_EXYNOS_DEV_PD
	s3c_device_csis0.dev.parent = &exynos4_device_pd[PD_CAM].dev;
	s3c_device_csis1.dev.parent = &exynos4_device_pd[PD_CAM].dev;
#endif
#endif
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
	smdk4x12_set_camera_flite_platdata();
	s3c_set_platdata(&exynos_flite0_default_data,
		sizeof(exynos_flite0_default_data), &exynos_device_flite0);
	s3c_set_platdata(&exynos_flite1_default_data,
		sizeof(exynos_flite1_default_data), &exynos_device_flite1);
#endif
#endif /* CONFIG_VIDEO_FIMC */
}
