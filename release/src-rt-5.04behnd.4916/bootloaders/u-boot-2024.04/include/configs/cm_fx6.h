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

/* MMC */
#define CFG_SYS_FSL_USDHC_NUM	3
#define CFG_SYS_FSL_ESDHC_ADDR	USDHC2_BASE_ADDR

/* RAM */
#define PHYS_SDRAM_1			MMDC0_ARB_BASE_ADDR
#define PHYS_SDRAM_2			MMDC1_ARB_BASE_ADDR
#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM_1
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* Serial console */
#define CFG_MXC_UART_BASE		UART4_BASE

/* Environment */

#define CFG_EXTRA_ENV_SETTINGS \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_addr_r=0x18000000\0" \
	"ramdisk_addr_r=0x13000000\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"fdtfile=undefined\0" \
	"stdin=serial,usbkbd\0" \
	"stdout=serial,vidconsole\0" \
	"stderr=serial,vidconsole\0" \
	"panel=HDMI\0" \
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

#define BOOT_TARGET_DEVICES(func) \
	func(USB, usb, 0) \
	func(MMC, mmc, 2) \
	func(SATA, sata, 0)

#include <config_distro_bootcmd.h>

/* NAND */
#define CFG_SYS_NAND_BASE		0x40000000
/* APBH DMA is required for NAND support */

/* Ethernet */
#define CFG_FEC_MXC_PHYADDR		0

/* USB */
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0

/* Boot */
#define CFG_SYS_BOOTMAPSZ	        (8 << 20)

/* misc */

/* EEPROM */

#endif	/* __CONFIG_CM_FX6_H */
