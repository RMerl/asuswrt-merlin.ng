/*
 * 802.11h DFS module source file
 * Broadcom 802.11abgn Networking Device Driver
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
 * $Id: wlc_dfs.c 785123 2020-03-13 11:24:32Z $
 */

/**
 * @file
 * @brief
 * Related to radar avoidance. Implements CAC (Channel Availability Check) state machine.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [DynamicFrequencySelection]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

/* XXX make sure WLDFS conditional is referenced after it is derived from WL11H and BAND5G.
 * in wlc_cfg.h. Move it up when it becomes an externally defined conditional.
 */
#ifdef WLDFS

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
#include <wl_export.h>
#include <wlc_ap.h>
#include <wlc_scan.h>
#include <wlc_phy_hal.h>
#include <phy_radar_api.h>
#include <phy_ac_chanmgr.h>
#include <wlc_quiet.h>
#include <wlc_csa.h>
#include <wlc_11h.h>
#include <wlc_dfs.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_bmac.h>
#include <wlc_modesw.h>
#include <wlc_stf.h>

#ifdef WLRSDB
#include <wlc_rsdb.h>
#endif /* WLRSDB */
#include <wlc_assoc.h>

/* IOVar table */
/* No ordering is imposed */
enum {
	IOV_DFS_PREISM,		/* preism cac time */
	IOV_DFS_POSTISM,	/* postism cac time */
	IOV_DFS_STATUS,		/* dfs cac status */
	IOV_DFS_ISM_MONITOR,    /* control the behavior of ISM state */
	IOV_DFS_CHANNEL_FORCED, /* next dfs channel forced */
	IOV_DFS_AP_MOVE,	/* Move the AP to specified RADAR Channel using second core. */
	IOV_DFS_STATUS_ALL,	/* dfs status of multiple cores / parallel radar scans */
	IOV_DYN160,		/* toggle dynamic 160MHz to 80MHz mode active */
	IOV_DFS_TEST_MODE,	/* detect radar, send CSA, but don't switch channel for Wave DFS */
	IOV_DFS_BW_FALLBACK,
	IOV_LAST
};

static const bcm_iovar_t wlc_dfs_iovars[] = {
	{"dfs_preism", IOV_DFS_PREISM, 0, IOVT_UINT32, 0},
	{"dfs_postism", IOV_DFS_POSTISM, 0, IOVT_UINT32, 0},
	{"dfs_status", IOV_DFS_STATUS, (0), IOVT_BUFFER, 0},
	/* it is required for regulatory testing */
	{"dfs_ism_monitor", IOV_DFS_ISM_MONITOR, (0), IOVT_UINT32, 0},
	{"dfs_channel_forced", IOV_DFS_CHANNEL_FORCED, (0), IOVT_UINT32, 0},
	{"dfs_ap_move", IOV_DFS_AP_MOVE, (0), IOVT_BUFFER, 0},
	{"dfs_status_all", IOV_DFS_STATUS_ALL, (0), IOVT_BUFFER, 0},
	{"dyn160", IOV_DYN160, 0, IOVT_UINT32, 0},
#ifdef WL_DFS_WAVE_MODE
	{"dfs_test_mode", IOV_DFS_TEST_MODE, 0, IOVT_UINT32, 0},
#endif /* WL_DFS_WAVE_MODE */
	{"dfs_bw_fallback", IOV_DFS_BW_FALLBACK, 0, IOVT_UINT32, 0},
	{NULL, 0, 0, 0, 0}
};

/* TDWR protection special case macros */
#define TDWR_CH20_MIN	120u	/* lowest 20MHz TDWR channel number */
#define TDWR_CH20_MAX	128u	/* highest 20MHz TDWR channel number */

#ifdef BGDFS
/* traffic thresholds used to decide if background DFS CAC results must be discarded */
#define BGDFS_DISCARD_RESULT_TRAFFIC_PERCENT_ADJACENT		(1)	/* adjacent channel */
#define BGDFS_DISCARD_RESULT_TRAFFIC_PERCENT_NONADJACENT_FCC	(17)	/* non-adjacent in FCC */
#define BGDFS_DISCARD_RESULT_TRAFFIC_PERCENT_NONADJACENT_ETSI	(30)	/* non-adjacent in ETSI */
#define BGDFS_DISCARD_RESULT_TRAFFIC_PERCENT_WEATHER_ETSI	(5)	/* EU weather channel */

/* bandwidth in MHz */
#define CHAN_BW_80MHZ 80
#define CHAN_BW_40MHZ 40
#define CHAN_BW_20MHZ 20

/* In which cases to consider txblanking (traffic) on main core at end of CAC on scan core */
#define BGDFS_TXBLANK_CHECK_MODE_NONE		0x00u		/* never */
#define BGDFS_TXBLANK_CHECK_MODE_ADJ		0x01u		/* if on adjacent channel */
#define BGDFS_TXBLANK_CHECK_MODE_NONADJ		0x02u		/* if on non-adjacent channel */
#define BGDFS_TXBLANK_CHECK_MODE_EU_WEATHER	0x04u		/* if on weather channel in EU */
#define BGDFS_TXBLANK_CHECK_MODE_ALL		0xFFu		/* always */

#endif /* BGDFS */

/* subband info bitmap position */
#define LOWER_20_POS_160MHZ	0x80u
#define LOWER_20_POS_80MHZ	0x08u
#define LOWER_20_POS_40MHZ	0x02u
#define LOWER_20_POS_20MHZ	0x01u

#if defined(RSDB_DFS_SCAN) || defined(BGDFS)
#define DFS_SCAN_IN_PROGRESS(dfs)  (dfs && dfs->dfs_scan && dfs->dfs_scan->inprogress)
#else /* RSDB_DFS_SCAN || BGDFS */
#define DFS_SCAN_IN_PROGRESS(dfs)  (FALSE)
#endif /* RSDB_DFS_SCAN || BGDFS */

#define RADAR_INFO_SHIFT_TYPE		0
#define RADAR_INFO_SHIFT_MINPW		4
#define RADAR_INFO_SHIFT_SUBBAND	14

#define RADAR_INFO_MASK_TYPE		0xf
#define RADAR_INFO_MASK_MINPW		0x1ff
#define RADAR_INFO_MASK_SUBBAND		0xf

/* Extract ITEM from bit packed radar info in RAD_INF. ITEM must be one of TYPE, MINPW or SUBBAND */
#define RADAR_INFO_EXTRACT(RAD_INF, ITEM) \
	((RAD_INF) >> RADAR_INFO_SHIFT_ ## ITEM & RADAR_INFO_MASK_ ## ITEM)

typedef struct wlc_dfs_cac {
	int	cactime_pre_ism;	/* configured preism cac time in second */
	int	cactime_post_ism;	/* configured postism cac time in second */
	uint32	nop_sec;		/* Non-Operation Period in second */
	int	ism_monitor;		/* 0 for off, non-zero to force ISM to monitor-only mode */
	wl_dfs_status_t	status;		/* data structure for handling dfs_status iovar */
	uint	cactime;      /* holds cac time in WLC_DFS_RADAR_CHECK_INTERVAL for current state */
	/* use of duration
	 * 1. used as a down-counter where timer expiry is needed.
	 * 2. (cactime - duration) is time elapsed at current state
	 */
	uint	duration;
	chanspec_t chanspec_next;	/* next dfs channel */
	/* When test_mode is non-zero, the AP will detect radar on a channel and
	 * will send channel switch announcements (CSA), but will not actually
	 * switch channels. This test mode enables testing with the IXIA WaveDFS tools,
	 * which put radar on the air and listen for a CSA to verify the AP recognized
	 * the radar, but then requires the AP to stay on the same channel. Test mode
	 * differs from ism_monitor. ism_monitor ignores radar in ISM state, not sending
	 * CSA. When enabling test_mode, IOV_DFS_PREISM and IOV_DFS_POSTISM should bet
	 * set to 0. Doing so prevents the AP from going into the CAC states, where
	 * radar detection prompts an immediate channel switch (w/o CSA).
	 */
	uint8	test_mode;
	bool	timer_running;
	chanspec_list_t *chanspec_forced_list; /* list of chanspecs to use when radar detected */
} wlc_dfs_cac_t;

#define DFS_MODESW_IDLE				0	/* mode switch for dfs scan is in idle. */
#define DFS_MODESW_DOWNGRADE_IN_PROGRESS	1	/* waiting for downgrade. */
#define DFS_MODESW_DOWNGRADE_IN_FINISHED	2	/* Downgrade is done. Scan in progress. */
#define DFS_MODESW_UPGRADE_IN_PROGRESS		3	/* Upgrade requested and pending state. */

#if defined(RSDB_DFS_SCAN) || defined(BGDFS)
struct dfs_scan_cmn {
	uint8	modeswitch_state; /* MIMO to SISO modewitch is pending. */
	uint8	inprogress;	/* DFS scan in progress */
	uint8	_was_ap;	/* Keep to AP state to trigger DFS scan. */
	chanspec_t home_chan;	/* previous home chan */
	wlc_info_t *scan_wlc;	/* wlc where dfs scan is running */
	int8 status;		/* Scan status */
	wl_chan_switch_t csa;	/* Chanswitch information after RADAR clear detection */
};
#endif /* RSDB_DFS_SCAN || BGDFS */

typedef struct dfs_scan_cmn dfs_scan_cmn_t;

/* Country module info */
struct wlc_dfs_info {
	wlc_info_t *wlc;
	uint chan_blocked[MAXCHANNEL];	/* 11h: seconds remaining in channel
					 * out of service period due to radar
					 */
	bool dfs_cac_enabled;		/* set if dfs cac enabled */
	struct wl_timer *dfs_timer;	/* timer for dfs cac handler */
	wlc_dfs_cac_t dfs_cac;		/* channel availability check */
	uint32 radar;			/* radar info: just on or off for now */

	dfs_scan_cmn_t *dfs_scan;	/* dfs_scan structure. Shared via objreg if RSDB */
	uint16 phymode;			/* last known phy mode; eg. PHYMODE_3x3_1x1 */
	bool upgrade_pending;		/* if upgrade to 4x4 mode from 3+1 is pending */
	chanspec_t sc_chspec;		/* chanspec of scan core */
	chanspec_t sc_last_cleared_chspec;
					/* last cleared chanspec of scan core */
	bool scan_both;			/* true when ISM on main core is running along with
					 * CAC on scan core
					 */
	bool modesw_cb_regd;		/* is modesw callback registered */
	bool move_stunted;		/* prevent channel change at end of dfs_ap_move */
	bool updown_cb_regd;		/* is updown callback registered */
	uint32 txdur_start;		/* tx duration (usecs) at start of CAC */
	uint32 txdur_delta_percent;	/* tx duration during CAC as percentage of CAC time */
	uint32 max_safe_tx_adj;		/* max safe tx on main core if on adjacent channel */
	uint32 max_safe_tx_nonadj;	/* max safe tx on main core if on non-adjacent chan */
	uint32 max_safe_tx_weather;	/* max safe tx on main core if scan on EU weather chan */
	uint32 txblank_check_mode;	/* Control considering tx on main core at end of CAC */
	bool in_eu;			/* whenever the interface goes up, update whether in EU */
	uint8 radar_subbands;		/* The sub-band on which the Radar was detected */
	uint8 chan_cac_pending[MAXCHANNEL];	/* 11h: CAC pending on channel */
	uint32 radar_info[2];           /* radar_info[0]=radar_info radar_info[1]=radar_info_2 */
	uint16 actionfr_retry_counter;
	bool radar_report_timer_running;
	struct wl_timer *radar_report_timer;	/* timer for dfs cac handler */
	bool bw_fallback;
};

/* local functions */
/* module */
static void wlc_dfs_csa_each_up_ap(wlc_info_t *wlc, wl_chan_switch_t *csa, bool docs);
static void wlc_dfs_to_backup_channel(wlc_dfs_info_t *dfs, bool radar_detected);
static void wlc_dfs_send_event(wlc_dfs_info_t *dfs, chanspec_t target_chanspec);
static int wlc_dfs_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
static void wlc_dfs_watchdog(void *ctx);
static int wlc_dfs_up(void *ctx);
static int wlc_dfs_down(void *ctx);
static void wlc_dfs_eu_toggle(wlc_dfs_info_t *dfs, bool in_eu);
static void wlc_dfs_updown_cb(void *ctx, bsscfg_up_down_event_data_t *updown_data);
static int wlc_dfs_get_dfs_status_all(wlc_dfs_info_t *dfs, uint8 *arg, int len);

#ifdef BGDFS
#define DFS_BG_UPGRADE_DELAY_MS		300 /* delay after which upgrade is announced */

#define PHYMODE(w) (((phy_info_t *)WLC_PI(w))->cmni->phymode)

static int wlc_dfs_get_chan_separation(wlc_info_t *wlc, chanspec_t chspec0, chanspec_t chspec1);
static uint32 wlc_dfs_get_max_safe_tx_threshold(wlc_dfs_info_t *dfs, bool adjacent, bool weather);
static bool wlc_dfs_discard_due_to_txblanking(wlc_dfs_info_t *dfs);
static void wlc_dfs_scan_complete_sc(wlc_dfs_info_t *dfs, int reason, int return_home_chan);
static void wlc_dfs_bg_upgrade_phy(wlc_dfs_info_t *dfs);
static void wlc_dfs_bg_downgrade_phy(wlc_dfs_info_t *dfs);
#ifdef WL_MODESW
static int wlc_dfs_bg_upgrade_wlc(wlc_info_t *wlc);
static int wlc_dfs_bg_downgrade_wlc(wlc_info_t *wlc);
#endif /* WL_MODESW */
static int wlc_dfs_bg_scan_prep(wlc_info_t *wlc);
#endif /* BGDFS */

#ifdef RSDB_DFS_SCAN
static bool wlc_dfs_is_phy_blanking_required(chanspec_t scan_chspec, chanspec_t home_chspec);
#endif /* RSDB_DFS_SCAN */

#if defined(RSDB_DFS_SCAN) || defined(BGDFS)
static wlc_info_t * wlc_dfs_other_wlc_if_dual(wlc_info_t *wlc);
static void wlc_dfs_scan_complete(wlc_dfs_info_t *dfs, int reason, int return_home_chan);
static int wlc_dfs_scan_prep(wlc_info_t *wlc);
static void wlc_dfs_scan_start(wlc_dfs_info_t *dfs, chanspec_t scan_chspec);
static void wlc_dfs_scan_cleanup(wlc_dfs_info_t *dfs, bool return_home_chan, bool upgrade);
#endif /* RSDB_DFS_SCAN || BGDFS */

#ifdef BCMDBG
static int wlc_dfs_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* others */
static void wlc_dfs_cacstate_init(wlc_dfs_info_t *dfs);
static int wlc_dfs_timer_init(wlc_dfs_info_t *dfs);
static void wlc_dfs_timer_add(wlc_dfs_info_t *dfs);
static bool wlc_dfs_timer_delete(wlc_dfs_info_t *dfs);
static void wlc_dfs_chanspec_oos(wlc_dfs_info_t *dfs, chanspec_t chanspec);
static chanspec_t wlc_dfs_valid_forced_chanspec(wlc_dfs_info_t *dfs);
static void wlc_dfs_rearrange_channel_list(wlc_dfs_info_t *dfs, chanspec_list_t *ch_list);
static chanspec_t wlc_dfs_chanspec(wlc_dfs_info_t *dfs, bool radar_detected);
static bool wlc_radar_detected(wlc_dfs_info_t *dfs, bool scan_core);
static void wlc_dfs_cacstate_idle_set(wlc_dfs_info_t *dfs);
static void wlc_dfs_cacstate_ism_set(wlc_dfs_info_t *dfs);
static void wlc_dfs_cacstate_ooc_set(wlc_dfs_info_t *dfs, uint target_state);
static void wlc_dfs_cacstate_idle(wlc_dfs_info_t *dfs);
static void wlc_dfs_cacstate_cac(wlc_dfs_info_t *dfs);
static void wlc_dfs_cacstate_ism(wlc_dfs_info_t *dfs);
static void wlc_dfs_cacstate_csa(wlc_dfs_info_t *dfs);
static void wlc_dfs_cacstate_ooc(wlc_dfs_info_t *dfs);
static void wlc_dfs_cacstate_handler(void *arg);
static bool wlc_dfs_validate_forced_param(wlc_info_t *wlc, chanspec_t chanspec);
static chanspec_t wlc_dfs_radar_channel_bw_fallback(wlc_dfs_info_t *dfs);

/* IE mgmt */
#ifdef STA
#ifdef BCMDBG
static int wlc_dfs_bcn_parse_ibss_dfs_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif // endif
#endif /* STA */

#ifdef SLAVE_RADAR
static void wlc_radar_report_handler(void *arg);
static int wlc_radar_report_timer_init(wlc_dfs_info_t *dfs);
#endif // endif

/* Local Data Structures */
static void (*wlc_dfs_cacstate_fn_ary[WL_DFS_CACSTATES])(wlc_dfs_info_t *dfs) = {
	wlc_dfs_cacstate_idle,
	wlc_dfs_cacstate_cac, /* preism_cac */
	wlc_dfs_cacstate_ism,
	wlc_dfs_cacstate_csa,
	wlc_dfs_cacstate_cac, /* postism_cac */
	wlc_dfs_cacstate_ooc, /* preism_ooc */
	wlc_dfs_cacstate_ooc /* postism_ooc */
};

#if defined(BCMDBG) || defined(WLMSG_DFS)
static const char *wlc_dfs_cacstate_str[WL_DFS_CACSTATES] = {
	"IDLE",
	"PRE-ISM Channel Availability Check",
	"In-Service Monitoring(ISM)",
	"Channel Switching Announcement(CSA)",
	"POST-ISM Channel Availability Check",
	"PRE-ISM Out Of Channels(OOC)",
	"POSTISM Out Of Channels(OOC)"
};
#endif /* BCMDBG || WLMSG_DFS */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/*
 * State change function for easier tracking of state changes.
 */
static INLINE void
wlc_dfs_cac_state_change(wlc_dfs_info_t *dfs, uint newstate)
{
	wlc_info_t *wlc = dfs->wlc;
	wlc_cac_event_t *cac_event;
	uint16 event_len;

#if defined(BCMDBG) || defined(WLMSG_DFS)
	WL_DFS(("DFS State %s -> %s\n", wlc_dfs_cacstate_str[dfs->dfs_cac.status.state],
		wlc_dfs_cacstate_str[newstate]));
#endif // endif
	dfs->dfs_cac.status.state = newstate;	/* Controlled (logged) state change */

	event_len = sizeof(wlc_cac_event_t) +
		(DFS_HAS_BACKGROUND_SCAN_CORE(wlc) ? sizeof(wl_dfs_sub_status_t) : 0);
	if (!(cac_event = (wlc_cac_event_t *) MALLOCZ(wlc->osh, event_len))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return;
	}
	/* Prepare and sent event */
	memset(cac_event, 0, event_len);
	cac_event->version = WLC_E_CAC_STATE_VERSION;
	cac_event->length = event_len;
	cac_event->type = WLC_E_CAC_STATE_TYPE_DFS_STATUS_ALL;
	wlc_dfs_get_dfs_status_all(dfs, (uint8*)(&(cac_event->scan_status)),
		(event_len - sizeof(wlc_cac_event_t) + sizeof(wl_dfs_status_all_t)));
	wlc_mac_event(wlc, WLC_E_CAC_STATE_CHANGE, NULL, 0, 0, 0,
		cac_event, event_len);
	MFREE(wlc->osh, cac_event, event_len);
}

/* called when observing toggle between EU to/from non-EU country */
static void
wlc_dfs_eu_toggle(wlc_dfs_info_t *dfs, bool in_eu)
{
	// update in structure
	dfs->in_eu = in_eu;
#ifdef BGDFS
	if (in_eu) {
		dfs->max_safe_tx_nonadj = BGDFS_DISCARD_RESULT_TRAFFIC_PERCENT_NONADJACENT_ETSI;
	} else {
		dfs->max_safe_tx_nonadj = BGDFS_DISCARD_RESULT_TRAFFIC_PERCENT_NONADJACENT_FCC;
	}
#endif /* BGDFS */
}

/* Callback from wl up/down. This Function helps reset on up/down events */
static void
wlc_dfs_updown_cb(void *ctx, bsscfg_up_down_event_data_t *updown_data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_dfs_info_t *dfs;

	ASSERT(wlc);
	ASSERT(updown_data);
	if (wlc->dfs == NULL) {
		return;
	}
	dfs = wlc->dfs;

	WL_DFS(("%s:got callback from updown. interface %s\n",
		__FUNCTION__, (updown_data->up?"up":"down")));

	if (updown_data->up == TRUE) {
		bool in_eu = wlc_is_dfs_eu_uk(wlc);
		if (in_eu != dfs->in_eu) {
			wlc_dfs_eu_toggle(dfs, in_eu);
		}
		return;
	}

	/* when brought down, mark current DFS channel as quiet unless in EU */
	if (WL11H_ENAB(wlc) && dfs->radar &&
			wlc_radar_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC) &&
			!wlc->is_edcrs_eu) {
		wlc_set_quiet_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC);
	}

#if defined(BGDFS) || defined(RSDB_DFS_SCAN)
	if (dfs->dfs_scan->status == DFS_SCAN_S_INPROGESS) {
		WL_DFS(("Aborting dfs_ap_move\n"));
		wlc_dfs_scan_abort(wlc_dfs_other_wlc_if_dual(wlc)->dfs);
	}
#endif /* BGDFS || RSDB_DFS_SCAN */

	dfs->scan_both = FALSE;
	dfs->move_stunted = FALSE;
}

/* module */
wlc_dfs_info_t *
BCMATTACHFN(wlc_dfs_attach)(wlc_info_t *wlc)
{
	wlc_dfs_info_t *dfs;

	if ((dfs = MALLOCZ(wlc->osh, sizeof(wlc_dfs_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	dfs->wlc = wlc;
	dfs->updown_cb_regd = FALSE;
	dfs->modesw_cb_regd = FALSE;

	/* register IE mgmt callbacks */
	/* parse */
#ifdef STA
#ifdef BCMDBG
	/* bcn */
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_IBSS_DFS_ID,
	                         wlc_dfs_bcn_parse_ibss_dfs_ie, dfs) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, ibss dfs in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif
#endif /* STA */

	if (wlc_dfs_timer_init(dfs) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_dfs_timer_init failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#ifdef SLAVE_RADAR
	if (wlc_radar_report_timer_init(dfs) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_radar_report_timer_init failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif

#ifdef BCMDBG
	if (wlc_dump_register(wlc->pub, "dfs", wlc_dfs_dump, dfs) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_dumpe_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif

	/* keep the module registration the last other add module unregistratin
	 * in the error handling code below...
	 */
	if (wlc_module_register(wlc->pub, wlc_dfs_iovars, "dfs", dfs, wlc_dfs_doiovar,
	                        wlc_dfs_watchdog, wlc_dfs_up, wlc_dfs_down) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

#ifndef DYN160_DISABLED
	wlc->pub->_dyn160 = WL_HAS_DYN160(wlc);
#endif /* DYN160_DISABLED */

/* As per current design, if both BGDFS and RSDB_DFS_SCAN are defined, BGDFS gets favored */
#ifdef BGDFS
	dfs->in_eu = FALSE;
	dfs->txblank_check_mode = BGDFS_TXBLANK_CHECK_MODE_ALL;
	dfs->max_safe_tx_adj = BGDFS_DISCARD_RESULT_TRAFFIC_PERCENT_ADJACENT;
	dfs->max_safe_tx_nonadj = BGDFS_DISCARD_RESULT_TRAFFIC_PERCENT_NONADJACENT_FCC;
	dfs->max_safe_tx_weather = BGDFS_DISCARD_RESULT_TRAFFIC_PERCENT_WEATHER_ETSI;
#ifndef BGDFS_DISABLED
	wlc->pub->_bgdfs = DFS_HAS_BACKGROUND_SCAN_CORE(wlc);
#endif /* BGDFS_DISABLED */
#ifdef WL_MODESW
	if (WLC_MODESW_ENAB(wlc->pub)) {
		ASSERT(wlc->modesw != NULL);
		if (wlc_modesw_notif_cb_register(wlc->modesw,
				wlc_dfs_opmode_change_cb, wlc) == BCME_OK) {
			dfs->modesw_cb_regd = TRUE;
		}
	}
#endif /* WL_MODESW */
	if (dfs->dfs_scan == NULL) {
		if ((dfs->dfs_scan = MALLOCZ(wlc->osh, sizeof(*(dfs->dfs_scan)))) == NULL) {
			WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
					wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->pub->osh)));
			ASSERT(0);
			goto fail;
		}
		dfs->dfs_scan->status = DFS_SCAN_S_IDLE;
	}
#elif defined(RSDB_DFS_SCAN)
	/* get from obj registry; if null allocate and set registry; increment reference counter */
	dfs->dfs_scan = (dfs_scan_cmn_t*)obj_registry_get(wlc->objr, OBJR_DFS_SCAN_INFO);
	if (dfs->dfs_scan == NULL) {
		if ((dfs->dfs_scan = MALLOCZ(wlc->osh, sizeof(dfs_scan_cmn_t))) == NULL) {
			WL_ERROR(("wl%d: %s: MALLOC failed, malloced %d bytes\n",
				wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->pub->osh)));
			ASSERT(0);
			goto fail;
		}
		obj_registry_set(wlc->objr, OBJR_DFS_SCAN_INFO, dfs->dfs_scan);
		dfs->dfs_scan->status = DFS_SCAN_S_IDLE;
	}
	(void)obj_registry_ref(wlc->objr, OBJR_DFS_SCAN_INFO);
#endif /* BGDFS, RSDB_DFS_SCAN, ... */

	if (wlc_bsscfg_updown_register(wlc, wlc_dfs_updown_cb, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
				wlc->pub->unit, __FUNCTION__));
		ASSERT(0);
		goto fail;
	}
	dfs->updown_cb_regd = TRUE;

#ifdef WL_MODESW
	/* Register callback to the mutx state notification list */
	wlc_modesw_mutx_state_upd_register(wlc);
#endif /* WL_MODESW */

	return dfs;

	/* error handling */
fail:
	wlc_dfs_detach(dfs);
	return NULL;
}

void
BCMATTACHFN(wlc_dfs_detach)(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc;

	if (dfs == NULL) {
		return;
	}

	wlc = dfs->wlc;

#ifdef WL_MODESW
	/* Unregister callback function to mutx state notification */
	wlc_modesw_mutx_state_upd_unregister(wlc);
#endif /* WL_MODESW */

	if (dfs->updown_cb_regd && wlc_bsscfg_updown_unregister(wlc,
			wlc_dfs_updown_cb, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_unregister() failed\n",
				wlc->pub->unit, __FUNCTION__));
		dfs->updown_cb_regd = FALSE;
	}
#if defined(BGDFS) && defined(WL_MODESW)
	/* modesw is always detached before dfs; but wlc->modesw is not NULL */
	if (BGDFS_ENAB(wlc->pub) && WLC_MODESW_ENAB(wlc->pub) &&
			dfs->modesw_cb_regd && wlc->modesw) {
		wlc_modesw_notif_cb_unregister(wlc->modesw, wlc_dfs_opmode_change_cb, wlc);
		dfs->modesw_cb_regd = FALSE;
	}
#endif /* BGDFS && WL_MODESW */

	wlc_module_unregister(wlc->pub, "dfs", dfs);

	if (dfs->dfs_timer != NULL)
		wl_free_timer(wlc->wl, dfs->dfs_timer);

	if (dfs->dfs_cac.chanspec_forced_list)
		MFREE(wlc->osh, dfs->dfs_cac.chanspec_forced_list,
			WL_CHSPEC_LIST_FIXED_SIZE +
			dfs->dfs_cac.chanspec_forced_list->num * sizeof(chanspec_t));
#ifdef BGDFS
	if (dfs->dfs_scan != NULL) {
		MFREE(wlc->osh, dfs->dfs_scan, sizeof(*(dfs->dfs_scan)));
	}
#elif defined(RSDB_DFS_SCAN)
	if (obj_registry_unref(wlc->objr, OBJR_DFS_SCAN_INFO) == 0) {
		if (dfs->dfs_scan != NULL) {
			MFREE(wlc->osh, dfs->dfs_scan, sizeof(dfs_scan_cmn_t));
		}
		obj_registry_set(wlc->objr, OBJR_DFS_SCAN_INFO, NULL);
	}
#endif /* RSDB_DFS_SCAN */

	MFREE(wlc->osh, dfs, sizeof(wlc_dfs_info_t));
}

/* Validate chanspec forced list configuration. */
static bool wlc_dfs_validate_forced_param(wlc_info_t *wlc, chanspec_t chanspec)
{
	if (!WL11H_ENAB(wlc) || !CHSPEC_IS5G(chanspec) ||
		wf_chspec_malformed(chanspec) ||
		(!wlc_valid_chanspec_db(wlc->cmi, chanspec)) ||
		(!N_ENAB(wlc->pub) && (CHSPEC_IS40(chanspec) ||
		CHSPEC_IS80(chanspec) || CHSPEC_IS8080(chanspec) ||
		CHSPEC_IS160(chanspec)))) {
		return FALSE;
	}

	return TRUE;
}

/*
 * wlc_dfs_csa_each_up_ap - announce CSA on each AP interface that is up
 * wlc - handle to the wl interface
 * csa - parameters for channel switch announcement
 * docs - when true, channel switch will be effected immediately without waiting for
 *        countdown to finish
 */
static void
wlc_dfs_csa_each_up_ap(wlc_info_t *wlc, wl_chan_switch_t *csa, bool docs)
{
	wlc_bsscfg_t *apcfg;
	int idx;
	WL_DFS(("Announcing CSA to chanspec 0x%x on all up APs\n", csa->chspec));
	FOREACH_UP_AP(wlc, idx, apcfg) {
		wlc_csa_do_csa(wlc->csa, apcfg, csa, docs);
	}
}

static int
wlc_dfs_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_dfs_info_t *dfs = (wlc_dfs_info_t *)ctx;
	wlc_info_t *wlc = dfs->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = BCME_OK;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	bool bool_val;
	bool bool_val2;
	uint i;
	wl_dfs_forced_t *dfs_forced;
	chanspec_t chanspec = 0;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (p_len >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)), &int_val2, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0) ? TRUE : FALSE;
	bool_val2 = (int_val2 != 0) ? TRUE : FALSE;
	BCM_REFERENCE(bool_val2);

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	/* Do the actual parameter implementation */
	switch (actionid) {
	case IOV_GVAL(IOV_DFS_PREISM):
		*ret_int_ptr = dfs->dfs_cac.cactime_pre_ism;
		break;

	case IOV_SVAL(IOV_DFS_PREISM):
		if ((int_val < -1) || (int_val >= WLC_DFS_CAC_TIME_SEC_MAX)) {
			err = BCME_RANGE;
			break;
		}
		dfs->dfs_cac.cactime_pre_ism = int_val;
		break;

	case IOV_GVAL(IOV_DFS_POSTISM):
		*ret_int_ptr = dfs->dfs_cac.cactime_post_ism;
		break;

	case IOV_SVAL(IOV_DFS_POSTISM):
		if ((int_val < -1) || (int_val >= WLC_DFS_CAC_TIME_SEC_MAX)) {
			err = BCME_RANGE;
			break;
		}
		dfs->dfs_cac.cactime_post_ism = int_val;
		break;

	case IOV_GVAL(IOV_DFS_STATUS):
		dfs->dfs_cac.status.duration =
		        (dfs->dfs_cac.cactime - dfs->dfs_cac.duration) *
		        WLC_DFS_RADAR_CHECK_INTERVAL;
		bcopy((char *)&dfs->dfs_cac.status, (char *)arg, sizeof(wl_dfs_status_t));
		break;

	case IOV_SVAL(IOV_DFS_ISM_MONITOR):
		dfs->dfs_cac.ism_monitor = bool_val;
		break;

	case IOV_GVAL(IOV_DFS_ISM_MONITOR):
		*ret_int_ptr = (int32)dfs->dfs_cac.ism_monitor;
		break;

	/* IOV_DFS_CHANNEL_FORCED is required for regulatory testing */
	case IOV_SVAL(IOV_DFS_CHANNEL_FORCED):
		if (p_len <= (int)sizeof(int_val)) {
			/* Validate Configuration */
			if (!wlc_dfs_validate_forced_param(wlc, (chanspec_t)int_val)) {
				err = BCME_BADCHAN;
				break;
			}
			/* Single configuration is required for regulatory testing */
			if (!dfs->dfs_cac.chanspec_forced_list) {
				dfs->dfs_cac.chanspec_forced_list =
					MALLOCZ(wlc->osh, (WL_CHSPEC_LIST_FIXED_SIZE +
					1 * sizeof(chanspec_t)));

			if (!dfs->dfs_cac.chanspec_forced_list) {
					err = BCME_NOMEM;
					break;
				}
			}
			dfs->dfs_cac.chanspec_forced_list->num = 1;

			if (CHSPEC_CHANNEL((chanspec_t)int_val) == 0) {
				/* This is to handle old command syntax - single chanspec */
				dfs->dfs_cac.chanspec_forced_list->list[0] =
					wlc_create_chspec(wlc, (uint8)int_val);
				break;
			}
			dfs->dfs_cac.chanspec_forced_list->list[0] =
				(chanspec_t)int_val;
		} else {
			dfs_forced = (wl_dfs_forced_t*)params;
			if (dfs_forced->version == DFS_PREFCHANLIST_VER) {
				/* We got list configuration */
				if (dfs_forced->chspec_list.num > WL_NUMCHANSPECS) {
					err = BCME_BADCHAN;
					break;
				}
				/* Validate Configuration */
				for (i = 0; i < dfs_forced->chspec_list.num; i++) {
					if (N_ENAB(wlc->pub)) {
						chanspec = dfs_forced->chspec_list.list[i];
					} else {
						chanspec = wlc_create_chspec(wlc,
							(uint8)dfs_forced->chspec_list.list[i]);
					}
					if (!wlc_dfs_validate_forced_param(wlc, chanspec)) {
						err = BCME_BADCHAN;
						return err;
					}
				}
				/* Clears existing list if any */
				if (dfs->dfs_cac.chanspec_forced_list) {
					MFREE(wlc->osh, dfs->dfs_cac.chanspec_forced_list,
					  WL_CHSPEC_LIST_FIXED_SIZE +
					  dfs->dfs_cac.chanspec_forced_list->num *
					  sizeof(chanspec_t));
					dfs->dfs_cac.chanspec_forced_list = NULL;
				}
				if (!dfs_forced->chspec_list.num) {
					/* Clear configuration done */
					break;
				}
				dfs->dfs_cac.chanspec_forced_list =
					MALLOCZ(wlc->osh, (WL_CHSPEC_LIST_FIXED_SIZE +
					dfs_forced->chspec_list.num * sizeof(chanspec_t)));

				if (!dfs->dfs_cac.chanspec_forced_list) {
					err = BCME_NOMEM;
					break;
				}
				dfs->dfs_cac.chanspec_forced_list->num =
					dfs_forced->chspec_list.num;

				for (i = 0; i < dfs_forced->chspec_list.num; i++) {
					if (N_ENAB(wlc->pub)) {
						chanspec = dfs_forced->chspec_list.list[i];
					} else {
						chanspec = wlc_create_chspec(wlc,
							(uint8)dfs_forced->chspec_list.list[i]);
					}
					dfs->dfs_cac.chanspec_forced_list->list[i] = chanspec;
				}
			} else {
				err = BCME_VERSION;
				break;
			}
		}
		break;

	case IOV_GVAL(IOV_DFS_CHANNEL_FORCED):
		if (p_len < sizeof(wl_dfs_forced_t)) {
			if (dfs->dfs_cac.chanspec_forced_list) {
				*ret_int_ptr = dfs->dfs_cac.chanspec_forced_list->list[0];
			} else
				*ret_int_ptr = 0;
		} else {
			wl_dfs_forced_t *inp = (wl_dfs_forced_t*)params;
			dfs_forced = (wl_dfs_forced_t*)arg;
			/* This is issued by new application */
			if (inp->version == DFS_PREFCHANLIST_VER) {
				dfs_forced->version = DFS_PREFCHANLIST_VER;
				if (dfs->dfs_cac.chanspec_forced_list) {
					uint nchan = dfs->dfs_cac.chanspec_forced_list->num;
					uint ioctl_size = WL_DFS_FORCED_PARAMS_FIXED_SIZE +
						dfs->dfs_cac.chanspec_forced_list->num *
						sizeof(chanspec_t);

					if (p_len < ioctl_size) {
						err = BCME_BUFTOOSHORT;
						break;
					}
					for (i = 0; i < nchan; i++) {
						chanspec =
							dfs->dfs_cac.chanspec_forced_list->list[i];
						dfs_forced->chspec_list.list[i] = chanspec;
					}
					dfs_forced->chspec_list.num = nchan;
					/* Rearranges channel list, bringing valid (non current)
					 * channels to head of the list
					 */
					wlc_dfs_rearrange_channel_list(dfs,
							&dfs_forced->chspec_list);
					break;
				} else {
					dfs_forced->chspec_list.num = 0;
					/* wlu reads dfs_forced->chspec when list is not there.
					 * So assigning it with 0 otherwise wlu reports junk.
					 * Driver does not use chspec, it uses chspec_list instead.
					 */
					dfs_forced->chspec = 0;
				}
			} else {
				err = BCME_VERSION;
				break;
			}
		}
		break;
#if defined(BGDFS) || defined(RSDB_DFS_SCAN)
	case IOV_SVAL(IOV_DFS_AP_MOVE):
	{
		wl_chan_switch_t csa;
		chanspec_t chspec = (chanspec_t)int_val;
		bool sc_unavail_160 = D11REV_IS(wlc->pub->corerev, 0x40) ||
				D11REV_IS(wlc->pub->corerev, 0x41);
		bool bw_160_80p80 = CHSPEC_IS160(WLC_BAND_PI_RADIO_CHANSPEC) ||
				CHSPEC_IS8080(WLC_BAND_PI_RADIO_CHANSPEC);

		if (wlc_11h_get_spect(wlc->m11h) == SPECT_MNGMT_OFF) {
			WL_DFS(("Rejecting dfs_ap_move 11h (spect) spectrum management is OFF\n"));
			err = BCME_NOTENABLED;
			break;
		}

		if (wlc->pub->_ap != 1) {
			WL_DFS(("Rejecting dfs_ap_move wlc->pub->_ap=%d\n", wlc->pub->_ap));
			err = BCME_NOTAP;
			break;
		}

		if (!VHT_ENAB(wlc->pub)) {
			WL_DFS(("Rejecting dfs_ap_move since not 11ac capable\n"));
			err = BCME_UNSUPPORTED;
			break;
		}

		/* scan core cant be used when operating on 160Mhz/80p80 for 4366B1 and C0 chips */
		if (bw_160_80p80 && sc_unavail_160) {
			WL_DFS(("Rejecting BGDFS if operating on 160/80p80\n"));
			err = BCME_UNSUPPORTED;
			break;
		}

		if (!RSDB_ENAB(wlc->pub) && !BGDFS_ENAB(wlc->pub)) {
			err = BCME_UNSUPPORTED;
			break;
		}
		if (!AP_ACTIVE(wlc) || !BSSCFG_AP(bsscfg)) {
			err = BCME_NOTAP;
			break;
		}
		if ((int16)chspec == -1) {
			WL_DFS(("Abort dfs_ap_move request in state %d\n",
					dfs->dfs_scan->status));
			if (dfs->dfs_scan->status == DFS_SCAN_S_INPROGESS) {
				WL_DFS(("Aborting dfs_ap_move\n"));
				wlc_dfs_scan_abort(wlc_dfs_other_wlc_if_dual(wlc)->dfs);
				break;
			} else if (dfs->dfs_scan->status ==
					DFS_SCAN_S_SCAN_MODESW_INPROGRESS) {
				WL_DFS(("Can not abort dfs_ap_move in middle of modesw; retry\n"));
				err = BCME_BUSY;
				break;
			}
		}
		/* helps test scan core without really effecting a move/channel change;
		 * affects last dfs_ap_move in progress only
		 */
		if ((int16)chspec == -2) {
			WL_DFS(("Stunt recent dfs_ap_move request in state %d\n",
					dfs->dfs_scan->status));
			dfs->move_stunted = TRUE;
#ifdef WL_AP_CHAN_CHANGE_EVENT
			wlc_channel_send_chan_event(wlc, WL_CHAN_REASON_DFS_AP_MOVE_STUNT,
				WLC_BAND_PI_RADIO_CHANSPEC);
#endif /* WL_AP_CHAN_CHANGE_EVENT */
			break;
		}

		WL_DFS(("%s scan channel: %04x, current channel %04x\n", __FUNCTION__,
				chspec, WLC_BAND_PI_RADIO_CHANSPEC));

		/* reject if the chanspec is already current radio chanspec */
		if (chspec == WLC_BAND_PI_RADIO_CHANSPEC) {
			err = BCME_BADCHAN;
			break;
		}
		if (wf_chspec_malformed(chspec) || !wlc_radar_chanspec(wlc->cmi, chspec)) {
			err = BCME_BADCHAN;
			break;
		}
		if (CHSPEC_BW(chspec) != CHSPEC_BW(WLC_BAND_PI_RADIO_CHANSPEC)) {
			WL_DFS(("Scan channel bandwidth %d must match current bandwidth %d\n",
					CHSPEC_BW(chspec),
					CHSPEC_BW(WLC_BAND_PI_RADIO_CHANSPEC)));
			err = BCME_BADCHAN;
			break;
		}
		if (!wlc_valid_dfs_chanspec(wlc, chspec)) {
			WL_DFS(("Scan channel 0x%x Not Available\n", chspec));
			err = BCME_BADCHAN;
			break;
		}

		csa.mode = DOT11_CSA_MODE_ADVISORY;
		csa.chspec = chspec;
		csa.count = bsscfg->current_bss->dtim_period;
		csa.reg = wlc_get_regclass(wlc->cmi, csa.chspec);
		csa.frame_type = CSA_BROADCAST_ACTION_FRAME;

		if (wlc->is_edcrs_eu && !wlc_quiet_chanspec(wlc->cmi, chspec)) {
			WL_DFS(("Skip CAC - channel 0x%x is already available (ETSI). To CSA\n",
					chspec));
			wlc_dfs_csa_each_up_ap(wlc, &csa, FALSE);
		} else {
#ifdef RXCHAIN_PWRSAVE
			/* If AP is currently in rxchain power save, come out of it */
			if (wlc_ap_in_rxchain_power_save(wlc->ap)) {
				wlc_reset_rxchain_pwrsave_mode(wlc->ap);
			}
#endif /* RXCHAIN_PWRSAVE */
			err = wlc_dfs_scan(dfs, &csa);
			if (err != BCME_OK) {
				WL_DFS(("DFS_AP_MOVE start failed, could not send event \n"));
				break;
			}
		}
#ifdef WL_AP_CHAN_CHANGE_EVENT
		wlc_channel_send_chan_event(wlc, WL_CHAN_REASON_DFS_AP_MOVE_START, chspec);
#endif /* WL_AP_CHAN_CHANGE_EVENT */
		break;
	}
	case IOV_GVAL(IOV_DFS_AP_MOVE):
	{
		wl_dfs_ap_move_status_t * ap_move_status = (wl_dfs_ap_move_status_t *)arg;
		if (len < sizeof(wl_dfs_ap_move_status_t)) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		memset(ap_move_status, 0, sizeof(wl_dfs_ap_move_status_t));

		ap_move_status->version = (int8)WL_DFS_AP_MOVE_VERSION;
		ap_move_status->move_status = (int8)dfs->dfs_scan->status;
		ap_move_status->chanspec = dfs->dfs_scan->csa.chspec;
		err = wlc_dfs_get_dfs_status_all(dfs, (uint8*)(&ap_move_status->scan_status),
				len-sizeof(wl_dfs_ap_move_status_t)+sizeof(wl_dfs_status_all_t));
		WL_DFS(("dfs_scan->status=%d move_status=%d\n", dfs->dfs_scan->status,
				ap_move_status->move_status));

		break;
	}
#endif /* BGDFS || RSDB_DFS_SCAN */

	case IOV_GVAL(IOV_DFS_STATUS_ALL):
		err = wlc_dfs_get_dfs_status_all(dfs, (uint8*)arg, len);
		break;

	case IOV_GVAL(IOV_DYN160):
#if defined(DYN160) && !defined(DYN160_DISABLED)
		*ret_int_ptr = DYN160_ACTIVE(wlc->pub);
#else
		err = BCME_UNSUPPORTED;
#endif /* DYN160 && DYN160_DISABLED */
		break;

	case IOV_SVAL(IOV_DYN160):
#if defined(DYN160) && !defined(DYN160_DISABLED)
		DYN160_ACTIVE_SET(wlc->pub, (uint32) int_val);
#else
		err = BCME_UNSUPPORTED;
#endif /* DYN160 && DYN160_DISABLED */
		break;
#ifdef WL_DFS_WAVE_MODE
	case IOV_SVAL(IOV_DFS_TEST_MODE):
		if (bool_val) {
			WL_DFS(("wl%d: Enabling DFS test mode. If radar is detected,"
				"the channel will not be changed.\n", wlc->pub->unit));
			if ((dfs->dfs_cac.cactime_post_ism != 0) ||
					(dfs->dfs_cac.cactime_pre_ism != 0)) {
				WL_DFS(("wl%d: IOV_DFS_PREISM and IOV_DFS_POSTISM should"
					" both be set to 0 while in DFS test mode.\n",
					wlc->pub->unit));
			}
		}
		dfs->dfs_cac.test_mode = bool_val;
		break;

	case IOV_GVAL(IOV_DFS_TEST_MODE):
		*ret_int_ptr = (int32)dfs->dfs_cac.test_mode;
		break;
#endif /* WL_DFS_WAVE_MODE */
	case IOV_GVAL(IOV_DFS_BW_FALLBACK):
		*ret_int_ptr = (int32) dfs->bw_fallback;
		break;

	case IOV_SVAL(IOV_DFS_BW_FALLBACK):
		dfs->bw_fallback = (uint32) int_val;
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

#if defined(BGDFS) && defined(WL_MODESW)
/* Initiates upgrade of wlc to full MIMO (4x4) using modesw module. */
static int
wlc_dfs_bg_upgrade_wlc(wlc_info_t *wlc)
{
	int idx = 0;
	wlc_bsscfg_t *bsscfg;
	uint8 new_oper_mode = 0, curr_oper_mode, bw = 0, nss;
	bool mode_switch_sched = FALSE, is160_8080 = FALSE;
	int err = BCME_UNSUPPORTED;

	wlc->dfs->phymode = PHYMODE(wlc);

	/* announce upgrade */
	FOREACH_UP_AP(wlc, idx, bsscfg)
	{
		bw = DOT11_OPER_MODE_80MHZ;
		if (WLC_BSS_CONNECTED(bsscfg)) {
			curr_oper_mode = wlc_modesw_derive_opermode(wlc->modesw,
					bsscfg->current_bss->chanspec, bsscfg,
					wlc->stf->op_rxstreams);
			bw = DOT11_OPER_MODE_CHANNEL_WIDTH(curr_oper_mode);
			nss = DOT11_OPER_MODE_RXNSS(curr_oper_mode);
			is160_8080 = DOT11_OPER_MODE_160_8080(curr_oper_mode);
			if (nss == WLC_BITSCNT(wlc->stf->valid_txchain_mask))
				continue;
		}

		new_oper_mode = DOT11_D8_OPER_MODE(0,
			WLC_BITSCNT(wlc->stf->valid_txchain_mask), 0, is160_8080, bw);
		err = wlc_modesw_handle_oper_mode_notif_request(wlc->modesw, bsscfg,
				new_oper_mode, TRUE, MODESW_CTRL_OPMODE_IE_REQD_OVERRIDE);
		if (err != BCME_OK) {
			WL_DFS(("wl%d: failed to request modesw %d\n", wlc->pub->unit, err));
			break;
		}
		mode_switch_sched = TRUE;
	}

	WL_DFS(("wl%d: mode switch up scheduled = %d opmode: 0x%02x, bw: 0x%02x\n",
			wlc->pub->unit, mode_switch_sched, new_oper_mode, bw));

	return mode_switch_sched ? BCME_BUSY : BCME_OK;
}

/* Initiates downgrade of wlc to reduced MIMO plus scan core (3+1) using modesw module. */
static int
wlc_dfs_bg_downgrade_wlc(wlc_info_t *wlc)
{
	int idx = 0;
	wlc_bsscfg_t *bsscfg;
	uint8 new_oper_mode = 0, curr_oper_mode, bw = 0, nss;
	bool mode_switch_sched = FALSE, is160_8080 = FALSE;
	int err = BCME_UNSUPPORTED;

	wlc->dfs->phymode = PHYMODE(wlc);

	if (!wlc->cmn->max_rateset) {
		if ((err = wlc_modesw_init_max_rateset(wlc->modesw, 0)) != BCME_OK) {
			// log and continue; don't return
			WL_DFS(("wl%d: failed to max rateset %d\n", wlc->pub->unit, err));
		}
	}

	FOREACH_UP_AP(wlc, idx, bsscfg)
	{
		bw = DOT11_OPER_MODE_80MHZ;
		if (WLC_BSS_CONNECTED(bsscfg)) {
			curr_oper_mode = wlc_modesw_derive_opermode(wlc->modesw,
					bsscfg->current_bss->chanspec, bsscfg,
					wlc->stf->op_rxstreams);
			bw = DOT11_OPER_MODE_CHANNEL_WIDTH(curr_oper_mode);
			nss = DOT11_OPER_MODE_RXNSS(curr_oper_mode);
			is160_8080 = DOT11_OPER_MODE_160_8080(curr_oper_mode);
			if (nss == WLC_BITSCNT(wlc->stf->valid_txchain_mask)-1)
				continue;
		}
		new_oper_mode = DOT11_D8_OPER_MODE(0, WLC_BITSCNT(wlc->stf->valid_txchain_mask)-1,
				0, is160_8080, bw);
		err = wlc_modesw_handle_oper_mode_notif_request(wlc->modesw, bsscfg,
				new_oper_mode, TRUE, MODESW_CTRL_OPMODE_IE_REQD_OVERRIDE);
		if (err != BCME_OK) {
			WL_DFS(("wl%d: failed to request modesw %d\n", wlc->pub->unit, err));
			break;
		}
		mode_switch_sched = TRUE;
	}

	WL_DFS(("wl%d: mode switch down scheduled = %d opmode: 0x%02x, bw: 0x%02x\n",
			wlc->pub->unit, mode_switch_sched, new_oper_mode, bw));

	return mode_switch_sched ? BCME_BUSY : BCME_OK;
}
#endif /* BGDFS && WL_MODESW */

#ifdef BGDFS
/* Chooses the cores on which RADAR scan is required and initiates downgrade as required */
static int
wlc_dfs_bg_scan_prep(wlc_info_t *wlc)
{
	wlc_dfs_info_t *dfs = wlc->dfs;
	dfs->phymode = PHYMODE(wlc);

	/* ISM must continue on main core while scan core goes into CAC */
	dfs->scan_both = wlc_radar_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC);

	if (dfs->phymode == PHYMODE_3x3_1x1) {
		return BCME_OK;
	}

#ifdef WL_MODESW
	if (!dfs->modesw_cb_regd) {
		if (wlc->modesw && wlc_modesw_notif_cb_register(wlc->modesw,
				wlc_dfs_opmode_change_cb, wlc) == BCME_OK) {
			dfs->modesw_cb_regd = TRUE;
		} else {
			WL_ERROR(("wl%d: %s: Could not initialize modesw notification callback\n",
					wlc->pub->unit, __FUNCTION__));
		}
	}
	/* downgrade from 4x4 to 3x3 + 1 radar scan core mode after announcing */
	if (dfs->modesw_cb_regd &&
			wlc_dfs_bg_downgrade_wlc(wlc) == BCME_BUSY) {
		/* downgrade phy later */
		return BCME_BUSY;
	}
	else
#endif /* WL_MODESW */
	{
		/* downgrade phy right away */
		wlc_dfs_bg_downgrade_phy(dfs);
		return BCME_OK;
	}
}

/* Downgrades PHY to enable scan core on a specific channel;
 * disables watchdog to avoid periodic calibration
 */
static void
wlc_dfs_bg_downgrade_phy(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;

	ASSERT(dfs->dfs_scan);

	/* disable periodic calibration; so that it doesn't interfere throughout CAC */
	wlc_phy_watchdog_override((phy_info_t *)WLC_PI(wlc), FALSE);

	wlc_mute(wlc, ON, PHY_MUTE_FOR_PREISM);
	wlc_phy_set_val_sc_chspec((phy_info_t *)WLC_PI(wlc), dfs->dfs_scan->csa.chspec);
	wlc_phy_set_val_phymode((phy_info_t *)WLC_PI(wlc), PHYMODE_3x3_1x1);
	dfs->phymode = PHYMODE(wlc);
	dfs->upgrade_pending = TRUE;
	wlc_mute(wlc, OFF, PHY_MUTE_FOR_PREISM);

	WL_DFS(("wl%d: downgraded phy to 3+1\n", wlc->pub->unit));
	ASSERT(dfs->phymode == PHYMODE_3x3_1x1);
}

/* Upgrade PHY to disable scan core and move to full MIMO (4x4) mode; enables watchdog */
static void
wlc_dfs_bg_upgrade_phy(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;

	ASSERT(dfs->dfs_scan);

	wlc_mute(wlc, ON, PHY_MUTE_FOR_PREISM);
	wlc_phy_set_val_phymode((phy_info_t *)WLC_PI(wlc), 0);
	dfs->phymode = PHYMODE(wlc);
	dfs->upgrade_pending = FALSE;
	wlc_mute(wlc, OFF, PHY_MUTE_FOR_PREISM);

	/* enable periodic calibration */
	wlc_phy_watchdog_override((phy_info_t *)WLC_PI(wlc), TRUE);

	WL_DFS(("wl%d: upgraded phy to 4x4\n", wlc->pub->unit));
	ASSERT(dfs->phymode == 0);
}

#ifdef WL_MODESW
/* callback registered to mode switch module; called on upgrade/downgrade */
void
wlc_dfs_opmode_change_cb(void *ctx, wlc_modesw_notif_cb_data_t *notif_data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_dfs_info_t *dfs;
#if defined(BCMDBG) || defined(WLMSG_MODESW)
	wlc_bsscfg_t *bsscfg = NULL;
#endif /* BCMDBG || WLMSG_MODESW */
	ASSERT(wlc);
	ASSERT(notif_data);
	if (wlc->dfs == NULL) {
		return;
	}
	dfs = wlc->dfs;

	WL_MODE_SWITCH(("wl%d: MODESW Callback status: %d, opmode: %x, signal: %d, state: %d\n",
			WLCWLUNIT(wlc), notif_data->status, notif_data->opmode, notif_data->signal,
			dfs->dfs_scan->modeswitch_state));
#if defined(BCMDBG) || defined(WLMSG_MODESW)
	bsscfg = notif_data->cfg;
#endif /* BCMDBG || WLMSG_MODESW */
	switch (notif_data->signal) {
		case MODESW_PHY_DN_COMPLETE:
		case MODESW_PHY_UP_COMPLETE:
			{
				int dfs_modesw_state = dfs->dfs_scan->modeswitch_state;
				if (dfs_modesw_state == DFS_MODESW_DOWNGRADE_IN_PROGRESS) {
					wlc_dfs_bg_downgrade_phy(dfs);
				} else if (dfs_modesw_state == DFS_MODESW_UPGRADE_IN_PROGRESS) {
					wlc_dfs_bg_upgrade_phy(dfs);
				}
				WL_MODE_SWITCH(("wl%d: Changed phy mode to (%d) by cfg = %d\n",
						WLCWLUNIT(wlc), dfs->phymode, bsscfg->_idx));
			}
			break;
		case MODESW_PHY_UP_START:
			WL_MODE_SWITCH(("wl%d: Changed chip phy mode to (%d) by cfg = %d\n",
					WLCWLUNIT(wlc), PHYMODE_MIMO, bsscfg->_idx));
			break;
	}

	wlc_dfs_handle_modeswitch(dfs, notif_data->signal);

	return;
}
#endif /* WL_MODESW */

#endif /* BGDFS */

#ifdef RSDB_DFS_SCAN
static bool
wlc_dfs_is_phy_blanking_required(chanspec_t scan_chspec, chanspec_t home_chspec)
{
	int scan_freq;
	int home_freq;
	int separation;

	/* phy separation is required only when both chanspecs are 5G */
	if (CHSPEC_IS2G(home_chspec) || CHSPEC_IS2G(scan_chspec)) {
		return FALSE;
	}

	scan_freq = wf_channel2mhz(CHSPEC_CHANNEL(scan_chspec), CHSPEC_IS5G(scan_chspec)
		? WF_CHAN_FACTOR_5_G : WF_CHAN_FACTOR_2_4_G);
	home_freq = wf_channel2mhz(CHSPEC_CHANNEL(home_chspec), CHSPEC_IS5G
		(home_chspec) ? WF_CHAN_FACTOR_5_G : WF_CHAN_FACTOR_2_4_G);
	separation = ABS(home_freq - scan_freq);

	if ((CHSPEC_IS20(home_chspec) && (separation < 80)) ||
			(CHSPEC_IS40(home_chspec) && (separation < 160)) ||
			(CHSPEC_IS80(home_chspec) && (separation < 240))) {
		return TRUE;
	}

	return FALSE;
}
#endif /* RSDB_DFS_SCAN */

#if defined(RSDB_DFS_SCAN) || defined(BGDFS)
/* apply/initiate any mode changes required before DFS scan */
static int
wlc_dfs_scan_prep(wlc_info_t *wlc)
{
	int ret_val = 0;
#ifdef BGDFS
	if (BGDFS_ENAB(wlc->pub)) {
		ret_val = wlc_dfs_bg_scan_prep(wlc);
	}
#elif defined(RSDB_DFS_SCAN)
	if (RSDB_ENAB(wlc->pub)) {
		ret_val = wlc_rsdb_dfs_scan_prep(wlc);
	}
#endif /* RSDB_DFS_SCAN, BGDFS, ... */
	return ret_val;
}

int
wlc_dfs_scan_in_progress(wlc_dfs_info_t *dfs)
{
	return DFS_SCAN_IN_PROGRESS(dfs);
}

void
wlc_dfs_scan_abort(wlc_dfs_info_t *dfs)
{
#ifdef BGDFS
	if (BGDFS_ENAB(dfs->wlc->pub) && dfs->phymode == PHYMODE_3x3_1x1) {
		wlc_dfs_scan_complete_sc(dfs, DFS_SCAN_S_SCAN_ABORTED, TRUE);
		wlc_dfs_bg_upgrade_phy(dfs);
	}
	else
#endif // endif
	{
		wlc_dfs_scan_complete(dfs, DFS_SCAN_S_SCAN_ABORTED, TRUE);
	}
}

int
wlc_dfs_scan(wlc_dfs_info_t *dfs, wl_chan_switch_t *csa)
{
	wlc_info_t *wlc = dfs->wlc;
	int ret_val = 0;
	ASSERT(wlc->pub->_ap);

	ASSERT(csa != NULL);

	memcpy(&dfs->dfs_scan->csa, csa, sizeof(wl_chan_switch_t));
	WL_DFS(("wl%d %s %d chspec=%x\n", wlc->pub->unit, __FUNCTION__, __LINE__,
		csa->chspec));

	/* request for wlc downgrade if in MIMO mode. */
	ret_val = wlc_dfs_scan_prep(wlc);
	dfs->dfs_scan->status = DFS_SCAN_S_IDLE;
	dfs->move_stunted = FALSE;

	if (ret_val == BCME_OK) {
		/* Start dfs scan (on other_wlc if RSDB) */
		wlc_dfs_scan_start(wlc_dfs_other_wlc_if_dual(wlc)->dfs, csa->chspec);
	} else if (ret_val == BCME_BUSY) {
		/* Wait for protocol & HW modeswitch to complete. */
		dfs->dfs_scan->modeswitch_state = DFS_MODESW_DOWNGRADE_IN_PROGRESS;
		dfs->dfs_scan->status = DFS_SCAN_S_SCAN_MODESW_INPROGRESS;
	} else {
		return ret_val;
	}

	return BCME_OK;
}

void
wlc_dfs_handle_modeswitch(wlc_dfs_info_t *dfs, uint new_state)
{
	switch (dfs->dfs_scan->modeswitch_state)
	{
		case DFS_MODESW_IDLE:
			WL_DFS(("wl%d %s: new state %d in idle state\n", dfs->wlc->pub->unit,
					__FUNCTION__, new_state));
			break;

		case DFS_MODESW_DOWNGRADE_IN_PROGRESS:
			if (new_state == MODESW_DN_AP_COMPLETE) {
				dfs->dfs_scan->modeswitch_state = DFS_MODESW_DOWNGRADE_IN_FINISHED;
				WL_DFS(("wl%d %s: downgrade completed %d\n", dfs->wlc->pub->unit,
					__FUNCTION__, __LINE__));
				/* Start dfs scan (on other wlc if RSDB) */
				wlc_dfs_scan_start(wlc_dfs_other_wlc_if_dual(dfs->wlc)->dfs,
					dfs->dfs_scan->csa.chspec);
			}
			break;

		case DFS_MODESW_UPGRADE_IN_PROGRESS:
			if (new_state == MODESW_UP_AP_COMPLETE ||
					new_state == MODESW_DN_AP_COMPLETE) {
				dfs->dfs_scan->modeswitch_state = DFS_MODESW_IDLE;
				WL_DFS(("wl%d %s: upgrade completed %d\n", dfs->wlc->pub->unit,
					__FUNCTION__, __LINE__));
			}
			break;

		default:
			break;
	}
}

static void
wlc_dfs_scan_start(wlc_dfs_info_t *dfs, chanspec_t scan_chspec)
{
	wlc_info_t *wlc = dfs->wlc;
#ifndef BGDFS
	wlc_info_t *other_wlc = NULL;
	bool phy_blank = FALSE;
#endif /* ndef BGDFS */

#ifdef BGDFS

	if (BGDFS_ENAB(wlc->pub)) {
		/* when called in BGDFS mode use the scan core for CAC */
		dfs->dfs_scan->inprogress = TRUE;
		dfs->dfs_scan->scan_wlc = wlc;
		dfs->dfs_scan->status = DFS_SCAN_S_INPROGESS;
		dfs->sc_chspec = scan_chspec;
		wlc_set_dfs_cacstate(dfs, ON);
		dfs->dfs_scan->_was_ap = wlc->pub->_ap;
		dfs->txdur_start = wlc_bmac_cca_read_counter(wlc->hw, M_CCA_TXDUR_L, M_CCA_TXDUR_H);
		WL_DFS(("wl%d.. %s txdur_start %uus\n", wlc->pub->unit, __FUNCTION__,
				dfs->txdur_start));
	}

#elif defined(RSDB_DFS_SCAN)

	/* Keep backup of home_chanspec */
	dfs->dfs_scan->home_chan = wlc->home_chanspec;

	other_wlc = wlc_dfs_other_wlc_if_dual(wlc);
	/* If other wlc is associated, we need to apply certain phy parameters for same band
	 * operation.
	 */
	if (other_wlc->pub->associated) {
		ASSERT(!MCHAN_ACTIVE(other_wlc->pub));
		phy_blank = wlc_dfs_is_phy_blanking_required(other_wlc->home_chanspec,
			scan_chspec);
	}
	dfs->dfs_scan->inprogress = FALSE;
	(void) wlc_mpc_off_req_set(wlc, MPC_OFF_REQ_DFS_SCAN_ACTIVE, MPC_OFF_REQ_DFS_SCAN_ACTIVE);

	/* ToDo: wlc_phy_set_rsdb_dfs_scan_in_progress((phy_info_t *)WLC_PI(wlc), ?)
	 *		CHSPEC_IS5G(home_chspec) && CHSPEC_IS5G(scan_chspec));
	 */
	/* If scanning close by channels set ucode radar blanking host flags */
	if (phy_blank) {
		wlc_mhf(wlc, MHF4, MHF4_RSDB_DFS_SCAN, MHF4_RSDB_DFS_SCAN, WLC_BAND_ALL);
		wlc_mhf(other_wlc, MHF4, MHF4_RSDB_DFS_SCAN, MHF4_RSDB_DFS_SCAN, WLC_BAND_ALL);
	}
	WL_DFS(("wl%d %s DFS scan chan=%x(%d)\n", wlc->pub->unit, __FUNCTION__,
		scan_chspec, CHSPEC_CHANNEL(scan_chspec)));

	dfs->dfs_scan->inprogress = TRUE;
	dfs->dfs_scan->scan_wlc = wlc;
	dfs->dfs_scan->status = DFS_SCAN_S_INPROGESS;

	wlc->home_chanspec = scan_chspec;
	/* Change to DFS Scan channel */
	wlc_suspend_mac_and_wait(wlc);
	wlc_set_chanspec(wlc, scan_chspec);
	wlc_enable_mac(wlc);

	/* Keep a backup of _ap status */
	dfs->dfs_scan->_was_ap = wlc->pub->_ap;
	/* Force AP=1 for dfs_scan. */
	wlc->pub->_ap = 1;
	/* Start CAC state machine */
	wlc_set_dfs_cacstate(dfs, ON);
#endif /* BGDFS, RSDB_DFS_SCAN, ... */
}

/* retrieves other wlc if dual band is in use; returns the same (passed) wlc otherwise */
static wlc_info_t *
wlc_dfs_other_wlc_if_dual(wlc_info_t *wlc)
{
#ifdef BGDFS
	if (BGDFS_ENAB(wlc->pub)) {
		return wlc; /* BGDFS uses single instance of WLC */
	}
#elif defined(RSDB_DFS_SCAN)
	if (RSDB_ENAB(wlc->pub)) {
		return wlc_rsdb_get_other_wlc(wlc);
	}
#endif /* BGDFS, RSDB_DFS_SCAN */
	return wlc;
}

#ifdef BGDFS
/* Given two chanspecs, returns channel separation between those in mHz units */
static int
wlc_dfs_get_chan_separation(wlc_info_t *wlc, chanspec_t chspec0, chanspec_t chspec1)
{
	int freq0, freq1, separation;

	BCM_REFERENCE(wlc);

	freq0 = wf_channel2mhz(CHSPEC_CHANNEL(chspec0), CHSPEC_IS5G(chspec0)
		? WF_CHAN_FACTOR_5_G : WF_CHAN_FACTOR_2_4_G);
	freq1 = wf_channel2mhz(CHSPEC_CHANNEL(chspec1), CHSPEC_IS5G(chspec1)
		? WF_CHAN_FACTOR_5_G : WF_CHAN_FACTOR_2_4_G);
	separation = ABS(freq0 - freq1);

	WL_DFS(("wl%d.. %s ch0:0x%x freq0: %dMHz ch1:0x%x freq1: %dMHz sep: %dMHz\n",
			wlc->pub->unit, __FUNCTION__,
			chspec0, freq0, chspec1, freq1, separation));

	return separation;
}

/* returns max safe tx threshold for requested type;
 * weather channel check in ETSI takes highest precedence
 * adjacent channel check comes next
 * when not adjacent, different thresholds will be returned depending on country
 *
 * When multiple conditions satisfy, the most stringent threshold is returned.
 */
static uint32
wlc_dfs_get_max_safe_tx_threshold(wlc_dfs_info_t *dfs, bool adjacent, bool weather)
{
	uint32 threshold = 0;

	threshold = adjacent ? dfs->max_safe_tx_adj :  dfs->max_safe_tx_nonadj;
	if (weather) {
		/* override with weather channel safe tx threshold only if more stringent
		 * than due to other factors computed earlier
		 */
		return MIN(threshold, dfs->max_safe_tx_weather);
	}

	return threshold;
}

/*
 * discard background scan results if traffic on main core exceeds certain thresholds
 *
 * Due to tx blanking scan core results will need to be discarded when main core traffic
 * exceeds certain thresholds.
 *
 * This function is used to discard 'radar free' result at end of scan core CAC if traffic
 * on main core (and hence txblanking) exceeds certain tested thresholds.
 *
 */
static bool
wlc_dfs_discard_due_to_txblanking(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;
	uint32 txdur_end, txdur_delta;
	bool adjacent = FALSE, weather = FALSE;
	uint32 threshold = 0;
	int separation;
	bool discard = FALSE;
	uint16 bw_flag = CHSPEC_BW(WLC_BAND_PI_RADIO_CHANSPEC), bw_mhz = CHAN_BW_20MHZ;

	// mark these variables if txblank_check_mode requires the respective checks
	bool check_adj = (dfs->txblank_check_mode & BGDFS_TXBLANK_CHECK_MODE_ADJ) != 0;
	bool check_nonadj = (dfs->txblank_check_mode & BGDFS_TXBLANK_CHECK_MODE_NONADJ) != 0;
	bool check_weather = (dfs->txblank_check_mode & BGDFS_TXBLANK_CHECK_MODE_EU_WEATHER) != 0;

	if (dfs->txblank_check_mode == 0) {
		return FALSE;
	}

	/* get tx duration now (end of CAC) */
	txdur_end = wlc_bmac_cca_read_counter(wlc->hw, M_CCA_TXDUR_L, M_CCA_TXDUR_H);
	/* get tx duration delta since start of CAC */
	txdur_delta = txdur_end - dfs->txdur_start; /* uint32 will auto-handle wrapping counters */

	/* compute traffic percentage to compare with tested thresholds */
	if (dfs->dfs_cac.cactime > 0 && txdur_delta > 0) {
		/*
		 * txdur_delta (in microsecs) won't be larger than max CAC = 600s = 0x23c34600us
		 * uint32 can fit ~4000s; intermediate and final values are smaller due to division
		 * Readable form:
		 * txdur_delta_percent = 100 * (txdur_delta_in_ms / cactime_in_ms)
		 * where
		 *   txdur_delta_in_ms = txdur_delta / 1000
		 *   cactime_in_ms = cactime * WLC_DFS_RADAR_CHECK_INTERVAL
		 */
		dfs->txdur_delta_percent = (txdur_delta / 10) /
				(dfs->dfs_cac.cactime * WLC_DFS_RADAR_CHECK_INTERVAL);
	} else {
		dfs->txdur_delta_percent = 0;
	}

	/* get channel seperation (in MHz) between main core and scan core channels */
	separation = wlc_dfs_get_chan_separation(wlc, WLC_BAND_PI_RADIO_CHANSPEC, dfs->sc_chspec);

	if (bw_flag == WL_CHANSPEC_BW_80) {
		bw_mhz = CHAN_BW_80MHZ;
	} else if (bw_flag == WL_CHANSPEC_BW_40) {
		bw_mhz = CHAN_BW_40MHZ;
	} else if (bw_flag == WL_CHANSPEC_BW_20) {
		bw_mhz = CHAN_BW_20MHZ;
	}
	/* scan channel is adjacent to main if center frequency seperation is within bandwidth */
	adjacent = separation <= bw_mhz;
	/* note if the scan core is operating on a weather radar channel (when in EU) */
	weather = wlc_is_european_weather_radar_channel(wlc, dfs->sc_chspec);

	/* skip threshold checks depending on txblank_check_mode */
	if (adjacent && !check_adj) {
		// skip only when weather check is also irrelevant
		if (!weather || !check_weather) {
			return FALSE;
		}
	}

	if (!adjacent && !check_nonadj) {
		// skip only when weather check is also irrelevant
		if (!weather || !check_weather) {
			return FALSE;
		}
	}

	threshold = wlc_dfs_get_max_safe_tx_threshold(dfs, adjacent, weather);

	/* compare with >= as integer will inherently round down */
	discard = (dfs->txdur_delta_percent >= threshold);

	WL_DFS(("wl%d.. %s txdur start:%uus, end:%uus, delta: %uus or %d%%, "
			"separation: %uMHz, adjacent: %d, weather: %d, th: %d%%, discard: %d\n",
			wlc->pub->unit, __FUNCTION__,
			dfs->txdur_start, txdur_end, txdur_delta, dfs->txdur_delta_percent,
			separation, adjacent, weather, threshold, discard));

	return discard;
}

/*
 * Called on completion of CAC on scan core.
 * If radar was found, initiates upgrade to full MIMO on current channel itself.
 * If radar wasn't found, initiates channel switch which will be followed by upgrade.
 */
static void
wlc_dfs_scan_complete_sc(wlc_dfs_info_t *dfs, int reason, int return_home_chan)
{
	wlc_info_t *wlc = dfs->wlc;
	int err;
	chanspec_t chspec = dfs->sc_chspec;
	bool discard_due_to_txblanking = FALSE;
#ifdef WL_AP_CHAN_CHANGE_EVENT
	wl_chan_change_reason_t rc;
	chanspec_t target_chspec = WLC_BAND_PI_RADIO_CHANSPEC;
	bool send_event = TRUE;
#endif /* WL_AP_CHAN_CHANGE_EVENT */
	dfs->dfs_scan->inprogress = FALSE;

	dfs->phymode = PHYMODE(wlc);
	if (dfs->phymode != PHYMODE_3x3_1x1) {
		return;
	}

	/* do txblanking related checks only when no radar was found and check-mode is non-zero */
	if (reason == DFS_SCAN_S_RADAR_FREE && dfs->txblank_check_mode != 0) {
		discard_due_to_txblanking = wlc_dfs_discard_due_to_txblanking(dfs);
		if (discard_due_to_txblanking) {
			reason = DFS_SCAN_S_SCAN_ABORTED;
		}
	}

	if (reason == DFS_SCAN_S_RADAR_FREE) {
		dfs->sc_last_cleared_chspec  = chspec;
	}

	WL_DFS(("wl%d.. %s chan=%x (%d) reason %d\n", wlc->pub->unit, __FUNCTION__,
			chspec, CHSPEC_CHANNEL(chspec), reason));

	/* cac completed. resume normal bss operation */
	if (dfs->scan_both) {
		/* restore phymode as main core channel changes can't be done in 3+1 */
		wlc_phy_set_val_phymode((phy_info_t *)WLC_PI(wlc), 0);

		wlc_dfs_cacstate_ism_set(dfs);
	}

	dfs->dfs_scan->status = reason;

	if (reason == DFS_SCAN_S_RADAR_FREE && dfs->move_stunted &&
			wlc->is_edcrs_eu) {
		dfs->dfs_cac.status.chanspec_cleared = dfs->dfs_scan->csa.chspec;
		/* clear the channel */
		wlc_clr_quiet_chanspec(wlc->cmi, dfs->dfs_cac.status.chanspec_cleared);
	}

	/* Channel is radar free. */
	if (reason == DFS_SCAN_S_RADAR_FREE && !dfs->move_stunted) {
		int idx;
		wlc_bsscfg_t *apcfg;
		wl_chan_switch_t *csa = &dfs->dfs_scan->csa;

		/* ensure CSA mode is advisory; since move is not initiated due to radar */
		csa->mode = DOT11_CSA_MODE_ADVISORY;
		csa->chspec = chspec;
		FOREACH_UP_AP(wlc, idx, apcfg) {
			if (BCME_OK != (err = wlc_csa_do_channel_switch(wlc->csa, apcfg,
					csa->chspec, csa->mode, csa->count, csa->reg,
					csa->frame_type))) {
				WL_ERROR(("wl%d %s Err in do chanswitch...%d\n", wlc->pub->unit,
						__FUNCTION__, err));
			}
			WL_DFS(("Started DFS CSA...\n"));
		}
		/* send event to user for completion */
#ifdef WL_AP_CHAN_CHANGE_EVENT
		target_chspec = csa->chspec;
#endif /* WL_CHAN_AP_CHANGE_EVENT */
	} else {
		wlc_dfs_scan_cleanup(dfs, TRUE, TRUE);
		dfs->dfs_scan->scan_wlc = NULL;
	}

#ifdef WL_AP_CHAN_CHANGE_EVENT
	/* send event for ABORT, RADAR or RADAR free */
	if (reason == DFS_SCAN_S_SCAN_ABORTED) {
		rc = WL_CHAN_REASON_DFS_AP_MOVE_ABORTED;
	} else if (reason == DFS_SCAN_S_RADAR_FOUND) {
		rc = WL_CHAN_REASON_DFS_AP_MOVE_RADAR_FOUND;
	} else if ((reason == DFS_SCAN_S_RADAR_FREE) && !(dfs->move_stunted)) {
		rc = WL_CHAN_REASON_DFS_AP_MOVE_SUCCESS;
	} else {
		send_event = FALSE;
	}

	if (send_event) {
		wlc_channel_send_chan_event(wlc, rc, target_chspec);
	}
#endif /* WL_AP_CHAN_CHANGE_EVENT */

}
#endif /* BGDFS */

/*
 * Called on completion of CAC on main core.
 */
static void
wlc_dfs_scan_complete(wlc_dfs_info_t *dfs, int reason, int return_home_chan)
{
	wlc_info_t *wlc = dfs->wlc;
	int err;
	chanspec_t chspec = WLC_BAND_PI_RADIO_CHANSPEC;

	WL_DFS(("wl%d.. %s chan=%x (%d) reason %d\n", wlc->pub->unit, __FUNCTION__,
			chspec, CHSPEC_CHANNEL(chspec), reason));

	dfs->dfs_scan->status = reason;

	/* Channel is radar free. */
	if (reason == DFS_SCAN_S_RADAR_FREE) {
		int idx;
		wlc_bsscfg_t *apcfg;
		wl_chan_switch_t *csa = &dfs->dfs_scan->csa;
#ifdef RSDB_DFS_SCAN
		wlc = wlc_dfs_other_wlc_if_dual(wlc);
#endif /* RSDB_DFS_SCAN */
		csa->chspec = chspec;
		FOREACH_UP_AP(wlc, idx, apcfg) {
			if (BCME_OK != (err = wlc_csa_do_channel_switch(wlc->csa, apcfg,
					csa->chspec, csa->mode, csa->count, csa->reg,
					csa->frame_type))) {
				WL_ERROR(("wl%d %s Err in do chanswitch...%d\n", wlc->pub->unit,
						__FUNCTION__, err));
			}
			WL_DFS(("Started DFS CSA...\n"));
		}
	} else {
		wlc_dfs_scan_cleanup(dfs, FALSE, TRUE);
		dfs->dfs_scan->scan_wlc = NULL;
	}
}

static void
wlc_dfs_scan_cleanup(wlc_dfs_info_t *dfs, bool return_home_chan, bool upgrade)
{
	wlc_info_t *wlc = dfs->wlc;

#ifdef RSDB_DFS_SCAN
	wlc_info_t *other_wlc = wlc_dfs_other_wlc_if_dual(wlc);
	wlc_phy_set_rsdb_dfs_scan_in_progress((phy_info_t *)WLC_PI(wlc), 0);
	/* unset the ucode radar blanking host flags unconditionally */
	wlc_mhf(wlc, MHF4, MHF4_RSDB_DFS_SCAN, 0, WLC_BAND_ALL);
	wlc_mhf(other_wlc, MHF4, MHF4_RSDB_DFS_SCAN, 0, WLC_BAND_ALL);
#endif /* RSDB_DFS_SCAN */

	if (!wlc_radar_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC)) {
		wlc_set_dfs_cacstate(dfs, OFF);
	}

	wlc->pub->_ap = dfs->dfs_scan->_was_ap;

	dfs->dfs_scan->inprogress = FALSE;
	dfs->scan_both = FALSE;

	if (return_home_chan &&
#ifdef BGDFS
			BGDFS_ENAB(wlc->pub) &&
			dfs->phymode != PHYMODE_3x3_1x1 &&
#endif /* BGDFS */
			TRUE) {
		/* Change the channel back to home channel */
		wlc_suspend_mac_and_wait(wlc);
		wlc_set_chanspec(wlc, dfs->dfs_scan->home_chan);
		wlc_enable_mac(wlc);
	}
#if !defined(BGDFS) && defined(RSDB_DFS_SCAN)
	(void) wlc_mpc_off_req_set(wlc, MPC_OFF_REQ_DFS_SCAN_ACTIVE, 0);
#endif /* !BGDFS && RSDB_DFS_SCAN */

	if (!upgrade)
		return;

#ifdef BGDFS
	if (BGDFS_ENAB(wlc->pub) && dfs->upgrade_pending &&
			dfs->dfs_scan->modeswitch_state != DFS_MODESW_UPGRADE_IN_PROGRESS) {
		dfs->dfs_scan->modeswitch_state = DFS_MODESW_UPGRADE_IN_PROGRESS;
#ifdef WL_MODESW
		if (WLC_MODESW_ENAB(wlc->pub)) {
			(void) wlc_dfs_bg_upgrade_wlc(wlc);
		}
#else
		wlc_dfs_bg_upgrade_phy(dfs);
#endif /* WL_MODESW */
	}
#elif defined(RSDB_DFS_SCAN)
	if (dfs->dfs_scan->modeswitch_state == DFS_MODESW_DOWNGRADE_IN_FINISHED) {
		dfs->dfs_scan->modeswitch_state = DFS_MODESW_UPGRADE_IN_PROGRESS;
		wlc_rsdb_upgrade_wlc(wlc_dfs_other_wlc_if_dual(wlc));
	}
#endif /* BGDFS, RSDB_DFS_SCAN */

}
#endif /* RSDB_DFS_SCAN || BGDFS */

static void
wlc_dfs_watchdog(void *ctx)
{
	wlc_dfs_info_t *dfs = (wlc_dfs_info_t *)ctx;
	wlc_info_t *wlc = dfs->wlc;

	(void)wlc;

	/* Restore channels 30 minutes after radar detect */
	if (WL11H_ENAB(wlc) && dfs->radar) {
		int chan;

		for (chan = 0; chan < MAXCHANNEL; chan++) {
			if (dfs->chan_blocked[chan] &&
			    dfs->chan_blocked[chan] != WLC_CHANBLOCK_FOREVER) {
				dfs->chan_blocked[chan]--;
				if (!dfs->chan_blocked[chan]) {
					WL_REGULATORY(("\t** DFS *** Channel %d is"
					               " clean after 30 minutes\n", chan));
				}
			}
		}
	}
}

/* Nothing to be done for now, Usually none of the bss are up
 * by now.
 */
static int
wlc_dfs_up(void *ctx)
{
	return BCME_OK;
}

static int
wlc_dfs_down(void *ctx)
{
	wlc_dfs_info_t *dfs = (wlc_dfs_info_t *)ctx;
	int callback = 0;

	/* cancel the radar timer */
	if (dfs->dfs_cac.timer_running == TRUE) {
		wlc_dfs_cacstate_idle_set(dfs);
		dfs->dfs_cac.status.chanspec_cleared = 0;
		if (!wlc_dfs_timer_delete(dfs))
			callback = 1;
		dfs->dfs_cac_enabled = FALSE;
	}

	return callback;
}

static int
wlc_dfs_get_dfs_status_all(wlc_dfs_info_t *dfs, uint8 *arg, int len)
{
	int all_min_sz = sizeof(wl_dfs_status_all_t);
	int sub_sz     = sizeof(wl_dfs_sub_status_t);
	int max_num_sub = len < all_min_sz ? 0 : (1 + (len - all_min_sz)/sub_sz);
	wl_dfs_status_all_t * all = (wl_dfs_status_all_t*) arg;
	wlc_info_t *wlc = dfs->wlc;
	wl_dfs_sub_status_t *sub0;

	dfs->dfs_cac.status.duration =
		(dfs->dfs_cac.cactime - dfs->dfs_cac.duration) *
		WLC_DFS_RADAR_CHECK_INTERVAL;

	WL_DFS(("%s: len=%d, all_min_sz=%d, sub_sz=%d, max_num_sub=%d\n",
			__FUNCTION__, len, all_min_sz, sub_sz, max_num_sub));

	if (len < all_min_sz || max_num_sub < 1) {
		return BCME_BUFTOOSHORT;
	}
	all->version = WL_DFS_STATUS_ALL_VERSION;
	all->num_sub_status = 1;
	sub0 = &(all->dfs_sub_status[0]);
	memset(sub0, 0, sizeof(*sub0));
	sub0->state = dfs->dfs_cac.status.state;
	sub0->duration = dfs->dfs_cac.status.duration;
	sub0->chanspec = SCAN_IN_PROGRESS(wlc->scan) ? wlc->home_chanspec :
		WLC_BAND_PI_RADIO_CHANSPEC;
	sub0->chanspec_last_cleared = dfs->dfs_cac.status.chanspec_cleared;
	sub0->sub_type = 0;
#ifdef BGDFS
	if (BGDFS_ENAB(wlc->pub) && all->num_sub_status < max_num_sub) {
		wl_dfs_sub_status_t *sub1 = sub0 + 1;
		memset(sub1, 0, sizeof(*sub1));
		all->num_sub_status++;
		if (wlc->dfs->phymode == PHYMODE_3x3_1x1) {
			sub1->state = dfs->dfs_cac.status.state;
			if (dfs->scan_both) {
				sub0->state = WL_DFS_CACSTATE_ISM;
			} else {
				sub0->state = WL_DFS_CACSTATE_IDLE;
			}
			sub1->duration = dfs->dfs_cac.status.duration;
			sub1->chanspec = dfs->sc_chspec;
		}
		sub1->chanspec_last_cleared = dfs->sc_last_cleared_chspec;
		sub1->sub_type = 1;
	}
#endif /* BGDFS */
	return BCME_OK;
}

#ifdef BCMDBG
static int
wlc_dfs_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_dfs_info_t *dfs = (wlc_dfs_info_t *)ctx;
	uint32 i;

	bcm_bprintf(b, "radar %d\n", dfs->radar);
	bcm_bprhex(b, "chan_blocked ", TRUE,
	           (uint8 *)dfs->chan_blocked, sizeof(dfs->chan_blocked));
	bcm_bprintf(b, "cactime_pre_ism %u cactime_post_ism %u nop_sec %u ism_monitor %d\n",
	            dfs->dfs_cac.cactime_pre_ism, dfs->dfs_cac.cactime_post_ism,
	            dfs->dfs_cac.nop_sec, dfs->dfs_cac.ism_monitor);
	if (dfs->dfs_cac.chanspec_forced_list) {
		for (i = 0; i < dfs->dfs_cac.chanspec_forced_list->num; i++)
			bcm_bprintf(b, "chanspec_forced %x status %d cactime %u\n",
			            dfs->dfs_cac.chanspec_forced_list->list[i], dfs->dfs_cac.status,
			            dfs->dfs_cac.cactime, dfs->dfs_cac);
	}
	bcm_bprintf(b, "duration %u chanspec_next %x timer_running %d\n",
	            dfs->dfs_cac.duration, dfs->dfs_cac.chanspec_next,
	            dfs->dfs_cac.timer_running);
	bcm_bprintf(b, "dfs_cac_enabled %d\n", dfs->dfs_cac_enabled);

	return BCME_OK;
}
#endif /* BCMDBG */

uint32
wlc_dfs_get_chan_info(wlc_dfs_info_t *dfs, uint channel)
{
	uint32 result;

	result = 0;
	if (dfs->chan_blocked[channel]) {
		int minutes;

		result |= WL_CHAN_INACTIVE;

		/* Store remaining minutes until channel comes
		 * in-service in high 8 bits.
		 */
		minutes = ROUNDUP(dfs->chan_blocked[channel], 60) / 60;
		result |= ((minutes & 0xff) << 24);
	}

	return (result);
}

#if defined(EXT_STA) || defined(CLIENT_CSA)
static wlc_bsscfg_t*
wlc_get_ap_bsscfg(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;
	wlc_bsscfg_t *bsscfg = NULL;
	int i;

	if (AP_ACTIVE(wlc)) {
		for (i = 0; i < 2; i++) {
			if (wlc->bsscfg[i] && BSSCFG_AP(wlc->bsscfg[i]) && wlc->bsscfg[i]->up) {
				bsscfg = wlc->bsscfg[i];
				/* one ap supported in Win7 */
				break;
			}
		}
		ASSERT(bsscfg);
	}
	return bsscfg;
}
#endif /* EXT_STA */

/*
 * Helper function to use correct pre- and post-ISM CAC time for european weather radar channels
 * which use a different CAC timer (default is 10 minutes for EU weather radar channels, 1 minute
 * for regular radar CAC).
 *
 * Returns cactime in WLC_DFS_RADAR_CHECK_INTERVAL units.
 */
static uint
wlc_dfs_ism_cactime(wlc_info_t *wlc, int secs_or_default)
{
	chanspec_t chspec = WLC_BAND_PI_RADIO_CHANSPEC;

#ifdef BGDFS
	/* override chanspec with scan core's if scan core is active (eg. in 3x1 mode) */
	if (BGDFS_ENAB(wlc->pub) && wlc->dfs->phymode == PHYMODE_3x3_1x1) {
		chspec = wlc->dfs->sc_chspec;
		WL_DFS(("wl%d: since phymode is 3x1 using scan core with chanspec %04x\n",
				wlc->pub->unit, chspec));
	}
#endif /* BGDFS */

	if (!wlc_quiet_chanspec(wlc->cmi, chspec)) {
		WL_DFS(("wl%d: Skip CAC - channel 0x%x is already available. Zero duration.\n",
				wlc->pub->unit, chspec));
		return 0;  /* zero CAC duration; no need of CAC */
	}

	if (secs_or_default == WLC_DFS_CAC_TIME_USE_DEFAULTS)
	{
		if (wlc_is_european_weather_radar_channel(wlc, chspec)) {

			secs_or_default = WLC_DFS_CAC_TIME_SEC_DEF_EUWR;

			WL_DFS(("wl%d: dfs chanspec %04x is european weather radar\n",
				wlc->pub->unit, chspec));
		}
		else {
			secs_or_default = WLC_DFS_CAC_TIME_SEC_DEFAULT;
		}
	}

	WL_DFS(("wl%d: chanspec %04x, %d second CAC time\n",
		wlc->pub->unit, chspec, secs_or_default));

	return (secs_or_default*1000)/WLC_DFS_RADAR_CHECK_INTERVAL;

}

/*
 * Return CAC duration in ms. ASSOC would want to know it.
 */
uint
wlc_dfs_get_cactime_ms(wlc_dfs_info_t *dfs)
{
	ASSERT(dfs);
	return (dfs->dfs_cac.cactime * WLC_DFS_RADAR_CHECK_INTERVAL);
}

static int
BCMATTACHFN(wlc_dfs_timer_init)(wlc_dfs_info_t *dfs)
{
	wlc_info_t* wlc = dfs->wlc;

	dfs->dfs_cac.ism_monitor = FALSE; /* put it to normal mode */

	dfs->dfs_cac.timer_running = FALSE;

#ifdef WL_DFS_WAVE_MODE
	dfs->dfs_cac.test_mode = FALSE;
#endif /* WL_DFS_WAVE_MODE */

	if (!(dfs->dfs_timer = wl_init_timer(wlc->wl, wlc_dfs_cacstate_handler, dfs, "dfs"))) {
		WL_ERROR(("wl%d: wlc_dfs_timer_init failed\n", wlc->pub->unit));
		return -1;
	}
	dfs->dfs_cac.cactime_pre_ism = dfs->dfs_cac.cactime_post_ism
		= WLC_DFS_CAC_TIME_USE_DEFAULTS;   /* use default values */

	dfs->dfs_cac.nop_sec = WLC_DFS_NOP_SEC_DEFAULT;

	return 0;
}

#ifdef SLAVE_RADAR
static void
wlc_radar_report_handler(void *arg)
{
	chanspec_t chanspec;
	wlc_dfs_info_t *dfs = (wlc_dfs_info_t *)arg;
	wlc_info_t *wlc = dfs->wlc;
	uint16 chanspec_bw = CHSPEC_BW(WLC_BAND_PI_RADIO_CHANSPEC);

	wlc_disassociate_client(wlc->cfg, FALSE);
	if (chanspec_bw == WL_CHANSPEC_BW_80) {
	       chanspec = CH80MHZ_CHSPEC(42, WL_CHANSPEC_CTL_SB_LL);
	}
	else if (chanspec_bw == WL_CHANSPEC_BW_40) {
	       chanspec = CH40MHZ_CHSPEC(38, WL_CHANSPEC_CTL_SB_LOWER);
	}
	else { /* chanspec is 20 MHz */
	       chanspec = CH20MHZ_CHSPEC(36);
	}
	if (wlc_valid_chanspec_db(wlc->cmi, chanspec)) {
		wlc_set_home_chanspec(wlc, chanspec);
		wlc_suspend_mac_and_wait(wlc);
		wlc_set_chanspec(wlc, chanspec);
		wlc_enable_mac(wlc);
	}

	wlc_roamscan_start(wlc->cfg, WLC_E_REASON_RADAR_DETECTED);
	wlc_dfs_cac_state_change(dfs, WL_DFS_CACSTATE_IDLE);
}

static int
BCMATTACHFN(wlc_radar_report_timer_init)(wlc_dfs_info_t *dfs)
{
	wlc_info_t* wlc = dfs->wlc;

	dfs->radar_report_timer_running = FALSE;

	if (!(dfs->radar_report_timer = wl_init_timer(wlc->wl,
		wlc_radar_report_handler, dfs, "radar_report_timer"))) {
		WL_ERROR(("wl%d: wlc_radar_report_timer_init failed\n", wlc->pub->unit));
		return BCME_ERROR;
	}
	return 0;
}
#endif /* SLAVE_RADAR */

static void
wlc_dfs_timer_add(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;

	if (dfs->dfs_cac.timer_running == FALSE) {
		dfs->dfs_cac.timer_running = TRUE;
		wl_add_timer(wlc->wl, dfs->dfs_timer, WLC_DFS_RADAR_CHECK_INTERVAL, TRUE);
	}
}

static bool
wlc_dfs_timer_delete(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;
	bool canceled = TRUE;
	if (dfs->dfs_cac.timer_running == TRUE) {
		if (dfs->dfs_timer != NULL) {
			canceled = wl_del_timer(wlc->wl, dfs->dfs_timer);
			ASSERT(canceled == TRUE);
		}
		dfs->dfs_cac.timer_running = FALSE;
	}
	return canceled;
}

static void
wlc_dfs_chanspec_oos(wlc_dfs_info_t *dfs, chanspec_t chanspec)
{
	wlc_info_t* wlc = dfs->wlc;
	bool is_us, is_ca, tdwr = FALSE, has_subband_info = DFS_HAS_SUBBAND_INFO(wlc);
	bool radar_on_tdwr_left = FALSE, radar_on_tdwr_right = FALSE;
	uint8 channel, radar_sb = dfs->radar_subbands, pos_20 = LOWER_20_POS_20MHZ;

	is_us = (!bcmp("US", wlc_channel_country_abbrev(wlc->cmi), 2));
	is_ca = (!bcmp("CA", wlc_channel_country_abbrev(wlc->cmi), 2));

	if (CHSPEC_IS160(chanspec) || CHSPEC_IS8080(chanspec)) {
		pos_20 = LOWER_20_POS_160MHZ;
	} else if (CHSPEC_IS80(chanspec)) {
		pos_20 = LOWER_20_POS_80MHZ;
	} else if (CHSPEC_IS40(chanspec)) {
		pos_20 = LOWER_20_POS_40MHZ;
	}

	/* radar_subbands has the bit map of all the subbands of a chanspec
	 * this is an 8 bit value for 160/80p80 MHz and a 4 bit value for 80 MHz, ...
	 */
	FOREACH_20_SB(chanspec, channel) {
		if (!wlc_radar_chanspec(wlc->cmi, CH20MHZ_CHSPEC(channel))) {
			pos_20 >>= 1;
			continue;
		}
		/* if subband info is known, only radar detected subset are marked OOS & passive */
		if (!has_subband_info || (has_subband_info && (radar_sb & pos_20))) {
			dfs->chan_blocked[channel] = dfs->dfs_cac.nop_sec;
			dfs->chan_cac_pending[channel] = TRUE;
			wlc_set_quiet_chanspec(wlc->cmi, CH20MHZ_CHSPEC(channel));
			WL_DFS(("wl%d: channel %d put out of service chspec%x\n",
					wlc->pub->unit, channel, CH20MHZ_CHSPEC(channel)));
			/* See special TDWR requirements below this loop */
			if (!tdwr && (is_us || is_ca) &&
					(channel >= TDWR_CH20_MIN && channel <= TDWR_CH20_MAX)) {
				tdwr = TRUE;
			}
			if (channel == TDWR_CH20_MIN) {
				radar_on_tdwr_left = TRUE;
			} else if (channel == TDWR_CH20_MAX) {
				radar_on_tdwr_right = TRUE;
			}
		} else if (!wlc->is_edcrs_eu) {
			// in non-EU, on moving out of radar ch, mark it passive
			wlc_set_quiet_chanspec(wlc->cmi, CH20MHZ_CHSPEC(channel));
		}
		pos_20 >>= 1;
	}

	/* Special cases for Terminal Doppler Weather Radar (TDWR):
	 *  US: Block entire 120-128 for NOP/30+m on radar detection in any TDWR subband. SVN-r38281
	 *  CA: Block entire 120-128 INDEFINITELY on radar detection in any TDWR subband. SVN-r89728
	 *  US/CA: Block 116 if radar is detected on 120. Block 132 if radar is detected on 128.
	 *  (TDWR channels aren't available in CA yet; these special cases for CA are anticipatory)
	 */
	if (tdwr) {
		uint32 tdwr_block_time = is_ca ? WLC_CHANBLOCK_FOREVER : dfs->dfs_cac.nop_sec;

		/* blocking all TDWR chanspecs */
		for (channel = TDWR_CH20_MIN; channel <= TDWR_CH20_MAX; channel += CH_20MHZ_APART) {
			wlc_set_quiet_chanspec(wlc->cmi, CH20MHZ_CHSPEC(channel));
			dfs->chan_blocked[channel] = tdwr_block_time;
			dfs->chan_cac_pending[channel] = TRUE;
		}

		/* blocking non-TDWR adjacent channel since radar detected on left edge of TDWR */
		if (radar_on_tdwr_left) {
			channel = TDWR_CH20_MIN - CH_20MHZ_APART;
			wlc_set_quiet_chanspec(wlc->cmi, CH20MHZ_CHSPEC(channel));
			dfs->chan_blocked[channel] = tdwr_block_time;
			dfs->chan_cac_pending[channel] = TRUE;
		}

		/* blocking non-TDWR adjacent channel since radar detected on right edge of TDWR */
		if (radar_on_tdwr_right) {
			channel = TDWR_CH20_MAX + CH_20MHZ_APART;
			wlc_set_quiet_chanspec(wlc->cmi, CH20MHZ_CHSPEC(channel));
			dfs->chan_blocked[channel] = tdwr_block_time;
			dfs->chan_cac_pending[channel] = TRUE;
		}
	}
}

/*
 * Returns first valid (non current) channel from dfs forced channel list,
 * 0 if none found
 */
static chanspec_t
wlc_dfs_valid_forced_chanspec(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;
	chanspec_t chspec, first_valid = 0;
	uint i;
	chanspec_t cur_ch = WLC_BAND_PI_RADIO_CHANSPEC;
	chanspec_list_t *forced = dfs->dfs_cac.chanspec_forced_list;

	if (forced == NULL || forced->num < 1) {
		return 0;
	}

	for (i = 0; i < forced->num && i < WL_NUMCHANSPECS; i++) {
		chspec = forced->list[i];
		if ((((~WL_CHANSPEC_CTL_SB_MASK) & cur_ch) ==
				((~WL_CHANSPEC_CTL_SB_MASK) & chspec)) ||
				wf_chspec_overlap(chspec, cur_ch)) {
			continue; /* skip since center matches current or the two overlap */
		}
		if (wlc_valid_dfs_chanspec(wlc, chspec)) {
			if (first_valid == 0) {
				first_valid = chspec;
			}
			/* CAC isn't required if not marked quiet. eg. non-DFS / (pre)cleared DFS */
			if (!wlc_quiet_chanspec(wlc->cmi, chspec)) {
				return chspec;
			}
		}
	}

	return first_valid;
}

/* Rearranges channel list, bringing valid usable (non current) channels to head of the list */
static void
wlc_dfs_rearrange_channel_list(wlc_dfs_info_t *dfs, chanspec_list_t *ch_list)
{
	wlc_info_t *wlc = dfs->wlc;
	chanspec_t chspec;
	uint i, valid = 0;
	chanspec_t cur_ch = WLC_BAND_PI_RADIO_CHANSPEC;

	if (ch_list == NULL || ch_list->num < 2) {
		return;
	}

	for (i = 0; i < ch_list->num; i++) {
		chspec = ch_list->list[i];
		if (((~WL_CHANSPEC_CTL_SB_MASK) & cur_ch) ==
				((~WL_CHANSPEC_CTL_SB_MASK) & chspec) ||	/* same center */
				wf_bw_chspec_to_half_mhz(chspec) >
				wf_bw_chspec_to_half_mhz(cur_ch) ||		/* larger bw */
				!wlc_valid_dfs_chanspec(wlc, chspec) ||		/* invalid */
				wlc_quiet_chanspec(wlc->cmi, chspec) ||		/* quiet */
				(wlc_radar_chanspec(wlc->cmi, chspec) &&	/* (radar && */
				wf_chspec_overlap(chspec, cur_ch))) {		/* overlap) */
			continue; /* don't bring to head of list */
		}
		/* bring those, that fail above tests, to head of list */
		ch_list->list[i] = ch_list->list[valid];
		ch_list->list[valid++] = chspec;
	}
}

/* Returns a valid channel or 0 on error */
static chanspec_t
wlc_dfs_radar_channel_bw_fallback(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;
	chanspec_t chspec, cur_ch = WLC_BAND_PI_RADIO_CHANSPEC;
	uint16 bw_idx, bw = CHSPEC_BW(cur_ch);
	/* list of bandwidth in decreasing order of magnitude */
	uint16 bw_list[] = { WL_CHANSPEC_BW_160, WL_CHANSPEC_BW_80,
		WL_CHANSPEC_BW_40, WL_CHANSPEC_BW_20 };
	/* length of the list of bandwidth */
	uint16 bw_list_len = sizeof(bw_list) / sizeof(bw_list[0]);

	/* get to the first acceptable bw */
	for (bw_idx = 0; bw_idx < bw_list_len && bw_list[bw_idx] != bw; bw_idx++) /* NO OP */;

	if (bw_idx == bw_list_len) {
		WL_DFS(("crnt_chspec: 0x%04x, bw_list[%d]: 0x%x, crnt_bw: 0x%x\n",
				cur_ch, bw_idx, (bw_idx < bw_list_len ? bw_list[bw_idx] : -1),
				bw));
		return 0;
	}

	while (bw_idx < bw_list_len) {
		bw = bw_list[bw_idx++];
		if (!wlc_is_valid_bw(wlc, wlc->cfg, BAND_5G_INDEX, bw)) {
			continue; /* skip unsupported bandwidth */
		}

		if ((bw == WL_CHANSPEC_BW_8080) || (bw == WL_CHANSPEC_BW_160)) {
			chspec = wf_chspec_primary80_chspec(cur_ch);
		} else if (bw == WL_CHANSPEC_BW_80) {
			chspec = wf_chspec_primary40_chspec(cur_ch);
		} else if (bw == WL_CHANSPEC_BW_40) {
			chspec = CH20MHZ_CHSPEC(wf_chspec_ctlchan(cur_ch));
		} else
			break;

		if (wlc_valid_dfs_chanspec(wlc, chspec)) {
			WL_DFS(("wl%d: %s : select chanspec 0x%04x bw: 0x%x\n", wlc->pub->unit,
				__FUNCTION__, chspec, CHSPEC_BW(chspec)));

			wlc_clr_quiet_chanspec(wlc->cmi, chspec);

			return chspec;
		}
	}

	return 0;
}

/*
 * Returns a forced channel if valid or does a random channel selection for DFS
 * Returns a valid chanspec of a valid radar free channel, using the AP configuration
 * to choose 20, 40 or 80 MHz bandwidth and side-band
 * When 'radar_detected' is TRUE, avoid DFS channels unless pre-cleared in ETSI
 * Returns 0 if there are no valid radar free channels available
 */
static chanspec_t
wlc_dfs_chanspec(wlc_dfs_info_t *dfs, bool radar_detected)
{
	chanspec_t chspec;

	chspec = wlc_dfs_valid_forced_chanspec(dfs);

	/* return fallback bw channel */
	if (dfs->bw_fallback && (chspec == 0) && radar_detected)
		chspec = wlc_dfs_radar_channel_bw_fallback(dfs);

	/* return if suitable channel is in forced list */
	if (chspec == 0) {
		WL_DFS(("no usable channels found in dfs_channel_forced list; going random now\n"));
		/* walk the channels looking for good channels */
		/* When Radar is detected, look for Non-DFS or cleared (EDCRS_EU) channels */
		chspec = wlc_channel_5gchanspec_rand(dfs->wlc, radar_detected);
	}

	if (radar_detected) {
		wlc_dfs_send_event(dfs, chspec);
	}

	return chspec;
}

/* check for a chanspec on which an AP can set up a BSS
 * Returns TRUE if the chanspec is valid for the local, not restricted, and
 * has not been blocked by a recent radar pulse detection.
 * Otherwise will return FALSE.
 */
bool
wlc_valid_dfs_chanspec(wlc_info_t *wlc, chanspec_t chspec)
{
	uint channel;
	wlc_dfs_info_t *dfs = wlc->dfs;

	if (!wlc_valid_chanspec_db(wlc->cmi, chspec) ||
			wlc_restricted_chanspec(wlc->cmi, chspec)) {
		return FALSE;
	}

	if (dfs == NULL) {
		return TRUE;
	}

	FOREACH_20_SB(chspec, channel) {
		if (dfs->chan_blocked[channel]) {
			return FALSE;
		}
	}

	return TRUE;
}

#if defined(SLAVE_RADAR) || defined(CLIENT_CSA)
/*
 * Since a STA does not decide it's own chanspec, this function
 * will help check whether CAC has been done for a given chanspec
 * after Non Occupancy period and whether this chanspec has been
 * cleared for ISM.
 */
bool
wlc_cac_is_clr_chanspec(wlc_dfs_info_t *dfs, chanspec_t chspec)
{
	uint channel;
	wlc_info_t *wlc = dfs->wlc;

	if (!wlc_valid_chanspec_db(wlc->cmi, chspec) ||
	    wlc_restricted_chanspec(wlc->cmi, chspec))
		return FALSE;

	FOREACH_20_SB(chspec, channel) {
		if (dfs->chan_cac_pending[channel])
			return FALSE;
	}
	return TRUE;
}

/*
 * Once a CAC was done on given chanspec and no radar found, clear
 * it for ISM availability.
 */
static void
wlc_cac_do_clr_chanspec(wlc_dfs_info_t *dfs, chanspec_t chspec)
{
	uint channel;
	wlc_info_t *wlc = dfs->wlc;

	if (!wlc_valid_chanspec_db(wlc->cmi, chspec) ||
	    wlc_restricted_chanspec(wlc->cmi, chspec))
		return;

	FOREACH_20_SB(chspec, channel) {
		dfs->chan_cac_pending[channel] = FALSE;
	}
}
#endif /* SLAVE_RADAR || CLIENT_CSA */

static bool
wlc_radar_detected(wlc_dfs_info_t *dfs, bool scan_core)
{
	wlc_info_t *wlc = dfs->wlc;
	int radar_info = 0;
	int radar_type = 0;
	int min_pw = 0;
	int subband_result = 0;

	int radar_info_2 = 0;
	int radar_type_2 = 0;
	int min_pw_2 = 0;
	int subband_result_2 = 0;

	bool has_subband_info = DFS_HAS_SUBBAND_INFO(wlc);
#if defined(BCMDBG) || defined(WLMSG_DFS) || defined(BCMDBG_ERR) || \
	defined(WLTEST_DFSMSG)
	uint i;
	char radar_type_str[24];
	char radar_type_str_2[24];
	char chanbuf[CHANSPEC_STR_LEN];
	static const struct {
		int radar_type;
		const char *radar_type_name;
	} radar_names[] = {
		{RADAR_TYPE_NONE, "NONE"},
		{RADAR_TYPE_ETSI_1, "ETSI_1"},
		{RADAR_TYPE_ETSI_2, "ETSI_2"},
		{RADAR_TYPE_ETSI_3, "ETSI_3"},
		{RADAR_TYPE_ETSI_4, "ETSI_4"},
		{RADAR_TYPE_STG2, "S2"},
		{RADAR_TYPE_STG3, "S3"},
		{RADAR_TYPE_UNCLASSIFIED, "UNCLASSIFIED"},
		{RADAR_TYPE_FCC_5, "FCC-5"},
		{RADAR_TYPE_JP1_2_JP2_3, "JP1-2/JP2-3"},
		{RADAR_TYPE_JP2_1, "JP2-1"},
		{RADAR_TYPE_JP4, "JP4"},
		{RADAR_TYPE_UK1, "UK1"},
		{RADAR_TYPE_UK2, "UK2"}
	};
#endif /* BCMDBG || WLMSG_DFS || BCMDBG_ERR || WLTEST_DFSMSG */
	wlc_bsscfg_t *cfg = wlc->cfg;
	int PLL_idx = scan_core ? 1 : 0;
	chanspec_t chspec = WLC_BAND_PI_RADIO_CHANSPEC;
	int radar_sim_mask = scan_core ? RADAR_SIM_SC : RADAR_SIM;

#ifdef BGDFS
	if (!BGDFS_ENAB(wlc->pub) && scan_core) {
		return FALSE;
	}
	dfs->phymode = PHYMODE(wlc);
	if (scan_core) {
		chspec = dfs->sc_chspec;
		if (dfs->phymode != PHYMODE_3x3_1x1) {
			return FALSE;
		}
	}
#else
	if (scan_core) {
		return FALSE;
	}
#endif /* BGDFS */

	if (!wlc_radar_chanspec(wlc->cmi, chspec)) {
		return FALSE;
	}

	if ((CHSPEC_IS8080(wlc->chanspec) || CHSPEC_IS160(wlc->chanspec)) && scan_core == FALSE) {
		radar_info = phy_radar_detect_run((phy_info_t *)WLC_PI(wlc), 0, 1);
		if (radar_info != RADAR_TYPE_NONE &&
				!wlc_radar_chanspec(wlc->cmi, CHBW_CHSPEC(WL_CHANSPEC_BW_80,
				LOWER_80_SB(wlc->chanspec)))) {
			WL_ERROR(("wl%d: DFS: PHY detected radar (0x%05x) on NON-DFS of ch0x%04x\n",
					WLCWLUNIT(wlc), radar_info, wlc->chanspec));
			radar_info = RADAR_TYPE_NONE;
		}
		radar_info_2 = phy_radar_detect_run((phy_info_t *)WLC_PI(wlc), 1, 1);
		if (radar_info_2 != RADAR_TYPE_NONE &&
				!wlc_radar_chanspec(wlc->cmi, CHBW_CHSPEC(WL_CHANSPEC_BW_80,
				UPPER_80_SB(wlc->chanspec)))) {
			WL_ERROR(("wl%d: DFS: PHY detected radar (0x%05x) on NON-DFS of ch0x%04x\n",
					WLCWLUNIT(wlc), radar_info_2, wlc->chanspec));
			radar_info_2 = RADAR_TYPE_NONE;
		}
	} else {
		radar_info = phy_radar_detect_run((phy_info_t *)WLC_PI(wlc), PLL_idx, 0);
		radar_info_2 = 0;
	}

	if (!(wlc_11h_get_spect_state(wlc->m11h, cfg) & radar_sim_mask) &&
			radar_info == RADAR_TYPE_NONE && radar_info_2 == RADAR_TYPE_NONE) {
		return FALSE;
	}

	WL_DFS(("wl%d: DFS: radar_info=0x%05x, radar_info_2=0x%05x\n",
			WLCWLUNIT(wlc), radar_info, radar_info_2));

	if (has_subband_info) {
		subband_result = radar_info >> 14 & 0xf;
		subband_result_2 = radar_info_2 >> 14 & 0xf;
	}

	/* Real Radar subband detection. Simulation done by wlc_dfs_set_radar */
	if ((CHSPEC_IS8080(wlc->chanspec) || CHSPEC_IS160(wlc->chanspec)) && scan_core == FALSE) {
		if (radar_info != RADAR_TYPE_NONE || radar_info_2 != RADAR_TYPE_NONE) {
			dfs->radar_subbands = (uint8)((subband_result << 4) | subband_result_2);
			dfs->radar_info[0] = radar_info;
			dfs->radar_info[1] = radar_info_2;
		}
	} else { // bw <= 80MHz
		if (radar_info != RADAR_TYPE_NONE) {
			dfs->radar_subbands = (uint8) subband_result;
			dfs->radar_info[0] = radar_info;
			dfs->radar_info[1] = 0;
		}
	}

	min_pw = radar_info >> 4 & 0x1ff;
	BCM_REFERENCE(min_pw);
	radar_type = radar_info & 0xf;
	BCM_REFERENCE(radar_type);

	min_pw_2 = radar_info_2 >> 4 & 0x1ff;
	BCM_REFERENCE(min_pw_2);
	radar_type_2 = radar_info_2 & 0xf;
	BCM_REFERENCE(radar_type_2);

#if defined(BCMDBG) || defined(WLMSG_DFS) || defined(BCMDBG_ERR) || \
	defined(WLTEST_DFSMSG)
	snprintf(radar_type_str, sizeof(radar_type_str),
			"%s", "UNKNOWN");
	for (i = 0; i < ARRAYSIZE(radar_names); i++) {
		if (radar_names[i].radar_type == radar_type)
			snprintf(radar_type_str, sizeof(radar_type_str),
					"%s", radar_names[i].radar_type_name);
	}
	if (radar_type != 0) {
		if (has_subband_info) {
			WL_PRINT(("WL%d: DFS: %s ########## RADAR%s DETECTED ON CHANNEL %s"
					" ########## min_pw=%d, subband_result=%d"
					", AT %dMS\n", wlc->pub->unit,
					radar_type_str,
					(scan_core ? "_SC" : ""),
					wf_chspec_ntoa(chspec, chanbuf),
					min_pw, subband_result,
					(dfs->dfs_cac.cactime - dfs->dfs_cac.duration) *
					WLC_DFS_RADAR_CHECK_INTERVAL));
		} else {
			WL_PRINT(("WL%d: DFS: %s ########## RADAR%s DETECTED ON CHANNEL %s"
					" ########## min_pw=%d, AT %dMS\n", wlc->pub->unit,
					radar_type_str,
					(scan_core ? "_SC" : ""),
					wf_chspec_ntoa(chspec, chanbuf),
					min_pw,
					(dfs->dfs_cac.cactime - dfs->dfs_cac.duration) *
					WLC_DFS_RADAR_CHECK_INTERVAL));
		}
	}

	snprintf(radar_type_str_2, sizeof(radar_type_str_2),
			"%s", "UNKNOWN");
	for (i = 0; i < ARRAYSIZE(radar_names); i++) {
		if (radar_names[i].radar_type == radar_type_2)
			snprintf(radar_type_str_2, sizeof(radar_type_str_2),
					"%s", radar_names[i].radar_type_name);
	}
	if (radar_type_2 != 0) {
		if (has_subband_info) {
			WL_PRINT(("WL%d: DFS: %s ########## RADAR DETECTED ON U80 CHANNEL %s"
				" ########## min_pw=%d, subband_result=%d"
				", AT %dMS\n", wlc->pub->unit,
				radar_type_str_2,
				wf_chspec_ntoa(chspec, chanbuf),
				min_pw_2, subband_result_2,
				(dfs->dfs_cac.cactime - dfs->dfs_cac.duration) *
				WLC_DFS_RADAR_CHECK_INTERVAL));
		} else {
			WL_PRINT(("WL%d: DFS: %s ########## RADAR DETECTED ON U80 CHANNEL %s"
				" ########## min_pw=%d, AT %dMS\n", wlc->pub->unit,
					radar_type_str_2,
					wf_chspec_ntoa(chspec, chanbuf),
					min_pw_2,
					(dfs->dfs_cac.cactime - dfs->dfs_cac.duration) *
					WLC_DFS_RADAR_CHECK_INTERVAL));
		}
	}

#endif /* BCMDBG || WLMSG_DFS || BCMDBG_ERR || WLTEST_DFSMSG */

	/* clear one-shot radar simulator */
	if (radar_sim_mask != 0) {
		wlc_11h_set_spect_state(wlc->m11h, cfg, radar_sim_mask, 0);
	}
	return TRUE;
}

/* set cacstate to IDLE and un-mute */
static void
wlc_dfs_cacstate_idle_set(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;
#if defined(BCMDBG) || defined(WLMSG_DFS)
	char chanbuf[CHANSPEC_STR_LEN];
#endif // endif

	wlc_dfs_cac_state_change(dfs, WL_DFS_CACSTATE_IDLE);
	wlc_mute(wlc, OFF, PHY_MUTE_FOR_PREISM);

	WL_DFS(("wl%d: dfs : state to %s chanspec %s at %dms\n",
		wlc->pub->unit, wlc_dfs_cacstate_str[dfs->dfs_cac.status.state],
		wf_chspec_ntoa_ex(WLC_BAND_PI_RADIO_CHANSPEC, chanbuf),
		(dfs->dfs_cac.cactime -
		dfs->dfs_cac.duration)*WLC_DFS_RADAR_CHECK_INTERVAL));

	dfs->dfs_cac.cactime =  /* unit in WLC_DFS_RADAR_CHECK_INTERVAL */
	dfs->dfs_cac.duration = wlc_dfs_ism_cactime(wlc, dfs->dfs_cac.cactime_post_ism);
}

/* set cacstate to ISM and un-mute */
static void
wlc_dfs_cacstate_ism_set(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;
	int  cal_mode;
#ifdef CLIENT_CSA
	bool update = TRUE;
#else
	bool update = FALSE;
#endif // endif

#if defined(BCMDBG) || defined(WLMSG_DFS)
	char chanbuf[CHANSPEC_STR_LEN];
#endif // endif

	dfs->dfs_cac.status.chanspec_cleared = WLC_BAND_PI_RADIO_CHANSPEC;
	 /* clear the channel */
	wlc_clr_quiet_chanspec(wlc->cmi, dfs->dfs_cac.status.chanspec_cleared);

	wlc_dfs_cac_state_change(dfs, WL_DFS_CACSTATE_ISM);
	wlc_mute(wlc, OFF, PHY_MUTE_FOR_PREISM);

	wlc_iovar_getint(wlc, "phy_percal", (int *)&cal_mode);
	wlc_iovar_setint(wlc, "phy_percal", PHY_PERICAL_SPHASE);
	wlc_phy_cal_perical(WLC_PI(wlc), PHY_PERICAL_UP_BSS);
	wlc_iovar_setint(wlc, "phy_percal", cal_mode);

	WL_DFS(("wl%d: dfs : state to %s chanspec %s at %dms\n",
		wlc->pub->unit, wlc_dfs_cacstate_str[dfs->dfs_cac.status.state],
		wf_chspec_ntoa_ex(WLC_BAND_PI_RADIO_CHANSPEC, chanbuf),
		(dfs->dfs_cac.cactime - dfs->dfs_cac.duration) * WLC_DFS_RADAR_CHECK_INTERVAL));

	dfs->dfs_cac.cactime =  /* unit in WLC_DFS_RADAR_CHECK_INTERVAL */
	dfs->dfs_cac.duration = wlc_dfs_ism_cactime(wlc, dfs->dfs_cac.cactime_post_ism);

#ifdef SLAVE_RADAR
	/* ISM started, lets prepare for join */
	wlc_join_bss_prep(wlc->cfg);
#endif /* SLAVE_RADAR */

#if defined(EXT_STA) || defined(CLIENT_CSA)
	if (WLEXTSTA_ENAB(wlc->pub) || update) {
		wlc_bsscfg_t *bsscfg;
		bsscfg = wlc_get_ap_bsscfg(dfs);
		if (bsscfg) {
			wlc_bss_mac_event(wlc, bsscfg, WLC_E_DFS_AP_RESUME, NULL,
			                  WLC_E_STATUS_SUCCESS, 0, 0, 0, 0);
		}
	}
#endif /* EXT_STA */
	BCM_REFERENCE(update);
}

/* set cacstate to OOC and mute */
static void
wlc_dfs_cacstate_ooc_set(wlc_dfs_info_t *dfs, uint target_state)
{
	wlc_info_t *wlc = dfs->wlc;

	wlc_mute(wlc, ON, PHY_MUTE_FOR_PREISM);

	wlc_dfs_cac_state_change(dfs, target_state);

	WL_DFS(("wl%d: dfs : state to %s at %dms\n",
		wlc->pub->unit, wlc_dfs_cacstate_str[dfs->dfs_cac.status.state],
		(dfs->dfs_cac.cactime - dfs->dfs_cac.duration) *
	        WLC_DFS_RADAR_CHECK_INTERVAL));

	dfs->dfs_cac.duration = dfs->dfs_cac.cactime; /* reset it */

#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub)) {
		wlc_bsscfg_t *bsscfg;
		bsscfg = wlc_get_ap_bsscfg(dfs);
		if (bsscfg) {
			wlc_bss_mac_event(wlc, bsscfg, WLC_E_DFS_AP_STOP, NULL,
			                  WLC_E_STATUS_NOCHANS, 0, 0, 0, 0);
		}
	}
#endif /* EXT_STA */
}

static void
wlc_dfs_cacstate_idle(wlc_dfs_info_t *dfs)
{
	wlc_dfs_timer_delete(dfs);
	dfs->dfs_cac_enabled = FALSE;
}

#if defined(RSDB_DFS_SCAN) || defined(BGDFS)
/* handles periodic CAC check when DFS scan is active in background */
static void
wlc_dfs_handle_dfs_scan_cacstate(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;
#if defined(BCMDBG) || defined(WLMSG_DFS)
	char chanbuf[CHANSPEC_STR_LEN];
#endif // endif

	BCM_REFERENCE(wlc);

	if (!DFS_SCAN_IN_PROGRESS(dfs))
		return;

#ifdef BGDFS
	if (BGDFS_ENAB(wlc->pub) && dfs->phymode == PHYMODE_3x3_1x1) {
		if (wlc_radar_detected(dfs, TRUE)) {
			if (dfs->dfs_cac.ism_monitor == TRUE) {
				/* channel switching is disabled */
				WL_DFS(("wl%d: dfs : current chanspec %s is maintained as channel "
						"switching is disabled\n", wlc->pub->unit,
						wf_chspec_ntoa_ex(dfs->sc_chspec, chanbuf)));
			} else {
				wlc_dfs_scan_complete_sc(dfs, DFS_SCAN_S_RADAR_FOUND, TRUE);
				wlc_dfs_chanspec_oos(dfs, dfs->sc_chspec);
			}
		} else if (dfs->scan_both && wlc_radar_detected(dfs, FALSE)) {
			if (dfs->dfs_cac.ism_monitor == TRUE) {
				/* channel switching is disabled */
				WL_DFS(("wl%d: dfs : current chanspec %s is maintained as channel "
						"switching is disabled\n", wlc->pub->unit,
						wf_chspec_ntoa_ex(WLC_BAND_PI_RADIO_CHANSPEC,
								chanbuf)));
			} else { /* Handling Radar detection on 3x3 core */
				wl_chan_switch_t csa;
				wlc_bsscfg_t *cfg = wlc->cfg;

				wlc_dfs_scan_complete(dfs, DFS_SCAN_S_RADAR_FOUND, TRUE);
				wlc_dfs_chanspec_oos(dfs, WLC_BAND_PI_RADIO_CHANSPEC);
				dfs->dfs_cac.chanspec_next = wlc_dfs_chanspec(dfs, TRUE);
				WL_DFS(("wl%d: Selected new channel 0x%x\n",
						wlc->pub->unit,	dfs->dfs_cac.chanspec_next));

				csa.chspec = dfs->dfs_cac.chanspec_next;
				csa.mode = DOT11_CSA_MODE_NO_TX;
				csa.count = MAX((WLC_DFS_CSA_MSEC/cfg->current_bss->beacon_period),
						WLC_DFS_CSA_BEACONS);
				/* ensure count is at least DTIM of this AP */
				csa.count = MAX(csa.count, cfg->current_bss->dtim_period);
				csa.reg = wlc_get_regclass(wlc->cmi, csa.chspec);
				csa.frame_type = CSA_BROADCAST_ACTION_FRAME;
				wlc_dfs_csa_each_up_ap(wlc, &csa, FALSE);
				wlc_dfs_cac_state_change(dfs, WL_DFS_CACSTATE_CSA);
				return;
			}
		}
	}
	else
#endif /* BGDFS */
	{
		if (wlc_radar_detected(dfs, FALSE)) {
			wlc_dfs_scan_complete(dfs, DFS_SCAN_S_RADAR_FOUND, TRUE);
			/* TBD If dfs_scan for next channel is required, need to handle that. */
		}
	}

	if (!dfs->dfs_cac.duration) {
#ifdef BGDFS
		if (BGDFS_ENAB(wlc->pub) && dfs->phymode == PHYMODE_3x3_1x1) {
			if (dfs->dfs_cac.ism_monitor != TRUE) {
				wlc_dfs_scan_complete_sc(dfs, DFS_SCAN_S_RADAR_FREE, TRUE);
			}
		}
		else
#endif // endif
		{
			wlc_dfs_scan_complete(dfs, DFS_SCAN_S_RADAR_FREE, TRUE);
		}
	}
}
#endif /* RSDB_DFS_SCAN || BGDFS */

static void
wlc_dfs_send_event(wlc_dfs_info_t *dfs, chanspec_t target_chanspec)
{
	wl_event_radar_detect_data_t radar_data = { 0 };
	wlc_info_t* wlc = dfs->wlc;

	radar_data.current_chanspec = WLC_BAND_PI_RADIO_CHANSPEC;
	radar_data.target_chanspec = target_chanspec;
	radar_data.radar_info[0].min_pw  = RADAR_INFO_EXTRACT(dfs->radar_info[0], MINPW);
	radar_data.radar_info[0].subband  = RADAR_INFO_EXTRACT(dfs->radar_info[0], SUBBAND);
	radar_data.radar_info[0].radar_type  = RADAR_INFO_EXTRACT(dfs->radar_info[0], TYPE);
	if (dfs->radar_info[1] != 0) {
		radar_data.radar_info[1].min_pw  = RADAR_INFO_EXTRACT(dfs->radar_info[1], MINPW);
		radar_data.radar_info[1].subband  = RADAR_INFO_EXTRACT(dfs->radar_info[1], SUBBAND);
		radar_data.radar_info[1].radar_type  = RADAR_INFO_EXTRACT(dfs->radar_info[1], TYPE);
	}
	wlc_bss_mac_event(wlc, wlc->cfg, WLC_E_RADAR_DETECTED, NULL, 0,
			0, 0, (void *)&radar_data, sizeof(radar_data));
	WL_DFS(("wl%d: DFS WLC_E_RADAR_DETECTED %04x/%04x on ch 0x%02x going to ch 0x%02x\n",
			WLCWLUNIT(wlc), dfs->radar_info[0], dfs->radar_info[1],
			radar_data.current_chanspec, radar_data.target_chanspec));
}

static void
wlc_dfs_to_backup_channel(wlc_dfs_info_t *dfs, bool radar_detected)
{
	uint target_state;
	wlc_info_t* wlc = dfs->wlc;

	if (radar_detected) {
		wlc_dfs_chanspec_oos(dfs, WLC_BAND_PI_RADIO_CHANSPEC);
	}
	dfs->dfs_cac.chanspec_next = wlc_dfs_chanspec(dfs, radar_detected);
	if (!dfs->dfs_cac.chanspec_next) {
		/* out of channels */
		if (dfs->dfs_cac.status.state == WL_DFS_CACSTATE_PREISM_CAC) {
			target_state = WL_DFS_CACSTATE_PREISM_OOC;
		} else {
			target_state = WL_DFS_CACSTATE_POSTISM_OOC;
		}
		wlc_dfs_cacstate_ooc_set(dfs, target_state);
		return;
	}

	wlc_do_chanswitch(wlc->cfg, dfs->dfs_cac.chanspec_next);
}

static void
wlc_dfs_cacstate_cac(wlc_dfs_info_t *dfs)
{
	wlc_info_t* wlc = dfs->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;
#if defined(BCMDBG) || defined(WLMSG_DFS)
	char chanbuf[CHANSPEC_STR_LEN];
#endif // endif

#ifdef SLAVE_RADAR
	int i;
	wlc_bsscfg_t *bsscfg;
	chanspec_t chanspec;
	uint16 chanspec_bw = CHSPEC_BW(WLC_BAND_PI_RADIO_CHANSPEC);
#endif /* SLAVE_RADAR */
	(void)cfg;
#if defined(RSDB_DFS_SCAN) || defined(BGDFS)
	if (DFS_SCAN_IN_PROGRESS(dfs)) {
		wlc_dfs_handle_dfs_scan_cacstate(dfs);
		return;
	}
#endif /* RSDB_DFS_SCAN || BGDFS */

	/*
	 * If CAC was done earlier and chanspec is clear, STA can go to ISM.
	 */
#ifdef SLAVE_RADAR
	if (WL11H_STA_ENAB(wlc) && wlc_radar_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC) &&
		(wlc_cac_is_clr_chanspec(wlc->dfs, WLC_BAND_PI_RADIO_CHANSPEC)))
	{
		dfs->dfs_cac.cactime = 0;
		dfs->dfs_cac.duration = 0;
	}
#endif /* SLAVE_RADAR */

	if (wlc_radar_detected(dfs, FALSE) == TRUE) {

#ifdef SLAVE_RADAR
		FOREACH_BSS(wlc, i, bsscfg) {
			if (BSSCFG_STA(bsscfg)) {
				/* radar detected. mark the channel back to QUIET channel */
				wlc_set_quiet_chanspec(wlc->cmi,
					dfs->dfs_cac.status.chanspec_cleared);
				dfs->dfs_cac.status.chanspec_cleared = 0; /* clear it */
				wlc_dfs_chanspec_oos(dfs, WLC_BAND_PI_RADIO_CHANSPEC);
				/* Radar detected during CAC */
				wlc_assoc_change_state(wlc->cfg, AS_DFS_CAC_FAIL);
				if (chanspec_bw == WL_CHANSPEC_BW_80) {
				       chanspec = CH80MHZ_CHSPEC(42, WL_CHANSPEC_CTL_SB_LL);
				}
				else if (chanspec_bw == WL_CHANSPEC_BW_40) {
				       chanspec = CH40MHZ_CHSPEC(38, WL_CHANSPEC_CTL_SB_LOWER);
				}
				else { /* chanspec is 20 MHz */
				       chanspec = CH20MHZ_CHSPEC(36);
				}
				if (wlc_valid_chanspec_db(wlc->cmi, chanspec)) {
					wlc_set_home_chanspec(wlc, chanspec);
					wlc_suspend_mac_and_wait(wlc);
					wlc_set_chanspec(wlc, chanspec);
					wlc_enable_mac(wlc);
				}
				/* Non Occupancy Period begins. */
				wlc_dfs_cacstate_idle_set(dfs); /* set to IDLE */
				return;
			}
		}
#endif /* SLAVE_RADAR */
		if (WL11H_AP_ENAB(wlc)) {
			wlc_dfs_to_backup_channel(dfs, TRUE);
#ifdef CLIENT_CSA
			if (((BSSCFG_STA(wlc->cfg) &&
				(DWDS_ENAB(wlc->cfg) ||
				MAP_ENAB(wlc->cfg))) ||
#if defined(WET) || defined(WET_DONGLE)
				(WET_ENAB(wlc)) ||
#endif /* WET || WET_DONGLE */
				FALSE)) {

				/* radar detected. mark the channel back to QUIET channel */
				wlc_set_quiet_chanspec(wlc->cmi,
					dfs->dfs_cac.status.chanspec_cleared);
				dfs->dfs_cac.status.chanspec_cleared = 0; /* clear it */
				wlc_dfs_chanspec_oos(dfs, WLC_BAND_PI_RADIO_CHANSPEC);
				/* Radar detected during CAC */
				wlc_assoc_change_state(wlc->cfg, AS_DFS_CAC_FAIL);
				WL_DFS(("wl%d: dfs radar detected \n", wlc->pub->unit));
			}
#endif /* CLIENT_CSA */
			if (wlc_radar_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC) == TRUE) {
				/* do cac with new channel */
				WL_DFS(("wl%d: dfs : state to %s chanspec %s at %dms\n",
					wlc->pub->unit,
					wlc_dfs_cacstate_str[dfs->dfs_cac.status.state],
					wf_chspec_ntoa_ex(WLC_BAND_PI_RADIO_CHANSPEC, chanbuf),
					(dfs->dfs_cac.cactime - dfs->dfs_cac.duration) *
					WLC_DFS_RADAR_CHECK_INTERVAL));
				/* Switched to new channel, set up correct pre-ISM timer */
				dfs->dfs_cac.duration =
				dfs->dfs_cac.cactime =
					wlc_dfs_ism_cactime(wlc, dfs->dfs_cac.cactime_pre_ism);
				return;
			}
			else {
				wlc_dfs_cacstate_idle_set(dfs); /* set to IDLE */
				return;
			}
		}

	}

	if (!dfs->dfs_cac.duration) {
		WL_DFS((" CAC duration 0\n"));
#if defined(SLAVE_RADAR)
		if (WL11H_STA_ENAB(wlc) &&
			!wlc_cac_is_clr_chanspec(dfs, WLC_BAND_PI_RADIO_CHANSPEC)) {
			wlc_cac_do_clr_chanspec(dfs, WLC_BAND_PI_RADIO_CHANSPEC);
			/* No radar during CAC */
			wlc_assoc_change_state(wlc->cfg, AS_DFS_ISM_INIT);
			return;
		}
#endif /* SLAVE_RADAR */
#ifdef CLIENT_CSA
		if ((BSSCFG_STA(wlc->cfg) && (DWDS_ENAB(wlc->cfg) || MAP_ENAB(wlc->cfg))) ||
#if defined(WET) || defined(WET_DONGLE)
			(WET_ENAB(wlc)) ||
#endif /* WET || WET_DONGLE */
			FALSE) {
			if (!wlc_cac_is_clr_chanspec(dfs, WLC_BAND_PI_RADIO_CHANSPEC)) {
				wlc_cac_do_clr_chanspec(dfs, WLC_BAND_PI_RADIO_CHANSPEC);
			}
		}
#endif /* CLIENT_CSA */

		/* cac completed. un-mute all. resume normal bss operation */
		wlc_dfs_cacstate_ism_set(dfs);

#ifdef CLIENT_CSA
		if ((BSSCFG_STA(wlc->cfg) && (DWDS_ENAB(wlc->cfg) || MAP_ENAB(wlc->cfg))) ||
#if defined(WET) || defined(WET_DONGLE)
			(WET_ENAB(wlc)) ||
#endif /* WET || WET_DONGLE */
			FALSE) {
			/* ISM started, lets prepare for join */
			wlc_assoc_change_state(wlc->cfg, AS_DFS_ISM_INIT);
			wlc_join_bss_prep(wlc->cfg);
		}
#endif /* CLIENT_CSA */
	}
}

static void
wlc_dfs_cacstate_ism(wlc_dfs_info_t *dfs)
{
	wlc_info_t* wlc = dfs->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;
	wl_chan_switch_t csa;
#if defined(BCMDBG) || defined(WLMSG_DFS)
	char chanbuf1[CHANSPEC_STR_LEN];
	char chanbuf2[CHANSPEC_STR_LEN];
#endif // endif

#ifdef BGDFS
	/* after moving to the new channel, modeswitch announcement is initiated */
	if (BGDFS_ENAB(wlc->pub) && dfs->upgrade_pending &&
			dfs->dfs_scan->modeswitch_state != DFS_MODESW_UPGRADE_IN_PROGRESS) {
		int delay = ((dfs->dfs_cac.cactime - dfs->dfs_cac.duration) *
				WLC_DFS_RADAR_CHECK_INTERVAL);
		if (delay > DFS_BG_UPGRADE_DELAY_MS) {
			dfs->dfs_scan->modeswitch_state = DFS_MODESW_UPGRADE_IN_PROGRESS;
			// delayed upgrade to 4x4
#ifdef WL_MODESW
			if (WLC_MODESW_ENAB(wlc->pub)) {
				(void) wlc_dfs_bg_upgrade_wlc(wlc);
			}
#else
			wlc_dfs_bg_upgrade_phy(dfs);
#endif /* WL_MODESW */
		}
	}
#endif /* BGDFS */

#ifdef WL_SCAN_DFS_HOME
	if (wlc->clk) {
		uint32 high, low;
		uint64 now = 0;
		wlc_read_tsf(wlc, &low, &high);
		now = (((uint64)((uint32)high) << 32) | (uint32)low);
		/* update the radar poll time stamp for every 150ms */
		wlc->scan->last_radar_poll = now;
	}
#endif /* WL_SCAN_DFS_HOME */

	if (wlc_radar_detected(dfs, FALSE) == FALSE)
		return;

	/* Ignore radar_detect, if STA conencted to upstream AP on radar channel
	 * and local AP is on same radar channel.
	 */
	if (WLC_APSTA_ON_RADAR_CHANNEL(wlc)) {
#ifdef CLIENT_CSA
		if ((BSSCFG_STA(wlc->cfg) && (DWDS_ENAB(wlc->cfg) || MAP_ENAB(wlc->cfg))) ||
#if defined(WET) || defined(WET_DONGLE)
			(WET_ENAB(wlc)) ||
#endif /* WET || WET_DONGLE */
			FALSE) {
			WL_DFS(("wl%d: dfs radar detected \n", wlc->pub->unit));
		} else
#endif /* CLIENT_CSA */
		{
			WL_DFS(("wl%d: dfs : radar detected but ignoring, dfs slave present\n",
				wlc->pub->unit));
			return;
		}
	}

	/* radar has been detected */

	if (dfs->dfs_cac.ism_monitor == TRUE) {
		/* channel switching is disabled */
		WL_DFS(("wl%d: dfs : current chanspec %s is maintained as channel switching is"
		        " disabled.\n",
		        wlc->pub->unit, wf_chspec_ntoa_ex(WLC_BAND_PI_RADIO_CHANSPEC, chanbuf1)));
		return;
	}

	wlc_dfs_chanspec_oos(dfs, WLC_BAND_PI_RADIO_CHANSPEC);
#ifdef SLAVE_RADAR
	/* Slave detected radar */
	wlc_11h_send_basic_report_radar(wlc, cfg, wlc_dfs_send_action_frame_complete);
	if (dfs->radar_report_timer_running == FALSE) {
		dfs->radar_report_timer_running = TRUE;
		wl_add_timer(wlc->wl, dfs->radar_report_timer,
		WLC_RADAR_NOTIFICATION_TIMEOUT, FALSE);
	}
	dfs->dfs_cac.duration = dfs->dfs_cac.cactime =
		wlc_dfs_ism_cactime(wlc, dfs->dfs_cac.cactime_post_ism);
	return;
#endif /* SLAVE_RADAR */
	if AP_ENAB(wlc->pub) {
		/* continue with CSA */
		/* it will be included in csa */
		dfs->dfs_cac.chanspec_next = wlc_dfs_chanspec(dfs, TRUE);
		WL_DFS(("Selected new channel 0x%x\n", dfs->dfs_cac.chanspec_next));
		/* Downgrade BW if newer channel is of lower BW */
		if (CHSPEC_IS160(WLC_BAND_PI_RADIO_CHANSPEC) &&
				(CHSPEC_BW(dfs->dfs_cac.chanspec_next) <
				CHSPEC_BW(WLC_BAND_PI_RADIO_CHANSPEC))) {
			int idx;
			wlc_bsscfg_t *apcfg;
			FOREACH_UP_AP(wlc, idx, apcfg) {
				WL_DFS(("Orig Chanspec %x Downgrading BW\n",
						WLC_BAND_PI_RADIO_CHANSPEC));
#ifdef WL_MODESW
				if (WLC_MODESW_ENAB(wlc->pub)) {
					int err = BCME_OK;
					uint32 ctrl_flags = MODESW_CTRL_AP_ACT_FRAMES |
						MODESW_CTRL_NO_ACK_DISASSOC;
					err = wlc_modesw_bw_switch(wlc->modesw,
							WLC_BAND_PI_RADIO_CHANSPEC,
							BW_SWITCH_TYPE_DNGRADE, apcfg,
							ctrl_flags);
					if (BCME_OK != err)
						WL_DFS(("wl%d: DFS wlc_modesw_bw_switch Failed\n",
								wlc->pub->unit));
				}
#endif /* WL_MODESW */
			}
		}

		/* send csa */
		if (!dfs->dfs_cac.chanspec_next) {
			/* out of channels */
			/* just use the current channel for csa */
			csa.chspec = WLC_BAND_PI_RADIO_CHANSPEC;
		} else {
			csa.chspec = dfs->dfs_cac.chanspec_next;
		}
		csa.mode = DOT11_CSA_MODE_NO_TX;
		csa.count = MAX((WLC_DFS_CSA_MSEC/cfg->current_bss->beacon_period),
				WLC_DFS_CSA_BEACONS);
		/* ensure count is at least DTIM of this AP */
		csa.count = MAX(csa.count, cfg->current_bss->dtim_period);
		csa.reg = wlc_get_regclass(wlc->cmi, csa.chspec);
		csa.frame_type = CSA_BROADCAST_ACTION_FRAME;
		/* Allow apsta to transmit CSA to upstream AP before all AP
		 * send Broadcast CSA to it's clients, As firmware mutes
		 * the DATA fifo in wlc_dfs_csa_each_up_ap
		 */
#ifdef CLIENT_CSA
		/* Intentional Radar detection with the configuration:
		 * Radio's primary bsscfg configured as STA with DWDS
		 * capability, MBSS created with AP ROLE.
		 * DWDS STA interface associated to upstream DWDS AP.
		 * In this case radar detection at DWDS_STA needs to be
		 * communicated to upstream AP.
		 * In case of two seperate radios (one is working as STA
		 * and another as AP) should not execute this. Radar
		 * detection at STA is not being honoured.
		 */
		if (!BSSCFG_AP(cfg) && (MAP_ENAB(cfg) || DWDS_ENAB(cfg) ||
#if defined(WET) || defined(WET_DONGLE)
			(WET_ENAB(wlc)) ||
#endif /* WET || WET_DONGLE */
			FALSE) && cfg->current_bss) {
			wlc_send_unicast_action_switch_channel(wlc->csa, cfg, &cfg->BSSID,
					&csa, DOT11_SM_ACTION_CHANNEL_SWITCH);
		}
#endif	/* CLIENT_CSA */
		wlc_dfs_csa_each_up_ap(wlc, &csa, FALSE);
	} /* AP_ENAB() */
	if (WL11H_AP_ENAB(wlc))
		wlc_dfs_cac_state_change(dfs, WL_DFS_CACSTATE_CSA);        /* next state */

	WL_DFS(("wl%d: dfs : state to %s chanspec current %s next %s at %dms, starting CSA"
		" process\n",
		wlc->pub->unit, wlc_dfs_cacstate_str[dfs->dfs_cac.status.state],
		wf_chspec_ntoa_ex(WLC_BAND_PI_RADIO_CHANSPEC, chanbuf1),
		wf_chspec_ntoa_ex(dfs->dfs_cac.chanspec_next, chanbuf2),
		(dfs->dfs_cac.cactime -
			dfs->dfs_cac.duration)*WLC_DFS_RADAR_CHECK_INTERVAL));

	dfs->dfs_cac.duration = dfs->dfs_cac.cactime =
		wlc_dfs_ism_cactime(wlc, dfs->dfs_cac.cactime_post_ism);
}

/* csa transmission */
static void
wlc_dfs_cacstate_csa(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;

	if ((wlc_11h_get_spect_state(wlc->m11h, cfg) &
	     (NEED_TO_SWITCH_CHANNEL | NEED_TO_UPDATE_BCN)) ||
	    (wlc->block_datafifo & DATA_BLOCK_QUIET))
	        return;

	/* csa completed - TBTT dpc switched channel */

	if (!(dfs->dfs_cac.chanspec_next)) {
	        /* ran out of channels, goto OOC */
	        wlc_dfs_cacstate_ooc_set(dfs, WL_DFS_CACSTATE_POSTISM_OOC);
		return;
	}

	if (wlc_radar_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC) == TRUE) {
		if (dfs->dfs_cac.cactime_post_ism) {
			wlc_dfs_cac_state_change(dfs, WL_DFS_CACSTATE_POSTISM_CAC);
			WL_DFS(("wl%d: dfs : state to %s at %dms\n",
				wlc->pub->unit,
				wlc_dfs_cacstate_str[dfs->dfs_cac.status.state],
			        (dfs->dfs_cac.cactime - dfs->dfs_cac.duration) *
			        WLC_DFS_RADAR_CHECK_INTERVAL));

			dfs->dfs_cac.duration =
			dfs->dfs_cac.cactime =
				wlc_dfs_ism_cactime(wlc, dfs->dfs_cac.cactime_post_ism);
				wlc_mute(wlc, ON, PHY_MUTE_FOR_PREISM);
		}
		else {
			wlc_dfs_cacstate_ism_set(dfs);
		}
	}
	else {
		wlc_dfs_cacstate_idle_set(dfs);
	}
	if (AP_ENAB(wlc->pub)) {
		wlc_update_beacon(wlc);
		wlc_update_probe_resp(wlc, TRUE);
	}
}

#ifdef SLAVE_RADAR
/*
 * This function will check the txstaus value for the Basic Measurement Report sent by the slave.
 * If ACK received, will send disassoc immediately.If ACK not received,
 * will retry again untill certain time.
 * If retry limit exceeded or time expired, will send disassoc requesti anyway.
 */
static bool
wlc_dfs_process_action_frame_status(wlc_info_t *wlc,
 wlc_dfs_info_t *dfs, uint txstatus)
{
	ASSERT(dfs);
	if (txstatus & TX_STATUS_ACK_RCV) {
		 /* Reset the Retry counter if ACK is received properly */
		dfs->actionfr_retry_counter = 0;
	}
	else if ((txstatus & TX_STATUS_MASK) == TX_STATUS_NO_ACK)
	{
		WL_DFS(("ACK NOT RECEIVED... retrying....%d\n",
			dfs->actionfr_retry_counter+1));
		if (dfs->actionfr_retry_counter < WLC_RADAR_REPORT_RETRY_LIMIT) {
			dfs->actionfr_retry_counter++;
			wlc_11h_send_basic_report_radar(wlc, wlc->cfg,
				wlc_dfs_send_action_frame_complete);
			return FALSE;
		} else {
			/* Retry limit Exceeded */
			dfs->actionfr_retry_counter = 0;
			return TRUE;
		}
	}
	return TRUE;
}

/*
 * Registered as an action frame call back, looks for the Frame Acknowledgement for
 * the basic report salve sends. Will retry until certain time and later send
 * dissasoc.
 */
void
wlc_dfs_send_action_frame_complete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	chanspec_t chanspec;
	uint16 chanspec_bw = CHSPEC_BW(WLC_BAND_PI_RADIO_CHANSPEC);
	wlc_dfs_info_t *dfs = (wlc_dfs_info_t *)arg;

	if (wlc_dfs_process_action_frame_status(wlc, dfs, txstatus) == TRUE) {
		/* delete the timer */
		if (dfs->radar_report_timer_running == TRUE)
			wl_del_timer(wlc->wl, dfs->radar_report_timer);
		wlc_disassociate_client(wlc->cfg, FALSE);
		if (chanspec_bw == WL_CHANSPEC_BW_80) {
		       chanspec = CH80MHZ_CHSPEC(42, WL_CHANSPEC_CTL_SB_LL);
		}
		else if (chanspec_bw == WL_CHANSPEC_BW_40) {
		       chanspec = CH40MHZ_CHSPEC(38, WL_CHANSPEC_CTL_SB_LOWER);
		}
		else { /* chanspec is 20 MHz */
		       chanspec = CH20MHZ_CHSPEC(36);
		}
		if (wlc_valid_chanspec_db(wlc->cmi, chanspec)) {
			wlc_set_home_chanspec(wlc, chanspec);
			wlc_suspend_mac_and_wait(wlc);
			wlc_set_chanspec(wlc, chanspec);
			wlc_enable_mac(wlc);
		}

		wlc_roamscan_start(wlc->cfg, WLC_E_REASON_RADAR_DETECTED);
		wlc_dfs_cac_state_change(dfs, WL_DFS_CACSTATE_IDLE);
	}
}
#endif /* SLAVE_RADAR */

/*
 * dfs has run Out Of Channel.
 * wait for a channel to come out of Non-Occupancy Period.
 */
static void
wlc_dfs_cacstate_ooc(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;
	uint    current_time;
#if defined(BCMDBG) || defined(WLMSG_DFS)
	char chanbuf[CHANSPEC_STR_LEN];
#endif // endif

	if (!(dfs->dfs_cac.chanspec_next = wlc_dfs_chanspec(dfs, FALSE))) {
		/* still no channel out of channels. Nothing to do */
		return;
	}

	wlc_do_chanswitch(wlc->cfg, dfs->dfs_cac.chanspec_next);

	if (wlc_radar_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC) == TRUE) {
		current_time = (dfs->dfs_cac.cactime -
			dfs->dfs_cac.duration)*WLC_DFS_RADAR_CHECK_INTERVAL;
		BCM_REFERENCE(current_time);

		/* unit of cactime is WLC_DFS_RADAR_CHECK_INTERVAL */
		if (dfs->dfs_cac.status.state == WL_DFS_CACSTATE_PREISM_OOC) {
			dfs->dfs_cac.cactime = dfs->dfs_cac.duration =
				wlc_dfs_ism_cactime(wlc, dfs->dfs_cac.cactime_pre_ism);
			wlc_dfs_cac_state_change(dfs, WL_DFS_CACSTATE_PREISM_CAC);
		} else {
			dfs->dfs_cac.cactime = dfs->dfs_cac.duration =
				wlc_dfs_ism_cactime(wlc, dfs->dfs_cac.cactime_post_ism);
			wlc_dfs_cac_state_change(dfs, WL_DFS_CACSTATE_POSTISM_CAC);
		}

		if (dfs->dfs_cac.cactime) {
			wlc_mute(wlc, ON, PHY_MUTE_FOR_PREISM);

			WL_DFS(("wl%d: dfs : state to %s chanspec %s at %dms\n",
				wlc->pub->unit,
				wlc_dfs_cacstate_str[dfs->dfs_cac.status.state],
				wf_chspec_ntoa_ex(WLC_BAND_PI_RADIO_CHANSPEC, chanbuf),
				current_time));
		} else {
			/* corresponding cac is disabled */
			wlc_dfs_cacstate_ism_set(dfs);
		}
	} else {
		wlc_dfs_cacstate_idle_set(dfs); /* set to idle */
	}
}

static void
wlc_dfs_cacstate_handler(void *arg)
{
	wlc_dfs_info_t *dfs = (wlc_dfs_info_t *)arg;
	wlc_info_t *wlc = dfs->wlc;

	if (!wlc->pub->up || !dfs->dfs_cac_enabled)
		return;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}

#ifdef EXT_STA
	if (WLEXTSTA_ENAB(wlc->pub) && SCAN_IN_PROGRESS(wlc->scan)) {
		return;
	}
#endif // endif

	ASSERT(dfs->dfs_cac.status.state < WL_DFS_CACSTATES);

	wlc_dfs_cacstate_fn_ary[dfs->dfs_cac.status.state](dfs);

	dfs->dfs_cac.duration--;
}

static void
wlc_dfs_cacstate_init(wlc_dfs_info_t *dfs)
{
	wlc_info_t *wlc = dfs->wlc;
	wlc_bsscfg_t *cfg = wlc->cfg;
	int skip_pre_ism = FALSE;
	chanspec_t chspec = WLC_BAND_PI_RADIO_CHANSPEC;
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

#ifdef SLAVE_RADAR
	ASSERT(WL11H_AP_ENAB(wlc) || WL11H_STA_ENAB(wlc));
#else
	ASSERT(WL11H_AP_ENAB(wlc));
#endif /* SLAVE_RADAR */

	if (!wlc->pub->up)
		return;

	/* DFS works on the primary virtual inteface only; not on virtual interfaces in MBSS */
	if (!BSSCFG_IS_PRIMARY(cfg)) {
		return;
	}

	if (SCAN_IN_PROGRESS(wlc->scan)) {
		return;
	}

	phy_radar_detect_enable((phy_info_t *)WLC_PI(wlc), dfs->radar != 0);

#ifdef BGDFS
	/* override chanspec with scan core's if scan core is active (eg. in 3x1 mode) */
	if (BGDFS_ENAB(wlc->pub) && (dfs->phymode = PHYMODE(wlc)) == PHYMODE_3x3_1x1) {
		chspec = dfs->sc_chspec;
		wlc_csa_reset_all(wlc->csa, cfg);
		wlc_quiet_reset_all(wlc->quiet, cfg);
		if (wlc_radar_chanspec(wlc->cmi, chspec) != TRUE) {
			wlc_dfs_cacstate_idle_set(dfs); /* set to idle */
			return;
		}
		dfs->dfs_cac_enabled = TRUE;
		wlc_dfs_timer_add(dfs);
		/* unit of cactime is WLC_DFS_RADAR_CHECK_INTERVAL */
		dfs->dfs_cac.cactime = wlc_dfs_ism_cactime(wlc, dfs->dfs_cac.cactime_pre_ism);
		dfs->dfs_scan->status = DFS_SCAN_S_INPROGESS;
		wlc_dfs_cac_state_change(dfs, WL_DFS_CACSTATE_PREISM_CAC);
		dfs->dfs_cac.duration = dfs->dfs_cac.cactime;
		// wlc_dfs_cacstate_ism_set(dfs);
		wlc_radar_detected(dfs, TRUE); /* refresh detector */
		WL_REGULATORY(("wl%d: %s: state to %s chanspec %s BGDFS\n",
				wlc->pub->unit, __FUNCTION__,
				wlc_dfs_cacstate_str[dfs->dfs_cac.status.state],
				wf_chspec_ntoa_ex(chspec, chanbuf)));
		return;
	}
#endif /* BGDFS */

	if (wlc_radar_chanspec(wlc->cmi, dfs->dfs_cac.status.chanspec_cleared) == TRUE) {
		/* restore QUIET setting unless in EU */
		if (!wlc->is_edcrs_eu) {
			wlc_set_quiet_chanspec_exclude(wlc->cmi,
				(dfs->dfs_cac.status.chanspec_cleared), chspec);
		}
	}
	dfs->dfs_cac.status.chanspec_cleared = 0; /* clear it */

	wlc_csa_reset_all(wlc->csa, cfg);
	wlc_quiet_reset_all(wlc->quiet, cfg);

	if (wlc_radar_chanspec(wlc->cmi, chspec) == TRUE) {
		dfs->dfs_cac_enabled = TRUE;
		wlc_dfs_timer_add(dfs);
#if defined(RSDB_DFS_SCAN) || defined(BGDFS)
		if (dfs->dfs_scan->status == DFS_SCAN_S_RADAR_FREE &&
			dfs->dfs_scan->csa.chspec == chspec &&
			(BGDFS_ENAB(wlc->pub) || RSDB_ENAB(wlc->pub))) {
			skip_pre_ism = TRUE;
			dfs->dfs_scan->status = DFS_SCAN_S_IDLE;
		}
#endif /* RSDB_DFS_SCAN || BGDFS */
		/* unit of cactime is WLC_DFS_RADAR_CHECK_INTERVAL */
		dfs->dfs_cac.cactime = wlc_dfs_ism_cactime(wlc, dfs->dfs_cac.cactime_pre_ism);
		if ((!WLC_APSTA_ON_RADAR_CHANNEL(wlc) && dfs->dfs_cac.cactime && !skip_pre_ism) &&
			(!(APSTA_ENAB(wlc->pub) && AP_ENAB(wlc->pub)) ||
#ifdef CLIENT_CSA
			!wlc_cac_is_clr_chanspec(dfs, WLC_BAND_PI_RADIO_CHANSPEC) ||
#endif /* CLIENT_CSA */
			FALSE)) {

			/* With below steps, dfs_slave_present flag does not get set and
			 * hence while going into CAC state machine on switching to DFS
			 * radar channel, APSTA configuration goes to CAC instead to ISM:
			 * Steps:
			 * 1: Boot Upstream AP boots into Non DFS channel say 149/80
			 * 2: Reboot Repeater, and associate to Upstream AP
			 * 3: Issue dfs_ap_move to DFS radar channel
			 * 4: At end of 60 sec, Repeater starts CAC on receiving CSA from
			 *    Upstream AP, though it is beaconing.
			 * To prevent this: APSTA_ENAB and AP_ENAB check
			 */
			/* preism cac is enabled */
			wlc_dfs_cac_state_change(dfs, WL_DFS_CACSTATE_PREISM_CAC);
			dfs->dfs_cac.duration = dfs->dfs_cac.cactime;
			wlc_mute(wlc, ON, PHY_MUTE_FOR_PREISM);
		} else {
			/* preism cac is disabled */
			wlc_dfs_cacstate_ism_set(dfs);
		}

		(void) wlc_radar_detected(dfs, FALSE); /* refresh detector */

	} else {
		wlc_dfs_cacstate_idle_set(dfs); /* set to idle */
	}

	WL_REGULATORY(("wl%d: %s: state to %s chanspec %s NOT BGDFS\n",
		wlc->pub->unit, __FUNCTION__,
		wlc_dfs_cacstate_str[dfs->dfs_cac.status.state],
		wf_chspec_ntoa_ex(chspec, chanbuf)));
}

void
wlc_set_dfs_cacstate(wlc_dfs_info_t *dfs, int state)
{
	wlc_info_t *wlc = dfs->wlc;

	(void)wlc;

	WL_REGULATORY(("wl%d: %s dfs from %s to %s on channel 0x%x\n",
		wlc->pub->unit, __FUNCTION__, dfs->dfs_cac_enabled ? "ON":"OFF",
		state ? "ON":"OFF", WLC_BAND_PI_RADIO_CHANSPEC));

	if (state == OFF) {
		if (dfs->dfs_cac_enabled) {
			wlc_dfs_cacstate_idle_set(dfs);
			wlc_dfs_cacstate_idle(dfs);
		}
	} else {
		/* start CAC unless the chanspec isn't valid now (eg. when marked inactive) */
		if (!wlc_valid_dfs_chanspec(wlc, WLC_BAND_PI_RADIO_CHANSPEC) &&
				!DFS_SCAN_IN_PROGRESS(dfs)) {
			WL_REGULATORY(("wl%d: %s dfs avoiding channel 0x%x; finding backup\n",
					wlc->pub->unit, __FUNCTION__,
					WLC_BAND_PI_RADIO_CHANSPEC));
			wlc_dfs_to_backup_channel(dfs, FALSE);
		} else {
			wlc_dfs_cacstate_init(dfs);
		}
	}
}

bool
wlc_dfs_monitor_mode(wlc_dfs_info_t *dfs)
{
	return (dfs->dfs_cac.ism_monitor != 0);
}

#ifdef WL_DFS_WAVE_MODE
bool
wlc_dfs_test_mode(wlc_dfs_info_t *dfs)
{
	return (dfs->dfs_cac.test_mode != 0);
}
#endif /* WL_DFS_WAVE_MODE */

chanspec_t
wlc_dfs_sel_chspec(wlc_dfs_info_t *dfs, bool force)
{
	wlc_info_t *wlc = dfs->wlc;
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

	(void)wlc;

	if (!force && dfs->dfs_cac.chanspec_next != 0)
		return dfs->dfs_cac.chanspec_next;

	dfs->dfs_cac.chanspec_next = wlc_dfs_chanspec(dfs, FALSE);

	WL_REGULATORY(("wl%d: %s: dfs selected chanspec %s\n",
	               wlc->pub->unit, __FUNCTION__,
	               wf_chspec_ntoa_ex(dfs->dfs_cac.chanspec_next, chanbuf)));

	return dfs->dfs_cac.chanspec_next;
}

void
wlc_dfs_reset_all(wlc_dfs_info_t *dfs)
{
	bzero(dfs->chan_blocked, sizeof(dfs->chan_blocked));
	bzero(dfs->chan_cac_pending, sizeof(dfs->chan_cac_pending));
}

int
wlc_dfs_set_radar(wlc_dfs_info_t *dfs, int radar, uint subband)
{
	wlc_info_t *wlc = dfs->wlc;
	wlcband_t *band5G;
	int radar_max = (int)WL_RADAR_SIMULATED;
	chanspec_t chanspec = WLC_BAND_PI_RADIO_CHANSPEC;
	int spec_state = RADAR_SIM;

#ifdef BGDFS
	if (BGDFS_ENAB(wlc->pub)) {
		/* override range max, chanspec and spec state */
		radar_max = (int)WL_RADAR_SIMULATED_SC;
		if (radar == (int)WL_RADAR_SIMULATED_SC) {
			WL_DFS(("wl%d: %s simulation requested on scan core\n",
					wlc->pub->unit, __FUNCTION__));
			if (dfs->phymode != PHYMODE_3x3_1x1) {
				WL_DFS(("wl%d: %s rejecting one shot radar simulation "
						"as scan core is not active\n",
						wlc->pub->unit, __FUNCTION__));
				return BCME_NOTREADY;
			}
			chanspec = dfs->sc_chspec;
			spec_state = RADAR_SIM_SC;
		}
	}
#endif /* BGDFS */

	if (radar < 0 || radar > radar_max) {
		return BCME_RANGE;
	}

	/*
	 * WL_RADAR_SIMULATED / WL_RADAR_SIMULATED_SC are used for Wi-Fi testing.
	 */
	if (radar == (int)WL_RADAR_SIMULATED || radar == (int)WL_RADAR_SIMULATED_SC) {
		bool has_subband_info = DFS_HAS_SUBBAND_INFO(wlc);
		/* Radar must be enabled to pull test trigger */
		if (dfs->radar != 1) {
			WL_DFS(("wl%d: %s rejecting one shot radar simulation "
					"as radar detection is not enabled\n",
					wlc->pub->unit, __FUNCTION__));
			return BCME_BADARG;
		}
		/* Can't do radar detect on non-radar channel */
		if (wlc_radar_chanspec(wlc->cmi, chanspec) != TRUE) {
			WL_DFS(("wl%d: %s rejecting one shot radar simulation "
					"as chanspec (0x%x) is not a DFS channel\n",
					wlc->pub->unit, __FUNCTION__, chanspec));
			return BCME_BADCHAN;
		}

		wlc_11h_set_spect_state(wlc->m11h, wlc->cfg, spec_state, spec_state);
		WL_DFS(("wl%d: %s enabled one shot radar simulation on %s core\n",
				wlc->pub->unit, __FUNCTION__,
				(radar == (int)WL_RADAR_SIMULATED_SC)?"scan":"main"));

		if (has_subband_info) {
			dfs->radar_subbands = (uint8)subband & 0xFF;
			WL_DFS(("Simulated radar on subband 0x%x\n", subband));
		}

		return BCME_OK;
	}

	if ((int)dfs->radar == radar) {
		return BCME_OK;
	}

	/* Check there is a 5G band available */
	if (BAND_5G(wlc->band->bandtype)) {
		band5G = wlc->band;
	} else if (NBANDS(wlc) > 1 &&
			BAND_5G(wlc->bandstate[OTHERBANDUNIT(wlc)]->bandtype)) {
		band5G = wlc->bandstate[OTHERBANDUNIT(wlc)];
	} else {
		return BCME_BADBAND;
	}

	/* bcmerror if APhy rev 3+ support in any bandunit */
	if (WLCISAPHY(band5G) && AREV_LT(band5G->phyrev, 3)) {
		return BCME_UNSUPPORTED;
	}

	dfs->radar = (uint32)radar;

	phy_radar_detect_enable((phy_info_t *)WLC_PI(wlc), radar != 0);

	/* if we are not currently on the APhy, then radar detect
	 * will be initialized in the phy init
	 */

	WL_DFS(("wl%d: %s DFS radar=%d\n",
			wlc->pub->unit, __FUNCTION__, dfs->radar));
	return BCME_OK;
}

uint32
wlc_dfs_get_radar(wlc_dfs_info_t *dfs)
{
	WL_DFS(("wl%d: %s DFS radar=%d\n",
			dfs->wlc->pub->unit, __FUNCTION__, dfs->radar));
	return dfs->radar;
}

#ifdef STA
#ifdef BCMDBG
static int
wlc_dfs_bcn_parse_ibss_dfs_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	return BCME_OK;
}
#endif /* BCMDBG */
#endif /* STA */
#endif /* WLDFS */
