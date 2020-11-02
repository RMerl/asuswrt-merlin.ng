/*
 * 802.11h Quiet module source file
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
 * $Id: wlc_quiet.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlDriver11DH]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef WLQUIET

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
#include <wlc_ap.h>
#include <wlc_bmac.h>
#include <wlc_assoc.h>
#include <wl_export.h>
#ifdef WLMCNX
#include <wlc_mcnx.h>
#endif // endif
#include <wlc_11h.h>
#include <wlc_quiet.h>
#include <wlc_utils.h>
#ifdef WLP2P
#include <wlc_p2p.h>
#endif // endif
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>

/* IOVar table */
/* No ordering is imposed */
enum {
	IOV_QUIET,	/* send Quiet IE in beacons */
	IOV_LAST
};

static const bcm_iovar_t wlc_quiet_iovars[] = {
#ifdef AP
	{"quiet", IOV_QUIET, (IOVF_SET_UP|IOVF_BSSCFG_AP_ONLY), IOVT_BUFFER, sizeof(dot11_quiet_t)},
#endif // endif
	{NULL, 0, 0, 0, 0}
};

/* ioctl table */
static const wlc_ioctl_cmd_t wlc_quiet_ioctls[] = {
	{WLC_SEND_QUIET, WLC_IOCF_DRIVER_UP|WLC_IOCF_BSSCFG_AP_ONLY, sizeof(dot11_quiet_t)}
};

/* Quiet module info */
struct wlc_quiet_info {
	wlc_info_t *wlc;
	int cfgh;			/* bsscfg cubby handle */
};

/* local functions */
/* module */
static int wlc_quiet_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
static int wlc_quiet_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif);
static bool wlc_quiet_read_tsf(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint32 *tsfl, uint32 *tsfh);

/* cubby */
static int wlc_quiet_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_quiet_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
#ifdef BCMDBG
static void wlc_quiet_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_quiet_bsscfg_dump NULL
#endif // endif

/* up/down */
static void wlc_quiet_bsscfg_up_down(void *ctx, bsscfg_up_down_event_data_t *evt_data);

/* timer */
static void wlc_quiet_timer(void *arg);

/* STA quiet procedures */
static void wlc_start_quiet0(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg);
static void wlc_start_quiet1(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg);
static void wlc_start_quiet2(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg);
static void wlc_start_quiet3(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg);

/* IE mgmt */
#ifdef AP
static uint wlc_quiet_calc_quiet_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_quiet_write_quiet_ie(void *ctx, wlc_iem_build_data_t *data);
#endif // endif
#ifdef STA
static int wlc_quiet_bcn_parse_quiet_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif // endif

/* cubby structure and access macros */
typedef struct wlc_quiet {
	uint32 ext_state;	/* external states */
	uint32 state;		/* internal states */
	uint32 start_tsf;	/* When to start being quiet */
	uint32 end_tsf;		/* When we can start xmiting again */
	/* dual purpose: wait for offset with beacon & wait for duration */
	struct wl_timer *timer;
	/* quiet info */
	uint8 count;
	uint8 period;
	uint16 duration;
	uint16 offset;
} wlc_quiet_t;
#define QUIET_BSSCFG_CUBBY_LOC(qm, cfg) ((wlc_quiet_t **)BSSCFG_CUBBY(cfg, (qm)->cfgh))
#define QUIET_BSSCFG_CUBBY(qm, cfg) (*QUIET_BSSCFG_CUBBY_LOC(qm, cfg))

/* quiet->state */
#define WAITING_FOR_FLUSH_COMPLETE 	(1 << 0)
#define WAITING_FOR_INTERVAL		(1 << 1)
#define WAITING_FOR_OFFSET		(1 << 2)
#define WAITING_FOR_DURATION		(1 << 3)

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* module */
wlc_quiet_info_t *
BCMATTACHFN(wlc_quiet_attach)(wlc_info_t *wlc)
{
	wlc_quiet_info_t *qm;
#ifdef AP
	uint16 bcnfstbmp = FT2BMP(FC_BEACON) | FT2BMP(FC_PROBE_RESP);
#endif // endif

	if ((qm = MALLOCZ(wlc->osh, sizeof(wlc_quiet_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	qm->wlc = wlc;

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((qm->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(wlc_quiet_t *),
	                wlc_quiet_bsscfg_init, wlc_quiet_bsscfg_deinit, wlc_quiet_bsscfg_dump,
	                (void *)qm)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register bsscfg up/down callbacks */
	if (wlc_bsscfg_updown_register(wlc, wlc_quiet_bsscfg_up_down, qm) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (wlc_module_register(wlc->pub, wlc_quiet_iovars, "quiet", (void *)qm, wlc_quiet_doiovar,
	                        NULL, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

	if (wlc_module_add_ioctl_fn(wlc->pub, qm, wlc_quiet_doioctl,
	                            ARRAYSIZE(wlc_quiet_ioctls), wlc_quiet_ioctls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_add_ioctl_fn() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register IE mgmt callback */
	/* calc/build */
#ifdef AP
	/* bcn/prbrsp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_QUIET_ID,
	      wlc_quiet_calc_quiet_ie_len, wlc_quiet_write_quiet_ie, qm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, quiet in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* AP */
	/* parse */
#ifdef STA
	/* bcn */
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_QUIET_ID,
	                         wlc_quiet_bcn_parse_quiet_ie, qm) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, quiet in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* STA */

	return qm;

	/* error handling */
fail:
	wlc_quiet_detach(qm);
	return NULL;
}

void
BCMATTACHFN(wlc_quiet_detach)(wlc_quiet_info_t *qm)
{
	wlc_info_t *wlc;

	if (qm == NULL)
		return;

	wlc = qm->wlc;

	/* unregister bsscfg up/down callbacks */
	wlc_bsscfg_updown_unregister(wlc, wlc_quiet_bsscfg_up_down, qm);

	wlc_module_remove_ioctl_fn(wlc->pub, qm);
	wlc_module_unregister(wlc->pub, "quiet", qm);

	MFREE(wlc->osh, qm, sizeof(wlc_quiet_info_t));
}

static int
wlc_quiet_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	int err = BCME_OK;
	int32 int_val = 0;
	int32 *ret_int_ptr;
#ifdef AP
	wlc_quiet_info_t *qm = (wlc_quiet_info_t *)ctx;
	wlc_info_t *wlc = qm->wlc;
	wlc_bsscfg_t *cfg;

	/* update bsscfg w/provided interface context */
	cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(cfg != NULL);
#endif // endif
	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;
	BCM_REFERENCE(ret_int_ptr);

	/* Do the actual parameter implementation */
	switch (actionid) {
#ifdef AP
	case IOV_SVAL(IOV_QUIET):
#ifdef RADIO_PWRSAVE
		/*
		 * Check for radio power save feature enabled
		 */
		if (RADIO_PWRSAVE_ENAB(wlc->ap)) {
			WL_ERROR(("Please disable Radio Power Save feature using"
			          "radio_pwrsave_enable IOVAR"
			          "to continue with quiet IE testing\n"));
			err = BCME_ERROR;
			break;
		}
#endif // endif
		wlc_quiet_do_quiet(qm, cfg, (dot11_quiet_t *)arg);

		break;
#endif /* AP */

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static int
wlc_quiet_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	wlc_quiet_info_t *qm = (wlc_quiet_info_t *)ctx;
	wlc_info_t *wlc = qm->wlc;
	int err = BCME_OK;

	switch (cmd) {
	case WLC_SEND_QUIET:
		err = wlc_iovar_op(wlc, "quiet", NULL, 0, arg, len, IOV_SET, wlcif);
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* bsscfg cubby */
static int
wlc_quiet_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_quiet_info_t *qm = (wlc_quiet_info_t *)ctx;
	wlc_info_t *wlc = qm->wlc;
	wlc_quiet_t **pquiet = QUIET_BSSCFG_CUBBY_LOC(qm, cfg);
	wlc_quiet_t *quiet;
	int err;

	if ((quiet = MALLOCZ(wlc->osh, sizeof(wlc_quiet_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		err = BCME_NOMEM;
		goto fail;
	}
	*pquiet = quiet;

	/* init quiet timer (for STA only) */
	if ((quiet->timer =
	     wl_init_timer(wlc->wl, wlc_quiet_timer, cfg, "quiet")) == NULL) {
		WL_ERROR(("wl%d: wl_init_timer for bsscfg %d quiet timer failed\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
		err = BCME_NORESOURCE;
		goto fail;
	}

	return BCME_OK;

fail:
	if (quiet != NULL)
		wlc_quiet_bsscfg_deinit(ctx, cfg);
	return err;
}

static void
wlc_quiet_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_quiet_info_t *qm = (wlc_quiet_info_t *)ctx;
	wlc_info_t *wlc = qm->wlc;
	wlc_quiet_t **pquiet = QUIET_BSSCFG_CUBBY_LOC(qm, cfg);
	wlc_quiet_t *quiet = *pquiet;

	if (quiet == NULL) {
		WL_ERROR(("wl%d: %s: Quiet info not found\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

	/* delete quiet timer */
	if (quiet->timer != NULL) {
		wl_free_timer(wlc->wl, quiet->timer);
		quiet->timer = NULL;
	}

	MFREE(wlc->osh, quiet, sizeof(wlc_quiet_t));
	*pquiet = NULL;
}

#ifdef BCMDBG
static void
wlc_quiet_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_quiet_info_t *qm = (wlc_quiet_info_t *)ctx;
	wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);

	ASSERT(quiet != NULL);

	/* Quiet info */
	bcm_bprintf(b, "\ttimer: %p\n", quiet->timer);
	bcm_bprintf(b, "\tie: period %d count %d, offset %d duration %d\n",
	            quiet->period, quiet->count, quiet->offset, quiet->duration);
	bcm_bprintf(b, "\text_state: 0x%x state: 0x%x\n", quiet->ext_state, quiet->state);
}
#endif /* BCMDBG */

/* bsscfg up/down callbacks */
static void
wlc_quiet_bsscfg_up_down(void *ctx, bsscfg_up_down_event_data_t *evt_data)
{
	wlc_quiet_info_t *qm = (wlc_quiet_info_t *)ctx;
	wlc_info_t *wlc = qm->wlc;
	wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, evt_data->bsscfg);

	/* Only process bsscfg down events. */
	if (!evt_data->up) {
		ASSERT(quiet != NULL);

		quiet->ext_state = 0;
		quiet->state = 0;

		/* cancel any quiet timer */
		evt_data->callbacks_pending =
		   wl_del_timer(wlc->wl, quiet->timer) ? 0 : 1;
	}
}

/* read tsf for the given cfg -- eg adjust using mcnx if needed */
/* return true iff mcnx is used to alter hw timestamp */
static bool
wlc_quiet_read_tsf(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint32 *tsfl, uint32 *tsfh)
{
	bool mcnx_alter = FALSE;
	wlc_read_tsf(wlc, tsfl, tsfh);
#ifdef WLMCNX
	if (MCNX_ENAB(wlc->pub)) {
	/* If ROML code is compiled with WLP2P_UCODE enabled, we need to
	* add offset to local tsf before doing caculations with tsf from
	* Beacon
	*/
		wlc_mcnx_l2r_tsf64(wlc->mcnx, cfg, *tsfh, *tsfl,
			tsfh, tsfl);
		mcnx_alter = TRUE;
	}
#endif /* WLMCNX */
	return mcnx_alter;
}

/* Maximum FIFO drain is how long it takes to fully drain the FIFO when transmitting at the slowest
 * supported rate (1Mbps).
 */
#define MAX_FIFO_DRAIN_TIME	33	/* millisecs */

#ifdef STA

/*
 *    Theory of quiet mode:
 *	AP sends quiet mode info element.
 *	wlc_11h_quiet()-parses IE. Take into account the overhead in
 *	to shutting down the FIFOs and determine
 *	proper interval and offset. Register quiet0 to be called when
 *	we have reached the target beacon interval.
 *
 *	step0() -	Now in correct beacon interval but need to wait
 *			for the right offset within beacon.  Use OS timer
 *			then proceed to step1().
 *
 *	step1()-	Timer dinged and we are now at the right place to start
 *			shutting down FIFO's. Wait for FIFO drain interrupts.
 *
 *	step2()-	Interrupts came in. We are now mute. Set timer
 *			when its time to come out of quiet mode.
 *
 *	step3()-	Timer dinged. Resume normal operation.
 *
 */

/* Came from: wlc_parse_11h()
 * Will go to: wlc_start_quiet0()
 */
static void
wlc_11h_quiet(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg,
	dot11_quiet_t *ie, struct dot11_bcn_prb *bcn)
{
	wlc_info_t *wlc = qm->wlc;
	wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);
	wlc_bss_info_t *current_bss = cfg->current_bss;
	uint32 bi_us = (uint32)current_bss->beacon_period * DOT11_TU_TO_US;
	uint32 tsf_h, tsf_l;
	uint32 delta_tsf_l, delta_tsf_h;
	uint32 offset, beacon_start;
	int32 countdown;
	bool mcnx_alter_tsf;

	ASSERT(quiet != NULL);

	/*
	 * Ignore further requests for quiet if we're already handling one or if it's a "holdoff"
	 * one.
	 *
	 * In "holdoff", Cisco APs continually send Quiet IEs with count=255 instead of omitting the
	 * Quiet IE when quiet period is disabled. It's safe to ignore these because when the quiet
	 * period is enabled, we'll eventually see a non-255 Quiet IE as the AP counts down to the
	 * quiet period.
	 */
	if (quiet->state != 0 || quiet->count == 255) {
		WL_REGULATORY(("quiet: rcvd but ignoring:count %d, period %d, duration %d, offset"
			" %d\n",
			ie->count, ie->period, ltoh16(ie->duration), ltoh16(ie->offset)));
		return;
	}

	/* Parse info and setup to wait for the right beacon interval */
	WL_REGULATORY(("quiet: count %d, period %d, duration %d, offset %d\n",
		ie->count, ie->period, ltoh16(ie->duration), ltoh16(ie->offset)));

	quiet->duration = ltoh16(ie->duration);	/* TU's */
	quiet->offset = ltoh16(ie->offset);	/* TU's offset from TBTT in Count field */
	quiet->period = ie->period;
	countdown = ie->count;			/* num beacons until start */

	if (!countdown)
		return;

	/* The FIFO's will take a while to drain.  If the offset is less than that
	 * time, back up into the previous beacon interval.
	 */
	if (quiet->offset < MAX_FIFO_DRAIN_TIME) {
		countdown--;
	}

	/* Calculate absolute TSF of when to start preparing to be quiet */
	tsf_l = ltoh32_ua(&bcn->timestamp[0]);
	tsf_h = ltoh32_ua(&bcn->timestamp[1]);
	offset = wlc_calc_tbtt_offset(ltoh16(bcn->beacon_interval), tsf_h, tsf_l);
	beacon_start = tsf_l - offset;

	/* get diff between bsscfg tsf and bcn tsf */
	mcnx_alter_tsf = wlc_quiet_read_tsf(wlc, cfg, &delta_tsf_l, &delta_tsf_h);
	wlc_uint64_sub(&delta_tsf_h, &delta_tsf_l, tsf_h, tsf_l); /* a - b => a */

	/* usecs for starting quiet period */
	quiet->start_tsf = beacon_start + delta_tsf_l + (bi_us * ie->count) +
		(quiet->offset * DOT11_TU_TO_US);
	quiet->end_tsf = quiet->start_tsf + quiet->duration * DOT11_TU_TO_US;

	if (APSTA_ENAB(wlc->pub) && AP_ACTIVE(wlc) && !mcnx_alter_tsf) {
		uint32 qoffset;
		uint32 mytsf_l, mytsf_h, myoffset;

		wlc_read_tsf(wlc, &mytsf_l, &mytsf_h);
		myoffset = wlc_calc_tbtt_offset(current_bss->beacon_period, mytsf_h, mytsf_l);
		qoffset = (uint32)(quiet->offset) + (myoffset - offset);
		if (qoffset < MAX_FIFO_DRAIN_TIME) {
			if (countdown == ie->count) {
				countdown--;
			}
		}

		quiet->offset = (int)qoffset;
		quiet->start_tsf += (mytsf_l - tsf_l);
		quiet->end_tsf = quiet->start_tsf + quiet->duration * DOT11_TU_TO_US;
	}

	/* Save this for last cuz this is what the dpc looks at */
	quiet->state = WAITING_FOR_INTERVAL;
	quiet->count = (uint8)countdown;
	WL_REGULATORY(("%s: Set timer for = %d TBTTs\n", __FUNCTION__, countdown));
	if (countdown == 0) {
		/* We need to go quiet in this interval, jump straight to offset. */
		wlc_start_quiet0(qm, cfg);
	} else {
		wl_add_timer(wlc->wl, quiet->timer,
			((current_bss->beacon_period * DOT11_TU_TO_US)/1000) * countdown, 0);
	}

	/* When the quiet timer expires, wlc_start_quiet0() will be called in timer handler
	 * wlc_quiet_timer()
	 */
}
#endif /* STA */

/* Triple use timer handler: Either
 *	- Waiting to advance to 'count' tbtts into offset waiting
 *	- Or waiting to advance to 'offset' usecs into bcn interval to start quiet period.
 *	- Or waiting for end of quiet period.
 *	Came from:  wlc_11h_quiet() via timer interrupt, wlc_start_quiet0() via timer interrupt, or
 *		wlc_start_quiet2() via timer interrupt.
 *	Will go to: wlc_start_quiet0(), wlc_start_quiet1() or wlc_start_quiet3()
 */
static void
wlc_quiet_timer(void *arg)
{
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t*)arg;
	wlc_info_t *wlc = cfg->wlc;
	wlc_quiet_info_t *qm = wlc->quiet;
	wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);

	if (!wlc->pub->up)
		return;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}

	WL_REGULATORY(("Entering %s\n", __FUNCTION__));

	if (quiet->state & WAITING_FOR_INTERVAL) {
		wlc_start_quiet0(qm, cfg);
		return;
	}
	if (quiet->state & WAITING_FOR_OFFSET) {
		wlc_start_quiet1(qm, cfg);
	}
	if (quiet->state & WAITING_FOR_DURATION) {
		wlc_start_quiet3(qm, cfg);
	}
}

/* Now in correct beacon interval but need to wait for offset.
 * Came from: quiet timer handler wlc_quiet_timer()
 * Will go to: wlc_start_quiet1() via wlc_quiet_timer()
 */
static void
wlc_start_quiet0(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = qm->wlc;
	wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);
	uint32 tsf_hi, tsf_lo;
	uint32 start_prep_ms;
	uint32 start_prep_tsf;
	uint32 cur_rate;
	uint32 fifo_drain_time;

	ASSERT(quiet != NULL);

	ASSERT(quiet->state & WAITING_FOR_INTERVAL);
	WL_REGULATORY(("Entering %s\n", __FUNCTION__));

	quiet->state = WAITING_FOR_OFFSET;
	/*
	 * Adjust offset based on estimated FIFO drain time (based on current TX rate).
	 *
	 * This is using a conservative version of the formula so that no matter how fast we
	 * transmit, we'll have a minimum FIFO drain time of 1ms.
	 */
	cur_rate = RSPEC2KBPS(wlc_get_rspec_history(cfg)) / 1000;
	fifo_drain_time = cur_rate ? (((MAX_FIFO_DRAIN_TIME - 1) / cur_rate) + 1) : 1;

	WL_REGULATORY(("%s: txrate=%d drain_time=%d\n", __FUNCTION__, cur_rate, fifo_drain_time));

	start_prep_tsf = quiet->start_tsf - fifo_drain_time * 1000;

	(void)wlc_quiet_read_tsf(wlc, cfg, &tsf_lo, &tsf_hi);

	if (((tsf_lo == (uint32)-1) && (tsf_hi == (uint32)-1)) || (tsf_lo > quiet->end_tsf) ||
	    (tsf_lo >= start_prep_tsf)) {
		/* Already late so call routine directly instead of through timer. */
		WL_REGULATORY(("%s: Already late, call quiet1 directly. \n", __FUNCTION__));
		wlc_start_quiet1(qm, cfg);
	} else {
		start_prep_ms = (start_prep_tsf - tsf_lo) / 1000;
		/* Set timer for difference (in millisecs) */
		WL_REGULATORY(("%s: Start FIFO drain in %d ms\n",
			__FUNCTION__, start_prep_ms));
		wl_add_timer(wlc->wl, quiet->timer, start_prep_ms, 0);
	}
}

/* Called from wlc_quiet_timer() when we are at proper offset within
 * proper beacon.  Need to initiate quiet period now.
 * Came from: wlc_start_quiet0() via wlc->quiet_timer.
 * Will go to: wlc_start_quiet2().
 */
static void
wlc_start_quiet1(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = qm->wlc;
	wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);

	ASSERT(quiet != NULL);

	/* TODO:
	      while in quiet mode. Places that will reject are:
		- wlc_roam_request  XXX Need this.
	*/

	WL_REGULATORY(("Entering %s\n", __FUNCTION__));

	ASSERT(quiet->state & WAITING_FOR_OFFSET);
	quiet->state &= ~WAITING_FOR_OFFSET;

	if (wlc_mac_request_entry(wlc, cfg, WLC_ACTION_QUIET) != BCME_OK) {
		/* blocked from entry, cancel quiet period */
		quiet->ext_state = 0;
		quiet->state = 0;	/* All done */
		return;
	}

	/* reject future scans until done with quiet */
	quiet->ext_state |= SILENCE;

	/* Send feedback upstream */
	wlc_bss_mac_event(wlc, cfg, WLC_E_QUIET_START, NULL, WLC_E_STATUS_SUCCESS, 0, 0, 0, 0);

	wlc_bmac_tx_fifo_suspend(wlc->hw, TX_DATA_FIFO);
	wlc_bmac_tx_fifo_suspend(wlc->hw, TX_CTL_FIFO);

	quiet->state |= WAITING_FOR_FLUSH_COMPLETE;

	/* Now wait for FIFO drain interrupt and proceed to wlc_start_quiet2() */
}

/* Fifo's have drained, now be quiet.
 * Came from: wlc_start_quiet1() via dpc FIFO drain handler.
 * Will go to: wlc_start_quiet3() via wlc_quiet_timer.
 */
static void
wlc_start_quiet2(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = qm->wlc;
	wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);
	uint32 tsf_hi, tsf_lo;

	ASSERT(quiet != NULL);

	WL_REGULATORY(("Entering %s\n", __FUNCTION__));
	ASSERT(!(quiet->state & WAITING_FOR_FLUSH_COMPLETE));

	/* zero the address match register so we do not send ACKs */
	wlc_clear_mac(cfg);

	(void)wlc_quiet_read_tsf(wlc, cfg, &tsf_lo, &tsf_hi);

	quiet->state |= WAITING_FOR_DURATION;

	if (((tsf_lo == (uint32)-1) && (tsf_hi == (uint32)-1)) || (tsf_lo > quiet->end_tsf)) {
		/* wlc_read_tsf failed, or end the quiet period immediately */
		WL_REGULATORY(("%s; Warning: ending quiet immediatly\n", __FUNCTION__));
		WL_REGULATORY(("Current tsf: tsf_h 0x%x, tsf_l 0x%x\n", tsf_hi, tsf_lo));
		WL_REGULATORY(("Start_tsf(lo) 0x%x, End_tsf(lo) 0x%x\n",
			quiet->start_tsf, quiet->end_tsf));
		wlc_start_quiet3(qm, cfg);
	}
	else {
		/* Set timer for difference (in millisecs) */
		WL_REGULATORY(("Arming timer to end quiet period in 0x%x - 0x%x = %d ms\n",
			quiet->end_tsf, tsf_lo, (quiet->end_tsf - tsf_lo) / 1000));

		wl_add_timer(wlc->wl, quiet->timer, (quiet->end_tsf - tsf_lo) / 1000, 0);
	}
}

/* Done with quiet period.
 * Called from: timer via wlc_start_quiet2().
 * Will go to: wlc_quiet_timer()->dpc handler.
 */
static void
wlc_start_quiet3(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = qm->wlc;
	wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);

	ASSERT(quiet != NULL);

	WL_REGULATORY(("Entering %s\n", __FUNCTION__));
	ASSERT(quiet->state & WAITING_FOR_DURATION);
	quiet->state &= ~WAITING_FOR_DURATION;

	/* restore the address match register for this cfg */
	wlc_set_mac(cfg);

	wlc_mute(wlc, OFF, 0);

	wlc_bss_mac_event(wlc, cfg, WLC_E_QUIET_END, NULL, WLC_E_STATUS_SUCCESS, 0, 0, 0, 0);

	WL_REGULATORY(("%s:Quiet Complete\n", __FUNCTION__));
	quiet->ext_state = 0;
	quiet->state = 0;	/* All done */
}

void
wlc_quiet_txfifo_suspend_complete(wlc_quiet_info_t *qm)
{
	wlc_info_t *wlc = qm->wlc;
	int idx;
	wlc_bsscfg_t *cfg;

	FOREACH_AS_STA(wlc, idx, cfg) {
		wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);
		ASSERT(quiet != NULL);

		if (quiet->state & WAITING_FOR_FLUSH_COMPLETE) {
			quiet->state &= ~WAITING_FOR_FLUSH_COMPLETE;
			wlc_start_quiet2(qm, cfg);
		}
	}
}

#ifdef AP
/* 802.11h Quiet */
static uint
wlc_quiet_calc_quiet_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_quiet_info_t *qm = (wlc_quiet_info_t *)ctx;
	wlc_info_t *wlc = qm->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, cfg)) {
		uint len = wlc_p2p_write_ie_quiet_len(wlc->p2p, cfg, data->ft);
		if (len != 0)
			return len;
	}
#endif // endif

	if (BSS_WL11H_ENAB(wlc, cfg) && BSSCFG_AP(cfg)) {
		wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);

		if (quiet->count != 0)
			return sizeof(dot11_quiet_t);
	}

	return 0;
}

static int
wlc_quiet_write_quiet_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_quiet_info_t *qm = (wlc_quiet_info_t *)ctx;
	wlc_info_t *wlc = qm->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

#ifdef WLP2P
	if (BSS_P2P_ENAB(wlc, cfg)) {
		int len = wlc_p2p_write_ie_quiet(wlc->p2p, cfg, data->ft, data->buf);
		if (len > 0)
			return BCME_OK;
	}
#endif // endif

	if (BSS_WL11H_ENAB(wlc, cfg) && BSSCFG_AP(cfg)) {
		wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);

		if (quiet->count != 0) {
			dot11_quiet_t *qie = (dot11_quiet_t *)data->buf;

			qie->id = DOT11_MNG_QUIET_ID;
			qie->len = 6;
			qie->count = quiet->count;
			qie->period = quiet->period;
			qie->duration = htol16(quiet->duration);
			qie->offset = htol16(quiet->offset);
			WL_REGULATORY(("wl%d: Sending Quiet IE: count:%d, duration:%d, offset:%d\n",
				qm->wlc->pub->unit, qie->count, qie->duration, qie->offset));
		}
	}

	return BCME_OK;
}
#endif /* AP */

#ifdef STA
static int
wlc_quiet_bcn_parse_quiet_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_quiet_info_t *qm = (wlc_quiet_info_t *)ctx;
	/* too short ies are possible: guarded against here */
	if (data->ie != NULL && data->ie_len >= sizeof(dot11_quiet_t)) {
		wlc_11h_quiet(qm, data->cfg, (dot11_quiet_t *)data->ie,
			(struct dot11_bcn_prb *)data->pparm->body);
	}

	return BCME_OK;
}
#endif // endif

void
wlc_quiet_do_quiet(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg, dot11_quiet_t *qie)
{
	wlc_info_t *wlc = qm->wlc;
	wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);

	ASSERT(quiet != NULL);

	quiet->count = qie->count;
	quiet->period = qie->period;
	quiet->duration = qie->duration;
	quiet->offset = qie->offset;

	WL_REGULATORY(("wl%d: Quiet ioctl: count %d, dur %d, offset %d\n",
	               wlc->pub->unit, qie->count, qie->duration, qie->offset));

	/* Update the beacon/probe response */
	wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_UPDATE_BCN, NEED_TO_UPDATE_BCN);
	wlc_bss_update_beacon(wlc, cfg);
	wlc_bss_update_probe_resp(wlc, cfg, TRUE);
}

void
wlc_quiet_reset_all(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg)
{
	wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);

	ASSERT(quiet != NULL);

	quiet->count = 0;
}

/* accessors */
uint
wlc_quiet_get_quiet_state(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg)
{
	wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);

	ASSERT(quiet != NULL);

	return quiet->ext_state;
}

uint
wlc_quiet_get_quiet_count(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg)
{
	wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);

	ASSERT(quiet != NULL);

	return quiet->count;
}

void
wlc_quiet_count_down(wlc_quiet_info_t *qm, wlc_bsscfg_t *cfg)
{
	wlc_quiet_t *quiet = QUIET_BSSCFG_CUBBY(qm, cfg);

	ASSERT(quiet != NULL);

	if (quiet->count) {
		quiet->count--;
	}
}
#endif /* WLQUIET */
