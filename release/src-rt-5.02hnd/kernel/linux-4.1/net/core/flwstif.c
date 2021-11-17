#if defined(CONFIG_BCM_KF_NBUFF)
/*--------------------------------------*/
/* flwstif.h and flwstif.c for Linux OS */
/*--------------------------------------*/

/* 
* <:copyright-BRCM:2014:DUAL/GPL:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
:>
*/

#include <linux/flwstif.h>
#include <linux/module.h>

static flwStIfGetHook_t flwStIfGet_hook_g = NULL;
static flwStIfPushHook_t flwStIfPush_hook_g = NULL; 

uint32_t flwStIf_request( FlwStIfReq_t req, void *ptr, unsigned long param1,
                          uint32_t param2, uint32_t param3, void *param4 )
{
    int ret=0;
    switch (req)
    {
        case FLWSTIF_REQ_GET:
            if (flwStIfGet_hook_g)
            {
                ret = flwStIfGet_hook_g(param1, (FlwStIf_t *)ptr);
            }
            else
            {
                ret = -1;
            }
            break;
        case FLWSTIF_REQ_PUSH:
            if (flwStIfPush_hook_g)
            {
                ret = flwStIfPush_hook_g(ptr, (void *)param1, param2, 
                                         param3, (FlwStIf_t *)param4);
            }
            else
            {
                ret = -1;
            }
            break;
        default:
            printk("Invalid Flw Stats Req type %d\n", (int)req);
            ret = -1;
            break;
    }
    return ret;
}

void flwStIf_bind( flwStIfGetHook_t flwStIfGetHook,
                   flwStIfPushHook_t flwStIfPushHook )
{
    if (flwStIfGetHook)
    {
        flwStIfGet_hook_g = flwStIfGetHook;
    }

    if (flwStIfPushHook)
    {
        flwStIfPush_hook_g = flwStIfPushHook;
    }
}

EXPORT_SYMBOL(flwStIf_bind);
EXPORT_SYMBOL(flwStIf_request);
#endif
