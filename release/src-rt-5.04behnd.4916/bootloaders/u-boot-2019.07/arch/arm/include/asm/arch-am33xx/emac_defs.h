/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2010 Texas Instruments
 *
 * Based on:
 *
 * ----------------------------------------------------------------------------
 *
 * dm644x_emac.h
 *
 * TI DaVinci (DM644X) EMAC peripheral driver header for DV-EVM
 *
 * Copyright (C) 2005 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 *
 */

#ifndef _EMAC_DEFS_H_
#define _EMAC_DEFS_H_

#ifdef CONFIG_TI816X
#define EMAC_BASE_ADDR			(0x4A100000)
#define EMAC_WRAPPER_BASE_ADDR		(0x4A100900)
#define EMAC_WRAPPER_RAM_ADDR		(0x4A102000)
#define EMAC_MDIO_BASE_ADDR		(0x4A100800)
#define EMAC_MDIO_BUS_FREQ		(250000000UL)
#define EMAC_MDIO_CLOCK_FREQ		(2000000UL)

typedef volatile unsigned int	dv_reg;
typedef volatile unsigned int	*dv_reg_p;

#define DAVINCI_EMAC_VERSION2
#define DAVINCI_EMAC_GIG_ENABLE
#endif

#endif  /* _EMAC_DEFS_H_ */
