// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Durai Manickam KR <durai.manickamkr@microchip.com>
 */

#include <common.h>
#include <debug_uart.h>
#include <fdtdec.h>
#include <init.h>
#include <led.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_sfr.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <dm/ofnode.h>

extern void at91_pda_detect(void);

DECLARE_GLOBAL_DATA_PTR;

void at91_prepare_cpu_var(void);

static void board_leds_init(void)
{
#if CONFIG_IS_ENABLED(LED)
	const char *led_name;
	struct udevice *dev;
	int ret;

	led_name = ofnode_conf_read_str("u-boot,boot-led");
	if (!led_name)
		return;

	ret = led_get_by_label(led_name, &dev);
	if (ret)
		return;

	led_set_state(dev, LEDST_ON);
#else
	at91_set_pio_output(AT91_PIO_PORTD, 17, 0);	/* LED RED */
	at91_set_pio_output(AT91_PIO_PORTD, 19, 0);	/* LED GREEN */
	at91_set_pio_output(AT91_PIO_PORTD, 21, 1);	/* LED BLUE */
#endif
}

int board_late_init(void)
{
	at91_prepare_cpu_var();

	at91_pda_detect();

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
	return 0;
}

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
	gd->bd->bi_boot_params = gd->bd->bi_dram[0].start + 0x100;

	board_leds_init();

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
