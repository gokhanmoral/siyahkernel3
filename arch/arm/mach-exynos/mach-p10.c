/* linux/arch/arm/mach-exynos/mach-p10.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/gpio_event.h>
#include <linux/gpio_keys.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/pwm_backlight.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/mmc/host.h>
#include <linux/memblock.h>
#include <linux/fb.h>
#include <linux/delay.h>

#include <video/platform_lcd.h>

#include <asm/mach/arch.h>
#include <asm/mach-types.h>

#include <media/exynos_gscaler.h>
#include <media/exynos_flite.h>
#include <media/exynos_fimc_is.h>
#include <plat/gpio-cfg.h>
#include <plat/regs-serial.h>
#include <plat/exynos5.h>
#include <plat/cpu.h>
#include <plat/clock.h>
#include <plat/devs.h>
#include <plat/sdhci.h>

#include <plat/fb.h>
#include <plat/fb-s5p.h>
#include <plat/fb-core.h>
#include <plat/regs-fb-v4.h>
#include <plat/iic.h>
#include <plat/pd.h>
#include <plat/ehci.h>
#include <plat/udc-ss.h>
#include <plat/s5p-mfc.h>
#include <plat/dp.h>
#include <plat/backlight.h>
#include <plat/fimg2d.h>
#include <plat/tv-core.h>
#include <plat/s3c64xx-spi.h>

#include <plat/mipi_csis.h>
#include <mach/map.h>
#include <mach/exynos-ion.h>
#include <mach/dev-sysmmu.h>
#include <mach/spi-clocks.h>
#include <mach/ppmu.h>
#include <mach/dev.h>
#ifdef CONFIG_EXYNOS4_DEV_DWMCI
#include <mach/dwmci.h>
#endif
#ifdef CONFIG_VIDEO_JPEG_V2X
#include <plat/jpeg.h>
#endif

#ifdef CONFIG_EXYNOS_C2C
#include <mach/c2c.h>
#endif

#ifdef CONFIG_VIDEO_EXYNOS_TV
#include <plat/tvout.h>
#endif

#ifdef CONFIG_MFD_MAX77686
#include <linux/mfd/max77686.h>
#endif

#include <mach/gpio-p10.h>
#include <mach/regs-clock.h>
#include <mach/regs-pmu5.h>
#if defined(CONFIG_TOUCHSCREEN_ATMEL_MXT1536E)
#include <linux/i2c/mxt1536e.h>
#elif defined(CONFIG_TOUCHSCREEN_ATMEL_MXT1664S)
#include <linux/i2c/mxt1664s.h>
#endif

#ifdef CONFIG_SND_SOC_WM8994
#include <linux/mfd/wm8994/pdata.h>
#include <linux/mfd/wm8994/gpio.h>
#endif

#if defined(CONFIG_EXYNOS_SETUP_THERMAL)
#include <plat/s5p-tmu.h>
#endif
#include <plat/media.h>

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define SMDK5250_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define SMDK5250_ULCON_DEFAULT	S3C2410_LCON_CS8

#define SMDK5250_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

#if defined(CONFIG_MHL_SII9234)
#include <linux/sii9234.h>
#endif
#ifdef CONFIG_30PIN_CONN
#include <linux/30pin_con.h>
#endif

static struct s3c2410_uartcfg smdk5250_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= SMDK5250_UCON_DEFAULT,
		.ulcon		= SMDK5250_ULCON_DEFAULT,
		.ufcon		= SMDK5250_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= SMDK5250_UCON_DEFAULT,
		.ulcon		= SMDK5250_ULCON_DEFAULT,
		.ufcon		= SMDK5250_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= SMDK5250_UCON_DEFAULT,
		.ulcon		= SMDK5250_ULCON_DEFAULT,
		.ufcon		= SMDK5250_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= SMDK5250_UCON_DEFAULT,
		.ulcon		= SMDK5250_ULCON_DEFAULT,
		.ufcon		= SMDK5250_UFCON_DEFAULT,
	},
};

#ifdef CONFIG_EXYNOS_MEDIA_DEVICE
struct platform_device exynos_device_md0 = {
	.name = "exynos-mdev",
	.id = 0,
};

struct platform_device exynos_device_md1 = {
	.name = "exynos-mdev",
	.id = 1,
};
struct platform_device exynos_device_md2 = {
	.name = "exynos-mdev",
	.id = 2,
};
#endif

#ifdef CONFIG_FB_S3C
#if defined(CONFIG_MACH_P10_DP_01)

static struct s3c_fb_pd_win p10_fb_win0 = {
	.win_mode = {
		.refresh	= 60,
		.left_margin	= 80,
		.right_margin	= 48,
		.upper_margin	= 37,
		.lower_margin	= 3,
		.hsync_len	= 32,
		.vsync_len	= 6,
		.xres		= 2560,
		.yres		= 1600,
	},
	.virtual_x		= 2560,
	.virtual_y		= 1600 * 2,
	.max_bpp		= 32,
	.default_bpp		= 24,
};

static struct s3c_fb_pd_win p10_fb_win1 = {
	.win_mode = {
		.refresh	= 60,
		.left_margin	= 80,
		.right_margin	= 48,
		.upper_margin	= 37,
		.lower_margin	= 3,
		.hsync_len	= 32,
		.vsync_len	= 6,
		.xres		= 2560,
		.yres		= 1600,
	},

	.virtual_x		= 2560,
	.virtual_y		= 1600 * 2,
	.max_bpp		= 32,
	.default_bpp		= 24,

};

static struct s3c_fb_pd_win p10_fb_win2 = {
	.win_mode = {
		.refresh	= 60,
		.left_margin	= 80,
		.right_margin	= 48,
		.upper_margin	= 37,
		.lower_margin	= 3,
		.hsync_len	= 32,
		.vsync_len	= 6,
		.xres		= 2560,
		.yres		= 1600,
	},

	.virtual_x		= 2560,
	.virtual_y		= 1600 * 2,
	.max_bpp		= 32,
	.default_bpp		= 24,

};

#elif defined(CONFIG_MACH_P10_DP_00)

static struct s3c_fb_pd_win p10_fb_win0 = {
	.win_mode = {
		.refresh	= 20,
		.left_margin	= 40,
		.right_margin	= 24,
		.upper_margin	= 20,
		.lower_margin	= 3,
		.hsync_len	= 16,
		.vsync_len	= 6,
		.xres		= 2560,
		.yres		= 1600,
	},
	.virtual_x		= 2560,
	.virtual_y		= 1600 * 2,
	.max_bpp		= 32,
	.default_bpp		= 24,
};

static struct s3c_fb_pd_win p10_fb_win1 = {
	.win_mode = {
		.refresh	= 20,
		.left_margin	= 40,
		.right_margin	= 24,
		.upper_margin	= 20,
		.lower_margin	= 3,
		.hsync_len	= 16,
		.vsync_len	= 6,
		.xres		= 2560,
		.yres		= 1600,

	},
	.virtual_x		= 2560,
	.virtual_y		= 1600 * 2,
	.max_bpp		= 32,
	.default_bpp		= 24,

};

static struct s3c_fb_pd_win p10_fb_win2 = {
	.win_mode = {
		.refresh	= 20,
		.left_margin	= 40,
		.right_margin	= 24,
		.upper_margin	= 20,
		.lower_margin	= 3,
		.hsync_len	= 16,
		.vsync_len	= 6,
		.xres		= 2560,
		.yres		= 1600,

	},
	.virtual_x		= 2560,
	.virtual_y		= 1600 * 2,
	.max_bpp		= 32,
	.default_bpp		= 24,

};
#endif
static void exynos_fimd_gpio_setup_24bpp(void)
{
	unsigned int reg = 0;
	unsigned int uRead = 0;

#if defined(CONFIG_S5P_DP)
		/* Set Hotplug detect for DP */
		s3c_gpio_cfgpin(GPIO_DP_HPD, S3C_GPIO_SFN(3));
#endif
	/*
	 * Set DISP1BLK_CFG register for Display path selection
	 *
	 * FIMD of DISP1_BLK Bypass selection : DISP1BLK_CFG[15]
	 * ---------------------
	 *  0 | MIE/MDNIE
	 *  1 | FIMD : selected
	 */
	reg = __raw_readl(S3C_VA_SYS + 0x0214);
	reg &= ~(1 << 15);	/* To save other reset values */
	reg |= (1 << 15);
	__raw_writel(reg, S3C_VA_SYS + 0x0214);
#if defined(CONFIG_S5P_DP)
#if defined(CONFIG_MACH_P10_DP_01)
	// MPLL => FIMD Bus clock
	uRead = __raw_readl(EXYNOS5_CLKSRC_TOP0);
	uRead = (uRead & ~(0x3<<14)) | (0x0<<14);
	__raw_writel(uRead, EXYNOS5_CLKSRC_TOP0);

	uRead = __raw_readl(EXYNOS5_CLKDIV_TOP0);
	uRead = (uRead & ~(0x7<<28)) | (0x2<<28);
	__raw_writel(uRead,EXYNOS5_CLKDIV_TOP0);

	/* Reference clcok selection for DPTX_PHY: pad_osc_clk_24M */
	reg = __raw_readl(S3C_VA_SYS + 0x04d4);
	reg = (reg & ~(0x1 << 0)) | (0x0 << 0);
	__raw_writel(reg, S3C_VA_SYS + 0x04d4);

	/* DPTX_PHY: XXTI */
	reg = __raw_readl(S3C_VA_SYS + 0x04d8);
	reg = (reg & ~(0x1 << 3)) | (0x0 << 3);
	__raw_writel(reg, S3C_VA_SYS + 0x04d8);
#elif defined(CONFIG_MACH_P10_DP_00)

	reg = __raw_readl(S3C_VA_SYS + 0x04d4);
	reg |= (1 << 0);
	__raw_writel(reg, S3C_VA_SYS + 0x04d4);

	/* DPTX_PHY: XXTI */
	reg = __raw_readl(S3C_VA_SYS + 0x04d8);
	reg &= ~(1 << 3);
	__raw_writel(reg, S3C_VA_SYS + 0x04d8);
#endif
#endif
}

static struct s3c_fb_platdata p10_lcd1_pdata __initdata = {
	.win[0]		= &p10_fb_win0,
	.win[1]		= &p10_fb_win1,
	.win[2]		= &p10_fb_win2,
	.default_win	= 2,
	.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
	.vidcon1	= 0,
	.setup_gpio	= exynos_fimd_gpio_setup_24bpp,
};
#endif

#ifdef CONFIG_S5P_DP
static void dp_lcd_set_power(struct plat_lcd_data *pd,
				unsigned int power)
{
	if (power) {

		/* LCD_PWM_IN_2.8V, AH21, XPWMOUT_0 => LCD_B_PWM */
#ifndef CONFIG_BACKLIGHT_PWM
		gpio_request_one(GPIO_LCD_PWM_IN_18V, GPIOF_OUT_INIT_LOW, "GPB2");
#endif

		/* LCD_APS_EN_2.8V, R6, XCI1RGB_2 => GPG0_2 */
		gpio_request_one(GPIO_LCD_APS_EN_18V, GPIOF_OUT_INIT_LOW, "GPG0");

		/* LCD_EN , XMMC2CDN => GPC2_2 */
		gpio_request_one(GPIO_LCD_EN, GPIOF_OUT_INIT_LOW, "GPC2");

		/* LCD_EN , XMMC2CDN => GPC2_2 */
		gpio_set_value(GPIO_LCD_EN, 1);

		/* LCD_APS_EN_2.8V, R6, XCI1RGB_2 => GPG0_2 */
		gpio_set_value(GPIO_LCD_APS_EN_18V, 1);

		udelay(1000);

		/* LCD_PWM_IN_2.8V, AH21, XPWMOUT_0=> LCD_B_PWM */
#ifndef CONFIG_BACKLIGHT_PWM
		gpio_set_value(GPIO_LCD_PWM_IN_18V, 1);
#endif
	} else {
		/* LCD_PWM_IN_2.8V, AH21, XPWMOUT_0=> LCD_B_PWM */
#ifndef CONFIG_BACKLIGHT_PWM
		gpio_set_value(GPIO_LCD_PWM_IN_18V, 0);
#endif

		/* LCD_APS_EN_2.8V, R6, XCI1RGB_2 => GPG0_2 */
		gpio_set_value(GPIO_LCD_APS_EN_18V, 0);

		/* LCD_EN , XMMC2CDN => GPC2_2 */
		gpio_set_value(GPIO_LCD_EN, 0);

		/* LCD_APS_EN_2.8V, R6, XCI1RGB_2 => GPG0_2 */
		gpio_free(GPIO_LCD_APS_EN_18V);

		/* LCD_EN , XMMC2CDN => GPC2_2 */
		gpio_free(GPIO_LCD_EN);

#ifndef CONFIG_BACKLIGHT_PWM
		gpio_free(GPIO_LCD_PWM_IN_18V);
#endif
	}
}

static struct plat_lcd_data p10_dp_lcd_data = {
	.set_power	= dp_lcd_set_power,
};

static struct platform_device p10_dp_lcd = {
	.name              = "platform-lcd",
	.dev.parent        = &s5p_device_fimd1.dev,
	.dev.platform_data = &p10_dp_lcd_data,
};

static struct video_info p10_dp_config = {
	.name			= "for p10 TEST",
#if defined(CONFIG_MACH_P10_DP_01)
	.h_total		= 2720,
	.h_active		= 2560,
	.h_sync_width		= 32,
	.h_back_porch		= 80,
	.h_front_porch		= 48,

	.v_total		= 1646,
	.v_active		= 1600,
	.v_sync_width		= 6,
	.v_back_porch		= 37,
	.v_front_porch		= 3,

	.v_sync_rate		= 60,

	.mvid			= 0,
	.nvid			= 0,

	.h_sync_polarity	= 0,
	.v_sync_polarity	= 0,
	.interlaced		= 0,

	.color_space		= COLOR_RGB,
	.dynamic_range		= VESA,
	.ycbcr_coeff		= COLOR_YCBCR601,
	.color_depth		= COLOR_8,

	.sync_clock		= 0,
	.even_field		= 0,

	.refresh_denominator	= REFRESH_DENOMINATOR_1,

	.test_pattern		= COLORBAR_32,
	.link_rate		= LINK_RATE_2_70GBPS,
	.lane_count		= LANE_COUNT4,

	.video_mute_on		= 0,

	.master_mode		= 0,
	.bist_mode		= 0,

#elif defined(CONFIG_MACH_P10_DP_00)

	.h_total		= 2720,
	.h_active		= 2560,
	.h_sync_width		= 16,
	.h_back_porch		= 40,
	.h_front_porch		= 24,

	.v_total		= 1646,
	.v_active		= 1600,
	.v_sync_width		= 6,
	.v_back_porch		= 20,
	.v_front_porch		= 3,

	.v_sync_rate		= 20,

	.mvid			= 0,
	.nvid			= 0,

	.h_sync_polarity	= 0,
	.v_sync_polarity	= 0,
	.interlaced		= 0,

	.color_space		= COLOR_RGB,
	.dynamic_range		= VESA,
	.ycbcr_coeff		= COLOR_YCBCR601,
	.color_depth		= COLOR_8,

	.sync_clock		= 0,
	.even_field		= 0,

	.refresh_denominator	= REFRESH_DENOMINATOR_1,

	.test_pattern		= COLORBAR_32,
	.link_rate		= LINK_RATE_1_62GBPS,
	.lane_count		= LANE_COUNT4,

	.video_mute_on		= 0,

	.master_mode		= 0,
	.bist_mode		= 0,

#endif
};

static void s5p_dp_backlight_on(void)
{
	/* LED_BACKLIGHT_RESET: XCI1RGB_5 => GPG0_5 */
	gpio_request_one(GPIO_LED_BACKLIGHT_RESET, GPIOF_OUT_INIT_LOW, "GPG0");

	gpio_set_value(GPIO_LED_BACKLIGHT_RESET, 1);
}

static void s5p_dp_backlight_off(void)
{
	/* LED_BACKLIGHT_RESET: XCI1RGB_5 => GPG0_5 */
	gpio_set_value(GPIO_LED_BACKLIGHT_RESET, 0);

	gpio_free(GPIO_LED_BACKLIGHT_RESET);

}

static struct s5p_dp_platdata p10_dp_data __initdata = {
	.video_info = &p10_dp_config,
	.phy_init	= s5p_dp_phy_init,
	.phy_exit	= s5p_dp_phy_exit,
	.backlight_on	= s5p_dp_backlight_on,
	.backlight_off	= s5p_dp_backlight_off,
};
#endif

/* LCD Backlight data */
#ifdef CONFIG_BACKLIGHT_PWM
static struct samsung_bl_gpio_info p10_bl_gpio_info = {
	.no = GPIO_LCD_PWM_IN_18V,
	.func = S3C_GPIO_SFN(2),
};

static struct platform_pwm_backlight_data p10_bl_data = {
	.pwm_id = 0,
	.pwm_period_ns	= 10000,
};
#endif

#ifdef CONFIG_EXYNOS_C2C
struct exynos_c2c_platdata smdk5250_c2c_pdata = {
	.setup_gpio	= NULL,
	.shdmem_addr	= C2C_SHAREDMEM_BASE,
	.shdmem_size	= C2C_MEMSIZE_64,
	.ap_sscm_addr	= NULL,
	.cp_sscm_addr	= NULL,
	.rx_width	= C2C_BUSWIDTH_16,
	.tx_width	= C2C_BUSWIDTH_16,
	.clk_opp100	= 400,
	.clk_opp50	= 200,
	.clk_opp25	= 100,
	.default_opp_mode	= C2C_OPP25,
	.get_c2c_state	= NULL,
	.c2c_sysreg	= S3C_VA_SYS + 0x0360,
};
#endif

#ifdef CONFIG_EXYNOS4_DEV_DWMCI
static void exynos_dwmci_cfg_gpio(int width)
{
	unsigned int gpio;

	for (gpio = EXYNOS5_GPC0(0); gpio < EXYNOS5_GPC0(2); gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV2);
	}

	switch (width) {
	case 8:
		for (gpio = EXYNOS5_GPC1(3); gpio <= EXYNOS5_GPC1(6); gpio++) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(4));
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
			s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV2);
		}
	case 4:
		for (gpio = EXYNOS5_GPC0(3); gpio <= EXYNOS5_GPC0(6); gpio++) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
			s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV2);
		}
		break;
	case 1:
		gpio = EXYNOS5_GPC0(3);
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
		s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV2);
	default:
		break;
	}
}

static struct dw_mci_board exynos_dwmci_pdata __initdata = {
	.num_slots		= 1,
	.quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION | DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz			= 66 * 1000 * 1000,
	.caps			= MMC_CAP_UHS_DDR50 | MMC_CAP_1_8V_DDR | MMC_CAP_8_BIT_DATA | MMC_CAP_CMD23,
	.caps2			= MMC_CAP2_PACKED_CMD | MMC_CAP2_CACHE_CTRL,
	.detect_delay_ms	= 200,
	.hclk_name		= "dwmci",
	.cclk_name		= "sclk_dwmci",
	.cfg_gpio		= exynos_dwmci_cfg_gpio,
};
#endif

#ifdef CONFIG_VIDEO_FIMG2D
static struct fimg2d_platdata fimg2d_data __initdata = {
	.hw_ver		= 0x42,
	.gate_clkname	= "fimg2d",
};
#endif

#ifdef CONFIG_S3C_DEV_HSMMC
static struct s3c_sdhci_platdata smdk5250_hsmmc0_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_PERMANENT,
	.clk_type		= S3C_SDHCI_CLK_DIV_EXTERNAL,
#ifdef CONFIG_EXYNOS4_SDHCI_CH0_8BIT
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#endif
};
#endif

#ifdef CONFIG_S3C_DEV_HSMMC1
static struct s3c_sdhci_platdata smdk5250_hsmmc1_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
	.clk_type		= S3C_SDHCI_CLK_DIV_EXTERNAL,
};
#endif

#ifdef CONFIG_S3C_DEV_HSMMC2
static struct s3c_sdhci_platdata smdk5250_hsmmc2_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_GPIO,
	.clk_type		= S3C_SDHCI_CLK_DIV_EXTERNAL,
	.ext_cd_gpio		= GPIO_T_FLASH_DETECT,
	.ext_cd_gpio_invert	= true,
#ifdef CONFIG_EXYNOS4_SDHCI_CH2_8BIT
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#endif
};
#endif

#ifdef CONFIG_S3C_DEV_HSMMC3
static struct s3c_sdhci_platdata smdk5250_hsmmc3_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
	.clk_type		= S3C_SDHCI_CLK_DIV_EXTERNAL,
};
#endif

#ifdef CONFIG_S3C64XX_DEV_SPI
static struct s3c64xx_spi_csinfo spi1_csi[] = {
	[0] = {
		.line = GPIO_5M_SPI_CS,
		.set_level = gpio_set_value,
		.fb_delay = 0x2,
	},
};

static struct spi_board_info spi1_board_info[] __initdata = {
	{
		.modalias = "spidev",
		.platform_data = NULL,
		.max_speed_hz = 10*1000*1000,
		.bus_num = 1,
		.chip_select = 0,
		.mode = SPI_MODE_0,
		.controller_data = &spi1_csi[0],
	}
};
#endif

#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
#if defined(CONFIG_ITU_A)
static int smdk5250_cam0_reset(int dummy)
{
	int err;
	/* Camera A */
	err = gpio_request(EXYNOS5_GPX1(2), "GPX1");
	if (err)
		printk(KERN_ERR "#### failed to request GPX1_2 ####\n");

	s3c_gpio_setpull(EXYNOS5_GPX1(2), S3C_GPIO_PULL_NONE);
	gpio_direction_output(EXYNOS5_GPX1(2), 0);
	gpio_direction_output(EXYNOS5_GPX1(2), 1);
	gpio_free(EXYNOS5_GPX1(2));

	return 0;
}
#endif
#if defined(CONFIG_ITU_B)
static int smdk5250_cam1_reset(int dummy)
{
	int err;
	/* Camera A */
	err = gpio_request(EXYNOS5_GPX1(0), "GPX1");
	if (err)
		printk(KERN_ERR "#### failed to request GPX1_2 ####\n");

	s3c_gpio_setpull(EXYNOS5_GPX1(0), S3C_GPIO_PULL_NONE);
	gpio_direction_output(EXYNOS5_GPX1(0), 0);
	gpio_direction_output(EXYNOS5_GPX1(0), 1);
	gpio_free(EXYNOS5_GPX1(0));

	return 0;
}
#endif

#ifdef CONFIG_VIDEO_S5K4BA
static struct s5k4ba_mbus_platform_data s5k4ba_mbus_plat = {
	.id		= 0,
	.fmt = {
		.width	= 1600,
		.height	= 1200,
		/* .code	= V4L2_MBUS_FMT_UYVY8_2X8, */
		.code	= V4L2_MBUS_FMT_VYUY8_2X8,
	},
	.clk_rate	= 24000000UL,
#ifdef CONFIG_ITU_A
	.set_power	= smdk5250_cam0_reset,
#endif
#ifdef CONFIG_ITU_B
	.set_power	= smdk5250_cam1_reset,
#endif
};

static struct i2c_board_info s5k4ba_info = {
	I2C_BOARD_INFO("S5K4BA", 0x2d),
	.platform_data = &s5k4ba_mbus_plat,
};
#endif

/* 1 MIPI Cameras */
#ifdef CONFIG_VIDEO_M5MOLS
static struct m5mols_platform_data m5mols_platdata = {
#ifdef CONFIG_CSI_C
	.gpio_rst = EXYNOS5_GPX1(2), /* ISP_RESET */
#endif
#ifdef CONFIG_CSI_D
	.gpio_rst = EXYNOS5_GPX1(0), /* ISP_RESET */
#endif
	.enable_rst = true, /* positive reset */
	.irq = IRQ_EINT(22),
};

static struct i2c_board_info m5mols_board_info = {
	I2C_BOARD_INFO("M5MOLS", 0x1F),
	.platform_data = &m5mols_platdata,
};
#endif
#endif /* CONFIG_VIDEO_EXYNOS_FIMC_LITE */

#ifdef CONFIG_VIDEO_EXYNOS_MIPI_CSIS
static struct regulator_consumer_supply mipi_csi_fixed_voltage_supplies[] = {
	REGULATOR_SUPPLY("mipi_csi", "s5p-mipi-csis.0"),
	REGULATOR_SUPPLY("mipi_csi", "s5p-mipi-csis.1"),
};

static struct regulator_init_data mipi_csi_fixed_voltage_init_data = {
	.constraints = {
		.always_on = 1,
	},
	.num_consumer_supplies	= ARRAY_SIZE(mipi_csi_fixed_voltage_supplies),
	.consumer_supplies	= mipi_csi_fixed_voltage_supplies,
};

static struct fixed_voltage_config mipi_csi_fixed_voltage_config = {
	.supply_name	= "DC_5V",
	.microvolts	= 5000000,
	.gpio		= -EINVAL,
	.init_data	= &mipi_csi_fixed_voltage_init_data,
};

static struct platform_device mipi_csi_fixed_voltage = {
	.name		= "reg-fixed-voltage",
	.id		= 3,
	.dev		= {
		.platform_data	= &mipi_csi_fixed_voltage_config,
	},
};
#endif

#ifdef CONFIG_VIDEO_M5MOLS
static struct regulator_consumer_supply m5mols_fixed_voltage_supplies[] = {
	REGULATOR_SUPPLY("core", NULL),
	REGULATOR_SUPPLY("dig_18", NULL),
	REGULATOR_SUPPLY("d_sensor", NULL),
	REGULATOR_SUPPLY("dig_28", NULL),
	REGULATOR_SUPPLY("a_sensor", NULL),
	REGULATOR_SUPPLY("dig_12", NULL),
};

static struct regulator_init_data m5mols_fixed_voltage_init_data = {
	.constraints = {
		.always_on = 1,
	},
	.num_consumer_supplies	= ARRAY_SIZE(m5mols_fixed_voltage_supplies),
	.consumer_supplies	= m5mols_fixed_voltage_supplies,
};

static struct fixed_voltage_config m5mols_fixed_voltage_config = {
	.supply_name	= "CAM_SENSOR",
	.microvolts	= 1800000,
	.gpio		= -EINVAL,
	.init_data	= &m5mols_fixed_voltage_init_data,
};

static struct platform_device m5mols_fixed_voltage = {
	.name		= "reg-fixed-voltage",
	.id		= 4,
	.dev		= {
		.platform_data	= &m5mols_fixed_voltage_config,
	},
};
#endif

#if defined(CONFIG_REGULATOR_MAX77686)
/* max77686 */
#ifdef CONFIG_SND_SOC_WM8994
static struct regulator_consumer_supply ldo3_supply[] = {
	REGULATOR_SUPPLY("vcc_1.8v", NULL),
	REGULATOR_SUPPLY("AVDD2", NULL),
	REGULATOR_SUPPLY("CPVDD", NULL),
	REGULATOR_SUPPLY("DBVDD1", NULL),
	REGULATOR_SUPPLY("DBVDD2", NULL),
	REGULATOR_SUPPLY("DBVDD3", NULL)
};
#else
static struct regulator_consumer_supply ldo3_supply[] = {
	REGULATOR_SUPPLY("vcc_1.8v", NULL),
};
#endif

static struct regulator_consumer_supply ldo8_supply[] = {
	REGULATOR_SUPPLY("vmipi_1.0v", NULL),
};

static struct regulator_consumer_supply ldo9_supply[] = {
	REGULATOR_SUPPLY("touch_vdd_1.8v", NULL),
};

static struct regulator_consumer_supply ldo10_supply[] = {
	REGULATOR_SUPPLY("vmipi_1.8v", NULL),
};

static struct regulator_consumer_supply ldo12_supply[] = {
	REGULATOR_SUPPLY("votg_3.0v", NULL),
};

static struct regulator_consumer_supply ldo15_supply[] = {
	REGULATOR_SUPPLY("vhsic_1.0v", NULL),
};

static struct regulator_consumer_supply ldo16_supply[] = {
	REGULATOR_SUPPLY("vhsic_1.8v", NULL),
};

static struct regulator_consumer_supply ldo17_supply[] = {
	REGULATOR_SUPPLY("from_1.8v", NULL),
};

static struct regulator_consumer_supply ldo18_supply[] = {
	REGULATOR_SUPPLY("cam_io_1.8v", NULL),
};

static struct regulator_consumer_supply ldo19_supply[] = {
	REGULATOR_SUPPLY("vt_cam_1.8v", NULL),
};

static struct regulator_consumer_supply ldo23_supply[] = {
	REGULATOR_SUPPLY("touch_avdd", NULL),
};
static struct regulator_consumer_supply ldo24_supply[] = {
	REGULATOR_SUPPLY("cam_af_2.8v", NULL),
};
static struct regulator_consumer_supply ldo25_supply[] = {
	REGULATOR_SUPPLY("vadc_3.3v", NULL),
};
static struct regulator_consumer_supply max77686_buck1 =
	REGULATOR_SUPPLY("vdd_mif", NULL);
static struct regulator_consumer_supply max77686_buck2 =
	REGULATOR_SUPPLY("vdd_arm", NULL);

static struct regulator_consumer_supply max77686_buck3 =
	REGULATOR_SUPPLY("vdd_int", NULL);

static struct regulator_consumer_supply max77686_buck4 =
	REGULATOR_SUPPLY("vdd_g3d", NULL);

static struct regulator_consumer_supply max77686_buck9 =
	REGULATOR_SUPPLY("cam_5m_core_1.8v", NULL);

#define REGULATOR_INIT(_ldo, _name, _min_uV, _max_uV, _always_on, _ops_mask,\
	_disabled) \
static struct regulator_init_data _ldo##_init_data = {          \
	.constraints = {                                        \
		.name   = _name,                                \
		.min_uV = _min_uV,                              \
		.max_uV = _max_uV,                              \
		.always_on      = _always_on,                   \
		.boot_on        = _always_on,                   \
		.apply_uV       = 1,                            \
		.valid_ops_mask = _ops_mask,                    \
		.state_mem      = {                             \
		.disabled       = _disabled,            \
		.enabled        = !(_disabled),         \
		}                                               \
	    },                                                      \
	    .num_consumer_supplies = ARRAY_SIZE(_ldo##_supply),     \
	    .consumer_supplies = &_ldo##_supply[0],                 \
	       };

REGULATOR_INIT(ldo3, "VCC_1.8V_AP", 1800000, 1800000, 1,
	REGULATOR_CHANGE_STATUS, 0);
REGULATOR_INIT(ldo8, "VMIPI_1.0V", 1000000, 1000000, 0,
	REGULATOR_CHANGE_STATUS, 1);
#if defined(CONFIG_TOUCHSCREEN_ATMEL_MXT1664S)
REGULATOR_INIT(ldo9, "TOUCH_VDD_1.8V", 1800000, 1800000, 0,
	REGULATOR_CHANGE_STATUS, 1);
#else
REGULATOR_INIT(ldo9, "TOUCH_VDD_1.8V", 2000000, 2000000, 0,
	REGULATOR_CHANGE_STATUS, 1);
#endif
REGULATOR_INIT(ldo10, "VMIPI_1.8V", 1800000, 1800000, 0,
	REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo12, "VUOTG_3.0V", 3000000, 3000000, 1,
	REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo15, "VHSIC_1.0V", 1000000, 1000000, 0,
	REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo16, "VHSIC_1.8V", 1800000, 1800000, 0,
	REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo17, "FROM_1.8V", 1800000, 1800000, 0,
	REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo18, "CAM_IO_1.8V", 1800000, 1800000, 0,
	REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo19, "VT_CAM_1.8V", 1800000, 1800000, 0,
	REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo23, "TSP_AVDD_3.3V", 2800000, 2800000, 0,
	REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo24, "CAM_AF_2.8V", 2800000, 2800000, 0,
	REGULATOR_CHANGE_STATUS, 1);
REGULATOR_INIT(ldo25, "VADC_3.3V", 3300000, 3300000, 0,
	REGULATOR_CHANGE_STATUS, 1);

static struct regulator_init_data max77686_buck1_data = {
	.constraints = {
		.name = "vdd_mif range",
		.min_uV = 900000,
		.max_uV = 1300000,
		.always_on = 1,
		.boot_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &max77686_buck1,
};

static struct regulator_init_data max77686_buck2_data = {
	.constraints = {
		.name = "vdd_arm range",
		.min_uV = 800000,
		.max_uV = 1500000,
		.always_on = 1,
		.boot_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
		.state_mem = {
		.disabled = 1,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &max77686_buck2,
};

static struct regulator_init_data max77686_buck3_data = {
	.constraints = {
		.name = "vdd_int range",
		.min_uV = 900000,
		.max_uV = 1300000,
		.always_on = 1,
		.boot_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
		.state_mem = {
		.disabled = 1,
		},
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &max77686_buck3,
};

static struct regulator_init_data max77686_buck4_data = {
	 .constraints = {
		 .name = "vdd_g3d range",
		 .min_uV = 900000,
		 .max_uV = 1300000,
		 .boot_on = 1,
		 .valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
		 REGULATOR_CHANGE_STATUS,
		 .state_mem = {
		 .disabled = 1,
		 },
	 },
	 .num_consumer_supplies = 1,
	 .consumer_supplies = &max77686_buck4,
};

static struct regulator_init_data max77686_buck9_data = {
	  .constraints = {
		.name = "cam_5m_core",
		.min_uV = 1800000,
		.max_uV = 1800000,
		.apply_uV = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem = {
		.disabled = 1,
		},
	 },
	 .num_consumer_supplies = 1,
	 .consumer_supplies = &max77686_buck9,
};

static struct max77686_regulator_data max77686_regulators[] = {
		{MAX77686_BUCK1, &max77686_buck1_data,},
		{MAX77686_BUCK2, &max77686_buck2_data,},
		{MAX77686_BUCK3, &max77686_buck3_data,},
		{MAX77686_BUCK4, &max77686_buck4_data,},
		{MAX77686_BUCK9, &max77686_buck9_data,},
		{MAX77686_LDO3, &ldo3_init_data,},
		{MAX77686_LDO8, &ldo8_init_data,},
		{MAX77686_LDO9, &ldo9_init_data,},
		{MAX77686_LDO10, &ldo10_init_data,},
		{MAX77686_LDO12, &ldo12_init_data,},
		{MAX77686_LDO15, &ldo15_init_data,},
		{MAX77686_LDO16, &ldo16_init_data,},
		{MAX77686_LDO17, &ldo17_init_data,},
		{MAX77686_LDO18, &ldo18_init_data,},
		{MAX77686_LDO19, &ldo19_init_data,},
		{MAX77686_LDO23, &ldo23_init_data,},
		{MAX77686_LDO24, &ldo24_init_data,},
		{MAX77686_LDO25, &ldo25_init_data,},
};

struct max77686_opmode_data max77686_opmode_data[MAX77686_REG_MAX] = {
		[MAX77686_LDO3] = {MAX77686_LDO3, MAX77686_OPMODE_NORMAL},
		[MAX77686_LDO12] = {MAX77686_LDO12, MAX77686_OPMODE_STANDBY},
		[MAX77686_BUCK1] = {MAX77686_BUCK1, MAX77686_OPMODE_STANDBY},
		[MAX77686_BUCK2] = {MAX77686_BUCK2, MAX77686_OPMODE_STANDBY},
		[MAX77686_BUCK3] = {MAX77686_BUCK3, MAX77686_OPMODE_STANDBY},
		[MAX77686_BUCK4] = {MAX77686_BUCK4, MAX77686_OPMODE_STANDBY},
};

static struct max77686_platform_data exynos4_max77686_info = {
		.num_regulators = ARRAY_SIZE(max77686_regulators),
		.regulators = max77686_regulators,
		.irq_gpio	= GPIO_PMIC_IRQ,
		.irq_base	= IRQ_BOARD_PMIC_START,
		.wakeup = 1,

		.opmode_data = max77686_opmode_data,
		.ramp_rate = MAX77686_RAMP_RATE_27MV,

		.buck234_gpio_dvs = {
			GPIO_PMIC_DVS1,
			GPIO_PMIC_DVS2,
			GPIO_PMIC_DVS3,
		},
		.buck234_gpio_selb = {
			GPIO_BUCK2_SEL,
			GPIO_BUCK3_SEL,
			GPIO_BUCK4_SEL,
		},

		/*for future work after DVS Table */
		.buck2_voltage[0] = 1100000,    /* 1.1V */
		.buck2_voltage[1] = 1100000,    /* 1.1V */
		.buck2_voltage[2] = 1100000,    /* 1.1V */
		.buck2_voltage[3] = 1100000,    /* 1.1V */
		.buck2_voltage[4] = 1100000,    /* 1.1V */
		.buck2_voltage[5] = 1100000,    /* 1.1V */
		.buck2_voltage[6] = 1100000,    /* 1.1V */
		.buck2_voltage[7] = 1100000,    /* 1.1V */

		.buck3_voltage[0] = 1100000,    /* 1.1V */
		.buck3_voltage[1] = 1100000,    /* 1.1V */
		.buck3_voltage[2] = 1100000,    /* 1.1V */
		.buck3_voltage[3] = 1100000,    /* 1.1V */
		.buck3_voltage[4] = 1100000,    /* 1.1V */
		.buck3_voltage[5] = 1100000,    /* 1.1V */
		.buck3_voltage[6] = 1100000,    /* 1.1V */
		.buck3_voltage[7] = 1100000,    /* 1.1V */

		.buck4_voltage[0] = 1100000,    /* 1.1V */
		.buck4_voltage[1] = 1100000,    /* 1.1V */
		.buck4_voltage[2] = 1100000,    /* 1.1V */
		.buck4_voltage[3] = 1100000,    /* 1.1V */
		.buck4_voltage[4] = 1100000,    /* 1.1V */
		.buck4_voltage[5] = 1100000,    /* 1.1V */
		.buck4_voltage[6] = 1100000,    /* 1.1V */
		.buck4_voltage[7] = 1100000,    /* 1.1V */
};
#endif /* CONFIG_REGULATOR_MAX77686 */

static struct i2c_board_info i2c_devs0[] __initdata = {
#ifdef CONFIG_VIDEO_EXYNOS_TV
	{
		I2C_BOARD_INFO("exynos_hdcp", (0x74 >> 1)),
	}
#endif
};

static struct i2c_board_info i2c_devs1[] __initdata = {
	{
		I2C_BOARD_INFO("dummy", (0x10)),
	}
};

#if defined(CONFIG_TOUCHSCREEN_ATMEL_MXT1536E)
static void ts_read_ta_status(bool *ta_status)
{
	*ta_status = false;
}

static void ts_power_on(void)
{
	struct regulator *regulator;

	regulator = regulator_get(NULL, "touch_vdd_1.8v");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "[TSP]ts_power_on : regulator_get failed\n");
		return;
	}

	regulator_enable(regulator);
	regulator_put(regulator);

	regulator = regulator_get(NULL, "touch_avdd");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "[TSP]ts_power_on : regulator_get failed\n");
		return;
	}
	regulator_enable(regulator);
	regulator_put(regulator);

	/* touch reset pin */
	s3c_gpio_cfgpin(GPIO_TOUCH_RESET, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_TOUCH_RESET, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_TOUCH_RESET, 1);

	/* touch interrupt pin */
	/* s3c_gpio_cfgpin(GPIO_TOUCH_CHG, S3C_GPIO_INPUT); */
	s3c_gpio_cfgpin(GPIO_TOUCH_CHG, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(GPIO_TOUCH_CHG, S3C_GPIO_PULL_NONE);
#if 0
	s3c_gpio_cfgpin(GPIO_TSP_SCL_18V, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_TSP_SCL_18V, S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(GPIO_TSP_SDA_18V, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_TSP_SDA_18V, S3C_GPIO_PULL_NONE);
#endif

	printk(KERN_DEBUG "mxt_power_on is finished\n");
}

static void ts_power_off(void)
{
	struct regulator *regulator;

	regulator = regulator_get(NULL, "touch_avdd");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "[TSP]ts_power_off : regulator_get failed\n");
		return;
	}

	if (regulator_is_enabled(regulator))
		regulator_force_disable(regulator);

	regulator_put(regulator);

	regulator = regulator_get(NULL, "touch_vdd_1.8v");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "[TSP]ts_power_on : regulator_get failed\n");
		return;
	}

	if (regulator_is_enabled(regulator))
		regulator_force_disable(regulator);

	regulator_put(regulator);

	/* touch interrupt pin */
	s3c_gpio_cfgpin(GPIO_TOUCH_CHG, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_TOUCH_CHG, S3C_GPIO_PULL_NONE);

	/* touch reset pin */
	s3c_gpio_cfgpin(GPIO_TOUCH_RESET, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_TOUCH_RESET, S3C_GPIO_PULL_NONE);
	/* gpio_set_value(GPIO_TOUCH_RESET, GPIO_LEVEL_LOW); */
	gpio_set_value(GPIO_TOUCH_RESET, 0);

	printk(KERN_ERR "mxt_power_off is finished\n");
}

/*
	Configuration for MXT1536-E
*/
#define MXT1536E_MAX_MT_FINGERS		10
#define MXT1536E_CHRGTIME_BATT		64
#define MXT1536E_CHRGTIME_CHRG		64
#define MXT1536E_THRESHOLD_BATT		30 /* 90 */
#define MXT1536E_THRESHOLD_CHRG		48
#define MXT1536E_CALCFG_BATT		224 /* 210 */
#define MXT1536E_CALCFG_CHRG		242

#define MXT1536E_ATCHCALSTHR_NORMAL	50
#define MXT1536E_ATCHFRCCALTHR_NORMAL	50
#define MXT1536E_ATCHFRCCALRATIO_NORMAL	0
#define MXT1536E_ATCHFRCCALTHR_WAKEUP	8
#define MXT1536E_ATCHFRCCALRATIO_WAKEUP	136

#define MXT1536E_IDLESYNCSPERX_BATT	38
#define MXT1536E_IDLESYNCSPERX_CHRG	40
#define MXT1536E_ACTVSYNCSPERX_BATT	38
#define MXT1536E_ACTVSYNCSPERX_CHRG	40

#define MXT1536E_IDLEACQINT_BATT	24
#define MXT1536E_IDLEACQINT_CHRG	24
#define MXT1536E_ACTACQINT_BATT		255
#define MXT1536E_ACTACQINT_CHRG		255
#define MXT1536E_ACTACQINT_BATT_WAKEUP	16
#define MXT1536E_ACTACQINT_CHRG_WAKEUP	16

#define MXT1536E_XLOCLIP_BATT		0
#define MXT1536E_XLOCLIP_CHRG		12
#define MXT1536E_XHICLIP_BATT		0
#define MXT1536E_XHICLIP_CHRG		12
#define MXT1536E_YLOCLIP_BATT		0
#define MXT1536E_YLOCLIP_CHRG		5
#define MXT1536E_YHICLIP_BATT		0
#define MXT1536E_YHICLIP_CHRG		5
#define MXT1536E_XEDGECTRL_BATT		0  /* 136 */
#define MXT1536E_XEDGECTRL_CHRG		128
#define MXT1536E_XEDGEDIST_BATT		0  /* 50 */
#define MXT1536E_XEDGEDIST_CHRG		0
#define MXT1536E_YEDGECTRL_BATT		0  /* 136 */
#define MXT1536E_YEDGECTRL_CHRG		136
#define MXT1536E_YEDGEDIST_BATT		0  /* 40 */
#define MXT1536E_YEDGEDIST_CHRG		30
#define MXT1536E_TCHHYST_BATT		15 /* 0 */
#define MXT1536E_TCHHYST_CHRG		5

static u8 t7_config_e[] = { GEN_POWERCONFIG_T7,
	48, 255, 25
};

static u8 t8_config_e[] = { GEN_ACQUISITIONCONFIG_T8,
	60, 0, 20, 20, 0, 0, 20,
	30,
	50,
	0
};
#if 0
static u8 t9_config_e[] = { TOUCH_MULTITOUCHSCREEN_T9,
	139, 0, 0, 48, 32, 0, 0x00, MXT1536E_THRESHOLD_BATT, 3, 4,
	50, 25, 2, 0, MXT1536E_MAX_MT_FINGERS, 5, 5, 50, 0, 0,
	0, 0, MXT1536E_XLOCLIP_BATT, MXT1536E_XHICLIP_BATT,
	MXT1536E_YLOCLIP_BATT, MXT1536E_YHICLIP_BATT,
	MXT1536E_XEDGECTRL_BATT, MXT1536E_XEDGEDIST_BATT,
	MXT1536E_YEDGECTRL_BATT, MXT1536E_YEDGEDIST_BATT,
	12, MXT1536E_TCHHYST_BATT, 43, 51, 0
};
#else
static u8 t9_config_e[] = { TOUCH_MULTITOUCHSCREEN_T9,
	139, 0, 0, 48, 32, 0, 0x80, MXT1536E_THRESHOLD_BATT, 2, 0,
	50, 25, 2, 0, MXT1536E_MAX_MT_FINGERS, 5, 5, 50, 0, 0,
	0, 0, MXT1536E_XLOCLIP_BATT, MXT1536E_XHICLIP_BATT,
	MXT1536E_YLOCLIP_BATT, MXT1536E_YHICLIP_BATT,
	MXT1536E_XEDGECTRL_BATT, MXT1536E_XEDGEDIST_BATT,
	MXT1536E_YEDGECTRL_BATT, MXT1536E_YEDGEDIST_BATT,
	12, MXT1536E_TCHHYST_BATT, 43, 51, 0
};
#endif

static u8 t15_config_e[] = { TOUCH_KEYARRAY_T15,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static u8 t18_config_e[] = { SPT_COMCONFIG_T18,
	0, 0
};

static u8 t19_config_e[] = { SPT_GPIOPWM_T19,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static u8 t25_config_e[] = { SPT_SELFTEST_T25,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static u8 t40_config_e[] = { PROCI_GRIPSUPPRESSION_T40,
	0, 0, 0, 0, 0
};

static u8 t42_config_e[] = { PROCI_TOUCHSUPPRESSION_T42,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static u8 t43_config_e[] = { SPT_DIGITIZER_T43,
	0, 0, 0, 0, 0, 0, 0
};

static u8 t46_config_e[] = { SPT_CTECONFIG_T46,
	0, 0, 16,
	24, 0, 0, 2, 47, 0, 0
};

static u8 t47_config_e[] = { PROCI_STYLUS_T47,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static u8 t48_config_e[] = {PROCG_NOISESUPPRESSION_T48,
	19, 192, MXT1536E_CALCFG_BATT, 0, 0, 0, 0, 0, 0, 0,
	176, 20, 0, 6, 6, 0, 0, 48, 4, 64,
	0, 0, 20, 0, 0, 0, 0, 20, 0, 0,
	0, 0, 0, 0, 112, MXT1536E_THRESHOLD_CHRG, 2, 16, 2, 80,
	MXT1536E_MAX_MT_FINGERS, 20, 40, 250, 250, 5, 5, 143, 50, 136,
	30, 12, MXT1536E_TCHHYST_CHRG, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0
};

/*
static u8 t48_config_chrg_e[] = {PROCG_NOISESUPPRESSION_T48,
	3, 128, MXT1536E_CALCFG_CHRG, 0, 0, 0, 0, 0, 0, 0,
	112, 15, 0, 6, 6, 0, 0, 44, 4, 64,
	0, 0, 20, 0, 0, 0, 0, 15, 0, 0,
	0, 0, 0, 0, 112, MXT1536E_THRESHOLD_CHRG, 2, 16, 2, 80,
	MXT1536E_MAX_MT_FINGERS, 20, 40, 251, 251, 6, 6, 144, 50, 136,
	30, 12, MXT1536E_TCHHYST_CHRG, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0
};
*/

static u8 t52_config_e[] = { TOUCH_PROXIMITY_KEY_T52,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static u8 t55_config_e[] = {ADAPTIVE_T55,
	0, 0, 0, 0, 0
};

static u8 t56_config_e[] = {PROCI_SHIELDLESS_T56,
	3,    0,  0,   24,  12,  12,  12,  12, 12, 12,
	12,  12,  12,  12,  12,  12,  12,  12, 12, 12,
	12,  12,  12,  12,  12,  12,  12,  12, 12, 12,
	12,  12,  12,  12,  12,  12,  12,  12, 12, 12,
	12,  12,  12,  12,  12,  12,  12,  12, 12, 12,
	12,  12,  64,  11,  55
};

static u8 t57_config_e[] = {SPT_GENERICDATA_T57,
	0, 0, 0
};

static u8 end_config_e[] = { RESERVED_T255 };

static const u8 *MXT1536E_config[] = {
	t7_config_e,
	t8_config_e,
	t9_config_e,
	t15_config_e,
	t18_config_e,
	t19_config_e,
	t25_config_e,
	t40_config_e,
	t42_config_e,
	t43_config_e,
	t46_config_e,
	t47_config_e,
	t48_config_e,
	t52_config_e,
	t55_config_e,
	t56_config_e,
	t57_config_e,
	end_config_e,
};

static struct mxt_platform_data mxt_data = {
	.max_finger_touches = MXT1536E_MAX_MT_FINGERS,
	.gpio_read_done = GPIO_TOUCH_CHG,

	.min_x = 0,
	.max_x = 2559,
	.min_y = 0,
	.max_y = 1599,
	.min_z = 0,
	.max_z = 255,
	.min_w = 0,
	.max_w = 30,
	.config_e = MXT1536E_config,
	/* .t48_config_batt_e = t48_config_e, */
	/* .t48_config_chrg_e = t48_config_chrg_e, */
	.power_on = ts_power_on,
	.power_off = ts_power_off,
	/* .register_cb = ts_register_callback, */
	.read_ta_status = ts_read_ta_status,
};
#endif

#if defined(CONFIG_TOUCHSCREEN_ATMEL_MXT1664S)
static void ts_read_ta_status(bool *ta_status)
{
	*ta_status = false;
}

static void ts_power_on(void)
{
	struct regulator *regulator;

	/* touch reset pin */
	s3c_gpio_cfgpin(GPIO_TOUCH_RESET, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_TOUCH_RESET, S3C_GPIO_PULL_NONE);
	gpio_set_value(GPIO_TOUCH_RESET, 0);

	regulator = regulator_get(NULL, "touch_vdd_1.8v");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "[TSP]ts_power_on : regulator_get failed\n");
		return;
	}

	regulator_enable(regulator);
	regulator_put(regulator);

	regulator = regulator_get(NULL, "touch_avdd");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "[TSP]ts_power_on : regulator_get failed\n");
		return;
	}
	regulator_enable(regulator);
	regulator_put(regulator);

	/* reset ic */
	mdelay(1);
	gpio_set_value(GPIO_TOUCH_RESET, 1);

	/* touch interrupt pin */
	/* s3c_gpio_cfgpin(GPIO_TOUCH_CHG, S3C_GPIO_INPUT); */

	s3c_gpio_cfgpin(GPIO_TOUCH_CHG, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(GPIO_TOUCH_CHG, S3C_GPIO_PULL_NONE);
#if 0
	s3c_gpio_cfgpin(GPIO_TSP_SCL_18V, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_TSP_SCL_18V, S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(GPIO_TSP_SDA_18V, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_TSP_SDA_18V, S3C_GPIO_PULL_NONE);
#endif
	msleep(MXT_HW_RESET_TIME);

	printk(KERN_DEBUG "mxt_power_on is finished\n");
}

static void ts_power_off(void)
{
	struct regulator *regulator;

	regulator = regulator_get(NULL, "touch_avdd");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "[TSP]ts_power_off : regulator_get failed\n");
		return;
	}

	if (regulator_is_enabled(regulator))
		regulator_force_disable(regulator);

	regulator_put(regulator);

	regulator = regulator_get(NULL, "touch_vdd_1.8v");
	if (IS_ERR(regulator)) {
		printk(KERN_ERR "[TSP]ts_power_on : regulator_get failed\n");
		return;
	}

	if (regulator_is_enabled(regulator))
		regulator_force_disable(regulator);

	regulator_put(regulator);

	/* touch interrupt pin */
	s3c_gpio_cfgpin(GPIO_TOUCH_CHG, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_TOUCH_CHG, S3C_GPIO_PULL_NONE);

	/* touch reset pin */
	s3c_gpio_cfgpin(GPIO_TOUCH_RESET, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_TOUCH_RESET, S3C_GPIO_PULL_NONE);
	/* gpio_set_value(GPIO_TOUCH_RESET, GPIO_LEVEL_LOW); */
	gpio_set_value(GPIO_TOUCH_RESET, 0);

	printk(KERN_ERR "mxt_power_off is finished\n");
}

/*
	Configuration for MXT1664-S
*/
#define MXT1664S_MAX_MT_FINGERS		10
#define MXT1664S_CHRGTIME_BATT		60
#define MXT1664S_CHRGTIME_CHRG		60
#define MXT1664S_THRESHOLD_BATT		70
#define MXT1664S_THRESHOLD_CHRG		70
#define MXT1664S_CALCFG_BATT		210
#define MXT1664S_CALCFG_CHRG		210

static u8 t7_config_s[] = { GEN_POWERCONFIG_T7,
	255, 255, 50
};

static u8 t8_config_s[] = { GEN_ACQUISITIONCONFIG_T8,
	MXT1664S_CHRGTIME_BATT, 0, 10, 10, 0, 0, 20, 35, 0, 0
};

static u8 t9_config_s[] = { TOUCH_MULTITOUCHSCREEN_T9,
	139, 0, 0, 32, 52, 0, 0x40, MXT1664S_THRESHOLD_BATT, 2, 1,
	0, 5, 2, 0, MXT1664S_MAX_MT_FINGERS, 10, 10, 20, 0x3F, 6,
	0xFF, 9, 0, 0, 0, 0, 0, 0, 0, 0,
	15, 15, 42, 42, 0
};

static u8 t15_config_s[] = { TOUCH_KEYARRAY_T15,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0
};

static u8 t18_config_s[] = { SPT_COMCONFIG_T18,
	0, 0
};

static u8 t24_config_s[] = {
	PROCI_ONETOUCHGESTUREPROCESSOR_T24,
	0, 4, 255, 3, 63, 100, 100, 1, 10, 20,
	40, 75, 2, 2, 100, 100, 25, 25, 0
};

static u8 t27_config_s[] = {
	PROCI_TWOTOUCHGESTUREPROCESSOR_T27,
	0, 1, 0, 244, 3, 35
};

static u8 t40_config_s[] = { PROCI_GRIPSUPPRESSION_T40,
	0, 20, 20, 20, 20
};

static u8 t42_config_s[] = { PROCI_TOUCHSUPPRESSION_T42,
	3, 60, 100, 60, 0, 20, 0, 0, 0, 0
};

static u8 t43_config_s[] = { SPT_DIGITIZER_T43,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0
};

static u8 t46_config_s[] = { SPT_CTECONFIG_T46,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0
};

static u8 t47_config_s[] = { PROCI_STYLUS_T47,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static u8 t48_config_s[] = {PROCG_NOISESUPPRESSION_T48,
	3, 128, MXT1664S_CALCFG_BATT, 0, 0, 0, 0, 0, 0, 0,
	48, 25, 0, 6, 6, 0, 0, 64, 4, 64,
	0, 0, 25, 0, 0, 0, 0, 20, 0, 100,
	0, 10, 10, 0, 48, MXT1664S_THRESHOLD_CHRG, 0, 50, 1, 0,
	MXT1664S_MAX_MT_FINGERS, 10, 10, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0
};

static u8 t48_config_chrg_s[] = {PROCG_NOISESUPPRESSION_T48,
	3, 128, MXT1664S_CALCFG_CHRG, 0, 0, 0, 0, 0, 0, 0,
	48, 25, 0, 6, 6, 0, 0, 64, 4, 64,
	0, 0, 25, 0, 0, 0, 0, 20, 0, 100,
	0, 10, 10, 0, 48, MXT1664S_THRESHOLD_CHRG, 0, 50, 1, 0,
	MXT1664S_MAX_MT_FINGERS, 10, 10, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0
};

static u8 t55_config_s[] = {ADAPTIVE_T55,
	0, 0, 0, 0, 0, 0
};

static u8 t56_config_s[] = {PROCI_SHIELDLESS_T56,
	3, 0, 1, 24, 21, 21, 22, 22, 22, 23,
	23, 24, 24, 25, 25, 25, 25, 25, 25, 25,
	24, 24, 24, 24, 24, 24, 24, 23, 24, 23,
	23, 23, 22, 22, 21, 21, 2, 255, 1, 2,
	0, 5
};

static u8 t57_config_s[] = {SPT_GENERICDATA_T57,
	227, 25, 0
};

static u8 end_config_s[] = { RESERVED_T255 };

static const u8 *MXT1644S_config[] = {
	t7_config_s,
	t8_config_s,
	t9_config_s,
	t15_config_s,
	t18_config_s,
	t24_config_s,
	t27_config_s	,
	t40_config_s,
	t42_config_s,
	t43_config_s,
	t46_config_s,
	t47_config_s,
	t48_config_s,
	t55_config_s,
	t56_config_s,
	t57_config_s,
	end_config_s,
};

static struct mxt_platform_data mxt_data = {
	.max_finger_touches = MXT1664S_MAX_MT_FINGERS,
	.gpio_read_done = GPIO_TOUCH_CHG,
	.min_x = 0,
	.max_x = 2559,
	.min_y = 0,
	.max_y = 1599,
	.min_z = 0,
	.max_z = 255,
	.min_w = 0,
	.max_w = 30,
	.config = MXT1644S_config,
	/* .t48_config_batt = t48_config_s, */
	/* .t48_config_chrg = t48_config_chrg_s, */
	.power_on = ts_power_on,
	.power_off = ts_power_off,
	/* .register_cb = ts_register_callback, */
	.read_ta_status = ts_read_ta_status,
};
#endif

static struct i2c_board_info i2c_devs3[] __initdata = {
#if defined(CONFIG_TOUCHSCREEN_ATMEL_MXT1536E)
	{
		I2C_BOARD_INFO(MXT_DEV_NAME, 0x4C),
		.platform_data = &mxt_data,
	}
#endif
#if defined(CONFIG_TOUCHSCREEN_ATMEL_MXT1664S)
	{
		I2C_BOARD_INFO(MXT_DEV_NAME, 0x4A),
		.platform_data = &mxt_data,
	}
#endif
};

static void __init universal_tsp_init(void)
{
	int gpio;

	gpio = GPIO_TOUCH_CHG;
	gpio_request(gpio, "TSP_INT");
	s3c_gpio_cfgpin(gpio, S3C_GPIO_INPUT);
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	s5p_register_gpio_interrupt(GPIO_TOUCH_CHG);
	i2c_devs3[0].irq = gpio_to_irq(gpio);

	printk(KERN_ERR "%s touch : %d\n", __func__, i2c_devs3[0].irq);

}

#if defined(CONFIG_REGULATOR_MAX77686)
static struct i2c_board_info i2c_devs5[] __initdata = {
	{
		I2C_BOARD_INFO("max77686", (0x12 >> 1)),
		.platform_data  = &exynos4_max77686_info,
	}
};
#endif

/* vbatt device (for WM8994) */
static struct regulator_consumer_supply vbatt_supplies[] = {
	REGULATOR_SUPPLY("LDO1VDD", "7-001a"),
	REGULATOR_SUPPLY("SPKVDD1", "7-001a"),
	REGULATOR_SUPPLY("SPKVDD2", "7-001a"),
};

static struct regulator_init_data vbatt_initdata = {
	.constraints = {
		.always_on = 1,
	},
	.num_consumer_supplies = ARRAY_SIZE(vbatt_supplies),
	.consumer_supplies = vbatt_supplies,
};

static struct fixed_voltage_config vbatt_config = {
	.init_data = &vbatt_initdata,
	.microvolts = 5000000,
	.supply_name = "VBATT",
	.gpio = -EINVAL,
};

static struct platform_device vbatt_device = {
	.name = "reg-fixed-voltage",
	.id = -1,
	.dev = {
		. platform_data = &vbatt_config,
	},
};

#ifdef CONFIG_SND_SOC_WM8994
static struct regulator_consumer_supply wm1811_ldo1_supplies[] = {
	REGULATOR_SUPPLY("AVDD1", "7-001a"),
};

static struct regulator_init_data wm1811_ldo1_initdata = {
	.constraints = {
		.name = "WM1811 LDO1",
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
	},
	.num_consumer_supplies = ARRAY_SIZE(wm1811_ldo1_supplies),
	.consumer_supplies = wm1811_ldo1_supplies,
};

static struct regulator_consumer_supply wm1811_ldo2_supplies[] = {
	REGULATOR_SUPPLY("DCVDD", "7-001a"),
};

static struct regulator_init_data wm1811_ldo2_initdata = {
	.constraints = {
		.name = "WM1811 LDO2",
		.always_on = true,  /* Actually status changed by LDO1 */
	},
	.num_consumer_supplies = ARRAY_SIZE(wm1811_ldo2_supplies),
	.consumer_supplies = wm1811_ldo2_supplies,
};

static struct wm8994_pdata wm1811_pdata = {
	.gpio_defaults = {
		[0] = WM8994_GP_FN_IRQ,   /* GPIO1 IRQ output, CMOS mode */
		[7] = WM8994_GPN_DIR | WM8994_GP_FN_PIN_SPECIFIC,  /* DACDAT3 */
		[8] = WM8994_CONFIGURE_GPIO | WM8994_GP_FN_PIN_SPECIFIC,  /* ADCDAT3 */
		[9] = WM8994_CONFIGURE_GPIO | WM8994_GP_FN_PIN_SPECIFIC,  /* LRCLK3 */
		[10] = WM8994_CONFIGURE_GPIO | WM8994_GP_FN_PIN_SPECIFIC, /* BCLK3 */
	},
	/* To do */
	.irq_base = IRQ_BOARD_CODEC_START,

	/* The enable is shared but assign it to LDO1 for software */
	.ldo = {
		{
			.enable = GPIO_CODEC_LDO_EN,
			.init_data = &wm1811_ldo1_initdata,
		},
		{
			.init_data = &wm1811_ldo2_initdata,
		},
	},

	/* Regulated mode at highest output voltage */
	.micbias = { 0x3f, 0x3e },
	.ldo_ena_always_driven = true,
};
#endif

#ifdef CONFIG_SND_SOC_WM8994
static struct i2c_board_info i2c_devs7[] __initdata = {
	{
		I2C_BOARD_INFO("wm1811", (0x34 >> 1)), /* Audio CODEC */
		.platform_data = &wm1811_pdata,
	},
};
#endif

struct gpio_keys_button p10_buttons[] = {
	{ .code = KEY_VOLUMEUP,   .gpio = GPIO_VOL_UP,   .active_low = 1, },
	{ .code = KEY_VOLUMEDOWN, .gpio = GPIO_VOL_DOWN, .active_low = 1, },
	{ .code = KEY_POWER,      .gpio = GPIO_nPOWER,   .active_low = 1, .wakeup = 1},
};

struct gpio_keys_platform_data p10_gpiokeys_platform_data = {
	p10_buttons,
	ARRAY_SIZE(p10_buttons),
};

static struct platform_device p10_keypad = {
	.name   = "gpio-keys",
	.dev    = {
		.platform_data = &p10_gpiokeys_platform_data,
	},
};

#if defined(CONFIG_MHL_SII9234)
static void sii9234_init(void)
{
	int ret = gpio_request(GPIO_HDMI_EN, "hdmi_en1");
	if (ret) {
		pr_err("%s: gpio_request() for HDMI_EN1 failed\n", __func__);
		return;
	}
	gpio_direction_output(GPIO_HDMI_EN, 0);
	if (ret) {
		pr_err("%s: gpio_direction_output() for HDMI_EN1 failed\n",
			__func__);
		return;
	}

	ret = gpio_request(GPIO_MHL_RST, "mhl_rst");
	if (ret) {
		pr_err("%s: gpio_request() for MHL_RST failed\n", __func__);
		return;
	}
	ret = gpio_direction_output(GPIO_MHL_RST, 0);
	if (ret) {
		pr_err("%s: gpio_direction_output() for MHL_RST failed\n",
			__func__);
		return;
	}
}

static void sii9234_hw_reset(void)
{
	gpio_set_value(GPIO_MHL_RST, 0);
	gpio_set_value(GPIO_HDMI_EN, 1);

	usleep_range(5000, 10000);
	gpio_set_value(GPIO_MHL_RST, 1);

	printk(KERN_ERR "[MHL]sii9234_hw_reset.\n");
	msleep(30);
}

static void sii9234_hw_off(void)
{
	gpio_set_value(GPIO_HDMI_EN, 0);
	gpio_set_value(GPIO_MHL_RST, 0);
	printk(KERN_ERR "[MHL]sii9234_hw_off.\n");
}

struct sii9234_platform_data sii9234_pdata = {
	.hw_reset = sii9234_hw_reset,
	.hw_off = sii9234_hw_off
};
static struct i2c_board_info i2c_devs15[] __initdata = {
	{
		I2C_BOARD_INFO("SII9234", 0x72>>1),
		.platform_data = &sii9234_pdata,
	},
	{
		I2C_BOARD_INFO("SII9234A", 0x7A>>1),
	},
	{
		I2C_BOARD_INFO("SII9234B", 0x92>>1),
	},
	{
		I2C_BOARD_INFO("SII9234C", 0xC8>>1),
	},
};
/* i2c-gpio emulation platform_data */
static struct i2c_gpio_platform_data i2c15_platdata = {
	.sda_pin		= GPIO_MHL_SDA_18V,
	.scl_pin		= GPIO_MHL_SCL_18V,
	.udelay			= 2,	/* 250 kHz*/
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device s3c_device_i2c15 = {
	.name			= "i2c-gpio",
	.id			= 15,
	.dev.platform_data	= &i2c15_platdata,
};

#endif

#ifdef CONFIG_30PIN_CONN
struct acc_con_platform_data acc_con_pdata = {
    /*
	.otg_en =
	.acc_power =
	.usb_ldo_en =
    */
	.accessory_irq_gpio = GPIO_ACCESSORY_INT,
	.dock_irq_gpio = GPIO_DOCK_INT,
	.mhl_irq_gpio = GPIO_MHL_INT,
	.hdmi_hpd_gpio = GPIO_HDMI_HPD,
};
struct platform_device sec_device_connector = {
	.name = "acc_con",
	.id = -1,
	.dev.platform_data = &acc_con_pdata,
};
#endif

#ifdef CONFIG_USB_EHCI_S5P
static struct s5p_ehci_platdata smdk5250_ehci_pdata;

static void __init smdk5250_ehci_init(void)
{
	struct s5p_ehci_platdata *pdata = &smdk5250_ehci_pdata;

	s5p_ehci_set_platdata(pdata);
}
#endif

#ifdef CONFIG_USB_OHCI_S5P
static struct s5p_ohci_platdata smdk5250_ohci_pdata;

static void __init smdk5250_ohci_init(void)
{
	struct s5p_ohci_platdata *pdata = &smdk5250_ohci_pdata;

	s5p_ohci_set_platdata(pdata);
}
#endif

#ifdef CONFIG_EXYNOS_DEV_SS_UDC
static struct exynos_ss_udc_plat smdk5250_ss_udc_pdata;

static void __init smdk5250_ss_udc_init(void)
{
	struct exynos_ss_udc_plat *pdata = &smdk5250_ss_udc_pdata;

	exynos_ss_udc_set_platdata(pdata);
}
#endif

#ifdef CONFIG_BATTERY_SAMSUNG
static struct platform_device samsung_device_battery = {
	.name	= "samsung-fake-battery",
	.id	= -1,
};
#endif

#ifdef CONFIG_BUSFREQ_OPP
/* BUSFREQ to control memory/bus*/
static struct device_domain busfreq;

static struct platform_device exynos5_busfreq = {
	.id = -1,
	.name = "exynos-busfreq",
};
#endif

static struct platform_device *p10_devices[] __initdata = {
	/* Samsung Power Domain */
#ifdef CONFIG_EXYNOS_DEV_PD
	&exynos5_device_pd[PD_G3D],
	&exynos5_device_pd[PD_ISP],
	&exynos5_device_pd[PD_GSCL],
	&exynos5_device_pd[PD_DISP1],
#endif

#ifdef CONFIG_S5P_DP
	&s5p_device_dp,
#endif

#ifdef CONFIG_FB_S3C
	&s5p_device_fimd1,
#endif

#ifdef CONFIG_S5P_DP
	&p10_dp_lcd,
#endif

	&s3c_device_wdt,

#ifdef CONFIG_S3C_DEV_HSMMC
	&s3c_device_hsmmc0,
#endif

#ifdef CONFIG_S3C_DEV_HSMMC1
	&s3c_device_hsmmc1,
#endif

#ifdef CONFIG_S3C_DEV_HSMMC2
	&s3c_device_hsmmc2,
#endif

#ifdef CONFIG_S3C_DEV_HSMMC3
	&s3c_device_hsmmc3,
#endif

#ifdef CONFIG_VIDEO_SAMSUNG_S5P_MFC
	&s5p_device_mfc,
#ifdef CONFIG_S5P_SYSTEM_MMU
	&SYSMMU_PLATDEV(mfc_l),
	&SYSMMU_PLATDEV(mfc_r),
#endif
#endif

#ifdef CONFIG_VIDEO_JPEG_V2X
	&s5p_device_jpeg,
#ifdef CONFIG_S5P_SYSTEM_MMU
	&SYSMMU_PLATDEV(jpeg),
#endif
#endif

#ifdef CONFIG_EXYNOS4_DEV_DWMCI
	&exynos_device_dwmci,
#endif
#ifdef CONFIG_ION_EXYNOS
	&exynos_device_ion,
#endif

#ifdef CONFIG_VIDEO_FIMG2D
	&s5p_device_fimg2d,
#ifdef CONFIG_S5P_SYSTEM_MMU
	&SYSMMU_PLATDEV(2d),
#endif
#endif

#ifdef CONFIG_EXYNOS_MEDIA_DEVICE
	&exynos_device_md0,
	&exynos_device_md1,
	&exynos_device_md2,
#endif

#ifdef CONFIG_VIDEO_EXYNOS5_FIMC_IS
	&exynos5_device_fimc_is,
#ifdef CONFIG_S5P_SYSTEM_MMU
	&SYSMMU_PLATDEV(is_isp),
	&SYSMMU_PLATDEV(is_drc),
	&SYSMMU_PLATDEV(is_fd),
	&SYSMMU_PLATDEV(is_cpu),
	&SYSMMU_PLATDEV(is_odc),
	&SYSMMU_PLATDEV(is_sclrc),
	&SYSMMU_PLATDEV(is_sclrp),
	&SYSMMU_PLATDEV(is_dis0),
	&SYSMMU_PLATDEV(is_dis1),
	&SYSMMU_PLATDEV(is_3dnr),
#endif
#endif

#ifdef CONFIG_VIDEO_EXYNOS_GSCALER
	&exynos5_device_gsc0,
	&exynos5_device_gsc1,
	&exynos5_device_gsc2,
	&exynos5_device_gsc3,
#ifdef CONFIG_S5P_SYSTEM_MMU
	&SYSMMU_PLATDEV(gsc0),
	&SYSMMU_PLATDEV(gsc1),
	&SYSMMU_PLATDEV(gsc2),
	&SYSMMU_PLATDEV(gsc3),
#endif
#endif

#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
	&exynos_device_flite0,
	&exynos_device_flite1,
#endif

#ifdef CONFIG_VIDEO_EXYNOS_MIPI_CSIS
	&s5p_device_mipi_csis0,
	&s5p_device_mipi_csis1,
	&mipi_csi_fixed_voltage,
#endif

#ifdef CONFIG_VIDEO_M5MOLS
	&m5mols_fixed_voltage,
#endif

	&s3c_device_rtc,

#ifdef CONFIG_VIDEO_EXYNOS_TV
#ifdef CONFIG_VIDEO_EXYNOS_HDMI
	&s5p_device_hdmi,
#endif
#ifdef CONFIG_VIDEO_EXYNOS_HDMIPHY
	&s5p_device_i2c_hdmiphy,
#endif
#ifdef CONFIG_VIDEO_EXYNOS_MIXER
	&s5p_device_mixer,
#endif
#ifdef CONFIG_VIDEO_EXYNOS_HDMI_CEC
	&s5p_device_cec,
#endif
#ifdef CONFIG_S5P_SYSTEM_MMU
	&SYSMMU_PLATDEV(tv),
#endif
#endif
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c3,
	&s3c_device_i2c5,
	&s3c_device_i2c7,
	&p10_keypad,

#ifdef CONFIG_USB_EHCI_S5P
	&s5p_device_ehci,
#endif

#ifdef CONFIG_USB_OHCI_S5P
	&s5p_device_ohci,
#endif

#ifdef CONFIG_EXYNOS_DEV_SS_UDC
	&exynos_device_ss_udc,
#endif

#ifdef CONFIG_BATTERY_SAMSUNG
	&samsung_device_battery,
#endif

#ifdef CONFIG_SND_SAMSUNG_I2S
	&exynos_device_i2s0,
#endif

	&samsung_asoc_dma,
	&samsung_asoc_idma,

#ifdef CONFIG_SND_SAMSUNG_PCM
	&exynos_device_pcm0,
#endif

#if defined(CONFIG_SND_SAMSUNG_RP) || defined(CONFIG_SND_SAMSUNG_ALP)
	&exynos_device_srp,
#endif

#ifdef CONFIG_EXYNOS_SETUP_THERMAL
	&exynos_device_tmu,
#endif

#ifdef CONFIG_S5P_DEV_ACE
	&s5p_device_ace,
#endif

#ifdef CONFIG_EXYNOS_C2C
	&exynos_device_c2c,
#endif

#ifdef CONFIG_S3C64XX_DEV_SPI
	&exynos_device_spi1,
#endif

#ifdef CONFIG_BUSFREQ_OPP
	&exynos5_busfreq,
#endif

#if defined(CONFIG_MHL_SII9234)
	&s3c_device_i2c15,	/* MHL */
#endif

#ifdef CONFIG_30PIN_CONN
	&sec_device_connector,
#endif

#ifdef CONFIG_VIDEO_EXYNOS_ROTATOR
#ifdef CONFIG_S5P_SYSTEM_MMU
	&SYSMMU_PLATDEV(rot),
#endif
	&exynos_device_rotator,
#endif

};

#ifdef CONFIG_EXYNOS_SETUP_THERMAL
/* below temperature base on the celcius degree */
struct tmu_data exynos_tmu_data __initdata = {
	.ts = {
		.stop_throttle  = 82,
		.start_throttle = 85,
		.stop_warning  = 95,
		.start_warning = 103,
		.start_tripping = 110, /* temp to do tripping */
	},
	.efuse_value = 55,
	.slope = 0x10008802,
	.mode = 0,
};
#endif

#ifdef CONFIG_VIDEO_EXYNOS_HDMI_CEC
static struct s5p_platform_cec hdmi_cec_data __initdata = {

};
#endif

#if defined(CONFIG_CMA)
static void __init exynos_reserve_mem(void)
{
	static struct cma_region regions[] = {
#ifdef CONFIG_ANDROID_PMEM_MEMSIZE_PMEM
		{
			.name = "pmem",
			.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_ANDROID_PMEM_MEMSIZE_PMEM_GPU1
		{
			.name = "pmem_gpu1",
			.size = CONFIG_ANDROID_PMEM_MEMSIZE_PMEM_GPU1 * SZ_1K,
			.start = 0,
		},
#endif
		{
			.name = "ion",
			.size = 30 * SZ_1M,
			.start = 0
		},
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_GSC0
		{
			.name = "gsc0",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_GSC0 * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_GSC1
		{
			.name = "gsc1",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_GSC1 * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_GSC2
		{
			.name = "gsc2",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_GSC2 * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_GSC3
		{
			.name = "gsc3",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_GSC3 * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMD
		{
			.name = "fimd",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_FIMD * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_AUDIO_SAMSUNG_MEMSIZE_SRP
		{
			.name = "srp",
			.size = CONFIG_AUDIO_SAMSUNG_MEMSIZE_SRP * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_S5P_MFC
		{
			.name		= "fw",
			.size		= 2 << 20,
			{ .alignment	= 128 << 10 },
			.start		= 0x44000000,
		},
		{
			.name		= "b1",
			.size		= 64 << 20,
			.start		= 0x45000000,
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_TV
		{
			.name = "tv",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_TV * SZ_1K,
			.start = 0
		},
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_MEMSIZE_ROT
		{
			.name = "rot",
			.size = CONFIG_VIDEO_SAMSUNG_MEMSIZE_ROT * SZ_1K,
			.start = 0,
		},
#endif
#ifdef CONFIG_VIDEO_EXYNOS5_FIMC_IS
#ifdef CONFIG_VIDEOBUF2_CMA_PHYS
		{
			.name = "fimc_is",
			.size = CONFIG_VIDEO_EXYNOS_MEMSIZE_FIMC_IS * SZ_1K,
			{
				.alignment = 1 << 26,
			},
			.start = 0
		},
#endif
#endif
		{
			.size = 0
		},
	};
	static const char map[] __initconst =
#ifdef CONFIG_EXYNOS_C2C
		"samsung-c2c=c2c_shdmem;"
#endif
		"android_pmem.0=pmem;android_pmem.1=pmem_gpu1;"
		"s3cfb.0=fimd;"
#ifdef CONFIG_AUDIO_SAMSUNG_MEMSIZE_SRP
		"samsung-rp=srp;"
#endif
		"exynos-gsc.0=gsc0;exynos-gsc.1=gsc1;exynos-gsc.2=gsc2;exynos-gsc.3=gsc3;"
		"ion-exynos=ion,gsc0,gsc1,gsc2,gsc3,fimd,fw,b1;"
		"exynos-rot=rot;"
		"s5p-mfc-v6/f=fw;"
		"s5p-mfc-v6/a=b1;"
		"s5p-mixer=tv;"
		"exynos5-fimc-is=fimc_is;";

    s5p_cma_region_reserve(regions, NULL, 0, map);
}
#else /* !CONFIG_CMA */
static inline void exynos_reserve_mem(void)
{
}
#endif

#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
static void __init smdk5250_camera_gpio_cfg(void)
{
	/* CAM A port(b0010) : PCLK, VSYNC, HREF, CLK_OUT */
	s3c_gpio_cfgrange_nopull(EXYNOS5_GPH0(0), 4, S3C_GPIO_SFN(2));
	/* CAM A port(b0010) : DATA[0-7] */
	s3c_gpio_cfgrange_nopull(EXYNOS5_GPH1(0), 8, S3C_GPIO_SFN(2));
	/* CAM B port(b0010) : PCLK, BAY_RGB[0-6] */
	s3c_gpio_cfgrange_nopull(EXYNOS5_GPG0(0), 8, S3C_GPIO_SFN(2));
	/* CAM B port(b0010) : BAY_Vsync, BAY_RGB[7-13] */
	s3c_gpio_cfgrange_nopull(EXYNOS5_GPG1(0), 8, S3C_GPIO_SFN(2));
	/* CAM B port(b0010) : BAY_Hsync, BAY_MCLK */
	s3c_gpio_cfgrange_nopull(EXYNOS5_GPG2(0), 2, S3C_GPIO_SFN(2));
	/* This is externel interrupt for m5mo */
#ifdef CONFIG_VIDEO_M5MOLS
	s3c_gpio_cfgpin(EXYNOS5_GPX2(6), S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(EXYNOS5_GPX2(6), S3C_GPIO_PULL_NONE);
#endif
}
#endif

#if defined(CONFIG_VIDEO_EXYNOS_GSCALER) && defined(CONFIG_VIDEO_EXYNOS_FIMC_LITE)
#if defined(CONFIG_VIDEO_S5K4BA)
static struct exynos_gscaler_isp_info s5k4ba = {
	.board_info	= &s5k4ba_info,
	.cam_srclk_name	= "xxti",
	.clk_frequency  = 24000000UL,
	.bus_type	= GSC_ITU_601,
#ifdef CONFIG_ITU_A
	.cam_clk_name	= "sclk_cam0",
	.i2c_bus_num	= 4,
	.cam_port	= CAM_PORT_A, /* A-Port : 0, B-Port : 1 */
#endif
#ifdef CONFIG_ITU_B
	.cam_clk_name	= "sclk_cam1",
	.i2c_bus_num	= 5,
	.cam_port	= CAM_PORT_B, /* A-Port : 0, B-Port : 1 */
#endif
	.flags		= GSC_CLK_INV_VSYNC,
};
/* This is for platdata of fimc-lite */
static struct s3c_platform_camera flite_s5k4ba = {
	.type		= CAM_TYPE_MIPI,
	.use_isp	= true,
	.inv_pclk	= 1,
	.inv_vsync	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,
};
#endif
#if defined(CONFIG_VIDEO_M5MOLS)
static struct exynos_gscaler_isp_info m5mols = {
	.board_info	= &m5mols_board_info,
	.cam_srclk_name	= "xxti",
	.clk_frequency  = 24000000UL,
	.bus_type	= GSC_MIPI_CSI2,
#ifdef CONFIG_CSI_C
	.cam_clk_name	= "sclk_cam0",
	.i2c_bus_num	= 4,
	.cam_port	= CAM_PORT_A, /* A-Port : 0, B-Port : 1 */
#endif
#ifdef CONFIG_CSI_D
	.cam_clk_name	= "sclk_cam1",
	.i2c_bus_num	= 5,
	.cam_port	= CAM_PORT_B, /* A-Port : 0, B-Port : 1 */
#endif
	.flags		= GSC_CLK_INV_PCLK | GSC_CLK_INV_VSYNC,
	.csi_data_align = 32,
};
/* This is for platdata of fimc-lite */
static struct s3c_platform_camera flite_m5mo = {
	.type		= CAM_TYPE_MIPI,
	.use_isp	= true,
	.inv_pclk	= 1,
	.inv_vsync	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,
};
#endif

static void __set_gsc_camera_config(struct exynos_platform_gscaler *data,
					u32 active_index, u32 preview,
					u32 camcording, u32 max_cam)
{
	data->active_cam_index = active_index;
	data->cam_preview = preview;
	data->cam_camcording = camcording;
	data->num_clients = max_cam;
}

static void __set_flite_camera_config(struct exynos_platform_flite *data,
					u32 active_index, u32 max_cam)
{
	data->active_cam_index = active_index;
	data->num_clients = max_cam;
}

static void __init smdk5250_set_camera_platdata(void)
{
	int gsc_cam_index = 0;
	int flite0_cam_index = 0;
	int flite1_cam_index = 0;
#if defined(CONFIG_VIDEO_M5MOLS)
	exynos_gsc0_default_data.isp_info[gsc_cam_index++] = &m5mols;
#if defined(CONFIG_CSI_C)
	exynos_flite0_default_data.cam[flite0_cam_index++] = &flite_m5mo;
#endif
#if defined(CONFIG_CSI_D)
	exynos_flite1_default_data.cam[flite1_cam_index++] = &flite_m5mo;
#endif
#endif

#if defined(CONFIG_VIDEO_S5K4BA)
	exynos_gsc0_default_data.isp_info[gsc_cam_index++] = &s5k4ba;
#if defined(CONFIG_ITU_A)
	exynos_flite0_default_data.cam[flite0_cam_index++] = &flite_s5k4ba;
#endif
#if defined(CONFIG_ITU_B)
	exynos_flite1_default_data.cam[flite1_cam_index++] = &flite_s5k4ba;
#endif
#endif
	/* flite platdata register */
	__set_flite_camera_config(&exynos_flite0_default_data, 0, flite0_cam_index);
	__set_flite_camera_config(&exynos_flite1_default_data, 0, flite1_cam_index);

	/* gscaler platdata register */
	/* GSC-0 */
	__set_gsc_camera_config(&exynos_gsc0_default_data, 0, 1, 0, gsc_cam_index);

	/* GSC-1 */
	/* GSC-2 */
	/* GSC-3 */
}
#endif /* CONFIG_VIDEO_EXYNOS_GSCALER */

static void __init p10_map_io(void)
{
	s5p_init_io(NULL, 0, S5P_VA_CHIPID);
	s3c24xx_init_clocks(24000000);
	s3c24xx_init_uarts(smdk5250_uartcfgs, ARRAY_SIZE(smdk5250_uartcfgs));
	exynos_reserve_mem();
}

#ifdef CONFIG_S5P_SYSTEM_MMU
static void __init exynos_sysmmu_init(void)
{
#ifdef CONFIG_VIDEO_JPEG_V2X
	ASSIGN_SYSMMU_POWERDOMAIN(jpeg, &exynos5_device_pd[PD_GSCL].dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(jpeg).dev, &s5p_device_jpeg.dev);
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_S5P_MFC
	sysmmu_set_owner(&SYSMMU_PLATDEV(mfc_l).dev, &s5p_device_mfc.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(mfc_r).dev, &s5p_device_mfc.dev);
#endif
#ifdef CONFIG_VIDEO_EXYNOS_TV
	ASSIGN_SYSMMU_POWERDOMAIN(tv, &exynos5_device_pd[PD_DISP1].dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(tv).dev, &s5p_device_mixer.dev);

#endif
#ifdef CONFIG_VIDEO_EXYNOS_GSCALER
	ASSIGN_SYSMMU_POWERDOMAIN(gsc0, &exynos5_device_pd[PD_GSCL].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(gsc1, &exynos5_device_pd[PD_GSCL].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(gsc2, &exynos5_device_pd[PD_GSCL].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(gsc3, &exynos5_device_pd[PD_GSCL].dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(gsc0).dev, &exynos5_device_gsc0.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(gsc1).dev, &exynos5_device_gsc1.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(gsc2).dev, &exynos5_device_gsc2.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(gsc3).dev, &exynos5_device_gsc3.dev);
#endif
#ifdef CONFIG_VIDEO_EXYNOS_ROTATOR
	sysmmu_set_owner(&SYSMMU_PLATDEV(rot).dev, &exynos_device_rotator.dev);
#endif
#ifdef CONFIG_VIDEO_FIMG2D
	sysmmu_set_owner(&SYSMMU_PLATDEV(2d).dev, &s5p_device_fimg2d.dev);
#endif
#ifdef CONFIG_VIDEO_EXYNOS5_FIMC_IS
	/*
	ASSIGN_SYSMMU_POWERDOMAIN(is_isp, &exynos4_device_pd[PD_ISP].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(is_drc, &exynos4_device_pd[PD_ISP].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(is_fd, &exynos4_device_pd[PD_ISP].dev);
	ASSIGN_SYSMMU_POWERDOMAIN(is_cpu, &exynos4_device_pd[PD_ISP].dev)
	*/
	sysmmu_set_owner(&SYSMMU_PLATDEV(is_isp).dev, &exynos5_device_fimc_is.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(is_drc).dev, &exynos5_device_fimc_is.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(is_fd).dev, &exynos5_device_fimc_is.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(is_cpu).dev, &exynos5_device_fimc_is.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(is_odc).dev, &exynos5_device_fimc_is.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(is_sclrc).dev, &exynos5_device_fimc_is.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(is_sclrp).dev, &exynos5_device_fimc_is.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(is_dis0).dev, &exynos5_device_fimc_is.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(is_dis1).dev, &exynos5_device_fimc_is.dev);
	sysmmu_set_owner(&SYSMMU_PLATDEV(is_3dnr).dev, &exynos5_device_fimc_is.dev);
#endif
}
#else /* !CONFIG_S5P_SYSTEM_MMU */
static inline void exynos_sysmmu_init(void)
{
}
#endif

static void p10_power_off(void)
{
	printk(KERN_EMERG "%s: set PS_HOLD low\n", __func__);

	writel(readl(EXYNOS5_PS_HOLD_CONTROL) & 0xFFFFFEFF, EXYNOS5_PS_HOLD_CONTROL);
	printk(KERN_EMERG "%s: Should not reach here\n", __func__);
}

static void __init p10_machine_init(void)
{
	pm_power_off = p10_power_off;

#ifdef CONFIG_S3C64XX_DEV_SPI
	struct clk *sclk = NULL;
	struct clk *prnt = NULL;
	struct device *spi1_dev = &exynos_device_spi1.dev;
#endif

#ifdef CONFIG_EXYNOS4_DEV_DWMCI
	exynos_dwmci_set_platdata(&exynos_dwmci_pdata);
#endif

#ifdef CONFIG_S3C_DEV_HSMMC
	s3c_sdhci0_set_platdata(&smdk5250_hsmmc0_pdata);
#endif

#ifdef CONFIG_S3C_DEV_HSMMC1
	s3c_sdhci1_set_platdata(&smdk5250_hsmmc1_pdata);
#endif

#ifdef CONFIG_S3C_DEV_HSMMC2
	s3c_sdhci2_set_platdata(&smdk5250_hsmmc2_pdata);
#endif

#ifdef CONFIG_S3C_DEV_HSMMC3
	s3c_sdhci3_set_platdata(&smdk5250_hsmmc3_pdata);
#endif

#ifdef CONFIG_ION_EXYNOS
	exynos_ion_set_platdata();
#endif

#ifdef CONFIG_FB_S3C
	dev_set_name(&s5p_device_fimd1.dev, "s3cfb.1");
	clk_add_alias("lcd", "exynos5-fb.1", "lcd", &s5p_device_fimd1.dev);
	clk_add_alias("sclk_fimd", "exynos5-fb.1", "sclk_fimd",
			&s5p_device_fimd1.dev);
	s5p_fb_setname(1, "exynos5-fb");

	s5p_fimd1_set_platdata(&p10_lcd1_pdata);
#endif

#ifdef CONFIG_BACKLIGHT_PWM
	samsung_bl_set(&p10_bl_gpio_info, &p10_bl_data);
#endif

#ifdef CONFIG_USB_EHCI_S5P
	smdk5250_ehci_init();
#endif

#ifdef CONFIG_USB_OHCI_S5P
	smdk5250_ohci_init();
#endif

#ifdef CONFIG_EXYNOS_DEV_SS_UDC
	smdk5250_ss_udc_init();
#endif

#if defined(CONFIG_VIDEO_SAMSUNG_S5P_MFC)
	exynos4_mfc_setup_clock(&s5p_device_mfc.dev, 300 * MHZ);

	dev_set_name(&s5p_device_mfc.dev, "s3c-mfc");
	clk_add_alias("mfc", "s5p-mfc-v6", "mfc", &s5p_device_mfc.dev);
	s5p_mfc_setname(&s5p_device_mfc, "s5p-mfc-v6");
#endif

#ifdef CONFIG_FB_S3C
	s5p_device_fimd1.dev.parent = &exynos5_device_pd[PD_DISP1].dev;
#endif

#ifdef CONFIG_S5P_DP
	s5p_dp_set_platdata(&p10_dp_data);
#endif

#ifdef CONFIG_VIDEO_FIMG2D
	s5p_fimg2d_set_platdata(&fimg2d_data);
#endif

	exynos_sysmmu_init();

	exynos5_sleep_gpio_table_set = p10_config_sleep_gpio_table;

	s3c_i2c0_set_platdata(NULL);
	i2c_register_board_info(0, i2c_devs0, ARRAY_SIZE(i2c_devs0));

	s3c_i2c1_set_platdata(NULL);
	i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));

	universal_tsp_init();
	s3c_i2c3_set_platdata(NULL);
	i2c_register_board_info(3, i2c_devs3, ARRAY_SIZE(i2c_devs3));

	s3c_i2c5_set_platdata(NULL);
	i2c_register_board_info(5, i2c_devs5, ARRAY_SIZE(i2c_devs5));

#ifdef CONFIG_SND_SOC_WM8994
	s3c_i2c7_set_platdata(NULL);
	i2c_register_board_info(7, i2c_devs7, ARRAY_SIZE(i2c_devs7));
#endif

#if defined(CONFIG_MHL_SII9234)
	sii9234_init();
	i2c_register_board_info(15, i2c_devs15, ARRAY_SIZE(i2c_devs15));
#endif

	platform_device_register(&vbatt_device);

	platform_add_devices(p10_devices, ARRAY_SIZE(p10_devices));

#ifdef CONFIG_FB_S3C
#if defined(CONFIG_MACH_P10_DP_01)
	exynos4_fimd_setup_clock(&s5p_device_fimd1.dev, "sclk_fimd", "sclk_vpll",
			270 * MHZ);
#elif defined(CONFIG_MACH_P10_DP_00)

	exynos4_fimd_setup_clock(&s5p_device_fimd1.dev, "sclk_fimd", "sclk_vpll",
			87 * MHZ);
#endif
#endif

#ifdef CONFIG_VIDEO_EXYNOS_MIPI_CSIS
#if defined(CONFIG_EXYNOS_DEV_PD)
	s5p_device_mipi_csis0.dev.parent = &exynos5_device_pd[PD_GSCL].dev;
	s5p_device_mipi_csis1.dev.parent = &exynos5_device_pd[PD_GSCL].dev;
#endif
	s3c_set_platdata(&s5p_mipi_csis0_default_data,
			sizeof(s5p_mipi_csis0_default_data), &s5p_device_mipi_csis0);
	s3c_set_platdata(&s5p_mipi_csis1_default_data,
			sizeof(s5p_mipi_csis1_default_data), &s5p_device_mipi_csis1);
#endif

#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
#if defined(CONFIG_EXYNOS_DEV_PD)
	exynos_device_flite0.dev.parent = &exynos5_device_pd[PD_GSCL].dev;
	exynos_device_flite1.dev.parent = &exynos5_device_pd[PD_GSCL].dev;
#endif
	smdk5250_camera_gpio_cfg();
	smdk5250_set_camera_platdata();
	s3c_set_platdata(&exynos_flite0_default_data,
			sizeof(exynos_flite0_default_data), &exynos_device_flite0);
	s3c_set_platdata(&exynos_flite1_default_data,
			sizeof(exynos_flite1_default_data), &exynos_device_flite1);
#endif

#ifdef CONFIG_VIDEO_EXYNOS5_FIMC_IS
	dev_set_name(&exynos5_device_fimc_is.dev, "s5p-mipi-csis.0");
	clk_add_alias("gscl_wrap", "exynos5-fimc-is", "gscl_wrap", &exynos5_device_fimc_is.dev);
	clk_add_alias("sclk_gscl_wrap", "exynos5-fimc-is", "sclk_gscl_wrap", &exynos5_device_fimc_is.dev);
	dev_set_name(&exynos5_device_fimc_is.dev, "exynos5-fimc-is");

	dev_set_name(&exynos5_device_fimc_is.dev, "exynos-gsc.0");
	clk_add_alias("gscl", "exynos5-fimc-is", "gscl", &exynos5_device_fimc_is.dev);
	dev_set_name(&exynos5_device_fimc_is.dev, "exynos5-fimc-is");

	exynos5_fimc_is_set_platdata(NULL);

#endif
#ifdef CONFIG_EXYNOS_SETUP_THERMAL
	s5p_tmu_set_platdata(&exynos_tmu_data);
#endif
#ifdef CONFIG_VIDEO_EXYNOS_GSCALER
#if defined(CONFIG_EXYNOS_DEV_PD)
	exynos5_device_gsc0.dev.parent = &exynos5_device_pd[PD_GSCL].dev;
	exynos5_device_gsc1.dev.parent = &exynos5_device_pd[PD_GSCL].dev;
	exynos5_device_gsc2.dev.parent = &exynos5_device_pd[PD_GSCL].dev;
	exynos5_device_gsc3.dev.parent = &exynos5_device_pd[PD_GSCL].dev;
#endif
	s3c_set_platdata(&exynos_gsc0_default_data, sizeof(exynos_gsc0_default_data),
			&exynos5_device_gsc0);
	s3c_set_platdata(&exynos_gsc1_default_data, sizeof(exynos_gsc1_default_data),
			&exynos5_device_gsc1);
	s3c_set_platdata(&exynos_gsc2_default_data, sizeof(exynos_gsc2_default_data),
			&exynos5_device_gsc2);
	s3c_set_platdata(&exynos_gsc3_default_data, sizeof(exynos_gsc3_default_data),
			&exynos5_device_gsc3);
	/* Gscaler can use MPLL(266MHz) or VPLL(300MHz).
	In case of P10, Gscaler should use MPLL(266MHz) because FIMD uses VPLL(86MHz).
	So mout_aclk_300_gscl_mid selects mout_mpll_user and then
	mout_aclk_300_gscl_mid is set to 267MHz
	even though the clock name(dout_aclk_300_gscl) implies and requires around 300MHz
	*/
	exynos5_gsc_set_parent_clock("mout_aclk_300_gscl_mid", "mout_mpll_user");
	exynos5_gsc_set_parent_clock("mout_aclk_300_gscl", "mout_aclk_300_gscl_mid");
	exynos5_gsc_set_parent_clock("aclk_300_gscl", "dout_aclk_300_gscl");
	exynos5_gsc_set_clock_rate("dout_aclk_300_gscl", 267000000);
#endif

#ifdef CONFIG_EXYNOS_C2C
	exynos_c2c_set_platdata(&smdk5250_c2c_pdata);
#endif

#ifdef CONFIG_VIDEO_JPEG_V2X
#ifdef CONFIG_EXYNOS_DEV_PD
	s5p_device_jpeg.dev.parent = &exynos5_device_pd[PD_GSCL].dev;
#endif
	exynos5_jpeg_setup_clock(&s5p_device_jpeg.dev, 150000000);
#endif

#if defined(CONFIG_VIDEO_EXYNOS_TV) && defined(CONFIG_VIDEO_EXYNOS_HDMI)
	dev_set_name(&s5p_device_hdmi.dev, "exynos5-hdmi");
	clk_add_alias("hdmi", "s5p-hdmi", "hdmi", &s5p_device_hdmi.dev);
	clk_add_alias("hdmiphy", "s5p-hdmi", "hdmiphy", &s5p_device_hdmi.dev);

	s5p_tv_setup();

/* setup dependencies between TV devices */
	/* This will be added after power domain for exynos5 is developed */
	s5p_device_hdmi.dev.parent = &exynos5_device_pd[PD_DISP1].dev;
	s5p_device_mixer.dev.parent = &exynos5_device_pd[PD_DISP1].dev;

	s5p_i2c_hdmiphy_set_platdata(NULL);
#ifdef CONFIG_VIDEO_EXYNOS_HDMI_CEC
	s5p_hdmi_cec_set_platdata(&hdmi_cec_data);
#endif
#endif

#ifdef CONFIG_S3C64XX_DEV_SPI
	sclk = clk_get(spi1_dev, "dout_spi1");
	if (IS_ERR(sclk))
		dev_err(spi1_dev, "failed to get sclk for SPI-1\n");
	prnt = clk_get(spi1_dev, "mout_mpll_user");
	if (IS_ERR(prnt))
		dev_err(spi1_dev, "failed to get prnt\n");
	if (clk_set_parent(sclk, prnt))
		printk(KERN_ERR "Unable to set parent %s of clock %s.\n",
				prnt->name, sclk->name);

	clk_set_rate(sclk, 800 * 1000 * 1000);
	clk_put(sclk);
	clk_put(prnt);

	if (!gpio_request(GPIO_5M_SPI_CS, "SPI_CS1")) {
		gpio_direction_output(GPIO_5M_SPI_CS, 1);
		s3c_gpio_cfgpin(GPIO_5M_SPI_CS, S3C_GPIO_SFN(1));
		s3c_gpio_setpull(GPIO_5M_SPI_CS, S3C_GPIO_PULL_UP);
		exynos_spi_set_info(1, EXYNOS_SPI_SRCCLK_SCLK,
			ARRAY_SIZE(spi1_csi));
	}

	spi_register_board_info(spi1_board_info, ARRAY_SIZE(spi1_board_info));
#endif

#ifdef CONFIG_BUSFREQ_OPP
	dev_add(&busfreq, &exynos5_busfreq.dev);
	ppmu_init(&exynos_ppmu[PPMU_CPU], &exynos5_busfreq.dev);
	ppmu_init(&exynos_ppmu[PPMU_DDR_C], &exynos5_busfreq.dev);
	ppmu_init(&exynos_ppmu[PPMU_DDR_R1], &exynos5_busfreq.dev);
	ppmu_init(&exynos_ppmu[PPMU_DDR_L], &exynos5_busfreq.dev);
#endif
}

#ifdef CONFIG_EXYNOS_C2C
static void __init exynos_c2c_reserve(void)
{
	static struct cma_region regions[] = {
		{
			.name = "c2c_shdmem",
			.size = 64 * SZ_1M,
			{ .alignment	= 64 * SZ_1M },
			.start = C2C_SHAREDMEM_BASE
		}, {
			.size = 0,
		}
	};

	s5p_cma_region_reserve(regions, NULL, 0, map);
}
#endif

MACHINE_START(SMDK5250, "SMDK5250")
	.boot_params	= S5P_PA_SDRAM + 0x100,
	.init_irq	= exynos5_init_irq,
	.map_io		= p10_map_io,
	.init_machine	= p10_machine_init,
	.timer		= &exynos4_timer,
#ifdef CONFIG_EXYNOS_C2C
	.reserve	= &exynos_c2c_reserve,
#endif
MACHINE_END
