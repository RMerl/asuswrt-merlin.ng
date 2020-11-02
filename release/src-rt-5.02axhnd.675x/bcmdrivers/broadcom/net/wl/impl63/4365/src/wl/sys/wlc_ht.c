/*
 * Common (OS-independent) portion of
 * Broadcom 802.11 Networking Device Driver
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
 * $Id: wlc_ht.c 779278 2019-09-24 07:23:39Z $
 */

/** 802.11n (High Throughput) */

#include <wlc_cfg.h>
#include <wlc_types.h>
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <siutils.h>
#include <wlioctl.h>
#include <wlc_pub.h>
#include <d11.h>
#include <wlc_bsscfg.h>
#include <wlc_rate.h>
#include <wlc.h>
#include <wlc_ht.h>

#ifdef WL11N
#include <wlc_ie_misc_hndlrs.h>
#include <wlc_prot_n.h>
#include <wlc_scb.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_reg.h>

#include <wlc_pcb.h>
#include <wlc_vht.h>
#include <wlc_csa.h>
#include <wlc_scb_ratesel.h>
#include <wlc_txbf.h>
#include <wlc_ampdu_cmn.h>
#include <wlc_amsdu.h>

#include <bcmdevs.h>
#include <wlc_bmac.h>
#include <wlc_ht.h>
#include <wlc_stf.h>
#include <wlc_obss.h>
#include <wlc_amsdu.h>
#ifdef WLC_HIGH_ONLY
#include <bcm_rpc_tp.h>
#include <bcm_rpc.h>
#include <bcm_xdr.h>
#include <wlc_rpc.h>
#include <wlc_rpctx.h>
#endif /* WLC_HIGH_ONLY */
#ifdef WLTDLS
#include <wlc_tdls.h>
#endif // endif
#include <wlc_ulb.h>

#include <wlc_tx.h>
#include <wlc_txc.h>
#include <wlc_ap.h>

/* IE mgmt */
static uint wlc_ht_calc_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ht_write_cap_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_ht_calc_op_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ht_write_op_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_ht_calc_brcm_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ht_write_brcm_cap_ie(void *ctx, wlc_iem_build_data_t *data);
#ifdef AP
static int wlc_ht_parse_cap_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif // endif
static int wlc_ht_scan_parse_cap_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_ht_scan_parse_op_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_ht_scan_parse_brcm_ie(void *ctx, wlc_iem_parse_data_t *data);
#ifdef WLTDLS
static uint wlc_ht_tdls_calc_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ht_tdls_write_cap_ie(void *ctx, wlc_iem_build_data_t *data);
#ifdef IEEE2012_TDLSSEPC
static uint wlc_ht_tdls_calc_op_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_ht_tdls_write_op_ie(void *ctx, wlc_iem_build_data_t *data);
#endif // endif
#endif /* AP */
#ifdef WLTDLS
static int wlc_ht_tdls_parse_cap_ie(void *ctx, wlc_iem_parse_data_t *parse);
#ifdef IEEE2012_TDLSSEPC
static int wlc_ht_tdls_parse_op_ie(void *ctx, wlc_iem_parse_data_t *parse);
#endif /* IEEE2012_TDLSSEPC */
#endif /* WLTDLS */

static int wlc_ht_scb_init(void *context, struct scb *scb);
static void wlc_ht_scb_dump(void *ctx, struct scb *scb, struct bcmstrbuf *b);

static int wlc_ht_bss_init(void *context, wlc_bsscfg_t *cfg);
static void wlc_ht_bss_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b);

static int wlc_setup_nmode(wlc_info_t *wlc, int nmode);
static void wlc_mimops_action_ht_complete(wlc_info_t *wlc, uint txstatus, void *arg);
static void wlc_ht_update_sgi_rx(wlc_ht_info_t *hti, int val);
static void wlc_ht_update_ldpc(wlc_ht_info_t *hti, int8 val);
static uint16 wlc_ht_phy_get_rate(wlc_ht_info_t *pub, uint8 htflags, uint8 mcs);
static void wlc_frameburst_size(wlc_info_t *wlc);
static void wlc_ht_frameburst_txop_set(wlc_ht_info_t *pub, uint16 val);
static uint16 wlc_ht_frameburst_txop_get(wlc_ht_info_t *pub);
static void *
wlc_send_action_ht_mimops(wlc_ht_info_t *hti, wlc_bsscfg_t *bsscfg, uint8 mimops_mode);

static int
wlc_update_bwcap(wlc_ht_info_t *hti, int bandtype, uint8 bwcap);
static void wlc_ht_mimops_cap_update(wlc_ht_info_t *hti, uint8 mimops_mode);

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_dump_htcap(wlc_ht_info_t *hti, struct bcmstrbuf *b);
#endif /* BCMDBG || BCMDBG_DUMP */

static void
wlc_update_mimo_band_bwcap(wlc_info_t *wlc, uint8 bwcap);

#ifdef STA
static void wlc_ht_wdog(void *ctx);
static void
wlc_ht_mimops_action(wlc_ht_info_t *hti, wlc_bsscfg_t *cfg);
#else
/* No need for timer otherwise */
#define wlc_ht_wdog (NULL)
#endif /* STA */

static void
wlc_ht_set_rifs_mode(wlc_ht_info_t *pub, wlc_bsscfg_t *cfg, int32 mode);

static void
wlc_ht_update_cfg_rifs_mode(wlc_ht_info_t *pub, wlc_bsscfg_t *cfg);

static void
wlc_ht_set_txburst_limit(wlc_ht_info_t *hti, wlc_bsscfg_t *cfg, uint16 v);

static void
wlc_ht_update_cfg_txburst_limit(wlc_ht_info_t *pub, wlc_bsscfg_t *cfg);

static int8 wlc_ht_mimops_get(wlc_ht_info_t *pub);
static bool wlc_ht_mimops_set(wlc_ht_info_t *pub, wlc_bsscfg_t *cfg, int32 int_val);
static void
wlc_ht_set_cap_params(wlc_ht_info_t *pub, uint8 p);

static uint8
wlc_ht_get_cap_params(wlc_ht_info_t *pub);

static void
wlc_mimops_action_ht_send(wlc_ht_info_t *pub, wlc_bsscfg_t *bsscfg, uint8 mimops_mode);

static void wlc_ht_set_cap(wlc_ht_info_t *hti, uint16 p);

#ifdef BCMDBG
static void
wlc_get_mcsset(wlc_ht_info_t *hti, void *arg, bool istx);
#endif /* BCMDBG */

static uint16 wlc_ht_get_scb_cap(wlc_ht_info_t *hti, struct scb *scb);
static void wlc_ht_set_cfg_mimops_PM(wlc_ht_info_t *hti, wlc_bsscfg_t *cfg, uint8 v);
static int32 wlc_ht_get_rifs_advert(wlc_ht_info_t *hti);
#if WL_HT_TXBW_OVERRIDE_ENAB
static void wlc_ht_update_txbw_override(wlc_ht_info_t *pub);
#endif /* WL_HT_TXBW_OVERRIDE_ENAB */

typedef struct wlc_ht_priv_info {
	wlc_info_t *wlc;
	uint8		mimo_band_bwcap;	/* bw cap per band type */
	bool		mimo_mixedmode;		/* mimo preamble type */
	ht_cap_ie_t	cap_ie;			/* HT CAP IE being advertised by this node */
	ht_add_ie_t	add_ie;			/* HT ADD IE being used by this node */
	uint8	ht_wsec_restriction;	/* the restriction of HT with TKIP or WEP */
	uint8	htphy_membership;		/* HT PHY membership */
	int32	rifs_advert; /* RIFS mode advertisement */
	uint16	txburst_limit;		/* global tx burst limit value */
	/* *_priv_offset - avoid rom invalid by dir refs to priv, use offset */
	int			scb_priv_offset;
	int 		bss_priv_offset;
} wlc_ht_priv_info_t;

typedef struct {
	wlc_ht_info_t pub;
	wlc_ht_priv_info_t priv;
} wlc_ht_t;

/* priv_t offset in module states */
static uint16 wlc_ht_info_priv_offset = sizeof(wlc_ht_info_t);

/* module states size */
#define WLC_HT_SIZE	(sizeof(wlc_ht_t))
/* moudle states location */
#define WLC_HT_INFO_PRIV(ht) ((wlc_ht_priv_info_t *) \
				    ((uint8*)(ht) + wlc_ht_info_priv_offset))

typedef struct wlc_ht_scb_info_priv {
	wlc_info_t *wlc;
	uint16		amsdu_ht_mtu_pref;	/* preferred HT AMSDU mtu in bytes */
	uint16		ht_capabilities;	/* current advertised capability set */
	uint8		ht_ampdu_params;	/* current adverised AMPDU config */
#if defined(BCMDBG) || defined(BCMDBG_DUMP) || defined(DONGLEBUILD)
	uint8		rclen;			/* regulatory class length */
	uint8		rclist[MAXRCLISTSIZE];	/* regulatory class list */
#endif /* BCMDBG || BCMDBG_DUMP */
} wlc_ht_scb_info_priv_t;

typedef struct wlc_ht_scb_cubby {
	wlc_ht_scb_info_pub_t pub;
	wlc_ht_scb_info_priv_t priv;
} wlc_ht_scb_cubby_t;

#define WLC_HT_SCB_CUBBY_SIZE (sizeof(wlc_ht_scb_cubby_t))

#define SCB_HT_CUBBY(ht, scb) \
	((wlc_ht_scb_cubby_t *)SCB_CUBBY((scb), (ht)->scbh))
#define SCB_HT_INFO_PRIV(ht, scb) ((wlc_ht_scb_info_priv_t *) \
	(((uint8*)SCB_HT_CUBBY((ht), (scb))) + \
	WLC_HT_INFO_PRIV((ht))->scb_priv_offset))
#define SCB_HT_INFO_PUB(ht, scb) ((wlc_ht_scb_info_pub_t *) \
	&(SCB_HT_CUBBY((ht), (scb))->pub))

typedef struct wlc_ht_bss_info_priv {
	/* SM PS */
	uint8	mimops_PM;
	uint8	mimops_ActionPM;
	uint8  	mimops_ActionRetry;
	bool    mimops_ActionPending;

	int32	rifs_mode;	 /* RIFS mode in the HT Info IE */
} wlc_ht_bss_info_priv_t;

typedef struct wlc_ht_bss_cubby {
	wlc_ht_bss_info_pub_t pub;
	wlc_ht_bss_info_priv_t priv;
} wlc_ht_bss_cubby_t;

#define WLC_HT_BSS_CUBBY_SIZE (sizeof(wlc_ht_bss_cubby_t))
#define BSS_HT_CUBBY(hti, cfg) \
	((wlc_ht_bss_cubby_t*)BSSCFG_CUBBY((cfg), (hti->bssh)))
#define BSS_HT_INFO_PRIV(hti, cfg) \
	((wlc_ht_bss_info_priv_t*)((uint8*)BSS_HT_CUBBY((hti), (cfg)) + \
	WLC_HT_INFO_PRIV((hti))->bss_priv_offset))
#define BSS_HT_INFO_PUB(hti, cfg) \
	((wlc_ht_bss_info_pub_t*)&(BSS_HT_CUBBY((hti), (cfg))->pub))

static int
wlc_ht_doiovar(void *context, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int alen, int vsize, struct wlc_if *wlcif);
static int
_hti_ioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif);

enum {
	IOV_NMODE = 0,  /* enable/disable support for 802.11N */
	IOV_CURR_MCSSET = 3,
	IOV_MIMO_TXBW = 4,
	IOV_AGGDBG = 5,
	IOV_NREQD = 10,
	IOV_LDPC_TX = 19,
	IOV_TXBURST_LIM = 22,
	IOV_HTPHY_MEMBERSHIP = 23,
	IOV_HT_WSEC_RESTRICT = 24,
	IOV_TXBURST_LIM_OVERRIDE = 25, /* set the advertised tx burst limit */
	IOV_CCK_TXBW = 26,		/* 11N, cck tx b/w override */
	IOV_OFDM_TXBW = 27,		/* 11N, ofdm tx b/w override */
	IOV_NRATE = 29,		/* legacy rate/mcs and stf mode */
	IOV_RIFS = 30,		/* MIMO, RIFS */
	IOV_RIFS_ADVERT = 31,	/* MIMO, RIFS mode advertisement */
	IOV_SGI_RX = 32, 	/* MIMO, SGI RX */
	IOV_SGI_TX = 33, 	/* MIMO, SGI TX */
	IOV_BW_CAP = 34, 	/* Set per-band bandwidth cap */
	IOV_MIMO_BW_CAP = 35,	/* set the advertised ch widths (deprecated) */
	IOV_GF_CAP = 38, 	/* get/set Green Field Cap bit in HT cap IE */
	IOV_LDPC_CAP = 39,		/* HTPHY, enable/disable LDPC RX */
	IOV_TXMCSSET = 40,
	IOV_RXMCSSET = 41,
	IOV_AMPDU_RTS = 42,
	IOV_HT_MIMOPS = 43,
	IOV_HT_AMSDU_RXMAX = 44,
	IOV_FBTXOPTHRESH = 45,	/* maximum txop limit for frameburst in usec */

	IOV_HTLAST
};

static const bcm_iovar_t ht_iovars[] = {
	{"amsdu_rxmax", IOV_HT_AMSDU_RXMAX,
	(IOVF_SET_DOWN), IOVT_BOOL, 0
	},
	{"mimo_ps", IOV_HT_MIMOPS,
	(IOVF_OPEN_ALLOW), IOVT_UINT8, 0
	},
#ifdef BCMDBG
	{"txmcsset", IOV_TXMCSSET,
	(0), IOVT_BUFFER, 0
	},
	{"rxmcsset", IOV_RXMCSSET,
	(0), IOVT_BUFFER, 0
	},
#endif /* BCMDBG */
	{"nrate", IOV_NRATE,
	(IOVF_OPEN_ALLOW), IOVT_UINT32, 0
	},
	{"nmode", IOV_NMODE,
	(IOVF_SET_DOWN|IOVF_OPEN_ALLOW|IOVF_RSDB_SET), IOVT_UINT32, 0
	},
	{"cur_mcsset", IOV_CURR_MCSSET,
	(IOVF_OPEN_ALLOW), IOVT_BUFFER, MCSSET_LEN
	},
#if defined(BCMDBG) || defined(WLTEST)
	{"mimo_txbw", IOV_MIMO_TXBW,
	(IOVF_MFG), IOVT_INT32, 0
	},
	{"ofdm_txbw", IOV_OFDM_TXBW,
	(0), IOVT_INT32, 0
	},
#endif /* BCMDBG || WLTEST */
#if defined(BCMDBG)
	{"aggdbg", IOV_AGGDBG,
	(0), IOVT_INT32, 0
	},
#endif // endif
	{"nreqd", IOV_NREQD,
	(IOVF_SET_DOWN|IOVF_OPEN_ALLOW), IOVT_UINT32, 0
	},
	{"bw_cap", IOV_BW_CAP,
	(IOVF_SET_DOWN|IOVF_OPEN_ALLOW), IOVT_BUFFER, 2*sizeof(uint32)
	},
	{"mimo_bw_cap", IOV_MIMO_BW_CAP,
	(IOVF_SET_DOWN|IOVF_OPEN_ALLOW), IOVT_UINT8, 0
	},
	{"rifs", IOV_RIFS,
	(IOVF_OPEN_ALLOW), IOVT_BOOL, 0
	},
	{"rifs_advert", IOV_RIFS_ADVERT,
	(IOVF_OPEN_ALLOW), IOVT_INT32, 0
	},
	{"sgi_rx", IOV_SGI_RX,
	(IOVF_OPEN_ALLOW), IOVT_UINT8, 0
	},
	{"sgi_tx", IOV_SGI_TX,
	(IOVF_OPEN_ALLOW), IOVT_INT8, 0
	},
	{"gf_cap", IOV_GF_CAP,
	(0), IOVT_BOOL, 0
	},
	{"ldpc_cap", IOV_LDPC_CAP,
	(0), IOVT_INT32, 0
	},
	{"ldpc_tx", IOV_LDPC_TX,
	(0), IOVT_INT32, 0
	},
#ifdef BCMDBG
	{"cck_txbw", IOV_CCK_TXBW,
	(0), IOVT_INT32, 0
	},
	{"txburst_limit", IOV_TXBURST_LIM,
	(0), IOVT_INT32, 0
	},
	{"htphy_membership", IOV_HTPHY_MEMBERSHIP,
	(IOVF_SET_DOWN), IOVT_BOOL, 0
	},
#endif /* BCMDBG */
	{"ht_wsec_restrict", IOV_HT_WSEC_RESTRICT,
	(0), IOVT_INT32, 0,
	},
	{"ampdu_rts", IOV_AMPDU_RTS, (0), IOVT_BOOL, 0},
	{"frameburst_txop", IOV_FBTXOPTHRESH, (IOVF_SET_UP), IOVT_UINT16, 0},
	{NULL, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

static void
wlc_ht_update_sgi_rx(wlc_ht_info_t *pub, int val)
{
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);
	wlc_info_t *wlc = hti->wlc;

	hti->cap_ie.cap &= ~(HT_CAP_SHORT_GI_20 | HT_CAP_SHORT_GI_40);
	hti->cap_ie.cap |= (val & WLC_N_SGI_20) ? HT_CAP_SHORT_GI_20 : 0;
	hti->cap_ie.cap |= (val & WLC_N_SGI_40) ? HT_CAP_SHORT_GI_40 : 0;

	if (wlc->pub->up) {
		wlc_update_beacon(wlc);
		wlc_update_probe_resp(wlc, TRUE);
	}
}

static void
wlc_ht_update_ldpc(wlc_ht_info_t *pub, int8 val)
{
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	wlc_info_t *wlc = hti->wlc;
	wlc->stf->ldpc = val;
	wlc->stf->ldpc_tx = ((val == ON) ? AUTO : OFF);
	hti->cap_ie.cap &= ~HT_CAP_LDPC_CODING;
	wlc_vht_set_ldpc_cap(wlc->vhti, FALSE);
	if (wlc->stf->ldpc != OFF) {
		hti->cap_ie.cap |= HT_CAP_LDPC_CODING;
		wlc_vht_set_ldpc_cap(wlc->vhti, TRUE);
	}
	if (wlc->pub->up) {
		wlc_update_beacon(wlc);
		wlc_update_probe_resp(wlc, TRUE);
		wlc_phy_ldpc_override_set(WLC_PI(wlc), (val ? TRUE : FALSE));
	}
}

void BCMATTACHFN(wlc_ht_init_defaults)(wlc_ht_info_t *pub)
{
	wlc_ht_priv_info_t *hti;
	wlc_info_t *wlc;

	hti = WLC_HT_INFO_PRIV(pub);

	wlc = hti->wlc;
	hti->cap_ie.cap |= HT_CAP;
	if (WLC_LDPC_CAP_PHY(wlc)) {
		wlc->stf->ldpc = ON;
		wlc->stf->ldpc_tx = AUTO;
	}

#ifdef DONGLEBUILD
	hti->cap_ie.cap &= ~HT_CAP_MAX_AMSDU;
#endif // endif

	if (D11REV_GE(wlc->pub->corerev, 31) && D11REV_LE(wlc->pub->corerev, 38)) {
		hti->cap_ie.cap &= ~HT_CAP_MAX_AMSDU;
	}

	if (WLCISLCNPHY(wlc->band) && LCNREV_LE(wlc->band->phyrev, 2)) {
		hti->cap_ie.cap &= ~HT_CAP_40MHZ;
	}

	if (wlc->stf->hw_rxchain == 1) {
		hti->cap_ie.cap &= ~HT_CAP_MIMO_PS_MASK;
		hti->cap_ie.cap |= (HT_CAP_MIMO_PS_ON << HT_CAP_MIMO_PS_SHIFT);
	}

	if (CHIPID(wlc->pub->sih->chip) == BCM4313_CHIP_ID) {
		hti->cap_ie.cap &= ~HT_CAP_GF;
	}
#if WL_HT_TXBW_OVERRIDE_ENAB
	pub->mimo_40txbw = AUTO;
	pub->ofdm_40txbw = AUTO;
	pub->cck_40txbw = AUTO;
	wlc_ht_update_txbw_override(pub);
#endif /* WL_HT_TXBW_OVERRIDE_ENAB */

	{
		uint8 bw_cap = WLC_BW_CAP_20MHZ;

#ifdef WL11ULB
	if (ULB_ENAB(wlc->pub)) {
		bw_cap |= (WLC_2P5MHZ_ULB_SUPP_HW(wlc) ? WLC_BW_CAP_2P5MHZ : 0);
		bw_cap |= (WLC_5MHZ_ULB_SUPP_HW(wlc)   ? WLC_BW_CAP_5MHZ   : 0);
		bw_cap |= (WLC_10MHZ_ULB_SUPP_HW(wlc)  ? WLC_BW_CAP_10MHZ  : 0);
		}
#endif /* WL11ULB */

		wlc_update_bwcap(pub, WLC_BAND_2G, bw_cap);
	}
	wlc_update_bwcap(pub, WLC_BAND_5G, WLC_BW_CAP_UNRESTRICTED);

	/* Enable setting the RIFS Mode bit by default in HT Info IE */
	hti->rifs_advert = AUTO;

	/* Set default values of SGI */
	if (WLC_SGI_CAP_PHY(wlc)) {
		if (WLCISLCNPHY(wlc->band))
			wlc_ht_update_sgi_rx(pub, WLC_N_SGI_20);
		else
			wlc_ht_update_sgi_rx(pub, (WLC_N_SGI_20 | WLC_N_SGI_40));
		pub->sgi_tx = AUTO;
	} else {
		wlc_ht_update_sgi_rx(pub, 0);
		pub->sgi_tx = OFF;
	}
#ifdef WLAMSDU
	if ((hti->cap_ie.cap & HT_CAP_MAX_AMSDU) && (WLC_PHY_11N_CAP(wlc->band))) {
		/* HW rcv fifo should be big enough */
		ASSERT(wlc_amsdu_mtu_get(wlc->ami) >= HT_MAX_AMSDU);
	}
#endif	/* WLAMSDU */

	/* init max burst txop (framebursting) */
	pub->max_fbtxop_user = MAXFRAMEBURST_TXOP;
}

static int
wlc_ht_up(void *ctx)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	/* ensure LDPC config is in sync */
	wlc_ht_update_ldpc(pub, hti->wlc->stf->ldpc);

	/* ensure country specific frameburst txop limits are honored */
	wlc_ht_frameburst_limit(pub);

	return BCME_OK;
}

wlc_ht_info_t *
BCMATTACHFN(wlc_ht_attach)(wlc_info_t *wlc)
{
	wlc_ht_info_t *hti;
	wlc_ht_priv_info_t *htip;

	uint16 capfstbmp =
	        FT2BMP(FC_BEACON) |
	        FT2BMP(FC_PROBE_RESP) |
#ifdef STA
	        FT2BMP(FC_ASSOC_REQ) |
	        FT2BMP(FC_REASSOC_REQ) |
#endif // endif
#ifdef AP
	        FT2BMP(FC_ASSOC_RESP) |
	        FT2BMP(FC_REASSOC_RESP) |
#endif // endif
	        FT2BMP(FC_PROBE_REQ) |
	        0;
	uint16 opfstbmp =
	        FT2BMP(FC_BEACON) |
	        FT2BMP(FC_PROBE_RESP) |
#ifdef AP
	        FT2BMP(FC_ASSOC_RESP) |
	        FT2BMP(FC_REASSOC_RESP) |
#endif // endif
	        0;

	uint16 brcmfstbmp =
#ifdef STA
	        FT2BMP(FC_ASSOC_REQ) |
	        FT2BMP(FC_REASSOC_REQ) |
#endif // endif
	        FT2BMP(FC_PROBE_REQ) |
	        0;
#ifdef AP
	uint16 cap_parse_fstbmp =
	        FT2BMP(FC_ASSOC_REQ) |
	        FT2BMP(FC_REASSOC_REQ) |
	        0;
#endif // endif
	uint16 scan_parse_fstbmp =
	        FT2BMP(WLC_IEM_FC_SCAN_BCN) |
	        FT2BMP(WLC_IEM_FC_SCAN_PRBRSP) |
	        0;

	/* allocate private states */
	if ((hti = MALLOCZ(wlc->osh, WLC_HT_SIZE)) == NULL) {
		goto fail;
	}
	/* offset must be set immediately after malloc (before WLC_HT_INFO_PRIV is used) */
	wlc_ht_info_priv_offset = OFFSETOF(wlc_ht_t, priv);

	htip = WLC_HT_INFO_PRIV(hti);
	htip->scb_priv_offset = OFFSETOF(wlc_ht_scb_cubby_t, priv);
	htip->bss_priv_offset = OFFSETOF(wlc_ht_bss_cubby_t, priv);
	htip->wlc = wlc;

	/* register IE mgmt callbacks */
	/* calc/build */
	/* bcn/prbrsp/assocreq/reassocreq/assocresp/reassocresp/prbreq */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, capfstbmp, DOT11_MNG_HT_CAP,
	      wlc_ht_calc_cap_ie_len, wlc_ht_write_cap_ie, hti) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, ht cap ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* bcn/prbrsp/assocresp/reassocresp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, opfstbmp, DOT11_MNG_HT_ADD,
	      wlc_ht_calc_op_ie_len, wlc_ht_write_op_ie, hti) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, ht op ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	/* assocreq/reassocreq/prbreq */
	if (wlc_iem_vs_add_build_fn_mft(wlc->iemi, brcmfstbmp, WLC_IEM_VS_IE_PRIO_BRCM_HT,
	      wlc_ht_calc_brcm_cap_ie_len, wlc_ht_write_brcm_cap_ie, hti) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_build_fn failed, brcm ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#ifdef WLTDLS
	/* tdlssetupreq */
	if (TDLS_SUPPORT(wlc->pub)) {
		if (wlc_ier_add_build_fn(wlc->ier_tdls_srq, DOT11_MNG_HT_CAP,
			wlc_ht_tdls_calc_cap_ie_len, wlc_ht_tdls_write_cap_ie, hti)
			!= BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, "
				"ht cap ie in tdls setupreq\n", wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
		/* tdlssetupresp */
		if (wlc_ier_add_build_fn(wlc->ier_tdls_srs, DOT11_MNG_HT_CAP,
			wlc_ht_tdls_calc_cap_ie_len, wlc_ht_tdls_write_cap_ie, hti) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, "
				"ht cap ie in tdls setupresp\n", wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
	/* tdlssetupconfirm */
#ifdef IEEE2012_TDLSSEPC
		if (wlc_ier_add_build_fn(wlc->ier_tdls_scf, DOT11_MNG_HT_ADD,
			wlc_ht_tdls_calc_op_ie_len, wlc_ht_tdls_write_op_ie, hti) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, "
				"ht op ie in tdls setupconfirm\n", wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
#else
		if (wlc_ier_add_build_fn(wlc->ier_tdls_scf, DOT11_MNG_HT_CAP,
			wlc_ht_tdls_calc_cap_ie_len, wlc_ht_tdls_write_cap_ie, hti) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, "
				"ht cap ie in tdls setupconfirm\n", wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
#endif // endif
		/* tdlsdiscresp */
		if (wlc_ier_add_build_fn(wlc->ier_tdls_drs, DOT11_MNG_HT_CAP,
			wlc_ht_tdls_calc_cap_ie_len, wlc_ht_tdls_write_cap_ie, hti) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_add_build_fn failed, "
				"ht cap ie in tdls discresp\n", wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
	}
#endif /* WLTDLS */
	/* parse */
#ifdef AP
	/* assocreq/reassocreq */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, cap_parse_fstbmp, DOT11_MNG_HT_CAP,
		wlc_ht_parse_cap_ie, hti) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, ht cap ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif
	/* bcn/prbrsp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, scan_parse_fstbmp, DOT11_MNG_HT_CAP,
	                             wlc_ht_scan_parse_cap_ie, hti) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, ht cap ie in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, scan_parse_fstbmp, DOT11_MNG_HT_ADD,
	                             wlc_ht_scan_parse_op_ie, hti) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, ht op ie in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	if (wlc_iem_vs_add_parse_fn_mft(wlc->iemi, scan_parse_fstbmp, WLC_IEM_VS_IE_PRIO_BRCM_HT,
	                                wlc_ht_scan_parse_brcm_ie, hti) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_vsadd_parse_fn failed, brcm ie in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#ifdef WLTDLS
	/* tdls */
	if (TDLS_SUPPORT(wlc->pub)) {
		if (wlc_ier_add_parse_fn(wlc->ier_tdls_srq, DOT11_MNG_HT_CAP,
			wlc_ht_tdls_parse_cap_ie, hti) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, "
				"ht cap ie in setupreq\n", wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
		if (wlc_ier_add_parse_fn(wlc->ier_tdls_srs, DOT11_MNG_HT_CAP,
			wlc_ht_tdls_parse_cap_ie, hti) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, "
				"ht cap ie in setupresp\n", wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
#ifdef IEEE2012_TDLSSEPC
		if (wlc_ier_add_parse_fn(wlc->ier_tdls_scf, DOT11_MNG_HT_ADD,
			wlc_ht_tdls_parse_op_ie, hti) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, "
				"ht op ie in setupconfirm\n", wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
#else
		if (wlc_ier_add_parse_fn(wlc->ier_tdls_scf, DOT11_MNG_HT_CAP,
			wlc_ht_tdls_parse_cap_ie, hti) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, "
				"ht cap ie in setupconfirm\n", wlc->pub->unit, __FUNCTION__));
			goto fail;
		}
#endif // endif
	}
#endif /* WLTDLS */
	/* register module up/down, watchdog, and iovar callbacks */
	if (wlc_module_register(wlc->pub, ht_iovars, "hti", hti, wlc_ht_doiovar,
	                        wlc_ht_wdog, wlc_ht_up, NULL)) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	/* register module IOCTL handlers */
	if (wlc_module_add_ioctl_fn(wlc->pub, hti, _hti_ioctl, 0, NULL)) {
		WL_ERROR(("wl%d: %s wlc_module_add_ioctl_fn() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	hti->scbh = wlc_scb_cubby_reserve(wlc, WLC_HT_SCB_CUBBY_SIZE,
		wlc_ht_scb_init, NULL, wlc_ht_scb_dump, (void *)hti);

	if (hti->scbh < 0) {
		WL_ERROR(("wl%d:%s: wlc_scb_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

	hti->bssh = wlc_bsscfg_cubby_reserve(wlc, WLC_HT_BSS_CUBBY_SIZE,
		wlc_ht_bss_init, NULL, wlc_ht_bss_dump, (void *)hti);

	if (hti->bssh < 0) {
		WL_ERROR(("wl%d:%s: wlc_bsscfg_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_register(wlc->pub, "htcap", (dump_fn_t)wlc_dump_htcap, (void *)hti);
#endif /* defined(BCMDBG) || defined(BCMDBG_DUMP) */

	hti->txburst_limit_override = AUTO;
	hti->ampdu_rts = TRUE;

	/* By default restrict TKIP and WEP associations from 11n HT rates */
	htip->ht_wsec_restriction = WLC_HT_TKIP_RESTRICT | WLC_HT_WEP_RESTRICT;
	htip->cap_ie.cap = 0;

	return hti;

fail:
	wlc_ht_detach(hti);
	return NULL;
} /* wlc_ht_attach */

void
BCMATTACHFN(wlc_ht_detach)(wlc_ht_info_t *pub)
{
	wlc_ht_priv_info_t *hti;
	wlc_info_t *wlc;

	if (pub == NULL)
		return;
	hti = WLC_HT_INFO_PRIV(pub);
	wlc = hti->wlc;

	wlc_module_remove_ioctl_fn(wlc->pub, pub);
	wlc_module_unregister(wlc->pub, "hti", pub);

	MFREE(wlc->osh, pub, WLC_HT_SIZE);
}

/* scb cubby init */
static int
wlc_ht_scb_init(void *context, struct scb *scb)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)context;
	wlc_ht_scb_cubby_t *ph = SCB_HT_CUBBY(pub, scb);
	bzero(ph, WLC_HT_SCB_CUBBY_SIZE);
	return BCME_OK;

}

static void
wlc_ht_scb_dump(void *ctx, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_scb_info_priv_t *cubby = SCB_HT_INFO_PRIV(pub, scb);
	wlc_ht_scb_info_pub_t *scb_pub = &(SCB_HT_CUBBY(pub, scb)->pub);

	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);
	BCM_REFERENCE(hti);

	if (scb->flags & SCB_AMSDUCAP) {
		bcm_bprintf(b, " AMSDU-MTU ht:%d vht:%d\n",
			cubby->amsdu_ht_mtu_pref,
			wlc_vht_get_scb_amsdu_mtu_pref(hti->wlc->vhti, scb));
	}

	if (SCB_HT_CAP(scb)) {
		wlc_dump_mcsset("     HT mcsset :", &scb->rateset.mcs[0], b);
		bcm_bprintf(b, "\n");
		bcm_bprintf(b,	"     HT capabilites 0x%04x ampdu_params 0x%02x "
			"mimops_enabled %d mimops_rtsmode %d",
			cubby->ht_capabilities,
			cubby->ht_ampdu_params,
			scb_pub->ht_mimops_enabled,
			scb_pub->ht_mimops_rtsmode);
		bcm_bprintf(b, "\n");
	}
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	wlc_dump_rclist("     rclist", cubby->rclist, cubby->rclen, b);
#endif /* BCMDBG || BCMDBG_DUMP || DONGLEBUILD */
}

/* bsscfg cubby init */
static int
wlc_ht_bss_init(void *context, wlc_bsscfg_t *cfg)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)context;
	wlc_ht_bss_cubby_t* ph = BSS_HT_CUBBY(pub, cfg);
	bzero(ph, WLC_HT_BSS_CUBBY_SIZE);
	BSS_HT_INFO_PRIV(pub, cfg)->rifs_mode = 0;
	wlc_ht_update_rifs_mode(pub, cfg);
	wlc_ht_update_txburst_limit(pub, cfg);

	return BCME_OK;
}

static void
wlc_ht_bss_dump(void *ctx, wlc_bsscfg_t *cfg, struct bcmstrbuf *b)
{
}

static uint8
wlc_ht_get_cap_params(wlc_ht_info_t *pub)
{
	wlc_ht_priv_info_t *hti;
	hti = WLC_HT_INFO_PRIV(pub);

	return hti->cap_ie.params;
}

static void
wlc_ht_set_cap_params(wlc_ht_info_t *pub, uint8 p)
{
	wlc_ht_priv_info_t *hti;
	hti = WLC_HT_INFO_PRIV(pub);

	hti->cap_ie.params = p;
}

uint16
wlc_ht_get_cap(wlc_ht_info_t *pub)
{
	wlc_ht_priv_info_t *hti;
	hti = WLC_HT_INFO_PRIV(pub);

	return hti->cap_ie.cap;
}

void
wlc_ht_set_cap(wlc_ht_info_t *pub, uint16 p)
{
	wlc_ht_priv_info_t *hti;
	hti = WLC_HT_INFO_PRIV(pub);

	hti->cap_ie.cap = p;
}

static void
wlc_ht_set_txburst_limit(wlc_ht_info_t *hti, wlc_bsscfg_t *cfg, uint16 v)
{
	BSS_HT_INFO_PUB(hti, cfg)->txburst_limit = v;
}

uint8
wlc_ht_get_phy_membership(wlc_ht_info_t *pub)
{
	wlc_ht_priv_info_t *hti;
	hti = WLC_HT_INFO_PRIV(pub);
	return hti->htphy_membership;
}

static void
wlc_ht_set_cfg_mimops_PM(wlc_ht_info_t *hti, wlc_bsscfg_t *cfg, uint8 v)
{
	BSS_HT_INFO_PRIV(hti, cfg)->mimops_PM = v;
}

static uint8 *
wlc_ht_get_mcs(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint16 ft, wlc_iem_ft_cbparm_t *ftcbparm)
{
	uint8 *mcs = NULL;

	switch (ft) {
	case FC_ASSOC_REQ:
	case FC_REASSOC_REQ:
		mcs = ftcbparm->assocreq.target->rateset.mcs;
		break;
	case FC_ASSOC_RESP:
	case FC_REASSOC_RESP:
		ASSERT(ftcbparm->assocresp.mcs != NULL);
		mcs = ftcbparm->assocresp.mcs;
		break;
	case FC_PROBE_REQ:
		ASSERT(ftcbparm->prbreq.mcs != NULL);
		mcs = ftcbparm->prbreq.mcs;
		break;
	case FC_BEACON:
	case FC_PROBE_RESP:
		if (ftcbparm->bcn.mcs != NULL)
			mcs = ftcbparm->bcn.mcs;
		else if (wlc->stf->throttle_state)
			mcs = wlc->band->hw_rateset.mcs;
		else
			mcs = cfg->current_bss->rateset.mcs;
		break;
	default:
		break;
	}

	return mcs;
}

/** HT Capability */
static uint
wlc_ht_calc_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	if (data->cbparm->ht)
		return TLV_HDR_LEN + HT_CAP_IE_LEN;

	return 0;
}

static int
wlc_ht_write_cap_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	wlc_info_t *wlc = hti->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (data->cbparm->ht) {
		ht_cap_ie_t cap_ie;
		uint8 *mcs;
		if (wlc->cmn->max_rateset) {
			/* Get the capabilties from max supported bw and nss.
			 * Populated by respective owner module.
			 */
			mcs = wlc->cmn->max_rateset->mcs;
		} else {
			mcs = wlc_ht_get_mcs(wlc, cfg, data->ft, data->cbparm->ft);
		}
		ASSERT(mcs != NULL);

		wlc_ht_build_cap_ie(cfg, &cap_ie, mcs, BAND_2G(wlc->band->bandtype));

		bcm_write_tlv(DOT11_MNG_HT_CAP, &cap_ie, HT_CAP_IE_LEN, data->buf);
	}

	return BCME_OK;
}

#ifdef AP
static int
wlc_ht_parse_cap_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	wlc_info_t *wlc = hti->wlc;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;

	if (!data->pparm->ht)
		return BCME_OK;

	if (data->ie == NULL) {
		switch (data->ft) {
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ:
			if (N_REQD(wlc->pub)) {
				/* non N client trying to associate, reject them */
				ftpparm->assocreq.status = DOT11_SC_ASSOC_RATE_MISMATCH;
				return BCME_ERROR;
			}
			break;
		}
		return BCME_OK;
	}

	/* find the HT cap IE, if found copy the mcs set into the requested rates */
	switch (data->ft) {
	case FC_ASSOC_REQ:
	case FC_REASSOC_REQ:
		ftpparm->assocreq.ht_cap_ie = data->ie;
		break;
	}

	return BCME_OK;
}
#endif /* AP */

#ifdef WLTDLS
static uint
wlc_ht_tdls_calc_cap_ie_len(void *ctx, wlc_iem_calc_data_t *calc)
{
	if (calc->cbparm->ht)
		return TLV_HDR_LEN + HT_CAP_IE_LEN;

	return 0;
}

static int
wlc_ht_tdls_write_cap_ie(void *ctx, wlc_iem_build_data_t *build)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	wlc_info_t *wlc = hti->wlc;
	wlc_bsscfg_t *cfg = build->cfg;

	if (build->cbparm->ht) {
		ht_cap_ie_t cap_ie;
		uint8 *mcs;

		mcs = wlc->band->hw_rateset.mcs;
		ASSERT(mcs != NULL);

		wlc_ht_build_cap_ie(cfg, &cap_ie, mcs, BAND_2G(wlc->band->bandtype));

		bcm_write_tlv(DOT11_MNG_HT_CAP, &cap_ie, HT_CAP_IE_LEN, build->buf);
	}

	return BCME_OK;
}
#endif /* WLTDLS */

/** HT Operation */
static uint
wlc_ht_calc_op_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	if (data->cbparm->ht)
		return TLV_HDR_LEN + HT_ADD_IE_LEN;

	return 0;
}

static int
wlc_ht_write_op_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);
	wlc_info_t *wlc = hti->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (data->cbparm->ht) {
		ht_add_ie_t add_ie;

		wlc_prot_n_build_add_ie(wlc->prot_n, cfg, &add_ie);

		bcm_write_tlv(DOT11_MNG_HT_ADD, &add_ie, HT_ADD_IE_LEN, data->buf);
	}

	return BCME_OK;
}

#ifdef WLTDLS
#ifdef IEEE2012_TDLSSEPC
static uint
wlc_ht_tdls_calc_op_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;

	if (!ftcbparm->tdls.ht_op_ie)
		return 0;

	if (data->cbparm->ht)
		return TLV_HDR_LEN + HT_ADD_IE_LEN;

	return 0;
}

static int
wlc_ht_tdls_write_op_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);
	wlc_info_t *wlc = hti->wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (data->cbparm->ht) {
		ht_add_ie_t add_ie;

		wlc_prot_n_build_add_ie(wlc->prot_n, cfg, &add_ie);

		bcm_write_tlv(DOT11_MNG_HT_ADD, &add_ie, HT_ADD_IE_LEN, data->buf);
	}

	return BCME_OK;
}
#endif /* IEEE2012_TDLSSEPC */
#endif /* WLTDLS */

/** BRCM HT Cap */
static uint
wlc_ht_calc_brcm_cap_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	if (!data->cbparm->ht)
		return 0;
	if (data->cfg->target_bss->flags & WLC_BSS_BRCM)
		return TLV_HDR_LEN + HT_CAP_IE_LEN + HT_PROP_IE_OVERHEAD;
	else
		return 0;
}

static int
wlc_ht_write_brcm_cap_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);
	wlc_info_t *wlc = hti->wlc;
	wlc_bsscfg_t *cfg = data->cfg;
	ht_cap_ie_t cap_ie;
	uint8 *mcs;

	if (!data->cbparm->ht)
		return BCME_OK;
	if (data->cfg->target_bss->flags & WLC_BSS_BRCM) {
		mcs = wlc_ht_get_mcs(wlc, cfg, data->ft, data->cbparm->ft);
		ASSERT(mcs != NULL);

		wlc_ht_build_cap_ie(cfg, &cap_ie, mcs, BAND_2G(wlc->band->bandtype));
		wlc_write_brcm_ht_cap_ie(wlc, data->buf, data->buf_len, &cap_ie);
	}
	return BCME_OK;
}

static void
wlc_ht_scan_parse_cap(ht_cap_ie_t *cap, wlc_iem_ft_pparm_t *ftpparm)
{
	wlc_bss_info_t *bi = ftpparm->scan.result;
	uint16 ht_cap = ltoh16_ua(&cap->cap);

	/* Mark the BSS as HT capable */
	bi->flags |= WLC_BSS_HT;

	/* Mark the BSS as 40 Intolerant if bit is set */
	if (ht_cap & HT_CAP_40MHZ_INTOLERANT)
		bi->flags |= WLC_BSS_40INTOL;

	/* Set SGI flags */
	if (ht_cap & HT_CAP_SHORT_GI_20)
		bi->flags |= WLC_BSS_SGI_20;
	if (ht_cap & HT_CAP_SHORT_GI_40)
		bi->flags |= WLC_BSS_SGI_40;

	/* copy the raw mcs set (supp_mcs) into the bss rateset struct */
	bcopy(&cap->supp_mcs[0], &bi->rateset.mcs[0], MCSSET_LEN);

	if (ht_cap & HT_CAP_40MHZ)
		ftpparm->scan.cap_bw_40 = TRUE;

	/* Set 40MHZ employed bit on bss */
	if (ftpparm->scan.op_bw_any && ftpparm->scan.cap_bw_40)
		bi->flags |= WLC_BSS_40MHZ;
}

/** callback function */
static int
wlc_ht_scan_parse_cap_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	wlc_info_t *wlc = hti->wlc;
	ht_cap_ie_t *cap;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;

	if (data->ie == NULL)
		return BCME_OK;

	/* find the 11n HT capability ie, mark the bss as 11n (HT) and save the mcs set */
	if ((cap = wlc_read_ht_cap_ie(wlc, data->ie, data->ie_len)) != NULL)
		wlc_ht_scan_parse_cap(cap, ftpparm);

	return BCME_OK;
}

/** callback function */
static int
wlc_ht_scan_parse_op_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	wlc_info_t *wlc = hti->wlc;
	ht_add_ie_t *op;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;
	int err = BCME_ERROR;

	if (data->ie == NULL)
		return BCME_OK;

	if ((op = wlc_read_ht_add_ie(wlc, data->ie, data->ie_len)) != NULL) {
#ifdef WL11ULB
		bi->chanspec = wlc_ht_chanspec(wlc, op->ctl_ch, op->byte1, data->cfg);
#else /* WL11ULB */
		bi->chanspec = wlc_ht_chanspec(wlc, op->ctl_ch, op->byte1);
#endif /* WL11ULB */

		if (op->byte1 & HT_BW_ANY)
			ftpparm->scan.op_bw_any = TRUE;

		/* Set 40MHZ employed bit on bss */
		if (ftpparm->scan.op_bw_any && ftpparm->scan.cap_bw_40)
			bi->flags |= WLC_BSS_40MHZ;

		err = BCME_OK;
	}

	return err;
}

/** callback function. Should be named 'wlc_ht_scan_parse_brcm_ht_ie' instead. */
static int
wlc_ht_scan_parse_brcm_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	wlc_info_t *wlc = hti->wlc;
	ht_cap_ie_t *cap;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;

	if (data->ie == NULL)
		return BCME_OK;

	/*
	 * Find the 11n HT capability ie, mark the bss as 11n (HT) and save the mcs set.
	 * brcm prop ht cap ie is only parsed if 'standard' ht cap ie is not present.
	 */
	if ((cap = wlc_read_ht_cap_ies(wlc, data->ie, data->ie_len)) != NULL)
		wlc_ht_scan_parse_cap(cap, ftpparm);

	return BCME_OK;
}

#ifdef WLTDLS
/* TODO: fold this into 'scan' processing if possible... */
static int
wlc_ht_tdls_parse_cap_ie(void *ctx, wlc_iem_parse_data_t *parse)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);
	wlc_info_t *wlc = hti->wlc;
	wlc_iem_ft_pparm_t *ftpparm = parse->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->tdls.result;
	ht_cap_ie_t *cap_ie;

	if (parse->ie == NULL)
		return BCME_OK;

	/* find the 11n HT capability ie, mark the bss as 11n (HT) and save the mcs set */
	if ((cap_ie = wlc_read_ht_cap_ie(wlc, parse->ie, parse->ie_len)) != NULL) {
		uint16 ht_cap = ltoh16_ua(&cap_ie->cap);

		/* Mark the BSS as HT capable */
		bi->flags |= WLC_BSS_HT;

		/* Mark the BSS as 40 Intolerant if bit is set */
		if (ht_cap & HT_CAP_40MHZ_INTOLERANT) {
			bi->flags |= WLC_BSS_40INTOL;
		}

		/* Set SGI flags */
		if (ht_cap & HT_CAP_SHORT_GI_20)
			bi->flags |= WLC_BSS_SGI_20;
		if (ht_cap & HT_CAP_SHORT_GI_40)
			bi->flags |= WLC_BSS_SGI_40;

		/* copy the raw mcs set (supp_mcs) into the bss rateset struct */
		bcopy(&cap_ie->supp_mcs[0], &bi->rateset.mcs[0], MCSSET_LEN);
	}

	return BCME_OK;
}

#ifdef IEEE2012_TDLSSEPC
static int
wlc_ht_tdls_parse_op_ie(void *ctx, wlc_iem_parse_data_t *parse)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);
	wlc_iem_ft_pparm_t *ftpparm = parse->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->tdls.result;
	wlc_info_t *wlc = hti->wlc;
	ht_add_ie_t	*ht_op_ie;
	chanspec_t chspec;

	if (parse->ie == NULL)
		return BCME_OK;

	ht_op_ie = wlc_read_ht_add_ie(wlc, parse->ie, parse->ie_len);

	if (ht_op_ie) {
#ifdef WL11ULB
		chspec = wlc_ht_chanspec(wlc, ht_op_ie->ctl_ch, ht_op_ie->byte1, parse->cfg);
#else /* WL11ULB */
		chspec = wlc_ht_chanspec(wlc, ht_op_ie->ctl_ch, ht_op_ie->byte1);
#endif /* WL11ULB */
		if (chspec != INVCHANSPEC) {
			bi->chanspec = chspec;
			bi->flags |= WLC_BSS_40MHZ;
		}
	}
	return BCME_OK;
}
#endif /* IEEE2012_TDLSSEPC */
#endif /* WLTDLS */

static int
_hti_ioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	wlc_info_t *wlc = hti->wlc;
	int val, *pval;
	bool bool_val;
	int bcmerror;

	/* default argument is generic integer */
	pval = (int *) arg;

	/* This will prevent the misaligned access */
	if (pval && (uint32)len >= sizeof(val))
		bcopy(pval, &val, sizeof(val));
	else
		val = 0;

	/* bool conversion to avoid duplication below */
	bool_val = (val != 0);

	bcmerror = 0;
	switch (cmd) {
		case WLC_GET_FAKEFRAG:
			*pval = pub->frameburst;
			break;
		case WLC_SET_FAKEFRAG:
			pub->frameburst = bool_val;
			wlc_frameburst_size(wlc);
			wlc_ht_frameburst_limit(pub);
			break;
		default:
			bcmerror = BCME_UNSUPPORTED;
			break;
	}
	return bcmerror;

}

static int
wlc_ht_doiovar(void *context, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int vsize, struct wlc_if *wlcif)
{
	int err = BCME_OK;
	wlc_ht_info_t *pub = (wlc_ht_info_t*)context;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	wlc_info_t *wlc = hti->wlc;
	int32 int_val = 0, int_val2 = 0;
	int32 *ret_int_ptr;
	wlc_bsscfg_t *bsscfg;
	bool bool_val;
	wlc_bss_info_t *current_bss;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);
	current_bss = bsscfg->current_bss;

	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;
	bool_val = (int_val != 0) ? TRUE : FALSE;
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	if (p_len >= (int)sizeof(int_val) * 2)
		bcopy((void*)((uintptr)params + sizeof(int_val)),
			&int_val2, sizeof(int_val));

	switch (actionid) {
#ifdef WLAMSDU
		case IOV_GVAL(IOV_HT_AMSDU_RXMAX):
			int_val = (int8)((wlc_ht_get_cap(wlc->hti) & HT_CAP_MAX_AMSDU) != 0);
			bcopy(&int_val, arg, vsize);
			break;

		case IOV_SVAL(IOV_HT_AMSDU_RXMAX): {
			uint16 cap = wlc_ht_get_cap(wlc->hti);
			cap &= ~HT_CAP_MAX_AMSDU;
			if (bool_val) {
				if (!wlc_amsdu_is_rxmax_valid(wlc->ami)) {
					err = BCME_RANGE;
					break;
				}
				cap |= HT_CAP_MAX_AMSDU;
			}
			wlc_ht_set_cap(wlc->hti, cap);
		}
			break;
#endif /* WLAMSDU */

		case IOV_GVAL(IOV_HT_MIMOPS):
			*ret_int_ptr = wlc_ht_mimops_get(wlc->hti);
			break;

		case IOV_SVAL(IOV_HT_MIMOPS):
			if (!wlc_ht_mimops_set(wlc->hti, bsscfg, int_val)) {
				err = BCME_RANGE;
			}
			break;
		case IOV_GVAL(IOV_AMPDU_RTS):
			*ret_int_ptr = (int32)pub->ampdu_rts;
			break;

		case IOV_SVAL(IOV_AMPDU_RTS):
			pub->ampdu_rts = (bool)int_val;
			break;

		case IOV_GVAL(IOV_NRATE):
		{
			wlcband_t *cur_band = wlc->band;
			ratespec_t rspec;
			uint32 wl_rspec;
#ifdef WL11ULB
			chanspec_t min_bw = wlc_ulb_get_bss_min_bw(wlc->ulb_info, bsscfg);
#endif /* WL11ULB */

			if (bsscfg->associated)
				cur_band = wlc->bandstate[CHSPEC_IS2G(current_bss->chanspec) ?
					BAND_2G_INDEX : BAND_5G_INDEX];

			rspec = wlc_get_rspec_history(bsscfg);

			/* both wlc ratespec and wl_ratespec have rate/mcs value in same place */
			wl_rspec = (rspec & RSPEC_RATE_MASK);

			/* have a preprocessor check to make sure this assumption is valid */
#if WL_RSPEC_RATE_MASK != RSPEC_RATE_MASK
#error "code doing wlc to wl ioctl ratespec conversion needs update"
#endif // endif

			wl_rspec |= RSPEC_TXEXP(rspec) << WL_RSPEC_TXEXP_SHIFT;
			wl_rspec |= RSPEC_ISSTBC(rspec) ? WL_RSPEC_STBC : 0;
			wl_rspec |= RSPEC_ISTXBF(rspec) ? WL_RSPEC_TXBF : 0;
			wl_rspec |= RSPEC_ISLDPC(rspec) ? WL_RSPEC_LDPC : 0;
			wl_rspec |= RSPEC_ISSGI(rspec) ? WL_RSPEC_SGI : 0;

#ifdef WL11ULB
			/* Only on the Query interface if the min_bw of bsscfg indicates setting of
			 * ULB BW then BW is set to appropriate ULB BW
			 */
			if (WLC_ULB_MODE_ENABLED(wlc) && (min_bw != WL_CHANSPEC_BW_20)) {
				wl_rspec |= (min_bw == WL_CHANSPEC_BW_10) ? WL_RSPEC_BW_10MHZ : 0;
				wl_rspec |= (min_bw == WL_CHANSPEC_BW_5) ? WL_RSPEC_BW_5MHZ : 0;
				wl_rspec |= (min_bw == WL_CHANSPEC_BW_2P5) ? WL_RSPEC_BW_2P5MHZ : 0;

			} else
#endif /* WL11ULB */
			{
				wl_rspec |= RSPEC_IS20MHZ(rspec) ? WL_RSPEC_BW_20MHZ : 0;
				wl_rspec |= RSPEC_IS40MHZ(rspec) ? WL_RSPEC_BW_40MHZ : 0;
				wl_rspec |= RSPEC_IS80MHZ(rspec) ? WL_RSPEC_BW_80MHZ : 0;
				wl_rspec |= RSPEC_IS160MHZ(rspec) ? WL_RSPEC_BW_160MHZ : 0;
			}

			if (RSPEC_ISLEGACY(rspec))
				wl_rspec |= WL_RSPEC_ENCODE_RATE;
			else if (RSPEC_ISHT(rspec))
				wl_rspec |= WL_RSPEC_ENCODE_HT;
			else if (RSPEC_ISVHT(rspec))
				wl_rspec |= WL_RSPEC_ENCODE_VHT;

			if ((cur_band->rspec_override & RSPEC_OVERRIDE_RATE))
				wl_rspec |= WL_RSPEC_OVERRIDE_RATE;
			if ((cur_band->rspec_override & RSPEC_OVERRIDE_MODE))
				wl_rspec |= WL_RSPEC_OVERRIDE_MODE;

			*ret_int_ptr = wl_rspec;

			break;
		}

		case IOV_SVAL(IOV_NRATE):
		{
			int cur_band = wlc->band->bandtype;

			if (bsscfg->associated)
				cur_band = CHSPEC_IS2G(current_bss->chanspec) ?
					WLC_BAND_2G : WLC_BAND_5G;

			err =
#ifdef WL11ULB
				wlc_set_iovar_ratespec_override(wlc, bsscfg, cur_band,
				(uint32)int_val, FALSE);
#else /* WL11ULB */
				wlc_set_iovar_ratespec_override(wlc, cur_band,
				(uint32)int_val, FALSE);
#endif /* WL11ULB */
			break;
		}
		case IOV_GVAL(IOV_NREQD):
			*ret_int_ptr = (int32)wlc->pub->_n_reqd;
			break;

		case IOV_SVAL(IOV_NREQD):
			if ((WLC_PHY_11N_CAP(wlc->band)) && N_ENAB(wlc->pub))
				wlc->pub->_n_reqd = bool_val;
			else
				err = BCME_UNSUPPORTED;
			break;
		case IOV_GVAL(IOV_SGI_TX):
			*ret_int_ptr = pub->sgi_tx;
			break;

		case IOV_SVAL(IOV_SGI_TX):
			if ((int_val != AUTO) && (int_val != OFF) && (int_val != ON)) {
				err = BCME_RANGE;
				break;
			}

			if (!WLC_SGI_CAP_PHY(wlc)) {
				err = BCME_UNSUPPORTED;
				break;
			}

			pub->sgi_tx = (int8)int_val;
			break;

		case IOV_GVAL(IOV_SGI_RX):
			*ret_int_ptr = (hti->cap_ie.cap & HT_CAP_SHORT_GI_20) ? WLC_N_SGI_20 : 0;
			*ret_int_ptr |= (hti->cap_ie.cap & HT_CAP_SHORT_GI_40) ? WLC_N_SGI_40 : 0;
			*ret_int_ptr |= (wlc_vht_get_cap_info(wlc->vhti) & VHT_CAP_INFO_SGI_80MHZ) ?
				WLC_VHT_SGI_80 : 0;
			*ret_int_ptr |= (wlc_vht_get_cap_info(wlc->vhti) &
				VHT_CAP_INFO_SGI_160MHZ) ? WLC_VHT_SGI_160 : 0;
			break;

		case IOV_SVAL(IOV_SGI_RX):
#ifdef WL11AC
			if (int_val > (WLC_N_SGI_20 | WLC_N_SGI_40 | WLC_VHT_SGI_80 |
				WLC_VHT_SGI_160))
#else
			if (int_val > (WLC_N_SGI_20 | WLC_N_SGI_40))
#endif // endif
			{
				err = BCME_RANGE;
				break;
			}

			if (!WLC_SGI_CAP_PHY(wlc)) {
				err = BCME_UNSUPPORTED;
				break;
			}

			wlc_ht_update_sgi_rx(pub, int_val);
			wlc_vht_update_sgi_rx(wlc->vhti, int_val);
			break;
		case IOV_GVAL(IOV_BW_CAP):
		case IOV_SVAL(IOV_BW_CAP):
				{
				wlcband_t *band = NULL;
				int bandtype = int_val;
				uint8 bw_cap = (uint8) int_val2;

				if (IOV_ISSET(actionid)) {
					err = wlc_update_bwcap(pub, bandtype, (uint8) bw_cap);
				} else {
					if (bandtype == WLC_BAND_5G) {
						band = wlc->bandstate[BAND_5G_INDEX];
					} else if (bandtype == WLC_BAND_2G) {
						band = wlc->bandstate[BAND_2G_INDEX];
					} else {
						err = BCME_BADBAND;
					}

					if (err == 0) {
						*ret_int_ptr = (int32)band->bw_cap;
					}
				}
				break;
			}

		case IOV_GVAL(IOV_MIMO_BW_CAP):
				/* Deprecated */
			*ret_int_ptr = hti->mimo_band_bwcap;
			break;

		case IOV_SVAL(IOV_MIMO_BW_CAP):
				/* Deprecated */
			if ((int_val < 0) || (int_val > WLC_N_BW_20IN2G_40IN5G)) {
				err = BCME_RANGE;
				break;
			}
			wlc_update_mimo_band_bwcap(wlc, (uint8) int_val);
			break;

#if WL_HT_TXBW_OVERRIDE_ENAB
		case IOV_GVAL(IOV_MIMO_TXBW):
			*ret_int_ptr = (pub->mimo_40txbw != AUTO) ?
				(pub->mimo_40txbw + WLC_TXBW_20MHZ):pub->mimo_40txbw;
			break;

		case IOV_SVAL(IOV_MIMO_TXBW):
			{
				int32 max_value;
				if (WLC_80MHZ_CAP_PHY(wlc)) {
					max_value = WLC_TXBW80_MAX;
				} else if (WLC_40MHZ_CAP_PHY(wlc)) {
					max_value = WLC_TXBW40_MAX;
				} else {
					max_value = WLC_TXBW_20MHZ_UP;
				}

				if (int_val != AUTO) {
					if ((int_val < WLC_TXBW_20MHZ) ||
						(int_val > max_value)) {
						err = BCME_RANGE;
						break;
					}
					int_val -= WLC_TXBW_20MHZ;
				}
				pub->mimo_40txbw = (int8)int_val;
				wlc_ht_update_txbw_override(pub);
			}
			break;

		case IOV_GVAL(IOV_OFDM_TXBW):
			*ret_int_ptr = (pub->ofdm_40txbw != AUTO) ?
				(pub->ofdm_40txbw + WLC_TXBW_20MHZ):pub->ofdm_40txbw;
			break;
		case IOV_SVAL(IOV_OFDM_TXBW):
			if (int_val != AUTO) {
				if ((int_val < WLC_TXBW_20MHZ) || (int_val > WLC_TXBW_40MHZ_DUP) ||
					(int_val == WLC_TXBW_40MHZ)) {
					err = BCME_RANGE;
					break;
				}
				int_val -= WLC_TXBW_20MHZ;
			}
			pub->ofdm_40txbw = (int8)int_val;
			wlc_ht_update_txbw_override(pub);

			break;

		case IOV_GVAL(IOV_CCK_TXBW):
			*ret_int_ptr = (pub->cck_40txbw != AUTO) ?
				(pub->cck_40txbw + WLC_TXBW_20MHZ):pub->cck_40txbw;
			break;

		case IOV_SVAL(IOV_CCK_TXBW):
			if (int_val != AUTO) {
				if ((int_val != WLC_TXBW_20MHZ) && (int_val != WLC_TXBW_20MHZ_UP)) {
					err = BCME_RANGE;
					break;
				}
				int_val -= WLC_TXBW_20MHZ;
			}
			pub->cck_40txbw = (int8)int_val;
			wlc_ht_update_txbw_override(pub);

			break;
#endif /* WL_HT_TXBW_OVERRIDE_ENAB */

#ifdef BCMDBG
		case IOV_SVAL(IOV_AGGDBG): {
#ifdef WLC_HIGH_ONLY
			rpc_tp_info_t *rpcb = bcm_rpc_tp_get(wlc->rpc);
#endif // endif
			int32 *shmbuf;
			int i;

			int_val2 = (int_val >> 16) & 0xffff;
			int_val = int_val & 0xffff;

#ifdef WLC_HIGH_ONLY
			bcm_rpc_tp_agg_set(rpcb, 1, TRUE);
#endif // endif

			/* block 1 */
			shmbuf = MALLOC(wlc->osh, int_val);
			if (shmbuf == NULL) {
				printf("malloc err\n");
				break;
			}
			for (i = 0; i < int_val/4; i++)
				shmbuf[i] = i;

			wlc_bmac_copyto_shm(wlc->hw, (uint)-1, shmbuf, int_val);
			MFREE(wlc->osh, shmbuf, int_val);

			/* block 2 */
			shmbuf = MALLOC(wlc->osh, int_val2);
			if (shmbuf == NULL) {
				printf("malloc err\n");
				break;
			}
			for (i = 0; i < int_val2/4; i++)
				shmbuf[i] = i;

			wlc_bmac_copyto_shm(wlc->hw, (uint)-1, shmbuf, int_val2);
			MFREE(wlc->osh, shmbuf, int_val2);

			/* block 3 */
			shmbuf = MALLOC(wlc->osh, int_val2);
			if (shmbuf == NULL) {
				printf("malloc err\n");
				break;
			}
			for (i = 0; i < int_val2/4; i++)
				shmbuf[i] = i;

			wlc_bmac_copyto_shm(wlc->hw, (uint)-1, shmbuf, int_val2);
			MFREE(wlc->osh, shmbuf, int_val2);

#ifdef WLC_HIGH_ONLY
			bcm_rpc_tp_agg_set(rpcb, 1, FALSE);
#endif // endif
			break;
		}
#endif /* BCMDBG */
		case IOV_GVAL(IOV_GF_CAP):

			if (!WLC_PHY_11N_CAP(wlc->band)) {
				err = BCME_UNSUPPORTED;
				break;
			}
			*ret_int_ptr = (hti->cap_ie.cap & HT_CAP_GF) ? 1 : 0;
			break;

#ifdef BCMDBG
		case IOV_GVAL(IOV_TXBURST_LIM):
			*ret_int_ptr = (int32)hti->txburst_limit;
			break;

		case IOV_GVAL(IOV_HTPHY_MEMBERSHIP):
			*ret_int_ptr = (int32)(hti->htphy_membership ? TRUE : FALSE);
			break;

		case IOV_SVAL(IOV_HTPHY_MEMBERSHIP):
			hti->htphy_membership = (bool_val ? (WLC_HTPHY | WLC_RATE_FLAG) : 0);
			break;
#endif /* BCMDBG */

		case IOV_GVAL(IOV_CURR_MCSSET):
			if (bsscfg->associated)
				bcopy(&current_bss->rateset.mcs[0], arg, MCSSET_LEN);
			else
				bcopy(&wlc->default_bss->rateset.mcs[0], arg, MCSSET_LEN);
			break;

		case IOV_GVAL(IOV_RIFS):
			*ret_int_ptr = pub->_rifs;
			break;

		case IOV_SVAL(IOV_RIFS):
			/* frameburst has to be enabled for RIFS */
			if (int_val && !pub->frameburst) {
				err = BCME_NOTREADY;
				break;
			}
			pub->_rifs = (uint8)int_val;
			if (wlc->pub->up) {
				wlc_ht_frameburst_limit(pub);

				/* pass _rifs flag down to wlc_hw_info structure */
				/* and update PHY holdoff and delay registers */
				wlc_phy_tkip_rifs_war(WLC_PI(wlc), pub->_rifs);
			}
			break;

		case IOV_GVAL(IOV_RIFS_ADVERT):
			*ret_int_ptr = hti->rifs_advert;
			break;

		case IOV_SVAL(IOV_RIFS_ADVERT):
			hti->rifs_advert = int_val;

			/* Modify the RIFS mode bit in the beacon and probe response
			 * frames. This is to indicate to STAs if RIFS is permitted
			 * in the BSS or not.
			 */
			if (AP_ENAB(wlc->pub)) {
				wlc_update_beacon(wlc);
				wlc_update_probe_resp(wlc, TRUE);
			}
			wlc_ht_update_rifs_mode(pub, NULL);

			break;

		case IOV_GVAL(IOV_LDPC_CAP):
			if (!WLC_LDPC_CAP_PHY(wlc)) {
				err = BCME_UNSUPPORTED;
				break;
			}
			*ret_int_ptr = (int32)wlc->stf->ldpc;
			break;

		case IOV_SVAL(IOV_LDPC_CAP):
			if (!WLC_LDPC_CAP_PHY(wlc)) {
				err = BCME_UNSUPPORTED;
				break;
			}

			/* Don't allow LDPC capability change if associated */
			if (wlc->pub->associated) {
				err = BCME_ASSOCIATED;
				break;
			}

			if ((int_val != OFF) && (int_val != ON)) {
				err = BCME_RANGE;
				break;
			}

			if (wlc->stf->ldpc != (int8)int_val)
				wlc_ht_update_ldpc(wlc->hti, (int8)int_val);
			break;

		case IOV_GVAL(IOV_LDPC_TX):
			if (!WLC_LDPC_CAP_PHY(wlc)) {
				err = BCME_UNSUPPORTED;
				break;
			}
			*ret_int_ptr = (int32)wlc->stf->ldpc_tx;
			break;

		case IOV_SVAL(IOV_LDPC_TX):
			if (!WLC_LDPC_CAP_PHY(wlc)) {
				err = BCME_UNSUPPORTED;
				break;
			}
			if ((int_val != AUTO) && (int_val != OFF) && (int_val != ON)) {
				err = BCME_RANGE;
				break;
			}

			wlc->stf->ldpc_tx = (int8)int_val;
			break;

		case IOV_GVAL(IOV_NMODE):
			if (N_ENAB(wlc->pub))
				*ret_int_ptr = (wlc->pub->_n_enab == SUPPORT_11N) ? WL_11N_2x2 :
					(wlc->stf->op_txstreams == WL_11N_4x4) ? WL_11N_4x4 :
					WL_11N_3x3;
			else
				*ret_int_ptr = OFF;
			break;
		case IOV_SVAL(IOV_NMODE):
			err = wlc_setup_nmode(wlc, int_val);
			break;
		case IOV_GVAL(IOV_HT_WSEC_RESTRICT):
			*ret_int_ptr = hti->ht_wsec_restriction;

			break;

		case IOV_SVAL(IOV_HT_WSEC_RESTRICT):
			if ((int_val < 0) || (int_val > 3))
				err = BCME_RANGE;
			else
				hti->ht_wsec_restriction = (uint8)int_val;

			break;
#ifdef BCMDBG
			case IOV_GVAL(IOV_TXMCSSET):
				wlc_get_mcsset(pub, arg, TRUE);

				break;

			case IOV_GVAL(IOV_RXMCSSET):
				wlc_get_mcsset(pub, arg, FALSE);

				break;
#endif /* BCMDBG */
			case IOV_GVAL(IOV_FBTXOPTHRESH):
				*ret_int_ptr = (int32)wlc_ht_frameburst_txop_get(pub);
				break;

			case IOV_SVAL(IOV_FBTXOPTHRESH):
				if (int_val < 0 || int_val >= 0xFFFF) {
					err = BCME_RANGE; /* bad value */
					break;
				}
				if (!pub->_rifs) {
					pub->max_fbtxop_user = (uint16)int_val;
				}
				wlc_ht_frameburst_limit(pub);
				break;

		default:
			err = BCME_UNSUPPORTED;
			break;
	}

	return err;
}

#ifdef STA
static void
wlc_ht_mimops_action(wlc_ht_info_t *hti, wlc_bsscfg_t *cfg)
{
	wlc_ht_bss_info_priv_t *cubby = BSS_HT_INFO_PRIV(hti, cfg);

	if (cubby->mimops_ActionRetry & WLC_MIMOPS_RETRY_SEND) {
		wlc_mimops_action_ht_send(hti, cfg, cubby->mimops_PM);
	} else if (cubby->mimops_ActionRetry & WLC_MIMOPS_RETRY_NOACK) {
		wlc_mimops_action_ht_send(hti, cfg, cubby->mimops_ActionPM);
	}
}
#endif /* STA */

static void *
wlc_send_action_ht_mimops(wlc_ht_info_t *pub, wlc_bsscfg_t *bsscfg, uint8 mimops_mode)
{
	void *p;
	uint8* pbody;
	uint body_len;
	struct dot11_action_ht_mimops * action_hdr;
	const struct ether_addr *ea;
	struct scb *scb;
	wlc_info_t *wlc;
	wlc_ht_priv_info_t *hti;
	hti = WLC_HT_INFO_PRIV(pub);

	ASSERT(mimops_mode != 2);
	if ((mimops_mode == 2) || (mimops_mode > 3))
		return NULL;
	wlc = hti->wlc;

	body_len = sizeof(struct dot11_action_ht_mimops);

	if (BSSCFG_STA(bsscfg)) {
		ea = &bsscfg->BSSID;
		scb = wlc_scbfindband(wlc, bsscfg, ea,
		                      CHSPEC_WLCBANDUNIT(bsscfg->current_bss->chanspec));
	} else {
		ea = &ether_bcast;
		scb = WLC_BCMCSCB_GET(wlc, bsscfg);
	}
	ASSERT(scb != NULL);

	if ((p = wlc_frame_get_mgmt(wlc, FC_ACTION, ea, &bsscfg->cur_etheraddr,
	                            &bsscfg->BSSID, body_len, &pbody)) == NULL) {
		return NULL;
	}

	action_hdr = (struct dot11_action_ht_mimops *)pbody;
	action_hdr->category = DOT11_ACTION_CAT_HT;
	action_hdr->action = DOT11_ACTION_ID_HT_MIMO_PS;

	/* The frame format meets the 802.11n Draft 1.06 spec */
	switch (mimops_mode) {
	case 0: action_hdr->control = SM_PWRSAVE_ENABLE;
		break;
	case 1: action_hdr->control = (SM_PWRSAVE_ENABLE | SM_PWRSAVE_MODE);
		break;
	case 2: ASSERT(0); /* invalid case */
	case 3: action_hdr->control = 0;
		break;
	}

	wlc_pcb_fn_register(wlc->pcb, wlc_mimops_action_ht_complete,
	                    (void *)(uintptr)bsscfg->ID, p);

	wlc_sendmgmt(wlc, p, bsscfg->wlcif->qi, scb);

	return p;
}

static void
wlc_mimops_action_ht_send(wlc_ht_info_t *pub, wlc_bsscfg_t *bsscfg, uint8 mimops_mode)
{
	wlc_ht_priv_info_t *hti;
	wlc_info_t *wlc;
	void *p;
	wlc_ht_bss_info_priv_t *cubby;

	hti = WLC_HT_INFO_PRIV(pub);
	wlc = hti->wlc;

	cubby = BSS_HT_INFO_PRIV(pub, bsscfg);

	/* FIXME PR81503: The wlc->mimops_ActionPending and related state should be per-bsscfg
	 * and the HT actions should propagate through all the cfgs.
	 */

	if (BSSCFG_STA(bsscfg) && (cubby->mimops_ActionPending) &&
	    !(cubby->mimops_ActionRetry & WLC_MIMOPS_RETRY_NOACK))
		return;

	p = wlc_send_action_ht_mimops(wlc->hti, bsscfg, mimops_mode);

	if (BSSCFG_AP(bsscfg))
		return;
	if (!p)
		cubby->mimops_ActionRetry |= WLC_MIMOPS_RETRY_SEND;
	else {
		cubby->mimops_ActionRetry &= ~WLC_MIMOPS_RETRY_SEND;
		cubby->mimops_ActionPending = TRUE;
		cubby->mimops_ActionPM = mimops_mode;
	}
}

static void
wlc_ht_mimops_cap_update(wlc_ht_info_t *pub, uint8 mimops_mode)
{
	wlc_ht_priv_info_t *hti;
	wlc_info_t *wlc;

	hti = WLC_HT_INFO_PRIV(pub);

	wlc = hti->wlc;
	hti->cap_ie.cap &= ~HT_CAP_MIMO_PS_MASK;
	hti->cap_ie.cap |= (mimops_mode << HT_CAP_MIMO_PS_SHIFT);

	if (AP_ENAB(wlc->pub) && wlc->clk) {
		wlc_update_beacon(wlc);
		wlc_update_probe_resp(wlc, TRUE);
	}
}

static void
wlc_mimops_action_ht_complete(wlc_info_t *wlc, uint txstatus, void *arg)
{
	wlc_bsscfg_t *cfg = wlc_bsscfg_find_by_ID(wlc, (uint16)(uintptr)arg);
	wlc_ht_bss_info_priv_t *cubby;

	/* in case bsscfg is freed before this callback is invoked */
	if (cfg == NULL) {
		WL_ERROR(("wl%d: %s: unable to find bsscfg by ID %p\n",
		          wlc->pub->unit, __FUNCTION__, arg));
		return;
	}
	cubby = BSS_HT_INFO_PRIV(wlc->hti, cfg);

	/* no ack */
	if (!(txstatus & TX_STATUS_ACK_RCV)) {
		WL_ERROR(("%s(): no ACK received!\n", __FUNCTION__));
		/* keep retry in watch_dog, send HT MS Power Save Action Frame again */
		cubby->mimops_ActionRetry |= WLC_MIMOPS_RETRY_NOACK;
		return;
	}

	cubby->mimops_ActionRetry &= ~WLC_MIMOPS_RETRY_NOACK;
	if (cubby->mimops_ActionPM == HT_CAP_MIMO_PS_ON) {
		if (VHT_ENAB_BAND(wlc->pub, wlc->band->bandtype))
			wlc_vht_update_mcs_cap(wlc->vhti);
		wlc_ht_mimops_cap_update(wlc->hti, HT_CAP_MIMO_PS_ON);

		/* only when HT action frame is ACKed, do we update the Rx chain HW state */
		wlc_phy_stf_chain_set(WLC_PI(wlc), wlc->stf->txchain, wlc->stf->rxchain);
	}
	cubby->mimops_ActionPending = FALSE;

	/* FIXME: PR81503: This code needs to know what bsscfg just completed the PS
	 * notification instead of just using wlc->cfg. We may need to move along a higher layer
	 * state machine that is updating multiple bsscfgs.
	 */
	if (cubby->mimops_ActionPM != cubby->mimops_PM)
		wlc_mimops_action_ht_send(wlc->hti, cfg, cubby->mimops_PM);

}

static int
wlc_nmode_validate(wlc_info_t *wlc, int32 nmode)
{
	int err = 0;

	switch (nmode) {

	case OFF:
		break;

	case AUTO:
	case WL_11N_2x2:
	case WL_11N_3x3:
	case WL_11N_4x4:
		if (!(WLC_PHY_11N_CAP(wlc->band)))
			err = BCME_BADBAND;
		break;

	default:
		err = BCME_RANGE;
		break;
	}
	return err;
}

int
wlc_set_nmode(wlc_ht_info_t *pub, int32 nmode)
{
	wlc_info_t *wlc;
	wlc_ht_priv_info_t *hti;
	uint i;
	int err, idx;
	struct scb *scb;
	struct scb_iter scbiter;
	wlc_bsscfg_t *bsscfg;
	wlcband_t *band;

	hti = WLC_HT_INFO_PRIV(pub);
	wlc = hti->wlc;

	err = wlc_nmode_validate(wlc, nmode);
	ASSERT(err == 0);
	if (err) {
		WL_ERROR(("nmode invalid: called with nmode=%d\n", nmode));
		return err;
	}
	switch (nmode) {
	case OFF:
		wlc->pub->_n_enab = OFF;
		/* If _n_enab is off, we should turn off vht as well */
		wlc->pub->_vht_enab = 0;

		wlc->default_bss->flags &= ~WLC_BSS_HT;
		/* delete the mcs rates from the default and hw ratesets */
		wlc_rateset_mcs_clear(&wlc->default_bss->rateset);
		for (i = 0; i < NBANDS(wlc); i++) {
			band = (NBANDS(wlc) == 1) ? wlc->band : wlc->bandstate[i];

			memset(band->hw_rateset.mcs, 0, MCSSET_LEN);
			if (band->rspec_override != 0 &&
			    !RSPEC_ISLEGACY(band->rspec_override)) {
				band->rspec_override = 0;
				wlc_reprate_init(wlc);
			}
			if (band->mrspec_override != 0 &&
			    !RSPEC_ISLEGACY(band->mrspec_override)) {
				band->mrspec_override = 0;
			}
		}

		/* Clear HT related state for SCBs */
		FOREACH_BSS(wlc, idx, bsscfg) {
			FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb) {
				wlc_ht_update_scbstate(wlc->hti, scb, NULL, NULL, NULL);
			}
		}

		break;

	case AUTO:
		if (wlc->stf->op_txstreams == WL_11N_4x4)
			nmode = WL_11N_4x4;
		else if (wlc->stf->op_txstreams == WL_11N_3x3)
			nmode = WL_11N_3x3;
		else
			nmode = WL_11N_2x2;
		/* Fall Through */

	case WL_11N_2x2:
	case WL_11N_3x3:
	case WL_11N_4x4:
		ASSERT(WLC_PHY_11N_CAP(wlc->band));
		/* force GMODE_AUTO if NMODE is ON */
		wlc_set_gmode(wlc, GMODE_AUTO, TRUE);
		if (nmode == WL_11N_4x4|| nmode == WL_11N_3x3)
			wlc->pub->_n_enab = SUPPORT_HT;
		else
			wlc->pub->_n_enab = SUPPORT_11N;
		wlc->default_bss->flags |= WLC_BSS_HT;
		if (WLC_PHY_VHT_CAP(wlc->band)) {
			wlc->pub->_vht_enab = 1;
		}
		/* add the mcs rates to the default and hw ratesets */
		wlc_rateset_mcs_build(&wlc->default_bss->rateset, wlc->stf->op_rxstreams);
		wlc_rateset_filter(&wlc->default_bss->rateset, &wlc->default_bss->rateset,
			FALSE, WLC_RATES_CCK_OFDM, RATE_MASK_FULL, wlc_get_mcsallow(wlc, NULL));

		for (i = 0; i < NBANDS(wlc); i++) {
			band = (NBANDS(wlc) == 1) ? wlc->band : wlc->bandstate[i];

			memcpy(band->hw_rateset.mcs,
				wlc->default_bss->rateset.mcs, MCSSET_LEN);
		}
		break;

	default:
		ASSERT(0);
		break;
	}

	/* The default ratest is changed to the new one.
	 * Make sure the defrateset_override is no longer valid.
	 */
	if (err == 0)
	  wlc->defrateset_override = FALSE;

	return err;
} /* wlc_set_nmode */

static int
wlc_setup_nmode(wlc_info_t *wlc, int nmode)
{
	int err;

	err = wlc_nmode_validate(wlc, nmode);

	if (err) {
		WL_ERROR(("%s: bail error\n", __FUNCTION__));
		return err;
	}
	/* save user pref */
	wlc_prot_n_cfg_set(wlc->prot_n, WLC_PROT_N_USER, nmode);

	/* do not update our current operating nmode if country regulations
	 * force nmode off
	 */
	if (wlc_channel_locale_flags(wlc->cmi) & WLC_NO_MIMO) {
		ASSERT(!N_ENAB(wlc->pub));
		return 0;
	}

	err = wlc_set_nmode(wlc->hti, nmode);
	/* wlc_set_nmode() should only return an error if the call
	 * to wlc_nmode_validate() above returned an error. Make an
	 * assertion that the two functions remain in sync.
	 */
	ASSERT(!err);

	return err;
}

void
wlc_ht_get_scb_ampdu_params(wlc_ht_info_t *hti, struct scb *scb,
	uint8* peer_density, uint32* max_rxlen, uint8* max_rxlen_factor)
{
	wlc_ht_scb_info_priv_t *cubby;
	uint8 params;
	cubby = SCB_HT_INFO_PRIV(hti, scb);
	params = cubby->ht_ampdu_params;

	*peer_density = (params & HT_PARAMS_DENSITY_MASK) >> HT_PARAMS_DENSITY_SHIFT;

	/* max_rxlen is only used for host aggregation (corerev < 40) */
	*max_rxlen = AMPDU_RX_FACTOR_BASE <<
		(params & HT_PARAMS_RX_FACTOR_MASK);

	*max_rxlen_factor =
		(params & HT_PARAMS_RX_FACTOR_MASK) +
		AMPDU_RX_FACTOR_BASE_PWR;

	return;
}

void
wlc_ht_publicaction(wlc_ht_info_t *pub, wlc_bsscfg_t *cfg, struct dot11_management_header *hdr,
	uint8 *body, int body_len, wlc_d11rxhdr_t *wrxh)
{
	wlc_ht_priv_info_t *hti;
	wlc_info_t *wlc;
	uint8 action_id;

	hti = WLC_HT_INFO_PRIV(pub);
	wlc = hti->wlc;

	WL_TRACE(("wl%d: wlc_ht_publicaction\n", wlc->pub->unit));

	action_id = body[DOT11_ACTION_ACT_OFF];

	WL_INFORM(("wl%d: %s: recv Action frame %d\n", wlc->pub->unit, __FUNCTION__, action_id));

	switch (action_id) {
	case DOT11_PUB_ACTION_BSS_COEX_MNG:
		wlc_recv_public_coex_action(wlc, cfg, hdr, body, body_len, wrxh);
		break;
	case DOT11_PUB_ACTION_CHANNEL_SWITCH:
		if (WL11H_ENAB(wlc))
			wlc_recv_public_csa_action(wlc->csa, hdr, body, body_len);
		break;
	default:
		WL_INFORM(("wl %d: unrecognized Public action frame\n", wlc->pub->unit));
		break;
	}
}

void
wlc_frameaction_ht(wlc_ht_info_t *pub, uint action_id, struct scb *scb,
	struct dot11_management_header *hdr, uint8 *body, int body_len)
{
	struct dot11_action_ht_mimops *ht_mimops;
	struct dot11_action_ht_ch_width *ht_ch_width;
	uint32 scb_is40_old;
	wlc_bsscfg_t *cfg;
	wlc_ht_scb_info_priv_t *cubby;
	wlc_ht_scb_info_pub_t *scb_pub;

	wlc_ht_priv_info_t *hti;
	wlc_info_t *wlc;
	if (!pub || !scb) {
		return;
	}

	hti = WLC_HT_INFO_PRIV(pub);
	wlc = hti->wlc;

	scb_pub = SCB_HT_INFO_PUB(pub, scb);

	cubby = SCB_HT_INFO_PRIV(pub, scb);

	cfg = SCB_BSSCFG(scb);
	ASSERT(cfg != NULL);
	BCM_REFERENCE(cfg);

	WL_INFORM(("wl%d: Rcvd HT action frame with id %d\n", wlc->pub->unit, action_id));

	switch (action_id) {
	case DOT11_ACTION_ID_HT_MIMO_PS:
		ht_mimops = (struct dot11_action_ht_mimops *)body;

		/* update the cached mimo_psmode */
		if (N_ENAB(wlc->pub)) {
			bool mimops_enabled = scb_pub->ht_mimops_enabled;
			bool mimops_rtsmode = scb_pub->ht_mimops_rtsmode;
			/* 802.11n Draft 1.04 spec support 4 bytes format
			 * and Draft 1.06 spec support 3 bytes format
			 */
			if (body_len == 3) {
				mimops_enabled =
				        ((ht_mimops->control & SM_PWRSAVE_ENABLE) ? TRUE:FALSE);
				mimops_rtsmode =
				        ((ht_mimops->control & SM_PWRSAVE_MODE) ? TRUE:FALSE);
			} else if (body_len == 4) {
				mimops_enabled = (ht_mimops->control ? TRUE:FALSE);
				mimops_rtsmode = (*(&ht_mimops->control+1) ? TRUE:FALSE);
			} else
				break;

			if ((scb_pub->ht_mimops_enabled != mimops_enabled) ||
			    (scb_pub->ht_mimops_rtsmode != mimops_rtsmode)) {
				scb_pub->ht_mimops_enabled = mimops_enabled;
				scb_pub->ht_mimops_rtsmode = mimops_rtsmode;
				wlc_scb_ratesel_init(wlc, scb);
			}
		}
		break;

	case DOT11_ACTION_ID_HT_CH_WIDTH:
		ht_ch_width = (struct dot11_action_ht_ch_width *)body;

		/* select the desired tx bandwidth (per frametype) */
		scb_is40_old = scb->flags & SCB_IS40;
		if (ht_ch_width->ch_width && (cubby->ht_capabilities & HT_CAP_40MHZ)) {
			scb->flags |= SCB_IS40;
		} else {
			scb->flags &= ~SCB_IS40;
		}

		/* if bandwidth changed, reinit rate selection */
		if (scb_is40_old != (scb->flags & SCB_IS40)) {
			wlc_scb_ratesel_init(wlc, scb);
		}

		/* Check for association of a 20MHz only HT STA in 40MHz operating */
		if (BSSCFG_AP(cfg))
			wlc_prot_n_cond_upd(wlc->prot_n, scb);
		break;

	default:
		WL_ERROR(("wl %d: unrecognized HT action frame\n", wlc->pub->unit));
		break;
	}
}

/**
 * This function is called on e.g. joining a network, receiving a beacon or on IOVAR handling.
 * The routine extracts HT configuration info from the supplied capability and additional IEs
 * and stores it in the scb ('communication partner') struct, currently this state includes:
 *	HT capable flag, AMPDU params, GF support, MIMO PS mode, RIFS support, AMSDU max size,
 *      preferred txbw for MIMO, ODFM and CCK
 * 'upd_mimo_ps' indicates if SM PS mode needs to be updated, valid only when cap_ie is not NULL.
 */
void
wlc_ht_update_scbstate(wlc_ht_info_t *pub, struct scb *scb,
	ht_cap_ie_t *cap_ie, ht_add_ie_t *add_ie, obss_params_t *obss_params)
{
	uint16 cap;
	bool allow_40Mhz = FALSE;
	bool reinit_ratesel = FALSE;
	bool scbwas40 = (scb->flags & SCB_IS40) != 0;
	bool ht_cap = SCB_HT_CAP(scb);
	uint32 scb_old_sgi = scb->flags2 & (SCB2_SGI20_CAP | SCB2_SGI40_CAP);
	wlc_rateset_t new_rateset = {0};
	bool update_mcsset = FALSE;
	wlc_bsscfg_t *bsscfg;
	wlc_info_t *wlc;
	wlc_ht_scb_info_priv_t *cubby;
	wlc_ht_scb_info_pub_t *scb_pub;

	wlc_ht_priv_info_t *hti;
	hti = WLC_HT_INFO_PRIV(pub);

	ASSERT(scb != NULL);

	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	wlc = hti->wlc;
	cubby = SCB_HT_INFO_PRIV(pub, scb);
	scb_pub = &(SCB_HT_CUBBY(pub, scb)->pub);

	/* clear HT based features */
	scb->flags &= ~(SCB_HTCAP | SCB_AMSDUCAP | SCB_AMPDUCAP |
		SCB_NONGF | SCB_IS40 | SCB_RIFSCAP | SCB_HT40INTOLERANT | SCB_STBCCAP);
	scb->flags2 &= ~(SCB2_SGI20_CAP | SCB2_SGI40_CAP | SCB2_LDPCCAP);

	/* check if there is a HT capability ie */
	if (cap_ie) {
		cap = ltoh16_ua(&cap_ie->cap);
		/* Store the HT config */
		cubby->ht_capabilities = cap;
		cubby->ht_ampdu_params = cap_ie->params;

		/* Mark the SCB as HT capable */
		scb->flags |= SCB_HTCAP;

		if (WLPROPRIETARY_11N_RATES_ENAB(wlc->pub) &&
			wlc->pub->ht_features == WLC_HT_FEATURES_PROPRATES_FORCE)
			scb->flags2 |= SCB2_HT_PROP_RATES_CAP;
#ifdef WL_BEAMFORMING
			if (TXBF_ENAB(wlc->pub)) {
				if (TXBF_N_SUPPORTED_BFR(cap_ie->txbf_cap))
					scb->flags3 |= SCB3_HT_BEAMFORMER;
				if (TXBF_N_SUPPORTED_BFE(cap_ie->txbf_cap))
					scb->flags3 |= SCB3_HT_BEAMFORMEE;
				wlc_txbf_scb_state_upd(wlc->txbf, scb, cap_ie->txbf_cap);
			}
#endif /* WL_BEAMFORMING */

#ifdef WLAMPDU
		/* mark ampdu flag and add AMPDU to txpath
		 * depending on the crypto settings for the SCB
		 * By spec, 11n device can send AMPDU only with Open or CCMP crypto.
		 */
		if (AMPDU_ENAB(wlc->pub)) {
			scb->flags |= SCB_AMPDUCAP;
			/* When AMPDU is not already configured, add AMPDU to tx mod
			 * if MFP is disabled
			 * or SCB is already authorized (despite MFP being enabled).
			 * SCB_MFP(scb) would be false for open / legacy WEP security.
			 */
			if (!SCB_TXMOD_CONFIGURED(scb, TXMOD_AMPDU) &&
					(!SCB_MFP(scb) || SCB_AUTHORIZED(scb))) {
				wlc_scb_ampdu_enable(wlc, scb);
			}
		} else {
			/* remove AMPDU from tx mod when wlc->pub->_ampdu_tx or rx
			 * is disabled
			 */
			wlc_scb_ampdu_disable(wlc, scb);
		}
#endif /* WLAMPDU */

#ifdef WLAMSDU_TX
		/* remote station is AMSDU capable? */
		if (AMSDU_TX_ENAB(wlc->pub)) {
			uint32 old_mtu = cubby->amsdu_ht_mtu_pref;

			scb->flags |= SCB_AMSDUCAP;
			cubby->amsdu_ht_mtu_pref = (cap & HT_CAP_MAX_AMSDU) ?
				HT_MAX_AMSDU : HT_MIN_AMSDU;
			/* if mtu changed, update AMSDU agg bytes */
			if (old_mtu != cubby->amsdu_ht_mtu_pref)
				wlc_amsdu_scb_agglimit_upd(wlc->ami, scb);
			/* only active amsdu agg when it's enabled */
			wlc_txmod_config(wlc, scb, TXMOD_AMSDU);
		} else
			wlc_txmod_unconfig(wlc, scb, TXMOD_AMSDU);
#endif /* WLAMSDU_TX */

		if (!(scb->flags2 & SCB2_IGN_SMPS)) {
			bool old_mimops = scb_pub->ht_mimops_enabled;
			bool old_mimops_rtsmode = scb_pub->ht_mimops_rtsmode;

			switch ((cap & HT_CAP_MIMO_PS_MASK) >> HT_CAP_MIMO_PS_SHIFT) {
			case HT_CAP_MIMO_PS_ON :
				scb_pub->ht_mimops_enabled = TRUE;
				scb_pub->ht_mimops_rtsmode = FALSE;
				break;
			case HT_CAP_MIMO_PS_RTS :
				scb_pub->ht_mimops_enabled = TRUE;
				scb_pub->ht_mimops_rtsmode = TRUE;
				break;
			case HT_CAP_MIMO_PS_OFF :
				scb_pub->ht_mimops_enabled = FALSE;
				scb_pub->ht_mimops_rtsmode = FALSE;
				break;
			default:
				scb_pub->ht_mimops_enabled = FALSE;
				scb_pub->ht_mimops_rtsmode = FALSE;
				WL_ERROR(("wl%d: %s, incorrect psmode\n",
				          wlc->pub->unit, __FUNCTION__));
				break;
			}

			if ((old_mimops != scb_pub->ht_mimops_enabled) ||
			    (old_mimops_rtsmode != scb_pub->ht_mimops_rtsmode))
				reinit_ratesel = TRUE;
		}

		/* mark peer as stbc capable if it supports ANY num of stbc streams */
		if (cap & HT_CAP_RX_STBC_MASK)
			scb->flags |= SCB_STBCCAP;

		if (cap & HT_CAP_LDPC_CODING)
			scb->flags2 |= SCB2_LDPCCAP;

		if (cap & HT_CAP_SHORT_GI_20)
			scb->flags2 |= SCB2_SGI20_CAP;

		if (cap & HT_CAP_SHORT_GI_40)
			scb->flags2 |= SCB2_SGI40_CAP;

		if (scb_old_sgi != (scb->flags2 & (SCB2_SGI20_CAP | SCB2_SGI40_CAP)))
			reinit_ratesel = TRUE;

		if (!(cap & HT_CAP_GF))
			scb->flags |= SCB_NONGF;

		if (cap & HT_CAP_40MHZ_INTOLERANT)
			scb->flags |= SCB_HT40INTOLERANT;

		allow_40Mhz = (cap & HT_CAP_40MHZ) != 0;
		if (add_ie) {
			bcopy(add_ie, &hti->add_ie, sizeof(ht_add_ie_t));
			if (allow_40Mhz)
				allow_40Mhz = (add_ie->byte1 & HT_BW_ANY) != 0;
			if (add_ie->byte1 & HT_RIFS_PERMITTED)
				scb->flags |= SCB_RIFSCAP;
			if (DOT11N_TXBURST_PRESENT(add_ie)) {
				hti->txburst_limit =
					(CHSPEC_WLCBANDUNIT(bsscfg->current_bss->chanspec) ==
					BAND_5G_INDEX) ?
					DOT11N_5G_TXBURST_LIMIT : DOT11N_2G_TXBURST_LIMIT;
				wlc_ht_set_txburst_limit(pub, bsscfg, hti->txburst_limit);
			} else {
				hti->txburst_limit = 0;
				wlc_ht_set_txburst_limit(pub, bsscfg, 0);
			}
		}

		wlc_obss_update_scbstate(wlc->obss, bsscfg, obss_params);

#if defined(PKTC) || defined(PKTC_DONGLE)
		/* Enable packet chaining for WDS scb
		 * a) if security is disabled;
		 * b) if security is enabled and security type is WEP;
		 * For all other security types will enable chaining
		 * once scb is authorized.
		 */
		if (SCB_LEGACY_WDS(scb) &&
			(!WSEC_ENABLED(bsscfg->wsec) || WSEC_WEP_ENABLED(bsscfg->wsec))) {
			wlc_scb_pktc_enable(scb, NULL);
		}
#endif /* PKTC || PKTC_DONGLE */
	} else {
		/* unconfig HT based features */
		wlc_txmod_unconfig(wlc, scb, TXMOD_AMSDU);
#ifdef WLAMPDU
		wlc_scb_ampdu_disable(wlc, scb);
#endif // endif
#if defined(PKTC) || defined(PKTC_DONGLE)
		wlc_scb_pktc_disable(scb);
#endif /* PKTC || PKTC_DONGLE */
	}

	/* Merge incoming rate with MCS rate of this device and store
	   temporarily in new_rateset.mcs[]
	*/
	if (cap_ie) {
		bcopy(&cap_ie->supp_mcs[0], &new_rateset.mcs[0], MCSSET_LEN);
		wlc_rateset_mcs_upd(&new_rateset, wlc->stf->op_txstreams);
		wlc_rateset_filter(&new_rateset, &new_rateset, FALSE, WLC_RATES_CCK_OFDM,
		                   RATE_MASK_FULL, wlc_get_mcsallow(wlc, wlc->cfg));
	}

	/* update the mcs rateset when mcs rateset is changed */
	if (ht_cap && SCB_HT_CAP(scb)) {
		uint i;
		for (i = 0; i < MCSSET_LEN; i++) {
			if (new_rateset.mcs[i] != scb->rateset.mcs[i]) {
				update_mcsset = TRUE;
				break;
			}
		}
	}

	/* update the mcs rateset when ht cap changes */
	if ((ht_cap != SCB_HT_CAP(scb)) || update_mcsset) {
		if (SCB_HT_CAP(scb))
			bcopy(&new_rateset.mcs[0], &scb->rateset.mcs[0], MCSSET_LEN);
		else
			bzero(&scb->rateset.mcs[0], MCSSET_LEN);
		reinit_ratesel = TRUE;
	}

	if (allow_40Mhz) {
		scb->flags |= SCB_IS40;
	}

	if (scbwas40 != allow_40Mhz)
		reinit_ratesel = TRUE;

	/* refresh rateselection if bw or psmode changed */
	if (reinit_ratesel) {
		/* skip updating the ratesel if txnss_override is set */
		if (wlc->stf->txstream_value == 0) {
			wlc_scb_ratesel_init(wlc, scb);
		}
	}
} /* wlc_ht_update_scbstate */

static uint16
wlc_ht_get_scb_cap(wlc_ht_info_t *hti, struct scb *scb)
{
	wlc_ht_scb_info_priv_t *cubby;
	if (!hti || !scb) {
		return 0;
	}
	cubby = SCB_HT_INFO_PRIV(hti, scb);

	return cubby->ht_capabilities;
}

bool
wlc_ht_is_scb_40MHZ_cap(wlc_ht_info_t *hti, struct scb *scb)
{
	return (wlc_ht_get_scb_cap(hti, scb) & HT_CAP_40MHZ);
}

uint16
wlc_ht_get_scb_amsdu_mtu_pref(wlc_ht_info_t *hti, struct scb *scb)
{
	wlc_ht_scb_info_priv_t *cubby;
	if (!hti || !scb) {
		return 0;
	}
	cubby = SCB_HT_INFO_PRIV(hti, scb);
	return cubby->amsdu_ht_mtu_pref;
}

/* Deprecated - use wlc_update_bwcap() instead. */
static void
wlc_update_mimo_band_bwcap(wlc_info_t *wlc, uint8 bwcap)
{
	wlcband_t *band;
	wlc_ht_priv_info_t *hti;
	if (!wlc->hti) {
		return;
	}
	hti = WLC_HT_INFO_PRIV(wlc->hti);

	/* 2G Band setup
	 * Skip if a single band 5G device
	 */
	band = wlc->bandstate[BAND_2G_INDEX];
	if (bwcap == WLC_N_BW_40ALL) { /* Enable 20MHz and 40MHz */
		band->bw_cap = WLC_BW_CAP_40MHZ;
	} else {
		band->bw_cap = WLC_BW_CAP_20MHZ;
	}

	/* 5G Band setup
	 * Only setup if dual band or single band 5G device
	 */
	if (NBANDS(wlc) == 2 || IS_SINGLEBAND_5G(wlc->deviceid)) {
		band = wlc->bandstate[BAND_5G_INDEX];

		if ((bwcap == WLC_N_BW_40ALL) ||
				(bwcap == WLC_N_BW_20IN2G_40IN5G)) {
			/* Enable 20MHz, 40MHz */
			band->bw_cap |= WLC_BW_CAP_40MHZ;
		} else { /* Enable 20MHz, Disable 40MHz */
			band->bw_cap |= WLC_BW_CAP_20MHZ;
			band->bw_cap &= ~WLC_BW_40MHZ_BIT;
		}

	}

	hti->mimo_band_bwcap = bwcap;

	wlc_ht_update_coex_support(wlc, wlc->pub->_coex);
}

#ifdef BCMDBG
void
wlc_get_mcsset(wlc_ht_info_t *pub, void *arg, bool istx)
{
	wlc_ht_priv_info_t *hti;
	wlc_info_t *wlc;
	uint8 streams;
	uint8 *mcs = (uint8*)arg;
	int i;

	hti = WLC_HT_INFO_PRIV(pub);
	wlc = hti->wlc;
	streams = istx ? wlc->stf->op_txstreams : wlc->stf->op_rxstreams;

	memset(mcs, 0, MCSSET_LEN);
	for (i = 0; i < streams; i++)
		mcs[i] = 0xff;
}
#endif /* BCMDBG */

void
BCMATTACHFN(wlc_ht_nvm_overrides)(wlc_ht_info_t *pub, uint n_disabled)
{
	wlc_ht_priv_info_t *hti;
	wlc_info_t *wlc;
	hti = WLC_HT_INFO_PRIV(pub);
	wlc = hti->wlc;

	/* *******nvram 11n config overrides Start ********* */

	/* apply the sgi override from nvram conf */
	if (n_disabled & WLFEATURE_DISABLE_11N_SGI_TX)
		pub->sgi_tx = OFF;

	if (n_disabled & WLFEATURE_DISABLE_11N_SGI_RX)
		wlc_ht_update_sgi_rx(wlc->hti, 0);

	/* apply the stbc override from nvram conf */
	if (n_disabled & WLFEATURE_DISABLE_11N_STBC_TX) {
		wlc->bandstate[BAND_2G_INDEX]->band_stf_stbc_tx = OFF;
		wlc->bandstate[BAND_5G_INDEX]->band_stf_stbc_tx = OFF;
		hti->cap_ie.cap &= ~HT_CAP_TX_STBC;
		wlc_vht_set_rx_stbc_cap(wlc->vhti, FALSE);
	}
	if (n_disabled & WLFEATURE_DISABLE_11N_STBC_RX)
		wlc_stf_stbc_rx_set(wlc, HT_CAP_RX_STBC_NO);

	/* apply the GF override from nvram conf */
	if (n_disabled & WLFEATURE_DISABLE_11N_GF)
		hti->cap_ie.cap &= ~HT_CAP_GF;
}

static int
wlc_update_bwcap(wlc_ht_info_t *pub, int bandtype, uint8 bwcap)
{
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	wlc_info_t *wlc = hti->wlc;
	wlcband_t *band = NULL;
	int err = 0;

	if (bandtype == WLC_BAND_5G) {
		if ((NBANDS(wlc) != 2) && !IS_SINGLEBAND_5G(wlc->deviceid))
			return BCME_BADBAND;

		band = wlc->bandstate[BAND_5G_INDEX];
	} else if (bandtype == WLC_BAND_2G) {
		if (IS_SINGLEBAND_5G(wlc->deviceid))
			return BCME_BADBAND;

		band = wlc->bandstate[BAND_2G_INDEX];
	} else {
		return BCME_BADARG;
	}

	switch (bwcap) {
	case WLC_BW_CAP_20MHZ:
		/* No validation required */
		break;

	case WLC_BW_CAP_40MHZ:
		/* Must be 11N capable to set 40 MHz */
		if (!wlc->pub->phy_bw40_capable)
			err = BCME_RANGE;
		break;

	case WLC_BW_CAP_80MHZ:
		/* Only allow 80MHz cap if 5GHz band and phy supports it */
		if ((bandtype != WLC_BAND_5G) || !wlc->pub->phy_bw80_capable)
			err = BCME_RANGE;
		break;

	case WLC_BW_CAP_160MHZ:
		/* Only allow 160MHz cap if 5GHz band and phy supports it */
		if ((bandtype != WLC_BAND_5G) ||
			!(wlc->pub->phy_bw160_capable || wlc->pub->phy_bw8080_capable))
			err = BCME_RANGE;
		break;

	case WLC_BW_CAP_UNRESTRICTED:
		if ((bandtype == WLC_BAND_5G) && (wlc->pub->phy_bw160_capable))
			bwcap = WLC_BW_CAP_160MHZ;
		else if ((bandtype == WLC_BAND_5G) && (wlc->pub->phy_bw8080_capable))
			bwcap = WLC_BW_CAP_160MHZ;
		else if ((bandtype == WLC_BAND_5G) && (wlc->pub->phy_bw80_capable))
			bwcap = WLC_BW_CAP_80MHZ;
		else if (wlc->pub->phy_bw40_capable)
			bwcap = WLC_BW_CAP_40MHZ;
		else
			bwcap = WLC_BW_CAP_20MHZ;
#ifdef WL11ULB
		if (ULB_ENAB(wlc->pub)) {
			int ulb_bw_cap = 0;

			if (WLC_2P5MHZ_ULB_SUPP_HW(wlc))
				ulb_bw_cap |= WLC_BW_CAP_2P5MHZ;

			if (WLC_5MHZ_ULB_SUPP_HW(wlc))
				ulb_bw_cap |= WLC_BW_CAP_5MHZ;

			if (WLC_10MHZ_ULB_SUPP_HW(wlc))
				ulb_bw_cap |= WLC_BW_CAP_10MHZ;

			bwcap |= ulb_bw_cap;
		}
#endif /* WL11ULB */
		break;

	default:
#ifdef WL11ULB
		if (bwcap & (WLC_BW_CAP_2P5MHZ | WLC_BW_CAP_5MHZ | WLC_BW_CAP_10MHZ)) {
			if (ULB_ENAB(wlc->pub)) {
				if (((bwcap & WLC_BW_CAP_2P5MHZ) && !WLC_2P5MHZ_ULB_SUPP_HW(wlc)) ||
					((bwcap & WLC_BW_CAP_5MHZ) && !WLC_5MHZ_ULB_SUPP_HW(wlc)) ||
					((bwcap & WLC_BW_CAP_10MHZ) &&
						!WLC_10MHZ_ULB_SUPP_HW(wlc))) {
					err = BCME_RANGE;
				}
			} else
				err = BCME_NOTENABLED;
		} else
#endif /* WL11ULB */
		{
			WL_ERROR(("wl%d: %s:Bandwidth capability:%d Unsupported\n",
				WLCWLUNIT(wlc), __FUNCTION__, bwcap));
			err = BCME_BADARG;
		}
		break;
	}

	if (err == 0) {
		band->bw_cap = bwcap;
		if (bwcap == WLC_BW_CAP_160MHZ && DYN160_ACTIVE(wlc->pub)) {
			wlc_vht_set_ext_nss_bw_sup(wlc->vhti, VHT_CAP_INFO_EXT_NSS_BW_HALF_160);
		}
		wlc_ht_update_coex_support(wlc, wlc->pub->_coex);
	}

	return err;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_dump_htcap(wlc_ht_info_t *pub, struct bcmstrbuf *b)
{
	uint16 stbc_val, mimo_ps_val;
	uint16 val;
	wlc_ht_priv_info_t *hti;
	wlc_info_t *wlc;

	hti = WLC_HT_INFO_PRIV(pub);
	wlc = hti->wlc;
	val = hti->cap_ie.cap;

	if ((wlc_channel_locale_flags(wlc->cmi) & WLC_NO_40MHZ) ||
	    (!WL_BW_CAP_40MHZ(wlc->band->bw_cap)) ||
	    (wlc->cfg != NULL &&
	     (BSSCFG_AP(wlc->cfg) || (!wlc->cfg->BSS && !BSSCFG_IS_TDLS(wlc->cfg))) &&
	     CHSPEC_IS20(wlc->cfg->current_bss->chanspec))) {
		val &= ~HT_CAP_40MHZ;
		val &= ~HT_CAP_SHORT_GI_40;
	}

	bcm_bprintf(b, "HT dump:\n");

	bcm_bprintf(b, "HT Cap 0X%04x\n", hti->cap_ie.cap);
	if (val & HT_CAP_LDPC_CODING)
		bcm_bprintf(b, "LDPC ");
	if (val & HT_CAP_40MHZ)
		bcm_bprintf(b, "40MHz ");

	mimo_ps_val = (val & HT_CAP_MIMO_PS_MASK) >> HT_CAP_MIMO_PS_SHIFT;
	if (mimo_ps_val == HT_CAP_MIMO_PS_ON)
		bcm_bprintf(b, "MIMO-PS-ON ");
	else if (mimo_ps_val == HT_CAP_MIMO_PS_RTS)
		bcm_bprintf(b, "MIMO-PS-RTS ");
	else if (mimo_ps_val == HT_CAP_MIMO_PS_OFF)
		bcm_bprintf(b, "MIMO-PS-OFF ");

	if (val & HT_CAP_GF)
		bcm_bprintf(b, "GF ");
	if (val & HT_CAP_SHORT_GI_20)
		bcm_bprintf(b, "SGI-20 ");
	if (val & HT_CAP_SHORT_GI_40)
		bcm_bprintf(b, "SGI-40 ");
	if (val & HT_CAP_TX_STBC)
		bcm_bprintf(b, "STBC-TX ");

	stbc_val = (val & HT_CAP_RX_STBC_MASK) >> HT_CAP_RX_STBC_SHIFT;
	if (stbc_val == HT_CAP_RX_STBC_ONE_STREAM)
		bcm_bprintf(b, "STBC-RX-1SS ");
	else if (stbc_val == HT_CAP_RX_STBC_TWO_STREAM)
		bcm_bprintf(b, "STBC-RX-2SS ");
	else if (stbc_val == HT_CAP_RX_STBC_THREE_STREAM)
		bcm_bprintf(b, "STBC-RX-3SS ");

	if (val & HT_CAP_DELAYED_BA)
		bcm_bprintf(b, "Delay-BA ");
	if (val & HT_CAP_MAX_AMSDU)
		bcm_bprintf(b, "AMSDU-Max ");
	if (val & HT_CAP_DSSS_CCK)
		bcm_bprintf(b, "DSSS-CCK ");
	if (val & HT_CAP_PSMP)
		bcm_bprintf(b, "PSMP ");
	if (val & HT_CAP_40MHZ_INTOLERANT)
		bcm_bprintf(b, "40-Intol ");
	if (val & HT_CAP_LSIG_TXOP)
		bcm_bprintf(b, "LSIG-TXOP ");
	wlc_dump_mcsset("\nhw_mcsset ", &wlc->band->hw_rateset.mcs[0], b);
	bcm_bprintf(b, "\n");
	return 0;
}
#endif	/* BCMDBG || BCMDBG_DUMP */

int
wlc_ht_add_ie_verify_rates(wlc_ht_info_t *pub, uint8 *rates, int len)
{
	int i;
	wlc_ht_priv_info_t *hti;
	hti = WLC_HT_INFO_PRIV(pub);

	for (i = 0; i < MCSSET_LEN; i++) {
		if ((hti->add_ie.basic_mcs[i] & rates[i]) !=
			hti->add_ie.basic_mcs[i]) {
			WL_ERROR(("wl%d: %s: rate verification fail; idx=%d rate=%x\n",
				hti->wlc->pub->unit, __FUNCTION__, i, rates[i]));
			return BCME_ERROR;
		}
	}
	return BCME_OK;
}

uint8
wlc_ht_get_wsec_restriction(wlc_ht_info_t *pub)
{
	wlc_ht_priv_info_t *hti;
	hti = WLC_HT_INFO_PRIV(pub);

	return hti->ht_wsec_restriction;
}

uint8
wlc_ht_get_mimo_band_bwcap(wlc_ht_info_t *pub)
{
	wlc_ht_priv_info_t *hti;
	hti = WLC_HT_INFO_PRIV(pub);

	return hti->mimo_band_bwcap;
}

static void
wlc_frameburst_size(wlc_info_t *wlc)
{
	if (!wlc->pub->up)
		return;

	wlc_write_shm(wlc, M_MBURST_SIZE, MAXTXFRAMEBURST);

	/* frameburst size changed/set; invalidate tx cache if there */
	if (WLC_TXC_ENAB(wlc) && wlc->txc) {
		wlc_txc_inv_all(wlc->txc);
	}
}

/* Configures frameburst txop threshold in usec */
static void
wlc_ht_frameburst_txop_set(wlc_ht_info_t *pub, uint16 val)
{
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);
	wlc_info_t *wlc = hti->wlc;

	if (!wlc->clk) {
		return;
	}
	if (pub->_rifs) {
		wlc_write_shm(wlc, M_MBURST_TXOP, (EDCF_AC_VO_TXOP_AP << 5));
	} else {
		wlc_write_shm(wlc, M_MBURST_TXOP, val);
	}

	/* Also, enable beacon txop limit for EU edcrs */
	if (wlc_is_edcrs_eu(wlc)) {
		wlc_mhf(wlc, MHF2, MHF2_LMT_TXOP,
			MHF2_LMT_TXOP, WLC_BAND_ALL);
	} else {
		wlc_mhf(wlc, MHF2, MHF2_LMT_TXOP,
			0, WLC_BAND_ALL);
	}

	/* frameburst txop threshold set; invalidate tx cache if there */
	if (WLC_TXC_ENAB(wlc) && wlc->txc) {
		wlc_txc_inv_all(wlc->txc);
	}
}

/* Returns frameburst txop threshold in usec */
static uint16
wlc_ht_frameburst_txop_get(wlc_ht_info_t *pub)
{
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);
	wlc_info_t *wlc = hti->wlc;
	uint16 val;

	if (pub->_rifs) {
		val = (EDCF_AC_VO_TXOP_AP << 5);
	} else {
		if (wlc->clk) {
			val = wlc_read_shm(wlc, M_MBURST_TXOP);
		} else {
			val = MIN(pub->max_fbtxop_user, pub->max_fbtxop_country);
		}
	}

	return val;
}

/* honor country specific frameburst txop limits and set on ucode */
void
wlc_ht_frameburst_limit(wlc_ht_info_t *pub)
{
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);
	wlc_info_t *wlc = hti->wlc;

	/* honor max frameburst txop setting based on current country code */
	pub->max_fbtxop_country = (uint16)(wlc_is_edcrs_eu(wlc) ? MAXFRAMEBURST_TXOP_EU :
			MAXFRAMEBURST_TXOP);

	wlc_ht_frameburst_txop_set(pub, MIN(pub->max_fbtxop_user, pub->max_fbtxop_country));
}

uint
wlc_calc_ba_time(wlc_ht_info_t* pub, ratespec_t rspec, uint8 preamble_type)
{
	wlc_ht_priv_info_t *hti;
	wlc_info_t *wlc;

	hti = WLC_HT_INFO_PRIV(pub);

	wlc = hti->wlc;

	WL_TRACE(("wl%d: wlc_calc_ba_time: rspec 0x%x, preamble_type %d\n",
		wlc->pub->unit, rspec, preamble_type));
	/* Spec 9.6: ack rate is the highest rate in BSSBasicRateSet that is less than
	 * or equal to the rate of the immediately previous frame in the FES
	 */
	rspec = WLC_BASIC_RATE(wlc, rspec);
	ASSERT(VALID_RATE_DBG(wlc, rspec));

	/* BA len == 32 == 16(ctl hdr) + 4(ba len) + 8(bitmap) + 4(fcs) */
	return wlc_calc_frame_time(wlc, rspec, preamble_type,
		(DOT11_CTL_HDR_LEN + DOT11_BA_LEN + DOT11_BA_CMP_BITMAP_LEN + DOT11_FCS_LEN));
}

uint
wlc_ht_calc_frame_len(wlc_ht_info_t* pub, ratespec_t ratespec, uint8 preamble_type,
	uint dur)
{
	uint nsyms, kNdps;
	uint mcs = ratespec & RSPEC_RATE_MASK;
	int tot_streams = wlc_ratespec_nsts(ratespec);

	wlc_ht_priv_info_t *hti;
	wlc_info_t *wlc;
	hti = WLC_HT_INFO_PRIV(pub);

	wlc = hti->wlc;

	ASSERT(WLC_PHY_11N_CAP(wlc->band));
	dur -= PREN_PREAMBLE + ((tot_streams - 1) * PREN_PREAMBLE_EXT);
	/* payload calculation matches that of regular ofdm */
	if (BAND_2G(wlc->band->bandtype))
		dur -= DOT11_OFDM_SIGNAL_EXTENSION;
	/* kNdbps = kbps * 4 */
	kNdps = MCS_RATE(mcs, RSPEC_IS40MHZ(ratespec), RSPEC_ISSGI(ratespec)) * 4;
	nsyms = dur / APHY_SYMBOL_TIME;
	return ((nsyms * kNdps) - ((APHY_SERVICE_NBITS + APHY_TAIL_NBITS)*1000)) / 8000;
}

uint
wlc_ht_calc_frame_time(wlc_ht_info_t* pub, ratespec_t ratespec, uint8 preamble_type,
	uint mac_len)
{
	wlc_ht_priv_info_t *hti;
	wlc_info_t *wlc;
	uint dur = 0, kNdps;
	uint nsyms;

	uint mcs = ratespec & RSPEC_RATE_MASK;
	int tot_streams = wlc_ratespec_nsts(ratespec);

	hti = WLC_HT_INFO_PRIV(pub);

	wlc = hti->wlc;

	ASSERT(WLC_PHY_11N_CAP(wlc->band));
	ASSERT(WLC_IS_MIMO_PREAMBLE(preamble_type));
	ASSERT(VALID_MCS(mcs));

	if (!VALID_MCS(mcs)) {
		mcs = 0;
	}

	dur = PREN_PREAMBLE + (tot_streams*PREN_PREAMBLE_EXT);
	if (preamble_type == WLC_MM_PREAMBLE)
		dur += PREN_MM_EXT;
	/* 1000Ndbps = kbps * 4 */
	kNdps = MCS_RATE(mcs, RSPEC_IS40MHZ(ratespec), RSPEC_ISSGI(ratespec)) * 4;

	if (!RSPEC_ISSTBC(ratespec))
		/* NSyms = CEILING((SERVICE + 8*NBytes + TAIL) / Ndbps) */
		nsyms = CEIL(
			(APHY_SERVICE_NBITS + 8 * mac_len + APHY_TAIL_NBITS)*1000,
			kNdps);
	else
		/* STBC needs to have even number of symbols */
		nsyms = 2 * CEIL((APHY_SERVICE_NBITS + 8 * mac_len + APHY_TAIL_NBITS)*1000,
			2 * kNdps);

	dur += APHY_SYMBOL_TIME * nsyms;
	if (BAND_2G(wlc->band->bandtype))
		dur += DOT11_OFDM_SIGNAL_EXTENSION;
	return dur;
}

INLINE void
wlc_ht_monitor(wlc_ht_info_t* pub, wlc_d11rxhdr_t *wrxh, uint8 *plcp,
	ratespec_t rspec, struct wl_rxsts *sts)
{
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	wlc_info_t *wlc = hti->wlc;

	struct dot11_header *h;
	uint16 subtype;

	h = (struct dot11_header *)(plcp + D11_PHY_HDR_LEN);
	subtype = (ltoh16(h->fc) & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT;

	if ((subtype == FC_SUBTYPE_QOS_DATA) || (subtype == FC_SUBTYPE_QOS_NULL)) {

		/* A-MPDU parsing */
		if ((wrxh->rxhdr.PhyRxStatus_0 & PRXS0_FT_MASK) == PRXS0_PREN) {
			if (WLC_IS_MIMO_PLCP_AMPDU(plcp)) {
				sts->nfrmtype |= WL_RXS_NFRM_AMPDU_FIRST;
				/* Save the rspec for later */
				wlc->monitor_ampdu_rspec = rspec;
			} else if (!(plcp[0] | plcp[1] | plcp[2])) {
				sts->nfrmtype |= WL_RXS_NFRM_AMPDU_SUB;
				/* Use the saved rspec */
				rspec = wlc->monitor_ampdu_rspec;
			}
		}
#ifdef WL11AC
		else if ((wrxh->rxhdr.PhyRxStatus_0 & PRXS0_FT_MASK) == FT_VHT) {
			if ((plcp[0] | plcp[1] | plcp[2]) &&
				!(wrxh->rxhdr.RxStatus2 & RXS_PHYRXST_VALID)) {
				/* First MPDU:
				 * PLCP header is valid, Phy RxStatus is not valid
				 */
				sts->nfrmtype |= WL_RXS_NFRM_AMPDU_FIRST;
				/* Save the rspec for later */
				wlc->monitor_ampdu_rspec = rspec;
				wlc->monitor_ampdu_counter++;
			} else if (!(plcp[0] | plcp[1] | plcp[2]) &&
				!(wrxh->rxhdr.RxStatus2 & RXS_PHYRXST_VALID)) {
				/* Sub MPDU:
				 * PLCP header is not valid, Phy RxStatus is not valid
				 */
				sts->nfrmtype |= WL_RXS_NFRM_AMPDU_SUB;
				/* Use the saved rspec */
				rspec = wlc->monitor_ampdu_rspec;
			} else if ((plcp[0] | plcp[1] | plcp[2]) &&
				(wrxh->rxhdr.RxStatus2 & RXS_PHYRXST_VALID)) {
				/* MPDU is not a part of A-MPDU:
				 * PLCP header is valid and Phy RxStatus is valid
				 */
				wlc->monitor_ampdu_counter++;
			} else {
				/* Last MPDU */
				/* XXX: done to take care of the last MPDU in A-mpdu
				* VHT packets are considered A-mpdu
				* Use the saved rspec
				*/
				rspec = wlc->monitor_ampdu_rspec;
			}

			sts->ampdu_counter = wlc->monitor_ampdu_counter;
		}
#endif /* WL11AC */

	}
	/* A-MSDU parsing */
	if (wrxh->rxhdr.RxStatus2 & RXS_AMSDU_MASK) {
		/* it's chained buffer, break it if necessary */
		sts->nfrmtype |= WL_RXS_NFRM_AMSDU_FIRST | WL_RXS_NFRM_AMSDU_SUB;
	}
}

/* Convert htflags and mcs values to
* rate in units of 500kbps
*/
static uint16
wlc_ht_phy_get_rate(wlc_ht_info_t *pub, uint8 htflags, uint8 mcs)
{

	ratespec_t rspec = HT_RSPEC(mcs);

	if (htflags & WL_RXS_HTF_40)
		rspec |= RSPEC_BW_40MHZ;

	if (htflags & WL_RXS_HTF_SGI)
		rspec |= RSPEC_SHORT_GI;

	return RSPEC2KBPS(rspec)/500;
}

INLINE void
wlc_ht_prep_rate_info(wlc_ht_info_t *pub, wlc_d11rxhdr_t *wrxh,
	ratespec_t rspec, struct wl_rxsts *sts)
{
	/* prepare HT rate/modulation info */
	sts->mcs = (rspec & RSPEC_RATE_MASK);

	if (CHSPEC_BW_GE(sts->chanspec, WL_CHANSPEC_BW_40)) {
		uint32 bw = RSPEC_BW(rspec);

		if (bw == RSPEC_BW_20MHZ) {
			if (CHSPEC_CTL_SB(sts->chanspec) == WL_CHANSPEC_CTL_SB_L) {
				sts->htflags = WL_RXS_HTF_20L;
			} else {
				sts->htflags = WL_RXS_HTF_20U;
			}
		} else if (bw == RSPEC_BW_40MHZ) {
			sts->htflags = WL_RXS_HTF_40;
		}
	}

	if (RSPEC_ISSGI(rspec))
		sts->htflags |= WL_RXS_HTF_SGI;
	if (RSPEC_ISLDPC(rspec))
		sts->htflags |= WL_RXS_HTF_LDPC;
	if (RSPEC_ISSTBC(rspec))
		sts->htflags |= (1 << WL_RXS_HTF_STBC_SHIFT);

	sts->datarate = wlc_ht_phy_get_rate(pub, sts->htflags, sts->mcs);
}

#ifdef WLTXMONITOR
INLINE void
wlc_ht_txmon_chspec(wlc_ht_info_t *pub, uint16 phytxctl, uint16 phytxctl1,
	uint16 chan_band, uint16 chan_num,
	struct wl_txsts *sts, uint16 *chan_bw)
{
	wlc_info_t *wlc = WLC_HT_INFO_PRIV(pub)->wlc;
	BCM_REFERENCE(wlc);
	*chan_bw = ((phytxctl1 & PHY_TXC1_BW_MASK) == PHY_TXC1_BW_40MHZ)?
		WL_CHANSPEC_BW_40: WL_CHANSPEC_BW_20;
	if ((phytxctl1 & PHY_TXC1_BW_MASK) == PHY_TXC1_BW_20MHZ_UP) {
		sts->chanspec = (chanspec_t)(chan_num | *chan_bw |
			WL_CHANSPEC_CTL_SB_UPPER | chan_band);
	} else if (*chan_bw == WL_CHANSPEC_BW_40) {
		if (chan_num == CHSPEC_CHANNEL(WLC_BAND_PI_RADIO_CHANSPEC)) {
			sts->chanspec = WLC_BAND_PI_RADIO_CHANSPEC;
		} else {
			sts->chanspec = (chanspec_t)(chan_num | *chan_bw |
				WL_CHANSPEC_CTL_SB_UPPER | chan_band);
		}
	} else {
		sts->chanspec = (chanspec_t)(chan_num | *chan_bw |
			chan_band);
	}
}

INLINE void
wlc_ht_txmon_htflags(wlc_ht_info_t *hti,
	uint16 phytxctl, uint16 phytxctl1, uint8 *plcp,
	uint16 chan_bw, uint16 chan_band, uint16 chan_num, ratespec_t *rspec,
	struct wl_txsts *sts)
{
	/* prepare HT rate/modulation info */
	sts->mcs = plcp[0] & MIMO_PLCP_MCS_MASK;
	if (chan_bw == WL_CHANSPEC_BW_40) {
		sts->htflags = WL_RXS_HTF_40;
	} else if ((phytxctl1 & PHY_TXC1_BW_MASK) == PHY_TXC1_BW_20MHZ_UP) {
		sts->htflags = WL_RXS_HTF_20U;
	} else {
		sts->htflags = WL_RXS_HTF_20L;
	}

	if (PLCP3_ISSGI(plcp[3]))
		sts->htflags |= WL_RXS_HTF_SGI;
	sts->htflags |= (((plcp[3] & PLCP3_STC_MASK) >> PLCP3_STC_SHIFT)
					<< WL_RXS_HTF_STBC_SHIFT);

	sts->encoding = WL_RXS_ENCODING_HT;
	sts->preamble = (phytxctl & PHY_TXC_SHORT_HDR) ?
		WL_RXS_PREAMBLE_HT_GF: WL_RXS_PREAMBLE_HT_MM;

	sts->datarate = wlc_ht_phy_get_rate(hti, sts->htflags, sts->mcs);
	sts->phytype = WL_RXS_PHY_N;
}

INLINE void
wlc_ht_txmon_agg_ft(wlc_ht_info_t *hti, void *p, struct dot11_header *h,
	uint8 frametype, struct wl_txsts *sts)
{
	uint8 *plcp;
	uint16 subtype;

	plcp = PKTDATA(WLC_HT_INFO_PRIV(hti)->wlc->osh, p);
	subtype = (ltoh16(h->fc) & FC_SUBTYPE_MASK) >> FC_SUBTYPE_SHIFT;

	if ((subtype == FC_SUBTYPE_QOS_DATA) || (subtype == FC_SUBTYPE_QOS_NULL)) {

		/* A-MSDU parsing */
		if (WLPKTTAG(p)->flags & WLF_AMSDU) {
			/* it's chained buffer, break it if necessary */
			sts->nfrmtype |= WL_RXS_NFRM_AMSDU_FIRST | WL_RXS_NFRM_AMSDU_SUB;
		} else if (frametype == FT_HT) { /* A-MPDU parsing */
			if (WLC_IS_MIMO_PLCP_AMPDU(plcp)) {
				sts->nfrmtype |= WL_RXS_NFRM_AMPDU_FIRST;
			} else if (!(plcp[0] | plcp[1] | plcp[2])) {
				sts->nfrmtype |= WL_RXS_NFRM_AMPDU_SUB;
			}
		}
	}
}
#endif /* WLTXMONITOR */

void
wlc_ht_upd_txbf_cap(wlc_bsscfg_t *cfg, uint8 bfr, uint8 bfe, uint32 *cap)
{
	*cap &=  ~(htol32(TXBF_N_DFT_BFR_CAP) | htol32(TXBF_N_DFT_BFE_CAP));

	if (bfr)
		*cap |= htol32(TXBF_N_DFT_BFR_CAP);

	if (bfe)
		*cap |= htol32(TXBF_N_DFT_BFE_CAP);

}

/** Ie hdlrs */
void
wlc_ht_build_cap_ie(wlc_bsscfg_t *cfg, ht_cap_ie_t *cap_ie, uint8 *supp_mcs, bool is2G)
{
	uint16 cap;
	int i;

	wlc_info_t *wlc = cfg->wlc;
	wlc_ht_info_t *pub = wlc->hti;
	wlc_ht_priv_info_t *hti;
	int rxstreams = wlc->stf->rxstreams;

	hti = WLC_HT_INFO_PRIV(pub);

#ifdef RXCHAIN_PWRSAVE
	if ((wlc->ap != NULL) && RXCHAIN_PWRSAVE_ENAB(wlc->ap)) {
		uint8 rxchain = wlc_rxchain_pwrsave_get_rxchain(wlc);
		rxstreams = (int)WLC_BITSCNT(rxchain);
	}
#endif /* RXCHAIN_PWRSAVE */

	/* update GF cap bit based on the override setting */
	wlc_prot_n_cap_upd(wlc->prot_n, cfg, &hti->cap_ie);

	if (is2G)
		hti->cap_ie.cap |= HT_CAP_DSSS_CCK;
	else
		hti->cap_ie.cap &= ~HT_CAP_DSSS_CCK;

	cap = hti->cap_ie.cap;

	/* clear 40MHz capability advertisement if
	 * - the locale does not allow 40MHz or
	 * - the band is not configured to support it or
	 * - chanspec was forced to 20MHz for AP/IBSS
	 */
	if ((wlc_channel_locale_flags(wlc->cmi) & WLC_NO_40MHZ) ||
		(!WL_BW_CAP_40MHZ(wlc->band->bw_cap)) ||
		((
#ifdef WLTDLS
		(cfg && (!BSSCFG_IS_TDLS(cfg) || !wlc_tdls_cert_test_enabled(wlc))) ||
#endif // endif
		(cfg != NULL && (BSSCFG_AP(cfg) || (!cfg->BSS && !BSSCFG_IS_TDLS(cfg))))) &&
		(CHSPEC_IS20(cfg->current_bss->chanspec) && !WL_BW_CAP_40MHZ(wlc->band->bw_cap)))) {
		cap &= ~HT_CAP_40MHZ;
		cap &= ~HT_CAP_SHORT_GI_40;
		cap &= ~HT_CAP_DSSS_CCK;
	}

	wlc_obss_coex_checkadd_40intol(wlc->obss, cfg, is2G, &cap);

	bzero(cap_ie, sizeof(ht_cap_ie_t));
	cap_ie->cap = htol16(cap);
	cap_ie->params = hti->cap_ie.params;

	/*
	 * create the MCS bitmap
	 */

	/* start with the hw supported MCS bitmask */
	bcopy(supp_mcs, &cap_ie->supp_mcs[0], MCSSET_LEN);

	/* filter out rates that have a stream count greater than our current
	 * rxstreams support.
	 * 1 stream: MCS 0-7 and MCS32
	 * 2 stream: MCS 8-15
	 * 3 stream: MCS 16-23
	 * 4 stream: MCS 24-31
	 */
	/* start at 1 since 0-7 are mandatory */
	for (i = 1; i < 4; i++) {
		if (i >= rxstreams) {
			cap_ie->supp_mcs[i] = 0;
		}
	}

#ifdef WL_BEAMFORMING
	if (TXBF_ENAB(wlc->pub))
		wlc_txbf_ht_upd_bfr_bfe_cap(wlc->txbf, cfg, &cap_ie->txbf_cap);
#endif // endif
	if (WLPROPRIETARY_11N_RATES_ENAB(wlc->pub)) {
		for (i = WLC_11N_FIRST_PROP_MCS; i <= WLC_MAXMCS; i++) {
			if (GET_PROPRIETARY_11N_MCS_NSS(i) > wlc->stf->op_rxstreams) {
				clrbit(cap_ie->supp_mcs, i);
			}
		}
	}
} /* wlc_ht_build_cap_ie */

uint8*
wlc_write_brcm_ht_cap_ie(wlc_info_t *wlc, uint8 *cp, int buflen, ht_cap_ie_t *cap_ie)
{
	ht_prop_cap_ie_t prop_cap_ie;

	prop_cap_ie.id = DOT11_MNG_PROPR_ID;
	prop_cap_ie.len = HT_CAP_IE_LEN + HT_PROP_IE_OVERHEAD;
	bcopy(BRCM_PROP_OUI, &prop_cap_ie.oui[0], DOT11_OUI_LEN);
	prop_cap_ie.type = HT_CAP_IE_TYPE;
	bcopy(cap_ie, &prop_cap_ie.cap_ie, sizeof(ht_cap_ie_t));

	cp = bcm_copy_tlv_safe(&prop_cap_ie, cp, buflen);

	return cp;
}

uint8*
wlc_write_brcm_ht_add_ie(wlc_info_t *wlc, uint8 *cp, int buflen, ht_add_ie_t *add_ie)
{
	ht_prop_add_ie_t prop_add_ie;

	prop_add_ie.id = DOT11_MNG_PROPR_ID;
	prop_add_ie.len = HT_ADD_IE_LEN + HT_PROP_IE_OVERHEAD;
	bcopy(BRCM_PROP_OUI, &prop_add_ie.oui[0], DOT11_OUI_LEN);
	prop_add_ie.type = HT_ADD_IE_TYPE;
	bcopy(add_ie, &prop_add_ie.add_ie, sizeof(ht_add_ie_t));

	cp = bcm_copy_tlv_safe(&prop_add_ie, cp, buflen);

	return cp;
}

ht_cap_ie_t *
wlc_read_ht_cap_ie(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len)
{
	bcm_tlv_t *cap_ie_tlv;

	/* check for the ana-assigned capability ie first */
	cap_ie_tlv = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_HT_CAP);
	if (cap_ie_tlv) {
		if (cap_ie_tlv->len >= HT_CAP_IE_LEN)
			return (ht_cap_ie_t *)&cap_ie_tlv->data;
		else
			WL_ERROR(("wl%d: %s: std len %d don't match\n",
			          wlc->pub->unit, __FUNCTION__, cap_ie_tlv->len));
	}

	return NULL;
}

ht_cap_ie_t *
wlc_read_brcm_ht_cap_ie(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len)
{
	ht_prop_cap_ie_t *prop_cap_ie;
	uint8 type = HT_CAP_IE_TYPE;

	/* ana-assigned not found; look for prop one */
	prop_cap_ie = (ht_prop_cap_ie_t *)bcm_find_vendor_ie(tlvs, tlvs_len,
		BRCM_PROP_OUI, &type, sizeof(type));

	if (prop_cap_ie) {
		if (prop_cap_ie->len >= (HT_CAP_IE_LEN + HT_PROP_IE_OVERHEAD))
			return &prop_cap_ie->cap_ie;
		else
			WL_ERROR(("wl%d: %s: len %d too short\n",
			          wlc->pub->unit, __FUNCTION__, prop_cap_ie->len));
	}

	return NULL;
}

/*
 * Find ht cap ie first. If not found look for brcm prop ht cap ie.
 */
ht_cap_ie_t *
wlc_read_ht_cap_ies(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len)
{
	ht_cap_ie_t *cap_ie;

	cap_ie = wlc_read_ht_cap_ie(wlc, tlvs, tlvs_len);
	if (!cap_ie)
		cap_ie = wlc_read_brcm_ht_cap_ie(wlc, tlvs, tlvs_len);

	return cap_ie;
}

ht_add_ie_t *
wlc_read_ht_add_ie(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len)
{
	bcm_tlv_t *add_ie_tlv;

	/* check for the ana-assigned capability ie first */
	add_ie_tlv = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_HT_ADD);
	if (add_ie_tlv) {
		if (add_ie_tlv->len >= HT_ADD_IE_LEN)
			return (ht_add_ie_t *)&add_ie_tlv->data;
		else
			WL_ERROR(("wl%d: %s: std len %d don't match\n",
			          wlc->pub->unit, __FUNCTION__, add_ie_tlv->len));
	}

	return NULL;
}

ht_add_ie_t *
wlc_read_brcm_ht_add_ie(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len)
{
	ht_prop_add_ie_t *prop_add_ie;
	uint8 type = HT_ADD_IE_TYPE;

	/* ana-assigned not found; look for prop one */
	prop_add_ie = (ht_prop_add_ie_t *)bcm_find_vendor_ie(tlvs, tlvs_len,
		BRCM_PROP_OUI, &type, sizeof(type));

	if (prop_add_ie) {
		if (prop_add_ie->len >= (HT_ADD_IE_LEN + HT_PROP_IE_OVERHEAD))
			return &prop_add_ie->add_ie;
		else
			WL_ERROR(("wl%d: %s: len %d too short\n",
			          wlc->pub->unit, __FUNCTION__, prop_add_ie->len));
	}

	return NULL;
}

/*
 * Find ht additional ie first. If not found look for brcm prop ht additional ie.
 */
ht_add_ie_t *
wlc_read_ht_add_ies(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len)
{
	ht_add_ie_t *add_ie;

	add_ie = wlc_read_ht_add_ie(wlc, tlvs, tlvs_len);
	if (!add_ie)
		add_ie = wlc_read_brcm_ht_add_ie(wlc, tlvs, tlvs_len);

	return add_ie;
}

obss_params_t *
wlc_ht_read_obss_scanparams_ie(wlc_info_t *wlc, uint8 *tlvs, int tlvs_len)
{
	bcm_tlv_t *obss_ie_tlv = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_HT_OBSS_ID);

	if (obss_ie_tlv) {
		if (obss_ie_tlv->len >= DOT11_OBSS_SCAN_IE_LEN)
			return (obss_params_t *)obss_ie_tlv->data;
		else
			WL_ERROR(("wl%d: %s: len %d too short\n",
				wlc->pub->unit, __FUNCTION__, obss_ie_tlv->len));
	}
	return NULL;
}

static void
wlc_ht_set_rifs_mode(wlc_ht_info_t *pub, wlc_bsscfg_t *cfg, int32 mode)
{
	BSS_HT_INFO_PRIV(pub, cfg)->rifs_mode = mode;
}

static int32
wlc_ht_get_rifs_advert(wlc_ht_info_t *pub)
{
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	return hti->rifs_advert;
}

void
wlc_ht_set_add_ie_basic_mcs(wlc_ht_info_t *pub, uint8 *basic_mcs, int len)
{
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);

	ASSERT(len == MCSSET_LEN);
	bcopy(hti->add_ie.basic_mcs, basic_mcs, len);
}

#ifdef STA
static void
wlc_ht_wdog(void *ctx)
{
	wlc_ht_info_t *pub = (wlc_ht_info_t *)ctx;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);
	wlc_info_t *wlc = hti->wlc;
	int i;
	wlc_bsscfg_t *cfg;
	FOREACH_AS_STA(wlc, i, cfg) {
		wlc_ht_mimops_action(wlc->hti, cfg);
	}
}
#endif /* STA */

static void
wlc_ht_update_cfg_rifs_mode(wlc_ht_info_t *pub, wlc_bsscfg_t *cfg)
{
	int8 n_cfg;
	wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);
	wlc_info_t *wlc = hti->wlc;

	wlc_ht_set_rifs_mode(pub, cfg, 0);
	if (!wlc->prot_n || !WLC_PROT_N_CFG(wlc->prot_n, cfg)) {
		return;
	}
	n_cfg = WLC_PROT_N_CFG_N(wlc->prot_n, cfg);
	if (wlc_ht_get_rifs_advert(pub) && (n_cfg != WLC_N_PROTECTION_MIXEDMODE)) {
		wlc_ht_set_rifs_mode(pub, cfg, 1);
	}
}

/* Set cfg to NULL if rifs_advert changes */
void
wlc_ht_update_rifs_mode(wlc_ht_info_t *pub, wlc_bsscfg_t *cfg)
{
	if (cfg) {
		wlc_ht_update_cfg_rifs_mode(pub, cfg);
	} else {
		/* global setting changed, update all cfg */
		wlc_ht_priv_info_t *hti = WLC_HT_INFO_PRIV(pub);
		wlc_info_t *wlc = hti->wlc;
		int i;
		wlc_bsscfg_t *cfg_tbl_item;

		FOREACH_BSS(wlc, i, cfg_tbl_item) {
			wlc_ht_update_cfg_rifs_mode(pub, cfg_tbl_item);
		}
	}
}

static void
wlc_ht_update_cfg_txburst_limit(wlc_ht_info_t *pub, wlc_bsscfg_t *cfg)
{
	wlc_bss_info_t *current_bss = cfg->current_bss;
	wlc_info_t *wlc = WLC_HT_INFO_PRIV(pub)->wlc;
	uint8 non11n_apsd_assoc;
	wlc_ht_set_txburst_limit(pub, cfg, 0);
	if (!current_bss) {
		return;
	}
	non11n_apsd_assoc = wlc_prot_n_get_non11n_apsd_assoc(wlc->prot_n, cfg);
	if (WLC_HT_GET_TXBURST_LIMIT_OVERRIDE(pub) == ON || non11n_apsd_assoc) {
		wlc_ht_set_txburst_limit(pub, cfg,
			((CHSPEC_WLCBANDUNIT(current_bss->chanspec) ==
			BAND_5G_INDEX) ?
			DOT11N_5G_TXBURST_LIMIT :
			DOT11N_2G_TXBURST_LIMIT));
	}
}

void
wlc_ht_update_txburst_limit(wlc_ht_info_t *pub, wlc_bsscfg_t *cfg)
{
	if (cfg) {
		wlc_ht_update_cfg_txburst_limit(pub, cfg);
	} else {
		/* global setting changed, update all cfg */
		wlc_info_t *wlc = WLC_HT_INFO_PRIV(pub)->wlc;
		wlc_bsscfg_t *cfg_tbl_item;
		int i;
		/* do for all cfg in wlc bsscfg table */
		FOREACH_BSS(wlc, i, cfg_tbl_item) {
			wlc_ht_update_cfg_txburst_limit(pub, cfg_tbl_item);
		}
	}
}

static int8
wlc_ht_mimops_get(wlc_ht_info_t *pub)
{
	return (int8)((wlc_ht_get_cap(pub) & HT_CAP_MIMO_PS_MASK) >> HT_CAP_MIMO_PS_SHIFT);
}

static bool
wlc_ht_mimops_set(wlc_ht_info_t *pub, wlc_bsscfg_t *cfg, int32 int_val)
{
	if ((int_val < 0) || (int_val > HT_CAP_MIMO_PS_OFF) || (int_val == 2)) {
		return FALSE;
	}

	wlc_ht_mimops_cap_update(pub, (uint8)int_val);

#ifndef WLC_NET80211
	/* FIXME: PR81503: This should probably iterate over all bsscfgs, sending action
	 * frames for each. Only when all have been informed can the process be declared
	 * complete.
	 */
	if (cfg && cfg->associated)
		wlc_send_action_ht_mimops(pub, cfg, (uint8)int_val);
#endif // endif

	return TRUE;
}

void
wlc_ht_mimops_handle_rxchain_update(wlc_ht_info_t *pub, uint8 mimops_mode)
{
	wlc_info_t *wlc = WLC_HT_INFO_PRIV(pub)->wlc;
#ifndef WLC_NET80211
	wlc_ht_set_cfg_mimops_PM(pub, wlc->cfg, mimops_mode);
#endif /* WLC_NET80211 */
	if (AP_ENAB(wlc->pub)) {
		wlc_phy_stf_chain_set(WLC_PI(wlc), wlc->stf->txchain, wlc->stf->rxchain);
		wlc_ht_mimops_cap_update(pub, mimops_mode);
		if (wlc->pub->associated)
			wlc_mimops_action_ht_send(pub, wlc->cfg, mimops_mode);
		return;
	}
	if (wlc->pub->associated) {
		if (mimops_mode == HT_CAP_MIMO_PS_OFF) {
			/* if mimops is off, turn on the Rx chain first */
			wlc_phy_stf_chain_set(WLC_PI(wlc), wlc->stf->txchain,
				wlc->stf->rxchain);
			wlc_ht_mimops_cap_update(pub, mimops_mode);
		}
		wlc_mimops_action_ht_send(pub, wlc->cfg, mimops_mode);
	} else {
		wlc_phy_stf_chain_set(WLC_PI(wlc), wlc->stf->txchain, wlc->stf->rxchain);
		wlc_ht_mimops_cap_update(wlc->hti, mimops_mode);
	}
}

void
wlc_ht_update_ampdu_rx_cap_params(wlc_ht_info_t *pub,
	uint8 rx_factor, uint8 mpdu_density)
{
	uint8 params;
	uint8 ht_rx_factor;
	wlc_info_t *wlc = WLC_HT_INFO_PRIV(pub)->wlc;

	params = wlc_ht_get_cap_params(wlc->hti);
	/* Adjust the HT rx_factor based on the VHT values */
	if (VHT_ENAB(wlc->pub)) {
		if (rx_factor > AMPDU_RX_FACTOR_64K)
			ht_rx_factor = AMPDU_RX_FACTOR_64K;
		else
			ht_rx_factor = rx_factor;
	} else {
		ht_rx_factor = rx_factor;
	}

	params &= ~(HT_PARAMS_RX_FACTOR_MASK | HT_PARAMS_DENSITY_MASK);
	params |= (ht_rx_factor & HT_PARAMS_RX_FACTOR_MASK);
	params |= (mpdu_density << HT_PARAMS_DENSITY_SHIFT) & HT_PARAMS_DENSITY_MASK;
	wlc_ht_set_cap_params(wlc->hti, params);

	if (AP_ENAB(wlc->pub) && wlc->clk) {
		wlc_update_beacon(wlc);
		wlc_update_probe_resp(wlc, TRUE);
	}

}

void
wlc_ht_fill_sta_fields(wlc_ht_info_t *pub, struct scb *scb, sta_info_t *sta)
{
	wlc_info_t *wlc = WLC_HT_INFO_PRIV(pub)->wlc;

	if (SCB_HT_CAP(scb)) {
		sta->flags |= WL_STA_N_CAP;
		sta->ht_capabilities = wlc_ht_get_scb_cap(wlc->hti, scb);
	}
}

bool
wlc_ht_is_40MHZ_cap(wlc_ht_info_t *pub)
{
	return ((wlc_ht_get_cap(pub) & HT_CAP_40MHZ) != 0);
}

void
wlc_ht_set_rx_stbc_cap(wlc_ht_info_t *hti, int val)
{
	uint16 cap;
	cap = wlc_ht_get_cap(hti);
	cap &= ~HT_CAP_RX_STBC_MASK;
	cap |= (val << HT_CAP_RX_STBC_SHIFT);
	wlc_ht_set_cap(hti, cap);
}

int8
wlc_ht_stbc_rx_get(wlc_ht_info_t *hti)
{
	return (wlc_ht_get_cap(hti) & HT_CAP_RX_STBC_MASK) >>
		HT_CAP_RX_STBC_SHIFT;
}

bool
wlc_ht_stbc_tx_set(wlc_ht_info_t *hti, int32 int_val)
{
	wlc_info_t* wlc = WLC_HT_INFO_PRIV(hti)->wlc;

	uint16 cap;
	if ((int_val != AUTO) && (int_val != OFF) && (int_val != ON)) {
		return FALSE;
	}

	cap = wlc_ht_get_cap(hti);

	if ((int_val == OFF) || (wlc->stf->op_txstreams == 1) || !WLC_STBC_CAP_PHY(wlc)) {
		cap &= ~HT_CAP_TX_STBC;
		wlc_ht_set_cap(hti, cap);
#ifdef WL11AC
		wlc_vht_set_tx_stbc_cap(wlc->vhti, FALSE);
#endif /* WL11AC */
	}
	else {
		cap |= HT_CAP_TX_STBC;
		wlc_ht_set_cap(hti, cap);
#ifdef WL11AC
		wlc_vht_set_tx_stbc_cap(wlc->vhti, TRUE);
#endif /* WL11AC */
	}

	wlc->bandstate[BAND_2G_INDEX]->band_stf_stbc_tx = (int8)int_val;
	wlc->bandstate[BAND_5G_INDEX]->band_stf_stbc_tx = (int8)int_val;

	return TRUE;
}

void
wlc_ht_checkadd_rifs_permitted(wlc_ht_info_t *hti, int8 n_cfg, uint8* byte1)
{
	wlc_info_t* wlc = WLC_HT_INFO_PRIV(hti)->wlc;

	/* A VHT AP shall set the RIFS Mode field in the HT Operation element to 0. */
	if (VHT_ENAB(wlc->pub) && BAND_5G(wlc->band->bandtype))
		return;

	if (wlc_ht_get_rifs_advert(hti) &&
		(n_cfg != WLC_N_PROTECTION_MIXEDMODE)) {
		*byte1 |= HT_RIFS_PERMITTED;
	}
}

void
wlc_ht_cap_enable_tx_stbc(wlc_ht_info_t *pub)
{
	uint16 cap = wlc_ht_get_cap(pub);
	cap |= HT_CAP_TX_STBC;
	wlc_ht_set_cap(pub, cap);
}

#if WL_HT_TXBW_OVERRIDE_ENAB
static void
wlc_ht_update_txbw_override(wlc_ht_info_t *pub)
{
	if (pub->mimo_40txbw != AUTO ||
		pub->ofdm_40txbw != AUTO ||
		pub->cck_40txbw != AUTO) {
		pub->txbw_override = TRUE;
	} else {
		pub->txbw_override = FALSE;
	}
}
#endif /* WL_HT_TXBW_OVERRIDE_ENAB */
#endif /* WL11N */

/* Given the band and the HT additional IE construct the chanspec */
#ifdef WL11ULB
chanspec_t
wlc_ht_chanspec(wlc_info_t *wlc, uint8 chan, uint8 extch, wlc_bsscfg_t *cfg)
#else /* WL11ULB */
chanspec_t
wlc_ht_chanspec(wlc_info_t *wlc, uint8 chan, uint8 extch)
#endif /* WL11ULB */
{
	uint16 radio_channel = chan;
	uint16 ctl_sb = 0;
	uint16 bw = WL_CHANSPEC_BW_20;
	uint16 band;
	chanspec_t chanspec = 0;

#ifdef WL11ULB
	if ((cfg != NULL) && BSSCFG_ULB_ENAB(wlc, cfg)) {
		bw = wlc_ulb_get_bss_min_bw(wlc->ulb_info, cfg);
		extch = DOT11_EXT_CH_NONE;
	}
#endif /* WL11ULB */

	if (CHANNEL_BANDUNIT(wlc, chan) == BAND_5G_INDEX)
		band = WL_CHANSPEC_BAND_5G;
	else
		band = WL_CHANSPEC_BAND_2G;
	switch (extch & DOT11_EXT_CH_MASK) {
		case DOT11_EXT_CH_UPPER:
			radio_channel = UPPER_20_SB(chan);
			ctl_sb = WL_CHANSPEC_CTL_SB_LOWER;
			bw = WL_CHANSPEC_BW_40;
			break;
		case DOT11_EXT_CH_LOWER:
			radio_channel = LOWER_20_SB(chan);
			ctl_sb = WL_CHANSPEC_CTL_SB_UPPER;
			bw = WL_CHANSPEC_BW_40;
			break;
		case DOT11_EXT_CH_NONE:
			break;
		default:
			WL_ERROR(("wl%d: Unexpected channel ext in additional HT IE,"
				" default to 20MHz\n", wlc->pub->unit));
			break;
	}

	chanspec = radio_channel | ctl_sb | bw | band;
	return chanspec;
}
