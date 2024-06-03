// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Achim Ehrlich <aehrlich@taskit.de>
 * taskit GmbH <www.taskit.de>
 *
 * (C) Copyright 2012-
 * Markus Hubig <mhubig@imko.de>
 * IMKO GmbH <www.imko.de>
 * (C) Copyright 2014
 * Heiko Schocher <hs@denx.de>
 * DENX Software Engineering GmbH
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/at91sam9_sdramc.h>
#include <asm/arch/at91sam9260_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/atmel_serial.h>
#include <asm/arch/at91_spi.h>
#include <spi.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/gpio.h>
#include <watchdog.h>
# include <net.h>
#ifndef CONFIG_DM_ETH
# include <netdev.h>
#endif
#include <g_dnl.h>

DECLARE_GLOBAL_DATA_PTR;

static void smartweb_request_gpio(void)
{
	gpio_request(CONFIG_SYS_NAND_ENABLE_PIN, "nand ena");
	gpio_request(CONFIG_SYS_NAND_READY_PIN, "nand rdy");
	gpio_request(AT91_PIN_PA26, "ena PHY");
}

static void smartweb_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Assign CS3 to NAND/SmartMedia Interface */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_CS3A_SMC_SMARTMEDIA;
	writel(csa, &matrix->ebicsa);

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
		AT91_SMC_MODE_TDF_CYCLE(2),
		&smc->cs[3].mode);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
}

static void smartweb_macb_hw_init(void)
{
	struct at91_port *pioa = (struct at91_port *)ATMEL_BASE_PIOA;

	/* Enable the PHY Chip via PA26 on the Stamp 2 Adaptor */
	at91_set_gpio_output(AT91_PIN_PA26, 0);

	/*
	 * Disable pull-up on:
	 *	RXDV (PA17) => PHY normal mode (not Test mode)
	 *	ERX0 (PA14) => PHY ADDR0
	 *	ERX1 (PA15) => PHY ADDR1
	 *	ERX2 (PA25) => PHY ADDR2
	 *	ERX3 (PA26) => PHY ADDR3
	 *	ECRS (PA28) => PHY ADDR4  => PHYADDR = 0x0
	 *
	 * PHY has internal pull-down
	 */
	writel(pin_to_mask(AT91_PIN_PA14) |
		pin_to_mask(AT91_PIN_PA15) |
		pin_to_mask(AT91_PIN_PA17) |
		pin_to_mask(AT91_PIN_PA25) |
		pin_to_mask(AT91_PIN_PA26) |
		pin_to_mask(AT91_PIN_PA28) |
		pin_to_mask(AT91_PIN_PA29),
		&pioa->pudr);

	at91_phy_reset();

	/* Re-enable pull-up */
	writel(pin_to_mask(AT91_PIN_PA14) |
		pin_to_mask(AT91_PIN_PA15) |
		pin_to_mask(AT91_PIN_PA17) |
		pin_to_mask(AT91_PIN_PA25) |
		pin_to_mask(AT91_PIN_PA26) |
		pin_to_mask(AT91_PIN_PA28) |
		pin_to_mask(AT91_PIN_PA29),
		&pioa->puer);

	/* Initialize EMAC=MACB hardware */
	at91_macb_hw_init();
}

#ifdef CONFIG_USB_GADGET_AT91
#include <linux/usb/at91_udc.h>

void at91_udp_hw_init(void)
{
	/* Enable PLLB */
	at91_pllb_clk_enable(get_pllb_init());

	/* Enable UDPCK clock, MCK is enabled in at91_clock_init() */
	at91_periph_clk_enable(ATMEL_ID_UDP);

	at91_system_clk_enable(AT91SAM926x_PMC_UDP);
}

struct at91_udc_data board_udc_data  = {
	.baseaddr = ATMEL_BASE_UDP0,
};
#endif

int board_early_init_f(void)
{
	/* enable this here, as we have SPL without serial support */
	at91_seriald_hw_init();
	smartweb_request_gpio();
	return 0;
}

int board_init(void)
{
	smartweb_request_gpio();
	/* power LED red */
	at91_set_gpio_output(AT91_PIN_PC6, 0);
	at91_set_gpio_output(AT91_PIN_PC7, 1);
	/* alarm LED off */
	at91_set_gpio_output(AT91_PIN_PC8, 0);
	at91_set_gpio_output(AT91_PIN_PC9, 0);
	/* prog LED red */
	at91_set_gpio_output(AT91_PIN_PC10, 0);
	at91_set_gpio_output(AT91_PIN_PC11, 1);

#ifdef CONFIG_USB_GADGET_AT91
	at91_udp_hw_init();
	at91_udc_probe(&board_udc_data);
#endif

	/* Adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	smartweb_nand_hw_init();
	smartweb_macb_hw_init();
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size(
		(void *)CONFIG_SYS_SDRAM_BASE,
		CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#ifndef CONFIG_DM_ETH
#ifdef CONFIG_MACB
int board_eth_init(bd_t *bis)
{
	return macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC0, 0x00);
}
#endif /* CONFIG_MACB */
#endif

#if defined(CONFIG_SPL_BUILD)
#include <spl.h>
#include <nand.h>
#include <spi_flash.h>

void matrix_init(void)
{
	struct at91_matrix *mat = (struct at91_matrix *)ATMEL_BASE_MATRIX;

	writel((readl(&mat->scfg[3]) & (~AT91_MATRIX_SLOT_CYCLE))
			| AT91_MATRIX_SLOT_CYCLE_(0x40),
			&mat->scfg[3]);
}

void at91_spl_board_init(void)
{
	smartweb_request_gpio();
	/* power LED orange */
	at91_set_gpio_output(AT91_PIN_PC6, 1);
	at91_set_gpio_output(AT91_PIN_PC7, 1);
	/* alarm LED orange */
	at91_set_gpio_output(AT91_PIN_PC8, 1);
	at91_set_gpio_output(AT91_PIN_PC9, 1);
	/* prog LED red */
	at91_set_gpio_output(AT91_PIN_PC10, 0);
	at91_set_gpio_output(AT91_PIN_PC11, 1);

	smartweb_nand_hw_init();
	at91_set_gpio_input(AT91_PIN_PA28, 1);
	at91_set_gpio_input(AT91_PIN_PA29, 1);

	/* check if both  button are pressed */
	if (at91_get_gpio_value(AT91_PIN_PA28) == 0 &&
		at91_get_gpio_value(AT91_PIN_PA29) == 0) {
		smartweb_nand_hw_init();
		nand_init();
		spl_nand_erase_one(0, 0);
	}
}

#define SDRAM_BASE_CONF	(AT91_SDRAMC_NC_9 | AT91_SDRAMC_NR_13 \
			 | AT91_SDRAMC_CAS_2 \
			 | AT91_SDRAMC_NB_4 | AT91_SDRAMC_DBW_32 \
			 | AT91_SDRAMC_TWR_VAL(2) | AT91_SDRAMC_TRC_VAL(7) \
			 | AT91_SDRAMC_TRP_VAL(2) | AT91_SDRAMC_TRCD_VAL(2) \
			 | AT91_SDRAMC_TRAS_VAL(5) | AT91_SDRAMC_TXSR_VAL(8))

void mem_init(void)
{
	struct at91_matrix *ma = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	struct at91_port *port = (struct at91_port *)ATMEL_BASE_PIOC;
	struct sdramc_reg setting;

	setting.cr = SDRAM_BASE_CONF;
	setting.mdr = AT91_SDRAMC_MD_SDRAM;
	setting.tr = (CONFIG_SYS_MASTER_CLOCK * 7) / 1000000;

	/*
	 * I write here directly in this register, because this
	 * approach is smaller than calling at91_set_a_periph() in a
	 * for loop. This saved me 96 bytes.
	 */
	writel(0xffff0000, &port->pdr);

	writel(readl(&ma->ebicsa) | AT91_MATRIX_CS1A_SDRAMC, &ma->ebicsa);
	sdramc_initialize(ATMEL_BASE_CS1, &setting);
}
#endif

int g_dnl_bind_fixup(struct usb_device_descriptor *dev, const char *name)
{
	g_dnl_set_serialnumber("1");
	return 0;
}
