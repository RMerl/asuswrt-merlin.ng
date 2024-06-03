// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010 Andreas Bießmann <andreas@biessmann.org>
 *
 * derived from previous work
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 */

#include <common.h>
#include <netdev.h>
#include <asm/mach-types.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pio.h>
#include <asm/arch/at91_common.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
int board_init(void)
{
	at91_pio_t *pio = (at91_pio_t *)AT91_PIO_BASE;

	/*
	 * Correct IRDA resistor problem
	 * Set PA23_TXD in Output
	 */
	writel(ATMEL_PMX_AA_TXD2, &pio->pioa.oer);

	/* arch number of AT91RM9200EK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91RM9200EK;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int board_early_init_f(void)
{
	at91_seriald_hw_init();
	return 0;
}

int dram_init (void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
			CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#ifdef CONFIG_DRIVER_AT91EMAC
int board_eth_init(bd_t *bis)
{
	return at91emac_register(bis, (u32) ATMEL_BASE_EMAC);
}
#endif
