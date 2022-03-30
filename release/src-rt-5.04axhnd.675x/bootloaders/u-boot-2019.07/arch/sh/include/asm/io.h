/*
 *  linux/include/asm-sh/io.h
 *
 *  Copyright (C) 1996-2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Modifications:
 *  16-Sep-1996	RMK	Inlined the inx/outx functions & optimised for both
 *			constant addresses and variable addresses.
 *  04-Dec-1997	RMK	Moved a lot of this stuff to the new architecture
 *			specific IO header files.
 *  27-Mar-1999	PJB	Second parameter of memcpy_toio is const..
 *  04-Apr-1999	PJB	Added check_signature.
 *  12-Dec-1999	RMK	More cleanups
 *  18-Jun-2000 RMK	Removed virt_to_* and friends definitions
 */
#ifndef __ASM_SH_IO_H
#define __ASM_SH_IO_H

#ifdef __KERNEL__

#include <linux/types.h>
#include <asm/byteorder.h>

/*
 * Generic virtual read/write.  Note that we don't support half-word
 * read/writes.  We define __arch_*[bl] here, and leave __arch_*w
 * to the architecture specific code.
 */
#define __arch_getb(a)			(*(volatile unsigned char *)(a))
#define __arch_getw(a)			(*(volatile unsigned short *)(a))
#define __arch_getl(a)			(*(volatile unsigned int *)(a))

#define __arch_putb(v, a)		(*(volatile unsigned char *)(a) = (v))
#define __arch_putw(v, a)		(*(volatile unsigned short *)(a) = (v))
#define __arch_putl(v, a)		(*(volatile unsigned int *)(a) = (v))

extern void __raw_writesb(unsigned int addr, const void *data, int bytelen);
extern void __raw_writesw(unsigned int addr, const void *data, int wordlen);
extern void __raw_writesl(unsigned int addr, const void *data, int longlen);

extern void __raw_readsb(unsigned int addr, void *data, int bytelen);
extern void __raw_readsw(unsigned int addr, void *data, int wordlen);
extern void __raw_readsl(unsigned int addr, void *data, int longlen);

#define __raw_writeb(v, a)		__arch_putb(v, a)
#define __raw_writew(v, a)		__arch_putw(v, a)
#define __raw_writel(v, a)		__arch_putl(v, a)

#define __raw_readb(a)			__arch_getb(a)
#define __raw_readw(a)			__arch_getw(a)
#define __raw_readl(a)			__arch_getl(a)

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

/*
 *  IO port access primitives
 *  -------------------------
 *
 * The SH doesn't have special IO access instructions; all IO is memory
 * mapped.  Note that these are defined to perform little endian accesses
 * only.  Their primary purpose is to access PCI and ISA peripherals.
 *
 * Note that we prevent GCC re-ordering or caching values in expressions
 * by introducing sequence points into the in*() definitions.  Note that
 * __raw_* do not guarantee this behaviour.
 *
 * The {in,out}[bwl] macros are for emulating x86-style PCI/ISA IO space.
 */
#define outb(v, p)               __raw_writeb(v, p)
#define outw(v, p)               __raw_writew(cpu_to_le16(v), p)
#define outl(v, p)               __raw_writel(cpu_to_le32(v), p)

#define inb(p)  ({ unsigned int __v = __raw_readb(p); __v; })
#define inw(p)  ({ unsigned int __v = __le16_to_cpu(__raw_readw(p)); __v; })
#define inl(p)  ({ unsigned int __v = __le32_to_cpu(__raw_readl(p)); __v; })

#define outsb(p, d, l)			__raw_writesb(p, d, l)
#define outsw(p, d, l)			__raw_writesw(p, d, l)
#define outsl(p, d, l)			__raw_writesl(p, d, l)

#define insb(p, d, l)			__raw_readsb(p, d, l)
#define insw(p, d, l)			__raw_readsw(p, d, l)
#define insl(p, d, l)			__raw_readsl(p, d, l)

#define outb_p(val, port)		outb((val), (port))
#define outw_p(val, port)		outw((val), (port))
#define outl_p(val, port)		outl((val), (port))
#define inb_p(port)			inb((port))
#define inw_p(port)			inw((port))
#define inl_p(port)			inl((port))

#define outsb_p(port, from, len)		outsb(port, from, len)
#define outsw_p(port, from, len)		outsw(port, from, len)
#define outsl_p(port, from, len)		outsl(port, from, len)
#define insb_p(port, to, len)		insb(port, to, len)
#define insw_p(port, to, len)		insw(port, to, len)
#define insl_p(port, to, len)		insl(port, to, len)

/* for U-Boot PCI */
#define out_8(port, val)	outb(val, port)
#define out_le16(port, val)	outw(val, port)
#define out_le32(port, val)	outl(val, port)
#define in_8(port)			inb(port)
#define in_le16(port)		inw(port)
#define in_le32(port)		inl(port)

/*
 * DMA-consistent mapping functions.  These allocate/free a region of
 * uncached, unwrite-buffered mapped memory space for use with DMA
 * devices.  This is the "generic" version.  The PCI specific version
 * is in pci.h
 */
extern void *consistent_alloc(int gfp, size_t size, dma_addr_t *handle);
extern void consistent_free(void *vaddr, size_t size, dma_addr_t handle);
extern void consistent_sync(void *vaddr, size_t size, int rw);

/*
 * String version of IO memory access ops:
 */
extern void _memcpy_fromio(void *, unsigned long, size_t);
extern void _memcpy_toio(unsigned long, const void *, size_t);
extern void _memset_io(unsigned long, int, size_t);

/*
 * If this architecture has PCI memory IO, then define the read/write
 * macros.  These should only be used with the cookie passed from
 * ioremap.
 */
#ifdef __mem_pci

#define readb(c) ({ unsigned int __v = __raw_readb(__mem_pci(c)); __v; })
#define readw(c)\
	({ unsigned int __v = le16_to_cpu(__raw_readw(__mem_pci(c))); __v; })
#define readl(c)\
	({ unsigned int __v = le32_to_cpu(__raw_readl(__mem_pci(c))); __v; })

#define writeb(v, c)		__raw_writeb(v, __mem_pci(c))
#define writew(v, c)		__raw_writew(cpu_to_le16(v), __mem_pci(c))
#define writel(v, c)		__raw_writel(cpu_to_le32(v), __mem_pci(c))

#define memset_io(c, v, l)		_memset_io(__mem_pci(c), (v), (l))
#define memcpy_fromio(a, c, l)	_memcpy_fromio((a), __mem_pci(c), (l))
#define memcpy_toio(c, a, l)	_memcpy_toio(__mem_pci(c), (a), (l))

#define eth_io_copy_and_sum(s, c, l, b) \
				eth_copy_and_sum((s), __mem_pci(c), (l), (b))

static inline int
check_signature(unsigned long io_addr, const unsigned char *signature,
		int length)
{
	int retval = 0;
	do {
		if (readb(io_addr) != *signature)
			goto out;
		io_addr++;
		signature++;
		length--;
	} while (length);
	retval = 1;
out:
	return retval;
}

#elif !defined(readb)

#define readb(addr)	__raw_readb(addr)
#define readw(addr)	__raw_readw(addr)
#define readl(addr)	__raw_readl(addr)
#define writeb(v, addr)	__raw_writeb(v, addr)
#define writew(v, addr)	__raw_writew(v, addr)
#define writel(v, addr)	__raw_writel(v, addr)

#define check_signature(io, sig, len)	(0)

#endif	/* __mem_pci */

static inline void sync(void)
{
}

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

#endif	/* __KERNEL__ */
#endif	/* __ASM_SH_IO_H */
