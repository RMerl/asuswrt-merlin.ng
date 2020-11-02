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


/**
 * =============================================================================
 *
 * WLAN Packet Forwarding Datapath from Bridged Traffic Acceleration with
 * WLAN Driver's Cached Flow Processing (CFP) binding.
 *
 * Incorporates 4 primary functionality:
 *   1. Management of 802.3 Mac Addresses (D3 LUT)
 *      Serves as a Cache of native OS Bridge layer (e.g. Linux Bridge FDB)
 *   2. Binning packets based on a 802.3 Mac Address using <index,incarn>
 *      D3 LUT pairs a 802.3 MacAddr to a 16bit <incarn,index>. Index may be
 *      a CFP flowId. Receive packets are binned based on a D3 LUT "index"
 *      using the pktlist construct.
 *   3. Linking packets in single linked list(sll), with native OS packet field.
 *   4. Transfer of accumulated packet lists from one subsystem to another,
 *      allowing packet lists (trains) to grow by sll appends. Packet trains
 *      are carried from ingres (LAN WFD) right through WLAN driver, growing
 *      and released in smaller or larger trains.
 *
 * WLAN PKTFWD replaces the PKTC Table lookup, the chain_id and chain_node based
 * linkage and dispatching of threads. chain_node based transfer of packets into
 * WLAN may co-exist.
 *
 * =============================================================================
 */

/**
 * =============================================================================
 *
 * Reference TODO :
 *
 *   1. Need to support DHD without WFD(Runner)
 *        PKTFWD library needs to be placed in a shared module, accessible by
 *      dhd.ko and wl.ko. DHD only needs to lookup LAN endpoints.
 *
 *   2. Need to direct WLAN to LAN upstream packets directly to WFD, to avoid
 *      the default Enet driver, being loaded by multiple radios. Impact to
 *      Enet TxRing depth, bandwidth on Enet system port, etc.
 *
 *   3. Need to collapse the dual thread (WFD and WLAN) by having WLAN thread
 *      directly invoking rdpa packet get APIs. This removes the need to first
 *      bin in a WFD pktlist_context, and then handoff to the bins in the WLAN
 *      packet_context, before transferring them to the WLAN interfaces work
 *      lists. Each WFD driver is colored to be WLAN DHD or NIC.
 *      This may impose the need for deeper WFD RxRings. Having a separate WFD
 *      thread allows for multi-CPU-core load balancing. E.g. WFD thread hosting
 *      WFD RxRing packet binning along with a 3rdparty networking middleware
 *      running on one CPU core, and a transfer to a WLAN thread for NIC mode.
 *      
 *   4. Transfer of pktlists to WLAN NIC driver as "chain_node" based lists when
 *      CacheFlowProcessing is not eligible or not present.
 *
 *   5. Cleanup all PKTC entry points, retaining a single wl_pktfwd_request.
 *      May retain the wl_pktfwd_match() which is invoked on WLAN receive path
 *      on a per packet basis for Receive is chainable.
 *        - wl_pktc_del_hook() registered with Blog SHIM
 *        - fdb_check_expired_wl_hook registered with Linux Bridge
 *
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
#include <wlc_scb.h>
#if defined(BCM_BLOG)
#include <wl_blog.h>
#endif /* BCM_BLOG */

#if !defined(WL_EAP_AP)
#if !defined(WLCFP)
#error "PKTFWD requires WLCFP"
#else /* WLCFP */
#if (PKTFWD_ENDPOINTS_MAX != CFP_FLOWID_SCB_TOTAL)
#error "Maximum PKTFWD ENDPOINTS and CFP Flows mismatch"
#endif
#endif /* WLCFP */
#endif /*WL_EAP_AP*/

/* Toggle pktfwd acceleration in dwds via nvram vars */
extern char * nvram_get(const char *name);
#define NVRAM_DWDS_AP_PKTFWD_ACCEL     "wl_dwds_ap_pktfwd"  /* 0: disable, 1: enable */
#define NVRAM_DWDS_STA_PKTFWD_ACCEL    "wl_dwds_sta_pktfwd"  /* 0: disable, 1: enable */
bool wl_dwds_ap_pktfwd_accel = false; /* default dwds ap pktfwd accel */
bool wl_dwds_sta_pktfwd_accel = false; /* default dwds sta pktfwd accel */

/**
 * =============================================================================
 * Section: PKTFWD Global System Object(s)
 * =============================================================================
 */

#if defined(WL_PKTFWD_RUNQ)
/** Ucast work lists are moved to a run queue for servicing. */
typedef struct wl_pktfwd_runq
{
    d3fwd_wlif_t      * d3fwd_wlif;     /* Current d3fwd_wlif in run queue */
    int                 credits;        /* number of pktlists to xmit */
    uint32_t            ucast_bmap;     /* bmap of pending ucast work */
    dll_t               ucast[D3FWD_PRIO_MAX]; /* ucast pktlist work by prio */
} wl_pktfwd_runq_t;
#endif  /* WL_PKTFWD_RUNQ */

/** Per Radio state */
typedef struct wl_pktfwd_radio
{
    pktlist_context_t * pktlist_context; /* WLAN radio downstream */
    pktqueue_table_t  * pktqueue_table;  /* WLAN radio upstream */
#if defined(WL_PKTFWD_TXEVAL)
    atomic_t            dispatch;
#endif  /* WL_PKTFWD_TXEVAL */
#if defined(WL_PKTFWD_RUNQ)
    wl_pktfwd_runq_t    runq;            /* WLAN radio run queue */
#endif  /* WL_PKTFWD_RUNQ */
    int                 wfd_idx;
} wl_pktfwd_radio_t;


/**
 * -----------------------------------------------------------------------------
 *
 * Singleton global object.
 *
 * - lock            : Global system lock
 * - stats           : Global system statistics
 * - d3lut           : Lookup table for 802.3 MacAddresses for LAN and WLAN.
 *
 * + pktlist_context : Per WLAN radio, consumer pktlist_context.
 * + pktqueue_table  : Per WLAN radio, pktqueue for upstream
 * + dispatch        : WLAN radio, flag to prevent dispatch re-entrancy
 * + runq            : Per WLAN radio, run queue of pktlists
 * + wfd_idx         : Per WLAN radio, wfd unit
 *
 * - d3fwd_used      : List of in-use d3fwd_wlif objects in a dbl linked list
 * - d3fwd_free      : List of free d3fwd_wlif objects in a dbl linked list
 * - d3fwd_wlif      : Preallocated pool of d3fwd_wlif objects
 * - radio_cnt       : Count of registered WLAN radios <wl_info>
 * - wlif_cnt        : Count of registered WLAN Interfaces <wl_if,net_device>
 * - tx_accel        : Runtime WFD to WLAN Transmit acceleration state
 * - initialized     : Flag for initialization state of singleton global
 * -----------------------------------------------------------------------------
 */

typedef struct wl_pktfwd                /* Global System State */
{
    spinlock_t          lock;           /* system lock */

    wl_pktfwd_stats_t   stats;          /* system wide statistics */

    d3lut_t           * d3lut;          /* 802.3 MAC Address LUT */

    wl_pktfwd_radio_t   radio[WL_PKTFWD_RADIOS]; /* Per radio state */

    dll_t               d3fwd_used;     /* list of in-use d3fwd_wlif */
    dll_t               d3fwd_free;     /* list of free d3fwd_wlif */
    d3fwd_wlif_t      * d3fwd_wlif;     /* prealloc d3fwd_wlif pool */

    int8_t              radio_cnt;      /* count of radio(s) attached */
    int8_t              wlif_cnt;       /* count of wl_if(s) attached */
    uint8_t             tx_accel;       /* wlan tx acceleration */
    bool                initialized;    /* global system initialized */
} wl_pktfwd_t;

/** Static initialization of singleton system global object */
wl_pktfwd_t wl_pktfwd_g =
{
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
    .lock               = __SPIN_LOCK_UNLOCKED(wl_pktfwd_g.lock),
#endif
    .stats              = { },
    .d3lut              = D3LUT_NULL,
    .radio              = /* WL_PKTFWD_RADIOS = 3 */
    {
        {
            .pktlist_context = PKTLIST_CONTEXT_NULL,
            .pktqueue_table  = PKTQUEUE_TABLE_NULL,
#if defined(WL_PKTFWD_RUNQ)
            .runq            = {},
#endif
            .wfd_idx         = -1
        },
        {
            .pktlist_context = PKTLIST_CONTEXT_NULL,
            .pktqueue_table  = PKTQUEUE_TABLE_NULL,
#if defined(WL_PKTFWD_RUNQ)
            .runq            = {},
#endif
            .wfd_idx         = -1
        },
        {
            .pktlist_context = PKTLIST_CONTEXT_NULL,
            .pktqueue_table  = PKTQUEUE_TABLE_NULL,
#if defined(WL_PKTFWD_RUNQ)
            .runq            = {},
#endif
            .wfd_idx         = -1
        }
    },
    .d3fwd_used         = DLL_STRUCT_INITIALIZER(wl_pktfwd_g, d3fwd_used),
    .d3fwd_free         = DLL_STRUCT_INITIALIZER(wl_pktfwd_g, d3fwd_free),
    .d3fwd_wlif         = D3FWD_WLIF_NULL,
    .radio_cnt          = 0,
    .wlif_cnt           = 0,
#if defined(BCM_WFD)
    .tx_accel           = ~0,
#else
    .tx_accel           = 0,
#endif
    .initialized        = false
};


/** Use following macros when accessing wl_pktfwd_g per radio state */
#define WL_PKTLIST_CONTEXT(unit)  wl_pktfwd_g.radio[unit].pktlist_context
#define WL_PKTQUEUE_TABLE(unit)   wl_pktfwd_g.radio[unit].pktqueue_table
#define WL_DISPATCH_P(unit)       &(wl_pktfwd_g.radio[unit].dispatch) /* addr */
#define WL_RUNQ_P(unit)           &(wl_pktfwd_g.radio[unit].runq)     /* addr */
#define WL_WFD_IDX(unit)          wl_pktfwd_g.radio[unit].wfd_idx


#if defined(WL_PKTFWD_TXEVAL)
/* WL_DISPATCH_P() returns address of atomic_t */
#define WL_DISPATCH_INIT(unit)      atomic_set(WL_DISPATCH_P(unit), 0)
#define WL_DISPATCH_CHECK(unit)     atomic_read(WL_DISPATCH_P(unit))
#define WL_DISPATCH_ENTER(unit)     atomic_set(WL_DISPATCH_P(unit), 1)
#define WL_DISPATCH_EXIT(unit)      atomic_set(WL_DISPATCH_P(unit), 0)
#else  /* ! WL_PKTFWD_TXEVAL */
#define WL_DISPATCH_INIT(unit)      do { /* noop */ } while (0)
#define WL_DISPATCH_CHECK(unit)     false
#define WL_DISPATCH_ENTER(unit)     do { /* noop */ } while (0)
#define WL_DISPATCH_EXIT(unit)      do { /* noop */ } while (0)
#endif /* ! WL_PKTFWD_TXEVAL */


/** System global lock macros mutual exclusive access */
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define PKTFWD_LOCK()       spin_lock_bh(&wl_pktfwd_g.lock)
#define PKTFWD_UNLK()       spin_unlock_bh(&wl_pktfwd_g.lock)
#else   /* ! (CONFIG_SMP || CONFIG_PREEMPT) */
#define PKTFWD_LOCK()       local_irq_disable()
#define PKTFWD_UNLK()       local_irq_enable()
#endif  /* ! (CONFIG_SMP || CONFIG_PREEMPT) */

/**
 * HI to LO priority dispatching of pktlists maintained in ucast[] work lists.
 *
 * Rotate: prio 0 is b7, prio 1 is b6, ... prio 7 is b0
 * __builtin_ffs() along with PKTFWD_UCAST_BMAP_FFS_TO_PRIO() will provide the
 * highest prio value first in the bitmap.
 *
 * Impementation Caveat: verified for 8 prio.
 */
#if (PKTLIST_PRIO_MAX != 8)
#error "PKTFWD_UCAST_BMAP 8bit caveat"
#endif

#define PKTFWD_UCAST_BMAP(prio) \
    (1 << ((PKTLIST_PRIO_MAX - 1) - (prio)))

#define PKTFWD_UCAST_BMAP_FFS_TO_PRIO(ffs_ucast_bmap) \
    (PKTLIST_PRIO_MAX - 1) - ((ffs_ucast_bmap) - 1)


/** Global stats for updates to "slow" path counters */
wl_pktfwd_stats_t * wl_pktfwd_stats_gp = &wl_pktfwd_g.stats; /* extern */

const char * wl_pktfwd_req_str[wl_pktfwd_req_max_e] =
    { "UNDEF", "TXSET", "TXGET", "D3INS", "FLUSH", "ASSOC", "PLIST" };


/**
 * =============================================================================
 * Section: PKTFWD Functional Interfaces
 * =============================================================================
 */

typedef int (*xmit_fn_t)(struct sk_buff *skb, struct net_device *dev);

d3lut_t * wl_pktfwd_lut(void) /* accessor function */
{
    return wl_pktfwd_g.d3lut;
}
EXPORT_SYMBOL(wl_pktfwd_lut);


pktlist_context_t * wl_pktfwd_pktlist_context(int domain) /* debug ONLY */
{
    PKTFWD_ASSERT(domain < WL_PKTFWD_RADIOS);
    return WL_PKTLIST_CONTEXT(domain);
}
EXPORT_SYMBOL(wl_pktfwd_pktlist_context);


/** Schedule work to WLAN thread, by waking it up */
static inline void
_wl_schedule_work(wl_info_t * wl)
{
    atomic_inc(&wl->callbacks);
#ifdef WL_ALL_PASSIVE
    wl->txq_txchain_dispatched = true;
    wake_up_interruptible(&wl->kthread_wqh);
#endif
}   /* _wl_schedule_work() */

/** WLAN thread dispatched all pending work, and sleeping */
static inline void
wl_complete_work(wl_info_t * wl)
{
#ifdef WL_ALL_PASSIVE
    wl->txq_txchain_dispatched = false;
#endif

#if (CC_PKTFWD_DEBUG >= 1)
    {
        int callbacks = atomic_dec_return(&wl->callbacks);
        if (callbacks < 0) {
            PKTFWD_ERROR("%s: wl%d: callbacks dropped below zero\n",
                __FUNCTION__, wl->unit);
            ASSERT(0);
        }
    }
#else
    atomic_dec(&wl->callbacks);
#endif /* ! (CC_PKTFWD_DEBUG >= 1) */

}   /* wl_complete_work() */


/** Fetch net_device given a interface index (subunit) */
static inline struct net_device *
__subunit_2_net_device(wl_info_t * wl, uint subunit)
{
    wl_if_t * wlif;
    struct net_device * net_device;

    PKTFWD_ASSERT(wl != (wl_info_t *) NULL);
    wlif = wl->if_list;
    net_device = (struct net_device *) NULL;

    while (wlif != NULL) {
        if (wlif->subunit == subunit) {
            net_device = wlif->dev;
            break;
        }
        wlif = wlif->next;
    }

    return net_device;
}   /* __subunit_2_net_device() */

/** Fetch the d3fwd_wlif for the primary interface of a radio */
static inline d3fwd_wlif_t *
__wl_2_d3fwd_wlif(wl_info_t * wl)
{
    wl_if_t * wlif;

    PKTFWD_ASSERT(wl != (wl_info_t *) NULL);
    PKTFWD_ASSERT(wl->dev != (struct net_device *) NULL);

    wlif = WL_DEV_IF(wl->dev);
    PKTFWD_ASSERT(wlif != (wl_if_t *) NULL);

    return (d3fwd_wlif_t *) (wlif->d3fwd_wlif);

}   /* __wl_2_d3fwd_wlif() */


/** Fetch the d3fwd_wlif for a given primary or virtual interface */
static inline d3fwd_wlif_t *
__wlif_2_d3fwd_wlif(wl_if_t * wlif)
{
    PKTFWD_ASSERT(wlif != (wl_if_t *) NULL);
    return (d3fwd_wlif_t *) (wlif->d3fwd_wlif);
}   /* __wlif_2_d3fwd_wlif() */


/** fetch the d3lut_elem_t given a flowid & pool index */
static inline d3lut_elem_t *
__wl_flowid_2_d3lut_elem(int d3domain, uint16_t flowid)
{
    uint16 gbl_index; /* global scoped endpoint index */
    d3lut_elem_t * d3lut_elem;
    wl_pktfwd_t  * wl_pktfwd = &wl_pktfwd_g;

    gbl_index = PKTFWD_GBL_ENDPOINT_IDX(d3domain, WL_CFPID2LUTID(flowid));
    PKTFWD_ASSERT(gbl_index < PKTFWD_ENDPOINTS_WLAN);

    d3lut_elem = wl_pktfwd->d3lut->elem_base + gbl_index;

    return d3lut_elem;

}   /* __wl_flowid_2_d3lut_elem() */


/** fetch the d3lut_elem_t given a symbol key::v32 */
static inline d3lut_elem_t *
__d3lut_key_2_d3lut_elem(uint16_t key_v16)
{
    uint16 gbl_index; /* global scoped endpoint index */
    d3lut_elem_t * d3lut_elem;
    wl_pktfwd_t  * wl_pktfwd = &wl_pktfwd_g;

    gbl_index = PKTFWD_GBF(key_v16, D3LUT_KEY_INDEX);
    PKTFWD_ASSERT(gbl_index < PKTFWD_ENDPOINTS_WLAN);

    d3lut_elem = wl_pktfwd->d3lut->elem_base + gbl_index;

    return d3lut_elem;

}   /* __d3lut_key_2_d3lut_elem() */


/** Fetch the d3fwd_wlif_t given a symbol key::v32 */
static inline d3fwd_wlif_t *
__d3lut_key_2_d3fwd_wlif(uint16_t key_v16)
{
    d3lut_elem_t * d3lut_elem;

    d3lut_elem = __d3lut_key_2_d3lut_elem(key_v16);
 
    return d3lut_elem->ext.d3fwd_wlif; /* could be D3FWD_WLIF_NULL */

}   /* __key_2_d3fwd_wlif() */


/**
 * -----------------------------------------------------------------------------
 * Function : Helper debug dump the system global header and statistics
 * -----------------------------------------------------------------------------
 */
static void
_wl_pktfwd_sys_dump(void)
{
    wl_pktfwd_t       * wl_pktfwd = &wl_pktfwd_g;
    wl_pktfwd_stats_t * wl_pktfwd_stats = &wl_pktfwd_g.stats;

    printk(PKTFWD_VRP_FMT " Dump, radios %u wlifs %u tx_accel %s dwds_accel(ap/sta) %s/%s init %s\n",
           PKTFWD_VRP_VAL(WL_PKTFWD, WL_PKTFWD_VERSIONCODE),
           wl_pktfwd->radio_cnt, wl_pktfwd->wlif_cnt,
           (wl_pktfwd->tx_accel == 0) ? "NO" : "YES",
           (wl_dwds_ap_pktfwd_accel == false) ? "NO" : "YES",
           (wl_dwds_sta_pktfwd_accel == false) ? "NO" : "YES",
           (wl_pktfwd->initialized == false) ? "NO" : "YES");

    /* Dump global system statistics */
    printk("\t Pkt Tx[CFP %u CHN %u FC %u] Rx[Fast %u, Slow %u] "
           "sta %u drop %u fail %u pktlist xmits %u preempts %u txevals %u\n",
        wl_pktfwd_stats->txf_cfp_pkts, wl_pktfwd_stats->txf_chn_pkts,
        wl_pktfwd_stats->txf_fkb_pkts,
        wl_pktfwd_stats->rx_fast_fwds, wl_pktfwd_stats->rx_slow_fwds,
        wl_pktfwd_stats->tot_stations, wl_pktfwd_stats->pkts_dropped,
        wl_pktfwd_stats->ops_failures,
        wl_pktfwd_stats->pktlist_xmit, wl_pktfwd_stats->xmit_preempt,
        wl_pktfwd_stats->txeval_xmit);

    memset(wl_pktfwd_stats, 0, sizeof(wl_pktfwd_g.stats));

}   /* _wl_pktfwd_sys_dump() */


/**
 * -----------------------------------------------------------------------------
 * Function : Helper routine to free all packet in a pktlist.
 * -----------------------------------------------------------------------------
 */

static void /* Without osh accounting */
wl_pktwd_pktlist_free(pktlist_context_t * pktlist_context,
                      pktlist_t * pktlist_free)
{
    pktlist_pkt_t * pkt;
    wl_pktfwd_t   * wl_pktfwd = &wl_pktfwd_g;

    PKTFWD_FUNC();

    wl_pktfwd->stats.pkts_dropped += pktlist_free->len;

#if defined(BCM_PKTFWD_FLCTL)
    __pktlist_fctable_add_credits(pktlist_context, pktlist_free->prio,
                                  pktlist_free->dest, pktlist_free->len);
#endif /* BCM_PKTFWD_FLCTL */

    while (pktlist_free->len--)
    {
        pkt = pktlist_free->head;
        pktlist_free->head = PKTLIST_PKT_SLL(pkt, SKBUFF_PTR);

        /* No osh accounting, as not yet in WLAN */
        PKTLIST_PKT_FREE(pkt);
    }

    PKTLIST_RESET(pktlist_free); /* head,tail, not reset */

    return;

}   /* wl_pktwd_pktlist_free() */


/**
 * -----------------------------------------------------------------------------
 * Function : Helper routine to reset a d3fwd_wlif object by freeing all packets
 * -----------------------------------------------------------------------------
 */
static void
_wl_pktwd_d3fwd_wlif_reset(d3fwd_wlif_t * d3fwd_wlif)
{
    d3fwd_wlif->ucast_bmap  = 0U; /* no pending work */

    d3fwd_wlif->osh         = (osl_t *) NULL;
    d3fwd_wlif->wlif        = (wl_if_t *) NULL;
    d3fwd_wlif->net_device  = (struct net_device *) NULL;
    d3fwd_wlif->wl_schedule = ~0; /* set to WLAN thread as scheduled */
    d3fwd_wlif->unit        = ~0; /* scribble */
#if defined(BCM_WFD)
    d3fwd_wlif->wfd_idx     = ~0; /* scribble */
#endif
    d3fwd_wlif->stations    = 0;  /* none */

}   /* _wl_pktwd_d3fwd_wlif_reset() */

static void
wl_pktwd_d3fwd_wlif_reset(d3fwd_wlif_t * d3fwd_wlif)
{
    int prio;
    osl_t * osh;
    dll_t * worklist;
    pktlist_t pktlist_free;
    pktlist_context_t * wlif_pktlist_context;
    wl_pktfwd_t       * wl_pktfwd = &wl_pktfwd_g;

    osh = d3fwd_wlif->osh;
    PKTLIST_RESET(&pktlist_free); /* len = 0U, key.v16 = don't care */

    PKTFWD_ASSERT(d3fwd_wlif->unit < WL_PKTFWD_RADIOS);
    wlif_pktlist_context = WL_PKTLIST_CONTEXT(d3fwd_wlif->unit);

    PKTFWD_ASSERT(wlif_pktlist_context != PKTLIST_CONTEXT_NULL);

    PKTLIST_LOCK(wlif_pktlist_context); // ++++++++++++++++++++++++++++++++++++

#if defined(WL_PKTFWD_RUNQ)
    {   /* If d3fwd_wlif is in run queue, remove it by freeing packets */
        wl_pktfwd_runq_t *runq = WL_RUNQ_P(d3fwd_wlif->unit);
        if (runq->d3fwd_wlif == d3fwd_wlif)
        {
            for (prio = 0; prio < D3FWD_PRIO_MAX; ++prio)
            {
                worklist = &runq->ucast[prio];
                if (!dll_empty(worklist))
                {
                    __pktlist_xfer_pktlist(worklist, &pktlist_free, SKBUFF_PTR);
                    dll_join(worklist, &wlif_pktlist_context->free);
                }
            }
            runq->ucast_bmap  = 0U;
            runq->d3fwd_wlif  = D3FWD_WLIF_NULL;
        }
    }
#endif /* WL_PKTFWD_RUNQ */

    /* Flush all pending packets accumulated in pktlists tracked by worklist */
    worklist = &d3fwd_wlif->mcast;
    if (!dll_empty(worklist))
    {
        __pktlist_xfer_pktlist(worklist, &pktlist_free, SKBUFF_PTR);
        dll_join(worklist, &wlif_pktlist_context->free);
    }

    for (prio = 0; prio < D3FWD_PRIO_MAX; ++prio)
    {
        worklist = &d3fwd_wlif->ucast[prio];
        if (!dll_empty(worklist))
        {
            __pktlist_xfer_pktlist(worklist, &pktlist_free, SKBUFF_PTR);
            dll_join(worklist, &wlif_pktlist_context->free);
        }
    }

    _wl_pktwd_d3fwd_wlif_reset(d3fwd_wlif);

    wl_pktfwd->stats.pkts_dropped += pktlist_free.len;

    PKTLIST_UNLK(wlif_pktlist_context); // ------------------------------------


    /* LOCK-FREE: kfree all packets in the pktlist_free */
    wl_pktwd_pktlist_free(wlif_pktlist_context, &pktlist_free);

}   /* wl_pktwd_d3fwd_wlif_reset() */


#if defined(BCM_PKTFWD_FLCTL)

/**
 * -----------------------------------------------------------------------------
 * Function : Update credits for a pktlist identified by prio & dest
 * -----------------------------------------------------------------------------
 */

void
wl_pktfwd_update_link_credits(wl_info_t * wl, uint16_t cfp_flowid,
    uint8_t * d3addr, uint32_t prio, int32_t credits, bool add)
{
    uint32_t dest;
    d3lut_elem_t      * d3lut_elem;
    pktlist_context_t * pktlist_context;
    wl_pktfwd_t       * wl_pktfwd = &wl_pktfwd_g;

    pktlist_context = WL_PKTLIST_CONTEXT(wl->unit);

#if defined(WLCFP)
    if (CFP_ENAB(wl->pub) == TRUE)
    {
        PKTFWD_ASSERT(cfp_flowid != ID16_INVALID);
        d3lut_elem = __wl_flowid_2_d3lut_elem(wl->unit, cfp_flowid);
    }
    else
#endif /* WLCFP */
    {
        /* TODO: d3addr lookup for every enq and deq of a frame is too costly
         * Attach a d3lut_elem index to station (SCB). */
        /* d3lut::lock is NOT taken !!! */
        d3lut_elem = d3lut_lkup(wl_pktfwd->d3lut, d3addr, wl->unit);
    }

    if ((d3lut_elem == D3LUT_ELEM_NULL) || !(d3lut_elem->ext.inuse))
        return; /* Station is not registered with PKTFWD */

    PKTFWD_ASSERT(d3lut_elem->ext.wlan);

    dest = D3LUT_ELEM_IDX(d3lut_elem->ext.flow_key.index);

    if (add)
        __pktlist_fctable_add_credits(pktlist_context, prio, dest, credits);
    else
        __pktlist_fctable_sub_credits(pktlist_context, prio, dest, credits);

}   /* wl_pktfwd_update_link_credits() */

#endif /* BCM_PKTFWD_FLCTL */


/**
 * -----------------------------------------------------------------------------
 * Function : Conditionally insert an address into the PKTFWD LUT.
 *            - net_device is a WLAN device and has been registered with PKTFWD
 *              Reference TODO #1: if DHD is supported, DHD should not register
 *              it's net_device with PKTFWD. WFD to DHD uses "flowring index"
 *              without pktlist based binning.
 *            - net_device is NOT for a Wireless Distribution Service (WDS)
 * Returns  : wfd_idx and element key <incarn,index>
 * -----------------------------------------------------------------------------
 */
static inline unsigned long
wl_pktfwd_cache(uint8_t * d3addr, struct net_device * net_device)
{
    bool is_wlan, cache_eligible;
    d3lut_elem_t * d3lut_elem;

    PKTFWD_PTRACE(D3LUT_SYM_FMT "%s", D3LUT_SYM_VAL(d3addr), net_device->name);

    is_wlan = is_netdev_wlan(net_device);  

    if (is_wlan)
    {
        d3fwd_wlif_t * d3fwd_wlif;
        wl_if_t * wlif;

        /* check if wlan virtual device */
        if (check_virt_wlan(net_device))
            goto wl_pktfwd_cache_failure;

        wlif = WL_DEV_IF(net_device);
        d3fwd_wlif = wlif->d3fwd_wlif;

        cache_eligible = ((wlif) && (wlif->d3fwd_wlif != D3FWD_WLIF_NULL)) ?
            true : false; /* only WLAN NIC endpoints, exclude DHD endpoints */

        if ((wl_dwds_ap_pktfwd_accel == false) && d3fwd_wlif && is_netdev_wlan_dwds_ap(d3fwd_wlif))
             goto wl_pktfwd_cache_failure;
        if ((wl_dwds_sta_pktfwd_accel == false) && d3fwd_wlif && is_netdev_wlan_dwds_client(d3fwd_wlif))
             goto wl_pktfwd_cache_failure;
    }
    else
        cache_eligible = true; /* LAN endpoints are always eligible */

    if (cache_eligible == false)
        goto wl_pktfwd_cache_failure;

    /* Insert into D3LUT */
    d3lut_elem = wl_pktfwd_lut_ins(d3addr, net_device, is_wlan);

    if (d3lut_elem == D3LUT_ELEM_NULL) /* collision maybe or a D3LUT error */
        goto wl_pktfwd_cache_failure;

    if (is_wlan) { /* audit D3LUT cached element ... no lock though */
        PKTFWD_ASSERT(d3lut_elem->ext.d3fwd_wlif != D3FWD_WLIF_NULL);
        PKTFWD_ASSERT(d3lut_elem->ext.d3fwd_wlif->unit == d3lut_elem->key.domain);
    } else {
        PKTFWD_ASSERT(d3lut_elem->key.domain == PKTFWD_XDOMAIN_IDX);
    }

    PKTFWD_PTRACE(D3LUT_ELEM_FMT, D3LUT_ELEM_VAL(d3lut_elem));

    /* 2b wfdidx | 16 chainidx (2b domain, 2b incarn, 12b index) */
    return (unsigned long) PKTC_WFD_CHAIN_IDX(d3lut_elem->key.domain, d3lut_elem->key.v16);

wl_pktfwd_cache_failure:

    PKTFWD_WARN("WL_PKTFWD_KEY_INVALID_UL");
    return WL_PKTFWD_KEY_INVALID_UL;

}   /* wl_pktfwd_cache() */


#if !(defined(BCM_AWL) && defined(WL_AWL_RX))
/**
 * -----------------------------------------------------------------------------
 *
 * Function : Lookup the PKTFWD LUT for a matching 802.3 d3addr and device
 *
 * Operation: Invoked in WLAN Rx Path (is chainable) via PKTC_TBL_FN_CMP().
 *
 * Invoked on a per packet basis for those packets that are not candidates
 * for CFP bypass.
 * A packet's 802.3 MacAddress will be searched in the D3LUT. If found, the
 * interface that manages this MacAddress (i.e. transmit interface) is fetched.
 * The packet will be eligible for packet chain based forwarding from WLAN to
 * LAN (or anothe WLAN) interface if the MacAddress is found and it is not a
 * loopback to the receiving interface (intra-BSS).
 *
 * Returns  :
 *   false  : if lookup fails, or a loopback (i.e. 802.3 d3addr is associated
 *            with a WLAN transmit net_device as the receive net_device).
 *   true   : if found and not a loopback.
 *
 * Caveat   : No test is performed on whether global wl_pktfwd is initialized.
 *            Lock free lookup is performed to the destination. At most a miss
 *            may occur, if the d3lut_elem was moved from one bin to another.
 *
 * -----------------------------------------------------------------------------
 */

int BCMFASTPATH
wl_pktfwd_match(uint8_t * d3addr, struct net_device * rx_net_device)
{
    d3fwd_wlif_t * d3fwd_wlif;
    d3lut_elem_t * d3lut_elem;
    wl_pktfwd_t  * wl_pktfwd = &wl_pktfwd_g;

    PKTFWD_PTRACE(D3LUT_SYM_FMT "rx %s",
        D3LUT_SYM_VAL(d3addr), rx_net_device->name);

    /* d3lut::lock is NOT taken !!! */
    d3lut_elem = d3lut_lkup(wl_pktfwd->d3lut, d3addr, D3LUT_LKUP_GLOBAL_POOL);

    if (d3lut_elem == D3LUT_ELEM_NULL)
        return false; /* Lookup failure */

    /* Found a matching d3addr in the D3LUT */

    if (!d3lut_elem->ext.wlan) /* WLAN -> non WLAN, i.e. not a loopback */
        return true;

    /* Destination is also a WLAN, locate the transmit net_device */
    d3fwd_wlif = d3lut_elem->ext.d3fwd_wlif;
    if (likely(d3fwd_wlif))
        return (d3fwd_wlif->net_device != rx_net_device);
    else
        return true;

}   /* wl_pktfwd_match() */


#if defined(WLCFP)
/**
 * -----------------------------------------------------------------------------
 * Function : Get CFP Flowid for a matching 802.3 d3addr in PKTFWD LUT
 *
 * Operation: Invoked in WLAN CFP Rx Path.
 *
 * Invoked on a per packet basis for WLAN receive packets.
 * A packet's transmit MacAddress (TA) will be searched in the D3LUT. If found,
 * return cfp_flowid for the matching TA.
 *
 * Caveat   : No test is performed on whether global wl_pktfwd is initialized.
 *            Lock free lookup is performed to the destination. At most a miss
 *            may occur, if the d3lut_elem was moved from one bin to another.
 * -----------------------------------------------------------------------------
 */

uint16_t
wl_pktfwd_get_cfp_flowid(wl_info_t * wl, uint8_t * d3addr)
{
    uint16_t cfp_flowid, d3lut_elem_idx;
    d3lut_elem_t * d3lut_elem;
    wl_pktfwd_t  * wl_pktfwd = &wl_pktfwd_g;

    cfp_flowid = ID16_INVALID;

    /* d3lut::lock is NOT taken !!! */
    d3lut_elem = d3lut_lkup(wl_pktfwd->d3lut, d3addr, wl->unit);

    if ((d3lut_elem != D3LUT_ELEM_NULL) && (d3lut_elem->ext.wlan))
    {
        d3lut_elem_idx = D3LUT_ELEM_IDX(d3lut_elem->ext.flow_key.index);
        cfp_flowid = WL_FWDID2LUTID(d3lut_elem_idx);
    }

    return cfp_flowid; /* Lookup failure */

}   /* wl_pktfwd_get_cfp_flowid() */

#endif /* WLCFP */

#endif /* ! (BCM_AWL && WL_AWL_RX) */


/**
 * -----------------------------------------------------------------------------
 *
 * Function : Requests from bridge, wlan subsystems. See enum wl_pktfwd_req_t.
 *            Retaining the wl_pktc_req() signature along with PKTC and PKTC_TBL
 *
 * Operation: Invoked by Linux bridge and WLAN driver via wl_pktc_req_hook().
 *
 * -----------------------------------------------------------------------------
 */

unsigned long
wl_pktfwd_request(int request,
              unsigned long param0, unsigned long param1, unsigned long param2)
{
    unsigned long response = 0UL;
    wl_pktfwd_t * wl_pktfwd = &wl_pktfwd_g;

    PKTFWD_ASSERT((request > wl_pktfwd_req_undefined_e)
               && (request < wl_pktfwd_req_max_e));
    PKTFWD_PTRACE("%s", wl_pktfwd_req_str[request]);

    PKTFWD_ASSERT(wl_pktfwd->initialized == true);
    if (wl_pktfwd->initialized == false)
        goto wl_pktfwd_request_done;

    switch (request)
    {
        /* IOVAR: set Transmit Acceleration of bridged traffic */
        case wl_pktfwd_req_set_txmode_e: /* param0: 1 = enable, 0 = disable */
            wl_pktfwd->tx_accel = (uint8_t)param0;
            break;

        /* IOVAR: get Transmit Acceleration of bridged traffic */
        case wl_pktfwd_req_get_txmode_e: /* response: 1 = enable, 0 = disable */
            response = (unsigned long)wl_pktfwd->tx_accel;
            break;

        case wl_pktfwd_req_ins_symbol_e: /* param0: d3addr, param1: dev_p */
            response = (unsigned long)
                wl_pktfwd_cache((uint8_t *)param0, (struct net_device *)param1);
            break;

        /* ndo_uninit (net_dev deregister or register failure: wl_uninit() */
        case wl_pktfwd_req_flush_full_e: /* param0: net_device */
            wl_pktfwd_lut_clr((struct net_device *)param0);
            break;

        case wl_pktfwd_req_assoc_sta_e: /* param0: d3addr, param1: 0|1 assoc */
                                                        /* param2: eventtype */

            if (param2 == WLC_E_ASSOC)
            {
                /* If a station is roaming, there might be stale entries in d3lut.
                 * Look for symbol in global pool and delete if found. */
                wl_pktfwd_lut_del((uint8_t *)param0, (struct net_device *) NULL);
            }

            if (param2 == WLC_E_ASSOC_IND)
                ++wl_pktfwd->stats.tot_stations;
            else if (param2 == WLC_E_DISASSOC_IND)
                --wl_pktfwd->stats.tot_stations;

            /* LUT is not populated on association, for consistency in bridge */
            break;

        case wl_pktfwd_req_pktlist_e: /* param0: unit */

            PKTFWD_ASSERT(param0 < WL_PKTFWD_RADIOS);
            if (WL_PKTLIST_CONTEXT(param0) != PKTLIST_CONTEXT_NULL)
                response = (unsigned long) WL_PKTLIST_CONTEXT(param0);
            else
                PKTFWD_ERROR("pktlist_context %lu not initialized", param0);
            break;

        default:
            ++wl_pktfwd->stats.ops_failures;
            PKTFWD_ERROR("request %u invalid", request);
            break;

    } /* switch */

wl_pktfwd_request_done:

    return response;

}   /* wl_pktfwd_request() */

d3lut_elem_t *
wl_pktfwd_lut_lkup(struct net_device * net_device, d3lut_t *d3lut, uint8_t *d3_addr, uint32_t pool)
{
    d3lut_elem_t * d3lut_elem;
    d3fwd_wlif_t * d3fwd_wlif;
    wl_if_t * wlif = (wl_if_t *) NULL;
	
    /* check WDS case first */
    if ((net_device != NULL) && is_netdev_wlan(net_device)) {
        wlif = WL_DEV_IF(net_device);
        d3fwd_wlif = wlif->d3fwd_wlif;

        if (d3fwd_wlif && 
            ((is_netdev_wlan_dwds_ap(d3fwd_wlif) || is_netdev_wlan_dwds_client(d3fwd_wlif)) && 
             (d3fwd_wlif->wds_d3lut_elem != NULL))) {
            d3lut_elem = d3fwd_wlif->wds_d3lut_elem;
            goto done;
        }
    }

    /* for other cases */
    d3lut_elem = d3lut_lkup(d3lut, d3_addr, pool);

done:
    return d3lut_elem;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Insert an 802.3 mac-address endpoint into the PKTFWD LUT
 * Operation: Invoked via HOOK_FN wl_pktfwd_request HOOK_FN, to populate the
 *
 * D3LUT table with a 802.3 MacAddress, and data fill the associated interface
 * information. It is permissible for a 802.3 MacAddress to be requested for
 * addition multiple times, resulting in a overwrite of the duplicate entry.
 * 
 * In the D3 LUT, elements are managed in pools (aka domains), with a domain per
 * WLAN radio and the last domain for all other non WLAN radios. Free elements
 * in the LAN pool (LAN domain) are managed in a LIFO freelist allocation
 * policy. WLAN domains, are managed in a allocate by index policy when CFP is
 * compiled, or LIFO freelist. This allows for an elements index to match that
 * of a CFP flowid.
 *
 * -----------------------------------------------------------------------------
 */

#if !defined(BCM_PKTFWD_FLCTL) && defined(WLCFP)
extern int /* Find the CFP flowid assigned to a Station for given MacAddr */
wlc_cfp_link_update(wlc_info_t * wlc, wlc_if_t * wlcif,
    uint8 * d3addr, uint16 * cfp_flowid);
#endif /* !BCM_PKTFWD_FLCTL && WLCFP */

void * /* d3lut_elem_t * */
wl_pktfwd_lut_ins(uint8_t * d3addr,
                  struct net_device * net_device, bool is_wlan)
{
    int  ssid = ~0, d3domain; /* d3addr in d3domain (by WLAN radio and LAN) */
#if defined(WLCFP) || defined(BCM_PKTFWD_FLCTL)
    uint16_t cfp_flowid = ID16_INVALID;
#endif /* WLCFP || BCM_PKTFWD_FLCTL */
#if defined(BCM_PKTFWD_FLCTL)
    uint32_t prio;
    int32_t         credits[PKTLIST_PRIO_MAX];       // Not Initialized.
#endif /* BCM_PKTFWD_FLCTL */
    d3lut_policy_t   d3lut_policy;
    d3lut_elem_t   * d3lut_elem;
    void           * if_handle;
    wl_pktfwd_t    * wl_pktfwd = &wl_pktfwd_g;
    wl_info_t      * wl = (wl_info_t *) NULL;
    wl_if_t        * wlif = (wl_if_t *) NULL;
    d3fwd_wlif_t   * d3fwd_wlif = (d3fwd_wlif_t *) NULL;

    PKTFWD_ASSERT(d3addr != (uint8_t *) NULL);
    PKTFWD_ASSERT(net_device != (struct net_device *) NULL);
    PKTFWD_ASSERT(wl_pktfwd->initialized == true);
    PKTFWD_PTRACE(D3LUT_SYM_FMT "%s", D3LUT_SYM_VAL(d3addr), net_device->name);

    /* WLAN and LAN domain: pool+policy selection and interface handle */

    if (is_wlan)
    {
        wl         = WL_INFO_GET(net_device);
        wlif       = WL_DEV_IF(net_device);
        d3fwd_wlif = wlif->d3fwd_wlif;

        PKTFWD_ASSERT(BLOG_GET_PHYTYPE(netdev_path_get_hw_port_type(net_device)) == BLOG_WLANPHY);

        if ((d3fwd_wlif == D3FWD_WLIF_NULL) || (d3fwd_wlif->unit == ~0)) {
            PKTFWD_WARN("d3fwd_wlif absent failure");
            ++wl_pktfwd->stats.ops_failures;

            goto wl_pktfwd_lut_ins_failure;
        }

        /* if d3lut_elem exists for dwds, just return it */
        if ((is_netdev_wlan_dwds_ap(d3fwd_wlif) || is_netdev_wlan_dwds_client(d3fwd_wlif)) && 
                        (d3fwd_wlif->wds_d3lut_elem != NULL)) {
            return (void *) d3fwd_wlif->wds_d3lut_elem;
        }

        d3domain  = wl->unit; /* per radio's dedicated pool of d3lut_elem's */
        if_handle = (void *) d3fwd_wlif; /* wlif extension */
        ssid      = wlif->subunit;  /* WLAN interface index */

        /*
         * Fetch link status and CFP assigned flowid.
         *
         * CFP flowid is used in pool_by_index allocation to accomplish a
         * 1:1 D3LUT element index to CFP flowid mapping.
         * Packets linked using D3LUT assigned index, may then be directly
         * handed to CFP.
         *
         * NOTE: A valid CFP flowid does not imply a flow is CFP capable.
         * CFP bypass acceleration is dynamically controlled by a CFP
         * control plane gatekeeper.
         */
#if defined(BCM_PKTFWD_FLCTL)
        if (wlc_scb_link_update(wl->wlc, wlif->wlcif, d3addr, &cfp_flowid)
                != BCME_OK) {
            PKTFWD_WARN("SCB linkup failure  " D3LUT_SYM_FMT,
                D3LUT_SYM_VAL(d3addr));
            ++wl_pktfwd->stats.ops_failures;

            goto wl_pktfwd_lut_ins_failure;
        }

        if (CFP_ENAB(wl->pub) == TRUE)
        {
            d3lut_policy.pool_by_index = WL_CFPID2LUTID(cfp_flowid);
            PKTFWD_ASSERT(d3lut_policy.v16 < PKTFWD_ENDPOINTS_MAX);
        }
        else /* ! CFP_ENAB */
        {
            d3lut_policy.pool_freelist = D3LUT_POLICY_POOL_FREELIST;
        }
#else /* ! BCM_PKTFWD_FLCTL */

#if defined(WLCFP)
        if (CFP_ENAB(wl->pub) == TRUE)
        {
            if (wlc_cfp_link_update(wl->wlc, wlif->wlcif, d3addr, &cfp_flowid)
                    == BCME_ERROR) {
                PKTFWD_WARN("cfp flowid failure");
                ++wl_pktfwd->stats.ops_failures;

                goto wl_pktfwd_lut_ins_failure;
            }

            d3lut_policy.pool_by_index = WL_CFPID2LUTID(cfp_flowid);
            PKTFWD_ASSERT(d3lut_policy.v16 < PKTFWD_ENDPOINTS_MAX);
        }
        else /* ! CFP_ENAB */
#endif   /* WLCFP */
        {
            /* TODO: SHOULD ensure that STA is associated */
            d3lut_policy.pool_freelist = D3LUT_POLICY_POOL_FREELIST;
        }
#endif /* ! BCM_PKTFWD_FLCTL */

    }
    else /* ! is_wlan */
    {
        if_handle = (void *) (net_device); /* for non-WLAN, use net_device */
        d3domain  = (D3LUT_POOL_TOT - 1); /* non-WLAN pool is last pool */
        d3lut_policy.pool_freelist = D3LUT_POLICY_POOL_FREELIST;
    }

    /*
     * Allocate a d3lut_elem and insert symbol into the d3lut dictionary.
     *
     * NOTE: d3lut_elem may already "pre-exist" in d3lut dictionary.
     *
     * d3domain identifies the d3lut element pool (WLAN/LAN) from which an
     * d3 element will be selected with a 1:1 or free list allocation policy
     */

    D3LUT_LOCK(wl_pktfwd->d3lut); // ++++++++++++++++++++++++++++++++++++++++++

    d3lut_elem = d3lut_ins(wl_pktfwd->d3lut, d3addr, d3domain, d3lut_policy);

    D3LUT_UNLK(wl_pktfwd->d3lut); // ------------------------------------------

    if (unlikely(d3lut_elem == D3LUT_ELEM_NULL)) {
        PKTFWD_WARN("d3lut_ins failure");
        ++wl_pktfwd->stats.ops_failures;

        goto wl_pktfwd_lut_ins_failure;
    }

    /* Data fill the d3lut extension */ 
    PKTFWD_LOCK();

    PKTFWD_ASSERT(d3lut_elem->ext.inuse == 1);
    if (is_wlan)
    {

#if defined(WLCFP)
        PKTFWD_ASSERT((d3lut_policy.pool_freelist == D3LUT_POLICY_POOL_FREELIST) ||
                (PKTFWD_DOMAIN_ENDPOINT_IDX(d3lut_elem->key.index) == d3lut_policy.v16));
#endif /* WLCFP */

        if (d3lut_elem->ext.assoc == 0)
        {
            d3lut_elem->ext.assoc  = 1; /* tag station as associated */
            d3lut_elem->ext.wlan   = 1;

            d3lut_elem->ext.d3fwd_wlif = (d3fwd_wlif_t *) if_handle;
            if (d3lut_elem->ext.d3fwd_wlif->unit != ~0)
                ++d3lut_elem->ext.d3fwd_wlif->stations;

            /* Extension inherits element's key: domain incarn index
             * Caution: key::index is global endpoint scoped
             */
            d3lut_elem->ext.flowid  = d3lut_elem->key.v16;
            d3lut_elem->ext.ssid    = ssid;

#if defined(BCM_PKTFWD_FLCTL)
            /* Get credits of  SCB pktq */
            wlc_scb_get_link_credits(wl->wlc, wlif->wlcif, d3addr,
                    cfp_flowid, credits);

            /* Update credits for the pktlist */
            for (prio = 0U; prio < PKTLIST_PRIO_MAX; ++prio)
            {
                /* FIXME: Stale packets from previous session are pending in
                 * pktlist and used some credits (-ve credits).
                 * Make sure pktlist is empty before allocating an
                 * d3lut element.
                 */ 
                PKTFWD_ASSERT(__pktlist_fctable_get_credits(
                    WL_PKTLIST_CONTEXT(wl->unit), prio,
                    D3LUT_ELEM_IDX(d3lut_elem->ext.flow_key.index)) == 0);

                __pktlist_fctable_set_credits(
                    WL_PKTLIST_CONTEXT(wl->unit), prio,
                    D3LUT_ELEM_IDX(d3lut_elem->ext.flow_key.index), /* dest */
                    credits[prio]);
            }
#endif /* BCM_PKTFWD_FLCTL */

        }

        wl = WL_INFO_GET(net_device);
        wlif = WL_DEV_IF(net_device);
        d3fwd_wlif = wlif->d3fwd_wlif;

        if (d3fwd_wlif && (is_netdev_wlan_dwds_ap(d3fwd_wlif) || is_netdev_wlan_dwds_client(d3fwd_wlif))) {
            /* save d3lut_elem for dwds use */
            d3lut_elem->key.incarn = 0; /* reset incarn as d3lut_elem is deleted due to roaming */
            d3fwd_wlif->wds_d3lut_elem = d3lut_elem;
        }
    }
    else /* ! is_wlan */
    {
        d3lut_elem->ext.net_device  = (struct net_device *) if_handle;
        d3lut_elem->ext.ssid        = ssid;
    }

    PKTFWD_UNLK();

    return (void *) d3lut_elem;

wl_pktfwd_lut_ins_failure:

    return (void *) D3LUT_ELEM_NULL;

}   /* wl_pktfwd_lut_ins() */


/**
 * -----------------------------------------------------------------------------
 * Function : Delete an endpoint from the PKTFWD LUT
 * HOOK_FN  : Invoked by Blog SHIM via wl_pktc_del_hook
 * -----------------------------------------------------------------------------
 */
static inline void /* must be called with d3lut lock and non-null d3lut_elem */
_wl_pktfwd_lut_del(uint8_t * d3addr, d3lut_elem_t * d3lut_elem,
                   uint32_t d3domain, struct net_device * net_device)
{
    wl_pktfwd_t  * wl_pktfwd = &wl_pktfwd_g;

    PKTFWD_ASSERT(d3lut_elem != D3LUT_ELEM_NULL);
    PKTFWD_ASSERT(wl_pktfwd->initialized == true);

    if (d3lut_elem->ext.wlan && d3lut_elem->ext.assoc)
    {
        d3fwd_wlif_t * d3fwd_wlif;
        d3fwd_wlif = d3lut_elem->ext.d3fwd_wlif;
        PKTFWD_ASSERT(d3fwd_wlif != D3FWD_WLIF_NULL);

        d3lut_elem->ext.assoc  = 0; /* tag station as disassociated */
        d3lut_elem->ext.wlan   = 0;
        d3lut_elem->ext.if_handle = (void *) NULL;

        if (d3fwd_wlif->stations)
            --d3fwd_wlif->stations; /* update d3fwd_wlif station count */
        /* Do we need to flush packets already in d3fwd_wlif::ucast ? No */

        d3lut_elem->ext.assoc  = 0; /* tag station as disassociated */
        d3lut_elem->ext.wlan   = 0;

#if defined(BCM_PKTFWD_FLCTL)
        if (WL_PKTLIST_CONTEXT(d3domain) != PKTLIST_CONTEXT_NULL)
        {
            /* Only for NIC domains with flow control enabled */
            for (uint32_t prio = 0U; prio < PKTLIST_PRIO_MAX; ++prio)
            {
                /* FIXME: Sanity check for full credits,
                 * But SCB is already deleted so can't query for credits */

                /* Reset credits for the station */
                __pktlist_fctable_set_credits(
                    WL_PKTLIST_CONTEXT(d3domain), prio,
                    D3LUT_ELEM_IDX(d3lut_elem->ext.flow_key.index), 0);
            }
        }
#endif /* BCM_PKTFWD_FLCTL */

        if (d3fwd_wlif && (is_netdev_wlan_dwds_ap(d3fwd_wlif) || is_netdev_wlan_dwds_client(d3fwd_wlif)))
        {
            d3fwd_wlif->wds_d3lut_elem = NULL;
            d3addr = d3lut_elem->sym.v8; /* replace d3addr to be able to delete */
        }
    }

#if defined(BCM_BLOG)
    /* Flush flows associated with the device */
    /* NOTE: called by blog_notify_async:DESTROY_BRIDGEFDB, which has blog_lock() already */
    if (net_device != NULL) {
          blog_notify(UPDATE_NETDEVICE, net_device, 0, 0);
    }
#endif

    (void) d3lut_del(wl_pktfwd->d3lut, d3addr, d3domain);
}

typedef void (* wl_pktfwd_del_hook_t)(unsigned long addr,
                                      struct net_device * net_device);
void
wl_pktfwd_lut_del(uint8_t * d3addr, struct net_device * net_device)
{
    int            d3domain; /* d3addr in d3domain (by WLAN radio and LAN) */
    d3lut_elem_t * d3lut_elem;
    wl_pktfwd_t  * wl_pktfwd = &wl_pktfwd_g;

    PKTFWD_TRACE(D3LUT_SYM_FMT, D3LUT_SYM_VAL(d3addr));

    if (net_device != NULL)
    {
        bool is_wlan, lut_del_eligible;

        is_wlan = is_netdev_wlan(net_device);

        if (is_wlan)
        {
            d3fwd_wlif_t * d3fwd_wlif;
            wl_if_t * wlif;
            wl_info_t * wl;

            /* check if wlan virtual device */
            if (check_virt_wlan(net_device))
                return;

            wlif = WL_DEV_IF(net_device);
            if (!virt_addr_valid(wlif))
                return;

            d3fwd_wlif = wlif->d3fwd_wlif;

            /* only WLAN NIC endpoints, exclude DHD endpoints */
            lut_del_eligible = ((wlif) && (wlif->d3fwd_wlif != D3FWD_WLIF_NULL)) ?
                true : false;

            if (lut_del_eligible == false)
                return;

            if ((wl_dwds_ap_pktfwd_accel == false) && d3fwd_wlif && is_netdev_wlan_dwds_ap(d3fwd_wlif))
                return;
            if ((wl_dwds_sta_pktfwd_accel == false) && d3fwd_wlif && is_netdev_wlan_dwds_client(d3fwd_wlif))
                return;

            wl = WL_INFO_GET(net_device);
            d3domain  = wl->unit; /* per radio's dedicated pool of d3lut_elem's */
        }
        else /* ! is_wlan */
        {
            d3domain  = (D3LUT_POOL_TOT - 1); /* non-WLAN pool is last pool */
        }
    }
    else /* net_device == NULL */
    {
        PKTFWD_WARN("net_device absent. Looking in global pool");
        d3domain = D3LUT_LKUP_GLOBAL_POOL;
    }

    D3LUT_LOCK(wl_pktfwd->d3lut); // ++++++++++++++++++++++++++++++++++++++++++

    d3lut_elem = wl_pktfwd_lut_lkup(net_device, wl_pktfwd->d3lut, d3addr, d3domain);
    if (d3lut_elem != D3LUT_ELEM_NULL)
    {
        if (d3domain == D3LUT_LKUP_GLOBAL_POOL)
            d3domain = d3lut_elem->key.domain;

        /* If WLAN and station was associated, update the d3fwd_wlif stats */
        _wl_pktfwd_lut_del(d3addr, d3lut_elem, d3domain, net_device);
    }

    D3LUT_UNLK(wl_pktfwd->d3lut); // ------------------------------------------

}   /* wl_pktfwd_lut_del() */


/**
 * -----------------------------------------------------------------------------
 * Function : Check whether packets passed through a PKTFWD LUT entry.
 *
 * HOOK_FN  : Invoked by Linux bridge layer via fdb_check_expired_wl_hook()
 *
 * Operation: Invoked by Linux bridge layer to determine whether a linux bridge
 * fdb entry may be refreshed as packets have passed through the pktfwd system
 * which is acting as a proxy bypass forwarding path to the linux bridge. 
 * -----------------------------------------------------------------------------
 */

typedef int (* wl_pktfwd_hit_hook_t)(uint8_t * d3addr,
                                     struct net_device * net_device);
int
wl_pktfwd_lut_hit(uint8_t * d3addr, struct net_device * net_device)
{
    bool is_wlan, lut_hit_eligible;
    int  d3domain; /* d3addr in d3domain (by WLAN radio and LAN) */
    int expired = -1;
    d3lut_elem_t * d3lut_elem;
    void         * if_handle;
    wl_pktfwd_t  * wl_pktfwd = &wl_pktfwd_g;

    PKTFWD_ASSERT(wl_pktfwd->initialized == true);

    if (net_device != NULL)
    {
        is_wlan = is_netdev_wlan(net_device);

        if (is_wlan)
        {
            wl_if_t      * wlif;
            wl_info_t    * wl;
            d3fwd_wlif_t * d3fwd_wlif;

            /* check if wlan virtual device */
            if (check_virt_wlan(net_device))
                return expired;

            wlif = WL_DEV_IF(net_device);

            /* only WLAN NIC endpoints, exclude DHD endpoints */
            lut_hit_eligible = ((wlif) && (wlif->d3fwd_wlif != D3FWD_WLIF_NULL)) ?
                true : false;

            if (lut_hit_eligible == false)
                return expired;

            wl = WL_INFO_GET(net_device);
            d3fwd_wlif = wlif->d3fwd_wlif;

            if ((wl_dwds_ap_pktfwd_accel == false) && d3fwd_wlif && is_netdev_wlan_dwds_ap(d3fwd_wlif))
                return expired;
            if ((wl_dwds_sta_pktfwd_accel == false) && d3fwd_wlif && is_netdev_wlan_dwds_client(d3fwd_wlif))
                return expired;

            PKTFWD_ASSERT(BLOG_GET_PHYTYPE(netdev_path_get_hw_port_type(net_device)) == BLOG_WLANPHY);

            d3domain  = wl->unit; /* per radio's dedicated pool of d3lut_elem's */
            if_handle = (void *) d3fwd_wlif; /* wlif extension */
        }
        else /* ! is_wlan */
        {
            if_handle = (void *) (net_device); /* for non-WLAN, use net_device */
            d3domain  = (D3LUT_POOL_TOT - 1); /* non-WLAN pool is last pool */
        }
    }
    else /* net_device == NULL */
    {
        PKTFWD_WARN("net_device absent. Looking in global pool");
        d3domain = D3LUT_LKUP_GLOBAL_POOL;
        expired = 1;
    }

    D3LUT_LOCK(wl_pktfwd->d3lut); // ++++++++++++++++++++++++++++++++++++++++++

    d3lut_elem = wl_pktfwd_lut_lkup(net_device, wl_pktfwd->d3lut, d3addr, d3domain);
    if ((d3lut_elem != D3LUT_ELEM_NULL) && (d3lut_elem->ext.inuse))
    {
        /* Sanity: validate interface */
        if (!expired) {
            if (is_wlan)
            {
                expired = !((void *)(d3lut_elem->ext.d3fwd_wlif) == if_handle);
            }
            else /* !is_wlan */
            {
                expired = !((void *)(d3lut_elem->ext.net_device) == if_handle);
            }
        }

        if (expired || d3lut_elem->ext.hit == 0)
        {
            /* delete d3lut_elem */
            _wl_pktfwd_lut_del(d3addr, d3lut_elem, d3domain, net_device);
            expired = 1;
        }
        else
        {
            d3lut_elem->ext.hit = 0;
            expired = 0; /* return to br_fdb.c 0, to retain fdb entry */
        }
    }

    D3LUT_UNLK(wl_pktfwd->d3lut); // ------------------------------------------

    PKTFWD_PTRACE(D3LUT_SYM_FMT "expire %d", D3LUT_SYM_VAL(d3addr), expired);

    return expired;

}   /* wl_pktfwd_lut_hit() */


/**
 * -----------------------------------------------------------------------------
 * Function : Flush all stations in a D3LUT for a given virtual interface.
 * Operation: Invoked on a failure to register a WLAN Virtual Interface
 * or when a Virtual Interface's net_device is unregistered with the network
 * stack.
 *
 * See wl_linux.c:: wl_netdev_ops::ndo_uninit = wl_uninit 
 *    wl_pktfwd_request(PKTC_TBL_FLUSH = wl_pktfwd_req_flush_full_e)
 * -----------------------------------------------------------------------------
 */

void
wl_pktfwd_lut_clr(struct net_device * net_device)
{
    wl_if_t        * wlif;
    d3fwd_wlif_t   * d3fwd_wlif;
    wl_pktfwd_t    * wl_pktfwd = &wl_pktfwd_g;

    PKTFWD_FUNC();
    PKTFWD_ASSERT(wl_pktfwd->initialized == true);

    /* check if wlan virtual device */
    if (check_virt_wlan(net_device))
        return;

    wlif = WL_DEV_IF(net_device);

    if (wlif)
    {
        d3fwd_wlif = __wlif_2_d3fwd_wlif(wlif);
        PKTFWD_ASSERT(d3fwd_wlif != D3FWD_WLIF_NULL);
        PKTFWD_ASSERT(d3fwd_wlif->wlif == wlif);
        PKTFWD_ASSERT(d3fwd_wlif->net_device == net_device);

        PKTFWD_ASSERT(d3fwd_wlif->ucast_bmap == 0U);

        d3fwd_wlif->stations = 0;
        D3FWD_STATS_EXPR(
            memset(d3fwd_wlif->stats, 0, sizeof(d3fwd_wlif->stats)));
    }
    else
    {
        d3fwd_wlif = D3FWD_WLIF_NULL;
    }
    
    D3LUT_LOCK(wl_pktfwd->d3lut); // ++++++++++++++++++++++++++++++++++++++++++

    d3lut_clr(wl_pktfwd->d3lut, d3fwd_wlif, d3fwd_wlif == D3FWD_WLIF_NULL);

    D3LUT_UNLK(wl_pktfwd->d3lut); // ------------------------------------------

}   /* wl_pktfwd_lut_clr() */


/**
 * -----------------------------------------------------------------------------
 *
 * Function : Register a WLAN Interface with the PKTFWD subsystem
 *            Allocate a d3fwd_wlif object from the free list to extend
 *            the wlif with a PKTFWD presence. The d3fwd_wlif extension is
 *            paired with the wlif and the parent net_device.
 * Operation: wl_alloc_if, wl_alloc_linux_if allocates a 1:1 <wlif,net_device>
 *            net_device is linked to a wlif by an extension to the net_device
 *            using priv_link_t. wlif->dev is setup.
 *            A call is then made (via wl_pktc_init) to wl_pktfwd_wlif_ins() to
 *            further extend the pair <wlif,net_device> with a d3fwd_wlif_t.
 *            In the case of the primary interface (wl_attach), the interface
 *            may be registered with the pktfwd layer before the radio (wl_info)
 *            is registered. This means that the wl_info::wfd_idx may not be
 *            valid. (wl_pktfwd_t -1, i.e. value 2 in a d3fwd_wlif_r 2bit field)
 * -----------------------------------------------------------------------------
 */

int
wl_pktfwd_wlif_ins(wl_if_t * wlif)
{
    int prio;
    d3fwd_wlif_t * d3fwd_wlif;
    wl_pktfwd_t  * wl_pktfwd = &wl_pktfwd_g;

    PKTFWD_FUNC();
    PKTFWD_ASSERT(wlif != (wl_if_t *) NULL);
    PKTFWD_ASSERT(wlif->wl != (wl_info_t *) NULL);
    PKTFWD_ASSERT(wlif->dev != (struct net_device *) NULL);
    PKTFWD_ASSERT(wlif->d3fwd_wlif == D3FWD_WLIF_NULL);

    PKTFWD_ASSERT(wl_pktfwd->initialized == true);

    // osl_set_wlunit(wl->osh, wl->unit); DSLCPE

    PKTFWD_LOCK();  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if (dll_empty(&wl_pktfwd->d3fwd_free)) {
        PKTFWD_ERROR("wlif %s d3fwd_wlif pool depleted failure", wlif->name);
        d3fwd_wlif = D3FWD_WLIF_NULL;
        ++wl_pktfwd->stats.ops_failures;
        goto wl_pktfwd_wlif_ins_done; /* Unlock and bail */
    }

    /* Allocate a free d3fwd_wlif_t from free list and move to active list */
    d3fwd_wlif = (d3fwd_wlif_t *) dll_head_p(&wl_pktfwd->d3fwd_free);
    dll_delete(&d3fwd_wlif->node);
    dll_append(&wl_pktfwd->d3fwd_used, &d3fwd_wlif->node);
    wl_pktfwd->wlif_cnt++;

    d3fwd_wlif->osh         = wlif->wl->osh;    /* same osh as radio */
    d3fwd_wlif->wlif        = wlif;             /* pair with parent wl_if */
    d3fwd_wlif->net_device  = wlif->dev;        /* pair with grand parent dev */
    d3fwd_wlif->ucast_bmap  = 0U;
    d3fwd_wlif->wl_schedule = 0;                /* WLAN thread scheduled */
    d3fwd_wlif->unit        = wlif->wl->unit;   /* WLAN radio domain */

#if defined(BCM_WFD)
    /**
     * In the case of primary wlif, wl::wfd_idx is not valid. After, a radio is
     * registered (wl_pktfwd_radio_ins) and wfd is bound (wl_pktfwd_wfd_ins),
     * the primary interface's wfd_idx will be explicitly set.
     * For all other virtual interfaces, the wfd_idx is set here.
     */
    d3fwd_wlif->wfd_idx     = WL_WFD_IDX(wlif->wl->unit); /* maybe -1 */
#endif
    d3fwd_wlif->stations    = 0;
    dll_init(&d3fwd_wlif->mcast);
    for (prio = 0; prio < D3FWD_PRIO_MAX; ++prio)
        dll_init(&d3fwd_wlif->ucast[prio]);

    wlif->d3fwd_wlif        = (void *) d3fwd_wlif;

wl_pktfwd_wlif_ins_done:

    PKTFWD_UNLK();  // --------------------------------------------------------

    if (d3fwd_wlif == D3FWD_WLIF_NULL) {
        PKTFWD_ERROR("wlif %s failure", wlif->name);
        return WL_PKTFWD_FAILURE;
    }

    PKTFWD_TRACE("wlif %s success", wlif->name);

    return WL_PKTFWD_SUCCESS;

}   /* wl_pktfwd_wlif_ins() */


/**
 * -----------------------------------------------------------------------------
 * Function : Deregister a WLAN Interface with the PKTFWD subsystem.
 *            Locate the d3fwd_wlif extension, reset its state and free it.
 * -----------------------------------------------------------------------------
 */

void
wl_pktfwd_wlif_del(wl_if_t * wlif)
{
    d3fwd_wlif_t * d3fwd_wlif;
    wl_pktfwd_t  * wl_pktfwd = &wl_pktfwd_g;

    PKTFWD_FUNC();
    PKTFWD_ASSERT(wlif != (wl_if_t *) NULL);
    PKTFWD_ASSERT(wl_pktfwd->initialized == true);

    /* Fetch the wlif extension */
    d3fwd_wlif = __wlif_2_d3fwd_wlif(wlif);
    wlif->d3fwd_wlif = D3FWD_WLIF_NULL;

    if (d3fwd_wlif == D3FWD_WLIF_NULL) {
        PKTFWD_WARN("wlif %s not registered", wlif->name);
        return;
    }

    if (d3fwd_wlif->stations != 0) { /* cautionary warning */
        PKTFWD_WARN("wlif %s stations %u warning",
            wlif->name, d3fwd_wlif->stations);
    }

    /* Reset entire d3fwd_wlif state */
    wl_pktwd_d3fwd_wlif_reset(d3fwd_wlif);

    PKTFWD_LOCK();  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    dll_delete(&d3fwd_wlif->node); /* Move to free list */
    dll_append(&wl_pktfwd->d3fwd_free, &d3fwd_wlif->node);

    PKTFWD_UNLK();  // --------------------------------------------------------


    D3LUT_LOCK(wl_pktfwd->d3lut); // ++++++++++++++++++++++++++++++++++++++++++

    /* ignore_ext_match = false, implying clear elements with matching */
    d3lut_clr(wl_pktfwd->d3lut, d3fwd_wlif, /* ignore_ext_match = */ false);

    D3LUT_UNLK(wl_pktfwd->d3lut); // ------------------------------------------

}   /* wl_pktfwd_wlif_del() */


/**
 * -----------------------------------------------------------------------------
 * Function : Debug dump a WLAN Interface's PKTFWD context
 * -----------------------------------------------------------------------------
 */

void
wl_pktfwd_wlif_dbg(wl_if_t * wlif)
{
    d3fwd_wlif_t * d3fwd_wlif;

    PKTFWD_ASSERT(wl_pktfwd_g.initialized == true);

    if (wlif == (wl_if_t *) NULL)
        return;

    d3fwd_wlif = __wlif_2_d3fwd_wlif(wlif);

    if (d3fwd_wlif == D3FWD_WLIF_NULL) {
        printk("wlif %s dbg: not registered\n", wlif->name);
        return;
    }

    d3fwd_wlif_dump(d3fwd_wlif);

}   /* wl_pktfwd_wlif_dbg() */


/**
 * -----------------------------------------------------------------------------
 * Function : Register a WLAN Radio with the PKTFWD subsystem
 * Operation: Allocate a pktlist_context for the wlan (consumer). This context
 *            will be registered with WFD during wl_wfd_bind().
 *            Bind all exported hooks. Binding may be redundantly done again
 *            when a another radio goes through attach. 
 * -----------------------------------------------------------------------------
 */

void *
wl_pktfwd_radio_ins(wl_info_t * wl)
{
    uint16_t domain;
    int unit, mem_bytes;
    char wl_name[IFNAMSIZ]; /* eth%d overwrite */
#if defined(BCM_PKTFWD_FLCTL)
    uint32_t prio, dest;
    pktlist_fctable_t * pktlist_fctable;
#endif /* BCM_PKTFWD_FLCTL */
    pktqueue_t        * pktqueue;
    pktqueue_table_t  * pktqueue_table;
    pktlist_context_t * pktlist_context;
    wl_pktfwd_t       * wl_pktfwd = &wl_pktfwd_g;

    PKTFWD_FUNC();
    PKTFWD_ASSERT((wl != (wl_info_t *) NULL) && (wl->unit < WL_PKTFWD_RADIOS));
    PKTFWD_ASSERT(wl_pktfwd->initialized == true);

    unit = wl->unit;
    (void) snprintf(wl_name, sizeof(wl_name), "wl%d", unit);

#if defined(WLCFP)
    /* Configure WLAN radio d3lut pool policy */
    if (CFP_ENAB(wl->pub) == TRUE) {
        PKTFWD_TRACE("Pool #%d D3LUT_POLICY_POOL_BY_INDEX", wl->unit);
        d3lut_policy_set(wl_pktfwd->d3lut, wl->unit, D3LUT_POLICY_POOL_BY_INDEX);
    }
#endif /* WLCFP */

    /* Instantiate a WLAN consumer pktlist_context */
    PKTFWD_ASSERT(WL_PKTLIST_CONTEXT(unit) == PKTLIST_CONTEXT_NULL);

    pktlist_context = pktlist_context_init(
                    PKTLIST_CONTEXT_PEER_NULL, PKTLIST_CONTEXT_XFER_NULL,
                    PKTLIST_CONTEXT_KEYMAP_NULL, wl->dev, wl_name, unit);
    if (pktlist_context == PKTLIST_CONTEXT_NULL) {
        PKTFWD_WARN("pktlist_context unit %d failure", unit);
        goto wl_pktfwd_radio_ins_failure;
    }

    WL_PKTLIST_CONTEXT(unit) = pktlist_context;

#if defined(BCM_PKTFWD_FLCTL)

    /* Construct packet admission credits */
    mem_bytes = sizeof(pktlist_fctable_t);
    pktlist_fctable = (pktlist_fctable_t *) kmalloc(mem_bytes, GFP_ATOMIC);
    if (pktlist_fctable == PKTLIST_FCTABLE_NULL) {
        PKTFWD_WARN("pktlist_fctable unit %d failure", unit);
        goto wl_pktfwd_radio_ins_failure;
    }

    pktlist_context->fctable = pktlist_fctable;

    /* Will be set by WFD but should it be initialized to default VI, VO? */
    pktlist_fctable->pkt_prio_favor = ~0;

    for (prio = 0U; prio < PKTLIST_PRIO_MAX; ++prio)
    {
        for (dest = 0U; dest < PKTLIST_DEST_MAX; ++dest)
        {
            /* Reset credits of a pktlist */
            __pktlist_fctable_set_credits(pktlist_context, prio, dest, 0);
        }
    }

#endif /* BCM_PKTFWD_FLCTL */

    /* Construct WLAN producer pktqueue_table */
    mem_bytes = sizeof(pktqueue_table_t);
    pktqueue_table = (pktqueue_table_t *)kmalloc(mem_bytes, GFP_ATOMIC);
    if (pktqueue_table == PKTQUEUE_TABLE_NULL) {
        PKTFWD_WARN("pktqueue_table unit %d failure", unit);
        goto wl_pktfwd_radio_ins_failure;
    }

    memset(pktqueue_table, 0, mem_bytes);
    for (domain = 0; domain < PKTQUEUE_QUEUES_MAX; domain++)
    {
        pktqueue = PKTQUEUE_TBL_QUEUE(pktqueue_table, domain);
        /* Set packet type to SKB */
        pktqueue->NBuffPtrType = SKBUFF_PTR;    /* never reset */
        PKTQUEUE_RESET(pktqueue); /* head,tail, not reset */
    }

    WL_PKTQUEUE_TABLE(unit) = pktqueue_table;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
    /* Register dump hook with WLAN "pktc" module for iovar queries */
    wlc_dump_add_fns(wl->pub, "pktc",
                     (dump_fn_t)wl_pktfwd_radio_dbg,
                     (clr_fn_t)wl_pktfwd_sys_clr, (void *) wl);
#endif

    // osl_set_wlunit(wl->osh, unit); DSLCPE
#if defined(PKTC_TBL) && defined(WL_ALL_PASSIVE)
    wl->txq_txchain_dispatched = false; /* global flag for wl thread dispatch */
#endif
#if defined(BCM_BLOG)
    wl->pub->fcache = 1; /* enable fcache by default */
#endif

    /**
     * wl_wfd_bind() is not yet invoked, when wl_pktc_attach is invoked.
     * so pointless trying to setup wl_pktfwd_g::wfd_idx[], leave as -1.
     */
    PKTFWD_LOCK(); // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    wl_pktfwd->radio_cnt++;
    PKTFWD_UNLK(); // ---------------------------------------------------------

    PKTFWD_TRACE("wl%d register success", unit);

    return (void *) wl_pktfwd;

wl_pktfwd_radio_ins_failure:

    if (pktlist_context != PKTLIST_CONTEXT_NULL)
    {
#if defined(BCM_PKTFWD_FLCTL)
        if (pktlist_fctable != PKTLIST_FCTABLE_NULL)
        {
            mem_bytes = sizeof(pktlist_fctable_t);
            kfree(pktlist_fctable);
            pktlist_context->fctable = PKTLIST_FCTABLE_NULL;
        }
#endif /* BCM_PKTFWD_FLCTL */

        WL_PKTLIST_CONTEXT(unit) = pktlist_context_fini(pktlist_context);
    }

    PKTFWD_ERROR("wl%d register failure", unit);

    return (void *) NULL;

}   /* wl_pktfwd_radio_ins() */


/**
 * -----------------------------------------------------------------------------
 * Function : Unregister a WLAN radio with the PKTFWD subsystem
 *            Do not destruct the pktfwd if there are no more radios. This will
 *            be done in the wl_pktfwd_sys_fini (rmmod action).
 * -----------------------------------------------------------------------------
 */

void
wl_pktfwd_radio_del(wl_info_t * wl)
{
    int unit;
    pktlist_context_t * pktlist_context;
    pktqueue_table_t  * pktqueue_table;
#if defined(BCM_PKTFWD_FLCTL)
    pktlist_fctable_t * pktlist_fctable;
#endif /* BCM_PKTFWD_FLCTL */
    wl_pktfwd_t       * wl_pktfwd = &wl_pktfwd_g;

    PKTFWD_FUNC();
    PKTFWD_ASSERT((wl != (wl_info_t *) NULL) && (wl->unit < WL_PKTFWD_RADIOS));
    PKTFWD_ASSERT(wl_pktfwd->initialized == true);

    unit = wl->unit;
    pktlist_context = WL_PKTLIST_CONTEXT(unit);

#if defined(BCM_PKTFWD_FLCTL)
    /* Destruct packet admission credits */
    pktlist_fctable = pktlist_context->fctable;
    if (pktlist_fctable != PKTLIST_FCTABLE_NULL)
    {
        kfree(pktlist_fctable);
        pktlist_context->fctable = PKTLIST_FCTABLE_NULL;

        /* FIXME: WFD fctable will point to stale pointer; Invalidate it. */

    }
#endif /* BCM_PKTFWD_FLCTL */

    WL_PKTLIST_CONTEXT(unit) = pktlist_context_fini(pktlist_context);

    /* Destruct pktqueue_table */
    pktqueue_table = WL_PKTQUEUE_TABLE(unit);
    WL_PKTQUEUE_TABLE(unit) = PKTQUEUE_TABLE_NULL;

    if (pktqueue_table != PKTQUEUE_TABLE_NULL)
    {
        int mem_bytes = sizeof(pktqueue_table_t);
        memset(pktqueue_table, 0xFF, mem_bytes); /* scribble */
        kfree(pktqueue_table);
    }

    PKTFWD_LOCK();
    WL_WFD_IDX(unit) = -1;
    wl_pktfwd->radio_cnt--;
    PKTFWD_UNLK();

    PKTFWD_TRACE("wl%d deregistered", unit);

}   /* wl_pktfwd_radio_del() */


/**
 * -----------------------------------------------------------------------------
 * Function : Dump the WLAN Radio specific PKTFWD state
 * -----------------------------------------------------------------------------
 */

void
wl_pktfwd_radio_dbg(wl_info_t * wl, struct bcmstrbuf * b)
{
    int  dev, unit;
    bool dump_verbose;
    d3fwd_wlif_t      * d3fwd_wlif;
    pktlist_context_t * pktlist_context;
    wl_pktfwd_t       * wl_pktfwd = &wl_pktfwd_g;

    dump_verbose = false;

    _wl_pktfwd_sys_dump();

    PKTFWD_ASSERT(wl_pktfwd->initialized == true);

    if ((wl == NULL) || (wl->unit >= WL_PKTFWD_RADIOS)) {
        PKTFWD_ERROR("wl %p invalid", wl);
        return;
    }

    unit = wl->unit;

#if defined(BCM_WFD)
    wl_pktfwd_wfd_dbg(wl);
#endif

    /* Dump all active devices (interfaces registered with pktfwd) for radio */
    d3fwd_wlif = wl_pktfwd->d3fwd_wlif;
    for (dev = 0; dev < WL_PKTFWD_DEVICES; ++dev, ++d3fwd_wlif)
    {   /* Do not traverse the d3fwd_free dll as no lock is taken */
        if (d3fwd_wlif->unit == unit) {
            wl_pktfwd_wlif_dbg(d3fwd_wlif->wlif);
        }
    }

    pktlist_context = WL_PKTLIST_CONTEXT(unit);
    if (pktlist_context == PKTLIST_CONTEXT_NULL) {
        PKTFWD_ERROR("radio unit %u pktlist_context invalid", unit);
        return;
    }

#if (CC_PKTFWD_DEBUG >= 1)
    dump_verbose = true;
#endif
    pktlist_context_dump(pktlist_context, false, dump_verbose);

}   /* wl_pktfwd_radio_dbg() */


#if defined(BCM_WFD)
/**
 * -----------------------------------------------------------------------------
 * Function : Invoked on wl_wfd_bind() to register a wfd_idx with PKTFWD. The
 *            primary wlif's d3fwd_wlif object is updated with the wfd_idx.
 * -----------------------------------------------------------------------------
 */

void
wl_pktfwd_wfd_ins(wl_info_t * wl, int wfd_idx)
{
    int unit;
    d3fwd_wlif_t * d3fwd_wlif; /* primary interface */

    PKTFWD_FUNC();
    PKTFWD_ASSERT(wl != (wl_info_t *) NULL);
    PKTFWD_ASSERT(wfd_idx >= 0);

    unit = wl->unit;
    PKTFWD_ASSERT(unit < WL_PKTFWD_RADIOS);

    d3fwd_wlif = __wl_2_d3fwd_wlif(wl);

    if (unit != wfd_idx)
        PKTFWD_ERROR("wl u%d mismatch wfd_idx %d\n", unit, wfd_idx);

    if (d3fwd_wlif == D3FWD_WLIF_NULL) {
        PKTFWD_ERROR("wl%d no primary interface", unit);
        return;
    }

    PKTFWD_LOCK(); // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    WL_WFD_IDX(unit)    = wfd_idx;

    /* Fix up the primary interfaces wfd_idx, now */
    d3fwd_wlif->wfd_idx = (uint8_t) wfd_idx; /* actually 4 bits */

    PKTFWD_UNLK(); // ---------------------------------------------------------

    PKTFWD_TRACE("wl%d wfd_idx %d register success", unit, wfd_idx);

}   /* wl_pktfwd_wfd_ins() */


/**
 * -----------------------------------------------------------------------------
 * Function : Invoked on wl_wfd_unbind() to de-register the wfd_idx for a radio.
 * -----------------------------------------------------------------------------
 */

void
wl_pktfwd_wfd_del(wl_info_t * wl)
{
    int unit;

    PKTFWD_FUNC();
    PKTFWD_ASSERT(wl != (wl_info_t *)NULL);
    unit = wl->unit;
    PKTFWD_ASSERT(unit < WL_PKTFWD_RADIOS);

    PKTFWD_LOCK();
    WL_WFD_IDX(unit) = -1;
    PKTFWD_UNLK();

    PKTFWD_TRACE("wl%d wfd_idx deregister", unit);

}   /* wl_pktfwd_wfd_del() */


/**
 * -----------------------------------------------------------------------------
 * Function :
 * -----------------------------------------------------------------------------
 */

void
wl_pktfwd_wfd_dbg(wl_info_t * wl)
{
    // use wl->wfd_idx to invoke a wfd_dump by wfd_idx ... not available.
    if ((wl != (wl_info_t *) NULL) && (wl->unit < WL_PKTFWD_RADIOS))
        printk("\t wl%d wfd_idx %d pktfwd %u\n", wl->unit, wl->wfd_idx,
                WL_WFD_IDX(wl->unit));

}   /* wl_pktfwd_wfd_dbg() */

#endif /* BCM_WFD */

/**
 * -----------------------------------------------------------------------------
 * Function : Construct all pktfwd subsystems. This function is not re-entrant.
 *            Invoked in wl_attach (see wl_linux.c), per radio.
 *
 * Initialize the D3LUT system with a pool of D3LUT_ELEM_TOT elements. When
 * WLCFP is enabled, all WLAN element pools will use the allocate by-index and
 * the last LAN pool will use by-freelist. This allows for a CFP flowid to be
 * paired by-value with the element's per domain key::index.
 *
 * Initialize the pool of d3fwd_wlif(s) for all radios.
 *
 * -----------------------------------------------------------------------------
 */

int
wl_pktfwd_sys_init(void)
{
    int dev, mem_bytes;
    d3fwd_wlif_t * d3fwd_wlif;
    wl_pktfwd_t  * wl_pktfwd = &wl_pktfwd_g;

    PKTFWD_FUNC();

    if (wl_pktfwd->initialized == true) /* global system already setup, bail */
    {
        PKTFWD_ERROR("already initialized");
        return WL_PKTFWD_SUCCESS;
    }

    /*
     * ---------------------------------------------
     * Section: Initialize the PKTFWD global objects
     * ---------------------------------------------
     */
    spin_lock_init(&wl_pktfwd->lock);

    wl_pktfwd->d3lut = d3lut_gp;

    /* Initialize the pool of d3fwd_wlif_t objects */
    dll_init(&wl_pktfwd->d3fwd_used);
    dll_init(&wl_pktfwd->d3fwd_free);

    /* Pre-allocate pool of d3fwd_wlif_t and add each d3fwd_wlif to free list */
    mem_bytes = sizeof(d3fwd_wlif_t) * WL_PKTFWD_DEVICES;
    wl_pktfwd->d3fwd_wlif = (d3fwd_wlif_t *) kmalloc(mem_bytes, GFP_ATOMIC);
    if (wl_pktfwd->d3fwd_wlif == D3FWD_WLIF_NULL) {
        PKTFWD_ERROR("d3fwd_wlif kmalloc %d failure", mem_bytes);
        goto wl_pktfwd_sys_init_failure;
    }
    memset(wl_pktfwd->d3fwd_wlif, 0, mem_bytes);

    d3fwd_wlif = wl_pktfwd->d3fwd_wlif; /* d3fwd_wlif pool base */
    for (dev = 0; dev < WL_PKTFWD_DEVICES; ++dev, ++d3fwd_wlif)
    {
        _wl_pktwd_d3fwd_wlif_reset(d3fwd_wlif); /* reset scribble */
        dll_init(&d3fwd_wlif->node); /* place into free list */
        dll_append(&wl_pktfwd->d3fwd_free, &d3fwd_wlif->node);
    }

#if defined(WL_PKTFWD_TXEVAL)
    {
        int domain;
        /* Initialize dispatch reentrancy per radio */
        for (domain = 0; domain < WL_PKTFWD_RADIOS; ++domain)
        {
            WL_DISPATCH_INIT(domain);
        }
    }
#endif  /* WL_PKTFWD_TXEVAL */

#if defined(WL_PKTFWD_RUNQ)
    {
        int domain, prio;
        wl_pktfwd_runq_t *runq;
        /* Initialize run queue per radio */
        for (domain = 0; domain < WL_PKTFWD_RADIOS; ++domain)
        {
            runq             = WL_RUNQ_P(domain);
            runq->d3fwd_wlif = D3FWD_WLIF_NULL;
            runq->credits    = 0;
            runq->ucast_bmap = 0U;
            for (prio = 0; prio < D3FWD_PRIO_MAX; ++prio)
            {
                dll_init(&runq->ucast[prio]);
            }
        }
    }
#endif  /* WL_PKTFWD_RUNQ */

    /*
     * ---------------------------------------------------------------
     * Section: Initialize the global network stack interfaces (hooks)
     * ---------------------------------------------------------------
     */
#if defined(PKTC)
    /* register with bridge and wlc */
    wl_pktc_req_hook = wl_pktfwd_request;
#endif
#if defined(BCM_BLOG)
    /* register hook with Blog Shim */
    wl_pktc_del_hook = (wl_pktfwd_del_hook_t) wl_pktfwd_lut_del;
#endif
    /* register bridge refresh */
    fdb_check_expired_wl_hook = (wl_pktfwd_hit_hook_t) wl_pktfwd_lut_hit;

    {
        /* toggle pktfwd acceleration in dwds via nvram */
        char *var_ap = NULL;
        char *var_sta = NULL;
        var_ap = nvram_get(NVRAM_DWDS_AP_PKTFWD_ACCEL);
        if (var_ap != NULL)
            wl_dwds_ap_pktfwd_accel = bcm_strtoul(var_ap, NULL, 0);
        var_sta = nvram_get(NVRAM_DWDS_STA_PKTFWD_ACCEL);
        if (var_sta != NULL)
            wl_dwds_sta_pktfwd_accel = bcm_strtoul(var_sta, NULL, 0);
    }

    wl_pktfwd->initialized = true;

    PKTFWD_PRINT(PKTFWD_VRP_FMT " Success",
           PKTFWD_VRP_VAL(WL_PKTFWD, WL_PKTFWD_VERSIONCODE));

    return WL_PKTFWD_SUCCESS;

wl_pktfwd_sys_init_failure:

    PKTFWD_ERROR(PKTFWD_VRP_FMT " System Construction Failure",
           PKTFWD_VRP_VAL(WL_PKTFWD, WL_PKTFWD_VERSIONCODE));

    wl_pktfwd_sys_fini();

    return WL_PKTFWD_FAILURE;

}   /* wl_pktfwd_sys_init() */


/**
 * -----------------------------------------------------------------------------
 * Function : Destructor for the PKTFWD global system.
 *            Invoked by wl_module_exit() and when wl_pktfwd_sys_init() fails.
 * TBD      : Evaluate race conditions on exposed hooks.
 * -----------------------------------------------------------------------------
 */

void
wl_pktfwd_sys_fini(void)
{
    d3fwd_wlif_t * d3fwd_wlif;
    wl_pktfwd_t  * wl_pktfwd = &wl_pktfwd_g;

    PKTFWD_FUNC();

#if defined(BCM_BLOG)
    wl_pktc_del_hook = NULL;
#endif
#if defined(PKTC)
    wl_pktc_req_hook = NULL;
#endif
    fdb_check_expired_wl_hook = NULL;

    if (wl_pktfwd->initialized == false) {
        if (wl_pktfwd->wlif_cnt)
            PKTFWD_WARN("wlif_cnt %u", wl_pktfwd->wlif_cnt);
        if (wl_pktfwd->radio_cnt)
            PKTFWD_WARN("radio_cnt %u", wl_pktfwd->radio_cnt);
        // PKTFWD_ASSERT(wl_pktfwd->wlif_cnt == 0);
        // PKTFWD_ASSERT(wl_pktfwd->radio_cnt == 0);
    }

    wl_pktfwd->initialized = false;

    wl_pktfwd->d3lut = D3LUT_NULL;

    dll_init(&wl_pktfwd->d3fwd_used);
    dll_init(&wl_pktfwd->d3fwd_free);
    d3fwd_wlif = wl_pktfwd->d3fwd_wlif;     /* d3fwd_wlif pool */
    wl_pktfwd->d3fwd_wlif = D3FWD_WLIF_NULL;

    wl_pktfwd->wlif_cnt  = (int8_t)0;
    wl_pktfwd->radio_cnt = (int8_t)0;

    if (d3fwd_wlif != D3FWD_WLIF_NULL)      /* Destruct the d3fwd_wlif pool */
    {
        int mem_bytes = sizeof(d3fwd_wlif_t) * WL_PKTFWD_DEVICES;
        memset(d3fwd_wlif, 0xFF, mem_bytes);
        kfree(d3fwd_wlif);
    }

    PKTFWD_PRINT(PKTFWD_VRP_FMT " Complete",
           PKTFWD_VRP_VAL(WL_PKTFWD, WL_PKTFWD_VERSIONCODE));
}   /* wl_pktfwd_sys_fini() */


/**
 * -----------------------------------------------------------------------------
 * Function : Dump all global subsystems. LOCKLESS
 * -----------------------------------------------------------------------------
 */

void
wl_pktfwd_sys_dump(void)
{
    int dev;
    d3fwd_wlif_t * d3fwd_wlif;
    wl_pktfwd_t  * wl_pktfwd = &wl_pktfwd_g;

    _wl_pktfwd_sys_dump();

    if (wl_pktfwd->initialized == false) /* global system not setup */
        return;

    /* Dump all active devices (INTERFACES registered with pktfwd) */
    d3fwd_wlif = wl_pktfwd->d3fwd_wlif; /* lockless walk of pool */
    for (dev = 0; dev < WL_PKTFWD_DEVICES; ++dev, ++d3fwd_wlif)
    {
        if (d3fwd_wlif->unit != ~0)
            d3fwd_wlif_dump(d3fwd_wlif);
    }

    /* Dump entire D3 Lookup Table (STATIONS registered with pktfwd) */
    d3lut_dump(wl_pktfwd->d3lut);

    /* Dump all pktlist_context instances (PACKETS transferred via pktfwd) */
    pktlist_context_dump_all();

    /* Dump all pktqueue_context instances (PACKETS transferred via pktfwd) */
    pktqueue_context_dump_all();

}   /* wl_pktfwd_sys_dump() */


/**
 * -----------------------------------------------------------------------------
 * Function : Helper clear function registered with wlan module
 * -----------------------------------------------------------------------------
 */
void
wl_pktfwd_sys_clr(wl_info_t * wl)
{
    int dev, unit;
    d3fwd_wlif_t * d3fwd_wlif;
    wl_pktfwd_t  * wl_pktfwd = &wl_pktfwd_g;

    if (wl_pktfwd->initialized == false) /* global system not setup */
        return;

    unit = wl->unit;

    memset(&wl_pktfwd->stats, 0, sizeof(wl_pktfwd_stats_t));

    /* Clear all active device's pktfwd stats */
    d3fwd_wlif = wl_pktfwd->d3fwd_wlif; /* lockless walk of pool */
    for (dev = 0; dev < WL_PKTFWD_DEVICES; ++dev, ++d3fwd_wlif)
    {
        if (d3fwd_wlif->unit == unit)
            d3fwd_wlif_clr(d3fwd_wlif);
    }

    d3lut_stats_clr(wl_pktfwd->d3lut);

}   /* wl_pktfwd_sys_clr() */


/**
 * =============================================================================
 * Section: Transmit Packet Processing
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 * Function : Callback function registered with WFD pktlist_context.
 *
 * Operation: WFD uses it's pktlist_context to bin packets into ucast and mcast
 * pktlists which are placed into pending-xfer work lists. Once all packets are
 * binned, WFD will xfer all pktlists to corresponding bin's in peer (WLAN)
 * pktlist_context. Once all pending xfer work lists are processed, WFD will
 * invoke wl_pktfwd_xfer_callback().
 * wl_pktfwd_xfer_callback() is responsible for moving all pending work lists in
 * the WLAN pktlist_context to the appropriate d3fwd_wlif and the d3fwd_wlif's
 * network device needs to be dispatched.        
 * wl_pktfwd_xfer_callback() is invoked in the "WFD" thread context.
 *
 * Helper   : Helper functions used by wl_pktfwd_xfer_callback() are,
 *      wl_pktfwd_xfer_pktlist_test() : Test for stale packets in pktlist
 *      wl_pktfwd_xfer_pktlist_free() : Free all packets in pktlist
 *      wl_pktfwd_xfer_pktlist()      : Move a pktlist from worklist to a
 *                                      d3fwd_wlif's worklist
 *      wl_pktfwd_xfer_work_ucast()   : xfer ucast worklists
 *      wl_pktfwd_xfer_work_mcast()   : xfer mcast worklists
 *
 * -----------------------------------------------------------------------------
 */

/** Test if packets in pktlist_t have a stale key (incarnation) mismatch */
static inline bool
wl_pktfwd_xfer_pktlist_test(pktlist_t * pktlist_pkts, d3lut_elem_t * d3lut_elem,
                            pktlist_t * pktlist_free, bool dump_error)
{
    pktfwd_key_t head_key, tail_key, elem_key;

    head_key.v16 = PKTLIST_PKT_KEY(pktlist_pkts->head, SKBUFF_PTR);
    tail_key.v16 = PKTLIST_PKT_KEY(pktlist_pkts->tail, SKBUFF_PTR);
    elem_key.v16 = d3lut_elem->key.v16;

    // pragma message "TODO: if tail matches, flush stale into pktlist_free"

    if ((head_key.v16 ^ tail_key.v16) | (tail_key.v16 ^ elem_key.v16))
    {
        if (dump_error == true)
        {
            PKTFWD_WARN("key mismatch: "
                "head " D3LUT_KEY_FMT "tail " D3LUT_KEY_FMT
                "elem " D3LUT_KEY_FMT "ext.flowd " D3LUT_KEY_FMT,
                D3LUT_KEY_VAL(head_key), D3LUT_KEY_VAL(tail_key),
                D3LUT_KEY_VAL(elem_key), D3LUT_KEY_VAL(d3lut_elem->ext.flow_key));
#if (CC_D3FWD_DEBUG >= 3)
            d3fwd_ext_dump(&d3lut_elem->ext);
#endif
        }

        return true; /* pktlist needs to be dropped */
    }

    return false;

}   /* wl_pktfwd_xfer_pktlist_test() */


static void /* Move all packets to be dropped into pktlist_free */
wl_pktfwd_xfer_pktlist_free(pktlist_context_t * wl_pktlist_context,
                        pktlist_elem_t * pktlist_elem, pktlist_t * pktlist_free)
{
    pktlist_t   * pktlist_pkts;
    wl_pktfwd_t * wl_pktfwd = &wl_pktfwd_g;

    pktlist_pkts = &pktlist_elem->pktlist;

    PKTFWD_WARN("%s %d", wl_pktlist_context->driver_name, pktlist_free->len);

    /* xfer all packets from pktlist_pkts to tail of pktlist_free */
    if (pktlist_free->len == 0U)
    {
        pktlist_free->head                  = pktlist_pkts->head;
        pktlist_free->tail                  = pktlist_pkts->tail;
    }
    else
    {
        PKTLIST_PKT_SET_SLL(pktlist_free->tail, pktlist_pkts->head, SKBUFF_PTR);
        pktlist_free->tail                  = pktlist_pkts->tail;
    }

    pktlist_free->len             += pktlist_pkts->len;
    wl_pktfwd->stats.pkts_dropped += pktlist_pkts->len;

    PKTLIST_RESET(pktlist_pkts); /* head,tail, not reset */

    /* Move pktlist_elem to the WLAN pktlist_context's free list */
    dll_delete(&pktlist_elem->node);
    dll_append(&wl_pktlist_context->free, &pktlist_elem->node);
    
    return;

}   /* wl_pktfwd_xfer_pktlist_free() */


static inline bool
wl_pktfwd_xfer_pktlist(pktlist_context_t * wl_pktlist_context,
    pktlist_elem_t * pktlist_elem, d3lut_elem_t * d3lut_elem,
    pktlist_t * pktlist_free)
{
    bool join;
    uint32_t prio, dest;
    pktlist_t    * pktlist_pkts;
    d3fwd_wlif_t * d3fwd_wlif;

    PKTFWD_PFUNC();

    pktlist_pkts = &pktlist_elem->pktlist;

    join = true;
    dest = pktlist_pkts->dest;
    prio = pktlist_pkts->prio;

    PKTFWD_LOCK();  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    /* Audit pktlists head and tail packet's key::incarnation with elem */
    if (wl_pktfwd_xfer_pktlist_test(pktlist_pkts, d3lut_elem,
                                    pktlist_free, true)) {
        join = false;
        PKTFWD_WARN("stale pkts detected\n");
        goto wl_pktc_handoff_pktlist_join_done; /* allow for partial drop? */
    }

    d3fwd_wlif = d3lut_elem->ext.d3fwd_wlif;
    if (d3fwd_wlif == D3FWD_WLIF_NULL) {
        join = false;
        PKTFWD_WARN("d3fwd_wlif deleted\n");
        goto wl_pktc_handoff_pktlist_join_done;
    }

    PKTFWD_ASSERT(d3fwd_wlif->wlif != (wl_if_t *) NULL);
    PKTFWD_ASSERT(D3LUT_ELEM_IDX(d3lut_elem->ext.flow_key.index) == dest);

    d3lut_elem->ext.hit = 1;

    /* FIXME: If pktlist is in runq worklist, it will be moved back to
     * d3fwd_wlif ucast worklist */
    dll_delete(&pktlist_elem->node);
    dll_append(&d3fwd_wlif->ucast[prio], &pktlist_elem->node);

    D3FWD_STATS_EXPR(
        if ((d3fwd_wlif->ucast_bmap & PKTFWD_UCAST_BMAP(prio)) == 0)
        {
            D3FWD_STATS_ADD(d3fwd_wlif->stats[prio].schedule, 1);
            PKTFWD_PTRACE("schedule %d dest %d prio %d pkts %d",
                d3fwd_wlif->stats[prio].schedule, dest, prio, pktlist_pkts->len);
        }
    )

    d3fwd_wlif->ucast_bmap |= PKTFWD_UCAST_BMAP(prio);

    if (d3fwd_wlif->wl_schedule == 0)
    {
        d3fwd_wlif->wl_schedule = ~0;
        _wl_schedule_work(d3fwd_wlif->wlif->wl);
    }

wl_pktc_handoff_pktlist_join_done:
    PKTFWD_UNLK();  // --------------------------------------------------------

    return join;

}   /* wl_pktfwd_xfer_pktlist() */


static inline void
wl_pktfwd_xfer_work_ucast(pktlist_context_t * wl_pktlist_context,
                          dll_t * uc_work, pktlist_t * pktlist_free)
{
    dll_t  * item, * next;
    pktlist_elem_t * pktlist_elem;
    d3lut_elem_t   * d3lut_elem;

    PKTFWD_PFUNC();

    /* Traverse a pktlist_context's ucast pending work list */
    for (item = dll_head_p(uc_work); ! dll_end(uc_work, item); item = next)
    {
        next = dll_next_p(item);    /* iterator's next */
        pktlist_elem = _envelope_of(item, pktlist_elem_t, node);

        /* pktlist_pkts had adopted first pkt's key, fetch the d3lut_elem */
        d3lut_elem = __d3lut_key_2_d3lut_elem(pktlist_elem->pktlist.key.v16);

        if ( ! wl_pktfwd_xfer_pktlist(wl_pktlist_context, pktlist_elem,
                                      d3lut_elem, pktlist_free))
        {
            goto wl_pktfwd_xfer_work_ucast_drop; /* fail d3fwd_wlif xfer */
        }

        continue; /* next pktlist in uncat worklist */

wl_pktfwd_xfer_work_ucast_drop: /* xfer all pkts to pktlist_free */
        wl_pktfwd_xfer_pktlist_free(wl_pktlist_context,
                                    pktlist_elem, pktlist_free);

    } /* for each pktlist_elem in uc_work list */

}   /* wl_pktfwd_xfer_work_ucast() */

/**
 * -----------------------------------------------------------------------------
 * Function     : wl_pktfwd_mcast_pktlist_xmit()
 * Description  : Make a copy of packets from multicast pktlist and trasfer to
 *                the network interfaces provided in the ssid_vector bitmap.
 *                For the last interface in ssid_vector, transmit the original
 *                packet.
 *
 * CAUTION : Invoked in "WFD" thread context
 * -----------------------------------------------------------------------------
 */
static inline void
wl_pktfwd_mcast_pktlist_xmit(
    pktlist_context_t * wl_pktlist_context,
    pktlist_t         * pktlist_mcast,
    pktlist_t         * pktlist_free)
{
    uint16_t ssid_vector, subunit;
    wl_info_t         * wl;
    pktlist_pkt_t     * pkt;
    struct sk_buff    * skb;
    struct net_device * net_device;
    xmit_fn_t           netdev_ops_xmit;
    wl_pktfwd_t       * wl_pktfwd = &wl_pktfwd_g;

    wl = WL_INFO_GET(wl_pktlist_context->driver);

    PKTFWD_ASSERT(pktlist_mcast->len != 0U);

    while (pktlist_mcast->len)
    {
        pkt                 = pktlist_mcast->head;
        pktlist_mcast->head = PKTLIST_PKT_SLL(pkt, SKBUFF_PTR);
        PKTLIST_PKT_SET_SLL(pkt, PKTLIST_PKT_NULL, SKBUFF_PTR);
        pktlist_mcast->len--;

        ssid_vector = ((struct sk_buff *) pkt)->wl.mcast.ssid_vector;

        while (ssid_vector)
        {
            subunit = pktfwd_map_ssid_vector_to_ssid(&ssid_vector);
            net_device = __subunit_2_net_device(wl, subunit);

            netdev_ops_xmit = (xmit_fn_t)(net_device->netdev_ops->ndo_start_xmit);

            PKTFWD_ASSERT(net_device != NULL);

            if (ssid_vector) /* Don't make a copy for only/last interface */
            {
                /* skb copy */
                skb = skb_copy(pkt, GFP_ATOMIC);

                if ( unlikely(skb ==(struct sk_buff *) NULL))
                {
                    PKTFWD_ERROR("skb copy failed; dropping packet<%p>", pkt);
                    wl_pktfwd->stats.pkts_dropped++;

                    /* Add it to pktlist_free */
                    if (pktlist_free->len != 0U) /* Do not use <head,tail> */
                    {
                        /* Append to tail */
                        PKTLIST_PKT_SET_SLL(pktlist_free->tail, pkt, SKBUFF_PTR);
                        pktlist_free->tail  = pkt;
                    }
                    else
                    {
                        /* Add packet to head */
                        pktlist_free->head = pktlist_free->tail = pkt;
                    }
                    ++pktlist_free->len;
                    break;
                }
            }
            else
            {
                skb = (struct sk_buff *) pkt;
            }

            PKTFWD_PTRACE("wl%d skb %p", wl->unit, skb);
            wl_pktfwd->stats.txf_chn_pkts++;

            skb->dev  = net_device;

            /* Add it to WLAN xmit queue and schedule WLAN thread */
            netdev_ops_xmit(skb, net_device);   /* wl_start() */

        }   /* while (ssid_vector) */
    } /* while (pktlist_mcast->len) */


}   /* wl_pktfwd_mcast_pktlist_xmit() */


static inline void
wl_pktfwd_xfer_work_mcast(pktlist_context_t * wl_pktlist_context,
                          dll_t * mc_work, pktlist_t * pktlist_free)
{
    dll_t  * item, * next;
    pktlist_t      * pktlist_mcast;
    pktlist_elem_t * pktlist_elem;

    PKTFWD_PFUNC();

    for (item = dll_head_p(mc_work); ! dll_end(mc_work, item); item = next)
    {
        next = dll_next_p(item);        /* iterator's next */
        pktlist_elem = _envelope_of(item, pktlist_elem_t, node);
        pktlist_mcast = &pktlist_elem->pktlist;

        wl_pktfwd_mcast_pktlist_xmit(wl_pktlist_context,
                                     pktlist_mcast, pktlist_free);

    }

}   /* wl_pktfwd_xfer_work_mcast() */


void /* Hook (peer) invoked by WFD to transfer all accumulated pktlists */
wl_pktfwd_xfer_callback(pktlist_context_t * wl_pktlist_context)
{
    uint32_t prio;
    dll_t * worklist;
    pktlist_t pktlist_free; /* local pktlist, for lockless kfree_skb */

    PKTFWD_PFUNC();

    PKTLIST_RESET(&pktlist_free); /* len = 0U, key.v16 = don't care */

    PKTLIST_LOCK(wl_pktlist_context);  // +++++++++++++++++++++++++++++++++++++

    /* Transfer "all" mcast pktlist(s) to each interface's mcast worklist */
    worklist = &wl_pktlist_context->mcast;
    if ( ! dll_empty(worklist) )
    {
        wl_pktfwd_xfer_work_mcast(wl_pktlist_context, worklist, &pktlist_free);
    }

    /* Transfer "all" ucast pktlist(s) to each interface's ucast worklist */
    for (prio = 0U; prio < PKTLIST_PRIO_MAX; ++prio)
    {
        worklist = &wl_pktlist_context->ucast[prio];
        if ( ! dll_empty(worklist) )
        {
            wl_pktfwd_xfer_work_ucast(wl_pktlist_context,
                                      worklist, &pktlist_free);
        }
    }

    PKTLIST_UNLK(wl_pktlist_context);  // -------------------------------------


    /* Now "lockless", kfree any packets that needed to be dropped */
    if (pktlist_free.len)
    {
        wl_pktwd_pktlist_free(wl_pktlist_context, &pktlist_free);
    }

}   /* wl_pktfwd_xfer_callback */


/**
 * =============================================================================
 * Section WLAN Packet Forwarding
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 * WLAN Transmit Path Forwarding
 *
 * Function : Dispatch the accumulated packet lists via CFP or as pkt_chaining.
 * -----------------------------------------------------------------------------
 */

#if defined(WLCFP)
static const union {
    uint32_t        u32;
    uint16_t        u16[2];
    uint8_t         u8[4];
} __ctl_oui3 = { .u8 = {0x00, 0x00, 0x00, 0x03} };

/** LLCSNAP: OUI[2] setting for Bridge Tunnel (Apple ARP and Novell IPX) */
#define ETHER_TYPE_APPLE_ARP    (0x80f3) /* Apple Address Resolution Protocol */
#define ETHER_TYPE_NOVELL_IPX   (0x8137) /* Novel IPX Protocol */
#define BRIDGE_TUNNEL_OUI2      (0xf8)   /* OUI[2] value for Bridge Tunnel */
#define IS_BRIDGE_TUNNEL(etype) \
    (((etype) == ETHER_TYPE_APPLE_ARP) || ((etype) == ETHER_TYPE_NOVELL_IPX))

/**
 * Copy Ethernet header alongwith LLCSNAP.
 * Handcoded construction of dot3_mac_llc_snap_header.
 *    6B 802.3 MacDA
 *    6B 802.3 MacSA
 *    2B Ethernet Data Length (includes 8B LLCSNAP header length)
 *    2B LLC DSAP SSAP
 *    1B LLC CTL
 *    3B SNAP OUI
 *    2B EtherType
 */
static inline void
wl_pktfwd_llc_snap_insert(osl_t * osh, struct sk_buff * skb)
{
    uint8_t * src_eth, * dst_eth;
    uint16_t ether_type, ether_payload_len;

    src_eth = (uint8_t *) PKTDATA(osh, skb);
    ether_type = ntohs(((struct ether_header *)src_eth)->ether_type);
    if (ether_type < ETHER_TYPE_MIN)
        return;
    dst_eth = PKTPUSH(osh, skb, DOT11_LLC_SNAP_HDR_LEN);
    ether_payload_len = PKTLEN(osh, skb) - ETHER_HDR_LEN;

    /* Copy 12B ethernet header: 32bit aligned source and destination. */
    ((uint32_t *)(dst_eth))[0] = ((const uint32_t *)(src_eth))[0];
    ((uint32_t *)(dst_eth))[1] = ((const uint32_t *)(src_eth))[1];
    ((uint32_t *)(dst_eth))[2] = ((const uint32_t *)(src_eth))[2];
    /* ethernet payload length: 2B */
    ((uint16_t *)(dst_eth))[6] = htons(ether_payload_len);
    /* Reference bcm_proto.h: static const uint8 llc_snap_hdr[] */
    /* dsap = 0xaa ssap = 0xaa: 2B copy */
    ((uint16_t *)(dst_eth))[7] = (uint16_t)0xAAAA; /* no need for htons */
    /* LLC ctl = 0x03, out[3] = { 0x00 0x00 0x00}: 32b aligned 4B copy */
    ((uint32_t *)(dst_eth))[4] = htonl(__ctl_oui3.u32);
    if (IS_BRIDGE_TUNNEL(ether_type)) { /* Set OUI[2] for Bridge Tunnel */
        ((struct dot3_mac_llc_snap_header*)(dst_eth))->oui[2] =
            BRIDGE_TUNNEL_OUI2;
    }
    /* Original EtherType is already in place */

}   /* wl_pktfwd_llc_snap_insert() */


/**
 * Transpose a pktlist into a CFP compliant pkt list.
 *
 * Returns a CFP capable packet list, prepared: PKTTAG and LLCSNAP
 */
static inline void
wl_pktfwd_pktlist_cfp(wl_info_t * wl, struct net_device * net_device,
                      wl_pktfwd_pktlist_t * wl_pktfwd_pktlist)
{
    struct sk_buff * skb, * skb_sll;
    uint32_t  cfp_exptime;

    PKTFWD_PFUNC();

    skb = wl_pktfwd_pktlist->head;
    bcm_prefetch(skb); /* before D11_TSFTimerLow access */

    cfp_exptime = wlc_cfp_exptime(wl->wlc); /* D11_TSFTimerLow */

    while (skb != (struct sk_buff *) NULL)
    {
        skb_sll = PKTLIST_PKT_SLL(skb, SKBUFF_PTR);
        PKTFWD_ASSERT(PKTLIST_PKT_SLL(skb, SKBUFF_PTR) == PKTLINK(skb));

        bcm_prefetch(PKTDATA(wl->osh, skb)); /* for llc snap insert */

        skb_cb_zero(skb); /* explicitly zero cb[], wl_cb[] */
        // CFP uses PKTLINK, sames as PKTLIST_PKT_SLL i.e. skb->prev
        skb->dev  = net_device;
        PKTSETCFPFLOWID(skb, wl_pktfwd_pktlist->flowid);

        wlc_cfp_pkt_prepare(wl->unit, wl_pktfwd_pktlist->flowid,
                            wl_pktfwd_pktlist->prio, skb, cfp_exptime);

        bcm_prefetch(skb_sll); /* prefetch next skb */

        wl_pktfwd_llc_snap_insert(wl->osh, skb);

        skb = skb_sll;
    }
    /* osh::alloced add */
    PKTACCOUNT(wl->osh, wl_pktfwd_pktlist->len, TRUE);

}   /* wl_pktfwd_pktlist_cfp() */
#endif /* WLCFP */


/** Transpose a pktlist into a pktc chain_node list */
static inline struct sk_buff *
wl_pktfwd_pktlist_pktc(wl_info_t * wl, struct net_device * net_device,
                       wl_pktfwd_pktlist_t * wl_pktfwd_pktlist)
{
    struct sk_buff * skb, *skb_sll, * skb_chain;

    PKTFWD_PTRACE("pktlist len %u", wl_pktfwd_pktlist->len);
    skb_chain = skb = wl_pktfwd_pktlist->head;

    while (skb != (struct sk_buff *)NULL)
    {
        skb_sll = PKTLIST_PKT_SLL(skb, SKBUFF_PTR);
        bcm_prefetch(skb_sll);

        skb->prev = skb->next = NULL;
        skb_cb_zero(skb); /* OPTIMIZE (redundant) as pktc takes wl_start path */
        skb->dev  = net_device;

        PKTSETCHAINED(wl->osh, skb);
        PKTSETCLINK(skb, skb_sll);

        skb = skb_sll;
    }

    /* osh::alloced add */
    PKTACCOUNT(wl->osh, wl_pktfwd_pktlist->len, TRUE);

    /* Complete transposition */
    PKTCSETCNT(skb_chain, wl_pktfwd_pktlist->len);

    return skb_chain; /* return transposed packet chain */

}   /* wl_pktfwd_pktlist_pktc() */


/*
 * -----------------------------------------------------------------------------
 *
 * Function   : wl_pktfwd_pktlist_xmit
 * Description: Dispatch all accumulated packets to WLAN NIC driver
 *              via slowpath or fastpath.
 *
 *              For CFP capable, prepare CFP compliant pktlist and
 *              sendup pktlist via CFP fast path
 *
 *              For non-CFP capable, transpose pktlist into "chain_node" form
 *              and sendup chain_node via slow path
 * Inputs     : wl_pktfwd_pktlist - List of packets linked with PKTLINK (used by CFP)
 * -----------------------------------------------------------------------------
 */
void
wl_pktfwd_pktlist_xmit(struct net_device * net_device,
                       wl_pktfwd_pktlist_t * wl_pktfwd_pktlist)
{
    struct sk_buff    * skb_chain;
    pktlist_context_t * pktlist_context;
    wl_pktfwd_t       * wl_pktfwd = &wl_pktfwd_g;
    wl_info_t         * wl;
    wl_if_t           * wlif;
    d3fwd_wlif_t      * d3fwd_wlif;
#if defined(WLCFP)
    bool                cfp_tx_enabled;
#endif /*WLCFP */

    wl_pktfwd_g.stats.pktlist_xmit++;

    PKTFWD_ASSERT(net_device != (struct net_device *) NULL);
    PKTFWD_ASSERT(wl_pktfwd_pktlist != (wl_pktfwd_pktlist_t *) NULL);

    wl = WL_INFO_GET(net_device);

    WL_LOCK(wl);  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    wlif = WL_DEV_IF(net_device);
    d3fwd_wlif = __wlif_2_d3fwd_wlif(wlif);

    pktlist_context = WL_PKTLIST_CONTEXT(wl->unit);

#if defined(WLCFP)
    /* Determine if CFP Tx is enabled and CFP INI is Established */
    cfp_tx_enabled = wlc_cfp_tx_enabled(wl->unit, wl_pktfwd_pktlist->flowid,
                                        wl_pktfwd_pktlist->prio);

    if (likely(cfp_tx_enabled)) {

        wl_pktfwd->stats.txf_cfp_pkts += wl_pktfwd_pktlist->len;

        D3FWD_STATS_ADD(d3fwd_wlif->stats[wl_pktfwd_pktlist->prio].cfp_pkts,
                        wl_pktfwd_pktlist->len);
        D3FWD_STATS_ADD(d3fwd_wlif->stats[wl_pktfwd_pktlist->prio].cfp_fwds, 1);

        /* Transpose to CPF list by preparing packets for CFP w/ llcsnap */
        wl_pktfwd_pktlist_cfp(wl, net_device, wl_pktfwd_pktlist);

        PKTFWD_PTRACE("wl%d CFP flowid %d head %p tail %p len %d prio %d",
                       wl->unit, wl_pktfwd_pktlist->flowid,
                       wl_pktfwd_pktlist->head, wl_pktfwd_pktlist->tail,
                       wl_pktfwd_pktlist->len, wl_pktfwd_pktlist->prio);

        PKTFWD_ASSERT(PKTLIST_PKT_SLL(wl_pktfwd_pktlist->tail, SKBUFF_PTR)
                            == NULL);

        wlc_cfp_tx_sendup(wl->unit, wl_pktfwd_pktlist->flowid,
                          wl_pktfwd_pktlist->prio, wl_pktfwd_pktlist->head,
                          wl_pktfwd_pktlist->tail, wl_pktfwd_pktlist->len);
    } else
#endif /*WLCFP */
    {
        wl_pktfwd->stats.txf_chn_pkts += wl_pktfwd_pktlist->len;

        D3FWD_STATS_ADD(d3fwd_wlif->stats[wl_pktfwd_pktlist->prio].chn_pkts,
                        wl_pktfwd_pktlist->len);
        D3FWD_STATS_ADD(d3fwd_wlif->stats[wl_pktfwd_pktlist->prio].chn_fwds, 1);

        /* Transpose to PKT chain_node form */
        skb_chain = wl_pktfwd_pktlist_pktc(wl, net_device, wl_pktfwd_pktlist);

#if defined(BCM_PKTFWD_FLCTL)
         /* In legacy path, packet might be queued to non-ampdu queue (TXMOD).
          * Increament credits here and update in wlc_ampdu_agg_pktc() */
        __pktlist_fctable_add_credits(pktlist_context, wl_pktfwd_pktlist->prio,
            WL_CFPID2LUTID(wl_pktfwd_pktlist->flowid), wl_pktfwd_pktlist->len);
#endif /* BCM_PKTFWD_FLCTL */

        PKTFWD_ASSERT(skb_chain != (struct sk_buff *)NULL);

        wl_pktc_tx(wl, wlif, skb_chain);
    }

    /* All packets from pktlist are dispatched, RESET len = 0U */
    wl_pktfwd_pktlist->len = 0;

    WL_UNLOCK(wl); // ---------------------------------------------------------

    return;

}   /* wl_pktfwd_pktlist_xmit() */


/** Dispatch all pending pktlists in d3fwd_wlif's ucast work lists */
static void
wl_pktfwd_d3fwd_wlif_xmit(wl_info_t * wl, d3fwd_wlif_t * d3fwd_wlif)
{
    uint32_t prio;
    pktlist_elem_t      * pktlist_elem;
    pktlist_context_t   * pktlist_context;
    wl_pktfwd_pktlist_t   wl_pktfwd_pktlist;  /* declared on the stack */

#if defined(WL_PKTFWD_RUNQ)
    wl_pktfwd_runq_t    * runq = WL_RUNQ_P(wl->unit); /* work lists in runq */
#else
    d3fwd_wlif_t        * runq = d3fwd_wlif; /* work lists in d3fwd_wlif */
#endif

    PKTFWD_PTRACE("wl%d", wl->unit);

    pktlist_context = WL_PKTLIST_CONTEXT(wl->unit);

    /* Prevent dispatch re-entrancy */
    if (WL_DISPATCH_CHECK(wl->unit)) return;
    WL_DISPATCH_ENTER(wl->unit);

    /* if (!wl->pub->up) { simply free up all pktlists in PI's work lists? } */

wl_pktfwd_dnstream_pktlist_dispatch_continue:

    PKTLIST_LOCK(pktlist_context);  // ++++++++++++++++++++++++++++++++++++++++
    PKTFWD_LOCK(); // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

wl_pktfwd_dnstream_next_prio_continue:

    /* Fetch a non empty ucast pktlist (by prio) */
    prio = __builtin_ffs(runq->ucast_bmap); /* prio is in ffs form */
    if ( !prio ) {
        goto wl_pktfwd_dnstream_done; /* nothing to dispatch */
    }

    /* Convert to prio: rotate-prio, w/ gcc __builtin_ffs adjustment of 1 */
    prio = PKTFWD_UCAST_BMAP_FFS_TO_PRIO(prio);

#if defined(WL_PKTFWD_RUNQ)
    if (--runq->credits <= 0) {
        wl_pktfwd_g.stats.xmit_preempt++;
        goto wl_pktfwd_dnstream_done; /* preempt as no more credits */
    }
#endif

    pktlist_elem = (pktlist_elem_t *) dll_head_p(&runq->ucast[prio]);

    bcm_prefetch(pktlist_elem);

    if ( unlikely(dll_empty(&runq->ucast[prio])) )
    {
        runq->ucast_bmap &= ~(PKTFWD_UCAST_BMAP(prio));
        D3FWD_STATS_ADD(d3fwd_wlif->stats[prio].complete, 1);
        goto wl_pktfwd_dnstream_next_prio_continue;
    }

    PKTFWD_ASSERT(pktlist_elem->pktlist.len != 0U);
    PKTFWD_ASSERT(pktlist_elem->pktlist.head != (pktlist_pkt_t *)NULL);

    /* Found a non-empty ucast pktlist. Transpose elem's pktlist */
    D3FWD_STATS_ADD(d3fwd_wlif->stats[prio].tot_pkts, pktlist_elem->pktlist.len);

    /* Extract pktlist_elem->pktlist elements to a WLAN pktlist and
     * move pktlist_elem to free list.
     */
    wl_pktfwd_pktlist.head = (struct sk_buff *)pktlist_elem->pktlist.head;
    wl_pktfwd_pktlist.tail = (struct sk_buff *)pktlist_elem->pktlist.tail;
    wl_pktfwd_pktlist.len  = pktlist_elem->pktlist.len;
    wl_pktfwd_pktlist.prio = prio;
    wl_pktfwd_pktlist.flowid = WL_FWDID2LUTID(pktlist_elem->pktlist.dest);

    PKTLIST_RESET(&pktlist_elem->pktlist); /* len = 0U, key.v16 = ~0 */

    dll_delete(&pktlist_elem->node);
    dll_append(&pktlist_context->free, &pktlist_elem->node);

    PKTFWD_UNLK(); // ---------------------------------------------------------
    PKTLIST_UNLK(pktlist_context); // -----------------------------------------

   /* Transmit accumulated packets via CFP or as pkt_chaining */
    wl_pktfwd_pktlist_xmit(d3fwd_wlif->net_device, &wl_pktfwd_pktlist);

    /* no txsbnd ... */
    goto wl_pktfwd_dnstream_pktlist_dispatch_continue;

    // ... cannot reach here ...

wl_pktfwd_dnstream_done:

#if !defined(WL_PKTFWD_RUNQ)
    wl_complete_work(wl);
    d3fwd_wlif->wl_schedule = 0;
#endif

    PKTFWD_UNLK();  // --------------------------------------------------------
    PKTLIST_UNLK(pktlist_context);  // ----------------------------------------

    WL_DISPATCH_EXIT(wl->unit);

    return;

}   /* wl_pktfwd_d3fwd_wlif_xmit() */


#if defined(WL_PKTFWD_RUNQ)
static d3fwd_wlif_t *
wl_pktfwd_runq_schedule(wl_info_t * wl)
{
    wl_if_t      * wlif_iter;
    wl_if_t      * wlif_term;
    d3fwd_wlif_t * d3fwd_wlif;

    wl_pktfwd_runq_t * runq = WL_RUNQ_P(wl->unit);

    /* last run queue schedule was incomplete ... resume */
    if (runq->ucast_bmap) {
        d3fwd_wlif    = runq->d3fwd_wlif;
        PKTFWD_ASSERT(d3fwd_wlif != D3FWD_WLIF_NULL);
        return d3fwd_wlif;
    }

    /* traverse wl::if_list, continuing or begining from start */
    wlif_iter = (runq->d3fwd_wlif == D3FWD_WLIF_NULL) ?
                wl->if_list : runq->d3fwd_wlif->wlif;
    wlif_term = wlif_iter; /* end of traversal demarcation */

    while (wlif_iter != (wl_if_t *) NULL)
    {
        d3fwd_wlif = (d3fwd_wlif_t *) wlif_iter->d3fwd_wlif;

        if ((d3fwd_wlif != D3FWD_WLIF_NULL) && (d3fwd_wlif->wl_schedule)) {
            int prio;
            runq->d3fwd_wlif = d3fwd_wlif;

            PKTFWD_LOCK();  // ++++++++++++++++++++++++++++++++++++++++++++++++

            /* move all work lists to runq */
            for (prio = 0; prio < D3FWD_PRIO_MAX; prio++) {
                if (!dll_empty(&d3fwd_wlif->ucast[prio])) {
                    dll_join(&d3fwd_wlif->ucast[prio], &runq->ucast[prio]);
                }
            }
            runq->ucast_bmap        = d3fwd_wlif->ucast_bmap;
            d3fwd_wlif->ucast_bmap  = 0U;

            wl_complete_work(wl);
            d3fwd_wlif->wl_schedule = 0;

            PKTFWD_UNLK();  // ------------------------------------------------

            return d3fwd_wlif;
        }

        wlif_iter = wlif_iter->next;
        if (wlif_iter == NULL) /* wrap around sll */
            wlif_iter = wl->if_list;

        if (wlif_iter == wlif_term)
            break; /* reached the demarcation to end loop */
    }

    return D3FWD_WLIF_NULL;

}   /* wl_pktfwd_runq_schedule() */

#endif /* WL_PKTFWD_RUNQ */


/** Iterate over all wlif(s) for a radio and dispatch accumulated pktlists */
void
wl_pktfwd_dnstream(wl_info_t * wl)
{
#if defined(WL_PKTFWD_RUNQ)
    d3fwd_wlif_t     * d3fwd_wlif;
    wl_pktfwd_runq_t * runq = WL_RUNQ_P(wl->unit);

    runq->credits = WL_PKTFWD_RUNQ; /* pktlists xmit credits (tunable?) */

    /* Supply credits to run queue: to be used across all wlif */
    while ((d3fwd_wlif = wl_pktfwd_runq_schedule(wl)) != D3FWD_WLIF_NULL)
    {
        wl_pktfwd_d3fwd_wlif_xmit(wl, d3fwd_wlif);

        if (runq->credits <= 0) {
			wl->txq_txchain_dispatched = true; /* re-enter wl_thread loop */
            return;
        }
    }

    /* If last dispatched worklist is from an incomplete runq and there are no
     * other d3flwd_wlif pending worklists to be dispatched then
     * wl->txq_txchain_dispatched will be set to TRUE and WLAN thread will be
     * scheduled.
     * Clear the pending txq work flag here.
     */
    wl->txq_txchain_dispatched = false;

#else  /* ! WL_PKTFWD_RUNQ */
    wl_if_t * wlif_iter  = wl->if_list;

    while (wlif_iter != NULL)
    {
        d3fwd_wlif_t * d3fwd_wlif = (d3fwd_wlif_t *) wlif_iter->d3fwd_wlif;
        if ((d3fwd_wlif != D3FWD_WLIF_NULL) && (d3fwd_wlif->wl_schedule)) {
            wl_pktfwd_d3fwd_wlif_xmit(wl, d3fwd_wlif);
        }
        wlif_iter = wlif_iter->next;
    }
#endif /* ! WL_PKTFWD_RUNQ */
}   /* wl_pktfwd_dnstream() */


#if defined(WL_PKTFWD_TXEVAL)

/*
 * -----------------------------------------------------------------------------
 *
 * Function   : wl_pktfwd_dispatch_pktlist
 * Description: Dispatch a pktlist identified by prio, dest if it is pending
 *              in d3fwd_wlif's ucast work lists.
 *
 *              Invoked on a TxStatus.
 *              WL_PKTFWD_TXEVAL
 * -----------------------------------------------------------------------------
 */
void
wl_pktfwd_dispatch_pktlist(wl_info_t * wl, wl_if_t * wlif,
    uint8_t * d3addr, uint16_t cfp_flowid, uint16_t prio)
{
    uint32_t dest;
    d3lut_elem_t          * d3lut_elem;
    d3fwd_wlif_t          * d3fwd_wlif;
    pktlist_t             * pktlist;
    pktlist_elem_t        * pktlist_elem;
    pktlist_context_t     * pktlist_context;
    wl_pktfwd_t           * wl_pktfwd = &wl_pktfwd_g;
    wl_pktfwd_pktlist_t     wl_pktfwd_pktlist;  /* declared on the stack */

    PKTFWD_PTRACE("wl%d", wl->unit);

    /* Prevent dispatch re-entrancy */
    if (WL_DISPATCH_CHECK(wl->unit)) return;
    WL_DISPATCH_ENTER(wl->unit);

    if (wlif == (wl_if_t *) NULL)
        d3fwd_wlif = __wl_2_d3fwd_wlif(wl);
    else
        d3fwd_wlif = __wlif_2_d3fwd_wlif(wlif);

    pktlist_context = WL_PKTLIST_CONTEXT(wl->unit);

    PKTFWD_ASSERT(d3fwd_wlif != D3FWD_WLIF_NULL);
    PKTFWD_ASSERT(pktlist_context != PKTLIST_CONTEXT_NULL);

#if defined(WLCFP)
    if (CFP_ENAB(wl->pub) == TRUE)
    {
        PKTFWD_ASSERT(cfp_flowid != ID16_INVALID);
        d3lut_elem = __wl_flowid_2_d3lut_elem(wl->unit, cfp_flowid);
    }
    else
#endif /* WLCFP */
    {
        /* d3lut::lock is NOT taken !!! */
        d3lut_elem = d3lut_lkup(wl_pktfwd->d3lut, d3addr, wl->unit);
    }

    if ((d3lut_elem == D3LUT_ELEM_NULL) || !(d3lut_elem->ext.inuse))
        /* Station is not registered with PKTFWD */
        goto wl_pktfwd_dispatch_pktlist_done;

    PKTFWD_ASSERT(d3lut_elem->ext.wlan);

    dest = D3LUT_ELEM_IDX(d3lut_elem->ext.flow_key.index);

    PKTFWD_ASSERT(dest <= PKTLIST_MCAST_ELEM);
    PKTFWD_ASSERT(prio <  PKTLIST_PRIO_MAX);

    pktlist_elem = PKTLIST_CTX_ELEM(pktlist_context, prio, dest);
    pktlist      = &pktlist_elem->pktlist;

    PKTLIST_LOCK(pktlist_context);  // ++++++++++++++++++++++++++++++++++++++++
    PKTFWD_LOCK(); // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    if ((pktlist->len != 0) &&
        (wl_pktfwd_xfer_pktlist_test(pktlist, d3lut_elem,
                                     PKTLIST_NULL, false) == false))
    {

        PKTFWD_ASSERT(pktlist->head != PKTLIST_PKT_NULL);

        /* Found a non-empty ucast pktlist. Transpose elem's pktlist */
        D3FWD_STATS_ADD(d3fwd_wlif->stats[prio].tot_pkts, pktlist->len);

        /* Extract pktlist_elem->pktlist elements to a WLAN pktlist and
         * move pktlist_elem to free list.
         */
        wl_pktfwd_pktlist.head  = (struct sk_buff *) pktlist->head;
        wl_pktfwd_pktlist.tail  = (struct sk_buff *) pktlist->tail;
        wl_pktfwd_pktlist.len   = pktlist->len;
        wl_pktfwd_pktlist.prio  = prio;
        wl_pktfwd_pktlist.flowid = WL_FWDID2LUTID(pktlist->dest);

        PKTLIST_RESET(pktlist); /* len = 0U, key.v16 = ~0 */

        /* Move pktlist to free list from work list (runq or d3fwd_wlif) */
        dll_delete(&pktlist_elem->node);
        dll_append(&pktlist_context->free, &pktlist_elem->node);
    }
    else
    {
        /* Nothing to do: No pending packets or stale packets;
         * Stale packets will be discarded in wl_pktfwd_xfer_callback() */
        wl_pktfwd_pktlist.len = 0U;
    }

    PKTFWD_UNLK(); // ---------------------------------------------------------
    PKTLIST_UNLK(pktlist_context); // -----------------------------------------

    if (wl_pktfwd_pktlist.len != 0)
    {
        WL_UNLOCK(wl); // -----------------------------------------------------

        /* "Now lockless: xmit accumulated packets via CFP or as pkt_chaining */
        wl_pktfwd_pktlist_xmit(d3fwd_wlif->net_device, &wl_pktfwd_pktlist);

        wl_pktfwd_g.stats.txeval_xmit++;

        WL_LOCK(wl);  // ++++++++++++++++++++++++++++++++++++++++++++++++++++++
    }

wl_pktfwd_dispatch_pktlist_done:
    WL_DISPATCH_EXIT(wl->unit);

}   /* wl_pktfwd_dispatch_pktlist() */

#endif /* WL_PKTFWD_TXEVAL */


void
wl_pktfwd_dnqueued(wl_info_t * wl, d3fwd_wlif_t * d3fwd_wlif,
                   uint32_t * ucast_pkts)
{
    uint32_t prio;
    pktlist_context_t * pktlist_context = WL_PKTLIST_CONTEXT(wl->unit);

    PKTLIST_LOCK(pktlist_context);  // ++++++++++++++++++++++++++++++++++++++++

    for (prio = 0; prio < D3FWD_PRIO_MAX; ++prio)
    {
        dll_t * list = &d3fwd_wlif->ucast[prio];
        dll_t * item, * next; /* dll iterator */
        uint32_t pkts = 0U;
        for (item = dll_head_p(list); ! dll_end(list, item); item = next)
        {
            next = dll_next_p(item);
            pktlist_elem_t * pktlist_elem = (pktlist_elem_t *)item;
            pkts += pktlist_elem->pktlist.len;
        }
        *(ucast_pkts + prio) = pkts;
    }

    PKTLIST_UNLK(pktlist_context);  // ----------------------------------------

}

/**
 * =============================================================================
 * Section WLAN Receive Packet Forwarding
 * =============================================================================
 */

#if !(defined(BCM_AWL) && defined(WL_AWL_RX))
/**
 * -----------------------------------------------------------------------------
 * WLAN Receive Path Forwarding
 *
 * Function: Accumalate CFP capable packets into local packet queue (indexed by
 * domain).
 * Once all packets are binned, CFP will invoke wl_pktfwd_flush_pktqueues().
 *
 * wl_pktfwd_flush_pktqueues() is responsible for flushing all packets from
 * local packet queue to corresponding egress network device sw queues and
 * wakeup egress driver thread using registered flush_pkts_fn and
 * flush_complete_fn handlers.
 *
 * Non-CFP packets ((chain or sll) are forwarded to egress network device using
 * the pktfwd 802.3 MacAddr Lookup table.
 *
 * -----------------------------------------------------------------------------
 */


/*
 * -----------------------------------------------------------------------------
 *
 * Function   : wl_pktfwd_pktqueue_add_pkt
 * Description: Tag packet with d3lut pktfwd_key_t and add it to a packet queue
 *              identified by the egress network device domain using the pktfwd
 *              802.3 MacAddr Lookup table.
 *
 * -----------------------------------------------------------------------------
 */

void
wl_pktfwd_pktqueue_add_pkt(wl_info_t * wl, struct net_device * rx_net_device,
                           void * pkt, uint16_t flowid)
{
    uint16_t d3domain, pktfwd_key;
    uint8_t           * d3_addr;
    d3lut_elem_t      * d3lut_elem;
    pktqueue_t        * pktqueue;
    pktqueue_table_t  * pktqueue_table;
    struct sk_buff    * skb;
    wl_pktfwd_t       * wl_pktfwd = &wl_pktfwd_g;
    struct ether_header * eh;

    PKTFWD_PTRACE("wl%d pkt %p", wl->unit, pkt);

    skb = (struct sk_buff *)pkt;

#if !defined WL_PKTFWD_INTRABSS
    /* For packets from CFP Rx path, check that the dst is not associated to
     * same BSS before forwarding packet to Network stack.
     * For Non-CFP packets, it is already taken care.
     */
    if ((flowid != ID16_INVALID) &&
        (wl_intrabss_forward(wl, rx_net_device, skb) == true))
    {
        return;
    }
#endif /* ! WL_PKTFWD_INTRABSS */

    skb->dev = rx_net_device;
    eh = (struct ether_header *)PKTDATA(wl->osh, skb);
    d3_addr = (uint8_t *) eh->ether_dhost;

    pktqueue_table = WL_PKTQUEUE_TABLE(wl->unit);
    d3domain = PKTQUEUE_NTKSTK_QUEUE_IDX;

    PKTFWD_LOCK(); // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    D3LUT_LOCK(wl_pktfwd->d3lut); // ++++++++++++++++++++++++++++++++++++++++++

    if (!ETHER_ISMULTI(d3_addr))
    {
        d3lut_elem = d3lut_lkup(wl_pktfwd->d3lut, d3_addr, D3LUT_LKUP_GLOBAL_POOL);
    }
    else
    {
        /* Send Multicast packets to Network stack */
        d3lut_elem = D3LUT_ELEM_NULL;
    }

    if ( likely(d3lut_elem != D3LUT_ELEM_NULL) &&
         (eh->ether_type != hton16(ETHER_TYPE_8021Q)))
    {	/* d3lut hit and Untagged frames */
        uint8_t prio4bit = 0;
        d3domain = d3lut_elem->key.domain; /* WLAN to WLAN/LAN */

        /* Reset wl FlowInf and set pktfwd FlowInf */
        skb->wl.u32 = 0U;

        pktfwd_key = PKTC_WFD_CHAIN_IDX(d3lut_elem->key.domain,
                                        d3lut_elem->key.v16);
        ENCODE_WLAN_PRIORITY_MARK(prio4bit, skb->mark);

        /* Tag packet with d3lut pktfwd_key_t  */
        skb->wl.pktfwd.is_ucast     = 1;
        skb->wl.pktfwd.pktfwd_key   = pktfwd_key;
        skb->wl.pktfwd.wl_prio      = prio4bit;
        skb->wl.pktfwd.ssid         = d3lut_elem->ext.ssid;
    }

    D3LUT_UNLK(wl_pktfwd->d3lut); // ------------------------------------------

#if defined(WL_PKTFWD_INTRABSS)
    if (d3lut_elem != D3LUT_ELEM_NULL)
    {
        wlc_bsscfg_t * bsscfg;
        bsscfg = wl_bsscfg_find(WL_DEV_IF(rx_net_device));

        if ((bsscfg->ap_isolate) &&
            (d3lut_elem->ext.d3fwd_wlif->net_device == rx_net_device))
        {
            /* If bsscfg->ap_isolate is set, send intrabss packets to NTKSTK */
            PKTFWD_ASSERT(d3lut_elem->ext.wlan);
            d3domain = PKTQUEUE_NTKSTK_QUEUE_IDX;
        }
    }

    /* Intrabss forwarding for d3lut miss packets is handled in
     * wl_pktfwd_flush_pktqueues */

#else /* ! WL_PKTFWD_INTRABSS */
    /* when bsscfg->ap_isolate is set, intrabss packets will reach here.
     * these frames has to be sent to NTKSTK */
    if ((d3lut_elem != D3LUT_ELEM_NULL) &&
        (d3lut_elem->ext.d3fwd_wlif->net_device == rx_net_device))
    {
        PKTFWD_ASSERT(d3lut_elem->ext.wlan);
        d3domain = PKTQUEUE_NTKSTK_QUEUE_IDX;
    }
#endif /* ! WL_PKTFWD_INTRABSS */

    pktqueue = PKTQUEUE_TBL_QUEUE(pktqueue_table, d3domain);

    if( likely(pktqueue->len != 0U) ) /* Do not use <head,tail> PKTQUEUE_RESET */
    {
        PKTQUEUE_PKT_SET_SLL(pktqueue->tail, skb, SKBUFF_PTR);
        pktqueue->tail = (pktqueue_pkt_t *)skb;
    }
    else
    {
        pktqueue->head = (pktqueue_pkt_t *)skb;
        pktqueue->tail = (pktqueue_pkt_t *)skb;
    }

    pktqueue->len++;

    D3FWD_STATS_EXPR(
    {
        uint16_t            prio;
        wl_if_t           * wlif;
        d3fwd_wlif_t      * d3fwd_wlif;

        wlif        = WL_DEV_IF(rx_net_device);
        d3fwd_wlif  = __wlif_2_d3fwd_wlif(wlif);
        prio        = PKTPRIO(skb);

        D3FWD_STATS_ADD(d3fwd_wlif->stats[prio].rx_tot_pkts, 1);

        if (d3domain != PKTQUEUE_NTKSTK_QUEUE_IDX)
            D3FWD_STATS_ADD(d3fwd_wlif->stats[prio].rx_fast_pkts, 1);
        else
            D3FWD_STATS_ADD(d3fwd_wlif->stats[prio].rx_slow_pkts, 1);
    });

    /* CAUTION: PKTQUEUE_PKT_SLL(skb) is NOT terminated by NULL ! */

    PKTFWD_UNLK(); // ---------------------------------------------------------

}   /* wl_pktfwd_pktqueue_add_pkt() */


/*
 * -----------------------------------------------------------------------------
 *
 * Function   : wl_pktfwd_flush_pktqueues
 * Description: Flush all accumulated recv packets to egress network device
 *              (WFD/LAN) packet queue.
 *
 * CAUTION:     Walking all packet queues without lock.
 * -----------------------------------------------------------------------------
 */

void
wl_pktfwd_flush_pktqueues(wl_info_t * wl)
{
    uint16_t                domain;
    pktqueue_t            * pktqueue;
    pktqueue_pkt_t        * pkt;
    pktqueue_table_t      * pktqueue_table;
    pktqueue_context_t    * pktqueue_context;
    wl_pktfwd_t           * wl_pktfwd = &wl_pktfwd_g;
    struct sk_buff        * skb;

    pktqueue_table = WL_PKTQUEUE_TABLE(wl->unit);
    domain = 0;

wl_pktfwd_flush_pktqueues_continue:

    pktqueue = PKTQUEUE_TBL_QUEUE(pktqueue_table, domain);

    if (pktqueue->len != 0U)
    {
        if (domain != PKTQUEUE_NTKSTK_QUEUE_IDX)
            pktqueue_context = pktqueue_get_domain_pktqueue_context(domain);
        else
            pktqueue_context = PKTQUEUE_CONTEXT_NULL;

        if (pktqueue_context != PKTQUEUE_CONTEXT_NULL)
        {
            wl_pktfwd->stats.rx_fast_fwds += pktqueue->len;
            pktqueue_context->pkts_stats += pktqueue->len;

            /* Flush packets from local pktqueue to egress device SW queue */
            pktqueue_context->flush_pkts_fn(pktqueue_context->driver, pktqueue);

            /* Wake egress network device thread: invoke "flush complete"
             * handler to wake peer driver.
             */
            pktqueue_context->flush_complete_fn(pktqueue_context->driver);
            pktqueue_context->queue_stats++;
        }
        else /* Send each pkt to Network stack */
        {
            wl_pktfwd->stats.rx_slow_fwds += pktqueue->len;
            while (pktqueue->len)
            {
                pkt             = pktqueue->head;
                pktqueue->head  = PKTQUEUE_PKT_SLL(pkt, SKBUFF_PTR);
                PKTQUEUE_PKT_SET_SLL(pkt, PKTQUEUE_PKT_NULL, SKBUFF_PTR);
                pktqueue->len--;

                skb = (struct sk_buff *)pkt;

#if defined(WL_PKTFWD_INTRABSS)
                if (!ETHER_ISMULTI(PKTDATA(wl->osh, skb)))
                {
                    if (wl_intrabss_forward(wl, skb->dev, skb) == true)
                        continue;
                }
#endif /* WL_PKTFWD_INTRABSS */

#if defined(WL_PKTQUEUE_RXCHAIN)
                wl_sendup_ex(wl, skb);
#else /* ! WL_PKTQUEUE_RXCHAIN */

#ifdef BCM_BLOG
                if (wl_handle_blog_sinit(wl, skb) == 0)
                    continue;
#endif /* BCM_BLOG */

                skb->protocol = eth_type_trans(skb, skb->dev);

#if defined(BCM_NBUFF_PKT) && defined(CC_BPM_SKB_POOL_BUILD)
                PKTTAINTED(wl->osh, skb);
#endif /* BCM_NBUFF_PKT && CC_BPM_SKB_POOL_BUILD */

#ifdef NAPI_POLL
                netif_receive_skb(skb);
#else /* ! NAPI_POLL */
                netif_rx(skb);
#endif /* ! NAPI_POLL */

#endif /* ! WL_PKTQUEUE_RXCHAIN */

            } /* while (pktqueue->len) */
        }

        PKTQUEUE_RESET(pktqueue); /* head,tail, not reset */

    } /* pktqueue->len != 0 */

    domain++;

    if (domain < PKTQUEUE_QUEUES_MAX)
        goto wl_pktfwd_flush_pktqueues_continue;

}   /* wl_pktfwd_flush_pktqueues() */



/**
 * -----------------------------------------------------------------------------
 * Function    : wl_pktfwd_upstream
 * Description : Forward sk_buff (chain or sll) to the egress network device
 *               using the pktfwd 802.3 MacAddr Lookup table.
 *
 *               With WL_PKTQUEUE_RXCHAIN, packets will binned to local domain
 *               PktQueues. WLAN driver should invoke
 *               wl_pktfwd_flush_pktqueues() to flush PktQueues.
 * -----------------------------------------------------------------------------
 */

int
wl_pktfwd_upstream(wl_info_t * wl, struct sk_buff * skb)
{
#if defined(WL_PKTQUEUE_RXCHAIN)
    void              * nskb;
    struct net_device * rx_net_device;
#else /* ! WL_PKTQUEUE_RXCHAIN */
    uint8_t           * d3_addr;
    struct net_device * net_device;
    d3lut_elem_t      * d3lut_elem;
    wl_pktfwd_t * wl_pktfwd = &wl_pktfwd_g;
#endif /* ! WL_PKTQUEUE_RXCHAIN */

    PKTFWD_PTRACE("wl%d skb %p", wl->unit, skb);

#if defined(BCM_WFD) && defined(CONFIG_BCM_PON)
    priv_link_t *priv_link = netdev_priv(skb->dev);

    if ((inject_to_fastpath) && (WL_IFTYPE_WDS != priv_link->wlif->if_type))
    {

#if defined(WL_PKTFWD_INTRABSS)
        if (ETHER_ISMULTI(PKTDATA(wl->osh, skb)) ||
            (wl_intrabss_forward(wl, skb->dev, skb) == false))
#endif /* WL_PKTFWD_INTRABSS */
        {
            /* call to registered fastpath callback */
            send_packet_to_upper_layer(skb);
        }
        return BCME_OK;
    }
#endif /* BCM_WFD && CONFIG_BCM_PON */

#if defined(WL_PKTQUEUE_RXCHAIN)

    rx_net_device = skb->dev;
    /* Break PKTC chain and bin packets to domain PktQueues */
    FOREACH_CHAINED_PKT(skb, nskb)
    {
        PKTCLRCHAINED(wl->osh, skb);
        PKTCCLRFLAGS(skb);
        wl_pktfwd_pktqueue_add_pkt(wl, rx_net_device, skb, ID16_INVALID);
    }

    return BCME_OK;

#else /* ! WL_PKTQUEUE_RXCHAIN */

    d3_addr = (uint8_t *) PKTDATA(wl->osh, skb);

    if (ETHER_ISMULTI(d3_addr)) /* Only unicast packets are bridged via LUT */
        return BCME_UNSUPPORTED;

    PKTFWD_LOCK(); // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    /* d3lut::lock is NOT taken !!! */
    d3lut_elem = d3lut_lkup(wl_pktfwd->d3lut, d3_addr, D3LUT_LKUP_GLOBAL_POOL);

    if (likely(d3lut_elem != D3LUT_ELEM_NULL))
    {
        if (!d3lut_elem->ext.wlan)
        {
            net_device = d3lut_elem->ext.net_device;
            d3lut_elem->ext.hit = 1;
        }
        else
        {
            d3fwd_wlif_t * d3fwd_wlif;
            d3fwd_wlif = d3lut_elem->ext.d3fwd_wlif;
            if (d3fwd_wlif != D3FWD_WLIF_NULL)
            {
                net_device = d3fwd_wlif->net_device;
                d3lut_elem->ext.hit = 1;
            }
            else
                net_device = (struct net_device *) NULL;
        }
    }
    else
        net_device = (struct net_device *) NULL;

    PKTFWD_UNLK(); // ---------------------------------------------------------

#if defined(WL_PKTFWD_INTRABSS)
    if (!ETHER_ISMULTI(d3_addr))
    {
        if (wl_intrabss_forward(wl, skb->dev, skb) == true)
            return BCME_OK;
    }
#endif /* WL_PKTFWD_INTRABSS */

    if ((net_device != (struct net_device *) NULL) &&
        (skb->dev != net_device))
    {
        xmit_fn_t netdev_ops_xmit;
        netdev_ops_xmit = (xmit_fn_t)(net_device->netdev_ops->ndo_start_xmit);
        netdev_ops_xmit(skb, net_device);
        wl_pktfwd->stats.rx_fast_fwds++;
        return BCME_OK;
    }
    else
    {
        wl_pktfwd->stats.rx_slow_fwds++;
        return BCME_ERROR;
    }

#endif /* ! WL_PKTQUEUE_RXCHAIN */

}   /* wl_pktfwd_upstream() */

#endif /* ! (BCM_AWL && WL_AWL_RX) */
