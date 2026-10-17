/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the SAMA5D4 Xplained ultra board.
 *
 * Copyright (C) 2014 Atmel
 *		      Bo Shen <voice.shen@atmel.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "at91-sama5_common.h"

/* SDRAM */
#define CFG_SYS_SDRAM_BASE           0x20000000
#define CFG_SYS_SDRAM_SIZE		0x20000000

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CFG_SYS_NAND_BASE		0x80000000
/* our ALE is AD21 */
#define CFG_SYS_NAND_MASK_ALE	(1 << 21)
/* our CLE is AD22 */
#define CFG_SYS_NAND_MASK_CLE	(1 << 22)
#endif

/* SPL */

#endif
