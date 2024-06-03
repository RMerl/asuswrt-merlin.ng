/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/asm-microblaze/io.h -- Misc I/O operations
 *
 *  Copyright (C) 2003     John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001,02  NEC Corporation
 *  Copyright (C) 2001,02  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
 */

#ifndef __MICROBLAZE_IO_H__
#define __MICROBLAZE_IO_H__

#include <asm/types.h>

#define IO_SPACE_LIMIT 0xFFFFFFFF

#define readb(addr) \
	({ unsigned char __v = (*(volatile unsigned char *)(addr)); __v; })

#define readw(addr) \
	({ unsigned short __v = (*(volatile unsigned short *)(addr)); __v; })

#define readl(addr) \
	({ unsigned int __v = (*(volatile unsigned int *)(addr)); __v; })

#define writeb(b, addr) \
	(void)((*(volatile unsigned char *)(addr)) = (b))

#define writew(b, addr) \
	(void)((*(volatile unsigned short *)(addr)) = (b))

#define writel(b, addr) \
	(void)((*(volatile unsigned int *)(addr)) = (b))

#define memset_io(a, b, c)        memset((void *)(a), (b), (c))
#define memcpy_fromio(a, b, c)    memcpy((a), (void *)(b), (c))
#define memcpy_toio(a, b, c)      memcpy((void *)(a), (b), (c))

#define inb(addr)	readb(addr)
#define inw(addr)	readw(addr)
#define inl(addr)	readl(addr)
#define outb(x, addr)	((void)writeb(x, addr))
#define outw(x, addr)	((void)writew(x, addr))
#define outl(x, addr)	((void)writel(x, addr))

/* Some #definitions to keep strange Xilinx code happy */
#define in_8(addr)	readb(addr)
#define in_be16(addr)	readw(addr)
#define in_be32(addr)	readl(addr)

#define out_8(addr, x)		outb(x, addr)
#define out_be16(addr, x)	outw(x, addr)
#define out_be32(addr, x)	outl(x, addr)

#define inb_p(port)		inb((port))
#define outb_p(val, port)	outb((val), (port))
#define inw_p(port)		inw((port))
#define outw_p(val, port)	outw((val), (port))
#define inl_p(port)		inl((port))
#define outl_p(val, port)	outl((val), (port))

/* Some defines to keep the MTD flash drivers happy */

#define __raw_readb readb
#define __raw_readw readw
#define __raw_readl readl
#define __raw_writeb writeb
#define __raw_writew writew
#define __raw_writel writel

static inline void io_insb(unsigned long port, void *dst, unsigned long count)
{
	unsigned char *p = dst;

	while (count--)
		*p++ = inb(port);
}

static inline void io_insw(unsigned long port, void *dst, unsigned long count)
{
	unsigned short *p = dst;

	while (count--)
		*p++ = inw(port);
}

static inline void io_insl(unsigned long port, void *dst, unsigned long count)
{
	unsigned long *p = dst;

	while (count--)
		*p++ = inl(port);
}

static inline void
io_outsb(unsigned long port, const void *src, unsigned long count)
{
	const unsigned char *p = src;

	while (count--)
		outb(*p++, port);
}

static inline void
io_outsw(unsigned long port, const void *src, unsigned long count)
{
	const unsigned short *p = src;

	while (count--)
		outw(*p++, port);
}

static inline void
io_outsl(unsigned long port, const void *src, unsigned long count)
{
	const unsigned long *p = src;

	while (count--)
		outl(*p++, port);
}

#define outsb(a, b, l) io_outsb(a, b, l)
#define outsw(a, b, l) io_outsw(a, b, l)
#define outsl(a, b, l) io_outsl(a, b, l)

#define insb(a, b, l) io_insb(a, b, l)
#define insw(a, b, l) io_insw(a, b, l)
#define insl(a, b, l) io_insl(a, b, l)

#define ioremap_nocache(physaddr, size)		(physaddr)
#define ioremap_writethrough(physaddr, size)	(physaddr)
#define ioremap_fullcache(physaddr, size)	(physaddr)

static inline void sync(void)
{
}

#include <asm-generic/io.h>

#endif /* __MICROBLAZE_IO_H__ */
