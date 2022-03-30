/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 */
#ifndef _TI_COMMON_SYS_PROTO_H_
#define _TI_COMMON_SYS_PROTO_H_

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_ARCH_OMAP2PLUS
#define TI_ARMV7_DRAM_ADDR_SPACE_START	0x80000000
#define TI_ARMV7_DRAM_ADDR_SPACE_END	0xFFFFFFFF

#define OMAP_INIT_CONTEXT_SPL			0
#define OMAP_INIT_CONTEXT_UBOOT_FROM_NOR	1
#define OMAP_INIT_CONTEXT_UBOOT_AFTER_SPL	2
#define OMAP_INIT_CONTEXT_UBOOT_AFTER_CH	3

static inline u32 running_from_sdram(void)
{
	u32 pc;
	asm volatile ("mov %0, pc" : "=r" (pc));
	return ((pc >= TI_ARMV7_DRAM_ADDR_SPACE_START) &&
	    (pc < TI_ARMV7_DRAM_ADDR_SPACE_END));
}

static inline u8 uboot_loaded_by_spl(void)
{
	/*
	 * u-boot can be running from sdram either because of configuration
	 * Header or by SPL. If because of CH, then the romcode sets the
	 * CHSETTINGS executed bit to true in the boot parameter structure that
	 * it passes to the bootloader.This parameter is stored in the ch_flags
	 * variable by both SPL and u-boot.Check out for CHSETTINGS, which is a
	 * mandatory section if CH is present.
	 */
	if (gd->arch.omap_ch_flags & CH_FLAGS_CHSETTINGS)
		return 0;
	else
		return running_from_sdram();
}

/*
 * The basic hardware init of OMAP(s_init()) can happen in 4
 * different contexts:
 *  1. SPL running from SRAM
 *  2. U-Boot running from FLASH
 *  3. Non-XIP U-Boot loaded to SDRAM by SPL
 *  4. Non-XIP U-Boot loaded to SDRAM by ROM code using the
 *     Configuration Header feature
 *
 * This function finds this context.
 * Defining as inline may help in compiling out unused functions in SPL
 */
static inline u32 omap_hw_init_context(void)
{
#ifdef CONFIG_SPL_BUILD
	return OMAP_INIT_CONTEXT_SPL;
#else
	if (uboot_loaded_by_spl())
		return OMAP_INIT_CONTEXT_UBOOT_AFTER_SPL;
	else if (running_from_sdram())
		return OMAP_INIT_CONTEXT_UBOOT_AFTER_CH;
	else
		return OMAP_INIT_CONTEXT_UBOOT_FROM_NOR;
#endif
}
#endif

#endif
