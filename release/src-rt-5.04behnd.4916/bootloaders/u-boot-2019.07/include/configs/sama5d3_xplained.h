/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the SAMA5D3 Xplained board.
 *
 * Copyright (C) 2014 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "at91-sama5_common.h"

/*
 * This needs to be defined for the OHCI code to work but it is defined as
 * ATMEL_ID_UHPHS in the CPU specific header files.
 */
#define ATMEL_ID_UHP			32

/*
 * Specify the clock enable bit in the PMC_SCER register.
 */
#define ATMEL_PMC_UHP			(1 <<  6)

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE           0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x10000000

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_INIT_SP_ADDR		0x318000
#else
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 - GENERATED_GBL_DATA_SIZE)
#endif

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x60000000
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ONFI_DETECTION
#endif

/* USB */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_UPLL
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE		0x00600000
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"SAMA5D3 Xplained"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2
#endif

#define CONFIG_SYS_LOAD_ADDR			0x22000000 /* load address */

/* SPL */
#define CONFIG_SPL_MAX_SIZE		0x18000
#define CONFIG_SPL_BSS_START_ADDR	0x20000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_SPL_MALLOC_START	0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#ifdef CONFIG_SD_BOOT
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME	"u-boot.img"

#elif CONFIG_NAND_BOOT
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_BASE
#endif
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x40000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_SIZE	0x800
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	0x20000
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0x0

#endif
