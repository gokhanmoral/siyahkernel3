/*
 *  This file contains work-arounds for many known SD/MMC
 *  and SDIO hardware bugs.
 *
 *  Copyright (c) 2011 Andrei Warkentin <andreiw@motorola.com>
 *  Copyright (c) 2011 Pierre Tardy <tardyp@gmail.com>
 *  Inspired from pci fixup code:
 *  Copyright (c) 1999 Martin Mares <mj@ucw.cz>
 *
 */

#include <linux/slab.h>
#include <linux/types.h>
#include <linux/scatterlist.h>
#include <linux/kernel.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/host.h>

#include "mmc_ops.h"

#ifndef SDIO_VENDOR_ID_TI
#define SDIO_VENDOR_ID_TI		0x0097
#endif

#ifndef SDIO_DEVICE_ID_TI_WL1271
#define SDIO_DEVICE_ID_TI_WL1271	0x4076
#endif

#ifndef SDIO_VENDOR_ID_BRCM
#define SDIO_VENDOR_ID_BRCM		0x02D0
#endif

#ifndef SDIO_DEVICE_ID_BRCM_BCM4330
#define SDIO_DEVICE_ID_BRCM_BCM4330	0x4330
#endif

#ifndef SDIO_DEVICE_ID_BRCM_BCM4334
#define SDIO_DEVICE_ID_BRCM_BCM4334	0x4334
#endif

#ifndef SDIO_DEVICE_ID_BRCM_BCM43241
#define SDIO_DEVICE_ID_BRCM_BCM43241	0x4324
#endif

/*
 * This hook just adds a quirk for all sdio devices
 */
static void add_quirk_for_sdio_devices(struct mmc_card *card, int data)
{
	if (mmc_card_sdio(card))
		card->quirks |= data;
}

static const struct mmc_fixup mmc_fixup_methods[] = {
	/* by default sdio devices are considered CLK_GATING broken */
	/* good cards will be whitelisted as they are tested */
	SDIO_FIXUP(SDIO_ANY_ID, SDIO_ANY_ID,
		   add_quirk_for_sdio_devices,
		   MMC_QUIRK_BROKEN_CLK_GATING),

	SDIO_FIXUP(SDIO_VENDOR_ID_TI, SDIO_DEVICE_ID_TI_WL1271,
		   remove_quirk, MMC_QUIRK_BROKEN_CLK_GATING),

	SDIO_FIXUP(SDIO_VENDOR_ID_TI, SDIO_DEVICE_ID_TI_WL1271,
		   add_quirk, MMC_QUIRK_NONSTD_FUNC_IF),

	SDIO_FIXUP(SDIO_VENDOR_ID_TI, SDIO_DEVICE_ID_TI_WL1271,
		   add_quirk, MMC_QUIRK_DISABLE_CD),

	SDIO_FIXUP(SDIO_VENDOR_ID_BRCM, SDIO_DEVICE_ID_BRCM_BCM4330,
			remove_quirk, MMC_QUIRK_BROKEN_CLK_GATING),

	SDIO_FIXUP(SDIO_VENDOR_ID_BRCM, SDIO_DEVICE_ID_BRCM_BCM4334,
		   remove_quirk, MMC_QUIRK_BROKEN_CLK_GATING),

	SDIO_FIXUP(SDIO_VENDOR_ID_BRCM, SDIO_DEVICE_ID_BRCM_BCM43241,
		   remove_quirk, MMC_QUIRK_BROKEN_CLK_GATING),

	END_FIXUP
};

void mmc_fixup_device(struct mmc_card *card, const struct mmc_fixup *table)
{
	const struct mmc_fixup *f;
	u64 rev = cid_rev_card(card);

	/* Non-core specific workarounds. */
	if (!table)
		table = mmc_fixup_methods;

	for (f = table; f->vendor_fixup; f++) {
		if ((f->manfid == CID_MANFID_ANY ||
		     f->manfid == card->cid.manfid) &&
		    (f->oemid == CID_OEMID_ANY ||
		     f->oemid == card->cid.oemid) &&
		    (f->name == CID_NAME_ANY ||
		     !strncmp(f->name, card->cid.prod_name,
			      sizeof(card->cid.prod_name))) &&
		    (f->cis_vendor == card->cis.vendor ||
		     f->cis_vendor == (u16) SDIO_ANY_ID) &&
		    (f->cis_device == card->cis.device ||
		     f->cis_device == (u16) SDIO_ANY_ID) &&
		    rev >= f->rev_start && rev <= f->rev_end) {
			dev_dbg(&card->dev, "calling %pF\n", f->vendor_fixup);
			f->vendor_fixup(card, f->data);
		}
	}
}
EXPORT_SYMBOL(mmc_fixup_device);

/*
 * Quirk code to fix bug in wear leveling firmware for certain Samsung emmc
 * chips
 */
static int mmc_movi_vendor_cmd(struct mmc_card *card, unsigned int arg)
{
	struct mmc_command cmd = {0};
	int err;
	u32 status;

	/* CMD62 is vendor CMD, it's not defined in eMMC spec. */
	cmd.opcode = 62;
	cmd.flags = MMC_RSP_R1B;
	cmd.arg = arg;

	err = mmc_wait_for_cmd(card->host, &cmd, 0);
	if (err)
		return err;

	do {
		err = mmc_send_status(card, &status);
		if (err)
			return err;
		if (card->host->caps & MMC_CAP_WAIT_WHILE_BUSY)
			break;
		if (mmc_host_is_spi(card->host))
			break;
	} while (R1_CURRENT_STATE(status) == R1_STATE_PRG);

	return err;
}

static int mmc_movi_erase_cmd(struct mmc_card *card,
			unsigned int arg1, unsigned int arg2)
{
	struct mmc_command cmd = {0};
	int err;
	u32 status;

	cmd.opcode = MMC_ERASE_GROUP_START;
	cmd.flags = MMC_RSP_R1;
	cmd.arg = arg1;

	err = mmc_wait_for_cmd(card->host, &cmd, 0);
	if (err)
		return err;

	memset(&cmd, 0, sizeof(struct mmc_command));
	cmd.opcode = MMC_ERASE_GROUP_END;
	cmd.flags = MMC_RSP_R1;
	cmd.arg = arg2;

	err = mmc_wait_for_cmd(card->host, &cmd, 0);
	if (err)
		return err;

	memset(&cmd, 0, sizeof(struct mmc_command));
	cmd.opcode = MMC_ERASE;
	cmd.flags = MMC_RSP_R1B;
	cmd.arg = 0x00000000;

	err = mmc_wait_for_cmd(card->host, &cmd, 0);
	if (err)
		return err;

	do {
		err = mmc_send_status(card, &status);
		if (err)
			return err;
		if (card->host->caps & MMC_CAP_WAIT_WHILE_BUSY)
			break;
		if (mmc_host_is_spi(card->host))
			break;
	} while (R1_CURRENT_STATE(status) == R1_STATE_PRG);

	return err;
}

#define TEST_MMC_FW_PATCHING

#ifdef TEST_MMC_FW_PATCHING
static struct mmc_command wcmd;
static struct mmc_data wdata;

static void mmc_movi_read_cmd(struct mmc_card *card, u8 *buffer)
{
	struct mmc_request brq;
	struct scatterlist sg;

	brq.cmd = &wcmd;
	brq.data = &wdata;

	wcmd.opcode = MMC_READ_SINGLE_BLOCK;
	wcmd.arg = 0;
	wcmd.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
	wdata.blksz = 512;
	brq.stop = NULL;
	wdata.blocks = 1;
	wdata.flags = MMC_DATA_READ;

	wdata.sg = &sg;
	wdata.sg_len = 1;

	sg_init_one(&sg, buffer, 512);

	mmc_set_data_timeout(&wdata, card);

	mmc_wait_for_req(card->host, &brq);
}
#endif /* TEST_MMC_FW_PATCHING */

/*
 * Copy entire page when wear leveling is happened
 */
static int mmc_set_wearlevel_page(struct mmc_card *card)
{
	int err;
#ifdef TEST_MMC_FW_PATCHING
	void *buffer;
	buffer = kmalloc(512, GFP_KERNEL);
	if (!buffer) {
		pr_err("Fail to alloc memory for set wearlevel\n");
		return -ENOMEM;
	}
#endif /* TEST_MMC_FW_PATCHING */

	/* modification vendor cmd */
	/* enter vendor command mode */
	err = mmc_movi_vendor_cmd(card, 0xEFAC62EC);
	if (err)
		goto out;

	err = mmc_movi_vendor_cmd(card, 0x10210000);
	if (err)
		goto out;

	/* set value 0x000000FF : It's hidden data
	 * When in vendor command mode, the erase command is used to
	 * patch the firmware in the internal sram.
	 */
	err = mmc_movi_erase_cmd(card, 0x0004DD9C, 0x000000FF);
	if (err) {
		pr_err("Fail to Set WL value1\n");
		goto err_set_wl;
	}

	/* set value 0xD20228FF : It's hidden data */
	err = mmc_movi_erase_cmd(card, 0x000379A4, 0xD20228FF);
	if (err) {
		pr_err("Fail to Set WL value2\n");
		goto err_set_wl;
	}

err_set_wl:
	/* exit vendor command mode */
	err = mmc_movi_vendor_cmd(card, 0xEFAC62EC);
	if (err)
		goto out;

	err = mmc_movi_vendor_cmd(card, 0x00DECCEE);
	if (err)
		goto out;

#ifdef TEST_MMC_FW_PATCHING
	/* read and verify vendor cmd */
	/* enter vendor cmd */
	err = mmc_movi_vendor_cmd(card, 0xEFAC62EC);
	if (err)
		goto out;

	err = mmc_movi_vendor_cmd(card, 0x10210002);
	if (err)
		goto out;

	err = mmc_movi_erase_cmd(card, 0x0004DD9C, 0x00000004);
	if (err) {
		pr_err("Fail to Check WL value1\n");
		goto err_check_wl;
	}

	mmc_movi_read_cmd(card, (u8 *)buffer);
	pr_debug("buffer[0] is 0x%x\n", *(u8 *)buffer);

	err = mmc_movi_erase_cmd(card, 0x000379A4, 0x00000004);
	if (err) {
		pr_err("Fail to Check WL value2\n");
		goto err_check_wl;
	}

	mmc_movi_read_cmd(card, (u8 *)buffer);
	pr_debug("buffer[0] is 0x%x\n", *(u8 *)buffer);

err_check_wl:
	/* exit vendor cmd mode */
	err = mmc_movi_vendor_cmd(card, 0xEFAC62EC);
	if (err)
		goto out;

	err = mmc_movi_vendor_cmd(card, 0x00DECCEE);
	if (err)
		goto out;

#endif /* TEST_MMC_FW_PATCHING */

 out:
#ifdef TEST_MMC_FW_PATCHING
	kfree(buffer);
#endif /* TEST_MMC_FW_PATCHING */
	return err;
}

void mmc_fixup_samsung_fw(struct mmc_card *card)
{
	int err;

	mmc_claim_host(card->host);
	err = mmc_set_wearlevel_page(card);
	mmc_release_host(card->host);
	if (err)
		pr_err("%s : Failed to fixup Samsung emmc firmware(%d)\n",
			mmc_hostname(card->host), err);
}
