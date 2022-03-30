// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <asm/arch/immap_ls102xa.h>
#include <asm/cache.h>
#include <asm/system.h>
#include <tsec.h>
#include <netdev.h>
#include <fsl_esdhc.h>
#include <config.h>
#include <fsl_wdog.h>

#include "fsl_epu.h"

#define DCSR_RCPM2_BLOCK_OFFSET	0x223000
#define DCSR_RCPM2_CPMFSMCR0	0x400
#define DCSR_RCPM2_CPMFSMSR0	0x404
#define DCSR_RCPM2_CPMFSMCR1	0x414
#define DCSR_RCPM2_CPMFSMSR1	0x418
#define CPMFSMSR_FSM_STATE_MASK	0x7f

DECLARE_GLOBAL_DATA_PTR;

#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)

/*
 * Bit[1] of the descriptor indicates the descriptor type,
 * and bit[0] indicates whether the descriptor is valid.
 */
#define PMD_TYPE_TABLE		0x3
#define PMD_TYPE_SECT		0x1

/* AttrIndx[2:0] */
#define PMD_ATTRINDX(t)		((t) << 2)

/* Section */
#define PMD_SECT_AF		(1 << 10)

#define BLOCK_SIZE_L1		(1UL << 30)
#define BLOCK_SIZE_L2		(1UL << 21)

/* TTBCR flags */
#define TTBCR_EAE		(1 << 31)
#define TTBCR_T0SZ(x)		((x) << 0)
#define TTBCR_T1SZ(x)		((x) << 16)
#define TTBCR_USING_TTBR0	(TTBCR_T0SZ(0) | TTBCR_T1SZ(0))
#define TTBCR_IRGN0_NC		(0 << 8)
#define TTBCR_IRGN0_WBWA	(1 << 8)
#define TTBCR_IRGN0_WT		(2 << 8)
#define TTBCR_IRGN0_WBNWA	(3 << 8)
#define TTBCR_IRGN0_MASK	(3 << 8)
#define TTBCR_ORGN0_NC		(0 << 10)
#define TTBCR_ORGN0_WBWA	(1 << 10)
#define TTBCR_ORGN0_WT		(2 << 10)
#define TTBCR_ORGN0_WBNWA	(3 << 10)
#define TTBCR_ORGN0_MASK	(3 << 10)
#define TTBCR_SHARED_NON	(0 << 12)
#define TTBCR_SHARED_OUTER	(2 << 12)
#define TTBCR_SHARED_INNER	(3 << 12)
#define TTBCR_EPD0		(0 << 7)
#define TTBCR			(TTBCR_SHARED_NON | \
				 TTBCR_ORGN0_NC	| \
				 TTBCR_IRGN0_NC	| \
				 TTBCR_USING_TTBR0 | \
				 TTBCR_EAE)

/*
 * Memory region attributes for LPAE (defined in pgtable):
 *
 * n = AttrIndx[2:0]
 *
 *		              n       MAIR
 *	UNCACHED              000     00000000
 *	BUFFERABLE            001     01000100
 *	DEV_WC                001     01000100
 *	WRITETHROUGH          010     10101010
 *	WRITEBACK             011     11101110
 *	DEV_CACHED            011     11101110
 *	DEV_SHARED            100     00000100
 *	DEV_NONSHARED         100     00000100
 *	unused                101
 *	unused                110
 *	WRITEALLOC            111     11111111
 */
#define MT_MAIR0		0xeeaa4400
#define MT_MAIR1		0xff000004
#define MT_STRONLY_ORDER	0
#define MT_NORMAL_NC		1
#define MT_DEVICE_MEM		4
#define MT_NORMAL		7

/* The phy_addr must be aligned to 4KB */
static inline void set_pgtable(u32 *page_table, u32 index, u32 phy_addr)
{
	u32 value = phy_addr | PMD_TYPE_TABLE;

	page_table[2 * index] = value;
	page_table[2 * index + 1] = 0;
}

/* The phy_addr must be aligned to 4KB */
static inline void set_pgsection(u32 *page_table, u32 index, u64 phy_addr,
				 u32 memory_type)
{
	u64 value;

	value = phy_addr | PMD_TYPE_SECT | PMD_SECT_AF;
	value |= PMD_ATTRINDX(memory_type);
	page_table[2 * index] = value & 0xFFFFFFFF;
	page_table[2 * index + 1] = (value >> 32) & 0xFFFFFFFF;
}

/*
 * Start MMU after DDR is available, we create MMU table in DRAM.
 * The base address of TTLB is gd->arch.tlb_addr. We use two
 * levels of translation tables here to cover 40-bit address space.
 *
 * The TTLBs are located at PHY 2G~4G.
 *
 * VA mapping:
 *
 *  -------  <---- 0GB
 * |       |
 * |       |
 * |-------| <---- 0x24000000
 * |///////|  ===> 192MB VA map for PCIe1 with offset 0x40_0000_0000
 * |-------| <---- 0x300000000
 * |       |
 * |-------| <---- 0x34000000
 * |///////|  ===> 192MB VA map for PCIe2 with offset 0x48_0000_0000
 * |-------| <---- 0x40000000
 * |       |
 * |-------| <---- 0x80000000 DDR0 space start
 * |\\\\\\\|
 *.|\\\\\\\|  ===> 2GB VA map for 2GB DDR0 Memory space
 * |\\\\\\\|
 *  -------  <---- 4GB DDR0 space end
 */
static void mmu_setup(void)
{
	u32 *level0_table = (u32 *)gd->arch.tlb_addr;
	u32 *level1_table = (u32 *)(gd->arch.tlb_addr + 0x1000);
	u64 va_start = 0;
	u32 reg;
	int i;

	/* Level 0 Table 2-3 are used to map DDR */
	set_pgsection(level0_table, 3, 3 * BLOCK_SIZE_L1, MT_NORMAL);
	set_pgsection(level0_table, 2, 2 * BLOCK_SIZE_L1, MT_NORMAL);
	/* Level 0 Table 1 is used to map device */
	set_pgsection(level0_table, 1, 1 * BLOCK_SIZE_L1, MT_DEVICE_MEM);
	/* Level 0 Table 0 is used to map device including PCIe MEM */
	set_pgtable(level0_table, 0, (u32)level1_table);

	/* Level 1 has 512 entries */
	for (i = 0; i < 512; i++) {
		/* Mapping for PCIe 1 */
		if (va_start >= CONFIG_SYS_PCIE1_VIRT_ADDR &&
		    va_start < (CONFIG_SYS_PCIE1_VIRT_ADDR +
				 CONFIG_SYS_PCIE_MMAP_SIZE))
			set_pgsection(level1_table, i,
				      CONFIG_SYS_PCIE1_PHYS_BASE + va_start,
				      MT_DEVICE_MEM);
		/* Mapping for PCIe 2 */
		else if (va_start >= CONFIG_SYS_PCIE2_VIRT_ADDR &&
			 va_start < (CONFIG_SYS_PCIE2_VIRT_ADDR +
				     CONFIG_SYS_PCIE_MMAP_SIZE))
			set_pgsection(level1_table, i,
				      CONFIG_SYS_PCIE2_PHYS_BASE + va_start,
				      MT_DEVICE_MEM);
		else
			set_pgsection(level1_table, i,
				      va_start,
				      MT_DEVICE_MEM);
		va_start += BLOCK_SIZE_L2;
	}

	asm volatile("dsb sy;isb");
	asm volatile("mcr p15, 0, %0, c2, c0, 2" /* Write RT to TTBCR */
			: : "r" (TTBCR) : "memory");
	asm volatile("mcrr p15, 0, %0, %1, c2" /* TTBR 0 */
			: : "r" ((u32)level0_table), "r" (0) : "memory");
	asm volatile("mcr p15, 0, %0, c10, c2, 0" /* write MAIR 0 */
			: : "r" (MT_MAIR0) : "memory");
	asm volatile("mcr p15, 0, %0, c10, c2, 1" /* write MAIR 1 */
			: : "r" (MT_MAIR1) : "memory");

	/* Set the access control to all-supervisor */
	asm volatile("mcr p15, 0, %0, c3, c0, 0"
		     : : "r" (~0));

	/* Enable the mmu */
	reg = get_cr();
	set_cr(reg | CR_M);
}

/*
 * This function is called from lib/board.c. It recreates MMU
 * table in main memory. MMU and i/d-cache are enabled here.
 */
void enable_caches(void)
{
	/* Invalidate all TLB */
	mmu_page_table_flush(gd->arch.tlb_addr,
			     gd->arch.tlb_addr +  gd->arch.tlb_size);
	/* Set up and enable mmu */
	mmu_setup();

	/* Invalidate & Enable d-cache */
	invalidate_dcache_all();
	set_cr(get_cr() | CR_C);
}
#endif /* #if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF) */


uint get_svr(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);

	return in_be32(&gur->svr);
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	char buf1[32], buf2[32];
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	unsigned int svr, major, minor, ver, i;

	svr = in_be32(&gur->svr);
	major = SVR_MAJ(svr);
	minor = SVR_MIN(svr);

	puts("CPU:   Freescale LayerScape ");

	ver = SVR_SOC_VER(svr);
	switch (ver) {
	case SOC_VER_SLS1020:
		puts("SLS1020");
		break;
	case SOC_VER_LS1020:
		puts("LS1020");
		break;
	case SOC_VER_LS1021:
		puts("LS1021");
		break;
	case SOC_VER_LS1022:
		puts("LS1022");
		break;
	default:
		puts("Unknown");
		break;
	}

	if (IS_E_PROCESSOR(svr) && (ver != SOC_VER_SLS1020))
		puts("E");

	printf(", Version: %d.%d, (0x%08x)\n", major, minor, svr);

	puts("Clock Configuration:");

	printf("\n       CPU0(ARMV7):%-4s MHz, ", strmhz(buf1, gd->cpu_clk));
	printf("\n       Bus:%-4s MHz, ", strmhz(buf1, gd->bus_clk));
	printf("DDR:%-4s MHz (%s MT/s data rate), ",
	       strmhz(buf1, gd->mem_clk/2), strmhz(buf2, gd->mem_clk));
	puts("\n");

	/* Display the RCW, so that no one gets confused as to what RCW
	 * we're actually using for this boot.
	 */
	puts("Reset Configuration Word (RCW):");
	for (i = 0; i < ARRAY_SIZE(gur->rcwsr); i++) {
		u32 rcw = in_be32(&gur->rcwsr[i]);

		if ((i % 4) == 0)
			printf("\n       %08x:", i * 4);
		printf(" %08x", rcw);
	}
	puts("\n");

	return 0;
}
#endif

#ifdef CONFIG_FSL_ESDHC
int cpu_mmc_init(bd_t *bis)
{
	return fsl_esdhc_mmc_init(bis);
}
#endif

int cpu_eth_init(bd_t *bis)
{
#ifdef CONFIG_TSEC_ENET
	tsec_standard_init(bis);
#endif

	return 0;
}

int arch_cpu_init(void)
{
	void *epu_base = (void *)(CONFIG_SYS_DCSRBAR + EPU_BLOCK_OFFSET);
	void *rcpm2_base =
		(void *)(CONFIG_SYS_DCSRBAR + DCSR_RCPM2_BLOCK_OFFSET);
	struct ccsr_scfg *scfg = (void *)CONFIG_SYS_FSL_SCFG_ADDR;
	u32 state;

	/*
	 * The RCPM FSM state may not be reset after power-on.
	 * So, reset them.
	 */
	state = in_be32(rcpm2_base + DCSR_RCPM2_CPMFSMSR0) &
		CPMFSMSR_FSM_STATE_MASK;
	if (state != 0) {
		out_be32(rcpm2_base + DCSR_RCPM2_CPMFSMCR0, 0x80);
		out_be32(rcpm2_base + DCSR_RCPM2_CPMFSMCR0, 0x0);
	}

	state = in_be32(rcpm2_base + DCSR_RCPM2_CPMFSMSR1) &
		CPMFSMSR_FSM_STATE_MASK;
	if (state != 0) {
		out_be32(rcpm2_base + DCSR_RCPM2_CPMFSMCR1, 0x80);
		out_be32(rcpm2_base + DCSR_RCPM2_CPMFSMCR1, 0x0);
	}

	/*
	 * After wakeup from deep sleep, Clear EPU registers
	 * as early as possible to prevent from possible issue.
	 * It's also safe to clear at normal boot.
	 */
	fsl_epu_clean(epu_base);

	setbits_be32(&scfg->snpcnfgcr, SCFG_SNPCNFGCR_SEC_RD_WR);

	return 0;
}

#ifdef CONFIG_ARMV7_NONSEC
/* Set the address at which the secondary core starts from.*/
void smp_set_core_boot_addr(unsigned long addr, int corenr)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);

	out_be32(&gur->scratchrw[0], addr);
}

/* Release the secondary core from holdoff state and kick it */
void smp_kick_all_cpus(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);

	out_be32(&gur->brrl, 0x2);

	/*
	 * LS1 STANDBYWFE is not captured outside the ARM module in the soc.
	 * So add a delay to wait bootrom execute WFE.
	 */
	udelay(1);

	asm volatile("sev");
}
#endif

void reset_cpu(ulong addr)
{
	struct watchdog_regs *wdog = (struct watchdog_regs *)WDOG1_BASE_ADDR;

	clrbits_be16(&wdog->wcr, WCR_SRS);

	while (1) {
		/*
		 * Let the watchdog trigger
		 */
	}
}

void arch_preboot_os(void)
{
	unsigned long ctrl;

	/* Disable PL1 Physical Timer */
	asm("mrc p15, 0, %0, c14, c2, 1" : "=r" (ctrl));
	ctrl &= ~ARCH_TIMER_CTRL_ENABLE;
	asm("mcr p15, 0, %0, c14, c2, 1" : : "r" (ctrl));
}
