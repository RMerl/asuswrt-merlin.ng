/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 1997 Tensilica Inc.
 */

#ifndef _XTENSA_TYPES_H
#define _XTENSA_TYPES_H

#include <asm-generic/int-ll64.h>

typedef unsigned short umode_t;

/*
 * These aren't exported outside the kernel to avoid name space clashes
 */
#ifdef __KERNEL__

#define BITS_PER_LONG 32

/* Dma addresses are 32-bits wide */

typedef u32 dma_addr_t;

typedef unsigned long phys_addr_t;
typedef unsigned long phys_size_t;


#endif /* __KERNEL__ */

#endif /* _XTENSA_TYPES_H */
