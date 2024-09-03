/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/
/****************************************************************************
 *
 * Author: Dima Mamut <dima.mamut@broadcom.com>
*****************************************************************************/
#ifndef _PMC_MDIO_H
#define _PMC_MDIO_H

#if defined(CONFIG_BRCM_SMC_BOOT)
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
