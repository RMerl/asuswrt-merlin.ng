// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <console.h>
#include <environment.h>
#include <ns16550.h>
#include <malloc.h>
#include <mmc.h>
#include <nand.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

phys_size_t get_effective_memsize(void)
{
	return CONFIG_SYS_L2_SIZE;
}

void board_init_f(ulong bootflag)
{
	u32 plat_ratio;
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	console_init_f();

	/* initialize selected port with appropriate baud rate */
	plat_ratio = in_be32(&gur->porpllsr) & MPC85xx_PORPLLSR_PLAT_RATIO;
	plat_ratio >>= 1;
	gd->bus_clk = CONFIG_SYS_CLK_FREQ * plat_ratio;

	NS16550_init((NS16550_t)CONFIG_SYS_NS16550_COM1,
		     gd->bus_clk / 16 / CONFIG_BAUDRATE);

	/* copy code to RAM and jump to it - this should not return */
	/* NOTE - code has to be copied out of NAND buffer before
	 * other blocks can be read.
	 */
	relocate_code(CONFIG_SPL_RELOC_STACK, 0, CONFIG_SPL_RELOC_TEXT_BASE);
}

void board_init_r(gd_t *gd, ulong dest_addr)
{
	/* Pointer is writable since we allocated a register for it */
	gd = (gd_t *)CONFIG_SPL_GD_ADDR;
	bd_t *bd;

	memset(gd, 0, sizeof(gd_t));
	bd = (bd_t *)(CONFIG_SPL_GD_ADDR + sizeof(gd_t));
	memset(bd, 0, sizeof(bd_t));
	gd->bd = bd;
	bd->bi_memstart = CONFIG_SYS_INIT_L2_ADDR;
	bd->bi_memsize = CONFIG_SYS_L2_SIZE;

	arch_cpu_init();
	get_clocks();
	mem_malloc_init(CONFIG_SPL_RELOC_MALLOC_ADDR,
			CONFIG_SPL_RELOC_MALLOC_SIZE);
	gd->flags |= GD_FLG_FULL_MALLOC_INIT;

	/* relocate environment function pointers etc. */
	nand_spl_load_image(CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE,
			    (uchar *)CONFIG_ENV_ADDR);
	gd->env_addr  = (ulong)(CONFIG_ENV_ADDR);
	gd->env_valid = ENV_VALID;

	i2c_init_all();

	dram_init();

#ifdef CONFIG_SPL_NAND_BOOT
	puts("TPL\n");
#else
	puts("SPL\n");
#endif

	nand_boot();
}
