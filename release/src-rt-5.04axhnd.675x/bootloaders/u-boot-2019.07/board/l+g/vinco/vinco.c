// SPDX-License-Identifier: GPL-2.0+
/*
 * Board file for the VInCo platform
 * Based on the the SAMA5-EK board file
 * Configuration settings for the VInCo platform.
 * Copyright (C) 2014 Atmel
 *		      Bo Shen <voice.shen@atmel.com>
 * Copyright (C) 2015 Free Electrons
 *		      Gregory CLEMENT <gregory.clement@free-electrons.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/atmel_usba_udc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <asm/arch/sama5d3_smc.h>
#include <asm/arch/sama5d4.h>
#include <atmel_hlcdc.h>
#include <atmel_mci.h>
#include <lcd.h>
#include <mmc.h>
#include <net.h>
#include <netdev.h>
#include <nand.h>
#include <spi.h>
#include <version.h>

DECLARE_GLOBAL_DATA_PTR;

/* FIXME gpio code here need to handle through DM_GPIO */
#ifndef CONFIG_DM_SPI
int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	at91_set_pio_output(AT91_PIO_PORTC, 3, 0);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	at91_set_pio_output(AT91_PIO_PORTC, 3, 1);
}

static void vinco_spi0_hw_init(void)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 0, 0);	/* SPI0_MISO */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 1, 0);	/* SPI0_MOSI */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 2, 0);	/* SPI0_SPCK */

	at91_set_pio_output(AT91_PIO_PORTC, 3, 1);	/* SPI0_CS0 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_SPI0);
}
#endif /* CONFIG_ATMEL_SPI */


#ifdef CONFIG_CMD_USB
static void vinco_usb_hw_init(void)
{
	at91_set_pio_output(AT91_PIO_PORTE, 11, 0);
	at91_set_pio_output(AT91_PIO_PORTE, 12, 0);
	at91_set_pio_output(AT91_PIO_PORTE, 10, 0);
}
#endif


#ifdef CONFIG_GENERIC_ATMEL_MCI
void vinco_mci0_hw_init(void)
{
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 5, 1);	/* MCI0 CDA */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 6, 1);	/* MCI0 DA0 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 7, 1);	/* MCI0 DA1 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 8, 1);	/* MCI0 DA2 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 9, 1);	/* MCI0 DA3 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 10, 1);	/* MCI0 DA4 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 11, 1);	/* MCI0 DA5 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 12, 1);	/* MCI0 DA6 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 13, 1);	/* MCI0 DA7 */
	at91_pio3_set_b_periph(AT91_PIO_PORTC, 4, 0);	/* MCI0 CLK */

	/*
	 * As the mci io internal pull down is too strong, so if the io needs
	 * external pull up, the pull up resistor will be very small, if so
	 * the power consumption will increase, so disable the interanl pull
	 * down to save the power.
	 */
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 4, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 5, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 6, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 7, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 8, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 9, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 10, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 11, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 12, 0);
	at91_pio3_set_pio_pulldown(AT91_PIO_PORTC, 13, 0);

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_MCI0);
}

int board_mmc_init(bd_t *bis)
{
	/* Enable power for MCI0 interface */
	at91_set_pio_output(AT91_PIO_PORTE, 7, 1);

	return atmel_mci_init((void *)ATMEL_BASE_MCI0);
}
#endif /* CONFIG_GENERIC_ATMEL_MCI */

#ifdef CONFIG_MACB
void vinco_macb0_hw_init(void)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 0, 0);	/* ETXCK_EREFCK */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 6, 0);	/* ERXDV */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 8, 0);	/* ERX0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 9, 0);	/* ERX1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 7, 0);	/* ERXER */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 2, 0);	/* ETXEN */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 12, 0);	/* ETX0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 13, 0);	/* ETX1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 17, 0);	/* EMDIO */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 16, 0);	/* EMDC */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_GMAC0);

	/* Enable Phy*/
	at91_set_pio_output(AT91_PIO_PORTE, 8, 1);
}
#endif

static void vinco_serial3_hw_init(void)
{
	at91_pio3_set_b_periph(AT91_PIO_PORTE, 17, 1);	/* TXD3 */
	at91_pio3_set_b_periph(AT91_PIO_PORTE, 16, 0);	/* RXD3 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_USART3);
}

int board_early_init_f(void)
{
	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOB);
	at91_periph_clk_enable(ATMEL_ID_PIOC);
	at91_periph_clk_enable(ATMEL_ID_PIOD);
	at91_periph_clk_enable(ATMEL_ID_PIOE);

	vinco_serial3_hw_init();

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifndef CONFIG_DM_SPI
	vinco_spi0_hw_init();
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
	vinco_mci0_hw_init();
#endif
#ifdef CONFIG_MACB
	vinco_macb0_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	vinco_usb_hw_init();
#endif
#ifdef CONFIG_USB_GADGET_ATMEL_USBA
	at91_udp_hw_init();
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
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_GMAC0, 0x00);
#endif

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
	usba_udc_probe(&pdata);
#ifdef CONFIG_USB_ETH_RNDIS
	usb_eth_initialize(bis);
#endif
#endif

	return rc;
}
