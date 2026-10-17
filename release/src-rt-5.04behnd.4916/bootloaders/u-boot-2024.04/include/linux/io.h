/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _LINUX_IO_H
#define _LINUX_IO_H

#include <linux/compiler.h>
#include <linux/types.h>
#include <asm/io.h>

#ifndef CONFIG_HAVE_ARCH_IOMAP
static inline u8 ioread8(const volatile void __iomem *addr)
{
	return readb(addr);
}

static inline u16 ioread16(const volatile void __iomem *addr)
{
	return readw(addr);
}

static inline u32 ioread32(const volatile void __iomem *addr)
{
	return readl(addr);
}
#endif /* !CONFIG_HAVE_ARCH_IOMAP */

#ifdef CONFIG_64BIT
static inline u64 ioread64(const volatile void __iomem *addr)
{
	return readq(addr);
}
#endif /* CONFIG_64BIT */

#ifndef CONFIG_HAVE_ARCH_IOMAP
static inline void iowrite8(u8 value, volatile void __iomem *addr)
{
	writeb(value, addr);
}

static inline void iowrite16(u16 value, volatile void __iomem *addr)
{
	writew(value, addr);
}

static inline void iowrite32(u32 value, volatile void __iomem *addr)
{
	writel(value, addr);
}
#endif /* !CONFIG_HAVE_ARCH_IOMAP */

#ifdef CONFIG_64BIT
static inline void iowrite64(u64 value, volatile void __iomem *addr)
{
	writeq(value, addr);
}
#endif /* CONFIG_64BIT */

#ifndef CONFIG_HAVE_ARCH_IOREMAP
static inline void __iomem *ioremap(resource_size_t offset,
				    resource_size_t size)
{
	return (void __iomem *)(unsigned long)offset;
}

static inline void iounmap(void __iomem *addr)
{
}
#endif

#define devm_ioremap(dev, offset, size)		ioremap(offset, size)

#endif /* _LINUX_IO_H */
