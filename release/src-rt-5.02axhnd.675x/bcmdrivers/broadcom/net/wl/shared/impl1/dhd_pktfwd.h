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

#if !defined(__dhd_pktfwd_h_included__)
#define      __dhd_pktfwd_h_included__

/**
 * =============================================================================
 * WLAN Packet Forwarding using a cache of bridged 802.3 end points.
 * =============================================================================
 */

#include <bcm_pktfwd.h>
#include <dhd.h>

#define DHD_PKTFWD_VERSION           (1)
#define DHD_PKTFWD_RELEASE           (0)
#define DHD_PKTFWD_PATCH             (0)
#define DHD_PKTFWD_VERSIONCODE       PKTFWD_VERSIONCODE(DHD_PKTFWD)

#define DHD_PKTFWD_FAILURE           (-1)
#define DHD_PKTFWD_SUCCESS           (0)

#define DHD_PKTFWD_RADIOS            (PKTFWD_DOMAINS_WLAN)
#define DHD_PKTFWD_DEVICES           (PKTFWD_DEVICES_TOT)

#define DHD_PKTFWD_KEY_INVALID       (PKTFWD_KEY_INVALID)
#define DHD_PKTFWD_KEY_INVALID_UL    ((unsigned long) DHD_PKTFWD_KEY_INVALID)

/* DHD sta idx space [1 .. DHD_MAX_STA] and PKTFWd LUT space [0 .. DHD_MAX_STA) */
#define DHD_STAIDX2LUTID(staidx)     ((staidx) - 1)
#define DHD_FWDID2LUTID(lut_index)   ((lut_index) + 1)


typedef struct dhd_pktfwd_pktlist
{
    struct sk_buff  * head;       /* CFP linked pktlist head */
    struct sk_buff  * tail;       /* CFP linked pktlist tail */
    uint32_t          len;        /* number of packets in pktlist */
    uint16_t          prio;       /* pktlist prio */
    uint16_t          flowid;     /* flowid */
} dhd_pktfwd_pktlist_t;

typedef struct dhd_pktfwd_stats          /* Global System Statistics */
{
    uint32_t        txf_chn_pkts;       /* CHN (chaining) fast path */
    uint32_t        pkts_dropped;       /* Total packets dropped */
    uint32_t        rx_fast_fwds;       /* LUT HIT, fast path forwarding */
    uint32_t        rx_slow_fwds;       /* LUT Miss, slow path forwarding */
    uint32_t        tot_stations;       /* Total associated STAs */
    uint32_t        ops_failures;       /* Total request failures */
} dhd_pktfwd_stats_t;

typedef enum dhd_pktfwd_req
{                                       /* external usage */
    dhd_pktfwd_req_undefined_e     = 0,  /* -------------- */
    dhd_pktfwd_req_set_txmode_e    = 1,  /*         wlc    */
    dhd_pktfwd_req_get_txmode_e    = 2,  /* bridge, wlc    */
    dhd_pktfwd_req_ins_symbol_e    = 3,  /* bridge         */
    dhd_pktfwd_req_flush_full_e    = 4,  /*         wl     */
    dhd_pktfwd_req_assoc_sta_e     = 5,  /*         wlc    */
    dhd_pktfwd_req_pktlist_e       = 6,  /*         wls    */
    dhd_pktfwd_req_max_e           = 7
} dhd_pktfwd_req_t;


/** PKTFWD 802.3 Address: LAN endpoint or WLAN Station insert/delete/hit/clr */
extern void * dhd_pktfwd_lut_ins(uint8_t * d3addr,
                                struct net_device * dev, bool is_wlan);
extern void   dhd_pktfwd_lut_del(uint8_t * d3addr,
                                struct net_device * net_device);
extern int    dhd_pktfwd_lut_hit(uint8_t * d3addr,
                                struct net_device * net_device);
extern void   dhd_pktfwd_lut_clr(struct net_device * dev); /* Flush full LUT */

/** PKTFWD WLAN Radio insert/delete/debug */
extern void * dhd_pktfwd_wlif_ins(void* wlif, void *osh,
				struct net_device *dev, unsigned int unit);
extern void   dhd_pktfwd_wlif_del(d3fwd_wlif_t * d3fwd_wlif);

/** PKTFWD WLAN Radio insert/delete/debug */
extern void * dhd_pktfwd_radio_ins(dhd_pub_t * dhdp, unsigned int unit,
	struct net_device * dev);
extern void   dhd_pktfwd_radio_del(unsigned int unit);

#if defined(BCM_WFD)
extern void   dhd_pktfwd_wfd_ins(struct net_device *dev,
	unsigned int wfd_idx, unsigned int unit);
extern void   dhd_pktfwd_wfd_del(unsigned int unit);
#endif /* BCM_WFD */

/** PKTFWD global subsystem construction/destruction/debug_dump : LOCKLESS */
extern int    dhd_pktfwd_sys_init(void);
extern void   dhd_pktfwd_sys_fini(void);
extern void   dhd_pktfwd_sys_dump(dhd_pub_t *dhdp, struct bcmstrbuf *b);
extern void   dhd_pktfwd_radio_dump(dhd_pub_t *dhdp, struct bcmstrbuf * b);
extern void   dhd_pktfwd_sys_clr(struct wl_info * wl);

/** Callback registered with WFD, to dispatch pktlists to d3fwd_wlif */
extern void dhd_pktfwd_xfer_callback(pktlist_context_t * pktlist_context);

extern unsigned long dhd_pktfwd_request(int request,
    unsigned long param0, unsigned long param1, unsigned long param2);

extern int    dhd_pktfwd_upstream(dhd_pub_t *dhdp, pNBuff_t pNBuff);

/** bins packet to domain specific pktqueue */
extern int    dhd_pktfwd_pktqueue_add_pkt(dhd_pub_t * dhd_pub,
    struct net_device * net_device, void * pkt);

/* Callback to flush pktqueues */
extern void   dhd_pktfwd_flush_pktqueues(dhd_pub_t * dhd_pub);


/* Callbacks to populate DHD domain D3LUT pool */
extern unsigned long (* dhd_pktc_req_hook)(int req_id, unsigned long param0,
    unsigned long param1, unsigned long param2);
extern void   (*dhd_pktc_del_hook)(unsigned long addr,
                                   struct net_device * net_device);
extern int    (*fdb_check_expired_dhd_hook)(unsigned char *addr,
                                            struct net_device * net_device);

/* PKTFWD KEYMAP: SET/RESET pktfwd_key to flowid map */
extern void   dhd_pktfwd_reset_keymap(uint32_t radio_idx, uint16_t dest,
    uint16_t flowid, uint16_t prio);
extern void   dhd_pktfwd_set_keymap(uint32_t radio_idx, uint16_t pktfwd_key,
    uint16_t flowid, uint16_t prio);

#endif /* __dhd_pktfwd_h_included__ */
