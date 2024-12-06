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
 * PMC RPC Service Driver
 *
 * Author: Samyon Furman <samyon.furman@broadcom.com>
*****************************************************************************/

#include <asm/cacheflush.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <itc_rpc.h>
#include <pmc_rpc.h>

//#define DEBUG 1

#define RPC_REQUEST_TIMEOUT              5 /* sec */

static int rpc_svc_request(char *tname, rpc_msg *msg, get_retcode_t cb)
{
    int ret = 0;
    int tunnel = 0;

    tunnel = rpc_get_fifo_tunnel_id(tname);
    if (tunnel < 0)
    {
        printk("%s:%d : Error: invalid tunnel %s\n",__FUNCTION__,__LINE__, tname);
        ret = tunnel;
        return -1;
    }

    ret = rpc_send_request_timeout(tunnel, msg, RPC_REQUEST_TIMEOUT);

#ifdef  DEBUG
    rpc_dump_msg(msg);
#endif
    if (ret)
    {
        printk("%s:%d : ERROR: rpc_send_request failure (%d)\n",__FUNCTION__,__LINE__, ret);
        return -1;
    }

    if (cb)
    {
        ret = cb(msg);
        if (ret)
            printk("%s:%d : ERROR: rpc_send_request failure (%d)\n",__FUNCTION__,__LINE__, ret);
    }
       
    return ret;
}

int pmc_svc_request(rpc_msg *msg, get_retcode_t cb)
{
    return rpc_svc_request("rg-smc", msg, cb);
}

int avs_svc_request(rpc_msg *msg, get_retcode_t cb)
{
    return rpc_svc_request("avs-smc", msg, cb);
}

