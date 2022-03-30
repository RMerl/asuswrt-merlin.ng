/*
    Copyright (c) 2019 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
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

#if !defined(__wl_awl_h_included__)
#define      __wl_awl_h_included__

#if defined(BCM_AWL)

/**
 * =============================================================================
 * WLAN Packet Forwarding using Archer Wireless
 * =============================================================================
 */

/**
 * module Interface
 */
/*
 * -----------------------------------------------------------------------------
 * AWL external interface functions
 * -----------------------------------------------------------------------------
 */
extern void* wl_awl_attach(struct wl_info *wl, uint unit);
extern void wl_awl_detach(struct wl_info *wl, void *awl);

extern void wl_awl_register_dev(struct net_device *dev);
extern void wl_awl_unregister_dev(struct net_device *dev);

/**
 * Downstream (Tx) Path Interface
 */
/* All Tx path interface is handled in dhd_wfd/dhd_pktfwd modules */
#define WL_AWL_TX

/**
 * Upstream (Rx) Path Interface
 */

/*
 * -----------------------------------------------------------------------------
 * AWL processing pre-processor flags
 * -----------------------------------------------------------------------------
 */
#define WL_AWL_RX


/*
 * -----------------------------------------------------------------------------
 * AWL RX external interface functions
 * -----------------------------------------------------------------------------
 */
#if defined(WL_AWL_RX)

extern int  wl_awl_upstream_match(uint8_t * d3addr, struct net_device * rx_net_device);
extern int  wl_awl_upstream_send_chain(struct wl_info * wl, struct sk_buff * pkt);
extern void wl_awl_upstream_add_pkt(struct wl_info * wl, struct net_device * net_device,
                           void * pkt, uint16_t flowid);
extern void wl_awl_upstream_send_all(struct wl_info *wl);

extern bool wl_awl_process_slowpath_rxpkts(struct wl_info *wl);

#endif /* WL_AWL_RX */

#endif /* BCM_AWL */
#endif /* __wl_awl_h_included__ */
