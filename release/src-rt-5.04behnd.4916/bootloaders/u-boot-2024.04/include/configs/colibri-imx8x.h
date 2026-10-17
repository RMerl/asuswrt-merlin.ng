/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Toradex
 */

#ifndef __COLIBRI_IMX8X_H
#define __COLIBRI_IMX8X_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define MEM_LAYOUT_ENV_SETTINGS \
	"fdt_addr_r=0x9d400000\0" \
	"kernel_addr_r=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"kernel_comp_addr_r=0xb0000000\0" \
	"kernel_comp_size=0x08000000\0" \
	"ramdisk_addr_r=0x9d500000\0" \
	"scriptaddr=0x9d480000\0"

/* Boot M4 */
#define M4_BOOT_ENV \
	"m4_0_image=m4_0.bin\0" \
	"loadm4image_0=load mmc ${mmcdev}:${mmcpart} ${loadaddr} ${m4_0_image}\0" \
	"m4boot_0=run loadm4image_0; dcache flush; bootaux ${loadaddr} 0\0"

/* Enable Distro Boot */
#define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, 1) \
	func(MMC, mmc, 0) \
	func(DHCP, dhcp, na)
#include <config_distro_bootcmd.h>

/* Initial environment variables */
#define CFG_EXTRA_ENV_SETTINGS \
	BOOTENV \
	M4_BOOT_ENV \
	MEM_LAYOUT_ENV_SETTINGS \
	"boot_script_dhcp=boot.scr\0" \
	"console=ttyLP3\0" \
	"fdt_board=eval-v3\0" \
	"initrd_addr=0x83800000\0" \
	"initrd_high=0xffffffffffffffff\0" \
	"setup=setenv setupargs console=tty1 console=${console},${baudrate} " \
		"consoleblank=0 earlycon\0" \
	"update_uboot=askenv confirm Did you load flash.bin resp. u-boot-dtb.imx (y/N)?; " \
		"if test \"$confirm\" = \"y\"; then " \
		"setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt " \
		"${blkcnt} / 0x200; mmc dev 0 1; mmc write ${loadaddr} 0x0 " \
		"${blkcnt}; fi\0"

#define CFG_SYS_SDRAM_BASE		0x80000000
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_2			0x880000000
#define PHYS_SDRAM_1_SIZE		SZ_2G		/* 2 GB */
#define PHYS_SDRAM_2_SIZE		0x00000000	/* 0 GB */

#endif /* __COLIBRI_IMX8X_H */
