// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/****************************************************************************
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/
#ifndef _PMC_QGPHY_H
#define _PMC_QGPHY_H

#if defined(CONFIG_BCM68880) || defined (CONFIG_BCM6837)
#include "pmc_ethtop.h"
static inline int pmc_qgphy_power_up(void)
{
    return pmc_ethtop_power_up(ETHTOP_COMMON);
}

static inline int pmc_qgphy_power_down(void)
{
    return pmc_ethtop_power_down(ETHTOP_COMMON);
}
#else
#define pmc_qgphy_power_up(x) (0)
#define pmc_qgphy_power_down(x) (0)
#endif

#endif
