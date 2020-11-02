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

#if !defined(__wl_pktfwd_h_included__)
#define      __wl_pktfwd_h_included__

/**
 * =============================================================================
 * WLAN Packet Forwarding using a cache of bridged 802.3 end points.
 * =============================================================================
 */

#include <bcm_pktfwd.h>
#include <linux/skbuff.h>

#if defined(BCM_AWL)
#include <wl_awl.h>
#endif /* BCM_AWL */

#define WL_PKTFWD_VERSION           (1)
#define WL_PKTFWD_RELEASE           (1)
#define WL_PKTFWD_PATCH             (0)
#define WL_PKTFWD_VERSIONCODE       PKTFWD_VERSIONCODE(WL_PKTFWD)

#if WL_PKTFWD_VERSIONCODE >= PKTFWD_VERSION(1, 1, 0)
/**
 * On trigger to AMPDU evalutate, appends PKTFWD Ingress pktlist to tail of
 * SCB's AMPDU queue.
 * */
//#define WL_PKTFWD_TXEVAL

#if defined(WLATF)
/** In classic ATF, bound number of pktlists transmitted per wl_thread loop */
#define WL_PKTFWD_RUNQ              (16) /* budget in units of pktlists */
#endif
#endif /* WL_PKTFWD_VERSION >= 1.1.0 */

#define WL_PKTFWD_FAILURE           (-1)
#define WL_PKTFWD_SUCCESS           (0)

#define WL_PKTFWD_RADIOS            (PKTFWD_DOMAINS_WLAN)
#define WL_PKTFWD_DEVICES           (PKTFWD_DEVICES_TOT)

#define WL_PKTFWD_KEY_INVALID       (PKTFWD_KEY_INVALID)
#define WL_PKTFWD_KEY_INVALID_UL    ((unsigned long) WL_PKTFWD_KEY_INVALID)

/* CFP flowId space [1 .. MAXSCB] and PKTFWd LUT space [0 .. MAXSCB) */
#define WL_CFPID2LUTID(cfp_flowid)  ((cfp_flowid) - 1)
#define WL_FWDID2LUTID(lut_index)   ((lut_index) + 1)

/** Forward declarations */
struct wl_if;
struct wl_info;
struct sk_buff;
struct net_device;
struct bcmstrbuf;

typedef struct wl_pktfwd_stats          /* Global System Statistics */
{
    uint32_t        txf_cfp_pkts;       /* CFP bypass fast path */
    uint32_t        txf_chn_pkts;       /* CHN (chaining) fast path */
    uint32_t        txf_fkb_pkts;       /* flowcache wl_xlate_to_skb path */
    uint32_t        rx_fast_fwds;       /* LUT HIT, fast path forwarding */
    uint32_t        rx_slow_fwds;       /* LUT Miss, slow path forwarding */
    uint32_t        tot_stations;       /* Total associated STAs */
    uint32_t        pkts_dropped;       /* Total packets dropped */
    uint32_t        ops_failures;       /* Total request failures */
    uint32_t        pktlist_xmit;       /* Total pktlists xmits */
    uint32_t        xmit_preempt;       /* Total wlif no-credits preempts */
    uint32_t        txeval_xmit;        /* Total pktlists xmits in txeval */
} wl_pktfwd_stats_t;

extern wl_pktfwd_stats_t * wl_pktfwd_stats_gp;

typedef enum wl_pktfwd_req
{                                       /* external usage */
    wl_pktfwd_req_undefined_e     = 0,  /* -------------- */
    wl_pktfwd_req_set_txmode_e    = 1,  /*         wlc    */
    wl_pktfwd_req_get_txmode_e    = 2,  /* bridge, wlc    */
    wl_pktfwd_req_ins_symbol_e    = 3,  /* bridge         */
    wl_pktfwd_req_flush_full_e    = 4,  /*         wl     */
    wl_pktfwd_req_assoc_sta_e     = 5,  /*         wlc    */
    wl_pktfwd_req_pktlist_e       = 6,  /*         wls    */
    wl_pktfwd_req_max_e           = 7
} wl_pktfwd_req_t;

/** Callback registered with WFD, to dispatch pktlists to d3fwd_wlif */
extern void wl_pktfwd_xfer_callback(pktlist_context_t * pktlist_context);

/**
 * PKTFWD API exported to Linux bridge and WLAN driver.
 */
extern unsigned long wl_pktfwd_request(int request,
    unsigned long param0, unsigned long param1, unsigned long param2);

/** PKTFWD 802.3 Address: LAN endpoint or WLAN Station insert/delete/hit/clr */
extern void * wl_pktfwd_lut_ins(uint8_t * d3addr,
                                struct net_device * dev, bool is_wlan);
extern void   wl_pktfwd_lut_del(uint8_t * d3addr,
                                struct net_device * net_device);
extern int    wl_pktfwd_lut_hit(uint8_t * d3addr,
                                struct net_device * net_device);
extern void   wl_pktfwd_lut_clr(struct net_device * dev); /* Flush full LUT */
extern d3lut_t * wl_pktfwd_lut(void); /* accessor function */

/** PKTFWD WLAN Radio insert/delete/debug */
extern int    wl_pktfwd_wlif_ins(struct wl_if * wlif);
extern void   wl_pktfwd_wlif_del(struct wl_if * wlif);
extern void   wl_pktfwd_wlif_dbg(struct wl_if * wlif); /* LOCKLESS */


/** PKTFWD WLAN Radio insert/delete/debug */
extern void * wl_pktfwd_radio_ins(struct wl_info * wl);
extern void   wl_pktfwd_radio_del(struct wl_info * wl);
extern void   wl_pktfwd_radio_dbg(struct wl_info * wl, struct bcmstrbuf * b);
extern pktlist_context_t * wl_pktfwd_pktlist_context(int domain); // debug ONLY

#if defined(BCM_WFD)
extern void   wl_pktfwd_wfd_ins(struct wl_info * wl, int wfd_idx);
extern void   wl_pktfwd_wfd_del(struct wl_info * wl);
extern void   wl_pktfwd_wfd_dbg(struct wl_info * wl);
#endif /* BCM_WFD */

/** PKTFWD global subsystem construction/destruction/debug_dump : LOCKLESS */
extern int    wl_pktfwd_sys_init(void);
extern void   wl_pktfwd_sys_fini(void);
extern void   wl_pktfwd_sys_dump(void);
extern void   wl_pktfwd_sys_clr(struct wl_info * wl);

typedef struct wl_pktfwd_pktlist
{
    struct sk_buff  * head;       /* CFP linked pktlist head */
    struct sk_buff  * tail;       /* CFP linked pktlist tail */
    uint32_t          len;        /* number of packets in pktlist */
    uint16_t          prio;       /* pktlist prio */
    uint16_t          flowid;     /* flowid */
} wl_pktfwd_pktlist_t;

/** WLAN transmit API pktlist to dispatch lsit to WLAN NIC driver */
extern void   wl_pktfwd_pktlist_xmit(struct net_device * net_device,
                 wl_pktfwd_pktlist_t * wl_pktfwd_pktlist);

/** WLAN Transmit (downstream) packet forwarding */
extern void   wl_pktfwd_dnstream(struct wl_info *wl);

extern void   wl_pktfwd_dnqueued(struct wl_info *wl, d3fwd_wlif_t * d3fwd_wlif,
                 uint32_t * ucast_pkts);

#if defined(WL_PKTFWD_TXEVAL)
extern void   wl_pktfwd_dispatch_pktlist(struct wl_info * wl,
    struct wl_if * wlif, uint8_t * d3addr, uint16_t cfp_flowid, uint16_t prio);
#endif /* WL_PKTFWD_TXEVAL */


/** WLAN Receive (upstream) packet forwarding */
#if defined(BCM_AWL) && defined(WL_AWL_RX)

#define wl_pktfwd_match                wl_awl_upstream_match
#define wl_pktfwd_upstream             wl_awl_upstream_send_chain
#define wl_pktfwd_pktqueue_add_pkt     wl_awl_upstream_add_pkt
#define wl_pktfwd_flush_pktqueues      wl_awl_upstream_send_all

/* TODO: API to get cfp_flowid in AWL platforms */
#define wl_pktfwd_get_cfp_flowid(wl, d3addr)    ID16_INVALID
#else /*  !BCM_AWL || !WL_AWL_RX */

/**
 * Invoked in wlc_rx.c and wlc_amsdu.c: receive is_chainable(PKTC_TBL_FN_CMP)
 * if (d3addr is found and not a loop back) return true, else return false.
 */
extern int    wl_pktfwd_match(uint8_t * d3addr, struct net_device * rx_dev);
extern int    wl_pktfwd_upstream(struct wl_info *wl, struct sk_buff * skb);

#if defined(WLCFP)

extern uint16_t wl_pktfwd_get_cfp_flowid(struct wl_info * wl, uint8_t * d3addr);
#endif /* WLCFP */

/** bins packet to domain specific pktqueue */
extern void   wl_pktfwd_pktqueue_add_pkt(struct wl_info * wl,
                                         struct net_device * rx_net_device,
                                         void * pkt, uint16_t flowid);
/* Callback to flush pktqueues */
extern void   wl_pktfwd_flush_pktqueues(struct wl_info * wl);

#endif /*  !BCM_AWL || !WL_AWL_RX */

#if defined(BCM_PKTFWD_FLCTL)
extern void   wl_pktfwd_update_link_credits(struct wl_info * wl, uint16_t cfp_flowid,
    uint8_t * d3addr, uint32_t prio, int32_t credits, bool add);
#endif /* BCM_PKTFWD_FLCTL */

#endif /* __wl_pktfwd_h_included__ */
