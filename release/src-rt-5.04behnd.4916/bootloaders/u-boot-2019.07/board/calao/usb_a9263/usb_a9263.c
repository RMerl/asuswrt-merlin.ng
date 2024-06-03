// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2013
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 * Thomas Petazzoni, Free Electrons, <thomas.petazzoni@free-electrons.com>
 * Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 */

#include <common.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_matrix.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm-generic/gpio.h>
#include <asm/io.h>
#include <net.h>
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_CMD_NAND
static void usb_a9263_nand_hw_init(void)
{
	unsigned long csa;
	at91_smc_t *smc = (at91_smc_t *)ATMEL_BASE_SMC0;
	at91_matrix_t *matrix = (at91_matrix_t *)ATMEL_BASE_MATRIX;

	/* Enable CS3 */
	csa = readl(&matrix->csa[0]) | AT91_MATRIX_CSA_EBI_CS3A;
	writel(csa, &matrix->csa[0]);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
	       AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(0),
	       &smc->cs[3].setup);

	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(3) |
	       AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(3),
	       &smc->cs[3].pulse);

	writel(AT91_SMC_CYCLE_NWE(5) | AT91_SMC_CYCLE_NRD(5),
	       &smc->cs[3].cycle);

	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
	       AT91_SMC_MODE_DBW_8 |
	       AT91_SMC_MODE_TDF_CYCLE(2), &smc->cs[3].mode);

	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOCDE);

	/* Configure RDY/BSY */
	gpio_request(CONFIG_SYS_NAND_READY_PIN, "NAND ready/busy");
	gpio_direction_input(CONFIG_SYS_NAND_READY_PIN);

	/* Enable NandFlash */
	gpio_request(CONFIG_SYS_NAND_ENABLE_PIN, "NAND enable");
	gpio_direction_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}
#endif

#ifdef CONFIG_MACB
static void usb_a9263_macb_hw_init(void)
{
	at91_periph_clk_enable(ATMEL_ID_EMAC);

	/*
	 * Disable pull-up on:
	 *  RXDV (PC25) => PHY normal mode (not Test mode)
	 *  ERX0 (PE25) => PHY ADDR0
	 *  ERX1 (PE26) => PHY ADDR1 => PHYADDR = 0x0
	 *
	 * PHY has internal weak pull-up/pull-down
	 */
	gpio_request(GPIO_PIN_PC(25), "PHY mode");
	gpio_direction_input(GPIO_PIN_PC(25));

	gpio_request(GPIO_PIN_PE(25), "PHY ADDR0");
	gpio_direction_input(GPIO_PIN_PE(25));

	gpio_request(GPIO_PIN_PE(26), "PHY ADDR1");
	gpio_direction_input(GPIO_PIN_PE(26));

	at91_phy_reset();

	/* It will set proper pinmux for ports PC25, PE25-26 */
	at91_macb_hw_init();
}
#endif

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	usb_a9263_nand_hw_init();
#endif
#ifdef CONFIG_MACB
	usb_a9263_macb_hw_init();
#endif
#ifdef CONFIG_USB_OHCI_NEW
	at91_uhp_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC, 0x0001);
#endif
	return rc;
}
