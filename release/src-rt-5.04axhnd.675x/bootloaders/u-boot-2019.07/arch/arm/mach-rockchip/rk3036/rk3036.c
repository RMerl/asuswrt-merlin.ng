// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */
#include <asm/io.h>
#include <asm/arch-rockchip/grf_rk3036.h>
#include <asm/arch-rockchip/hardware.h>

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
#define GRF_BASE	0x20008000
	struct rk3036_grf * const grf = (void *)GRF_BASE;
	enum {
		GPIO1C3_SHIFT		= 6,
		GPIO1C3_MASK		= 3 << GPIO1C3_SHIFT,
		GPIO1C3_GPIO		= 0,
		GPIO1C3_MMC0_D1,
		GPIO1C3_UART2_SOUT,

		GPIO1C2_SHIFT		= 4,
		GPIO1C2_MASK		= 3 << GPIO1C2_SHIFT,
		GPIO1C2_GPIO		= 0,
		GPIO1C2_MMC0_D0,
		GPIO1C2_UART2_SIN,
	};
	/*
	 * NOTE: sd card and debug uart use same iomux in rk3036,
	 * so if you enable uart,
	 * you can not boot from sdcard
	 */
	rk_clrsetreg(&grf->gpio1c_iomux,
		     GPIO1C3_MASK << GPIO1C3_SHIFT |
		     GPIO1C2_MASK << GPIO1C2_SHIFT,
		     GPIO1C3_UART2_SOUT << GPIO1C3_SHIFT |
		     GPIO1C2_UART2_SIN << GPIO1C2_SHIFT);
}
#endif
