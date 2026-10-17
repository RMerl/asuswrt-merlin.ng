// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Mihai Sain <mihai.sain@microchip.com>
 *
 */

#include <common.h>
#include <debug_uart.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/atmel_pio4.h>
#include <asm/arch/atmel_sdhci.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sama5d2.h>

extern void at91_pda_detect(void);

DECLARE_GLOBAL_DATA_PTR;

static void rgb_leds_init(void)
{
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 7, 0);	/* LED RED */
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 8, 0);	/* LED GREEN */
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 9, 1);	/* LED BLUE */
}

static void board_usb_hw_init(void)
{
	atmel_pio4_set_pio_output(AT91_PIO_PORTA, 6, 1);
}

int board_late_init(void)
{
	at91_video_show_board_info();

	at91_pda_detect();

	return 0;
}

static void board_uart0_hw_init(void)
{
	atmel_pio4_set_c_periph(AT91_PIO_PORTB, 26, ATMEL_PIO_PUEN_MASK);	/* URXD0 */
	atmel_pio4_set_c_periph(AT91_PIO_PORTB, 27, 0);				/* UTXD0 */

	at91_periph_clk_enable(ATMEL_ID_UART0);
}

void board_debug_uart_init(void)
{
	board_uart0_hw_init();
}

int board_early_init_f(void)
{
	debug_uart_init();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = gd->bd->bi_dram[0].start + 0x100;

	rgb_leds_init();

	board_usb_hw_init();

	return 0;
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}
