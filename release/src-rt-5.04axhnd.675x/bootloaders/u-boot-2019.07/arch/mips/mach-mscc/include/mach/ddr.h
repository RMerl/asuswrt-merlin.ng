/* SPDX-License-Identifier: (GPL-2.0+ OR MIT) */
/*
 * Copyright (c) 2018 Microsemi Corporation
 */

#ifndef __ASM_MACH_DDR_H
#define __ASM_MACH_DDR_H

#include <asm/cacheops.h>
#include <asm/io.h>
#include <asm/reboot.h>
#include <mach/common.h>

#define MIPS_VCOREIII_MEMORY_DDR3
#define MIPS_VCOREIII_DDR_SIZE CONFIG_SYS_SDRAM_SIZE

#if defined(CONFIG_DDRTYPE_H5TQ1G63BFA)	/* Serval1 Refboard */

/* Hynix H5TQ1G63BFA (1Gbit DDR3, x16) @ 3.20ns */
#define VC3_MPAR_bank_addr_cnt    3
#define VC3_MPAR_row_addr_cnt     13
#define VC3_MPAR_col_addr_cnt     10
#define VC3_MPAR_tREFI            2437
#define VC3_MPAR_tRAS_min         12
#define VC3_MPAR_CL               6
#define VC3_MPAR_tWTR             4
#define VC3_MPAR_tRC              16
#define VC3_MPAR_tFAW             16
#define VC3_MPAR_tRP              5
#define VC3_MPAR_tRRD             4
#define VC3_MPAR_tRCD             5
#define VC3_MPAR_tMRD             4
#define VC3_MPAR_tRFC             35
#define VC3_MPAR_CWL              5
#define VC3_MPAR_tXPR             38
#define VC3_MPAR_tMOD             12
#define VC3_MPAR_tDLLK            512
#define VC3_MPAR_tWR              5

#elif defined(CONFIG_DDRTYPE_MT41J128M16HA)	/* Validation board */

/* Micron MT41J128M16HA-15E:D (2Gbit DDR3, x16) @ 3.20ns */
#define VC3_MPAR_bank_addr_cnt    3
#define VC3_MPAR_row_addr_cnt     14
#define VC3_MPAR_col_addr_cnt     10
#define VC3_MPAR_tREFI            2437
#define VC3_MPAR_tRAS_min         12
#define VC3_MPAR_CL               5
#define VC3_MPAR_tWTR             4
#define VC3_MPAR_tRC              16
#define VC3_MPAR_tFAW             16
#define VC3_MPAR_tRP              5
#define VC3_MPAR_tRRD             4
#define VC3_MPAR_tRCD             5
#define VC3_MPAR_tMRD             4
#define VC3_MPAR_tRFC             50
#define VC3_MPAR_CWL              5
#define VC3_MPAR_tXPR             54
#define VC3_MPAR_tMOD             12
#define VC3_MPAR_tDLLK            512
#define VC3_MPAR_tWR              5

#elif defined(CONFIG_DDRTYPE_MT41K256M16)	/* JR2 Validation board */

/* Micron MT41K256M16 (4Gbit, DDR3L-800, 256Mbitx16) @ 3.20ns */
#define VC3_MPAR_bank_addr_cnt    3
#define VC3_MPAR_row_addr_cnt     15
#define VC3_MPAR_col_addr_cnt     10
#define VC3_MPAR_tREFI            2437
#define VC3_MPAR_tRAS_min         12
#define VC3_MPAR_CL               5
#define VC3_MPAR_tWTR             4
#define VC3_MPAR_tRC              16
#define VC3_MPAR_tFAW             16
#define VC3_MPAR_tRP              5
#define VC3_MPAR_tRRD             4
#define VC3_MPAR_tRCD             5
#define VC3_MPAR_tMRD             4
#define VC3_MPAR_tRFC             82
#define VC3_MPAR_CWL              5
#define VC3_MPAR_tXPR             85
#define VC3_MPAR_tMOD             12
#define VC3_MPAR_tDLLK            512
#define VC3_MPAR_tWR              5

#elif defined(CONFIG_DDRTYPE_H5TQ4G63MFR)	/* JR2 Reference board */

/* Hynix H5TQ4G63MFR-PBC (4Gbit, DDR3-800, 256Mbitx16) - 2kb pages @ 3.20ns */
#define VC3_MPAR_bank_addr_cnt    3
#define VC3_MPAR_row_addr_cnt     15
#define VC3_MPAR_col_addr_cnt     10
#define VC3_MPAR_tREFI            2437
#define VC3_MPAR_tRAS_min         12
#define VC3_MPAR_CL               6
#define VC3_MPAR_tWTR             4
#define VC3_MPAR_tRC              17
#define VC3_MPAR_tFAW             16
#define VC3_MPAR_tRP              5
#define VC3_MPAR_tRRD             4
#define VC3_MPAR_tRCD             5
#define VC3_MPAR_tMRD             4
#define VC3_MPAR_tRFC             82
#define VC3_MPAR_CWL              5
#define VC3_MPAR_tXPR             85
#define VC3_MPAR_tMOD             12
#define VC3_MPAR_tDLLK            512
#define VC3_MPAR_tWR              5

#elif defined(CONFIG_DDRTYPE_MT41K128M16JT)

/* Micron Micron MT41K128M16JT-125 (2Gbit DDR3L, 128Mbitx16) @ 3.20ns */
#define VC3_MPAR_bank_addr_cnt    3
#define VC3_MPAR_row_addr_cnt     14
#define VC3_MPAR_col_addr_cnt     10
#define VC3_MPAR_tREFI            2437
#define VC3_MPAR_tRAS_min         12
#define VC3_MPAR_CL               6
#define VC3_MPAR_tWTR             4
#define VC3_MPAR_tRC              16
#define VC3_MPAR_tFAW             16
#define VC3_MPAR_tRP              5
#define VC3_MPAR_tRRD             4
#define VC3_MPAR_tRCD             5
#define VC3_MPAR_tMRD             4
#define VC3_MPAR_tRFC             82
#define VC3_MPAR_CWL              5
#define VC3_MPAR_tXPR             85
#define VC3_MPAR_tMOD             12
#define VC3_MPAR_tDLLK            512
#define VC3_MPAR_tWR              5

#elif defined(CONFIG_DDRTYPE_MT47H128M8HQ)	/* Luton10/26 Refboards */

/* Micron 1Gb MT47H128M8-3 16Meg x 8 x 8 banks, DDR-533@CL4 @ 4.80ns */
#define VC3_MPAR_bank_addr_cnt    3
#define VC3_MPAR_row_addr_cnt     14
#define VC3_MPAR_col_addr_cnt     10
#define VC3_MPAR_tREFI            1625
#define VC3_MPAR_tRAS_min         9
#define VC3_MPAR_CL               4
#define VC3_MPAR_tWTR             2
#define VC3_MPAR_tRC              12
#define VC3_MPAR_tFAW             8
#define VC3_MPAR_tRP              4
#define VC3_MPAR_tRRD             2
#define VC3_MPAR_tRCD             4

#define VC3_MPAR_tRPA             4
#define VC3_MPAR_tRP              4

#define VC3_MPAR_tMRD             2
#define VC3_MPAR_tRFC             27

#define VC3_MPAR__400_ns_dly      84

#define VC3_MPAR_tWR              4
#undef MIPS_VCOREIII_MEMORY_DDR3
#else

#error Unknown DDR system configuration - please add!

#endif

#if defined(CONFIG_SOC_OCELOT) || defined(CONFIG_SOC_JR2) || \
	defined(CONFIG_SOC_SERVALT) || defined(CONFIG_SOC_SERVAL)
#define MIPS_VCOREIII_MEMORY_16BIT 1
#endif

#define MIPS_VCOREIII_MEMORY_SSTL_ODT 7
#define MIPS_VCOREIII_MEMORY_SSTL_DRIVE 7
#define VCOREIII_DDR_DQS_MODE_CALIBRATE

#ifdef MIPS_VCOREIII_MEMORY_16BIT
#define VC3_MPAR_16BIT       1
#else
#define VC3_MPAR_16BIT       0
#endif

#ifdef MIPS_VCOREIII_MEMORY_DDR3
#define VC3_MPAR_DDR3_MODE    1	/* DDR3 */
#define VC3_MPAR_BURST_LENGTH 8	/* Always 8 (1) for DDR3 */
#ifdef MIPS_VCOREIII_MEMORY_16BIT
#define VC3_MPAR_BURST_SIZE   1	/* Always 1 for DDR3/16bit */
#else
#define VC3_MPAR_BURST_SIZE   0
#endif
#else
#define VC3_MPAR_DDR3_MODE    0	/* DDR2 */
#ifdef MIPS_VCOREIII_MEMORY_16BIT
#define VC3_MPAR_BURST_LENGTH 4	/* in DDR2 16-bit mode, use burstlen 4 */
#else
#define VC3_MPAR_BURST_LENGTH 8	/* For 8-bit IF we must run burst-8 */
#endif
#define VC3_MPAR_BURST_SIZE   0	/* Always 0 for DDR2 */
#endif

#define VC3_MPAR_RL VC3_MPAR_CL
#if !defined(MIPS_VCOREIII_MEMORY_DDR3)
#define VC3_MPAR_WL (VC3_MPAR_RL - 1)
#define VC3_MPAR_MD VC3_MPAR_tMRD
#define VC3_MPAR_ID VC3_MPAR__400_ns_dly
#define VC3_MPAR_SD VC3_MPAR_tXSRD
#define VC3_MPAR_OW (VC3_MPAR_WL - 2)
#define VC3_MPAR_OR (VC3_MPAR_WL - 3)
#define VC3_MPAR_RP (VC3_MPAR_bank_addr_cnt < 3 ? VC3_MPAR_tRP : VC3_MPAR_tRPA)
#define VC3_MPAR_FAW (VC3_MPAR_bank_addr_cnt < 3 ? 1 : VC3_MPAR_tFAW)
#define VC3_MPAR_BL (VC3_MPAR_BURST_LENGTH == 4 ? 2 : 4)
#define MSCC_MEMPARM_MR0 \
	(VC3_MPAR_BURST_LENGTH == 8 ? 3 : 2) | (VC3_MPAR_CL << 4) | \
	((VC3_MPAR_tWR - 1) << 9)
/* DLL-on, Full-OD, AL=0, RTT=off, nDQS-on, RDQS-off, out-en */
#define MSCC_MEMPARM_MR1 0x382
#define MSCC_MEMPARM_MR2 0
#define MSCC_MEMPARM_MR3 0
#else
#define VC3_MPAR_WL VC3_MPAR_CWL
#define VC3_MPAR_MD VC3_MPAR_tMOD
#define VC3_MPAR_ID VC3_MPAR_tXPR
#define VC3_MPAR_SD VC3_MPAR_tDLLK
#define VC3_MPAR_OW 2
#define VC3_MPAR_OR 2
#define VC3_MPAR_RP VC3_MPAR_tRP
#define VC3_MPAR_FAW VC3_MPAR_tFAW
#define VC3_MPAR_BL 4
#define MSCC_MEMPARM_MR0 ((VC3_MPAR_RL - 4) << 4) | ((VC3_MPAR_tWR - 4) << 9)
/* ODT_RTT: “0x0040” for 120ohm, and “0x0004” for 60ohm. */
#define MSCC_MEMPARM_MR1 0x0040
#define MSCC_MEMPARM_MR2 ((VC3_MPAR_WL - 5) << 3)
#define MSCC_MEMPARM_MR3 0
#endif				/* MIPS_VCOREIII_MEMORY_DDR3 */

#define MSCC_MEMPARM_MEMCFG                                             \
	((MIPS_VCOREIII_DDR_SIZE > SZ_512M) ?				\
	 ICPU_MEMCTRL_CFG_DDR_512MBYTE_PLUS : 0) |			\
	(VC3_MPAR_16BIT ? ICPU_MEMCTRL_CFG_DDR_WIDTH : 0) |		\
	(VC3_MPAR_DDR3_MODE ? ICPU_MEMCTRL_CFG_DDR_MODE : 0) |		\
	(VC3_MPAR_BURST_SIZE ? ICPU_MEMCTRL_CFG_BURST_SIZE : 0) |	\
	(VC3_MPAR_BURST_LENGTH == 8 ? ICPU_MEMCTRL_CFG_BURST_LEN : 0) | \
	(VC3_MPAR_bank_addr_cnt == 3 ? ICPU_MEMCTRL_CFG_BANK_CNT : 0) | \
	ICPU_MEMCTRL_CFG_MSB_ROW_ADDR(VC3_MPAR_row_addr_cnt - 1) |	\
	ICPU_MEMCTRL_CFG_MSB_COL_ADDR(VC3_MPAR_col_addr_cnt - 1)

#if defined(CONFIG_SOC_OCELOT) || defined(CONFIG_SOC_JR2) || \
	defined(CONFIG_SOC_SERVALT) || defined(CONFIG_SOC_SERVAL)
#define MSCC_MEMPARM_PERIOD					\
	ICPU_MEMCTRL_REF_PERIOD_MAX_PEND_REF(8) |		\
	ICPU_MEMCTRL_REF_PERIOD_REF_PERIOD(VC3_MPAR_tREFI)

#define MSCC_MEMPARM_TIMING0                                            \
	ICPU_MEMCTRL_TIMING0_RD_TO_WR_DLY(VC3_MPAR_RL + VC3_MPAR_BL + 1 - \
					  VC3_MPAR_WL) |		\
	ICPU_MEMCTRL_TIMING0_WR_CS_CHANGE_DLY(VC3_MPAR_BL - 1) |	\
	ICPU_MEMCTRL_TIMING0_RD_CS_CHANGE_DLY(VC3_MPAR_BL) |		\
	ICPU_MEMCTRL_TIMING0_RAS_TO_PRECH_DLY(VC3_MPAR_tRAS_min - 1) |	\
	ICPU_MEMCTRL_TIMING0_WR_TO_PRECH_DLY(VC3_MPAR_WL +		\
					     VC3_MPAR_BL +		\
					     VC3_MPAR_tWR - 1) |	\
	ICPU_MEMCTRL_TIMING0_RD_TO_PRECH_DLY(VC3_MPAR_BL - 1) |		\
		ICPU_MEMCTRL_TIMING0_WR_DATA_XFR_DLY(VC3_MPAR_WL - 1) |	\
	ICPU_MEMCTRL_TIMING0_RD_DATA_XFR_DLY(VC3_MPAR_RL - 3)

#define MSCC_MEMPARM_TIMING1                                            \
	ICPU_MEMCTRL_TIMING1_RAS_TO_RAS_SAME_BANK_DLY(VC3_MPAR_tRC - 1) | \
	ICPU_MEMCTRL_TIMING1_BANK8_FAW_DLY(VC3_MPAR_FAW - 1) |		\
	ICPU_MEMCTRL_TIMING1_PRECH_TO_RAS_DLY(VC3_MPAR_RP - 1) |	\
	ICPU_MEMCTRL_TIMING1_RAS_TO_RAS_DLY(VC3_MPAR_tRRD - 1) |	\
	ICPU_MEMCTRL_TIMING1_RAS_TO_CAS_DLY(VC3_MPAR_tRCD - 1) |	\
	ICPU_MEMCTRL_TIMING1_WR_TO_RD_DLY(VC3_MPAR_WL +			\
					  VC3_MPAR_BL +			\
					  VC3_MPAR_tWTR - 1)

#define MSCC_MEMPARM_TIMING2					\
	ICPU_MEMCTRL_TIMING2_PRECH_ALL_DLY(VC3_MPAR_RP - 1) |	\
	ICPU_MEMCTRL_TIMING2_MDSET_DLY(VC3_MPAR_MD - 1) |		\
	ICPU_MEMCTRL_TIMING2_REF_DLY(VC3_MPAR_tRFC - 1) |		\
	ICPU_MEMCTRL_TIMING2_INIT_DLY(VC3_MPAR_ID - 1)

#define MSCC_MEMPARM_TIMING3						\
	ICPU_MEMCTRL_TIMING3_WR_TO_RD_CS_CHANGE_DLY(VC3_MPAR_WL +	\
						    VC3_MPAR_tWTR - 1) |\
	ICPU_MEMCTRL_TIMING3_ODT_RD_DLY(VC3_MPAR_OR - 1) |		\
	ICPU_MEMCTRL_TIMING3_ODT_WR_DLY(VC3_MPAR_OW - 1) |		\
	ICPU_MEMCTRL_TIMING3_LOCAL_ODT_RD_DLY(VC3_MPAR_RL - 3)

#else
#define MSCC_MEMPARM_PERIOD					\
	ICPU_MEMCTRL_REF_PERIOD_MAX_PEND_REF(1) |		\
	ICPU_MEMCTRL_REF_PERIOD_REF_PERIOD(VC3_MPAR_tREFI)

#define MSCC_MEMPARM_TIMING0                                            \
	ICPU_MEMCTRL_TIMING0_RAS_TO_PRECH_DLY(VC3_MPAR_tRAS_min - 1) |	\
	ICPU_MEMCTRL_TIMING0_WR_TO_PRECH_DLY(VC3_MPAR_CL +		\
					     (VC3_MPAR_BURST_LENGTH == 8 ? 2 : 0) + \
					     VC3_MPAR_tWR) |		\
	ICPU_MEMCTRL_TIMING0_RD_TO_PRECH_DLY(VC3_MPAR_BURST_LENGTH == 8 ? 3 : 1) | \
	ICPU_MEMCTRL_TIMING0_WR_DATA_XFR_DLY(VC3_MPAR_CL - 3) |		\
	ICPU_MEMCTRL_TIMING0_RD_DATA_XFR_DLY(VC3_MPAR_CL - 3)

#define MSCC_MEMPARM_TIMING1                                            \
	ICPU_MEMCTRL_TIMING1_RAS_TO_RAS_SAME_BANK_DLY(VC3_MPAR_tRC - 1) | \
	ICPU_MEMCTRL_TIMING1_BANK8_FAW_DLY(VC3_MPAR_tFAW - 1) |		\
	ICPU_MEMCTRL_TIMING1_PRECH_TO_RAS_DLY(VC3_MPAR_tRP - 1) |	\
	ICPU_MEMCTRL_TIMING1_RAS_TO_RAS_DLY(VC3_MPAR_tRRD - 1) |	\
	ICPU_MEMCTRL_TIMING1_RAS_TO_CAS_DLY(VC3_MPAR_tRCD - 1) |	\
	ICPU_MEMCTRL_TIMING1_WR_TO_RD_DLY(VC3_MPAR_CL +			\
					  (VC3_MPAR_BURST_LENGTH == 8 ? 2 : 0) + \
					  VC3_MPAR_tWTR)
#define MSCC_MEMPARM_TIMING2                                            \
	ICPU_MEMCTRL_TIMING2_PRECH_ALL_DLY(VC3_MPAR_tRPA - 1) |		\
	ICPU_MEMCTRL_TIMING2_MDSET_DLY(VC3_MPAR_tMRD - 1) |		\
	ICPU_MEMCTRL_TIMING2_REF_DLY(VC3_MPAR_tRFC - 1) |		\
	ICPU_MEMCTRL_TIMING2_FOUR_HUNDRED_NS_DLY(VC3_MPAR__400_ns_dly)

#define MSCC_MEMPARM_TIMING3						\
	ICPU_MEMCTRL_TIMING3_WR_TO_RD_CS_CHANGE_DLY(VC3_MPAR_CL - 1) |	\
	ICPU_MEMCTRL_TIMING3_ODT_WR_DLY(VC3_MPAR_CL - 1) |		\
	ICPU_MEMCTRL_TIMING3_LOCAL_ODT_RD_DLY(VC3_MPAR_CL - 1)

#endif

enum {
	DDR_TRAIN_OK,
	DDR_TRAIN_CONTINUE,
	DDR_TRAIN_ERROR,
};

/*
 * We actually have very few 'pause' possibilities apart from
 * these assembly nops (at this very early stage).
 */
#define PAUSE() asm volatile("nop; nop; nop; nop; nop; nop; nop; nop")

/* NB: Assumes inlining as no stack is available! */
static inline void set_dly(u32 bytelane, u32 dly)
{
	register u32 r = readl(BASE_CFG + ICPU_MEMCTRL_DQS_DLY(bytelane));

	r &= ~ICPU_MEMCTRL_DQS_DLY_DQS_DLY_M;
	r |= ICPU_MEMCTRL_DQS_DLY_DQS_DLY(dly);
	writel(r, BASE_CFG + ICPU_MEMCTRL_DQS_DLY(bytelane));
}

static inline bool incr_dly(u32 bytelane)
{
	register u32 r = readl(BASE_CFG + ICPU_MEMCTRL_DQS_DLY(bytelane));

	if (ICPU_MEMCTRL_DQS_DLY_DQS_DLY(r) < 31) {
		writel(r + 1, BASE_CFG + ICPU_MEMCTRL_DQS_DLY(bytelane));
		return true;
	}

	return false;
}

static inline bool adjust_dly(int adjust)
{
	register u32 r = readl(BASE_CFG + ICPU_MEMCTRL_DQS_DLY(0));

	if (ICPU_MEMCTRL_DQS_DLY_DQS_DLY(r) < 31) {
		writel(r + adjust, BASE_CFG + ICPU_MEMCTRL_DQS_DLY(0));
		return true;
	}

	return false;
}

/* NB: Assumes inlining as no stack is available! */
static inline void center_dly(u32 bytelane, u32 start)
{
	register u32 r = readl(BASE_CFG + ICPU_MEMCTRL_DQS_DLY(bytelane)) - start;

	writel(start + (r >> 1), BASE_CFG + ICPU_MEMCTRL_DQS_DLY(bytelane));
}

static inline void memphy_soft_reset(void)
{
	setbits_le32(BASE_CFG + ICPU_MEMPHY_CFG, ICPU_MEMPHY_CFG_PHY_FIFO_RST);
	PAUSE();
	clrbits_le32(BASE_CFG + ICPU_MEMPHY_CFG, ICPU_MEMPHY_CFG_PHY_FIFO_RST);
	PAUSE();
}

#if defined(CONFIG_SOC_OCELOT) || defined(CONFIG_SOC_JR2) || \
	defined(CONFIG_SOC_SERVALT) || defined(CONFIG_SOC_SERVAL)
static u8 training_data[] = { 0xfe, 0x11, 0x33, 0x55, 0x77, 0x99, 0xbb, 0xdd };

static inline void sleep_100ns(u32 val)
{
	/* Set the timer tick generator to 100 ns */
	writel(VCOREIII_TIMER_DIVIDER - 1, BASE_CFG + ICPU_TIMER_TICK_DIV);

	/* Set the timer value */
	writel(val, BASE_CFG + ICPU_TIMER_VALUE(0));

	/* Enable timer 0 for one-shot */
	writel(ICPU_TIMER_CTRL_ONE_SHOT_ENA | ICPU_TIMER_CTRL_TIMER_ENA,
	       BASE_CFG + ICPU_TIMER_CTRL(0));

	/* Wait for timer 0 to reach 0 */
	while (readl(BASE_CFG + ICPU_TIMER_VALUE(0)) != 0)
		;
}

#if defined(CONFIG_SOC_OCELOT) || defined(CONFIG_SOC_SERVAL)
/*
 * DDR memory sanity checking failed, tally and do hard reset
 *
 * NB: Assumes inlining as no stack is available!
 */
static inline void hal_vcoreiii_ddr_failed(void)
{
	register u32 reset;

#if defined(CONFIG_SOC_OCELOT)
	writel(readl(BASE_CFG + ICPU_GPR(6)) + 1, BASE_CFG + ICPU_GPR(6));

	clrbits_le32(BASE_DEVCPU_GCB + PERF_GPIO_OE, BIT(19));
#endif

	/* We have to execute the reset function from cache. Indeed,
	 * the reboot workaround in _machine_restart() will change the
	 * SPI NOR into SW bitbang.
	 *
	 * This will render the CPU unable to execute directly from
	 * the NOR, which is why the reset instructions are prefetched
	 * into the I-cache.
	 *
	 * When failing the DDR initialization we are executing from
	 * NOR.
	 *
	 * The last instruction in _machine_restart() will reset the
	 * MIPS CPU (and the cache), and the CPU will start executing
	 * from the reset vector.
	 */
	reset = KSEG0ADDR(_machine_restart);
	icache_lock((void *)reset, 128);
	asm volatile ("jr %0"::"r" (reset));

	panic("DDR init failed\n");
}
#else				/* JR2 || ServalT */
static inline void hal_vcoreiii_ddr_failed(void)
{
	writel(0, BASE_CFG + ICPU_RESET);
	writel(PERF_SOFT_RST_SOFT_CHIP_RST, BASE_CFG + PERF_SOFT_RST);

	panic("DDR init failed\n");
}
#endif

#if defined(CONFIG_SOC_OCELOT)
static inline void hal_vcoreiii_ddr_reset_assert(void)
{
	/* DDR has reset pin on GPIO 19 toggle Low-High to release */
	setbits_le32(BASE_DEVCPU_GCB + PERF_GPIO_OE, BIT(19));
	writel(BIT(19), BASE_DEVCPU_GCB + PERF_GPIO_OUT_CLR);
	sleep_100ns(10000);
}

static inline void hal_vcoreiii_ddr_reset_release(void)
{
	/* DDR has reset pin on GPIO 19 toggle Low-High to release */
	setbits_le32(BASE_DEVCPU_GCB + PERF_GPIO_OE, BIT(19));
	writel(BIT(19), BASE_DEVCPU_GCB + PERF_GPIO_OUT_SET);
	sleep_100ns(10000);
}

#else				/* JR2 || ServalT || Serval */
static inline void hal_vcoreiii_ddr_reset_assert(void)
{
	/* Ensure the memory controller physical iface is forced reset */
	writel(readl(BASE_CFG + ICPU_MEMPHY_CFG) |
	       ICPU_MEMPHY_CFG_PHY_RST, BASE_CFG + ICPU_MEMPHY_CFG);

	/* Ensure the memory controller is forced reset */
	writel(readl(BASE_CFG + ICPU_RESET) |
	       ICPU_RESET_MEM_RST_FORCE, BASE_CFG + ICPU_RESET);
}
#endif				/* JR2 || ServalT || Serval */

/*
 * DDR memory sanity checking done, possibly enable ECC.
 *
 * NB: Assumes inlining as no stack is available!
 */
static inline void hal_vcoreiii_ddr_verified(void)
{
#ifdef MIPS_VCOREIII_MEMORY_ECC
	/* Finally, enable ECC */
	register u32 val = readl(BASE_CFG + ICPU_MEMCTRL_CFG);

	val |= ICPU_MEMCTRL_CFG_DDR_ECC_ERR_ENA;
	val &= ~ICPU_MEMCTRL_CFG_BURST_SIZE;

	writel(val, BASE_CFG + ICPU_MEMCTRL_CFG);
#endif

	/* Reset Status register - sticky bits */
	writel(readl(BASE_CFG + ICPU_MEMCTRL_STAT), BASE_CFG + ICPU_MEMCTRL_STAT);
}

/* NB: Assumes inlining as no stack is available! */
static inline int look_for(u32 bytelane)
{
	register u32 i;

	/* Reset FIFO in case any previous access failed */
	for (i = 0; i < sizeof(training_data); i++) {
		register u32 byte;

		memphy_soft_reset();
		/* Reset sticky bits */
		writel(readl(BASE_CFG + ICPU_MEMCTRL_STAT),
		       BASE_CFG + ICPU_MEMCTRL_STAT);
		/* Read data */
		byte = __raw_readb((void __iomem *)MSCC_DDR_TO + bytelane +
				   (i * 4));

		/*
		 * Prevent the compiler reordering the instruction so
		 * the read of RAM happens after the check of the
		 * errors.
		 */
		rmb();
		if (readl(BASE_CFG + ICPU_MEMCTRL_STAT) &
		    (ICPU_MEMCTRL_STAT_RDATA_MASKED |
		     ICPU_MEMCTRL_STAT_RDATA_DUMMY)) {
			/* Noise on the line */
			goto read_error;
		}
		/* If mismatch, increment DQS - if possible */
		if (byte != training_data[i]) {
 read_error:
			if (!incr_dly(bytelane))
				return DDR_TRAIN_ERROR;
			return DDR_TRAIN_CONTINUE;
		}
	}
	return DDR_TRAIN_OK;
}

/* NB: Assumes inlining as no stack is available! */
static inline int look_past(u32 bytelane)
{
	register u32 i;

	/* Reset FIFO in case any previous access failed */
	for (i = 0; i < sizeof(training_data); i++) {
		register u32 byte;

		memphy_soft_reset();
		/* Ack sticky bits */
		writel(readl(BASE_CFG + ICPU_MEMCTRL_STAT),
		       BASE_CFG + ICPU_MEMCTRL_STAT);
		byte = __raw_readb((void __iomem *)MSCC_DDR_TO + bytelane +
				   (i * 4));
		/*
		 * Prevent the compiler reordering the instruction so
		 * the read of RAM happens after the check of the
		 * errors.
		 */
		rmb();
		if (readl(BASE_CFG + ICPU_MEMCTRL_STAT) &
		    (ICPU_MEMCTRL_STAT_RDATA_MASKED |
		     ICPU_MEMCTRL_STAT_RDATA_DUMMY)) {
			/* Noise on the line */
			goto read_error;
		}
		/* Bail out when we see first mismatch */
		if (byte != training_data[i]) {
 read_error:
			return DDR_TRAIN_OK;
		}
	}
	/* All data compares OK, increase DQS and retry */
	if (!incr_dly(bytelane))
		return DDR_TRAIN_ERROR;

	return DDR_TRAIN_CONTINUE;
}

static inline int hal_vcoreiii_train_bytelane(u32 bytelane)
{
	register int res;
	register u32 dqs_s;

	set_dly(bytelane, 0);	/* Start training at DQS=0 */
	while ((res = look_for(bytelane)) == DDR_TRAIN_CONTINUE)
		;
	if (res != DDR_TRAIN_OK)
		return res;

	dqs_s = readl(BASE_CFG + ICPU_MEMCTRL_DQS_DLY(bytelane));
	while ((res = look_past(bytelane)) == DDR_TRAIN_CONTINUE)
		;
	if (res != DDR_TRAIN_OK)
		return res;
	/* Reset FIFO - for good measure */
	memphy_soft_reset();
	/* Adjust to center [dqs_s;cur] */
	center_dly(bytelane, dqs_s);
	return DDR_TRAIN_OK;
}

/* This algorithm is converted from the TCL training algorithm used
 * during silicon simulation.
 * NB: Assumes inlining as no stack is available!
 */
static inline int hal_vcoreiii_init_dqs(void)
{
#define MAX_DQS 32
	register u32 i, j;

	for (i = 0; i < MAX_DQS; i++) {
		set_dly(0, i);	/* Byte-lane 0 */
		for (j = 0; j < MAX_DQS; j++) {
			__maybe_unused register u32  byte;

			set_dly(1, j);	/* Byte-lane 1 */
			/* Reset FIFO in case any previous access failed */
			memphy_soft_reset();
			writel(readl(BASE_CFG + ICPU_MEMCTRL_STAT),
			       BASE_CFG + ICPU_MEMCTRL_STAT);
			byte = __raw_readb((void __iomem *)MSCC_DDR_TO);
			byte = __raw_readb((void __iomem *)(MSCC_DDR_TO + 1));
			if (!(readl(BASE_CFG + ICPU_MEMCTRL_STAT) &
			    (ICPU_MEMCTRL_STAT_RDATA_MASKED |
			     ICPU_MEMCTRL_STAT_RDATA_DUMMY)))
				return 0;
		}
	}
	return -1;
}

static inline int dram_check(void)
{
	register u32 i;

	for (i = 0; i < 8; i++) {
		__raw_writel(~i, (void __iomem *)(MSCC_DDR_TO + (i * 4)));
		if (__raw_readl((void __iomem *)(MSCC_DDR_TO + (i * 4))) != ~i)
			return 1;
	}
	return 0;
}
#else				/* Luton */

static inline void sleep_100ns(u32 val)
{
}

static inline void hal_vcoreiii_ddr_reset_assert(void)
{
	setbits_le32(BASE_CFG + ICPU_MEMPHY_CFG, ICPU_MEMPHY_CFG_PHY_RST);
	setbits_le32(BASE_CFG + ICPU_RESET, ICPU_RESET_MEM_RST_FORCE);
}

static inline void hal_vcoreiii_ddr_reset_release(void)
{
}

static inline void hal_vcoreiii_ddr_failed(void)
{
	register u32 memphy_cfg = readl(BASE_CFG + ICPU_MEMPHY_CFG);

	/* Do a fifo reset and start over */
	writel(memphy_cfg | ICPU_MEMPHY_CFG_PHY_FIFO_RST,
	       BASE_CFG + ICPU_MEMPHY_CFG);
	writel(memphy_cfg & ~ICPU_MEMPHY_CFG_PHY_FIFO_RST,
	       BASE_CFG + ICPU_MEMPHY_CFG);
	writel(memphy_cfg | ICPU_MEMPHY_CFG_PHY_FIFO_RST,
	       BASE_CFG + ICPU_MEMPHY_CFG);
}

static inline void hal_vcoreiii_ddr_verified(void)
{
}

static inline int look_for(u32 data)
{
	register u32 byte = __raw_readb((void __iomem *)MSCC_DDR_TO);

	if (data != byte) {
		if (!incr_dly(0))
			return DDR_TRAIN_ERROR;
		return DDR_TRAIN_CONTINUE;
	}

	return DDR_TRAIN_OK;
}

/* This algorithm is converted from the TCL training algorithm used
 * during silicon simulation.
 * NB: Assumes inlining as no stack is available!
 */
static inline int hal_vcoreiii_train_bytelane(u32 bytelane)
{
	register int res;

	set_dly(bytelane, 0);	/* Start training at DQS=0 */
	while ((res = look_for(0xff)) == DDR_TRAIN_CONTINUE)
		;
	if (res != DDR_TRAIN_OK)
		return res;

	set_dly(bytelane, 0);	/* Start training at DQS=0 */
	while ((res = look_for(0x00)) == DDR_TRAIN_CONTINUE)

		;

	if (res != DDR_TRAIN_OK)
		return res;

	adjust_dly(-3);

	return DDR_TRAIN_OK;
}

static inline int hal_vcoreiii_init_dqs(void)
{
	return 0;
}

static inline int dram_check(void)
{
	register u32 i;

	for (i = 0; i < 8; i++) {
		__raw_writel(~i, (void __iomem *)(MSCC_DDR_TO + (i * 4)));

		if (__raw_readl((void __iomem *)(MSCC_DDR_TO + (i * 4))) != ~i)
			return 1;
	}

	return 0;
}
#endif

/*
 * NB: Called *early* to init memory controller - assumes inlining as
 * no stack is available!
 */
static inline void hal_vcoreiii_init_memctl(void)
{
	/* Ensure DDR is in reset */
	hal_vcoreiii_ddr_reset_assert();

	/* Wait maybe not needed, but ... */
	PAUSE();

	/* Drop sys ctl memory controller forced reset */
	clrbits_le32(BASE_CFG + ICPU_RESET, ICPU_RESET_MEM_RST_FORCE);

	PAUSE();

	/* Drop Reset, enable SSTL */
	writel(ICPU_MEMPHY_CFG_PHY_SSTL_ENA, BASE_CFG + ICPU_MEMPHY_CFG);
	PAUSE();

	/* Start the automatic SSTL output and ODT drive-strength calibration */
	writel(ICPU_MEMPHY_ZCAL_ZCAL_PROG_ODT(MIPS_VCOREIII_MEMORY_SSTL_ODT) |
	       /* drive strength */
	       ICPU_MEMPHY_ZCAL_ZCAL_PROG(MIPS_VCOREIII_MEMORY_SSTL_DRIVE) |
	       /* Start calibration process */
	       ICPU_MEMPHY_ZCAL_ZCAL_ENA, BASE_CFG + ICPU_MEMPHY_ZCAL);

	/* Wait for ZCAL to clear */
	while (readl(BASE_CFG + ICPU_MEMPHY_ZCAL) & ICPU_MEMPHY_ZCAL_ZCAL_ENA)
		;
#if defined(CONFIG_SOC_OCELOT) || defined(CONFIG_SOC_JR2) || \
	defined(CONFIG_SOC_SERVALT)
	/* Check no ZCAL_ERR */
	if (readl(BASE_CFG + ICPU_MEMPHY_ZCAL_STAT)
	    & ICPU_MEMPHY_ZCAL_STAT_ZCAL_ERR)
		hal_vcoreiii_ddr_failed();
#endif
	/* Drive CL, CK, ODT */
	setbits_le32(BASE_CFG + ICPU_MEMPHY_CFG, ICPU_MEMPHY_CFG_PHY_ODT_OE |
		     ICPU_MEMPHY_CFG_PHY_CK_OE | ICPU_MEMPHY_CFG_PHY_CL_OE);

	/* Initialize memory controller */
	writel(MSCC_MEMPARM_MEMCFG, BASE_CFG + ICPU_MEMCTRL_CFG);
	writel(MSCC_MEMPARM_PERIOD, BASE_CFG + ICPU_MEMCTRL_REF_PERIOD);

#if defined(CONFIG_SOC_OCELOT) || defined(CONFIG_SOC_JR2) || \
	defined(CONFIG_SOC_SERVALT) || defined(CONFIG_SOC_SERVAL)
	writel(MSCC_MEMPARM_TIMING0, BASE_CFG + ICPU_MEMCTRL_TIMING0);
#else /* Luton */
	clrbits_le32(BASE_CFG + ICPU_MEMCTRL_TIMING0, ((1 << 20) - 1));
	setbits_le32(BASE_CFG + ICPU_MEMCTRL_TIMING0, MSCC_MEMPARM_TIMING0);
#endif

	writel(MSCC_MEMPARM_TIMING1, BASE_CFG + ICPU_MEMCTRL_TIMING1);
	writel(MSCC_MEMPARM_TIMING2, BASE_CFG + ICPU_MEMCTRL_TIMING2);
	writel(MSCC_MEMPARM_TIMING3, BASE_CFG + ICPU_MEMCTRL_TIMING3);
	writel(MSCC_MEMPARM_MR0, BASE_CFG + ICPU_MEMCTRL_MR0_VAL);
	writel(MSCC_MEMPARM_MR1, BASE_CFG + ICPU_MEMCTRL_MR1_VAL);
	writel(MSCC_MEMPARM_MR2, BASE_CFG + ICPU_MEMCTRL_MR2_VAL);
	writel(MSCC_MEMPARM_MR3, BASE_CFG + ICPU_MEMCTRL_MR3_VAL);

#if defined(CONFIG_SOC_OCELOT) || defined(CONFIG_SOC_SERVAL)
	/* Termination setup - enable ODT */
	writel(ICPU_MEMCTRL_TERMRES_CTRL_LOCAL_ODT_RD_ENA |
	       /* Assert ODT0 for any write */
	       ICPU_MEMCTRL_TERMRES_CTRL_ODT_WR_ENA(3),
	       BASE_CFG + ICPU_MEMCTRL_TERMRES_CTRL);

	/* Release Reset from DDR */
#if defined(CONFIG_SOC_OCELOT)
	hal_vcoreiii_ddr_reset_release();
#endif

	writel(readl(BASE_CFG + ICPU_GPR(7)) + 1, BASE_CFG + ICPU_GPR(7));
#elif defined(CONFIG_SOC_JR2) || defined(CONFIG_SOC_SERVALT)
	writel(ICPU_MEMCTRL_TERMRES_CTRL_ODT_WR_ENA(3),
	       BASE_CFG + ICPU_MEMCTRL_TERMRES_CTRL);
#else				/* Luton */
	/* Termination setup - disable ODT */
	writel(0, BASE_CFG + ICPU_MEMCTRL_TERMRES_CTRL);

#endif
}

static inline void hal_vcoreiii_wait_memctl(void)
{
	/* Now, rip it! */
	writel(ICPU_MEMCTRL_CTRL_INITIALIZE, BASE_CFG + ICPU_MEMCTRL_CTRL);

	while (!(readl(BASE_CFG + ICPU_MEMCTRL_STAT)
		 & ICPU_MEMCTRL_STAT_INIT_DONE))
		;

	/* Settle...? */
	sleep_100ns(10000);
#if defined(CONFIG_SOC_OCELOT) || defined(CONFIG_SOC_JR2) || \
	defined(CONFIG_SOC_SERVALT) || defined(CONFIG_SOC_SERVAL)
	/* Establish data contents in DDR RAM for training */

	__raw_writel(0xcacafefe, ((void __iomem *)MSCC_DDR_TO));
	__raw_writel(0x22221111, ((void __iomem *)MSCC_DDR_TO + 0x4));
	__raw_writel(0x44443333, ((void __iomem *)MSCC_DDR_TO + 0x8));
	__raw_writel(0x66665555, ((void __iomem *)MSCC_DDR_TO + 0xC));
	__raw_writel(0x88887777, ((void __iomem *)MSCC_DDR_TO + 0x10));
	__raw_writel(0xaaaa9999, ((void __iomem *)MSCC_DDR_TO + 0x14));
	__raw_writel(0xccccbbbb, ((void __iomem *)MSCC_DDR_TO + 0x18));
	__raw_writel(0xeeeedddd, ((void __iomem *)MSCC_DDR_TO + 0x1C));
#else
	__raw_writel(0xff, ((void __iomem *)MSCC_DDR_TO));
#endif
}
#endif				/* __ASM_MACH_DDR_H */
