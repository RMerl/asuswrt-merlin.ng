// SPDX-License-Identifier: GPL-2.0+
/*
 * [partely copied from arch/arm/cpu/arm926ejs/at91/arm9260_devices.c]
 *
 * (C) Copyright 2011
 * Andreas Bießmann <andreas@biessmann.org>
 *
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
	at91_set_a_periph(AT91_PIO_PORTA, 17, 1);		/* TXD0 */
	at91_set_a_periph(AT91_PIO_PORTA, 18, PUP);		/* RXD0 */
	at91_periph_clk_enable(ATMEL_ID_USART0);
}

void at91_serial1_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTB, 20, PUP);		/* RXD1 */
	at91_set_a_periph(AT91_PIO_PORTB, 21, 1);		/* TXD1 */
	at91_periph_clk_enable(ATMEL_ID_USART1);
}

void at91_serial2_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTA, 22, PUP);		/* RXD2 */
	at91_set_a_periph(AT91_PIO_PORTA, 23, 1);		/* TXD2 */
	at91_periph_clk_enable(ATMEL_ID_USART2);
}

void at91_seriald_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTA, 30, PUP);		/* DRXD */
	at91_set_a_periph(AT91_PIO_PORTA, 31, 1);		/* DTXD */
	/* writing SYS to PCER has no effect on AT91RM9200 */
}
