// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <console.h>
#include <environment.h>
#include <asm/spl.h>
#include <malloc.h>
#include <ns16550.h>
#include <nand.h>
#include <i2c.h>
#include "../common/qixis.h"
#include "b4860qds_qixis.h"

DECLARE_GLOBAL_DATA_PTR;

phys_size_t get_effective_memsize(void)
{
	return CONFIG_SYS_L3_SIZE;
}

unsigned long get_board_sys_clk(void)
{
	u8 sysclk_conf = QIXIS_READ(brdcfg[1]);

	switch ((sysclk_conf & 0x0C) >> 2) {
	case QIXIS_CLK_100:
		return 100000000;
	case QIXIS_CLK_125:
		return 125000000;
	case QIXIS_CLK_133:
		return 133333333;
	}
	return 66666666;
}

unsigned long get_board_ddr_clk(void)
{
	u8 ddrclk_conf = QIXIS_READ(brdcfg[1]);

	switch (ddrclk_conf & 0x03) {
	case QIXIS_CLK_100:
		return 100000000;
	case QIXIS_CLK_125:
		return 125000000;
	case QIXIS_CLK_133:
		return 133333333;
	}
	return 66666666;
}

void board_init_f(ulong bootflag)
{
	u32 plat_ratio, sys_clk, uart_clk;
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;

	/* Memcpy existing GD at CONFIG_SPL_GD_ADDR */
	memcpy((void *)CONFIG_SPL_GD_ADDR, (void *)gd, sizeof(gd_t));

	/* Update GD pointer */
	gd = (gd_t *)(CONFIG_SPL_GD_ADDR);

	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("" : : : "memory");

	console_init_f();

	/* initialize selected port with appropriate baud rate */
	sys_clk = get_board_sys_clk();
	plat_ratio = (in_be32(&gur->rcwsr[0]) >> 25) & 0x1f;
	uart_clk = sys_clk * plat_ratio / 2;

	NS16550_init((NS16550_t)CONFIG_SYS_NS16550_COM1,
		     uart_clk / 16 / CONFIG_BAUDRATE);

	relocate_code(CONFIG_SPL_RELOC_STACK, (gd_t *)CONFIG_SPL_GD_ADDR, 0x0);
}

void board_init_r(gd_t *gd, ulong dest_addr)
{
	bd_t *bd;

	bd = (bd_t *)(gd + sizeof(gd_t));
	memset(bd, 0, sizeof(bd_t));
	gd->bd = bd;
	bd->bi_memstart = CONFIG_SYS_INIT_L3_ADDR;
	bd->bi_memsize = CONFIG_SYS_L3_SIZE;

	arch_cpu_init();
	get_clocks();
	mem_malloc_init(CONFIG_SPL_RELOC_MALLOC_ADDR,
			CONFIG_SPL_RELOC_MALLOC_SIZE);
	gd->flags |= GD_FLG_FULL_MALLOC_INIT;

#ifndef CONFIG_SPL_NAND_BOOT
	env_init();
	env_relocate();
#else
	/* relocate environment function pointers etc. */
	nand_spl_load_image(CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE,
			    (uchar *)CONFIG_ENV_ADDR);
	gd->env_addr  = (ulong)(CONFIG_ENV_ADDR);
	gd->env_valid = ENV_VALID;
#endif

	i2c_init_all();

	puts("\n\n");

	dram_init();

#ifdef CONFIG_SPL_NAND_BOOT
	nand_boot();
#endif
}
