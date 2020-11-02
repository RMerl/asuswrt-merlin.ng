/**
 * @file
 * @brief
 * 802.11h Channel Switch Announcement and Extended Channel Switch Announcement module
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
 * $Id: wlc_csa.c 776745 2019-07-08 15:10:41Z $
 */

/**
 * @file
 * @brief
 * Related to radar avoidance.
 * After a valid channel has been selected, AP sends out Channel Switch Announcement (CSA) at same
 * time the beacon and probe response are updated with CSA IE. The channel switching IE gets update
 * on each dpc. When this counter in the IE reaches zero, the AP switch the channel and same time
 * the probe is updated with new channel and offset info in the additional IE. All the STA in the
 * BSS synchronize to the beacon to switch their channel at same time.
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
#ifdef WLCSA

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
#include <wlc_hw.h>
#include <wlc_ap.h>
#include <wlc_assoc.h>
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif // endif
#include <wl_export.h>
#include <wlc_11h.h>
#include <wlc_csa.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_reg.h>
#include <wlc_pcb.h>
#ifdef WLOFFLD
#include <wlc_offloads.h>
#endif // endif
#include <wlc_ht.h>
#include <wlc_txc.h>
#include <wlc_ampdu.h>
#include <wlc_tx.h>
#include <wlc_dfs.h>
#include <wlc_bmac.h>
#ifdef WDS
#include <wlc_wds.h>
#endif /* WDS */

/* IOVar table */
/* No ordering is imposed */
enum {
	IOV_CHANSPEC_SWITCH,	/* send CSA with chanspec as input */
	IOV_CHANSPEC_SWITCH_RESTRICT_TXWIN
};

static const bcm_iovar_t wlc_csa_iovars[] = {
#ifdef AP
	{"csa", IOV_CHANSPEC_SWITCH, (IOVF_SET_UP), IOVT_BUFFER, sizeof(wl_chan_switch_t)},
#endif /* AP */
	{NULL, 0, 0, 0, 0}
};

/* CSA module info */
struct wlc_csa_info {
	wlc_info_t *wlc;
	int cfgh;			/* bsscfg cubby handle */
	int scbh;			/* scb cubby handle */
	bcm_notif_h csa_notif_hdl;
};

/* local functions */
/* module */
static int wlc_csa_up(void *ctx);
static int wlc_csa_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);

/* cubby */
static int wlc_csa_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_csa_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
static int wlc_csa_scb_init(void *ctx, struct scb *scb);
static void wlc_csa_scb_deinit(void *ctx, struct scb *scb);
#ifdef BCMDBG
static void wlc_csa_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
static void wlc_csa_scb_dump(void *ctx, struct scb *scb, struct bcmstrbuf *b);
#else
#define wlc_csa_bsscfg_dump NULL
#define wlc_csa_scb_dump NULL
#endif // endif

/* up/down */
static void wlc_csa_bsscfg_up_down(void *ctx, bsscfg_up_down_event_data_t *evt_data);

/* timer */
static void wlc_csa_timeout(void *arg);

/* action frame */
static int wlc_send_action_switch_channel_ex(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	const struct ether_addr *dst, wl_chan_switch_t *csa, uint8 action_id);
static void wlc_send_public_action_switch_channel(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
  const struct ether_addr *dst, wl_chan_switch_t *csa);

/* channel switch */
#ifdef AP
static int wlc_csa_apply_channel_switch(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg);
#endif // endif

/* unicast csa action ack */
static void wlc_csa_unicast_tx_complete(wlc_info_t *wlc, uint txstatus, void *arg);

/* IE mgmt */
#ifdef AP
static uint wlc_csa_calc_csa_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_csa_write_csa_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_csa_calc_ext_csa_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_csa_write_ext_csa_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_assoc_parse_psta_ie(void *ctx, wlc_iem_parse_data_t *data);
#ifdef WL11AC
static uint wlc_csa_calc_csw_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_csa_write_csw_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_csa_csw_calc_wide_bw_ie_len(void *ctx, wlc_iem_calc_data_t *calc);
static int wlc_csa_csw_write_wide_bw_ie(void *ctx, wlc_iem_build_data_t *build);
#endif // endif
#endif /* AP */
#ifdef STA
static int wlc_csa_bcn_parse_csa_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_csa_bcn_parse_ext_csa_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif /* STA */
static void wlc_csa_obss_dynbw_notif_cb_notif(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	int status, int signal, chanspec_t chanspec);

/* cubby structure and access macros */
typedef struct {
	struct wl_timer *csa_timer;	/* time to switch channel after last beacon */
	wl_chan_switch_t csa;
	struct {
		chanspec_t chanspec;		/* target chanspec */
		uint32 pending;			/* channel switch pending */
	} channel_sw;
} wlc_csa_t;
#define CSA_BSSCFG_CUBBY_LOC(csa, cfg) ((wlc_csa_t **)BSSCFG_CUBBY(cfg, (csa)->cfgh))
#define CSA_BSSCFG_CUBBY(csa, cfg) (*CSA_BSSCFG_CUBBY_LOC(csa, cfg))

#define CSA_UNICAST_RELOCATION_NONE	0x0
#define CSA_UNICAST_RELOCATION_PENDING	0x1
#define CSA_UNICAST_RELOCATION_SUCCESS	0x2
#define CSA_UNICAST_ACK_COUNTER	3
#define CSA_BLOCK_DATAFIFO_COUNT	2	/* 2 Beacon Counts */

/* XXX allocate the struct and reserve a pointer to the struct in the scb
 * as the scb cubby when this structure grows larger than a pointer...
 */
typedef struct {
	uint8	dcs_relocation_state; /* unicast CSA state */
	uint8	dcs_ack_counter; /* unicast CSA ACK counter */
} wlc_csa_scb_cubby_t;
#define CSA_SCB_CUBBY_LOC(csa, scb) ((wlc_csa_scb_cubby_t **)SCB_CUBBY((scb), (csa)->scbh))
#define CSA_SCB_CUBBY(csa, scb) (*(CSA_SCB_CUBBY_LOC(csa, scb)))

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* module */
wlc_csa_info_t *
BCMATTACHFN(wlc_csa_attach)(wlc_info_t *wlc)
{
	wlc_csa_info_t *csam;
	bcm_notif_module_t *notif;
#ifdef AP
	uint16 arqfstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);
	uint16 bcnfstbmp = FT2BMP(FC_BEACON) | FT2BMP(FC_PROBE_RESP);
#endif // endif

	if ((csam = MALLOCZ(wlc->osh, sizeof(wlc_csa_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	csam->wlc = wlc;

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((csam->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(wlc_csa_t *),
	                wlc_csa_bsscfg_init, wlc_csa_bsscfg_deinit, wlc_csa_bsscfg_dump,
	                (void *)csam)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* reserve cubby in the scb container for per-scb private data */
	if ((csam->scbh = wlc_scb_cubby_reserve(wlc, sizeof(wlc_csa_scb_cubby_t *),
	                wlc_csa_scb_init, wlc_csa_scb_deinit, wlc_csa_scb_dump,
	                (void *)csam)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_scb_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register bsscfg up/down callbacks */
	if (wlc_bsscfg_updown_register(wlc, wlc_csa_bsscfg_up_down, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register IE mgmt callback */
#ifdef AP
	/* bcn/prbrsp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_CHANNEL_SWITCH_ID,
	      wlc_csa_calc_csa_ie_len, wlc_csa_write_csa_ie, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, csa in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_EXT_CSA_ID,
	      wlc_csa_calc_ext_csa_ie_len, wlc_csa_write_ext_csa_ie, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, ext csa in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* assoc/reassoc req */
	if (wlc_iem_vs_add_parse_fn(wlc->iemi, arqfstbmp, WLC_IEM_VS_IE_PRIO_BRCM_PSTA,
	                                wlc_assoc_parse_psta_ie, wlc) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, psta in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#ifdef WL11AC
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_CHANNEL_SWITCH_WRAPPER_ID,
	      wlc_csa_calc_csw_ie_len, wlc_csa_write_csw_ie, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, csw in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_ier_add_build_fn(wlc->ier_csw, DOT11_MNG_WIDE_BW_CHANNEL_SWITCH_ID,
	      wlc_csa_csw_calc_wide_bw_ie_len, wlc_csa_csw_write_wide_bw_ie, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, wbw in csw ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* WL11AC */
#endif /* AP */
#ifdef STA
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_CHANNEL_SWITCH_ID,
	                         wlc_csa_bcn_parse_csa_ie, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, csa in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_EXT_CSA_ID,
	                         wlc_csa_bcn_parse_ext_csa_ie, csam) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, ext csa in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif /* STA */

	/* keep the module registration the last other add module unregistratin
	 * in the error handling code below...
	 */
	if (wlc_module_register(wlc->pub, wlc_csa_iovars, "csa", csam, wlc_csa_doiovar,
	                        NULL, wlc_csa_up, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};
	notif = wlc->notif;
	ASSERT(notif != NULL);
	if (bcm_notif_create_list(notif, &csam->csa_notif_hdl) != BCME_OK) {
		WL_ERROR(("wl%d: %s: csa bcm_notif_create_list() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	return csam;

	/* error handling */
fail:
	if (csam != NULL)
		MFREE(wlc->osh, csam, sizeof(wlc_csa_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_csa_detach)(wlc_csa_info_t *csam)
{
	wlc_info_t *wlc = csam->wlc;

	wlc_module_unregister(wlc->pub, "csa", csam);

	/* unregister bsscfg up/down callbacks */
	wlc_bsscfg_updown_unregister(wlc, wlc_csa_bsscfg_up_down, csam);
	if (csam->csa_notif_hdl != NULL) {
		bcm_notif_delete_list(&csam->csa_notif_hdl);
	}
	MFREE(wlc->osh, csam, sizeof(wlc_csa_info_t));
}

static const uint8 BCMINITDATA(csw_ie_tags)[] = {
	DOT11_MNG_WIDE_BW_CHANNEL_SWITCH_ID,
	DOT11_MNG_VHT_TRANSMIT_POWER_ENVELOPE_ID,
};

static int
wlc_csa_up(void *ctx)
{
#ifdef WL11AC
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;

	/* ignore the return code */
	(void)wlc_ier_sort_cbtbl(wlc->ier_csw, csw_ie_tags, sizeof(csw_ie_tags));
#endif // endif

	return BCME_OK;
}

static int
wlc_csa_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
	int32 int_val2 = 0;
	int32 *ret_int_ptr;
	bool bool_val;
	bool bool_val2;
#ifdef AP
	wlc_bsscfg_t *apcfg;
	int idx;
#endif // endif

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
	BCM_REFERENCE(ret_int_ptr);

	bool_val = (int_val != 0) ? TRUE : FALSE;
	bool_val2 = (int_val2 != 0) ? TRUE : FALSE;
	BCM_REFERENCE(bool_val);
	BCM_REFERENCE(bool_val2);

	/* update wlcif pointer */
	if (wlcif == NULL)
		wlcif = bsscfg->wlcif;
	ASSERT(wlcif != NULL);

	/* Do the actual parameter implementation */
	switch (actionid) {
#ifdef AP
	case IOV_SVAL(IOV_CHANSPEC_SWITCH): {
		wl_chan_switch_t *csa = (wl_chan_switch_t *)arg;
#ifdef WL_AP_CHAN_CHANGE_EVENT
		bool sent = FALSE;
#endif /* WL_AP_CHAN_CHANGE_EVENT */

		if (wlc_dfs_scan_in_progress(wlc->dfs)) {
			return BCME_BUSY;
		}

		if (wf_chspec_malformed(csa->chspec)) {
			err = BCME_BADCHAN;
			break;
		}

#ifdef BCMDBG
		if (BSSCFG_STA(bsscfg)) {
			if (!bsscfg->up) {
				err = BCME_NOTREADY;
				break;
			}

			wlc_send_action_switch_channel_ex(csam, bsscfg, &bsscfg->BSSID,
				csa, DOT11_SM_ACTION_CHANNEL_SWITCH);
			break;
		}
#endif /* BCMDBG */

		FOREACH_UP_AP(wlc, idx, apcfg) {
			err = wlc_csa_do_channel_switch(csam, apcfg,
			  csa->chspec, csa->mode, csa->count, csa->reg, csa->frame_type);
#ifdef WL_AP_CHAN_CHANGE_EVENT
			if ((err == BCME_OK) && (!sent) &&
				(wlc_radar_chanspec(wlc->cmi, csa->chspec) == TRUE)) {

				wlc_channel_send_chan_event(wlc, WL_CHAN_REASON_CSA, csa->chspec);
				/* send event to User Only once */
				sent = TRUE;
			}
#endif /* WL_AP_CHAN_CHANGE_EVENT */
		}

		break;
	}
#endif /* AP */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* bsscfg cubby */
static int
wlc_csa_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t **pcsa = CSA_BSSCFG_CUBBY_LOC(csam, cfg);
	wlc_csa_t *csa;
	int err;

	if ((csa = MALLOCZ(wlc->osh, sizeof(wlc_csa_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		err = BCME_NOMEM;
		goto fail;
	}
	*pcsa = csa;

	/* init CSA timer */
	if ((csa->csa_timer =
	     wl_init_timer(wlc->wl, wlc_csa_timeout, (void *)cfg, "csa")) == NULL) {
		WL_ERROR(("wl%d: %s: wl_init_timer failed\n",
		          wlc->pub->unit, __FUNCTION__));
		err = BCME_NORESOURCE;
		goto fail;
	}

	return BCME_OK;

fail:
	if (csa != NULL)
		wlc_csa_bsscfg_deinit(ctx, cfg);
	return err;
}

static void
wlc_csa_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t **pcsa = CSA_BSSCFG_CUBBY_LOC(csam, cfg);
	wlc_csa_t *csa = *pcsa;

	if (csa == NULL) {
		WL_ERROR(("wl%d: %s: CSA info not found\n", wlc->pub->unit, __FUNCTION__));
		return;
	}

	/* delete CSA timer */
	if (csa->csa_timer != NULL) {
		wl_free_timer(wlc->wl, csa->csa_timer);
		csa->csa_timer = NULL;
	}

	MFREE(wlc->osh, csa, sizeof(wlc_csa_t));
	*pcsa = NULL;
}

static int
wlc_csa_scb_init(void *ctx, struct scb *scb)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_scb_cubby_t **pcsa_scb = CSA_SCB_CUBBY_LOC(csam, scb);
	wlc_csa_scb_cubby_t *csa_scb;

	if ((csa_scb = MALLOCZ(wlc->osh, sizeof(wlc_csa_scb_cubby_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return BCME_NOMEM;
	}

	*pcsa_scb = csa_scb;

	return BCME_OK;
}

static void
wlc_csa_scb_deinit(void *ctx, struct scb *scb)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_scb_cubby_t **pcsa_scb = CSA_SCB_CUBBY_LOC(csam, scb);
	wlc_csa_scb_cubby_t *csa_scb = *pcsa_scb;

	if (csa_scb != NULL)
		MFREE(wlc->osh, csa_scb, sizeof(wlc_csa_scb_cubby_t));

	*pcsa_scb = NULL;
}

#ifdef BCMDBG
static void
wlc_csa_bsscfg_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);

	ASSERT(csa != NULL);

	/* CSA info */
	bcm_bprintf(b, "\tcsa->csa_timer %p\n", csa->csa_timer);
	bcm_bprintf(b, "\tcsa->csa.mode %d, csa->csa.count %d\n",
	            csa->csa.mode, csa->csa.count);
	bcm_bprintf(b, "\tcsa->csa.chspec 0x%x, csa->csa.reg %d \n",
	            csa->csa.chspec, csa->csa.reg);
}

static void
wlc_csa_scb_dump(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_csa_scb_cubby_t *csa_scb = CSA_SCB_CUBBY(csam, scb);

	ASSERT(csa_scb != NULL);

	/* CSA info */
	bcm_bprintf(b, "     csa_scb->dcs_relocation_state %d, csa_scb->dcs_ack_counter %d\n",
	            csa_scb->dcs_relocation_state, csa_scb->dcs_ack_counter);
}
#endif /* BCMDBG */

static void
wlc_csa_bsscfg_up_down(void *ctx, bsscfg_up_down_event_data_t *evt_data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, evt_data->bsscfg);

	/* Only process bsscfg down events. */
	if (!evt_data->up) {
		ASSERT(csa != NULL);

		/* cancel any csa timer */
		evt_data->callbacks_pending =
		   (wl_del_timer(wlc->wl, csa->csa_timer) ? 0 : 1);
	}
}

static void
wlc_csa_unicast_tx_complete(wlc_info_t *wlc, uint txstatus, void *pkt)
{
	struct scb *scb;
	wlc_csa_info_t *csam = wlc->csa;
	wlc_csa_scb_cubby_t *csa_scb = NULL;

	/* make sure the scb still exists */
	if ((scb = WLPKTTAGSCBGET(pkt)) == NULL) {
		WL_ERROR(("wl%d: %s: unable to find scb from the pkt %p\n",
		          wlc->pub->unit, __FUNCTION__, pkt));
		return;
	}

	csa_scb = CSA_SCB_CUBBY(csam, scb);
	ASSERT(csa_scb != NULL);

	if (csa_scb->dcs_relocation_state == CSA_UNICAST_RELOCATION_PENDING) {
		csa_scb->dcs_ack_counter++;
		if (csa_scb->dcs_ack_counter == CSA_UNICAST_ACK_COUNTER)
			csa_scb->dcs_relocation_state = CSA_UNICAST_RELOCATION_SUCCESS;
	}
}

#ifdef WL_CS_RESTRICT_RELEASE
#define WL_CS_TRAIN_TXPOWER_PKTSNUM	16
static bool
wlc_csa_train_txpower_sendpkt(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *da)
{
	return wlc_sendnulldata(wlc, cfg, da, 0, 0, PRIO_8021D_VO, NULL, NULL);
}
static void
wlc_csa_train_txpower(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = cfg->wlc;
	int count = WL_CS_TRAIN_TXPOWER_PKTSNUM;

	STATIC_ASSERT(WL_CS_TRAIN_TXPOWER_PKTSNUM > 0);

	if (BSSCFG_STA(cfg)) {
		if (!BSSCFG_PSTA(cfg) && cfg->BSS) {
			while (wlc_csa_train_txpower_sendpkt(wlc, cfg, &cfg->BSSID) && --count);
		}
	} else if (BSSCFG_AP(cfg)) {
		bool can_again;
		do {
			struct scb *scb;
			struct scb_iter scbiter;

			can_again = FALSE;

			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
				if (SCB_ASSOCIATED(scb) || SCB_LEGACY_WDS(scb)) {
					can_again = wlc_csa_train_txpower_sendpkt(wlc,
						cfg, &scb->ea) ? (--count != 0) : FALSE;
					if (!can_again) {
						break;
					}
				}
			}
		} while (can_again);
	}
}
#else
#define wlc_csa_train_txpower(csam, cfg)
#endif /* WL_CS_RESTRICT_RELEASE */

static void
wlc_csa_timeout(void *arg)
{
	wlc_bsscfg_t *cfg = (wlc_bsscfg_t *)arg;
	wlc_info_t *wlc = cfg->wlc;
	wlc_csa_info_t *csam = wlc->csa;
#ifdef STA
	wlc_bsscfg_t *active_assoc_cfg = wlc->as_info->assoc_req[0];
#endif // endif
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
	struct scb_iter scbiter;
	struct scb *scb;
	bool switch_chnl = TRUE;

	ASSERT(csa != NULL);

	if (!wlc->pub->up)
		return;

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", wlc->pub->unit, __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}

#ifdef STA
	if ((active_assoc_cfg != NULL) &&
	    ((active_assoc_cfg->assoc->state == AS_WAIT_FOR_AP_CSA) ||
	     (active_assoc_cfg->assoc->state == AS_WAIT_FOR_AP_CSA_ROAM_FAIL))) {
		wlc_11h_set_spect_state(wlc->m11h, active_assoc_cfg, NEED_TO_SWITCH_CHANNEL, 0);
		wlc_ap_mute(wlc, TRUE, active_assoc_cfg, -1);
		if (active_assoc_cfg->assoc->state == AS_WAIT_FOR_AP_CSA) {
			wlc_join_BSS(active_assoc_cfg,
				wlc->as_info->join_targets->ptrs[wlc->as_info->join_targets_last]);
		}
		else {
			wlc_roam_complete(active_assoc_cfg, WLC_E_STATUS_FAIL,
			                  &active_assoc_cfg->BSSID,
			                  WLC_DOT11_BSSTYPE(active_assoc_cfg->target_bss->infra));
		}
		return;
	}
#endif /* STA */

	if (BSSCFG_AP(cfg) && (csa->csa.frame_type == CSA_UNICAST_ACTION_FRAME)) {
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
			wlc_csa_scb_cubby_t *csa_scb = CSA_SCB_CUBBY(csam, scb);
			ASSERT(csa_scb != NULL);

			if (!SCB_ISMULTI(scb) && (SCB_ASSOCIATED(scb) || SCB_LEGACY_WDS(scb)) &&
				(scb->psta_prim == NULL) &&
				(csa_scb->dcs_relocation_state != CSA_UNICAST_RELOCATION_SUCCESS)) {
#if defined(BCMDBG)
				char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
				WL_CHANINT(("dcs: csa ack pending from %s\n",
				            bcm_ether_ntoa(&scb->ea, eabuf)));
				switch_chnl = FALSE;
				break;
			}
		}
	}

	if (BSSCFG_AP(cfg) && (csa->csa.frame_type == CSA_UNICAST_ACTION_FRAME)) {
		/* When multiple STAs assoc to AP, due to interference, some STA
		 * maynot ack csa. checking all the STAs acking csa would cause
		 * AP cannot change channel. In order to avoid this case, skip
		 * checking if all STA acks csa, to have a quick fix. Checking
		 * all STAs ack csa will be used for csa retry to make csa
		 * more robust (further enhancement)
		 */
		/* XXX: Consider WDS link status for sending unicast CSA action frames
		 * if checking acks for CSA frames of all stations.
		 */
		/* if (switch_chnl) */
		{
			/* time to switch channels... */
#ifdef WL_DFS_WAVE_MODE
			if (!wlc_dfs_test_mode(wlc->dfs))
#endif /* WL_DFS_WAVE_MODE */
			{
				wlc_do_chanswitch(cfg, csa->csa.chspec);
			}
			wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_SWITCH_CHANNEL, 0);
			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
				wlc_csa_scb_cubby_t *csa_scb = CSA_SCB_CUBBY(csam, scb);
				ASSERT(csa_scb != NULL);

				if ((SCB_ASSOCIATED(scb) || SCB_LEGACY_WDS(scb)) &&
					!SCB_ISMULTI(scb) && (scb->psta_prim == NULL)) {
					csa_scb->dcs_relocation_state = CSA_UNICAST_RELOCATION_NONE;
					csa_scb->dcs_ack_counter = 0;
				}
			}
			wlc_csa_train_txpower(csam, cfg);
		}
	}
	else if (((BSSCFG_AP(cfg) || csa->csa.count == 0 || csa->channel_sw.pending) ||
		(PSTA_ENAB(wlc->pub) && BSSCFG_STA(cfg))) &&
	    (wlc_11h_get_spect_state(wlc->m11h, cfg) & NEED_TO_SWITCH_CHANNEL)) {
#ifdef WL_MULTIQUEUE
		chanspec_t cur_chspec = WLC_BAND_PI_RADIO_CHANSPEC;
#endif /* WL_MULTIQUEUE */
		chanspec_t tgt_chspec = csa->csa.chspec;
		if (BSSCFG_STA(cfg)) {
			csa->channel_sw.pending = FALSE;
			csa->csa.count = 0;
		}

#ifdef WL_MULTIQUEUE
		/* For CSA to 160Mhz channels from 80Mhz, check for pending packets on the fifo
		 * and flush the fifos
		 */
		if (DYN160_ACTIVE(wlc->pub) && csa->csa.mode == DOT11_CSA_MODE_ADVISORY &&
				(CHSPEC_IS80(cur_chspec) && CHSPEC_IS160(tgt_chspec))) {
			wlc->txfifo_detach_transition_queue = wlc->active_queue;
			wlc->txfifo_detach_pending = TRUE;
			wlc_bmac_tx_fifo_sync(wlc->hw, BITMAP_FLUSH_ALL_TX_FIFOS, FLUSHFIFO);
		}
#endif /* WL_MULTIQUEUE */

		/* time to switch channels... */
#ifdef WL_DFS_WAVE_MODE
		if (!wlc_dfs_test_mode(wlc->dfs))
#endif /* WL_DFS_WAVE_MODE */
		{
			wlc_do_chanswitch(cfg, tgt_chspec);
		}
		wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_SWITCH_CHANNEL, 0);
		wlc_csa_train_txpower(csam, cfg);
	}

	if ((BSSCFG_AP(cfg) && (csa->csa.frame_type == CSA_UNICAST_ACTION_FRAME)) && !switch_chnl) {
		wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_SWITCH_CHANNEL, 0);
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
			wlc_csa_scb_cubby_t *csa_scb = CSA_SCB_CUBBY(csam, scb);
			ASSERT(csa_scb != NULL);

			if (!SCB_ISMULTI(scb) && (SCB_ASSOCIATED(scb) || SCB_LEGACY_WDS(scb)) &&
				(scb->psta_prim == NULL)) {
				csa_scb->dcs_relocation_state = CSA_UNICAST_RELOCATION_NONE;
				csa_scb->dcs_ack_counter = 0;
			}
		}
	}

	/* Send Channel Switch Indication to upper (OS) layer. */
	if (BSSCFG_STA(cfg) || (BSSCFG_AP(cfg) && switch_chnl))
		wlc_bss_mac_event(wlc, cfg, WLC_E_CSA_COMPLETE_IND, NULL,
			WLC_E_STATUS_SUCCESS, 0, 0, &csa->csa.mode, sizeof(csa->csa.mode));
	wlc_csa_obss_dynbw_notif_cb_notif(csam, cfg,
		BCME_OK, CSA_CHANNEL_CHANGE_END, csa->csa.chspec);
}

#ifdef STA
/* STA: Handle incoming Channel Switch Anouncement */
static void
wlc_csa_process_channel_switch(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_bss_info_t *current_bss = cfg->current_bss;
	wlcband_t *band;
	uint chanswitch_time;
	DBGONLY(char chanbuf[CHANSPEC_STR_LEN]; )

	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);

	ASSERT(csa != NULL);
	band = wlc->bandstate[CHSPEC_IS2G(csa->csa.chspec) ? BAND_2G_INDEX : BAND_5G_INDEX];

	/* add 160 and 80+80 */
	if (band && CHSPEC_IS8080(csa->csa.chspec) &&
	    (!VHT_ENAB_BAND(wlc->pub, band->bandtype) ||
	     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_160MHZ) ||
	     !WL_BW_CAP_160MHZ(band->bw_cap))) {
			csa->csa.chspec = wf_chspec_primary80_chspec(csa->csa.chspec);
	}

	if (band && CHSPEC_IS160(csa->csa.chspec) &&
	    (!VHT_ENAB_BAND(wlc->pub, band->bandtype) ||
	     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_160MHZ) ||
	     !WL_BW_CAP_160MHZ(band->bw_cap))) {
			csa->csa.chspec = wf_chspec_primary80_chspec(csa->csa.chspec);
	}

	/* Sanitize user setting for 80MHz against current settings
	* Reduce an 80MHz chanspec to 40MHz if needed.
	*/
	if (wf_chspec_malformed(csa->csa.chspec)) {
		WL_REGULATORY(("wl%d: %s: malformed csa channel ignore %x\n",
			wlc->pub->unit, __FUNCTION__, csa->csa.chspec));
		/* if malformed, below calls to change into lower bw won't work */
		goto ignore_csa;
	}

	if (CHSPEC_IS80(csa->csa.chspec) &&
		!VALID_80CHANSPEC(wlc, csa->csa.chspec)) {
		csa->csa.chspec = wf_chspec_primary40_chspec(csa->csa.chspec);
	}

	/* Convert a 40MHz AP channel to a 20MHz channel if we are not in NMODE or
	 * the locale does not allow 40MHz
	 * or the band is not configured for 40MHz operation
	 */
	if (band && CHSPEC_IS40(csa->csa.chspec) &&
	    (!N_ENAB(wlc->pub) ||
	     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_40MHZ) ||
	     !WL_BW_CAP_40MHZ(band->bw_cap))) {
		csa->csa.chspec = CH20MHZ_CHSPEC(wf_chspec_ctlchan((csa->csa.chspec)));
	}

	if (!wlc_valid_chanspec_db(wlc->cmi, csa->csa.chspec)) {
		WL_REGULATORY(("wl%d: %s: Received invalid chanspec: %s\n",
			wlc->pub->unit, __FUNCTION__, wf_chspec_ntoa_ex(csa->csa.chspec, chanbuf)));
		/* Received bogus, malformed or otherwise screwed CSA.
		 * Err on the side of caution and goto an active A band channel now
		 */
		csa->csa.chspec = wlc_next_chanspec(wlc->cmi,
			CH20MHZ_CHSPEC((CH_MAX_2G_CHANNEL+1)), CHAN_TYPE_CHATTY, 0);
		if (csa->csa.chspec == wlc->home_chanspec)
			csa->csa.chspec =
				wlc_next_chanspec(wlc->cmi, csa->csa.chspec, CHAN_TYPE_CHATTY, 0);
		csa->csa.count = 1;

		if (csa->csa.chspec == INVCHANSPEC) {
			WL_REGULATORY(("wl%d: %s: compute new channel failed\n",
				wlc->pub->unit, __FUNCTION__));
			/* exhausted options to accomodate csa - ignore as it is bogus */
			goto ignore_csa;
		}
	}

	if (csa->csa.mode && WL11H_ENAB(wlc) && !CHSPEC_IS2G(wlc->home_chanspec)) {
		wlc_set_quiet_chanspec(wlc->cmi, wlc->home_chanspec);
		if (WLC_BAND_PI_RADIO_CHANSPEC == wlc->home_chanspec) {
			WL_REGULATORY(("%s: Muting now\n", __FUNCTION__));
			wlc_mute(wlc, ON, 0);
		}
	}

	if (csa->channel_sw.pending == TRUE) {
		/* timer for channel switch already started */
		WL_REGULATORY(("%s: ignoring csa: mode %d, chanspec %s, count %d\n",
			__FUNCTION__, csa->csa.mode, wf_chspec_ntoa_ex(csa->csa.chspec, chanbuf),
			csa->csa.count));
		return;
	}

	WL_REGULATORY(("wl%d.%d: %s: Recved CSA: mode %d, chanspec %s, count %d\n",
	               wlc->pub->unit, cfg->_idx, __FUNCTION__,
	               csa->csa.mode, wf_chspec_ntoa_ex(csa->csa.chspec, chanbuf), csa->csa.count));

	wlc_csa_obss_dynbw_notif_cb_notif(csam, cfg,
			BCME_OK, CSA_CHANNEL_CHANGE_START, csa->csa.chspec);

	if (csa->csa.count == 0) {
		wl_del_timer(wlc->wl, csa->csa_timer);
		WL_REGULATORY(("%s: switch now\n", __FUNCTION__));

		wlc_do_chanswitch(cfg, csa->csa.chspec);

		/* Send Channel Switch Indication to upper (OS) layer. */
		wlc_bss_mac_event(wlc, cfg, WLC_E_CSA_COMPLETE_IND, NULL,
			WLC_E_STATUS_SUCCESS, 0, 0, &csa->csa.mode, sizeof(csa->csa.mode));

		wlc_csa_train_txpower(csam, cfg);
		wlc_csa_obss_dynbw_notif_cb_notif(csam, cfg,
			BCME_OK, CSA_CHANNEL_CHANGE_END, csa->csa.chspec);
		return;
	}

	WL_REGULATORY(("%s: Scheduling channel switch in %d tbtts\n", __FUNCTION__,
	               csa->csa.count));

	csa->channel_sw.pending = TRUE;
	csa->channel_sw.chanspec = csa->csa.chspec;
	chanswitch_time = (csa->csa.count * current_bss->beacon_period * DOT11_TU_TO_US) / 1000;

	wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_SWITCH_CHANNEL, NEED_TO_SWITCH_CHANNEL);
	wl_del_timer(wlc->wl, csa->csa_timer);
	wl_add_timer(wlc->wl, csa->csa_timer, chanswitch_time, 0);

#ifdef AP
	/* If this STA connected to up-stream AP and has an overlaping chanspec
	 * with a local AP, propagate csa to downstream STA's (for local AP).
	 */
	if (
#ifdef WLMCHAN
		(!MCHAN_ENAB(wlc->pub) || wlc_mchan_stago_is_disabled(wlc->mchan)) &&
#endif // endif
		WL11H_AP_ENAB(wlc))
	{
		wlc_bsscfg_t *ap_bsscfg;
		int idx;
		FOREACH_UP_AP(wlc, idx, ap_bsscfg) {

			if (ap_bsscfg->current_bss->chanspec == cfg->current_bss->chanspec) {
				wlc_csa_t *apcsa = CSA_BSSCFG_CUBBY(csam, ap_bsscfg);

				/* copy over the csa info */
				bcopy(&csa->csa, &apcsa->csa, sizeof(wl_chan_switch_t));
				/* decrement by 1 since this is for our next beacon */
				apcsa->csa.count--;
				/* send csa action frame and update bcn, prb rsp */
				wlc_csa_apply_channel_switch(csam, ap_bsscfg);
			}
		}
	}
#endif /* AP */
	return;
ignore_csa:
	csa->csa.count = 0;
	return;
}
#endif /* STA */

#ifdef WL11AC

/* Use wide bw channel switch IE, HT (<=40MHz) chanspec and cfg (optionally to use HT CCFS2
 * to get wide bw (>=80MHz) chanspec if specified by the IE and CCFS2
 */
static chanspec_t
wlc_csa_get_wide_chanspec(dot11_wide_bw_chan_switch_ie_t *wide_bw_ie, chanspec_t ht_chanspec,
		wlc_bsscfg_t *cfg)
{
	uint16 ht_ctl = wf_chspec_ctlchan(ht_chanspec);
#if defined(WL11AC_160) || defined(WL11AC_80P80)
	uint8 chan0 = wide_bw_ie->center_frequency_segment_0;
#endif /* WL11AC_160 || WL11AC_80P80 */
	uint8 chan1 = wide_bw_ie->center_frequency_segment_1;

	BCM_REFERENCE(ht_ctl);

	if (cfg != NULL && cfg->current_bss != NULL && cfg->current_bss->ht_ccfs2 != 0) {
		chan1 = cfg->current_bss->ht_ccfs2;
	}
	switch (wide_bw_ie->channel_width) {
		case VHT_OP_CHAN_WIDTH_80:
			if (chan1 == 0) { /* implies 80MHz */
				return wf_channel2chspec(ht_ctl, WL_CHANSPEC_BW_80);
			}
#if defined(WL11AC_160) || defined(WL11AC_80P80)
			if (chan1 == 50 || chan1 == 114) {
				return wf_channel2chspec(ht_ctl, WL_CHANSPEC_BW_160);
			}
			return wf_chspec_get8080_chspec(ht_ctl, chan0, chan1);
		case VHT_OP_CHAN_WIDTH_160:
			return wf_channel2chspec(ht_ctl, WL_CHANSPEC_BW_160);
		case VHT_OP_CHAN_WIDTH_80_80:
			return wf_chspec_get8080_chspec(ht_ctl, chan0, chan1);
#endif /* WL11AC_160 || WL11AC_80P80 */
		case VHT_OP_CHAN_WIDTH_20_40: /* fall-through */
		default:
			return ht_chanspec;
	}
}

#endif /* WL11AC */

#ifdef WL11N
void
wlc_recv_public_csa_action(wlc_csa_info_t *csam, struct dot11_management_header *hdr,
	uint8 *body, int body_len)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_bsscfg_t *cfg;
	struct dot11y_action_ext_csa *action_hdr;
	uint8 extch;
	wlc_csa_t *csa;
	wl_chan_switch_t cs_req = {0, 0, 0, 0, 0};
#ifdef PSTA
	int32 idx;
	wl_chan_switch_t pcsa;
#endif /* PSTA */
#ifdef WL11AC
	dot11_wide_bw_chan_switch_ie_t *wide_bw_chan_switch_ie;
	bcm_tlv_t *ies;
	uint ies_len;
#endif /* WL11AC */
	struct scb *scb;

	if ((cfg = wlc_bsscfg_find_by_bssid(wlc, &hdr->bssid)) == NULL &&
	    (cfg = wlc_bsscfg_find_by_hwaddr(wlc, &hdr->da)) == NULL) {
#if defined(BCMDBG) || defined(BCMDBG_ERR)
		char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
		WL_ERROR(("wl%d: %s: Unable to find bsscfg for %s\n",
		          wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(&hdr->sa, eabuf)));
		return;
	}

	if (WL11H_ENAB(wlc) &&
		wlc_11h_get_spect_state(wlc->m11h, cfg) & NEED_TO_SWITCH_CHANNEL) {
		WL_REGULATORY(("wl%d:%s Already have a scheduled channel switch!\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	}

	scb = wlc_scbfind(wlc, cfg, &hdr->sa);
	if (!BSSCFG_STA(cfg) && !(scb && SCB_LEGACY_WDS(scb))) {
		WL_ERROR(("wl%d.%d: %s: not a STA and not from WDS client\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		return;
	}

	if (body_len < (int)(sizeof(struct dot11y_action_ext_csa))) {
		WL_ERROR(("wl%d: %s: Invalid len %d < %d\n",	wlc->pub->unit, __FUNCTION__,
			body_len, (int)(TLV_HDR_LEN + sizeof(struct dot11_csa_body))));
		return;
	}

	action_hdr = (struct dot11y_action_ext_csa *)body;
	/* valid the IE in this action frame */
	WL_INFORM(("wl%d: %s: mode %d, reg %d, channel %d, count %d\n",
		wlc->pub->unit, __FUNCTION__, action_hdr->b.mode, action_hdr->b.reg,
		action_hdr->b.channel, action_hdr->b.count));

	cs_req.mode = action_hdr->b.mode;
	cs_req.count = action_hdr->b.count;
	extch = wlc_rclass_extch_get(wlc->cmi, action_hdr->b.reg);
#ifdef WL11ULB
	cs_req.chspec = wlc_ht_chanspec(wlc, action_hdr->b.channel, extch, cfg);
#else /* WL11ULB */
	cs_req.chspec = wlc_ht_chanspec(wlc, action_hdr->b.channel, extch);
#endif /* WL11ULB */
#ifdef WL11AC
	if (body_len > (int)(sizeof(struct dot11y_action_ext_csa))) {
		ies = (bcm_tlv_t*) (body + (sizeof(struct dot11y_action_ext_csa)));
		ies_len = body_len - (sizeof(struct dot11y_action_ext_csa));
		wide_bw_chan_switch_ie = (dot11_wide_bw_chan_switch_ie_t*)
			bcm_parse_tlvs_min_bodylen(ies, ies_len,
			DOT11_MNG_WIDE_BW_CHANNEL_SWITCH_ID,
			DOT11_WIDE_BW_SWITCH_IE_LEN);

		if (wide_bw_chan_switch_ie == NULL) {
			WL_REGULATORY(("wl%d: %s: Bad CSA Mngmt Action frame"
			               "(Wide Bandwidth Channel Switch IE)\n",
			               wlc->pub->unit, __FUNCTION__));
			WLCNTINCR(wlc->pub->_cnt->rxbadproto);
			wlc_send_action_err(wlc, hdr, body, body_len);
			return;
		}
		WL_REGULATORY(("wl%d: %s: WBchannel width %d, WBCFS0 %d, WBCFS1 %d"
				" control channel %d\n", wlc->pub->unit, __FUNCTION__,
				wide_bw_chan_switch_ie->channel_width,
				wide_bw_chan_switch_ie->center_frequency_segment_0,
				wide_bw_chan_switch_ie->center_frequency_segment_1,
				action_hdr->b.channel));
		cs_req.chspec = wlc_csa_get_wide_chanspec(wide_bw_chan_switch_ie,
				cs_req.chspec, cfg);
	}
#endif /* WL11AC */

	cs_req.reg = action_hdr->b.reg;

#ifdef WDS
	if (scb && SCB_LEGACY_WDS(scb)) {
		wlc_wds_process_csa(wlc, SCB_BSSCFG(scb), &cs_req);
		return;
	}
#endif /* WDS */

	csa = CSA_BSSCFG_CUBBY(csam, cfg);
	ASSERT(csa != NULL);
	memcpy(&csa->csa, &cs_req, sizeof(wl_chan_switch_t));

#ifdef PSTA
	pcsa = csa->csa;
#endif /* PSTA */
#ifdef STA
	if (cfg->associated &&
	    bcmp(&hdr->bssid, &cfg->BSSID, ETHER_ADDR_LEN) == 0) {
		wlc_csa_process_channel_switch(csam, cfg);
	}
#endif /* STA */

#ifdef PSTA
	/* Process public csa action for each associated proxysta bsscfg */
	FOREACH_PSTA(wlc, idx, cfg) {
		if (cfg->associated &&
			bcmp(&hdr->bssid, &cfg->BSSID, ETHER_ADDR_LEN) == 0) {
			CSA_BSSCFG_CUBBY(csam, cfg)->csa = pcsa;
			wlc_csa_process_channel_switch(csam, cfg);
		}
	}
#endif /* PSTA */
}
#endif /* WL11N */

static uint8 *
wlc_write_csa_body(wl_chan_switch_t *cs, uint8 *cp)
{
	struct dot11_csa_body *b = (struct dot11_csa_body *)cp;

	b->mode = cs->mode;
	b->reg = cs->reg;
	b->channel = wf_chspec_ctlchan(cs->chspec);
	b->count = cs->count;
	cp += sizeof(struct dot11_csa_body);

	return cp;
}

static uint8 *
wlc_write_ext_csa_ie(wl_chan_switch_t *cs, uint8 *cp)
{
	dot11_ext_csa_ie_t *chan_switch_ie = (dot11_ext_csa_ie_t *)cp;

	chan_switch_ie->id = DOT11_MNG_EXT_CSA_ID;
	chan_switch_ie->len = DOT11_EXT_CSA_IE_LEN;
	cp += TLV_HDR_LEN;
	cp = wlc_write_csa_body(cs, cp);

	return cp;
}

static uint8 *
wlc_write_csa_ie(wl_chan_switch_t *cs, uint8 *cp, int buflen)
{
	dot11_chan_switch_ie_t *chan_switch_ie;

	/* perform buffer length check. */
	/* if not big enough, return buffer untouched */
	BUFLEN_CHECK_AND_RETURN((TLV_HDR_LEN + DOT11_SWITCH_IE_LEN), buflen, cp);

	chan_switch_ie = (dot11_chan_switch_ie_t *)cp;
	chan_switch_ie->id = DOT11_MNG_CHANNEL_SWITCH_ID;
	chan_switch_ie->len = DOT11_SWITCH_IE_LEN;
	chan_switch_ie->mode = cs->mode;
	chan_switch_ie->channel = wf_chspec_ctlchan(cs->chspec);
	chan_switch_ie->count = cs->count;
	cp += (TLV_HDR_LEN + DOT11_SWITCH_IE_LEN);

	return cp;
}

#ifdef WL11AC
#ifdef AP
static uint
wlc_calc_chan_switch_wrapper_ie_len(wlc_csa_info_t *csam, wl_chan_switch_t *cs,
	wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;

	/* prepare IE mgmt calls */
	bzero(&ftcbparm, sizeof(ftcbparm));
	ftcbparm.csw.chspec = cs->chspec;
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;

	/* calc IEs' length */
	return TLV_HDR_LEN + wlc_ier_calc_len(wlc->ier_csw, cfg, 0, &cbparm);
}
#endif /* AP */

static uint8 *
wlc_write_chan_switch_wrapper_ie(wlc_csa_info_t *csam, wl_chan_switch_t *cs,
	wlc_bsscfg_t *cfg, uint8 *cp, int buflen)
{
	wlc_info_t *wlc = csam->wlc;
	dot11_chan_switch_wrapper_ie_t *chan_switch_wrapper;
	uint cp_len;
	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;

	/* prepare IE mgmt calls */
	bzero(&ftcbparm, sizeof(ftcbparm));
	ftcbparm.csw.chspec = cs->chspec;
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;

	/* calc IEs' length */
	cp_len = wlc_ier_calc_len(wlc->ier_csw, cfg, 0, &cbparm);
	if (TLV_HDR_LEN + cp_len >= (uint)buflen)
		return cp;

	chan_switch_wrapper = (dot11_chan_switch_wrapper_ie_t *)cp;
	chan_switch_wrapper->id = DOT11_MNG_CHANNEL_SWITCH_WRAPPER_ID;
	chan_switch_wrapper->len = (uint8)cp_len;

	cp += TLV_HDR_LEN;

	/* build IEs */
	if (wlc_ier_build_frame(wlc->ier_csw, cfg, WLC_IEM_FC_UNK,
	                        &cbparm, cp, cp_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_build_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return cp;
	}

	return cp + cp_len;
}

uint8 *
wlc_csa_write_chan_switch_wrapper_ie(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg, uint8 *cp, int buflen)
{
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
	ASSERT(csa != NULL);

	return wlc_write_chan_switch_wrapper_ie(csam, &csa->csa, cfg, cp, buflen);
}
#endif /* WL11AC */

static uint8 *
wlc_write_extch_ie(chanspec_t chspec, uint8 *cp, int buflen)
{
	dot11_extch_ie_t *extch_ie = (dot11_extch_ie_t *)cp;
	chanspec_t chspec40;

	/* length check */
	/* if buffer too small, return untouched buffer */
	BUFLEN_CHECK_AND_RETURN((TLV_HDR_LEN + DOT11_EXTCH_IE_LEN), buflen, cp);

	extch_ie->id = DOT11_MNG_EXT_CHANNEL_OFFSET;
	extch_ie->len = DOT11_EXTCH_IE_LEN;

	if (CHSPEC_IS8080(chspec) || CHSPEC_IS160(chspec)) {
		chspec40 = wf_chspec_primary80_chspec(chspec);
		chspec40 = wf_chspec_primary40_chspec(chspec);
		extch_ie->extch = CHSPEC_SB_UPPER(chspec40) ?
		                  DOT11_EXT_CH_LOWER : DOT11_EXT_CH_UPPER;
	}
	else if (CHSPEC_IS80(chspec)) {
		chspec40 = wf_chspec_primary40_chspec(chspec);
		extch_ie->extch = CHSPEC_SB_UPPER(chspec40) ?
		                  DOT11_EXT_CH_LOWER : DOT11_EXT_CH_UPPER;
	} else if (CHSPEC_IS40(chspec)) {
		extch_ie->extch = CHSPEC_SB_UPPER(chspec) ? DOT11_EXT_CH_LOWER : DOT11_EXT_CH_UPPER;
	} else {
		ASSERT(CHSPEC_IS20(chspec));
		extch_ie->extch = DOT11_EXT_CH_NONE;
	}
	cp += (TLV_HDR_LEN + DOT11_EXTCH_IE_LEN);

	return cp;
}

#ifdef WL11AC
static uint8 *
wlc_write_wide_bw_csa_ie(wl_chan_switch_t *cs, uint8 *cp, int buflen, wlc_info_t *wlc)
{
	dot11_wide_bw_chan_switch_ie_t *wide_bw_chan_switch_ie;
	uint8 center_chan;

	/* perform buffer length check. */
	/* if not big enough, return buffer untouched */
	BUFLEN_CHECK_AND_RETURN((TLV_HDR_LEN + DOT11_WIDE_BW_SWITCH_IE_LEN), buflen, cp);

	wide_bw_chan_switch_ie = (dot11_wide_bw_chan_switch_ie_t *)cp;
	wide_bw_chan_switch_ie->id = DOT11_MNG_WIDE_BW_CHANNEL_SWITCH_ID;
	wide_bw_chan_switch_ie->len = DOT11_WIDE_BW_SWITCH_IE_LEN;

	/* updating as per D8.0 Table 9-252 */
	if (CHSPEC_IS80(cs->chspec) || CHSPEC_IS160(cs->chspec) || CHSPEC_IS8080(cs->chspec)) {
		wide_bw_chan_switch_ie->channel_width = VHT_OP_CHAN_WIDTH_80;
	} else {
		wide_bw_chan_switch_ie->channel_width = VHT_OP_CHAN_WIDTH_20_40;
	}

	wide_bw_chan_switch_ie->center_frequency_segment_1 = 0;
	if (CHSPEC_IS8080(cs->chspec)) {
		wide_bw_chan_switch_ie->center_frequency_segment_0 =
			wf_chspec_primary80_channel(cs->chspec);
		if (!DYN160_ACTIVE(wlc->pub)) {
			wide_bw_chan_switch_ie->center_frequency_segment_1 =
				wf_chspec_secondary80_channel(cs->chspec);
		}
	} else if (CHSPEC_IS160(cs->chspec)) {
		wide_bw_chan_switch_ie->center_frequency_segment_0 =
			wf_chspec_primary80_channel(cs->chspec);
		if (!DYN160_ACTIVE(wlc->pub)) {
			center_chan = CHSPEC_CHANNEL(cs->chspec) >> WL_CHANSPEC_CHAN_SHIFT;
			wide_bw_chan_switch_ie->center_frequency_segment_1 = center_chan;
		}
	} else {
		center_chan = CHSPEC_CHANNEL(cs->chspec) >> WL_CHANSPEC_CHAN_SHIFT;
		wide_bw_chan_switch_ie->center_frequency_segment_0 = center_chan;
	}

	cp += (TLV_HDR_LEN + DOT11_WIDE_BW_SWITCH_IE_LEN);

	return cp;
}
#endif /* WL11AC */

#ifdef AP
#ifdef WL11AC
/* Wide Bandwidth IE in CS Wrapper IE */
static uint
wlc_csa_csw_calc_wide_bw_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	wlc_iem_ft_cbparm_t *ftcbparm = calc->cbparm->ft;

	/* wb_csa_ie doesn't present in 20MHz channels */
	if (!CHSPEC_IS20(ftcbparm->csw.chspec))
		return TLV_HDR_LEN + DOT11_WIDE_BW_SWITCH_IE_LEN;

	return 0;
}

static int
wlc_csa_csw_write_wide_bw_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_csa_t *csa;
	wlc_iem_ft_cbparm_t *ftcbparm = build->cbparm->ft;

	csa = CSA_BSSCFG_CUBBY(csam, build->cfg);
	ASSERT(csa != NULL);

	/* wb_csa_ie doesn't present in 20MHz channels */
	if (!CHSPEC_IS20(ftcbparm->csw.chspec))
		wlc_write_wide_bw_csa_ie(&csa->csa, build->buf, build->buf_len, csam->wlc);

	return BCME_OK;
}

/* CS wrapper */
static uint
wlc_csa_calc_csw_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_csa_t *csa;

	if (!data->cbparm->vht)
		return 0;

	if (BAND_2G(wlc->band->bandtype))
		return 0;

	if (!(BSSCFG_AP(cfg) &&
	      wlc_csa_get_csa_count(csam, cfg) > 0))
		return 0;

	csa = CSA_BSSCFG_CUBBY(csam, cfg);
	ASSERT(csa != NULL);

	return wlc_calc_chan_switch_wrapper_ie_len(csam, &csa->csa, cfg);
}

static int
wlc_csa_write_csw_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_csa_t *csa;

	if (!data->cbparm->vht)
		return BCME_OK;

	if (BAND_2G(wlc->band->bandtype))
		return BCME_OK;

	if (!(BSSCFG_AP(cfg) &&
	      wlc_csa_get_csa_count(csam, cfg) > 0))
		return BCME_OK;

	csa = CSA_BSSCFG_CUBBY(csam, cfg);
	ASSERT(csa != NULL);

	wlc_write_chan_switch_wrapper_ie(csam, &csa->csa, cfg, data->buf, data->buf_len);
	return BCME_OK;
}
#endif /* WL11AC */

/* Extended CSA IE */
static uint
wlc_csa_calc_ext_csa_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (data->cbparm->ht &&
	    BSSCFG_AP(cfg) &&
	    wlc_csa_get_csa_count(csam, cfg) > 0)
		return TLV_HDR_LEN + DOT11_EXT_CSA_IE_LEN;

	return 0;
}

static int
wlc_csa_write_ext_csa_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (data->cbparm->ht &&
	    BSSCFG_AP(cfg) &&
	    wlc_csa_get_csa_count(csam, cfg) > 0) {
		wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
		ASSERT(csa != NULL);
		wlc_write_ext_csa_ie(&csa->csa, data->buf);
	}

	return BCME_OK;
}

static int
wlc_assoc_parse_psta_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bsscfg_t *cfg = data->cfg;
	struct scb *scb = ftpparm->assocreq.scb;
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

	if (data->ie == NULL) {
#if defined(BCMDBG) || defined(WLMSG_ASSOC)
		WL_ASSOC(("wl%d: %s attempted association with no primary PSTA information\n",
		          wlc->pub->unit, bcm_ether_ntoa(&ftpparm->assocreq.scb->ea, eabuf)));
#endif // endif
		return BCME_OK;
	}

	if (data->ie[TLV_LEN_OFF] == MEMBER_OF_BRCM_PROP_IE_LEN) {
		scb->psta_prim = wlc_scbfindband(wlc, cfg,
		  (struct ether_addr *)&data->ie[TLV_HDR_LEN + HT_PROP_IE_OVERHEAD],
		  CHSPEC_WLCBANDUNIT(wlc->cfg->current_bss->chanspec));
	}
	/* WAR to fix non-primary associating before primary interface assoc */
	if (scb->psta_prim == NULL) {
		ftpparm->assocreq.status = DOT11_SC_ASSOC_FAIL;
		return BCME_ERROR;
	}

	ftpparm->assocreq.status = DOT11_SC_SUCCESS;
	return BCME_OK;
}
#endif /* AP */

#if defined(STA) || defined(WDS)
bool
wlc_csa_parse_ie_ext(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg, wl_chan_switch_t *csa,
	uint8 *params, int len)
{
	wlc_info_t *wlc = cfg->wlc;
	bcm_tlv_t *tag = (bcm_tlv_t*)params;
	uint8 *end = params + len;
	uint8 extch = DOT11_EXT_CH_NONE;
	uint8 type;
	bool csa_ie_found = FALSE;

	ASSERT(csa != NULL);

	/* 11n & 11y csa takes precedence */
	if ((params < end) &&
	    (tag = bcm_parse_tlvs(params, len, DOT11_MNG_EXT_CSA_ID)) != NULL) {
		dot11_ext_csa_ie_t *csa_ie = (dot11_ext_csa_ie_t *)tag;
		if (tag->len >= DOT11_EXT_CSA_IE_LEN) {
			csa_ie_found = TRUE;
			csa->mode = csa_ie->b.mode;
			csa->count = csa_ie->b.count;
			extch = wlc_rclass_extch_get(wlc->cmi, csa_ie->b.reg);
#ifdef WL11ULB
			csa->chspec = wlc_ht_chanspec(wlc, csa_ie->b.channel, extch,
					cfg);
#else /* WL11ULB */
			csa->chspec = wlc_ht_chanspec(wlc, csa_ie->b.channel, extch);
#endif /* WL11ULB */
			csa->reg = csa_ie->b.reg;
		} else {
			WL_REGULATORY(("wl%d: %s: CSA IE length != 4\n",
				wlc->pub->unit, __FUNCTION__));
		}
	}

	if (!csa_ie_found && (params < end) &&
	    (tag = bcm_parse_tlvs(params, len, DOT11_MNG_CHANNEL_SWITCH_ID)) != NULL) {
		bool err = FALSE;
		dot11_chan_switch_ie_t *csa_ie = (dot11_chan_switch_ie_t *)tag;
		/* look for brcm extch IE first, if exist, use it,
		 * otherwise look for IEEE extch IE
		 */
		if (tag->len < DOT11_SWITCH_IE_LEN)
			err = TRUE;

		csa_ie_found = TRUE;
		type = BRCM_EXTCH_IE_TYPE;
		tag = (bcm_tlv_t *)bcm_find_vendor_ie((uchar *)tag, (uint)(end-(uint8 *)tag),
			BRCM_PROP_OUI, &type, sizeof(type));
		if (tag && tag->len == BRCM_EXTCH_IE_LEN)
			extch = ((dot11_brcm_extch_ie_t *)tag)->extch;
		else {
			tag = bcm_parse_tlvs(params, len, DOT11_MNG_EXT_CHANNEL_OFFSET);
			if (tag) {
				if (tag->len >= DOT11_EXTCH_IE_LEN) {
					extch = ((dot11_extch_ie_t *)tag)->extch;
					if (extch != DOT11_EXT_CH_LOWER &&
					    extch != DOT11_EXT_CH_UPPER &&
					    extch != DOT11_EXT_CH_NONE)
						extch = DOT11_EXT_CH_NONE;
				} else {
					WL_ERROR(("wl%d: wlc_parse_11h: extension channel offset"
						" len %d length too short\n",
						wlc->pub->unit, tag->len));
					csa_ie_found = FALSE;
					err = TRUE;
				}
			}
		}

		if (!err) {
			csa->mode = csa_ie->mode;
			csa->count = csa_ie->count;
#ifdef WL11ULB
			csa->chspec = wlc_ht_chanspec(wlc, csa_ie->channel, extch, cfg);
#else /* WL11ULB */
			csa->chspec = wlc_ht_chanspec(wlc, csa_ie->channel, extch);
#endif /* WL11ULB */
			csa->reg = 0;
		}
	}

#ifdef WL11AC
	/* Look for the Channel Switch Wrapper IE for additional CSA info */
	if ((params < end) &&
		(tag = bcm_parse_tlvs(params, len, DOT11_MNG_CHANNEL_SWITCH_WRAPPER_ID)) != NULL) {
		dot11_wide_bw_chan_switch_ie_t *wide_bw_ie;

		/* search for the Wide Bandwidth Channel Switch sub-element for
		 * wide channel width info
		 */
		wide_bw_ie = (dot11_wide_bw_chan_switch_ie_t *)
			bcm_parse_tlvs_min_bodylen(tag->data, tag->len,
					DOT11_MNG_WIDE_BW_CHANNEL_SWITCH_ID,
					DOT11_WIDE_BW_SWITCH_IE_LEN);
		if (wide_bw_ie == NULL) {
			WL_REGULATORY(("wl%d: %s: WIDE BANDWIDTH IE == NULL\n",
					wlc->pub->unit, __FUNCTION__));
		}

		/* determine the wide bw chanspec if the wide_bw_ie
		* was found (and valid len)
		*/
		if (csa_ie_found && wide_bw_ie != NULL) {
			csa->chspec = wlc_csa_get_wide_chanspec(
				(dot11_wide_bw_chan_switch_ie_t *)wide_bw_ie, csa->chspec, cfg);
		}
	}
#endif /* WL11AC */

	/* Should reset the csa.count back to zero if csa_ie not found */
	if (!csa_ie_found && csa->count) {
		WL_REGULATORY(("wl%d.%d: %s: no csa ie found, reset csa.count %d to 0\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, csa->count));
		csa->count = 0;
	}

	if (csa_ie_found)
		WL_REGULATORY(("%s: Found a CSA, count = %d\n", __FUNCTION__, csa->count));

	return (csa_ie_found);
}
#endif /* STA || WDS */

#ifdef STA
static bool
wlc_parse_csa_ie(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg, uint8 *params, int len)
{
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
	return (wlc_csa_parse_ie_ext(csam, cfg, &(csa->csa), params, len));
}

static int
wlc_csa_bcn_parse_ext_csa_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (wlc_parse_csa_ie(csam, cfg, data->buf, data->buf_len)) {
		wlc_csa_process_channel_switch(csam, cfg);
	}

	return BCME_OK;
}
#endif /* STA */

#ifdef AP
/* 802.11h Channel Switch Announcement */
static uint
wlc_csa_calc_csa_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (WL11H_ENAB(wlc) && BSSCFG_AP(cfg) &&
	    wlc_csa_get_csa_count(csam, cfg) > 0)
		return TLV_HDR_LEN + DOT11_SWITCH_IE_LEN;

	return 0;
}

static int
wlc_csa_write_csa_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_info_t *wlc = csam->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (WL11H_ENAB(wlc) && BSSCFG_AP(cfg) &&
	    wlc_csa_get_csa_count(csam, cfg) > 0) {
		wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
		ASSERT(csa != NULL);
		wlc_write_csa_ie(&csa->csa, data->buf, data->buf_len);
	}

	return BCME_OK;
}
#endif /* AP */

#ifdef STA
static int
wlc_csa_bcn_parse_csa_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_csa_info_t *csam = (wlc_csa_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;

	if (wlc_parse_csa_ie(csam, cfg, data->buf, data->buf_len)) {
		wlc_csa_process_channel_switch(csam, cfg);
	}

	return BCME_OK;
}
#endif /* STA */

void
wlc_recv_csa_action(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	wlc_info_t *wlc = csam->wlc;
	struct dot11_action_frmhdr *action_hdr;
	dot11_chan_switch_ie_t *csa_ie;
	bcm_tlv_t *ext_ie;
	bcm_tlv_t *ies;
	uint ies_len;
	uint8 extch = DOT11_EXT_CH_NONE;
#if (defined(BCMDBG) && defined(AP)) || defined(CLIENT_CSA)
#ifdef WL11AC
	bcm_tlv_t *wide_bw_ie = NULL;
	uint8 channel_width = VHT_OP_CHAN_WIDTH_20_40;
#endif /* WL11AC */
#endif /* BCMDBG && AP */

	ASSERT(cfg != NULL);

	action_hdr = (struct dot11_action_frmhdr *)body;

	ies = (bcm_tlv_t*)action_hdr->data;
	ies_len = body_len - DOT11_ACTION_HDR_LEN;

	csa_ie = (dot11_chan_switch_ie_t*)
		bcm_parse_tlvs_min_bodylen(ies, ies_len,
		DOT11_MNG_CHANNEL_SWITCH_ID,
		DOT11_SWITCH_IE_LEN);

	if (csa_ie == NULL) {
		WL_REGULATORY(("wl%d:%s: Bad CSA Spectrum Mngmt Action frame\n",
		               wlc->pub->unit, __FUNCTION__));
		WLCNTINCR(wlc->pub->_cnt->rxbadproto);
		wlc_send_action_err(wlc, hdr, body, body_len);
		return;
	}

	if (WL11H_ENAB(wlc) &&
		wlc_11h_get_spect_state(wlc->m11h, cfg) & NEED_TO_SWITCH_CHANNEL) {
		WL_REGULATORY(("wl%d:%s Already have a scheduled channel switch!\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	}

	/* check if we have an extension channel ie */
	if (N_ENAB(wlc->pub)) {
		/* Check for 11n spec IE first */
		ext_ie = bcm_parse_tlvs(ies, ies_len, DOT11_MNG_EXT_CHANNEL_OFFSET);
		if (ext_ie != NULL &&
		    ext_ie->len == DOT11_EXTCH_IE_LEN) {
			extch = ((dot11_extch_ie_t *)ext_ie)->extch;
		} else {
			uint8 extch_subtype = BRCM_EXTCH_IE_TYPE;

			/* Check for BRCM OUI format */
			ext_ie = bcm_find_vendor_ie(ies, ies_len, BRCM_PROP_OUI,
			                            &extch_subtype, 1);
			if (ext_ie != NULL &&
			    ext_ie->len == BRCM_EXTCH_IE_LEN) {
				extch = ((dot11_brcm_extch_ie_t *)ext_ie)->extch;
			}
		}
	}

#if (defined(BCMDBG) && defined(AP)) || defined(CLIENT_CSA)
#ifdef WL11AC
	/* check if we have an wide bandwidth channel switch ie */
	if (VHT_ENAB(wlc->pub)) {
		/* Check for 11ac spec IE */
		wide_bw_ie = bcm_parse_tlvs_min_bodylen(ies, ies_len,
			DOT11_MNG_WIDE_BW_CHANNEL_SWITCH_ID, DOT11_WIDE_BW_SWITCH_IE_LEN);
		if (wide_bw_ie != NULL) {
			channel_width = ((dot11_wide_bw_chan_switch_ie_t *)
			                 wide_bw_ie)->channel_width;
			WL_REGULATORY(("wl%d: wlc_recv_csa_action: mode %d, channel %d, count %d,"
			   "channel width %d\n", wlc->pub->unit, csa_ie->mode,
			   csa_ie->channel, csa_ie->count, channel_width));
		}
	}
#endif /* WL11AC */
#endif /* BCMDBG && AP */

	WL_REGULATORY(("wl%d: wlc_recv_csa_action: mode %d, channel %d, count %d, extension %d\n",
	               wlc->pub->unit, csa_ie->mode, csa_ie->channel, csa_ie->count, extch));
	BCM_REFERENCE(extch);

#if (defined(BCMDBG) && defined(AP)) || defined(CLIENT_CSA)
	if (BSSCFG_AP(cfg)) {
		chanspec_t chspec;
		if (extch == DOT11_EXT_CH_NONE) {
			chspec = CH20MHZ_CHSPEC(csa_ie->channel);
		} else {
			chspec = wf_channel2chspec(csa_ie->channel, WL_CHANSPEC_BW_40);
		}

#ifdef WL11AC
		if (channel_width != VHT_OP_CHAN_WIDTH_20_40 && wide_bw_ie != NULL) {
			chspec = wlc_csa_get_wide_chanspec(
					(dot11_wide_bw_chan_switch_ie_t *)wide_bw_ie, chspec, cfg);
		}
#endif /* WL11AC */
		wlc_csa_do_channel_switch(csam, cfg, chspec, csa_ie->mode, csa_ie->count, 0,
			CSA_BROADCAST_ACTION_FRAME);
	}
#endif /* BCMDBG && AP */

	return;
}

void
wlc_recv_ext_csa_action(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	wlc_info_t *wlc = csam->wlc;
	struct dot11_action_ext_csa *action_hdr;
	dot11_ext_csa_ie_t *req_ie;

	(void)wlc;

	ASSERT(cfg != NULL);

	action_hdr = (struct dot11_action_ext_csa *)body;
	req_ie = &action_hdr->chan_switch_ie;

	/* valid the IE in this action frame */
	if (N_ENAB(wlc->pub) &&
	    body_len >= (int)(sizeof(struct dot11_action_ext_csa))) {
		if (req_ie->id == DOT11_MNG_EXT_CSA_ID) {
			WL_REGULATORY(("wl%d: wlc_recv_ext_csa_action: mode %d, reg %d, channel %d,"
				"count %d\n", wlc->pub->unit, req_ie->b.mode,
				req_ie->b.reg, req_ie->b.channel, req_ie->b.count));
			return;
		}
	}
	WL_REGULATORY(("wl%d: wlc_recv_ext_csa_action: unknown ID %d", wlc->pub->unit, req_ie->id));
}

bool
wlc_csa_quiet_mode(wlc_csa_info_t *csam, uint8 *tag, uint tag_len)
{
	bool quiet = FALSE;
	dot11_chan_switch_ie_t *csa_ie;

	if (!tag || tag_len <= DOT11_BCN_PRB_LEN)
		return quiet;

	tag_len = tag_len - DOT11_BCN_PRB_LEN;
	tag = tag + DOT11_BCN_PRB_LEN;

	csa_ie = (dot11_chan_switch_ie_t *)
	        bcm_parse_tlvs(tag, tag_len, DOT11_MNG_CHANNEL_SWITCH_ID);
	if (csa_ie && csa_ie->mode)
		quiet = TRUE;

	return quiet;
}

void
wlc_csa_reset_all(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);

	ASSERT(csa != NULL);

	if (csa->csa_timer) {
		wl_del_timer(wlc->wl, csa->csa_timer);
		csa->channel_sw.pending = FALSE;
	}
	csa->csa.count = 0;
}

#ifdef AP
int
wlc_csa_do_channel_switch(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	chanspec_t chanspec, uint8 mode, uint8 count, uint8 reg_class, uint8 frame_type)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
	int bcmerror;
	char chanbuf[CHANSPEC_STR_LEN];
	BCM_REFERENCE(chanbuf);

	ASSERT(csa != NULL);

	if (!BSSCFG_AP(cfg))
		return BCME_NOTAP;

	if (!cfg->up) {
		return BCME_NOTREADY;
	}

	ASSERT(!wf_chspec_malformed(chanspec));

	if (!wlc_valid_chanspec_db(wlc->cmi, chanspec)) {
		return BCME_BADCHAN;
	}

	WL_REGULATORY(("wl%d.%d: %s: CSA request: mode %d, chanspec %s, count %d\n",
		wlc->pub->unit, cfg->_idx, __FUNCTION__, mode,
		wf_chspec_ntoa_ex(chanspec, chanbuf), count));

	/* reject if the target channel is same as the current channel */
	if (chanspec == WLC_BAND_PI_RADIO_CHANSPEC) {
		return BCME_BADCHAN;
	}

	/* when 11h is enabled, reject if the target channel is a radar channel but
	 * is invalid according to DFS state machine
	 */
	if (WL11H_ENAB(wlc) && wlc_radar_chanspec(wlc->cmi, chanspec) &&
			!wlc_valid_dfs_chanspec(wlc, chanspec)) {
		return BCME_BADCHAN;
	}
	wlc_csa_obss_dynbw_notif_cb_notif(csam, cfg,
		BCME_OK, CSA_CHANNEL_CHANGE_START, chanspec);
	csa->csa.mode = (mode != 0) ? 1 : 0;
	csa->csa.count = count;
	csa->csa.chspec = chanspec;
	if (reg_class != 0)
		csa->csa.reg = reg_class;
	else
		csa->csa.reg = wlc_get_regclass(wlc->cmi, chanspec);

	csa->csa.frame_type = (frame_type != CSA_UNICAST_ACTION_FRAME) ?
	  CSA_BROADCAST_ACTION_FRAME : CSA_UNICAST_ACTION_FRAME;

	/* and update beacon and probe response for the specified bsscfg */
	bcmerror = wlc_csa_apply_channel_switch(csam, cfg);

	/* adds NEED_TO_UPDATE_BCN to wlc->spect_state, send csa action frames, */
	if (!bcmerror) {
		wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_SWITCH_CHANNEL,
		                        NEED_TO_SWITCH_CHANNEL);
		if (mode == DOT11_CSA_MODE_NO_TX) {
			/* CSA Mode 0, Data transmission can take place till the time of
			 * actual channel switch. So, block fifo here only if Mode is 1
			 */
			wlc_block_datafifo(wlc, DATA_BLOCK_QUIET, DATA_BLOCK_QUIET);
		}
		BSSCFG_SET_CSA_IN_PROGRESS(cfg);
	}

	return BCME_OK;
}

/* This function applies the parameters of channel switch set else where */
/* It is assumed that the csa parameters have been set else where */
/* We send out the necessary csa action frames */
/* We mark wlc->spect_state with NEED_TO_UPDATE_BCN flag */
/* We only want the beacons and probe responses updated for the specified bss */
/* The actual channel switch will be initiated else where */
static int
wlc_csa_apply_channel_switch(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = csam->wlc;
	int bcmerror;

	if (!BSSCFG_AP(cfg))
		return BCME_NOTAP;

	if (!cfg->up) {
		return BCME_NOTREADY;
	}

	bcmerror = wlc_send_action_switch_channel(csam, cfg);
	if (bcmerror == BCME_OK) {
		wlc_11h_set_spect_state(wlc->m11h, cfg, NEED_TO_UPDATE_BCN, NEED_TO_UPDATE_BCN);
		wlc_bss_update_beacon(wlc, cfg);
		wlc_bss_update_probe_resp(wlc, cfg, TRUE);
	}

	return bcmerror;
}

void
wlc_csa_do_switch(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg, chanspec_t chspec)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);

	ASSERT(csa != NULL);

	/* Stop the current queue with flow control */
	wlc_txflowcontrol_override(wlc, cfg->wlcif->qi, ON, TXQ_STOP_FOR_PKT_DRAIN);

	csa->csa.mode = DOT11_CSA_MODE_ADVISORY;
	csa->csa.chspec = chspec;
	csa->csa.reg = wlc_get_regclass(wlc->cmi, chspec);
	csa->csa.count = cfg->current_bss->dtim_period + 1;

	wlc_csa_obss_dynbw_notif_cb_notif(csam, cfg,
			BCME_OK, CSA_CHANNEL_CHANGE_START, csa->csa.chspec);

	wlc_csa_apply_channel_switch(csam, cfg);
}
#endif /* AP */

#ifdef CLIENT_CSA
int wlc_send_unicast_action_switch_channel(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	const struct ether_addr *dst, wl_chan_switch_t *csa, uint8 action_id)
{
	int ret;
	ret = wlc_send_action_switch_channel_ex(csam, cfg, dst, csa, action_id);
	return ret;
}
#endif /* CLIENT_CSA */

static int
wlc_send_action_switch_channel_ex(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
	const struct ether_addr *dst, wl_chan_switch_t *csa, uint8 action_id)
{
	wlc_info_t *wlc = csam->wlc;
	void *p;
	uint8* pbody;
	uint8* cp;
	uint body_len;
	struct dot11_action_frmhdr *action_hdr;
	bool ext_ie = FALSE;
	uint8 *bufend;
	bool ret;
#ifdef WL11AC
	bool wide_bw_ie = FALSE;
#endif /* WL11AC */
	struct scb *scb = NULL;

	/* Action switch_channel */
	body_len = DOT11_ACTION_HDR_LEN + TLV_HDR_LEN;
	if (action_id == DOT11_SM_ACTION_CHANNEL_SWITCH) {
		body_len += DOT11_SWITCH_IE_LEN;
		/* account for extension channel IE if operate in 40MHz */
		if ((N_ENAB(wlc->pub) && (CHSPEC_IS40(csa->chspec))) ||
		    (VHT_ENAB(wlc->pub) && CHSPEC_BW_GE(csa->chspec, WL_CHANSPEC_BW_80))) {
			body_len += (TLV_HDR_LEN + DOT11_EXTCH_IE_LEN);
			ext_ie = TRUE;
		}
#ifdef WL11AC
		/* account for wide bandwidth channel switch IE if operate in 80MHz */
		if (VHT_ENAB(wlc->pub) && CHSPEC_BW_GE(csa->chspec, WL_CHANSPEC_BW_80)) {
			body_len += (TLV_HDR_LEN + DOT11_WIDE_BW_SWITCH_IE_LEN);
			wide_bw_ie = TRUE;
		}
#endif /* WL11AC */
	} else
		body_len += DOT11_EXT_CSA_IE_LEN;

	if ((p = wlc_frame_get_mgmt(wlc, FC_ACTION, dst, &cfg->cur_etheraddr,
	                            &cfg->BSSID, body_len, &pbody)) == NULL) {
		return BCME_NOMEM;
	}

	/* mark the end of buffer */
	bufend = pbody + body_len;

	action_hdr = (struct dot11_action_frmhdr *)pbody;
	action_hdr->category = DOT11_ACTION_CAT_SPECT_MNG;
	action_hdr->action = action_id;

	if (action_id == DOT11_SM_ACTION_CHANNEL_SWITCH) {
		cp = wlc_write_csa_ie(csa, action_hdr->data, (body_len - DOT11_ACTION_HDR_LEN));
		if (ext_ie)
			cp = wlc_write_extch_ie(csa->chspec, cp, BUFLEN(cp, bufend));
#ifdef WL11AC
		if (wide_bw_ie)
			cp = wlc_write_wide_bw_csa_ie(csa, cp, BUFLEN(cp, bufend), wlc);
#endif /* WL11AC */
	} else
		cp = wlc_write_ext_csa_ie(csa, action_hdr->data);

	ASSERT((cp - pbody) == (int)body_len);

	WL_COEX(("wl%d: %s: Send CSA (id=%d) Action frame\n",
		wlc->pub->unit, __FUNCTION__, action_id));

	if (csa->frame_type == CSA_UNICAST_ACTION_FRAME)
		wlc_pcb_fn_register(wlc->pcb, wlc_csa_unicast_tx_complete, (void *)p, p);

	/* For client csa, Dst is Unicast. If MFP is enabled, in that case extra
	 * 8 bytes needs to be populated with CCMP parameters before Action category.
	 * For this requirement scb is required to get correct key. Failing of this
	 * lead to not set FC_WEP in frame control info hence parser logic fails
	 * to read CSA at upstream AP
	 */
	if (!ETHER_ISBCAST(dst) && !ETHER_ISMULTI(dst)) {
		scb = wlc_scbfindband(wlc, cfg, dst,
			CHSPEC_WLCBANDUNIT(cfg->current_bss->chanspec));
	}
	ret = wlc_sendmgmt(wlc, p, cfg->wlcif->qi, scb);

	return (ret ? BCME_OK : BCME_ERROR);
}

static void
wlc_send_public_action_switch_channel(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg,
  const struct ether_addr *dst, wl_chan_switch_t *csa)
{
	wlc_info_t *wlc = csam->wlc;
	void *p;
	uint8* pbody;
	uint8* cp;
	uint body_len;
	struct dot11_action_frmhdr *action_hdr;
#ifdef WL11AC
	uint8 *bufend;
	bool wide_bw_ie = FALSE;
#endif /* WL11AC */

	/* Action switch_channel */
	body_len = sizeof(struct dot11y_action_ext_csa);
#ifdef WL11AC
	if (VHT_ENAB(wlc->pub) && (CHSPEC_BW_GE(csa->chspec, WL_CHANSPEC_BW_80))) {
		wide_bw_ie = TRUE;
		body_len += (TLV_HDR_LEN + DOT11_WIDE_BW_SWITCH_IE_LEN);
	}
#endif /* WL11AC */

	if ((p = wlc_frame_get_mgmt(wlc, FC_ACTION, dst, &cfg->cur_etheraddr,
	                            &cfg->BSSID, body_len, &pbody)) == NULL) {
		return;
	}

#ifdef WL11AC
	/* mark the end of buffer */
	bufend = pbody + body_len;
#endif /* WL11AC */
	action_hdr = (struct dot11_action_frmhdr *)pbody;
	action_hdr->category = DOT11_ACTION_CAT_PUBLIC;
	action_hdr->action = DOT11_PUB_ACTION_CHANNEL_SWITCH;

	cp = wlc_write_csa_body(csa, action_hdr->data);
#ifdef WL11AC
	if (wide_bw_ie)
		cp = wlc_write_wide_bw_csa_ie(csa, cp, BUFLEN(cp, bufend), wlc);
#endif /* WL11AC */

	ASSERT((cp - pbody) == (int)body_len);
	BCM_REFERENCE(cp);

	WL_COEX(("wl%d: %s: Send CSA Public Action frame\n", wlc->pub->unit, __FUNCTION__));

	if (csa->frame_type == CSA_UNICAST_ACTION_FRAME)
		wlc_pcb_fn_register(wlc->pcb, wlc_csa_unicast_tx_complete, (void *)p, p);

	wlc_sendmgmt(wlc, p, cfg->wlcif->qi, NULL);
}

int
wlc_send_action_switch_channel(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
	int bcmerror = BCME_OK;

	(void)wlc;

#ifdef PSPRETEND
	/* When sending the CSA, packets are going to be dropped somewhere in the process as
	 * the radio channel changes and the packets in transit are still set up for the
	 * original channel.
	 * PS pretend would try to save these packets but this is difficult to coordinate.
	 * For now, it is better to disable ps pretend in the short time window around CSA.
	 * It is useful to 'fake' a ps pretend event hitting the ps pretend retry limit,
	 * as the ps pretend will be reactivated when the channel conditions become
	 * normal again.
	 */
	if (PS_PRETEND_ENABLED(cfg)) {
		struct scb *scb;
		struct scb_iter scbiter;

		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {

			if (!SCB_ISMULTI(scb)) {
				/* set 'now' time for ps pretend so that we can delay it being
				 * re-enabled. Note that ps pretend may not actually have happened.
				 */
				scb->ps_pretend_start = R_REG(wlc->osh, &wlc->regs->tsf_timerlow);

				/* disable ps pretend (if not current engaged) */
				if (!SCB_PS_PRETEND(scb)) {
					/* invalidate txc */
					if (WLC_TXC_ENAB(wlc))
						wlc_txc_inv(wlc->txc, scb);

					/* also set the PS_PRETEND_RECENT flag. This is going to
					 * limit packet release prior to the CSA.
					 */
					scb->ps_pretend |= (PS_PRETEND_PREVENT | PS_PRETEND_RECENT);

					WL_PS(("wl%d.%d: %s: preventing ps pretend for "MACF"\n",
					        wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__,
					        ETHER_TO_MACF(scb->ea)));
				}
				/* move us to the retry limit for the scb, so packets will be no
				 * longer saved. If ps pretend was already triggered, this will
				 * add the PS_PRETEND_PREVENT flag as the retry limit is now hit.
				 */
				scb->ps_pretend_succ_count = cfg->ps_pretend_retry_limit;
			}
		}
	}
#endif /* PSPRETEND */

	ASSERT(csa != NULL);
	if (csa->csa.frame_type == CSA_UNICAST_ACTION_FRAME) {
		struct scb *scb;
		struct scb_iter scbiter;

		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
			wlc_csa_scb_cubby_t *csa_scb = CSA_SCB_CUBBY(csam, scb);
			ASSERT(csa_scb != NULL);

			if ((SCB_ASSOCIATED(scb) || SCB_LEGACY_WDS(scb)) && !SCB_ISMULTI(scb) &&
				(scb->psta_prim == NULL)) {
				bcmerror = wlc_send_action_switch_channel_ex(csam,
				                                             scb->bsscfg,
				                                             &scb->ea,
					&csa->csa, DOT11_SM_ACTION_CHANNEL_SWITCH);

				if (bcmerror < 0) {
					break;
				} else {
					if (N_ENAB(wlc->pub)) {
						wlc_send_public_action_switch_channel(csam,
						  scb->bsscfg, &scb->ea, &csa->csa);
						csa_scb->dcs_relocation_state =
							CSA_UNICAST_RELOCATION_PENDING;
					}
				}
			} else {
				csa_scb->dcs_relocation_state = CSA_UNICAST_RELOCATION_NONE;
			}
		}
	} else {
		wlc_send_action_switch_channel_ex(csam, cfg, &ether_bcast,
			&csa->csa, DOT11_SM_ACTION_CHANNEL_SWITCH);
		wlc_send_public_action_switch_channel(csam, cfg, &ether_bcast, &csa->csa);
	}

	return bcmerror;
}

#define CSA_PRE_SWITCH_TIME	10	/* pre-csa switch time, in unit of ms */

#ifdef WL_CS_RESTRICT_RELEASE
static void
wlc_csa_restrict_start(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = csam->wlc;
	bool start = (BSSCFG_AP(cfg) != 0);
	if (start) {
		wlc_scb_restrict_start(wlc, cfg);
	}
	wlc_ampdu_txeval_all(wlc);
}
#else
#define wlc_csa_restrict_start(csam, cfg)
#endif /* WL_CS_RESTRICT_RELEASE */

#ifdef PSPRETEND
static void
wlc_csa_pspretend_start(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	/* After datapath is enabled, enable psPretend */
	if (PS_PRETEND_ENABLED(cfg)) {
		struct scb *scb;
		struct scb_iter scbiter;

		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {

			if (!SCB_ISMULTI(scb)) {
				scb->ps_pretend_start = R_REG(wlc->osh,
					&wlc->regs->tsf_timerlow);

				/* Enable ps pretend (if not current engaged) */
				if (!SCB_PS_PRETEND(scb)) {
					/* invalidate txc */
					if (WLC_TXC_ENAB(wlc))
						wlc_txc_inv(wlc->txc, scb);

					scb->ps_pretend &= ~PS_PRETEND_PREVENT;

					WL_PS(("wl%d.%d: %s: "
						"Enabling ps pretend for "MACF"\n",
						wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
						__FUNCTION__,
						ETHER_TO_MACF(scb->ea)));
				 }
				 scb->ps_pretend_succ_count = 0;
			 }
		 }
	 }
}
#else
#define wlc_csa_pspretend_start(wlc, cfg)
#endif /* PSPRETEND */

void
wlc_csa_count_down(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);
	wlc_bss_info_t *current_bss = cfg->current_bss;
	chanspec_t tgt_chspec = csa->csa.chspec;

	ASSERT(csa != NULL);

	/* Resume tx */
	if (csa->csa.count == 0) {
		BSSCFG_CLR_CSA_IN_PROGRESS(cfg);
		if (DYN160_ACTIVE(wlc->pub) && (wlc->block_datafifo & DATA_BLOCK_TXCHAIN)) {
			wlc_block_datafifo(wlc, DATA_BLOCK_TXCHAIN, 0);
		}
	        if (wlc->block_datafifo & DATA_BLOCK_QUIET) {
			wlc_block_datafifo(wlc, DATA_BLOCK_QUIET, 0);
		}
		wlc_csa_restrict_start(csam, cfg);
		wlc_csa_pspretend_start(wlc, cfg);
		wlc_ampdu_cleanup_cs(wlc, cfg);
		return;
	}

	/* When DYN160 is active, for CSAs from 80Mhz to 160Mhz, data fifo should be blocked
	 * 2 BI before the channel is changed. This is done so that there is enough time
	 * to drain the pending packets
	 */
	if (csa->csa.mode == DOT11_CSA_MODE_ADVISORY) {
		if (csa->csa.count <= CSA_BLOCK_DATAFIFO_COUNT && DYN160_ACTIVE(wlc->pub) &&
			(CHSPEC_IS80(WLC_BAND_PI_RADIO_CHANSPEC) && CHSPEC_IS160(tgt_chspec))) {
			/* Block it in case of CSA to 160Mhz 2x2 <-> 80Mhz 4x4 */
			if (!(wlc->block_datafifo & DATA_BLOCK_TXCHAIN)) {
				wlc_block_datafifo(wlc, DATA_BLOCK_TXCHAIN, DATA_BLOCK_TXCHAIN);
			}
		}
	}

	/* to updated channel switch count of csa ie. */
	if (--csa->csa.count == 0) {
		/* set up time to switch channel after beacon is sent */
		wl_del_timer(wlc->wl, csa->csa_timer);
		wl_add_timer(wlc->wl, csa->csa_timer,
		             (current_bss->beacon_period < CSA_PRE_SWITCH_TIME ?
		              current_bss->beacon_period : CSA_PRE_SWITCH_TIME), 0);
		/* block data traffic but allow control */
		wlc_block_datafifo(wlc, DATA_BLOCK_QUIET, DATA_BLOCK_QUIET);
	}
}

void
wlc_csa_do_csa(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg, wl_chan_switch_t *cs, bool docs)
{
	wlc_info_t *wlc = csam->wlc;
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);

	ASSERT(csa != NULL);

	csa->csa = *cs;

	/* need to send legacy CSA and new 11n Ext-CSA if is n-enabled */
	wlc_send_action_switch_channel(csam, cfg);
	if (csa->csa.mode != 0) {
		/* CSA Mode 0, Data transmission can take place till the time of
		 * actual channel switch. So, block fifo here only if Mode is 1
		 */
		wlc_block_datafifo(wlc, DATA_BLOCK_QUIET, DATA_BLOCK_QUIET);
	}

	if (docs) {
		wlc_do_chanswitch(cfg, cs->chspec);
		wlc_csa_obss_dynbw_notif_cb_notif(csam, cfg,
			BCME_OK, CSA_CHANNEL_CHANGE_END, csa->csa.chspec);
	} else {
		/* dpc handles NEED_TO_UPDATE_BCN, NEED_TO_SWITCH_CHANNEL */
		wlc_11h_set_spect_state(wlc->m11h, cfg,
		                         NEED_TO_UPDATE_BCN | NEED_TO_SWITCH_CHANNEL,
		                         NEED_TO_UPDATE_BCN | NEED_TO_SWITCH_CHANNEL);

		BSSCFG_SET_CSA_IN_PROGRESS(cfg);
	}

	wlc_bss_update_beacon(wlc, cfg);
	wlc_bss_update_probe_resp(wlc, cfg, TRUE);
}

/* accessor functions */
uint8
wlc_csa_get_csa_count(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);

	ASSERT(csa != NULL);

	return csa->csa.count;
}

chanspec_t
wlc_csa_get_csa_chspec(wlc_csa_info_t *csam, wlc_bsscfg_t *cfg)
{
	wlc_csa_t *csa = CSA_BSSCFG_CUBBY(csam, cfg);

	ASSERT(csa != NULL);

	return csa->csa.chspec;
}

/* These functions register/unregister a callback that wlc_prot obss_notif_cb_notif may invoke. */
int
wlc_csa_obss_dynbw_notif_cb_register(wlc_csa_info_t *csam,
	wlc_csa_notif_cb_fn_t cb, void *arg)
{
	bcm_notif_h hdl = csam->csa_notif_hdl;
	return bcm_notif_add_interest(hdl, (bcm_notif_client_callback)cb, arg);
}

int
wlc_csa_obss_dynbw_notif_cb_unregister(wlc_csa_info_t *csam,
	wlc_csa_notif_cb_fn_t cb, void *arg)
{
	bcm_notif_h hdl = csam->csa_notif_hdl;
	return bcm_notif_remove_interest(hdl, (bcm_notif_client_callback)cb, arg);
}

static void
wlc_csa_obss_dynbw_notif_cb_notif(wlc_csa_info_t *csam,
	wlc_bsscfg_t *cfg, int status,
	int signal, chanspec_t new_chanspec)
{
	wlc_csa_notif_cb_data_t notif_data;
	bcm_notif_h hdl = csam->csa_notif_hdl;
	notif_data.cfg = cfg;
	notif_data.status = status;
	notif_data.signal = signal;
	notif_data.chanspec = new_chanspec;
	bcm_notif_signal(hdl, &notif_data);
	return;
}
#endif /* WLCSA */
