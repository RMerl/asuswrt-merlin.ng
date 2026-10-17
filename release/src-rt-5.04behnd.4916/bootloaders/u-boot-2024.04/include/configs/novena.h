/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the Novena U-Boot.
 *
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* System configurations */

#include "mx6_common.h"

/* U-Boot Commands */

/* U-Boot general configurations */

/* U-Boot environment */
/*
 * Environment is on MMC, starting at offset 512KiB from start of the card.
 * Please place first partition at offset 1MiB from the start of the card
 * as recommended by GNU/fdisk. See below for details:
 * http://homepage.ntlworld.com./jonathan.deboynepollard/FGA/disc-partition-alignment.html
 */

/* Booting Linux */

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* I2C EEPROM */

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR	0
#define CFG_SYS_FSL_USDHC_NUM	2

/* PCI express */
#ifdef CONFIG_CMD_PCI
#define CFG_PCIE_IMX_PERST_GPIO	IMX_GPIO_NR(3, 29)
#define CFG_PCIE_IMX_POWER_GPIO	IMX_GPIO_NR(7, 12)
#endif

/* PMIC */
#define CFG_POWER_PFUZE100_I2C_ADDR	0x08

/* UART */
#define CFG_MXC_UART_BASE		UART2_BASE

/* USB Configs */
#ifdef CONFIG_CMD_USB
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0
#endif

/* Extra U-Boot environment. */
#define CFG_EXTRA_ENV_SETTINGS					\
	"fdt_high=0xffffffff\0"						\
	"initrd_high=0xffffffff\0"					\
	"consdev=ttymxc1\0"						\
	"baudrate=115200\0"						\
	"bootdev=/dev/mmcblk0p1\0"					\
	"rootdev=/dev/mmcblk0p2\0"					\
	"netdev=eth0\0"							\
	"kernel_addr_r="__stringify(CONFIG_SYS_LOAD_ADDR)"\0"		\
	"pxefile_addr_r="__stringify(CONFIG_SYS_LOAD_ADDR)"\0"		\
	"scriptaddr="__stringify(CONFIG_SYS_LOAD_ADDR)"\0"			\
	"ramdisk_addr_r=0x28000000\0"					\
	"fdt_addr_r=0x18000000\0"					\
	"fdtfile=imx6q-novena.dtb\0"					\
	"stdout=serial,vidconsole\0"					\
	"stderr=serial,vidconsole\0"					\
	"addcons="							\
		"setenv bootargs ${bootargs} "				\
		"console=${consdev},${baudrate}\0"			\
	"addip="							\
		"setenv bootargs ${bootargs} "				\
		"ip=${ipaddr}:${serverip}:${gatewayip}:"		\
			"${netmask}:${hostname}:${netdev}:off\0"	\
	"addmisc="							\
		"setenv bootargs ${bootargs} ${miscargs}\0"		\
	"addargs=run addcons addmisc\0"					\
	"mmcload="							\
		"mmc rescan ; "						\
		"ext4load mmc 0:1 ${kernel_addr_r} ${bootfile}\0"	\
	"netload="							\
		"tftp ${kernel_addr_r} ${hostname}/${bootfile}\0"	\
	"miscargs=nohlt panic=1\0"					\
	"mmcargs=setenv bootargs root=${rootdev} rw rootwait\0"		\
	"nfsargs="							\
		"setenv bootargs root=/dev/nfs rw "			\
			"nfsroot=${serverip}:${rootpath},v3,tcp\0"	\
	"mmc_mmc="							\
		"run mmcload mmcargs addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"mmc_nfs="							\
		"run mmcload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"net_mmc="							\
		"run netload mmcargs addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"net_nfs="							\
		"run netload nfsargs addip addargs ; "			\
		"bootm ${kernel_addr_r}\0"				\
	"update_sd_spl_filename=SPL\0"					\
	"update_sd_uboot_filename=u-boot.img\0"				\
	"update_sd_firmware="	/* Update the SD firmware partition */	\
		"if mmc rescan ; then "					\
		"if dhcp ${update_sd_spl_filename} ; then "		\
		"mmc write ${loadaddr} 2 0x200 ; "			\
		"fi ; "							\
		"if dhcp ${update_sd_uboot_filename} ; then "		\
		"fatwrite mmc 0:1 ${loadaddr} u-boot.img ${filesize} ; "\
		"fi ; "							\
		"fi\0"							\
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(SATA, sata, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#endif				/* __CONFIG_H */
