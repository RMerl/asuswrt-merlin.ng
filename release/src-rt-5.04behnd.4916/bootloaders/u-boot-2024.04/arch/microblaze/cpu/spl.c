// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 - 2014 Xilinx, Inc
 *
 * Michal Simek <michal.simek@amd.com>
 */

#include <command.h>
#include <image.h>
#include <log.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/u-boot.h>
#include <linux/stringify.h>

void board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = BOOT_DEVICE_NOR;
	spl_boot_list[1] = BOOT_DEVICE_RAM;
	spl_boot_list[2] = BOOT_DEVICE_SPI;
}

/* Board initialization after bss clearance */
void spl_board_init(void)
{
	/* enable console uart printing */
	preloader_console_init();
}

#ifdef CONFIG_SPL_OS_BOOT
void __noreturn jump_to_image_linux(struct spl_image_info *spl_image)
{
	debug("Entering kernel arg pointer: 0x%p\n", spl_image->arg);
	typedef void (*image_entry_arg_t)(char *, ulong, ulong)
		__attribute__ ((noreturn));
	image_entry_arg_t image_entry =
		(image_entry_arg_t)spl_image->entry_point;

	image_entry(NULL, 0, (ulong)spl_image->arg);
}

int spl_start_uboot(void)
{
	return 0;
}
#endif /* CONFIG_SPL_OS_BOOT */

int do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	__asm__ __volatile__ (
	    "mts rmsr, r0;" \
	    "brai " __stringify(CONFIG_XILINX_MICROBLAZE0_VECTOR_BASE_ADDR));

	return 0;
}
