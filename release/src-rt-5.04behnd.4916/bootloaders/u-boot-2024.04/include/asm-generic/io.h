/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Generic I/O functions.
 *
 * Copyright (c) 2016 Imagination Technologies Ltd.
 */

#ifndef __ASM_GENERIC_IO_H__
#define __ASM_GENERIC_IO_H__

/*
 * This file should be included at the end of each architecture-specific
 * asm/io.h such that we may provide generic implementations without
 * conflicting with architecture-specific code.
 */

#ifndef __ASSEMBLY__

/**
 * phys_to_virt() - Return a virtual address mapped to a given physical address
 * @paddr: the physical address
 *
 * Returns a virtual address which the CPU can access that maps to the physical
 * address @paddr. This should only be used where it is known that no dynamic
 * mapping is required. In general, map_physmem should be used instead.
 *
 * Returns: a virtual address which maps to @paddr
 */
#ifndef phys_to_virt
static inline void *phys_to_virt(phys_addr_t paddr)
{
	return (void *)(unsigned long)paddr;
}
#endif

/**
 * virt_to_phys() - Return the physical address that a virtual address maps to
 * @vaddr: the virtual address
 *
 * Returns the physical address which the CPU-accessible virtual address @vaddr
 * maps to.
 *
 * Returns: the physical address which @vaddr maps to
 */
#ifndef virt_to_phys
static inline phys_addr_t virt_to_phys(void *vaddr)
{
	return (phys_addr_t)((unsigned long)vaddr);
}
#endif

/*
 * Flags for use with map_physmem() & unmap_physmem(). Architectures need not
 * support all of these, in which case they will be defined as zero here &
 * ignored. Callers that may run on multiple architectures should therefore
 * treat them as hints rather than requirements.
 */
#ifndef MAP_NOCACHE
# define MAP_NOCACHE	0	/* Produce an uncached mapping */
#endif
#ifndef MAP_WRCOMBINE
# define MAP_WRCOMBINE	0	/* Allow write-combining on the mapping */
#endif
#ifndef MAP_WRBACK
# define MAP_WRBACK	0	/* Map using write-back caching */
#endif
#ifndef MAP_WRTHROUGH
# define MAP_WRTHROUGH	0	/* Map using write-through caching */
#endif

/**
 * map_physmem() - Return a virtual address mapped to a given physical address
 * @paddr: the physical address
 * @len: the length of the required mapping
 * @flags: flags affecting the type of mapping
 *
 * Return a virtual address through which the CPU may access the memory at
 * physical address @paddr. The mapping will be valid for at least @len bytes,
 * and may be affected by flags passed to the @flags argument. This function
 * may create new mappings, so should generally be paired with a matching call
 * to unmap_physmem once the caller is finished with the memory in question.
 *
 * Returns: a virtual address suitably mapped to @paddr
 */
#ifndef map_physmem
static inline void *map_physmem(phys_addr_t paddr, unsigned long len,
				unsigned long flags)
{
	return phys_to_virt(paddr);
}
#endif

/**
 * unmap_physmem() - Remove mappings created by a prior call to map_physmem()
 * @vaddr: the virtual address which map_physmem() previously returned
 * @flags: flags matching those originally passed to map_physmem()
 *
 * Unmap memory which was previously mapped by a call to map_physmem(). If
 * map_physmem() dynamically created a mapping for the memory in question then
 * unmap_physmem() will remove that mapping.
 */
#ifndef unmap_physmem
static inline void unmap_physmem(void *vaddr, unsigned long flags)
{
}
#endif

/*
 * __raw_{read,write}{b,w,l,q}() access memory in native endianness.
 *
 * On some architectures memory mapped IO needs to be accessed differently.
 * On the simple architectures, we just read/write the memory location
 * directly.
 */

#ifndef __raw_readb
#define __raw_readb __raw_readb
static inline u8 __raw_readb(const volatile void __iomem *addr)
{
	return *(const volatile u8 __force *)addr;
}
#endif

#ifndef __raw_readw
#define __raw_readw __raw_readw
static inline u16 __raw_readw(const volatile void __iomem *addr)
{
	return *(const volatile u16 __force *)addr;
}
#endif

#ifndef __raw_readl
#define __raw_readl __raw_readl
static inline u32 __raw_readl(const volatile void __iomem *addr)
{
	return *(const volatile u32 __force *)addr;
}
#endif

#ifdef CONFIG_64BIT
#ifndef __raw_readq
#define __raw_readq __raw_readq
static inline u64 __raw_readq(const volatile void __iomem *addr)
{
	return *(const volatile u64 __force *)addr;
}
#endif
#endif /* CONFIG_64BIT */

#ifndef __raw_writeb
#define __raw_writeb __raw_writeb
static inline void __raw_writeb(u8 value, volatile void __iomem *addr)
{
	*(volatile u8 __force *)addr = value;
}
#endif

#ifndef __raw_writew
#define __raw_writew __raw_writew
static inline void __raw_writew(u16 value, volatile void __iomem *addr)
{
	*(volatile u16 __force *)addr = value;
}
#endif

#ifndef __raw_writel
#define __raw_writel __raw_writel
static inline void __raw_writel(u32 value, volatile void __iomem *addr)
{
	*(volatile u32 __force *)addr = value;
}
#endif

#ifdef CONFIG_64BIT
#ifndef __raw_writeq
#define __raw_writeq __raw_writeq
static inline void __raw_writeq(u64 value, volatile void __iomem *addr)
{
	*(volatile u64 __force *)addr = value;
}
#endif
#endif /* CONFIG_64BIT */

/*
 * {read,write}s{b,w,l,q}() repeatedly access the same memory address in
 * native endianness in 8-, 16-, 32- or 64-bit chunks (@count times).
 */
#ifndef readsb
#define readsb readsb
static inline void readsb(const volatile void __iomem *addr, void *buffer,
			  unsigned int count)
{
	if (count) {
		u8 *buf = buffer;

		do {
			u8 x = __raw_readb(addr);
			*buf++ = x;
		} while (--count);
	}
}
#endif

#ifndef readsw
#define readsw readsw
static inline void readsw(const volatile void __iomem *addr, void *buffer,
			  unsigned int count)
{
	if (count) {
		u16 *buf = buffer;

		do {
			u16 x = __raw_readw(addr);
			*buf++ = x;
		} while (--count);
	}
}
#endif

#ifndef readsl
#define readsl readsl
static inline void readsl(const volatile void __iomem *addr, void *buffer,
			  unsigned int count)
{
	if (count) {
		u32 *buf = buffer;

		do {
			u32 x = __raw_readl(addr);
			*buf++ = x;
		} while (--count);
	}
}
#endif

#ifdef CONFIG_64BIT
#ifndef readsq
#define readsq readsq
static inline void readsq(const volatile void __iomem *addr, void *buffer,
			  unsigned int count)
{
	if (count) {
		u64 *buf = buffer;

		do {
			u64 x = __raw_readq(addr);
			*buf++ = x;
		} while (--count);
	}
}
#endif
#endif /* CONFIG_64BIT */

#ifndef writesb
#define writesb writesb
static inline void writesb(volatile void __iomem *addr, const void *buffer,
			   unsigned int count)
{
	if (count) {
		const u8 *buf = buffer;

		do {
			__raw_writeb(*buf++, addr);
		} while (--count);
	}
}
#endif

#ifndef writesw
#define writesw writesw
static inline void writesw(volatile void __iomem *addr, const void *buffer,
			   unsigned int count)
{
	if (count) {
		const u16 *buf = buffer;

		do {
			__raw_writew(*buf++, addr);
		} while (--count);
	}
}
#endif

#ifndef writesl
#define writesl writesl
static inline void writesl(volatile void __iomem *addr, const void *buffer,
			   unsigned int count)
{
	if (count) {
		const u32 *buf = buffer;

		do {
			__raw_writel(*buf++, addr);
		} while (--count);
	}
}
#endif

#ifdef CONFIG_64BIT
#ifndef writesq
#define writesq writesq
static inline void writesq(volatile void __iomem *addr, const void *buffer,
			   unsigned int count)
{
	if (count) {
		const u64 *buf = buffer;

		do {
			__raw_writeq(*buf++, addr);
		} while (--count);
	}
}
#endif
#endif /* CONFIG_64BIT */

#ifndef PCI_IOBASE
#define PCI_IOBASE ((void __iomem *)0)
#endif

/*
 * {in,out}s{b,w,l}{,_p}() are variants of the above that repeatedly access a
 * single I/O port multiple times.
 */

#ifndef insb
#define insb insb
static inline void insb(unsigned long addr, void *buffer, unsigned int count)
{
	readsb(PCI_IOBASE + addr, buffer, count);
}
#endif

#ifndef insw
#define insw insw
static inline void insw(unsigned long addr, void *buffer, unsigned int count)
{
	readsw(PCI_IOBASE + addr, buffer, count);
}
#endif

#ifndef insl
#define insl insl
static inline void insl(unsigned long addr, void *buffer, unsigned int count)
{
	readsl(PCI_IOBASE + addr, buffer, count);
}
#endif

#ifndef outsb
#define outsb outsb
static inline void outsb(unsigned long addr, const void *buffer,
			 unsigned int count)
{
	writesb(PCI_IOBASE + addr, buffer, count);
}
#endif

#ifndef outsw
#define outsw outsw
static inline void outsw(unsigned long addr, const void *buffer,
			 unsigned int count)
{
	writesw(PCI_IOBASE + addr, buffer, count);
}
#endif

#ifndef outsl
#define outsl outsl
static inline void outsl(unsigned long addr, const void *buffer,
			 unsigned int count)
{
	writesl(PCI_IOBASE + addr, buffer, count);
}
#endif

#ifndef ioread8_rep
#define ioread8_rep ioread8_rep
static inline void ioread8_rep(const volatile void __iomem *addr, void *buffer,
			       unsigned int count)
{
	readsb(addr, buffer, count);
}
#endif

#ifndef ioread16_rep
#define ioread16_rep ioread16_rep
static inline void ioread16_rep(const volatile void __iomem *addr,
				void *buffer, unsigned int count)
{
	readsw(addr, buffer, count);
}
#endif

#ifndef ioread32_rep
#define ioread32_rep ioread32_rep
static inline void ioread32_rep(const volatile void __iomem *addr,
				void *buffer, unsigned int count)
{
	readsl(addr, buffer, count);
}
#endif

#ifdef CONFIG_64BIT
#ifndef ioread64_rep
#define ioread64_rep ioread64_rep
static inline void ioread64_rep(const volatile void __iomem *addr,
				void *buffer, unsigned int count)
{
	readsq(addr, buffer, count);
}
#endif
#endif /* CONFIG_64BIT */

#ifndef iowrite8_rep
#define iowrite8_rep iowrite8_rep
static inline void iowrite8_rep(volatile void __iomem *addr,
				const void *buffer,
				unsigned int count)
{
	writesb(addr, buffer, count);
}
#endif

#ifndef iowrite16_rep
#define iowrite16_rep iowrite16_rep
static inline void iowrite16_rep(volatile void __iomem *addr,
				 const void *buffer,
				 unsigned int count)
{
	writesw(addr, buffer, count);
}
#endif

#ifndef iowrite32_rep
#define iowrite32_rep iowrite32_rep
static inline void iowrite32_rep(volatile void __iomem *addr,
				 const void *buffer,
				 unsigned int count)
{
	writesl(addr, buffer, count);
}
#endif

#ifdef CONFIG_64BIT
#ifndef iowrite64_rep
#define iowrite64_rep iowrite64_rep
static inline void iowrite64_rep(volatile void __iomem *addr,
				 const void *buffer,
				 unsigned int count)
{
	writesq(addr, buffer, count);
}
#endif
#endif /* CONFIG_64BIT */

#endif /* !__ASSEMBLY__ */
#endif /* __ASM_GENERIC_IO_H__ */
