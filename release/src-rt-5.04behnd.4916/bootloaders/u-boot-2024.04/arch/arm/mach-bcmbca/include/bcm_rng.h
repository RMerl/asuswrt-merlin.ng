// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2017 Broadcom
 */
/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2019 Broadcom Ltd.
 */
/*
 * 
 */

#ifndef __BCM_RNG_H__
#define __BCM_RNG_H__

typedef enum {
	RNG_PERM_NSEC_ACCESS_ENABLE,	/* Also ENABLES sec access */
	RNG_PERM_SEC_ACCESS_ENABLE,	/* Also DISABLES nonsec access */
	RNG_PERM_ALL_ACCESS_DISABLE
} RNG_PERM;

int rng_pac_lock(uint32_t mode);
void rng_init(void);
int rng_get_rand(u8* data, int len);

#endif
