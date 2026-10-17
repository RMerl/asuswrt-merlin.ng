/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __CLOCK_H__
#define __CLOCK_H__

/** System PLL reference clock */
#define PLL_REF_CLK                     50000000        /* 50 MHz */
#define NS_PER_REF_CLK_TICK             (1000000000 / PLL_REF_CLK)

/**
 * Returns the I/O clock speed in Hz
 */
u64 octeontx_get_io_clock(void);

/**
 * Returns the core clock speed in Hz
 */
u64 octeontx_get_core_clock(void);

#endif /* __CLOCK_H__ */
