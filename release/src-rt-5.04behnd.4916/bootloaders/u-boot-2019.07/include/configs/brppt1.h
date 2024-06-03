/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * brtpp1.h
 *
 * specific parts for B&R T-Series Motherboard
 *
 * Copyright (C) 2013 Hannes Schmelzer <oe5hpm@oevsv.at> -
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 */

#ifndef __CONFIG_BRPPT1_H__
#define __CONFIG_BRPPT1_H__

#include <configs/bur_cfg_common.h>
#include <configs/bur_am335x_common.h>
/* ------------------------------------------------------------------------- */
/* memory */
#define CONFIG_SYS_MALLOC_LEN		(5 * 1024 * 1024)
#define CONFIG_SYS_BOOTM_LEN		SZ_32M

/* Clock Defines */
#define V_OSCK				26000000  /* Clock output from T2 */
#define V_SCLK				(V_OSCK)

#define CONFIG_POWER_TPS65217

/* Support both device trees and ATAGs. */
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
/*#define CONFIG_MACH_TYPE		3589*/
#define CONFIG_MACH_TYPE		0xFFFFFFFF /* TODO: check with kernel*/

/*
 * When we have NAND flash we expect to be making use of mtdparts,
 * both for ease of use in U-Boot and for passing information on to
 * the Linux kernel.
 */

#ifdef CONFIG_SPL_OS_BOOT
#define CONFIG_SYS_SPL_ARGS_ADDR		0x80F80000

/* RAW SD card / eMMC */
#define CONFIG_SYS_MMCSD_RAW_MODE_KERNEL_SECTOR	0x900	/* address 0x120000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTOR	0x80	/* address 0x10000 */
#define CONFIG_SYS_MMCSD_RAW_MODE_ARGS_SECTORS	0x80	/* 64KiB */

/* NAND */
#ifdef CONFIG_NAND
#define CONFIG_SYS_NAND_SPL_KERNEL_OFFS		0x140000
#endif /* CONFIG_NAND */
#endif /* CONFIG_SPL_OS_BOOT */

#ifdef CONFIG_NAND
#define CONFIG_SPL_NAND_BASE
#define CONFIG_SPL_NAND_DRIVERS
#define CONFIG_SPL_NAND_ECC
#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE
#define CONFIG_SYS_NAND_U_BOOT_OFFS	0x80000
#endif /* CONFIG_NAND */

/* Always 64 KiB env size */
#define CONFIG_ENV_SIZE			(64 << 10)

#ifdef CONFIG_NAND
#define NANDTGTS \
"mtdids=" CONFIG_MTDIDS_DEFAULT "\0" \
"mtdparts=" CONFIG_MTDPARTS_DEFAULT "\0" \
"cfgscr=mw ${dtbaddr} 0; nand read ${cfgaddr} cfgscr && source ${cfgaddr};" \
" fdt addr ${dtbaddr} || cp ${fdtcontroladdr} ${dtbaddr} 4000\0" \
"nandargs=setenv bootargs console=${console} ${optargs} ${optargs_rot} " \
	"root=mtd6 rootfstype=jffs2 b_mode=${b_mode}\0" \
"b_nand=nand read ${loadaddr} kernel; nand read ${dtbaddr} dtb; " \
	"run nandargs; run cfgscr; bootz ${loadaddr} - ${dtbaddr}\0" \
"b_tgts_std=usb0 nand net\0" \
"b_tgts_rcy=net usb0 nand\0" \
"b_tgts_pme=usb0 nand net\0"
#else
#define NANDTGTS ""
#endif /* CONFIG_NAND */

#define MMCSPI_TGTS \
"t30args#0=setenv bootargs ${optargs_rot} ${optargs} console=${console} " \
	"b_mode=${b_mode} root=/dev/mmcblk0p2 rootfstype=ext4\0" \
"b_t30lgcy#0=" \
	"load ${loaddev}:2 ${loadaddr} /boot/PPTImage.md5 && " \
	"load ${loaddev}:2 ${loadaddr} /boot/zImage && " \
	"load ${loaddev}:2 ${dtbaddr} /boot/am335x-ppt30.dtb || " \
	"load ${loaddev}:1 ${dtbaddr} am335x-ppt30-legacy.dtb; "\
	"run t30args#0; run cfgscr; bootz ${loadaddr} - ${dtbaddr}\0" \
"t30args#1=setenv bootargs ${optargs_rot} ${optargs} console=${console} " \
	"b_mode=${b_mode}\0" \
"b_t30lgcy#1=" \
	"load ${loaddev}:1 ${loadaddr} zImage && " \
	"load ${loaddev}:1 ${dtbaddr} am335x-ppt30.dtb && " \
	"load ${loaddev}:1 ${ramaddr} rootfsPPT30.uboot && " \
	"run t30args#1; run cfgscr; bootz ${loadaddr} ${ramaddr} ${dtbaddr}\0" \
"b_mmc0=load ${loaddev}:1 ${scraddr} bootscr.img && source ${scraddr}\0" \
"b_mmc1=load ${loaddev}:1 ${scraddr} /boot/bootscr.img && source ${scraddr}\0" \
"b_tgts_std=mmc0 mmc1 t30lgcy#0 t30lgcy#1 usb0 net\0" \
"b_tgts_rcy=t30lgcy#1 usb0 net\0" \
"b_tgts_pme=net usb0 mmc0 mmc1\0" \
"loaddev=mmc 1\0"

#ifdef CONFIG_ENV_IS_IN_MMC
#define MMCTGTS \
MMCSPI_TGTS \
"cfgscr=mw ${dtbaddr} 0;" \
" mmc dev 1; mmc read ${cfgaddr} 200 80; source ${cfgaddr};" \
" fdt addr ${dtbaddr} || cp ${fdtcontroladdr} ${dtbaddr} 4000\0"
#else
#define MMCTGTS ""
#endif /* CONFIG_MMC */

#ifdef CONFIG_SPI
#define SPITGTS \
MMCSPI_TGTS \
"cfgscr=mw ${dtbaddr} 0;" \
" sf probe; sf read ${cfgaddr} 0xC0000 10000; source ${cfgaddr};" \
" fdt addr ${dtbaddr} || cp ${fdtcontroladdr} ${dtbaddr} 4000\0"
#else
#define SPITGTS ""
#endif /* CONFIG_SPI */

#define LOAD_OFFSET(x)			0x8##x

#ifndef CONFIG_SPL_BUILD
#define CONFIG_EXTRA_ENV_SETTINGS \
BUR_COMMON_ENV \
"verify=no\0" \
"autoload=0\0" \
"scraddr=" __stringify(LOAD_OFFSET(0000000)) "\0" \
"cfgaddr=" __stringify(LOAD_OFFSET(0020000)) "\0" \
"dtbaddr=" __stringify(LOAD_OFFSET(0040000)) "\0" \
"loadaddr=" __stringify(LOAD_OFFSET(0100000)) "\0" \
"ramaddr=" __stringify(LOAD_OFFSET(2000000)) "\0" \
"console=ttyO0,115200n8\0" \
"optargs=consoleblank=0 quiet panic=2\0" \
"b_break=0\0" \
"b_usb0=usb start && load usb 0 ${scraddr} bootscr.img && source ${scraddr}\0" \
"b_net=tftp ${scraddr} netscript.img && source ${scraddr}\0" \
MMCTGTS \
SPITGTS \
NANDTGTS \
"b_deftgts=if test ${b_mode} = 12; then setenv b_tgts ${b_tgts_pme};" \
" elif test ${b_mode} = 0; then setenv b_tgts ${b_tgts_rcy};" \
" else setenv b_tgts ${b_tgts_std}; fi\0" \
"b_default=run b_deftgts; for target in ${b_tgts};"\
" do echo \"### booting ${target} ###\"; run b_${target};" \
" if test ${b_break} = 1; then; exit; fi; done\0"
#endif /* !CONFIG_SPL_BUILD*/

#ifdef CONFIG_NAND
/*
 * GPMC  block.  We support 1 device and the physical address to
 * access CS0 at is 0x8000000.
 */
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x8000000
/* don't change OMAP_ELM, ECCSCHEME. ROM code only supports this */
#define CONFIG_NAND_OMAP_ECCSCHEME	OMAP_ECC_BCH8_CODE_HW
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_BLOCK_SIZE	(128*1024)
#define CONFIG_SYS_NAND_PAGE_SIZE	2048
#define CONFIG_SYS_NAND_PAGE_COUNT	(CONFIG_SYS_NAND_BLOCK_SIZE / \
					CONFIG_SYS_NAND_PAGE_SIZE)
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BAD_BLOCK_POS	NAND_LARGE_BADBLOCK_POS
#define CONFIG_SYS_NAND_ECCPOS		{2, 3, 4, 5, 6, 7, 8, 9, \
					10, 11, 12, 13, 14, 15, 16, 17, \
					18, 19, 20, 21, 22, 23, 24, 25, \
					26, 27, 28, 29, 30, 31, 32, 33, \
					34, 35, 36, 37, 38, 39, 40, 41, \
					42, 43, 44, 45, 46, 47, 48, 49, \
					50, 51, 52, 53, 54, 55, 56, 57, }

#define CONFIG_SYS_NAND_ECCSIZE		512
#define CONFIG_SYS_NAND_ECCBYTES	14

#define CONFIG_SYS_NAND_U_BOOT_START	CONFIG_SYS_TEXT_BASE

#define CONFIG_NAND_OMAP_GPMC_WSCFG	1
#endif /* CONFIG_NAND */

#if defined(CONFIG_SPI)
/* SPI Flash */
#define CONFIG_SYS_SPI_U_BOOT_OFFS		0x40000
/* Environment */
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT
#define CONFIG_ENV_SECT_SIZE			CONFIG_ENV_SIZE
#define CONFIG_ENV_OFFSET			0x20000
#define CONFIG_ENV_OFFSET_REDUND		(CONFIG_ENV_OFFSET + \
						 CONFIG_ENV_SECT_SIZE)
#elif defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_SYS_MMC_ENV_DEV		1
#define CONFIG_SYS_MMC_ENV_PART		2
#define CONFIG_ENV_OFFSET		0x40000	/* TODO: Adresse definieren */
#define CONFIG_ENV_OFFSET_REDUND	(CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE)
#define CONFIG_SYS_REDUNDAND_ENVIRONMENT

#elif defined(CONFIG_ENV_IS_IN_NAND)
/* No NAND env support in SPL */
#define CONFIG_ENV_OFFSET		0x60000
#define CONFIG_SYS_ENV_SECT_SIZE	CONFIG_ENV_SIZE
#else
#error "no storage for Environment defined!"
#endif

#endif	/* ! __CONFIG_BRPPT1_H__ */
