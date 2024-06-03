// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * NVIDIA Inc, <www.nvidia.com>
 *
 * Allen Martin <amartin@nvidia.com>
 */
#include <common.h>
#include <debug_uart.h>
#include <spl.h>

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/apb_misc.h>
#include <asm/arch-tegra/board.h>
#include <asm/spl.h>
#include "cpu.h"

void spl_board_init(void)
{
	struct apb_misc_pp_ctlr *apb_misc =
				(struct apb_misc_pp_ctlr *)NV_PA_APB_MISC_BASE;

	/* enable JTAG */
	writel(0xC0, &apb_misc->cfg_ctl);

	board_init_uart_f();

	/* Initialize periph GPIOs */
	gpio_early_init_uart();

	clock_early_init();
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif
	preloader_console_init();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_RAM;
}

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	debug("image entry point: 0x%lX\n", spl_image->entry_point);

	start_cpu((u32)spl_image->entry_point);
	halt_avp();
}
