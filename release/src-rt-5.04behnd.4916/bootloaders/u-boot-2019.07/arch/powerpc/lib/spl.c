// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2012 Stefan Roese <sr@denx.de>
 */
#include <common.h>
#include <config.h>
#include <spl.h>
#include <image.h>
#include <linux/compiler.h>

/*
 * This function jumps to an image with argument. Normally an FDT or ATAGS
 * image.
 */
#ifdef CONFIG_SPL_OS_BOOT
void __noreturn jump_to_image_linux(struct spl_image_info *spl_image)
{
	debug("Entering kernel arg pointer: 0x%p\n", spl_image->arg);
	typedef void (*image_entry_arg_t)(void *, ulong r4, ulong r5, ulong r6,
					  ulong r7, ulong r8, ulong r9)
		__attribute__ ((noreturn));
	image_entry_arg_t image_entry =
		(image_entry_arg_t)spl_image->entry_point;

	image_entry(spl_image->arg, 0, 0, EPAPR_MAGIC, CONFIG_SYS_BOOTMAPSZ,
		    0, 0);
}
#endif /* CONFIG_SPL_OS_BOOT */
