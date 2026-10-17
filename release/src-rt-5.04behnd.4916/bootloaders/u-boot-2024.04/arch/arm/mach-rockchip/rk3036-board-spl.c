// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015-2019 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <debug_uart.h>
#include <init.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/sdram_rk3036.h>

#define TIMER_LOAD_COUNT_L	0x00
#define TIMER_LOAD_COUNT_H	0x04
#define TIMER_CONTROL_REG	0x10
#define TIMER_EN	0x1
#define	TIMER_FMODE	(0 << 1)
#define	TIMER_RMODE	(1 << 1)

void rockchip_stimer_init(void)
{
	asm volatile("mcr p15, 0, %0, c14, c0, 0"
		     : : "r"(CONFIG_COUNTER_FREQUENCY));

	writel(0, CONFIG_ROCKCHIP_STIMER_BASE + TIMER_CONTROL_REG);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE);
	writel(0xffffffff, CONFIG_ROCKCHIP_STIMER_BASE + 4);
	writel(TIMER_EN | TIMER_FMODE, CONFIG_ROCKCHIP_STIMER_BASE +
	       TIMER_CONTROL_REG);
}

void board_init_f(ulong dummy)
{
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif

	/* Init secure timer */
	rockchip_stimer_init();
	/* Init ARM arch timer in arch/arm/cpu/armv7/arch_timer.c */
	timer_init();

	sdram_init();

	/* return to maskrom */
	back_to_bootrom(BROM_BOOT_NEXTSTAGE);
}

/* Place Holders */
void board_init_r(gd_t *id, ulong dest_addr)
{
	/*
	 * Function attribute is no-return
	 * This Function never executes
	 */
	while (1)
		;
}
