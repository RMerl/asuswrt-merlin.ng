/*
    Copyright (c) 2019 Broadcom
    All Rights Reserved

    <:label-BRCM:2019:DUAL/GPL:standard
    
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
