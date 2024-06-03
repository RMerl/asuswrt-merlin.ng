/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Andrew Ruder <andrew.ruder@elecsyscorp.com>
 */

#ifndef _ASM_ARM_PXA_CONFIG_
#define _ASM_ARM_PXA_CONFIG_

#include <asm/arch/pxa-regs.h>

/*
 * Generic timer support
 */
#if defined(CONFIG_CPU_PXA27X) || defined(CONFIG_CPU_MONAHANS)
#define	CONFIG_SYS_TIMER_RATE	3250000
#elif defined(CONFIG_CPU_PXA25X)
#define	CONFIG_SYS_TIMER_RATE	3686400
#else
#error "Timer frequency unknown - please config PXA CPU type"
#endif

#define CONFIG_SYS_TIMER_COUNTER	OSCR

#endif /* _ASM_ARM_PXA_CONFIG_ */
