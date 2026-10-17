/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

#ifndef __ASM_ARC_TYPES_H
#define __ASM_ARC_TYPES_H

#include <asm-generic/int-ll64.h>

typedef unsigned short umode_t;

#define BITS_PER_LONG 32

/* Dma addresses are 32-bits wide. */

typedef u32 dma_addr_t;

typedef unsigned long phys_addr_t;
typedef unsigned long phys_size_t;

#endif /* __ASM_ARC_TYPES_H */
