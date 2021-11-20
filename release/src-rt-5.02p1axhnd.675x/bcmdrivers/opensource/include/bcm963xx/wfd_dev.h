#ifndef __WFD_DEV_H_INCLUDED__
#define __WFD_DEV_H_INCLUDED__
/*
<:copyright-BRCM:2014:DUAL/GPL:standard

   Copyright (c) 2014 Broadcom 
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

#include <linux/netdevice.h>
#include <linux/bcm_colors.h>
#include "pktHdr.h"
#include "bcm_assert_locks.h"

/** WFD return codes */
#define WFD_FAILURE         (-1)
#define WFD_SUCCESS         (0)

#define WFD_ASSERT(exp)     BCM_ASSERT_A(exp)

#define WFD_ERROR(fmt, arg...) \
    printk(CLRr "%s: " fmt CLRnl, __FUNCTION__, ##arg)

typedef enum {
    WFD_WL_FWD_HOOKTYPE_INVALID,
    WFD_WL_FWD_HOOKTYPE_SKB,
    WFD_WL_FWD_HOOKTYPE_FKB
} enumWFD_WlFwdHookType;

struct pktlist_context; /* fwd declaration for BCM_PKTFWD library */
extern int wfd_bind(struct net_device *wl_dev_p,
                    struct pktlist_context *wl_pktlist_context,
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

#endif /* __WFD_DEV_H_INCLUDED__ */
