/*
* linux/drivers/mmc/ffu/ffu.c
* eMMC Field Firmware Update module
* author : sangeun.ha@samsung.com
*
* Copyright (c) 2012 Samsung Electronics Co., Ltd.
*		http://www.samsung.com
*
* Based on linux/drivers/mmc/ffu/ffu.c
*
*/

#include "ffu.h"
#include "movinand.h"

static struct device_driver *drv = NULL;
static struct platform_driver *plat_drv = NULL;
static struct device *dev = NULL;
static struct platform_device *plat_dev = NULL;
static struct mshci_host *host = NULL;

#include "movinand.c"
#include "../../base/base.h"


static struct mshci_s3c *sc = NULL;

struct mmc_host* ffu_mmc_host(void)
{
	if( host )
		return host->mmc;

	return 0;
}

static int device_find(const char *name, struct bus_type *bus)
{
	dev = bus_find_device_by_name(&platform_bus_type, NULL, MMC_DEVNAME);

	if( dev )
	{
		plat_dev = container_of( dev, struct platform_device, dev);
		printk(KERN_ALERT "Found platform device %s : %x\n",
				plat_dev->name, (unsigned int)plat_dev);
		return 0;
	}

	printk(KERN_ALERT "Can not find %s device\n", MMC_DEVNAME);

	return -1;
}
static int initFFU(void)
{
	drv = driver_find(MMC_DEVNAME,&platform_bus_type);

	if( drv )
	{
		plat_drv = container_of(drv,struct platform_driver, driver);
		printk(KERN_ALERT "Found platform driver %s : %x\n",
				MMC_DEVNAME, (unsigned int)plat_drv);
	}
	else
	{
		printk(KERN_ALERT "Can not find %s driver\n", MMC_DEVNAME);
		return -1;
	}

	return device_find(MMC_DEVNAME, &platform_bus_type);
}

static int getHostData(void)
{
	struct mmc_host *mmchost;

	host = platform_get_drvdata(plat_dev);
	sc = mshci_priv(host);
	if( host )
	{
		struct mmc_ios *ios;
		struct platform_device *pdev = sc->pdev;
		if( pdev != plat_dev )
		{
			printk( KERN_ALERT "FFU has problem. host data is wrong. exit ffu.\n");
			return -1;
		}
		printk( KERN_ALERT "FFU getting host data success.\n");

		mmchost = host->mmc;
		ios = &mmchost->ios;
		printk( KERN_ALERT "%s: clock %uHz busmode %u powermode %u cs %u Vdd %u "
				"width %u timing %u\n",
				mmc_hostname(mmchost), ios->clock, ios->bus_mode,
				ios->power_mode, ios->chip_select, ios->vdd,
				ios->bus_width, ios->timing);

	}

	return 0;
}

static int doWorks(void)
{
	int ret = movinand_fwUpdate();

	return ret;
}

int mmc_send_status(u32 *status)
{
	int err;
	struct mmc_command cmd = {0};

	cmd.opcode = MMC_SEND_STATUS;
	cmd.arg = 1 << 16;
	cmd.flags = MMC_RSP_SPI_R2 | MMC_RSP_R1 | MMC_CMD_AC;

//	__mmc_claim_host(host->mmc, NULL);
	host->mmc->claimed = 1;
	err = mmc_wait_for_cmd(host->mmc, &cmd, 3);
	host->mmc->claimed = 0;

//	mmc_release_host(host->mmc);

	if (err)
		return err;

	if (status)
		*status = cmd.resp[0];

	return 0;
}


int mmc_send_rw_cmd(struct mmc_host *host, int rw, void *data_buf, int startblk, int size)
{
	struct mmc_request mrq = {0};
	struct mmc_command cmd = {0};
	struct mmc_data data = {0};
	struct scatterlist sg;
	struct mmc_card *card;
	u32 status;
	int err = 0;

	card = host->card;
	mrq.cmd = &cmd;
	mrq.data = &data;
	cmd.opcode = rw;
	cmd.arg = startblk;

	cmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;

	/* Set data 2 */
	data.blksz = 512;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	/* Send CMD */
	data.sg = &sg;
	data.sg_len = 1;

	mmc_set_data_timeout(&data, host->card);

	sg_init_one(&sg, data_buf, 512);

	__mmc_claim_host(host, NULL);

	if (card->host->ops->pre_req) {
		mmc_host_clk_hold(card->host);
		card->host->ops->pre_req(card->host, &mrq, false);
		mmc_host_clk_release(card->host);
	}

	mmc_wait_for_req(card->host, &mrq);

	if (card->host->ops->post_req) {
		mmc_host_clk_hold(card->host);
		card->host->ops->post_req(card->host, &mrq, false);
		mmc_host_clk_release(card->host);
	}

	mmc_release_host(host);

	if (cmd.error) {
		printk(KERN_ERR "%s: CMD%d: Command Error, return %d.\n",
				mmc_hostname(host),
				cmd.opcode, cmd.error);
		return cmd.error;
	}
	if (data.error) {
		printk(KERN_ERR "%s: CMD%d: Data Error, return %d.\n",
				mmc_hostname(host),
				cmd.opcode, data.error);
		return data.error;
	}

	/* check status */
	do {
		err = mmc_send_status(&status);
		if (err)
			return err;
	} while (status != 0x900);

	return err;
}

static struct sg_table *sgt = 0;

void set_data(struct mmc_card *card, struct mmc_data *data, char *buff, int size)
{
	struct scatterlist *sg_ptr;
	unsigned int nents, left,i;
	unsigned int seg_size;

	seg_size = card->host->max_seg_size;

	data->blksz = 512;
	data->blocks = (size+512-1)/512;

	left = data->blksz * data->blocks;
	nents = (left -1 ) / seg_size + 1;


	sgt = kmalloc( sizeof( struct sg_table), GFP_KERNEL );
	sg_alloc_table(sgt, nents, GFP_KERNEL);

	sg_ptr = sgt->sgl;

	data->sg = sg_ptr;
	data->sg_len = nents;

	sg_init_table(sg_ptr, nents);

	for_each_sg(data->sg, sg_ptr, data->sg_len, i) {
		sg_set_page(sg_ptr, virt_to_page(buff + (i * seg_size)),
				min(seg_size, left),
				offset_in_page(buff + (i * seg_size)));
		left = left - seg_size;
	}

	mmc_set_data_timeout(data, card);
}

int mmc_send_cmd(struct mmc_command *cmd, int data_flag, void *dataAddr, int size)
{
	struct mmc_request mrq = {0};
	struct mmc_data data = {0};
	struct mmc_host *mmchost = ffu_mmc_host();
	struct mmc_card *card;
	int err = 0;

	card = mmchost->card;

	mrq.cmd = cmd;
	mrq.data = &data;

	/* Set data 2 */
	data.flags = data_flag;
	set_data(mmchost->card, &data, dataAddr, size);

	__mmc_claim_host(mmchost, NULL);

	if (card->host->ops->pre_req) {
		mmc_host_clk_hold(card->host);
		card->host->ops->pre_req(card->host, &mrq, false);
		mmc_host_clk_release(card->host);
	}

	mmc_wait_for_req(card->host, &mrq);

	if (card->host->ops->post_req) {
		mmc_host_clk_hold(card->host);
		card->host->ops->post_req(card->host, &mrq, false);
		mmc_host_clk_release(card->host);
	}

	mmc_release_host(mmchost);
	if( sgt )
	{
		sg_free_table(sgt);
		kfree(sgt);
		sgt = 0;
	}

	if (cmd->error) {
		printk(KERN_ERR "%s: CMD%d: Command Error, return %d.\n",
				mmc_hostname(mmchost),
				cmd->opcode, cmd->error);
		return cmd->error;
	}
	if (data.error) {
		printk(KERN_ERR "%s: CMD%d: Data Error, return %d.\n",
				mmc_hostname(mmchost),
				cmd->opcode, data.error);
		return data.error;
	}


	return err;
}

static __init int ffu_init(void)
{
	int ret;

	ret = initFFU();
	if( !ret )
		ret = getHostData();

	doWorks();

	return ret;
}

static __exit void ffu_exit(void)
{
}

module_init(ffu_init);
module_exit(ffu_exit);
MODULE_LICENSE("GPL v2");
