/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 */

#ifndef __ASM_ARC_IO_H
#define __ASM_ARC_IO_H

#include <linux/types.h>
#include <asm/byteorder.h>

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
#define __iormb()		asm volatile("" : : : "memory")
#define __iowmb()		asm volatile("" : : : "memory")
#endif

static inline void sync(void)
{
	/* Not yet implemented */
}

static inline u8 __raw_readb(const volatile void __iomem *addr)
{
	u8 b;

	__asm__ __volatile__("ldb%U1	%0, %1\n"
			     : "=r" (b)
			     : "m" (*(volatile u8 __force *)addr)
			     : "memory");
	return b;
}

static inline u16 __raw_readw(const volatile void __iomem *addr)
{
	u16 s;

	__asm__ __volatile__("ldw%U1	%0, %1\n"
			     : "=r" (s)
			     : "m" (*(volatile u16 __force *)addr)
			     : "memory");
	return s;
}

static inline u32 __raw_readl(const volatile void __iomem *addr)
{
	u32 w;

	__asm__ __volatile__("ld%U1	%0, %1\n"
			     : "=r" (w)
			     : "m" (*(volatile u32 __force *)addr)
			     : "memory");
	return w;
}

static inline void __raw_writeb(u8 b, volatile void __iomem *addr)
{
	__asm__ __volatile__("stb%U1	%0, %1\n"
			     :
			     : "r" (b), "m" (*(volatile u8 __force *)addr)
			     : "memory");
}

static inline void __raw_writew(u16 s, volatile void __iomem *addr)
{
	__asm__ __volatile__("stw%U1	%0, %1\n"
			     :
			     : "r" (s), "m" (*(volatile u16 __force *)addr)
			     : "memory");
}

static inline void __raw_writel(u32 w, volatile void __iomem *addr)
{
	__asm__ __volatile__("st%U1	%0, %1\n"
			     :
			     : "r" (w), "m" (*(volatile u32 __force *)addr)
			     : "memory");
}

static inline int __raw_readsb(unsigned int addr, void *data, int bytelen)
{
	__asm__ __volatile__ ("1:ld.di	r8, [r0]\n"
			      "sub.f	r2, r2, 1\n"
			      "bnz.d	1b\n"
			      "stb.ab	r8, [r1, 1]\n"
			      :
			      : "r" (addr), "r" (data), "r" (bytelen)
			      : "r8");
	return bytelen;
}

static inline int __raw_readsw(unsigned int addr, void *data, int wordlen)
{
	__asm__ __volatile__ ("1:ld.di	r8, [r0]\n"
			      "sub.f	r2, r2, 1\n"
			      "bnz.d	1b\n"
			      "stw.ab	r8, [r1, 2]\n"
			      :
			      : "r" (addr), "r" (data), "r" (wordlen)
			      : "r8");
	return wordlen;
}

static inline int __raw_readsl(unsigned int addr, void *data, int longlen)
{
	__asm__ __volatile__ ("1:ld.di	r8, [r0]\n"
			      "sub.f	r2, r2, 1\n"
			      "bnz.d	1b\n"
			      "st.ab	r8, [r1, 4]\n"
			      :
			      : "r" (addr), "r" (data), "r" (longlen)
			      : "r8");
	return longlen;
}

static inline int __raw_writesb(unsigned int addr, void *data, int bytelen)
{
	__asm__ __volatile__ ("1:ldb.ab	r8, [r1, 1]\n"
			      "sub.f	r2, r2, 1\n"
			      "bnz.d	1b\n"
			      "st.di	r8, [r0, 0]\n"
			      :
			      : "r" (addr), "r" (data), "r" (bytelen)
			      : "r8");
	return bytelen;
}

static inline int __raw_writesw(unsigned int addr, void *data, int wordlen)
{
	__asm__ __volatile__ ("1:ldw.ab	r8, [r1, 2]\n"
			      "sub.f	r2, r2, 1\n"
			      "bnz.d	1b\n"
			      "st.ab.di	r8, [r0, 0]\n"
			      :
			      : "r" (addr), "r" (data), "r" (wordlen)
			      : "r8");
	return wordlen;
}

static inline int __raw_writesl(unsigned int addr, void *data, int longlen)
{
	__asm__ __volatile__ ("1:ld.ab	r8, [r1, 4]\n"
			      "sub.f	r2, r2, 1\n"
			      "bnz.d	1b\n"
			      "st.ab.di	r8, [r0, 0]\n"
			      :
			      : "r" (addr), "r" (data), "r" (longlen)
			      : "r8");
	return longlen;
}

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
#define readb(c)		({ u8  __v = readb_relaxed(c); __iormb(); __v; })
#define readw(c)		({ u16 __v = readw_relaxed(c); __iormb(); __v; })
#define readl(c)		({ u32 __v = readl_relaxed(c); __iormb(); __v; })

#define writeb(v,c)		({ __iowmb(); writeb_relaxed(v,c); })
#define writew(v,c)		({ __iowmb(); writew_relaxed(v,c); })
#define writel(v,c)		({ __iowmb(); writel_relaxed(v,c); })

/*
 * Relaxed API for drivers which can handle barrier ordering themselves
 *
 * Also these are defined to perform little endian accesses.
 * To provide the typical device register semantics of fixed endian,
 * swap the byte order for Big Endian
 *
 * http://lkml.kernel.org/r/201603100845.30602.arnd@arndb.de
 */
#define readb_relaxed(c)	__raw_readb(c)
#define readw_relaxed(c) ({ u16 __r = le16_to_cpu((__force __le16) \
					__raw_readw(c)); __r; })
#define readl_relaxed(c) ({ u32 __r = le32_to_cpu((__force __le32) \
					__raw_readl(c)); __r; })

#define writeb_relaxed(v,c)	__raw_writeb(v,c)
#define writew_relaxed(v,c)	__raw_writew((__force u16) cpu_to_le16(v),c)
#define writel_relaxed(v,c)	__raw_writel((__force u32) cpu_to_le32(v),c)

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
