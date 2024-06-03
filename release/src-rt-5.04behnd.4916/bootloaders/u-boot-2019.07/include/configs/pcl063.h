/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Collabora Ltd.
 *
 * Based on include/configs/xpress.h:
 * Copyright (C) 2015-2016 Stefan Roese <sr@denx.de>
 */
#ifndef __PCL063_H
#define __PCL063_H

#include <linux/sizes.h>
#include "mx6_common.h"

/* SPL options */
#include "imx6_spl.h"

/*
 * There is a bug in some i.MX6UL processors that results in the initial
 * portion of OCRAM being unavailable when booting from (at least) an SD
 * card.
 *
 * Tweak the SPL text base address to avoid this.
 */

#define CONFIG_SYS_FSL_USDHC_NUM	1

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(16 * SZ_1M)

/* Console configs */
#define CONFIG_MXC_UART_BASE		UART1_BASE

/* MMC Configs */
#define CONFIG_FSL_USDHC

#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC1_BASE_ADDR

/* Miscellaneous configurable options */
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x10000000)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_SIZE			SZ_256M

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_ENV_SIZE			(16 << 10)
#define CONFIG_ENV_OFFSET		(512 << 10)

/* NAND */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000

/* USB Configs */
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	1

#define CONFIG_IMX_THERMAL

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=ttymxc0,115200n8\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"fdt_addr_r=0x82000000\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"kernel_addr_r=0x81000000\0" \
	"pxefile_addr_r=0x87100000\0" \
	"ramdisk_addr_r=0x82100000\0" \
	"scriptaddr=0x87000000\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(UBIFS, ubifs, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#endif /* __PCL063_H */
