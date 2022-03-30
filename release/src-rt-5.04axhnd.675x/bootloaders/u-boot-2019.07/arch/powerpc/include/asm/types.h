#ifndef _PPC_TYPES_H
#define _PPC_TYPES_H

#include <asm-generic/int-ll64.h>

#ifndef __ASSEMBLY__

typedef unsigned short umode_t;

typedef struct {
	__u32 u[4];
} __attribute__((aligned(16))) vector128;

#ifdef __KERNEL__

#define BITS_PER_LONG 32

#ifdef CONFIG_PHYS_64BIT
typedef unsigned long long dma_addr_t;
#else
/* DMA addresses are 32-bits wide */
typedef u32 dma_addr_t;
#endif

#ifdef CONFIG_PHYS_64BIT
typedef unsigned long long phys_addr_t;
typedef unsigned long long phys_size_t;
#else
typedef unsigned long phys_addr_t;
typedef unsigned long phys_size_t;
#endif

#endif /* __KERNEL__ */
#endif /* __ASSEMBLY__ */

#endif
