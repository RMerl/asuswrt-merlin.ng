/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Holger Brunck, Keymile GmbH Hannover, <holger.brunck@keymile.com>
 * Christian Herzig, Keymile AG Switzerland, <christian.herzig@keymile.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define NAND_MAX_CHIPS				1
#define CFG_SYS_NAND_BASE CFG_SYS_KMBEC_FPGA_BASE /* PRIO_BASE_ADDRESS */

/* include common defines/options for all Keymile boards */
#include "km/km-mpc83xx.h"

/**
 * KMCOGE5NE has 512 MB RAM
 */
#define CFG_SYS_DDR_CS0_CONFIG (\
	CSCONFIG_EN | \
	CSCONFIG_AP | \
	CSCONFIG_ODT_WR_ONLY_CURRENT | \
	CSCONFIG_BANK_BIT_3 | \
	CSCONFIG_ROW_BIT_13 | \
	CSCONFIG_COL_BIT_10)

/* enable POST tests */
#define CFG_POST (CFG_SYS_POST_MEMORY|CFG_SYS_POST_MEM_REGIONS)
#define CFG_POST_EXTERNAL_WORD_FUNCS /* use own functions, not generic */
#define CPM_POST_WORD_ADDR  CONFIG_SYS_MEMTEST_END
#define CFG_TESTPIN_REG  gprt3	/* for kmcoge5ne */
#define CFG_TESTPIN_MASK 0x20	/* for kmcoge5ne */

#endif /* CONFIG */
