// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <command.h>
#include <malloc.h>
#include <asm/immap.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard (void) {
	ulong val;
	uchar val8;

	puts ("Board: ");
	puts("Freescale M5249EVB");
	val8 = ((uchar)~((uchar)mbar2_readLong(MCFSIM_GPIO1_READ) >> 4)) & 0xf;
	printf(" (Switch=%1X)\n", val8);

	/*
	 * Set LED on
	 */
	val = mbar2_readLong(MCFSIM_GPIO1_OUT) & ~CONFIG_SYS_GPIO1_LED;
	mbar2_writeLong(MCFSIM_GPIO1_OUT, val);   /* Set LED on */

	return 0;
};


int dram_init(void)
{
	unsigned long	junk = 0xa5a59696;

	/*
	 *  Note:
	 *	RC = ([(RefreshTime/#rows) / (1/BusClk)] / 16) - 1
	 */

#ifdef CONFIG_SYS_FAST_CLK
	/*
	 * Busclk=70MHz, RefreshTime=64ms, #rows=4096 (4K)
	 * SO=1, NAM=0, COC=0, RTIM=01 (6clk refresh), RC=39
	 */
	mbar_writeShort(MCFSIM_DCR, 0x8239);
#elif CONFIG_SYS_PLL_BYPASS
	/*
	 * Busclk=5.6448MHz, RefreshTime=64ms, #rows=8192 (8K)
	 * SO=1, NAM=0, COC=0, RTIM=01 (6clk refresh), RC=02
	 */
	mbar_writeShort(MCFSIM_DCR, 0x8202);
#else
	/*
	 * Busclk=36MHz, RefreshTime=64ms, #rows=4096 (4K)
	 * SO=1, NAM=0, COC=0, RTIM=01 (6clk refresh), RC=22 (562 bus clock cycles)
	 */
	mbar_writeShort(MCFSIM_DCR, 0x8222);
#endif

	/*
	 * SDRAM starts at 0x0000_0000, CASL=10, CBM=010, PS=10 (16bit port),
	 * PM=1 (continuous page mode)
	 */

	/* RE=0 (keep auto-refresh disabled while setting up registers) */
	mbar_writeLong(MCFSIM_DACR0, 0x00003324);

	/* BAM=007c (bits 22,21 are bank selects; 256kB blocks) */
	mbar_writeLong(MCFSIM_DMR0, 0x01fc0001);

	/** Precharge sequence **/
	mbar_writeLong(MCFSIM_DACR0, 0x0000332c); /* Set DACR0[IP] (bit 3) */
	*((volatile unsigned long *) 0x00) = junk; /* write to a memory location to init. precharge */
	udelay(0x10); /* Allow several Precharge cycles */

	/** Refresh Sequence **/
	mbar_writeLong(MCFSIM_DACR0, 0x0000b324); /* Enable the refresh bit, DACR0[RE] (bit 15) */
	udelay(0x7d0); /* Allow gobs of refresh cycles */

	/** Mode Register initialization **/
	mbar_writeLong(MCFSIM_DACR0, 0x0000b364);  /* Enable DACR0[IMRS] (bit 6); RE remains enabled */
	*((volatile unsigned long *) 0x800) = junk; /* Access RAM to initialize the mode register */

	gd->ram_size = CONFIG_SYS_SDRAM_SIZE * 1024 * 1024;

	return 0;
};


int testdram (void) {
	/* TODO: XXX XXX XXX */
	printf ("DRAM test not implemented!\n");

	return (0);
}
