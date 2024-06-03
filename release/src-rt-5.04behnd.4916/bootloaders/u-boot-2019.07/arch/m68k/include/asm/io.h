/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * IO header file
 *
 * Copyright (C) 2004-2007, 2012 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 */

#ifndef __ASM_M68K_IO_H__
#define __ASM_M68K_IO_H__

#include <asm/byteorder.h>

#ifndef _IO_BASE
#define _IO_BASE 0
#endif

#define __raw_readb(addr) (*(volatile u8 *)(addr))
#define __raw_readw(addr) (*(volatile u16 *)(addr))
#define __raw_readl(addr) (*(volatile u32 *)(addr))

#define __raw_writeb(b,addr) ((*(volatile u8 *) (addr)) = (b))
#define __raw_writew(w,addr) ((*(volatile u16 *) (addr)) = (w))
#define __raw_writel(l,addr) ((*(volatile u32 *) (addr)) = (l))

#define readb(addr)		in_8((volatile u8 *)(addr))
#define writeb(b,addr)		out_8((volatile u8 *)(addr), (b))
#if !defined(__BIG_ENDIAN)
#define readw(addr)		(*(volatile u16 *) (addr))
#define readl(addr)		(*(volatile u32 *) (addr))
#define writew(b,addr)		((*(volatile u16 *) (addr)) = (b))
#define writel(b,addr)		((*(volatile u32 *) (addr)) = (b))
#else
#define readw(addr)		in_be16((volatile u16 *)(addr))
#define readl(addr)		in_be32((volatile u32 *)(addr))
#define writew(b,addr)		out_be16((volatile u16 *)(addr),(b))
#define writel(b,addr)		out_be32((volatile u32 *)(addr),(b))
#endif

/*
 * The insw/outsw/insl/outsl macros don't do byte-swapping.
 * They are only used in practice for transferring buffers which
 * are arrays of bytes, and byte-swapping is not appropriate in
 * that case.  - paulus
 */
#define insb(port, buf, ns)	_insb((u8 *)((port)+_IO_BASE), (buf), (ns))
#define outsb(port, buf, ns)	_outsb((u8 *)((port)+_IO_BASE), (buf), (ns))
#define insw(port, buf, ns)	_insw_ns((u16 *)((port)+_IO_BASE), (buf), (ns))
#define outsw(port, buf, ns)	_outsw_ns((u16 *)((port)+_IO_BASE), (buf), (ns))
#define insl(port, buf, nl)	_insl_ns((u32 *)((port)+_IO_BASE), (buf), (nl))
#define outsl(port, buf, nl)	_outsl_ns((u32 *)((port)+_IO_BASE), (buf), (nl))

#define inb(port)		in_8((u8 *)((port)+_IO_BASE))
#define outb(val, port)		out_8((u8 *)((port)+_IO_BASE), (val))
#if !defined(__BIG_ENDIAN)
#define inw(port)		in_be16((u16 *)((port)+_IO_BASE))
#define outw(val, port)		out_be16((u16 *)((port)+_IO_BASE), (val))
#define inl(port)		in_be32((u32 *)((port)+_IO_BASE))
#define outl(val, port)		out_be32((u32 *)((port)+_IO_BASE), (val))
#else
#define inw(port)		in_le16((u16 *)((port)+_IO_BASE))
#define outw(val, port)		out_le16((u16 *)((port)+_IO_BASE), (val))
#define inl(port)		in_le32((u32 *)((port)+_IO_BASE))
#define outl(val, port)		out_le32((u32 *)((port)+_IO_BASE), (val))
#endif

#define mb() __asm__ __volatile__ ("" : : : "memory")

static inline void _insb(volatile u8 * port, void *buf, int ns)
{
	u8 *data = (u8 *) buf;
	while (ns--)
		*data++ = *port;
}

static inline void _outsb(volatile u8 * port, const void *buf, int ns)
{
	u8 *data = (u8 *) buf;
	while (ns--)
		*port = *data++;
}

static inline void _insw(volatile u16 * port, void *buf, int ns)
{
	u16 *data = (u16 *) buf;
	while (ns--)
		*data++ = __sw16(*port);
}

static inline void _outsw(volatile u16 * port, const void *buf, int ns)
{
	u16 *data = (u16 *) buf;
	while (ns--) {
		*port = __sw16(*data);
		data++;
	}
}

static inline void _insl(volatile u32 * port, void *buf, int nl)
{
	u32 *data = (u32 *) buf;
	while (nl--)
		*data++ = __sw32(*port);
}

static inline void _outsl(volatile u32 * port, const void *buf, int nl)
{
	u32 *data = (u32 *) buf;
	while (nl--) {
		*port = __sw32(*data);
		data++;
	}
}

static inline void _insw_ns(volatile u16 * port, void *buf, int ns)
{
	u16 *data = (u16 *) buf;
	while (ns--)
		*data++ = *port;
}

static inline void _outsw_ns(volatile u16 * port, const void *buf, int ns)
{
	u16 *data = (u16 *) buf;
	while (ns--) {
		*port = *data++;
	}
}

static inline void _insl_ns(volatile u32 * port, void *buf, int nl)
{
	u32 *data = (u32 *) buf;
	while (nl--)
		*data++ = *port;
}

static inline void _outsl_ns(volatile u32 * port, const void *buf, int nl)
{
	u32 *data = (u32 *) buf;
	while (nl--) {
		*port = *data;
		data++;
	}
}

/*
 * The *_ns versions below don't do byte-swapping.
 * Neither do the standard versions now, these are just here
 * for older code.
 */
#define insw_ns(port, buf, ns)	_insw_ns((u16 *)((port)+_IO_BASE), (buf), (ns))
#define outsw_ns(port, buf, ns)	_outsw_ns((u16 *)((port)+_IO_BASE), (buf), (ns))
#define insl_ns(port, buf, nl)	_insl_ns((u32 *)((port)+_IO_BASE), (buf), (nl))
#define outsl_ns(port, buf, nl)	_outsl_ns((u32 *)((port)+_IO_BASE), (buf), (nl))

#define IO_SPACE_LIMIT ~0

/*
 * 8, 16 and 32 bit, big and little endian I/O operations, with barrier.
 */
static inline int in_8(volatile u8 * addr)
{
	return (int)*addr;
}

static inline void out_8(volatile u8 * addr, int val)
{
	*addr = (u8) val;
}

static inline int in_le16(volatile u16 * addr)
{
	return __sw16(*addr);
}

static inline int in_be16(volatile u16 * addr)
{
	return (*addr & 0xFFFF);
}

static inline void out_le16(volatile u16 * addr, int val)
{
	*addr = __sw16(val);
}

static inline void out_be16(volatile u16 * addr, int val)
{
	*addr = (u16) val;
}

static inline unsigned in_le32(volatile u32 * addr)
{
	return __sw32(*addr);
}

static inline unsigned in_be32(volatile u32 * addr)
{
	return (*addr);
}

static inline void out_le32(volatile unsigned *addr, int val)
{
	*addr = __sw32(val);
}

static inline void out_be32(volatile unsigned *addr, int val)
{
	*addr = val;
}

/* Clear and set bits in one shot. These macros can be used to clear and
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

static inline void sync(void)
{
	/* This sync function is for PowerPC or other architecture instruction
	 * ColdFire does not have this instruction. Dummy function, added for
	 * compatibility (CFI driver)
	 */
}

#include <asm-generic/io.h>

#endif				/* __ASM_M68K_IO_H__ */
