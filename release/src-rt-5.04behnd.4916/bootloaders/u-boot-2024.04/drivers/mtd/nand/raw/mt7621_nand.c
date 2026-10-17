// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <log.h>
#include <nand.h>
#include <malloc.h>
#include <asm/addrspace.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/sizes.h>
#include <linux/bitops.h>
#include <linux/bitfield.h>
#include "mt7621_nand.h"

/* NFI core registers */
#define NFI_CNFG			0x000
#define   CNFG_OP_MODE			GENMASK(14, 12)
#define     CNFG_OP_CUSTOM		6
#define   CNFG_AUTO_FMT_EN		BIT(9)
#define   CNFG_HW_ECC_EN		BIT(8)
#define   CNFG_BYTE_RW			BIT(6)
#define   CNFG_READ_MODE		BIT(1)

#define NFI_PAGEFMT			0x004
#define   PAGEFMT_FDM_ECC		GENMASK(15, 12)
#define   PAGEFMT_FDM			GENMASK(11, 8)
#define   PAGEFMT_SPARE			GENMASK(5, 4)
#define   PAGEFMT_PAGE			GENMASK(1, 0)

#define NFI_CON				0x008
#define   CON_NFI_SEC			GENMASK(15, 12)
#define   CON_NFI_BWR			BIT(9)
#define   CON_NFI_BRD			BIT(8)
#define   CON_NFI_RST			BIT(1)
#define   CON_FIFO_FLUSH		BIT(0)

#define NFI_ACCCON			0x00c
#define   ACCCON_POECS			GENMASK(31, 28)
#define   ACCCON_POECS_DEF		3
#define   ACCCON_PRECS			GENMASK(27, 22)
#define   ACCCON_PRECS_DEF		3
#define   ACCCON_C2R			GENMASK(21, 16)
#define   ACCCON_C2R_DEF		7
#define   ACCCON_W2R			GENMASK(15, 12)
#define   ACCCON_W2R_DEF		7
#define   ACCCON_WH			GENMASK(11, 8)
#define   ACCCON_WH_DEF			15
#define   ACCCON_WST			GENMASK(7, 4)
#define   ACCCON_WST_DEF		15
#define   ACCCON_WST_MIN		3
#define   ACCCON_RLT			GENMASK(3, 0)
#define   ACCCON_RLT_DEF		15
#define   ACCCON_RLT_MIN		3

#define NFI_CMD				0x020

#define NFI_ADDRNOB			0x030
#define   ADDR_ROW_NOB			GENMASK(6, 4)
#define   ADDR_COL_NOB			GENMASK(2, 0)

#define NFI_COLADDR			0x034
#define NFI_ROWADDR			0x038

#define NFI_STRDATA			0x040
#define   STR_DATA			BIT(0)

#define NFI_CNRNB			0x044
#define   CB2R_TIME			GENMASK(7, 4)
#define   STR_CNRNB			BIT(0)

#define NFI_DATAW			0x050
#define NFI_DATAR			0x054

#define NFI_PIO_DIRDY			0x058
#define   PIO_DIRDY			BIT(0)

#define NFI_STA				0x060
#define   STA_NFI_FSM			GENMASK(19, 16)
#define     STA_FSM_CUSTOM_DATA		14
#define   STA_BUSY			BIT(8)
#define   STA_ADDR			BIT(1)
#define   STA_CMD			BIT(0)

#define NFI_ADDRCNTR			0x070
#define   SEC_CNTR			GENMASK(15, 12)
#define   SEC_ADDR			GENMASK(9, 0)

#define NFI_CSEL			0x090
#define   CSEL				GENMASK(1, 0)

#define NFI_FDM0L			0x0a0
#define NFI_FDML(n)			(0x0a0 + ((n) << 3))

#define NFI_FDM0M			0x0a4
#define NFI_FDMM(n)			(0x0a4 + ((n) << 3))

#define NFI_MASTER_STA			0x210
#define   MAS_ADDR			GENMASK(11, 9)
#define   MAS_RD			GENMASK(8, 6)
#define   MAS_WR			GENMASK(5, 3)
#define   MAS_RDDLY			GENMASK(2, 0)

/* ECC engine registers */
#define ECC_ENCCON			0x000
#define   ENC_EN			BIT(0)

#define ECC_ENCCNFG			0x004
#define   ENC_CNFG_MSG			GENMASK(28, 16)
#define   ENC_MODE			GENMASK(5, 4)
#define     ENC_MODE_NFI		1
#define   ENC_TNUM			GENMASK(2, 0)

#define ECC_ENCIDLE			0x00c
#define   ENC_IDLE			BIT(0)

#define ECC_DECCON			0x100
#define   DEC_EN			BIT(0)

#define ECC_DECCNFG			0x104
#define   DEC_EMPTY_EN			BIT(31)
#define   DEC_CS			GENMASK(28, 16)
#define   DEC_CON			GENMASK(13, 12)
#define     DEC_CON_EL			2
#define   DEC_MODE			GENMASK(5, 4)
#define     DEC_MODE_NFI		1
#define   DEC_TNUM			GENMASK(2, 0)

#define ECC_DECIDLE			0x10c
#define   DEC_IDLE			BIT(1)

#define ECC_DECENUM			0x114
#define   ERRNUM_S			2
#define   ERRNUM_M			GENMASK(3, 0)

#define ECC_DECDONE			0x118
#define   DEC_DONE7			BIT(7)
#define   DEC_DONE6			BIT(6)
#define   DEC_DONE5			BIT(5)
#define   DEC_DONE4			BIT(4)
#define   DEC_DONE3			BIT(3)
#define   DEC_DONE2			BIT(2)
#define   DEC_DONE1			BIT(1)
#define   DEC_DONE0			BIT(0)

#define ECC_DECEL(n)			(0x11c + (n) * 4)
#define   DEC_EL_ODD_S			16
#define   DEC_EL_M			0x1fff
#define   DEC_EL_BYTE_POS_S		3
#define   DEC_EL_BIT_POS_M		GENMASK(2, 0)

#define ECC_FDMADDR			0x13c

/* ENCIDLE and DECIDLE */
#define   ECC_IDLE			BIT(0)

#define ACCTIMING(tpoecs, tprecs, tc2r, tw2r, twh, twst, trlt) \
	(FIELD_PREP(ACCCON_POECS, tpoecs) | \
	 FIELD_PREP(ACCCON_PRECS, tprecs) | \
	 FIELD_PREP(ACCCON_C2R, tc2r) | \
	 FIELD_PREP(ACCCON_W2R, tw2r) | \
	 FIELD_PREP(ACCCON_WH, twh) | \
	 FIELD_PREP(ACCCON_WST, twst) | \
	 FIELD_PREP(ACCCON_RLT, trlt))

#define MASTER_STA_MASK			(MAS_ADDR | MAS_RD | MAS_WR | \
					 MAS_RDDLY)
#define NFI_RESET_TIMEOUT		1000000
#define NFI_CORE_TIMEOUT		500000
#define ECC_ENGINE_TIMEOUT		500000

#define ECC_SECTOR_SIZE			512
#define ECC_PARITY_BITS			13

#define NFI_FDM_SIZE			8

/* Register base */
#define NFI_BASE			0x1e003000
#define NFI_ECC_BASE			0x1e003800

static struct mt7621_nfc nfc_dev;

static const u16 mt7621_nfi_page_size[] = { SZ_512, SZ_2K, SZ_4K };
static const u8 mt7621_nfi_spare_size[] = { 16, 26, 27, 28 };
static const u8 mt7621_ecc_strength[] = { 4, 6, 8, 10, 12 };

static inline u32 nfi_read32(struct mt7621_nfc *nfc, u32 reg)
{
	return readl(nfc->nfi_regs + reg);
}

static inline void nfi_write32(struct mt7621_nfc *nfc, u32 reg, u32 val)
{
	writel(val, nfc->nfi_regs + reg);
}

static inline u16 nfi_read16(struct mt7621_nfc *nfc, u32 reg)
{
	return readw(nfc->nfi_regs + reg);
}

static inline void nfi_write16(struct mt7621_nfc *nfc, u32 reg, u16 val)
{
	writew(val, nfc->nfi_regs + reg);
}

static inline void ecc_write16(struct mt7621_nfc *nfc, u32 reg, u16 val)
{
	writew(val, nfc->ecc_regs + reg);
}

static inline u32 ecc_read32(struct mt7621_nfc *nfc, u32 reg)
{
	return readl(nfc->ecc_regs + reg);
}

static inline void ecc_write32(struct mt7621_nfc *nfc, u32 reg, u32 val)
{
	return writel(val, nfc->ecc_regs + reg);
}

static inline u8 *oob_fdm_ptr(struct nand_chip *nand, int sect)
{
	return nand->oob_poi + sect * NFI_FDM_SIZE;
}

static inline u8 *oob_ecc_ptr(struct mt7621_nfc *nfc, int sect)
{
	struct nand_chip *nand = &nfc->nand;

	return nand->oob_poi + nand->ecc.steps * NFI_FDM_SIZE +
		sect * (nfc->spare_per_sector - NFI_FDM_SIZE);
}

static inline u8 *page_data_ptr(struct nand_chip *nand, const u8 *buf,
				int sect)
{
	return (u8 *)buf + sect * nand->ecc.size;
}

static int mt7621_ecc_wait_idle(struct mt7621_nfc *nfc, u32 reg)
{
	u32 val;
	int ret;

	ret = readw_poll_timeout(nfc->ecc_regs + reg, val, val & ECC_IDLE,
				 ECC_ENGINE_TIMEOUT);
	if (ret) {
		pr_warn("ECC engine timed out entering idle mode\n");
		return -EIO;
	}

	return 0;
}

static int mt7621_ecc_decoder_wait_done(struct mt7621_nfc *nfc, u32 sect)
{
	u32 val;
	int ret;

	ret = readw_poll_timeout(nfc->ecc_regs + ECC_DECDONE, val,
				 val & (1 << sect), ECC_ENGINE_TIMEOUT);
	if (ret) {
		pr_warn("ECC decoder for sector %d timed out\n", sect);
		return -ETIMEDOUT;
	}

	return 0;
}

static void mt7621_ecc_encoder_op(struct mt7621_nfc *nfc, bool enable)
{
	mt7621_ecc_wait_idle(nfc, ECC_ENCIDLE);
	ecc_write16(nfc, ECC_ENCCON, enable ? ENC_EN : 0);
}

static void mt7621_ecc_decoder_op(struct mt7621_nfc *nfc, bool enable)
{
	mt7621_ecc_wait_idle(nfc, ECC_DECIDLE);
	ecc_write16(nfc, ECC_DECCON, enable ? DEC_EN : 0);
}

static int mt7621_ecc_correct_check(struct mt7621_nfc *nfc, u8 *sector_buf,
				    u8 *fdm_buf, u32 sect)
{
	struct nand_chip *nand = &nfc->nand;
	u32 decnum, num_error_bits, fdm_end_bits;
	u32 error_locations, error_bit_loc;
	u32 error_byte_pos, error_bit_pos;
	int bitflips = 0;
	u32 i;

	decnum = ecc_read32(nfc, ECC_DECENUM);
	num_error_bits = (decnum >> (sect << ERRNUM_S)) & ERRNUM_M;
	fdm_end_bits = (nand->ecc.size + NFI_FDM_SIZE) << 3;

	if (!num_error_bits)
		return 0;

	if (num_error_bits == ERRNUM_M)
		return -1;

	for (i = 0; i < num_error_bits; i++) {
		error_locations = ecc_read32(nfc, ECC_DECEL(i / 2));
		error_bit_loc = (error_locations >> ((i % 2) * DEC_EL_ODD_S)) &
				DEC_EL_M;
		error_byte_pos = error_bit_loc >> DEC_EL_BYTE_POS_S;
		error_bit_pos = error_bit_loc & DEC_EL_BIT_POS_M;

		if (error_bit_loc < (nand->ecc.size << 3)) {
			if (sector_buf) {
				sector_buf[error_byte_pos] ^=
					(1 << error_bit_pos);
			}
		} else if (error_bit_loc < fdm_end_bits) {
			if (fdm_buf) {
				fdm_buf[error_byte_pos - nand->ecc.size] ^=
					(1 << error_bit_pos);
			}
		}

		bitflips++;
	}

	return bitflips;
}

static int mt7621_nfc_wait_write_completion(struct mt7621_nfc *nfc,
					    struct nand_chip *nand)
{
	u16 val;
	int ret;

	ret = readw_poll_timeout(nfc->nfi_regs + NFI_ADDRCNTR, val,
				 FIELD_GET(SEC_CNTR, val) >= nand->ecc.steps,
				 NFI_CORE_TIMEOUT);

	if (ret) {
		pr_warn("NFI core write operation timed out\n");
		return -ETIMEDOUT;
	}

	return ret;
}

static void mt7621_nfc_hw_reset(struct mt7621_nfc *nfc)
{
	u32 val;
	int ret;

	/* reset all registers and force the NFI master to terminate */
	nfi_write16(nfc, NFI_CON, CON_FIFO_FLUSH | CON_NFI_RST);

	/* wait for the master to finish the last transaction */
	ret = readw_poll_timeout(nfc->nfi_regs + NFI_MASTER_STA, val,
				 !(val & MASTER_STA_MASK), NFI_RESET_TIMEOUT);
	if (ret) {
		pr_warn("Failed to reset NFI master in %dms\n",
			NFI_RESET_TIMEOUT);
	}

	/* ensure any status register affected by the NFI master is reset */
	nfi_write16(nfc, NFI_CON, CON_FIFO_FLUSH | CON_NFI_RST);
	nfi_write16(nfc, NFI_STRDATA, 0);
}

static inline void mt7621_nfc_hw_init(struct mt7621_nfc *nfc)
{
	u32 acccon;

	/*
	 * CNRNB: nand ready/busy register
	 * -------------------------------
	 * 7:4: timeout register for polling the NAND busy/ready signal
	 * 0  : poll the status of the busy/ready signal after [7:4]*16 cycles.
	 */
	nfi_write16(nfc, NFI_CNRNB, CB2R_TIME | STR_CNRNB);

	mt7621_nfc_hw_reset(nfc);

	/* Apply default access timing */
	acccon = ACCTIMING(ACCCON_POECS_DEF, ACCCON_PRECS_DEF, ACCCON_C2R_DEF,
			   ACCCON_W2R_DEF, ACCCON_WH_DEF, ACCCON_WST_DEF,
			   ACCCON_RLT_DEF);

	nfi_write32(nfc, NFI_ACCCON, acccon);
}

static int mt7621_nfc_send_command(struct mt7621_nfc *nfc, u8 command)
{
	u32 val;
	int ret;

	nfi_write32(nfc, NFI_CMD, command);

	ret = readl_poll_timeout(nfc->nfi_regs + NFI_STA, val, !(val & STA_CMD),
				 NFI_CORE_TIMEOUT);
	if (ret) {
		pr_warn("NFI core timed out entering command mode\n");
		return -EIO;
	}

	return 0;
}

static int mt7621_nfc_send_address_byte(struct mt7621_nfc *nfc, int addr)
{
	u32 val;
	int ret;

	nfi_write32(nfc, NFI_COLADDR, addr);
	nfi_write32(nfc, NFI_ROWADDR, 0);
	nfi_write16(nfc, NFI_ADDRNOB, 1);

	ret = readl_poll_timeout(nfc->nfi_regs + NFI_STA, val,
				 !(val & STA_ADDR), NFI_CORE_TIMEOUT);
	if (ret) {
		pr_warn("NFI core timed out entering address mode\n");
		return -EIO;
	}

	return 0;
}

static void mt7621_nfc_cmd_ctrl(struct mtd_info *mtd, int dat,
				unsigned int ctrl)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(mtd_to_nand(mtd));

	if (ctrl & NAND_ALE) {
		mt7621_nfc_send_address_byte(nfc, dat & 0xff);
	} else if (ctrl & NAND_CLE) {
		mt7621_nfc_hw_reset(nfc);
		nfi_write16(nfc, NFI_CNFG,
			    FIELD_PREP(CNFG_OP_MODE, CNFG_OP_CUSTOM));
		mt7621_nfc_send_command(nfc, dat);
	}
}

static int mt7621_nfc_dev_ready(struct mtd_info *mtd)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(mtd_to_nand(mtd));

	if (nfi_read32(nfc, NFI_STA) & STA_BUSY)
		return 0;

	return 1;
}

static void mt7621_nfc_select_chip(struct mtd_info *mtd, int chipnr)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(mtd_to_nand(mtd));

	nfi_write16(nfc, NFI_CSEL, 0);
}

static void mt7621_nfc_wait_pio_ready(struct mt7621_nfc *nfc)
{
	int ret;
	u16 val;

	ret = readw_poll_timeout(nfc->nfi_regs + NFI_PIO_DIRDY, val,
				 val & PIO_DIRDY, NFI_CORE_TIMEOUT);
	if (ret < 0)
		pr_err("NFI core PIO mode not ready\n");
}

static u32 mt7621_nfc_pio_read(struct mt7621_nfc *nfc, bool br)
{
	u32 reg, fsm;

	/* after each byte read, the NFI_STA reg is reset by the hardware */
	reg = nfi_read32(nfc, NFI_STA);
	fsm = FIELD_GET(STA_NFI_FSM, reg);

	if (fsm != STA_FSM_CUSTOM_DATA) {
		reg = nfi_read16(nfc, NFI_CNFG);
		reg |= CNFG_READ_MODE | CNFG_BYTE_RW;
		if (!br)
			reg &= ~CNFG_BYTE_RW;
		nfi_write16(nfc, NFI_CNFG, reg);

		/*
		 * set to max sector to allow the HW to continue reading over
		 * unaligned accesses
		 */
		nfi_write16(nfc, NFI_CON, CON_NFI_SEC | CON_NFI_BRD);

		/* trigger to fetch data */
		nfi_write16(nfc, NFI_STRDATA, STR_DATA);
	}

	mt7621_nfc_wait_pio_ready(nfc);

	return nfi_read32(nfc, NFI_DATAR);
}

static void mt7621_nfc_read_data(struct mt7621_nfc *nfc, u8 *buf, u32 len)
{
	while (((uintptr_t)buf & 3) && len) {
		*buf = mt7621_nfc_pio_read(nfc, true);
		buf++;
		len--;
	}

	while (len >= 4) {
		*(u32 *)buf = mt7621_nfc_pio_read(nfc, false);
		buf += 4;
		len -= 4;
	}

	while (len) {
		*buf = mt7621_nfc_pio_read(nfc, true);
		buf++;
		len--;
	}
}

static void mt7621_nfc_read_data_discard(struct mt7621_nfc *nfc, u32 len)
{
	while (len >= 4) {
		mt7621_nfc_pio_read(nfc, false);
		len -= 4;
	}

	while (len) {
		mt7621_nfc_pio_read(nfc, true);
		len--;
	}
}

static void mt7621_nfc_pio_write(struct mt7621_nfc *nfc, u32 val, bool bw)
{
	u32 reg, fsm;

	reg = nfi_read32(nfc, NFI_STA);
	fsm = FIELD_GET(STA_NFI_FSM, reg);

	if (fsm != STA_FSM_CUSTOM_DATA) {
		reg = nfi_read16(nfc, NFI_CNFG);
		reg &= ~(CNFG_READ_MODE | CNFG_BYTE_RW);
		if (bw)
			reg |= CNFG_BYTE_RW;
		nfi_write16(nfc, NFI_CNFG, reg);

		nfi_write16(nfc, NFI_CON, CON_NFI_SEC | CON_NFI_BWR);
		nfi_write16(nfc, NFI_STRDATA, STR_DATA);
	}

	mt7621_nfc_wait_pio_ready(nfc);
	nfi_write32(nfc, NFI_DATAW, val);
}

static void mt7621_nfc_write_data(struct mt7621_nfc *nfc, const u8 *buf,
				  u32 len)
{
	while (((uintptr_t)buf & 3) && len) {
		mt7621_nfc_pio_write(nfc, *buf, true);
		buf++;
		len--;
	}

	while (len >= 4) {
		mt7621_nfc_pio_write(nfc, *(const u32 *)buf, false);
		buf += 4;
		len -= 4;
	}

	while (len) {
		mt7621_nfc_pio_write(nfc, *buf, true);
		buf++;
		len--;
	}
}

static void mt7621_nfc_write_data_empty(struct mt7621_nfc *nfc, u32 len)
{
	while (len >= 4) {
		mt7621_nfc_pio_write(nfc, 0xffffffff, false);
		len -= 4;
	}

	while (len) {
		mt7621_nfc_pio_write(nfc, 0xff, true);
		len--;
	}
}

static void mt7621_nfc_write_byte(struct mtd_info *mtd, u8 byte)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(mtd_to_nand(mtd));

	mt7621_nfc_pio_write(nfc, byte, true);
}

static void mt7621_nfc_write_buf(struct mtd_info *mtd, const u8 *buf, int len)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(mtd_to_nand(mtd));

	return mt7621_nfc_write_data(nfc, buf, len);
}

static u8 mt7621_nfc_read_byte(struct mtd_info *mtd)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(mtd_to_nand(mtd));

	return mt7621_nfc_pio_read(nfc, true);
}

static void mt7621_nfc_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(mtd_to_nand(mtd));

	mt7621_nfc_read_data(nfc, buf, len);
}

static int mt7621_nfc_calc_ecc_strength(struct mt7621_nfc *nfc,
					u32 avail_ecc_bytes)
{
	struct nand_chip *nand = &nfc->nand;
	struct mtd_info *mtd = nand_to_mtd(nand);
	u32 strength;
	int i;

	strength = avail_ecc_bytes * 8 / ECC_PARITY_BITS;

	/* Find the closest supported ecc strength */
	for (i = ARRAY_SIZE(mt7621_ecc_strength) - 1; i >= 0; i--) {
		if (mt7621_ecc_strength[i] <= strength)
			break;
	}

	if (unlikely(i < 0)) {
		pr_err("OOB size (%u) is not supported\n", mtd->oobsize);
		return -EINVAL;
	}

	nand->ecc.strength = mt7621_ecc_strength[i];
	nand->ecc.bytes = DIV_ROUND_UP(nand->ecc.strength * ECC_PARITY_BITS, 8);

	pr_debug("ECC strength adjusted to %u bits\n", nand->ecc.strength);

	return i;
}

static int mt7621_nfc_set_spare_per_sector(struct mt7621_nfc *nfc)
{
	struct nand_chip *nand = &nfc->nand;
	struct mtd_info *mtd = nand_to_mtd(nand);
	u32 size;
	int i;

	size = nand->ecc.bytes + NFI_FDM_SIZE;

	/* Find the closest supported spare size */
	for (i = 0; i < ARRAY_SIZE(mt7621_nfi_spare_size); i++) {
		if (mt7621_nfi_spare_size[i] >= size)
			break;
	}

	if (unlikely(i >= ARRAY_SIZE(mt7621_nfi_spare_size))) {
		pr_err("OOB size (%u) is not supported\n", mtd->oobsize);
		return -EINVAL;
	}

	nfc->spare_per_sector = mt7621_nfi_spare_size[i];

	return i;
}

static int mt7621_nfc_ecc_init(struct mt7621_nfc *nfc)
{
	struct nand_chip *nand = &nfc->nand;
	struct mtd_info *mtd = nand_to_mtd(nand);
	u32 avail_ecc_bytes, encode_block_size, decode_block_size;
	u32 ecc_enccfg, ecc_deccfg;
	int ecc_cap;

	nand->ecc.options |= NAND_ECC_CUSTOM_PAGE_ACCESS;

	nand->ecc.size = ECC_SECTOR_SIZE;
	nand->ecc.steps = mtd->writesize / nand->ecc.size;

	avail_ecc_bytes = mtd->oobsize / nand->ecc.steps - NFI_FDM_SIZE;

	ecc_cap = mt7621_nfc_calc_ecc_strength(nfc, avail_ecc_bytes);
	if (ecc_cap < 0)
		return ecc_cap;

	/* Sector + FDM */
	encode_block_size = (nand->ecc.size + NFI_FDM_SIZE) * 8;
	ecc_enccfg = ecc_cap | FIELD_PREP(ENC_MODE, ENC_MODE_NFI) |
		     FIELD_PREP(ENC_CNFG_MSG, encode_block_size);

	/* Sector + FDM + ECC parity bits */
	decode_block_size = ((nand->ecc.size + NFI_FDM_SIZE) * 8) +
			    nand->ecc.strength * ECC_PARITY_BITS;
	ecc_deccfg = ecc_cap | FIELD_PREP(DEC_MODE, DEC_MODE_NFI) |
		     FIELD_PREP(DEC_CS, decode_block_size) |
		     FIELD_PREP(DEC_CON, DEC_CON_EL) | DEC_EMPTY_EN;

	mt7621_ecc_encoder_op(nfc, false);
	ecc_write32(nfc, ECC_ENCCNFG, ecc_enccfg);

	mt7621_ecc_decoder_op(nfc, false);
	ecc_write32(nfc, ECC_DECCNFG, ecc_deccfg);

	return 0;
}

static int mt7621_nfc_set_page_format(struct mt7621_nfc *nfc)
{
	struct nand_chip *nand = &nfc->nand;
	struct mtd_info *mtd = nand_to_mtd(nand);
	int i, spare_size;
	u32 pagefmt;

	spare_size = mt7621_nfc_set_spare_per_sector(nfc);
	if (spare_size < 0)
		return spare_size;

	for (i = 0; i < ARRAY_SIZE(mt7621_nfi_page_size); i++) {
		if (mt7621_nfi_page_size[i] == mtd->writesize)
			break;
	}

	if (unlikely(i >= ARRAY_SIZE(mt7621_nfi_page_size))) {
		pr_err("Page size (%u) is not supported\n", mtd->writesize);
		return -EINVAL;
	}

	pagefmt = FIELD_PREP(PAGEFMT_PAGE, i) |
		  FIELD_PREP(PAGEFMT_SPARE, spare_size) |
		  FIELD_PREP(PAGEFMT_FDM, NFI_FDM_SIZE) |
		  FIELD_PREP(PAGEFMT_FDM_ECC, NFI_FDM_SIZE);

	nfi_write16(nfc, NFI_PAGEFMT, pagefmt);

	return 0;
}

static int mt7621_nfc_attach_chip(struct nand_chip *nand)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(nand);
	int ret;

	if (nand->options & NAND_BUSWIDTH_16) {
		pr_err("16-bit buswidth is not supported");
		return -EINVAL;
	}

	ret = mt7621_nfc_ecc_init(nfc);
	if (ret)
		return ret;

	return mt7621_nfc_set_page_format(nfc);
}

static void mt7621_nfc_write_fdm(struct mt7621_nfc *nfc)
{
	struct nand_chip *nand = &nfc->nand;
	u32 vall, valm;
	u8 *oobptr;
	int i, j;

	for (i = 0; i < nand->ecc.steps; i++) {
		vall = 0;
		valm = 0;
		oobptr = oob_fdm_ptr(nand, i);

		for (j = 0; j < 4; j++)
			vall |= (u32)oobptr[j] << (j * 8);

		for (j = 0; j < 4; j++)
			valm |= (u32)oobptr[j + 4] << (j * 8);

		nfi_write32(nfc, NFI_FDML(i), vall);
		nfi_write32(nfc, NFI_FDMM(i), valm);
	}
}

static void mt7621_nfc_read_sector_fdm(struct mt7621_nfc *nfc, u32 sect)
{
	struct nand_chip *nand = &nfc->nand;
	u32 vall, valm;
	u8 *oobptr;
	int i;

	vall = nfi_read32(nfc, NFI_FDML(sect));
	valm = nfi_read32(nfc, NFI_FDMM(sect));
	oobptr = oob_fdm_ptr(nand, sect);

	for (i = 0; i < 4; i++)
		oobptr[i] = (vall >> (i * 8)) & 0xff;

	for (i = 0; i < 4; i++)
		oobptr[i + 4] = (valm >> (i * 8)) & 0xff;
}

static int mt7621_nfc_read_page_hwecc(struct mtd_info *mtd,
				      struct nand_chip *nand, uint8_t *buf,
				      int oob_required, int page)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(nand);
	int bitflips = 0, ret = 0;
	int rc, i;

	nand_read_page_op(nand, page, 0, NULL, 0);

	nfi_write16(nfc, NFI_CNFG, FIELD_PREP(CNFG_OP_MODE, CNFG_OP_CUSTOM) |
		    CNFG_READ_MODE | CNFG_AUTO_FMT_EN | CNFG_HW_ECC_EN);

	mt7621_ecc_decoder_op(nfc, true);

	nfi_write16(nfc, NFI_CON, FIELD_PREP(CON_NFI_SEC, nand->ecc.steps) |
		    CON_NFI_BRD);

	for (i = 0; i < nand->ecc.steps; i++) {
		if (buf)
			mt7621_nfc_read_data(nfc, page_data_ptr(nand, buf, i),
					     nand->ecc.size);
		else
			mt7621_nfc_read_data_discard(nfc, nand->ecc.size);

		rc = mt7621_ecc_decoder_wait_done(nfc, i);

		mt7621_nfc_read_sector_fdm(nfc, i);

		if (rc < 0) {
			ret = -EIO;
			continue;
		}

		rc = mt7621_ecc_correct_check(nfc,
			buf ? page_data_ptr(nand, buf, i) : NULL,
			oob_fdm_ptr(nand, i), i);

		if (rc < 0) {
			pr_warn("Uncorrectable ECC error at page %d step %d\n",
				page, i);
			bitflips = nand->ecc.strength + 1;
			mtd->ecc_stats.failed++;
		} else {
			if (rc > bitflips)
				bitflips = rc;
			mtd->ecc_stats.corrected += rc;
		}
	}

	mt7621_ecc_decoder_op(nfc, false);

	nfi_write16(nfc, NFI_CON, 0);

	if (ret < 0)
		return ret;

	return bitflips;
}

static int mt7621_nfc_read_page_raw(struct mtd_info *mtd,
				    struct nand_chip *nand, uint8_t *buf,
				    int oob_required, int page)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(nand);
	int i;

	nand_read_page_op(nand, page, 0, NULL, 0);

	nfi_write16(nfc, NFI_CNFG, FIELD_PREP(CNFG_OP_MODE, CNFG_OP_CUSTOM) |
		    CNFG_READ_MODE);

	nfi_write16(nfc, NFI_CON, FIELD_PREP(CON_NFI_SEC, nand->ecc.steps) |
		    CON_NFI_BRD);

	for (i = 0; i < nand->ecc.steps; i++) {
		/* Read data */
		if (buf)
			mt7621_nfc_read_data(nfc, page_data_ptr(nand, buf, i),
					     nand->ecc.size);
		else
			mt7621_nfc_read_data_discard(nfc, nand->ecc.size);

		/* Read FDM */
		mt7621_nfc_read_data(nfc, oob_fdm_ptr(nand, i), NFI_FDM_SIZE);

		/* Read ECC parity data */
		mt7621_nfc_read_data(nfc, oob_ecc_ptr(nfc, i),
				     nfc->spare_per_sector - NFI_FDM_SIZE);
	}

	nfi_write16(nfc, NFI_CON, 0);

	return 0;
}

static int mt7621_nfc_read_oob_hwecc(struct mtd_info *mtd,
				     struct nand_chip *nand, int page)
{
	return mt7621_nfc_read_page_hwecc(mtd, nand, NULL, 1, page);
}

static int mt7621_nfc_read_oob_raw(struct mtd_info *mtd,
				   struct nand_chip *nand, int page)
{
	return mt7621_nfc_read_page_raw(mtd, nand, NULL, 1, page);
}

static int mt7621_nfc_check_empty_page(struct nand_chip *nand, const u8 *buf)
{
	struct mtd_info *mtd = nand_to_mtd(nand);
	u8 *oobptr;
	u32 i, j;

	if (buf) {
		for (i = 0; i < mtd->writesize; i++)
			if (buf[i] != 0xff)
				return 0;
	}

	for (i = 0; i < nand->ecc.steps; i++) {
		oobptr = oob_fdm_ptr(nand, i);
		for (j = 0; j < NFI_FDM_SIZE; j++)
			if (oobptr[j] != 0xff)
				return 0;
	}

	return 1;
}

static int mt7621_nfc_write_page_hwecc(struct mtd_info *mtd,
				       struct nand_chip *nand,
				       const u8 *buf, int oob_required,
				       int page)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(nand);

	if (mt7621_nfc_check_empty_page(nand, buf)) {
		/*
		 * MT7621 ECC engine always generates parity code for input
		 * pages, even for empty pages. Doing so will write back ECC
		 * parity code to the oob region, which means such pages will
		 * no longer be empty pages.
		 *
		 * To avoid this, stop write operation if current page is an
		 * empty page.
		 */
		return 0;
	}

	nand_prog_page_begin_op(nand, page, 0, NULL, 0);

	nfi_write16(nfc, NFI_CNFG, FIELD_PREP(CNFG_OP_MODE, CNFG_OP_CUSTOM) |
		    CNFG_AUTO_FMT_EN | CNFG_HW_ECC_EN);

	mt7621_ecc_encoder_op(nfc, true);

	mt7621_nfc_write_fdm(nfc);

	nfi_write16(nfc, NFI_CON, FIELD_PREP(CON_NFI_SEC, nand->ecc.steps) |
		    CON_NFI_BWR);

	if (buf)
		mt7621_nfc_write_data(nfc, buf, mtd->writesize);
	else
		mt7621_nfc_write_data_empty(nfc, mtd->writesize);

	mt7621_nfc_wait_write_completion(nfc, nand);

	mt7621_ecc_encoder_op(nfc, false);

	nfi_write16(nfc, NFI_CON, 0);

	return nand_prog_page_end_op(nand);
}

static int mt7621_nfc_write_page_raw(struct mtd_info *mtd,
				     struct nand_chip *nand,
				     const u8 *buf, int oob_required,
				     int page)
{
	struct mt7621_nfc *nfc = nand_get_controller_data(nand);
	int i;

	nand_prog_page_begin_op(nand, page, 0, NULL, 0);

	nfi_write16(nfc, NFI_CNFG, FIELD_PREP(CNFG_OP_MODE, CNFG_OP_CUSTOM));

	nfi_write16(nfc, NFI_CON, FIELD_PREP(CON_NFI_SEC, nand->ecc.steps) |
		    CON_NFI_BWR);

	for (i = 0; i < nand->ecc.steps; i++) {
		/* Write data */
		if (buf)
			mt7621_nfc_write_data(nfc, page_data_ptr(nand, buf, i),
					      nand->ecc.size);
		else
			mt7621_nfc_write_data_empty(nfc, nand->ecc.size);

		/* Write FDM */
		mt7621_nfc_write_data(nfc, oob_fdm_ptr(nand, i),
				      NFI_FDM_SIZE);

		/* Write dummy ECC parity data */
		mt7621_nfc_write_data_empty(nfc, nfc->spare_per_sector -
					    NFI_FDM_SIZE);
	}

	mt7621_nfc_wait_write_completion(nfc, nand);

	nfi_write16(nfc, NFI_CON, 0);

	return nand_prog_page_end_op(nand);
}

static int mt7621_nfc_write_oob_hwecc(struct mtd_info *mtd,
				      struct nand_chip *nand, int page)
{
	return mt7621_nfc_write_page_hwecc(mtd, nand, NULL, 1, page);
}

static int mt7621_nfc_write_oob_raw(struct mtd_info *mtd,
				    struct nand_chip *nand, int page)
{
	return mt7621_nfc_write_page_raw(mtd, nand, NULL, 1, page);
}

static int mt7621_nfc_ooblayout_free(struct mtd_info *mtd, int section,
				     struct mtd_oob_region *oob_region)
{
	struct nand_chip *nand = mtd_to_nand(mtd);

	if (section >= nand->ecc.steps)
		return -ERANGE;

	oob_region->length = NFI_FDM_SIZE - 1;
	oob_region->offset = section * NFI_FDM_SIZE + 1;

	return 0;
}

static int mt7621_nfc_ooblayout_ecc(struct mtd_info *mtd, int section,
				    struct mtd_oob_region *oob_region)
{
	struct nand_chip *nand = mtd_to_nand(mtd);

	if (section)
		return -ERANGE;

	oob_region->offset = NFI_FDM_SIZE * nand->ecc.steps;
	oob_region->length = mtd->oobsize - oob_region->offset;

	return 0;
}

static const struct mtd_ooblayout_ops mt7621_nfc_ooblayout_ops = {
	.rfree = mt7621_nfc_ooblayout_free,
	.ecc = mt7621_nfc_ooblayout_ecc,
};

/*
 * This function will override the default one which is not supposed to be
 * used for ECC syndrome based pages.
 */
static int mt7621_nfc_block_bad(struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct mtd_oob_ops ops;
	int ret, i = 0;
	u16 bad;

	memset(&ops, 0, sizeof(ops));
	ops.oobbuf = (uint8_t *)&bad;
	ops.ooboffs = nand->badblockpos;
	if (nand->options & NAND_BUSWIDTH_16) {
		ops.ooboffs &= ~0x01;
		ops.ooblen = 2;
	} else {
		ops.ooblen = 1;
	}
	ops.mode = MTD_OPS_RAW;

	/* Read from first/last page(s) if necessary */
	if (nand->bbt_options & NAND_BBT_SCANLASTPAGE)
		ofs += mtd->erasesize - mtd->writesize;

	do {
		ret = mtd_read_oob(mtd, ofs, &ops);
		if (ret)
			return ret;

		if (likely(nand->badblockbits == 8))
			ret = bad != 0xFF;
		else
			ret = hweight8(bad) < nand->badblockbits;

		i++;
		ofs += mtd->writesize;
	} while (!ret && (nand->bbt_options & NAND_BBT_SCAN2NDPAGE) && i < 2);

	return ret;
}

static void mt7621_nfc_init_chip(struct mt7621_nfc *nfc)
{
	struct nand_chip *nand = &nfc->nand;
	struct mtd_info *mtd;
	int ret;

	nand_set_controller_data(nand, nfc);

	nand->options |= NAND_NO_SUBPAGE_WRITE;

	nand->ecc.mode = NAND_ECC_HW_SYNDROME;
	nand->ecc.read_page = mt7621_nfc_read_page_hwecc;
	nand->ecc.read_page_raw = mt7621_nfc_read_page_raw;
	nand->ecc.write_page = mt7621_nfc_write_page_hwecc;
	nand->ecc.write_page_raw = mt7621_nfc_write_page_raw;
	nand->ecc.read_oob = mt7621_nfc_read_oob_hwecc;
	nand->ecc.read_oob_raw = mt7621_nfc_read_oob_raw;
	nand->ecc.write_oob = mt7621_nfc_write_oob_hwecc;
	nand->ecc.write_oob_raw = mt7621_nfc_write_oob_raw;

	nand->dev_ready = mt7621_nfc_dev_ready;
	nand->select_chip = mt7621_nfc_select_chip;
	nand->write_byte = mt7621_nfc_write_byte;
	nand->write_buf = mt7621_nfc_write_buf;
	nand->read_byte = mt7621_nfc_read_byte;
	nand->read_buf = mt7621_nfc_read_buf;
	nand->cmd_ctrl = mt7621_nfc_cmd_ctrl;
	nand->block_bad = mt7621_nfc_block_bad;

	mtd = nand_to_mtd(nand);
	mtd_set_ooblayout(mtd, &mt7621_nfc_ooblayout_ops);

	/* Reset NFI master */
	mt7621_nfc_hw_init(nfc);

	ret = nand_scan_ident(mtd, 1, NULL);
	if (ret)
		return;

	mt7621_nfc_attach_chip(nand);

	ret = nand_scan_tail(mtd);
	if (ret)
		return;

	nand_register(0, mtd);
}

static void mt7621_nfc_set_regs(struct mt7621_nfc *nfc)
{
	nfc->nfi_regs = (void __iomem *)CKSEG1ADDR(NFI_BASE);
	nfc->ecc_regs = (void __iomem *)CKSEG1ADDR(NFI_ECC_BASE);
}

void mt7621_nfc_spl_init(struct mt7621_nfc *nfc)
{
	struct nand_chip *nand = &nfc->nand;

	mt7621_nfc_set_regs(nfc);

	nand_set_controller_data(nand, nfc);

	nand->options |= NAND_NO_SUBPAGE_WRITE;

	nand->ecc.mode = NAND_ECC_HW_SYNDROME;
	nand->ecc.read_page = mt7621_nfc_read_page_hwecc;

	nand->dev_ready = mt7621_nfc_dev_ready;
	nand->select_chip = mt7621_nfc_select_chip;
	nand->read_byte = mt7621_nfc_read_byte;
	nand->read_buf = mt7621_nfc_read_buf;
	nand->cmd_ctrl = mt7621_nfc_cmd_ctrl;

	/* Reset NFI master */
	mt7621_nfc_hw_init(nfc);
}

int mt7621_nfc_spl_post_init(struct mt7621_nfc *nfc)
{
	struct nand_chip *nand = &nfc->nand;
	int nand_maf_id, nand_dev_id;
	int ret;

	ret = nand_detect(nand, &nand_maf_id, &nand_dev_id, NULL);

	if (ret)
		return ret;

	nand->numchips = 1;
	nand->mtd.size = nand->chipsize;

	return mt7621_nfc_attach_chip(nand);
}

void board_nand_init(void)
{
	mt7621_nfc_set_regs(&nfc_dev);
	mt7621_nfc_init_chip(&nfc_dev);
}
