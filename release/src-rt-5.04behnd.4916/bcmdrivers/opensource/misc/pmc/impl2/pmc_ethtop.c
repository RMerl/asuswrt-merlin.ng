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
