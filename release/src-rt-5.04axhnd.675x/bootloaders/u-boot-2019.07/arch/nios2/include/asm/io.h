/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 */

#ifndef __ASM_NIOS2_IO_H_
#define __ASM_NIOS2_IO_H_

static inline void sync(void)
{
	__asm__ __volatile__ ("sync" : : : "memory");
}

/*
 * Given a physical address and a length, return a virtual address
 * that can be used to access the memory range with the caching
 * properties specified by "flags".
 */
#define MAP_NOCACHE	1

static inline void *
map_physmem(phys_addr_t paddr, unsigned long len, unsigned long flags)
{
	DECLARE_GLOBAL_DATA_PTR;
	if (flags)
		return (void *)(paddr | gd->arch.io_region_base);
	else
		return (void *)(paddr | gd->arch.mem_region_base);
}
#define map_physmem map_physmem

static inline void *phys_to_virt(phys_addr_t paddr)
{
	DECLARE_GLOBAL_DATA_PTR;

	return (void *)(paddr | gd->arch.mem_region_base);
}
#define phys_to_virt phys_to_virt

static inline phys_addr_t virt_to_phys(void * vaddr)
{
	DECLARE_GLOBAL_DATA_PTR;
	return (phys_addr_t)vaddr & gd->arch.physaddr_mask;
}
#define virt_to_phys virt_to_phys

#define __raw_writeb(v,a)       (*(volatile unsigned char  *)(a) = (v))
#define __raw_writew(v,a)       (*(volatile unsigned short *)(a) = (v))
#define __raw_writel(v,a)       (*(volatile unsigned int   *)(a) = (v))

#define __raw_readb(a)          (*(volatile unsigned char  *)(a))
#define __raw_readw(a)          (*(volatile unsigned short *)(a))
#define __raw_readl(a)          (*(volatile unsigned int   *)(a))

#define readb(addr)\
	({unsigned char val;\
	 asm volatile( "ldbio %0, 0(%1)" :"=r"(val) : "r" (addr)); val;})
#define readw(addr)\
	({unsigned short val;\
	 asm volatile( "ldhio %0, 0(%1)" :"=r"(val) : "r" (addr)); val;})
#define readl(addr)\
	({unsigned long val;\
	 asm volatile( "ldwio %0, 0(%1)" :"=r"(val) : "r" (addr)); val;})

#define writeb(val,addr)\
	asm volatile ("stbio %0, 0(%1)" : : "r" (val), "r" (addr))
#define writew(val,addr)\
	asm volatile ("sthio %0, 0(%1)" : : "r" (val), "r" (addr))
#define writel(val,addr)\
	asm volatile ("stwio %0, 0(%1)" : : "r" (val), "r" (addr))

#define inb(addr)	readb(addr)
#define inw(addr)	readw(addr)
#define inl(addr)	readl(addr)
#define outb(val, addr)	writeb(val,addr)
#define outw(val, addr)	writew(val,addr)
#define outl(val, addr)	writel(val,addr)

static inline void insb (unsigned long port, void *dst, unsigned long count)
{
	unsigned char *p = dst;
	while (count--) *p++ = inb (port);
}
static inline void insw (unsigned long port, void *dst, unsigned long count)
{
	unsigned short *p = dst;
	while (count--) *p++ = inw (port);
}
static inline void insl (unsigned long port, void *dst, unsigned long count)
{
	unsigned long *p = dst;
	while (count--) *p++ = inl (port);
}

static inline void outsb (unsigned long port, const void *src, unsigned long count)
{
	const unsigned char *p = src;
	while (count--) outb (*p++, port);
}

static inline void outsw (unsigned long port, const void *src, unsigned long count)
{
	const unsigned short *p = src;
	while (count--) outw (*p++, port);
}
static inline void outsl (unsigned long port, const void *src, unsigned long count)
{
	const unsigned long *p = src;
	while (count--) outl (*p++, port);
}

/*
 * Clear and set bits in one shot. These macros can be used to clear and
 * set multiple bits in a register using a single call. These macros can
 * also be used to set a multiple-bit bit pattern using a mask, by
 * specifying the mask in the 'clear' parameter and the new bit pattern
 * in the 'set' parameter.
 */

#define out_arch(type,endian,a,v)	__raw_write##type(cpu_to_##endian(v),a)
#define in_arch(type,endian,a)		endian##_to_cpu(__raw_read##type(a))

#define out_le32(a,v)	out_arch(l,le32,a,v)
#define out_le16(a,v)	out_arch(w,le16,a,v)

#define in_le32(a)	in_arch(l,le32,a)
#define in_le16(a)	in_arch(w,le16,a)

#define out_be32(a,v)	out_arch(l,be32,a,v)
#define out_be16(a,v)	out_arch(w,be16,a,v)

#define in_be32(a)	in_arch(l,be32,a)
#define in_be16(a)	in_arch(w,be16,a)

#define out_8(a,v)	__raw_writeb(v,a)
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

#define memset_io(a, b, c)		memset((void *)(a), (b), (c))
#define memcpy_fromio(a, b, c)		memcpy((a), (void *)(b), (c))
#define memcpy_toio(a, b, c)		memcpy((void *)(a), (b), (c))

#include <asm-generic/io.h>

#endif /* __ASM_NIOS2_IO_H_ */
