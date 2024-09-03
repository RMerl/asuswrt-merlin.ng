/*
    Copyright (c) 2019 Broadcom
    All Rights Reserved

    <:label-BRCM:2019:DUAL/GPL:standard
    
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

/*
*******************************************************************************
* File Name  : bcm_archer.h
*
* Description: Archer Header File
*
*******************************************************************************
*/

#ifndef __BCM_ARCHER_H__
#define __BCM_ARCHER_H__

/*
**********************************
* Interface for WLAN
**********************************
*/

#include <linux/bcm_log.h>

#include "pktHdr.h"

#define ARCHER_WLAN_WL_PRIO(skb)   (skb)->wl.awl.wl_prio
#define ARCHER_WLAN_RADIO_IDX(skb) (skb)->wl.awl.radio
#define ARCHER_WLAN_INTF_IDX(skb)  (skb)->wl.awl.ifidx

typedef enum {
    ARCHER_WLAN_RADIO_MODE_SKB,
    ARCHER_WLAN_RADIO_MODE_FKB,
    ARCHER_WLAN_RADIO_MODE_MAX
} archer_wlan_radio_mode_t;


/* WLAN flow-miss using pktlist instead of single skb */
#define ARCHER_WLAN_MISS_PKTLIST
#define ARCHER_WLAN_RX_BUDGET              64

typedef struct {
    struct net_device *dev_p;
    struct pktlist_context *wl_pktlist_context;
    archer_wlan_radio_mode_t mode;
    HOOK32 wl_completeHook;
    int wl_radio_idx;
} archer_wlan_bind_arg_t;

static inline int archer_wlan_bind(struct net_device *dev_p,
                                   struct pktlist_context *wl_pktlist_context,
                                   archer_wlan_radio_mode_t mode,
                                   HOOK32 wl_completeHook,
                                   int wl_radio_idx)
{
    archer_wlan_bind_arg_t arg = { .dev_p = dev_p,
                                   .wl_pktlist_context = wl_pktlist_context,
                                   .mode = mode,
                                   .wl_completeHook = wl_completeHook,
                                   .wl_radio_idx = wl_radio_idx };

    bcmFun_t *hook = bcmFun_get(BCM_FUN_ID_ARCHER_WLAN_BIND);

    return hook(&arg);
}

typedef struct {
    int radio_index;
} archer_wlan_unbind_arg_t;

static inline int archer_wlan_unbind(int radio_index)
{
    archer_wlan_unbind_arg_t arg = { .radio_index = radio_index };

    bcmFun_t *hook = bcmFun_get(BCM_FUN_ID_ARCHER_WLAN_UNBIND);

    return hook(&arg);
}

typedef void (* archer_wlan_rx_miss_handler_t)(void *context, pktlist_t *pktl_p);

typedef struct {
    int radio_index;
    archer_wlan_rx_miss_handler_t handler;
    void *context;
} archer_wlan_rx_register_arg_t;

static inline int archer_wlan_rx_register(int radio_index,
                                          archer_wlan_rx_miss_handler_t handler,
                                          void *context)
{
    archer_wlan_rx_register_arg_t arg = { .radio_index = radio_index,
                                          .handler = handler,
                                          .context = context };

    bcmFun_t *hook = bcmFun_get(BCM_FUN_ID_ARCHER_WLAN_RX_REGISTER);

    return hook(&arg);
}

static inline void archer_wlan_rx_send(void *void_p, bcmFun_t *hook)
{
    hook(void_p);
}

#endif /* __BCM_ARCHER_H__ */
