#ifndef __WFD_DEV_H_INCLUDED__
#define __WFD_DEV_H_INCLUDED__
/*
<:copyright-BRCM:2014:DUAL/GPL:standard

   Copyright (c) 2014 Broadcom 
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

#include <linux/netdevice.h>
#include "pktHdr.h"

typedef enum {
    WFD_WL_FWD_HOOKTYPE_INVALID,
    WFD_WL_FWD_HOOKTYPE_SKB,
    WFD_WL_FWD_HOOKTYPE_FKB
}enumWFD_WlFwdHookType;

extern int wfd_bind(struct net_device *wl_dev_p, 
                    enumWFD_WlFwdHookType eFwdHookType, 
                    bool isTxChainingReqd,
                    HOOK4PARM wfd_fwdHook, 
                    HOOK32 wfd_completeHook,
                    HOOK3PARM wfd_mcastHook,
                    int wl_radio_idx);

extern void wfd_unbind(int wfd_idx, enumWFD_WlFwdHookType hook_type);
extern int wfd_registerdevice(uint32_t wfd_idx, int ifidx, struct net_device *dev);
extern int wfd_unregisterdevice(uint32_t wfd_idx, int ifidx);
extern struct net_device *wfd_dev_by_id_get(uint32_t radio_id, uint32_t if_id);
#endif

