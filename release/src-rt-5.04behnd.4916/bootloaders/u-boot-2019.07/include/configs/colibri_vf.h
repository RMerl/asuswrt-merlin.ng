/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015-2019 Toradex, Inc.
 *
 * Configuration settings for the Toradex VF50/VF61 modules.
 *
 * Based on vf610twr.h:
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define CONFIG_SYS_FSL_CLK

#define CONFIG_SKIP_LOWLEVEL_INIT

#ifdef CONFIG_VIDEO_FSL_DCU_FB
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_SYS_FSL_DCU_LE

#define CONFIG_SYS_DCU_ADDR		DCU0_BASE_ADDR
#define DCU_LAYER_MAX_NUM		64
#endif

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * SZ_1M)

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

/* NAND support */
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_MAX_NAND_DEVICE	1

#define CONFIG_IPADDR		192.168.10.2
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_SERVERIP		192.168.10.1

#define CONFIG_LOADADDR			0x80008000
#define CONFIG_FDTADDR			0x84000000

/* We boot from the gfxRAM area of the OCRAM. */
#define CONFIG_BOARD_SIZE_LIMIT		520192

#define MEM_LAYOUT_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"fdt_addr_r=0x82000000\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"kernel_addr_r=0x81000000\0" \
	"pxefile_addr_r=0x87100000\0" \
	"ramdisk_addr_r=0x82100000\0" \
	"scriptaddr=0x87000000\0"

#define NFS_BOOTCMD \
	"nfsargs=ip=:::::eth0: root=/dev/nfs\0"	\
	"nfsboot=run setup; " \
	"setenv bootargs ${defargs} ${nfsargs} ${mtdparts} " \
	"${setupargs} ${vidargs}; echo Booting from NFS...;" \
	"dhcp ${kernel_addr_r} && "	\
	"tftp ${fdt_addr_r} ${soc}-colibri-${fdt_board}.dtb && " \
	"run fdt_fixup && bootz ${kernel_addr_r} - ${fdt_addr_r}\0" \

#define SD_BOOTCMD \
	"set_sdargs=setenv sdargs root=PARTUUID=${uuid} ro rootwait\0"	\
	"sdboot=run setup; run sdfinduuid; run set_sdargs; " \
	"setenv bootargs ${defargs} ${sdargs} ${mtdparts} " \
	"${setupargs} ${vidargs}; echo Booting from MMC/SD card...; " \
	"load mmc ${sddev}:${sdbootpart} ${kernel_addr_r} ${kernel_file} && " \
	"load mmc ${sddev}:${sdbootpart} ${fdt_addr_r} " \
		"${soc}-colibri-${fdt_board}.dtb && " \
	"run fdt_fixup && bootz ${kernel_addr_r} - ${fdt_addr_r}\0" \
	"sdbootpart=1\0" \
	"sddev=0\0" \
	"sdfinduuid=part uuid mmc ${sddev}:${sdrootpart} uuid\0" \
	"sdrootpart=2\0"


#define UBI_BOOTCMD \
	"ubiargs=ubi.mtd=ubi root=ubi0:rootfs rootfstype=ubifs " \
	"ubi.fm_autoconvert=1\0" \
	"ubiboot=run setup; " \
	"setenv bootargs ${defargs} ${ubiargs} ${mtdparts} "   \
	"${setupargs} ${vidargs}; echo Booting from NAND...; " \
	"ubi part ubi && " \
	"ubi read ${kernel_addr_r} kernel && " \
	"ubi read ${fdt_addr_r} dtb && " \
	"run fdt_fixup && bootz ${kernel_addr_r} - ${fdt_addr_r}\0" \

#define CONFIG_BOOTCOMMAND "run ubiboot; " \
	"setenv fdtfile ${soc}-colibri-${fdt_board}.dtb && run distro_bootcmd;"

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#undef BOOTENV_RUN_NET_USB_START
#define BOOTENV_RUN_NET_USB_START ""

#define DFU_ALT_NAND_INFO "vf-bcb part 0,1;u-boot part 0,2;ubi part 0,4"

#define CONFIG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	MEM_LAYOUT_ENV_SETTINGS \
	NFS_BOOTCMD \
	SD_BOOTCMD \
	UBI_BOOTCMD \
	"console=ttyLP0\0" \
	"defargs=user_debug=30\0" \
	"dfu_alt_info=" DFU_ALT_NAND_INFO "\0" \
	"fdt_board=eval-v3\0" \
	"fdt_fixup=;\0" \
	"kernel_file=zImage\0" \
	"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
	"setsdupdate=mmc rescan && set interface mmc && " \
		"fatload ${interface} 0:1 ${loadaddr} flash_blk.img && " \
		"source ${loadaddr}\0" \
	"setup=setenv setupargs console=tty1 console=${console}" \
		",${baudrate}n8 ${memargs}\0" \
	"setupdate=run setsdupdate || run setusbupdate\0" \
	"setusbupdate=usb start && set interface usb && " \
		"fatload ${interface} 0:1 ${loadaddr} flash_blk.img && " \
		"source ${loadaddr}\0" \
	"splashpos=m,m\0" \
	"video-mode=dcufb:640x480-16@60,monitor=lcd\0"

/* Miscellaneous configurable options */
#define CONFIG_SYS_CBSIZE		1024	/* Console I/O Buffer Size */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

#define CONFIG_SYS_MEMTEST_START	0x80010000
#define CONFIG_SYS_MEMTEST_END		0x87C00000

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

/* Physical memory map */
#define PHYS_SDRAM			(0x80000000)
#define PHYS_SDRAM_SIZE			(256 * SZ_1M)

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* Environment organization */
#ifdef CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_SIZE			(64 * 2048)
#define CONFIG_ENV_RANGE		(4 * 64 * 2048)
#define CONFIG_ENV_OFFSET		(12 * 64 * 2048)
#endif

/* USB Host Support */
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET

/* USB DFU */
#define CONFIG_SYS_DFU_DATA_BUF_SIZE (SZ_1M)

#endif /* __CONFIG_H */
