/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/condor.h
 *     This file is Condor board configuration.
 *
 * Copyright (C) 2019 Renesas Electronics Corporation
 */

#ifndef __CONDOR_H
#define __CONDOR_H

#include "rcar-gen3-common.h"

/* Environment compatibility */

/* SH Ether */
#define CFG_SH_ETHER_USE_PORT	0
#define CFG_SH_ETHER_PHY_ADDR	0x1
#define CFG_SH_ETHER_PHY_MODE PHY_INTERFACE_MODE_RMII
#define CFG_SH_ETHER_CACHE_WRITEBACK
#define CFG_SH_ETHER_CACHE_INVALIDATE
#define CFG_SH_ETHER_ALIGNE_SIZE	64

/* Board Clock */
/* XTAL_CLK : 33.33MHz */

#endif /* __CONDOR_H */
