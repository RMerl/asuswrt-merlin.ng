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

int pmc_shutdown(wake_type_t wake_type, int param)
{
    switch (wake_type)
    {
    case WAKE_XPORT:
        printk("pmc_shutdown() wake on XPORT %d\n", param);
        bcm_rpc_ba_wol_intr_enable();
        break;
    case WAKE_IRQ:
        printk("pmc_shutdown() wake on IRQ %d\n", param);
        bcm_rpc_ba_wol_intr_enable();
        break;
    case WAKE_TIMER:
        printk("pmc_shutdown() wake on timer %d (N/A)\n", param);
        break;
    default:
        printk("pmc_shutdown() wake type %d is not implemented yet\n", param);
    }

    return 0;
}
