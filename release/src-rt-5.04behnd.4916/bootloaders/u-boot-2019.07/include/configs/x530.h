/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Allied Telesis Labs
 */

#ifndef _CONFIG_X530_H
#define _CONFIG_X530_H

/*
 * High Level Configuration Options (easy to change)
 */

#define CONFIG_DISPLAY_BOARDINFO_LATE

#define CONFIG_SYS_TCLK		250000000	/* 250MHz */

/*
 * NS16550 Configuration
 */
#define CONFIG_SYS_NS16550_SERIAL
#define CONFIG_SYS_NS16550_CLK		CONFIG_SYS_TCLK
#if !defined(CONFIG_DM_SERIAL)
#define CONFIG_SYS_NS16550_REG_SIZE	(-4)
#define CONFIG_SYS_NS16550_COM1		MV_UART_CONSOLE_BASE
#endif

/*
 * Serial Port configuration
 * The following definitions let you select what serial you want to use
 * for your console driver.
 */

#define CONFIG_CONS_INDEX	1	/*Console on UART0 */

/*
 * Commands configuration
 */
#define CONFIG_CMD_PCI

/* NAND */
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_MAX_NAND_DEVICE 1

#define BBT_CUSTOM_SCAN
#define BBT_CUSTOM_SCAN_PAGE 0
#define BBT_CUSTOM_SCAN_POSITION 2048

/* SPI NOR flash default params, used by sf commands */

#define MTDIDS_DEFAULT			"nand0=nand"
#define MTDPARTS_DEFAULT		"mtdparts=nand:240M(user),8M(errlog),8M(nand-bbt)"
#define MTDPARTS_MTDOOPS		"errlog"

/* Partition support */

/* Additional FS support/configuration */

/* USB/EHCI configuration */
#define CONFIG_EHCI_IS_TDI

/* Environment in SPI NOR flash */
#define CONFIG_ENV_OFFSET		(1 << 20) /* 1MiB in */
#define CONFIG_ENV_SIZE			(64 << 10) /* 64KiB */
#define CONFIG_ENV_SECT_SIZE		(256 << 10) /* 256KiB sectors */
#define CONFIG_ENV_ADDR			CONFIG_ENV_OFFSET

#define CONFIG_PHY_MARVELL		/* there is a marvell phy */
#define PHY_ANEG_TIMEOUT	8000	/* PHY needs a longer aneg time */

/* PCIe support */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_PCI_SCAN_SHOW
#endif

/* NAND */
#define CONFIG_SYS_NAND_USE_FLASH_BBT
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_LZO
#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_MTDPARTS

#define CONFIG_SYS_MALLOC_LEN		(4 << 20)

#include <asm/arch/config.h>

/*
 * Other required minimal configurations
 */
#define CONFIG_ARCH_CPU_INIT	/* call arch_cpu_init() */
#define CONFIG_SYS_RESET_ADDRESS 0xffff0000	/* Rst Vector Adr */

#define CONFIG_SYS_ALT_MEMTEST

/* Keep device tree and initrd in low memory so the kernel can access them */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"fdt_high=0x10000000\0"		\
	"initrd_high=0x10000000\0"

#define CONFIG_SYS_LOAD_ADDR	0x1000000
#define CONFIG_UBI_PART			user
#define CONFIG_UBIFS_VOLUME		user

/* SPL */

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

/* SPL related SPI defines */
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x24000
#define CONFIG_SYS_U_BOOT_OFFS		CONFIG_SYS_SPI_U_BOOT_OFFS

#endif /* _CONFIG_X530_H */
