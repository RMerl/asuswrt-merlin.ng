/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Cadence Design Systems Inc.
 */

#ifndef _XTENSA_SYSTEM_H
#define _XTENSA_SYSTEM_H

#include <asm/arch/core.h>

#if XCHAL_HAVE_INTERRUPTS
#define local_irq_save(flags) \
	__asm__ __volatile__ ("rsil %0, %1" \
			      : "=a"(flags) \
			      : "I"(XCHAL_EXCM_LEVEL) \
			      : "memory")
#define local_irq_restore(flags) \
	__asm__ __volatile__ ("wsr %0, ps\n\t" \
			      "rsync" \
			      :: "a"(flags) : "memory")
#else
#define local_irq_save(flags) ((void)(flags))
#define local_irq_restore(flags) ((void)(flags))
#endif

#endif
