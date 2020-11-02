/*
 * Separate alloc/free module for wlc_xxx.c files. Decouples
 * the code that does alloc/free from other code so data
 * structure changes don't affect ROMMED code as much.
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
 * $Id: wlc_alloc.c 708271 2017-06-30 11:14:45Z $
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
#include <proto/802.11.h>
#include <proto/802.11e.h>
#ifdef	BCMCCX
#include <proto/802.11_ccx.h>
#endif	/* BCMCCX */
#include <proto/wpa.h>
#include <proto/vlan.h>
#include <wlioctl.h>
#if defined(BCMSUP_PSK) || defined(BCMCCX)
#include <proto/eapol.h>
#endif /* defined(BCMSUP_PSK) || defined(BCMCCX) */
#include <bcmwpa.h>
#ifdef BCMCCX
#include <bcmcrypto/ccx.h>
#endif /* BCMCCX */
#include <bcmdevs.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_alloc.h>
#include <wlc_keymgmt.h>
#include <wlc_objregistry.h>
#if defined(WLRSDB)
#include <wlc_rsdb.h>
#endif // endif

static wlc_pub_t *wlc_pub_malloc(wlc_info_t * wlc, osl_t *osh, uint unit, uint devid);
static void wlc_pub_mfree(wlc_info_t * wlc, osl_t *osh, wlc_pub_t *pub);

static void wlc_tunables_init(wlc_tunables_t *tunables, uint devid);

static bool wlc_attach_malloc_high(wlc_info_t *wlc, osl_t *osh, uint unit, uint *err, uint devid);
static bool wlc_attach_malloc_misc(wlc_info_t *wlc, osl_t *osh, uint unit, uint *err, uint devid);
static void wlc_detach_mfree_high(wlc_info_t *wlc, osl_t *osh);
static void wlc_detach_mfree_misc(wlc_info_t *wlc, osl_t *osh);
#ifdef WLC_HIGH
static int wlc_get_wlcindex(wlc_info_t *wlc);
#endif // endif

#ifndef NTXD_USB_4319
#define NTXD_USB_4319 0
#define NRPCTXBUFPOST_USB_4319 0
#endif // endif
#ifndef DNGL_MEM_RESTRICT_RXDMA /* used only for BMAC low driver */
#define DNGL_MEM_RESTRICT_RXDMA 0
#endif // endif
#ifdef WLC_HIGH
static int
wlc_get_wlcindex(wlc_info_t *wlc)
{
	wlc_cmn_info_t* wlc_cmn;
	int idx;
	wlc_cmn = wlc->cmn;
	for ((idx) = 0; (int) (idx) < MAX_RSDB_MAC_NUM; (idx)++) {
		if (wlc == wlc_cmn->wlc[(idx)])
			return idx;
	}
	return -1;
}
#endif // endif
void
BCMATTACHFN(wlc_tunables_init)(wlc_tunables_t *tunables, uint devid)
{
	/* tx/rx ring size for DMAs with 512 descriptor ring size max */
	tunables->ntxd = NTXD;
	tunables->nrxd = NRXD;

	/* tx/rx ring size for DMAs with 4096 descriptor ring size max */
	tunables->ntxd_large = NTXD_LARGE;
	tunables->nrxd_large = NRXD_LARGE;

#if defined(TXQ_MUX)
	/* low TxQ high/low watermarks */
	tunables->txq_highwater = WLC_TXQ_HIGHWATER;
	tunables->txq_lowwater = WLC_TXQ_LOWWATER;
#endif /* TXQ_MUX */

	tunables->rxbufsz = RXBUFSZ;
	tunables->nrxbufpost = NRXBUFPOST;
	tunables->maxscb = MAXSCB;
	tunables->ampdunummpdu2streams = AMPDU_NUM_MPDU;
	tunables->ampdunummpdu3streams = AMPDU_NUM_MPDU_3STREAMS;
	tunables->maxpktcb = MAXPKTCB;
	tunables->maxucodebss = WLC_MAX_UCODE_BSS;
	tunables->maxucodebss4 = WLC_MAX_UCODE_BSS4;
	tunables->maxbss = MAXBSS;
	tunables->maxubss = MAXUSCANBSS;
	tunables->datahiwat = WLC_DATAHIWAT;
	tunables->ampdudatahiwat = WLC_AMPDUDATAHIWAT;
	tunables->rxbnd = RXBND;
	tunables->txsbnd = TXSBND;
	tunables->pktcbnd = PKTCBND;
	tunables->txmr = TXMR;
	tunables->txpft = TXPREFTHRESH;
	tunables->txpfc = TXPREFCTL;
	tunables->txblen = TXBURSTLEN;
	tunables->rxpft = RXPREFTHRESH;
	tunables->rxpfc = RXPREFCTL;
	tunables->rxblen = RXBURSTLEN;
	tunables->mrrs = MRRS;
#ifdef WLC_HIGH_ONLY
	tunables->rpctxbufpost = NRPCTXBUFPOST;
#ifdef USBAP
	if (devid == BCM4319_CHIP_ID) {
		tunables->ntxd = NTXD_USB_4319;
		tunables->rpctxbufpost = NRPCTXBUFPOST_USB_4319;
	}
	else if (BCM4350_CHIP(devid)) {
		tunables->rpctxbufpost = NRPCTXBUFPOST_USB_43556;
	}
	else if (devid == BCM43236_CHIP_ID) {
		tunables->ntxd = NTXD_USB_43236;
		tunables->nrxd = NRXD_USB_43236;
		tunables->rpctxbufpost = NRPCTXBUFPOST_USB_43236;
		tunables->nrxbufpost = NRXBUFPOST_USB_43236;
	}
	else if (devid == BCM43526_CHIP_ID) {
		tunables->ntxd = NTXD_USB_43526;
		tunables->nrxd = NRXD_USB_43526;
		tunables->rpctxbufpost = NRPCTXBUFPOST_USB_43526;
		tunables->nrxbufpost = NRXBUFPOST_USB_43526;
	}
	else if (devid == BCM43556_CHIP_ID) {
		tunables->ntxd = NTXD_USB_43556;
		tunables->nrxd = NRXD_USB_43556;
		tunables->rpctxbufpost = NRPCTXBUFPOST_USB_43556;
		tunables->nrxbufpost = NRXBUFPOST_USB_43556;
	}
#endif /* USBAP */
#endif /* WLC_HIGH_ONLY */
#if defined(WLC_LOW_ONLY)
	tunables->dngl_mem_restrict_rxdma = DNGL_MEM_RESTRICT_RXDMA;
#endif // endif
	tunables->maxtdls = WLC_MAXTDLS;
#ifdef DONGLEBUILD
	tunables->pkt_maxsegs = 1;
#else /* DONGLEBUILD */
	tunables->pkt_maxsegs = MAX_DMA_SEGS;
#endif /* DONGLEBUILD */
	tunables->maxscbcubbies = MAXSCBCUBBIES;
	tunables->maxbsscfgcubbies = MAXBSSCFGCUBBIES;

	tunables->max_notif_clients = MAX_NOTIF_CLIENTS;
	tunables->max_notif_servers = MAX_NOTIF_SERVERS;
	tunables->max_mempools = MAX_MEMPOOLS;
#ifdef BCMPCIEDEV
	tunables->amsdu_resize_buflen = 0;
#else
	tunables->amsdu_resize_buflen = PKTBUFSZ/4;
#endif // endif
	tunables->ampdu_pktq_size = AMPDU_PKTQ_LEN;
	tunables->ampdu_pktq_fav_size = AMPDU_PKTQ_FAVORED_LEN;
	tunables->maxpcbcds = MAXPCBCDS;

#ifdef PROP_TXSTATUS
	tunables->wlfcfifocreditac0 = WLFCFIFOCREDITAC0;
	tunables->wlfcfifocreditac1 = WLFCFIFOCREDITAC1;
	tunables->wlfcfifocreditac2 = WLFCFIFOCREDITAC2;
	tunables->wlfcfifocreditac3 = WLFCFIFOCREDITAC3;
	tunables->wlfcfifocreditbcmc = WLFCFIFOCREDITBCMC;
	tunables->wlfcfifocreditother = WLFCFIFOCREDITOTHER;
	tunables->wlfc_fifo_cr_pending_thresh_ac_bk = WLFC_FIFO_CR_PENDING_THRESH_AC_BK;
	tunables->wlfc_fifo_cr_pending_thresh_ac_be = WLFC_FIFO_CR_PENDING_THRESH_AC_BE;
	tunables->wlfc_fifo_cr_pending_thresh_ac_vi = WLFC_FIFO_CR_PENDING_THRESH_AC_VI;
	tunables->wlfc_fifo_cr_pending_thresh_ac_vo = WLFC_FIFO_CR_PENDING_THRESH_AC_VO;
	tunables->wlfc_fifo_cr_pending_thresh_bcmc = WLFC_FIFO_CR_PENDING_THRESH_BCMC;
	tunables->wlfc_trigger = WLFC_INDICATION_TRIGGER;
	tunables->wlfc_fifo_bo_cr_ratio = WLFC_FIFO_BO_CR_RATIO;
	tunables->wlfc_comp_txstatus_thresh = WLFC_COMP_TXSTATUS_THRESH;
#endif /* PROP_TXSTATUS */

	/* set 4360 specific tunables */
	if (IS_AC2_DEV(devid) ||
		IS_DEV_AC3X3(devid) || IS_DEV_AC2X2(devid)) {
		tunables->ntxd = NTXD_AC3X3;
		tunables->nrxd = NRXD_AC3X3;
		tunables->rxbnd = RXBND_AC3X3;
		tunables->nrxbufpost = NRXBUFPOST_AC3X3;
		tunables->pktcbnd = PKTCBND_AC3X3;

		/* tx/rx ring size for DMAs with 4096 descriptor ring size max */
		tunables->ntxd_large = NTXD_LARGE_AC3X3;
		tunables->nrxd_large = NRXD_LARGE_AC3X3;
	}
	if (IS_AC2_DEV(devid)) {
		tunables->txmr = TXMR_AC2;
		tunables->txpft = TXPREFTHRESH_AC2;
		tunables->txpfc = TXPREFCTL_AC2;
		tunables->txblen = TXBURSTLEN_AC2;
		tunables->txblen = TXBURSTLEN_AC2;
		tunables->rxpft = RXPREFTHRESH_AC2;
		tunables->rxpfc = RXPREFCTL_AC2;
		tunables->rxblen = RXBURSTLEN_AC2;
		tunables->mrrs = MRRS_AC2;
	}

	/* Key management */
	tunables->max_keys = WLC_KEYMGMT_MAX_KEYS;

#ifdef MFP
	tunables->max_keys += WLC_KEYMGMT_NUM_BSS_IGTK;
#endif /* MFP */

	/* IE mgmt */
	tunables->max_ie_build_cbs = MAXIEBUILDCBS;
	tunables->max_vs_ie_build_cbs = MAXVSIEBUILDCBS;
	tunables->max_ie_parse_cbs = MAXIEPARSECBS;
	tunables->max_vs_ie_parse_cbs = MAXVSIEPARSECBS;
	tunables->max_ie_regs = MAXIEREGS;

	tunables->num_rxivs = WLC_KEY_NUM_RX_SEQ;

	tunables->maxbestn = BESTN_MAX;
	tunables->maxmscan = MSCAN_MAX;
	tunables->nrxbufpost_fifo1 = NRXBUFPOST_FIFO1;
	tunables->ntxd_lfrag =	NTXD_LFRAG;
	tunables->nrxd_fifo1 =	NRXD_FIFO1;
	tunables->maxroamthresh = MAX_ROAM_TIME_THRESH;
	tunables->copycount = COPY_CNT_BYTES;
	tunables->nrxd_classified_fifo =	NRXD_CLASSIFIED_FIFO;
	tunables->bufpost_classified_fifo = NRXBUFPOST_CLASSIFIED_FIFO;

	tunables->scan_settle_time = WLC_DEFAULT_SETTLE_TIME;
	tunables->min_scballoc_mem = MIN_SCBALLOC_MEM;
}

static wlc_pub_t *
BCMATTACHFN(wlc_pub_malloc)(wlc_info_t * wlc, osl_t *osh, uint unit, uint devid)
{
	wlc_pub_t *pub;

	if ((pub = (wlc_pub_t*) MALLOCZ(osh, sizeof(wlc_pub_t))) == NULL) {
		goto fail;
	}

	pub->cmn = (wlc_pub_cmn_t*) obj_registry_get(wlc->objr, OBJR_WLC_PUB_CMN_INFO);
	if (pub->cmn == NULL) {
		if ((pub->cmn = (wlc_pub_cmn_t*) MALLOCZ(osh, sizeof(wlc_pub_cmn_t))) == NULL) {
			goto fail;
		}
		 obj_registry_set(wlc->objr, OBJR_WLC_PUB_CMN_INFO, pub->cmn);
	}
	(void)obj_registry_ref(wlc->objr, OBJR_WLC_PUB_CMN_INFO);

	if ((pub->tunables = (wlc_tunables_t *)
	     MALLOCZ(osh, sizeof(wlc_tunables_t))) == NULL) {
		goto fail;
	}

	/* need to init the tunables now */
	wlc_tunables_init(pub->tunables, devid);

#ifdef WLCNT
	if ((pub->_cnt = (wl_cnt_wlc_t *)MALLOCZ(osh, sizeof(wl_cnt_wlc_t))) == NULL) {
		goto fail;
	}

	if ((pub->_mcst_cnt = MALLOCZ(osh, WL_CNT_MCST_STRUCT_SZ)) == NULL) {
		goto fail;
	}

#if defined(WL_PSMX)
	if ((pub->_mcxst_cnt = MALLOCZ(osh, WL_CNT_MCXST_STRUCT_SZ)) == NULL) {
		goto fail;
	}
#endif // endif

	if ((pub->_wme_cnt = (wl_wme_cnt_t*)
		MALLOCZ(osh, sizeof(wl_wme_cnt_t))) == NULL) {
		goto fail;
	}
#endif /* WLCNT */

	return pub;

fail:
	wlc_pub_mfree(wlc, osh, pub);
	return NULL;
}

static void
BCMATTACHFN(wlc_pub_mfree)(wlc_info_t * wlc, osl_t *osh, wlc_pub_t *pub)
{
	if (pub == NULL)
		return;

	if (pub->tunables) {
		MFREE(osh, pub->tunables, sizeof(wlc_tunables_t));
		pub->tunables = NULL;
	}

#ifdef WLCNT
	if (pub->_cnt) {
		MFREE(osh, pub->_cnt, sizeof(wl_cnt_wlc_t));
		pub->_cnt = NULL;
	}

	if (pub->_mcst_cnt) {
		MFREE(osh, pub->_mcst_cnt, WL_CNT_MCST_STRUCT_SZ);
		pub->_mcst_cnt = NULL;
	}

#if defined(WL_PSMX)
	if (pub->_mcxst_cnt) {
		MFREE(osh, pub->_mcxst_cnt, WL_CNT_MCXST_STRUCT_SZ);
		pub->_mcxst_cnt = NULL;
	}
#endif // endif
	if (pub->_wme_cnt) {
		MFREE(osh, pub->_wme_cnt, sizeof(wl_wme_cnt_t));
		pub->_wme_cnt = NULL;
	}
#endif /* WLCNT */

	if (pub->cmn && obj_registry_unref(wlc->objr, OBJR_WLC_PUB_CMN_INFO) == 0) {
		obj_registry_set(wlc->objr, OBJR_WLC_PUB_CMN_INFO, NULL);
		MFREE(osh, pub->cmn, sizeof(wlc_pub_cmn_t));
	}

	MFREE(osh, pub, sizeof(wlc_pub_t));
}

#ifdef WLCHANIM
static void
BCMATTACHFN(wlc_chanim_mfree)(osl_t *osh, chanim_info_t *c_info)
{
	wlc_chanim_stats_t *headptr = c_info->stats;
	wlc_chanim_stats_t *curptr;
#ifdef WLCHANIM_US
	wlc_chanim_stats_us_t *head_us = c_info->stats_us;
	wlc_chanim_stats_us_t *cur_us;
#endif // endif
	while (headptr) {
		curptr = headptr;
		headptr = headptr->next;
		MFREE(osh, curptr, sizeof(wlc_chanim_stats_t));
	}

#ifdef WLCHANIM_US
	while (head_us) {
		cur_us = head_us;
		head_us = head_us->next;
		MFREE(osh, cur_us, sizeof(*cur_us));
	}
#endif // endif

	c_info->stats = NULL;
#ifdef WLCHANIM_US
	c_info->stats_us = NULL;
#endif // endif
	MFREE(osh, c_info, sizeof(chanim_info_t));
}
#endif /* WLCHANIM */

static bool
BCMATTACHFN(wlc_attach_malloc_high)(wlc_info_t *wlc, osl_t *osh, uint unit, uint *err, uint devid)
{
#ifdef WLC_HIGH
	int i;

	/* OBJECT REGISTRY: check if shared key has value already stored */
	wlc->cmn = (wlc_cmn_info_t*) obj_registry_get(wlc->objr, OBJR_WLC_CMN_INFO);

	if (wlc->cmn == NULL) {
		if ((wlc->cmn =  (wlc_cmn_info_t*) MALLOCZ(osh,
			sizeof(wlc_cmn_info_t))) == NULL) {
			*err = 1035;
			goto fail;
		}
		/* OBJECT REGISTRY: We are the first instance, store value for key */
		obj_registry_set(wlc->objr, OBJR_WLC_CMN_INFO, wlc->cmn);
	}
	{
		int ref_count;

		/* OBJECT REGISTRY: Reference the stored value in both instances */
		ref_count = obj_registry_ref(wlc->objr, OBJR_WLC_CMN_INFO);
		ASSERT(ref_count <= MAX_RSDB_MAC_NUM);
		wlc->cmn->wlc[ref_count - 1] = wlc;

		/* rsdb_mode must be initialized before wlc_stf_phy_chain_calc is called */
#if defined(WLRSDB)
#if !defined(WLRSDB_DISABLED)
		wlc->cmn->rsdb_mode = WLC_RSDB_MODE_RSDB;
#elif defined(RSDB_MODE_80P80)
		wlc->cmn->rsdb_mode = WLC_RSDB_MODE_80P80;
#else
		wlc->cmn->rsdb_mode = WLC_RSDB_MODE_2X2;
#endif // endif
#endif /* defined(WLRSDB) */
#if defined(WLTEST)
		if (wlc->cmn->lifetime_mg == NULL) {
			if ((wlc->cmn->lifetime_mg = (wl_lifetime_mg_t*) MALLOCZ(osh,
				sizeof(wl_lifetime_mg_t))) == NULL) {
				*err = 1050;
				goto fail;
			}
		}
#endif // endif
	}
	wlc->as_info = (wlc_assoc_info_t*)obj_registry_get(wlc->objr, OBJR_ASSOCIATION_INFO);

	if (wlc->as_info == NULL) {
		if ((wlc->as_info =
			(wlc_assoc_info_t*)MALLOCZ(osh, sizeof(wlc_assoc_info_t))) == NULL) {
			*err = 1050;
			goto fail;
		}
#ifdef STA
		if ((wlc->as_info->join_targets = (wlc_bss_list_t*)
			 MALLOCZ(osh, sizeof(wlc_bss_list_t))) == NULL) {
			*err = 1019;
			goto fail;
		}
#endif /* STA */
		obj_registry_set(wlc->objr, OBJR_ASSOCIATION_INFO, wlc->as_info);
	}
	(void)obj_registry_ref(wlc->objr, OBJR_ASSOCIATION_INFO);

	wlc->bandstate[0] = (wlcband_t*) obj_registry_get(wlc->objr, OBJR_WLC_BANDSTATE);
	if (wlc->bandstate[0] == NULL) {
		if ((wlc->bandstate[0] = (wlcband_t*)
			MALLOCZ(osh, (sizeof(wlcband_t) * MAXBANDS))) == NULL) {
			*err = 1010;
			goto fail;
		}
		obj_registry_set(wlc->objr, OBJR_WLC_BANDSTATE, wlc->bandstate[0]);
	}
	(void)(obj_registry_ref(wlc->objr, OBJR_WLC_BANDSTATE));

	for (i = 1; i < MAXBANDS; i++) {
		wlc->bandstate[i] =
		        (wlcband_t *)((uintptr)wlc->bandstate[0] + (sizeof(wlcband_t) * i));
	}

	if ((wlc->bandinst = (wlcband_inst_t **)
		MALLOCZ(osh, sizeof(wlcband_inst_t *) * MAXBANDS)) == NULL) {
		*err = 1016;
		goto fail;
	}

	for (i = 0; i < MAXBANDS; i++) {
		if ((wlc->bandinst[i] = (wlcband_inst_t *)
			MALLOCZ(osh, sizeof(wlcband_inst_t))) == NULL) {
			*err = 1018;
			goto fail;
		}
	}

	/* OBJECT REGISTRY: check if shared key has value already stored */
	wlc->modulecb = (modulecb_t*) obj_registry_get(wlc->objr, OBJR_MODULE_ID);
	if (wlc->modulecb == NULL) {
		if ((wlc->modulecb = (modulecb_t*) MALLOCZ(osh,
			sizeof(modulecb_t) * wlc->pub->max_modules)) == NULL) {
			*err = 1012;
			goto fail;
		}
		/* OBJECT REGISTRY: We are the first instance, store value for key */
		obj_registry_set(wlc->objr, OBJR_MODULE_ID, wlc->modulecb);
	}
	/* OBJECT REGISTRY: Reference the stored value in both instances */
	(void)obj_registry_ref(wlc->objr, OBJR_MODULE_ID);

	if ((wlc->modulecb_data = (modulecb_data_t*)
	     MALLOCZ(osh, sizeof(modulecb_data_t) * wlc->pub->max_modules)) == NULL) {
		*err = 1013;
		goto fail;
	}

	wlc->bsscfg = (wlc_bsscfg_t**) obj_registry_get(wlc->objr, OBJR_BSSCFG_PTR);
	if (wlc->bsscfg == NULL) {
		if ((wlc->bsscfg = (wlc_bsscfg_t**) MALLOCZ(osh,
			sizeof(wlc_bsscfg_t*) * WLC_MAXBSSCFG)) == NULL) {
			*err = 1016;
			goto fail;
		}
		obj_registry_set(wlc->objr, OBJR_BSSCFG_PTR, wlc->bsscfg);
	}
	(void)obj_registry_ref(wlc->objr, OBJR_BSSCFG_PTR);
#ifndef WLRSDB_DVT
	wlc->default_bss = obj_registry_get(wlc->objr, OBJR_DEFAULT_BSS);
#endif // endif

	if (wlc->default_bss == NULL) {
		if ((wlc->default_bss = (wlc_bss_info_t*)
		     MALLOCZ(osh, sizeof(wlc_bss_info_t))) == NULL) {
			*err = 1010;
			goto fail;
		}
		obj_registry_set(wlc->objr, OBJR_DEFAULT_BSS, wlc->default_bss);
	}
#ifndef WLRSDB_DVT
	(void)obj_registry_ref(wlc->objr, OBJR_DEFAULT_BSS);
#endif // endif
	if ((wlc->stf = (wlc_stf_t*)
	     MALLOCZ(osh, sizeof(wlc_stf_t))) == NULL) {
		*err = 1017;
		goto fail;
	}

	if ((wlc->corestate->macstat_snapshot = (uint16*)
	     MALLOCZ(osh, sizeof(uint16) * MACSTAT_OFFSET_SZ)) == NULL) {
		*err = 1027;
		goto fail;
	}

#if defined(WL_PSMX)
	if ((wlc->corestate->macxstat_snapshot = (uint16*)
	     MALLOCZ(osh, sizeof(uint16) * MACXSTAT_OFFSET_SZ)) == NULL) {
		*err = 1034;
		goto fail;
	}
#endif // endif

#ifndef WLC_NET80211
	if ((wlc->scan_results = (wlc_bss_list_t*)
	     MALLOCZ(osh, sizeof(wlc_bss_list_t))) == NULL) {
		*err = 1007;
		goto fail;
	}

	if ((wlc->custom_scan_results = (wlc_bss_list_t*)
	     MALLOCZ(osh, sizeof(wlc_bss_list_t))) == NULL) {
		*err = 1008;
		goto fail;
	}

	if ((wlc->txmod_fns = (txmod_info_t*)
	     MALLOCZ(osh, sizeof(txmod_info_t) * TXMOD_LAST)) == NULL) {
		*err = 1014;
		goto fail;
	}

#if defined(DELTASTATS)
	if ((wlc->delta_stats = (delta_stats_info_t*)
	     MALLOCZ(osh, sizeof(delta_stats_info_t))) == NULL) {
		*err = 1023;
		goto fail;
	}
#endif /* DELTASTATS */

#ifdef WLCHANIM
	if ((wlc->chanim_info = (chanim_info_t*)
	     MALLOCZ(osh, sizeof(chanim_info_t))) == NULL) {
		*err = 1031;
		goto fail;
	}
#endif // endif

#endif /* !WLC_NET80211 */

#if defined(PKTC) || defined(PKTC_DONGLE)
	if ((wlc->pktc_info = (wlc_pktc_info_t*)
	     MALLOCZ(osh, sizeof(wlc_pktc_info_t))) == NULL) {
		*err = 1032;
		goto fail;
	}
#endif // endif

#if defined(SRHWVSDB) && !defined(SRHWVSDB_DISABLED)
	/* 'BCMATTACHFN' function is always contained in RAM, so no _ENAB() macro usage here */
	if ((wlc->srvsdb_info = (struct wlc_srvsdb_info*)
	     MALLOCZ(osh, sizeof(struct wlc_srvsdb_info))) == NULL) {
		*err = 1033;
		goto fail;
	}
#endif /* SRHWVSDB SRHWVSDB_DISABLED */

#ifdef WLROAMPROF
	if (MAXBANDS > 0) {
		int i;

		if ((wlc->bandstate[0]->roam_prof = (wl_roam_prof_t *)
		     MALLOCZ(osh, sizeof(wl_roam_prof_t) *
		     WL_MAX_ROAM_PROF_BRACKETS * MAXBANDS)) == NULL) {
			*err = 1032;
			goto fail;
		}

		for (i = 1; i < MAXBANDS; i++) {
			wlc->bandstate[i]->roam_prof =
				&wlc->bandstate[0]->roam_prof[i * WL_MAX_ROAM_PROF_BRACKETS];
		}
	}
#endif /* WLROAMPROF */
#endif /* WLC_HIGH */

	return TRUE;

#ifdef WLC_HIGH
fail:
	return FALSE;
#endif // endif
}

static bool
BCMATTACHFN(wlc_attach_malloc_misc)(wlc_info_t *wlc, osl_t *osh, uint unit, uint *err, uint devid)
{
	return TRUE;
}

/*
 * The common driver entry routine. Error codes should be unique
 */
wlc_info_t *
BCMATTACHFN(wlc_attach_malloc)(osl_t *osh, uint unit, uint *err, uint devid, void *objr)
{
	wlc_info_t *wlc;

	if ((wlc = (wlc_info_t*) MALLOCZ(osh, sizeof(wlc_info_t))) == NULL) {
		*err = 1002;
		goto fail;
	}
	wlc->hwrxoff = WL_HWRXOFF;
	wlc->hwrxoff_pktget = (wlc->hwrxoff % 4) ?  wlc->hwrxoff : (wlc->hwrxoff + 2);

	/* Store the object registry */
	wlc->objr = objr;

	/* allocate wlc_pub_t state structure */
	if ((wlc->pub = wlc_pub_malloc(wlc, osh, unit, devid)) == NULL) {
		*err = 1003;
		goto fail;
	}
	wlc->pub->wlc = wlc;

	wlc->pub->max_modules = WLC_MAXMODULES;

#ifdef BCMPKTPOOL
	wlc->pub->pktpool = SHARED_POOL;
#endif /* BCMPKTPOOL */

#ifdef BCMFRAGPOOL
	wlc->pub->pktpool_lfrag = SHARED_FRAG_POOL;
#endif /* BCMFRAGPOOL */

#ifdef BCMRXFRAGPOOL
	wlc->pub->pktpool_rxlfrag = SHARED_RXFRAG_POOL;
#endif /* BCMRXFRAGPOOL */

	if ((wlc->corestate = (wlccore_t*)
	     MALLOCZ(osh, sizeof(wlccore_t))) == NULL) {
		*err = 1011;
		goto fail;
	}

#ifdef WLOFFLD
	if ((wlc->_wlc_rate_measurement = (wlc_rate_measurement_t *)
		MALLOCZ(osh, sizeof(wlc_rate_measurement_t))) == NULL) {
		*err = 1012;
		goto fail;
	}
#endif /* WLOFFLD */

	if (!wlc_attach_malloc_high(wlc, osh, unit, err, devid))
		goto fail;

	if (!wlc_attach_malloc_misc(wlc, osh, unit, err, devid))
		goto fail;

	return wlc;

fail:
	wlc_detach_mfree(wlc, osh);
	return NULL;
}
extern void *get_osh_cmn(osl_t *osh);

static void
BCMATTACHFN(wlc_detach_mfree_high)(wlc_info_t *wlc, osl_t *osh)
{
#ifdef WLC_HIGH
	int idx;
	if (wlc->scan_results) {
		MFREE(osh, wlc->scan_results, sizeof(wlc_bss_list_t));
		wlc->scan_results = NULL;
	}

	if (wlc->custom_scan_results) {
		MFREE(osh, wlc->custom_scan_results, sizeof(wlc_bss_list_t));
		wlc->custom_scan_results = NULL;
	}

	if (wlc->modulecb && (obj_registry_unref(wlc->objr, OBJR_MODULE_ID) == 0)) {
		obj_registry_set(wlc->objr, OBJR_MODULE_ID, NULL);
		MFREE(osh, wlc->modulecb, sizeof(modulecb_t) * wlc->pub->max_modules);
	}
	wlc->modulecb = NULL;

	if (wlc->modulecb_data) {
		MFREE(osh, wlc->modulecb_data, sizeof(modulecb_data_t) * wlc->pub->max_modules);
		wlc->modulecb_data = NULL;
	}

	if (wlc->bsscfg && (obj_registry_unref(wlc->objr, OBJR_BSSCFG_PTR) == 0)) {
		obj_registry_set(wlc->objr, OBJR_BSSCFG_PTR, NULL);
		MFREE(osh, wlc->bsscfg, sizeof(wlc_bsscfg_t*) * WLC_MAXBSSCFG);
	}
	wlc->bsscfg = NULL;

	if (obj_registry_unref(wlc->objr, OBJR_DEFAULT_BSS) == 0) {
		if (wlc->default_bss) {
			MFREE(osh, wlc->default_bss, sizeof(wlc_bss_info_t));
			wlc->default_bss = NULL;
		}
		obj_registry_set(wlc->objr, OBJR_DEFAULT_BSS, NULL);
	}

	if (wlc->txmod_fns)
		MFREE(osh, wlc->txmod_fns, sizeof(txmod_info_t) * TXMOD_LAST);

	if (wlc->stf) {
		MFREE(osh, wlc->stf, sizeof(wlc_stf_t));
		wlc->stf = NULL;
	}

#if defined(DELTASTATS)
	if (wlc->delta_stats) {
		MFREE(osh, wlc->delta_stats, sizeof(delta_stats_info_t));
		wlc->delta_stats = NULL;
	}
#endif /* DELTASTATS */
#if defined(SRHWVSDB) && !defined(SRHWVSDB_DISABLED)
	if (wlc->srvsdb_info) {
		MFREE(osh, wlc->srvsdb_info, sizeof(struct wlc_srvsdb_info));
		wlc->srvsdb_info = NULL;
	}
#endif /* SRHWVSDB SRHWVSDB_DISABLED */
#if defined(WLTEST)
	if (wlc->cmn->lifetime_mg) {
		MFREE(osh, wlc->cmn->lifetime_mg, sizeof(wl_lifetime_mg_t));
		wlc->cmn->lifetime_mg = NULL;
	}
#endif // endif
	if (obj_registry_unref(wlc->objr, OBJR_WLC_CMN_INFO) == 0) {
		obj_registry_set(wlc->objr, OBJR_WLC_CMN_INFO, NULL);
		MFREE(osh, wlc->cmn, sizeof(wlc_cmn_info_t));
	}
	else {
		idx = wlc_get_wlcindex(wlc);
		ASSERT(idx >= 0);
		wlc->cmn->wlc[idx] = NULL;
	}

	if (wlc->as_info && obj_registry_unref(wlc->objr, OBJR_ASSOCIATION_INFO) == 0) {
#ifdef STA
		if (wlc->as_info->join_targets) {
			MFREE(osh, wlc->as_info->join_targets, sizeof(wlc_bss_list_t));
			wlc->as_info->join_targets = NULL;
		}
#endif /* STA */

		if (wlc->as_info) {
			MFREE(osh, wlc->as_info, sizeof(wlc_assoc_info_t));
		}
		obj_registry_set(wlc->objr, OBJR_ASSOCIATION_INFO, NULL);
	}

	if (wlc->bandstate[0] && obj_registry_unref(wlc->objr, OBJR_WLC_BANDSTATE) == 0) {
		obj_registry_set(wlc->objr, OBJR_WLC_BANDSTATE, NULL);
#ifdef WLROAMPROF
		if (wlc->bandstate[0]->roam_prof) {
			MFREE(osh, wlc->bandstate[0]->roam_prof,
				sizeof(wl_roam_prof_t) * MAXBANDS * WL_MAX_ROAM_PROF_BRACKETS);
		}
#endif /* WLROAMPROF */
		MFREE(osh, wlc->bandstate[0], (sizeof(wlcband_t) * MAXBANDS));
	}

	if (wlc->bandinst) {
		int i = 0;
		for (i = 0; i < MAXBANDS; i++) {
			if (wlc->bandinst[i]) {
				MFREE(osh, wlc->bandinst[i], sizeof(wlcband_inst_t));
				wlc->bandinst[i] = NULL;
			}
		}
		MFREE(osh, wlc->bandinst, sizeof(wlcband_inst_t *) * MAXBANDS);
		wlc->bandinst = NULL;
	}
#endif /* WLC_HIGH */
}

static void
BCMATTACHFN(wlc_detach_mfree_misc)(wlc_info_t *wlc, osl_t *osh)
{
}

void
BCMATTACHFN(wlc_detach_mfree)(wlc_info_t *wlc, osl_t *osh)
{
	if (wlc == NULL)
		return;

	wlc_detach_mfree_misc(wlc, osh);

	wlc_detach_mfree_high(wlc, osh);

#ifdef WLOFFLD
	if (wlc->_wlc_rate_measurement) {
		MFREE(osh, wlc->_wlc_rate_measurement, sizeof(wlc_rate_measurement_t));
		wlc->_wlc_rate_measurement = NULL;
	}

#endif /* WLOFFLD */

	if (wlc->corestate) {
#ifdef WLC_HIGH
		if (wlc->corestate->macstat_snapshot) {
			MFREE(osh, wlc->corestate->macstat_snapshot,
				sizeof(uint16) * MACSTAT_OFFSET_SZ);
			wlc->corestate->macstat_snapshot = NULL;
		}

#if defined(WL_PSMX)
		if (wlc->corestate->macxstat_snapshot) {
			MFREE(osh, wlc->corestate->macxstat_snapshot,
				sizeof(uint16) * MACXSTAT_OFFSET_SZ);
			wlc->corestate->macxstat_snapshot = NULL;
		}
#endif // endif
#endif /* WLC_HIGH */
		MFREE(osh, wlc->corestate, sizeof(wlccore_t));
		wlc->corestate = NULL;
	}

#ifdef WLCHANIM
	if (wlc->chanim_info) {
		wlc_chanim_mfree(osh, wlc->chanim_info);
		wlc->chanim_info = NULL;
	}
#endif // endif

#if defined(PKTC) || defined(PKTC_DONGLE)
	if (wlc->pktc_info) {
		MFREE(osh, wlc->pktc_info, sizeof(wlc_pktc_info_t));
		wlc->pktc_info = NULL;
	}
#endif // endif

	if (wlc->pub) {
		/* free pub struct */
		wlc_pub_mfree(wlc, osh, wlc->pub);
		wlc->pub = NULL;
	}

	/* free the wlc */
	MFREE(osh, wlc, sizeof(wlc_info_t));
	wlc = NULL;
}
