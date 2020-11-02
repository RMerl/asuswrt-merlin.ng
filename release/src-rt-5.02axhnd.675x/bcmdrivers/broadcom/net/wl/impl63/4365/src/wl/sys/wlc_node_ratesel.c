/*
 * Net80211 rate selection algorithm wrapper of Broadcom
 * 802.11 Networking Adapter Device Driver.
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
 * $Id: wlc_node_ratesel.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlRateSelection]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/socket.h>
#include <sys/mbuf.h>

#include <net/if.h>
#include <net/if_media.h>
#ifdef __NetBSD__
#include <net/if_ether.h>
/* NB: disable Broadcom proto/ethernet.h */
#define _NET_ETHERNET_H_
#define	ETHER_ISMULTI(a)	ETHER_IS_MULTICAST((const uint8_t *)(a))
#define	M_LASTFRAG	0
#endif // endif

#include <net80211/ieee80211_var.h>
#include <net80211/ieee80211_regdomain.h>

#undef WME_OUI
#undef WPA_OUI
#undef RSN_CAP_PREAUTH
#undef WME_OUI_TYPE
#undef WPA_OUI_TYPE
#undef WPS_OUI_TYPE

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>

#include <proto/802.11.h>
#include <d11.h>

#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_node.h>

#include <wlc_phy_hal.h>
#include <wlc_antsel.h>
#include <wlc_node_ratesel.h>

#include <wl_dbg.h>
#include <wlc_net80211.h>
#include <wlc_types.h>

struct wlc_ratesel_info {
	wlc_info_t	*wlc;		/* pointer to main wlc structure */
	wlc_pub_t	*pub;		/* public common code handler */
	ratesel_info_t *rsi;
	int32 scb_handle;
	int32 cubby_sz;
};

typedef struct ratesel_cubby ratesel_cubby_t;

#if defined(WME_PER_AC_TX_PARAMS)
#define NODE_RATESEL_CUBBY(wrsi, node_ratesel, ac) 	\
	((void *)(((char*)((rcb_t *)node_ratesel)) + (ac * (wrsi)->cubby_sz)))
#define FID2AC(pub, fid) \
	(WME_PER_AC_MAXRATE_ENAB(pub) ? wme_fifo2ac[WLC_TXFID_GET_QUEUE(fid)] : 0)
#else /* WME_PER_AC_TX_PARAMS */
#define NODE_RATESEL_CUBBY(wrsi, node_ratesel, ac)	\
	(((rcb_t *)node_ratesel))
#define FID2AC(pub, fid) (0)
#endif /* WME_PER_AC_TX_PARAMS */

static rcb_t *wlc_node_ratesel_get_cubby(wlc_ratesel_info_t *wrsi, void *node_ratesel,
	uint16 frameid);
#ifdef WL11N
void wlc_node_ratesel_rssi_enable(rssi_ctx_t *ctx);
void wlc_node_ratesel_rssi_disable(rssi_ctx_t *ctx);
int wlc_node_ratesel_get_rssi(rssi_ctx_t *ctx);
#endif // endif

wlc_ratesel_info_t *
BCMATTACHFN(wlc_node_ratesel_attach)(wlc_info_t *wlc)
{
	wlc_ratesel_info_t *wrsi;

	if (!(wrsi = (wlc_ratesel_info_t *)MALLOC(wlc->osh, sizeof(wlc_ratesel_info_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	bzero((char *)wrsi, sizeof(wlc_ratesel_info_t));
	wrsi->wlc = wlc;
	wrsi->pub = wlc->pub;

	if ((wrsi->rsi = wlc_ratesel_attach(wlc)) == NULL) {
		WL_ERROR(("%s: failed\n", __FUNCTION__));
		goto fail;
	}

	/* reserve cubby in the scb container for per-scb-ac private data */
	wrsi->scb_handle = 0;

	wrsi->cubby_sz = wlc_ratesel_rcb_sz();

#ifdef WL11N
	wlc_ratesel_rssi_attach(wrsi->rsi, wlc_node_ratesel_rssi_enable,
		wlc_node_ratesel_rssi_disable, wlc_node_ratesel_get_rssi);
#endif // endif

	return wrsi;

fail:
	if (wrsi->rsi)
		wlc_ratesel_detach(wrsi->rsi);

	MFREE(wlc->osh, wrsi, sizeof(wlc_ratesel_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_node_ratesel_detach)(wlc_ratesel_info_t *wrsi)
{
	if (!wrsi)
		return;

	wlc_ratesel_detach(wrsi->rsi);

	MFREE(wrsi->pub->osh, wrsi, sizeof(wlc_ratesel_info_t));
}

static rcb_t *
wlc_node_ratesel_get_cubby(wlc_ratesel_info_t *wrsi, void *node_ratesel, uint16 frameid)
{
	int ac;

	ASSERT(wrsi);

	ac = FID2AC(wrsi->pub, frameid);
	ASSERT(ac < AC_COUNT);
	return (NODE_RATESEL_CUBBY(wrsi, node_ratesel, ac));
}

/* select transmit rate given per-scb state */
void BCMFASTPATH
wlc_node_ratesel_gettxrate(wlc_ratesel_info_t *wrsi, void *node_ratesel, uint16 *frameid,
	ratesel_txparams_t *cur_rate, uint16 *flags)
{
	rcb_t *state;

	state = wlc_node_ratesel_get_cubby(wrsi, node_ratesel, *frameid);
	if (state == NULL) {
		ASSERT(0);
		return;
	}

	wlc_ratesel_gettxrate(state, frameid, cur_rate, flags);
}

#ifdef WL11N
void
wlc_node_ratesel_probe_ready(wlc_ratesel_info_t *wrsi, void *node_ratesel, uint16 frameid,
	bool is_ampdu, uint8 ampdu_txretry)
{
	rcb_t *state;

	state = wlc_node_ratesel_get_cubby(wrsi, node_ratesel, frameid);
	if (state == NULL) {
		ASSERT(0);
		return;
	}
	wlc_ratesel_probe_ready(state, frameid, is_ampdu, ampdu_txretry);
}

void
wlc_node_ratesel_upd_rxstats(wlc_ratesel_info_t *wrsi, ratespec_t rx_rspec, uint16 rxstatus2)
{
	wlc_ratesel_upd_rxstats(wrsi->rsi, rx_rspec, rxstatus2);
}
#endif /* WL11N */

/* non-AMPDU txstatus rate update, default to use non-mcs rates only */
void
wlc_node_ratesel_upd_txstatus_normalack(wlc_ratesel_info_t *wrsi, void *node_ratesel,
	tx_status_t *txs, uint16 sfbl, uint16 lfbl, uint8 tx_mcs, uint8 antselid, bool fbr)

{
	rcb_t *state;

	state = wlc_node_ratesel_get_cubby(wrsi, node_ratesel, txs->frameid);
	if (state == NULL) {
		ASSERT(0);
		return;
	}

	wlc_ratesel_upd_txstatus_normalack(state, txs, sfbl, lfbl, tx_mcs, antselid, fbr);
}

#ifdef WL11N
void
wlc_node_ratesel_aci_change(wlc_ratesel_info_t *wrsi, bool aci_state)
{
	wlc_ratesel_aci_change(wrsi->rsi, aci_state);
}

/*
 * Return the fallback rate of the specified mcs rate.
 * Ensure that is a mcs rate too.
 */
ratespec_t
wlc_node_ratesel_getmcsfbr(wlc_ratesel_info_t *wrsi, void *node_ratesel, uint16 frameid, uint8 mcs)
{
	rcb_t *state;

	state = wlc_node_ratesel_get_cubby(wrsi, node_ratesel, frameid);
	ASSERT(state);

	return (wlc_ratesel_getmcsfbr(state, frameid, mcs));
}

#ifdef WLAMPDU_MAC
/*
 * The case that (mrt+fbr) == 0 is handled as RTS transmission failure.
 */
void
wlc_node_ratesel_upd_txs_ampdu(wlc_ratesel_info_t *wrsi, struct scb *scb, uint16 frameid,
	uint8 mrt, uint8 mrt_succ, uint8 fbr, uint8 fbr_succ,
	bool tx_error, uint8 tx_mcs, uint8 antselid)
{
	rcb_t *state;

	state = wlc_node_ratesel_get_cubby(wrsi, node_ratesel, frameid);
	ASSERT(state);

	wlc_ratesel_upd_txs_ampdu(state, frameid, mrt, mrt_succ, fbr, fbr_succ, tx_error,
		tx_mcs, antselid);
}
#endif /* WLAMPDU_MAC */

/* update state upon received BA */
void BCMFASTPATH
wlc_node_ratesel_upd_txs_blockack(wlc_ratesel_info_t *wrsi, void *node_ratesel, tx_status_t *txs,
	uint8 suc_mpdu, uint8 tot_mpdu, bool ba_lost, uint8 retry, uint8 fb_lim, bool tx_error,
	uint8 mcs, uint8 antselid)
{
	rcb_t *state;

	state = wlc_node_ratesel_get_cubby(wrsi, node_ratesel, txs->frameid);
	ASSERT(state);

	wlc_ratesel_upd_txs_blockack(state, txs, suc_mpdu, tot_mpdu, ba_lost, retry, fb_lim,
		tx_error, mcs, antselid);
}
#endif /* WL11N */

bool
wlc_node_ratesel_minrate(wlc_ratesel_info_t *wrsi, void *node_ratesel, tx_status_t *txs)
{
	rcb_t *state;

	state = wlc_node_ratesel_get_cubby(wrsi, node_ratesel, txs->frameid);
	ASSERT(state);

	return (wlc_ratesel_minrate(state, txs));
}

void
wlc_ratesel_init_node(wlc_info_t *wlc, struct ieee80211_node *ni, void *ratesel_info,
	wlc_rateset_t *rateset)
{
	wlc_ratesel_info_t *wrsi = wlc->wrsi;
	uint8 bw = BW_20MHZ;
	int8 sgi_tx = OFF;
	int8 ldpc_tx = OFF;
	int8 vht_ldpc_tx = OFF;
	uint8 active_antcfg_num = 0;
	uint8 antselid_init = 0;
	int32 ac, n_ac;
	uint *txc_ptr;
	wlc_rateset_t new_rateset;
	struct wl_node *wn = WL_NODE(ni);
	bool singlestream = FALSE;

	wlc_antsel_ratesel(wlc->asi, &active_antcfg_num, &antselid_init);

	bw = (ni->ni_flags & IEEE80211_NODE_HT) && (ni->ni_chw == 40);

	if (wlc->stf->ldpc_tx == AUTO) {
		if ((bw != BW_80MHZ && (ni->ni_flags & IEEE80211_NODE_LDPC)) ||
			(bw == BW_80MHZ && (ni->ni_flags & IEEE80211_NODE_LDPC)))
			/* XXX: Revisit the ldpc decesion for bw 80MHz as it
			 * requires VHT check.
			 */
			ldpc_tx = AUTO;
	}

	if (wlc->sgi_tx == AUTO) {
		if ((bw == BW_40MHZ) && (ni->ni_flags & IEEE80211_NODE_SGI40))
			sgi_tx = AUTO;
		if ((bw == BW_20MHZ) && (ni->ni_flags & IEEE80211_NODE_SGI20))
			sgi_tx = AUTO;

		/* Disable SGI Tx in 20MHz on IPA chips */
		if (bw == BW_20MHZ && wlc->stf->ipaon)
			sgi_tx = OFF;
	}

	singlestream = N_ENAB(wlc->pub) && (((ni->ni_flags & (IEEE80211_NODE_MIMO_PS |
		IEEE80211_NODE_MIMO_RTS)) == IEEE80211_NODE_MIMO_PS) ||
		(wlc->stf->op_txstreams == 1));

#if defined(WME_PER_AC_TX_PARAMS)
	n_ac = AC_COUNT;
#else
	n_ac = 1;
#endif /* defined(WME_PER_AC_TX_PARAMS) */

	txc_ptr = wlc_node_txcptr_to_invalidate_txc(wlc, wn);

	for (ac = 0; ac < n_ac; ac++) {

		bzero((char *)&new_rateset, sizeof(wlc_rateset_t));

		/* Rates above per AC max rate, below per AC min are removed from the rateset */
		if (WME_PER_AC_MAXRATE_ENAB(wrsi->pub) &&
			((ni->ni_flags & IEEE80211_NODE_QOS) != 0))
			wlc_ratesel_filter_rateset(wrsi->rsi, rateset, &new_rateset, bw,
				wrsi->wlc->wme_max_rate[ac], 0);
		else
			bcopy(rateset, &new_rateset, sizeof(wlc_rateset_t));

#ifdef WL11N
		if (N_ENAB(wlc->pub)) {
			if ((singlestream ||
				(wlc->stf->op_txstreams == 1) || (wlc->stf->siso_tx == 1))) {
				new_rateset.mcs[1] = 0;
				new_rateset.mcs[2] = 0;
			} else if (wlc->stf->op_txstreams == 2)
				new_rateset.mcs[2] = 0;
		}
#endif // endif
		WL_RATE(("%s: scb 0x%p ac %d state 0x%p bw %d\n", __FUNCTION__,
			wn, ac, ratesel_info, bw));

		wlc_ratesel_init(wrsi->rsi, ratesel_info, wn, txc_ptr, &new_rateset, bw, sgi_tx,
			ldpc_tx, vht_ldpc_tx, active_antcfg_num, antselid_init);
	}
}

void
wlc_scb_ratesel_init_all(wlc_info_t *wlc)
{
	/* do nothing */
}

#ifdef WL11N
void wlc_node_ratesel_rssi_enable(rssi_ctx_t *ctx)
{
	/* do nothing */
}

void wlc_node_ratesel_rssi_disable(rssi_ctx_t *ctx)
{
	/* do nothing */
}

int wlc_node_ratesel_get_rssi(rssi_ctx_t *ctx)
{
	struct wl_node *wn = (struct wl_node *)ctx;
	return (bwl_get_antavgrssi(wn, 0));
}
#endif /* WL11N */

void wlc_ratesel_node_free(wlc_info_t *wlc, void *node_ratesel)
{
	wlc_ratesel_info_t *wrsi = wlc->wrsi;
	int32 rcb_size;

#if defined(WME_PER_AC_TX_PARAMS)
	rcb_size = AC_COUNT * wrsi->cubby_sz;
#else
	rcb_size = wrsi->cubby_sz;
#endif // endif

	if (!node_ratesel)
		return;

	MFREE(wlc->osh, node_ratesel, rcb_size);
}

void *wlc_ratesel_node_alloc(wlc_info_t *wlc)
{
	wlc_ratesel_info_t *wrsi = wlc->wrsi;
	void *node_ratesel;
	int32 rcb_size;

#if defined(WME_PER_AC_TX_PARAMS)
	rcb_size = AC_COUNT * wrsi->cubby_sz;
#else
	rcb_size = wrsi->cubby_sz;
#endif // endif

	node_ratesel = MALLOC(wlc->osh, rcb_size);

	return node_ratesel;
}
