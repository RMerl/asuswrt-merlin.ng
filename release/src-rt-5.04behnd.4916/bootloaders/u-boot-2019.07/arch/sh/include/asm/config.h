/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2009 Freescale Semiconductor, Inc.
 */

#ifndef _ASM_CONFIG_H_
#define _ASM_CONFIG_H_

#include <asm/processor.h>

/* Timer */
#define CONFIG_SYS_TIMER_COUNTS_DOWN
#define CONFIG_SYS_TIMER_COUNTER	(TMU_BASE + 0x8)	/* TCNT0 */
#define CONFIG_SYS_TIMER_RATE		(CONFIG_SYS_CLK_FREQ / 4)

#endif
