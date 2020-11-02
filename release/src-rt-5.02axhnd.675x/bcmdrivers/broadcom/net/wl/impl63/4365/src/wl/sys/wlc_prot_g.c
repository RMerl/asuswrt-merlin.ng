/*
 * 11g protection module source
 * Broadcom 802.11abg Networking Device Driver
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
 * $Id: wlc_prot_g.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlanSwArchitectureProt]
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
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_dbg.h>
#include <wlc_scb.h>
#include <wlc_prot.h>
#include <wlc_prot_priv.h>
#include <wlc_prot_g.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>

/* iovar table */
enum {
	IOV_NONERP_ASSOC,	/* get status of nonerp_assoc */
};

static const bcm_iovar_t prot_g_iovars[] = {
#ifdef BCMDBG
	{"nonerp_present", IOV_NONERP_ASSOC, (0), IOVT_BOOL, 0},
#endif // endif
	{NULL, 0, 0, 0, 0}
};

/* ioctl table */
static const wlc_ioctl_cmd_t prot_g_ioctls[] = {
	{WLC_GET_LEGACY_ERP, 0, 0},
	{WLC_SET_LEGACY_ERP, 0, 0},
	{WLC_GET_GMODE_PROTECTION, 0, 0},
	{WLC_GET_GMODE_PROTECTION_OVERRIDE, 0, 0},
	{WLC_SET_GMODE_PROTECTION_OVERRIDE, 0, 0}
};

/* module private states */
typedef struct {
	wlc_info_t *wlc;
	int cfgh;		/* bsscfg cubby handle */
	uint16 cfg_offset;	/* bss_prot_g_cfg_t offset in bsscfg cubby client */
	uint16 cond_offset;	/* bss_prot_g_cond_t offset in bsscfg cubby client */
	uint16 to_offset;	/* bss_prot_g_to_t offset in bsscfg cubby client */
	uint8 gmode_user;	/* user config gmode, operating band->gmode is different */
} wlc_prot_g_info_priv_t;

/* wlc_prot_g_info_priv_t offset in module states */
static uint16 wlc_prot_g_info_priv_offset = sizeof(wlc_prot_g_info_t);

/* module states layout */
typedef struct {
	wlc_prot_g_info_t pub;
	wlc_prot_g_info_priv_t priv;
} wlc_prot_g_t;
/* module states size */
#define WLC_PROT_G_SIZE	(sizeof(wlc_prot_g_t))
/* moudle states location */
#define WLC_PROT_G_INFO_PRIV(prot) ((wlc_prot_g_info_priv_t *) \
				    ((uintptr)(prot) + wlc_prot_g_info_priv_offset))

/* bsscfg specific states */
/* configurations */
typedef	struct {
	int8	g_override;		/* override for use of g spec protection */
	bool	erp_ie_use_protection;	/* currently advertising Use_Protection */
	bool	erp_ie_nonerp;		/* currently advertising NonERP_Present */
	int8	barker_preamble;	/* current Barker Preamble Mode */
	bool	include_legacy_erp;	/* include Legacy ERP info elt ID 47 as well as g ID 42 */
	bool	barker_overlap_control; /* TRUE: be aware of overlapping BSSs for barker */
} bss_prot_g_cfg_t;

/* conditions - each field must be 8 bits */
typedef struct {
	uint8	nonerp_assoc;		/* NonERP STAs associated */
	uint8	longslot_assoc;		/* Long Slot timing only STAs associated */
	uint8	longpre_assoc;		/* bss/ibss: cck long preamble only STAs associated */
} bss_prot_g_cond_t;
/* access macros */
#define NONERP_ASSOC	OFFSETOF(bss_prot_g_cond_t, nonerp_assoc)
#define LONGSLOT_ASSOC	OFFSETOF(bss_prot_g_cond_t, longslot_assoc)
#define LONGPRE_ASSOC	OFFSETOF(bss_prot_g_cond_t, longpre_assoc)

/* timeouts */
typedef struct {
	/* 11g */
	uint	longpre_detect_timeout;	/* #sec until long preamble bcns gone */
	uint	barker_detect_timeout;	/* #sec until bcns signaling Barker long preamble
					 * only is gone
					 */
	uint	nonerp_ibss_timeout;	/* #sec until nonerp IBSS beacons gone */
	uint	nonerp_ovlp_timeout;	/* #sec until nonerp overlapping BSS bcns gone */
	uint	g_ibss_timeout;		/* #sec until bcns signaling Use_Protection gone */
} bss_prot_g_to_t;

/* bsscfg states layout */
typedef struct {
	wlc_prot_g_cfg_t pub;
	bss_prot_g_cfg_t cfg;
	bss_prot_g_cond_t cond;
	bss_prot_g_to_t to;
} bss_prot_g_t;
/* bsscfg states size */
#define BSS_PROT_G_SIZE	(sizeof(bss_prot_g_t))
/* bsscfg states location */
#define BSS_PROT_G_CUBBY_LOC(prot, cfg) ((bss_prot_g_t **)BSSCFG_CUBBY((cfg), (prot)->cfgh))
#define BSS_PROT_G_CUBBY(prot, cfg) (*BSS_PROT_G_CUBBY_LOC(prot, cfg))
#define BSS_PROT_G_CFG(prot, cfg) ((bss_prot_g_cfg_t *) \
				   ((uintptr)BSS_PROT_G_CUBBY(prot, cfg) + \
				    WLC_PROT_G_INFO_PRIV(prot)->cfg_offset))
#define BSS_PROT_G_COND(prot, cfg) ((bss_prot_g_cond_t *) \
				    ((uintptr)BSS_PROT_G_CUBBY(prot, cfg) + \
				     WLC_PROT_G_INFO_PRIV(prot)->cond_offset))
#define BSS_PROT_G_TO(prot, cfg) ((bss_prot_g_to_t *) \
				  ((uintptr)BSS_PROT_G_CUBBY(prot, cfg) + \
				   WLC_PROT_G_INFO_PRIV(prot)->to_offset))

/* local functions */

/* module entries */
static int wlc_prot_g_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
static void wlc_prot_g_watchdog(void *ctx);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_prot_g_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif
static int wlc_prot_g_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif);

/* bsscfg cubby */
static int wlc_prot_g_bss_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_prot_g_bss_deinit(void *ctx, wlc_bsscfg_t *cfg);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void wlc_prot_g_bss_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_prot_g_bss_dump NULL
#endif // endif

/* update configuration */
static void wlc_prot_g_cfg_upd(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg, uint idx, int val);
/* wlc_prot_g_cfg_upd() idx */
#define	WLC_PROT_G_SPEC		1	/* _g variable */
#define	WLC_PROT_G_OVR		2	/* g_override variable */
#define WLC_PROT_G_INCLEGACYERP	3	/* include_legacy_erp variable */
#define WLC_PROT_G_ERPIENONERP	4	/* erp_ie_nonerp variable */
#define WLC_PROT_G_ERPIEUSEPROT	5	/* erp_ie_use_protection variable */
#define WLC_PROT_G_BARKERPAM	6	/* barker_preamble variable */
#define WLC_PROT_G_BARKEROVLP	7	/* barker_overlap_control variable */

#ifdef AP
/* update condition */
static void wlc_prot_g_cond_set(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg, uint coff, bool set);
#endif // endif

/* IE mgmt */
static uint wlc_prot_g_calc_erp_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_prot_g_write_erp_ie(void *ctx, wlc_iem_build_data_t *data);
#ifdef STA
static int wlc_prot_g_bcn_parse_erp_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif // endif

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* 11g protection code starts here... */

/* module entries */
wlc_prot_g_info_t *
BCMATTACHFN(wlc_prot_g_attach)(wlc_info_t *wlc)
{
	wlc_prot_g_info_t *prot;
	wlc_prot_g_info_priv_t *priv;
	uint j;
	uint16 bcnfstbmp = FT2BMP(FC_BEACON) | FT2BMP(FC_PROBE_RESP);

	/* sanity check */
	ASSERT(wlc != NULL);

	/* allocate module states */
	if ((prot = MALLOCZ(wlc->osh, WLC_PROT_G_SIZE)) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	wlc_prot_g_info_priv_offset = OFFSETOF(wlc_prot_g_t, priv);
	priv = WLC_PROT_G_INFO_PRIV(prot);
	priv->wlc = wlc;

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((prot->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_prot_g_t *),
	                wlc_prot_g_bss_init, wlc_prot_g_bss_deinit, wlc_prot_g_bss_dump,
	                prot)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	priv->cfg_offset = OFFSETOF(bss_prot_g_t, cfg);
	priv->cond_offset = OFFSETOF(bss_prot_g_t, cond);
	priv->to_offset = OFFSETOF(bss_prot_g_t, to);

	/* register module callbacks */
	if (wlc_module_register(wlc->pub, prot_g_iovars, "prot_g", prot, wlc_prot_g_doiovar,
	                        wlc_prot_g_watchdog, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

	if (wlc_module_add_ioctl_fn(wlc->pub, prot, wlc_prot_g_doioctl,
	                            ARRAYSIZE(prot_g_ioctls), prot_g_ioctls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_add_ioctl_fn() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* IE mgmt callbacks */
	/* calc/build */
	/* bcn/prbrsp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, bcnfstbmp, DOT11_MNG_ERP_ID,
	      wlc_prot_g_calc_erp_ie_len, wlc_prot_g_write_erp_ie, prot) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_build_fn failed, erp in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* parse */
#ifdef STA
	/* bcn */
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_BEACON, DOT11_MNG_ERP_ID,
	                         wlc_prot_g_bcn_parse_erp_ie, prot) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, erp in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "prot_g", wlc_prot_g_dump, (void *)prot);
#endif // endif

	/* default configurations */
	for (j = 0; j < NBANDS(wlc); j++) {
		wlcband_t *band;

		/* Use band 1 for single band 11a */
		if (IS_SINGLEBAND_5G(wlc->deviceid))
			j = BAND_5G_INDEX;

		band = wlc->bandstate[j];

		/* init gmode value */
		if (BAND_2G(band->bandtype)) {
			wlc_prot_g_cfg_set(prot, WLC_PROT_G_USER, GMODE_AUTO);
		}
	}

	return prot;

	/* error handling */
fail:
	wlc_prot_g_detach(prot);
	return NULL;
}

void
BCMATTACHFN(wlc_prot_g_detach)(wlc_prot_g_info_t *prot)
{
	wlc_prot_g_info_priv_t *priv;
	wlc_info_t *wlc;

	if (prot == NULL)
		return;

	priv = WLC_PROT_G_INFO_PRIV(prot);

	wlc = priv->wlc;

	wlc_module_remove_ioctl_fn(wlc->pub, prot);
	wlc_module_unregister(wlc->pub, "prot_g", prot);

	MFREE(wlc->osh, prot, WLC_PROT_G_SIZE);
}

static int
wlc_prot_g_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_prot_g_info_t *prot = (wlc_prot_g_info_t *)ctx;
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	wlc_bsscfg_t *cfg;
	int err = BCME_OK;
	int32 *ret_int_ptr;
	bss_prot_g_cond_t *pgd;

	/* update bsscfg w/provided interface context */
	cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(cfg != NULL);

	pgd = BSS_PROT_G_COND(prot, cfg);
	ASSERT(pgd != NULL);
	BCM_REFERENCE(pgd);

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;
	BCM_REFERENCE(ret_int_ptr);

	switch (actionid) {
#ifdef BCMDBG
	case IOV_GVAL(IOV_NONERP_ASSOC):
		*ret_int_ptr = pgd->nonerp_assoc ? 1 : 0;
		break;
#endif // endif

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static void
wlc_prot_g_watchdog(void *ctx)
{
	wlc_prot_g_info_t *prot = (wlc_prot_g_info_t *)ctx;
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	int idx;
	wlc_bsscfg_t *cfg;

#ifdef AP
	/* XXX wlc_ap_process_assocreq() doesn't check 'gmode' so we shouldn't either...
	 * if (wlc->band->gmode) {
	 */
	FOREACH_UP_AP(wlc, idx, cfg) {
		bss_prot_g_cond_t *pgd;

		pgd = BSS_PROT_G_COND(prot, cfg);
		ASSERT(pgd != NULL);

		/* update our nonerp flag based on associated STAs */
		if (pgd->nonerp_assoc && !wlc_prot_scb_scan(wlc, cfg, SCB_NONERP, SCB_NONERP))
			wlc_prot_g_cond_set(prot, cfg, NONERP_ASSOC, FALSE);

		/* update our longslot flag based on associated STAs */
		if (pgd->longslot_assoc && !wlc_prot_scb_scan(wlc, cfg, SCB_LONGSLOT, SCB_LONGSLOT))
			wlc_prot_g_cond_set(prot, cfg, LONGSLOT_ASSOC, FALSE);

		/* update our long preamble flag based on associated STAs */
		if (pgd->longpre_assoc && !wlc_prot_scb_scan(wlc, cfg, SCB_SHORTPREAMBLE, 0))
			wlc_prot_g_cond_set(prot, cfg, LONGPRE_ASSOC, FALSE);
	}
	/* }
	 */
#endif /* AP */

	FOREACH_AS_BSS(wlc, idx, cfg) {
		bss_prot_g_to_t *pgt;

		pgt = BSS_PROT_G_TO(prot, cfg);
		ASSERT(pgt != NULL);

		/* decrement beacon detection timeouts */

		/* overlapping NonERP (legacy, non-11g) bcn */
		if (pgt->nonerp_ovlp_timeout)
			pgt->nonerp_ovlp_timeout--;
		/* member NonERP IBSS bcn */
		if (pgt->nonerp_ibss_timeout)
			pgt->nonerp_ibss_timeout--;
		/* Beacon from a long-only preamble STA detected */
		if (pgt->longpre_detect_timeout)
			pgt->longpre_detect_timeout--;
		/* Beacon signaling Barker long preamble only mode detected */
		if (pgt->barker_detect_timeout)
			pgt->barker_detect_timeout--;
		/* Beacon signaling Use_Protection detected */
		if (pgt->g_ibss_timeout)
			pgt->g_ibss_timeout--;

		if (BSSCFG_AP(cfg) || !cfg->BSS) {
			if (wlc->band->gmode)
				wlc_prot_g_mode_upd(prot, cfg);
		}
	}
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_prot_g_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_prot_g_info_t *prot = (wlc_prot_g_info_t *)ctx;
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	int idx;
	wlc_bsscfg_t *cfg;

	bcm_bprintf(b, "prot_g: priv_offset %d cfgh %d gmode_user %d\n",
	            wlc_prot_g_info_priv_offset, prot->cfgh, priv->gmode_user);

	FOREACH_AS_BSS(wlc, idx, cfg) {
		bcm_bprintf(b, "bsscfg %d >\n", WLC_BSSCFG_IDX(cfg));
	        wlc_prot_g_bss_dump(prot, cfg, b);
	}

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

/* bsscfg cubby */
static int
wlc_prot_g_bss_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_prot_g_info_t *prot = (wlc_prot_g_info_t *)ctx;
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	bss_prot_g_t **ppg = BSS_PROT_G_CUBBY_LOC(prot, cfg);
	bss_prot_g_t *pg = NULL;
	int err = BCME_OK;

	/* allocate memory and point bsscfg cubby to it */
	if ((pg = MALLOCZ(wlc->osh, BSS_PROT_G_SIZE)) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		err = BCME_NOMEM;
		goto fail;
	}
	*ppg = pg;

	/* default 11g protection configurations */
	wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_SPEC, FALSE);
	wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_OVR, WLC_PROTECTION_AUTO);
	wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_BARKEROVLP, TRUE);
	wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_BARKERPAM, WLC_BARKER_SHORT_ALLOWED);

#if !defined(MACOSX) && !defined(__NetBSD__)
	/* 802.11g draft 4.0 NonERP elt advertisement */
	wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_INCLEGACYERP, TRUE);
#endif // endif

	return BCME_OK;

fail:
	wlc_prot_g_bss_deinit(ctx, cfg);
	return err;
}

static void
wlc_prot_g_bss_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_prot_g_info_t *prot = (wlc_prot_g_info_t *)ctx;
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc;
	bss_prot_g_t **ppg = BSS_PROT_G_CUBBY_LOC(prot, cfg);
	bss_prot_g_t *pg = *ppg;

	if (pg == NULL)
		return;

	wlc = priv->wlc;

	MFREE(wlc->osh, pg, BSS_PROT_G_SIZE);
	*ppg = NULL;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
wlc_prot_g_bss_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_prot_g_info_t *prot = (wlc_prot_g_info_t *)ctx;
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_prot_g_cfg_t *wpgc;
	bss_prot_g_cfg_t *pgc;
	bss_prot_g_cond_t *pgd;
	bss_prot_g_to_t *pgt;

	ASSERT(cfg != NULL);

	wpgc = WLC_PROT_G_CFG(prot, cfg);
	ASSERT(wpgc != NULL);

	pgc = BSS_PROT_G_CFG(prot, cfg);
	ASSERT(pgc != NULL);

	pgd = BSS_PROT_G_COND(prot, cfg);
	ASSERT(pgd != NULL);

	pgt = BSS_PROT_G_TO(prot, cfg);
	ASSERT(pgt != NULL);

	bcm_bprintf(b, "\tcfg_offset %d cond_offset %d to_offset %d\n",
	            priv->cfg_offset, priv->cond_offset, priv->to_offset);
	bcm_bprintf(b, "\tnonerp %d use_prot %d barker_preamble %d\n",
	            pgc->erp_ie_nonerp, pgc->erp_ie_use_protection,
	            pgc->barker_preamble);
	bcm_bprintf(b, "\tg_prot %d g_prot_ovrrd %d\n",
	            wpgc->_g, pgc->g_override);
	bcm_bprintf(b, "\tinclude_legacy_erp %d barker_overlap_control %d\n",
	            pgc->include_legacy_erp, pgc->barker_overlap_control);

	bcm_bprintf(b, "\tlongpre_assoc %d nonerp_assoc %d longslot_assoc %d\n",
	            pgd->longpre_assoc, pgd->nonerp_assoc,
	            pgd->longslot_assoc);

	bcm_bprintf(b, "\tlongpre_detect_timeout %d barker_detect_timeout %d "
	            "nonerp_ovlp_timeout %d nonerp_ibss_timeout %d "
	            "g_prot_ibss_detect_timeout %d\n",
	            pgt->longpre_detect_timeout, pgt->barker_detect_timeout,
	            pgt->nonerp_ovlp_timeout, pgt->nonerp_ibss_timeout,
	            pgt->g_ibss_timeout);
}
#endif /* BCMDBG || BCMDBG_DUMP */

/* centralized protection config change function to simplify debugging, no consistency checking
 * this should be called only on changes to avoid overhead in periodic function
 */
void
wlc_prot_g_cfg_set(wlc_prot_g_info_t *prot, uint idx, int val)
{
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);

	WL_TRACE(("wlc_prot_g_cfg_set: idx %d, val %d\n", idx, val));

	switch (idx) {
	case WLC_PROT_G_USER:
		priv->gmode_user = (uint8)val;
		break;

	default:
		ASSERT(0);
		break;
	}
}

static void
wlc_prot_g_cfg_upd(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg, uint idx, int val)
{
	wlc_prot_g_info_priv_t *priv;
	wlc_prot_g_cfg_t *wpgc;
	bss_prot_g_cfg_t *pgc;

	ASSERT(cfg != NULL);

	wpgc = WLC_PROT_G_CFG(prot, cfg);
	ASSERT(wpgc != NULL);

	pgc = BSS_PROT_G_CFG(prot, cfg);
	ASSERT(pgc != NULL);

	priv = WLC_PROT_G_INFO_PRIV(prot);

	WL_PROT(("wl%d.%d %s: idx %d, val %d\n",
	         priv->wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, idx, val));
	BCM_REFERENCE(priv);

	switch (idx) {
	case WLC_PROT_G_SPEC:
		wpgc->_g = (bool)val;
		break;
	case WLC_PROT_G_OVR:
		pgc->g_override = (int8)val;
		break;
	case WLC_PROT_G_INCLEGACYERP:
		pgc->include_legacy_erp = (bool)val;
		break;
	case WLC_PROT_G_ERPIENONERP:
		pgc->erp_ie_nonerp = (bool)val;
		break;
	case WLC_PROT_G_ERPIEUSEPROT:
		pgc->erp_ie_use_protection = (bool)val;
		break;
	case WLC_PROT_G_BARKERPAM:
		pgc->barker_preamble = (int8)val;
		break;
	case WLC_PROT_G_BARKEROVLP:
		pgc->barker_overlap_control = (bool)val;
		break;

	default:
		ASSERT(0);
		break;
	}
}

/* update g_cfg, but comply with its override */
void
wlc_prot_g_cfg_init(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg)
{
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	wlc_prot_g_cfg_t *wpgc;
	bss_prot_g_cfg_t *pgc;

	/* shared */
	wlc_prot_cfg_init(wlc->prot, cfg);

	/* 11g specific */
	ASSERT(cfg != NULL);

	wpgc = WLC_PROT_G_CFG(prot, cfg);
	ASSERT(wpgc != NULL);

	pgc = BSS_PROT_G_CFG(prot, cfg);
	ASSERT(pgc != NULL);

	/* don't update if there is no change */
	if (pgc->g_override == WLC_PROTECTION_AUTO) {
		if (wpgc->_g)
			wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_SPEC, (int)FALSE);
	}
	else {
		if (wpgc->_g != (pgc->g_override == WLC_PROTECTION_ON))
			wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_SPEC,
			                   (pgc->g_override == WLC_PROTECTION_ON));
	}
}

void
wlc_prot_g_mode_reset(wlc_prot_g_info_t *prot)
{
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;

	wlc_set_gmode(wlc, priv->gmode_user, FALSE);
}

/* update g_protection, barker_preamble and shortslot_override
 * parameters.
 * g_protection gets update based on protection control and override
 * setting.
 * whenever the state of protection changed, the beacon and probe also
 * gets updated with new protection setting.
 */
bool
wlc_prot_g_mode_upd(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg)
{
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	bool update = FALSE;
	bool nonerp;
	bool erp_ie_use_protection = FALSE;
	int8 barker_preamble;
	bool shortslot = wlc->shortslot;
	bool g_prot;
	bss_prot_cfg_t *pc;
	wlc_prot_g_cfg_t *wpgc;
	bss_prot_g_cfg_t *pgc;
	bss_prot_g_cond_t *pgd;
	bss_prot_g_to_t *pgt;

	ASSERT(cfg != NULL);

	pc = BSS_PROT_CFG(wlc->prot, cfg);
	ASSERT(pc != NULL);

	wpgc = WLC_PROT_G_CFG(prot, cfg);
	ASSERT(wpgc != NULL);

	pgc = BSS_PROT_G_CFG(prot, cfg);
	ASSERT(pgc != NULL);

	pgd = BSS_PROT_G_COND(prot, cfg);
	ASSERT(pgd != NULL);

	pgt = BSS_PROT_G_TO(prot, cfg);
	ASSERT(pgt != NULL);

	ASSERT(wlc->band->gmode);

	nonerp = (pgd->nonerp_assoc || pgt->nonerp_ibss_timeout > 0);

	if (pgc->erp_ie_nonerp != nonerp) {
		wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_ERPIENONERP, nonerp);
		update = TRUE;
	}

	/* Update current 11g protection */
	if (pgc->g_override == WLC_PROTECTION_AUTO) {
		/* Turn 11g protection on if NonERP STA associated or legacy BSSs on our channel */
		if (((pc->overlap == WLC_PROTECTION_CTL_LOCAL ||
		      pc->overlap == WLC_PROTECTION_CTL_OVERLAP) && nonerp) ||
		    (pc->overlap == WLC_PROTECTION_CTL_OVERLAP &&
		     pgt->nonerp_ovlp_timeout != 0)) {
			erp_ie_use_protection = TRUE;
		}

		/* Use 11g protection if we are advertising protection or
		 * we have seen Use_Protection signaled in our IBSS
		 */
		g_prot = (erp_ie_use_protection || pgt->g_ibss_timeout);

	} else {
		g_prot = (pgc->g_override == WLC_PROTECTION_ON);
		erp_ie_use_protection = g_prot;
	}

	if (wpgc->_g != g_prot)
		wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_SPEC, g_prot);

	if (pgc->erp_ie_use_protection != erp_ie_use_protection) {
		wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_ERPIEUSEPROT, erp_ie_use_protection);
		update = TRUE;
	}

	/* Update current 11g Barker Preamble Mode (short/long CCK preamble indications) */
	if (pgd->longpre_assoc || pgt->longpre_detect_timeout > 0)
		barker_preamble = WLC_BARKER_LONG_ONLY;
	else
		barker_preamble = WLC_BARKER_SHORT_ALLOWED;

	if (pgc->barker_preamble != barker_preamble) {
		wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_BARKERPAM, barker_preamble);
		update = TRUE;
	}

	/* decide short slot/long slot mode */
	if (BSSCFG_AP(cfg)) {
		if (wlc->shortslot_override == WLC_SHORTSLOT_AUTO) {
			shortslot = TRUE;
			if (pgd->longslot_assoc) {
				shortslot = FALSE;
			}
			else if (!wlc->ignore_bcns && pgt->nonerp_ovlp_timeout) {
				shortslot = FALSE;
			}
		} else {
			/* an override has been specified for shortslot mode */
			shortslot = (wlc->shortslot_override == WLC_SHORTSLOT_ON);
		}
	} else if (!cfg->BSS) {	/* IBSS shortslot setting */
		/* apply shortslot setting if override */
		if (wlc->shortslot_override != WLC_SHORTSLOT_AUTO)
			shortslot = (wlc->shortslot_override == WLC_SHORTSLOT_ON);
	}

	if (wlc->shortslot != shortslot) {
		wlc_switch_shortslot(wlc, shortslot);
		update = TRUE;
	}

	if (update) {
		wlc_bss_update_brcm_ie(wlc, cfg);
		WL_APSTA_BCN(("wl%d.%d: %s() -> wlc_update_beacon()\n",
		              wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		wlc_bss_update_beacon(wlc, cfg);
		wlc_bss_update_probe_resp(wlc, cfg, TRUE);
	}

	if ((pgc->barker_preamble == WLC_BARKER_LONG_ONLY) ||
	    (pgt->barker_detect_timeout > 0))
		wlc_prot_cfg_upd(wlc->prot, cfg, WLC_PROT_SHORTPREAMBLE, FALSE);
	else
		wlc_prot_cfg_upd(wlc->prot, cfg, WLC_PROT_SHORTPREAMBLE, TRUE);

	return update;
}

static void
_wlc_prot_g_ovlp_upd(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg,
	uint8 *erp, int erp_len, bool is_erp, bool short_cap)
{
	bss_prot_g_cfg_t *pgc;
	bss_prot_g_to_t *pgt;

	ASSERT(cfg != NULL);

	pgc = BSS_PROT_G_CFG(prot, cfg);
	ASSERT(pgc != NULL);

	pgt = BSS_PROT_G_TO(prot, cfg);
	ASSERT(pgt != NULL);

	/* Check for overlapping Barker long-preamble-only signaled */
	if (pgc->barker_overlap_control &&
	    erp_len > 0 && (erp[0] & DOT11_MNG_BARKER_PREAMBLE))
		pgt->barker_detect_timeout = WLC_IBSS_BCN_TIMEOUT;

	/* Check for overlapping NonERP long-preamble-only beacons */
	if (pgc->barker_overlap_control &&
	    !is_erp && !short_cap)
		pgt->longpre_detect_timeout = WLC_IBSS_BCN_TIMEOUT;

	/* Check for overlapping NonERP beacon
	 * Consider as NonERP beacon if there are no OFDM rates
	 * or the ERP Info indicates that NonERP STAs are associated
	 */
	if (!is_erp ||
	    (erp_len > 0 && (erp[0] & DOT11_MNG_NONERP_PRESENT))) {
		pgt->nonerp_ovlp_timeout = WLC_IBSS_BCN_TIMEOUT;
	}

	wlc_prot_g_mode_upd(prot, cfg);
}

void
wlc_prot_g_ovlp_upd(wlc_prot_g_info_t *prot, chanspec_t chspec,
	uint8 *erp, int erp_len, bool is_erp, bool short_cap)
{
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	int idx;
	wlc_bsscfg_t *cfg;

	FOREACH_AS_BSS(wlc, idx, cfg) {
		if (BSSCFG_STA(cfg) && cfg->BSS)
			continue;

		if (!CHSPEC_CTLOVLP(chspec, cfg->current_bss->chanspec, CH_20MHZ_APART))
			continue;

		_wlc_prot_g_ovlp_upd(prot, cfg, erp, erp_len, is_erp, short_cap);
	}
}

#ifdef STA
static void
wlc_prot_g_cfg_track(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg,
	uint8 *erp, int erp_len)
{
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	wlc_prot_g_cfg_t *wpgc;
	bss_prot_g_cfg_t *pgc;
	bss_prot_g_to_t *pgt;

	ASSERT(cfg != NULL);

	wpgc = WLC_PROT_G_CFG(prot, cfg);
	ASSERT(wpgc != NULL);

	pgc = BSS_PROT_G_CFG(prot, cfg);
	ASSERT(pgc != NULL);

	pgt = BSS_PROT_G_TO(prot, cfg);
	ASSERT(pgt != NULL);

	/* Track 11g modes */
	ASSERT(wlc->band->gmode);

	if (erp != NULL && erp_len > 0) {

		/* Track protection mode */
		if (pgc->g_override == WLC_PROTECTION_AUTO) {
			bool tmp = ((erp[0] & DOT11_MNG_USE_PROTECTION) != 0);
			if (wpgc->_g != tmp)
				wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_SPEC, tmp);
		}

		/* Track Barker Preamble Mode */
		if (erp[0] & DOT11_MNG_BARKER_PREAMBLE)
			wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_BARKERPAM, WLC_BARKER_LONG_ONLY);
		else
			wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_BARKERPAM,
			                   WLC_BARKER_SHORT_ALLOWED);

		/* update our shortpreamble use */
		if (pgc->barker_preamble == WLC_BARKER_LONG_ONLY ||
		    pgt->barker_detect_timeout > 0)
			wlc_prot_cfg_upd(wlc->prot, cfg, WLC_PROT_SHORTPREAMBLE, FALSE);
		else
			wlc_prot_cfg_upd(wlc->prot, cfg, WLC_PROT_SHORTPREAMBLE, TRUE);
	}
}
#endif /* STA */

#ifdef AP
static uint8 *
wlc_prot_g_cond_loc(void *ctx, wlc_bsscfg_t *cfg, uint off)
{
	wlc_prot_g_info_t *prot = (wlc_prot_g_info_t *)ctx;
	bss_prot_g_cond_t *pgd;

	ASSERT(cfg != NULL);

	pgd = BSS_PROT_G_COND(prot, cfg);
	ASSERT(pgd != NULL);

	return (uint8 *)pgd + off;
}

static void
wlc_prot_g_cond_set(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg, uint offset, bool set)
{
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);

	WL_PROT(("wl%d.%d %s: offset %d, val %d\n",
	         priv->wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, offset, set));

	wlc_prot_cond_set(priv->wlc, cfg, offset, set, wlc_prot_g_cond_loc, prot);
}

void
wlc_prot_g_cond_upd(wlc_prot_g_info_t *prot, struct scb *scb)
{
	wlc_bsscfg_t *cfg;

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);

	/* Check for association of a NonERP STA */
	if (scb->flags & SCB_NONERP)
		wlc_prot_g_cond_set(prot, cfg, NONERP_ASSOC, TRUE);

	/* Check for association of a non-short slot capable STA */
	if (scb->flags & SCB_LONGSLOT)
		wlc_prot_g_cond_set(prot, cfg, LONGSLOT_ASSOC, TRUE);

	/* Check for association of a non-short preamble capable STA */
	if (!(scb->flags & SCB_SHORTPREAMBLE))
		wlc_prot_g_cond_set(prot, cfg, LONGPRE_ASSOC, TRUE);
}
#endif /* AP */

void
wlc_prot_g_to_upd(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg,
	uint8 *erp, int erp_len, bool is_erp, bool shortpreamble)
{
	wlc_prot_g_info_priv_t *priv;
	bss_prot_g_to_t *pgt;

	ASSERT(cfg != NULL);

	pgt = BSS_PROT_G_TO(prot, cfg);
	ASSERT(pgt != NULL);

	priv = WLC_PROT_G_INFO_PRIV(prot);
	ASSERT(priv->wlc->band->gmode);
	BCM_REFERENCE(priv);

	if (erp_len > 0) {
		/* Check for Barker Long preamble only signaled in our IBSS */
		if (erp[0] & DOT11_MNG_BARKER_PREAMBLE) {
			pgt->barker_detect_timeout = WLC_IBSS_BCN_TIMEOUT;
		}
		/* Check for Use_Protection signaled in our IBSS */
		if (erp[0] & DOT11_MNG_USE_PROTECTION)
			pgt->g_ibss_timeout = WLC_IBSS_BCN_TIMEOUT;
	}

	/* Check for NonERP beacons */
	if (!is_erp)
		pgt->nonerp_ibss_timeout = WLC_IBSS_BCN_TIMEOUT;

	/* Check for long-preamble-only STAs in our IBSS */
	if (!shortpreamble)
		pgt->longpre_detect_timeout = WLC_IBSS_BCN_TIMEOUT;

	wlc_prot_g_mode_upd(prot, cfg);
}

/* ERP IE */
static uint
wlc_prot_g_calc_erp_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_prot_g_info_t *prot = (wlc_prot_g_info_t *)ctx;
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;

	if (!wlc->band->gmode)
		return 0;

	return TLV_HDR_LEN + DOT11_MNG_ERP_LEN;
}

static int
wlc_prot_g_write_erp_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_prot_g_info_t *prot = (wlc_prot_g_info_t *)ctx;
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	uint8 erp_flags[DOT11_MNG_ERP_LEN] = { 0 };
	bss_prot_g_cfg_t *pgc;

	if (!wlc->band->gmode)
		return BCME_OK;

	pgc = BSS_PROT_G_CFG(prot, data->cfg);
	ASSERT(pgc != NULL);

	if (pgc->erp_ie_nonerp)
		erp_flags[0] |= DOT11_MNG_NONERP_PRESENT;
	if (pgc->erp_ie_use_protection)
		erp_flags[0] |= DOT11_MNG_USE_PROTECTION;
	if (pgc->barker_preamble)
		erp_flags[0] |= DOT11_MNG_BARKER_PREAMBLE;

	/* ERP Information Element */
	bcm_write_tlv(DOT11_MNG_ERP_ID, erp_flags, DOT11_MNG_ERP_LEN, data->buf);
	return BCME_OK;
}

#ifdef STA
static int
wlc_prot_g_bcn_parse_erp_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_prot_g_info_t *prot = (wlc_prot_g_info_t *)ctx;
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	struct scb *scb = data->pparm->ft->bcn.scb;
	wlc_bsscfg_t *cfg = data->cfg;

	if (data->ie == NULL || data->ie_len <= TLV_BODY_OFF) {
		return BCME_OK;
	}
	if (!wlc->band->gmode)
		return BCME_OK;

	if (!cfg->BSS)
		return BCME_OK;

	/* Track 11g modes */
	wlc_prot_g_cfg_track(prot, cfg, &data->ie[TLV_BODY_OFF], data->ie[TLV_LEN_OFF]);

	/* update the AP's scb */
	if (scb == NULL)
		return BCME_OK;

	scb->flags &= ~SCB_SHORTPREAMBLE;
	if (!(data->ie[TLV_BODY_OFF] & DOT11_MNG_BARKER_PREAMBLE))
		scb->flags |= SCB_SHORTPREAMBLE;

	return BCME_OK;
}
#endif /* STA */

static int
wlc_prot_g_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	wlc_prot_g_info_t *prot = (wlc_prot_g_info_t *)ctx;
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	int val, *pval;
	bool bool_val;
	int err = BCME_OK;
	wlc_bsscfg_t *cfg;
	wlc_prot_g_cfg_t *wpgc;
	bss_prot_g_cfg_t *pgc;

	/* default argument is generic integer */
	pval = arg ? (int *) arg : NULL;

	/* This will prevent the misaligned access */
	if (pval && (uint32)len >= sizeof(val))
		bcopy(pval, &val, sizeof(val));
	else
		val = 0;

	/* bool conversion to avoid duplication below */
	bool_val = (val != 0);

	/* update bsscfg pointer */
	cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(cfg != NULL);

	wpgc = WLC_PROT_G_CFG(prot, cfg);
	ASSERT(wpgc != NULL);

	pgc = BSS_PROT_G_CFG(prot, cfg);
	ASSERT(pgc != NULL);

	switch (cmd) {
	case WLC_GET_LEGACY_ERP:
		*pval = pgc->include_legacy_erp;
		break;

	case WLC_SET_LEGACY_ERP:
		if (pgc->include_legacy_erp == bool_val)
			break;

		wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_INCLEGACYERP, bool_val);

		if (BSSCFG_AP(cfg) && wlc->clk) {
			WL_APSTA_BCN(("wl%d: WLC_SET_LEGACY_ERP -> wlc_update_beacon()\n",
				wlc->pub->unit));
			wlc_bss_update_beacon(wlc, cfg);
			wlc_bss_update_probe_resp(wlc, cfg, TRUE);
		}
		break;

	case WLC_GET_GMODE_PROTECTION:
		*pval = wpgc->_g;
		break;

	case WLC_GET_GMODE_PROTECTION_OVERRIDE:
		*pval = pgc->g_override;
		break;

	case WLC_SET_GMODE_PROTECTION_OVERRIDE:
		if ((val != WLC_PROTECTION_AUTO) &&
		    (val != WLC_PROTECTION_OFF) &&
		    (val != WLC_PROTECTION_ON)) {
			err = BCME_RANGE;
			break;
		}

		wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_OVR, val);

		if (!WLCISGPHY(wlc->band))
			break;

		if (cfg->associated) {
			/* let watchdog or beacon processing update protection */
		} else {
			wlc_prot_g_cfg_init(prot, cfg);
		}

		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

void
wlc_prot_g_init(wlc_prot_g_info_t *prot, wlc_bsscfg_t *cfg)
{
	wlc_prot_g_info_priv_t *priv = WLC_PROT_G_INFO_PRIV(prot);
	wlc_info_t *wlc = priv->wlc;
	wlc_prot_g_cfg_t *wpgc;
	bss_prot_g_cond_t *pgd;
	bss_prot_g_to_t *pgt;

	/* shared */
	wlc_prot_init(wlc->prot, cfg);

	/* 11g specific */
	ASSERT(cfg != NULL);

	wpgc = WLC_PROT_G_CFG(prot, cfg);
	ASSERT(wpgc != NULL);

	pgd = BSS_PROT_G_COND(prot, cfg);
	ASSERT(pgd != NULL);

	pgt = BSS_PROT_G_TO(prot, cfg);
	ASSERT(pgt != NULL);

	bzero(pgd, sizeof(*pgd));
	bzero(pgt, sizeof(*pgt));

	/* Update the gmode protection state */
	if ((BSSCFG_AP(cfg) || !cfg->BSS)) {
		if (wlc->band->gmode) {
			wlc_prot_g_cfg_init(prot, cfg);
			wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_ERPIEUSEPROT, wpgc->_g);
		}
		/* default ERP bits */
		wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_ERPIENONERP, FALSE);
		wlc_prot_g_cfg_upd(prot, cfg, WLC_PROT_G_BARKERPAM, WLC_BARKER_SHORT_ALLOWED);

	}
}
