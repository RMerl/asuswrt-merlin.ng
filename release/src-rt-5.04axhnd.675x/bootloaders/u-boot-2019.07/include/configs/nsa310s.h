/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015
 * Gerald Kerma <dreagle@doukki.net>
 * Tony Dinh <mibodhi@gmail.com>
 * Luka Perkov <luka.perkov@sartura.hr>
 */

#ifndef _CONFIG_NSA310S_H
#define _CONFIG_NSA310S_H

/* high level configuration options */
#define CONFIG_FEROCEON_88FR131	1	/* CPU Core subversion */
#define CONFIG_KW88F6192		1	/* SOC Name */
#define CONFIG_KW88F6702		1	/* SOC Name */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/* compression configuration */
#define CONFIG_BZIP2

/* commands configuration */

/*
 * mv-common.h should be defined after CMD configs since it used them
 * to enable certain macros
 */
#include "mv-common.h"

/* environment variables configuration */
#ifdef CONFIG_CMD_NAND
#define CONFIG_ENV_SECT_SIZE	0x20000
#endif
#define CONFIG_ENV_SIZE		0x20000
#define CONFIG_ENV_OFFSET	0xe0000

/* default environment variables */
#define CONFIG_BOOTCOMMAND \
	"setenv bootargs ${console} ${mtdparts} ${bootargs_root}; " \
	"ubi part root; " \
	"ubifsmount ubi:rootfs; " \
	"ubifsload 0x800000 ${kernel}; " \
	"ubifsload 0x700000 ${fdt}; " \
	"ubifsumount; " \
	"fdt addr 0x700000; fdt resize; fdt chosen; " \
	"bootz 0x800000 - 0x700000"

#define CONFIG_EXTRA_ENV_SETTINGS \
	"console=console=ttyS0,115200\0" \
	"mtdids=nand0=orion_nand\0" \
	"mtdparts="CONFIG_MTDPARTS_DEFAULT \
	"kernel=/boot/zImage\0" \
	"fdt=/boot/nsa310s.dtb\0" \
	"bootargs_root=ubi.mtd=3 root=ubi0:rootfs rootfstype=ubifs rw\0"

/* Ethernet driver configuration */
#ifdef CONFIG_CMD_NET
#define CONFIG_NETCONSOLE
#define CONFIG_MVGBE_PORTS	{1, 0}	/* enable port 0 only */
#define CONFIG_PHY_BASE_ADR	1
#define CONFIG_RESET_PHY_R
#endif /* CONFIG_CMD_NET */

/* SATA driver configuration */
#ifdef CONFIG_IDE
#define __io
#define CONFIG_IDE_PREINIT
#define CONFIG_MVSATA_IDE_USE_PORT0
#define CONFIG_SYS_ATA_IDE0_OFFSET	MV_SATA_PORT0_OFFSET
#endif /* CONFIG_IDE */

/* RTC driver configuration */
#ifdef CONFIG_CMD_DATE
#define CONFIG_RTC_MV
#endif /* CONFIG_CMD_DATE */

#endif /* _CONFIG_NSA310S_H */
