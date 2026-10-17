/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the SAMA5D3 Xplained board.
 *
 * Copyright (C) 2014 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>
#include "at91-sama5_common.h"

/*
 * This needs to be defined for the OHCI code to work but it is defined as
 * ATMEL_ID_UHPHS in the CPU specific header files.
 */
#define ATMEL_ID_UHP			32

/*
 * Specify the clock enable bit in the PMC_SCER register.
 */
#define ATMEL_PMC_UHP			(1 <<  6)

/* SDRAM */
#define CFG_SYS_SDRAM_BASE           0x20000000
#define CFG_SYS_SDRAM_SIZE		0x10000000

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_NAND_BASE		0x60000000
/* our ALE is AD21 */
#define CFG_SYS_NAND_MASK_ALE	(1 << 21)
/* our CLE is AD22 */
#define CFG_SYS_NAND_MASK_CLE	(1 << 22)
#endif

/* SPL */

/* size of u-boot.bin to load */

/* Falcon boot support on raw MMC */
/* U-Boot proper stored by default at 0x200 (256 KiB) */

#endif
