/*
 * SW probe response module source file
 * disable ucode sending probe response,
 * driver will decide whether send probe response,
 * after check the received probe request.
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
 * $Id: wlc_probresp.c 732650 2017-11-20 10:14:47Z $
 */

#include <wlc_cfg.h>

#ifdef WLPROBRESP_SW

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_tpc.h>
#include <wlc_csa.h>
#include <wlc_quiet.h>
#include <wlc_bmac.h>
#include <wlc_probresp.h>
#include <wlc_probresp_mac_filter.h>
#include <wlc_pcb.h>

#define WLC_PROBRESP_MAXFILTERS		3	/* max filter functions */
#define WLC_PROBRESP_INVALID_INDEX	-1

typedef	struct probreqcb {
	void *hdl;
	probreq_filter_fn_t filter_fn;
} probreqcb_t;

/* IOVar table */
/* No ordering is imposed */
enum {
	IOV_PROBRESP_SW, /* SW probe response enable/dsiable */
	IOV_LAST
};

static const bcm_iovar_t wlc_probresp_iovars[] = {
	{"probresp_sw", IOV_PROBRESP_SW, (0), IOVT_BOOL, 0},
	{NULL, 0, 0, 0, 0}
};

/* SW probe response module info */
struct wlc_probresp_info {
	wlc_info_t *wlc;
	probreqcb_t probreq_filters[WLC_PROBRESP_MAXFILTERS];
	int p2p_index;
};

/* local functions */
/* module */
static int wlc_probresp_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
#ifdef WLPROBRESP_INTRANSIT_FILTER
static void wlc_proberesp_cb(wlc_info_t *wlc, void *pkt, uint txs);
#endif /* WLPROBRESP_INTRANSIT_FILTER */

wlc_probresp_info_t *
BCMATTACHFN(wlc_probresp_attach)(wlc_info_t *wlc)
{
	wlc_probresp_info_t *mprobresp;

	if (!wlc)
		return NULL;

	mprobresp = MALLOCZ(wlc->osh, sizeof(wlc_probresp_info_t));
	if (mprobresp == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	mprobresp->wlc = wlc;
	mprobresp->p2p_index = WLC_PROBRESP_INVALID_INDEX;

	/* keep the module registration the last other add module unregistration
	 * in the error handling code below...
	 */
	if (wlc_module_register(wlc->pub, wlc_probresp_iovars, "probresp", mprobresp,
		wlc_probresp_doiovar, NULL, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

#ifdef WLPROBRESP_INTRANSIT_FILTER
	if (wlc_pcb_fn_set(wlc->pcb, 0, WLF2_PCB1_PROBRESP, wlc_proberesp_cb) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_pcb_fn_set() failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* WLPROBRESP_INTRANSIT_FILTER */
	return mprobresp;

	/* error handling */
fail:
	wlc_probresp_detach(mprobresp);

	return NULL;
}

void
BCMATTACHFN(wlc_probresp_detach)(wlc_probresp_info_t *mprobresp)
{
	wlc_info_t *wlc;

	if (!mprobresp)
		return;
	wlc = mprobresp->wlc;
	wlc_module_unregister(wlc->pub, "probresp", mprobresp);
#ifdef WLPROBRESP_INTRANSIT_FILTER
	wlc_pcb_fn_set(wlc->pcb, 0, WLF2_PCB1_PROBRESP, NULL);
#endif /* WLPROBRESP_INTRANSIT_FILTER */
	MFREE(wlc->osh, mprobresp, sizeof(wlc_probresp_info_t));
}

static int
wlc_probresp_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_probresp_info_t *mprobresp = (wlc_probresp_info_t *)ctx;
	wlc_info_t *wlc = mprobresp->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
	int32 *ret_int_ptr;
	bool bool_val;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0);

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	/* Do the actual parameter implementation */
	switch (actionid) {
	case IOV_GVAL(IOV_PROBRESP_SW):
		*ret_int_ptr = (int32)wlc->pub->_probresp_sw;
		break;
	case IOV_SVAL(IOV_PROBRESP_SW):
		if (wlc->pub->_probresp_sw != bool_val) {
			wlc->pub->_probresp_sw = bool_val;

			wlc_enable_probe_req(wlc, PROBE_REQ_PROBRESP_MASK,
				bool_val ? PROBE_REQ_PROBRESP_MASK : 0);

			wlc_disable_probe_resp(wlc, PROBE_RESP_SW_MASK,
				bool_val ? PROBE_RESP_SW_MASK : 0);
		}
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static void
wlc_probresp_send_probe_resp(wlc_probresp_info_t *mprobresp, wlc_bsscfg_t *bsscfg,
	struct ether_addr *da)
{
	wlc_info_t *wlc;
	void *p;
	uint8 *pbody;
	int len;

	wlc = mprobresp->wlc;

	ASSERT(bsscfg != NULL);

	ASSERT(wlc->pub->up);

	len = wlc->pub->bcn_tmpl_len;

#ifdef BCMPCIEDEV
	 /* for full dongle operation, do not queue Probe response packets
	  * while channel switch is in progress
	  */
	if (BCMPCIEDEV_ENAB() && (wlc_bmac_tx_fifo_suspended(wlc->hw, TX_CTL_FIFO)))
		return;
#endif // endif
	/* build response and send */
	if ((p = wlc_frame_get_mgmt(wlc, FC_PROBE_RESP, da, &bsscfg->cur_etheraddr,
	                            &bsscfg->BSSID, len, &pbody)) == NULL) {
		WL_ERROR(("wl%d.%d: %s: wlc_frame_get_mgmt failed\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
	}
	else {
#ifdef WLPROBRESP_INTRANSIT_FILTER
		if (!wlc_probresp_intransit_filter_add_da(wlc->mprobresp_mac_filter, bsscfg, da)) {
			WL_ERROR(("wl%d.%d: %s: no mem for intransit filter!\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
		}
		WLF2_PCB1_REG(p, WLF2_PCB1_PROBRESP);
#endif /* WLPROBRESP_INTRANSIT_FILTER */
		/* Generate probe response body */
		wlc_bcn_prb_body(wlc, FC_PROBE_RESP, bsscfg, pbody, &len, FALSE);
		PKTSETLEN(wlc->osh, p, len + DOT11_MGMT_HDR_LEN);
#ifdef BCMPCIEDEV
		/* For full dongle operation, do not queue probe response packets into psq */
		if (BCMPCIEDEV_ENAB())
			WLPKTTAG(p)->flags |= WLF_PSDONTQ;
#endif /* BCMPCIEDEV */

#ifdef WLPROBRESP_INTRANSIT_FILTER
		wlc_queue_80211_frag(wlc, p, bsscfg->wlcif->qi, NULL, bsscfg, FALSE, NULL, 0);
#else
		wlc_sendmgmt(wlc, p, bsscfg->wlcif->qi, NULL);
#endif /* WLPROBRESP_INTRANSIT_FILTER */
	}
}

static bool
wlc_probresp_filter_probe_request(wlc_probresp_info_t *mprobresp, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_management_header *hdr,
	uint8 *body, int body_len)
{
	int i;
	bool sendProbeResp = TRUE;

	/* if network type is closed then don't send prob response */
	if (cfg->closednet_nobcprbresp)	{
		bcm_tlv_t *ssid;
		/* If it is a directed probe request, we need to send out the response */
		ssid = bcm_parse_tlvs(body, body_len, DOT11_MNG_SSID_ID);
		if (ssid == NULL)
			return FALSE;
		/* Check if the ssid in the probe request matches our bsscfg->SSID */
		if ((ssid->len == 0) || (ssid->len != cfg->SSID_len) ||
			(bcmp(ssid->data, cfg->SSID, ssid->len) != 0)) {
			if ((eacmp(&hdr->da, &cfg->cur_etheraddr) != 0) &&
				((eacmp(&hdr->bssid, &cfg->BSSID)) != 0))
				return FALSE;
		}
	}

	for (i = 0; i < WLC_PROBRESP_MAXFILTERS; i ++) {
		if ((i != mprobresp->p2p_index) && mprobresp->probreq_filters[i].hdl) {
			if (!(mprobresp->probreq_filters[i].filter_fn(
				mprobresp->probreq_filters[i].hdl, cfg,
				wrxh, plcp, hdr, body, body_len, NULL)))
				sendProbeResp = FALSE;
		}
	}

	/* call p2p filter function last */
	if (mprobresp->p2p_index != WLC_PROBRESP_INVALID_INDEX) {
		sendProbeResp = mprobresp->probreq_filters[mprobresp->p2p_index].filter_fn(
			mprobresp->probreq_filters[mprobresp->p2p_index].hdl, cfg,
			wrxh, plcp, hdr, body, body_len, &sendProbeResp);
	}

	if (sendProbeResp) {
		wlc_probresp_send_probe_resp(mprobresp, cfg, &hdr->sa);
	}

	return sendProbeResp;
}

#ifdef WLPROBRESP_INTRANSIT_FILTER
/* Called on free of a probe response packet */
static void
wlc_proberesp_cb(wlc_info_t *wlc, void *p, uint txs)
{
	struct dot11_management_header *hdr;
	wlc_bsscfg_t *bsscfg;
	int err = 0;

	/* get the bsscfg from the pkttag */
	bsscfg = wlc_bsscfg_find(wlc, WLPKTTAGBSSCFGGET(p), &err);

	hdr = (struct dot11_management_header*)wlc_pkt_get_d11_hdr(wlc, p);

	/* in case bsscfg is freed before this callback is invoked */
	if (bsscfg == NULL) {
		return;
	}

	wlc_probresp_intransit_filter_rem_da(wlc->mprobresp_mac_filter, bsscfg, &hdr->da);
}
#endif /* WLPROBRESP_INTRANSIT_FILTER */

/* register filter function */
int
BCMATTACHFN(wlc_probresp_register)(wlc_probresp_info_t *mprobresp, void *hdl,
	probreq_filter_fn_t filter_fn, bool p2p)
{
	int i;
	if (!mprobresp || !hdl || !filter_fn)
		return BCME_BADARG;

	/* find an empty entry and just add, no duplication check! */
	for (i = 0; i < WLC_PROBRESP_MAXFILTERS; i ++) {
		if (!mprobresp->probreq_filters[i].hdl) {
			mprobresp->probreq_filters[i].hdl = hdl;
			mprobresp->probreq_filters[i].filter_fn = filter_fn;
			if (p2p)
				mprobresp->p2p_index = i;
			return BCME_OK;
		}
	}

	/* it is time to increase the capacity */
	ASSERT(i < WLC_PROBRESP_MAXFILTERS);
	return BCME_NORESOURCE;
}
/* register filter function */
int
BCMATTACHFN(wlc_probresp_unregister)(wlc_probresp_info_t *mprobresp, void *hdl)
{
	int i;
	if (!mprobresp || !hdl)
		return BCME_BADARG;

	for (i = 0; i < WLC_PROBRESP_MAXFILTERS; i ++) {
		if (mprobresp->probreq_filters[i].hdl == hdl) {
			bzero(&mprobresp->probreq_filters[i], sizeof(probreqcb_t));
			return BCME_OK;
		}
	}

	/* table not found! */
	return BCME_NOTFOUND;
}

/* process probe request frame */
void
wlc_probresp_recv_process_prbreq(wlc_probresp_info_t *mprobresp, wlc_d11rxhdr_t *wrxh,
	uint8 *plcp, struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg;
	wlc_bsscfg_t *bsscfg_hwaddr;
	bcm_tlv_t *ssid;

	if (!mprobresp)
		return;

	wlc = mprobresp->wlc;

	if (!WLPROBRESP_SW_ENAB(wlc))
		return;

	if ((ssid = bcm_parse_tlvs(body, body_len, DOT11_MNG_SSID_ID)) != NULL) {
		wlc_bsscfg_t *cfg;
		int idx;
		bsscfg_hwaddr = wlc_bsscfg_find_by_hwaddr(wlc, &hdr->da);
		bsscfg = wlc_bsscfg_find_by_bssid(wlc, &hdr->bssid);
		if (bsscfg || bsscfg_hwaddr) {
			cfg = bsscfg ? bsscfg : bsscfg_hwaddr;
			if (BSSCFG_AP(cfg) && cfg->up && cfg->enable &&
			            ((bsscfg == bsscfg_hwaddr) ||
			            ETHER_ISBCAST(&hdr->da) ||
			            ETHER_ISBCAST(&hdr->bssid)) &&
			            ((ssid->len == 0) ||
			            ((ssid->len == cfg->SSID_len) &&
			            (bcmp(ssid->data, cfg->SSID, ssid->len) == 0)))) {
				wlc_probresp_filter_probe_request(mprobresp, cfg,
					wrxh, plcp, hdr, body, body_len);
			}
		} else if (ETHER_ISBCAST(&hdr->da) && ETHER_ISBCAST(&hdr->bssid)) {
			FOREACH_UP_AP(wlc, idx, cfg) {
				if (cfg->enable && ((ssid->len == 0) ||
				            ((ssid->len == cfg->SSID_len) &&
				            (bcmp(ssid->data, cfg->SSID,
				            ssid->len) == 0)))) {
					wlc_probresp_filter_probe_request(mprobresp, cfg,
						wrxh, plcp, hdr, body, body_len);
				}
			}
		}
	}
}

#endif /* WLPROBRESP_SW */
