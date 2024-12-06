/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/
/****************************************************************************
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/
#ifndef _PMC_MDIO_H
#define _PMC_MDIO_H

#if defined(CONFIG_BCMBCA_PMC_ETHTOP) && defined(CONFIG_SMC_BASED) && !defined(CONFIG_BCMBCA_NO_SMC_BOOT)
#include "pmc_ethtop.h"
static inline int pmc_mdio_power_up(void)
{
    return pmc_ethtop_power_up(ETHTOP_MDIO);
}

static inline int pmc_mdio_power_down(void)
{
    return pmc_ethtop_power_down(ETHTOP_MDIO);
}
#else
#define pmc_mdio_power_up() (0)
#define pmc_mdio_power_down() (0)
#endif

#endif
