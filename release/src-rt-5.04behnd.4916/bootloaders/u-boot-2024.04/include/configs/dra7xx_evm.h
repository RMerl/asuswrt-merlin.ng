/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * Texas Instruments Incorporated.
 * Lokesh Vutla	  <lokeshvutla@ti.com>
 *
 * Configuration settings for the TI DRA7XX board.
 * See ti_omap5_common.h for omap5 common settings.
 */

#ifndef __CONFIG_DRA7XX_EVM_H
#define __CONFIG_DRA7XX_EVM_H

#include <env/ti/dfu.h>

#define CFG_MAX_MEM_MAPPED		0x80000000

#ifndef CONFIG_QSPI_BOOT
/* MMC ENV related defines */
#endif

#if (CONFIG_CONS_INDEX == 1)
#define CONSOLEDEV			"ttyS0"
#elif (CONFIG_CONS_INDEX == 3)
#define CONSOLEDEV			"ttyS2"
#endif
#define CFG_SYS_NS16550_COM1		UART1_BASE	/* Base EVM has UART0 */
#define CFG_SYS_NS16550_COM2		UART2_BASE	/* UART2 */
#define CFG_SYS_NS16550_COM3		UART3_BASE	/* UART3 */

#ifndef CONFIG_SPL_BUILD
#define DFUARGS \
	"dfu_bufsiz=0x10000\0" \
	DFU_ALT_INFO_MMC \
	DFU_ALT_INFO_EMMC \
	DFU_ALT_INFO_RAM \
	DFU_ALT_INFO_QSPI
#endif

#ifdef CONFIG_SPL_BUILD
#ifdef CONFIG_SPL_DFU
#define DFUARGS \
	"dfu_bufsiz=0x10000\0" \
	DFU_ALT_INFO_RAM
#endif
#endif

#include <configs/ti_omap5_common.h>

/*
 * Default to using SPI for environment, etc.
 * 0x000000 - 0x040000 : QSPI.SPL (256KiB)
 * 0x040000 - 0x140000 : QSPI.u-boot (1MiB)
 * 0x140000 - 0x1C0000 : QSPI.u-boot-spl-os (512KiB)
 * 0x1C0000 - 0x1D0000 : QSPI.u-boot-env (64KiB)
 * 0x1D0000 - 0x1E0000 : QSPI.u-boot-env.backup1 (64KiB)
 * 0x1E0000 - 0x9E0000 : QSPI.kernel (8MiB)
 * 0x9E0000 - 0x2000000 : USERLAND
 */
#define CFG_SYS_SPI_KERNEL_OFFS	0x1E0000
#define CFG_SYS_SPI_ARGS_OFFS	0x140000
#define CFG_SYS_SPI_ARGS_SIZE	0x80000

/* SPI SPL */

/* NAND support */
#ifdef CONFIG_MTD_RAW_NAND
/* NAND: device related configs */
/* NAND: driver related configs */
#define CFG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }
#define CFG_SYS_NAND_ECCSIZE		512
#define CFG_SYS_NAND_ECCBYTES	14
#endif /* !CONFIG_MTD_RAW_NAND */

/* Parallel NOR Support */
#if defined(CONFIG_NOR)
/* NOR: device related configs */
#define CFG_SYS_FLASH_SIZE		(64 * 1024 * 1024) /* 64 MB */
#define CFG_SYS_FLASH_BASE		(0x08000000)
/* Reduce SPL size by removing unlikey targets */
#endif  /* NOR support */

#endif /* __CONFIG_DRA7XX_EVM_H */
