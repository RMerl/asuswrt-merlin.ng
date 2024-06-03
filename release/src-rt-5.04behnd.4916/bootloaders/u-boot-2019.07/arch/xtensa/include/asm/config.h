/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2009 Tensilica Inc.
 * Copyright (C) 2014 - 2016 Cadence Design Systems Inc.
 */

#ifndef _ASM_CONFIG_H_
#define _ASM_CONFIG_H_

#include <asm/arch/core.h>

#define CONFIG_LMB

/*
 * Make boot parameters available in the MMUv2 virtual memory layout by
 * restricting used physical memory to the first 128MB.
 */
#if XCHAL_HAVE_PTP_MMU
#define CONFIG_VERY_BIG_RAM
#define CONFIG_MAX_MEM_MAPPED (128 << 20)
#endif

#endif
