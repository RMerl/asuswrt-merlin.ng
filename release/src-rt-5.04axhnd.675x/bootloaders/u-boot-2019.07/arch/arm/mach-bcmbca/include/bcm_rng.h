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

#include <asm/arch/rng.h>

#define RNG_FIFO_DATA_BIT_MAX		(16*32) 
#define RNG_FIFO_DATA_MAX		(RNG_FIFO_DATA_BIT_MAX/8) 
#define RNG_FIFO_CNT_EMPTY_SHIFT	31
#define RNG_FIFO_CNT_FULL_SHIFT		30

int rng_pac_lock(uint32_t mode);
void rng_init(void);
int rng_get_rand(u8* data, int len);

#endif
