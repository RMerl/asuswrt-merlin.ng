/*
 * NAND Flash Driver
 *
 * Copyright (C) 2006-2014 Texas Instruments.
 *
 * Based on Linux DaVinci NAND driver by TI.
 */

#ifndef _DAVINCI_NAND_H_
#define _DAVINCI_NAND_H_

#include <linux/mtd/rawnand.h>
#include <asm/arch/hardware.h>

#define NAND_READ_START  	0x00
#define NAND_READ_END    	0x30
#define NAND_STATUS      	0x70

#define MASK_CLE		0x10
#define MASK_ALE		0x08

#ifdef CONFIG_SYS_NAND_MASK_CLE
#undef MASK_CLE
#define MASK_CLE CONFIG_SYS_NAND_MASK_CLE
#endif
#ifdef CONFIG_SYS_NAND_MASK_ALE
#undef MASK_ALE
#define MASK_ALE CONFIG_SYS_NAND_MASK_ALE
#endif

struct davinci_emif_regs {
	uint32_t	ercsr;
	uint32_t	awccr;
	uint32_t	sdbcr;
	uint32_t	sdrcr;
	union {
		uint32_t abncr[4];
		struct {
			uint32_t ab1cr;
			uint32_t ab2cr;
			uint32_t ab3cr;
			uint32_t ab4cr;
		};
	};
	uint32_t	sdtimr;
	uint32_t	ddrsr;
	uint32_t	ddrphycr;
	uint32_t	ddrphysr;
	uint32_t	totar;
	uint32_t	totactr;
	uint32_t	ddrphyid_rev;
	uint32_t	sdsretr;
	uint32_t	eirr;
	uint32_t	eimr;
	uint32_t	eimsr;
	uint32_t	eimcr;
	uint32_t	ioctrlr;
	uint32_t	iostatr;
	uint32_t	rsvd0;
	uint32_t	one_nand_cr;
	uint32_t	nandfcr;
	uint32_t	nandfsr;
	uint32_t	rsvd1[2];
	uint32_t	nandfecc[4];
	uint32_t	rsvd2[15];
	uint32_t	nand4biteccload;
	uint32_t	nand4bitecc[4];
	uint32_t	nanderradd1;
	uint32_t	nanderradd2;
	uint32_t	nanderrval1;
	uint32_t	nanderrval2;
};

#define davinci_emif_regs \
	((struct davinci_emif_regs *)DAVINCI_ASYNC_EMIF_CNTRL_BASE)

#define DAVINCI_NANDFCR_NAND_ENABLE(n)			(1 << ((n) - 2))
#define DAVINCI_NANDFCR_4BIT_ECC_SEL_MASK		(3 << 4)
#define DAVINCI_NANDFCR_4BIT_ECC_SEL(n)			(((n) - 2) << 4)
#define DAVINCI_NANDFCR_1BIT_ECC_START(n)		(1 << (8 + ((n) - 2)))
#define DAVINCI_NANDFCR_4BIT_ECC_START			(1 << 12)
#define DAVINCI_NANDFCR_4BIT_CALC_START			(1 << 13)
#define DAVINCI_NANDFCR_CS2NAND				(1 << 0)

/* Chip Select setup */
#define DAVINCI_ABCR_STROBE_SELECT			(1 << 31)
#define DAVINCI_ABCR_EXT_WAIT				(1 << 30)
#define DAVINCI_ABCR_WSETUP(n)				(n << 26)
#define DAVINCI_ABCR_WSTROBE(n)				(n << 20)
#define DAVINCI_ABCR_WHOLD(n)				(n << 17)
#define DAVINCI_ABCR_RSETUP(n)				(n << 13)
#define DAVINCI_ABCR_RSTROBE(n)				(n << 7)
#define DAVINCI_ABCR_RHOLD(n)				(n << 4)
#define DAVINCI_ABCR_TA(n)				(n << 2)
#define DAVINCI_ABCR_ASIZE_16BIT			1
#define DAVINCI_ABCR_ASIZE_8BIT				0

void davinci_nand_init(struct nand_chip *nand);

#endif
