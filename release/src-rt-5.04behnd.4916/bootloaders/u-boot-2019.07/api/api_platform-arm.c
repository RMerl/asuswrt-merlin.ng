// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
 *
 * This file contains routines that fetch data from ARM-dependent sources
 * (bd_info etc.)
 */

#include <config.h>
#include <linux/types.h>
#include <api_public.h>

#include <asm/u-boot.h>
#include <asm/global_data.h>

#include "api_private.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Important notice: handling of individual fields MUST be kept in sync with
 * include/asm-arm/u-boot.h and include/asm-arm/global_data.h, so any changes
 * need to reflect their current state and layout of structures involved!
 */
int platform_sys_info(struct sys_info *si)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		platform_set_mr(si, gd->bd->bi_dram[i].start,
				gd->bd->bi_dram[i].size, MR_ATTR_DRAM);

	return 1;
}
