/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006-2008
 * Texas Instruments.
 * Richard Woodruff <r-woodruff2@ti.com>
 * Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * (C) Copyright 2009
 * Frederik Kriewitz <frederik@kriewitz.eu>
 *
 * Configuration settings for the DevKit8000 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ti_omap3_common.h>

/* BOOTP/DHCP options */

#define MEM_LAYOUT_ENV_SETTINGS \
	DEFAULT_LINUX_BOOT_ENV

/* Environment information */
#define CFG_EXTRA_ENV_SETTINGS \
	MEM_LAYOUT_ENV_SETTINGS \
	"console=ttyO2,115200n8\0" \
	"mmcdev=0\0" \
	"vram=12M\0" \
	"dvimode=1024x768MR-16@60\0" \
	"defaultdisplay=dvi\0" \
	"nfsopts=hard,tcp,rsize=65536,wsize=65536\0" \
	"kernelopts=rw\0" \
	"commonargs=" \
		"setenv bootargs console=${console} " \
		"vram=${vram} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay}\0" \
	"mmcargs=" \
		"run commonargs; " \
		"setenv bootargs ${bootargs} " \
		"root=/dev/mmcblk0p2 " \
		"rootwait " \
		"${kernelopts}\0" \
	"nandargs=" \
		"run commonargs; " \
		"setenv bootargs ${bootargs} " \
		"omapfb.mode=dvi:${dvimode} " \
		"omapdss.def_disp=${defaultdisplay} " \
		"root=/dev/mtdblock4 " \
		"rootfstype=jffs2 " \
		"${kernelopts}\0" \
	"netargs=" \
		"run commonargs; " \
		"setenv bootargs ${bootargs} " \
		"root=/dev/nfs " \
		"nfsroot=${serverip}:${rootpath},${nfsopts} " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:${netmask}:${hostname}::off " \
		"${kernelopts} " \
		"dnsip1=${dnsip} " \
		"dnsip2=${dnsip2}\0" \
	"loadbootscript=fatload mmc ${mmcdev} ${loadaddr} boot.scr\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source ${loadaddr}\0" \
	"loaduimage=fatload mmc ${mmcdev} ${loadaddr} uImage\0" \
	"eraseenv=nand unlock 0x260000 0x20000; nand erase 0x260000 0x20000\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"nandboot=echo Booting from nand ...; " \
		"run nandargs; " \
		"nand read ${loadaddr} 280000 400000; " \
		"bootm ${loadaddr}\0" \
	"netboot=echo Booting from network ...; " \
		"dhcp ${loadaddr}; " \
		"run netargs; " \
		"bootm ${loadaddr}\0" \
	"autoboot=mmc dev ${mmcdev}; if mmc rescan; then " \
			"if run loadbootscript; then " \
				"run bootscript; " \
			"else " \
				"if run loaduimage; then " \
					"run mmcboot; " \
				"else run nandboot; " \
				"fi; " \
			"fi; " \
		"else run nandboot; fi\0"

/* Defines for SPL */

/* NAND boot config */
#define CFG_SYS_NAND_ECCPOS		{2, 3, 4, 5, 6, 7, 8, 9,\
						10, 11, 12, 13}

#define CFG_SYS_NAND_ECCSIZE		512
#define CFG_SYS_NAND_ECCBYTES	3

#define CFG_SYS_NAND_U_BOOT_SIZE	0x200000

#endif /* __CONFIG_H */
