/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * IO header file
 *
 * Copyright (C) 2001-2007 Tensilica Inc.
 * Based on the Linux/Xtensa version of this header.
 */

#ifndef _XTENSA_IO_H
#define _XTENSA_IO_H

#include <linux/types.h>
#include <asm/byteorder.h>

/*
 * swap functions to change byte order from little-endian to big-endian and
 * vice versa.
 */

static inline unsigned short _swapw(unsigned short v)
{
	return (v << 8) | (v >> 8);
}

static inline unsigned int _swapl(unsigned int v)
{
	return (v << 24) | ((v & 0xff00) << 8) |
		((v >> 8) & 0xff00) | (v >> 24);
}

/*
 * Generic I/O
 */

#define readb(addr) \
	({ unsigned char __v = (*(volatile unsigned char *)(addr)); __v; })
#define readw(addr) \
	({ unsigned short __v = (*(volatile unsigned short *)(addr)); __v; })
#define readl(addr) \
	({ unsigned int __v = (*(volatile unsigned int *)(addr)); __v; })
#define writeb(b, addr) (void)((*(volatile unsigned char *)(addr)) = (b))
#define writew(b, addr) (void)((*(volatile unsigned short *)(addr)) = (b))
#define writel(b, addr) (void)((*(volatile unsigned int *)(addr)) = (b))

#define __raw_readb readb
#define __raw_readw readw
#define __raw_readl readl
#define __raw_writeb writeb
#define __raw_writew writew
#define __raw_writel writel

/* These are the definitions for the x86 IO instructions
 * inb/inw/inl/outb/outw/outl, the "string" versions
 * insb/insw/insl/outsb/outsw/outsl, and the "pausing" versions
 * inb_p/inw_p/...
 * The macros don't do byte-swapping.
 */

#define inb(port)		readb((u8 *)((port)))
#define outb(val, port)		writeb((val), (u8 *)((unsigned long)(port)))
#define inw(port)		readw((u16 *)((port)))
#define outw(val, port)		writew((val), (u16 *)((unsigned long)(port)))
#define inl(port)		readl((u32 *)((port)))
#define outl(val, port)		writel((val), (u32 *)((unsigned long)(port)))

#define inb_p(port)		inb((port))
#define outb_p(val, port)	outb((val), (port))
#define inw_p(port)		inw((port))
#define outw_p(val, port)	outw((val), (port))
#define inl_p(port)		inl((port))
#define outl_p(val, port)	outl((val), (port))

void insb(unsigned long port, void *dst, unsigned long count);
void insw(unsigned long port, void *dst, unsigned long count);
void insl(unsigned long port, void *dst, unsigned long count);
void outsb(unsigned long port, const void *src, unsigned long count);
void outsw(unsigned long port, const void *src, unsigned long count);
void outsl(unsigned long port, const void *src, unsigned long count);

#define IO_SPACE_LIMIT ~0

#define memset_io(a, b, c)	memset((void *)(a), (b), (c))
#define memcpy_fromio(a, b, c)	memcpy((a), (void *)(b), (c))
#define memcpy_toio(a, b, c)	memcpy((void *)(a), (b), (c))

/* At this point the Xtensa doesn't provide byte swap instructions */

#ifdef __XTENSA_EB__
# define in_8(addr) (*(u8 *)(addr))
# define in_le16(addr) _swapw(*(u16 *)(addr))
# define in_le32(addr) _swapl(*(u32 *)(addr))
# define out_8(b, addr) *(u8 *)(addr) = (b)
# define out_le16(b, addr) *(u16 *)(addr) = _swapw(b)
# define out_le32(b, addr) *(u32 *)(addr) = _swapl(b)
#elif defined(__XTENSA_EL__)
# define in_8(addr)  (*(u8 *)(addr))
# define in_le16(addr) (*(u16 *)(addr))
# define in_le32(addr) (*(u32 *)(addr))
# define out_8(b, addr) *(u8 *)(addr) = (b)
# define out_le16(b, addr) *(u16 *)(addr) = (b)
# define out_le32(b, addr) *(u32 *)(addr) = (b)
#else
# error processor byte order undefined!
#endif


/*
 * Convert a physical pointer to a virtual kernel pointer for /dev/mem access
 */
#define xlate_dev_mem_ptr(p)    __va(p)

/*
 * Convert a virtual cached pointer to an uncached pointer
 */
#define xlate_dev_kmem_ptr(p)   p

/*
 * Dummy function to keep U-Boot's cfi_flash.c driver happy.
 */
static inline void sync(void)
{
}

#include <asm-generic/io.h>

#endif	/* _XTENSA_IO_H */
