// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007 Stanislav Galabov <sgalabov@gmail.com>
 *
 * This file contains routines that fetch data from bd_info sources
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
 * include/asm-generic/u-boot.h, so any changes
 * need to reflect their current state and layout of structures involved!
 */
int platform_sys_info(struct sys_info *si)
{

	platform_set_mr(si, gd->bd->bi_memstart,
			gd->bd->bi_memsize, MR_ATTR_DRAM);

	return 1;
}
