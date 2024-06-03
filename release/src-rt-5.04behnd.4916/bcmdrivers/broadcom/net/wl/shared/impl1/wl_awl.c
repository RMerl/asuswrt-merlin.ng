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
 *          *wl_awl_upstream_send_chain()     => unchain, add to SLL, send
 *       wlc_sendup_chain()
 *        wl_sendup()
 *         *wl_awl_upstream_send_chain()     => unchain, add to SLL, send
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
#include <wl_dbg.h>
#include <wlc_pub.h>
#include <wlc_dump.h>
#include <wl_linux.h>
#include <802.3.h>
#include <bcmendian.h>
#include <wlc_cfp.h>
#if defined(BCM_BLOG)
#include <fcache.h>
#endif /* BCM_BLOG */
#include <wl_blog.h>
#if !defined(WLCFP)
#error "AWL requires WLCFP"
#endif /* WLCFP */
#if (PKTFWD_ENDPOINTS_MAX < SCB_USER_FLOWID_TOTAL)
#error "Maximum PKTFWD ENDPOINTS and SCB Flows mismatch"
#endif
#include <wlc_bsscfg.h>
#include <bcmendian.h>
#include <pktHdr.h>
#include <bcm_archer.h>
#include <wl_pktc.h>		/* wl_pktc redirected to wl_pktfwd */
#include <wlan_shared_defs.h>

#if defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE) || defined(WLBIN_COMPAT)
#include <bcm_spdsvc.h>
#endif /* CONFIG_BCM_SPDSVC || CONFIG_BCM_SPDSVC_MODULE || WLBIN_COMPAT */

#if defined(BCM_WLAN_PER_CLIENT_FLOW_LEARNING) && defined(WL_PKTFWD_INTRABSS)
#include <wlc.h>
#include <wlc_rx.h>
#endif

#include <wl_awl.h>
#include <wlc_scb.h>

/**
 * =============================================================================
 * Section: WL_AWL Local defines and macros
 * =============================================================================
 */

/**
 * Pre-processor flags, (few in wl_awl.h)
 */
/*
 * Enable this flag for debug message control with-in this file
 * default disabled
 */
//#define WL_AWL_DEBUG

#ifndef WLC_DUMP_INST
#define WLC_DUMP_INST(_wl)	((_wl)->pub)
#endif

/*
 * Enable this flag for checking skb link pointers
 * default disabled
 */
#define WL_AWL_SKB_AUDIT

#define WL_AWL_SYS_CB()             (&wl_awl_sys_g)
#define WL_AWL_CB(wl)               \
	(((wl)->unit < WL_AWL_MAX_RADIOS) ? (&wl_awl_cb_g[(wl)->unit]) : NULL)
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
#define WL_AWL_PRINTF(b, fmt, args...)                                  \
	if (procfs == true) seq_printf((struct seq_file *)(b), fmt, ##args); \
	else bcm_bprintf((struct bcmstrbuf *)(b), fmt, ##args)

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define WL_AWL_PKTLIST_LOCK(lock)   spin_lock_bh(&(lock))
#define WL_AWL_PKTLIST_UNLK(lock)   spin_unlock_bh(&(lock))
#else
#define WL_AWL_PKTLIST_LOCK(lock)   local_irq_disable()
#define WL_AWL_PKTLIST_UNLK(lock)   local_irq_enable()
#endif /* ! (CONFIG_SMP || CONFIG_PREEMPT) */

/* Max supported Radios */
#define WL_AWL_MAX_RADIOS           WLAN_RADIOS_MAX

/**
 * Upstream path
 */
/* Processing modes */
#define WL_AWL_RX_MODE_PT           0
#define WL_AWL_RX_MODE_LITE         1
#define WL_AWL_RX_MODE_FULL         2

/* Packet bound */
#if defined(ARCHER_WLAN_RX_BUDGET)
#define WL_AWL_RX_PKT_BOUND         ARCHER_WLAN_RX_BUDGET
#else /* !ARCHER_WLAN_RX_BUDGET */
#define WL_AWL_RX_PKT_BOUND         32	/* 64 */
#endif /* !ARCHER_WLAN_RX_BUDGET */

#if defined(WL_AWL_SKB_AUDIT)
/* Check for non-null skb->prev, log and set to null */
#define WL_AWL_SKB_PREV_AUDIT(skb)    \
	do {                               \
	    if (skb->prev)                 \
	        printk("%s: 0x%px <- 0x%px ->  0x%px\n", __FUNCTION__, skb->prev, skb, skb->next); \
	    skb->prev = NULL;              \
	} while (0)

/* Check for non-null skb->next, log and set to null */
#define WL_AWL_SKB_NEXT_AUDIT(skb)    \
	do {                               \
	    if (skb->next)                 \
	        printk("%s: 0x%px <- 0x%px ->  0x%px\n", __FUNCTION__, skb->prev, skb, skb->next); \
	        skb->next = NULL;          \
	} while (0)
#else /* !WL_AWL_SKB_AUDIT */
#define WL_AWL_SKB_PREV_AUDIT(skb)   do { } while (0)
#define WL_AWL_SKB_NEXT_AUDIT(skb)   do { } while (0)
#endif /* !WL_AWL_SKB_AUDIT */

#if defined(ARCHER_WLAN_MISS_PKTLIST)
#define wl_awl_rx_flow_miss_handler_wl_dpc wl_awl_rx_flow_miss_handler_wl_dpc_sll
#define wl_awl_rx_flow_miss_handler_archer wl_awl_rx_flow_miss_handler_archer_sll
#else /* !ARCHER_WLAN_MISS_PKTLIST */
#define wl_awl_rx_flow_miss_handler_wl_dpc wl_awl_rx_flow_miss_handler_wl_dpc_skb
#define wl_awl_rx_flow_miss_handler_archer wl_awl_rx_flow_miss_handler_archer_skb
#endif /* !ARCHER_WLAN_MISS_PKTLIST */

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
	uint8 mode;
	uint8 bound;
	uint8 avgsize;
	pktlist_t w2a_pktl;
	pktlist_t a2w_pktl;
	spinlock_t a2w_pktl_lock;
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
	unsigned long a2w_chn_packets;
	unsigned long a2w_max_chn;
} wl_awl_rx_t;

/**
 * -----------------------------------------------------------------------------
 * struct wl_awl_tx
 *
 * - mode            : disable, enable
 * -----------------------------------------------------------------------------
 */
typedef struct wl_awl_tx {
	uint8 mode;
} wl_awl_tx_t;

/**
 * -----------------------------------------------------------------------------
 * struct wl_awl
 *
 * - tx            : down stream path cb
 * - rx            : upstream path cb
 * - ifs           : Interface device array
 * -----------------------------------------------------------------------------
 */
typedef struct wl_awl {
	struct wl_awl_rx rx;
	struct wl_awl_tx tx;
	struct net_device *ifs[WL_MAX_IFS];
} wl_awl_t;

/**
 * -----------------------------------------------------------------------------
 * struct wl_awl_sys
 *
 * - proc_dir      : pointer to awl proc file system directory
 * - proc_stats    : pointer to awl stats proc file
 * - radio         : array of awl radio control block pointers
 * -----------------------------------------------------------------------------
 */
typedef struct wl_awl_sys {
	struct proc_dir_entry *proc_dir;
	struct proc_dir_entry *proc_stats;
	struct wl_awl *radio[WL_AWL_MAX_RADIOS];
} wl_awl_sys_t;

/**
 * =============================================================================
 * Section: WL_AWL Global System Object(s)
 * =============================================================================
 */
struct wl_awl_sys wl_awl_sys_g = {0};
struct wl_awl wl_awl_cb_g[WL_AWL_MAX_RADIOS] = { };

char wl_awl_rx_mode_str_g[][8] = {
	"OFF", "LITE", "ON "
};

/**
 * =============================================================================
 * Section: WL_AWL external function declerations from other files
 * =============================================================================
 */
extern void BCMFASTPATH wlc_recv(wlc_info_t * wlc, void *p);
extern void BCMFASTPATH wl_sendup(wl_info_t * wl, wl_if_t * wlif, void *p, int numpkt);

/**
 * =============================================================================
 * Section: WL_AWL local function declerations
 * =============================================================================
 */

static inline int wl_awl_rx_process_intrabss(struct wl_info *wl, struct sk_buff *skb);

/**
 * =============================================================================
 * Section: WL_AWL Local function definitions
 * =============================================================================
 */
static inline struct net_device *wl_awl_skb_intf_dev(wl_awl_t *awl,
	struct sk_buff *skb)
{
	uint8 ifidx = ARCHER_WLAN_INTF_IDX(skb);

	return (ifidx < WL_MAX_IFS) ? awl->ifs[ifidx] : NULL;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Dump AWL stats
 *
 * -----------------------------------------------------------------------------
 */
void
wl_awl_dump_radio(wl_awl_t *awl, void *b, bool procfs)
{
	WL_AWL_PRINTF(b, " WL_AWL:\n");

	/* Dump Rx statistics */
	WL_AWL_PRINTF(b, "  Rx: Mode %s bound %d\n",
	    wl_awl_rx_mode_str_g[awl->rx.mode], awl->rx.bound);
	WL_AWL_PRINTF(b, "      W2A List [rxd %lu flt %lu fwd %lu cls %lu "
	    "chn %lu/%lu]",
	    awl->rx.w2a_rx_packets, awl->rx.w2a_flt_packets,
	    awl->rx.w2a_fwd_packets, awl->rx.w2a_fwd_calls,
	    awl->rx.w2a_chn_packets, awl->rx.w2a_max_chn);
	WL_AWL_PRINTF(b, "      A2W List [rxd %lu drp %lu fwd %lu cls %lu "
	    "chn %lu/%lu]\n",
	    awl->rx.a2w_rx_packets, awl->rx.a2w_flt_packets,
	    awl->rx.a2w_fwd_packets, awl->rx.a2w_fwd_calls,
	    awl->rx.a2w_chn_packets, awl->rx.a2w_max_chn);
	WL_AWL_PRINTF(b, "\n");
}

/**
 * -----------------------------------------------------------------------------
 * Function : proc file stats read procedure
 *
 * -----------------------------------------------------------------------------
 */
int
wl_awl_stats_file_read_proc(struct seq_file *m, void *v)
{
	struct wl_awl_sys *awl_sys = WL_AWL_SYS_CB();
	uint unit;
	bool procfs = true;

	for (unit = 0; unit < WL_AWL_MAX_RADIOS; unit++) {
	    wl_awl_t *awl = awl_sys->radio[unit];
	    if (awl != NULL) {
	        WL_AWL_PRINTF(m, "Radio[%d]\n==========\n", unit);
	        wl_awl_dump_radio(awl, m, procfs);
	    }
	}

	return 0;
}

/**
 * -----------------------------------------------------------------------------
 * Function : proc file system teardown
 *
 * -----------------------------------------------------------------------------
 */
void
wl_awl_proc_deinit(wl_awl_t *awl, uint unit)
{
	struct wl_awl_sys *awl_sys = WL_AWL_SYS_CB();
	bool deinit = true;

	awl_sys->radio[unit] = NULL;

	/* Check if all intrfaces are de-initialized */
	for (unit = 0; unit < WL_AWL_MAX_RADIOS; unit++) {
	    if (awl_sys->radio[unit]) {
	        deinit = false;
	        break;
	    }
	}

	if (deinit == true) {
	    /* Remove proc file system interface */
	    if (awl_sys->proc_stats != NULL) {
	        remove_proc_entry("stats", awl_sys->proc_dir);
	        awl_sys->proc_stats = NULL;
	    }

	    if (awl_sys->proc_dir != NULL) {
	        remove_proc_entry("wl_awl", NULL);
	        awl_sys->proc_dir = NULL;
	    }
	}

	return;
}

/**
 * -----------------------------------------------------------------------------
 * Function : proc file system initialization
 *
 * -----------------------------------------------------------------------------
 */
void
wl_awl_proc_init(wl_awl_t *awl, uint unit)
{
	struct wl_awl_sys *awl_sys = WL_AWL_SYS_CB();

	awl_sys->radio[unit] = awl;

	if (awl_sys->proc_dir) {
	    /* proc file system already initialized */
	    return;
	}

	/* Create proc directory */
	awl_sys->proc_dir = proc_mkdir("wl_awl", NULL);
	if (awl_sys->proc_dir == NULL) {
	    WL_AWL_ERROR("%s failed to create [wl_awl] proc directory\n",
	        __FUNCTION__);
	    return;
	}

	/* Create stats file */
	awl_sys->proc_stats = proc_create_single("stats", 0, awl_sys->proc_dir,
	    wl_awl_stats_file_read_proc);
	if (awl_sys->proc_stats != NULL) {
	    return;
	}

	WL_AWL_ERROR("%s failed to create [wl_awl/stats] proc file\n",
	    __FUNCTION__);

	wl_awl_proc_deinit(awl, unit);

	return;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Forward sk_buff (sll) to the Archer Wireless
 *
 * -----------------------------------------------------------------------------
 */
static bcmFun_t *wl_awl_rx_send_hook_g = NULL;

int wl_awl_upstream(wl_awl_t * awl, pktlist_t * pktlist)
{
	WL_AWL_PTRACE("wl_awl: pktlist 0x%px", pktlist);

	if (likely(pktlist->len != 0)) {
		/* ingress_port for wlan is 0 */
#if defined(CONFIG_BCM_CROSSBOW_FULL_OFFLOAD) && defined(CL474762)
		archer_wlan_rx_send(pktlist, wl_awl_rx_send_hook_g);
#else /* ! CONFIG_BCM_CROSSBOW_FULL_OFFLOAD */
		archer_wlan_rx_send(pktlist->head, wl_awl_rx_send_hook_g);
#endif /* ! CONFIG_BCM_CROSSBOW_FULL_OFFLOAD */

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
void wl_awl_pktlist_add(pktlist_t * pktlist, struct sk_buff *skb)
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
void wl_awl_pktlist_free(struct wl_info *wl, pktlist_t * pktlist)
{
	struct sk_buff *skb;
	int pkts;

	pkts = pktlist->len;
	if (pkts != 0) {
		skb = (struct sk_buff *)pktlist->head;

		while (pkts--) {
			struct sk_buff *nskb = skb->prev;
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
 * Function : free skb's from SLL by the given net_device
 *            unlink and free the packets
 *
 * -----------------------------------------------------------------------------
 */
int
wl_awl_pktlist_flush(struct wl_info *wl, pktlist_t *pktlist, struct net_device *dev)
{
	struct sk_buff *skb;
	int pkts;
	int dropped_pkts = 0;

	pkts = pktlist->len;
	if (pkts != 0) {
		skb = (struct sk_buff *)pktlist->head;
		PKTLIST_RESET(pktlist);

		while (pkts--) {
			struct sk_buff *nskb = skb->prev;
			skb->next = skb->prev = NULL;

			if (skb->dev == dev) {
				PKTFREE(wl->pub->osh, skb, FALSE);
				dropped_pkts++;
			} else {
				wl_awl_pktlist_add(pktlist, skb);
			}
			skb = nskb;
		}
	}

	return dropped_pkts;
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
int wl_awl_rx_sendup(struct wl_info *wl, struct sk_buff *skb)
{

#if defined(BCM_NBUFF_PKT) && defined(CC_BPM_SKB_POOL_BUILD)
	PKTTAINTED(wl->osh, skb);
#endif /* BCM_NBUFF_PKT && CC_BPM_SKB_POOL_BUILD */

	/* Copied essesntial from wl_sendup() */
#if defined(CONFIG_BCM_SPDSVC) || defined(CONFIG_BCM_SPDSVC_MODULE) || defined(WLBIN_COMPAT)
	if (wl_spdsvc_rx(skb) == BCME_OK) {
		return PKT_DONE;
	}
#endif /* CONFIG_BCM_SPDSVC || CONFIG_BCM_SPDSVC_MODULE || WLBIN_COMPAT */

#ifdef BCM_BLOG
	if (wl_handle_blog_sinit(wl, skb) == PKT_DONE) {
		return PKT_DONE;
	}
#endif /* BCM_BLOG */

	/* Process intra-bss packets as they are not handled in bridge or wl_linux */
	if (wl_awl_rx_process_intrabss(wl, skb) == BCME_OK) {
		return PKT_DONE;
	}

	skb->protocol = eth_type_trans(skb, skb->dev);

#ifdef NAPI_POLL
	netif_receive_skb(skb);
#else /* NAPI_POLL */
	if (in_interrupt())
		netif_rx(skb);
	else
		netif_rx_ni(skb);
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
void wl_awl_rx_flow_miss_handler_archer_sll(void *ctxt, pktlist_t * misspktl)
{
	struct wl_info *wl = (struct wl_info *)ctxt;
	struct sk_buff *skb;
	wl_awl_t *awl;
	int npkts;

	if ((wl == NULL) || (misspktl == NULL)) {
		WL_AWL_ERROR("%s (0x%px, 0x%px) with NULL parameters\n",
			__FUNCTION__, wl, misspktl);
		return;
	}

	awl = WL_AWL_CB(wl);

	/* De-link packets from the SLL */
	skb = misspktl->head;
	npkts = misspktl->len;
	PKTLIST_RESET(misspktl);

	/* update chain packet status */
	awl->rx.a2w_chn_packets = npkts;
	if (awl->rx.a2w_chn_packets > awl->rx.a2w_max_chn)
		awl->rx.a2w_max_chn = awl->rx.a2w_chn_packets;

	/* Let wl process the packets */
	WL_LOCK(wl);

	/* Process each packet */
	while (npkts--) {
		struct sk_buff *nskb = skb->prev;
		int rc = PKT_DROP;
		struct net_device *net;

		skb->next = skb->prev = NULL;

		awl->rx.a2w_rx_packets++;

		net = wl_awl_skb_intf_dev(awl, skb);
		if (net && (net == skb->dev)) {
			/* Process the flow-miss packets */
			rc = wl_awl_rx_sendup(wl, skb);
		}

		if (rc != PKT_DROP) {
			awl->rx.a2w_fwd_packets++;
			awl->rx.a2w_fwd_calls++;
		} else {
			PKTFREE(wl->pub->osh, skb, FALSE);
			awl->rx.a2w_flt_packets++;
		}

		skb = nskb;
	}

	WL_UNLOCK(wl);

	return;
}

void wl_awl_rx_flow_miss_handler_archer_skb(void *ctxt, struct sk_buff *skb)
{
	struct wl_info *wl = (struct wl_info *)ctxt;
	pktlist_t misspktl;

	if ((wl == NULL) || (skb == NULL)) {
		WL_AWL_ERROR("%s (0x%px, 0x%px) with NULL parameters\n", __FUNCTION__, wl, skb);
		return;
	}

	PKTLIST_RESET(&misspktl);
	wl_awl_pktlist_add(&misspktl, skb);

	return wl_awl_rx_flow_miss_handler_archer_sll(ctxt, &misspktl);
}

/**
 * -----------------------------------------------------------------------------
 * Function : Callback function from Archer CPU for flow-miss
 *
 *   appends flow-miss packet list to WLAN SLL and invokes WL dpc
 *   minimum processing in Archer context
 * -----------------------------------------------------------------------------
 */
void wl_awl_rx_flow_miss_handler_wl_dpc_sll(void *ctxt, pktlist_t * misspktl)
{
	struct wl_info *wl = (struct wl_info *)ctxt;
	wl_awl_t *awl;
	pktlist_t *pktlist;
	int npkts;

	/* Get the WL public control block */
	if ((wl == NULL) || (misspktl == NULL)) {
		WL_AWL_ERROR("%s (0x%px, 0x%px) with NULL parameters\n",
			__FUNCTION__, wl, misspktl);
		return;
	}

	if ((npkts = misspktl->len) == 0) {
		/* No patckets in the list */
		return;
	}

	/* Add to the slow path SLL with a lock, This is happening from Archer context */
	awl = WL_AWL_CB(wl);
	pktlist = WL_AWL_RX_A2W_PKTL(awl);

	WL_AWL_PKTLIST_LOCK(awl->rx.a2w_pktl_lock);

	/* Apend to the a2w SLL */
	if (likely(pktlist->len != 0)) {
		/* pend to tail */
		PKTLIST_PKT_SET_SLL(pktlist->tail, misspktl->head, SKBUFF_PTR);
	} else {
		pktlist->head = misspktl->head;
	}
	pktlist->tail = misspktl->tail;
	pktlist->len += npkts;

	/* Wakeup DPC thread or run DPC tasklet */
	atomic_add(npkts, &wl->callbacks);
#ifdef WL_ALL_PASSIVE
	wl->awl_sp_rxq_dispatched = true;
#endif /* WL_ALL_PASSIVE */

	WL_AWL_PKTLIST_UNLK(awl->rx.a2w_pktl_lock);

	PKTLIST_RESET(misspktl);

	awl->rx.a2w_rx_packets += npkts;
	awl->rx.a2w_chn_packets = npkts;
	if (awl->rx.a2w_chn_packets > awl->rx.a2w_max_chn)
		awl->rx.a2w_max_chn = awl->rx.a2w_chn_packets;

#ifdef WL_ALL_PASSIVE
	wake_up_interruptible(&wl->kthread_wqh);
#endif /* WL_ALL_PASSIVE */

	return;
}

void wl_awl_rx_flow_miss_handler_wl_dpc_skb(void *ctxt, struct sk_buff *skb)
{
	struct wl_info *wl = (struct wl_info *)ctxt;
	pktlist_t misspktl;

	/* Get the WL public control block */
	if ((wl == NULL) || (skb == NULL)) {
		WL_AWL_ERROR("%s (0x%px, 0x%px) with NULL parameters\n",
			__FUNCTION__, wl, skb);
		return;
	}

	PKTLIST_RESET(&misspktl);
	wl_awl_pktlist_add(&misspktl, skb);

	return wl_awl_rx_flow_miss_handler_wl_dpc_sll(ctxt, &misspktl);
}

/**
 * -----------------------------------------------------------------------------
 * Function : For Archer flow-miss packets process Intra BSS as they are not
 *            handled in the bridge.
 *
 *            Check if the packet is destined to the same interface using
 *            SCB lookup. If intrabss, attach a blog, 16bit PKTFWD key and
 *            send it to WLAN driver.
 *            Flowcache will learn about this flow and the following packets
 *            will be offloaded to Archer.
 * -----------------------------------------------------------------------------
 */
int wl_awl_rx_process_intrabss(struct wl_info *wl, struct sk_buff *skb)
{
	uint8_t *d3_addr;
	wlc_if_t *wlcif;
	struct net_device *net_device;
	wlc_bsscfg_t *bsscfg;
	bool intrabss = false;
	struct ether_header *eh = (struct ether_header *)PKTDATA(wl->osh, skb);
	d3_addr = (uint8_t *) eh->ether_dhost;
	net_device = skb->dev;

#ifndef BCM_WLAN_PER_CLIENT_FLOW_LEARNING
	if (ETHER_ISMULTI(d3_addr) ||
	    (WL_DEV_IF(net_device)->if_type == WL_IFTYPE_WDS))
#else
	if ((WL_DEV_IF(net_device)->if_type == WL_IFTYPE_WDS))
#endif
	{
		/* WDS and Multicast packets are not supported */
		return BCME_UNSUPPORTED;
	}

	wlcif = WL_DEV_IF(net_device)->wlcif;
	bsscfg = wlc_bsscfg_find_by_wlcif(wl->wlc, wlcif);

#ifdef BCM_WLAN_PER_CLIENT_FLOW_LEARNING
	if (ETHER_ISMULTI(d3_addr))
		return wl_pktfwd_rx_mcast_handler(WL_DEV_IF(net_device), skb);
#endif

#if defined(PKT_INTRABSS_FWD)
	skb_cb_zero(skb);	/* explicitly zero cb[], wl_cb[] */
#endif

	/* Forward packets destined within the BSS */
	if (BSSCFG_AP(bsscfg) && !bsscfg->ap_isolate) {
		struct scb *scb;

		if ((eacmp(d3_addr, wl->pub->cur_etheraddr.octet) != 0) &&
		    (scb = wlc_scbfind(wl->wlc, bsscfg, (struct ether_addr *)d3_addr))) {
			/* Check that the dst is associated to same BSS
			 * before forwarding within the BSS.
			 */
			if (SCB_ASSOCIATED(scb) &&
#ifdef WL_MLO
				((mlo_role(wl->wlc->mlc_dev) == MLC_ROLE_MAP) ||
					!SCB_MLO_CAP(scb)) &&
#endif /* WL_MLO */
				TRUE) {
				intrabss = true;
			}
		}
	}

	if (intrabss == true) {
		xmit_fn_t netdev_ops_xmit;

#if defined(BCM_BLOG)
		/* Attach a blog and PKTFWD 16bit key */
		if (skb->blog_p != NULL) {
			uint32_t chain_idx = PKTC_INVALID_CHAIN_IDX;

			chain_idx =
			    wl_pktc_req(PKTC_TBL_UPDATE, (unsigned long)d3_addr,
					(unsigned long)net_device, 0);

			if (chain_idx != PKTC_INVALID_CHAIN_IDX) {
				/* Update chain_idx in blog */
				skb->blog_p->wfd.nic_ucast.is_tx_hw_acc_en = 1;
				skb->blog_p->wfd.nic_ucast.is_chain = 1;
				skb->blog_p->wfd.nic_ucast.chain_idx =
				    chain_idx;
				skb->blog_p->wfd.nic_ucast.wfd_idx =
				    ((chain_idx & PKTC_WFD_IDX_BITMASK) >>
				     PKTC_WFD_IDX_BITPOS);
			}
		}
#endif /* BCM_BLOG */

		/* Send it WLAN driver wl_start() */
		netdev_ops_xmit =
		    (xmit_fn_t) (net_device->netdev_ops->ndo_start_xmit);
		netdev_ops_xmit(skb, net_device);

		return BCME_OK;
	}

	return BCME_ERROR;
}				/* wl_awl_rx_process_intrabss() */


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
bool wl_awl_process_slowpath_rxpkts(struct wl_info * wl)
{
	bool more = FALSE;
	wl_awl_t *awl;
	pktlist_t *pktlist;
	int npkts, ppkts = 0;
	struct sk_buff *skb = NULL;

	awl = WL_AWL_CB(wl);

	/* Get the pending packets list */
	pktlist = WL_AWL_RX_A2W_PKTL(awl);

	/* Nothing to do if mode is not WL DPC */
	if (awl->rx.mode != WL_AWL_RX_MODE_FULL)
		goto done;

	WL_AWL_PKTLIST_LOCK(awl->rx.a2w_pktl_lock);

	npkts = pktlist->len;
	if (npkts == 0) {
		WL_AWL_PKTLIST_UNLK(awl->rx.a2w_pktl_lock);
		goto done;
	}

	skb = (struct sk_buff *)pktlist->head;
	if (npkts > awl->rx.bound) {
		struct sk_buff *nskb = skb;
		ppkts = npkts = awl->rx.bound;

		/* Unlink first npkts from the SLL */
		do {
			nskb = nskb->prev;
		} while (--ppkts);

		pktlist->head = nskb;
		pktlist->len -= npkts;
	} else {
		PKTLIST_RESET(pktlist);
	}

	if (pktlist->len == 0) {
#ifdef WL_ALL_PASSIVE
		wl->awl_sp_rxq_dispatched = FALSE;
#endif /* WL_ALL_PASSIVE */
	} else {
		more = true;
	}
	atomic_sub(npkts, &wl->callbacks);

	WL_AWL_PKTLIST_UNLK(awl->rx.a2w_pktl_lock);

	/* Let wl process the packets */
	WL_LOCK(wl);

	do {
		struct sk_buff *nskb = skb->prev;
		struct net_device *net;
		int rc = PKT_DROP;

		skb->next = skb->prev = NULL;
		/* Process the flow-miss packets */
		net = wl_awl_skb_intf_dev(awl, skb);
		if (net && (net == skb->dev)) {
			rc = wl_awl_rx_sendup(wl, skb);
		}

		if (rc != PKT_DROP) {
			awl->rx.a2w_fwd_packets++;
			awl->rx.a2w_fwd_calls++;
		} else {
			PKTFREE(wl->pub->osh, skb, FALSE);
			awl->rx.a2w_flt_packets++;
		}

		skb = nskb;
		ppkts++;
	} while (--npkts);

	WL_UNLOCK(wl);

done:

	return more;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Function to send Rx SLL to Archer
 *
 *  Called from from protocol layer for commiting the packets to archer
 * -----------------------------------------------------------------------------
 */
void wl_awl_upstream_send_all(struct wl_info *wl)
{
	wl_awl_t *awl = WL_AWL_CB(wl);
	pktlist_t *pktlist = WL_AWL_RX_W2A_PKTL(awl);

	WL_AWL_PTRACE("wl%d_awl: pktlist 0x%px", wl->unit, pktlist);

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
wl_awl_upstream_add_pkt(struct wl_info *wl, struct net_device *net_device,
	void *pkt, uint16_t flowid)
{
	wl_awl_t *awl = WL_AWL_CB(wl);
	pktlist_t *pktlist = WL_AWL_RX_W2A_PKTL(awl);
	struct sk_buff *skb;
	wl_if_t *wlif;

	WL_AWL_PTRACE("wl%d pkt %px", wl->unit, pkt);

	awl->rx.w2a_rx_packets++;

	skb = (struct sk_buff *)pkt;

	/* pass through mode or flow cache disabled, process within wlan thread */
	if (awl->rx.mode == WL_AWL_RX_MODE_PT) {
#if defined(BCM_WLAN_PER_CLIENT_FLOW_LEARNING) && defined(WL_PKTFWD_INTRABSS)
		uint8_t *d3_addr = (uint8_t *) PKTDATA(wl->osh, skb);
		if (ETHER_ISMULTI(d3_addr) &&
			wl_pktfwd_rx_mcast_handler(WL_DEV_IF(net_device), skb) == BCME_OK) {
			awl->rx.w2a_flt_packets++;
			return;
		}
#endif

		if (wl_intrabss_forward(wl, net_device, skb) == FALSE) {
			wl_sendup(wl, WL_DEV_IF(net_device), skb, 1);
		}
		awl->rx.w2a_flt_packets++;
		return;
	}
#ifndef PKT_INTRABSS_FWD
	skb_cb_zero(skb);	/* explicitly zero cb[], wl_cb[] */
#endif
	skb->dev = net_device;

	if (skb->prev) {
		printk("0x%px <- 0x%px ->  0x%px\n", skb->prev, skb, skb->next);
		skb->prev = NULL;
	}

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
int wl_awl_upstream_send_chain(struct wl_info *wl, struct sk_buff *skb)
{
	wl_awl_t *awl = WL_AWL_CB(wl);
	pktlist_t *pktlist = WL_AWL_RX_W2A_PKTL(awl);
	wl_if_t *wlif;
	struct net_device *net = skb->dev;
	void *nskb = NULL;

	WL_AWL_PTRACE("wl%d skb 0x%px 0x%px %d", wl->unit, skb, skb->data, skb->len);

	/* pass through mode or flow cache disabled, process within wlan thread */
	if (awl->rx.mode == WL_AWL_RX_MODE_PT) {
		bool is_mcast =
		    ETHER_ISMULTI((uint8_t *) PKTDATA(wl->osh, skb));
#if defined(BCM_WLAN_PER_CLIENT_FLOW_LEARNING) && defined(WL_PKTFWD_INTRABSS)
		if (is_mcast && wl_pktfwd_rx_mcast_handler(WL_DEV_IF(net), skb) == BCME_OK) {
			awl->rx.w2a_rx_packets++;
			return BCME_OK;
		}
#endif
		if (!is_mcast && wl_intrabss_forward(wl, net, skb)) {
			return BCME_OK;
		} else {
			return BCME_ERROR;
		}
	}

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
#ifndef PKT_INTRABSS_FWD
		skb_cb_zero(skb);	/* explicitly zero cb[], wl_cb[] */
#endif

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
wl_awl_upstream_match(uint8_t * d3addr, struct net_device *rx_net_device)
{
	/* Always return true, as all packets are sent to Archer first */
	/* TODO: What about same radio ? */
	return true;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
/**
 * -----------------------------------------------------------------------------
 * Function : Helper debug dump the AWL rx statistics
 * -----------------------------------------------------------------------------
 */
static int wl_awl_dump(void *ctx, struct bcmstrbuf *b)
{
	struct wl_info *wl = ctx;
	wl_awl_t *awl = WL_AWL_CB(wl);

	WL_AWL_PTRACE("wl%d_awl:\n", wl->unit);

	wl_awl_dump_radio(awl, b, false);

	return BCME_OK;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Helper to reset statistics collected
 * -----------------------------------------------------------------------------
 */
static int wl_awl_clr_stats(void *ctx)
{
	struct wl_info *wl = ctx;
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

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

/**
 * -----------------------------------------------------------------------------
 * Function : WL AWL initialization function
 *            Initialize AWL RX control block, register with Archer
 * -----------------------------------------------------------------------------
 */
void *wl_awl_attach(struct wl_info *wl, uint unit)
{
	wl_awl_t *awl = NULL;
	archer_wlan_rx_miss_handler_t a2w_flow_miss_handler =
	    wl_awl_rx_flow_miss_handler_wl_dpc;
	int rxmode = WL_AWL_RX_MODE_PT;

	WL_AWL_PTRACE("wl%d_awl: wl 0x%px", unit, wl);

	if (unit >= WL_AWL_MAX_RADIOS) {
	    /* Can not support this radio */
	    WL_AWL_ERROR("%s unit [%d] exceeds max supported units\n",
	        __FUNCTION__, unit);
	    return NULL;
	}

	awl = WL_AWL_CB(wl);
	memset(awl, 0, sizeof(wl_awl_t));
	awl->tx.mode = 1;

#if defined(WL_AWL_RX)
	{
		char varname[] = "wlXX_awl_rxmode";
		char *var;
		rxmode = WL_AWL_RX_MODE_FULL;

		snprintf(varname, sizeof(varname), "wl%d_awl_rxmode", unit);
		var = getvar(NULL, varname);
		if (var) {
			rxmode = bcm_strtoul(var, NULL, 0);
		}
	}
#endif /* WL_AWL_RX */

	if (unit >= WL_PKTFWD_RADIOS) {
	    /* Can not support PKTFWD acceleration, force passthrough mode */
	    rxmode = WL_AWL_RX_MODE_PT;
	    awl->tx.mode = 0;
	    WL_AWL_LOG("%s unit [%d] exceeds max acceleration units, disabling\n",
	        __FUNCTION__, unit);
	}


	/* Register with Archer call back functions for Rx */
	if (archer_wlan_rx_register(unit, a2w_flow_miss_handler, wl) != 0) {
	    /* Archer can not support acceleration, force passthrough mode */
	    rxmode = WL_AWL_RX_MODE_PT;
	    awl->tx.mode = 0;
	    WL_AWL_LOG("%s unit [%d] acceleration failed, disabling\n",
	        __FUNCTION__, unit);
	}

	if (rxmode == WL_AWL_RX_MODE_LITE) {
		a2w_flow_miss_handler = wl_awl_rx_flow_miss_handler_archer;
	}

	awl->rx.mode = rxmode;

	awl->rx.bound = WL_AWL_RX_PKT_BOUND;

	awl->rx.w2a_pktl.head = PKTLIST_PKT_NULL;
	awl->rx.w2a_pktl.tail = PKTLIST_PKT_NULL;
	awl->rx.w2a_pktl.len = 0;

	awl->rx.a2w_pktl.head = PKTLIST_PKT_NULL;
	awl->rx.a2w_pktl.tail = PKTLIST_PKT_NULL;
	awl->rx.a2w_pktl.len = 0;

	spin_lock_init(&awl->rx.a2w_pktl_lock);

	wl_awl_rx_send_hook_g = bcmFun_get(BCM_FUN_ID_ARCHER_WLAN_RX_SEND);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* Register dump hook with WLAN "awl" module for iovar queries */
	wlc_dump_add_fns(WLC_DUMP_INST(wl), "awl", wl_awl_dump, wl_awl_clr_stats, wl);
#endif /* BCMDBG || BCMDBG_DUMP */

#ifdef WL_ALL_PASSIVE
	wl->awl_sp_rxq_dispatched = FALSE;
#endif /* WL_ALL_PASSIVE */

	wl_awl_proc_init(awl, unit);

	WL_AWL_LOG("wl%d_awl attach successful tx: %d, rx: %d\n",
	    unit, awl->tx.mode, awl->rx.mode);

	return (void *)awl;
}

/**
 * -----------------------------------------------------------------------------
 * Function : WL AWL de-initialization function
 *            de-register with archer, free pending packets in the SLL's
 * -----------------------------------------------------------------------------
 */
void wl_awl_detach(struct wl_info *wl, void *ctxt)
{
	wl_awl_t *awl = WL_AWL_CB(wl);
	pktlist_t *pktlist;
	int npkts;

	WL_AWL_PTRACE("wl%d_awl: wl 0x%px, awl 0x%px", wl->unit, wl, awl);

	if (awl == NULL) {
	    /* nothing todo */
	    return;
	}

	/* De-register with Archer call back functions for Rx */
	archer_wlan_rx_register(wl->unit, NULL, NULL);

	pktlist = WL_AWL_RX_A2W_PKTL(awl);
	if (pktlist->len) {
		/* Free A2W packet SLL under lock */
		WL_AWL_PKTLIST_LOCK(awl->rx.a2w_pktl_lock);
		npkts = pktlist->len;
		wl_awl_pktlist_free(wl, pktlist);
		WL_AWL_PKTLIST_UNLK(awl->rx.a2w_pktl_lock);
		if (npkts)
			atomic_sub(npkts, &wl->callbacks);
	}

	/* Free Lock-less W2A packet SLL */
	wl_awl_pktlist_free(wl, WL_AWL_RX_W2A_PKTL(awl));

	wl_awl_proc_deinit(awl, wl->unit);

	WL_AWL_LOG("wl%d_awl detach complete", wl->unit);

	return;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Function to register net device
 * -----------------------------------------------------------------------------
 */
int
wl_awl_register_dev(struct net_device *dev)
{
	if (dev && is_netdev_wlan(dev)) {
	    wl_if_t *wlif = WL_DEV_IF(dev);

	    if (wlif && wlif->wl) {
	        int ifidx = wlif->subunit;
	        wl_awl_t *awl = WL_AWL_CB(wlif->wl);

	        WL_LOCK(wlif->wl);

	        if (awl->ifs[ifidx] != NULL) {
	            WL_AWL_ERROR("wl%d_awl register [%s] overwrite %d, 0x%px\n",
	                    wlif->wl->unit, dev->name, ifidx, awl->ifs[ifidx]);
	        }
	        awl->ifs[ifidx] = dev;
	        WL_UNLOCK(wlif->wl);
	        WL_AWL_LOG("wl%d_awl registered [%s]\n", wlif->wl->unit, dev->name);
	    }
	}

	return 0;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Function to unregister net device
 * -----------------------------------------------------------------------------
 */
int
wl_awl_unregister_dev(struct net_device *dev)
{
	if (dev && is_netdev_wlan(dev)) {
		wl_if_t *wlif = WL_DEV_IF(dev);
		int ifidx;

		if (wlif && wlif->wl) {
			wl_awl_t *awl = WL_AWL_CB(wlif->wl);
#if !defined(WL_AWL_SKIP_UNREGISTER_FLUSH)
			int flt_pkts;

			if (awl == NULL) {
				/* nothing todo */
				return 0;
			}

			/* Flush A2W packet SLL under lock */
			WL_AWL_PKTLIST_LOCK(awl->rx.a2w_pktl_lock);
			flt_pkts = wl_awl_pktlist_flush(wlif->wl, WL_AWL_RX_A2W_PKTL(awl), dev);
			awl->rx.a2w_flt_packets += flt_pkts;
			atomic_sub(flt_pkts, &wlif->wl->callbacks);
			WL_AWL_PKTLIST_UNLK(awl->rx.a2w_pktl_lock);
#endif /* !WL_AWL_SKIP_UNREGISTER_FLUSH */

			WL_LOCK(wlif->wl);

			/* Flush W2A packet SLL */
#if !defined(WL_AWL_SKIP_UNREGISTER_FLUSH)
			flt_pkts = wl_awl_pktlist_flush(wlif->wl, WL_AWL_RX_W2A_PKTL(awl), dev);
			awl->rx.w2a_flt_packets += flt_pkts;
#endif /* !WL_AWL_SKIP_UNREGISTER_FLUSH */

			ifidx = wlif->subunit;
			if (ifidx < WL_MAX_IFS) {
				if (dev != awl->ifs[ifidx]) {
					WL_AWL_ERROR("wl%d_awl unregister [%s] mismatch %d 0x%px\n",
						wlif->wl->unit, dev->name, ifidx, awl->ifs[ifidx]);
				}
				awl->ifs[ifidx] = NULL;
			} else {
				WL_AWL_ERROR("wl%d_awl unregister [%s] dev unknown\n",
					wlif->wl->unit, dev->name);
			}
			WL_UNLOCK(wlif->wl);
			WL_AWL_LOG("wl%d_awl unregistered [%s]\n", wlif->wl->unit,
				dev->name);
		}
	}

	return 0;
}
