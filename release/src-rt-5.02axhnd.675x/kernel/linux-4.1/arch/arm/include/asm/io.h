/*
 *  arch/arm/include/asm/io.h
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
 *  05-Oct-2004 BJD     Moved memory string functions to use void __iomem
 */
#ifndef __ASM_ARM_IO_H
#define __ASM_ARM_IO_H

#ifdef __KERNEL__

#include <linux/types.h>
#include <linux/blk_types.h>
#include <asm/byteorder.h>
#include <asm/memory.h>
#include <asm-generic/pci_iomap.h>
#include <xen/xen.h>

/*
 * ISA I/O bus memory addresses are 1:1 with the physical address.
 */
#define isa_virt_to_bus virt_to_phys
#define isa_page_to_bus page_to_phys
#define isa_bus_to_virt phys_to_virt

/*
 * Atomic MMIO-wide IO modify
 */
extern void atomic_io_modify(void __iomem *reg, u32 mask, u32 set);
extern void atomic_io_modify_relaxed(void __iomem *reg, u32 mask, u32 set);

/*
 * Generic IO read/write.  These perform native-endian accesses.  Note
 * that some architectures will want to re-define __raw_{read,write}w.
 */
void __raw_writesb(volatile void __iomem *addr, const void *data, int bytelen);
void __raw_writesw(volatile void __iomem *addr, const void *data, int wordlen);
void __raw_writesl(volatile void __iomem *addr, const void *data, int longlen);

void __raw_readsb(const volatile void __iomem *addr, void *data, int bytelen);
void __raw_readsw(const volatile void __iomem *addr, void *data, int wordlen);
void __raw_readsl(const volatile void __iomem *addr, void *data, int longlen);

#if __LINUX_ARM_ARCH__ < 6
/*
 * Half-word accesses are problematic with RiscPC due to limitations of
 * the bus. Rather than special-case the machine, just let the compiler
 * generate the access for CPUs prior to ARMv6.
 */
#define __raw_readw(a)         (__chk_io_ptr(a), *(volatile unsigned short __force *)(a))
#define __raw_writew(v,a)      ((void)(__chk_io_ptr(a), *(volatile unsigned short __force *)(a) = (v)))
#else
/*
 * When running under a hypervisor, we want to avoid I/O accesses with
 * writeback addressing modes as these incur a significant performance
 * overhead (the address generation must be emulated in software).
 */
#define __raw_writew __raw_writew
static inline void __raw_writew(u16 val, volatile void __iomem *addr)
{
	asm volatile("strh %1, %0"
		     : "+Q" (*(volatile u16 __force *)addr)
		     : "r" (val));
}

#define __raw_readw __raw_readw
static inline u16 __raw_readw(const volatile void __iomem *addr)
{
	u16 val;
	asm volatile("ldrh %1, %0"
		     : "+Q" (*(volatile u16 __force *)addr),
		       "=r" (val));
	return val;
}
#endif

#define __raw_writeb __raw_writeb
static inline void __raw_writeb(u8 val, volatile void __iomem *addr)
{
	asm volatile("strb %1, %0"
		     : "+Qo" (*(volatile u8 __force *)addr)
		     : "r" (val));
}

#define __raw_writel __raw_writel
static inline void __raw_writel(u32 val, volatile void __iomem *addr)
{
	asm volatile("str %1, %0"
		     : "+Qo" (*(volatile u32 __force *)addr)
		     : "r" (val));
}

#define __raw_readb __raw_readb
static inline u8 __raw_readb(const volatile void __iomem *addr)
{
	u8 val;
	asm volatile("ldrb %1, %0"
		     : "+Qo" (*(volatile u8 __force *)addr),
		       "=r" (val));
	return val;
}

#define __raw_readl __raw_readl
static inline u32 __raw_readl(const volatile void __iomem *addr)
{
	u32 val;
	asm volatile("ldr %1, %0"
		     : "+Qo" (*(volatile u32 __force *)addr),
		       "=r" (val));
	return val;
}

/*
 * Architecture ioremap implementation.
 */
#define MT_DEVICE		0
#define MT_DEVICE_NONSHARED	1
#define MT_DEVICE_CACHED	2
#define MT_DEVICE_WC		3
#if defined(CONFIG_BCM_KF_ARM_BCM963XX) && defined(CONFIG_PLAT_BCM63XX_ACP)
#define MT_DEVICE_NONSECURED	15
#endif /* CONFIG_BCM_KF_ARM_BCM963XX && CONFIG_PLAT_BCM63XX_ACP */
/*
 * types 4 onwards can be found in asm/mach/map.h and are undefined
 * for ioremap
 */

/*
 * __arm_ioremap takes CPU physical address.
 * __arm_ioremap_pfn takes a Page Frame Number and an offset into that page
 * The _caller variety takes a __builtin_return_address(0) value for
 * /proc/vmalloc to use - and should only be used in non-inline functions.
 */
extern void __iomem *__arm_ioremap_pfn_caller(unsigned long, unsigned long,
	size_t, unsigned int, void *);
extern void __iomem *__arm_ioremap_caller(phys_addr_t, size_t, unsigned int,
	void *);

extern void __iomem *__arm_ioremap_pfn(unsigned long, unsigned long, size_t, unsigned int);
extern void __iomem *__arm_ioremap(phys_addr_t, size_t, unsigned int);
extern void __iomem *__arm_ioremap_exec(phys_addr_t, size_t, bool cached);
extern void __iounmap(volatile void __iomem *addr);
extern void __arm_iounmap(volatile void __iomem *addr);

extern void __iomem * (*arch_ioremap_caller)(phys_addr_t, size_t,
	unsigned int, void *);
extern void (*arch_iounmap)(volatile void __iomem *);

/*
 * Bad read/write accesses...
 */
extern void __readwrite_bug(const char *fn);

/*
 * A typesafe __io() helper
 */
static inline void __iomem *__typesafe_io(unsigned long addr)
{
	return (void __iomem *)addr;
}

#define IOMEM(x)	((void __force __iomem *)(x))

/* IO barriers */
#ifdef CONFIG_ARM_DMA_MEM_BUFFERABLE
#include <asm/barrier.h>
#define __iormb()		rmb()
#define __iowmb()		wmb()
#else
#define __iormb()		do { } while (0)
#define __iowmb()		do { } while (0)
#endif

/* PCI fixed i/o mapping */
#define PCI_IO_VIRT_BASE	0xfee00000
#define PCI_IOBASE		((void __iomem *)PCI_IO_VIRT_BASE)

#if defined(CONFIG_PCI)
void pci_ioremap_set_mem_type(int mem_type);
#else
static inline void pci_ioremap_set_mem_type(int mem_type) {}
#endif

extern int pci_ioremap_io(unsigned int offset, phys_addr_t phys_addr);

/*
 * Now, pick up the machine-defined IO definitions
 */
#ifdef CONFIG_NEED_MACH_IO_H
#include <mach/io.h>
#elif defined(CONFIG_PCI)
#define IO_SPACE_LIMIT	((resource_size_t)0xfffff)
#define __io(a)		__typesafe_io(PCI_IO_VIRT_BASE + ((a) & IO_SPACE_LIMIT))
#else
#define __io(a)		__typesafe_io((a) & IO_SPACE_LIMIT)
#endif

/*
 * This is the limit of PC card/PCI/ISA IO space, which is by default
 * 64K if we have PC card, PCI or ISA support.  Otherwise, default to
 * zero to prevent ISA/PCI drivers claiming IO space (and potentially
 * oopsing.)
 *
 * Only set this larger if you really need inb() et.al. to operate over
 * a larger address space.  Note that SOC_COMMON ioremaps each sockets
 * IO space area, and so inb() et.al. must be defined to operate as per
 * readb() et.al. on such platforms.
 */
#ifndef IO_SPACE_LIMIT
#if defined(CONFIG_PCMCIA_SOC_COMMON) || defined(CONFIG_PCMCIA_SOC_COMMON_MODULE)
#define IO_SPACE_LIMIT ((resource_size_t)0xffffffff)
#elif defined(CONFIG_PCI) || defined(CONFIG_ISA) || defined(CONFIG_PCCARD)
#define IO_SPACE_LIMIT ((resource_size_t)0xffff)
#else
#define IO_SPACE_LIMIT ((resource_size_t)0)
#endif
#endif

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
#define outb(v,p)	({ __iowmb(); __raw_writeb(v,__io(p)); })
#define outw(v,p)	({ __iowmb(); __raw_writew((__force __u16) \
					cpu_to_le16(v),__io(p)); })
#define outl(v,p)	({ __iowmb(); __raw_writel((__force __u32) \
					cpu_to_le32(v),__io(p)); })

#define inb(p)	({ __u8 __v = __raw_readb(__io(p)); __iormb(); __v; })
#define inw(p)	({ __u16 __v = le16_to_cpu((__force __le16) \
			__raw_readw(__io(p))); __iormb(); __v; })
#define inl(p)	({ __u32 __v = le32_to_cpu((__force __le32) \
			__raw_readl(__io(p))); __iormb(); __v; })

#define outsb(p,d,l)		__raw_writesb(__io(p),d,l)
#define outsw(p,d,l)		__raw_writesw(__io(p),d,l)
#define outsl(p,d,l)		__raw_writesl(__io(p),d,l)

#define insb(p,d,l)		__raw_readsb(__io(p),d,l)
#define insw(p,d,l)		__raw_readsw(__io(p),d,l)
#define insl(p,d,l)		__raw_readsl(__io(p),d,l)
#endif

/*
 * String version of IO memory access ops:
 */
extern void _memcpy_fromio(void *, const volatile void __iomem *, size_t);
extern void _memcpy_toio(volatile void __iomem *, const void *, size_t);
extern void _memset_io(volatile void __iomem *, int, size_t);

#define mmiowb()

/*
 *  Memory access primitives
 *  ------------------------
 *
 * These perform PCI memory accesses via an ioremap region.  They don't
 * take an address as such, but a cookie.
 *
 * Again, this are defined to perform little endian accesses.  See the
 * IO port primitives for more information.
 */
#ifndef readl
#define readb_relaxed(c) ({ u8  __r = __raw_readb(c); __r; })
#define readw_relaxed(c) ({ u16 __r = le16_to_cpu((__force __le16) \
					__raw_readw(c)); __r; })
#define readl_relaxed(c) ({ u32 __r = le32_to_cpu((__force __le32) \
					__raw_readl(c)); __r; })

#define writeb_relaxed(v,c)	__raw_writeb(v,c)
#define writew_relaxed(v,c)	__raw_writew((__force u16) cpu_to_le16(v),c)
#define writel_relaxed(v,c)	__raw_writel((__force u32) cpu_to_le32(v),c)

#define readb(c)		({ u8  __v = readb_relaxed(c); __iormb(); __v; })
#define readw(c)		({ u16 __v = readw_relaxed(c); __iormb(); __v; })
#define readl(c)		({ u32 __v = readl_relaxed(c); __iormb(); __v; })

#define writeb(v,c)		({ __iowmb(); writeb_relaxed(v,c); })
#define writew(v,c)		({ __iowmb(); writew_relaxed(v,c); })
#define writel(v,c)		({ __iowmb(); writel_relaxed(v,c); })

#define readsb(p,d,l)		__raw_readsb(p,d,l)
#define readsw(p,d,l)		__raw_readsw(p,d,l)
#define readsl(p,d,l)		__raw_readsl(p,d,l)

#define writesb(p,d,l)		__raw_writesb(p,d,l)
#define writesw(p,d,l)		__raw_writesw(p,d,l)
#define writesl(p,d,l)		__raw_writesl(p,d,l)

#define memset_io(c,v,l)	_memset_io(c,(v),(l))
#define memcpy_fromio(a,c,l)	_memcpy_fromio((a),c,(l))
#define memcpy_toio(c,a,l)	_memcpy_toio(c,(a),(l))

#endif	/* readl */

/*
 * ioremap and friends.
 *
 * ioremap takes a PCI memory address, as specified in
 * Documentation/io-mapping.txt.
 *
 */
#define ioremap(cookie,size)		__arm_ioremap((cookie), (size), MT_DEVICE)
#define ioremap_nocache(cookie,size)	__arm_ioremap((cookie), (size), MT_DEVICE)
#define ioremap_cache(cookie,size)	__arm_ioremap((cookie), (size), MT_DEVICE_CACHED)
#define ioremap_wc(cookie,size)		__arm_ioremap((cookie), (size), MT_DEVICE_WC)
#if defined(CONFIG_BCM_KF_ARM_BCM963XX) && defined(CONFIG_PLAT_BCM63XX_ACP)
#define ioremap_nonsecured(cookie,size)	__arm_ioremap((cookie), (size), MT_DEVICE_NONSECURED)
#endif /* CONFIG_BCM_KF_ARM_BCM963XX && CONFIG_PLAT_BCM63XX_ACP */
#define iounmap				__arm_iounmap

/*
 * io{read,write}{16,32}be() macros
 */
#define ioread16be(p)		({ __u16 __v = be16_to_cpu((__force __be16)__raw_readw(p)); __iormb(); __v; })
#define ioread32be(p)		({ __u32 __v = be32_to_cpu((__force __be32)__raw_readl(p)); __iormb(); __v; })

#define iowrite16be(v,p)	({ __iowmb(); __raw_writew((__force __u16)cpu_to_be16(v), p); })
#define iowrite32be(v,p)	({ __iowmb(); __raw_writel((__force __u32)cpu_to_be32(v), p); })

#ifndef ioport_map
#define ioport_map ioport_map
extern void __iomem *ioport_map(unsigned long port, unsigned int nr);
#endif
#ifndef ioport_unmap
#define ioport_unmap ioport_unmap
extern void ioport_unmap(void __iomem *addr);
#endif

struct pci_dev;

#define pci_iounmap pci_iounmap
extern void pci_iounmap(struct pci_dev *dev, void __iomem *addr);

/*
 * Convert a physical pointer to a virtual kernel pointer for /dev/mem
 * access
 */
#define xlate_dev_mem_ptr(p)	__va(p)

/*
 * Convert a virtual cached pointer to an uncached pointer
 */
#define xlate_dev_kmem_ptr(p)	p

#include <asm-generic/io.h>

/*
 * can the hardware map this into one segment or not, given no other
 * constraints.
 */
#define BIOVEC_MERGEABLE(vec1, vec2)	\
	((bvec_to_phys((vec1)) + (vec1)->bv_len) == bvec_to_phys((vec2)))

struct bio_vec;
extern bool xen_biovec_phys_mergeable(const struct bio_vec *vec1,
				      const struct bio_vec *vec2);
#define BIOVEC_PHYS_MERGEABLE(vec1, vec2)				\
	(__BIOVEC_PHYS_MERGEABLE(vec1, vec2) &&				\
	 (!xen_domain() || xen_biovec_phys_mergeable(vec1, vec2)))

#ifdef CONFIG_MMU
#define ARCH_HAS_VALID_PHYS_ADDR_RANGE
extern int valid_phys_addr_range(phys_addr_t addr, size_t size);
extern int valid_mmap_phys_addr_range(unsigned long pfn, size_t size);
extern int devmem_is_allowed(unsigned long pfn);
#endif

/*
 * Register ISA memory and port locations for glibc iopl/inb/outb
 * emulation.
 */
extern void register_isa_ports(unsigned int mmio, unsigned int io,
			       unsigned int io_shift);

#endif	/* __KERNEL__ */
#endif	/* __ASM_ARM_IO_H */
