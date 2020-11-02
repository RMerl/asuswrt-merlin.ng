/**
 * @file
 * @brief
 * Multiple connection capable hardware/ucode management source (a split from wlc_p2p.c)
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
 * $Id: wlc_mcnx.c 708017 2017-06-29 14:11:45Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#ifdef WLMCNX
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <bcmwpa.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_addrmatch.h>
#include <wlc_bmac.h>
#include <wlc_utils.h>
#include <wlc_mcnx.h>
#include <bcm_notif_pub.h>
#include <wl_dbg.h>

/* iovar table */
enum {
	IOV_MCNX,		/* feature enable/disble */
	IOV_MCNX_DEBUG,		/* debug message enable/disable */
};

static const bcm_iovar_t mcnx_iovars[] = {
	{"mcnx", IOV_MCNX, IOVF_SET_DOWN, IOVT_BOOL, 0},
#ifdef BCMDBG
	{"mcnx_dbg", IOV_MCNX_DEBUG, 0, IOVT_UINT32, 0},
#endif // endif
	{NULL, 0, 0, 0, 0}
};

/* P2P ucode module specific states */
struct wlc_mcnx_info {
	wlc_info_t	*wlc;	/* wlc info pointer */
	int		cfgh;	/* bsscfg cubby handle */
	/* d11 M_P2P_HPS block cache */
	uint16		hps;
	/* d11 BSS control blocks & bsscfg mapping */
	uint8		d11cbo[M_P2P_BSS_MAX];
	/* d11 RCMTA RA/BSSID/TA allocation */
	uint8		d11rao[CEIL(M_ADDR_BMP_BLK_SZ, NBBY)];
	/* d11 p2p SHM location */
	uint32		d11shm;
	/* notifier handles */
	bcm_notif_h	assoc_upd_notif_hdl;
	bcm_notif_h	bss_upd_notif_hdl;
	bcm_notif_h	tsf_upd_notif_hdl;
	bcm_notif_h	intr_notif_hdl;
	/* p2p indexes in shm address match */
	int		p2p_start_idx;
	int		p2p_max_idx;
	/* skip time update requests */
	int		mac_suspends;
/* ==== please keep these debug stuff at the bottom ==== */
#ifdef BCMDBG
	uint32		debug;
#endif // endif
};

/* debug */
#ifdef BCMDBG
/* printf */
#define MCNX_DBG_TSF	0x1
#define MCNX_DBG_INTR	0x2
#endif // endif
#ifdef BCMDBG
#define WL_MCNX_TSF(mcnx, x)	do {					\
		if (WL_MCNX_ON() && ((mcnx)->debug & MCNX_DBG_TSF))	\
			WL_PRINT(x);					\
	} while (0)
#define WL_MCNX_TSF_ON(mcnx)	(WL_MCNX_ON() && ((mcnx)->debug & MCNX_DBG_TSF))
#define WL_MCNX_INTR(mcnx, x)	do {					\
		if (WL_MCNX_ON() && ((mcnx)->debug & MCNX_DBG_INTR))	\
			WL_PRINT(x);					\
	} while (0)
#define WL_MCNX_INTR_ON(mcnx)	(WL_MCNX_ON() && ((mcnx)->debug & MCNX_DBG_INTR))
#else
#define WL_MCNX_TSF(mcnx, x)
#define WL_MCNX_TSF_ON(mcnx)	FALSE
#define WL_MCNX_INTR(mcnx, x)
#define WL_MCNX_INTR_ON(mcnx)	FALSE
#endif /* BCMDBG */
#ifdef BCMDBG
#define WL_MCNX_TS(wlc)		(wlc->clk ? R_REG(wlc->osh, &wlc->regs->tsf_timerlow) : 0xDEADDAED)
#endif // endif

/* d11cbo - BSS allocation */
#define D11_CBO_AVAIL	255	/* BSS block is available */
/* other values from 0 to 254 are valid bsscfg index */

/* d11cbi - BSS index */
#define D11_CBI_PRI	0	/* for primary bsscfg */

/* rcmta_ra/bssid_idx */
#define D11_RAI_PRI	(M_ADDR_BMP_BLK_SZ - 1)	/* for primary bsscfg */

/* update the pretbtt periodically in case the clock drifts.
 * ucode is supposed to do it as part of the TSF offset update but apparently
 * it doesn't seem the case so do it in the driver for now.
 */
#define TSFO_MAX_DIFF	(1 << P2P_UCODE_TIME_SHIFT)	/* in usec */

/* bsscfg cubby structure */
typedef struct {
	/* p2p flags */
	uint16		flags;
	/* base TSF offset (to detect clock drift) */
	int16		tsfo_base;
	/* multiple MAC address manipulation */
	uint8		rcmta_ra_idx;		/* RCMTA idx for RA to be used for P2P */
	uint8		rcmta_bssid_idx;	/* RCMTA idx for BSSID to be used for P2P */
	/* d11 info - see 'M_P2P_BSS_BLK' in d11.h */
	uint8		d11cbi;			/* d11 SHM per BSS control block index */
	/* network parms */
	/* XXX Update these two altogether when processing the received beacons. When updated
	 * they can be used to extraplate the NoA start and TBTT.
	 */
	uint32		tbtt_h;			/* remote tbtt in local TSF time */
	uint32		tbtt_l;			/* remote tbtt in local TSF time */
	int32		tsfo_h;			/* TSF offset (local - remote) */
	int32		tsfo_l;			/* TSF offset (local - remote) */
	uint16		bcn_prd;		/* beacon interval in TUs */
	/* bcn timestamp as tbtt adoptation states */
	uint32		bcn_offset;
	bool		bcn_offset_ok;
	bool		bcn_offset_cache;
	uint8		num_bad_bcn_offset;
#ifdef UC_TBTT_WAR
	/* last pretbtt time */
	uint32		pre_tbtt_h;
	uint32		pre_tbtt_l;
#endif // endif
	uint32		unaligned_tbtt_tsfo;	/* TSF offset for unaligned TBTT */
/* ==== please keep these debug stuff at the bottom ==== */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
struct {
	uint32 intr[M_P2P_I_BLK_SZ];
	uint32 bcnadopt;
} stats;
#endif // endif
} bss_mcnx_info_t;

/* mcnx info flags */
#define MCNX_TBTT_INFO	0x1	/* tbtt info in driver is init'd and valid */
#define MCNX_BSS_INFO	0x2	/* BSS info (TSF, BSS, etc.) in SHM is init'd */
#define MCNX_FORCED_HPS	0x4	/* h/w HPS bit(s) are set */

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#define INTRCNTINC(bmi, i)	((bmi)->stats.intr[i]++)
#define BCNADCNTINC(bmi)	((bmi)->stats.bcnadopt++)
#else
#define INTRCNTINC(bmi, i)	BCM_REFERENCE(bmi)
#define BCNADCNTINC(bmi)	BCM_REFERENCE(bmi)
#endif // endif

/* bsscfg specific info access accessor */
#define MCNX_BSSCFG_CUBBY_LOC(mcnx, cfg) ((bss_mcnx_info_t **)BSSCFG_CUBBY((cfg), (mcnx)->cfgh))
#define MCNX_BSSCFG_CUBBY(mcnx, cfg) (*(MCNX_BSSCFG_CUBBY_LOC(mcnx, cfg)))
#define BSS_MCNX_INFO(mcnx, cfg) MCNX_BSSCFG_CUBBY(mcnx, cfg)

/* local prototypes */

/* module */
static int wlc_mcnx_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
static int wlc_mcnx_up(void *ctx);
static int wlc_mcnx_down(void *ctx);

/* dump */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static uint16 wlc_mcnx_rcmta_type_get(wlc_mcnx_info_t *mcnx, int idx);
static int wlc_mcnx_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* bsscfg cubby */
static int wlc_mcnx_info_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_mcnx_info_deinit(void *ctx, wlc_bsscfg_t *cfg);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void wlc_mcnx_info_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);
#else
#define wlc_mcnx_info_dump NULL
#endif // endif

/* callbacks */
static void wlc_mcnx_bss_updn_cb(void *ctx, bsscfg_up_down_event_data_t *notif_data);

/* notifications */
static void wlc_mcnx_assoc_upd_notif(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool assoc);
static void wlc_mcnx_bss_upd_notif(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool up);
static void wlc_mcnx_tsf_upd_notif(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg);
static void wlc_mcnx_intr_notif(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	int intr, uint32 tsf_h, uint32 tsf_l);

/* time */
static void _wlc_mcnx_tsf_adopt(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 ltsf_h, uint32 ltsf_l, uint32 rtsf_h, uint32 rtsf_l);
static void wlc_mcnx_tbtt_cache(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 tsf_h, uint32 tsf_l);

/* address match tables */
static int wlc_set_bssid_hw(wlc_info_t *wlc, int index, const struct ether_addr *ea, bool a2_also);
static int wlc_set_ra_hw(wlc_info_t *wlc, int index, const struct ether_addr *ea);
#define PRIMARY_INTF -1	/* Primary interface uses RXE (<rev 40) or hardcoded indexes (>=rev 40) */

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* module attach/detach */
wlc_mcnx_info_t *
BCMATTACHFN(wlc_mcnx_attach)(wlc_info_t *wlc)
{
	wlc_mcnx_info_t *mcnx;
	bcm_notif_module_t *notif;

	/* sanity check */
	ASSERT(wlc != NULL);

	/* module states */
	if ((mcnx = MALLOCZ(wlc->osh, sizeof(wlc_mcnx_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	mcnx->wlc = wlc;
	mcnx->d11shm = (uint32)~0;

	/* d11 shm control block mapping */
	memset(mcnx->d11cbo, D11_CBO_AVAIL, sizeof(mcnx->d11cbo));

	/* reserve the last entry in M_ADDR_BMP_BLK for primary bsscfg */
	setbit(mcnx->d11rao, D11_RAI_PRI);

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((mcnx->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_mcnx_info_t *),
	                wlc_mcnx_info_init, wlc_mcnx_info_deinit, wlc_mcnx_info_dump,
	                mcnx)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register bsscfg up/down callback */
	if (wlc_bsscfg_updown_register(wlc, wlc_mcnx_bss_updn_cb, mcnx) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	notif = wlc->notif;
	ASSERT(notif != NULL);

	/* create notification list for assoc update. */
	if (bcm_notif_create_list(notif, &mcnx->assoc_upd_notif_hdl) != BCME_OK) {
		WL_ERROR(("wl%d: %s: assoc bcm_notif_create_list() failed\n",
		         wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* create notification list for bss update. */
	if (bcm_notif_create_list(notif, &mcnx->bss_upd_notif_hdl) != BCME_OK) {
		WL_ERROR(("wl%d: %s: bss bcm_notif_create_list() failed\n",
		         wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* create notification list for noa update. */
	if (bcm_notif_create_list(notif, &mcnx->tsf_upd_notif_hdl) != BCME_OK) {
		WL_ERROR(("wl%d: %s: tsf bcm_notif_create_list() failed\n",
		         wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* create notification list for interrupt. */
	if (bcm_notif_create_list(notif, &mcnx->intr_notif_hdl) != BCME_OK) {
		WL_ERROR(("wl%d: %s: intr bcm_notif_create_list() failed\n",
		         wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* register module up/down, watchdog, and iovar callbacks */
	if (wlc_module_register(wlc->pub, mcnx_iovars, "mcnx", mcnx, wlc_mcnx_doiovar,
	                        NULL, wlc_mcnx_up, wlc_mcnx_down) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "mcnx", wlc_mcnx_dump, (void *)mcnx);
#endif // endif

	/* ENABLE MCNX by default */
	if (wlc_mcnx_cap(mcnx) &&
	    wlc_mcnx_enab(mcnx, TRUE) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_mcnx_enab() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	if (D11REV_GE(wlc->pub->corerev, 40))
		mcnx->p2p_max_idx = AMT_MAXIDX_P2P_USE + 1;
	else
		mcnx->p2p_max_idx = RCMTA_SIZE;

	mcnx->p2p_start_idx = mcnx->p2p_max_idx - M_ADDR_BMP_BLK_SZ;

	return mcnx;

fail:
	/* error handling */
	wlc_mcnx_detach(mcnx);
	return NULL;
}

void
BCMATTACHFN(wlc_mcnx_detach)(wlc_mcnx_info_t *mcnx)
{
	wlc_info_t *wlc;

	if (mcnx == NULL)
		return;

	wlc = mcnx->wlc;

	wlc_module_unregister(wlc->pub, "mcnx", mcnx);

	if (mcnx->intr_notif_hdl != NULL)
		bcm_notif_delete_list(&mcnx->intr_notif_hdl);
	if (mcnx->tsf_upd_notif_hdl != NULL)
		bcm_notif_delete_list(&mcnx->tsf_upd_notif_hdl);
	if (mcnx->bss_upd_notif_hdl != NULL)
		bcm_notif_delete_list(&mcnx->bss_upd_notif_hdl);
	if (mcnx->assoc_upd_notif_hdl != NULL)
		bcm_notif_delete_list(&mcnx->assoc_upd_notif_hdl);

	wlc_bsscfg_updown_unregister(wlc, wlc_mcnx_bss_updn_cb, mcnx);

	MFREE(wlc->osh, mcnx, sizeof(wlc_mcnx_info_t));
}

/* feature enable/disable */
int
wlc_mcnx_enab(wlc_mcnx_info_t *mcnx, bool enable)
{
	wlc_info_t *wlc = mcnx->wlc;
	wlc_hw_info_t *hw = wlc->hw;
	int status;

	if (hw == NULL)
		return BCME_ERROR;

	if ((status = wlc_bmac_mcnx_enab(hw, enable)) != BCME_OK)
		return status;

	wlc->pub->_mcnx = enable;

	return BCME_OK;
}

/* h/w capable of multiple connection? */
bool
wlc_mcnx_cap(wlc_mcnx_info_t *mcnx)
{
	wlc_info_t *wlc = mcnx->wlc;
	wlc_hw_info_t *hw = wlc->hw;

	if (hw == NULL)
		return FALSE;

	return wlc_bmac_mcnx_cap(hw);
}

/* setup ADDR_BMP entry with proper attributes */
static void
wlc_mcnx_rcmta_type_set(wlc_mcnx_info_t *mcnx, int idx, uint16 mask, uint16 val)
{
	wlc_info_t *wlc = mcnx->wlc;
	uint16 orig_val;
	uint16 new_val;

	if (!wlc->clk)
		return;

	ASSERT((val & ~mask) == 0);
	ASSERT(idx >= mcnx->p2p_start_idx && idx < mcnx->p2p_max_idx);
	if (D11REV_GE(wlc->pub->corerev, 40)) {
		new_val = orig_val = wlc_read_amtinfo_by_idx(wlc, idx);
	} else {
		new_val = orig_val =
			wlc_mcnx_read_shm(mcnx, M_ADDR_BMP_BLK(idx - mcnx->p2p_start_idx));
	}

	new_val = (orig_val & ~mask) | val;
	if (new_val == orig_val)
		return;
	if (D11REV_GE(wlc->pub->corerev, 40)) {
		wlc_write_amtinfo_by_idx(wlc, idx, new_val);
	} else {
		/* Cannot be APSTA at the same time */
		ASSERT(((new_val & ADDR_BMP_STA) == 0 || (new_val & ADDR_BMP_AP) == 0));
		wlc_mcnx_write_shm(mcnx, M_ADDR_BMP_BLK(idx - mcnx->p2p_start_idx), new_val);
	}
}

/* handle related iovars */
static int
wlc_mcnx_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_mcnx_info_t *mcnx = (wlc_mcnx_info_t *)ctx;
	wlc_info_t *wlc = mcnx->wlc;
	int32 int_val = 0;
	int err = BCME_OK;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* all iovars require mcnx being enabled */
	switch (actionid) {
	case IOV_GVAL(IOV_MCNX):
#ifndef DONGLEBUILD
	case IOV_SVAL(IOV_MCNX):
#endif // endif
		break;
	default:
		if (MCNX_ENAB(wlc->pub))
			break;
		WL_ERRORP(("wl%d: %s: iovar %s get/set requires mcnx enabled\n",
		          wlc->pub->unit, __FUNCTION__, name));
		return BCME_ERROR;
	}
	switch (actionid) {
	case IOV_GVAL(IOV_MCNX):
		*((uint32*)a) = wlc->pub->_mcnx;
		break;
#ifndef DONGLEBUILD
	case IOV_SVAL(IOV_MCNX):
		err = wlc_mcnx_enab(mcnx, int_val != 0);
		break;
#endif // endif
#ifdef BCMDBG
	case IOV_SVAL(IOV_MCNX_DEBUG):
		mcnx->debug = (uint)int_val;
		break;
#endif // endif
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

/* wl init callback */
static int
wlc_mcnx_up(void *ctx)
{
	wlc_mcnx_info_t *mcnx = (wlc_mcnx_info_t *)ctx;
	wlc_info_t *wlc = mcnx->wlc;
	uint b, c;
	int idx;
	wlc_bsscfg_t *cfg;

	if (!MCNX_ENAB(wlc->pub))
		return BCME_OK;

	WL_TRACE(("%s\n", __FUNCTION__));
	/* SHM location */
	mcnx->d11shm = wlc_read_shm(wlc, M_P2P_BLK_PTR) << 1;

	/* clear RCMTA entries and ADDR_BMP blocks */
	for (b = (uint)mcnx->p2p_start_idx; b < (uint)mcnx->p2p_max_idx; b ++) {
		wlc_set_ra_hw(mcnx->wlc, b, &ether_null);
		wlc_mcnx_rcmta_type_set(mcnx, b, ~0, 0);
	}

	/* clear BSS control blocks */
	for (b = 0; b < M_P2P_BSS_MAX; b ++) {
		for (c = 0; c < M_P2P_BSS_BLK_SZ; c ++)
			wlc_mcnx_write_shm(mcnx, M_P2P_BSS(b, c), 0);
		wlc_mcnx_write_shm(mcnx, M_P2P_PRE_TBTT(b), 0);
	}

	/* clear interrupt blocks */
	for (b = 0; b < M_P2P_BSS_MAX; b ++) {
		for (c = 0; c < M_P2P_I_BLK_SZ; c ++)
			wlc_mcnx_write_shm(mcnx, M_P2P_I(b, c), 0);
	}

	/* clear TSF blocks */
	for (b = 0; b < M_P2P_BSS_MAX; b ++) {
		for (c = 0; c < M_P2P_TSF_BLK_SZ; c ++)
			wlc_mcnx_write_shm(mcnx, M_P2P_TSF(b, c), 0);
	}

	/* clear HPS blocks */
	wlc_mcnx_write_shm(mcnx, M_P2P_HPS, 0);

#ifdef WLTSFSYNC
	/* disable ucode TSF adjustment */
	wlc_skip_adjtsf(wlc, TRUE, NULL, WLC_SKIP_ADJTSF_MCNX, WLC_BAND_ALL);
#endif // endif

	/* dealing with big hammer reinit/reset here and
	 * reinitialize necessary h/w resources to get things going...
	 */
	FOREACH_AS_BSS(wlc, idx, cfg) {
		WL_MCNX(("wl%d.%d: %s: UP init BSS\n",
		        wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));

		if (wlc->pub->up) {
			wlc_set_mac(cfg);
			wlc_set_bssid(cfg);
		}

		/* handle Infra STA */
		if (BSSCFG_STA(cfg) && cfg->BSS &&
		    cfg->up) {
			/* set p2p assoc state since a reset would have cleared the p2p
			 * shm values. This would reprogram part of the p2p shm values.
			 * The rest of the tbtt based shm values will get reprogrammed
			 * upon first reception of beacons.
			 */
			wlc_mcnx_assoc_upd(mcnx, cfg, TRUE);
			wlc_mcnx_dtim_upd(mcnx, cfg, TRUE);

			/* Update maccontrol PM related bits */
			wlc_set_ps_ctrl(cfg);

			/* keep the chip awake until a beacon is received */
			if (cfg->pm->PMenabled) {
				wlc_set_pmawakebcn(cfg, TRUE);
				wlc_dtimnoslp_set(cfg);
			}

			/* force TSF update when a beacon is received */
			wlc_mcnx_tbtt_inv(mcnx, cfg);
		}
		/* handle AP and IBSS */
		else if (BSSCFG_AP(cfg) ||
		         (BSSCFG_IBSS(cfg) &&
		          cfg->up)) {
			wlc_mcnx_bss_upd(mcnx, cfg, TRUE);
		}
	}

	return BCME_OK;
}

static int
wlc_mcnx_down(void *ctx)
{
#ifdef WLTSFSYNC
	wlc_mcnx_info_t *mcnx = (wlc_mcnx_info_t *)ctx;
	wlc_info_t *wlc = mcnx->wlc;

	/* enable ucode TSF adjustment */
	wlc_skip_adjtsf(wlc, FALSE, NULL, WLC_SKIP_ADJTSF_MCNX, WLC_BAND_ALL);
#endif // endif

	return BCME_OK;
}

/* suspend/resume TSF, TBTT, pre-TBTT, NoA, CTWindow, etc related h/w and ucode */
void
wlc_mcnx_mac_suspend(wlc_mcnx_info_t *mcnx)
{
	wlc_info_t *wlc = mcnx->wlc;

	mcnx->mac_suspends ++;

	if (mcnx->mac_suspends > 1)
		return;
	wlc_mhf(wlc, MHF1, MHF1_P2P_SKIP_TIME_UPD, MHF1_P2P_SKIP_TIME_UPD, WLC_BAND_ALL);
}

void
wlc_mcnx_mac_resume(wlc_mcnx_info_t *mcnx)
{
	wlc_info_t *wlc = mcnx->wlc;

	mcnx->mac_suspends --;
	ASSERT(mcnx->mac_suspends >= 0);

	if (mcnx->mac_suspends > 0)
		return;
	wlc_mhf(wlc, MHF1, MHF1_P2P_SKIP_TIME_UPD, 0, WLC_BAND_ALL);
}

/* alloc/free d11 shm control block */
static int
wlc_mcnx_d11cb_alloc(wlc_mcnx_info_t *mcnx, uint8 user, uint8 *idx)
{
	uint8 i;

	for (i = 0; i < M_P2P_BSS_MAX; i ++) {
		if (mcnx->d11cbo[i] == D11_CBO_AVAIL)
			break;
	}

	if (i >= M_P2P_BSS_MAX)
		return BCME_NORESOURCE;

	mcnx->d11cbo[i] = user;
	*idx = i;

	return BCME_OK;
}

static void
wlc_mcnx_d11cb_free(wlc_mcnx_info_t *mcnx, uint8 user, uint8 idx)
{
	ASSERT(idx < M_P2P_BSS_MAX);
	ASSERT(mcnx->d11cbo[idx] == user);
	mcnx->d11cbo[idx] = D11_CBO_AVAIL;
}

int
wlc_mcnx_BSS_idx(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	return bmi->d11cbi;
}

/* alloc/free rcmta RA slot */
/* Slots in RCMTA at the bottom. See d11.h for explanation */
static int
wlc_mcnx_rcmta_alloc(wlc_mcnx_info_t *mcnx, uint8 *idx)
{
	int i;

	for (i = 0; i < M_ADDR_BMP_BLK_SZ; i ++) {
		if (isset(mcnx->d11rao, i))
			continue;
		*idx = mcnx->p2p_start_idx + i;
		setbit(mcnx->d11rao, i);
		return BCME_OK;
	}

	return BCME_NORESOURCE;
}

static void
wlc_mcnx_rcmta_free(wlc_mcnx_info_t *mcnx, uint8 idx)
{
	if (idx < mcnx->p2p_start_idx || idx >= mcnx->p2p_max_idx)
		return;
	clrbit(mcnx->d11rao, idx - mcnx->p2p_start_idx);
}

/* SHM access */
uint16
wlc_mcnx_read_shm(wlc_mcnx_info_t *mcnx, uint offset)
{
	wlc_info_t *wlc = mcnx->wlc;

	ASSERT(mcnx->d11shm != (uint32)~0);

	return wlc_read_shm(wlc, mcnx->d11shm + offset);
}

void
wlc_mcnx_write_shm(wlc_mcnx_info_t *mcnx, uint offset, uint16 value)
{
	wlc_info_t *wlc = mcnx->wlc;

	ASSERT(mcnx->d11shm != (uint32)~0);

	wlc_write_shm(wlc, mcnx->d11shm + offset, value);
}

/* estimate the next tbtt in local TSF time based on the 'ltsf' time */
/* 'ltsf' must be in local TSF time */
void
wlc_mcnx_next_l_tbtt64(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 ltsf_h, uint32 ltsf_l, uint32 *ltbtt_h, uint32 *ltbtt_l)
{
	uint32 rtbtt_h, rtbtt_l;
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	/* immediate next tbtt */
	wlc_mcnx_l2r_tsf64(mcnx, cfg, ltsf_h, ltsf_l, &rtbtt_h, &rtbtt_l);
	wlc_tsf64_to_next_tbtt64(bmi->bcn_prd, &rtbtt_h, &rtbtt_l);
	wlc_mcnx_r2l_tsf64(mcnx, cfg, rtbtt_h, rtbtt_l, ltbtt_h, ltbtt_l);
}

/* estimate the last tbtt in local TSF time based on the 'ltsf' time */
/* 'ltsf' must be in local TSF time */
void
wlc_mcnx_last_l_tbtt64(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 ltsf_h, uint32 ltsf_l, uint32 *ltbtt_h, uint32 *ltbtt_l)
{
	uint32 rtbtt_h, rtbtt_l;
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	/* immediate next tbtt */
	wlc_mcnx_l2r_tsf64(mcnx, cfg, ltsf_h, ltsf_l, &rtbtt_h, &rtbtt_l);
	wlc_tsf64_to_next_tbtt64(bmi->bcn_prd, &rtbtt_h, &rtbtt_l);
	/* last tbtt */
	wlc_uint64_sub(&rtbtt_h, &rtbtt_l, 0, bmi->bcn_prd << 10);
	wlc_mcnx_r2l_tsf64(mcnx, cfg, rtbtt_h, rtbtt_l, ltbtt_h, ltbtt_l);
}

/* update SHM HPS */
void
wlc_mcnx_hps_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, uint8 what, bool enable)
{
	wlc_info_t *wlc = mcnx->wlc;
	uint16 hps = mcnx->hps;

	if (!wlc->clk)
		return;

	if (DEVICEREMOVED(wlc))
		return;

	if (enable)
		hps |= what;
	else
		hps &= ~what;

#if defined(BCMDBG) || defined(WLMSG_PS)
	if (!hps)
		WL_PS(("wl%d.%d: PM-MODE: clear HPS (P2P no sleep and no PM)\n",
		       wlc->pub->unit, WLC_BSSCFG_IDX(cfg)));
	if (hps)
		WL_PS(("wl%d.%d: PM-MODE: set HPS 0x%x (P2P permit sleep and enable PM)\n",
		       wlc->pub->unit, WLC_BSSCFG_IDX(cfg), hps));
#endif	/* BCMDBG || WLMSG_PS */

	wlc_mcnx_write_shm(mcnx, M_P2P_HPS, hps);
	mcnx->hps = hps;
}

uint16
wlc_mcnx_hps_get(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = mcnx->wlc;
	int bss;

	ASSERT(cfg != NULL);

	if (!wlc->clk)
		return 0;

	bss = wlc_mcnx_BSS_idx(mcnx, cfg);
	ASSERT(bss < M_P2P_BSS_MAX);

	return wlc_mcnx_read_shm(mcnx, M_P2P_HPS) & (M_P2P_HPS_NOA(bss) | M_P2P_HPS_CTW(bss));
}

void
wlc_mcnx_hps_set(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool enable)
{
	int bss;

	ASSERT(cfg != NULL);

	bss = wlc_mcnx_BSS_idx(mcnx, cfg);
	ASSERT(bss < M_P2P_BSS_MAX);

	wlc_mcnx_hps_upd(mcnx, cfg, M_P2P_HPS_NOA(bss) | M_P2P_HPS_CTW(bss), enable);
}

void
wlc_mcnx_hps_force(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	bmi->flags |= MCNX_FORCED_HPS;
	wlc_mcnx_hps_set(mcnx, cfg, TRUE);
}

/* adjust the TSF and TBTT by 'adj' */
void
wlc_mcnx_tbtt_adj(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, int adj)
{
	wlc_info_t *wlc = mcnx->wlc;
	bss_mcnx_info_t *bmi;
	uint32 tsf_h, tsf_l;
	uint32 tbtt_h, tbtt_l;
	int adj_h, adj_l;

	(void)wlc;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	adj_l = adj;
	if (adj_l < 0)
		adj_h = ~0;
	else
		adj_h = 0;

	WL_MCNX(("wl%d.%d: %s: adjust 0x%x%08x (%d)\n",
	         wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__, adj_h, adj_l, adj));

	/* adjust TSF/TBTT */
	wlc_mcnx_read_tsf64(mcnx, cfg, &tbtt_h, &tbtt_l);
	wlc_tsf64_to_next_tbtt64(bmi->bcn_prd, &tbtt_h, &tbtt_l);
	wlc_uint64_sub(&tbtt_h, &tbtt_l, 0, bmi->bcn_prd << 10);
	wlc_mcnx_r2l_tsf64(mcnx, cfg, tbtt_h, tbtt_l, &tsf_h, &tsf_l);
	wlc_uint64_add(&tsf_h, &tsf_l, adj_h, adj_l);
	wlc_mcnx_tbtt_set(mcnx, cfg, tsf_h, tsf_l, tbtt_h, tbtt_l);
}

/* MI_P2P interrupt handler */
void
wlc_p2p_int_proc(wlc_info_t *wlc, uint8 *p2p_interrupts, uint32 tsf_l, uint32 tsf_h)
{
	wlc_mcnx_info_t *mcnx = wlc->mcnx;
	uint b, i;
	wlc_bsscfg_t *cfg;
	int idx;

	/* dispatch interrupt to P2P client or GO */
	for (b = 0; b < M_P2P_BSS_MAX; b ++) {

		if ((p2p_interrupts[b] == 0) ||
		    ((idx = mcnx->d11cbo[b]) == D11_CBO_AVAIL))
			continue;

		for (i = 0; i < M_P2P_I_BLK_SZ; i ++) {
			bss_mcnx_info_t *bmi;
			int err;

			/* any P2P event/interrupt? */
			if ((p2p_interrupts[b] & (1 << i)) == 0)
				continue;

			if ((cfg = wlc_bsscfg_find(wlc, idx, &err)) == NULL) {
				WL_ERROR(("wl%d: %s: failed to find bsscfg %d\n",
				          wlc->pub->unit, __FUNCTION__, idx));
				continue;
			}

			bmi = BSS_MCNX_INFO(mcnx, cfg);
			ASSERT(bmi != NULL);

			INTRCNTINC(bmi, i);

			if (WL_MCNX_INTR_ON(mcnx)) {
				uint32 rtsf_h, rtsf_l;

				wlc_mcnx_l2r_tsf64(mcnx, cfg, tsf_h, tsf_l, &rtsf_h, &rtsf_l);
				WL_MCNX_INTR(mcnx,
				     ("wl%d.%d: intr %u at tick 0x%x%08x (local) 0x%x%08x (remote) "
				      "tbtt offset %u\n",
				      wlc->pub->unit, WLC_BSSCFG_IDX(cfg), i,
				      tsf_h, tsf_l, rtsf_h, rtsf_l,
				      wlc_calc_tbtt_offset(bmi->bcn_prd, rtsf_h, rtsf_l)));
			}

			/* some of our own processing */
			if (i == M_P2P_I_PRE_TBTT) {
#ifdef UC_TBTT_WAR
				uint32 dur_h = tsf_h, dur_l = tsf_l;

				wlc_uint64_sub(&dur_h, &dur_l, bmi->pre_tbtt_h, bmi->pre_tbtt_l);
				if (wlc_uint64_lt(dur_h, dur_l, 0, 1024))
					continue;
				bmi->pre_tbtt_h = tsf_h;
				bmi->pre_tbtt_l = tsf_l;
#endif // endif
				if (BSSCFG_AP(cfg)) {
					wlc_mcnx_tbtt_cache(mcnx, cfg, tsf_h, tsf_l);
				}
			}

			/* notify registered clients of the interrupt */
			wlc_mcnx_intr_notif(mcnx, cfg, i, tsf_h, tsf_l);

			/* TODO: register a wlc_mcnx_intr_fn_t callback funtion
			 * to invoke wlc_bss_tbtt()
			 */
			if (i == M_P2P_I_PRE_TBTT) {
				wlc_bss_tbtt(cfg);
			}
		}
	}
}

/* We will be smarter about when to adopt the beacon timestamp as tbtt.
 * We need to cover 2 cases:
 *  a. tbtt_offset is mostly ok, meaning wlc_calc_tbtt_offset() returned acceptable values.
 *  b. tbtt_offset is mostly not ok, meaning wlc_calc_tbtt_offset() returned unacceptable values.
 *     (ie tbtt_offset > bcn_wait_prd-1)
 *
 * Input: bcn_offset - this is the value returned by wlc_calc_tbtt_offset().
 *	  wrxh, plcp - needed indirectly by function wlc_compute_bcntsfoff().
 *
 * Output: returns either original or adjusted bcn_offset (adjusted means adopt bcn time stamp.)
 */
#define MAX_NUM_BAD_BCN_OFFSET	15
#define MAX_P2P_BCN_OFFSET_MS	((uint32)(wlc->bcn_wait_prd - 1))

static uint32
wlc_mcnx_bcn_offset_adjust(wlc_mcnx_info_t *mcnx, uint32 bcn_offset,
                         wlc_d11rxhdr_t *wrxh, uint8 *plcp, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = mcnx->wlc;
	bss_mcnx_info_t *bmi;

	if (!(wrxh && plcp && cfg))
		return (bcn_offset);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	WL_MCNX_TSF(mcnx,
	           ("%s: wl%d.%d bcn_offset currently at %d,"
	            "bcn_wait_prd = 0x%x\n",
	            __FUNCTION__, wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
	            bcn_offset, wlc->bcn_wait_prd));

	/* PR6902 WAR: adopt an unaligned TBTT if it appears to be too far out of alignment
	 */
	/* (uint32)(wlc->bcn_wait_prd - 1) */
	if (((bcn_offset >> 10) > MAX_P2P_BCN_OFFSET_MS)) {
		bool short_preamble = ((wrxh->rxhdr.PhyRxStatus_0 & PRXS0_SHORTH) != 0);
		ratespec_t rspec = wlc_recv_compute_rspec(wrxh, plcp);
		uint32 bcn_tsf_offset = wlc_compute_bcntsfoff(wlc, rspec, short_preamble, FALSE);
		uint32 bcn_offset_diff = 0;

		WL_MCNX_TSF(mcnx,
		           ("%s: bcn_offset %d value too large, bcn_wait_prd = 0x%x\n",
		            __FUNCTION__, bcn_offset, wlc->bcn_wait_prd));

		/* We've gotten bcn with good calculated tbtt_offset, check and skip anomalies.
		 * case a.
		 */
		if (bmi->bcn_offset_ok == TRUE) {
			WL_MCNX_TSF(mcnx, ("%s: bcn_offset OK\n", __FUNCTION__));
			if (bmi->num_bad_bcn_offset++ <= MAX_NUM_BAD_BCN_OFFSET) {
				WL_MCNX_TSF(mcnx,
				           ("%s: skip this anomaly, num_bad_bcn_offset = %d\n",
				            __FUNCTION__, bmi->num_bad_bcn_offset));
			}
			else {
				WL_MCNX_TSF(mcnx,
				           ("%s: %d bad bcn_offet in a row, adopt bcn ts as tbtt, "
				            "cached bcn_offset = %d\n",
				            __FUNCTION__, bmi->num_bad_bcn_offset,
				            bcn_offset));
				bmi->bcn_offset_ok = FALSE;
				bmi->bcn_offset_cache = TRUE;
				bmi->num_bad_bcn_offset = 0;
				bmi->bcn_offset = bcn_offset;
				bcn_offset = bcn_tsf_offset;
			}
		}
		/* We've gotten bcn with bad calculated tbtt_offset, check and skip anomalies.
		 * case b.
		 */
		else {
			/* For this case, need to cache the bad calculated tbtt_offset when we
			 * adopt the bcn time_stamp as tbtt.
			 */
			WL_MCNX_TSF(mcnx, ("%s: bcn_offset NOT OK\n", __FUNCTION__));
			if (bmi->bcn_offset_cache == FALSE) {
				bmi->bcn_offset_cache = TRUE;
				bmi->bcn_offset = bcn_offset;
				WL_MCNX_TSF(mcnx,
				           ("%s: bcn_offset cache, cached bcn_offset = %d\n",
				            __FUNCTION__, bcn_offset));
				/* use bcn_offset_diff default of 0 */
			}
			else {
				uint32 comp_val;
				bcn_offset_diff = bcn_offset - bmi->bcn_offset;
				comp_val = ABS((int32)bcn_offset_diff)>>10;
				/* (uint32)(wlc->bcn_wait_prd - 1) */
				if ((comp_val > MAX_P2P_BCN_OFFSET_MS) &&
				    (bmi->num_bad_bcn_offset++ <= MAX_NUM_BAD_BCN_OFFSET)) {
					WL_MCNX_TSF(mcnx,
					           ("%s: %d in a row skip and use cached "
					            "bcn_offset, bcn_offset_diff = %d(0x%x)\n",
					            __FUNCTION__, bmi->num_bad_bcn_offset,
					            (int32)bcn_offset_diff, bcn_offset_diff));
				}
				else {
					if (comp_val > MAX_P2P_BCN_OFFSET_MS)
						WL_MCNX_TSF(mcnx,
						    ("%s: %d bad bcn_offset in a row, "
						     "bcn_offset_diff = %d, adopt bcn ts, "
						     "cached bcn_offset = %d\n",
						     __FUNCTION__, bmi->num_bad_bcn_offset,
						     bcn_offset_diff, bcn_offset));
					bmi->num_bad_bcn_offset = 0;
					bmi->bcn_offset = bcn_offset;
					bcn_offset_diff = 0;
				}

			}
			bcn_offset = bcn_tsf_offset + bcn_offset_diff;
		}
	}
	/* calcuated tbtt_offset from bcn within tolerance, just use it and reset the states */
	else {
		bmi->bcn_offset_ok = TRUE;
		bmi->num_bad_bcn_offset = 0;
		bmi->bcn_offset_cache = FALSE;
	}
	WL_MCNX_TSF(mcnx,
	           ("%s: wl%d.%d bcn_offset is now %d, bcn_wait_prd = 0x%x\n",
	            __FUNCTION__, wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
	            bcn_offset, wlc->bcn_wait_prd));

	return (bcn_offset);
}

/* Figure out the TBTTs in remote TSF and local TSF times from the beacon/prbresp
 * timestamp and beacon interval fields.
 * 'tsf' must be initialized to the local TSF time when the beacon/prbresp
 * timestamp field was timestamped.
 * 'tsf' carries out the local TSF time of the TBTT and 'bcn' carries out
 * the remote TSF time of the TBTT.
 */
void
wlc_mcnx_tbtt_calc(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_bcn_prb *bcn,
	uint32 *tsf_h, uint32 *tsf_l, uint32 *bcn_h, uint32 *bcn_l)
{
	uint32 bcn_offset;
	uint32 bcn_int = 0;

	*bcn_l = 0;
	*bcn_h = 0;

	ASSERT(bcn != NULL);

	if (bcn != NULL) {
		/* read the bcn/prb TSF value */
		*bcn_l = ltoh32_ua(&bcn->timestamp[0]);
		*bcn_h = ltoh32_ua(&bcn->timestamp[1]);
		bcn_int = ltoh16_ua(&bcn->beacon_interval);
	}
	/* bad AP beacon frame, set beacon interval to WECA maximum */
	if (bcn_int == 0)
		bcn_int = 1024;
	/* beacon offset to the TBTT */
	bcn_offset = wlc_calc_tbtt_offset(bcn_int, *bcn_h, *bcn_l);

	/* p2p specific to handle unaligned tbtt */
	bcn_offset = wlc_mcnx_bcn_offset_adjust(mcnx, bcn_offset, wrxh, plcp, cfg);

	/* remote TBTT */
	wlc_uint64_sub(bcn_h, bcn_l, 0, bcn_offset);

	/* remote TBTT in local TSF time */
	wlc_uint64_sub(tsf_h, tsf_l, 0, bcn_offset);
}

/* Figure out the TBTTs in remote TSF and local TSF times from the beacon/prbresp
 * timestamp and beacon interval fields and the rxh timestamp.
 * 'tsf' carries out the local TSF time of the TBTT and 'bcn' carries out
 * the remote TSF time of the TBTT.
 */
void
wlc_mcnx_tbtt_calc_bcn(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_bcn_prb *bcn,
	uint32 *tsf_h, uint32 *tsf_l, uint32 *bcn_h, uint32 *bcn_l)
{
	wlc_info_t *wlc = mcnx->wlc;
	ratespec_t rspec;
	uint32 hdratime;

	/* local TSF at beacon reception */
	rspec = wlc_recv_compute_rspec(wrxh, plcp);
	/* 802.11 header airtime */
	hdratime = wlc_compute_bcn_payloadtsfoff(wlc, rspec);
	/* local TSF inserted in the rxh is at RxStart which is before 802.11 header */
	wlc_read_tsf(wlc, tsf_l, tsf_h);
	wlc_recover_tsf64(wlc, wrxh, tsf_h, tsf_l);
	/* local TSF at past 802.11 header at beacon reception */
	wlc_uint64_add(tsf_h, tsf_l, 0, hdratime);

	wlc_mcnx_tbtt_calc(mcnx, cfg, wrxh, plcp, bcn, tsf_h, tsf_l, bcn_h, bcn_l);

	WL_MCNX_TSF(mcnx, ("wl%d.%d: tbtt 0x%x%08x (local) 0x%x%08x (remote) at tick 0x%x\n",
	                wlc->pub->unit, WLC_BSSCFG_IDX(cfg), *tsf_h, *tsf_l, *bcn_h, *bcn_l,
	                WL_MCNX_TS(wlc)));
}

/* calculate TSF offset (local - remote) and singal an TSF update request if necessary */
/* XXX This function only depends on the difference between rtsf and ltsf and should
 * be ultimately changed to just take an offset.  This function is really just setting the
 * BSS's TSF offset.
 */
static void
wlc_mcnx_tsf_cache(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 ltsf_h, uint32 ltsf_l, uint32 rtsf_h, uint32 rtsf_l)
{
	wlc_info_t *wlc = mcnx->wlc;
	bss_mcnx_info_t *bmi;

	(void)wlc;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	/* cache the tsf offset (local - remote) */
	bmi->tsfo_h = ltsf_h;
	bmi->tsfo_l = ltsf_l;
	wlc_uint64_sub((uint32 *)&bmi->tsfo_h, (uint32 *)&bmi->tsfo_l, rtsf_h, rtsf_l);
}

/* cache remote TBTT and corresponding local TSF time, and calculate TSF offset (local - remote) */
static void
_wlc_mcnx_tbtt_cache(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 tsf_h, uint32 tsf_l, uint32 tbtt_h, uint32 tbtt_l)
{
	wlc_info_t *wlc = mcnx->wlc;
	bss_mcnx_info_t *bmi;
	int16 drift;

	(void)wlc;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	/* cache the tbtt */
	bmi->tbtt_h = tsf_h;
	bmi->tbtt_l = tsf_l;

	/* cache the tsf offset (local - remote) */
	wlc_mcnx_tsf_cache(mcnx, cfg, tsf_h, tsf_l, tbtt_h, tbtt_l);

	/* mark the cfg->tbtt has been initialized */
	bmi->flags |= MCNX_TBTT_INFO;

	WL_MCNX_TSF(mcnx, ("wl%d.%d: tbtt 0x%x%08x (local) 0x%x%08x (remote) offset 0x%x%08x "
	        "(local - remote) at tick 0x%x\n",
	        wlc->pub->unit, WLC_BSSCFG_IDX(cfg), tsf_h, tsf_l, tbtt_h, tbtt_l,
	        bmi->tsfo_h, bmi->tsfo_l, WL_MCNX_TS(wlc)));

	/* compare the tsf offset with the last saved one to decide if we need to
	 * update the pretbtt...
	 */
	if (BSSCFG_STA(cfg)) {
		drift = (int16)bmi->tsfo_l - bmi->tsfo_base;
		if (ABS(drift) >= TSFO_MAX_DIFF) {
			WL_MCNX(("wl%d: bss %d clock drifted by %dus (%x, %x), "
			         "request to update TSF\n",
			         wlc->pub->unit, bmi->d11cbi, drift,
			         (uint16)bmi->tsfo_base, (uint16)bmi->tsfo_l));
			bmi->flags &= ~MCNX_BSS_INFO;
		}
	}
}

/* cache TBTT based on the current TSF, done at preTBTT interrupt (for AP/GO only) */
static void
wlc_mcnx_tbtt_cache(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, uint32 tsf_h, uint32 tsf_l)
{
	bss_mcnx_info_t *bmi;
	uint32 bcnint;

	ASSERT(cfg != NULL);
	ASSERT(BSSCFG_AP(cfg) || !cfg->BSS);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	bcnint = bmi->bcn_prd;

	wlc_tsf64_to_next_tbtt64(bcnint, &tsf_h, &tsf_l);
	wlc_uint64_sub(&tsf_h, &tsf_l, 0, bcnint << 10);
	_wlc_mcnx_tbtt_cache(mcnx, cfg, tsf_h, tsf_l, tsf_h, tsf_l);
}

/* update STA/Client specific stuff when a STA (re)associates to a BSS */
void
wlc_mcnx_assoc_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool assoc)
{
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);
	ASSERT(BSSCFG_STA(cfg));

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	bmi->flags = 0;

	bmi->bcn_offset_ok = FALSE;
	bmi->bcn_offset_cache = FALSE;
	bmi->num_bad_bcn_offset = 0;
	bmi->bcn_offset = 0;

	if (!assoc)
		wlc_mcnx_reset_bss(mcnx, cfg);
	else
		wlc_mcnx_st_upd(mcnx, cfg, TRUE);

	wlc_mcnx_assoc_upd_notif(mcnx, cfg, assoc);
}

/* update AP/GO or IBSS specific stuff when an BSS is brought up/down */
void
wlc_mcnx_bss_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool is_up)
{
	wlc_info_t *wlc = mcnx->wlc;

	ASSERT(cfg != NULL);
	ASSERT(BSSCFG_AP(cfg) || !cfg->BSS);

	if (!is_up)
		wlc_mcnx_reset_bss(mcnx, cfg);
	else {
		uint32 tsf_l, tsf_h;
		bss_mcnx_info_t *bmi;

		bmi = BSS_MCNX_INFO(mcnx, cfg);
		ASSERT(bmi != NULL);

		bmi->bcn_prd = cfg->current_bss->beacon_period;

		wlc_mcnx_st_upd(mcnx, cfg, TRUE);

		wlc_read_tsf(wlc, &tsf_l, &tsf_h);

		if (BSSCFG_AP(cfg)) {
			wlc_mcnx_tbtt_cache(mcnx, cfg, tsf_h, tsf_l);
			wlc_mcnx_tbtt_upd(mcnx, cfg, TRUE);
		}
		else {
			wlc_mcnx_tbtt_set(mcnx, cfg, tsf_h, tsf_l, 0, 0);
		}
		wlc_mcnx_dtim_upd(mcnx, cfg, TRUE);
	}

	wlc_mcnx_bss_upd_notif(mcnx, cfg, is_up);
}

/* enable BSS PowerSave via M_P2P_BSS_ST word */
/* XXX ucode make per-BSS PowerSave feature depending on BSS type being P2P GC.
 * To workaround this limitation the non-GC connections need to change their BSS
 * type to be GC explicitly via this API.
 */
static void
_wlc_mcnx_ps_enab(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool enab)
{
	wlc_info_t *wlc = mcnx->wlc;
	uint16 state;
	int bss;

	(void)wlc;

	ASSERT(cfg != NULL);
	ASSERT(BSSCFG_STA(cfg));
	ASSERT(!P2P_CLIENT(wlc, cfg));

	bss = wlc_mcnx_BSS_idx(mcnx, cfg);
	ASSERT(bss < M_P2P_BSS_MAX);

	state = wlc_mcnx_read_shm(mcnx, M_P2P_BSS_ST(bss));
	if (enab) {
		state &= ~M_P2P_BSS_ST_STA;
		state |= M_P2P_BSS_ST_GC;
	}
	else {
		state &= ~M_P2P_BSS_ST_GC;
		state |= M_P2P_BSS_ST_STA;
	}

	WL_MCNX(("wl%d.%d: enable bss %d PS state 0x%x at tick 0x%x\n",
	        wlc->pub->unit, WLC_BSSCFG_IDX(cfg), bss, state, WL_MCNX_TS(wlc)));

	wlc_mcnx_write_shm(mcnx, M_P2P_BSS_ST(bss), state);
}

void
wlc_mcnx_ps_enab(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool enab)
{
	wlc_info_t *wlc = mcnx->wlc;

	if (!wlc->pub->up)
		return;

	wlc_mcnx_mac_suspend(mcnx);
	_wlc_mcnx_ps_enab(mcnx, cfg, enab);
	wlc_mcnx_mac_resume(mcnx);
}

/* update BSS block states M_P2P_BSS_ST word */
static void
_wlc_mcnx_st_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool set)
{
	wlc_info_t *wlc = mcnx->wlc;
	uint16 state;
	int bss;
	uint16 go_ind_bmp;

	(void)wlc;

	ASSERT(cfg != NULL);

	state = set ?
	        (P2P_GO(wlc, cfg) ? M_P2P_BSS_ST_GO :
	         BSSCFG_AP(cfg) ? M_P2P_BSS_ST_AP :
	         P2P_CLIENT(wlc, cfg) ? M_P2P_BSS_ST_GC :
	         M_P2P_BSS_ST_STA) :
	        0;

	bss = wlc_mcnx_BSS_idx(mcnx, cfg);
	ASSERT(bss < M_P2P_BSS_MAX);

	WL_MCNX(("wl%d.%d: update bss %d state 0x%x at tick 0x%x\n",
	        wlc->pub->unit, WLC_BSSCFG_IDX(cfg), bss, state, WL_MCNX_TS(wlc)));

	wlc_mcnx_write_shm(mcnx, M_P2P_BSS_ST(bss), state);

	go_ind_bmp = wlc_mcnx_read_shm(mcnx, M_P2P_GO_IND_BMP);
	if (state == M_P2P_BSS_ST_GO) {
		go_ind_bmp |= (1 << WLC_BSSCFG_IDX(cfg));
	}
	else {
		go_ind_bmp &= ~(1 << WLC_BSSCFG_IDX(cfg));
	}
	wlc_mcnx_write_shm(mcnx, M_P2P_GO_IND_BMP, go_ind_bmp);
}

void
wlc_mcnx_st_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool set)
{
	wlc_info_t *wlc = mcnx->wlc;

	if (!wlc->pub->up)
		return;

	wlc_mcnx_mac_suspend(mcnx);
	_wlc_mcnx_st_upd(mcnx, cfg, set);
	wlc_mcnx_mac_resume(mcnx);
}

/* update BSS block tbtt scheduling info
 * M_P2P_PRE_TBTT/M_P2P_BSS_N_PRE_TBTT/M_P2P_BSS_BCN_INT words and
 * activate or deactivate the BSS block
 */
static void
_wlc_mcnx_tbtt_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool set)
{
	wlc_info_t *wlc = mcnx->wlc;
	int bss;

	(void)wlc;

	ASSERT(cfg != NULL);

	bss = wlc_mcnx_BSS_idx(mcnx, cfg);
	ASSERT(bss < M_P2P_BSS_MAX);

	if (set) {
		uint32 bcnint;
		uint32 n_pretbtt, pretbtt;
		uint32 rtsf_h, rtsf_l;
		uint32 rtbtt_h, rtbtt_l;
		uint32 ltbtt_h, ltbtt_l;
		uint32 offset;
		bss_mcnx_info_t *bmi;

		bmi = BSS_MCNX_INFO(mcnx, cfg);
		ASSERT(bmi != NULL);

		if (cfg->associated) {
			bcnint = cfg->current_bss->beacon_period;
		} else {
			bcnint = cfg->target_bss->beacon_period;
		}
		bmi->bcn_prd = (uint16)bcnint;

		/* estimate the immediate next tbtt time based on the current TSF.
		 * M_P2P_BSS_N_PRE_TBTT(bss) is in lower 32-bit h/w TSF counter time.
		 */
		/* remotel TSF time */
		wlc_mcnx_read_tsf64(mcnx, cfg, &rtsf_h, &rtsf_l);
		offset = wlc_calc_tbtt_offset(bcnint, rtsf_h, rtsf_l);
		rtbtt_h = rtsf_h;
		rtbtt_l = rtsf_l;
		wlc_uint64_sub(&rtbtt_h, &rtbtt_l, 0, offset);
		bcnint <<= 10;
		/* take the tbtt before the current TSF to take care of
		 * early NoA/CTWindow adoption; otherwise the one after.
		 * (is it an ucode requirement in order to generate NoA/CTWindow
		 * interrupt if we are in absent period or in CTWindow???).
		 */
		if (cfg->associated ||
		    BSSCFG_AP(cfg) || BSSCFG_IBSS(cfg))
			wlc_uint64_add(&rtbtt_h, &rtbtt_l, 0, bcnint);
		/* local TSF time */
		wlc_mcnx_r2l_tsf64(mcnx, cfg, rtbtt_h, rtbtt_l, &ltbtt_h, &ltbtt_l);
		/* wake up early when in PS mode */
		pretbtt = wlc_pretbtt_calc(cfg);
		n_pretbtt = ltbtt_l - pretbtt;
		if (wlc->ebd->thresh)
			n_pretbtt -= wlc->ebd->earliest_offset;

		WL_MCNX(("wl%d: update bss %d at tick 0x%08X "
		        "tbtt 0x%X%08X (local) 0x%X%08X (remote) pre-tbtt 0x%08X (local)\n",
		        wlc->pub->unit, bss, WL_MCNX_TS(wlc),
		        ltbtt_h, ltbtt_l, rtbtt_h, rtbtt_l, n_pretbtt));

		/* activate BSS control block */
		wlc_mcnx_write_shm(mcnx, M_P2P_PRE_TBTT(bss), (uint16)pretbtt);
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_N_PRE_TBTT(bss),
			(uint16)(n_pretbtt >> P2P_UCODE_TIME_SHIFT));
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_BCN_INT(bss),
			(uint16)(bcnint >> P2P_UCODE_TIME_SHIFT));
	}
	else if (!DEVICEREMOVED(wlc)) {
		WL_MCNX(("wl%d: %s: stop bss %d tbtt at tick 0x%x\n",
		        wlc->pub->unit, __FUNCTION__, bss, WL_MCNX_TS(wlc)));

		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_BCN_INT(bss), 0);
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_N_PRE_TBTT(bss), 0);
		wlc_mcnx_write_shm(mcnx, M_P2P_PRE_TBTT(bss), 0);
	}
}

void
wlc_mcnx_tbtt_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool set)
{
	wlc_info_t *wlc = mcnx->wlc;

	if (!wlc->pub->up)
		return;

	wlc_mcnx_mac_suspend(mcnx);
	_wlc_mcnx_tbtt_upd(mcnx, cfg, set);
	wlc_mcnx_mac_resume(mcnx);
}

/* update BSS block dtim info M_P2P_BSS_DTIM_PRD. */
static void
_wlc_mcnx_dtim_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool set)
{
	wlc_info_t *wlc = mcnx->wlc;
	int bss;

	(void)wlc;

	bss = wlc_mcnx_BSS_idx(mcnx, cfg);
	ASSERT(bss < M_P2P_BSS_MAX);

	if (set) {
		uint16 dtimprd;

		dtimprd = cfg->current_bss->dtim_period;
		if (dtimprd == 0) {
			dtimprd = 1;
			WL_MCNX(("wl%d: force bss %d dtim %d\n",
			         wlc->pub->unit, bss, dtimprd));
		}

		WL_MCNX(("wl%d: update bss %d at tick 0x%08X dtim %d\n",
		        wlc->pub->unit, bss, WL_MCNX_TS(wlc), dtimprd));

		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_DTIM_PRD(bss), dtimprd);
	}
	else {
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_DTIM_PRD(bss), 0);
	}
}

void
wlc_mcnx_unaligned_tbtt_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, uint32 tbtt_tsfo)
{
	wlc_info_t *wlc = mcnx->wlc;
	bss_mcnx_info_t *bmi;

	BCM_REFERENCE(wlc);

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	bmi->unaligned_tbtt_tsfo = tbtt_tsfo;
}
void
wlc_mcnx_dtim_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool set)
{
	wlc_info_t *wlc = mcnx->wlc;

	if (!wlc->pub->up)
		return;

	wlc_mcnx_mac_suspend(mcnx);
	_wlc_mcnx_dtim_upd(mcnx, cfg, set);
	wlc_mcnx_mac_resume(mcnx);
}

/* adopt the specified TBTT if needed: tsf - local TSF at TBTT;
 * tbtt - remote TSF at TBTT.
 */
static void
_wlc_mcnx_tbtt_set(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 tsf_h, uint32 tsf_l, uint32 tbtt_h, uint32 tbtt_l)
{
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	_wlc_mcnx_tbtt_cache(mcnx, cfg, tsf_h, tsf_l, tbtt_h, tbtt_l);

	if (!(bmi->flags & MCNX_BSS_INFO)) {
		_wlc_mcnx_tsf_adopt(mcnx, cfg, tsf_h, tsf_l, tbtt_h, tbtt_l);
		wlc_mcnx_tbtt_upd(mcnx, cfg, TRUE);
		wlc_mcnx_tsf_upd_notif(mcnx, cfg);
	}
}

/* force to adopt remote tbtt and corresponing local tsf */
void
wlc_mcnx_tbtt_set(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 tsf_h, uint32 tsf_l, uint32 tbtt_h, uint32 tbtt_l)
{
	wlc_info_t *wlc = mcnx->wlc;

	if (!wlc->pub->up)
		return;

	wlc_mcnx_tbtt_inv(mcnx, cfg);

	wlc_mcnx_mac_suspend(mcnx);
	_wlc_mcnx_tbtt_set(mcnx, cfg, tsf_h, tsf_l, tbtt_h, tbtt_l);
	wlc_mcnx_mac_resume(mcnx);
}

/* adopt remote tbtt based on the bcn timestamp, done at beacon reception */
void
wlc_mcnx_adopt_bcn(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	wlc_d11rxhdr_t *wrxh, uint8 *plcp, struct dot11_bcn_prb *bcn)
{
	wlc_info_t *wlc = mcnx->wlc;
	uint32 bcn_l, bcn_h;
	uint32 tsf_l, tsf_h;
	bss_mcnx_info_t *bmi;

	if (!wlc->pub->up)
		return;

	wlc_mcnx_tbtt_calc_bcn(mcnx, cfg, wrxh, plcp, bcn, &tsf_h, &tsf_l, &bcn_h, &bcn_l);

	wlc_mcnx_mac_suspend(mcnx);
	_wlc_mcnx_tbtt_set(mcnx, cfg, tsf_h, tsf_l, bcn_h, bcn_l);
	wlc_mcnx_mac_resume(mcnx);

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	BCNADCNTINC(bmi);
}

#ifdef WLTDLS
void
wlc_mcnx_adopt_bss(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, wlc_bss_info_t *bi)
{
	wlc_info_t *wlc = mcnx->wlc;
	uint32 bcn_l, bcn_h;
	uint32 tsf_l, tsf_h;

	ASSERT(cfg != NULL);
	ASSERT(bi != NULL);

	/* Avoid crash in external driver */
	if (bi->bcn_prb == NULL)
		return;

	/* recover the local tsf value from the saved low 32 bits of the local tsf */
	wlc_read_tsf(wlc, &tsf_l, &tsf_h);
	if (tsf_l < bi->rx_tsf_l)
		tsf_h -= 1;
	tsf_l = bi->rx_tsf_l;

	wlc_mcnx_tbtt_calc(mcnx, cfg, NULL, NULL, bi->bcn_prb, &tsf_h, &tsf_l, &bcn_h, &bcn_l);
	_wlc_mcnx_tbtt_set(mcnx, cfg, tsf_h, tsf_l, bcn_h, bcn_l);
}
#endif /* WLTDLS */

/* reset BSS block */
void
wlc_mcnx_reset_bss(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = mcnx->wlc;
	bss_mcnx_info_t *bmi;
	int bss;
	uint c;

	WL_TRACE(("%s\n", __FUNCTION__));

	if (!wlc->pub->up)
		return;

	if (DEVICEREMOVED(wlc))
		return;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	wlc_mcnx_hps_set(mcnx, cfg, FALSE);

	bss = bmi->d11cbi;
	ASSERT(bss < M_P2P_BSS_MAX);

	WL_MCNX(("wl%d: %s: reset bss %d at tick 0x%x\n",
	        wlc->pub->unit, __FUNCTION__, bss, WL_MCNX_TS(wlc)));

	wlc_mcnx_mac_suspend(mcnx);
	for (c = 0; c < M_P2P_BSS_BLK_SZ; c ++)
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS(bss, c), 0);
	wlc_mcnx_mac_resume(mcnx);
}

/* invalidate TSF and force an update on TSF, TBTT, DTIM, and BSS block */
void
wlc_mcnx_tbtt_inv(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	WL_MCNX(("wl%d: bss %d request to update TSF\n",
	         mcnx->wlc->pub->unit, bmi->d11cbi));

	bmi->flags &= ~MCNX_BSS_INFO;
}

/* adopt remote TSF */
/* XXX TODO: Per BSS TSF was done at a later stage hence it's different than all other
 * tsf offset values in the driver (local - remote) in that it is remote - local.
 * Change all of these tsf offset values in the driver to remote - local to make
 * them all consistent.
 */
/* XXX This function only depends on the difference between rtsf and ltsf and should
 * be ultimately changed to just take an offset.  This function is really just setting the
 * BSS's TSF offset.
 */
static void
_wlc_mcnx_tsf_adopt(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 ltsf_h, uint32 ltsf_l, uint32 rtsf_h, uint32 rtsf_l)
{
	wlc_info_t *wlc = mcnx->wlc;
	uint32 off_l, off_h;
	bss_mcnx_info_t *bmi;
	bool tsf_upd = TRUE;

#ifdef WLTSFSYNC
	if (!wlc->pub->up)
		return;
#endif // endif

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	/* tsf offset in SHM is remote - local */
	off_h = rtsf_h;
	off_l = rtsf_l;
	wlc_uint64_sub(&off_h, &off_l, ltsf_h, ltsf_l);

#ifndef WLTSFSYNC
	tsf_upd = (bmi->flags & MCNX_BSS_INFO) == 0;
#endif // endif
	if (tsf_upd) {
		int bss;

		bss = bmi->d11cbi;
		ASSERT(bss < M_P2P_BSS_MAX);

		WL_MCNX(("wl%d.%d: update bss %u TSF offset 0x%x%08x "
		        "(remote 0x%x%08x - local 0x%x%08x)\n",
		        wlc->pub->unit, WLC_BSSCFG_IDX(cfg), bss,
		        off_h, off_l, rtsf_h, rtsf_l, ltsf_h, ltsf_l));

		wlc_mcnx_write_shm(mcnx, M_P2P_TSF(bss, 3), (uint16)(off_h >> 16));
		wlc_mcnx_write_shm(mcnx, M_P2P_TSF(bss, 2), (uint16)off_h);
		wlc_mcnx_write_shm(mcnx, M_P2P_TSF(bss, 1), (uint16)(off_l >> 16));
		wlc_mcnx_write_shm(mcnx, M_P2P_TSF(bss, 0), (uint16)off_l);
	}

	/* reflect the current TSF offset */
	bmi->tsfo_base = (int16)bmi->tsfo_l;
	bmi->flags |= MCNX_BSS_INFO;

#ifndef WLTSFSYNC
	if (cfg->BSS && cfg->assoc->state == AS_WAIT_RCV_BCN) {
		wlc_skip_adjtsf(wlc, FALSE, cfg, -1, WLC_BAND_ALL);
	}
#endif /* WLTSFSYNC */
}

/* adjust TSF (TSF offset really) */
void
wlc_mcnx_tsf_adopt(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 ltsf_h, uint32 ltsf_l, uint32 rtsf_h, uint32 rtsf_l)
{
	wlc_info_t *wlc = mcnx->wlc;

	if (!wlc->pub->up)
		return;

	wlc_mcnx_tsf_cache(mcnx, cfg, ltsf_h, ltsf_l, rtsf_h, rtsf_l);

	wlc_mcnx_mac_suspend(mcnx);
	_wlc_mcnx_tsf_adopt(mcnx, cfg, ltsf_h, ltsf_l, rtsf_h, rtsf_l);
	wlc_mcnx_mac_resume(mcnx);
}

/* adjust connections' TSF and TBTT by the offset */
/* This function must be called after the h/w TSF counters are adjusted */
void
wlc_mcnx_tbtt_adj_all(wlc_mcnx_info_t *mcnx, int32 off_h, int32 off_l)
{
	wlc_info_t *wlc = mcnx->wlc;
	int idx;
	wlc_bsscfg_t *cfg;
	uint32 ltbtt_l, ltbtt_h;
	uint32 rtbtt_l, rtbtt_h;

	WL_MCNX(("wl%d: %s: offset 0x%x%08x\n",
	         wlc->pub->unit, __FUNCTION__, off_h, off_l));

	/* update all BSSs' tsf offset/pre tbtt */
	FOREACH_AS_BSS(wlc, idx, cfg) {
		bss_mcnx_info_t *bmi;
		bool upd;

		bmi = BSS_MCNX_INFO(mcnx, cfg);
		ASSERT(bmi != NULL);

		/* TBTT info hasn't been plumbed in this bsscfg */
		if (!(bmi->flags & MCNX_TBTT_INFO))
			continue;

		/* Note: the h/w TSF counters have been adjusted by the offset 'off'
		 * before here...
		 */

		/* update TSF offset for this kind of connection... */
		upd = BSSCFG_STA(cfg) && (cfg->BSS || cfg != wlc->cfg);

		/* remote TSF without connection's TSF offset adjusted */
		/* i.e., the connection's TSF is before it should have been
		 * when the h/w TSF counters have been adjusted down (negative offset)
		 */
		wlc_mcnx_read_tsf64(mcnx, cfg, &rtbtt_h, &rtbtt_l);
		/* remote TSF with the TSF offset adjusted */
		if (upd)
			wlc_uint64_sub(&rtbtt_h, &rtbtt_l, off_h, off_l);
		/* next TBTT in remote TSF with TSF offset adjusted */
		wlc_tsf64_to_next_tbtt64(bmi->bcn_prd, &rtbtt_h, &rtbtt_l);
		/* next TBTT in local TSF without TSF offset adjusted */
		/* i.e., the local TBTT is after it should have been
		 * when the h/w TSF counters have been adjusted down (negative offset)
		 */
		wlc_mcnx_r2l_tsf64(mcnx, cfg, rtbtt_h, rtbtt_l, &ltbtt_h, &ltbtt_l);
		WL_MCNX(("wl%d.%d: adjust TBTT 0x%x%08x (local) 0x%x%08x (remote) "
		         "by offset 0x%x%08x\n", wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
		         ltbtt_h, ltbtt_l, rtbtt_h, rtbtt_l, off_h, off_l));
		/* next TBTT in local TSF with TSF offset adjusted */
		if (upd)
			wlc_uint64_add(&ltbtt_h, &ltbtt_l, off_h, off_l);
		WL_MCNX(("wl%d.%d: new TBTT 0x%x%08x (local) 0x%x%08x (remote)\n",
		         wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
		         ltbtt_h, ltbtt_l, rtbtt_h, rtbtt_l));

		/* use the above extrapolated local and remote tbtt times */
		wlc_mcnx_tbtt_set(mcnx, cfg, ltbtt_h, ltbtt_l, rtbtt_h, rtbtt_l);
	}
}

/* read the BSS's TSF i.e. local TSF for AP or remote TSF for STA */
/* XXX Is it ok when tsf offset is uninitialized i.e. before STA receives
 * the first beacon? Hopefully no one uses the read TSF that early...
 */
void
wlc_mcnx_read_tsf64(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, uint32 *tsf_h, uint32 *tsf_l)
{
	wlc_info_t *wlc = mcnx->wlc;
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	wlc_read_tsf(wlc, tsf_l, tsf_h);
	wlc_uint64_sub(tsf_h, tsf_l, bmi->tsfo_h, bmi->tsfo_l);
}

/* For P2P, need to setup cfprep and cfpstart registers.
 * These two registers are not setup during association because
 * p2p uses pretbtt shm blocks to keep track of time.
 */
void
wlc_mcnx_wowl_setup(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = mcnx->wlc;
	uint32 bcn_period;
	uint32 ltsf_h, ltsf_l;
	uint32 rtsf_h, rtsf_l;
	uint32 rtbtt_h, rtbtt_l;

	/* Only need to do this if we're in infrastructure mode */
	if (!cfg->BSS)
		return;

	bcn_period = cfg->current_bss->beacon_period;

	/* XXX: currently, wowl ucode depends on cfprep and cfpstart registers to track tbtt.
	 * (cfprep => bcn_period, cfpstart => tbtt)
	 * These registers don't need to reflect the actual remote tsf time value of the AP.
	 * They can be based on the local tsf time value, as long as the local TBTT and remote
	 * TBTT happen at the same time with the same interval.
	 */

	/* Read the current tsf timer value */
	wlc_read_tsf(wlc, &ltsf_l, &ltsf_h);

	/* get sta's next tbtt */
	wlc_mcnx_l2r_tsf64(mcnx, cfg, ltsf_h, ltsf_l, &rtsf_h, &rtsf_l);
	rtbtt_h = rtsf_h;
	rtbtt_l = rtsf_l;
	wlc_tsf64_to_next_tbtt64(bcn_period, &rtbtt_h, &rtbtt_l);

	wlc_tsf_adj(wlc, cfg, rtsf_h, rtsf_l, ltsf_h, ltsf_l,
	            rtbtt_l + (bcn_period << 10), (bcn_period << 10), FALSE);
}

/* These functions register/unregister a callback that wlc_mcnx_assoc_upd may invoke. */
int
BCMATTACHFN(wlc_mcnx_assoc_upd_register)(wlc_mcnx_info_t *mcnx, wlc_mcnx_assoc_upd_fn_t cb,
            void *arg)
{
	bcm_notif_h hdl = mcnx->assoc_upd_notif_hdl;
	return bcm_notif_add_interest(hdl, (bcm_notif_client_callback)cb, arg);
}

int
BCMATTACHFN(wlc_mcnx_assoc_upd_unregister)(wlc_mcnx_info_t *mcnx, wlc_mcnx_assoc_upd_fn_t cb,
            void *arg)
{
	bcm_notif_h hdl = mcnx->assoc_upd_notif_hdl;
	return bcm_notif_remove_interest(hdl, (bcm_notif_client_callback)cb, arg);
}

static void
wlc_mcnx_assoc_upd_notif(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool assoc)
{
	wlc_mcnx_assoc_upd_data_t notif_data;
	bcm_notif_h hdl = mcnx->assoc_upd_notif_hdl;

	notif_data.cfg = cfg;
	notif_data.assoc = assoc;
	bcm_notif_signal(hdl, &notif_data);
}

/* These functions register/unregister a callback that wlc_mcnx_bss_upd may invoke. */
int
BCMATTACHFN(wlc_mcnx_bss_upd_register)(wlc_mcnx_info_t *mcnx, wlc_mcnx_bss_upd_fn_t cb, void *arg)
{
	bcm_notif_h hdl = mcnx->bss_upd_notif_hdl;
	return bcm_notif_add_interest(hdl, (bcm_notif_client_callback)cb, arg);
}

int
BCMATTACHFN(wlc_mcnx_bss_upd_unregister)(wlc_mcnx_info_t *mcnx, wlc_mcnx_bss_upd_fn_t cb, void *arg)
{
	bcm_notif_h hdl = mcnx->bss_upd_notif_hdl;
	return bcm_notif_remove_interest(hdl, (bcm_notif_client_callback)cb, arg);
}

static void
wlc_mcnx_bss_upd_notif(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool is_up)
{
	wlc_mcnx_bss_upd_data_t notif_data;
	bcm_notif_h hdl = mcnx->bss_upd_notif_hdl;

	notif_data.cfg = cfg;
	notif_data.up = is_up;
	bcm_notif_signal(hdl, &notif_data);
}

/* These functions register/unregister a callback that wlc_mcnx_tsf_upd may invoke. */
int
BCMATTACHFN(wlc_mcnx_tsf_upd_register)(wlc_mcnx_info_t *mcnx, wlc_mcnx_tsf_upd_fn_t cb, void *arg)
{
	bcm_notif_h hdl = mcnx->tsf_upd_notif_hdl;
	return bcm_notif_add_interest(hdl, (bcm_notif_client_callback)cb, arg);
}

int
BCMATTACHFN(wlc_mcnx_tsf_upd_unregister)(wlc_mcnx_info_t *mcnx, wlc_mcnx_tsf_upd_fn_t cb, void *arg)
{
	bcm_notif_h hdl = mcnx->tsf_upd_notif_hdl;
	return bcm_notif_remove_interest(hdl, (bcm_notif_client_callback)cb, arg);
}

static void
wlc_mcnx_tsf_upd_notif(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	wlc_mcnx_tsf_upd_data_t notif_data;
	bcm_notif_h hdl = mcnx->tsf_upd_notif_hdl;

	notif_data.cfg = cfg;
	bcm_notif_signal(hdl, &notif_data);
}

/* These functions register/unregister a callback that wlc_p2p_int_proc may invoke. */
int
wlc_mcnx_intr_register(wlc_mcnx_info_t *mcnx, wlc_mcnx_intr_fn_t cb, void *arg)
{
	bcm_notif_h hdl = mcnx->intr_notif_hdl;
	return bcm_notif_add_interest(hdl, (bcm_notif_client_callback)cb, arg);
}

int
wlc_mcnx_intr_unregister(wlc_mcnx_info_t *mcnx, wlc_mcnx_intr_fn_t cb, void *arg)
{
	bcm_notif_h hdl = mcnx->intr_notif_hdl;
	return bcm_notif_remove_interest(hdl, (bcm_notif_client_callback)cb, arg);
}

static void
wlc_mcnx_intr_notif(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, int intr,
	uint32 tsf_h, uint32 tsf_l)
{
	wlc_mcnx_intr_data_t notif_data;
	bcm_notif_h hdl = mcnx->intr_notif_hdl;

	notif_data.cfg = cfg;
	notif_data.intr = intr;
	notif_data.tsf_h = tsf_h;
	notif_data.tsf_l = tsf_l;
	bcm_notif_signal(hdl, &notif_data);
}

/* bsscfg cubby */
static int
wlc_mcnx_info_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_mcnx_info_t *mcnx = (wlc_mcnx_info_t *)ctx;
	wlc_info_t *wlc = mcnx->wlc;
	bss_mcnx_info_t **pbmi = MCNX_BSSCFG_CUBBY_LOC(mcnx, cfg);
	bss_mcnx_info_t *bmi = NULL;
	int err;

	/* early bail out if it's not configured */
	if (!MCNX_ENAB(wlc->pub))
		goto done;

	/* allocate memory and point bsscfg cubby to it */
	if ((bmi = MALLOCZ(wlc->osh, sizeof(bss_mcnx_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		err = BCME_NOMEM;
		goto fail;
	}
	*pbmi = bmi;

	/* GO/Client/AP/STA/Device all need an BSS entry. */
	if ((err = wlc_mcnx_d11cb_alloc(mcnx, (uint8)WLC_BSSCFG_IDX(cfg),
	                                &bmi->d11cbi)) != BCME_OK) {
		WL_ERROR(("wl%d: %s: failed to alloc d11 shm control block\n",
		          wlc->pub->unit, __FUNCTION__));

		/* Don't call deinit() yet since it assumes valid rcmta idx'es */
		*pbmi = NULL;
		MFREE(wlc->osh, bmi, sizeof(bss_mcnx_info_t));
		return BCME_NORESOURCE;
	}

	/* by default it doesn't require RCMTA entries */
	bmi->rcmta_ra_idx = (uint8)wlc->pub->max_addrma_idx;
	bmi->rcmta_bssid_idx = (uint8)wlc->pub->max_addrma_idx;
	bmi->bcn_prd = cfg->current_bss->beacon_period;

	/* primary STA in APSTA mode uses the RXE for RA and BSSID */
	if (cfg == wlc_bsscfg_primary(wlc)) {
		/* primary bsscfg must get BSS 0 */
		ASSERT(bmi->d11cbi == D11_CBI_PRI);
		goto done;
	}

	/* assign an RCMTA entry as the RA by default, and in cases
	 * such as the RA isn't needed i.e. the primary bsscfg whose RA
	 * is the same as the RXE MAC address the RCMTA entry will be
	 * programmed with all 0s'.
	 */
	if ((err = wlc_mcnx_rcmta_alloc(mcnx, &bmi->rcmta_ra_idx)) != BCME_OK) {
		WL_ERROR(("wl%d.%d: %s: failed to alloc RCMTA entry for RA\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		goto fail;
	}

	/* GO/AP al use the RA as the BSSID hence the same RCMTA entry */
	if (BSSCFG_AP(cfg) ||
#if defined(NDIS) && (NDISVER >= 0x0630)
		/* XXX - for P2P device interface rcmta entry for BSSID/TX is
		 * really not needed and causes GO fail to ack receiving ucast
		 * frame when GC up first in Win8
		 */
		P2P_DEV(wlc, cfg) ||
#endif /* NDIS && (NDISVER >= 0x0630) */
		FALSE) {
		bmi->rcmta_bssid_idx = bmi->rcmta_ra_idx;
	}

#if defined(NDIS) && (NDISVER >= 0x0630)
	if (!P2P_DEV(wlc, cfg))
#endif /* NDIS && (NDISVER >= 0x0630) */
	/* STA/Client all need an BSSID entry */
	if (BSSCFG_STA(cfg) &&
	    (err = wlc_mcnx_rcmta_alloc(mcnx, &bmi->rcmta_bssid_idx)) != BCME_OK) {
		WL_ERROR(("wl%d.%d: %s: failed to alloc RCMTA entry for BSSID\n",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		goto fail;
	}

done:
	if (bmi) {
		WL_MCNX(("wl%d.%d : d11cbi=%d,rcmtai=%d,bssididx=%d\n",
			wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
			bmi->d11cbi, bmi->rcmta_ra_idx, bmi->rcmta_bssid_idx));
	}
	return BCME_OK;

fail:
	wlc_mcnx_info_deinit(ctx, cfg);
	return err;
}

static void
wlc_mcnx_info_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_mcnx_info_t *mcnx = (wlc_mcnx_info_t *)ctx;
	wlc_info_t *wlc = mcnx->wlc;
	bss_mcnx_info_t **pbmi = MCNX_BSSCFG_CUBBY_LOC(mcnx, cfg);
	bss_mcnx_info_t *bmi = *pbmi;

	if (bmi == NULL)
		return;

	if (bmi->rcmta_bssid_idx < mcnx->p2p_max_idx &&
	    bmi->rcmta_bssid_idx != bmi->rcmta_ra_idx) {
		wlc_mcnx_rcmta_free(mcnx, bmi->rcmta_bssid_idx);
	}
	if (bmi->rcmta_ra_idx < mcnx->p2p_max_idx) {
		wlc_mcnx_rcmta_free(mcnx, bmi->rcmta_ra_idx);
	}
	if (bmi->d11cbi < M_P2P_BSS_MAX) {
		wlc_mcnx_d11cb_free(mcnx, WLC_BSSCFG_IDX(cfg), bmi->d11cbi);
	}

	MFREE(wlc->osh, bmi, sizeof(bss_mcnx_info_t));
	*pbmi = NULL;
}

/* bsscfg up/down */
static void
wlc_mcnx_bss_updn_cb(void *ctx, bsscfg_up_down_event_data_t *notif_data)
{
	wlc_mcnx_info_t *mcnx = (wlc_mcnx_info_t *)ctx;
	wlc_bsscfg_t *cfg;

	if (!MCNX_ENAB(mcnx->wlc->pub))
		return;

	ASSERT(notif_data != NULL);

	cfg = notif_data->bsscfg;
	ASSERT(cfg != NULL);

	if (!notif_data->up) {
		bss_mcnx_info_t *bmi;

		bmi = BSS_MCNX_INFO(mcnx, cfg);
		ASSERT(bmi != NULL);

		if (bmi->rcmta_bssid_idx < mcnx->p2p_max_idx &&
		    bmi->rcmta_bssid_idx != bmi->rcmta_ra_idx) {
			wlc_mcnx_bssid_unset(mcnx, cfg);
		}
		if (bmi->rcmta_ra_idx < mcnx->p2p_max_idx) {
			wlc_mcnx_ra_unset(mcnx, cfg);
		}
		if (bmi->d11cbi < M_P2P_BSS_MAX) {
			wlc_mcnx_reset_bss(mcnx, cfg);
		}
	}
}

/* set/unset BSSID in RCMTA and BSSID attributes in ADDR_BMP */
int
wlc_mcnx_bssid_set(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	int idx;
	int bss;
	uint16 mask, val;

	if (D11REV_GE(mcnx->wlc->pub->corerev, 40)) {
		mask = AMTINFO_BMP_BSSID;
		val = AMTINFO_BMP_BSSID;
	} else {
		mask = ADDR_BMP_BSSID;
		val = ADDR_BMP_BSSID;
	}

	ASSERT(cfg != NULL);

	idx = wlc_mcnx_rcmta_bssid_idx(mcnx, cfg);

	/* STA/GC need A2 bit, AP/GO does not */
	wlc_set_bssid_hw(mcnx->wlc, idx, &cfg->BSSID, BSSCFG_STA(cfg));

	bss = wlc_mcnx_BSS_idx(mcnx, cfg);
	ASSERT(bss < M_P2P_BSS_MAX);

	if (BSSCFG_STA(cfg)) {
		if (D11REV_LT(mcnx->wlc->pub->corerev, 40)) {
			/* N.B. security key lookup */
			mask |= ADDR_BMP_TA;
			val |= ADDR_BMP_TA;
		}
		/* let wlc_mcnx_ra_set/unset to handle ADDR_BMP_BSS_IDX for AP/GO */
		mask |= ADDR_BMP_BSS_IDX_MASK;
		val |= (uint16)bss << ADDR_BMP_BSS_IDX_SHIFT;
	}
	if (D11REV_GE(mcnx->wlc->pub->corerev, 40) &&
		!ether_cmp(&cfg->BSSID, &cfg->cur_etheraddr)) {
		/* For corerev >= 40, set the flag only if this is not my address
		 * (and hence BSSID ONLY).
		 */
		val &= ~AMTINFO_BMP_BSSID;
	}

	wlc_mcnx_rcmta_type_set(mcnx, idx, mask, val);

	return BCME_OK;
}

void
wlc_mcnx_bssid_unset(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	int idx;
	uint16 mask;

	if (D11REV_GE(mcnx->wlc->pub->corerev, 40)) {
		mask = AMTINFO_BMP_BSSID;
	} else {
		mask = ADDR_BMP_BSSID;
	}

	idx = wlc_mcnx_rcmta_bssid_idx(mcnx, cfg);

	wlc_set_bssid_hw(mcnx->wlc, idx, &ether_null, FALSE);
	wlc_mcnx_rcmta_type_set(mcnx, idx, mask, 0);
}

/* set/unset RA in RCMTA and RA attributes in ADDR_BMP */
int
wlc_mcnx_ra_set(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	wlc_info_t *wlc = mcnx->wlc;
	int idx;
	int bss;
	uint16 mask = 0;
	uint16 val = 0;

	ASSERT(cfg != NULL);

	WL_TRACE(("%s\n", __FUNCTION__));
	idx = wlc_mcnx_rcmta_ra_idx(mcnx, cfg);
	ASSERT(idx < mcnx->p2p_max_idx && idx >= mcnx->p2p_start_idx);

	wlc_set_ra_hw(wlc, idx, &cfg->cur_etheraddr);

	if (D11REV_LT(mcnx->wlc->pub->corerev, 40)) {
		mask = ADDR_BMP_RA;
		val = ADDR_BMP_RA;
		if (P2P_DEV(wlc, cfg)) {
			mask |= ADDR_BMP_P2P_DISC;
			val |= ADDR_BMP_P2P_DISC;
		}
		else if (P2P_GO(wlc, cfg)) {
			mask |= ADDR_BMP_P2P_GO|ADDR_BMP_P2P_GC;
			val |= ADDR_BMP_P2P_GO;
		}
		else if  (P2P_CLIENT(wlc, cfg)) {
			mask |= ADDR_BMP_P2P_GC|ADDR_BMP_P2P_GO;
			val |= ADDR_BMP_P2P_GC;
		}
		else if (BSSCFG_AP(cfg)) {
			mask |= ADDR_BMP_AP|ADDR_BMP_STA;
			val |= ADDR_BMP_AP;
		} else {
			mask |= ADDR_BMP_STA|ADDR_BMP_AP;
			val |= ADDR_BMP_STA;
		}
	}

	bss = wlc_mcnx_BSS_idx(mcnx, cfg);
	ASSERT(bss < M_P2P_BSS_MAX);

	mask |= ADDR_BMP_BSS_IDX_MASK;
	val |= (uint16)bss << ADDR_BMP_BSS_IDX_SHIFT;

	wlc_mcnx_rcmta_type_set(mcnx, idx, mask, val);

	return BCME_OK;
}

void
wlc_mcnx_ra_unset(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	int idx;

	WL_TRACE(("%s\n", __FUNCTION__));

	idx = wlc_mcnx_rcmta_ra_idx(mcnx, cfg);
	ASSERT(idx < mcnx->p2p_max_idx && idx >= mcnx->p2p_start_idx);

	wlc_set_ra_hw(mcnx->wlc, idx, &ether_null);
	wlc_mcnx_rcmta_type_set(mcnx, idx, ~0, 0);
}

/* accessors */
int
wlc_mcnx_rcmta_bssid_idx(wlc_mcnx_info_t *mcnx, const wlc_bsscfg_t *cfg)
{
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	return bmi->rcmta_bssid_idx;
}

int
wlc_mcnx_rcmta_ra_idx(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	return bmi->rcmta_ra_idx;
}

bool
wlc_mcnx_tbtt_valid(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	return (bmi->flags & MCNX_TBTT_INFO) ? TRUE : FALSE;
}

bool
wlc_mcnx_hps_forced(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg)
{
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	if (bmi == NULL)
		return FALSE;

	return (bmi->flags & MCNX_FORCED_HPS) ? TRUE : FALSE;
}

/* local/remote time conversions */
uint32
wlc_mcnx_r2l_tsf32(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, uint32 rtsf)
{
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	return bmi->tsfo_l + rtsf;
}

uint32
wlc_mcnx_l2r_tsf32(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, uint32 ltsf)
{
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	return ltsf - bmi->tsfo_l;
}

void
wlc_mcnx_r2l_tsf64(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 rtsf_h, uint32 rtsf_l, uint32 *ltsf_h, uint32 *ltsf_l)
{
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	*ltsf_l = rtsf_l;
	*ltsf_h = rtsf_h;
	wlc_uint64_add(ltsf_h, ltsf_l, bmi->tsfo_h, bmi->tsfo_l);
}

void
wlc_mcnx_l2r_tsf64(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg,
	uint32 ltsf_h, uint32 ltsf_l, uint32 *rtsf_h, uint32 *rtsf_l)
{
	bss_mcnx_info_t *bmi;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	*rtsf_l = ltsf_l;
	*rtsf_h = ltsf_h;
	wlc_uint64_sub(rtsf_h, rtsf_l, bmi->tsfo_h, bmi->tsfo_l);
}

/* per bss tbtt conversion function from (16+P2P_UCODE_TIME_SHIFT) bits to 32 or 64 bits */
void
wlc_tbtt_ucode_to_tbtt32(uint32 tsf_l, uint32 *tbtt_l)
{
	uint32 tbtt_adj;

	if (*tbtt_l & ~((1 << (16+P2P_UCODE_TIME_SHIFT)) - 1)) {
		/* make sure tbtt_l passed in is not more than (16+P2P_UCODE_TIME_SHIFT) bits */
		return;
	}

	/* tbtt should be > tsf_l and
	 * difference between tbtt and tsf_l should be < bcn_period
	 */
	/* convert tbtt from (16+P2P_UCODE_TIME_SHIFT) bits to 32 bits */
	tbtt_adj = tsf_l & ((1 << (16+P2P_UCODE_TIME_SHIFT)) - 1);
	/* compare low (16+P2P_UCODE_TIME_SHIFT) bits of tsf_l and tbtt_l */
	if (tbtt_adj >= *tbtt_l) {
		/* add one to the upper bits */
		tbtt_adj = (1 << (16+P2P_UCODE_TIME_SHIFT));
	}
	else {
		tbtt_adj = 0;
	}
	*tbtt_l |= ((tsf_l & ~((1 << (16+P2P_UCODE_TIME_SHIFT)) - 1)) + tbtt_adj);
}

void
wlc_tbtt_ucode_to_tbtt64(uint32 tsf_h, uint32 tsf_l, uint32 *tbtt_h, uint32 *tbtt_l)
{
	*tbtt_h = tsf_h;
	wlc_tbtt_ucode_to_tbtt32(tsf_l, tbtt_l);

	if (*tbtt_l < tsf_l) {
		(*tbtt_h)++;
	}
}

/* active window/awake period/CT window enable/disable.
 * it will take effect in the next tbtt period if we are asssociated...
 */
static void
_wlc_mcnx_ctw_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool enab, uint32 ctw)
{
	wlc_info_t *wlc = mcnx->wlc;
	bss_mcnx_info_t *bmi;
	int bss;

	(void)wlc;

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	ASSERT(bmi != NULL);

	bss = bmi->d11cbi;
	ASSERT(bss < M_P2P_BSS_MAX);

	if (enab) {
		uint32 bcnint = bmi->bcn_prd;
		uint32 rtsf_h, rtsf_l;
		uint32 rtbtt_h, rtbtt_l;
		uint32 ltbtt_h, ltbtt_l;
		uint32 offset;
		uint16 st;

		/* need to manually set the state in ordet to make CTWindow
		 * early adoption to work...
		 */
		st = wlc_mcnx_read_shm(mcnx, M_P2P_BSS_ST(bss));
		st &= ~M_P2P_BSS_ST_CTW;

		/* estimate the immediate next tbtt time based on the current TSF.
		 * M_P2P_BSS_CTW_END(bss) is in lower 32-bit h/w TSF counter time.
		 */
		/* remote TSF time */
		wlc_mcnx_read_tsf64(mcnx, cfg, &rtsf_h, &rtsf_l);
		offset = wlc_calc_tbtt_offset(bcnint, rtsf_h, rtsf_l);
		rtbtt_h = rtsf_h;
		rtbtt_l = rtsf_l;
		wlc_uint64_sub(&rtbtt_h, &rtbtt_l, 0, offset);
		bcnint <<= 10;
		/* take the tbtt before the current TSF to take care of
		 * early CTWindow adoption; otherwise the one after.
		 */
		if (cfg->associated ||
		    BSSCFG_AP(cfg) || BSSCFG_IBSS(cfg)) {
			wlc_uint64_add(&rtbtt_h, &rtbtt_l, 0, bcnint);
			offset = bcnint - offset;
		}
		/* local TSF time */
		wlc_mcnx_r2l_tsf64(mcnx, cfg, rtbtt_h, rtbtt_l, &ltbtt_h, &ltbtt_l);

		/* update ST here to allow ucode to behave correctly
		 * in current tbtt period...
		 */
		if (!cfg->associated) {
			/* we are in CTWindow */
			if (offset >> P2P_UCODE_TIME_SHIFT < ctw >> P2P_UCODE_TIME_SHIFT) {
				/* race condition??? */
				if (wlc_mcnx_read_shm(mcnx, M_P2P_BSS_CTW(bss)) == 0) {
					uint16 ctwend = (uint16)((ltbtt_l + ctw) >>
						P2P_UCODE_TIME_SHIFT);

					WL_MCNX(("wl%d: update bss %d at tick 0x%x CTWend %u\n",
					         wlc->pub->unit, bss, WL_MCNX_TS(wlc),
					         ltbtt_l + ctw));

					wlc_mcnx_write_shm(mcnx, M_P2P_BSS_N_CTW_END(bss), ctwend);
				}
				st |= M_P2P_BSS_ST_CTW;
			}
			/* we are outside of CTWindow */
			else if (wlc_mcnx_hps_get(mcnx, cfg) ==
			         (M_P2P_HPS_NOA(bss) | M_P2P_HPS_CTW(bss))) {
				st |= M_P2P_BSS_ST_SUPR;
			}
		}

		WL_MCNX(("wl%d: update bss %d at tick 0x%x CTWindow %u state 0x%x\n",
		         wlc->pub->unit, bss, WL_MCNX_TS(wlc), ctw, st));

		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_CTW(bss), (uint16)(ctw >> P2P_UCODE_TIME_SHIFT));

		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_ST(bss), st);
	}
	else {
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_CTW(bss), 0);
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_N_CTW_END(bss), 0);
	}
}

void
wlc_mcnx_ctw_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool enab, uint32 ctw)
{
	wlc_info_t *wlc = mcnx->wlc;

	if (!wlc->pub->up)
		return;

	wlc_mcnx_mac_suspend(mcnx);
	_wlc_mcnx_ctw_upd(mcnx, cfg, enab, ctw);
	wlc_mcnx_mac_resume(mcnx);
}

/* program the h/w with NoA schedule specified */
/* sched->start is in local TSF time */
static void
_wlc_mcnx_abs_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool enab, wlc_mcnx_abs_t *sched)
{
	wlc_info_t *wlc = mcnx->wlc;
	int bss;
	uint16 st;

	(void)wlc;

	ASSERT(cfg != NULL);

	bss = wlc_mcnx_BSS_idx(mcnx, cfg);
	ASSERT(bss < M_P2P_BSS_MAX);

	/* need to manually set the state in order to make NoA
	 * early adoption to work...
	 */
	st = wlc_mcnx_read_shm(mcnx, M_P2P_BSS_ST(bss));
	st &= ~M_P2P_BSS_ST_ABS;
	wlc_mcnx_write_shm(mcnx, M_P2P_BSS_ST(bss), st);

	if (enab) {
		uint32 start = sched->start;
		uint32 interval = sched->interval;
		uint32 duration = sched->duration;
		uint32 count = sched->count;
		uint32 presence;
		uint32 remain;

		presence = interval - ((duration >> P2P_UCODE_TIME_SHIFT) << P2P_UCODE_TIME_SHIFT);
		remain = interval - (((duration >> P2P_UCODE_TIME_SHIFT) +
			(presence >> P2P_UCODE_TIME_SHIFT)) << P2P_UCODE_TIME_SHIFT);

		WL_MCNX(("wl%d: update bss %d at tick 0x%08X absence 0x%08X (remote) "
		         "0x%x (local) dur %u td %u rem %u cnt %u st 0x%x\n",
		         wlc->pub->unit, bss, WL_MCNX_TS(wlc), wlc_mcnx_l2r_tsf32(mcnx, cfg, start),
		         start >> P2P_UCODE_TIME_SHIFT << P2P_UCODE_TIME_SHIFT,
		         duration >> P2P_UCODE_TIME_SHIFT << P2P_UCODE_TIME_SHIFT,
		         presence >> P2P_UCODE_TIME_SHIFT << P2P_UCODE_TIME_SHIFT,
		         remain, count, st));

		ASSERT(remain < (1 << P2P_UCODE_TIME_SHIFT));

		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_N_NOA(bss),
			(uint16)(start >> P2P_UCODE_TIME_SHIFT));
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_NOA_DUR(bss),
			(uint16)(duration >> P2P_UCODE_TIME_SHIFT));
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_NOA_TD(bss),
			(uint16)(presence >> P2P_UCODE_TIME_SHIFT));
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_NOA_OFS(bss), (uint16)remain);
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_NOA_CNT(bss), (uint16)count);
	}
	else {

		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_NOA_CNT(bss), 0);
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_N_NOA(bss), 0);
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_NOA_DUR(bss), 0);
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_NOA_TD(bss), 0);
		wlc_mcnx_write_shm(mcnx, M_P2P_BSS_NOA_OFS(bss), 0);
	}
}

void
wlc_mcnx_abs_upd(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, bool enab, wlc_mcnx_abs_t *sched)
{
	wlc_info_t *wlc = mcnx->wlc;

	if (!wlc->pub->up)
		return;

	wlc_mcnx_mac_suspend(mcnx);
	_wlc_mcnx_abs_upd(mcnx, cfg, enab, sched);
	wlc_mcnx_mac_resume(mcnx);
}

static int
wlc_set_ra_hw(wlc_info_t *wlc, int idx, const struct ether_addr *ea)
{
	uint16 attr = AMT_ATTR_VALID|AMT_ATTR_A1;

	ASSERT(ea != NULL);

	if (!wlc->clk)
		return BCME_NOCLK;

	if (ETHER_ISNULLADDR(ea))
		attr = 0;

	if (idx == PRIMARY_INTF) {
		idx = WLC_ADDRMATCH_IDX_MAC;
	} else {
		/* In APSTA and multiSTA where multiple connections share the same MAC address
		   which is already in the RXE.  The RCMTA doesn't allow dup entries with RXE
		   so swap in null_addr if it matches our primary interface.
		   XX Not sure how AMT works with dups yet, so treating AMT the same for now.
		   SKipping this for AMT
		*/
		if (D11REV_LT(wlc->pub->corerev, 40)) {
			if (bcmp(ea, &wlc->pub->cur_etheraddr, ETHER_ADDR_LEN) == 0) {
				WL_ERROR(("%s: ra == primary, swap in null_ether\n", __FUNCTION__));
				ea = &ether_null;
				/* attr = 0; */
			}
		}
	}

	wlc_set_addrmatch(wlc, idx, ea, attr);
	return BCME_OK;
}

#ifdef NOTYET
static int
wlc_set_ta_hw(wlc_info_t *wlc, int idx, const struct ether_addr *ea)
{
	uint16 attr = AMT_ATTR_VALID|AMT_ATTR_A2;

	ASSERT(ea != NULL);

	if (!wlc->clk)
		return BCME_NOCLK;

	if (ETHER_ISNULLADDR(ea))
		attr = 0;

	/* No special entry for primary TA in RMCTA */
	wlc_set_addrmatch(wlc, idx, ea, attr);
	return BCME_OK;
}
#endif /* NOTYET */

/*****
AP/GO/ibss      A3              No DS           a2_also == FALSE
GC/infra STA/   A2|A3           From DS         a2_also == TRUE
*/
static int
wlc_set_bssid_hw(wlc_info_t *wlc, int idx, const struct ether_addr *ea, bool a2_also)
{
	uint16 attr = AMT_ATTR_VALID|AMT_ATTR_A3;

	WL_TRACE(("%s\n", __FUNCTION__));

	ASSERT(ea != NULL);

	if (!wlc->clk)
		return BCME_NOCLK;

	if (a2_also) {
		/* infra STA/GC ucode need to inspect A1 too for BSSID
		 * when (FromDS, ToDS) = (0, 1)
		 */
		attr |= (AMT_ATTR_A1 | AMT_ATTR_A2);
	}

	if (ETHER_ISNULLADDR(ea))
		attr = 0;

	if (idx == PRIMARY_INTF)
		idx = WLC_ADDRMATCH_IDX_BSSID;

	wlc_set_addrmatch(wlc, idx, ea, attr);
	return BCME_OK;
}

/*
Set BSS Index in bits 10:8 of SHM addr - M_P2P_GO_IND_BMP.
This is used by ucode to determine the appropriate null frame
template to use when BTCX PS protection is enabled.
This is primarily used by MCHAN when switching interfaces.
This is also used when the only interface active is a STA.
For all other cases, BTCX PS protection is not used.
*/
void
wlc_mcnx_shm_bss_idx_set(wlc_mcnx_info_t *mcnx, int bss_idx)
{
	uint16 go_ind_bmp;

	go_ind_bmp = wlc_mcnx_read_shm(mcnx, M_P2P_GO_IND_BMP);
	go_ind_bmp &= ~M_P2P_BSS_INDEX_MASK;
	go_ind_bmp |= ((bss_idx << M_P2P_BSS_INDEX_SHIFT_BITS) & M_P2P_BSS_INDEX_MASK);
	wlc_mcnx_write_shm(mcnx, M_P2P_GO_IND_BMP, go_ind_bmp);
}

/* debug... */
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static void
wlc_mcnx_BSS_dump(wlc_mcnx_info_t *mcnx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_info_t *wlc = mcnx->wlc;
	uint bss;
	uint offset;
	uint16 val;
	uint i;
	const char *p2p_bss[] = {
		"BCN_INT",
		"DTIM_PRD",
		"ST",
		"N_PRE_TBTT",
		"CTW",
		"N_CTW_END",
		"NOA_CNT",
		"N_NOA",
		"NOA_DUR",
		"NOA_TD",
		"NOA_OFS",
		"DTIM_CNT",
	};

	if (!wlc->clk)
		return;

	bss = wlc_mcnx_BSS_idx(mcnx, cfg);
	ASSERT(bss < M_P2P_BSS_MAX);

	bcm_bprintf(b, "\tSHM BSS: %u\n", bss);
	for (i = 0; i < M_P2P_BSS_BLK_SZ; i ++) {
		offset = M_P2P_BSS(bss, i);
		val = wlc_mcnx_read_shm(mcnx, offset);
		bcm_bprintf(b, "\t\t%04x(%04x:%04x): %04x %s\n",
		            mcnx->d11shm + offset, offset, offset >> 1, val,
		            i < ARRAYSIZE(p2p_bss) ? p2p_bss[i] : "unknown");
	}
	offset = M_P2P_PRE_TBTT(bss);
	val = wlc_mcnx_read_shm(mcnx, offset);
	bcm_bprintf(b, "\t\t%04x(%04x:%04x): %04x %s\n",
	            mcnx->d11shm + offset, offset, offset >> 1, val,
	            "PRE_TBTT");
	bcm_bprintf(b, "\tSHM INTR: %u\n", bss);
	for (i = 0; i < M_P2P_I_BLK_SZ; i ++) {
		offset = M_P2P_I(bss, i);
		val = wlc_mcnx_read_shm(mcnx, offset);
		bcm_bprintf(b, "\t\t%04x(%04x:%04x): %04x\n",
		            mcnx->d11shm + offset, offset, offset >> 1, val);
	}
	bcm_bprintf(b, "\tSHM TSF: %u\n", bss);
	offset = M_P2P_TSF(bss, 0);
	bcm_bprintf(b, "\t\t%04x(%04x:%04x): %04x%04x%04x%04x (remote - local)\n",
	            mcnx->d11shm + offset, offset, offset >> 1,
	            wlc_mcnx_read_shm(mcnx, M_P2P_TSF(bss, 3)),
	            wlc_mcnx_read_shm(mcnx, M_P2P_TSF(bss, 2)),
	            wlc_mcnx_read_shm(mcnx, M_P2P_TSF(bss, 1)),
	            wlc_mcnx_read_shm(mcnx, M_P2P_TSF(bss, 0)));
}

static uint16
wlc_mcnx_rcmta_type_get(wlc_mcnx_info_t *mcnx, int idx)
{
	uint16 ret_val = 0;
	wlc_info_t *wlc = mcnx->wlc;

	ASSERT(idx >= mcnx->p2p_start_idx && idx < mcnx->p2p_max_idx);

	if (D11REV_GE(wlc->pub->corerev, 40))
		ret_val = wlc_read_amtinfo_by_idx(wlc, idx);
	else
		ret_val = wlc_mcnx_read_shm(mcnx, M_ADDR_BMP_BLK(idx - mcnx->p2p_start_idx));
	return ret_val;
}

static void
wlc_mcnx_rcmta_dump(wlc_mcnx_info_t *mcnx, struct bcmstrbuf *b)
{
	wlc_info_t *wlc = mcnx->wlc;
	int i;
	struct ether_addr ea;
	char eabuf[ETHER_ADDR_STR_LEN];

	if (!wlc->clk)
		return;

	for (i = mcnx->p2p_start_idx; i < mcnx->p2p_max_idx; i ++) {
		uint16 type;
		char typestr[64];
		uint16 attr;
		const bcm_bit_desc_t at_flags[] = {
			{ADDR_BMP_RA, "RA"},
			{ADDR_BMP_TA, "TA"},
			{ADDR_BMP_BSSID, "BSSID"},
			{ADDR_BMP_AP, "AP"},
			{ADDR_BMP_STA, "STA"},
			{ADDR_BMP_P2P_GO, "GO"},
			{ADDR_BMP_P2P_GC, "GC"},
			{ADDR_BMP_P2P_DISC, "DISC"},
			{0, NULL}
		};

		type = wlc_mcnx_rcmta_type_get(mcnx, i);

		wlc_get_addrmatch(wlc, i, &ea, &attr);
		if (ETHER_ISNULLADDR(&ea) && type == 0 &&
		    !isset(mcnx->d11rao, i - mcnx->p2p_start_idx))
			continue;

		bcm_format_flags(at_flags, type, typestr, sizeof(typestr));
		bcm_bprintf(b, "\t%d %s %d 0x%04x[%s]\n",
		            i, bcm_ether_ntoa(&ea, eabuf),
		            i - mcnx->p2p_start_idx, type, typestr);
	}
}

static const char *intr_names[] = {
	"pretbtt",
	"ctwend",
	"abs",
	"psc"
};

static void
wlc_mcnx_info_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
	wlc_mcnx_info_t *mcnx = (wlc_mcnx_info_t *)ctx;
	wlc_info_t *wlc = mcnx->wlc;
	char flagstr[64];
	uint32 tsfo_l = 0, tsfo_h = 0;
	bss_mcnx_info_t *bmi;
	uint i;
	const bcm_bit_desc_t cfg_flags[] = {
		{MCNX_TBTT_INFO, "tbtt"},
		{MCNX_BSS_INFO, "bss"},
		{MCNX_FORCED_HPS, "hps"},
		{0, NULL}
	};

	ASSERT(cfg != NULL);

	bmi = BSS_MCNX_INFO(mcnx, cfg);
	if (bmi == NULL)
		return;

	bcm_bprintf(b, "\tRA rcmta: %d", bmi->rcmta_ra_idx);
	if (wlc->clk &&
	    bmi->rcmta_ra_idx >= mcnx->p2p_start_idx && bmi->rcmta_ra_idx < mcnx->p2p_max_idx) {
		bcm_bprintf(b, " type: 0x%x", wlc_mcnx_rcmta_type_get(mcnx, bmi->rcmta_ra_idx));
	}
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "\tBSSID rcmta: %d", bmi->rcmta_bssid_idx);
	if (wlc->clk &&
	    bmi->rcmta_bssid_idx >= mcnx->p2p_start_idx &&
	    bmi->rcmta_bssid_idx < mcnx->p2p_max_idx) {
		bcm_bprintf(b, " type: 0x%x", wlc_mcnx_rcmta_type_get(mcnx, bmi->rcmta_bssid_idx));
	}
	bcm_bprintf(b, "\n");

	/* bsscfg flags */
	bcm_format_flags(cfg_flags, bmi->flags, flagstr, sizeof(flagstr));
	bcm_bprintf(b, "\tflags: 0x%x [%s]\n", bmi->flags, flagstr);
	/* cached TBTT in local TSF time */
	bcm_bprintf(b, "\ttbtt: 0x%x%08x (local)\n", bmi->tbtt_h, bmi->tbtt_l);
	/* extrapolated TBTT (verify the code is correct...) */
	if (wlc->clk && cfg->associated && bmi->bcn_prd != 0) {
		uint32 tbtt_l, tbtt_h, tbtt_o;
		wlc_read_tsf(wlc, &tbtt_l, &tbtt_h);
		wlc_mcnx_l2r_tsf64(mcnx, cfg, tbtt_h, tbtt_l, &tbtt_h, &tbtt_l);
		tbtt_o = wlc_calc_tbtt_offset(bmi->bcn_prd, tbtt_h, tbtt_l);
		wlc_uint64_sub(&tbtt_h, &tbtt_l, 0, tbtt_o);
		wlc_uint64_add(&tbtt_h, &tbtt_l, 0, bmi->bcn_prd << 10);
		wlc_mcnx_r2l_tsf64(mcnx, cfg, tbtt_h, tbtt_l, &tbtt_h, &tbtt_l);
		bcm_bprintf(b, "\tnext tbtt: 0x%x%08x (local)\n", tbtt_h, tbtt_l);
	}
	/* cached TSF offsets */
	bcm_bprintf(b, "\ttsfo: 0x%x%08x (local - remote)\n", bmi->tsfo_h, bmi->tsfo_l);
	wlc_uint64_sub(&tsfo_h, &tsfo_l, bmi->tsfo_h, bmi->tsfo_l);
	bcm_bprintf(b, "\ttsfo: 0x%x%08x (remote - local)\n", tsfo_h, tsfo_l);
	/* extrapolated times */
	if (wlc->clk && cfg->associated) {
		uint32 tsf_l, tsf_h;
		wlc_read_tsf(wlc, &tsf_l, &tsf_h);
		bcm_bprintf(b, "\ttsf: 0x%x%08x (local)\n", tsf_h, tsf_l);
		wlc_mcnx_l2r_tsf64(mcnx, cfg, tsf_h, tsf_l, &tsf_h, &tsf_l);
		bcm_bprintf(b, "\ttsf: 0x%x%08x (remote)\n", tsf_h, tsf_l);
	}
	/* s/w stats */
	bcm_bprintf(b, "\t");
	for (i = 0; i < M_P2P_I_BLK_SZ; i ++)
		bcm_bprintf(b, "%s: %u ", intr_names[i], bmi->stats.intr[i]);
	bcm_bprintf(b, "\n");
	bcm_bprintf(b, "\tbcnadopt: %u\n", bmi->stats.bcnadopt);

	/* SHM BSS block */
	wlc_mcnx_BSS_dump(mcnx, cfg, b);
}

static int
wlc_mcnx_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_mcnx_info_t *mcnx = (wlc_mcnx_info_t *)ctx;
	wlc_info_t *wlc = mcnx->wlc;
	int idx;
	wlc_bsscfg_t *cfg;
	char eabuf[ETHER_ADDR_STR_LEN];

	bcm_bprintf(b, "<INFO>\n");
	bcm_bprintf(b, "\tmcnx: %d\n", MCNX_ENAB(wlc->pub));
	bcm_bprintf(b, "\tsuspend: %d\n", mcnx->mac_suspends);
	bcm_bprintf(b, "<RESOURCE>\n");
	for (idx = 0; idx < (int)sizeof(mcnx->d11cbo); idx ++)
		bcm_bprintf(b, "\td11cbo[%d]: 0x%x\n", idx, mcnx->d11cbo[idx]);
	for (idx = 0; idx < (int)sizeof(mcnx->d11rao); idx ++)
		bcm_bprintf(b, "\td11rao[%d]: 0x%x\n", idx, mcnx->d11rao[idx]);
	bcm_bprintf(b, "\td11shm: 0x%04x(0x%04x)\n", mcnx->d11shm, mcnx->d11shm >> 1);

	/* Power Save */
	bcm_bprintf(b, "<Power Save>\n");
	bcm_bprintf(b, "\thps: %04x\n", mcnx->hps);
	if (wlc->clk) {
		uint offset = M_P2P_HPS;
		uint16 val = wlc_mcnx_read_shm(mcnx, offset);
		bcm_bprintf(b, "\tSHM HPS:\n");
		bcm_bprintf(b, "\t\t%04x(%04x:%04x): %04x\n",
		            mcnx->d11shm + offset, offset, offset >> 1, val);
	}

	/* SHM GO channel */
	bcm_bprintf(b, "<Channel>\n");
	if (wlc->clk) {
		uint offset = M_P2P_GO_CHANNEL;
		uint16 val = wlc_mcnx_read_shm(mcnx, offset);
		bcm_bprintf(b, "\tSHM CHANNEL:\n");
		bcm_bprintf(b, "\t\t%04x(%04x:%04x): %04x\n",
		            mcnx->d11shm + offset, offset, offset >> 1, val);
	}

	/* RCMTA and ADDR_BMP */
	bcm_bprintf(b, "<RCMTA>\n");
	wlc_mcnx_rcmta_dump(mcnx, b);

	/* group owners SHM BSS blocks */
	FOREACH_BSS(wlc, idx, cfg) {
		if (!P2P_GO(wlc, cfg))
			continue;
		bcm_bprintf(b, "<GO %s bsscfg %d>\n",
		            bcm_ether_ntoa(&cfg->cur_etheraddr, eabuf), WLC_BSSCFG_IDX(cfg));
		wlc_mcnx_info_dump(mcnx, cfg, b);
	}

	/* clients SHM BSS blocks */
	FOREACH_BSS(wlc, idx, cfg) {
		if (!P2P_CLIENT(wlc, cfg))
			continue;
		bcm_bprintf(b, "<Client %s bsscfg %d>\n",
		            bcm_ether_ntoa(&cfg->cur_etheraddr, eabuf), WLC_BSSCFG_IDX(cfg));
		wlc_mcnx_info_dump(mcnx, cfg, b);
	}

	/* non-p2p SHM BSS blocks */
	FOREACH_BSS(wlc, idx, cfg) {
		if (P2P_IF(wlc, cfg))
			continue;
		bcm_bprintf(b, "<WLAN %s bsscfg %d>\n",
		            bcm_ether_ntoa(&cfg->cur_etheraddr, eabuf), WLC_BSSCFG_IDX(cfg));
		wlc_mcnx_info_dump(mcnx, cfg, b);
	}

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */
#endif	/* WLMCNX */
