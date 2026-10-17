/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2017 Andes Technology Corporation
 * Rick Chen, Andes Technology Corporation <rick@andestech.com>
 *
 */
#ifndef __ASM_RISCV_IO_H
#define __ASM_RISCV_IO_H

#include <linux/types.h>
#include <asm/barrier.h>
#include <asm/byteorder.h>

static inline void sync(void)
{
}

#define __arch_getb(a)			(*(volatile unsigned char *)(a))
#define __arch_getw(a)			(*(volatile unsigned short *)(a))
#define __arch_getl(a)			(*(volatile unsigned int *)(a))
#define __arch_getq(a)			(*(volatile unsigned long long *)(a))

#define __arch_putb(v, a)		(*(volatile unsigned char *)(a) = (v))
#define __arch_putw(v, a)		(*(volatile unsigned short *)(a) = (v))
#define __arch_putl(v, a)		(*(volatile unsigned int *)(a) = (v))
#define __arch_putq(v, a)		(*(volatile unsigned long long *)(a) = (v))

#define __raw_writeb(v, a)		__arch_putb(v, a)
#define __raw_writew(v, a)		__arch_putw(v, a)
#define __raw_writel(v, a)		__arch_putl(v, a)
#define __raw_writeq(v, a)		__arch_putq(v, a)

#define __raw_readb(a)			__arch_getb(a)
#define __raw_readw(a)			__arch_getw(a)
#define __raw_readl(a)			__arch_getl(a)
#define __raw_readq(a)			__arch_getq(a)

/* adding for cadence_qspi_apb.c */
#define memcpy_fromio(a, c, l)		memcpy((a), (c), (l))
#define memcpy_toio(c, a, l)		memcpy((c), (a), (l))

#define dmb()		mb()
#define __iormb()	rmb()
#define __iowmb()	wmb()

static inline void writeb(u8 val, volatile void __iomem *addr)
{
	__iowmb();
	__arch_putb(val, addr);
}

static inline void writew(u16 val, volatile void __iomem *addr)
{
	__iowmb();
	__arch_putw(val, addr);
}

static inline void writel(u32 val, volatile void __iomem *addr)
{
	__iowmb();
	__arch_putl(val, addr);
}

static inline void writeq(u64 val, volatile void __iomem *addr)
{
	__iowmb();
	__arch_putq(val, addr);
}

static inline u8 readb(const volatile void __iomem *addr)
{
	u8	val;

	val = __arch_getb(addr);
	__iormb();
	return val;
}

static inline u16 readw(const volatile void __iomem *addr)
{
	u16	val;

	val = __arch_getw(addr);
	__iormb();
	return val;
}

static inline u32 readl(const volatile void __iomem *addr)
{
	u32	val;

	val = __arch_getl(addr);
	__iormb();
	return val;
}

static inline u64 readq(const volatile void __iomem *addr)
{
	u64	val;

	val = __arch_getq(addr);
	__iormb();
	return val;
}

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

#define out_arch(type, endian, a, v)	__raw_write##type(cpu_to_##endian(v), a)
#define in_arch(type, endian, a)	endian##_to_cpu(__raw_read##type(a))

#define out_le32(a, v)			out_arch(l, le32, a, v)
#define out_le16(a, v)			out_arch(w, le16, a, v)

#define in_le32(a)			in_arch(l, le32, a)
#define in_le16(a)			in_arch(w, le16, a)

#define out_be32(a, v)			out_arch(l, be32, a, v)
#define out_be16(a, v)			out_arch(w, be16, a, v)

#define in_be32(a)			in_arch(l, be32, a)
#define in_be16(a)			in_arch(w, be16, a)

#define out_8(a, v)			__raw_writeb(v, a)
#define in_8(a)				__raw_readb(a)

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

/*
 * Now, pick up the machine-defined IO definitions
 * #include <asm/arch/io.h>
 */

/*
 *  IO port access primitives
 *  -------------------------
 *
 * The RISC-V doesn't have special IO access instructions just like ARM;
 * all IO is memory mapped.
 * Note that these are defined to perform little endian accesses
 * only.  Their primary purpose is to access PCI and ISA peripherals.
 *
 * Note that for a big endian machine, this implies that the following
 * big endian mode connectivity is in place, as described by numerious
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
#define outb(v, p)			__raw_writeb(v, __io(p))
#define outw(v, p)			__raw_writew(cpu_to_le16(v), __io(p))
#define outl(v, p)			__raw_writel(cpu_to_le32(v), __io(p))

#define inb(p)	({ unsigned int __v = __raw_readb(__io(p)); __v; })
#define inw(p)	({ unsigned int __v = le16_to_cpu(__raw_readw(__io(p))); __v; })
#define inl(p)	({ unsigned int __v = le32_to_cpu(__raw_readl(__io(p))); __v; })

#define outsb(p, d, l)			writesb(__io(p), d, l)
#define outsw(p, d, l)			writesw(__io(p), d, l)
#define outsl(p, d, l)			writesl(__io(p), d, l)

#define insb(p, d, l)			readsb(__io(p), d, l)
#define insw(p, d, l)			readsw(__io(p), d, l)
#define insl(p, d, l)			readsl(__io(p), d, l)

static inline void readsb(const volatile void __iomem *addr, void *data,
			  unsigned int bytelen)
{
	unsigned char *ptr;
	unsigned char *ptr2;

	ptr = (unsigned char *)addr;
	ptr2 = (unsigned char *)data;

	while (bytelen) {
		*ptr2 = *ptr;
		ptr2++;
		bytelen--;
	}
}

static inline void readsw(const volatile void __iomem *addr, void *data,
			  unsigned int wordlen)
{
	unsigned short *ptr;
	unsigned short *ptr2;

	ptr = (unsigned short *)addr;
	ptr2 = (unsigned short *)data;

	while (wordlen) {
		*ptr2 = *ptr;
		ptr2++;
		wordlen--;
	}
}

static inline void readsl(const volatile void __iomem *addr, void *data,
			  unsigned int longlen)
{
	unsigned int *ptr;
	unsigned int *ptr2;

	ptr = (unsigned int *)addr;
	ptr2 = (unsigned int *)data;

	while (longlen) {
		*ptr2 = *ptr;
		ptr2++;
		longlen--;
	}
}

static inline void writesb(volatile void __iomem *addr, const void *data,
			   unsigned int bytelen)
{
	unsigned char *ptr;
	unsigned char *ptr2;

	ptr = (unsigned char *)addr;
	ptr2 = (unsigned char *)data;

	while (bytelen) {
		*ptr = *ptr2;
		ptr2++;
		bytelen--;
	}
}

static inline void writesw(volatile void __iomem *addr, const void *data,
			   unsigned int wordlen)
{
	unsigned short *ptr;
	unsigned short *ptr2;

	ptr = (unsigned short *)addr;
	ptr2 = (unsigned short *)data;

	while (wordlen) {
		*ptr = *ptr2;
		ptr2++;
		wordlen--;
	}
}

static inline void writesl(volatile void __iomem *addr, const void *data,
			   unsigned int longlen)
{
	unsigned int *ptr;
	unsigned int *ptr2;

	ptr = (unsigned int *)addr;
	ptr2 = (unsigned int *)data;

	while (longlen) {
		*ptr = *ptr2;
		ptr2++;
		longlen--;
	}
}

#define readsb readsb
#define readsw readsw
#define readsl readsl
#define writesb writesb
#define writesw writesw
#define writesl writesl

#endif

#define outb_p(val, port)		outb((val), (port))
#define outw_p(val, port)		outw((val), (port))
#define outl_p(val, port)		outl((val), (port))
#define inb_p(port)			inb((port))
#define inw_p(port)			inw((port))
#define inl_p(port)			inl((port))

#define outsb_p(port, from, len)	outsb(port, from, len)
#define outsw_p(port, from, len)	outsw(port, from, len)
#define outsl_p(port, from, len)	outsl(port, from, len)
#define insb_p(port, to, len)		insb(port, to, len)
#define insw_p(port, to, len)		insw(port, to, len)
#define insl_p(port, to, len)		insl(port, to, len)

/*
 * Unordered I/O memory access primitives.  These are even more relaxed than
 * the relaxed versions, as they don't even order accesses between successive
 * operations to the I/O regions.
 */
#define readb_cpu(c)		({ u8  __r = __raw_readb(c); __r; })
#define readw_cpu(c)		({ u16 __r = le16_to_cpu((__force __le16)__raw_readw(c)); __r; })
#define readl_cpu(c)		({ u32 __r = le32_to_cpu((__force __le32)__raw_readl(c)); __r; })

#define writeb_cpu(v, c)	((void)__raw_writeb((v), (c)))
#define writew_cpu(v, c)	((void)__raw_writew((__force u16)cpu_to_le16(v), (c)))
#define writel_cpu(v, c)	((void)__raw_writel((__force u32)cpu_to_le32(v), (c)))

#ifdef CONFIG_64BIT
#define readq_cpu(c)		({ u64 __r = le64_to_cpu((__force __le64)__raw_readq(c)); __r; })
#define writeq_cpu(v, c)	((void)__raw_writeq((__force u64)cpu_to_le64(v), (c)))
#endif

/*
 * Relaxed I/O memory access primitives. These follow the Device memory
 * ordering rules but do not guarantee any ordering relative to Normal memory
 * accesses.  These are defined to order the indicated access (either a read or
 * write) with all other I/O memory accesses to the same peripheral. Since the
 * platform specification defines that all I/O regions are strongly ordered on
 * channel 0, no explicit fences are required to enforce this ordering.
 */
/* FIXME: These are now the same as asm-generic */
#define __io_rbr()		do {} while (0)
#define __io_rar()		do {} while (0)
#define __io_rbw()		do {} while (0)
#define __io_raw()		do {} while (0)

#define readb_relaxed(c)	({ u8  __v; __io_rbr(); __v = readb_cpu(c); __io_rar(); __v; })
#define readw_relaxed(c)	({ u16 __v; __io_rbr(); __v = readw_cpu(c); __io_rar(); __v; })
#define readl_relaxed(c)	({ u32 __v; __io_rbr(); __v = readl_cpu(c); __io_rar(); __v; })

#define writeb_relaxed(v, c)	({ __io_rbw(); writeb_cpu((v), (c)); __io_raw(); })
#define writew_relaxed(v, c)	({ __io_rbw(); writew_cpu((v), (c)); __io_raw(); })
#define writel_relaxed(v, c)	({ __io_rbw(); writel_cpu((v), (c)); __io_raw(); })

#ifdef CONFIG_64BIT
#define readq_relaxed(c)	({ u64 __v; __io_rbr(); __v = readq_cpu(c); __io_rar(); __v; })
#define writeq_relaxed(v, c)	({ __io_rbw(); writeq_cpu((v), (c)); __io_raw(); })
#endif

#include <asm-generic/io.h>

#endif	/* __ASM_RISCV_IO_H */
