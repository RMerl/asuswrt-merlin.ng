/*
 * BT Coex module
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
 * $Id: wlc_btcx.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [BTCoexistenceHardware] [SoftwareApplicationNotes] [UcodeBTCoExistence]
 */

#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <sbchipc.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <bcmwifi_channels.h>
#include <bcmdevs.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_scb.h>
#include <wlc_btcx.h>
#include <wlc_scan.h>
#include <wlc_assoc.h>
#include <wlc_bmac.h>
#include <wlc_ap.h>
#include <wlc_stf.h>
#include <wlc_ampdu.h>
#include <wlc_ampdu_rx.h>
#include <wlc_ampdu_cmn.h>
#ifdef WLMCNX
#include <wlc_mcnx.h>
#endif // endif
#include <wlc_hw_priv.h>
#ifdef BCMLTECOEX
#include <wlc_ltecx.h>
#endif /* BCMLTECOEX */
#ifdef WLRSDB
#include <wlc_rsdb.h>
#endif /* WLRSDB */

static int wlc_btc_mode_set(wlc_info_t *wlc, int int_val);
static int wlc_btc_wire_set(wlc_info_t *wlc, int int_val);
static int wlc_btc_flags_idx_set(wlc_info_t *wlc, int int_val, int int_val2);
static int wlc_btc_flags_idx_get(wlc_info_t *wlc, int int_val);
static int wlc_btc_params_set(wlc_info_t *wlc, int int_val, int int_val2);
static int wlc_btc_params_get(wlc_info_t *wlc, int int_val);
static void wlc_btc_stuck_war50943(wlc_info_t *wlc, bool enable);
static void wlc_btc_rssi_threshold_get(wlc_info_t *wlc);
static int16 wlc_btc_siso_ack_get(wlc_info_t *wlc);
#ifdef WLC_LOW
static uint16 wlc_btc_sisoack_read_shm(wlc_info_t *wlc);
static void wlc_btc_sisoack_write_shm(wlc_info_t *wlc, uint16 sisoack);
#endif // endif
int wlc_btcx_desense(wlc_btc_info_t *btc, int band);
static bool wlc_btc_check_btrssi(wlc_info_t *wlc);

#if defined(STA) && defined(BTCX_PM0_IDLE_WAR)
static void wlc_btc_pm_adjust(wlc_info_t *wlc,  bool bt_active);
#endif // endif
static int wlc_btc_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
static void wlc_btcx_watchdog(void *arg);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_dump_btcx(wlc_info_t *wlc, struct bcmstrbuf *b);
static int wlc_clr_btcxdump(wlc_info_t *wlc);
#endif /* BCMDBG || BCMDBG_DUMP */

enum {
	IOV_BTC_MODE,           /* BT Coexistence mode */
	IOV_BTC_WIRE,           /* BTC number of wires */
	IOV_BTC_STUCK_WAR,       /* BTC stuck WAR */
	IOV_BTC_FLAGS,          /* BT Coex ucode flags */
	IOV_BTC_PARAMS,         /* BT Coex shared memory parameters */
	IOV_BTCX_CLEAR_DUMP,    /* clear btcx stats */
	IOV_BTC_SISO_ACK,       /* Specify SISO ACK antenna, disabled when 0 */
	IOV_BTC_RXGAIN_THRESH,  /* Specify restage rxgain thresholds */
	IOV_BTC_RXGAIN_FORCE,   /* Force for BTC restage rxgain */
	IOV_BTC_RXGAIN_LEVEL,   /* Set the BTC restage rxgain level */
};

const bcm_iovar_t btc_iovars[] = {
	{"btc_mode", IOV_BTC_MODE, 0, IOVT_UINT32, 0},
	{"btc_stuck_war", IOV_BTC_STUCK_WAR, 0, IOVT_BOOL, 0 },
	{"btc_flags", IOV_BTC_FLAGS, (IOVF_SET_UP | IOVF_GET_UP), IOVT_BUFFER, 0 },
	{"btc_params", IOV_BTC_PARAMS, (IOVF_SET_UP | IOVF_GET_UP), IOVT_BUFFER, 0 },
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	{"btcx_clear_dump", IOV_BTCX_CLEAR_DUMP, (IOVF_SET_CLK), IOVT_VOID, 0 },
#endif // endif
	{"btc_siso_ack", IOV_BTC_SISO_ACK, 0, IOVT_INT16, 0
	},
	{"btc_rxgain_thresh", IOV_BTC_RXGAIN_THRESH, 0, IOVT_UINT32, 0
	},
	{"btc_rxgain_force", IOV_BTC_RXGAIN_FORCE, 0, IOVT_UINT32, 0
	},
	{"btc_rxgain_level", IOV_BTC_RXGAIN_LEVEL, 0, IOVT_UINT32, 0
	},
	{NULL, 0, 0, 0, 0}
};

/* BTC stuff */

struct wlc_btc_info {
	wlc_info_t *wlc;
	uint16  bth_period;             /* bt coex period. read from shm. */
	bool    bth_active;             /* bt active session */
	uint8   prot_rssi_thresh;       /* rssi threshold for forcing protection */
	uint8   ampdutx_rssi_thresh;    /* rssi threshold to turn off ampdutx */
	uint8   ampdurx_rssi_thresh;    /* rssi threshold to turn off ampdurx */
	uint8   high_threshold;         /* threshold to switch to btc_mode 4 */
	uint8   low_threshold;          /* threshold to switch to btc_mode 1 */
	uint8   host_requested_pm;      /* saved pm state specified by host */
	uint8   mode_overridden;        /* override btc_mode for long range */
	/* cached value for btc in high driver to avoid frequent RPC calls */
	int     mode;
	int     wire;
	int16   siso_ack;               /* txcoremask for siso ack (e.g., 1: use core 1 for ack) */
	int     restage_rxgain_level;
	int     restage_rxgain_force;
	int     restage_rxgain_active;
	uint8   restage_rxgain_on_rssi_thresh;  /* rssi threshold to turn on rxgain restaging */
	uint8   restage_rxgain_off_rssi_thresh; /* rssi threshold to turn off rxgain restaging */
	uint16	agg_off_bm;
	bool    siso_ack_ovr;           /* siso_ack set 0: automatically 1: by iovar */
#ifdef WLBTCPROF
	wlc_btc_prev_connect_t *btc_prev_connect; /* btc previous connection info */
	wlc_btc_select_profile_t *btc_profile; /* User selected profile for 2G and 5G params */
#endif /* WLBTCPROF */

	uint8 btrssi[BTC_BTRSSI_SIZE]; /* actual rssi = -1 x (btrssi + 10) e.g., 60 -> -70dBm */
	uint8 btrssi_cnt;              /* number of btrssi samples */
	uint8 btrssi_ptr;              /* pointer for moving average */
};

/* Read bt rssi from shm and do moving average. Return true if bt rssi < thresh. */
static bool wlc_btc_check_btrssi(wlc_info_t *wlc)
{
	wlc_btc_info_t *btc = wlc->btch;
	uint16 btcx_blk_ptr, btrssi, btrssi_avg = 0;
	uint8 i;

	if (!btc->bth_active)
		return FALSE;

	btcx_blk_ptr = 2 * wlc_bmac_read_shm(wlc->hw, M_BTCX_BLK_PTR);
	/* read btrssi idx from shm */
	btrssi = wlc_bmac_read_shm(wlc->hw, btcx_blk_ptr + M_BTCX_RSSI);
	/* actual bt rssi = -1 x (btrssi x 5 + 10) */

	if (btrssi) {
		/* clear shm because ucode keeps max btrssi idx */
		wlc_bmac_write_shm(wlc->hw, btcx_blk_ptr + M_BTCX_RSSI, 0);
		btrssi = btrssi * 5;
		btc->btrssi [btc->btrssi_ptr++] = (uint8)btrssi;

		if (btc->btrssi_ptr == BTC_BTRSSI_SIZE)
			btc->btrssi_ptr = 0;
		if (btc->btrssi_cnt < BTC_BTRSSI_SIZE)
			btc->btrssi_cnt++;
	}

	if (btc->btrssi_cnt >= BTC_BTRSSI_MIN_NUM) {
		for (i = 0; i < BTC_BTRSSI_SIZE; i++)
			btrssi_avg += btc->btrssi [i];
		btrssi_avg = btrssi_avg / btc->btrssi_cnt;
		if (btrssi_avg > BTC_BTRSSI_THRESH)
			return TRUE; /* protection (e.g., tdm) needed */
	}

	return FALSE;
}

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

wlc_btc_info_t *
BCMATTACHFN(wlc_btc_attach)(wlc_info_t *wlc)
{
	wlc_btc_info_t *btc;

	if ((btc = (wlc_btc_info_t*)
		MALLOCZ(wlc->osh, sizeof(wlc_btc_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	btc->wlc = wlc;

	/* register module */
	if (wlc_module_register(wlc->pub, btc_iovars, "btc", btc, wlc_btc_doiovar,
		wlc_btcx_watchdog, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* register dump stats for btcx */
	wlc_dump_register(wlc->pub, "btcx", (dump_fn_t)wlc_dump_btcx, (void *)wlc);
#endif /* BCMDBG || BCMDBG_DUMP */

#ifdef WLBTCPROF
	if ((btc->btc_prev_connect = (wlc_btc_prev_connect_t *)
		MALLOCZ(wlc->osh, sizeof(wlc_btc_prev_connect_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	if ((btc->btc_profile = (wlc_btc_select_profile_t *)
		MALLOCZ(wlc->osh, sizeof(wlc_btc_select_profile_t) * BTC_SUPPORT_BANDS)) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	bzero(btc->btc_prev_connect, sizeof(wlc_btc_prev_connect_t));
	bzero(btc->btc_profile, sizeof(wlc_btc_profile_t) * BTC_SUPPORT_BANDS);

	memset(&btc->btc_prev_connect->prev_2G_profile, 0, sizeof(struct wlc_btc_profile));
	memset(&btc->btc_prev_connect->prev_5G_profile, 0, sizeof(struct wlc_btc_profile));

	btc->btc_prev_connect->prev_band = WLC_BAND_ALL;
#endif /* WLBTCPROF */

	btc->siso_ack_ovr = FALSE;

	bzero(btc->btrssi, sizeof(uint8)*BTC_BTRSSI_SIZE);
	btc->btrssi_cnt = 0;
	btc->btrssi_ptr = 0;

	return btc;

	/* error handling */
fail:
	wlc_btc_detach(btc);
	return NULL;
}

void
BCMATTACHFN(wlc_btc_detach)(wlc_btc_info_t *btc)
{
	wlc_info_t *wlc;

	if (btc == NULL)
		return;

	wlc = btc->wlc;
	wlc_module_unregister(wlc->pub, "btc", btc);

#ifdef WLBTCPROF
	MFREE(wlc->osh, btc->btc_prev_connect, sizeof(wlc_btc_prev_connect_t));
	MFREE(wlc->osh, btc->btc_profile, sizeof(wlc_btc_select_profile_t));
#endif /* WLBTCPROF */
	MFREE(wlc->osh, btc, sizeof(wlc_btc_info_t));
}

static int
wlc_btc_mode_set(wlc_info_t *wlc, int int_val)
{
	int err = wlc_bmac_btc_mode_set(wlc->hw, int_val);
	wlc->btch->mode = wlc_bmac_btc_mode_get(wlc->hw);
	return err;
}

int
wlc_btc_mode_get(wlc_info_t *wlc)
{
	return wlc->btch->mode;
}

int
wlc_btcx_desense(wlc_btc_info_t *btc, int band)
{
	int i;
	wlc_info_t *wlc = (wlc_info_t *)btc->wlc;
	int btc_mode = wlc_btc_mode_get(wlc);

	/* Dynamic restaging of rxgain for BTCoex */
	if (!SCAN_IN_PROGRESS(wlc->scan) &&
	    btc->bth_active &&
	    (wlc->cfg->link->rssi != WLC_RSSI_INVALID)) {
		if (!btc->restage_rxgain_active &&
		    ((BAND_5G(band) &&
		      ((btc->restage_rxgain_force &
			BTC_RXGAIN_FORCE_5G_MASK) == BTC_RXGAIN_FORCE_5G_ON)) ||
		     (BAND_2G(band) &&
		      ((btc->restage_rxgain_force &
			BTC_RXGAIN_FORCE_2G_MASK) == BTC_RXGAIN_FORCE_2G_ON) &&
		      (!btc->restage_rxgain_on_rssi_thresh ||
		       (btc_mode == WL_BTC_DISABLE) ||
		       (btc->restage_rxgain_on_rssi_thresh &&
		       ((btc_mode == WL_BTC_HYBRID) || (btc_mode == WL_BTC_FULLTDM)) &&
			(-wlc->cfg->link->rssi < btc->restage_rxgain_on_rssi_thresh)))))) {
			if ((i = wlc_iovar_setint(wlc, "phy_btc_restage_rxgain",
				btc->restage_rxgain_level)) == BCME_OK) {
				btc->restage_rxgain_active = 1;
			}
			WL_BTCPROF(("wl%d: BTC restage rxgain (%x) ON: RSSI %d "
				"Thresh -%d, bt %d, (err %d)\n",
				wlc->pub->unit, wlc->stf->rxchain, wlc->cfg->link->rssi,
				btc->restage_rxgain_on_rssi_thresh,
				btc->bth_active, i));
		}
		else if (btc->restage_rxgain_active &&
			((BAND_5G(band) &&
			((btc->restage_rxgain_force &
			BTC_RXGAIN_FORCE_5G_MASK) == BTC_RXGAIN_FORCE_OFF)) ||
			(BAND_2G(band) &&
			(((btc->restage_rxgain_force &
			BTC_RXGAIN_FORCE_2G_MASK) == BTC_RXGAIN_FORCE_OFF) ||
			(btc->restage_rxgain_off_rssi_thresh &&
			((btc_mode == WL_BTC_HYBRID) || (btc_mode == WL_BTC_FULLTDM)) &&
			(-wlc->cfg->link->rssi > btc->restage_rxgain_off_rssi_thresh)))))) {
			  if ((i = wlc_iovar_setint(wlc, "phy_btc_restage_rxgain", 0)) == BCME_OK) {
				btc->restage_rxgain_active = 0;
			  }
			  WL_BTCPROF(("wl%d: BTC restage rxgain (%x) OFF: RSSI %d "
				"Thresh -%d, bt %d, (err %d)\n",
				wlc->pub->unit, wlc->stf->rxchain, wlc->cfg->link->rssi,
				btc->restage_rxgain_off_rssi_thresh,
				btc->bth_active, i));
		}
	} else if (btc->restage_rxgain_active) {
		if ((i = wlc_iovar_setint(wlc, "phy_btc_restage_rxgain", 0)) == BCME_OK) {
			btc->restage_rxgain_active = 0;
		}
		WL_BTCPROF(("wl%d: BTC restage rxgain (%x) OFF: RSSI %d bt %d (err %d)\n",
			wlc->pub->unit, wlc->stf->rxchain, wlc->cfg->link->rssi,
			btc->bth_active, i));
	}

	return BCME_OK;
}

#ifdef WLBTCPROF
int
wlc_btcx_set_btc_profile_param(struct wlc_info *wlc, chanspec_t chanspec, bool force)
{
	int err = BCME_OK;
	wlc_btc_profile_t *btc_cur_profile, *btc_prev_profile;
	wlc_btc_info_t *btch = wlc->btch;
	int btc_inactive_offset[WL_NUM_TXCHAIN_MAX] = {0, 0, 0, 0};

	int band = CHSPEC_IS2G(chanspec) ? WLC_BAND_2G : WLC_BAND_5G;

	if (!btch)
	{
		WL_INFORM(("%s invalid btch\n", __FUNCTION__));
		err = BCME_ERROR;
		goto finish;
	}

	if (!btch->btc_prev_connect)
	{
		WL_INFORM(("%s invalid btc_prev_connect\n", __FUNCTION__));
		err = BCME_ERROR;
		goto finish;
	}

	if (!btch->btc_profile)
	{
		WL_INFORM(("%s invalid btc_profile\n", __FUNCTION__));
		err = BCME_ERROR;
		goto finish;
	}

	if (band == WLC_BAND_2G)
	{
		if (btch->btc_profile[BTC_PROFILE_2G].enable == BTC_PROFILE_OFF)
			goto finish;

		if (!force && btch->btc_prev_connect->prev_band == WLC_BAND_2G)
			goto finish;

		if ((btch->btc_prev_connect->prev_2G_mode == BTC_PROFILE_DISABLE) &&
			(btch->btc_profile[BTC_PROFILE_2G].enable == BTC_PROFILE_DISABLE))
			goto finish;

		if (btch->btc_profile[BTC_PROFILE_2G].enable == BTC_PROFILE_DISABLE)
		{
			btch->btc_prev_connect->prev_2G_mode = BTC_PROFILE_DISABLE;
			memset(&btch->btc_profile[BTC_PROFILE_2G].select_profile,
				0, sizeof(wlc_btc_profile_t));
		}
		else
		{
			btch->btc_prev_connect->prev_2G_mode = BTC_PROFILE_ENABLE;
		}

		btch->btc_prev_connect->prev_band = WLC_BAND_2G;
		btc_cur_profile = &btch->btc_profile[BTC_PROFILE_2G].select_profile;
		btc_prev_profile = &btch->btc_prev_connect->prev_2G_profile;
	}
	else
	{
		if (btch->btc_profile[BTC_PROFILE_5G].enable == BTC_PROFILE_OFF)
			goto finish;

		if (!force && btch->btc_prev_connect->prev_band == WLC_BAND_5G)
			goto finish;

		if ((btch->btc_prev_connect->prev_5G_mode == BTC_PROFILE_DISABLE) &&
			(btch->btc_profile[BTC_PROFILE_5G].enable == BTC_PROFILE_DISABLE))
			goto finish;

		if (btch->btc_profile[BTC_PROFILE_5G].enable == BTC_PROFILE_DISABLE)
		{
			btch->btc_prev_connect->prev_5G_mode = BTC_PROFILE_DISABLE;
			memset(&btch->btc_profile[BTC_PROFILE_5G].select_profile,
				0, sizeof(wlc_btc_profile_t));
		}
		else
		{
			btch->btc_prev_connect->prev_5G_mode = BTC_PROFILE_ENABLE;
		}

		btch->btc_prev_connect->prev_band = WLC_BAND_5G;
		btc_cur_profile = &btch->btc_profile[BTC_PROFILE_5G].select_profile;
		btc_prev_profile = &btch->btc_prev_connect->prev_5G_profile;
	}

	WL_BTCPROF(("%s chanspec 0x%x\n", __FUNCTION__, chanspec));

	/* setBTCOEX_MODE */
	err = wlc_btc_mode_set(wlc, btc_cur_profile->mode);
	WL_BTCPROF(("btc mode %d\n", btc_cur_profile->mode));
	if (err)
	{
		err = BCME_ERROR;
		goto finish;
	}

	/* setDESENSE_LEVEL */
	btch->restage_rxgain_level = btc_cur_profile->desense_level;
	WL_BTCPROF(("btc desense level %d\n", btc_cur_profile->desense_level));

	/* setDESENSE */
	btch->restage_rxgain_force =
		(btch->btc_profile[BTC_PROFILE_2G].select_profile.desense)?
		BTC_RXGAIN_FORCE_2G_ON : 0;
	btch->restage_rxgain_force |=
		(btch->btc_profile[BTC_PROFILE_5G].select_profile.desense)?
		BTC_RXGAIN_FORCE_5G_ON : 0;
	WL_BTCPROF(("btc rxgain force 0x%x\n", btch->restage_rxgain_force));

	/* setting 2G thresholds, 5G thresholds are not used */
	btch->restage_rxgain_on_rssi_thresh =
		(uint8)((btch->btc_profile[BTC_PROFILE_2G].select_profile.desense_thresh_high *
		-1) & 0xFF);
	btch->restage_rxgain_off_rssi_thresh =
		(uint8)((btch->btc_profile[BTC_PROFILE_2G].select_profile.desense_thresh_low *
		-1) & 0xFF);
	WL_BTCPROF(("btc rxgain on 0x%x rxgain off 0x%x\n",
		btch->restage_rxgain_on_rssi_thresh,
		btch->restage_rxgain_off_rssi_thresh));

	/* check the state of bt_active signal */
		if (BT3P_HW_COEX(wlc) && wlc->clk) {
			wlc_bmac_btc_period_get(wlc->hw, &btch->bth_period,
				&btch->bth_active, &btch->agg_off_bm);
		}

	/* apply desense settings */
	wlc_btcx_desense(btch, band);

	/* setChain_Ack */
	wlc_btc_siso_ack_set(wlc, (int16)btc_cur_profile->chain_ack[0], TRUE);
	WL_BTCPROF(("btc chain ack 0x%x num chains %d\n",
		btc_cur_profile->chain_ack[0],
		btc_cur_profile->num_chains));

	/* setTX_CHAIN_POWER */
	if (btch->bth_active) {
	wlc_channel_set_tx_power(wlc, band, btc_cur_profile->num_chains,
		&btc_cur_profile->chain_power_offset[0],
		&btc_prev_profile->chain_power_offset[0]);
	*btc_prev_profile = *btc_cur_profile;
	} else {
		wlc_channel_set_tx_power(wlc, band, btc_cur_profile->num_chains,
			&btc_inactive_offset[0], &btc_prev_profile->chain_power_offset[0]);
	}

finish:
	return err;
}

int
wlc_btcx_select_profile_set(wlc_info_t *wlc, uint8 *pref, int len)
{
	wlc_btc_info_t *btch;

	btch = wlc->btch;
	if (!btch)
	{
		WL_INFORM(("%s invalid btch\n", __FUNCTION__));
		return BCME_ERROR;
	}

	if (pref)
	{
		if (!bcmp(pref, btch->btc_profile, len))
			return BCME_OK;

		bcopy(pref, btch->btc_profile, len);

		if (wlc_btcx_set_btc_profile_param(wlc, wlc->chanspec, TRUE))
		{
			WL_ERROR(("wl%d: %s: setting btc profile first time error: chspec %d!\n",
				wlc->pub->unit, __FUNCTION__, CHSPEC_CHANNEL(wlc->chanspec)));
		}

		return BCME_OK;
	}

	return BCME_ERROR;
}

int
wlc_btcx_select_profile_get(wlc_info_t *wlc, uint8 *pref, int len)
{
	wlc_btc_info_t *btch = wlc->btch;

	if (!btch)
	{
		WL_INFORM(("%s invalid btch\n", __FUNCTION__));
		return BCME_ERROR;
	}

	if (pref)
	{
		bcopy(btch->btc_profile, pref, len);
		return BCME_OK;
	}

	return BCME_ERROR;
}
#endif /* WLBTCPROF */

int
wlc_btc_siso_ack_set(wlc_info_t *wlc, int16 siso_ack, bool force)
{
#ifdef WLC_LOW
	wlc_btc_info_t *btch = wlc->btch;
	uint16	sisoack_shm;
	uint16	sisoack_txpwr = 0;
	int		btparam_txpwr = 0;
	bool	sisoack_flag = FALSE;

	if (!btch)
		return BCME_ERROR;

	if (force) {
		if (siso_ack == AUTO)
			btch->siso_ack_ovr = FALSE;
		else {
			/* sanity check forced value */
			if (!(siso_ack & TXCOREMASK))
				return BCME_BADARG;
			btch->siso_ack = siso_ack;
			btch->siso_ack_ovr = TRUE;
		}
	}

	if (!btch->siso_ack_ovr) {
		/* no override, set siso_ack according to btc_mode/chipids/boardflag etc. */
		if (siso_ack == AUTO) {
			siso_ack = 0;
			if (wlc->pub->boardflags & BFL_FEM_BT) {
				/* check boardflag: antenna shared w BT */
				/* futher check srom, nvram */
				if (wlc->hw->btc->btcx_aa == 0x3) { /* two antenna */
					if (wlc->pub->boardflags2 &
					    BFL2_BT_SHARE_ANT0) { /* core 0 shared */
						siso_ack = 0x2;
					} else {
						siso_ack = 0x1;
					}
				} else if (wlc->hw->btc->btcx_aa == 0x7) { /* three antenna */
					; /* not supported yet */
				}
			}
		} else { /* For discrete chips this BFL_FEM_BT flag may not be set */
			if (btch->mode == WL_BTC_HYBRID)
				siso_ack = TXCORE0_MASK;
			else
				siso_ack = AUTO;
		}
		btch->siso_ack = siso_ack;
	}
	btparam_txpwr = wlc_btc_params_get(wlc, BTC_PARAMS_FW_START_IDX + BTC_FW_SISO_ACK_TX_PWR);
	if (btparam_txpwr < 255) {
		sisoack_txpwr = btparam_txpwr << 8;
		sisoack_flag = TRUE;
	}

	/* update siso_ack shm */
	sisoack_shm = wlc_btc_sisoack_read_shm(wlc);
	if (sisoack_flag) {
		sisoack_shm = (sisoack_txpwr & BTC_SISOACK_TXPWR_MASK);
	} else {
		sisoack_shm &= BTC_SISOACK_TXPWR_MASK; /* txpwr offset set by phy */
	}
	sisoack_shm = sisoack_shm | (btch->siso_ack & BTC_SISOACK_CORES_MASK);

	wlc_btc_sisoack_write_shm(wlc, sisoack_shm);
#endif /* WLC_LOW */
	return BCME_OK;
}

int16
wlc_btc_siso_ack_get(wlc_info_t *wlc)
{
	return wlc->btch->siso_ack;
}
#ifdef WLC_LOW
static uint16
wlc_btc_sisoack_read_shm(wlc_info_t *wlc)
{
	uint16 sisoack = 0;

	if (wlc->clk) {
		if (D11REV_GE(wlc->pub->corerev, 40)) {
			sisoack = wlc_bmac_read_shm(wlc->hw, M_COREMASK_BTRESP);
		} else {
			sisoack = wlc_bmac_read_shm(wlc->hw, M_COREMASK_BTRESP_PRE40);
		}
	}

	return sisoack;
}

static void
wlc_btc_sisoack_write_shm(wlc_info_t *wlc, uint16 sisoack)
{
	if (wlc->clk) {
		if (D11REV_GE(wlc->pub->corerev, 40)) {
			wlc_bmac_write_shm(wlc->hw, M_COREMASK_BTRESP, sisoack);
		} else {
			wlc_bmac_write_shm(wlc->hw, M_COREMASK_BTRESP_PRE40, sisoack);
		}
	}
}
#endif /* WLC_LOW */
static int
wlc_btc_wire_set(wlc_info_t *wlc, int int_val)
{
	int err;
	err = wlc_bmac_btc_wire_set(wlc->hw, int_val);
	wlc->btch->wire = wlc_bmac_btc_wire_get(wlc->hw);
	return err;
}

int
wlc_btc_wire_get(wlc_info_t *wlc)
{
	return wlc->btch->wire;
}

void wlc_btc_mode_sync(wlc_info_t *wlc)
{
	wlc->btch->mode = wlc_bmac_btc_mode_get(wlc->hw);
	wlc->btch->wire = wlc_bmac_btc_wire_get(wlc->hw);
}

uint8 wlc_btc_save_host_requested_pm(wlc_info_t *wlc, uint8 val)
{
	return (wlc->btch->host_requested_pm = val);
}

bool wlc_btc_get_bth_active(wlc_info_t *wlc)
{
	return wlc->btch->bth_active;
}

uint16 wlc_btc_get_bth_period(wlc_info_t *wlc)
{
	return wlc->btch->bth_period;
}

static int
wlc_btc_flags_idx_set(wlc_info_t *wlc, int int_val, int int_val2)
{
	return wlc_bmac_btc_flags_idx_set(wlc->hw, int_val, int_val2);
}

static int
wlc_btc_flags_idx_get(wlc_info_t *wlc, int int_val)
{
	return wlc_bmac_btc_flags_idx_get(wlc->hw, int_val);
}

static int
wlc_btc_params_set(wlc_info_t *wlc, int int_val, int int_val2)
{
	return wlc_bmac_btc_params_set(wlc->hw, int_val, int_val2);
}

static int
wlc_btc_params_get(wlc_info_t *wlc, int int_val)
{
	return wlc_bmac_btc_params_get(wlc->hw, int_val);
}

static void
wlc_btc_stuck_war50943(wlc_info_t *wlc, bool enable)
{
	wlc_bmac_btc_stuck_war50943(wlc->hw, enable);
}

static void
wlc_btc_rssi_threshold_get(wlc_info_t *wlc)
{
	wlc_bmac_btc_rssi_threshold_get(wlc->hw,
		&wlc->btch->prot_rssi_thresh,
		&wlc->btch->high_threshold,
		&wlc->btch->low_threshold);
}

void  wlc_btc_4313_gpioctrl_init(wlc_info_t *wlc)
{
	if (CHIPID(wlc->pub->sih->chip) == BCM4313_CHIP_ID) {
	/* ePA 4313 brds */
		if (wlc->pub->boardflags & BFL_FEM) {
			if (wlc->pub->boardrev >= 0x1250 && (wlc->pub->boardflags & BFL_FEM_BT)) {
				wlc_mhf(wlc, MHF5, MHF5_4313_BTCX_GPIOCTRL, MHF5_4313_BTCX_GPIOCTRL,
					WLC_BAND_ALL);
			} else
				wlc_mhf(wlc, MHF4, MHF4_EXTPA_ENABLE,
				MHF4_EXTPA_ENABLE, WLC_BAND_ALL);
			/* iPA 4313 brds */
			} else {
				if (wlc->pub->boardflags & BFL_FEM_BT)
					wlc_mhf(wlc, MHF5, MHF5_4313_BTCX_GPIOCTRL,
						MHF5_4313_BTCX_GPIOCTRL, WLC_BAND_ALL);
			}
	}
}

uint
wlc_btc_frag_threshold(wlc_info_t *wlc, struct scb *scb)
{
	ratespec_t rspec;
	uint rate, thresh;
	wlc_bsscfg_t *cfg;

	/* Make sure period is known */
	if (wlc->btch->bth_period == 0)
		return 0;

	ASSERT(scb != NULL);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	/* if BT SCO is ongoing, packet length should not exceed 1/2 of SCO period */
	rspec = wlc_get_rspec_history(cfg);
	rate = RSPEC2KBPS(rspec);

	/*  use one half of the duration as threshold.  convert from usec to bytes */
	/* thresh = (bt_period * rate) / 1000 / 8 / 2  */
	thresh = (wlc->btch->bth_period * rate) >> 14;

	if (thresh < DOT11_MIN_FRAG_LEN)
		thresh = DOT11_MIN_FRAG_LEN;
	return thresh;
}

#if defined(STA) && defined(BTCX_PM0_IDLE_WAR)
static void
wlc_btc_pm_adjust(wlc_info_t *wlc,  bool bt_active)
{
	wlc_bsscfg_t *cfg = wlc->cfg;
	/* only bt is not active, set PM to host requested mode */
	if (wlc->btch->host_requested_pm != PM_FORCE_OFF) {
		if (bt_active) {
				if (PM_OFF == wlc->btch->host_requested_pm &&
				cfg->pm->PM != PM_FAST)
				wlc_set_pm_mode(wlc, PM_FAST, cfg);
		} else {
			if (wlc->btch->host_requested_pm != cfg->pm->PM)
				wlc_set_pm_mode(wlc, wlc->btch->host_requested_pm, cfg);
		}
	}
}
#endif /* STA */

/* XXX FIXME
 * BTCX settings are now global but we may later on need to change it for multiple BSS
 * hence pass the bsscfg as a parm.
 */
void
wlc_enable_btc_ps_protection(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bool protect)
{
	if (MCHAN_ACTIVE(wlc->pub))
		return;

	if ((wlc_btc_wire_get(wlc) >= WL_BTC_3WIRE) &&
		wlc_btc_mode_get(wlc)) {
		wlc_bsscfg_t *pc;
		int btc_flags = wlc_bmac_btc_flags_get(wlc->hw);
		uint16 protections;
		uint16 active = 0;
		uint16 ps;

		pc = wlc->cfg;
		BCM_REFERENCE(pc);

		/* if radio is disable, driver may be down, quit here */
		if (wlc->pub->radio_disabled || !wlc->pub->up)
			return;

#if defined(STA) && !defined(BCMNODOWN)
		/* ??? if ismpc, driver should be in down state if up/down is allowed */
		if (wlc->mpc && wlc_ismpc(wlc))
			return;
#endif // endif
		wlc_btc_rssi_threshold_get(wlc);

		/* enable protection for hybrid mode when rssi below certain threshold */
		if ((wlc->btch->prot_rssi_thresh &&
			-wlc->cfg->link->rssi > wlc->btch->prot_rssi_thresh) ||
			(btc_flags & WL_BTC_FLAG_ACTIVE_PROT))
		{
			active = MHF3_BTCX_ACTIVE_PROT;
		}

		/* enable protection if bt rssi < threshold */
		if (D11REV_IS(wlc->pub->corerev, 48) && wlc_btc_check_btrssi(wlc)) {
			active = MHF3_BTCX_ACTIVE_PROT;
		}

		ps = (btc_flags & WL_BTC_FLAG_PS_PROTECT) ? MHF3_BTCX_PS_PROTECT : 0;
		BCM_REFERENCE(ps);

#ifdef STA
		/* Enable PS protection when the primary bsscfg is associated as
		 * an infra STA and is the only connection
		 */
		if (BSSCFG_STA(pc) && pc->current_bss->infra &&
		    WLC_BSS_CONNECTED(pc) && wlc->stas_connected == 1 &&
		    (wlc->aps_associated == 0 || wlc_ap_stas_associated(wlc->ap) == 0)) {
			/* when WPA/PSK security is enabled wait until WLC_PORTOPEN() is TRUE */
			if (pc->WPA_auth == WPA_AUTH_DISABLED || !WSEC_ENABLED(pc->wsec) ||
			    WLC_PORTOPEN(pc))
				protections = active | ps;
			/* XXX temporarily disable protections in between
			 * association is done and key is plumbed???
			 */
			else
				protections = 0;
		}
		/*
		Enable PS protection if there is only one BSS
		associated as STA and there are no APs. All else
		enable CTS2SELF.
		*/
		else if (wlc->stas_connected > 0)
		{
			if ((wlc->stas_connected == 1) && (wlc->aps_associated == 0))
				protections = active | ps;
			else
				protections = active;
		}
		/* Enable CTS-to-self protection when AP(s) are up and there are
		 * STAs associated
		 */
		else
#endif /* STA */
#ifdef AP
		if (wlc->aps_associated > 0 && wlc_ap_stas_associated(wlc->ap) > 0)
			protections = active;
		/* No protection */
		else
#endif /* AP */
			protections = 0;

		wlc_mhf(wlc, MHF3, MHF3_BTCX_ACTIVE_PROT | MHF3_BTCX_PS_PROTECT,
		        protections, WLC_BAND_2G);

		if (BCM4365_CHIP(wlc->pub->sih->chip)) {
			if (protections & MHF3_BTCX_ACTIVE_PROT) {
				wlc_btc_siso_ack_set(wlc, 0, FALSE);
			} else {
				wlc_btc_siso_ack_set(wlc, AUTO, FALSE);
			}
		}
#ifdef WLMCNX
		/*
		For non-VSDB the only time we turn on PS protection is when there is only
		one STA associated - primary or GC. In this case, set the BSS index in
		designated SHM location as well.
		*/
		if ((MCNX_ENAB(wlc->pub)) && (protections & ps)) {
			uint idx;
			wlc_bsscfg_t *cfg;
			int bss_idx;

			FOREACH_AS_STA(wlc, idx, cfg) {
				if (!cfg->BSS)
					continue;
				bss_idx = wlc_mcnx_BSS_idx(wlc->mcnx, cfg);
				wlc_mcnx_shm_bss_idx_set(wlc->mcnx, bss_idx);
				break;
			}
		}
#endif /* WLMCNX */
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_dump_btcx(wlc_info_t *wlc, struct bcmstrbuf *b)
{
	uint8 idx, offset;
	uint16 hi, lo;
	uint32 buff[C_BTCX_DBGBLK_SZ/2];
	uint16 base = D11REV_LT(wlc->pub->corerev, 40) ?
	M_BTCX_DBGBLK: ((wlc_bmac_read_shm(wlc->hw, M_UCODE_MACSTAT1_PTR) + 0xd) << 1);

	if (!wlc->clk) {
		return BCME_NOCLK;
	}

	for (idx = 0; idx < C_BTCX_DBGBLK_SZ; idx += 2) {
		offset = idx*2;
		lo = wlc_bmac_read_shm(wlc->hw, base+offset);
		hi = wlc_bmac_read_shm(wlc->hw, base+offset+2);
		buff[idx>>1] = (hi<<16) | lo;
	}

	bcm_bprintf(b, "nrfact: %u, ntxconf: %u (%u%%), txconf_durn(us): %u\n",
		buff[0], buff[1], buff[0] ? (buff[1]*100)/buff[0]: 0, buff[2]);
	return 0;
}

static int wlc_clr_btcxdump(wlc_info_t *wlc)
{
	uint16 base = D11REV_LT(wlc->pub->corerev, 40) ?
	M_BTCX_DBGBLK: ((wlc_bmac_read_shm(wlc->hw, M_UCODE_MACSTAT1_PTR) + 0xd) << 1);

	if (!wlc->clk) {
	return BCME_NOCLK;
	}

	wlc_bmac_set_shm(wlc->hw, base, 0, C_BTCX_DBGBLK_SZ*2);
	return 0;
}
#endif /* BCMDBG || BCMDBG_DUMP */

/* Read relevant BTC params to determine if aggregation has to be enabled/disabled */
void
wlc_btcx_read_btc_params(wlc_info_t *wlc)
{
	wlc_btc_info_t *btc = wlc->btch;
	if (BT3P_HW_COEX(wlc) && wlc->clk) {
		wlc_bmac_btc_period_get(wlc->hw, &btc->bth_period,
			&btc->bth_active, &btc->agg_off_bm);
		if (!btc->bth_active && btc->btrssi_cnt) {
			bzero(btc->btrssi, sizeof(uint8)*BTC_BTRSSI_SIZE);
			btc->btrssi_cnt = 0;
			btc->btrssi_ptr = 0;
		}
	}
}

#ifdef WLRSDB
void
wlc_btcx_update_coex_iomask(wlc_info_t *wlc)
{
#if defined(BCMECICOEX) && defined(WLC_LOW)

	wlc_hw_info_t *wlc_hw = wlc->hw;

	/* Should come here only for RSDB capable devices */
	ASSERT(wlc_bmac_rsdb_cap(wlc_hw));

	if (!RSDB_ENAB(wlc->pub) ||
		(wlc_rsdb_mode(wlc) == PHYMODE_MIMO) ||
		(wlc_rsdb_mode(wlc) == PHYMODE_80P80)) {

		/* In the MIMO/80p80 mode set coex_io_mask to 0x3 on core 1
		 * (i.e) mask both Txconf and Prisel on Core 1.
		 * Leave coex_io_mask on core 0 to its default value (0x0)
		 */
		if (!si_coreunit(wlc_hw->sih)) {
			d11regs_t *sregs;

			/* Unmask coex_io_mask on core0 to 0 */
			AND_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.coex_io_mask,
				~((1 << COEX_IOMASK_PRISEL_POS) | (1 << COEX_IOMASK_TXCONF_POS)));

			/* Set: core 1 */
			sregs = si_d11_switch_addrbase(wlc_hw->sih, 1);

			/* Enable MAC control IHR access on core 1 */
			OR_REG(wlc_hw->osh, &sregs->maccontrol, MCTL_IHR_EN);

			/* Mask Txconf and Prisel on core 1 */
			OR_REG(wlc_hw->osh, &sregs->u.d11regs.coex_io_mask,
				((1 << COEX_IOMASK_PRISEL_POS) | (1 << COEX_IOMASK_TXCONF_POS)));

			/* Disable MAC control IHR access on core 1 */
			/* OR_REG(wlc_hw->osh, &sregs->maccontrol, ~MCTL_IHR_EN); */

			/* Restore: core 0 */
			si_d11_switch_addrbase(wlc_hw->sih, 0);
		}
	} else {
		wlc_cmn_info_t* wlc_cmn = wlc->cmn;
		wlc_info_t *other_wlc;
		int idx;
		int coex_io_mask, coex_io_mask_ch;

		if (si_coreunit(wlc_hw->sih)) {
			/* Enable MAC control IHR access for core1 */
			OR_REG(wlc_hw->osh, &wlc_hw->regs->maccontrol, MCTL_IHR_EN);
		}
		/* read present core iomask */
		coex_io_mask = R_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.coex_io_mask);

		/* by default let's not mask txconf and prisel from this core */
		coex_io_mask_ch = coex_io_mask &
			~((1 << COEX_IOMASK_TXCONF_POS)
			| (1 << COEX_IOMASK_PRISEL_POS));
		FOREACH_WLC(wlc_cmn, idx, other_wlc) {
			if (wlc != other_wlc) {
				if (CHSPEC_IS5G(wlc_hw->chanspec) &&
					CHSPEC_IS2G(other_wlc->hw->chanspec)) {
					/* mask txconf and prisel from this core */
					coex_io_mask_ch = coex_io_mask |
						(1 << COEX_IOMASK_TXCONF_POS) |
						(1 << COEX_IOMASK_PRISEL_POS);
				}
			}
		}

		/* update coex_io_mask if there is a change */
		if (coex_io_mask_ch != coex_io_mask) {
			W_REG(wlc_hw->osh, &wlc_hw->regs->u.d11regs.coex_io_mask, coex_io_mask_ch);
		}
	}
#endif /* BCMECICOEX && WLC_LOW */
}
#endif /* WLRSDB */

static void
wlc_btcx_watchdog(void *arg)
{
	wlc_btc_info_t *btc = (wlc_btc_info_t *)arg;
	wlc_info_t *wlc = (wlc_info_t *)btc->wlc;
	int btc_mode = wlc_btc_mode_get(wlc);
	/* update critical BT state, only for 2G band */
	if (btc_mode && BAND_2G(wlc->band->bandtype)) {
		wlc_enable_btc_ps_protection(wlc, wlc->cfg, TRUE);
#if defined(STA) && defined(BTCX_PM0_IDLE_WAR)
		wlc_btc_pm_adjust(wlc, wlc->btch->bth_active);
#endif /* STA */
		/* rssi too low, give up */
		if (wlc->btch->low_threshold &&
			-wlc->cfg->link->rssi >
			wlc->btch->low_threshold) {
			if (!IS_BTCX_FULLTDM(btc_mode)) {
				wlc->btch->mode_overridden = (uint8)btc_mode;
				wlc_btc_mode_set(wlc, WL_BTC_FULLTDM);
			}
		} else if (wlc->btch->high_threshold &&
			-wlc->cfg->link->rssi <
			wlc->btch->high_threshold) {
			if (btc_mode != WL_BTC_PARALLEL) {
				wlc->btch->mode_overridden = (uint8)btc_mode;
				wlc_btc_mode_set(wlc, WL_BTC_PARALLEL);
			}
		} else {
			if (wlc->btch->mode_overridden) {
				wlc_btc_mode_set(wlc, wlc->btch->mode_overridden);
				wlc->btch->mode_overridden = 0;
			}
		}
	}
#if defined(BCMECICOEX) && defined(WL11N)
	if (CHIPID(wlc->pub->sih->chip) == BCM4331_CHIP_ID &&
		(wlc->pub->boardflags & BFL_FEM_BT)) {
		/* for X28, middle chain has to be disabeld when bt is active */
		/* to assure smooth rate policy */
		if ((btc_mode == WL_BTC_LITE || btc_mode == WL_BTC_HYBRID) &&
			BAND_2G(wlc->band->bandtype) && wlc->btch->bth_active) {
			if (btc_mode == WL_BTC_LITE)
				wlc_stf_txchain_set(wlc, 0x5, TRUE,
					WLC_TXCHAIN_ID_BTCOEX);
		} else {
			wlc_stf_txchain_set(wlc, wlc->stf->hw_txchain,
				FALSE, WLC_TXCHAIN_ID_BTCOEX);
		}
	}
#endif // endif

#ifdef WLAMPDU
	if (BT3P_HW_COEX(wlc) && BAND_2G(wlc->band->bandtype)) {
		if (wlc_btc_mode_not_parallel(btc_mode)) {
			/* Make sure STA is on the home channel to avoid changing AMPDU
			 * state during scanning
			 */
			if (AMPDU_ENAB(wlc->pub) && !SCAN_IN_PROGRESS(wlc->scan) &&
			    wlc->pub->associated) {
				/* process all bt related disabling/enabling here */
				if (wlc_btc_turnoff_aggr(wlc)) {
					bool txaggr = ON, rxaggr = ON, dyaggr = OFF;
					if (IS_BTCX_FULLTDM(btc_mode)) {
						txaggr = OFF;
						rxaggr = OFF;
					} else if (btc_mode == WL_BTC_HYBRID) {
						if (D11REV_GE(wlc->pub->corerev, 48)) {
							dyaggr = ON;
						} else {
							txaggr = OFF;
						}
						/* shutoff one rxchain to avoid steep rate drop */
						if (CHIPID(wlc->pub->sih->chip) ==
							BCM43225_CHIP_ID) {
							wlc_stf_rxchain_set(wlc, 1, TRUE);
							wlc->stf->rxchain_restore_delay = 0;
						}
					}
					wlc_ampdu_agg_state_update_tx_all(wlc, txaggr);
					wlc_ampdu_agg_state_update_rx_all(wlc, rxaggr);
					wlc_btc_hflg(wlc, dyaggr, BTCX_HFLG_DYAGG);
				} else {
#ifdef BCMLTECOEX
					/* If LTECX is enabled,
					  * Aggregation is resumed in LTECX Watchdog
					  */
					if (!BCMLTECOEX_ENAB(wlc->pub))
#endif /* BCMLTECOEX */
					{
						wlc_ampdu_agg_state_update_tx_all(wlc, ON);
						wlc_btc_hflg(wlc, OFF, BTCX_HFLG_DYAGG);
					}
					if ((btc_mode == WL_BTC_HYBRID) &&
						(CHIPID(wlc->pub->sih->chip) == BCM43225_CHIP_ID) &&
						(++wlc->stf->rxchain_restore_delay > 5)) {
						/* restore rxchain. */
						wlc_stf_rxchain_set(wlc,
							wlc->stf->hw_rxchain, TRUE);
						wlc->stf->rxchain_restore_delay = 0;
					}
				}
			}
		} else {
#ifdef BCMLTECOEX
			/* If LTECX is enabled,
			  * Aggregation is resumed in LTECX Watchdog
			  */
			if (!BCMLTECOEX_ENAB(wlc->pub))
#endif /* BCMLTECOEX */
			{
				/* Dynamic BTC mode requires this */
				wlc_ampdu_agg_state_update_tx_all(wlc, ON);
			}
		}
	}
#endif /* WLAMPDU */

	/* Dynamic restaging of rxgain for BTCoex */
	wlc_btcx_desense(btc, wlc->band->bandtype);

	if (wlc->clk && (wlc->pub->sih->boardvendor == VENDOR_APPLE) &&
	    ((CHIPID(wlc->pub->sih->chip) == BCM4331_CHIP_ID) ||
	     (CHIPID(wlc->pub->sih->chip) == BCM4360_CHIP_ID))) {
		wlc_write_shm(wlc, M_COREMASK_BTRESP, (uint16)btc->siso_ack);
	}
}

/* handle BTC related iovars */

static int
wlc_btc_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_btc_info_t *btc = (wlc_btc_info_t *)ctx;
	wlc_info_t *wlc = (wlc_info_t *)btc->wlc;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	bool bool_val;
	int err = 0;

	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (p_len >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)), &int_val2, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {

	case IOV_SVAL(IOV_BTC_FLAGS):
		err = wlc_btc_flags_idx_set(wlc, int_val, int_val2);
		break;

	case IOV_GVAL(IOV_BTC_FLAGS): {
		*ret_int_ptr = wlc_btc_flags_idx_get(wlc, int_val);
		break;
		}

	case IOV_SVAL(IOV_BTC_PARAMS):
		err = wlc_btc_params_set(wlc, int_val, int_val2);
		break;

	case IOV_GVAL(IOV_BTC_PARAMS):
		*ret_int_ptr = wlc_btc_params_get(wlc, int_val);
		break;

	case IOV_SVAL(IOV_BTC_MODE):
		err = wlc_btc_mode_set(wlc, int_val);
		break;

	case IOV_GVAL(IOV_BTC_MODE):
		*ret_int_ptr = wlc_btc_mode_get(wlc);
		break;

	case IOV_SVAL(IOV_BTC_WIRE):
		err = wlc_btc_wire_set(wlc, int_val);
		break;

	case IOV_GVAL(IOV_BTC_WIRE):
		*ret_int_ptr = wlc_btc_wire_get(wlc);
		break;

	case IOV_SVAL(IOV_BTC_STUCK_WAR):
		wlc_btc_stuck_war50943(wlc, bool_val);
		break;

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	case IOV_SVAL(IOV_BTCX_CLEAR_DUMP):
		err = wlc_clr_btcxdump(wlc);
		break;
#endif /* BCMDBG || BCMDBG_DUMP */

	case IOV_GVAL(IOV_BTC_SISO_ACK):
		*ret_int_ptr = wlc_btc_siso_ack_get(wlc);
		break;

	case IOV_SVAL(IOV_BTC_SISO_ACK):
		wlc_btc_siso_ack_set(wlc, (int16)int_val, TRUE);
		break;

	case IOV_GVAL(IOV_BTC_RXGAIN_THRESH):
		*ret_int_ptr = ((uint32)btc->restage_rxgain_on_rssi_thresh |
			((uint32)btc->restage_rxgain_off_rssi_thresh << 8));
		break;

	case IOV_SVAL(IOV_BTC_RXGAIN_THRESH):
		if (int_val == 0) {
			err = wlc_iovar_setint(wlc, "phy_btc_restage_rxgain", 0);
			if (err == BCME_OK) {
				btc->restage_rxgain_on_rssi_thresh = 0;
				btc->restage_rxgain_off_rssi_thresh = 0;
				btc->restage_rxgain_active = 0;
				WL_BTCPROF(("wl%d: BTC restage rxgain disabled\n", wlc->pub->unit));
			} else {
				err = BCME_NOTREADY;
			}
		} else {
			btc->restage_rxgain_on_rssi_thresh = (uint8)(int_val & 0xFF);
			btc->restage_rxgain_off_rssi_thresh = (uint8)((int_val >> 8) & 0xFF);
			WL_BTCPROF(("wl%d: BTC restage rxgain enabled\n", wlc->pub->unit));
		}
		WL_BTCPROF(("wl%d: BTC restage rxgain thresh ON: -%d, OFF -%d\n",
			wlc->pub->unit,
			btc->restage_rxgain_on_rssi_thresh,
			btc->restage_rxgain_off_rssi_thresh));
		break;

	case IOV_GVAL(IOV_BTC_RXGAIN_FORCE):
		*ret_int_ptr = btc->restage_rxgain_force;
		break;

	case IOV_SVAL(IOV_BTC_RXGAIN_FORCE):
		btc->restage_rxgain_force = int_val;
		break;

	case IOV_GVAL(IOV_BTC_RXGAIN_LEVEL):
		*ret_int_ptr = btc->restage_rxgain_level;
		break;

	case IOV_SVAL(IOV_BTC_RXGAIN_LEVEL):
		btc->restage_rxgain_level = int_val;
		if (btc->restage_rxgain_active) {
			if ((err = wlc_iovar_setint(wlc, "phy_btc_restage_rxgain",
				btc->restage_rxgain_level)) != BCME_OK) {
				/* Need to apply new level on next update */
				btc->restage_rxgain_active = 0;
				err = BCME_NOTREADY;
			}
			WL_BTCPROF(("wl%d: set BTC rxgain level %d (active %d)\n",
				wlc->pub->unit,
				btc->restage_rxgain_level,
				btc->restage_rxgain_active));
		}
		break;

	default:
		err = BCME_UNSUPPORTED;
	}
	return err;
}

/* E.g., To set BTCX_HFLG_SKIPLMP, wlc_btc_hflg(wlc, 1, BTCX_HFLG_SKIPLMP) */
void
wlc_btc_hflg(wlc_info_t *wlc, bool set, uint16 val)
{
	uint16 btc_blk_ptr, btc_hflg;

	if (!wlc->clk)
		return;

	btc_blk_ptr = 2 * wlc_bmac_read_shm(wlc->hw, M_BTCX_BLK_PTR);
	btc_hflg = wlc_bmac_read_shm(wlc->hw, btc_blk_ptr + M_BTCX_HOST_FLAGS);

	if (set)
		btc_hflg |= val;
	else
		btc_hflg &= ~val;

	wlc_bmac_write_shm(wlc->hw, btc_blk_ptr + M_BTCX_HOST_FLAGS, btc_hflg);
}

/* Returns true if Aggregation needs to be turned off for BTCX */
bool
wlc_btc_turnoff_aggr(wlc_info_t *wlc)
{
	if ((wlc == NULL) || (wlc->btch == NULL)) {
		return FALSE;
	}
	return (wlc->btch->bth_period && (wlc->btch->bth_period < BT_AMPDU_THRESH));
}

bool
wlc_btc_mode_not_parallel(int btc_mode)
{
	return (btc_mode && (btc_mode != WL_BTC_PARALLEL));
}

bool
wlc_btc_active(wlc_info_t *wlc)
{
	return (wlc->btch->bth_active);
}
