/*
 * Copyright (C) 2012 Samsung Electronics Co.Ltd
 * Authors:
 *	Eunchul Kim <chulspro.kim@samsung.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include "drmP.h"
#include "drm_backlight.h"
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/cma.h>
#include <plat/map-base.h>

#include "regs-fimc.h"
#include <drm/exynos_drm.h>
#include "exynos_drm_drv.h"
#include "exynos_drm_gem.h"
#include "exynos_drm_ipp.h"
#include "exynos_drm_fimc.h"

/*
 * FIMC is stand for Fully Interactive Mobile Camera and
 * supports image scaler/rotator and input/output DMA operations.
 * input DMA reads image data from the memory.
 * output DMA writes image data to memory.
 * FIMC supports image rotation and image effect functions.
 */

#define FIMC_MAX_DEVS	4
#define FIMC_MAX_SRC	2
#define FIMC_MAX_DST	32
#define FIMC_CLK_RATE	166750000
#define FIMC_WIDTH_ITU_709	1280

#define get_fimc_context(dev)	platform_get_drvdata(to_platform_device(dev))
#define get_ctx_from_ippdrv(ippdrv)	container_of(ippdrv,\
					struct fimc_context, ippdrv);
#define fimc_read(offset)		readl(ctx->regs + (offset));
#define fimc_write(cfg, offset)	writel(cfg, ctx->regs + (offset));

enum fimc_wb {
	FIMC_WB_NONE,
	FIMC_WB_A,
	FIMC_WB_B
};

/*
 * A structure of scaler.
 *
 * @range: narrow, wide.
 * @bypass: unused scaler path.
 * @up_h: horizontal scale up.
 * @up_v: vertical scale up.
 * @hratio: horizontal ratio.
 * @vratio: vertical ratio.
 */
struct fimc_scaler {
	bool	range;
	bool bypass;
	bool up_h;
	bool up_v;
	u32 hratio;
	u32 vratio;
};

/*
 * A structure of scaler capability.
 *
 * find user manual table 43-1.
 * @in_hori: scaler input horizontal size.
 * @bypass: scaler bypass mode.
 * @dst_h_wo_rot: target horizontal size without output rotation.
 * @dst_h_rot: target horizontal size with output rotation.
 * @rl_w_wo_rot: real width without input rotation.
 * @rl_h_rot: real height without output rotation.
 */
struct fimc_capability {
	/* scaler */
	u32	in_hori;
	u32	bypass;
	/* output rotator */
	u32	dst_h_wo_rot;
	u32	dst_h_rot;
	/* input rotator */
	u32	rl_w_wo_rot;
	u32	rl_h_rot;
};

/*
 * A structure of fimc context.
 *
 * @ippdrv: prepare initialization using ippdrv.
 * @regs_res: register resources.
 * @regs: memory mapped io registers.
 * @lock: locking of operations.
 * @sclk_fimc_clk: fimc source clock.
 * @fimc_clk: fimc clock.
 * @wb_clk: writeback a clock.
 * @wb_b_clk: writeback b clock.
 * @sc: scaler infomations.
 * @capa: scaler capability.
 * @odr: ordering of YUV.
 * @ver: fimc version.
 * @pol: porarity of writeback.
 * @id: fimc id.
 * @irq: irq number.
 * @suspended: qos operations.
 */
struct fimc_context {
	struct exynos_drm_ippdrv	ippdrv;
	struct resource	*regs_res;
	void __iomem	*regs;
	struct mutex	lock;
	struct clk	*sclk_fimc_clk;
	struct clk	*fimc_clk;
	struct clk	*wb_clk;
	struct clk	*wb_b_clk;
	struct fimc_scaler	sc;
	struct fimc_capability	*capa;
	enum exynos_drm_fimc_ver	ver;
	struct exynos_drm_fimc_pol	pol;
	int	id;
	int	irq;
	bool	suspended;
};

struct fimc_capability fimc51_capa[FIMC_MAX_DEVS] = {
	{
		.in_hori = 4224,
		.bypass = 8192,
		.dst_h_wo_rot = 4224,
		.dst_h_rot = 1920,
		.rl_w_wo_rot = 8192,
		.rl_h_rot = 1920,
	}, {
		.in_hori = 4224,
		.bypass = 8192,
		.dst_h_wo_rot = 4224,
		.dst_h_rot = 1920,
		.rl_w_wo_rot = 8192,
		.rl_h_rot = 1920,
	}, {
		.in_hori = 4224,
		.bypass = 8192,
		.dst_h_wo_rot = 4224,
		.dst_h_rot = 1920,
		.rl_w_wo_rot = 8192,
		.rl_h_rot = 1920,
	}, {
		.in_hori = 1920,
		.bypass = 8192,
		.dst_h_wo_rot = 1920,
		.dst_h_rot = 1366,
		.rl_w_wo_rot = 8192,
		.rl_h_rot = 1366,
	},
};

static void fimc_sw_reset(struct fimc_context *ctx, bool pattern)
{
	u32 cfg;

	DRM_DEBUG_KMS("%s:pattern[%d]\n", __func__, pattern);

	cfg = fimc_read(EXYNOS_CISRCFMT);
	cfg |= EXYNOS_CISRCFMT_ITU601_8BIT;
	if (pattern)
		cfg |= EXYNOS_CIGCTRL_TESTPATTERN_COLOR_BAR;

	fimc_write(cfg, EXYNOS_CISRCFMT);

	/* s/w reset */
	cfg = fimc_read(EXYNOS_CIGCTRL);
	cfg |= (EXYNOS_CIGCTRL_SWRST);
	fimc_write(cfg, EXYNOS_CIGCTRL);

	/* s/w reset complete */
	cfg = fimc_read(EXYNOS_CIGCTRL);
	cfg &= ~EXYNOS_CIGCTRL_SWRST;
	fimc_write(cfg, EXYNOS_CIGCTRL);

	/* reset sequence */
	fimc_write(0x0, EXYNOS_CIFCNTSEQ);
}

static void fimc_set_camblk_fimd0_wb(struct fimc_context *ctx)
{
	u32 camblk_cfg;

	DRM_DEBUG_KMS("%s\n", __func__);

	camblk_cfg = readl(SYSREG_CAMERA_BLK);
	camblk_cfg &= ~(SYSREG_FIMD0WB_DEST_MASK);
	camblk_cfg |= ctx->id << (SYSREG_FIMD0WB_DEST_SHIFT);

	writel(camblk_cfg, SYSREG_CAMERA_BLK);
}

static void fimc_set_type_ctrl(struct fimc_context *ctx, enum fimc_wb wb)
{
	u32 cfg;

	DRM_DEBUG_KMS("%s:wb[%d]\n", __func__, wb);

	cfg = fimc_read(EXYNOS_CIGCTRL);
	cfg &= ~(EXYNOS_CIGCTRL_TESTPATTERN_MASK |
		EXYNOS_CIGCTRL_SELCAM_ITU_MASK |
		EXYNOS_CIGCTRL_SELCAM_MIPI_MASK |
		EXYNOS_CIGCTRL_SELCAM_FIMC_MASK |
		EXYNOS_CIGCTRL_SELWB_CAMIF_MASK |
		EXYNOS_CIGCTRL_SELWRITEBACK_MASK);

	switch (wb) {
	case FIMC_WB_A:
		cfg |= (EXYNOS_CIGCTRL_SELWRITEBACK_A |
			EXYNOS_CIGCTRL_SELWB_CAMIF_WRITEBACK);
		break;
	case FIMC_WB_B:
		cfg |= (EXYNOS_CIGCTRL_SELWRITEBACK_B |
			EXYNOS_CIGCTRL_SELWB_CAMIF_WRITEBACK);
		break;
	case FIMC_WB_NONE:
	default:
		cfg |= (EXYNOS_CIGCTRL_SELCAM_ITU_A |
			EXYNOS_CIGCTRL_SELWRITEBACK_A |
			EXYNOS_CIGCTRL_SELCAM_MIPI_A |
			EXYNOS_CIGCTRL_SELCAM_FIMC_ITU);
		break;
	}

	fimc_write(cfg, EXYNOS_CIGCTRL);
}

static void fimc_set_polarity(struct fimc_context *ctx,
	struct exynos_drm_fimc_pol *pol)
{
	u32 cfg;

	DRM_DEBUG_KMS("%s:inv_pclk[%d]inv_vsync[%d]\n",
		__func__, pol->inv_pclk, pol->inv_vsync);
	DRM_DEBUG_KMS("%s:inv_href[%d]inv_hsync[%d]\n",
		__func__, pol->inv_href, pol->inv_hsync);

	cfg = fimc_read(EXYNOS_CIGCTRL);
	cfg &= ~(EXYNOS_CIGCTRL_INVPOLPCLK | EXYNOS_CIGCTRL_INVPOLVSYNC |
		 EXYNOS_CIGCTRL_INVPOLHREF | EXYNOS_CIGCTRL_INVPOLHSYNC);

	if (pol->inv_pclk)
		cfg |= EXYNOS_CIGCTRL_INVPOLPCLK;
	if (pol->inv_vsync)
		cfg |= EXYNOS_CIGCTRL_INVPOLVSYNC;
	if (pol->inv_href)
		cfg |= EXYNOS_CIGCTRL_INVPOLHREF;
	if (pol->inv_hsync)
		cfg |= EXYNOS_CIGCTRL_INVPOLHSYNC;

	fimc_write(cfg, EXYNOS_CIGCTRL);
}

static void fimc_handle_jpeg(struct fimc_context *ctx, bool enable)
{
	u32 cfg;

	DRM_DEBUG_KMS("%s:enable[%d]\n", __func__, enable);

	cfg = fimc_read(EXYNOS_CIGCTRL);
	if (enable)
		cfg |= EXYNOS_CIGCTRL_CAM_JPEG;
	else
		cfg &= ~EXYNOS_CIGCTRL_CAM_JPEG;

	fimc_write(cfg, EXYNOS_CIGCTRL);
}

static void fimc_handle_irq(struct fimc_context *ctx, bool enable,
	bool overflow, bool level)
{
	u32 cfg;

	DRM_DEBUG_KMS("%s:enable[%d]overflow[%d]level[%d]\n", __func__,
			enable, overflow, level);

	cfg = fimc_read(EXYNOS_CIGCTRL);
	if (enable) {
		cfg &= ~(EXYNOS_CIGCTRL_IRQ_OVFEN | EXYNOS_CIGCTRL_IRQ_LEVEL);
		cfg |= EXYNOS_CIGCTRL_IRQ_ENABLE;
		if (overflow)
			cfg |= EXYNOS_CIGCTRL_IRQ_OVFEN;
		if (level)
			cfg |= EXYNOS_CIGCTRL_IRQ_LEVEL;
	} else
		cfg &= ~(EXYNOS_CIGCTRL_IRQ_OVFEN | EXYNOS_CIGCTRL_IRQ_ENABLE);

	fimc_write(cfg, EXYNOS_CIGCTRL);
}

static void fimc_clear_irq(struct fimc_context *ctx)
{
	u32 cfg;

	DRM_DEBUG_KMS("%s\n", __func__);

	cfg = fimc_read(EXYNOS_CIGCTRL);
	cfg |= EXYNOS_CIGCTRL_IRQ_CLR;
	fimc_write(cfg, EXYNOS_CIGCTRL);
}

static bool fimc_check_ovf(struct fimc_context *ctx)
{
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	u32 cfg, status, flag;

	status = fimc_read(EXYNOS_CISTATUS);
	flag = EXYNOS_CISTATUS_OVFIY | EXYNOS_CISTATUS_OVFICB |
		EXYNOS_CISTATUS_OVFICR;

	if (status & flag) {
		cfg = fimc_read(EXYNOS_CIWDOFST);
		cfg |= (EXYNOS_CIWDOFST_CLROVFIY | EXYNOS_CIWDOFST_CLROVFICB |
			EXYNOS_CIWDOFST_CLROVFICR);

		fimc_write(cfg, EXYNOS_CIWDOFST);

		cfg = fimc_read(EXYNOS_CIWDOFST);
		cfg &= ~(EXYNOS_CIWDOFST_CLROVFIY | EXYNOS_CIWDOFST_CLROVFICB |
			EXYNOS_CIWDOFST_CLROVFICR);

		fimc_write(cfg, EXYNOS_CIWDOFST);

		dev_err(ippdrv->dev, "occured overflow at %d, status 0x%x.\n",
			ctx->id, status);
		return true;
	}

	return false;
}

static int fimc_get_pf(struct fimc_context *ctx, bool present)
{
	u32 ret, pf;

	DRM_DEBUG_KMS("%s:present[%d]\n", __func__, present);

	pf = fimc_read(EXYNOS_CISTATUS2);

	if (present)
		ret = EXYNOS_CISTATUS2_GET_FRAMECOUNT_PRESENT(pf);
	else
		ret = EXYNOS_CISTATUS2_GET_FRAMECOUNT_BEFORE(pf);

	return ret;
}

static void fimc_handle_lastend(struct fimc_context *ctx, bool enable)
{
	u32 cfg;

	DRM_DEBUG_KMS("%s:enable[%d]\n", __func__, enable);

	cfg = fimc_read(EXYNOS_CIOCTRL);
	if (enable)
		cfg |= EXYNOS_CIOCTRL_LASTENDEN;
	else
		cfg &= ~EXYNOS_CIOCTRL_LASTENDEN;

	fimc_write(cfg, EXYNOS_CIOCTRL);
}

static int fimc_src_set_fmt_order(struct fimc_context *ctx, u32 fmt)
{
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	u32 cfg;

	DRM_DEBUG_KMS("%s:fmt[0x%x]\n", __func__, fmt);

	/* RGB */
	cfg = fimc_read(EXYNOS_CISCCTRL);
	cfg &= ~EXYNOS_CISCCTRL_INRGB_FMT_RGB_MASK;

	switch (fmt) {
	case DRM_FORMAT_RGB565:
		cfg |= EXYNOS_CISCCTRL_INRGB_FMT_RGB565;
		fimc_write(cfg, EXYNOS_CISCCTRL);
		return 0;
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_XRGB8888:
		cfg |= EXYNOS_CISCCTRL_INRGB_FMT_RGB888;
		fimc_write(cfg, EXYNOS_CISCCTRL);
		return 0;
	default:
		/* bypass */
		break;
	}

	/* YUV */
	cfg = fimc_read(EXYNOS_MSCTRL);
	cfg &= ~(EXYNOS_MSCTRL_ORDER2P_SHIFT_MASK |
		EXYNOS_MSCTRL_C_INT_IN_2PLANE |
		EXYNOS_MSCTRL_ORDER422_YCBYCR);

	switch (fmt) {
	case DRM_FORMAT_YUYV:
		cfg |= EXYNOS_MSCTRL_ORDER422_YCBYCR;
		break;
	case DRM_FORMAT_YVYU:
		cfg |= EXYNOS_MSCTRL_ORDER422_YCRYCB;
		break;
	case DRM_FORMAT_UYVY:
		cfg |= EXYNOS_MSCTRL_ORDER422_CBYCRY;
		break;
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YUV444:
		cfg |= EXYNOS_MSCTRL_ORDER422_CRYCBY;
		break;
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV61:
		cfg |= (EXYNOS_MSCTRL_ORDER2P_LSB_CRCB |
			EXYNOS_MSCTRL_C_INT_IN_2PLANE);
		break;
	case DRM_FORMAT_YUV422:
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		cfg |= EXYNOS_MSCTRL_C_INT_IN_3PLANE;
		break;
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV12M:
	case DRM_FORMAT_NV16:
		cfg |= (EXYNOS_MSCTRL_ORDER2P_LSB_CBCR |
			EXYNOS_MSCTRL_C_INT_IN_2PLANE);
		break;
	default:
		dev_err(ippdrv->dev, "inavlid source yuv order 0x%x.\n", fmt);
		return -EINVAL;
	}

	fimc_write(cfg, EXYNOS_MSCTRL);

	return 0;
}

static int fimc_src_set_fmt(struct device *dev, u32 fmt)
{
	struct fimc_context *ctx = get_fimc_context(dev);
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	u32 cfg;

	DRM_DEBUG_KMS("%s:fmt[0x%x]\n", __func__, fmt);

	cfg = fimc_read(EXYNOS_MSCTRL);
	cfg &= ~EXYNOS_MSCTRL_INFORMAT_RGB;

	switch (fmt) {
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_XRGB8888:
		cfg |= EXYNOS_MSCTRL_INFORMAT_RGB;
		break;
	case DRM_FORMAT_YUV444:
		cfg |= EXYNOS_MSCTRL_INFORMAT_YCBCR420;
		break;
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_YVYU:
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
		cfg |= EXYNOS_MSCTRL_INFORMAT_YCBCR422_1PLANE;
		break;
	case DRM_FORMAT_NV16:
	case DRM_FORMAT_NV61:
	case DRM_FORMAT_YUV422:
		cfg |= EXYNOS_MSCTRL_INFORMAT_YCBCR422;
		break;
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV12M:
	case DRM_FORMAT_NV21:
		cfg |= EXYNOS_MSCTRL_INFORMAT_YCBCR420;
		break;
	default:
		dev_err(ippdrv->dev, "inavlid source format 0x%x.\n", fmt);
		return -EINVAL;
	}

	fimc_write(cfg, EXYNOS_MSCTRL);

	cfg = fimc_read(EXYNOS_CIDMAPARAM);
	cfg &= ~EXYNOS_CIDMAPARAM_R_MODE_MASK;

	if (fmt == DRM_FORMAT_NV12M)
		cfg |= EXYNOS_CIDMAPARAM_R_MODE_64X32;
	else
		cfg |= EXYNOS_CIDMAPARAM_R_MODE_LINEAR;

	fimc_write(cfg, EXYNOS_CIDMAPARAM);

	return fimc_src_set_fmt_order(ctx, fmt);
}

static int fimc_src_set_transf(struct device *dev,
		enum drm_exynos_degree degree,
		enum drm_exynos_flip flip)
{
	struct fimc_context *ctx = get_fimc_context(dev);
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	u32 cfg;

	DRM_DEBUG_KMS("%s:degree[%d]flip[0x%x]\n", __func__,
		degree, flip);

	cfg = fimc_read(EXYNOS_MSCTRL);
	cfg &= ~(EXYNOS_MSCTRL_FLIP_X_MIRROR |
		EXYNOS_MSCTRL_FLIP_Y_MIRROR);
	cfg &= ~EXYNOS_CITRGFMT_INROT90_CLOCKWISE;

	switch (degree) {
	case EXYNOS_DRM_DEGREE_0:
		if (flip & EXYNOS_DRM_FLIP_HORIZONTAL)
			cfg |= EXYNOS_MSCTRL_FLIP_X_MIRROR;
		if (flip & EXYNOS_DRM_FLIP_VERTICAL)
			cfg |= EXYNOS_MSCTRL_FLIP_Y_MIRROR;
		break;
	case EXYNOS_DRM_DEGREE_90:
		cfg |= EXYNOS_CITRGFMT_INROT90_CLOCKWISE;
		if (flip & EXYNOS_DRM_FLIP_HORIZONTAL)
			cfg |= EXYNOS_MSCTRL_FLIP_X_MIRROR;
		if (flip & EXYNOS_DRM_FLIP_VERTICAL)
			cfg |= EXYNOS_MSCTRL_FLIP_Y_MIRROR;
		break;
	case EXYNOS_DRM_DEGREE_180:
		cfg |= (EXYNOS_MSCTRL_FLIP_X_MIRROR |
			EXYNOS_MSCTRL_FLIP_Y_MIRROR);
		if (flip & EXYNOS_DRM_FLIP_HORIZONTAL)
			cfg &= ~EXYNOS_MSCTRL_FLIP_X_MIRROR;
		if (flip & EXYNOS_DRM_FLIP_VERTICAL)
			cfg &= ~EXYNOS_MSCTRL_FLIP_Y_MIRROR;
		break;
	case EXYNOS_DRM_DEGREE_270:
		cfg |= (EXYNOS_CITRGFMT_INROT90_CLOCKWISE |
			EXYNOS_MSCTRL_FLIP_X_MIRROR |
			EXYNOS_MSCTRL_FLIP_Y_MIRROR);
		if (flip & EXYNOS_DRM_FLIP_HORIZONTAL)
			cfg &= ~EXYNOS_MSCTRL_FLIP_X_MIRROR;
		if (flip & EXYNOS_DRM_FLIP_VERTICAL)
			cfg &= ~EXYNOS_MSCTRL_FLIP_Y_MIRROR;
		break;
	default:
		dev_err(ippdrv->dev, "inavlid degree value %d.\n", degree);
		return -EINVAL;
	}

	fimc_write(cfg, EXYNOS_MSCTRL);

	return (cfg & EXYNOS_CITRGFMT_INROT90_CLOCKWISE) ? 1 : 0;
}

static int fimc_set_window(struct fimc_context *ctx,
	struct drm_exynos_pos *pos, struct drm_exynos_sz *sz)
{
	u32 cfg, h1, h2, v1, v2;

	/* cropped image */
	h1 = pos->x;
	h2 = sz->hsize - pos->w - pos->x;
	v1 = pos->y;
	v2 = sz->vsize - pos->h - pos->y;

	DRM_DEBUG_KMS("%s:x[%d]y[%d]w[%d]h[%d]hsize[%d]vsize[%d]\n",
	__func__, pos->x, pos->y, pos->w, pos->h, sz->hsize, sz->vsize);
	DRM_DEBUG_KMS("%s:h1[%d]h2[%d]v1[%d]v2[%d]\n", __func__,
		h1, h2, v1, v2);

	/*
	 * set window offset 1, 2 size
	 * check figure 43-21 in user manual
	 */
	cfg = fimc_read(EXYNOS_CIWDOFST);
	cfg &= ~(EXYNOS_CIWDOFST_WINHOROFST_MASK |
		EXYNOS_CIWDOFST_WINVEROFST_MASK);
	cfg |= (EXYNOS_CIWDOFST_WINHOROFST(h1) |
		EXYNOS_CIWDOFST_WINVEROFST(v1));
	cfg |= EXYNOS_CIWDOFST_WINOFSEN;
	fimc_write(cfg, EXYNOS_CIWDOFST);

	cfg = (EXYNOS_CIWDOFST2_WINHOROFST2(h2) |
		EXYNOS_CIWDOFST2_WINVEROFST2(v2));
	fimc_write(cfg, EXYNOS_CIWDOFST2);

	return 0;
}

static int fimc_src_set_size(struct device *dev, int swap,
	struct drm_exynos_pos *pos, struct drm_exynos_sz *sz)
{
	struct fimc_context *ctx = get_fimc_context(dev);
	u32 cfg;

	/* ToDo: check width and height */

	DRM_DEBUG_KMS("%s:swap[%d]hsize[%d]vsize[%d]\n",
		__func__, swap, sz->hsize, sz->vsize);

	/* original size */
	if (swap)
		cfg = EXYNOS_ORGISIZE_HORIZONTAL(sz->vsize) |
			EXYNOS_ORGISIZE_VERTICAL(sz->hsize);
	else
		cfg = EXYNOS_ORGISIZE_HORIZONTAL(sz->hsize) |
			EXYNOS_ORGISIZE_VERTICAL(sz->vsize);

	fimc_write(cfg, EXYNOS_ORGISIZE);

	DRM_DEBUG_KMS("%s:x[%d]y[%d]w[%d]h[%d]\n", __func__,
		pos->x, pos->y, pos->w, pos->h);

	/* set input DMA image size */
	cfg = fimc_read(EXYNOS_CIREAL_ISIZE);
	cfg &= ~(EXYNOS_CIREAL_ISIZE_HEIGHT_MASK |
		EXYNOS_CIREAL_ISIZE_WIDTH_MASK);
	cfg |= (EXYNOS_CIREAL_ISIZE_WIDTH(sz->hsize) |
		EXYNOS_CIREAL_ISIZE_HEIGHT(sz->vsize));
	fimc_write(cfg, EXYNOS_CIREAL_ISIZE);

	/*
	 * set input FIFO image size
	 * for now, we support only ITU601 8 bit mode
	 */
	cfg = (EXYNOS_CISRCFMT_ITU601_8BIT |
		EXYNOS_CISRCFMT_SOURCEHSIZE(sz->hsize) |
		EXYNOS_CISRCFMT_SOURCEVSIZE(sz->vsize));
	fimc_write(cfg, EXYNOS_CISRCFMT);

	return fimc_set_window(ctx, pos, sz);
}

static int fimc_src_set_addr(struct device *dev, dma_addr_t *base,
		u32 id, enum drm_exynos_ipp_buf_ctrl buf_ctrl)
{
	struct fimc_context *ctx = get_fimc_context(dev);
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	struct drm_exynos_ipp_config *config =
		&ippdrv->property.config[EXYNOS_DRM_OPS_SRC];

	DRM_DEBUG_KMS("%s:id[%d]buf_ctrl[%d]\n", __func__, id, buf_ctrl);

	if (id > FIMC_MAX_SRC) {
		dev_err(ippdrv->dev, "inavlid id value %d.\n", id);
		return -EINVAL;
	}

	/* address register set */
	switch (buf_ctrl) {
	case IPP_BUF_CTRL_MAP:
	case IPP_BUF_CTRL_UNMAP:
		fimc_write(base[EXYNOS_DRM_PLANAR_Y], EXYNOS_CIIYSA(id));

		if (config->fmt == DRM_FORMAT_YVU420) {
			fimc_write(base[EXYNOS_DRM_PLANAR_CR],
				EXYNOS_CIICRSA(id));
			fimc_write(base[EXYNOS_DRM_PLANAR_CB],
				EXYNOS_CIICBSA(id));
		} else {
			fimc_write(base[EXYNOS_DRM_PLANAR_CB],
				EXYNOS_CIICBSA(id));
			fimc_write(base[EXYNOS_DRM_PLANAR_CR],
				EXYNOS_CIICRSA(id));
		}
	default:
		/* bypass */
		break;
	}

	return 0;
}

static struct exynos_drm_ipp_ops fimc_src_ops = {
	.set_fmt = fimc_src_set_fmt,
	.set_transf = fimc_src_set_transf,
	.set_size = fimc_src_set_size,
	.set_addr = fimc_src_set_addr,
};

static int fimc_dst_set_fmt_order(struct fimc_context *ctx, u32 fmt)
{
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	u32 cfg;

	DRM_DEBUG_KMS("%s:fmt[0x%x]\n", __func__, fmt);

	/* RGB */
	cfg = fimc_read(EXYNOS_CISCCTRL);
	cfg &= ~EXYNOS_CISCCTRL_OUTRGB_FMT_RGB_MASK;

	switch (fmt) {
	case DRM_FORMAT_RGB565:
		cfg |= EXYNOS_CISCCTRL_OUTRGB_FMT_RGB565;
		fimc_write(cfg, EXYNOS_CISCCTRL);
		return 0;
	case DRM_FORMAT_RGB888:
		cfg |= EXYNOS_CISCCTRL_OUTRGB_FMT_RGB888;
		fimc_write(cfg, EXYNOS_CISCCTRL);
		return 0;
	case DRM_FORMAT_XRGB8888:
		cfg |= (EXYNOS_CISCCTRL_OUTRGB_FMT_RGB888 |
			EXYNOS_CISCCTRL_EXTRGB_EXTENSION);
		fimc_write(cfg, EXYNOS_CISCCTRL);
		break;
	default:
		/* bypass */
		break;
	}

	/* YUV */
	cfg = fimc_read(EXYNOS_CIOCTRL);
	cfg &= ~(EXYNOS_CIOCTRL_ORDER2P_MASK |
		EXYNOS_CIOCTRL_ORDER422_MASK |
		EXYNOS_CIOCTRL_YCBCR_PLANE_MASK);

	switch (fmt) {
	case DRM_FORMAT_XRGB8888:
		cfg |= EXYNOS_CIOCTRL_ALPHA_OUT;
		break;
	case DRM_FORMAT_YUYV:
		cfg |= EXYNOS_CIOCTRL_ORDER422_YCBYCR;
		break;
	case DRM_FORMAT_YVYU:
		cfg |= EXYNOS_CIOCTRL_ORDER422_YCRYCB;
		break;
	case DRM_FORMAT_UYVY:
		cfg |= EXYNOS_CIOCTRL_ORDER422_CBYCRY;
		break;
	case DRM_FORMAT_VYUY:
		cfg |= EXYNOS_CIOCTRL_ORDER422_CRYCBY;
		break;
	case DRM_FORMAT_NV21:
	case DRM_FORMAT_NV61:
		cfg |= EXYNOS_CIOCTRL_ORDER2P_LSB_CRCB;
		cfg |= EXYNOS_CIOCTRL_YCBCR_2PLANE;
		break;
	case DRM_FORMAT_YUV422:
	case DRM_FORMAT_YUV420:
	case DRM_FORMAT_YVU420:
		cfg |= EXYNOS_CIOCTRL_YCBCR_3PLANE;
		break;
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV12M:
	case DRM_FORMAT_NV16:
		cfg |= EXYNOS_CIOCTRL_ORDER2P_LSB_CBCR;
		cfg |= EXYNOS_CIOCTRL_YCBCR_2PLANE;
		break;
	default:
		dev_err(ippdrv->dev, "inavlid target yuv order 0x%x.\n", fmt);
		return -EINVAL;
	}

	fimc_write(cfg, EXYNOS_CIOCTRL);

	return 0;
}

static int fimc_dst_set_fmt(struct device *dev, u32 fmt)
{
	struct fimc_context *ctx = get_fimc_context(dev);
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	u32 cfg;

	DRM_DEBUG_KMS("%s:fmt[0x%x]\n", __func__, fmt);

	cfg = fimc_read(EXYNOS_CIEXTEN);

	if (fmt == DRM_FORMAT_AYUV) {
		cfg |= EXYNOS_CIEXTEN_YUV444_OUT;
		fimc_write(cfg, EXYNOS_CIEXTEN);
	} else {
		cfg &= ~EXYNOS_CIEXTEN_YUV444_OUT;
		fimc_write(cfg, EXYNOS_CIEXTEN);

		cfg = fimc_read(EXYNOS_CITRGFMT);
		cfg &= ~EXYNOS_CITRGFMT_OUTFORMAT_MASK;

		switch (fmt) {
		case DRM_FORMAT_RGB565:
		case DRM_FORMAT_RGB888:
		case DRM_FORMAT_XRGB8888:
			cfg |= EXYNOS_CITRGFMT_OUTFORMAT_RGB;
			break;
		case DRM_FORMAT_YUYV:
		case DRM_FORMAT_YVYU:
		case DRM_FORMAT_UYVY:
		case DRM_FORMAT_VYUY:
			cfg |= EXYNOS_CITRGFMT_OUTFORMAT_YCBCR422_1PLANE;
			break;
		case DRM_FORMAT_NV16:
		case DRM_FORMAT_NV61:
		case DRM_FORMAT_YUV422:
			cfg |= EXYNOS_CITRGFMT_OUTFORMAT_YCBCR422;
			break;
		case DRM_FORMAT_YUV420:
		case DRM_FORMAT_YVU420:
		case DRM_FORMAT_NV12:
		case DRM_FORMAT_NV12M:
		case DRM_FORMAT_NV21:
			cfg |= EXYNOS_CITRGFMT_OUTFORMAT_YCBCR420;
			break;
		default:
			dev_err(ippdrv->dev, "inavlid target format 0x%x.\n",
				fmt);
			return -EINVAL;
		}

		fimc_write(cfg, EXYNOS_CITRGFMT);
	}

	cfg = fimc_read(EXYNOS_CIDMAPARAM);
	cfg &= ~EXYNOS_CIDMAPARAM_W_MODE_MASK;

	if (fmt == DRM_FORMAT_NV12M)
		cfg |= EXYNOS_CIDMAPARAM_W_MODE_64X32;
	else
		cfg |= EXYNOS_CIDMAPARAM_W_MODE_LINEAR;

	fimc_write(cfg, EXYNOS_CIDMAPARAM);

	return fimc_dst_set_fmt_order(ctx, fmt);
}

static int fimc_dst_set_transf(struct device *dev,
		enum drm_exynos_degree degree,
		enum drm_exynos_flip flip)
{
	struct fimc_context *ctx = get_fimc_context(dev);
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	u32 cfg;

	DRM_DEBUG_KMS("%s:degree[%d]flip[0x%x]\n", __func__,
		degree, flip);

	cfg = fimc_read(EXYNOS_CITRGFMT);
	cfg &= ~EXYNOS_CITRGFMT_FLIP_MASK;
	cfg &= ~EXYNOS_CITRGFMT_OUTROT90_CLOCKWISE;

	switch (degree) {
	case EXYNOS_DRM_DEGREE_0:
		if (flip & EXYNOS_DRM_FLIP_HORIZONTAL)
			cfg |= EXYNOS_CITRGFMT_FLIP_X_MIRROR;
		if (flip & EXYNOS_DRM_FLIP_VERTICAL)
			cfg |= EXYNOS_CITRGFMT_FLIP_Y_MIRROR;
		break;
	case EXYNOS_DRM_DEGREE_90:
		cfg |= EXYNOS_CITRGFMT_OUTROT90_CLOCKWISE;
		if (flip & EXYNOS_DRM_FLIP_HORIZONTAL)
			cfg |= EXYNOS_CITRGFMT_FLIP_X_MIRROR;
		if (flip & EXYNOS_DRM_FLIP_VERTICAL)
			cfg |= EXYNOS_CITRGFMT_FLIP_Y_MIRROR;
		break;
	case EXYNOS_DRM_DEGREE_180:
		cfg |= (EXYNOS_CITRGFMT_FLIP_X_MIRROR |
			EXYNOS_CITRGFMT_FLIP_Y_MIRROR);
		if (flip & EXYNOS_DRM_FLIP_HORIZONTAL)
			cfg &= ~EXYNOS_CITRGFMT_FLIP_X_MIRROR;
		if (flip & EXYNOS_DRM_FLIP_VERTICAL)
			cfg &= ~EXYNOS_CITRGFMT_FLIP_Y_MIRROR;
		break;
	case EXYNOS_DRM_DEGREE_270:
		cfg |= (EXYNOS_CITRGFMT_OUTROT90_CLOCKWISE |
			EXYNOS_CITRGFMT_FLIP_X_MIRROR |
			EXYNOS_CITRGFMT_FLIP_Y_MIRROR);
		if (flip & EXYNOS_DRM_FLIP_HORIZONTAL)
			cfg &= ~EXYNOS_CITRGFMT_FLIP_X_MIRROR;
		if (flip & EXYNOS_DRM_FLIP_VERTICAL)
			cfg &= ~EXYNOS_CITRGFMT_FLIP_Y_MIRROR;
		break;
	default:
		dev_err(ippdrv->dev, "inavlid degree value %d.\n", degree);
		return -EINVAL;
	}

	fimc_write(cfg, EXYNOS_CITRGFMT);

	return (cfg & EXYNOS_CITRGFMT_OUTROT90_CLOCKWISE) ? 1 : 0;
}

static int fimc_get_ratio_shift(u32 src, u32 dst, u32 *ratio, u32 *shift)
{
	DRM_DEBUG_KMS("%s:src[%d]dst[%d]\n", __func__, src, dst);

	if (src >= dst * 64) {
		DRM_ERROR("failed to make ratio and shift.\n");
		return -EINVAL;
	} else if (src >= dst * 32) {
		*ratio = 32;
		*shift = 5;
	} else if (src >= dst * 16) {
		*ratio = 16;
		*shift = 4;
	} else if (src >= dst * 8) {
		*ratio = 8;
		*shift = 3;
	} else if (src >= dst * 4) {
		*ratio = 4;
		*shift = 2;
	} else if (src >= dst * 2) {
		*ratio = 2;
		*shift = 1;
	} else {
		*ratio = 1;
		*shift = 0;
	}

	return 0;
}

static int fimc_set_prescaler(struct fimc_context *ctx, struct fimc_scaler *sc,
		struct drm_exynos_pos *src, struct drm_exynos_pos *dst)
{
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	u32 cfg, shfactor;
	u32 pre_dst_width, pre_dst_height;
	u32 pre_hratio, hfactor, pre_vratio, vfactor;
	int ret = 0;

	ret = fimc_get_ratio_shift(src->w, dst->w, &pre_hratio, &hfactor);
	if (ret) {
		dev_err(ippdrv->dev, "failed to get ratio horizontal.\n");
		return ret;
	}

	ret = fimc_get_ratio_shift(src->h, dst->h, &pre_vratio, &vfactor);
	if (ret) {
		dev_err(ippdrv->dev, "failed to get ratio vertical.\n");
		return ret;
	}

	pre_dst_width = src->w / pre_hratio;
	pre_dst_height = src->h / pre_vratio;
	DRM_DEBUG_KMS("%s:pre_dst_width[%d]pre_dst_height[%d]\n", __func__,
		pre_dst_width, pre_dst_height);
	DRM_DEBUG_KMS("%s:pre_hratio[%d]hfactor[%d]pre_vratio[%d]vfactor[%d]\n",
		__func__, pre_hratio, hfactor, pre_vratio, vfactor);

	sc->hratio = (src->w << 14) / (dst->w << hfactor);
	sc->vratio = (src->h << 14) / (dst->h << vfactor);
	sc->up_h = (dst->w >= src->w) ? true : false;
	sc->up_v = (dst->h >= src->h) ? true : false;
	DRM_DEBUG_KMS("%s:hratio[%d]vratio[%d]up_h[%d]up_v[%d]\n",
	__func__, sc->hratio, sc->vratio, sc->up_h, sc->up_v);

	shfactor = 10 - (hfactor + vfactor);
	DRM_DEBUG_KMS("%s:shfactor[%d]\n", __func__, shfactor);

	cfg = (EXYNOS_CISCPRERATIO_SHFACTOR(shfactor) |
		EXYNOS_CISCPRERATIO_PREHORRATIO(pre_hratio) |
		EXYNOS_CISCPRERATIO_PREVERRATIO(pre_vratio));
	fimc_write(cfg, EXYNOS_CISCPRERATIO);

	cfg = (EXYNOS_CISCPREDST_PREDSTWIDTH(pre_dst_width) |
		EXYNOS_CISCPREDST_PREDSTHEIGHT(pre_dst_height));
	fimc_write(cfg, EXYNOS_CISCPREDST);

	return ret;
}

static void fimc_set_scaler(struct fimc_context *ctx, struct fimc_scaler *sc)
{
	u32 cfg, cfg_ext;

	DRM_DEBUG_KMS("%s:range[%d]bypass[%d]up_h[%d]up_v[%d]\n",
		__func__, sc->range, sc->bypass, sc->up_h, sc->up_v);
	DRM_DEBUG_KMS("%s:hratio[%d]vratio[%d]\n",
		__func__, sc->hratio, sc->vratio);

	cfg = fimc_read(EXYNOS_CISCCTRL);
	cfg &= ~(EXYNOS_CISCCTRL_SCALERBYPASS |
		EXYNOS_CISCCTRL_SCALEUP_H | EXYNOS_CISCCTRL_SCALEUP_V |
		EXYNOS_CISCCTRL_MAIN_V_RATIO_MASK |
		EXYNOS_CISCCTRL_MAIN_H_RATIO_MASK |
		EXYNOS_CISCCTRL_CSCR2Y_WIDE |
		EXYNOS_CISCCTRL_CSCY2R_WIDE);

	if (sc->range)
		cfg |= (EXYNOS_CISCCTRL_CSCR2Y_WIDE |
			EXYNOS_CISCCTRL_CSCY2R_WIDE);
	if (sc->bypass)
		cfg |= EXYNOS_CISCCTRL_SCALERBYPASS;
	if (sc->up_h)
		cfg |= EXYNOS_CISCCTRL_SCALEUP_H;
	if (sc->up_v)
		cfg |= EXYNOS_CISCCTRL_SCALEUP_V;

	cfg |= (EXYNOS_CISCCTRL_MAINHORRATIO((sc->hratio >> 6)) |
		EXYNOS_CISCCTRL_MAINVERRATIO((sc->vratio >> 6)));
	fimc_write(cfg, EXYNOS_CISCCTRL);

	cfg_ext = fimc_read(EXYNOS_CIEXTEN);
	cfg_ext &= ~EXYNOS_CIEXTEN_MAINHORRATIO_EXT_MASK;
	cfg_ext &= ~EXYNOS_CIEXTEN_MAINVERRATIO_EXT_MASK;
	cfg_ext |= (EXYNOS_CIEXTEN_MAINHORRATIO_EXT(sc->hratio) |
		EXYNOS_CIEXTEN_MAINVERRATIO_EXT(sc->vratio));
	fimc_write(cfg_ext, EXYNOS_CIEXTEN);
}

static int fimc_dst_set_size(struct device *dev, int swap,
	struct drm_exynos_pos *pos, struct drm_exynos_sz *sz)
{
	struct fimc_context *ctx = get_fimc_context(dev);
	u32 cfg, swap_w;

	DRM_DEBUG_KMS("%s:swap[%d]hsize[%d]vsize[%d]\n",
		__func__, swap, sz->hsize, sz->vsize);

	/* original size */
	if (swap) {
		cfg = (EXYNOS_ORGOSIZE_HORIZONTAL(sz->vsize) |
			EXYNOS_ORGOSIZE_VERTICAL(sz->hsize));
		swap_w = sz->vsize;
	} else {
		cfg = (EXYNOS_ORGOSIZE_HORIZONTAL(sz->hsize) |
			EXYNOS_ORGOSIZE_VERTICAL(sz->vsize));
		swap_w = sz->hsize;
	}

	fimc_write(cfg, EXYNOS_ORGOSIZE);

	DRM_DEBUG_KMS("%s:swap_w[%d]x[%d]y[%d]w[%d]h[%d]\n",
		__func__, swap_w, pos->x, pos->y, pos->w, pos->h);

	/* CSC ITU */
	cfg = fimc_read(EXYNOS_CIGCTRL);
	cfg &= ~EXYNOS_CIGCTRL_CSC_MASK;

	if (swap_w >= FIMC_WIDTH_ITU_709)
		cfg |= EXYNOS_CIGCTRL_CSC_ITU709;
	else
		cfg |= EXYNOS_CIGCTRL_CSC_ITU601;

	fimc_write(cfg, EXYNOS_CIGCTRL);

	/* target image size */
	cfg = fimc_read(EXYNOS_CITRGFMT);
	cfg &= ~(EXYNOS_CITRGFMT_TARGETH_MASK |
		EXYNOS_CITRGFMT_TARGETV_MASK);
	cfg |= (EXYNOS_CITRGFMT_TARGETHSIZE(pos->w) |
		EXYNOS_CITRGFMT_TARGETVSIZE(pos->h));
	fimc_write(cfg, EXYNOS_CITRGFMT);

	/* target area */
	cfg = EXYNOS_CITAREA_TARGET_AREA(pos->w * pos->h);
	fimc_write(cfg, EXYNOS_CITAREA);

	/* ToDo: Move Scaler in this line and YUV */

	/* offset Y(RGB), Cb, Cr */
	cfg = (EXYNOS_CIOYOFF_HORIZONTAL(pos->x) |
		EXYNOS_CIOYOFF_VERTICAL(pos->y));
	fimc_write(cfg, EXYNOS_CIOYOFF);
	cfg = (EXYNOS_CIOCBOFF_HORIZONTAL(pos->x) |
		EXYNOS_CIOCBOFF_VERTICAL(pos->y));
	fimc_write(cfg, EXYNOS_CIOCBOFF);
	cfg = (EXYNOS_CIOCROFF_HORIZONTAL(pos->x) |
		EXYNOS_CIOCROFF_VERTICAL(pos->y));
	fimc_write(cfg, EXYNOS_CIOCROFF);

	return 0;
}

static int fimc_dst_set_buf_seq(struct fimc_context *ctx, u32 shift,
	enum drm_exynos_ipp_buf_ctrl buf_ctrl)
{
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	bool enable;
	u32 cfg;
	u32 mask = 0x00000001 << shift;

	DRM_DEBUG_KMS("%s:shift[%d]buf_ctrl[%d]\n", __func__, shift, buf_ctrl);

	/* mask register set */
	cfg = fimc_read(EXYNOS_CIFCNTSEQ);

	switch (buf_ctrl) {
	case IPP_BUF_CTRL_MAP:
	case IPP_BUF_CTRL_QUEUE:
		enable = true;
		break;
	case IPP_BUF_CTRL_UNMAP:
	case IPP_BUF_CTRL_DEQUEUE:
		enable = false;
		break;
	default:
		dev_err(ippdrv->dev, "invalid buf ctrl parameter.\n");
		return -EINVAL;
	}

	/* sequence id */
	cfg &= (~mask);
	cfg |= (enable << shift);
	fimc_write(cfg, EXYNOS_CIFCNTSEQ);

	return 0;
}

static int fimc_dst_set_addr(struct device *dev, dma_addr_t *base,
			u32 id, enum drm_exynos_ipp_buf_ctrl buf_ctrl)
{
	struct fimc_context *ctx = get_fimc_context(dev);
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	struct drm_exynos_ipp_config *config =
		&ippdrv->property.config[EXYNOS_DRM_OPS_DST];

	DRM_DEBUG_KMS("%s:id[%d]buf_ctrl[%d]\n", __func__, id, buf_ctrl);

	if (id > FIMC_MAX_DST) {
		dev_err(ippdrv->dev, "inavlid id value %d.\n", id);
		return -EINVAL;
	}

	/* address register set */
	switch (buf_ctrl) {
	case IPP_BUF_CTRL_MAP:
	case IPP_BUF_CTRL_UNMAP:
		fimc_write(base[EXYNOS_DRM_PLANAR_Y], EXYNOS_CIOYSA(id));

		if (config->fmt == DRM_FORMAT_YVU420) {
			fimc_write(base[EXYNOS_DRM_PLANAR_CR],
				EXYNOS_CIOCBSA(id));
			fimc_write(base[EXYNOS_DRM_PLANAR_CB],
				EXYNOS_CIOCRSA(id));
		} else {
			fimc_write(base[EXYNOS_DRM_PLANAR_CB],
				EXYNOS_CIOCBSA(id));
			fimc_write(base[EXYNOS_DRM_PLANAR_CR],
				EXYNOS_CIOCRSA(id));
		}
		fimc_handle_irq(ctx, true, false, true);
	default:
		/* bypass */
		break;
	}

	return fimc_dst_set_buf_seq(ctx, id, buf_ctrl);
}

static struct exynos_drm_ipp_ops fimc_dst_ops = {
	.set_fmt = fimc_dst_set_fmt,
	.set_transf = fimc_dst_set_transf,
	.set_size = fimc_dst_set_size,
	.set_addr = fimc_dst_set_addr,
};

static int fimc_power_on(struct fimc_context *ctx, bool enable)
{
	DRM_DEBUG_KMS("%s:enable[%d]\n", __func__, enable);

	if (enable) {
		clk_enable(ctx->sclk_fimc_clk);
		clk_enable(ctx->fimc_clk);
		clk_enable(ctx->wb_clk);
		/* ToDo : wb_b_clk */
		ctx->suspended = false;
	} else {
		clk_disable(ctx->sclk_fimc_clk);
		clk_disable(ctx->fimc_clk);
		clk_disable(ctx->wb_clk);
		/* ToDo : wb_b_clk */
		ctx->suspended = true;
	}

	return 0;
}

static irqreturn_t fimc_irq_handler(int irq, void *dev_id)
{
	struct fimc_context *ctx = dev_id;
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	int pf, buf_idx;

	fimc_clear_irq(ctx);
	if (fimc_check_ovf(ctx))
		return IRQ_NONE;

	pf = fimc_get_pf(ctx, true);
	if (pf == 0)
		return IRQ_HANDLED;

	buf_idx = pf-1;
	DRM_DEBUG_KMS("%s:id[%d]buf_idx[%d]\n", __func__, ctx->id, buf_idx);

	if (list_empty(&ippdrv->event_list)) {
		DRM_DEBUG_KMS("%s:event list empty.\n", __func__);

		fimc_handle_irq(ctx, false, false, true);
		return IRQ_HANDLED;
	}

	/* ToDo: How can fix interrupt pending ? */
	ipp_send_event_handler(ippdrv, buf_idx);

	return IRQ_HANDLED;
}

static int fimc_ippdrv_reset(struct device *dev)
{
	struct fimc_context *ctx = get_fimc_context(dev);

	DRM_DEBUG_KMS("%s\n", __func__);

	/* reset h/w block */
	fimc_sw_reset(ctx, false);

	/* reset scaler capability */
	memset(&ctx->sc, 0x0, sizeof(ctx->sc));

	return 0;
}

static int fimc_ippdrv_start(struct device *dev, enum drm_exynos_ipp_cmd cmd)
{
	struct fimc_context *ctx = get_fimc_context(dev);
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	struct drm_exynos_ipp_config *config_src =
		&ippdrv->property.config[EXYNOS_DRM_OPS_SRC];
	struct drm_exynos_ipp_config *config_dst =
		&ippdrv->property.config[EXYNOS_DRM_OPS_DST];
	int ret;
	int enable = 1;
	u32 cfg0, cfg1;

	DRM_DEBUG_KMS("%s:cmd[%d]\n", __func__, cmd);

	ippdrv->cmd = cmd;

	/* ToDo: Merge WB and M2M */
	switch (cmd) {
	case IPP_CMD_M2M:
		fimc_set_type_ctrl(ctx, FIMC_WB_NONE);
		fimc_set_polarity(ctx, &ctx->pol);
		fimc_handle_lastend(ctx, false);

		/* ToDo: window size, prescaler config */

		ret = fimc_set_prescaler(ctx, &ctx->sc, &config_src->pos,
			&config_dst->pos);
		if (ret) {
			dev_err(dev, "failed to set precalser.\n");
			return ret;
		}

		/* If set ture, we can save jpeg about screen */
		fimc_handle_jpeg(ctx, false);
		fimc_set_scaler(ctx, &ctx->sc);

		/* setup dma */
		cfg0 = fimc_read(EXYNOS_MSCTRL);
		cfg0 &= ~EXYNOS_MSCTRL_INPUT_MASK;
		cfg0 |= EXYNOS_MSCTRL_INPUT_MEMORY;
		fimc_write(cfg0, EXYNOS_MSCTRL);
		break;
	case IPP_CMD_WB:
		fimc_set_camblk_fimd0_wb(ctx);
		exynos_drm_ippnb_send_event(IPP_SET_WRITEBACK, (void *)enable);

		fimc_set_type_ctrl(ctx, FIMC_WB_A);
		fimc_set_polarity(ctx, &ctx->pol);
		fimc_handle_lastend(ctx, true);

		/* ToDo: window size, prescaler config */

		ret = fimc_set_prescaler(ctx, &ctx->sc, &config_src->pos,
			&config_dst->pos);
		if (ret) {
			dev_err(dev, "failed to set precalser.\n");
			return ret;
		}

		/* If set ture, we can save jpeg about screen */
		fimc_handle_jpeg(ctx, false);
		fimc_set_scaler(ctx, &ctx->sc);
		break;
	case IPP_CMD_OUT:
	default:
		ret = -EINVAL;
		dev_err(dev, "invalid operations.\n");
		return ret;
	}

	/* Reset */
	fimc_write(0x0, EXYNOS_CISTATUS);

	cfg0 = fimc_read(EXYNOS_CIIMGCPT);
	cfg0 &= ~EXYNOS_CIIMGCPT_IMGCPTEN_SC;
	cfg0 |= EXYNOS_CIIMGCPT_IMGCPTEN_SC;

	/* Scaler */
	cfg1 = fimc_read(EXYNOS_CISCCTRL);
	cfg1 &= ~EXYNOS_CISCCTRL_SCAN_MASK;
	cfg1 |= (EXYNOS_CISCCTRL_PROGRESSIVE |
		EXYNOS_CISCCTRL_SCALERSTART);

	fimc_write(cfg1, EXYNOS_CISCCTRL);

	/* Enable image capture*/
	cfg0 |= EXYNOS_CIIMGCPT_IMGCPTEN;
	fimc_write(cfg0, EXYNOS_CIIMGCPT);

	/* Disable frame end irq */
	cfg0 = fimc_read(EXYNOS_CIGCTRL);
	cfg0 &= ~EXYNOS_CIGCTRL_IRQ_END_DISABLE;
	fimc_write(cfg0, EXYNOS_CIGCTRL);

	cfg0 = fimc_read(EXYNOS_CIOCTRL);
	cfg0 &= ~EXYNOS_CIOCTRL_WEAVE_MASK;
	fimc_write(cfg0, EXYNOS_CIOCTRL);

	/* ToDo: m2m start errata - refer fimd */
	if (cmd == IPP_CMD_M2M) {
		cfg0 = fimc_read(EXYNOS_MSCTRL);
		cfg0 |= EXYNOS_MSCTRL_ENVID;
		fimc_write(cfg0, EXYNOS_MSCTRL);

		cfg0 = fimc_read(EXYNOS_MSCTRL);
		cfg0 |= EXYNOS_MSCTRL_ENVID;
		fimc_write(cfg0, EXYNOS_MSCTRL);
	}

	return 0;
}

static int fimc_ippdrv_check_prepare(struct device *dev,
	struct exynos_drm_ippdrv *ippdrv)
{
	/* ToDo: check prepare */
	DRM_DEBUG_KMS("%s\n", __func__);

	return 0;
}

static void fimc_ippdrv_stop(struct device *dev, enum drm_exynos_ipp_cmd cmd)
{
	struct fimc_context *ctx = get_fimc_context(dev);
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;
	int enable = 0;
	u32 cfg;

	DRM_DEBUG_KMS("%s:cmd[%d]\n", __func__, cmd);

	switch (cmd) {
	case IPP_CMD_M2M:
		/* Source clear */
		cfg = fimc_read(EXYNOS_MSCTRL);
		cfg &= ~EXYNOS_MSCTRL_INPUT_MASK;
		cfg &= ~EXYNOS_MSCTRL_ENVID;
		fimc_write(cfg, EXYNOS_MSCTRL);
		break;
	case IPP_CMD_WB:
		exynos_drm_ippnb_send_event(IPP_SET_WRITEBACK, (void *)enable);
		break;
	case IPP_CMD_OUT:
	default:
		dev_err(dev, "invalid operations.\n");
		break;
	}

	ippdrv->cmd = IPP_CMD_NONE;

	/* Scaler disable */
	cfg = fimc_read(EXYNOS_CISCCTRL);
	cfg &= ~EXYNOS_CISCCTRL_SCALERSTART;
	fimc_write(cfg, EXYNOS_CISCCTRL);

	/* Disable image capture */
	cfg = fimc_read(EXYNOS_CIIMGCPT);
	cfg &= ~(EXYNOS_CIIMGCPT_IMGCPTEN_SC | EXYNOS_CIIMGCPT_IMGCPTEN);
	fimc_write(cfg, EXYNOS_CIIMGCPT);

	/* Enable frame end irq */
	cfg = fimc_read(EXYNOS_CIGCTRL);
	cfg |= EXYNOS_CIGCTRL_IRQ_END_DISABLE;
	fimc_write(cfg, EXYNOS_CIGCTRL);
}

static struct fimc_capability *fimc_get_capability(
	enum exynos_drm_fimc_ver ver)
{
	struct fimc_capability *capa;

	DRM_DEBUG_KMS("%s:ver[0x%x]\n", __func__, ver);

	/* ToDo: version check */
	switch (ver) {
	case FIMC_EXYNOS_4412:
	default:
		capa = fimc51_capa;
		break;
	}

	return capa;
}

static int __devinit fimc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct fimc_context *ctx;
	struct clk	*parent_clk;
	struct resource *res;
	struct exynos_drm_ippdrv *ippdrv;
	struct exynos_drm_fimc_pdata *pdata;
	int ret = -EINVAL;

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(dev, "no platform data specified.\n");
		return -EINVAL;
	}

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	/* clock control */
	ctx->sclk_fimc_clk = clk_get(dev, "sclk_fimc");
	if (IS_ERR(ctx->sclk_fimc_clk)) {
		dev_err(dev, "failed to get src fimc clock.\n");
		ret = PTR_ERR(ctx->sclk_fimc_clk);
		goto err_ctx;
	}
	clk_enable(ctx->sclk_fimc_clk);

	ctx->fimc_clk = clk_get(dev, "fimc");
	if (IS_ERR(ctx->fimc_clk)) {
		dev_err(dev, "failed to get fimc clock.\n");
		ret = PTR_ERR(ctx->fimc_clk);
		clk_put(ctx->sclk_fimc_clk);
		goto err_ctx;
	}

	ctx->wb_clk = clk_get(dev, "pxl_async0");
	if (IS_ERR(ctx->wb_clk)) {
		dev_err(dev, "failed to get writeback a clock.\n");
		ret = PTR_ERR(ctx->wb_clk);
		clk_put(ctx->sclk_fimc_clk);
		clk_put(ctx->fimc_clk);
		goto err_ctx;
	}

	ctx->wb_b_clk = clk_get(dev, "pxl_async1");
	if (IS_ERR(ctx->wb_b_clk)) {
		dev_err(dev, "failed to get writeback b clock.\n");
		ret = PTR_ERR(ctx->wb_b_clk);
		clk_put(ctx->sclk_fimc_clk);
		clk_put(ctx->fimc_clk);
		clk_put(ctx->wb_clk);
		goto err_ctx;
	}

	if (pdata->ver == FIMC_EXYNOS_4212 ||
		pdata->ver == FIMC_EXYNOS_4412)
		parent_clk = clk_get(dev, "mout_mpll_user");
	else
		parent_clk = clk_get(dev, "mout_mpll");

	if (IS_ERR(parent_clk)) {
		dev_err(dev, "failed to get parent clock.\n");
		ret = PTR_ERR(parent_clk);
		clk_put(ctx->sclk_fimc_clk);
		clk_put(ctx->fimc_clk);
		clk_put(ctx->wb_clk);
		clk_put(ctx->wb_b_clk);
		goto err_ctx;
	}

	if (clk_set_parent(ctx->sclk_fimc_clk, parent_clk)) {
		dev_err(dev, "failed to set parent.\n");
		clk_put(parent_clk);
		clk_put(ctx->sclk_fimc_clk);
		clk_put(ctx->fimc_clk);
		clk_put(ctx->wb_clk);
		clk_put(ctx->wb_b_clk);
		goto err_ctx;
	}
	clk_put(parent_clk);
	clk_set_rate(ctx->sclk_fimc_clk, FIMC_CLK_RATE);
	clk_disable(ctx->sclk_fimc_clk);

	/* resource memory */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "failed to find registers.\n");
		ret = -ENOENT;
		goto err_clk;
	}

	ctx->regs_res = request_mem_region(res->start, resource_size(res),
					   dev_name(dev));
	if (!ctx->regs_res) {
		dev_err(dev, "failed to claim register region.\n");
		ret = -ENOENT;
		goto err_clk;
	}

	ctx->regs = ioremap(res->start, resource_size(res));
	if (!ctx->regs) {
		dev_err(dev, "failed to map registers.\n");
		ret = -ENXIO;
		goto err_req_region;
	}

	/* resource irq */
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_err(dev, "failed to request irq resource.\n");
		goto err_get_regs;
	}

	ctx->irq = res->start;
	ret = request_irq(ctx->irq, fimc_irq_handler, 0, "drm_fimc", ctx);
	if (ret < 0) {
		dev_err(dev, "failed to request irq.\n");
		goto err_get_regs;
	}

	/* context initailization */
	ctx->ver = pdata->ver;
	ctx->id = pdev->id;
	ctx->capa = fimc_get_capability(ctx->ver);
	if (!ctx->capa) {
		dev_err(dev, "failed to get capability.\n");
		goto err_get_irq;
	}
	ctx->pol = pdata->pol;

	DRM_DEBUG_KMS("%s:id[%d]\n", __func__, ctx->id);

	ippdrv = &ctx->ippdrv;
	ippdrv->dev = dev;
	ippdrv->ops[EXYNOS_DRM_OPS_SRC] = &fimc_src_ops;
	ippdrv->ops[EXYNOS_DRM_OPS_DST] = &fimc_dst_ops;
	ippdrv->reset = fimc_ippdrv_reset;
	ippdrv->start = fimc_ippdrv_start;
	ippdrv->check_prepare = fimc_ippdrv_check_prepare;
	ippdrv->stop = fimc_ippdrv_stop;

	mutex_init(&ctx->lock);
	platform_set_drvdata(pdev, ctx);

	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);

	ret = exynos_drm_ippdrv_register(ippdrv);
	if (ret < 0) {
		dev_err(dev, "failed to register drm fimc device.\n");
		goto err_get_irq;
	}

	dev_info(&pdev->dev, "drm fimc registered successfully.\n");

	return 0;

err_get_irq:
	free_irq(ctx->irq, ctx);
err_get_regs:
	iounmap(ctx->regs);
err_req_region:
	release_resource(ctx->regs_res);
	kfree(ctx->regs_res);
err_clk:
	clk_put(ctx->sclk_fimc_clk);
	clk_put(ctx->fimc_clk);
	clk_put(ctx->wb_clk);
	clk_put(ctx->wb_b_clk);
err_ctx:
	kfree(ctx);
	return ret;
}

static int __devexit fimc_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct fimc_context *ctx = get_fimc_context(dev);
	struct exynos_drm_ippdrv *ippdrv = &ctx->ippdrv;

	exynos_drm_ippdrv_unregister(ippdrv);

	pm_runtime_set_suspended(dev);
	pm_runtime_disable(dev);

	free_irq(ctx->irq, ctx);
	iounmap(ctx->regs);
	release_resource(ctx->regs_res);
	kfree(ctx->regs_res);

	clk_put(ctx->sclk_fimc_clk);
	clk_put(ctx->fimc_clk);
	clk_put(ctx->wb_clk);
	clk_put(ctx->wb_b_clk);

	kfree(ctx);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int fimc_suspend(struct device *dev)
{
	struct fimc_context *ctx = get_fimc_context(dev);

	DRM_DEBUG_KMS("%s:id[%d]\n", __func__, ctx->id);
	if (pm_runtime_suspended(dev))
		return 0;
	/* ToDo */
	return fimc_power_on(ctx, false);
}

static int fimc_resume(struct device *dev)
{
	struct fimc_context *ctx = get_fimc_context(dev);

	DRM_DEBUG_KMS("%s:id[%d]\n", __func__, ctx->id);
	if (!pm_runtime_suspended(dev))
		return fimc_power_on(ctx, true);
	/* ToDo */
	return 0;
}
#endif

#ifdef CONFIG_PM_RUNTIME
static int fimc_runtime_suspend(struct device *dev)
{
	struct fimc_context *ctx = get_fimc_context(dev);

	DRM_DEBUG_KMS("%s:id[%d]\n", __func__, ctx->id);
	/* ToDo */
	return  fimc_power_on(ctx, false);
}

static int fimc_runtime_resume(struct device *dev)
{
	struct fimc_context *ctx = get_fimc_context(dev);

	DRM_DEBUG_KMS("%s:id[%d]\n", __func__, ctx->id);
	/* ToDo */
	return  fimc_power_on(ctx, true);
}
#endif

static const struct dev_pm_ops fimc_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(fimc_suspend, fimc_resume)
	SET_RUNTIME_PM_OPS(fimc_runtime_suspend, fimc_runtime_resume, NULL)
};

/* ToDo: need to check use case platform_device_id */
struct platform_driver fimc_driver = {
	.probe		= fimc_probe,
	.remove		= __devexit_p(fimc_remove),
	.driver		= {
		.name	= "exynos-drm-fimc",
		.owner	= THIS_MODULE,
		.pm	= &fimc_pm_ops,
	},
};

