/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the Novena U-Boot.
 *
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* System configurations */
#define CONFIG_KEYBOARD

#include "mx6_common.h"

/* U-Boot Commands */

/* U-Boot general configurations */

/* U-Boot environment */
#define CONFIG_ENV_SIZE			(16 * 1024)
/*
 * Environment is on MMC, starting at offset 512KiB from start of the card.
 * Please place first partition at offset 1MiB from the start of the card
 * as recommended by GNU/fdisk. See below for details:
 * http://homepage.ntlworld.com./jonathan.deboynepollard/FGA/disc-partition-alignment.html
 */
#ifdef CONFIG_CMD_MMC
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_ENV_OFFSET		(512 * 1024)
#define CONFIG_ENV_SIZE_REDUND		CONFIG_ENV_SIZE
#define CONFIG_ENV_OFFSET_REDUND	\
		(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#endif

/* Booting Linux */
#define CONFIG_BOOTFILE			"fitImage"
#define CONFIG_HOSTNAME			"novena"

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		0x20000000

#define CONFIG_SYS_MALLOC_LEN		(64 * 1024 * 1024)

/* SPL */
#include "imx6_spl.h"			/* common IMX6 SPL configuration */

/* Ethernet Configuration */
#ifdef CONFIG_CMD_NET
#define CONFIG_FEC_MXC
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		0x7
#define CONFIG_ARP_TIMEOUT		200UL
#endif

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_I2C_MULTI_BUS
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_SPD_BUS_NUM		0

/* I2C EEPROM */
#ifdef CONFIG_CMD_EEPROM
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	2
#define CONFIG_SYS_I2C_EEPROM_BUS	2
#endif

/* MMC Configs */
#define CONFIG_SYS_FSL_ESDHC_ADDR	0
#define CONFIG_SYS_FSL_USDHC_NUM	2

/* PCI express */
#ifdef CONFIG_CMD_PCI
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCIE_IMX
#define CONFIG_PCIE_IMX_PERST_GPIO	IMX_GPIO_NR(3, 29)
#define CONFIG_PCIE_IMX_POWER_GPIO	IMX_GPIO_NR(7, 12)
#endif

/* PMIC */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE100
#define CONFIG_POWER_PFUZE100_I2C_ADDR	0x08

/* SATA Configs */
#define CONFIG_LBA48

/* UART */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART2_BASE

/* USB Configs */
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
/* Gadget part */
#define CONFIG_USBD_HS
#define CONFIG_NETCONSOLE
#endif

/* Video output */
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_SPLASH_SCREEN
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_LOGO
#define CONFIG_IMX_HDMI
#define CONFIG_IMX_VIDEO_SKIP

/* Extra U-Boot environment. */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS					\
	"fdt_high=0xffffffff\0"						\
	"initrd_high=0xffffffff\0"					\
	"consdev=ttymxc1\0"						\
	"baudrate=115200\0"						\
	"bootdev=/dev/mmcblk0p1\0"					\
	"rootdev=/dev/mmcblk0p2\0"					\
	"netdev=eth0\0"							\
	"kernel_addr_r="__stringify(CONFIG_LOADADDR)"\0"		\
	"pxefile_addr_r="__stringify(CONFIG_LOADADDR)"\0"		\
	"scriptaddr="__stringify(CONFIG_LOADADDR)"\0"			\
	"ramdisk_addr_r=0x28000000\0"		   			\
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

#else
#define CONFIG_EXTRA_ENV_SETTINGS
#endif /* CONFIG_SPL_BUILD */

#endif				/* __CONFIG_H */
