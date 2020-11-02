/*
 * Wrapper to scb rate selection algorithm of Broadcom
 * 802.11 Networking Adapter Device Driver.
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
 * $Id: wlc_scb_ratesel.c 726420 2017-10-12 13:34:19Z $
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlRateSelection]
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
#include <wlioctl.h>

#include <proto/802.11.h>
#include <d11.h>

#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>

#include <wlc_scb.h>
#include <wlc_phy_hal.h>
#include <wlc_antsel.h>
#include <wlc_scb_ratesel.h>
#ifdef WL_LPC
#include <wlc_scb_powersel.h>
#endif // endif
#include <wlc_txc.h>

#include <wl_dbg.h>

#ifdef WL11AC
#include <wlc_vht.h>
#endif // endif
#include <wlc_ht.h>

/* iovar table */
enum {
	IOV_SCBRATE_DUMMY, /* dummy one in order to register the module */
	IOV_SCB_RATESET,    /* dump the per-scb rate set */
	IOV_RATE_HISTO_REPORT /* get per scb rate map histogram */
};

static const bcm_iovar_t scbrate_iovars[] = {
	{"scbrate_dummy", IOV_SCBRATE_DUMMY, (IOVF_SET_DOWN), IOVT_BOOL, 0},
#if defined(WLSCB_HISTO)
	{"rate_histo_report", IOV_RATE_HISTO_REPORT, 0, IOVT_BUFFER, 0},
#endif /* WLSCB_HISTO */
	{NULL, 0, 0, 0, 0}
};

typedef struct ppr_rateset {
	uint8 vht_mcsmap;              /* supported vht mcs nss bit map */
	uint8 mcs[PHY_CORE_MAX];       /* supported mcs index bit map */
	uint16 vht_mcsmap_prop;        /* vht proprietary rates bit map */
} ppr_rateset_t;

/* Supported rates for current chanspec/country */
typedef struct ppr_support_rates {
	chanspec_t chanspec;
	clm_country_t country;
	ppr_rateset_t ppr_20_rates;
#if defined(WL11N) || defined(WL11AC)
	ppr_rateset_t ppr_40_rates;
#endif // endif
#ifdef WL11AC
	ppr_rateset_t ppr_80_rates;
	ppr_rateset_t ppr_160_rates;
#endif // endif
	uint8 txstreams;
} ppr_support_rates_t;

struct wlc_ratesel_info {
	wlc_info_t	*wlc;		/* pointer to main wlc structure */
	wlc_pub_t	*pub;		/* public common code handler */
	ratesel_info_t *rsi;
	int32 scb_handle;
	int32 cubby_sz;
	ppr_support_rates_t *ppr_rates;
};

typedef struct ratesel_cubby ratesel_cubby_t;

/* rcb is per scb per ac rate control block. */
struct ratesel_cubby {
	rcb_t *scb_cubby;
};

/* vht rate default override
 * for 0 stream the value 0b 1111 1111 1111 1111 = 0xFFFFu
 * for 1 stream the value 0b 1111 1111 1111 1110 = 0xFFFEu
 * for 2 stream the value 0b 1111 1111 1111 1010 = 0xFFFAu
 * for 3 stream the value 0b 1111 1111 1110 1010 = 0xFFEAu
 * for 4 stream the value 0b 1111 1111 1010 1010 = 0xFFAAu
 * ...
 */
#define VHT_MCSMAP_0 0xFFFFu /* 0 vht mcs streams */
#define VHT_MCSMAP_1 0xFFFEu /* 1 vht mcs streams */
#define VHT_MCSMAP_2 0xFFFAu /* 2 vht mcs streams */
#define VHT_MCSMAP_3 0xFFEAu /* 3 vht mcs streams */
#define VHT_MCSMAP_4 0xFFAAu /* 4 vht mcs streams */
#define VHT_MCSMAP_5 0xFEAAu /* 5 vht mcs streams */
#define VHT_MCSMAP_6 0xFAAAu /* 6 vht mcs streams */
#define VHT_MCSMAP_7 0xEAAAu /* 7 vht mcs streams */
#define VHT_MCSMAP_8 0xAAAAu /* 8 vht mcs streams */

#define SCB_RATESEL_INFO(wss, scb) ((SCB_CUBBY((scb), (wrsi)->scb_handle)))

#if defined(WME_PER_AC_TX_PARAMS)
#define SCB_RATESEL_CUBBY(wrsi, scb, ac) 	\
	((void *)(((char*)((ratesel_cubby_t *)SCB_RATESEL_INFO(wrsi, scb))->scb_cubby) + \
		(ac * (wrsi)->cubby_sz)))
#else /* WME_PER_AC_TX_PARAMS */
#define SCB_RATESEL_CUBBY(wrsi, scb, ac)	\
	(((ratesel_cubby_t *)SCB_RATESEL_INFO(wrsi, scb))->scb_cubby)
#endif /* WME_PER_AC_TX_PARAMS */

static int wlc_scb_ratesel_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid,
	const char *name, void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);

static int wlc_scb_ratesel_scb_init(void *context, struct scb *scb);
static void wlc_scb_ratesel_scb_deinit(void *context, struct scb *scb);
#ifdef BCMDBG
extern void wlc_scb_ratesel_dump_scb(void *ctx, struct scb *scb, struct bcmstrbuf *b);
#endif // endif
#if defined(BCMDBG_DUMP)
static int wlc_scb_ratesel_dump_rateset(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	const struct ether_addr *ea, void *buf, int len);
#endif // endif

#if defined(WLSCB_HISTO)
static int wlc_scb_get_rate_histo(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	wl_rate_histo_report_t *req, void *buf, int len);
#endif /* WLSCB_HISTO */
static ratespec_t wlc_scb_ratesel_getcurspec(wlc_ratesel_info_t *wrsi,
	struct scb *scb, uint8 ac);

static rcb_t *wlc_scb_ratesel_get_cubby(wlc_ratesel_info_t *wrsi, struct scb *scb,
	uint8 ac);
static int wlc_scb_ratesel_cubby_sz(void);
#ifdef WL11N
void wlc_scb_ratesel_rssi_enable(rssi_ctx_t *ctx);
void wlc_scb_ratesel_rssi_disable(rssi_ctx_t *ctx);
int wlc_scb_ratesel_get_rssi(rssi_ctx_t *ctx);
#endif // endif
/* Get CLM enabled rates bitmap for a bw */
static  ppr_rateset_t *
wlc_scb_ratesel_get_ppr_rates(wlc_info_t *wlc, wl_tx_bw_t bw);

static void wlc_scb_ratesel_ppr_updbmp(wlc_info_t *wlc, ppr_t *target_pwrs);

static void
wlc_scb_ratesel_ppr_filter(wlc_info_t *wlc, ppr_rateset_t *clm_rates,
	wlc_rateset_t *scb_rates, bool scb_VHT);

static wl_tx_bw_t rspecbw_to_bcmbw(uint8 bw);

wlc_ratesel_info_t *
BCMATTACHFN(wlc_scb_ratesel_attach)(wlc_info_t *wlc)
{
	wlc_ratesel_info_t *wrsi;
#ifdef WL11AC
	ppr_support_rates_t *ppr_rates;
#endif // endif

	if (!(wrsi = (wlc_ratesel_info_t *)MALLOC(wlc->osh, sizeof(wlc_ratesel_info_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	bzero((char *)wrsi, sizeof(wlc_ratesel_info_t));
#ifdef WL11AC
	if (!(ppr_rates = (ppr_support_rates_t *)MALLOC(wlc->osh, sizeof(ppr_support_rates_t)))) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}
	bzero(ppr_rates, sizeof(*ppr_rates));
	wrsi->ppr_rates = ppr_rates;
#endif // endif
	wrsi->wlc = wlc;
	wrsi->pub = wlc->pub;

	if ((wrsi->rsi = wlc_ratesel_attach(wlc)) == NULL) {
		WL_ERROR(("%s: failed\n", __FUNCTION__));
		goto fail;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, scbrate_iovars, "scbrate", wrsi, wlc_scb_ratesel_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s:wlc_module_register failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* reserve cubby in the scb container for per-scb-ac private data */
	wrsi->scb_handle = wlc_scb_cubby_reserve(wlc, wlc_scb_ratesel_cubby_sz(),
	                                        wlc_scb_ratesel_scb_init,
	                                        wlc_scb_ratesel_scb_deinit,
#ifdef BCMDBG
	                                        wlc_scb_ratesel_dump_scb,
#else
	                                        NULL,
#endif // endif
	                                        (void *)wlc);

	if (wrsi->scb_handle < 0) {
		WL_ERROR(("wl%d: %s:wlc_scb_cubby_reserve failed\n", wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	wrsi->cubby_sz = wlc_ratesel_rcb_sz();

#ifdef WL11N
	wlc_ratesel_rssi_attach(wrsi->rsi, wlc_scb_ratesel_rssi_enable,
		wlc_scb_ratesel_rssi_disable, wlc_scb_ratesel_get_rssi);
#endif // endif

	return wrsi;

fail:
	if (wrsi->rsi)
		wlc_ratesel_detach(wrsi->rsi);

	MFREE(wlc->osh, wrsi, sizeof(wlc_ratesel_info_t));
	return NULL;
}

void
BCMATTACHFN(wlc_scb_ratesel_detach)(wlc_ratesel_info_t *wrsi)
{
	if (!wrsi)
		return;

	wlc_module_unregister(wrsi->pub, "scbrate", wrsi);
	wlc_ratesel_detach(wrsi->rsi);

#ifdef WL11AC
	MFREE(wrsi->pub->osh, wrsi->ppr_rates, sizeof(ppr_support_rates_t));
#endif // endif
	MFREE(wrsi->pub->osh, wrsi, sizeof(wlc_ratesel_info_t));
}

/* alloc per ac cubby space on scb attach. */
static int
wlc_scb_ratesel_scb_init(void *context, struct scb *scb)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	wlc_ratesel_info_t *wrsi = wlc->wrsi;
	ratesel_cubby_t *cubby_info = SCB_RATESEL_INFO(wrsi, scb);
	rcb_t *scb_rate_cubby;
	int cubby_size;

#if defined(WME_PER_AC_TX_PARAMS)
	cubby_size = AC_COUNT * wrsi->cubby_sz;
#else
	cubby_size = wrsi->cubby_sz;
#endif // endif

	WL_RATE(("%s scb %p allocate cubby space.\n", __FUNCTION__, scb));
	if (scb && !SCB_INTERNAL(scb)) {
		scb_rate_cubby = (rcb_t *)MALLOC(wlc->osh, cubby_size);
		if (!scb_rate_cubby)
			return BCME_NOMEM;
		bzero(scb_rate_cubby, cubby_size);
		cubby_info->scb_cubby = scb_rate_cubby;
	}
	return BCME_OK;
}

/* free cubby space after scb detach */
static void
wlc_scb_ratesel_scb_deinit(void *context, struct scb *scb)
{
	wlc_info_t *wlc = (wlc_info_t *)context;
	wlc_ratesel_info_t *wrsi = wlc->wrsi;
	ratesel_cubby_t *cubby_info = SCB_RATESEL_INFO(wrsi, scb);
	int cubby_size;

#if defined(WME_PER_AC_TX_PARAMS)
	cubby_size = AC_COUNT * wrsi->cubby_sz;
#else
	cubby_size = wrsi->cubby_sz;
#endif // endif

	WL_RATE(("%s scb %p free cubby space.\n", __FUNCTION__, scb));
	if (wlc && cubby_info && !SCB_INTERNAL(scb) && cubby_info->scb_cubby) {
		MFREE(wlc->osh, cubby_info->scb_cubby, cubby_size);
		cubby_info->scb_cubby = NULL;
	}
}

/* handle SCBRATE related iovars */
static int
wlc_scb_ratesel_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int val_size, struct wlc_if *wlcif)
{
	wlc_ratesel_info_t *wrsi = hdl;
	wlc_info_t *wlc;
	wlc_bsscfg_t *bsscfg;
	int err = 0;

	wlc = wrsi->wlc;
	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	if (!bsscfg) {
		return BCME_ERROR;
	}

	switch (actionid) {
#if defined(BCMDBG_DUMP)
	case IOV_GVAL(IOV_SCB_RATESET):
		err = wlc_scb_ratesel_dump_rateset(wlc, bsscfg,
			(struct ether_addr *)params, arg, alen);
		break;
#endif // endif

#if defined(WLSCB_HISTO)
	case IOV_GVAL(IOV_RATE_HISTO_REPORT):
		if (plen < sizeof(wl_rate_histo_report_t)) {
			err = wlc_scb_get_rate_histo(wlc, bsscfg, NULL, arg, alen);
		} else {
			err = wlc_scb_get_rate_histo(wlc, bsscfg,
					(wl_rate_histo_report_t *) params, arg, alen);
		}
		break;
#endif /* WLSCB_HISTO */

	default:
		err = BCME_UNSUPPORTED;
	}
	return err;
}

static rcb_t *
wlc_scb_ratesel_get_cubby(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 ac)
{
	ASSERT(wrsi);

	ac = (WME_PER_AC_MAXRATE_ENAB(wrsi->pub) ? ac : 0);
	ASSERT(ac < AC_COUNT);

	return (SCB_RATESEL_CUBBY(wrsi, scb, ac));
}

#ifdef BCMDBG
extern void
wlc_scb_ratesel_dump_scb(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;
	wlc_ratesel_info_t *wrsi = wlc->wrsi;
	int ac;
	rcb_t *rcb;

	if (SCB_INTERNAL(scb))
		return;

	wlc_dump_rspec(wlc->scbstate, wlc_scb_ratesel_get_primary(wlc, scb, NULL), b);

	for (ac = 0; ac < WME_MAX_AC(wlc, scb); ac++) {
		rcb = SCB_RATESEL_CUBBY(wrsi, scb, ac);
		wlc_ratesel_dump_rcb(rcb, ac, b);
	}
}
#endif /* BCMDBG */

#if defined(BCMDBG_DUMP)
/* get the fixrate per scb/ac. */
int
wlc_scb_ratesel_get_fixrate(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_ratesel_info_t *wrsi = (wlc_ratesel_info_t *)ctx;
	int ac;
	rcb_t *state;

	for (ac = 0; ac < WME_MAX_AC(wrsi->wlc, scb); ac++) {
		if (!(state = SCB_RATESEL_CUBBY(wrsi, scb, ac))) {
			WL_ERROR(("skip ac %d\n", ac));
			continue;
		}
		wlc_ratesel_get_fixrate(state, ac, b);
	}
	return 0;
}

/* set the fixrate per scb/ac. */
int
wlc_scb_ratesel_set_fixrate(void *ctx, struct scb *scb, int ac, uint8 val)
{
	wlc_ratesel_info_t *wrsi = (wlc_ratesel_info_t *)ctx;
	int i;
	rcb_t *state = NULL;

	/* check AC validity */
	if (ac == -1) { /* For all access class */
		for (i = 0; i < WME_MAX_AC(wrsi->wlc, scb); i++) {
			if (!(state = SCB_RATESEL_CUBBY(wrsi, scb, i))) {
				WL_ERROR(("ac %d does not exist!\n", ac));
				return BCME_ERROR;
			}

			if (wlc_ratesel_set_fixrate(state, i, val) < 0)
				return BCME_BADARG;
		}
	} else if ((ac >= 0) && (ac < WME_MAX_AC(wrsi->wlc, scb))) { /* For single ac */
		if (!(state = SCB_RATESEL_CUBBY(wrsi, scb, ac))) {
				WL_ERROR(("ac %d does not exist!\n", ac));
				return BCME_ERROR;
		}

		if (wlc_ratesel_set_fixrate(state, ac, val) < 0)
			return BCME_BADARG;
	} else {
		WL_ERROR(("ac %d out of range [0, %d]\n", ac, WME_MAX_AC(wrsi->wlc, scb) - 1));
		return BCME_ERROR;
	}

	return 0;
}

/* dump per-scb state, tunable parameters, upon request, (say by wl ratedump) */
int
wlc_scb_ratesel_scbdump(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_ratesel_info_t *wrsi = (wlc_ratesel_info_t *)ctx;
	int32 ac;
	rcb_t *state;

	for (ac = 0; ac < WME_MAX_AC(wrsi->wlc, scb); ac++) {
		state = SCB_RATESEL_CUBBY(wrsi, scb, ac);
		if (state) {
			bcm_bprintf(b, "AC[%d] --- \n\t", ac);
			wlc_ratesel_scbdump(state, b);
		}
	}

	return 0;
}

/* dump the per scb rate set used by rate sel */
static int
wlc_scb_ratesel_dump_rateset(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	const struct ether_addr *ea, void *buf, int len)
{
	struct scb *scb;
	wlc_ratesel_info_t *wrsi;
	rcb_t *state;
	int ac, max_ac;
	struct bcmstrbuf b;

	ASSERT(ea != NULL);
	if (ea == NULL)
		return BCME_BADARG;

	if ((scb = wlc_scbfind(wlc, bsscfg, ea)) == NULL)
		return BCME_BADADDR;

	wrsi = wlc->wrsi;
	bcm_binit(&b, (char*)buf, len);
	max_ac = WME_MAX_AC(wlc, scb);

	for (ac = 0; ac < max_ac; ac ++) {
		state = SCB_RATESEL_CUBBY(wrsi, scb, ac);
		if (max_ac > 1)
			bcm_bprintf(&b, "AC[%d] --- \n\t", ac);
		wlc_ratesel_dump_rateset(state, &b);
	}
	return BCME_OK;
}
#endif // endif

#if defined(WLSCB_HISTO)
/* array of 55 elements (0-54); elements provide mapping to index for
 *	1, 2, 5 (5.5), 6, 9, 11, 12, 18, 24, 36, 48, 54
 * eg.
 *	wlc_legacy_rate_index[1] is 0
 *	wlc_legacy_rate_index[2] is 1
 *	wlc_legacy_rate_index[5] is 2
 *	...
 *	wlc_legacy_rate_index[48] is 10
 *	wlc_legacy_rate_index[54] is 11
 * For numbers in between (eg. 3, 4, 49, 50) maps to closest lower index
 * eg.
 *	wlc_legacy_rate_index[3] is 1 (same as 2Mbps)
 *	wlc_legacy_rate_index[4] is 1 (same as 2Mbps)
 */
uint8 wlc_legacy_rate_index[] = {
	0, 0,					/* 1Mbps (same for 0-1) */
	1, 1, 1,				/* 2Mbps (same for 2-4) */
	2,					/* 5Mbps (5.5) */
	3, 3, 3,				/* 6Mbps (same for 6-8) */
	4, 4,					/* 9Mbps (same for 9-10) */
	5,					/* 11Mbps */
	6, 6, 6, 6, 6, 6,			/* 12Mbps (same for 12-17) */
	7, 7, 7, 7, 7, 7,			/* 18Mbps (same for 18-23) */
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,	/* 24Mbps (same for 24-35) */
	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,	/* 36Mbps (same for 36-47) */
	10, 10, 10, 10, 10, 10,			/* 48Mbps (same for 48-53) */
	11					/* 54Mbps */
};

const uint8 wlc_legacy_rate_index_len =
	sizeof(wlc_legacy_rate_index) / sizeof(wlc_legacy_rate_index[0]);

/* fill the per scb rate histogram for the SCB corresponding to passed address (req->ea) */
static int
wlc_scb_get_rate_histo(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	wl_rate_histo_report_t *req, void *buf, int len)
{
	struct scb *scb = NULL;
	struct scb_iter scbiter;
	wl_rate_histo_report_t *rpt = buf;
	int rpt_len = sizeof(wl_rate_histo_report_t) + sizeof(wl_rate_histo_maps1_t);
	uint8 zero_ea[sizeof(struct ether_addr)] = { 0 };

	if (req != NULL) {
		if (req->ver != WL_HISTO_VER_1) {
		    return BCME_VERSION;
		}
		if (req->type != WL_HISTO_TYPE_RATE_MAP1) {
		    return BCME_UNSUPPORTED;
		}
		if (req->length < WL_HISTO_VER_1_FIXED_LEN) {
		    return BCME_BUFTOOSHORT;
		}
		if (req->fixed_len < WL_HISTO_VER_1_FIXED_LEN) {
			return BCME_BUFTOOSHORT;
		}
	}

	if (rpt == NULL || len < rpt_len) {
		return BCME_BUFTOOSHORT;
	}

	/* find scb if peer MAC is provided and is non-zero */
	if (req != NULL && memcmp(&zero_ea, &req->ea, sizeof(zero_ea)) &&
			(scb = wlc_scbfind(wlc, bsscfg, &req->ea)) == NULL) {
		return BCME_BADADDR;
	}

	/* if peer MAC is not provided or is zero, find the first scb */
	if (scb == NULL) {
		// if (BSSCFG_AP(bsscfg)) { ... }
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
			if (SCB_ASSOCIATED(scb)) {
				break;
			}
		}
		if (scb == NULL || !SCB_ASSOCIATED(scb)) {
			return BCME_NOTASSOCIATED;
		}
	}

	rpt->ver = WL_HISTO_VER_1;
	rpt->type = WL_HISTO_TYPE_RATE_MAP1;
	rpt->length = rpt_len;
	memcpy(&rpt->ea, &scb->ea, sizeof(rpt->ea));
	rpt->fixed_len = WL_HISTO_VER_1_FIXED_LEN;
	/* compute delta time from start timestamp */
	scb->histo.rx.seconds = wlc->pub->now - scb->histo.rx.seconds;
	scb->histo.tx.seconds = wlc->pub->now - scb->histo.tx.seconds;
	memcpy(&rpt->data, &scb->histo, sizeof(wl_rate_histo_maps1_t));

	/* reset on read */
	bzero(&scb->histo, sizeof(wl_rate_histo_maps1_t));
	scb->histo.rx.seconds = scb->histo.tx.seconds = wlc->pub->now;

	return BCME_OK;
}
#endif /* WLSCB_HISTO */

#ifdef WLATF
/* Get the rate selection control block pointer from ratesel cubby */
rcb_t *
wlc_scb_ratesel_getrcb(wlc_info_t *wlc, struct scb *scb, uint ac)
{
	wlc_ratesel_info_t *wrsi = wlc->wrsi;
	ASSERT(wrsi);

	if (!WME_PER_AC_MAXRATE_ENAB(wlc->pub)) {
		ac = 0;
	}
	ASSERT(ac < (uint)WME_MAX_AC(wlc, scb));

	return (SCB_RATESEL_CUBBY(wrsi, scb, ac));
}
#endif /* WLATF */

#ifdef WL11N
bool
wlc_scb_ratesel_sync(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 ac, uint now, int rssi)
{
	rcb_t *state;

	state = wlc_scb_ratesel_get_cubby(wrsi, scb, ac);

	return wlc_ratesel_sync(state, now, rssi);
}
#endif /* WL11N */

extern const uint8 prio2fifo[NUMPRIO];
extern int
wlc_wme_downgrade_fifo(wlc_info_t *wlc, uint* p_fifo, struct scb *scb);

static ratespec_t
wlc_scb_ratesel_getcurspec(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 ac)
{
	rcb_t *state;

	state = wlc_scb_ratesel_get_cubby(wrsi, scb, ac);
	if (state == NULL) {
		WL_ERROR(("%s: null state wrsi = %p scb = %p ac = %d\n",
			__FUNCTION__, wrsi, scb, ac));
		ASSERT(0);
		return (WLC_RATE_6M | RSPEC_BW_20MHZ);
	}
	return wlc_ratesel_getcurspec(state);
}

/* given only wlc and scb, return best guess at the primary rate */
ratespec_t
wlc_scb_ratesel_get_primary(wlc_info_t *wlc, struct scb *scb, void *pkt)
{
	ratespec_t rspec = 0;
	wlcband_t *scbband = wlc_scbband(scb);
	uint phyctl1_stf = wlc->stf->ss_opmode;
	uint8 prio;

#ifdef WL11N
	uint32 mimo_txbw = 0;
#if WL_HT_TXBW_OVERRIDE_ENAB
	uint32 _txbw2rspecbw[] = {
		RSPEC_BW_20MHZ, /* WL_TXBW20L	*/
		RSPEC_BW_20MHZ, /* WL_TXBW20U	*/
		RSPEC_BW_40MHZ, /* WL_TXBW40	*/
		RSPEC_BW_40MHZ, /* WL_TXBW40DUP */
		RSPEC_BW_20MHZ, /* WL_TXBW20LL */
		RSPEC_BW_20MHZ, /* WL_TXBW20LU */
		RSPEC_BW_20MHZ, /* WL_TXBW20UL */
		RSPEC_BW_20MHZ, /* WL_TXBW20UU */
		RSPEC_BW_40MHZ, /* WL_TXBW40L */
		RSPEC_BW_40MHZ, /* WL_TXBW40U */
		RSPEC_BW_80MHZ /* WL_TXBW80 */
	};
	int8 txbw_override_idx;
#endif /* WL_HT_TXBW_OVERRIDE_ENAB */
#endif /* WL11N */

	prio = 0;
	if ((pkt != NULL) && SCB_QOS(scb)) {
		prio = (uint8)PKTPRIO(pkt);
		ASSERT(prio <= MAXPRIO);
	}

	if (scbband == NULL) {
		ASSERT(0);
		return 0;
	}
	/* XXX 4360: the following rspec calc code is now in 3 places;
	 * here, d11achdrs and d11nhdrs.
	 * Need to consolidate this.
	 */
	if (RSPEC_ACTIVE(scbband->rspec_override)) {
		/* get override if active */
		rspec = scbband->rspec_override;
	} else {
		/* let ratesel figure it out if override not present */
		rspec = wlc_scb_ratesel_getcurspec(wlc->wrsi, scb, WME_PRIO2AC(prio));
	}

#ifdef WL11N
	if (N_ENAB(wlc->pub)) {
		/* apply siso/cdd to single stream mcs's or ofdm if rspec is auto selected */
		if (((IS_MCS(rspec) && IS_SINGLE_STREAM(rspec & RSPEC_RATE_MASK)) ||
			IS_OFDM(rspec)) &&
			!(rspec & RSPEC_OVERRIDE_MODE)) {

			rspec &= ~(RSPEC_TXEXP_MASK | RSPEC_STBC);

			/* For SISO MCS use STBC if possible */
			if (IS_MCS(rspec) && (WLC_IS_STBC_TX_FORCED(wlc) ||
				((RSPEC_ISVHT(rspec) && WLC_STF_SS_STBC_VHT_AUTO(wlc, scb)) ||
				(RSPEC_ISHT(rspec) && WLC_STF_SS_STBC_HT_AUTO(wlc, scb))))) {
				ASSERT(WLC_STBC_CAP_PHY(wlc));
				rspec |= RSPEC_STBC;
			} else if (phyctl1_stf == PHY_TXC1_MODE_CDD) {
				rspec |= (1 << RSPEC_TXEXP_SHIFT);
			}
		}

		/* bandwidth */
		if (RSPEC_BW(rspec) != RSPEC_BW_UNSPECIFIED) {
			mimo_txbw = RSPEC_BW(rspec);
		}
		else if ((CHSPEC_IS8080(wlc->chanspec) &&
			scb->flags3 & SCB3_IS_80_80) &&
			RSPEC_ISVHT(rspec)) {
			mimo_txbw = RSPEC_BW_160MHZ;
		}
		else if ((CHSPEC_IS160(wlc->chanspec) &&
			scb->flags3 & SCB3_IS_160) &&
			RSPEC_ISVHT(rspec)) {
			mimo_txbw = RSPEC_BW_160MHZ;
		}
		else if (CHSPEC_BW_GE(wlc->chanspec, WL_CHANSPEC_BW_80) &&
			RSPEC_ISVHT(rspec)) {
			mimo_txbw = RSPEC_BW_80MHZ;
		} else if (CHSPEC_BW_GE(wlc->chanspec, WL_CHANSPEC_BW_40)) {
			/* default txbw is 20in40 */
			mimo_txbw = RSPEC_BW_20MHZ;

			if (RSPEC_ISHT(rspec) || RSPEC_ISVHT(rspec)) {
				if (scb->flags & SCB_IS40) {
					mimo_txbw = RSPEC_BW_40MHZ;
#ifdef WLMCHAN
				/* XXX 4360: why would scb->flags indicate is40 if the sta
				 * is associated at a 20MHz? Do we need different flags for
				 * capability (is40) from operational for the current state,
				 * which would be is20?  This same problem needs to be fixed
				 * for the 80MHz case.
				 */
				/* PR95044: if mchan enabled and bsscfg is AP, then must
				 * check the bsscfg chanspec to make sure our AP is
				 * operating on 40MHz channel.
				 */
				if (MCHAN_ENAB(wlc->pub) && BSSCFG_AP(scb->bsscfg) &&
					CHSPEC_IS20(scb->bsscfg->current_bss->chanspec)) {
					mimo_txbw = RSPEC_BW_20MHZ;
				}
#endif /* WLMCHAN */
				}
			}
		} else	{
			mimo_txbw = RSPEC_BW_20MHZ;
		}

#if WL_HT_TXBW_OVERRIDE_ENAB
			if (CHSPEC_IS40(wlc->chanspec) || CHSPEC_IS80(wlc->chanspec)) {
				WL_HT_TXBW_OVERRIDE_IDX(wlc->hti, rspec, txbw_override_idx);

				if (txbw_override_idx >= 0) {
					mimo_txbw = _txbw2rspecbw[txbw_override_idx];
				}
			}
#endif /* WL_HT_TXBW_OVERRIDE_ENAB */
		rspec &= ~RSPEC_BW_MASK;
		rspec |= mimo_txbw;
	} else
#endif /* WL11N */
	{
		rspec |= RSPEC_BW_20MHZ;
		/* for nphy, stf of ofdm frames must follow policies */
		if ((WLCISNPHY(scbband) || WLCISHTPHY(scbband)) && IS_OFDM(rspec)) {
			rspec &= ~RSPEC_TXEXP_MASK;
			if (phyctl1_stf == PHY_TXC1_MODE_CDD) {
				rspec |= (1 << RSPEC_TXEXP_SHIFT);
			}
		}
	}

	if (!RSPEC_ACTIVE(scbband->rspec_override)) {
		if (IS_MCS(rspec) && (WLC_HT_GET_SGI_TX(wlc->hti) != OFF)) {
			rspec |= RSPEC_SHORT_GI;
		}
		else {
			rspec &= ~RSPEC_SHORT_GI;
		}

	}

	if (!RSPEC_ACTIVE(scbband->rspec_override)) {
		ASSERT(!(rspec & RSPEC_LDPC_CODING));
		rspec &= ~RSPEC_LDPC_CODING;
		if (wlc->stf->ldpc_tx == ON ||
			(SCB_LDPC_CAP(scb) && wlc->stf->ldpc_tx == AUTO)) {
			if (IS_MCS(rspec))
				rspec |= RSPEC_LDPC_CODING;
		}
	}
	return rspec;
}

/* wrapper function to select transmit rate given per-scb state */
void BCMFASTPATH
wlc_scb_ratesel_gettxrate(wlc_ratesel_info_t *wrsi, struct scb *scb, uint16 *frameid,
	ratesel_txparams_t *cur_rate, uint16 *flags)
{
	rcb_t *state;

	state = wlc_scb_ratesel_get_cubby(wrsi, scb, cur_rate->ac);
	if (state == NULL) {
		WL_ERROR(("%s: null state wrsi = %p scb = %p ac = %d\n",
			__FUNCTION__, wrsi, scb, cur_rate->ac));
		ASSERT(0);
		return;
	}

	wlc_ratesel_gettxrate(state, frameid, cur_rate, flags);
}

#ifdef WL11N
void
wlc_scb_ratesel_probe_ready(wlc_ratesel_info_t *wrsi, struct scb *scb, uint16 frameid,
	bool is_ampdu, uint8 ampdu_txretry, uint8 ac)
{
	rcb_t *state;

	state = wlc_scb_ratesel_get_cubby(wrsi, scb, ac);
	if (state == NULL) {
		WL_ERROR(("%s: null state wrsi = %p scb = %p ac = %d\n",
			__FUNCTION__, wrsi, scb, ac));
		ASSERT(0);
		return;
	}
	wlc_ratesel_probe_ready(state, frameid, is_ampdu, ampdu_txretry);
}

void BCMFASTPATH
wlc_scb_ratesel_upd_rxstats(wlc_ratesel_info_t *wrsi, ratespec_t rx_rspec, uint16 rxstatus2)
{
	wlc_ratesel_upd_rxstats(wrsi->rsi, rx_rspec, rxstatus2);
}
#endif /* WL11N */

/* non-AMPDU txstatus rate update, default to use non-mcs rates only */
void
wlc_scb_ratesel_upd_txstatus_normalack(wlc_ratesel_info_t *wrsi, struct scb *scb, tx_status_t *txs,
	uint16 sfbl, uint16 lfbl, uint8 tx_mcs,
	bool sgi, uint8 antselid, bool fbr, uint8 ac)
{
	rcb_t *state;

	state = wlc_scb_ratesel_get_cubby(wrsi, scb, ac);
	if (state == NULL) {
		ASSERT(0);
		return;
	}
	wlc_ratesel_upd_txstatus_normalack(state, txs, sfbl, lfbl, tx_mcs, sgi, antselid, fbr);
}

#ifdef WL11N
void
wlc_scb_ratesel_aci_change(wlc_ratesel_info_t *wrsi, bool aci_state)
{
	wlc_ratesel_aci_change(wrsi->rsi, aci_state);
}

/*
 * Return the fallback rate of the specified mcs rate.
 * Ensure that is a mcs rate too.
 */
ratespec_t
wlc_scb_ratesel_getmcsfbr(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 ac, uint8 mcs)
{
	rcb_t *state;

	state = wlc_scb_ratesel_get_cubby(wrsi, scb, ac);
	ASSERT(state);

	return (wlc_ratesel_getmcsfbr(state, ac, mcs));
}

#ifdef WLAMPDU_MAC
/*
 * The case that (mrt+fbr) == 0 is handled as RTS transmission failure.
 */
void
wlc_scb_ratesel_upd_txs_ampdu(wlc_ratesel_info_t *wrsi, struct scb *scb,
	ratesel_txs_t *rs_txs, tx_status_t *txs, bool tx_error)
{
	rcb_t *state;

	state = wlc_scb_ratesel_get_cubby(wrsi, scb, rs_txs->ac);
	ASSERT(state);

	wlc_ratesel_upd_txs_ampdu(state, rs_txs, txs, tx_error);
}
#endif /* WLAMPDU_MAC */

/* update state upon received BA */
void BCMFASTPATH
wlc_scb_ratesel_upd_txs_blockack(wlc_ratesel_info_t *wrsi, struct scb *scb, tx_status_t *txs,
	uint8 suc_mpdu, uint8 tot_mpdu, bool ba_lost, uint8 retry, uint8 fb_lim, bool tx_error,
	uint8 mcs, bool sgi, uint8 antselid, uint8 ac)
{
	rcb_t *state;

	state = wlc_scb_ratesel_get_cubby(wrsi, scb, ac);
	ASSERT(state);

	wlc_ratesel_upd_txs_blockack(state, txs, suc_mpdu, tot_mpdu, ba_lost, retry, fb_lim,
		tx_error, mcs, sgi, antselid);
}
#endif /* WL11N */

bool
wlc_scb_ratesel_minrate(wlc_ratesel_info_t *wrsi, struct scb *scb, tx_status_t *txs, uint8 ac)
{
	rcb_t *state;

	state = wlc_scb_ratesel_get_cubby(wrsi, scb, ac);
	ASSERT(state);

	return (wlc_ratesel_minrate(state, txs));
}

static void
wlc_scb_ratesel_ppr_filter(wlc_info_t *wlc, ppr_rateset_t *clm_rates,
	wlc_rateset_t *scb_rates, bool scb_VHT)
{
	uint8 i;

	for (i = 0; i < PHYCORENUM(wlc->stf->op_txstreams); i++) {
		scb_rates->mcs[i] &= clm_rates->mcs[i];
#ifdef WL11AC
		if (WLCISACPHY(wlc->band) && scb_VHT) {
			/* check VHT 8_9 */
			uint16 clm_vht_rate = VHT_MCS_MAP_GET_MCS_PER_SS(i+1,
				clm_rates->vht_mcsmap);
			uint16 scb_vht_rate = VHT_MCS_MAP_GET_MCS_PER_SS(i+1,
				scb_rates->vht_mcsmap);

			if (scb_vht_rate != VHT_CAP_MCS_MAP_NONE) {
				if (clm_vht_rate == VHT_CAP_MCS_MAP_NONE) {
					VHT_MCS_MAP_SET_MCS_PER_SS(i+1, VHT_CAP_MCS_MAP_NONE,
						scb_rates->vht_mcsmap);
				} else if (scb_vht_rate > clm_vht_rate) {
					VHT_MCS_MAP_SET_MCS_PER_SS(i+1, clm_vht_rate,
						scb_rates->vht_mcsmap);
				}
#ifndef NO_PROPRIETARY_VHT_RATES
				/* check VHT 10_11 */
				clm_vht_rate =
					VHT_MCS_MAP_GET_MCS_PER_SS(i+1,
						clm_rates->vht_mcsmap_prop);

				/* Proprietary rates map can be either
				 * VHT_PROP_MCS_MAP_10_11 or VHT_CAP_MCS_MAP_NONE,
				 * if clm_vht_rate is set to VHT_PROP_MCS_MAP_10_11,
				 * no action required.
				 */
				if (clm_vht_rate == VHT_PROP_MCS_MAP_NONE) {
					VHT_MCS_MAP_SET_MCS_PER_SS(i+1,
						VHT_PROP_MCS_MAP_NONE,
						scb_rates->vht_mcsmap_prop);
				}
#endif /* !NO_PROPRIETARY_VHT_RATES */
			}
		}
#endif /* WL11AC */
	}
}

/* initialize per-scb state utilized by rate selection
 *   ATTEN: this fcn can be called to "reinit", avoid dup MALLOC
 *   this new design makes this function the single entry points for any select_rates changes
 *   this function should be called when any its parameters changed: like bw or stream
 *   this function will build select_rspec[] with all constraint and rateselection will
 *      be operating on this constant array with reference to known_rspec[] for threshold
 */

void
wlc_scb_ratesel_init(wlc_info_t *wlc, struct scb *scb)
{
	wlc_ratesel_info_t *wrsi = wlc->wrsi;
	rcb_t *state;
	uint8 bw = BW_20MHZ;
	int8 sgi_tx = OFF;
	int8 ldpc_tx = OFF;
	int8 vht_ldpc_tx = OFF;
	uint8 active_antcfg_num = 0;
	uint8 antselid_init = 0;
	int32 ac;
	uint *txc_ptr = NULL;
	wlc_rateset_t new_rateset;
	ppr_rateset_t *clm_rateset;
	uint8 i;

	if (SCB_INTERNAL(scb))
		return;
#ifdef WL11N
	if (WLANTSEL_ENAB(wlc))
		wlc_antsel_ratesel(wlc->asi, &active_antcfg_num, &antselid_init);

	bw = wlc_scb_link_bw_update(wlc, scb);

	if (wlc->stf->ldpc_tx == AUTO) {
		if (bw < BW_80MHZ && SCB_LDPC_CAP(scb))
			ldpc_tx = AUTO;
#ifdef WL11AC
		if (SCB_VHT_LDPC_CAP(wlc->vhti, scb))
			vht_ldpc_tx = AUTO;
#endif /* WL11AC */
	} else if (wlc->stf->ldpc_tx == ON) {
		if (SCB_LDPC_CAP(scb))
			ldpc_tx = ON;
		if (SCB_VHT_LDPC_CAP(wlc->vhti, scb))
			vht_ldpc_tx = ON;
	}

	if (WLC_HT_GET_SGI_TX(wlc->hti) == AUTO) {
		if (scb->flags2 & SCB2_SGI20_CAP)
			sgi_tx |= SGI_BW20;
		if (bw >= BW_40MHZ && (scb->flags2 & SCB2_SGI40_CAP))
			sgi_tx |= SGI_BW40;
		if (bw >= BW_80MHZ && SCB_VHT_SGI80(wlc->vhti, scb))
			sgi_tx |= SGI_BW80;
		if (bw >= BW_160MHZ && SCB_VHT_SGI160(wlc->vhti, scb))
			sgi_tx |= SGI_BW160;
		/* Disable SGI Tx in 20MHz on IPA chips */
		if (bw == BW_20MHZ && wlc->stf->ipaon)
			sgi_tx = OFF;
	}
#endif /* WL11N */

#ifdef WL11AC
	/* Set up the mcsmap in scb->rateset.vht_mcsmap */
	if (SCB_VHT_CAP(scb))
	{
		uint8 streams = wlc->stf->txstream_value;
		WL_RATE(("wl%d: %s txstream_value=%d\n",
				wlc->pub->unit, __FUNCTION__, streams));
		if (streams == 0) {
			wlc_vht_upd_rate_mcsmap(wlc->vhti, scb);
		} else  {
			uint16 vht_mcsmaps[] = { VHT_MCSMAP_0,
				VHT_MCSMAP_1, VHT_MCSMAP_2, VHT_MCSMAP_3, VHT_MCSMAP_4,
				VHT_MCSMAP_5, VHT_MCSMAP_6, VHT_MCSMAP_7, VHT_MCSMAP_8
			};
			uint8 num_map = sizeof(vht_mcsmaps)/sizeof(vht_mcsmaps[0]);
			if (streams < num_map && streams <= MAX_STREAMS_SUPPORTED) {
				scb->rateset.vht_mcsmap = vht_mcsmaps[streams];
			} else {
				WL_ERROR(("wl%d: %s invalid txstream_value %d\n",
						wlc->pub->unit, __FUNCTION__, streams));
				scb->rateset.vht_mcsmap = vht_mcsmaps[0]; // implies no vht stream
				// ASSERT(0);
			}
		}
	}
#endif /* WL11AC */

	/* HT rate overide for BTCOEX */
	if ((SCB_HT_CAP(scb) && wlc->stf->txstream_value)) {
		for (i = 1; i < 4; i++) {
			if (i >= wlc->stf->txstream_value) {
				scb->rateset.mcs[i] = 0;
			}
		}
#if defined(WLPROPRIETARY_11N_RATES)
		for (i = WLC_11N_FIRST_PROP_MCS; i <= WLC_MAXMCS; i++) {
			if (GET_PROPRIETARY_11N_MCS_NSS(i) > wlc->stf->txstream_value)
				clrbit(scb->rateset.mcs, i);
		}
#endif /* WLPROPRIETARY_11N_RATES */
	}

	for (ac = 0; ac < WME_MAX_AC(wlc, scb); ac++) {
		uint8 vht_ratemask = 0;
		uint32 max_rate;
		state = SCB_RATESEL_CUBBY(wrsi, scb, ac);

		if (state == NULL) {
			ASSERT(0);
			return;
		}

		bcopy(&scb->rateset, &new_rateset, sizeof(wlc_rateset_t));

#ifdef WL11N
		if (BSS_N_ENAB(wlc, scb->bsscfg)) {
			if (((WLC_HT_GET_SCB_MIMOPS_ENAB(wlc->hti, scb) &&
				!WLC_HT_GET_SCB_MIMOPS_RTS_ENAB(wlc->hti, scb)) ||
				(wlc->stf->op_txstreams == 1) || (wlc->stf->siso_tx == 1))) {
				new_rateset.mcs[1] = 0;
				new_rateset.mcs[2] = 0;
			} else if (wlc->stf->op_txstreams == 2)
				new_rateset.mcs[2] = 0;
		}
#endif // endif

#ifdef WL11AC
		vht_ratemask = wlc_vht_get_scb_ratemask_per_band(wlc->vhti, scb);
#endif // endif
		WL_RATE(("%s: scb 0x%p ac %d state 0x%p bw %s op_txstreams %d"
			" active_ant %d band %d vht:%u vht_rm:0x%x\n",
			__FUNCTION__, scb, ac, state, (bw == BW_20MHZ) ?
			"20" : ((bw == BW_40MHZ) ? "40" :
			(bw == BW_80MHZ) ? "80" : "80+80/160"),
			wlc->stf->op_txstreams, active_antcfg_num,
			wlc->band->bandtype, SCB_VHT_CAP(scb), vht_ratemask));

		max_rate = 0;
#if defined(WME_PER_AC_TX_PARAMS)
		if (WME_PER_AC_MAXRATE_ENAB(wrsi->pub) && SCB_WME(scb))
			max_rate = (uint32)wrsi->wlc->wme_max_rate[ac];
#endif // endif
		if (WLCISACPHY(wlc->band)) {
			BCM_REFERENCE(clm_rateset);
			/* XXX The check below is only a temp hack to avoid rates to be excluded on
			 * certain boards.
			 */
			if (!((ACREV_IS(wlc->band->phyrev, 32) ||
				ACREV_IS(wlc->band->phyrev, 33)) &&
				(wlc->pub->boardflags2 & BFL2_TXPWRCTRL_EN) == 0)) {
				clm_rateset =
					wlc_scb_ratesel_get_ppr_rates(wlc, rspecbw_to_bcmbw(bw));
				if (clm_rateset)
					wlc_scb_ratesel_ppr_filter(wlc, clm_rateset, &new_rateset,
						SCB_VHT_CAP(scb));
			}
		}

		if (WLC_TXC_ENAB(wlc))
			txc_ptr = wlc_txc_inv_ptr(wlc->txc, scb);

		wlc_ratesel_init(wrsi->rsi, state, scb, txc_ptr, &new_rateset, bw, sgi_tx,
			ldpc_tx, vht_ldpc_tx, vht_ratemask, active_antcfg_num, antselid_init,
			max_rate, 0);
	}

#ifdef WL_LPC
	wlc_scb_lpc_init(wlc->wlpci, scb);
#endif // endif
}

void
wlc_scb_ratesel_init_all(wlc_info_t *wlc)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACHSCB(wlc->scbstate, &scbiter, scb)
		wlc_scb_ratesel_init(wlc, scb);

#ifdef WL_LPC
	wlc_scb_lpc_init_all(wlc->wlpci);
#endif // endif
}

void
wlc_scb_ratesel_init_bss(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	struct scb *scb;
	struct scb_iter scbiter;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
		wlc_scb_ratesel_init(wlc, scb);
	}
#ifdef WL_LPC
	wlc_scb_lpc_init_bss(wlc->wlpci, cfg);
#endif // endif
}

void
wlc_scb_ratesel_rfbr(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 ac)
{
	rcb_t *state;

	state = wlc_scb_ratesel_get_cubby(wrsi, scb, ac);
	ASSERT(state);

	wlc_ratesel_rfbr(state);
}

void
wlc_scb_ratesel_rfbr_bss(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	struct scb *scb;
	struct scb_iter scbiter;
	rcb_t *state;
	int32 ac;
	wlc_ratesel_info_t *wrsi = wlc->wrsi;

	FOREACH_BSS_SCB(wlc->scbstate, &scbiter, cfg, scb) {
		for (ac = 0; ac < WME_MAX_AC(wlc, scb); ac++) {
			state = SCB_RATESEL_CUBBY(wrsi, scb, ac);
			ASSERT(state);

			wlc_ratesel_rfbr(state);
		}
	}
}

static int wlc_scb_ratesel_cubby_sz(void)
{
	return (sizeof(struct ratesel_cubby));
}

#ifdef WL11N
void wlc_scb_ratesel_rssi_enable(rssi_ctx_t *ctx)
{
	struct scb *scb = (struct scb *)ctx;

	scb->rssi_enabled++;
}

void wlc_scb_ratesel_rssi_disable(rssi_ctx_t *ctx)
{
	struct scb *scb = (struct scb *)ctx;

	scb->rssi_enabled--;
}

int wlc_scb_ratesel_get_rssi(rssi_ctx_t *ctx)
{
	struct scb *scb = (struct scb *)ctx;

	if (BSSCFG_STA(scb->bsscfg))
		return scb->bsscfg->link->rssi;
#if defined(AP) || defined(WLTDLS) || defined(WLAWDL)
	if (scb->rssi_enabled <= 0)
		WL_ERROR(("%s: scb %p rssi_enabled %d\n",
			__FUNCTION__, scb, scb->rssi_enabled));
	ASSERT(scb->rssi_enabled > 0);
	return wlc_scb_rssi(scb);
#endif // endif
	return 0;
}
#endif /* WL11N */

#ifdef WL_LPC
/* External functions */
void
wlc_scb_ratesel_get_info(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 ac,
	uint8 rate_stab_thresh, uint32 *new_rate_kbps, bool *rate_stable,
	rate_lcb_info_t *lcb_info)
{
	rcb_t *state = wlc_scb_ratesel_get_cubby(wrsi, scb, ac);
	wlc_ratesel_get_info(state, rate_stab_thresh, new_rate_kbps, rate_stable, lcb_info);
	return;
}

void
wlc_scb_ratesel_reset_vals(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 ac)
{
	rcb_t *state = NULL;

	if (!scb)
		return;

	state = SCB_RATESEL_CUBBY(wrsi, scb, ac);
	wlc_ratesel_lpc_init(state);
	return;
}

void
wlc_scb_ratesel_clr_cache(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 ac)
{
	rcb_t *state = wlc_scb_ratesel_get_cubby(wrsi, scb, ac);
	wlc_ratesel_clr_cache(state);
	return;
}
#endif /* WL_LPC */

/* Get current CLM enabled rates bitmap */
static ppr_rateset_t *
wlc_scb_ratesel_get_ppr_rates(wlc_info_t *wlc, wl_tx_bw_t bw)
{
	wlc_ratesel_info_t *wrsi = wlc->wrsi;
	if (wrsi->ppr_rates->chanspec != wlc->chanspec ||
		wrsi->ppr_rates->country != wlc_get_country(wlc) ||
		wrsi->ppr_rates->txstreams != wlc->stf->txstreams) {
		wlc_scb_ratesel_ppr_upd(wlc);
	}

	switch (bw) {
	case WL_TX_BW_20:
		return &wrsi->ppr_rates->ppr_20_rates;
#if defined(WL11AC) || defined(WL11N)
	case WL_TX_BW_40:
		return &wrsi->ppr_rates->ppr_40_rates;
#endif // endif
#ifdef WL11AC
	case WL_TX_BW_80:
		return &wrsi->ppr_rates->ppr_80_rates;
	case WL_TX_BW_160:
		return &wrsi->ppr_rates->ppr_160_rates;
#endif // endif
	default:
		ASSERT(0);
		return NULL;
	}
}

static void
wlc_scb_ratesel_get_ppr_rates_bitmp(wlc_info_t *wlc, ppr_t *target_pwrs, wl_tx_bw_t bw,
	ppr_rateset_t *rates)
{
	uint8 chain;
	ppr_vht_mcs_rateset_t mcs_limits;

	rates->vht_mcsmap = 0xff; /* No VHT rates support by default */
	rates->vht_mcsmap_prop = VHT_PROP_MCS_MAP_NONE_ALL;
	for (chain = 0; chain < PHYCORENUM(wlc->stf->op_txstreams); chain++) {
		ppr_get_vht_mcs(target_pwrs, bw, chain+1, WL_TX_MODE_NONE, chain+1, &mcs_limits);
		if (mcs_limits.pwr[0] != WL_RATE_DISABLED) {
			rates->mcs[chain] = 0xff; /* All rates are enabled for this block */
			/* Check VHT rate [8-9] */
#ifdef WL11AC
			if (WLCISACPHY(wlc->band)) {
				if (mcs_limits.pwr[9] != WL_RATE_DISABLED) {
					/* All VHT rates are enabled */
					VHT_MCS_MAP_SET_MCS_PER_SS(chain+1, VHT_CAP_MCS_MAP_0_9,
						rates->vht_mcsmap);
				} else if (mcs_limits.pwr[8] != WL_RATE_DISABLED) {
					/* VHT 0_8 are enabled */
					VHT_MCS_MAP_SET_MCS_PER_SS(chain+1, VHT_CAP_MCS_MAP_0_8,
						rates->vht_mcsmap);
				} else {
					/* VHT 8-9 are disabled in this case */
					VHT_MCS_MAP_SET_MCS_PER_SS(chain+1, VHT_CAP_MCS_MAP_0_7,
						rates->vht_mcsmap);
				}
#ifndef NO_PROPRIETARY_VHT_RATES
				/* Check VHT 10_11 */
				if (mcs_limits.pwr[11] != WL_RATE_DISABLED) {
					/* Both VHT 10_11 are enabled */
					VHT_MCS_MAP_SET_MCS_PER_SS(chain+1,
						VHT_PROP_MCS_MAP_10_11,
						rates->vht_mcsmap_prop);
				} else {
					/* VHT 10_11 are disabled in this case */
					VHT_MCS_MAP_SET_MCS_PER_SS(chain+1,
						VHT_CAP_MCS_MAP_NONE,
						rates->vht_mcsmap_prop);
				}
#endif /* !NO_PROPRIETARY_VHT_RATES */
			}
#endif /* WL11AC */
		}
	}

}

static void
wlc_scb_ratesel_ppr_updbmp(wlc_info_t *wlc, ppr_t *target_pwrs)
{
	wlc_ratesel_info_t *wrsi = wlc->wrsi;
	wlc_scb_ratesel_get_ppr_rates_bitmp(wlc, target_pwrs, WL_TX_BW_20,
		&wrsi->ppr_rates->ppr_20_rates);
#if defined(WL11N) || defined(WL11AC)
	wlc_scb_ratesel_get_ppr_rates_bitmp(wlc, target_pwrs, WL_TX_BW_40,
		&wrsi->ppr_rates->ppr_40_rates);
#endif // endif
#if defined(WL11AC)
	wlc_scb_ratesel_get_ppr_rates_bitmp(wlc, target_pwrs, WL_TX_BW_80,
		&wrsi->ppr_rates->ppr_80_rates);
	wlc_scb_ratesel_get_ppr_rates_bitmp(wlc, target_pwrs, WL_TX_BW_160,
		&wrsi->ppr_rates->ppr_160_rates);
#endif // endif
}

/* Update ppr enabled rates bitmap */
extern void
wlc_scb_ratesel_ppr_upd(wlc_info_t *wlc)
{
	phy_tx_power_t power;
	wl_tx_bw_t ppr_bw;
	ppr_t* reg_limits = NULL;

	wlc_cm_info_t *wlc_cm = wlc->cmi;
	clm_country_t country = wlc_get_country(wlc);

	bzero(&power, sizeof(power));

	ppr_bw = ppr_get_max_bw();
	if ((power.ppr_target_powers = ppr_create(wlc->osh, ppr_bw)) == NULL) {
		goto free_power;
	}
	if ((power.ppr_board_limits = ppr_create(wlc->osh, ppr_bw)) == NULL) {
		goto free_power;
	}
	if ((reg_limits = ppr_create(wlc->osh, PPR_CHSPEC_BW(wlc->chanspec))) == NULL) {
		goto free_power;
	}
	wlc_channel_reg_limits(wlc_cm, wlc->chanspec, reg_limits);

	wlc_phy_txpower_get_current(WLC_PI(wlc), reg_limits, &power);

	/* update the rate bitmap with retrieved target power */
	wlc_scb_ratesel_ppr_updbmp(wlc, power.ppr_target_powers);

	wlc->wrsi->ppr_rates->country = country;
	wlc->wrsi->ppr_rates->chanspec = wlc->chanspec;
	wlc->wrsi->ppr_rates->txstreams = wlc->stf->txstreams;
free_power:
	if (power.ppr_board_limits)
		ppr_delete(wlc->osh, power.ppr_board_limits);
	if (power.ppr_target_powers)
		ppr_delete(wlc->osh, power.ppr_target_powers);
	if (reg_limits)
		ppr_delete(wlc->osh, reg_limits);
}

static wl_tx_bw_t rspecbw_to_bcmbw(uint8 bw)
{
	wl_tx_bw_t bcmbw = WL_TX_BW_20;
	switch (bw) {
	case BW_40MHZ:
		bcmbw = WL_TX_BW_40;
		break;
	case BW_80MHZ:
		bcmbw = WL_TX_BW_80;
		break;
	case BW_160MHZ:
		bcmbw = WL_TX_BW_160;
		break;
	}

	return bcmbw;
}

void
wlc_scb_ratesel_get_ratecap(wlc_ratesel_info_t *wrsi, struct scb *scb, uint8 *sgi,
	uint16 mcs_bitmap[], uint8 ac)
{
	rcb_t *state = wlc_scb_ratesel_get_cubby(wrsi, scb, ac);

	wlc_ratesel_get_ratecap(state, sgi, mcs_bitmap);

	return;
}
