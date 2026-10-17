/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Samsung Electronics
 * Sanghee Kim <sh0130.kim@samsung.com>
 * Piotr Wilczek <p.wilczek@samsung.com>
 * Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * Configuation settings for the Odroid-U3 (EXYNOS4412) board.
 */

#ifndef __CONFIG_ODROID_U3_H
#define __CONFIG_ODROID_U3_H

#include <configs/exynos4-common.h>

#ifndef CONFIG_SYS_L2CACHE_OFF
#define CFG_SYS_PL310_BASE	0x10502000
#endif

#define CFG_SYS_BOOTMAPSZ	0x10000000
#define CFG_SYS_SDRAM_BASE	0x40000000
#define SDRAM_BANK_SIZE		(256 << 20)	/* 256 MB */
#define PHYS_SDRAM_1		CFG_SYS_SDRAM_BASE

#include <linux/sizes.h>

/* Partitions name */
#define PARTS_BOOT		"boot"
#define PARTS_ROOT		"platform"

#define CFG_DFU_ALT \
	"uImage fat 0 1;" \
	"zImage fat 0 1;" \
	"Image.itb fat 0 1;" \
	"uInitrd fat 0 1;" \
	"exynos4412-odroidu3.dtb fat 0 1;" \
	"exynos4412-odroidx2.dtb fat 0 1;" \
	""PARTS_BOOT" part 0 1;" \
	""PARTS_ROOT" part 0 2\0" \

#define CFG_SET_DFU_ALT_BUF_LEN	(SZ_1K)

#define CFG_DFU_ALT_BOOT_EMMC \
	"u-boot raw 0x3e 0x800 mmcpart 1;" \
	"bl1 raw 0x0 0x1e mmcpart 1;" \
	"bl2 raw 0x1e 0x1d mmcpart 1;" \
	"tzsw raw 0x83e 0x138 mmcpart 1\0"

#define CFG_DFU_ALT_BOOT_SD \
	"u-boot raw 0x3f 0x800;" \
	"bl1 raw 0x1 0x1e;" \
	"bl2 raw 0x1f 0x1d;" \
	"tzsw raw 0x83f 0x138\0"

#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 2) \
	func(MMC, mmc, 0)

#include <config_distro_bootcmd.h>

/*
 * Bootable media layout:
 * dev:    SD   eMMC(part boot)
 * BL1      1    0
 * BL2     31   30
 * UBOOT   63   62
 * TZSW  2111 2110
 * ENV   2560 2560(part user)
 *
 * MBR Primary partiions:
 * Num Name   Size  Offset
 * 1.  BOOT:  100MiB 2MiB
 * 2.  ROOT:  -
*/
#define CFG_EXTRA_ENV_SETTINGS \
	"loadbootscript=load mmc ${mmcbootdev}:${mmcbootpart} ${scriptaddr} " \
		"boot.scr\0" \
	"loadkernel=load mmc ${mmcbootdev}:${mmcbootpart} ${kernel_addr_r} " \
		"${kernelname}\0" \
	"loadinitrd=load mmc ${mmcbootdev}:${mmcbootpart} ${ramdisk_addr_r} " \
		"${initrdname}\0" \
	"loaddtb=load mmc ${mmcbootdev}:${mmcbootpart} ${fdt_addr_r} " \
		"${fdtfile}\0" \
	"check_ramdisk=" \
		"if run loadinitrd; then " \
			"setenv initrd_addr ${ramdisk_addr_r};" \
		"else " \
			"setenv initrd_addr -;" \
		"fi;\0" \
	"check_dtb=" \
		"if run loaddtb; then " \
			"setenv fdt_addr ${fdt_addr_r};" \
		"else " \
			"setenv fdt_addr;" \
		"fi;\0" \
	"kernel_args=" \
		"setenv bootargs root=/dev/mmcblk${mmcrootdev}p${mmcrootpart}" \
		" rootwait ${console} ${opts}\0" \
	"boot_script=" \
		"run loadbootscript;" \
		"source ${scriptaddr}\0" \
	"boot_fit=" \
		"setenv kernelname Image.itb;" \
		"run loadkernel;" \
		"run kernel_args;" \
		"bootm ${kernel_addr_r}#${board_name}\0" \
	"boot_uimg=" \
		"setenv kernelname uImage;" \
		"run check_dtb;" \
		"run check_ramdisk;" \
		"run loadkernel;" \
		"run kernel_args;" \
		"bootm ${kernel_addr_r} ${initrd_addr} ${fdt_addr};\0" \
	"boot_zimg=" \
		"setenv kernelname zImage;" \
		"run check_dtb;" \
		"run check_ramdisk;" \
		"run loadkernel;" \
		"run kernel_args;" \
		"bootz ${kernel_addr_r} ${initrd_addr} ${fdt_addr};\0" \
	"autoboot=" \
		"if test -e mmc ${mmcbootdev} boot.scr; then; " \
			"run boot_script; " \
		"elif test -e mmc ${mmcbootdev} Image.itb; then; " \
			"run boot_fit;" \
		"elif test -e mmc ${mmcbootdev} zImage; then; " \
			"run boot_zimg;" \
		"elif test -e mmc ${mmcbootdev} uImage; then; " \
			"run boot_uimg;" \
		"fi;\0" \
	"console=console=ttySAC1,115200n8\0" \
	"mmcbootdev=0\0" \
	"mmcbootpart=1\0" \
	"mmcrootdev=0\0" \
	"mmcrootpart=2\0" \
	"dfu_alt_system="CFG_DFU_ALT \
	"dfu_alt_info=Please reset the board\0" \
	"consoleon=set console console=ttySAC1,115200n8; save; reset\0" \
	"consoleoff=set console console=ram; save; reset\0" \
	"initrdname=uInitrd\0" \
	"ramdisk_addr_r=0x42000000\0" \
	"scriptaddr=0x42000000\0" \
	"fdt_addr_r=0x40800000\0" \
	"kernel_addr_r=0x41000000\0" \
	BOOTENV

#endif	/* __CONFIG_H */
