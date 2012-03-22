/* linux/arch/arm/mach-exynos/asv-4x12.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS4X12 - ASV(Adaptive Supply Voltage) driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>

#include <mach/asv.h>
#include <mach/map.h>

#define VA_LID		(S5P_VA_CHIPID + 0x14)
#define NR_SS_CASE	3

unsigned int rev_lid;

unsigned int not_ss_lid[NR_SS_CASE][2] = {
	/* lid, wno */
	{0X00197EA7, 0x6},
	{0X00197EA7, 0x7},
	{0x00197EF9, 0xFFFFFFFF},
};

/* ASV function for Non-Fused Chip */

static int exynos4x12_nonfuse_get_hpm(struct samsung_asv *asv_info)
{
	unsigned int wno;

	wno = (rev_lid >> 6) & 0x1F;

	asv_info->hpm_result = wno;

	return 0;
}

static int exynos4x12_nonfuse_get_ids(struct samsung_asv *asv_info)
{
	unsigned int lid;

	lid = (rev_lid >> 11) & 0x1FFFFF;

	asv_info->ids_result = lid;

	return 0;
}

static int exynos4x12_nonfuse_asv_store_result(struct samsung_asv *asv_info)
{
	unsigned int i;
	unsigned int ret = 4;

	for (i = 0; i < NR_SS_CASE; i++) {
		if (asv_info->ids_result == not_ss_lid[i][0]) {
			if (asv_info->hpm_result == not_ss_lid[i][1]) {
				ret = 4;
				break;
			} else {
				ret = 0xff;
				break;
			}
		}
	}

	exynos_result_of_asv = ret;

	pr_info("EXYNOS4X12: RESULT : %d\n", exynos_result_of_asv);

	return 0;
}

/* ASV function for Fused Chip */
#define IDS_ARM_OFFSET	24
#define IDS_ARM_MASK	0xFF
#define HPM_OFFSET	12
#define HPM_MASK	0x1F

struct asv_judge_table exynos4x12_limit[] = {
	/* HPM, IDS */
	{  0,   0},		/* Reserved Group */
	{  0,   0},		/* Reserved Group */
	{ 14,   9},
	{ 16,  14},
	{ 18,  17},
	{ 20,  20},
	{ 21,  24},
	{ 22,  30},
	{ 23,  34},
	{ 24,  39},
	{100, 100},
	{999, 999},		/* Reserved Group */
};

static int exynos4x12_fuse_get_hpm(struct samsung_asv *asv_info)
{
	asv_info->hpm_result = (asv_info->pkg_id >> HPM_OFFSET) & HPM_MASK;

	return 0;
}

static int exynos4x12_fuse_get_ids(struct samsung_asv *asv_info)
{
	asv_info->ids_result = (asv_info->pkg_id >> IDS_ARM_OFFSET) & IDS_ARM_MASK;

	return 0;
}

static int exynos4x12_fuse_asv_store_result(struct samsung_asv *asv_info)
{
	unsigned int i;

	/* Not use ASV 0,1,11 group */
	for (i = 2; i < ARRAY_SIZE(exynos4x12_limit) - 1; i++) {
		if (asv_info->ids_result <= exynos4x12_limit[i].ids_limit) {
			exynos_result_of_asv = i;
			break;
		}
	}

	pr_info("EXYNOS4X12: IDS : %d HPM : %d RESULT : %d\n",
		asv_info->ids_result, asv_info->hpm_result, exynos_result_of_asv);

	switch (exynos_result_of_asv) {
	case 0:
	case 1:
	case 2:
	case 3:
		exynos4x12_set_abb(ABB_MODE_100V);
		break;

	default:
		exynos4x12_set_abb(ABB_MODE_130V);
		break;
	}

	return 0;
}

int exynos4x12_asv_init(struct samsung_asv *asv_info)
{
	unsigned int lid_reg;
	unsigned int tmp;
	unsigned int i;

	exynos_result_of_asv = 0;

	pr_info("EXYNOS4X12: Adaptive Support Voltage init\n");

	tmp = __raw_readl(S5P_VA_CHIPID + 0x4);

	/* Store PKG_ID */
	asv_info->pkg_id = tmp;

	/* If no fused Chip */
	if (!((tmp >> IDS_ARM_OFFSET) & IDS_ARM_MASK) || !((tmp >> HPM_OFFSET) & HPM_MASK)) {
		rev_lid = 0;

		lid_reg = __raw_readl(VA_LID);

		for (i = 0; i < 32; i++) {
			tmp = (lid_reg >> i) & 0x1;
			rev_lid += tmp << (31 - i);
		}

		asv_info->get_ids = exynos4x12_nonfuse_get_ids;
		asv_info->get_hpm = exynos4x12_nonfuse_get_hpm;
		asv_info->store_result = exynos4x12_nonfuse_asv_store_result;

		return 0;
	}

	asv_info->get_ids = exynos4x12_fuse_get_ids;
	asv_info->get_hpm = exynos4x12_fuse_get_hpm;
	asv_info->store_result = exynos4x12_fuse_asv_store_result;

	return 0;
}
