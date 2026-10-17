// SPDX-License-Identifier: GPL-2.0+
/*
 * Rockchip NAND Flash controller driver.
 * Copyright (C) 2021 Rockchip Inc.
 * Author: Yifeng Zhao <yifeng.zhao@rock-chips.com>
 */

#include <common.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <fdtdec.h>
#include <inttypes.h>
#include <linux/delay.h>
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/rawnand.h>
#include <memalign.h>
#include <nand.h>

/*
 * NFC Page Data Layout:
 *	1024 bytes data + 4Bytes sys data + 28Bytes~124Bytes ECC data +
 *	1024 bytes data + 4Bytes sys data + 28Bytes~124Bytes ECC data +
 *	......
 * NAND Page Data Layout:
 *	1024 * n data + m Bytes oob
 * Original Bad Block Mask Location:
 *	First byte of oob(spare).
 * nand_chip->oob_poi data layout:
 *	4Bytes sys data + .... + 4Bytes sys data + ECC data.
 */

/* NAND controller register definition */
#define NFC_READ			(0)
#define NFC_WRITE			(1)

#define NFC_FMCTL			(0x00)
#define   FMCTL_CE_SEL_M		0xFF
#define   FMCTL_CE_SEL(x)		(1 << (x))
#define   FMCTL_WP			BIT(8)
#define   FMCTL_RDY			BIT(9)

#define NFC_FMWAIT			(0x04)
#define   FLCTL_RST			BIT(0)
#define   FLCTL_WR			(1)	/* 0: read, 1: write */
#define   FLCTL_XFER_ST			BIT(2)
#define   FLCTL_XFER_EN			BIT(3)
#define   FLCTL_ACORRECT		BIT(10) /* Auto correct error bits. */
#define   FLCTL_XFER_READY		BIT(20)
#define   FLCTL_XFER_SECTOR		(22)
#define   FLCTL_TOG_FIX			BIT(29)

#define   BCHCTL_BANK_M			(7 << 5)
#define   BCHCTL_BANK			(5)

#define   DMA_ST			BIT(0)
#define   DMA_WR			(1)	/* 0: write, 1: read */
#define   DMA_EN			BIT(2)
#define   DMA_AHB_SIZE			(3)	/* 0: 1, 1: 2, 2: 4 */
#define   DMA_BURST_SIZE		(6)	/* 0: 1, 3: 4, 5: 8, 7: 16 */
#define   DMA_INC_NUM			(9)	/* 1 - 16 */

#define ECC_ERR_CNT(x, e) ((((x) >> (e).low) & (e).low_mask) |\
		(((x) >> (e).high) & (e).high_mask) << (e).low_bn)
#define   INT_DMA			BIT(0)
#define NFC_BANK			(0x800)
#define NFC_BANK_STEP			(0x100)
#define   BANK_DATA			(0x00)
#define   BANK_ADDR			(0x04)
#define   BANK_CMD			(0x08)
#define NFC_SRAM0			(0x1000)
#define NFC_SRAM1			(0x1400)
#define NFC_SRAM_SIZE			(0x400)
#define NFC_TIMEOUT_MS			(500)
#define NFC_MAX_OOB_PER_STEP		128
#define NFC_MIN_OOB_PER_STEP		64
#define MAX_DATA_SIZE			0xFFFC
#define MAX_ADDRESS_CYC			6
#define NFC_ECC_MAX_MODES		4
#define NFC_RB_DELAY_US			50
#define NFC_MAX_PAGE_SIZE		(16 * 1024)
#define NFC_MAX_OOB_SIZE		(16 * 128)
#define NFC_MAX_NSELS			(8) /* Some Socs only have 1 or 2 CSs. */
#define NFC_SYS_DATA_SIZE		(4) /* 4 bytes sys data in oob pre 1024 data.*/
#define RK_DEFAULT_CLOCK_RATE		(150 * 1000 * 1000) /* 150 Mhz */
#define ACCTIMING(csrw, rwpw, rwcs)	((csrw) << 12 | (rwpw) << 5 | (rwcs))

enum nfc_type {
	NFC_V6,
	NFC_V8,
	NFC_V9,
};

/**
 * struct rk_ecc_cnt_status: represent a ecc status data.
 * @err_flag_bit: error flag bit index at register.
 * @low: ECC count low bit index at register.
 * @low_mask: mask bit.
 * @low_bn: ECC count low bit number.
 * @high: ECC count high bit index at register.
 * @high_mask: mask bit
 */
struct ecc_cnt_status {
	u8 err_flag_bit;
	u8 low;
	u8 low_mask;
	u8 low_bn;
	u8 high;
	u8 high_mask;
};

/**
 * @type: NFC version
 * @ecc_strengths: ECC strengths
 * @ecc_cfgs: ECC config values
 * @flctl_off: FLCTL register offset
 * @bchctl_off: BCHCTL register offset
 * @dma_data_buf_off: DMA_DATA_BUF register offset
 * @dma_oob_buf_off: DMA_OOB_BUF register offset
 * @dma_cfg_off: DMA_CFG register offset
 * @dma_st_off: DMA_ST register offset
 * @bch_st_off: BCG_ST register offset
 * @randmz_off: RANDMZ register offset
 * @int_en_off: interrupt enable register offset
 * @int_clr_off: interrupt clean register offset
 * @int_st_off: interrupt status register offset
 * @oob0_off: oob0 register offset
 * @oob1_off: oob1 register offset
 * @ecc0: represent ECC0 status data
 * @ecc1: represent ECC1 status data
 */
struct nfc_cfg {
	enum nfc_type type;
	u8 ecc_strengths[NFC_ECC_MAX_MODES];
	u32 ecc_cfgs[NFC_ECC_MAX_MODES];
	u32 flctl_off;
	u32 bchctl_off;
	u32 dma_cfg_off;
	u32 dma_data_buf_off;
	u32 dma_oob_buf_off;
	u32 dma_st_off;
	u32 bch_st_off;
	u32 randmz_off;
	u32 int_en_off;
	u32 int_clr_off;
	u32 int_st_off;
	u32 oob0_off;
	u32 oob1_off;
	struct ecc_cnt_status ecc0;
	struct ecc_cnt_status ecc1;
};

struct rk_nfc_nand_chip {
	struct nand_chip chip;

	u16 boot_blks;
	u16 metadata_size;
	u32 boot_ecc;
	u32 timing;

	u8 nsels;
	u8 sels[0];
	/* Nothing after this field. */
};

struct rk_nfc {
	struct nand_hw_control controller;
	const struct nfc_cfg *cfg;
	struct udevice *dev;

	struct clk *nfc_clk;
	struct clk *ahb_clk;
	void __iomem *regs;

	int selected_bank;
	u32 band_offset;
	u32 cur_ecc;
	u32 cur_timing;

	u8 *page_buf;
	u32 *oob_buf;

	unsigned long assigned_cs;
};

static inline struct rk_nfc_nand_chip *rk_nfc_to_rknand(struct nand_chip *chip)
{
	return container_of(chip, struct rk_nfc_nand_chip, chip);
}

static inline u8 *rk_nfc_buf_to_data_ptr(struct nand_chip *chip, const u8 *p, int i)
{
	return (u8 *)p + i * chip->ecc.size;
}

static inline u8 *rk_nfc_buf_to_oob_ptr(struct nand_chip *chip, int i)
{
	u8 *poi;

	poi = chip->oob_poi + i * NFC_SYS_DATA_SIZE;

	return poi;
}

static inline u8 *rk_nfc_buf_to_oob_ecc_ptr(struct nand_chip *chip, int i)
{
	struct rk_nfc_nand_chip *rknand = rk_nfc_to_rknand(chip);
	u8 *poi;

	poi = chip->oob_poi + rknand->metadata_size + chip->ecc.bytes * i;

	return poi;
}

static inline int rk_nfc_data_len(struct nand_chip *chip)
{
	return chip->ecc.size + chip->ecc.bytes + NFC_SYS_DATA_SIZE;
}

static inline u8 *rk_nfc_data_ptr(struct nand_chip *chip, int i)
{
	struct rk_nfc *nfc = nand_get_controller_data(chip);

	return nfc->page_buf + i * rk_nfc_data_len(chip);
}

static inline u8 *rk_nfc_oob_ptr(struct nand_chip *chip, int i)
{
	struct rk_nfc *nfc = nand_get_controller_data(chip);

	return nfc->page_buf + i * rk_nfc_data_len(chip) + chip->ecc.size;
}

static int rk_nfc_hw_ecc_setup(struct nand_chip *chip, u32 strength)
{
	struct rk_nfc *nfc = nand_get_controller_data(chip);
	u32 reg, i;

	for (i = 0; i < NFC_ECC_MAX_MODES; i++) {
		if (strength == nfc->cfg->ecc_strengths[i]) {
			reg = nfc->cfg->ecc_cfgs[i];
			break;
		}
	}

	if (i >= NFC_ECC_MAX_MODES)
		return -EINVAL;

	writel(reg, nfc->regs + nfc->cfg->bchctl_off);

	/* Save chip ECC setting */
	nfc->cur_ecc = strength;

	return 0;
}

static void rk_nfc_select_chip(struct mtd_info *mtd, int cs)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct rk_nfc *nfc = nand_get_controller_data(chip);
	struct rk_nfc_nand_chip *rknand = rk_nfc_to_rknand(chip);
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	u32 val;

	if (cs < 0) {
		nfc->selected_bank = -1;
		/* Deselect the currently selected target. */
		val = readl(nfc->regs + NFC_FMCTL);
		val &= ~FMCTL_CE_SEL_M;
		writel(val, nfc->regs + NFC_FMCTL);
		return;
	}

	nfc->selected_bank = rknand->sels[cs];
	nfc->band_offset = NFC_BANK + nfc->selected_bank * NFC_BANK_STEP;

	val = readl(nfc->regs + NFC_FMCTL);
	val &= ~FMCTL_CE_SEL_M;
	val |= FMCTL_CE_SEL(nfc->selected_bank);

	writel(val, nfc->regs + NFC_FMCTL);

	/*
	 * Compare current chip timing with selected chip timing and
	 * change if needed.
	 */
	if (nfc->cur_timing != rknand->timing) {
		writel(rknand->timing, nfc->regs + NFC_FMWAIT);
		nfc->cur_timing = rknand->timing;
	}

	/*
	 * Compare current chip ECC setting with selected chip ECC setting and
	 * change if needed.
	 */
	if (nfc->cur_ecc != ecc->strength)
		rk_nfc_hw_ecc_setup(chip, ecc->strength);
}

static inline int rk_nfc_wait_ioready(struct rk_nfc *nfc)
{
	u32 timeout = (CONFIG_SYS_HZ * NFC_TIMEOUT_MS) / 1000;
	u32 time_start;

	time_start = get_timer(0);
	do {
		if (readl(nfc->regs + NFC_FMCTL) & FMCTL_RDY)
			return 0;
	} while (get_timer(time_start) < timeout);

	dev_err(nfc->dev, "wait for io ready timedout\n");
	return -ETIMEDOUT;
}

static void rk_nfc_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct rk_nfc *nfc = nand_get_controller_data(chip);
	void __iomem *bank_base;
	int i = 0;

	bank_base = nfc->regs + nfc->band_offset + BANK_DATA;

	for (i = 0; i < len; i++)
		buf[i] = readl(bank_base);
}

static void rk_nfc_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct rk_nfc *nfc = nand_get_controller_data(chip);
	void __iomem *bank_base;
	int i = 0;

	bank_base = nfc->regs + nfc->band_offset + BANK_DATA;

	for (i = 0; i < len; i++)
		writel(buf[i], bank_base);
}

static void rk_nfc_cmd(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct rk_nfc *nfc = nand_get_controller_data(chip);
	void __iomem *bank_base;

	bank_base = nfc->regs + nfc->band_offset;

	if (ctrl & NAND_CTRL_CHANGE) {
		if (ctrl & NAND_ALE)
			bank_base += BANK_ADDR;
		else if (ctrl & NAND_CLE)
			bank_base += BANK_CMD;
		chip->IO_ADDR_W = bank_base;
	}

	if (dat != NAND_CMD_NONE)
		writel(dat & 0xFF, chip->IO_ADDR_W);
}

static uint8_t rockchip_nand_read_byte(struct mtd_info *mtd)
{
	uint8_t ret;

	rk_nfc_read_buf(mtd, &ret, 1);

	return ret;
}

static int rockchip_nand_dev_ready(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct rk_nfc *nfc = nand_get_controller_data(chip);

	if (readl(nfc->regs + NFC_FMCTL) & FMCTL_RDY)
		return 1;

	return 0;
}

static void rk_nfc_xfer_start(struct rk_nfc *nfc, u8 rw, u8 n_KB,
			      dma_addr_t dma_data, dma_addr_t dma_oob)
{
	u32 dma_reg, fl_reg, bch_reg;

	dma_reg = DMA_ST | ((!rw) << DMA_WR) | DMA_EN | (2 << DMA_AHB_SIZE) |
	      (7 << DMA_BURST_SIZE) | (16 << DMA_INC_NUM);

	fl_reg = (rw << FLCTL_WR) | FLCTL_XFER_EN | FLCTL_ACORRECT |
		 (n_KB << FLCTL_XFER_SECTOR) | FLCTL_TOG_FIX;

	if (nfc->cfg->type == NFC_V6 || nfc->cfg->type == NFC_V8) {
		bch_reg = readl_relaxed(nfc->regs + nfc->cfg->bchctl_off);
		bch_reg = (bch_reg & (~BCHCTL_BANK_M)) |
			  (nfc->selected_bank << BCHCTL_BANK);
		writel(bch_reg, nfc->regs + nfc->cfg->bchctl_off);
	}

	writel(dma_reg, nfc->regs + nfc->cfg->dma_cfg_off);
	writel((u32)dma_data, nfc->regs + nfc->cfg->dma_data_buf_off);
	writel((u32)dma_oob, nfc->regs + nfc->cfg->dma_oob_buf_off);
	writel(fl_reg, nfc->regs + nfc->cfg->flctl_off);
	fl_reg |= FLCTL_XFER_ST;
	writel(fl_reg, nfc->regs + nfc->cfg->flctl_off);
}

static int rk_nfc_wait_for_xfer_done(struct rk_nfc *nfc)
{
	unsigned long timeout = (CONFIG_SYS_HZ * NFC_TIMEOUT_MS) / 1000;
	void __iomem *ptr = nfc->regs + nfc->cfg->flctl_off;
	u32 time_start;

	time_start = get_timer(0);

	do {
		if (readl(ptr) & FLCTL_XFER_READY)
			return 0;
	} while (get_timer(time_start) < timeout);

	dev_err(nfc->dev, "wait for io ready timedout\n");
	return -ETIMEDOUT;
}

static int rk_nfc_write_page_raw(struct mtd_info *mtd,
				 struct nand_chip *chip,
				 const u8 *buf,
				 int oob_required,
				 int page)
{
	struct rk_nfc_nand_chip *rknand = rk_nfc_to_rknand(chip);
	struct rk_nfc *nfc = nand_get_controller_data(chip);
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	int i, pages_per_blk;

	pages_per_blk = mtd->erasesize / mtd->writesize;
	if ((page < (pages_per_blk * rknand->boot_blks)) &&
	    rknand->boot_ecc != ecc->strength) {
		/*
		 * There's currently no method to notify the MTD framework that
		 * a different ECC strength is in use for the boot blocks.
		 */
		return -EIO;
	}

	if (!buf)
		memset(nfc->page_buf, 0xff, mtd->writesize + mtd->oobsize);

	for (i = 0; i < ecc->steps; i++) {
		/* Copy data to the NFC buffer. */
		if (buf)
			memcpy(rk_nfc_data_ptr(chip, i),
			       rk_nfc_buf_to_data_ptr(chip, buf, i),
			       ecc->size);
		/*
		 * The first four bytes of OOB are reserved for the
		 * boot ROM. In some debugging cases, such as with a
		 * read, erase and write back test these 4 bytes stored
		 * in OOB also need to be written back.
		 *
		 * The function nand_block_bad detects bad blocks like:
		 *
		 * bad = chip->oob_poi[chip->badblockpos];
		 *
		 * chip->badblockpos == 0 for a large page NAND Flash,
		 * so chip->oob_poi[0] is the bad block mask (BBM).
		 *
		 * The OOB data layout on the NFC is:
		 *
		 *    PA0  PA1  PA2  PA3  | BBM OOB1 OOB2 OOB3 | ...
		 *
		 * or
		 *
		 *    0xFF 0xFF 0xFF 0xFF | BBM OOB1 OOB2 OOB3 | ...
		 *
		 * The code here just swaps the first 4 bytes with the last
		 * 4 bytes without losing any data.
		 *
		 * The chip->oob_poi data layout:
		 *
		 *    BBM  OOB1 OOB2 OOB3 |......|  PA0  PA1  PA2  PA3
		 *
		 * The oobfree structure already has reserved these 4 bytes
		 * together with 2 bytes for BBM by reducing it's length:
		 *
		 * oobfree[0].length = rknand->metadata_size - NFC_SYS_DATA_SIZE - 2;
		 */
		if (!i)
			memcpy(rk_nfc_oob_ptr(chip, i),
			       rk_nfc_buf_to_oob_ptr(chip, ecc->steps - 1),
			       NFC_SYS_DATA_SIZE);
		else
			memcpy(rk_nfc_oob_ptr(chip, i),
			       rk_nfc_buf_to_oob_ptr(chip, i - 1),
			       NFC_SYS_DATA_SIZE);
		/* Copy ECC data to the NFC buffer. */
		memcpy(rk_nfc_oob_ptr(chip, i) + NFC_SYS_DATA_SIZE,
		       rk_nfc_buf_to_oob_ecc_ptr(chip, i),
		       ecc->bytes);
	}

	nand_prog_page_begin_op(chip, page, 0, NULL, 0);
	rk_nfc_write_buf(mtd, buf, mtd->writesize + mtd->oobsize);
	return nand_prog_page_end_op(chip);
}

static int rk_nfc_write_page_hwecc(struct mtd_info *mtd,
				   struct nand_chip *chip,
				   const u8 *buf,
				   int oob_required,
				   int page)
{
	struct rk_nfc *nfc = nand_get_controller_data(chip);
	struct rk_nfc_nand_chip *rknand = rk_nfc_to_rknand(chip);
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	int oob_step = (ecc->bytes > 60) ? NFC_MAX_OOB_PER_STEP :
			NFC_MIN_OOB_PER_STEP;
	int pages_per_blk = mtd->erasesize / mtd->writesize;
	int ret = 0, i, boot_rom_mode = 0;
	dma_addr_t dma_data, dma_oob;
	u32 tmp;
	u8 *oob;

	nand_prog_page_begin_op(chip, page, 0, NULL, 0);

	if (buf)
		memcpy(nfc->page_buf, buf, mtd->writesize);
	else
		memset(nfc->page_buf, 0xFF, mtd->writesize);

	/*
	 * The first blocks (4, 8 or 16 depending on the device) are used
	 * by the boot ROM and the first 32 bits of OOB need to link to
	 * the next page address in the same block. We can't directly copy
	 * OOB data from the MTD framework, because this page address
	 * conflicts for example with the bad block marker (BBM),
	 * so we shift all OOB data including the BBM with 4 byte positions.
	 * As a consequence the OOB size available to the MTD framework is
	 * also reduced with 4 bytes.
	 *
	 *    PA0  PA1  PA2  PA3 | BBM OOB1 OOB2 OOB3 | ...
	 *
	 * If a NAND is not a boot medium or the page is not a boot block,
	 * the first 4 bytes are left untouched by writing 0xFF to them.
	 *
	 *   0xFF 0xFF 0xFF 0xFF | BBM OOB1 OOB2 OOB3 | ...
	 *
	 * The code here just swaps the first 4 bytes with the last
	 * 4 bytes without losing any data.
	 *
	 * The chip->oob_poi data layout:
	 *
	 *    BBM  OOB1 OOB2 OOB3 |......|  PA0  PA1  PA2  PA3
	 *
	 * Configure the ECC algorithm supported by the boot ROM.
	 */
	if (page < (pages_per_blk * rknand->boot_blks)) {
		boot_rom_mode = 1;
		if (rknand->boot_ecc != ecc->strength)
			rk_nfc_hw_ecc_setup(chip, rknand->boot_ecc);
	}

	for (i = 0; i < ecc->steps; i++) {
		if (!i)
			oob = chip->oob_poi + (ecc->steps - 1) * NFC_SYS_DATA_SIZE;
		else
			oob = chip->oob_poi + (i - 1) * NFC_SYS_DATA_SIZE;

		tmp = oob[0] | oob[1] << 8 | oob[2] << 16 | oob[3] << 24;

		if (nfc->cfg->type == NFC_V9)
			nfc->oob_buf[i] = tmp;
		else
			nfc->oob_buf[i * (oob_step / 4)] = tmp;
	}

	dma_data = dma_map_single((void *)nfc->page_buf,
				  mtd->writesize, DMA_TO_DEVICE);
	dma_oob = dma_map_single(nfc->oob_buf,
				 ecc->steps * oob_step,
				 DMA_TO_DEVICE);

	rk_nfc_xfer_start(nfc, NFC_WRITE, ecc->steps, dma_data,
			  dma_oob);
	ret = rk_nfc_wait_for_xfer_done(nfc);

	dma_unmap_single(dma_data, mtd->writesize,
			 DMA_TO_DEVICE);
	dma_unmap_single(dma_oob, ecc->steps * oob_step,
			 DMA_TO_DEVICE);

	if (boot_rom_mode && rknand->boot_ecc != ecc->strength)
		rk_nfc_hw_ecc_setup(chip, ecc->strength);

	if (ret) {
		dev_err(nfc->dev, "write: wait transfer done timeout.\n");
		return -ETIMEDOUT;
	}

	return nand_prog_page_end_op(chip);
}

static int rk_nfc_write_oob(struct mtd_info *mtd,
			    struct nand_chip *chip, int page)
{
	return rk_nfc_write_page_hwecc(mtd, chip, NULL, 1, page);
}

static int rk_nfc_read_page_raw(struct mtd_info *mtd,
				struct nand_chip *chip,
				u8 *buf,
				int oob_required,
				int page)
{
	struct rk_nfc_nand_chip *rknand = rk_nfc_to_rknand(chip);
	struct rk_nfc *nfc = nand_get_controller_data(chip);
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	int i, pages_per_blk;

	pages_per_blk = mtd->erasesize / mtd->writesize;
	if ((page < (pages_per_blk * rknand->boot_blks)) &&
	    nfc->selected_bank == 0 &&
	    rknand->boot_ecc != ecc->strength) {
		/*
		 * There's currently no method to notify the MTD framework that
		 * a different ECC strength is in use for the boot blocks.
		 */
		return -EIO;
	}

	nand_read_page_op(chip, page, 0, NULL, 0);
	rk_nfc_read_buf(mtd, nfc->page_buf, mtd->writesize + mtd->oobsize);
	for (i = 0; i < ecc->steps; i++) {
		/*
		 * The first four bytes of OOB are reserved for the
		 * boot ROM. In some debugging cases, such as with a read,
		 * erase and write back test, these 4 bytes also must be
		 * saved somewhere, otherwise this information will be
		 * lost during a write back.
		 */
		if (!i)
			memcpy(rk_nfc_buf_to_oob_ptr(chip, ecc->steps - 1),
			       rk_nfc_oob_ptr(chip, i),
			       NFC_SYS_DATA_SIZE);
		else
			memcpy(rk_nfc_buf_to_oob_ptr(chip, i - 1),
			       rk_nfc_oob_ptr(chip, i),
			       NFC_SYS_DATA_SIZE);

		/* Copy ECC data from the NFC buffer. */
		memcpy(rk_nfc_buf_to_oob_ecc_ptr(chip, i),
		       rk_nfc_oob_ptr(chip, i) + NFC_SYS_DATA_SIZE,
		       ecc->bytes);

		/* Copy data from the NFC buffer. */
		if (buf)
			memcpy(rk_nfc_buf_to_data_ptr(chip, buf, i),
			       rk_nfc_data_ptr(chip, i),
			       ecc->size);
	}

	return 0;
}

static int rk_nfc_read_page_hwecc(struct mtd_info *mtd,
				  struct nand_chip *chip,
				  u8 *buf,
				  int oob_required,
				  int page)
{
	struct rk_nfc *nfc = nand_get_controller_data(chip);
	struct rk_nfc_nand_chip *rknand = rk_nfc_to_rknand(chip);
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	int oob_step = (ecc->bytes > 60) ? NFC_MAX_OOB_PER_STEP :
			NFC_MIN_OOB_PER_STEP;
	int pages_per_blk = mtd->erasesize / mtd->writesize;
	dma_addr_t dma_data, dma_oob;
	int ret = 0, i, cnt, boot_rom_mode = 0;
	int max_bitflips = 0, bch_st, ecc_fail = 0;
	u8 *oob;
	u32 tmp;

	nand_read_page_op(chip, page, 0, NULL, 0);

	dma_data = dma_map_single(nfc->page_buf,
				  mtd->writesize,
				  DMA_FROM_DEVICE);
	dma_oob = dma_map_single(nfc->oob_buf,
				 ecc->steps * oob_step,
				 DMA_FROM_DEVICE);

	/*
	 * The first blocks (4, 8 or 16 depending on the device)
	 * are used by the boot ROM.
	 * Configure the ECC algorithm supported by the boot ROM.
	 */
	if (page < (pages_per_blk * rknand->boot_blks) &&
	    nfc->selected_bank == 0) {
		boot_rom_mode = 1;
		if (rknand->boot_ecc != ecc->strength)
			rk_nfc_hw_ecc_setup(chip, rknand->boot_ecc);
	}

	rk_nfc_xfer_start(nfc, NFC_READ, ecc->steps, dma_data,
			  dma_oob);
	ret = rk_nfc_wait_for_xfer_done(nfc);

	dma_unmap_single(dma_data, mtd->writesize,
			 DMA_FROM_DEVICE);
	dma_unmap_single(dma_oob, ecc->steps * oob_step,
			 DMA_FROM_DEVICE);

	if (ret) {
		ret = -ETIMEDOUT;
		dev_err(nfc->dev, "read: wait transfer done timeout.\n");
		goto timeout_err;
	}

	for (i = 0; i < ecc->steps; i++) {
		if (!i)
			oob = chip->oob_poi + (ecc->steps - 1) * NFC_SYS_DATA_SIZE;
		else
			oob = chip->oob_poi + (i - 1) * NFC_SYS_DATA_SIZE;

		if (nfc->cfg->type == NFC_V9)
			tmp = nfc->oob_buf[i];
		else
			tmp = nfc->oob_buf[i * (oob_step / 4)];

		*oob++ = (u8)tmp;
		*oob++ = (u8)(tmp >> 8);
		*oob++ = (u8)(tmp >> 16);
		*oob++ = (u8)(tmp >> 24);
	}

	for (i = 0; i < (ecc->steps / 2); i++) {
		bch_st = readl_relaxed(nfc->regs +
				       nfc->cfg->bch_st_off + i * 4);
		if (bch_st & BIT(nfc->cfg->ecc0.err_flag_bit) ||
		    bch_st & BIT(nfc->cfg->ecc1.err_flag_bit)) {
			mtd->ecc_stats.failed++;
			ecc_fail = 1;
		} else {
			cnt = ECC_ERR_CNT(bch_st, nfc->cfg->ecc0);
			mtd->ecc_stats.corrected += cnt;
			max_bitflips = max_t(u32, max_bitflips, cnt);

			cnt = ECC_ERR_CNT(bch_st, nfc->cfg->ecc1);
			mtd->ecc_stats.corrected += cnt;
			max_bitflips = max_t(u32, max_bitflips, cnt);
		}
	}

	if (buf)
		memcpy(buf, nfc->page_buf, mtd->writesize);

timeout_err:
	if (boot_rom_mode && rknand->boot_ecc != ecc->strength)
		rk_nfc_hw_ecc_setup(chip, ecc->strength);

	if (ret)
		return ret;

	if (ecc_fail) {
		dev_err(nfc->dev, "read page: %x ecc error!\n", page);
		return 0;
	}

	return max_bitflips;
}

static int rk_nfc_read_oob(struct mtd_info *mtd,
			   struct nand_chip *chip, int page)
{
	return rk_nfc_read_page_hwecc(mtd, chip, NULL, 1, page);
}

static inline void rk_nfc_hw_init(struct rk_nfc *nfc)
{
	/* Disable flash wp. */
	writel(FMCTL_WP, nfc->regs + NFC_FMCTL);
	/* Config default timing 40ns at 150 Mhz NFC clock. */
	writel(0x1081, nfc->regs + NFC_FMWAIT);
	nfc->cur_timing = 0x1081;
	/* Disable randomizer and DMA. */
	writel(0, nfc->regs + nfc->cfg->randmz_off);
	writel(0, nfc->regs + nfc->cfg->dma_cfg_off);
	writel(FLCTL_RST, nfc->regs + nfc->cfg->flctl_off);
}

static int rk_nfc_enable_clks(struct udevice *dev, struct rk_nfc *nfc)
{
	int ret;

	if (!IS_ERR(nfc->nfc_clk)) {
		ret = clk_prepare_enable(nfc->nfc_clk);
		if (ret)
			dev_err(dev, "failed to enable NFC clk\n");
	}

	ret = clk_prepare_enable(nfc->ahb_clk);
	if (ret) {
		dev_err(dev, "failed to enable ahb clk\n");
		if (!IS_ERR(nfc->nfc_clk))
			clk_disable_unprepare(nfc->nfc_clk);
	}

	return 0;
}

static void rk_nfc_disable_clks(struct rk_nfc *nfc)
{
	if (!IS_ERR(nfc->nfc_clk))
		clk_disable_unprepare(nfc->nfc_clk);
	clk_disable_unprepare(nfc->ahb_clk);
}

static int rk_nfc_ecc_init(struct rk_nfc *nfc, struct nand_chip *chip)
{
	struct rk_nfc_nand_chip *rknand = rk_nfc_to_rknand(chip);
	const u8 *strengths = nfc->cfg->ecc_strengths;
	struct mtd_info *mtd = nand_to_mtd(chip);
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	u8 max_strength, nfc_max_strength;
	int i;

	nfc_max_strength = nfc->cfg->ecc_strengths[0];
	/* If optional dt settings not present. */
	if (!ecc->size || !ecc->strength ||
	    ecc->strength > nfc_max_strength) {
		chip->ecc.size = 1024;
		ecc->steps = mtd->writesize / ecc->size;

		/*
		 * HW ECC always requests the number of ECC bytes per 1024 byte
		 * blocks. The first 4 OOB bytes are reserved for sys data.
		 */
		max_strength = ((mtd->oobsize / ecc->steps) - 4) * 8 /
				 fls(8 * 1024);
		if (max_strength > nfc_max_strength)
			max_strength = nfc_max_strength;

		for (i = 0; i < 4; i++) {
			if (max_strength >= strengths[i])
				break;
		}

		if (i >= 4) {
			dev_err(nfc->dev, "unsupported ECC strength\n");
			return -EOPNOTSUPP;
		}

		ecc->strength = strengths[i];
	}
	ecc->steps = mtd->writesize / ecc->size;
	ecc->bytes = DIV_ROUND_UP(ecc->strength * fls(8 * chip->ecc.size), 8);

	if (ecc->bytes * ecc->steps > mtd->oobsize - rknand->metadata_size)
		return -EINVAL;

	ecc->layout = kzalloc(sizeof(*ecc->layout), GFP_KERNEL);
	if (!ecc->layout)
		return -ENOMEM;

	ecc->layout->eccbytes = ecc->bytes * ecc->steps;

	for (i = 0; i < ecc->layout->eccbytes; i++)
		ecc->layout->eccpos[i] = rknand->metadata_size + i;

	ecc->layout->oobfree[0].length = rknand->metadata_size - NFC_SYS_DATA_SIZE - 2;
	ecc->layout->oobfree[0].offset = 2;

	return 0;
}

static int rk_nfc_nand_chip_init(ofnode node, struct rk_nfc *nfc, int devnum)
{
	struct rk_nfc_nand_chip *rknand;
	struct udevice *dev = nfc->dev;
	struct nand_ecc_ctrl *ecc;
	struct nand_chip *chip;
	struct mtd_info *mtd;
	u32 cs[NFC_MAX_NSELS];
	int nsels, i, ret;
	u32 tmp;

	if (!ofnode_get_property(node, "reg", &nsels))
		return -ENODEV;
	nsels /= sizeof(u32);
	if (!nsels || nsels > NFC_MAX_NSELS) {
		dev_err(dev, "invalid reg property size %d\n", nsels);
		return -EINVAL;
	}

	rknand = kzalloc(sizeof(*rknand) + nsels * sizeof(u8), GFP_KERNEL);
	if (!rknand)
		return -ENOMEM;

	rknand->nsels = nsels;
	rknand->timing = nfc->cur_timing;

	ret = ofnode_read_u32_array(node, "reg", cs, nsels);
	if (ret < 0) {
		dev_err(dev, "Could not retrieve reg property\n");
		return -EINVAL;
	}

	for (i = 0; i < nsels; i++) {
		if (cs[i] >= NFC_MAX_NSELS) {
			dev_err(dev, "invalid CS: %u\n", cs[i]);
			return -EINVAL;
		}

		if (test_and_set_bit(cs[i], &nfc->assigned_cs)) {
			dev_err(dev, "CS %u already assigned\n", cs[i]);
			return -EINVAL;
		}

		rknand->sels[i] = cs[i];
	}

	chip = &rknand->chip;
	ecc = &chip->ecc;
	ecc->mode = NAND_ECC_HW_SYNDROME;

	ret = ofnode_read_u32(node, "nand-ecc-strength", &tmp);
	ecc->strength = ret ? 0 : tmp;

	ret = ofnode_read_u32(node, "nand-ecc-step-size", &tmp);
	ecc->size = ret ? 0 : tmp;

	mtd = nand_to_mtd(chip);
	mtd->owner = THIS_MODULE;
	mtd->dev->parent = dev;

	nand_set_controller_data(chip, nfc);

	chip->flash_node = node;
	chip->chip_delay = NFC_RB_DELAY_US;
	chip->select_chip = rk_nfc_select_chip;
	chip->cmd_ctrl = rk_nfc_cmd;
	chip->read_buf = rk_nfc_read_buf;
	chip->write_buf = rk_nfc_write_buf;
	chip->read_byte = rockchip_nand_read_byte;
	chip->dev_ready = rockchip_nand_dev_ready;
	chip->controller = &nfc->controller;

	chip->bbt_options = NAND_BBT_USE_FLASH | NAND_BBT_NO_OOB;
	chip->options |= NAND_NO_SUBPAGE_WRITE | NAND_USE_BOUNCE_BUFFER;

	if (IS_ENABLED(CONFIG_ROCKCHIP_NAND_SKIP_BBTSCAN))
		chip->options |= NAND_SKIP_BBTSCAN;

	rk_nfc_hw_init(nfc);
	ret = nand_scan_ident(mtd, nsels, NULL);
	if (ret)
		return ret;

	ret = rk_nfc_ecc_init(nfc, chip);
	if (ret) {
		dev_err(dev, "rk_nfc_ecc_init failed: %d\n", ret);
		return ret;
	}

	ret = ofnode_read_u32(node, "rockchip,boot-blks", &tmp);
	rknand->boot_blks = ret ? 0 : tmp;

	ret = ofnode_read_u32(node, "rockchip,boot-ecc-strength", &tmp);
	rknand->boot_ecc = ret ? ecc->strength : tmp;

	rknand->metadata_size = NFC_SYS_DATA_SIZE * ecc->steps;

	if (rknand->metadata_size < NFC_SYS_DATA_SIZE + 2) {
		dev_err(dev,
			"driver needs at least %d bytes of meta data\n",
			NFC_SYS_DATA_SIZE + 2);
		return -EIO;
	}

	if (!nfc->page_buf) {
		nfc->page_buf = kzalloc(NFC_MAX_PAGE_SIZE, GFP_KERNEL);
		if (!nfc->page_buf) {
			kfree(ecc->layout);
			return -ENOMEM;
		}
	}

	if (!nfc->oob_buf) {
		nfc->oob_buf = kzalloc(NFC_MAX_OOB_SIZE, GFP_KERNEL);
		if (!nfc->oob_buf) {
			kfree(ecc->layout);
			kfree(nfc->page_buf);
			nfc->page_buf = NULL;
			return -ENOMEM;
		}
	}

	ecc->read_page = rk_nfc_read_page_hwecc;
	ecc->read_page_raw = rk_nfc_read_page_raw;
	ecc->read_oob = rk_nfc_read_oob;
	ecc->write_page = rk_nfc_write_page_hwecc;
	ecc->write_page_raw = rk_nfc_write_page_raw;
	ecc->write_oob = rk_nfc_write_oob;

	ret = nand_scan_tail(mtd);
	if (ret) {
		dev_err(dev, "nand_scan_tail failed: %d\n", ret);
		return ret;
	}

	return nand_register(devnum, mtd);
}

static int rk_nfc_nand_chips_init(struct udevice *dev, struct rk_nfc *nfc)
{
	int ret, i = 0;
	ofnode child;

	ofnode_for_each_subnode(child, dev_ofnode(dev)) {
		ret = rk_nfc_nand_chip_init(child, nfc, i++);
		if (ret)
			return ret;
	}

	return 0;
}

static struct nfc_cfg nfc_v6_cfg = {
		.type			= NFC_V6,
		.ecc_strengths		= {60, 40, 24, 16},
		.ecc_cfgs		= {
			0x00040011, 0x00040001, 0x00000011, 0x00000001,
		},
		.flctl_off		= 0x08,
		.bchctl_off		= 0x0C,
		.dma_cfg_off		= 0x10,
		.dma_data_buf_off	= 0x14,
		.dma_oob_buf_off	= 0x18,
		.dma_st_off		= 0x1C,
		.bch_st_off		= 0x20,
		.randmz_off		= 0x150,
		.int_en_off		= 0x16C,
		.int_clr_off		= 0x170,
		.int_st_off		= 0x174,
		.oob0_off		= 0x200,
		.oob1_off		= 0x230,
		.ecc0			= {
			.err_flag_bit	= 2,
			.low		= 3,
			.low_mask	= 0x1F,
			.low_bn		= 5,
			.high		= 27,
			.high_mask	= 0x1,
		},
		.ecc1			= {
			.err_flag_bit	= 15,
			.low		= 16,
			.low_mask	= 0x1F,
			.low_bn		= 5,
			.high		= 29,
			.high_mask	= 0x1,
		},
};

static struct nfc_cfg nfc_v8_cfg = {
		.type			= NFC_V8,
		.ecc_strengths		= {16, 16, 16, 16},
		.ecc_cfgs		= {
			0x00000001, 0x00000001, 0x00000001, 0x00000001,
		},
		.flctl_off		= 0x08,
		.bchctl_off		= 0x0C,
		.dma_cfg_off		= 0x10,
		.dma_data_buf_off	= 0x14,
		.dma_oob_buf_off	= 0x18,
		.dma_st_off		= 0x1C,
		.bch_st_off		= 0x20,
		.randmz_off		= 0x150,
		.int_en_off		= 0x16C,
		.int_clr_off		= 0x170,
		.int_st_off		= 0x174,
		.oob0_off		= 0x200,
		.oob1_off		= 0x230,
		.ecc0			= {
			.err_flag_bit	= 2,
			.low		= 3,
			.low_mask	= 0x1F,
			.low_bn		= 5,
			.high		= 27,
			.high_mask	= 0x1,
		},
		.ecc1			= {
			.err_flag_bit	= 15,
			.low		= 16,
			.low_mask	= 0x1F,
			.low_bn		= 5,
			.high		= 29,
			.high_mask	= 0x1,
		},
};

static struct nfc_cfg nfc_v9_cfg = {
		.type			= NFC_V9,
		.ecc_strengths		= {70, 60, 40, 16},
		.ecc_cfgs		= {
			0x00000001, 0x06000001, 0x04000001, 0x02000001,
		},
		.flctl_off		= 0x10,
		.bchctl_off		= 0x20,
		.dma_cfg_off		= 0x30,
		.dma_data_buf_off	= 0x34,
		.dma_oob_buf_off	= 0x38,
		.dma_st_off		= 0x3C,
		.bch_st_off		= 0x150,
		.randmz_off		= 0x208,
		.int_en_off		= 0x120,
		.int_clr_off		= 0x124,
		.int_st_off		= 0x128,
		.oob0_off		= 0x200,
		.oob1_off		= 0x204,
		.ecc0			= {
			.err_flag_bit	= 2,
			.low		= 3,
			.low_mask	= 0x7F,
			.low_bn		= 7,
			.high		= 0,
			.high_mask	= 0x0,
		},
		.ecc1			= {
			.err_flag_bit	= 18,
			.low		= 19,
			.low_mask	= 0x7F,
			.low_bn		= 7,
			.high		= 0,
			.high_mask	= 0x0,
		},
};

static const struct udevice_id rk_nfc_id_table[] = {
	{
		.compatible = "rockchip,px30-nfc",
		.data = (unsigned long)&nfc_v9_cfg
	},
	{
		.compatible = "rockchip,rk2928-nfc",
		.data = (unsigned long)&nfc_v6_cfg
	},
	{
		.compatible = "rockchip,rv1108-nfc",
		.data = (unsigned long)&nfc_v8_cfg
	},
	{ /* sentinel */ }
};

static int rk_nfc_probe(struct udevice *dev)
{
	struct rk_nfc *nfc = dev_get_priv(dev);
	int ret = 0;

	nfc->cfg = (void *)dev_get_driver_data(dev);
	nfc->dev = dev;

	nfc->regs = dev_read_addr_ptr(dev);
	if (!nfc->regs) {
		ret = -EINVAL;
		goto release_nfc;
	}

	nfc->nfc_clk = devm_clk_get(dev, "nfc");
	if (IS_ERR(nfc->nfc_clk)) {
		dev_dbg(dev, "no NFC clk\n");
		/* Some earlier models, such as rk3066, have no NFC clk. */
	}

	nfc->ahb_clk = devm_clk_get(dev, "ahb");
	if (IS_ERR(nfc->ahb_clk)) {
		dev_err(dev, "no ahb clk\n");
		ret = PTR_ERR(nfc->ahb_clk);
		goto release_nfc;
	}

	ret = rk_nfc_enable_clks(dev, nfc);
	if (ret)
		goto release_nfc;

	spin_lock_init(&nfc->controller.lock);
	init_waitqueue_head(&nfc->controller.wq);

	rk_nfc_hw_init(nfc);

	ret = rk_nfc_nand_chips_init(dev, nfc);
	if (ret) {
		dev_err(dev, "failed to init NAND chips\n");
		goto clk_disable;
	}
	return 0;

clk_disable:
	rk_nfc_disable_clks(nfc);
release_nfc:
	return ret;
}

U_BOOT_DRIVER(rockchip_nfc) = {
	.name = "rockchip_nfc",
	.id = UCLASS_MTD,
	.of_match = rk_nfc_id_table,
	.probe = rk_nfc_probe,
	.priv_auto = sizeof(struct rk_nfc),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(rockchip_nfc),
					  &dev);
	if (ret && ret != -ENODEV)
		log_err("Failed to initialize ROCKCHIP NAND controller. (error %d)\n",
			ret);
}

int nand_spl_load_image(uint32_t offs, unsigned int size, void *dst)
{
	struct mtd_info *mtd;
	size_t length = size;

	mtd = get_nand_dev_by_index(0);
	return nand_read_skip_bad(mtd, offs, &length, NULL, size, (u_char *)dst);
}

void nand_deselect(void) {}
