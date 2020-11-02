/**
 * Advanced IBSS implementation for Broadcom 802.11 Networking Driver
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
 * $Id: wlc_aibss.c 708017 2017-06-29 14:11:45Z $
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
#include <proto/vlan.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_key.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_bmac.h>
#include <wl_export.h>
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif // endif
#ifdef WLMCNX
#include <wlc_mcnx.h>
#endif // endif
#include <wlc_utils.h>
#include <wlc_scb.h>
#include <wlc_pcb.h>
#include <wlc_tbtt.h>
#include <wlc_apps.h>
#include <wlc_aibss.h>
#include <wlc_tx.h>
/* iovar table */
enum {
	IOV_AIBSS = 0,			/* enable/disable AIBSS feature */
	IOV_AIBSS_BCN_FORCE_CONFIG = 1,	/* bcn xmit configuration */
	IOV_AIBSS_TXFAIL_CONFIG = 2,
        IOV_AIBSS_IFADD = 3,		/* add AIBSS interface */
	IOV_AIBSS_IFDEL = 4,		/* delete AIBSS interface */
	IOV_AIBSS_PS = 5
	};

static const bcm_iovar_t aibss_iovars[] = {
	{"aibss", IOV_AIBSS, (IOVF_BSS_SET_DOWN), IOVT_BOOL, 0},
	{"aibss_bcn_force_config", IOV_AIBSS_BCN_FORCE_CONFIG,
	(IOVF_BSS_SET_DOWN), IOVT_BUFFER, sizeof(aibss_bcn_force_config_t)},
	{"aibss_txfail_config", IOV_AIBSS_TXFAIL_CONFIG,
	(0), IOVT_BUFFER, (OFFSETOF(aibss_txfail_config_t, max_atim_failure))},
	{"aibss_ifadd", IOV_AIBSS_IFADD, 0, IOVT_BUFFER, sizeof(wl_aibss_if_t)},
	{"aibss_ifdel", IOV_AIBSS_IFDEL, 0, IOVT_BUFFER, ETHER_ADDR_LEN},
	{"aibss_ps", IOV_AIBSS_PS, (IOVF_SET_DOWN), IOVT_BOOL, 0},
	{NULL, 0, 0, 0, 0}
};

/* AIBSS module specific state */
typedef struct wlc_aibss_info_priv {
	wlc_info_t *wlc;	/* pointer to main wlc structure */
	wlc_pub_t *pub;
	osl_t *osh;
	uint32	initial_min_bcn_dur;	/* duration to check if STA xmitted 1 bcn */
	uint32	bcn_flood_dur;
	uint32	min_bcn_dur;	/* duration to check if STA xmitted 1 bcn after bcn_flood time */
	struct wl_timer *ibss_timer;	/* per entry timer */
	uint32	ibss_start_time;	/* ticks when the IBSS started */
	uint32 last_txbcnfrm;		/* maintains the prev txbcnfrm count */
	int32 cfg_cubby_handle;		/* BSSCFG cubby offset */
	wlc_bsscfg_t *bsscfg;		/* dedicated bsscfg for IBSS */
} wlc_aibss_info_priv_t;

typedef struct  {
	wlc_aibss_info_t aibss_pub;
	wlc_aibss_info_priv_t aibss_priv;
} wlc_aibss_mem_t;

typedef struct aibss_cfg_info {
	uint32 bcn_timeout;		/* dur in seconds to receive 1 bcn */
	uint32 max_tx_retry;		/* no of consecutive no acks to send txfail event */
	bool pm_allowed;
	struct pktq	atimq;			/* ATIM queue */
	bool bcmc_pend;
	uint32 max_atim_failure;
} aibss_cfg_info_t;

#define ATIMQ_LEN MAXSCB				/* ATIM PKT Q length */

#define WLC_AIBSS_INFO_SIZE (sizeof(wlc_aibss_mem_t))
#define UCODE_IBSS_PS_SUPPORT_VER		(0x3ae0000)		/* BOM version 942.0 */

#define TX_FIFO_BITMAP(fifo)		(1<<(fifo))	/* TX FIFO number to bit map */

#define AIBSS_PMQ_INT_THRESH		(0x40) /* keep thres high to avoid PMQ interrupt */

static uint16 wlc_aibss_info_priv_offset = OFFSETOF(wlc_aibss_mem_t, aibss_priv);

/* module specific states location */
#define WLC_AIBSS_INFO_PRIV(aibss_info) \
	((wlc_aibss_info_priv_t *)((uint8*)(aibss_info) + wlc_aibss_info_priv_offset))

static int wlc_aibss_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
        void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
static void wlc_aibss_timer(void *arg);
static void wlc_aibss_bss_updn(void *ctx, bsscfg_up_down_event_data_t *evt);
static bool wlc_aibss_validate_chanspec(wlc_info_t *wlc, chanspec_t chanspec);
static void _wlc_aibss_tbtt_impl_cb(void *aibss_info, wlc_tbtt_ent_data_t *notif_data);
static void _wlc_aibss_tbtt_impl(wlc_aibss_info_t *aibss);
static void _wlc_aibss_pretbtt_cb(void *aibss_info, wlc_tbtt_ent_data_t *notif_data);
static void _wlc_aibss_ctrl_pkt_txstatus(wlc_info_t *wlc, void *pkt, uint txstatus);
static bool _wlc_aibss_sendatim(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb);
static void _wlc_aibss_send_atim_q(wlc_info_t *wlc, struct pktq *q);
static bool _wlc_aibss_check_pending_data(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bool force_all);
static void _wlc_aibss_set_active_scb(wlc_aibss_info_t *aibss_info, wlc_bsscfg_t *cfg);
static void wlc_aibss_pretbtt_query_cb(void *ctx, bss_pretbtt_query_data_t *notif_data);
static void _wlc_aibss_data_pkt_txstatus(wlc_info_t *wlc, void *pkt, uint txstatus);

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

wlc_aibss_info_t *
BCMATTACHFN(wlc_aibss_attach)(wlc_info_t *wlc)
{
	wlc_aibss_info_t *aibss_info;
	wlc_aibss_info_priv_t *aibss;

	/* sanity checks */
	if (!(aibss_info = (wlc_aibss_info_t *)MALLOCZ(wlc->osh, WLC_AIBSS_INFO_SIZE))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	aibss->wlc = wlc;
	aibss->pub = wlc->pub;
	aibss->osh = wlc->osh;

	/* create IBSS timer for xmit extra beacons */
	if ((aibss->ibss_timer =
	    wl_init_timer(wlc->wl, wlc_aibss_timer, aibss_info, "ibss_timer")) == NULL) {
		WL_ERROR(("wl%d: wl_init_timer for AIBSS failed\n", wlc->pub->unit));
		goto fail;
	}

	/* reserve cubby in the bsscfg container for private data */
	if ((aibss->cfg_cubby_handle = wlc_bsscfg_cubby_reserve(wlc, sizeof(aibss_cfg_info_t),
	                NULL, NULL, NULL, (void *)aibss_info)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* reserve cubby in the scb container to monitor per SCB tx stats */
	if ((aibss_info->scb_handle = wlc_scb_cubby_reserve(aibss->wlc,
		sizeof(aibss_scb_info_t), NULL, NULL, NULL, NULL)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_scb_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* bsscfg up/down callback */
	if (wlc_bsscfg_updown_register(wlc, wlc_aibss_bss_updn, aibss_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register pretbtt query callback */
	if (wlc_bss_pretbtt_query_register(wlc, wlc_aibss_pretbtt_query_cb, aibss) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bss_pretbtt_query_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register packet callback function */
	if (wlc_pcb_fn_set(wlc->pcb, 1, WLF2_PCB2_AIBSS_CTRL, _wlc_aibss_ctrl_pkt_txstatus)
		!= BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_pcb_fn_set() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register packet callback function */
	if (wlc_pcb_fn_set(wlc->pcb, 0, WLF2_PCB1_AIBSS_DATA, _wlc_aibss_data_pkt_txstatus)
		!= BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_pcb_fn_set() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, aibss_iovars, "aibss",
		aibss_info, wlc_aibss_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: AIBSS wlc_module_register() failed\n", wlc->pub->unit));
		goto fail;
	}

	return aibss_info;

fail:
	if (aibss->ibss_timer != NULL) {
		wl_free_timer(wlc->wl, aibss->ibss_timer);
		aibss->ibss_timer = NULL;
	}

	MFREE(wlc->osh, aibss_info, WLC_AIBSS_INFO_SIZE);

	return NULL;
}

void
BCMATTACHFN(wlc_aibss_detach)(wlc_aibss_info_t *aibss_info)
{
	wlc_aibss_info_priv_t *aibss;
	wlc_info_t	*wlc;

	if (!aibss_info)
		return;

	aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc = aibss->wlc;

	if (aibss->ibss_timer != NULL) {
		wl_free_timer(wlc->wl, aibss->ibss_timer);
		aibss->ibss_timer = NULL;
	}

	wlc_module_unregister(aibss->pub, "aibss", aibss_info);

	MFREE(aibss->wlc->osh, aibss, WLC_AIBSS_INFO_SIZE);
}

static int
wlc_aibss_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)hdl;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);

	int32 int_val = 0;
	bool bool_val;
	uint32 *ret_uint_ptr;
	int err = 0;
	wlc_bsscfg_t *bsscfg;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;

	ret_uint_ptr = (uint32 *)a;

	bsscfg = wlc_bsscfg_find_by_wlcif(aibss->wlc, wlcif);
	ASSERT(bsscfg != NULL);

	switch (actionid) {

		case IOV_GVAL(IOV_AIBSS):
			*ret_uint_ptr = AIBSS_ENAB(aibss->pub);
			break;

		case IOV_SVAL(IOV_AIBSS):
			aibss->pub->_aibss = bool_val;
			break;

		case IOV_GVAL(IOV_AIBSS_BCN_FORCE_CONFIG):{
			aibss_bcn_force_config_t *bcn_config = (aibss_bcn_force_config_t *)a;

			store16_ua(&bcn_config->version, AIBSS_BCN_FORCE_CONFIG_VER_0);
			store16_ua(&bcn_config->len, sizeof(aibss_bcn_force_config_t));
			store32_ua(&bcn_config->initial_min_bcn_dur, aibss->initial_min_bcn_dur);
			store32_ua(&bcn_config->bcn_flood_dur, aibss->bcn_flood_dur);
			store32_ua(&bcn_config->min_bcn_dur, aibss->min_bcn_dur);
			break;
		}

		case IOV_SVAL(IOV_AIBSS_BCN_FORCE_CONFIG):{
			aibss_bcn_force_config_t *bcn_config = (aibss_bcn_force_config_t *)p;

			aibss->initial_min_bcn_dur = load32_ua(&bcn_config->initial_min_bcn_dur);
			aibss->bcn_flood_dur = load32_ua(&bcn_config->bcn_flood_dur);
			aibss->min_bcn_dur = load32_ua(&bcn_config->min_bcn_dur);
			break;
		}

		case IOV_GVAL(IOV_AIBSS_TXFAIL_CONFIG): {
			aibss_txfail_config_t *txfail_config = (aibss_txfail_config_t *)a;
			aibss_cfg_info_t	*cfg_cubby;

			if (!BSSCFG_IBSS(bsscfg)) {
				err = BCME_ERROR;
				break;
			}

			cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);
			store16_ua((uint8 *)&txfail_config->version, AIBSS_TXFAIL_CONFIG_CUR_VER);
			store16_ua((uint8 *)&txfail_config->len, sizeof(aibss_txfail_config_t));
			store32_ua((uint8 *)&txfail_config->bcn_timeout, cfg_cubby->bcn_timeout);
			store32_ua((uint8 *)&txfail_config->max_tx_retry, cfg_cubby->max_tx_retry);
			if (alen >= sizeof(aibss_txfail_config_t)) {
				store32_ua((uint8 *)&txfail_config->max_atim_failure,
					cfg_cubby->max_atim_failure);
			}
			break;
		}

		case IOV_SVAL(IOV_AIBSS_TXFAIL_CONFIG): {
			aibss_txfail_config_t *txfail_config = (aibss_txfail_config_t *)a;
			aibss_cfg_info_t	*cfg_cubby;
			uint16 version = load16_ua(&txfail_config->version);
			uint16 len = load16_ua(&txfail_config->len);

			if (!BSSCFG_IBSS(bsscfg)) {
				err = BCME_ERROR;
				break;
			}

			if (version > AIBSS_TXFAIL_CONFIG_CUR_VER) {
				err = BCME_VERSION;
				break;
			}

			cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);
			cfg_cubby->bcn_timeout = load32_ua((uint8 *)&txfail_config->bcn_timeout);
			cfg_cubby->max_tx_retry = load32_ua((uint8 *)&txfail_config->max_tx_retry);
			if (version == AIBSS_TXFAIL_CONFIG_VER_1 &&
				len == sizeof(aibss_txfail_config_t)) {
				cfg_cubby->max_atim_failure =
					load32_ua((uint8 *)&txfail_config->max_atim_failure);
			}
			break;
		}
		case IOV_SVAL(IOV_AIBSS_IFADD): {
			wl_aibss_if_t *aibss_if = (wl_aibss_if_t*)a;
			wlc_info_t *wlc = aibss->wlc;
			wlc_bsscfg_t *new_bsscfg;
			int idx;
			uint32 flags = WLC_BSSCFG_HW_BCN | WLC_BSSCFG_HW_PRB |
				WLC_BSSCFG_TX_SUPR_ENAB;

			/* return failure if
			 *	1) primary interface is running is AP/IBSS mode
			 *	2) a dedicated IBSS interface already exists
			 *	3) TODO: any other AP/GO interface exists
			 */
			if (!wlc->cfg->BSS || BSSCFG_AP(wlc->cfg) || aibss->bsscfg != NULL) {
				err = BCME_BADARG;
				break;
			}

			/* bsscfg with the given MAC address exists */
			if (wlc_bsscfg_find_by_hwaddr(wlc, &aibss_if->addr) != NULL) {
				err = BCME_BADARG;
				break;
			}

			/* validate channel spec */
			if (!wlc_aibss_validate_chanspec(wlc, aibss_if->chspec)) {
				err = BCME_BADARG;
				break;
			}

			/* try to allocate one bsscfg for IBSS */
			if ((idx = wlc_bsscfg_get_free_idx(wlc)) == -1) {
				WL_ERROR(("wl%d: no free bsscfg\n", wlc->pub->unit));
				err = BCME_ERROR;
				break;
			}
			if ((new_bsscfg = wlc_bsscfg_alloc(wlc, idx, flags,
				&aibss_if->addr, FALSE)) == NULL) {
				WL_ERROR(("wl%d: cannot create bsscfg\n", wlc->pub->unit));
				err = BCME_ERROR;
				break;
			}
			if ((err = wlc_bsscfg_init(wlc, new_bsscfg)) != BCME_OK) {
				WL_ERROR(("wl%d: failed to init bsscfg\n", wlc->pub->unit));
				wlc_bsscfg_free(wlc, new_bsscfg);
				break;
			}
			break;
		}
		case IOV_SVAL(IOV_AIBSS_IFDEL): {
			wlc_info_t *wlc = aibss->wlc;
			wlc_bsscfg_t *del_bsscfg = wlc_bsscfg_find_by_hwaddr(wlc, a);

			/* bsscfg not found */
			if (del_bsscfg == NULL || aibss->bsscfg == NULL) {
				err = BCME_BADARG;
				break;
			}

			/* can't delete current IOVAR processing bsscfg */
			if (del_bsscfg == bsscfg) {
				err = BCME_BADARG;
				break;
			}

			if (del_bsscfg->enable)
				wlc_bsscfg_disable(wlc, del_bsscfg);
			wlc_bsscfg_free(wlc, del_bsscfg);
			aibss->bsscfg = NULL;
			break;
		}

		case IOV_GVAL(IOV_AIBSS_PS):
			*ret_uint_ptr = (int32)aibss->wlc->pub->_aibss_ps;
			break;

		case IOV_SVAL(IOV_AIBSS_PS):
			aibss->wlc->pub->_aibss_ps = bool_val;
			break;

		default:
			err = BCME_UNSUPPORTED;
			break;
	}
	return err;
}

/* bsscfg up/down */
static void
wlc_aibss_bss_updn(void *ctx, bsscfg_up_down_event_data_t *evt)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)ctx;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	wlc_bsscfg_t *cfg;
	aibss_cfg_info_t *cfg_cubby = NULL;

	ASSERT(evt != NULL);

	cfg = evt->bsscfg;
	ASSERT(cfg != NULL);

	if (!AIBSS_ENAB(wlc->pub) || !BSSCFG_IBSS(cfg))
		return;

	if (evt->up) {
		uint32 val = 0;
		uint32 mask = (MCTL_INFRA | MCTL_DISCARD_PMQ);
#ifdef WLMCNX
		int bss = wlc_mcnx_BSS_idx(wlc->mcnx, cfg);
#endif /* WLMCNX */

		if (wlc->pub->_aibss_ps && wlc->ucode_rev < UCODE_IBSS_PS_SUPPORT_VER) {
			WL_ERROR(("wl%d: %s: AIBSS PS feature not supported\n",
				aibss->wlc->pub->unit, __FUNCTION__));
			return;
		}

		if (wlc_tbtt_ent_fn_add(wlc->tbtt, cfg, _wlc_aibss_pretbtt_cb,
			_wlc_aibss_tbtt_impl_cb, wlc->aibss_info) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_tbtt_ent_fn_add() failed\n",
			  wlc->pub->unit, __FUNCTION__));
			return;
		}

		aibss->bsscfg = cfg;

		/* Set MCTL_INFRA for non-primary AIBSS interface to support concurrent mode */
		if (cfg != wlc->cfg) {
			val |= MCTL_INFRA;
		}

		/* DisabLe PMQ entries to avoid uCode updating pmq entry on  rx packets */
		val |= MCTL_DISCARD_PMQ;

		/* Update PMQ Interrupt threshold to avoid MI_PMQ interrupt */
		wlc_write_shm(wlc, SHM_PMQ_INTR_THRESH, AIBSS_PMQ_INT_THRESH);

		wlc_ap_ctrl(wlc, TRUE, cfg, -1);
		wlc_mctrl(wlc, mask, val);

		/* Set IBSS and GO bit to enable beaconning */
#ifdef WLMCNX
		wlc_mcnx_mac_suspend(wlc->mcnx);
		wlc_mcnx_write_shm(wlc->mcnx, M_P2P_BSS_ST(bss),
			(M_P2P_BSS_ST_GO | M_P2P_BSS_ST_IBSS));
		wlc_mcnx_mac_resume(wlc->mcnx);
#endif /* WLMCNX */

		cfg_cubby = BSSCFG_CUBBY(cfg, aibss->cfg_cubby_handle);

		pktq_init(&cfg_cubby->atimq, 1, ATIMQ_LEN);

		/* if configured, start timer to check for bcn xmit */
		if (aibss->initial_min_bcn_dur && aibss->bcn_flood_dur &&
			aibss->min_bcn_dur) {

			/* IBSS interface up, start the timer to monitor the
			 * beacon transmission
			 */
			aibss->ibss_start_time = OSL_SYSUPTIME();
			wl_add_timer(wlc->wl, aibss->ibss_timer, aibss->initial_min_bcn_dur,
				FALSE);

			/* Disable PM till the flood beacon is complete */
			wlc_set_pmoverride(cfg, TRUE);
		}
	}
	else {
		aibss->bsscfg = NULL;

		/* IBSS not enabled, stop monitoring the link */
		if (aibss->ibss_start_time) {
			aibss->ibss_start_time = 0;
			wl_del_timer(wlc->wl, aibss->ibss_timer);
		}
	}
}

void
wlc_aibss_timer(void *arg)
{
	wlc_aibss_info_t *aibss_info = (wlc_aibss_info_t *)arg;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	uint32	timer_dur = aibss->min_bcn_dur;
	wl_cnt_t *cnt = wlc->pub->_cnt;
	wlc_bsscfg_t *cfg = aibss->bsscfg;

	WL_TRACE(("wl%d: wlc_ibss_timer", wlc->pub->unit));

	if (!wlc->pub->up || !AIBSS_ENAB(wlc->pub)) {
		return;
	}

	if (aibss->ibss_start_time &&
		((OSL_SYSUPTIME() - aibss->ibss_start_time) < aibss->bcn_flood_dur)) {
		timer_dur = aibss->initial_min_bcn_dur;
	}
	else if (cfg->pm->PM_override) {
		wlc_set_pmoverride(cfg, FALSE);
	}

	/* Get the number of beacons sent */
	wlc_statsupd(wlc);

	/* If no beacons sent from prev timer, send one */
	if ((cnt->txbcnfrm - aibss->last_txbcnfrm) == 0)
	{
		/* Not sent enough bcn, send bcn in next TBTT
		 * even if we recv bcn from a peer IBSS STA
		 */
		wlc_bmac_mhf(wlc->hw, MHF1, MHF1_FORCE_SEND_BCN,
			MHF1_FORCE_SEND_BCN, WLC_BAND_ALL);
	}
	else {
		wlc_bmac_mhf(wlc->hw, MHF1, MHF1_FORCE_SEND_BCN, 0, WLC_BAND_ALL);

		/* update the beacon counter */
		aibss->last_txbcnfrm = cnt->txbcnfrm;
	}

	/* ADD TIMER */
	wl_add_timer(wlc->wl, aibss->ibss_timer, timer_dur, FALSE);
}

void
wlc_aibss_check_txfail(wlc_aibss_info_t *aibss_info, wlc_bsscfg_t *cfg, struct scb *scb)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	aibss_cfg_info_t	*cfg_cubby = BSSCFG_CUBBY(cfg, aibss->cfg_cubby_handle);
	aibss_scb_info_t	*scb_info = SCB_CUBBY((scb), aibss_info->scb_handle);

	if (cfg_cubby->bcn_timeout &&
		(scb_info->no_bcn_counter >= cfg_cubby->bcn_timeout)) {
		wlc_bss_mac_event(wlc, cfg, WLC_E_AIBSS_TXFAIL, &scb->ea,
		                  WLC_E_STATUS_FAIL, AIBSS_BCN_FAILURE,
		                  0, NULL, 0);

		/* Reset the counters */
		scb_info->no_bcn_counter = 0;
	}
}

static bool wlc_aibss_validate_chanspec(wlc_info_t *wlc, chanspec_t chanspec)
{
	/* use default chanspec */
	if (chanspec == 0)
		return TRUE;

	/* validate chanspec */
	if (wf_chspec_malformed(chanspec) || !wlc_valid_chanspec_db(wlc->cmi, chanspec) ||
		wlc_radar_chanspec(wlc->cmi, chanspec))
		return FALSE;

	/* If mchan not enabled, don't allow IBSS on different channel */
	if (!MCHAN_ENAB(wlc->pub) && wlc->pub->associated && chanspec != wlc->home_chanspec) {
		return FALSE;
	}

	return TRUE;
}

void wlc_aibss_tbtt(wlc_aibss_info_t *aibss_info)
{
	_wlc_aibss_tbtt_impl(aibss_info);
}

bool wlc_aibss_sendpmnotif(wlc_aibss_info_t *aibss_info, wlc_bsscfg_t *bsscfg,
	ratespec_t rate_override, int prio, bool track)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	aibss_cfg_info_t	*cfg_cubby;

	if (bsscfg == NULL || bsscfg != aibss->bsscfg)
		return FALSE;

	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

	/* Let wlc_aibss_atim_window_end take care of putting the device to sleep */
	cfg_cubby->pm_allowed = FALSE;

	/* Clear PM states since IBSS PS is not depend on it */
	bsscfg->pm->PMpending = FALSE;
	wlc_pm_pending_complete(wlc);

	return TRUE;
}

void _wlc_aibss_ctrl_pkt_txstatus(wlc_info_t *wlc, void *pkt, uint txstatus)
{
	struct scb *scb;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(wlc->aibss_info);
	wlc_bsscfg_t *bsscfg = aibss->bsscfg;
	aibss_cfg_info_t	*cfg_cubby;
	aibss_scb_info_t *scb_info;

	scb = WLPKTTAGSCBGET(pkt);
	if (scb == NULL)
		return;
	bsscfg = SCB_BSSCFG(scb);

	scb_info = SCB_CUBBY((scb), wlc->aibss_info->scb_handle);
	if (bsscfg == NULL || bsscfg != aibss->bsscfg || SCB_ISMULTI(scb))
		return;

	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

	if (txstatus & TX_STATUS_ACK_RCV) {
		WL_PS(("%s():wl%d: ATIM frame %p sent\n",
			__FUNCTION__, wlc->pub->unit, pkt));

		scb_info->atim_acked = TRUE;
		scb_info->atim_failure_count = 0;

		/* Clear PS state if we receive an ACK for atim frame */
		if (SCB_PS(scb)) {
			wlc_apps_process_ps_switch(wlc, &scb->ea, PS_SWITCH_OFF);
		}
	}
	else {
		WL_PS(("%s():wl%d: ATIM frame %p sent (no ACK)\n",
			__FUNCTION__, wlc->pub->unit, pkt));
		scb_info->atim_failure_count++;

		if (cfg_cubby->max_atim_failure &&
			scb_info->atim_failure_count >= cfg_cubby->max_atim_failure) {
			wlc_bss_mac_event(wlc, bsscfg, WLC_E_AIBSS_TXFAIL, &scb->ea,
			                  WLC_E_STATUS_FAIL, AIBSS_ATIM_FAILURE,
			                  0, NULL, 0);
			scb_info->atim_failure_count = 0;
		}

		/* XXX minimize PS state change, let atim_window_end
		 * take care of enabling PS
		 */
	}

	if (!pktq_empty(&cfg_cubby->atimq)) {
		_wlc_aibss_send_atim_q(wlc, &cfg_cubby->atimq);
	}
}

void _wlc_aibss_pretbtt_cb(void *aibss_info, wlc_tbtt_ent_data_t *notif_data)
{
	_wlc_aibss_tbtt_impl(aibss_info);
}

void _wlc_aibss_tbtt_impl_cb(void *aibss_info, wlc_tbtt_ent_data_t *notif_data)
{
	/* TBTT CLBK */
}

void _wlc_aibss_tbtt_impl(wlc_aibss_info_t *aibss_info)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);
	wlc_info_t *wlc = aibss->wlc;
	wlc_bsscfg_t *bsscfg = aibss->bsscfg;
	aibss_cfg_info_t	*cfg_cubby;
	uint16 mhf_val;
	uint32 pat_hi, pat_lo;
	struct ether_addr eaddr;
	char eabuf[ETHER_ADDR_STR_LEN];
	volatile uint16 *pmqctrlstatus = (volatile uint16 *)&wlc->regs->pmqreg.w.pmqctrlstatus;
	volatile uint32 *pmqhostdata = (volatile uint32 *)&wlc->regs->pmqreg.pmqhostdata;

	BCM_REFERENCE(eabuf);
	BCM_REFERENCE(eaddr);

	if (bsscfg == NULL) {
		return;
	}

	/* read entries until empty or pmq exeeding limit */
	while ((R_REG(wlc->osh, pmqhostdata)) & PMQH_NOT_EMPTY) {
		pat_lo = R_REG(wlc->osh, &wlc->regs->pmqpatl);
		pat_hi = R_REG(wlc->osh, &wlc->regs->pmqpath);
		eaddr.octet[5] = (pat_hi >> 8)  & 0xff;
		eaddr.octet[4] =  pat_hi	& 0xff;
		eaddr.octet[3] = (pat_lo >> 24) & 0xff;
		eaddr.octet[2] = (pat_lo >> 16) & 0xff;
		eaddr.octet[1] = (pat_lo >> 8)  & 0xff;
		eaddr.octet[0] =  pat_lo	& 0xff;

		WL_PS(("wl%d.%d: ATIM failed, pmq entry added for %s\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(bsscfg), bcm_ether_ntoa(&eaddr, eabuf)));
	}

	/* Clear all the PMQ entry before sending ATIM frames */
	W_REG(wlc->osh, pmqctrlstatus, PMQH_DEL_MULT);

	/* Check for BCN Flood status */
	mhf_val =  wlc_bmac_mhf_get(wlc->hw, MHF1, WLC_BAND_AUTO);
	if (mhf_val & MHF1_FORCE_SEND_BCN) {
		wl_cnt_t *cnt = wlc->pub->_cnt;

		wlc_statsupd(wlc);

		/* check if we have sent atleast 1 bcn and clear
		 * the force bcn bit
		 */
		if (cnt->txbcnfrm != aibss->last_txbcnfrm) {
			wlc_bmac_mhf(wlc->hw, MHF1, MHF1_FORCE_SEND_BCN, 0, WLC_BAND_ALL);
		}
	}

	if (AIBSS_BSS_PS_ENAB(bsscfg)) {
		cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

		cfg_cubby->pm_allowed = FALSE;
		wlc_set_ps_ctrl(bsscfg);

		/* BCMC traffic */
		if (TXPKTPENDGET(wlc, TX_BCMC_FIFO)) {
			/* XXX uCodes sends the BCMC packet in the atim window
			 * itself - need to debug
			 */
			cfg_cubby->bcmc_pend = TRUE;
			_wlc_aibss_sendatim(wlc, bsscfg, WLC_BCMCSCB_GET(wlc, bsscfg));
		}

		/* Check pending packets for peers and sendatim to them */
		_wlc_aibss_set_active_scb(aibss_info, bsscfg);
		_wlc_aibss_check_pending_data(wlc, bsscfg, FALSE);
	}

	return;
}

void
wlc_aibss_ps_start(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	struct scb *bcmc_scb;

	WL_PS(("%s: Entering\n", __FUNCTION__));

	/* Enable PS for BCMC scb */
	bcmc_scb = WLC_BCMCSCB_GET(wlc, cfg);
	ASSERT(bcmc_scb);
	bcmc_scb->PS = TRUE;

	/* In case of AIBSS PS on primary interface host might enable
	 * PM mode well before the IBSS PS is enabled. PMEnabled will not
	 * be set untill IBSS PS is set so update correct pm state.
	 */
	wlc_set_pmstate(cfg, (cfg->pm->PM != PM_OFF));

	/* PS for other SCB shall be enabled after first ATIM window */
}

void
wlc_aibss_ps_stop(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	struct scb_iter scbiter;
	struct scb *scb, *bcmc_scb;

	WL_PS(("%s: Entering\n", __FUNCTION__));

	/* Assume all sta in PS and pull them out one by one */
	if (!BSSCFG_IBSS(cfg)) {
		return;
	}

	bcmc_scb = WLC_BCMCSCB_GET(wlc, cfg);
	ASSERT(bcmc_scb);
	bcmc_scb->PS = FALSE;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
		if (SCB_PS(scb)) {
			wlc_apps_process_ps_switch(wlc, &scb->ea, PS_SWITCH_OFF);
		}
	}
}

void wlc_aibss_atim_window_end(wlc_info_t *wlc)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(wlc->aibss_info);
	wlc_bsscfg_t *bsscfg = aibss->bsscfg;
	struct scb *scb;
	struct scb_iter scbiter;
	bool be_awake = FALSE;
	aibss_cfg_info_t *cfg_cubby;

	if (aibss->bsscfg == NULL && !AIBSS_BSS_PS_ENAB(aibss->bsscfg)) {
		WL_ERROR(("wl%d: %s: ATIM enabled in non AIBSS mode\n",
			wlc->pub->unit, __FUNCTION__));
		return;
	}

	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		aibss_scb_info_t *scb_info = SCB_CUBBY((scb), wlc->aibss_info->scb_handle);
		uint8 scb_ps_on = PS_SWITCH_PMQ_ENTRY;

		if ((scb_info->atim_acked) ||
			scb_info->atim_rcvd) {
			scb_ps_on = PS_SWITCH_OFF;
		}

		if (SCB_PS(scb) != scb_ps_on) {
			wlc_apps_process_ps_switch(wlc, &scb->ea, scb_ps_on);
		}

		scb_info->atim_rcvd = FALSE;
		scb_info->atim_acked = FALSE;

		if (!SCB_PS(scb)) {
			be_awake = TRUE;
		}
	}

	/* Stay awake for the bcn period if we have bcmc traffic */
	if (cfg_cubby->bcmc_pend) {
		be_awake = TRUE;
	}

	/* Set only if we need to sleep or continue to stay awake */
	/* Set it in pretbtt */
	if (!be_awake) {
		uint16 st;
#ifdef WLMCNX
		int bss = wlc_mcnx_BSS_idx(wlc->mcnx, bsscfg);
#endif /* WLMCNX */

		cfg_cubby->pm_allowed = TRUE;
		wlc_set_ps_ctrl(bsscfg);

#ifdef WLMCNX
		/* clear the wake bit */
		st = wlc_mcnx_read_shm(wlc->mcnx, M_P2P_BSS_ST(bss));
		st &= ~M_P2P_BSS_ST_WAKE;
		wlc_mcnx_write_shm(wlc->mcnx, M_P2P_BSS_ST(bss), st);
#endif /* WLMCNX */
	}

	if (TXPKTPENDGET(wlc, TX_ATIM_FIFO)) {
		WL_ERROR(("%s ATIM failed, flush ATIM fifo [atim pending %d]\r\n",
			__FUNCTION__, TXPKTPENDGET(wlc, TX_ATIM_FIFO)));
		wlc_bmac_tx_fifo_sync(wlc->hw, TX_FIFO_BITMAP(TX_ATIM_FIFO), FLUSHFIFO);
	}
	/* clear bcmc pending for this window */
	cfg_cubby->bcmc_pend = FALSE;
}

void
_wlc_aibss_send_atim_q(wlc_info_t *wlc, struct pktq *q)
{
	void *p;
	int prec = 0;
	struct scb *scb;

	/* XXX do we need this ?
	if (wlc->in_send_q) {
		WL_INFORM(("wl%d: in_send_q, qi=%p\n", wlc->pub->unit, qi));
		return;
	}

	wlc->in_send_q = TRUE;
	*/

	/* Send all the enq'd pkts that we can.
	 * Dequeue packets with precedence with empty HW fifo only
	 */
	 while ((p = pktq_deq(q, &prec))) {
		wlc_txh_info_t txh_info;
		wlc_pkttag_t *pkttag = WLPKTTAG(p);

		scb = WLPKTTAGSCBGET(p);
		if ((pkttag->flags & WLF_TXHDR) == 0) {
			wlc_mgmt_ctl_d11hdrs(wlc, p, scb, TX_ATIM_FIFO, 0);
		}

		wlc_get_txh_info(wlc, p, &txh_info);

		if (!(uint)TXAVAIL(wlc, TX_ATIM_FIFO)) {
			/* Mark precedences related to this FIFO, unsendable */
			WLC_TX_FIFO_CLEAR(wlc, TX_ATIM_FIFO);
			break;
		}
#ifndef NEW_TXQ
		wlc_txfifo(wlc, TX_ATIM_FIFO, p, &txh_info, TRUE, 1);
#else
#endif /* !NEQ_TXQ */
	}
	/* wlc->in_send_q = FALSE; */
}

void
_wlc_aibss_set_active_scb(wlc_aibss_info_t *aibss_info, wlc_bsscfg_t *cfg)
{
	int prec = 0;
	struct pktq *txq = WLC_GET_TXQ(cfg->wlcif->qi);
	void *head_pkt, *pkt;
	struct scb *scb;

	PKTQ_PREC_ITER(txq, prec) {
		head_pkt = NULL;
		while (pktq_ppeek(txq, prec) != head_pkt) {
			aibss_scb_info_t	*scb_info;
			pkt = pktq_pdeq(txq, prec);
			if (!head_pkt) {
				head_pkt = pkt;
			}

			scb = WLPKTTAGSCBGET(pkt);
			scb_info = SCB_CUBBY((scb), aibss_info->scb_handle);
			scb_info->pkt_pend = TRUE;
			pktq_penq(txq, prec, pkt);
		}
	}
}

void
wlc_aibss_ps_process_atim(wlc_info_t *wlc, struct ether_addr *ea)
{
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(wlc->aibss_info);
	struct scb *scb = wlc_scbfind(wlc, aibss->bsscfg, ea);
	aibss_scb_info_t *scb_info;
	char buf[ETHER_ADDR_STR_LEN];

	BCM_REFERENCE(buf);

	if (scb == NULL) {
		WL_ERROR(("wl%d.%d: ATIM received from unknown peer %s\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(aibss->bsscfg), bcm_ether_ntoa(ea, buf)));
		return;
	}

	WL_PS(("wl%d.%d: ATIM received\n", wlc->pub->unit, WLC_BSSCFG_IDX(aibss->bsscfg)));

	scb_info = SCB_CUBBY((scb), wlc->aibss_info->scb_handle);
	scb_info->atim_rcvd = TRUE;

	if (SCB_PS(scb)) {
		wlc_apps_process_ps_switch(wlc, &scb->ea, PS_SWITCH_OFF);
	}
	wlc_set_ps_ctrl(aibss->bsscfg);
}

bool
_wlc_aibss_sendatim(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb)
{
	void *p;
	uint8 *pbody;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(wlc->aibss_info);
	aibss_cfg_info_t *cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);
#ifdef BCMDBG
		char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	WL_PS(("%s: Entering\n", __FUNCTION__));

	ASSERT(wlc->pub->up);

	/* HANDLE BCMC ATIM too */
	if (pktq_full(&cfg_cubby->atimq)) {
		WL_ERROR(("wl%d.%d: ATIM queue overflow \n", wlc->pub->unit,
			WLC_BSSCFG_IDX(bsscfg)));
		return FALSE;
	}

	if ((p = wlc_frame_get_mgmt(wlc, FC_ATIM, &scb->ea, &bsscfg->cur_etheraddr,
		&bsscfg->current_bss->BSSID, 0, &pbody)) == NULL) {
		WL_ERROR(("wl%d.%d: Unable to get pkt for ATIM frame\n", wlc->pub->unit,
			WLC_BSSCFG_IDX(bsscfg)));
		return FALSE;
	}

	WLPKTTAG(p)->flags |= WLF_PSDONTQ;
	WLF2_PCB2_REG(p, WLF2_PCB2_AIBSS_CTRL);
	WLPKTTAG(p)->shared.packetid = 0;

	WLPKTTAGSCBSET(p, scb);
	WLPKTTAGBSSCFGSET(p, WLC_BSSCFG_IDX(bsscfg));
	PKTSETLEN(wlc->osh, p, DOT11_MGMT_HDR_LEN);

	pktq_penq(&cfg_cubby->atimq, 0, p);
	_wlc_aibss_send_atim_q(wlc, &cfg_cubby->atimq);
	return TRUE;
}

bool
_wlc_aibss_check_pending_data(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bool force_all)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
		aibss_scb_info_t *scb_info = SCB_CUBBY((scb), wlc->aibss_info->scb_handle);
		uint psq_len = wlc_apps_psq_len(wlc, scb);

		/* Send ATIM for the peer which are already in PS */
		if ((SCB_PS(scb) && psq_len) || scb_info->pkt_pend) {
			scb_info->pkt_pend = FALSE;
			_wlc_aibss_sendatim(wlc, bsscfg, scb);
		}
	}

	return TRUE;
}

bool
wlc_aibss_pm_allowed(wlc_aibss_info_t *aibss_info, wlc_bsscfg_t *bsscfg)
{
	aibss_cfg_info_t	*cfg_cubby;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(aibss_info);

	if (!BSSCFG_IBSS(bsscfg))
		return TRUE;

	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);
	return cfg_cubby->pm_allowed;
}

/* return the required pretbtt value */
static void
wlc_aibss_pretbtt_query_cb(void *ctx, bss_pretbtt_query_data_t *notif_data)
{
	wlc_bsscfg_t *cfg;

	ASSERT(notif_data != NULL);

	cfg = notif_data->cfg;
	ASSERT(cfg != NULL);

	if (!BSSCFG_IS_AIBSS_PS_ENAB(cfg)) {
		return;
	}

	/* XXX increase the pre-tbtt by 4 ms inorder for radio to be ready to xmit bcn
	 * after wake-up from sleep, this need to be optimized
	 */
	notif_data->pretbtt += 4000;
}

static void
_wlc_aibss_data_pkt_txstatus(wlc_info_t *wlc, void *pkt, uint txstatus)
{
	struct scb *scb;
	wlc_aibss_info_priv_t *aibss = WLC_AIBSS_INFO_PRIV(wlc->aibss_info);
	wlc_bsscfg_t *bsscfg = aibss->bsscfg;
	aibss_scb_info_t *scb_info;
	aibss_cfg_info_t *cfg_cubby;

	scb = WLPKTTAGSCBGET(pkt);
	if (scb == NULL)
		return;
	bsscfg = SCB_BSSCFG(scb);

	scb_info = SCB_CUBBY((scb), wlc->aibss_info->scb_handle);
	if (bsscfg == NULL || bsscfg != aibss->bsscfg || SCB_ISMULTI(scb))
		return;

	cfg_cubby = BSSCFG_CUBBY(bsscfg, aibss->cfg_cubby_handle);

	/* Ignore TXSTATUS from PKTFREE */
	if (txstatus == 0)
		return;

	if (txstatus & TX_STATUS_ACK_RCV) {
		scb_info->tx_noack_count = 0;
	}
	else {
		scb_info->tx_noack_count++;

		if (cfg_cubby->max_tx_retry &&
			(scb_info->tx_noack_count >= cfg_cubby->max_tx_retry)) {
			wlc_bss_mac_event(wlc, bsscfg, WLC_E_AIBSS_TXFAIL, &scb->ea,
			                  WLC_E_STATUS_FAIL, AIBSS_TX_FAILURE,
			                  0, NULL, 0);

			scb_info->tx_noack_count = 0;
		}
	}
	return;
}
