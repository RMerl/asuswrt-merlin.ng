// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries
 */

#include <common.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/io.h>

unsigned int get_chip_id(void)
{
	/* The 0x40 is the offset of cidr in DBGU */
	return readl(ATMEL_BASE_DBGU + 0x40) & ~ARCH_ID_VERSION_MASK;
}

unsigned int get_extension_chip_id(void)
{
	/* The 0x44 is the offset of exid in DBGU */
	return readl(ATMEL_BASE_DBGU + 0x44);
}

unsigned int has_emac1(void)
{
	return cpu_is_sam9x60();
}

unsigned int has_emac0(void)
{
	return cpu_is_sam9x60();
}

unsigned int has_lcdc(void)
{
	return cpu_is_sam9x60();
}

char *get_cpu_name(void)
{
	unsigned int extension_id = get_extension_chip_id();

	if (cpu_is_sam9x60()) {
		switch (extension_id) {
		case ARCH_EXID_SAM9X60:
			return "SAM9X60";
		case ARCH_EXID_SAM9X60_D6K:
			return "SAM9X60 8MiB SDRAM SiP";
		case ARCH_EXID_SAM9X60_D5M:
			return "SAM9X60 64MiB DDR2 SiP";
		case ARCH_EXID_SAM9X60_D1G:
			return "SAM9X60 128MiB DDR2 SiP";
		default:
			return "Unknown CPU type";
		}
	} else {
		return "Unknown CPU type";
	}
}

void at91_seriald_hw_init(void)
{
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 9, 1);	/* DRXD */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 10, 1);	/* DTXD */

	at91_periph_clk_enable(ATMEL_ID_DBGU);
}

void at91_mci_hw_init(void)
{
	/* Initialize the SDMMC0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 17, 1);	/* CLK */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 16, 1);	/* CMD */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 15, 1);	/* DAT0 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 18, 1);	/* DAT1 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 19, 1);	/* DAT2 */
	at91_pio3_set_a_periph(AT91_PIO_PORTA, 20, 1);	/* DAT3 */

	at91_periph_clk_enable(ATMEL_ID_SDMMC0);
}

#ifdef CONFIG_MACB
void at91_macb_hw_init(void)
{
	if (has_emac0()) {
		/* Enable EMAC0 clock */
		at91_periph_clk_enable(ATMEL_ID_EMAC0);
		/* EMAC0 pins setup */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 4, 0);	/* ETXCK */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 3, 0);	/* ERXDV */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 0, 0);	/* ERX0 */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 1, 0);	/* ERX1 */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 2, 0);	/* ERXER */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 7, 0);	/* ETXEN */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 9, 0);	/* ETX0 */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 10, 0);	/* ETX1 */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 5, 0);	/* EMDIO */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 6, 0);	/* EMDC */
	}

	if (has_emac1()) {
		/* Enable EMAC1 clock */
		at91_periph_clk_enable(ATMEL_ID_EMAC1);
		/* EMAC1 pins setup */
		at91_pio3_set_b_periph(AT91_PIO_PORTC, 29, 0);	/* ETXCK */
		at91_pio3_set_b_periph(AT91_PIO_PORTC, 28, 0);	/* ECRSDV */
		at91_pio3_set_b_periph(AT91_PIO_PORTC, 20, 0);	/* ERXO */
		at91_pio3_set_b_periph(AT91_PIO_PORTC, 21, 0);	/* ERX1 */
		at91_pio3_set_b_periph(AT91_PIO_PORTC, 16, 0);	/* ERXER */
		at91_pio3_set_b_periph(AT91_PIO_PORTC, 27, 0);	/* ETXEN */
		at91_pio3_set_b_periph(AT91_PIO_PORTC, 18, 0);	/* ETX0 */
		at91_pio3_set_b_periph(AT91_PIO_PORTC, 19, 0);	/* ETX1 */
		at91_pio3_set_b_periph(AT91_PIO_PORTC, 31, 0);	/* EMDIO */
		at91_pio3_set_b_periph(AT91_PIO_PORTC, 30, 0);	/* EMDC */
	}

#ifndef CONFIG_RMII
	/* Only emac0 support MII */
	if (has_emac0()) {
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 16, 0);	/* ECRS */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 17, 0);	/* ECOL */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 13, 0);	/* ERX2 */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 14, 0);	/* ERX3 */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 15, 0);	/* ERXCK */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 11, 0);	/* ETX2 */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 12, 0);	/* ETX3 */
		at91_pio3_set_a_periph(AT91_PIO_PORTB, 8, 0);	/* ETXER */
	}
#endif
}
#endif
