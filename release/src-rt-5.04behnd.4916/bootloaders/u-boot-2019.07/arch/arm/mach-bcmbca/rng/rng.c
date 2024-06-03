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
#include "bcm_rng.h"

__weak void rng_init(void) {}

__weak int rng_get_rand(u8* data, int len)
{
    return 0;
}

#ifndef CONFIG_SMC_BASED
__weak int rng_pac_lock(uint32_t perm)
{
    return 0;
}
#endif
