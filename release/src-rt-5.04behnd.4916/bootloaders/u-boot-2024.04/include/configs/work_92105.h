/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * WORK Microwave work_92105 board configuration file
 *
 * (C) Copyright 2014  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 */

#ifndef __CONFIG_WORK_92105_H__
#define __CONFIG_WORK_92105_H__

/* SoC and board defines */
#include <linux/sizes.h>
#include <asm/arch/cpu.h>

/*
 * Memory configurations
 */
#define CFG_SYS_SDRAM_BASE		EMC_DYCS0_BASE
#define CFG_SYS_SDRAM_SIZE		SZ_128M

/*
 * U-Boot General Configurations
 */

/*
 * NAND chip timings for FIXME: which one?
 */

#define CFG_LPC32XX_NAND_MLC_TCEA_DELAY  333333333
#define CFG_LPC32XX_NAND_MLC_BUSY_DELAY   10000000
#define CFG_LPC32XX_NAND_MLC_NAND_TA      18181818
#define CFG_LPC32XX_NAND_MLC_RD_HIGH      31250000
#define CFG_LPC32XX_NAND_MLC_RD_LOW       45454545
#define CFG_LPC32XX_NAND_MLC_WR_HIGH      40000000
#define CFG_LPC32XX_NAND_MLC_WR_LOW       83333333

/*
 * NAND
 */

/* driver configuration */
#define CFG_SYS_MAX_NAND_CHIPS 1
#define CFG_SYS_NAND_BASE MLC_NAND_BASE

/*
 * GPIO
 */

/*
 * Environment
 */

/*
 * SPL
 */

/* SPL will be executed at offset 0 */
/* SPL will use SRAM as stack */
/* Use the framework and generic lib */
/* SPL will use serial */
/* SPL will load U-Boot from NAND offset 0x40000 */
/* U-Boot will be 0x40000 bytes, loaded and run at CONFIG_TEXT_BASE */
#define CFG_SYS_NAND_U_BOOT_START CONFIG_TEXT_BASE
#define CFG_SYS_NAND_U_BOOT_DST   CONFIG_TEXT_BASE

/*
 * Include SoC specific configuration
 */
#include <asm/arch/config.h>

#endif  /* __CONFIG_WORK_92105_H__*/
