/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#ifndef __SANDBOX_ASM_IO_H
#define __SANDBOX_ASM_IO_H

#include <linux/types.h>

enum sandboxio_size_t {
	SB_SIZE_8,
	SB_SIZE_16,
	SB_SIZE_32,
	SB_SIZE_64,
};

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

/* Map from a pointer to our RAM buffer */
phys_addr_t map_to_sysmem(const void *ptr);

unsigned long sandbox_read(const void *addr, enum sandboxio_size_t size);
void sandbox_write(void *addr, unsigned int val, enum sandboxio_size_t size);

#define readb(addr) sandbox_read((const void *)addr, SB_SIZE_8)
#define readw(addr) sandbox_read((const void *)addr, SB_SIZE_16)
#define readl(addr) sandbox_read((const void *)addr, SB_SIZE_32)
#ifdef CONFIG_SANDBOX64
#define readq(addr) sandbox_read((const void *)addr, SB_SIZE_64)
#endif
#define writeb(v, addr) sandbox_write((void *)addr, v, SB_SIZE_8)
#define writew(v, addr) sandbox_write((void *)addr, v, SB_SIZE_16)
#define writel(v, addr) sandbox_write((void *)addr, v, SB_SIZE_32)
#ifdef CONFIG_SANDBOX64
#define writeq(v, addr) sandbox_write((void *)addr, v, SB_SIZE_64)
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
int _inl(unsigned int addr);
int _inw(unsigned int addr);
int _inb(unsigned int addr);

void _outl(unsigned int value, unsigned int addr);
void _outw(unsigned int value, unsigned int addr);
void _outb(unsigned int value, unsigned int addr);

#define inb(port)	_inb((uintptr_t)(port))
#define inw(port)	_inw((uintptr_t)(port))
#define inl(port)	_inl((uintptr_t)(port))

#define outb(val, port)	_outb(val, (uintptr_t)(port))
#define outw(val, port)	_outw(val, (uintptr_t)(port))
#define outl(val, port)	_outl(val, (uintptr_t)(port))

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

#include <iotrace.h>
#include <asm/types.h>
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

/**
 * nomap_sysmem() - pass through an address unchanged
 *
 * This is used to indicate an address which should NOT be mapped, e.g. in
 * SMBIOS tables. Using this function instead of a case shows that the sandbox
 * conversion has been done
 */
static inline void *nomap_sysmem(phys_addr_t paddr, unsigned long len)
{
	return (void *)(uintptr_t)paddr;
}

static inline phys_addr_t nomap_to_sysmem(const void *ptr)
{
	return (phys_addr_t)(uintptr_t)ptr;
}

#endif
