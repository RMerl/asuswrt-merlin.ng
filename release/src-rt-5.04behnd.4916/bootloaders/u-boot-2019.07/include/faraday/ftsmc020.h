/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 */

/*
 * Static Memory Controller
 */
#ifndef __FTSMC020_H
#define __FTSMC020_H

#ifndef __ASSEMBLY__

struct ftsmc020_bank {
	unsigned int    cr;
	unsigned int    tpr;
};

struct ftsmc020 {
	struct ftsmc020_bank bank[4];	/* 0x00 - 0x1c */
	unsigned int	pad[8];		/* 0x20 - 0x3c */
	unsigned int	ssr;		/* 0x40 */
};

void ftsmc020_init(void);

#endif /* __ASSEMBLY__ */

/*
 * Memory Bank Configuration Register
 */
#define FTSMC020_BANK_ENABLE	(1 << 28)
#define FTSMC020_BANK_BASE(x)	((x) & 0x0fff1000)

#define FTSMC020_BANK_WPROT	(1 << 11)

#define FTSMC020_BANK_TYPE1	(1 << 10)
#define FTSMC020_BANK_TYPE2	(1 << 9)
#define FTSMC020_BANK_TYPE3	(1 << 8)

#define FTSMC020_BANK_SIZE_32K	(0xb << 4)
#define FTSMC020_BANK_SIZE_64K	(0xc << 4)
#define FTSMC020_BANK_SIZE_128K	(0xd << 4)
#define FTSMC020_BANK_SIZE_256K	(0xe << 4)
#define FTSMC020_BANK_SIZE_512K	(0xf << 4)
#define FTSMC020_BANK_SIZE_1M	(0x0 << 4)
#define FTSMC020_BANK_SIZE_2M	(0x1 << 4)
#define FTSMC020_BANK_SIZE_4M	(0x2 << 4)
#define FTSMC020_BANK_SIZE_8M	(0x3 << 4)
#define FTSMC020_BANK_SIZE_16M	(0x4 << 4)
#define FTSMC020_BANK_SIZE_32M	(0x5 << 4)
#define FTSMC020_BANK_SIZE_64M	(0x6 << 4)

#define FTSMC020_BANK_MBW_8	(0x0 << 0)
#define FTSMC020_BANK_MBW_16	(0x1 << 0)
#define FTSMC020_BANK_MBW_32	(0x2 << 0)

/*
 * Memory Bank Timing Parameter Register
 */
#define FTSMC020_TPR_ETRNA(x)	(((x) & 0xf) << 28)
#define FTSMC020_TPR_EATI(x)	(((x) & 0xf) << 24)
#define FTSMC020_TPR_RBE	(1 << 20)
#define FTSMC020_TPR_AST(x)	(((x) & 0x3) << 18)
#define FTSMC020_TPR_CTW(x)	(((x) & 0x3) << 16)
#define FTSMC020_TPR_ATI(x)	(((x) & 0xf) << 12)
#define FTSMC020_TPR_AT2(x)	(((x) & 0x3) << 8)
#define FTSMC020_TPR_WTC(x)	(((x) & 0x3) << 6)
#define FTSMC020_TPR_AHT(x)	(((x) & 0x3) << 4)
#define FTSMC020_TPR_TRNA(x)	(((x) & 0xf) << 0)

#endif	/* __FTSMC020_H */
