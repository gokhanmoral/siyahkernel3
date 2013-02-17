/*
* linux/drivers/mmc/ffu/ffu.h
* eMMC Field Firmware Update module
* author : sangeun.ha@samsung.com
*
* Copyright (c) 2012 Samsung Electronics Co., Ltd.
*		http://www.samsung.com
*
* Based on linux/drivers/mmc/ffu/ffu.h
*
*/

#ifndef __FFU_H
#define __FFU_H

#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/mmc/host.h>
#include <linux/mmc/mmc.h>
#include <linux/slab.h>

#define MMC_DEVNAME "dw_mmc"
#include "../host/mshci.h"

struct mshci_s3c {
	struct mshci_host       *host;
	struct platform_device  *pdev;
	struct resource         *ioarea;
	struct s3c_mshci_platdata *pdata;
	unsigned int            cur_clk;
	int                     ext_cd_irq;
	int                     ext_cd_gpio;

	struct clk              *clk_io;
	struct clk              *clk_bus[1];
};

struct device_driver* ffu_driver(void);
struct platform_driver* ffU_platform_driver(void);
struct device* ffu_device(void);
struct platform_device* ffu_platform_device(void);
struct mshci_host* ffu_host(void);
struct mmc_host* ffu_mmc_host(void);
int mmc_send_status(u32 *status);
int mmc_send_rw_cmd(struct mmc_host *host, int rw, void *data_buf, int startblk, int size);
int mmc_send_cmd(struct mmc_command *cmd, int data_flag, void *dataAddr, int size);
void setLowClock(void);
void setHighClock(void);
#endif
