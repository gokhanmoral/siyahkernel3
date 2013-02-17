/*
* linux/drivers/mmc/ffu/movinand.h
* eMMC Field Firmware Update module
* author : sangeun.ha@samsung.com
*
* Copyright (c) 2012 Samsung Electronics Co., Ltd.
*		http://www.samsung.com
*
* Based on linux/drivers/mmc/ffu/movinand.h
*
*/

#ifndef __MOVINAND_H
#define __MOVINAND_H

struct struct_bad_info {
	unsigned int init_bad_block;
	unsigned int runtime_bad_block;
	unsigned int remain_reserved_block;
};

struct struct_location {
	unsigned short block;
	unsigned char die;
	unsigned char chip;
	unsigned char count;
	unsigned char offset;
	unsigned short page;
};

struct struct_smart {
	unsigned int error_mode;
	unsigned int super_block_size;
	unsigned int super_page_size;
	unsigned int optimal_write_size;
	unsigned int number_of_banks;
	struct struct_bad_info bad_info[4];
	unsigned int max_erase_count;
	unsigned int min_erase_count;
	unsigned int avg_erase_count;
	unsigned int read_reclaim;
	unsigned int optimal_trim_size;
	unsigned char fw_hash[32];
	unsigned int max_erase_count_slc;
	unsigned int min_erase_count_slc;
	unsigned int avg_erase_count_slc;
	unsigned int max_erase_count_mlc;
	unsigned int min_erase_count_mlc;
	unsigned int avg_erase_count_mlc;
	unsigned int number_of_uerror;
	struct struct_location uerror[10][2];
	unsigned int erase_unit_size;
	unsigned char patch_version[16];
	unsigned int fcb_scan_result;
	unsigned int ftl_open_count;
};

int mmc_vendor_cmd62(unsigned int arg);
int mmc_smartreport(void);

int mmc_reset_write(struct mmc_host *host,const void *buff,int size);
int mmc_reset_erase(struct mmc_host *host,unsigned int start,
		unsigned int end,unsigned int arg);
#endif
