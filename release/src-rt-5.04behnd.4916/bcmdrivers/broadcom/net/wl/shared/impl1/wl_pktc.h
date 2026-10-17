/*
    Copyright (c) 2017 Broadcom
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

#ifndef _wl_pktc_h_
#define _wl_pktc_h_

#include <typedefs.h>
#include <osl.h>

#include <dhd_nic_common.h>

#if !defined(BCM_PKTFWD)

#include <linux/netdevice.h>
#include <linux/skbuff.h>

struct wl_info;
struct wl_if;

typedef int (*HardStartXmitFuncP) (struct sk_buff * skb, struct net_device * dev);

struct wl_pktc_tbl {
};
typedef struct wl_pktc_tbl wl_pktc_tbl_t;

#define PKTC_TBL_FN_CMP(brc, da, rxifp) \
    pktc_tbl_cmp_fn((wl_pktc_tbl_t *)brc, da, rxifp)

static inline int pktc_tbl_cmp_fn(wl_pktc_tbl_t * tbl, uint8_t * da, struct net_device *rxifp)
{
	return 0;
}

/* PKTC requests */
#define PKTC_TBL_INIT			1	/* Initialize pktc table */
#define PKTC_TBL_GET_BY_DA		2	/* Get pktc_tbl for pkt chaining by dest addr */
#define PKTC_TBL_GET_BY_IDX		3	/* Get pktc_tbl for pkt chaining by table idx */
#define PKTC_TBL_UPDATE			4	/* To update pktc_tbl entry */
#define PKTC_TBL_UPDATE_WLAN_HANDLE	5	/* To update wlan handle */
#define PKTC_TBL_SET_TX_MODE		6	/* To set pktc tx mode: enabled=0, disabled=1 */
#define PKTC_TBL_GET_TX_MODE		7	/* To get pktc tx mode: enabled=0, disabled=1 */
#define PKTC_TBL_DELETE			8	/* To delete pktc_tbl entry */
#define PKTC_TBL_DUMP			9	/* To dump pktc_tbl table */
#define PKTC_TBL_GET_START_ADDRESS	10	/* Get the address/top of pktc_tbl table */
#define PKTC_TBL_DELETE_WLAN_HANDLE	11	/* To delete wlan handle in wldev table */
#define PKTC_TBL_UPDATE_WFD_IDX_BY_DEV	12	/* To update WFD Index by WLAN Dev */
#define PKTC_TBL_FLUSH			13	/* To flush out entire pktc_tbl table */
#define PKTC_TBL_SET_STA_ASSOC		14	/* To set STA state in the pktc_tbl entry */
#define PKTC_TBL_BRIDGE_EVENT		15	/* To handle bridge events */

extern unsigned long wl_pktc_req(int rid, unsigned long p0, unsigned long p1,
		unsigned long p2);

#define PKTC_CHAIN_IDX_MASK 0xFFFF	/* 16b chain idx */
#define PKTC_INVALID_CHAIN_IDX	0xFFFF
#define PKTC_WFD_IDX_BITMASK	(0x30000)
#define PKTC_WFD_IDX_BITPOS	16

extern wl_pktc_tbl_t *wl_pktc_attach(struct wl_info *wl, struct wl_if *wlif);
extern void wl_pktc_detach(struct wl_info *wl);

extern int wl_pktc_init(struct wl_if *wlif, struct net_device *dev);
extern void wl_pktc_free(struct wl_if *wlif);
extern int32 wl_rxchainhandler(struct wl_info *wl, struct sk_buff *skb);


#else /* BCM_PKTFWD -------------------------------------------------------- */

/**
 * Transpose PKTC identifiers to wl_pktfwd.h
 */
#include <wl_pktfwd.h>

/** Use in bridge layer: return value of wl_pktc_req_hook(PKTC_TBL_UPDATE) is
 * tested whether valid or not.
 * If valid,
 *   wfd_idx is extracted using PKTC_WFD_IDX_BITMASK, PKTC_WFD_IDX_BITPOS.
 *   pktfwd key is passed to network processor via blog.
 */
#ifndef CONFIG_BCM_WLAN_16BIT_STATION_CHAIN_IDX_SUPPORT
#error "BCM_PKTFWD requires 16bit key"
#endif

#define PKTC_CHAIN_IDX_MASK         0xFFFF	/* 16b chain idx */
#define PKTC_INVALID_CHAIN_IDX	    WL_PKTFWD_KEY_INVALID
#define PKTC_WFD_IDX_BITMASK	    (0x30000)
#define PKTC_WFD_IDX_BITPOS	    16

/** Given a per WFD chain index, convert to global WFD_CHAIN index */
#define PKTC_WFD_CHAIN_IDX(wfd_idx, chain_idx) \
	((((wfd_idx) << PKTC_WFD_IDX_BITPOS) & PKTC_WFD_IDX_BITMASK) | \
		((chain_idx) & PKTC_CHAIN_IDX_MASK))

/** Invoked in wlc driver in rx ischainable: wlc_rx.c, wlc_amsdu.c */
#define PKTC_TBL_FN_CMP(tbl, sym, dev)  wl_pktfwd_match((sym), (dev))

/** wl_pktc_req requests redirected to wl_pktfwd_request */
#define PKTC_TBL_SET_TX_MODE        wl_pktfwd_req_set_txmode_e	/*         wlc */
#define PKTC_TBL_GET_TX_MODE        wl_pktfwd_req_get_txmode_e	/* bridge, wlc */
#define PKTC_TBL_UPDATE             wl_pktfwd_req_ins_symbol_e	/* bridge      */
#define PKTC_TBL_FLUSH              wl_pktfwd_req_flush_full_e	/*         wl  */
#define PKTC_TBL_SET_STA_ASSOC      wl_pktfwd_req_assoc_sta_e	/*         wls */
#define PKTC_TBL_BRIDGE_EVENT       wl_pktfwd_req_bridge_event_e	/* bridge  */

#define wl_pktc_req(rq, p0, p1, p2) wl_pktfwd_request((rq), (p0), (p1), (p2))

/** Redirect wlif init/free and radio attach/detach to wl_pktfwd ins/del */
#define wl_pktc_init(wlif, dev)     wl_pktfwd_wlif_ins(wlif)
#define wl_pktc_free(wlif)          wl_pktfwd_wlif_del(wlif)

#define wl_pktc_attach(wl, wlif)    wl_pktfwd_radio_ins(wl)
#define wl_pktc_detach(wl)          wl_pktfwd_radio_del(wl)

#define wl_rxchainhandler(wl, pkt)  wl_pktfwd_upstream((wl), (pkt))

#endif /* BCM_PKTFWD -------------------------------------------------------- */

#endif /* _wl_pktc_h_ */
