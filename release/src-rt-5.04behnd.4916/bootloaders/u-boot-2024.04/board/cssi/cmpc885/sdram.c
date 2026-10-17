// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 CS Group
 * Charles Frey <charles.frey@c-s.fr>
 */

#include <config.h>
#include <linux/sizes.h>
#include <linux/delay.h>
#include <init.h>
#include <asm/io.h>
#include <mpc8xx.h>
#include <watchdog.h>
#include <asm/ppc.h>
#include <asm/immap_8xx.h>

DECLARE_GLOBAL_DATA_PTR;

#define ADDR_CPLD_R_TYPE	((unsigned char __iomem *)CONFIG_CPLD_BASE + 3)

#define _NOT_USED_  0xFFFFEC04

static const uint sdram_table[] = {
	/* DRAM - single read. (offset 0 in upm RAM) */
	0x0F0CEC04, 0x0FFFEC04, 0x00AF2C04, 0x0FFFEC00,
	0x0FFCE004, 0xFFFFEC05, _NOT_USED_, _NOT_USED_,

	/* DRAM - burst read. (offset 8 in upm RAM) */
	0x0F0CEC04, 0x0FFFEC04, 0x00AF2C04, 0x00FFEC00,
	0x00FFEC00, 0x00FFEC00, 0x0FFCE000, 0x1FFFEC05,

	/* DRAM - Precharge all banks. (offset 16 in upm RAM) */
	_NOT_USED_, 0x0FFCE004, 0x1FFFEC05, _NOT_USED_,

	/* DRAM - NOP. (offset 20 in upm RAM) */
	0x1FFFEC05, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* DRAM - single write. (offset 24 in upm RAM) */
	0x0F0CEC04, 0x0FFFEC00, 0x00AF2004, 0x0FFFEC04,
	0x0FFCE004, 0x0FFFEC04, 0xFFFFEC05, _NOT_USED_,

	/* DRAM - burst write. (offset 32 in upm RAM) */
	0x0F0CEC04, 0x0FFFEC00, 0x00AF2000, 0x00FFEC00,
	0x00FFEC00, 0x00FFEC04, 0x0FFFEC04, 0x0FFCE004,
	0x1FFFEC05, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* refresh  (offset 48 in upm RAM) */
	0x0FFDE404, 0x0FFEAC04, 0x0FFD6C84, 0x0FFFEC04,
	0x0FFFEC04, 0x0FFFEC04, 0x0FFFEC04, 0x1FFFEC85,

	/* init (offset 56 in upm RAM) */
	0x0FEEA874, 0x0FBD6474, 0x1FFFEC45, _NOT_USED_,

	/* exception. (offset 60 in upm RAM) */
	0x0FFCE004, 0xFFFFEC05, _NOT_USED_, _NOT_USED_
};

/* SDRAM initialization */
int dram_init(void)
{
	immap_t __iomem *immap = (immap_t __iomem *)CONFIG_SYS_IMMR;
	memctl8xx_t __iomem *memctl = &immap->im_memctl;
	u32 max_size, mamr;
	u8 val;

	printf("UPMA init for SDRAM (CAS latency 2), ");
	printf("init address 0x%08x, size ", (int)dram_init);

	/* Verify the SDRAM size of the board */
	val = (in_8(ADDR_CPLD_R_TYPE) & 0x30) >> 4;

	if (val == 0x03 || val == 0x00) {
		max_size	= 64	* SZ_1M;	/* 64 Mo of SDRAM */
		mamr		= 0x20104000;
	} else {
		max_size	= 128	* SZ_1M;	/* 128 Mo of SDRAM */
		mamr		= 0x20206000;
	}

	/* Configure CS1 */
	out_be32(&memctl->memc_or1,
		 ~(max_size - 1) | OR_CSNT_SAM | OR_ACS_DIV2);
	out_be32(&memctl->memc_br1, CFG_SYS_SDRAM_BASE | BR_MS_UPMA | BR_V);

	/* Configure UPMA for CS1 */
	upmconfig(UPMA, (uint *)sdram_table, ARRAY_SIZE(sdram_table));

	out_be16(&memctl->memc_mptpr, MPTPR_PTP_DIV32);
	/* disable refresh */
	out_be32(&memctl->memc_mamr, mamr);
	udelay(100);

	/* NOP to maintain DQM high */
	out_be32(&memctl->memc_mcr, 0x80002114);
	udelay(200);

	out_be32(&memctl->memc_mcr, 0x80002111); /* PRECHARGE cmd */
	out_be32(&memctl->memc_mcr, 0x80002830); /* AUTO REFRESH cmd */
	out_be32(&memctl->memc_mar, 0x00000088);
	out_be32(&memctl->memc_mcr, 0x80002138);

	/* Enable refresh */
	setbits_be32(&memctl->memc_mamr, MAMR_PTAE);

	gd->ram_size = get_ram_size((long *)CFG_SYS_SDRAM_BASE, max_size);

	return 0;
}
