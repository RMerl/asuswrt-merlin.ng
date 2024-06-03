// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>

/*
 * if CONFIG_AT91_GPIO_PULLUP ist set, keep pullups on on all
 * peripheral pins. Good to have if hardware is soldered optionally
 * or in case of SPI no slave is selected. Avoid lines to float
 * needlessly. Use a short local PUP define.
 *
 * Due to errata "TXD floats when CTS is inactive" pullups are always
 * on for TXD pins.
 */
#ifdef CONFIG_AT91_GPIO_PULLUP
# define PUP CONFIG_AT91_GPIO_PULLUP
#else
# define PUP 0
#endif

void at91_serial0_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTC, 8, 1);		/* TXD0 */
	at91_set_a_periph(AT91_PIO_PORTC, 9, 0);		/* RXD0 */
	at91_periph_clk_enable(ATMEL_ID_USART0);
}

void at91_serial1_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTC, 12, 1);		/* TXD1 */
	at91_set_a_periph(AT91_PIO_PORTC, 13, 0);		/* RXD1 */
	at91_periph_clk_enable(ATMEL_ID_USART1);
}

void at91_serial2_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTC, 14, 1);		/* TXD2 */
	at91_set_a_periph(AT91_PIO_PORTC, 15, 0);		/* RXD2 */
	at91_periph_clk_enable(ATMEL_ID_USART2);
}

void at91_seriald_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTA, 9, 0);		/* DRXD */
	at91_set_a_periph(AT91_PIO_PORTA, 10, 1);		/* DTXD */
	at91_periph_clk_enable(ATMEL_ID_SYS);
}

#ifdef CONFIG_ATMEL_SPI
void at91_spi0_hw_init(unsigned long cs_mask)
{
	at91_set_a_periph(AT91_PIO_PORTA, 0, PUP);	/* SPI0_MISO */
	at91_set_a_periph(AT91_PIO_PORTA, 1, PUP);	/* SPI0_MOSI */
	at91_set_a_periph(AT91_PIO_PORTA, 2, PUP);	/* SPI0_SPCK */

	at91_periph_clk_enable(ATMEL_ID_SPI0);

	if (cs_mask & (1 << 0)) {
		at91_set_a_periph(AT91_PIO_PORTA, 3, 1);
	}
	if (cs_mask & (1 << 1)) {
		at91_set_a_periph(AT91_PIO_PORTA, 4, 1);
	}
	if (cs_mask & (1 << 2)) {
		at91_set_a_periph(AT91_PIO_PORTA, 5, 1);
	}
	if (cs_mask & (1 << 3)) {
		at91_set_a_periph(AT91_PIO_PORTA, 6, 1);
	}
	if (cs_mask & (1 << 4)) {
		at91_set_pio_output(AT91_PIO_PORTA, 3, 1);
	}
	if (cs_mask & (1 << 5)) {
		at91_set_pio_output(AT91_PIO_PORTA, 4, 1);
	}
	if (cs_mask & (1 << 6)) {
		at91_set_pio_output(AT91_PIO_PORTA, 5, 1);
	}
	if (cs_mask & (1 << 7)) {
		at91_set_pio_output(AT91_PIO_PORTA, 6, 1);
	}
}

void at91_spi1_hw_init(unsigned long cs_mask)
{
	at91_set_a_periph(AT91_PIO_PORTB, 30, PUP);	/* SPI1_MISO */
	at91_set_a_periph(AT91_PIO_PORTB, 31, PUP);	/* SPI1_MOSI */
	at91_set_a_periph(AT91_PIO_PORTB, 29, PUP);	/* SPI1_SPCK */

	at91_periph_clk_enable(ATMEL_ID_SPI1);

	if (cs_mask & (1 << 0)) {
		at91_set_a_periph(AT91_PIO_PORTB, 28, 1);
	}
	if (cs_mask & (1 << 1)) {
		at91_set_b_periph(AT91_PIO_PORTA, 24, 1);
	}
	if (cs_mask & (1 << 2)) {
		at91_set_b_periph(AT91_PIO_PORTA, 25, 1);
	}
	if (cs_mask & (1 << 3)) {
		at91_set_a_periph(AT91_PIO_PORTA, 26, 1);
	}
	if (cs_mask & (1 << 4)) {
		at91_set_pio_output(AT91_PIO_PORTB, 28, 1);
	}
	if (cs_mask & (1 << 5)) {
		at91_set_pio_output(AT91_PIO_PORTA, 24, 1);
	}
	if (cs_mask & (1 << 6)) {
		at91_set_pio_output(AT91_PIO_PORTA, 25, 1);
	}
	if (cs_mask & (1 << 7)) {
		at91_set_pio_output(AT91_PIO_PORTA, 26, 1);
	}
}
#endif
