/* /linux/driver/video/samsung/mdnie.c
 *
 * Samsung SoC mDNIe driver.
 *
 * Copyright (c) 2011 Samsung Electronics.
 * Author: InKi Dae  <inki.dae@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include <plat/fimd_lite_ext.h>
#include <mach/map.h>

#include "regs-mdnie.h"
#include "mdnie.h"
#include "s5p_fimd_ext.h"
#include "cmc623.h"


enum mdnie_alg {
	ALG_MIN,
	ALG_DNR_HDTR		= ALG_MIN,
	ALG_DNR_HDTR_PCA,
	ALG_DNR_HDTR_OVE,
	ALG_MAX			= ALG_DNR_HDTR_OVE,
};

enum mdnie_block {
	ROI_MIN,
	ROI_PCA_OVE		= ROI_MIN,
	ROI_HDTR,
	ROI_DNR,
	ROI_DITHER,
	ROI_MAX			= ROI_DITHER,
};

enum mdnie_roi_side {
	ROI_INSIDE,
	ROI_OUTSIDE,
};

enum mdnie_abc_mode {
	LABC_OFF_CABC_OFF,
	LABC_ON_CABC_OFF	= 2,
	LABC_ON_CABC_ON,
};

enum mdnie_display_type {
	LCD_TYPE,
	OLED_TYPE,
};

enum mdnie_illuminance_type {
	ILLUMINANCE_VALUE,
	ILLUMINANCE_LEVEL,
};

enum mdnie_pca_mode {
	PCA_PURPLE,
	PCA_YELLOW,
	PCA_RED,
	PCA_GREEN,
	PCA_SKY,
	PCA_SKIN
};

enum mdnie_quadrant {
	QUA_LEFT_TOP,
	QUA_RIGHT_TOP,
	QUA_LEFT_BOTTOM,
	QUA_RIGHT_BOTTOM,
};

enum mdnie_pwm_selection {
	ABC_FINAL_PWM,
	PWM_COEFF_CNT,
};

enum mdnie_pwm_polarity {
	PWM_LOW_ACTIVE,
	PWM_HIGH_ACTIVE,
};

enum mdnie_labc_mode {
	USER_PWM,
	CABC_PWM,
	LABC_PWM,
	LABC_CABC_PWM,
};

struct roi_rect {
	unsigned int start_x;
	unsigned int end_x;
	unsigned int start_y;
	unsigned int end_y;
};

struct s5p_mdnie {
	struct device			*dev;
	struct clk			*clk;
	void __iomem			*iomem_base;
	unsigned int			irq;

	struct mdnie_platform_data	*pdata;
};

static struct mdnie_platform_data
	*to_mdnie_platform_data(struct s5p_fimd_ext_device *fx_dev)
{
	return fx_dev->dev.platform_data ?
		(struct mdnie_platform_data *)fx_dev->dev.platform_data : NULL;
}

static int mdnie_set_mode(struct s5p_mdnie *mdnie,
		enum mdnie_mode e_mode)
{
	unsigned int i = 0;
	struct mdnie_tables *tables;

	if (e_mode < CMC623_MODE_MIN || e_mode > CMC623_MODE_MAX) {
		dev_err(mdnie->dev, "invalid mdnie mode.\n");
		return -EINVAL;
	}

	tables = &cmc623_tables;

	if (!tables) {
		dev_err(mdnie->dev, "mdnie tables is NULL.\n");
		return -EINVAL;
	}

	while (tables->tables[e_mode][i += 2] != END_CMD) {
		writel(tables->tables[e_mode][i + 1],
				mdnie->iomem_base +
				tables->tables[e_mode][i] * 4);
	}

	return 0;
}

static void mdnie_set_size(struct s5p_mdnie *mdnie,
		unsigned int width, unsigned int height)
{
	unsigned int size;

	writel(0x0, mdnie->iomem_base + MDNIE_R0);

	/* Input Data Unmask */
	writel(0x077, mdnie->iomem_base + MDNIE_R1);

	/* LCD width */
	size = readl(mdnie->iomem_base + MDNIE_R3);
	size &= ~MDNIE_R34_WIDTH_MASK;
	size |= width;
	writel(size, mdnie->iomem_base + MDNIE_R3);

	/* LCD height */
	size = readl(mdnie->iomem_base + MDNIE_R4);
	size &= ~MDNIE_R35_HEIGHT_MASK;
	size |= height;
	writel(size, mdnie->iomem_base + MDNIE_R4);

	/* unmask all */
	writel(0, mdnie->iomem_base + MDNIE_R28);
}

static int mdnie_init_hardware(struct s5p_mdnie *mdnie)
{
	struct mdnie_platform_data *pdata = NULL;

	pdata = mdnie->pdata;

	/* set display size. */
	mdnie_set_size(mdnie, pdata->width, pdata->height);

	return 0;
}

static int mdnie_setup(struct s5p_fimd_ext_device *fx_dev, unsigned int enable)
{
	struct s5p_mdnie *mdnie = fimd_ext_get_drvdata(fx_dev);

	if (enable) {
		mdnie_init_hardware(mdnie);

		/* default mode is UI. */
		mdnie_set_mode(mdnie, CMC623_MODE_UI);
	}

	return 0;
}

static int mdnie_probe(struct s5p_fimd_ext_device *fx_dev)
{
	struct resource *res;
	struct s5p_mdnie *mdnie;

	mdnie = kzalloc(sizeof(struct s5p_mdnie), GFP_KERNEL);
	if (!mdnie) {
		dev_err(&fx_dev->dev, "failed to alloc mdnie object.\n");
		return -EFAULT;
	}

	mdnie->dev = &fx_dev->dev;

	mdnie->pdata = to_mdnie_platform_data(fx_dev);
	if (mdnie->pdata == NULL) {
		dev_err(&fx_dev->dev, "platform_data is NULL.\n");
		return -EFAULT;
	}

	res = s5p_fimd_ext_get_resource(fx_dev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&fx_dev->dev, "failed to get io memory region.\n");
		return -EINVAL;
	}

	mdnie->iomem_base = ioremap(res->start, resource_size(res));
	if (!mdnie->iomem_base) {
		dev_err(&fx_dev->dev, "failed to remap io region.\n'");
		return -EFAULT;
	}

	fimd_ext_set_drvdata(fx_dev, mdnie);

	dev_info(&fx_dev->dev, "mDNIe driver has been probed.\n");

	return 0;
}

static struct s5p_fimd_ext_driver fimd_ext_driver = {
	.driver		= {
		.name	= "mdnie",
		.owner	= THIS_MODULE,
	},
	.probe		= mdnie_probe,
	.setup		= mdnie_setup,
};

static int __init mdnie_init(void)
{
	return s5p_fimd_ext_driver_register(&fimd_ext_driver);
}

static void __exit mdnie_exit(void)
{
}

arch_initcall(mdnie_init);
module_exit(mdnie_exit);

MODULE_AUTHOR("InKi Dae <inki.dae@samsung.com>");
MODULE_DESCRIPTION("mDNIe Driver");
MODULE_LICENSE("GPL");

