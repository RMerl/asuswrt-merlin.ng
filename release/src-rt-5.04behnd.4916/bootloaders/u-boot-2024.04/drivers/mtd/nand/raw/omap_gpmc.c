// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004-2008 Texas Instruments, <www.ti.com>
 * Rohit Choraria <rohitkc@ti.com>
 */

#include <common.h>
#include <log.h>
#include <system-constants.h>
#include <asm/io.h>
#include <dm.h>
#include <linux/errno.h>

#ifdef CONFIG_ARCH_OMAP2PLUS
#include <asm/arch/mem.h>
#endif

#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/mtd/omap_gpmc.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/rawnand.h>
#include <linux/bch.h>
#include <linux/compiler.h>
#include <nand.h>

#include "omap_elm.h"

#ifndef GPMC_MAX_CS
#define GPMC_MAX_CS	4
#endif

#define BADBLOCK_MARKER_LENGTH	2
#define SECTOR_BYTES		512
#define ECCSIZE0_SHIFT		12
#define ECCSIZE1_SHIFT		22
#define ECC1RESULTSIZE		0x1
#define ECCCLEAR		(0x1 << 8)
#define ECCRESULTREG1		(0x1 << 0)
/* 4 bit padding to make byte aligned, 56 = 52 + 4 */
#define BCH4_BIT_PAD		4

#ifdef CONFIG_BCH
static u8  bch8_polynomial[] = {0xef, 0x51, 0x2e, 0x09, 0xed, 0x93, 0x9a, 0xc2,
				0x97, 0x79, 0xe5, 0x24, 0xb5};
#endif
static uint8_t cs_next;

#if defined(CONFIG_NAND_OMAP_GPMC_WSCFG)
static const int8_t wscfg[CONFIG_SYS_MAX_NAND_DEVICE] =
	{ CONFIG_NAND_OMAP_GPMC_WSCFG };
#else
/* wscfg is preset to zero since its a static variable */
static const int8_t wscfg[CONFIG_SYS_MAX_NAND_DEVICE];
#endif

/*
 * Driver configurations
 */
struct omap_nand_info {
	struct bch_control *control;
	enum omap_ecc ecc_scheme;
	uint8_t cs;
	uint8_t ws;		/* wait status pin (0,1) */
	void __iomem *fifo;
};

/* We are wasting a bit of memory but al least we are safe */
static struct omap_nand_info omap_nand_info[GPMC_MAX_CS];

/*
 * omap_nand_hwcontrol - Set the address pointers corretly for the
 *			following address/data/command operation
 */
static void omap_nand_hwcontrol(struct mtd_info *mtd, int32_t cmd,
				uint32_t ctrl)
{
	register struct nand_chip *this = mtd_to_nand(mtd);
	struct omap_nand_info *info = nand_get_controller_data(this);
	int cs = info->cs;

	/*
	 * Point the IO_ADDR to DATA and ADDRESS registers instead
	 * of chip address
	 */
	switch (ctrl) {
	case NAND_CTRL_CHANGE | NAND_CTRL_CLE:
		this->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_cmd;
		break;
	case NAND_CTRL_CHANGE | NAND_CTRL_ALE:
		this->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_adr;
		break;
	case NAND_CTRL_CHANGE | NAND_NCE:
		this->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_dat;
		break;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

/* Check wait pin as dev ready indicator */
static int omap_dev_ready(struct mtd_info *mtd)
{
	register struct nand_chip *this = mtd_to_nand(mtd);
	struct omap_nand_info *info = nand_get_controller_data(this);
	return gpmc_cfg->status & (1 << (8 + info->ws));
}

/*
 * gen_true_ecc - This function will generate true ECC value, which
 * can be used when correcting data read from NAND flash memory core
 *
 * @ecc_buf:	buffer to store ecc code
 *
 * @return:	re-formatted ECC value
 */
static uint32_t gen_true_ecc(uint8_t *ecc_buf)
{
	return ecc_buf[0] | (ecc_buf[1] << 16) | ((ecc_buf[2] & 0xF0) << 20) |
		((ecc_buf[2] & 0x0F) << 8);
}

/*
 * omap_correct_data - Compares the ecc read from nand spare area with ECC
 * registers values and corrects one bit error if it has occurred
 * Further details can be had from OMAP TRM and the following selected links:
 * http://en.wikipedia.org/wiki/Hamming_code
 * http://www.cs.utexas.edu/users/plaxton/c/337/05f/slides/ErrorCorrection-4.pdf
 *
 * @mtd:		 MTD device structure
 * @dat:		 page data
 * @read_ecc:		 ecc read from nand flash
 * @calc_ecc:		 ecc read from ECC registers
 *
 * Return: 0 if data is OK or corrected, else returns -1
 */
static int __maybe_unused omap_correct_data(struct mtd_info *mtd, uint8_t *dat,
				uint8_t *read_ecc, uint8_t *calc_ecc)
{
	uint32_t orig_ecc, new_ecc, res, hm;
	uint16_t parity_bits, byte;
	uint8_t bit;

	/* Regenerate the orginal ECC */
	orig_ecc = gen_true_ecc(read_ecc);
	new_ecc = gen_true_ecc(calc_ecc);
	/* Get the XOR of real ecc */
	res = orig_ecc ^ new_ecc;
	if (res) {
		/* Get the hamming width */
		hm = hweight32(res);
		/* Single bit errors can be corrected! */
		if (hm == 12) {
			/* Correctable data! */
			parity_bits = res >> 16;
			bit = (parity_bits & 0x7);
			byte = (parity_bits >> 3) & 0x1FF;
			/* Flip the bit to correct */
			dat[byte] ^= (0x1 << bit);
		} else if (hm == 1) {
			printf("Error: Ecc is wrong\n");
			/* ECC itself is corrupted */
			return 2;
		} else {
			/*
			 * hm distance != parity pairs OR one, could mean 2 bit
			 * error OR potentially be on a blank page..
			 * orig_ecc: contains spare area data from nand flash.
			 * new_ecc: generated ecc while reading data area.
			 * Note: if the ecc = 0, all data bits from which it was
			 * generated are 0xFF.
			 * The 3 byte(24 bits) ecc is generated per 512byte
			 * chunk of a page. If orig_ecc(from spare area)
			 * is 0xFF && new_ecc(computed now from data area)=0x0,
			 * this means that data area is 0xFF and spare area is
			 * 0xFF. A sure sign of a erased page!
			 */
			if ((orig_ecc == 0x0FFF0FFF) && (new_ecc == 0x00000000))
				return 0;
			printf("Error: Bad compare! failed\n");
			/* detected 2 bit error */
			return -EBADMSG;
		}
	}
	return 0;
}

/*
 * omap_enable_hwecc - configures GPMC as per ECC scheme before read/write
 * @mtd:	MTD device structure
 * @mode:	Read/Write mode
 */
__maybe_unused
static void omap_enable_hwecc(struct mtd_info *mtd, int32_t mode)
{
	struct nand_chip *nand = mtd_to_nand(mtd);
	struct omap_nand_info *info = nand_get_controller_data(nand);
	unsigned int dev_width = (nand->options & NAND_BUSWIDTH_16) ? 1 : 0;
	u32 val;

	/* Clear ecc and enable bits */
	writel(ECCCLEAR | ECCRESULTREG1, &gpmc_cfg->ecc_control);

	/* program ecc and result sizes */
	val = ((((nand->ecc.size >> 1) - 1) << ECCSIZE1_SHIFT) |
			ECC1RESULTSIZE);
	writel(val, &gpmc_cfg->ecc_size_config);

	switch (mode) {
	case NAND_ECC_READ:
	case NAND_ECC_WRITE:
		writel(ECCCLEAR | ECCRESULTREG1, &gpmc_cfg->ecc_control);
		break;
	case NAND_ECC_READSYN:
		writel(ECCCLEAR, &gpmc_cfg->ecc_control);
		break;
	default:
		printf("%s: error: unrecognized Mode[%d]!\n", __func__, mode);
		break;
	}

	/* (ECC 16 or 8 bit col) | ( CS  )  | ECC Enable */
	val = (dev_width << 7) | (info->cs << 1) | (0x1);
	writel(val, &gpmc_cfg->ecc_config);
}

/*
 *  omap_calculate_ecc - Read ECC result
 *  @mtd:	MTD structure
 *  @dat:	unused
 *  @ecc_code:	ecc_code buffer
 *  Using noninverted ECC can be considered ugly since writing a blank
 *  page ie. padding will clear the ECC bytes. This is no problem as
 *  long nobody is trying to write data on the seemingly unused page.
 *  Reading an erased page will produce an ECC mismatch between
 *  generated and read ECC bytes that has to be dealt with separately.
 *  E.g. if page is 0xFF (fresh erased), and if HW ECC engine within GPMC
 *  is used, the result of read will be 0x0 while the ECC offsets of the
 *  spare area will be 0xFF which will result in an ECC mismatch.
 */
static int omap_calculate_ecc(struct mtd_info *mtd, const uint8_t *dat,
				uint8_t *ecc_code)
{
	u32 val;

	val = readl(&gpmc_cfg->ecc1_result);
	ecc_code[0] = val & 0xFF;
	ecc_code[1] = (val >> 16) & 0xFF;
	ecc_code[2] = ((val >> 8) & 0x0F) | ((val >> 20) & 0xF0);

	return 0;
}

/* GPMC ecc engine settings for read */
#define BCH_WRAPMODE_1          1       /* BCH wrap mode 1 */
#define BCH8R_ECC_SIZE0         0x1a    /* ecc_size0 = 26 */
#define BCH8R_ECC_SIZE1         0x2     /* ecc_size1 = 2 */
#define BCH4R_ECC_SIZE0         0xd     /* ecc_size0 = 13 */
#define BCH4R_ECC_SIZE1         0x3     /* ecc_size1 = 3 */

/* GPMC ecc engine settings for write */
#define BCH_WRAPMODE_6          6       /* BCH wrap mode 6 */
#define BCH_ECC_SIZE0           0x0     /* ecc_size0 = 0, no oob protection */
#define BCH_ECC_SIZE1           0x20    /* ecc_size1 = 32 */

/**
 * omap_enable_hwecc_bch - Program GPMC to perform BCH ECC calculation
 * @mtd: MTD device structure
 * @mode: Read/Write mode
 *
 * When using BCH with SW correction (i.e. no ELM), sector size is set
 * to 512 bytes and we use BCH_WRAPMODE_6 wrapping mode
 * for both reading and writing with:
 * eccsize0 = 0  (no additional protected byte in spare area)
 * eccsize1 = 32 (skip 32 nibbles = 16 bytes per sector in spare area)
 */
static void __maybe_unused omap_enable_hwecc_bch(struct mtd_info *mtd,
						 int mode)
{
	unsigned int bch_type;
	unsigned int dev_width, nsectors;
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct omap_nand_info *info = nand_get_controller_data(chip);
	u32 val, wr_mode;
	unsigned int ecc_size1, ecc_size0;

	/* GPMC configurations for calculating ECC */
	switch (info->ecc_scheme) {
	case OMAP_ECC_BCH8_CODE_HW_DETECTION_SW:
		bch_type = 1;
		nsectors = 1;
		wr_mode   = BCH_WRAPMODE_6;
		ecc_size0 = BCH_ECC_SIZE0;
		ecc_size1 = BCH_ECC_SIZE1;
		break;
	case OMAP_ECC_BCH8_CODE_HW:
		bch_type = 1;
		nsectors = 1;
		if (mode == NAND_ECC_READ) {
			wr_mode   = BCH_WRAPMODE_1;
			ecc_size0 = BCH8R_ECC_SIZE0;
			ecc_size1 = BCH8R_ECC_SIZE1;
		} else {
			wr_mode   = BCH_WRAPMODE_6;
			ecc_size0 = BCH_ECC_SIZE0;
			ecc_size1 = BCH_ECC_SIZE1;
		}
		break;
	case OMAP_ECC_BCH16_CODE_HW:
		bch_type = 0x2;
		nsectors = 1;
		if (mode == NAND_ECC_READ) {
			wr_mode   = 0x01;
			ecc_size0 = 52; /* ECC bits in nibbles per sector */
			ecc_size1 = 0;  /* non-ECC bits in nibbles per sector */
		} else {
			wr_mode   = 0x01;
			ecc_size0 = 0;  /* extra bits in nibbles per sector */
			ecc_size1 = 52; /* OOB bits in nibbles per sector */
		}
		break;
	default:
		return;
	}

	writel(ECCRESULTREG1, &gpmc_cfg->ecc_control);

	/* Configure ecc size for BCH */
	val = (ecc_size1 << ECCSIZE1_SHIFT) | (ecc_size0 << ECCSIZE0_SHIFT);
	writel(val, &gpmc_cfg->ecc_size_config);

	dev_width = (chip->options & NAND_BUSWIDTH_16) ? 1 : 0;

	/* BCH configuration */
	val = ((1			<< 16) | /* enable BCH */
	       (bch_type		<< 12) | /* BCH4/BCH8/BCH16 */
	       (wr_mode			<<  8) | /* wrap mode */
	       (dev_width		<<  7) | /* bus width */
	       (((nsectors - 1) & 0x7)	<<  4) | /* number of sectors */
	       (info->cs		<<  1) | /* ECC CS */
	       (0x1));				 /* enable ECC */

	writel(val, &gpmc_cfg->ecc_config);

	/* Clear ecc and enable bits */
	writel(ECCCLEAR | ECCRESULTREG1, &gpmc_cfg->ecc_control);
}

/**
 * omap_calculate_ecc_bch - Generate BCH ECC bytes for one sector
 * @mtd:        MTD device structure
 * @dat:        The pointer to data on which ecc is computed
 * @ecc_code:   The ecc_code buffer
 *
 * Support calculating of BCH4/8/16 ECC vectors for one sector
 * within a page. Sector number is in @sector.
 */
static int __maybe_unused omap_calculate_ecc_bch(struct mtd_info *mtd, const u8 *dat,
						 u8 *ecc_code)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct omap_nand_info *info = nand_get_controller_data(chip);
	const uint32_t *ptr;
	uint32_t val = 0;
	int8_t i = 0, j;

	switch (info->ecc_scheme) {
#ifdef CONFIG_BCH
	case OMAP_ECC_BCH8_CODE_HW_DETECTION_SW:
#endif
	case OMAP_ECC_BCH8_CODE_HW:
		ptr = &gpmc_cfg->bch_result_0_3[0].bch_result_x[3];
		val = readl(ptr);
		ecc_code[i++] = (val >>  0) & 0xFF;
		ptr--;
		for (j = 0; j < 3; j++) {
			val = readl(ptr);
			ecc_code[i++] = (val >> 24) & 0xFF;
			ecc_code[i++] = (val >> 16) & 0xFF;
			ecc_code[i++] = (val >>  8) & 0xFF;
			ecc_code[i++] = (val >>  0) & 0xFF;
			ptr--;
		}

		break;
	case OMAP_ECC_BCH16_CODE_HW:
		val = readl(&gpmc_cfg->bch_result_4_6[0].bch_result_x[2]);
		ecc_code[i++] = (val >>  8) & 0xFF;
		ecc_code[i++] = (val >>  0) & 0xFF;
		val = readl(&gpmc_cfg->bch_result_4_6[0].bch_result_x[1]);
		ecc_code[i++] = (val >> 24) & 0xFF;
		ecc_code[i++] = (val >> 16) & 0xFF;
		ecc_code[i++] = (val >>  8) & 0xFF;
		ecc_code[i++] = (val >>  0) & 0xFF;
		val = readl(&gpmc_cfg->bch_result_4_6[0].bch_result_x[0]);
		ecc_code[i++] = (val >> 24) & 0xFF;
		ecc_code[i++] = (val >> 16) & 0xFF;
		ecc_code[i++] = (val >>  8) & 0xFF;
		ecc_code[i++] = (val >>  0) & 0xFF;
		for (j = 3; j >= 0; j--) {
			val = readl(&gpmc_cfg->bch_result_0_3[0].bch_result_x[j]
									);
			ecc_code[i++] = (val >> 24) & 0xFF;
			ecc_code[i++] = (val >> 16) & 0xFF;
			ecc_code[i++] = (val >>  8) & 0xFF;
			ecc_code[i++] = (val >>  0) & 0xFF;
		}
		break;
	default:
		return -EINVAL;
	}
	/* ECC scheme specific syndrome customizations */
	switch (info->ecc_scheme) {
#ifdef CONFIG_BCH
	case OMAP_ECC_BCH8_CODE_HW_DETECTION_SW:
		/* Add constant polynomial to remainder, so that
		 * ECC of blank pages results in 0x0 on reading back
		 */
		for (i = 0; i < chip->ecc.bytes; i++)
			ecc_code[i] ^= bch8_polynomial[i];
		break;
#endif
	case OMAP_ECC_BCH8_CODE_HW:
		/* Set 14th ECC byte as 0x0 for ROM compatibility */
		ecc_code[chip->ecc.bytes - 1] = 0x0;
		break;
	case OMAP_ECC_BCH16_CODE_HW:
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static inline void omap_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct omap_nand_info *info = nand_get_controller_data(chip);
	u32 alignment = ((uintptr_t)buf | len) & 3;

	if (alignment & 1)
		readsb(info->fifo, buf, len);
	else if (alignment & 3)
		readsw(info->fifo, buf, len >> 1);
	else
		readsl(info->fifo, buf, len >> 2);
}

#ifdef CONFIG_NAND_OMAP_GPMC_PREFETCH

#define PREFETCH_CONFIG1_CS_SHIFT	24
#define PREFETCH_FIFOTHRESHOLD_MAX	0x40
#define PREFETCH_FIFOTHRESHOLD(val)	((val) << 8)
#define PREFETCH_STATUS_COUNT(val)	(val & 0x00003fff)
#define PREFETCH_STATUS_FIFO_CNT(val)	((val >> 24) & 0x7F)
#define ENABLE_PREFETCH			(1 << 7)

/**
 * omap_prefetch_enable - configures and starts prefetch transfer
 * @fifo_th: fifo threshold to be used for read/ write
 * @count: number of bytes to be transferred
 * @is_write: prefetch read(0) or write post(1) mode
 * @cs: chip select to use
 */
static int omap_prefetch_enable(int fifo_th, unsigned int count, int is_write, int cs)
{
	uint32_t val;

	if (fifo_th > PREFETCH_FIFOTHRESHOLD_MAX)
		return -EINVAL;

	if (readl(&gpmc_cfg->prefetch_control))
		return -EBUSY;

	/* Set the amount of bytes to be prefetched */
	writel(count, &gpmc_cfg->prefetch_config2);

	val = (cs << PREFETCH_CONFIG1_CS_SHIFT) | (is_write & 1) |
		PREFETCH_FIFOTHRESHOLD(fifo_th) | ENABLE_PREFETCH;
	writel(val, &gpmc_cfg->prefetch_config1);

	/*  Start the prefetch engine */
	writel(1, &gpmc_cfg->prefetch_control);

	return 0;
}

/**
 * omap_prefetch_reset - disables and stops the prefetch engine
 */
static void omap_prefetch_reset(void)
{
	writel(0, &gpmc_cfg->prefetch_control);
	writel(0, &gpmc_cfg->prefetch_config1);
}

static int __read_prefetch_aligned(struct nand_chip *chip, uint32_t *buf, int len)
{
	int ret;
	uint32_t cnt;
	struct omap_nand_info *info = nand_get_controller_data(chip);

	ret = omap_prefetch_enable(PREFETCH_FIFOTHRESHOLD_MAX, len, 0, info->cs);
	if (ret < 0)
		return ret;

	do {
		int i;

		cnt = readl(&gpmc_cfg->prefetch_status);
		cnt = PREFETCH_STATUS_FIFO_CNT(cnt);

		for (i = 0; i < cnt / 4; i++) {
			*buf++ = readl(info->fifo);
			len -= 4;
		}
	} while (len);

	omap_prefetch_reset();

	return 0;
}

static void omap_nand_read_prefetch(struct mtd_info *mtd, uint8_t *buf, int len)
{
	int ret;
	uintptr_t head, tail;
	struct nand_chip *chip = mtd_to_nand(mtd);

	/*
	 * If the destination buffer is unaligned, start with reading
	 * the overlap byte-wise.
	 */
	head = ((uintptr_t)buf) % 4;
	if (head) {
		omap_nand_read_buf(mtd, buf, head);
		buf += head;
		len -= head;
	}

	/*
	 * Only transfer multiples of 4 bytes in a pre-fetched fashion.
	 * If there's a residue, care for it byte-wise afterwards.
	 */
	tail = len % 4;

	ret = __read_prefetch_aligned(chip, (uint32_t *)buf, len - tail);
	if (ret < 0) {
		/* fallback in case the prefetch engine is busy */
		omap_nand_read_buf(mtd, buf, len);
	} else if (tail) {
		buf += len - tail;
		omap_nand_read_buf(mtd, buf, tail);
	}
}
#endif /* CONFIG_NAND_OMAP_GPMC_PREFETCH */

#ifdef CONFIG_NAND_OMAP_ELM

/*
 * omap_reverse_list - re-orders list elements in reverse order [internal]
 * @list:	pointer to start of list
 * @length:	length of list
*/
static void omap_reverse_list(u8 *list, unsigned int length)
{
	unsigned int i, j;
	unsigned int half_length = length / 2;
	u8 tmp;
	for (i = 0, j = length - 1; i < half_length; i++, j--) {
		tmp = list[i];
		list[i] = list[j];
		list[j] = tmp;
	}
}

/*
 * omap_correct_data_bch - Compares the ecc read from nand spare area
 * with ECC registers values and corrects one bit error if it has occurred
 *
 * @mtd:	MTD device structure
 * @dat:	page data
 * @read_ecc:	ecc read from nand flash (ignored)
 * @calc_ecc:	ecc read from ECC registers
 *
 * Return: 0 if data is OK or corrected, else returns -1
 */
static int omap_correct_data_bch(struct mtd_info *mtd, uint8_t *dat,
				uint8_t *read_ecc, uint8_t *calc_ecc)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct omap_nand_info *info = nand_get_controller_data(chip);
	struct nand_ecc_ctrl *ecc = &chip->ecc;
	uint32_t error_count = 0, error_max;
	uint32_t error_loc[ELM_MAX_ERROR_COUNT];
	enum bch_level bch_type;
	uint32_t i, ecc_flag = 0;
	uint8_t count;
	uint32_t byte_pos, bit_pos;
	int err = 0;

	/* check calculated ecc */
	for (i = 0; i < ecc->bytes && !ecc_flag; i++) {
		if (calc_ecc[i] != 0x00)
			goto not_ecc_match;
	}
	return 0;
not_ecc_match:

	/* check for whether it's an erased-page */
	for (i = 0; i < ecc->bytes; i++) {
		if (read_ecc[i] != 0xff)
			goto not_erased;
	}
	for (i = 0; i < SECTOR_BYTES; i++) {
		if (dat[i] != 0xff)
			goto not_erased;
	}
	return 0;
not_erased:

	/*
	 * Check for whether it's an erased page with a correctable
	 * number of bitflips. Erased pages have all 1's in the data,
	 * so we just compute the number of 0 bits in the data and
	 * see if it's under the correction threshold.
	 *
	 * NOTE: The check for a perfect erased page above is faster for
	 * the more common case, even though it's logically redundant.
	 */
	for (i = 0; i < ecc->bytes; i++)
		error_count += hweight8(~read_ecc[i]);

	for (i = 0; i < SECTOR_BYTES; i++)
		error_count += hweight8(~dat[i]);

	if (error_count <= ecc->strength) {
		memset(read_ecc, 0xFF, ecc->bytes);
		memset(dat, 0xFF, SECTOR_BYTES);
		debug("nand: %u bit-flip(s) corrected in erased page\n",
		      error_count);
		return error_count;
	}

	/*
	 * while reading ECC result we read it in big endian.
	 * Hence while loading to ELM we have rotate to get the right endian.
	 */
	switch (info->ecc_scheme) {
	case OMAP_ECC_BCH8_CODE_HW:
		bch_type = BCH_8_BIT;
		omap_reverse_list(calc_ecc, ecc->bytes - 1);
		break;
	case OMAP_ECC_BCH16_CODE_HW:
		bch_type = BCH_16_BIT;
		omap_reverse_list(calc_ecc, ecc->bytes);
		break;
	default:
		return -EINVAL;
	}
	/* use elm module to check for errors */
	elm_config(bch_type);
	error_count = 0;
	err = elm_check_error(calc_ecc, bch_type, &error_count, error_loc);
	if (err)
		return err;

	/* correct bch error */
	for (count = 0; count < error_count; count++) {
		switch (info->ecc_scheme) {
		case OMAP_ECC_BCH8_CODE_HW:
			/* 14th byte in ECC is reserved to match ROM layout */
			error_max = SECTOR_BYTES + (ecc->bytes - 1);
			break;
		case OMAP_ECC_BCH16_CODE_HW:
			error_max = SECTOR_BYTES + ecc->bytes;
			break;
		default:
			return -EINVAL;
		}
		byte_pos = error_max - (error_loc[count] / 8) - 1;
		bit_pos  = error_loc[count] % 8;
		if (byte_pos < SECTOR_BYTES) {
			dat[byte_pos] ^= 1 << bit_pos;
			debug("nand: bit-flip corrected @data=%d\n", byte_pos);
		} else if (byte_pos < error_max) {
			read_ecc[byte_pos - SECTOR_BYTES] ^= 1 << bit_pos;
			debug("nand: bit-flip corrected @oob=%d\n", byte_pos -
								SECTOR_BYTES);
		} else {
			err = -EBADMSG;
			printf("nand: error: invalid bit-flip location\n");
		}
	}
	return (err) ? err : error_count;
}

/**
 * omap_read_page_bch - hardware ecc based page read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	buffer to store read data
 * @oob_required: caller expects OOB data read to chip->oob_poi
 * @page:	page number to read
 *
 */
static int omap_read_page_bch(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	uint8_t *p = buf;
	uint8_t *ecc_calc = chip->buffers->ecccalc;
	uint8_t *ecc_code = chip->buffers->ecccode;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	uint8_t *oob = chip->oob_poi;
	uint32_t oob_pos;
	u32 data_pos = 0;

	/* oob area start */
	oob_pos = (eccsize * eccsteps) + chip->ecc.layout->eccpos[0];
	oob += chip->ecc.layout->eccpos[0];

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize,
	     oob += eccbytes) {
		/* Enable ECC engine */
		chip->ecc.hwctl(mtd, NAND_ECC_READ);

		/* read data */
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, data_pos, -1);
		chip->read_buf(mtd, p, eccsize);

		/* read respective ecc from oob area */
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, oob_pos, -1);
		chip->read_buf(mtd, oob, eccbytes);
		/* read syndrome */
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);

		data_pos += eccsize;
		oob_pos += eccbytes;
	}

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = chip->oob_poi[eccpos[i]];

	/* error detect & correct */
	eccsteps = chip->ecc.steps;
	p = buf;

	for (i = 0 ; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		int stat;
		stat = chip->ecc.correct(mtd, p, &ecc_code[i], &ecc_calc[i]);
		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;
	}

	return 0;
}
#endif /* CONFIG_NAND_OMAP_ELM */

/*
 * OMAP3 BCH8 support (with BCH library)
 */
#ifdef CONFIG_BCH
/**
 * omap_correct_data_bch_sw - Decode received data and correct errors
 * @mtd: MTD device structure
 * @data: page data
 * @read_ecc: ecc read from nand flash
 * @calc_ecc: ecc read from HW ECC registers
 */
static int omap_correct_data_bch_sw(struct mtd_info *mtd, u_char *data,
				 u_char *read_ecc, u_char *calc_ecc)
{
	int i, count;
	/* cannot correct more than 8 errors */
	unsigned int errloc[8];
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct omap_nand_info *info = nand_get_controller_data(chip);

	count = decode_bch(info->control, NULL, SECTOR_BYTES,
				read_ecc, calc_ecc, NULL, errloc);
	if (count > 0) {
		/* correct errors */
		for (i = 0; i < count; i++) {
			/* correct data only, not ecc bytes */
			if (errloc[i] < SECTOR_BYTES << 3)
				data[errloc[i] >> 3] ^= 1 << (errloc[i] & 7);
			debug("corrected bitflip %u\n", errloc[i]);
#ifdef DEBUG
			puts("read_ecc: ");
			/*
			 * BCH8 have 13 bytes of ECC; BCH4 needs adoption
			 * here!
			 */
			for (i = 0; i < 13; i++)
				printf("%02x ", read_ecc[i]);
			puts("\n");
			puts("calc_ecc: ");
			for (i = 0; i < 13; i++)
				printf("%02x ", calc_ecc[i]);
			puts("\n");
#endif
		}
	} else if (count < 0) {
		puts("ecc unrecoverable error\n");
	}
	return count;
}

/**
 * omap_free_bch - Release BCH ecc resources
 * @mtd: MTD device structure
 */
static void __maybe_unused omap_free_bch(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct omap_nand_info *info = nand_get_controller_data(chip);

	if (info->control) {
		free_bch(info->control);
		info->control = NULL;
	}
}
#endif /* CONFIG_BCH */

/**
 * omap_select_ecc_scheme - configures driver for particular ecc-scheme
 * @nand: NAND chip device structure
 * @ecc_scheme: ecc scheme to configure
 * @pagesize: number of main-area bytes per page of NAND device
 * @oobsize: number of OOB/spare bytes per page of NAND device
 */
static int omap_select_ecc_scheme(struct nand_chip *nand,
	enum omap_ecc ecc_scheme, unsigned int pagesize, unsigned int oobsize) {
	struct omap_nand_info	*info		= nand_get_controller_data(nand);
	struct nand_ecclayout	*ecclayout	= nand->ecc.layout;
	int eccsteps = pagesize / SECTOR_BYTES;
	int i;

	switch (ecc_scheme) {
	case OMAP_ECC_HAM1_CODE_SW:
		debug("nand: selected OMAP_ECC_HAM1_CODE_SW\n");
		/* For this ecc-scheme, ecc.bytes, ecc.layout, ... are
		 * initialized in nand_scan_tail(), so just set ecc.mode */
		info->control		= NULL;
		nand->ecc.mode		= NAND_ECC_SOFT;
		nand->ecc.layout	= NULL;
		nand->ecc.size		= 0;
		break;

	case OMAP_ECC_HAM1_CODE_HW:
		debug("nand: selected OMAP_ECC_HAM1_CODE_HW\n");
		/* check ecc-scheme requirements before updating ecc info */
		if ((3 * eccsteps) + BADBLOCK_MARKER_LENGTH > oobsize) {
			printf("nand: error: insufficient OOB: require=%d\n", (
				(3 * eccsteps) + BADBLOCK_MARKER_LENGTH));
			return -EINVAL;
		}
		info->control		= NULL;
		/* populate ecc specific fields */
		memset(&nand->ecc, 0, sizeof(struct nand_ecc_ctrl));
		nand->ecc.mode		= NAND_ECC_HW;
		nand->ecc.strength	= 1;
		nand->ecc.size		= SECTOR_BYTES;
		nand->ecc.bytes		= 3;
		nand->ecc.hwctl		= omap_enable_hwecc;
		nand->ecc.correct	= omap_correct_data;
		nand->ecc.calculate	= omap_calculate_ecc;
		/* define ecc-layout */
		ecclayout->eccbytes	= nand->ecc.bytes * eccsteps;
		for (i = 0; i < ecclayout->eccbytes; i++) {
			if (nand->options & NAND_BUSWIDTH_16)
				ecclayout->eccpos[i] = i + 2;
			else
				ecclayout->eccpos[i] = i + 1;
		}
		ecclayout->oobfree[0].offset = i + BADBLOCK_MARKER_LENGTH;
		ecclayout->oobfree[0].length = oobsize - ecclayout->eccbytes -
						BADBLOCK_MARKER_LENGTH;
		break;

	case OMAP_ECC_BCH8_CODE_HW_DETECTION_SW:
#ifdef CONFIG_BCH
		debug("nand: selected OMAP_ECC_BCH8_CODE_HW_DETECTION_SW\n");
		/* check ecc-scheme requirements before updating ecc info */
		if ((13 * eccsteps) + BADBLOCK_MARKER_LENGTH > oobsize) {
			printf("nand: error: insufficient OOB: require=%d\n", (
				(13 * eccsteps) + BADBLOCK_MARKER_LENGTH));
			return -EINVAL;
		}
		/* check if BCH S/W library can be used for error detection */
		info->control = init_bch(13, 8, 0x201b);
		if (!info->control) {
			printf("nand: error: could not init_bch()\n");
			return -ENODEV;
		}
		/* populate ecc specific fields */
		memset(&nand->ecc, 0, sizeof(struct nand_ecc_ctrl));
		nand->ecc.mode		= NAND_ECC_HW;
		nand->ecc.strength	= 8;
		nand->ecc.size		= SECTOR_BYTES;
		nand->ecc.bytes		= 13;
		nand->ecc.hwctl		= omap_enable_hwecc_bch;
		nand->ecc.correct	= omap_correct_data_bch_sw;
		nand->ecc.calculate	= omap_calculate_ecc_bch;
		nand->ecc.steps		= eccsteps;
		/* define ecc-layout */
		ecclayout->eccbytes	= nand->ecc.bytes * eccsteps;
		ecclayout->eccpos[0]	= BADBLOCK_MARKER_LENGTH;
		for (i = 1; i < ecclayout->eccbytes; i++) {
			if (i % nand->ecc.bytes)
				ecclayout->eccpos[i] =
						ecclayout->eccpos[i - 1] + 1;
			else
				ecclayout->eccpos[i] =
						ecclayout->eccpos[i - 1] + 2;
		}
		ecclayout->oobfree[0].offset = i + BADBLOCK_MARKER_LENGTH;
		ecclayout->oobfree[0].length = oobsize - ecclayout->eccbytes -
						BADBLOCK_MARKER_LENGTH;
		break;
#else
		printf("nand: error: CONFIG_BCH required for ECC\n");
		return -EINVAL;
#endif

	case OMAP_ECC_BCH8_CODE_HW:
#ifdef CONFIG_NAND_OMAP_ELM
		debug("nand: selected OMAP_ECC_BCH8_CODE_HW\n");
		/* check ecc-scheme requirements before updating ecc info */
		if ((14 * eccsteps) + BADBLOCK_MARKER_LENGTH > oobsize) {
			printf("nand: error: insufficient OOB: require=%d\n", (
				(14 * eccsteps) + BADBLOCK_MARKER_LENGTH));
			return -EINVAL;
		}
		/* intialize ELM for ECC error detection */
		elm_init();
		info->control		= NULL;
		/* populate ecc specific fields */
		memset(&nand->ecc, 0, sizeof(struct nand_ecc_ctrl));
		nand->ecc.mode		= NAND_ECC_HW;
		nand->ecc.strength	= 8;
		nand->ecc.size		= SECTOR_BYTES;
		nand->ecc.bytes		= 14;
		nand->ecc.hwctl		= omap_enable_hwecc_bch;
		nand->ecc.correct	= omap_correct_data_bch;
		nand->ecc.calculate	= omap_calculate_ecc_bch;
		nand->ecc.read_page	= omap_read_page_bch;
		nand->ecc.steps		= eccsteps;
		/* define ecc-layout */
		ecclayout->eccbytes	= nand->ecc.bytes * eccsteps;
		for (i = 0; i < ecclayout->eccbytes; i++)
			ecclayout->eccpos[i] = i + BADBLOCK_MARKER_LENGTH;
		ecclayout->oobfree[0].offset = i + BADBLOCK_MARKER_LENGTH;
		ecclayout->oobfree[0].length = oobsize - ecclayout->eccbytes -
						BADBLOCK_MARKER_LENGTH;
		break;
#else
		printf("nand: error: CONFIG_NAND_OMAP_ELM required for ECC\n");
		return -EINVAL;
#endif

	case OMAP_ECC_BCH16_CODE_HW:
#ifdef CONFIG_NAND_OMAP_ELM
		debug("nand: using OMAP_ECC_BCH16_CODE_HW\n");
		/* check ecc-scheme requirements before updating ecc info */
		if ((26 * eccsteps) + BADBLOCK_MARKER_LENGTH > oobsize) {
			printf("nand: error: insufficient OOB: require=%d\n", (
				(26 * eccsteps) + BADBLOCK_MARKER_LENGTH));
			return -EINVAL;
		}
		/* intialize ELM for ECC error detection */
		elm_init();
		/* populate ecc specific fields */
		nand->ecc.mode		= NAND_ECC_HW;
		nand->ecc.size		= SECTOR_BYTES;
		nand->ecc.bytes		= 26;
		nand->ecc.strength	= 16;
		nand->ecc.hwctl		= omap_enable_hwecc_bch;
		nand->ecc.correct	= omap_correct_data_bch;
		nand->ecc.calculate	= omap_calculate_ecc_bch;
		nand->ecc.read_page	= omap_read_page_bch;
		nand->ecc.steps		= eccsteps;
		/* define ecc-layout */
		ecclayout->eccbytes	= nand->ecc.bytes * eccsteps;
		for (i = 0; i < ecclayout->eccbytes; i++)
			ecclayout->eccpos[i] = i + BADBLOCK_MARKER_LENGTH;
		ecclayout->oobfree[0].offset = i + BADBLOCK_MARKER_LENGTH;
		ecclayout->oobfree[0].length = oobsize - nand->ecc.bytes -
						BADBLOCK_MARKER_LENGTH;
		break;
#else
		printf("nand: error: CONFIG_NAND_OMAP_ELM required for ECC\n");
		return -EINVAL;
#endif
	default:
		debug("nand: error: ecc scheme not enabled or supported\n");
		return -EINVAL;
	}

	/* nand_scan_tail() sets ham1 sw ecc; hw ecc layout is set by driver */
	if (ecc_scheme != OMAP_ECC_HAM1_CODE_SW)
		nand->ecc.layout = ecclayout;

	info->ecc_scheme = ecc_scheme;
	return 0;
}

#ifndef CONFIG_SPL_BUILD
/*
 * omap_nand_switch_ecc - switch the ECC operation between different engines
 * (h/w and s/w) and different algorithms (hamming and BCHx)
 *
 * @hardware		- true if one of the HW engines should be used
 * @eccstrength		- the number of bits that could be corrected
 *			  (1 - hamming, 4 - BCH4, 8 - BCH8, 16 - BCH16)
 */
int __maybe_unused omap_nand_switch_ecc(uint32_t hardware, uint32_t eccstrength)
{
	struct nand_chip *nand;
	struct mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	int err = 0;

	if (!mtd) {
		printf("nand: error: no NAND devices found\n");
		return -ENODEV;
	}

	nand = mtd_to_nand(mtd);
	nand->options |= NAND_OWN_BUFFERS;
	nand->options &= ~NAND_SUBPAGE_READ;
	/* Setup the ecc configurations again */
	if (hardware) {
		if (eccstrength == 1) {
			err = omap_select_ecc_scheme(nand,
					OMAP_ECC_HAM1_CODE_HW,
					mtd->writesize, mtd->oobsize);
		} else if (eccstrength == 8) {
			err = omap_select_ecc_scheme(nand,
					OMAP_ECC_BCH8_CODE_HW,
					mtd->writesize, mtd->oobsize);
		} else if (eccstrength == 16) {
			err = omap_select_ecc_scheme(nand,
					OMAP_ECC_BCH16_CODE_HW,
					mtd->writesize, mtd->oobsize);
		} else {
			printf("nand: error: unsupported ECC scheme\n");
			return -EINVAL;
		}
	} else {
		if (eccstrength == 1) {
			err = omap_select_ecc_scheme(nand,
					OMAP_ECC_HAM1_CODE_SW,
					mtd->writesize, mtd->oobsize);
		} else if (eccstrength == 8) {
			err = omap_select_ecc_scheme(nand,
					OMAP_ECC_BCH8_CODE_HW_DETECTION_SW,
					mtd->writesize, mtd->oobsize);
		} else {
			printf("nand: error: unsupported ECC scheme\n");
			return -EINVAL;
		}
	}

	/* Update NAND handling after ECC mode switch */
	if (!err)
		err = nand_scan_tail(mtd);
	return err;
}
#endif /* CONFIG_SPL_BUILD */

/*
 * Board-specific NAND initialization. The following members of the
 * argument are board-specific:
 * - IO_ADDR_R: address to read the 8 I/O lines of the flash device
 * - IO_ADDR_W: address to write the 8 I/O lines of the flash device
 * - cmd_ctrl: hardwarespecific function for accesing control-lines
 * - waitfunc: hardwarespecific function for accesing device ready/busy line
 * - ecc.hwctl: function to enable (reset) hardware ecc generator
 * - ecc.mode: mode of ecc, see defines
 * - chip_delay: chip dependent delay for transfering data from array to
 *   read regs (tR)
 * - options: various chip options. They can partly be set to inform
 *   nand_scan about special functionality. See the defines for further
 *   explanation
 */
int gpmc_nand_init(struct nand_chip *nand, void __iomem *nand_base)
{
	int32_t gpmc_config = 0;
	int cs = cs_next++;
	int err = 0;
	struct omap_nand_info *info;

	/*
	 * xloader/Uboot's gpmc configuration would have configured GPMC for
	 * nand type of memory. The following logic scans and latches on to the
	 * first CS with NAND type memory.
	 * TBD: need to make this logic generic to handle multiple CS NAND
	 * devices.
	 */
	while (cs < GPMC_MAX_CS) {
		/* Check if NAND type is set */
		if ((readl(&gpmc_cfg->cs[cs].config1) & 0xC00) == 0x800) {
			/* Found it!! */
			break;
		}
		cs++;
	}
	if (cs >= GPMC_MAX_CS) {
		printf("nand: error: Unable to find NAND settings in "
			"GPMC Configuration - quitting\n");
		return -ENODEV;
	}

	gpmc_config = readl(&gpmc_cfg->config);
	/* Disable Write protect */
	gpmc_config |= 0x10;
	writel(gpmc_config, &gpmc_cfg->config);

	nand->IO_ADDR_R = (void __iomem *)&gpmc_cfg->cs[cs].nand_dat;
	nand->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_cmd;

	info = &omap_nand_info[cs];
	info->control = NULL;
	info->cs = cs;
	info->ws = wscfg[cs];
	info->fifo = nand_base;
	nand_set_controller_data(nand, &omap_nand_info[cs]);
	nand->cmd_ctrl	= omap_nand_hwcontrol;
	nand->options	|= NAND_NO_PADDING | NAND_CACHEPRG;
	nand->chip_delay = 100;
	nand->ecc.layout = kzalloc(sizeof(*nand->ecc.layout), GFP_KERNEL);
	if (!nand->ecc.layout)
		return -ENOMEM;

	/* configure driver and controller based on NAND device bus-width */
	gpmc_config = readl(&gpmc_cfg->cs[cs].config1);
#if defined(CONFIG_SYS_NAND_BUSWIDTH_16BIT)
	nand->options |= NAND_BUSWIDTH_16;
	writel(gpmc_config | (0x1 << 12), &gpmc_cfg->cs[cs].config1);
#else
	nand->options &= ~NAND_BUSWIDTH_16;
	writel(gpmc_config & ~(0x1 << 12), &gpmc_cfg->cs[cs].config1);
#endif
	/* select ECC scheme */
#if defined(CONFIG_NAND_OMAP_ECCSCHEME)
	err = omap_select_ecc_scheme(nand, CONFIG_NAND_OMAP_ECCSCHEME,
			CONFIG_SYS_NAND_PAGE_SIZE, CONFIG_SYS_NAND_OOBSIZE);
#else
	/* pagesize and oobsize are not required to configure sw ecc-scheme */
	err = omap_select_ecc_scheme(nand, OMAP_ECC_HAM1_CODE_SW,
			0, 0);
#endif
	if (err)
		return err;

#ifdef CONFIG_NAND_OMAP_GPMC_PREFETCH
	nand->read_buf = omap_nand_read_prefetch;
#else
	nand->read_buf = omap_nand_read_buf;
#endif

	nand->dev_ready = omap_dev_ready;

	return 0;
}

/* First NAND chip for SPL use only */
static __maybe_unused struct nand_chip *nand_chip;

#if CONFIG_IS_ENABLED(SYS_NAND_SELF_INIT)

static int gpmc_nand_probe(struct udevice *dev)
{
	struct nand_chip *nand = dev_get_priv(dev);
	struct mtd_info *mtd = nand_to_mtd(nand);
	struct resource res;
	void __iomem *base;
	int ret;

	ret = dev_read_resource(dev, 0, &res);
	if (ret)
		return ret;

	base = devm_ioremap(dev, res.start, resource_size(&res));
	gpmc_nand_init(nand, base);
	mtd->dev = dev;
	nand_set_flash_node(nand, dev_ofnode(dev));

	ret = nand_scan(mtd, CONFIG_SYS_NAND_MAX_CHIPS);
	if (ret)
		return ret;

	ret = nand_register(0, mtd);
	if (ret)
		return ret;

	if (!nand_chip)
		nand_chip = nand;

	return 0;
}

static const struct udevice_id gpmc_nand_ids[] = {
	{ .compatible = "ti,am64-nand" },
	{ .compatible = "ti,omap2-nand" },
	{ }
};

U_BOOT_DRIVER(gpmc_nand) = {
	.name           = "gpmc-nand",
	.id             = UCLASS_MTD,
	.of_match       = gpmc_nand_ids,
	.probe          = gpmc_nand_probe,
	.priv_auto	= sizeof(struct nand_chip),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

#ifdef CONFIG_NAND_OMAP_ELM
	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(gpmc_elm), &dev);
	if (ret && ret != -ENODEV) {
		pr_err("%s: Failed to get ELM device: %d\n", __func__, ret);
		return;
	}
#endif

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(gpmc_nand), &dev);
	if (ret && ret != -ENODEV)
		pr_err("%s: Failed to get GPMC device: %d\n", __func__, ret);
}

#else

int board_nand_init(struct nand_chip *nand)
{
	return gpmc_nand_init(nand, (void __iomem *)CFG_SYS_NAND_BASE);
}

#endif /* CONFIG_SYS_NAND_SELF_INIT */

#if defined(CONFIG_SPL_NAND_INIT)

/* nand_init() is provided by nand.c */

/* Unselect after operation */
void nand_deselect(void)
{
	struct mtd_info *mtd = nand_to_mtd(nand_chip);

	if (nand_chip->select_chip)
		nand_chip->select_chip(mtd, -1);
}

static int nand_is_bad_block(int block)
{
	struct mtd_info *mtd = nand_to_mtd(nand_chip);

	loff_t ofs = block * CONFIG_SYS_NAND_BLOCK_SIZE;

	return nand_chip->block_bad(mtd, ofs);
}

static int nand_read_page(int block, int page, uchar *dst)
{
	int page_addr = block * SYS_NAND_BLOCK_PAGES + page;
	loff_t ofs = page_addr * CONFIG_SYS_NAND_PAGE_SIZE;
	int ret;
	size_t len = CONFIG_SYS_NAND_PAGE_SIZE;
	struct mtd_info *mtd = nand_to_mtd(nand_chip);

	ret = nand_read(mtd, ofs, &len, dst);
	if (ret)
		printf("nand_read failed %d\n", ret);

	return ret;
}

#include "nand_spl_loaders.c"
#endif /* CONFIG_SPL_NAND_INIT */
