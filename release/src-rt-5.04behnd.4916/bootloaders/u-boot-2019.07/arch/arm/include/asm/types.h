#ifndef __ASM_ARM_TYPES_H
#define __ASM_ARM_TYPES_H

#include <asm-generic/int-ll64.h>

typedef unsigned short umode_t;

/*
 * These aren't exported outside the kernel to avoid name space clashes
 */
#ifdef __KERNEL__

#ifdef	CONFIG_ARM64
#define BITS_PER_LONG 64
#else	/* CONFIG_ARM64 */
#define BITS_PER_LONG 32
#endif	/* CONFIG_ARM64 */

#ifdef CONFIG_PHYS_64BIT
typedef unsigned long long phys_addr_t;
typedef unsigned long long phys_size_t;
#else
/* DMA addresses are 32-bits wide */
typedef unsigned long phys_addr_t;
typedef unsigned long phys_size_t;
#endif

/*
 * A dma_addr_t can hold any valid DMA address, i.e., any address returned
 * by the DMA API.
 *
 * If the DMA API only uses 32-bit addresses, dma_addr_t need only be 32
 * bits wide.  Bus addresses, e.g., PCI BARs, may be wider than 32 bits,
 * but drivers do memory-mapped I/O to ioremapped kernel virtual addresses,
 * so they don't care about the size of the actual bus addresses.
 */
#ifdef CONFIG_DMA_ADDR_T_64BIT
typedef unsigned long long dma_addr_t;
#else
typedef u32 dma_addr_t;
#endif

#endif /* __KERNEL__ */

#endif
