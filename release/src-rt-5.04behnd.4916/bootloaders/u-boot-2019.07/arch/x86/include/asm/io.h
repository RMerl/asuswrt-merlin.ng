/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef _ASM_IO_H
#define _ASM_IO_H

#include <linux/compiler.h>

/*
 * This file contains the definitions for the x86 IO instructions
 * inb/inw/inl/outb/outw/outl and the "string versions" of the same
 * (insb/insw/insl/outsb/outsw/outsl). You can also use "pausing"
 * versions of the single-IO instructions (inb_p/inw_p/..).
 *
 * This file is not meant to be obfuscating: it's just complicated
 * to (a) handle it all in a way that makes gcc able to optimize it
 * as well as possible and (b) trying to avoid writing the same thing
 * over and over again with slight variations and possibly making a
 * mistake somewhere.
 */

/*
 * Thanks to James van Artsdalen for a better timing-fix than
 * the two short jumps: using outb's to a nonexistent port seems
 * to guarantee better timings even on fast machines.
 *
 * On the other hand, I'd like to be sure of a non-existent port:
 * I feel a bit unsafe about using 0x80 (should be safe, though)
 *
 *		Linus
 */

 /*
  *  Bit simplified and optimized by Jan Hubicka
  *  Support of BIGMEM added by Gerhard Wichert, Siemens AG, July 1999.
  *
  *  isa_memset_io, isa_memcpy_fromio, isa_memcpy_toio added,
  *  isa_read[wl] and isa_write[wl] fixed
  *  - Arnaldo Carvalho de Melo <acme@conectiva.com.br>
  */

#define IO_SPACE_LIMIT 0xffff

#include <asm/types.h>


#ifdef __KERNEL__


/*
 * readX/writeX() are used to access memory mapped devices. On some
 * architectures the memory mapped IO stuff needs to be accessed
 * differently. On the x86 architecture, we just read/write the
 * memory location directly.
 */

#define readb(addr) (*(volatile u8 *)(uintptr_t)(addr))
#define readw(addr) (*(volatile u16 *)(uintptr_t)(addr))
#define readl(addr) (*(volatile u32 *)(uintptr_t)(addr))
#define readq(addr) (*(volatile u64 *)(uintptr_t)(addr))
#define __raw_readb readb
#define __raw_readw readw
#define __raw_readl readl
#define __raw_readq readq

#define writeb(b, addr) (*(volatile u8 *)(addr) = (b))
#define writew(b, addr) (*(volatile u16 *)(addr) = (b))
#define writel(b, addr) (*(volatile u32 *)(addr) = (b))
#define writeq(b, addr) (*(volatile u64 *)(addr) = (b))
#define __raw_writeb writeb
#define __raw_writew writew
#define __raw_writel writel
#define __raw_writeq writeq

#define memset_io(a,b,c)	memset((a),(b),(c))
#define memcpy_fromio(a,b,c)	memcpy((a),(b),(c))
#define memcpy_toio(a,b,c)	memcpy((a),(b),(c))

#define out_arch(type, endian, a, v)	__raw_write##type(cpu_to_##endian(v), a)
#define in_arch(type, endian, a)	endian##_to_cpu(__raw_read##type(a))

#define out_le64(a, v)	out_arch(q, le64, a, v)
#define out_le32(a, v)	out_arch(l, le32, a, v)
#define out_le16(a, v)	out_arch(w, le16, a, v)

#define in_le64(a)	in_arch(q, le64, a)
#define in_le32(a)	in_arch(l, le32, a)
#define in_le16(a)	in_arch(w, le16, a)

#define out_be32(a, v)	out_arch(l, be32, a, v)
#define out_be16(a, v)	out_arch(w, be16, a, v)

#define in_be32(a)	in_arch(l, be32, a)
#define in_be16(a)	in_arch(w, be16, a)

#define out_8(a, v)	__raw_writeb(v, a)
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

#define clrbits_be16(addr, clear) clrbits(be16, addr, clear)
#define setbits_be16(addr, set) setbits(be16, addr, set)
#define clrsetbits_be16(addr, clear, set) clrsetbits(be16, addr, clear, set)

#define clrbits_le16(addr, clear) clrbits(le16, addr, clear)
#define setbits_le16(addr, set) setbits(le16, addr, set)
#define clrsetbits_le16(addr, clear, set) clrsetbits(le16, addr, clear, set)

#define clrbits_8(addr, clear) clrbits(8, addr, clear)
#define setbits_8(addr, set) setbits(8, addr, set)
#define clrsetbits_8(addr, clear, set) clrsetbits(8, addr, clear, set)

#endif /* __KERNEL__ */

#ifdef SLOW_IO_BY_JUMPING
#define __SLOW_DOWN_IO "\njmp 1f\n1:\tjmp 1f\n1:"
#else
#define __SLOW_DOWN_IO "\noutb %%al,$0xed"
#endif

#ifdef REALLY_SLOW_IO
#define __FULL_SLOW_DOWN_IO __SLOW_DOWN_IO __SLOW_DOWN_IO __SLOW_DOWN_IO __SLOW_DOWN_IO
#else
#define __FULL_SLOW_DOWN_IO __SLOW_DOWN_IO
#endif


/*
 * Talk about misusing macros..
 */
#define __OUT1(s,x) \
static inline void _out##s(unsigned x value, unsigned short port) {

#define __OUT2(s,s1,s2) \
__asm__ __volatile__ ("out" #s " %" s1 "0,%" s2 "1"


#define __OUT(s,s1,x) \
__OUT1(s,x) __OUT2(s,s1,"w") : : "a" (value), "Nd" (port)); } \
__OUT1(s##_p,x) __OUT2(s,s1,"w") __FULL_SLOW_DOWN_IO : : "a" (value), "Nd" (port));}

#define __IN1(s) \
static inline RETURN_TYPE _in##s(unsigned short port) { RETURN_TYPE _v;

#define __IN2(s,s1,s2) \
__asm__ __volatile__ ("in" #s " %" s2 "1,%" s1 "0"

#define __IN(s,s1,i...) \
__IN1(s) __IN2(s,s1,"w") : "=a" (_v) : "Nd" (port) ,##i ); return _v; } \
__IN1(s##_p) __IN2(s,s1,"w") __FULL_SLOW_DOWN_IO : "=a" (_v) : "Nd" (port) ,##i ); return _v; }

#define __INS(s) \
static inline void ins##s(unsigned short port, void * addr, unsigned long count) \
{ __asm__ __volatile__ ("rep ; ins" #s \
: "=D" (addr), "=c" (count) : "d" (port),"0" (addr),"1" (count)); }

#define __OUTS(s) \
static inline void outs##s(unsigned short port, const void * addr, unsigned long count) \
{ __asm__ __volatile__ ("rep ; outs" #s \
: "=S" (addr), "=c" (count) : "d" (port),"0" (addr),"1" (count)); }

#define RETURN_TYPE unsigned char
__IN(b,"")
#undef RETURN_TYPE
#define RETURN_TYPE unsigned short
__IN(w,"")
#undef RETURN_TYPE
#define RETURN_TYPE unsigned int
__IN(l,"")
#undef RETURN_TYPE

#define inb(port)	_inb((uintptr_t)(port))
#define inw(port)	_inw((uintptr_t)(port))
#define inl(port)	_inl((uintptr_t)(port))

__OUT(b,"b",char)
__OUT(w,"w",short)
__OUT(l,,int)

#define outb(val, port)	_outb(val, (uintptr_t)(port))
#define outw(val, port)	_outw(val, (uintptr_t)(port))
#define outl(val, port)	_outl(val, (uintptr_t)(port))

__INS(b)
__INS(w)
__INS(l)

__OUTS(b)
__OUTS(w)
__OUTS(l)

/* IO space accessors */
#define clrio(type, addr, clear) \
	out##type(in##type(addr) & ~(clear), (addr))

#define setio(type, addr, set) \
	out##type(in##type(addr) | (set), (addr))

#define clrsetio(type, addr, clear, set) \
	out##type((in##type(addr) & ~(clear)) | (set), (addr))

#define clrio_32(addr, clear) clrio(l, addr, clear)
#define clrio_16(addr, clear) clrio(w, addr, clear)
#define clrio_8(addr, clear) clrio(b, addr, clear)

#define setio_32(addr, set) setio(l, addr, set)
#define setio_16(addr, set) setio(w, addr, set)
#define setio_8(addr, set) setio(b, addr, set)

#define clrsetio_32(addr, clear, set) clrsetio(l, addr, clear, set)
#define clrsetio_16(addr, clear, set) clrsetio(w, addr, clear, set)
#define clrsetio_8(addr, clear, set) clrsetio(b, addr, clear, set)

static inline void sync(void)
{
}

/*
 * TODO: The kernel offers some more advanced versions of barriers, it might
 * have some advantages to use them instead of the simple one here.
 */
#define dmb()		__asm__ __volatile__ ("" : : : "memory")
#define __iormb()	dmb()
#define __iowmb()	dmb()

/*
 * Read/write from/to an (offsettable) iomem cookie. It might be a PIO
 * access or a MMIO access, these functions don't care. The info is
 * encoded in the hardware mapping set up by the mapping functions
 * (or the cookie itself, depending on implementation and hw).
 *
 * The generic routines don't assume any hardware mappings, and just
 * encode the PIO/MMIO as part of the cookie. They coldly assume that
 * the MMIO IO mappings are not in the low address range.
 *
 * Architectures for which this is not true can't use this generic
 * implementation and should do their own copy.
 */

/*
 * We assume that all the low physical PIO addresses (0-0xffff) always
 * PIO. That means we can do some sanity checks on the low bits, and
 * don't need to just take things for granted.
 */
#define PIO_RESERVED	0x10000UL

/*
 * Ugly macros are a way of life.
 */
#define IO_COND(addr, is_pio, is_mmio) do {			\
	unsigned long port = (unsigned long __force)addr;	\
	if (port >= PIO_RESERVED) {				\
		is_mmio;					\
	} else {						\
		is_pio;						\
	}							\
} while (0)

static inline u8 ioread8(const volatile void __iomem *addr)
{
	IO_COND(addr, return inb(port), return readb(addr));
	return 0xff;
}

static inline u16 ioread16(const volatile void __iomem *addr)
{
	IO_COND(addr, return inw(port), return readw(addr));
	return 0xffff;
}

static inline u32 ioread32(const volatile void __iomem *addr)
{
	IO_COND(addr, return inl(port), return readl(addr));
	return 0xffffffff;
}

static inline void iowrite8(u8 value, volatile void __iomem *addr)
{
	IO_COND(addr, outb(value, port), writeb(value, addr));
}

static inline void iowrite16(u16 value, volatile void __iomem *addr)
{
	IO_COND(addr, outw(value, port), writew(value, addr));
}

static inline void iowrite32(u32 value, volatile void __iomem *addr)
{
	IO_COND(addr, outl(value, port), writel(value, addr));
}

#include <asm-generic/io.h>

#endif /* _ASM_IO_H */
