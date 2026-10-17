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
#define MAX_MMAP_REGIONS		16
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

#undef RNG_BASE
#undef TRNG_BASE
#if defined(HW_RNG_SUPPORT)
#if !defined(CONFIG_BRCM_SMC_BASED)
#define RNG_BASE	0xff800000
#define RNG_OFFSET	0x00000b80
#define RNG_SIZE	0x00001000
#endif

#if defined (PLATFORM_FLAVOR_4912) || defined (PLATFORM_FLAVOR_63146) || \
    defined (PLATFORM_FLAVOR_6813) || defined (PLATFORM_FLAVOR_6764)  || \
    defined (PLATFORM_FLAVOR_6765) || defined (PLATFORM_FLAVOR_6766)
#define TRNG_BASE	0xFF803000
#define TRNG_OFFSET	0x00000800
#define TRNG_SIZE	0x00010000
#endif
#endif

#undef TIMR_BASE
#if defined (PLATFORM_FLAVOR_63158) || defined (PLATFORM_FLAVOR_63178) || \
    defined (PLATFORM_FLAVOR_47622) || \
    defined (PLATFORM_FLAVOR_63146) || defined (PLATFORM_FLAVOR_4912)  || \
    defined (PLATFORM_FLAVOR_6813)  || defined (PLATFORM_FLAVOR_6756)  || \
    defined (PLATFORM_FLAVOR_6855)  || defined (PLATFORM_FLAVOR_6888)  || \
    defined (PLATFORM_FLAVOR_6765)  || defined (PLATFORM_FLAVOR_6766) || defined (PLATFORM_FLAVOR_6764)
#define UART0_BASE			0xff812000
#elif defined (PLATFORM_FLAVOR_6858) || defined (PLATFORM_FLAVOR_6856)
#define UART0_BASE			0xff800000
#elif defined (PLATFORM_FLAVOR_68880) || defined (PLATFORM_FLAVOR_6837) || defined (PLATFORM_FLAVOR_68375)
#define UART0_BASE			0x8421C000
#endif
#define UART0_SIZE			0x1000
#define UART0_BAUD			115200
#define UART0_CLOCK			(50 * 1000 * 1000)


#undef  PMC_BASE
#if defined PMC_PHYS_BASE
#if defined (PMC_OFFSET)
#define PMC_BASE			(PMC_PHYS_BASE + PMC_OFFSET)
#else
#define PMC_BASE			(PMC_PHYS_BASE)
#endif
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
#define PLAT_BCM_CRASH_UART_CLK_IN_HZ	UART0_CLOCK
#define PLAT_BCM_CONSOLE_BAUDRATE	UART0_BAUD

#define PLAT_ARM_CRASH_UART_BASE	PLAT_BCM_CRASH_UART_BASE
#define PLAT_ARM_CRASH_UART_CLK_IN_HZ	PLAT_BCM_CRASH_UART_CLK_IN_HZ
#define ARM_CONSOLE_BAUDRATE		PLAT_BCM_CONSOLE_BAUDRATE

#define DEVICE0_BASE			0x08000000
#define DEVICE0_SIZE			0x00021000
#define DEVICE1_BASE			0x09000000
#define DEVICE1_SIZE			0x00011000


#if defined(WDTIMR0_BASE)
#undef WDTIMR0_BASE
#if defined (PLATFORM_FLAVOR_68880) || defined (PLATFORM_FLAVOR_6837) || defined (PLATFORM_FLAVOR_68375)
#define WDTIMR0_BASE (TIMER_PHYS_BASE+WDTIMR0_OFFSET)
#define WDTIMR_BASE  (TIMER_PHYS_BASE)
#else
#define WDTIMR0_BASE (PERF_PHYS_BASE+WDTIMR0_OFFSET)
#define WDTIMR_BASE  (PERF_PHYS_BASE)
#endif
#define WDTIMR_SIZE  0x4000
#endif

/*
 * GIC related constants
 */

#undef GICD_BASE
#undef GICC_BASE
#undef SCU_BASE
#define GICD_BASE			(GIC_PHYS_BASE + GICD_OFFSET)
#define GICC_BASE			(GIC_PHYS_BASE + GICC_OFFSET)

#define GICR_BASE			0


#define BRCM_IRQ_SEC_SGI_0		8
#define BRCM_IRQ_SEC_SGI_1		9
#define BRCM_IRQ_SEC_SGI_2		10
#define BRCM_IRQ_SEC_SGI_3		11
#define BRCM_IRQ_SEC_SGI_4		12
#define BRCM_IRQ_SEC_SGI_5		13
#define BRCM_IRQ_SEC_SGI_6		14
#define BRCM_IRQ_SEC_SGI_7		15

/*
 * System counter
 */
#if defined (PLATFORM_FLAVOR_6765) || defined (PLATFORM_FLAVOR_6766) || defined (PLATFORM_FLAVOR_6764)
#define SYS_COUNTER_FREQ_IN_TICKS	(80 * 1000 * 1000)
#else
#define SYS_COUNTER_FREQ_IN_TICKS	(50 * 1000 * 1000)
#endif

#if defined (PLATFORM_FLAVOR_68880) || defined (PLATFORM_FLAVOR_6837) || defined (PLATFORM_FLAVOR_68375)
#define	PLATFORM_USING_SMC
#endif

#endif /* __PLATFORM_DEF_H__ */
