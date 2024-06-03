// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012-2013 Atmel Corporation
 * Bo Shen <voice.shen@atmel.com>
 */

#include <common.h>
#include <asm/arch/sama5d3.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>

unsigned int has_emac()
{
	return cpu_is_sama5d31() || cpu_is_sama5d35() || cpu_is_sama5d36();
}

unsigned int has_gmac()
{
	return !cpu_is_sama5d31();
}

unsigned int has_lcdc()
{
	return !cpu_is_sama5d35();
}

char *get_cpu_name()
{
	unsigned int extension_id = get_extension_chip_id();

	if (cpu_is_sama5d3())
		switch (extension_id) {
		case ARCH_EXID_SAMA5D31:
			return "SAMA5D31";
		case ARCH_EXID_SAMA5D33:
			return "SAMA5D33";
		case ARCH_EXID_SAMA5D34:
			return "SAMA5D34";
		case ARCH_EXID_SAMA5D35:
			return "SAMA5D35";
		case ARCH_EXID_SAMA5D36:
			return "SAMA5D36";
		default:
			return "Unknown CPU type";
		}
	else
		return "Unknown CPU type";
}

void at91_serial0_hw_init(void)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 18, 1);	/* TXD0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 17, 0);	/* RXD0 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_USART0);
}

void at91_serial1_hw_init(void)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 29, 1);	/* TXD1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 28, 0);	/* RXD1 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_USART1);
}

void at91_serial2_hw_init(void)
{
	at91_pio3_set_b_periph(AT91_PIO_PORTE, 26, 1);	/* TXD2 */
	at91_pio3_set_b_periph(AT91_PIO_PORTE, 25, 0);	/* RXD2 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_USART2);
}

void at91_seriald_hw_init(void)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 31, 1);	/* DTXD */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 30, 0);	/* DRXD */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_DBGU);
}

#if defined(CONFIG_ATMEL_SPI)
void at91_spi0_hw_init(unsigned long cs_mask)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 10, 0);       /* SPI0_MISO */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 11, 0);       /* SPI0_MOSI */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 12, 0);       /* SPI0_SPCK */

	if (cs_mask & (1 << 0))
		at91_set_pio_output(AT91_PIO_PORTD, 13, 1);
	if (cs_mask & (1 << 1))
		at91_set_pio_output(AT91_PIO_PORTD, 14, 1);
	if (cs_mask & (1 << 2))
		at91_set_pio_output(AT91_PIO_PORTD, 15, 1);
	if (cs_mask & (1 << 3))
		at91_set_pio_output(AT91_PIO_PORTD, 16, 1);

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_SPI0);
}
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
void at91_mci_hw_init(void)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 0, 0);	/* MCI0 CMD */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 1, 0);	/* MCI0 DA0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 2, 0);	/* MCI0 DA1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 3, 0);        /* MCI0 DA2 */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 4, 0);        /* MCI0 DA3 */
#ifdef CONFIG_ATMEL_MCI_8BIT
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 5, 0);        /* MCI0 DA4 */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 6, 0);        /* MCI0 DA5 */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 7, 0);        /* MCI0 DA6 */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 8, 0);        /* MCI0 DA7 */
#endif
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 9, 0);        /* MCI0 CLK */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_MCI0);
}
#endif

#ifdef CONFIG_MACB
void at91_macb_hw_init(void)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 7, 0);	/* ETXCK_EREFCK */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 5, 0);	/* ERXDV */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 2, 0);	/* ERX0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 3, 0);	/* ERX1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 6, 0);	/* ERXER */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 4, 0);	/* ETXEN */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 0, 0);	/* ETX0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 1, 0);	/* ETX1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 9, 0);	/* EMDIO */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 8, 0);	/* EMDC */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_EMAC);
}

void at91_gmac_hw_init(void)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 0, 0);	/* GTX0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 1, 0);	/* GTX1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 2, 0);	/* GTX2 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 3, 0);	/* GTX3 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 4, 0);	/* GRX0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 5, 0);	/* GRX1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 6, 0);	/* GRX2 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 7, 0);	/* GRX3 */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 8, 0);	/* GTXCK */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 9, 0);	/* GTXEN */

	at91_pio3_set_a_periph(AT91_PIO_PORTB, 11, 0);	/* GRXCK */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 13, 0);	/* GRXER */

	at91_pio3_set_a_periph(AT91_PIO_PORTB, 16, 0);	/* GMDC */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 17, 0);	/* GMDIO */
	at91_pio3_set_a_periph(AT91_PIO_PORTB, 18, 0);	/* G125CK */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_GMAC);
}
#endif

#ifdef CONFIG_LCD
void at91_lcd_hw_init(void)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 24, 0);	/* LCDPWM */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 25, 0);	/* LCDDISP */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 26, 0);	/* LCDVSYNC */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 27, 0);	/* LCDHSYNC */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 28, 0);	/* LCDDOTCK */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 29, 0);	/* LCDDEN */

	/* The lower 16-bit of LCD only available on Port A */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  0, 0);	/* LCDD0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  1, 0);	/* LCDD1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  2, 0);	/* LCDD2 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  3, 0);	/* LCDD3 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  4, 0);	/* LCDD4 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  5, 0);	/* LCDD5 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  6, 0);	/* LCDD6 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  7, 0);	/* LCDD7 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  8, 0);	/* LCDD8 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA,  9, 0);	/* LCDD9 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 10, 0);	/* LCDD10 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 11, 0);	/* LCDD11 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 12, 0);	/* LCDD12 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 13, 0);	/* LCDD13 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 14, 0);	/* LCDD14 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 15, 0);	/* LCDD15 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_LCDC);
}
#endif

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
void at91_udp_hw_init(void)
{
	/* Enable UPLL clock */
	at91_upll_clk_enable();
	/* Enable UDPHS clock */
	at91_periph_clk_enable(ATMEL_ID_UDPHS);
}
#endif
