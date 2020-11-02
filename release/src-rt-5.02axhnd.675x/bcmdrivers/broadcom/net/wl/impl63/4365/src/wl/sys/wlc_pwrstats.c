/**
 * @file
 * @brief
 * Power statistics: Collect power related stats and expose to other modules
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
 * $Id:$
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wlc_scan.h>
#include <wlc_rm.h>
#include <wlc_pwrstats.h>
#ifdef WLAWDL
#include <wlc_awdl.h>
#endif /* WLAWDL */
#include <wl_export.h>

#ifdef WLMCNX
#include <wlc_mcnx.h>
#endif /* WLMCNX */

#ifdef EVENT_LOG_COMPILE
#include <event_log.h>
#endif // endif

/* For recording connection stats */
typedef struct wlc_conn_stats {
	uint32	connect_count;
	uint32	connect_dur;
	uint32	connect_start;
	uint32	connect_end;
	bool	connect_in_progress;
} wlc_conn_stats_t;

/* Info block related to pwrstats */
typedef struct wlc_pwrstats_info {
	wlc_info_t *wlc;
	bool   wakelog;		/* Do pmwake logging */

	/* Transient logging state */
	uint32 *pmwake_start_time;
	uint32 pmwake_prev_reason;

	/* For recording phy (tx/rx duration) info */
	wl_pwr_phy_stats_t *phy;

	/* absolute cumulative drift value; reset on wrapping */
	uint32	cum_drift_abs;
	wlc_pm_debug_t *pm_state;
	uint32 *pmd_event_wake_dur;

	/* Wake info/history for reporting */
	pm_awake_data_v2_t	*pmwake;
	wlc_conn_stats_t *cs;
	uint32	frts_start; /* ms at start of current frts period */
	bool	frts_in_progress;
	bool	frts_data; /* Is current frts started due to data frame? */
	uint32	frts_data_dur; /* cumulative frts dur due to data frame */
} wlc_pwrstats_info_t;

typedef struct wlc_pwrstats_mem {
	wlc_pwrstats_info_t info;
	wl_pwr_phy_stats_t phy;
	pm_awake_data_v2_t pmwake;
	wlc_conn_stats_t cs;
	wlc_pm_debug_t pm_state[WLC_STA_AWAKE_STATES_MAX]; /* timestamped wake bits */
	uint32 pmd_event_wake_dur[WLC_PMD_EVENT_MAX];      /* cumulative usecs per wake reason */
	uint32 pmwake_start_time[WLC_PMD_EVENT_MAX];
} wlc_pwrstats_mem_t;

/* iovar table */
enum {
	IOV_PWRSTATS,
	IOV_DRIFT_STATS_RESET,
	IOV_LAST
};

static const bcm_iovar_t pwrstats_iovars[] = {
	{"pwrstats", IOV_PWRSTATS,
	0, IOVT_BUFFER, WL_PWR_STATS_HDRLEN,
	},
	{"drift_stats_reset", IOV_DRIFT_STATS_RESET,
	0, IOVT_VOID, 0
	},
	{NULL, 0, 0, 0, 0}
};

static int wlc_pwrstats_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
static int wlc_pwrstats_get(wlc_info_t *wlc, void *params, uint p_len,
	void *arg, int len, wlc_bsscfg_t *bsscfg);
static int wlc_pwrstats_get_wake(wlc_info_t *wlc, uint8 *destptr, int destlen);
static int wlc_pwrstats_get_connection(wlc_pwrstats_info_t *ps, uint8 *destptr, int destlen);
static void wlc_pwrstats_watchdog(void *context);
static void wlc_pwrstats_phy_stats_upd(wlc_info_t *wlc);
static void wlc_pwrstats_reset_drift_stats(wlc_pwrstats_info_t *ps);
#ifdef WLMCNX
static void wlc_pwrstats_drift_upd(wlc_pwrstats_info_t *ps, int32 drift);
#endif /* WLMCNX */

static void
pwrstats_bsscfg_up_down(void *ctx, bsscfg_up_down_event_data_t *evt_data)
{
	wlc_pwrstats_info_t *ps = (wlc_pwrstats_info_t *)ctx;
	wlc_bsscfg_t *cfg = evt_data->bsscfg;

	BCM_REFERENCE(ps);
	if (evt_data == NULL || evt_data->bsscfg == NULL) {
		WL_ERROR(("%s:evt_data or bsscfg NULL\n", __FUNCTION__));
		return;
	}

	if (!evt_data->up) {
		/* Only invoke for primary infra STA */
		if (cfg->BSS && BSSCFG_STA(cfg) && cfg == cfg->wlc->cfg) {
			wlc_pwrstats_frts_end(cfg->wlc->pwrstats);
		}
	}
}

void *
BCMATTACHFN(wlc_pwrstats_attach)(wlc_info_t *wlc)
{
	wlc_pwrstats_info_t *ps = NULL;
	wlc_pwrstats_mem_t *ps_mem = NULL;
	int err;
	/* try and allocate the power stats stuff */
	if ((ps_mem = MALLOCZ(wlc->osh, sizeof(wlc_pwrstats_mem_t)))) {
		ps = (wlc_pwrstats_info_t *)ps_mem;
		ps->pmwake = &ps_mem->pmwake;
		ps->phy = &ps_mem->phy;
		ps->cs = &ps_mem->cs;
		ps->pm_state = ps_mem->pm_state;
		ps->pmd_event_wake_dur = ps_mem->pmd_event_wake_dur;
		ps->pmwake_start_time = ps_mem->pmwake_start_time;
		ps->pmwake->pm_state_offset = sizeof(pm_awake_data_v2_t);
		ps->pmwake->pm_state_len = WLC_STA_AWAKE_STATES_MAX;
		ps->pmwake->pmd_event_wake_dur_offset = sizeof(pm_awake_data_v2_t) +
			WLC_STA_AWAKE_STATES_MAX * sizeof(wlc_pm_debug_t);
		ps->pmwake->pmd_event_wake_dur_len = WLC_PMD_EVENT_MAX;
	} else
		goto fail;

	/* register module */
	if (wlc_module_register(wlc->pub, pwrstats_iovars, "pwrstats", ps, wlc_pwrstats_doiovar,
		wlc_pwrstats_watchdog, NULL, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wlc->pub->_pwrstats = TRUE;
	ps->wakelog = TRUE;
	ps->wlc = wlc;

	/* bss up/down */
	err = wlc_bsscfg_updown_register(wlc, pwrstats_bsscfg_up_down, (void *)ps);
	if (err != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register failed, error %d\n",
		        WLCWLUNIT(wlc), __FUNCTION__, err));
		goto fail;
	}

	return ps;
fail:
	wlc_pwrstats_detach(ps);
	return NULL;
}

void
BCMATTACHFN(wlc_pwrstats_detach)(void *pwrstats)
{
	wlc_pwrstats_info_t *ps = (wlc_pwrstats_info_t *)pwrstats;

	if (ps == NULL)
		return;

	/* unregister for bss up/down */
	wlc_bsscfg_updown_unregister(ps->wlc, pwrstats_bsscfg_up_down, (void *)ps);

	wlc_module_unregister(ps->wlc->pub, "pwrstats", ps);
	MFREE(ps->wlc->osh, ps, sizeof(wlc_pwrstats_mem_t));
}

static int
wlc_pwrstats_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_pwrstats_info_t *ps = (wlc_pwrstats_info_t *)hdl;
	wlc_info_t * wlc = ps->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = BCME_OK;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	switch (actionid) {
	case IOV_GVAL(IOV_PWRSTATS):
		if (PWRSTATS_ENAB(wlc->pub))
			err = wlc_pwrstats_get(wlc, params, p_len, arg, len, bsscfg);
		else
			err = BCME_UNSUPPORTED;
		break;
	case IOV_SVAL(IOV_DRIFT_STATS_RESET):
		wlc_pwrstats_reset_drift_stats(ps);
		break;
	}
	return err;
}

static void
wlc_pwrstats_watchdog(void *context)
{
	wlc_pwrstats_info_t *ps = (wlc_pwrstats_info_t *)context;
	/* Update some power stats regardless of CCA_STATS definiiton. */
	if (PWRSTATS_ENAB(ps->wlc->pub))
		wlc_pwrstats_phy_stats_upd(ps->wlc);
}

/* Invoked to start frts time accounting */
void
wlc_pwrstats_frts_start(void *ctx)
{
	wlc_pwrstats_info_t *ps = (wlc_pwrstats_info_t *)ctx;

	if (ps->frts_in_progress == FALSE) {
		ps->frts_in_progress = TRUE;
		ps->frts_start = OSL_SYSUPTIME();
	}
}
/* Invoked at the end of frts to update cumulative frts time */
void
wlc_pwrstats_frts_end(void *ctx)
{
	wlc_pwrstats_info_t *ps = (wlc_pwrstats_info_t *)ctx;

	if (ps->frts_in_progress) {
		ps->frts_in_progress = FALSE;
		ps->pmwake->frts_time += OSL_SYSUPTIME() - ps->frts_start;
		ps->pmwake->frts_end_cnt++;
		if (ps->frts_data)
			ps->frts_data_dur += OSL_SYSUPTIME() - ps->frts_start;
		ps->frts_data = 0;
	}
}

/* This function updates frts time till now and restarts frts only if it is in progress.
 * It is invoked to report accurate time to host while the timer is active.
 */
void
wlc_pwrstats_frts_checkpoint(wlc_pwrstats_info_t *ps)
{
	uint32 data_backup = ps->frts_data;
	if (ps->frts_in_progress) {
		wlc_pwrstats_frts_end(ps);
		ps->frts_data = data_backup;
		wlc_pwrstats_frts_start(ps);
	}
}

/* Fill pm_alert with pwrstats maintainted data and return updated ptr */
uint8 *
wlc_pwrstats_fill_pmalert(wlc_info_t *wlc, uint8 *data)
{
	wlc_pwrstats_info_t *ps = wlc->pwrstats;
	wl_pmalert_pmstate_t *pmstate;
	wl_pmalert_event_dur_t *event_dur;
	wl_pmalert_fixed_t *fixed;

	wlc_pwrstats_frts_checkpoint(wlc->pwrstats);

	fixed = (wl_pmalert_fixed_t *)data;
	fixed->last_drift = ps->pmwake->last_drift;
	fixed->min_drift = ps->pmwake->min_drift;
	fixed->max_drift = ps->pmwake->max_drift;
	fixed->avg_drift = ps->pmwake->avg_drift;
	fixed->drift_cnt = ps->pmwake->drift_cnt;
	fixed->frts_time = ps->frts_data_dur;
	fixed->frts_end_cnt = ps->pmwake->frts_end_cnt;

	data += sizeof(wl_pmalert_fixed_t);
	pmstate = (wl_pmalert_pmstate_t *)data;
	pmstate->type = WL_PMALERT_PMSTATE;
	pmstate->len = sizeof(wl_pmalert_pmstate_t) +
		sizeof(wlc_pm_debug_t) * (WLC_STA_AWAKE_STATES_MAX - 1);
	pmstate->pmwake_idx = ps->pmwake->pmwake_idx;
	memcpy(pmstate->pmstate, ps->pm_state, pmstate->len);

	data += pmstate->len;
	event_dur = (wl_pmalert_event_dur_t *)data;
	event_dur->type = WL_PMALERT_EVENT_DUR;
	event_dur->len = sizeof(wl_pmalert_event_dur_t) +
		sizeof(uint32) * (WLC_PMD_EVENT_MAX - 1);
	memcpy(event_dur->event_dur, ps->pmd_event_wake_dur,
		sizeof(event_dur->event_dur[0]) * WLC_PMD_EVENT_MAX);

	data += event_dur->len;

	event_dur = (wl_pmalert_event_dur_t *)data;
	event_dur->type = WL_PMALERT_EPM_START_EVENT_DUR;
	event_dur->len = sizeof(wl_pmalert_event_dur_t) +
		sizeof(uint32) * (WLC_PMD_EVENT_MAX - 1);
	memcpy(event_dur->event_dur, wlc->excess_pmwake->pp_start_event_dur,
		sizeof(event_dur->event_dur[0]) * WLC_PMD_EVENT_MAX);

	data += event_dur->len;

	return data;
}

static void
wlc_pwrstats_phy_stats_upd(wlc_info_t *wlc)
{
	wlc_pwrstats_info_t *ps = wlc->pwrstats;
	cca_ucode_counts_t tmp;

	ASSERT(ps);
	if (wlc_bmac_cca_stats_read(wlc->hw, &tmp) == BCME_OK) {
		ps->phy->tx_dur = tmp.txdur;
		ps->phy->rx_dur = tmp.ibss + tmp.obss + tmp.noctg + tmp.nopkt;
	}
}

static int
wlc_pwrstats_get_phy(wlc_pwrstats_info_t *ps, uint8 *destptr, int destlen)
{
	wl_pwr_phy_stats_t *phy_stats = ps->phy;
	uint16 taglen = sizeof(wl_pwr_phy_stats_t);

	/* Make sure there's room for this section */
	if (destlen < ROUNDUP(sizeof(wl_pwr_phy_stats_t), sizeof(uint32)))
		return BCME_BUFTOOSHORT;

	/* Update common structure fields and copy into the destination */
	phy_stats->type = WL_PWRSTATS_TYPE_PHY;
	phy_stats->len = taglen;
	memcpy(destptr, phy_stats, taglen);

	/* Report use of this segment (plus padding) */
	return (ROUNDUP(taglen, sizeof(uint32)));
}

static int
wlc_pwrstats_get_wake(wlc_info_t *wlc, uint8 *destptr, int destlen)
{
	wlc_pwrstats_info_t *ps = wlc->pwrstats;
	wl_pwr_pm_awake_stats_v2_t wake_stats;
	uint16 taglen = sizeof(wake_stats) +
		sizeof(wlc_pm_debug_t) * WLC_PMD_EVENT_MAX +
		sizeof(uint32) * WLC_STA_AWAKE_STATES_MAX;

	/* Make sure there's room for this section */
	if (destlen < ROUNDUP(taglen, sizeof(uint32)))
		return BCME_BUFTOOSHORT;

	/* Update fixed/common structure fields */
	wake_stats.type = WL_PWRSTATS_TYPE_PM_AWAKE2;
	wake_stats.len = taglen;

	wlc_pwrstats_frts_checkpoint(wlc->pwrstats);

	/* Copy struct with accumulated debug/wake state data */
	memcpy(&wake_stats.awake_data, ps->pmwake,
		sizeof(wake_stats.awake_data));

	/* Update immediate fields */
	wake_stats.awake_data.curr_time = OSL_SYSUPTIME();
#ifdef WLC_LOW
	{
		bool wasup = si_iscoreup(wlc->pub->sih);

		wake_stats.awake_data.pm_dur = wlc_get_accum_pmdur(wlc);
		if (!wasup)
			wlc_corereset(wlc, WLC_USE_COREFLAGS);
		wake_stats.awake_data.hw_macc = R_REG(wlc->osh, &wlc->regs->maccontrol);
		wake_stats.awake_data.sw_macc = wlc->hw->maccontrol;
		if (!wasup)
			wlc_coredisable(wlc->hw);
	}
#endif /* WLC_LOW */
	wake_stats.awake_data.mpc_dur = wlc_get_mpc_dur(wlc);
	/* Avoid divide by zero */
	if (ps->pmwake->drift_cnt)
		wake_stats.awake_data.avg_drift =
			ps->cum_drift_abs/ps->pmwake->drift_cnt;
	else
		/* Make sure min_drift is not reported as max int-32 value
		 * immediately after drift stats are reset.
		 */
		wake_stats.awake_data.min_drift = 0;

	memcpy(destptr, &wake_stats, sizeof(wake_stats));
	destptr += sizeof(wake_stats);

	memcpy(destptr, ps->pm_state,
		sizeof(wlc_pm_debug_t) * WLC_STA_AWAKE_STATES_MAX);

	destptr += sizeof(wlc_pm_debug_t) * WLC_STA_AWAKE_STATES_MAX;
	memcpy(destptr, ps->pmd_event_wake_dur,
		sizeof(uint32) * WLC_PMD_EVENT_MAX);
	/* Report use of this segment (plus padding) */
	return (ROUNDUP(taglen, sizeof(uint32)));
}

#define WLC_PWRSTATS_DEFTYPES { \
		WL_PWRSTATS_TYPE_PHY, \
		WL_PWRSTATS_TYPE_SCAN, \
		WL_PWRSTATS_TYPE_CONNECTION, \
		WL_PWRSTATS_TYPE_PCIE, \
		WL_PWRSTATS_TYPE_PM_AWAKE2 \
	}

void
wlc_pwrstats_reset_drift_stats(wlc_pwrstats_info_t *ps)
{
	ps->cum_drift_abs = 0;
	ps->pmwake->drift_cnt = 0;
	ps->pmwake->min_drift = INT32_MAX;
	ps->pmwake->max_drift = 0;
	ps->pmwake->last_drift = 0;
}

void
wlc_pwrstats_bcn_process(wlc_bsscfg_t *bsscfg, void *pwrstats, uint16 seq)
{
	wlc_pwrstats_info_t *ps = (wlc_pwrstats_info_t *)pwrstats;
	int32 drift = 0;

	/* To silence compiler warning that variable is unused */
	BCM_REFERENCE(drift);

	/* Reset drift parameters on receiving 1st bcn after assoc */
	if (bsscfg->assoc->state == AS_WAIT_RCV_BCN) {
		wlc_pwrstats_reset_drift_stats(ps);
	}
#ifdef WLMCNX
	else if (MCNX_ENAB(ps->wlc->pub)) {
		/* Assume drift is never beyond 31-bit magnitude */
		drift = wlc_mcnx_read_shm(ps->wlc->mcnx, M_P2P_TSF_DRIFT(1)) << 16;
		drift |= wlc_mcnx_read_shm(ps->wlc->mcnx, M_P2P_TSF_DRIFT(0));
		wlc_pwrstats_drift_upd(ps, drift);
	}
#endif /* WLMCNX */
	WL_PWRSTATS_INFO("Bcn recvd: seq:%d drift:%d\n",
		seq, drift, 0, 0);
}

static int
wlc_pwrstats_get_connection(wlc_pwrstats_info_t *ps, uint8 *destptr, int destlen)
{
	wlc_conn_stats_t *cs = ps->cs;
	wl_pwr_connect_stats_t connect_stats;
	uint16 taglen = sizeof(wl_pwr_connect_stats_t);

	/* Make sure there's room for this section */
	if (destlen < ROUNDUP(sizeof(wl_pwr_connect_stats_t), sizeof(uint32)))
		return BCME_BUFTOOSHORT;

	/* Update common structure fields and copy into the destination */
	connect_stats.type = WL_PWRSTATS_TYPE_CONNECTION;
	connect_stats.len = taglen;
	connect_stats.count = cs->connect_count;
	connect_stats.dur = cs->connect_dur + wlc_pwrstats_curr_connect_time(ps);

	memcpy(destptr, &connect_stats, taglen);

	/* Report use of this segment (plus padding) */
	return (ROUNDUP(taglen, sizeof(uint32)));
}

int
wlc_pwrstats_get(wlc_info_t *wlc, void *params, uint p_len,
	void *arg, int len, wlc_bsscfg_t *bsscfg)
{
	int rc = BCME_OK;
	uint16 ntypes = 0;
	uint16 typelist[] = WLC_PWRSTATS_DEFTYPES;
	uint16 *tptr = typelist;

	uint idx;
	uint16 type;

	uint8 *destptr;
	wl_pwrstats_t *pwrstats = arg;
	uint totlen = WL_PWR_STATS_HDRLEN;

	ASSERT(ISALIGNED(pwrstats, sizeof(uint32)));

	/* If there are parameters, get count of types */
	if (p_len > sizeof(uint16)) {
		memcpy(&ntypes, params, sizeof(uint16));
		if (p_len < sizeof(uint16) * (ntypes + 1)) {
			rc = BCME_BADARG;
			goto done;
		}
	}

	/* Malloc for the input list and copy it, set up for default */
	if (ntypes) {
		tptr = (uint16*)MALLOC(wlc->osh, ntypes * sizeof(uint16));
		if (tptr == NULL) {
			rc = BCME_NOMEM;
			goto done;
		}
		memcpy(tptr, (uint16*)params + 1, ntypes * sizeof(uint16));
	} else {
		ntypes = ARRAYSIZE(typelist);
	}

	/* Prepare the return header */
	pwrstats->version = WL_PWRSTATS_VERSION;
	destptr = (uint8*)arg + WL_PWR_STATS_HDRLEN;

	/* Go through the requested types */
	for (idx = 0; idx < ntypes; idx++, totlen += rc, destptr += rc) {
		type = tptr[idx];

		switch (type) {
		case WL_PWRSTATS_TYPE_PHY:
			rc = wlc_pwrstats_get_phy(wlc->pwrstats, destptr, len - totlen);
			break;

		case WL_PWRSTATS_TYPE_SCAN:
			rc = wlc_pwrstats_get_scan(wlc->scan, destptr, len - totlen);
			break;

		case WL_PWRSTATS_TYPE_PM_AWAKE2:
			rc = wlc_pwrstats_get_wake(wlc, destptr, len-totlen);
			break;

		case WL_PWRSTATS_TYPE_CONNECTION:
			rc = wlc_pwrstats_get_connection(wlc->pwrstats, destptr, len - totlen);
			break;
#ifdef DONGLEBUILD
		case WL_PWRSTATS_TYPE_PCIE:
#ifdef BCMPCIEDEV
			if (BCMPCIEDEV_ENAB()) {
				/* Need to generate an ioctl/iovar request to the bus */
				rc = strlen("bus:pwrstats");
				if ((len - totlen) > (rc + 1 + sizeof(uint16))) {
					strcpy((char*)destptr, "bus:pwrstats");
					destptr[rc] = '\0';
					memcpy(&destptr[rc + 1], &type, sizeof(uint16));
					rc = wl_busioctl(wlc->wl, BUS_GET_VAR, destptr,
						(len - totlen), NULL, NULL, FALSE);
					/* The result is iovar status, convert as needed */
					if (rc == BCME_OK) {
						rc = ((wl_pwr_pcie_stats_t*)destptr)->len;
						rc = ROUNDUP(rc, sizeof(uint32));
					}
				}
			} else {
				rc = BCME_UNSUPPORTED;
			}
#endif /* BCMPCIEDEV */
			break;
#endif /* DONGLEBUILD */
		default:
			/* XXX: TBD: Return some sort of indication that
			 * this tag is not supported in this firmware.
			 */
			rc = 0;
			break;
		}

		/* Exit loop on error */
		if (rc < 0)
			break;
	}

	/* Update total length */
	pwrstats->length = totlen;

done:
	if (tptr != NULL && tptr != typelist)
		MFREE(wlc->osh, tptr, ntypes * sizeof(uint16));

	return rc;
}

#ifdef WLMCNX
void
wlc_pwrstats_drift_upd(wlc_pwrstats_info_t *ps, int32 drift)
{
	ps->pmwake->last_drift = drift;

	if (ABS(ps->pmwake->min_drift) > ABS(drift))
		ps->pmwake->min_drift = drift;
	if (ABS(ps->pmwake->max_drift) < ABS(drift))
		ps->pmwake->max_drift = drift;

	/* Overflow/Wrapping? reset the drifts */
	if ((0xffffffff - ps->cum_drift_abs) < ABS(drift)) {
		ps->cum_drift_abs = ABS(drift);
		ps->pmwake->drift_cnt = 1;
		ps->pmwake->min_drift = drift;
		ps->pmwake->max_drift = drift;
	}
	else {
		ps->cum_drift_abs += ABS(drift);
		ps->pmwake->drift_cnt++;
	}
}
#endif /* WLMCNX */

void
wlc_pwrstats_wake_reason_upd(wlc_info_t *wlc, bool stay_awake)
{
	uint16 wake_mask = 0;
	int idx;
	wlc_bsscfg_t *cfg;
	wlc_pwrstats_info_t *ps = wlc->pwrstats;

	/* If wakelog is false we're done */
	if (!ps->wakelog) {
		return;
	}
	/* Nothing on now: if no change, return now. */
	if (!stay_awake && !ps->pmwake_prev_reason) {
		return;
	}

	/* check conditions again and record in bitmask */
	if (wlc->wake)
		wake_mask |= WLC_PMD_WAKE_SET;

	if (wlc->PMawakebcn)
		wake_mask |= WLC_PMD_PM_AWAKE_BCN;

	if (BTA_ACTIVE(wlc))
		wake_mask |= WLC_PMD_BTA_ACTIVE;

	if (SCAN_IN_PROGRESS(wlc->scan))
		wake_mask |= WLC_PMD_SCAN_IN_PROGRESS;

	if (WLC_RM_IN_PROGRESS(wlc))
		wake_mask |= WLC_PMD_RM_IN_PROGRESS;

	if (AS_IN_PROGRESS(wlc))
		wake_mask |= WLC_PMD_AS_IN_PROGRESS;

	if (wlc->PMpending)
		wake_mask |= WLC_PMD_PM_PEND;

	if (wlc->PSpoll)
		wake_mask |= WLC_PMD_PS_POLL;

	if (wlc->check_for_unaligned_tbtt)
		wake_mask |= WLC_PMD_CHK_UNALIGN_TBTT;

	if (wlc->apsd_sta_usp)
		wake_mask |= WLC_PMD_APSD_STA_UP;

	if (wlc->txpend16165war)
		wake_mask |= WLC_PMD_TX_PEND_WAR;

	if (wlc->gptimer_stay_awake_req)
		wake_mask |= WLC_PMD_GPTIMER_STAY_AWAKE;

#ifdef WLAWDL
	if (AWDL_ENAB(wlc->pub) && wlc_isawdl_awake(wlc))
		wake_mask |= WLC_PMD_AWDL_AWAKE;
#endif // endif
	if (wlc->pm2_radio_shutoff_pending)
		wake_mask |= WLC_PMD_PM2_RADIO_SOFF_PEND;

	ps->pmwake->mpc_dur = wlc_get_mpc_dur(wlc);
	ps->pmwake->pm_dur = wlc_get_accum_pmdur(wlc);

	FOREACH_AS_STA(wlc, idx, cfg) {
		if (cfg != wlc->cfg &&
#ifdef WLAWDL
			!BSSCFG_AWDL(wlc, cfg) &&
#endif // endif
#ifdef WLP2P
			!BSS_P2P_ENAB(wlc, cfg) &&
#endif // endif
			TRUE)
			wake_mask |= WLC_PMD_NON_PRIM_STA_UP;
	}

	/* stay awake as long as an infra STA is associating */
	FOREACH_AS_STA(wlc, idx, cfg) {
		if (cfg->BSS) {
			if (cfg->assoc != NULL &&
			    cfg->assoc->state != AS_IDLE)
			{
				wake_mask |= WLC_PMD_AS_IN_PROGRESS;
			}
		}
	}

	FOREACH_UP_AP(wlc, idx, cfg) {
#ifdef WLP2P
		if (!BSS_P2P_ENAB(wlc, cfg))
#endif // endif
		{
			wake_mask |= WLC_PMD_AP_UP;
		}
	}

	/* We couldn't determine the stay awake cause. */
	if (stay_awake && wake_mask == 0) {
		/* Add it to the AP bit until until we have an 'unknown' reason */
		wake_mask |= WLC_PMD_AP_UP;
	}

	if (ps->pmwake->pmwake_idx == WLC_STA_AWAKE_STATES_MAX)
		ps->pmwake->pmwake_idx = 0;

	if (wake_mask != ps->pmwake_prev_reason) {
		int i;
		uint32 curr_time = OSL_SYSUPTIME();

		ps->pm_state[ps->pmwake->pmwake_idx].timestamp = curr_time;
		ps->pm_state[ps->pmwake->pmwake_idx++].reason = wake_mask;

		/* Check which events added to the duration */
		for (i = 0; i < WLC_PMD_EVENT_MAX; i++) {
			if ((wake_mask & (1 << i)) &&
				!(ps->pmwake_prev_reason & (1 << i))) {
				/* Event turned on: log time for later computation */
				ps->pmwake_start_time[i] = curr_time;
			} else if (!(wake_mask & (1 << i)) &&
				(ps->pmwake_prev_reason & (1 << i))) {
				/* Event turned off: counts towards duration for event */
				if (curr_time > ps->pmwake_start_time[i])
					ps->pmd_event_wake_dur[i] +=
						(curr_time - ps->pmwake_start_time[i]);
				else
					ps->pmd_event_wake_dur[i] +=
						(ps->pmwake_start_time[i] - curr_time);

				ps->pmwake_start_time[i] = 0;
			}
		}
	}

	ps->pmwake_prev_reason = wake_mask;
}

void
wlc_pwrstats_copy_event_wake_dur(void *buf, void *pwrstats)
{
	wlc_pwrstats_info_t *ps = (wlc_pwrstats_info_t *)pwrstats;
	memcpy(buf, ps->pmd_event_wake_dur, sizeof(uint32) * WLC_PMD_EVENT_MAX);
}

/* This is invoked at the end of connection. This function updates
 * the cumulative connect time and returns the last connect time.
 */
uint32
wlc_pwrstats_connect_time_upd(void *pwrstats)
{
	wlc_pwrstats_info_t *ps = (wlc_pwrstats_info_t *)pwrstats;
	uint32 connect_dur = 0;
	wlc_conn_stats_t *cs = ps->cs;

	if (cs->connect_in_progress == TRUE) {
		cs->connect_in_progress = FALSE;
		cs->connect_end = OSL_SYSUPTIME();
		connect_dur = cs->connect_end - cs->connect_start;
		cs->connect_dur += connect_dur;
		cs->connect_count++;
	}
	return connect_dur;
}

uint32
wlc_pwrstats_curr_connect_time(void *pwrstats)
{
	wlc_pwrstats_info_t *ps = (wlc_pwrstats_info_t *)pwrstats;
	wlc_conn_stats_t *cs = ps->cs;
	uint32 curr_connect_time = 0;

	/* Calculate awake time due to active connection */
	if (cs->connect_in_progress == TRUE) {
		curr_connect_time = OSL_SYSUPTIME() - cs->connect_start;
	}
	return curr_connect_time;
}

void
wlc_pwrstats_connect_start(void *pwrstats)
{
	wlc_pwrstats_info_t *ps = (wlc_pwrstats_info_t *)pwrstats;
	wlc_conn_stats_t *cs = ps->cs;
	if (cs->connect_in_progress == FALSE) {
		cs->connect_start = OSL_SYSUPTIME();
		cs->connect_in_progress = TRUE;
	}
}

void
wlc_pwrstats_set_frts_data(wlc_pwrstats_info_t *ps, bool isdata)
{
	ps->frts_data = isdata;
}
uint32
wlc_pwrstats_get_frts_data_dur(wlc_pwrstats_info_t *ps)
{
	return ps->frts_data_dur;
}
