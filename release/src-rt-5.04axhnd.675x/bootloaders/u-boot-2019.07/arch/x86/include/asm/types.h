#ifndef __ASM_I386_TYPES_H
#define __ASM_I386_TYPES_H

#include <asm-generic/int-ll64.h>

typedef unsigned short umode_t;

/*
 * These aren't exported outside the kernel to avoid name space clashes
 */
#ifdef __KERNEL__

#if CONFIG_IS_ENABLED(X86_64)
#define BITS_PER_LONG 64
#else
#define BITS_PER_LONG 32
#endif

/* Dma addresses are 32-bits wide.  */

typedef u32 dma_addr_t;

typedef unsigned long long phys_addr_t;
typedef unsigned long long phys_size_t;

#endif /* __KERNEL__ */

#endif
