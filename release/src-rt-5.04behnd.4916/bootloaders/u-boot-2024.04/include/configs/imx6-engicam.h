/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 Amarula Solutions B.V.
 * Copyright (C) 2016 Engicam S.r.l.
 *
 * Configuration settings for the Engicam i.MX6 SOM Starter Kits.
 */

#ifndef __IMX6_ENGICAM_CONFIG_H
#define __IMX6_ENGICAM_CONFIG_H

#include <linux/sizes.h>
#include <linux/stringify.h>
#include "mx6_common.h"

/* Total Size of Environment Sector */

/* Environment */
#ifndef CONFIG_ENV_IS_NOWHERE
/* Environment in MMC */
# if defined(CONFIG_ENV_IS_IN_MMC)
/* Environment in NAND */
# endif
#endif

/* Default environment */
#define CFG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"splashpos=m,m\0" \
	"splashimage=" __stringify(CONFIG_SYS_LOAD_ADDR) "\0" \
	"image=uImage\0" \
	"fit_image=fit.itb\0" \
	"fdt_high=0xffffffff\0" \
	"fdt_addr=" FDT_ADDR "\0" \
	"boot_fdt=try\0" \
	"mmcpart=1\0" \
	"nandroot=ubi0:rootfs rootfstype=ubifs\0" \
	"mmcautodetect=yes\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"ubiargs=setenv bootargs console=${console},${baudrate} " \
		"ubi.mtd=5 root=${nandroot} ${mtdparts}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"loadfit=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${fit_image}\0" \
	"altbootcmd=run recoveryboot\0"\
	"fitboot=echo Booting FIT image from mmc ...; " \
		"run mmcargs; " \
		"bootm ${loadaddr}\0" \
	"_mmcboot=run mmcargs; " \
		"run mmcargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootm ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootm; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootm; " \
		"fi\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"if mmc rescan; then " \
			"if run loadbootscript; then " \
				"run bootscript; " \
			"else " \
				"if run loadfit; then " \
					"run fitboot; " \
				"else " \
					"if run loadimage; then " \
						"run _mmcboot; " \
					"fi; " \
				"fi; " \
			"fi; " \
		"fi\0" \
	"nandboot=echo Booting from nand ...; " \
		"if mtdparts; then " \
			"echo Starting nand boot ...; " \
		"else " \
			"mtdparts default; " \
		"fi; " \
		"run ubiargs; " \
		"nand read ${loadaddr} kernel 0x800000; " \
		"nand read ${fdt_addr} dtb 0x100000; " \
		"bootm ${loadaddr} - ${fdt_addr}\0" \
	"recoveryboot=if test ${modeboot} = mmcboot; then " \
			"run mmcboot; " \
		"else " \
			"run nandboot; " \
		"fi\0"

/* Miscellaneous configurable options */

#ifdef CONFIG_MX6UL
# define DRAM_OFFSET(x)			0x87##x
# define FDT_ADDR			__stringify(DRAM_OFFSET(800000))
#else
# define DRAM_OFFSET(x)			0x1##x
# define FDT_ADDR			__stringify(DRAM_OFFSET(8000000))
#endif

/* Physical Memory Map */
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CFG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CFG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CFG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* UART */
#ifdef CONFIG_MXC_UART
# ifdef CONFIG_MX6UL
#  define CFG_MXC_UART_BASE		UART1_BASE
# else
#  define CFG_MXC_UART_BASE		UART4_BASE
# endif
#endif

/* MMC */

/* NAND */
#ifdef CONFIG_NAND_MXS
# define CFG_SYS_NAND_BASE		0x40000000
# define CFG_SYS_NAND_U_BOOT_START	CONFIG_TEXT_BASE

/* MTD device */
#endif

#endif /* __IMX6_ENGICAM_CONFIG_H */
