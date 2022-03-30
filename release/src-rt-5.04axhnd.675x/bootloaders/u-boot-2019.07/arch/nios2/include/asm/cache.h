/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#ifndef __ASM_NIOS2_CACHE_H_
#define __ASM_NIOS2_CACHE_H_

/*
 * Valid L1 data cache line sizes for the NIOS2 architecture are 4,
 * 16, and 32 bytes. We default to the largest of these values for
 * alignment of DMA buffers.
 */
#define ARCH_DMA_MINALIGN	32

#endif /* __ASM_NIOS2_CACHE_H_ */
