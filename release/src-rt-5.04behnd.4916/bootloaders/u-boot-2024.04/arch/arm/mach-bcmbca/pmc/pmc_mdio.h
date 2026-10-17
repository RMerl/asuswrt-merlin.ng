// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2024 Broadcom Corporation
   All Rights Reserved
*/

/****************************************************************************
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/
#ifndef _PMC_MDIO_H
#define _PMC_MDIO_H

#if defined(CONFIG_BCMBCA_PMC_ETHTOP) && defined(CONFIG_SMC_BASED)
#include "pmc_ethtop.h"
static inline int pmc_mdio_power_up(void)
{
    return pmc_ethtop_power_up(ETHTOP_COMMON);
}

static inline int pmc_mdio_power_down(void)
{
    return pmc_ethtop_power_down(ETHTOP_COMMON);
}
#else
#define pmc_mdio_power_up() (0)
#define pmc_mdio_power_down() (0)
#endif

#endif
