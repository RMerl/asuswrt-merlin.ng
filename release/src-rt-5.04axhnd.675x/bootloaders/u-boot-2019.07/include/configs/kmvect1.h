/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *                    Dave Liu <daveliu@freescale.com>
 *
 * Copyright (C) 2007 Logic Product Development, Inc.
 *                    Peter Barada <peterb@logicpd.com>
 *
 * Copyright (C) 2007 MontaVista Software, Inc.
 *                    Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * (C) Copyright 2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */

#define CONFIG_HOSTNAME		"kmvect1"
#define CONFIG_KM_BOARD_NAME   "kmvect1"
/* at end of uboot partition, before env */
#define CONFIG_SYS_QE_FW_ADDR   0xF00B0000

/* include common defines/options for all Keymile boards */
#include "km/keymile-common.h"
#include "km/km-powerpc.h"
#include "km/km-mpc83xx.h"
#include "km/km-mpc8309.h"

#define CONFIG_SYS_MAMR	(MxMR_GPL_x4DIS | \
			 0x0000c000 | \
			 MxMR_WLFx_2X)
/*
 * QE UEC ethernet configuration
 */
#define CONFIG_MV88E6352_SWITCH
#define CONFIG_KM_MVEXTSW_ADDR		0x10

/* ethernet port connected to simple switch 88e6122 (UEC0) */
#define CONFIG_UEC_ETH1
#define CONFIG_SYS_UEC1_UCC_NUM		0	/* UCC1 */
#define CONFIG_SYS_UEC1_RX_CLK		QE_CLK9
#define CONFIG_SYS_UEC1_TX_CLK		QE_CLK10

#define CONFIG_FIXED_PHY		0xFFFFFFFF
#define CONFIG_SYS_FIXED_PHY_ADDR	0x1E	/* unused address */
#define CONFIG_SYS_FIXED_PHY_PORT(devnum, speed, duplex) \
		{devnum, speed, duplex}
#define CONFIG_SYS_FIXED_PHY_PORTS \
		CONFIG_SYS_FIXED_PHY_PORT("UEC0", SPEED_100, DUPLEX_FULL)

#define CONFIG_SYS_UEC1_ETH_TYPE	FAST_ETH
#define CONFIG_SYS_UEC1_PHY_ADDR	CONFIG_SYS_FIXED_PHY_ADDR
#define CONFIG_SYS_UEC1_INTERFACE_TYPE	PHY_INTERFACE_MODE_MII
#define CONFIG_SYS_UEC1_INTERFACE_SPEED	100

#endif /* __CONFIG_H */
