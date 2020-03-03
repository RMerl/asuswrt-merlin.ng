#ifndef _ASM_IO_64_NONATOMIC_HI_LO_H_
#define _ASM_IO_64_NONATOMIC_HI_LO_H_

#include <linux/io.h>
#include <asm-generic/int-ll64.h>

static inline __u64 hi_lo_readq(const volatile void __iomem *addr)
{
	const volatile u32 __iomem *p = addr;
	u32 low, high;

	high = readl(p + 1);
	low = readl(p);

	return low + ((u64)high << 32);
}

static inline void hi_lo_writeq(__u64 val, volatile void __iomem *addr)
{
	writel(val >> 32, addr + 4);
	writel(val, addr);
}

#ifndef readq
#define readq hi_lo_readq
#endif

#ifndef writeq
#define writeq hi_lo_writeq
#endif

#endif	/* _ASM_IO_64_NONATOMIC_HI_LO_H_ */
