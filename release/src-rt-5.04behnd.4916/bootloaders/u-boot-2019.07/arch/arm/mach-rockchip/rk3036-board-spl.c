// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <debug_uart.h>
#include <asm/io.h>
#include <asm/arch-rockchip/bootrom.h>
#include <asm/arch-rockchip/sdram_rk3036.h>
#include <asm/arch-rockchip/timer.h>

void board_init_f(ulong dummy)
{
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif
	rockchip_timer_init();
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
