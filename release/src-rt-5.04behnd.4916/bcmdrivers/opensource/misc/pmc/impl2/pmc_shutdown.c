/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom 
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
 * Author: Yuval Raviv <yuval.raviv@broadcom.com>
*****************************************************************************/
#include "pmc_shutdown.h"
#include "ba_rpc_svc.h"

int pmc_setup_wake_trig(wake_type_t wake_type, int param)
{
    int rc = -1;
    switch (wake_type)
    {
    case WAKE_XPORT:
        printk("pmc_setup_wake_trig() wake on XPORT %d\n", param);
        break;
    case WAKE_IRQ:
    case WAKE_IRQ_WOL:
        printk("pmc_setup_wake_trig() wake on IRQ %d\n", param);
        break;
    case WAKE_TIMER:
        printk("pmc_setup_wake_trig() wake on timer in %d min)\n", param);
        break;
    default:
        printk("pmc_setup_wake_trig() wake type %d is not implemented yet\n", param);
        return rc;
    }

    rc = bcm_rpc_ba_setup_wake_trigger(wake_type, param);
    return rc;
}

int pmc_deep_sleep(void)
{
    return bcm_rpc_ba_deep_sleep();
}
