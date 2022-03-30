/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Config file for Compulab CM-FX6 board
 *
 * Copyright (C) 2014, Compulab Ltd - http://compulab.co.il/
 *
 * Author: Nikita Kiryanov <nikita@compulab.co.il>
 */

#ifndef __CONFIG_CM_FX6_H
#define __CONFIG_CM_FX6_H

#include "mx6_common.h"

/* Machine config */
#define CONFIG_SYS_LITTLE_ENDIAN
#define CONFIG_MACH_TYPE		4273

/* MMC */
#define CONFIG_SYS_FSL_USDHC_NUM	3
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

/* RAM */
#define PHYS_SDRAM_1			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_2			MMDC1_ARB_BASE_ADDR
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_START	0x10000000
#define CONFIG_SYS_MEMTEST_END		0x10010000
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Serial console */
#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART4_BASE
#define CONFIG_SYS_BAUDRATE_TABLE	{9600, 19200, 38400, 57600, 115200}

/* SPI flash */

/* MTD support */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SPI_FLASH_MTD
#endif

/* Environment */
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#define CONFIG_ENV_SIZE			(8 * 1024)
#define CONFIG_ENV_OFFSET		(768 * 1024)

#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_addr_r=0x18000000\0" \
	"ramdisk_addr_r=0x13000000\0" \
	"kernel_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"pxefile_addr_r=" __stringify(CONFIG_LOADADDR) "\0" \
	"scriptaddr=" __stringify(CONFIG_LOADADDR) "\0" \
	"fdtfile=undefined\0" \
	"stdin=serial,usbkbd\0" \
	"stdout=serial,vga\0" \
	"stderr=serial,vga\0" \
	"panel=HDMI\0" \
	"autoload=no\0" \
	"uImage=uImage-cm-fx6\0" \
	"zImage=zImage-cm-fx6\0" \
	"kernel=uImage-cm-fx6\0" \
	"dtb=cm-fx6.dtb\0" \
	"console=ttymxc3,115200\0" \
	"ethprime=FEC0\0" \
	"video_hdmi=mxcfb0:dev=hdmi,1920x1080M-32@50,if=RGB32\0" \
	"video_dvi=mxcfb0:dev=dvi,1280x800M-32@50,if=RGB32\0" \
	"doboot=bootm ${kernel_addr_r}\0" \
	"doloadfdt=false\0" \
	"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"setboottypez=setenv kernel ${zImage};" \
		"setenv doboot bootz ${kernel_addr_r} - ${fdt_addr_r};" \
		"setenv doloadfdt true;\0" \
	"setboottypem=setenv kernel ${uImage};" \
		"setenv doboot bootm ${kernel_addr_r};" \
		"setenv doloadfdt false;\0"\
	"mmcroot=/dev/mmcblk0p2 rw rootwait\0" \
	"sataroot=/dev/sda2 rw rootwait\0" \
	"nandroot=/dev/mtdblock4 rw\0" \
	"nandrootfstype=ubifs\0" \
	"mmcargs=setenv bootargs console=${console} root=${mmcroot} " \
		"${video} ${extrabootargs}\0" \
	"sataargs=setenv bootargs console=${console} root=${sataroot} " \
		"${video} ${extrabootargs}\0" \
	"nandargs=setenv bootargs console=${console} " \
		"root=${nandroot} " \
		"rootfstype=${nandrootfstype} " \
		"${video} ${extrabootargs}\0" \
	"nandboot=if run nandloadkernel; then " \
			"run nandloadfdt;" \
			"run setboottypem;" \
			"run storagebootcmd;" \
			"run setboottypez;" \
			"run storagebootcmd;" \
		"fi;\0" \
	"run_eboot=echo Starting EBOOT ...; "\
		"mmc dev 2 && " \
		"mmc rescan && mmc read 10042000 a 400 && go 10042000\0" \
	"loadkernel=load ${storagetype} ${storagedev} ${kernel_addr_r} ${kernel};\0"\
	"loadfdt=load ${storagetype} ${storagedev} ${fdt_addr_r} ${dtb};\0" \
	"nandloadkernel=nand read ${kernel_addr_r} 0 780000;\0" \
	"nandloadfdt=nand read ${fdt_addr_r} 780000 80000;\0" \
	"setupmmcboot=setenv storagetype mmc; setenv storagedev 2;\0" \
	"setupsataboot=setenv storagetype sata; setenv storagedev 0;\0" \
	"setupnandboot=setenv storagetype nand;\0" \
	"storagebootcmd=echo Booting from ${storagetype} ...;" \
			"run ${storagetype}args; run doboot;\0" \
	"trybootk=if run loadkernel; then " \
		"if ${doloadfdt}; then " \
			"run loadfdt;" \
		"fi;" \
		"run storagebootcmd;" \
		"fi;\0" \
	"trybootsmz=" \
		"run setboottypem;" \
		"run trybootk;" \
		"run setboottypez;" \
		"run trybootk;\0" \
	"legacy_bootcmd=" \
		"run setupmmcboot;" \
		"mmc dev ${storagedev};" \
		"if mmc rescan; then " \
			"run trybootsmz;" \
		"fi;" \
		"run setupsataboot;" \
		"if sata init; then " \
			"run trybootsmz;" \
		"fi;" \
		"run setupnandboot;" \
		"run nandboot;\0" \
	"findfdt="\
		"if test $board_name = Utilite && test $board_rev = MX6Q ; then " \
			"setenv fdtfile imx6q-utilite-pro.dtb; fi; " \
		"if test $fdtfile = undefined; then " \
			"echo WARNING: Could not determine dtb to use; fi; \0" \
	BOOTENV

#define CONFIG_PREBOOT		"usb start;sf probe"

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 2) \
	func(SATA, sata, 0)

#include <config_distro_bootcmd.h>
#else
#define CONFIG_EXTRA_ENV_SETTINGS
#endif

/* NAND */
#ifndef CONFIG_SPL_BUILD
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_MAX_CHIPS	1
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_ONFI_DETECTION
/* APBH DMA is required for NAND support */
#endif

/* Ethernet */
#define CONFIG_FEC_MXC
#define CONFIG_FEC_MXC_PHYADDR		0
#define CONFIG_FEC_XCV_TYPE		RGMII
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_PHY_ATHEROS
#define CONFIG_ETHPRIME			"FEC0"
#define CONFIG_ARP_TIMEOUT		200UL
#define CONFIG_NET_RETRY_COUNT		5

/* USB */
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET	/* For OTG port */

/* I2C */
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C2		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_MXC_I2C3		/* enable I2C bus 3 */
#define CONFIG_SYS_I2C_SPEED		100000
#define CONFIG_SYS_MXC_I2C3_SPEED	400000

#define CONFIG_SYS_I2C_EEPROM_ADDR	0x50
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN	1
#define CONFIG_SYS_I2C_EEPROM_BUS	2

/* SATA */
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_LBA48
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR

/* Boot */
#define CONFIG_SYS_BOOTMAPSZ	        (8 << 20)
#define CONFIG_SERIAL_TAG

/* misc */
#define CONFIG_SYS_MALLOC_LEN			(10 * 1024 * 1024)

/* SPL */
#include "imx6_spl.h"
#define CONFIG_SYS_SPI_U_BOOT_OFFS	(64 * 1024)

/* Display */
#define CONFIG_IMX_HDMI

#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SOURCE
#define CONFIG_VIDEO_BMP_RLE8

#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO

/* EEPROM */
#define CONFIG_ENV_EEPROM_IS_ON_I2C
#define CONFIG_SYS_I2C_EEPROM_ADDR_LEN		1
#define CONFIG_SYS_EEPROM_PAGE_WRITE_BITS	4
#define CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS	5
#define CONFIG_SYS_EEPROM_SIZE			256

#endif	/* __CONFIG_CM_FX6_H */
