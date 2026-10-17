// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <init.h>
#include <ram.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/sdram_px30.h>

#define TIMER_LOAD_COUNT0	0x00
#define TIMER_LOAD_COUNT1	0x04
#define TIMER_CUR_VALUE0	0x08
#define TIMER_CUR_VALUE1	0x0c
#define TIMER_CONTROL_REG	0x10

#define TIMER_EN	0x1
#define	TIMER_FMODE	(0 << 1)
#define	TIMER_RMODE	(1 << 1)

void secure_timer_init(void)
{
	writel(0, CONFIG_ROCKCHIP_STIMER_BASE + TIMER_CONTROL_REG);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + TIMER_LOAD_COUNT0);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + TIMER_LOAD_COUNT1);
	writel(TIMER_EN | TIMER_FMODE,
	       CONFIG_ROCKCHIP_STIMER_BASE + TIMER_CONTROL_REG);
}

void board_init_f(ulong dummy)
{
	int ret;

#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
	/*
	 * Debug UART can be used from here if required:
	 *
	 * debug_uart_init();
	 * printch('a');
	 * printhex8(0x1234);
	 * printascii("string");
	 */
	printascii("U-Boot TPL board init\n");
#endif

	secure_timer_init();
	ret = sdram_init();
	if (ret)
		printascii("sdram_init failed\n");

	/* return to maskrom */
	back_to_bootrom(BROM_BOOT_NEXTSTAGE);
}
