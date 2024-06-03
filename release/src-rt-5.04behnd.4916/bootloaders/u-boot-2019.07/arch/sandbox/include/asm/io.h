/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __SANDBOX_ASM_IO_H
#define __SANDBOX_ASM_IO_H

void *phys_to_virt(phys_addr_t paddr);
#define phys_to_virt phys_to_virt

phys_addr_t virt_to_phys(void *vaddr);
#define virt_to_phys virt_to_phys

void *map_physmem(phys_addr_t paddr, unsigned long len, unsigned long flags);
#define map_physmem map_physmem

/*
 * Take down a mapping set up by map_physmem().
 */
void unmap_physmem(const void *vaddr, unsigned long flags);
#define unmap_physmem unmap_physmem

#include <asm-generic/io.h>

/* For sandbox, we want addresses to point into our RAM buffer */
static inline void *map_sysmem(phys_addr_t paddr, unsigned long len)
{
	return map_physmem(paddr, len, MAP_WRBACK);
}

/* Remove a previous mapping */
static inline void unmap_sysmem(const void *vaddr)
{
	unmap_physmem(vaddr, MAP_WRBACK);
}

/* Map from a pointer to our RAM buffer */
phys_addr_t map_to_sysmem(const void *ptr);

/* Define nops for sandbox I/O access */
#define readb(addr) ((void)addr, 0)
#define readw(addr) ((void)addr, 0)
#define readl(addr) ((void)addr, 0)
#ifdef CONFIG_SANDBOX64
#define readq(addr) ((void)addr, 0)
#endif
#define writeb(v, addr) ((void)addr)
#define writew(v, addr) ((void)addr)
#define writel(v, addr) ((void)addr)
#ifdef CONFIG_SANDBOX64
#define writeq(v, addr) ((void)addr)
#endif

/*
 * Clear and set bits in one shot. These macros can be used to clear and
 * set multiple bits in a register using a single call. These macros can
 * also be used to set a multiple-bit bit pattern using a mask, by
 * specifying the mask in the 'clear' parameter and the new bit pattern
 * in the 'set' parameter.
 */

#define out_arch(type,endian,a,v)	write##type(cpu_to_##endian(v),a)
#define in_arch(type,endian,a)		endian##_to_cpu(read##type(a))

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

#define out_8(a,v)	writeb(v,a)
#define in_8(a)		readb(a)

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

/* I/O access functions */
int inl(unsigned int addr);
int inw(unsigned int addr);
int inb(unsigned int addr);

void outl(unsigned int value, unsigned int addr);
void outw(unsigned int value, unsigned int addr);
void outb(unsigned int value, unsigned int addr);

#define out_arch(type,endian,a,v)	write##type(cpu_to_##endian(v),a)
#define in_arch(type,endian,a)		endian##_to_cpu(read##type(a))

#define out_le32(a,v)	out_arch(l,le32,a,v)
#define out_le16(a,v)	out_arch(w,le16,a,v)

#define in_le32(a)	in_arch(l,le32,a)
#define in_le16(a)	in_arch(w,le16,a)

#define out_be32(a,v)	out_arch(l,be32,a,v)
#define out_be16(a,v)	out_arch(w,be16,a,v)

#define in_be32(a)	in_arch(l,be32,a)
#define in_be16(a)	in_arch(w,be16,a)

#define out_8(a,v)	writeb(v,a)
#define in_8(a)		readb(a)

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

static inline void _insw(volatile u16 *port, void *buf, int ns)
{
}

static inline void _outsw(volatile u16 *port, const void *buf, int ns)
{
}

static inline void memset_io(volatile void *addr, unsigned char val, int count)
{
}

static inline void memcpy_fromio(void *dst, const volatile void *src, int count)
{
}

static inline void memcpy_toio(volatile void *dst, const void *src, int count)
{
}

#define insw(port, buf, ns)		_insw((u16 *)port, buf, ns)
#define outsw(port, buf, ns)		_outsw((u16 *)port, buf, ns)

#include <iotrace.h>
#include <asm/types.h>

#endif
