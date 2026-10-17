/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2021 Broadcom Ltd.
 */

#ifndef _68375_MISC_H
#define _68375_MISC_H

/* PERF */
typedef struct PerfControl { /* GenInt */
	uint32_t        RevID;             /* (00) word 0 */
#define CHIP_ID_SHIFT   8
#define CHIP_ID_MASK    (0xfffff << CHIP_ID_SHIFT)
#define REV_ID_MASK     0xff
} PerfControl;

#define PERF_BASE              0x84210000
#define PERF ((volatile PerfControl * const) PERF_BASE)

#endif
