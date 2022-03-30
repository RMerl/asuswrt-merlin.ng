// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * Hayden Fraser (Hayden.Fraser@freescale.com)
 */

#include <common.h>
#include <asm/immap.h>
#include <netdev.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	puts("Board: ");
	puts("Freescale MCF5253 DEMO\n");
	return 0;
};

int dram_init(void)
{
	u32 dramsize = 0;

	/*
	 * Check to see if the SDRAM has already been initialized
	 * by a run control tool
	 */
	if (!(mbar_readLong(MCFSIM_DCR) & 0x8000)) {
		u32 RC, temp;

		RC = (CONFIG_SYS_CLK / 1000000) >> 1;
		RC = (RC * 15) >> 4;

		/* Initialize DRAM Control Register: DCR */
		mbar_writeShort(MCFSIM_DCR, (0x8400 | RC));
		__asm__("nop");

		mbar_writeLong(MCFSIM_DACR0, 0x00003224);
		__asm__("nop");

		/* Initialize DMR0 */
		dramsize = (CONFIG_SYS_SDRAM_SIZE << 20);
		temp = (dramsize - 1) & 0xFFFC0000;
		mbar_writeLong(MCFSIM_DMR0, temp | 1);
		__asm__("nop");

		mbar_writeLong(MCFSIM_DACR0, 0x0000322c);
		mb();
		__asm__("nop");

		/* Write to this block to initiate precharge */
		*(u32 *) (CONFIG_SYS_SDRAM_BASE) = 0xa5a5a5a5;
		mb();
		__asm__("nop");

		/* Set RE bit in DACR */
		mbar_writeLong(MCFSIM_DACR0,
			       mbar_readLong(MCFSIM_DACR0) | 0x8000);
		__asm__("nop");

		/* Wait for at least 8 auto refresh cycles to occur */
		udelay(500);

		/* Finish the configuration by issuing the MRS */
		mbar_writeLong(MCFSIM_DACR0,
			       mbar_readLong(MCFSIM_DACR0) | 0x0040);
		__asm__("nop");

		*(u32 *) (CONFIG_SYS_SDRAM_BASE + 0x800) = 0xa5a5a5a5;
		mb();
	}

	gd->ram_size = dramsize;

	return 0;
}

int testdram(void)
{
	/* TODO: XXX XXX XXX */
	printf("DRAM test not implemented!\n");

	return (0);
}

#ifdef CONFIG_IDE
#include <ata.h>
int ide_preinit(void)
{
	return (0);
}

void ide_set_reset(int idereset)
{
	atac_t *ata = (atac_t *) CONFIG_SYS_ATA_BASE_ADDR;
	long period;
	/*  t1,  t2,  t3,  t4,  t5,  t6,  t9, tRD,  tA */
	int piotms[5][9] = { {70, 165, 60, 30, 50, 5, 20, 0, 35},	/* PIO 0 */
	{50, 125, 45, 20, 35, 5, 15, 0, 35},	/* PIO 1 */
	{30, 100, 30, 15, 20, 5, 10, 0, 35},	/* PIO 2 */
	{30, 80, 30, 10, 20, 5, 10, 0, 35},	/* PIO 3 */
	{25, 70, 20, 10, 20, 5, 10, 0, 35}	/* PIO 4 */
	};

	if (idereset) {
		/* control reset */
		out_8(&ata->cr, 0);
		udelay(100);
	} else {
		mbar2_writeLong(CIM_MISCCR, CIM_MISCCR_CPUEND);

#define CALC_TIMING(t) (t + period - 1) / period
		period = 1000000000 / (CONFIG_SYS_CLK / 2);	/* period in ns */

		/*ata->ton = CALC_TIMING (180); */
		out_8(&ata->t1, CALC_TIMING(piotms[2][0]));
		out_8(&ata->t2w, CALC_TIMING(piotms[2][1]));
		out_8(&ata->t2r, CALC_TIMING(piotms[2][1]));
		out_8(&ata->ta, CALC_TIMING(piotms[2][8]));
		out_8(&ata->trd, CALC_TIMING(piotms[2][7]));
		out_8(&ata->t4, CALC_TIMING(piotms[2][3]));
		out_8(&ata->t9, CALC_TIMING(piotms[2][6]));

		/* IORDY enable */
		out_8(&ata->cr, 0x40);
		udelay(2000);
		/* IORDY enable */
		setbits_8(&ata->cr, 0x01);
	}
}
#endif				/* CONFIG_IDE */


#ifdef CONFIG_DRIVER_DM9000
int board_eth_init(bd_t *bis)
{
	return dm9000_initialize(bis);
}
#endif
