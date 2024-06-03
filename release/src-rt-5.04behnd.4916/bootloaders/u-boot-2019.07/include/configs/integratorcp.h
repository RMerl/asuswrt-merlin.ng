/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Texas Instruments.
 * Kshitij Gupta <kshitij@ti.com>
 * Configuation settings for the TI OMAP Innovator board.
 *
 * (C) Copyright 2004
 * ARM Ltd.
 * Philippe Robin, <philippe.robin@arm.com>
 * Configuration for Compact Integrator board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "integrator-common.h"

/* Integrator CP-specific configuration */
#define CONFIG_SYS_HZ_CLOCK		1000000	/* Timer 1 is clocked at 1Mhz */

/*
 * Hardware drivers
 */
#define CONFIG_SMC91111
#define CONFIG_SMC_USE_32_BIT
#define CONFIG_SMC91111_BASE    0xC8000000
#undef CONFIG_SMC91111_EXT_PHY

/*
 * Command line configuration.
 */
#define CONFIG_BOOTCOMMAND "tftpboot ; bootm"
#define CONFIG_SERVERIP 192.168.1.100
#define CONFIG_IPADDR 192.168.1.104
#define CONFIG_BOOTFILE "uImage"

/*
 * Miscellaneous configurable options
 */
#define PHYS_FLASH_SIZE			0x01000000	/* 16MB */
#define CONFIG_SYS_MAX_FLASH_SECT	64
#define CONFIG_SYS_MONITOR_LEN		0x00100000

/*
 * Move up the U-Boot & monitor area if more flash is fitted.
 * If this U-Boot is to be run on Integrators with varying flash sizes,
 * drivers/mtd/cfi_flash.c::flash_init() can read the Integrator CP_FLASHPROG
 * register and dynamically assign CONFIG_ENV_ADDR & CONFIG_SYS_MONITOR_BASE
 * - CONFIG_SYS_MONITOR_BASE is set to indicate that the environment is not
 * embedded in the boot monitor(s) area
 */
#if ( PHYS_FLASH_SIZE == 0x04000000 )

#define CONFIG_ENV_ADDR		0x27F00000
#define CONFIG_SYS_MONITOR_BASE	0x27F40000

#elif (PHYS_FLASH_SIZE == 0x02000000 )

#define CONFIG_ENV_ADDR		0x25F00000
#define CONFIG_SYS_MONITOR_BASE	0x25F40000

#else

#define CONFIG_ENV_ADDR		0x24F00000
#define CONFIG_SYS_MONITOR_BASE	0x27F40000

#endif

#define CONFIG_ENV_SECT_SIZE	0x40000		/* 256KB */
#define CONFIG_ENV_SIZE		8192		/* 8KB */

#endif /* __CONFIG_H */
