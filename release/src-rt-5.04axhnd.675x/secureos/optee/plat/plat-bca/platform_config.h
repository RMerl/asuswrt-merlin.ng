/*
<:copyright-BRCM:2019:DUAL/GPL:standard

   Copyright (c) 2019 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#ifndef PLATFORM_CONFIG_H
#define PLATFORM_CONFIG_H

#include <stdint.h>
#include <bcm_mem_reserve.h>

/* Make stacks aligned to data cache line length */
#define STACK_ALIGNMENT		64

/* Idenify the TEE code space */
#define SECRAM_BASE		(CFG_OPTEE_AREA_ADDR)
#define SECRAM_SIZE		(CFG_OPTEE_CORE_SIZE)
/* Define 1MB of reserved memory */
#define CFG_SHMEM_START		(CFG_OPTEE_SHRM_ADDR)
#define CFG_SHMEM_SIZE		(CFG_OPTEE_SHRM_SIZE)

#define TZDRAM_BASE		SECRAM_BASE
#define TZDRAM_SIZE		SECRAM_SIZE

#if defined(PLATFORM_FLAVOR_63138) 
#define CFG_TEE_CORE_NB_CORE	2
#define PER_BASE                 0xfffe8000
#define UART0_PHYS_BASE          PER_BASE     /* uart registers */
#define UART0_PHYS_OFFSET        0x00000600


/* Secure Range Checkers Registers. */
#define RANGE_CHK_PHYS_BASE         0x80002000
#define RANGE_CHK_PHYS_SIZE         0x1000

#define RANGE_CHK_LOCK_OFFSET       0x800
#define RANGE_CHK_CONTROL_1_OFFSET  0x818
#define RANGE_CHK_UBUS_1_LOW_OFFSET 0x81C
#define RANGE_CHK_BASE_1_OFFSET     0x820

#define RANGE_CHK_CONTROL_2_OFFSET  0x824
#define RANGE_CHK_UBUS_2_LOW_OFFSET 0x828
#define RANGE_CHK_BASE_2_OFFSET     0x82C

#define RANGE_CHK_CONTROL_3_OFFSET  0x830
#define RANGE_CHK_UBUS_3_LOW_OFFSET 0x834
#define RANGE_CHK_BASE_3_OFFSET     0x838

#define RANGE_CHK_CONTROL_4_OFFSET  0x83C
#define RANGE_CHK_UBUS_4_LOW_OFFSET 0x840
#define RANGE_CHK_BASE_4_OFFSET     0x844

#define GIC_BASE		0x8001e000
#define GT_BASE			0x8001e000
#define GT_OFFSET		0x00000200
#define GT_REG_SIZE		0x00001000

#define PL310_BASE		0x8001d000
#define PL310_MAP_SIZE	0x00001000


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

#define PL310_AUX_CTRL_INIT		0x6e450001

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

#define PCSC_BASE		0x09100000

#define IT_UART1		40
#define IT_PCSC			37

#define PMC_PHYS_BASE		0x80400000
#define PMC_MAP_SIZE		0x00100000

#define GICC_OFFSET		0x100
#define GICD_OFFSET		0x1000

#else

#if defined(PLATFORM_FLAVOR_6856) || defined(PLATFORM_FLAVOR_6846) || defined (PLATFORM_FLAVOR_63146)
#define CFG_TEE_CORE_NB_CORE     2
#elif defined(PLATFORM_FLAVOR_63178)
#define CFG_TEE_CORE_NB_CORE     3
#else
#define CFG_TEE_CORE_NB_CORE     4
#endif

#define PER_BASE                 0xff800000
#define UART0_PHYS_BASE          PER_BASE
#if defined(PLATFORM_FLAVOR_6858) || defined(PLATFORM_FLAVOR_6856) || defined(PLATFORM_FLAVOR_6846) || defined(PLATFORM_FLAVOR_4908)
#define UART0_PHYS_OFFSET          0x00000640  /* uart registers */
#else
#define UART0_PHYS_OFFSET          0x00012000  /* uart registers */
#endif

/* Secure Range Checkers Registers. */
#if defined(PLATFORM_FLAVOR_4908)
#define RANGE_CHK_PHYS_BASE         0x80019000
#else
#define RANGE_CHK_PHYS_BASE         0x80181000
#endif
#define RANGE_CHK_PHYS_SIZE         0x1000

#define RANGE_CHK_LOCK_OFFSET       0x800
#define RANGE_CHK_CONTROL_1_OFFSET  0x828
#define RANGE_CHK_UBUS_1_LOW_OFFSET 0x82C
#define RANGE_CHK_UBUS_1_UPP_OFFSET 0x830
#define RANGE_CHK_BASE_1_OFFSET     0x834

#define RANGE_CHK_CONTROL_2_OFFSET  0x840
#define RANGE_CHK_UBUS_2_LOW_OFFSET 0x844
#define RANGE_CHK_UBUS_2_UPP_OFFSET 0x848
#define RANGE_CHK_BASE_2_OFFSET     0x84C

#define RANGE_CHK_CONTROL_3_OFFSET  0x858
#define RANGE_CHK_UBUS_3_LOW_OFFSET 0x85C
#define RANGE_CHK_UBUS_3_UPP_OFFSET 0x850
#define RANGE_CHK_BASE_3_OFFSET     0x854

#define RANGE_CHK_CONTROL_4_OFFSET  0x870
#define RANGE_CHK_UBUS_4_LOW_OFFSET 0x874
#define RANGE_CHK_UBUS_4_UPP_OFFSET 0x878
#define RANGE_CHK_BASE_4_OFFSET     0x87C


#define GIC_BASE		0x81000000
#define PCSC_BASE		0x09100000

#define IT_UART1		40
#define IT_PCSC			37

#define CLUSTER_RESET_BASE	0x81060000
#define CLUSTER_RESET_OFFSET	0x120
#define CLUSTER_RESET_SIZE	0x00001000

#define PMC_PHYS_BASE		0x80301000
#define PMC_MAP_SIZE		0x00100000

#define GICC_OFFSET		0x00002000
#if defined(PLATFORM_FLAVOR_6846) || defined(PLATFORM_FLAVOR_6858) || defined(PLATFORM_FLAVOR_6856)
#define GICD_OFFSET		0x00000000
#else
#define GICD_OFFSET		0x00001000
#endif

#if defined(PLATFORM_FLAVOR_6846) || defined(PLATFORM_FLAVOR_6856) || defined(PLATFORM_FLAVOR_63178) || defined(PLATFORM_FLAVOR_6756)
#define PMC_VERSION_3
#endif

#endif

#define CONSOLE_UART_BASE	UART0_PHYS_BASE
#define CONSOLE_UART_OFFSET	UART0_PHYS_OFFSET

#define BOOT_LOOKUP_BASE	0xFFFF0000
#define BOOT_LUT_RESET_OFFSET	0x20
#define BOOT_LOOKUP_SIZE	0x00001000

/* Secure Range Checkers Register Values. */
#define RANGE_CHK_BASE_1_START     CFG_ATF_AREA_ADDR /* Start address of ATF */
#define RANGE_CHK_BASE_1_SIZE_512K 0x07        /* 512KB */
#define RANGE_CHK_BASE_1_VAL       (RANGE_CHK_BASE_1_START | RANGE_CHK_BASE_1_SIZE_512K)
#define RANGE_CHK_BASE_2_START     (RANGE_CHK_BASE_1_START + (1024 * 512))
#define RANGE_CHK_BASE_2_SIZE_256K 0x06        /* 256KB. */
#define RANGE_CHK_BASE_2_VAL       (RANGE_CHK_BASE_2_START | RANGE_CHK_BASE_2_SIZE_256K)
#define RANGE_CHK_BASE_3_START     CFG_OPTEE_AREA_ADDR /* Start address OPTEE */
#define RANGE_CHK_BASE_3_SIZE_4MB  0x0A        /* 4MB. */
#define RANGE_CHK_BASE_3_VAL       (RANGE_CHK_BASE_3_START | RANGE_CHK_BASE_3_SIZE_4MB)

#define RANGE_CHK_CONTROL_INTF_EN  (0x3f << 16)/* interface enable. */
#define RANGE_CHK_CONTROL_EN       (0x3f << 8) /* interface range check enable. */
#define RANGE_CHK_CONTROL_SEC_ACC  (0x0c << 4) /* secure read-write access settings. */
#define RANGE_CHK_CONTROL_NSEC_ACC (0x0d << 4) /* secure read-write and non-secure read access settings. */
#define RANGE_CHK_CONTROL_ALL_ACC  (0x0f << 4) /* all access settings. */
#define RANGE_CHK_CONTROL_ABT      (0x01 << 0) /* transaction abort enable. */
#define RANGE_CHK_CONTROL_SEC_RW   (RANGE_CHK_CONTROL_INTF_EN | RANGE_CHK_CONTROL_EN | RANGE_CHK_CONTROL_SEC_ACC | RANGE_CHK_CONTROL_ABT)
#define RANGE_CHK_CONTROL_NSEC_RD  (RANGE_CHK_CONTROL_INTF_EN | RANGE_CHK_CONTROL_EN | RANGE_CHK_CONTROL_NSEC_ACC | RANGE_CHK_CONTROL_ABT)
#define RANGE_CHK_CONTROL_ALL_RW   (RANGE_CHK_CONTROL_INTF_EN | RANGE_CHK_CONTROL_EN | RANGE_CHK_CONTROL_ALL_ACC | RANGE_CHK_CONTROL_ABT)

#define RANGE_CHK_UBUS_EN          0xffffffff  /* Enable the UBUS PORTx checker. */

#define RANGE_CHK_LOCK_EN          (0x1 << 31) /* Lock enable. */
#define RANGE_CHK_RANGE_LOCK_ALL   0xff        /* lock range checkers 0:7. */
#define RANGE_CHK_LOCK_VAL         (RANGE_CHK_LOCK_EN | RANGE_CHK_RANGE_LOCK_ALL)
#define RANGE_CHK_LOCK(n)          (1 << n)    /* Lock for n th range checker */


/*
 * Assumes that either TZSRAM isn't large enough or TZSRAM doesn't exist,
 * everything is in TZDRAM.
 * +------------------+
 * |        | TEE_RAM | <-- CFG_TEE_RAM_PH_SIZE
 * | TZDRAM +---------+
 * |        | TA_RAM  | <-- CFG_TA_RAM_SIZE
 * +--------+---------+
 */
#define CFG_TEE_RAM_PH_SIZE	(2 * 1024 * 1024)
#define CFG_TEE_RAM_VA_SIZE	CFG_TEE_RAM_PH_SIZE
#define CFG_TEE_RAM_START	TZDRAM_BASE
#define CFG_TEE_LOAD_ADDR	CFG_TEE_RAM_START

#define CFG_TA_RAM_START	(CFG_TEE_RAM_START + CFG_TEE_RAM_PH_SIZE)
#define CFG_TA_RAM_SIZE		(TZDRAM_SIZE - CFG_TEE_RAM_PH_SIZE)

#ifndef UART_BAUDRATE
#define UART_BAUDRATE		115200
#endif
#ifndef CONSOLE_BAUDRATE
#define CONSOLE_BAUDRATE	UART_BAUDRATE
#endif

/* For virtual platforms where there isn't a clock */
#ifndef CONSOLE_UART_CLK_IN_HZ
#define CONSOLE_UART_CLK_IN_HZ	1
#endif

#endif /*PLATFORM_CONFIG_H*/
#define SOTP_KEY_SEC_SIZE		(8+1+3)
#define SOTP_KEY_SEC_START	28
#define SOTP_BASE		0xff800c00

/* Renamed the defines in OPTEE 3.7.0 */
#define TEE_RAM_START		CFG_TEE_RAM_START
#define TEE_RAM_PH_SIZE		CFG_TEE_RAM_PH_SIZE
#define TEE_SHMEM_START		CFG_SHMEM_START
#define TEE_SHMEM_SIZE		CFG_SHMEM_SIZE
#define TA_RAM_START		CFG_TA_RAM_START
#define TA_RAM_SIZE		CFG_TA_RAM_SIZE
