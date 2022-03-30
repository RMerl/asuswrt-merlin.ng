// SPDX-License-Identifier: GPL-2.0+
/*
 * Board functions for Sysam AMCORE (MCF5307 based) board
 *
 * (C) Copyright 2016  Angelo Dureghello <angelo@sysam.it>
 *
 * This file copies memory testdram() from sandburst/common/sb_common.c
 */

#include <common.h>
#include <asm/immap.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/platform_data/serial_coldfire.h>

DECLARE_GLOBAL_DATA_PTR;

void init_lcd(void)
{
	/* setup for possible K0108 lcd connected on the parallel port */
	sim_t *sim = (sim_t *)(MMAP_SIM);

	out_be16(&sim->par, 0x300);

	gpio_t *gpio = (gpio_t *)(MMAP_GPIO);

	out_be16(&gpio->paddr, 0xfcff);
	out_be16(&gpio->padat, 0x0c00);
}

int checkboard(void)
{
	puts("Board: ");
	puts("AMCORE v.001(alpha)\n");

	init_lcd();

	return 0;
}

/*
 * in dram_init we are here executing from flash
 * case 1:
 * is with no ACR/flash cache enabled
 * nop = 40ns (scope measured)
 */
void fudelay(int usec)
{
	while (usec--)
		asm volatile ("nop");
}

int dram_init(void)
{
	u32 dramsize, RC;

	sdramctrl_t *dc = (sdramctrl_t *)(MMAP_DRAMC);

	/*
	 * SDRAM  MT48LC4M32B2 details
	 * Memory block 0: 16 MB of SDRAM at address $00000000
	 * Port size: 32-bit port
	 *
	 * Memory block 0 wired as follows:
	 * CPU   : A15 A14 A13 A12 A11 A10 A9 A17 A18 A19 A20 A21 A22 A23
	 * SDRAM :  A0  A1  A2  A3  A4  A5  A6 A7  A8  A9 A10 A11 BA0 BA1
	 *
	 * Ensure that there is a delay of at least 100 microseconds from
	 * processor reset to the following code so that the SDRAM is ready
	 * for commands.
	 */
	fudelay(100);

	/*
	 * DCR
	 * set proper  RC as per specification
	 */
	RC = (CONFIG_SYS_CPU_CLK / 1000000) >> 1;
	RC = (RC * 15) >> 4;

	/* 0x8000 is the faster option */
	out_be16(&dc->dcr, 0x8200 | RC);

	/*
	 * DACR0, page mode continuous, CMD on A20 0x0300
	 */
	out_be32(&dc->dacr0, 0x00003304);

	dramsize = ((CONFIG_SYS_SDRAM_SIZE)-1) & 0xfffc0000;
	out_be32(&dc->dmr0,  dramsize|1);

	/* issue a PRECHARGE ALL */
	out_be32(&dc->dacr0, 0x0000330c);
	out_be32((u32 *)0x00000004, 0xbeaddeed);
	/* issue AUTOREFRESH */
	out_be32(&dc->dacr0, 0x0000b304);
	/* let refresh occur */
	fudelay(1);

	out_be32(&dc->dacr0, 0x0000b344);
	out_be32((u32 *)0x00000c00, 0xbeaddeed);

	gd->ram_size = get_ram_size(CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);

	return 0;
}

static struct coldfire_serial_platdata mcf5307_serial_plat = {
	.base = CONFIG_SYS_UART_BASE,
	.port = 0,
	.baudrate = CONFIG_BAUDRATE,
};

U_BOOT_DEVICE(coldfire_serial) = {
	.name = "serial_coldfire",
	.platdata = &mcf5307_serial_plat,
};
