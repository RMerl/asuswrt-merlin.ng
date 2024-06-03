/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

/* ram memory-related information */
#define PHYS_SDRAM_1			0x40000000
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define PHYS_SDRAM_1_SIZE		0x3E000000
#define CONFIG_SYS_LOAD_ADDR		PHYS_SDRAM_1	/* default load addr */

#define CONFIG_SYS_HZ_CLOCK		1000000000	/* 1 GHz */

/* Environment */

#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR
#define CONFIG_SYS_BOOTM_LEN		SZ_16M

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#define CONFIG_BOOTFILE			"uImage"
#define CONFIG_EXTRA_ENV_SETTINGS				\
			"kernel_addr_r=0x40000000\0"		\
			"fdtfile=stih410-b2260.dtb\0"		\
			"fdt_addr_r=0x47000000\0"		\
			"scriptaddr=0x50000000\0"		\
			"pxefile_addr_r=0x50100000\0"		\
			"fdt_high=0xffffffffffffffff\0"		\
			"initrd_high=0xffffffffffffffff\0"	\
			"ramdisk_addr_r=0x48000000\0"		\
			BOOTENV


#define CONFIG_ENV_SIZE 0x4000

/* Extra Commands */
#define CONFIG_CMD_ASKENV

#define CONFIG_SETUP_MEMORY_TAGS

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		0x1800000
#define CONFIG_SYS_GBL_DATA_SIZE	1024	/* Global data structures */
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_TEXT_BASE - \
					 CONFIG_SYS_MALLOC_LEN - \
					 CONFIG_SYS_GBL_DATA_SIZE)

/* Monitor Command Prompt */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */

#define CONFIG_SYS_MAX_FLASH_BANKS	1

#define CONFIG_SKIP_LOWLEVEL_INIT

/* USB Configs */
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	2

#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_USB_ETHER_MCS7830
#define CONFIG_USB_ETHER_SMSC95XX

/* NET Configs */

#endif /* __CONFIG_H */
