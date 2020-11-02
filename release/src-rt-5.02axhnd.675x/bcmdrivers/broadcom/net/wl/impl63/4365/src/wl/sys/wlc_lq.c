/**
 * @file
 * @brief
 * Code that controls the link quality
 * Broadcom 802.11bang Networking Device Driver
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
 * $Id: wlc_lq.c 780644 2019-10-31 03:39:21Z $
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
#include <wlioctl.h>
#include <bcmwpa.h>
#include <bcmwifi_channels.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
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
#include <wlc_frmutil.h>
#include <wl_export.h>
#include <wlc_assoc.h>
#include <wlc_bmac.h>
#include <wlc_lq.h>
#ifdef WLOFFLD
#include <wlc_offloads.h>
#endif // endif
#if defined(WL_AIR_IQ)
#include "wlc_airiq.h"
#endif /* WL_AIR_IQ */

/* iovar table */
enum {
	IOV_RSSI_ANT,
	IOV_SNR,
	IOV_NOISE_LTE,
	IOV_RSSI_EVENT,
	IOV_CHAN_QUAL_EVENT,
	IOV_RSSI_WINDOW_SZ,
	IOV_SNR_WINDOW_SZ,
	IOV_LINKQUAL_ONOFF,
	IOV_GET_LINKQUAL_STATS,
	IOV_CHANIM_ENAB,
	IOV_CHANIM_STATE, /* chan interference detect */
	IOV_CHANIM_MODE,
	IOV_CHANIM_STATS, /* the chanim stats */
	IOV_CCASTATS_THRES,
	IOV_CRSGLITCH_THRES, /* chan interference threshold */
	IOV_BGNOISE_THRES,  /* background noise threshold */
	IOV_SAMPLE_PERIOD,
	IOV_THRESHOLD_TIME,
	IOV_MAX_ACS,
	IOV_LOCKOUT_PERIOD,
	IOV_ACS_RECORD,
	IOV_LQ_LAST
};

static const bcm_iovar_t wlc_lq_iovars[] = {
	{"rssi_event", IOV_RSSI_EVENT,
	(0), IOVT_BUFFER, sizeof(wl_rssi_event_t)
	},
	{"chq_event", IOV_CHAN_QUAL_EVENT,
	(0), IOVT_BUFFER, sizeof(wl_chan_qual_event_t)
	},
#ifdef STA
	{"rssi_win", IOV_RSSI_WINDOW_SZ,
	(0), IOVT_UINT16, 0
	},
	{"snr_win", IOV_SNR_WINDOW_SZ,
	(0), IOVT_UINT16, 0
	},
#endif /* STA */

	{"phy_rssi_ant", IOV_RSSI_ANT,
	(0), IOVT_BUFFER, sizeof(wl_rssi_ant_t)
	},
	{"snr", IOV_SNR,
	(0), IOVT_INT32, 0
	},
	{"noise_lte", IOV_NOISE_LTE,
	(0), IOVT_INT32, 0
	},

	{"chanim_enab", IOV_CHANIM_ENAB,
	(0), IOVT_UINT32, 0
	},
#ifdef WLCHANIM
	{"chanim_state", IOV_CHANIM_STATE,
	(0), IOVT_BOOL, 0
	},
	{"chanim_mode", IOV_CHANIM_MODE,
	(0), IOVT_UINT8, 0
	},
	{"chanim_ccathres", IOV_CCASTATS_THRES,
	(0), IOVT_UINT8, 0
	},
	{"chanim_glitchthres", IOV_CRSGLITCH_THRES,
	(0), IOVT_UINT32, 0
	},
	{"chanim_bgnoisethres", IOV_BGNOISE_THRES,
	(0), IOVT_INT8, 0
	},
	{"chanim_sample_period", IOV_SAMPLE_PERIOD,
	(0), IOVT_UINT8, 0
	},
	{"chanim_threshold_time", IOV_THRESHOLD_TIME,
	(0), IOVT_UINT8, 0
	},
	{"chanim_max_acs", IOV_MAX_ACS,
	(0), IOVT_UINT8, 0
	},
	{"chanim_lockout_period", IOV_LOCKOUT_PERIOD,
	(0), IOVT_UINT32, 0
	},
	{"chanim_acs_record", IOV_ACS_RECORD,
	(0), IOVT_BUFFER, sizeof(wl_acs_record_t),
	},
	{"chanim_stats", IOV_CHANIM_STATS,
	(0), IOVT_BUFFER, sizeof(wl_chanim_stats_t),
	},
#endif /* WLCHANIM */
	{NULL, 0, 0, 0, 0}
};

struct lq_info {
	wlc_info_t	*wlc;			/* pointer to main wlc structure */
	wlc_pub_t	*pub;			/* public common code handler */
};

typedef struct {
	chanim_accum_t *accum;		/* accumulative counts */
	wlc_chanim_stats_t *stats;	/* respective chspec stats wlc->chanim_info->stats */
	int ref_cnt;				/* 0 means entry is available */
	chanspec_t chanspec;		/* chanspec of the interface */
	int idx;					/* index to loop through the interfaces */
	chanim_acc_us_t *acc_us;
	chanim_acc_us_t *last_acc_us;
	wlc_chanim_stats_us_t *stats_us;
	chanim_stats_us_t chanim_stats_us;
} chanim_interface_info_t;

struct chanim_interfaces {
	chanim_interface_info_t *if_info; /* stores the stats of an interface */
	int if_info_size;			/* bytes allocated for info block */
	int num_ifs;				/*  for mchan this will be 2, 1 otherwise */
};

static int wlc_lq_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *p, uint plen, void *arg, int alen, int val_size, struct wlc_if *wlcif);

static void wlc_lq_rssi_ant_get(wlc_info_t *wlc, int8 *rssi);
static void wlc_lq_rssi_event_timeout(void *arg);
#ifdef CCA_STATS
static void wlc_lq_cca_chan_qual_event_timeout(void *arg);
#endif // endif

#ifdef WLCHANIM
#ifdef WLCHANIM_US
static wlc_chanim_stats_us_t *wlc_lq_chanim_create_stats_us(wlc_info_t *wlc, chanspec_t chanspec);
static int wlc_lq_chanim_get_stats_us(chanim_info_t *c_info, wl_chanim_stats_us_t* iob,
	int *len, int count, uint32 dur);
static wlc_chanim_stats_us_t *wlc_lq_chanim_find_stats_us(wlc_info_t *wlc, chanspec_t chanspec);
static wlc_chanim_stats_us_t *wlc_lq_chanim_chanspec_to_stats_us(chanim_info_t *c_info,
	chanspec_t chanspec);
static void wlc_lq_chanim_insert_stats_us(wlc_chanim_stats_us_t **rootp,
	wlc_chanim_stats_us_t *new);
static void wlc_lq_chanim_us_accum(chanim_info_t* c_info, chanim_cnt_us_t *cur_cnt,
	chanim_acc_us_t *acc);
static void wlc_chanim_msec_timeout(void *arg);
#endif /* WLCHANIM_US */
static bool wlc_lq_chanim_any_if_info_setup(chanim_interfaces_t *ifaces);
static int wlc_lq_chanim_attach(wlc_info_t *wlc);
static void wlc_lq_chanim_detach(wlc_info_t *wlc);
static wlc_chanim_stats_t *wlc_lq_chanim_create_stats(wlc_info_t *wlc, chanspec_t chanspec);
static int wlc_lq_chanim_get_stats(chanim_info_t *c_info, wl_chanim_stats_t* iob, int *len, int);
static wlc_chanim_stats_t *wlc_lq_chanim_find_stats(wlc_info_t *wlc, chanspec_t chanspec);
static int wlc_lq_chanim_get_acs_record(chanim_info_t *c_info, int buf_len, void *output);
static void wlc_lq_chanim_insert_stats(wlc_chanim_stats_t **rootp, wlc_chanim_stats_t *new);
static void wlc_lq_chanim_meas(wlc_info_t *wlc, chanim_cnt_t *chanim_cnt,
	chanim_cnt_us_t *chanim_cnt_us);
static void wlc_lq_chanim_glitch_accum(chanim_info_t* c_info, chanim_cnt_t *cur_cnt,
	chanim_accum_t *acc);
static void wlc_lq_chanim_badplcp_accum(chanim_info_t* c_info, chanim_cnt_t *cur_cnt,
	chanim_accum_t *acc);
static void wlc_lq_chanim_ccastats_accum(chanim_info_t* c_info, chanim_cnt_t *cur_cnt,
	chanim_accum_t *acc);
static void wlc_lq_chanim_accum(wlc_info_t* wlc, chanspec_t chanspec, chanim_accum_t *acc,
	chanim_acc_us_t *acc_us);
static void wlc_lq_chanim_clear_acc(wlc_info_t* wlc, chanim_accum_t* acc,
	chanim_acc_us_t *acc_us, bool chan_switch);
static int8 wlc_lq_chanim_phy_noise(wlc_info_t *wlc);
static void wlc_lq_chanim_close(wlc_info_t* wlc, chanspec_t chanspec, chanim_accum_t* acc,
	wlc_chanim_stats_t *cur_stats, chanim_acc_us_t *acc_us,
	wlc_chanim_stats_us_t *cur_stats_us, bool chan_switch);
static bool wlc_lq_chanim_interfered_glitch(wlc_chanim_stats_t *stats, uint32 thres);
static bool wlc_lq_chanim_interfered_cca(wlc_chanim_stats_t *stats, uint32 thres);
static bool wlc_lq_chanim_interfered_noise(wlc_chanim_stats_t *stats, int8 thres);
static chanim_interface_info_t *wlc_lq_chanim_if_info_find(chanim_interfaces_t *ifaces,
	chanspec_t chanspec);

#ifdef BCMDBG
static int wlc_lq_chanim_display(wlc_info_t *wlc, chanspec_t chanspec,
	wlc_chanim_stats_t *cur_stats);
#endif // endif

#if defined(BCMDBG_DUMP)
static int wlc_dump_chanim(wlc_info_t *wlc, struct bcmstrbuf *b);
#endif // endif
#endif /* WLCHANIM */

#ifdef BCMDBG
static int wlc_dump_lq(wlc_info_t *wlc, struct bcmstrbuf *b);
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST)
static int wlc_dump_rssi(wlc_info_t *wlc, struct bcmstrbuf *b);
#endif // endif

#ifdef WLCQ
static int wlc_lq_channel_qa_eval(wlc_info_t *wlc);
static void wlc_lq_channel_qa_sample_cb(wlc_info_t *wlc, uint8 channel, int8 noise_dbm);
#endif /* WLCQ */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

int
BCMATTACHFN(wlc_lq_attach)(wlc_info_t *wlc)
{
	/* register module */
	if (wlc_module_register(wlc->pub, wlc_lq_iovars, "lq", wlc, wlc_lq_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s: wlc_module_register failed\n", wlc->pub->unit, __FUNCTION__));
		return -1;
	}

#ifdef BCMDBG
	wlc_dump_register(wlc->pub, "lq", (dump_fn_t)wlc_dump_lq, (void *)wlc);
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST)
	wlc_dump_register(wlc->pub, "rssi", (dump_fn_t)wlc_dump_rssi, (void *)wlc);
#endif // endif

#ifdef WLCHANIM
	if (wlc_lq_chanim_attach(wlc)) {
		WL_ERROR(("wl%d: %s: chanim attach failed\n", wlc->pub->unit, __FUNCTION__));
		return -1;
	}
#endif /* WLCHANIM */

	return 0;
}

void
BCMATTACHFN(wlc_lq_detach)(wlc_info_t *wlc)
{
#ifdef WLCHANIM
	wlc_lq_chanim_detach(wlc);
#endif // endif

	wlc_module_unregister(wlc->pub, "lq", wlc);
}

static int
wlc_lq_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int val_size, struct wlc_if *wlcif)
{
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg;
	int32 int_val = 0;
	uint32 uint_val;
	int32 *ret_int_ptr;
	bool bool_val;
	int err = 0;

	wlc = (wlc_info_t *)hdl;

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)a;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	uint_val = (uint)int_val;
	BCM_REFERENCE(uint_val);
	bool_val = (int_val != 0) ? TRUE : FALSE;
	BCM_REFERENCE(bool_val);

	switch (actionid) {
	case IOV_GVAL(IOV_RSSI_ANT): {
		wl_rssi_ant_t rssi_ant;

		bzero((char *)&rssi_ant, sizeof(wl_rssi_ant_t));
		rssi_ant.version = WL_RSSI_ANT_VERSION;

		/* only get RSSI for one antenna for all SISO PHY */
		if (WLCISAPHY(wlc->band) || WLCISGPHY(wlc->band) || WLCISLPPHY(wlc->band) ||
			WLCISLCNPHY(wlc->band))
		{
			rssi_ant.count = 1;
			rssi_ant.rssi_ant[0] = (int8)(bsscfg->link->rssi);

		} else if (WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) {
			int8 rssi[WL_RSSI_ANT_MAX] = {0, 0, 0, 0};
			uint8 i;

			wlc_lq_rssi_ant_get(wlc, rssi);
			rssi_ant.count = (WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) ?
			    WL_ANT_HT_RX_MAX : WL_ANT_RX_MAX;

			for (i = WL_ANT_IDX_1; i < rssi_ant.count; i++) {
				if (wlc->stf->rxchain & (1 << i))
					rssi_ant.rssi_ant[i] = rssi[i];
			}
		}
		else {
			rssi_ant.count = 0;
		}

		bcopy(&rssi_ant, a, sizeof(wl_rssi_ant_t));
		break;
	}

	case IOV_GVAL(IOV_RSSI_EVENT):
		memcpy(a, bsscfg->link->rssi_event, sizeof(wl_rssi_event_t));
		break;

	case IOV_SVAL(IOV_RSSI_EVENT): {
		wlc_link_qual_t *link = bsscfg->link;
		if (link->rssi_event_timer) {
			wl_del_timer(wlc->wl, link->rssi_event_timer);
			wl_free_timer(wlc->wl, link->rssi_event_timer);
			link->rssi_event_timer = NULL;
		}
		link->is_rssi_event_timer_active = FALSE;
		link->rssi_level = 0; 	/* reset current rssi level */
		memcpy(link->rssi_event, a, sizeof(wl_rssi_event_t));
		if (link->rssi_event->rate_limit_msec) {
			link->rssi_event_timer = wl_init_timer(wlc->wl,
				wlc_lq_rssi_event_timeout, bsscfg, "rssi_event");
		}
		break;
	}

#ifdef CCA_STATS
	case IOV_GVAL(IOV_CHAN_QUAL_EVENT):
		if (wlc->cca_chan_qual == NULL)
			return BCME_NOTREADY;
		memcpy(a, &(wlc->cca_chan_qual->event), sizeof(wl_chan_qual_event_t));
		break;

	case IOV_SVAL(IOV_CHAN_QUAL_EVENT): {
		cca_chan_qual_t *chq = wlc->cca_chan_qual;
		if (chq == NULL)
			return BCME_NOTREADY;
		if (chq->cca_event_timer) {
			wl_del_timer(wlc->wl, chq->cca_event_timer);
			wl_free_timer(wlc->wl, chq->cca_event_timer);
			chq->cca_event_timer = NULL;
		}
		chq->is_cca_event_timer_active = FALSE;
		memset(chq->level, 0, sizeof(chq->level));	/* reset all current levels */
		memcpy(&(chq->event), a, sizeof(wl_chan_qual_event_t));
		if (chq->event.rate_limit_msec) {
			chq->cca_event_timer = wl_init_timer(wlc->wl,
				wlc_lq_cca_chan_qual_event_timeout, wlc, "chan_qual_event");
		}
		break;
	}
#endif /* CCA_STATS */

#ifdef STA
	case IOV_SVAL(IOV_RSSI_WINDOW_SZ):
		if (((int_val & RSSI_PKT_WIN_SZ_MASK) == 0) ||
		    ((int_val & RSSI_PKT_WIN_SZ_MASK) > MA_WINDOW_SZ)) {
			err = BCME_RANGE;
			break;
		}

		if ((int_val & (int_val - 1) & RSSI_PKT_WIN_SZ_MASK) != 0) {
			/* Value passed is not power of 2 */
			err = BCME_BADARG;
			break;
		}
		bsscfg->link->rssi_pkt_win_sz = (uint16)int_val;
		wlc_lq_rssi_reset_ma(bsscfg, bsscfg->link->rssi);
		wlc_lq_rssi_snr_noise_reset_ma(wlc, bsscfg, bsscfg->link->rssi,
			bsscfg->link->snr, wlc->noise_lvl);
		break;

	case IOV_GVAL(IOV_RSSI_WINDOW_SZ):
		*ret_int_ptr = (int32)bsscfg->link->rssi_pkt_win_sz;
		break;

	case IOV_SVAL(IOV_SNR_WINDOW_SZ):
		if (((int_val & RSSI_PKT_WIN_SZ_MASK) == 0) ||
		    ((int_val & RSSI_PKT_WIN_SZ_MASK) > MA_WINDOW_SZ)) {
			err = BCME_RANGE;
			break;
		}

		if ((int_val & (int_val - 1) & RSSI_PKT_WIN_SZ_MASK) != 0) {
			/* Value passed is not power of 2 */
			err = BCME_BADARG;
			break;
		}
		bsscfg->link->snr_pkt_win_sz = (uint16)int_val;
		wlc_lq_rssi_snr_noise_reset_ma(wlc, bsscfg, bsscfg->link->rssi,
			bsscfg->link->snr, wlc->noise_lvl);
		break;

	case IOV_GVAL(IOV_SNR_WINDOW_SZ):
		*ret_int_ptr = (int32)bsscfg->link->snr_pkt_win_sz;
		break;
#endif /* STA */

	case IOV_GVAL(IOV_SNR):
		*ret_int_ptr = (int32)bsscfg->link->snr;
		break;

	case IOV_GVAL(IOV_NOISE_LTE): {
		*ret_int_ptr = (int32)wlc->noise_lte_lvl;
		break;
	}

	case IOV_GVAL(IOV_CHANIM_ENAB):
		*ret_int_ptr = (int32)WLC_CHANIM_ENAB(wlc);
		break;

#ifdef WLCHANIM
	case IOV_GVAL(IOV_CHANIM_STATE): {
		chanspec_t chspec;
		wlc_chanim_stats_t *stats;

		if (plen < (int)sizeof(int)) {
			err = BCME_BUFTOOSHORT;
			break;
		}

		chspec = (chanspec_t) int_val;

		if (wf_chspec_malformed(chspec)) {
			err = BCME_BADCHAN;
			break;
		}

		stats = wlc_lq_chanim_chanspec_to_stats(wlc->chanim_info, chspec, FALSE);

		if (!stats) {
			err = BCME_RANGE;
			break;
		}

		if (WLC_CHANIM_MODE_EXT(wlc->chanim_info))
			*ret_int_ptr = (int32) chanim_mark(wlc->chanim_info).state;
		else
			*ret_int_ptr = (int32) wlc_lq_chanim_interfered(wlc, chspec);

		break;
	}

	case IOV_SVAL(IOV_CHANIM_STATE):
		if (WLC_CHANIM_MODE_EXT(wlc->chanim_info))
			chanim_mark(wlc->chanim_info).state = (bool)int_val;
		break;

	case IOV_GVAL(IOV_CHANIM_MODE):
		*ret_int_ptr = (int32)chanim_config(wlc->chanim_info).mode;
		break;

	case IOV_GVAL(IOV_SAMPLE_PERIOD):
		*ret_int_ptr = (int32)chanim_config(wlc->chanim_info).sample_period;
		break;

	case IOV_SVAL(IOV_SAMPLE_PERIOD):
		if (int_val < SAMPLE_PERIOD_MIN)
			err = BCME_RANGE;
		chanim_config(wlc->chanim_info).sample_period = (uint8)int_val;
		break;

	case IOV_SVAL(IOV_CHANIM_MODE):
		if (int_val >= CHANIM_MODE_MAX) {
			err = BCME_RANGE;
			break;
		}

		chanim_config(wlc->chanim_info).mode = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_CCASTATS_THRES):
		*ret_int_ptr = (int32)chanim_config(wlc->chanim_info).ccastats_thres;
		break;

	case IOV_SVAL(IOV_CCASTATS_THRES):
		chanim_config(wlc->chanim_info).ccastats_thres = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_CRSGLITCH_THRES):
		*ret_int_ptr = chanim_config(wlc->chanim_info).crsglitch_thres;
		break;

	case IOV_SVAL(IOV_CRSGLITCH_THRES):
		chanim_config(wlc->chanim_info).crsglitch_thres = int_val;
		break;

	case IOV_GVAL(IOV_BGNOISE_THRES):
		*ret_int_ptr = (int32)chanim_config(wlc->chanim_info).bgnoise_thres;
		break;

	case IOV_SVAL(IOV_BGNOISE_THRES):
		chanim_config(wlc->chanim_info).bgnoise_thres = (int8)int_val;
		break;

	case IOV_GVAL(IOV_THRESHOLD_TIME):
		*ret_int_ptr = (int32)chanim_config(wlc->chanim_info).threshold_time;
		break;

	case IOV_SVAL(IOV_THRESHOLD_TIME):
		if (int_val < THRESHOLD_TIME_MIN)
			err = BCME_RANGE;
		chanim_config(wlc->chanim_info).threshold_time = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_MAX_ACS):
		*ret_int_ptr = (int32)chanim_config(wlc->chanim_info).max_acs;
		break;

	case IOV_SVAL(IOV_MAX_ACS):
		if (int_val > CHANIM_ACS_RECORD)
			err = BCME_RANGE;
		chanim_config(wlc->chanim_info).max_acs = (uint8)int_val;
		break;

	case IOV_GVAL(IOV_LOCKOUT_PERIOD):
		*ret_int_ptr = chanim_config(wlc->chanim_info).lockout_period;
		break;

	case IOV_SVAL(IOV_LOCKOUT_PERIOD):
		chanim_config(wlc->chanim_info).lockout_period = int_val;
		break;

	case IOV_GVAL(IOV_ACS_RECORD):
		if (alen < (int)sizeof(wl_acs_record_t))
			err = BCME_BUFTOOSHORT;
		else
			err = wlc_lq_chanim_get_acs_record(wlc->chanim_info, alen, a);
		break;

	case IOV_GVAL(IOV_CHANIM_STATS): {
		wl_chanim_stats_t input = *((wl_chanim_stats_t *)p);
		int buflen = 0;
		if (input.count == WL_CHANIM_COUNT_ONE || input.count == WL_CHANIM_COUNT_ALL ||
			input.count == WL_CHANIM_READ_VERSION) {
			wl_chanim_stats_t *iob = (wl_chanim_stats_t*) a;

			if (input.count == WL_CHANIM_READ_VERSION) {
				iob->version = WL_CHANIM_STATS_VERSION;
				iob->count = 0;
				return err;
			}
			if ((uint)alen < WL_CHANIM_STATS_FIXED_LEN) {
				err = BCME_BUFTOOSHORT;
				break;
			}

			buflen = (int)input.buflen;

			if ((uint)buflen < WL_CHANIM_STATS_FIXED_LEN) {
				err = BCME_BUFTOOSHORT;
				break;
			}
			err = wlc_lq_chanim_get_stats(wlc->chanim_info, iob, &buflen, input.count);
		}
#ifdef WLCHANIM_US
		else if (input.count == WL_CHANIM_COUNT_US_ONE ||
				input.count == WL_CHANIM_COUNT_US_ALL ||
				input.count == WL_CHANIM_COUNT_US_RESET ||
				input.count == WL_CHANIM_US_DUR ||
				input.count == WL_CHANIM_US_DUR_GET) {
			wl_chanim_stats_us_t *iob = (wl_chanim_stats_us_t*) a;
			wl_chanim_stats_us_t input_us = *((wl_chanim_stats_us_t *)p);
			if (input.count != WL_CHANIM_COUNT_US_RESET) {
				if ((uint)alen < WL_CHANIM_STATS_US_FIXED_LEN) {
					err = BCME_BUFTOOSHORT;
					break;
				}

				buflen = (int)input.buflen;
				if ((uint)buflen < WL_CHANIM_STATS_US_FIXED_LEN) {
					err = BCME_BUFTOOSHORT;
					break;
				}
			}
			err = wlc_lq_chanim_get_stats_us(wlc->chanim_info, iob,
					&buflen, input.count, input_us.dur);
		}
#endif /* WLCHANIM_US */
		else {
			err = BCME_UNSUPPORTED;
		}
		break;
	}
#endif /* WLCHANIM */

	default:
		err = BCME_UNSUPPORTED;
	}

	return err;
}

void
wlc_lq_rssi_init(wlc_info_t *wlc, int rssi)
{
	uint i, k, max_ant;

	/* legacy PHY */
	max_ant = (uint32)((WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) ?
	    WL_ANT_HT_RX_MAX : WL_ANT_RX_MAX);
	for (i = 0; i < WLC_RSSI_WINDOW_SZ; i++) {
		for (k = WL_ANT_IDX_1; k < max_ant; k++) {
			if (((wlc->stf->rxchain >> k) & 1) == 0)
				continue;
			wlc->rssi_win_rfchain[k][i] = (int16)rssi;
		}
	}
	wlc->rssi_win_rfchain_idx = 0;
}

/* Reset RSSI moving average */
void
wlc_lq_rssi_snr_noise_reset_ma(wlc_info_t *wlc, wlc_bsscfg_t *cfg, int rssi, int snr, int noise)
{
	int i;
	wlc_link_qual_t *link = cfg->link;

	link->rssi_pkt_tot = 0;
	link->rssi_pkt_count = 0;
	link->rssi_pkt_index = 0;

	link->snr_ma = 0;
	link->snr_no_of_valid_values = 0;
	link->snr_index = 0;

	for (i = 0; i < MA_WINDOW_SZ; i++) {
		link->rssi_pkt_window[i] = 0;
		link->snr_window[i] = 0;
	}

	link->rssi = rssi;
	link->rssi_qdb = 0;
	link->snr = snr;
	wlc->noise_lvl = noise;
	wlc->noise_lte_lvl = noise;
}

/* Reset RSSI moving average */
void
wlc_lq_rssi_reset_ma(wlc_bsscfg_t *cfg, int rssi)
{
	int i;
	wlc_link_qual_t *link = cfg->link;

	link->rssi_pkt_tot = 0;
	link->rssi_pkt_count = 0;
	for (i = 0; i < MA_WINDOW_SZ; i++)
		link->rssi_pkt_window[i] = 0;
	link->rssi_pkt_index = 0;

	link->rssi = rssi;
}

int
wlc_lq_rssi_update_ma(wlc_bsscfg_t *cfg, int nval, int qdb, bool unicast)
{
	wlc_link_qual_t *link = cfg->link;
#ifdef DOS
	link->rssi = nval;
	return nval;
#endif // endif

	if (nval != WLC_RSSI_INVALID) {
		bool admit_mcast_only = RSSI_ADMIT_MCAST_ONLY(link->rssi_pkt_win_sz);
		uint16 rssi_pkt_win_sz = link->rssi_pkt_win_sz & RSSI_PKT_WIN_SZ_MASK;

		/* rssi filtering with unicast when possible
		 * (count all before rssi window reaching full and no unicast for a while)
		 */
		if (RSSI_ADMIT_ALL_FRAME(link->rssi_pkt_win_sz) ||
		    (admit_mcast_only && !unicast) ||
		    (!admit_mcast_only && (unicast || !link->last_rssi_is_unicast)) ||
		    (link->rssi_pkt_count < rssi_pkt_win_sz)) {
			/* evict old value */
			link->rssi_pkt_tot -= link->rssi_pkt_window[link->rssi_pkt_index];

			/* admit new value - combine rssi(in nval) and rssi_qdb (in qdb) */
			nval = (nval << 2) + qdb;
			link->rssi_pkt_tot += nval;
			link->rssi_pkt_window[link->rssi_pkt_index] = nval;
			link->rssi_pkt_index = MODINC_POW2(link->rssi_pkt_index,
				rssi_pkt_win_sz);
			if (link->rssi_pkt_count < rssi_pkt_win_sz)
				link->rssi_pkt_count++;
			link->rssi = (link->rssi_pkt_tot / link->rssi_pkt_count);
			link->rssi_qdb = link->rssi & 3;
			link->rssi = link->rssi >> 2;

			if (RSSI_PKT_WIN_DEBUG(link->rssi_pkt_win_sz))
				WL_PRINT(("rssi_flt(0x%04x:%d): new=%d(%c) -> rssi=%d\n",
					link->rssi_pkt_win_sz, link->rssi_pkt_count, nval,
					(unicast ? 'U' : 'M'), link->rssi));
		}

		link->last_rssi_is_unicast = unicast;
	}
	else if (link->rssi_pkt_count == 0)
		link->rssi = WLC_RSSI_INVALID;

	return link->rssi;
}

void
wlc_lq_rssi_event_update(wlc_bsscfg_t *cfg)
{
	int level;
	wlc_info_t *wlc = cfg->wlc;
	wlc_link_qual_t *link = cfg->link;

	/* no update if timer active */
	if (link->is_rssi_event_timer_active)
		return;

	/* find rssi level */
	for (level = 0; level < link->rssi_event->num_rssi_levels; level++) {
		if (link->rssi <= link->rssi_event->rssi_levels[level])
			break;
	}

	if (level != link->rssi_level) {
		/* rssi level changed - post rssi event */
		wl_event_data_rssi_t value;
		value.rssi  = hton32(link->rssi);
		value.snr   = hton32(link->snr);
		value.noise = hton32(wlc->noise_lvl);
		link->rssi_level = (uint8)level;
		wlc_bss_mac_event(wlc, cfg, WLC_E_RSSI, NULL, 0, 0, 0, &value, sizeof(value));
		if (link->rssi_event_timer && link->rssi_event->rate_limit_msec) {
			/* rate limit rssi events */
			link->is_rssi_event_timer_active = TRUE;
			wl_add_timer(wlc->wl, link->rssi_event_timer,
				link->rssi_event->rate_limit_msec, FALSE);
		}
	}
}

/* The rssi compute is done in low level driver and embedded in the rx pkt in wlc_d11rxhdr,
 * per-antenna rssi are also embedded in wlc_d11rxhdr for moving average cal here
 */
int8
wlc_lq_rssi_pktrxh_cal(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh)
{
	int i, max_ant;

	max_ant = (uint32)((WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) ?
	    WL_ANT_HT_RX_MAX : WL_ANT_RX_MAX);
	if (!wrxh->do_rssi_ma && wrxh->rssi != WLC_RSSI_INVALID) {
		/* go through all valid antennas */
		for (i = WL_ANT_IDX_1; i < max_ant; i++) {
			if (((wlc->stf->rxchain >> i) & 1) == 0)
				continue;
			wlc->rssi_win_rfchain[i][wlc->rssi_win_rfchain_idx] = wrxh->rxpwr[i];
		}

		wlc->rssi_win_rfchain_idx = MODINC_POW2(wlc->rssi_win_rfchain_idx,
			WLC_RSSI_WINDOW_SZ);
		wrxh->do_rssi_ma = 1;
	}

	return wrxh->rssi;
}

static void
wlc_lq_rssi_ant_get(wlc_info_t *wlc, int8 *rssi)
{
	uint32 i, k, chains, idx;
	/* use int32 to avoid overflow when accumulate int8 */
	int32 rssi_sum[WL_RSSI_ANT_MAX] = {0, 0, 0, 0};

	idx = wlc->rssi_win_rfchain_idx;

	chains = (uint32)((WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) ?
	    WL_ANT_HT_RX_MAX : WL_ANT_RX_MAX);

#if defined(WLOFFLD) /* RADAR <13563970> */
	/*
	 * Check if we have any packets received in last tick.
	 * if not then return offload engines rssi value else
	 * fall back to original value
	 */
	if (WLOFFLD_CAP(wlc)) {
		if (wlc_ol_bcn_is_enable(wlc->ol)) {
			if (wlc->_wlc_rate_measurement->rxframes_rate < MA_WINDOW_SZ) {
				int8 ol_rssi = wlc_ol_rssi_get_value(wlc->ol);
				if (ol_rssi < 0) {
					for (k = WL_ANT_IDX_1; k < chains; k++) {
						if (((wlc->stf->rxchain >> k) & 1) == 0) {
							rssi[k] = WLC_RSSI_INVALID;
							continue;
						}
						rssi[k] = wlc_ol_rssi_get_ant(wlc->ol, k);
					}
					WL_TRACE(("%s: RSSI (%d) from OFFLOAD Engine\n",
						__FUNCTION__, ol_rssi));
					return;
				} else {
					WL_TRACE(("%s: Garbage RSSI (%d) from OFFLOAD Engine\n",
						__FUNCTION__, ol_rssi));
				}
			} else {
				WL_TRACE(("%s: No RSSI from OFFLOAD Engine: Not enough PKT\n",
				__FUNCTION__));
			}
		} else {
			WL_TRACE(("%s: No RSSI from OFFLOAD Engine: Beacon disabled\n",
				__FUNCTION__));
		}
		wlc_ol_inc_rssi_cnt_host(wlc->ol);
	} else {
		WL_TRACE(("%s: No RSSI from OFFLOAD Engine: No OFFLOAD CAP\n",
			__FUNCTION__));
	}
#endif /* defined(WLOFFLD) */

	for (i = 0; i < WLC_RSSI_WINDOW_SZ; i++) {
		for (k = WL_ANT_IDX_1; k < chains; k++) {
			if (((wlc->stf->rxchain >> k) & 1) == 0)
				continue;
			rssi_sum[k] += wlc->rssi_win_rfchain[k][idx];
		}
		idx = MODINC_POW2(idx, WLC_RSSI_WINDOW_SZ);
	}

	for (k = WL_ANT_IDX_1; k < chains; k++) {
		rssi[k] = (int8)(rssi_sum[k] / WLC_RSSI_WINDOW_SZ);
	}
}

static void
wlc_lq_rssi_event_timeout(void *arg)
{
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t*)arg;
	cfg->link->is_rssi_event_timer_active = FALSE;
}

/* Smooth SNR observation with an 8-point moving average
 * XXX - Ignore boundary conditions and sample age
 */
int
wlc_lq_snr_update_ma(wlc_bsscfg_t *cfg, int nval, bool unicast)
{
	wlc_link_qual_t *link = cfg->link;
#ifdef DOS
	link->snr = nval;
	return nval;
#endif // endif

	if (nval != WLC_SNR_INVALID) {
		bool admit_mcast_only = RSSI_ADMIT_MCAST_ONLY(link->snr_pkt_win_sz);
		uint16 snr_pkt_win_sz = link->snr_pkt_win_sz & RSSI_PKT_WIN_SZ_MASK;

		/* snr filtering with unicast when possible
		 * (count all before snr window reaching full and no unicast for a while)
		 */
		if (RSSI_ADMIT_ALL_FRAME(link->snr_pkt_win_sz) ||
		    (admit_mcast_only && !unicast) ||
		    (!admit_mcast_only && (unicast || !link->last_snr_is_unicast)) ||
		    (link->snr_no_of_valid_values < snr_pkt_win_sz)) {
			/* evict old value */
			link->snr_ma -= (int32)link->snr_window[link->snr_index];

			/* admit new value */
			link->snr_ma += (int32)nval;
			link->snr_window[link->snr_index] = (uint8)nval;
			link->snr_index = MODINC_POW2(link->snr_index, snr_pkt_win_sz);

			if (link->snr_no_of_valid_values < snr_pkt_win_sz) {
				link->snr_no_of_valid_values++;
			}

			link->snr = ((link->snr_ma + (link->snr_no_of_valid_values>>1)) /
				link->snr_no_of_valid_values);

			if (RSSI_PKT_WIN_DEBUG(link->snr_pkt_win_sz))
				WL_PRINT(("snr_flt(0x%04x:%d): new=%d(%c) -> snr=%d\n",
					link->snr_pkt_win_sz, link->snr_no_of_valid_values, nval,
					(unicast ? 'U' : 'M'), link->snr));
		}

		link->last_snr_is_unicast = unicast;
	}
	else if (link->snr_no_of_valid_values == 0)
		link->snr = WLC_SNR_INVALID;

	return link->snr;
}

int
wlc_lq_noise_update_ma(wlc_info_t *wlc, int nval)
{
#ifdef DOS
	wlc->noise_lvl = nval;
	return nval;
#endif // endif

	/* Asymmetric noise floor filter:
	 *	Going up slowly by only +1
	 *	Coming down faster by diff/2
	 */
	if (nval != WLC_NOISE_INVALID) {
		if (wlc->noise_lvl == WLC_NOISE_INVALID)
			wlc->noise_lvl = nval;
		else if (nval > wlc->noise_lvl)
			wlc->noise_lvl ++;
		else if (nval < wlc->noise_lvl)
			wlc->noise_lvl += (nval - wlc->noise_lvl - 1) / 2;

#ifdef CCA_STATS
		if (wlc_lq_cca_chan_qual_event_update(wlc, WL_CHAN_QUAL_NF, wlc->noise_lvl)) {
			cca_chan_qual_event_t event_output;

			event_output.status = 0;
			event_output.id = WL_CHAN_QUAL_NF;
			event_output.chanspec = wlc->chanspec;
			event_output.len = sizeof(event_output.noise);
			event_output.noise = wlc->noise_lvl;

			wlc_bss_mac_event(wlc, NULL, WLC_E_CCA_CHAN_QUAL, NULL,
				0, 0, 0, &event_output, sizeof(event_output));
		}
#endif /* CCA_STATS */
	}

	return wlc->noise_lvl;
}

int
wlc_lq_noise_lte_update_ma(wlc_info_t *wlc, int nval)
{
#ifdef DOS
	wlc->noise_lte_lvl = nval;
	return nval;
#endif // endif

	/* Asymmetric noise floor filter:
	 *	Going up slowly by only +1
	 *	Coming down faster by diff/2
	 */
	if (nval != WLC_NOISE_INVALID) {
		if (wlc->noise_lvl == WLC_NOISE_INVALID)
			wlc->noise_lvl = nval;
		else if (nval > wlc->noise_lte_lvl)
			wlc->noise_lte_lvl ++;
		else if (nval < wlc->noise_lte_lvl)
			wlc->noise_lte_lvl += (nval - wlc->noise_lte_lvl - 1) / 2;

#ifdef CCA_STATS
		if (wlc_lq_cca_chan_qual_event_update(wlc, WL_CHAN_QUAL_NF_LTE,
			wlc->noise_lte_lvl)) {
			cca_chan_qual_event_t event_output;

			event_output.status = 0;
			event_output.id = WL_CHAN_QUAL_NF_LTE;
			event_output.chanspec = wlc->chanspec;
			event_output.len = sizeof(event_output.noise);
			event_output.noise = wlc->noise_lte_lvl;

			wlc_bss_mac_event(wlc, NULL, WLC_E_CCA_CHAN_QUAL, NULL,
				0, 0, 0, &event_output, sizeof(event_output));
		}
#endif /* CCA_STATS */
	}

	return wlc->noise_lte_lvl;
}

/*
This function returns SNR during the recently received frame.
SNR is computed by PHY during frame reception and keeps it in the
D11 frame header. This function reads that value from the D11 frame header
and returns it.
For CCK frame SNR in the D11 frame is in dB in Q.2 format.
For OFDM frames SNR in the D11 frame is 9.30139866 * SNR dB in Q.0 format.
This function returns the SNR for both the frames in dB in Q.0 format.
Brief documentation is available about signalQuality parameter in D11 frame is
available at http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/MAC-PhyInterface.
*/
int
wlc_lq_recv_snr_compute(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh, int noise_lvl)
{
	int frameType;
	int signalQuality, snr = WLC_SNR_INVALID;
	int32 snr32;
	int16 multiplication_coefficient;

	if (WLCISLPPHY(wlc->band)) {
		frameType = wrxh->rxhdr.PhyRxStatus_0 & PRXS0_FT_MASK;
		signalQuality = wrxh->rxhdr.PhyRxStatus_1 & PRXS1_SQ_MASK;
		signalQuality = signalQuality >> PRXS1_SQ_SHIFT;
		if (frameType == PRXS0_CCK) {
			snr = signalQuality;		/* Q.2 format */
			snr = ((snr + 2) >> 2);		/* bring to Q.0 format and round */
		}
		else {
			/* signalQuality = 9.30139866 * SNR dB. So convert it into
			* SNR in db at Q.0 format.
			*/
			multiplication_coefficient = (int16)((1.0/9.30139866) * (1<<18)); /* Q.18 */
			snr32 = signalQuality * multiplication_coefficient;	/* snr dB in Q.18 */
			snr = (snr32 + (1 << 17)) >> 18;		/* snr in dB in Q.0 */
		}
	} else if (WLCISLCN40PHY(wlc->band)) {
		if (wrxh->rssi != WLC_RSSI_INVALID) {
			frameType = wrxh->rxhdr.PhyRxStatus_0 & PRXS0_FT_MASK;
			if (frameType != PRXS0_CCK) {
				snr = (wrxh->rxhdr.PhyRxStatus_1 & PRXS1_SQ_MASK)
					>> PRXS1_SQ_SHIFT;
			}
			else {
				if (!noise_lvl)
					noise_lvl = WLC_RSSI_NO_SIGNAL;
				snr = wrxh->rssi - noise_lvl;

				/* Backup just in case Noise Est is incorrect */
				if (snr < 0)
					snr = 3;
				if (snr >= WLC_SNR_EXCELLENT) {
					snr = WLC_SNR_EXCELLENT;
				}
			}
			WL_INFORM(("frtyp=%d, snr=%d, rssi=%d, noise=%d\n",
				frameType, snr, wrxh->rssi,
				noise_lvl));
		}
	}

	/* return SNR */
	return snr;
}

#ifdef CCA_STATS
static void
wlc_lq_cca_chan_qual_event_timeout(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t*)arg;
	wlc->cca_chan_qual->is_cca_event_timer_active = FALSE;
}

/* return TRUE to indicate interested metric level changed */
bool
wlc_lq_cca_chan_qual_event_update(wlc_info_t *wlc, uint8 id, int v)
{
	cca_chan_qual_t *chq = wlc->cca_chan_qual;
	uint8 level, prev_level;

	if (chq == NULL)
		return FALSE;

	/* no update if timer active */
	if (chq->is_cca_event_timer_active)
		return FALSE;

	/* check for supported metric */
	if ((id >= chq->event.num_metrics) || (id >= WL_CHAN_QUAL_TOTAL))
		return FALSE;

	/* find metric level */
	prev_level = chq->level[id];
	for (level = 0; level < chq->event.metric[id].num_levels; level++) {
		int thresh;
		if (level < prev_level)
			thresh = chq->event.metric[id].htol[level];
		else
			thresh = chq->event.metric[id].ltoh[level];

		if (v < thresh)
			break;
	}

	if (level == prev_level)
		return FALSE;

	chq->level[id] = level;

	if (chq->cca_event_timer && chq->event.rate_limit_msec) {
		/* rate limit events */
		chq->is_cca_event_timer_active = TRUE;
		wl_add_timer(wlc->wl, chq->cca_event_timer,
			chq->event.rate_limit_msec, FALSE);
	}
	return TRUE;
}
#endif /* CCA_STATS */

#ifdef WLCQ
static int
wlc_lq_channel_qa_eval(wlc_info_t *wlc)
{
	int k;
	int sample_count;
	int rssi_avg;
	int noise_est;
	int quality_metric;

	sample_count = (int)wlc->channel_qa_sample_num;
	rssi_avg = 0;
	for (k = 0; k < sample_count; k++)
		rssi_avg += wlc->channel_qa_sample[k];
	rssi_avg = (rssi_avg + sample_count/2) / sample_count;

	noise_est = rssi_avg;

	if (noise_est < -85)
		quality_metric = 3;
	else if (noise_est < -75)
		quality_metric = 2;
	else if (noise_est < -65)
		quality_metric = 1;
	else
		quality_metric = 0;

	WL_INFORM(("wl%d: %s: samples rssi {%d %d} avg %d qa %d\n",
		wlc->pub->unit, __FUNCTION__,
		wlc->channel_qa_sample[0], wlc->channel_qa_sample[1],
		rssi_avg, quality_metric));

	return (quality_metric);
}

/* this callback chain must defer calling phy_noise_sample_request */
static void
wlc_lq_channel_qa_sample_cb(wlc_info_t *wlc, uint8 channel, int8 noise_dbm)
{
	bool moretest = FALSE;

	if (!wlc->channel_qa_active)
		return;

	if (channel != wlc->channel_qa_channel) {
		/* bad channel, try again */
		WL_INFORM(("wl%d: %s: retry, samples from channel %d instead of channel %d\n",
		           wlc->pub->unit, __FUNCTION__, channel, wlc->channel_qa_channel));
		moretest = TRUE;
	} else {
		/* save the sample */
		wlc->channel_qa_sample[wlc->channel_qa_sample_num++] = (int8)noise_dbm;
		if (wlc->channel_qa_sample_num < WLC_CHANNEL_QA_NSAMP) {
			/* still need more samples */
			moretest = TRUE;
		} else {
			/* done with the channel quality measurement */
			wlc->channel_qa_active = FALSE;

			/* evaluate the samples to a quality metric */
			wlc->channel_quality = wlc_lq_channel_qa_eval(wlc);
		}
	}

	if (moretest)
		wlc_lq_channel_qa_sample_req(wlc);

}

int
wlc_lq_channel_qa_start(wlc_info_t *wlc)
{
	/* do nothing if there is already a request for a measurement */
	if (wlc->channel_qa_active)
		return 0;

	WL_INFORM(("wl%d: %s: starting qa measure\n", wlc->pub->unit, __FUNCTION__));

	wlc->channel_qa_active = TRUE;

	wlc->channel_quality = -1;	/* clear to invalid value */
	wlc->channel_qa_sample_num = 0;	/* clear the sample array */

	wlc_lq_channel_qa_sample_req(wlc);

	return 0;
}

void
wlc_lq_channel_qa_sample_req(wlc_info_t *wlc)
{
	/* wait until after a scan if one is in progress */
	if (SCAN_IN_PROGRESS(wlc->scan)) {
		WL_NONE(("wl%d: %s: deferring sample request until after scan\n", wlc->pub->unit,
			__FUNCTION__));
		return;
	}

	wlc->channel_qa_channel = CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC);

	WL_NONE(("wl%d: %s(): requesting samples for channel %d\n", wlc->pub->unit,
	         __FUNCTION__, wlc->channel_qa_channel));

	WL_INFORM(("wlc_noise_cb(): WLC_NOISE_REQUEST_CQ.\n"));

	wlc_lq_noise_sample_request(wlc, WLC_NOISE_REQUEST_CQ, wlc->channel_qa_channel);

}
#endif /* defined(WLCQ)  */

void
wlc_lq_noise_cb(wlc_info_t *wlc, uint8 channel, int8 noise_dbm)
{
	if (wlc->noise_req & WLC_NOISE_REQUEST_SCAN) {
		if (wlc->phynoise_chan_scan == channel)
			wlc->phy_noise_list[channel] = noise_dbm;

		/* TODO - probe responses may have been constructed, fixup those dummy values
		 *  if being blocked by CQRM sampling at different channels, make another request
		 *     if we are still in the requested scan channel and scan hasn't finished yet
		 */
		wlc->noise_req &= ~WLC_NOISE_REQUEST_SCAN;
	}

#ifdef WLCQ
	if (wlc->noise_req & WLC_NOISE_REQUEST_CQ) {
		wlc->noise_req &= ~WLC_NOISE_REQUEST_CQ;
		WL_INFORM(("wlc_noise_cb(): WLC_NOISE_REQUEST_CQ.\n"));
		wlc_lq_channel_qa_sample_cb(wlc, channel, noise_dbm);
	}
#endif // endif

#if defined(STA) && defined(WLRM)
	if (wlc->noise_req & WLC_NOISE_REQUEST_RM) {
		wlc->noise_req &= ~WLC_NOISE_REQUEST_RM;
		WL_INFORM(("wlc_noise_cb(): WLC_NOISE_REQUEST_RM.\n"));
		if (WLRM_ENAB(wlc->pub) && wlc->rm_info->rm_state->rpi_active) {
			if (wlc_rm_rpi_sample(wlc->rm_info, noise_dbm))
				wlc_lq_noise_sample_request(wlc, WLC_NOISE_REQUEST_RM,
				                       CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC));
		}
	}
#endif // endif

	return;

}

void
wlc_lq_noise_sample_request(wlc_info_t *wlc, uint8 request, uint8 channel)
{
	bool sampling_in_progress = (wlc->noise_req != 0);

	WL_TRACE(("%s(): request=%d, channel=%d\n", __FUNCTION__, request, channel));

	switch (request) {
	case WLC_NOISE_REQUEST_SCAN:

		/* fill in dummy value in case the sampling failed or channel mismatch */
		wlc->phy_noise_list[channel] = PHY_NOISE_FIXED_VAL_NPHY;
		wlc->phynoise_chan_scan = channel;

		wlc->noise_req |= WLC_NOISE_REQUEST_SCAN;
		break;

	case WLC_NOISE_REQUEST_CQ:

		wlc->noise_req |= WLC_NOISE_REQUEST_CQ;
		break;

	case WLC_NOISE_REQUEST_RM:

		wlc->noise_req |= WLC_NOISE_REQUEST_RM;
		break;

	default:
		ASSERT(0);
		break;
	}

	if (sampling_in_progress)
		return;

	wlc_phy_noise_sample_request_external(WLC_PI(wlc));

	return;
}

#ifdef BCMDBG
static int
wlc_dump_lq(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	bcm_bprintf(b, "LQ dump:\n");
	return BCME_OK;
}
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(WLTEST)
static int
wlc_dump_rssi(wlc_info_t *wlc, struct bcmstrbuf * b)
{
	uint32 i, idx, antidx, max_ant;
	int32 tot;

	if (!wlc->pub->up)
		return BCME_NOTUP;

	bcm_bprintf(b, "History and average of latest %d RSSI values:\n", WLC_RSSI_WINDOW_SZ);

	max_ant = (uint32)((WLCISHTPHY(wlc->band) || WLCISACPHY(wlc->band)) ?
	    WL_ANT_HT_RX_MAX : WL_ANT_RX_MAX);
	for (antidx = WL_ANT_IDX_1; antidx < max_ant; antidx++) {
		if (((wlc->stf->rxchain >> antidx) & 1) == 0)
			continue;
		tot = 0;
		bcm_bprintf(b, "Ant%d: [", antidx);

		idx = wlc->rssi_win_rfchain_idx;
		for (i = 0; i < WLC_RSSI_WINDOW_SZ; i++) {
			bcm_bprintf(b, "%3d ", wlc->rssi_win_rfchain[antidx][idx]);
			tot += wlc->rssi_win_rfchain[antidx][idx];
			idx = MODINC_POW2(idx, WLC_RSSI_WINDOW_SZ);
		}
		bcm_bprintf(b, "]");

		tot /= WLC_RSSI_WINDOW_SZ;
		bcm_bprintf(b, "avg [%4d]\n", tot);
	}

	return 0;
}
#endif /* BCMDBG || BCMDBG_DUMP || WLTEST */

#ifdef WLCHANIM

#ifdef WLCHANIM_US
static wlc_chanim_stats_us_t *
wlc_lq_chanim_create_stats_us(wlc_info_t *wlc, chanspec_t chanspec)
{
	wlc_chanim_stats_us_t *new_stats = NULL;
	chanspec_t ctl_chanspec;

	/* if the chanspec passed is malformed or Zero avoid allocation of memory */
	if (chanspec == 0 || wf_chspec_malformed(chanspec)) {
		return NULL;
	}

	new_stats = (wlc_chanim_stats_us_t *) MALLOCZ(wlc->osh, sizeof(wlc_chanim_stats_us_t));

	if (!new_stats) {
		WL_ERROR(("wl%d: %s: out of memory %d bytes\n",
			wlc->pub->unit, __FUNCTION__, (uint)sizeof(wlc_chanim_stats_us_t)));
	}
	else {
		memset(new_stats, 0, sizeof(*new_stats));
		ctl_chanspec = wf_chspec_ctlchspec(chanspec);
		new_stats->chanim_stats_us.chanspec = ctl_chanspec;
		new_stats->next = NULL;
	}
	return new_stats;
}

static void
wlc_lq_chanim_insert_stats_us(wlc_chanim_stats_us_t **rootp, wlc_chanim_stats_us_t *new)
{
	wlc_chanim_stats_us_t *curptr;
	wlc_chanim_stats_us_t *previous;

	curptr = *rootp;
	previous = NULL;

	while (curptr &&
		(curptr->chanim_stats_us.chanspec < new->chanim_stats_us.chanspec)) {
		previous = curptr;
		curptr = curptr->next;
	}
	new->next = curptr;

	if (previous == NULL) {
		*rootp = new;
	} else {
		previous->next = new;
	}
}

static wlc_chanim_stats_us_t *
wlc_lq_chanim_chanspec_to_stats_us(chanim_info_t *c_info, chanspec_t chanspec)
{
	chanspec_t ctl_chanspec;
	wlc_chanim_stats_us_t *cur_stats_us = c_info->stats_us;
	chanim_interface_info_t *if_info = NULL;

	/* For quicker access, look in cache first. Otherwise, walk the list. */
	if ((if_info = wlc_lq_chanim_if_info_find(c_info->ifs, chanspec)) != NULL) {
		if (if_info->stats_us != NULL &&
				if_info->stats_us->chanim_stats_us.chanspec == chanspec) {
			return if_info->stats_us;
		}
	}

	ctl_chanspec = wf_chspec_ctlchspec(chanspec);
	while (cur_stats_us) {
		if (cur_stats_us->chanim_stats_us.chanspec == ctl_chanspec) {
			return cur_stats_us;
		}
		cur_stats_us = cur_stats_us->next;
	}
	return cur_stats_us;
}

static wlc_chanim_stats_us_t *
wlc_lq_chanim_find_stats_us(wlc_info_t *wlc, chanspec_t chanspec)
{
	wlc_chanim_stats_us_t *stats_us = NULL;
	chanim_info_t *c_info = wlc->chanim_info;
	stats_us = wlc_lq_chanim_chanspec_to_stats_us(c_info, chanspec);

	if (!stats_us) {
		stats_us = wlc_lq_chanim_create_stats_us(wlc, chanspec);
		if (stats_us) {
			wlc_lq_chanim_insert_stats_us(&c_info->stats_us, stats_us);
		}
	}

	return stats_us;
}

static void
wlc_lq_chanim_us_accum(chanim_info_t* c_info, chanim_cnt_us_t *cur_cnt_us, chanim_acc_us_t *acc_us)
{
	int i;
	uint32 ccastats_us_delta = 0;
	chanim_cnt_us_t *last_cnt_us;

	last_cnt_us = &c_info->last_cnt_us;

	for (i = 0; i < CCASTATS_MAX; i++) {
		if (last_cnt_us->ccastats_cnt[i] || acc_us->chanspec) {
			ccastats_us_delta = cur_cnt_us->ccastats_cnt[i] -
				last_cnt_us->ccastats_cnt[i];
			acc_us->ccastats_us[i] += ccastats_us_delta;
		}
		last_cnt_us->ccastats_cnt[i] = cur_cnt_us->ccastats_cnt[i];
	}
	ccastats_us_delta = cur_cnt_us->busy_tm - last_cnt_us->busy_tm;
	acc_us->busy_tm += ccastats_us_delta;
	last_cnt_us->busy_tm =  cur_cnt_us->busy_tm;

	ccastats_us_delta = cur_cnt_us->rxcrs_pri20 - last_cnt_us->rxcrs_pri20;
	acc_us->rxcrs_pri20 += ccastats_us_delta;
	last_cnt_us->rxcrs_pri20 =  cur_cnt_us->rxcrs_pri20;

	ccastats_us_delta = cur_cnt_us->rxcrs_sec20 - last_cnt_us->rxcrs_sec20;
	acc_us->rxcrs_sec20 += ccastats_us_delta;
	last_cnt_us->rxcrs_sec20 =  cur_cnt_us->rxcrs_sec20;

	ccastats_us_delta = cur_cnt_us->rxcrs_sec40 - last_cnt_us->rxcrs_sec40;
	acc_us->rxcrs_sec40 += ccastats_us_delta;
	last_cnt_us->rxcrs_sec40 =  cur_cnt_us->rxcrs_sec40;
}
#endif /* WLCHANIM_US */

static wlc_chanim_stats_t *
wlc_lq_chanim_create_stats(wlc_info_t *wlc, chanspec_t chanspec)
{

	wlc_chanim_stats_t *new_stats = NULL;
	chanspec_t ctl_chanspec;

	/* if the chanspec passed is malformed or Zero avoid allocation of memory */
	if (chanspec == 0 || wf_chspec_malformed(chanspec)) {
		return NULL;
	}

	new_stats = (wlc_chanim_stats_t *) MALLOCZ(wlc->osh, sizeof(wlc_chanim_stats_t));

	if (!new_stats) {
		WL_ERROR(("wl%d: %s: out of memory %d bytes\n",
			wlc->pub->unit, __FUNCTION__, (uint)sizeof(wlc_chanim_stats_t)));
	}
	else {
		memset(new_stats, 0, sizeof(*new_stats));
		ctl_chanspec = wf_chspec_ctlchspec(chanspec);
		new_stats->chanim_stats.chanspec = ctl_chanspec;
		new_stats->next = NULL;
	}
	return new_stats;
}

static void
wlc_lq_chanim_insert_stats(wlc_chanim_stats_t **rootp, wlc_chanim_stats_t *new)
{
	wlc_chanim_stats_t *curptr;
	wlc_chanim_stats_t *previous;

	curptr = *rootp;
	previous = NULL;

	while (curptr &&
		(curptr->chanim_stats.chanspec < new->chanim_stats.chanspec)) {
		previous = curptr;
		curptr = curptr->next;
	}
	new->next = curptr;

	if (previous == NULL)
		*rootp = new;
	else
		previous->next = new;
}

wlc_chanim_stats_t *
wlc_lq_chanim_chanspec_to_stats(chanim_info_t *c_info, chanspec_t chanspec, bool scan_param)
{
	chanspec_t ctl_chanspec;
	wlc_chanim_stats_t *cur_stats = c_info->stats;
	chanim_interface_info_t *if_info = NULL;

	if (!scan_param) {
		/* For quicker access, look in cache first. Otherwise, walk the list. */
		if ((if_info = wlc_lq_chanim_if_info_find(c_info->ifs, chanspec)) != NULL) {

			return if_info->stats;
		}
	}

	ctl_chanspec = wf_chspec_ctlchspec(chanspec);
	while (cur_stats) {
		if (cur_stats->chanim_stats.chanspec == ctl_chanspec)
			return cur_stats;
		cur_stats = cur_stats->next;
	}
	return cur_stats;
}

static wlc_chanim_stats_t *
wlc_lq_chanim_find_stats(wlc_info_t *wlc, chanspec_t chanspec)
{
	wlc_chanim_stats_t *stats = NULL;
	chanim_info_t *c_info = wlc->chanim_info;

	if (SCAN_IN_PROGRESS(wlc->scan)) {
		stats = wlc_lq_chanim_chanspec_to_stats(c_info, chanspec, TRUE);

		if (!stats) {
			stats = wlc_lq_chanim_create_stats(wlc, chanspec);
			if (stats) {
				wlc_lq_chanim_insert_stats(&c_info->stats, stats);
			}
		}
	} else {
		stats = &c_info->cur_stats;
		stats->chanim_stats.chanspec = chanspec;
		stats->next = NULL;
	}
	return stats;
}

static void
wlc_lq_chanim_meas(wlc_info_t *wlc, chanim_cnt_t *chanim_cnt, chanim_cnt_us_t *chanim_cnt_us)
{
	uint16 rxcrsglitch = 0;
	uint16 rxbadplcp = 0;
	int i;

	/* Read rxcrsglitch count from shared memory */
	rxcrsglitch = wlc_bmac_read_shm(wlc->hw,
		MACSTAT_ADDR(MCSTOFF_RXCRSGLITCH));
	chanim_cnt->glitch_cnt = rxcrsglitch;

	chanim_cnt->bphy_glitch_cnt = wlc_bmac_read_shm(wlc->hw,
		MACSTAT_ADDR(MCSTOFF_BPHYGLITCH));

	rxbadplcp = wlc_bmac_read_shm(wlc->hw,
		MACSTAT_ADDR(MCSTOFF_RXBADPLCP));
	chanim_cnt->badplcp_cnt = rxbadplcp;

	chanim_cnt->bphy_badplcp_cnt = wlc_bmac_read_shm(wlc->hw,
		MACSTAT_ADDR(MCSTOFF_BPHY_BADPLCP));
#ifdef WLCHANIM_US
	wlc_bmaq_lq_stats_read(wlc->hw, chanim_cnt_us);
#endif /* WLCHANIM_US */
	chanim_cnt_us->busy_tm = 0;
	if (WLC_CCASTATS_CAP(wlc)) {
		for (i = 0; i < CCASTATS_MAX; i++) {
			chanim_cnt->ccastats_cnt[i] =
				wlc_bmac_cca_read_counter(wlc->hw, 4 * i, (4 * i + 2));
#ifdef WLCHANIM_US
			chanim_cnt_us->ccastats_cnt[i] =
				wlc_bmac_cca_read_counter(wlc->hw, 4 * i, (4 * i + 2));
			if (i > 0 && i <= 4) {
				chanim_cnt_us->busy_tm += chanim_cnt_us->ccastats_cnt[i];
			}
#endif /* WLCHANIM_US */
		}
	}
	chanim_cnt->ccastats_cnt[CCASTATS_NOPKT] += wlc_bmac_cca_read_counter(wlc->hw,
			M_CCA_WIFI_L, M_CCA_WIFI_H);
	chanim_cnt->timestamp = OSL_SYSUPTIME();
}

static void
wlc_lq_chanim_glitch_accum(chanim_info_t* c_info, chanim_cnt_t *cur_cnt, chanim_accum_t *acc)
{
	uint16 glitch_delta = 0;
	uint16 bphy_glitch_delta = 0;
	chanim_cnt_t *last_cnt;

	last_cnt = &c_info->last_cnt;

	/* The statistics glitch_delta are computed when there is a non zero value of
	   last_cnt->glitch_cnt. Bphy statistics are also being updated here because,
	   last_cnt->glitch_cnt is the sum of both OFDM and BPHY glitch counts.
	   So, if there is a non zero value of total glitch count, it is a good idea
	   to update both OFDM and BPHY glitch counts.
	 */
	if (last_cnt->glitch_cnt || acc->chanspec) {
		glitch_delta = cur_cnt->glitch_cnt - last_cnt->glitch_cnt;
		bphy_glitch_delta = cur_cnt->bphy_glitch_cnt - last_cnt->bphy_glitch_cnt;
		acc->glitch_cnt += glitch_delta;
		acc->bphy_glitch_cnt += bphy_glitch_delta;
	}
	last_cnt->glitch_cnt = cur_cnt->glitch_cnt;
	last_cnt->bphy_glitch_cnt = cur_cnt->bphy_glitch_cnt;
}

static void
wlc_lq_chanim_badplcp_accum(chanim_info_t* c_info, chanim_cnt_t *cur_cnt, chanim_accum_t *acc)
{
	uint16 badplcp_delta = 0;
	uint16 bphy_badplcp_delta = 0;
	chanim_cnt_t *last_cnt;

	last_cnt = &c_info->last_cnt;

	/* The statistics badplcp_delta are computed when there is a non zero value of
	   last_cnt->badplcp_cnt. Bphy statistics are also being updated here because,
	   last_cnt->badplcp_cnt is the sum of both OFDM and BPHY badplcp counts.
	   So, if there is a non zero value of total badplcp count, it is a good idea
	   to update both OFDM and BPHY badplcp counts.
	 */
	if (last_cnt->badplcp_cnt) {
		badplcp_delta = cur_cnt->badplcp_cnt - last_cnt->badplcp_cnt;
		bphy_badplcp_delta = cur_cnt->bphy_badplcp_cnt - last_cnt->bphy_badplcp_cnt;
		acc->badplcp_cnt += badplcp_delta;
		acc->bphy_badplcp_cnt += bphy_badplcp_delta;
	}
	last_cnt->badplcp_cnt = cur_cnt->badplcp_cnt;
	last_cnt->bphy_badplcp_cnt = cur_cnt->bphy_badplcp_cnt;
}

static void
wlc_lq_chanim_ccastats_accum(chanim_info_t* c_info, chanim_cnt_t *cur_cnt, chanim_accum_t *acc)
{
	int i;
	uint32 ccastats_delta = 0;
	chanim_cnt_t *last_cnt;

	last_cnt = &c_info->last_cnt;

	for (i = 0; i < CCASTATS_MAX; i++) {
		if (last_cnt->ccastats_cnt[i] || acc->chanspec) {
			ccastats_delta = cur_cnt->ccastats_cnt[i] - last_cnt->ccastats_cnt[i];
			acc->ccastats_us[i] += ccastats_delta;
		}
		last_cnt->ccastats_cnt[i] = cur_cnt->ccastats_cnt[i];
	}
}

/*
 * based on current read, accumulate the count
 * also, update the last
 */
static void
wlc_lq_chanim_accum(wlc_info_t* wlc, chanspec_t chanspec, chanim_accum_t *acc,
	chanim_acc_us_t *acc_us)
{
	chanim_info_t* c_info = wlc->chanim_info;
	chanim_cnt_t cur_cnt, *last_cnt;
	chanim_cnt_us_t cur_cnt_us;
	uint cur_time;
	uint interval = 0;

	/* read the current measurement counters */
	wlc_lq_chanim_meas(wlc, &cur_cnt, &cur_cnt_us);

	last_cnt = &c_info->last_cnt;
	cur_time = OSL_SYSUPTIME();
	if (last_cnt->timestamp) {
		if (c_info->chanswitch_overhead && !SCAN_IN_PROGRESS(wlc->scan)) {
			interval = (cur_time - last_cnt->timestamp) -
				(c_info->chanswitch_overhead/1000);
			c_info->chanswitch_overhead = 0;
		} else {
			interval = cur_time - last_cnt->timestamp;
		}
	}

	/* update the accumulator with current deltas */
	wlc_lq_chanim_glitch_accum(c_info, &cur_cnt, acc);
	wlc_lq_chanim_badplcp_accum(c_info, &cur_cnt, acc);
	if (WLC_CCASTATS_CAP(wlc)) {
		wlc_lq_chanim_ccastats_accum(c_info, &cur_cnt, acc);
	}
#ifdef WLCHANIM_US
	wlc_lq_chanim_us_accum(c_info, &cur_cnt_us, acc_us);
	acc_us->chanspec = chanspec;
#endif /* WLCHANIM_US */
	last_cnt->timestamp = cur_time;
	acc->stats_ms += interval;
	acc->chanspec = chanspec;
}

static void
wlc_lq_chanim_clear_acc(wlc_info_t* wlc, chanim_accum_t* acc, chanim_acc_us_t* acc_us,
	bool chan_switch)
{
	int i;

	if (acc) {
		acc->glitch_cnt = 0;
		acc->badplcp_cnt = 0;
		acc->bphy_glitch_cnt = 0;
		acc->bphy_badplcp_cnt = 0;

		if (WLC_CCASTATS_CAP(wlc))
			for (i = 0; i < CCASTATS_MAX; i++)
				acc->ccastats_us[i] = 0;

		acc->stats_ms = 0;
	}
#ifdef WLCHANIM_US
	if (acc_us && (chan_switch == TRUE)) {
		acc_us->busy_tm = 0;
		acc_us->rxcrs_pri20 = 0;
		acc_us->rxcrs_sec20 = 0;
		acc_us->rxcrs_sec40 = 0;

		if (WLC_CCASTATS_CAP(wlc)) {
			for (i = 0; i < CCASTATS_MAX; i++) {
				acc_us->ccastats_us[i] = 0;
			}
		}
	}
#endif /* WLCHANIM_US */
}

static int8
wlc_lq_chanim_phy_noise(wlc_info_t *wlc)
{

	int32 rxiq = 0;
	int8 result = 0;
	int err = 0;

#if CORE4 >= 4	/* used for report 4 core rx_iq_est */
	int16 rxiq_buff[4] = {0};
#endif // endif
	if (!WLCISSSLPNPHY(wlc->band) && !WLCISAPHY(wlc->band) &&
		!WLCISLCNPHY(wlc->band) && SCAN_IN_PROGRESS(wlc->scan)) {
		int cnt = 10, valid_cnt = 0;
		int i;
		int sum = 0;

		rxiq = 10 << 8 | 3; /* default: samples = 1024 (2^10) and antenna = 3 */

#if CORE4 >= 4	/* used for report 4 core rx_iq_est */
		rxiq_buff[0] = (int16)(rxiq & 0xffff);
		rxiq_buff[1] = (rxiq >> 16) & 0xffff;

		/* iovar set */
		if ((err = wlc_iovar_op(wlc, "phy_rxiqest", NULL, 0, (void *)&rxiq_buff,
			sizeof(rxiq_buff), IOV_SET, NULL)) < 0) {
			WL_ERROR(("failed to set phy_rxiqest\n"));
			return err;
		}
#else
		if ((err = wlc_iovar_setint(wlc, "phy_rxiqest", rxiq)) < 0) {
			WL_ERROR(("failed to set phy_rxiqest\n"));
			return err;
		}
#endif /* CORE4 */

		for (i =  0; i < cnt; i++) { /* iovar get */
			rxiq_buff[0] = rxiq_buff[1] = rxiq_buff[2] = rxiq_buff[3] = 0;
			rxiq = 0;
#if CORE4 >= 4	/* used for report 4 core rx_iq_est */

			if ((err = wlc_iovar_op(wlc, "phy_rxiqest", NULL, 0, (void *)&rxiq_buff,
				sizeof(rxiq_buff), IOV_GET, NULL)) < 0) {
				WL_ERROR(("failed to get phy_rxiqest\n"));
				return err;
			}
			/* use the last byte to compute the bgnoise estimation
			 * phy_rxiqest returns values in dBm (negative number).
			 * We require only some portion of the values as determined
			 * by the last byte.
			 */
			rxiq_buff[1] &= 0xff;
			rxiq_buff[0] &= 0xff;
			rxiq = (int32)(rxiq_buff[1]<<16) + (int32)rxiq_buff[0];
#else

			if ((err = wlc_iovar_getint(wlc, "phy_rxiqest", &rxiq)) < 0) {
				WL_ERROR(("failed to get phy_rxiqest\n"));
				return err;
			}
#endif /* CORE4 */
			if (rxiq >> 8)
				result = (int8)MAX((rxiq >> 8) & 0xff, (rxiq & 0xff));
			else
				result = (int8)(rxiq & 0xff);

			if (result) {
				sum += result;
				valid_cnt++;
			}
		}

		if (valid_cnt)
			result = sum/valid_cnt;
	}

	if (!SCAN_IN_PROGRESS(wlc->scan))
		result = wlc_phy_noise_avg(WLC_PI(wlc));

	WL_CHANINT(("bgnoise: %d dBm\n", result));

	return result;
}

/*
 * convert the stats from the accumulative counters to the final stats
 * also clear the accumulative counter.
 */
static void
wlc_lq_chanim_close(wlc_info_t* wlc, chanspec_t chanspec, chanim_accum_t* acc,
	wlc_chanim_stats_t *cur_stats, chanim_acc_us_t *acc_us,
	wlc_chanim_stats_us_t *cur_stats_us, bool chan_switch)
{
	int i;
	uint8 stats_frac = 0;
	int32 aci_chan_vld_dur;
#ifdef WLCHANIM_US
	chanim_info_t* c_info = wlc->chanim_info;
	chanim_cnt_us_t *last_cnt_us;
#endif /* WLCHANIM_US */
	// Don't include valid TX & RX in idle time, as that bloats the glitches un-necesarily
	// raising desense. Only use doze(sleep time) and stats_ms(in case its doing p2p)
	aci_chan_vld_dur = (int32) ((acc->stats_ms * 1000) - acc->ccastats_us[CCASTATS_DOZE] + 500)
		 / 1000;

	/* normalized to per second count */
	if ((acc->stats_ms) && (aci_chan_vld_dur > 0)) {

		cur_stats->chanim_stats.glitchcnt = acc->glitch_cnt * 1000 / aci_chan_vld_dur;

		cur_stats->chanim_stats.bphy_glitchcnt = acc->bphy_glitch_cnt *
			1000 / aci_chan_vld_dur;

		cur_stats->chanim_stats.badplcp = acc->badplcp_cnt * 1000 / aci_chan_vld_dur;

		cur_stats->chanim_stats.bphy_badplcp = acc->bphy_badplcp_cnt *
		        1000 / aci_chan_vld_dur;

		cur_stats->is_valid = TRUE;
	} else {
		cur_stats->chanim_stats.glitchcnt = 0;

		cur_stats->chanim_stats.bphy_glitchcnt = 0;

		cur_stats->chanim_stats.badplcp = 0;

		cur_stats->chanim_stats.bphy_badplcp = 0;

		cur_stats->is_valid = FALSE;
	}

	if (WLC_CCASTATS_CAP(wlc)) {
		uint txop_us = 0;
		uint slottime = APHY_SLOT_TIME;
		uint txop = 0, txop_nom = 0;
		uint8 txop_percent = 0;

		if (wlc->band->gmode && !wlc->shortslot)
			slottime = BPHY_SLOT_TIME;

		for (i = 0; i < CCASTATS_MAX; i++) {
			/* normalize to be 0-100 */

			if (acc->stats_ms) {
				if (i == CCASTATS_TXOP)
					stats_frac = (uint8)CEIL(acc->ccastats_us[i] * slottime,
					  acc->stats_ms * 10);
				else
					stats_frac = (uint8)CEIL(100 * acc->ccastats_us[i],
					  acc->stats_ms * 1000);
			}

			if (stats_frac > 100) {
				WL_INFORM(("stats(%d) > 100: ccastats_us: %d, acc->statss_ms: %d\n",
					stats_frac, acc->ccastats_us[i], acc->stats_ms));
				stats_frac = 100;
			}
			cur_stats->chanim_stats.ccastats[i] = stats_frac;
		}

		/* calc chan_idle */
		txop_us = (acc->stats_ms * 1000) - acc->ccastats_us[CCASTATS_DOZE];
		txop_nom = txop_us / slottime;
		txop = acc->ccastats_us[CCASTATS_TXOP] +
			(acc->ccastats_us[CCASTATS_TXDUR] -
			acc->ccastats_us[CCASTATS_BDTXDUR]) / slottime;
		if (txop_nom) {
			 txop_percent = (uint8)CEIL(100 * txop, txop_nom);
			 txop_percent = MIN(100, txop_percent);
		}
		cur_stats->chanim_stats.chan_idle = txop_percent;

	}
	cur_stats->chanim_stats.bgnoise = wlc_lq_chanim_phy_noise(wlc);

	cur_stats->chanim_stats.timestamp = OSL_SYSUPTIME();
#ifdef WLCHANIM_US
	last_cnt_us = &c_info->last_cnt_us;
	cur_stats_us->chanim_stats_us.total_tm = ((OSL_SYSUPTIME() -
		last_cnt_us->start_tm)*1000) +
		last_cnt_us->total_tm;
	cur_stats_us->chanim_stats_us.busy_tm = acc_us->busy_tm;
	for (i = 0; i < CCASTATS_MAX; i++) {
		cur_stats_us->chanim_stats_us.ccastats_us[i] = acc_us->ccastats_us[i];
	}

	cur_stats_us->chanim_stats_us.rxcrs_pri20 = acc_us->rxcrs_pri20;
	cur_stats_us->chanim_stats_us.rxcrs_sec20 = acc_us->rxcrs_sec20;
	cur_stats_us->chanim_stats_us.rxcrs_sec40 = acc_us->rxcrs_sec40;
	if (chan_switch == TRUE) {
		last_cnt_us->start_tm = 0;
	}
	if (SCAN_IN_PROGRESS(wlc->scan) && (chanspec == wlc->home_chanspec))
	{
		chanim_interface_info_t *if_info =
			wlc_lq_chanim_if_info_find(wlc->chanim_info->ifs, chanspec);
		if (if_info) {
			memcpy(if_info->acc_us, acc_us, sizeof(chanim_acc_us_t));
		}
	}
#endif /* WLCHANIM_US */
	wlc_lq_chanim_clear_acc(wlc, acc, acc_us, chan_switch);
}

#ifdef BCMDBG
static int
wlc_lq_chanim_display(wlc_info_t *wlc, chanspec_t chanspec, wlc_chanim_stats_t *cur_stats)
{
	chanim_info_t *c_info = wlc->chanim_info;

	if (!cur_stats)
		return BCME_ERROR;

	BCM_REFERENCE(c_info);

	WL_CHANINT(("**intf: %d glitch cnt: %d badplcp: %d noise: %d chanspec: 0x%x \n",
		chanim_mark(c_info).state, cur_stats->chanim_stats.glitchcnt,
		cur_stats->chanim_stats.badplcp, cur_stats->chanim_stats.bgnoise, chanspec));

	if (WLC_CCASTATS_CAP(wlc)) {
		WL_CHANINT(("***cca stats: txdur: %d, inbss: %d, obss: %d,"
		  "nocat: %d, nopkt: %d, doze: %d\n",
		  cur_stats->chanim_stats.ccastats[CCASTATS_TXDUR],
		  cur_stats->chanim_stats.ccastats[CCASTATS_INBSS],
		  cur_stats->chanim_stats.ccastats[CCASTATS_OBSS],
		  cur_stats->chanim_stats.ccastats[CCASTATS_NOCTG],
		  cur_stats->chanim_stats.ccastats[CCASTATS_NOPKT],
		  cur_stats->chanim_stats.ccastats[CCASTATS_DOZE]));
	}
	return BCME_OK;
}
#endif /* BCMDBG */

/*
 * Given a chanspec, find the matching interface info. If there isn't a match, then
 * find an empty slot. A reference count is incremented if this is called from a
 * BSSCFG callback.
 */
static chanim_interface_info_t *
wlc_lq_chanim_if_info_setup(chanim_info_t *c_info, chanspec_t chanspec, bool frm_bsscfg_cb)
{
	wlc_info_t *wlc = c_info->wlc;
	chanim_interfaces_t *ifaces = c_info->ifs;
	chanim_interface_info_t *if_info = NULL;
	int i;

	/* Find an existing entry with a matching chanspec */
	for (i = 0; i < ifaces->num_ifs; i++) {
		if (chanspec == ifaces->if_info[i].chanspec) {
			if_info = &ifaces->if_info[i];	/* Found one */
			break;
		}
	}

	if (if_info == NULL) {
		/* Not found. Find an empty slot */
		for (i = 0; i < ifaces->num_ifs; i++) {
			if (ifaces->if_info[i].chanspec == 0) {
				/* Found one */
				if_info = &ifaces->if_info[i];
				memset(if_info->accum, 0, sizeof(chanim_accum_t));
				/* Get stats for channel (cached) */
				if_info->stats = wlc_lq_chanim_find_stats(wlc, chanspec);
				if_info->ref_cnt = 0;
				if_info->chanspec = chanspec;
				ASSERT(if_info->stats != NULL);
#ifdef WLCHANIM_US
				if_info->stats_us = wlc_lq_chanim_find_stats_us(wlc, chanspec);
				ASSERT(if_info->stats_us != NULL);
#endif /* WLCHANIM_US */
				break;
			}
		}
	}

	ASSERT(!MCHAN_ENAB(wlc->pub) || if_info != NULL);

	if (if_info && frm_bsscfg_cb) {
		/* Only increment ref count if called from BSS cfg up. */
		if_info->ref_cnt++;
	}

	return if_info;
}

/*
 * Given a chanspec, release the interface. NOTE: it will only free if reference
 * count is zero. A reference count is decremented if this is called from a BSSCFG
 * callback.
 */
static void
wlc_lq_chanim_if_info_release(chanim_info_t *c_info, chanspec_t chanspec, bool frm_bsscfg_cb)
{
	chanim_interfaces_t *ifaces = c_info->ifs;
	chanim_interface_info_t *if_info = NULL;
	int i;

	for (i = 0; i < ifaces->num_ifs; i++) {
		if (ifaces->if_info[i].chanspec == chanspec) {
			if_info = &ifaces->if_info[i];	/* Found match */
			break;
		}
	}

	if (if_info) {
		if (frm_bsscfg_cb) {
			if_info->ref_cnt--;
		}

		/* Till all the interfaces which are refering the same
		 * stats info are not down the if_info is not reset
		 */
		if (if_info->ref_cnt == 0) {
			if_info->stats = NULL;
			if_info->chanspec = 0;
		}
	}
}

static chanim_interface_info_t *
wlc_lq_chanim_if_info_find(chanim_interfaces_t *ifaces, chanspec_t chanspec)
{
	chanim_interface_info_t *if_info = NULL;
	int i;

	for (i = 0; i < ifaces->num_ifs; i++) {
		if (ifaces->if_info[i].chanspec == chanspec) {
			if_info = &ifaces->if_info[i];	/* Found match */
			break;
		}
	}

	return if_info;
}
#if defined(WLMCHAN) && !defined(WLMCHAN_DISABLED)
/*
 * When trying to find a replacement slot in multichannel case, the adopt
 * hook may use a chanspec that is a different bandwidth. In this case, calling
 * the above lookup will not find an exact match. Search based on control
 * channel instead.
 */
static chanim_interface_info_t *
wlc_lq_chanim_if_info_find_ctl(chanim_interfaces_t *ifaces, chanspec_t chanspec)
{
	chanim_interface_info_t *if_info = NULL;
	int i;
	chanspec_t ctl_chanspec = wf_chspec_ctlchan(chanspec);

	for (i = 0; i < ifaces->num_ifs; i++) {
		if (ifaces->if_info[i].chanspec != 0) {
			if (wf_chspec_ctlchan(ifaces->if_info[i].chanspec) == ctl_chanspec) {
				if_info = &ifaces->if_info[i];	/* Found match */
				break;
			}
		}
	}
	return if_info;
}
#endif /* WLMCHAN && !WLMCHAN_DISABLED */
/*
 * This is called when an i/f is changing a channel. In this case, it accumulate/close
 * the old channel and re-purpose the accumulator for the new channel.
 */
static void
wlc_lq_chanim_if_switch_channels(wlc_info_t *wlc,
                                 chanim_interface_info_t *if_info,
                                 chanspec_t chanspec,
                                 chanspec_t prev_chanspec)
{
#ifdef WLCHANIM_US
	chanim_info_t* c_info = wlc->chanim_info;
	chanim_cnt_us_t *last_cnt_us;
	int i;
#endif /* WLCHANIM_US */
	ASSERT(if_info != NULL);

#ifdef WLCHANIM_US
	last_cnt_us = &c_info->last_cnt_us;
#endif /* WLCHANIM_US */
	/*
	 * NOTE: the new chanspec is passed in because it will overwrite the chanspec field with
	 * this value.
	 */
	wlc_lq_chanim_accum(wlc, chanspec, if_info->accum, if_info->acc_us);

	/* Close the previous channel. */
	wlc_lq_chanim_close(wlc, prev_chanspec, if_info->accum, if_info->stats,
		if_info->acc_us, if_info->stats_us, TRUE);
#ifdef BCMDBG
	if (wlc_lq_chanim_display(wlc, prev_chanspec, if_info->stats) != BCME_OK) {
		WL_ERROR(("wl%d: %s: if_info->stats is NULL\n", wlc->pub->unit, __FUNCTION__));
	}
#endif // endif
	/* during scan, if the recently closed prev_chanspec stats are in c_info->cur_stats,
	 * copy node content (except next pointer) to c_info->stats linked list entry as well
	 */
	if (if_info->stats == &wlc->chanim_info->cur_stats && SCAN_IN_PROGRESS(wlc->scan) &&
			CHSPEC_IS20(prev_chanspec)) {
		wlc_chanim_stats_t *stats = wlc_lq_chanim_find_stats(wlc, prev_chanspec);
		if (stats != NULL &&
			stats->chanim_stats.chanspec == if_info->stats->chanim_stats.chanspec) {
			if (stats->chanim_stats.timestamp <
				if_info->stats->chanim_stats.timestamp) {
				stats->chanim_stats = if_info->stats->chanim_stats;
				stats->is_valid = if_info->stats->is_valid;
			}
		}
	}
	/*
	 * Since it is switching to a new channel, update the cached pointer to point to the new
	 * counters.
	 */
	if_info->stats = wlc_lq_chanim_find_stats(wlc, chanspec);
	ASSERT(if_info->stats != NULL);
	/* Finally, update chanspec to reflect the new one. */
	if_info->chanspec = chanspec;
#ifdef WLCHANIM_US
	if_info->stats_us = wlc_lq_chanim_find_stats_us(wlc, chanspec);
	ASSERT(if_info->stats_us != NULL);

	/*
	 * Since we are switching to new channel update  accumulated counter with new channel
	 * stats
	 */
	last_cnt_us->start_tm = OSL_SYSUPTIME();
	last_cnt_us->total_tm = if_info->stats_us->chanim_stats_us.total_tm;
	if_info->acc_us->busy_tm = if_info->stats_us->chanim_stats_us.busy_tm;
	if_info->acc_us->rxcrs_pri20 = if_info->stats_us->chanim_stats_us.rxcrs_pri20;
	if_info->acc_us->rxcrs_sec20 = if_info->stats_us->chanim_stats_us.rxcrs_sec20;
	if_info->acc_us->rxcrs_sec40 = if_info->stats_us->chanim_stats_us.rxcrs_sec40;
	for (i = 0; i < CCASTATS_MAX; i++) {
		if_info->acc_us->ccastats_us[i] = if_info->stats_us->chanim_stats_us.ccastats_us[i];
	}
#endif /* WLCHANIM_US */
}

#if defined(WLMCHAN) && !defined(WLMCHAN_DISABLED)

/*
 * For multi-channel, it requires special hooks called during bss channel
 * creation and deletion. In addition, a hook is called during channel
 * adopt.
 */
void
wlc_lq_chanim_create_bss_chan_context(wlc_info_t *wlc, chanspec_t chanspec,
                                      chanspec_t prev_chanspec)
{
	chanim_interface_info_t *if_info = NULL;

	if ((prev_chanspec != 0) && (prev_chanspec != chanspec)) {
		/* An interface is switching channels. */
		if_info = wlc_lq_chanim_if_info_find_ctl(wlc->chanim_info->ifs, prev_chanspec);
	}

	if (if_info != NULL) {
		wlc_lq_chanim_if_switch_channels(wlc, if_info, chanspec, prev_chanspec);
	} else {
		/* Set up a new i/f */
		if_info = wlc_lq_chanim_if_info_setup(wlc->chanim_info, chanspec, FALSE);
	}

	ASSERT(if_info != NULL);
}

void
wlc_lq_chanim_delete_bss_chan_context(wlc_info_t *wlc, chanspec_t chanspec)
{
	wlc_lq_chanim_if_info_release(wlc->chanim_info, chanspec, FALSE);
}

/*
 * This is called from mchan's when it is about to adopt a new channel context. This routine
 * will check if we have an entry for the new channel. If not, then that means we're
 * transitioning to a new channel (a real channel change). Otherwise, we already have an
 * entry. No action is required.
 */
int
wlc_lq_chanim_adopt_bss_chan_context(wlc_info_t *wlc, chanspec_t chanspec, chanspec_t prev_chanspec)
{
	chanim_info_t *c_info = wlc->chanim_info;
	chanim_interfaces_t *ifaces = c_info->ifs;
	chanim_interface_info_t *if_info;

	ASSERT(chanspec != prev_chanspec);

	/* Is there an entry for the new channel? */
	if_info = wlc_lq_chanim_if_info_find(ifaces, chanspec);

	if (if_info) {
		/* Yes, no action required */
		return BCME_OK;
	}

	if (SCAN_IN_PROGRESS(wlc->scan)) {
		return BCME_BUSY;
	}

	/*
	 * The above search did not find a match. This can happen when a station chanspec changed
	 * bandwidth in adopt. Instead, search using control channel and switch to the new channel.
	 */
	if ((if_info = wlc_lq_chanim_if_info_find_ctl(ifaces, chanspec))) {
		wlc_lq_chanim_if_switch_channels(wlc, if_info, chanspec, if_info->chanspec);
	}
	return BCME_OK;
}
#endif /* WLMCHAN && !WLMCHAN_DISABLED */

/* Register with bsscfg for creating and tearing down i/f */
static void
wlc_lq_chanim_bsscfg_updn(void *ctx, bsscfg_up_down_event_data_t *evt)
{
	chanim_info_t *c_info = (chanim_info_t *) ctx;
	wlc_info_t *wlc = c_info->wlc;
	chanspec_t cs;

	ASSERT(evt != NULL);

	/*
	 * XXX Use bsscfg's current bss chanspec for the operational chanspec.
	 * However, in STA mode, this is set to 0x1001 instead of the real chanspec.
	 * Hence, this check.
	 */
	if (AP_ENAB(c_info->wlc->pub)) {
		cs = evt->bsscfg->current_bss->chanspec;
	} else {
		cs = wlc->chanspec;
	}

	if (evt->up) {
		(void) wlc_lq_chanim_if_info_setup(c_info, cs, TRUE);
	} else {
		wlc_lq_chanim_if_info_release(c_info, cs, TRUE);
	}
}

/*
 * Populate chanim info structure. Allocates a chanim interfaces structure. This
 * structure supports multiple interfaces (up to CHANIM_NUM_INTERFACES). An
 * additional entry is reserved for handling the channel scan case for quick lookup.
 *
 * XXX - CHANIM_NUM_INTERFACES is the number of interfaces to support. Currently,
 * this is a placeholder and set to 2 to satisfy VSDB case. Need to replace this
 * with something equivalent from upper layer.
 */
static int
BCMATTACHFN(wlc_lq_chanim_attach)(wlc_info_t *wlc)
{
	chanim_info_t *c_info;
	chanim_interfaces_t *ifaces;
	chanim_interface_info_t *if_info;
	chanim_accum_t *accum;
	int i;
	int num_ifs = MCHAN_ENAB(wlc->pub) ?
		CHANIM_NUM_INTERFACES_MCHAN : CHANIM_NUM_INTERFACES_SINGLECHAN;
	size_t chanim_info_sz = ((num_ifs+1) * sizeof(chanim_interface_info_t));
	size_t chanim_accum_sz = ((num_ifs+1) * sizeof(chanim_accum_t));
#ifdef WLCHANIM_US
	chanim_acc_us_t *acc_us;
	chanim_acc_us_t *last_acc_us;
	size_t chanim_acc_us_sz = ((num_ifs+1) * sizeof(chanim_acc_us_t));
#endif /* WLCHANIM_US */
	c_info = wlc->chanim_info;

	ASSERT(wlc->chanim_info != NULL);

#if defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "chanim", (dump_fn_t)wlc_dump_chanim, (void *)wlc);
#endif // endif

	c_info->config.crsglitch_thres = CRSGLITCH_THRESHOLD_DEFAULT;
	c_info->config.ccastats_thres = CCASTATS_THRESHOLD_DEFAULT;
	c_info->config.bgnoise_thres = BGNOISE_THRESHOLD_DEFAULT;
	c_info->config.mode = CHANIM_DETECT;
	c_info->config.sample_period = SAMPLE_PERIOD_DEFAULT;
	c_info->config.threshold_time = THRESHOLD_TIME_DEFAULT;
	c_info->config.lockout_period = LOCKOUT_PERIOD_DEFAULT;
	c_info->config.max_acs = MAX_ACS_DEFAULT;
	c_info->config.scb_max_probe = CHANIM_SCB_MAX_PROBE;

	c_info->stats = NULL;
#ifdef WLCHANIM_US
	c_info->stats_us = NULL;
#endif /* WLCHANIM_US */
	c_info->wlc = wlc;

	ifaces = MALLOCZ(wlc->osh, sizeof(chanim_interfaces_t));

	if (ifaces == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return -1;
	}

	/*
	 * The chanim bsscfg info is a structure for holding an array of accumulators.
	 * In addition, it caches the normalized stats counters f r quick access. It
	 * supports N number of interfaces, but allocates one additional. The extra
	 * entry is used for quick access for channel scans. NOTE: It only allocates
	 * a single block.
	 */
	if_info = ((chanim_interface_info_t *)MALLOCZ(wlc->osh, chanim_info_sz));

	if (if_info == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		MFREE(wlc->osh, ifaces, sizeof(chanim_interfaces_t));
		return -1;
	}

	accum = ((chanim_accum_t *) MALLOCZ(wlc->osh, chanim_accum_sz));

	if (accum == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		MFREE(wlc->osh, if_info, chanim_info_sz);
		MFREE(wlc->osh, ifaces, sizeof(chanim_interfaces_t));
		return -1;
	}

#ifdef WLCHANIM_US
	acc_us = ((chanim_acc_us_t *) MALLOCZ(wlc->osh, chanim_acc_us_sz));

	if (acc_us == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		MFREE(wlc->osh, if_info, chanim_info_sz);
		MFREE(wlc->osh, ifaces, sizeof(chanim_interfaces_t));
		return -1;
	}
	last_acc_us = ((chanim_acc_us_t *) MALLOCZ(wlc->osh, chanim_acc_us_sz));

	if (last_acc_us == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		MFREE(wlc->osh, if_info, chanim_info_sz);
		MFREE(wlc->osh, ifaces, sizeof(chanim_interfaces_t));
		return -1;
	}
#endif /* WLCHANIM_US */
	ifaces->if_info = if_info;
	ifaces->if_info_size = (int)chanim_info_sz;
	ifaces->num_ifs = num_ifs;
	wlc->chanim_info->ifs = ifaces;

	for (i = 0; i <= num_ifs; i++) {
		if_info[i].accum = accum++;
#ifdef WLCHANIM_US
		if_info[i].acc_us = acc_us++;
		if_info[i].last_acc_us = last_acc_us++;
#endif /* WLCHANIM_US */
		if_info[i].idx = i;
	}

	/*
	 * Initialize if_info dedicated for scan (this saves an extra check for chanspec valid in
	 * chanim_update).
	 */
	if_info[num_ifs].chanspec = 0x1001;
	if_info[num_ifs].stats = wlc_lq_chanim_find_stats(wlc, if_info[num_ifs].chanspec);

#ifdef WLCHANIM_US
	if_info[num_ifs].stats_us = wlc_lq_chanim_find_stats_us(wlc, if_info[num_ifs].chanspec);
	c_info->last_cnt_us.start_tm = OSL_SYSUPTIME();
#endif /* WLCHANIM_US */
	if (!MCHAN_ENAB(wlc->pub)) {
		if (wlc_bsscfg_updown_register(wlc, wlc_lq_chanim_bsscfg_updn,
		                               wlc->chanim_info) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
			          wlc->pub->unit, __FUNCTION__));
			MFREE(wlc->osh, if_info, chanim_info_sz);
			MFREE(wlc->osh, accum, chanim_accum_sz);
#ifdef WLCHANIM_US
			MFREE(wlc->osh, acc_us, chanim_acc_us_sz);
			MFREE(wlc->osh, last_acc_us, chanim_acc_us_sz);
#endif /* WLCHANIM_US */
			MFREE(wlc->osh, ifaces, sizeof(chanim_interfaces_t));
			return -1;
		}
	}
	/* init chanim timer */
#ifdef WLCHANIM_US
	c_info->chanim_timer = wl_init_timer(wlc->wl, wlc_chanim_msec_timeout,
			(void *)wlc, "chanim");
	if (!c_info->chanim_timer) {
		WL_ERROR(("wl%d: %s: wl_init_timer failed\n",
				wlc->pub->unit, __FUNCTION__));
		return BCME_NORESOURCE;
	}
#endif // endif
	return 0;
}

static void
BCMATTACHFN(wlc_lq_chanim_detach)(wlc_info_t *wlc)
{
	chanim_info_t *c_info = wlc->chanim_info;
	chanim_interfaces_t *ifaces = c_info->ifs;

	if (!MCHAN_ENAB(wlc->pub)) {
		wlc_bsscfg_updown_unregister(wlc, wlc_lq_chanim_bsscfg_updn, c_info);
	}
	if (!ifaces)
		return;
	MFREE(wlc->osh, ifaces->if_info[0].accum,  ((ifaces->num_ifs+1) * sizeof(chanim_accum_t)));
#ifdef WLCHANIM_US
	MFREE(wlc->osh, ifaces->if_info[0].acc_us, ((ifaces->num_ifs+1) * sizeof(chanim_acc_us_t)));
	MFREE(wlc->osh, ifaces->if_info[0].last_acc_us,
			((ifaces->num_ifs+1) * sizeof(chanim_acc_us_t)));
#endif /* WLCHANIM_US */
	MFREE(wlc->osh, ifaces->if_info, ifaces->if_info_size);
	MFREE(wlc->osh, ifaces, sizeof(chanim_interfaces_t));
}

static bool
wlc_lq_chanim_any_if_info_setup(chanim_interfaces_t *ifaces)
{
	int i;

	for (i = 0; i < ifaces->num_ifs; i++) {
		if (ifaces->if_info[i].chanspec != 0) {
			return TRUE;
		}
	}
	return FALSE;
}

/*
 * the main function for chanim information update
 * it could occur 1) on watchdog 2) on channel switch
 * based on the flag.
 */
int
wlc_lq_chanim_update(wlc_info_t *wlc, chanspec_t chanspec, uint32 flags)
{
	chanim_interfaces_t *ifaces = wlc->chanim_info->ifs;
	chanim_info_t* c_info = wlc->chanim_info;

	if (!WLC_CHANIM_ENAB(wlc)) {
		WL_ERROR(("wl%d: %s: WLC_CHANIM not enabled \n", wlc->pub->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	/* on watchdog trigger */
	if (flags & CHANIM_WD) {
		int i;

		if (!WLC_CHANIM_MODE_DETECT(c_info) ||
			(SCAN_IN_PROGRESS(wlc->scan))) {
			WL_SCAN(("wl%d: %s: WLC_CHANIM upd blocked scan/detect\n", wlc->pub->unit,
				__FUNCTION__));
			return BCME_NOTREADY;
		}

		/*
		 * Cycle through list of interfaces, accumulate matching chanspec and close,
		 * if required.
		 */
		for (i = 0; i < ifaces->num_ifs; i++) {
			chanim_interface_info_t *if_info = &ifaces->if_info[i];

			if (if_info->chanspec != 0 && !wf_chspec_malformed(if_info->chanspec)) {
				if (chanspec == if_info->chanspec) {
					wlc_lq_chanim_accum(wlc, if_info->chanspec, if_info->accum,
						if_info->acc_us);
				}

				if ((if_info->accum->stats_ms < CHANIM_CUR_CHAN_SAMPLING_DUR) &&
					chanspec == wlc->home_chanspec) {
					return BCME_NOTREADY;
				}

				if ((wlc->pub->now % chanim_config(c_info).sample_period) == 0) {
					wlc_lq_chanim_close(wlc, if_info->chanspec,
						if_info->accum,	if_info->stats, if_info->acc_us,
						if_info->stats_us, FALSE);

#ifdef WLCHANIM_US
					memcpy(ifaces->if_info[ifaces->num_ifs].acc_us,
						if_info->acc_us, sizeof(chanim_acc_us_t));
					ifaces->if_info[ifaces->num_ifs].stats_us =
						if_info->stats_us;
					ifaces->if_info[ifaces->num_ifs].chanspec =
						if_info->chanspec;
#endif /* WLCHANIM_US */
#ifdef BCMDBG
					if (wlc_lq_chanim_display(wlc, if_info->chanspec,
						if_info->stats) != BCME_OK)
					{
						WL_ERROR(("wl%d: %s: if_info->stats is NULL\n",
							wlc->pub->unit, __FUNCTION__));
					}
#endif // endif
					break;
				}
			}
		}
	}

	/* on channel switch */
	WL_CHANLOG(wlc, __FUNCTION__, TS_ENTER, 0);
	if (flags & CHANIM_CHANSPEC) {
		chanim_interface_info_t *if_info;
		chanspec_t prev_chanspec;

		/* Switching to a new channel */
		if_info = wlc_lq_chanim_if_info_find(wlc->chanim_info->ifs, chanspec);

		if (SCAN_IN_PROGRESS(wlc->scan) || !wlc_lq_chanim_any_if_info_setup(ifaces)) {
			if_info = &ifaces->if_info[ifaces->num_ifs];
			prev_chanspec = if_info->chanspec;
			wlc_lq_chanim_if_switch_channels(wlc, if_info, chanspec, prev_chanspec);
		} else {
			if (if_info != NULL)
				wlc_lq_chanim_accum(wlc, chanspec, if_info->accum, if_info->acc_us);
			/* When an interface changes its chanspec then close the current interface
			 * which would be interface at Zero index when mchan is not enabled.
			 * For mchan case the interface gets updated with the new chanspec by
			 * wlc_lq_chanim_create_bss_chan_context()
			 */
			if (!MCHAN_ENAB(wlc->pub) && if_info == NULL) {
				if_info = &ifaces->if_info[0];
				prev_chanspec = if_info->chanspec;
				wlc_lq_chanim_if_switch_channels(wlc, if_info, chanspec,
					prev_chanspec);
				/* Preserve original behavior. Is there a better way? */
				if (wlc->home_chanspec != if_info->chanspec) {
					if_info->chanspec = wlc->home_chanspec;
				}
			}

		}
	}
	WL_CHANLOG(wlc, __FUNCTION__, TS_EXIT, 0);
	return BCME_OK;
}

void
wlc_lq_chanim_acc_reset(wlc_info_t *wlc)
{
	chanim_info_t* c_info = wlc->chanim_info;
	chanim_interfaces_t *ifaces = wlc->chanim_info->ifs;
	chanim_interface_info_t *if_info;

	if_info = wlc_lq_chanim_if_info_find(ifaces, wlc->chanspec);

	if (if_info != NULL) {
		wlc_lq_chanim_clear_acc(wlc, if_info->accum, if_info->acc_us, TRUE);
	}
	bzero((char*)&c_info->last_cnt, sizeof(chanim_cnt_t));
}

static bool
wlc_lq_chanim_interfered_glitch(wlc_chanim_stats_t *stats, uint32 thres)
{
	bool interfered = FALSE;

	interfered = stats->chanim_stats.glitchcnt > thres;
	return interfered;
}

static bool
wlc_lq_chanim_interfered_cca(wlc_chanim_stats_t *stats, uint32 thres)
{
	bool interfered = FALSE;
	uint8 stats_sum;

	stats_sum = stats->chanim_stats.ccastats[CCASTATS_NOPKT];
	interfered = stats_sum > (uint8)thres;

	return interfered;
}

static bool
wlc_lq_chanim_interfered_noise(wlc_chanim_stats_t *stats, int8 thres)
{
	bool interfered = FALSE;
	int8 bgnoise;

	bgnoise = stats->chanim_stats.bgnoise;
	interfered = bgnoise > (uint8)thres;

	return interfered;
}

bool
wlc_lq_chanim_interfered(wlc_info_t *wlc, chanspec_t chanspec)
{
	bool interfered = FALSE;
	wlc_chanim_stats_t *cur_stats;
	chanim_info_t *c_info = wlc->chanim_info;

	cur_stats = wlc_lq_chanim_chanspec_to_stats(wlc->chanim_info, chanspec, FALSE);

	if (!cur_stats)  {
		WL_INFORM(("%s: no stats allocated for chanspec 0x%x\n",
			__FUNCTION__, chanspec));
		return interfered;
	}

	if (wlc_lq_chanim_interfered_glitch(cur_stats, chanim_config(c_info).crsglitch_thres) ||
		wlc_lq_chanim_interfered_cca(cur_stats, chanim_config(c_info).ccastats_thres) ||
		wlc_lq_chanim_interfered_noise(cur_stats, chanim_config(c_info).bgnoise_thres))
		interfered = TRUE;

	if (chanspec == wlc->home_chanspec)
		chanim_mark(c_info).state = interfered;

	return interfered;
}

static void
wlc_lq_chanim_scb_probe(wlc_info_t *wlc, bool activate)
{
	chanim_info_t * c_info = wlc->chanim_info;

	ASSERT(AP_ENAB(wlc->pub));

	if (activate) {
		/* store the original values, and replace with the chanim values */
		chanim_mark(c_info).scb_timeout = wlc->ap->scb_timeout;
		chanim_mark(c_info).scb_max_probe = wlc->ap->scb_max_probe;
		wlc->ap->scb_timeout = chanim_config(c_info).sample_period;
		wlc->ap->scb_max_probe = chanim_config(c_info).scb_max_probe;
	}
	else {
		/* swap back on exit */
		wlc->ap->scb_timeout = chanim_mark(c_info).scb_timeout;
		wlc->ap->scb_max_probe = chanim_mark(c_info).scb_max_probe;
	}
}

void
wlc_lq_chanim_upd_act(wlc_info_t *wlc)
{
	chanim_info_t * c_info = wlc->chanim_info;

	if (wlc_lq_chanim_interfered(wlc, wlc->home_chanspec) &&
		(wlc->chanspec == wlc->home_chanspec)) {
		if (chanim_mark(c_info).detecttime && !WLC_CHANIM_ACT(c_info)) {
			if ((wlc->pub->now - chanim_mark(c_info).detecttime) >
				(uint)chanim_act_delay(c_info)) {
			    chanim_mark(c_info).flags |= CHANIM_ACTON;
				WL_CHANINT(("***chanim action set\n"));
			}
		}
		else if (!WLC_CHANIM_ACT(c_info)) {
			chanim_mark(c_info).detecttime = wlc->pub->now;
#ifdef AP
			/* start to probe */
			wlc_lq_chanim_scb_probe(wlc, TRUE);
#endif /* AP */
		}
	}
	else {
#ifdef AP
		if (chanim_mark(c_info).detecttime)
			wlc_lq_chanim_scb_probe(wlc, FALSE);
#endif /* AP */
		chanim_mark(c_info).detecttime = 0;
		chanim_mark(c_info).flags &= ~CHANIM_ACTON;
	}
}

void
wlc_lq_chanim_upd_acs_record(chanim_info_t *c_info, chanspec_t home_chspc,
	chanspec_t selected, uint8 trigger)
{
	chanim_acs_record_t* cur_record = &c_info->record[chanim_mark(c_info).record_idx];
	wlc_chanim_stats_t *cur_stats;
	cur_stats = wlc_lq_chanim_chanspec_to_stats(c_info, home_chspc, FALSE);

	if (WLC_CHANIM_MODE_EXT(c_info))
		return;

	bzero(cur_record, sizeof(chanim_acs_record_t));

	cur_record->trigger = trigger;
	cur_record->timestamp = OSL_SYSUPTIME();
	cur_record->selected_chspc = selected;
	cur_record->valid = TRUE;

	if (cur_stats) {
		cur_record->glitch_cnt = cur_stats->chanim_stats.glitchcnt;
		cur_record->ccastats = cur_stats->chanim_stats.ccastats[CCASTATS_NOPKT];
	}

	chanim_mark(c_info).record_idx ++;
	if (chanim_mark(c_info).record_idx == CHANIM_ACS_RECORD)
		chanim_mark(c_info).record_idx = 0;
}

static int
wlc_lq_chanim_get_acs_record(chanim_info_t *c_info, int buf_len, void *output)
{
	wl_acs_record_t *record = (wl_acs_record_t *)output;
	uint8 idx = chanim_mark(c_info).record_idx;
	int i, count = 0;

	if (WLC_CHANIM_MODE_EXT(c_info))
		return BCME_OK;

	for (i = 0; i < CHANIM_ACS_RECORD; i++) {
		if (c_info->record[idx].valid) {
			bcopy(&c_info->record[idx], &record->acs_record[i],
				sizeof(chanim_acs_record_t));
			count++;
		}
		idx = (idx + 1) % CHANIM_ACS_RECORD;
	}

	record->count = (uint8)count;
	record->timestamp = OSL_SYSUPTIME();
	return BCME_OK;
}

#ifdef WLCHANIM_US
static int
wlc_lq_chanim_get_stats_us(chanim_info_t *c_info, wl_chanim_stats_us_t* iob,
	int *len, int cnt, uint32 dur)
{
	uint32 count = 0;
	uint32 datalen;
	wlc_chanim_stats_us_t* stats_us = NULL;
	int bcmerror = BCME_OK;
	int buflen = *len;
	wlc_chanim_stats_us_t cur_stats_us;
	datalen = WL_CHANIM_STATS_US_FIXED_LEN;

	if (cnt == WL_CHANIM_COUNT_US_ALL) {
		stats_us = c_info->stats_us;
	} else if (cnt == WL_CHANIM_COUNT_US_ONE) {
		int i;
		/*
		 * There are multiple i/f. To preserve original behavior, find the i/f that matches
		 * home chanspec.
		 */
		for (i = 0; i < c_info->ifs->num_ifs; i++) {
			chanim_interface_info_t *if_info = &c_info->ifs->if_info[i];
			if (c_info->wlc->home_chanspec == if_info->chanspec) {
				cur_stats_us = *if_info->stats_us;
				cur_stats_us.next = NULL;
				stats_us = &cur_stats_us;
				break;
			}
		}
	} else if (cnt == WL_CHANIM_COUNT_US_RESET) {
		int32 home_chspec_band = CHSPEC_BAND(c_info->wlc->home_chanspec);
		chanim_cnt_us_t *last_cnt_us;

		last_cnt_us = &c_info->last_cnt_us;
		stats_us = c_info->stats_us;
		while (stats_us) {
			chanspec_t  chanspec_temp;
			if (home_chspec_band == CHSPEC_BAND(stats_us->chanim_stats_us.chanspec)) {
				int i = 0;
				for (i = 0; i < c_info->ifs->num_ifs; i++) {
					chanim_interface_info_t *if_info = &c_info->ifs->if_info[i];
					if (c_info->wlc->home_chanspec == if_info->chanspec) {
						if (if_info->acc_us)  {
							if_info->acc_us->busy_tm = 0;
							if_info->acc_us->rxcrs_pri20 = 0;
							if_info->acc_us->rxcrs_sec20 = 0;
							if_info->acc_us->rxcrs_sec40 = 0;

							for (i = 0; i < CCASTATS_MAX; i++) {
								if_info->acc_us->ccastats_us[i] = 0;
							}
						}
						break;
					}
				}
				chanspec_temp = stats_us->chanim_stats_us.chanspec;
				bzero(&stats_us->chanim_stats_us, sizeof(chanim_stats_us_t));
				stats_us->chanim_stats_us.chanspec = chanspec_temp;
				last_cnt_us->start_tm = OSL_SYSUPTIME();
				last_cnt_us->total_tm = 0;
			}

			stats_us = stats_us->next;
		}
		iob->count = WL_CHANIM_COUNT_US_RESET;
		iob->version = WL_CHANIM_STATS_US_VERSION;
		return bcmerror;
	} else if (cnt == WL_CHANIM_US_DUR) {
		int i;
		for (i = 0; i < c_info->ifs->num_ifs; i++) {
			chanim_interface_info_t *if_info = &c_info->ifs->if_info[i];
			if (c_info->wlc->home_chanspec == if_info->chanspec) {
				bzero(&if_info->chanim_stats_us, sizeof(chanim_stats_us_t));
				wlc_lq_chanim_accum(c_info->wlc, if_info->chanspec, if_info->accum,
					if_info->acc_us);
				if_info->chanim_stats_us.total_tm = dur * 1000;
				if_info->chanim_stats_us.chanspec = if_info->chanspec;
				bcopy(if_info->acc_us, if_info->last_acc_us,
					sizeof(chanim_acc_us_t));
				break;
			}
		}
		wl_del_timer(c_info->wlc->wl, c_info->chanim_timer);
		wl_add_timer(c_info->wlc->wl, c_info->chanim_timer, dur, 0);
		iob->version = WL_CHANIM_STATS_US_VERSION;
		return bcmerror;
	} else if (cnt == WL_CHANIM_US_DUR_GET) {
		int i;

		for (i = 0; i < c_info->ifs->num_ifs; i++) {
			chanim_interface_info_t *if_info = &c_info->ifs->if_info[i];
			if (c_info->wlc->home_chanspec == if_info->chanspec) {
				bcopy(&if_info->chanim_stats_us, &iob->stats_us[count],
						sizeof(chanim_stats_us_t));
				count++;
				datalen += sizeof(chanim_stats_us_t);
				buflen -= sizeof(chanim_stats_us_t);
				iob->count = count;
				iob->buflen = datalen;
				iob->version = WL_CHANIM_STATS_US_VERSION;
				return bcmerror;
			}
		}

	}
	if (stats_us == NULL) {
		memset(&iob->stats_us[0], 0, sizeof(chanim_stats_us_t));
		count = 0;
	} else {
		int32 home_chspec_band = CHSPEC_BAND(c_info->wlc->home_chanspec);

		while (stats_us) {
			if (buflen < (int)sizeof(chanim_stats_us_t)) {
				bcmerror = BCME_BUFTOOSHORT;
				break;
			}
			if (home_chspec_band == CHSPEC_BAND(stats_us->chanim_stats_us.chanspec)) {
				bcopy(&stats_us->chanim_stats_us, &iob->stats_us[count],
					sizeof(chanim_stats_us_t));
				count++;
				datalen += sizeof(chanim_stats_us_t);
				buflen -= sizeof(chanim_stats_us_t);
			}

			stats_us = stats_us->next;
		}
	}

	iob->count = count;
	iob->buflen = datalen;
	iob->version = WL_CHANIM_STATS_US_VERSION;

	return bcmerror;
}

static void wlc_chanim_msec_timeout(void *arg)
{
	wlc_info_t *wlc = (wlc_info_t *)arg;
	chanim_interfaces_t *ifaces = wlc->chanim_info->ifs;
	chanim_info_t* c_info = wlc->chanim_info;
	int i, j;
	for (i = 0; i < ifaces->num_ifs; i++) {
		chanim_interface_info_t *if_info = &c_info->ifs->if_info[i];
		if (c_info->wlc->home_chanspec == if_info->chanspec) {
			wlc_lq_chanim_accum(c_info->wlc, if_info->chanspec, if_info->accum,
					if_info->acc_us);
			if_info->chanim_stats_us.chanspec = if_info->chanspec;
			if_info->chanim_stats_us.busy_tm =
					if_info->acc_us->busy_tm - if_info->last_acc_us->busy_tm;
			for (j = 0; j < CCASTATS_MAX; j++) {
				if_info->chanim_stats_us.ccastats_us[j]	=
					if_info->acc_us->ccastats_us[j]-
					if_info->last_acc_us->ccastats_us[j];
			}
			if_info->chanim_stats_us.rxcrs_pri20 = if_info->acc_us->rxcrs_pri20 -
				if_info->last_acc_us->rxcrs_pri20;
			if_info->chanim_stats_us.rxcrs_sec20 = if_info->acc_us->rxcrs_sec20 -
				if_info->last_acc_us->rxcrs_sec20;
			if_info->chanim_stats_us.rxcrs_sec40 = if_info->acc_us->rxcrs_sec40 -
				if_info->last_acc_us->rxcrs_sec40;
			break;
		}
	}
}
#endif /* WLCHANIM_US */

static int
wlc_lq_chanim_get_stats(chanim_info_t *c_info, wl_chanim_stats_t* iob, int *len, int cnt)
{
	uint32 count = 0;
	uint32 datalen;
	wlc_chanim_stats_t* stats = NULL;
	int bcmerror = BCME_OK;
	int buflen = *len;
	wlc_chanim_stats_t cur_stats;

	iob->version = WL_CHANIM_STATS_VERSION;
	datalen = WL_CHANIM_STATS_FIXED_LEN;

	if (cnt == WL_CHANIM_COUNT_ALL)
		stats = c_info->stats;
	else {
int i;
		/*
		 * There are multiple i/f. To preserve original behavior, find the i/f that matches
		 * home chanspec.
		 */
		for (i = 0; i < c_info->ifs->num_ifs; i++) {
			chanim_interface_info_t *if_info = &c_info->ifs->if_info[i];
			if (c_info->wlc->home_chanspec == if_info->chanspec) {
				cur_stats = *if_info->stats;
				cur_stats.next = NULL;
				stats = &cur_stats;
				break;
			}
		}
	}

	if (stats == NULL) {
		memset(&iob->stats[0], 0, sizeof(chanim_stats_t));
		count = 0;
	} else {
		int32 home_chspec_band = CHSPEC_BAND(c_info->wlc->home_chanspec);

		while (stats) {
			if (buflen < (int)sizeof(chanim_stats_t)) {
				bcmerror = BCME_BUFTOOSHORT;
				break;
			}
			if (home_chspec_band == CHSPEC_BAND(stats->chanim_stats.chanspec)) {
				bcopy(&stats->chanim_stats, &iob->stats[count],
					sizeof(chanim_stats_t));

				count++;
				datalen += sizeof(chanim_stats_t);
				buflen -= sizeof(chanim_stats_t);
			}
			stats = stats->next;
		}
	}

	iob->count = count;
	iob->buflen = datalen;
	return bcmerror;
}

#ifdef APCS
static bool
chanim_chk_lockout(chanim_info_t *c_info)
{
	uint8 cur_idx = chanim_mark(c_info).record_idx;
	uint8 start_idx;
	chanim_acs_record_t *start_record;
	uint32 cur_time;

	if (!chanim_config(c_info).max_acs)
		return TRUE;

	start_idx = MODSUB(cur_idx, chanim_config(c_info).max_acs, CHANIM_ACS_RECORD);
	start_record = &c_info->record[start_idx];
	cur_time = OSL_SYSUPTIME();

	if (start_record->valid && ((cur_time - start_record->timestamp) <
			chanim_config(c_info).lockout_period * 1000)) {
		WL_CHANINT(("***chanim lockout true\n"));
		return TRUE;
	}

	return FALSE;
}

/* function for chanim mitigation (action) */
void
wlc_lq_chanim_action(wlc_info_t *wlc)
{
	chanim_info_t *c_info = wlc->chanim_info;
	struct scb *scb;
	struct scb_iter scbiter;
	/* clear the action flag and reset detecttime */
	chanim_mark(c_info).flags &= ~CHANIM_ACTON;
	chanim_mark(c_info).detecttime = 0;
#ifdef AP
	wlc_lq_chanim_scb_probe(wlc, FALSE);
#endif /* AP */

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		if (SCB_ASSOCIATED(scb) &&
		    (wlc->pub->now - scb->used < (uint)chanim_act_delay(c_info)))
			break;
	}

	if (!scb) {
		wl_uint32_list_t request;

		if (chanim_chk_lockout(c_info)) {
			WL_CHANINT(("***chanim scan is not allowed due to lockout\n"));
			return;
		}

		request.count = 0;

		if (APCS_ENAB(wlc->pub)) {
			(void)wlc_cs_scan_start(wlc->cfg, &request, TRUE, FALSE, TRUE,
				wlc->band->bandtype, APCS_CHANIM, NULL, NULL);
		}
	}
	return;
}
#endif /* APCS */
#endif /* WLCHANIM */

typedef struct wlc_lq_stats_notif {
	wlc_bmac_obss_counts_t *init_stats;	/* stats at the time when notif reg is called. */
	wlc_info_t *wlc;
	uint32 req_time_ms;
	stats_cb cb;
	uint16 connID;
	void *arg;
	struct wl_timer *notif_timer;
} wlc_lq_stats_notif_t;

static void
wlc_lq_register_obss_stats_cleanup(wlc_info_t *wlc, wlc_lq_stats_notif_t *notif_ctx);

static void
wlc_lq_notif_timer(void *arg);

static void
wlc_lq_register_obss_stats_cleanup(wlc_info_t *wlc, wlc_lq_stats_notif_t *notif_ctx)
{
	if (notif_ctx) {
		if (notif_ctx->notif_timer) {
			wl_del_timer(wlc->wl, notif_ctx->notif_timer);
			wl_free_timer(wlc->wl, notif_ctx->notif_timer);
		}
		if (notif_ctx->init_stats)
			MFREE(wlc->osh, notif_ctx->init_stats,
				sizeof(wlc_bmac_obss_counts_t));
		MFREE(wlc->osh, notif_ctx, sizeof(wlc_lq_stats_notif_t));
	}
}

static void
wlc_lq_notif_timer(void *arg)
{
	wlc_lq_stats_notif_t *notif_ctx = (wlc_lq_stats_notif_t *)arg;
	wlc_bmac_obss_counts_t curr = {0};
	wlc_bmac_obss_counts_t *prev = notif_ctx->init_stats;
	wlc_bmac_obss_counts_t delta;

	if (wlc_bsscfg_is_intf_up(notif_ctx->wlc, notif_ctx->connID) ==
		FALSE) {
		(notif_ctx->cb)(notif_ctx->wlc, notif_ctx->arg,
			notif_ctx->req_time_ms, NULL);
		goto fail;
	}
	/* get cur_statsent stats in  notif_ctx->init_stats */
	wlc_bmac_obss_stats_read(notif_ctx->wlc->hw, &curr);

	/* calc delta */
	delta.usecs = curr.usecs - prev->usecs;
	delta.txdur = curr.txdur - prev->txdur;
	delta.ibss = curr.ibss - prev->ibss;
	delta.obss = curr.obss - prev->obss;
	delta.noctg = curr.noctg - prev->noctg;
	delta.nopkt = curr.nopkt - prev->nopkt;
	delta.PM = curr.PM - prev->PM;
	delta.txopp = curr.txopp - prev->txopp;
	delta.slot_time_txop = curr.slot_time_txop;
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	delta.gdtxdur = curr.gdtxdur - prev->gdtxdur;
	delta.bdtxdur = curr.bdtxdur - prev->bdtxdur;
#endif // endif

#ifdef WL_OBSS_DYNBW
	if (D11REV_GE(notif_ctx->wlc->pub->corerev, 40)) {
		delta.rxdrop20s = curr.rxdrop20s - prev->rxdrop20s;
		delta.rx20s = curr.rx20s - prev->rx20s;

		delta.rxcrs_pri = curr.rxcrs_pri - prev->rxcrs_pri;
		delta.rxcrs_sec20 = curr.rxcrs_sec20 - prev->rxcrs_sec20;
		delta.rxcrs_sec40 = curr.rxcrs_sec40 - prev->rxcrs_sec40;

		delta.sec_rssi_hist_hi = curr.sec_rssi_hist_hi - prev->sec_rssi_hist_hi;
		delta.sec_rssi_hist_med = curr.sec_rssi_hist_med - prev->sec_rssi_hist_med;
		delta.sec_rssi_hist_low = curr.sec_rssi_hist_low - prev->sec_rssi_hist_low;
	}
#endif /* WL_OBSS_DYNBW */
	/* call back using cb with the delta stats */
	(notif_ctx->cb)(notif_ctx->wlc, notif_ctx->arg, notif_ctx->req_time_ms, &delta);

fail:
	/* free the notif_ctx */
	wlc_lq_register_obss_stats_cleanup(notif_ctx->wlc, notif_ctx);
}

int
wlc_lq_register_dynbw_stats_cb(wlc_info_t *wlc, uint32 req_time_ms, stats_cb cb,
	uint16 connID, void *arg)
{
	wlc_lq_stats_notif_t *notif_ctx = NULL;
	int err = BCME_OK;

	/* if the radio/intf is down, clear the context and signal failure
	*/
	if (wlc_bsscfg_is_intf_up(wlc, connID) ==
		FALSE) {
		(cb)(wlc, arg, req_time_ms, NULL);
		goto cb_fail;
	}

	notif_ctx = (wlc_lq_stats_notif_t *) MALLOCZ(wlc->osh, sizeof(wlc_lq_stats_notif_t));

	if (notif_ctx == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced bytes\n",
		          wlc->pub->unit,
		          __FUNCTION__));
		err = BCME_NOMEM;
		goto cb_fail;
	}
	notif_ctx->cb = cb;
	notif_ctx->arg = arg;
	notif_ctx->req_time_ms = req_time_ms;
	notif_ctx->wlc = wlc;
	notif_ctx->connID = connID;
	notif_ctx->init_stats = (wlc_bmac_obss_counts_t *) MALLOCZ(wlc->osh,
		sizeof(wlc_bmac_obss_counts_t));

	if (notif_ctx->init_stats == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced bytes\n",
		          wlc->pub->unit,
		          __FUNCTION__));
		err = BCME_NOMEM;
		goto cb_fail;
	}

	notif_ctx->notif_timer = wl_init_timer(wlc->wl, wlc_lq_notif_timer, notif_ctx, "lq_notif");

	if (!notif_ctx->notif_timer) {
		err = BCME_NOMEM;
		goto cb_fail;
	}

	/* store current stats in  notif_ctx->init_stats */
	wlc_bmac_obss_stats_read(wlc->hw, notif_ctx->init_stats);

	/* start timer */
	wl_add_timer(wlc->wl, notif_ctx->notif_timer, req_time_ms, 0);

	return BCME_OK;

cb_fail:
	wlc_lq_register_obss_stats_cleanup(wlc, notif_ctx);
	return err;
}

#if defined(BCMDBG_DUMP)
#ifdef WLCHANIM
static int
wlc_dump_chanim(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	chanim_info_t* c_info = wlc->chanim_info;
	wlc_chanim_stats_t *stats = c_info->stats;

	bcm_bprintf(b, "CHAN Interference Measurement:\n");
	bcm_bprintf(b, "Stats during last scan:\n");
	while (stats) {
		bcm_bprintf(b, " chanspec: 0x%x crsglitch cnt: %d bad plcp: %d noise: %d\n",
			stats->chanim_stats.chanspec, stats->chanim_stats.glitchcnt,
			stats->chanim_stats.badplcp, stats->chanim_stats.bgnoise);

		if (WLC_CCASTATS_CAP(wlc))
			bcm_bprintf(b, "\t cca_txdur: %d cca_inbss: %d cca_obss:"
			  "%d cca_nocat: %d cca_nopkt: %d\n",
			  stats->chanim_stats.ccastats[CCASTATS_TXDUR],
			  stats->chanim_stats.ccastats[CCASTATS_INBSS],
			  stats->chanim_stats.ccastats[CCASTATS_OBSS],
			  stats->chanim_stats.ccastats[CCASTATS_NOCTG],
			  stats->chanim_stats.ccastats[CCASTATS_NOPKT]);
		stats = stats->next;
	}
	bcm_bprintf(b, "curent stats:\n");

	stats = &c_info->cur_stats;

	bcm_bprintf(b, " chanspec: 0x%x crsglitch cnt: %d bad plcp: %d noise: %d\n",
			stats->chanim_stats.chanspec, stats->chanim_stats.glitchcnt,
			stats->chanim_stats.badplcp, stats->chanim_stats.bgnoise);

	if (WLC_CCASTATS_CAP(wlc)) {
		bcm_bprintf(b, "\t cca_txdur: %d cca_inbss: %d cca_obss:"
				"%d cca_nocat: %d cca_nopkt: %d\n",
				stats->chanim_stats.ccastats[CCASTATS_TXDUR],
				stats->chanim_stats.ccastats[CCASTATS_INBSS],
				stats->chanim_stats.ccastats[CCASTATS_OBSS],
				stats->chanim_stats.ccastats[CCASTATS_NOCTG],
				stats->chanim_stats.ccastats[CCASTATS_NOPKT]);
	}
	return BCME_OK;
}
#endif /* WLCHANIM */
#endif // endif
