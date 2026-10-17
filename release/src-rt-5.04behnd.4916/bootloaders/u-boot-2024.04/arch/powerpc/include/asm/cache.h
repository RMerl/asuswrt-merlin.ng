/*
 * include/asm-ppc/cache.h
 */
#ifndef __ARCH_PPC_CACHE_H
#define __ARCH_PPC_CACHE_H

#include <asm/processor.h>

/* bytes per L1 cache line */
#if defined(CONFIG_MPC8xx)
#define	L1_CACHE_SHIFT	4
#elif defined(CONFIG_PPC64BRIDGE)
#define L1_CACHE_SHIFT	7
#elif defined(CONFIG_E500MC)
#define L1_CACHE_SHIFT	6
#else
#define	L1_CACHE_SHIFT	5
#endif

#define L1_CACHE_BYTES          (1 << L1_CACHE_SHIFT)

/*
 * Use the L1 data cache line size value for the minimum DMA buffer alignment
 * on PowerPC.
 */
#define ARCH_DMA_MINALIGN	L1_CACHE_BYTES

#define	L1_CACHE_ALIGN(x)       (((x)+(L1_CACHE_BYTES-1))&~(L1_CACHE_BYTES-1))
#define	L1_CACHE_PAGES		8

#define	SMP_CACHE_BYTES L1_CACHE_BYTES

#ifdef MODULE
#define __cacheline_aligned __attribute__((__aligned__(L1_CACHE_BYTES)))
#else
#define __cacheline_aligned					\
  __attribute__((__aligned__(L1_CACHE_BYTES)))			\
  __section(".data.cacheline_aligned")
#endif

#if defined(__KERNEL__) && !defined(__ASSEMBLY__)
extern void flush_dcache_range(unsigned long start, unsigned long stop);
extern void clean_dcache_range(unsigned long start, unsigned long stop);
extern void invalidate_dcache_range(unsigned long start, unsigned long stop);
extern void flush_dcache(void);
extern void invalidate_dcache(void);
extern void invalidate_icache(void);
extern void unlock_ram_in_cache(void);
#endif /* __ASSEMBLY__ */

#if defined(__KERNEL__) && !defined(__ASSEMBLY__)
int l2cache_init(void);
void enable_cpc(void);
void disable_cpc_sram(void);
#endif

/* prep registers for L2 */
#define CACHECRBA       0x80000823      /* Cache configuration register address */
#define L2CACHE_MASK	0x03	/* Mask for 2 L2 Cache bits */
#define L2CACHE_512KB	0x00	/* 512KB */
#define L2CACHE_256KB	0x01	/* 256KB */
#define L2CACHE_1MB	0x02	/* 1MB */
#define L2CACHE_NONE	0x03	/* NONE */
#define L2CACHE_PARITY  0x08    /* Mask for L2 Cache Parity Protected bit */

#ifdef CONFIG_MPC8xx
/* Cache control on the MPC8xx is provided through some additional
 * special purpose registers.
 */
#define IC_CST		560	/* Instruction cache control/status */
#define IC_ADR		561	/* Address needed for some commands */
#define IC_DAT		562	/* Read-only data register */
#define DC_CST		568	/* Data cache control/status */
#define DC_ADR		569	/* Address needed for some commands */
#define DC_DAT		570	/* Read-only data register */

/* Commands.  Only the first few are available to the instruction cache.
*/
#define	IDC_ENABLE	0x02000000	/* Cache enable */
#define IDC_DISABLE	0x04000000	/* Cache disable */
#define IDC_LDLCK	0x06000000	/* Load and lock */
#define IDC_UNLINE	0x08000000	/* Unlock line */
#define IDC_UNALL	0x0a000000	/* Unlock all */
#define IDC_INVALL	0x0c000000	/* Invalidate all */

#define DC_FLINE	0x0e000000	/* Flush data cache line */
#define DC_SFWT		0x01000000	/* Set forced writethrough mode */
#define DC_CFWT		0x03000000	/* Clear forced writethrough mode */
#define DC_SLES		0x05000000	/* Set little endian swap mode */
#define DC_CLES		0x07000000	/* Clear little endian swap mode */

/* Status.
*/
#define IDC_ENABLED	0x80000000	/* Cache is enabled */
#define IDC_CERR1	0x00200000	/* Cache error 1 */
#define IDC_CERR2	0x00100000	/* Cache error 2 */
#define IDC_CERR3	0x00080000	/* Cache error 3 */

#define DC_DFWT		0x40000000	/* Data cache is forced write through */
#define DC_LES		0x20000000	/* Caches are little endian mode */

#if !defined(__ASSEMBLY__)
static inline uint rd_ic_cst(void)
{
	return mfspr(IC_CST);
}

static inline void wr_ic_cst(uint val)
{
	mtspr(IC_CST, val);
}

static inline void wr_ic_adr(uint val)
{
	mtspr(IC_ADR, val);
}

static inline uint rd_dc_cst(void)
{
	return mfspr(DC_CST);
}

static inline void wr_dc_cst(uint val)
{
	mtspr(DC_CST, val);
}

static inline void wr_dc_adr(uint val)
{
	mtspr(DC_ADR, val);
}
#endif
#endif /* CONFIG_MPC8xx */

#endif
