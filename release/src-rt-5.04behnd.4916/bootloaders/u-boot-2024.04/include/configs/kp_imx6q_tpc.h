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

/* Miscellaneous configurable options */

/* FEC ethernet */

/* USB Configs */
#ifdef CONFIG_CMD_USB
#define CFG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CFG_MXC_USB_FLAGS		0
#endif

#define CFG_EXTRA_ENV_SETTINGS	\
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
	"uboot_file=u-boot.img\0" \
	"SPL_file=SPL\0" \
	"wic_file=kp-image-kpimx6qtpc.wic\0" \
	"upd_image=st.4k\0" \
	"updargs=setenv bootargs console=${console} ${smp} ${displayargs}\0" \
	"initrd_ram_dev=/dev/ram\0" \
	"addswupdate=setenv bootargs ${bootargs} root=${initrd_ram_dev} rw\0" \
	"loadusb=usb start; " \
	       "fatload usb 0 ${loadaddr} ${upd_image}\0" \
	"upd_uboot_sd=" \
	    "if tftp ${loadaddr} ${uboot_file}; then " \
	       "setexpr blkc ${filesize} / 0x200;" \
	       "setexpr blkc ${blkc} + 1;" \
	       "mmc write ${loadaddr} 0x8A ${blkc};" \
	    "fi;\0" \
	"upd_SPL_sd=" \
	    "if tftp ${loadaddr} ${SPL_file}; then " \
	       "setexpr blkc ${filesize} / 0x200;" \
	       "setexpr blkc ${blkc} + 1;" \
	       "mmc write ${loadaddr} 0x2 ${blkc};" \
	    "fi;\0" \
	"upd_SPL_mmc=mmc dev 1; mmc partconf 1 0 1 1; run upd_SPL_sd\0" \
	"upd_uboot_mmc=mmc dev 1; mmc partconf 1 0 1 1; run upd_uboot_sd\0" \
	"up_mmc=run upd_SPL_mmc; run upd_uboot_mmc\0" \
	"up_sd=run upd_SPL_sd; run upd_uboot_sd\0" \
	"upd_wic=" \
	    "if tftp ${loadaddr} ${wic_file}; then " \
	       "setexpr blkc ${filesize} / 0x200;" \
	       "setexpr blkc ${blkc} + 1;" \
	       "mmc write ${loadaddr} 0x0 ${blkc};" \
	    "fi;\0" \
	"usbupd=echo Booting update from usb ...; " \
	       "setenv bootargs; " \
	       "run updargs; " \
	       "run addinitrd; " \
	       "run addswupdate; " \
	       "run loadusb; " \
	       "bootm ${loadaddr}#${fit_config}\0" \
	BOOTENV

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 0) \
	func(MMC, mmc, 1) \
	func(USB, usb, 0) \
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* Environment */

#endif	/* __KP_IMX6Q_TPC_IMX6_CONFIG_H_ */
