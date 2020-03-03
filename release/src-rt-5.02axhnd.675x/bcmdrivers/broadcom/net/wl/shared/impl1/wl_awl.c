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

/**
 * =============================================================================
 *
 * WLAN Packet Forwarding Datapath to Archer WLAN
 *
 * Replace WFD and use Archer WLAN Interface for pktlist and pkt forwarding
 * Replace PKTFWD and use Archer WLAN for upstream processing
 *
 * TODO
 *  - Statistics update (for fast path) ?
 *  - rxbound for slow path packets
 *
 * =============================================================================
 */

/**
 * =============================================================================
 * wl_worker_thread_func()
 *  wl_dpc_rxwork()
 *   wl_dpc()
 *    wlc_bmac_recv()
 *     ===== (CFP) =====
 *     wlc_cfp_bmac_recv()
 *      wlc_cfp_scb_chain_sendup()
 *       wl_cfp_sendup()
 *        *wl_awl_upstream_add_pkt()        => Add packet to lock-less SLL
 *        * ......                           .........
 *        *wl_awl_upstream_add_pkt()        => Add packet to lock-less SLL
 *      *wl_awl_upstream_send_all()            send to Archer (archer_wlan_rx_send)
 *      ....
 *     ===== (NON-CFP) =====
 *     wlc_recv()
 *      wlc_recvdata()
 *       wlc_ampdu_recvdata()
 *        wlc_recvdata_ordered()
 *         wl_sendup()
 *          *wl_awl_upstream_send_chained()     => unchain, add to SLL, send
 *       wlc_sendup_chain()
 *        wl_sendup()
 *         *wl_awl_upstream_send_chained()     => unchain, add to SLL, send
 *     ....
 *    ....
 *   ....
 *  ....
 *  *wl_awl_process_slowpath_rxpkts()
 *    => unlink packets list from lock based SLL.
 *    wl_awl_rx_sendup()                       => Process Rx packets
 *     wl_handle_blog_sinit()
 *     netif_receive_skb()
 *
 * archer_wlan_rx_send()
 * {
 *   => Add packet list (SLL) to the socket queue
 *   => notify socket
 * }
 *
 *
 * Archer CPU context
 * ==================
 * archer_wlan_socket_miss_thread()
 * {
 *   *wl_awl_rx_flow_miss_handler_wl_dpc()
 *     => Add packet to the lock based SLL.
 *     wake_up_interruptible()                 => Wake up wl dpc
 * }
 * =============================================================================
 */

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/bcm_skb_defines.h>

#include <d11_cfg.h>
#include <bcm_pktfwd.h>
#include <wl_pktc.h> /* wl_pktc redirected to wl_pktfwd */
#include <wl_pktfwd.h>
#include <wl_dbg.h>
#include <wlc_pub.h>
#include <wlc_dump.h>
#include <wl_linux.h>
#include <802.3.h>
#include <bcmendian.h>
#include <wlc_cfp.h>
#if defined(BCM_BLOG)
#include <wl_blog.h>
#endif /* BCM_BLOG */
#if !defined(WL_EAP_AP)
#if !defined(WLCFP)
#error "AWL requires WLCFP"
#else /* WLCFP */
#if (PKTFWD_ENDPOINTS_MAX != CFP_FLOWID_SCB_TOTAL)
#error "Maximum PKTFWD ENDPOINTS and CFP Flows mismatch"
#endif
#endif /* WLCFP */
#endif /* !WL_EAP_AP */
#include <wlc_bsscfg.h>

#include <bcmendian.h>
#include <pktHdr.h>
#include <bcm_archer.h>

#if defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE)
#include <bcm_spdsvc.h>
#endif /* CONFIG_BCM_SPDSVC || CONFIG_BCM_SPDSVC_MODULE */

#include <wl_awl.h>

/**
 * =============================================================================
 * Section: WL_AWL Local defines and macros
 * =============================================================================
 */

/**
 * Pre-processor flags, (few in wl_awl.h)
 */
/*
 * Enable this flag for lite rx processing (default disabled)
 * - ETHER_TYPE_BRCM/BRCM_AIRIQ filtered for archer and will go through wl_rx_frame
 * - Archer flow miss will directly call blog_sinit and netif_rx (in archer context)
 */
//#define WL_AWL_RX_LITE

/*
 * Enable this flag for debug message control with-in this file
 * default disabled
 */
//#define WL_AWL_DEBUG

/*
 * Enable this flag if Intrabss frames to be filtered before sending to Archer
 *
 * Note: All packets will go through the D3LUT lookup
 */
#define WL_AWL_FILTER_INTRABSS

/*
 * Enable this flag for checking skb link pointers
 * default disabled
 */
#define WL_AWL_SKB_AUDIT

#define WL_AWL_CB(wl)             (&wl_awl_cb_g[(wl)->unit])
#define WL_AWL_RX_W2A_PKTL(awl)     (&awl->rx.w2a_pktl)
#define WL_AWL_RX_A2W_PKTL(awl)     (&awl->rx.a2w_pktl)

/* Debug macros */
#if defined(WL_AWL_DEBUG)
#define WL_AWL_PTRACE               printk
#define WL_AWL_TRACE                printk
#define WL_AWL_ERROR                printk
#else /* !WL_AWL_DEBUG */
#define WL_AWL_PTRACE               PKTFWD_PTRACE
#define WL_AWL_TRACE                PKTFWD_TRACE
#define WL_AWL_ERROR                PKTFWD_ERROR
#endif /* !WL_AWL_DEBUG */
#define WL_AWL_LOG                  printk

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define WL_AWL_PKTLIST_LOCK(lock)   spin_lock_bh(&(lock))
#define WL_AWL_PKTLIST_UNLK(lock)   spin_unlock_bh(&(lock))
#else
#define WL_AWL_PKTLIST_LOCK(lock)   local_irq_disable()
#define WL_AWL_PKTLIST_UNLK(lock)   local_irq_enable()
#endif  /* ! (CONFIG_SMP || CONFIG_PREEMPT) */

/**
 * Upstream path
 */
/* Processing modes */
#define WL_AWL_RX_MODE_PT           0
#define WL_AWL_RX_MODE_LITE         1
#define WL_AWL_RX_MODE_FULL         2

/* Packet bound */
#define WL_AWL_RX_PKT_BOUND         32   /* 64 */

#if defined(WL_AWL_SKB_AUDIT)
/* Check for non-null skb->prev, log and set to null */
#define WL_AWL_SKB_PREV_AUDIT(skb)    \
	do {                               \
	    if (skb->prev)                 \
	        printk("%s: 0x%p <- 0x%p ->  0x%p\n", __FUNCTION__, skb->prev, skb, skb->next); \
	    skb->prev = NULL;              \
	} while (0)

/* Check for non-null skb->next, log and set to null */
#define WL_AWL_SKB_NEXT_AUDIT(skb)    \
	do {                               \
	    if (skb->next)                 \
	        printk("%s: 0x%p <- 0x%p ->  0x%p\n", __FUNCTION__, skb->prev, skb, skb->next); \
	        skb->next = NULL;          \
	} while (0)
#else /* !WL_AWL_SKB_AUDIT */
#define WL_AWL_SKB_PREV_AUDIT(skb)   do { } while (0)
#define WL_AWL_SKB_NEXT_AUDIT(skb)   do { } while (0)
#endif /* !WL_AWL_SKB_AUDIT */

/**
 * -----------------------------------------------------------------------------
 * struct wl_awl_rx
 *
 * - mode            : pass through (disable), Lite, Full
 * - bound           : Rx packet bound to pass to Archer WLAN
 *
 * - w2a_pktl        : Rx Packet List WLAN -> Archer
 * - a2w_pktl        : Rx Packet List Archer -> WLAN (flow-miss)
 * - a2w_pktl_lock   : Lock for a2w packet list
 *
 * - w2a_rx_packets  : Count of Rx packets received by wl_awl for Archer WLAN
 * - w2a_flt_packets : Count of Rx packets filtered (not sent to Archer)
 * - w2a_fwd_packets : Count of Rx packets forwarded to Archer WLAN
 * - w2a_fwd_calls   : Count of packet forward calls to Archer
 *
 * - w2a_rx_packets  : Count of Rx packets received by wl_awl from Archer (flow-miss)
 * - w2a_flt_packets : Count of Rx packets filtered (not sent to WL)
 * - w2a_fwd_packets : Count of Rx flow-miss packets forwarded to WL
 * - w2a_fwd_calls   : Count of flow-miss packet forward calls to WL
 *
 * -----------------------------------------------------------------------------
 */
typedef struct wl_awl_rx {
	uint8         mode;
	uint8         bound;
	uint8         avgsize;
	pktlist_t     w2a_pktl;
	pktlist_t     a2w_pktl;
	spinlock_t    a2w_pktl_lock;
	unsigned long w2a_rx_packets;
	unsigned long w2a_flt_packets;
	unsigned long w2a_fwd_packets;
	unsigned long w2a_fwd_calls;
	unsigned long w2a_chn_packets;
	unsigned long w2a_max_chn;
	unsigned long a2w_rx_packets;
	unsigned long a2w_flt_packets;
	unsigned long a2w_fwd_packets;
	unsigned long a2w_fwd_calls;
	unsigned long a2w_drp_packets;
} wl_awl_rx_t;

/**
 * -----------------------------------------------------------------------------
 * struct wl_awl_tx
 *
 * - mode            : disable, enable
 * -----------------------------------------------------------------------------
 */
typedef struct wl_awl_tx {
	uint8         mode;
} wl_awl_tx_t;

/**
 * -----------------------------------------------------------------------------
 * struct wl_awl
 *
 * - tx            : down stream path cb
 * - rx            : upstream path cb
 * -----------------------------------------------------------------------------
 */
typedef struct wl_awl {
	struct wl_awl_rx rx;
	struct wl_awl_tx tx;
} wl_awl_t;

/**
 * =============================================================================
 * Section: WL_AWL Global System Object(s)
 * =============================================================================
 */
struct wl_awl wl_awl_cb_g[WL_PKTFWD_RADIOS] = {};

char wl_awl_rx_mode_str_g[][8] = {
	"OFF", "LITE", "YES"
};

/**
 * =============================================================================
 * Section: WL_AWL external function declerations from other files
 * =============================================================================
 */
extern void BCMFASTPATH wlc_recv(wlc_info_t *wlc, void *p);

/**
 * =============================================================================
 * Section: WL_AWL local function declerations
 * =============================================================================
 */
int wl_awl_rx_process_intrabss(struct wl_info *wl, struct sk_buff *skb);

/**
 * =============================================================================
 * Section: WL_AWL Local function definitions
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 * Function : Forward sk_buff (sll) to the Archer Wireless
 *
 * -----------------------------------------------------------------------------
 */
int
wl_awl_upstream(wl_awl_t *awl, pktlist_t *pktlist)
{
	WL_AWL_PTRACE("wl_awl: pktlist 0x%p", pktlist);

	if (likely(pktlist->len != 0)) {
	    /* ingress_port for wlan is 0 */
	    archer_wlan_rx_send(pktlist->head);

	    awl->rx.w2a_chn_packets = pktlist->len;
	    if (awl->rx.w2a_chn_packets > awl->rx.w2a_max_chn)
	        awl->rx.w2a_max_chn = awl->rx.w2a_chn_packets;
	    awl->rx.w2a_fwd_packets += pktlist->len;
	    awl->rx.w2a_fwd_calls++;
	    PKTLIST_RESET(pktlist);
	}

	return BCME_OK;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Add skb to Archer SLL
 *
 * -----------------------------------------------------------------------------
 */
void
wl_awl_pktlist_add(pktlist_t *pktlist, struct sk_buff *skb)
{
	if (likely(pktlist->len != 0)) {
	    /* pend to tail */
	    PKTLIST_PKT_SET_SLL(pktlist->tail, skb, SKBUFF_PTR);
	    pktlist->tail = skb;
	} else {
	    pktlist->head = pktlist->tail = skb;
	}
	++pktlist->len;
}

/**
 * -----------------------------------------------------------------------------
 * Function : free skb's from SLL
 *            unlink and free the packets
 *
 * -----------------------------------------------------------------------------
 */
void
wl_awl_pktlist_free(struct wl_info *wl, pktlist_t *pktlist)
{
	struct sk_buff *skb;
	int pkts;

	pkts = pktlist->len;
	if (pkts != 0) {
	    skb = (struct sk_buff*)pktlist->head;

	    while (pkts--) {
	        struct sk_buff* nskb = skb->prev;
	        skb->next = skb->prev = NULL;

	        /* Free the packet */
	        PKTFREE(wl->pub->osh, skb, FALSE);
	        skb = nskb;
	    }
	    PKTLIST_RESET(pktlist);
	}

	return;
}

/**
 * -----------------------------------------------------------------------------
 * Function : wl_sendup essential processing for flow-miss packets from Archer
 *            Filters packets for spdsvc, blog and sends the packet to network
 *            stack.
 *            <0: Error, not processed
 *             0:  Packet processed, send to stack
 *             1:  Packet filter processed by spdsvc or blog
 * -----------------------------------------------------------------------------
 */
int
wl_awl_rx_sendup(struct wl_info *wl, struct sk_buff *skb)
{

	/* Copied essesntial from wl_sendup() */
#if defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE)
	if (wl_spdsvc_rx(skb) == BCME_OK) {
		return PKT_DONE;
	}
#endif /* CONFIG_BCM_SPDSVC || CONFIG_BCM_SPDSVC_MODULE */

	PKTSETFCDONE(skb);
#ifdef BCM_BLOG
	if (wl_handle_blog_sinit(wl, skb) == PKT_DONE) {
		return PKT_DONE;
	}
#endif /* BCM_BLOG */
	PKTCLRFCDONE(skb);

#if !defined(WL_AWL_FILTER_INTRABSS)
	/* process intra-bss packets as they are not handled in bridge or wl_linux */
	if (wl_awl_rx_process_intrabss(wl, skb) == BCME_OK) {
	    return PKT_DONE;
	}
#endif /* !WL_AWL_FILTER_INTRABSS */

	if (!skb->dev)
		return -1;

	skb->protocol = eth_type_trans(skb, skb->dev);

#if defined(BCM_NBUFF_PKT) && defined(CC_BPM_SKB_POOL_BUILD)
	PKTTAINTED(wl->osh, skb);
#endif /* BCM_NBUFF_PKT && CC_BPM_SKB_POOL_BUILD */

#ifdef NAPI_POLL
	netif_receive_skb(skb);
#else /* NAPI_POLL */
	netif_rx(skb);
#endif /* NAPI_POLL */

	return PKT_NORM;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Callback function from Archer CPU for flow-miss
 *            completely handled with-in Archer context
 *            Might miss some features (dump, wmf, wet, spdsvc .... etc) otherwise
 *            good for initial integration
 * -----------------------------------------------------------------------------
 */
void
wl_awl_rx_flow_miss_handler_archer(void *ctxt, struct sk_buff *skb)
{
	struct wl_info *wl = (struct wl_info*)ctxt;
	wl_awl_t *awl;
	int rc;

	if ((wl == NULL) || (skb == NULL)) {
	    WL_AWL_ERROR("%s (0x%p, 0x%p) with NULL parameters\n",
	        __FUNCTION__, wl, skb);
	    return;
	}

	awl = WL_AWL_CB(wl);
	awl->rx.a2w_rx_packets++;

	/* Process the flow-miss packets */
	rc = wl_awl_rx_sendup(wl, skb);

	if (rc == 0) {
	    awl->rx.a2w_fwd_packets++;
	    awl->rx.a2w_fwd_calls++;
	} else {
		awl->rx.a2w_flt_packets++;
	}

	return;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Callback function from Archer CPU for flow-miss
 *
 *   adds packets to SLL and invokes WL dpc
 *   minimum processing in Archer context
 * -----------------------------------------------------------------------------
 */
void
wl_awl_rx_flow_miss_handler_wl_dpc(void *ctxt, struct sk_buff *skb)
{
	struct wl_info *wl = (struct wl_info*)ctxt;
	wl_awl_t *awl;
	pktlist_t *pktlist;

	/* Get the WL public control block */
	if ((wl == NULL) || (skb == NULL)) {
	    WL_AWL_ERROR("%s (0x%p, 0x%p) with NULL parameters\n",
	        __FUNCTION__, wl, skb);
	    return;
	}

	/* Add to the slow path SLL with a lock, This is happening from Archer context */
	awl = WL_AWL_CB(wl);
	pktlist = WL_AWL_RX_A2W_PKTL(awl);

	WL_AWL_PKTLIST_LOCK(awl->rx.a2w_pktl_lock);

	/* Add to the SLL */
	wl_awl_pktlist_add(pktlist, skb);
	awl->rx.a2w_rx_packets++;

	WL_AWL_PKTLIST_UNLK(awl->rx.a2w_pktl_lock);

	/* Wakeup DPC thread or run DPC tasklet */
	atomic_inc(&wl->callbacks);
#ifdef WL_ALL_PASSIVE
	wl->awl_sp_rxq_dispatched = true;
	wake_up_interruptible(&wl->kthread_wqh);
#endif /* WL_ALL_PASSIVE */

	return;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Process Intra BSS packets as they are not handled in the bridge
 *            and wl_linux
 *
 *   Check if the packet is destined to the same domain using D3LUT
 *   if intrabss, find the station and tx the packet back to NIC driver
 * -----------------------------------------------------------------------------
 */
int
wl_awl_rx_process_intrabss(struct wl_info *wl, struct sk_buff *skb)
{
	d3lut_t       * d3lut = wl_pktfwd_lut();
	uint8_t       * d3_addr;
	d3lut_elem_t  * d3lut_elem;
	bool          intrabss = FALSE;
	wl_if_t       * wlif;

	d3_addr = (uint8_t *) PKTDATA(wl->osh, skb);
	wlif = WL_DEV_IF(skb->dev);

	if (ETHER_ISMULTI(d3_addr)) /* Only unicast packets are bridged via LUT */ {
	    return BCME_UNSUPPORTED;
	}

	d3lut_elem = d3lut_lkup(d3lut, d3_addr, D3LUT_LKUP_GLOBAL_POOL);

	if (likely(d3lut_elem != D3LUT_ELEM_NULL)) {
	    if ((d3lut_elem->key.domain == wl->unit) && (d3lut_elem->ext.ssid == wlif->subunit)) {
	        intrabss = TRUE;
	    }
	}

	if (intrabss == TRUE) {
	    wlc_bsscfg_t * bsscfg = wlc_bsscfg_find_by_wlcif(wl->wlc, wlif->wlcif);

	    /* Clear pkttag information saved in recv path */
	    WLPKTTAGCLEAR(skb);
/* To be enabled if next flow to be done through FC instead of directly */
//#if defined(BCM_BLOG) && !defined(WL_AWL_FILTER_INTRABSS)
//	    wl_handle_blog_emit(wl, wlif, skb, net);
//#endif /* BCM_BLOG && !WL_AWL_FILTER_INTRABSS */

	    wlc_sendpkt(wl->wlc, skb, bsscfg->wlcif);
	    return BCME_OK;
	}

	return BCME_ERROR;
}

/**
 * =============================================================================
 * Section: WL_AWL External function definitions
 * =============================================================================
 */
/**
 * -----------------------------------------------------------------------------
 * WL_AWL: RX path External Interface functions
 * -----------------------------------------------------------------------------
 */

/**
 * -----------------------------------------------------------------------------
 * Function : Function to process slow path upstream packets from Archer flow-miss
 *
 *   Called from bus layer in the context of WL DPC
 *   No perim locks in this function as caller already take care of locking
 *   Retrieve packets from SLL and call wl_linux rx packet handler
 * -----------------------------------------------------------------------------
 */
bool
wl_awl_process_slowpath_rxpkts(struct wl_info *wl)
{
	bool more = FALSE;
	wl_awl_t *awl;
	pktlist_t *pktlist;
	int npkts, ppkts = 0;
	struct sk_buff *skb = NULL;
	wl_if_t *if_list;
	bool dev_matched;

	awl = WL_AWL_CB(wl);

	/* Nothing to do if mode is not WL DPC */
	if (awl->rx.mode != WL_AWL_RX_MODE_FULL)
	    goto done;

	/* Get the pending packets list */
	pktlist = WL_AWL_RX_A2W_PKTL(awl);

	WL_AWL_PKTLIST_LOCK(awl->rx.a2w_pktl_lock);

	ppkts = npkts = pktlist->len;
	if (npkts == 0) {
	    WL_AWL_PKTLIST_UNLK(awl->rx.a2w_pktl_lock);
	    goto done;
	}

	skb = (struct sk_buff*)pktlist->head;
	PKTLIST_RESET(pktlist);

	WL_AWL_PKTLIST_UNLK(awl->rx.a2w_pktl_lock);

	/* Let wl process the packets */
	WL_LOCK(wl);
	while (npkts--) {
	    struct sk_buff* nskb = skb->prev;
	    skb->next = skb->prev = NULL;
	    /* Audit the wlif */
	    if_list = wl->if_list;
	    dev_matched = FALSE;
	    while (if_list) {
	        if (skb->dev == if_list->dev) {
	            dev_matched = TRUE;
	            break;
	        }
	        if_list = if_list->next;
	    }
	    if (!dev_matched) {
	        WL_AWL_ERROR("%s: wl%d: drop packets skb->dev %s\n", __FUNCTION__,
	            wl->unit, skb->dev ? skb->dev->name : "NULL");
	        PKTFREE(wl->pub->osh, skb, FALSE);
	        awl->rx.a2w_drp_packets++;
	    }
	    else {
	        /* Process the flow-miss packets */
	        wl_awl_rx_sendup(wl, skb);
	        awl->rx.a2w_fwd_packets++;
	    }
	    awl->rx.a2w_fwd_calls++;
	    skb = nskb;
	}
	WL_UNLOCK(wl);

done:
	if (!more) {
#ifdef WL_ALL_PASSIVE
	    wl->awl_sp_rxq_dispatched = FALSE;
#endif /* WL_ALL_PASSIVE */
	    atomic_sub(ppkts, &wl->callbacks);
	}

	return more;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Function to send Rx SLL to Archer
 *
 *  Called from from protocol layer for commiting the packets to archer
 * -----------------------------------------------------------------------------
 */
void
wl_awl_upstream_send_all(struct wl_info *wl)
{
	wl_awl_t *awl = WL_AWL_CB(wl);
	pktlist_t *pktlist = WL_AWL_RX_W2A_PKTL(awl);

	WL_AWL_PTRACE("wl%d_awl: pktlist 0x%p", wl->unit, pktlist);

	wl_awl_upstream(awl, pktlist);

	return;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Function to add Rx packet to wl_awl SLL
 *
 *  Called from from protocol layer for single skb.
 *  Caller must call wl_awl_upstream_send() to finally push the list Archer
 * -----------------------------------------------------------------------------
 */
void
wl_awl_upstream_add_pkt(struct wl_info * wl, struct net_device * net_device,
                           void * pkt, uint16_t flowid)
{
	wl_awl_t *awl = WL_AWL_CB(wl);
	pktlist_t *pktlist = WL_AWL_RX_W2A_PKTL(awl);
	struct sk_buff *skb;
	wl_if_t * wlif;

	WL_AWL_PTRACE("wl%d pkt %p", wl->unit, pkt);

	awl->rx.w2a_rx_packets++;

	skb = (struct sk_buff *)pkt;
	skb_cb_zero(skb); /* explicitly zero cb[], wl_cb[] */
	skb->dev = net_device;

	if (skb->prev) {
	    printk("0x%p <- 0x%p ->  0x%p\n", skb->prev, skb, skb->next);
	    skb->prev = NULL;
	}

#if defined(WL_AWL_FILTER_INTRABSS)
	/* Filter out intra BSS packets and forward back to NIC without sending to Archer */
	if (wl_awl_rx_process_intrabss(wl, skb) == BCME_OK) {
	    awl->rx.w2a_flt_packets++;
	    return;
	}
#endif /* WL_AWL_FILTER_INTRABSS */

	wlif = WL_DEV_IF(net_device);
	ARCHER_WLAN_RADIO_IDX(skb) = wl->unit;
	ARCHER_WLAN_INTF_IDX(skb) = wlif->subunit;
	wl_awl_pktlist_add(pktlist, skb);

	/* Check packet bounds and send to Archer */
	if (pktlist->len >= awl->rx.bound) {
	    wl_awl_upstream(awl, pktlist);
	}

	return;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Function to add and send chained Rx packets to Archer
 *
 *  Convert chained Rx packet to SLL
 *  Send SLL to Archer
 *
 * -----------------------------------------------------------------------------
 */
int
wl_awl_upstream_send_chain(struct wl_info * wl, struct sk_buff * skb)
{
	wl_awl_t *awl = WL_AWL_CB(wl);
	pktlist_t *pktlist = WL_AWL_RX_W2A_PKTL(awl);
	wl_if_t * wlif;
	struct net_device * net = skb->dev;
	void *nskb = NULL;

	WL_AWL_PTRACE("wl%d skb 0x%p 0x%p %d", wl->unit, skb, skb->data, skb->len);

	/* Go through all the packets in the chain */
	FOREACH_CHAINED_PKT(skb, nskb) {
	    /* Remove Chaining */
	    if (PKTISCHAINED(skb)) {
	        PKTCLRCHAINED(wl->osh, skb);
	        PKTCCLRFLAGS(skb);
	    }
	    skb->dev = net;

	    /* Initialize required skb fields */
	    wlif = WL_DEV_IF(skb->dev);
	    skb_cb_zero(skb); /* explicitly zero cb[], wl_cb[] */

	    /* Add to the archer SLL list */
	    ARCHER_WLAN_RADIO_IDX(skb) = wl->unit;
	    ARCHER_WLAN_INTF_IDX(skb) = wlif->subunit;
	    wl_awl_pktlist_add(pktlist, skb);
	    awl->rx.w2a_rx_packets++;
	}

	/* Send all packets to Archer */
	wl_awl_upstream(awl, pktlist);

	return BCME_OK;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Check if packet can be chained for the given net device
 *            For archer all packets are chain forwarded
 * -----------------------------------------------------------------------------
 */
int BCMFASTPATH
wl_awl_upstream_match(uint8_t * d3addr, struct net_device * rx_net_device)
{
	/* Always return true, as all packets are sent to Archer first */
	/* TODO: What about same radio ? */
	return true;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Helper debug dump the AWL rx statistics
 * -----------------------------------------------------------------------------
 */
void
wl_awl_dump(struct wl_info *wl, struct bcmstrbuf *b)
{
	wl_awl_t *awl = WL_AWL_CB(wl);

	WL_AWL_PTRACE("wl%d_awl:\n", wl->unit);

	/* Dump Rx statistics */
	bcm_bprintf(b, " WL_AWL Rx: %s bound %d",
	    wl_awl_rx_mode_str_g[awl->rx.mode], awl->rx.bound);
	bcm_bprintf(b, " W2A List [rxd %lu flt %lu fwd %lu cls %lu chn %lu/%lu]",
	    awl->rx.w2a_rx_packets, awl->rx.w2a_flt_packets,
	    awl->rx.w2a_fwd_packets, awl->rx.w2a_fwd_calls,
	    awl->rx.w2a_chn_packets, awl->rx.w2a_max_chn);
	bcm_bprintf(b, " A2W List [rxd %lu flt %lu fwd %lu cls %lu drp %lu ]\n",
	    awl->rx.a2w_rx_packets, awl->rx.a2w_flt_packets,
	    awl->rx.a2w_fwd_packets, awl->rx.a2w_fwd_calls,
	    awl->rx.a2w_drp_packets);
}

/**
 * -----------------------------------------------------------------------------
 * Function : Helper to reset statistics collected
 * -----------------------------------------------------------------------------
 */
void
wl_awl_clr_stats(struct wl_info * wl)
{
	wl_awl_t *awl = WL_AWL_CB(wl);

	WL_AWL_PTRACE("wl%d_awl:\n", wl->unit);

	awl->rx.w2a_rx_packets = 0;
	awl->rx.w2a_flt_packets = 0;
	awl->rx.w2a_fwd_packets = 0;
	awl->rx.w2a_fwd_calls = 0;
	awl->rx.w2a_chn_packets = 0;
	awl->rx.w2a_max_chn = 0;
	awl->rx.a2w_rx_packets = 0;
	awl->rx.a2w_flt_packets = 0;
	awl->rx.a2w_fwd_packets = 0;
	awl->rx.a2w_fwd_calls = 0;
	awl->rx.a2w_drp_packets = 0;

	return;
}

/**
 * -----------------------------------------------------------------------------
 * Function : WL AWL initialization function
 *            Initialize AWL RX control block, register with Archer
 * -----------------------------------------------------------------------------
 */
void*
wl_awl_attach(struct wl_info *wl, uint unit)
{
	wl_awl_t *awl = WL_AWL_CB(wl);
	archer_wlan_rx_miss_handler_t a2w_flow_miss_handler = wl_awl_rx_flow_miss_handler_wl_dpc;

	WL_AWL_PTRACE("wl%d_awl: wl 0x%p", unit, wl);


	memset(awl, 0, sizeof(wl_awl_t));
	awl->tx.mode = 1;
#if defined(WL_AWL_RX)
#if defined(WL_AWL_RX_LITE)
	awl->rx.mode = WL_AWL_RX_MODE_LITE;
	a2w_flow_miss_handler = wl_awl_rx_flow_miss_handler_archer;
#else /* WL_AWL_RX_LITE */
	awl->rx.mode = WL_AWL_RX_MODE_FULL;
#endif /* WL_AWL_RX_LITE */
#else /* !WL_AWL_RX */
	awl->rx.mode = WL_AWL_RX_MODE_PT;
#endif /* !WL_AWL_RX */

	awl->rx.bound = WL_AWL_RX_PKT_BOUND;

	awl->rx.w2a_pktl.head = PKTLIST_PKT_NULL;
	awl->rx.w2a_pktl.tail = PKTLIST_PKT_NULL;
	awl->rx.w2a_pktl.len = 0;

	awl->rx.a2w_pktl.head = PKTLIST_PKT_NULL;
	awl->rx.a2w_pktl.tail = PKTLIST_PKT_NULL;
	awl->rx.a2w_pktl.len = 0;

	spin_lock_init(&awl->rx.a2w_pktl_lock);

	/* Register with Archer call back functions for Rx */
	if (archer_wlan_rx_register(unit, a2w_flow_miss_handler, wl) != 0) {
	    WL_AWL_ERROR("archer_wlan_rx_register failed\n");
	    return NULL;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* Register dump hook with WLAN "awl" module for iovar queries */
	wlc_dump_add_fns(wl->pub, "awl", (dump_fn_t)wl_awl_dump,
	    (clr_fn_t)wl_awl_clr_stats, (void *) wl);
#endif /* BCMDBG || BCMDBG_DUMP */

#ifdef WL_ALL_PASSIVE
	wl->awl_sp_rxq_dispatched = FALSE;
#endif /* WL_ALL_PASSIVE */

	WL_AWL_LOG("wl%d_awl attach successful tx: %d, rx: %d\n",
	    unit, awl->tx.mode, awl->rx.mode);

	return (void*)awl;
}

/**
 * -----------------------------------------------------------------------------
 * Function : WL AWL de-initialization function
 *            de-register with archer, free pending packets in the SLL's
 * -----------------------------------------------------------------------------
 */
void
wl_awl_detach(struct wl_info *wl, void *ctxt)
{
	wl_awl_t *awl = WL_AWL_CB(wl);

	WL_AWL_PTRACE("wl%d_awl: wl 0x%p, awl 0x%p", wl->unit, wl, awl);

	/* De-register with Archer call back functions for Rx */
	archer_wlan_rx_register(wl->unit, NULL, NULL);

	/* Free A2W packet SLL under lock */
	WL_AWL_PKTLIST_LOCK(awl->rx.a2w_pktl_lock);
	wl_awl_pktlist_free(wl, WL_AWL_RX_A2W_PKTL(awl));
	WL_AWL_PKTLIST_UNLK(awl->rx.a2w_pktl_lock);

	/* Free Lock-less W2A packet SLL */
	wl_awl_pktlist_free(wl, WL_AWL_RX_W2A_PKTL(awl));

	WL_AWL_LOG("wl%d_awl detach complete", wl->unit);

	return;
}
