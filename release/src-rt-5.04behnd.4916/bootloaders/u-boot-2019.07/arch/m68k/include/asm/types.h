#ifndef _M68K_TYPES_H
#define _M68K_TYPES_H

#include <asm-generic/int-ll64.h>

#ifndef __ASSEMBLY__

typedef unsigned short umode_t;

typedef struct {
	__u32 u[4];
} __attribute__((aligned(16))) vector128;

#ifdef __KERNEL__

#define BITS_PER_LONG 32

/* DMA addresses are 32-bits wide */
typedef u32 dma_addr_t;

typedef unsigned long phys_addr_t;
typedef unsigned long phys_size_t;

#endif /* __KERNEL__ */
#endif /* __ASSEMBLY__ */

#endif
