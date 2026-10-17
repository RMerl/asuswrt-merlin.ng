/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
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
 * Modifications:
 * ver. 1.0: Sep 2005, TI PSP Team - Created EMAC version for uBoot.
 */

#ifndef _DM644X_EMAC_H_
#define _DM644X_EMAC_H_

#include <asm/arch/hardware.h>

#define EMAC_BASE_ADDR			DAVINCI_EMAC_CNTRL_REGS_BASE
#define EMAC_WRAPPER_BASE_ADDR		DAVINCI_EMAC_WRAPPER_CNTRL_REGS_BASE
#define EMAC_WRAPPER_RAM_ADDR		DAVINCI_EMAC_WRAPPER_RAM_BASE
#define EMAC_MDIO_BASE_ADDR		DAVINCI_MDIO_CNTRL_REGS_BASE
#define DAVINCI_EMAC_VERSION2

/* MDIO module input frequency */
#define EMAC_MDIO_BUS_FREQ		clk_get(DAVINCI_MDIO_CLKID)
/* MDIO clock output frequency */
#define EMAC_MDIO_CLOCK_FREQ		2000000		/* 2.0 MHz */

#endif  /* _DM644X_EMAC_H_ */
