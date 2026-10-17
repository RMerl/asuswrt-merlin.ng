/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (c) 2015 Purna Chandra Mandal <purna.mandal@microchip.com>
 *
 * Microchip PIC32MZ[DA] Starter Kit.
 */

#ifndef __PIC32MZDASK_CONFIG_H
#define __PIC32MZDASK_CONFIG_H

/* System Configuration */

/*--------------------------------------------
 * CPU configuration
 */

/*----------------------------------------------------------------------
 * Memory Layout
 */
/* Initial RAM for temporary stack, global data */
#define CFG_SYS_INIT_RAM_SIZE	0x10000
#define CFG_SYS_INIT_RAM_ADDR	\
	(CONFIG_SYS_SRAM_BASE + CONFIG_SYS_SRAM_SIZE - CFG_SYS_INIT_RAM_SIZE)

/* SDRAM Configuration (for final code, data, stack, heap) */
#define CFG_SYS_SDRAM_BASE		0x88000000

/* Memory Test */

/*----------------------------------------------------------------------
 * Commands
 */

/*------------------------------------------------------------
 * Console Configuration
 */

/*--------------------------------------------------
 * USB Configuration
 */

/* -------------------------------------------------
 * Environment
 */

/* ---------------------------------------------------------------------
 * Board boot configuration
 */

#define MEM_LAYOUT_ENV_SETTINGS					\
	"kernel_addr_r="__stringify(CONFIG_SYS_LOAD_ADDR)"\0"	\
	"fdt_addr_r=0x89d00000\0"				\
	"scriptaddr=0x88300000\0"				\

#define CFG_LEGACY_BOOTCMD_ENV					\
	"legacy_bootcmd= "						\
		"if load mmc 0 ${scriptaddr} uEnv.txt; then "		\
			"env import -tr ${scriptaddr} ${filesize}; "	\
			"if test -n \"${bootcmd_uenv}\" ; then "	\
				"echo Running bootcmd_uenv ...; "	\
				"run bootcmd_uenv; "			\
			"fi; "						\
		"fi; \0"

#define BOOT_TARGET_DEVICES(func)	\
	func(MMC, mmc, 0)		\
	func(USB, usb, 0)		\
	func(DHCP, dhcp, na)

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS	\
	MEM_LAYOUT_ENV_SETTINGS		\
	CFG_LEGACY_BOOTCMD_ENV	\
	BOOTENV

#endif	/* __PIC32MZDASK_CONFIG_H */
