// SPDX-License-Identifier: GPL-2.0+
/*
 * Bluewater Systems Snapper 9260/9G20 modules
 *
 * (C) Copyright 2011 Bluewater Systems
 *   Author: Andre Renaud <andre@bluewatersys.com>
 *   Author: Ryan Mallon <ryan@bluewatersys.com>
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/mach-types.h>
#include <asm/arch/at91sam9260_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/atmel_serial.h>
#include <net.h>
#include <netdev.h>
#include <i2c.h>
#include <pca953x.h>

DECLARE_GLOBAL_DATA_PTR;

/* IO Expander pins */
#define IO_EXP_ETH_RESET	(0 << 1)
#define IO_EXP_ETH_POWER	(1 << 1)

static void macb_hw_init(void)
{
	struct at91_port *pioa = (struct at91_port *)ATMEL_BASE_PIOA;

	at91_periph_clk_enable(ATMEL_ID_EMAC0);

	/* Disable pull-ups to prevent PHY going into test mode */
	writel(pin_to_mask(AT91_PIN_PA14) |
	       pin_to_mask(AT91_PIN_PA15) |
	       pin_to_mask(AT91_PIN_PA18),
	       &pioa->pudr);

	/* Power down ethernet */
	pca953x_set_dir(0x28, IO_EXP_ETH_POWER, PCA953X_DIR_OUT);
	pca953x_set_val(0x28, IO_EXP_ETH_POWER, 1);

	/* Hold ethernet in reset */
	pca953x_set_dir(0x28, IO_EXP_ETH_RESET, PCA953X_DIR_OUT);
	pca953x_set_val(0x28, IO_EXP_ETH_RESET, 0);

	/* Enable ethernet power */
	pca953x_set_val(0x28, IO_EXP_ETH_POWER, 0);

	at91_phy_reset();

	/* Bring the ethernet out of reset */
	pca953x_set_val(0x28, IO_EXP_ETH_RESET, 1);

	/* The phy internal reset take 21ms */
	udelay(21 * 1000);

	/* Re-enable pull-up */
	writel(pin_to_mask(AT91_PIN_PA14) |
	       pin_to_mask(AT91_PIN_PA15) |
	       pin_to_mask(AT91_PIN_PA18),
	       &pioa->puer);

	at91_macb_hw_init();
}

static void nand_hw_init(void)
{
	struct at91_smc *smc       = (struct at91_smc    *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Enable CS3 as NAND/SmartMedia */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_CS3A_SMC_SMARTMEDIA;
	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(4) | AT91_SMC_PULSE_NCS_WR(4) |
	       AT91_SMC_PULSE_NRD(4) | AT91_SMC_PULSE_NCS_RD(4),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(7) | AT91_SMC_CYCLE_NRD(7),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
	       AT91_SMC_MODE_DBW_8 |
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);

	/* Configure RDY/BSY */
	gpio_request(CONFIG_SYS_NAND_READY_PIN, "nand_rdy");
	gpio_direction_input(CONFIG_SYS_NAND_READY_PIN);

	/* Enable NandFlash */
	gpio_request(CONFIG_SYS_NAND_ENABLE_PIN, "nand_ce");
	gpio_direction_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}

int board_init(void)
{
	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOB);
	at91_periph_clk_enable(ATMEL_ID_PIOC);

	/* The mach-type is the same for both Snapper 9260 and 9G20 */
	gd->bd->bi_arch_number = MACH_TYPE_SNAPPER_9260;

	/* Address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	/* Initialise peripherals */
	at91_seriald_hw_init();
	i2c_set_bus_num(0);
	nand_hw_init();
	macb_hw_init();

	return 0;
}

int board_eth_init(bd_t *bis)
{
	return macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC0, 0x1f);
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

void reset_phy(void)
{
}

static struct atmel_serial_platdata at91sam9260_serial_plat = {
	.base_addr = ATMEL_BASE_DBGU,
};

U_BOOT_DEVICE(at91sam9260_serial) = {
	.name	= "serial_atmel",
	.platdata = &at91sam9260_serial_plat,
};
