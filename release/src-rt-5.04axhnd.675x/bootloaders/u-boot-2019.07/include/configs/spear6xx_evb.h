/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Vipin Kumar, STMicroelectronics, <vipin.kumar@st.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 * (easy to change)
 */
#if defined(CONFIG_USBTTY)
#define CONFIG_SPEAR_USBTTY
#endif

#include <configs/spear-common.h>

/* Serial Configuration (PL011) */
#define CONFIG_SYS_SERIAL0			0xD0000000
#define CONFIG_SYS_SERIAL1			0xD0080000
#define CONFIG_PL01x_PORTS			{ (void *)CONFIG_SYS_SERIAL0, \
						(void *)CONFIG_SYS_SERIAL1 }

/* NAND flash configuration */
#define CONFIG_SYS_FSMC_NAND_SP
#define CONFIG_SYS_FSMC_NAND_8BIT
#define CONFIG_SYS_NAND_BASE			0xD2000000

/* Ethernet PHY configuration */
#define CONFIG_PHY_NATSEMI

/* Environment Settings */
#define CONFIG_EXTRA_ENV_SETTINGS              CONFIG_EXTRA_ENV_USBTTY

#endif  /* __CONFIG_H */
