/*
 * Common interface to the 802.11 Station Control Block (scb) structure
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wlc_scb.c 785169 2020-03-16 10:49:11Z $
 */

/**
 * @file
 * @brief
 * SCB is a per-station data structure that is stored in the wl driver. SCB container provides a
 * mechanism through which different wl driver modules can each allocate and maintain private space
 * in the scb used for their own purposes. The scb subsystem (wlc_scb.c) does not need to know
 * anything about the different modules that may have allocated space in scb. It can also be used
 * by per-port code immediately after wlc_attach() has been done (but before wlc_up()).
 *
 * - "container" refers to the entire space within scb that can be allocated opaquely to other
 *   modules.
 * - "cubby" refers to the per-module private space in the container.
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <proto/wpa.h>
#include <sbconfig.h>
#include <pcicfg.h>
#include <bcmsrom.h>
#include <wlioctl.h>
#include <epivers.h>
#ifdef BCMCCX
#include <bcmcrypto/ccx.h>
#endif /* BCMCCX */
#include <bcmwpa.h>

#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_keymgmt.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <wlc_antsel.h>
#include <wl_export.h>
#include <wlc_ap.h>
#include <wlc_scb_ratesel.h>
#include <wlc_assoc.h>
#ifdef PROP_TXSTATUS
#include <wlfc_proto.h>
#include <wl_wlfc.h>
#include <wlc_apps.h>
#ifdef WLAMPDU
#include <wlc_ampdu.h>
#endif /* WLAMPDU */
#ifdef WLAMSDU_TX
#include <wlc_amsdu.h>
#endif /* WLAMSDU_TX */
#include <wlc_nar.h>
#endif /* PROP_TXSTATUS */

#ifdef WL11N
#include <wlc_ampdu_cmn.h>
#endif /* WLAMPDU */

#ifdef TRAFFIC_MGMT
#include <wlc_traffic_mgmt.h>
#endif // endif
#include <wlc_pcb.h>
#include <wlc_txc.h>
#include <wlc_macfltr.h>
#ifdef WL_RELMCAST
#include "wlc_relmcast.h"
#endif // endif
#include <wlc_vht.h>
#ifdef WLTDLS
#include <wlc_tdls.h>
#endif /* WLTDLS */
#ifdef NEW_TXQ
#include <wlc_tx.h>
#endif // endif
#ifdef WLTAF
#include <wlc_taf.h>
#endif /* WLTAF */

#ifdef BCMPCIEDEV
#include <flring_fc.h>
#define SCB_BCMC_MIN_LFRAGS	12
#endif // endif

#define SCB_MAX_CUBBY		(pub->tunables->maxscbcubbies)
#define SCB_MAGIC 0x0505a5a5

#define INTERNAL_SCB		0x00000001
#define USER_SCB		0x00000002

#define	SCBHASHINDEX(hash, id)	((id[3] ^ id[4] ^ id[5]) % (hash))

#define SCBHANDLE_PS_STATE_MASK (1 << 8)
#define SCBHANDLE_INFORM_PKTPEND_MASK (1 << 9)

#ifdef SCBFREELIST
#ifdef INT_SCB_OPT
#error "SCBFREELIST incompatible with INT_SCB_OPT"
/* To make it compatible, freelist needs to track internal vs external */
#endif /* INT_SCB_OPT */
#endif /* SCBFREELIST */
/** structure for storing per-cubby client info */
typedef struct cubby_info {
	scb_cubby_init_t	fn_init;	/* fn called during scb malloc */
	scb_cubby_deinit_t	fn_deinit;	/* fn called during scb free */
	scb_cubby_dump_t 	fn_dump;	/* fn called during scb dump */
} cubby_info_t;

typedef struct cubby_info_ctx {
	void			*context;	/* context to be passed to all cb fns */
} cubby_info_ctx_t;

/** structure for storing public and private global scb module state */
struct scb_module {
	wlc_info_t	*wlc;			/* global wlc info handle */
	wlc_pub_t	*pub;			/* public part of wlc */
	uint16		nscb;			/* total number of allocated scbs */
	uint		scbtotsize;		/* total scb size including container */
	uint 		ncubby;			/* current num of cubbies */
	cubby_info_t	*cubby_info;		/* cubby client info */
	cubby_info_ctx_t	*cubby_info_ctx;		/* cubby client info */
#ifdef SCBFREELIST
	struct scb      *free_list;		/* Free list of SCBs */
#endif // endif
	int		cfgh;			/* scb bsscfg cubby handle */
	bcm_notif_h 	scb_state_notif_hdl;	/* scb state notifier handle. */
};

/** station control block - one per remote MAC address */
struct scb_info {
	struct scb 	*scbpub;	/* public portion of scb */
	struct scb_info *hashnext;	/* pointer to next scb under same hash entry */
	struct scb_info	*next;		/* pointer to next allocated scb */
	struct wlcband	*band;		/* pointer to our associated band */
#ifdef MACOSX
	struct scb_info *hashnext_copy;
	struct scb_info *next_copy;
#endif // endif
};

/* Helper macro for txpath in scb */
/* A feature in Tx path goes through following states:
 * Unregisterd -> Registered [Global state]
 * Registerd -> Configured -> Active -> Configured [Per-scb state]
 */

/* Set the next feature of given feature */
#define SCB_TXMOD_SET(scb, fid, _next_fid) { \
	scb->tx_path[fid].next_tx_fn = wlc->txmod_fns[_next_fid].tx_fn; \
	scb->tx_path[fid].next_handle = wlc->txmod_fns[_next_fid].ctx; \
	scb->tx_path[fid].next_fid = _next_fid; \
}
static void wlc_scb_hash_add(wlc_info_t *wlc, struct scb *scb, int bandunit,
	wlc_bsscfg_t *bsscfg);
static void wlc_scb_hash_del(wlc_info_t *wlc, struct scb *scbd, int bandunit,
	wlc_bsscfg_t *bsscfg);
static void wlc_scb_list_add(wlc_info_t *wlc, struct scb_info *scbinfo,
	wlc_bsscfg_t *bsscfg);
static void wlc_scb_list_del(wlc_info_t *wlc, struct scb *scbd,
	wlc_bsscfg_t *bsscfg);

static struct scb *wlc_scbvictim(wlc_info_t *wlc);
static struct scb *wlc_scb_getnext(struct scb *scb);
static struct wlc_bsscfg *wlc_scb_next_bss(scb_module_t *scbstate, int idx);
static int wlc_scbinit(wlc_info_t *wlc, struct wlcband *band, struct scb_info *scbinfo,
	uint32 scbflags);
static void wlc_scb_reset(scb_module_t *scbstate, struct scb_info *scbinfo);
static struct scb_info *wlc_scb_allocmem(scb_module_t *scbstate);
static void wlc_scb_freemem(scb_module_t *scbstate, struct scb_info *scbinfo);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_scb_dump(wlc_info_t *wlc, struct bcmstrbuf *b);
/** Dump the active txpath for the current SCB */
static int wlc_scb_txpath_dump(wlc_info_t *wlc, struct scb *scb, struct bcmstrbuf *b);
/** SCB Flags Names Initialization */
static const bcm_bit_desc_t scb_flags[] =
{
	{SCB_NONERP, "NonERP"},
	{SCB_LONGSLOT, "LgSlot"},
	{SCB_SHORTPREAMBLE, "ShPre"},
	{SCB_8021XHDR, "1X"},
	{SCB_WPA_SUP, "WPASup"},
	{SCB_DEAUTH, "DeA"},
	{SCB_WMECAP, "WME"},
	{SCB_BRCM, "BRCM"},
	{SCB_WDS_LINKUP, "WDSLinkUP"},
	{SCB_LEGACY_AES, "LegacyAES"},
	{SCB_MYAP, "MyAP"},
	{SCB_PENDING_PROBE, "PendingProbe"},
	{SCB_AMSDUCAP, "AMSDUCAP"},
	{SCB_USEME, "XXX"},
	{SCB_HTCAP, "HT"},
	{SCB_RECV_PM, "RECV_PM"},
	{SCB_AMPDUCAP, "AMPDUCAP"},
	{SCB_IS40, "40MHz"},
	{SCB_NONGF, "NONGFCAP"},
	{SCB_APSDCAP, "APSDCAP"},
	{SCB_PENDING_FREE, "PendingFree"},
	{SCB_PENDING_PSPOLL, "PendingPSPoll"},
	{SCB_RIFSCAP, "RIFSCAP"},
	{SCB_HT40INTOLERANT, "40INTOL"},
	{SCB_WMEPS, "WMEPSOK"},
	{SCB_COEX_MGMT, "OBSSCoex"},
	{SCB_IBSS_PEER, "IBSS Peer"},
	{SCB_STBCCAP, "STBC"},
#ifdef WLBTAMP
	{SCB_11ECAP, "11e"},
#endif // endif
	{0, NULL}
};
static const bcm_bit_desc_t scb_flags2[] =
{
	{SCB2_SGI20_CAP, "SGI20"},
	{SCB2_SGI40_CAP, "SGI40"},
	{SCB2_RX_LARGE_AGG, "LGAGG"},
#ifdef BCMWAPI_WAI
	{SCB2_WAIHDR, "WAI"},
#endif /* BCMWAPI_WAI */
	{SCB2_LDPCCAP, "LDPC"},
	{SCB2_VHTCAP, "VHT"},
	{SCB2_AMSDU_IN_AMPDU_CAP, "AGG^2"},
	{SCB2_P2P, "P2P"},
	{SCB2_DWDS_ACTIVE, "DWDS_ACTIVE"},
	{0, NULL}
};
static const bcm_bit_desc_t scb_flags3[] =
{
	{SCB3_A4_DATA, "A4_DATA"},
	{SCB3_A4_NULLDATA, "A4_NULLDATA"},
	{SCB3_A4_8021X, "A4_8021X"},
	{SCB3_DWDS_CAP, "DWDS_CAP"},
	{SCB3_1024QAM_CAP, "1024QAM_CAP"},
	{SCB3_MU_CAP, "MU_CAP"},
	{SCB3_MAP_CAP, "MAP_CAP"},
	{0, NULL}
};
static const bcm_bit_desc_t scb_states[] =
{
	{AUTHENTICATED, "AUTH"},
	{ASSOCIATED, "ASSOC"},
	{PENDING_AUTH, "AUTH_PEND"},
	{PENDING_ASSOC, "ASSOC_PEND"},
	{AUTHORIZED, "AUTH_8021X"},
	{TAKEN4IBSS, "IBSS"},
	{0, NULL}
};
#endif /* BCMDBG || BCMDBG_DUMP */

#ifdef SCBFREELIST
static void wlc_scbfreelist_free(scb_module_t *scbstate);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void wlc_scbfreelist_dump(scb_module_t *scbstate, struct bcmstrbuf *b);
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */
#endif /* SCBFREELIST */

#define SCBINFO(_scb) (_scb ? (struct scb_info *)((_scb)->scb_priv) : NULL)
#ifdef MACOSX

#define SCBSANITYCHECK(_scb)  { \
		if (((_scb) != NULL) &&				\
		    ((((_scb))->magic != SCB_MAGIC) ||	\
		     (SCBINFO(_scb)->hashnext != SCBINFO(_scb)->hashnext_copy) || \
		     (SCBINFO(_scb)->next != SCBINFO(_scb)->next_copy)))	\
			osl_panic("scbinfo corrupted: magic: 0x%x hn: %p hnc: %p n: %p nc: %p\n", \
			      ((_scb))->magic, SCBINFO(_scb)->hashnext, \
			      SCBINFO(_scb)->hashnext_copy,		\
			      SCBINFO(_scb)->next, SCBINFO(_scb)->next_copy);	\
	}

#define SCBFREESANITYCHECK(_scb)  { \
		if (((_scb) != NULL) &&				\
		    ((((_scb))->magic != ~SCB_MAGIC) || \
		     (SCBINFO(_scb)->next != SCBINFO(_scb)->next_copy)))	\
			osl_panic("scbinfo corrupted: magic: 0x%x hn: %p hnc: %p n: %p nc: %p\n", \
			      ((_scb))->magic, SCBINFO(_scb)->hashnext, \
			      SCBINFO(_scb)->hashnext_copy,		\
			      SCBINFO(_scb)->next, SCBINFO(_scb)->next_copy);	\
	}

#else

#define SCBSANITYCHECK(_scbinfo)	do {} while (0)
#define SCBFREESANITYCHECK(_scbinfo)	do {} while (0)

#endif /* MACOSX */

/** bsscfg cubby */
typedef struct scb_bsscfg_cubby {
	struct scb	**scbhash[MAXBANDS];	/* scb hash table */
	uint8		nscbhash;		/* scb hash table size */
	struct scb	*scb;			/* station control block link list */
} scb_bsscfg_cubby_t;

#define SCB_BSSCFG_CUBBY(ss, cfg) ((scb_bsscfg_cubby_t *)BSSCFG_CUBBY(cfg, (ss)->cfgh))

static int wlc_scb_bsscfg_init(void *context, wlc_bsscfg_t *cfg);
static void wlc_scb_bsscfg_deinit(void *context, wlc_bsscfg_t *cfg);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void wlc_scb_bsscfg_dump(void *context, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_scb_bsscfg_dump NULL
#endif // endif

#ifdef TXQ_MUX
void
wlc_scb_tx_next(uint fid, struct scb *scb, void *pkt, uint prec, const char *fn)
{

#ifdef BCMC_MUX_DEBUG
	WL_ERROR(("%s(): Caller %s()  fid=%u scb%s=0x%p pkt=0x%p\n",
		__FUNCTION__, fn, fid, SCB_INTERNAL(scb) ? " (Internal) " : "", scb, pkt));
#endif // endif

	scb->tx_path[fid].next_tx_fn(scb->tx_path[fid].next_handle, scb, pkt, prec);
}
#endif /* TXQ_MUX */

static int
wlc_scb_bsscfg_init(void *context, wlc_bsscfg_t *cfg)
{
	scb_module_t *scbstate = (scb_module_t *)context;
	scb_bsscfg_cubby_t *scb_cfg = SCB_BSSCFG_CUBBY(scbstate, cfg);
	uint8 nscbhash, *scbhash;
	wlc_pub_t *pub = scbstate->pub;
	uint32 i, len;

	nscbhash = ((pub->tunables->maxscb + 7)/8); /* # scb hash buckets */

	len = (sizeof(struct scb *) * MAXBANDS * nscbhash);
	scbhash = MALLOC(pub->osh, len);
	if (scbhash == NULL)
		return BCME_NOMEM;

	bzero((char *)scbhash, len);

	scb_cfg->nscbhash = nscbhash;
	for (i = 0; i < MAXBANDS; i++) {
		scb_cfg->scbhash[i] = (struct scb **)((uintptr)scbhash +
		                      (i * scb_cfg->nscbhash * sizeof(struct scb *)));
	}

	return BCME_OK;
}

static void
wlc_scb_bsscfg_deinit(void *context, wlc_bsscfg_t *cfg)
{
	scb_module_t *scbstate = (scb_module_t *)context;
	scb_bsscfg_cubby_t *scb_cfg = SCB_BSSCFG_CUBBY(scbstate, cfg);
	uint32 len;

	/* clear all scbs */
	wlc_scb_bsscfg_scbclear(cfg->wlc, cfg, TRUE);

	if (scb_cfg->scbhash[0] != NULL) {
		len = (sizeof(struct scb *) * MAXBANDS * scb_cfg->nscbhash);
		MFREE(scbstate->pub->osh, scb_cfg->scbhash[0], len);
	}
}

scb_module_t *
BCMATTACHFN(wlc_scb_attach)(wlc_info_t *wlc)
{
	scb_module_t *scbstate;
	int len;
	wlc_pub_t *pub = wlc->pub;

	len = sizeof(scb_module_t) + (sizeof(cubby_info_ctx_t) * SCB_MAX_CUBBY);
	if ((scbstate = MALLOC(pub->osh, len)) == NULL)
		return NULL;
	bzero((char *)scbstate, len);
	/* OBJECT REGISTRY: check if shared key has value already stored */
	scbstate->cubby_info = (cubby_info_t *) obj_registry_get(wlc->objr, OBJR_SCB_CUBBY);
	if (scbstate->cubby_info == NULL) {
		len = (sizeof(cubby_info_t) * SCB_MAX_CUBBY);
		if ((scbstate->cubby_info = MALLOC(pub->osh, len)) == NULL) {
			WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
				pub->unit, __FUNCTION__, MALLOCED(pub->osh)));
		}
		/* OBJECT REGISTRY: We are the first instance, store value for key */
		obj_registry_set(wlc->objr, OBJR_SCB_CUBBY, scbstate->cubby_info);
	}

	/* OBJECT REGISTRY: Reference the stored value in both instances */
	(void)obj_registry_ref(wlc->objr, OBJR_SCB_CUBBY);
	scbstate->cubby_info_ctx = (cubby_info_ctx_t *)
		((uintptr)scbstate + sizeof(scb_module_t));

	scbstate->wlc = wlc;
	scbstate->pub = pub;

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((scbstate->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(scb_bsscfg_cubby_t),
		wlc_scb_bsscfg_init, wlc_scb_bsscfg_deinit, wlc_scb_bsscfg_dump,
		(void *)scbstate)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve failed\n",
			wlc->pub->unit, __FUNCTION__));
		MFREE(pub->osh, scbstate, len);
		return NULL;
	}

	scbstate->scbtotsize = sizeof(struct scb);
	scbstate->scbtotsize += sizeof(int) * MA_WINDOW_SZ; /* sizeof rssi_window */
	scbstate->scbtotsize += sizeof(struct tx_path_node) * TXMOD_LAST;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(pub, "scb", (dump_fn_t)wlc_scb_dump, (void *)wlc);
#endif // endif

	/* create notification list for scb state change. */
	if (bcm_notif_create_list(wlc->notif, &scbstate->scb_state_notif_hdl) != BCME_OK) {
		WL_ERROR(("wl%d: %s: scb bcm_notif_create_list() failed\n",
			wlc->pub->unit, __FUNCTION__));
		MFREE(pub->osh, scbstate, len);
		return NULL;
	}

	return scbstate;
}

void
BCMATTACHFN(wlc_scb_detach)(scb_module_t *scbstate)
{
	wlc_pub_t *pub;
	int len;

	if (!scbstate)
		return;

	if (scbstate->scb_state_notif_hdl != NULL)
		bcm_notif_delete_list(&scbstate->scb_state_notif_hdl);

	pub = scbstate->pub;

#ifdef SCBFREELIST
	wlc_scbfreelist_free(scbstate);
#endif // endif

	ASSERT(scbstate->nscb == 0);
	if (scbstate && scbstate->cubby_info &&
	(obj_registry_unref(scbstate->wlc->objr, OBJR_SCB_CUBBY) == 0)) {
		obj_registry_set(scbstate->wlc->objr, OBJR_SCB_CUBBY, NULL);
		len = (sizeof(cubby_info_t) * SCB_MAX_CUBBY);
		MFREE(scbstate->pub->osh, scbstate->cubby_info, len);
		scbstate->cubby_info = NULL;
	}
	len = sizeof(scb_module_t) + (sizeof(cubby_info_ctx_t) * SCB_MAX_CUBBY);
	MFREE(scbstate->pub->osh, scbstate, len);
}

/* Methods for iterating along a list of scb */

/** Direct access to the next */
static struct scb *
wlc_scb_getnext(struct scb *scb)
{
	if (scb) {
		SCBSANITYCHECK(scb);
		return (SCBINFO(scb)->next ? SCBINFO(scb)->next->scbpub : NULL);
	}
	return NULL;
}
static struct wlc_bsscfg *
wlc_scb_next_bss(scb_module_t *scbstate, int idx)
{
	wlc_bsscfg_t	*next_bss = NULL;

	/* get next bss walking over hole */
	while (idx < WLC_MAXBSSCFG) {
		next_bss = WLC_BSSCFG(scbstate->wlc, idx);
		if (next_bss != NULL)
			break;
		idx++;
	}
	return next_bss;
}

/** Initialize an iterator keeping memory of the next scb as it moves along the list */
void
wlc_scb_iterinit(scb_module_t *scbstate, struct scb_iter *scbiter, wlc_bsscfg_t *bsscfg)
{
	scb_bsscfg_cubby_t *scb_cfg;
	ASSERT(scbiter != NULL);

	if (bsscfg == NULL) {
		/* walk scbs of all bss */
		scbiter->all = TRUE;
		scbiter->next_bss = wlc_scb_next_bss(scbstate, 0);
		if (scbiter->next_bss == NULL) {
			/* init next scb pointer also to null */
			scbiter->next = NULL;
			return;
		}
	} else {
		/* walk scbs of specified bss */
		scbiter->all = FALSE;
		scbiter->next_bss = bsscfg;
	}

	ASSERT(scbiter->next_bss != NULL);
	scb_cfg = SCB_BSSCFG_CUBBY(scbstate, scbiter->next_bss);
	SCBSANITYCHECK(scb_cfg->scb);

	/* Prefetch next scb, so caller can free an scb before going on to the next */
	scbiter->next = scb_cfg->scb;
}

/** move the iterator */
struct scb *
wlc_scb_iternext(scb_module_t *scbstate, struct scb_iter *scbiter)
{
	scb_bsscfg_cubby_t *scb_cfg;
	struct scb *scb;

	ASSERT(scbiter != NULL);

	while (scbiter->next_bss) {

		/* get the next scb in the current bsscfg */
		if ((scb = scbiter->next) != NULL) {
			/* get next scb of bss */
			SCBSANITYCHECK(scb);
			scbiter->next = (SCBINFO(scb)->next ? SCBINFO(scb)->next->scbpub : NULL);
			return scb;
		}

		/* get the next bsscfg if we have run out of scbs in the current bsscfg */
		if (scbiter->all) {
			scbiter->next_bss =
			        wlc_scb_next_bss(scbstate, WLC_BSSCFG_IDX(scbiter->next_bss) + 1);
			if (scbiter->next_bss != NULL) {
				scb_cfg = SCB_BSSCFG_CUBBY(scbstate, scbiter->next_bss);
				scbiter->next = scb_cfg->scb;
			}
		} else {
			scbiter->next_bss = NULL;
		}
	}

	/* done with all bsscfgs and scbs */
	scbiter->next = NULL;

	return NULL;
}

/**
 * Multiple modules have the need of reserving some private data storage related to a specific
 * communication partner. During ATTACH time, this function is called multiple times, typically one
 * time per module that requires this storage. This function does not allocate memory, but
 * calculates values to be used for a future memory allocation by wlc_scb_allocmem() instead.
 *
 * Return value: negative values are errors.
 */
int
BCMATTACHFN(wlc_scb_cubby_reserve)(wlc_info_t *wlc, uint size, scb_cubby_init_t fn_init,
	scb_cubby_deinit_t fn_deinit, scb_cubby_dump_t fn_dump, void *context)
{
	uint offset;
	scb_module_t *scbstate = wlc->scbstate;
	cubby_info_t *cubby_info;
	cubby_info_ctx_t *cubby_info_ctx;
	wlc_pub_t *pub = wlc->pub;

	ASSERT(scbstate->nscb == 0);
	ASSERT((scbstate->scbtotsize % PTRSZ) == 0);

	if (scbstate->ncubby >= (uint)SCB_MAX_CUBBY) {
		ASSERT(scbstate->ncubby < (uint)SCB_MAX_CUBBY);
		return BCME_NORESOURCE;
	}

	/* housekeeping info is stored in scb_module struct */
	cubby_info_ctx = &scbstate->cubby_info_ctx[scbstate->ncubby];
	cubby_info = &scbstate->cubby_info[scbstate->ncubby++];
	cubby_info->fn_init = fn_init;
	cubby_info->fn_deinit = fn_deinit;
	cubby_info->fn_dump = fn_dump;
	cubby_info_ctx->context = context;

	/* actual cubby data is stored at the end of scb's */
	offset = scbstate->scbtotsize;

	/* roundup to pointer boundary */
	scbstate->scbtotsize = ROUNDUP(scbstate->scbtotsize + size, PTRSZ);

	return offset;
}

struct wlcband *
wlc_scbband(struct scb *scb)
{
	return SCBINFO(scb)->band;
}

#ifdef SCBFREELIST
static struct scb_info *
wlc_scbget_free(scb_module_t *scbstate)
{
	struct scb_info *ret = NULL;
	if (scbstate->free_list == NULL)
		return NULL;
	ret = SCBINFO(scbstate->free_list);
	SCBFREESANITYCHECK(ret->scbpub);
	scbstate->free_list = (ret->next ? ret->next->scbpub : NULL);
#ifdef MACOSX
	ret->next_copy = NULL;
#endif // endif
	ret->next = NULL;
	wlc_scb_reset(scbstate, ret);
	return ret;
}

static void
wlc_scbadd_free(scb_module_t *scbstate, struct scb_info *ret)
{
	SCBFREESANITYCHECK(scbstate->free_list);
	ret->next = SCBINFO(scbstate->free_list);
	scbstate->free_list = ret->scbpub;
#ifdef MACOSX
	ret->scbpub->magic = ~SCB_MAGIC;
	ret->next_copy = ret->next;
#endif // endif
}

static void
wlc_scbfreelist_free(scb_module_t *scbstate)
{
	struct scb_info *ret = NULL;
	ret = SCBINFO(scbstate->free_list);
	while (ret) {
#ifdef MACOSX
		SCBFREESANITYCHECK(ret->scbpub);
#endif // endif
		scbstate->free_list = (ret->next ? ret->next->scbpub : NULL);
		wlc_scb_freemem(scbstate, ret);
		ret = scbstate->free_list ? SCBINFO(scbstate->free_list) : NULL;
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static
void wlc_scbfreelist_dump(scb_module_t *scbstate, struct bcmstrbuf *b)
{
	struct scb_info *entry = NULL;
	int i = 1;

	bcm_bprintf(b, "scbfreelist:\n");
	entry = SCBINFO(scbstate->free_list);
	while (entry) {
#ifdef MACOSX
		SCBFREESANITYCHECK(entry->scbpub);
#endif // endif
		bcm_bprintf(b, "%d: 0x%x\n", i, entry);
		entry = entry->next ? SCBINFO(entry->next->scbpub) : NULL;
		i++;
	}
}
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */
#endif /* SCBFREELIST */

void
wlc_internalscb_free(wlc_info_t *wlc, struct scb *scb)
{
	scb->permanent = FALSE;
	wlc_scbfree(wlc, scb);
}

static void
wlc_scb_reset(scb_module_t *scbstate, struct scb_info *scbinfo)
{
	struct scb *scbpub = scbinfo->scbpub;

	bzero((char*)scbinfo, sizeof(struct scb_info));
	scbinfo->scbpub = scbpub;
	bzero(scbpub, scbstate->scbtotsize);
	scbpub->scb_priv = (void *) scbinfo;
	/* init substructure pointers */
	scbpub->rssi_window = (int *)((char *)scbpub + sizeof(struct scb));
	scbpub->tx_path = (struct tx_path_node *)
	                ((char *)scbpub->rssi_window + (sizeof(int)*MA_WINDOW_SZ));
}

/**
 * After all the modules indicated how much cubby space they need in the scb, the actual scb can be
 * allocated. This happens one time fairly late within the attach phase, but also when e.g.
 * communication with a new remote party is started.
 */
static struct scb_info *
wlc_scb_allocmem(scb_module_t *scbstate)
{
	struct scb_info *scbinfo = NULL;
	struct scb *scbpub;

	scbinfo = MALLOCZ(scbstate->pub->osh, sizeof(struct scb_info));
	if (!scbinfo) {
		WL_ERROR(("wl%d: %s: Internalscb alloc failure for scb_info %d\n",
			scbstate->pub->unit, __FUNCTION__, (int)sizeof(struct scb_info)));
		return NULL;
	}
	scbpub = MALLOCZ(scbstate->pub->osh, scbstate->scbtotsize);
	scbinfo->scbpub = scbpub;
	if (!scbpub) {
		/* set field to null so freeing mem does */
		/* not cause exception by freeing bad ptr */
		scbinfo->scbpub = NULL;
		wlc_scb_freemem(scbstate, scbinfo);
		WL_ERROR(("wl%d: %s: Internalscb alloc failure for scbtotsize %d\n",
			scbstate->pub->unit, __FUNCTION__, (int)scbstate->scbtotsize));
		return NULL;
	}

	wlc_scb_reset(scbstate, scbinfo);

	return scbinfo;
}

struct scb *
wlc_internalscb_alloc(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	const struct ether_addr *ea, struct wlcband *band)
{
	struct scb_info *scbinfo = NULL;
	scb_module_t *scbstate = wlc->scbstate;
	int bcmerror = 0;
	struct scb *scb;

#ifdef SCBFREELIST
	/* If not found on freelist then allocate a new one */
	if ((scbinfo = wlc_scbget_free(scbstate)) == NULL)
#endif // endif
	{
		scbinfo = wlc_scb_allocmem(scbstate);
		if (!scbinfo) {
			WL_ERROR(("wl%d: %s wlc_scb_allocmem failed\n",
				wlc->pub->unit, __FUNCTION__));
			return NULL;
		}
	}

	scb = scbinfo->scbpub;
	scb->bsscfg = cfg;
	scb->ea = *ea;

	bcmerror = wlc_scbinit(wlc, band, scbinfo, INTERNAL_SCB);
	if (bcmerror) {
		WL_ERROR(("wl%d: %s failed with err %d\n",
			wlc->pub->unit, __FUNCTION__, bcmerror));
		wlc_internalscb_free(wlc, scb);
		return NULL;
	}
	scb->permanent = TRUE;

	/* force wlc_scb_set_bsscfg() */
	scb->bsscfg = NULL;
	wlc_scb_set_bsscfg(scb, cfg);

#ifdef TXQ_MUX
	WL_ERROR(("%s: --------------------->allocated internal SCB:%p\n", __FUNCTION__, scb));
#endif // endif

	return scb;
}

static struct scb *
wlc_userscb_alloc(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	const struct ether_addr *ea, struct wlcband *band)
{
	scb_module_t *scbstate = wlc->scbstate;
	struct scb_info *scbinfo = NULL;
	struct scb *oldscb;
	int bcmerror;
	struct scb *scb;

	if ((scbstate->nscb <= wlc->pub->tunables->maxscb) &&
#ifdef DONGLEBUILD
		/* Make sure free_mem never gets below minimum threshold due to scb_allocs */
		(OSL_MEM_AVAIL() > wlc->pub->tunables->min_scballoc_mem) &&
#endif // endif
		1) {
#ifdef SCBFREELIST
		/* If not found on freelist then allocate a new one */
		if ((scbinfo = wlc_scbget_free(scbstate)) == NULL)
#endif // endif
		{
			scbinfo = wlc_scb_allocmem(scbstate);
			if (!scbinfo) {
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
					wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			}
		}
	}
	if (!scbinfo) {
		/* free the oldest entry */
		if (!(oldscb = wlc_scbvictim(wlc))) {
			WL_ERROR(("wl%d: %s: no SCBs available to reclaim\n",
			          wlc->pub->unit, __FUNCTION__));
			return NULL;
		}
		if (!wlc_scbfree(wlc, oldscb)) {
			WL_ERROR(("wl%d: %s: Couldn't free a victimized scb\n",
			          wlc->pub->unit, __FUNCTION__));
			return NULL;
		}
		ASSERT(scbstate->nscb <= wlc->pub->tunables->maxscb);
#ifdef SCBFREELIST
		/* If not found on freelist then allocate a new one */
		if ((scbinfo = wlc_scbget_free(scbstate)) == NULL)
#endif // endif
		{
			/* allocate memory for scb */
			if (!(scbinfo = wlc_scb_allocmem(scbstate))) {
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
				return NULL;
			}
		}
	}

	scbstate->nscb++;

	scb = scbinfo->scbpub;
	scb->bsscfg = cfg;
	scb->ea = *ea;

	bcmerror = wlc_scbinit(wlc, band, scbinfo, USER_SCB);
	if (bcmerror) {
		WL_ERROR(("wl%d: %s failed with err %d\n", wlc->pub->unit, __FUNCTION__, bcmerror));
		wlc_scbfree(wlc, scb);
		return NULL;
	}

	/* add it to the link list */
	wlc_scb_list_add(wlc, scbinfo, cfg);

	/* install it in the cache */
	wlc_scb_hash_add(wlc, scb, band->bandunit, cfg);

	/* force wlc_scb_set_bsscfg() */
	scb->bsscfg = NULL;
	wlc_scb_set_bsscfg(scb, cfg);

#ifdef TXQ_MUX
	WL_ERROR(("%s(): --------------------->allocated user SCB:%p\n", __FUNCTION__, scb));
#endif // endif

	return scb;
}

static int
wlc_scbinit(wlc_info_t *wlc, struct wlcband *band, struct scb_info *scbinfo, uint32 scbflags)
{
	struct scb *scb = NULL;
	scb_module_t *scbstate = wlc->scbstate;
	cubby_info_t *cubby_info;
	cubby_info_ctx_t *cubby_info_ctx;
	uint i;
	int bcmerror = 0;

	scb = scbinfo->scbpub;
	ASSERT(scb != NULL);

	scb->used = wlc->pub->now;
	scb->bandunit = band->bandunit;
	scbinfo->band = band;

	for (i = 0; i < NUMPRIO; i++)
		scb->seqctl[i] = 0xFFFF;
	scb->seqctl_nonqos = 0xFFFF;

#if defined(BCMPCIEDEV)
	if (BCMPCIEDEV_ENAB()) {
		for (i = 0; i < FLOWRING_PER_SCB_MAX; i++) {
			RAVG_INIT(TXPKTLEN_RAVG(scb, i), TXPKTLEN_RBUF(scb, i),
				(ETHER_MAX_DATA/2), RAVG_EXP_PKT);
			RAVG_INIT(WEIGHT_RAVG(scb, i), WEIGHT_RBUF(scb, i),
				0, RAVG_EXP_WGT);
		}
#ifdef ATL_PERC
		scb->flr_staperc = 0;
#endif // endif
	}
#endif /* BCMPCIEDEV */

#ifdef MACOSX
	scb->magic = SCB_MAGIC;
#endif // endif

	/* no other inits are needed for internal scb */
	if (scbflags & INTERNAL_SCB) {
		scb->flags2 |= SCB2_INTERNAL;
#ifdef INT_SCB_OPT
		return BCME_OK;
#endif // endif
	}

	for (i = 0; i < scbstate->ncubby; i++) {
		cubby_info = &scbstate->cubby_info[i];
		cubby_info_ctx = &scbstate->cubby_info_ctx[i];
		if (cubby_info->fn_init) {
			bcmerror = cubby_info->fn_init(cubby_info_ctx->context, scb);
			if (bcmerror) {
				WL_ERROR(("wl%d: %s: Cubby failed\n",
				          wlc->pub->unit, __FUNCTION__));
				return bcmerror;
			}
		}
	}

#if defined(AP)
	wlc_scb_rssi_init(scb, WLC_RSSI_INVALID);
#endif // endif
#ifdef PSPRETEND
	scb->ps_pretend = PS_PRETEND_NOT_ACTIVE;
	scb->ps_pretend_failed_ack_count = 0;
#endif // endif
#ifdef WLCNTSCB
	bzero((char*)&scb->scb_stats, sizeof(scb->scb_stats));
#endif // endif
	return bcmerror;
}

static void
wlc_scb_freemem(scb_module_t *scbstate, struct scb_info *scbinfo)
{

	if (scbinfo->scbpub)
		MFREE(scbstate->pub->osh, scbinfo->scbpub, scbstate->scbtotsize);
	MFREE(scbstate->pub->osh, scbinfo, sizeof(struct scb_info));
}

#ifdef PROP_TXSTATUS
struct scb * wlc_scbfind_from_wlcif(wlc_info_t *wlc, struct wlc_if *wlcif, uint8 *addr)
{
	struct scb *scb = NULL;
	wlc_bsscfg_t *bsscfg;

	if (wlcif && (wlcif->type == WLC_IFTYPE_WDS)) {
		return (wlcif->u.scb);
	}

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);

	if (!bsscfg)
		return NULL;

	if (BSSCFG_STA(bsscfg) && bsscfg->BSS) {
#ifdef WLTDLS
		if ((TDLS_ENAB(wlc->pub)) && !(ETHER_ISMULTI(addr)) && !ETHER_ISNULLADDR(addr))
			scb = wlc_tdls_scbfind_all_ex(wlc, (struct ether_addr *)addr);
#endif // endif
		if (scb == NULL) {
		if (!ETHER_ISNULLADDR(&bsscfg->BSSID))
			scb = wlc_scbfind(wlc, bsscfg, &bsscfg->BSSID);
		else
			scb = wlc_scbfind(wlc, bsscfg, &bsscfg->prev_BSSID);
		}
	}
	else if (!ETHER_ISMULTI(addr)) {
#ifdef WLAWDL
		scb = wlc_scbfind_dualband(wlc, bsscfg, (struct ether_addr *)addr);
#else
		scb = wlc_scbfind(wlc, bsscfg, (struct ether_addr *)addr);
#endif // endif
	} else
		scb = bsscfg->bcmc_scb[wlc->band->bandunit];

	return scb;
}

void
wlc_scb_update_available_traffic_info(wlc_info_t *wlc, uint8 mac_handle, uint8 ta_bmp)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (scb->mac_address_handle &&
			(scb->mac_address_handle == mac_handle)) {
			SCB_PROPTXTSTATUS_SETTIM(scb, ta_bmp);
			if (AP_ENAB(wlc->pub))
				wlc_apps_pvb_update_from_host(wlc, scb, TRUE);
			break;
		}
	}
}

bool
wlc_flow_ring_scb_update_available_traffic_info(wlc_info_t *wlc, uint8 mac_handle,
	uint8 tid, bool op)
{
	struct scb *scb;
	struct scb_iter scbiter;
	uint8 ta_bmp;
	bool  ret = TRUE;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (scb->mac_address_handle &&
			(scb->mac_address_handle == mac_handle)) {
			ta_bmp = SCB_PROPTXTSTATUS_TIM(scb);
			ta_bmp = (ta_bmp & ~(0x1 << tid));
			ta_bmp = (ta_bmp | (op << tid));
			SCB_PROPTXTSTATUS_SETTIM(scb, ta_bmp);
			if (BSSCFG_AP(scb->bsscfg) || BSSCFG_IS_TDLS(scb->bsscfg)) {
				ret = wlc_apps_pvb_update_from_host(wlc, scb, op);
				if (!ret) {
					ta_bmp = (ta_bmp & ~(0x1 << tid));
					SCB_PROPTXTSTATUS_SETTIM(scb, ta_bmp);
				}
			}
			break;
		}
	}
	return ret;
}
uint16
wlc_flow_ring_get_scb_handle(wlc_info_t *wlc, struct wlc_if *wlcif, uint8 *da)
{
	struct scb *scb;
	uint16	ret = 0xff;

	scb = wlc_scbfind_from_wlcif(wlc, wlcif, da);

	if (!scb || !scb->bsscfg)
		return ret;

	if (BSSCFG_AP(scb->bsscfg) || BSSCFG_AWDL(wlc, scb->bsscfg) ||
		BSSCFG_IS_TDLS(scb->bsscfg)) {
		ret = scb->mac_address_handle;
		if (BSSCFG_AP(scb->bsscfg) || BSSCFG_IS_TDLS(scb->bsscfg)) {
			ret |= SCBHANDLE_INFORM_PKTPEND_MASK;
			if (!SCB_ISMULTI(scb) && SCB_PS(scb))
				ret |= SCBHANDLE_PS_STATE_MASK;
		}
	}
	return ret;
}

void wlc_flush_flowring_pkts(wlc_info_t *wlc, struct wlc_if *wlcif, uint8 *addr,
	uint16 flowid, uint8 tid_ac)
{
	struct scb *scb;

	scb = wlc_scbfind_from_wlcif(wlc, wlcif, addr);

#ifdef WLNAR
	if (scb && wlc->nar_handle) {
		wlc_nar_flush_flowid_pkts(wlc->nar_handle, scb, flowid);
	}
#endif // endif
#ifdef WLAMSDU_TX
	if (scb && SCB_AMSDU(scb) && AMSDU_TX_ENAB(wlc->pub) && wlc->ami) {
		wlc_amsdu_flush_flowid_pkts(wlc->ami, scb, flowid);
	}
#endif /* WLAMSDU_TX */
#ifdef WLAMPDU
	if (scb && SCB_AMPDU(scb)) {
		wlc_ampdu_flush_flowid_pkts(wlc, scb, flowid);
	}
#endif // endif

	/* Flush txq packets for given flowid */
	wlc_txq_flush_flowid_pkts(wlc, flowid);
}

#if defined(BCMPCIEDEV)
uint32
wlc_flow_ring_reset_weight(wlc_info_t *wlc, struct wlc_if *wlcif,
	uint8 *da, uint8 fl)
{
	struct scb *scb;
	ratespec_t rspec = 0;
	uint32 rate = 0;
	uint32 phyrate;
	uint32 pktlen_avg = 0;
	uint32 weight_avg = 0;
	flowring_op_param_t op;

	if (BCMPCIEDEV_ENAB()) {

	ASSERT(fl < FLOWRING_PER_SCB_MAX);

	scb = wlc_scbfind_from_wlcif(wlc, wlcif, da);
	if (scb != NULL && scb->bsscfg != NULL) {
		/* Reseting moving average packet length to default */
		RAVG_INIT(TXPKTLEN_RAVG(scb, fl), TXPKTLEN_RBUF(scb, fl),
				(ETHER_MAX_DATA/2), RAVG_EXP_PKT);
		pktlen_avg = RAVG_AVG(TXPKTLEN_RAVG(scb, fl), RAVG_EXP_PKT);

		rspec = wlc_ravg_get_scb_cur_rspec(wlc, scb);

		/* Reseting moving average weight to default */
		if (rspec > 0) {
			uint32 weight = wlc_scb_calc_weight(pktlen_avg,
				RSPEC2RATE(rspec), RSPEC_ISLEGACY(rspec));

			RAVG_INIT(WEIGHT_RAVG(scb, fl), WEIGHT_RBUF(scb, fl),
				weight, RAVG_EXP_WGT);
			weight_avg = RAVG_AVG(WEIGHT_RAVG(scb, fl), RAVG_EXP_WGT);

			rate = RSPEC2RATE(rspec);
			if (RSPEC_ISLEGACY(rspec)) {
				phyrate = rate << 6;
			}
			else {
				phyrate = rate >> 3;
			}

			/* rate is in kByte. calculate No. of pkts (len=ETHER_MAX_DATA)
			 * in 50ms period (SCB_FL_TXPKTS_RATIO=20), to estimate no. of
			 * lfrag needed for flow ring
			 */
			WEIGHT_PHYRATE(scb) = phyrate;
			op.lfrag_max = (phyrate * 1000) / (ETHER_MAX_DATA * 2);
			op.lfrag_max /= SCB_FL_TXPKTS_RATIO;

			/* For BCMC flowring, adjust lfrag_max to a bare minimum.
			 * On 2.4G, the default phyrate will be limited to 1Mbps,
			 * translating to just 1 lfrag. This affects BCMC flows.
			 */
			if (SCB_ISMULTI(scb)) {
				if (op.lfrag_max < SCB_BCMC_MIN_LFRAGS)
					op.lfrag_max = SCB_BCMC_MIN_LFRAGS;
			}

			op.phyrate = phyrate;
			op.weight = weight_avg;
			op.mumimo = SCB_MU(scb);
			op.dwds = SCB_DWDS(scb);
#ifdef WLATF_PERC
			op.atm_perc = MIN_RESET_ATF_PERC;
#endif // endif
			wlfc_upd_flr_weight(wlc->wl, scb->mac_address_handle, fl, (void*)&op);
		}
	}
	}
	return weight_avg;
}

/* Updating weight of all flowrings of given scb to the pciedev bus layer.
 * Called from WLC module watchdog.
 */
BCMFASTPATH void
wlc_scb_upd_all_flr_weight(wlc_info_t *wlc, struct scb *scb)
{
	if (BCMPCIEDEV_ENAB()) {
		uint32 avg_weight = 0;
		uint8 fl;
		ratespec_t rspec = 0;
		uint32 rate;
		uint32 phyrate;

		flowring_op_param_t op;

		for (fl = 0; fl < FLOWRING_PER_SCB_MAX; fl++) {
			avg_weight = RAVG_AVG(WEIGHT_RAVG(scb, fl), RAVG_EXP_WGT);
			if (avg_weight > 0) {

				rspec = wlc_ravg_get_scb_cur_rspec(wlc, scb);
				rate = RSPEC2RATE(rspec);

				if (RSPEC_ISLEGACY(rspec)) {
					phyrate = rate << 6;
				}
				else {
					phyrate = rate >> 3;
				}

				/* rate is in kByte. calculate No. of pkts
				 * in 50ms period (SCB_FL_TXPKTS_RATIO=20), to estimate no. of
				 * lfrag needed for flow ring
				 */
				WEIGHT_PHYRATE(scb) = (WEIGHT_PHYRATE(scb) + phyrate)/2;

				op.lfrag_max = (WEIGHT_PHYRATE(scb) * 1000) / (ETHER_MAX_DATA * 2);
				op.lfrag_max /= SCB_FL_TXPKTS_RATIO;

				op.phyrate = phyrate;
				op.weight = avg_weight;
				op.mumimo = SCB_MU(scb);
				op.dwds = SCB_DWDS(scb);

				wlfc_upd_flr_weight(wlc->wl, scb->mac_address_handle,
					fl, (void*)&op);
			}

			/* reset txpkts counter */
			WEIGHT_TXPKT(scb, fl) = 0;
		}
	}
}

#ifdef WLATF_PERC
BCMFASTPATH void
wlc_scb_upd_all_flr_perc(wlc_info_t *wlc, struct scb *scb, uint32 perc)
{
	if (BCMPCIEDEV_ENAB()) {
		uint32 avg_weight = 0;
		uint8 fl;
		flowring_op_param_t op;
		ratespec_t rspec = 0;
		uint32 rate;
		uint32 phyrate;
		for (fl = 0; fl < FLOWRING_PER_SCB_MAX; fl++) {
			avg_weight = RAVG_AVG(WEIGHT_RAVG(scb, fl), RAVG_EXP_WGT);
			if (avg_weight > 0) {
				op.weight = avg_weight;
				op.atm_perc = perc;
				rspec = wlc_ravg_get_scb_cur_rspec(wlc, scb);
				rate = RSPEC2RATE(rspec);

				if (RSPEC_ISLEGACY(rspec)) {
					phyrate = rate << 6;
				} else {
					phyrate = rate >> 3;
				}

				/* rate is in kByte. calculate No. of pkts
				 * in 50ms period (SCB_FL_TXPKTS_RATIO=20), to estimate no. of
				 * lfrag needed for flow ring
				 */
				WEIGHT_PHYRATE(scb) = (WEIGHT_PHYRATE(scb) + phyrate)/2;

				op.lfrag_max = (WEIGHT_PHYRATE(scb) * 1000) / (ETHER_MAX_DATA * 2);
				op.lfrag_max /= SCB_FL_TXPKTS_RATIO;

				op.phyrate = phyrate;
				op.mumimo = SCB_MU(scb);
				wlfc_upd_flr_weight(wlc->wl, scb->mac_address_handle, fl,
						(void*)&op);
			}
			/* reset txpkts counter */
			WEIGHT_TXPKT(scb, fl) = 0;
		}
	}
}
#endif /* WLATF_PERC */

#endif /* BCMPCIEDEV */
#endif /* PROP_TXSTATUS */

#ifdef WLTAF
#define WLTAF_TS2_EBOS_FACTOR           5
#define WLTAF_TS2_ATOS_FACTOR           0
#define WLTAF_TS2_ATOS2_FACTOR          4
/*
 * TS2 shaper will allow EBOS/ATOS to release more data than ATOS2
 * ATOS2 configuration causes more traffic to be released from other traffic stream.
 * EBOS configuration forces more traffic to released from EBOS traffic stream.
 */
BCMFASTPATH uint32
wlc_ts2_traffic_shaper(struct scb *scb, uint32 weight)
{
	/* increase EBOS stream priority */
	if (SCB_TS_EBOS(scb))
		weight = (weight >> WLTAF_TS2_EBOS_FACTOR);
	/* default - no explicit shaping */
	if (SCB_TS_ATOS(scb))
		weight = (weight >> WLTAF_TS2_ATOS_FACTOR);
	/* lower ATOS2 stream priority */
	if (SCB_TS_ATOS2(scb))
		weight = (weight << WLTAF_TS2_ATOS2_FACTOR);

	if (!weight) {
		/* assign a minimum weight for ebos/atos streams */
		weight = 1;
	}

	return weight;
}
#endif /* WLTAF */

#if defined(BCMPCIEDEV)
/* Calculating the weight based on average packet length.
 * Adding weight into the moving average buffer.
 */
BCMFASTPATH void
wlc_ravg_add_weight(wlc_info_t *wlc, struct scb *scb, int fl,
	ratespec_t rspec)
{
	uint32 weight = 0;
	uint32 avg_pktlen = 0;

	if (BCMPCIEDEV_ENAB()) {
	ASSERT(fl < FLOWRING_PER_SCB_MAX);

	/* calculating the average packet length  */
	avg_pktlen = RAVG_AVG(TXPKTLEN_RAVG(scb, fl), RAVG_EXP_PKT);

	/* calculating the weight based on avg packet length and rate spec */
	weight = wlc_scb_calc_weight(avg_pktlen, RSPEC2RATE(rspec),
		RSPEC_ISLEGACY(rspec));
#ifdef WLTAF
	if (WLTAF_ENAB(wlc->pub)) {
		/* apply ts2 traffic shaper to weight */
		weight = wlc_ts2_traffic_shaper(scb, weight);
	}
#endif // endif
	/* adding weight into the moving average buffer */
	RAVG_ADD(WEIGHT_RAVG(scb, fl), WEIGHT_RBUF(scb, fl),
		weight, RAVG_EXP_WGT);
	}
}

BCMFASTPATH ratespec_t
wlc_ravg_get_scb_cur_rspec(wlc_info_t *wlc, struct scb *scb)
{
	ratespec_t cur_rspec = 0;

	if (BCMPCIEDEV_ENAB()) {

	if (SCB_ISMULTI(scb) || SCB_INTERNAL(scb)) {
		if (RSPEC_ACTIVE(wlc->band->mrspec_override))
			cur_rspec = wlc->band->mrspec_override;
		else
			cur_rspec = scb->rateset.rates[0];
	} else {
		cur_rspec = wlc_scb_ratesel_get_primary(wlc, scb, NULL);
	}
	}
	return cur_rspec;
}
#endif /* BCMPCIEDEV */

uint32 BCMFASTPATH
wlc_scb_dot11hdrsize(struct scb *scb)
{
	wlc_bsscfg_t *bsscfg = SCB_BSSCFG(scb);
	wlc_info_t *wlc = bsscfg->wlc;
	wlc_key_info_t key_info;
	uint32 len;

	len = DOT11_MAC_HDR_LEN + DOT11_FCS_LEN;

	if (SCB_QOS(scb))
		len += DOT11_QOS_LEN;

	if (SCB_A4_DATA(scb))
		len += ETHER_ADDR_LEN;

	wlc_keymgmt_get_scb_key(wlc->keymgmt, scb, WLC_KEY_ID_PAIRWISE,
			WLC_KEY_FLAG_NONE, &key_info);

	if (key_info.algo != CRYPTO_ALGO_OFF) {
		len += key_info.iv_len;
		len += key_info.icv_len;
		if (key_info.algo == CRYPTO_ALGO_TKIP)
			len += TKIP_MIC_SIZE;
	}

	return len;
}

#ifdef WL11AC
static uint8
wlc_scb_get_bw_from_scb_oper_mode(wlc_vht_info_t *vhti, struct scb *scb)
{
	uint8 bw = 0, bw160_8080 = 0;
	uint8 mode = 0;
	mode = wlc_vht_get_scb_opermode(vhti, scb);
	bw160_8080 = DOT11_OPER_MODE_160_8080(mode);
	mode &= DOT11_OPER_MODE_CHANNEL_WIDTH_MASK;
	if (mode == DOT11_OPER_MODE_20MHZ)
		bw = BW_20MHZ;
	else if (mode == DOT11_OPER_MODE_40MHZ)
		bw = BW_40MHZ;
	else if (mode == DOT11_OPER_MODE_80MHZ && !bw160_8080)
		bw = BW_80MHZ;
	else if (mode == DOT11_OPER_MODE_80MHZ && bw160_8080)
		bw = BW_160MHZ;

	return bw;
}
#endif /* WL11AC */

uint8
wlc_scb_link_bw_update(wlc_info_t *wlc, struct scb *scb)
{
	uint8 bw = BW_20MHZ;
#ifdef WL11AC
	uint8 scb_oper_mode_bw = BW_20MHZ;
#endif // endif
	chanspec_t chanspec;

	/* Use configured chanspec rather than the instantaneous phy chanspec for WDS links */
	if (SCB_LEGACY_WDS(scb)) {
		chanspec = wlc->home_chanspec;
	} else {
		chanspec = wlc->chanspec;
	}

#ifdef WL11AC
	if (SCB_VHT_CAP(scb) &&
		CHSPEC_IS8080(chanspec) &&
		((scb->flags3 & SCB3_IS_80_80)))
		bw = BW_160MHZ;
	else if (SCB_VHT_CAP(scb) &&
		CHSPEC_IS160(chanspec) &&
		((scb->flags3 & SCB3_IS_160)))
		bw = BW_160MHZ;
	else if (CHSPEC_BW_GE(chanspec, WL_CHANSPEC_BW_80) &&
		SCB_VHT_CAP(scb))
		bw = BW_80MHZ;
	else
#endif /* WL11AC */
	if (((scb->flags & SCB_IS40)) &&
	    CHSPEC_BW_GE(chanspec, WL_CHANSPEC_BW_40))
		bw = BW_40MHZ;

	/* here bw derived from chanspec and capabilities */
#ifdef WL11AC
	/* process operating mode notification for channel bw */

	if ((SCB_HT_CAP(scb) || SCB_VHT_CAP(scb)) &&
		wlc_vht_get_scb_opermode_enab(wlc->vhti, scb) &&
		!DOT11_OPER_MODE_RXNSS_TYPE(wlc_vht_get_scb_opermode(wlc->vhti, scb))) {
		scb_oper_mode_bw = wlc_scb_get_bw_from_scb_oper_mode(wlc->vhti, scb);
		bw = (scb_oper_mode_bw < bw)?scb_oper_mode_bw:bw;
	}

#endif /* WL11AC */

	scb->link_bw = bw;
	return (bw);
}

bool
wlc_scbfree(wlc_info_t *wlc, struct scb *scbd)
{
	struct scb_info *remove = SCBINFO(scbd);
	scb_module_t *scbstate = wlc->scbstate;
	cubby_info_t *cubby_info;
	cubby_info_ctx_t *cubby_info_ctx;
	uint i;
	uint8 prio;
	struct scb_iter scbiter;
	struct scb *scb;

	if (scbd->permanent)
		return FALSE;

	/* Return if SCB is already being deleted else mark it */
	if (scbd->flags & SCB_PENDING_FREE)
		return FALSE;

	scbd->flags |= SCB_PENDING_FREE;

#ifdef INT_SCB_OPT
	/* no other cleanups are needed for internal scb */
	if (SCB_INTERNAL(scbd)) {
		goto free;
	}
#endif // endif

#if defined(WL_MULTIQUEUE) && defined(NEW_TXQ)
	wlc_tx_fifo_scb_flush(wlc, scbd);
#endif /* WL_MULTIQUEUE && NEW_TXQ */

	for (i = 0; i < scbstate->ncubby; i++) {
		uint j = scbstate->ncubby - 1 - i;
		cubby_info = &scbstate->cubby_info[j];
		cubby_info_ctx = &scbstate->cubby_info_ctx[j];
		if (cubby_info->fn_deinit)
			cubby_info->fn_deinit(cubby_info_ctx->context, scbd);
	}

#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		/* release MAC handle back to the pool, if applicable */
		if (scbd->mac_address_handle) {
			wlfc_MAC_table_update(wlc->wl, &scbd->ea.octet[0],
				WLFC_CTL_TYPE_MACDESC_DEL,
				scbd->mac_address_handle, ((scbd->bsscfg->wlcif == NULL) ?
				0 : scbd->bsscfg->wlcif->index));
			wlfc_release_MAC_descriptor_handle(wlc->wlfc_data,
				scbd->mac_address_handle);
			WLFC_DBGMESG(("STA: MAC-DEL for [%02x:%02x:%02x:%02x:%02x:%02x], "
				"handle: [%d], if:%d, t_idx:%d..\n",
				scbd->ea.octet[0], scbd->ea.octet[1], scbd->ea.octet[2],
				scbd->ea.octet[3], scbd->ea.octet[4], scbd->ea.octet[5],
				scbd->mac_address_handle,
				((scbd->bsscfg->wlcif == NULL) ? 0 : scbd->bsscfg->wlcif->index),
				WLFC_MAC_DESC_GET_LOOKUP_INDEX(scbd->mac_address_handle)));
		}
	}
#endif /* PROP_TXSTATUS */

#ifdef AP
	/* free any leftover authentication state */
	if (scbd->challenge) {
		MFREE(wlc->osh, scbd->challenge, 2 + scbd->challenge[1]);
		scbd->challenge = NULL;
	}
	/* free WDS state */
	if (scbd->wds != NULL) {
		/* process event queue */
		wlc_eventq_flush(wlc->eventq);
		if (scbd->wds->wlif) {
			wlc_if_event(wlc, WLC_E_IF_DEL, scbd->wds);
			wl_del_if(wlc->wl, scbd->wds->wlif);
			scbd->wds->wlif = NULL;
			SCB_DWDS_DEACTIVATE(scbd);
		}
		wlc_wlcif_free(wlc, wlc->osh, scbd->wds);
		scbd->wds = NULL;
	}
	/* free wpaie if stored */
	if (scbd->wpaie) {
		MFREE(wlc->osh, scbd->wpaie, scbd->wpaie_len);
		scbd->wpaie_len = 0;
		scbd->wpaie = NULL;
	}
#endif /* AP */

	/* free any frame reassembly buffer */
	for (prio = 0; prio < NUMPRIO; prio++) {
		if (scbd->fragbuf[prio]) {
			PKTFREE(wlc->osh, scbd->fragbuf[prio], FALSE);
			scbd->fragbuf[prio] = NULL;
			scbd->fragresid[prio] = 0;
		}
	}

	FOREACHSCB(scbstate, &scbiter, scb) {
		if (scb->psta_prim == scbd)
			scb->psta_prim = NULL;
	}

	scbd->state = 0;

#if defined(PKTC) || defined(PKTC_DONGLE)
	/* Clear scb pointer in rfc */
	wlc_scb_pktc_disable(scbd);
#endif // endif

#ifndef INT_SCB_OPT
	if (SCB_INTERNAL(scbd)) {
		goto free;
	}
#endif // endif
	if (!ETHER_ISMULTI(scbd->ea.octet)) {
		wlc_scb_hash_del(wlc, scbd, remove->band->bandunit, SCB_BSSCFG(scbd));
	}
	/* delete it from the link list */
	wlc_scb_list_del(wlc, scbd, SCB_BSSCFG(scbd));

	/* update total allocated scb number */
	scbstate->nscb--;

free:
#ifdef SCBFREELIST
	wlc_scbadd_free(scbstate, remove);
#else
	/* free scb memory */
	wlc_scb_freemem(scbstate, remove);
#endif // endif

	return TRUE;
}

static void
wlc_scb_list_add(wlc_info_t *wlc, struct scb_info *scbinfo, wlc_bsscfg_t *bsscfg)
{
	scb_bsscfg_cubby_t *scb_cfg;

	ASSERT(bsscfg != NULL);

	scb_cfg = SCB_BSSCFG_CUBBY(wlc->scbstate, bsscfg);

	SCBSANITYCHECK((scb_cfg)->scb);

	/* update scb link list */
	scbinfo->next = SCBINFO(scb_cfg->scb);
#ifdef MACOSX
	scbinfo->next_copy = scbinfo->next;
#endif // endif
	scb_cfg->scb = scbinfo->scbpub;
}

static void
wlc_scb_list_del(wlc_info_t *wlc, struct scb *scbd, wlc_bsscfg_t *bsscfg)
{
	scb_bsscfg_cubby_t *scb_cfg;
	struct scb_info *scbinfo;
	struct scb_info *remove = SCBINFO(scbd);

	ASSERT(bsscfg != NULL);

	/* delete it from the link list */

	scb_cfg = SCB_BSSCFG_CUBBY(wlc->scbstate, bsscfg);
	scbinfo = SCBINFO(scb_cfg->scb);
	if (scbinfo == remove) {
		scb_cfg->scb = wlc_scb_getnext(scbd);
	} else {
		while (scbinfo) {
			SCBSANITYCHECK(scbinfo->scbpub);
			if (scbinfo->next == remove) {
				scbinfo->next = remove->next;
#ifdef MACOSX
				scbinfo->next_copy = scbinfo->next;
#endif // endif
				break;
			}
			scbinfo = scbinfo->next;
		}
		ASSERT(scbinfo != NULL);
	}
}

/** free all scbs of a bsscfg */
void
wlc_scb_bsscfg_scbclear(struct wlc_info *wlc, wlc_bsscfg_t *bsscfg, bool perm)
{
	struct scb_iter scbiter;
	struct scb *scb;

	if (wlc->scbstate == NULL)
		return;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		if (scb->permanent) {
			if (!perm)
				continue;
			scb->permanent = FALSE;
		}
		wlc_scbfree(wlc, scb);
	}
}

static struct scb *
wlc_scbvictim(wlc_info_t *wlc)
{
	uint oldest;
	struct scb *scb;
	struct scb *oldscb;
	uint now, age;
	struct scb_iter scbiter;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	wlc_bsscfg_t *bsscfg = NULL;

#ifdef AP
	/* search for an unauthenticated scb */
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (!scb->permanent && (scb->state == UNAUTHENTICATED))
			return scb;
	}
#endif /* AP */

	/* free the oldest scb */
	now = wlc->pub->now;
	oldest = 0;
	oldscb = NULL;
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		bsscfg = SCB_BSSCFG(scb);
		ASSERT(bsscfg != NULL);
		if (BSSCFG_STA(bsscfg) && bsscfg->BSS && SCB_ASSOCIATED(scb))
			continue;
		if (!scb->permanent && ((age = (now - scb->used)) >= oldest)) {
			oldest = age;
			oldscb = scb;
		}
	}
	/* handle extreme case(s): all are permanent ... or there are no scb's at all */
	if (oldscb == NULL)
		return NULL;

#ifdef AP
	bsscfg = SCB_BSSCFG(oldscb);

	if (BSSCFG_AP(bsscfg)) {
		/* if the oldest authenticated SCB has only been idle a short time then
		 * it is not a candidate to reclaim
		 */
		if (oldest < SCB_SHORT_TIMEOUT)
			return NULL;

		/* notify the station that we are deauthenticating it */
		(void)wlc_senddeauth(wlc, bsscfg, oldscb, &oldscb->ea, &bsscfg->BSSID,
		                     &bsscfg->cur_etheraddr, DOT11_RC_INACTIVITY);
		wlc_deauth_complete(wlc, bsscfg, WLC_E_STATUS_SUCCESS, &oldscb->ea,
		              DOT11_RC_INACTIVITY, 0);
	}
#endif /* AP */

	WL_ASSOC(("wl%d: %s: relcaim scb %s, idle %d sec\n",  wlc->pub->unit, __FUNCTION__,
	          bcm_ether_ntoa(&oldscb->ea, eabuf), oldest));

	return oldscb;
}

#if defined(PKTC) || defined(PKTC_DONGLE)
void
wlc_scb_pktc_enable(struct scb *scb, const wlc_key_info_t *key_info)
{
	wlc_bsscfg_t *bsscfg = SCB_BSSCFG(scb);
	wlc_info_t *wlc = bsscfg->wlc;
	wlc_key_info_t tmp_ki;

	SCB_PKTC_DISABLE(scb);

	/* XXX For now don't enable chaining for following configs.
	 * Will enable as and when functionality is added.
	 */
	if (wlc->wet && BSSCFG_STA(bsscfg))
		return;

	/* No chaining for non qos, non ampdu stas */
	if (!SCB_QOS(scb) || !SCB_WME(scb) || !SCB_AMPDU(scb)) {
		return;
	}

	if (!(SCB_ASSOCIATED(scb) || SCB_LEGACY_WDS(scb)) && !SCB_AUTHORIZED(scb)) {
		return;
	}

#ifdef PKTC_DONGLE
	if (BSS_TDLS_BUFFER_STA(scb->bsscfg))
		return;
#endif // endif

	if (key_info == NULL) {
		(void)wlc_keymgmt_get_scb_key(wlc->keymgmt, scb, WLC_KEY_ID_PAIRWISE,
			WLC_KEY_FLAG_NONE, &tmp_ki);
		key_info = &tmp_ki;
	}

	if (!WLC_KEY_ALLOWS_PKTC(key_info, SCB_BSSCFG(scb)))
		return;

	SCB_PKTC_ENABLE(scb);
}

void
wlc_scb_pktc_disable(struct scb *scb)
{
	wlc_bsscfg_t *bsscfg = SCB_BSSCFG(scb);

	if (bsscfg) {
		bool cidx;
		wlc_info_t *wlc = bsscfg->wlc;
		/* Invalidate rfc entry if scb is in it */
		cidx = (BSSCFG_STA(bsscfg) && !(SCB_DWDS_CAP(scb) || SCB_MAP_CAP(scb))) ? 0 : 1;
		if (wlc->pktc_info->rfc[cidx].scb == scb) {
			WL_NONE(("wl%d: %s: Invalidate rfc %d before freeing scb %p\n",
			         wlc->pub->unit, __FUNCTION__, cidx, scb));
			wlc->pktc_info->rfc[cidx].scb = NULL;
		}

#if defined(DWDS)
		/* For DWDS/MAP SCB's, if capabilities are reset before cleaning up chaining_info
		 * then wrong cidx is used to clean pktc_info->rfc.
		 * For sanity, checking for SCB in all entries.
		 */
		if (wlc->pktc_info->rfc[cidx ^ 1].scb == scb) {
			WL_ERROR(("wl%d: %s: ERROR: SCB capabilities are RESET\n",
				wlc->pub->unit, __FUNCTION__));
			WL_NONE(("wl%d: %s: Invalidate rfc %d before freeing scb %p\n",
			         wlc->pub->unit, __FUNCTION__, cidx, scb));
			wlc->pktc_info->rfc[cidx].scb = NULL;
		}
#endif /* DWDS */

	}

	SCB_PKTC_DISABLE(scb);
}
#endif /* PKTC || PKTC_DONGLE */

/** "|" operation. */
void
wlc_scb_setstatebit(struct scb *scb, uint8 state)
{
	wlc_bsscfg_t *bsscfg;
	wlc_info_t *wlc;
	scb_module_t *scbstate;
	uint8	oldstate;

	WL_NONE(("set state %x\n", state));
	ASSERT(scb != NULL);

	bsscfg = SCB_BSSCFG(scb);
	wlc = bsscfg->wlc;
	scbstate = wlc->scbstate;
	oldstate = scb->state;

	if (state & AUTHENTICATED)
	{
		scb->state &= ~PENDING_AUTH;
	}
	if (state & ASSOCIATED)
	{
		ASSERT((scb->state | state) & AUTHENTICATED);
		scb->state &= ~PENDING_ASSOC;
	}

#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	if (state & AUTHORIZED)
	{
		if (!((scb->state | state) & ASSOCIATED) && !SCB_LEGACY_WDS(scb) &&
		    !SCB_IS_IBSS_PEER(scb)) {
			char eabuf[ETHER_ADDR_STR_LEN];
			WL_ASSOC(("wlc_scb : authorized %s is not a associated station, "
				"state = %x\n", bcm_ether_ntoa(&scb->ea, eabuf),
				scb->state));
		}
	}
#endif /* BCMDBG || WLMSG_ASSOC */

	scb->state |= state;
	WL_NONE(("wlc_scb : state = %x\n", scb->state));

#if defined(PKTC) || defined(PKTC_DONGLE)
	/* When transitioning to ASSOCIATED/AUTHORIZED state try if we can
	 * enable packet chaining for this SCB.
	 */
	if (SCB_BSSCFG(scb))
		wlc_scb_pktc_enable(scb, NULL);
#endif // endif

	if (oldstate != scb->state)
	{
		scb_state_upd_data_t data;
		data.scb = scb;
		data.oldstate = oldstate;
		bcm_notif_signal(scbstate->scb_state_notif_hdl, &data);
	}
}

/** "& ~" operation */
void
wlc_scb_clearstatebit(struct scb *scb, uint8 state)
{
	wlc_bsscfg_t *bsscfg;
	wlc_info_t *wlc;
	scb_module_t *scbstate;
	uint8	oldstate;

	ASSERT(scb != NULL);
	WL_NONE(("clear state %x\n", state));
	bsscfg = SCB_BSSCFG(scb);
	wlc = bsscfg->wlc;
	scbstate = wlc->scbstate;
	oldstate = scb->state;
	scb->state &= ~state;
	WL_NONE(("wlc_scb : state = %x\n", scb->state));
#if defined(PKTC) || defined(PKTC_DONGLE)
	/* Clear scb pointer in rfc */
	wlc_scb_pktc_disable(scb);
#endif // endif
	if (oldstate != scb->state)
	{
		scb_state_upd_data_t data;
		data.scb = scb;
		data.oldstate = oldstate;
		bcm_notif_signal(scbstate->scb_state_notif_hdl, &data);
	}
}

/**
 * "|" operation . idx = position of the bsscfg in the wlc array of multi ssids.
 */
void
wlc_scb_setstatebit_bsscfg(struct scb *scb, uint8 state, int idx)
{
	ASSERT(scb != NULL);
	WL_NONE(("set state : %x   bsscfg idx : %d\n", state, idx));
	if (state & ASSOCIATED)
	{

		ASSERT(SCB_AUTHENTICATED_BSSCFG(scb, idx));
		/* clear all bits (idx is set below) */
		memset(&scb->auth_bsscfg, 0, SCB_BSSCFG_BITSIZE);
		scb->state &= ~PENDING_ASSOC;
	}

	if (state & AUTHORIZED)
	{
		ASSERT(SCB_ASSOCIATED_BSSCFG(scb, idx));
	}
	setbit(scb->auth_bsscfg, idx);
	scb->state |= state;
	WL_NONE(("wlc_scb : state = %x\n", scb->state));
}

/**
 * "& ~" operation .
 * idx = position of the bsscfg in the wlc array of multi ssids.
 */
void
wlc_scb_clearstatebit_bsscfg(struct scb *scb, uint8 state, int idx)

{
	int i;
	ASSERT(scb != NULL);
	WL_NONE(("clear state : %x   bsscfg idx : %d\n", state, idx));
	/*
	   any clear of a stable state should lead to clear a bit
	   Warning though : this implies that, if we want to switch from
	   associated to authenticated, the clear happens before the set
	   otherwise this bit will be clear in authenticated state.
	*/
	if ((state & AUTHENTICATED) || (state & ASSOCIATED) || (state & AUTHORIZED))
	{
		clrbit(scb->auth_bsscfg, idx);
	}
	/* quik hack .. clear first ... */
	scb->state &= ~state;
	for (i = 0; i < SCB_BSSCFG_BITSIZE; i++)
	{
		/* reset if needed */
		if (scb->auth_bsscfg[i])
		{
			scb->state |= state;
			break;
		}
	}
}

/** reset all state. */
void
wlc_scb_resetstate(struct scb *scb)
{
	WL_NONE(("reset state\n"));
	ASSERT(scb != NULL);
	memset(&scb->auth_bsscfg, 0, SCB_BSSCFG_BITSIZE);
	scb->state = 0;
	WL_NONE(("wlc_scb : state = %x\n", scb->state));
}

/** set/change bsscfg */
void
wlc_scb_set_bsscfg(struct scb *scb, wlc_bsscfg_t *cfg)
{
	wlc_bsscfg_t *oldcfg = SCB_BSSCFG(scb);
	wlc_info_t *wlc;

	ASSERT(cfg != NULL);

	wlc = cfg->wlc;

	scb->bsscfg = cfg;

	/* when assigning the owner the first time or when assigning a different owner */
	if (oldcfg == NULL || oldcfg != cfg) {
		wlcband_t *band = wlc_scbband(scb);
		wlc_rateset_t *rs;

		/* changing bsscfg */
		if (oldcfg != NULL) {
			/* delete scb from hash table and scb list of old bsscfg */
			wlc_scb_hash_del(wlc, scb, band->bandunit, oldcfg);
			wlc_scb_list_del(wlc, scb, oldcfg);
			/* add scb to hash table and scb list of new bsscfg */
			wlc_scb_hash_add(wlc, scb, band->bandunit, cfg);
			wlc_scb_list_add(wlc, SCBINFO(scb), cfg);
		}

		/* flag the scb is used by IBSS */
		if (!SCB_INTERNAL(scb)) {
			if (cfg->BSS)
				wlc_scb_clearstatebit(scb, TAKEN4IBSS);
			else {
				wlc_scb_resetstate(scb);
				wlc_scb_setstatebit(scb, TAKEN4IBSS);
			}
		}

		/* invalidate txc */
		if (WLC_TXC_ENAB(wlc))
			wlc_txc_inv(wlc->txc, scb);

		/* use current, target, or per-band default rateset? */
		if (wlc->pub->up &&
#ifdef WLAWDL
			!BSSCFG_AWDL(wlc, cfg) &&
#endif // endif
			wlc_valid_chanspec(wlc->cmi, cfg->target_bss->chanspec))
			if (cfg->associated)
				rs = &cfg->current_bss->rateset;
			else
				rs = &cfg->target_bss->rateset;
		else
			rs = &band->defrateset;

		/*
		 * Initialize the per-scb rateset:
		 * - if we are AP, start with only the basic subset of the
		 *	network rates.  It will be updated when receive the next
		 *	probe request or association request.
		 * - if we are IBSS and gmode, special case:
		 *	start with B-only subset of network rates and probe for ofdm rates
		 * - else start with the network rates.
		 *	It will be updated on join attempts.
		 */
		/* initialize the scb rateset */
		if (BSSCFG_AP(cfg)) {
			uint8 mcsallow = 0;
#ifdef WLP2P
			if (BSS_P2P_ENAB(wlc, cfg))
				wlc_rateset_filter(rs /* src */, &scb->rateset /* dst */,
					FALSE, WLC_RATES_OFDM, RATE_MASK,
					wlc_get_mcsallow(wlc, cfg));
			else
#endif // endif
			/* XXX Does not match with the comment above. Remove the
			 * HT rates and possibly OFDM rates if not needed. If there
			 * is a valid reason to add the HT rates, then check if we have to
			 * add VHT rates as well.
			 */
			if (BSS_N_ENAB(wlc, cfg))
				mcsallow = WLC_MCS_ALLOW;
			wlc_rateset_filter(rs /* src */, &scb->rateset /* dst */,
				TRUE, WLC_RATES_CCK_OFDM, RATE_MASK, mcsallow);
		}
		else if (!cfg->BSS && band->gmode) {
			wlc_rateset_filter(rs /* src */, &scb->rateset /* dst */,
				FALSE, WLC_RATES_CCK, RATE_MASK, 0);
			/* if resulting set is empty, then take all network rates instead */
			if (scb->rateset.count == 0)
				wlc_rateset_filter(rs /* src */, &scb->rateset /* dst */,
					FALSE, WLC_RATES_CCK_OFDM, RATE_MASK, 0);
		}
		else {
#ifdef WLP2P
			if (BSS_P2P_ENAB(wlc, cfg))
				wlc_rateset_filter(rs /* src */, &scb->rateset /* dst */, FALSE,
					WLC_RATES_OFDM, RATE_MASK, wlc_get_mcsallow(wlc, cfg));
			else
#endif // endif
			wlc_rateset_filter(rs /* src */, &scb->rateset /* dst */,
				FALSE, WLC_RATES_CCK_OFDM, RATE_MASK, 0);
		}

		if (!SCB_INTERNAL(scb)) {
			wlc_scb_ratesel_init(wlc, scb);
#ifdef STA
			/* send ofdm rate probe */
			if (BSSCFG_STA(cfg) && !cfg->BSS &&
#ifdef WLAWDL
				!BSSCFG_AWDL(wlc, cfg) &&
#endif // endif
			    band->gmode && wlc->pub->up)
				wlc_rateprobe(wlc, cfg, &scb->ea, WLC_RATEPROBE_RSPEC);
#endif /* STA */
		}

		wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_SCB_BSSCFG_CHANGED,
			oldcfg, scb, NULL, NULL);
	}
}

static void
wlc_scb_bsscfg_reinit(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	uint prev_count;
	const wlc_rateset_t *rs;
	wlcband_t *band;
	struct scb *scb;
	struct scb_iter scbiter;
	bool cck_only;
	bool reinit_forced;

	WL_INFORM(("wl%d: %s: bandunit 0x%x phy_type 0x%x gmode 0x%x\n", wlc->pub->unit,
		__FUNCTION__, wlc->band->bandunit, wlc->band->phytype, wlc->band->gmode));

	/* sanitize any existing scb rates against the current hardware rates */
	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		/* XXX SCB : should the following be done only if scb->bandunit matches
		 * wlc->band->bandunit?
		 */
		prev_count = scb->rateset.count;
		/* Keep only CCK if gmode == GMODE_LEGACY_B */
		band = SCBINFO(scb)->band;
		if (BAND_2G(band->bandtype) && (band->gmode == GMODE_LEGACY_B)) {
			rs = &cck_rates;
			cck_only = TRUE;
		} else {
			rs = &band->hw_rateset;
			cck_only = FALSE;
		}
		if (!wlc_rate_hwrs_filter_sort_validate(&scb->rateset /* [in+out] */, rs /* [in] */,
			FALSE, wlc->stf->op_txstreams)) {
			/* continue with default rateset.
			 * since scb rateset does not carry basic rate indication,
			 * clear basic rate bit.
			 */
			WL_RATE(("wl%d: %s: invalid rateset in scb 0x%p bandunit 0x%x "
				"phy_type 0x%x gmode 0x%x\n", wlc->pub->unit, __FUNCTION__,
				scb, band->bandunit, band->phytype, band->gmode));
#ifdef BCMDBG
			wlc_rateset_show(wlc, &scb->rateset, &scb->ea);
#endif // endif

			wlc_rateset_default(&scb->rateset, &band->hw_rateset,
			                    band->phytype, band->bandtype, cck_only, RATE_MASK,
			                    wlc_get_mcsallow(wlc, scb->bsscfg),
			                    CHSPEC_WLC_BW(scb->bsscfg->current_bss->chanspec),
			                    wlc->stf->op_txstreams);
			reinit_forced = TRUE;
		}
		else
			reinit_forced = FALSE;

		/* if the count of rates is different, then the rate state
		 * needs to be reinitialized
		 */
		if (reinit_forced || (scb->rateset.count != prev_count))
			wlc_scb_ratesel_init(wlc, scb);

		WL_RATE(("wl%d: %s: bandunit 0x%x, phy_type 0x%x gmode 0x%x. final rateset is\n",
			wlc->pub->unit, __FUNCTION__,
			band->bandunit, band->phytype, band->gmode));
#ifdef BCMDBG
		wlc_rateset_show(wlc, &scb->rateset, &scb->ea);
#endif // endif
		wlc_keymgmt_notify(wlc->keymgmt, WLC_KEYMGMT_NOTIF_SCB_BSSCFG_CHANGED,
			NULL /* oldcfg */, scb, NULL, NULL);
	}
}

void
wlc_scb_reinit(wlc_info_t *wlc)
{
	int32 idx;
	wlc_bsscfg_t *bsscfg;

	FOREACH_BSS(wlc, idx, bsscfg) {
		wlc_scb_bsscfg_reinit(wlc, bsscfg);
	}
}

static INLINE struct scb* BCMFASTPATH
_wlc_scbfind(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, const struct ether_addr *ea, int bandunit)
{
	int indx;
	struct scb_info *scbinfo;
	scb_bsscfg_cubby_t *scb_cfg;
#if defined(BCMDBG)
	char sa[ETHER_ADDR_STR_LEN];
#endif // endif

	ASSERT(bsscfg != NULL);

	/* All callers of wlc_scbfind() should first be checking to see
	 * if the SCB they're looking for is a BC/MC address.  Because we're
	 * using per bsscfg BCMC SCBs, we can't "find" BCMC SCBs without
	 * knowing which bsscfg.
	 */
	ASSERT(ea);
#if defined(BCMDBG)
	if (ea && ETHER_ISMULTI(ea)) {
		WL_ERROR(("wl%d: %s: ea %s\n", wlc->pub->unit, __FUNCTION__,
			bcm_ether_ntoa(ea, sa)));
	}
#endif // endif
	ASSERT(!ETHER_ISMULTI(ea));

	/* search for the scb which corresponds to the remote station ea */
	scb_cfg = SCB_BSSCFG_CUBBY(wlc->scbstate, bsscfg);
	if (scb_cfg) {
		indx = SCBHASHINDEX(scb_cfg->nscbhash, ea->octet);
		scbinfo = (scb_cfg->scbhash[bandunit][indx] ?
			SCBINFO(scb_cfg->scbhash[bandunit][indx]) : NULL);
		for (; scbinfo; scbinfo = scbinfo->hashnext) {
			SCBSANITYCHECK(scbinfo->scbpub);
			if (memcmp((const char*)ea,
				(const char*)&(scbinfo->scbpub->ea),
				sizeof(*ea)) == 0)
				break;
		}

		return (scbinfo ? scbinfo->scbpub : NULL);
	}
	return (NULL);
}

/** Find station control block corresponding to the remote id */
struct scb * BCMFASTPATH
wlc_scbfind(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, const struct ether_addr *ea)
{
	struct scb *scb = NULL;

	scb = _wlc_scbfind(wlc, bsscfg, ea, wlc->band->bandunit);

#if defined(WLMCHAN) || defined(WLAWDL)
/* current band could be different, so search again for all scb's */
	if (!scb && MCHAN_ACTIVE(wlc->pub) && NBANDS(wlc) > 1)
		scb = wlc_scbfindband(wlc, bsscfg, ea, OTHERBANDUNIT(wlc));
#endif /* WLMCHAN || WLAWDL */
	return scb;
}

/**
 * Lookup station control block corresponding to the remote id.
 * If not found, create a new entry.
 */
static INLINE struct scb *
_wlc_scblookup(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, const struct ether_addr *ea, int bandunit)
{
	struct scb *scb;
	struct wlcband *band;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char sa[ETHER_ADDR_STR_LEN];
#endif // endif

	/* Don't allocate/find a BC/MC SCB this way. */
	ASSERT(!ETHER_ISMULTI(ea));
	if (ETHER_ISMULTI(ea))
		return NULL;

	/* apply mac filter */
	switch (wlc_macfltr_addr_match(wlc->macfltr, bsscfg, ea)) {
	case WLC_MACFLTR_ADDR_DENY:
		WL_ASSOC(("wl%d.%d mac restrict: Denying %s\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
		          bcm_ether_ntoa(ea, sa)));
		return NULL;
	case WLC_MACFLTR_ADDR_NOT_ALLOW:
		WL_ASSOC(("wl%d.%d mac restrict: Not allowing %s\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
		          bcm_ether_ntoa(ea, sa)));
		return NULL;
#ifdef BCMDBG
	case WLC_MACFLTR_ADDR_ALLOW:
		WL_ASSOC(("wl%d.%d mac restrict: Allowing %s\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
		          bcm_ether_ntoa(ea, sa)));
		break;
	case WLC_MACFLTR_ADDR_NOT_DENY:
		WL_ASSOC(("wl%d.%d mac restrict: Not denying %s\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
		          bcm_ether_ntoa(ea, sa)));
		break;
	case WLC_MACFLTR_DISABLED:
		WL_NONE(("wl%d.%d no mac restrict: lookup %s\n",
		         wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
		         bcm_ether_ntoa(ea, sa)));
		break;
#endif /* BCMDBG */
	}

	if ((scb = _wlc_scbfind(wlc, bsscfg, ea, bandunit)))
		return (scb);

	/* no scb match, allocate one for the desired bandunit */
	band = wlc->bandstate[bandunit];
	return wlc_userscb_alloc(wlc, bsscfg, ea, band);
}

struct scb *
wlc_scblookup(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, const struct ether_addr *ea)
{
	return (_wlc_scblookup(wlc, bsscfg, ea, wlc->band->bandunit));
}

struct scb *
wlc_scblookupband(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, const struct ether_addr *ea, int bandunit)
{
	/* assert that the band is the current band, or we are dual band and it is the other band */
	ASSERT((bandunit == (int)wlc->band->bandunit) ||
	       (NBANDS(wlc) > 1 && bandunit == (int)OTHERBANDUNIT(wlc)));

	return (_wlc_scblookup(wlc, bsscfg, ea, bandunit));
}

/** Get scb from band */
struct scb * BCMFASTPATH
wlc_scbfindband(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, const struct ether_addr *ea, int bandunit)
{
	/* assert that the band is the current band, or we are dual band and it is the other band */
	ASSERT((bandunit == (int)wlc->band->bandunit) ||
	       (NBANDS(wlc) > 1 && bandunit == (int)OTHERBANDUNIT(wlc)));

	return (_wlc_scbfind(wlc, bsscfg, ea, bandunit));
}

/**
 * Determine if any SCB associated to ap cfg
 * cfg specifies a specific ap cfg to compare to.
 * If cfg is NULL, then compare to any ap cfg.
 */
bool
wlc_scb_associated_to_ap(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	struct scb_iter scbiter;
	struct scb *scb;
	bool associated = FALSE;

	ASSERT((cfg == NULL) || BSSCFG_AP(cfg));

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_ASSOCIATED(scb) && BSSCFG_AP(scb->bsscfg)) {
			if ((cfg == NULL) || (cfg == scb->bsscfg)) {
				associated = TRUE;
			}
		}
	}

	return (associated);
}

void wlc_scb_switch_band(wlc_info_t *wlc, struct scb *scb, int new_bandunit,
	wlc_bsscfg_t *bsscfg)
{
	struct scb_info *scbinfo = SCBINFO(scb);

	/* first, del scb from hash table in old band */
	wlc_scb_hash_del(wlc, scb, scb->bandunit, bsscfg);
	/* next add scb to hash table in new band */
	wlc_scb_hash_add(wlc, scb, new_bandunit, bsscfg);
	/* update the scb's band */
	scb->bandunit = (uint)new_bandunit;
	scbinfo->band = wlc->bandstate[new_bandunit];

	return;
}

void wlc_internal_scb_switch_band(wlc_info_t *wlc, struct scb *scb, int new_bandunit)
{
	struct scb_info *scbinfo = SCBINFO(scb);
	/* update the scb's band */
	scb->bandunit = (uint)new_bandunit;
	scbinfo->band = wlc->bandstate[new_bandunit];
}

/**
 * Move the scb's band info.
 * Parameter description:
 *
 * wlc - global wlc_info structure
 * bsscfg - the bsscfg that is about to move to a new chanspec
 * chanspec - the new chanspec the bsscfg is moving to
 *
 */
void
wlc_scb_update_band_for_cfg(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, chanspec_t chanspec)
{
	struct scb_iter scbiter;
	struct scb *scb, *stale_scb;
	int bandunit;
	bool reinit = FALSE;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		if (SCB_ASSOCIATED(scb)) {
			bandunit = CHSPEC_WLCBANDUNIT(chanspec);
			if (scb->bandunit != (uint)bandunit) {
				/* We're about to move our scb to the new band.
				 * Check to make sure there isn't an scb entry for us there.
				 * If there is one for us, delete it first.
				 */
				if ((stale_scb = _wlc_scbfind(wlc, bsscfg,
				                      &bsscfg->BSSID, bandunit)) &&
				    (stale_scb->permanent == FALSE)) {
					WL_ASSOC(("wl%d.%d: %s: found stale scb %p on %s band, "
					          "remove it\n",
					          wlc->pub->unit, bsscfg->_idx, __FUNCTION__,
					          stale_scb,
					          (bandunit == BAND_5G_INDEX) ? "5G" : "2G"));
					/* mark the scb for removal */
					stale_scb->stale_remove = TRUE;
				}
				/* Now perform the move of our scb to the new band */
				wlc_scb_switch_band(wlc, scb, bandunit, bsscfg);
				reinit = TRUE;
			}
		}
	}
	/* remove stale scb's marked for removal */
	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		if (scb->stale_remove == TRUE) {
			WL_ASSOC(("remove stale scb %p\n", scb));
			scb->stale_remove = FALSE;
			wlc_scbfree(wlc, scb);
		}
	}

	if (reinit) {
		wlc_scb_reinit(wlc);
	}
}

struct scb *
wlc_scbibssfindband(wlc_info_t *wlc, const struct ether_addr *ea, int bandunit,
                    wlc_bsscfg_t **bsscfg)
{
	int idx;
	wlc_bsscfg_t *cfg;
	struct scb *scb = NULL;

	/* assert that the band is the current band, or we are dual band
	 * and it is the other band.
	 */
	ASSERT((bandunit == (int)wlc->band->bandunit) ||
	       (NBANDS(wlc) > 1 && bandunit == (int)OTHERBANDUNIT(wlc)));

	FOREACH_IBSS(wlc, idx, cfg) {
		/* Find the bsscfg and scb matching specified peer mac */
		scb = _wlc_scbfind(wlc, cfg, ea, bandunit);
		if (scb != NULL) {
			*bsscfg = cfg;
			break;
		}
	}

	return scb;
}

struct scb *
wlc_scbapfind(wlc_info_t *wlc, const struct ether_addr *ea, wlc_bsscfg_t **bsscfg)
{
	int idx;
	wlc_bsscfg_t *cfg;
	struct scb *scb = NULL;

	*bsscfg = NULL;

	FOREACH_UP_AP(wlc, idx, cfg) {
		/* Find the bsscfg and scb matching specified peer mac */
		scb = wlc_scbfind(wlc, cfg, ea);
		if (scb != NULL) {
			*bsscfg = cfg;
			break;
		}
	}

	return scb;
}

struct scb * BCMFASTPATH
wlc_scbbssfindband(wlc_info_t *wlc, const struct ether_addr *hwaddr,
                   const struct ether_addr *ea, int bandunit, wlc_bsscfg_t **bsscfg)
{
	int idx;
	wlc_bsscfg_t *cfg;
	struct scb *scb = NULL;

	/* assert that the band is the current band, or we are dual band
	 * and it is the other band.
	 */
	ASSERT((bandunit == (int)wlc->band->bandunit) ||
	       (NBANDS(wlc) > 1 && bandunit == (int)OTHERBANDUNIT(wlc)));

	*bsscfg = NULL;

	FOREACH_BSS(wlc, idx, cfg) {
		/* Find the bsscfg and scb matching specified hwaddr and peer mac */
		if (eacmp(cfg->cur_etheraddr.octet, hwaddr->octet) == 0) {
			scb = _wlc_scbfind(wlc, cfg, ea, bandunit);
#ifdef WLAWDL
			if (!scb && (BSSCFG_AWDL(wlc, cfg)))
				scb = _wlc_scbfind(wlc, cfg, ea, OTHERBANDUNIT(wlc));
#endif // endif
			if (scb != NULL) {
				*bsscfg = cfg;
				break;
			}
		}
	}

	return scb;
}

/**
 * (de)authorize/(de)authenticate single station
 * 'enable' TRUE means authorize, FLASE means deauthorize/deauthenticate
 * 'flag' is AUTHORIZED or AUTHENICATED for the type of operation
 * 'rc' is the reason code for a deauthenticate packet
 */
void
wlc_scb_set_auth(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb, bool enable, uint32 flag,
                 int rc)
{
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	void *pkt = NULL;

	if (SCB_LEGACY_WDS(scb)) {
		WL_ERROR(("wl%d.%d %s: WDS=" MACF " enable=%d flag=%x\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
				__FUNCTION__, ETHERP_TO_MACF(&scb->ea), enable, flag));
	}

	if (enable) {
		if (flag == AUTHORIZED) {
			wlc_scb_setstatebit(scb, AUTHORIZED);
			scb->flags &= ~SCB_DEAUTH;

			if (BSSCFG_AP(bsscfg) && wlc_eventq_test_ind(wlc->eventq, WLC_E_AUTHORIZED))
				wlc_bss_mac_event(wlc, bsscfg, WLC_E_AUTHORIZED,
					(struct ether_addr *)&scb->ea,
					WLC_E_AUTHORIZED, 0, 0, 0, 0);

#ifdef WL11N
			if (SCB_MFP(scb) && N_ENAB(wlc->pub) && SCB_AMPDU(scb) &&
				(scb->wsec == AES_ENABLED)) {
				wlc_scb_ampdu_enable(wlc, scb);
			}
#endif /* WL11N */
#ifdef TRAFFIC_MGMT
		if (BSSCFG_AP(bsscfg)) {
			wlc_scb_trf_mgmt(wlc, bsscfg, scb);
		}
#endif // endif
		} else {
			wlc_scb_setstatebit(scb, AUTHENTICATED);
		}
	} else {
		if (flag == AUTHORIZED) {

			wlc_scb_clearstatebit(scb, AUTHORIZED);
		} else {

			if (wlc->pub->up && (SCB_AUTHENTICATED(scb) || SCB_LEGACY_WDS(scb))) {
				/* It was observed that if AP is running in DWDS AP mode along with
				 * another DWDS repeater and both have the same ssid, one of them
				 * can have the stale entry of client if client was earlier
				 * connected to either of two and later join to another with wl
				 * join command.
				 *
				 * This stale entry persist at AP/DWDS repeater end. The reason for
				 * this behaviour is due to ucode level ACK is coming for the Qos
				 * NUll frame being sent from the AP/DWDS repeater if client's
				 * idle timeout is over.
				 *
				 * Proposed solution:-> Remove client's entry via IOVAR requested
				 * by Application. This is the task of Application to figure out
				 * the stale entry of client among the AP/Repeaters.
				 *
				 * Mark SCB for Deletion, in case STA already associated/roamed to
				 * another DWDS interface(or Repeater). No need to send explicit
				 * deauth for this case.
				 */
				if (rc == DOT11_RC_STALE_DETECTION) {
					/* Clear states and mark the scb for deletion. SCB free
					 * will happen from the inactivity timeout context in
					 * wlc_ap_stastimeout()
					 */
					wlc_scb_clearstatebit(scb, AUTHENTICATED | ASSOCIATED
							| AUTHORIZED);
					wlc_scb_setstatebit(scb, MARKED_FOR_DELETION);
				} else {
					pkt = wlc_senddeauth(wlc, bsscfg, scb, &scb->ea,
						&bsscfg->BSSID,	&bsscfg->cur_etheraddr,
						(uint16)rc);
				}
			}
			if (pkt != NULL) {
				wlc_deauth_send_cbargs_t *args;

				args = MALLOC(wlc->osh, sizeof(wlc_deauth_send_cbargs_t));
				bcopy(&scb->ea, &args->ea, sizeof(struct ether_addr));
				args->_idx = WLC_BSSCFG_IDX(bsscfg);
				args->pkt = pkt;
				if (wlc_pcb_fn_register(wlc->pcb,
					wlc_deauth_sendcomplete, (void *)args, pkt))
					WL_ERROR(("wl%d: wlc_scb_set_auth: could not "
					          "register callback\n", wlc->pub->unit));
			}
		}
	}
	WL_ASSOC(("wl%d: %s: %s %s%s\n", wlc->pub->unit, __FUNCTION__,
		bcm_ether_ntoa(&scb->ea, eabuf),
		(enable ? "" : "de"),
		((flag == AUTHORIZED) ? "authorized" : "authenticated")));
}

static void
wlc_scb_hash_add(wlc_info_t *wlc, struct scb *scb, int bandunit, wlc_bsscfg_t *bsscfg)
{
	scb_bsscfg_cubby_t *scb_cfg;
	int indx;
	struct scb_info *scbinfo;

	ASSERT(bsscfg != NULL);

	scb_cfg = SCB_BSSCFG_CUBBY(wlc->scbstate, bsscfg);
	indx = SCBHASHINDEX(scb_cfg->nscbhash, scb->ea.octet);
	scbinfo = (scb_cfg->scbhash[bandunit][indx] ?
	           SCBINFO(scb_cfg->scbhash[bandunit][indx]) : NULL);

	SCBINFO(scb)->hashnext = scbinfo;
#ifdef MACOSX
	SCBINFO(scb)->hashnext_copy = SCBINFO(scb)->hashnext;
#endif // endif

	scb_cfg->scbhash[bandunit][indx] = scb;
}

static void
wlc_scb_hash_del(wlc_info_t *wlc, struct scb *scbd, int bandunit, wlc_bsscfg_t *bsscfg)
{
	scb_bsscfg_cubby_t *scb_cfg;
	int indx;
	struct scb_info *scbinfo;
	struct scb_info *remove = SCBINFO(scbd);

	ASSERT(bsscfg != NULL);

	scb_cfg = SCB_BSSCFG_CUBBY(wlc->scbstate, bsscfg);
	indx = SCBHASHINDEX(scb_cfg->nscbhash, scbd->ea.octet);

	/* delete it from the hash */
	scbinfo = (scb_cfg->scbhash[bandunit][indx] ?
	           SCBINFO(scb_cfg->scbhash[bandunit][indx]) : NULL);
	ASSERT(scbinfo != NULL);
	SCBSANITYCHECK(scbinfo->scbpub);
	/* special case for the first */
	if (scbinfo == remove) {
		if (scbinfo->hashnext)
		    SCBSANITYCHECK(scbinfo->hashnext->scbpub);
		scb_cfg->scbhash[bandunit][indx] =
		        (scbinfo->hashnext ? scbinfo->hashnext->scbpub : NULL);
	} else {
		for (; scbinfo; scbinfo = scbinfo->hashnext) {
			SCBSANITYCHECK(scbinfo->hashnext->scbpub);
			if (scbinfo->hashnext == remove) {
				scbinfo->hashnext = remove->hashnext;
#ifdef MACOSX
				scbinfo->hashnext_copy = scbinfo->hashnext;
#endif // endif
				break;
			}
		}
		ASSERT(scbinfo != NULL);
	}
}

void
wlc_scb_sortrates(wlc_info_t *wlc, struct scb *scb)
{
	struct scb_info *scbinfo = SCBINFO(scb);
	wlc_rate_hwrs_filter_sort_validate(&scb->rateset /* [in+out] */,
		&scbinfo->band->hw_rateset /* [in] */, FALSE,
		wlc->stf->op_txstreams);
}

void
BCMINITFN(wlc_scblist_validaterates)(wlc_info_t *wlc)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		wlc_scb_sortrates(wlc, scb);
		if (scb->rateset.count == 0)
			wlc_scbfree(wlc, scb);
	}
}

#if defined(AP) || defined(WLTDLS)
int
wlc_scb_rssi(struct scb *scb)
{
	int rssi = 0, cnt;
	int i;

	for (i = 0, cnt = 0; i < MA_WINDOW_SZ; i++)
		if (scb->rssi_window[i] != WLC_RSSI_INVALID)
		{
			rssi += scb->rssi_window[i];
			cnt++;
		}
	if (cnt > 1) rssi /= cnt;

	return (rssi);
}

int8
wlc_scb_rssi_chain(struct scb *scb, int chain)
{
	int8 rssi_avg = WLC_RSSI_INVALID, cnt;
	int32 rssi = 0;
	int i;

	for (i = 0, cnt = 0; i < MA_WINDOW_SZ; i++) {
		if (scb->rssi_chain[chain][i] != WLC_RSSI_INVALID) {
			rssi += scb->rssi_chain[chain][i];
			cnt++;
		}
	}

	if (cnt >= 1) {
		rssi_avg = rssi/cnt;
	}

	return (rssi_avg);
}

/* return the rssi of last received packet per scb and
 * per antenna chain.
 */
int8
wlc_scb_pkt_rssi_chain(struct scb *scb, int chain)
{
	int last_rssi_index;
	int8 rssi = 0;

	last_rssi_index = MODDEC_POW2(scb->rssi_index, MA_WINDOW_SZ);
	if ((chain >= WL_ANT_IDX_1) && (chain < WL_RSSI_ANT_MAX) &&
		(scb->rssi_chain[chain][last_rssi_index] != WLC_RSSI_INVALID))
		rssi = (int8)scb->rssi_chain[chain][last_rssi_index];

	return rssi;
}

void
wlc_scb_rssi_init(struct scb *scb, int rssi)
{
	int i, j;
	scb->rssi_enabled = 1;

	for (i = 0; i < MA_WINDOW_SZ; i++) {
		scb->rssi_window[i] = rssi;
		for (j = 0; j < WL_RSSI_ANT_MAX; j++)
			scb->rssi_chain[j][i] = rssi;
	}

	scb->rssi_index = 0;
}

/** Enable or disable RSSI update for a particular requestor module */
bool
wlc_scb_rssi_update_enable(struct scb *scb, bool enable, scb_rssi_requestor_t rid)
{
	if (enable) {
		scb->rssi_upd |= (1<<rid);
	} else {
		scb->rssi_upd &= ~(1<<rid);
	}
	return (scb->rssi_upd != 0);
}

#endif /* AP || WLTDLS */

/**
 * Give then tx_fn, return the feature id from txmod_fns array.
 * If tx_fn is NULL, 0 will be returned
 * If entry is not found, it's an ERROR!
 */
static INLINE scb_txmod_t
wlc_scb_txmod_fid(wlc_info_t *wlc, txmod_tx_fn_t tx_fn)
{
	scb_txmod_t txmod;

	for (txmod = TXMOD_START; txmod < TXMOD_LAST; txmod++)
		if (tx_fn == wlc->txmod_fns[txmod].tx_fn)
			return txmod;

	/* Should not reach here */
	ASSERT(txmod < TXMOD_LAST);
	return txmod;
}

/**
 * Add a feature to the path. It should not be already on the path and should be configured
 * Does not take care of evicting anybody
 */
void
wlc_scb_txmod_activate(wlc_info_t *wlc, struct scb *scb, scb_txmod_t fid)
{
	/* Numeric value designating this feature's position in tx_path */
	static const uint8 txmod_position[TXMOD_LAST] = {
		0, /* TXMOD_START */
		1, /* TXMOD_TDLS */
		6, /* TXMOD_APPS */
		2, /* TXMOD_TRF_MGMT */
		3, /* TXMOD_NAR */
		4, /* TXMOD_AMSDU */
		5, /* TXMOD_AMPDU */
		7, /* TXMOD_TRANSMIT */
	};

	uint curr_mod_position;
	scb_txmod_t prev, next;
	txmod_info_t curr_mod_info = wlc->txmod_fns[fid];

	ASSERT(SCB_TXMOD_CONFIGURED(scb, fid) &&
	       !SCB_TXMOD_ACTIVE(scb, fid));

	curr_mod_position = txmod_position[fid];

	prev = TXMOD_START;

	while ((next = wlc_scb_txmod_fid(wlc, scb->tx_path[prev].next_tx_fn)) != 0 &&
	       txmod_position[next] < curr_mod_position)
		prev = next;

	/* next == 0 indicate this is the first addition to the path
	 * it HAS to be TXMOD_TRANSMIT as it's the one that puts the packet in
	 * txq. If this changes, then assert will need to be removed.
	 */
	ASSERT(next != 0 || fid == TXMOD_TRANSMIT);
	ASSERT(txmod_position[next] != curr_mod_position);

	SCB_TXMOD_SET(scb, prev, fid);
	SCB_TXMOD_SET(scb, fid, next);

	/* invoke any activate notify functions now that it's in the path */
	if (curr_mod_info.activate_notify_fn)
		curr_mod_info.activate_notify_fn(curr_mod_info.ctx, scb);
}

/**
 * Remove a fid from the path. It should be already on the path
 * Does not take care of replacing it with any other feature.
 */
void
wlc_scb_txmod_deactivate(wlc_info_t *wlc, struct scb *scb, scb_txmod_t fid)
{
	scb_txmod_t prev, next;
	txmod_info_t curr_mod_info = wlc->txmod_fns[fid];

	/* If not active, do nothing */
	if (!SCB_TXMOD_ACTIVE(scb, fid))
		return;

	/* if deactivate notify function is present, call it */
	if (curr_mod_info.deactivate_notify_fn)
		curr_mod_info.deactivate_notify_fn(curr_mod_info.ctx, scb);

	prev = TXMOD_START;

	while ((next = wlc_scb_txmod_fid(wlc, scb->tx_path[prev].next_tx_fn))
	       != fid)
		prev = next;

	SCB_TXMOD_SET(scb, prev, wlc_scb_txmod_fid(wlc, scb->tx_path[fid].next_tx_fn));
	scb->tx_path[fid].next_tx_fn = NULL;
}

#ifdef WLAWDL
void
wlc_scb_awdl_free(struct wlc_info *wlc)
{
	struct scb *scb;
	struct scb_iter scbiter;
	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (scb->bsscfg && BSSCFG_AWDL(wlc, scb->bsscfg)) {
			scb->permanent = FALSE;
			wlc_scbfree(wlc, scb);
		}
	}
}
struct scb * wlc_scbfind_dualband(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	struct ether_addr *addr)
{
	struct scb *scb;
	scb = wlc_scbfind(wlc, bsscfg, addr);
	if (NBANDS(wlc) == 1)
		return scb;
	if (!scb)
		scb = wlc_scbfindband(wlc, bsscfg, addr, OTHERBANDUNIT(wlc));
	return scb;
}
#endif /* WLAWDL */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_scb_txpath_dump(wlc_info_t *wlc, struct scb *scb, struct bcmstrbuf *b)
{
	static const char *txmod_names[TXMOD_LAST] = {
		"Start",
		"TDLS",
		"APPS",
		"Traffic Mgmt",
		"NAR",
		"A-MSDU",
		"A-MPDU",
		"Transmit",
#ifdef TXQ_MUX
		"SCBQ"
#endif // endif
	};
	scb_txmod_t fid, next_fid;

	bcm_bprintf(b, "     Tx Path: ");
	fid = TXMOD_START;
	do {
		next_fid = wlc_scb_txmod_fid(wlc, scb->tx_path[fid].next_tx_fn);
		/* for each txmod print out name and # total pkts held fr all scbs */
		bcm_bprintf(b, "-> %s (allscb pkts=%u)",
			txmod_names[next_fid],
			(wlc->txmod_fns[next_fid].pktcnt_fn) ?
			wlc_txmod_get_pkts_pending(wlc, next_fid) : -1);
		fid = next_fid;
	} while (fid != TXMOD_TRANSMIT && fid != 0);
	bcm_bprintf(b, "\n");
	return 0;
}

void
wlc_scb_dump_scb(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct scb *scb, struct bcmstrbuf *b, int idx)
{
	uint i;
	char eabuf[ETHER_ADDR_STR_LEN];
	char flagstr[64];
	char flagstr2[64];
	char flagstr3[64];
	char statestr[64];
	cubby_info_t *cubby_info;
	cubby_info_ctx_t *cubby_info_ctx;
#ifdef AP
	char ssidbuf[SSID_FMT_BUF_LEN] = "";
#endif /* AP */

	bcm_format_flags(scb_flags, scb->flags, flagstr, 64);
	bcm_format_flags(scb_flags2, scb->flags2, flagstr2, 64);
	bcm_format_flags(scb_flags3, scb->flags3, flagstr3, 64);
	bcm_format_flags(scb_states, scb->state, statestr, 64);

	if (SCB_INTERNAL(scb))
		bcm_bprintf(b, "  I");
	else
		bcm_bprintf(b, "%3d", idx);
	bcm_bprintf(b, "%s %s\n", (scb->permanent? "*":" "),
	            bcm_ether_ntoa(&scb->ea, eabuf));

	bcm_bprintf(b, "     State:0x%02x (%s) Used:%d(%d)\n",
	            scb->state, statestr, scb->used,
	            (int)(scb->used - wlc->pub->now));
	bcm_bprintf(b, "     Band:%s",
	            ((scb->bandunit == BAND_2G_INDEX) ? BAND_2G_NAME :
	             BAND_5G_NAME));
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "     Flags:0x%x", scb->flags);
	if (flagstr[0] != '\0')
		bcm_bprintf(b, " (%s)", flagstr);
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "     Flags2:0x%x", scb->flags2);
	if (flagstr2[0] != '\0')
		bcm_bprintf(b, " (%s)", flagstr2);
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "     Flags3:0x%x", scb->flags3);
	if (flagstr3[0] != '\0')
		bcm_bprintf(b, " (%s)", flagstr3);
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "\n");
	if (cfg != NULL)
		bcm_bprintf(b, "     Cfg:%d(%p)", WLC_BSSCFG_IDX(cfg), cfg);

	bcm_bprintf(b, "\n");

	wlc_dump_rateset("     rates", &scb->rateset, b);
	bcm_bprintf(b, "\n");

	if (scb->rateset.htphy_membership) {
		bcm_bprintf(b, "     membership %d(b)",
		            (scb->rateset.htphy_membership & RATE_MASK));
		bcm_bprintf(b, "\n");
		bcm_bprintf(b, "     Prop HT rates support:%d\n",
			(scb->flags2 & SCB2_HT_PROP_RATES_CAP) ? 1: 0);
	}

#ifdef AP
	if (cfg != NULL && BSSCFG_AP(cfg)) {
		bcm_bprintf(b, "     AID:0x%x PS:%d Listen:%d WDS:%d(%p) RSSI:%d",
		            scb->aid, scb->PS, scb->listen, (scb->wds ? 1 : 0),
		            scb->wds, wlc_scb_rssi(scb));
		wlc_format_ssid(ssidbuf, cfg->SSID, cfg->SSID_len);
		bcm_bprintf(b, " BSS %d \"%s\"\n",
		            WLC_BSSCFG_IDX(cfg), ssidbuf);
	}
#endif // endif
#ifdef STA
	if (cfg != NULL && BSSCFG_STA(cfg)) {
		bcm_bprintf(b, "     MAXSP:%u DEFL:0x%x TRIG:0x%x DELV:0x%x\n",
		            scb->apsd.maxsplen, scb->apsd.ac_defl,
		            scb->apsd.ac_trig, scb->apsd.ac_delv);
	}
#endif // endif
	bcm_bprintf(b,  "     WPA_auth 0x%x wsec 0x%x\n", scb->WPA_auth, scb->wsec);

#if defined(STA) && defined(DBG_BCN_LOSS)
	bcm_bprintf(b,	"	  last_rx:%d last_rx_rssi:%d last_bcn_rssi: "
	            "%d last_tx: %d\n",
	            scb->dbg_bcn.last_rx, scb->dbg_bcn.last_rx_rssi, scb->dbg_bcn.last_bcn_rssi,
	            scb->dbg_bcn.last_tx);
#endif // endif

	for (i = 0; i < wlc->scbstate->ncubby; i++) {
		cubby_info = &wlc->scbstate->cubby_info[i];
		cubby_info_ctx = &wlc->scbstate->cubby_info_ctx[i];
		if (cubby_info->fn_dump)
			cubby_info->fn_dump(cubby_info_ctx->context, scb, b);
	}

	wlc_scb_txpath_dump(wlc, scb, b);
}

static void
wlc_scb_bsscfg_dump(void *context, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	int k;
	struct scb *scb;
	struct scb_iter scbiter;
	scb_module_t *scbstate = (scb_module_t *)context;

	bcm_bprintf(b, "# of scbs: %u\n", scbstate->nscb);
	bcm_bprintf(b, "# of cubbies: %u, scb size: %u\n",
	            scbstate->ncubby, scbstate->scbtotsize);

	bcm_bprintf(b, "idx  ether_addr\n");
	k = 0;
	FOREACH_BSS_SCB(scbstate, &scbiter, cfg, scb) {
		wlc_scb_dump_scb(scbstate->wlc, cfg, scb, b, k);
		k++;
	}

	return;
}

static int
wlc_scb_dump(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	int32 idx;
	wlc_bsscfg_t *bsscfg;

	FOREACH_BSS(wlc, idx, bsscfg) {
		wlc_scb_bsscfg_dump(wlc->scbstate, bsscfg, b);
	}

#ifdef SCBFREELIST
	wlc_scbfreelist_dump(wlc->scbstate, b);
#endif /* SCBFREELIST */
	return 0;
}
#endif /* BCMDBG || BCMDBG_DUMP */

int
wlc_scb_save_wpa_ie(wlc_info_t *wlc, struct scb *scb, bcm_tlv_t *ie)
{
	uint ie_len;

	ASSERT(scb != NULL);
	ASSERT(ie != NULL);

	ie_len = TLV_HDR_LEN + ie->len;

	/* Optimization */
	if (scb->wpaie != NULL && ie != NULL &&
	    scb->wpaie_len == ie_len)
		goto cp;

	/* Free old WPA IE if one exists */
	if (scb->wpaie != NULL) {
	        MFREE(wlc->osh, scb->wpaie, scb->wpaie_len);
	        scb->wpaie_len = 0;
	        scb->wpaie = NULL;
	}

	/* Store the WPA IE for later retrieval */
	if ((scb->wpaie = MALLOC(wlc->osh, ie_len)) == NULL) {
		WL_ERROR(("wl%d: %s: unable to allocate memory\n",
		          wlc->pub->unit, __FUNCTION__));
		return BCME_NOMEM;
	}

cp:	/* copy */
	bcopy(ie, scb->wpaie, ie_len);
	scb->wpaie_len = ie_len;

	return BCME_OK;
}

int
wlc_scb_state_upd_register(wlc_info_t *wlc, bcm_notif_client_callback fn, void *arg)
{
	bcm_notif_h hdl = wlc->scbstate->scb_state_notif_hdl;

	return bcm_notif_add_interest(hdl, fn, arg);
}

int
wlc_scb_state_upd_unregister(wlc_info_t *wlc, bcm_notif_client_callback fn, void *arg)
{
	bcm_notif_h hdl = wlc->scbstate->scb_state_notif_hdl;

	return bcm_notif_remove_interest(hdl, fn, arg);
}

#ifdef WL_CS_RESTRICT_RELEASE
void
wlc_scb_restrict_wd(wlc_info_t *wlc)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (scb->restrict_deadline) {
			scb->restrict_deadline--;
		}
		if (!scb->restrict_deadline) {
			scb->restrict_txwin = 0;
		}
	}
}

void
wlc_scb_restrict_start(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		if (!SCB_ISMULTI(scb) && SCB_AMPDU(scb)) {
			scb->restrict_txwin = SCB_RESTRICT_MIN_TXWIN;
			scb->restrict_deadline = SCB_RESTRICT_WD_TIMEOUT + 1;
		}
	}
}
#endif /* WL_CS_RESTRICT_RELEASE */

#ifdef PROP_TXSTATUS
int wlc_scb_wlfc_entry_add(wlc_info_t *wlc, struct scb *scb)
{
	int err = BCME_OK;

	if (wlc == NULL || scb == NULL) {
		err = BCME_BADARG;
		goto end;
	}

	if (!PROP_TXSTATUS_ENAB(wlc->pub)) {
		err = BCME_UNSUPPORTED;
		goto end;
	}

	/* allocate a handle from bitmap f we haven't already done so */
	if (scb->mac_address_handle == 0)
		scb->mac_address_handle = wlfc_allocate_MAC_descriptor_handle(wlc);
	err = wlfc_MAC_table_update(wlc->wl, &scb->ea.octet[0],
		WLFC_CTL_TYPE_MACDESC_ADD, scb->mac_address_handle,
		((SCB_BSSCFG(scb) == NULL) ? 0 : SCB_BSSCFG(scb)->wlcif->index));

	if (err != BCME_OK) {
		WL_ERROR(("wl%d.%d: %s() wlfc_MAC_table_update() failed %d\n",
			WLCWLUNIT(wlc), WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
			__FUNCTION__, err));
	}

end:
	return err;
}
#endif /* PROP_TXSTATUS */

void
wlc_scbfind_delete(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct ether_addr *ea)
{
	int i;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_ASSOC */
	struct scb *scb;

	for (i = 0; i < (int)NBANDS(wlc); i++) {
		/* Use band 1 for single band 11a */
		if (IS_SINGLEBAND_5G(wlc->deviceid))
			i = BAND_5G_INDEX;

		scb = wlc_scbfindband(wlc, bsscfg, ea, i);
		if (scb) {
			WL_ASSOC(("wl%d: %s: scb for the STA-%s"
				" already exists\n", wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(ea, eabuf)));
			wlc_scbfree(wlc, scb);
		}
	}
}
int
wlc_txq_scb_init(void *ctx, struct scb *scb)
{
	/* Init the basic feature tx path to regular tx function */
	wlc_txmod_config((wlc_info_t *)ctx, scb, TXMOD_TRANSMIT);
	return 0;
}

void
wlc_pktq_scb_free(wlc_info_t *wlc, struct pktq *q, struct scb *remove)
{
	int prec;
	void *head_pkt, *pkt;

	PKTQ_PREC_ITER(q, prec) {
		head_pkt = NULL;
		while (pktq_ppeek(q, prec) != head_pkt) {
			pkt = pktq_pdeq(q, prec);
			if (WLPKTTAGSCBGET(pkt) != remove) {
				if (!head_pkt)
					head_pkt = pkt;
				pktq_penq(q, prec, pkt);
			} else
				PKTFREE(wlc->osh, pkt, TRUE);
		}
	}
}

void
wlc_txq_scb_deinit(void *context, struct scb *remove)
{
	wlc_info_t *wlc = (wlc_info_t *) context;
	wlc_txq_info_t *qi;

	for (qi = wlc->tx_queues; qi != NULL; qi = qi->next) {
#ifdef NEW_TXQ
		/* Free the packets for this scb in low txq */
		wlc_low_txq_scb_flush(wlc, qi, remove);
#endif /* NEW_TXQ */
		wlc_pktq_scb_free(wlc, WLC_GET_TXQ(qi), remove);
	}

#ifdef WL_BSSCFG_TX_SUPR
	if (remove->bsscfg != NULL && remove->bsscfg->psq != NULL)
		wlc_pktq_scb_free(wlc, remove->bsscfg->psq, remove);
#endif // endif
}

/* Register the function to handle this feature */
void
BCMATTACHFN(wlc_txmod_fn_register)(wlc_info_t *wlc, scb_txmod_t feature_id, void *ctx,
            txmod_fns_t fns)
{
	ASSERT(feature_id < TXMOD_LAST);
	/* tx_fn can't be NULL */
	ASSERT(fns.tx_fn != NULL && fns.pktcnt_fn != NULL);
	wlc->txmod_fns[feature_id].tx_fn = fns.tx_fn;
	wlc->txmod_fns[feature_id].deactivate_notify_fn = fns.deactivate_notify_fn;
	wlc->txmod_fns[feature_id].activate_notify_fn = fns.activate_notify_fn;
	wlc->txmod_fns[feature_id].pktcnt_fn = fns.pktcnt_fn;
	wlc->txmod_fns[feature_id].ctx = ctx;
}

/* Add the fid to handle packets for this SCB, if allowed */
void
wlc_txmod_config(wlc_info_t *wlc, struct scb *scb, scb_txmod_t fid)
{
	ASSERT(fid < TXMOD_LAST);

	/* Don't do anything if not yet registered or
	 * already configured
	 */
	if ((wlc->txmod_fns[fid].tx_fn == NULL) ||
	    (SCB_TXMOD_CONFIGURED(scb, fid)))
		return;

	/* Indicate that the feature is configured */
	scb->tx_path[fid].configured = TRUE;

	ASSERT(!SCB_TXMOD_ACTIVE(scb, fid));
	/* Try to activate this feature by adding it to the path
	 * If conflicting features exist in the path, then
	 *     - If this feature is of higher precedence, deactivate other features
	 *     - Else, mark this feature deactivated
	 * Note: When evicting other features, they remain 'configured'
	 */
	switch (fid) {
	case TXMOD_APPS:
	case TXMOD_AMSDU:
	case TXMOD_NAR:
	case TXMOD_AMPDU:
	case TXMOD_TDLS:
	case TXMOD_TRF_MGMT:
	case TXMOD_TRANSMIT:
#if defined(TXQ_MUX)
	case TXMOD_SCBQ:
#endif /* TXQ_MUX */
		break;

	case TXMOD_START:
	case TXMOD_LAST:
		ASSERT(0);
	};

	wlc_scb_txmod_activate(wlc, scb, fid);
}

/* Remove the feature to handle packets for this SCB.
 * If just configured but not in path, just marked unconfigured
 * If in path, feature is removed and, if applicable, replaced by any other feature
 */
void
wlc_txmod_unconfig(wlc_info_t *wlc, struct scb *scb, scb_txmod_t fid)
{
	ASSERT(fid < TXMOD_LAST && fid != TXMOD_TRANSMIT);

	if (!SCB_TXMOD_CONFIGURED(scb, fid))
		return;

	scb->tx_path[fid].configured = FALSE;

	/* Nothing to do if not active */
	if (!SCB_TXMOD_ACTIVE(scb, fid))
		return;

	wlc_scb_txmod_deactivate(wlc, scb, fid);

	/* Restore any other features to the path */
	switch (fid) {
	case TXMOD_APPS:
	case TXMOD_TDLS:
	case TXMOD_TRF_MGMT:
	case TXMOD_AMSDU:
	case TXMOD_NAR:
	case TXMOD_AMPDU:
	case TXMOD_TRANSMIT:
#if defined(TXQ_MUX)
	case TXMOD_SCBQ:
#endif /* TXQ_MUX */
	case TXMOD_START:
	case TXMOD_LAST:
		break;
	}
}
