// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <clock_legacy.h>
#include <console.h>
#include <env_internal.h>
#include <init.h>
#include <malloc.h>
#include <ns16550.h>
#include <nand.h>
#include <i2c.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <spi_flash.h>
#include <asm/global_data.h>
#include "../common/sleep.h"
#include "../common/spl.h"

DECLARE_GLOBAL_DATA_PTR;

phys_size_t get_effective_memsize(void)
{
	return CONFIG_SYS_L3_SIZE;
}

#define FSL_CORENET_CCSR_PORSR1_RCW_MASK	0xFF800000
void board_init_f(ulong bootflag)
{
	u32 plat_ratio, sys_clk, uart_clk;
#if defined(CONFIG_SPL_NAND_BOOT) && defined(CONFIG_A008044_WORKAROUND)
	u32 porsr1, pinctl;
	u32 svr = get_svr();
#endif
	ccsr_gur_t *gur = (void *)CFG_SYS_MPC85xx_GUTS_ADDR;

#if defined(CONFIG_SPL_NAND_BOOT) && defined(CONFIG_A008044_WORKAROUND)
	if (IS_SVR_REV(svr, 1, 0)) {
		/*
		 * There is T1040 SoC issue where NOR, FPGA are inaccessible
		 * during NAND boot because IFC signals > IFC_AD7 are not
		 * enabled. This workaround changes RCW source to make all
		 * signals enabled.
		 */
		porsr1 = in_be32(&gur->porsr1);
		pinctl = ((porsr1 & ~(FSL_CORENET_CCSR_PORSR1_RCW_MASK))
			  | 0x24800000);
		out_be32((unsigned int *)(CFG_SYS_DCSRBAR + 0x20000),
			 pinctl);
	}
#endif

	/* Memcpy existing GD at CONFIG_SPL_GD_ADDR */
	memcpy((void *)CONFIG_SPL_GD_ADDR, (void *)gd, sizeof(gd_t));

	/* Update GD pointer */
	gd = (gd_t *)(CONFIG_SPL_GD_ADDR);

#ifdef CONFIG_DEEP_SLEEP
	/* disable the console if boot from deep sleep */
	if (is_warm_boot())
		fsl_dp_disable_console();
#endif
	/* compiler optimization barrier needed for GCC >= 3.4 */
	__asm__ __volatile__("" : : : "memory");

	console_init_f();

	/* initialize selected port with appropriate baud rate */
	sys_clk = get_board_sys_clk();
	plat_ratio = (in_be32(&gur->rcwsr[0]) >> 25) & 0x1f;
	uart_clk = sys_clk * plat_ratio / 2;

	ns16550_init((struct ns16550 *)CFG_SYS_NS16550_COM1,
		     uart_clk / 16 / CONFIG_BAUDRATE);

	relocate_code(CONFIG_SPL_RELOC_STACK, (gd_t *)CONFIG_SPL_GD_ADDR, 0x0);
}

void board_init_r(gd_t *gd, ulong dest_addr)
{
	struct bd_info *bd;

	bd = (struct bd_info *)(gd + sizeof(gd_t));
	memset(bd, 0, sizeof(struct bd_info));
	gd->bd = bd;

	arch_cpu_init();
	get_clocks();
	mem_malloc_init(CONFIG_SPL_RELOC_MALLOC_ADDR,
			CONFIG_SPL_RELOC_MALLOC_SIZE);
	gd->flags |= GD_FLG_FULL_MALLOC_INIT;

#ifdef CONFIG_SPL_MMC_BOOT
	mmc_initialize(bd);
#endif

	/* relocate environment function pointers etc. */
#if defined(CONFIG_ENV_IS_IN_NAND) || defined(CONFIG_ENV_IS_IN_MMC) || \
	defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#ifdef CONFIG_SPL_NAND_BOOT
	nand_spl_load_image(CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE,
			    (uchar *)SPL_ENV_ADDR);
#endif
#ifdef CONFIG_SPL_MMC_BOOT
	mmc_spl_load_image(CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE,
			   (uchar *)SPL_ENV_ADDR);
#endif
#ifdef CONFIG_SPL_SPI_BOOT
	fsl_spi_spl_load_image(CONFIG_ENV_OFFSET, CONFIG_ENV_SIZE,
			       (uchar *)SPL_ENV_ADDR);
#endif
	gd->env_addr  = (ulong)(SPL_ENV_ADDR);
	gd->env_valid = ENV_VALID;
#endif

	i2c_init_all();

	puts("\n\n");

	dram_init();

#ifdef CONFIG_SPL_MMC_BOOT
	mmc_boot();
#elif defined(CONFIG_SPL_SPI_BOOT)
	fsl_spi_boot();
#elif defined(CONFIG_SPL_NAND_BOOT)
	nand_boot();
#endif
}
