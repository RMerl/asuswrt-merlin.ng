/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2013 Freescale Semiconductor, Inc.
 *
 * Configuration settings for Udoo board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "mx6_common.h"

#include "imx6_spl.h"

/* Provide the MACH_TYPE value that the vendor kernel requires. */
#define CONFIG_MACH_TYPE		4800

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(2 * SZ_1M)

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART2_BASE

/* SATA Configs */

#ifdef CONFIG_CMD_SATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR
#define CONFIG_LBA48
#endif

/* Network support */

#define CONFIG_FEC_MXC
#define IMX_FEC_BASE                    ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE             RGMII
#define CONFIG_ETHPRIME                 "FEC"
#define CONFIG_FEC_MXC_PHYADDR          6

#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 500 * SZ_1M)

/* MMC Configuration */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=ttymxc1,115200\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdtfile=undefined\0" \
	"fdt_addr=0x18000000\0" \
	"fdt_addr_r=0x18000000\0" \
	"ip_dyn=yes\0" \
	"mmcdev=0\0" \
	"mmcrootfstype=ext4\0" \
	"findfdt="\
		"if test ${board_rev} = MX6Q; then " \
			"setenv fdtfile imx6q-udoo.dtb; fi; " \
		"if test ${board_rev} = MX6DL; then " \
			"setenv fdtfile imx6dl-udoo.dtb; fi; " \
		"if test ${fdtfile} = undefined; then " \
			"echo WARNING: Could not determine dtb to use; fi\0" \
	"kernel_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"pxefile_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"ramdisk_addr_r=0x13000000\0" \
	"scriptaddr=" __stringify(CONFIG_LOADADDR) "\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(SATA, sata, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Environment organization */
#define CONFIG_ENV_SIZE			(8 * 1024)

#define CONFIG_ENV_OFFSET		(6 * 64 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV		0

#endif			       /* __CONFIG_H * */
