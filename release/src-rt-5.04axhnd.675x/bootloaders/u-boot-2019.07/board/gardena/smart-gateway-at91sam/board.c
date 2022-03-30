// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Atmel Corporation
 * Copyright (C) 2019 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <debug_uart.h>
#include <led.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/clk.h>

DECLARE_GLOBAL_DATA_PTR;

static void at91_prepare_cpu_var(void)
{
	env_set("cpu", get_cpu_name());
}

int board_late_init(void)
{
	at91_prepare_cpu_var();

	if (IS_ENABLED(CONFIG_LED))
		led_default_state();

	return 0;
}

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	at91_seriald_hw_init();
}
#endif

int board_early_init_f(void)
{
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif
	return 0;
}

int board_init(void)
{
	/* Address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);

	return 0;
}
