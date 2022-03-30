/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 1994, 1995 Waldorf GmbH
 * Copyright (C) 1994 - 2000, 06 Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 * Copyright (C) 2004, 2005  MIPS Technologies, Inc.  All rights reserved.
 *	Author: Maciej W. Rozycki <macro@mips.com>
 */
#ifndef _ASM_IO_H
#define _ASM_IO_H

#include <linux/bug.h>
#include <linux/compiler.h>
#include <linux/types.h>

#include <asm/addrspace.h>
#include <asm/byteorder.h>
#include <asm/cpu-features.h>
#include <asm/pgtable-bits.h>
#include <asm/processor.h>
#include <asm/string.h>

#include <ioremap.h>
#include <mangle-port.h>
#include <spaces.h>

/*
 * Raw operations are never swapped in software.  OTOH values that raw
 * operations are working on may or may not have been swapped by the bus
 * hardware.  An example use would be for flash memory that's used for
 * execute in place.
 */
# define __raw_ioswabb(a, x)	(x)
# define __raw_ioswabw(a, x)	(x)
# define __raw_ioswabl(a, x)	(x)
# define __raw_ioswabq(a, x)	(x)
# define ____raw_ioswabq(a, x)	(x)

/* ioswab[bwlq], __mem_ioswab[bwlq] are defined in mangle-port.h */

#define IO_SPACE_LIMIT 0xffff

#ifdef CONFIG_DYNAMIC_IO_PORT_BASE

static inline ulong mips_io_port_base(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	return gd->arch.io_port_base;
}

static inline void set_io_port_base(unsigned long base)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->arch.io_port_base = base;
	barrier();
}

#else /* !CONFIG_DYNAMIC_IO_PORT_BASE */

static inline ulong mips_io_port_base(void)
{
	return 0;
}

static inline void set_io_port_base(unsigned long base)
{
	BUG_ON(base);
}

#endif /* !CONFIG_DYNAMIC_IO_PORT_BASE */

/*
 *     virt_to_phys    -       map virtual addresses to physical
 *     @address: address to remap
 *
 *     The returned physical address is the physical (CPU) mapping for
 *     the memory address given. It is only valid to use this function on
 *     addresses directly mapped or allocated via kmalloc.
 *
 *     This function does not give bus mappings for DMA transfers. In
 *     almost all conceivable cases a device driver should not be using
 *     this function
 */
static inline unsigned long virt_to_phys(volatile const void *address)
{
	unsigned long addr = (unsigned long)address;

	/* this corresponds to kernel implementation of __pa() */
#ifdef CONFIG_64BIT
	if (addr < CKSEG0)
		return XPHYSADDR(addr);
#endif
	return CPHYSADDR(addr);
}
#define virt_to_phys virt_to_phys

/*
 *     phys_to_virt    -       map physical address to virtual
 *     @address: address to remap
 *
 *     The returned virtual address is a current CPU mapping for
 *     the memory address given. It is only valid to use this function on
 *     addresses that have a kernel mapping
 *
 *     This function does not handle bus mappings for DMA transfers. In
 *     almost all conceivable cases a device driver should not be using
 *     this function
 */
static inline void *phys_to_virt(unsigned long address)
{
	return (void *)(address + PAGE_OFFSET - PHYS_OFFSET);
}
#define phys_to_virt phys_to_virt

/*
 * ISA I/O bus memory addresses are 1:1 with the physical address.
 */
static inline unsigned long isa_virt_to_bus(volatile void *address)
{
	return (unsigned long)address - PAGE_OFFSET;
}

static inline void *isa_bus_to_virt(unsigned long address)
{
	return (void *)(address + PAGE_OFFSET);
}

#define isa_page_to_bus page_to_phys

/*
 * However PCI ones are not necessarily 1:1 and therefore these interfaces
 * are forbidden in portable PCI drivers.
 *
 * Allow them for x86 for legacy drivers, though.
 */
#define virt_to_bus virt_to_phys
#define bus_to_virt phys_to_virt

static inline void __iomem *__ioremap_mode(phys_addr_t offset, unsigned long size,
	unsigned long flags)
{
	void __iomem *addr;
	phys_addr_t phys_addr;

	addr = plat_ioremap(offset, size, flags);
	if (addr)
		return addr;

	phys_addr = fixup_bigphys_addr(offset, size);
	return (void __iomem *)(unsigned long)CKSEG1ADDR(phys_addr);
}

/*
 * ioremap     -   map bus memory into CPU space
 * @offset:    bus address of the memory
 * @size:      size of the resource to map
 *
 * ioremap performs a platform specific sequence of operations to
 * make bus memory CPU accessible via the readb/readw/readl/writeb/
 * writew/writel functions and the other mmio helpers. The returned
 * address is not guaranteed to be usable directly as a virtual
 * address.
 */
#define ioremap(offset, size)						\
	__ioremap_mode((offset), (size), _CACHE_UNCACHED)

/*
 * ioremap_nocache     -   map bus memory into CPU space
 * @offset:    bus address of the memory
 * @size:      size of the resource to map
 *
 * ioremap_nocache performs a platform specific sequence of operations to
 * make bus memory CPU accessible via the readb/readw/readl/writeb/
 * writew/writel functions and the other mmio helpers. The returned
 * address is not guaranteed to be usable directly as a virtual
 * address.
 *
 * This version of ioremap ensures that the memory is marked uncachable
 * on the CPU as well as honouring existing caching rules from things like
 * the PCI bus. Note that there are other caches and buffers on many
 * busses. In particular driver authors should read up on PCI writes
 *
 * It's useful if some control registers are in such an area and
 * write combining or read caching is not desirable:
 */
#define ioremap_nocache(offset, size)					\
	__ioremap_mode((offset), (size), _CACHE_UNCACHED)
#define ioremap_uc ioremap_nocache

/*
 * ioremap_cachable -	map bus memory into CPU space
 * @offset:	    bus address of the memory
 * @size:	    size of the resource to map
 *
 * ioremap_nocache performs a platform specific sequence of operations to
 * make bus memory CPU accessible via the readb/readw/readl/writeb/
 * writew/writel functions and the other mmio helpers. The returned
 * address is not guaranteed to be usable directly as a virtual
 * address.
 *
 * This version of ioremap ensures that the memory is marked cachable by
 * the CPU.  Also enables full write-combining.	 Useful for some
 * memory-like regions on I/O busses.
 */
#define ioremap_cachable(offset, size)					\
	__ioremap_mode((offset), (size), _page_cachable_default)

/*
 * These two are MIPS specific ioremap variant.	 ioremap_cacheable_cow
 * requests a cachable mapping, ioremap_uncached_accelerated requests a
 * mapping using the uncached accelerated mode which isn't supported on
 * all processors.
 */
#define ioremap_cacheable_cow(offset, size)				\
	__ioremap_mode((offset), (size), _CACHE_CACHABLE_COW)
#define ioremap_uncached_accelerated(offset, size)			\
	__ioremap_mode((offset), (size), _CACHE_UNCACHED_ACCELERATED)

static inline void iounmap(const volatile void __iomem *addr)
{
	plat_iounmap(addr);
}

#ifdef CONFIG_CPU_CAVIUM_OCTEON
#define war_octeon_io_reorder_wmb()		wmb()
#else
#define war_octeon_io_reorder_wmb()		do { } while (0)
#endif

#define __BUILD_MEMORY_SINGLE(pfx, bwlq, type, irq)			\
									\
static inline void pfx##write##bwlq(type val,				\
				    volatile void __iomem *mem)		\
{									\
	volatile type *__mem;						\
	type __val;							\
									\
	war_octeon_io_reorder_wmb();					\
									\
	__mem = (void *)__swizzle_addr_##bwlq((unsigned long)(mem));	\
									\
	__val = pfx##ioswab##bwlq(__mem, val);				\
									\
	if (sizeof(type) != sizeof(u64) || sizeof(u64) == sizeof(long)) \
		*__mem = __val;						\
	else if (cpu_has_64bits) {					\
		type __tmp;						\
									\
		__asm__ __volatile__(					\
			".set	arch=r4000"	"\t\t# __writeq""\n\t"	\
			"dsll32 %L0, %L0, 0"			"\n\t"	\
			"dsrl32 %L0, %L0, 0"			"\n\t"	\
			"dsll32 %M0, %M0, 0"			"\n\t"	\
			"or	%L0, %L0, %M0"			"\n\t"	\
			"sd	%L0, %2"			"\n\t"	\
			".set	mips0"				"\n"	\
			: "=r" (__tmp)					\
			: "0" (__val), "m" (*__mem));			\
	} else								\
		BUG();							\
}									\
									\
static inline type pfx##read##bwlq(const volatile void __iomem *mem)	\
{									\
	volatile type *__mem;						\
	type __val;							\
									\
	__mem = (void *)__swizzle_addr_##bwlq((unsigned long)(mem));	\
									\
	if (sizeof(type) != sizeof(u64) || sizeof(u64) == sizeof(long)) \
		__val = *__mem;						\
	else if (cpu_has_64bits) {					\
		__asm__ __volatile__(					\
			".set	arch=r4000"	"\t\t# __readq" "\n\t"	\
			"ld	%L0, %1"			"\n\t"	\
			"dsra32 %M0, %L0, 0"			"\n\t"	\
			"sll	%L0, %L0, 0"			"\n\t"	\
			".set	mips0"				"\n"	\
			: "=r" (__val)					\
			: "m" (*__mem));				\
	} else {							\
		__val = 0;						\
		BUG();							\
	}								\
									\
	return pfx##ioswab##bwlq(__mem, __val);				\
}

#define __BUILD_IOPORT_SINGLE(pfx, bwlq, type, p)			\
									\
static inline void pfx##out##bwlq##p(type val, unsigned long port)	\
{									\
	volatile type *__addr;						\
	type __val;							\
									\
	war_octeon_io_reorder_wmb();					\
									\
	__addr = (void *)__swizzle_addr_##bwlq(mips_io_port_base() + port); \
									\
	__val = pfx##ioswab##bwlq(__addr, val);				\
									\
	/* Really, we want this to be atomic */				\
	BUILD_BUG_ON(sizeof(type) > sizeof(unsigned long));		\
									\
	*__addr = __val;						\
}									\
									\
static inline type pfx##in##bwlq##p(unsigned long port)			\
{									\
	volatile type *__addr;						\
	type __val;							\
									\
	__addr = (void *)__swizzle_addr_##bwlq(mips_io_port_base() + port); \
									\
	BUILD_BUG_ON(sizeof(type) > sizeof(unsigned long));		\
									\
	__val = *__addr;						\
									\
	return pfx##ioswab##bwlq(__addr, __val);			\
}

#define __BUILD_MEMORY_PFX(bus, bwlq, type)				\
									\
__BUILD_MEMORY_SINGLE(bus, bwlq, type, 1)

#define BUILDIO_MEM(bwlq, type)						\
									\
__BUILD_MEMORY_PFX(__raw_, bwlq, type)					\
__BUILD_MEMORY_PFX(, bwlq, type)					\
__BUILD_MEMORY_PFX(__mem_, bwlq, type)					\

BUILDIO_MEM(b, u8)
BUILDIO_MEM(w, u16)
BUILDIO_MEM(l, u32)
BUILDIO_MEM(q, u64)

#define __BUILD_IOPORT_PFX(bus, bwlq, type)				\
	__BUILD_IOPORT_SINGLE(bus, bwlq, type, )			\
	__BUILD_IOPORT_SINGLE(bus, bwlq, type, _p)

#define BUILDIO_IOPORT(bwlq, type)					\
	__BUILD_IOPORT_PFX(, bwlq, type)				\
	__BUILD_IOPORT_PFX(__mem_, bwlq, type)

BUILDIO_IOPORT(b, u8)
BUILDIO_IOPORT(w, u16)
BUILDIO_IOPORT(l, u32)
#ifdef CONFIG_64BIT
BUILDIO_IOPORT(q, u64)
#endif

#define __BUILDIO(bwlq, type)						\
									\
__BUILD_MEMORY_SINGLE(____raw_, bwlq, type, 0)

__BUILDIO(q, u64)

#define readb_relaxed			readb
#define readw_relaxed			readw
#define readl_relaxed			readl
#define readq_relaxed			readq

#define writeb_relaxed			writeb
#define writew_relaxed			writew
#define writel_relaxed			writel
#define writeq_relaxed			writeq

#define readb_be(addr)							\
	__raw_readb((__force unsigned *)(addr))
#define readw_be(addr)							\
	be16_to_cpu(__raw_readw((__force unsigned *)(addr)))
#define readl_be(addr)							\
	be32_to_cpu(__raw_readl((__force unsigned *)(addr)))
#define readq_be(addr)							\
	be64_to_cpu(__raw_readq((__force unsigned *)(addr)))

#define writeb_be(val, addr)						\
	__raw_writeb((val), (__force unsigned *)(addr))
#define writew_be(val, addr)						\
	__raw_writew(cpu_to_be16((val)), (__force unsigned *)(addr))
#define writel_be(val, addr)						\
	__raw_writel(cpu_to_be32((val)), (__force unsigned *)(addr))
#define writeq_be(val, addr)						\
	__raw_writeq(cpu_to_be64((val)), (__force unsigned *)(addr))

/*
 * Some code tests for these symbols
 */
#define readq				readq
#define writeq				writeq

#define __BUILD_MEMORY_STRING(bwlq, type)				\
									\
static inline void writes##bwlq(volatile void __iomem *mem,		\
				const void *addr, unsigned int count)	\
{									\
	const volatile type *__addr = addr;				\
									\
	while (count--) {						\
		__mem_write##bwlq(*__addr, mem);			\
		__addr++;						\
	}								\
}									\
									\
static inline void reads##bwlq(volatile void __iomem *mem, void *addr,	\
			       unsigned int count)			\
{									\
	volatile type *__addr = addr;					\
									\
	while (count--) {						\
		*__addr = __mem_read##bwlq(mem);			\
		__addr++;						\
	}								\
}

#define __BUILD_IOPORT_STRING(bwlq, type)				\
									\
static inline void outs##bwlq(unsigned long port, const void *addr,	\
			      unsigned int count)			\
{									\
	const volatile type *__addr = addr;				\
									\
	while (count--) {						\
		__mem_out##bwlq(*__addr, port);				\
		__addr++;						\
	}								\
}									\
									\
static inline void ins##bwlq(unsigned long port, void *addr,		\
			     unsigned int count)			\
{									\
	volatile type *__addr = addr;					\
									\
	while (count--) {						\
		*__addr = __mem_in##bwlq(port);				\
		__addr++;						\
	}								\
}

#define BUILDSTRING(bwlq, type)						\
									\
__BUILD_MEMORY_STRING(bwlq, type)					\
__BUILD_IOPORT_STRING(bwlq, type)

BUILDSTRING(b, u8)
BUILDSTRING(w, u16)
BUILDSTRING(l, u32)
#ifdef CONFIG_64BIT
BUILDSTRING(q, u64)
#endif


#ifdef CONFIG_CPU_CAVIUM_OCTEON
#define mmiowb() wmb()
#else
/* Depends on MIPS II instruction set */
#define mmiowb() asm volatile ("sync" ::: "memory")
#endif

static inline void memset_io(volatile void __iomem *addr, unsigned char val, int count)
{
	memset((void __force *)addr, val, count);
}
static inline void memcpy_fromio(void *dst, const volatile void __iomem *src, int count)
{
	memcpy(dst, (void __force *)src, count);
}
static inline void memcpy_toio(volatile void __iomem *dst, const void *src, int count)
{
	memcpy((void __force *)dst, src, count);
}

/*
 * Read a 32-bit register that requires a 64-bit read cycle on the bus.
 * Avoid interrupt mucking, just adjust the address for 4-byte access.
 * Assume the addresses are 8-byte aligned.
 */
#ifdef __MIPSEB__
#define __CSR_32_ADJUST 4
#else
#define __CSR_32_ADJUST 0
#endif

#define csr_out32(v, a) (*(volatile u32 *)((unsigned long)(a) + __CSR_32_ADJUST) = (v))
#define csr_in32(a)    (*(volatile u32 *)((unsigned long)(a) + __CSR_32_ADJUST))

/*
 * U-Boot specific
 */
#define sync()		mmiowb()

#define MAP_NOCACHE	1

static inline void *
map_physmem(phys_addr_t paddr, unsigned long len, unsigned long flags)
{
	if (flags == MAP_NOCACHE)
		return ioremap(paddr, len);

	return (void *)CKSEG0ADDR(paddr);
}
#define map_physmem map_physmem

#define __BUILD_CLRBITS(bwlq, sfx, end, type)				\
									\
static inline void clrbits_##sfx(volatile void __iomem *mem, type clr)	\
{									\
	type __val = __raw_read##bwlq(mem);				\
	__val = end##_to_cpu(__val);					\
	__val &= ~clr;							\
	__val = cpu_to_##end(__val);					\
	__raw_write##bwlq(__val, mem);					\
}

#define __BUILD_SETBITS(bwlq, sfx, end, type)				\
									\
static inline void setbits_##sfx(volatile void __iomem *mem, type set)	\
{									\
	type __val = __raw_read##bwlq(mem);				\
	__val = end##_to_cpu(__val);					\
	__val |= set;							\
	__val = cpu_to_##end(__val);					\
	__raw_write##bwlq(__val, mem);					\
}

#define __BUILD_CLRSETBITS(bwlq, sfx, end, type)			\
									\
static inline void clrsetbits_##sfx(volatile void __iomem *mem,		\
					type clr, type set)		\
{									\
	type __val = __raw_read##bwlq(mem);				\
	__val = end##_to_cpu(__val);					\
	__val &= ~clr;							\
	__val |= set;							\
	__val = cpu_to_##end(__val);					\
	__raw_write##bwlq(__val, mem);					\
}

#define BUILD_CLRSETBITS(bwlq, sfx, end, type)				\
									\
__BUILD_CLRBITS(bwlq, sfx, end, type)					\
__BUILD_SETBITS(bwlq, sfx, end, type)					\
__BUILD_CLRSETBITS(bwlq, sfx, end, type)

#define __to_cpu(v)		(v)
#define cpu_to__(v)		(v)

#define out_arch(type, endian, a, v)	__raw_write##type(cpu_to_##endian(v),a)
#define in_arch(type, endian, a)	endian##_to_cpu(__raw_read##type(a))

#define out_le64(a, v)	out_arch(q, le64, a, v)
#define out_le32(a, v)	out_arch(l, le32, a, v)
#define out_le16(a, v)	out_arch(w, le16, a, v)

#define in_le64(a)	in_arch(q, le64, a)
#define in_le32(a)	in_arch(l, le32, a)
#define in_le16(a)	in_arch(w, le16, a)

#define out_be64(a, v)	out_arch(q, be64, a, v)
#define out_be32(a, v)	out_arch(l, be32, a, v)
#define out_be16(a, v)	out_arch(w, be16, a, v)

#define in_be64(a)	in_arch(q, be64, a)
#define in_be32(a)	in_arch(l, be32, a)
#define in_be16(a)	in_arch(w, be16, a)

#define out_8(a, v)	__raw_writeb(v, a)
#define in_8(a)		__raw_readb(a)

BUILD_CLRSETBITS(b, 8, _, u8)
BUILD_CLRSETBITS(w, le16, le16, u16)
BUILD_CLRSETBITS(w, be16, be16, u16)
BUILD_CLRSETBITS(w, 16, _, u16)
BUILD_CLRSETBITS(l, le32, le32, u32)
BUILD_CLRSETBITS(l, be32, be32, u32)
BUILD_CLRSETBITS(l, 32, _, u32)
BUILD_CLRSETBITS(q, le64, le64, u64)
BUILD_CLRSETBITS(q, be64, be64, u64)
BUILD_CLRSETBITS(q, 64, _, u64)

#include <asm-generic/io.h>

#endif /* _ASM_IO_H */
