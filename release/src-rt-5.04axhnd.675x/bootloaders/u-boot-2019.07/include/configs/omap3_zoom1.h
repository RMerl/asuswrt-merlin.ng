/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 * Nishanth Menon <nm@ti.com>
 *
 * Configuration settings for the TI OMAP3430 Zoom MDK board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/cpu.h>		/* get chip and board defs */
#include <asm/arch/omap.h>
#include <configs/ti_omap3_common.h>

/* Remove SPL boot option - we do not support that on LDP yet */

/* Generic NAND definition conflicts with debug_base */
#undef CONFIG_SYS_NAND_BASE

#define CONFIG_REVISION_TAG		1

#define CONFIG_ENV_SIZE			(128 << 10)	/* 128 KiB */

/*
 * Hardware drivers
 */

/* USB device configuration */
#define CONFIG_USB_DEVICE		1
#define CONFIG_USB_TTY			1
/* Change these to suit your needs */
#define CONFIG_USBD_VENDORID		0x0451
#define CONFIG_USBD_PRODUCTID		0x5678
#define CONFIG_USBD_MANUFACTURER	"Texas Instruments"
#define CONFIG_USBD_PRODUCT_NAME	"Zoom1"

#if defined(CONFIG_CMD_NAND)
/* NAND: SPL falcon mode configs */
#ifdef CONFIG_SPL_OS_BOOT
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS	0x280000
#endif
#endif

/*
 * TWL4030
 */

/*
 * Board NAND Info.
 */
#define CONFIG_SYS_NAND_BASE		NAND_BASE	/* physical address */
							/* to access nand at */
							/* CS0 */

/* Environment information */

#define CONFIG_EXTRA_ENV_SETTINGS \
	"loadaddr=0x82000000\0" \
	"fdtaddr=0x80f80000\0" \
	"bootfile=uImage\0" \
	"fdtfile=omap3-ldp.dtb\0" \
	"bootdir=/\0" \
	"bootpart=0:1\0" \
	"usbtty=cdc_acm\0" \
	"console=ttyO2,115200n8\0" \
	"mmcdev=0\0" \
	"videomode=1024x768@60,vxres=1024,vyres=768\0" \
	"videospec=omapfb:vram:2M,vram:4M\0" \
	"mmcargs=setenv bootargs console=${console} " \
		"video=${videospec},mode:${videomode} " \
		"root=/dev/mmcblk0p2 rw " \
		"rootfstype=ext3 rootwait\0" \
	"nandargs=setenv bootargs console=${console} " \
		"video=${videospec},mode:${videomode} " \
		"root=/dev/mtdblock4 rw " \
		"rootfstype=jffs2\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loadimage=load mmc ${bootpart} ${loadaddr} ${bootdir}/${bootfile}\0" \
	"loadfdt=load mmc ${bootpart} ${fdtaddr} ${bootdir}/${fdtfile}\0" \
	"loadzimage=setenv bootfile zImage; if run loadimage; then run loadfdt;fi\0"\
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"mmczboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootz ${loadaddr} - ${fdtaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 280000 400000; " \
		"bootm ${loadaddr}\0" \

#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; if mmc rescan; then " \
		"if run loadbootscript; then " \
			"run bootscript; " \
		"else " \
			"if run loadimage; then " \
				"run mmcboot; " \
			"else if run loadzimage; then " \
				"run mmczboot; " \
			"else run nandboot; " \
			"fi; fi;" \
		"fi; " \
	"else run nandboot; fi"

/*
 * Miscellaneous configurable options
 */
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1)	/* memtest */
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_2 + \
					0x01F00000) /* 31MB */

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

#define ONENAND_ENV_OFFSET		0x260000 /* environment starts here */

#define CONFIG_SYS_ENV_SECT_SIZE	(128 << 10)	/* 128 KiB */
#define CONFIG_ENV_OFFSET		0x260000
#define CONFIG_ENV_ADDR			0x260000

#endif				/* __CONFIG_H */
