/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2014, 2020 Synopsys, Inc. All rights reserved.
 */

#ifndef __ASM_ARC_IO_H
#define __ASM_ARC_IO_H

#include <linux/types.h>
#include <asm/byteorder.h>

/*
 * Compiler barrier. It prevents compiler from reordering instructions before
 * and after it. It doesn't prevent HW (CPU) from any reordering though.
 */
#define __comp_b()		asm volatile("" : : : "memory")

#ifdef __ARCHS__

/*
 * ARCv2 based HS38 cores are in-order issue, but still weakly ordered
 * due to micro-arch buffering/queuing of load/store, cache hit vs. miss ...
 *
 * Explicit barrier provided by DMB instruction
 *  - Operand supports fine grained load/store/load+store semantics
 *  - Ensures that selected memory operation issued before it will complete
 *    before any subsequent memory operation of same type
 *  - DMB guarantees SMP as well as local barrier semantics
 *    (asm-generic/barrier.h ensures sane smp_*mb if not defined here, i.e.
 *    UP: barrier(), SMP: smp_*mb == *mb)
 *  - DSYNC provides DMB+completion_of_cache_bpu_maintenance_ops hence not needed
 *    in the general case. Plus it only provides full barrier.
 */

#define mb()	asm volatile("dmb 3\n" : : : "memory")
#define rmb()	asm volatile("dmb 1\n" : : : "memory")
#define wmb()	asm volatile("dmb 2\n" : : : "memory")

#else

/*
 * ARCompact based cores (ARC700) only have SYNC instruction which is super
 * heavy weight as it flushes the pipeline as well.
 * There are no real SMP implementations of such cores.
 */

#define mb()	asm volatile("sync\n" : : : "memory")
#endif

#ifdef __ARCHS__
#define __iormb()		rmb()
#define __iowmb()		wmb()
#else
#define __iormb()		__comp_b()
#define __iowmb()		__comp_b()
#endif

static inline void sync(void)
{
	/* Not yet implemented */
}

/*
 * We must use 'volatile' in C-version read/write IO accessors implementation
 * to avoid merging several reads (writes) into one read (write), or optimizing
 * them out by compiler.
 * We must use compiler barriers before and after operation (read or write) so
 * it won't be reordered by compiler.
 */
#define __arch_getb(a)		({ u8  __v; __comp_b(); __v = *(volatile u8  *)(a); __comp_b(); __v; })
#define __arch_getw(a)		({ u16 __v; __comp_b(); __v = *(volatile u16 *)(a); __comp_b(); __v; })
#define __arch_getl(a)		({ u32 __v; __comp_b(); __v = *(volatile u32 *)(a); __comp_b(); __v; })
#define __arch_getq(a)		({ u64 __v; __comp_b(); __v = *(volatile u64 *)(a); __comp_b(); __v; })

#define __arch_putb(v, a)	({ __comp_b(); *(volatile u8  *)(a) = (v); __comp_b(); })
#define __arch_putw(v, a)	({ __comp_b(); *(volatile u16 *)(a) = (v); __comp_b(); })
#define __arch_putl(v, a)	({ __comp_b(); *(volatile u32 *)(a) = (v); __comp_b(); })
#define __arch_putq(v, a)	({ __comp_b(); *(volatile u64 *)(a) = (v); __comp_b(); })


/*
 * We add memory barriers for __raw_readX / __raw_writeX accessors same way as
 * it is done for readX and writeX accessors as lots of U-Boot driver uses
 * __raw_readX / __raw_writeX instead of proper accessor with barrier.
 */
#define __raw_writeb(v, c)	({ __iowmb(); __arch_putb(v, c); })
#define __raw_writew(v, c)	({ __iowmb(); __arch_putw(v, c); })
#define __raw_writel(v, c)	({ __iowmb(); __arch_putl(v, c); })
#define __raw_writeq(v, c)	({ __iowmb(); __arch_putq(v, c); })

#define __raw_readb(c)		({ u8  __v = __arch_getb(c); __iormb(); __v; })
#define __raw_readw(c)		({ u16 __v = __arch_getw(c); __iormb(); __v; })
#define __raw_readl(c)		({ u32 __v = __arch_getl(c); __iormb(); __v; })
#define __raw_readq(c)		({ u64 __v = __arch_getq(c); __iormb(); __v; })


static inline void __raw_writesb(unsigned long addr, const void *data,
				 int bytelen)
{
	u8 *buf = (uint8_t *)data;

	__iowmb();

	while (bytelen--)
		__arch_putb(*buf++, addr);
}

static inline void __raw_writesw(unsigned long addr, const void *data,
				 int wordlen)
{
	u16 *buf = (uint16_t *)data;

	__iowmb();

	while (wordlen--)
		__arch_putw(*buf++, addr);
}

static inline void __raw_writesl(unsigned long addr, const void *data,
				 int longlen)
{
	u32 *buf = (uint32_t *)data;

	__iowmb();

	while (longlen--)
		__arch_putl(*buf++, addr);
}

static inline void __raw_readsb(unsigned long addr, void *data, int bytelen)
{
	u8 *buf = (uint8_t *)data;

	while (bytelen--)
		*buf++ = __arch_getb(addr);

	__iormb();
}

static inline void __raw_readsw(unsigned long addr, void *data, int wordlen)
{
	u16 *buf = (uint16_t *)data;

	while (wordlen--)
		*buf++ = __arch_getw(addr);

	__iormb();
}

static inline void __raw_readsl(unsigned long addr, void *data, int longlen)
{
	u32 *buf = (uint32_t *)data;

	while (longlen--)
		*buf++ = __arch_getl(addr);

	__iormb();
}

/*
 * Relaxed I/O memory access primitives. These follow the Device memory
 * ordering rules but do not guarantee any ordering relative to Normal memory
 * accesses.
 */
#define readb_relaxed(c)	({ u8  __r = __arch_getb(c); __r; })
#define readw_relaxed(c)	({ u16 __r = le16_to_cpu((__force __le16)__arch_getw(c)); __r; })
#define readl_relaxed(c)	({ u32 __r = le32_to_cpu((__force __le32)__arch_getl(c)); __r; })
#define readq_relaxed(c)	({ u64 __r = le64_to_cpu((__force __le64)__arch_getq(c)); __r; })

#define writeb_relaxed(v, c)	((void)__arch_putb((v), (c)))
#define writew_relaxed(v, c)	((void)__arch_putw((__force u16)cpu_to_le16(v), (c)))
#define writel_relaxed(v, c)	((void)__arch_putl((__force u32)cpu_to_le32(v), (c)))
#define writeq_relaxed(v, c)	((void)__arch_putq((__force u64)cpu_to_le64(v), (c)))

/*
 * MMIO can also get buffered/optimized in micro-arch, so barriers needed
 * Based on ARM model for the typical use case
 *
 *	<ST [DMA buffer]>
 *	<writel MMIO "go" reg>
 *  or:
 *	<readl MMIO "status" reg>
 *	<LD [DMA buffer]>
 *
 * http://lkml.kernel.org/r/20150622133656.GG1583@arm.com
 */
#define readb(c)	({ u8  __v = readb_relaxed(c); __iormb(); __v; })
#define readw(c)	({ u16 __v = readw_relaxed(c); __iormb(); __v; })
#define readl(c)	({ u32 __v = readl_relaxed(c); __iormb(); __v; })
#define readq(c)	({ u64 __v = readq_relaxed(c); __iormb(); __v; })

#define writeb(v, c)	({ __iowmb(); writeb_relaxed(v, c); })
#define writew(v, c)	({ __iowmb(); writew_relaxed(v, c); })
#define writel(v, c)	({ __iowmb(); writel_relaxed(v, c); })
#define writeq(v, c)	({ __iowmb(); writeq_relaxed(v, c); })

#define out_arch(type, endian, a, v)	__raw_write##type(cpu_to_##endian(v), a)
#define in_arch(type, endian, a)	endian##_to_cpu(__raw_read##type(a))

#define out_le32(a, v)	out_arch(l, le32, a, v)
#define out_le16(a, v)	out_arch(w, le16, a, v)

#define in_le32(a)	in_arch(l, le32, a)
#define in_le16(a)	in_arch(w, le16, a)

#define out_be32(a, v)	out_arch(l, be32, a, v)
#define out_be16(a, v)	out_arch(w, be16, a, v)

#define in_be32(a)	in_arch(l, be32, a)
#define in_be16(a)	in_arch(w, be16, a)

#define out_8(a, v)	__raw_writeb(v, a)
#define in_8(a)		__raw_readb(a)

/*
 * Clear and set bits in one shot. These macros can be used to clear and
 * set multiple bits in a register using a single call. These macros can
 * also be used to set a multiple-bit bit pattern using a mask, by
 * specifying the mask in the 'clear' parameter and the new bit pattern
 * in the 'set' parameter.
 */

#define clrbits(type, addr, clear) \
	out_##type((addr), in_##type(addr) & ~(clear))

#define setbits(type, addr, set) \
	out_##type((addr), in_##type(addr) | (set))

#define clrsetbits(type, addr, clear, set) \
	out_##type((addr), (in_##type(addr) & ~(clear)) | (set))

#define clrbits_be32(addr, clear) clrbits(be32, addr, clear)
#define setbits_be32(addr, set) setbits(be32, addr, set)
#define clrsetbits_be32(addr, clear, set) clrsetbits(be32, addr, clear, set)

#define clrbits_le32(addr, clear) clrbits(le32, addr, clear)
#define setbits_le32(addr, set) setbits(le32, addr, set)
#define clrsetbits_le32(addr, clear, set) clrsetbits(le32, addr, clear, set)

#define clrbits_be16(addr, clear) clrbits(be16, addr, clear)
#define setbits_be16(addr, set) setbits(be16, addr, set)
#define clrsetbits_be16(addr, clear, set) clrsetbits(be16, addr, clear, set)

#define clrbits_le16(addr, clear) clrbits(le16, addr, clear)
#define setbits_le16(addr, set) setbits(le16, addr, set)
#define clrsetbits_le16(addr, clear, set) clrsetbits(le16, addr, clear, set)

#define clrbits_8(addr, clear) clrbits(8, addr, clear)
#define setbits_8(addr, set) setbits(8, addr, set)
#define clrsetbits_8(addr, clear, set) clrsetbits(8, addr, clear, set)

#include <asm-generic/io.h>

#endif	/* __ASM_ARC_IO_H */
