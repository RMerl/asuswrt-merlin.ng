/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * K+P iMX6Q KP_IMX6Q_TPC board configuration
 *
 * Copyright (C) 2018 Lukasz Majewski <lukma@denx.de>
 */

#ifndef __KP_IMX6Q_TPC_IMX6_CONFIG_H_
#define __KP_IMX6Q_TPC_IMX6_CONFIG_H_

#include <asm/arch/imx-regs.h>

#include "mx6_common.h"

/* SPL */
#include "imx6_spl.h"			/* common IMX6 SPL configuration */

/* Miscellaneous configurable options */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(4 * SZ_1M)

/* FEC ethernet */
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		0
#define CONFIG_ARP_TIMEOUT		200UL

/* I2C Configs */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_SPEED		100000

/* MMC Configs */
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_USDHC_NUM	2
#define CONFIG_SYS_MMC_ENV_DEV		1 /* 0 = SDHC2, 1 = SDHC4 (eMMC) */

/* UART */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART1_BASE
#define CONFIG_CONS_INDEX		1
#define CONFIG_BAUDRATE			115200

/* USB Configs */
#ifdef CONFIG_CMD_USB
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2 /* Enabled USB controller number */
#endif

/* Watchdog */
#define CONFIG_WATCHDOG_TIMEOUT_MSECS	60000

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#define CONFIG_LOADADDR			0x12000000
#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR

#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"console=ttymxc0,115200\0"	\
	"fdt_addr=0x18000000\0"		\
	"fdt_high=0xffffffff\0"		\
	"initrd_high=0xffffffff\0"	\
	"kernel_addr_r=0x10008000\0"	\
	"fdt_addr_r=0x13000000\0"	\
	"ramdisk_addr_r=0x18000000\0"	\
	"scriptaddr=0x14000000\0"	\
	"kernel_file=fitImage\0"\
	"rdinit=/sbin/init\0" \
	"addinitrd=setenv bootargs ${bootargs} rdinit=${rdinit} ${debug} \0" \
	"fit_config=mx6q_tpc70_conf\0" \
	"upd_image=st.4k\0" \
	"updargs=setenv bootargs console=${console} ${smp}"\
	       "rdinit=${rdinit} ${debug} ${displayargs}\0" \
	"loadusb=usb start; " \
	       "fatload usb 0 ${loadaddr} ${upd_image}\0" \
	"usbupd=echo Booting update from usb ...; " \
	       "setenv bootargs; " \
	       "run updargs; " \
	       "run loadusb; " \
	       "bootm ${loadaddr}#${fit_config}\0" \
	BOOTENV

#define CONFIG_BOOTCOMMAND		"run usbupd; run distro_bootcmd"

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>
#endif

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)

#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Environment */
#define CONFIG_ENV_SIZE	(SZ_8K)
#define CONFIG_ENV_OFFSET       0x100000
#define CONFIG_ENV_OFFSET_REDUND (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT

#endif	/* __KP_IMX6Q_TPC_IMX6_CONFIG_H_ */
