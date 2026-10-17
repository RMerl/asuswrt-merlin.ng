// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Sandeep Sheriker M <sandeep.sheriker@microchip.com>
 */

#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_sfr.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <debug_uart.h>
#include <asm/mach-types.h>

extern void at91_pda_detect(void);

DECLARE_GLOBAL_DATA_PTR;

void at91_prepare_cpu_var(void);

static void board_leds_init(void)
{
	at91_set_pio_output(AT91_PIO_PORTB, 11, 0);	/* LED RED */
	at91_set_pio_output(AT91_PIO_PORTB, 12, 0);	/* LED GREEN */
	at91_set_pio_output(AT91_PIO_PORTB, 13, 1);	/* LED BLUE */
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
	at91_prepare_cpu_var();

	at91_pda_detect();

	return 0;
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	at91_seriald_hw_init();
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	return 0;
}
#endif

#define MAC24AA_MAC_OFFSET     0xfa

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
#ifdef CONFIG_I2C_EEPROM
	at91_set_ethaddr(MAC24AA_MAC_OFFSET);
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
