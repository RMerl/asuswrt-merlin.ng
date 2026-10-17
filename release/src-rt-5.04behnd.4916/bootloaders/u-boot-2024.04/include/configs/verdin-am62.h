/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Configuration header file for Verdin AM62 SoM
 *
 * Copyright 2023 Toradex - https://www.toradex.com/
 */

#ifndef __VERDIN_AM62_H
#define __VERDIN_AM62_H

#define RAMDISK_ADDR_R		0x90300000
#define SCRIPTADDR		0x90280000

/* DDR Configuration */
#define CFG_SYS_SDRAM_BASE	0x80000000
#define CFG_SYS_SDRAM_SIZE	SZ_2G /* Maximum supported size, auto-detection is used */

#define MEM_LAYOUT_ENV_SETTINGS \
	"fdt_addr_r=0x90200000\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernel_comp_addr_r=0x80200000\0" \
	"kernel_comp_size=0x08000000\0" \
	"ramdisk_addr_r=" __stringify(RAMDISK_ADDR_R) "\0" \
	"scriptaddr=" __stringify(SCRIPTADDR) "\0"

#if CONFIG_TARGET_VERDIN_AM62_A53
/* Enable Distro Boot */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>
#else /* CONFIG_TARGET_VERDIN_AM62_A53 */
#define BOOTENV \
	""
#endif /* CONFIG_TARGET_VERDIN_AM62_A53 */

/* Incorporate settings into the U-Boot environment */
#define CFG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	MEM_LAYOUT_ENV_SETTINGS \
	"boot_scripts=boot.scr\0" \
	"boot_script_dhcp=boot.scr\0" \
	"console=ttyS2\0" \
	"fdt_board=dev\0" \
	"setup=setenv setupargs console=tty1 console=${console},${baudrate} " \
		"consoleblank=0 earlycon=ns16550a,mmio32,0x02800000\0" \
	"update_tiboot3=askenv confirm Did you load tiboot3.bin (y/N)?; " \
		"if test \"$confirm\" = \"y\"; then " \
		"setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt " \
		"${blkcnt} / 0x200; mmc dev 0 1; mmc write ${loadaddr} 0x0 " \
		"${blkcnt}; fi\0" \
	"update_tispl=askenv confirm Did you load tispl.bin (y/N)?; " \
		"if test \"$confirm\" = \"y\"; then " \
		"setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt " \
		"${blkcnt} / 0x200; mmc dev 0 1; mmc write ${loadaddr} 0x400 " \
		"${blkcnt}; fi\0" \
	"update_uboot=askenv confirm Did you load u-boot.img (y/N)?; " \
		"if test \"$confirm\" = \"y\"; then " \
		"setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt " \
		"${blkcnt} / 0x200; mmc dev 0 1; mmc write ${loadaddr} 0x1400 " \
		"${blkcnt}; fi\0"

#endif /* __VERDIN_AM62_H */
