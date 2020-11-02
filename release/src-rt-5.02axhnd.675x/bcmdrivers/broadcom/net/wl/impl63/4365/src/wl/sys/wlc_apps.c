/*
 * Common interface to the 802.11 AP Power Save state per scb
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
 * $Id: wlc_apps.c 775738 2019-06-12 03:29:56Z $
 */

/**
 * @file
 * @brief
 * Twiki: [WlDriverPowerSave]
 */

/* Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifndef AP
#error "AP must be defined to include this module"
#endif  /* AP */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.11.h>
#include <proto/wpa.h>
#include <wlioctl.h>
#include <epivers.h>

#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_keymgmt.h>
#include <wlc_channel.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_ap.h>
#include <wlc_apps.h>
#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <bcmwpa.h>
#include <wlc_bmac.h>
#ifdef WLC_HIGH_ONLY
#include <wlc_rpctx.h>
#endif // endif
#ifdef WLP2P
#include <wlc_p2p.h>
#endif // endif
#ifdef PROP_TXSTATUS
#include <wlfc_proto.h>
#include <wl_wlfc.h>
#endif // endif
#ifdef WLTDLS
#include <wlc_tdls.h>
#endif // endif
#include <wl_export.h>
#ifdef WLAMPDU
#include <wlc_ampdu.h>
#endif /* WLAMPDU */
#include <wlc_pcb.h>
#ifdef WLTOEHW
#include <wlc_tso.h>
#endif /* WLTOEHW */
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#ifdef WLWNM
#include <wlc_wnm.h>
#endif // endif
#include "wlc_txc.h"
#include <wlc_tx.h>
#include <wlc_mbss.h>
#ifdef WL_BEAMFORMING
#include <wlc_txbf.h>
#endif /* WL_BEAMFORMING */
#ifdef WLNAR
#include <wlc_nar.h>
#endif // endif

static void wlc_apps_ps_timedout(wlc_info_t *wlc, struct scb *scb);
static bool wlc_apps_ps_send(wlc_info_t *wlc, struct scb *scb, uint prec_bmp, uint32 flags);
static bool wlc_apps_ps_enq_resp(wlc_info_t *wlc, struct scb *scb, void *pkt, int prec);
static void wlc_apps_ps_enq(void *ctx, struct scb *scb, void *pkt, uint prec);
static void _wlc_apps_ps_enq(void *ctx, struct scb *scb, void *pkt, uint prec);
static int wlc_apps_apsd_delv_count(wlc_info_t *wlc, struct scb *scb);
static int wlc_apps_apsd_ndelv_count(wlc_info_t *wlc, struct scb *scb);
static void wlc_apps_apsd_send(wlc_info_t *wlc, struct scb *scb);
static void wlc_apps_txq_to_psq(wlc_info_t *wlc, struct scb *scb);
static void wlc_apps_move_to_psq(wlc_info_t *wlc, struct pktq *txq, struct scb* scb);
static int wlc_apps_send_psp_response_cb(wlc_info_t *wlc, wlc_bsscfg_t *cfg, void *pkt, void *data);
#ifdef PROP_TXSTATUS
#ifdef WLNAR
static void wlc_apps_nar_txq_to_psq(wlc_info_t *wlc, struct scb *scb);
#endif /* WLNAR */
#ifdef WLAMPDU
static void wlc_apps_ampdu_txq_to_psq(wlc_info_t *wlc, struct scb *scb);
#endif /* WLAMPDU */
#endif /* PROP_TXSTATUS */
static int wlc_apps_down(void *hdl);
static void wlc_apps_apsd_eosp_send(wlc_info_t *wlc, struct scb *scb);
static int wlc_apps_bss_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_apps_bss_deinit(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_apps_bss_wd_ps_check(void *handle);

#if defined(MBSS)
static void wlc_apps_bss_ps_off_start(wlc_info_t *wlc, struct scb *bcmc_scb);
#else
#define wlc_apps_bss_ps_off_start(wlc, bcmc_scb)
#endif /* MBSS */

/* IE mgmt */
static uint wlc_apps_calc_tim_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_apps_write_tim_ie(void *ctx, wlc_iem_build_data_t *data);
static void wlc_apps_scb_state_upd_cb(void *ctx, scb_state_upd_data_t *notif_data);

#ifdef PROP_TXSTATUS
/* Maximum suppressed broadcast packets handled */
#define BCMC_MAX 4
#endif // endif

/* Flags for psp_flags */
#define PS_MORE_DATA		0x01	/* last PS resp pkt indicated more data */
#define PS_PSP_REQ_PEND		0x02	/* a pspoll req is pending */
#define PS_PSP_ONRESP		0x04	/* a pspoll req is under handling */

/* PS transition status flags of apps_scb_psinfo */
#define SCB_PS_TRANS_OFF_PEND		0x01	/* Pend PS off until txfifo draining is done */
#define SCB_PS_TRANS_OFF_BLOCKED	0x02	/* Block attempts to switch PS off */
#define SCB_PS_TRANS_OFF_IN_PROG	0x04	/* PS off transition is already in progress */

/* PS transition status flags of apps_bss_psinfo */
#define BSS_PS_TRANS_OFF_BLOCKED	0x01	/* Block attempts to switch PS of bcmc scb off */

/*
 * PSQ pktq let needs to handle large no of ampdus released and requeued.
 */
#define PSQ_PKTQ_LEN_DEFAULT        512	/* Max 512 packets */

struct apps_scb_psinfo {
	struct pktq	psq;		/* PS-defer queue */
	bool		psp_pending;	/* whether uncompleted PS-POLL response is pending */
	uint8		psp_flags;	/* various ps mode bit flags defined below */
	bool		first_suppr_handled;	/* have we handled first supr'd frames ? */
	bool		in_pvb;		/* This STA already set in partial virtual bitmap */
	bool		apsd_usp;	/* APSD Unscheduled Service Period in progress */
	int		apsd_cnt;	/* Frames left in this service period */
	mbool		tx_block;	/* tx has been blocked */
	uint32		ps_discard;	/* cnt of PS pkts which were dropped */
	bool		ext_qos_null;	/* Send extra QoS NULL frame to indicate EOSP
					 * if last frame sent in SP was MMPDU
					 */
	uint8		ps_trans_status;	/* PS transition status flag */
#ifdef PROP_TXSTATUS
	bool		apsd_hpkt_timer_on;
	struct wl_timer  *apsd_hpkt_timer;
	bool		apsd_tx_pending;
	struct scb *scb;
	wlc_info_t *wlc;
	int		in_transit;
#endif // endif
	uint16		auxpmq_idx;
	bool		change_scb_state_to_auth; /* After disassoc, scb goes into reset state
						   * and switch back to auth state, in case scb
						   * is in power save and AP holding disassoc
						   * frame for this, let the disassoc frame
						   * go out with NULL data packet from STA and
						   * wlc_apps module to change the state to auth
						   * state.
						   */
};

typedef struct apps_bss_info
{
	char		pvb[251];		/* full partial virtual bitmap */
	uint16		aid_lo;			/* lowest aid with traffic pending */
	uint16		aid_hi;			/* highest aid with traffic pending */
	uint		ps_deferred;		/* cnt of all PS pkts buffered on unicast scbs */
	uint32		ps_nodes;		/* num of STAs in PS-mode */
#if defined(WLCNT)
	uint32		bcmc_pkts_seen;		/* Packets thru BC/MC SCB queue */
	uint32		bcmc_discards;		/* Packets discarded due to full queue */
#endif /* WLCNT */
	uint8		ps_trans_status;	/* PS transition status flag */
} apps_bss_info_t;

struct apps_wlc_psinfo
{
	int		cfgh;			/* bsscfg cubby handle */
	osl_t		*osh;			/* pointer to os handle */
	uint		ps_deferred;		/* cnt of all PS pkts buffered on unicast scbs */
	uint32		ps_discard;		/* cnt of all PS pkts which were dropped */
	uint32		ps_aged;		/* cnt of all aged PS pkts */

	uint		psq_pkts_lo;		/* # ps pkts are always enq'able on scb */
	uint		psq_pkts_hi;		/* max # of ps pkts enq'able on a single scb */
	int		scb_handle;		/* scb cubby handle to retrieve data from scb */
	uint32		ps_nodes_all;		/* Count of nodes in PS across all BBSes */
};

/* AC bitmap to precedence bitmap mapping (constructed in wlc_attach) */
static uint wlc_acbitmap2precbitmap[16] = { 0 };

/* Map AC bitmap to precedence bitmap */
#define WLC_ACBITMAP_TO_PRECBITMAP(ab)  wlc_acbitmap2precbitmap[(ab) & 0xf]

#define SCB_PSINFO(psinfo, scb) ((struct apps_scb_psinfo *)SCB_CUBBY(scb, (psinfo)->scb_handle))
/* apps info accessor */
#define APPS_BSSCFG_CUBBY_LOC(psinfo, cfg) ((apps_bss_info_t **)BSSCFG_CUBBY((cfg), (psinfo)->cfgh))
#define APPS_BSSCFG_CUBBY(psinfo, cfg) (*(APPS_BSSCFG_CUBBY_LOC(psinfo, cfg)))
#define BSS_PS_NODES(psinfo, bsscfg) ((APPS_BSSCFG_CUBBY(psinfo, bsscfg))->ps_nodes)

static uint BCMRAMFN(wlc_apps_ac2precbmp_info)(uint8);
static int wlc_apps_scb_psinfo_init(void *context, struct scb *scb);
static void wlc_apps_scb_psinfo_deinit(void *context, struct scb *scb);
static uint wlc_apps_txpktcnt(void *context);
#ifdef PROP_TXSTATUS
static void wlc_apps_apsd_hpkt_tmout(void *arg);
#endif // endif

#ifdef PROP_TXSTATUS_DEBUG
void wlfc_display_debug_info(void* _wlc, int hi, int lo);
#endif // endif

static void wlc_apps_apsd_complete(wlc_info_t *wlc, void *pkt, uint txs);
static void wlc_apps_psp_resp_complete(wlc_info_t *wlc, void *pkt, uint txs);

/* Accessor Function */
static uint BCMRAMFN(wlc_apps_ac2precbmp_info)(uint8 inf)
{
	return (WLC_ACBITMAP_TO_PRECBITMAP(inf));
}

static txmod_fns_t BCMATTACHDATA(apps_txmod_fns) = {
	wlc_apps_ps_enq,
	wlc_apps_txpktcnt,
	NULL,
	NULL
};

#if defined(BCMDBG)
static void wlc_apps_scb_psinfo_dump(void *context, struct scb *scb, struct bcmstrbuf *b);

/* Limited dump routine for APPS SCB info */
static void
wlc_apps_scb_psinfo_dump(void *context, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	struct apps_scb_psinfo *scb_psinfo;
	struct apps_bss_info *bss_info;
	struct pktq *pktq;
	wlc_bsscfg_t *bsscfg;

	if (scb == NULL)
		return;

	bsscfg = scb->bsscfg;
	if (bsscfg == NULL)
		return;

	if (!BSSCFG_AP(bsscfg))
		return;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	bcm_bprintf(b, "     APPS psinfo on SCB %p is %p; scb-PS is %s;"
#ifdef PSPRETEND
		" scb-PS pretend is %s (0x%x) count %d nack %d time_in_pps %d ms\n",
		scb, scb_psinfo, SCB_PS(scb) ? "on" : "off",
		SCB_PS_PRETEND(scb) ? "on" : "off", scb->ps_pretend,
		scb->ps_pretend_count, scb->ps_pretend_failed_ack_count,
		(scb->ps_pretend_total_time_in_pps + 500)/1000);
#else
		"\n", scb, scb_psinfo, SCB_PS(scb) ? "on" : "off");
#endif // endif

	bcm_bprintf(b, "     tx_block %d ext_qos_null %d psp_pending %d discards %d\n",
		scb_psinfo->tx_block, scb_psinfo->ext_qos_null, scb_psinfo->psp_pending,
		scb_psinfo->ps_discard);

	pktq = &scb_psinfo->psq;
	if (pktq == NULL) {
		bcm_bprintf(b, "       Packet queue is NULL\n");
		return;
	}

	if (SCB_ISMULTI(scb)) {
		bcm_bprintf(b, "       SCB is multi. node count %d\n",
			BSS_PS_NODES(wlc->psinfo, bsscfg));
	}
	bss_info = APPS_BSSCFG_CUBBY(wlc->psinfo, bsscfg);
	bcm_bprintf(b, "       Pkt Q %p. Que len %d. Max %d. Avail %d. Seen %d. Disc %d\n",
	            pktq, pktq_len(pktq), pktq_max(pktq), pktq_avail(pktq),
	            WLCNTVAL(bss_info->bcmc_pkts_seen), WLCNTVAL(bss_info->bcmc_discards));
}
#else
/* Use NULL to pass as reference on init */
#define wlc_apps_scb_psinfo_dump NULL
#endif /* BCMDBG */

#ifdef PSPRETEND
int
BCMATTACHFN(wlc_pspretend_attach)(wlc_info_t *wlc)
{
	struct wlc_pps_info*  pps_info = MALLOCZ(wlc->osh, sizeof(*pps_info));

	wlc->pps_info = pps_info;

	if (pps_info == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return -1;
	}

	pps_info->wlc = wlc;

	if (!(pps_info->ps_pretend_probe_timer = wl_init_timer(wlc->wl, wlc_ap_pspretend_probe,
	                                                       wlc, "ps_pretend_probe"))) {
		WL_ERROR(("wl%d: wl_init_timer for ps_pretend_probe_timer failed\n",
		           wlc->pub->unit));
		wlc_pspretend_detach(pps_info);
		return -2;
	}
	pps_info->is_ps_pretend_probe_timer_running = FALSE;

	return 0;
}

void
BCMATTACHFN(wlc_pspretend_detach)(wlc_pps_info_t* pps_info)
{
	wlc_info_t *wlc;

	if (!pps_info) {
		return;
	}
	wlc = pps_info->wlc;

	if (pps_info->ps_pretend_probe_timer) {
		wl_del_timer(wlc->wl, pps_info->ps_pretend_probe_timer);
		wl_free_timer(wlc->wl, pps_info->ps_pretend_probe_timer);
	}

	MFREE(wlc->osh, pps_info, sizeof(*pps_info));
	wlc->pps_info = NULL;
}
#endif /* PSPRETEND */

int
BCMATTACHFN(wlc_apps_attach)(wlc_info_t *wlc)
{
	apps_wlc_psinfo_t *wlc_psinfo;
	int i;

	if (!(wlc_psinfo = MALLOCZ(wlc->osh, sizeof(apps_wlc_psinfo_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		wlc->psinfo = NULL;
		return -1;
	}

	if ((wlc_psinfo->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(apps_bss_info_t *),
		wlc_apps_bss_init, wlc_apps_bss_deinit, NULL,
		(void *)wlc_psinfo)) < 0) {
			WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
				wlc->pub->unit, __FUNCTION__));
			return -1;
	}

	/* Set the psq watermarks */
	wlc_psinfo->psq_pkts_lo = PSQ_PKTS_LO;
	wlc_psinfo->psq_pkts_hi = PSQ_PKTS_HI;

	/* calculate the total ps pkts required */
	wlc->pub->psq_pkts_total = wlc_psinfo->psq_pkts_hi +
	    (wlc->pub->tunables->maxscb * wlc_psinfo->psq_pkts_lo);

	/* reserve cubby in the scb container for per-scb private data */
	wlc_psinfo->scb_handle = wlc_scb_cubby_reserve(wlc, sizeof(struct apps_scb_psinfo),
		wlc_apps_scb_psinfo_init, wlc_apps_scb_psinfo_deinit,
		wlc_apps_scb_psinfo_dump, (void *)wlc);

	if (wlc_psinfo->scb_handle < 0) {
		WL_ERROR(("wl%d: %s: wlc_scb_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		wlc_apps_detach(wlc);
		return -2;
	}

	/* construct mapping from AC bitmap to precedence bitmap */
	for (i = 0; i < 16; i++) {
		wlc_acbitmap2precbitmap[i] = 0;
		if (AC_BITMAP_TST(i, AC_BE))
			wlc_acbitmap2precbitmap[i] |= WLC_PREC_BMP_AC_BE;
		if (AC_BITMAP_TST(i, AC_BK))
			wlc_acbitmap2precbitmap[i] |= WLC_PREC_BMP_AC_BK;
		if (AC_BITMAP_TST(i, AC_VI))
			wlc_acbitmap2precbitmap[i] |= WLC_PREC_BMP_AC_VI;
		if (AC_BITMAP_TST(i, AC_VO))
			wlc_acbitmap2precbitmap[i] |= WLC_PREC_BMP_AC_VO;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, NULL, "apps", wlc, NULL, wlc_apps_bss_wd_ps_check, NULL,
		wlc_apps_down)) {
		WL_ERROR(("wl%d: %s wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return -4;
	}

	/* register packet class callback */
	if (wlc_pcb_fn_set(wlc->pcb, 1, WLF2_PCB2_APSD, wlc_apps_apsd_complete) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_pcb_fn_set() failed\n", wlc->pub->unit, __FUNCTION__));
		return -5;
	}
	if (wlc_pcb_fn_set(wlc->pcb, 1, WLF2_PCB2_PSP_RSP, wlc_apps_psp_resp_complete) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_pcb_fn_set() failed\n", wlc->pub->unit, __FUNCTION__));
		return -6;
	}

	/* register IE mgmt callbacks */
	/* bcn */
	if (wlc_iem_add_build_fn(wlc->iemi, FC_BEACON, DOT11_MNG_TIM_ID,
	      wlc_apps_calc_tim_ie_len, wlc_apps_write_tim_ie, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, tim in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		return -7;
	}

#ifdef PSPRETEND
	if (wlc_pspretend_attach(wlc)) {
		return -8;
	}
#endif // endif

	wlc_txmod_fn_register(wlc, TXMOD_APPS, wlc, apps_txmod_fns);

	/* Add client callback to the scb state notification list */
	if (wlc_scb_state_upd_register(wlc,
	                (bcm_notif_client_callback)wlc_apps_scb_state_upd_cb, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s: unable to register callback %p\n",
		          wlc->pub->unit, __FUNCTION__, wlc_apps_scb_state_upd_cb));
		return -9;
	}

	wlc_psinfo->osh = wlc->osh;
	wlc->psinfo = wlc_psinfo;

	return 0;
}

void
BCMATTACHFN(wlc_apps_detach)(wlc_info_t *wlc)
{
	apps_wlc_psinfo_t *wlc_psinfo;

	ASSERT(wlc);

#ifdef PSPRETEND
	wlc_pspretend_detach(wlc->pps_info);
#endif // endif

	wlc_psinfo = wlc->psinfo;

	if (!wlc_psinfo)
		return;

	/* All PS packets shall have been flushed */
	ASSERT(wlc_psinfo->ps_deferred == 0);

	wlc_scb_state_upd_unregister(wlc,
	    (bcm_notif_client_callback)wlc_apps_scb_state_upd_cb, wlc);

	wlc_module_unregister(wlc->pub, "apps", wlc);

	MFREE(wlc->osh, wlc_psinfo, sizeof(apps_wlc_psinfo_t));
	wlc->psinfo = NULL;
}

static int
wlc_apps_down(void *hdl)
{
	wlc_info_t *wlc = (wlc_info_t *)hdl;
	struct apps_scb_psinfo *scb_psinfo;
	struct scb_iter scbiter;
	struct scb *tscb;

	FOREACHSCB(wlc->scbstate, &scbiter, tscb) {
		scb_psinfo = SCB_PSINFO(wlc->psinfo, tscb);

		if (!pktq_empty(&scb_psinfo->psq))
			wlc_apps_ps_flush(wlc, tscb);
	}
	if (MBSS_ENAB(wlc->pub)) {
		int i;
		wlc_bsscfg_t *cfg;

		FOREACH_BSS(wlc, i, cfg) {
			int j;
			struct scb *(*tbcmc_scb)[MAXBANDS] = &wlc->bsscfg[i]->bcmc_scb;

			for (j = 0; j < MAXBANDS; j++) {
				if ((*tbcmc_scb)[j])
					wlc_apps_ps_flush(wlc, (*tbcmc_scb)[j]);
			}
		}
	}

	return 0;
}

/* bsscfg cubby */
static int
wlc_apps_bss_init(void *ctx, wlc_bsscfg_t *cfg)
{
	apps_wlc_psinfo_t *wlc_psinfo = (apps_wlc_psinfo_t *)ctx;
	wlc_info_t *wlc = cfg->wlc;
	apps_bss_info_t **papps_bss = APPS_BSSCFG_CUBBY_LOC(wlc_psinfo, cfg);
	apps_bss_info_t *apps_bss = NULL;
	UNUSED_PARAMETER(wlc);

	/* Allocate only for AP bsscfg */
	if (BSSCFG_AP(cfg)) {
		if (!(apps_bss = (apps_bss_info_t *)MALLOCZ(wlc_psinfo->osh,
			sizeof(apps_bss_info_t)))) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc_psinfo->osh)));
			return BCME_NOMEM;
		}
	}
	*papps_bss = apps_bss;

	return BCME_OK;
}

static void
wlc_apps_bss_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	apps_wlc_psinfo_t *wlc_psinfo = (apps_wlc_psinfo_t *)ctx;
	apps_bss_info_t **papps_bss = APPS_BSSCFG_CUBBY_LOC(wlc_psinfo, cfg);
	apps_bss_info_t *apps_bss = *papps_bss;

	if (apps_bss != NULL) {
		MFREE(wlc_psinfo->osh, apps_bss, sizeof(apps_bss_info_t));
		*papps_bss = NULL;
	}

	return;
}

/* Return the count of all the packets being held by APPS module */
static uint
wlc_apps_txpktcnt(void *context)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	apps_wlc_psinfo_t *wlc_psinfo = wlc->psinfo;

	return (wlc_psinfo->ps_deferred);
}

static int
wlc_apps_scb_psinfo_init(void *context, struct scb *scb)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	struct apps_scb_psinfo *scb_psinfo;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);

	/* Initialize the auxpmq_idx first, this will be used in wlc_apps_scb_psinfo_deinit().
	 * So that if apsd_hpkt_timer fails, the deinit function won't do incorrect handling.
	 */
	scb_psinfo->auxpmq_idx = AUXPMQ_INVALID_IDX;

#ifdef PROP_TXSTATUS
	if (!(scb_psinfo->apsd_hpkt_timer = wl_init_timer(wlc->wl,
		wlc_apps_apsd_hpkt_tmout, scb_psinfo, "appsapsdhkpt"))) {
		WL_ERROR(("wl: apsd_hpkt_timer failed\n"));
		return 1;
	}
	scb_psinfo->wlc = wlc;
	scb_psinfo->scb = scb;
#endif // endif
	/* PS state init */
	pktq_init(&scb_psinfo->psq, WLC_PREC_COUNT, PSQ_PKTQ_LEN_DEFAULT);

	return 0;
}

static void
wlc_apps_scb_psinfo_deinit(void *context, struct scb *remove)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	struct apps_scb_psinfo *scb_psinfo;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, remove);

#ifdef PROP_TXSTATUS
	wl_del_timer(wlc->wl, scb_psinfo->apsd_hpkt_timer);
	scb_psinfo->apsd_hpkt_timer_on = FALSE;
	wl_free_timer(wlc->wl, scb_psinfo->apsd_hpkt_timer);
#endif // endif

	if (AP_ENAB(wlc->pub) || (BSS_TDLS_ENAB(wlc, SCB_BSSCFG(remove)) && SCB_PS(remove))) {
		uint8 ps = (wlc->block_datafifo & DATA_BLOCK_PS) ?
			PS_SWITCH_OFF : PS_SWITCH_FIFO_FLUSHED;
		if (!SCB_ISMULTI(remove)) {
			struct apps_bss_info *bss_info =
				APPS_BSSCFG_CUBBY(wlc->psinfo, SCB_BSSCFG(remove));
			if (bss_info &&
				(bss_info->ps_trans_status & BSS_PS_TRANS_OFF_BLOCKED)) {
				WL_ERROR(("wl%d.%d: "MACF" deinited."
					" bcmc_scb PS off blocked. PS %d\n",
					wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(remove)),
					ETHER_TO_MACF(remove->ea), SCB_PS(remove)));
			}
		}
		if (SCB_PS(remove) && !SCB_ISMULTI(remove))
			wlc_apps_scb_ps_off(wlc, remove, TRUE);
		else if (!pktq_empty(&scb_psinfo->psq))
			wlc_apps_ps_flush(wlc, remove);
		wlc_bmac_process_ps_switch(wlc->hw, &remove->ea, ps | PS_SWITCH_STA_REMOVED,
			&(scb_psinfo->auxpmq_idx));
	}
}

#ifdef PROP_TXSTATUS
#ifdef PROP_TXSTATUS_DEBUG
void
wlfc_display_debug_info(void* _wlc, int hi, int lo)
{
	wlc_info_t* wlc = (wlc_info_t*)_wlc;
	struct scb *scb;
	struct scb_iter scbiter;
	struct apps_bss_info *bss_info;
	struct apps_scb_psinfo *scb_psinfo;
	char* pvb;

	printf("WLC discards:%d, ps_deferred:%d, MAC_handle_bitmap:0x%08x\n",
		wlc->psinfo->ps_discard,
		wlc->psinfo->ps_deferred,
		((wlfc_mac_desc_handle_map_t*)wlc->wlfc_data)->bitmap);

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		scb_psinfo = ((struct apps_scb_psinfo *)SCB_CUBBY(scb, wlc->psinfo->scb_handle));
		printf("scb at %p for [%02x:%02x:%02x:%02x:%02x:%02x], handle:0x%02x, t_idx:%d\n",
			scb,
			scb->ea.octet[0], scb->ea.octet[1], scb->ea.octet[2],
			scb->ea.octet[3], scb->ea.octet[4], scb->ea.octet[5],
			scb->mac_address_handle,
			WLFC_MAC_DESC_GET_LOOKUP_INDEX(scb->mac_address_handle));
		printf("  (psq_items,state, ta_bmp)=(%d,%s,0x%x), psq_bucket: %d items\n",
			scb_psinfo->psq.len,
			((scb->PS == FALSE) ? " OPEN" : "CLOSE"), SCB_PROPTXTSTATUS_TIM(scb),
			scb_psinfo->psq.len);
		bss_info = APPS_BSSCFG_CUBBY(wlc->psinfo, scb->bsscfg);

		pvb = bss_info->pvb;
		printf("pvb: [%02x-%02x-%02x-%02x]\n", pvb[0], pvb[1], pvb[2], pvb[3]);
	}
	if (hi) {
		printf("setting psq (hi,lo) to (%d,%d)\n", hi, lo);
		wlc->psinfo->psq_pkts_hi = hi;
		wlc->psinfo->psq_pkts_lo = lo;
	}
	else
		printf("leaving psq (hi,lo) as is (%d,%d)\n",
		wlc->psinfo->psq_pkts_hi, wlc->psinfo->psq_pkts_lo);
	return;
}
#endif /* PROP_TXSTATUS_DEBUG */

static int
wlc_apps_push_wlfc_psmode_update(wlc_info_t *wlc, uint8 mac_handle, uint8 open_close)
{
	int rc = BCME_OK;
	/* space for type(1), length(1) and value */
	uint8	results[1+1+WLFC_CTL_VALUE_LEN_MAC];

	results[0] = open_close;
	results[1] = WLFC_CTL_VALUE_LEN_MAC;
	results[2] = mac_handle;
	if ((rc = wlfc_push_signal_data(wlc->wl, results, sizeof(results), FALSE)) == BCME_OK)
		rc = wlfc_sendup_ctl_info_now(wlc->wl);
	return rc;
}
#endif /* PROP_TXSTATUS */

static void
wlc_apps_apsd_usp_end(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

#ifdef WLTDLS
	if (BSS_TDLS_ENAB(wlc, scb->bsscfg)) {
		if (wlc_tdls_in_pti_interval(wlc->tdls, scb))
			return;
		wlc_tdls_apsd_usp_end(wlc->tdls, scb);
	}
#endif /* WLTDLS */

	scb_psinfo->apsd_usp = FALSE;

#ifdef WLTDLS
	if (BSS_TDLS_ENAB(wlc, scb->bsscfg) &&
		(wlc_apps_apsd_delv_count(wlc, scb) > 0)) {
		/* send PTI again */
		wlc_tdls_send_pti(wlc->tdls, scb);
	}
#endif /* WLTDLS */

}

/* This routine deals with all PS transitions from ON->OFF */
void
wlc_apps_scb_ps_off(wlc_info_t *wlc, struct scb *scb, bool discard)
{
	apps_wlc_psinfo_t *wlc_psinfo = wlc->psinfo;
	struct apps_scb_psinfo *scb_psinfo;
	struct scb *bcmc_scb;
	apps_bss_info_t *bss_info;
	wlc_bsscfg_t *bsscfg;
	struct ether_addr ea;

	/* sanity */
	ASSERT(scb);

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	bss_info = APPS_BSSCFG_CUBBY(wlc->psinfo, bsscfg);
	ASSERT(bss_info->ps_nodes);

	/* process ON -> OFF PS transition */
	WL_PS(("wl%d.%d: wlc_apps_scb_ps_off, "MACF" aid %d\n",
		wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
		ETHER_TO_MACF(scb->ea), AID2PVBMAP(scb->aid)));

#if defined(BCMDBG) && defined(PSPRETEND)
	if (SCB_PS_PRETEND(scb)) {
		uint32 time_in_pretend = R_REG(wlc->osh, &wlc->regs->tsf_timerlow) -
		                         scb->ps_pretend_start;
		scb->ps_pretend_total_time_in_pps += time_in_pretend;
		WL_PS(("wl%d.%d: ps pretend state about to exit, %d ms in pretend state\n",
		        wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), (time_in_pretend + 500)/1000));
	}
#endif /* PSPRETEND */

	/* update PS state info */
	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	/* if already in this process but came here due to pkt callback then
	 * just return.
	 */
	if ((scb_psinfo->ps_trans_status & SCB_PS_TRANS_OFF_IN_PROG))
		return;
	scb_psinfo->ps_trans_status |= SCB_PS_TRANS_OFF_IN_PROG;

	bss_info->ps_nodes--;
	wlc_psinfo->ps_nodes_all--;
	scb->PS = FALSE;
#ifdef PSPRETEND
	scb->ps_pretend &= ~PS_PRETEND_ON;
	scb->ps_pretend_failed_ack_count = 0;
#endif // endif
#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		wlc_apps_push_wlfc_psmode_update(wlc, scb->mac_address_handle,
			WLFC_CTL_TYPE_MAC_OPEN);
		/* reset the traffic availability map */
		SCB_PROPTXTSTATUS_SETTIM(scb, 0);
		SCB_PROPTXTSTATUS_SETPKTWAITING(scb, 0);
#ifdef WLAMPDU
		/* We might have suppressed pkts during Power-Save ON */
		/* Reset AMPDU Seqcnt with a BAR */
		if (AMPDU_ENAB(wlc->pub)) {
			struct scb_iter scbiter;
			struct scb *scb_i = NULL;
			FOREACHSCB(wlc->scbstate, &scbiter, scb_i) {
				if (scb_i->flags & SCB_PENDING_FREE)
					continue;

				if (scb_i->bsscfg == scb->bsscfg) {
					wlc_ampdu_send_bar_cfg(wlc->ampdu_tx,
						scb_i);
				}
			}
		}
#endif /* AMPDU */
	}
#endif /* PROP_TXSTATUS */

	/* Unconfigure the APPS from the txpath */
	wlc_txmod_unconfig(wlc, scb, TXMOD_APPS);

	/* If this is last STA to leave PS mode,
	 * trigger BCMC FIFO drain and
	 * set BCMC traffic to go through regular fifo
	 */
	if (bss_info->ps_nodes == 0 && !(bsscfg->flags & WLC_BSSCFG_NOBCMC) &&
		!(bss_info->ps_trans_status & BSS_PS_TRANS_OFF_BLOCKED)) {
		WL_PS(("wl%d.%d: wlc_apps_scb_ps_off - bcmc off\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
		/*  Use the bsscfg pointer of this scb to help us locate the
		 *  correct bcmc_scb so that we can turn off PS
		 */
		bcmc_scb = WLC_BCMCSCB_GET(wlc, bsscfg);
		ASSERT(bcmc_scb->bsscfg == bsscfg);

		if (MBSS_ENAB(wlc->pub)) { /* MBSS PS handling is a bit more complicated. */
			wlc_apps_bss_ps_off_start(wlc, bcmc_scb);
		} else
#if defined(WLAIBSS) && !defined(WLAIBSS_DISABLED)
		if (!BSSCFG_IBSS(bsscfg) || !AIBSS_ENAB(wlc->pub))
#endif /* WLAIBSS && !WLAIBSS_DISABLED */
		{
			bcmc_scb->PS = FALSE;
#ifdef PSPRETEND
			bcmc_scb->ps_pretend &= ~PS_PRETEND_ON;
#endif // endif
			/* If packets are pending in TX_BCMC_FIFO,
			 * then ask ucode to transmit them immediately
			 */
			if (TXPKTPENDGET(wlc, TX_BCMC_FIFO) && !wlc->bcmcfifo_drain) {
				wlc->bcmcfifo_drain = TRUE;
				wlc_mhf(wlc, MHF2, MHF2_TXBCMC_NOW, MHF2_TXBCMC_NOW, WLC_BAND_AUTO);
			}
		}
	} else if ((bss_info->ps_trans_status & BSS_PS_TRANS_OFF_BLOCKED)) {
		WL_PS(("wl%d.%d: wlc_apps_scb_ps_off - bcmc off BLOCKED! %d\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), bss_info->ps_nodes));
	}

	scb_psinfo->psp_pending = FALSE;
	scb_psinfo->first_suppr_handled = FALSE;
	wlc_apps_apsd_usp_end(wlc, scb);
	scb_psinfo->apsd_cnt = 0;

	/* save ea before calling wlc_apps_ps_flush */
	ea = scb->ea;

	/* Note: We do not clear up any pending PS-POLL pkts
	 * which may be enq'd with the IGNOREPMQ bit set. The
	 * relevant STA should stay awake until it rx's these
	 * response pkts
	 */

	/* Move pmq entries to Q1 (ctl) for immediate tx */
	if (discard == FALSE)
		while (wlc_apps_ps_send(wlc, scb, WLC_PREC_BMP_ALL, 0))
			;
	else /* free any pending frames */
		wlc_apps_ps_flush(wlc, scb);

	/* Check if the PVB entry needs to be cleared */
	if (scb_psinfo->in_pvb)
		wlc_apps_pvb_update(wlc, scb);

	/* callbacks in wlc_apps_ps_flush may have freed scb */
	if (!ETHER_ISMULTI(&ea) && (wlc_scbfind(wlc, bsscfg, &ea) == NULL)) {
		WL_PS(("wl%d: %s: exiting, scb for "MACF" was freed",
			wlc->pub->unit, __FUNCTION__, ETHER_TO_MACF(scb->ea)));
		return;
	}
	scb_psinfo->ps_trans_status &= ~SCB_PS_TRANS_OFF_IN_PROG;
#ifdef WL_BEAMFORMING
	if (TXBF_ENAB(wlc->pub)) {
		/* Notify txbf module of the scb's PS change */
		wlc_txbf_scb_ps_notify(wlc->txbf, scb, FALSE);
	}
#endif /* WL_BEAMFORMING */

#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		wlc_check_txq_fc(wlc, SCB_WLCIFP(scb)->qi);
#ifdef WLAMPDU
		if (AMPDU_ENAB(wlc->pub)) {
			wlc_check_ampdu_fc(wlc->ampdu_tx, scb);
		}
#endif /* WLAMPDU */
#ifdef BCMPCIEDEV
		if (BCMPCIEDEV_ENAB()) {
			scb_psinfo->in_transit = 0;
		}
#endif // endif
	}
#endif /* PROP_TXSTATUS */
	if (scb_psinfo->change_scb_state_to_auth) {
		wlc_scb_resetstate(scb);
		wlc_scb_setstatebit(scb, AUTHENTICATED);

		wlc_bss_mac_event(wlc, bsscfg, WLC_E_DISASSOC_IND, &scb->ea,
			WLC_E_STATUS_SUCCESS, DOT11_RC_BUSY, 0, NULL, 0);
		scb_psinfo->change_scb_state_to_auth = FALSE;
	}
}

static void
wlc_apps_bcmc_scb_ps_on(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bool pmq_suppr_bcmc_pkt)
{
	apps_bss_info_t *bss_info = APPS_BSSCFG_CUBBY(wlc->psinfo, bsscfg);
	struct scb *bcmc_scb;
	/*  Use the bsscfg pointer of this scb to help us locate the
	 *  correct bcmc_scb so that we can turn on PS
	 */
	bcmc_scb = WLC_BCMCSCB_GET(wlc, bsscfg);
	ASSERT(bcmc_scb->bsscfg == bsscfg);

	if (pmq_suppr_bcmc_pkt) {
		bss_info->ps_trans_status |= BSS_PS_TRANS_OFF_BLOCKED;
		WL_PS(("wl%d.%d: %s: Req by bcmc suppr pkt!\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
	} else if (SCB_PS(bcmc_scb)) {
		WL_PS(("wl%d.%d: %s: [bcmc_scb] Already in PS!\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
		/* ps on had already been processed by bcmc pmq suppressed pkt,
		 * and now we are here upon successive unicast pmq suppressed pkt
		 * or actual pmq entry addition interrupt.
		 * Therefore do nothing here and just get out.
		 */
		return;
	}
#if defined(MBSS) && (defined(BCMDBG) || defined(WLMSG_PS))
	if (MBSS_ENAB(wlc->pub)) {
		uint32 mc_fifo_pkts = wlc_mbss_get_bcmc_pkts_sent(wlc, bsscfg);

		if (mc_fifo_pkts != 0) {
			WL_PS(("wl%d.%d: START PS-ON; bcmc %d\n", wlc->pub->unit,
				WLC_BSSCFG_IDX(bsscfg), mc_fifo_pkts));
		}
	}
#endif /* MBSS && (BCMDBG || WLMSG_PS) */
#if defined(WLAIBSS) && !defined(WLAIBSS_DISABLED)
	if (!BSSCFG_IBSS(bsscfg) || !AIBSS_ENAB(wlc->pub))
#endif /* WLAIBSS && WLAIBSS_DISABLED */
	{
		WL_PS(("wl%d.%d: %s: bcmc SCB PS on!\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(bsscfg), __FUNCTION__));
		bcmc_scb->PS = TRUE;
		if (wlc->bcmcfifo_drain) {
			wlc->bcmcfifo_drain = FALSE;
			wlc_mhf(wlc, MHF2, MHF2_TXBCMC_NOW, 0, WLC_BAND_AUTO);
		}
	}
}

/* This deals with all PS transitions from OFF->ON */
void
wlc_apps_scb_ps_on(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;
	apps_wlc_psinfo_t *wlc_psinfo;
	apps_bss_info_t *bss_info;
	wlc_bsscfg_t *bsscfg;

	ASSERT(scb);

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	if (BSSCFG_STA(bsscfg) && !BSS_TDLS_ENAB(wlc, bsscfg) && !BSSCFG_IBSS(bsscfg)) {
		WL_PS(("wlc_apps_scb_ps_on, "MACF" aid %d: BSSCFG_STA(bsscfg)=%s, "
			"BSS_TDLS_ENAB(wlc,bsscfg)=%s\n\n",
			ETHER_TO_MACF(scb->ea), AID2PVBMAP(scb->aid),
			BSSCFG_STA(bsscfg) ? "TRUE" : "FALSE",
			BSS_TDLS_ENAB(wlc, bsscfg) ? "TRUE" : "FALSE"));
		return;
	}

	/* process OFF -> ON PS transition */
	WL_PS(("wl%d.%d wlc_apps_scb_ps_on, "MACF" aid %d, pkts pending %d\n", wlc->pub->unit,
	        WLC_BSSCFG_IDX(bsscfg), ETHER_TO_MACF(scb->ea),
	        AID2PVBMAP(scb->aid), TXPKTPENDTOT(wlc)));

	wlc_psinfo = wlc->psinfo;
	bss_info = APPS_BSSCFG_CUBBY(wlc_psinfo, scb->bsscfg);

	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);
	scb_psinfo->first_suppr_handled = FALSE;

	/* update PS state info */
	bss_info->ps_nodes++;
	wlc_psinfo->ps_nodes_all++;
	scb->PS = TRUE;
	scb->tbtt = 0;
	scb_psinfo->ps_trans_status &= ~SCB_PS_TRANS_OFF_PEND;

#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		uint8 vqdepth = 0;
#ifdef WLTDLS
		if (BSS_TDLS_ENAB(wlc, bsscfg)) {
			if ((scb->apsd.maxsplen == WLC_APSD_USP_UNB) || (scb->apsd.maxsplen == 0))
				vqdepth = TDLS_WLFC_DEFAULT_FWQ_DEPTH;
			else
				vqdepth = scb->apsd.maxsplen;
		}
#else
		vqdepth = wlc->wlfc_vqdepth;
#endif // endif

		wlc_apps_push_wlfc_psmode_update(wlc, scb->mac_address_handle,
			WLFC_CTL_TYPE_MAC_CLOSE);
		/* Leaving bitmap open for all ACs for now */
		if (WLFC_CONTROL_SIGNALS_TO_HOST_ENAB(wlc->pub) && (wlc->wlfc_vqdepth > 0))
			wlfc_psmode_request(wlc->wl, scb->mac_address_handle, vqdepth,
			0xff, WLFC_CTL_TYPE_MAC_REQUEST_CREDIT);

		/* TODO: Suppress pkts of this scb to host */
	}
#endif	/* PROP_TXSTATUS */

	/* If this is first STA to enter PS mode, set BCMC traffic
	 * to go through BCMC Fifo. If bcmcfifo_drain is set, then clear
	 * the drain bit.
	 */
	if (bss_info->ps_nodes == 1 && !(bsscfg->flags & WLC_BSSCFG_NOBCMC)) {
		wlc_apps_bcmc_scb_ps_on(wlc, SCB_BSSCFG(scb), FALSE);
	}

	/* Add the APPS to the txpath for this SCB */
	wlc_txmod_config(wlc, scb, TXMOD_APPS);

#ifdef PROP_TXSTATUS
	/* suppress tx fifo first */
	wlc_txfifo_suppress(wlc, scb);
#ifdef BCMPCIEDEV
	if (BCMPCIEDEV_ENAB()) {
		scb_psinfo->in_transit = 0;
	}
#endif // endif
#endif /* BCMPCIEDEV */

	/* ps enQ any pkts on the txq, narq, ampduq */
	wlc_apps_txq_to_psq(wlc, scb);

#ifdef PROP_TXSTATUS
#ifdef WLNAR
	wlc_apps_nar_txq_to_psq(wlc, scb);
#endif /* WLNAR */
#ifdef WLAMPDU
	/* This causes problems for PSPRETEND */
	wlc_apps_ampdu_txq_to_psq(wlc, scb);
#endif /* WLAMPDU */
#endif /* PROP_TXSTATUS */

#if defined(WLAIBSS) && !defined(WLAIBSS_DISABLED)
	if (!BSSCFG_IBSS(bsscfg) || !AIBSS_ENAB(wlc->pub))
#endif /* WLAIBSS && !WLAIBSS_DISABLED */
	{
		/* If there is anything in the data fifo then allow it to drain */
		if (TXPKTPENDTOT(wlc) > 0)
			wlc_block_datafifo(wlc, DATA_BLOCK_PS, DATA_BLOCK_PS);
	}
#ifdef WL_BEAMFORMING
	if (TXBF_ENAB(wlc->pub)) {
		/* Notify txbf module of the scb's PS change */
		wlc_txbf_scb_ps_notify(wlc->txbf, scb, TRUE);
	}
#endif /* WL_BEAMFORMING */
}

/* "normalize" a packet queue - move packets tagged with WLF3_SUPR flag
 * to the front and retain the order in case other packets were inserted
 * in the queue before.
 */
static void
wlc_pktq_supr_norm(wlc_info_t *wlc, struct pktq *pktq)
{
	struct spktq scratch;
	void *pkt;
	int prec;

	if (pktq_len(pktq) == 0)
		return;

	pktqinit(&scratch, pktq_len(pktq));

	PKTQ_PREC_ITER(pktq, prec) {
		void *head_pkt = pktq_ppeek(pktq, prec);

		while ((pkt = pktq_pdeq_tail(pktq, prec)) != NULL) {
			if (WLPKTTAG(pkt)->flags3 & WLF3_SUPR) {
				WLPKTTAG(pkt)->flags3 &= ~WLF3_SUPR;
				pktenq_head(&scratch, pkt);
			}
			else {
				pktq_penq_head(pktq, prec, pkt);
			}
			if (pkt == head_pkt)
				break;
		}

		if (pktq_len(&scratch) == 0)
			continue;

		while ((pkt = pktdeq_tail(&scratch)) != NULL) {
			pktq_penq_head(pktq, prec, pkt);
		}

		ASSERT(pktq_empty(&scratch));
	}
}

/* "normalize" the SCB's PS queue - move packets tagged with WLF3_SUPR flag
 * to the front and retain the order in case other packets were inserted
 * in the queue before.
 */
void
wlc_apps_scb_psq_norm(wlc_info_t *wlc, struct scb *scb)
{
	apps_wlc_psinfo_t *wlc_psinfo;
	struct apps_scb_psinfo *scb_psinfo;
	struct pktq *pktq;

	wlc_psinfo = wlc->psinfo;
	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);

	pktq = &scb_psinfo->psq;

	wlc_pktq_supr_norm(wlc, pktq);
}

/* "normalize" the BSS's txq queue - move packets tagged with WLF3_SUPR flag
 * to the front and retain the order in case other packets were inserted
 * in the queue before.
 */
static void
wlc_apps_bsscfg_txq_norm(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	struct pktq *txq = WLC_GET_TXQ(cfg->wlcif->qi);

	wlc_pktq_supr_norm(wlc, txq);
}

/* Process any pending PS states */
void
wlc_apps_process_pend_ps(wlc_info_t *wlc)
{
	struct scb_iter scbiter;
	struct scb *scb;
	int txpktpendtot = TXPKTPENDTOT(wlc);

	/* PMQ todo : we should keep track of pkt pending for each scb and wait for
	   individual drains, instead of blocking and draining the whole pipe.
	*/

	if ((wlc->block_datafifo & DATA_BLOCK_PS) && txpktpendtot == 0) {
		int idx;
		wlc_bsscfg_t *cfg;
		WL_PS(("wlc_apps_process_pend_ps unblocking fifo\n"));
		wlc_block_datafifo(wlc, DATA_BLOCK_PS, 0);
		/* notify bmac to clear the PMQ */
		wlc_bmac_process_ps_switch(wlc->hw, NULL,
			(PS_SWITCH_FIFO_FLUSHED | PS_SWITCH_MAC_INVALID), NULL);
		FOREACH_BSS(wlc, idx, cfg) {
			scb = WLC_BCMCSCB_GET(wlc, cfg);
			if (scb && SCB_PS(scb)) {
				WL_PS(("wl%d.%d wlc_apps_process_pend_ps: Normalizing bcmc PSQ"
					" of BSSID "MACF"\n",
					wlc->pub->unit, idx, ETHER_TO_MACF(cfg->BSSID)));
				wlc_apps_bsscfg_txq_norm(wlc, cfg);
			}
		}
		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			WL_PS(("wl%d.%d wlc_apps_process_pend_ps: Normalizing PSQ for STA "MACF"\n",
			       wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
			       ETHER_TO_MACF(scb->ea)));
			if (SCB_PS(scb) && !SCB_ISMULTI(scb)) {
				struct apps_scb_psinfo *scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

				wlc_apps_scb_psq_norm(wlc, scb);
				if ((scb_psinfo->ps_trans_status & SCB_PS_TRANS_OFF_PEND)) {
#ifdef PSPRETEND
					if (SCB_PS_PRETEND_BLOCKED(scb)) {
						WL_ERROR(("wl%d.%d: %s: SCB_PS_PRETEND_BLOCKED, "
						"expected to see PMQ PPS entry\n", wlc->pub->unit,
						WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
					}
#endif // endif
					WL_PS(("wl%d.%d wlc_apps_process_pend_ps: Allowing PS Off"
						" for STA "MACF"\n", wlc->pub->unit,
						WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
						ETHER_TO_MACF(scb->ea)));
					wlc_apps_scb_ps_off(wlc, scb, FALSE);
				}
			}
#ifdef PSPRETEND
			if (SCB_PS_PRETEND_PROBING(scb)) {
				uint32 elapsed = (R_REG(wlc->osh, &wlc->regs->tsf_timerlow) -
				                          scb->ps_pretend_start)/1000;
				wlc_ap_do_pspretend_probe(wlc, scb, elapsed);
			}
#endif // endif
		}

		if (MBSS_ENAB(wlc->pub)) {
			wlc_apps_bss_ps_on_done(wlc);
		}
		/* If any suppressed BCMC packets at the head of txq,
		 * they need to be sent to hw fifo right now.
		 */
		if (wlc->active_queue != NULL && WLC_TXQ_OCCUPIED(wlc)) {
			wlc_send_q(wlc, wlc->active_queue);
		}
	}
}

/* Free any pending PS packets for this STA */
void
wlc_apps_ps_flush(wlc_info_t *wlc, struct scb *scb)
{
#if defined(BCMDBG) || defined(WLMSG_PS)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	void *pkt;
	struct ether_addr ea;
	struct apps_scb_psinfo *scb_psinfo;
	apps_wlc_psinfo_t *wlc_psinfo;
	wlc_bsscfg_t *bsscfg;

	ASSERT(scb);
	ASSERT(wlc);

	/* save ea and bsscfg before call wlc_pkt_flush */
	ea = scb->ea;
	bsscfg = scb->bsscfg;
	wlc_psinfo = wlc->psinfo;
	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);

	WL_PS(("wl%d.%d: wlc_apps_ps_flush: flushing %d packets for %s aid %d\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), pktq_len(&scb_psinfo->psq),
	       bcm_ether_ntoa(&scb->ea, eabuf), AID2PVBMAP(scb->aid)));

	/* Don't care about dequeue precedence */
	while ((pkt = pktq_deq(&scb_psinfo->psq, NULL))) {
		if (!SCB_ISMULTI(scb))
			wlc_psinfo->ps_deferred--;
		WLPKTTAG(pkt)->flags &= ~WLF_PSMARK; /* clear the timestamp */
		/* reclaim callbacks and free */
		PKTFREE(wlc->osh, pkt, TRUE);

		/* callback may have freed scb */
		if (!ETHER_ISMULTI(&ea) && (wlc_scbfind(wlc, bsscfg, &ea) == NULL)) {
			WL_PS(("wl%d.%d: wlc_apps_ps_flush: exiting, scb for %s was freed",
				wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
				bcm_ether_ntoa(&ea, eabuf)));
			return;
		}
	}

	ASSERT(pktq_empty(&scb_psinfo->psq));

	/* If there is a valid aid (the bcmc scb wont have one) then ensure
	 * the PVB is cleared.
	 */
	if (scb->aid && scb_psinfo->in_pvb)
		wlc_apps_pvb_update(wlc, scb);
}

#ifdef PROP_TXSTATUS
void
wlc_apps_ps_flush_mchan(wlc_info_t *wlc, struct scb *scb)
{
	void *pkt;
	int prec;
	struct apps_scb_psinfo *scb_psinfo;
	apps_wlc_psinfo_t *wlc_psinfo;
	struct pktq tmp_q;

	ASSERT(scb);
	ASSERT(wlc);

	wlc_psinfo = wlc->psinfo;
	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);
	pktq_init(&tmp_q, WLC_PREC_COUNT, PKTQ_LEN_DEFAULT);

	/* Don't care about dequeue precedence */
	while ((pkt = pktq_deq(&scb_psinfo->psq, &prec))) {
		if (!SCB_ISMULTI(scb))
			wlc_psinfo->ps_deferred--;

		if (!(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(pkt)->wl_hdr_information) &
			WLFC_PKTFLAG_PKTFROMHOST)) {
			pktq_penq(&tmp_q, prec, pkt);
			continue;
		}

		WLPKTTAG(pkt)->flags &= ~WLF_PSMARK; /* clear the timestamp */

		wlc_suppress_sync_fsm(wlc, scb, pkt, TRUE);
		wlc_process_wlhdr_txstatus(wlc, WLFC_CTL_PKTFLAG_WLSUPPRESS, pkt, FALSE);
		PKTFREE(wlc->osh, pkt, TRUE);
	}

	/* Enqueue back the frames generated in dongle */
	while ((pkt = pktq_deq(&tmp_q, &prec))) {
		pktq_penq(&scb_psinfo->psq, prec, pkt);
	}

	/* If there is a valid aid (the bcmc scb wont have one) then ensure
	* the PVB is cleared.
	*/
	if (scb->aid && scb_psinfo->in_pvb)
		wlc_apps_pvb_update(wlc, scb);
}
#endif /* defined(PROP_TXSTATUS) && defined(WLMCHAN) */

/* Return TRUE if packet has been enqueued on a ps queue, FALSE otherwise */
#define WLC_PS_APSD_HPKT_TIME 12 /* in ms */

bool
wlc_apps_psq(wlc_info_t *wlc, void *pkt, int prec)
{
#if defined(BCMDBG) || defined(WLMSG_PS)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	apps_wlc_psinfo_t *wlc_psinfo;
	struct apps_scb_psinfo *scb_psinfo;
	struct scb *scb;
	int psq_len;
	int psq_len_diff;

	scb = WLPKTTAGSCBGET(pkt);
	ASSERT(SCB_PS(scb));
	ASSERT(wlc);

	/* Do not enq bcmc pkts on a psq, also
	 * ageing out packets may have disassociated the STA, so return FALSE if so
	 * unless scb is wds
	 */
	if (!SCB_ASSOCIATED(scb) && !BSSCFG_IBSS(SCB_BSSCFG(scb)) && !SCB_WDS(scb)) {
		return FALSE;
	}

	ASSERT(!SCB_ISMULTI(scb));

	wlc_psinfo = wlc->psinfo;
	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);

#if defined(PROP_TXSTATUS)
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		/* If the host sets a flag marking the packet as "in response to
		   credit request for pspoll" then only the fimrware enqueues it.
		   Otherwise wlc drops it by sending a wlc_suppress.
		*/
		if ((WL_TXSTATUS_GET_FLAGS(WLPKTTAG(pkt)->wl_hdr_information) &
			WLFC_PKTFLAG_PKTFROMHOST) &&
			HOST_PROPTXSTATUS_ACTIVATED(wlc) &&
			(!(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(pkt)->wl_hdr_information) &
			WLFC_PKTFLAG_PKT_REQUESTED))) {
			WLFC_DBGMESG(("R[%d]\n", (WLPKTTAG(pkt)->wl_hdr_information & 0xff)));
			return FALSE;
		}
	}
#endif /* PROP_TXSTATUS */

	psq_len = pktq_len(&scb_psinfo->psq);
	/* Deferred PS pkt flow control
	 * If this scb currently contains less than the minimum number of PS pkts
	 * per scb then enq it. If the total number of PS enq'd pkts exceeds the
	 * watermark and more than the minimum number of pkts are already enq'd
	 * for this STA then do not enq the pkt.
	 */
#ifdef PROP_TXSTATUS
	if (!PROP_TXSTATUS_ENAB(wlc->pub) || !HOST_PROPTXSTATUS_ACTIVATED(wlc))
#endif // endif
	{
	if (psq_len > (int)wlc_psinfo->psq_pkts_lo &&
	    wlc_psinfo->ps_deferred > wlc_psinfo->psq_pkts_hi) {
		WL_PS(("wl%d.%d: wlc_apps_psq: can't buffer packet for %s aid %d, %d "
		       "queued for scb, %d for WL\n", wlc->pub->unit,
		       WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), bcm_ether_ntoa(&scb->ea, eabuf),
		       AID2PVBMAP(scb->aid), pktq_len(&scb_psinfo->psq),
		       wlc_psinfo->ps_deferred));
		return FALSE;
	}
	}

	WL_PS(("wl%d.%d:%s(): enq to PSQ, prec = 0x%x, scb_psinfo->apsd_usp = %s\n", wlc->pub->unit,
		WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__, prec,
		scb_psinfo->apsd_usp ? "TRUE" : "FALSE"));

	if (!wlc_prec_enq(wlc, &scb_psinfo->psq, pkt, prec)) {
		WL_PS(("wl%d.%d:%s(): wlc_prec_enq() failed\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
		return FALSE;
	}

	/* increment total count of PS pkts enqueued in WL driver */
	psq_len_diff = pktq_len(&scb_psinfo->psq) - psq_len;
	if (psq_len_diff == 1)
		wlc_psinfo->ps_deferred++;
	else if (psq_len_diff == 0)
		WL_PS(("wl%d.%d:%s(): wlc_prec_enq() dropped pkt.\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
	else
		ASSERT(0);

#ifdef WLTDLS
	if (BSS_TDLS_ENAB(wlc, scb->bsscfg)) {
		if (!scb_psinfo->apsd_usp)
			wlc_tdls_send_pti(wlc->tdls, scb);
		else if (wlc_tdls_in_pti_interval(wlc->tdls, scb)) {
			scb_psinfo->apsd_cnt = wlc_apps_apsd_delv_count(wlc, scb);
			if (scb_psinfo->apsd_cnt)
				wlc_apps_apsd_send(wlc, scb);
			return TRUE;
		}
	}
#endif /* WLTDLS */

	/* Check if the PVB entry needs to be set */
	if (scb->aid && !scb_psinfo->in_pvb)
		wlc_apps_pvb_update(wlc, scb);

#ifdef PROP_TXSTATUS
	if (PROP_TXSTATUS_ENAB(wlc->pub) && SCB_PROPTXTSTATUS_PKTWAITING(scb)) {
		uint16 fc = 0;
		if (SCB_PROPTXTSTATUS_POLLRETRY(scb)) {
			fc |= FC_RETRY;
			SCB_PROPTXTSTATUS_SETPOLLRETRY(scb, 0);
		}
		if (HOST_PROPTXSTATUS_ACTIVATED(wlc))
			wlc_apps_send_psp_response(wlc, scb, fc);
	}
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {

		if (scb_psinfo->apsd_hpkt_timer_on) {

			wl_del_timer(wlc->wl, scb_psinfo->apsd_hpkt_timer);

			if (!scb_psinfo->apsd_tx_pending && scb_psinfo->apsd_usp &&
				scb_psinfo->apsd_cnt) {
				wlc_apps_apsd_send(wlc, scb);

				if (scb_psinfo->apsd_cnt > 1 &&
					wlc_apps_apsd_delv_count(wlc, scb) == 1) {
					ac_bitmap_t ac_to_request;
					ac_to_request = scb->apsd.ac_delv & AC_BITMAP_ALL;
					wlfc_psmode_request(wlc->wl, scb->mac_address_handle,
						1, ac_to_request, WLFC_CTL_TYPE_MAC_REQUEST_PACKET);
					wl_add_timer(wlc->wl, scb_psinfo->apsd_hpkt_timer,
						WLC_PS_APSD_HPKT_TIME, FALSE);
				} else
					scb_psinfo->apsd_hpkt_timer_on = FALSE;
			} else
				scb_psinfo->apsd_hpkt_timer_on = FALSE;

		}
	}
#endif /* PROP_TXSTATUS */
	return (TRUE);
}

/*
 * Move a PS-buffered packet to the txq and send the txq.
 * Returns TRUE if a packet was available to dequeue and send.
 * extra_flags are added to packet flags (for SDU, only to last MPDU)
 */
static bool
wlc_apps_ps_send(wlc_info_t *wlc, struct scb *scb, uint prec_bmp, uint32 extra_flags)
{
	void *pkt = NULL;
	struct apps_scb_psinfo *scb_psinfo;
	apps_wlc_psinfo_t *wlc_psinfo;
	int prec;
	struct pktq *psq;

	ASSERT(wlc);
	wlc_psinfo = wlc->psinfo;

	ASSERT(scb);
	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);

	psq = &scb_psinfo->psq;

	WL_PS(("wl%d.%d:%s\n", wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
	/* Dequeue the packet with highest precedence out of a given set of precedences */
	if (!(pkt = pktq_mdeq(psq, prec_bmp, &prec)))
		return FALSE;		/* no traffic to send */

	/*
	 * If it's the first MPDU in a series of suppressed MPDUs that make up an SDU,
	 * enqueue all of them together before calling wlc_send_q.
	 */
	/*
	 * It's possible that hardware resources may be available for
	 * one fragment but not for another (momentarily).
	 */
	if (WLPKTTAG(pkt)->flags & WLF_TXHDR) {
		struct dot11_header *h;
		uint tsoHdrSize = 0;
		void *next_pkt;
		uint seq_num, next_seq_num;
		bool control;

#ifdef WLTOEHW
		tsoHdrSize = (wlc->toe_bypass ?
			0 : wlc_tso_hdr_length((d11ac_tso_t*)PKTDATA(wlc->osh, pkt)));
#endif // endif
		h = (struct dot11_header *)
			(PKTDATA(wlc->osh, pkt) + tsoHdrSize + D11_TXH_LEN_EX(wlc));
		control = FC_TYPE(ltoh16(h->fc)) == FC_TYPE_CTL;

		/* Control frames does not have seq field; directly queue
		 * them.
		 */
		if (!control) {
			seq_num = ltoh16(h->seq) >> SEQNUM_SHIFT;

			while ((next_pkt = pktq_ppeek(psq, prec)) != NULL) {
				/* Stop if different SDU */
				if (!(WLPKTTAG(next_pkt)->flags & WLF_TXHDR))
					break;

				/* Stop if different sequence number */
#ifdef WLTOEHW
				tsoHdrSize = (wlc->toe_bypass ? 0 :
					wlc_tso_hdr_length((d11ac_tso_t*)
						PKTDATA(wlc->osh, next_pkt)));
#endif // endif
				h = (struct dot11_header *) (PKTDATA(wlc->osh, next_pkt) +
					tsoHdrSize + D11_TXH_LEN_EX(wlc));
				control = FC_TYPE(ltoh16(h->fc)) == FC_TYPE_CTL;

				/* stop if different ft; control frames does
				 * not have sequence control.
				 */
				if (control)
					break;

				next_seq_num = ltoh16(h->seq) >> SEQNUM_SHIFT;
				if (next_seq_num != seq_num)
					break;

				/* Enqueue the PS-Poll response at higher precedence level */
				wlc_apps_ps_enq_resp(wlc, scb, pkt,
					WLC_PRIO_TO_HI_PREC(PKTPRIO(pkt)));

				/* Dequeue the peeked packet */
				pkt = pktq_pdeq(psq, prec);
				ASSERT(pkt == next_pkt);
			}
		}
	}

	/* Set additional flags on SDU or on final MPDU */
	WLPKTTAG(pkt)->flags |= extra_flags;

	WLPKTTAGBSSCFGSET(pkt, WLC_BSSCFG_IDX(scb->bsscfg));

	/* Enqueue the PS-Poll response at higher precedence level */
	if (!wlc_apps_ps_enq_resp(wlc, scb, pkt, WLC_PRIO_TO_HI_PREC(PKTPRIO(pkt))))
		wlc_apps_apsd_usp_end(wlc, scb);

	/* Send to hardware (latency for first APSD-delivered frame is especially important) */
	wlc_send_q(wlc, SCB_WLCIFP(scb)->qi);

	/* Check if the PVB entry needs to be cleared */
	if (scb_psinfo->in_pvb)
		wlc_apps_pvb_update(wlc, scb);

#ifdef PROP_TXSTATUS
	if (extra_flags & WLF_APSD)
		scb_psinfo->apsd_tx_pending = TRUE;
#endif // endif

	return TRUE;
}

static bool
wlc_apps_ps_enq_resp(wlc_info_t *wlc, struct scb *scb, void *pkt, int prec)
{
	apps_wlc_psinfo_t *wlc_psinfo;
	wlc_txq_info_t *qi = SCB_WLCIFP(scb)->qi;

	wlc_psinfo = wlc->psinfo;

	/* Decrement the global ps pkt cnt */
	if (!SCB_ISMULTI(scb))
		wlc_psinfo->ps_deferred--;

	/* register WLF2_PCB2_PSP_RSP for pkt */
	WLF2_PCB2_REG(pkt, WLF2_PCB2_PSP_RSP);

	/* Ensure the pkt marker (used for ageing) is cleared */
	WLPKTTAG(pkt)->flags &= ~WLF_PSMARK;

	WL_PS(("wl%d.%d: ps_enq_resp %p supr %d apsd %d\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), pkt,
	       (WLPKTTAG(pkt)->flags & WLF_TXHDR) ? 1 : 0,
	       (WLPKTTAG(pkt)->flags & WLF_APSD) ? 1 : 0));

	/* Enqueue in order of precedence */
	if (!wlc_prec_enq(wlc, WLC_GET_TXQ(qi), pkt, prec)) {
		WL_ERROR(("wl%d.%d: %s: txq full, frame discarded\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
		PKTFREE(wlc->osh, pkt, TRUE);
		return FALSE;
	}

	return TRUE;
}

static bool
wlc_apps_psq_ageing_needed(wlc_info_t *wlc, struct scb *scb)
{
	/* Using scb->listen + 1 sec for ageing to avoid packet drop.
	 * In WMM-PS:Test Case 4.10(M.V) which is legacy mixed with wmmps.
	 * buffered frame will be dropped because ageing occurs.
	 */
	wlc_bss_info_t *current_bss = scb->bsscfg->current_bss;
	uint16 interval = scb->listen + (1000/current_bss->beacon_period);

#ifdef WLWNM_AP
	if (WLWNM_ENAB(wlc->pub)) {
		uint32 wnm_scbcap = wlc_wnm_get_scbcap(wlc, scb);
		int sleep_interval = wlc_wnm_scb_sm_interval(wlc, scb);

		if (SCB_WNM_SLEEP(wnm_scbcap) && sleep_interval) {
			interval = MAX((current_bss->dtim_period * sleep_interval), interval);
		}
	}
#endif /* WLWNM_AP */

	return (scb->tbtt >= interval);
}

/* Reclaim as many PS pkts as possible
 *	Reclaim from all STAs with pending traffic.
 */
void
wlc_apps_psq_ageing(wlc_info_t *wlc)
{
	apps_wlc_psinfo_t *wlc_psinfo = wlc->psinfo;
	struct apps_scb_psinfo *scb_psinfo;
	struct scb_iter scbiter;
	struct scb *tscb;

	if (wlc_psinfo->ps_nodes_all == 0) {
		return; /* No one in PS */
	}

	FOREACHSCB(wlc->scbstate, &scbiter, tscb) {
		scb_psinfo = SCB_PSINFO(wlc->psinfo, tscb);
		if (!tscb->permanent && SCB_PS(tscb) && wlc_apps_psq_ageing_needed(wlc, tscb)) {
			tscb->tbtt = 0;
			/* Initiate an ageing event per listen interval */
			if (!pktq_empty(&scb_psinfo->psq))
				wlc_apps_ps_timedout(wlc, tscb);
		}
	}
}

/* check if we should age pkts or not */
static void
wlc_apps_ps_timedout(wlc_info_t *wlc, struct scb *scb)
{
#if defined(BCMDBG) || defined(WLMSG_PS)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	void *pkt, *head_pkt;
	struct ether_addr ea;
	struct apps_scb_psinfo *scb_psinfo;
	apps_wlc_psinfo_t *wlc_psinfo = wlc->psinfo;
	struct pktq *psq;
	int prec;
	wlc_bsscfg_t *bsscfg;

	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);

	psq = &scb_psinfo->psq;

	if (pktq_empty(psq))
		return;

	PKTQ_PREC_ITER(psq, prec) {
		/* Age out all pkts that have been
		 * through one previous listen interval
		 */
		head_pkt = NULL;
		while (pktq_ppeek(psq, prec) != head_pkt) {
			pkt = pktq_pdeq(psq, prec);
			ASSERT(pkt != NULL);
			/* If not marked just move on */
			if ((WLPKTTAG(pkt)->flags & WLF_PSMARK) == 0) {
				WLPKTTAG(pkt)->flags |= WLF_PSMARK;
				if (!head_pkt)
					head_pkt = pkt;
				pktq_penq(psq, prec, pkt);
				continue;
			}

			wlc_psinfo->ps_deferred--;
			wlc_psinfo->ps_aged++;

			/* save ea and bsscfg before call wlc_pkt_flush */
			ea = scb->ea;
			bsscfg = scb->bsscfg;

			WL_PS(("wl%d.%d: wlc_apps_ps_timedout: timing out packet for %s aid %d, %d "
			       "remain\n", wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
			       bcm_ether_ntoa(&scb->ea, eabuf),
			       AID2PVBMAP(scb->aid), pktq_len(psq)));

			/* call callback and free */
			PKTFREE(wlc->osh, pkt, TRUE);

			/* callback may have freed scb */
			if (wlc_scbfind(wlc, bsscfg, &ea) == NULL) {
				WL_PS(("wl%d.%d: wlc_apps_ps_timedout: exiting, scb for %s was "
				       "freed after last packet timeout\n", wlc->pub->unit,
				       WLC_BSSCFG_IDX(bsscfg), bcm_ether_ntoa(&ea, eabuf)));
				return;
			}
		}
	}

	/* Check if the PVB entry needs to be cleared */
	if (scb_psinfo->in_pvb)
		wlc_apps_pvb_update(wlc, scb);
}

/* Try to PS enq a pkt, return false if we could not */
static void
_wlc_apps_ps_enq(void *ctx, struct scb *scb, void *pkt, uint prec)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	apps_wlc_psinfo_t *wlc_psinfo = wlc->psinfo;

	ASSERT(!SCB_ISMULTI(scb));
	ASSERT(!PKTISCHAINED(pkt));

#ifdef WLTDLS
	/* for TDLS PTI resp, don't enq to PSQ, send right away */
	if (BSS_TDLS_ENAB(wlc, SCB_BSSCFG(scb)) && SCB_PS(scb) &&
		(WLPKTTAG(pkt)->flags & WLF_PSDONTQ)) {
		WL_PS(("wl%d.%d:%s(): skip enq to PSQ\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
		SCB_TX_NEXT(TXMOD_APPS, scb, pkt, prec);
		return;
	}
#endif /* WLTDLS */

	if (!wlc_apps_psq(wlc, pkt, prec)) {
		struct apps_scb_psinfo *scb_psinfo;
		WL_PS(("wl%d.%d: wlc_apps_ps_enq: ps pkt discarded\n", wlc->pub->unit,
		       WLC_BSSCFG_IDX(SCB_BSSCFG(scb))));
		wlc_psinfo->ps_discard++;
		ASSERT(scb);
		scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);
		ASSERT(scb_psinfo);
		scb_psinfo->ps_discard++;
#ifdef PROP_TXSTATUS
		if (PROP_TXSTATUS_ENAB(wlc->pub)) {
			/*
			   wlc decided to discard the packet, host should hold onto it,
			   this is effectively a "suppress by wl" instead of D11
			*/
			wlc_suppress_sync_fsm(wlc, scb, pkt, TRUE);
			wlc_process_wlhdr_txstatus(wlc, WLFC_CTL_PKTFLAG_WLSUPPRESS, pkt, FALSE);
#ifdef PROP_TXSTATUS_DEBUG
			(wlfc_state_get(wlc->wl))->dbgstats->wlfc_wlfc_sup++;
#endif // endif
		}
#endif /* PROP_TXSTATUS */

		PKTFREE(wlc->osh, pkt, TRUE);
	}
}

static void
wlc_apps_ps_enq(void *ctx, struct scb *scb, void *pkt, uint prec)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	_wlc_apps_ps_enq(ctx, scb, pkt, prec);
	BCM_REFERENCE(wlc);
#if defined(BCMPCIEDEV) && defined(PROP_TXSTATUS)
	if (PROP_TXSTATUS_ENAB(wlc->pub) && BCMPCIEDEV_ENAB()) {
		struct apps_wlc_psinfo *wlc_psinfo;
		struct apps_scb_psinfo *scb_psinfo;
		ASSERT(wlc);
		ASSERT(scb);
		wlc_psinfo = wlc->psinfo;
		scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);
		if (scb_psinfo->in_transit > 0) {
			scb_psinfo->in_transit --;
		}
	}
#endif // endif
}

/* Try to ps enq the pkts on the txq */
static void
wlc_apps_txq_to_psq(wlc_info_t *wlc, struct scb *scb)
{
	struct pktq *txq = WLC_GET_TXQ(SCB_WLCIFP(scb)->qi);
	wlc_apps_move_to_psq(wlc, txq, scb);
}

#ifdef PROP_TXSTATUS
#ifdef WLNAR
/* Try to ps enq the pkts on narq */
static void
wlc_apps_nar_txq_to_psq(wlc_info_t *wlc, struct scb *scb)
{
	struct pktq *txq = wlc_nar_txq(wlc->nar_handle, scb);
	if (txq) {
		wlc_apps_move_to_psq(wlc, txq, scb);
	}
}
#endif /* WLNAR */

#ifdef WLAMPDU
/* This causes problems for PSPRETEND */
/* ps enq pkts on ampduq */
static void
wlc_apps_ampdu_txq_to_psq(wlc_info_t *wlc, struct scb *scb)
{
	if (AMPDU_ENAB(wlc->pub)) {
		struct pktq *txq = wlc_ampdu_txq(wlc->ampdu_tx, scb);
		if (txq) wlc_apps_move_to_psq(wlc, txq, scb);
	}
}
#endif /* WLAMPDU */
#endif /* PROP_TXSTATUS */

static void wlc_apps_move_to_psq(wlc_info_t *wlc, struct pktq *txq, struct scb* scb)
{
	void *head_pkt = NULL, *pkt;
	int prec;
#ifdef WLTDLS
	bool q_empty = TRUE;
	apps_wlc_psinfo_t *wlc_psinfo;
	struct apps_scb_psinfo *scb_psinfo;
#endif // endif

	ASSERT(AP_ENAB(wlc->pub) || BSS_TDLS_BUFFER_STA(SCB_BSSCFG(scb)) ||
		BSSCFG_IBSS(SCB_BSSCFG(scb)));

	PKTQ_PREC_ITER(txq, prec) {
		head_pkt = NULL;
		/* PS enq all the pkts we can */
		while (pktq_ppeek(txq, prec) != head_pkt) {
			pkt = pktq_pdeq(txq, prec);
			if (pkt == NULL) {
				/* txq could be emptied in _wlc_apps_ps_enq() */
				WL_ERROR(("WARNING: wl%d: %s: NULL pkt\n", wlc->pub->unit,
					__FUNCTION__));
				break;
			}
			if (scb != WLPKTTAGSCBGET(pkt)) {
				if (!head_pkt)
					head_pkt = pkt;
				pktq_penq(txq, prec, pkt);
				continue;
			}
			/* Enqueueing at the same prec may create a remote
			 * possibility of suppressed pkts being reordered.
			 * Needs to be investigated...
			 */
			_wlc_apps_ps_enq(wlc, scb, pkt, prec);
#if defined(WLTDLS)
			if (TDLS_SUPPORT(wlc->pub))
				q_empty = FALSE;
#endif /* defined(WLTDLS) */
		}
	}

#if defined(WLTDLS)
	if (TDLS_SUPPORT(wlc->pub)) {
		ASSERT(wlc);
		wlc_psinfo = wlc->psinfo;

		ASSERT(scb);
		ASSERT(scb->bsscfg);
		scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);
		if (!q_empty && !scb_psinfo->apsd_usp)
			wlc_tdls_send_pti(wlc->tdls, scb);
	}
#endif /* defined(WLTDLS) */
}

#ifdef PROP_TXSTATUS
/* Set/clear PVB entry according to current state of power save queues at the host */
bool
wlc_apps_pvb_update_from_host(wlc_info_t *wlc, struct scb *scb, bool op)
{
#if defined(BCMPCIEDEV)
	if (PROP_TXSTATUS_ENAB(wlc->pub) && BCMPCIEDEV_ENAB()) {
		struct apps_wlc_psinfo *wlc_psinfo;
		struct apps_scb_psinfo *scb_psinfo;

		ASSERT(wlc);
		ASSERT(scb);
		wlc_psinfo = wlc->psinfo;
		scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);
		uint8 max_depth;

		max_depth = wlc->wlfc_vqdepth;
#ifdef WLTDLS
		if (BSSCFG_IS_TDLS(scb->bsscfg)) {
			max_depth = TDLS_WLFC_DEFAULT_FWQ_DEPTH;
		}
#endif // endif
		/* allow fecting one more packet if apsd session is running */
		max_depth += (scb_psinfo->apsd_usp == TRUE);
		/* fetch the host packets given the wlc->wlfc_vqdepth constraint
		*/
		if (SCB_PROPTXTSTATUS_TIM(scb) &&
			(pktq_len(&scb_psinfo->psq) + scb_psinfo->in_transit < max_depth)) {
			scb_psinfo->in_transit ++;
			return FALSE;
		}
		if (!op && SCB_PROPTXTSTATUS_PKTWAITING(scb)) {
			SCB_PROPTXTSTATUS_SETPKTWAITING(scb, 0);
			/*
			 * Send a null data frame if there are no PS buffered
			 * frames on APSD non-delivery-enabled ACs (WMM/APSD 3.6.1.6).
			 */
			if (pktq_empty(&scb_psinfo->psq) ||
					wlc_apps_apsd_ndelv_count(wlc, scb) == 0) {
				/* Ensure pkt is not queued on psq */
				if (wlc_sendnulldata(wlc, scb->bsscfg, &scb->ea, 0,
						WLF_PSDONTQ, PRIO_8021D_BE,
						wlc_apps_send_psp_response_cb, NULL) == FALSE) {
					WL_ERROR(("wl%d: %s: PS-Poll null data response failed\n",
							wlc->pub->unit, __FUNCTION__));
					scb_psinfo->psp_pending = FALSE;
				} else {
					scb_psinfo->psp_pending = TRUE;
				}
				scb_psinfo->psp_flags &= ~PS_MORE_DATA;
			}
		}

	}
#endif /* BCMPCIEDEV */
	if (!BSSCFG_IBSS(scb->bsscfg)) {
	wlc_apps_pvb_update(wlc, scb);
	}
	return TRUE;
}
#endif /* PROP_TXSTATUS */

/* Set/clear PVB entry according to current state of power save queues */
void
wlc_apps_pvb_update(wlc_info_t *wlc, struct scb *scb)
{
	uint16 aid;
	apps_wlc_psinfo_t *wlc_psinfo;
	struct apps_scb_psinfo *scb_psinfo;
	int ps_count;
	apps_bss_info_t *bss_info;

	ASSERT(wlc);
	wlc_psinfo = wlc->psinfo;

	ASSERT(scb);
	ASSERT(scb->bsscfg);
	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);
	bss_info = APPS_BSSCFG_CUBBY(wlc->psinfo, scb->bsscfg);

	aid = AID2PVBMAP(scb->aid);
	ASSERT(aid);

	/*
	 * WMM/APSD 3.6.1.4: if no ACs are delivery-enabled (legacy), or all ACs are
	 * delivery-enabled (special case), the PVB should indicate if any packet is
	 * buffered.  Otherwise, the PVB should indicate if any packets are buffered
	 * for non-delivery-enabled ACs only.
	 */

	ps_count = ((scb->apsd.ac_delv == AC_BITMAP_NONE ||
	             scb->apsd.ac_delv == AC_BITMAP_ALL) ?
	            pktq_len(&scb_psinfo->psq) :
	            wlc_apps_apsd_ndelv_count(wlc, scb));

#ifdef PROP_TXSTATUS
	/*
	If there is no packet locally, check the traffic availability flags from the
	host.
	*/
	if (ps_count == 0 && PROP_TXSTATUS_ENAB(wlc->pub) &&
			HOST_PROPTXSTATUS_ACTIVATED(wlc)) {
		ac_bitmap_t ac_to_request;
		if (scb->apsd.ac_delv == AC_BITMAP_NONE ||
			scb->apsd.ac_delv == AC_BITMAP_ALL) {
			/* If no ACs are delivery-enabled (legacy), or all ACs
			 * are delivery-enabled (special case), the PVB should
			 * indicate if any packet is buffered.
			 */
			ac_to_request = AC_BITMAP_ALL;
			ps_count = ((SCB_PROPTXTSTATUS_TIM(scb) & ac_to_request) ? 1 : 0);
		}
		else {
			/* Otherwise, the PVB should indicate if any packets are buffered
			 * for non-delivery-enabled ACs only
			 */
			ac_to_request = ~scb->apsd.ac_delv & AC_BITMAP_ALL;

			ps_count = ((SCB_PROPTXTSTATUS_TIM(scb) & ac_to_request) ? 1 : 0);
			if (ps_count) {
				if (wlc_apps_apsd_ndelv_count(wlc, scb) == 0) {
					if (scb_psinfo->in_transit < wlc->wlfc_vqdepth) {
						/* If ps_count > 0, we need to request
						 * for a packet from host
						 */
						wlfc_psmode_request(wlc->wl,
								scb->mac_address_handle, 1,
								ac_to_request,
								WLFC_CTL_TYPE_MAC_REQUEST_PACKET);
					}
				}
			}
		}
	}
#endif /* PROP_TXSTATUS */

#ifdef WLTAF
	/* The TAF scheduler might not release traffic to a station in PS mode. So it could be that
	 * the ps queue is empty, despite higher pending traffic. So we check that case.
	 */
	if (WLTAF_ENAB(wlc->pub)) {
		if (!ps_count && wlc_taf_traffic_active(wlc->taf_handle, scb)) {
			ps_count = 1;
		}
	}
#endif // endif

	if (ps_count > 0 && SCB_PS(scb)) {
		if (scb_psinfo->in_pvb)
			return;

		WL_PS(("wl%d.%d: wlc_apps_pvb_update, setting aid %d\n", wlc->pub->unit,
		       WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), aid));
		/* set the bit in the pvb */
		setbit(bss_info->pvb, aid);

		/* reset the aid range */
		if ((aid < bss_info->aid_lo) || !bss_info->aid_lo)
			bss_info->aid_lo = aid;
		if (aid > bss_info->aid_hi)
			bss_info->aid_hi = aid;

		scb_psinfo->in_pvb = TRUE;
	} else {
		if (!scb_psinfo->in_pvb)
			return;

		WL_PS(("wl%d.%d: wlc_apps_pvb_entry, clearing aid %d\n", wlc->pub->unit,
		       WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), aid));
		/* clear the bit in the pvb */
		clrbit(bss_info->pvb, aid);

		if (bss_info->ps_nodes == 0) {
			bss_info->aid_lo = bss_info->aid_hi = 0;
		} else {
			/* reset the aid range */
			if (aid == bss_info->aid_hi) {
				/* find the next lowest aid value with PS pkts pending */
				for (aid = aid - 1; aid; aid--)
					if (isset(bss_info->pvb, aid)) {
						bss_info->aid_hi = aid;
						break;
					}
				/* no STAs with pending traffic ? */
				if (aid == 0)
					bss_info->aid_hi = bss_info->aid_lo = 0;
			} else if (aid == bss_info->aid_lo) {
				/* find the next highest aid value with PS pkts pending */
				for (aid = aid + 1; aid < wlc->pub->tunables->maxscb; aid++)
					if (isset(bss_info->pvb, aid)) {
						bss_info->aid_lo = aid;
						break;
					}
				ASSERT(aid != wlc->pub->tunables->maxscb);
			}
		}

		scb_psinfo->in_pvb = FALSE;
	}

	/* Update the PVB in the bcn template */
	WL_APSTA_BCN(("wl%d: wlc_apps_pvb_entry -> wlc_bss_update_beacon\n", wlc->pub->unit));
	wlc_bss_update_beacon(wlc, scb->bsscfg);
}

static void
wlc_bss_apps_tbtt_update(wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	struct scb *scb;
	struct scb_iter scbiter;

	ASSERT(cfg != NULL);
	ASSERT(BSSCFG_AP(cfg));

	/* If touching all the PS scbs is too inefficient then we
	 * can maintain a single count and only create an ageing event
	 * using the longest listen interval requested by a STA
	 */

	/* increment the tbtt count on all PS scbs */
	/* APSTA: For APSTA, don't bother aging AP SCBs */
	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
		if (!scb->permanent && SCB_PS(scb))
			if (scb->tbtt < 0xFFFF) /* do not wrap around */
				scb->tbtt++;
	}
}

void
wlc_apps_tbtt_update(wlc_info_t *wlc)
{
	int idx;
	wlc_bsscfg_t *cfg;

	/* If touching all the PS scbs multiple times is too inefficient
	 * then we can restore the old code and have all scbs updated in one pass.
	 */

	FOREACH_UP_AP(wlc, idx, cfg)
	        wlc_bss_apps_tbtt_update(cfg);
}

int wlc_apps_psq_len(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;
	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	return pktq_len(&scb_psinfo->psq);
}

/* called from bmac when a PS state switch is detected from the transmitter.
 * On PS ON switch, directly call wlc_apps_scb_ps_on(wlc, scb);
 *  On PS OFF, check if there are tx packets pending. If so, make a PS OFF reservation
 *  and wait for the drain. Otherwise, switch to PS OFF.
 *  Sends a message to the bmac pmq manager to signal that we detected this switch.
 *  PMQ manager will delete entries when switch states are in sync and the queue is drained.
 *  return 1 if a switch occured. This allows the caller to invalidate
 *  the header cache.
 */
int BCMFASTPATH
wlc_apps_process_ps_switch(wlc_info_t *wlc, struct ether_addr *ea, int8 ps_on)
{

	struct scb *scb = NULL;
	int32 idx;
	wlc_bsscfg_t *cfg;
	struct apps_scb_psinfo *scb_psinfo;

	/* Look for sta's that are associated with the AP, TDLS peers or IBSS peers. */
	FOREACH_BSS(wlc, idx, cfg) {
		if ((BSSCFG_AP(cfg) && cfg->up) || BSS_TDLS_ENAB(wlc, cfg) ||
			BSSCFG_IBSS(cfg)) {
			scb = wlc_scbfind(wlc, cfg, ea);
			if (scb != NULL)
				break;
		}
	}

	/* only process ps transitions for associated sta's, IBSS bsscfg and WDS peers */
	if (!scb || !(SCB_ASSOCIATED(scb) || BSSCFG_IBSS(cfg) || SCB_WDS(scb))) {
		/* send notification to bmac that this entry doesn't exist
		   up here.
		 */
		uint8 ps = PS_SWITCH_STA_REMOVED;
		uint16 *auxpmq_idx = NULL;

		ps |= (wlc->block_datafifo & DATA_BLOCK_PS) ?
			PS_SWITCH_OFF : PS_SWITCH_FIFO_FLUSHED;

		if (scb) {
			auxpmq_idx = &(SCB_PSINFO(wlc->psinfo, scb)->auxpmq_idx);
		}

		wlc_bmac_process_ps_switch(wlc->hw, ea, ps, auxpmq_idx);
		return 0;
	}

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	if (ps_on) {
		if ((ps_on & PS_SWITCH_PMQ_SUPPR_PKT)) {
			WL_PS(("wl%d.%d: %s: Req by suppr pkt! "MACF"\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
				__FUNCTION__, ETHERP_TO_MACF(ea)));
			scb_psinfo->ps_trans_status |= SCB_PS_TRANS_OFF_BLOCKED;
		} else {
			apps_bss_info_t *bss_info = APPS_BSSCFG_CUBBY(wlc->psinfo, cfg);
			WL_PS(("wl%d.%d: %s: Actual PMQ entry addition! "MACF"\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
				__FUNCTION__, ETHERP_TO_MACF(ea)));
			/* This PS ON request is from actual PMQ entry addition. */
			scb_psinfo->ps_trans_status &= ~SCB_PS_TRANS_OFF_BLOCKED;
			bss_info->ps_trans_status &= ~BSS_PS_TRANS_OFF_BLOCKED;
		}
		if (!SCB_PS(scb)) {
#ifdef PSPRETEND
			/* reset pretend status */
			scb->ps_pretend &= ~PS_PRETEND_ON;

			if ((ps_on & PS_SWITCH_PMQ_PSPRETEND) && !SCB_ISMULTI(scb)) {
				wlc_apps_scb_pspretend_on(wlc, scb, PS_PRETEND_ACTIVE_PMQ);
			}
			else
#endif /* PSPRETEND */
			{
				wlc_apps_scb_ps_on(wlc, scb);
			}

#ifdef PSPRETEND
			WL_PS(("wl%d.%d: "MACF" - PS %s, pretend mode %s (succ count %d)\n",
			      wlc->pub->unit, WLC_BSSCFG_IDX(cfg), ETHERP_TO_MACF(ea),
			      SCB_PS(scb) ? "on":"off", SCB_PS_PRETEND(scb) ? "on" :
			      "off", scb->ps_pretend_succ_count));
#else
			WL_PS(("wl%d.%d: "MACF" - PS %s, pretend mode off\n",
			      wlc->pub->unit, WLC_BSSCFG_IDX(cfg), ETHERP_TO_MACF(ea),
			      SCB_PS(scb) ? "on":"off"));
#endif // endif
		}
#ifdef PSPRETEND
		else if (SCB_PS_PRETEND(scb) && (ps_on & PS_SWITCH_PMQ_PSPRETEND)) {
			WL_PS(("wl%d.%d: "MACF" PS pretend was already active now with new PMQ "
				   "PPS entry\n", wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
				   ETHERP_TO_MACF(ea)));
			scb->ps_pretend |= PS_PRETEND_ACTIVE_PMQ;
		}
#endif // endif
		else {
			/* STA is already in PS, clear PS OFF pending bit only */
			scb_psinfo->ps_trans_status &= ~SCB_PS_TRANS_OFF_PEND;
		}
	} else if ((scb_psinfo->ps_trans_status & SCB_PS_TRANS_OFF_BLOCKED)) {
		WL_PS(("wl%d.%d: "MACF" PS off attempt is blocked by WAITPMQ\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), ETHERP_TO_MACF(ea)));
	} else {
		if ((wlc->block_datafifo & DATA_BLOCK_PS)) {
			/* Prevent ON -> OFF transitions while data fifo is blocked.
			 * We need to finish flushing HW and reque'ing before we
			 * can allow the STA to come out of PS.
			 */
			WL_PS(("wl%d.%d: %s "MACF" DATA_BLOCK_PS %d pkts pending%s\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
				ETHERP_TO_MACF(ea), TXPKTPENDTOT(wlc),
				SCB_PS_PRETEND(scb) ? " (ps pretend active)" : ""));
			scb_psinfo->ps_trans_status |= SCB_PS_TRANS_OFF_PEND;
		}
#ifdef PSPRETEND
		else if (SCB_PS_PRETEND_BLOCKED(scb)) {
			/* Prevent ON -> OFF transitions if we were expecting to have
			 * seen a PMQ entry for ps pretend and we have not had it yet.
			 * This is to ensure that when that entry does come later, it
			 * does not cause us to enter ps pretend mode when that condition
			 * should have been cleared
			 */
			WL_PS(("wl%d.%d: "MACF" ps pretend pending off waiting for the PPS PMQ "
				   "entry\n", wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
				   ETHERP_TO_MACF(ea)));
			scb_psinfo->ps_trans_status |= SCB_PS_TRANS_OFF_PEND;
		}
#endif /* PSPRETEND */
		else if (SCB_PS(scb))  {
			wlc_apps_scb_ps_off(wlc, scb, FALSE);
		}
	}

	/* indicate fifo state  */
	if (!(wlc->block_datafifo & DATA_BLOCK_PS))
		ps_on |= PS_SWITCH_FIFO_FLUSHED;
	wlc_bmac_process_ps_switch(wlc->hw, &scb->ea, ps_on, &(scb_psinfo->auxpmq_idx));
	return 0;

}

void
wlc_apps_pspoll_resp_prepare(wlc_info_t *wlc, struct scb *scb,
                             void *pkt, struct dot11_header *h, bool last_frag)
{
	struct apps_scb_psinfo *scb_psinfo;

	ASSERT(scb);
	ASSERT(SCB_PS(scb));

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	/*
	 * FC_MOREDATA is set for every response packet being sent while STA is in PS.
	 * This forces STA to send just one more PS-Poll.  If by that time we actually
	 * have more data, it'll be sent, else a Null data frame without FC_MOREDATA will
	 * be sent.  This technique often improves TCP/IP performance.  The last NULL Data
	 * frame is sent with the WLF_PSDONTQ flag.
	 */

	h->fc |= htol16(FC_MOREDATA);

	/* Register pkt callback for PS-Poll response */
	if (last_frag && !SCB_ISMULTI(scb)) {
		WLF2_PCB2_REG(pkt, WLF2_PCB2_PSP_RSP);
		scb_psinfo->psp_pending = TRUE;
	}

	scb_psinfo->psp_flags |= PS_MORE_DATA;
}

/* Fix PDU that is being sent as a PS-Poll response or APSD delivery frame. */
void
wlc_apps_ps_prep_mpdu(wlc_info_t *wlc, void *pkt, wlc_txh_info_t *txh_info)
{
	bool last_frag;
	struct dot11_header *h;
	uint16	macCtlLow, frameid;
	struct scb *scb;
	wlc_bsscfg_t *bsscfg;
	wlc_key_info_t key_info;

	scb = WLPKTTAGSCBGET(pkt);

	h = txh_info->d11HdrPtr;

	WL_PS(("wl%d.%d: wlc_apps_ps_prep_mpdu: pkt %p flags 0x%x fc 0x%x\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), pkt, WLPKTTAG(pkt)->flags, h->fc));

	/*
	 * Set the IGNOREPMQ bit.
	 *
	 * PS bcast/mcast pkts have following differences from ucast:
	 *    1. use the BCMC fifo
	 *    2. FC_MOREDATA is set by ucode (except for the kludge)
	 *    3. Don't set IGNOREPMQ bit as ucode ignores PMQ when draining
	 *       during DTIM, and looks at PMQ when draining through
	 *       MHF2_TXBCMC_NOW
	 */
	if (ETHER_ISMULTI(txh_info->TxFrameRA)) {
		ASSERT(!SCB_WDS(scb));

		/* Kludge required from wlc_dofrag */
		bsscfg = SCB_BSSCFG(scb);
		frameid = bcmc_fid_generate(wlc, bsscfg, txh_info->TxFrameID);

		/* Update the TxFrameID in both the txh_info struct and the packet header */
		txh_info->TxFrameID = htol16(frameid);

		if (D11REV_GE(wlc->pub->corerev, 40)) {
			d11actxh_t* vhtHdr = &(txh_info->hdrPtr->txd);
			vhtHdr->PktInfo.TxFrameID =  htol16(frameid);
		} else {
			d11txh_t* nonVHTHdr = &(txh_info->hdrPtr->d11txh);
			nonVHTHdr->TxFrameID = htol16(frameid);
		}

		ASSERT(!SCB_A4_DATA(scb));

		/* APSTA: MUST USE BSS AUTH DUE TO SINGLE BCMC SCB; IS THIS OK? */
		wlc_keymgmt_get_bss_tx_key(wlc->keymgmt, bsscfg, FALSE, &key_info);

		if (!bcmwpa_is_wpa_auth(bsscfg->WPA_auth) || key_info.algo != CRYPTO_ALGO_AES_CCM)
			h->fc |= htol16(FC_MOREDATA);
	}
	else if (!SCB_ISMULTI(scb)) {
		/* There is a hack to send uni-cast P2P_PROBE_RESP frames using bsscfg's
		* mcast scb because of no uni-cast scb is available for bsscfg, we need to exclude
		* such hacked packtes from uni-cast processing.
		*/
		last_frag = (ltoh16(h->fc) & FC_MOREFRAG) == 0;
		/* Set IGNOREPMQ bit (otherwise, it may be suppressed again) */
		macCtlLow = ltoh16(txh_info->MacTxControlLow);
		if (D11REV_GE(wlc->pub->corerev, 40)) {
			d11actxh_t* vhtHdr = &(txh_info->hdrPtr->txd);
			macCtlLow |= D11AC_TXC_IPMQ;
			vhtHdr->PktInfo.MacTxControlLow = htol16(macCtlLow);
#ifdef PSPRETEND
			{
				uint16 macCtlHigh = ltoh16(txh_info->MacTxControlHigh);
				macCtlHigh &= ~D11AC_TXC_PPS;
				vhtHdr->PktInfo.MacTxControlHigh = htol16(macCtlHigh);
			}
#endif // endif
		} else {
			d11txh_t* nonVHTHdr = &(txh_info->hdrPtr->d11txh);
			macCtlLow |= TXC_IGNOREPMQ;
			nonVHTHdr->MacTxControlLow = htol16(macCtlLow);
		}

		/*
		 * Set FC_MOREDATA and EOSP bit and register callback.  WLF_APSD is set
		 * for all APSD delivery frames.  WLF_PSDONTQ is set only for the final
		 * Null frame of a series of PS-Poll responses.
		 */
		if (WLPKTTAG(pkt)->flags & WLF_APSD)
			wlc_apps_apsd_prepare(wlc, scb, pkt, h, last_frag);
		else if (!(WLPKTTAG(pkt)->flags & WLF_PSDONTQ))
			wlc_apps_pspoll_resp_prepare(wlc, scb, pkt, h, last_frag);
	}
}

static void
wlc_apps_psp_resp_complete(wlc_info_t *wlc, void *pkt, uint txs)
{
	struct scb *scb;
	struct apps_scb_psinfo *scb_psinfo;
	struct scb *newscb;
	struct scb_iter scbiter;
	bool match = FALSE;

	/* Is this scb still around */
	if ((scb = WLPKTTAGSCBGET(pkt)) == NULL)
		return;

	/* Before using the scb, need to check if this is stale first */
	FOREACHSCB(wlc->scbstate, &scbiter, newscb) {
		if (newscb == scb) {
			match = TRUE;
			break;
		}
	}
	if (!match)
		return;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	/* clear multiple ps-poll frame protection */
	scb_psinfo->psp_flags &= ~PS_PSP_ONRESP;

	if (scb_psinfo->psp_pending) {
		scb_psinfo->psp_pending = FALSE;
		if (scb_psinfo->tx_block != 0) {
			WL_PS(("wl%d.%d: %s: tx blocked\n", wlc->pub->unit,
			       WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
			return;
		}
		if (scb_psinfo->psp_flags & PS_PSP_REQ_PEND) {
			/* send the next ps pkt if requested */
			scb_psinfo->psp_flags &= ~(PS_MORE_DATA | PS_PSP_REQ_PEND);
			WL_ERROR(("wl%d.%d:%s send more frame.\n", wlc->pub->unit,
				WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
			wlc_apps_ps_send(wlc, scb, WLC_PREC_BMP_ALL, 0);
		}
	}
}

static int wlc_apps_send_psp_response_cb(wlc_info_t *wlc, wlc_bsscfg_t *cfg, void *pkt, void *data)
{
	/* register packet callback */
	WLF2_PCB2_REG(pkt, WLF2_PCB2_PSP_RSP);
	return BCME_OK;
}

void
wlc_apps_send_psp_response(wlc_info_t *wlc, struct scb *scb, uint16 fc)
{
	struct apps_scb_psinfo *scb_psinfo;

	ASSERT(scb);
	ASSERT(wlc);

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	WL_PS(("wl%d.%d:%s\n", wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
	/* Ignore trigger frames received during tx block period */
	if (scb_psinfo->tx_block != 0) {
		WL_PS(("wl%d.%d: %s tx blocked; ignoring PS poll\n", wlc->pub->unit,
		       WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
		return;
	}

#ifdef PROP_TXSTATUS
	/*
	Immediate:
	caseA - psq is not empty
	caseB - (psq is empty) && (vqdepth != 0) && (ta_bmp == 0)
	caseC - (psq is empty) && (vqdepth == 0) && (ta_bmp == 0)

	Asynchronous:
	caseD: (psq is empty) && (vqdepth == 0) &&  (ta_bmp != 0)
	caseE: (psq is empty) && (vqdepth != 0) &&  (ta_bmp != 0)

	Just deal with async cases here, rest should not have to change.
	*/
	if (PROP_TXSTATUS_ENAB(wlc->pub)) {
		if (SCB_PROPTXTSTATUS_PKTWAITING(scb)) {
			SCB_PROPTXTSTATUS_SETPKTWAITING(scb, 0);
		}
		if ((pktq_len(&scb_psinfo->psq) < wlc->wlfc_vqdepth) &&
			SCB_PROPTXTSTATUS_TIM(scb)) {
			/* request the host for a packet */
			wlfc_psmode_request(wlc->wl, scb->mac_address_handle,
				1, AC_BITMAP_ALL, WLFC_CTL_TYPE_MAC_REQUEST_PACKET);

			if (fc & FC_RETRY) {
				/* remember the fc retry bit */
				SCB_PROPTXTSTATUS_SETPOLLRETRY(scb, 1);
			}
			if (pktq_empty(&scb_psinfo->psq) && SCB_PROPTXTSTATUS_TIM(scb)) {
				SCB_PROPTXTSTATUS_SETPKTWAITING(scb, 1);
				return;
			}
		}
	}
#endif /* PROP_TXSTATUS */

	/* enable multiple ps-poll frame check */
	if (scb_psinfo->psp_flags & PS_PSP_ONRESP) {
		WL_PS(("wl%d.%d: %s. previous ps-poll frame under handling. drop new ps-poll frame",
			wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__));
		return;
	}
	else
		scb_psinfo->psp_flags |= PS_PSP_ONRESP;

	/*
	 * Send a null data frame if there are no PS buffered
	 * frames on APSD non-delivery-enabled ACs (WMM/APSD 3.6.1.6).
	 */
	if (pktq_empty(&scb_psinfo->psq) || wlc_apps_apsd_ndelv_count(wlc, scb) == 0) {
		/* Ensure pkt is not queued on psq */
		if (wlc_sendnulldata(wlc, scb->bsscfg, &scb->ea, 0, WLF_PSDONTQ,
			PRIO_8021D_BE, wlc_apps_send_psp_response_cb, NULL) == FALSE) {
			WL_ERROR(("wl%d: %s: PS-Poll null data response failed\n",
			          wlc->pub->unit, __FUNCTION__));
			scb_psinfo->psp_pending = FALSE;
		} else
			scb_psinfo->psp_pending = TRUE;

		scb_psinfo->psp_flags &= ~PS_MORE_DATA;
	}
	/* Check if we should ignore the ps poll */
	else if (scb_psinfo->psp_pending && !SCB_ISMULTI(scb)) {
		/* Reply to a non retried PS Poll pkt after the current
		 * psp_pending has completed (if that pending pkt indicated "more
		 * data"). This aids the stalemate introduced if a STA acks a ps
		 * poll response but the AP misses that ack
		 */
		if ((scb_psinfo->psp_flags & PS_MORE_DATA) && !(fc & FC_RETRY))
			scb_psinfo->psp_flags |= PS_PSP_REQ_PEND;
	} else {
		/* Check whether there are any legacy frames before sending
		 * any delivery enabled frames
		 */
		if (wlc_apps_apsd_delv_count(wlc, scb) > 0) {
			ac_bitmap_t ac_non_delv = ~scb->apsd.ac_delv & AC_BITMAP_ALL;
			uint32 precbitmap = WLC_ACBITMAP_TO_PRECBITMAP(ac_non_delv);

			wlc_apps_ps_send(wlc, scb, precbitmap, 0);
		} else {
			wlc_apps_ps_send(wlc, scb, WLC_PREC_BMP_ALL, 0);
		}
	}
}

/* get PVB info */
static INLINE void
wlc_apps_tim_pvb(apps_bss_info_t *bss_psinfo, uint8 *offset, int16 *length)
{
	uint8 n1, n2;

	n1 = (uint8)(bss_psinfo->aid_lo/8);
	/* n1 must be highest even number */
	n1 &= ~1;
	n2 = (uint8)(bss_psinfo->aid_hi/8);

	*offset = n1;
	*length = n2 - n1 + 1;

	ASSERT(*offset <= 127);
	ASSERT(*length >= 1 && *length <= 251);
}

/* calculate TIM IE length */
static uint
wlc_apps_tim_len(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	apps_bss_info_t *bss_psinfo;
	uint8 offset;
	int16 length;

	ASSERT(cfg != NULL);
	ASSERT(BSSCFG_AP(cfg));

	bss_psinfo = APPS_BSSCFG_CUBBY(wlc->psinfo, cfg);

	wlc_apps_tim_pvb(bss_psinfo, &offset, &length);

	return TLV_HDR_LEN + DOT11_MNG_TIM_FIXED_LEN + length;
}

/* Fill in the TIM element for the specified bsscfg */
static int
wlc_apps_tim_create(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *buf, uint len)
{
	apps_bss_info_t *bss_psinfo;
	uint8 offset;
	int16 length;
	wlc_bss_info_t *current_bss;

	ASSERT(cfg != NULL);
	ASSERT(BSSCFG_AP(cfg));
	ASSERT(buf != NULL);

	/* perform length check to make sure tim buffer is big enough */
	if (wlc_apps_tim_len(wlc, cfg) > len)
		return BCME_BUFTOOSHORT;

	current_bss = cfg->current_bss;

	bss_psinfo = APPS_BSSCFG_CUBBY(wlc->psinfo, cfg);

	wlc_apps_tim_pvb(bss_psinfo, &offset, &length);

	buf[0] = DOT11_MNG_TIM_ID;
	/* set the length of the TIM */
	buf[1] = (uint8)(DOT11_MNG_TIM_FIXED_LEN + length);
	buf[2] = (uint8)(current_bss->dtim_period - 1);
	buf[3] = (uint8)current_bss->dtim_period;
	/* set the offset field of the TIM */
	buf[4] = offset;
	/* copy the PVB into the TIM */
	bcopy(&bss_psinfo->pvb[offset], &buf[5], length);
	return BCME_OK;
}

bool
wlc_apps_scb_supr_enq(wlc_info_t *wlc, struct scb *scb, void *pkt)
{
	struct apps_scb_psinfo *scb_psinfo;
	apps_wlc_psinfo_t *wlc_psinfo = wlc->psinfo;

	ASSERT(scb != NULL);
	ASSERT(SCB_PS(scb));
	ASSERT(!SCB_ISMULTI(scb));
	ASSERT((SCB_ASSOCIATED(scb)) || (SCB_WDS(scb) != NULL));
	ASSERT(pkt != NULL);

	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);

	if (WLF2_PCB2(pkt) == WLF2_PCB2_PSP_RSP) {
		scb_psinfo->psp_flags &= ~PS_PSP_ONRESP;
		scb_psinfo->psp_pending = FALSE;
	}

	/* unregister pkt callback */
	WLF2_PCB2_UNREG(pkt);

	/* PR56242: tag the pkt so that we can identify them later and move them
	 * to the front when tx fifo drain/flush finishes.
	 */
	WLPKTTAG(pkt)->flags3 |= WLF3_SUPR;
	/* Mark as retrieved from HW FIFO */
	WLPKTTAG(pkt)->flags |= WLF_FIFOPKT;

	/* If enqueue to psq successfully, return FALSE so that PDU is not freed */
	/* Enqueue at higher precedence as these are suppressed packets */
	if (wlc_apps_psq(wlc, pkt, WLC_PRIO_TO_HI_PREC(PKTPRIO(pkt)))) {
		WLPKTTAG(pkt)->flags &= ~WLF_APSD;
		return FALSE;
	}

	WL_ERROR(("wl%d: %s: ps suppr pkt discarded\n", wlc->pub->unit, __FUNCTION__));
	wlc_psinfo->ps_discard++;
	scb_psinfo->ps_discard++;
	return TRUE;
}

/* Enqueue a suppressed PDU to psq after fixing up the PDU */
bool
wlc_apps_suppr_frame_enq(wlc_info_t *wlc, void *pkt, tx_status_t *txs, bool last_frag)
{
	uint16 frag = 0;
	uint16 retries = txs->status.raw_bits & (TX_STATUS_FRM_RTX_MASK | TX_STATUS_RTS_RTX_MASK);
	uint16 seq_num = 0;
	struct scb *scb = WLPKTTAGSCBGET(pkt);
	struct apps_scb_psinfo *scb_psinfo;
	apps_wlc_psinfo_t *wlc_psinfo = wlc->psinfo;
	struct dot11_header *h;
	uint16 txc_hwseq;
	wlc_txh_info_t txh_info;
	bool control;

	if (!SCB_PS(scb)) {
		/* Due to races in what indications are processed first, we either get
		 * a PMQ indication that a SCB has entered PS mode, or we get a PMQ
		 * suppressed packet. This is the patch where a PMQ suppressed packet is
		 * the first indication that a SCB is in PS mode.
		 * Signal the PS switch with the flag that the indication was a suppress packet.
		 */
		WL_PS(("%s: PMQ entry interrupt delayed! "MACF"\n",
		__FUNCTION__, ETHER_TO_MACF(scb->ea)));
		if (ETHER_ISBCAST(&scb->ea)) {
			/* This BCMC SCB PS process is valid only if the unicast SCB that triggered
			 * this PMQ suppression is associated or from IBSS bss.
			 */
			wlc_bsscfg_t *bsscfg = SCB_BSSCFG(scb);
			bool valid = BSSCFG_IBSS(bsscfg);

			if (!valid) {
				struct scb *scbtmp;
				struct scb_iter scbiter;
				FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scbtmp) {
					if (SCB_ASSOCIATED(scbtmp)) {
						valid = TRUE;
					}
				}
			}
			if (valid) {
				wlc_apps_bcmc_scb_ps_on(wlc, bsscfg, TRUE);
			} else {
				WL_ERROR(("%s: Invalid BCMC PMQ suppress!\n", __FUNCTION__));
			}
		} else {
			wlc_apps_process_ps_switch(wlc, &scb->ea, PS_SWITCH_PMQ_SUPPR_PKT);
		}
	}

	wlc_get_txh_info(wlc, pkt, &txh_info);

	h = txh_info.d11HdrPtr;
	control = FC_TYPE(ltoh16(h->fc)) == FC_TYPE_CTL;

	if (!control) {
		seq_num = ltoh16(h->seq);
		frag = seq_num & FRAGNUM_MASK;
	}

	ASSERT(scb != NULL);

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	/* Is this the first suppressed frame, and either is partial
	 * MSDU or has been retried at least once, driver needs to
	 * preserve the retry count and sequence number in the PDU so that
	 * next time it is transmitted, the receiver can put it in order
	 * or discard based on retries. For partial MSDU, reused sequence
	 * number will allow reassembly
	 */
	if (!scb_psinfo->first_suppr_handled && (frag || retries) && !control) {
		/* If the seq num was hw generated then get it from the
		 * status pkt otherwise get it from the original pkt
		 */
		if (D11REV_GE(wlc->pub->corerev, 40)) {
			txc_hwseq = txh_info.MacTxControlLow & htol16(D11AC_TXC_ASEQ);
		} else {
			txc_hwseq = txh_info.MacTxControlLow & htol16(TXC_HWSEQ);
		}

		if (txc_hwseq)
			seq_num = txs->sequence;
		else
			seq_num = seq_num >> SEQNUM_SHIFT;

		h->seq = htol16((seq_num << SEQNUM_SHIFT) | (frag & FRAGNUM_MASK));

		/* Clear hwseq flag in maccontrol low */
		/* set the retry counts */
		if (D11REV_GE(wlc->pub->corerev, 40)) {
			d11actxh_t* vhtHdr = &(txh_info.hdrPtr->txd);
			vhtHdr->PktInfo.MacTxControlLow &=  ~htol16(D11AC_TXC_ASEQ);
			vhtHdr->PktInfo.TxStatus = htol16(retries);
		} else {
			d11txh_t* nonVHTHdr = &(txh_info.hdrPtr->d11txh);
			nonVHTHdr->MacTxControlLow &= ~htol16(TXC_HWSEQ);
			nonVHTHdr->TxStatus = htol16(retries);
		}

		WL_PS(("Partial MSDU PDU %p - frag:%d seq_num:%d retries: %d\n", pkt,
		       frag, seq_num, retries));
	}

	/* This ensures that all the MPDUs of the same SDU get
	 * same seq_num. This is a case when first fragment was retried
	 */
	if (last_frag || !(frag || retries))
		scb_psinfo->first_suppr_handled = TRUE;

#ifdef BCMDBG
	{
	char eabuf[ETHER_ADDR_STR_LEN];

	WL_PS(("SUPPRESSED packet %p - %s %s PS:%d \n", pkt,
	       (FC_TYPE(ltoh16(h->fc)) == FC_TYPE_DATA) ? "data" :
	       (FC_TYPE(ltoh16(h->fc)) == FC_TYPE_MNG) ? "mgmt" :
	       (FC_TYPE(ltoh16(h->fc)) == FC_TYPE_CTL) ? "ctrl" :
	       "unknown",
	       bcm_ether_ntoa(&h->a1, eabuf), SCB_PS(scb)));
	}
#endif // endif

	/* PR56242: tag the pkt so that we can identify them later and move them
	 * to the front when tx fifo drain/flush finishes.
	 */
	WLPKTTAG(pkt)->flags3 |= WLF3_SUPR;
	/* Mark as retrieved from HW FIFO */
	WLPKTTAG(pkt)->flags |= WLF_FIFOPKT;

	/* If in PS mode, enqueue the suppressed PDU to PSQ for ucast SCB otherwise txq */
	if (SCB_PS(scb) && !SCB_ISMULTI(scb) && !(WLPKTTAG(pkt)->flags & WLF_PSDONTQ)) {
#ifdef PROP_TXSTATUS
		/* in proptxstatus, the host will resend these suppressed packets */
		/* -memredux- dropped the packet body already anyway. */
		if (!PROP_TXSTATUS_ENAB(wlc->pub) || !HOST_PROPTXSTATUS_ACTIVATED(wlc) ||
			!(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(pkt)->wl_hdr_information) &
			WLFC_PKTFLAG_PKTFROMHOST))
#endif // endif
		{
		/* If enqueue to psq successfully, return FALSE so that PDU is not freed */
		/* Enqueue at higher precedence as these are suppressed packets */
		if (wlc_apps_psq(wlc, pkt, WLC_PRIO_TO_HI_PREC(PKTPRIO(pkt))))
			return FALSE;
		}
		WL_PS(("wl%d.%d: %s: "MACF" ps suppr pkt discarded\n", wlc->pub->unit,
		          WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__, ETHER_TO_MACF(scb->ea)));
		wlc_psinfo->ps_discard++;
		scb_psinfo->ps_discard++;
		return TRUE;
	}

#ifdef PROP_TXSTATUS
		if (!PROP_TXSTATUS_ENAB(wlc->pub) || !HOST_PROPTXSTATUS_ACTIVATED(wlc) ||
			!(WL_TXSTATUS_GET_FLAGS(WLPKTTAG(pkt)->wl_hdr_information) &
			WLFC_PKTFLAG_PKTFROMHOST) ||
			(pktq_plen(WLC_GET_TXQ(SCB_WLCIFP(scb)->qi),
			WLC_PRIO_TO_HI_PREC(PKTPRIO(pkt))) < BCMC_MAX)) {
#endif // endif
	if (wlc_prec_enq(wlc, WLC_GET_TXQ(SCB_WLCIFP(scb)->qi),
		pkt, WLC_PRIO_TO_HI_PREC(PKTPRIO(pkt))))
		return FALSE;
#ifdef PROP_TXSTATUS
		}
#endif // endif

	return TRUE;
}

#ifdef PROP_TXSTATUS
static void
wlc_apps_apsd_hpkt_tmout(void *arg)
{
	struct apps_scb_psinfo *scb_psinfo;
	struct scb *scb;
	wlc_info_t *wlc;

	scb_psinfo = (struct apps_scb_psinfo *)arg;
	ASSERT(scb_psinfo);
	scb = scb_psinfo->scb;
	wlc = scb_psinfo->wlc;

	ASSERT(scb);
	ASSERT(wlc);

	/* send the eosp if still valid (entry to p2p abs makes apsd_usp false)
	* and no pkt in transit/waiting on pkt complete
	*/

	if (scb_psinfo->apsd_usp == TRUE && !scb_psinfo->apsd_tx_pending &&
		(scb_psinfo->apsd_cnt > 0 || scb_psinfo->ext_qos_null)) {
			wlc_apps_apsd_send(wlc, scb);
	}
	scb_psinfo->apsd_hpkt_timer_on = FALSE;
}
#endif /* PROP_TXSTATUS */

static void
wlc_apps_apsd_send(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;
	uint prec_bmp;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);
	ASSERT(scb_psinfo->apsd_cnt > 0 || scb_psinfo->ext_qos_null);

	/*
	 * If there are no buffered frames, send a QoS Null on the highest delivery-enabled AC
	 * (which AC to use is not specified by WMM/APSD).
	 */
	if (scb_psinfo->ext_qos_null ||
	    wlc_apps_apsd_delv_count(wlc, scb) == 0) {
#ifdef WLTDLS
	    if (BSS_TDLS_ENAB(wlc, scb->bsscfg) &&
	        wlc_tdls_in_pti_interval(wlc->tdls, scb))
			return;
#endif /* WLTDLS */

#ifndef PROP_TXSTATUS
		wlc_apps_apsd_eosp_send(wlc, scb);
		return;
#else
		if (PROP_TXSTATUS_ENAB(wlc->pub) && HOST_PROPTXSTATUS_ACTIVATED(wlc)) {
			/* return only, if Host does not have any packet to send again */
			ac_bitmap_t ac_to_request = scb->apsd.ac_delv & AC_BITMAP_ALL;
			if (SCB_PROPTXTSTATUS_TIM(scb) & ac_to_request) {
				if ((wlc_apps_apsd_delv_count(wlc, scb) == 0) &&
						!scb_psinfo->apsd_hpkt_timer_on) {
					wlfc_psmode_request(wlc->wl, scb->mac_address_handle, 1,
						ac_to_request, WLFC_CTL_TYPE_MAC_REQUEST_PACKET);

					wl_add_timer(wlc->wl, scb_psinfo->apsd_hpkt_timer,
						WLC_PS_APSD_HPKT_TIME, FALSE);
					scb_psinfo->apsd_hpkt_timer_on = TRUE;
					return;
				}
			} else {
				wlc_apps_apsd_eosp_send(wlc, scb);
				return;
			}
		}
#endif /* PROP_TXSTATUS */
	}
	prec_bmp = wlc_apps_ac2precbmp_info(scb->apsd.ac_delv);

#ifdef PROP_TXSTATUS
	/* Continuous  pkt flow till last packet is is needed for Wi-Fi P2P 6.1.12/6.1.13.
	* by fetching pkts from host one after another
	* and wait till either timer expires or new packet is received
	*/
	if (PROP_TXSTATUS_ENAB(wlc->pub) && HOST_PROPTXSTATUS_ACTIVATED(wlc)) {
		if (!scb_psinfo->apsd_hpkt_timer_on &&
			scb_psinfo->apsd_cnt > 1 &&
			wlc_apps_apsd_delv_count(wlc, scb) == 1) {
			ac_bitmap_t ac_to_request = scb->apsd.ac_delv & AC_BITMAP_ALL;
			wlfc_psmode_request(wlc->wl, scb->mac_address_handle,
				1, ac_to_request, WLFC_CTL_TYPE_MAC_REQUEST_PACKET);
			wl_add_timer(wlc->wl, scb_psinfo->apsd_hpkt_timer,
				WLC_PS_APSD_HPKT_TIME, FALSE);
			scb_psinfo->apsd_hpkt_timer_on = TRUE;
			return;
		}
	}
#endif /* PROP_TXSTATUS */
	/*
	 * Send a delivery frame.  When the frame goes out, the wlc_apsd_complete
	 * callback will attempt to send the next delivery frame.
	 */
	if (!wlc_apps_ps_send(wlc, scb, prec_bmp, WLF_APSD))
		wlc_apps_apsd_usp_end(wlc, scb);

}

#ifdef WLTDLS
void
wlc_apps_apsd_tdls_send(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);

	if (BSS_TDLS_ENAB(wlc, scb->bsscfg)) {
		if (!scb_psinfo->apsd_usp)
			return;

		scb_psinfo->apsd_cnt = wlc_apps_apsd_delv_count(wlc, scb);

		if (scb_psinfo->apsd_cnt)
			wlc_apps_apsd_send(wlc, scb);
		else
			wlc_apps_apsd_eosp_send(wlc, scb);
	}
	return;
}
#endif /* WLTDLS */

static const uint8 apsd_delv_acbmp2maxprio[] = {
	PRIO_8021D_BE, PRIO_8021D_BE, PRIO_8021D_BK, PRIO_8021D_BK,
	PRIO_8021D_VI, PRIO_8021D_VI, PRIO_8021D_VI, PRIO_8021D_VI,
	PRIO_8021D_NC, PRIO_8021D_NC, PRIO_8021D_NC, PRIO_8021D_NC,
	PRIO_8021D_NC, PRIO_8021D_NC, PRIO_8021D_NC, PRIO_8021D_NC
};

/* Send frames in a USP, called in response to receiving a trigger frame */
void
wlc_apps_apsd_trigger(wlc_info_t *wlc, struct scb *scb, int ac)
{
	struct apps_scb_psinfo *scb_psinfo;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);

	/* Ignore trigger frames received during tx block period */
	if (scb_psinfo->tx_block != 0) {
		WL_PS(("wl%d.%d: wlc_apps_apsd_trigger: tx blocked; ignoring trigger\n",
		       wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb))));
		return;
	}

	/* Ignore trigger frames received during an existing USP */
	if (scb_psinfo->apsd_usp) {
		WL_PS(("wl%d.%d: wlc_apps_apsd_trigger: already in USP; ignoring trigger\n",
		       wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb))));
		return;
	}

	WL_PS(("wl%d.%d: wlc_apps_apsd_trigger: ac %d buffered %d delv %d\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), ac, pktq_plen(&scb_psinfo->psq, ac),
	       wlc_apps_apsd_delv_count(wlc, scb)));

	scb_psinfo->apsd_usp = TRUE;

	/*
	 * Send the first delivery frame.  Subsequent delivery frames will be sent by the
	 * completion callback of each previous frame.  This is not very efficient, but if
	 * we were to queue a bunch of frames to different FIFOs, there would be no
	 * guarantee that the MAC would send the EOSP last.
	 */

	scb_psinfo->apsd_cnt = scb->apsd.maxsplen;

	wlc_apps_apsd_send(wlc, scb);
}

static void
wlc_apps_apsd_eosp_send(wlc_info_t *wlc, struct scb *scb)
{
	int prio = (int)apsd_delv_acbmp2maxprio[scb->apsd.ac_delv & 0xf];
	struct apps_scb_psinfo *scb_psinfo;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);

	WL_PS(("wl%d.%d: wlc_apps_apsd_send: sending QoS Null prio=%d\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), prio));

	scb_psinfo->ext_qos_null = FALSE;
	scb_psinfo->apsd_cnt = 0;

	if (wlc_sendnulldata(wlc, scb->bsscfg, &scb->ea, 0,
		(WLF_PSDONTQ | WLF_APSD), prio, NULL, NULL) == FALSE) {
		WL_ERROR(("wl%d: %s: could not send QoS Null\n",
		          wlc->pub->unit, __FUNCTION__));
		wlc_apps_apsd_usp_end(wlc, scb);
	}

	/* just reset the apsd_uspflag, don't update the apsd_endtime to allow TDLS PTI */
	/* to send immediately for the first packet */
	if (BSS_TDLS_ENAB(wlc, scb->bsscfg))
		wlc_apps_apsd_usp_end(wlc, scb);
}

/* Make decision if we need to count MMPDU in SP */
static bool
wlc_apps_apsd_count_mmpdu_in_sp(wlc_info_t *wlc, struct scb *scb, void *pkt)
{
	return TRUE;
}

void
wlc_apps_apsd_prepare(wlc_info_t *wlc, struct scb *scb, void *pkt,
                      struct dot11_header *h, bool last_frag)
{
	struct apps_scb_psinfo *scb_psinfo;
	uint16 *pqos;
	bool qos;
	bool more = FALSE;
	bool eosp = FALSE;

	/* The packet must have 802.11 header */
	ASSERT(WLPKTTAG(pkt)->flags & WLF_MPDU);

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	/* Set MoreData if there are still buffered delivery frames */
	if (wlc_apps_apsd_delv_count(wlc, scb) > 0)
		h->fc |= htol16(FC_MOREDATA);
	else
		h->fc &= ~htol16(FC_MOREDATA);

	qos = ((ltoh16(h->fc) & FC_KIND_MASK) == FC_QOS_DATA) ||
	      ((ltoh16(h->fc) & FC_KIND_MASK) == FC_QOS_NULL);

	/* SP countdown */
	if (last_frag &&
	    (qos || wlc_apps_apsd_count_mmpdu_in_sp(wlc, scb, pkt))) {
		/* Indicate EOSP when this is the last MSDU in the psq */
		/* JQL: should we keep going in case there are on-the-fly
		 * MSDUs and let the completion callback to check if there is
		 * any other buffered MSDUs then and indicate the EOSP using
		 * an extra QoS NULL frame?
		 */
		more = (ltoh16(h->fc) & FC_MOREDATA) != 0;
		if (!more)
			scb_psinfo->apsd_cnt = 1;
		/* Decrement count of packets left in service period */
		if (scb_psinfo->apsd_cnt != WLC_APSD_USP_UNB)
			scb_psinfo->apsd_cnt--;
	}

	/* SP termination */
	if (qos) {
		pqos = (uint16 *)((uint8 *)h +
			(SCB_A4_DATA(scb) ? DOT11_A4_HDR_LEN : DOT11_A3_HDR_LEN));
		ASSERT(ISALIGNED(pqos, sizeof(*pqos)));

		/* Set EOSP if this is the last frame in the Service Period */
#ifdef WLTDLS
		/* Trigger frames are delivered in PTI interval, because
		 * the the PTI response frame triggers the delivery of buffered
		 * frames before PTI response is processed by TDLS module.
		 * QOS null frames have WLF_PSDONTQ, why should they not terminate
		 * an SP?
		 */
		if (BSS_TDLS_ENAB(wlc, scb->bsscfg) &&
			(wlc_tdls_in_pti_interval(wlc->tdls, scb) ||
			(SCB_PS(scb) && (WLPKTTAG(pkt)->flags & WLF_PSDONTQ) &&
			(scb_psinfo->apsd_cnt != 0)	&&
			((ltoh16(h->fc) & FC_KIND_MASK) != FC_QOS_NULL)))) {
			eosp = FALSE;
		}
		else
#endif // endif
		eosp = scb_psinfo->apsd_cnt == 0 && last_frag;
		if (eosp)
			*pqos |= htol16(QOS_EOSP_MASK);
		else
			*pqos &= ~htol16(QOS_EOSP_MASK);
	}
	/* Send an extra QoS Null to terminate the USP in case
	 * the MSDU doesn't have a EOSP field i.e. MMPDU.
	 */
	else if (scb_psinfo->apsd_cnt == 0)
		scb_psinfo->ext_qos_null = TRUE;

	/* Register callback to end service period after this frame goes out */
	if (last_frag) {
		WLF2_PCB2_REG(pkt, WLF2_PCB2_APSD);
	}

	WL_PS(("wl%d.%d: %s: pkt %p qos %d more %d eosp %d cnt %d lastfrag %d\n",
	       wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__, pkt, qos, more, eosp,
	       scb_psinfo->apsd_cnt, last_frag));
}

/* End the USP when the EOSP has gone out */
static void
wlc_apps_apsd_complete(wlc_info_t *wlc, void *pkt, uint txs)
{
	struct scb *scb;
	struct apps_scb_psinfo *scb_psinfo;
	struct scb *newscb;
	struct scb_iter scbiter;
	bool match = FALSE;

#ifdef BCMDBG
	/* What to do if not ack'd?  Don't want to hang in USP forever... */
	if (txs & TX_STATUS_ACK_RCV)
		WL_PS(("%s():wl%d: delivery frame %p sent\n",
			__FUNCTION__, wlc->pub->unit, pkt));
	else
		WL_PS(("%s():wl%d: delivery frame %p sent (no ACK)\n",
			__FUNCTION__, wlc->pub->unit, pkt));
#endif // endif

	/* Is this scb still around */
	if ((scb = WLPKTTAGSCBGET(pkt)) == NULL) {
		WL_ERROR(("%s(): scb = %p\n",
			__FUNCTION__, scb));
		return;
	}

	/* Before using the scb, need to check if this is stale first */
	FOREACHSCB(wlc->scbstate, &scbiter, newscb) {
		if (newscb == scb) {
			match = TRUE;
			break;
		}
	}
	if (!match) {
		WL_ERROR(("%s(): scb = %p, match = FALSE\n",
			__FUNCTION__, scb));
		return;
	}

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

#ifdef PROP_TXSTATUS
	scb_psinfo->apsd_tx_pending = FALSE;
#endif // endif
	/* Send more frames until the End Of Service Period */
	if (scb_psinfo->apsd_cnt > 0 || scb_psinfo->ext_qos_null) {
		if (scb_psinfo->tx_block != 0) {
			WL_PS(("wl%d.%d: %s: tx blocked, cnt %u\n",
			       wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
			       __FUNCTION__, scb_psinfo->apsd_cnt));
			return;
		}
#ifdef PROP_TXSTATUS
		if (!scb_psinfo->apsd_hpkt_timer_on)
#endif // endif
			wlc_apps_apsd_send(wlc, scb);
		return;
	}

	wlc_apps_apsd_usp_end(wlc, scb);
}

void
wlc_apps_scb_tx_block(wlc_info_t *wlc, struct scb *scb, uint reason, bool block)
{
	struct apps_scb_psinfo *scb_psinfo;

	ASSERT(scb != NULL);

	WL_PS(("wl%d.%d: %s: block %d reason %d\n", wlc->pub->unit,
	       WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), __FUNCTION__, block, reason));

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	if (block) {
		mboolset(scb_psinfo->tx_block, reason);
		/* terminate the APSD USP */
		scb_psinfo->apsd_usp = FALSE;
		scb_psinfo->apsd_cnt = 0;
#ifdef PROP_TXSTATUS
	scb_psinfo->apsd_tx_pending = FALSE;
#endif // endif
	}
	else {
		mboolclr(scb_psinfo->tx_block, reason);
	}
}

int wlc_apps_scb_apsd_cnt(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;

	ASSERT(scb != NULL);

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);

	WL_PS(("wl%d.%d: %s: apsd_cnt = %d\n", wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
	       __FUNCTION__, scb_psinfo->apsd_cnt));

	return scb_psinfo->apsd_cnt;
}

/*
 * Return the number of frames pending on delivery-enabled ACs.
 */
static int
wlc_apps_apsd_delv_count(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;
	uint32 precbitmap;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);

	if (scb->apsd.ac_delv == AC_BITMAP_NONE)
		return 0;

	precbitmap = wlc_apps_ac2precbmp_info(scb->apsd.ac_delv);

	return pktq_mlen(&scb_psinfo->psq, precbitmap);
}

/*
 * Return the number of frames pending on non-delivery-enabled ACs.
 */
static int
wlc_apps_apsd_ndelv_count(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;
	ac_bitmap_t ac_non_delv;
	uint32 precbitmap;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);

	if (scb->apsd.ac_delv == AC_BITMAP_ALL)
		return 0;

	ac_non_delv = ~scb->apsd.ac_delv & AC_BITMAP_ALL;
	precbitmap = wlc_apps_ac2precbmp_info(ac_non_delv);

	return pktq_mlen(&scb_psinfo->psq, precbitmap);
}

uint8
wlc_apps_apsd_ac_available(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;
	uint32 precbitmap;
	uint8 ac_bitmap = 0;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);

	precbitmap = wlc_apps_ac2precbmp_info(WLC_PREC_BMP_AC_BK);
	if (pktq_mlen(&scb_psinfo->psq, precbitmap))
		ac_bitmap |= TDLS_PU_BUFFER_STATUS_AC_BK;

	precbitmap = wlc_apps_ac2precbmp_info(WLC_PREC_BMP_AC_BE);
	if (pktq_mlen(&scb_psinfo->psq, precbitmap))
		ac_bitmap |= TDLS_PU_BUFFER_STATUS_AC_BE;

	precbitmap = wlc_apps_ac2precbmp_info(WLC_PREC_BMP_AC_VI);
	if (pktq_mlen(&scb_psinfo->psq, precbitmap))
		ac_bitmap |= TDLS_PU_BUFFER_STATUS_AC_VO;

	precbitmap = wlc_apps_ac2precbmp_info(WLC_PREC_BMP_AC_VO);
	if (pktq_mlen(&scb_psinfo->psq, precbitmap))
		ac_bitmap |= TDLS_PU_BUFFER_STATUS_AC_VI;

	return ac_bitmap;
}

/* periodically check whether BC/MC queue needs to be flushed */
static void
wlc_apps_bss_wd_ps_check(void *handle)
{
	wlc_info_t *wlc = (wlc_info_t *)handle;
	struct scb *bcmc_scb;
	wlc_bsscfg_t *bsscfg;
	uint i;

	FOREACH_AP(wlc, i, bsscfg) {
		if (!(bsscfg->flags & WLC_BSSCFG_NOBCMC) && !wlc->excursion_active) {
			bcmc_scb = WLC_BCMCSCB_GET(wlc, bsscfg);
			ASSERT(bcmc_scb != NULL);
			ASSERT(bcmc_scb->bsscfg == bsscfg);

			if ((SCB_PS(bcmc_scb) == TRUE) && (TXPKTPENDGET(wlc, TX_BCMC_FIFO) == 0)) {
				if (MBSS_ENAB(wlc->pub)) {
					if (bsscfg->bcmc_fid_shm != INVALIDFID) {
						WL_ERROR(("wl%d.%d: %s: cfg(%p) bcmc_fid = 0x%x "
							"bcmc_fid_shm = 0x%x, resetting bcmc_fids"
							" tot pend %d mc_pkts %d\n",
							wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
							__FUNCTION__, bsscfg, bsscfg->bcmc_fid,
							bsscfg->bcmc_fid_shm,
							TXPKTPENDTOT(wlc),
							wlc_mbss_get_bcmc_pkts_sent(wlc, bsscfg)));
					}
					/* Let's reset the FIDs since we have completed flush */
					wlc_mbss_bcmc_reset(wlc, bsscfg);
				} else {
					BCMCFID(wlc, INVALIDFID);
					if (!BSSCFG_IBSS(bsscfg) || !AIBSS_ENAB(wlc->pub)) {
						bcmc_scb->PS = FALSE;
					}
				}
			}
		}
	}
}

#if defined(MBSS)

/* Enqueue a BC/MC packet onto it's BSS's PS queue */
int
wlc_apps_bcmc_ps_enqueue(wlc_info_t *wlc, struct scb *bcmc_scb, void *pkt)
{
	struct apps_scb_psinfo *scb_psinfo;
	apps_bss_info_t *bss_info;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, bcmc_scb);
	bss_info = APPS_BSSCFG_CUBBY(wlc->psinfo, bcmc_scb->bsscfg);

	/* Check that packet queue length is not exceeded */
	if (pktq_full(&scb_psinfo->psq) || pktq_pfull(&scb_psinfo->psq, MAXPRIO)) {
		WL_NONE(("wlc_apps_bcmc_ps_enqueue: queue full.\n"));
		WLCNTINCR(bss_info->bcmc_discards);
		return BCME_ERROR;
	}
	(void)pktq_penq(&scb_psinfo->psq, MAXPRIO, pkt);
	WLCNTINCR(bss_info->bcmc_pkts_seen);

	return BCME_OK;
}

/* Last STA has gone out of PS.  Check state of its BSS */

static void
wlc_apps_bss_ps_off_start(wlc_info_t *wlc, struct scb *bcmc_scb)
{
	wlc_bsscfg_t *bsscfg;

	bsscfg = bcmc_scb->bsscfg;
	ASSERT(bsscfg != NULL);

	if (!BCMC_PKTS_QUEUED(bsscfg)) {
		/* No pkts in BCMC fifo */
		wlc_apps_bss_ps_off_done(wlc, bsscfg);
	} else { /* Mark in transition */
		ASSERT(bcmc_scb->PS); /* Should only have BCMC pkts if in PS */
		bsscfg->flags |= WLC_BSSCFG_PS_OFF_TRANS;
		WL_PS(("wl%d.%d: START PS-OFF. last fid 0x%x. shm fid 0x%x\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), bsscfg->bcmc_fid,
			bsscfg->bcmc_fid_shm));
#if defined(BCMDBG_MBSS_PROFILE) /* Start transition timing */
		if (bsscfg->ps_start_us == 0) {
			bsscfg->ps_start_us = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);
		}
#endif /* BCMDBG_MBSS_PROFILE */
	}
}

#if defined(WLC_HIGH) && defined(MBSS)
/*
 * Due to a STA transitioning to PS on, all packets have been drained from the
 * data fifos.  Update PS state of all BSSs (if not in PS-OFF transition).
 *
 * Note that it's possible that a STA has come out of PS mode during the
 * transition, so we may return to PS-OFF (abort the transition).  Since we
 * don't keep state of which STA and which BSS started the transition, we
 * simply check them all.
 */

void
wlc_apps_bss_ps_on_done(wlc_info_t *wlc)
{
	wlc_bsscfg_t *bsscfg;
	struct scb *bcmc_scb;
	int i;

	FOREACH_UP_AP(wlc, i, bsscfg) {
		if (!(bsscfg->flags & WLC_BSSCFG_PS_OFF_TRANS)) { /* Ignore BSS in PS-OFF trans */
			bcmc_scb = WLC_BCMCSCB_GET(wlc, bsscfg);
			if (BSS_PS_NODES(wlc->psinfo,  bsscfg) != 0) {
#if defined(MBSS)
				if (!SCB_PS(bcmc_scb) && MBSS_ENAB(wlc->pub)) {
					/* PS off, MC pkts to data fifo should be cleared */
					ASSERT(wlc_mbss_get_bcmc_pkts_sent(wlc, bsscfg) == 0);
					wlc_mbss_increment_ps_trans_cnt(wlc, bsscfg);
					WL_NONE(("wl%d.%d: DONE PS-ON\n",
						wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
				}
#endif // endif
				bcmc_scb->PS = TRUE;
			} else { /* Unaffected BSS or transition aborted for this BSS */
				bcmc_scb->PS = FALSE;
#ifdef PSPRETEND
				bcmc_scb->ps_pretend &= ~PS_PRETEND_ON;
#endif // endif
			}
		}
	}
}

/*
 * Last STA for a BSS exitted PS; BSS has no pkts in BC/MC fifo.
 * Check whether other stations have entered PS since and update
 * state accordingly.
 *
 * That is, it is possible that the BSS state will remain PS
 * TRUE (PS delivery mode enabled) if a STA has changed to PS-ON
 * since the start of the PS-OFF transition.
 */

void
wlc_apps_bss_ps_off_done(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	struct scb *bcmc_scb;

	ASSERT(bsscfg->bcmc_fid_shm == INVALIDFID);
	ASSERT(bsscfg->bcmc_fid == INVALIDFID);

	bcmc_scb = WLC_BCMCSCB_GET(wlc, bsscfg);
	ASSERT(SCB_PS(bcmc_scb));

	if (BSS_PS_NODES(wlc->psinfo, bsscfg) != 0) {
		/* Aborted transtion:  Set PS delivery mode */
		bcmc_scb->PS = TRUE;
	} else { /* Completed transition: Clear PS delivery mode */
		bcmc_scb->PS = FALSE;
#ifdef PSPRETEND
		bcmc_scb->ps_pretend &= ~PS_PRETEND_ON;
#endif // endif
#ifdef MBSS
		wlc_mbss_increment_ps_trans_cnt(wlc, bsscfg);
#endif // endif
		if (bsscfg->flags & WLC_BSSCFG_PS_OFF_TRANS) {
			WL_PS(("wl%d.%d: DONE PS-OFF.\n", wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg)));
		}
	}

	bsscfg->flags &= ~WLC_BSSCFG_PS_OFF_TRANS; /* Clear transition flag */

	/* Forward any packets in MC-PSQ according to new state */
	while (wlc_apps_ps_send(wlc, bcmc_scb, WLC_PREC_BMP_ALL, 0))
		/* Repeat until queue empty */
		;

#if defined(BCMDBG_MBSS_PROFILE)
	if (bsscfg->ps_start_us != 0) {
		uint32 diff_us;

		diff_us = R_REG(wlc->osh, &wlc->regs->tsf_timerlow) - bsscfg->ps_start_us;
		if (diff_us > bsscfg->max_ps_off_us) bsscfg->max_ps_off_us = diff_us;
		bsscfg->tot_ps_off_us += diff_us;
		bsscfg->ps_off_count++;
		bsscfg->ps_start_us = 0;
	}
#endif /* BCMDBG_MBSS_PROFILE */
}

#endif /* WLC_HIGH && MBSS */
#endif /* MBSS */

/*
 * Return the bitmap of ACs with buffered traffic.
 */
uint8
wlc_apps_apsd_ac_buffer_status(wlc_info_t *wlc, struct scb *scb)
{
	struct apps_scb_psinfo *scb_psinfo;
	uint32 precbitmap;
	uint8 ac_bitmap = 0;
	int i;

	scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
	ASSERT(scb_psinfo);

	for (i = 0; i < AC_COUNT; i++) {
		precbitmap = wlc_apps_ac2precbmp_info((1 << i));

		if (pktq_mlen(&scb_psinfo->psq, precbitmap))
			AC_BITMAP_SET(ac_bitmap, i);
	}

	return ac_bitmap;
}

#ifdef PKTQ_LOG
struct pktq* wlc_apps_prec_pktq(wlc_info_t *wlc, struct scb *scb)
{
	struct pktq *q;
	apps_wlc_psinfo_t *wlc_psinfo;
	struct apps_scb_psinfo *scb_psinfo;

	wlc_psinfo = wlc->psinfo;
	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);
	q = &scb_psinfo->psq;

	return q;
}
#endif // endif

/* TIM */
static uint
wlc_apps_calc_tim_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (BSSCFG_AP(cfg))
		return wlc_apps_tim_len(wlc, cfg);

	return 0;
}

static int
wlc_apps_write_tim_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (BSSCFG_AP(cfg)) {
		wlc_apps_tim_create(wlc, cfg, data->buf, data->buf_len);

		data->cbparm->ft->bcn.tim_ie = data->buf;

#ifdef MBSS
		if (MBSS_ENAB(wlc->pub)) {
			wlc_spt_t *bcn_template = wlc_mbss_get_bcn_template(wlc, cfg);

			ASSERT(bcn_template != NULL);
			bcn_template->tim_ie = data->buf;
		}
#endif /* MBSS */
	}

	return BCME_OK;
}

static void
wlc_apps_scb_state_upd_cb(void *ctx, scb_state_upd_data_t *notif_data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	struct scb *scb;
	uint8 oldstate;

	ASSERT(notif_data != NULL);

	scb = notif_data->scb;
	ASSERT(scb != NULL);
	oldstate = notif_data->oldstate;

	if (BSSCFG_AP(scb->bsscfg) && (oldstate & ASSOCIATED) && !SCB_ASSOCIATED(scb)) {
		struct apps_bss_info *bss_info;
		WL_PS(("%s: pvb update needed as SCB is disassociated\n", __FUNCTION__));
		wlc_apps_pvb_update(wlc, scb);
		bss_info = APPS_BSSCFG_CUBBY(wlc->psinfo, SCB_BSSCFG(scb));
		if (bss_info && (bss_info->ps_trans_status & BSS_PS_TRANS_OFF_BLOCKED)) {
			WL_ERROR(("wl%d.%d: "MACF" disassociated. bcmc_scb PS off blocked. PS %d\n",
				wlc->pub->unit, WLC_BSSCFG_IDX(SCB_BSSCFG(scb)),
				ETHER_TO_MACF(scb->ea), SCB_PS(scb)));
		}
	}

}

#ifdef PSPRETEND
/* wlc_apps_process_pspretend_status runs at the end of every TX status. It operates
 * the logic of the ps pretend feature so that the ps pretend will operate during the
 * next tx status
 */
void
wlc_apps_process_pspretend_status(wlc_info_t *wlc, struct scb *scb, bool pps_recvd_ack)
{
	wlc_bsscfg_t* bsscfg;
	wlc_pps_info_t* pps_info;

	/* basic check for null pointer */
	if (!(wlc && scb)) {
		return;
	}

	/* if scb is multicast or in normal PS mode, return as we aren't handling
	 * ps pretend for these
	 */
	if (SCB_ISMULTI(scb) || SCB_PS_PRETEND_NORMALPS(scb)) {
		return;
	}

	bsscfg = SCB_BSSCFG(scb);
	pps_info = wlc->pps_info;

	if (!SCB_PS_PRETEND(scb)) {
		if (pps_recvd_ack) {
			/* the following piece of code is the case of normal successful
			 * tx status with everything all correct and normal
			*/

			/* Implement some scheme against successive ps pretend. Each time ps pretend
			 * is activated, the successive ps pretend count was incremented.
			 * This is decremented for every tx status with at least one good ack.
			 */
			if (scb->ps_pretend_succ_count > 0) {
				scb->ps_pretend_succ_count--;
			}

			/* If we had hit the ps pretend retry limit, then we are now preventing
			 * ps pretend. Check to see if 100ms has elapsed since the previous
			 * ps pretend and the count has decremented to zero. This is
			 * some measure that the air conditions are back to normal.
			 * Then we stop preventing ps pretend to return to normal operation.
			 */
			if ((scb->ps_pretend & PS_PRETEND_PREVENT) &&
				(scb->ps_pretend_succ_count == 0)) {
				/* we have received some successive ack, so we can
				* re-enable ucode pretend ps feature for this scb after 100 ms
				*/
				uint32 ps_pretend_time = R_REG(wlc->osh, &wlc->regs->tsf_timerlow) -
				                         scb->ps_pretend_start;

				/* if we are more than 100 ms since last ps pretend */
				if (ps_pretend_time > 100*1000) {
					/* invalidate txc */
					if (WLC_TXC_ENAB(wlc))
						wlc_txc_inv(wlc->txc, scb);

					scb->ps_pretend &= ~PS_PRETEND_PREVENT;

					WL_PS(("wl%d.%d: remove ps pretend prevent for "MACF"\n",
					       wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
					       ETHER_TO_MACF(scb->ea)));
				}
			}

			/* This is the count of consecutive bad tx status. As soon as we get
			 * a good tx status, this count is immediately cleared.
			 * It is used for the threshold mode detection.
			 */
			scb->ps_pretend_failed_ack_count = 0;

			/* if we were previously primed to enter threshold ps pretend due to
			 * previously meeting the threshold target, we can relax because we
			 * have just received a good tx status, so clear these flags
			 */
			if (SCB_PS_PRETEND_THRESHOLD(scb)) {
				scb->ps_pretend &= ~PS_PRETEND_THRESHOLD;

				WL_PS(("wl%d.%d: "MACF" received successful txstatus, "
					   "canceling threshold ps pretend pending\n",
					   wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
					   ETHER_TO_MACF(scb->ea)));
			}
		} else { /* !pps_recvd_ack */
			/* successive failure count */
			if (scb->ps_pretend_failed_ack_count < MAXNBVAL(sizeof(uint8))) {
				scb->ps_pretend_failed_ack_count++;
			}

			/* if we are doing threshold, check if limit is reached, then
			 * enable packets to cause PPS PMQ entry addition upon
			 * noack tx status
			 */
			if (PS_PRETEND_THRESHOLD_ENABLED(bsscfg) &&
				(scb->ps_pretend_failed_ack_count >=
				bsscfg->ps_pretend_threshold) &&
				!SCB_PS_PRETEND_THRESHOLD(scb)) {
				scb->ps_pretend |= PS_PRETEND_THRESHOLD;

				WL_PS(("wl%d.%d: "MACF" successive failed txstatus (%d) "
					   "is at threshold (%d) for ps pretend\n",
					   wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg),
					   ETHER_TO_MACF(scb->ea),
					   scb->ps_pretend_failed_ack_count,
					   bsscfg->ps_pretend_threshold));
			}
		}

		return;
	}

	/*******************************************************************
	 ****** All code following is relevant if ps pretend is active *****
	 *******************************************************************
	 */

	ASSERT(SCB_PS_PRETEND(scb));

	/* if we did receive a good txstatus, and we are current active in threshold
	 * mode of ps pretend, then we can exit. This can happen if we did not flush
	 * the fifo and packets in transit are still trying to go to air and at least
	 * one of them was successful.
	 * So in that case, make a reservation (by pend bit) for PS off, so the scb will
	 * get out of PM in wlc_apps_process_pend_ps() after txfifo draining is done.
	 */
	if (pps_recvd_ack && SCB_PS_PRETEND_THRESHOLD(scb)) {
		struct apps_scb_psinfo *scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
		ASSERT(wlc->block_datafifo & DATA_BLOCK_PS);
		/* We have received an ack in pretend state and are free to exit */
		WL_PS(("wl%d.%d: "MACF" received successful txstatus in threshold "
			   "ps pretend active state\n",
			   wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), ETHER_TO_MACF(scb->ea)));
		scb_psinfo->ps_trans_status |= SCB_PS_TRANS_OFF_PEND;

		return;
	}

	/* for normal ps pretend using ucode method, check to see
	* if we hit the pspretend_retry_limit. If so, once the whole
	* fifo has been cleared, we then disable ps pretend for
	* this scb until sometime later when the check
	* for successful tx status re-enables ps pretend
	*/
	if (D11REV_GE(wlc->pub->corerev, 40)) {
		/* using ucode method for ps pretend */

		if (!SCB_PS_PRETEND_THRESHOLD(scb) &&
			(scb->ps_pretend_succ_count >=
		        bsscfg->ps_pretend_retry_limit) &&
		    (TXPKTPENDTOT(wlc) == 0)) {

			/* invalidate txc */
			if (WLC_TXC_ENAB(wlc))
				wlc_txc_inv(wlc->txc, scb);

			/* prevent this scb from ucode pretend ps in the
			* future (this is cleared when things return to
			* normal)
			* Wait until tx fifo is empty so that all packets
			* are drained first before disabling ps pretend
			*/
			scb->ps_pretend |= PS_PRETEND_PREVENT;

			WL_PS(("wl%d.%d: "MACF" successive ps pretend (%d) "
				   "exceeds limit (%d), fifo empty\n",
				   wlc->pub->unit, WLC_BSSCFG_IDX(bsscfg), ETHER_TO_MACF(scb->ea),
				   scb->ps_pretend_succ_count,
				   bsscfg->ps_pretend_retry_limit));

			wlc_apps_scb_ps_off(wlc, scb, FALSE);

			return;
		}
	}

	/*
	 * All code following is with ps pretend remaining active without
	 * any transition to non active to be taken
	 */

	/* if we aren't probing this scb yet, it means we only just entered ps pretend
	 * So, send a probe now and begin probing
	 */
	if (!SCB_PS_PRETEND_PROBING(scb)) {
		scb->grace_attempts = 0;
		scb->ps_pretend |= PS_PRETEND_PROBING;

		if (!(wlc->block_datafifo & DATA_BLOCK_PS)) {
			/* send a probe immediately if the fifo is empty */
			wlc_ap_do_pspretend_probe(wlc, scb, 0);
		}
	}

	/* The probing works on the back of a periodic timer. If that timer isn't
	 * running, then start it.
	 * For the first time interval, do 5 milliseconds to send a followup probe
	 * quickly.
	 */
	if (pps_info->ps_pretend_probe_timer && !pps_info->is_ps_pretend_probe_timer_running) {
		wl_add_timer(wlc->wl, pps_info->ps_pretend_probe_timer, 5, FALSE);
		pps_info->is_ps_pretend_probe_timer_running = TRUE;
	}
}

/* returns TRUE if we transitioned ps pretend off > on */
bool
wlc_apps_scb_pspretend_on(wlc_info_t *wlc, struct scb *scb, uint8 flags)
{
	bool ps_retry = FALSE;

	if (!SCB_PS(scb)) {

		wlc_apps_scb_ps_on(wlc, scb);

		/* check PS status in case transition did not work */
		if (SCB_PS(scb)) {

			ps_retry = TRUE;

			/* record entry time for pps */
			scb->ps_pretend_start = R_REG(wlc->osh,
			                        &wlc->regs->tsf_timerlow);

			scb->ps_pretend |= (PS_PRETEND_ACTIVE | PS_PRETEND_RECENT | flags);

			/* count of successive ps pretend transitions */
			scb->ps_pretend_count++;
			scb->ps_pretend_succ_count++;
			WL_PS(("wl%d.%d: "MACF" pretend PS retry mode now "
				   "active %s current PMQ PPS entry, "
				   "count %d/%d\n", wlc->pub->unit,
				   WLC_BSSCFG_IDX(SCB_BSSCFG(scb)), ETHER_TO_MACF(scb->ea),
				   (scb->ps_pretend & PS_PRETEND_ACTIVE_PMQ) ? "with" : "without",
				   scb->ps_pretend_succ_count, scb->ps_pretend_count));
		}
	}
	return ps_retry;
}

bool
wlc_apps_pps_retry(wlc_info_t *wlc, struct scb *scb, void *p, tx_status_t *txs)
{
	bool pps_retry = FALSE;
	uint supr_status = txs->status.suppr_ind;
	d11actxh_t* vhtHdr = NULL;
	uint16	macCtlHigh = 0;

	/* quite complex logic to decide to save the packet with ps pretend
	 *
	 * Test to save the packet with ps pretend (pps_retry flag).....
	 * 1. we are not in ordinary PS mode
	 * 2. suppressed for reasons other ps pretend, forget about saving it unless
	 *    we are already in PS pretend active state, so we might as well save it
	 * 3. ps pretend has got to be globally enabled & the scb has not to be excluded
	 *    from ps pretend
	 * 4. the packet has to have the D11AC_TXC_PPS bit set if we doing normal ps
	 *    pretend. This is regardless of whether TX_STATUS_SUPR_PPS is true; if
	 *    not suppressed it means this is the first tx status with the initial
	 *    transmission failure.
	 * 5. or we have reached the threshold in case of threshold mode
	 * ...... then we save the packet with ps pretend, and mark it for retry
	 *
	 * This 'if..else if' construct is designed to test the most common cases
	 * first and then break out of the logic at the earliest case. Even in the
	 * case we set pps_retry to FALSE, we are needing that case to prevent
	 * moving through the rest of the logic.
	 *
	 */
	if (SCB_PS_PRETEND_NORMALPS(scb)) {
		/* 1 */
		pps_retry = FALSE;
	}
	else if (supr_status && (supr_status != TX_STATUS_SUPR_PPS) &&
			!SCB_PS_PRETEND(scb)) {
		/* 2 */
		pps_retry = FALSE;
	}
	else if (SCB_PS_PRETEND_ENABLED(scb)) {
		/* 3 */
		/* For pcie Full dongle, retried pkts are never enqueud into PSq
		 * pkts are suppressed back to flowring and fetched again
		 * This retry limit for PS_PRETEND is broken with pcieFD
		 * D11AC_TXC_PPS will be set for all the pkts everytime
		 */
		wlc_pkt_get_vht_hdr(wlc, p, &vhtHdr);
		macCtlHigh = ltoh16(vhtHdr->PktInfo.MacTxControlHigh);

		if (macCtlHigh & D11AC_TXC_PPS) {
			/* 4 */
			pps_retry = TRUE;
		}
		else {
			/* packet does not have PPS bit set.
			 * We normally get here if we previously
			 * exceeded the pspretend_retry_limit and the packet subsequently
			 * fails to be sent
			 */
			pps_retry = FALSE;
		}
	}
	else if (SCB_PS_PRETEND_THRESHOLD(scb)) {
		/* 5 */
		pps_retry = TRUE;
	} else if (SCB_PS_PRETEND_CSA_PREVENT(scb, SCB_BSSCFG(scb)) && !supr_status) {
		/* During the period around CSA, PSP is explicitly turned off.
		 * Many packets during this time may get dropped due to no
		 * suppression and no ack. To meet ZPL requirements, these
		 * packets should not be dropped but rather explicitly re-fetched
		 * and re-tried.
		 */
		pps_retry = TRUE;
	}
	return pps_retry;
}
#endif /* PSPRETEND */

void wlc_apps_map_pkts(wlc_info_t *wlc, struct scb *scb, map_pkts_cb_fn cb, void *ctx)
{
	apps_wlc_psinfo_t *wlc_psinfo;
	struct apps_scb_psinfo *scb_psinfo;
	wlc_psinfo = wlc->psinfo;
	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);
	wlc_scb_psq_map_pkts(wlc, &scb_psinfo->psq, cb, ctx);
}

#ifdef WL_AUXPMQ
/* clear auxpmq index for each SCB. */
void
wlc_apps_clear_auxpmq(wlc_info_t *wlc)
{
	struct scb_iter scbiter;
	struct scb *scb;

	FOREACHSCB(wlc->scbstate, &scbiter, scb) {
		struct apps_scb_psinfo *scb_psinfo = SCB_PSINFO(wlc->psinfo, scb);
		scb_psinfo->auxpmq_idx = AUXPMQ_INVALID_IDX;
	}
}
#endif /* WL_AUXPMQ */

void
wlc_apps_set_change_scb_state(wlc_info_t *wlc, struct scb *scb, bool reset)
{
	apps_wlc_psinfo_t *wlc_psinfo;
	struct apps_scb_psinfo *scb_psinfo;
	wlc_psinfo = wlc->psinfo;
	scb_psinfo = SCB_PSINFO(wlc_psinfo, scb);
	scb_psinfo->change_scb_state_to_auth = reset;
}
