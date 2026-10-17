// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 * Copyright (C) 2008 Ronetix Ilko Iliev (www.ronetix.at)
 * Copyright (C) 2009 Jean-Christophe PLAGNIOL-VILLARD <plagnioj@jcrosoft.com>
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <linux/sizes.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_matrix.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscellaneous platform dependent initializations
 */

#ifdef CONFIG_CMD_NAND
static void pm9263_nand_hw_init(void)
{
	unsigned long csa;
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC0;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;

	/* Enable CS3 */
	csa = readl(&matrix->csa[0]) | AT91_MATRIX_CSA_EBI_CS3A;
	writel(csa, &matrix->csa[0]);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(1) |
		AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(1),
		&smc->cs[3].setup);

	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(3) |
		AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(3),
		&smc->cs[3].pulse);

	writel(AT91_SMC_CYCLE_NWE(5) | AT91_SMC_CYCLE_NRD(5),
		&smc->cs[3].cycle);

	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
		AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
		AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
		AT91_SMC_MODE_DBW_8 |
#endif
		AT91_SMC_MODE_TDF_CYCLE(2),
		&smc->cs[3].mode);

	/* Configure RDY/BSY */
	gpio_direction_input(CFG_SYS_NAND_READY_PIN);

	/* Enable NandFlash */
	gpio_direction_output(CFG_SYS_NAND_ENABLE_PIN, 1);
}
#endif

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	/* arch number of PM9263 Board */
	gd->bd->bi_arch_number = MACH_TYPE_PM9263;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_CMD_NAND
	pm9263_nand_hw_init();
#endif
#ifdef CONFIG_USB_OHCI_NEW
	at91_uhp_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	/* dram_init must store complete RAM size in gd->ram_size */
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM,
				PHYS_SDRAM_SIZE);
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard (void)
{
	char *ss;

	printf ("Board : Ronetix PM9263\n");

	switch (gd->fb_base) {
	case PHYS_PSRAM:
		ss = "(PSRAM)";
		break;

	case ATMEL_BASE_SRAM0:
		ss = "(Internal SRAM)";
		break;

	default:
		ss = "";
		break;
	}
	printf("Video memory : 0x%08lX %s\n", gd->fb_base, ss );

	printf ("\n");
	return 0;
}
#endif
