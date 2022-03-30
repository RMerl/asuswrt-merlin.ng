/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */
//#define DEBUG

#include <common.h>
#include <nand.h>
#include "brcmnand.h"
#include "brcmnand_spl.h"

#define SPARE_MAX_SIZE          (27 * 16)
#define CTRLR_CACHE_SIZE        512
#define FC_WORDS		(CTRLR_CACHE_SIZE  >> 2)

#define NAND_CHIPID(chip)       ((chip)->chip_device_id >> 16)

/* Flash manufacturers. */
#define FLASHTYPE_SAMSUNG       0xec
#define FLASHTYPE_ST            0x20
#define FLASHTYPE_MICRON        0x2c
#define FLASHTYPE_HYNIX         0xad
#define FLASHTYPE_TOSHIBA       0x98
#define FLASHTYPE_MXIC          0xc2
#define FLASHTYPE_SPANSION      0x01

/* Samsung flash parts. */
#define SAMSUNG_K9F5608U0A      0x55
#define SAMSUNG_K9F1208U0       0x76
#define SAMSUNG_K9F1G08U0       0xf1

/* ST flash parts. */
#define ST_NAND512W3A2CN6       0x76
#define ST_NAND01GW3B2CN6       0xf1

/* Micron flash parts. */
#define MICRON_MT29F1G08AAC     0xf1
#define MICRON_MT29F2G08ABA     0xda
#define MICRON_MT29F4G08ABA     0xdc
#define MICRON_MT29F8G08ABA     0x38
#define MICRON_MT29F8G16ABA     0xd3

/* Hynix flash parts. */
#define HYNIX_H27U1G8F2B        0xf1
#define HYNIX_H27U518S2C        0x76

/* MXIC flash parts */
#define MXIC_MX30LF1208AA       0xf0
#define MXIC_MX30LF1G08AA       0xf1

/* SPANSION flash parts */
#define SPANSION_S34ML01G1      0xf1
#define SPANSION_S34ML02G1      0xda
#define SPANSION_S34ML04G1      0xdc

/* Flash id to name mapping. */
#define NAND_MAKE_ID(A,B)    \
    (((unsigned short) (A) << 8) | ((unsigned short) B & 0xff))

#define NAND_FLASH_DEVICES                                                    \
  {{NAND_MAKE_ID(FLASHTYPE_SAMSUNG,SAMSUNG_K9F5608U0A),"Samsung K9F5608U0"},  \
   {NAND_MAKE_ID(FLASHTYPE_SAMSUNG,SAMSUNG_K9F1208U0),"Samsung K9F1208U0"},   \
   {NAND_MAKE_ID(FLASHTYPE_SAMSUNG,SAMSUNG_K9F1G08U0),"Samsung K9F1G08U0"},   \
   {NAND_MAKE_ID(FLASHTYPE_ST,ST_NAND512W3A2CN6),"ST NAND512W3A2CN6"},        \
   {NAND_MAKE_ID(FLASHTYPE_ST,ST_NAND01GW3B2CN6),"ST NAND01GW3B2CN6"},        \
   {NAND_MAKE_ID(FLASHTYPE_MICRON,MICRON_MT29F1G08AAC),"Micron MT29F1G08AAC"},\
   {NAND_MAKE_ID(FLASHTYPE_MICRON,MICRON_MT29F2G08ABA),"Micron MT29F2G08ABA"},\
   {NAND_MAKE_ID(FLASHTYPE_MICRON,MICRON_MT29F4G08ABA),"Micron MT29F4G08ABA"},\
   {NAND_MAKE_ID(FLASHTYPE_MICRON,MICRON_MT29F8G08ABA),"Micron MT29F8G08ABA"},\
   {NAND_MAKE_ID(FLASHTYPE_MICRON,MICRON_MT29F8G16ABA),"Micron MT29F8G16ABA"},\
   {NAND_MAKE_ID(FLASHTYPE_HYNIX,HYNIX_H27U1G8F2B),"Hynix H27U1G8F2B"},       \
   {NAND_MAKE_ID(FLASHTYPE_HYNIX,HYNIX_H27U518S2C),"Hynix H27U518S2C"},       \
   {NAND_MAKE_ID(FLASHTYPE_MXIC,MXIC_MX30LF1208AA),"MXIC MX30LF1208AA"},      \
   {NAND_MAKE_ID(FLASHTYPE_MXIC,MXIC_MX30LF1G08AA),"MXIC MX30LF1G08AA"},      \
   {NAND_MAKE_ID(FLASHTYPE_SPANSION,SPANSION_S34ML01G1),"Spansion S34ML01G1"},\
   {NAND_MAKE_ID(FLASHTYPE_SPANSION,SPANSION_S34ML02G1),"Spansion S34ML02G1"},\
   {NAND_MAKE_ID(FLASHTYPE_SPANSION,SPANSION_S34ML04G1),"Spansion S34ML04G1"},\
   {0,""}                                                                     \
  }

#define NAND_FLASH_MANUFACTURERS        \
  {{FLASHTYPE_SAMSUNG, "Samsung"},      \
   {FLASHTYPE_ST, "ST"},                \
   {FLASHTYPE_MICRON, "Micron"},        \
   {FLASHTYPE_HYNIX, "Hynix"},          \
   {FLASHTYPE_TOSHIBA, "Toshiba"},      \
   {FLASHTYPE_MXIC, "MXIC"},            \
   {FLASHTYPE_SPANSION, "Spansion"},    \
   {0,""}                               \
  }

/* Condition to determine the spare layout. */
#define LAYOUT_PARMS(L,S,P)     \
    (((unsigned int)(L)<<28) | ((unsigned int)(S)<<16) | (P))

/* Each bit in the ECCMSK array represents a spare area byte. Bits that are
 * set correspond to spare area bytes that are reserved for the ECC or bad
 * block indicator. Bits that are not set can be used for data such as the
 * JFFS2 clean marker. This macro returns 0 if the spare area byte at offset,
 * OFS, is available and non-0 if it is being used for the ECC or BI.
 */
#define ECC_MASK_BIT(ECCMSK, OFS)   (ECCMSK[OFS / 8] & (1 << (OFS % 8)))

#define SPARE_BI_MARKER         0
#define SPARE_GOOD_MARKER       0xFF

/* Fixed definition for NAND controller on all revision */
#define CMD_NULL			0x00
#define CMD_PAGE_READ			0x01
#define CMD_SPARE_AREA_READ		0x02
#define CMD_STATUS_READ			0x03
#define CMD_PROGRAM_PAGE		0x04
#define CMD_PROGRAM_SPARE_AREA		0x05
#define CMD_COPY_BACK			0x06
#define CMD_DEVICE_ID_READ		0x07
#define CMD_BLOCK_ERASE			0x08
#define CMD_FLASH_RESET			0x09
#define CMD_BLOCKS_LOCK			0x0a
#define CMD_BLOCKS_LOCK_DOWN		0x0b
#define CMD_BLOCKS_UNLOCK		0x0c
#define CMD_READ_BLOCKS_LOCK_STATUS	0x0d
#define CMD_PARAMETER_READ		0x0e
#define CMD_PARAMETER_CHANGE_COL	0x0f
#define CMD_LOW_LEVEL_OP		0x10

#define NBC_AUTO_DEV_ID_CFG		(1 << 30)

#define NIS_CTLR_READY			(1 << 31)
#define NIS_FLASH_READY			(1 << 30)
#define NIS_CACHE_VALID			(1 << 29)
#define NIS_SPARE_VALID			(1 << 28)
#define NIS_FLASH_STS_MASK		0x000000ff

#define NC_DEV_SIZE_SHIFT		24
#define NC_DEV_SIZE_MASK		(0x0f << NC_DEV_SIZE_SHIFT)
#define NC_FUL_ADDR_SHIFT		16
#define NC_FUL_ADDR_MASK		(0x7 << NC_FUL_ADDR_SHIFT)
#define NC_COL_ADDR_SHIFT		12
#define NC_COL_ADDR_MASK		(0x7 << NC_COL_ADDR_SHIFT)
#define NC_BLK_ADDR_SHIFT		8
#define NC_BLK_ADDR_MASK		(0x07 << NC_BLK_ADDR_SHIFT)

#define NAC_ECC_LVL_SHIFT		16
#define NAC_ECC_LVL_MASK		0x001f0000
#define NAC_ECC_LVL_DISABLE		0
#define NAC_ECC_LVL_BCH_1		1
#define NAC_ECC_LVL_BCH_2		2
#define NAC_ECC_LVL_BCH_3		3
#define NAC_ECC_LVL_BCH_4		4
#define NAC_ECC_LVL_BCH_5		5
#define NAC_ECC_LVL_BCH_6		6
#define NAC_ECC_LVL_BCH_7		7
#define NAC_ECC_LVL_BCH_8		8
#define NAC_ECC_LVL_BCH_9		9
#define NAC_ECC_LVL_BCH_10		10
#define NAC_ECC_LVL_BCH_11		11
#define NAC_ECC_LVL_BCH_12		12
#define NAC_ECC_LVL_BCH_13		13
#define NAC_ECC_LVL_BCH_14		14
#define NAC_ECC_LVL_HAMMING		15	/* Hamming if spare are size = 16, BCH15 otherwise */
#define NAC_ECC_LVL_BCH15		15
#define NAC_ECC_LVL_BCH_16		16
#define NAC_ECC_LVL_BCH_17		17
/* BCH18 to 30 use sector size = 1K */
#define NAC_SECTOR_SIZE_1K		(1 << 7)
#define NAC_SPARE_SZ_SHIFT		0
#define NAC_SPARE_SZ_MASK		0x0000007f

#define NT_TREH_MASK			0x000f0000
#define NT_TREH_SHIFT			16
#define NT_TRP_MASK			0x00f00000
#define NT_TRP_SHIFT			20
#define NT_TREAD_MASK			0x0000000f
#define NT_TREAD_SHIFT			0

struct cfg_decode_map {
	uint16_t dev_size_reg;
	uint16_t dev_size_shift;
	uint32_t dev_size_mask;
	uint16_t block_size_reg;
	uint16_t block_size_shift;
	uint32_t block_size_mask;
	uint16_t page_size_reg;
	uint16_t page_size_shift;
	uint32_t page_size_mask;
	uint32_t *block_tbl;
	uint32_t *page_tbl;
};

struct brcmnand_controller {
	void __iomem *nand_base;
	void __iomem *nand_fc;	/* flash cache */
	uint16_t nand_version;
	const uint16_t *reg_offsets;
	const struct cfg_decode_map *cfg_dec_map;
	uint8_t *flash_cache;
};

struct brcmnand_chip {
	struct brcmnand_controller *ctrl;
	uint32_t chip_device_id;
	uint64_t chip_total_size;
	uint32_t chip_block_size;
	uint32_t chip_page_size;
	uint32_t chip_spare_size;
	uint32_t chip_spare_step_size;
	uint32_t chip_ecc_level;
	uint32_t sector_size_1k;
	uint32_t chip_bi_index_1;
	uint32_t chip_bi_index_2;
};

enum brcmnand_reg {
	BRCMNAND_CMD_START = 0,
	BRCMNAND_CMD_EXT_ADDRESS,
	BRCMNAND_CMD_ADDRESS,
	BRCMNAND_INTFC_STATUS,
	BRCMNAND_CS_SELECT,
	BRCMNAND_CS_XOR,
	BRCMNAND_CS_ACC_CONTROL,
	BRCMNAND_CS_CFG_EXT,
	BRCMNAND_CS_CFG,
	BRCMNAND_ID,
	BRCMNAND_OOB_READ_BASE,
	BRCMNAND_TIMING1,
	BRCMNAND_TIMING2,
};

/* BRCMNAND v6.0 - v7.0 */
static const u16 brcmnand_regs_v60[] = {
	[BRCMNAND_CMD_START] = 0x04,
	[BRCMNAND_CMD_EXT_ADDRESS] = 0x08,
	[BRCMNAND_CMD_ADDRESS] = 0x0c,
	[BRCMNAND_INTFC_STATUS] = 0x14,
	[BRCMNAND_CS_SELECT] = 0x18,
	[BRCMNAND_CS_XOR] = 0x1c,
	[BRCMNAND_CS_ACC_CONTROL] = 0x50,
	[BRCMNAND_CS_CFG_EXT] = 0,
	[BRCMNAND_CS_CFG] = 0x54,
	[BRCMNAND_TIMING1] = 0x58,
	[BRCMNAND_TIMING2] = 0x5c,
	[BRCMNAND_ID] = 0x194,
	[BRCMNAND_OOB_READ_BASE] = 0x200,
};

/* BRCMNAND v7.1 */
static const u16 brcmnand_regs_v71[] = {
	[BRCMNAND_CMD_START] = 0x04,
	[BRCMNAND_CMD_EXT_ADDRESS] = 0x08,
	[BRCMNAND_CMD_ADDRESS] = 0x0c,
	[BRCMNAND_INTFC_STATUS] = 0x14,
	[BRCMNAND_CS_SELECT] = 0x18,
	[BRCMNAND_CS_XOR] = 0x1c,
	[BRCMNAND_CS_ACC_CONTROL] = 0x50,
	[BRCMNAND_CS_CFG_EXT] = 0x54,
	[BRCMNAND_CS_CFG] = 0x58,
	[BRCMNAND_TIMING1] = 0x5c,
	[BRCMNAND_TIMING2] = 0x60,
	[BRCMNAND_ID] = 0x194,
	[BRCMNAND_OOB_READ_BASE] = 0x200,
};

uint32_t blk_tbl_v60[] = {
	SZ_8K,
	SZ_16K,
	SZ_128K,
	SZ_256K,
	SZ_512K,
	SZ_1M,
	SZ_2M,
	0,
};

uint32_t pg_tbl_v60[] = {
	SZ_512,
	SZ_2K,
	SZ_4K,
	SZ_8K,
	0,
};

static const struct cfg_decode_map cfg_decode_map_v60 = {
	.dev_size_reg = BRCMNAND_CS_CFG,
	.dev_size_shift = 24,
	.dev_size_mask = (0xf << 24),
	.block_size_reg = BRCMNAND_CS_CFG,
	.block_size_shift = 28,
	.block_size_mask = (0x7 << 28),
	.page_size_reg = BRCMNAND_CS_CFG,
	.page_size_shift = 20,
	.page_size_mask = (0x3 << 20),
	.block_tbl = blk_tbl_v60,
	.page_tbl = pg_tbl_v60,
};

static const struct cfg_decode_map cfg_decode_map_v71 = {
	.dev_size_reg = BRCMNAND_CS_CFG,
	.dev_size_shift = 24,
	.dev_size_mask = (0xf << 24),
	.block_size_reg = BRCMNAND_CS_CFG_EXT,
	.block_size_shift = 4,
	.block_size_mask = (0xff << 4),
	.page_size_reg = BRCMNAND_CS_CFG_EXT,
	.page_size_shift = 0,
	.page_size_mask = 0xf,
	.block_tbl = NULL,
	.page_tbl = NULL,
};

static struct brcmnand_chip nand_chip;
static struct brcmnand_controller nand_ctrl;

static u32 nand_readreg(struct brcmnand_controller *ctrl, u32 offs)
{
	return brcmnand_readl(ctrl->nand_base + offs);
}

static void nand_writereg(struct brcmnand_controller *ctrl, u32 offs, u32 val)
{
	brcmnand_writel(val, ctrl->nand_base + offs);
}

static u32 brcmnand_read_reg(struct brcmnand_controller *ctrl, enum brcmnand_reg reg)
{
	u16 offs = ctrl->reg_offsets[reg];

	if (offs)
		return nand_readreg(ctrl, offs);
	else
		return 0;
}

static void brcmnand_write_reg(struct brcmnand_controller *ctrl, enum brcmnand_reg reg, u32 val)
{
	u16 offs = ctrl->reg_offsets[reg];

	if (offs)
		nand_writereg(ctrl, offs, val);
}

static void brcmnand_rmw_reg(struct brcmnand_controller *ctrl, enum brcmnand_reg reg, u32 mask, unsigned
			     int shift, u32 val)
{
	u32 tmp = brcmnand_read_reg(ctrl, reg);

	tmp &= ~mask;
	tmp |= val << shift;
	brcmnand_write_reg(ctrl, reg, tmp);
}

static u32 brcmnand_read_fc(struct brcmnand_controller *ctrl, uint32_t offset)
{
	return __raw_readl(ctrl->nand_fc + offset);
}

static u8 oob_reg_read(struct brcmnand_controller *ctrl, u32 offs)
{
	u16 offset0, reg_offs;

	offset0 = ctrl->reg_offsets[BRCMNAND_OOB_READ_BASE];

	reg_offs = offset0 + (offs & ~0x03);

	return nand_readreg(ctrl, reg_offs) >> (24 - ((offs & 0x03) << 3));
}

static int brcmnand_revision_init(struct brcmnand_controller *ctrl)
{
	ctrl->nand_version = nand_readreg(ctrl, 0) & 0xffff;

	/* Only support v4.0+? */
	if (ctrl->nand_version < 0x0600 || ctrl->nand_version > 0x0701) {
		dev_err(ctrl->dev, "version %#x not supported\n", ctrl->nand_version);
		return -ENODEV;
	}

	/* Register offsets */
	if (ctrl->nand_version >= 0x0701) {
		ctrl->reg_offsets = brcmnand_regs_v71;
		ctrl->cfg_dec_map = &cfg_decode_map_v71;
	} else {
		ctrl->reg_offsets = brcmnand_regs_v60;
		ctrl->cfg_dec_map = &cfg_decode_map_v60;
	}

	return 0;
}

static int brcmnand_wait_status(struct brcmnand_controller *ctrl, unsigned int status_mask)
{

	const unsigned int nand_poll_max = 2000000;

	unsigned int data;
	unsigned int poll_count = 0;
	int ret = 0;

	do {
		data = brcmnand_read_reg(ctrl, BRCMNAND_INTFC_STATUS);
	} while (!(status_mask & data) && (++poll_count < nand_poll_max));

	data = brcmnand_read_reg(ctrl, BRCMNAND_INTFC_STATUS);
	if (!(status_mask & data)) {
		printf("Status wait timeout: nandsts=0x%8.8x mask=0x%8.8x, count=" "%u\n", data, status_mask, poll_count);
		ret = -ETIMEDOUT;
	}

	return (ret);
}

extern int brcmnand_wait_cmd(struct brcmnand_controller *ctrl)
{
	return brcmnand_wait_status(ctrl, NIS_CTLR_READY);
}

extern int brcmnand_wait_device(struct brcmnand_controller *ctrl)
{
	return brcmnand_wait_status(ctrl, NIS_FLASH_READY);
}

extern int brcmnand_wait_cache(struct brcmnand_controller *ctrl)
{
	return brcmnand_wait_status(ctrl, NIS_CACHE_VALID);
}

extern int brcmnand_wait_spare(struct brcmnand_controller *ctrl)
{
	return brcmnand_wait_status(ctrl, NIS_SPARE_VALID);
}

static void brcmnand_reset_device(struct brcmnand_controller *ctrl)
{
	brcmnand_write_reg(ctrl, BRCMNAND_CS_SELECT, (NBC_AUTO_DEV_ID_CFG | 1));
	brcmnand_wait_device(ctrl);
	brcmnand_write_reg(ctrl, BRCMNAND_CS_XOR, 0x0);
}

static uint32_t brcmnand_block_size_mapped(uint32_t * block_tbl, uint32_t block_size)
{
	uint32_t i = 0;

	while (block_tbl[i]) {
		if (block_tbl[i] == block_size)
			return i;
		i++;
	}

	printf("Invalid block size %dKB for this nand controller!\n", block_size >> 10);
	return 0;
}

static uint64_t brcmnand_get_dev_size(struct brcmnand_controller *ctrl)
{
	uint32_t dev_size, reg;
	const struct cfg_decode_map *cfg_map = ctrl->cfg_dec_map;

	reg = brcmnand_read_reg(ctrl, cfg_map->dev_size_reg);
	dev_size = (reg & cfg_map->dev_size_mask) >> cfg_map->dev_size_shift;

	return 1ULL << (dev_size + 22);
}

static uint32_t brcmnand_get_blk_size(struct brcmnand_controller *ctrl)
{
	uint32_t blk_size, reg;
	const struct cfg_decode_map *cfg_map = ctrl->cfg_dec_map;

	reg = brcmnand_read_reg(ctrl, cfg_map->block_size_reg);
	blk_size = (reg & cfg_map->block_size_mask) >> cfg_map->block_size_shift;

	if (cfg_map->block_tbl)
		return cfg_map->block_tbl[blk_size];
	else
		return 1 << (blk_size + 13);
}

static uint32_t brcmnand_get_pg_size(struct brcmnand_controller *ctrl)
{
	uint32_t page_size, reg;
	const struct cfg_decode_map *cfg_map = ctrl->cfg_dec_map;

	reg = brcmnand_read_reg(ctrl, cfg_map->page_size_reg);
	page_size = (reg & cfg_map->page_size_mask) >> cfg_map->page_size_shift;

	if (cfg_map->page_tbl)
		return cfg_map->page_tbl[page_size];
	else
		return 1 << (page_size + 9);
}

static void brcmnand_set_dev_size(struct brcmnand_controller *ctrl, uint32_t dev_size_in_order)
{
	const struct cfg_decode_map *cfg_map = ctrl->cfg_dec_map;

	brcmnand_rmw_reg(ctrl, cfg_map->dev_size_reg, cfg_map->dev_size_mask, cfg_map->dev_size_shift, dev_size_in_order - 22);
}

static void brcmnand_set_block_size(struct brcmnand_controller *ctrl, uint32_t block_size_in_order)
{
	const struct cfg_decode_map *cfg_map = ctrl->cfg_dec_map;
	uint32_t block_size, reg_val;

	if (cfg_map->block_tbl) {
		block_size = 1 << block_size_in_order;
		reg_val = brcmnand_block_size_mapped(cfg_map->block_tbl, block_size);
	} else
		reg_val = block_size_in_order - 13;

	brcmnand_rmw_reg(ctrl, cfg_map->block_size_reg, cfg_map->block_size_mask, cfg_map->block_size_shift, reg_val);
}

static void brcmnand_copy_from_cache(struct brcmnand_controller *ctrl, unsigned char *buffer, int offset, int numbytes)
{
	int use_buffer = 0, read_bytes, i;
	uint32_t *buf;

#if defined(CONFIG_BCM47189)
	{
		uint32_t ioctrl = NAND_FLASH_CTRL_WRAP->ioctrl;
		ioctrl = ioctrl | NAND_APB_LITTLE_ENDIAN;
		NAND_FLASH_CTRL_WRAP->ioctrl = ioctrl;
	}
#endif
	if (offset & 0x3) {
		printk("brcmnand_copy_from_cache invalid offset %d!\n", offset);
		return;
	}

	use_buffer = ((uintptr_t) buffer & 0x3) || (numbytes & 0x3);
	if (use_buffer)
		buf = (uint32_t *) ctrl->flash_cache;
	else
		buf = (uint32_t *) buffer;
	read_bytes = ((numbytes + 0x3) >> 2) << 2;

	debug(">> brcmnand_copy_from_cache - use_buffer %d buffer 0x%p offset %d read bytes %d\n",
	      use_buffer, buf, offset, read_bytes);

	for (i = 0; i < read_bytes; i += 4, buf++) {
		*buf = brcmnand_read_fc(ctrl, i + offset);
#if 0
		debug("0x%08x ", *buf);
		if ((i + 1) % 16 == 0)
			debug("\n");
#endif
	}
	//debug("\n");

	if (use_buffer)
		memcpy(buffer, ctrl->flash_cache, numbytes);

#if defined(CONFIG_BCM47189)
	{
		uint32_t ioctrl = NAND_FLASH_CTRL_WRAP->ioctrl;
		ioctrl = ioctrl & ~NAND_APB_LITTLE_ENDIAN;
		NAND_FLASH_CTRL_WRAP->ioctrl = ioctrl;
	}
#endif

}

static void brcmnand_copy_from_spare(struct brcmnand_controller *ctrl, unsigned char *buffer, int numbytes)
{
	for (int i = 0; i < numbytes; i++)
		buffer[i] = oob_reg_read(ctrl, i);
}

static void brcmnand_check_onfi(struct brcmnand_chip *chip, struct brcmnand_controller *ctrl)
{
	struct nand_onfi_params onfi;
	uint64_t onfi_total_size;
	uint32_t size_in_order = 0;

	memset(&onfi, 0x0, sizeof(onfi));
	brcmnand_write_reg(ctrl, BRCMNAND_CMD_START, CMD_PARAMETER_READ);
	if (brcmnand_wait_cmd(ctrl) == 0 && brcmnand_wait_cache(ctrl) == 0) {
		// Hardware NAND controller does not take into account LUNs, so if this value is not 1 we calculate the die stack NAND size
		brcmnand_copy_from_cache(ctrl, (unsigned char *)&onfi, 0, sizeof(onfi));
		if ((onfi.sig[0] == 'O') && (onfi.sig[1] == 'N') && (onfi.sig[2] == 'F') && (onfi.sig[3] == 'I')) {
			debug("ONFI detected, page size 0x%x, page per block %d block per lun %d lun count%d\n",
			      le32_to_cpu(onfi.byte_per_page), le32_to_cpu(onfi.pages_per_block),
			      le32_to_cpu(onfi.blocks_per_lun), onfi.lun_count);

			//adjust size based on # of luns
			if (onfi.lun_count != 1) {
				onfi_total_size = le32_to_cpu(onfi.byte_per_page) * le32_to_cpu(onfi.pages_per_block);
				onfi_total_size *= le32_to_cpu(onfi.blocks_per_lun);
				onfi_total_size *= onfi.lun_count;

				if (onfi_total_size != chip->chip_total_size) {
					printf("Correct total size based on ONFI old size 0x%llx to new size 0x%llx\n",
					       chip->chip_total_size, onfi_total_size);
					chip->chip_total_size = onfi_total_size;
					while (onfi_total_size >>= 1) {
						size_in_order++;
					}

					brcmnand_set_dev_size(ctrl, size_in_order);
				}
			}
		}
	}
}

static int brcmnand_adjust_cfg(struct brcmnand_chip *chip)
{
	struct brcmnand_controller *ctrl = chip->ctrl;
	const struct cfg_decode_map *cfg_map = ctrl->cfg_dec_map;
	uint32_t mask, reg_val;
	uint32_t ecc_lvl;

	/* Special case changes from what the NAND controller configured. */
	switch (NAND_CHIPID(chip)) {
	case NAND_MAKE_ID(FLASHTYPE_HYNIX, HYNIX_H27U1G8F2B):
		/* 128 MB device size, 4 full address bytes, 2 column address bytes, 2 block address bytes */
		mask = cfg_map->dev_size_mask | NC_FUL_ADDR_MASK | NC_COL_ADDR_MASK | NC_BLK_ADDR_MASK;
		reg_val =
		    (5 << cfg_map->
		     dev_size_shift) | (0x04 << NC_FUL_ADDR_SHIFT) | (0x2 << NC_COL_ADDR_SHIFT) | (0x2 << NC_BLK_ADDR_SHIFT);
		brcmnand_rmw_reg(ctrl, BRCMNAND_CS_CFG, mask, 0, reg_val);
		break;

	case NAND_MAKE_ID(FLASHTYPE_SAMSUNG, SAMSUNG_K9F5608U0A):
	case NAND_MAKE_ID(FLASHTYPE_SAMSUNG, SAMSUNG_K9F1208U0):
	case NAND_MAKE_ID(FLASHTYPE_SAMSUNG, SAMSUNG_K9F1G08U0):
	case NAND_MAKE_ID(FLASHTYPE_HYNIX, HYNIX_H27U518S2C):
		/* Set device id "cell type" to 0 (SLC). */
		chip->chip_device_id &= ~NAND_CI_CELLTYPE_MSK;
		brcmnand_write_reg(ctrl, BRCMNAND_ID, chip->chip_device_id);
		break;

	case NAND_MAKE_ID(FLASHTYPE_MXIC, MXIC_MX30LF1208AA):
		/* This 64MB device was detected as 256MB device on 63268. Manually update
		 * device size in the cfg register.
		 */
		brcmnand_set_dev_size(ctrl, 26);
		break;

	case NAND_MAKE_ID(FLASHTYPE_SPANSION, SPANSION_S34ML01G1):
		/* Set device size to 128MB, it is misconfigured to 512MB. */
		brcmnand_set_dev_size(ctrl, 27);
		break;

	case NAND_MAKE_ID(FLASHTYPE_SPANSION, SPANSION_S34ML02G1):
		/* Set device size to 256MB, it is misconfigured to 512MB. */
		brcmnand_set_dev_size(ctrl, 28);
		break;

	case NAND_MAKE_ID(FLASHTYPE_SPANSION, SPANSION_S34ML04G1):
		/* Set block size to 128KB, it is misconfigured to 512MB in 63138, 47189. */
		if (ctrl->nand_version <= 0x00000700)
			brcmnand_set_block_size(ctrl, 17);
		break;
	}

	chip->chip_total_size = brcmnand_get_dev_size(ctrl);
	chip->chip_block_size = brcmnand_get_blk_size(ctrl);
	chip->chip_page_size = brcmnand_get_pg_size(ctrl);

	/* for SPL we don't care spare area. Only need to know the bbi location */
	reg_val = brcmnand_read_reg(ctrl, BRCMNAND_CS_ACC_CONTROL);
	chip->chip_ecc_level = ecc_lvl = (reg_val & NAC_ECC_LVL_MASK) >> NAC_ECC_LVL_SHIFT;

	/* The access control register spare size is the number of spare area
	 * bytes per 512 bytes of data.  The chip_spare_size is the number
	 * of spare area bytes per page.
	 */
	chip->chip_spare_step_size = ((reg_val & NAC_SPARE_SZ_MASK) >> NAC_SPARE_SZ_SHIFT);
	chip->chip_spare_size = chip->chip_spare_step_size * (chip->chip_page_size >> 9);

	if (ecc_lvl == NAC_ECC_LVL_HAMMING) {
		if (chip->chip_page_size == 512) {
			chip->chip_bi_index_1 = chip->chip_bi_index_2 = 5;
		} else {
			chip->chip_bi_index_1 = 0;
			chip->chip_bi_index_2 = 1;
		}
	} else if (ecc_lvl == NAC_ECC_LVL_BCH_4 && chip->chip_page_size == 512) {
		chip->chip_bi_index_1 = chip->chip_bi_index_2 = 5;
	} else
		chip->chip_bi_index_1 = chip->chip_bi_index_2 = 0;

	chip->sector_size_1k = (reg_val & NAC_SECTOR_SIZE_1K) ? 1 : 0;

	brcmnand_check_onfi(chip, ctrl);

	return 0;
}

static void brcmnand_adjust_timing(struct brcmnand_chip *chip)
{
	struct brcmnand_controller *ctrl = chip->ctrl;

	/* adjust reading timing */
	/* Default of TRP=4 and TREAD=5 for Hynix parts on 63268 */
	/* Almost all parts could use TRP=3 and TREAD=4 */

	brcmnand_rmw_reg(ctrl, BRCMNAND_TIMING1, NT_TREH_MASK, NT_TREH_SHIFT, 2);
	brcmnand_rmw_reg(ctrl, BRCMNAND_TIMING1, NT_TRP_MASK, NT_TRP_SHIFT, 4);

	brcmnand_rmw_reg(ctrl, BRCMNAND_TIMING2, NT_TREAD_MASK, NT_TREAD_SHIFT, 5);
}

/* Simplified version for bad block marker read. Only read the first spare area step size */
/* This is good enough for BBM */
static int brcmnand_read_spare_area(struct brcmnand_chip *chip, uint64_t page_addr, unsigned char *buffer, int len)
{
	int ret = -EIO;
	struct brcmnand_controller *ctrl = chip->ctrl;

	if (len > chip->chip_spare_step_size)
		len = chip->chip_spare_step_size;

	brcmnand_write_reg(ctrl, BRCMNAND_CMD_ADDRESS, (uint32_t) page_addr);
	brcmnand_write_reg(ctrl, BRCMNAND_CMD_EXT_ADDRESS, (uint32_t) (page_addr >> 32));
	brcmnand_write_reg(ctrl, BRCMNAND_CMD_START, CMD_PAGE_READ);

	if ((ret = brcmnand_wait_cmd(ctrl)) == 0) {
		/* wait until data is available in the spare area registers */
		if ((ret = brcmnand_wait_spare(ctrl)) == 0)
			brcmnand_copy_from_spare(ctrl, buffer, len);
	}

	return ret;
}

static int brcmnand_read_page(struct brcmnand_chip *chip, uint64_t start_addr, unsigned char *buffer, int len)
{
	int ret = -EIO;
	struct brcmnand_controller *ctrl = chip->ctrl;

	if (len <= chip->chip_block_size) {
		uint64_t page_addr = start_addr & ~(chip->chip_page_size - 1);
		uint32_t index = 0;
		uint32_t subpage;
		int length = len;

		do {
			for (subpage = 0, ret = 0; (subpage < chip->chip_page_size) && (ret == 0); subpage += CTRLR_CACHE_SIZE) {
				brcmnand_write_reg(ctrl, BRCMNAND_CMD_ADDRESS, (uint32_t) page_addr + subpage);
				brcmnand_write_reg(ctrl, BRCMNAND_CMD_EXT_ADDRESS, (uint32_t) (page_addr >> 32));
				brcmnand_write_reg(ctrl, BRCMNAND_CMD_START, CMD_PAGE_READ);

				if ((ret = brcmnand_wait_cmd(ctrl)) == 0) {
					/* wait until data is available in the cache */
					if ((ret = brcmnand_wait_cache(ctrl)) != 0) {
						ret = -EIO;
					}

					if ((ret == 0) && (start_addr < (page_addr + subpage + CTRLR_CACHE_SIZE)) && ((start_addr + len) > page_addr + subpage)) {	// copy from cache only if buffer is within the subpage
						uint32_t copy_size, offset;

						if (start_addr <= page_addr + subpage) {
							offset = 0;

							if ((start_addr + len) >= (page_addr + subpage + CTRLR_CACHE_SIZE))
								copy_size = CTRLR_CACHE_SIZE;
							else
								copy_size = (start_addr + len) - (page_addr + subpage);
						} else {	// start_addr > page_addr + subpage
							offset = start_addr - (page_addr + subpage);

							if ((start_addr + len) >= (page_addr + subpage + CTRLR_CACHE_SIZE))
								copy_size = page_addr + subpage + CTRLR_CACHE_SIZE - start_addr;
							else
								copy_size = start_addr + len - start_addr;
						}

						brcmnand_copy_from_cache(ctrl, &buffer[index], offset, copy_size);

						index += copy_size;
						length -= copy_size;
					}
				}
			}

			if (ret != 0)
				break;

			page_addr += chip->chip_page_size;

		} while (length);
	}

	return (ret);
}

void brcmnand_init(void)
{
	struct brcmnand_chip *chip = &nand_chip;
	struct brcmnand_controller *ctrl = &nand_ctrl;

	ctrl->flash_cache = memalign(sizeof(uint32_t), CTRLR_CACHE_SIZE);
	if (ctrl->flash_cache == NULL) {
		printf("nand_flash_init failed to allocate flash buffer!\n");
		hang();
	}

	/* TODO get this base from device tree */
	ctrl->nand_base = (void __iomem *)CONFIG_SYS_NAND_BASE;
	ctrl->nand_fc = (void __iomem *)(CONFIG_SYS_NAND_BASE + 0x400);

	chip->ctrl = ctrl;
	brcmnand_revision_init(ctrl);
	brcmnand_reset_device(ctrl);

	/* Read the chip id. Only use the most signficant 16 bits. */
	chip->chip_device_id = brcmnand_read_reg(ctrl, BRCMNAND_ID);
	brcmnand_adjust_cfg(chip);
	brcmnand_adjust_timing(chip);

	printf("nand flash device id 0x%x, total size %dMB\n", chip->chip_device_id, (uint32_t) (chip->chip_total_size >> 20));
	printf("block size %dKB, page size %d bytes, spare area %d bytes\n",
	       chip->chip_block_size >> 10, chip->chip_page_size, chip->chip_spare_size);
	if (chip->chip_ecc_level == 0)
		printf("ECC disabled\n");
	else if (chip->chip_ecc_level == NAC_ECC_LVL_HAMMING)
		printf("ECC Hamming\n");
	else {
		printf("ECC BCH-%d", chip->chip_ecc_level << chip->sector_size_1k);
		printf(" %s\n", chip->sector_size_1k ? "(1KB sector)" : "");
	}
}

/* Check if the block is good or bad. If bad returns 1, if good returns 0 */
int brcmnand_is_bad_block(int blk)
{
	struct brcmnand_chip *chip = &nand_chip;
	unsigned char spare[16];
	uint32_t page_addr = (blk * chip->chip_block_size) & ~(chip->chip_page_size - 1);
	int i, size;

	// always return good for block 0, because if it's a bad chip quite possibly the board is useless
	if (blk == 0)
		return 0;

	/* bad block markers are always within first spare area step size. only need to read this many bytes */
	size = max(chip->chip_bi_index_1, chip->chip_bi_index_2) + 1;
	if (size > chip->chip_spare_step_size || size > 16) {
		printf("bad block marker invalid location %d %d\n",
			chip->chip_bi_index_1, chip->chip_bi_index_2);
		return 1;
	}
	    
	/* Read the spare area of first and second page and check for bad block indicator */
	for (i = 0; i < 2; i += 1, page_addr += chip->chip_page_size) {
		if (brcmnand_read_spare_area(chip, page_addr, spare, size) == 0) {
			if ((spare[chip->chip_bi_index_1] != SPARE_GOOD_MARKER)
			    || (spare[chip->chip_bi_index_2] != SPARE_GOOD_MARKER)) {
				return 1;	// bad block
			}
		} else {
			return 1;	//bad block
		}
	}

	return 0;		// good block
}

int brcmnand_read_buf(int blk, int offset, u8 *buffer, u32 len)
{
	int ret;
	struct brcmnand_chip *chip = &nand_chip;
	uint64_t start_addr;
	uint64_t blk_addr;
	uint32_t blk_offset;
	uint32_t size;
	uint32_t total_block = chip->chip_total_size / chip->chip_block_size;

	ret = len;

	debug(">> brcmnand_read_buf - 1 blk=0x%8.8x, offset=%d, len=%d buffer 0x%p\n", blk, offset, len, buffer);

	start_addr = (blk * chip->chip_block_size) + offset;
	blk_addr = start_addr & ~(chip->chip_block_size - 1);
	blk_offset = start_addr - blk_addr;
	size = chip->chip_block_size - blk_offset;

	if (size > len)
		size = len;

	if (blk >= total_block) {
		printf("Attempt to read block number(%d) beyond the nand max blk(%d) \n", blk, total_block - 1);
		return -EINVAL;
	}

	if (len)
		do {
			if (brcmnand_read_page(chip, start_addr, buffer, size) != 0) {
				ret = -EIO;
				break;
			}

			len -= size;
			if (len) {
				blk++;

				debug(">> brcmnand_read_buf - 2 blk=0x%8.8x, len=%u\n", blk, len);

				start_addr = blk * chip->chip_block_size;
				buffer += size;
				if (len > chip->chip_block_size)
					size = chip->chip_block_size;
				else
					size = len;
			}
		} while (len);

	if (brcmnand_is_bad_block(blk)) {	/* don't check for bad block during page read/write since may be reading/writing to bad block marker,
					   check for bad block after read to allow for data recovery */
		printf("brcmnand_read_buf(): Attempt to read bad nand block %d\n", blk);
		return -EIO;
	}

	debug(">> brcmnand_read_buf - ret=%d\n", ret);

	return (ret);
}

uint32_t brcmnand_get_page_size(void)
{
	return (nand_chip.chip_page_size);
}

uint32_t brcmnand_get_block_size(void)
{
	return (nand_chip.chip_block_size);
}

uint64_t brcmnand_get_total_size(void)
{
	return nand_chip.chip_total_size;
}

