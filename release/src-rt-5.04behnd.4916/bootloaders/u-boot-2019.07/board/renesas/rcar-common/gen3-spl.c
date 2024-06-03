// SPDX-License-Identifier: GPL-2.0
/*
 * R-Car Gen3 recovery SPL
 *
 * Copyright (C) 2019 Marek Vasut <marek.vasut@gmail.com>
 */

#include <common.h>
#include <asm/io.h>
#include <spl.h>

#define RCAR_CNTC_BASE	0xE6080000
#define CNTCR_EN	BIT(0)

void board_init_f(ulong dummy)
{
	writel(CNTCR_EN, RCAR_CNTC_BASE);
	timer_init();
}

void spl_board_init(void)
{
	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();
}

u32 spl_boot_device(void)
{
	return BOOT_DEVICE_UART;
}

void __noreturn jump_to_image_no_args(struct spl_image_info *spl_image)
{
	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	if (spl_image->os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		typedef void (*image_entry_arg_t)(int, int, int, int)
			__attribute__ ((noreturn));
		image_entry_arg_t image_entry =
			(image_entry_arg_t)(uintptr_t) spl_image->entry_point;
		image_entry(IH_MAGIC, CONFIG_SPL_TEXT_BASE, 0, 0);
	} else {
		typedef void __noreturn (*image_entry_noargs_t)(void);
		image_entry_noargs_t image_entry =
			(image_entry_noargs_t)spl_image->entry_point;
		image_entry();
	}
}

void s_init(void)
{
}

void reset_cpu(ulong addr)
{
}
