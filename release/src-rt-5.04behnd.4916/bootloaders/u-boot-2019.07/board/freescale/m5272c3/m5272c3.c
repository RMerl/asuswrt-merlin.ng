// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2012 Freescale Semiconductor, Inc. All Rights Reserved.
 */

#include <common.h>
#include <asm/immap.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard (void) {
	puts ("Board: ");
	puts ("Freescale MCF5272C3 EVB\n");
	return 0;
	};

int dram_init(void)
{
	sdramctrl_t * sdp = (sdramctrl_t *)(MMAP_SDRAM);

	out_be16(&sdp->sdram_sdtr, 0xf539);
	out_be16(&sdp->sdram_sdcr, 0x4211);

	/* Dummy write to start SDRAM */
	*((volatile unsigned long *)0) = 0;

	gd->ram_size = CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;

	return 0;
	};

int testdram (void) {
	/* TODO: XXX XXX XXX */
	printf ("DRAM test not implemented!\n");

	return (0);
}
