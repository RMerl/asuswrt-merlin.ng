/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Embest/Timll DevKit3250 board configuration file
 *
 * Copyright (C) 2011-2015 Vladimir Zapolskiy <vz@mleia.com>
 */

#ifndef __CONFIG_DEVKIT3250_H__
#define __CONFIG_DEVKIT3250_H__

/* SoC and board defines */
#include <linux/sizes.h>
#include <asm/arch/cpu.h>

/*
 * Memory configurations
 */
#define CFG_SYS_SDRAM_BASE		EMC_DYCS0_BASE
#define CFG_SYS_SDRAM_SIZE		SZ_64M

/*
 * DMA
 */

/*
 * GPIO
 */

/*
 * NOR Flash
 */
#define CFG_SYS_FLASH_BASE		EMC_CS0_BASE
#define CFG_SYS_FLASH_SIZE		SZ_4M

/*
 * NAND controller
 */
#define CFG_SYS_NAND_BASE		SLC_NAND_BASE
#define CFG_SYS_NAND_BASE_LIST	{ CFG_SYS_NAND_BASE }

/*
 * NAND chip timings
 */
#define CFG_LPC32XX_NAND_SLC_WDR_CLKS	14
#define CFG_LPC32XX_NAND_SLC_WWIDTH		66666666
#define CFG_LPC32XX_NAND_SLC_WHOLD		200000000
#define CFG_LPC32XX_NAND_SLC_WSETUP		50000000
#define CFG_LPC32XX_NAND_SLC_RDR_CLKS	14
#define CFG_LPC32XX_NAND_SLC_RWIDTH		66666666
#define CFG_LPC32XX_NAND_SLC_RHOLD		200000000
#define CFG_LPC32XX_NAND_SLC_RSETUP		50000000

/*
 * USB
 */
#define CFG_USB_ISP1301_I2C_ADDR		0x2d

/*
 * U-Boot General Configurations
 */

/*
 * Pass open firmware flat tree
 */

/*
 * Environment
 */

#define CFG_EXTRA_ENV_SETTINGS		\
	"ethaddr=00:01:90:00:C0:81\0"		\
	"dtbaddr=0x81000000\0"			\
	"nfsroot=/opt/projects/images/vladimir/oe/devkit3250/rootfs\0"	\
	"tftpdir=vladimir/oe/devkit3250\0"	\
	"userargs=oops=panic\0"

/*
 * U-Boot Commands
 */

/* U-Boot will be 0x60000 bytes, loaded and run at CONFIG_TEXT_BASE */
#define CFG_SYS_NAND_U_BOOT_SIZE	0x60000

#define CFG_SYS_NAND_U_BOOT_START	CONFIG_TEXT_BASE
#define CFG_SYS_NAND_U_BOOT_DST	CONFIG_TEXT_BASE

/* See common/spl/spl.c  spl_set_header_raw_uboot() */

/*
 * Include SoC specific configuration
 */
#include <asm/arch/config.h>

#endif  /* __CONFIG_DEVKIT3250_H__*/
