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
#ifndef _PMC_ETHTOP_H
#define _PMC_ETHTOP_H

#if defined(CONFIG_BCMBCA_PMC_ETHTOP) && defined(CONFIG_SMC_BASED) && !defined(CONFIG_BCMBCA_NO_SMC_BOOT)
typedef enum {
    ETHTOP_COMMON,
    ETHTOP_MDIO, 
    ETHTOP_QGPHY,
    ETHTOP_XPORT0,
    ETHTOP_XPORT1,
    ETHTOP_XPORT2,
    ETHTOP_LAST, //Must be last
} eth_block_t;

int pmc_ethtop_power_up(eth_block_t block_id);
int pmc_ethtop_power_down(eth_block_t block_id);
#else
#define pmc_ethtop_power_up(x) (0)
#define pmc_ethtop_power_down(x) (0)
#endif

#endif
