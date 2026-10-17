// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020, Cortina Access Inc..
 */

#include <common.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/sizes.h>
#include <log.h>
#include <asm/io.h>
#include <memalign.h>
#include <nand.h>
#include <dm/device_compat.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/errno.h>
#include <linux/mtd/rawnand.h>
#include <asm/gpio.h>
#include <fdtdec.h>
#include <bouncebuf.h>
#include <dm.h>
#include "cortina_nand.h"

static unsigned int *pread, *pwrite;

static const struct udevice_id cortina_nand_dt_ids[] = {
	{
	 .compatible = "cortina,ca-nand",
	 },
	{ /* sentinel */ }
};

static struct nand_ecclayout eccoob;

/* Information about an attached NAND chip */
struct fdt_nand {
	int enabled;		/* 1 to enable, 0 to disable */
	s32 width;		/* bit width, must be 8 */
	u32 nand_ecc_strength;
};

struct nand_drv {
	u32 fifo_index;
	struct nand_ctlr *reg;
	struct dma_global *dma_glb;
	struct dma_ssp *dma_nand;
	struct tx_descriptor_t *tx_desc;
	struct rx_descriptor_t *rx_desc;
	struct fdt_nand config;
	unsigned int flash_base;
};

struct ca_nand_info {
	struct udevice *dev;
	struct nand_drv nand_ctrl;
	struct nand_chip nand_chip;
};

/**
 * Wait for command completion
 *
 * @param reg	nand_ctlr structure
 * @return
 *	1 - Command completed
 *	0 - Timeout
 */
static int nand_waitfor_cmd_completion(struct nand_ctlr *reg, unsigned int mask)
{
	unsigned int reg_v = 0;

	if (readl_poll_timeout(&reg->flash_flash_access_start, reg_v,
			       !(reg_v & mask), (FLASH_LONG_DELAY << 2))) {
		pr_err("Nand CMD timeout!\n");
		return 0;
	}

	return 1;
}

/**
 * Read one byte from the chip
 *
 * @param mtd	MTD device structure
 * Return:	data byte
 *
 * Read function for 8bit bus-width
 */
static uint8_t read_byte(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct nand_drv *info;
	u8 ret_v;

	info = (struct nand_drv *)nand_get_controller_data(chip);

	clrsetbits_le32(&info->reg->flash_flash_access_start, GENMASK(31, 0),
			NFLASH_GO | NFLASH_RD);

	if (!nand_waitfor_cmd_completion(info->reg, NFLASH_GO))
		printf("%s: Command timeout\n", __func__);

	ret_v = readl(&info->reg->flash_nf_data) >> (8 * info->fifo_index++);
	info->fifo_index %= 4;

	return (uint8_t)ret_v;
}

/**
 * Read len bytes from the chip into a buffer
 *
 * @param mtd	MTD device structure
 * @param buf	buffer to store data to
 * @param len	number of bytes to read
 *
 * Read function for 8bit bus-width
 */
static void read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	int i;
	unsigned int reg;
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(chip);

	for (i = 0; i < len; i++) {
		clrsetbits_le32(&info->reg->flash_flash_access_start,
				GENMASK(31, 0), NFLASH_GO | NFLASH_RD);

		if (!nand_waitfor_cmd_completion(info->reg, NFLASH_GO))
			printf("%s: Command timeout\n", __func__);

		reg = readl(&info->reg->flash_nf_data) >>
		    (8 * info->fifo_index++);
		memcpy(buf + i, &reg, 1);
		info->fifo_index %= 4;
	}
}

/**
 * Check READY pin status to see if it is ready or not
 *
 * @param mtd	MTD device structure
 * @return
 *	1 - ready
 *	0 - not ready
 */
static int nand_dev_ready(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	int reg_val;
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(chip);

	reg_val = readl(&info->reg->flash_status);
	if (reg_val & NFLASH_READY)
		return 1;
	else
		return 0;
}

/* Dummy implementation: we don't support multiple chips */
static void nand_select_chip(struct mtd_info *mtd, int chipnr)
{
	switch (chipnr) {
	case -1:
	case 0:
		break;

	default:
		WARN_ON(chipnr);
	}
}

int init_nand_dma(struct nand_chip *nand)
{
	int i;
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(nand);

	setbits_le32(&info->dma_glb->dma_glb_dma_lso_ctrl, TX_DMA_ENABLE);
	setbits_le32(&info->dma_glb->dma_glb_dma_ssp_rx_ctrl,
		     TX_DMA_ENABLE | DMA_CHECK_OWNER);
	setbits_le32(&info->dma_glb->dma_glb_dma_ssp_tx_ctrl,
		     RX_DMA_ENABLE | DMA_CHECK_OWNER);

	info->tx_desc = malloc_cache_aligned((sizeof(struct tx_descriptor_t) *
					      CA_DMA_DESC_NUM));
	info->rx_desc = malloc_cache_aligned((sizeof(struct rx_descriptor_t) *
					      CA_DMA_DESC_NUM));

	if (!info->rx_desc && info->tx_desc) {
		printf("Fail to alloc DMA descript!\n");
		kfree(info->tx_desc);
		return -ENOMEM;
	} else if (info->rx_desc && !info->tx_desc) {
		printf("Fail to alloc DMA descript!\n");
		kfree(info->tx_desc);
		return -ENOMEM;
	}

	/* set RX DMA base address and depth */
	clrsetbits_le32(&info->dma_nand->dma_q_rxq_base_depth,
			GENMASK(31, 4), (uintptr_t)info->rx_desc);
	clrsetbits_le32(&info->dma_nand->dma_q_rxq_base_depth,
			GENMASK(3, 0), CA_DMA_DEPTH);

	/* set TX DMA base address and depth */
	clrsetbits_le32(&info->dma_nand->dma_q_txq_base_depth,
			GENMASK(31, 4), (uintptr_t)info->tx_desc);
	clrsetbits_le32(&info->dma_nand->dma_q_txq_base_depth,
			GENMASK(3, 0), CA_DMA_DEPTH);

	memset((unsigned char *)info->tx_desc, 0,
	       (sizeof(struct tx_descriptor_t) * CA_DMA_DESC_NUM));
	memset((unsigned char *)info->rx_desc, 0,
	       (sizeof(struct rx_descriptor_t) * CA_DMA_DESC_NUM));

	for (i = 0; i < CA_DMA_DESC_NUM; i++) {
		/* set owner bit as SW */
		info->tx_desc[i].own = 1;
		/* enable Scatter-Gather memory copy */
		info->tx_desc[i].sgm = 0x1;
	}

	return 0;
}

/**
 * Send command to NAND device
 *
 * @param mtd		MTD device structure
 * @param command	the command to be sent
 * @param column	the column address for this command, -1 if none
 * @param page_addr	the page address for this command, -1 if none
 */
static void ca_nand_command(struct mtd_info *mtd, unsigned int command,
			    int column, int page_addr)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct nand_drv *info;
	unsigned int reg_v = 0;
	u32 cmd = 0, cnt = 0, addr1 = 0, addr2 = 0;
	int ret;

	info = (struct nand_drv *)nand_get_controller_data(chip);
	/*
	 * Write out the command to the device.
	 *
	 * Only command NAND_CMD_RESET or NAND_CMD_READID will come
	 * here before mtd->writesize is initialized.
	 */

	/* Emulate NAND_CMD_READOOB */
	if (command == NAND_CMD_READOOB) {
		assert(mtd->writesize != 0);
		column += mtd->writesize;
		command = NAND_CMD_READ0;
	}

	/* Reset FIFO before issue new command */
	clrsetbits_le32(&info->reg->flash_nf_ecc_reset, GENMASK(31, 0),
			ECC_RESET_ALL);
	ret =
	    readl_poll_timeout(&info->reg->flash_nf_ecc_reset, reg_v,
			       !(reg_v & RESET_NFLASH_FIFO), FLASH_SHORT_DELAY);
	if (ret) {
		printf("FIFO reset timeout\n");
		clrsetbits_le32(&info->reg->flash_nf_ecc_reset, GENMASK(31, 0),
				ECC_RESET_ALL);
		udelay(10);
	}

	/* Reset FIFO index
	 * Next read start from flash_nf_data[0]
	 */
	info->fifo_index = 0;

	clrsetbits_le32(&info->reg->flash_nf_access, GENMASK(11, 10),
			NFLASH_REG_WIDTH_8);

	/*
	 * Program and erase have their own busy handlers
	 * status and sequential in needs no delay
	 */
	switch (command) {
	case NAND_CMD_READID:
		/* Command */
		clrsetbits_le32(&info->reg->flash_nf_command, GENMASK(31, 0),
				NAND_CMD_READID);
		/* 1 byte CMD cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(1, 0),
				REG_CMD_COUNT_1TOGO);
		/* 1 byte CMD cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(6, 4),
				REG_ADDR_COUNT_1);
		/* Data cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(21, 8),
				REG_DATA_COUNT_DATA_4);
		/* 0 OOB cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(31, 22),
				REG_OOB_COUNT_EMPTY);

		/* addresses */
		clrsetbits_le32(&info->reg->flash_nf_address_1, GENMASK(31, 0),
				column & ADDR1_MASK2);
		clrsetbits_le32(&info->reg->flash_nf_address_2, GENMASK(31, 0),
				0);

		/* clear FLASH_NF_ACCESS */
		clrsetbits_le32(&info->reg->flash_nf_access, GENMASK(31, 0),
				DISABLE_AUTO_RESET);

		break;
	case NAND_CMD_PARAM:
		/* Command */
		clrsetbits_le32(&info->reg->flash_nf_command, GENMASK(31, 0),
				NAND_CMD_PARAM);
		/* 1 byte CMD cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(1, 0),
				REG_CMD_COUNT_1TOGO);
		/* 1 byte ADDR cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(6, 4),
				REG_ADDR_COUNT_1);
		/* Data cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(21, 8),
				(SZ_4K - 1) << 8);
		/* 0 OOB cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(31, 22),
				REG_OOB_COUNT_EMPTY);

		/* addresses */
		clrsetbits_le32(&info->reg->flash_nf_address_1, GENMASK(31, 0),
				column & ADDR1_MASK2);
		clrsetbits_le32(&info->reg->flash_nf_address_2, GENMASK(31, 0),
				0);

		break;
	case NAND_CMD_READ0:
		if (chip->chipsize < SZ_32M) {
			cmd = NAND_CMD_READ0;
			cnt = REG_CMD_COUNT_1TOGO | REG_ADDR_COUNT_3;
			addr1 = (((page_addr & ADDR1_MASK0) << 8));
			addr2 = ((page_addr & ADDR2_MASK0) >> 24);
		} else if (chip->chipsize >= SZ_32M &&
			   (chip->chipsize <= SZ_128M)) {
			cmd = NAND_CMD_READ0;
			cnt = REG_ADDR_COUNT_4;
			if (mtd->writesize > (REG_DATA_COUNT_512_DATA >> 8)) {
				cmd |= (NAND_CMD_READSTART << 8);
				cnt |= REG_CMD_COUNT_2TOGO;
			} else {
				cnt |= REG_CMD_COUNT_1TOGO;
			}
			addr1 = ((page_addr << 16) | (column & ADDR1_MASK1));
			addr2 = (page_addr >> 16);
		} else {
			cmd = NAND_CMD_READ0 | (NAND_CMD_READSTART << 8);
			cnt = REG_CMD_COUNT_2TOGO | REG_ADDR_COUNT_5;
			addr1 = ((page_addr << 16) | (column & ADDR1_MASK1));
			addr2 = (page_addr >> 16);
		}

		/* Command */
		clrsetbits_le32(&info->reg->flash_nf_command, GENMASK(31, 0),
				cmd);
		/* CMD & ADDR cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(7, 0), cnt);
		/* Data cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(21, 8),
				(mtd->writesize - 1) << 8);
		/* OOB cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(31, 22),
				(mtd->oobsize - 1) << 22);

		/* addresses */
		clrsetbits_le32(&info->reg->flash_nf_address_1, GENMASK(31, 0),
				addr1);
		clrsetbits_le32(&info->reg->flash_nf_address_2, GENMASK(31, 0),
				addr2);

		return;
	case NAND_CMD_SEQIN:
		if (chip->chipsize < SZ_32M) {
			cnt = REG_CMD_COUNT_2TOGO | REG_ADDR_COUNT_3;
			addr1 = (((page_addr & ADDR1_MASK0) << 8));
			addr2 = ((page_addr & ADDR2_MASK0) >> 24);
		} else if (chip->chipsize >= SZ_32M &&
			   (chip->chipsize <= SZ_128M)) {
			cnt = REG_CMD_COUNT_2TOGO | REG_ADDR_COUNT_4;
			addr1 = ((page_addr << 16) | (column & ADDR1_MASK1));
			addr2 = (page_addr >> 16);
		} else {
			cnt = REG_CMD_COUNT_2TOGO | REG_ADDR_COUNT_5;
			addr1 = ((page_addr << 16) | (column & ADDR1_MASK1));
			addr2 = (page_addr >> 16);
		}

		/* Command */
		clrsetbits_le32(&info->reg->flash_nf_command, GENMASK(31, 0),
				NAND_CMD_SEQIN | (NAND_CMD_PAGEPROG << 8));
		/* CMD cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(7, 0), cnt);
		/* Data cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(21, 8),
				(mtd->writesize - 1) << 8);
		/* OOB cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(31, 22),
				(mtd->oobsize - 1) << 22);

		/* addresses */
		clrsetbits_le32(&info->reg->flash_nf_address_1, GENMASK(31, 0),
				addr1);
		clrsetbits_le32(&info->reg->flash_nf_address_2, GENMASK(31, 0),
				addr2);

		return;
	case NAND_CMD_PAGEPROG:
		return;
	case NAND_CMD_ERASE1:
		/* Command */
		clrsetbits_le32(&info->reg->flash_nf_command, GENMASK(31, 0),
				NAND_CMD_ERASE1 | (NAND_CMD_ERASE2 << 8));
		/* 2 byte CMD cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(1, 0),
				REG_CMD_COUNT_2TOGO);
		/* 3 byte ADDR cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(6, 4),
				REG_ADDR_COUNT_3);
		/* 0 Data cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(21, 8),
				REG_DATA_COUNT_EMPTY);
		/* 0 OOB cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(31, 22),
				REG_OOB_COUNT_EMPTY);

		/* addresses */
		clrsetbits_le32(&info->reg->flash_nf_address_1, GENMASK(31, 0),
				page_addr);
		clrsetbits_le32(&info->reg->flash_nf_address_2, GENMASK(31, 0),
				0);

		/* Issue command */
		clrsetbits_le32(&info->reg->flash_flash_access_start,
				GENMASK(31, 0), NFLASH_GO | NFLASH_RD);
		break;
	case NAND_CMD_ERASE2:
		return;
	case NAND_CMD_STATUS:
		/* Command */
		clrsetbits_le32(&info->reg->flash_nf_command, GENMASK(31, 0),
				NAND_CMD_STATUS);
		/* 1 byte CMD cycle */
		clrbits_le32(&info->reg->flash_nf_count, GENMASK(1, 0));
		/* 0 byte Addr cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(6, 4),
				REG_ADDR_COUNT_EMPTY);
		/* 1 Data cycle */
		clrbits_le32(&info->reg->flash_nf_count, GENMASK(21, 8));
		/* 0 OOB cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(31, 22),
				REG_OOB_COUNT_EMPTY);

		break;
	case NAND_CMD_RESET:
		/* Command */
		clrsetbits_le32(&info->reg->flash_nf_command, GENMASK(31, 0),
				NAND_CMD_RESET);
		/* 1 byte CMD cycle */
		clrbits_le32(&info->reg->flash_nf_count, GENMASK(1, 0));
		/* 0 byte Addr cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(6, 4),
				REG_ADDR_COUNT_EMPTY);
		/* 0 Data cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(21, 8),
				REG_DATA_COUNT_EMPTY);
		/* 0 OOB cycle */
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(31, 22),
				REG_OOB_COUNT_EMPTY);

		/* addresses */
		clrsetbits_le32(&info->reg->flash_nf_address_1, GENMASK(31, 0),
				column & ADDR1_MASK2);
		clrsetbits_le32(&info->reg->flash_nf_address_2, GENMASK(31, 0),
				0);

		/* Issue command */
		clrsetbits_le32(&info->reg->flash_flash_access_start,
				GENMASK(31, 0), NFLASH_GO | NFLASH_WT);

		break;
	case NAND_CMD_RNDOUT:
	default:
		printf("%s: Unsupported command %d\n", __func__, command);
		return;
	}

	if (!nand_waitfor_cmd_completion(info->reg, NFLASH_GO))
		printf("Command 0x%02X timeout\n", command);
}

/**
 * Set up NAND bus width and page size
 *
 * @param info		nand_info structure
 * Return: 0 if ok, -1 on error
 */
static int set_bus_width_page_size(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(chip);

	if (info->config.width == SZ_8) {
		clrsetbits_le32(&info->reg->flash_nf_access, GENMASK(31, 0),
				NFLASH_REG_WIDTH_8);
	} else if (info->config.width == SZ_16) {
		clrsetbits_le32(&info->reg->flash_nf_access, GENMASK(31, 0),
				NFLASH_REG_WIDTH_16);
	} else {
		debug("%s: Unsupported bus width %d\n", __func__,
		      info->config.width);
		return -1;
	}

	if (mtd->writesize == SZ_512) {
		setbits_le32(&info->reg->flash_type, FLASH_TYPE_512);
	} else if (mtd->writesize == SZ_2K) {
		setbits_le32(&info->reg->flash_type, FLASH_TYPE_2K);
	} else if (mtd->writesize == SZ_4K) {
		setbits_le32(&info->reg->flash_type, FLASH_TYPE_4K);
	} else if (mtd->writesize == SZ_8K) {
		setbits_le32(&info->reg->flash_type, FLASH_TYPE_8K);
	} else {
		debug("%s: Unsupported page size %d\n", __func__,
		      mtd->writesize);
		return -1;
	}

	return 0;
}

static int ca_do_bch_correction(struct nand_chip *chip,
				unsigned int err_num, u8 *buff_ptr, int i)
{
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(chip);
	unsigned int reg_v, err_loc0, err_loc1;
	int k, max_bitflips = 0;

	for (k = 0; k < (err_num + 1) / 2; k++) {
		reg_v = readl(&info->reg->flash_nf_bch_error_loc01 + k);
		err_loc0 = reg_v & BCH_ERR_LOC_MASK;
		err_loc1 = (reg_v >> 16) & BCH_ERR_LOC_MASK;

		if (err_loc0 / 8 < BCH_DATA_UNIT) {
			printf("pdata[%x]:%x =>", ((i / chip->ecc.bytes) *
				chip->ecc.size + ((reg_v & 0x1fff) >> 3)),
				buff_ptr[(reg_v & 0x1fff) >> 3]);

			buff_ptr[err_loc0 / 8] ^=
				(1 << (reg_v & BCH_CORRECT_LOC_MASK));

			printf("%x\n", buff_ptr[(reg_v & 0x1fff) >> 3]);

			max_bitflips++;
		}

		if (((k + 1) * 2) <= err_num && ((err_loc1 / 8) <
						 BCH_DATA_UNIT)) {
			printf("pdata[%x]:%x =>", ((i / chip->ecc.bytes) *
				chip->ecc.size + (((reg_v >> 16) & 0x1fff) >>
				3)), buff_ptr[((reg_v >> 16) & 0x1fff) >> 3]);

			buff_ptr[err_loc1 / 8] ^= (1 << ((reg_v >> 16) &
						   BCH_CORRECT_LOC_MASK));

			printf("%x\n", buff_ptr[((reg_v >> 16) & 0x1fff) >> 3]);

			max_bitflips++;
		}
	}

	return max_bitflips;
}

static int ca_do_bch_decode(struct mtd_info *mtd, struct nand_chip *chip,
			    const u8 *buf, int page, unsigned int addr)
{
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(chip);
	unsigned int reg_v, err_num;
	unsigned char *ecc_code = chip->buffers->ecccode;
	unsigned char *ecc_end_pos;
	int ret, i, j, k, n, step, eccsteps, max_bitflips = 0;
	u8 *buff_ptr = (u8 *)buf;

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = chip->oob_poi[eccoob.eccpos[i]];

	for (i = 0, eccsteps = chip->ecc.steps; eccsteps;
	     i += chip->ecc.bytes, eccsteps--) {
		ecc_end_pos = ecc_code + chip->ecc.bytes;

		for (j = 0, k = 0; j < chip->ecc.bytes; j += 4, k++) {
			reg_v = 0;
			for (n = 0; n < 4 && ecc_code != ecc_end_pos;
			     ++n, ++ecc_code) {
				reg_v |= *ecc_code << (8 * n);
			}
			clrsetbits_le32(&info->reg->flash_nf_bch_oob0 + k,
					GENMASK(31, 0), reg_v);
		}

		/* Clear ECC buffer */
		setbits_le32(&info->reg->flash_nf_ecc_reset, RESET_NFLASH_ECC);
		ret = readl_poll_timeout(&info->reg->flash_nf_ecc_reset, reg_v,
					 !(reg_v & RESET_NFLASH_ECC),
					 FLASH_SHORT_DELAY);
		if (ret)
			pr_err("Reset ECC buffer fail\n");

		clrsetbits_le32(&info->reg->flash_nf_bch_control, GENMASK(8, 8),
				BCH_DISABLE);

		/* Start BCH */
		step = i / chip->ecc.bytes;
		clrsetbits_le32(&info->reg->flash_nf_bch_control,
				GENMASK(6, 4), step << 4);
		setbits_le32(&info->reg->flash_nf_bch_control, BCH_ENABLE);
		udelay(10);
		setbits_le32(&info->reg->flash_nf_bch_control, BCH_COMPARE);

		ret = readl_poll_timeout(&info->reg->flash_nf_bch_status, reg_v,
					 (reg_v & BCH_DECO_DONE),
					 FLASH_SHORT_DELAY);
		if (ret)
			pr_err("ECC Decode timeout\n");

		/* Stop compare */
		clrbits_le32(&info->reg->flash_nf_bch_control, BCH_COMPARE);

		reg_v = readl(&info->reg->flash_nf_bch_status);
		err_num = (reg_v >> 8) & BCH_ERR_NUM_MASK;
		reg_v &= BCH_ERR_MASK;

		/* Uncorrectable */
		if (reg_v == BCH_UNCORRECTABLE) {
			max_bitflips =
			nand_check_erased_ecc_chunk(buff_ptr,
						    chip->ecc.size,
						    &chip->buffers->ecccode[i],
						    chip->ecc.bytes,
						    NULL, 0,
						    chip->ecc.strength);

			if (max_bitflips) {
				mtd->ecc_stats.failed++;
				pr_err("Uncorrectable error\n");
				pr_err(" Page:%x  step:%d\n", page, step);

				return -1;
			}
		} else if (reg_v == BCH_CORRECTABLE_ERR) {
			printf("Correctable error(%x)!! addr:%lx\n",
			       err_num, (unsigned long)addr - mtd->writesize);
			printf("Dst buf: %p [ColSel:%x ]\n",
			       buff_ptr + reg_v * BCH_DATA_UNIT, step);

			max_bitflips =
			   ca_do_bch_correction(chip, err_num, buff_ptr, i);
		}

		buff_ptr += BCH_DATA_UNIT;
	}

	/* Disable BCH */
	clrsetbits_le32(&info->reg->flash_nf_bch_control, GENMASK(31, 0),
			BCH_DISABLE);

	return max_bitflips;
}

static int ca_do_bch_encode(struct mtd_info *mtd, struct nand_chip *chip,
			    int page)
{
	struct nand_drv *info;
	unsigned int reg_v;
	int i, j, n, eccsteps, gen_index;

	info = (struct nand_drv *)nand_get_controller_data(chip);

	for (i = 0, n = 0, eccsteps = chip->ecc.steps; eccsteps;
	     i += chip->ecc.bytes, eccsteps--, n++) {
		gen_index = 0;
		for (j = 0; j < chip->ecc.bytes; j += 4, gen_index++) {
			reg_v =
			    readl(&info->reg->flash_nf_bch_gen0_0 + gen_index +
				  18 * n);
			chip->oob_poi[eccoob.eccpos[i + j]] = reg_v & OOB_MASK;
			chip->oob_poi[eccoob.eccpos[i + j + 1]] =
			    (reg_v >> 8) & OOB_MASK;
			chip->oob_poi[eccoob.eccpos[i + j + 2]] =
			    (reg_v >> 16) & OOB_MASK;
			chip->oob_poi[eccoob.eccpos[i + j + 3]] =
			    (reg_v >> 24) & OOB_MASK;
		}
	}

	/* Disable BCH */
	clrsetbits_le32(&info->reg->flash_nf_bch_control, GENMASK(8, 8),
			BCH_DISABLE);

	return 0;
}

/**
 * Page read/write function
 *
 * @param mtd		mtd info structure
 * @param chip		nand chip info structure
 * @param buf		data buffer
 * @param page		page number
 * @param with_ecc	1 to enable ECC, 0 to disable ECC
 * @param is_writing	0 for read, 1 for write
 * Return:		0 when successfully completed
 *			-ETIMEDOUT when command timeout
 */
static int nand_rw_page(struct mtd_info *mtd, struct nand_chip *chip,
			const u8 *buf, int page, int with_ecc, int is_writing)
{
	unsigned int reg_v, ext_addr, addr, dma_index;
	struct tx_descriptor_t *tx_desc;
	struct rx_descriptor_t *rx_desc;
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(chip);
	int ret;

	/* reset ecc control */
	clrsetbits_le32(&info->reg->flash_nf_ecc_reset, GENMASK(31, 0),
			RESET_NFLASH_ECC);

	/*  flash interrupt */
	clrsetbits_le32(&info->reg->flash_flash_interrupt, GENMASK(0, 0),
			REGIRQ_CLEAR);

	/* reset ecc control */
	clrsetbits_le32(&info->reg->flash_nf_ecc_reset, GENMASK(31, 0),
			RESET_NFLASH_ECC);

	/* Disable TXQ */
	clrbits_le32(&info->dma_nand->dma_q_txq_control, GENMASK(0, 0));

	/* Clear interrupt */
	setbits_le32(&info->dma_nand->dma_q_rxq_coal_interrupt, GENMASK(0, 0));
	setbits_le32(&info->dma_nand->dma_q_txq_coal_interrupt, GENMASK(0, 0));

	if (with_ecc == 1) {
		switch (info->config.nand_ecc_strength) {
		case ECC_STRENGTH_8:
			reg_v = BCH_ERR_CAP_8;
			break;
		case ECC_STRENGTH_16:
			reg_v = BCH_ERR_CAP_16;
			break;
		case ECC_STRENGTH_24:
			reg_v = BCH_ERR_CAP_24;
			break;
		case ECC_STRENGTH_40:
			reg_v = BCH_ERR_CAP_40;
			break;
		default:
			reg_v = BCH_ERR_CAP_16;
			break;
		}
		reg_v |= BCH_ENABLE;

		/* BCH decode for flash read */
		if (is_writing == 0)
			reg_v |= BCH_DECODE;
		clrsetbits_le32(&info->reg->flash_nf_bch_control,
				GENMASK(31, 0), reg_v);
	} else {
		clrsetbits_le32(&info->reg->flash_nf_bch_control,
				GENMASK(31, 0), 0);
	}

	/* Fill Extend address */
	ext_addr = ((page << chip->page_shift) / EXT_ADDR_MASK);

	clrsetbits_le32(&info->reg->flash_nf_access,
			GENMASK(7, 0), (uintptr_t)ext_addr);

	addr = (uintptr_t)((page << chip->page_shift) % EXT_ADDR_MASK);
	addr = (uintptr_t)(addr + info->flash_base);

	dma_index = readl(&info->dma_nand->dma_q_txq_wptr) & CA_DMA_Q_PTR_MASK;

	tx_desc = info->tx_desc;
	rx_desc = info->rx_desc;

	/* TX/RX descriptor for page data */
	tx_desc[dma_index].own = OWN_DMA;
	tx_desc[dma_index].buf_len = mtd->writesize;
	rx_desc[dma_index].own = OWN_DMA;
	rx_desc[dma_index].buf_len = mtd->writesize;
	if (is_writing == 0) {
		tx_desc[dma_index].buf_adr = (uintptr_t)addr;
		rx_desc[dma_index].buf_adr = (uintptr_t)(buf);
	} else {
		tx_desc[dma_index].buf_adr = (uintptr_t)buf;
		rx_desc[dma_index].buf_adr = (uintptr_t)(addr);
	}

	dma_index++;
	dma_index %= CA_DMA_DESC_NUM;

	/* TX/RX descriptor for OOB area */
	addr = (uintptr_t)(addr + mtd->writesize);
	tx_desc[dma_index].own = OWN_DMA;
	tx_desc[dma_index].buf_len = mtd->oobsize;
	rx_desc[dma_index].own = OWN_DMA;
	rx_desc[dma_index].buf_len = mtd->oobsize;
	if (is_writing) {
		tx_desc[dma_index].buf_adr = (uintptr_t)(chip->oob_poi);
		rx_desc[dma_index].buf_adr = (uintptr_t)addr;
	} else {
		tx_desc[dma_index].buf_adr = (uintptr_t)addr;
		rx_desc[dma_index].buf_adr = (uintptr_t)(chip->oob_poi);
		dma_index++;
		dma_index %= CA_DMA_DESC_NUM;
	}

	if (is_writing == 1) {
		clrsetbits_le32(&info->reg->flash_fifo_control, GENMASK(1, 0),
				FIFO_WRITE);
	} else {
		clrsetbits_le32(&info->reg->flash_fifo_control, GENMASK(1, 0),
				FIFO_READ);
	}

	/* Start FIFO request */
	clrsetbits_le32(&info->reg->flash_flash_access_start, GENMASK(2, 2),
			NFLASH_FIFO_REQ);

	/* Update DMA write pointer */
	clrsetbits_le32(&info->dma_nand->dma_q_txq_wptr, GENMASK(12, 0),
			dma_index);

	/* Start DMA */
	clrsetbits_le32(&info->dma_nand->dma_q_txq_control, GENMASK(0, 0),
			TX_DMA_ENABLE);

	/* Wait TX DMA done */
	ret =
	    readl_poll_timeout(&info->dma_nand->dma_q_txq_coal_interrupt,
			       reg_v, (reg_v & 1), FLASH_LONG_DELAY);
	if (ret) {
		pr_err("TX DMA timeout\n");
		return -ETIMEDOUT;
	}
	/* clear tx interrupt */
	setbits_le32(&info->dma_nand->dma_q_txq_coal_interrupt, 1);

	/* Wait RX DMA done */
	ret =
	    readl_poll_timeout(&info->dma_nand->dma_q_rxq_coal_interrupt, reg_v,
			       (reg_v & 1), FLASH_LONG_DELAY);
	if (ret) {
		pr_err("RX DMA timeout\n");
		return -ETIMEDOUT;
	}
	/* clear rx interrupt */
	setbits_le32(&info->dma_nand->dma_q_rxq_coal_interrupt, 1);

	/* wait NAND CMD done */
	if (is_writing == 0) {
		if (!nand_waitfor_cmd_completion(info->reg, NFLASH_FIFO_REQ))
			printf("%s: Command timeout\n", __func__);
	}

	/* Update DMA read pointer */
	clrsetbits_le32(&info->dma_nand->dma_q_rxq_rptr, GENMASK(12, 0),
			dma_index);

	/* ECC correction */
	if (with_ecc == 1) {
		ret =
		    readl_poll_timeout(&info->reg->flash_nf_bch_status,
				       reg_v, (reg_v & BCH_GEN_DONE),
				       FLASH_LONG_DELAY);

		if (ret) {
			pr_err("BCH_GEN timeout! flash_nf_bch_status=[0x%x]\n",
			       reg_v);
			return -ETIMEDOUT;
		}

		if (is_writing == 0)
			ca_do_bch_decode(mtd, chip, buf, page, addr);
		else
			ca_do_bch_encode(mtd, chip, page);
	}

	if (is_writing) {
		dma_index++;
		dma_index %= CA_DMA_DESC_NUM;

		/* Update DMA R/W pointer */
		clrsetbits_le32(&info->dma_nand->dma_q_txq_wptr, GENMASK(12, 0),
				dma_index);

		/* Wait TX DMA done */
		ret =
		   readl_poll_timeout(&info->dma_nand->dma_q_txq_coal_interrupt,
				      reg_v, (reg_v & 1), FLASH_LONG_DELAY);
		if (ret) {
			pr_err("TX DMA timeout\n");
			return -ETIMEDOUT;
		}
		/* clear tx interrupt */
		setbits_le32(&info->dma_nand->dma_q_txq_coal_interrupt, 1);

		/* Wait RX DMA done */
		ret =
		   readl_poll_timeout(&info->dma_nand->dma_q_rxq_coal_interrupt,
				      reg_v, (reg_v & 1), FLASH_LONG_DELAY);
		if (ret) {
			pr_err("RX DMA timeout\n");
			return -ETIMEDOUT;
		}
		/* clear rx interrupt */
		setbits_le32(&info->dma_nand->dma_q_rxq_coal_interrupt, 1);

		/* wait NAND CMD done */
		if (!nand_waitfor_cmd_completion(info->reg, NFLASH_FIFO_REQ))
			printf("%s: Command timeout\n", __func__);

		/* Update DMA R/W pointer */
		clrsetbits_le32(&info->dma_nand->dma_q_rxq_rptr, GENMASK(12, 0),
				dma_index);
	}

	return 0;
}

/**
 * Hardware ecc based page read function
 *
 * @param mtd	mtd info structure
 * @param chip	nand chip info structure
 * @param buf	buffer to store read data
 * @param page	page number to read
 * Return:	0 when successfully completed
 *		-ETIMEDOUT when command timeout
 */
static int nand_read_page_hwecc(struct mtd_info *mtd,
				struct nand_chip *chip, uint8_t *buf,
				int oob_required, int page)
{
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(chip);
	int ret;

	ret = nand_rw_page(mtd, chip, buf, page, 1, 0);
	if (ret)
		return ret;

	/* Reset FIFO */
	clrsetbits_le32(&info->reg->flash_nf_ecc_reset, GENMASK(31, 0),
			ECC_RESET_ALL);

	return 0;
}

/**
 * Hardware ecc based page write function
 *
 * @param mtd	mtd info structure
 * @param chip	nand chip info structure
 * @param buf	data buffer
 * Return:	0 when successfully completed
 *		-ETIMEDOUT when command timeout
 */
static int nand_write_page_hwecc(struct mtd_info *mtd,
				 struct nand_chip *chip, const uint8_t *buf,
				 int oob_required, int page)
{
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(chip);
	int ret;

	ret = nand_rw_page(mtd, chip, (uint8_t *)buf, page, 1, 1);
	if (ret)
		return ret;

	/* Reset FIFO */
	clrsetbits_le32(&info->reg->flash_nf_ecc_reset, GENMASK(31, 0),
			ECC_RESET_ALL);

	return 0;
}

/**
 * Read raw page data without ecc
 *
 * @param mtd	mtd info structure
 * @param chip	nand chip info structure
 * @param buf	buffer to store read data
 * @param page	page number to read
 * Return:	0 when successfully completed
 *		-ETIMEDOUT when command timeout
 */
static int nand_read_page_raw(struct mtd_info *mtd,
			      struct nand_chip *chip, uint8_t *buf,
			      int oob_required, int page)
{
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(chip);
	int ret;

	ret = nand_rw_page(mtd, chip, buf, page, 0, 0);
	if (ret)
		return ret;

	/* Reset FIFO */
	clrsetbits_le32(&info->reg->flash_nf_ecc_reset, GENMASK(31, 0),
			ECC_RESET_ALL);

	return 0;
}

/**
 * Raw page write function
 *
 * @param mtd	mtd info structure
 * @param chip	nand chip info structure
 * @param buf	data buffer
 * Return:	0 when successfully completed
 *		-ETIMEDOUT when command timeout
 */
static int nand_write_page_raw(struct mtd_info *mtd,
			       struct nand_chip *chip, const uint8_t *buf,
			       int oob_required, int page)
{
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(chip);
	int ret;

	ret = nand_rw_page(mtd, chip, buf, page, 0, 1);
	if (ret)
		return ret;

	/* Reset FIFO */
	clrsetbits_le32(&info->reg->flash_nf_ecc_reset, GENMASK(31, 0),
			ECC_RESET_ALL);

	return 0;
}

/**
 * OOB data read/write function
 *
 * @param mtd		mtd info structure
 * @param chip		nand chip info structure
 * @param page		page number to read
 * @param with_ecc	1 to enable ECC, 0 to disable ECC
 * @param is_writing	0 for read, 1 for write
 * Return:		0 when successfully completed
 *			-ETIMEDOUT when command timeout
 */
static int nand_rw_oob(struct mtd_info *mtd, struct nand_chip *chip,
		       int page, int with_ecc, int is_writing)
{
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(chip);
	u32 reg_val;
	int rw_index;

	if (is_writing) {
		reg_val = NFLASH_GO | NFLASH_WT;
		pwrite = (unsigned int *)chip->oob_poi;
	} else {
		reg_val = NFLASH_GO | NFLASH_RD;
		pread = (unsigned int *)chip->oob_poi;
	}

	for (rw_index = 0; rw_index < mtd->oobsize / 4; rw_index++) {
		clrsetbits_le32(&info->reg->flash_nf_access, GENMASK(31, 0),
				NFLASH_REG_WIDTH_32);
		if (is_writing)
			clrsetbits_le32(&info->reg->flash_nf_data,
					GENMASK(31, 0), pwrite[rw_index]);

		clrsetbits_le32(&info->reg->flash_flash_access_start,
				GENMASK(11, 10), reg_val);

		if (!nand_waitfor_cmd_completion(info->reg, NFLASH_GO))
			printf("%s: Command timeout\n", __func__);

		if (!is_writing)
			pread[rw_index] = readl(&info->reg->flash_nf_data);
	}
	return 0;
}

/**
 * OOB data read function
 *
 * @param mtd		mtd info structure
 * @param chip		nand chip info structure
 * @param page		page number to read
 */
static int nand_read_oob(struct mtd_info *mtd, struct nand_chip *chip, int page)
{
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(chip);
	int ret;

	chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);
	if (mtd->writesize <= (REG_DATA_COUNT_512_DATA >> 8))
		clrsetbits_le32(&info->reg->flash_nf_command, GENMASK(7, 0),
				NAND_CMD_READOOB);
	ret = nand_rw_oob(mtd, chip, page, 0, 0);

	/* Reset FIFO */
	clrsetbits_le32(&info->reg->flash_nf_ecc_reset,
			GENMASK(31, 0), ECC_RESET_ALL);

	return ret;
}

/**
 * OOB data write function
 *
 * @param mtd	mtd info structure
 * @param chip	nand chip info structure
 * @param page	page number to write
 * Return:	0 when successfully completed
 *		-ETIMEDOUT when command timeout
 */
static int nand_write_oob(struct mtd_info *mtd, struct nand_chip *chip,
			  int page)
{
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(chip);
	int ret;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, mtd->writesize, page);
	if (mtd->writesize <= (REG_DATA_COUNT_512_DATA >> 8)) {
		clrsetbits_le32(&info->reg->flash_nf_command, GENMASK(31, 0),
				NAND_CMD_READOOB | (NAND_CMD_SEQIN << 8) |
				(NAND_CMD_PAGEPROG << 16));
		clrsetbits_le32(&info->reg->flash_nf_count, GENMASK(1, 0),
				REG_CMD_COUNT_3TOGO);
	}
	ret = nand_rw_oob(mtd, chip, page, 1, 1);

	/* Reset FIFO */
	clrsetbits_le32(&info->reg->flash_nf_ecc_reset,
			GENMASK(31, 0), ECC_RESET_ALL);

	return ret;
}

/**
 * Decode NAND parameters from the device tree
 *
 * @param dev		Driver model device
 * @param config	Device tree NAND configuration
 */
static int fdt_decode_nand(struct udevice *dev, struct nand_drv *info)
{
	int ecc_strength;

	info->reg = dev_read_addr_ptr(dev);
	info->dma_glb = dev_read_addr_index_ptr(dev, 1);
	info->dma_nand = dev_read_addr_index_ptr(dev, 2);
	info->config.enabled = dev_read_enabled(dev);
	ecc_strength = dev_read_u32_default(dev, "nand-ecc-strength", 16);
	info->flash_base =
	    dev_read_u32_default(dev, "nand_flash_base_addr", NAND_BASE_ADDR);

	switch (ecc_strength) {
	case ECC_STRENGTH_8:
		info->config.nand_ecc_strength = ECC_STRENGTH_8;
		break;
	case ECC_STRENGTH_16:
		info->config.nand_ecc_strength = ECC_STRENGTH_16;
		break;
	case ECC_STRENGTH_24:
		info->config.nand_ecc_strength = ECC_STRENGTH_24;
		break;
	case ECC_STRENGTH_40:
		info->config.nand_ecc_strength = ECC_STRENGTH_40;
		break;
	default:
		info->config.nand_ecc_strength = ECC_STRENGTH_16;
	}

	return 0;
}

/**
 * config flash type
 *
 * @param chip	nand chip info structure
 */
static void nand_config_flash_type(struct nand_chip *nand)
{
	struct nand_drv *info =
	    (struct nand_drv *)nand_get_controller_data(nand);
	struct mtd_info *mtd = nand_to_mtd(nand);

	switch (mtd->writesize) {
	case WRITE_SIZE_512:
		clrsetbits_le32(&info->reg->flash_type, GENMASK(31, 0),
				FLASH_PIN | FLASH_TYPE_512);
		break;
	case WRITE_SIZE_2048:
		clrsetbits_le32(&info->reg->flash_type, GENMASK(31, 0),
				FLASH_PIN | FLASH_TYPE_2K);
		break;
	case WRITE_SIZE_4096:
		clrsetbits_le32(&info->reg->flash_type, GENMASK(31, 0),
				FLASH_PIN | FLASH_TYPE_4K);
		break;
	case WRITE_SIZE_8192:
		clrsetbits_le32(&info->reg->flash_type, GENMASK(31, 0),
				FLASH_PIN | FLASH_TYPE_8K);
		break;
	default:
		pr_err("Unsupported page size(0x%x)!", nand->ecc.size);
	}
}

/**
 * config oob layout
 *
 * @param chip  nand chip info structure
 * Return:	0 when successfully completed
 *		-EINVAL when ECC bytes exceed OOB size
 */
static int nand_config_oob_layout(struct nand_chip *nand)
{
	int i, ecc_start_offset;
	struct mtd_info *mtd = nand_to_mtd(nand);

	/* Calculate byte count for ECC */
	eccoob.eccbytes = mtd->writesize / nand->ecc.size * nand->ecc.bytes;

	if (mtd->oobsize < eccoob.eccbytes) {
		pr_err("Spare area(%d) too small for BCH%d\n", nand->ecc.bytes,
		       nand->ecc.strength / 8);
		pr_err("page_sz: %d\n", nand->ecc.size);
		pr_err("oob_sz: %d\n", nand->ecc.bytes);
		return -EINVAL;
	}

	/* Update OOB layout */
	ecc_start_offset = mtd->oobsize - eccoob.eccbytes;
	memset(eccoob.eccpos, 0, sizeof(eccoob.eccpos));
	for (i = 0; i < eccoob.eccbytes; ++i)
		eccoob.eccpos[i] = i + ecc_start_offset;

	/* Unused spare area
	 * OOB[0] is bad block marker.
	 * Extra two byte is reserved as
	 * erase marker just right before ECC code.
	 */
	eccoob.oobavail = nand->ecc.bytes - eccoob.eccbytes - 2;
	eccoob.oobfree[0].offset = 2;
	eccoob.oobfree[0].length =
	    mtd->oobsize - eccoob.eccbytes - eccoob.oobfree[0].offset - 1;

	return 0;
}

static int ca_nand_probe(struct udevice *dev)
{
	struct ca_nand_info *ca_nand = dev_get_priv(dev);
	struct nand_chip *nand = &ca_nand->nand_chip;
	struct nand_drv *info = &ca_nand->nand_ctrl;
	struct fdt_nand *config = &info->config;
	struct mtd_info *our_mtd;
	int ret;

	if (fdt_decode_nand(dev, info)) {
		printf("Could not decode nand-flash in device tree\n");
		return -1;
	}
	if (!config->enabled)
		return -1;

	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.layout = &eccoob;

	nand->cmdfunc = ca_nand_command;
	nand->read_byte = read_byte;
	nand->read_buf = read_buf;
	nand->ecc.read_page = nand_read_page_hwecc;
	nand->ecc.write_page = nand_write_page_hwecc;
	nand->ecc.read_page_raw = nand_read_page_raw;
	nand->ecc.write_page_raw = nand_write_page_raw;
	nand->ecc.read_oob = nand_read_oob;
	nand->ecc.write_oob = nand_write_oob;
	nand->ecc.strength = config->nand_ecc_strength;
	nand->select_chip = nand_select_chip;
	nand->dev_ready = nand_dev_ready;
	nand_set_controller_data(nand, &ca_nand->nand_ctrl);

	/* Disable subpage writes as we do not provide ecc->hwctl */
	nand->options |= NAND_NO_SUBPAGE_WRITE | NAND_SKIP_BBTSCAN;

	/* Configure flash type as P-NAND */
	clrsetbits_le32(&info->reg->flash_type, FLASH_PIN,
			FLASH_TYPE_4K | FLASH_SIZE_436OOB);
	config->width = FLASH_WIDTH;

	our_mtd = nand_to_mtd(nand);
	ret = nand_scan_ident(our_mtd, CONFIG_SYS_NAND_MAX_CHIPS, NULL);
	if (ret)
		return ret;

	nand->ecc.size = BCH_DATA_UNIT;
	nand->ecc.bytes = BCH_GF_PARAM_M * (nand->ecc.strength / 8);

	/* Reconfig flash type according to ONFI */
	nand_config_flash_type(nand);

	ret = set_bus_width_page_size(our_mtd);
	if (ret)
		return ret;

	/* Set the bad block position */
	nand->badblockpos =
	    our_mtd->writesize >
	    512 ? NAND_LARGE_BADBLOCK_POS : NAND_SMALL_BADBLOCK_POS;

	/* Arrange OOB layout */
	ret = nand_config_oob_layout(nand);
	if (ret)
		return ret;

	/* Init DMA descriptor ring */
	ret = init_nand_dma(nand);
	if (ret)
		return ret;

	ret = nand_scan_tail(our_mtd);
	if (ret)
		return ret;

	ret = nand_register(0, our_mtd);
	if (ret) {
		dev_err(dev, "Failed to register MTD: %d\n", ret);
		return ret;
	}

	ret = set_bus_width_page_size(our_mtd);
	if (ret)
		return ret;

	printf("P-NAND    : %s\n", our_mtd->name);
	printf("Chip  Size: %lldMB\n", nand->chipsize / (1024 * 1024));
	printf("Block Size: %dKB\n", our_mtd->erasesize / 1024);
	printf("Page  Size: %dB\n", our_mtd->writesize);
	printf("OOB   Size: %dB\n", our_mtd->oobsize);

	return 0;
}

U_BOOT_DRIVER(cortina_nand) = {
	.name = "CA-PNAND",
	.id = UCLASS_MTD,
	.of_match = cortina_nand_dt_ids,
	.probe = ca_nand_probe,
	.priv_auto = sizeof(struct ca_nand_info),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(cortina_nand), &dev);
	if (ret && ret != -ENODEV)
		pr_err("Failed to initialize %s. (error %d)\n", dev->name, ret);
}
