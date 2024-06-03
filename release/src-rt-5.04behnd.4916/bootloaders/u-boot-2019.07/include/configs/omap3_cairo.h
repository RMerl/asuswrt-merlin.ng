/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the QUIPOS Cairo board.
 *
 * Copyright (C) DENX GmbH
 *
 * Author :
 *	Albert ARIBAUD <albert.aribaud@3adev.fr>
 *
 * Derived from EVM  code by
 *	Manikandan Pillai <mani.pillai@ti.com>
 * Itself derived from Beagle Board and 3430 SDP code by
 *	Richard Woodruff <r-woodruff2@ti.com>
 *	Syed Mohammed Khasim <khasim@ti.com>
 *
 * Also derived from include/configs/omap3_beagle.h
 */

#ifndef __OMAP3_CAIRO_CONFIG_H
#define __OMAP3_CAIRO_CONFIG_H

/*
 * 1MB into the SDRAM to allow for SPL's bss at the beginning of SDRAM
 * 64 bytes before this address should be set aside for u-boot.img's
 * header. That is 0x800FFFC0--0x80100000 should not be used for any
 * other needs.  We use this rather than the inherited defines from
 * ti_armv7_common.h for backwards compatibility.
 */
#define CONFIG_SYS_UBOOT_START		CONFIG_SYS_TEXT_BASE
#define CONFIG_SPL_BSS_START_ADDR	0x80000000
#define CONFIG_SPL_BSS_MAX_SIZE		(512 << 10)	/* 512 KB */
#define CONFIG_SYS_SPL_MALLOC_START	0x80208000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x100000

#include <configs/ti_omap3_common.h>

#define CONFIG_REVISION_TAG		1
#define CONFIG_ENV_OVERWRITE

/* Enable Multi Bus support for I2C */
#define CONFIG_I2C_MULTI_BUS		1

/* Probe all devices */
#define CONFIG_SYS_I2C_NOPROBES		{ {0x0, 0x0} }

/*
 * TWL4030
 */

/*
 * Board NAND Info.
 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1		/* Max number of NAND */
							/* devices */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"machid=ffffffff\0" \
	"fdt_high=0x87000000\0" \
	"baudrate=115200\0" \
	"fec_addr=00:50:C2:7E:90:F0\0" \
	"netmask=255.255.255.0\0" \
	"ipaddr=192.168.2.9\0" \
	"gateway=192.168.2.1\0" \
	"serverip=192.168.2.10\0" \
	"nfshost=192.168.2.10\0" \
	"stdin=serial\0" \
	"stdout=serial\0" \
	"stderr=serial\0" \
	"bootargs_mmc_ramdisk=mem=128M " \
		"console=ttyO1,115200n8 " \
		"root=/dev/ram0 rw " \
		"initrd=0x81600000,16M " \
		"mpurate=600 ramdisk_size=16384 omapfb.rotate=1 " \
		"omapfb.rotate_type=1 omap_vout.vid1_static_vrfb_alloc=y\0" \
	"mmcboot=mmc init; " \
		"fatload mmc 0 0x80000000 uImage; " \
		"fatload mmc 0 0x81600000 ramdisk.gz; " \
		"setenv bootargs ${bootargs_mmc_ramdisk}; " \
		"bootm 0x80000000\0" \
	"bootargs_nfs=mem=99M console=ttyO0,115200n8 noinitrd rw ip=dhcp " \
	"root=/dev/nfs " \
	"nfsroot=192.168.2.10:/home/spiid/workdir/Quipos/rootfs,nolock " \
	"mpurate=600 omapfb.rotate=1 omapfb.rotate_type=1 " \
	"omap_vout.vid1_static_vrfb_alloc=y\0" \
	"boot_nfs=run get_kernel; setenv bootargs ${bootargs_nfs}; " \
	"bootm 0x80000000\0" \
	"bootargs_nand=mem=128M console=ttyO1,115200n8 noinitrd " \
	"root=/dev/mtdblock4 rw rootfstype=jffs2 mpurate=600 " \
	"omap_vout.vid1_static_vrfb_alloc=y omapfb.rotate=1 " \
	"omapfb.rotate_type=1\0" \
	"boot_nand=nand read.i 0x80000000 280000 300000; setenv " \
	"bootargs ${bootargs_nand}; bootm 0x80000000\0" \
	"ledorange=i2c dev 1; i2c mw 60 00 00 1; i2c mw 60 14 FF 1; " \
	"i2c mw 60 15 FF 1; i2c mw 60 16 FF 1; i2c mw 60 17 FF 1; " \
	"i2c mw 60 09 10 1; i2c mw 60 06 10 1\0" \
	"ledgreen=i2c dev 1; i2c mw 60 00 00 1; i2c mw 60 14 FF 1; " \
	"i2c mw 60 15 FF 1; i2c mw 60 16 FF 1; i2c mw 60 17 FF 1; i2c " \
	"mw 60 09 00 1; i2c mw 60 06 10 1\0" \
	"ledoff=i2c dev 1; i2c mw 60 00 00 1; i2c mw 60 14 FF 1; " \
	"i2c mw 60 15 FF 1; i2c mw 60 16 FF 1; i2c mw 60 17 FF 1; " \
	"i2c mw 60 09 00 1; i2c mw 60 06 0 1\0" \
	"ledred=i2c dev 1; i2c mw 60 00 00 1; i2c mw 60 14 FF 1; " \
	"i2c mw 60 15 FF 1; i2c mw 60 16 FF 1; i2c mw 60 17 FF 1; " \
	"i2c mw 60 09 10 1; i2c mw 60 06 0 1\0" \
	"flash_xloader=mw.b 0x81600000 0xff 0x20000; " \
		"nand erase 0 20000; " \
		"fatload mmc 0 0x81600000 MLO; " \
		"nandecc hw; " \
		"nand write.i 0x81600000 0 20000;\0" \
	"flash_uboot=mw.b 0x81600000 0xff 0x40000; " \
		"nand erase 80000 40000; " \
		"fatload mmc 0 0x81600000 u-boot.bin; " \
		"nandecc sw; " \
		"nand write.i 0x81600000 80000 40000;\0" \
	"flash_kernel=mw.b 0x81600000 0xff 0x300000; " \
		"nand erase 280000 300000; " \
		"fatload mmc 0 0x81600000 uImage; " \
		"nandecc sw; " \
		"nand write.i 0x81600000 280000 300000;\0" \
	"flash_rootfs=fatload mmc 0 0x81600000 rootfs.jffs2; " \
		"nandecc sw; " \
		"nand write.jffs2 0x680000 0xFF ${filesize}; " \
		"nand erase 680000 ${filesize}; " \
		"nand write.jffs2 81600000 680000 ${filesize};\0" \
	"flash_scrub=nand scrub; " \
		"run flash_xloader; " \
		"run flash_uboot; " \
		"run flash_kernel; " \
		"run flash_rootfs;\0" \
	"flash_all=run ledred; " \
		"nand erase.chip; " \
		"run ledorange; " \
		"run flash_xloader; " \
		"run flash_uboot; " \
		"run flash_kernel; " \
		"run flash_rootfs; " \
		"run ledgreen; " \
		"run boot_nand; \0" \

#define CONFIG_BOOTCOMMAND \
	"if fatload mmc 0 0x81600000 MLO; then run flash_all; " \
	"else run boot_nand; fi"

/*
 * OMAP3 has 12 GP timers, they can be driven by the system clock
 * (12/13/16.8/19.2/38.4MHz) or by 32KHz clock. We use 13MHz (V_SCLK).
 * This rate is divided by a local divisor.
 */
#define CONFIG_SYS_PTV			2       /* Divisor: 2^(PTV+1) => 8 */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */
#if defined(CONFIG_CMD_NAND)
#define CONFIG_SYS_FLASH_BASE		NAND_BASE
#endif

/* Monitor at start of flash */
#define CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_FLASH_BASE
#define CONFIG_SYS_ONENAND_BASE		ONENAND_MAP

#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */
#define ONENAND_ENV_OFFSET		0x260000 /* environment starts here */

#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_OFFSET		0x260000
#define CONFIG_ENV_ADDR			0x260000

/* Defines for SPL */

/* NAND boot config */
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	0
#define CONFIG_SYS_NAND_ECCPOS		{2, 3, 4, 5, 6, 7, 8, 9,\
						10, 11, 12, 13}
#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	3
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_HAM1_CODE_HW
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000
/* NAND: SPL falcon mode configs */
#ifdef CONFIG_SPL_OS_BOOT
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS	0x280000
#endif

/* env defaults */
#define CONFIG_BOOTFILE			"uImage"

/* Provide the MACH_TYPE value the vendor kernel requires */
#define CONFIG_MACH_TYPE	3063

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/* **** PISMO SUPPORT *** */

#define CONFIG_SYS_MAX_FLASH_SECT	520	/* max number of sectors */
						/* on one chip */
#define CONFIG_SYS_MAX_FLASH_BANKS	2	/* max number of flash banks */

/*-----------------------------------------------------------------------
 * CFI FLASH driver setup
 */
/* timeout values are in ticks */
#define CONFIG_SYS_FLASH_ERASE_TOUT	(100 * CONFIG_SYS_HZ)
#define CONFIG_SYS_FLASH_WRITE_TOUT	(100 * CONFIG_SYS_HZ)

/* Flash banks JFFS2 should use */
#define CONFIG_SYS_MAX_MTD_BANKS	(CONFIG_SYS_MAX_FLASH_BANKS + \
					CONFIG_SYS_MAX_NAND_DEVICE)
#define CONFIG_SYS_JFFS2_MEM_NAND
/* use flash_info[2] */
#define CONFIG_SYS_JFFS2_FIRST_BANK	CONFIG_SYS_MAX_FLASH_BANKS
#define CONFIG_SYS_JFFS2_NUM_BANKS	1

#endif /* __OMAP3_CAIRO_CONFIG_H */
