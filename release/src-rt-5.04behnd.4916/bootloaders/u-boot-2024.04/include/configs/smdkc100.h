/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 Samsung Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * HeungJun Kim <riverful.kim@samsung.com>
 * Inki Dae <inki.dae@samsung.com>
 *
 * Configuation settings for the SAMSUNG SMDKC100 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <asm/arch/cpu.h>		/* get chip and board defs */

/* input clock of PLL: SMDKC100 has 12MHz input clock */

/* DRAM Base */
#define CFG_SYS_SDRAM_BASE		0x30000000

/* Text Base */

/*
 * select serial console configuration
 */

#define COMMON_BOOT	"console=ttySAC0,115200n8" \
				" mem=128M " \
				" " CONFIG_MTDPARTS_DEFAULT

#define CFG_EXTRA_ENV_SETTINGS					\
	"updateb=" \
		"onenand erase 0x0 0x40000;" \
		"onenand write 0x32008000 0x0 0x40000\0" \
	"updatek=" \
		"onenand erase 0x60000 0x300000;" \
		"onenand write 0x31008000 0x60000 0x300000\0" \
	"updateu=" \
		"onenand erase block 147-4095;" \
		"onenand write 0x32000000 0x1260000 0x8C0000\0" \
	"bootk=" \
		"onenand read 0x30007FC0 0x60000 0x300000;" \
		"bootm 0x30007FC0\0" \
	"flashboot=" \
		"set bootargs root=/dev/mtdblock${bootblock} " \
		"rootfstype=${rootfstype} " \
		"ubi.mtd=${ubiblock} ${opts} " COMMON_BOOT ";" \
		"run bootk\0" \
	"ubifsboot=" \
		"set bootargs root=ubi0!rootfs rootfstype=ubifs " \
		" ubi.mtd=${ubiblock} ${opts} " COMMON_BOOT "; " \
		"run bootk\0" \
	"boottrace=setenv opts initcall_debug; run bootcmd\0" \
	"android=" \
		"set bootargs root=ubi0!ramdisk ubi.mtd=${ubiblock} " \
		"rootfstype=ubifs init=/init.sh " COMMON_BOOT "; " \
		"run bootk\0" \
	"nfsboot=" \
		"set bootargs root=/dev/nfs ubi.mtd=${ubiblock} " \
		"nfsroot=${nfsroot},nolock " \
		"ip=${ipaddr}:${serverip}:${gatewayip}:" \
		"${netmask}:nowplus:usb0:off " COMMON_BOOT "; " \
		"run bootk\0" \
	"ramboot=" \
		"set bootargs root=/dev/ram0 rw rootfstype=ext2" \
		" console=ttySAC0,115200n8 mem=128M" \
		" initrd=0x33000000,8M ramdisk=8192\0" \
	"rootfstype=cramfs\0" \
	"meminfo=mem=128M\0" \
	"nfsroot=/nfsroot/arm\0" \
	"bootblock=5\0" \
	"ubiblock=4\0" \
	"ubi=enabled"

/*
 * Miscellaneous configurable options
 */

/* SMDKC100 has 1 banks of DRAM, we use only one in U-Boot */
#define PHYS_SDRAM_1		CFG_SYS_SDRAM_BASE	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	(128 << 20)	/* 0x8000000, 128 MB Bank #1 */

/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */

/*-----------------------------------------------------------------------
 * Boot configuration
 */

#define CFG_SYS_ONENAND_BASE		0xE7100000

/*
 * Ethernet Contoller driver
 */
#ifdef CONFIG_CMD_NET
#define CFG_ENV_SROM_BANK   3       /* Select SROM Bank-3 for Ethernet*/
#endif /* CONFIG_CMD_NET */

#endif	/* __CONFIG_H */
