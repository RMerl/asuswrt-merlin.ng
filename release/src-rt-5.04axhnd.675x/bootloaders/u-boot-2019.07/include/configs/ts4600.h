/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2016 Savoir-faire Linux Inc.
 *
 * Author: Sebastien Bourdelin <sebastien.bourdelin@savoirfairelinux.com>
 *
 * Derived from MX28EVK code by
 *   Fabio Estevam <fabio.estevam@freescale.com>
 *   Freescale Semiconductor, Inc.
 *
 * Configuration settings for the TS4600 Board
 */
#ifndef __CONFIGS_TS4600_H__
#define __CONFIGS_TS4600_H__

/* U-Boot Commands */

/* Memory configuration */
#define PHYS_SDRAM_1			0x40000000	/* Base address */
#define PHYS_SDRAM_1_SIZE		0x40000000	/* Max 1 GB RAM */
#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

/* Environment */
#define CONFIG_ENV_SIZE			(8 * 1024)

/* Environment is in MMC */
#if defined(CONFIG_CMD_MMC) && defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_ENV_OFFSET		(256 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV		0
#endif

/* Boot Linux */
#define CONFIG_LOADADDR		0x42000000
#define CONFIG_SYS_LOAD_ADDR	CONFIG_LOADADDR

/* Extra Environment */
#define CONFIG_EXTRA_ENV_SETTINGS \
	"fdt_addr=0x41000000\0" \
	"loadkernel=load mmc ${mmcdev}:${mmcpart} ${loadaddr} zImage\0" \
	"loadfdt=load mmc ${mmcdev}:${mmcpart} ${fdt_addr} imx28-ts4600.dtb\0" \
	"loadbootscript=load mmc ${mmcdev}:${mmcpart} ${loadaddr} boot.ub\0" \
	"bootscript=echo Running bootscript from mmc...; " \
		"setenv mmcdev 0; " \
		"setenv mmcpart 2; " \
		"run loadbootscript && source ${loadaddr}; \0" \
	"sdboot=echo Booting from SD card ...; " \
		"setenv mmcdev 0; " \
		"setenv mmcpart 2; " \
		"setenv root /dev/mmcblk0p3; " \
		"run loadkernel && run loadfdt; \0" \
	"startbootsequence=run bootscript || run sdboot \0" \

#define CONFIG_BOOTCOMMAND \
	"mmc rescan; " \
	"run startbootsequence; " \
	"setenv cmdline_append console=ttyAMA0,115200; " \
	"setenv bootargs root=${root} rootwait rw ${cmdline_append}; " \
	"bootz ${loadaddr} - ${fdt_addr}; "

/* The rest of the configuration is shared */
#include <configs/mxs.h>

#endif /* __CONFIGS_TS4600_H__ */
