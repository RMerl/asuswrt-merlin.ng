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
 * PVT access driver
 *
 * Author: Samyon Furman <samyon.furman@broadcom.com>
*****************************************************************************/
#include <linux/errno.h>
#include "itc_rpc.h"
#include "pmc_drv.h"
#include <linux/types.h>
#include <api_public.h>
#include <string.h>
#include <stdio.h>

enum avs_svc_func_idx
{
    AVS_SVC_HOST_CMD,
    AVS_SVC_SET_INIT_PARAMS,
    AVS_SVC_GET_INIT_PARAMS,
    AVS_SVC_GET_DATA,
    AVS_SVC_FUNC_MAX
};

enum pmc_svc_data_idx
{
    kDIE_TEMP,
    kCORE_VIN,
    kCPU_VIN,
    kCPU_FREQ,
    kCPU_FREQ_MHz,
    k_VIN_1p8,
    k_VIN_3p3,
    k_VIN_1p2,
};

#define RPC_SERVICE_VER_AVS_GET_DATA            (0)
#define AVS_SVC_RPC_REQUEST_TIMEOUT          (1) /* sec */

static uint8_t avs_svc_msg_get_retcode(rpc_msg *msg)
{
    return (uint8_t)msg->data[2];
}

static inline int pmc_svc_request(rpc_msg *msg)
{
    int ret = 0;
   
    ret = rpc_send_request_timeout(RPC_TUNNEL_ARM_SMC_NS, msg, AVS_SVC_RPC_REQUEST_TIMEOUT);
    if (ret) 
    {
        printf("%s:%d : ERROR: avs_svc: rpc_send_request failure (%d)\n",__FUNCTION__, __LINE__, ret);
        return -1;
    }

    ret = avs_svc_msg_get_retcode(msg);
    if (ret) 
        printf("%s:%d : ERROR: avs_svc: rpc_send_request failure (%d)\n",__FUNCTION__, __LINE__, ret);

    return ret;
}

int GetPVT(int sel, int island, int *value)
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
    ret = pmc_svc_request(&msg);
    if (ret)
    {
        printk("ERROR: avs_svc: failure (%d)\n", ret);
        return ret;
    }

    *value = (int)msg.data[1];

    return ret;
}

int pmc_convert_pvtmon(int sel, int value)
{
    return value;
}


