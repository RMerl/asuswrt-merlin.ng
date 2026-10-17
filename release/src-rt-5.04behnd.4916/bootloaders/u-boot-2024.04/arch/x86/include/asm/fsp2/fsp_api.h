/* SPDX-License-Identifier: Intel */
/*
 * Copyright (C) 2015-2016 Intel Corp.
 * (Written by Andrey Petrov <andrey.petrov@intel.com> for Intel Corp.)
 * (Written by Alexandru Gagniuc <alexandrux.gagniuc@intel.com> for Intel Corp.)
 * Mostly taken from coreboot fsp2_0/memory_init.c
 */

#ifndef __ASM_FSP2_API_H
#define __ASM_FSP2_API_H

#include <asm/fsp/fsp_api.h>

struct fspm_upd;
struct fsps_upd;
struct hob_header;

enum fsp_boot_mode {
	FSP_BOOT_WITH_FULL_CONFIGURATION = 0x00,
	FSP_BOOT_WITH_MINIMAL_CONFIGURATION = 0x01,
	FSP_BOOT_ASSUMING_NO_CONFIGURATION_CHANGES = 0x02,
	FSP_BOOT_ON_S4_RESUME = 0x05,
	FSP_BOOT_ON_S3_RESUME = 0x11,
	FSP_BOOT_ON_FLASH_UPDATE = 0x12,
	FSP_BOOT_IN_RECOVERY_MODE = 0x20
};

struct __packed fsp_upd_header {
	u64	signature;
	u8	revision;
	u8	reserved[23];
};

/**
 * fsp_memory_init() - Init the SDRAM
 *
 * @s3wake: true if we are booting from resume, so cannot reinit the mememory
 *	from scatch since we will lose its contents
 * @use_spi_flash: true to use the fast SPI driver to read FSP, otherwise use
 *	mapped SPI
 * Return: 0 if OK, -ve on error
 */
int fsp_memory_init(bool s3wake, bool use_spi_flash);

typedef asmlinkage int (*fsp_memory_init_func)(struct fspm_upd *params,
					       struct hob_header **hobp);

/**
 * fsp_silicon_init() - Init the silicon
 *
 * This calls the FSP's 'silicon init' entry point
 *
 * @s3wake: true if we are booting from resume, so cannot reinit the mememory
 *	from scatch since we will lose its contents
 * @use_spi_flash: true to use the fast SPI driver to read FSP, otherwise use
 *	mapped SPI
 * Return: 0 if OK, -ve on error
 */
int fsp_silicon_init(bool s3wake, bool use_spi_flash);

typedef asmlinkage int (*fsp_silicon_init_func)(struct fsps_upd *params);

/**
 *  fsp_setup_pinctrl() - Set up the pinctrl for FSP
 *
 * @ctx: Event context (not used)
 * @event: Event information (not used)
 */
int fsp_setup_pinctrl(void *ctx, struct event *event);

#endif
