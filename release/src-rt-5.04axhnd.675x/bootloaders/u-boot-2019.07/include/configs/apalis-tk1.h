/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Toradex, Inc.
 *
 * Configuration settings for the Toradex Apalis TK1 modules.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#include "tegra124-common.h"

#define CONFIG_ARCH_MISC_INIT

/* Board-specific serial config */
#define CONFIG_TEGRA_ENABLE_UARTA
#define CONFIG_SYS_NS16550_COM1		NV_PA_APB_UARTA_BASE

/* Environment in eMMC, before config block at the end of 1st "boot sector" */
#define CONFIG_ENV_OFFSET		(-CONFIG_ENV_SIZE + \
					 CONFIG_TDX_CFG_BLOCK_OFFSET)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_SYS_MMC_ENV_PART		1

/* PCI host support */
#undef CONFIG_PCI_SCAN_SHOW

/* PCI networking support */
#define CONFIG_E1000_NO_NVM

/* General networking support */
#define CONFIG_TFTP_TSIZE

#undef CONFIG_IPADDR
#define CONFIG_IPADDR		192.168.10.2
#define CONFIG_NETMASK		255.255.255.0
#undef CONFIG_SERVERIP
#define CONFIG_SERVERIP		192.168.10.1

#define DFU_ALT_EMMC_INFO	"apalis-tk1.img raw 0x0 0x500 mmcpart 1; " \
				"boot part 0 1 mmcpart 0; " \
				"rootfs part 0 2 mmcpart 0; " \
				"uImage fat 0 1 mmcpart 0; " \
				"tegra124-apalis-eval.dtb fat 0 1 mmcpart 0"

#define EMMC_BOOTCMD \
	"set_emmcargs=setenv emmcargs ip=off root=PARTUUID=${uuid} " \
		"ro rootfstype=ext4 rootwait\0" \
	"emmcboot=run setup; run emmcfinduuid; run set_emmcargs; " \
		"setenv bootargs ${defargs} ${emmcargs} " \
		"${setupargs} ${vidargs}; echo Booting from internal eMMC; " \
		"run emmcdtbload; " \
		"load mmc ${emmcdev}:${emmcbootpart} ${kernel_addr_r} " \
		"${boot_file} && run fdt_fixup && " \
		"bootm ${kernel_addr_r} - ${dtbparam}\0" \
	"emmcbootpart=1\0" \
	"emmcdev=0\0" \
	"emmcdtbload=setenv dtbparam; load mmc ${emmcdev}:${emmcbootpart} " \
		"${fdt_addr_r} ${soc}-apalis-${fdt_board}.dtb && " \
		"setenv dtbparam ${fdt_addr_r}\0" \
	"emmcfinduuid=part uuid mmc ${mmcdev}:${emmcrootpart} uuid\0" \
	"emmcrootpart=2\0"

#define NFS_BOOTCMD \
	"nfsargs=ip=:::::eth0:on root=/dev/nfs rw\0" \
	"nfsboot=pci enum; run setup; setenv bootargs ${defargs} ${nfsargs} " \
		"${setupargs} ${vidargs}; echo Booting via DHCP/TFTP/NFS...; " \
		"run nfsdtbload; dhcp ${kernel_addr_r} " \
		"&& run fdt_fixup && bootm ${kernel_addr_r} - ${dtbparam}\0" \
	"nfsdtbload=setenv dtbparam; tftp ${fdt_addr_r} " \
		"${soc}-apalis-${fdt_board}.dtb " \
		"&& setenv dtbparam ${fdt_addr_r}\0"

#define SD_BOOTCMD \
	"set_sdargs=setenv sdargs ip=off root=PARTUUID=${uuid} ro " \
	"rootfstype=ext4 rootwait\0" \
	"sdboot=run setup; run sdfinduuid; run set_sdargs; " \
		"setenv bootargs ${defargs} ${sdargs} ${setupargs} " \
		"${vidargs}; echo Booting from SD card in 8bit slot...; " \
		"run sddtbload; load mmc ${sddev}:${sdbootpart} " \
		"${kernel_addr_r} ${boot_file} && run fdt_fixup && " \
		"bootm ${kernel_addr_r} - ${dtbparam}\0" \
	"sdbootpart=1\0" \
	"sddev=1\0" \
	"sddtbload=setenv dtbparam; load mmc ${sddev}:${sdbootpart} " \
		"${fdt_addr_r} ${soc}-apalis-${fdt_board}.dtb " \
		"&& setenv dtbparam ${fdt_addr_r}\0" \
	"sdfinduuid=part uuid mmc ${sddev}:${sdrootpart} uuid\0" \
	"sdrootpart=2\0"

#define USB_BOOTCMD \
	"set_usbargs=setenv usbargs ip=off root=PARTUUID=${uuid} ro " \
		"rootfstype=ext4 rootwait\0" \
	"usbboot=run setup; usb start; run usbfinduuid; run set_usbargs; " \
		"setenv bootargs ${defargs} ${setupargs} " \
		"${usbargs} ${vidargs}; echo Booting from USB stick...; " \
		"run usbdtbload; load usb ${usbdev}:${usbbootpart} " \
		"${kernel_addr_r} ${boot_file} && run fdt_fixup && " \
		"bootm ${kernel_addr_r} - ${dtbparam}\0" \
	"usbbootpart=1\0" \
	"usbdev=0\0" \
	"usbdtbload=setenv dtbparam; load usb ${usbdev}:${usbbootpart} " \
		"${fdt_addr_r} ${soc}-apalis-${fdt_board}.dtb " \
		"&& setenv dtbparam ${fdt_addr_r}\0" \
	"usbfinduuid=part uuid usb ${usbdev}:${usbrootpart} uuid\0" \
	"usbrootpart=2\0"

#define BOARD_EXTRA_ENV_SETTINGS \
	"boot_file=uImage\0" \
	"console=ttyS0\0" \
	"defargs=lp0_vec=2064@0xf46ff000 core_edp_mv=1150 core_edp_ma=4000 " \
		"usb_port_owner_info=2 lane_owner_info=6 emc_max_dvfs=0\0" \
	"dfu_alt_info=" DFU_ALT_EMMC_INFO "\0" \
	EMMC_BOOTCMD \
	"fdt_board=eval\0" \
	"fdt_fixup=;\0" \
	NFS_BOOTCMD \
	SD_BOOTCMD \
	USB_BOOTCMD \
	"setethupdate=if env exists ethaddr; then; else setenv ethaddr " \
		"00:14:2d:00:00:00; fi; pci enum && tftpboot ${loadaddr} " \
		"flash_eth.img && source ${loadaddr}\0" \
	"setsdupdate=setenv interface mmc; setenv drive 1; mmc rescan; " \
		"load ${interface} ${drive}:1 ${loadaddr} flash_blk.img " \
		"|| setenv drive 2; mmc rescan; load ${interface} ${drive}:1 " \
		"${loadaddr} flash_blk.img && " \
		"source ${loadaddr}\0" \
	"setup=setenv setupargs igb_mac=${ethaddr} " \
		"consoleblank=0 no_console_suspend=1 console=tty1 " \
		"console=${console},${baudrate}n8 debug_uartport=lsport,0 " \
		"${memargs}\0" \
	"setupdate=run setsdupdate || run setusbupdate || run setethupdate\0" \
	"setusbupdate=usb start && setenv interface usb; setenv drive 0; " \
		"load ${interface} ${drive}:1 ${loadaddr} flash_blk.img && " \
		"source ${loadaddr}\0" \
	USB_BOOTCMD \
	"vidargs=video=tegrafb0:640x480-16@60 fbcon=map:1\0"

/* Increase console I/O buffer size */
#undef CONFIG_SYS_CBSIZE
#define CONFIG_SYS_CBSIZE		1024

/* Increase arguments buffer size */
#undef CONFIG_SYS_BARGSIZE
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

/* Increase maximum number of arguments */
#undef CONFIG_SYS_MAXARGS
#define CONFIG_SYS_MAXARGS		32

#define CONFIG_CMD_TIME

#define CONFIG_SYS_BOOT_RAMDISK_HIGH

#include "tegra-common-usb-gadget.h"
#include "tegra-common-post.h"

/* Reserve top 1M for secure RAM */
#define CONFIG_ARMV7_SECURE_BASE		0xfff00000
#define CONFIG_ARMV7_SECURE_RESERVE_SIZE	0x00100000

#endif /* __CONFIG_H */
