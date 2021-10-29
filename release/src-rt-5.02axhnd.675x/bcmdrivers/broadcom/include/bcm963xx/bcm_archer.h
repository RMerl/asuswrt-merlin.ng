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

#if defined(CONFIG_BCM_ARCHER_WLAN)

/*
**********************************
* Interface for WLAN
**********************************
*/

#include "pktHdr.h"

/**
 * skb.wl.u32 usage with Archer WLAN
 * 
 * 3..0: radio_index
 * 7..4: interface index
 * 31..8: reserved
 */

typedef struct {
    uint32_t radio    : 4;   /* radio index */
    uint32_t ifidx    : 4;   /* interface index */
    uint32_t reserved : 24;
} archer_wlan_skb_wl_t;

#define ARCHER_WLAN_RADIO_IDX(skb) ((archer_wlan_skb_wl_t *)(&(skb)->wl.u32))->radio
#define ARCHER_WLAN_INTF_IDX(skb)  ((archer_wlan_skb_wl_t *)(&(skb)->wl.u32))->ifidx

typedef enum {
    ARCHER_WLAN_RADIO_MODE_SKB,
    ARCHER_WLAN_RADIO_MODE_FKB,
    ARCHER_WLAN_RADIO_MODE_MAX
} archer_wlan_radio_mode_t;


/* WLAN flow-miss using pktlist instead of single skb */
#define ARCHER_WLAN_MISS_PKTLIST
#define ARCHER_WLAN_RX_BUDGET              64

extern int archer_wlan_bind(struct net_device *dev_p,
                            struct pktlist_context *wl_pktlist_context,
                            archer_wlan_radio_mode_t mode,
                            HOOK32 wl_completeHook,
                            int wl_radio_idx);

extern int archer_wlan_unbind(int radio_index);

typedef void (* archer_wlan_rx_miss_handler_t)(void *context, pktlist_t *pktl_p);

extern int archer_wlan_rx_register(int radio_index, archer_wlan_rx_miss_handler_t handler, void *context);

extern void archer_wlan_rx_send(struct sk_buff *skb_p);

#endif /* CONFIG_BCM_ARCHER_WLAN */
#endif /* __BCM_ARCHER_H__ */
