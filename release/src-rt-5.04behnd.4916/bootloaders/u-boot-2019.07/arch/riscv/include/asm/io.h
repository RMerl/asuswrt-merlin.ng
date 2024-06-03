/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 *
 */
#ifndef __ASM_RISCV_IO_H
#define __ASM_RISCV_IO_H

#ifdef __KERNEL__

#include <linux/types.h>
#include <asm/barrier.h>
#include <asm/byteorder.h>

static inline void sync(void)
{
}

#ifdef CONFIG_ARCH_MAP_SYSMEM
static inline void *map_sysmem(phys_addr_t paddr, unsigned long len)
{
	if (paddr < PHYS_SDRAM_0_SIZE + PHYS_SDRAM_1_SIZE)
		paddr = paddr | 0x40000000;
	return (void *)(uintptr_t)paddr;
}

static inline void *unmap_sysmem(const void *vaddr)
{
	phys_addr_t paddr = (phys_addr_t)vaddr;

	paddr = paddr & ~0x40000000;
	return (void *)(uintptr_t)paddr;
}

static inline phys_addr_t map_to_sysmem(const void *ptr)
{
	return (phys_addr_t)(uintptr_t)ptr;
}
#endif

/*
 * Generic virtual read/write.  Note that we don't support half-word
 * read/writes.  We define __arch_*[bl] here, and leave __arch_*w
 * to the architecture specific code.
 */
#define __arch_getb(a)			(*(unsigned char *)(a))
#define __arch_getw(a)			(*(unsigned short *)(a))
#define __arch_getl(a)			(*(unsigned int *)(a))
#define __arch_getq(a)			(*(unsigned long long *)(a))

#define __arch_putb(v, a)		(*(unsigned char *)(a) = (v))
#define __arch_putw(v, a)		(*(unsigned short *)(a) = (v))
#define __arch_putl(v, a)		(*(unsigned int *)(a) = (v))
#define __arch_putq(v, a)		(*(unsigned long long *)(a) = (v))

#define __raw_writeb(v, a)		__arch_putb(v, a)
#define __raw_writew(v, a)		__arch_putw(v, a)
#define __raw_writel(v, a)		__arch_putl(v, a)
#define __raw_writeq(v, a)		__arch_putq(v, a)

#define __raw_readb(a)			__arch_getb(a)
#define __raw_readw(a)			__arch_getw(a)
#define __raw_readl(a)			__arch_getl(a)
#define __raw_readq(a)			__arch_getq(a)

#define dmb()		mb()
#define __iormb()	rmb()
#define __iowmb()	wmb()

static inline void writeb(u8 val, volatile void __iomem *addr)
{
	__iowmb();
	__arch_putb(val, addr);
}

static inline void writew(u16 val, volatile void __iomem *addr)
{
	__iowmb();
	__arch_putw(val, addr);
}

static inline void writel(u32 val, volatile void __iomem *addr)
{
	__iowmb();
	__arch_putl(val, addr);
}

static inline void writeq(u64 val, volatile void __iomem *addr)
{
	__iowmb();
	__arch_putq(val, addr);
}

static inline u8 readb(const volatile void __iomem *addr)
{
	u8	val;

	val = __arch_getb(addr);
	__iormb();
	return val;
}

static inline u16 readw(const volatile void __iomem *addr)
{
	u16	val;

	val = __arch_getw(addr);
	__iormb();
	return val;
}

static inline u32 readl(const volatile void __iomem *addr)
{
	u32	val;

	val = __arch_getl(addr);
	__iormb();
	return val;
}

static inline u64 readq(const volatile void __iomem *addr)
{
	u64	val;

	val = __arch_getq(addr);
	__iormb();
	return val;
}

/*
 * The compiler seems to be incapable of optimising constants
 * properly.  Spell it out to the compiler in some cases.
 * These are only valid for small values of "off" (< 1<<12)
 */
#define __raw_base_writeb(val, base, off)	__arch_base_putb(val, base, off)
#define __raw_base_writew(val, base, off)	__arch_base_putw(val, base, off)
#define __raw_base_writel(val, base, off)	__arch_base_putl(val, base, off)

#define __raw_base_readb(base, off)	__arch_base_getb(base, off)
#define __raw_base_readw(base, off)	__arch_base_getw(base, off)
#define __raw_base_readl(base, off)	__arch_base_getl(base, off)

#define out_arch(type, endian, a, v)	__raw_write##type(cpu_to_##endian(v), a)
#define in_arch(type, endian, a)	endian##_to_cpu(__raw_read##type(a))

#define out_le32(a, v)			out_arch(l, le32, a, v)
#define out_le16(a, v)			out_arch(w, le16, a, v)

#define in_le32(a)			in_arch(l, le32, a)
#define in_le16(a)			in_arch(w, le16, a)

#define out_be32(a, v)			out_arch(l, be32, a, v)
#define out_be16(a, v)			out_arch(w, be16, a, v)

#define in_be32(a)			in_arch(l, be32, a)
#define in_be16(a)			in_arch(w, be16, a)

#define out_8(a, v)			__raw_writeb(v, a)
#define in_8(a)				__raw_readb(a)

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

/*
 * Now, pick up the machine-defined IO definitions
 * #include <asm/arch/io.h>
 */

/*
 *  IO port access primitives
 *  -------------------------
 *
 * The NDS32 doesn't have special IO access instructions just like ARM;
 * all IO is memory mapped.
 * Note that these are defined to perform little endian accesses
 * only.  Their primary purpose is to access PCI and ISA peripherals.
 *
 * Note that for a big endian machine, this implies that the following
 * big endian mode connectivity is in place, as described by numerious
 * ARM documents:
 *
 *    PCI:  D0-D7   D8-D15 D16-D23 D24-D31
 *    ARM: D24-D31 D16-D23  D8-D15  D0-D7
 *
 * The machine specific io.h include defines __io to translate an "IO"
 * address to a memory address.
 *
 * Note that we prevent GCC re-ordering or caching values in expressions
 * by introducing sequence points into the in*() definitions.  Note that
 * __raw_* do not guarantee this behaviour.
 *
 * The {in,out}[bwl] macros are for emulating x86-style PCI/ISA IO space.
 */
#ifdef __io
#define outb(v, p)			__raw_writeb(v, __io(p))
#define outw(v, p)			__raw_writew(cpu_to_le16(v), __io(p))
#define outl(v, p)			__raw_writel(cpu_to_le32(v), __io(p))

#define inb(p)	({ unsigned int __v = __raw_readb(__io(p)); __v; })
#define inw(p)	({ unsigned int __v = le16_to_cpu(__raw_readw(__io(p))); __v; })
#define inl(p)	({ unsigned int __v = le32_to_cpu(__raw_readl(__io(p))); __v; })

#define outsb(p, d, l)			writesb(__io(p), d, l)
#define outsw(p, d, l)			writesw(__io(p), d, l)
#define outsl(p, d, l)			writesl(__io(p), d, l)

#define insb(p, d, l)			readsb(__io(p), d, l)
#define insw(p, d, l)			readsw(__io(p), d, l)
#define insl(p, d, l)			readsl(__io(p), d, l)

static inline void readsb(unsigned int *addr, void *data, int bytelen)
{
	unsigned char *ptr;
	unsigned char *ptr2;

	ptr = (unsigned char *)addr;
	ptr2 = (unsigned char *)data;

	while (bytelen) {
		*ptr2 = *ptr;
		ptr2++;
		bytelen--;
	}
}

static inline void readsw(unsigned int *addr, void *data, int wordlen)
{
	unsigned short *ptr;
	unsigned short *ptr2;

	ptr = (unsigned short *)addr;
	ptr2 = (unsigned short *)data;

	while (wordlen) {
		*ptr2 = *ptr;
		ptr2++;
		wordlen--;
	}
}

static inline void readsl(unsigned int *addr, void *data, int longlen)
{
	unsigned int *ptr;
	unsigned int *ptr2;

	ptr = (unsigned int *)addr;
	ptr2 = (unsigned int *)data;

	while (longlen) {
		*ptr2 = *ptr;
		ptr2++;
		longlen--;
	}
}

static inline void writesb(unsigned int *addr, const void *data, int bytelen)
{
	unsigned char *ptr;
	unsigned char *ptr2;

	ptr = (unsigned char *)addr;
	ptr2 = (unsigned char *)data;

	while (bytelen) {
		*ptr = *ptr2;
		ptr2++;
		bytelen--;
	}
}

static inline void writesw(unsigned int *addr, const void *data, int wordlen)
{
	unsigned short *ptr;
	unsigned short *ptr2;

	ptr = (unsigned short *)addr;
	ptr2 = (unsigned short *)data;

	while (wordlen) {
		*ptr = *ptr2;
		ptr2++;
		wordlen--;
	}
}

static inline void writesl(unsigned int *addr, const void *data, int longlen)
{
	unsigned int *ptr;
	unsigned int *ptr2;

	ptr = (unsigned int *)addr;
	ptr2 = (unsigned int *)data;

	while (longlen) {
		*ptr = *ptr2;
		ptr2++;
		longlen--;
	}
}
#endif

#define outb_p(val, port)		outb((val), (port))
#define outw_p(val, port)		outw((val), (port))
#define outl_p(val, port)		outl((val), (port))
#define inb_p(port)			inb((port))
#define inw_p(port)			inw((port))
#define inl_p(port)			inl((port))

#define outsb_p(port, from, len)	outsb(port, from, len)
#define outsw_p(port, from, len)	outsw(port, from, len)
#define outsl_p(port, from, len)	outsl(port, from, len)
#define insb_p(port, to, len)		insb(port, to, len)
#define insw_p(port, to, len)		insw(port, to, len)
#define insl_p(port, to, len)		insl(port, to, len)

/*
 * DMA-consistent mapping functions.  These allocate/free a region of
 * uncached, unwrite-buffered mapped memory space for use with DMA
 * devices.  This is the "generic" version.  The PCI specific version
 * is in pci.h
 */

/*
 * String version of IO memory access ops:
 */

/*
 * If this architecture has PCI memory IO, then define the read/write
 * macros.  These should only be used with the cookie passed from
 * ioremap.
 */
#ifdef __mem_pci

#define readb(c) ({ unsigned int __v = \
			__raw_readb(__mem_pci(c)); __v; })
#define readw(c) ({ unsigned int __v = \
			le16_to_cpu(__raw_readw(__mem_pci(c))); __v; })
#define readl(c) ({ unsigned int __v = \
			le32_to_cpu(__raw_readl(__mem_pci(c))); __v; })

#define writeb(v, c)		__raw_writeb(v, __mem_pci(c))
#define writew(v, c)		__raw_writew(cpu_to_le16(v), __mem_pci(c))
#define writel(v, c)		__raw_writel(cpu_to_le32(v), __mem_pci(c))

#define memset_io(c, v, l)	_memset_io(__mem_pci(c), (v), (l))
#define memcpy_fromio(a, c, l)	_memcpy_fromio((a), __mem_pci(c), (l))
#define memcpy_toio(c, a, l)	_memcpy_toio(__mem_pci(c), (a), (l))

#define eth_io_copy_and_sum(s, c, l, b) \
	eth_copy_and_sum((s), __mem_pci(c), (l), (b))

static inline int check_signature(ulong io_addr, const uchar *s, int len)
{
	int retval = 0;

	do {
		if (readb(io_addr) != *s)
			goto out;
		io_addr++;
		s++;
		len--;
	} while (len);
	retval = 1;
out:
	return retval;
}
#endif	/* __mem_pci */

/*
 * If this architecture has ISA IO, then define the isa_read/isa_write
 * macros.
 */
#ifdef __mem_isa

#define isa_readb(addr)			__raw_readb(__mem_isa(addr))
#define isa_readw(addr)			__raw_readw(__mem_isa(addr))
#define isa_readl(addr)			__raw_readl(__mem_isa(addr))
#define isa_writeb(val, addr)		__raw_writeb(val, __mem_isa(addr))
#define isa_writew(val, addr)		__raw_writew(val, __mem_isa(addr))
#define isa_writel(val, addr)		__raw_writel(val, __mem_isa(addr))
#define isa_memset_io(a, b, c)		_memset_io(__mem_isa(a), (b), (c))
#define isa_memcpy_fromio(a, b, c)	_memcpy_fromio((a), __mem_isa(b), (c))
#define isa_memcpy_toio(a, b, c)	_memcpy_toio(__mem_isa((a)), (b), (c))

#define isa_eth_io_copy_and_sum(a, b, c, d) \
	eth_copy_and_sum((a), __mem_isa(b), (c), (d))

static inline int
isa_check_signature(ulong io_addr, const uchar *s, int len)
{
	int retval = 0;

	do {
		if (isa_readb(io_addr) != *s)
			goto out;
		io_addr++;
		s++;
		len--;
	} while (len);
	retval = 1;
out:
	return retval;
}

#else	/* __mem_isa */

#define isa_readb(addr)			(__readwrite_bug("isa_readb"), 0)
#define isa_readw(addr)			(__readwrite_bug("isa_readw"), 0)
#define isa_readl(addr)			(__readwrite_bug("isa_readl"), 0)
#define isa_writeb(val, addr)		__readwrite_bug("isa_writeb")
#define isa_writew(val, addr)		__readwrite_bug("isa_writew")
#define isa_writel(val, addr)		__readwrite_bug("isa_writel")
#define isa_memset_io(a, b, c)		__readwrite_bug("isa_memset_io")
#define isa_memcpy_fromio(a, b, c)	__readwrite_bug("isa_memcpy_fromio")
#define isa_memcpy_toio(a, b, c)	__readwrite_bug("isa_memcpy_toio")

#define isa_eth_io_copy_and_sum(a, b, c, d) \
	__readwrite_bug("isa_eth_io_copy_and_sum")

#define isa_check_signature(io, sig, len)	(0)

#endif	/* __mem_isa */
#endif	/* __KERNEL__ */

#include <asm-generic/io.h>

#endif	/* __ASM_RISCV_IO_H */
