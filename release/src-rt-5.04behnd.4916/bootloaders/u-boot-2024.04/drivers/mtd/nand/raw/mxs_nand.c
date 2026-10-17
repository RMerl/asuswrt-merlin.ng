// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX28 NAND flash driver
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * Based on code from LTIB:
 * Freescale GPMI NFC NAND Flash Driver
 *
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 * Copyright (C) 2008 Embedded Alley Solutions, Inc.
 * Copyright 2017-2019 NXP
 */

#include <common.h>
#include <clk.h>
#include <cpu_func.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <malloc.h>
#include <mxs_nand.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/mach-imx/regs-bch.h>
#include <asm/mach-imx/regs-gpmi.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/mtd/rawnand.h>
#include <linux/sizes.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/math64.h>

#define	MXS_NAND_DMA_DESCRIPTOR_COUNT		4

#if defined(CONFIG_MX6) || defined(CONFIG_MX7) || defined(CONFIG_IMX8) || \
	defined(CONFIG_IMX8M)
#define	MXS_NAND_CHUNK_DATA_CHUNK_SIZE_SHIFT	2
#else
#define	MXS_NAND_CHUNK_DATA_CHUNK_SIZE_SHIFT	0
#endif
#define	MXS_NAND_METADATA_SIZE			10
#define	MXS_NAND_BITS_PER_ECC_LEVEL		13

#if !defined(CONFIG_SYS_CACHELINE_SIZE) || CONFIG_SYS_CACHELINE_SIZE < 32
#define	MXS_NAND_COMMAND_BUFFER_SIZE		32
#else
#define	MXS_NAND_COMMAND_BUFFER_SIZE		CONFIG_SYS_CACHELINE_SIZE
#endif

#define	MXS_NAND_BCH_TIMEOUT			10000

#define TO_CYCLES(duration, period) DIV_ROUND_UP_ULL(duration, period)

struct nand_ecclayout fake_ecc_layout;

/*
 * Cache management functions
 */
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
static void mxs_nand_flush_data_buf(struct mxs_nand_info *info)
{
	uint32_t addr = (uintptr_t)info->data_buf;

	flush_dcache_range(addr, addr + info->data_buf_size);
}

static void mxs_nand_inval_data_buf(struct mxs_nand_info *info)
{
	uint32_t addr = (uintptr_t)info->data_buf;

	invalidate_dcache_range(addr, addr + info->data_buf_size);
}

static void mxs_nand_flush_cmd_buf(struct mxs_nand_info *info)
{
	uint32_t addr = (uintptr_t)info->cmd_buf;

	flush_dcache_range(addr, addr + MXS_NAND_COMMAND_BUFFER_SIZE);
}
#else
static inline void mxs_nand_flush_data_buf(struct mxs_nand_info *info) {}
static inline void mxs_nand_inval_data_buf(struct mxs_nand_info *info) {}
static inline void mxs_nand_flush_cmd_buf(struct mxs_nand_info *info) {}
#endif

static struct mxs_dma_desc *mxs_nand_get_dma_desc(struct mxs_nand_info *info)
{
	struct mxs_dma_desc *desc;

	if (info->desc_index >= MXS_NAND_DMA_DESCRIPTOR_COUNT) {
		printf("MXS NAND: Too many DMA descriptors requested\n");
		return NULL;
	}

	desc = info->desc[info->desc_index];
	info->desc_index++;

	return desc;
}

static void mxs_nand_return_dma_descs(struct mxs_nand_info *info)
{
	int i;
	struct mxs_dma_desc *desc;

	for (i = 0; i < info->desc_index; i++) {
		desc = info->desc[i];
		memset(desc, 0, sizeof(struct mxs_dma_desc));
		desc->address = (dma_addr_t)desc;
	}

	info->desc_index = 0;
}

static uint32_t mxs_nand_aux_status_offset(void)
{
	return (MXS_NAND_METADATA_SIZE + 0x3) & ~0x3;
}

static inline bool mxs_nand_bbm_in_data_chunk(struct bch_geometry *geo,
					      struct mtd_info *mtd,
					      unsigned int *chunk_num)
{
	unsigned int i, j;

	if (geo->ecc_chunk0_size != geo->ecc_chunkn_size) {
		dev_err(mtd->dev, "The size of chunk0 must equal to chunkn\n");
		return false;
	}

	i = (mtd->writesize * 8 - MXS_NAND_METADATA_SIZE * 8) /
		(geo->gf_len * geo->ecc_strength +
				geo->ecc_chunkn_size * 8);

	j = (mtd->writesize * 8 - MXS_NAND_METADATA_SIZE * 8) -
		(geo->gf_len * geo->ecc_strength +
				geo->ecc_chunkn_size * 8) * i;

	if (j < geo->ecc_chunkn_size * 8) {
		*chunk_num = i + 1;
		dev_dbg(mtd->dev, "Set ecc to %d and bbm in chunk %d\n",
			geo->ecc_strength, *chunk_num);
		return true;
	}

	return false;
}

static inline int mxs_nand_calc_ecc_layout_by_info(struct bch_geometry *geo,
						   struct mtd_info *mtd,
						   unsigned int ecc_strength,
						   unsigned int ecc_step)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(chip);
	unsigned int block_mark_bit_offset;

	switch (ecc_step) {
	case SZ_512:
		geo->gf_len = 13;
		break;
	case SZ_1K:
		geo->gf_len = 14;
		break;
	default:
		return -EINVAL;
	}

	geo->ecc_chunk0_size = ecc_step;
	geo->ecc_chunkn_size = ecc_step;
	geo->ecc_strength = round_up(ecc_strength, 2);

	/* Keep the C >= O */
	if (geo->ecc_chunkn_size < mtd->oobsize)
		return -EINVAL;

	if (geo->ecc_strength > nand_info->max_ecc_strength_supported)
		return -EINVAL;

	geo->ecc_chunk_count = mtd->writesize / geo->ecc_chunkn_size;

	/* For bit swap. */
	block_mark_bit_offset = mtd->writesize * 8 -
		(geo->ecc_strength * geo->gf_len * (geo->ecc_chunk_count - 1)
				+ MXS_NAND_METADATA_SIZE * 8);

	geo->block_mark_byte_offset = block_mark_bit_offset / 8;
	geo->block_mark_bit_offset  = block_mark_bit_offset % 8;

	return 0;
}

static inline int mxs_nand_legacy_calc_ecc_layout(struct bch_geometry *geo,
					   struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(chip);
	unsigned int block_mark_bit_offset;
	int corr, ds_corr;

	/* The default for the length of Galois Field. */
	geo->gf_len = 13;

	/* The default for chunk size. */
	geo->ecc_chunk0_size = 512;
	geo->ecc_chunkn_size = 512;

	if (geo->ecc_chunkn_size < mtd->oobsize) {
		geo->gf_len = 14;
		geo->ecc_chunk0_size *= 2;
		geo->ecc_chunkn_size *= 2;
	}

	geo->ecc_chunk_count = mtd->writesize / geo->ecc_chunkn_size;

	/*
	 * Determine the ECC layout with the formula:
	 *	ECC bits per chunk = (total page spare data bits) /
	 *		(bits per ECC level) / (chunks per page)
	 * where:
	 *	total page spare data bits =
	 *		(page oob size - meta data size) * (bits per byte)
	 */
	geo->ecc_strength = ((mtd->oobsize - MXS_NAND_METADATA_SIZE) * 8)
			/ (geo->gf_len * geo->ecc_chunk_count);

	geo->ecc_strength = min(round_down(geo->ecc_strength, 2),
				nand_info->max_ecc_strength_supported);

	/* check ecc strength, same as nand_ecc_is_strong_enough() did*/
	if (chip->ecc_step_ds) {
		corr = mtd->writesize * geo->ecc_strength /
		       geo->ecc_chunkn_size;
		ds_corr = mtd->writesize * chip->ecc_strength_ds /
		       chip->ecc_step_ds;
		if (corr < ds_corr ||
		    geo->ecc_strength < chip->ecc_strength_ds)
			return -EINVAL;
	}

	block_mark_bit_offset = mtd->writesize * 8 -
		(geo->ecc_strength * geo->gf_len * (geo->ecc_chunk_count - 1)
				+ MXS_NAND_METADATA_SIZE * 8);

	geo->block_mark_byte_offset = block_mark_bit_offset / 8;
	geo->block_mark_bit_offset  = block_mark_bit_offset % 8;

	return 0;
}

static inline int mxs_nand_calc_ecc_for_large_oob(struct bch_geometry *geo,
					   struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(chip);
	unsigned int block_mark_bit_offset;
	unsigned int max_ecc;
	unsigned int bbm_chunk;
	unsigned int i;

	/* sanity check for the minimum ecc nand required */
	if (!(chip->ecc_strength_ds > 0 && chip->ecc_step_ds > 0))
		return -EINVAL;
	geo->ecc_strength = chip->ecc_strength_ds;

	/* calculate the maximum ecc platform can support*/
	geo->gf_len = 14;
	geo->ecc_chunk0_size = 1024;
	geo->ecc_chunkn_size = 1024;
	geo->ecc_chunk_count = mtd->writesize / geo->ecc_chunkn_size;
	max_ecc = ((mtd->oobsize - MXS_NAND_METADATA_SIZE) * 8)
			/ (geo->gf_len * geo->ecc_chunk_count);
	max_ecc = min(round_down(max_ecc, 2),
				nand_info->max_ecc_strength_supported);


	/* search a supported ecc strength that makes bbm */
	/* located in data chunk  */
	geo->ecc_strength = chip->ecc_strength_ds;
	while (!(geo->ecc_strength > max_ecc)) {
		if (mxs_nand_bbm_in_data_chunk(geo, mtd, &bbm_chunk))
			break;
		geo->ecc_strength += 2;
	}

	/* if none of them works, keep using the minimum ecc */
	/* nand required but changing ecc page layout  */
	if (geo->ecc_strength > max_ecc) {
		geo->ecc_strength = chip->ecc_strength_ds;
		/* add extra ecc for meta data */
		geo->ecc_chunk0_size = 0;
		geo->ecc_chunk_count = (mtd->writesize / geo->ecc_chunkn_size) + 1;
		geo->ecc_for_meta = 1;
		/* check if oob can afford this extra ecc chunk */
		if (mtd->oobsize * 8 < MXS_NAND_METADATA_SIZE * 8 +
				geo->gf_len * geo->ecc_strength
				* geo->ecc_chunk_count) {
			printf("unsupported NAND chip with new layout\n");
			return -EINVAL;
		}

		/* calculate in which chunk bbm located */
		bbm_chunk = (mtd->writesize * 8 - MXS_NAND_METADATA_SIZE * 8 -
			geo->gf_len * geo->ecc_strength) /
			(geo->gf_len * geo->ecc_strength +
					geo->ecc_chunkn_size * 8) + 1;
	}

	/* calculate the number of ecc chunk behind the bbm */
	i = (mtd->writesize / geo->ecc_chunkn_size) - bbm_chunk + 1;

	block_mark_bit_offset = mtd->writesize * 8 -
		(geo->ecc_strength * geo->gf_len * (geo->ecc_chunk_count - i)
				+ MXS_NAND_METADATA_SIZE * 8);

	geo->block_mark_byte_offset = block_mark_bit_offset / 8;
	geo->block_mark_bit_offset  = block_mark_bit_offset % 8;

	return 0;
}

/*
 * Wait for BCH complete IRQ and clear the IRQ
 */
static int mxs_nand_wait_for_bch_complete(struct mxs_nand_info *nand_info)
{
	int timeout = MXS_NAND_BCH_TIMEOUT;
	int ret;

	ret = mxs_wait_mask_set(&nand_info->bch_regs->hw_bch_ctrl_reg,
		BCH_CTRL_COMPLETE_IRQ, timeout);

	writel(BCH_CTRL_COMPLETE_IRQ, &nand_info->bch_regs->hw_bch_ctrl_clr);

	return ret;
}

/*
 * This is the function that we install in the cmd_ctrl function pointer of the
 * owning struct nand_chip. The only functions in the reference implementation
 * that use these functions pointers are cmdfunc and select_chip.
 *
 * In this driver, we implement our own select_chip, so this function will only
 * be called by the reference implementation's cmdfunc. For this reason, we can
 * ignore the chip enable bit and concentrate only on sending bytes to the NAND
 * Flash.
 */
static void mxs_nand_cmd_ctrl(struct mtd_info *mtd, int data, unsigned int ctrl)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);
	struct mxs_dma_desc *d;
	uint32_t channel = MXS_DMA_CHANNEL_AHB_APBH_GPMI0 + nand_info->cur_chip;
	int ret;

	/*
	 * If this condition is true, something is _VERY_ wrong in MTD
	 * subsystem!
	 */
	if (nand_info->cmd_queue_len == MXS_NAND_COMMAND_BUFFER_SIZE) {
		printf("MXS NAND: Command queue too long\n");
		return;
	}

	/*
	 * Every operation begins with a command byte and a series of zero or
	 * more address bytes. These are distinguished by either the Address
	 * Latch Enable (ALE) or Command Latch Enable (CLE) signals being
	 * asserted. When MTD is ready to execute the command, it will
	 * deasert both latch enables.
	 *
	 * Rather than run a separate DMA operation for every single byte, we
	 * queue them up and run a single DMA operation for the entire series
	 * of command and data bytes.
	 */
	if (ctrl & (NAND_ALE | NAND_CLE)) {
		if (data != NAND_CMD_NONE)
			nand_info->cmd_buf[nand_info->cmd_queue_len++] = data;
		return;
	}

	/*
	 * If control arrives here, MTD has deasserted both the ALE and CLE,
	 * which means it's ready to run an operation. Check if we have any
	 * bytes to send.
	 */
	if (nand_info->cmd_queue_len == 0)
		return;

	/* Compile the DMA descriptor -- a descriptor that sends command. */
	d = mxs_nand_get_dma_desc(nand_info);
	d->cmd.data =
		MXS_DMA_DESC_COMMAND_DMA_READ | MXS_DMA_DESC_IRQ |
		MXS_DMA_DESC_CHAIN | MXS_DMA_DESC_DEC_SEM |
		MXS_DMA_DESC_WAIT4END | (3 << MXS_DMA_DESC_PIO_WORDS_OFFSET) |
		(nand_info->cmd_queue_len << MXS_DMA_DESC_BYTES_OFFSET);

	d->cmd.address = (dma_addr_t)nand_info->cmd_buf;

	d->cmd.pio_words[0] =
		GPMI_CTRL0_COMMAND_MODE_WRITE |
		GPMI_CTRL0_WORD_LENGTH |
		(nand_info->cur_chip << GPMI_CTRL0_CS_OFFSET) |
		GPMI_CTRL0_ADDRESS_NAND_CLE |
		GPMI_CTRL0_ADDRESS_INCREMENT |
		nand_info->cmd_queue_len;

	mxs_dma_desc_append(channel, d);

	/* Flush caches */
	mxs_nand_flush_cmd_buf(nand_info);

	/* Execute the DMA chain. */
	ret = mxs_dma_go(channel);
	if (ret)
		printf("MXS NAND: Error sending command\n");

	mxs_nand_return_dma_descs(nand_info);

	/* Reset the command queue. */
	nand_info->cmd_queue_len = 0;
}

/*
 * Test if the NAND flash is ready.
 */
static int mxs_nand_device_ready(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(chip);
	uint32_t tmp;

	tmp = readl(&nand_info->gpmi_regs->hw_gpmi_stat);
	tmp >>= (GPMI_STAT_READY_BUSY_OFFSET + nand_info->cur_chip);

	return tmp & 1;
}

/*
 * Select the NAND chip.
 */
static void mxs_nand_select_chip(struct mtd_info *mtd, int chip)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);

	nand_info->cur_chip = chip;
}

/*
 * Handle block mark swapping.
 *
 * Note that, when this function is called, it doesn't know whether it's
 * swapping the block mark, or swapping it *back* -- but it doesn't matter
 * because the the operation is the same.
 */
static void mxs_nand_swap_block_mark(struct bch_geometry *geo,
				     uint8_t *data_buf, uint8_t *oob_buf)
{
	uint32_t bit_offset = geo->block_mark_bit_offset;
	uint32_t buf_offset = geo->block_mark_byte_offset;

	uint32_t src;
	uint32_t dst;

	/*
	 * Get the byte from the data area that overlays the block mark. Since
	 * the ECC engine applies its own view to the bits in the page, the
	 * physical block mark won't (in general) appear on a byte boundary in
	 * the data.
	 */
	src = data_buf[buf_offset] >> bit_offset;
	src |= data_buf[buf_offset + 1] << (8 - bit_offset);

	dst = oob_buf[0];

	oob_buf[0] = src;

	data_buf[buf_offset] &= ~(0xff << bit_offset);
	data_buf[buf_offset + 1] &= 0xff << bit_offset;

	data_buf[buf_offset] |= dst << bit_offset;
	data_buf[buf_offset + 1] |= dst >> (8 - bit_offset);
}

/*
 * Read data from NAND.
 */
static void mxs_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int length)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);
	struct mxs_dma_desc *d;
	uint32_t channel = MXS_DMA_CHANNEL_AHB_APBH_GPMI0 + nand_info->cur_chip;
	int ret;

	if (length > NAND_MAX_PAGESIZE) {
		printf("MXS NAND: DMA buffer too big\n");
		return;
	}

	if (!buf) {
		printf("MXS NAND: DMA buffer is NULL\n");
		return;
	}

	/* Compile the DMA descriptor - a descriptor that reads data. */
	d = mxs_nand_get_dma_desc(nand_info);
	d->cmd.data =
		MXS_DMA_DESC_COMMAND_DMA_WRITE | MXS_DMA_DESC_IRQ |
		MXS_DMA_DESC_DEC_SEM | MXS_DMA_DESC_WAIT4END |
		(1 << MXS_DMA_DESC_PIO_WORDS_OFFSET) |
		(length << MXS_DMA_DESC_BYTES_OFFSET);

	d->cmd.address = (dma_addr_t)nand_info->data_buf;

	d->cmd.pio_words[0] =
		GPMI_CTRL0_COMMAND_MODE_READ |
		GPMI_CTRL0_WORD_LENGTH |
		(nand_info->cur_chip << GPMI_CTRL0_CS_OFFSET) |
		GPMI_CTRL0_ADDRESS_NAND_DATA |
		length;

	mxs_dma_desc_append(channel, d);

	/*
	 * A DMA descriptor that waits for the command to end and the chip to
	 * become ready.
	 *
	 * I think we actually should *not* be waiting for the chip to become
	 * ready because, after all, we don't care. I think the original code
	 * did that and no one has re-thought it yet.
	 */
	d = mxs_nand_get_dma_desc(nand_info);
	d->cmd.data =
		MXS_DMA_DESC_COMMAND_NO_DMAXFER | MXS_DMA_DESC_IRQ |
		MXS_DMA_DESC_NAND_WAIT_4_READY | MXS_DMA_DESC_DEC_SEM |
		MXS_DMA_DESC_WAIT4END | (1 << MXS_DMA_DESC_PIO_WORDS_OFFSET);

	d->cmd.address = 0;

	d->cmd.pio_words[0] =
		GPMI_CTRL0_COMMAND_MODE_WAIT_FOR_READY |
		GPMI_CTRL0_WORD_LENGTH |
		(nand_info->cur_chip << GPMI_CTRL0_CS_OFFSET) |
		GPMI_CTRL0_ADDRESS_NAND_DATA;

	mxs_dma_desc_append(channel, d);

	/* Invalidate caches */
	mxs_nand_inval_data_buf(nand_info);

	/* Execute the DMA chain. */
	ret = mxs_dma_go(channel);
	if (ret) {
		printf("MXS NAND: DMA read error\n");
		goto rtn;
	}

	/* Invalidate caches */
	mxs_nand_inval_data_buf(nand_info);

	memcpy(buf, nand_info->data_buf, length);

rtn:
	mxs_nand_return_dma_descs(nand_info);
}

/*
 * Write data to NAND.
 */
static void mxs_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf,
				int length)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);
	struct mxs_dma_desc *d;
	uint32_t channel = MXS_DMA_CHANNEL_AHB_APBH_GPMI0 + nand_info->cur_chip;
	int ret;

	if (length > NAND_MAX_PAGESIZE) {
		printf("MXS NAND: DMA buffer too big\n");
		return;
	}

	if (!buf) {
		printf("MXS NAND: DMA buffer is NULL\n");
		return;
	}

	memcpy(nand_info->data_buf, buf, length);

	/* Compile the DMA descriptor - a descriptor that writes data. */
	d = mxs_nand_get_dma_desc(nand_info);
	d->cmd.data =
		MXS_DMA_DESC_COMMAND_DMA_READ | MXS_DMA_DESC_IRQ |
		MXS_DMA_DESC_DEC_SEM | MXS_DMA_DESC_WAIT4END |
		(1 << MXS_DMA_DESC_PIO_WORDS_OFFSET) |
		(length << MXS_DMA_DESC_BYTES_OFFSET);

	d->cmd.address = (dma_addr_t)nand_info->data_buf;

	d->cmd.pio_words[0] =
		GPMI_CTRL0_COMMAND_MODE_WRITE |
		GPMI_CTRL0_WORD_LENGTH |
		(nand_info->cur_chip << GPMI_CTRL0_CS_OFFSET) |
		GPMI_CTRL0_ADDRESS_NAND_DATA |
		length;

	mxs_dma_desc_append(channel, d);

	/* Flush caches */
	mxs_nand_flush_data_buf(nand_info);

	/* Execute the DMA chain. */
	ret = mxs_dma_go(channel);
	if (ret)
		printf("MXS NAND: DMA write error\n");

	mxs_nand_return_dma_descs(nand_info);
}

/*
 * Read a single byte from NAND.
 */
static uint8_t mxs_nand_read_byte(struct mtd_info *mtd)
{
	uint8_t buf;
	mxs_nand_read_buf(mtd, &buf, 1);
	return buf;
}

static bool mxs_nand_erased_page(struct mtd_info *mtd, struct nand_chip *nand,
				 u8 *buf, int chunk, int page)
{
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);
	struct bch_geometry *geo = &nand_info->bch_geometry;
	unsigned int flip_bits = 0, flip_bits_noecc = 0;
	unsigned int threshold;
	unsigned int base = geo->ecc_chunkn_size * chunk;
	u32 *dma_buf = (u32 *)buf;
	int i;

	threshold = geo->gf_len / 2;
	if (threshold > geo->ecc_strength)
		threshold = geo->ecc_strength;

	for (i = 0; i < geo->ecc_chunkn_size; i++) {
		flip_bits += hweight8(~buf[base + i]);
		if (flip_bits > threshold)
			return false;
	}

	nand->cmdfunc(mtd, NAND_CMD_READ0, 0, page);
	nand->read_buf(mtd, buf, mtd->writesize);

	for (i = 0; i < mtd->writesize / 4; i++) {
		flip_bits_noecc += hweight32(~dma_buf[i]);
		if (flip_bits_noecc > threshold)
			return false;
	}

	mtd->ecc_stats.corrected += flip_bits;

	memset(buf, 0xff, mtd->writesize);

	printf("The page(%d) is an erased page(%d,%d,%d,%d).\n", page, chunk, threshold, flip_bits, flip_bits_noecc);

	return true;
}

/*
 * Read a page from NAND.
 */
static int mxs_nand_ecc_read_page(struct mtd_info *mtd, struct nand_chip *nand,
					uint8_t *buf, int oob_required,
					int page)
{
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);
	struct bch_geometry *geo = &nand_info->bch_geometry;
	struct mxs_bch_regs *bch_regs = nand_info->bch_regs;
	struct mxs_dma_desc *d;
	uint32_t channel = MXS_DMA_CHANNEL_AHB_APBH_GPMI0 + nand_info->cur_chip;
	uint32_t corrected = 0, failed = 0;
	uint8_t	*status;
	int i, ret;
	int flag = 0;

	/* Compile the DMA descriptor - wait for ready. */
	d = mxs_nand_get_dma_desc(nand_info);
	d->cmd.data =
		MXS_DMA_DESC_COMMAND_NO_DMAXFER | MXS_DMA_DESC_CHAIN |
		MXS_DMA_DESC_NAND_WAIT_4_READY | MXS_DMA_DESC_WAIT4END |
		(1 << MXS_DMA_DESC_PIO_WORDS_OFFSET);

	d->cmd.address = 0;

	d->cmd.pio_words[0] =
		GPMI_CTRL0_COMMAND_MODE_WAIT_FOR_READY |
		GPMI_CTRL0_WORD_LENGTH |
		(nand_info->cur_chip << GPMI_CTRL0_CS_OFFSET) |
		GPMI_CTRL0_ADDRESS_NAND_DATA;

	mxs_dma_desc_append(channel, d);

	/* Compile the DMA descriptor - enable the BCH block and read. */
	d = mxs_nand_get_dma_desc(nand_info);
	d->cmd.data =
		MXS_DMA_DESC_COMMAND_NO_DMAXFER | MXS_DMA_DESC_CHAIN |
		MXS_DMA_DESC_WAIT4END |	(6 << MXS_DMA_DESC_PIO_WORDS_OFFSET);

	d->cmd.address = 0;

	d->cmd.pio_words[0] =
		GPMI_CTRL0_COMMAND_MODE_READ |
		GPMI_CTRL0_WORD_LENGTH |
		(nand_info->cur_chip << GPMI_CTRL0_CS_OFFSET) |
		GPMI_CTRL0_ADDRESS_NAND_DATA |
		(mtd->writesize + mtd->oobsize);
	d->cmd.pio_words[1] = 0;
	d->cmd.pio_words[2] =
		GPMI_ECCCTRL_ENABLE_ECC |
		GPMI_ECCCTRL_ECC_CMD_DECODE |
		GPMI_ECCCTRL_BUFFER_MASK_BCH_PAGE;
	d->cmd.pio_words[3] = mtd->writesize + mtd->oobsize;
	d->cmd.pio_words[4] = (dma_addr_t)nand_info->data_buf;
	d->cmd.pio_words[5] = (dma_addr_t)nand_info->oob_buf;

	if (nand_info->en_randomizer) {
		d->cmd.pio_words[2] |= GPMI_ECCCTRL_RANDOMIZER_ENABLE |
				       GPMI_ECCCTRL_RANDOMIZER_TYPE2;
		d->cmd.pio_words[3] |= (page % 256) << 16;
	}

	mxs_dma_desc_append(channel, d);

	/* Compile the DMA descriptor - disable the BCH block. */
	d = mxs_nand_get_dma_desc(nand_info);
	d->cmd.data =
		MXS_DMA_DESC_COMMAND_NO_DMAXFER | MXS_DMA_DESC_CHAIN |
		MXS_DMA_DESC_NAND_WAIT_4_READY | MXS_DMA_DESC_WAIT4END |
		(3 << MXS_DMA_DESC_PIO_WORDS_OFFSET);

	d->cmd.address = 0;

	d->cmd.pio_words[0] =
		GPMI_CTRL0_COMMAND_MODE_WAIT_FOR_READY |
		GPMI_CTRL0_WORD_LENGTH |
		(nand_info->cur_chip << GPMI_CTRL0_CS_OFFSET) |
		GPMI_CTRL0_ADDRESS_NAND_DATA |
		(mtd->writesize + mtd->oobsize);
	d->cmd.pio_words[1] = 0;
	d->cmd.pio_words[2] = 0;

	mxs_dma_desc_append(channel, d);

	/* Compile the DMA descriptor - deassert the NAND lock and interrupt. */
	d = mxs_nand_get_dma_desc(nand_info);
	d->cmd.data =
		MXS_DMA_DESC_COMMAND_NO_DMAXFER | MXS_DMA_DESC_IRQ |
		MXS_DMA_DESC_DEC_SEM;

	d->cmd.address = 0;

	mxs_dma_desc_append(channel, d);

	/* Invalidate caches */
	mxs_nand_inval_data_buf(nand_info);

	/* Execute the DMA chain. */
	ret = mxs_dma_go(channel);
	if (ret) {
		printf("MXS NAND: DMA read error\n");
		goto rtn;
	}

	ret = mxs_nand_wait_for_bch_complete(nand_info);
	if (ret) {
		printf("MXS NAND: BCH read timeout\n");
		goto rtn;
	}

	mxs_nand_return_dma_descs(nand_info);

	/* Invalidate caches */
	mxs_nand_inval_data_buf(nand_info);

	/* Read DMA completed, now do the mark swapping. */
	mxs_nand_swap_block_mark(geo, nand_info->data_buf, nand_info->oob_buf);

	/* Loop over status bytes, accumulating ECC status. */
	status = nand_info->oob_buf + mxs_nand_aux_status_offset();
	for (i = 0; i < geo->ecc_chunk_count; i++) {
		if (status[i] == 0x00)
			continue;

		if (status[i] == 0xff) {
			if (!nand_info->en_randomizer &&
			    (is_mx6dqp() || is_mx7() || is_mx6ul() ||
			     is_imx8() || is_imx8m()))
				if (readl(&bch_regs->hw_bch_debug1))
					flag = 1;
			continue;
		}

		if (status[i] == 0xfe) {
			if (mxs_nand_erased_page(mtd, nand,
						 nand_info->data_buf, i, page))
				break;
			failed++;
			continue;
		}

		corrected += status[i];
	}

	/* Propagate ECC status to the owning MTD. */
	mtd->ecc_stats.failed += failed;
	mtd->ecc_stats.corrected += corrected;

	/*
	 * It's time to deliver the OOB bytes. See mxs_nand_ecc_read_oob() for
	 * details about our policy for delivering the OOB.
	 *
	 * We fill the caller's buffer with set bits, and then copy the block
	 * mark to the caller's buffer. Note that, if block mark swapping was
	 * necessary, it has already been done, so we can rely on the first
	 * byte of the auxiliary buffer to contain the block mark.
	 */
	memset(nand->oob_poi, 0xff, mtd->oobsize);

	nand->oob_poi[0] = nand_info->oob_buf[0];

	memcpy(buf, nand_info->data_buf, mtd->writesize);

	if (flag)
		memset(buf, 0xff, mtd->writesize);
rtn:
	mxs_nand_return_dma_descs(nand_info);

	return ret;
}

/*
 * Write a page to NAND.
 */
static int mxs_nand_ecc_write_page(struct mtd_info *mtd,
				struct nand_chip *nand, const uint8_t *buf,
				int oob_required, int page)
{
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);
	struct bch_geometry *geo = &nand_info->bch_geometry;
	struct mxs_dma_desc *d;
	uint32_t channel = MXS_DMA_CHANNEL_AHB_APBH_GPMI0 + nand_info->cur_chip;
	int ret;

	memcpy(nand_info->data_buf, buf, mtd->writesize);
	memcpy(nand_info->oob_buf, nand->oob_poi, mtd->oobsize);

	/* Handle block mark swapping. */
	mxs_nand_swap_block_mark(geo, nand_info->data_buf, nand_info->oob_buf);

	/* Compile the DMA descriptor - write data. */
	d = mxs_nand_get_dma_desc(nand_info);
	d->cmd.data =
		MXS_DMA_DESC_COMMAND_NO_DMAXFER | MXS_DMA_DESC_IRQ |
		MXS_DMA_DESC_DEC_SEM | MXS_DMA_DESC_WAIT4END |
		(6 << MXS_DMA_DESC_PIO_WORDS_OFFSET);

	d->cmd.address = 0;

	d->cmd.pio_words[0] =
		GPMI_CTRL0_COMMAND_MODE_WRITE |
		GPMI_CTRL0_WORD_LENGTH |
		(nand_info->cur_chip << GPMI_CTRL0_CS_OFFSET) |
		GPMI_CTRL0_ADDRESS_NAND_DATA;
	d->cmd.pio_words[1] = 0;
	d->cmd.pio_words[2] =
		GPMI_ECCCTRL_ENABLE_ECC |
		GPMI_ECCCTRL_ECC_CMD_ENCODE |
		GPMI_ECCCTRL_BUFFER_MASK_BCH_PAGE;
	d->cmd.pio_words[3] = (mtd->writesize + mtd->oobsize);
	d->cmd.pio_words[4] = (dma_addr_t)nand_info->data_buf;
	d->cmd.pio_words[5] = (dma_addr_t)nand_info->oob_buf;

	if (nand_info->en_randomizer) {
		d->cmd.pio_words[2] |= GPMI_ECCCTRL_RANDOMIZER_ENABLE |
				       GPMI_ECCCTRL_RANDOMIZER_TYPE2;
		/*
		 * Write NAND page number needed to be randomized
		 * to GPMI_ECCCOUNT register.
		 *
		 * The value is between 0-255. For additional details
		 * check 9.6.6.4 of i.MX7D Applications Processor reference
		 */
		d->cmd.pio_words[3] |= (page % 256) << 16;
	}

	mxs_dma_desc_append(channel, d);

	/* Flush caches */
	mxs_nand_flush_data_buf(nand_info);

	/* Execute the DMA chain. */
	ret = mxs_dma_go(channel);
	if (ret) {
		printf("MXS NAND: DMA write error\n");
		goto rtn;
	}

	ret = mxs_nand_wait_for_bch_complete(nand_info);
	if (ret) {
		printf("MXS NAND: BCH write timeout\n");
		goto rtn;
	}

rtn:
	mxs_nand_return_dma_descs(nand_info);
	return 0;
}

/*
 * Read OOB from NAND.
 *
 * This function is a veneer that replaces the function originally installed by
 * the NAND Flash MTD code.
 */
static int mxs_nand_hook_read_oob(struct mtd_info *mtd, loff_t from,
					struct mtd_oob_ops *ops)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(chip);
	int ret;

	if (ops->mode == MTD_OPS_RAW)
		nand_info->raw_oob_mode = 1;
	else
		nand_info->raw_oob_mode = 0;

	ret = nand_info->hooked_read_oob(mtd, from, ops);

	nand_info->raw_oob_mode = 0;

	return ret;
}

/*
 * Write OOB to NAND.
 *
 * This function is a veneer that replaces the function originally installed by
 * the NAND Flash MTD code.
 */
static int mxs_nand_hook_write_oob(struct mtd_info *mtd, loff_t to,
					struct mtd_oob_ops *ops)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(chip);
	int ret;

	if (ops->mode == MTD_OPS_RAW)
		nand_info->raw_oob_mode = 1;
	else
		nand_info->raw_oob_mode = 0;

	ret = nand_info->hooked_write_oob(mtd, to, ops);

	nand_info->raw_oob_mode = 0;

	return ret;
}

/*
 * Mark a block bad in NAND.
 *
 * This function is a veneer that replaces the function originally installed by
 * the NAND Flash MTD code.
 */
static int mxs_nand_hook_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(chip);
	int ret;

	nand_info->marking_block_bad = 1;

	ret = nand_info->hooked_block_markbad(mtd, ofs);

	nand_info->marking_block_bad = 0;

	return ret;
}

/*
 * There are several places in this driver where we have to handle the OOB and
 * block marks. This is the function where things are the most complicated, so
 * this is where we try to explain it all. All the other places refer back to
 * here.
 *
 * These are the rules, in order of decreasing importance:
 *
 * 1) Nothing the caller does can be allowed to imperil the block mark, so all
 *    write operations take measures to protect it.
 *
 * 2) In read operations, the first byte of the OOB we return must reflect the
 *    true state of the block mark, no matter where that block mark appears in
 *    the physical page.
 *
 * 3) ECC-based read operations return an OOB full of set bits (since we never
 *    allow ECC-based writes to the OOB, it doesn't matter what ECC-based reads
 *    return).
 *
 * 4) "Raw" read operations return a direct view of the physical bytes in the
 *    page, using the conventional definition of which bytes are data and which
 *    are OOB. This gives the caller a way to see the actual, physical bytes
 *    in the page, without the distortions applied by our ECC engine.
 *
 * What we do for this specific read operation depends on whether we're doing
 * "raw" read, or an ECC-based read.
 *
 * It turns out that knowing whether we want an "ECC-based" or "raw" read is not
 * easy. When reading a page, for example, the NAND Flash MTD code calls our
 * ecc.read_page or ecc.read_page_raw function. Thus, the fact that MTD wants an
 * ECC-based or raw view of the page is implicit in which function it calls
 * (there is a similar pair of ECC-based/raw functions for writing).
 *
 * Since MTD assumes the OOB is not covered by ECC, there is no pair of
 * ECC-based/raw functions for reading or or writing the OOB. The fact that the
 * caller wants an ECC-based or raw view of the page is not propagated down to
 * this driver.
 *
 * Since our OOB *is* covered by ECC, we need this information. So, we hook the
 * ecc.read_oob and ecc.write_oob function pointers in the owning
 * struct mtd_info with our own functions. These hook functions set the
 * raw_oob_mode field so that, when control finally arrives here, we'll know
 * what to do.
 */
static int mxs_nand_ecc_read_oob(struct mtd_info *mtd, struct nand_chip *nand,
				int page)
{
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);

	/*
	 * First, fill in the OOB buffer. If we're doing a raw read, we need to
	 * get the bytes from the physical page. If we're not doing a raw read,
	 * we need to fill the buffer with set bits.
	 */
	if (nand_info->raw_oob_mode) {
		/*
		 * If control arrives here, we're doing a "raw" read. Send the
		 * command to read the conventional OOB and read it.
		 */
		nand->cmdfunc(mtd, NAND_CMD_READ0, mtd->writesize, page);
		nand->read_buf(mtd, nand->oob_poi, mtd->oobsize);
	} else {
		/*
		 * If control arrives here, we're not doing a "raw" read. Fill
		 * the OOB buffer with set bits and correct the block mark.
		 */
		memset(nand->oob_poi, 0xff, mtd->oobsize);

		nand->cmdfunc(mtd, NAND_CMD_READ0, mtd->writesize, page);
		mxs_nand_read_buf(mtd, nand->oob_poi, 1);
	}

	return 0;

}

/*
 * Write OOB data to NAND.
 */
static int mxs_nand_ecc_write_oob(struct mtd_info *mtd, struct nand_chip *nand,
					int page)
{
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);
	uint8_t block_mark = 0;

	/*
	 * There are fundamental incompatibilities between the i.MX GPMI NFC and
	 * the NAND Flash MTD model that make it essentially impossible to write
	 * the out-of-band bytes.
	 *
	 * We permit *ONE* exception. If the *intent* of writing the OOB is to
	 * mark a block bad, we can do that.
	 */

	if (!nand_info->marking_block_bad) {
		printf("NXS NAND: Writing OOB isn't supported\n");
		return -EIO;
	}

	/* Write the block mark. */
	nand->cmdfunc(mtd, NAND_CMD_SEQIN, mtd->writesize, page);
	nand->write_buf(mtd, &block_mark, 1);
	nand->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	/* Check if it worked. */
	if (nand->waitfunc(mtd, nand) & NAND_STATUS_FAIL)
		return -EIO;

	return 0;
}

/*
 * Claims all blocks are good.
 *
 * In principle, this function is *only* called when the NAND Flash MTD system
 * isn't allowed to keep an in-memory bad block table, so it is forced to ask
 * the driver for bad block information.
 *
 * In fact, we permit the NAND Flash MTD system to have an in-memory BBT, so
 * this function is *only* called when we take it away.
 *
 * Thus, this function is only called when we want *all* blocks to look good,
 * so it *always* return success.
 */
static int mxs_nand_block_bad(struct mtd_info *mtd, loff_t ofs)
{
	return 0;
}

static int mxs_nand_set_geometry(struct mtd_info *mtd, struct bch_geometry *geo)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);
	int err;

	if (chip->ecc_strength_ds > nand_info->max_ecc_strength_supported) {
		printf("unsupported NAND chip, minimum ecc required %d\n"
			, chip->ecc_strength_ds);
		return -EINVAL;
	}

	/* use the legacy bch setting by default */
	if ((!nand_info->use_minimum_ecc && mtd->oobsize < 1024) ||
	    !(chip->ecc_strength_ds > 0 && chip->ecc_step_ds > 0)) {
		dev_dbg(mtd->dev, "use legacy bch geometry\n");
		err = mxs_nand_legacy_calc_ecc_layout(geo, mtd);
		if (!err)
			return 0;
	}

	/* for large oob nand */
	if (mtd->oobsize > 1024) {
		dev_dbg(mtd->dev, "use large oob bch geometry\n");
		err = mxs_nand_calc_ecc_for_large_oob(geo, mtd);
		if (!err)
			return 0;
	}

	/* otherwise use the minimum ecc nand chips required */
	dev_dbg(mtd->dev, "use minimum ecc bch geometry\n");
	err = mxs_nand_calc_ecc_layout_by_info(geo, mtd, chip->ecc_strength_ds,
					       chip->ecc_step_ds);

	if (err)
		dev_err(mtd->dev, "none of the bch geometry setting works\n");

	return err;
}

void mxs_nand_dump_geo(struct mtd_info *mtd)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);
	struct bch_geometry *geo = &nand_info->bch_geometry;

	dev_dbg(mtd->dev, "BCH Geometry :\n"
		"GF Length\t\t: %u\n"
		"ECC Strength\t\t: %u\n"
		"ECC for Meta\t\t: %u\n"
		"ECC Chunk0 Size\t\t: %u\n"
		"ECC Chunkn Size\t\t: %u\n"
		"ECC Chunk Count\t\t: %u\n"
		"Block Mark Byte Offset\t: %u\n"
		"Block Mark Bit Offset\t: %u\n",
		geo->gf_len,
		geo->ecc_strength,
		geo->ecc_for_meta,
		geo->ecc_chunk0_size,
		geo->ecc_chunkn_size,
		geo->ecc_chunk_count,
		geo->block_mark_byte_offset,
		geo->block_mark_bit_offset);
}

/*
 * At this point, the physical NAND Flash chips have been identified and
 * counted, so we know the physical geometry. This enables us to make some
 * important configuration decisions.
 *
 * The return value of this function propagates directly back to this driver's
 * board_nand_init(). Anything other than zero will cause this driver to
 * tear everything down and declare failure.
 */
int mxs_nand_setup_ecc(struct mtd_info *mtd)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);
	struct bch_geometry *geo = &nand_info->bch_geometry;
	struct mxs_bch_regs *bch_regs = nand_info->bch_regs;
	uint32_t tmp;
	int ret;

	nand_info->en_randomizer = 0;
	nand_info->oobsize = mtd->oobsize;
	nand_info->writesize = mtd->writesize;

	ret = mxs_nand_set_geometry(mtd, geo);
	if (ret)
		return ret;

	mxs_nand_dump_geo(mtd);

	/* Configure BCH and set NFC geometry */
	mxs_reset_block(&bch_regs->hw_bch_ctrl_reg);

	/* Configure layout 0 */
	tmp = (geo->ecc_chunk_count - 1) << BCH_FLASHLAYOUT0_NBLOCKS_OFFSET;
	tmp |= MXS_NAND_METADATA_SIZE << BCH_FLASHLAYOUT0_META_SIZE_OFFSET;
	tmp |= (geo->ecc_strength >> 1) << BCH_FLASHLAYOUT0_ECC0_OFFSET;
	tmp |= geo->ecc_chunk0_size >> MXS_NAND_CHUNK_DATA_CHUNK_SIZE_SHIFT;
	tmp |= (geo->gf_len == 14 ? 1 : 0) <<
		BCH_FLASHLAYOUT0_GF13_0_GF14_1_OFFSET;
	writel(tmp, &bch_regs->hw_bch_flash0layout0);
	nand_info->bch_flash0layout0 = tmp;

	tmp = (mtd->writesize + mtd->oobsize)
		<< BCH_FLASHLAYOUT1_PAGE_SIZE_OFFSET;
	tmp |= (geo->ecc_strength >> 1) << BCH_FLASHLAYOUT1_ECCN_OFFSET;
	tmp |= geo->ecc_chunkn_size >> MXS_NAND_CHUNK_DATA_CHUNK_SIZE_SHIFT;
	tmp |= (geo->gf_len == 14 ? 1 : 0) <<
		BCH_FLASHLAYOUT1_GF13_0_GF14_1_OFFSET;
	writel(tmp, &bch_regs->hw_bch_flash0layout1);
	nand_info->bch_flash0layout1 = tmp;

	/* Set erase threshold to ecc strength for mx6ul, mx6qp and mx7 */
	if (is_mx6dqp() || is_mx7() ||
	    is_mx6ul() || is_imx8() || is_imx8m())
		writel(BCH_MODE_ERASE_THRESHOLD(geo->ecc_strength),
		       &bch_regs->hw_bch_mode);

	/* Set *all* chip selects to use layout 0 */
	writel(0, &bch_regs->hw_bch_layoutselect);

	/* Enable BCH complete interrupt */
	writel(BCH_CTRL_COMPLETE_IRQ_EN, &bch_regs->hw_bch_ctrl_set);

	return 0;
}

/*
 * Allocate DMA buffers
 */
int mxs_nand_alloc_buffers(struct mxs_nand_info *nand_info)
{
	uint8_t *buf;
	const int size = NAND_MAX_PAGESIZE + NAND_MAX_OOBSIZE;

	nand_info->data_buf_size = roundup(size, MXS_DMA_ALIGNMENT);

	/* DMA buffers */
	buf = memalign(MXS_DMA_ALIGNMENT, nand_info->data_buf_size);
	if (!buf) {
		printf("MXS NAND: Error allocating DMA buffers\n");
		return -ENOMEM;
	}

	memset(buf, 0, nand_info->data_buf_size);

	nand_info->data_buf = buf;
	nand_info->oob_buf = buf + NAND_MAX_PAGESIZE;
	/* Command buffers */
	nand_info->cmd_buf = memalign(MXS_DMA_ALIGNMENT,
				MXS_NAND_COMMAND_BUFFER_SIZE);
	if (!nand_info->cmd_buf) {
		free(buf);
		printf("MXS NAND: Error allocating command buffers\n");
		return -ENOMEM;
	}
	memset(nand_info->cmd_buf, 0, MXS_NAND_COMMAND_BUFFER_SIZE);
	nand_info->cmd_queue_len = 0;

	return 0;
}

/*
 * Initializes the NFC hardware.
 */
static int mxs_nand_init_dma(struct mxs_nand_info *info)
{
	int i = 0, j, ret = 0;

	info->desc = malloc(sizeof(struct mxs_dma_desc *) *
				MXS_NAND_DMA_DESCRIPTOR_COUNT);
	if (!info->desc) {
		ret = -ENOMEM;
		goto err1;
	}

	/* Allocate the DMA descriptors. */
	for (i = 0; i < MXS_NAND_DMA_DESCRIPTOR_COUNT; i++) {
		info->desc[i] = mxs_dma_desc_alloc();
		if (!info->desc[i]) {
			ret = -ENOMEM;
			goto err2;
		}
	}

	/* Init the DMA controller. */
	mxs_dma_init();
	for (j = MXS_DMA_CHANNEL_AHB_APBH_GPMI0;
		j <= MXS_DMA_CHANNEL_AHB_APBH_GPMI7; j++) {
		ret = mxs_dma_init_channel(j);
		if (ret)
			goto err3;
	}

	/* Reset the GPMI block. */
	mxs_reset_block(&info->gpmi_regs->hw_gpmi_ctrl0_reg);
	mxs_reset_block(&info->bch_regs->hw_bch_ctrl_reg);

	/*
	 * Choose NAND mode, set IRQ polarity, disable write protection and
	 * select BCH ECC.
	 */
	clrsetbits_le32(&info->gpmi_regs->hw_gpmi_ctrl1,
			GPMI_CTRL1_GPMI_MODE,
			GPMI_CTRL1_ATA_IRQRDY_POLARITY | GPMI_CTRL1_DEV_RESET |
			GPMI_CTRL1_BCH_MODE);

	return 0;

err3:
	for (--j; j >= MXS_DMA_CHANNEL_AHB_APBH_GPMI0; j--)
		mxs_dma_release(j);
err2:
	for (--i; i >= 0; i--)
		mxs_dma_desc_free(info->desc[i]);
	free(info->desc);
err1:
	if (ret == -ENOMEM)
		printf("MXS NAND: Unable to allocate DMA descriptors\n");
	return ret;
}

/*
 * <1> Firstly, we should know what's the GPMI-clock means.
 *     The GPMI-clock is the internal clock in the gpmi nand controller.
 *     If you set 100MHz to gpmi nand controller, the GPMI-clock's period
 *     is 10ns. Mark the GPMI-clock's period as GPMI-clock-period.
 *
 * <2> Secondly, we should know what's the frequency on the nand chip pins.
 *     The frequency on the nand chip pins is derived from the GPMI-clock.
 *     We can get it from the following equation:
 *
 *         F = G / (DS + DH)
 *
 *         F  : the frequency on the nand chip pins.
 *         G  : the GPMI clock, such as 100MHz.
 *         DS : GPMI_HW_GPMI_TIMING0:DATA_SETUP
 *         DH : GPMI_HW_GPMI_TIMING0:DATA_HOLD
 *
 * <3> Thirdly, when the frequency on the nand chip pins is above 33MHz,
 *     the nand EDO(extended Data Out) timing could be applied.
 *     The GPMI implements a feedback read strobe to sample the read data.
 *     The feedback read strobe can be delayed to support the nand EDO timing
 *     where the read strobe may deasserts before the read data is valid, and
 *     read data is valid for some time after read strobe.
 *
 *     The following figure illustrates some aspects of a NAND Flash read:
 *
 *                   |<---tREA---->|
 *                   |             |
 *                   |         |   |
 *                   |<--tRP-->|   |
 *                   |         |   |
 *                  __          ___|__________________________________
 *     RDN            \________/   |
 *                                 |
 *                                 /---------\
 *     Read Data    --------------<           >---------
 *                                 \---------/
 *                                |     |
 *                                |<-D->|
 *     FeedbackRDN  ________             ____________
 *                          \___________/
 *
 *          D stands for delay, set in the HW_GPMI_CTRL1:RDN_DELAY.
 *
 *
 * <4> Now, we begin to describe how to compute the right RDN_DELAY.
 *
 *  4.1) From the aspect of the nand chip pins:
 *        Delay = (tREA + C - tRP)               {1}
 *
 *        tREA : the maximum read access time.
 *        C    : a constant to adjust the delay. default is 4000ps.
 *        tRP  : the read pulse width, which is exactly:
 *                   tRP = (GPMI-clock-period) * DATA_SETUP
 *
 *  4.2) From the aspect of the GPMI nand controller:
 *         Delay = RDN_DELAY * 0.125 * RP        {2}
 *
 *         RP   : the DLL reference period.
 *            if (GPMI-clock-period > DLL_THRETHOLD)
 *                   RP = GPMI-clock-period / 2;
 *            else
 *                   RP = GPMI-clock-period;
 *
 *            Set the HW_GPMI_CTRL1:HALF_PERIOD if GPMI-clock-period
 *            is greater DLL_THRETHOLD. In other SOCs, the DLL_THRETHOLD
 *            is 16000ps, but in mx6q, we use 12000ps.
 *
 *  4.3) since {1} equals {2}, we get:
 *
 *                     (tREA + 4000 - tRP) * 8
 *         RDN_DELAY = -----------------------     {3}
 *                           RP
 */
static void mxs_compute_timings(struct nand_chip *chip,
				const struct nand_sdr_timings *sdr)
{
	struct mxs_nand_info *nand_info = nand_get_controller_data(chip);
	unsigned long clk_rate;
	unsigned int dll_wait_time_us;
	unsigned int dll_threshold_ps = nand_info->max_chain_delay;
	unsigned int period_ps, reference_period_ps;
	unsigned int data_setup_cycles, data_hold_cycles, addr_setup_cycles;
	unsigned int tRP_ps;
	bool use_half_period;
	int sample_delay_ps, sample_delay_factor;
	u16 busy_timeout_cycles;
	u8 wrn_dly_sel;
	u32 timing0;
	u32 timing1;
	u32 ctrl1n;

	if (sdr->tRC_min >= 30000) {
		/* ONFI non-EDO modes [0-3] */
		clk_rate = 22000000;
		wrn_dly_sel = GPMI_CTRL1_WRN_DLY_SEL_4_TO_8NS;
	} else if (sdr->tRC_min >= 25000) {
		/* ONFI EDO mode 4 */
		clk_rate = 80000000;
		wrn_dly_sel = GPMI_CTRL1_WRN_DLY_SEL_NO_DELAY;
		debug("%s, setting ONFI onfi edo 4\n", __func__);
	} else {
		/* ONFI EDO mode 5 */
		clk_rate = 100000000;
		wrn_dly_sel = GPMI_CTRL1_WRN_DLY_SEL_NO_DELAY;
		debug("%s, setting ONFI onfi edo 5\n", __func__);
	}

	/* SDR core timings are given in picoseconds */
	period_ps = div_u64((u64)NSEC_PER_SEC * 1000, clk_rate);

	addr_setup_cycles = TO_CYCLES(sdr->tALS_min, period_ps);
	data_setup_cycles = TO_CYCLES(sdr->tDS_min, period_ps);
	data_hold_cycles = TO_CYCLES(sdr->tDH_min, period_ps);
	busy_timeout_cycles = TO_CYCLES(sdr->tWB_max + sdr->tR_max, period_ps);

	timing0 = (addr_setup_cycles << GPMI_TIMING0_ADDRESS_SETUP_OFFSET) |
		      (data_hold_cycles << GPMI_TIMING0_DATA_HOLD_OFFSET) |
		      (data_setup_cycles << GPMI_TIMING0_DATA_SETUP_OFFSET);
	timing1 = (busy_timeout_cycles * 4096) << GPMI_TIMING1_DEVICE_BUSY_TIMEOUT_OFFSET;

	/*
	 * Derive NFC ideal delay from {3}:
	 *
	 *                     (tREA + 4000 - tRP) * 8
	 *         RDN_DELAY = -----------------------
	 *                                RP
	 */
	if (period_ps > dll_threshold_ps) {
		use_half_period = true;
		reference_period_ps = period_ps / 2;
	} else {
		use_half_period = false;
		reference_period_ps = period_ps;
	}

	tRP_ps = data_setup_cycles * period_ps;
	sample_delay_ps = (sdr->tREA_max + 4000 - tRP_ps) * 8;
	if (sample_delay_ps > 0)
		sample_delay_factor = sample_delay_ps / reference_period_ps;
	else
		sample_delay_factor = 0;

	ctrl1n = (wrn_dly_sel << GPMI_CTRL1_WRN_DLY_SEL_OFFSET);
	if (sample_delay_factor)
		ctrl1n |= (sample_delay_factor << GPMI_CTRL1_RDN_DELAY_OFFSET) |
			GPMI_CTRL1_DLL_ENABLE |
			(use_half_period ? GPMI_CTRL1_HALF_PERIOD : 0);

	writel(timing0, &nand_info->gpmi_regs->hw_gpmi_timing0);
	writel(timing1, &nand_info->gpmi_regs->hw_gpmi_timing1);

	/*
	 * Clear several CTRL1 fields, DLL must be disabled when setting
	 * RDN_DELAY or HALF_PERIOD.
	 */
	writel(GPMI_CTRL1_CLEAR_MASK, &nand_info->gpmi_regs->hw_gpmi_ctrl1_clr);
	writel(ctrl1n, &nand_info->gpmi_regs->hw_gpmi_ctrl1_set);

	clk_set_rate(nand_info->gpmi_clk, clk_rate);

	/* Wait 64 clock cycles before using the GPMI after enabling the DLL */
	dll_wait_time_us = USEC_PER_SEC / clk_rate * 64;
	if (!dll_wait_time_us)
		dll_wait_time_us = 1;

	/* Wait for the DLL to settle. */
	udelay(dll_wait_time_us);
}

static int mxs_nand_setup_interface(struct mtd_info *mtd, int chipnr,
				    const struct nand_data_interface *conf)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	const struct nand_sdr_timings *sdr;

	sdr = nand_get_sdr_timings(conf);
	if (IS_ERR(sdr))
		return PTR_ERR(sdr);

	/* Stop here if this call was just a check */
	if (chipnr < 0)
		return 0;

	/* Do the actual derivation of the controller timings */
	mxs_compute_timings(chip, sdr);

	return 0;
}

int mxs_nand_init_spl(struct nand_chip *nand)
{
	struct mxs_nand_info *nand_info;
	int err;

	nand_info = malloc(sizeof(struct mxs_nand_info));
	if (!nand_info) {
		printf("MXS NAND: Failed to allocate private data\n");
		return -ENOMEM;
	}
	memset(nand_info, 0, sizeof(struct mxs_nand_info));

	nand_info->gpmi_regs = (struct mxs_gpmi_regs *)MXS_GPMI_BASE;
	nand_info->bch_regs = (struct mxs_bch_regs *)MXS_BCH_BASE;

	if (is_mx6sx() || is_mx7() || is_imx8() || is_imx8m())
		nand_info->max_ecc_strength_supported = 62;
	else
		nand_info->max_ecc_strength_supported = 40;

	if (IS_ENABLED(CONFIG_NAND_MXS_USE_MINIMUM_ECC))
		nand_info->use_minimum_ecc = true;

	err = mxs_nand_alloc_buffers(nand_info);
	if (err)
		return err;

	err = mxs_nand_init_dma(nand_info);
	if (err)
		return err;

	nand_set_controller_data(nand, nand_info);

	nand->options |= NAND_NO_SUBPAGE_WRITE;

	nand->cmd_ctrl		= mxs_nand_cmd_ctrl;
	nand->dev_ready		= mxs_nand_device_ready;
	nand->select_chip	= mxs_nand_select_chip;

	nand->read_byte		= mxs_nand_read_byte;
	nand->read_buf		= mxs_nand_read_buf;

	nand->ecc.read_page	= mxs_nand_ecc_read_page;

	nand->ecc.mode		= NAND_ECC_HW;

	return 0;
}

int mxs_nand_init_ctrl(struct mxs_nand_info *nand_info)
{
	struct mtd_info *mtd;
	struct nand_chip *nand;
	int err;

	nand = &nand_info->chip;
	mtd = nand_to_mtd(nand);
	err = mxs_nand_alloc_buffers(nand_info);
	if (err)
		return err;

	err = mxs_nand_init_dma(nand_info);
	if (err)
		goto err_free_buffers;

	memset(&fake_ecc_layout, 0, sizeof(fake_ecc_layout));

#ifdef CONFIG_SYS_NAND_USE_FLASH_BBT
	nand->bbt_options |= NAND_BBT_USE_FLASH | NAND_BBT_NO_OOB;
#endif

	nand_set_controller_data(nand, nand_info);
	nand->options |= NAND_NO_SUBPAGE_WRITE;

	if (nand_info->dev)
		nand->flash_node = dev_ofnode(nand_info->dev);

	nand->cmd_ctrl		= mxs_nand_cmd_ctrl;

	nand->dev_ready		= mxs_nand_device_ready;
	nand->select_chip	= mxs_nand_select_chip;
	nand->block_bad		= mxs_nand_block_bad;

	nand->read_byte		= mxs_nand_read_byte;

	nand->read_buf		= mxs_nand_read_buf;
	nand->write_buf		= mxs_nand_write_buf;

	if (nand_info->gpmi_clk)
		nand->setup_data_interface = mxs_nand_setup_interface;

	/* first scan to find the device and get the page size */
	if (nand_scan_ident(mtd, CONFIG_SYS_MAX_NAND_DEVICE, NULL))
		goto err_free_buffers;

	if (mxs_nand_setup_ecc(mtd))
		goto err_free_buffers;

	nand->ecc.read_page	= mxs_nand_ecc_read_page;
	nand->ecc.write_page	= mxs_nand_ecc_write_page;
	nand->ecc.read_oob	= mxs_nand_ecc_read_oob;
	nand->ecc.write_oob	= mxs_nand_ecc_write_oob;

	nand->ecc.layout	= &fake_ecc_layout;
	nand->ecc.mode		= NAND_ECC_HW;
	nand->ecc.size		= nand_info->bch_geometry.ecc_chunkn_size;
	nand->ecc.strength	= nand_info->bch_geometry.ecc_strength;

	/* second phase scan */
	err = nand_scan_tail(mtd);
	if (err)
		goto err_free_buffers;

	/* Hook some operations at the MTD level. */
	if (mtd->_read_oob != mxs_nand_hook_read_oob) {
		nand_info->hooked_read_oob = mtd->_read_oob;
		mtd->_read_oob = mxs_nand_hook_read_oob;
	}

	if (mtd->_write_oob != mxs_nand_hook_write_oob) {
		nand_info->hooked_write_oob = mtd->_write_oob;
		mtd->_write_oob = mxs_nand_hook_write_oob;
	}

	if (mtd->_block_markbad != mxs_nand_hook_block_markbad) {
		nand_info->hooked_block_markbad = mtd->_block_markbad;
		mtd->_block_markbad = mxs_nand_hook_block_markbad;
	}

	err = nand_register(0, mtd);
	if (err)
		goto err_free_buffers;

	return 0;

err_free_buffers:
	free(nand_info->data_buf);
	free(nand_info->cmd_buf);

	return err;
}

#ifndef CONFIG_NAND_MXS_DT
void board_nand_init(void)
{
	struct mxs_nand_info *nand_info;

	nand_info = malloc(sizeof(struct mxs_nand_info));
	if (!nand_info) {
		printf("MXS NAND: Failed to allocate private data\n");
			return;
	}
	memset(nand_info, 0, sizeof(struct mxs_nand_info));

	nand_info->gpmi_regs = (struct mxs_gpmi_regs *)MXS_GPMI_BASE;
	nand_info->bch_regs = (struct mxs_bch_regs *)MXS_BCH_BASE;

	/* Refer to Chapter 17 for i.MX6DQ, Chapter 18 for i.MX6SX */
	if (is_mx6sx() || is_mx7())
		nand_info->max_ecc_strength_supported = 62;
	else
		nand_info->max_ecc_strength_supported = 40;

#ifdef CONFIG_NAND_MXS_USE_MINIMUM_ECC
	nand_info->use_minimum_ecc = true;
#endif

	if (mxs_nand_init_ctrl(nand_info) < 0)
		goto err;

	return;

err:
	free(nand_info);
}
#endif

/*
 * Read NAND layout for FCB block generation.
 */
void mxs_nand_get_layout(struct mtd_info *mtd, struct mxs_nand_layout *l)
{
	struct mxs_bch_regs *bch_regs = (struct mxs_bch_regs *)MXS_BCH_BASE;
	u32 tmp;

	tmp = readl(&bch_regs->hw_bch_flash0layout0);
	l->nblocks = (tmp & BCH_FLASHLAYOUT0_NBLOCKS_MASK) >>
			BCH_FLASHLAYOUT0_NBLOCKS_OFFSET;
	l->meta_size = (tmp & BCH_FLASHLAYOUT0_META_SIZE_MASK) >>
			BCH_FLASHLAYOUT0_META_SIZE_OFFSET;

	tmp = readl(&bch_regs->hw_bch_flash0layout1);
	l->data0_size = 4 * ((tmp & BCH_FLASHLAYOUT0_DATA0_SIZE_MASK) >>
			BCH_FLASHLAYOUT0_DATA0_SIZE_OFFSET);
	l->ecc0 = (tmp & BCH_FLASHLAYOUT0_ECC0_MASK) >>
			BCH_FLASHLAYOUT0_ECC0_OFFSET;
	l->datan_size = 4 * ((tmp & BCH_FLASHLAYOUT1_DATAN_SIZE_MASK) >>
			BCH_FLASHLAYOUT1_DATAN_SIZE_OFFSET);
	l->eccn = (tmp & BCH_FLASHLAYOUT1_ECCN_MASK) >>
			BCH_FLASHLAYOUT1_ECCN_OFFSET;
	l->gf_len = (tmp & BCH_FLASHLAYOUT1_GF13_0_GF14_1_MASK) >>
		     BCH_FLASHLAYOUT1_GF13_0_GF14_1_OFFSET;
}

/*
 * Set BCH to specific layout used by ROM bootloader to read FCB.
 */
void mxs_nand_mode_fcb_62bit(struct mtd_info *mtd)
{
	u32 tmp;
	struct mxs_bch_regs *bch_regs = (struct mxs_bch_regs *)MXS_BCH_BASE;
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);

	nand_info->en_randomizer = 1;

	mtd->writesize = 1024;
	mtd->oobsize = 1862 - 1024;

	/* 8 ecc_chunks_*/
	tmp = 7	<< BCH_FLASHLAYOUT0_NBLOCKS_OFFSET;
	/* 32 bytes for metadata */
	tmp |= 32 << BCH_FLASHLAYOUT0_META_SIZE_OFFSET;
	/* using ECC62 level to be performed */
	tmp |= 0x1F << BCH_FLASHLAYOUT0_ECC0_OFFSET;
	/* 0x20 * 4 bytes of the data0 block */
	tmp |= 0x20 << BCH_FLASHLAYOUT0_DATA0_SIZE_OFFSET;
	tmp |= 0 << BCH_FLASHLAYOUT0_GF13_0_GF14_1_OFFSET;
	writel(tmp, &bch_regs->hw_bch_flash0layout0);

	/* 1024 for data + 838 for OOB */
	tmp = 1862 << BCH_FLASHLAYOUT1_PAGE_SIZE_OFFSET;
	/* using ECC62 level to be performed */
	tmp |= 0x1F << BCH_FLASHLAYOUT1_ECCN_OFFSET;
	/* 0x20 * 4 bytes of the data0 block */
	tmp |= 0x20 << BCH_FLASHLAYOUT1_DATAN_SIZE_OFFSET;
	tmp |= 0 << BCH_FLASHLAYOUT1_GF13_0_GF14_1_OFFSET;
	writel(tmp, &bch_regs->hw_bch_flash0layout1);
}

/*
 * Set BCH to specific layout used by ROM bootloader to read FCB.
 */
void mxs_nand_mode_fcb_40bit(struct mtd_info *mtd)
{
	u32 tmp;
	struct mxs_bch_regs *bch_regs = (struct mxs_bch_regs *)MXS_BCH_BASE;
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);

	/* no randomizer in this setting*/
	nand_info->en_randomizer = 0;

	mtd->writesize = 1024;
	mtd->oobsize = 1576 - 1024;

	/* 8 ecc_chunks_*/
	tmp = 7	<< BCH_FLASHLAYOUT0_NBLOCKS_OFFSET;
	/* 32 bytes for metadata */
	tmp |= 32 << BCH_FLASHLAYOUT0_META_SIZE_OFFSET;
	/* using ECC40 level to be performed */
	tmp |= 0x14 << BCH_FLASHLAYOUT0_ECC0_OFFSET;
	/* 0x20 * 4 bytes of the data0 block */
	tmp |= 0x20 << BCH_FLASHLAYOUT0_DATA0_SIZE_OFFSET;
	tmp |= 0 << BCH_FLASHLAYOUT0_GF13_0_GF14_1_OFFSET;
	writel(tmp, &bch_regs->hw_bch_flash0layout0);

	/* 1024 for data + 552 for OOB */
	tmp = 1576 << BCH_FLASHLAYOUT1_PAGE_SIZE_OFFSET;
	/* using ECC40 level to be performed */
	tmp |= 0x14 << BCH_FLASHLAYOUT1_ECCN_OFFSET;
	/* 0x20 * 4 bytes of the data0 block */
	tmp |= 0x20 << BCH_FLASHLAYOUT1_DATAN_SIZE_OFFSET;
	tmp |= 0 << BCH_FLASHLAYOUT1_GF13_0_GF14_1_OFFSET;
	writel(tmp, &bch_regs->hw_bch_flash0layout1);
}

/*
 * Restore BCH to normal settings.
 */
void mxs_nand_mode_normal(struct mtd_info *mtd)
{
	struct mxs_bch_regs *bch_regs = (struct mxs_bch_regs *)MXS_BCH_BASE;
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(nand);

	nand_info->en_randomizer = 0;

	mtd->writesize = nand_info->writesize;
	mtd->oobsize = nand_info->oobsize;

	writel(nand_info->bch_flash0layout0, &bch_regs->hw_bch_flash0layout0);
	writel(nand_info->bch_flash0layout1, &bch_regs->hw_bch_flash0layout1);
}

uint32_t mxs_nand_mark_byte_offset(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(chip);
	struct bch_geometry *geo = &nand_info->bch_geometry;

	return geo->block_mark_byte_offset;
}

uint32_t mxs_nand_mark_bit_offset(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(chip);
	struct bch_geometry *geo = &nand_info->bch_geometry;

	return geo->block_mark_bit_offset;
}
