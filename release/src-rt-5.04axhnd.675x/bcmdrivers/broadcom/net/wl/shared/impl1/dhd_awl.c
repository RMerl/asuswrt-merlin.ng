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
 * TODO - RX
 *  - BPM buffers i.o skb's
 *  - Statistics update (for fast path) ?
 *  - rxbound for slow path packets
 *
 * =============================================================================
 */

/**
 * =============================================================================
 * dhd context
 * ===========
 * dhd_bus_dpc()
 * {
 *   dhdpcie_bus_process_mailbox_intr()
 *     dhd_bus_readframes()
 *       dhd_prot_process_msgbuf_rxcpl()
 *          dhd_prot_process_rxcpln()
 *            *dhd_awl_upstream_add()             => Add packet to lock-less SLL
 *            * ......                            .........
 *            *dhd_awl_upstream_add()             => Add packet to lock-less SLL
 *            *dhd_awl_upstream_send()            send to Archer (archer_wlan_rx_send)
 *            ....
 *         ....
 *     ....
 *   ....
 *   *dhd_awl_process_slowpath_rxpkts()
 *      => unlink packets list from lock based SLL.
 *      dhd_bus_rx_frame()                        => Process Rx packets
 * }
 *
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
 *   *dhd_awl_rx_flow_miss_handler_dhd_dpc()
 *     => Add packet to the lock based SLL.
 *     dhd_sched_dpc()                            => Wake up dhd dpc
 * }
 * =============================================================================
 */

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <linux/types.h>
#include <linux/netdevice.h>

#include <bcm_pktfwd.h>
#include <dhd_pktfwd.h>
#include <ethernet.h>
#include <bcmevent.h>
#include <wl_pktc.h>

#include <dhd.h>
#include <dhd_linux.h>
#include <dhd_flowring.h>
#include <dhd_bus.h>
#include <dhd_dbg.h>
#include <dhd_wfd.h>

#include <bcmendian.h>
#include <pktHdr.h>
#include <bcm_archer.h>

#include <dhd_awl.h>

/**
 * =============================================================================
 * Section: DHD_AWL Local defines and macros
 * =============================================================================
 */

/**
 * Pre-processor flags, (few in dhd_awl.h)
 */
/*
 * Enable this flag for debug message control with-in this file
 * default disabled
 */
//#define DHD_AWL_DEBUG

/*
 * Enable this flag for checking skb link pointers
 * default disabled
 */
#define DHD_AWL_SKB_AUDIT

#define DHD_AWL_CB(dhdp)             (&dhd_awl_cb_g[(dhdp)->unit])
#define DHD_AWL_RX_W2A_PKTL(awl)     (&awl->rx.w2a_pktl)
#define DHD_AWL_RX_A2W_PKTL(awl)     (&awl->rx.a2w_pktl)

/* Debug macros */
#if defined(DHD_AWL_DEBUG)
#define DHD_AWL_PTRACE               printk
#define DHD_AWL_TRACE                printk
#define DHD_AWL_ERROR                printk
#else /* !DHD_AWL_DEBUG */
#define DHD_AWL_PTRACE               PKTFWD_PTRACE
#define DHD_AWL_TRACE                PKTFWD_TRACE
#define DHD_AWL_ERROR                PKTFWD_ERROR
#endif /* !DHD_AWL_DEBUG */
#define DHD_AWL_LOG                  printk

#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define DHD_AWL_PKTLIST_LOCK(lock)   spin_lock_bh(&(lock))
#define DHD_AWL_PKTLIST_UNLK(lock)   spin_unlock_bh(&(lock))
#else
#define DHD_AWL_PKTLIST_LOCK(lock)   local_irq_disable()
#define DHD_AWL_PKTLIST_UNLK(lock)   local_irq_enable()
#endif  /* ! (CONFIG_SMP || CONFIG_PREEMPT) */

/**
 * Upstream path
 */
/* Processing modes */
#define DHD_AWL_RX_MODE_PT           0
#define DHD_AWL_RX_MODE_LITE         1
#define DHD_AWL_RX_MODE_FULL         2

/* Packet bound */
#if defined(ARCHER_WLAN_RX_BUDGET)
#define DHD_AWL_RX_PKT_BOUND         ARCHER_WLAN_RX_BUDGET
#else /* !ARCHER_WLAN_RX_BUDGET */
#define DHD_AWL_RX_PKT_BOUND         32   /* 64 */
#endif /* !ARCHER_WLAN_RX_BUDGET */

#define DHD_AWL_A2W_MAX_PKTL_LEN	0

/**
 * Downstream path
 */
/* Processing modes */                 /* Archer Acceleration */
#define DHD_AWL_TX_MODE_OFF          0 /* Off */
#define DHD_AWL_TX_MODE_ON           1 /* ON */
#define DHD_AWL_TX_MODE_DPC          2 /* DHD in DPC with fast path in dhd CPU context */
#define DHD_AWL_TX_MODE_LAST         3

#if defined(DHD_AWL_SKB_AUDIT)
/* Check for non-null skb->prev, log and set to null */
#define DHD_AWL_SKB_PREV_AUDIT(skb)    \
	do {                               \
	    if (skb->prev)                 \
	        printk("%s: 0x%px <- 0x%px ->  0x%px\n", __FUNCTION__, skb->prev, skb, skb->next); \
	    skb->prev = NULL;              \
	} while (0)

/* Check for non-null skb->next, log and set to null */
#define DHD_AWL_SKB_NEXT_AUDIT(skb)    \
	do {                               \
	    if (skb->next)                 \
	        printk("%s: 0x%px <- 0x%px ->  0x%px\n", __FUNCTION__, skb->prev, skb, skb->next); \
	        skb->next = NULL;          \
	} while (0)
#else /* !DHD_AWL_SKB_AUDIT */
#define DHD_AWL_SKB_PREV_AUDIT(skb)   do { } while (0)
#define DHD_AWL_SKB_NEXT_AUDIT(skb)   do { } while (0)
#endif /* !DHD_AWL_SKB_AUDIT */

#if defined(ARCHER_WLAN_MISS_PKTLIST)
#define dhd_awl_rx_flow_miss_handler_dhd_dpc dhd_awl_rx_flow_miss_handler_dhd_dpc_sll
#define dhd_awl_rx_flow_miss_handler_archer  dhd_awl_rx_flow_miss_handler_archer_sll
#define dhd_awl_rx_flow_miss_handler_archer_dhd dhd_awl_rx_flow_miss_handler_archer_dhd_sll
#else /* !ARCHER_WLAN_MISS_PKTLIST */
#define dhd_awl_rx_flow_miss_handler_dhd_dpc dhd_awl_rx_flow_miss_handler_dhd_dpc_skb
#define dhd_awl_rx_flow_miss_handler_archer  dhd_awl_rx_flow_miss_handler_archer_skb
#define dhd_awl_rx_flow_miss_handler_archer_dhd dhd_awl_rx_flow_miss_handler_archer_dhd_skb
#endif /* !ARCHER_WLAN_MISS_PKTLIST */

/**
 * -----------------------------------------------------------------------------
 * struct dhd_awl_rx
 *
 * - mode            : pass through (disable), Lite, Full
 * - bound           : Rx packet bound to pass to Archer WLAN
 *
 * - w2a_pktl        : Rx Packet List WLAN -> Archer
 * - a2w_pktl        : Rx Packet List Archer -> WLAN (flow-miss)
 * - a2w_pktl_lock   : Lock for a2w packet list
 *
 * - w2a_rx_packets  : Count of Rx packets received by dhd_awl for Archer WLAN
 * - w2a_flt_packets : Count of Rx packets filtered (not sent to Archer)
 * - w2a_fwd_packets : Count of Rx packets forwarded to Archer WLAN
 * - w2a_fwd_calls   : Count of packet forward calls to Archer
 *
 * - w2a_rx_packets  : Count of Rx packets received by dhd_awl from Archer (flow-miss)
 * - w2a_flt_packets : Count of Rx packets filtered (not sent to DHD)
 * - w2a_fwd_packets : Count of Rx flow-miss packets forwarded to DHD
 * - w2a_fwd_calls   : Count of flow-miss packet forward calls to DHD
 *
 * -----------------------------------------------------------------------------
 */
typedef struct dhd_awl_rx {
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
	unsigned long a2w_chn_packets;
	unsigned long a2w_max_chn;
	unsigned long a2w_max_pktl_len;
	unsigned long a2w_discard_packets;
} dhd_awl_rx_t;

/**
 * -----------------------------------------------------------------------------
 * struct dhd_awl_tx
 *
 * - mode            : disable, enable
 * -----------------------------------------------------------------------------
 */
typedef struct dhd_awl_tx {
	uint8             mode;
	pktlist_context_t *awl_pktlist_context;
	unsigned long     a2w_rx_calls;
	unsigned long     a2w_flt_calls;
	unsigned long     a2w_fwd_items;
	unsigned long     a2w_fwd_calls;
	unsigned long     a2w_chn_items;
	unsigned long     a2w_max_chn;
} dhd_awl_tx_t;

/**
 * -----------------------------------------------------------------------------
 * struct dhd_awl
 *
 * - tx            : down stream path cb
 * - rx            : upstream path cb
 * -----------------------------------------------------------------------------
 */
typedef struct dhd_awl {
	struct dhd_awl_rx rx;
	struct dhd_awl_tx tx;
} dhd_awl_t;

/**
 * =============================================================================
 * Section: External function declerations
 * =============================================================================
 */
extern int  fcacheStatus(void);
extern int dhd_dpc_prio;

/**
 * =============================================================================
 * Section: DHD_AWL Global System Object(s)
 * =============================================================================
 */
struct dhd_awl dhd_awl_cb_g[DHD_PKTFWD_RADIOS] = {};

char dhd_awl_rx_mode_str_g[][8] = {
	"OFF", "LITE", "YES"
};

char dhd_awl_tx_mode_str_g[][8] = {
	"OFF", "YES", "DPC"
};

/**
 * =============================================================================
 * Section: DHD_AWL Local function definitions
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 * WLAN Receive Path Forwarding
 *
 * Function : Forward sk_buff (sll) to the Archer Wireless
 * -----------------------------------------------------------------------------
 */
int
dhd_awl_upstream(dhd_awl_t *awl, pktlist_t *pktlist)
{
	DHD_AWL_PTRACE("dhd_awl: pktlist 0x%px", pktlist);

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
dhd_awl_pktlist_add(pktlist_t *pktlist, struct sk_buff *skb)
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
 * Function : Callback function from Archer CPU for flow-miss
 *            completely handled with-in Archer context
 *            Might miss some features (dump, wmf, wet, spdsvc .... etc) otherwise
 *            good for initial integration
 * -----------------------------------------------------------------------------
 */
void
dhd_awl_rx_flow_miss_handler_archer_sll(void *ctxt, pktlist_t *misspktl)
{
	dhd_pub_t *dhdp = (dhd_pub_t*)ctxt;
	uint8_t *eth;
	unsigned int len;
	BlogAction_t blog_action;
	dhd_awl_t *awl;
	struct sk_buff *skb;
	int npkts;

	/* Get the DHD public control block */
	if ((dhdp == NULL) || (misspktl == NULL)) {
	    DHD_AWL_ERROR("%s (0x%px, 0x%px) with NULL parameters\n",
	        __FUNCTION__, dhdp, misspktl);
	    return;
	}

	awl = DHD_AWL_CB(dhdp);

	skb = misspktl->head;
	npkts = misspktl->len;
	PKTLIST_RESET(misspktl);

	awl->rx.a2w_chn_packets = npkts;
	if (awl->rx.a2w_chn_packets > awl->rx.a2w_max_chn)
	    awl->rx.a2w_max_chn = awl->rx.a2w_chn_packets;

	while (npkts--) {
	    struct sk_buff* nskb = skb->prev;
	    uint32_t hw_port;

	    skb->next = skb->prev = NULL;

	    awl->rx.a2w_rx_packets++;

	    /* Copied essential parts from dhd_rx_frame() */
	    /* This code executes under Archer context,
	       To move this to DHD, need to put this in a queue and wakeup DHD thread
	       and let DHD thread process it
	     */
	    hw_port = netdev_path_get_hw_port((struct net_device *)(skb->dev));
	    blog_action = blog_sinit(skb, skb->dev, TYPE_ETH, hw_port, BLOG_WLANPHY);

	    if (PKT_DONE == blog_action) {
	        awl->rx.a2w_flt_packets++;
	        return;
	    }

	    eth = skb->data;
	    len = skb->len;
	    skb->protocol = eth_type_trans(skb, skb->dev);
	    skb->data = eth;
	    skb->len = len;

	    /* Strip header, count, deliver upward */
	    skb_pull(skb, ETH_HLEN);

	    netif_receive_skb(skb);

	    /* update stats */
	    awl->rx.a2w_fwd_packets++;
	    awl->rx.a2w_fwd_calls++;

	    skb = nskb;
	}

	return;
}

void
dhd_awl_rx_flow_miss_handler_archer_skb(void *ctxt, struct sk_buff *skb)
{
	dhd_pub_t *dhdp = (dhd_pub_t*)ctxt;
	pktlist_t misspktl;

	if ((dhdp == NULL) || (skb == NULL)) {
	    DHD_AWL_ERROR("%s (0x%px, 0x%px) with NULL parameters\n",
	        __FUNCTION__, dhdp, skb);
	    return;
	}

	PKTLIST_RESET(&misspktl);
	dhd_awl_pktlist_add(&misspktl, skb);

	return dhd_awl_rx_flow_miss_handler_archer_sll(ctxt, &misspktl);
}

/**
 * -----------------------------------------------------------------------------
 * Function : Callback function from Archer CPU for flow-miss
 *
 *   completely handled with-in Archer context
 *   might load Archer CPU with dhd processing
 *   good for Archer offload and general testing purpose
 * -----------------------------------------------------------------------------
 */
void
dhd_awl_rx_flow_miss_handler_archer_dhd_sll(void *ctxt, pktlist_t *misspktl)
{
	dhd_pub_t *dhdp = (dhd_pub_t*)ctxt;
	dhd_awl_t *awl;
	struct sk_buff *skb;
	int npkts;

	/* Get the DHD public control block */
	if ((dhdp == NULL) || (misspktl == NULL)) {
	    DHD_AWL_ERROR("%s (0x%px, 0x%px) with NULL parameters\n",
	        __FUNCTION__, dhdp, misspktl);
	    return;
	}

	awl = DHD_AWL_CB(dhdp);

	/* De-link packets from the SLL */
	skb = misspktl->head;
	npkts = misspktl->len;
	PKTLIST_RESET(misspktl);

	/* update chain packet status */
	awl->rx.a2w_chn_packets = npkts;
	if (awl->rx.a2w_chn_packets > awl->rx.a2w_max_chn)
	    awl->rx.a2w_max_chn = awl->rx.a2w_chn_packets;

	/* Process each packet */
	while (npkts--) {
	    struct sk_buff* nskb = skb->prev;

	    skb->next = skb->prev = NULL;

	    awl->rx.a2w_rx_packets++;

	    /* Call DHD Rx handler */
	    DHD_LOCK(dhdp);

	    dhd_bus_rx_frame(dhdp->bus, skb, ARCHER_WLAN_INTF_IDX(skb), 1);

	    DHD_UNLOCK(dhdp);

	    /* Update stats */
	    awl = DHD_AWL_CB(dhdp);
	    awl->rx.a2w_rx_packets++;
	    awl->rx.a2w_fwd_packets++;
	    awl->rx.a2w_fwd_calls++;

	    skb = nskb;
	}

	return;
}

void
dhd_awl_rx_flow_miss_handler_archer_dhd_skb(void *ctxt, struct sk_buff *skb)
{
	dhd_pub_t *dhdp = (dhd_pub_t*)ctxt;
	pktlist_t misspktl;

	if ((dhdp == NULL) || (skb == NULL)) {
	    DHD_AWL_ERROR("%s (0x%px, 0x%px) with NULL parameters\n",
	        __FUNCTION__, dhdp, skb);
	    return;
	}

	PKTLIST_RESET(&misspktl);
	dhd_awl_pktlist_add(&misspktl, skb);

	return dhd_awl_rx_flow_miss_handler_archer_dhd_sll(ctxt, &misspktl);
}

/**
 * -----------------------------------------------------------------------------
 * Function : Callback function from Archer CPU for flow-miss
 *
 *   adds packets to SLL and invokes DHD dpc
 *   minimum processing in Archer context
 * -----------------------------------------------------------------------------
 */
void
dhd_awl_rx_flow_miss_handler_dhd_dpc_sll(void *ctxt, pktlist_t *misspktl)
{
	dhd_pub_t *dhdp = (dhd_pub_t*)ctxt;
	dhd_awl_t *awl;
	pktlist_t *pktlist;
	int npkts;

	/* Get the DHD public control block */
	if ((dhdp == NULL) || (misspktl == NULL)) {
	    DHD_AWL_ERROR("%s (0x%px, 0x%px) with NULL parameters\n",
	        __FUNCTION__, dhdp, misspktl);
	    return;
	}

	if ((npkts = misspktl->len) == 0) {
	    /* No patckets in the list */
	    return;
	}

	/* Add to the slow path SLL with a lock, This is happening from Archer context */
	awl = DHD_AWL_CB(dhdp);
	pktlist = DHD_AWL_RX_A2W_PKTL(awl);

	if (awl->rx.a2w_max_pktl_len &&
	    (pktlist->len + misspktl->len) > awl->rx.a2w_max_pktl_len) {
	    pktlist_pkt_t * pkt;

	    awl->rx.a2w_discard_packets += npkts;

	    while (npkts--) {
	        pkt = misspktl->head;
	        misspktl->head = PKTLIST_PKT_SLL(pkt, SKBUFF_PTR);

	        /* No osh accounting, as not yet in DHD */
	        PKTLIST_PKT_FREE(pkt);
	    }

	    PKTLIST_RESET(misspktl); /* head,tail, not reset */

	    return;
	}

	DHD_AWL_PKTLIST_LOCK(awl->rx.a2w_pktl_lock);

	/* Question: SLL with skb->next i.o skb->prev ? as dhd_linux uses skb->next ? */
	if (likely(pktlist->len == 0)) {
	    pktlist->head = misspktl->head;
	} else {
	    /* Append to tail */
	    PKTLIST_PKT_SET_SLL(pktlist->tail, misspktl->head, SKBUFF_PTR);
	}
	pktlist->tail = misspktl->tail;
	pktlist->len += npkts;

	DHD_AWL_PKTLIST_UNLK(awl->rx.a2w_pktl_lock);

	PKTLIST_RESET(misspktl);

	awl->rx.a2w_rx_packets += npkts;
	awl->rx.a2w_chn_packets = npkts;
	if (awl->rx.a2w_chn_packets > awl->rx.a2w_max_chn)
	    awl->rx.a2w_max_chn = awl->rx.a2w_chn_packets;

	/* Wakeup DPC thread or run DPC tasklet */
	dhd_sched_dpc(dhdp);

	return;
}

void
dhd_awl_rx_flow_miss_handler_dhd_dpc_skb(void *ctxt, struct sk_buff *skb)
{
	dhd_pub_t *dhdp = (dhd_pub_t*)ctxt;
	pktlist_t misspktl;

	/* Get the WL public control block */
	if ((dhdp == NULL) || (skb == NULL)) {
	    DHD_AWL_ERROR("%s (0x%px, 0x%px) with NULL parameters\n",
	        __FUNCTION__, dhdp, skb);
	    return;
	}

	PKTLIST_RESET(&misspktl);
	dhd_awl_pktlist_add(&misspktl, skb);

	return dhd_awl_rx_flow_miss_handler_dhd_dpc_sll(ctxt, &misspktl);
}

/**
 * =============================================================================
 * Section: DHD_AWL External function definitions
 * =============================================================================
 */
/**
 * -----------------------------------------------------------------------------
 * DHD_AWL: RX path External Interface functions
 * -----------------------------------------------------------------------------
 */

/**
 * -----------------------------------------------------------------------------
 * Function : Function to process slow path upstream packets from Archer flow-miss
 *
 *   Called from bus layer in the context of DHD DPC
 *   No perim locks in this function as caller already take care of locking
 *   Retrieve packets from SLL and call dhd_linux rx packet handler
 * -----------------------------------------------------------------------------
 */
bool
dhd_awl_process_slowpath_rxpkts(dhd_pub_t *dhdp, int rxbound)
{
	bool more = FALSE;
	dhd_awl_t *awl;
	pktlist_t *pktlist;
	int npkts, ppkts = 0;
	struct sk_buff *skb = NULL;

	awl = DHD_AWL_CB(dhdp);

	/* Nothing to do if mode is not DHD DPC */
	if (awl->rx.mode != DHD_AWL_RX_MODE_FULL)
	    return more;

	/* Get the pending packets list */
	pktlist = DHD_AWL_RX_A2W_PKTL(awl);

	DHD_AWL_PKTLIST_LOCK(awl->rx.a2w_pktl_lock);

	npkts = pktlist->len;
	if (npkts == 0) {
	    DHD_AWL_PKTLIST_UNLK(awl->rx.a2w_pktl_lock);
	    return more;
	}

	skb = (struct sk_buff*)pktlist->head;
	if (npkts > rxbound) {
	    struct sk_buff* nskb = skb;
	    ppkts = npkts = rxbound;

	    /* Unlink first npkts from the SLL */
	    do {
	        nskb = nskb->prev;
	    } while (--ppkts);

	    pktlist->head = nskb;
	    pktlist->len -= npkts;
	} else {
	    PKTLIST_RESET(pktlist);
	}

	DHD_AWL_PKTLIST_UNLK(awl->rx.a2w_pktl_lock);

	/* Let dhd_bus process the packets */
	DHD_LOCK(dhdp);

	do {
	    struct sk_buff* nskb = skb->prev;
	    skb->next = skb->prev = NULL;
	    PKTFRMNATIVE(dhdp->osh, skb);
	    dhd_bus_rx_frame(dhdp->bus, skb, ARCHER_WLAN_INTF_IDX(skb), 1);
	    awl->rx.a2w_fwd_packets++;
	    awl->rx.a2w_fwd_calls++;
	    skb = nskb;
	} while (--npkts);

	DHD_UNLOCK(dhdp);

	if (pktlist->len != 0) {
	    more = true;
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
int
dhd_awl_upstream_send(dhd_pub_t *dhdp)
{
	dhd_awl_t *awl = DHD_AWL_CB(dhdp);
	pktlist_t *pktlist = DHD_AWL_RX_W2A_PKTL(awl);

	DHD_AWL_PTRACE("dhd%d_awl: pktlist 0x%px", dhdp->unit, pktlist);

	return dhd_awl_upstream(awl, pktlist);
}

/**
 * -----------------------------------------------------------------------------
 * Function : Function to add Rx packet to dhd_awl SLL
 *
 *  Called from from protocol layer for single skb.
 *  Caller must call dhd_awl_upstream_send() to finally push the list Archer
 * -----------------------------------------------------------------------------
 */
int
dhd_awl_upstream_add(dhd_pub_t *dhdp, void *pkt, uint ifidx)
{
	dhd_awl_t *awl = DHD_AWL_CB(dhdp);
	pktlist_t *pktlist = DHD_AWL_RX_W2A_PKTL(awl);
	struct sk_buff * skb;

	DHD_AWL_PTRACE("dhd%d_awl skb 0x%px\n", dhdp->unit, pkt);

	awl->rx.w2a_rx_packets++;

	/* pass through mode or flow cache disabled, process within wlan thread */
	if ((awl->rx.mode == DHD_AWL_RX_MODE_PT) || fcacheStatus() == 0) {
	    dhd_bus_rx_frame(dhdp->bus, pkt, ifidx, 1);
	    awl->rx.w2a_flt_packets++;
	    return BCME_OK;
	}

	if (awl->rx.mode == DHD_AWL_RX_MODE_LITE) {
	    struct ether_header *eh;
	    uint16 eth_type;
	    /*
	     * Filter out must pass packets to DHD without archer
	     * This might miss the following packets to go through archer without DHD
	     * net device reg_state check
	     * DHD_WMF, DHD_WET, BCM_SPDSVC, loopback
	     */
	    if (!IS_SKBUFF_PTR(pkt)) {
	        dhd_bus_rx_frame(dhdp->bus, pkt, ifidx, 1);
	        awl->rx.w2a_flt_packets++;
	        return BCME_OK;
	    }

	    eh = (struct ether_header *)PKTDATA(dhdp->osh, pkt);
	    eth_type = ntoh16(eh->ether_type);

	    if ((eth_type == ETHER_TYPE_BRCM) || (eth_type == ETHER_TYPE_BRCM_AIRIQ)) {
	        dhd_bus_rx_frame(dhdp->bus, pkt, ifidx, 1);
	        awl->rx.w2a_flt_packets++;
	        return BCME_OK;
	    }
	}

	/* Prepare SKB for forwarding to Archer */
	skb = PKTTONATIVE(dhdp->osh, pkt);
	skb->dev = dhd_idx2net(dhdp, ifidx);

	if (skb->prev) {
	    printk("0x%px <- 0x%px ->  0x%px\n", skb->prev, skb, skb->next);
	    skb->prev = NULL;
	}

	ARCHER_WLAN_RADIO_IDX(skb) = dhdp->unit;
	ARCHER_WLAN_INTF_IDX(skb) = ifidx;

	if (likely(pktlist->len != 0)) {
	    /* pend to tail */
	    PKTLIST_PKT_SET_SLL(pktlist->tail, skb, SKBUFF_PTR);
	    pktlist->tail = skb;
	} else {
	    pktlist->head = pktlist->tail = skb;
	}
	++pktlist->len;

	/* Check packet bounds and send to Archer */
	if (pktlist->len >= awl->rx.bound) {
	    dhd_awl_upstream(awl, pktlist);
	}

	return BCME_OK;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Function to transfer archer fast path packets to wlan
 *
 *  Called from archer upstream task to wake up dpc task to process the fast path
 *  down steam packets
 *
 * -----------------------------------------------------------------------------
 */
void
dhd_awl_xfer_callback(pktlist_context_t * awl_pktlist_context)
{
	dhd_awl_t *awl;
	dhd_pub_t *dhdp;

	ASSERT(awl_pktlist_context);

	DHD_AWL_PTRACE("dhd%d_awl xfer_callback\n", awl_pktlist_context->unit);

	dhdp = g_dhd_info[awl_pktlist_context->unit];

	ASSERT(dhdp);

	awl = DHD_AWL_CB(dhdp);

	dhd_sched_dpc(dhdp);
	
	awl->tx.a2w_rx_calls++;

	return;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Function process fast path downstream packets in dhd dpc context
 * -----------------------------------------------------------------------------
 */
bool
dhd_awl_process_fastpath_txpkts(dhd_pub_t *dhdp, int txbound)
{
	dhd_awl_t         *awl = DHD_AWL_CB(dhdp);
	int               prio;
	bool               more = false;
	pktlist_context_t *awl_pktlist_context = awl->tx.awl_pktlist_context;
	pktlist_context_t *dhd_pktlist_context = awl_pktlist_context->peer;
	int               work_items = 0;

	if (awl_pktlist_context == PKTLIST_CONTEXT_NULL) {
	    /* Not supported */
	    return more;
	}

	/* Grab the awl pktlist_context, maybe a different thread/cpu */
	PKTLIST_LOCK(awl_pktlist_context);

	if (!dll_empty(&awl_pktlist_context->mcast)) {
	    /* Dispatch active mcast pktlists from awl to dhd - not by priority */
	    __pktlist_xfer_work(awl_pktlist_context, dhd_pktlist_context,
	                        &awl_pktlist_context->mcast,
	                        &dhd_pktlist_context->mcast, "MCAST", FKBUFF_PTR);
	    ++work_items;
	}

	/* Dispatch active ucast pktlists from awl to dhd - by priority */
	for(prio = 0; prio < PKTLIST_PRIO_MAX; ++prio)
	{
	    if (!dll_empty(&awl_pktlist_context->ucast[prio])) {
	        /* Process non empty ucast[] worklists in archer pktlist context */
	        __pktlist_xfer_work(awl_pktlist_context, dhd_pktlist_context,
	                            &awl_pktlist_context->ucast[prio],
	                            &dhd_pktlist_context->ucast[prio],
	                            "UCAST", FKBUFF_PTR);
	        ++work_items;
	    }
	}

	/* Release peer's pktlist context */
	PKTLIST_UNLK(awl_pktlist_context);

	if (work_items) {

	    (awl_pktlist_context->xfer_fn)(awl_pktlist_context->peer);

	    awl->tx.a2w_fwd_calls ++;
	    awl->tx.a2w_fwd_items += work_items;
	    awl->tx.a2w_chn_items = work_items;
	    if (awl->tx.a2w_chn_items > awl->tx.a2w_max_chn)
	        awl->tx.a2w_max_chn = awl->tx.a2w_chn_items;
	}

	return more;
}

/**
 * -----------------------------------------------------------------------------
 * Function : bind radio packet list to archer based on tx mode
 * -----------------------------------------------------------------------------
 */
int
dhd_awl_wfd_bind(struct net_device *net,
	struct pktlist_context *dhd_pktlist_context,
	archer_wlan_radio_mode_t mode,
	HOOK32 wl_completeHook,
	int unit)
{
	dhd_awl_t   *awl = &dhd_awl_cb_g[unit];
	int         wfd_idx = -1;
	struct pktlist_context *awl_pktlist_context;

	if (awl->tx.mode == DHD_AWL_TX_MODE_DPC) {
	    /* Create intemeidate packet list to transfer to a different cpu */
	    /*
	     * ========== ARCHER======|=========DHD DPC==========
	     * archer_ds => (LOCK) [A W L] (LOCK) <= DHD
	     */
	    /* Instantiate AWL consumer pktlist_context */
	    ASSERT(awl->tx.awl_pktlist_context == PKTLIST_CONTEXT_NULL);
	    awl_pktlist_context = pktlist_context_init(
	        dhd_pktlist_context, (pktlist_context_xfer_fn_t)wl_completeHook,
	        dhd_pktlist_context->keymap_fn, dhd_pktlist_context->driver,
	        dhd_pktlist_context->driver_name, unit);

	    if (awl_pktlist_context == PKTLIST_CONTEXT_NULL)
	    {
	        DHD_AWL_ERROR("pktlist_context unit %d failure", unit);
	        return wfd_idx;
	    }
	    DHD_AWL_LOG("%s Created Tx AWL pktlist 0x%px\n", __FUNCTION__, awl_pktlist_context);

	    awl->tx.awl_pktlist_context = awl_pktlist_context;

	    /* Register with Archer */
	    wfd_idx = archer_wlan_bind(net, awl_pktlist_context, mode,
	                              (HOOK32)dhd_awl_xfer_callback, unit);
	} else {
	    /* Register with Archer */
	    wfd_idx = archer_wlan_bind(net, dhd_pktlist_context, mode, wl_completeHook, unit);
	}

	return wfd_idx;
}

/**
 * -----------------------------------------------------------------------------
 * Function : unbind radio packet list from archer
 * -----------------------------------------------------------------------------
 */
int dhd_awl_wfd_unbind(int unit)
{
	dhd_awl_t   *awl = &dhd_awl_cb_g[unit];

	/* free up packets in the intermediate list */
	if (awl->tx.awl_pktlist_context) {
	    pktlist_context_fini(awl->tx.awl_pktlist_context);
	}

	return archer_wlan_unbind(unit);
}

/**
 * -----------------------------------------------------------------------------
 * Function : Helper debug dump the AWL rx statistics
 * -----------------------------------------------------------------------------
 */
void
dhd_awl_dump(dhd_pub_t *dhdp, struct bcmstrbuf *b)
{
	dhd_awl_t *awl = DHD_AWL_CB(dhdp);

	DHD_AWL_PTRACE("dhd%d_awl:\n", dhdp->unit);

	/* Dump Tx statistics */
	bcm_bprintf(b, " DHD_AWL Tx: %4s bound %d",
	    dhd_awl_tx_mode_str_g[awl->tx.mode], -1);
	bcm_bprintf(b, " A2W Xfer [rxd %lu drp %lu fwd %lu cls %lu chn %lu/%lu]\n",
	    awl->tx.a2w_rx_calls, awl->tx.a2w_flt_calls, awl->tx.a2w_fwd_items,
	    awl->tx.a2w_fwd_calls, awl->tx.a2w_chn_items, awl->tx.a2w_max_chn);

	/* Dump Rx statistics */
	bcm_bprintf(b, " DHD_AWL Rx: %4s bound %d a2w_max_pktl_len %d\n",
	    dhd_awl_rx_mode_str_g[awl->rx.mode], awl->rx.bound, awl->rx.a2w_max_pktl_len);
	bcm_bprintf(b, " W2A List [rxd %lu flt %lu fwd %lu cls %lu chn %lu/%lu]",
	    awl->rx.w2a_rx_packets, awl->rx.w2a_flt_packets,
	    awl->rx.w2a_fwd_packets, awl->rx.w2a_fwd_calls,
	    awl->rx.w2a_chn_packets, awl->rx.w2a_max_chn);
	bcm_bprintf(b, " A2W List [rxd %lu drp %lu fwd %lu cls %lu chn %lu/%lu discard %lu]\n",
	    awl->rx.a2w_rx_packets, awl->rx.a2w_flt_packets,
	    awl->rx.a2w_fwd_packets, awl->rx.a2w_fwd_calls,
	    awl->rx.a2w_chn_packets, awl->rx.a2w_max_chn,
	    awl->rx.a2w_discard_packets);
}

/**
 * -----------------------------------------------------------------------------
 * Function : DHD AWL initialization function
 * -----------------------------------------------------------------------------
 */
void*
dhd_awl_attach(dhd_pub_t *dhdp, uint unit)
{
	dhd_awl_t *awl = DHD_AWL_CB(dhdp);
	archer_wlan_rx_miss_handler_t a2w_flow_miss_handler = dhd_awl_rx_flow_miss_handler_dhd_dpc;
	int rxmode = DHD_AWL_RX_MODE_PT;
	int max_pktl_len = DHD_AWL_A2W_MAX_PKTL_LEN;

	DHD_AWL_PTRACE("dhd%d_awl: dhdp 0x%px", unit, dhdp);

	memset(awl, 0, sizeof(dhd_awl_t));

#if defined(DHD_AWL_TX)
	awl->tx.mode = DHD_AWL_TX_MODE_ON;
	awl->tx.awl_pktlist_context = PKTLIST_CONTEXT_NULL;

	{
	    char varname[] = "wlXX_awl_txmode";
	    char *var;

	    snprintf(varname, sizeof(varname), "wl%d_awl_txmode", dhd_get_instance(dhdp));
	    var = getvar(NULL, varname);
	    if (var) {
	        awl->tx.mode =  bcm_strtoul(var, NULL, 0);
	        if (awl->tx.mode > DHD_AWL_TX_MODE_LAST) {
	            awl->tx.mode = DHD_AWL_TX_MODE_ON;
	        }
	    }
	}
#endif /* DHD_AWL_TX */

	if (awl->tx.mode >= DHD_AWL_TX_MODE_DPC) {
	    char *var;
	    /* Assuming DHD has not already initialized the dpc thread/tasklet */
	    var = getvar(NULL, "dhd_dpc_prio");
	    if (var) {
	        dhd_dpc_prio =  bcm_strtoul(var, NULL, 0);
	    } else if (dhd_dpc_prio < 0) {
	        dhd_dpc_prio = 5;
	    }
	}

#if defined(BCM_DHD_RUNNER)
	if (DHD_RNR_OFFL_RXCMPL(dhdp)) {
	    /* When DoA is enabled, AWL is pass through */
	    rxmode = DHD_AWL_RX_MODE_PT;
	}
#endif /* BCM_DHD_RUNNER */

#if defined(DHD_AWL_RX)
	{
	    char varname[32];
	    char *var;
	    rxmode = DHD_AWL_RX_MODE_FULL;

	    snprintf(varname, sizeof(varname), "wl%d_awl_rxmode", dhd_get_instance(dhdp));
	    var = getvar(NULL, varname);
	    if (var) {
	        rxmode =  bcm_strtoul(var, NULL, 0);
	    }

	    snprintf(varname, sizeof(varname), "wl%d_awl_a2w_max_pktl_len", unit);
	    var = getvar(NULL, varname);
	    if (var) {
	        max_pktl_len =  bcm_strtoul(var, NULL, 0);
	    }
	}
#endif /* DHD_AWL_RX */

	if (rxmode == DHD_AWL_RX_MODE_LITE) {
	    a2w_flow_miss_handler = dhd_awl_rx_flow_miss_handler_archer;
	}
	awl->rx.mode = rxmode;

	awl->rx.bound = DHD_AWL_RX_PKT_BOUND;

	awl->rx.a2w_max_pktl_len = max_pktl_len;

	awl->rx.w2a_pktl.head = PKTLIST_PKT_NULL;
	awl->rx.w2a_pktl.tail = PKTLIST_PKT_NULL;
	awl->rx.w2a_pktl.len = 0;

	awl->rx.a2w_pktl.head = PKTLIST_PKT_NULL;
	awl->rx.a2w_pktl.tail = PKTLIST_PKT_NULL;
	awl->rx.a2w_pktl.len = 0;

	spin_lock_init(&awl->rx.a2w_pktl_lock);

	/* Register with Archer call back functions for Rx */
	if (archer_wlan_rx_register(unit, a2w_flow_miss_handler, dhdp) != 0) {
	    DHD_AWL_ERROR("archer_wlan_rx_register failed\n");
	    return NULL;
	}

	DHD_AWL_LOG("dhd%d_awl attach successful tx: %d, rx: %d\n",
	    dhdp->unit, awl->tx.mode, awl->rx.mode);

	return (void*)awl;
}

/**
 * -----------------------------------------------------------------------------
 * Function : DHD AWL de-initialization function
 *
 * -----------------------------------------------------------------------------
 */
void
dhd_awl_detach(dhd_pub_t *dhdp, void *awl)
{
	DHD_AWL_PTRACE("dhd%d_awl: dhdp 0x%px, awl 0x%px", dhdp->unit, dhdp, awl);

	/* De-register with Archer call back functions for Rx */
	archer_wlan_rx_register(dhdp->unit, NULL, NULL);

	DHD_AWL_LOG("dhd%d_awl detach complete", dhdp->unit);

	return;
}
