// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard (void)
{
	puts ("Board: ");
	puts ("senTec COBRA5272 Board\n");
	return 0;
};

int dram_init(void)
{
	volatile sdramctrl_t *sdp = (sdramctrl_t *) (MMAP_SDRAM);

	sdp->sdram_sdtr = 0xf539;
	sdp->sdram_sdcr = 0x4211;

	/* Dummy write to start SDRAM */
	*((volatile unsigned long *) 0) = 0;

	gd->ram_size = CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;

	return 0;
};

int testdram (void)
{
	/* TODO: XXX XXX XXX */
	printf ("DRAM test not implemented!\n");

	return (0);
}
