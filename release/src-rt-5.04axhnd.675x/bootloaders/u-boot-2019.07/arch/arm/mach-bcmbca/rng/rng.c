// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2019 Broadcom Ltd.
 */
/*
 * 
 */
// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i4.cfg

#include <common.h>
#include <linux/types.h>
#include "linux/printk.h"
#include <asm/arch/rng.h>
#include <asm/arch/misc.h>
#include "bcm_rng.h"


int rng_pac_lock(uint32_t perm)
{
    uint32_t startRegion;
    if (perm == RNG_PERM_DISABLE_ALL)  {
        RNG->perm = perm; 
    } else {
        RNG->perm |= perm; 
    }
}

