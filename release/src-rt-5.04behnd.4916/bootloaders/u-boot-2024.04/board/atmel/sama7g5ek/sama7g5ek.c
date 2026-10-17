// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Microchip Technology, Inc.
 *		      Eugen Hristev <eugen.hristev@microchip.com>
 */

#include <common.h>
#include <debug_uart.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/atmel_pio4.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sama7g5.h>

DECLARE_GLOBAL_DATA_PTR;

static void board_leds_init(void)
{
	atmel_pio4_set_pio_output(AT91_PIO_PORTB, 8, 0);	/* LED RED */
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 13, 0);	/* LED GREEN */
	atmel_pio4_set_pio_output(AT91_PIO_PORTD, 20, 1);	/* LED BLUE */
}

int board_late_init(void)
{
	return 0;
}

#if (IS_ENABLED(CONFIG_DEBUG_UART_BOARD_INIT))
static void board_uart0_hw_init(void)
{
	/* FLEXCOM3 IO0 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTD, 17, ATMEL_PIO_PUEN_MASK);
	/* FLEXCOM3 IO1 */
	atmel_pio4_set_f_periph(AT91_PIO_PORTD, 16, 0);

	at91_periph_clk_enable(ATMEL_ID_FLEXCOM3);
}

void board_debug_uart_init(void)
{
	board_uart0_hw_init();
}
#endif

int board_early_init_f(void)
{
	return 0;
}

#define MAC24AA_MAC_OFFSET     0xfa

#if (IS_ENABLED(CONFIG_MISC_INIT_R))
int misc_init_r(void)
{
#if (IS_ENABLED(CONFIG_I2C_EEPROM))
	at91_set_ethaddr(MAC24AA_MAC_OFFSET);
	at91_set_eth1addr(MAC24AA_MAC_OFFSET);
#endif
	return 0;
}
#endif

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

	board_leds_init();

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CFG_SYS_SDRAM_BASE,
				    CFG_SYS_SDRAM_SIZE);
	return 0;
}
