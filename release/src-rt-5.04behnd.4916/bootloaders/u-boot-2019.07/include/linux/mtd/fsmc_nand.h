/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#ifndef __FSMC_NAND_H__
#define __FSMC_NAND_H__

#include <linux/mtd/rawnand.h>

struct fsmc_regs {
	u32 ctrl;			/* 0x00 */
	u8 reserved_1[0x40 - 0x04];
	u32 pc;				/* 0x40 */
	u32 sts;			/* 0x44 */
	u32 comm;			/* 0x48 */
	u32 attrib;			/* 0x4c */
	u32 ioata;			/* 0x50 */
	u32 ecc1;			/* 0x54 */
	u32 ecc2;			/* 0x58 */
	u32 ecc3;			/* 0x5c */
	u8 reserved_2[0xfe0 - 0x60];
	u32 peripid0;			/* 0xfe0 */
	u32 peripid1;			/* 0xfe4 */
	u32 peripid2;			/* 0xfe8 */
	u32 peripid3;			/* 0xfec */
	u32 pcellid0;			/* 0xff0 */
	u32 pcellid1;			/* 0xff4 */
	u32 pcellid2;			/* 0xff8 */
	u32 pcellid3;			/* 0xffc */
};

/* ctrl register definitions */
#define FSMC_WP			(1 << 7)

/* pc register definitions */
#define FSMC_RESET		(1 << 0)
#define FSMC_WAITON		(1 << 1)
#define FSMC_ENABLE		(1 << 2)
#define FSMC_DEVTYPE_NAND	(1 << 3)
#define FSMC_DEVWID_8		(0 << 4)
#define FSMC_DEVWID_16		(1 << 4)
#define FSMC_ECCEN		(1 << 6)
#define FSMC_ECCPLEN_512	(0 << 7)
#define FSMC_ECCPLEN_256	(1 << 7)
#define FSMC_TCLR_1		(1 << 9)
#define FSMC_TAR_1		(1 << 13)

/* sts register definitions */
#define FSMC_CODE_RDY		(1 << 15)

/* comm register definitions */
#define FSMC_TSET_0		(0 << 0)
#define FSMC_TWAIT_6		(6 << 8)
#define FSMC_THOLD_4		(4 << 16)
#define FSMC_THIZ_1		(1 << 24)

/* peripid2 register definitions */
#define FSMC_REVISION_MSK	(0xf)
#define FSMC_REVISION_SHFT	(0x4)

#define FSMC_VER8		0x8

/*
 * There are 13 bytes of ecc for every 512 byte block and it has to be read
 * consecutively and immediately after the 512 byte data block for hardware to
 * generate the error bit offsets
 * Managing the ecc bytes in the following way is easier. This way is similar to
 * oobfree structure maintained already in u-boot nand driver
 */
#define FSMC_MAX_ECCPLACE_ENTRIES	32

struct fsmc_nand_eccplace {
	u32 offset;
	u32 length;
};

struct fsmc_eccplace {
	struct fsmc_nand_eccplace eccplace[FSMC_MAX_ECCPLACE_ENTRIES];
};

extern int fsmc_nand_init(struct nand_chip *nand);
#endif
