/*
    Copyright (c) 2019 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

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
