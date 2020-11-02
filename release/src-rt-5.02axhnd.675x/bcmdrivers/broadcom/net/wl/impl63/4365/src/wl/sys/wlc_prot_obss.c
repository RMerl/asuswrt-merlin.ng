/*
 * OBSS Protection support
 * Broadcom 802.11 Networking Device Driver
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
 * $Id$
 */

/**
 * @file
 * @brief
 * Out of Band BSS
 */

/**
 * @file
 * @brief
 * XXX Twiki: [ObssBw]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <bcmwifi_channels.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scan.h>
#ifdef APCS
#include <wlc_apcs.h>
#endif // endif
#include <wlc_rm.h>
#include <wlc_ap.h>
#include <wlc_scb.h>
#include <wlc_txc.h>
#include <wlc_frmutil.h>
#include <wl_export.h>
#include <wlc_assoc.h>
#include <wlc_bmac.h>
#ifdef WLOFFLD
#include <wlc_offloads.h>
#endif // endif
#include "wlc_vht.h"

#include <wlc_txc.h>
#include <wlc_prot_obss.h>
#include <wlc_lq.h>
#include <wlc_obss_util.h>

typedef struct {
	uint32 obss_inactivity_period;	/* quiet time prior to deactivating OBSS protection */
	uint8 obss_dur_thres;		/* OBSS protection trigger/RX CRS Sec */
	int8 obss_sec_rssi_lim0;	/* OBSS secondary RSSI limit 0 */
	int8 obss_sec_rssi_lim1;	/* OBSS secondary RSSI limit 1 */
} wlc_prot_obss_config_t;

/* module private states */
typedef struct {
	wlc_info_t *wlc;	/* pointer to main wlc structure */
	wlc_prot_obss_config_t *config;
	/* ucode previous and current stat counters */
	wlc_bmac_obss_counts_t *prev_stats;
	wlc_bmac_obss_counts_t *curr_stats;
	/* Cummulative stat counters */
	wlc_bmac_obss_counts_t *total_stats;
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* For diagnostic measurements */
	wlc_bmac_obss_counts_t *msrmnt_stored;
	cca_stats_n_flags *results;
	struct wl_timer *stats_timer;
#endif // endif
	uint16 obss_inactivity;	/* # of secs of OBSS inactivity */
	int8 mode;
} wlc_prot_obss_info_priv_t;

/*
 * module states layout
 */
typedef struct {
	wlc_prot_obss_info_t pub;
	wlc_prot_obss_info_priv_t priv;
	wlc_prot_obss_config_t config;	/* OBSS protection configuration */
	/* ucode previous and current stat counters */
	wlc_bmac_obss_counts_t prev_stats;
	wlc_bmac_obss_counts_t curr_stats;
	/* Cummulative stat counters */
	wlc_bmac_obss_counts_t total_stats;
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* For diagnostic measurements */
	wlc_bmac_obss_counts_t msrmnt_stored;
	cca_stats_n_flags results;
#endif // endif
} wlc_prot_obss_t;

static uint16 wlc_prot_obss_info_priv_offset = OFFSETOF(wlc_prot_obss_t, priv);

#define WLC_PROT_OBSS_SIZE (sizeof(wlc_prot_obss_t))
#define WLC_PROT_OBSS_INFO_PRIV(prot) ((wlc_prot_obss_info_priv_t *) \
		((uintptr)(prot) + wlc_prot_obss_info_priv_offset))

static const bcm_iovar_t wlc_prot_obss_iovars[] = {
	{"obss_prot", IOV_OBSS_PROT,
	(0), IOVT_BUFFER, sizeof(wl_config_t),
	},
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	{"ccastats", IOV_CCASTATS,
	(IOVF_GET_UP), IOVT_BUFFER, sizeof(cca_stats_n_flags),
	},
#endif // endif
	{NULL, 0, 0, 0, 0}
};

static bool wlc_prot_obss_secondary_interference_detected(wlc_prot_obss_info_t *prot,
	uint8 obss_dur_threshold);

static void
wlc_prot_obss_detection(wlc_prot_obss_info_t *prot, chanspec_t chanspec);

static int
wlc_prot_obss_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *p, uint plen,
	void *a, int alen, int val_size, struct wlc_if *wlcif);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_prot_obss_dump(wlc_prot_obss_info_t *prot, void *input, int buf_len, void *output);

static void
wlc_prot_obss_stats_sample(wlc_prot_obss_info_t *prot, int duration);

static void
wlc_prot_obss_stats_timeout(void *arg);
#endif // endif

static void
wlc_prot_obss_enable(wlc_prot_obss_info_t *prot, bool enable);

static int
wlc_prot_obss_init(void *cntxt);

static void
wlc_prot_obss_watchdog(void *cntxt);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
wlc_prot_obss_stats_sample(wlc_prot_obss_info_t *prot, int duration)
{
	wlc_prot_obss_info_priv_t *priv = WLC_PROT_OBSS_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;

	if (duration) {
		/* Store results of first read */
		wlc_bmac_obss_stats_read(wlc->hw, priv->msrmnt_stored);
		wl_add_timer(wlc->wl, priv->stats_timer, duration, 0);
		priv->results->msrmnt_time = duration;
	} else {
		wlc_bmac_obss_counts_t delta, now;
		wlc_bmac_obss_counts_t *curr = &now;
		wlc_bmac_obss_counts_t *prev = priv->msrmnt_stored;

		/* Read current values and compute the delta. */
		wlc_bmac_obss_stats_read(wlc->hw, curr);

		memset(&delta, '\0', sizeof(delta));

		delta.usecs = curr->usecs - prev->usecs;
		delta.txdur = curr->txdur - prev->txdur;
		delta.ibss = curr->ibss - prev->ibss;
		delta.obss = curr->obss - prev->obss;
		delta.noctg = curr->noctg - prev->noctg;
		delta.nopkt = curr->nopkt - prev->nopkt;
		delta.PM = curr->PM - prev->PM;
		delta.txopp = curr->txopp - prev->txopp;
		delta.gdtxdur = curr->gdtxdur - prev->gdtxdur;
		delta.bdtxdur = curr->bdtxdur - prev->bdtxdur;
		delta.slot_time_txop = curr->slot_time_txop;
#ifdef ISID_STATS
		delta.crsglitch = curr->crsglitch - prev->crsglitch;
		delta.badplcp = curr->badplcp - prev->badplcp;
		delta.bphy_crsglitch = curr->bphy_crsglitch - prev->bphy_crsglitch;
		delta.bphy_badplcp = curr->bphy_badplcp - prev->bphy_badplcp;
#endif // endif
		delta.rxdrop20s = curr->rxdrop20s - prev->rxdrop20s;
		delta.rx20s = curr->rx20s - prev->rx20s;
		delta.rxcrs_pri = curr->rxcrs_pri - prev->rxcrs_pri;
		delta.rxcrs_sec20 = curr->rxcrs_sec20 - prev->rxcrs_sec20;
		delta.rxcrs_sec40 = curr->rxcrs_sec40 - prev->rxcrs_sec40;
		delta.sec_rssi_hist_hi = curr->sec_rssi_hist_hi - prev->sec_rssi_hist_hi;
		delta.sec_rssi_hist_med = curr->sec_rssi_hist_med - prev->sec_rssi_hist_med;
		delta.sec_rssi_hist_low = curr->sec_rssi_hist_low - prev->sec_rssi_hist_low;

		delta.suspend = curr->suspend - prev->suspend;
		delta.suspend_cnt = curr->suspend_cnt - prev->suspend_cnt;
		delta.txfrm = curr->txfrm - prev->txfrm;
		delta.rxstrt = curr->rxstrt - prev->rxstrt;
		delta.rxglitch = curr->rxglitch - prev->rxglitch;
		delta.rxwifi = curr->rxwifi - prev->rxwifi;
		delta.edcrs = curr->edcrs - prev->edcrs;

		memcpy(priv->msrmnt_stored, &delta, sizeof(wlc_bmac_obss_counts_t));
		priv->results->msrmnt_done = 1;
	}
}

static void
wlc_prot_obss_stats_timeout(void *arg)
{
	wlc_prot_obss_info_t *prot = (wlc_prot_obss_info_t *) arg;
	wlc_prot_obss_stats_sample(prot, 0);
}
#endif // endif

static void
wlc_prot_obss_enable(wlc_prot_obss_info_t *prot, bool enable)
{
	wlc_prot_obss_info_priv_t *priv = WLC_PROT_OBSS_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;

	WL_NONE(("wl%d: %s CTS2SELF\n", wlc->pub->unit, enable ? "Enabling" : "Disabling"));
	wlc_bmac_mhf(wlc->hw, MHF1,
		MHF1_CTS2SELF,
	    enable ? MHF1_CTS2SELF : 0,
	    WLC_BAND_5G);
	if (WLC_TXC_ENAB(wlc))
		wlc_txc_inv_all(wlc->txc);
	prot->protection = enable;
}

/*
 * Returns TRUE if OBSS interference detected. This is
 * determined by RX CRS secondary duration exceeding
 * threshold limit.
 */
bool
wlc_prot_obss_secondary_interference_detected(wlc_prot_obss_info_t *prot,
	uint8 obss_dur_threshold)
{
	wlc_prot_obss_info_priv_t *priv = WLC_PROT_OBSS_INFO_PRIV(prot);

	uint32 rxcrs_sec40, rxcrs_sec20, limit, sample_dur;

	/* Calculate RX CRS Secondary duration */
	rxcrs_sec40 = priv->curr_stats->rxcrs_sec40 - priv->prev_stats->rxcrs_sec40;
	rxcrs_sec20 = priv->curr_stats->rxcrs_sec20 - priv->prev_stats->rxcrs_sec20;

	/* Sample duration is the TSF difference (in usec) between two reads. */
	sample_dur = priv->curr_stats->usecs - priv->prev_stats->usecs;

	/* OBSS detected if RX CRS Secondary exceeds configured limit */
	limit = (obss_dur_threshold * sample_dur) / 100;

	if ((rxcrs_sec40 + rxcrs_sec20) >= limit)
		WL_INFORM(("SEC.OBSS:sdur=%d,rxcrs_sec=%d\n",
			sample_dur, rxcrs_sec40 + rxcrs_sec20));
	return ((rxcrs_sec40 + rxcrs_sec20) >= limit) ? TRUE : FALSE;
}

static int
wlc_prot_obss_init(void *cntxt)
{
	wlc_prot_obss_info_t *prot = (wlc_prot_obss_info_t *) cntxt;
	wlc_prot_obss_info_priv_t *priv = WLC_PROT_OBSS_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;

	if (WLC_CCASTATS_CAP(wlc)) {
		/* Seed the stats */
		wlc_bmac_obss_stats_read(wlc->hw, priv->curr_stats);
	}

	if (WLC_PROT_OBSS_ENAB(wlc->pub)) {
		/* Set secondary RSSI histogram limits */
		wlc_bmac_write_shm(wlc->hw, M_SECRSSI0_MIN,
			priv->config->obss_sec_rssi_lim0);
		wlc_bmac_write_shm(wlc->hw, M_SECRSSI1_MIN,
		    priv->config->obss_sec_rssi_lim1);
	}
	return BCME_OK;
}

/*
 * OBSS protection logic. Enable OBSS protection if it detects OBSS interference.
 * Disable OBSS protection if it no longer is required.
 */
static void
wlc_prot_obss_detection(wlc_prot_obss_info_t *prot, chanspec_t chanspec)
{
	wlc_prot_obss_info_priv_t *priv = WLC_PROT_OBSS_INFO_PRIV(prot);

	if (priv->mode != AUTO) {
		return;
	}

	if (CHSPEC_IS20(chanspec)) {
		if (WLC_PROT_OBSS_PROTECTION(prot)) {
			/* In 20 MHz, disable OBSS protection */
			wlc_prot_obss_enable(prot, FALSE);
		}

		return;
	}

	if (wlc_prot_obss_secondary_interference_detected(prot,
			priv->config->obss_dur_thres)) {
		if (!WLC_PROT_OBSS_PROTECTION(prot)) {
			/* Enable OBSS Full Protection */
			wlc_prot_obss_enable(prot, TRUE);
		}
		/* Clear inactivity timer. */
		priv->obss_inactivity = 0;
	} else {
		/* No OBSS detected. */
		if (WLC_PROT_OBSS_PROTECTION(prot)) {
			/* OBSS protection is in progress. Disable after inactivity period */
			priv->obss_inactivity++;

			if (priv->obss_inactivity >= priv->config->obss_inactivity_period) {
				/* OBSS inactivity criteria met. Disable OBSS protection */
				wlc_prot_obss_enable(prot, FALSE);
			}
		}
		return;
	}
}

static void
wlc_prot_obss_watchdog(void *cntxt)
{
	wlc_prot_obss_info_t *prot = (wlc_prot_obss_info_t *) cntxt;
	wlc_prot_obss_info_priv_t *priv = WLC_PROT_OBSS_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;

	if (WLC_CCASTATS_CAP(wlc)) {
		wlc_obss_util_update(wlc, priv->curr_stats, priv->prev_stats,
			priv->total_stats, wlc->chanspec);
		if (WLC_PROT_OBSS_ENAB(wlc->pub)) {
			wlc_prot_obss_detection(prot, wlc->chanspec);
		}
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_prot_obss_dump(wlc_prot_obss_info_t *prot, void *input, int buf_len, void *output)
{
	wlc_prot_obss_info_priv_t *priv = WLC_PROT_OBSS_INFO_PRIV(prot);
	cca_msrmnt_query *q = (cca_msrmnt_query *) input;
	cca_stats_n_flags *results;

	if (!q->msrmnt_query) {
		priv->results->msrmnt_done = 0;
		wlc_prot_obss_stats_sample(prot, q->time_req);
		memset(output, 0, buf_len);
	} else {
		char *buf_ptr;
		struct bcmstrbuf b;

		results = (cca_stats_n_flags *) output;
		buf_ptr = results->buf;
		buf_len = buf_len - OFFSETOF(cca_stats_n_flags, buf);
		buf_len = (buf_len > 0) ? buf_len : 0;

		results->msrmnt_time = priv->results->msrmnt_time;
		results->msrmnt_done = priv->results->msrmnt_done;
		bcm_binit(&b, buf_ptr, buf_len);

		if (results->msrmnt_done) {
			wlc_obss_util_stats(priv->wlc, priv->msrmnt_stored, priv->prev_stats,
				priv->curr_stats, q->report_opt, &b);
		} else {
			bcm_bprintf(&b, "BUSY\n");
		}
	}
	return 0;
}
#endif // endif

static int
wlc_prot_obss_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
		void *p, uint plen, void *a, int alen, int val_size, struct wlc_if *wlcif)
{
	wlc_prot_obss_info_t *prot = (wlc_prot_obss_info_t *) hdl;
	wlc_prot_obss_info_priv_t *priv = WLC_PROT_OBSS_INFO_PRIV(prot);
	int32 int_val = 0;
	uint32 uint_val;
	int32 *ret_int_ptr;
	bool bool_val;
	int err = 0;

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)a;
	BCM_REFERENCE(ret_int_ptr);

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	uint_val = (uint)int_val;
	BCM_REFERENCE(uint_val);
	bool_val = (int_val != 0) ? TRUE : FALSE;
	BCM_REFERENCE(bool_val);

	switch (actionid) {
	case IOV_GVAL(IOV_OBSS_PROT): {
		if (WLC_PROT_OBSS_ENAB(priv->wlc->pub)) {
			wl_config_t cfg;

			cfg.config = (uint32) priv->mode;
			cfg.status = WLC_PROT_OBSS_PROTECTION(prot);
			memcpy(a, &cfg, sizeof(wl_config_t));
		} else
			err = BCME_UNSUPPORTED;
		break;
	}
	case IOV_SVAL(IOV_OBSS_PROT):
		if (WLC_PROT_OBSS_ENAB(priv->wlc->pub)) {
			/* Set OBSS protection */
			if (int_val == AUTO) {
				/* Set OBSS protection to AUTO and reset timer. */
				priv->mode = AUTO;
				priv->obss_inactivity = 0;
			} else if (int_val == OFF) {
				/* Disable OBSS protection */
				wlc_prot_obss_enable(prot, FALSE);
				priv->mode = FALSE;
			} else if (int_val == ON) {
				/* Enable OBSS protection */
				wlc_prot_obss_enable(prot, TRUE);
				priv->mode = TRUE;
			} else {
				err = BCME_BADARG;
			}
		} else
			err = BCME_UNSUPPORTED;
	        break;
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	case IOV_GVAL(IOV_CCASTATS):
	        err = wlc_prot_obss_dump(prot, p, alen, a);
		break;
#endif // endif
	default:
		err = BCME_UNSUPPORTED;
	}
	return err;
}

wlc_prot_obss_info_t *
BCMATTACHFN(wlc_prot_obss_attach)(wlc_info_t *wlc)
{
	wlc_prot_obss_t *obss;
	wlc_prot_obss_info_t *prot;
	wlc_prot_obss_info_priv_t *priv;
	obss = (wlc_prot_obss_t *) MALLOCZ(wlc->osh, WLC_PROT_OBSS_SIZE);

	if (obss == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit,
		          __FUNCTION__,
		          MALLOCED(wlc->osh)));
		wlc->pub->_prot_obss = FALSE;
		return NULL;
	}
	prot = &obss->pub;
	priv = WLC_PROT_OBSS_INFO_PRIV(prot);
	priv->wlc = wlc;
	priv->config = &obss->config;
	priv->prev_stats = &obss->prev_stats;
	priv->curr_stats = &obss->curr_stats;
	priv->total_stats = &obss->total_stats;
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	priv->msrmnt_stored = &obss->msrmnt_stored;
	priv->results = &obss->results;
	priv->stats_timer = wl_init_timer(wlc->wl, wlc_prot_obss_stats_timeout, prot, "obss_prot");

	if (priv->stats_timer == NULL) {
		WL_ERROR(("wl%d: %s: wl_init_timer failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif

	if (D11REV_GE(wlc->pub->corerev, 40)) {
		priv->config->obss_sec_rssi_lim0 = OBSS_SEC_RSSI_LIM0_DEFAULT;
		priv->config->obss_sec_rssi_lim1 = OBSS_SEC_RSSI_LIM1_DEFAULT;
		priv->config->obss_inactivity_period = OBSS_INACTIVITY_PERIOD_DEFAULT;
		priv->config->obss_dur_thres = OBSS_DUR_THRESHOLD_DEFAULT;
		priv->mode = AUTO;
		wlc->pub->_prot_obss = TRUE;
	}
	if (wlc_module_register(wlc->pub, wlc_prot_obss_iovars,
			"prot_obss", prot, wlc_prot_obss_doiovar,
			wlc_prot_obss_watchdog, wlc_prot_obss_init, NULL)) {
		WL_ERROR(("wl%d: %s: wlc_module_register failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	return prot;
fail:
	ASSERT(1);
	if (prot)
		MFREE(wlc->osh, obss, WLC_PROT_OBSS_SIZE);
	wlc->pub->_prot_obss = FALSE;
	return NULL;
}

void
BCMATTACHFN(wlc_prot_obss_detach)(wlc_prot_obss_info_t *prot)
{
	wlc_prot_obss_info_priv_t *priv;
	wlc_info_t *wlc;
	if (!prot)
		return;
	priv = WLC_PROT_OBSS_INFO_PRIV(prot);
	wlc = priv->wlc;
	wlc->pub->_prot_obss = FALSE;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wl_free_timer(wlc->wl, priv->stats_timer);
#endif // endif

	wlc_module_unregister(wlc->pub, "prot_obss", prot);

	MFREE(wlc->osh, prot, WLC_PROT_OBSS_SIZE);
}
