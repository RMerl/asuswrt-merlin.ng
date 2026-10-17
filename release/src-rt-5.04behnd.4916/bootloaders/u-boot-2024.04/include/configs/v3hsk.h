/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/configs/v3hsk.h
 *     This file is V3HSK board configuration.
 *
 * Copyright (C) 2019 Renesas Electronics Corporation
 * Copyright (C) 2019 Cogent Embedded, Inc.
 */

#ifndef __V3HSK_H
#define __V3HSK_H

#include "rcar-gen3-common.h"

/* Environment compatibility */

/* SH Ether */
#define CFG_SH_ETHER_USE_PORT	0
#define CFG_SH_ETHER_PHY_ADDR	0x0
#define CFG_SH_ETHER_PHY_MODE	PHY_INTERFACE_MODE_RGMII_ID
#define CFG_SH_ETHER_CACHE_WRITEBACK
#define CFG_SH_ETHER_CACHE_INVALIDATE
#define CFG_SH_ETHER_ALIGNE_SIZE	64

/* Board Clock */
/* XTAL_CLK : 33.33MHz */

#endif /* __V3HSK_H */
