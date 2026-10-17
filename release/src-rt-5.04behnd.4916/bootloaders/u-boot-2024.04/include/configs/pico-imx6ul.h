/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015 Technexion Ltd.
 *
 * Configuration settings for the Technexion PICO-IMX6UL-EMMC board.
 */
#ifndef __PICO_IMX6UL_CONFIG_H
#define __PICO_IMX6UL_CONFIG_H


#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include "mx6_common.h"
#include <asm/mach-imx/gpio.h>

/* Network support */

#define CFG_FEC_MXC_PHYADDR		0x1

#define CFG_MXC_UART_BASE		UART6_BASE_ADDR

/* MMC Configs */
#define CFG_SYS_FSL_ESDHC_ADDR	USDHC1_BASE_ADDR

/* USB Configs */
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0

#define DFU_DEFAULT_POLL_TIMEOUT 300

#define CFG_DFU_ENV_SETTINGS \
	"dfu_alt_info=" \
		"spl raw 0x2 0x400;" \
		"u-boot raw 0x8a 0x400;" \
		"/boot/zImage ext4 0 1;" \
		"/boot/imx6ul-pico-hobbit.dtb ext4 0 1;" \
		"/boot/imx6ul-pico-pi.dtb ext4 0 1;" \
		"rootfs part 0 1\0" \

#define BOOTMENU_ENV \
	"bootmenu_0=Boot using PICO-Dwarf baseboard=" \
		"setenv fdtfile imx6ul-pico-dwarf.dtb\0" \
	"bootmenu_1=Boot using PICO-Hobbit baseboard=" \
		"setenv fdtfile imx6ul-pico-hobbit.dtb\0" \
	"bootmenu_2=Boot using PICO-Pi baseboard=" \
		"setenv fdtfile imx6ul-pico-pi.dtb\0" \

#define CFG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"splashpos=m,m\0" \
	"console=ttymxc5\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdtfile=" CONFIG_DEFAULT_FDT_FILE "\0" \
	"videomode=video=ctfb:x:800,y:480,depth:24,mode:0,pclk:30000,le:46,ri:210,up:22,lo:23,hs:20,vs:10,sync:0,vmode:0\0" \
	BOOTMENU_ENV \
	"fdt_addr=0x83000000\0" \
	"fdt_addr_r=0x83000000\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"pxefile_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"ramdisk_addr_r=0x83000000\0" \
	"ramdiskaddr=0x83000000\0" \
	"scriptaddr=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"mmcautodetect=yes\0" \
	CFG_DFU_ENV_SETTINGS \
	"findfdt=" \
		"if test $fdtfile = ask ; then " \
			"bootmenu -1; fi;" \
		"if test $fdtfile != ask ; then " \
			"saveenv; fi;\0" \
	"finduuid=part uuid mmc 0:1 uuid\0" \
	"partitions=" \
		"uuid_disk=${uuid_gpt_disk};" \
		"name=rootfs,size=0,uuid=${uuid_gpt_rootfs}\0" \
	"fastboot_partition_alias_system=rootfs\0" \
	"setup_emmc=mmc dev 0; gpt write mmc 0 $partitions; reset;\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(PXE, pxe, na) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>
#include <linux/stringify.h>

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#ifdef CONFIG_VIDEO
#define MXS_LCDIF_BASE MX6UL_LCDIF1_BASE_ADDR
#endif

#endif /* __PICO_IMX6UL_CONFIG_H */
