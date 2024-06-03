/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2011 Andes Technology Corporation
 * Shawn Lin, Andes Technology Corporation <nobuhiro@andestech.com>
 * Macpaul Lin, Andes Technology Corporation <macpaul@andestech.com>
 */

#ifndef _ASM_CACHE_H
#define _ASM_CACHE_H

/* cache */
int	icache_status(void);
void	icache_enable(void);
void	icache_disable(void);
int	dcache_status(void);
void	dcache_enable(void);
void	dcache_disable(void);
void cache_flush(void);

#define DEFINE_GET_SYS_REG(reg) \
	static inline unsigned long GET_##reg(void)		\
	{							\
		unsigned long val;				\
		__asm__ volatile (				\
		"mfsr %0, $"#reg : "=&r" (val) : : "memory"	\
		);						\
		return val;					\
	}

enum cache_t {ICACHE, DCACHE};
DEFINE_GET_SYS_REG(ICM_CFG);
DEFINE_GET_SYS_REG(DCM_CFG);
/* I-cache sets (# of cache lines) per way */
#define ICM_CFG_OFF_ISET	0
/* I-cache ways */
#define ICM_CFG_OFF_IWAY	3
#define ICM_CFG_MSK_ISET	(0x7 << ICM_CFG_OFF_ISET)
#define ICM_CFG_MSK_IWAY	(0x7 << ICM_CFG_OFF_IWAY)
/* D-cache sets (# of cache lines) per way */
#define DCM_CFG_OFF_DSET	0
/* D-cache ways */
#define DCM_CFG_OFF_DWAY	3
#define DCM_CFG_MSK_DSET	(0x7 << DCM_CFG_OFF_DSET)
#define DCM_CFG_MSK_DWAY	(0x7 << DCM_CFG_OFF_DWAY)
/* I-cache line size */
#define ICM_CFG_OFF_ISZ	6
#define ICM_CFG_MSK_ISZ		(0x7UL << ICM_CFG_OFF_ISZ)
/* D-cache line size */
#define DCM_CFG_OFF_DSZ	6
#define DCM_CFG_MSK_DSZ		(0x7UL << DCM_CFG_OFF_DSZ)

/*
 * The current upper bound for NDS32 L1 data cache line sizes is 32 bytes.
 * We use that value for aligning DMA buffers unless the board config has
 * specified an alternate cache line size.
 */
#ifdef CONFIG_SYS_CACHELINE_SIZE
#define ARCH_DMA_MINALIGN	CONFIG_SYS_CACHELINE_SIZE
#else
#define ARCH_DMA_MINALIGN	32
#endif

#endif /* _ASM_CACHE_H */
