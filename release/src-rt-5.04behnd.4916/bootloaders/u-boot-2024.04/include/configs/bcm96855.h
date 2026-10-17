/* SPDX-License-Identifier: GPL-2.0+
 *
 * (C) Copyright 2022 Broadcom Ltd.
 *
 */
#ifndef __BCM96855_H
#define __BCM96855_H

#include <linux/sizes.h>

/* UART */
#define CFG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, \
					  230400, 500000, 1500000 }

#define CFG_ENV_CALLBACK_LIST_STATIC "boardid:boardid,active_port:active_port,voiceboardid:voiceboardid,bufmem:bufmem,rnrmem:rnrmem,rdp1:rdp1,rdp2:rdp2,"

/* RAM */
#define CFG_PHYS_SDRAM_1		0x00000000UL
#define CFG_PHYS_SDRAM_1_SIZE		(2UL * SZ_1G) /* Maximum possible bnk 0 size */
#define CFG_SYS_SDRAM_BASE		CFG_PHYS_SDRAM_1

#define CFG_SYS_INIT_RAM_ADDR		0x82000000
#define CFG_SYS_INIT_RAM_SIZE		0x00040000
#define CFG_SYS_SEC_CRED_ADDR		((CFG_SYS_INIT_RAM_ADDR+CFG_SYS_INIT_RAM_SIZE) - 0x400)

#if defined(CONFIG_TPL_BUILD)
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_TPL_TEXT_BASE + 0x00f00000)
#define CFG_SYS_PAGETBL_BASE		CONFIG_SYS_INIT_SP_ADDR
#define CFG_SYS_PAGETBL_SIZE		0x00004000
#elif defined(CONFIG_SPL_BUILD)
/* SPL code, data, bss   0x82000000 - 0x90220000 */
/* SPL DDR std, dpfe     0x82020000 - 0x90230000 */
/* SPL GD, stack, heap   0x90230000 - 0x90240000 */
/* SPL page table        0x82700000 - 0x82704000 */
#define CFG_SYS_INIT_SP_OFFSET \
	(CFG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CFG_SYS_INIT_RAM_ADDR + CFG_SYS_INIT_SP_OFFSET)
#define CFG_SYS_PAGETBL_BASE		0x82700000
#define CFG_SYS_PAGETBL_SIZE		0x4000
/* These definitions are now moved to defconfig. But kept here as comments for easy reference */
//#define CONFIG_SPL_TEXT_BASE			0x82000000
//#define CONFIG_SPL_MAX_SIZE			0x20000
#else
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_TEXT_BASE + SZ_16M)
#endif

#define CFG_SYS_NAND_BASE		0xff801800
#define CFG_SYS_BOOTMAPSZ		(128 << 20)
#define CFG_SMP_PEN_ADDR		0x0

#endif
