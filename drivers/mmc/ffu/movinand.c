/*
* linux/drivers/mmc/ffu/movinand.c
* eMMC Field Firmware Update module
* author : sangeun.ha@samsung.com
*
* Copyright (c) 2012 Samsung Electronics Co., Ltd.
*		http://www.samsung.com
*
* Based on linux/drivers/mmc/ffu/movinand.c
*
*/
#include "ffu.h"
#include "movinand.h"
#include <linux/delay.h>
#include <linux/mmc/card.h>
#include <linux/gpio.h>
#include <linux/reboot.h>
#include "../../../arch/arm/plat-samsung/include/plat/mshci.h"
#include "fw.h"

#define MMC_CMD_RETRIES 3

static inline void mmc_delay(unsigned int ms)
{
	if (ms < 1000 / HZ) {
		cond_resched();
		mdelay(ms);
	} else {
		msleep(ms);
	}
}

int mmc_all_send_cid(struct mmc_host *host, u32 *cid)
{
	int err;
	struct mmc_command cmd = {0};

	BUG_ON(!host);
	BUG_ON(!cid);

	cmd.opcode = MMC_ALL_SEND_CID;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_R2 | MMC_CMD_BCR;

	host->claimed = 1;
	err = mmc_wait_for_cmd(host, &cmd, 3);
	host->claimed = 0;

	if (err)
		return err;

	memcpy(cid, cmd.resp, sizeof(u32) * 4);
	printk(KERN_ALERT"FFU : %s complete\n",__FUNCTION__);

	return 0;
}

int mmc_set_relative_addr(struct mmc_card *card)
{
	int err;
	struct mmc_host *host = card->host;
	struct mmc_command cmd = {0};

	cmd.opcode = MMC_SET_RELATIVE_ADDR;
	cmd.arg = card->rca << 16;
	cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;

	host->claimed = 1;
	err = mmc_wait_for_cmd(card->host, &cmd, 3);
	host->claimed = 0;
	if (err)
		return err;
	printk(KERN_ALERT"FFU : %s complete\n",__FUNCTION__);

	return 0;
}

void mmc_set_ungated(struct mmc_host *host)
{
	unsigned long flags;

	spin_lock_irqsave(&host->clk_lock, flags);
	host->clk_gated = false;
	spin_unlock_irqrestore(&host->clk_lock, flags);
}

static inline void mmc_set_ios(struct mmc_host *host)
{
	struct mmc_ios *ios = &host->ios;

	if (ios->clock > 0)
		mmc_set_ungated(host);
	host->ops->set_ios(host, ios);
}
static void __mmc_set_clock(struct mmc_host *host, unsigned int hz)
{
	if (hz > host->f_max)
		hz = host->f_max;

	host->ios.clock = hz;
	mmc_set_ios(host);
}
void mmc_ungate_clock(struct mmc_host *host)
{
	if (host->clk_old) {
		BUG_ON(host->ios.clock);
		__mmc_set_clock(host, host->clk_old);
	}
}

static bool mmc_host_may_gate_card(struct mmc_card *card)
{
	if (!card)
		return true;
	return !(card->quirks & MMC_QUIRK_BROKEN_CLK_GATING);
}
void mmc_host_clk_hold(struct mmc_host *host)
{
	unsigned long flags;

	/* cancel any clock gating work scheduled by
	 * mmc_host_clk_release() */
	cancel_delayed_work_sync(&host->clk_gate_work);
	mutex_lock(&host->clk_gate_mutex);
	spin_lock_irqsave(&host->clk_lock, flags);
	if (host->clk_gated) {
		spin_unlock_irqrestore(&host->clk_lock, flags);
		mmc_ungate_clock(host);
		spin_lock_irqsave(&host->clk_lock, flags);
		pr_debug("%s: ungated MCI clock\n", mmc_hostname(host));
	}
	host->clk_requests++;
	spin_unlock_irqrestore(&host->clk_lock, flags);
	mutex_unlock(&host->clk_gate_mutex);
}
void mmc_host_clk_release(struct mmc_host *host)
{
	unsigned long flags;

	spin_lock_irqsave(&host->clk_lock, flags);
	host->clk_requests--;
	if (mmc_host_may_gate_card(host->card) &&
			!host->clk_requests)
		queue_delayed_work(system_nrt_wq, &host->clk_gate_work,
				msecs_to_jiffies(host->clkgate_delay));

	spin_unlock_irqrestore(&host->clk_lock, flags);
}

#define READ 0
#define WRITE 1

void mmc_set_clock(struct mmc_host *host, unsigned int hz)
{
	mmc_host_clk_hold(host);
	__mmc_set_clock(host, hz);
	mmc_host_clk_release(host);
}

int mmc_vendor_cmd60(unsigned int arg)
{
	int err;
	struct mmc_host *host = ffu_mmc_host();
	unsigned int claim = 0;
	int resp;
	struct mmc_command cmd = {0};

	if( host )
	{
		claim = host->claimed;
		cmd.opcode = 60;
		cmd.arg = arg;
		cmd.flags = MMC_RSP_SPI_R1B;

		host->claimed = 1;
		err = mmc_wait_for_cmd(host, &cmd, 3);
		host->claimed = claim;
		if (err)
		{
			printk(KERN_ALERT"FFU : CMD60 error with arg : %x\n",arg);
			return err;
		}

		do{
			mmc_send_status(&resp);
		}while(resp != 0x900);

		return 0;
	}

	return -1;
}

int mmc_vendor_cmd62(unsigned int arg)
{
	struct mmc_host *host = ffu_mmc_host();
	int resp;
	struct mmc_command cmd = {0};

	if( host )
	{
		unsigned int claim;
		int err;
		claim = host->claimed;
		cmd.opcode = 62;
		cmd.arg = arg;
		cmd.flags = MMC_RSP_SPI_R1B;

		host->claimed = 1;
		err = mmc_wait_for_cmd(host, &cmd, 3);
		host->claimed = claim;
		if (err)
		{
			printk(KERN_ALERT"FFU : CMD62 error with arg : %x\n",arg);
			return err;
		}

		do{
			mmc_send_status(&resp);
		}while(resp != 0x900);

		return 0;
	}

	return -1;
}


int mmc_smartreport(void)
{
	struct mmc_host *host;
	int resp;
	char *buff = kmalloc(512, GFP_KERNEL);

	if(!buff)
	{
		printk(KERN_ALERT"[%s] Memory allocation fail\n",__FUNCTION__);
		return -1;
	}

	host = ffu_mmc_host();


	if( !host )
		goto errout;

	if(mmc_vendor_cmd62(0xEFAC62EC))
	{
		printk(KERN_ALERT "[FFU] error while smart report step 1.\n");
		goto errout;
	}

	mmc_send_status(&resp);

	if(mmc_vendor_cmd62(0x0000CCEE))
	{
		printk(KERN_ALERT "[FFU] error while smart report step 2.\n");
		goto errout;
	}

	mmc_send_status(&resp);


	mmc_send_rw_cmd(host,MMC_READ_SINGLE_BLOCK, buff, 0x1000, 512);

	if(mmc_vendor_cmd62(0xEFAC62EC))
	{
		printk(KERN_ALERT "[FFU] error while smart report step 3.\n");
		goto errout;
	}

	mmc_send_status(&resp);

	if(mmc_vendor_cmd62(0x00DECCEE))
	{
		printk(KERN_ALERT "[FFU] error while smart report step 4.\n");
		goto errout;
	}

	mmc_send_status(&resp);

	if( !(buff[312] == 'V' &&  buff[313] == 'H' &&
			buff[314] == 'X' && buff[315] == '0') )
	{
		printk(KERN_ALERT"FFU It's not a VHX. %x%x%x%x",
				buff[312], buff[313], buff[314], buff[315]);
		return -1;
	}
		
	{
		struct struct_smart *smart = (struct struct_smart*)buff;
		int n1, n2, n3;
		n1 = smart->patch_version[12];
		n2 = smart->patch_version[13];
		n3 = smart->patch_version[14];

		if( n3 == 0x12 && n2 == 0x12 && n1 == 0x15 )
		{
			printk(KERN_ALERT "Pass ffu");

			if( buff )
				kfree(buff);

			return 1;
		}
		else if( !(n3 == 0x12 && n2 == 0x4 && n1 == 0x5) )
		{
			printk(KERN_ALERT "It's not needed!!");

			if( buff )
				kfree(buff);

			return 1;
		}
	}

	if( buff )
		kfree(buff);
	printk(KERN_ALERT"FFU : %s complete\n",__FUNCTION__);
	return 0;

errout:
	if( buff )
		kfree(buff);
	return -1;
}

int mmc_patch_read(struct mmc_host *host, const void *buff, int size)
{
	int err;
	int blkcnt;
	struct mmc_command cmd = {0};

	blkcnt = (size+512-1)/512;

	cmd.opcode= 23;
	cmd.arg = blkcnt;
	cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;

	host->claimed = 1;
	err = mmc_wait_for_cmd(host, &cmd, 3);
	host->claimed = 0;

	if( err )
		printk(KERN_ALERT"FFU : %s, %d error\n",__FUNCTION__, __LINE__);

	mmc_delay(100);

	cmd.opcode = 18;
	cmd.arg =0;
	cmd.flags = MMC_RSP_R1;// | MMC_CMD_AC;

	err = mmc_send_cmd(&cmd, MMC_DATA_READ, (int*)buff, size);
	printk(KERN_ALERT"<%s:%d>",__FUNCTION__, __LINE__);

	if( err )
		printk(KERN_ALERT"FFU : %s, %d error\n",__FUNCTION__, __LINE__);

	host->use_spi_crc = 0;
	printk(KERN_ALERT"FFU : %s complete\n",__FUNCTION__);

	return err;
}


int mmc_patch_write(struct mmc_host *host, const void *buff, int size)
{
	int err;
	int blkcnt;
	struct mmc_command cmd = {0};

	blkcnt = (size+512-1)/512;

	cmd.opcode= 23;
	cmd.arg = blkcnt;
	cmd.flags = MMC_RSP_R1 | MMC_CMD_AC;

	host->claimed = 1;
	err = mmc_wait_for_cmd(host, &cmd, 3);
	host->claimed = 0;

	if( err )
		printk(KERN_ALERT"FFU : %s, %d error\n",__FUNCTION__, __LINE__);

	mmc_delay(100);

	cmd.opcode = 25;
	cmd.arg =0;
	//cmd.flags = MMC_RSP_R1; /*MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;*/
	cmd.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_ADTC;

	err = mmc_send_cmd(&cmd, MMC_DATA_WRITE, (int*)buff, size);
	printk(KERN_ALERT"<%s:%d>",__FUNCTION__, __LINE__);

	if( err )
		printk(KERN_ALERT"FFU : %s, %d error\n",__FUNCTION__, __LINE__);

	host->use_spi_crc = 0;
	printk(KERN_ALERT"FFU : %s complete\n",__FUNCTION__);

	return err;
}

int mmc_patch_execute(struct mmc_host *host,int arg)
{
	int err = 0;
	int resp;
	struct mmc_command cmd = {0};

	cmd.opcode = 35;
	cmd.arg = arg;
	cmd.flags = MMC_RSP_R1B;

	host->claimed = 1;
	err = mmc_wait_for_cmd(host, &cmd, 3);
	if( err )
		return err;
	host->claimed = 0;
	do
	{
		mmc_send_status(&resp);
	}while(resp != 0x900);

	memset(&cmd, 0, sizeof(cmd));

	cmd.opcode = 36;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_R1B;

	host->claimed = 1;
	err = mmc_wait_for_cmd(host, &cmd, 3);
	if( err )
		return err;

	host->claimed = 0;
	do
	{
		mmc_send_status(&resp);
	}while(resp != 0x900);

	memset(&cmd, 0, sizeof(cmd));

	cmd.opcode = 38;
	cmd.arg = 0;
	cmd.flags = MMC_RSP_R1B;

	host->claimed = 1;
	err = mmc_wait_for_cmd(host, &cmd, 3);
	if( err )
		return err;

	host->claimed = 0;
	do
	{
		mmc_send_status(&resp);
	}while(resp != 0x900);

	return 0;
}

//#define PATCH_NORMAL
int mmc_patch_normal(void)
{
	struct mmc_host *host;
	char *buff;
	int i;

	int sz = sizeof(movinand_fw);

	host = ffu_mmc_host();

	buff = (char*)kmalloc(sizeof(movinand_fw), GFP_KERNEL);
	if( !buff )
		return -1;

	if(mmc_vendor_cmd62(0xEFAC62EC))
		goto errout;

	if(mmc_vendor_cmd62(0x10210001))
		goto errout;

	memcpy(buff,movinand_fw, sz);

	printk(KERN_ALERT"FFU start f/w write\n");

	if( mmc_patch_write(host, buff,sz ) )
	{
		printk(KERN_ALERT"<%s:%d> errorno",__FUNCTION__, __LINE__);
		goto errout;
	}

	printk(KERN_ALERT"FFU complete f/w write\n");

	if(mmc_vendor_cmd62(0xEFAC62EC))
		goto errout;

	if(mmc_vendor_cmd62(0x10210002))
		goto errout;

	memset(buff, 0, sz);

	if(mmc_patch_read(host, buff, sz) )
	{
		printk(KERN_ALERT"<%s:%d> errorno",__FUNCTION__, __LINE__);
		goto errout;
	}

	for(i = 0 ; i < sz ; i++)
		if( movinand_fw[i] != buff[i] )
			break;

	if( i < sz )
	{
		printk(KERN_ALERT"Patch validation failed");
		goto errout;
	}
		printk(KERN_ALERT"Patch validation succeed");

	if(mmc_vendor_cmd62(0xEFAC62EC))
		goto errout;

	if(mmc_vendor_cmd62(0x10210010))
		goto errout;

	if(mmc_patch_execute(host,0x20020001))
	{
		printk(KERN_ALERT"Patch execute failed");
		goto errout;
	}

	kfree(buff);
	return 0;

errout:
	if(buff)
		kfree(buff);

	return -1;
}

int mmc_init(void)
{
	struct mmc_host *host;
	int err;
	int cid[4];
	struct mmc_command cmd = {0};

	host = ffu_mmc_host();

	// CMD0
	cmd.opcode = 0;
	cmd.arg = 0;
	cmd.flags = 0;
	host->claimed = 1;
	err = mmc_wait_for_cmd(host, &cmd, 3);
	if( err )
	{
		printk(KERN_ALERT"FFU : CMD0 error after patch %d",err);
		return err;
	}
	host->claimed = 0;

	// CMD1
	do
	{
		memset(&cmd, 0, sizeof(cmd));
		cmd.opcode = 1;
		cmd.arg = 0x40FF8080;
		cmd.flags = MMC_RSP_R3 | MMC_CMD_BCR;
		host->claimed = 1;
		err = mmc_wait_for_cmd(host, &cmd, 3);
		if( err )
			return err;
		host->claimed = 0;
	}while(cmd.resp[0] >> 31 == 0);

	// CMD2
	if(mmc_all_send_cid(host, cid))
	{
		printk(KERN_ALERT"FFU CMD2 error");
		return -1;
	}

	// CMD3
	if(mmc_set_relative_addr(host->card))
	{
		printk(KERN_ALERT"FFU CMD3 error");
		return -1;
	}

	// CMD10
	memset(&cmd, 0, sizeof(cmd));
	cmd.opcode = 10;
	cmd.arg = 1 << 16;
	cmd.flags = MMC_RSP_R2 | MMC_CMD_AC;

	host->claimed = 1;
	err = mmc_wait_for_cmd(host, &cmd, MMC_CMD_RETRIES);
	host->claimed = 0;
	if (err)
		return err;

	printk(KERN_ALERT"FFU cid : %x %x %x %x",
			cmd.resp[0], cmd.resp[1],cmd.resp[2],cmd.resp[3]);


	return 0;
}

int check_need_patch(void)
{
	struct mmc_host *host;
	struct mmc_card *card;
	int prv;

	host = ffu_mmc_host();

	if( !host )
		return -1;

	card = host->card;

	if( !card )
		return -1;

	card = host->card;

	prv = (card->raw_cid[2] >> 16) & 0xFF;

	if( card->cid.manfid != 0x15 )	// not movinand
		return -1;

	return 0;
}

int movinand_fwUpdate(void)
{
	int ret = 0;

	if( check_need_patch() )
	{
		printk(KERN_ALERT"FFU : does not needed");
		return 0;
	}

	ret = mmc_smartreport();
	if( ret )
		goto out;

	mmc_set_clock(ffu_mmc_host(), 400000);
	ret = mmc_patch_normal();

	mdelay(200);

	mmc_init();

	kernel_restart(0);

out:
	if( ret == 1 )
		return 0;

	return ret;
}
