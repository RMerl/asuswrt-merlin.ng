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

#include "pmc_drv.h"
#include "power_rpc_svc.h"
#include "pmc_ethtop.h"

static char *name[] = {
    [ETHTOP_COMMON] = "ethtop",
    [ETHTOP_MDIO] = "mdio",
    [ETHTOP_QGPHY] = "qgphy",
    [ETHTOP_XPORT0] = "xport0",
    [ETHTOP_XPORT1] = "xport1",
    [ETHTOP_XPORT2] = "xport2",
};

int pmc_ethtop_power_up(eth_block_t block_id)
{
    if (block_id >= ETHTOP_LAST)
    {
        printk("%s ERROR Unsupported block = [%d]\n",__FUNCTION__, block_id);
        return -1;
    }

    return bcm_rpc_pwr_set_domain_state(name[block_id], PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);
}

int pmc_ethtop_power_down(eth_block_t block_id)
{
    if (block_id >= ETHTOP_LAST)
    {
        printk("%s ERROR Unsupported block = [%d]\n",__FUNCTION__, block_id);
        return -1;
    }

    return bcm_rpc_pwr_set_domain_state(name[block_id], PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);
}
