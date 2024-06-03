// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <ns16550.h>
#include <asm/io.h>
#include <nand.h>
#include <asm/fsl_law.h>
#include <fsl_ddr_sdram.h>


const static u32 sysclk_tbl[] = {
	66666000, 7499900, 83332500, 8999900,
	99999000, 11111000, 12499800, 13333200
};

void board_init_f(ulong bootflag)
{
	int px_spd;
	u32 plat_ratio, sys_clk, bus_clk;
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

#if defined(CONFIG_SYS_NAND_BR_PRELIM) && defined(CONFIG_SYS_NAND_OR_PRELIM)
	set_lbc_br(0, CONFIG_SYS_NAND_BR_PRELIM);
	set_lbc_or(0, CONFIG_SYS_NAND_OR_PRELIM);
#endif
	/* for FPGA */
	set_lbc_br(2, CONFIG_SYS_BR2_PRELIM);
	set_lbc_or(2, CONFIG_SYS_OR2_PRELIM);

	/* initialize selected port with appropriate baud rate */
	px_spd = in_8((unsigned char *)(PIXIS_BASE + PIXIS_SPD));
	sys_clk = sysclk_tbl[px_spd & PIXIS_SPD_SYSCLK_MASK];
	plat_ratio = in_be32(&gur->porpllsr) & MPC85xx_PORPLLSR_PLAT_RATIO;
	bus_clk = sys_clk * plat_ratio / 2;

	NS16550_init((NS16550_t)CONFIG_SYS_NS16550_COM1,
			bus_clk / 16 / CONFIG_BAUDRATE);

	puts("\nNAND boot... ");

	/* copy code to RAM and jump to it - this should not return */
	/* NOTE - code has to be copied out of NAND buffer before
	 * other blocks can be read.
	 */
	relocate_code(CONFIG_SPL_RELOC_STACK, 0,
			CONFIG_SPL_RELOC_TEXT_BASE);
}

void board_init_r(gd_t *gd, ulong dest_addr)
{
	puts("\nSecond program loader running in sram...");
	nand_boot();
}

void putc(char c)
{
	if (c == '\n')
		NS16550_putc((NS16550_t)CONFIG_SYS_NS16550_COM1, '\r');

	NS16550_putc((NS16550_t)CONFIG_SYS_NS16550_COM1, c);
}

void puts(const char *str)
{
	while (*str)
		putc(*str++);
}
