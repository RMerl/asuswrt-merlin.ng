/* SPDX-License-Identifier: GPL-2.0+
 *
 * (C) Copyright 2022 Broadcom Ltd.
 *
 */

#ifndef __BCM968375_H
#define __BCM968375_H

#include <linux/sizes.h>

/* UART */
#define CFG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, \
					  230400, 500000, 1500000 }

#define CFG_ENV_CALLBACK_LIST_STATIC "boardid:boardid,active_port:active_port,voiceboardid:voiceboardid,bufmem:bufmem,rnrmem:rnrmem,rdp1:rdp1,rdp2:rdp2,"

#define CPU_RELEASE_ADDR		0x1000
#define CONFIG_GICV2			1
#define GICD_BASE			0x81001000
#define GICC_BASE			0x81002000

/* RAM */
#define CFG_PHYS_SDRAM_1		0x00000000UL
#define CFG_PHYS_SDRAM_2		0x100000000UL
#define CFG_PHYS_SDRAM_1_SIZE		(2UL * SZ_1G) /* Maximum possible bnk 0 size */
#define CFG_PHYS_SDRAM_2_SIZE		(2UL * SZ_1G) /* Maximum possible bnk 1 size */
#define CFG_SYS_SDRAM_BASE		CFG_PHYS_SDRAM_1

#define CFG_SYS_INIT_STD_32K_ADDR	0xfff80000
#define CFG_SYS_INIT_STD_32K_SIZE	0x8000
#define CFG_SYS_SEC_CRED_ADDR		0x600000

#define CFG_DDR_REGION_TRUSTED_START	0x00600000UL
#define CFG_DDR_REGION_TRUSTED_END	0x00800000UL

#ifdef CONFIG_TPL_BUILD
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_TPL_TEXT_BASE + 0x00f00000)
#define CFG_SYS_PAGETBL_BASE		CONFIG_SYS_INIT_SP_ADDR
#define CFG_SYS_PAGETBL_SIZE		0x10000
#elif CONFIG_SPL_BUILD
/* SPL code,data,sp,gd,hp 0x05000000 - 0x05800000 */
/* SPL page table         0x05800000 - 0x05810000 */
/* SPL bss                0x05810000 - 0x05820000 */
#define CFG_SYS_INIT_RAM_ADDR		0x82000000
#define CFG_SYS_INIT_RAM_SIZE		0x80000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SPL_TEXT_BASE + SZ_8M)
#define CFG_SYS_PAGETBL_BASE		CONFIG_SYS_INIT_SP_ADDR
#define CFG_SYS_PAGETBL_SIZE		0x10000
/* These definitions are now moved to defconfig. But kept here as comments for easy reference */
//#define CONFIG_SPL_TEXT_BASE			0x05000000
//#define CONFIG_SPL_BSS_START_ADDR	0x05810000
//#define CONFIG_SPL_MAX_SIZE			0x60000
//#define CONFIG_SPL_BSS_MAX_SIZE		0x1000
#else
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_TEXT_BASE + SZ_16M)
#endif

#define CFG_SYS_BOOTMAPSZ		(128 << 20)

#endif
