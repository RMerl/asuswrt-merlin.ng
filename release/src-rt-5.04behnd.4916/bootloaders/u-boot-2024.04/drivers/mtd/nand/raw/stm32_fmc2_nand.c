// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) STMicroelectronics 2019
 * Author: Christophe Kerello <christophe.kerello@st.com>
 */

#define LOG_CATEGORY UCLASS_MTD

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <nand.h>
#include <reset.h>
#include <asm/gpio.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <linux/ioport.h>
#include <linux/mtd/rawnand.h>
#include <linux/printk.h>
#include <linux/time.h>

/* Bad block marker length */
#define FMC2_BBM_LEN			2

/* ECC step size */
#define FMC2_ECC_STEP_SIZE		512

/* Command delay */
#define FMC2_RB_DELAY_US		30

/* Max chip enable */
#define FMC2_MAX_CE			2

/* Timings */
#define FMC2_THIZ			1
#define FMC2_TIO			8000
#define FMC2_TSYNC			3000
#define FMC2_PCR_TIMING_MASK		0xf
#define FMC2_PMEM_PATT_TIMING_MASK	0xff

/* FMC2 Controller Registers */
#define FMC2_BCR1			0x0
#define FMC2_PCR			0x80
#define FMC2_SR				0x84
#define FMC2_PMEM			0x88
#define FMC2_PATT			0x8c
#define FMC2_HECCR			0x94
#define FMC2_BCHISR			0x254
#define FMC2_BCHICR			0x258
#define FMC2_BCHPBR1			0x260
#define FMC2_BCHPBR2			0x264
#define FMC2_BCHPBR3			0x268
#define FMC2_BCHPBR4			0x26c
#define FMC2_BCHDSR0			0x27c
#define FMC2_BCHDSR1			0x280
#define FMC2_BCHDSR2			0x284
#define FMC2_BCHDSR3			0x288
#define FMC2_BCHDSR4			0x28c

/* Register: FMC2_BCR1 */
#define FMC2_BCR1_FMC2EN		BIT(31)

/* Register: FMC2_PCR */
#define FMC2_PCR_PWAITEN		BIT(1)
#define FMC2_PCR_PBKEN			BIT(2)
#define FMC2_PCR_PWID			GENMASK(5, 4)
#define FMC2_PCR_PWID_BUSWIDTH_8	0
#define FMC2_PCR_PWID_BUSWIDTH_16	1
#define FMC2_PCR_ECCEN			BIT(6)
#define FMC2_PCR_ECCALG			BIT(8)
#define FMC2_PCR_TCLR			GENMASK(12, 9)
#define FMC2_PCR_TCLR_DEFAULT		0xf
#define FMC2_PCR_TAR			GENMASK(16, 13)
#define FMC2_PCR_TAR_DEFAULT		0xf
#define FMC2_PCR_ECCSS			GENMASK(19, 17)
#define FMC2_PCR_ECCSS_512		1
#define FMC2_PCR_ECCSS_2048		3
#define FMC2_PCR_BCHECC			BIT(24)
#define FMC2_PCR_WEN			BIT(25)

/* Register: FMC2_SR */
#define FMC2_SR_NWRF			BIT(6)

/* Register: FMC2_PMEM */
#define FMC2_PMEM_MEMSET		GENMASK(7, 0)
#define FMC2_PMEM_MEMWAIT		GENMASK(15, 8)
#define FMC2_PMEM_MEMHOLD		GENMASK(23, 16)
#define FMC2_PMEM_MEMHIZ		GENMASK(31, 24)
#define FMC2_PMEM_DEFAULT		0x0a0a0a0a

/* Register: FMC2_PATT */
#define FMC2_PATT_ATTSET		GENMASK(7, 0)
#define FMC2_PATT_ATTWAIT		GENMASK(15, 8)
#define FMC2_PATT_ATTHOLD		GENMASK(23, 16)
#define FMC2_PATT_ATTHIZ		GENMASK(31, 24)
#define FMC2_PATT_DEFAULT		0x0a0a0a0a

/* Register: FMC2_BCHISR */
#define FMC2_BCHISR_DERF		BIT(1)
#define FMC2_BCHISR_EPBRF		BIT(4)

/* Register: FMC2_BCHICR */
#define FMC2_BCHICR_CLEAR_IRQ		GENMASK(4, 0)

/* Register: FMC2_BCHDSR0 */
#define FMC2_BCHDSR0_DUE		BIT(0)
#define FMC2_BCHDSR0_DEF		BIT(1)
#define FMC2_BCHDSR0_DEN		GENMASK(7, 4)

/* Register: FMC2_BCHDSR1 */
#define FMC2_BCHDSR1_EBP1		GENMASK(12, 0)
#define FMC2_BCHDSR1_EBP2		GENMASK(28, 16)

/* Register: FMC2_BCHDSR2 */
#define FMC2_BCHDSR2_EBP3		GENMASK(12, 0)
#define FMC2_BCHDSR2_EBP4		GENMASK(28, 16)

/* Register: FMC2_BCHDSR3 */
#define FMC2_BCHDSR3_EBP5		GENMASK(12, 0)
#define FMC2_BCHDSR3_EBP6		GENMASK(28, 16)

/* Register: FMC2_BCHDSR4 */
#define FMC2_BCHDSR4_EBP7		GENMASK(12, 0)
#define FMC2_BCHDSR4_EBP8		GENMASK(28, 16)

#define FMC2_TIMEOUT_5S			5000000

enum stm32_fmc2_ecc {
	FMC2_ECC_HAM = 1,
	FMC2_ECC_BCH4 = 4,
	FMC2_ECC_BCH8 = 8
};

struct stm32_fmc2_timings {
	u8 tclr;
	u8 tar;
	u8 thiz;
	u8 twait;
	u8 thold_mem;
	u8 tset_mem;
	u8 thold_att;
	u8 tset_att;
};

struct stm32_fmc2_nand {
	struct nand_chip chip;
	struct stm32_fmc2_timings timings;
	struct gpio_desc wp_gpio;
	int ncs;
	int cs_used[FMC2_MAX_CE];
};

static inline struct stm32_fmc2_nand *to_fmc2_nand(struct nand_chip *chip)
{
	return container_of(chip, struct stm32_fmc2_nand, chip);
}

struct stm32_fmc2_nfc {
	struct nand_hw_control base;
	struct stm32_fmc2_nand nand;
	struct nand_ecclayout ecclayout;
	fdt_addr_t io_base;
	fdt_addr_t data_base[FMC2_MAX_CE];
	fdt_addr_t cmd_base[FMC2_MAX_CE];
	fdt_addr_t addr_base[FMC2_MAX_CE];
	struct clk clk;

	u8 cs_assigned;
	int cs_sel;
};

static inline struct stm32_fmc2_nfc *to_stm32_nfc(struct nand_hw_control *base)
{
	return container_of(base, struct stm32_fmc2_nfc, base);
}

static void stm32_fmc2_nfc_timings_init(struct nand_chip *chip)
{
	struct stm32_fmc2_nfc *nfc = to_stm32_nfc(chip->controller);
	struct stm32_fmc2_nand *nand = to_fmc2_nand(chip);
	struct stm32_fmc2_timings *timings = &nand->timings;
	u32 pmem, patt;

	/* Set tclr/tar timings */
	clrsetbits_le32(nfc->io_base + FMC2_PCR,
			FMC2_PCR_TCLR | FMC2_PCR_TAR,
			FIELD_PREP(FMC2_PCR_TCLR, timings->tclr) |
			FIELD_PREP(FMC2_PCR_TAR, timings->tar));

	/* Set tset/twait/thold/thiz timings in common bank */
	pmem = FIELD_PREP(FMC2_PMEM_MEMSET, timings->tset_mem);
	pmem |= FIELD_PREP(FMC2_PMEM_MEMWAIT, timings->twait);
	pmem |= FIELD_PREP(FMC2_PMEM_MEMHOLD, timings->thold_mem);
	pmem |= FIELD_PREP(FMC2_PMEM_MEMHIZ, timings->thiz);
	writel(pmem, nfc->io_base + FMC2_PMEM);

	/* Set tset/twait/thold/thiz timings in attribut bank */
	patt = FIELD_PREP(FMC2_PATT_ATTSET, timings->tset_att);
	patt |= FIELD_PREP(FMC2_PATT_ATTWAIT, timings->twait);
	patt |= FIELD_PREP(FMC2_PATT_ATTHOLD, timings->thold_att);
	patt |= FIELD_PREP(FMC2_PATT_ATTHIZ, timings->thiz);
	writel(patt, nfc->io_base + FMC2_PATT);
}

static void stm32_fmc2_nfc_setup(struct nand_chip *chip)
{
	struct stm32_fmc2_nfc *nfc = to_stm32_nfc(chip->controller);
	u32 pcr = 0, pcr_mask;

	/* Configure ECC algorithm (default configuration is Hamming) */
	pcr_mask = FMC2_PCR_ECCALG;
	pcr_mask |= FMC2_PCR_BCHECC;
	if (chip->ecc.strength == FMC2_ECC_BCH8) {
		pcr |= FMC2_PCR_ECCALG;
		pcr |= FMC2_PCR_BCHECC;
	} else if (chip->ecc.strength == FMC2_ECC_BCH4) {
		pcr |= FMC2_PCR_ECCALG;
	}

	/* Set buswidth */
	pcr_mask |= FMC2_PCR_PWID;
	if (chip->options & NAND_BUSWIDTH_16)
		pcr |= FIELD_PREP(FMC2_PCR_PWID, FMC2_PCR_PWID_BUSWIDTH_16);

	/* Set ECC sector size */
	pcr_mask |= FMC2_PCR_ECCSS;
	pcr |= FIELD_PREP(FMC2_PCR_ECCSS, FMC2_PCR_ECCSS_512);

	clrsetbits_le32(nfc->io_base + FMC2_PCR, pcr_mask, pcr);
}

static void stm32_fmc2_nfc_select_chip(struct mtd_info *mtd, int chipnr)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct stm32_fmc2_nfc *nfc = to_stm32_nfc(chip->controller);
	struct stm32_fmc2_nand *nand = to_fmc2_nand(chip);

	if (chipnr < 0 || chipnr >= nand->ncs)
		return;

	if (nand->cs_used[chipnr] == nfc->cs_sel)
		return;

	nfc->cs_sel = nand->cs_used[chipnr];
	chip->IO_ADDR_R = (void __iomem *)nfc->data_base[nfc->cs_sel];
	chip->IO_ADDR_W = (void __iomem *)nfc->data_base[nfc->cs_sel];

	stm32_fmc2_nfc_setup(chip);
	stm32_fmc2_nfc_timings_init(chip);
}

static void stm32_fmc2_nfc_set_buswidth_16(struct stm32_fmc2_nfc *nfc,
					   bool set)
{
	u32 pcr;

	pcr = set ? FIELD_PREP(FMC2_PCR_PWID, FMC2_PCR_PWID_BUSWIDTH_16) :
		    FIELD_PREP(FMC2_PCR_PWID, FMC2_PCR_PWID_BUSWIDTH_8);

	clrsetbits_le32(nfc->io_base + FMC2_PCR, FMC2_PCR_PWID, pcr);
}

static void stm32_fmc2_nfc_set_ecc(struct stm32_fmc2_nfc *nfc, bool enable)
{
	clrsetbits_le32(nfc->io_base + FMC2_PCR, FMC2_PCR_ECCEN,
			enable ? FMC2_PCR_ECCEN : 0);
}

static void stm32_fmc2_nfc_clear_bch_irq(struct stm32_fmc2_nfc *nfc)
{
	writel(FMC2_BCHICR_CLEAR_IRQ, nfc->io_base + FMC2_BCHICR);
}

static void stm32_fmc2_nfc_cmd_ctrl(struct mtd_info *mtd, int cmd,
				    unsigned int ctrl)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct stm32_fmc2_nfc *nfc = to_stm32_nfc(chip->controller);

	if (cmd == NAND_CMD_NONE)
		return;

	if (ctrl & NAND_CLE) {
		writeb(cmd, nfc->cmd_base[nfc->cs_sel]);
		return;
	}

	writeb(cmd, nfc->addr_base[nfc->cs_sel]);
}

/*
 * Enable ECC logic and reset syndrome/parity bits previously calculated
 * Syndrome/parity bits is cleared by setting the ECCEN bit to 0
 */
static void stm32_fmc2_nfc_hwctl(struct mtd_info *mtd, int mode)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct stm32_fmc2_nfc *nfc = to_stm32_nfc(chip->controller);

	stm32_fmc2_nfc_set_ecc(nfc, false);

	if (chip->ecc.strength != FMC2_ECC_HAM) {
		clrsetbits_le32(nfc->io_base + FMC2_PCR, FMC2_PCR_WEN,
				mode == NAND_ECC_WRITE ? FMC2_PCR_WEN : 0);

		stm32_fmc2_nfc_clear_bch_irq(nfc);
	}

	stm32_fmc2_nfc_set_ecc(nfc, true);
}

/*
 * ECC Hamming calculation
 * ECC is 3 bytes for 512 bytes of data (supports error correction up to
 * max of 1-bit)
 */
static int stm32_fmc2_nfc_ham_calculate(struct mtd_info *mtd, const u8 *data,
					u8 *ecc)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct stm32_fmc2_nfc *nfc = to_stm32_nfc(chip->controller);
	u32 heccr, sr;
	int ret;

	ret = readl_poll_timeout(nfc->io_base + FMC2_SR, sr,
				 sr & FMC2_SR_NWRF, FMC2_TIMEOUT_5S);
	if (ret < 0) {
		log_err("Ham timeout\n");
		return ret;
	}

	heccr = readl(nfc->io_base + FMC2_HECCR);

	ecc[0] = heccr;
	ecc[1] = heccr >> 8;
	ecc[2] = heccr >> 16;

	stm32_fmc2_nfc_set_ecc(nfc, false);

	return 0;
}

static int stm32_fmc2_nfc_ham_correct(struct mtd_info *mtd, u8 *dat,
				      u8 *read_ecc, u8 *calc_ecc)
{
	u8 bit_position = 0, b0, b1, b2;
	u32 byte_addr = 0, b;
	u32 i, shifting = 1;

	/* Indicate which bit and byte is faulty (if any) */
	b0 = read_ecc[0] ^ calc_ecc[0];
	b1 = read_ecc[1] ^ calc_ecc[1];
	b2 = read_ecc[2] ^ calc_ecc[2];
	b = b0 | (b1 << 8) | (b2 << 16);

	/* No errors */
	if (likely(!b))
		return 0;

	/* Calculate bit position */
	for (i = 0; i < 3; i++) {
		switch (b % 4) {
		case 2:
			bit_position += shifting;
		case 1:
			break;
		default:
			return -EBADMSG;
		}
		shifting <<= 1;
		b >>= 2;
	}

	/* Calculate byte position */
	shifting = 1;
	for (i = 0; i < 9; i++) {
		switch (b % 4) {
		case 2:
			byte_addr += shifting;
		case 1:
			break;
		default:
			return -EBADMSG;
		}
		shifting <<= 1;
		b >>= 2;
	}

	/* Flip the bit */
	dat[byte_addr] ^= (1 << bit_position);

	return 1;
}

/*
 * ECC BCH calculation and correction
 * ECC is 7/13 bytes for 512 bytes of data (supports error correction up to
 * max of 4-bit/8-bit)
 */

static int stm32_fmc2_nfc_bch_calculate(struct mtd_info *mtd, const u8 *data,
					u8 *ecc)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct stm32_fmc2_nfc *nfc = to_stm32_nfc(chip->controller);
	u32 bchpbr, bchisr;
	int ret;

	/* Wait until the BCH code is ready */
	ret = readl_poll_timeout(nfc->io_base + FMC2_BCHISR, bchisr,
				 bchisr & FMC2_BCHISR_EPBRF, FMC2_TIMEOUT_5S);
	if (ret < 0) {
		log_err("Bch timeout\n");
		return ret;
	}

	/* Read parity bits */
	bchpbr = readl(nfc->io_base + FMC2_BCHPBR1);
	ecc[0] = bchpbr;
	ecc[1] = bchpbr >> 8;
	ecc[2] = bchpbr >> 16;
	ecc[3] = bchpbr >> 24;

	bchpbr = readl(nfc->io_base + FMC2_BCHPBR2);
	ecc[4] = bchpbr;
	ecc[5] = bchpbr >> 8;
	ecc[6] = bchpbr >> 16;

	if (chip->ecc.strength == FMC2_ECC_BCH8) {
		ecc[7] = bchpbr >> 24;

		bchpbr = readl(nfc->io_base + FMC2_BCHPBR3);
		ecc[8] = bchpbr;
		ecc[9] = bchpbr >> 8;
		ecc[10] = bchpbr >> 16;
		ecc[11] = bchpbr >> 24;

		bchpbr = readl(nfc->io_base + FMC2_BCHPBR4);
		ecc[12] = bchpbr;
	}

	stm32_fmc2_nfc_set_ecc(nfc, false);

	return 0;
}

static int stm32_fmc2_nfc_bch_correct(struct mtd_info *mtd, u8 *dat,
				      u8 *read_ecc, u8 *calc_ecc)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct stm32_fmc2_nfc *nfc = to_stm32_nfc(chip->controller);
	u32 bchdsr0, bchdsr1, bchdsr2, bchdsr3, bchdsr4, bchisr;
	u16 pos[8];
	int i, ret, den, eccsize = chip->ecc.size;
	unsigned int nb_errs = 0;

	/* Wait until the decoding error is ready */
	ret = readl_poll_timeout(nfc->io_base + FMC2_BCHISR, bchisr,
				 bchisr & FMC2_BCHISR_DERF, FMC2_TIMEOUT_5S);
	if (ret < 0) {
		log_err("Bch timeout\n");
		return ret;
	}

	bchdsr0 = readl(nfc->io_base + FMC2_BCHDSR0);
	bchdsr1 = readl(nfc->io_base + FMC2_BCHDSR1);
	bchdsr2 = readl(nfc->io_base + FMC2_BCHDSR2);
	bchdsr3 = readl(nfc->io_base + FMC2_BCHDSR3);
	bchdsr4 = readl(nfc->io_base + FMC2_BCHDSR4);

	stm32_fmc2_nfc_set_ecc(nfc, false);

	/* No errors found */
	if (likely(!(bchdsr0 & FMC2_BCHDSR0_DEF)))
		return 0;

	/* Too many errors detected */
	if (unlikely(bchdsr0 & FMC2_BCHDSR0_DUE))
		return -EBADMSG;

	pos[0] = FIELD_GET(FMC2_BCHDSR1_EBP1, bchdsr1);
	pos[1] = FIELD_GET(FMC2_BCHDSR1_EBP2, bchdsr1);
	pos[2] = FIELD_GET(FMC2_BCHDSR2_EBP3, bchdsr2);
	pos[3] = FIELD_GET(FMC2_BCHDSR2_EBP4, bchdsr2);
	pos[4] = FIELD_GET(FMC2_BCHDSR3_EBP5, bchdsr3);
	pos[5] = FIELD_GET(FMC2_BCHDSR3_EBP6, bchdsr3);
	pos[6] = FIELD_GET(FMC2_BCHDSR4_EBP7, bchdsr4);
	pos[7] = FIELD_GET(FMC2_BCHDSR4_EBP8, bchdsr4);

	den = FIELD_GET(FMC2_BCHDSR0_DEN, bchdsr0);
	for (i = 0; i < den; i++) {
		if (pos[i] < eccsize * 8) {
			__change_bit(pos[i], (unsigned long *)dat);
			nb_errs++;
		}
	}

	return nb_errs;
}

static int stm32_fmc2_nfc_read_page(struct mtd_info *mtd,
				    struct nand_chip *chip, u8 *buf,
				    int oob_required, int page)
{
	int i, s, stat, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	int eccstrength = chip->ecc.strength;
	u8 *p = buf;
	u8 *ecc_calc = chip->buffers->ecccalc;
	u8 *ecc_code = chip->buffers->ecccode;
	unsigned int max_bitflips = 0;

	for (i = mtd->writesize + FMC2_BBM_LEN, s = 0; s < eccsteps;
	     s++, i += eccbytes, p += eccsize) {
		chip->ecc.hwctl(mtd, NAND_ECC_READ);

		/* Read the nand page sector (512 bytes) */
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, s * eccsize, -1);
		chip->read_buf(mtd, p, eccsize);

		/* Read the corresponding ECC bytes */
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, i, -1);
		chip->read_buf(mtd, ecc_code, eccbytes);

		/* Correct the data */
		stat = chip->ecc.correct(mtd, p, ecc_code, ecc_calc);
		if (stat == -EBADMSG)
			/* Check for empty pages with bitflips */
			stat = nand_check_erased_ecc_chunk(p, eccsize,
							   ecc_code, eccbytes,
							   NULL, 0,
							   eccstrength);

		if (stat < 0) {
			mtd->ecc_stats.failed++;
		} else {
			mtd->ecc_stats.corrected += stat;
			max_bitflips = max_t(unsigned int, max_bitflips, stat);
		}
	}

	/* Read oob */
	if (oob_required) {
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, mtd->writesize, -1);
		chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);
	}

	return max_bitflips;
}

static void stm32_fmc2_nfc_init(struct stm32_fmc2_nfc *nfc, bool has_parent)
{
	u32 pcr = readl(nfc->io_base + FMC2_PCR);

	/* Set CS used to undefined */
	nfc->cs_sel = -1;

	/* Enable wait feature and nand flash memory bank */
	pcr |= FMC2_PCR_PWAITEN;
	pcr |= FMC2_PCR_PBKEN;

	/* Set buswidth to 8 bits mode for identification */
	pcr &= ~FMC2_PCR_PWID;

	/* ECC logic is disabled */
	pcr &= ~FMC2_PCR_ECCEN;

	/* Default mode */
	pcr &= ~FMC2_PCR_ECCALG;
	pcr &= ~FMC2_PCR_BCHECC;
	pcr &= ~FMC2_PCR_WEN;

	/* Set default ECC sector size */
	pcr &= ~FMC2_PCR_ECCSS;
	pcr |= FIELD_PREP(FMC2_PCR_ECCSS, FMC2_PCR_ECCSS_2048);

	/* Set default tclr/tar timings */
	pcr &= ~FMC2_PCR_TCLR;
	pcr |= FIELD_PREP(FMC2_PCR_TCLR, FMC2_PCR_TCLR_DEFAULT);
	pcr &= ~FMC2_PCR_TAR;
	pcr |= FIELD_PREP(FMC2_PCR_TAR, FMC2_PCR_TAR_DEFAULT);

	/* Enable FMC2 controller */
	if (!has_parent)
		setbits_le32(nfc->io_base + FMC2_BCR1, FMC2_BCR1_FMC2EN);

	writel(pcr, nfc->io_base + FMC2_PCR);
	writel(FMC2_PMEM_DEFAULT, nfc->io_base + FMC2_PMEM);
	writel(FMC2_PATT_DEFAULT, nfc->io_base + FMC2_PATT);
}

static void stm32_fmc2_nfc_calc_timings(struct nand_chip *chip,
					const struct nand_sdr_timings *sdrt)
{
	struct stm32_fmc2_nfc *nfc = to_stm32_nfc(chip->controller);
	struct stm32_fmc2_nand *nand = to_fmc2_nand(chip);
	struct stm32_fmc2_timings *tims = &nand->timings;
	unsigned long hclk = clk_get_rate(&nfc->clk);
	unsigned long hclkp = NSEC_PER_SEC / (hclk / 1000);
	unsigned long timing, tar, tclr, thiz, twait;
	unsigned long tset_mem, tset_att, thold_mem, thold_att;

	tar = max_t(unsigned long, hclkp, sdrt->tAR_min);
	timing = DIV_ROUND_UP(tar, hclkp) - 1;
	tims->tar = min_t(unsigned long, timing, FMC2_PCR_TIMING_MASK);

	tclr = max_t(unsigned long, hclkp, sdrt->tCLR_min);
	timing = DIV_ROUND_UP(tclr, hclkp) - 1;
	tims->tclr = min_t(unsigned long, timing, FMC2_PCR_TIMING_MASK);

	tims->thiz = FMC2_THIZ;
	thiz = (tims->thiz + 1) * hclkp;

	/*
	 * tWAIT > tRP
	 * tWAIT > tWP
	 * tWAIT > tREA + tIO
	 */
	twait = max_t(unsigned long, hclkp, sdrt->tRP_min);
	twait = max_t(unsigned long, twait, sdrt->tWP_min);
	twait = max_t(unsigned long, twait, sdrt->tREA_max + FMC2_TIO);
	timing = DIV_ROUND_UP(twait, hclkp);
	tims->twait = clamp_val(timing, 1, FMC2_PMEM_PATT_TIMING_MASK);

	/*
	 * tSETUP_MEM > tCS - tWAIT
	 * tSETUP_MEM > tALS - tWAIT
	 * tSETUP_MEM > tDS - (tWAIT - tHIZ)
	 */
	tset_mem = hclkp;
	if (sdrt->tCS_min > twait && (tset_mem < sdrt->tCS_min - twait))
		tset_mem = sdrt->tCS_min - twait;
	if (sdrt->tALS_min > twait && (tset_mem < sdrt->tALS_min - twait))
		tset_mem = sdrt->tALS_min - twait;
	if (twait > thiz && (sdrt->tDS_min > twait - thiz) &&
	    (tset_mem < sdrt->tDS_min - (twait - thiz)))
		tset_mem = sdrt->tDS_min - (twait - thiz);
	timing = DIV_ROUND_UP(tset_mem, hclkp);
	tims->tset_mem = clamp_val(timing, 1, FMC2_PMEM_PATT_TIMING_MASK);

	/*
	 * tHOLD_MEM > tCH
	 * tHOLD_MEM > tREH - tSETUP_MEM
	 * tHOLD_MEM > max(tRC, tWC) - (tSETUP_MEM + tWAIT)
	 */
	thold_mem = max_t(unsigned long, hclkp, sdrt->tCH_min);
	if (sdrt->tREH_min > tset_mem &&
	    (thold_mem < sdrt->tREH_min - tset_mem))
		thold_mem = sdrt->tREH_min - tset_mem;
	if ((sdrt->tRC_min > tset_mem + twait) &&
	    (thold_mem < sdrt->tRC_min - (tset_mem + twait)))
		thold_mem = sdrt->tRC_min - (tset_mem + twait);
	if ((sdrt->tWC_min > tset_mem + twait) &&
	    (thold_mem < sdrt->tWC_min - (tset_mem + twait)))
		thold_mem = sdrt->tWC_min - (tset_mem + twait);
	timing = DIV_ROUND_UP(thold_mem, hclkp);
	tims->thold_mem = clamp_val(timing, 1, FMC2_PMEM_PATT_TIMING_MASK);

	/*
	 * tSETUP_ATT > tCS - tWAIT
	 * tSETUP_ATT > tCLS - tWAIT
	 * tSETUP_ATT > tALS - tWAIT
	 * tSETUP_ATT > tRHW - tHOLD_MEM
	 * tSETUP_ATT > tDS - (tWAIT - tHIZ)
	 */
	tset_att = hclkp;
	if (sdrt->tCS_min > twait && (tset_att < sdrt->tCS_min - twait))
		tset_att = sdrt->tCS_min - twait;
	if (sdrt->tCLS_min > twait && (tset_att < sdrt->tCLS_min - twait))
		tset_att = sdrt->tCLS_min - twait;
	if (sdrt->tALS_min > twait && (tset_att < sdrt->tALS_min - twait))
		tset_att = sdrt->tALS_min - twait;
	if (sdrt->tRHW_min > thold_mem &&
	    (tset_att < sdrt->tRHW_min - thold_mem))
		tset_att = sdrt->tRHW_min - thold_mem;
	if (twait > thiz && (sdrt->tDS_min > twait - thiz) &&
	    (tset_att < sdrt->tDS_min - (twait - thiz)))
		tset_att = sdrt->tDS_min - (twait - thiz);
	timing = DIV_ROUND_UP(tset_att, hclkp);
	tims->tset_att = clamp_val(timing, 1, FMC2_PMEM_PATT_TIMING_MASK);

	/*
	 * tHOLD_ATT > tALH
	 * tHOLD_ATT > tCH
	 * tHOLD_ATT > tCLH
	 * tHOLD_ATT > tCOH
	 * tHOLD_ATT > tDH
	 * tHOLD_ATT > tWB + tIO + tSYNC - tSETUP_MEM
	 * tHOLD_ATT > tADL - tSETUP_MEM
	 * tHOLD_ATT > tWH - tSETUP_MEM
	 * tHOLD_ATT > tWHR - tSETUP_MEM
	 * tHOLD_ATT > tRC - (tSETUP_ATT + tWAIT)
	 * tHOLD_ATT > tWC - (tSETUP_ATT + tWAIT)
	 */
	thold_att = max_t(unsigned long, hclkp, sdrt->tALH_min);
	thold_att = max_t(unsigned long, thold_att, sdrt->tCH_min);
	thold_att = max_t(unsigned long, thold_att, sdrt->tCLH_min);
	thold_att = max_t(unsigned long, thold_att, sdrt->tCOH_min);
	thold_att = max_t(unsigned long, thold_att, sdrt->tDH_min);
	if ((sdrt->tWB_max + FMC2_TIO + FMC2_TSYNC > tset_mem) &&
	    (thold_att < sdrt->tWB_max + FMC2_TIO + FMC2_TSYNC - tset_mem))
		thold_att = sdrt->tWB_max + FMC2_TIO + FMC2_TSYNC - tset_mem;
	if (sdrt->tADL_min > tset_mem &&
	    (thold_att < sdrt->tADL_min - tset_mem))
		thold_att = sdrt->tADL_min - tset_mem;
	if (sdrt->tWH_min > tset_mem &&
	    (thold_att < sdrt->tWH_min - tset_mem))
		thold_att = sdrt->tWH_min - tset_mem;
	if (sdrt->tWHR_min > tset_mem &&
	    (thold_att < sdrt->tWHR_min - tset_mem))
		thold_att = sdrt->tWHR_min - tset_mem;
	if ((sdrt->tRC_min > tset_att + twait) &&
	    (thold_att < sdrt->tRC_min - (tset_att + twait)))
		thold_att = sdrt->tRC_min - (tset_att + twait);
	if ((sdrt->tWC_min > tset_att + twait) &&
	    (thold_att < sdrt->tWC_min - (tset_att + twait)))
		thold_att = sdrt->tWC_min - (tset_att + twait);
	timing = DIV_ROUND_UP(thold_att, hclkp);
	tims->thold_att = clamp_val(timing, 1, FMC2_PMEM_PATT_TIMING_MASK);
}

static int stm32_fmc2_nfc_setup_interface(struct mtd_info *mtd, int chipnr,
					  const struct nand_data_interface *cf)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	const struct nand_sdr_timings *sdrt;

	sdrt = nand_get_sdr_timings(cf);
	if (IS_ERR(sdrt))
		return PTR_ERR(sdrt);

	if (sdrt->tRC_min < 30000)
		return -EOPNOTSUPP;

	if (chipnr == NAND_DATA_IFACE_CHECK_ONLY)
		return 0;

	stm32_fmc2_nfc_calc_timings(chip, sdrt);
	stm32_fmc2_nfc_timings_init(chip);

	return 0;
}

static void stm32_fmc2_nfc_nand_callbacks_setup(struct nand_chip *chip)
{
	chip->ecc.hwctl = stm32_fmc2_nfc_hwctl;

	/*
	 * Specific callbacks to read/write a page depending on
	 * the algo used (Hamming, BCH).
	 */
	if (chip->ecc.strength == FMC2_ECC_HAM) {
		/* Hamming is used */
		chip->ecc.calculate = stm32_fmc2_nfc_ham_calculate;
		chip->ecc.correct = stm32_fmc2_nfc_ham_correct;
		chip->ecc.bytes = chip->options & NAND_BUSWIDTH_16 ? 4 : 3;
		chip->ecc.options |= NAND_ECC_GENERIC_ERASED_CHECK;
		return;
	}

	/* BCH is used */
	chip->ecc.read_page = stm32_fmc2_nfc_read_page;
	chip->ecc.calculate = stm32_fmc2_nfc_bch_calculate;
	chip->ecc.correct = stm32_fmc2_nfc_bch_correct;

	if (chip->ecc.strength == FMC2_ECC_BCH8)
		chip->ecc.bytes = chip->options & NAND_BUSWIDTH_16 ? 14 : 13;
	else
		chip->ecc.bytes = chip->options & NAND_BUSWIDTH_16 ? 8 : 7;
}

static int stm32_fmc2_nfc_calc_ecc_bytes(int step_size, int strength)
{
	/* Hamming */
	if (strength == FMC2_ECC_HAM)
		return 4;

	/* BCH8 */
	if (strength == FMC2_ECC_BCH8)
		return 14;

	/* BCH4 */
	return 8;
}

NAND_ECC_CAPS_SINGLE(stm32_fmc2_nfc_ecc_caps, stm32_fmc2_nfc_calc_ecc_bytes,
		     FMC2_ECC_STEP_SIZE,
		     FMC2_ECC_HAM, FMC2_ECC_BCH4, FMC2_ECC_BCH8);

static int stm32_fmc2_nfc_parse_child(struct stm32_fmc2_nfc *nfc, ofnode node)
{
	struct stm32_fmc2_nand *nand = &nfc->nand;
	u32 cs[FMC2_MAX_CE];
	int ret, i;

	if (!ofnode_get_property(node, "reg", &nand->ncs))
		return -EINVAL;

	nand->ncs /= sizeof(u32);
	if (!nand->ncs) {
		log_err("Invalid reg property size\n");
		return -EINVAL;
	}

	ret = ofnode_read_u32_array(node, "reg", cs, nand->ncs);
	if (ret < 0) {
		log_err("Could not retrieve reg property\n");
		return -EINVAL;
	}

	for (i = 0; i < nand->ncs; i++) {
		if (cs[i] >= FMC2_MAX_CE) {
			log_err("Invalid reg value: %d\n", nand->cs_used[i]);
			return -EINVAL;
		}

		if (nfc->cs_assigned & BIT(cs[i])) {
			log_err("Cs already assigned: %d\n", nand->cs_used[i]);
			return -EINVAL;
		}

		nfc->cs_assigned |= BIT(cs[i]);
		nand->cs_used[i] = cs[i];
	}

	gpio_request_by_name_nodev(node, "wp-gpios", 0, &nand->wp_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);

	nand->chip.flash_node = node;

	return 0;
}

static int stm32_fmc2_nfc_parse_dt(struct udevice *dev,
				   struct stm32_fmc2_nfc *nfc)
{
	ofnode child;
	int ret, nchips = 0;

	dev_for_each_subnode(child, dev)
		nchips++;

	if (!nchips) {
		log_err("NAND chip not defined\n");
		return -EINVAL;
	}

	if (nchips > 1) {
		log_err("Too many NAND chips defined\n");
		return -EINVAL;
	}

	dev_for_each_subnode(child, dev) {
		ret = stm32_fmc2_nfc_parse_child(nfc, child);
		if (ret)
			return ret;
	}

	return 0;
}

static struct udevice *stm32_fmc2_nfc_get_cdev(struct udevice *dev)
{
	struct udevice *pdev = dev_get_parent(dev);
	struct udevice *cdev = NULL;
	bool ebi_found = false;

	if (pdev && ofnode_device_is_compatible(dev_ofnode(pdev),
						"st,stm32mp1-fmc2-ebi"))
		ebi_found = true;

	if (ofnode_device_is_compatible(dev_ofnode(dev),
					"st,stm32mp1-fmc2-nfc")) {
		if (ebi_found)
			cdev = pdev;

		return cdev;
	}

	if (!ebi_found)
		cdev = dev;

	return cdev;
}

static int stm32_fmc2_nfc_probe(struct udevice *dev)
{
	struct stm32_fmc2_nfc *nfc = dev_get_priv(dev);
	struct stm32_fmc2_nand *nand = &nfc->nand;
	struct nand_chip *chip = &nand->chip;
	struct mtd_info *mtd = &chip->mtd;
	struct nand_ecclayout *ecclayout;
	struct udevice *cdev;
	struct reset_ctl reset;
	int oob_index, chip_cs, mem_region, ret;
	unsigned int i;
	int start_region = 0;
	fdt_addr_t addr;

	spin_lock_init(&nfc->controller.lock);
	init_waitqueue_head(&nfc->controller.wq);

	cdev = stm32_fmc2_nfc_get_cdev(dev);
	if (!cdev)
		return -EINVAL;

	ret = stm32_fmc2_nfc_parse_dt(dev, nfc);
	if (ret)
		return ret;

	nfc->io_base = dev_read_addr(cdev);
	if (nfc->io_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	if (dev == cdev)
		start_region = 1;

	for (chip_cs = 0, mem_region = start_region; chip_cs < FMC2_MAX_CE;
	     chip_cs++, mem_region += 3) {
		if (!(nfc->cs_assigned & BIT(chip_cs)))
			continue;

		addr = dev_read_addr_index(dev, mem_region);
		if (addr == FDT_ADDR_T_NONE) {
			dev_err(dev, "Resource data_base not found for cs%d", chip_cs);
			return ret;
		}
		nfc->data_base[chip_cs] = addr;

		addr = dev_read_addr_index(dev, mem_region + 1);
		if (addr == FDT_ADDR_T_NONE) {
			dev_err(dev, "Resource cmd_base not found for cs%d", chip_cs);
			return ret;
		}
		nfc->cmd_base[chip_cs] = addr;

		addr = dev_read_addr_index(dev, mem_region + 2);
		if (addr == FDT_ADDR_T_NONE) {
			dev_err(dev, "Resource addr_base not found for cs%d", chip_cs);
			return ret;
		}
		nfc->addr_base[chip_cs] = addr;
	}

	/* Enable the clock */
	ret = clk_get_by_index(cdev, 0, &nfc->clk);
	if (ret)
		return ret;

	ret = clk_enable(&nfc->clk);
	if (ret)
		return ret;

	/* Reset */
	ret = reset_get_by_index(dev, 0, &reset);
	if (!ret) {
		reset_assert(&reset);
		udelay(2);
		reset_deassert(&reset);
	}

	stm32_fmc2_nfc_init(nfc, dev != cdev);

	chip->controller = &nfc->base;
	chip->select_chip = stm32_fmc2_nfc_select_chip;
	chip->setup_data_interface = stm32_fmc2_nfc_setup_interface;
	chip->cmd_ctrl = stm32_fmc2_nfc_cmd_ctrl;
	chip->chip_delay = FMC2_RB_DELAY_US;
	chip->options |= NAND_BUSWIDTH_AUTO | NAND_NO_SUBPAGE_WRITE |
			 NAND_USE_BOUNCE_BUFFER;

	/* Default ECC settings */
	chip->ecc.mode = NAND_ECC_HW;
	chip->ecc.size = FMC2_ECC_STEP_SIZE;
	chip->ecc.strength = FMC2_ECC_BCH8;

	/* Disable Write Protect */
	if (dm_gpio_is_valid(&nand->wp_gpio))
		dm_gpio_set_value(&nand->wp_gpio, 0);

	ret = nand_scan_ident(mtd, nand->ncs, NULL);
	if (ret)
		return ret;

	/*
	 * Only NAND_ECC_HW mode is actually supported
	 * Hamming => ecc.strength = 1
	 * BCH4 => ecc.strength = 4
	 * BCH8 => ecc.strength = 8
	 * ECC sector size = 512
	 */
	if (chip->ecc.mode != NAND_ECC_HW) {
		dev_err(dev, "Nand_ecc_mode is not well defined in the DT\n");
		return -EINVAL;
	}

	ret = nand_check_ecc_caps(chip, &stm32_fmc2_nfc_ecc_caps,
				  mtd->oobsize - FMC2_BBM_LEN);
	if (ret) {
		dev_err(dev, "No valid ECC settings set\n");
		return ret;
	}

	if (chip->bbt_options & NAND_BBT_USE_FLASH)
		chip->bbt_options |= NAND_BBT_NO_OOB;

	stm32_fmc2_nfc_nand_callbacks_setup(chip);

	/* Define ECC layout */
	ecclayout = &nfc->ecclayout;
	ecclayout->eccbytes = chip->ecc.bytes *
			      (mtd->writesize / chip->ecc.size);
	oob_index = FMC2_BBM_LEN;
	for (i = 0; i < ecclayout->eccbytes; i++, oob_index++)
		ecclayout->eccpos[i] = oob_index;
	ecclayout->oobfree->offset = oob_index;
	ecclayout->oobfree->length = mtd->oobsize - ecclayout->oobfree->offset;
	chip->ecc.layout = ecclayout;

	if (chip->options & NAND_BUSWIDTH_16)
		stm32_fmc2_nfc_set_buswidth_16(nfc, true);

	ret = nand_scan_tail(mtd);
	if (ret)
		return ret;

	return nand_register(0, mtd);
}

static const struct udevice_id stm32_fmc2_nfc_match[] = {
	{ .compatible = "st,stm32mp15-fmc2" },
	{ .compatible = "st,stm32mp1-fmc2-nfc" },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(stm32_fmc2_nfc) = {
	.name = "stm32_fmc2_nfc",
	.id = UCLASS_MTD,
	.of_match = stm32_fmc2_nfc_match,
	.probe = stm32_fmc2_nfc_probe,
	.priv_auto	= sizeof(struct stm32_fmc2_nfc),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(stm32_fmc2_nfc),
					  &dev);
	if (ret && ret != -ENODEV)
		log_err("Failed to initialize STM32 FMC2 NFC controller. (error %d)\n",
			ret);
}
