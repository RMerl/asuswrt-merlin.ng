/*
<:copyright-BRCM:2023:DUAL/GPL:standard

   Copyright (c) 2023 Broadcom
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
