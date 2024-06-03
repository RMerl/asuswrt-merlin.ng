/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Holger Brunck, Keymile GmbH Hannover, <holger.brunck@keymile.com>
 * Christian Herzig, Keymile AG Switzerland, <christian.herzig@keymile.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_HOSTNAME		"kmcoge5ne"
#define CONFIG_KM_BOARD_NAME	"kmcoge5ne"
#define CONFIG_KM_DEF_NETDEV	"netdev=eth1\0"
#define CONFIG_NAND_ECC_BCH
#define CONFIG_NAND_KMETER1
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define NAND_MAX_CHIPS				1
#define CONFIG_SYS_NAND_BASE CONFIG_SYS_KMBEC_FPGA_BASE /* PRIO_BASE_ADDRESS */

#define CONFIG_KM_UBI_PARTITION_NAME_BOOT	"ubi0"
#define CONFIG_KM_UBI_PARTITION_NAME_APP	"ubi1"

/* include common defines/options for all Keymile boards */
#include "km/keymile-common.h"
#include "km/km-powerpc.h"
#include "km/km-mpc83xx.h"
#include "km/km-mpc8360.h"

/*
 * System Clock Setup
 */
#define CONFIG_83XX_CLKIN		66000000
#define CONFIG_SYS_CLK_FREQ		66000000
#define CONFIG_83XX_PCICLK		66000000

/**
 * KMCOGE5NE has 512 MB RAM
 */
#define CONFIG_SYS_DDR_CS0_CONFIG (\
	CSCONFIG_EN | \
	CSCONFIG_AP | \
	CSCONFIG_ODT_WR_ONLY_CURRENT | \
	CSCONFIG_BANK_BIT_3 | \
	CSCONFIG_ROW_BIT_13 | \
	CSCONFIG_COL_BIT_10)

/*
 * BFTIC3 on the local bus CS4
 */
#define CONFIG_SYS_BFTIC3_BASE			0xB0000000
#define CONFIG_SYS_BFTIC3_SIZE			256

/* enable POST tests */
#define CONFIG_POST (CONFIG_SYS_POST_MEMORY|CONFIG_SYS_POST_MEM_REGIONS)
#define CONFIG_POST_EXTERNAL_WORD_FUNCS /* use own functions, not generic */
#define CPM_POST_WORD_ADDR  CONFIG_SYS_MEMTEST_END
#define CONFIG_TESTPIN_REG  gprt3	/* for kmcoge5ne */
#define CONFIG_TESTPIN_MASK 0x20	/* for kmcoge5ne */

#endif /* CONFIG */
