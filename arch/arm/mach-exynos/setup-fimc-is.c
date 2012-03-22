/* linux/arch/arm/mach-exynos/setup-fimc-is.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * FIMC-IS gpio and clock configuration
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <plat/clock.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>
#include <plat/map-s5p.h>
#include <plat/cpu.h>
#include <mach/map.h>

struct platform_device; /* don't need the contents */

void exynos_fimc_is_cfg_gpio(struct platform_device *pdev)
{
	int ret;
	/* 1. UART setting for FIMC-IS */
	/* GPM3[5] : TXD_UART_ISP */
	ret = gpio_request(EXYNOS4212_GPM3(5), "GPM3");
	if (ret)
		printk(KERN_ERR "#### failed to request GPM3_5 ####\n");
	s3c_gpio_cfgpin(EXYNOS4212_GPM3(5), (0x3<<20));
	s3c_gpio_setpull(EXYNOS4212_GPM3(5), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4212_GPM3(5));

	/* GPM3[7] : RXD_UART_ISP */
	ret = gpio_request(EXYNOS4212_GPM3(7), "GPM3");
	if (ret)
		printk(KERN_ERR "#### failed to request GPM3_7 ####\n");
	s3c_gpio_cfgpin(EXYNOS4212_GPM3(7), (0x3<<28));
	s3c_gpio_setpull(EXYNOS4212_GPM3(7), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4212_GPM3(7));

	/* 2. GPIO setting for FIMC-IS */
	ret = gpio_request(EXYNOS4212_GPM4(0), "GPM4");
	if (ret)
		printk(KERN_ERR "#### failed to request GPM4_0 ####\n");
	s3c_gpio_cfgpin(EXYNOS4212_GPM4(0), (0x2<<0));
	s3c_gpio_setpull(EXYNOS4212_GPM4(0), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4212_GPM4(0));

	ret = gpio_request(EXYNOS4212_GPM4(1), "GPM4");
	if (ret)
		printk(KERN_ERR "#### failed to request GPM4_1 ####\n");
	s3c_gpio_cfgpin(EXYNOS4212_GPM4(1), (0x2<<4));
	s3c_gpio_setpull(EXYNOS4212_GPM4(1), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4212_GPM4(1));

	ret = gpio_request(EXYNOS4212_GPM4(2), "GPM4");
	if (ret)
		printk(KERN_ERR "#### failed to request GPM4_2 ####\n");
	s3c_gpio_cfgpin(EXYNOS4212_GPM4(2), (0x2<<8));
	s3c_gpio_setpull(EXYNOS4212_GPM4(2), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4212_GPM4(2));

	ret = gpio_request(EXYNOS4212_GPM4(3), "GPM4");
	if (ret)
		printk(KERN_ERR "#### failed to request GPM4_3 ####\n");
	s3c_gpio_cfgpin(EXYNOS4212_GPM4(3), (0x2<<12));
	s3c_gpio_setpull(EXYNOS4212_GPM4(3), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4212_GPM4(3));
}

int exynos_fimc_is_cfg_clk(struct platform_device *pdev)
{
	struct clk *aclk_mcuisp_muxed = NULL;
	struct clk *aclk_mcuisp_div0 = NULL;
	struct clk *aclk_mcuisp_div1 = NULL;
	struct clk *aclk_200 = NULL;
	struct clk *aclk_200_div0 = NULL;
	struct clk *aclk_200_div1 = NULL;
	struct clk *sclk_uart_isp = NULL;
	struct clk *sclk_uart_isp_src = NULL;
	struct clk *sclk_pwm_isp = NULL;
	struct clk *sclk_pwm_isp_src = NULL;
	/*
	 * initialize Clocks
	*/
	/* 1. MCUISP */
	aclk_mcuisp_muxed = clk_get(&pdev->dev, "aclk_400_muxed");
	if (IS_ERR(aclk_mcuisp_muxed))
		printk(KERN_ERR "failed to get aclk_mcuisp_muxed\n");
	aclk_mcuisp_div0 = clk_get(&pdev->dev, "sclk_mcuisp_div0");
	if (IS_ERR(aclk_mcuisp_div0))
		printk(KERN_ERR "failed to get aclk_mcuisp_div0\n");
	aclk_mcuisp_div1 = clk_get(&pdev->dev, "sclk_mcuisp_div1");
	if (IS_ERR(aclk_mcuisp_div1))
		printk(KERN_ERR "failed to get aclk_mcuisp_div1\n");
	clk_set_rate(aclk_mcuisp_div0, 100 * 1000000);
	clk_set_rate(aclk_mcuisp_div1, 100 * 1000000);
	clk_put(aclk_mcuisp_muxed);
	clk_put(aclk_mcuisp_div0);
	clk_put(aclk_mcuisp_div1);
	/* 2. ACLK_ISP */
	aclk_200 = clk_get(&pdev->dev, "aclk_200_muxed");
	if (IS_ERR(aclk_200))
		printk(KERN_ERR "failed to get aclk_200\n");
	aclk_200_div0 = clk_get(&pdev->dev, "sclk_aclk_div0");
	if (IS_ERR(aclk_200_div0))
		printk(KERN_ERR "failed to get aclk_200_div0\n");
	aclk_200_div1 = clk_get(&pdev->dev, "sclk_aclk_div1");
	if (IS_ERR(aclk_200_div1))
		printk(KERN_ERR "failed to get aclk_200_div1\n");
	clk_set_rate(aclk_200_div0, 80 * 1000000);
	clk_set_rate(aclk_200_div1, 80 * 1000000);
	clk_put(aclk_200);
	clk_put(aclk_200_div0);
	clk_put(aclk_200_div1);
	/* 3. UART-ISP */
	sclk_uart_isp = clk_get(&pdev->dev, "sclk_uart_isp");
	if (IS_ERR(sclk_uart_isp))
		printk(KERN_ERR "failed to get sclk_uart_isp\n");
	sclk_uart_isp_src = clk_get(&pdev->dev, "mout_mpll_user");
	if (IS_ERR(sclk_uart_isp_src))
		printk(KERN_ERR "failed to get sclk_uart_isp_src\n");
	clk_set_parent(sclk_uart_isp, sclk_uart_isp_src);
	clk_set_rate(sclk_uart_isp, 50 * 1000000);
	clk_put(sclk_uart_isp);
	clk_put(sclk_uart_isp_src);
	/* 4. PWM-ISP */
	sclk_pwm_isp = clk_get(&pdev->dev, "sclk_pwm_isp");
	if (IS_ERR(sclk_pwm_isp))
		printk(KERN_ERR "failed to get sclk_pwm_isp\n");
	sclk_pwm_isp_src = clk_get(&pdev->dev, "mout_mpll_user");
	if (IS_ERR(sclk_pwm_isp_src))
		printk(KERN_ERR "failed to get sclk_pwm_isp_src\n");
	clk_set_parent(sclk_pwm_isp, sclk_pwm_isp_src);
	clk_set_rate(sclk_pwm_isp, 50 * 1000000);
	clk_put(sclk_pwm_isp);
	clk_put(sclk_pwm_isp_src);
	return 0;
}

int exynos_fimc_is_clk_on(struct platform_device *pdev)
{
	struct clk *aclk_mcuisp_muxed = NULL;
	struct clk *aclk_mcuisp_div0 = NULL;
	struct clk *aclk_mcuisp_div1 = NULL;
	struct clk *aclk_200 = NULL;
	struct clk *aclk_200_div0 = NULL;
	struct clk *aclk_200_div1 = NULL;
	struct clk *sclk_uart_isp = NULL;
	struct clk *sclk_pwm_isp = NULL;

	/* 1. MCUISP */
	aclk_mcuisp_muxed = clk_get(&pdev->dev, "aclk_400_muxed");
	if (IS_ERR(aclk_mcuisp_muxed))
		printk(KERN_ERR "failed to get aclk_mcuisp_muxed\n");
	aclk_mcuisp_div0 = clk_get(&pdev->dev, "sclk_mcuisp_div0");
	if (IS_ERR(aclk_mcuisp_div0))
		printk(KERN_ERR "failed to get aclk_mcuisp_div0\n");
	aclk_mcuisp_div1 = clk_get(&pdev->dev, "sclk_mcuisp_div1");
	if (IS_ERR(aclk_mcuisp_div1))
		printk(KERN_ERR "failed to get aclk_mcuisp_div1\n");
	clk_enable(aclk_mcuisp_muxed);
	clk_enable(aclk_mcuisp_div0);
	clk_enable(aclk_mcuisp_div1);
	clk_put(aclk_mcuisp_muxed);
	clk_put(aclk_mcuisp_div0);
	clk_put(aclk_mcuisp_div1);
	/* 2. ACLK_ISP */
	aclk_200 = clk_get(&pdev->dev, "aclk_200_muxed");
	if (IS_ERR(aclk_200))
		printk(KERN_ERR "failed to get aclk_200\n");
	aclk_200_div0 = clk_get(&pdev->dev, "sclk_aclk_div0");
	if (IS_ERR(aclk_200_div0))
		printk(KERN_ERR "failed to get aclk_200_div0\n");
	aclk_200_div1 = clk_get(&pdev->dev, "sclk_aclk_div1");
	if (IS_ERR(aclk_200_div1))
		printk(KERN_ERR "failed to get aclk_200_div1\n");
	clk_enable(aclk_200);
	clk_enable(aclk_200_div0);
	clk_enable(aclk_200_div1);
	clk_put(aclk_200);
	clk_put(aclk_200_div0);
	clk_put(aclk_200_div1);
	/* 3. UART-ISP */
	sclk_uart_isp = clk_get(&pdev->dev, "sclk_uart_isp");
	if (IS_ERR(sclk_uart_isp))
		printk(KERN_ERR "failed to get sclk_uart_isp\n");
	clk_enable(sclk_uart_isp);
	clk_put(sclk_uart_isp);
	/* 4. PWM-ISP */
	sclk_pwm_isp = clk_get(&pdev->dev, "sclk_pwm_isp");
	if (IS_ERR(sclk_pwm_isp))
		printk(KERN_ERR "failed to get sclk_pwm_isp\n");
	clk_enable(sclk_pwm_isp);
	clk_put(sclk_pwm_isp);
	return 0;
}

int exynos_fimc_is_clk_off(struct platform_device *pdev)
{
	struct clk *aclk_mcuisp_muxed = NULL;
	struct clk *aclk_mcuisp_div0 = NULL;
	struct clk *aclk_mcuisp_div1 = NULL;
	struct clk *aclk_200 = NULL;
	struct clk *aclk_200_div0 = NULL;
	struct clk *aclk_200_div1 = NULL;
	struct clk *sclk_uart_isp = NULL;
	struct clk *sclk_pwm_isp = NULL;

	/* 1. MCUISP */
	aclk_mcuisp_muxed = clk_get(&pdev->dev, "aclk_400_muxed");
	if (IS_ERR(aclk_mcuisp_muxed))
		printk(KERN_ERR "failed to get aclk_mcuisp_muxed\n");
	aclk_mcuisp_div0 = clk_get(&pdev->dev, "sclk_mcuisp_div0");
	if (IS_ERR(aclk_mcuisp_div0))
		printk(KERN_ERR "failed to get aclk_mcuisp_div0\n");
	aclk_mcuisp_div1 = clk_get(&pdev->dev, "sclk_mcuisp_div1");
	if (IS_ERR(aclk_mcuisp_div1))
		printk(KERN_ERR "failed to get aclk_mcuisp_div1\n");
	clk_disable(aclk_mcuisp_muxed);
	clk_disable(aclk_mcuisp_div0);
	clk_disable(aclk_mcuisp_div1);
	clk_put(aclk_mcuisp_muxed);
	clk_put(aclk_mcuisp_div0);
	clk_put(aclk_mcuisp_div1);
	/* 2. ACLK_ISP */
	aclk_200 = clk_get(&pdev->dev, "aclk_200_muxed");
	if (IS_ERR(aclk_200))
		printk(KERN_ERR "failed to get aclk_200\n");
	aclk_200_div0 = clk_get(&pdev->dev, "sclk_aclk_div0");
	if (IS_ERR(aclk_200_div0))
		printk(KERN_ERR "failed to get aclk_200_div0\n");
	aclk_200_div1 = clk_get(&pdev->dev, "sclk_aclk_div1");
	if (IS_ERR(aclk_200_div1))
		printk(KERN_ERR "failed to get aclk_200_div1\n");
	clk_disable(aclk_200);
	clk_disable(aclk_200_div0);
	clk_disable(aclk_200_div1);
	clk_put(aclk_200);
	clk_put(aclk_200_div0);
	clk_put(aclk_200_div1);
	/* 3. UART-ISP */
	sclk_uart_isp = clk_get(&pdev->dev, "sclk_uart_isp");
	if (IS_ERR(sclk_uart_isp))
		printk(KERN_ERR "failed to get sclk_uart_isp\n");
	clk_disable(sclk_uart_isp);
	clk_put(sclk_uart_isp);
	/* 4. PWM-ISP */
	sclk_pwm_isp = clk_get(&pdev->dev, "sclk_pwm_isp");
	if (IS_ERR(sclk_pwm_isp))
		printk(KERN_ERR "failed to get sclk_pwm_isp\n");
	clk_disable(sclk_pwm_isp);
	clk_put(sclk_pwm_isp);
	return 0;
}

#if defined(CONFIG_ARCH_EXYNOS5)
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>

static void cam_power_on(void)
{
	struct regulator *regulator;

	/* ISP */
	regulator = regulator_get(NULL, "cam_5m_core_1.8v");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "%s : regulator_get failed\n", __func__);
		return;
	}
	regulator_enable(regulator);
	regulator_put(regulator);

	/* ldo18 */
	regulator = regulator_get(NULL, "cam_io_1.8v");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "%s : regulator_get failed\n", __func__);
		return;
	}
	regulator_enable(regulator);
	regulator_put(regulator);

	/* ldo24 */
	regulator = regulator_get(NULL, "cam_af_2.8v");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "%s : regulator_get failed\n", __func__);
		return;
	}
	regulator_enable(regulator);
	regulator_put(regulator);

	/* ldo17 */
	regulator = regulator_get(NULL, "from_1.8v");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "%s : regulator_get failed\n", __func__);
		return;
	}
	regulator_enable(regulator);
	regulator_put(regulator);
}

void exynos5_fimc_is_cfg_gpio(struct platform_device *pdev)
{
	int ret;

	/* 1. UART setting for FIMC-IS */
	ret = gpio_request(GPIO_5M_nRST, "GPIO_5M_nRST");
	if (ret)
		printk(KERN_ERR "#### failed to request GPIO_5M_nRST ####\n");
	s3c_gpio_cfgpin(GPIO_5M_nRST, (0x2<<0));
	s3c_gpio_setpull(GPIO_5M_nRST, S3C_GPIO_PULL_NONE);
	gpio_free(GPIO_5M_nRST);

#if defined(CONFIG_MACH_P11) || defined(CONFIG_MACH_P10)
#else
	ret = gpio_request(EXYNOS5_GPE0(1), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_1 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(1), (0x2<<4));
	s3c_gpio_setpull(EXYNOS5_GPE0(1), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(1));

	ret = gpio_request(EXYNOS5_GPE0(2), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_2 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(2), (0x3<<8));
	s3c_gpio_setpull(EXYNOS5_GPE0(2), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(2));

	ret = gpio_request(EXYNOS5_GPE0(3), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_3 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(3), (0x3<<12));
	s3c_gpio_setpull(EXYNOS5_GPE0(3), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(3));

	ret = gpio_request(EXYNOS5_GPE0(4), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_4 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(4), (0x3<<16));
	s3c_gpio_setpull(EXYNOS5_GPE0(4), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(4));

	ret = gpio_request(EXYNOS5_GPE0(5), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_5 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(5), (0x3<<20));
	s3c_gpio_setpull(EXYNOS5_GPE0(5), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(5));

	ret = gpio_request(EXYNOS5_GPE0(6), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_6 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(6), (0x3<<24));
	s3c_gpio_setpull(EXYNOS5_GPE0(6), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(6));
#endif

	ret = gpio_request(GPIO_ISP_TXD, "GPIO_ISP_TXD");
	if (ret)
		printk(KERN_ERR "#### failed to request GPIO_ISP_TXD ####\n");
	s3c_gpio_cfgpin(GPIO_ISP_TXD, (0x3<<28));
	s3c_gpio_setpull(GPIO_ISP_TXD, S3C_GPIO_PULL_NONE);
	gpio_free(GPIO_ISP_TXD);

#if defined(CONFIG_MACH_P11) || defined(CONFIG_MACH_P10)
#else
	ret = gpio_request(EXYNOS5_GPE1(0), "GPE1");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE1_0 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE1(0), (0x3<<0));
	s3c_gpio_setpull(EXYNOS5_GPE1(0), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE1(0));
#endif

	ret = gpio_request(GPIO_ISP_RXD, "GPIO_ISP_RXD");
	if (ret)
		printk(KERN_ERR "#### failed to request GPIO_ISP_RXD ####\n");
	s3c_gpio_cfgpin(GPIO_ISP_RXD, (0x3<<4));
	s3c_gpio_setpull(GPIO_ISP_RXD, S3C_GPIO_PULL_NONE);
	gpio_free(GPIO_ISP_RXD);

	/* 2. GPIO setting for FIMC-IS */
	ret = gpio_request(GPIO_5M_CAM_SDA_18V, "GPIO_5M_CAM_SDA_18V");
	if (ret)
		printk(KERN_ERR "#### failed to request GPIO_5M_CAM_SDA_18V ####\n");
	s3c_gpio_cfgpin(GPIO_5M_CAM_SDA_18V, (0x2<<0));
	s3c_gpio_setpull(GPIO_5M_CAM_SDA_18V, S3C_GPIO_PULL_NONE);
	gpio_free(GPIO_5M_CAM_SDA_18V);

	ret = gpio_request(GPIO_5M_CAM_SCL_18V, "GPIO_5M_CAM_SCL_18V");
	if (ret)
		printk(KERN_ERR "#### failed to request GPIO_5M_CAM_SCL_18V ####\n");
	s3c_gpio_cfgpin(GPIO_5M_CAM_SCL_18V, (0x2<<4));
	s3c_gpio_setpull(GPIO_5M_CAM_SCL_18V, S3C_GPIO_PULL_NONE);
	gpio_free(GPIO_5M_CAM_SCL_18V);

	ret = gpio_request(GPIO_VT_CAM_SDA_18V, "GPIO_VT_CAM_SDA_18V");
	if (ret)
		printk(KERN_ERR "#### failed to request GPIO_VT_CAM_SDA_18V ####\n");
	s3c_gpio_cfgpin(GPIO_VT_CAM_SDA_18V, (0x2<<8));
	s3c_gpio_setpull(GPIO_VT_CAM_SDA_18V, S3C_GPIO_PULL_NONE);
	gpio_free(GPIO_VT_CAM_SDA_18V);

	ret = gpio_request(GPIO_VT_CAM_SCL_18V, "GPIO_VT_CAM_SCL_18V");
	if (ret)
		printk(KERN_ERR "#### failed to request GPIO_5M_CAM_SDA_18V ####\n");
	s3c_gpio_cfgpin(GPIO_VT_CAM_SCL_18V, (0x2<<12));
	s3c_gpio_setpull(GPIO_VT_CAM_SCL_18V, S3C_GPIO_PULL_NONE);
	gpio_free(GPIO_VT_CAM_SCL_18V);

	ret = gpio_request(GPIO_CMC_CLK_18V, "GPIO_CMC_CLK_18V");
	if (ret)
		printk(KERN_ERR "#### failed to request GPIO_CMC_CLK_18V ####\n");
	s3c_gpio_cfgpin(GPIO_CMC_CLK_18V, (0x3<<0));
	s3c_gpio_setpull(GPIO_CMC_CLK_18V, S3C_GPIO_PULL_NONE);
	gpio_free(GPIO_CMC_CLK_18V);

	ret = gpio_request(GPIO_CMC_CS_18V, "GPIO_CMC_CS_18V");
	if (ret)
		printk(KERN_ERR "#### failed to request GPIO_CMC_CS_18V ####\n");
	s3c_gpio_cfgpin(GPIO_CMC_CS_18V, (0x3<<4));
	s3c_gpio_setpull(GPIO_CMC_CS_18V, S3C_GPIO_PULL_NONE);
	gpio_free(GPIO_CMC_CS_18V);

	/* CAM A port(b0010) : CLK_OUT */
	s3c_gpio_cfgrange_nopull(GPIO_CAM_MCLK, 1, S3C_GPIO_SFN(2));

#if defined(CONFIG_MACH_P11) || defined(CONFIG_MACH_P10)
	/* power_on */
	cam_power_on();

	/* CAM A port : POWER */
	s3c_gpio_cfgpin(GPIO_CAM_IO_EN, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_CAM_IO_EN, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_CAM_IO_EN, 1);

	/* CAM A reset*/
	ret = gpio_request(GPIO_5M_nRST, "GPIO_5M_nRST");
	if (ret)
		printk(KERN_ERR "#### failed to request GPIO_5M_nRST ####\n");

	s3c_gpio_setpull(GPIO_5M_nRST, S3C_GPIO_PULL_NONE);
	gpio_direction_output(GPIO_5M_nRST, 0);
	gpio_direction_output(GPIO_5M_nRST, 1);
	gpio_free(GPIO_5M_nRST);
#else
	/* CAM A port(b0010) : DATA[0-7] */
	/* s3c_gpio_cfgrange_nopull(EXYNOS5_GPH1(0), 8, S3C_GPIO_SFN(2)); */

	/* Camera A */
	ret = gpio_request(EXYNOS5_GPX1(2), "GPX1");
	if (ret)
		printk(KERN_ERR "#### failed to request GPX1_2 ####\n");

	s3c_gpio_setpull(EXYNOS5_GPX1(2), S3C_GPIO_PULL_NONE);
	gpio_direction_output(EXYNOS5_GPX1(2), 0);
	gpio_direction_output(EXYNOS5_GPX1(2), 1);
	gpio_free(EXYNOS5_GPX1(2));
#endif
}

int exynos5_fimc_is_cfg_clk(struct platform_device *pdev)
{
	struct clk *aclk_mcuisp = NULL;
	struct clk *aclk_266 = NULL;
	struct clk *aclk_mcuisp_div0 = NULL;
	struct clk *aclk_mcuisp_div1 = NULL;
	struct clk *aclk_266_div0 = NULL;
	struct clk *aclk_266_div1 = NULL;
	struct clk *aclk_266_mpwm = NULL;
	struct clk *sclk_uart_isp = NULL;
	struct clk *sclk_uart_isp_div = NULL;
	struct clk *mout_mpll = NULL;
	struct clk *sclk_mipi = NULL;
	struct clk *cam_src = NULL;
	struct clk *cam_A_clk = NULL;
	unsigned long mcu_isp_400;
	unsigned long isp_266;
	unsigned long isp_uart;
	unsigned long mipi;
	unsigned long epll;

	/*
	 * initialize Clocks
	*/

	printk(KERN_DEBUG "exynos5_fimc_is_cfg_clk\n");
	/* 1. MCUISP */
	aclk_mcuisp = clk_get(&pdev->dev, "aclk_400_isp");
	if (IS_ERR(aclk_mcuisp))
		return PTR_ERR(aclk_mcuisp);

	aclk_mcuisp_div0 = clk_get(&pdev->dev, "aclk_isp_400_div0");
	if (IS_ERR(aclk_mcuisp_div0))
		return PTR_ERR(aclk_mcuisp_div0);

	aclk_mcuisp_div1 = clk_get(&pdev->dev, "aclk_isp_400_div1");
	if (IS_ERR(aclk_mcuisp_div1))
		return PTR_ERR(aclk_mcuisp_div1);

	clk_set_rate(aclk_mcuisp_div0, 400 * 1000000);
	clk_set_rate(aclk_mcuisp_div1, 400 * 1000000);

	mcu_isp_400 = clk_get_rate(aclk_mcuisp);
	printk(KERN_DEBUG "mcu_isp_400 : %ld\n", mcu_isp_400);

	mcu_isp_400 = clk_get_rate(aclk_mcuisp_div0);
	printk(KERN_DEBUG "mcu_isp_400_div0 : %ld\n", mcu_isp_400);

	mcu_isp_400 = clk_get_rate(aclk_mcuisp_div1);
	printk(KERN_DEBUG "aclk_mcuisp_div1 : %ld\n", mcu_isp_400);

	clk_put(aclk_mcuisp);
	clk_put(aclk_mcuisp_div0);
	clk_put(aclk_mcuisp_div1);

	/* 2. ACLK_ISP */
	aclk_266 = clk_get(&pdev->dev, "aclk_266_isp");
	if (IS_ERR(aclk_266))
		return PTR_ERR(aclk_266);
	aclk_266_div0 = clk_get(&pdev->dev, "aclk_isp_266_div0");
	if (IS_ERR(aclk_266_div0))
		return PTR_ERR(aclk_266_div0);
	aclk_266_div1 = clk_get(&pdev->dev, "aclk_isp_266_div1");
	if (IS_ERR(aclk_266_div1))
		return PTR_ERR(aclk_266_div1);
	aclk_266_mpwm = clk_get(&pdev->dev, "aclk_isp_266_divmpwm");
	if (IS_ERR(aclk_266_mpwm))
		return PTR_ERR(aclk_266_mpwm);

	clk_set_rate(aclk_266_div0, 134 * 1000000);
	clk_set_rate(aclk_266_div1, 68 * 1000000);

	isp_266 = clk_get_rate(aclk_266);
	printk(KERN_DEBUG "isp_266 : %ld\n", isp_266);

	isp_266 = clk_get_rate(aclk_266_div0);
	printk(KERN_DEBUG "isp_266_div0 : %ld\n", isp_266);

	isp_266 = clk_get_rate(aclk_266_div1);
	printk(KERN_DEBUG "isp_266_div1 : %ld\n", isp_266);

	isp_266 = clk_get_rate(aclk_266_mpwm);
	printk(KERN_DEBUG "isp_266_mpwm : %ld\n", isp_266);

	clk_put(aclk_266);
	clk_put(aclk_266_div0);
	clk_put(aclk_266_div1);
	clk_put(aclk_266_mpwm);

	/* 3. UART-ISP */
	sclk_uart_isp = clk_get(&pdev->dev, "sclk_uart_src_isp");
	if (IS_ERR(sclk_uart_isp))
		return PTR_ERR(sclk_uart_isp);

	sclk_uart_isp_div = clk_get(&pdev->dev, "sclk_uart_isp");
	if (IS_ERR(sclk_uart_isp_div))
		return PTR_ERR(sclk_uart_isp_div);

	clk_set_parent(sclk_uart_isp_div, sclk_uart_isp);
	clk_set_rate(sclk_uart_isp_div, 50 * 1000000);

	isp_uart = clk_get_rate(sclk_uart_isp);
	printk(KERN_DEBUG "isp_uart : %ld\n", isp_uart);
	isp_uart = clk_get_rate(sclk_uart_isp_div);
	printk(KERN_DEBUG "isp_uart_div : %ld\n", isp_uart);

	clk_put(sclk_uart_isp);
	clk_put(sclk_uart_isp_div);

	/* MIPI-CSI */
	mout_mpll = clk_get(&pdev->dev, "mout_mpll_user");
	if (IS_ERR(mout_mpll))
		return PTR_ERR(mout_mpll);
	sclk_mipi = clk_get(&pdev->dev, "sclk_gscl_wrap");
	if (IS_ERR(sclk_mipi))
		return PTR_ERR(sclk_mipi);

	clk_set_parent(sclk_mipi, mout_mpll);
	clk_set_rate(sclk_mipi, 266 * 1000000);

	mipi = clk_get_rate(mout_mpll);
	printk(KERN_DEBUG "mipi_src : %ld\n", mipi);
	mipi = clk_get_rate(sclk_mipi);
	printk(KERN_DEBUG "mipi_div : %ld\n", mipi);

	clk_put(mout_mpll);
	clk_put(sclk_mipi);

	/* camera A */
	cam_src = clk_get(&pdev->dev, "xxti");
	if (IS_ERR(cam_src))
		return PTR_ERR(cam_src);
	cam_A_clk = clk_get(&pdev->dev, "sclk_cam0");
	if (IS_ERR(cam_A_clk))
		return PTR_ERR(cam_A_clk);

	epll = clk_get_rate(cam_src);
	printk(KERN_DEBUG "epll : %ld\n", epll);

	clk_set_parent(cam_A_clk, cam_src);
	clk_set_rate(cam_A_clk, 24 * 1000000);

	clk_put(cam_src);
	clk_put(cam_A_clk);

	return 0;
}

int exynos5_fimc_is_clk_on(struct platform_device *pdev)
{
	struct clk *gsc_ctrl = NULL;
	struct clk *isp_ctrl = NULL;
	struct clk *mipi_ctrl = NULL;
	struct clk *cam_A_clk = NULL;

	printk(KERN_DEBUG "exynos5_fimc_is_clk_on\n");

	gsc_ctrl = clk_get(&pdev->dev, "gscl");
	if (IS_ERR(gsc_ctrl))
		return PTR_ERR(gsc_ctrl);

	clk_enable(gsc_ctrl);
	clk_put(gsc_ctrl);

	isp_ctrl = clk_get(&pdev->dev, "isp");
	if (IS_ERR(isp_ctrl))
		return PTR_ERR(isp_ctrl);

	clk_enable(isp_ctrl);
	clk_put(isp_ctrl);

	mipi_ctrl = clk_get(&pdev->dev, "gscl_wrap");
	if (IS_ERR(mipi_ctrl))
		return PTR_ERR(mipi_ctrl);

	clk_enable(mipi_ctrl);
	clk_put(mipi_ctrl);

	cam_A_clk = clk_get(&pdev->dev, "sclk_cam0");
	if (IS_ERR(cam_A_clk))
		return PTR_ERR(cam_A_clk);

	clk_enable(cam_A_clk);
	clk_put(cam_A_clk);

	return 0;
}

int exynos5_fimc_is_clk_off(struct platform_device *pdev)
{
	struct clk *gsc_ctrl = NULL;
	struct clk *isp_ctrl = NULL;
	struct clk *mipi_ctrl = NULL;
	struct clk *cam_A_clk = NULL;

	printk(KERN_DEBUG "exynos5_fimc_is_clk_on\n");

	gsc_ctrl = clk_get(&pdev->dev, "gscl");
	if (IS_ERR(gsc_ctrl))
		return PTR_ERR(gsc_ctrl);

	clk_disable(gsc_ctrl);
	clk_put(gsc_ctrl);

	isp_ctrl = clk_get(&pdev->dev, "isp");
	if (IS_ERR(isp_ctrl))
		return PTR_ERR(isp_ctrl);

	clk_disable(isp_ctrl);
	clk_put(isp_ctrl);

	mipi_ctrl = clk_get(&pdev->dev, "gscl_wrap");
	if (IS_ERR(mipi_ctrl))
		return PTR_ERR(mipi_ctrl);

	clk_disable(mipi_ctrl);
	clk_put(mipi_ctrl);

	cam_A_clk = clk_get(&pdev->dev, "sclk_cam0");
	if (IS_ERR(cam_A_clk))
		return PTR_ERR(cam_A_clk);

	clk_disable(cam_A_clk);
	clk_put(cam_A_clk);

	return 0;
}
#endif
