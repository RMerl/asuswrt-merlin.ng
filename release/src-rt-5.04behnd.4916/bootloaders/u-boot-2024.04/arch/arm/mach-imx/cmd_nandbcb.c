/*
 * i.MX nand boot control block(bcb).
 *
 * Based on the common/imx-bbu-nand-fcb.c from barebox and imx kobs-ng
 *
 * Copyright (C) 2017 Jagan Teki <jagan@amarulasolutions.com>
 * Copyright (C) 2016 Sergey Kubushyn <ksi@koi8.net>
 *
 * Reconstucted by Han Xu <han.xu@nxp.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <log.h>
#include <malloc.h>
#include <nand.h>
#include <dm/devres.h>
#include <linux/bug.h>

#include <asm/io.h>
#include <jffs2/jffs2.h>
#include <linux/bch.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>

#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/imx-nandbcb.h>
#include <asm/mach-imx/imximage.cfg>
#include <mxs_nand.h>
#include <linux/mtd/mtd.h>
#include <nand.h>
#include <fuse.h>

#include "../../../cmd/legacy-mtd-utils.h"

/* FCB related flags */
/* FCB layout with leading 12B reserved */
#define FCB_LAYOUT_RESV_12B		BIT(0)
/* FCB layout with leading 32B meta data */
#define FCB_LAYOUT_META_32B		BIT(1)
/* FCB encrypted by Hamming code */
#define FCB_ENCODE_HAMMING		BIT(2)
/* FCB encrypted by 40bit BCH */
#define FCB_ENCODE_BCH_40b		BIT(3)
/* FCB encrypted by 62bit BCH */
#define FCB_ENCODE_BCH_62b		BIT(4)
/* FCB encrypted by BCH */
#define FCB_ENCODE_BCH			(FCB_ENCODE_BCH_40b | FCB_ENCODE_BCH_62b)
/* FCB data was randomized */
#define FCB_RANDON_ENABLED		BIT(5)

/* Firmware related flags */
/* No 1K padding */
#define FIRMWARE_NEED_PADDING		BIT(8)
/* Extra firmware*/
#define FIRMWARE_EXTRA_ONE		BIT(9)
/* Secondary firmware on fixed address */
#define FIRMWARE_SECONDARY_FIXED_ADDR	BIT(10)

/* Boot search related flags */
#define BT_SEARCH_CNT_FROM_FUSE		BIT(16)

struct platform_config {
	int misc_flags;
};

static struct platform_config plat_config;

/* imx6q/dl/solo */
static struct platform_config imx6qdl_plat_config = {
	.misc_flags = FCB_LAYOUT_RESV_12B |
		     FCB_ENCODE_HAMMING |
		     FIRMWARE_NEED_PADDING,
};

static struct platform_config imx6sx_plat_config = {
	.misc_flags = FCB_LAYOUT_META_32B |
		     FCB_ENCODE_BCH_62b |
		     FIRMWARE_NEED_PADDING |
		     FCB_RANDON_ENABLED,
};

static struct platform_config imx7d_plat_config = {
	.misc_flags = FCB_LAYOUT_META_32B |
		     FCB_ENCODE_BCH_62b |
		     FIRMWARE_NEED_PADDING |
		     FCB_RANDON_ENABLED,
};

/* imx6ul/ull/ulz */
static struct platform_config imx6ul_plat_config = {
	.misc_flags = FCB_LAYOUT_META_32B |
		     FCB_ENCODE_BCH_40b |
		     FIRMWARE_NEED_PADDING,
};

static struct platform_config imx8mq_plat_config = {
	.misc_flags = FCB_LAYOUT_META_32B |
		     FCB_ENCODE_BCH_62b |
		     FIRMWARE_NEED_PADDING |
		     FCB_RANDON_ENABLED |
		     FIRMWARE_EXTRA_ONE,
};

/* all other imx8mm */
static struct platform_config imx8mm_plat_config = {
	.misc_flags = FCB_LAYOUT_META_32B |
		     FCB_ENCODE_BCH_62b |
		     FIRMWARE_NEED_PADDING |
		     FCB_RANDON_ENABLED,
};

/* imx8mn */
static struct platform_config imx8mn_plat_config = {
	.misc_flags = FCB_LAYOUT_META_32B |
		     FCB_ENCODE_BCH_62b |
		     FCB_RANDON_ENABLED |
		     FIRMWARE_SECONDARY_FIXED_ADDR |
		     BT_SEARCH_CNT_FROM_FUSE,
};

/* imx8qx/qm */
static struct platform_config imx8q_plat_config = {
	.misc_flags = FCB_LAYOUT_META_32B |
		     FCB_ENCODE_BCH_62b |
		     FCB_RANDON_ENABLED |
		     FIRMWARE_SECONDARY_FIXED_ADDR |
		     BT_SEARCH_CNT_FROM_FUSE,
};

/* boot search related variables and definitions */
static int g_boot_search_count = 4;
static int g_boot_secondary_offset;
static int g_boot_search_stride;
static int g_pages_per_stride;

/* mtd config structure */
struct boot_config {
	int dev;
	struct mtd_info *mtd;
	loff_t maxsize;
	loff_t input_size;
	loff_t offset;
	loff_t boot_stream1_address;
	loff_t boot_stream2_address;
	size_t boot_stream1_size;
	size_t boot_stream2_size;
	size_t max_boot_stream_size;
	int stride_size_in_byte;
	int search_area_size_in_bytes;
	int search_area_size_in_pages;
	int secondary_boot_stream_off_in_MB;
};

/* boot_stream config structure */
struct boot_stream_config {
	char bs_label[32];
	loff_t bs_addr;
	size_t bs_size;
	void *bs_buf;
	loff_t next_bs_addr;
	bool need_padding;
};

/* FW index */
#define FW1_ONLY	1
#define FW2_ONLY	2
#define FW_ALL		FW1_ONLY | FW2_ONLY
#define FW_INX(x)	(1 << (x))

/* NAND convert macros */
#define CONV_TO_PAGES(x)	((u32)(x) / (u32)(mtd->writesize))
#define CONV_TO_BLOCKS(x)	((u32)(x) / (u32)(mtd->erasesize))

#define GETBIT(v, n)		(((v) >> (n)) & 0x1)
#define IMX8MQ_SPL_SZ 0x3e000
#define IMX8MQ_HDMI_FW_SZ 0x19c00

static int nandbcb_get_info(int argc, char * const argv[],
			    struct boot_config *boot_cfg)
{
	int dev;
	struct mtd_info *mtd;

	dev = nand_curr_device;
	if (dev < 0) {
		printf("failed to get nand_curr_device, run nand device\n");
		return CMD_RET_FAILURE;
	}

	mtd = get_nand_dev_by_index(dev);
	if (!mtd) {
		printf("failed to get mtd info\n");
		return CMD_RET_FAILURE;
	}

	boot_cfg->dev = dev;
	boot_cfg->mtd = mtd;

	return CMD_RET_SUCCESS;
}

static int nandbcb_get_size(int argc, char * const argv[], int num,
			    struct boot_config *boot_cfg)
{
	int dev;
	loff_t offset, size, maxsize;
	struct mtd_info *mtd;

	dev = boot_cfg->dev;
	mtd = boot_cfg->mtd;
	size = 0;

	if (mtd_arg_off_size(argc - num, argv + num, &dev, &offset, &size,
			     &maxsize, MTD_DEV_TYPE_NAND, mtd->size))
		return CMD_RET_FAILURE;

	boot_cfg->maxsize = maxsize;
	boot_cfg->offset = offset;

	debug("max: %llx, offset: %llx\n", maxsize, offset);

	if (size && size != maxsize)
		boot_cfg->input_size = size;

	return CMD_RET_SUCCESS;
}

static int nandbcb_set_boot_config(int argc, char * const argv[],
				   struct boot_config *boot_cfg)
{
	struct mtd_info *mtd;
	loff_t maxsize;
	loff_t boot_stream1_address, boot_stream2_address, max_boot_stream_size;

	if (!boot_cfg->mtd) {
		printf("Didn't get the mtd info, quit\n");
		return CMD_RET_FAILURE;
	}
	mtd = boot_cfg->mtd;

	/*
	 * By default
	 * set the search count as 4
	 * set each FCB/DBBT/Firmware offset at the beginning of blocks
	 * customers may change the value as needed
	 */

	/* if need more compact layout, change these values */
	/* g_boot_search_count was set as 4 at the definition*/
	/* g_pages_per_stride was set as block size */

	g_pages_per_stride = mtd->erasesize / mtd->writesize;

	g_boot_search_stride = mtd->writesize * g_pages_per_stride;

	boot_cfg->stride_size_in_byte = g_boot_search_stride * mtd->writesize;
	boot_cfg->search_area_size_in_bytes =
		g_boot_search_count * g_boot_search_stride;
	boot_cfg->search_area_size_in_pages =
		boot_cfg->search_area_size_in_bytes / mtd->writesize;

	/* after FCB/DBBT, split the rest of area for two Firmwares */
	if (!boot_cfg->maxsize) {
		printf("Didn't get the maxsize, quit\n");
		return CMD_RET_FAILURE;
	}
	maxsize = boot_cfg->maxsize;
	/* align to page boundary */
	maxsize = ((u32)(maxsize + mtd->writesize - 1)) / (u32)mtd->writesize
			* mtd->writesize;

	boot_stream1_address = 2 * boot_cfg->search_area_size_in_bytes;
	boot_stream2_address = ((maxsize - boot_stream1_address) / 2 +
			       boot_stream1_address);

	if (g_boot_secondary_offset)
		boot_stream2_address =
			(loff_t)g_boot_secondary_offset * 1024 * 1024;

	max_boot_stream_size = boot_stream2_address - boot_stream1_address;

	/* sanity check */
	if (max_boot_stream_size <= 0) {
		debug("st1_addr: %llx, st2_addr: %llx, max: %llx\n",
		      boot_stream1_address, boot_stream2_address,
		      max_boot_stream_size);
		printf("something wrong with firmware address settings\n");
		return CMD_RET_FAILURE;
	}
	boot_cfg->boot_stream1_address = boot_stream1_address;
	boot_cfg->boot_stream2_address = boot_stream2_address;
	boot_cfg->max_boot_stream_size = max_boot_stream_size;

	/* set the boot_stream size as the input size now */
	if (boot_cfg->input_size) {
		boot_cfg->boot_stream1_size = boot_cfg->input_size;
		boot_cfg->boot_stream2_size = boot_cfg->input_size;
	}

	return CMD_RET_SUCCESS;
}

static int nandbcb_check_space(struct boot_config *boot_cfg)
{
	size_t maxsize = boot_cfg->maxsize;
	size_t max_boot_stream_size = boot_cfg->max_boot_stream_size;
	loff_t boot_stream2_address = boot_cfg->boot_stream2_address;

	if (boot_cfg->boot_stream1_size &&
	    boot_cfg->boot_stream1_size > max_boot_stream_size) {
		printf("boot stream1 doesn't fit, check partition size or settings\n");
		return CMD_RET_FAILURE;
	}

	if (boot_cfg->boot_stream2_size &&
	    boot_cfg->boot_stream2_size > maxsize - boot_stream2_address) {
		printf("boot stream2 doesn't fit, check partition size or settings\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

#if defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL)
static uint8_t reverse_bit(uint8_t b)
{
	b = (b & 0xf0) >> 4 | (b & 0x0f) << 4;
	b = (b & 0xcc) >> 2 | (b & 0x33) << 2;
	b = (b & 0xaa) >> 1 | (b & 0x55) << 1;

	return b;
}

static void encode_bch_ecc(void *buf, struct fcb_block *fcb, int eccbits)
{
	int i, j, m = 13;
	int blocksize = 128;
	int numblocks = 8;
	int ecc_buf_size = (m * eccbits + 7) / 8;
	struct bch_control *bch = init_bch(m, eccbits, 0);
	u8 *ecc_buf = kzalloc(ecc_buf_size, GFP_KERNEL);
	u8 *tmp_buf = kzalloc(blocksize * numblocks, GFP_KERNEL);
	u8 *psrc, *pdst;

	/*
	 * The blocks here are bit aligned. If eccbits is a multiple of 8,
	 * we just can copy bytes. Otherwiese we must move the blocks to
	 * the next free bit position.
	 */
	WARN_ON(eccbits % 8);

	memcpy(tmp_buf, fcb, sizeof(*fcb));

	for (i = 0; i < numblocks; i++) {
		memset(ecc_buf, 0, ecc_buf_size);
		psrc = tmp_buf + i * blocksize;
		pdst = buf + i * (blocksize + ecc_buf_size);

		/* copy data byte aligned to destination buf */
		memcpy(pdst, psrc, blocksize);

		/*
		 * imx-kobs use a modified encode_bch which reverse the
		 * bit order of the data before calculating bch.
		 * Do this in the buffer and use the bch lib here.
		 */
		for (j = 0; j < blocksize; j++)
			psrc[j] = reverse_bit(psrc[j]);

		encode_bch(bch, psrc, blocksize, ecc_buf);

		/* reverse ecc bit */
		for (j = 0; j < ecc_buf_size; j++)
			ecc_buf[j] = reverse_bit(ecc_buf[j]);

		/* Here eccbuf is byte aligned and we can just copy it */
		memcpy(pdst + blocksize, ecc_buf, ecc_buf_size);
	}

	kfree(ecc_buf);
	kfree(tmp_buf);
	free_bch(bch);
}
#else

static u8 calculate_parity_13_8(u8 d)
{
	u8 p = 0;

	p |= (GETBIT(d, 6) ^ GETBIT(d, 5) ^ GETBIT(d, 3) ^ GETBIT(d, 2)) << 0;
	p |= (GETBIT(d, 7) ^ GETBIT(d, 5) ^ GETBIT(d, 4) ^ GETBIT(d, 2) ^
	      GETBIT(d, 1)) << 1;
	p |= (GETBIT(d, 7) ^ GETBIT(d, 6) ^ GETBIT(d, 5) ^ GETBIT(d, 1) ^
	      GETBIT(d, 0)) << 2;
	p |= (GETBIT(d, 7) ^ GETBIT(d, 4) ^ GETBIT(d, 3) ^ GETBIT(d, 0)) << 3;
	p |= (GETBIT(d, 6) ^ GETBIT(d, 4) ^ GETBIT(d, 3) ^ GETBIT(d, 2) ^
	      GETBIT(d, 1) ^ GETBIT(d, 0)) << 4;

	return p;
}

static void encode_hamming_13_8(void *_src, void *_ecc, size_t size)
{
	int i;
	u8 *src = _src;
	u8 *ecc = _ecc;

	for (i = 0; i < size; i++)
		ecc[i] = calculate_parity_13_8(src[i]);
}
#endif

static u32 calc_chksum(void *buf, size_t size)
{
	u32 chksum = 0;
	u8 *bp = buf;
	size_t i;

	for (i = 0; i < size; i++)
		chksum += bp[i];

	return ~chksum;
}

static void fill_fcb(struct fcb_block *fcb, struct boot_config *boot_cfg)
{
	struct mtd_info *mtd = boot_cfg->mtd;
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct mxs_nand_info *nand_info = nand_get_controller_data(chip);
	struct mxs_nand_layout l;

	mxs_nand_get_layout(mtd, &l);

	fcb->fingerprint = FCB_FINGERPRINT;
	fcb->version = FCB_VERSION_1;

	fcb->datasetup = 80;
	fcb->datahold = 60;
	fcb->addr_setup = 25;
	fcb->dsample_time = 6;

	fcb->pagesize = mtd->writesize;
	fcb->oob_pagesize = mtd->writesize + mtd->oobsize;
	fcb->sectors = mtd->erasesize / mtd->writesize;

	fcb->meta_size = l.meta_size;
	fcb->nr_blocks = l.nblocks;
	fcb->ecc_nr = l.data0_size;
	fcb->ecc_level = l.ecc0;
	fcb->ecc_size = l.datan_size;
	fcb->ecc_type = l.eccn;
	fcb->bchtype = l.gf_len;

	/* DBBT search area starts from the next block after all FCB */
	fcb->dbbt_start = boot_cfg->search_area_size_in_pages;

	fcb->bb_byte = nand_info->bch_geometry.block_mark_byte_offset;
	fcb->bb_start_bit = nand_info->bch_geometry.block_mark_bit_offset;

	fcb->phy_offset = mtd->writesize;

	fcb->disbbm = 0;

	fcb->fw1_start = CONV_TO_PAGES(boot_cfg->boot_stream1_address);
	fcb->fw2_start = CONV_TO_PAGES(boot_cfg->boot_stream2_address);
	fcb->fw1_pages = CONV_TO_PAGES(boot_cfg->boot_stream1_size);
	fcb->fw2_pages = CONV_TO_PAGES(boot_cfg->boot_stream2_size);

	fcb->checksum = calc_chksum((void *)fcb + 4, sizeof(*fcb) - 4);
}

static int fill_dbbt_data(struct mtd_info *mtd, void *buf, int num_blocks)
{
	int n, n_bad_blocks = 0;
	u32 *bb = buf + 0x8;
	u32 *n_bad_blocksp = buf + 0x4;

	for (n = 0; n < num_blocks; n++) {
		loff_t offset = (loff_t)n * mtd->erasesize;
			if (mtd_block_isbad(mtd, offset)) {
				n_bad_blocks++;
				*bb = n;
				bb++;
		}
	}

	*n_bad_blocksp = n_bad_blocks;

	return n_bad_blocks;
}

/*
 * return 1	- bad block
 * return 0	- read successfully
 * return < 0	- read failed
 */
static int read_fcb(struct boot_config *boot_cfg, struct fcb_block *fcb,
		    loff_t off)
{
	struct mtd_info *mtd;
	void *fcb_raw_page;
	size_t size;
	int ret = 0;

	mtd = boot_cfg->mtd;

	fcb_raw_page = kzalloc(mtd->writesize + mtd->oobsize, GFP_KERNEL);
	if (!fcb_raw_page) {
		debug("failed to allocate fcb_raw_page\n");
		ret = -ENOMEM;
		return ret;
	}

	/*
	 * User BCH hardware to decode ECC for FCB
	 */
	if (plat_config.misc_flags & FCB_ENCODE_BCH) {
		size = sizeof(struct fcb_block);

		/* switch nand BCH to FCB compatible settings */
		if (plat_config.misc_flags & FCB_ENCODE_BCH_62b)
			mxs_nand_mode_fcb_62bit(mtd);
		else if (plat_config.misc_flags & FCB_ENCODE_BCH_40b)
			mxs_nand_mode_fcb_40bit(mtd);

		ret = nand_read_skip_bad(mtd, off, &size, NULL, mtd->size, (u_char *)fcb);

		/* switch BCH back */
		mxs_nand_mode_normal(mtd);
		printf("NAND FCB read from 0x%llx offset 0x%zx read: %s\n",
		       off, size, ret ? "ERROR" : "OK");

	} else if (plat_config.misc_flags & FCB_ENCODE_HAMMING) {
		/* raw read*/
		mtd_oob_ops_t ops = {
			.datbuf = (u8 *)fcb_raw_page,
			.oobbuf = ((u8 *)fcb_raw_page) + mtd->writesize,
			.len = mtd->writesize,
			.ooblen = mtd->oobsize,
			.mode = MTD_OPS_RAW
			};

		ret = mtd_read_oob(mtd, off, &ops);
		printf("NAND FCB read from 0x%llx offset 0x%zx read: %s\n",
		       off, ops.len, ret ? "ERROR" : "OK");
	}

	if (ret)
		goto fcb_raw_page_err;

	if ((plat_config.misc_flags & FCB_ENCODE_HAMMING) &&
	    (plat_config.misc_flags & FCB_LAYOUT_RESV_12B))
		memcpy(fcb, fcb_raw_page + 12, sizeof(struct fcb_block));

/* TODO: check if it can pass Hamming check */

fcb_raw_page_err:
	kfree(fcb_raw_page);

	return ret;
}

static int write_fcb(struct boot_config *boot_cfg, struct fcb_block *fcb)
{
	struct mtd_info *mtd;
	void *fcb_raw_page = NULL;
	int i, ret = 0;
	loff_t off;
	size_t size;

	mtd = boot_cfg->mtd;

	/*
	 * We prepare raw page only for i.MX6, for i.MX7 we
	 * leverage BCH hw module instead
	 */
	if ((plat_config.misc_flags & FCB_ENCODE_HAMMING) &&
	    (plat_config.misc_flags & FCB_LAYOUT_RESV_12B)) {
		fcb_raw_page = kzalloc(mtd->writesize + mtd->oobsize,
				       GFP_KERNEL);
		if (!fcb_raw_page) {
			debug("failed to allocate fcb_raw_page\n");
			ret = -ENOMEM;
			return ret;
		}

#if defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL)
		/* 40 bit BCH, for i.MX6UL(L) */
		encode_bch_ecc(fcb_raw_page + 32, fcb, 40);
#else
		memcpy(fcb_raw_page + 12, fcb, sizeof(struct fcb_block));
		encode_hamming_13_8(fcb_raw_page + 12,
				    fcb_raw_page + 12 + 512, 512);
#endif
		/*
		 * Set the first and second byte of OOB data to 0xFF,
		 * not 0x00. These bytes are used as the Manufacturers Bad
		 * Block Marker (MBBM). Since the FCB is mostly written to
		 * the first page in a block, a scan for
		 * factory bad blocks will detect these blocks as bad, e.g.
		 * when function nand_scan_bbt() is executed to build a new
		 * bad block table.
		 */
		memset(fcb_raw_page + mtd->writesize, 0xFF, 2);
	}

	/* start writing FCB from the very beginning */
	off = 0;

	for (i = 0; i < g_boot_search_count; i++) {
		if (mtd_block_isbad(mtd, off)) {
			printf("Block %d is bad, skipped\n", i);
			off += mtd->erasesize;
			continue;
		}

		/*
		 * User BCH hardware module to generate ECC for FCB
		 */
		if (plat_config.misc_flags & FCB_ENCODE_BCH) {
			size = sizeof(struct fcb_block);

			/* switch nand BCH to FCB compatible settings */
			if (plat_config.misc_flags & FCB_ENCODE_BCH_62b)
				mxs_nand_mode_fcb_62bit(mtd);
			else if (plat_config.misc_flags & FCB_ENCODE_BCH_40b)
				mxs_nand_mode_fcb_40bit(mtd);

			ret = nand_write(mtd, off, &size, (u_char *)fcb);

			/* switch BCH back */
			mxs_nand_mode_normal(mtd);
			printf("NAND FCB write to 0x%zx offset 0x%llx written: %s\n",
			       size, off, ret ? "ERROR" : "OK");

		} else if (plat_config.misc_flags & FCB_ENCODE_HAMMING) {
			/* raw write */
			mtd_oob_ops_t ops = {
				.datbuf = (u8 *)fcb_raw_page,
				.oobbuf = ((u8 *)fcb_raw_page) +
					  mtd->writesize,
				.len = mtd->writesize,
				.ooblen = mtd->oobsize,
				.mode = MTD_OPS_RAW
			};

			ret = mtd_write_oob(mtd, off, &ops);
			printf("NAND FCB write to 0x%llx offset 0x%zx written: %s\n", off, ops.len, ret ? "ERROR" : "OK");
		}

		if (ret)
			goto fcb_raw_page_err;

		/* next writing location */
		off += g_boot_search_stride;
	}

fcb_raw_page_err:
	kfree(fcb_raw_page);

	return ret;
}

/*
 * return 1	- bad block
 * return 0	- read successfully
 * return < 0	- read failed
 */
static int read_dbbt(struct boot_config *boot_cfg, struct dbbt_block *dbbt,
		      void *dbbt_data_page, loff_t off)
{
	size_t size;
	size_t actual_size;
	struct mtd_info *mtd;
	loff_t to;
	int ret;

	mtd = boot_cfg->mtd;

	size = sizeof(struct dbbt_block);
	ret = nand_read_skip_bad(mtd, off, &size, &actual_size, mtd->size, (u_char *)dbbt);
	printf("NAND DBBT read from 0x%llx offset 0x%zx read: %s\n",
	       off, size, ret ? "ERROR" : "OK");
	if (ret)
		return ret;

	/* dbbtpages == 0 if no bad blocks */
	if (dbbt->dbbtpages > 0) {
		to = off + 4 * mtd->writesize + actual_size - size;
		size = mtd->writesize;
		ret = nand_read_skip_bad(mtd, to, &size, NULL, mtd->size, dbbt_data_page);
		printf("DBBT data read from 0x%llx offset 0x%zx read: %s\n",
		       to, size, ret ? "ERROR" : "OK");

		if (ret)
			return ret;
	}

	return 0;
}

static int write_dbbt(struct boot_config *boot_cfg, struct dbbt_block *dbbt,
		      void *dbbt_data_page)
{
	int i;
	loff_t off, to;
	size_t size;
	struct mtd_info *mtd;
	int ret;

	mtd = boot_cfg->mtd;

	/* start writing DBBT after all FCBs */
	off = boot_cfg->search_area_size_in_bytes;
	size = mtd->writesize;

	for (i = 0; i < g_boot_search_count; i++) {
		if (mtd_block_isbad(mtd, off)) {
			printf("Block %d is bad, skipped\n",
			       (int)(i + CONV_TO_BLOCKS(off)));
			off += mtd->erasesize;
			continue;
		}

		ret = nand_write(mtd, off, &size, (u_char *)dbbt);
		printf("NAND DBBT write to 0x%llx offset 0x%zx written: %s\n",
		       off, size, ret ? "ERROR" : "OK");
		if (ret)
			return ret;

		/* dbbtpages == 0 if no bad blocks */
		if (dbbt->dbbtpages > 0) {
			to = off + 4 * mtd->writesize;
			ret = nand_write(mtd, to, &size, dbbt_data_page);
			printf("DBBT data write to 0x%llx offset 0x%zx written: %s\n",
			       to, size, ret ? "ERROR" : "OK");

		if (ret)
			return ret;
		}

		/* next writing location */
		off += g_boot_search_stride;
	}

	return 0;
}

/* reuse the check_skip_len from nand_util.c with minor change*/
static int check_skip_length(struct boot_config *boot_cfg, loff_t offset,
			     size_t length, size_t *used)
{
	struct mtd_info *mtd = boot_cfg->mtd;
	size_t maxsize = boot_cfg->maxsize;
	size_t len_excl_bad = 0;
	int ret = 0;

	while (len_excl_bad < length) {
		size_t block_len, block_off;
		loff_t block_start;

		if (offset >= maxsize)
			return -1;

		block_start = offset & ~(loff_t)(mtd->erasesize - 1);
		block_off = offset & (mtd->erasesize - 1);
		block_len = mtd->erasesize - block_off;

		if (!nand_block_isbad(mtd, block_start))
			len_excl_bad += block_len;
		else
			ret = 1;

		offset += block_len;
		*used += block_len;
	}

	/* If the length is not a multiple of block_len, adjust. */
	if (len_excl_bad > length)
		*used -= (len_excl_bad - length);

	return ret;
}

static int nandbcb_get_next_good_blk_addr(struct boot_config *boot_cfg,
					  struct boot_stream_config *bs_cfg)
{
	struct mtd_info *mtd = boot_cfg->mtd;
	loff_t offset = bs_cfg->bs_addr;
	size_t length = bs_cfg->bs_size;
	size_t used = 0;
	int ret;

	ret = check_skip_length(boot_cfg, offset, length, &used);

	if (ret < 0)
		return ret;

	/* get next image address */
	bs_cfg->next_bs_addr = (u32)(offset + used + mtd->erasesize - 1)
				 / (u32)mtd->erasesize * mtd->erasesize;

	return ret;
}

static int nandbcb_write_bs_skip_bad(struct boot_config *boot_cfg,
				     struct boot_stream_config *bs_cfg)
{
	struct mtd_info *mtd;
	void *buf;
	loff_t offset, maxsize;
	size_t size;
	size_t length;
	int ret;
	bool padding_flag = false;

	mtd = boot_cfg->mtd;
	offset = bs_cfg->bs_addr;
	maxsize = boot_cfg->maxsize;
	size = bs_cfg->bs_size;

	/* some boot images may need leading offset */
	if (bs_cfg->need_padding &&
	    (plat_config.misc_flags & FIRMWARE_NEED_PADDING))
		padding_flag = 1;

	if (padding_flag)
		length = ALIGN(size + FLASH_OFFSET_STANDARD, mtd->writesize);
	else
		length = ALIGN(size, mtd->writesize);

	buf = kzalloc(length, GFP_KERNEL);
	if (!buf) {
		printf("failed to allocate buffer for firmware\n");
		ret = -ENOMEM;
		return ret;
	}

	if (padding_flag)
		memcpy(buf + FLASH_OFFSET_STANDARD, bs_cfg->bs_buf, size);
	else
		memcpy(buf, bs_cfg->bs_buf, size);

	ret = nand_write_skip_bad(mtd, offset, &length, NULL, maxsize,
				  (u_char *)buf, WITH_WR_VERIFY);
	printf("Write %s @0x%llx offset, 0x%zx bytes written: %s\n",
	       bs_cfg->bs_label, offset, length, ret ? "ERROR" : "OK");

	if (ret)
		/* write image failed, quit */
		goto err;

	/* get next good blk address if needed */
	if (bs_cfg->need_padding) {
		ret = nandbcb_get_next_good_blk_addr(boot_cfg, bs_cfg);
		if (ret < 0) {
			printf("Next image cannot fit in NAND partition\n");
			goto err;
		}
	}

	/* now we know how the exact image size written to NAND */
	bs_cfg->bs_size = length;
	return 0;
err:
	kfree(buf);
	return ret;
}

static int nandbcb_write_fw(struct boot_config *boot_cfg, u_char *buf,
			    int index)
{
	int i;
	loff_t offset;
	size_t size;
	loff_t next_bs_addr;
	struct boot_stream_config bs_cfg;
	int ret;

	for (i = 0; i < 2; ++i) {
		if (!(FW_INX(i) & index))
			continue;

		if (i == 0) {
			offset = boot_cfg->boot_stream1_address;
			size = boot_cfg->boot_stream1_size;
		} else {
			offset = boot_cfg->boot_stream2_address;
			size = boot_cfg->boot_stream2_size;
		}

		/* write Firmware*/
		if (!(plat_config.misc_flags & FIRMWARE_EXTRA_ONE)) {
			memset(&bs_cfg, 0, sizeof(struct boot_stream_config));
			sprintf(bs_cfg.bs_label, "firmware%d", i);
			bs_cfg.bs_addr = offset;
			bs_cfg.bs_size = size;
			bs_cfg.bs_buf = buf;
			bs_cfg.need_padding = 1;

			ret = nandbcb_write_bs_skip_bad(boot_cfg, &bs_cfg);
			if (ret)
				return ret;

			/* update the boot stream size */
			if (i == 0)
				boot_cfg->boot_stream1_size = bs_cfg.bs_size;
			else
				boot_cfg->boot_stream2_size = bs_cfg.bs_size;

		} else {
		/* some platforms need extra firmware */
			memset(&bs_cfg, 0, sizeof(struct boot_stream_config));
			sprintf(bs_cfg.bs_label, "fw%d_part%d", i, 1);
			bs_cfg.bs_addr = offset;
			bs_cfg.bs_size = IMX8MQ_HDMI_FW_SZ;
			bs_cfg.bs_buf = buf;
			bs_cfg.need_padding = 1;

			ret = nandbcb_write_bs_skip_bad(boot_cfg, &bs_cfg);
			if (ret)
				return ret;

			/* update the boot stream size */
			if (i == 0)
				boot_cfg->boot_stream1_size = bs_cfg.bs_size;
			else
				boot_cfg->boot_stream2_size = bs_cfg.bs_size;

			/* get next image address */
			next_bs_addr = bs_cfg.next_bs_addr;

			memset(&bs_cfg, 0, sizeof(struct boot_stream_config));
			sprintf(bs_cfg.bs_label, "fw%d_part%d", i, 2);
			bs_cfg.bs_addr = next_bs_addr;
			bs_cfg.bs_size = IMX8MQ_SPL_SZ;
			bs_cfg.bs_buf = (u_char *)(buf + IMX8MQ_HDMI_FW_SZ);
			bs_cfg.need_padding = 0;

			ret = nandbcb_write_bs_skip_bad(boot_cfg, &bs_cfg);
			if (ret)
				return ret;
		}
	}

	return 0;
}

static int nandbcb_init(struct boot_config *boot_cfg, u_char *buf)
{
	struct mtd_info *mtd;
	nand_erase_options_t opts;
	struct fcb_block *fcb;
	struct dbbt_block *dbbt;
	void *dbbt_page, *dbbt_data_page;
	int ret;
	loff_t maxsize, off;

	mtd = boot_cfg->mtd;
	maxsize = boot_cfg->maxsize;
	off = boot_cfg->offset;

	/* erase */
	memset(&opts, 0, sizeof(opts));
	opts.offset = off;
	opts.length = maxsize - 1;
	ret = nand_erase_opts(mtd, &opts);
	if (ret) {
		printf("%s: erase failed (ret = %d)\n", __func__, ret);
		return ret;
	}

	/*
	 * Reference documentation from i.MX6DQRM section 8.5.2.2
	 *
	 * Nand Boot Control Block(BCB) contains two data structures,
	 * - Firmware Configuration Block(FCB)
	 * - Discovered Bad Block Table(DBBT)
	 *
	 * FCB contains,
	 * - nand timings
	 * - DBBT search page address,
	 * - start page address of primary firmware
	 * - start page address of secondary firmware
	 *
	 * setup fcb:
	 * - number of blocks = mtd partition size / mtd erasesize
	 * - two firmware blocks, primary and secondary
	 * - first 4 block for FCB/DBBT
	 * - rest split in half for primary and secondary firmware
	 * - same firmware write twice
	 */

	/* write Firmware*/
	ret = nandbcb_write_fw(boot_cfg, buf, FW_ALL);
	if (ret)
		goto err;

	/* fill fcb */
	fcb = kzalloc(sizeof(*fcb), GFP_KERNEL);
	if (!fcb) {
		debug("failed to allocate fcb\n");
		ret = -ENOMEM;
		return ret;
	}
	fill_fcb(fcb, boot_cfg);

	ret = write_fcb(boot_cfg, fcb);

	/* fill dbbt */
	dbbt_page = kzalloc(mtd->writesize, GFP_KERNEL);
	if (!dbbt_page) {
		debug("failed to allocate dbbt_page\n");
		ret = -ENOMEM;
		goto fcb_err;
	}

	dbbt_data_page = kzalloc(mtd->writesize, GFP_KERNEL);
	if (!dbbt_data_page) {
		debug("failed to allocate dbbt_data_page\n");
		ret = -ENOMEM;
		goto dbbt_page_err;
	}

	dbbt = dbbt_page;
	dbbt->checksum = 0;
	dbbt->fingerprint = DBBT_FINGERPRINT;
	dbbt->version = DBBT_VERSION_1;
	ret = fill_dbbt_data(mtd, dbbt_data_page, CONV_TO_BLOCKS(maxsize));
	if (ret < 0)
		goto dbbt_data_page_err;
	else if (ret > 0)
		dbbt->dbbtpages = 1;

	/* write dbbt */
	ret = write_dbbt(boot_cfg, dbbt, dbbt_data_page);
	if (ret < 0)
		printf("failed to write FCB/DBBT\n");

dbbt_data_page_err:
	kfree(dbbt_data_page);
dbbt_page_err:
	kfree(dbbt_page);
fcb_err:
	kfree(fcb);
err:
	return ret;
}

static int do_nandbcb_bcbonly(int argc, char *const argv[])
{
	struct fcb_block *fcb;
	struct dbbt_block *dbbt;
	struct mtd_info *mtd;
	nand_erase_options_t opts;
	size_t maxsize;
	loff_t off;
	void *dbbt_page, *dbbt_data_page;
	int ret;
	struct boot_config cfg;

	if (argc < 4)
		return CMD_RET_USAGE;

	memset(&cfg, 0, sizeof(struct boot_config));
	if (nandbcb_get_info(argc, argv, &cfg))
		return CMD_RET_FAILURE;

	/* only get the partition info */
	if (nandbcb_get_size(2, argv, 1, &cfg))
		return CMD_RET_FAILURE;

	if (nandbcb_set_boot_config(argc, argv, &cfg))
		return CMD_RET_FAILURE;

	mtd = cfg.mtd;

	cfg.boot_stream1_address = hextoul(argv[2], NULL);
	cfg.boot_stream1_size = hextoul(argv[3], NULL);
	cfg.boot_stream1_size = ALIGN(cfg.boot_stream1_size, mtd->writesize);

	if (argc > 5) {
		cfg.boot_stream2_address = hextoul(argv[4], NULL);
		cfg.boot_stream2_size = hextoul(argv[5], NULL);
		cfg.boot_stream2_size = ALIGN(cfg.boot_stream2_size,
					      mtd->writesize);
	}

	/* sanity check */
	nandbcb_check_space(&cfg);

	maxsize = cfg.maxsize;
	off = cfg.offset;

	/* erase the previous FCB/DBBT */
	memset(&opts, 0, sizeof(opts));
	opts.offset = off;
	opts.length = g_boot_search_stride * 2;
	ret = nand_erase_opts(mtd, &opts);
	if (ret) {
		printf("%s: erase failed (ret = %d)\n", __func__, ret);
		return CMD_RET_FAILURE;
	}

	/* fill fcb */
	fcb = kzalloc(sizeof(*fcb), GFP_KERNEL);
	if (!fcb) {
		printf("failed to allocate fcb\n");
		ret = -ENOMEM;
		return CMD_RET_FAILURE;
	}

	fill_fcb(fcb, &cfg);

	/* write fcb */
	ret = write_fcb(&cfg, fcb);

	/* fill dbbt */
	dbbt_page = kzalloc(mtd->writesize, GFP_KERNEL);
	if (!dbbt_page) {
		printf("failed to allocate dbbt_page\n");
		ret = -ENOMEM;
		goto fcb_err;
	}

	dbbt_data_page = kzalloc(mtd->writesize, GFP_KERNEL);
	if (!dbbt_data_page) {
		printf("failed to allocate dbbt_data_page\n");
		ret = -ENOMEM;
		goto dbbt_page_err;
	}

	dbbt = dbbt_page;
	dbbt->checksum = 0;
	dbbt->fingerprint = DBBT_FINGERPRINT;
	dbbt->version = DBBT_VERSION_1;
	ret = fill_dbbt_data(mtd, dbbt_data_page, CONV_TO_BLOCKS(maxsize));
	if (ret < 0)
		goto dbbt_data_page_err;
	else if (ret > 0)
		dbbt->dbbtpages = 1;

	/* write dbbt */
	ret = write_dbbt(&cfg, dbbt, dbbt_data_page);

dbbt_data_page_err:
	kfree(dbbt_data_page);
dbbt_page_err:
	kfree(dbbt_page);
fcb_err:
	kfree(fcb);

	if (ret < 0) {
		printf("failed to write FCB/DBBT\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

/* dump data which is read from NAND chip */
void dump_structure(struct boot_config *boot_cfg, struct fcb_block *fcb,
		    struct dbbt_block *dbbt, void *dbbt_data_page)
{
	int i;
	struct mtd_info *mtd = boot_cfg->mtd;

	#define P1(x) printf("  %s = 0x%08x\n", #x, fcb->x)
		printf("FCB\n");
		P1(checksum);
		P1(fingerprint);
		P1(version);
	#undef P1
	#define P1(x)	printf("  %s = %d\n", #x, fcb->x)
		P1(datasetup);
		P1(datahold);
		P1(addr_setup);
		P1(dsample_time);
		P1(pagesize);
		P1(oob_pagesize);
		P1(sectors);
		P1(nr_nand);
		P1(nr_die);
		P1(celltype);
		P1(ecc_type);
		P1(ecc_nr);
		P1(ecc_size);
		P1(ecc_level);
		P1(meta_size);
		P1(nr_blocks);
		P1(ecc_type_sdk);
		P1(ecc_nr_sdk);
		P1(ecc_size_sdk);
		P1(ecc_level_sdk);
		P1(nr_blocks_sdk);
		P1(meta_size_sdk);
		P1(erase_th);
		P1(bootpatch);
		P1(patch_size);
		P1(fw1_start);
		P1(fw2_start);
		P1(fw1_pages);
		P1(fw2_pages);
		P1(dbbt_start);
		P1(bb_byte);
		P1(bb_start_bit);
		P1(phy_offset);
		P1(bchtype);
		P1(readlatency);
		P1(predelay);
		P1(cedelay);
		P1(postdelay);
		P1(cmdaddpause);
		P1(datapause);
		P1(tmspeed);
		P1(busytimeout);
		P1(disbbm);
		P1(spare_offset);
#if !defined(CONFIG_MX6) || defined(CONFIG_MX6SX) || \
	defined(CONFIG_MX6UL) || defined(CONFIG_MX6ULL)
		P1(onfi_sync_enable);
		P1(onfi_sync_speed);
		P1(onfi_sync_nand_data);
		P1(disbbm_search);
		P1(disbbm_search_limit);
		P1(read_retry_enable);
#endif
	#undef P1
	#define P1(x)	printf("  %s = 0x%08x\n", #x, dbbt->x)
		printf("DBBT :\n");
		P1(checksum);
		P1(fingerprint);
		P1(version);
	#undef P1
	#define P1(x)	printf("  %s = %d\n", #x, dbbt->x)
		P1(dbbtpages);
	#undef P1

	for (i = 0; i < dbbt->dbbtpages; ++i)
		printf("%d ", *((u32 *)(dbbt_data_page + i)));

	if (!(plat_config.misc_flags & FIRMWARE_EXTRA_ONE)) {
		printf("Firmware: image #0 @ 0x%x size 0x%x\n",
		       fcb->fw1_start, fcb->fw1_pages * mtd->writesize);
		printf("Firmware: image #1 @ 0x%x size 0x%x\n",
		       fcb->fw2_start, fcb->fw2_pages * mtd->writesize);
	} else {
		printf("Firmware: image #0 @ 0x%x size 0x%x\n",
		       fcb->fw1_start, fcb->fw1_pages * mtd->writesize);
		printf("Firmware: image #1 @ 0x%x size 0x%x\n",
		       fcb->fw2_start, fcb->fw2_pages * mtd->writesize);
		/* TODO: Add extra image information */
	}
}

static bool check_fingerprint(void *data, int fingerprint)
{
	int off = 4;

	return (*(int *)(data + off) == fingerprint);
}

static int fuse_secondary_boot(u32 bank, u32 word, u32 mask, u32 off)
{
	int err;
	u32 val;
	int ret;

	err = fuse_read(bank, word, &val);
	if (err)
		return 0;

	val = (val & mask) >> off;

	if (val > 10)
		return 0;

	switch (val) {
	case 0:
		ret = 4;
		break;
	case 1:
		ret = 1;
		break;
	default:
		ret = 2 << val;
		break;
	}

	return ret;
};

static int fuse_to_search_count(u32 bank, u32 word, u32 mask, u32 off)
{
	int err;
	u32 val;
	int ret;

	/* by default, the boot search count from fuse should be 2 */
	err = fuse_read(bank, word, &val);
	if (err)
		return 2;

	val = (val & mask) >> off;

	switch (val) {
		case 0:
			ret = 2;
			break;
		case 1:
		case 2:
		case 3:
			ret = 1 << val;
			break;
		default:
			ret = 2;
	}

	return ret;
}

static int nandbcb_dump(struct boot_config *boot_cfg)
{
	int i;
	loff_t off;
	struct mtd_info *mtd = boot_cfg->mtd;
	struct fcb_block fcb, fcb_copy;
	struct dbbt_block dbbt, dbbt_copy;
	void *dbbt_data_page, *dbbt_data_page_copy;
	bool fcb_not_found, dbbt_not_found;
	int ret = 0;

	dbbt_data_page = kzalloc(mtd->writesize, GFP_KERNEL);
	if (!dbbt_data_page) {
		printf("failed to allocate dbbt_data_page\n");
		ret = -ENOMEM;
		return ret;
	}

	dbbt_data_page_copy = kzalloc(mtd->writesize, GFP_KERNEL);
	if (!dbbt_data_page_copy) {
		printf("failed to allocate dbbt_data_page\n");
		ret = -ENOMEM;
		goto dbbt_page_err;
	}

	/* read fcb */
	fcb_not_found = 1;
	off = 0;
	for (i = 0; i < g_boot_search_count; ++i) {
		if (fcb_not_found) {
			ret = read_fcb(boot_cfg, &fcb, off);

			if (ret < 0)
				goto dbbt_page_copy_err;
			else if (ret == 1)
				continue;
			else if (ret == 0)
				if (check_fingerprint(&fcb, FCB_FINGERPRINT))
					fcb_not_found = 0;
		} else {
			ret = read_fcb(boot_cfg, &fcb_copy, off);

			if (ret < 0)
				goto dbbt_page_copy_err;
			if (memcmp(&fcb, &fcb_copy,
				   sizeof(struct fcb_block))) {
				printf("FCB copies are not identical\n");
				ret = -EINVAL;
				goto dbbt_page_copy_err;
			}
		}

		/* next read location */
		off += g_boot_search_stride;
	}

	/* read dbbt*/
	dbbt_not_found = 1;
	off = boot_cfg->search_area_size_in_bytes;
	for (i = 0; i < g_boot_search_count; ++i) {
		if (dbbt_not_found) {
			ret = read_dbbt(boot_cfg, &dbbt, dbbt_data_page, off);

			if (ret < 0)
				goto dbbt_page_copy_err;
			else if (ret == 1)
				continue;
			else if (ret == 0)
				if (check_fingerprint(&dbbt, DBBT_FINGERPRINT))
					dbbt_not_found = 0;
		} else {
			ret = read_dbbt(boot_cfg, &dbbt_copy,
					dbbt_data_page_copy, off);

			if (ret < 0)
				goto dbbt_page_copy_err;
			if (memcmp(&dbbt, &dbbt_copy,
				   sizeof(struct dbbt_block))) {
				printf("DBBT copies are not identical\n");
				ret = -EINVAL;
				goto dbbt_page_copy_err;
			}
			if (dbbt.dbbtpages > 0 &&
			    memcmp(dbbt_data_page, dbbt_data_page_copy,
				   mtd->writesize)) {
				printf("DBBT data copies are not identical\n");
				ret = -EINVAL;
				goto dbbt_page_copy_err;
			}
		}

		/* next read location */
		off += g_boot_search_stride;
	}

	dump_structure(boot_cfg, &fcb, &dbbt, dbbt_data_page);

dbbt_page_copy_err:
	kfree(dbbt_data_page_copy);
dbbt_page_err:
	kfree(dbbt_data_page);

	return ret;
}

static int do_nandbcb_dump(int argc, char * const argv[])
{
	struct boot_config cfg;
	int ret;

	if (argc != 2)
		return CMD_RET_USAGE;

	memset(&cfg, 0, sizeof(struct boot_config));
	if (nandbcb_get_info(argc, argv, &cfg))
		return CMD_RET_FAILURE;

	if (nandbcb_get_size(argc, argv, 1, &cfg))
		return CMD_RET_FAILURE;

	if (nandbcb_set_boot_config(argc, argv, &cfg))
		return CMD_RET_FAILURE;

	ret = nandbcb_dump(&cfg);
	if (ret)
		return ret;

	return ret;
}

static int do_nandbcb_init(int argc, char * const argv[])
{
	u_char *buf;
	size_t size;
	loff_t addr;
	char *endp;
	int ret;
	struct boot_config cfg;

	if (argc != 4)
		return CMD_RET_USAGE;

	memset(&cfg, 0, sizeof(struct boot_config));
	if (nandbcb_get_info(argc, argv, &cfg))
		return CMD_RET_FAILURE;

	if (nandbcb_get_size(argc, argv, 2, &cfg))
		return CMD_RET_FAILURE;
	size = cfg.boot_stream1_size;

	if (nandbcb_set_boot_config(argc, argv, &cfg))
		return CMD_RET_FAILURE;

	addr = hextoul(argv[1], &endp);
	if (*argv[1] == 0 || *endp != 0)
		return CMD_RET_FAILURE;

	buf = map_physmem(addr, size, MAP_WRBACK);
	if (!buf) {
		puts("failed to map physical memory\n");
		return CMD_RET_FAILURE;
	}

	ret = nandbcb_init(&cfg, buf);

	return ret == 0 ? CMD_RET_SUCCESS : CMD_RET_FAILURE;
}

static int do_nandbcb(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	const char *cmd;
	int ret = 0;

	if (argc < 3)
		goto usage;

	/* check the platform config first */
	if (is_mx6sx()) {
		plat_config = imx6sx_plat_config;
	} else if (is_mx7()) {
		plat_config = imx7d_plat_config;
	} else if (is_mx6ul() || is_mx6ull()) {
		plat_config = imx6ul_plat_config;
	} else if (is_mx6() && !is_mx6sx() && !is_mx6ul() && !is_mx6ull()) {
		plat_config = imx6qdl_plat_config;
	} else if (is_imx8mq()) {
		plat_config = imx8mq_plat_config;
	} else if (is_imx8mm()) {
		plat_config = imx8mm_plat_config;
	} else if (is_imx8mn() || is_imx8mp()) {
		plat_config = imx8mn_plat_config;
	} else if (is_imx8qm() || is_imx8qxp()) {
		plat_config = imx8q_plat_config;
	} else {
		printf("ERROR: Unknown platform\n");
		return CMD_RET_FAILURE;
	}

	if ((plat_config.misc_flags) & BT_SEARCH_CNT_FROM_FUSE) {
		if (is_imx8qxp())
			g_boot_search_count = fuse_to_search_count(0, 720, 0xc0, 6);
		if (is_imx8mn() || is_imx8mp())
			g_boot_search_count = fuse_to_search_count(2, 2, 0x6000, 13);
		printf("search count set to %d from fuse\n",
		       g_boot_search_count);
	}

	if (plat_config.misc_flags & FIRMWARE_SECONDARY_FIXED_ADDR) {
		if (is_imx8mn())
			g_boot_secondary_offset = fuse_secondary_boot(2, 1, 0xff0000, 16);
	}

	cmd = argv[1];
	--argc;
	++argv;

	if (strcmp(cmd, "init") == 0) {
		ret = do_nandbcb_init(argc, argv);
		goto done;
	}

	if (strcmp(cmd, "dump") == 0) {
		ret = do_nandbcb_dump(argc, argv);
		goto done;
	}

	if (strcmp(cmd, "bcbonly") == 0) {
		ret = do_nandbcb_bcbonly(argc, argv);
		goto done;
	}

done:
	if (ret != -1)
		return ret;
usage:
	return CMD_RET_USAGE;
}

U_BOOT_LONGHELP(nandbcb,
	"init addr off|partition len - update 'len' bytes starting at\n"
	"       'off|part' to memory address 'addr', skipping  bad blocks\n"
	"nandbcb bcbonly off|partition fw1-off fw1-size [fw2-off fw2-size]\n"
	"	    - write BCB only (FCB and DBBT)\n"
	"       where `fwx-size` is fw sizes in bytes, `fw1-off`\n"
	"       and `fw2-off` - firmware offsets\n"
	"       FIY, BCB isn't erased automatically, so mtd erase should\n"
	"       be called in advance before writing new BCB:\n"
	"           > mtd erase mx7-bcb\n"
	"nandbcb dump off|partition - dump/verify boot structures\n");

U_BOOT_CMD(nandbcb, 7, 1, do_nandbcb,
	   "i.MX NAND Boot Control Blocks write",
	   nandbcb_help_text
);
