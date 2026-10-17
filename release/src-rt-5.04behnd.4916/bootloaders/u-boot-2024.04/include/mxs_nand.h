/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * NXP GPMI NAND flash driver
 *
 * Copyright (C) 2018 Toradex
 * Authors:
 * Stefan Agner <stefan.agner@toradex.com>
 */

#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <asm/cache.h>
#include <nand.h>
#include <asm/mach-imx/dma.h>
#include <clk.h>

/**
 * @gf_len:                   The length of Galois Field. (e.g., 13 or 14)
 * @ecc_strength:             A number that describes the strength of the ECC
 *                            algorithm.
 * @ecc_chunk0_size:          The size, in bytes, of a first ECC chunk.
 * @ecc_chunkn_size:          The size, in bytes, of a single ECC chunk after
 *                            the first chunk in the page.
 * @ecc_chunk_count:          The number of ECC chunks in the page,
 * @block_mark_byte_offset:   The byte offset in the ECC-based page view at
 *                            which the underlying physical block mark appears.
 * @block_mark_bit_offset:    The bit offset into the ECC-based page view at
 *                            which the underlying physical block mark appears.
 * @ecc_for_meta:             The flag to indicate if there is a dedicate ecc
 *                            for meta.
 */
struct bch_geometry {
	unsigned int  gf_len;
	unsigned int  ecc_strength;
	unsigned int  ecc_chunk0_size;
	unsigned int  ecc_chunkn_size;
	unsigned int  ecc_chunk_count;
	unsigned int  block_mark_byte_offset;
	unsigned int  block_mark_bit_offset;
	unsigned int  ecc_for_meta; /* ECC for meta data */
};

struct mxs_nand_info {
	struct nand_chip chip;
	struct udevice *dev;
	unsigned int	max_ecc_strength_supported;
	int		max_chain_delay;
	bool		use_minimum_ecc;
	int		cur_chip;

	uint32_t	cmd_queue_len;
	uint32_t	data_buf_size;
	struct bch_geometry bch_geometry;

	uint8_t		*cmd_buf;
	uint8_t		*data_buf;
	uint8_t		*oob_buf;

	uint8_t		marking_block_bad;
	uint8_t		raw_oob_mode;

	struct mxs_gpmi_regs *gpmi_regs;
	struct mxs_bch_regs *bch_regs;
	struct clk *gpmi_clk;

	/* Functions with altered behaviour */
	int		(*hooked_read_oob)(struct mtd_info *mtd,
				loff_t from, struct mtd_oob_ops *ops);
	int		(*hooked_write_oob)(struct mtd_info *mtd,
				loff_t to, struct mtd_oob_ops *ops);
	int		(*hooked_block_markbad)(struct mtd_info *mtd,
				loff_t ofs);

	/* DMA descriptors */
	struct mxs_dma_desc	**desc;
	uint32_t		desc_index;

	/* Hardware BCH interface and randomizer */
	u32 en_randomizer;
	u32 writesize;
	u32 oobsize;
	u32 bch_flash0layout0;
	u32 bch_flash0layout1;
};

struct mxs_nand_layout {
	u32 nblocks;
	u32 meta_size;
	u32 data0_size;
	u32 ecc0;
	u32 datan_size;
	u32 eccn;
	u32 gf_len;
};

int mxs_nand_init_ctrl(struct mxs_nand_info *nand_info);
int mxs_nand_init_spl(struct nand_chip *nand);
int mxs_nand_setup_ecc(struct mtd_info *mtd);

void mxs_nand_mode_fcb_62bit(struct mtd_info *mtd);
void mxs_nand_mode_fcb_40bit(struct mtd_info *mtd);
void mxs_nand_mode_normal(struct mtd_info *mtd);
u32 mxs_nand_mark_byte_offset(struct mtd_info *mtd);
u32 mxs_nand_mark_bit_offset(struct mtd_info *mtd);
void mxs_nand_get_layout(struct mtd_info *mtd, struct mxs_nand_layout *l);
