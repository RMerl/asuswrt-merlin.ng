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

