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
 * WLAN Packet Forwarding Datapath
 *
 * Incorporates 4 primary functionality:
 *   1. Management of 802.3 Mac Addresses (D3 LUT)
 *      Serves as a Cache of native OS Bridge layer (e.g. Linux Bridge FDB)
 *   2. Binning packets based on a 802.3 Mac Address using <index,incarn>
 *      D3 LUT pairs a 802.3 MacAddr to a 16bit <incarn,index>. Index may be
 *      a staid. Receive packets are binned based on a D3 LUT "index"
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
 *   5. Cleanup all PKTC entry points, retaining a single dhd_pktfwd_request.
 *      May retain the dhd_pktfwd_match() which is invoked on WLAN receive path
 *      on a per packet basis for Receive is chainable.
 *        - dhd_pktc_del_hook() registered with Blog SHIM
 *        - fdb_check_expired_dhd_hook registered with Linux Bridge
 *
 * =============================================================================
 */
#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/bcm_skb_defines.h>
#include <linux/proc_fs.h>
#include <net/switchdev.h>

#include <bcm_pktfwd.h>
#include <dhd_pktfwd.h>
#include <ethernet.h>
#include <bcmevent.h>
#include <bcmendian.h>
#include <wl_pktc.h>

#include <dhd.h>
#include <dhd_linux.h>
#include <dhd_flowring.h>
#include <dhd_bus.h>
#include <dhd_dbg.h>
#include <dhd_wfd.h>

#if defined(BCM_AWL)
#include <dhd_awl.h>
#endif /* BCM_AWL */

#if defined(BCM_CPE_DHD_GSO)
#include <dhd_sw_gso.h>
#endif /* BCM_CPE_DHD_GSO */

typedef int (*xmit_fn_t) (struct sk_buff * skb, struct net_device * dev);

/* Forward declarations */
struct dhd_pktfwd_keymap;
typedef struct dhd_pktfwd_keymap dhd_pktfwd_keymap_t;

#ifndef DHD_PERIM_ASSERT
#define DHD_PERIM_ASSERT(_pub)	({ BCM_REFERENCE(_pub); })
#endif

#define DHD_PKTFWD_KEYMAP_NULL          ((dhd_pktfwd_keymap_t *) NULL)

/* CAUTION: Should not exceed PKTFWD_ENDPOINTS_MAX */
#define DHD_PKTFWD_DEST_MAX             (DHD_MAX_STA)
#define DHD_PKTFWD_FLOWIDS_MAX          (PKTFWD_PRIO_MAX * DHD_PKTFWD_DEST_MAX)

/* Toggle pktfwd acceleration in dwds via nvram vars */
extern char *nvram_get(const char *name);
#define NVRAM_DWDS_AP_PKTFWD_ACCEL	"dhd_dwds_ap_pktfwd"	/* 0: disable, 1: enable */
#define NVRAM_DWDS_STA_PKTFWD_ACCEL	"dhd_dwds_sta_pktfwd"	/* 0: disable, 1: enable */
#define NVRAM_PKTFWD_ACCEL		"dhd_pktfwd"	/* 0: disable, 1: enable */
bool dhd_dwds_ap_pktfwd_accel = true;	/* default dwds ap pktfwd accel */
bool dhd_dwds_sta_pktfwd_accel = true;	/* default dwds sta pktfwd accel */
bool dhd_pktfwd_accel = true;	/* default pktfwd accel */

/* DWDS DA station flag */
#define STA_FLAG_IDLE    0
#define STA_FLAG_ACTIVE  1

/**
 * =============================================================================
 * Section: PKTFWD Global System Object(s)
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 *
 * Singleton global object.
 *
 * - lock            : Global system lock
 * - stats           : Global system statistics
 * - d3lut           : Lookup table for 802.3 MacAddresses for LAN and WLAN.
 * - pktlist_context : Per WLAN radio, consumer pktlist_context.
 * - d3fwd_used      : List of in-use d3fwd_wlif objects in a dbl linked list
 * - d3fwd_free      : List of free d3fwd_wlif objects in a dbl linked list
 * - d3fwd_wlif      : Preallocated pool of d3fwd_wlif objects
 * - radio_cnt       : Count of registered WLAN radios <dhd_info>
 * - wlif_cnt        : Count of registered WLAN Interfaces <wl_if,net_device>
 * - tx_accel        : Runtime WFD to WLAN Transmit acceleration state
 * - initialized     : Flag for initialization state of singleton global
 *
 * -----------------------------------------------------------------------------
 */

typedef struct dhd_pktfwd {	/* Global System State */
	spinlock_t lock;	/* system lock */
	dhd_pktfwd_stats_t stats;	/* system wide statistics */

	d3lut_t *d3lut;		/* 802.3 MAC Address LUT */
	int wfd_idx[DHD_PKTFWD_RADIOS];
	pktlist_context_t *pktlist_context[DHD_PKTFWD_RADIOS];	/* WLAN downstream */
	pktqueue_table_t *pktqueue_table[DHD_PKTFWD_RADIOS];	/* WLAN upstream */
	dhd_pktfwd_keymap_t *dhd_pktfwd_keymap[DHD_PKTFWD_RADIOS];

	dll_t d3fwd_used;	/* list of in-use d3fwd_wlif */
	dll_t d3fwd_free;	/* list of free d3fwd_wlif */
	d3fwd_wlif_t *d3fwd_wlif;	/* prealloc d3fwd_wlif pool */

	int8_t radio_cnt;	/* count of radio(s) attached */
	int8_t wlif_cnt;	/* count of wl_if(s) attached */
	uint8_t tx_accel;	/* wlan tx acceleration */
	bool initialized;	/* global system initialized */
} dhd_pktfwd_t;

/** Static initialization of singleton system global object */
dhd_pktfwd_t dhd_pktfwd_g = {
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
	.lock = __SPIN_LOCK_UNLOCKED(dhd_pktfwd_g.lock),
#endif
	.stats = {},
	.d3lut = D3LUT_NULL,
	.wfd_idx = {-1, -1, -1, -1},
	.pktlist_context = {
		PKTLIST_CONTEXT_NULL,
		PKTLIST_CONTEXT_NULL,
		PKTLIST_CONTEXT_NULL,
		PKTLIST_CONTEXT_NULL
	}, /* DHD_PKTFWD_RADIOS = 4 */
	.pktqueue_table = {
		PKTQUEUE_TABLE_NULL,
		PKTQUEUE_TABLE_NULL,
		PKTQUEUE_TABLE_NULL,
		PKTQUEUE_TABLE_NULL
	}, /* DHD_PKTFWD_RADIOS = 4 */
	.dhd_pktfwd_keymap = {
		DHD_PKTFWD_KEYMAP_NULL,
		DHD_PKTFWD_KEYMAP_NULL,
		DHD_PKTFWD_KEYMAP_NULL,
		DHD_PKTFWD_KEYMAP_NULL
	}, /* DHD_PKTFWD_RADIOS = 4 */
	.d3fwd_used = DLL_STRUCT_INITIALIZER(dhd_pktfwd_g, d3fwd_used),
	.d3fwd_free = DLL_STRUCT_INITIALIZER(dhd_pktfwd_g, d3fwd_free),
	.d3fwd_wlif = D3FWD_WLIF_NULL,
	.radio_cnt = 0,
	.wlif_cnt = 0,
#if defined(BCM_WFD)
	.tx_accel = ~0,
#else
	.tx_accel = 0,
#endif
	.initialized = false
};

const char *dhd_pktfwd_req_str[dhd_pktfwd_req_max_e] = {
	"UNDEF", "TXSET", "TXGET", "D3INS", "FLUSH", "ASSOC", "PLIST"
};

/** System global lock macros mutual exclusive access */
#if defined(CONFIG_SMP) || defined(CONFIG_PREEMPT)
#define PKTFWD_LOCK()       spin_lock_bh(&dhd_pktfwd_g.lock)
#define PKTFWD_UNLK()       spin_unlock_bh(&dhd_pktfwd_g.lock)
#else				/* ! (CONFIG_SMP || CONFIG_PREEMPT) */
#define PKTFWD_LOCK()       local_irq_disable()
#define PKTFWD_UNLK()       local_irq_enable()
#endif /* ! (CONFIG_SMP || CONFIG_PREEMPT) */

/*
 * Mixed mode: PKTFWD enabled for some radios only
 */
#define DHD_PKTFWD_SUPPORTED(unit)   ((unit) < DHD_PKTFWD_RADIOS)
#define DHD_WFD_IDX_VALID(unit)      (dhd_pktfwd_g.wfd_idx[(unit)] >= 0)

static int BCMFASTPATH
dhd_pktfwd_pktlist_xmit(pktlist_context_t * dhd_pktlist_context,
		pktlist_elem_t * pktlist_elem, d3lut_elem_t * d3lut_elem);

static int
dhd_pktfwd_get_keymap(uint32_t radio_idx, uint16_t * pktfwd_key,
		uint16_t * flowid, uint16_t prio, bool k2f);

static void
_dhd_pktfwd_lut_del(uint8_t * d3addr, d3lut_elem_t * d3lut_elem,
		uint32_t d3domain, struct net_device *net_device);

/**
 * =============================================================================
 * Section: DHD PKTFWD Key mapping
 * =============================================================================
 */
struct dhd_pktfwd_keymap {
	uint16 flowid_pktfwdkey[DHD_PKTFWD_FLOWIDS_MAX];
	uint16 pktfwdkey_flowid[PKTFWD_PRIO_MAX][DHD_PKTFWD_DEST_MAX];
};

/** Reset pktfwd keymap for a given flowid */
void
dhd_pktfwd_reset_keymap(uint32_t radio_idx, uint16_t dest, uint16_t flowid, uint16_t prio)
{
	dhd_pktfwd_keymap_t *dhd_pktfwd_keymap;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	if (!DHD_PKTFWD_SUPPORTED(radio_idx)) {
		/* PKTFWD not supported for this radio */
		return;
	}

	if ((dest == ID16_INVALID) && (flowid != ID16_INVALID)) {
		uint16_t pktfwd_key;
		/* get the actual dest by given flowid */
		dhd_pktfwd_get_keymap(radio_idx, &pktfwd_key, &flowid, prio, PKTFWD_KEYMAP_F2K);
		dest = PKTLIST_DEST(pktfwd_key);
	}

	if ((dest == ID16_INVALID) || (flowid == ID16_INVALID))
		return;

	PKTFWD_LOCK();		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	dhd_pktfwd_keymap = dhd_pktfwd->dhd_pktfwd_keymap[radio_idx];

	/* reset Key and Flowid mapping */
	dhd_pktfwd_keymap->pktfwdkey_flowid[prio][dest] = ID16_INVALID;
	dhd_pktfwd_keymap->flowid_pktfwdkey[flowid] = ID16_INVALID;

	PKTFWD_UNLK();		// --------------------------------------------------------

} /* dhd_pktfwd_reset_keymap() */

/** Set pktfwd keymap for a given flowid & pktfwd_key */
void
dhd_pktfwd_set_keymap(uint32_t radio_idx, uint16_t pktfwd_key, uint16_t flowid, uint16_t prio)
{
	uint16_t dest = PKTLIST_DEST(pktfwd_key);
	dhd_pktfwd_keymap_t *dhd_pktfwd_keymap;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	if (!DHD_PKTFWD_SUPPORTED(radio_idx)) {
		/* PKTFWD not supported for this radio */
		return;
	}

	PKTFWD_LOCK();		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	dhd_pktfwd_keymap = dhd_pktfwd->dhd_pktfwd_keymap[radio_idx];

	/* set Key and Flowid mapping */
	dhd_pktfwd_keymap->flowid_pktfwdkey[flowid] = pktfwd_key;
	dhd_pktfwd_keymap->pktfwdkey_flowid[prio][dest] = flowid;

	PKTFWD_UNLK();		// --------------------------------------------------------

} /* dhd_pktfwd_set_keymap() */

/** Returns pktfwd key for a given flowid and vice-versa based on k2f */
static int
dhd_pktfwd_get_keymap(uint32_t radio_idx, uint16_t * pktfwd_key,
		uint16_t * flowid, uint16_t prio, bool k2f)
{
	uint16_t dest;
	dhd_pktfwd_keymap_t *dhd_pktfwd_keymap;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	PKTFWD_LOCK();		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	dhd_pktfwd_keymap = dhd_pktfwd->dhd_pktfwd_keymap[radio_idx];

	if (k2f == PKTFWD_KEYMAP_K2F) {	/* PKTFWD_KEYMAP_K2F - key to flowid */
		dest = PKTLIST_DEST(*pktfwd_key);
		*flowid = dhd_pktfwd_keymap->pktfwdkey_flowid[prio][dest];
	} else {		/* PKTFWD_KEYMAP_F2K - flowid to key */
		*pktfwd_key = dhd_pktfwd_keymap->flowid_pktfwdkey[*flowid];
	}

	PKTFWD_UNLK();		// --------------------------------------------------------

	return BCME_OK;
} /* dhd_pktfwd_get_keymap */

/**
 * -----------------------------------------------------------------------------
 * Function : Helper routine to reset a d3fwd_wlif object by freeing all packets
 * -----------------------------------------------------------------------------
 */
static void _dhd_pktwd_d3fwd_wlif_reset(d3fwd_wlif_t * d3fwd_wlif)
{
	d3fwd_wlif->ucast_bmap = 0U;	/* no pending work */

	d3fwd_wlif->osh = (osl_t *) NULL;
	d3fwd_wlif->wlif = NULL;
	d3fwd_wlif->net_device = (struct net_device *)NULL;
	d3fwd_wlif->wl_schedule = ~0;	/* set to WLAN thread as scheduled */
	d3fwd_wlif->unit = WLIF_UNIT_INVALID;	/* 4bit: scribble */
#if defined(BCM_WFD)
	d3fwd_wlif->wfd_idx = ~0;	/* scribble */
#endif
	d3fwd_wlif->stations = 0;	/* none */
	d3fwd_wlif->flags = 0;		/* none */

} /* _dhd_pktwd_d3fwd_wlif_reset() */

/**
 * -----------------------------------------------------------------------------
 * Function : Helper routine to free all packet in a pktlist.
 * CAUTION: DHD does not support PKTC like chaining
 * -----------------------------------------------------------------------------
 */

static void /* Without osh accounting */ dhd_pktwd_pktlist_free(osl_t * osh,
								pktlist_t *
								pktlist_free)
{
	pktlist_pkt_t *pkt;

	PKTFWD_FUNC();

	while (pktlist_free->len--) {
		pkt = pktlist_free->head;

		ASSERT(NBUFF_PTR_TYPE(pkt) == DHD_NBUFF_PTR_TYPE);

		pktlist_free->head = PKTLIST_PKT_SLL(pkt, DHD_NBUFF_PTR_TYPE);

		PKTLIST_PKT_FREE(pkt);
	}

	return;

} /* dhd_pktwd_pktlist_free() */

/**
 * -----------------------------------------------------------------------------
 * Function :
 *    1. decouple wds_d3lut_elem with d3lut_wlif
 *    2. Recycle sta_list for WDS back to d3lut_wlif sta_pool
 *    3. free wds_d3lut_elem
 * -----------------------------------------------------------------------------
 */

static int _dhd_pktwd_free_wds_d3lut_elem(d3fwd_wlif_t * d3fwd_wlif)
{
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	int ret = 0;

	if (d3fwd_wlif->wds_d3lut_elem) {
		//Recycle wds sta back to sta pool
		d3lut_sta_t *sta = NULL;
		d3lut_elem_t *d3lut_elem = NULL;

		d3lut_elem = d3fwd_wlif->wds_d3lut_elem;
		d3fwd_wlif->wds_d3lut_elem = NULL;

		while (!dll_empty(&(d3lut_elem)->sta_list)) {	/* dwds d3lut_elem */
			sta =
			    (d3lut_sta_t *) dll_head_p(&(d3lut_elem)->sta_list);

			memset(sta->mac, 0, ETHER_ADDR_LEN);
			sta->d3lut_elem = NULL;
			dll_delete(&sta->node);	/* Move to free list */
			dll_append(&d3fwd_wlif->sta_free_list, &sta->node);
		}

		(void)d3lut_del(dhd_pktfwd->d3lut, d3lut_elem->sym.v8,
				d3lut_elem->key.domain);
		ret = 1;
	}

	return ret;
}

static int dhd_pktwd_free_wds_d3lut_elem(d3fwd_wlif_t * d3fwd_wlif)
{
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	int ret = 0;

	D3LUT_LOCK(dhd_pktfwd->d3lut);	// ++++++++++++++++++++++++++++++++++++++++++
	ret = _dhd_pktwd_free_wds_d3lut_elem(d3fwd_wlif);
	D3LUT_UNLK(dhd_pktfwd->d3lut);	// ------------------------------------------

	return ret;
}

static void dhd_pktwd_d3fwd_wlif_reset(d3fwd_wlif_t * d3fwd_wlif)
{
	int prio;
	osl_t *osh;
	dll_t *worklist;
	pktlist_t pktlist_free;
	pktlist_context_t *dhdif_pktlist_context;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	osh = d3fwd_wlif->osh;
	PKTLIST_RESET(&pktlist_free);	/* len = 0U, key.v16 = don't care */

	PKTFWD_ASSERT(d3fwd_wlif->unit < DHD_PKTFWD_RADIOS);
	dhdif_pktlist_context = dhd_pktfwd->pktlist_context[d3fwd_wlif->unit];

	PKTFWD_ASSERT(dhdif_pktlist_context != PKTLIST_CONTEXT_NULL);

	/* Flush all pending packets accumulated in pktlists tracked by worklist */
	worklist = &d3fwd_wlif->mcast;
	if (!dll_empty(worklist)) {
		__pktlist_xfer_pktlist(worklist, &pktlist_free, DHD_NBUFF_PTR_TYPE);
		dll_join(worklist, &dhdif_pktlist_context->free);
	}

	for (prio = 0; prio < D3FWD_PRIO_MAX; ++prio) {
		worklist = &d3fwd_wlif->ucast[prio];
		if (!dll_empty(worklist)) {
			__pktlist_xfer_pktlist(worklist, &pktlist_free, DHD_NBUFF_PTR_TYPE);
			dll_join(worklist, &dhdif_pktlist_context->free);
		}
	}

	dhd_pktwd_free_wds_d3lut_elem(d3fwd_wlif);
	_dhd_pktwd_d3fwd_wlif_reset(d3fwd_wlif);

	dhd_pktfwd->stats.pkts_dropped += pktlist_free.len;

	/* kfree all packets in the pktlist_free */
	dhd_pktwd_pktlist_free(osh, &pktlist_free);

} /* dhd_pktwd_d3fwd_wlif_reset() */

/**
 * -----------------------------------------------------------------------------
 *
 * Function : Register a WLAN Interface with the PKTFWD subsystem
 *            Allocate a d3fwd_wlif object from the free list to extend
 *            the wlif with a PKTFWD presence. The d3fwd_wlif extension is
 *            paired with the wlif and the parent net_device.
 * Operation: dhd_alloc_if, dhd_alloc_linux_if allocates a 1:1 <wlif,net_device>
 *            net_device is linked to a wlif by an extension to the net_device
 *            using priv_link_t. wlif->dev is setup.
 *            A call is then made (via dhd_pktc_init) to dhd_pktfwd_wlif_ins() to
 *            further extend the pair <wlif,net_device> with a d3fwd_wlif_t.
 *            In the case of the primary interface (dhd_attach), the interface
 *            may be registered with the pktfwd layer before the radio (dhd_info)
 *            is registered. This means that the dhd_info::wfd_idx may not be
 *            valid. (dhd_pktfwd_t -1, i.e. value 2 in a d3fwd_wlif_r 2bit field)
 * -----------------------------------------------------------------------------
 */

void *dhd_pktfwd_wlif_ins(void *wlif, void *osh, struct net_device *dev, unsigned int unit)
{
	int prio;
	d3fwd_wlif_t *d3fwd_wlif;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	PKTFWD_FUNC();

	PKTFWD_ASSERT(dhd_pktfwd->initialized == true);

	if (!DHD_PKTFWD_SUPPORTED(unit)) {
		/* PKTFWD not supported for this radio */
		return D3FWD_WLIF_NULL;
	}

	PKTFWD_LOCK();		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	if (dll_empty(&dhd_pktfwd->d3fwd_free)) {
		PKTFWD_ERROR("d3fwd_wlif pool depleted failure");
		d3fwd_wlif = D3FWD_WLIF_NULL;
		++dhd_pktfwd->stats.ops_failures;
		goto dhd_pktfwd_wlif_ins_done;	/* Unlock and bail */
	}

	/* Allocate a free d3fwd_wlif_t from free list and move to active list */
	d3fwd_wlif = (d3fwd_wlif_t *) dll_head_p(&dhd_pktfwd->d3fwd_free);
	dll_delete(&d3fwd_wlif->node);
	dll_append(&dhd_pktfwd->d3fwd_used, &d3fwd_wlif->node);
	dhd_pktfwd->wlif_cnt++;

	d3fwd_wlif->osh = osh;	/* same osh as radio */
	d3fwd_wlif->wlif = wlif;	/* pair with parent wl_if */
	d3fwd_wlif->net_device = dev;	/* pair with grand parent dev */
	d3fwd_wlif->ucast_bmap = 0U;
	d3fwd_wlif->wl_schedule = 0;	/* WLAN thread scheduled */
	d3fwd_wlif->unit = unit;	/* WLAN radio domain */
#if defined(BCM_WFD)
	/**
	 * In the case of primary wlif, wl::wfd_idx is not valid. After, a radio is
	 * registered (dhd_pktfwd_radio_ins) and wfd is bound (dhd_pktfwd_wfd_ins),
	 * the primary interface's wfd_idx will be explicitly set.
	 * For all other virtual interfaces, the wfd_idx is set here.
	 */
	d3fwd_wlif->wfd_idx = dhd_pktfwd->wfd_idx[unit];	/* maybe -1 */
#endif
	d3fwd_wlif->stations = 0;
	d3fwd_wlif->flags = 0;		/* none */
	dll_init(&d3fwd_wlif->mcast);
	for (prio = 0; prio < D3FWD_PRIO_MAX; ++prio)
		dll_init(&d3fwd_wlif->ucast[prio]);

dhd_pktfwd_wlif_ins_done:

	PKTFWD_UNLK();		// --------------------------------------------------------

	if (d3fwd_wlif == D3FWD_WLIF_NULL) {
		PKTFWD_ERROR("%s failure", __FUNCTION__);
		return NULL;
	}

	/* mark as DHD device */
	netdev_wlan_dhd_set(dev);

	PKTFWD_TRACE("%s success", __FUNCTION__);

	return (void *)d3fwd_wlif;

} /* dhd_pktfwd_wlif_ins() */

/**
 * -----------------------------------------------------------------------------
 * Function : Register a WLAN Radio with the PKTFWD subsystem
 * Operation: Allocate a pktlist_context for the wlan (consumer). This context
 *            will be registered with WFD during dhd_wfd_bind().
 *            Bind all exported hooks. Binding may be redundantly done again
 *            when a another radio goes through attach.
 * -----------------------------------------------------------------------------
 */

void *dhd_pktfwd_radio_ins(dhd_pub_t * dhdp, unsigned int unit, struct net_device *dev)
{
	uint16_t domain;
	int mem_bytes;
	char dhd_name[IFNAMSIZ];	/* eth%d overwrite */
	pktqueue_t *pktqueue;
	pktqueue_table_t *pktqueue_table;
	dhd_pktfwd_keymap_t *dhd_pktfwd_keymap;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	PKTFWD_FUNC();
	PKTFWD_ASSERT(dhd_pktfwd->initialized == true);

	if (!DHD_PKTFWD_SUPPORTED(unit)) {
		/* PKTFWD not supported for this radio */
		PKTFWD_ERROR("dhd<%d> exceeded max unit %d", unit, DHD_PKTFWD_RADIOS);
		goto dhd_pktfwd_radio_ins_failure;
	}

	(void)snprintf(dhd_name, sizeof(dhd_name), "dhd%d", unit);

	/**
	 * dhd_wfd_bind() is not yet invoked, when dhd_pktc_attach is invoked.
	 * so pointless trying to setup dhd_pktfwd_g::wfd_idx[], leave as -1.
	 */
	PKTFWD_LOCK();		// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	dhd_pktfwd->radio_cnt++;
	PKTFWD_UNLK();		// ---------------------------------------------------------

	/* Configure WLAN radio d3lut pool policy */
	PKTFWD_TRACE("Pool #%d D3LUT_POLICY_POOL_BY_INDEX", dhdp->unit);
	d3lut_policy_set(dhd_pktfwd->d3lut, dhdp->unit, D3LUT_POLICY_POOL_BY_INDEX);

	/* Instantiate a WLAN consumer pktlist_context */
	PKTFWD_ASSERT(dhd_pktfwd->pktlist_context[unit] == PKTLIST_CONTEXT_NULL);
	dhd_pktfwd->pktlist_context[unit] = pktlist_context_init(PKTLIST_CONTEXT_PEER_NULL,
			PKTLIST_CONTEXT_XFER_NULL, dhd_pktfwd_get_keymap, dev, dhd_name, unit);

	if (dhd_pktfwd->pktlist_context[unit] == PKTLIST_CONTEXT_NULL) {
		PKTFWD_WARN("pktlist_context unit %d failure", unit);
		goto dhd_pktfwd_radio_ins_failure;
	}

	/* Construct WLAN producer pktqueue_table */
	mem_bytes = sizeof(pktqueue_table_t);
	pktqueue_table = (pktqueue_table_t *) kmalloc(mem_bytes, GFP_ATOMIC);
	if (pktqueue_table == PKTQUEUE_TABLE_NULL) {
		PKTFWD_WARN("pktqueue_table unit %d failure", unit);
		goto dhd_pktfwd_radio_ins_failure;
	}
	memset(pktqueue_table, 0, mem_bytes);
	for (domain = 0; domain < PKTQUEUE_QUEUES_MAX; domain++) {
		pktqueue = PKTQUEUE_TBL_QUEUE(pktqueue_table, domain);
		pktqueue->NBuffPtrType = DHD_NBUFF_PTR_TYPE;	/* never reset */
		PKTQUEUE_RESET(pktqueue);	/* head,tail, not reset */
	}

	dhd_pktfwd->pktqueue_table[unit] = pktqueue_table;

	/* Construct DHD pktfwd keymap */
	mem_bytes = sizeof(dhd_pktfwd_keymap_t);
	dhd_pktfwd_keymap =
	    (dhd_pktfwd_keymap_t *) kmalloc(mem_bytes, GFP_ATOMIC);
	if (dhd_pktfwd_keymap == DHD_PKTFWD_KEYMAP_NULL) {
		PKTFWD_WARN("dhd_pktfwd_keymap unit %d failure", unit);
		goto dhd_pktfwd_radio_ins_failure;
	}
	memset(dhd_pktfwd_keymap, 0xFF, mem_bytes);	/* ID16_INVALID */
	dhd_pktfwd->dhd_pktfwd_keymap[unit] = dhd_pktfwd_keymap;

#if defined(BCM_CPE_DHD_GSO) && defined(SW_GSO_PKTLIST)
	dhd_gsopktlist_init(unit);
#endif /* BCM_CPE_DHD_GSO && SW_GSO_PKTLIST */

	PKTFWD_TRACE("wl%d register success", unit);

	return (void *)dhd_pktfwd;

dhd_pktfwd_radio_ins_failure:

	dhd_pktfwd_radio_del(unit);

	PKTFWD_ERROR("wl%d register failure", unit);

	return (void *)NULL;

} /* dhd_pktfwd_radio_ins() */

/**
 * -----------------------------------------------------------------------------
 * Function : Unregister a WLAN radio with the PKTFWD subsystem
 *            Do not destruct the pktfwd if there are no more radios. This will
 *            be done in the dhd_pktfwd_sys_fini (rmmod action).
 * -----------------------------------------------------------------------------
 */

void dhd_pktfwd_radio_del(unsigned int unit)
{
	int mem_bytes;
	pktqueue_table_t *pktqueue_table;
	dhd_pktfwd_keymap_t *dhd_pktfwd_keymap;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	PKTFWD_FUNC();
	PKTFWD_ASSERT(dhd_pktfwd->initialized == true);

	if (!DHD_PKTFWD_SUPPORTED(unit)) {
		/* PKTFWD not supported for this radio */
		return;
	}

	dhd_pktfwd->pktlist_context[unit] =
	    pktlist_context_fini(dhd_pktfwd->pktlist_context[unit]);

	/* Destruct pktqueue_table */
	pktqueue_table = dhd_pktfwd->pktqueue_table[unit];
	dhd_pktfwd->pktqueue_table[unit] = PKTQUEUE_TABLE_NULL;
	if (pktqueue_table != PKTQUEUE_TABLE_NULL) {
		mem_bytes = sizeof(pktqueue_table_t);
		memset(pktqueue_table, 0xFF, mem_bytes);	/* scribble */
		kfree(pktqueue_table);
	}

	/* Destruct pktfwd keymap */
	dhd_pktfwd_keymap = dhd_pktfwd->dhd_pktfwd_keymap[unit];
	dhd_pktfwd->dhd_pktfwd_keymap[unit] = DHD_PKTFWD_KEYMAP_NULL;
	if (dhd_pktfwd_keymap != DHD_PKTFWD_KEYMAP_NULL) {
		mem_bytes = sizeof(dhd_pktfwd_keymap_t);
		memset(dhd_pktfwd_keymap, 0, mem_bytes);	/* scribble */
		kfree(dhd_pktfwd_keymap);
	}

	PKTFWD_LOCK();
	dhd_pktfwd->wfd_idx[unit] = -1;
	dhd_pktfwd->radio_cnt--;
	PKTFWD_UNLK();

	PKTFWD_TRACE("wl%d deregistered", unit);

} /* dhd_pktfwd_radio_del() */

#if defined(BCM_WFD)
/**
 * -----------------------------------------------------------------------------
 * Function : Invoked on dhd_wfd_bind() to register a wfd_idx with PKTFWD. The
 *            primary wlif's d3fwd_wlif object is updated with the wfd_idx.
 * -----------------------------------------------------------------------------
 */

void
dhd_pktfwd_wfd_ins(struct net_device *dev, unsigned int wfd_idx, unsigned int unit)
{
	d3fwd_wlif_t *d3fwd_wlif;	/* primary interface */
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	dhd_pktfwd_priv_t *dhd_pktfwd_priv;

	PKTFWD_FUNC();
	PKTFWD_ASSERT(wfd_idx >= 0);

	if (!DHD_PKTFWD_SUPPORTED(unit)) {
		/* PKTFWD not supported for this radio */
		return;
	}

	dhd_pktfwd_priv = dhd_pktfwd_get_priv(dev);
	d3fwd_wlif = dhd_pktfwd_priv->d3fwd_wlif;

	if (unit != wfd_idx)
		PKTFWD_ERROR("wl u%d mismatch wfd_idx %d\n", unit, wfd_idx);

	if (d3fwd_wlif == D3FWD_WLIF_NULL) {
		PKTFWD_ERROR("wl%d no primary interface", unit);
		return;
	}

	PKTFWD_LOCK();		// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	dhd_pktfwd->wfd_idx[unit] = wfd_idx;

	/* Fix up the primary interfaces wfd_idx, now */
	d3fwd_wlif->wfd_idx = (uint8_t) wfd_idx;	/* actually 4 bits */

	PKTFWD_UNLK();		// ---------------------------------------------------------

	PKTFWD_TRACE("wl%d wfd_idx %d register success", unit, wfd_idx);

} /* dhd_pktfwd_wfd_ins() */

/**
 * -----------------------------------------------------------------------------
 * Function : Invoked on dhd_wfd_unbind() to de-register the wfd_idx for a radio.
 * -----------------------------------------------------------------------------
 */

void dhd_pktfwd_wfd_del(unsigned int unit)
{
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	PKTFWD_FUNC();

	if (!DHD_PKTFWD_SUPPORTED(unit)) {
		/* PKTFWD not supported for this radio */
		return;
	}

	PKTFWD_LOCK();
	dhd_pktfwd->wfd_idx[unit] = -1;
	PKTFWD_UNLK();

	PKTFWD_TRACE("wl%d wfd_idx deregister", unit);

} /* dhd_pktfwd_wfd_del() */

static void dhd_pktfwd_wfd_dbg(dhd_pub_t * dhdp, struct bcmstrbuf *b)
{
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	// use dhdp->wfd_idx to invoke a wfd_dump by wfd_idx ... not available.
	if ((dhdp != (dhd_pub_t *) NULL) && (dhdp->unit < DHD_PKTFWD_RADIOS))
		bcm_bprintf(b, "\t dhd%d wfd_idx %d pktfwd %u\n", dhdp->unit,
				dhdp->wfd_idx, dhd_pktfwd->wfd_idx[dhdp->unit]);

} /* dhd_pktfwd_wfd_dbg() */

#endif /* BCM_WFD */

/**
 * -----------------------------------------------------------------------------
 * Function : Deregister a WLAN Interface with the PKTFWD subsystem.
 *            Locate the d3fwd_wlif extension, reset its state and free it.
 * -----------------------------------------------------------------------------
 */
void dhd_pktfwd_wlif_del(d3fwd_wlif_t * d3fwd_wlif)
{
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	PKTFWD_FUNC();
	PKTFWD_ASSERT(dhd_pktfwd->initialized == true);

	if (d3fwd_wlif == D3FWD_WLIF_NULL) {
		PKTFWD_WARN("%s not registered", __FUNCTION__);
		return;
	}

	if (d3fwd_wlif->stations != 0) {	/* cautionary warning */
		PKTFWD_WARN("%s stations %u warning", __FUNCTION__, d3fwd_wlif->stations);
	}

	/* Reset entire d3fwd_wlif state */
	dhd_pktwd_d3fwd_wlif_reset(d3fwd_wlif);

	PKTFWD_LOCK();		// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	dll_delete(&d3fwd_wlif->node);	/* Move to free list */
	dll_append(&dhd_pktfwd->d3fwd_free, &d3fwd_wlif->node);

	PKTFWD_UNLK();		// --------------------------------------------------------

	D3LUT_LOCK(dhd_pktfwd->d3lut);	// ++++++++++++++++++++++++++++++++++++++++++

	/* ignore_ext_match = false, implying clear elements with matching */
	d3lut_clr(dhd_pktfwd->d3lut, d3fwd_wlif, /* ignore_ext_match = */
		  false);

	D3LUT_UNLK(dhd_pktfwd->d3lut);	// ------------------------------------------

} /* dhd_pktfwd_wlif_del() */

/**
 * -----------------------------------------------------------------------------
 * Function : Debug dump a WLAN Interface's PKTFWD context
 * -----------------------------------------------------------------------------
 *  TODO: Not being called until we find a way to pass strbuf
 *  to dump PKTFWD stats
 */

void dhd_pktfwd_wlif_dbg(d3fwd_wlif_t * d3fwd_wlif, struct bcmstrbuf *b)
{
	if (d3fwd_wlif == D3FWD_WLIF_NULL) {
		printk("dhdif dbg: d3fwd_wlif not registered\n");
		return;
	}

	d3fwd_wlif_dump(d3fwd_wlif);
} /* dhd_pktfwd_wlif_dbg() */

/**
 * -----------------------------------------------------------------------------
 * Function : Update WLAN Interface Link Status
 * -----------------------------------------------------------------------------
 */
void dhd_pktfwd_link_update(dhd_pub_t * dhdp, wl_event_msg_t * event)
{
	struct net_device *net_device;
	uint8 ifidx;
	uint flags;

	flags = ntoh16(event->flags);

	if ((flags & WLC_EVENT_MSG_LINK) == 0) {
		/* wl link down */
		ifidx = (uint8) dhd_ifname2idx(dhdp->info, event->ifname);

		net_device = dhd_idx2net(dhdp, ifidx);
		PKTFWD_ASSERT(net_device != NULL);

		/* cleanup d3lut associated with the device */
		dhd_pktfwd_lut_clr(net_device);
	}
}

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
dhd_pktfwd_cache(uint8_t * d3addr, struct net_device *net_device, bool perim_held)
{
	bool is_wlan, cache_eligible;
	d3lut_elem_t *d3lut_elem;
	unsigned long wfd_chain_idx = WL_PKTFWD_KEY_INVALID_UL;
	struct net_device *root_net_device = net_device;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	dhd_pub_t *dhdp = NULL;
	bool perim_unlock = false;

	PKTFWD_PTRACE(D3LUT_SYM_FMT "%s", D3LUT_SYM_VAL(d3addr), net_device->name);

	/* pktfwd is disabled */
	if (dhd_pktfwd_accel == false)
		goto dhd_pktfwd_cache_failure;

	/* Get the root net device for further processing */
	root_net_device = netdev_path_get_root(net_device);
	is_wlan = is_netdev_wlan(root_net_device);

	/* pktfwd acceleration is not allowed for wan device */
	if (is_netdev_wan(root_net_device)) {
		PKTFWD_WARN
		    ("%s: dev name=%s is WAN device, pktfwd acceleration not allowed!\n",
		     __FUNCTION__, root_net_device->name);
		goto dhd_pktfwd_cache_failure;
	}

	/* pktfwd acceleration is not allowed for vlan device created by vconfig */
	if (is_vlan_dev(net_device)) {
		PKTFWD_WARN("%s: dev name=%s is VLAN device created by vconfig,"
			" pktfwd acceleration not allowed!\n",
			__FUNCTION__, net_device->name);
		goto dhd_pktfwd_cache_failure;
	}
#ifdef CONFIG_VETH
	/* pktfwd acceleration is not allowed for veth device */
	if ((net_device->rtnl_link_ops != NULL) &&
			(!strcmp(net_device->rtnl_link_ops->kind, "veth"))) {
		PKTFWD_WARN("%s: dev name=%s is VETH device, pktfwd acceleration not allowed!\n",
			__FUNCTION__, net_device->name);
		goto dhd_pktfwd_cache_failure;
	}
#endif

	if (is_wlan) {
		dhd_pktfwd_priv_t *dhd_pktfwd_priv;

		dhd_pktfwd_priv = dhd_pktfwd_get_priv(root_net_device);

		ASSERT(dhd_pktfwd_priv);

		dhdp = g_dhd_info[dhd_pktfwd_priv->radio_unit];
		ASSERT(dhdp);

		if (is_netdev_vlan(root_net_device)) {
			PKTFWD_WARN
			    ("%s: dev name=%s is VLAN device, pktfwd acceleration not allowed!\n",
			     __FUNCTION__, root_net_device->name);
			goto dhd_pktfwd_cache_failure;
		}

		cache_eligible =
			(dhd_pktfwd_priv->ifp && (dhd_pktfwd_priv->d3fwd_wlif != D3FWD_WLIF_NULL))
				? true : false;	/* only WLAN NIC endpoints, exclude DHD endpoints */

		if ((dhd_dwds_ap_pktfwd_accel == false) && dhd_pktfwd_priv->d3fwd_wlif &&
				is_netdev_wlan_dwds_ap(dhd_pktfwd_priv->d3fwd_wlif))
			goto dhd_pktfwd_cache_failure;
		if ((dhd_dwds_sta_pktfwd_accel == false) && dhd_pktfwd_priv->d3fwd_wlif &&
				is_netdev_wlan_dwds_client(dhd_pktfwd_priv->d3fwd_wlif))
			goto dhd_pktfwd_cache_failure;
	} else {
		/* Allow default LAN vlan devices (e.g., ethx.0, treat
		 * them as untag devices) to create d3lut_elem for acceleration.
		 * Otherwise, no pktfwd accel for LAN vlan devices.
		 * i.e. PKTFWD acceleration is not allowed when dst is behind
		 * eth1.110 but allowed for eth1.0
		 */
		if (is_netdev_vlan(net_device)) {
			int len = strlen(net_device->name);
			if ((len > 3) && !((net_device->name[len - 2] == '.') &&
					(net_device->name[len - 1] == '0'))) {
				goto dhd_pktfwd_cache_failure;
			}
		}
		/* LAN endpoints, exclude bonding endpoints */
		cache_eligible = (root_net_device->priv_flags & IFF_BONDING) ? false : true;
	}

	if (cache_eligible == false)
		goto dhd_pktfwd_cache_failure;

	if (dhdp) {
		if (perim_held) {
			DHD_PERIM_ASSERT(dhdp);
		} else {
			DHD_LOCK(dhdp);
			perim_unlock = true;
		}
	}
	D3LUT_LOCK(dhd_pktfwd->d3lut);	// ++++++++++++++++++++++++++++++++++++++++++

	/* Insert into D3LUT */
	d3lut_elem = dhd_pktfwd_lut_ins(d3addr, net_device, is_wlan);

	if (d3lut_elem == D3LUT_ELEM_NULL) {	/* collision maybe or a D3LUT error */
		PKTFWD_WARN("DHD_PKTFWD_KEY_INVALID_UL");
		goto dhd_pktfwd_cache_fail_unlock;
	}

	if (is_wlan) {		/* audit D3LUT cached element ... no lock though */
		PKTFWD_ASSERT(d3lut_elem->ext.d3fwd_wlif != D3FWD_WLIF_NULL);
		PKTFWD_ASSERT(d3lut_elem->ext.d3fwd_wlif->unit == d3lut_elem->key.domain);
	} else {
		PKTFWD_ASSERT(d3lut_elem->key.domain == PKTFWD_XDOMAIN_IDX);
	}

	PKTFWD_PTRACE(D3LUT_ELEM_FMT, D3LUT_ELEM_VAL(d3lut_elem));

	/* 2b wfdidx | 16 chainidx (3b domain, 1b incarn, 12b index) */
	wfd_chain_idx =
		(unsigned long)PKTC_WFD_CHAIN_IDX(d3lut_elem->key.domain, d3lut_elem->key.v16);

dhd_pktfwd_cache_fail_unlock:

	D3LUT_UNLK(dhd_pktfwd->d3lut);	// ------------------------------------------
	if (perim_unlock)
		DHD_UNLOCK(dhdp);
	return wfd_chain_idx;

dhd_pktfwd_cache_failure:

	PKTFWD_WARN("DHD_PKTFWD_KEY_INVALID_UL");
	return DHD_PKTFWD_KEY_INVALID_UL;

} /* dhd_pktfwd_cache() */

/**
 * -----------------------------------------------------------------------------
 *
 * Function : Requests from bridge, wlan subsystems. See enum dhd_pktfwd_req_t.
 *            Retaining the dhd_pktc_req() signature along with PKTC and PKTC_TBL
 *
 * Operation: Invoked by Linux bridge and WLAN driver via dhd_pktc_req_hook().
 *
 * -----------------------------------------------------------------------------
 */

unsigned long
dhd_pktfwd_request(int request, unsigned long param0,
	unsigned long param1, unsigned long param2)
{
	unsigned long response = 0UL;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	PKTFWD_ASSERT((request > dhd_pktfwd_req_undefined_e) && (request < dhd_pktfwd_req_max_e));
	PKTFWD_PTRACE("%s", dhd_pktfwd_req_str[request]);

	PKTFWD_ASSERT(dhd_pktfwd->initialized == true);

	if (dhd_pktfwd->initialized == false)
		goto dhd_pktfwd_request_done;

	switch (request) {
		/* IOVAR: set Transmit Acceleration of bridged traffic */
	case dhd_pktfwd_req_set_txmode_e:	/* param0: 1 = enable, 0 = disable */
		dhd_pktfwd->tx_accel = (uint8_t) param0;
		break;

		/* IOVAR: get Transmit Acceleration of bridged traffic */
	case dhd_pktfwd_req_get_txmode_e:	/* response: 1 = enable, 0 = disable */
		response = (unsigned long)dhd_pktfwd->tx_accel;
		break;

	case dhd_pktfwd_req_ins_symbol_e: /* param0: d3addr, param1: dev_p, param2: perim */
		response = (unsigned long)dhd_pktfwd_cache((uint8_t *)param0,
				(struct net_device *)param1, (bool)param2);
		break;

		/* ndo_uninit (net_dev deregister or register failure: dhd_uninit() */
	case dhd_pktfwd_req_flush_full_e:	/* param0: net_device */
		dhd_pktfwd_lut_clr((struct net_device *)param0);
		break;

	case dhd_pktfwd_req_assoc_sta_e:	/* param0: d3addr, param1: 0|1 assoc */
		/* param2: eventtype */
		if (param2 == WLC_E_ASSOC) {
			/* If a station is roaming, there might be stale entries in d3lut.
			 * Look for symbol in global pool and delete if found.
			 */
			dhd_pktfwd_lut_del((uint8_t *) param0, (struct net_device *)NULL);
		}

		if (param2 == WLC_E_ASSOC_IND)
			++dhd_pktfwd->stats.tot_stations;
		else if (param2 == WLC_E_DISASSOC_IND)
			--dhd_pktfwd->stats.tot_stations;

		/* LUT is not populated on association, for consistency in bridge */
		break;

	case dhd_pktfwd_req_pktlist_e:	/* param0: unit */

		if (DHD_PKTFWD_SUPPORTED(param0)) {
			if (dhd_pktfwd->pktlist_context[param0] !=
			    PKTLIST_CONTEXT_NULL)
				response =
				    (unsigned long)dhd_pktfwd->
				    pktlist_context[param0];
			else
				PKTFWD_ERROR
				    ("pktlist_context %lu not initialized",
				     param0);
		} else {
			response = (unsigned long)PKTLIST_CONTEXT_NULL;
		}
		break;

	case dhd_pktfwd_req_bridge_event_e:	/* param0: addr, param1: device, param2: event */
		{
			struct net_device *dev = (struct net_device *)param1;

			if (param2 == SWITCHDEV_FDB_DEL_TO_DEVICE) {
				/* Get the root net device for further processing */
				dev = netdev_path_get_root(dev);
				dhd_pktfwd_lut_del((uint8_t *) param0, dev);
			}
		}
		break;

	default:
		++dhd_pktfwd->stats.ops_failures;
		PKTFWD_ERROR("request %u invalid", request);
		break;

	} /* switch */

dhd_pktfwd_request_done:

	return response;

} /* dhd_pktfwd_request() */

/* helper function to return matched sta from sta list */
d3lut_sta_t *_d3lut_find_sta(dll_t * sta_list, uint8_t * d3addr)
{
	int i = 0;
	dll_t *item;
	d3lut_sta_t *sta_found = NULL;

	if (dll_empty(sta_list))
		return NULL;

	dll_for_each(item, sta_list) {
		d3lut_sta_t *sta = (d3lut_sta_t *) item;

		if (memcmp(sta->mac, d3addr, ETHER_ADDR_LEN) == 0) {
			sta_found = sta;
			goto done;
		}
		if (i++ > PKTFWD_ENDPOINTS_MAX) {
			if (printk_ratelimit())
				PKTFWD_ERROR
				    ("%s: ERROR sta num is greater than %d!\n",
				     __FUNCTION__, PKTFWD_ENDPOINTS_MAX);
			goto done;
		}
	}
done:
	return sta_found;
}

d3lut_elem_t *
dhd_pktfwd_lut_lkup(struct net_device * net_device, d3lut_t * d3lut,
	uint8_t * d3_addr, uint32_t pool)
{
	int dev;
	d3lut_elem_t *d3lut_elem = NULL;
	d3fwd_wlif_t *d3fwd_wlif;
	dhd_pktfwd_priv_t *dhd_pktfwd_priv;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	/* check WDS case first */
	if ((net_device != NULL) && is_netdev_wlan(net_device)) {

		dhd_pktfwd_priv = dhd_pktfwd_get_priv(net_device);
		d3fwd_wlif = dhd_pktfwd_priv->d3fwd_wlif;

		if ((D3LUT_LKUP_GLOBAL_POOL != pool) && d3fwd_wlif && (d3fwd_wlif->unit != pool)) {
			PKTFWD_ERROR("%s: pool %d and unit %d mismatched\n",
				__FUNCTION__, pool, d3fwd_wlif->unit);
			goto done;
		}
		if (d3fwd_wlif && ((is_netdev_wlan_dwds_ap(d3fwd_wlif) ||
				is_netdev_wlan_dwds_client(d3fwd_wlif)) &&
				(d3fwd_wlif->wds_d3lut_elem != NULL))) {
			d3lut_elem = d3fwd_wlif->wds_d3lut_elem;
			goto done;
		}
	}

	/* for infrastructure cases */
	d3lut_elem = d3lut_lkup(d3lut, d3_addr, pool);

	/* look for existing wds d3lut_elem to match d3addr from wlan upstream
	 * Overwrite d3lut_elem only if the lkup is invoked for wlan device
	 */
	if ((d3lut_elem == NULL) && ((net_device == NULL) || is_netdev_wlan(net_device))) {
		d3fwd_wlif = dhd_pktfwd->d3fwd_wlif;	/* d3fwd_wlif pool base */
		for (dev = 0; dev < WL_PKTFWD_DEVICES; ++dev, ++d3fwd_wlif) {
			if ((D3LUT_LKUP_GLOBAL_POOL != pool) && (d3fwd_wlif->unit != pool))
				continue;

			if (d3fwd_wlif->wds_d3lut_elem != NULL) {
				d3lut_sta_t *sta;

				/* check if addr matches */
				sta =
				    _d3lut_find_sta(&d3fwd_wlif->
						    wds_d3lut_elem->sta_list,
						    d3_addr);
				if (sta != NULL) {
					return sta->d3lut_elem;
				}
			}
		}
	}

done:
	return d3lut_elem;
}

void * /* d3lut_elem_t * */
dhd_pktfwd_lut_ins(uint8_t * d3addr, struct net_device *net_device, bool is_wlan)
{
	int ssid = ~0, d3domain;	/* d3addr in d3domain (by WLAN radio and LAN) */
	d3lut_policy_t d3lut_policy;
	d3lut_elem_t *d3lut_elem = D3LUT_ELEM_NULL;
	void *if_handle;
	dhd_pub_t *dhdp;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	uint16_t staidx = ID16_INVALID;
	d3fwd_wlif_t *d3fwd_wlif = D3FWD_WLIF_NULL;
	struct net_device *virt_net_device = net_device;	/* save virtual device */
	uint8_t *d3addr_orig = d3addr;

	PKTFWD_ASSERT(d3addr != (uint8_t *) NULL);
	PKTFWD_ASSERT(net_device != (struct net_device *)NULL);
	PKTFWD_ASSERT(dhd_pktfwd->initialized == true);
	PKTFWD_PTRACE(D3LUT_SYM_FMT "%s", D3LUT_SYM_VAL(d3addr), net_device->name);

	/* WLAN and LAN domain: pool+policy selection and interface handle */

	/* convert net device to be root */
	net_device = netdev_path_get_root(net_device);

	if (is_wlan) {
		uint32_t radio_idx = -1;
		dhd_pktfwd_priv_t *dhd_pktfwd_priv;

		dhd_pktfwd_priv = dhd_pktfwd_get_priv(net_device);
		ASSERT(dhd_pktfwd_priv);
		d3fwd_wlif = dhd_pktfwd_priv->d3fwd_wlif;

		radio_idx = dhd_pktfwd_priv->radio_unit;
		ssid = dhd_pktfwd_priv->ifidx;
		dhdp = g_dhd_info[radio_idx];

		ASSERT(dhdp);

		PKTFWD_ASSERT(BLOG_GET_PHYTYPE
			      (netdev_path_get_hw_port_type(net_device)) ==
			      BLOG_WLANPHY);
		if ((dhdp == NULL) || (!dhdp->up) ||
			(d3fwd_wlif == D3FWD_WLIF_NULL) ||
			(d3fwd_wlif->unit == WLIF_UNIT_INVALID)) {
			PKTFWD_WARN("d3fwd_wlif absent failure");
			++dhd_pktfwd->stats.ops_failures;

			goto dhd_pktfwd_lut_ins_done;
		}

		if (!d3fwd_wlif->net_device) {
			PKTFWD_WARN("d3fwd_wlif netdevice absent failure");
			++dhd_pktfwd->stats.ops_failures;
			goto dhd_pktfwd_lut_ins_done;
		}

		/* don't add itself to the d3lut.
		 * This is caused by the SWITCHDEV_FDB_ADD_TO_DEVICE event when switching bridges.
		 */
		if (!memcmp(d3addr, d3fwd_wlif->net_device->dev_addr, ETHER_ADDR_LEN)) {
			goto dhd_pktfwd_lut_ins_done;
		}

		/* if d3lut_elem exists for dwds, add sta and return it */
		if ((is_netdev_wlan_dwds_ap(d3fwd_wlif) ||
				is_netdev_wlan_dwds_client(d3fwd_wlif))) {
			if (d3fwd_wlif->wds_d3lut_elem != NULL) {
				d3lut_sta_t *sta;
				bool exist = false;

				/* check if exists */
				sta = _d3lut_find_sta(&d3fwd_wlif->wds_d3lut_elem->sta_list,
						d3addr);
				if (sta != NULL)
					exist = true;

				/* add d3addr sta to sta list if it doesn't exist */
				if (exist == false) {
					if (dll_empty(&d3fwd_wlif->sta_free_list)) {
						PKTFWD_WARN("sta_free_list is empty");
						goto dhd_pktfwd_lut_ins_done;
					}

					d3lut_elem = dhd_pktfwd_lut_lkup(NULL,
							dhd_pktfwd->d3lut,
							d3addr, D3LUT_LKUP_GLOBAL_POOL);
					if (d3lut_elem != D3LUT_ELEM_NULL) {
						struct net_device *dev;
						if (d3lut_elem->ext.wlan)
							dev = d3lut_elem->ext.d3fwd_wlif->
								net_device;
						else
							dev = d3lut_elem->ext.net_device;

						_dhd_pktfwd_lut_del(d3addr, d3lut_elem,
							d3lut_elem->key.domain, dev);
					}
					sta = (d3lut_sta_t *)
						dll_head_p(&d3fwd_wlif->sta_free_list);
					memcpy(sta->mac, d3addr, ETHER_ADDR_LEN);
					sta->d3lut_elem = d3fwd_wlif->wds_d3lut_elem;
					dll_delete(&sta->node);
					dll_prepend(&d3fwd_wlif->wds_d3lut_elem->sta_list,
						&sta->node);
				}
				d3lut_elem = d3fwd_wlif->wds_d3lut_elem;
				goto dhd_pktfwd_lut_ins_done;
			} else {
#if defined(BCM_PKTFWD_DWDS)
				dhd_alloc_dwds_idx(dhdp, ssid /* ifidx */);
#endif
				/* assigned WDS peer macaddr for d3lut_elem creation. */
				if (!is_zero_ether_addr(d3fwd_wlif->wds_peer_mac))
					d3addr = d3fwd_wlif->wds_peer_mac;
			}
		}

		d3domain = radio_idx;
		if_handle = (void *)d3fwd_wlif;	/* wlif extension */

		/* Cant use flowid here since flowid is 16 bit while d3lut key is 12 bits */
		staidx = dhd_if_get_staidx(dhdp, ssid /* ifidx */ , d3addr);

		if (staidx == ID16_INVALID) {
			goto dhd_pktfwd_lut_ins_done;
		}

		d3lut_policy.pool_by_index = DHD_STAIDX2LUTID(staidx);
		PKTFWD_ASSERT(d3lut_policy.v16 < PKTFWD_ENDPOINTS_MAX);
	} else {		/* ! is_wlan */

		if_handle = (void *)(net_device);	/* for non-WLAN, use net_device */
		d3domain = (D3LUT_POOL_TOT - 1);	/* non-WLAN pool is last pool */
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
	d3lut_elem = d3lut_ins(dhd_pktfwd->d3lut, d3addr, d3domain, d3lut_policy);

	if (unlikely(d3lut_elem == D3LUT_ELEM_NULL)) {
		PKTFWD_WARN("d3lut_ins failure");
		++dhd_pktfwd->stats.ops_failures;

		goto dhd_pktfwd_lut_ins_done;
	}
	/* Save the virtual device pointer so that we
	 * can derive the upper layer bridge later
	 */
	d3lut_elem->ext.virt_net_device = virt_net_device;

	/* Data fill the d3lut extension */
	PKTFWD_ASSERT(d3lut_elem->ext.inuse == 1);
	if (is_wlan) {
		if (d3lut_elem->ext.assoc == 0) {
			d3lut_elem->ext.assoc = 1;	/* tag station as associated */
			d3lut_elem->ext.wlan = 1;

			d3lut_elem->ext.d3fwd_wlif = (d3fwd_wlif_t *) if_handle;
			if (d3lut_elem->ext.d3fwd_wlif->unit != WLIF_UNIT_INVALID)
				++d3lut_elem->ext.d3fwd_wlif->stations;

			/* Extension inherits element's key: domain incarn index
			 * Caution: key::index is global endpoint scoped
			 */
			d3lut_elem->ext.flowid = d3lut_elem->key.v16;
			d3lut_elem->ext.ssid = ssid;
		}

		/* keep this d3lut_elem for DWDS */
		if (d3fwd_wlif && (is_netdev_wlan_dwds_ap(d3fwd_wlif) ||
				is_netdev_wlan_dwds_client(d3fwd_wlif))) {
			d3lut_sta_t *sta = NULL;
			/* save d3lut_elem for dwds use */
			/* reset incarn as d3lut_elem is deleted due to roaming */
			d3lut_elem->key.incarn = 0;
			d3fwd_wlif->wds_d3lut_elem = d3lut_elem;

			/* add sta into list */
			if (!dll_empty(&d3fwd_wlif->sta_free_list)) {
				sta = (d3lut_sta_t *) dll_head_p(&d3fwd_wlif->sta_free_list);
				memcpy(sta->mac, d3addr_orig, ETHER_ADDR_LEN);
				sta->d3lut_elem = d3lut_elem;
				dll_delete(&sta->node);
				dll_append(&d3lut_elem->sta_list, &sta->node);
			} else {
				PKTFWD_WARN("sta_free_list is empty");
				d3lut_elem = D3LUT_ELEM_NULL;
			}
		}
	} else {		/* ! is_wlan */

		d3lut_elem->ext.net_device = (struct net_device *)if_handle;
		d3lut_elem->ext.ssid = ssid;
	}

dhd_pktfwd_lut_ins_done:

	return (void *)d3lut_elem;
} /* dhd_pktfwd_lut_ins() */

/**
 * -----------------------------------------------------------------------------
 * Function : Delete an endpoint from the PKTFWD LUT
 * HOOK_FN  : Invoked by Blog SHIM via dhd_pktc_del_hook
 * -----------------------------------------------------------------------------
 */
static inline void /* must be called with d3lut lock and non-null d3lut_elem */
_dhd_pktfwd_lut_del(uint8_t * d3addr, d3lut_elem_t * d3lut_elem,
	uint32_t d3domain, struct net_device *net_device)
{
	d3fwd_wlif_t *d3fwd_wlif = NULL;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	bool to_del = true;
	bool is_wlan = false;

	PKTFWD_ASSERT(d3lut_elem != D3LUT_ELEM_NULL);
	PKTFWD_ASSERT(dhd_pktfwd->initialized == true);

	is_wlan = d3lut_elem->ext.wlan;
	/* union of d3fwd_wlif is eth netdevice if is_wlan is false */
	d3fwd_wlif = d3lut_elem->ext.d3fwd_wlif;
	PKTFWD_ASSERT(d3fwd_wlif != D3FWD_WLIF_NULL);

	if (is_wlan && d3fwd_wlif &&
		(is_netdev_wlan_dwds_ap(d3fwd_wlif) || is_netdev_wlan_dwds_client(d3fwd_wlif))) {
		d3lut_sta_t *sta;

		/* check if exists */
		if (d3fwd_wlif->wds_d3lut_elem != NULL) {
			sta = _d3lut_find_sta(&d3fwd_wlif->wds_d3lut_elem->sta_list, d3addr);
			if (sta != NULL) {
				/* remove this sta from list */
				memset(sta->mac, 0, ETHER_ADDR_LEN);
				sta->d3lut_elem = NULL;
				dll_delete(&sta->node);	/* Move to free list */
				dll_append(&d3fwd_wlif->sta_free_list, &sta->node);
			}

			/*
			 * delete the d3lut_elem only when no more sta in the list for the dwds case
			 */
			if (!dll_empty(&d3fwd_wlif->wds_d3lut_elem->sta_list)) {
				to_del = false;
			}
		}
	}

	if ((to_del == true) && (d3lut_elem->ext.wlan && d3lut_elem->ext.assoc)) {
		d3lut_elem->ext.assoc = 0;	/* tag station as disassociated */
		d3lut_elem->ext.wlan = 0;
		d3lut_elem->ext.if_handle = (void *)NULL;

		if (d3fwd_wlif && (d3fwd_wlif->stations))
			--d3fwd_wlif->stations;	/* update d3fwd_wlif station count */
		/* Do we need to flush packets already in d3fwd_wlif::ucast ? No */
	}

	if (to_del == true) {
		/* regardless of dwds or non-dwds, clean up d3fwd_wlif.
		 * note: d3fwd_wlif could be eth dev pointer where d3fwd_wlif->wds_d3lut_elem
		 * is stall to cause crash.
		 */
		if (is_wlan && d3fwd_wlif && (d3fwd_wlif->wds_d3lut_elem != NULL)) {
			d3addr = d3fwd_wlif->wds_d3lut_elem->sym.v8;
			_dhd_pktwd_free_wds_d3lut_elem(d3fwd_wlif);
			/* reset peer mac */
			memset(&d3fwd_wlif->wds_peer_mac[0], 0, ETHER_ADDR_LEN);
#if defined(BCM_PKTFWD_DWDS)
			{
				dhd_pktfwd_priv_t *dhd_pktfwd_priv = NULL;
				net_device = d3fwd_wlif->net_device;

				if ((net_device) && is_netdev_wlan(net_device))
					dhd_pktfwd_priv = dhd_pktfwd_get_priv(net_device);

				if (dhd_pktfwd_priv) {
					int ssid = dhd_pktfwd_priv->ifidx;
					uint32_t radio_idx =
					    dhd_pktfwd_priv->radio_unit;

					dhd_pub_t *dhdp = g_dhd_info[radio_idx];
					dhd_free_dwds_idx(dhdp, ssid /* ifidx */);
				}
			}
#endif /* BCM_PKTFWD_DWDS */
		}
		(void)d3lut_del(dhd_pktfwd->d3lut, d3addr, d3domain);
	}
}

typedef void (*dhd_pktfwd_del_hook_t) (unsigned long addr, struct net_device * net_device);
void dhd_pktfwd_lut_del(uint8_t * d3addr, struct net_device *net_device)
{
	int d3domain;		/* d3addr in d3domain (by WLAN radio and LAN) */
	d3lut_elem_t *d3lut_elem;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	PKTFWD_TRACE(D3LUT_SYM_FMT, D3LUT_SYM_VAL(d3addr));

	if (net_device != NULL) {
		bool is_wlan, lut_del_eligible;

		is_wlan = is_netdev_wlan(net_device);

		if ((is_wlan && is_netdev_vlan(net_device)) || is_vlan_dev(net_device))
			return;

		if (is_wlan) {
			dhd_pktfwd_priv_t *dhd_pktfwd_priv = dhd_pktfwd_get_priv(net_device);
			ASSERT(dhd_pktfwd_priv);

			/* only WLAN NIC endpoints, exclude DHD endpoints */
			lut_del_eligible = (dhd_pktfwd_priv->ifp &&
					(dhd_pktfwd_priv->d3fwd_wlif != D3FWD_WLIF_NULL)) ? true :
					false;

			if (lut_del_eligible == false)
				return;

			if ((dhd_dwds_ap_pktfwd_accel == false) &&
					dhd_pktfwd_priv->d3fwd_wlif &&
					is_netdev_wlan_dwds_ap(dhd_pktfwd_priv->d3fwd_wlif))
				return;
			if ((dhd_dwds_sta_pktfwd_accel == false) &&
					dhd_pktfwd_priv->d3fwd_wlif &&
					is_netdev_wlan_dwds_client(dhd_pktfwd_priv->d3fwd_wlif))
				return;

			/* per radio's dedicated pool of d3lut_elem's */
			d3domain = dhd_pktfwd_priv->radio_unit;
		} else {	/* ! is_wlan */

			d3domain = (D3LUT_POOL_TOT - 1);	/* non-WLAN pool is last pool */
		}
	} else {		/* net_device == NULL */

		PKTFWD_WARN("net_device absent. Looking in global pool");
		d3domain = D3LUT_LKUP_GLOBAL_POOL;
	}

	D3LUT_LOCK(dhd_pktfwd->d3lut);	// ++++++++++++++++++++++++++++++++++++++++++

	d3lut_elem = dhd_pktfwd_lut_lkup(net_device, dhd_pktfwd->d3lut, d3addr, d3domain);

	if (d3lut_elem != D3LUT_ELEM_NULL) {
		if (d3domain == D3LUT_LKUP_GLOBAL_POOL)
			d3domain = d3lut_elem->key.domain;

		/* If WLAN and station was associated, update the d3fwd_wlif stats */
		_dhd_pktfwd_lut_del(d3addr, d3lut_elem, d3domain, net_device);
	}

	D3LUT_UNLK(dhd_pktfwd->d3lut);	// ------------------------------------------

} /* dhd_pktfwd_lut_del() */

/**
 * -----------------------------------------------------------------------------
 * Function : Check whether packets passed through a PKTFWD LUT entry.
 *
 * HOOK_FN  : Invoked by Linux bridge layer via fdb_check_expired_dhd_hook()
 *
 * Operation: Invoked by Linux bridge layer to determine whether a linux bridge
 * fdb entry may be refreshed as packets have passed through the pktfwd system
 * which is acting as a proxy bypass forwarding path to the linux bridge.
 * -----------------------------------------------------------------------------
 */

typedef int (*dhd_pktfwd_hit_hook_t) (uint8_t * d3addr, struct net_device * net_device);
int dhd_pktfwd_lut_hit(uint8_t * d3addr, struct net_device *net_device)
{
	bool is_wlan, lut_hit_eligible;
	int d3domain;		/* d3addr in d3domain (by WLAN radio and LAN) */
	int expired = -1;
	d3lut_elem_t *d3lut_elem;
	void *if_handle;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	d3lut_sta_t *sta = NULL;

	PKTFWD_ASSERT(dhd_pktfwd->initialized == true);

	/* pktfwd is disabled */
	if (dhd_pktfwd_accel == false)
		return expired;

	if (net_device != NULL) {
		is_wlan = is_netdev_wlan(net_device);

		if ((is_wlan && is_netdev_vlan(net_device)) || is_vlan_dev(net_device))
			return expired;

		if (is_wlan) {
			dhd_pktfwd_priv_t *dhd_pktfwd_priv;
			d3fwd_wlif_t *d3fwd_wlif;

			dhd_pktfwd_priv = dhd_pktfwd_get_priv(net_device);
			ASSERT(dhd_pktfwd_priv);

			/* only WLAN NIC endpoints, exclude DHD endpoints */
			lut_hit_eligible = (dhd_pktfwd_priv->ifp &&
				(dhd_pktfwd_priv->d3fwd_wlif != D3FWD_WLIF_NULL)) ? true : false;

			if (lut_hit_eligible == false)
				return expired;

			d3fwd_wlif = dhd_pktfwd_priv->d3fwd_wlif;

			if ((dhd_dwds_ap_pktfwd_accel == false) && d3fwd_wlif &&
					is_netdev_wlan_dwds_ap(d3fwd_wlif))
				return expired;
			if ((dhd_dwds_sta_pktfwd_accel == false) && d3fwd_wlif &&
					is_netdev_wlan_dwds_client(d3fwd_wlif))
				return expired;

			PKTFWD_ASSERT(BLOG_GET_PHYTYPE(
					netdev_path_get_hw_port_type(net_device)) == BLOG_WLANPHY);

			/* per radio's dedicated pool of d3lut_elem's */
			d3domain = dhd_pktfwd_priv->radio_unit;
			if_handle = (void *)d3fwd_wlif;	/* wlif extension */
		} else {	/* ! is_wlan */

			if_handle = (void *)(net_device);	/* for non-WLAN, use net_device */
			d3domain = (D3LUT_POOL_TOT - 1);	/* non-WLAN pool is last pool */
		}
	} else {		/* net_device == NULL */

		PKTFWD_WARN("net_device absent. Looking in global pool");
		d3domain = D3LUT_LKUP_GLOBAL_POOL;
		expired = 1;
	}

	D3LUT_LOCK(dhd_pktfwd->d3lut);	// ++++++++++++++++++++++++++++++++++++++++++

	d3lut_elem = dhd_pktfwd_lut_lkup(net_device, dhd_pktfwd->d3lut, d3addr, d3domain);

	if ((d3lut_elem != D3LUT_ELEM_NULL) && (d3lut_elem->ext.inuse)) {
		/* Sanity: validate interface */
		if (expired != 1) {
			if (is_wlan) {
				expired =
				    !((void *)(d3lut_elem->ext.d3fwd_wlif) ==
				      if_handle);
			} else {	/* !is_wlan */

				expired =
				    !((void *)(d3lut_elem->ext.net_device) ==
				      if_handle);
			}
		}

		/* d3lut_elem->sta_list is only used for dwds ap/client */
		if (!dll_empty(&d3lut_elem->sta_list)) {	/* dwds d3lut_elem */
			sta = _d3lut_find_sta(&d3lut_elem->sta_list, d3addr);
		}

		if (expired || ((sta == NULL) && (d3lut_elem->ext.hit == 0)) ||
		    ((sta != NULL) && (sta->flag == STA_FLAG_IDLE))) {
			/* delete d3lut_elem */
			_dhd_pktfwd_lut_del(d3addr, d3lut_elem, d3domain, net_device);
			expired = 1;
		} else {
			if (sta != NULL)
				sta->flag = STA_FLAG_IDLE;
			else
				d3lut_elem->ext.hit = 0;
			expired = 0;	/* return to br_fdb.c 0, to retain fdb entry */
		}
	}

	D3LUT_UNLK(dhd_pktfwd->d3lut);	// ------------------------------------------

	PKTFWD_PTRACE(D3LUT_SYM_FMT "expire %d", D3LUT_SYM_VAL(d3addr), expired);

	return expired;

} /* dhd_pktfwd_lut_hit() */

/**
 * -----------------------------------------------------------------------------
 * Function : Flush all stations in a D3LUT for a given virtual interface.
 * Operation: Invoked on a failure to register a WLAN Virtual Interface
 * or when a Virtual Interface's net_device is unregistered with the network
 * stack.
 *
 * See dhd_linux.c:: dhd_netdev_ops::ndo_uninit = dhd_uninit
 *    dhd_pktfwd_request(PKTC_TBL_FLUSH = dhd_pktfwd_req_flush_full_e)
 * -----------------------------------------------------------------------------
 */

void dhd_pktfwd_lut_clr(struct net_device *net_device)
{
	void *wlif;
	d3fwd_wlif_t *d3fwd_wlif;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	dhd_pktfwd_priv_t *dhd_pktfwd_priv;

	PKTFWD_FUNC();
	PKTFWD_ASSERT(dhd_pktfwd->initialized == true);
	/* Should be called only for wlan root interface */
	ASSERT(is_netdev_wlan(net_device) && !is_netdev_vlan(net_device));

	dhd_pktfwd_priv = dhd_pktfwd_get_priv(net_device);
	ASSERT(dhd_pktfwd_priv);
	wlif = dhd_pktfwd_priv->ifp;

	if (wlif) {
		d3fwd_wlif = dhd_pktfwd_priv->d3fwd_wlif;
		if (d3fwd_wlif != D3FWD_WLIF_NULL) {
			PKTFWD_ASSERT(d3fwd_wlif->wlif == wlif);
			PKTFWD_ASSERT(d3fwd_wlif->net_device == net_device);

			PKTFWD_ASSERT(d3fwd_wlif->ucast_bmap == 0U);
			dhd_pktwd_free_wds_d3lut_elem(d3fwd_wlif);
			d3fwd_wlif->stations = 0;
			D3FWD_STATS_EXPR(memset
					 (d3fwd_wlif->stats, 0,
					  sizeof(d3fwd_wlif->stats)));
		}
	} else {
		d3fwd_wlif = D3FWD_WLIF_NULL;
	}

	D3LUT_LOCK(dhd_pktfwd->d3lut);	// ++++++++++++++++++++++++++++++++++++++++++

	d3lut_clr(dhd_pktfwd->d3lut, d3fwd_wlif, d3fwd_wlif == D3FWD_WLIF_NULL);

	D3LUT_UNLK(dhd_pktfwd->d3lut);	// ------------------------------------------

} /* dhd_pktfwd_lut_clr() */

/** ====================================
 *  Proc filesystem to read stats
 *  ====================================
 */

static struct proc_dir_entry *proc_pktfwd_dir = NULL;	/* /proc/pktfwd_dhd */
static struct proc_dir_entry *proc_pktfwd_stats_file = NULL;	/* /proc/pktfwd_dhd/stats */
static struct proc_dir_entry *proc_pktfwd_enable_file = NULL;	/* /proc/pktfwd_dhd/enable */

static ssize_t
pktfwd_stats_file_read_proc(struct file *filep, char __user * page, size_t count, loff_t * data);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
static const struct file_operations stats_fops = {
	.owner = THIS_MODULE,
	.read = pktfwd_stats_file_read_proc,
};
#else
static const struct proc_ops stats_fops = {
	.proc_read = pktfwd_stats_file_read_proc,
};
#endif

static ssize_t
pktfwd_enable_file_read_proc(struct file *filep, char __user * page, size_t cnt, loff_t * offset);
static ssize_t
pktfwd_enable_file_write_proc(struct file *filep, const char *buf, size_t cnt, loff_t * offset);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
static const struct file_operations enable_fops = {
	.owner = THIS_MODULE,
	.read = pktfwd_enable_file_read_proc,
	.write = pktfwd_enable_file_write_proc,
};
#else
static const struct proc_ops enable_fops = {
	.proc_read = pktfwd_enable_file_read_proc,
	.proc_write = pktfwd_enable_file_write_proc,
};
#endif

/**
 * -----------------------------------------------------------------------------
 * Function : initialize the proc entry
 * -----------------------------------------------------------------------------
 */
static void pktfwd_proc_init(void)
{
	if (!(proc_pktfwd_dir = proc_mkdir("pktfwd_dhd", NULL)))
		return;		/* root dir creation failed */

	if (!
	    (proc_pktfwd_stats_file =
	     proc_create("pktfwd_dhd/stats", 0644, NULL, &stats_fops)))
		goto fail;

	if (!
	    (proc_pktfwd_enable_file =
	     proc_create("pktfwd_dhd/enable", 0644, NULL, &enable_fops)))
		goto fail;

	return;

fail:
	printk("%s %s: Failed to create proc /pktfwd_dhd\n", __FILE__,
	       __FUNCTION__);
	if (proc_pktfwd_dir != NULL) {
		if (proc_pktfwd_stats_file != NULL) {
			remove_proc_entry("stats", proc_pktfwd_dir);
			proc_pktfwd_stats_file = NULL;
		}
		if (proc_pktfwd_enable_file != NULL) {
			remove_proc_entry("enable", proc_pktfwd_dir);
			proc_pktfwd_enable_file = NULL;
		}
		remove_proc_entry("pktfwd_dhd", NULL);
		proc_pktfwd_dir = NULL;
	}
	return;
}

/**
 * -----------------------------------------------------------------------------
 * Function : initialize the proc entry
 * -----------------------------------------------------------------------------
 */
static void pktfwd_proc_fini(void)
{
	if (proc_pktfwd_dir != NULL) {
		if (proc_pktfwd_stats_file != NULL) {
			remove_proc_entry("stats", proc_pktfwd_dir);
			proc_pktfwd_stats_file = NULL;
		}
		if (proc_pktfwd_enable_file != NULL) {
			remove_proc_entry("enable", proc_pktfwd_dir);
			proc_pktfwd_enable_file = NULL;
		}
		remove_proc_entry("pktfwd_dhd", NULL);
		proc_pktfwd_dir = NULL;
	}
}

/**
 * -----------------------------------------------------------------------------
 * Function : pktfwd stats - proc file read handler
 *     using : cat /proc/pktfwd_dhd/stats
 * -----------------------------------------------------------------------------
 */
static ssize_t
pktfwd_stats_file_read_proc(struct file *filep, char __user * page, size_t count, loff_t * data)
{
	char buf[1024];
	struct bcmstrbuf strbuf;

	bcm_binit(&strbuf, buf, sizeof(buf));

	bcm_bprintf_bypass = TRUE;
	dhd_pktfwd_sys_dump(NULL, &strbuf);
	bcm_bprintf_bypass = FALSE;

	return 0;
}

/**
 * -----------------------------------------------------------------------------
 * Function : pktfwd enable - proc file read/write handler
 *     using : cat /proc/pktfwd_dhd/enable
 *             echo 0 > /proc/pktfwd_dhd/enable
 * -----------------------------------------------------------------------------
 */
static ssize_t
pktfwd_enable_file_read_proc(struct file *filep, char __user * page, size_t cnt, loff_t * offset)
{
	//EOF
	if (*offset != 0)
		return 0;

	*offset += sprintf(page + *offset, "%d\n", dhd_pktfwd_accel);

	return *offset;
}

static ssize_t
pktfwd_enable_file_write_proc(struct file *filep, const char *buf, size_t cnt, loff_t * offset)
{
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	char input[64];
	int32 argc = 0;
	uint32 val32;

	if (copy_from_user(input, buf, cnt) != 0)
		return -EFAULT;

	argc = sscanf(input, "%d\n", &val32);
	if (argc == 0) {
		printk("%s: wrong input\n", __FUNCTION__);
		return cnt;
	}

	dhd_pktfwd_accel = val32;
	printk("dhd_pktfwd_accel = %d\n", dhd_pktfwd_accel);

	if (dhd_pktfwd_accel == 0) {
		int wlif_cnt = 0;
		d3fwd_wlif_t *d3fwd_wlif;
		dll_t *item, *next;
		for (item = dll_head_p(&dhd_pktfwd->d3fwd_used);
		     !dll_end(&dhd_pktfwd->d3fwd_used, item); item = next) {
			next = dll_next_p(item);	/* iterator's next */
			d3fwd_wlif = (d3fwd_wlif_t *) item;

			D3LUT_LOCK(dhd_pktfwd->d3lut);	// +++++++++++++++++++++++++++++++++++++++++
			_dhd_pktwd_free_wds_d3lut_elem(d3fwd_wlif);
			d3lut_clr(dhd_pktfwd->d3lut, d3fwd_wlif, true);
			D3LUT_UNLK(dhd_pktfwd->d3lut);	// -----------------------------------------

			wlif_cnt++;
		}
		PKTFWD_ASSERT(wlif_cnt == dhd_pktfwd->wlif_cnt);
	}

	return cnt;
}

/**
 * -----------------------------------------------------------------------------
 * Function : Construct all pktfwd subsystems. This function is not re-entrant.
 *            Invoked in dhd_attach (see dhd_linux.c), per radio.
 *
 * Initialize the D3LUT system with a pool of D3LUT_ELEM_TOT elements.
 * All WLAN element pools will use the allocate by-index and
 * the last LAN pool will use by-freelist. This allows flowid to be
 * paired by-value with the element's per domain key::index.
 *
 * Initialize the pool of d3fwd_wlif(s) for all radios.
 *
 * -----------------------------------------------------------------------------
 */

int dhd_pktfwd_sys_init(void)
{
	int i, dev, mem_bytes;
	d3fwd_wlif_t *d3fwd_wlif;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	d3lut_sta_t *sta;

	PKTFWD_FUNC();

	if (dhd_pktfwd->initialized == true) {	/* global system already setup, bail */
		PKTFWD_ERROR("already initialized");
		return DHD_PKTFWD_SUCCESS;
	}

	PKTFWD_TRACE("D3LUT_ELEM %d %d\n", D3LUT_ELEM_TOT, D3LUT_ELEM_MAX);

	/*
	 * ---------------------------------------------
	 * Section: Initialize the PKTFWD global objects
	 * ---------------------------------------------
	 */
	spin_lock_init(&dhd_pktfwd->lock);

	dhd_pktfwd->d3lut = d3lut_gp;

	/* Initialize the pool of d3fwd_wlif_t objects */
	dll_init(&dhd_pktfwd->d3fwd_used);
	dll_init(&dhd_pktfwd->d3fwd_free);

	/* Pre-allocate pool of d3fwd_wlif_t and add each d3fwd_wlif to free list */
	mem_bytes = sizeof(d3fwd_wlif_t) * DHD_PKTFWD_DEVICES;
	dhd_pktfwd->d3fwd_wlif =
	    (d3fwd_wlif_t *) kmalloc(mem_bytes, GFP_ATOMIC);
	if (dhd_pktfwd->d3fwd_wlif == D3FWD_WLIF_NULL) {
		PKTFWD_ERROR("d3fwd_wlif kmalloc %d failure", mem_bytes);
		goto dhd_pktfwd_sys_init_failure;
	}
	memset(dhd_pktfwd->d3fwd_wlif, 0, mem_bytes);

	mem_bytes = sizeof(d3lut_sta_t) * PKTFWD_ENDPOINTS_MAX;
	d3fwd_wlif = dhd_pktfwd->d3fwd_wlif;	/* d3fwd_wlif pool base */
	for (dev = 0; dev < DHD_PKTFWD_DEVICES; ++dev, ++d3fwd_wlif) {
		_dhd_pktwd_d3fwd_wlif_reset(d3fwd_wlif);	/* reset scribble */
		dll_init(&d3fwd_wlif->node);	/* place into free list */
		dll_append(&dhd_pktfwd->d3fwd_free, &d3fwd_wlif->node);

		/* Pre-allocate pool of stations to free list */
		d3fwd_wlif->sta_pool =
		    (d3lut_sta_t *) kmalloc(mem_bytes, GFP_ATOMIC);
		if (d3fwd_wlif->sta_pool == NULL) {
			PKTFWD_ERROR("d3fwd_wlif->sta_pool kmalloc %d failure", mem_bytes);
			goto dhd_pktfwd_sys_init_failure;
		}
		memset(d3fwd_wlif->sta_pool, 0, mem_bytes);
		sta = d3fwd_wlif->sta_pool;
		dll_init(&d3fwd_wlif->sta_free_list);
		for (i = 0; i < PKTFWD_ENDPOINTS_MAX; i++, ++sta) {
			dll_init(&sta->node);
			dll_append(&d3fwd_wlif->sta_free_list, &sta->node);
		}
	}

	/*
	 * ---------------------------------------------------------------
	 * Section: Initialize the global network stack interfaces (hooks)
	 * ---------------------------------------------------------------
	 */
#if defined(PKTC)
	/* register with bridge and wlc */
	dhd_pktc_req_hook = dhd_pktfwd_request;
#endif
#if defined(BCM_BLOG)
	/* register hook with Blog Shim */
	dhd_pktc_del_hook = (dhd_pktfwd_del_hook_t) dhd_pktfwd_lut_del;
#endif
	/* register bridge refresh */
	fdb_check_expired_dhd_hook = (dhd_pktfwd_hit_hook_t) dhd_pktfwd_lut_hit;

	/* initialize proc entry */
	pktfwd_proc_init();

	{
		/* toggle pktfwd acceleration in dwds via nvram */
		char *var_ap = NULL;
		char *var_sta = NULL;
		char *var_pktfwd = NULL;
		var_ap = nvram_get(NVRAM_DWDS_AP_PKTFWD_ACCEL);
		if (var_ap != NULL)
			dhd_dwds_ap_pktfwd_accel = bcm_strtoul(var_ap, NULL, 0);
		var_sta = nvram_get(NVRAM_DWDS_STA_PKTFWD_ACCEL);
		if (var_sta != NULL)
			dhd_dwds_sta_pktfwd_accel =
			    bcm_strtoul(var_sta, NULL, 0);

		/* toggle pktfwd acceleration via nvram */
		var_pktfwd = nvram_get(NVRAM_PKTFWD_ACCEL);
		if (var_pktfwd != NULL)
			dhd_pktfwd_accel = bcm_strtoul(var_pktfwd, NULL, 0);
	}

#if defined(CONFIG_BCM_SW_GSO) && defined(BCM_CPE_DHD_GSO)
#ifdef AWL_LOCAL_IN_BYPASS_FILTER
	awl_localin_filter_proc_init();
#endif /* AWL_LOCAL_IN_BYPASS_FILTER */
#endif /* CONFIG_BCM_SW_GSO && BCM_CPE_DHD_GSO */

	dhd_pktfwd->initialized = true;

	PKTFWD_PRINT(PKTFWD_VRP_FMT " Success", PKTFWD_VRP_VAL(dhd_PKTFWD, DHD_PKTFWD_VERSIONCODE));

	return DHD_PKTFWD_SUCCESS;

dhd_pktfwd_sys_init_failure:

	PKTFWD_ERROR(PKTFWD_VRP_FMT " System Construction Failure",
		PKTFWD_VRP_VAL(dhd_PKTFWD, DHD_PKTFWD_VERSIONCODE));

	dhd_pktfwd_sys_fini();

	return DHD_PKTFWD_FAILURE;

} /* dhd_pktfwd_sys_init() */

/**
 * -----------------------------------------------------------------------------
 * Function : Destructor for the PKTFWD global system.
 *            Invoked by dhd_module_exit() and when dhd_pktfwd_sys_init() fails.
 * TBD      : Evaluate race conditions on exposed hooks.
 * -----------------------------------------------------------------------------
 */

void dhd_pktfwd_sys_fini(void)
{
	d3fwd_wlif_t *d3fwd_wlif;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	PKTFWD_FUNC();

#if defined(BCM_BLOG)
	dhd_pktc_del_hook = NULL;
#endif
#if defined(PKTC)
	dhd_pktc_req_hook = NULL;
#endif
	fdb_check_expired_dhd_hook = NULL;

	if (dhd_pktfwd->initialized == false) {
		if (dhd_pktfwd->wlif_cnt)
			PKTFWD_WARN("wlif_cnt %u", dhd_pktfwd->wlif_cnt);
		if (dhd_pktfwd->radio_cnt)
			PKTFWD_WARN("radio_cnt %u", dhd_pktfwd->radio_cnt);
		// PKTFWD_ASSERT(dhd_pktfwd->wlif_cnt == 0);
		// PKTFWD_ASSERT(dhd_pktfwd->radio_cnt == 0);
	}
#if defined(CONFIG_BCM_SW_GSO) && defined(BCM_CPE_DHD_GSO)
#ifdef AWL_LOCAL_IN_BYPASS_FILTER
	awl_localin_filter_proc_fini();
#endif /* AWL_LOCAL_IN_BYPASS_FILTER */
#endif /* CONFIG_BCM_SW_GSO && BCM_CPE_DHD_GSO */

	dhd_pktfwd->initialized = false;

	dhd_pktfwd->d3lut = D3LUT_NULL;

	dll_init(&dhd_pktfwd->d3fwd_used);
	dll_init(&dhd_pktfwd->d3fwd_free);
	d3fwd_wlif = dhd_pktfwd->d3fwd_wlif;	/* d3fwd_wlif pool */

	dhd_pktfwd->wlif_cnt = (int8_t) 0;
	dhd_pktfwd->radio_cnt = (int8_t) 0;

	if (d3fwd_wlif != D3FWD_WLIF_NULL) {	/* Destruct the d3fwd_wlif pool */
		int i, dev;
		int mem_bytes;
		d3lut_sta_t *sta;

		mem_bytes = sizeof(d3lut_sta_t) * PKTFWD_ENDPOINTS_MAX;
		for (dev = 0; dev < DHD_PKTFWD_DEVICES; ++dev, ++d3fwd_wlif) {
			dll_init(&d3fwd_wlif->sta_free_list);
			sta = d3fwd_wlif->sta_pool;
			for (i = 0; i < PKTFWD_ENDPOINTS_MAX; i++, ++sta) {
				dll_init(&sta->node);
			}
			if (d3fwd_wlif->sta_pool != NULL) {	/* Destruct the sta pool */
				memset(d3fwd_wlif->sta_pool, 0xFF, mem_bytes);
				kfree(d3fwd_wlif->sta_pool);
			}
			d3fwd_wlif->sta_pool = NULL;
		}

		mem_bytes = sizeof(d3fwd_wlif_t) * DHD_PKTFWD_DEVICES;
		memset(dhd_pktfwd->d3fwd_wlif, 0xFF, mem_bytes);
		kfree(dhd_pktfwd->d3fwd_wlif);
	}
	dhd_pktfwd->d3fwd_wlif = D3FWD_WLIF_NULL;

	/* de-initialize proc entry */
	pktfwd_proc_fini();

	PKTFWD_PRINT(PKTFWD_VRP_FMT " Complete",
		PKTFWD_VRP_VAL(dhd_PKTFWD, DHD_PKTFWD_VERSIONCODE));

} /* dhd_pktfwd_sys_fini() */

/** fetch the d3lut_elem_t given a symbol key::v32 */
static inline d3lut_elem_t *__d3lut_key_2_d3lut_elem(uint16_t key_v16)
{
	uint16 gbl_index;	/* global scoped endpoint index */
	d3lut_elem_t *d3lut_elem;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	gbl_index = PKTFWD_GBF(key_v16, D3LUT_KEY_INDEX);
	PKTFWD_ASSERT(gbl_index < PKTFWD_ENDPOINTS_WLAN);

	d3lut_elem = dhd_pktfwd->d3lut->elem_base + gbl_index;

	return d3lut_elem;

} /* __d3lut_key_2_d3lut_elem() */

/**
 * =============================================================================
 * Section: Transmit Packet Processing
 * =============================================================================
 */

static void /* move all packets to be dropped into pktlist_free */
dhd_pktfwd_xfer_pktlist_free(pktlist_context_t * dhd_pktlist_context,
	pktlist_elem_t * pktlist_elem, pktlist_t * pktlist_free)
{
	pktlist_t *pktlist_pkts;

	pktlist_pkts = &pktlist_elem->pktlist;

	PKTFWD_WARN("%s pktlist_free->len<%d> pktlist_pkts->len<%d> ",
		dhd_pktlist_context->driver_name, pktlist_free->len, pktlist_pkts->len);

	ASSERT(NBUFF_PTR_TYPE(pktlist_pkts->head) == DHD_NBUFF_PTR_TYPE);

	/* xfer all packets from pktlist_pkts to tail of pktlist_free */
	if (pktlist_free->len == 0u) {
		pktlist_free->head = pktlist_pkts->head;
		pktlist_free->tail = pktlist_pkts->tail;
	} else {
		PKTLIST_PKT_SET_SLL(pktlist_free->tail, pktlist_pkts->head, DHD_NBUFF_PTR_TYPE);
		pktlist_free->tail = pktlist_pkts->tail;
	}

	pktlist_free->len += pktlist_pkts->len;

	PKTLIST_RESET(pktlist_pkts);	/* head,tail, not reset */

	/* move pktlist_elem to the wlan pktlist_context's free list */
	dll_delete(&pktlist_elem->node);
	dll_append(&dhd_pktlist_context->free, &pktlist_elem->node);

	return;
} /* dhd_pktfwd_xfer_pktlist_free() */

static inline void
dhd_pktfwd_xfer_work_ucast(pktlist_context_t * dhd_pktlist_context,
	dll_t * uc_work, pktlist_t * pktlist_free, uint32_t prio)
{
	dll_t *item, *next;
	pktlist_elem_t *pktlist_elem;
	d3lut_elem_t *d3lut_elem;

	PKTFWD_PFUNC();

	/* Traverse a pktlist_context's ucast pending work list */
	for (item = dll_head_p(uc_work); !dll_end(uc_work, item); item = next) {
		next = dll_next_p(item);	/* iterator's next */
		pktlist_elem = _envelope_of(item, pktlist_elem_t, node);

		if (pktlist_elem->pktlist.key.v16 == ID16_INVALID)
			goto dhd_pktfwd_xfer_work_ucast_drop;	/* flowring has been evicted */

		/* pktlist_pkts had adopted first pkt's key, fetch the d3lut_elem */
		d3lut_elem =
		    __d3lut_key_2_d3lut_elem(pktlist_elem->pktlist.key.v16);

		/* FIXME:: No need for context switch??? wl schedules a threads
		 */
		if (dhd_pktfwd_pktlist_xmit(dhd_pktlist_context, pktlist_elem, d3lut_elem)) {
			goto dhd_pktfwd_xfer_work_ucast_drop;	/* fail d3fwd_wlif xfer */
		}

		PKTLIST_RESET(&pktlist_elem->pktlist);	/* len = 0U, key.v16 = ~0 */

		/* Move pktlist_elem to the WLAN pktlist_context's free list */
		dll_delete(&pktlist_elem->node);
		dll_append(&dhd_pktlist_context->free, &pktlist_elem->node);

		continue;	/* next pktlist in uncat worklist */

dhd_pktfwd_xfer_work_ucast_drop:	/* xfer all pkts to pktlist_free */
		dhd_pktfwd_xfer_pktlist_free(dhd_pktlist_context, pktlist_elem, pktlist_free);
	} /* for each pktlist_elem in uc_work list */

} /* dhd_pktfwd_xfer_work_ucast() */

/**
 * -----------------------------------------------------------------------------
 * Function : Helper routine to handle to Multicast packet
 * -----------------------------------------------------------------------------
 */
static void BCMFASTPATH
_dhd_pktfwd_mcasthandler(uint32_t radio_idx, uint16_t ifidx, pNBuff_t * pNBuf)
{
	int ret;
	dhd_pub_t *dhd_pub;
#if defined(DHD_WMF)
	dhd_wmf_t *wmf;
#endif /* DHD_WMF */

	dhd_pub = g_dhd_info[radio_idx];

	DHD_LOCK(dhd_pub);	//+++++++++++++++++++++++++

	if (dhd_idx2net(dhd_pub, ifidx) == NULL)
		goto dhd_pktfwd_mcasthandler_free;

#if defined(BCM_NBUFF_WLMCAST)
	if (PKTATTACHTAG(dhd_pub->osh, pNBuf)) {
		PKTFWD_ERROR("PKTTAG attach failed; dropping packet<%px>", pNBuf);
		goto dhd_pktfwd_mcasthandler_free;
	}
#endif /* defined(BCM_NBUFF_WLMCAST */

#if defined(BCM_NBUFF_WLMCAST)
	if (g_multicast_priority > 0) {
		PKTSETPRIO(pNBuf, g_multicast_priority);
	} else
#endif /* BCM_NBUFF_WLMCAST */
	{
		if (!PKTPRIO(pNBuf)) {
			pktsetprio(pNBuf, FALSE);
#ifndef BCM_WLAN_PER_CLIENT_FLOW_LEARNING
			if (!PKTPRIO(pNBuf))
				PKTSETPRIO(pNBuf, PRIO_8021D_VI);
#endif
		}
	}

#if defined(DHD_WMF)
	wmf = dhd_wmf_conf(dhd_pub, ifidx);

	if (wmf->wmf_enable) {
		/* set  WAN multicast indication before sending to EMF module */
		DHD_PKT_SET_WFD_BUF(pNBuf);
		DHD_PKT_SET_WAN_MCAST(pNBuf);

		PKTFWD_PTRACE("pakcet<%px> from WMF MCAST and is WANMCAST:%d",
			pNBuf, DHD_PKT_GET_WAN_MCAST(pNBuf));

		ret = dhd_wmf_packets_handle(dhd_pub, pNBuf, NULL, ifidx, 0);

		if (ret == WMF_TAKEN) {
			dhd_pub->tx_multicast++;
#ifdef BCM_WFD
			dhd_pub->tx_packets_wfd_mcast++;
#endif
			goto dhd_pktfwd_mcasthandler_success;
		} else if (ret == WMF_DROP) {
			goto dhd_pktfwd_mcasthandler_free;
		}
	}
#endif /* DHD_WMF */

	DHD_PKT_CLR_WFD_BUF(pNBuf);

	ret = dhd_flowid_update(dhd_pub, ifidx, dhd_pub->flow_prio_map[(PKTPRIO(pNBuf))], pNBuf);
	if (ret == BCME_OK) {
		ret = dhd_sendpkt(dhd_pub, ifidx, pNBuf);
		if (ret)
			goto dhd_pktfwd_mcasthandler_drop;
		else {
#ifdef BCM_WFD
			dhd_pub->tx_packets_wfd_mcast++;
#endif
			goto dhd_pktfwd_mcasthandler_success;
		}
	}

dhd_pktfwd_mcasthandler_free:
	PKTFREE(dhd_pub->osh, pNBuf, FALSE);

dhd_pktfwd_mcasthandler_drop:
	dhd_if_inc_txpkt_drop_cnt(dhd_pub, ifidx);
	dhd_pub->tx_dropped++;
#ifdef BCM_WFD
	dhd_pub->tx_packets_dropped_wfd_mcast++;
#endif

dhd_pktfwd_mcasthandler_success:

	DHD_UNLOCK(dhd_pub);	//-----------------------

	return;
} /* _dhd_pktfwd_mcasthandler() */

/**
 * -----------------------------------------------------------------------------
 * Function     : dhd_pktfwd_mcast_pktlist_xmit()
 * Description  : Make a copy of packets from multicast pktlist and trasfer to
 *                the network interfaces provided in the ssid_vector bitmap.
 *                For the last interface in ssid_vector, transmit the original
 *                packet.
 *
 * CAUTION : Invoked in "WFD" thread context
 * -----------------------------------------------------------------------------
 */
static inline void
dhd_pktfwd_mcast_pktlist_xmit(pktlist_context_t * dhd_pktlist_context,
	pktlist_t * pktlist_mcast, pktlist_t * pktlist_free)
{
	uint16_t ssid_vector, ifidx;
	dhd_pub_t *dhd_pub;
	pktlist_pkt_t *pkt;
	FkBuff_t *fkb, *fkb_cloned;
	struct net_device *net_device;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	int orig_pkt_handled = 0;

	dhd_pub = g_dhd_info[dhd_pktlist_context->unit];

	PKTFWD_ASSERT(pktlist_mcast->len != 0U);

	while (pktlist_mcast->len) {
		pkt = pktlist_mcast->head;
		pktlist_mcast->head = PKTLIST_PKT_SLL(pkt, FKBUFF_PTR);
		PKTLIST_PKT_SET_SLL(pkt, PKTLIST_PKT_NULL, FKBUFF_PTR);
		pktlist_mcast->len--;
		orig_pkt_handled = 0;

		fkb = (FkBuff_t *) PNBUFF_2_FKBUFF(pkt);
		ssid_vector = fkb->wl.mcast.ssid_vector;
		/* clear fkb dhdhdr,all fkb is master fkb here */
		dhd_clr_fkb_dhdhdr_flag(fkb);
		DHD_PKT_CLR_FKB_FLOW_UNHANDLED(FKBUFF_2_PNBUFF(fkb));

		while (ssid_vector) {
			ifidx = pktfwd_map_ssid_vector_to_ssid(&ssid_vector);
#if !defined(BCM_AWL) && defined(BCM_WFD)
			if (!wfd_dev_by_id_get
			    (dhd_pktlist_context->unit, ifidx))
				continue;
#endif
			net_device = dhd_idx2net(dhd_pub, ifidx);

			PKTFWD_ASSERT(net_device != NULL);

			if (ssid_vector) {	/* Don't make a copy for only/last interface */
				/* FKB clone */
				fkb_cloned = fkb_clone(fkb);

				if (fkb_cloned == (FkBuff_t *) NULL) {
					PKTFWD_ERROR
					    ("fkb clone failed; dropping packet<%px>",
					     fkb);
					break;
				}
			} else {
				orig_pkt_handled = 1;
				fkb_cloned = fkb;
			}

			PKTFWD_PTRACE("dhd<%d> pkt %px", dhd_pub->unit, fkb_cloned);
			dhd_pktfwd->stats.txf_chn_pkts++;
			dhd_pktfwd->stats.txf_mcast_pkts++;
			_dhd_pktfwd_mcasthandler(dhd_pktlist_context->unit, ifidx,
				FKBUFF_2_PNBUFF(fkb_cloned));
		} /* while (ssid_vector) */

		if (!orig_pkt_handled) {
			dhd_pktfwd->stats.pkts_dropped++;

			/* Add it to pktlist_free */
			if (pktlist_free->len != 0U) {	/* Do not use <head,tail> */
				/* Append to tail */
				PKTLIST_PKT_SET_SLL(pktlist_free->tail, pkt, FKBUFF_PTR);
				pktlist_free->tail = pkt;
			} else {
				/* Add packet to head */
				pktlist_free->head = pktlist_free->tail = pkt;
			}
			++pktlist_free->len;
		}

	} /* while (pktlist_mcast->len) */

} /* dhd_pktfwd_mcast_pktlist_xmit() */

static inline void
dhd_pktfwd_xfer_work_mcast(pktlist_context_t * dhd_pktlist_context,
	dll_t * mc_work, pktlist_t * pktlist_free)
{
	dll_t *item, *next;
	pktlist_t *pktlist_mcast;
	pktlist_elem_t *pktlist_elem;

	PKTFWD_PFUNC();

	for (item = dll_head_p(mc_work); !dll_end(mc_work, item); item = next) {
		next = dll_next_p(item);	/* iterator's next */
		pktlist_elem = _envelope_of(item, pktlist_elem_t, node);
		pktlist_mcast = &pktlist_elem->pktlist;

		dhd_pktfwd_mcast_pktlist_xmit(dhd_pktlist_context, pktlist_mcast, pktlist_free);

	}

} /* dhd_pktfwd_xfer_work_mcast() */

void /* Hook (peer) invoked by WFD to transfer all accumulated pktlists */
dhd_pktfwd_xfer_callback(pktlist_context_t * dhd_pktlist_context)
{
	uint32_t prio;
	dll_t *worklist;
	pktlist_t pktlist_free;	/* local pktlist, for lockless kfree_skb */

	PKTFWD_PFUNC();

	PKTLIST_RESET(&pktlist_free);	/* len = 0U, key.v16 = don't care */

	/* Transfer "all" mcast pktlist(s) to each interface's mcast worklist */
	worklist = &dhd_pktlist_context->mcast;
	if (!dll_empty(worklist)) {
		dhd_pktfwd_xfer_work_mcast(dhd_pktlist_context, worklist, &pktlist_free);
	}

	/* Transfer "all" ucast pktlist(s) to each interface's ucast worklist */
	for (prio = 0U; prio < PKTLIST_PRIO_MAX; ++prio) {
		worklist = &dhd_pktlist_context->ucast[prio];
		if (!dll_empty(worklist)) {
			dhd_pktfwd_xfer_work_ucast(dhd_pktlist_context, worklist, &pktlist_free,
					prio);
		}
	}

	/* Now "lockless", kfree any packets that needed to be dropped */
	if (pktlist_free.len) {
		pktlist_pkt_t *pkt, *npkt;
		dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

		dhd_pktfwd->stats.pkts_dropped += pktlist_free.len;

		for (pkt = (pktlist_pkt_t *) pktlist_free.head;
		     pkt != PKTLIST_PKT_NULL; pkt = npkt) {

			ASSERT(NBUFF_PTR_TYPE(pkt) == DHD_NBUFF_PTR_TYPE);

			npkt = PKTLIST_PKT_SLL(pkt, DHD_NBUFF_PTR_TYPE);
			PKTLIST_PKT_SET_SLL(pkt, PKTLIST_PKT_NULL, DHD_NBUFF_PTR_TYPE);

			/* No osh accounting, as not yet in DHD */
			PKTLIST_PKT_FREE(pkt);
		}
	}

} /* dhd_pktfwd_xfer_callback */

/** Test if packets in pktlist_t have a stale key (incarnation) mismatch */
static inline bool
dhd_pktfwd_xfer_pktlist_test(pktlist_t * pktlist_pkts, d3lut_elem_t * d3lut_elem)
{
	if (pktlist_pkts->key.v16 ^ d3lut_elem->key.v16) {
		PKTFWD_WARN("key mismatch: pktlist key %d elem key %d",
			pktlist_pkts->key.v16, d3lut_elem->key.v16);
#if (CC_D3FWD_DEBUG >= 3)
		d3fwd_ext_dump(&d3lut_elem->ext);
#endif
		return true;	/* pktlist needs to be dropped */
	}

	return false;
} /* dhd_pktfwd_xfer_pktlist_test() */

/**
 * Transpose a pktlist into a DHD compliant pkt list.
 *
 * Returns a DHD capable packet list, prepared: PKTTAG
 */
static inline void
dhd_pktfwd_pktlist_prepare(pktlist_t * pktlist, int ifidx, uint16_t flowid)
{
#if defined(NBUFF_IS_SKBUFF)
	struct sk_buff *xkbuff;
#else
	FkBuff_t *xkbuff;
#endif /* NBUFF_IS_SKBUFF */
	pNBuff_t pNBuf, pNBuf_sll;

	PKTFWD_PFUNC();

	pNBuf = (pNBuff_t) pktlist->head;
	ASSERT(NBUFF_PTR_TYPE(pNBuf) == DHD_NBUFF_PTR_TYPE);
#if defined(NBUFF_IS_SKBUFF)
	xkbuff = PNBUFF_2_SKBUFF(pNBuf);
#else
	xkbuff = (FkBuff_t *) PNBUFF_2_FKBUFF(pNBuf);
#endif /* NBUFF_IS_SKBUFF */

	bcm_prefetch(xkbuff);

	while (pNBuf != (pNBuff_t) NULL) {
		pNBuf_sll = PKTLIST_PKT_SLL(pNBuf, DHD_NBUFF_PTR_TYPE);
		PKTFWD_ASSERT(PKTLIST_PKT_SLL(pNBuf, DHD_NBUFF_PTR_TYPE) == PKTLINK(pNBuf));

#if defined(NBUFF_IS_SKBUFF)
		xkbuff = PNBUFF_2_SKBUFF(pNBuf);
#else
		xkbuff = (FkBuff_t *) PNBUFF_2_FKBUFF(pNBuf);
#endif /* NBUFF_IS_SKBUFF */
		/* Reset flags */
		DHD_PKTTAG_FD(pNBuf)->flags = 0;

		ASSERT(ifidx == xkbuff->wl.ucast.dhd.ssid);
		ASSERT(flowid == xkbuff->wl.ucast.dhd.flowring_idx);

		/* Save the flowid */
		DHD_PKT_SET_FLOWID(pNBuf, flowid);
#if !defined(BCM_EAPFWD)
		/* tag this packet as coming from wfd */
		DHD_PKT_SET_WFD_BUF(pNBuf);
#endif /* !defined(BCM_EAPFWD) */
		DHD_PKT_CLR_DATA_DHDHDR(pNBuf);
		pNBuf = pNBuf_sll;
	}
} /* dhd_pktfwd_pktlist_prepare */

#if defined(BCM_CPE_DHD_GSO) && defined(SW_GSO_PKTLIST)
/* Enqueue GSO PKTLIST to FlowRing */
int dhd_gsopktlist_flush(int radio_idx, pktlist_t * pktlist, uint16 flowid)
{
	int ifidx;
	int pkts_queued = 0;
	dhd_pub_t *dhd_pub;
	flow_ring_node_t *flow_ring_node;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	int ret = 0;
	pktlist_t pktlist_free;	/* local pktlist, for lockless kfree_skb */
	pktlist_pkt_t *pkt, *npkt;

	unsigned long flags;

	if (pktlist->len == 0)
		return ret;

	dhd_pub = g_dhd_info[radio_idx];

	DHD_LOCK(dhd_pub);	//+++++++++++++++++++++++++

	flow_ring_node = DHD_FLOW_RING(dhd_pub, flowid);

	DHD_FLOWRING_LOCK(flow_ring_node->lock, flags);

	if (((flow_ring_node->status != FLOW_RING_STATUS_PENDING) &&
			(flow_ring_node->status != FLOW_RING_STATUS_OPEN)) ||
			flow_ring_node->flowid != flowid ||
			flow_ring_node->active != TRUE) {
		PKTFWD_PTRACE("%s: on flowid %d when flow ring status is %d\r\n",
			__FUNCTION__, flowid, flow_ring_node->status);

		ret = BCME_ERROR;

		DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);
		goto dhd_gsopktlist_flush_done;
	}

	ifidx = flow_ring_node->flow_info.ifindex;

#if 0				//The task performed in dhd_bcmgsolist_out()
	/* Prepare pkttag for DHD */
	dhd_pktfwd_pktlist_prepare(pktlist, ifidx, flowid);
#endif

	/* Add to flowring backup queue */
#if defined(DHD_SBF)
	pkts_queued = dhd_pktfwd_pktlist_enqueue_sbf(dhd_pub, flowid, pktlist);
#else				/* !DHD_SBF */
	ret = dhd_pktfwd_pktlist_enqueue(dhd_pub, flowid,
			pktlist->head, pktlist->tail, pktlist->len);

	if (!ret) {
		/* On success all packets are queued */
		pkts_queued = pktlist->len;
		pktlist->len = 0;
	} else {
		/* On failure all packets are dropped */
		pkts_queued = 0;
	}
#endif /* !DHD_SBF */

	DHD_FLOWRING_UNLOCK(flow_ring_node->lock, flags);

	/* Flush all pending tx queued packets in bus(s) managed on this CPU core */
	if (pkts_queued) {
		dhd_pub->tx_packets += pkts_queued;

#if IS_ENABLED(CONFIG_BCM_DHD_ARCHER) || defined(BCM_DHD_RUNNER)
		if (DHD_FLOWRING_RNR_OFFL(flow_ring_node))
			/* from queue to flowring */
			dhd_bus_schedule_queue(dhd_pub->bus, flowid, FALSE);
		else
#endif
#ifdef BCM_WFD
			dhd_wfd_invoke_func(radio_idx, dhd_bus_txqueue_flush);
#endif

#ifdef BCM_WFD
		dhd_pub->tx_packets_wfd += pkts_queued;
#endif
		dhd_pktfwd->stats.txf_chn_pkts += pkts_queued;
	}

	/* Remaining pkts to be dropped */
	if (pktlist->len) {
		dhd_pub->tx_dropped += pktlist->len;
#ifdef BCM_WFD
		dhd_pub->tx_packets_dropped_wfd += pktlist->len;
#endif
		dhd_if_add_txpkt_drop_cnt(dhd_pub, ifidx, pktlist->len);
	}

dhd_gsopktlist_flush_done:

	/* xfer all packets from pktlist_pkts to tail of pktlist_free */
	PKTLIST_RESET(&pktlist_free);	/* len = 0U, key.v16 = don't care */

	if (pktlist_free.len == 0u) {
		pktlist_free.head = pktlist->head;
		pktlist_free.tail = pktlist->tail;
	} else {
		PKTLIST_PKT_SET_SLL(pktlist_free.tail, pktlist->head, FKBUFF_PTR);
		pktlist_free.tail = pktlist->tail;
	}

	pktlist_free.len += pktlist->len;

	/* Return number of packets to be dropped */
	ret = pktlist->len;

	PKTLIST_RESET(pktlist);	/* head,tail, not reset */

	DHD_UNLOCK(dhd_pub);	//-----------------------

	/* Now "lockless", kfree any packets that needed to be dropped */
	if (unlikely(pktlist_free.len > 0)) {
		dhd_pktfwd->stats.pkts_dropped += pktlist_free.len;

		for (pkt = (pktlist_pkt_t *) pktlist_free.head;
		     pkt != PKTLIST_PKT_NULL; pkt = npkt) {
			npkt = PKTLIST_PKT_SLL(pkt, FKBUFF_PTR);
			PKTLIST_PKT_SET_SLL(pkt, PKTLIST_PKT_NULL, FKBUFF_PTR);

			/* No osh accounting, as not yet in DHD */
			PKTLIST_PKT_FREE(pkt);
		}
	}

	return ret;
}
#endif /* BCM_CPE_DHD_GSO && SW_GSO_PKTLIST */

/* Transmit all the packets */
static int BCMFASTPATH
dhd_pktfwd_pktlist_xmit(pktlist_context_t * dhd_pktlist_context,
	pktlist_elem_t * pktlist_elem, d3lut_elem_t * d3lut_elem)
{
	uint16 flowid, dest;
	int ifidx, radio_idx, ret, pkts_queued;
	dhd_pub_t *dhd_pub;
	pktlist_t *pktlist;
	flow_ring_node_t *flow_ring_node;
	dhd_pktfwd_keymap_t *dhd_pktfwd_keymap;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	d3fwd_wlif_t *d3fwd_wlif;

	PKTLIST_ASSERT(dhd_pktlist_context != PKTLIST_CONTEXT_NULL);

	ret = BCME_OK;
	radio_idx = dhd_pktlist_context->unit;
	/* Locate the pktlist element and the contained pktlist */
	pktlist = &pktlist_elem->pktlist;
	dest = PKTLIST_DEST(pktlist_elem->pktlist.key.v16);

	dhd_pub = g_dhd_info[radio_idx];
	dhd_pktfwd_keymap = dhd_pktfwd->dhd_pktfwd_keymap[radio_idx];

	DHD_LOCK(dhd_pub);	//+++++++++++++++++++++++++

	/* Audit pktlists head and tail packet's key::incarnation with elem */
	if (dhd_pktfwd_xfer_pktlist_test(pktlist, d3lut_elem)) {
		PKTFWD_WARN("stale pkts detected\n");
		ret = BCME_ERROR;
		goto dhd_pktfwd_pktlist_xmit_done;
	}

	d3fwd_wlif = d3lut_elem->ext.d3fwd_wlif;
	if (d3fwd_wlif == D3FWD_WLIF_NULL) {
		PKTFWD_WARN("d3fwd_wlif deleted\n");
		ret = BCME_ERROR;
		goto dhd_pktfwd_pktlist_xmit_done;
	}

	flowid = dhd_pktfwd_keymap->pktfwdkey_flowid[pktlist->prio][dest];

	DHD_UNLOCK(dhd_pub);	//-----------------------

	D3LUT_LOCK(dhd_pktfwd->d3lut);	// ++++++++++++++++++++++++++++++++++++++++++
	if (!dll_empty(&d3lut_elem->sta_list)) {	/* dwds d3lut_elem */
		d3lut_sta_t *sta;
		pktlist_pkt_t *pkt;
		pkt = pktlist->head;
		sta = _d3lut_find_sta(&d3lut_elem->sta_list, (uint8_t *)PKTDATA(dhd_pub->osh, pkt));
		if (sta != NULL) {
			sta->flag = STA_FLAG_ACTIVE;
		}
	} else {
		d3lut_elem->ext.hit = 1;
	}
	D3LUT_UNLK(dhd_pktfwd->d3lut);	// ------------------------------------------

	if (flowid == ID16_INVALID) {
		pktlist_pkt_t *pkt;
		struct net_device *net_device;
		xmit_fn_t netdev_ops_xmit;

		PKTLIST_ASSERT(d3fwd_wlif != D3FWD_WLIF_NULL);

		net_device = d3fwd_wlif->net_device;
		netdev_ops_xmit =
		    (xmit_fn_t) (net_device->netdev_ops->ndo_start_xmit);

		/* Send each packet via Slow path */
		while (pktlist->len--) {
			pkt = pktlist->head;
			ASSERT(NBUFF_PTR_TYPE(pkt) == DHD_NBUFF_PTR_TYPE);
			pktlist->head =
			    PKTLIST_PKT_SLL(pkt, DHD_NBUFF_PTR_TYPE);
			PKTLIST_PKT_SET_SLL(pkt, PKTLIST_PKT_NULL, DHD_NBUFF_PTR_TYPE);

			/* Reset flags */
			DHD_PKTTAG_FD(pkt)->flags = 0;
			/* Update flow for this packet */
			DHD_PKT_SET_FKB_FLOW_UNHANDLED(pkt);

			netdev_ops_xmit(pkt, net_device); /* dhd_start_xmit() */
		}
		return BCME_OK;
	}

	DHD_LOCK(dhd_pub);	//+++++++++++++++++++++++++

	flow_ring_node = DHD_FLOW_RING(dhd_pub, flowid);
	if ((flow_ring_node->status != FLOW_RING_STATUS_PENDING) &&
	    (flow_ring_node->status != FLOW_RING_STATUS_OPEN)) {
		PKTFWD_PTRACE
		    ("%s: on flowid %d when flow ring status is %d\r\n",
		     __FUNCTION__, flowid, flow_ring_node->status);

		ret = BCME_ERROR;
		goto dhd_pktfwd_pktlist_xmit_done;
	}

	ifidx = flow_ring_node->flow_info.ifindex;

	/* Prepare pkttag for DHD */
	dhd_pktfwd_pktlist_prepare(pktlist, ifidx, flowid);

	/* Add to flowring backup queue */
#if defined(DHD_SBF)
	pkts_queued = dhd_pktfwd_pktlist_enqueue_sbf(dhd_pub, flowid, pktlist);
#else				/* !DHD_SBF */
	ret = dhd_pktfwd_pktlist_enqueue(dhd_pub, flowid,
			pktlist->head, pktlist->tail, pktlist->len);

	if (!ret) {
		/* On success all packets are queued */
		pkts_queued = pktlist->len;
		pktlist->len = 0;
	} else {
		/* On failure all packets are dropped */
		pkts_queued = 0;
	}
#endif /* !DHD_SBF */

	/* Return number of packets to be dropped */
	ret = pktlist->len;

	/* Flush all pending tx queued packets in bus(s) managed on this CPU core */
	if (pkts_queued) {
		dhd_pub->tx_packets += pkts_queued;
#ifdef BCM_WFD
		dhd_pub->tx_packets_wfd += pkts_queued;
		dhd_wfd_invoke_func(radio_idx, dhd_bus_txqueue_flush);
#endif
		dhd_pktfwd->stats.txf_chn_pkts += pkts_queued;
	}

	/* Remaining pkts to be dropped */
	if (pktlist->len) {
		dhd_pub->tx_dropped += pktlist->len;
#ifdef BCM_WFD
		dhd_pub->tx_packets_dropped_wfd += pktlist->len;
#endif
		dhd_if_add_txpkt_drop_cnt(dhd_pub, ifidx, pktlist->len);
	}

dhd_pktfwd_pktlist_xmit_done:

	DHD_UNLOCK(dhd_pub);	//-----------------------

	return ret;
} /* dhd_pktfwd_pktlist_xmit */

/**
 * =============================================================================
 * Section DHD Receive Packet Forwarding
 * =============================================================================
 */

/**
 * -----------------------------------------------------------------------------
 * DHD Receive Path Forwarding
 *
 * Function: Accumalate DHD recv packets into local packet queue (indexed by
 * domain).
 * Once all packets from RxCmpl ring are binned, DHD will invoke
 * dhd_pktfwd_flush_pktqueues().
 *
 * dhd_pktfwd_flush_pktqueues() is responsible for flushing all packets from
 * local packet queue to corresponding egress network device sw queues and
 * wakeup egress driver thread using registered flush_pkts_fn and
 * flush_complete_fn handlers.
 *
 * -----------------------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 *
 * Function   : dhd_pktfwd_pktqueue_add_pkt
 * Description: Tag packet with d3lut pktfwd_key_t and add it to a packet queue
 *              identified by the egress network device domain using the pktfwd
 *              802.3 MacAddr Lookup table.
 *
 * Impl Caveat: Supports only FKB frames
 *              NTKSTK packets are not enqueued to PktQueue and sent up
 *              immediately. This is done to avoid duplication of convoluted
 *              code sections of dhd_rx_frame().
 * -----------------------------------------------------------------------------
 */

int
dhd_pktfwd_pktqueue_add_pkt(dhd_pub_t * dhd_pub, struct net_device *rx_net_device, void *pkt)
{
	int ret = BCME_OK;
	uint16_t d3domain, pktfwd_key, prio, ether_type;
	uint8_t *d3_addr;
	d3lut_elem_t *d3lut_elem;
	pktqueue_t *pktqueue;
	pktqueue_table_t *pktqueue_table;
#if defined(NBUFF_IS_SKBUFF)
	struct sk_buff *xkbuff;
#else
	FkBuff_t *xkbuff;
#endif /* defined(NBUFF_IS_SKBUFF) */
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	struct ether_header *eh;

	PKTFWD_PTRACE("uint %d pkt %px", dhd_pub->unit, pkt);

	if ((dhd_pub == NULL) || (!DHD_PKTFWD_SUPPORTED(dhd_pub->unit))) {
		/* PKTFWD not supported for this radio */
		return BCME_UNSUPPORTED;
	}

	ASSERT(NBUFF_PTR_TYPE(pkt) == DHD_NBUFF_PTR_TYPE);

	eh = (struct ether_header *) PKTDATA(dhd_pub->osh, pkt);
#if defined(BCM_WFD) && defined(BCM_WLAN_NIC_RX_RNR_ACCEL)
	if (DHD_WFD_IDX_VALID(dhd_pub->unit) && (inject_to_fastpath) &&
		(eh->ether_type != hton16(ETHER_TYPE_BRCM)))
	{
		if (send_nbuff_to_wfd != NULL)
		{
			send_nbuff_to_wfd(pkt, rx_net_device);
			return BCME_OK;
		}
	}
#endif

#if defined(NBUFF_IS_SKBUFF)
	xkbuff = PNBUFF_2_SKBUFF(pkt);
#else
	xkbuff = (FkBuff_t *) PNBUFF_2_FKBUFF(pkt);
#endif /* defined(NBUFF_IS_SKBUFF) */

	d3_addr = (uint8_t *) eh->ether_dhost;

	if (ETHER_ISMULTI(d3_addr)) {	/* Only unicast packets are bridged via LUT */
		return BCME_UNSUPPORTED;
	}

	pktqueue_table = dhd_pktfwd->pktqueue_table[dhd_pub->unit];

	PKTFWD_LOCK();		// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	/* Bypass PKTFWD if
	 * - pktfwd is disabled
	 * - destination to self
	 * - VLAN tagged
	 * - Non IPv4 and IPv6
	 */
	ether_type = ntoh16(eh->ether_type);
	if ((dhd_pktfwd_accel == false) ||
		(!memcmp(d3_addr, rx_net_device->dev_addr, ETHER_ADDR_LEN)) ||
		((ether_type != ETHER_TYPE_IP) &&
		(ether_type != ETHER_TYPE_IPV6))) {

		dhd_pktfwd->stats.rx_slow_fwds++;
		d3domain = PKTQUEUE_NTKSTK_QUEUE_IDX;	/* Network stack traffic */
		ret = BCME_UNSUPPORTED;
		goto dhd_pktfwd_pktqueue_add_pkt_bypass;
	}

	D3LUT_LOCK(dhd_pktfwd->d3lut);	// +++++++++++++++++++++++++++++++++++++++++

	/* use d3_addr to lookup d3lut_elem */
	d3lut_elem =
	    dhd_pktfwd_lut_lkup(NULL, dhd_pktfwd->d3lut, d3_addr, D3LUT_LKUP_GLOBAL_POOL);
	prio = PKTPRIO(pkt);

	if (d3lut_elem != D3LUT_ELEM_NULL) {
		struct net_device *rx_br_dev = NULL, *dst_br_dev =
		    NULL, *dst_dev = NULL;
		bool rtnl_lock_taken = FALSE;

		dst_dev = d3lut_elem->ext.virt_net_device;

		/* if tx device is vlan on top of WLAN, pass it through the network
		 * stack to add vlan tag.
		 */
		if (d3lut_elem->ext.wlan && is_netdev_vlan(dst_dev)) {
			d3lut_elem = D3LUT_ELEM_NULL;
		} else {
			if (rtnl_trylock()) {
				rtnl_lock_taken = TRUE;
			}

			rx_br_dev =
			    bcm_netdev_master_upper_dev_get_nolock
			    (rx_net_device);
			dst_br_dev =
			    bcm_netdev_master_upper_dev_get_nolock(d3lut_elem->
								   ext.
								   virt_net_device);

			if (rtnl_lock_taken) {
				rtnl_unlock();
			}

			/* Check if the packet is destined to different bridge interface
			 * Pass it trough the network stack in that case
			 */
			if ((rx_br_dev == NULL) || (dst_br_dev != rx_br_dev)) {
				d3lut_elem = D3LUT_ELEM_NULL;
			}
		}
	}

	/* d3lut hit and Untagged frames */
	if (likely(d3lut_elem != D3LUT_ELEM_NULL)) {
		d3domain = d3lut_elem->key.domain;	/* WLAN to WLAN/LAN */
		/* update hit flag */
		d3lut_elem->ext.hit = 1;

		/* Reset dhd FlowInf and set pktfwd FlowInf */
		xkbuff->wl.u32 = 0U;

		pktfwd_key = PKTC_WFD_CHAIN_IDX(d3lut_elem->key.domain, d3lut_elem->key.v16);
		/* Tag packet with d3lut pktfwd_key_t  */
		xkbuff->wl.pktfwd.is_ucast = 1;
		xkbuff->wl.pktfwd.pktfwd_key = pktfwd_key;
		xkbuff->wl.pktfwd.wl_prio = prio;
		xkbuff->wl.pktfwd.ssid = d3lut_elem->ext.ssid;
	} else {		/* !d3lut_elem || VLAN */

		/* Send d3lut miss and VLAN tagged frames to Network stack */
		d3domain = PKTQUEUE_NTKSTK_QUEUE_IDX;	/* Network stack traffic */
	}

	D3LUT_UNLK(dhd_pktfwd->d3lut);	// -----------------------------------------

	if ((d3domain == PKTQUEUE_NTKSTK_QUEUE_IDX) ||
	    (pktqueue_get_domain_pktqueue_context(d3domain) ==
	     PKTQUEUE_CONTEXT_NULL)) {
		/* TODO: Attach Rx interface ifidx/net_device to packet and bin it to
		 * packet queue. In flush handler, send each packet in PktQueue to
		 * Network stack (using dhd_sendup()).
		 */
		PKTFWD_PTRACE
		    ("PktQueue context is not registered for domain<%d>",
		     d3domain);

		/* For DHD, NTKSTK packets are not enqueued to PktQueue and
		 * sent up immediately.
		 * This is done to avoid duplication of convoluted code sections of
		 * dhd_rx_frame().
		 */
		dhd_pktfwd->stats.rx_slow_fwds++;
		ret = BCME_UNSUPPORTED;
		goto dhd_pktfwd_pktqueue_add_pkt_bypass;
	}

	pktqueue = PKTQUEUE_TBL_QUEUE(pktqueue_table, d3domain);

	if (likely(pktqueue->len != 0U)) {	/* Do not use <head,tail> PKTQUEUE_RESET */
		PKTQUEUE_PKT_SET_SLL(pktqueue->tail, pkt, DHD_NBUFF_PTR_TYPE);
		pktqueue->tail = (pktqueue_pkt_t *) pkt;
	} else {
		pktqueue->head = (pktqueue_pkt_t *) pkt;
		pktqueue->tail = (pktqueue_pkt_t *) pkt;
	}

	pktqueue->len++;

dhd_pktfwd_pktqueue_add_pkt_bypass:

	D3FWD_STATS_EXPR({
		dhd_pktfwd_priv_t * dhd_pktfwd_priv;
		d3fwd_wlif_t * d3fwd_wlif;
		dhd_pktfwd_priv = dhd_pktfwd_get_priv(rx_net_device);
		d3fwd_wlif = dhd_pktfwd_priv->d3fwd_wlif;
		D3FWD_STATS_ADD(d3fwd_wlif->stats[prio].rx_tot_pkts, 1);
		if (d3domain != PKTQUEUE_NTKSTK_QUEUE_IDX)
			D3FWD_STATS_ADD(d3fwd_wlif->stats[prio].rx_fast_pkts, 1);
		else
			D3FWD_STATS_ADD(d3fwd_wlif->stats[prio].rx_slow_pkts, 1);
	});

	/* CAUTION: PKTQUEUE_PKT_SLL(pkt) is NOT terminated by NULL ! */

	PKTFWD_UNLK();		// ---------------------------------------------------------

	return ret;

} /* dhd_pktfwd_pktqueue_add_pkt() */

/*
 * -----------------------------------------------------------------------------
 *
 * Function   : dhd_pktfwd_flush_pktqueues
 * Description: Flush all accumulated recv packets to egress network device
 *              (WFD/LAN) packet queue.
 *
 * CAUTION:     Walking all packet queues without lock.
 * -----------------------------------------------------------------------------
 */

void dhd_pktfwd_flush_pktqueues(dhd_pub_t * dhd_pub)
{
	uint16_t domain;
	pktqueue_t *pktqueue;
	pktqueue_pkt_t *pkt;
	pktqueue_table_t *pktqueue_table;
	pktqueue_context_t *pktqueue_context;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	if ((dhd_pub == NULL) || (!DHD_PKTFWD_SUPPORTED(dhd_pub->unit))) {
		/* PKTFWD not supported for this radio */
		return;
	}

	pktqueue_table = dhd_pktfwd->pktqueue_table[dhd_pub->unit];
	domain = 0;

dhd_pktfwd_flush_pktqueues_continue:

	pktqueue = PKTQUEUE_TBL_QUEUE(pktqueue_table, domain);

	if (pktqueue->len != 0U) {
		/* osh::alloced sub */
		PKTACCOUNT(dhd_pub->osh, pktqueue->len, FALSE);

		pktqueue_context = pktqueue_get_domain_pktqueue_context(domain);

		if (pktqueue_context != PKTQUEUE_CONTEXT_NULL) {
			dhd_pktfwd->stats.rx_fast_fwds += pktqueue->len;
			pktqueue_context->pkts_stats += pktqueue->len;

			/* Flush packets from local pktqueue to egress device SW queue */
			pktqueue_context->flush_pkts_fn(pktqueue_context->
							driver, pktqueue);

			/* Wake egress network device thread: invoke "flush complete"
			 * handler to wake peer driver.
			 */
			pktqueue_context->flush_complete_fn(pktqueue_context->
							    driver);
			pktqueue_context->queue_stats++;
		} else {	/* Send each pkt to Network stack */

			dhd_pktfwd->stats.rx_slow_fwds += pktqueue->len;
			while (pktqueue->len) {
				pkt = pktqueue->head;

				pktqueue->head =
				    PKTQUEUE_PKT_SLL(pkt, DHD_NBUFF_PTR_TYPE);
				PKTQUEUE_PKT_SET_SLL(pkt, PKTQUEUE_PKT_NULL, DHD_NBUFF_PTR_TYPE);

				pktqueue->len--;

				/* Should not reach here */
				/* TODO: Send packet to Network Stack */
				PKTFWD_ERROR("Dropping packet %px\n", pkt);
				PKTQUEUE_PKT_FREE(pkt);
				dhd_pktfwd->stats.pkts_dropped++;

			} /* while (pktqueue->len) */
		}

		PKTQUEUE_RESET(pktqueue);	/* head,tail, not reset */

	}
	/* pktqueue->len != 0 */
	domain++;

	if (domain < PKTQUEUE_NTKSTK_QUEUE_IDX)
		goto dhd_pktfwd_flush_pktqueues_continue;

} /* dhd_pktfwd_flush_pktqueues() */

/**
 * -----------------------------------------------------------------------------
 * Function    : dhd_pktfwd_upstream
 * Description : Forward sk_buff (chain or sll) to the egress network device
 *               using the pktfwd 802.3 MacAddr Lookup table.
 *
 * -----------------------------------------------------------------------------
 */

int dhd_pktfwd_upstream(dhd_pub_t * dhdp, pNBuff_t pNBuff)
{
	uint8_t *d3_addr;
	struct net_device *net_device;
	d3lut_elem_t *d3lut_elem;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	struct ether_header *eh;

	PKTFWD_PTRACE("wl%d pNBuff %px", dhdp->unit, pNBuff);

#if defined(BCM_AWL) && defined(DHD_AWL_RX)
	/* Skip pktfwd for Archer upstream path */
	return BCME_UNSUPPORTED;
#endif /* BCM_AWL && DHD_AWL_RX */

	eh = (struct ether_header *)PKTDATA(dhdp->osh, pNBuff);
#if defined(BCM_WFD) && defined(BCM_WLAN_NIC_RX_RNR_ACCEL)
	if (DHD_WFD_IDX_VALID(dhdp->unit) && (inject_to_fastpath) &&
		(eh->ether_type != hton16(ETHER_TYPE_BRCM))) {
		struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);
		send_packet_to_upper_layer(skb);
		return BCME_OK;
	}
#endif

	d3_addr = (uint8_t *) eh->ether_dhost;

	if (ETHER_ISMULTI(d3_addr)) {	/* Only unicast packets are bridged via LUT */
		return BCME_UNSUPPORTED;
	}

	/* pktfwd is disabled */
	if (dhd_pktfwd_accel == false) {
		dhd_pktfwd->stats.rx_slow_fwds++;
		return BCME_UNSUPPORTED;
	}

	D3LUT_LOCK(dhd_pktfwd->d3lut);	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	d3lut_elem = d3lut_lkup(dhd_pktfwd->d3lut, d3_addr, D3LUT_LKUP_GLOBAL_POOL);

	if (likely(d3lut_elem != D3LUT_ELEM_NULL)) {
		if (!d3lut_elem->ext.wlan) {
			net_device = d3lut_elem->ext.net_device;
			d3lut_elem->ext.hit = 1;
		} else {
			d3fwd_wlif_t *d3fwd_wlif;
			d3fwd_wlif = d3lut_elem->ext.d3fwd_wlif;
			if (d3fwd_wlif != D3FWD_WLIF_NULL) {
				net_device = d3fwd_wlif->net_device;
				d3lut_elem->ext.hit = 1;
			} else
				net_device = (struct net_device *)NULL;
		}

		if (net_device != NULL) {
			struct net_device *rx_br_dev = NULL, *dst_br_dev = NULL;
			struct sk_buff *skb = PNBUFF_2_SKBUFF(pNBuff);
			bool rtnl_lock_taken = FALSE;

			/* if tx device is vlan on top of WLAN, pass it through the network
			 * stack to add vlan tag.
			 */
			if (d3lut_elem->ext.wlan &&
					is_netdev_vlan(d3lut_elem->ext.virt_net_device)) {
				d3lut_elem = D3LUT_ELEM_NULL;
			} else if (rtnl_trylock()) {
				rtnl_lock_taken = TRUE;
			}

			rx_br_dev = bcm_netdev_master_upper_dev_get_nolock(skb->dev);

			if (d3lut_elem != D3LUT_ELEM_NULL) {
				struct net_device *d3_lut_ndev = d3lut_elem->ext.virt_net_device;
				dst_br_dev = bcm_netdev_master_upper_dev_get_nolock(d3_lut_ndev);
			}

			if (rtnl_lock_taken) {
				rtnl_unlock();
			}

			/* Check if the packet is destined to different bridge interface
			 * Pass it trough the network stack in that case
			 */
			if ((rx_br_dev == NULL) || (dst_br_dev != rx_br_dev)) {
				d3lut_elem = D3LUT_ELEM_NULL;
				net_device = NULL;
			}
		}
	} else
		net_device = (struct net_device *)NULL;

	D3LUT_UNLK(dhd_pktfwd->d3lut);	// ---------------------------------------------------------

	if (net_device != (struct net_device *)NULL) {
		xmit_fn_t netdev_ops_xmit;
		netdev_ops_xmit =
		    (xmit_fn_t) (net_device->netdev_ops->ndo_start_xmit);
		netdev_ops_xmit(pNBuff, net_device);
		dhd_pktfwd->stats.rx_fast_fwds++;
		return BCME_OK;
	} else {
		dhd_pktfwd->stats.rx_slow_fwds++;
		return BCME_ERROR;
	}

} /* dhd_pktfwd_upstream() */

/**
 * -----------------------------------------------------------------------------
 * Function : Helper debug dump the system global header and statistics
 * -----------------------------------------------------------------------------
 */
static void _dhd_pktfwd_sys_dump(struct bcmstrbuf *b)
{
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;
	dhd_pktfwd_stats_t *dhd_pktfwd_stats = &dhd_pktfwd_g.stats;

	bcm_bprintf(b, PKTFWD_VRP_FMT
		" Dump, radios %u wlifs %u tx_accel %s dwds_accel(ap/sta) %s/%s init %s\n",
		PKTFWD_VRP_VAL(DHD_PKTFWD, DHD_PKTFWD_VERSIONCODE),
		dhd_pktfwd->radio_cnt, dhd_pktfwd->wlif_cnt,
		(dhd_pktfwd_accel == false) ? "NO" : "YES",
		(dhd_dwds_ap_pktfwd_accel == false) ? "NO" : "YES",
		(dhd_dwds_sta_pktfwd_accel == false) ? "NO" : "YES",
		(dhd_pktfwd->initialized == false) ? "NO" : "YES");

	/* Dump global system statistics */
	bcm_bprintf(b, "\t Pkt Tx[CHN %u MC %u] Rx[Fast %u,  Slow %u] "
		"sta %u drop %u fail %u\n",
		dhd_pktfwd_stats->txf_chn_pkts,
		dhd_pktfwd_stats->txf_mcast_pkts,
		dhd_pktfwd_stats->rx_fast_fwds,
		dhd_pktfwd_stats->rx_slow_fwds,
		dhd_pktfwd_stats->tot_stations,
		dhd_pktfwd_stats->pkts_dropped,
		dhd_pktfwd_stats->ops_failures);

} /* _dhd_pktfwd_sys_dump() */

/**
 * -----------------------------------------------------------------------------
 * Function : Dump all global subsystems. LOCKLESS
 * TODO: Need to find a way to dump logs into bcmstrbuf.
 * bcm_bprintf is available in bcmlib.ko
 * -----------------------------------------------------------------------------
 */

void dhd_pktfwd_sys_dump(dhd_pub_t * dhdp, struct bcmstrbuf *b)
{
	int dev;
	d3fwd_wlif_t *d3fwd_wlif;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	if (dhdp && (!DHD_PKTFWD_SUPPORTED(dhdp->unit))) {
		/* PKTFWD not enabled for this radio */
		return;
	}

	_dhd_pktfwd_sys_dump(b);

	/* XXX: pass strbuf to all */
	if (dhd_pktfwd->initialized == false)	/* global system not setup */
		return;

	/* Dump all active devices (INTERFACES registered with pktfwd) */
	d3fwd_wlif = dhd_pktfwd->d3fwd_wlif;	/* lockless walk of pool */
	for (dev = 0; dev < DHD_PKTFWD_DEVICES; ++dev, ++d3fwd_wlif) {
		if (d3fwd_wlif->unit != WLIF_UNIT_INVALID)
			d3fwd_wlif_dump(d3fwd_wlif);
	}

	/* Dump entire D3 Lookup Table (STATIONS registered with pktfwd) */
	d3lut_dump(dhd_pktfwd->d3lut);

	/* Dump all pktlist_context instances (PACKETS transferred via pktfwd) */
	pktlist_context_dump_all();

	/* Dump all pktqueue_context instances (PACKETS transferred via pktfwd) */
	pktqueue_context_dump_all();

} /* dhd_pktfwd_sys_dump() */

/**
 * -----------------------------------------------------------------------------
 * Function : Dump the WLAN Radio specific PKTFWD state
 * -----------------------------------------------------------------------------
 */

void dhd_pktfwd_radio_dump(dhd_pub_t * dhdp, struct bcmstrbuf *b)
{
#if 0
	int dev;
	bool dump_verbose;
	d3fwd_wlif_t *d3fwd_wlif;
	pktlist_context_t *pktlist_context;
	dhd_pktfwd_t *dhd_pktfwd = &dhd_pktfwd_g;

	dump_verbose = false;
#endif

	if (dhdp == NULL) {
		PKTFWD_ERROR("dhd %px invalid", dhdp);
		return;
	}

	if (!DHD_PKTFWD_SUPPORTED(dhdp->unit)) {
		/* PKTFWD not enabled for this radio */
		return;
	}

	_dhd_pktfwd_sys_dump(b);

#if defined(BCM_WFD)
	dhd_pktfwd_wfd_dbg(dhdp, b);
#endif

#if 0				/* Do not dump pktfwd stats until we found a way to pass strbuf */
	/* Dump all active devices (interfaces registered with pktfwd) for radio */
	d3fwd_wlif = dhd_pktfwd->d3fwd_wlif;
	/* Do not traverse the d3fwd_free dll as no lock is taken */
	for (dev = 0; dev < DHD_PKTFWD_DEVICES; ++dev, ++d3fwd_wlif) {
		if (d3fwd_wlif->unit == dhdp->unit) {
			dhd_pktfwd_wlif_dbg(d3fwd_wlif, b);
		}
	}

	pktlist_context = dhd_pktfwd->pktlist_context[dhdp->unit];
	if (pktlist_context == PKTLIST_CONTEXT_NULL) {
		PKTFWD_ERROR("radio unit %u pktlist_context invalid", dhdp->unit);
		return;
	}
#if (CC_PKTFWD_DEBUG >= 1)
	dump_verbose = true;
#endif
	pktlist_context_dump(pktlist_context, false, dump_verbose);
#endif /* 0 */

} /* dhd_pktfwd_radio_dbg() */
