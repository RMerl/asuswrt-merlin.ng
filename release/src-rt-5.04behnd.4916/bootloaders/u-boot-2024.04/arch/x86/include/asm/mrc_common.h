/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2016 Google, Inc
 */

#ifndef __ASM_MRC_COMMON_H
#define __ASM_MRC_COMMON_H

#include <linux/linkage.h>

/**
 * mrc_common_init() - Set up SDRAM
 *
 * This calls the memory reference code (MRC) to set up SDRAM
 *
 * @dev:	Northbridge device
 * @pei_data:	Platform-specific data required by the MRC
 * @use_asm_linkage: true if the call to MRC requires asmlinkage, false if it
 * uses normal U-Boot calling
 * Return: 0 if OK, -ve on error
 */
int mrc_common_init(struct udevice *dev, void *pei_data, bool use_asm_linkage);

asmlinkage void sdram_console_tx_byte(unsigned char byte);

int mrc_locate_spd(struct udevice *dev, int size, const void **spd_datap);

void report_memory_config(void);

/**
 * mrc_add_memory_area() - Add a new usable memory area to our list
 *
 * Note: @start and @end must not span the first 4GB boundary
 *
 * @info:	Place to store memory info
 * @start:	Start of this memory area
 * @end:	End of this memory area + 1
 */
int mrc_add_memory_area(struct memory_info *info, uint64_t start,
			  uint64_t end);

/*
 * This function looks for the highest region of memory lower than 4GB which
 * has enough space for U-Boot where U-Boot is aligned on a page boundary.
 * It overrides the default implementation found elsewhere which simply
 * picks the end of ram, wherever that may be. The location of the stack,
 * the relocation address, and how far U-Boot is moved by relocation are
 * set in the global data structure.
 */
phys_size_t mrc_common_board_get_usable_ram_top(phys_size_t total_size);

void mrc_common_dram_init_banksize(void);

#endif
