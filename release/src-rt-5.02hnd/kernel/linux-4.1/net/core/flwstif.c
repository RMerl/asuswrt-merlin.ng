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
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
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
