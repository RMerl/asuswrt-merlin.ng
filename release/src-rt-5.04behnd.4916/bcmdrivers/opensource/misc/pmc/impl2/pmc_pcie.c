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
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include "pmc_drv.h"
#include <bp3_license.h>

/**
 * Re-configure UBUS4 for the given PCI-E core.
 * - master token credits for PCIE -> Runner access
 * - master decode window
 */

#define PCIE_NUM_MAX  (4)
char pcie_domain_name[PCIE_NUM_MAX][PWR_DOMAIN_NAME_MAX_LEN] = { "pcie0", "pcie1", "pcie2", "pcie3"};

int pmc_pcie_power_up(int unit, int is_dual_lane)
{
    int ret = 0;

    if (unit >= PCIE_NUM_MAX)
    {
        printk("%s: ERROR: pcie_unit[%d] > PCIE_NUM_MAX[4]\n",__FUNCTION__, unit);
        return -1;
    }

    if (bcm_license_check_msg(BP3_FEATURE_PCIE0 + unit) <= 0)
        return -1;

    ret = bcm_rpc_pwr_set_domain_state(pcie_domain_name[unit], PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_ON, PWR_DOM_RESET_DEASSERT);

    return ret; 
}

int pmc_pcie_power_down(int unit, int is_dual_lane)
{
    int ret = 0;

    if (unit > PCIE_NUM_MAX)
    {
        printk("%s: ERROR: pcie_unit[%d] > PCIE_NUM_MAX[4]\n",__FUNCTION__, unit);
        return -1;
    }

    ret = bcm_rpc_pwr_set_domain_state(pcie_domain_name[unit], PWR_DOMAIN_NAME_MAX_LEN, PWR_DOM_STATE_OFF, PWR_DOM_RESET_ASSERT);

    return ret;     
}

EXPORT_SYMBOL(pmc_pcie_power_up);
EXPORT_SYMBOL(pmc_pcie_power_down);
