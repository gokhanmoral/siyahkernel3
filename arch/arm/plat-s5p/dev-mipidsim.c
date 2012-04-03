/* linux/arch/arm/plat-s5p/dev-mipidsim.c
 *
 * Copyright (c) 2011 Samsung Electronics
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/fb.h>

#include <mach/map.h>
#include <mach/irqs.h>
#include <mach/regs-clock.h>

#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/fb.h>

#include <plat/dsim.h>
#include <plat/mipi_dsi.h>

#if defined(CONFIG_LCD_MIPI_S6E8AB0)
static struct mipi_dsim_config dsim_info = {
	.e_interface			= DSIM_VIDEO,
	.e_pixel_format	= DSIM_24BPP_888,
	/* main frame fifo auto flush at VSYNC pulse */
	.auto_flush = false,
	.eot_disable = false,
	.auto_vertical_cnt = true,
	.hse = false,
	.hfp = false,
	.hbp = false,
	.hsa = false,

	.e_no_data_lane = DSIM_DATA_LANE_4,
	.e_byte_clk = DSIM_PLL_OUT_DIV8,
	.e_burst_mode = DSIM_BURST,

	.p = 2,
	.m =57,
	.s = 1,

	/* D-PHY PLL stable time spec :min = 200usec ~ max 400usec */
	.pll_stable_time = 500,

	.esc_clk = 20 * 1000000,	/* escape clk : 10MHz */

	/* stop state holding counter after bta change count 0 ~ 0xfff */
	.stop_holding_cnt = 0x0fff,
	.bta_timeout = 0xff,		/* bta timeout 0 ~ 0xff */
	.rx_timeout = 0xffff,		/* lp rx timeout 0 ~ 0xffff */

	.dsim_ddi_pd = &s6e8ab0_mipi_lcd_driver,
};

static struct mipi_dsim_lcd_config dsim_lcd_info = {
	.rgb_timing.left_margin = 0xa,
	.rgb_timing.right_margin = 0xa,
	.rgb_timing.upper_margin = 80,
	.rgb_timing.lower_margin =  48,
	.rgb_timing.hsync_len = 5,
	.rgb_timing.vsync_len = 32,
	.cpu_timing.cs_setup = 0,
	.cpu_timing.wr_setup = 1,
	.cpu_timing.wr_act = 0,
	.cpu_timing.wr_hold = 0,
	.lcd_size.width = 1280,
	.lcd_size.height = 800,
};
#elif defined (CONFIG_LCD_MIPI_S6E63M0)
static struct mipi_dsim_config dsim_info = {
	.e_interface			= DSIM_VIDEO,
	.e_pixel_format	= DSIM_24BPP_888,
	/* main frame fifo auto flush at VSYNC pulse */
	.auto_flush = false,
	.eot_disable = false,
	.auto_vertical_cnt = true,
	.hse = false,
	.hfp = false,
	.hbp = false,
	.hsa = false,

	.e_no_data_lane = DSIM_DATA_LANE_2,
	.e_byte_clk = DSIM_PLL_OUT_DIV8,
	.e_burst_mode = DSIM_NON_BURST_SYNC_PULSE,

	.p = 3,
	.m = 90,
	.s = 1,

	/* D-PHY PLL stable time spec :min = 200usec ~ max 400usec */
	.pll_stable_time = 500,

	.esc_clk = 10 * 1000000,	/* escape clk : 10MHz */

	/* stop state holding counter after bta change count 0 ~ 0xfff */
	.stop_holding_cnt = 0x0fff,
	.bta_timeout = 0xff,		/* bta timeout 0 ~ 0xff */
	.rx_timeout = 0xffff,		/* lp rx timeout 0 ~ 0xffff */

	.dsim_ddi_pd = &s6e63m0_mipi_lcd_driver,
};

static struct mipi_dsim_lcd_config dsim_lcd_info = {
	.rgb_timing.left_margin = 0x16,
	.rgb_timing.right_margin = 0x16,
	.rgb_timing.upper_margin = 0x28,
	.rgb_timing.lower_margin =  0x1,
	.rgb_timing.hsync_len = 0x2,
	.rgb_timing.vsync_len = 0x3,
	.cpu_timing.cs_setup = 0,
	.cpu_timing.wr_setup = 1,
	.cpu_timing.wr_act = 0,
	.cpu_timing.wr_hold = 0,
	.lcd_size.width = 480,
	.lcd_size.height = 800,
};
#elif defined (CONFIG_LCD_MIPI_TC358764)
static struct mipi_dsim_config dsim_info = {
	.e_interface			= DSIM_VIDEO,
	.e_pixel_format	= DSIM_24BPP_888,
	/* main frame fifo auto flush at VSYNC pulse */
	.auto_flush = false,
	.eot_disable = false,
	.auto_vertical_cnt = false,
	.hse = false,
	.hfp = false,
	.hbp = false,
	.hsa = false,

	.e_no_data_lane = DSIM_DATA_LANE_4,
	.e_byte_clk = DSIM_PLL_OUT_DIV8,
	.e_burst_mode = DSIM_BURST,

	.p = 3,
	.m = 115,
	.s = 1,

	/* D-PHY PLL stable time spec :min = 200usec ~ max 400usec */
	.pll_stable_time = 500,

	.esc_clk = 0.4 * 1000000,	/* escape clk : 10MHz */

	/* stop state holding counter after bta change count 0 ~ 0xfff */
	.stop_holding_cnt = 0x0f,
	.bta_timeout = 0xff,		/* bta timeout 0 ~ 0xff */
	.rx_timeout = 0xffff,		/* lp rx timeout 0 ~ 0xffff */

	.dsim_ddi_pd = &tc358764_mipi_lcd_driver,
};

static struct mipi_dsim_lcd_config dsim_lcd_info = {
	.rgb_timing.left_margin = 0x4,
	.rgb_timing.right_margin = 0x4,
	.rgb_timing.upper_margin = 0x4,
	.rgb_timing.lower_margin =  0x4,
	.rgb_timing.hsync_len = 0x4,
	.rgb_timing.vsync_len = 0x4,
	.cpu_timing.cs_setup = 0,
	.cpu_timing.wr_setup = 1,
	.cpu_timing.wr_act = 0,
	.cpu_timing.wr_hold = 0,
	.lcd_size.width = 1280,
	.lcd_size.height = 800,
};
#endif

static struct resource s5p_dsim_resource[] = {
	[0] = {
		.start = S5P_PA_DSIM0,
		.end   = S5P_PA_DSIM0 + SZ_64K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_MIPIDSI0,
		.end   = IRQ_MIPIDSI0,
		.flags = IORESOURCE_IRQ,
	},
};

static struct s5p_platform_mipi_dsim dsim_platform_data = {
	.clk_name		= "dsim0",
	.dsim_config		= &dsim_info,
	.dsim_lcd_config		= &dsim_lcd_info,

	.part_reset		= s5p_dsim_part_reset,
	.init_d_phy		= s5p_dsim_init_d_phy,
	.get_fb_frame_done	= NULL,
	.trigger		= NULL,

	/*
	 * the stable time of needing to write data on SFR
	 * when the mipi mode becomes LP mode.
	 */
	.delay_for_stabilization = 600,
};

struct platform_device s5p_device_mipi_dsim = {
	.name			= "s5p-mipi-dsim",
	.id			= -1,
	.num_resources		= ARRAY_SIZE(s5p_dsim_resource),
	.resource		= s5p_dsim_resource,
	.dev			= {
		.platform_data = (void *)&dsim_platform_data,
	},
};
