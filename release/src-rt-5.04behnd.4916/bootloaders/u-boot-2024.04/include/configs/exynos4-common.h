/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2014 Samsung Electronics
 *
 * Configuration settings for the SAMSUNG EXYNOS5 board.
 */

#ifndef __CONFIG_EXYNOS4_COMMON_H
#define __CONFIG_EXYNOS4_COMMON_H

#include "exynos-common.h"

#define DFU_DEFAULT_POLL_TIMEOUT 300

/* Common environment variables */
#define ENV_ITB \
	"loadkernel=load mmc ${mmcbootdev}:${mmcbootpart} ${kerneladdr} " \
		"${kernelname}\0" \
	"loadinitrd=load mmc ${mmcbootdev}:${mmcbootpart} ${initrdaddr} " \
		"${initrdname}\0" \
	"loaddtb=load mmc ${mmcbootdev}:${mmcbootpart} ${fdtaddr} " \
		"${fdtfile}\0" \
	"check_ramdisk=" \
		"if run loadinitrd; then " \
			"setenv initrd_addr ${initrdaddr};" \
		"else " \
			"setenv initrd_addr -;" \
		"fi;\0" \
	"check_dtb=" \
		"if run loaddtb; then " \
			"setenv fdt_addr ${fdtaddr};" \
		"else " \
			"setenv fdt_addr;" \
		"fi;\0" \
	"kernel_args=" \
		"setenv bootargs root=/dev/mmcblk${mmcdev}p${mmcrootpart}" \
		" ${lpj} rootwait ${console} ${meminfo} ${opts} ${lcdinfo};\0" \
	"boot_fit=" \
		"setenv kerneladdr 0x42000000;" \
		"setenv kernelname Image.itb;" \
		"run loadkernel;" \
		"run kernel_args;" \
		"bootm ${kerneladdr}#${board_name}\0" \
	"boot_uimg=" \
		"setenv kerneladdr 0x40007FC0;" \
		"setenv kernelname uImage;" \
		"run check_dtb;" \
		"run check_ramdisk;" \
		"run loadkernel;" \
		"run kernel_args;" \
		"bootm ${kerneladdr} ${initrd_addr} ${fdt_addr};\0" \
	"boot_zimg=" \
		"setenv kerneladdr 0x40007FC0;" \
		"setenv kernelname zImage;" \
		"run check_dtb;" \
		"run check_ramdisk;" \
		"run loadkernel;" \
		"run kernel_args;" \
		"bootz ${kerneladdr} ${initrd_addr} ${fdt_addr};\0" \
	"autoboot=" \
		"if test -e mmc ${mmcdev}:${mmcbootpart} Image.itb; then; " \
			"run boot_fit;" \
		"elif test -e mmc ${mmcdev}:${mmcbootpart} zImage; then; " \
			"run boot_zimg;" \
		"elif test -e mmc ${mmcdev}:${mmcbootpart} uImage; then; " \
			"run boot_uimg;" \
		"fi;\0"

#endif	/* __CONFIG_EXYNOS4_COMMON_H */
