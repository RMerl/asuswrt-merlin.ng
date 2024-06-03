/*
 *  linux/include/asm-arm/io.h
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
#ifndef __ASM_ARM_IO_H
#define __ASM_ARM_IO_H

#ifdef __KERNEL__

#include <linux/types.h>
#include <asm/byteorder.h>
#include <asm/memory.h>
#include <asm/barriers.h>
#if 0	/* XXX###XXX */
#include <asm/arch/hardware.h>
#endif	/* XXX###XXX */

static inline void sync(void)
{
}

/*
 * Generic virtual read/write.  Note that we don't support half-word
 * read/writes.  We define __arch_*[bl] here, and leave __arch_*w
 * to the architecture specific code.
 */
#define __arch_getb(a)			(*(volatile unsigned char *)(a))
#define __arch_getw(a)			(*(volatile unsigned short *)(a))
#define __arch_getl(a)			(*(volatile unsigned int *)(a))
#define __arch_getq(a)			(*(volatile unsigned long long *)(a))

#define __arch_putb(v,a)		(*(volatile unsigned char *)(a) = (v))
#define __arch_putw(v,a)		(*(volatile unsigned short *)(a) = (v))
#define __arch_putl(v,a)		(*(volatile unsigned int *)(a) = (v))
#define __arch_putq(v,a)		(*(volatile unsigned long long *)(a) = (v))

static inline void __raw_writesb(unsigned long addr, const void *data,
				 int bytelen)
{
	uint8_t *buf = (uint8_t *)data;
	while(bytelen--)
		__arch_putb(*buf++, addr);
}

static inline void __raw_writesw(unsigned long addr, const void *data,
				 int wordlen)
{
	uint16_t *buf = (uint16_t *)data;
	while(wordlen--)
		__arch_putw(*buf++, addr);
}

static inline void __raw_writesl(unsigned long addr, const void *data,
				 int longlen)
{
	uint32_t *buf = (uint32_t *)data;
	while(longlen--)
		__arch_putl(*buf++, addr);
}

static inline void __raw_readsb(unsigned long addr, void *data, int bytelen)
{
	uint8_t *buf = (uint8_t *)data;
	while(bytelen--)
		*buf++ = __arch_getb(addr);
}

static inline void __raw_readsw(unsigned long addr, void *data, int wordlen)
{
	uint16_t *buf = (uint16_t *)data;
	while(wordlen--)
		*buf++ = __arch_getw(addr);
}

static inline void __raw_readsl(unsigned long addr, void *data, int longlen)
{
	uint32_t *buf = (uint32_t *)data;
	while(longlen--)
		*buf++ = __arch_getl(addr);
}

#define __raw_writeb(v,a)	__arch_putb(v,a)
#define __raw_writew(v,a)	__arch_putw(v,a)
#define __raw_writel(v,a)	__arch_putl(v,a)
#define __raw_writeq(v,a)	__arch_putq(v,a)

#define __raw_readb(a)		__arch_getb(a)
#define __raw_readw(a)		__arch_getw(a)
#define __raw_readl(a)		__arch_getl(a)
#define __raw_readq(a)		__arch_getq(a)

/*
 * TODO: The kernel offers some more advanced versions of barriers, it might
 * have some advantages to use them instead of the simple one here.
 */
#define mb()		dsb()
#define __iormb()	dmb()
#define __iowmb()	dmb()

#define writeb(v,c)	({ u8  __v = v; __iowmb(); __arch_putb(__v,c); __v; })
#define writew(v,c)	({ u16 __v = v; __iowmb(); __arch_putw(__v,c); __v; })
#define writel(v,c)	({ u32 __v = v; __iowmb(); __arch_putl(__v,c); __v; })
#define writeq(v,c)	({ u64 __v = v; __iowmb(); __arch_putq(__v,c); __v; })

#define readb(c)	({ u8  __v = __arch_getb(c); __iormb(); __v; })
#define readw(c)	({ u16 __v = __arch_getw(c); __iormb(); __v; })
#define readl(c)	({ u32 __v = __arch_getl(c); __iormb(); __v; })
#define readq(c)	({ u64 __v = __arch_getq(c); __iormb(); __v; })

/*
 * Relaxed I/O memory access primitives. These follow the Device memory
 * ordering rules but do not guarantee any ordering relative to Normal memory
 * accesses.
 */
#define readb_relaxed(c)	({ u8  __r = __raw_readb(c); __r; })
#define readw_relaxed(c)	({ u16 __r = le16_to_cpu((__force __le16) \
						__raw_readw(c)); __r; })
#define readl_relaxed(c)	({ u32 __r = le32_to_cpu((__force __le32) \
						__raw_readl(c)); __r; })
#define readq_relaxed(c)	({ u64 __r = le64_to_cpu((__force __le64) \
						__raw_readq(c)); __r; })

#define writeb_relaxed(v, c)	((void)__raw_writeb((v), (c)))
#define writew_relaxed(v, c)	((void)__raw_writew((__force u16) \
						    cpu_to_le16(v), (c)))
#define writel_relaxed(v, c)	((void)__raw_writel((__force u32) \
						    cpu_to_le32(v), (c)))
#define writeq_relaxed(v, c)	((void)__raw_writeq((__force u64) \
						    cpu_to_le64(v), (c)))

/*
 * The compiler seems to be incapable of optimising constants
 * properly.  Spell it out to the compiler in some cases.
 * These are only valid for small values of "off" (< 1<<12)
 */
#define __raw_base_writeb(val,base,off)	__arch_base_putb(val,base,off)
#define __raw_base_writew(val,base,off)	__arch_base_putw(val,base,off)
#define __raw_base_writel(val,base,off)	__arch_base_putl(val,base,off)

#define __raw_base_readb(base,off)	__arch_base_getb(base,off)
#define __raw_base_readw(base,off)	__arch_base_getw(base,off)
#define __raw_base_readl(base,off)	__arch_base_getl(base,off)

/*
 * Clear and set bits in one shot. These macros can be used to clear and
 * set multiple bits in a register using a single call. These macros can
 * also be used to set a multiple-bit bit pattern using a mask, by
 * specifying the mask in the 'clear' parameter and the new bit pattern
 * in the 'set' parameter.
 */

#define out_arch(type,endian,a,v)	__raw_write##type(cpu_to_##endian(v),a)
#define in_arch(type,endian,a)		endian##_to_cpu(__raw_read##type(a))

#define out_le64(a,v)	out_arch(q,le64,a,v)
#define out_le32(a,v)	out_arch(l,le32,a,v)
#define out_le16(a,v)	out_arch(w,le16,a,v)

#define in_le64(a)	in_arch(q,le64,a)
#define in_le32(a)	in_arch(l,le32,a)
#define in_le16(a)	in_arch(w,le16,a)

#define out_be32(a,v)	out_arch(l,be32,a,v)
#define out_be16(a,v)	out_arch(w,be16,a,v)

#define in_be32(a)	in_arch(l,be32,a)
#define in_be16(a)	in_arch(w,be16,a)

#define out_32(a,v)	__raw_writel(v,a)
#define out_16(a,v)	__raw_writew(v,a)
#define out_8(a,v)	__raw_writeb(v,a)

#define in_32(a)	__raw_readl(a)
#define in_16(a)	__raw_readw(a)
#define in_8(a)		__raw_readb(a)

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

#define clrbits_32(addr, clear) clrbits(32, addr, clear)
#define setbits_32(addr, set) setbits(32, addr, set)
#define clrsetbits_32(addr, clear, set) clrsetbits(32, addr, clear, set)

#define clrbits_be16(addr, clear) clrbits(be16, addr, clear)
#define setbits_be16(addr, set) setbits(be16, addr, set)
#define clrsetbits_be16(addr, clear, set) clrsetbits(be16, addr, clear, set)

#define clrbits_le16(addr, clear) clrbits(le16, addr, clear)
#define setbits_le16(addr, set) setbits(le16, addr, set)
#define clrsetbits_le16(addr, clear, set) clrsetbits(le16, addr, clear, set)

#define clrbits_16(addr, clear) clrbits(16, addr, clear)
#define setbits_16(addr, set) setbits(16, addr, set)
#define clrsetbits_16(addr, clear, set) clrsetbits(16, addr, clear, set)

#define clrbits_8(addr, clear) clrbits(8, addr, clear)
#define setbits_8(addr, set) setbits(8, addr, set)
#define clrsetbits_8(addr, clear, set) clrsetbits(8, addr, clear, set)

/*
 * Now, pick up the machine-defined IO definitions
 */
#if 0	/* XXX###XXX */
#include <asm/arch/io.h>
#endif	/* XXX###XXX */

/*
 *  IO port access primitives
 *  -------------------------
 *
 * The ARM doesn't have special IO access instructions; all IO is memory
 * mapped.  Note that these are defined to perform little endian accesses
 * only.  Their primary purpose is to access PCI and ISA peripherals.
 *
 * Note that for a big endian machine, this implies that the following
 * big endian mode connectivity is in place, as described by numerous
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
#define outb(v,p)			__raw_writeb(v,__io(p))
#define outw(v,p)			__raw_writew(cpu_to_le16(v),__io(p))
#define outl(v,p)			__raw_writel(cpu_to_le32(v),__io(p))

#define inb(p)	({ unsigned int __v = __raw_readb(__io(p)); __v; })
#define inw(p)	({ unsigned int __v = le16_to_cpu(__raw_readw(__io(p))); __v; })
#define inl(p)	({ unsigned int __v = le32_to_cpu(__raw_readl(__io(p))); __v; })

#define outsb(p,d,l)			__raw_writesb(__io(p),d,l)
#define outsw(p,d,l)			__raw_writesw(__io(p),d,l)
#define outsl(p,d,l)			__raw_writesl(__io(p),d,l)

#define insb(p,d,l)			__raw_readsb(__io(p),d,l)
#define insw(p,d,l)			__raw_readsw(__io(p),d,l)
#define insl(p,d,l)			__raw_readsl(__io(p),d,l)
#endif

#define outb_p(val,port)		outb((val),(port))
#define outw_p(val,port)		outw((val),(port))
#define outl_p(val,port)		outl((val),(port))
#define inb_p(port)			inb((port))
#define inw_p(port)			inw((port))
#define inl_p(port)			inl((port))

#define outsb_p(port,from,len)		outsb(port,from,len)
#define outsw_p(port,from,len)		outsw(port,from,len)
#define outsl_p(port,from,len)		outsl(port,from,len)
#define insb_p(port,to,len)		insb(port,to,len)
#define insw_p(port,to,len)		insw(port,to,len)
#define insl_p(port,to,len)		insl(port,to,len)

#define writesl(a, d, s)	__raw_writesl((unsigned long)a, d, s)
#define readsl(a, d, s)		__raw_readsl((unsigned long)a, d, s)
#define writesw(a, d, s)	__raw_writesw((unsigned long)a, d, s)
#define readsw(a, d, s)		__raw_readsw((unsigned long)a, d, s)
#define writesb(a, d, s)	__raw_writesb((unsigned long)a, d, s)
#define readsb(a, d, s)		__raw_readsb((unsigned long)a, d, s)

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

extern void __readwrite_bug(const char *fn);

/*
 * If this architecture has PCI memory IO, then define the read/write
 * macros.  These should only be used with the cookie passed from
 * ioremap.
 */
#ifdef __mem_pci

#define readb(c) ({ unsigned int __v = __raw_readb(__mem_pci(c)); __v; })
#define readw(c) ({ unsigned int __v = le16_to_cpu(__raw_readw(__mem_pci(c))); __v; })
#define readl(c) ({ unsigned int __v = le32_to_cpu(__raw_readl(__mem_pci(c))); __v; })

#define writeb(v,c)		__raw_writeb(v,__mem_pci(c))
#define writew(v,c)		__raw_writew(cpu_to_le16(v),__mem_pci(c))
#define writel(v,c)		__raw_writel(cpu_to_le32(v),__mem_pci(c))

#define memset_io(c,v,l)		_memset_io(__mem_pci(c),(v),(l))
#define memcpy_fromio(a,c,l)		_memcpy_fromio((a),__mem_pci(c),(l))
#define memcpy_toio(c,a,l)		_memcpy_toio(__mem_pci(c),(a),(l))

#define eth_io_copy_and_sum(s,c,l,b) \
				eth_copy_and_sum((s),__mem_pci(c),(l),(b))

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

#else
#define memset_io(a, b, c)		memset((void *)(a), (b), (c))
#define memcpy_fromio(a, b, c)		memcpy((a), (void *)(b), (c))
#define memcpy_toio(a, b, c)		memcpy((void *)(a), (b), (c))

#if !defined(readb)

#define readb(addr)			(__readwrite_bug("readb"),0)
#define readw(addr)			(__readwrite_bug("readw"),0)
#define readl(addr)			(__readwrite_bug("readl"),0)
#define writeb(v,addr)			__readwrite_bug("writeb")
#define writew(v,addr)			__readwrite_bug("writew")
#define writel(v,addr)			__readwrite_bug("writel")

#define eth_io_copy_and_sum(a,b,c,d)	__readwrite_bug("eth_io_copy_and_sum")

#define check_signature(io,sig,len)	(0)

#endif
#endif	/* __mem_pci */

/*
 * If this architecture has ISA IO, then define the isa_read/isa_write
 * macros.
 */
#ifdef __mem_isa

#define isa_readb(addr)			__raw_readb(__mem_isa(addr))
#define isa_readw(addr)			__raw_readw(__mem_isa(addr))
#define isa_readl(addr)			__raw_readl(__mem_isa(addr))
#define isa_writeb(val,addr)		__raw_writeb(val,__mem_isa(addr))
#define isa_writew(val,addr)		__raw_writew(val,__mem_isa(addr))
#define isa_writel(val,addr)		__raw_writel(val,__mem_isa(addr))
#define isa_memset_io(a,b,c)		_memset_io(__mem_isa(a),(b),(c))
#define isa_memcpy_fromio(a,b,c)	_memcpy_fromio((a),__mem_isa(b),(c))
#define isa_memcpy_toio(a,b,c)		_memcpy_toio(__mem_isa((a)),(b),(c))

#define isa_eth_io_copy_and_sum(a,b,c,d) \
				eth_copy_and_sum((a),__mem_isa(b),(c),(d))

static inline int
isa_check_signature(unsigned long io_addr, const unsigned char *signature,
		    int length)
{
	int retval = 0;
	do {
		if (isa_readb(io_addr) != *signature)
			goto out;
		io_addr++;
		signature++;
		length--;
	} while (length);
	retval = 1;
out:
	return retval;
}

#else	/* __mem_isa */

#define isa_readb(addr)			(__readwrite_bug("isa_readb"),0)
#define isa_readw(addr)			(__readwrite_bug("isa_readw"),0)
#define isa_readl(addr)			(__readwrite_bug("isa_readl"),0)
#define isa_writeb(val,addr)		__readwrite_bug("isa_writeb")
#define isa_writew(val,addr)		__readwrite_bug("isa_writew")
#define isa_writel(val,addr)		__readwrite_bug("isa_writel")
#define isa_memset_io(a,b,c)		__readwrite_bug("isa_memset_io")
#define isa_memcpy_fromio(a,b,c)	__readwrite_bug("isa_memcpy_fromio")
#define isa_memcpy_toio(a,b,c)		__readwrite_bug("isa_memcpy_toio")

#define isa_eth_io_copy_and_sum(a,b,c,d) \
				__readwrite_bug("isa_eth_io_copy_and_sum")

#define isa_check_signature(io,sig,len)	(0)

#endif	/* __mem_isa */
#endif	/* __KERNEL__ */

#include <asm-generic/io.h>
#include <iotrace.h>

#endif	/* __ASM_ARM_IO_H */
