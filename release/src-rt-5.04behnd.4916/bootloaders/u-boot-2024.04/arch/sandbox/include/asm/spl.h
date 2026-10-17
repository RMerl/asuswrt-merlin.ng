/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Google, Inc
 */

#ifndef __asm_spl_h
#define __asm_spl_h

enum {
	BOOT_DEVICE_MMC1,
	BOOT_DEVICE_MMC2,
	BOOT_DEVICE_MMC2_2,
	BOOT_DEVICE_BOARD,
	BOOT_DEVICE_VBE,
	BOOT_DEVICE_CPGMAC,
	BOOT_DEVICE_NOR,
	BOOT_DEVICE_SPI,
	BOOT_DEVICE_NAND,
};

/**
 * sandbox_find_next_phase() - Find the next phase of U-Boot
 *
 * This function is intended to be called from within sandbox SPL. It uses
 * a few rules to find the filename of the next U-Boot phase. See also
 * os_find_u_boot().
 *
 * @fname:	place to put full path to U-Boot
 * @maxlen:	maximum size of @fname
 * @use_img:	select the 'u-boot.img' file instead of the 'u-boot' ELF file
 */
int sandbox_find_next_phase(char *fname, int maxlen, bool use_img);

#endif
