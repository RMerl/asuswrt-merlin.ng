// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2006
 * Atmel Nordic AB <www.atmel.com>
 * Ulf Samuelsson <ulf@atmel.com>
 *
 * (C) Copyright 2010
 * Andreas Bießmann <andreas@biessmann.org>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/clk.h>
#include <asm/arch/at91_pio.h>
#include <status_led.h>

/* bit mask in PIO port B */
#define	GREEN_LED	(1<<0)
#define	YELLOW_LED	(1<<1)
#define	RED_LED		(1<<2)

void	green_led_on(void)
{
	at91_pio_t *pio = (at91_pio_t *)ATMEL_BASE_PIO;
	writel(GREEN_LED, &pio->piob.codr);
}

void	 yellow_led_on(void)
{
	at91_pio_t *pio = (at91_pio_t *)ATMEL_BASE_PIO;
	writel(YELLOW_LED, &pio->piob.codr);
}

void	 red_led_on(void)
{
	at91_pio_t *pio = (at91_pio_t *)ATMEL_BASE_PIO;
	writel(RED_LED, &pio->piob.codr);
}

void	green_led_off(void)
{
	at91_pio_t *pio = (at91_pio_t *)ATMEL_BASE_PIO;
	writel(GREEN_LED, &pio->piob.sodr);
}

void	yellow_led_off(void)
{
	at91_pio_t *pio = (at91_pio_t *)ATMEL_BASE_PIO;
	writel(YELLOW_LED, &pio->piob.sodr);
}

void	red_led_off(void)
{
	at91_pio_t *pio = (at91_pio_t *)ATMEL_BASE_PIO;
	writel(RED_LED, &pio->piob.sodr);
}

void coloured_LED_init (void)
{
	at91_pio_t *pio = (at91_pio_t *)ATMEL_BASE_PIO;

	at91_periph_clk_enable(ATMEL_ID_PIOB);

	/* Disable peripherals on LEDs */
	writel(GREEN_LED | YELLOW_LED | RED_LED, &pio->piob.per);
	/* Enable pins as outputs */
	writel(GREEN_LED | YELLOW_LED | RED_LED, &pio->piob.oer);
	/* Turn all LEDs OFF */
	writel(GREEN_LED | YELLOW_LED | RED_LED, &pio->piob.sodr);
}
