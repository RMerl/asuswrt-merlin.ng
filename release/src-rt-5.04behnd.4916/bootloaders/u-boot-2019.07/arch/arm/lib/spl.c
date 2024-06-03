// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010-2012
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 * Tom Rini <trini@ti.com>
 */

#include <common.h>
#include <config.h>
#include <spl.h>
#include <image.h>
#include <linux/compiler.h>
#include <asm/mach-types.h>

#ifndef CONFIG_SPL_DM
/* Pointer to as well as the global data structure for SPL */
DECLARE_GLOBAL_DATA_PTR;

/*
 * WARNING: This is going away very soon. Don't use it and don't submit
 * pafches that rely on it. The global_data area is set up in crt0.S.
 */
gd_t gdata __attribute__ ((section(".data")));
#endif

/*
 * In the context of SPL, board_init_f() prepares the hardware for execution
 * from system RAM (DRAM, DDR...). As system RAM may not be available yet,
 * board_init_f() must use the current GD to store any data which must be
 * passed on to later stages. These data include the relocation destination,
 * the future stack, and the future GD location. BSS is cleared after this
 * function (and therefore must be accessible).
 *
 * We provide this version by default but mark it as __weak to allow for
 * platforms to do this in their own way if needed. Please see the top
 * level U-Boot README "Board Initialization Flow" section for info on what
 * to put in this function.
 */
void __weak board_init_f(ulong dummy)
{
}

/*
 * This function jumps to an image with argument. Normally an FDT or ATAGS
 * image.
 */
#ifdef CONFIG_SPL_OS_BOOT
#ifdef CONFIG_ARM64
void __noreturn jump_to_image_linux(struct spl_image_info *spl_image)
{
	debug("Entering kernel arg pointer: 0x%p\n", spl_image->arg);
	cleanup_before_linux();
	armv8_switch_to_el2((u64)spl_image->arg, 0, 0, 0,
			    spl_image->entry_point, ES_TO_AARCH64);
}
#else
void __noreturn jump_to_image_linux(struct spl_image_info *spl_image)
{
	unsigned long machid = 0xffffffff;
#ifdef CONFIG_MACH_TYPE
	machid = CONFIG_MACH_TYPE;
#endif

	debug("Entering kernel arg pointer: 0x%p\n", spl_image->arg);
	typedef void (*image_entry_arg_t)(int, int, void *)
		__attribute__ ((noreturn));
	image_entry_arg_t image_entry =
		(image_entry_arg_t)(uintptr_t) spl_image->entry_point;
	cleanup_before_linux();
	image_entry(0, machid, spl_image->arg);
}
#endif	/* CONFIG_ARM64 */
#endif
