/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014
 * Texas Instruments Incorporated.
 * Felipe Balbi <balbi@ti.com>
 *
 * Configuration settings for the TI Beagle x15 board.
 * See ti_omap5_common.h for omap5 common settings.
 */

#ifndef __CONFIG_AM57XX_EVM_H
#define __CONFIG_AM57XX_EVM_H

#include <env/ti/dfu.h>
#include <linux/sizes.h>

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
#else
#ifdef CONFIG_SPL_DFU
#define DFUARGS \
	"dfu_bufsiz=0x10000\0" \
	DFU_ALT_INFO_RAM
#endif
#endif

#include <configs/ti_omap5_common.h>

/* CPSW Ethernet */
#define PHY_ANEG_TIMEOUT	8000	/* PHY needs longer aneg time at 1G */

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
#define CFG_SYS_SPI_KERNEL_OFFS      0x1E0000
#define CFG_SYS_SPI_ARGS_OFFS        0x140000
#define CFG_SYS_SPI_ARGS_SIZE        0x80000

/* SPI SPL */

#endif /* __CONFIG_AM57XX_EVM_H */
