/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Stefan Roese <sr@denx.de>
 */

#ifndef _CONFIG_DB_88F6820_AMC_H
#define _CONFIG_DB_88F6820_AMC_H

/*
 * High Level Configuration Options (easy to change)
 */

#define CONFIG_SYS_TCLK		200000000	/* 200MHz */

/*
 * Commands configuration
 */

/* USB/EHCI configuration */
#define CONFIG_EHCI_IS_TDI

/* Environment in SPI NOR flash */
#define CONFIG_ENV_OFFSET		(1 << 20) /* 1MiB in */
#define CONFIG_ENV_SIZE			(64 << 10) /* 64KiB */
#define CONFIG_ENV_SECT_SIZE		(256 << 10) /* 256KiB sectors */

#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/* PCIe support */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_PCI_SCAN_SHOW
#endif

/* NAND */
#define CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* Keep device tree and initrd in lower memory so the kernel can access them */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

/* SPL */
/*
 * Select the boot device here
 *
 * Currently supported are:
 * SPL_BOOT_SPI_NOR_FLASH	- Booting via SPI NOR flash
 *
 * MMC is not populated on this board.
 * NAND support may be added in the future.
 */
#define SPL_BOOT_SPI_NOR_FLASH		1
#define CONFIG_SPL_BOOT_DEVICE		SPL_BOOT_SPI_NOR_FLASH

/* Defines for SPL */
#define CONFIG_SPL_SIZE			(140 << 10)
#define CONFIG_SPL_MAX_SIZE		(CONFIG_SPL_SIZE - 0x0030)

#define CONFIG_SPL_BSS_START_ADDR	(0x40000000 + CONFIG_SPL_SIZE)
#define CONFIG_SPL_BSS_MAX_SIZE		(16 << 10)

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_MALLOC_SIMPLE
#endif

#define CONFIG_SPL_STACK		(0x40000000 + ((192 - 16) << 10))
#define CONFIG_SPL_BOOTROM_SAVE		(CONFIG_SPL_STACK + 4)

#if CONFIG_SPL_BOOT_DEVICE == SPL_BOOT_SPI_NOR_FLASH
/* SPL related SPI defines */
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x24000
#define CONFIG_SYS_U_BOOT_OFFS		CONFIG_SYS_SPI_U_BOOT_OFFS
#endif

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"
#undef CONFIG_SYS_MAXARGS
#define CONFIG_SYS_MAXARGS 96

#endif /* _CONFIG_DB_88F6820_AMC_H */
