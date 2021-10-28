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

#if !defined(__dhd_awl_h_included__)
#define      __dhd_awl_h_included__

#if defined(BCM_AWL)

/**
 * =============================================================================
 * WLAN Packet Forwarding using Archer Wireless
 * =============================================================================
 */

#include <dhd.h>


/**
 * module Interface
 */
/*
 * -----------------------------------------------------------------------------
 * AWL external interface functions
 * -----------------------------------------------------------------------------
 */
extern void* dhd_awl_attach(dhd_pub_t *dhdp, uint unit);
extern void dhd_awl_detach(dhd_pub_t *dhdp, void *awl);
extern void dhd_awl_dump(dhd_pub_t *dhdp, struct bcmstrbuf *b);

/**
 * Downstream (Tx) Path Interface
 */
/* All Tx path interface is handled in dhd_wfd/dhd_pktfwd modules */
#define DHD_AWL_TX

/**
 * Upstream (Rx) Path Interface
 */

/*
 * -----------------------------------------------------------------------------
 * AWL processing pre-processor flags
 * -----------------------------------------------------------------------------
 */
/* Default disabled until Archer WLAN is fully integrated */
#define DHD_AWL_RX


/*
 * -----------------------------------------------------------------------------
 * AWL RX external interface functions
 * -----------------------------------------------------------------------------
 */
#if defined(DHD_AWL_RX)

extern int dhd_awl_upstream_add(dhd_pub_t *dhdp, void *pkt, uint ifidx);
extern int dhd_awl_upstream_send(dhd_pub_t *dhdp);
extern bool dhd_awl_process_slowpath_rxpkts(dhd_pub_t *dhdp, int rxbound);

#define dhd_rxchain_frame                      dhd_awl_upstream_add
#define dhd_rxchain_commit                     dhd_awl_upstream_send

#else /* !DHD_AWL_RX */

/* Pass through mode */
#define dhd_rxchain_frame(dhdp, pkt, ifidx)    dhd_bus_rx_frame((dhdp)->bus, pkt, ifidx, 1)
#define dhd_rxchain_commit(dhdp)               do { } while (0)

#endif /* !DHD_AWL_RX */

#endif /* BCM_AWL */
#endif /* __dhd_awl_h_included__ */
