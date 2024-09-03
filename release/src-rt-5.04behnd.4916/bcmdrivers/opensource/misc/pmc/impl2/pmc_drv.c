/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2022 Broadcom Ltd.
 */
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

/*****************************************************************************
 *  Description:
 *      Code for PMC Linux
 *****************************************************************************/

#include "pmc_drv.h"
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/of_fdt.h>
#include "pmc_core_api.h"
#include "avs_svc.h"
#include "pmc_rpc.h"

static uint8_t avs_svc_msg_get_retcode(rpc_msg *msg)
{
    return (uint8_t)msg->data[2];
}

int GetPVTKH2(int sel, int island, int *value)
{
    rpc_msg msg;
    int ret = 0;

    rpc_msg_init(&msg, RPC_SERVICE_AVS, AVS_SVC_GET_DATA ,RPC_SERVICE_VER_AVS_GET_DATA, 0, 0, 0);
    switch (sel){
    case kTEMPERATURE:
        msg.data[0] = kDIE_TEMP;
        break;
    case kV_VIN:
        if (island)
            msg.data[0] = kCPU_VIN;
        else
            msg.data[0] = kCORE_VIN;

        break;
    default:
        printk("ERROR: not supported selector (%d)\n", sel);
        return -EINVAL;
    }
    ret = pmc_svc_request(&msg, avs_svc_msg_get_retcode);
    if (ret)
    {
        printk("ERROR: avs_svc: failure (%d)\n", ret);
        return ret;
    }

    *value = (int)msg.data[1];

    return ret;
}
EXPORT_SYMBOL(GetPVTKH2);

int pmc_convert_pvtmon(int sel, int value)
{
    return value;
}

EXPORT_SYMBOL(pmc_convert_pvtmon);

int GetRCalSetting_1UM_VERT(int *rcal)
{
	return 0;
}
EXPORT_SYMBOL(GetRCalSetting_1UM_VERT);

