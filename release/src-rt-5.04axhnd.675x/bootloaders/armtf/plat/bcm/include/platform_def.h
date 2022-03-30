/*
 * Copyright (c) 2015-2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_DEF_H__
#define __PLATFORM_DEF_H__

/* Following two defines needed to makes use of shared PMC driver code */
#define _CFE_
#define _ATF_

#include <arch.h>
#include <common_def.h>
#include <tbbr_img_def.h>
#include <bcm_map_part.h>
#include <bcm_mem_reserve.h>

#define PLAT_ARM_NS_IMAGE_BASE 0x0

#define PLATFORM_STACK_SIZE 0x1000

#define PLATFORM_MAX_CPUS_PER_CLUSTER	4
#define PLATFORM_CLUSTER_COUNT		1
#define PLATFORM_CLUSTER0_CORE_COUNT	PLATFORM_MAX_CPUS_PER_CLUSTER
#define PLATFORM_CLUSTER1_CORE_COUNT	0
#define PLATFORM_CORE_COUNT		(PLATFORM_CLUSTER0_CORE_COUNT + \
					 PLATFORM_CLUSTER1_CORE_COUNT)

#define BRCM_PRIMARY_CPU		0

#define PLAT_NUM_PWR_DOMAINS		(PLATFORM_CLUSTER_COUNT + \
					PLATFORM_CORE_COUNT)
#define PLAT_MAX_PWR_LVL		MPIDR_AFFLVL1

#define PLAT_MAX_RET_STATE		1
#define PLAT_MAX_OFF_STATE		2

/* Local power state for power domains in Run state. */
#define PLAT_LOCAL_STATE_RUN		0
/* Local power state for retention. Valid only for CPU power domains */
#define PLAT_LOCAL_STATE_RET		1
/*
 * Local power state for OFF/power-down. Valid for CPU and cluster power
 * domains.
 */
#define PLAT_LOCAL_STATE_OFF		2

/*
 * Macros used to parse state information from State-ID if it is using the
 * recommended encoding for State-ID.
 */
#define PLAT_LOCAL_PSTATE_WIDTH		4
#define PLAT_LOCAL_PSTATE_MASK		((1 << PLAT_LOCAL_PSTATE_WIDTH) - 1)

/*
 * Some data must be aligned on the biggest cache line size in the platform.
 * This is known only to the platform as it might have a combination of
 * integrated and external caches.
 */
#define CACHE_WRITEBACK_SHIFT		6
#define CACHE_WRITEBACK_GRANULE		(1 << CACHE_WRITEBACK_SHIFT)


/*
 * BL3-1 specific defines.
 *
 * Put BL3-1 at the top of the Trusted SRAM. BL31_BASE is calculated using the
 * current BL3-1 debug size plus a little space for growth.
 */
#define BL31_BASE			(CFG_ATF_AREA_ADDR)
#define BL31_LIMIT			(CFG_ATF_AREA_ADDR + CFG_ATF_AREA_SIZE)


#define PLAT_ARM_NS_IMAGE_OFFSET	(0x0)
#define PLAT_PHY_ADDR_SPACE_SIZE	(1ULL << 32)
#define PLAT_VIRT_ADDR_SPACE_SIZE	(1ULL << 32)

/*
 * BL3-2 specific defines.
 *
 */
#if defined(AARCH32)
#define BL32_BASE			BL31_BASE
#define BL32_SIZE			CFG_ATF_AREA_SIZE
#define BL32_LIMIT			BL31_LIMIT
#define BL322_BASE			(CFG_OPTEE_AREA_ADDR)
#else
#define BL32_BASE			(CFG_OPTEE_AREA_ADDR)
#define BL32_SIZE			(CFG_OPTEE_AREA_SIZE)
#define BL32_LIMIT			(CFG_OPTEE_AREA_ADDR + CFG_OPTEE_AREA_SIZE)
#endif

#define ADDR_SPACE_SIZE			(1ull << 32)
#define MAX_MMAP_REGIONS		10
#define MAX_XLAT_TABLES			16
#define MAX_IO_DEVICES			3
#define MAX_IO_HANDLES			4

#if defined BOOTLUT_PHYS_BASE
#undef BOOTLUT_BASE
#define BOOTLUT_BASE			BOOTLUT_PHYS_BASE
#else
#define BOOTLUT_BASE			0xffff0000
#endif

#if !defined(BOOTLUT_SIZE)
#define BOOTLUT_SIZE			0x1000
#endif

#if defined (BIUCFG_BASE)
#undef  BIUCFG_BASE
#define BIUCFG_BASE			(BIUCFG_PHYS_BASE + BIUCFG_OFFSET)
#elif defined (BIUCTRL_BASE)
#undef  BIUCTRL_BASE
#define BIUCTRL_BASE			(URB_PHYS_BASE + URB_BIUCTRL_OFFSET)
#define BIUCTRL_SIZE			URB_SIZE
#endif

#if defined (B15_CTRL_BASE)
#undef B15_CTRL_BASE
#define B15_CTRL_BASE B15_CTRL_PHYS_BASE
#define B15_CTRL_SIZE 0x3000
#endif

#undef TIMR_BASE
#if defined (PLATFORM_FLAVOR_63158) || defined (PLATFORM_FLAVOR_63178) || defined (PLATFORM_FLAVOR_47622) || defined (PLATFORM_FLAVOR_6878) || defined (PLATFORM_FLAVOR_63146) || defined (PLATFORM_FLAVOR_4912) || defined (PLATFORM_FLAVOR_6813) || defined (PLATFORM_FLAVOR_6756) || defined (PLATFORM_FLAVOR_6855)
#define UART0_BASE			0xff812000
#elif defined (PLATFORM_FLAVOR_6858) || defined (PLATFORM_FLAVOR_6856) || defined (PLATFORM_FLAVOR_4908) || defined (PLATFORM_FLAVOR_6846)
#define UART0_BASE			0xff800000
#elif defined (PLATFORM_FLAVOR_63138) || defined (PLATFORM_FLAVOR_63148)
#define TIMR_BASE	PERF_PHYS_BASE
#define TIMR_SIZE	0x10000
#define TIMR_OFFSET	0x80

#undef UART0_BASE
/* 63138 and 63148 do not support PL011 UART.
   UART is needed only for debug print.
   For now, just use JTAG to debug these platforms.
*/
#define UART0_BASE			0x0
#endif
#define UART0_SIZE			0x1000
#define UART0_CLK_IN_HZ			(50 * 1000 * 1000)


#undef  PMC_BASE
#if defined (PMC_OFFSET)
#define PMC_BASE			(PMC_PHYS_BASE + PMC_OFFSET)
#else
#define PMC_BASE			(PMC_PHYS_BASE)
#endif

#if defined(PROC_MON_BASE)
#undef PROC_MON_BASE
#if defined(PROC_MON_PHYS_BASE)
#define PROC_MON_BASE		(PROC_MON_PHYS_BASE)
#else
#define PROC_MON_BASE		(PMC_PHYS_BASE + PROC_MON_OFFSET)
#endif
#endif

#undef PMC_SIZE
#define PMC_SIZE			0x00200000

#define PLAT_BCM_CRASH_UART_BASE	UART0_BASE
#define PLAT_BCM_CRASH_UART_CLK_IN_HZ	UART0_CLK_IN_HZ
#define PLAT_BCM_CONSOLE_BAUDRATE	115200

#define PLAT_ARM_CRASH_UART_BASE	PLAT_BCM_CRASH_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ	PLAT_BCM_CRASH_UART_CLK_IN_HZ
#define ARM_CONSOLE_BAUDRATE	PLAT_BCM_CONSOLE_BAUDRATE

#define DEVICE0_BASE			0x08000000
#define DEVICE0_SIZE			0x00021000
#define DEVICE1_BASE			0x09000000
#define DEVICE1_SIZE			0x00011000


#if defined(WDTIMR0_BASE)
#undef WDTIMR0_BASE
#define WDTIMR0_BASE (PERF_PHYS_BASE+WDTIMR0_OFFSET)
#define WDTIMR_BASE  (PERF_PHYS_BASE)
#define WDTIMR_SIZE  0x4000
#endif

/*
 * GIC related constants
 */

#undef GICD_BASE
#undef GICC_BASE
#undef SCU_BASE
#if defined (PLATFORM_FLAVOR_63138) || defined (PLATFORM_FLAVOR_63148)
#define GICD_BASE			GICD_PHYS_BASE
#define GICC_BASE			GICC_PHYS_BASE
#if defined (PLATFORM_FLAVOR_63138)
#define SCU_BASE			SCU_PHYS_BASE
#endif
#else
#define GICD_BASE			(GIC_PHYS_BASE + GICD_OFFSET)
#define GICC_BASE			(GIC_PHYS_BASE + GICC_OFFSET)
#endif

#define GICR_BASE			0


#define BRCM_IRQ_SEC_SGI_0		8
#define BRCM_IRQ_SEC_SGI_1		9
#define BRCM_IRQ_SEC_SGI_2		10
#define BRCM_IRQ_SEC_SGI_3		11
#define BRCM_IRQ_SEC_SGI_4		12
#define BRCM_IRQ_SEC_SGI_5		13
#define BRCM_IRQ_SEC_SGI_6		14
#define BRCM_IRQ_SEC_SGI_7		15

#if defined (PLATFORM_FLAVOR_63138)
#define PL310_BASE		0x8001D000
#define PL310_MAP_SIZE	0x00002000

#undef  SCU_BASE
#define SCU_BASE		0x8001E000
#define SCU_ERRATA744369	0x30

#define BIT32(nr)		(1 << (nr))
/*
 * Outer cache iomem
 */
#define PL310_LINE_SIZE		32
#define PL310_8_WAYS		8

/* reg1 */
#define PL310_CTRL		0x100
#define PL310_AUX_CTRL		0x104
#define PL310_TAG_RAM_CTRL	0x108
#define PL310_DATA_RAM_CTRL	0x10C
/* reg7 */
#define PL310_SYNC		0x730
#define PL310_INV_BY_WAY	0x77C
#define PL310_CLEAN_BY_WAY	0x7BC
#define PL310_FLUSH_BY_WAY	0x7FC
#define PL310_INV_BY_PA		0x770
#define PL310_CLEAN_BY_PA	0x7B0
#define PL310_FLUSH_BY_PA	0x7F0
#define PL310_FLUSH_BY_INDEXWAY	0x7F8
/* reg9 */
#define PL310_DCACHE_LOCKDOWN_BASE 0x900
#define PL310_ICACHE_LOCKDOWN_BASE 0x904
/* reg12 */
#define PL310_ADDR_FILT_START	0xC00
#define PL310_ADDR_FILT_END	0xC04
/* reg15 */
#define PL310_DEBUG_CTRL	0xF40
#define PL310_PREFETCH_CTRL	0xF60
#define PL310_POWER_CTRL	0xF80

#define PL310_CTRL_ENABLE_BIT	BIT32(0)
#define PL310_AUX_16WAY_BIT	BIT32(16)

/*
 * PL310 TAG RAM Control Register
 *
 * bit[10:8]:1 - 2 cycle of write accesses latency
 * bit[6:4]:1 - 2 cycle of read accesses latency
 * bit[2:0]:1 - 2 cycle of setup latency
 */
#ifndef PL310_TAG_RAM_CTRL_INIT
#define PL310_TAG_RAM_CTRL_INIT		0x00000111
#endif

/*
 * PL310 DATA RAM Control Register
 *
 * bit[10:8]:2 - 3 cycle of write accesses latency
 * bit[6:4]:2 - 3 cycle of read accesses latency
 * bit[2:0]:2 - 3 cycle of setup latency
 */
#ifndef PL310_DATA_RAM_CTRL_INIT
#define PL310_DATA_RAM_CTRL_INIT	0x00000111
#endif

/*
 * PL310 Auxiliary Control Register
 *
 * I/Dcache prefetch enabled (bit29:28=2b11)
 * NS can access interrupts (bit27=1)
 * NS can lockown cache lines (bit26=1)
 * Pseudo-random replacement policy (bit25=0)
 * Force write allocated (default)
 * Shared attribute internally ignored (bit22=1, bit13=0)
 * Parity disabled (bit21=0)
 * Event monitor disabled (bit20=0)
 * Platform fmavor specific way config (dual / quad):
 * - 64kb way size (bit19:17=3b011)
 * - 16-way associciativity (bit16=1)
 * Platform fmavor specific way config (dual lite / solo):
 * - 32kb way size (bit19:17=3b010)
 * - no 16-way associciativity (bit16=0)
 * Store buffer device limitation enabled (bit11=1)
 * Cacheable accesses have high prio (bit10=0)
 * Full Line Zero (FLZ) disabled (bit0=0)
 */

#define PL310_AUX_CTRL_INIT		0x4e450001

/*
 * PL310 Prefetch Control Register
 *
 * Double linefill disabled (bit30=0)
 * I/D prefetch enabled (bit29:28=2b11)
 * Prefetch drop enabled (bit24=1)
 * Incr double linefill disable (bit23=0)
 * Prefetch offset = 7 (bit4:0)
 */
#define PL310_PREFETCH_CTRL_INIT	0x31000007

/*
 * PL310 Power Register
 *
 * Dynamic clock gating enabled
 * Standby mode enabled
 */
#define PL310_POWER_CTRL_INIT		0x00000003

#define GICC_CTLR_OFFSET		0x0
#define GICC_PMR_OFFSET			0x4
#define GICD_CTLR_OFFSET		0x0
#define GICD_TYPER_OFFSET		0x4
#define GICD_IGROUPR0_OFFSET		0x80

#endif

/*
 * System counter
 */
#define SYS_COUNTER_FREQ_IN_TICKS	(50 * 1000 * 1000)

#endif /* __PLATFORM_DEF_H__ */
