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
