/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013 Keymile AG
 * Valentin Longchamp <valentin.longchamp@keymile.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* KMLION1 */
#if defined(CONFIG_KMLION1)
#define CONFIG_HOSTNAME		"kmlion1"
#define CONFIG_KM_BOARD_NAME	"kmlion1"

/* KMCOGE4 */
#elif defined(CONFIG_KMCOGE4)
#define CONFIG_HOSTNAME		"kmcoge4"
#define CONFIG_KM_BOARD_NAME	"kmcoge4"

#else
#error ("Board not supported")
#endif

#define CONFIG_KMP204X

#include "km/kmp204x-common.h"

#if defined(CONFIG_KMLION1)
/* App1 Local bus */
#define CONFIG_SYS_LBAPP1_BASE		0xD0000000
#define CONFIG_SYS_LBAPP1_BASE_PHYS	0xFD0000000ull

#define CONFIG_SYS_LBAPP1_BR_PRELIM (BR_PHYS_ADDR(CONFIG_SYS_LBAPP1_BASE_PHYS) \
				| BR_PS_8	/* Port Size 8 bits */ \
				| BR_DECC_OFF	/* no error corr */ \
				| BR_MS_GPCM	/* MSEL = GPCM */ \
				| BR_V)		/* valid */

#define CONFIG_SYS_LBAPP1_OR_PRELIM (OR_AM_256MB	/* length 256MB */ \
				| OR_GPCM_ACS_DIV2 /* LCS 1/2 clk after */ \
				| OR_GPCM_CSNT /* LCS 1/4 clk before */ \
				| OR_GPCM_SCY_2 /* 2 clk wait cycles */ \
				| OR_GPCM_TRLX /* relaxed tmgs */ \
				| OR_GPCM_EAD) /* extra bus clk cycles */
/* Local bus app1 Base Address */
#define CONFIG_SYS_BR2_PRELIM  CONFIG_SYS_LBAPP1_BR_PRELIM
/* Local bus app1 Options */
#define CONFIG_SYS_OR2_PRELIM  CONFIG_SYS_LBAPP1_OR_PRELIM
#endif

/* App2 Local bus */
#define CONFIG_SYS_LBAPP2_BASE		0xE0000000
#define CONFIG_SYS_LBAPP2_BASE_PHYS	0xFE0000000ull

#define CONFIG_SYS_LBAPP2_BR_PRELIM (BR_PHYS_ADDR(CONFIG_SYS_LBAPP2_BASE_PHYS) \
				| BR_PS_8	/* Port Size 8 bits */ \
				| BR_DECC_OFF	/* no error corr */ \
				| BR_MS_GPCM	/* MSEL = GPCM */ \
				| BR_V)		/* valid */

#define CONFIG_SYS_LBAPP2_OR_PRELIM (OR_AM_256MB	/* length 256MB */ \
				| OR_GPCM_ACS_DIV2 /* LCS 1/2 clk after */ \
				| OR_GPCM_CSNT /* LCS 1/4 clk before */ \
				| OR_GPCM_SCY_2 /* 2 clk wait cycles */ \
				| OR_GPCM_TRLX /* relaxed tmgs */ \
				| OR_GPCM_EAD) /* extra bus clk cycles */
/* Local bus app2 Base Address */
#define CONFIG_SYS_BR3_PRELIM  CONFIG_SYS_LBAPP2_BR_PRELIM
/* Local bus app2 Options */
#define CONFIG_SYS_OR3_PRELIM  CONFIG_SYS_LBAPP2_OR_PRELIM

#endif	/* __CONFIG_H */
