/*
 * wlc_fbt.c -- FBT module source.
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
 * $Id: wlc_fbt.c 788162 2020-06-23 10:25:26Z $
 */

/**
 * @file
 * @brief
 * Fast BSS Transition (802.11r) - roaming / fast authentication related
 */

/**
 * @file
 * @brief
 * XXX Twiki: [FBT]
 */

#ifdef BCMINTSUP
#include <wlc_cfg.h>
#endif /* BCMINTSUP */

#if !defined(WLFBT)
#error "WLFBT must be defined"
#endif /* !defined(WLFBT) */

#ifdef BCMINTSUP
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <proto/eap.h>
#include <proto/eapol.h>
#include <bcmwpa.h>
#include <bcmcrypto/prf.h>

#include <proto/802.11.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_keymgmt.h>
#include <wlc_led.h>
#include <wlc_rm.h>
#include <wlc_assoc.h>
#include <wl_export.h>
#include <wlc_scb.h>
#include <wlc_scb_ratesel.h>
#include <wlc_obss.h>
#include <wlc_ie_mgmt.h>
#include <wlc_ie_mgmt_ft.h>
#include <wlc_ie_mgmt_vs.h>
#include <wlc_ie_reg.h>
#if defined(BCMSUP_PSK) || defined(WLFBT)
#include <wlc_wpa.h>
#endif /* BCMSUP_PSK */
#include <wlc_sup.h>
#ifdef WOWL
#include <wlc_wowl.h>
#endif // endif
#ifdef WLBTAMP
#include <proto/802.11_bta.h>
#endif /* WLBTAMP */

#else /* external supplicant */

#include <stdio.h>
#include <typedefs.h>
#include <wlioctl.h>
#include <proto/eapol.h>
#include <proto/eap.h>
#include <bcmwpa.h>
#include <sup_dbg.h>
#include <bcmutils.h>
#include <string.h>
#include <bcmendian.h>
#include <bcmcrypto/prf.h>
#include <proto/eapol.h>
#include <bcm_osl.h>
#include "bcm_supenv.h"
#include "wpaif.h"
#include "wlc_sup.h"
#include "wlc_wpa.h"
#endif /* BCMINTSUP */

#ifdef AP
#include <wlc_ap.h>
#include <wlc_scan.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#endif /* AP */
#ifdef WLMCHAN
#include <wlc_mchan.h>
#endif // endif
#include <proto/802.11r.h>
#include "wlc_fbt.h"
#include "wlc_cac.h"
#include <wlc_tx.h>

#ifdef AP
/* MIC count for fixed elements in FBT IE */
#define FBTIE_MIC_IE_COUNT		3

/* OFFSET to optional FBT IE */
#define DOT11_SUBELEM_ID_OFF    0

typedef struct wlc_fbt_ies
{
	dot11_mdid_ie_t mdie;
	uint8 *ftie;
	uchar *r1kh_id;
	uchar *r0kh_id;
	uint8 *gtk;
	uint8 *igtk;
	uint8 *rsnie;
	uint8 *pmkid;
	uint8 *ricie;
	uint ric_len;
	uint ric_elem_count;
	uint8 ftie_len;
	uint8 r1kh_id_len;
	uint8 r0kh_id_len;
	uint8 gtk_len;
	uint8 igtk_len;
	uint8 rsnie_len;
	uint8 pmkid_len;
	uint8 pad0;
} wlc_fbt_ies_t;
#endif /* AP */

#if !defined(DONGLEBUILD) && (defined(BCMDBG) || defined(WLTEST))
#define WL_FT_DBG_IOVARS_ENAB 1
#else
#define WL_FT_DBG_IOVARS_ENAB 0
#endif /* !DONGLEBUILD && (BCMDBG || WLTEST) */

typedef struct wlc_fbt_priv {
	/* references to driver `common' things */
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_pub_t *pub;			/* pointer to wlc public portion */
	void *wl;			/* per-port handle */
	osl_t *osh;			/* PKT* stuff wants this */

	uint16 bss_fbt_priv_offset; /* offset of priv cubby in bsscfg */
#if WL_FT_DBG_IOVARS_ENAB
	uint32 msg_level;		/* control debug output */
#endif /* WL_FT_DBG_IOVARS_ENAB */
} wlc_fbt_priv_t;

#ifdef WLFBT_STA_OVERRIDES
	#if defined(WL_ENAB_RUNTIME_CHECK) || !defined(DONGLEBUILD)
		#define WLFBT_STA_OVERRIDES_ENAB(fbt) ((fbt)->_sta_overrides)
	#elif defined(WLFBT_STA_OVERRIDES_DISABLED)
		#define WLFBT_STA_OVERRIDES_ENAB(fbt) ((void)(fbt), FALSE)
	#else
		#define WLFBT_STA_OVERRIDES_ENAB(fbt) ((void)(fbt), TRUE)
	#endif

	#define WLC_FT_PARAM_OVERRIDE_ACTIVE(fbt, bss_priv) \
		(WLFBT_ENAB((fbt)->mod_priv.wlc->pub) && \
		WLFBT_STA_OVERRIDES_ENAB(fbt) && (bss_priv) != NULL && \
		(bss_priv)->fbt_bss_overrides != NULL)
#else
	#define WLFBT_STA_OVERRIDES_ENAB(fbt) ((void)(fbt), FALSE)
	#define WLC_FT_PARAM_OVERRIDE_ACTIVE(fbt, priv) (FALSE)
#endif /* WLFBT_STA_OVERRIDES */

typedef struct bss_fbt_overrides {
	uint8 *override_ptrs[WL_FBT_PARAM_TYPE_FIRST_INVALID];
	uint8 override_lens[WL_FBT_PARAM_TYPE_FIRST_INVALID];
} wlc_bss_fbt_overrides_t;

struct wlc_fbt_info {
	wlc_fbt_pub_t	mod_pub;
	wlc_fbt_priv_t	mod_priv;
#ifdef AP
	int				scbh;		/* scb cubby handle */
	struct wl_timer *fbt_timer;	/* timer */
#endif /* AP */
	bool _sta_overrides;
};

typedef struct bss_fbt_priv {
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_pub_t *pub;			/* pointer to wlc public portion */
	void *wl;			/* per-port handle */
	osl_t *osh;			/* PKT* stuff wants this */
	wlc_bsscfg_t *cfg;		/* pointer to sup's bsscfg */
	wlc_fbt_info_t *m_handle;	/* module handle */

	wpapsk_t *wpa;			/* volatile, initialized in set_sup */
	wpapsk_info_t *wpa_info;		/* persistent wpa related info */

	struct ether_addr peer_ea;      /* peer's ea */

	uint16 mdid;		/* Mobility domain id */
	uint16 mdie_len;	/* FTIE len */
	uchar *mdie;		/* MDIE */
	uint16 ftie_len;	/* FTIE len */
	uchar *ftie;		/* FTIE */
	bcm_tlv_t *r0khid;
	bcm_tlv_t *r1khid;
	uchar pmkr0name[WPA2_PMKID_LEN];
	uchar pmkr1name[WPA2_PMKID_LEN];
	uint8 current_akm_type;		/* AKM suite used for current association */
	bool use_sup_wpa;		/* TRUE if idsup is enabled and owns wpa/wpa_info */
	wlc_bss_fbt_overrides_t *fbt_bss_overrides; /* when WLFBT_STA_OVERRIDES_ENAB */
} bss_fbt_priv_t;

typedef struct bss_fbt_info {
	bss_fbt_pub_t	bss_pub;
	bss_fbt_priv_t	bss_priv;
#ifdef AP
	wlc_fbt_info_t	*fbt_info;
	uint16			mdid; /* Mobility Domain Identifier */
	char			fbt_r0kh_id[FBT_R0KH_ID_LEN]; /* Config R0 Key holder Identifier */
	uint			fbt_r0kh_id_len; /* R0 Key holder ID Length */
	uint8			fbt_r1kh_id[ETHER_ADDR_LEN];	/* Configured R1 Key holder ID */
	uint8			fbt_over_ds;		/* FBT over DS support */
	uint8			fbt_res_req_cap;	/* resource req capability support */
	uint			reassoc_time;		/* reassoc deadline timer value in TU */
	bool                    fbt_timer_set;          /* Fbt timer is set or not */
#endif /* AP */
} bss_fbt_info_t;

#ifdef AP
/* SCB Config Cubby structure */
typedef struct wlc_fbt_scb {
	struct ether_addr macaddr; /* station mac address */
	uint8	pmk_r1_name[WPA2_PMKID_LEN];
	uint8	status;		/* DOT11_SC_SUCCESS or other status code */
	wpa_ptk_t ptk;		/* pairwise key */
	wpa_gtk_t gtk;		/* group key */
	uint	auth_resp_ielen; /* length of ies below */
	uint8	*auth_resp_ies; /* IEs contains MDIE, RSNIE, FBTIE
							(ANonce, SNonce, R0KH-ID, R1KH-ID)
							*/
	uint	auth_time; /* the system time in milliseconds when
						FBT authentication request is received by AP
						*/
	wlc_fbt_ies_t fbties_assoc;
} wlc_fbt_scb_t;

typedef struct wlc_fbt_scb_cubby {
	wlc_fbt_scb_t *cubby;
} wlc_fbt_scb_cubby_t;

#define FBT_SCB_CUBBY(fbt, scb) ((wlc_fbt_scb_cubby_t *)SCB_CUBBY((scb), (fbt)->scbh))
#define FBT_SCB(fbt, scb) ((FBT_SCB_CUBBY(fbt, scb))->cubby)
#endif /* AP */
/* wlc_fbt_info_priv_t offset in module states */
static uint16 wlc_fbt_info_priv_offset = OFFSETOF(wlc_fbt_info_t, mod_priv);

#define WLC_FBT_PRIV_INFO(fbt_info)		((wlc_fbt_priv_t *)((uint8 *)fbt_info + \
	wlc_fbt_info_priv_offset))

#define FBT_BSSCFG_CUBBY_LOC(fbt, cfg) ((bss_fbt_info_t **)BSSCFG_CUBBY(cfg, (fbt)->mod_pub.cfgh))
#define FBT_BSSCFG_CUBBY(fbt, cfg) (*FBT_BSSCFG_CUBBY_LOC(fbt, cfg))

#define BSS_PRIV_OFFSET(fbt_info)	((WLC_FBT_PRIV_INFO(fbt_info))->bss_fbt_priv_offset)
#define FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg) ((bss_fbt_priv_t *)((uint8 *) \
	(FBT_BSSCFG_CUBBY(fbt_info, cfg))+ 	BSS_PRIV_OFFSET(fbt_info)))
#define FBT_BSSCFG_CUBBY_PUB(fbt_info, cfg) ((bss_fbt_pub_t *)FBT_BSSCFG_CUBBY(fbt_info, cfg))

#define UNIT(ptr)	((ptr)->pub->unit)
#define CUR_EA(ptr)	(((bss_fbt_priv_t *)ptr)->cfg->cur_etheraddr)
#define PEER_EA(ptr)	(((bss_fbt_priv_t *)ptr)->peer_ea)
#define BSS_EA(ptr)	(((bss_fbt_priv_t *)ptr)->cfg->BSSID)
#define BSS_SSID(ptr)	(((bss_fbt_priv_t *)ptr)->cfg->current_bss->SSID)
#define BSS_SSID_LEN(ptr)	(((bss_fbt_priv_t *)ptr)->cfg->current_bss->SSID_len)

/* Transaction Sequence Numbers for FT MIC calculation. */
#define FT_MIC_ASSOC_REQUEST_TSN	5	/* TSN for association request frames. */
#define FT_MIC_REASSOC_RESPONSE_TSN	6	/* TSN for reassociation request response frames. */

#if WL_FT_DBG_IOVARS_ENAB

#define FT_MSG_LVL_IMPORTANT	0x1
#define FT_MSG_LVL_CHATTY	0x2
#define FT_MSG_LVL_ENTRY	0x4

#define WL_FT_MSG(ft, args) \
	do { \
		if ((ft) && ((ft)->msg_level & FT_MSG_LVL_IMPORTANT) != 0) { \
			WL_PRINT(args); \
		} \
	} while (0)

#define WL_FT_CHATTY(ft, args) \
	do { \
		if ((ft) && ((ft)->msg_level & FT_MSG_LVL_CHATTY) != 0) { \
			WL_PRINT(args); \
		} \
	} while (0)

#define WL_FT_ENTRY(ft, args) \
	do { \
		if ((ft) && ((ft)->msg_level & FT_MSG_LVL_ENTRY) != 0) { \
			WL_PRINT(args); \
		} \
	} while (0)

#else
#define WL_FT_MSG(ft, args)
#define WL_FT_CHATTY(ft, args)
#define WL_FT_ENTRY(ft, args)
#endif /* WL_FT_DBG_IOVARS_ENAB */

#ifdef BCMDBG
#if defined(STA) && defined(FBT_STA)
static void wlc_fbt_dump_fbt_keys(bss_fbt_priv_t *fbt_bss_priv, uchar *pmkr0, uchar *pmkr1);
#endif /* STA && FBT_STA */
#ifdef AP
static void wlc_fbt_scb_dump(void *context, struct scb *scb, struct bcmstrbuf *b);
static void
wlc_fbt_bsscfg_dump(void *context, wlc_bsscfg_t *bsscfg, struct bcmstrbuf *b);
#endif /* AP */
#else
#define wlc_fbt_scb_dump NULL
#define wlc_fbt_bsscfg_dump NULL
#endif /* BCMDBG */

static int wlc_fbt_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif);
static void wlc_fbt_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_fbt_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg);
#if defined(STA) && defined(FBT_STA)
static void wlc_fbt_handle_joinproc(void *ctx, bss_assoc_state_data_t *evt_data);
#endif /* STA && FBT_STA */
static void wlc_fbt_bsscfg_updown_callbk(void *ctx, bsscfg_up_down_event_data_t *evt);
static void wlc_fbt_free_ies(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);
#if defined(STA) && defined(FBT_STA)
static bool wlc_fbt_update_ftie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);
static void wlc_fbt_upd_authie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);
#endif /* STA && FBT_STA */
#ifdef AP
static bool wlc_fbt_fbtoverds_flag(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg);
#ifndef WLFBT_DISABLED
static bool wlc_fbt_fbtoverds_enable(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bool fbt_over_ds);
#endif /* WLFBT_DISABLED */
static void wlc_fbtap_reassociation_timer(void *arg);
static int wlc_fbtap_process_auth_resp(wlc_info_t *wlc, wlc_fbt_info_t *fbt_info,
	struct scb *scb, wlc_fbt_auth_resp_t *fbt_auth_resp);
static int wlc_fbtap_process_ds_auth_resp(wlc_info_t *wlc, wlc_fbt_info_t *fbt_info,
	wlc_bsscfg_t *bsscfg, wlc_fbt_auth_resp_t *fbt_auth_resp);
static int wlc_fbt_scb_init(void *context, struct scb *scb);
static void wlc_fbt_scb_deinit(void *context, struct scb *scb);
static void wlc_fbtap_parse_fbties(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	uchar *tlvs, uint tlvs_len, wlc_fbt_ies_t *fbt_ies);
static int wlc_fbtap_validate_auth_fbties(wlc_info_t *wlc, bss_fbt_info_t *fbt_bsscfg,
	wlc_fbt_ies_t fbties, char *sa);
static void wlc_fbtap_add_timer(wlc_info_t *wlc, struct scb *scb, uint timeval);
static void wlc_fbtap_free_fbties(wlc_info_t *wlc, wlc_fbt_ies_t *fbties);
static int wlc_fbtap_process_action_resp(wlc_info_t *wlc, wlc_fbt_info_t *fbt_info,
	wlc_bsscfg_t *bsscfg, wlc_fbt_action_resp_t *fbt_action_resp);
static uint8 *wlc_fbtap_write_ric_ie(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	wlc_fbt_ies_t *fbties_assoc, struct scb *scb, uint8 *cp,
	uint *ricdata_len, uint *ric_ie_count);
static uint8 *wlc_fbt_write_rsn_ie_safe(wlc_info_t *wlc, wlc_bsscfg_t *cfg,
	uint8 *buf, int buflen);
static uint wlc_fbtap_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbtap_write_rsn_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_fbtap_calc_md_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbtap_write_md_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_fbtap_auth_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbtap_auth_write_rsn_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_fbtap_auth_calc_md_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbtap_auth_write_md_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_fbtap_auth_calc_ft_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbtap_auth_write_ft_ie(void *ctx, wlc_iem_build_data_t *data);
static uint wlc_fbtap_calc_ft_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbtap_write_ft_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_fbtap_parse_rsn_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_fbtap_parse_md_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_fbtap_parse_ft_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_fbtap_parse_rde_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_fbtap_auth_parse_rsn_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_fbtap_auth_parse_ft_ie(void *ctx, wlc_iem_parse_data_t *data);
static void wlc_fbtap_scb_state_upd(wlc_fbt_info_t *fbt_info, scb_state_upd_data_t *data);
#endif /* AP */

#ifdef BCMSUP_PSK
static void wlc_fbt_sup_updown_callbk(void *ctx, sup_init_event_data_t *evt);
static void wlc_fbt_sup_init(void *ctx, sup_init_event_data_t *evt);
static void wlc_fbt_sup_deinit(void *ctx, wlc_bsscfg_t *cfg);
#endif /* BCMSUP_PSK */

#if defined(STA) && defined(FBT_STA)
/* IE mgmt */
static uint wlc_fbt_auth_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbt_auth_write_rsn_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_fbt_parse_rsn_ie(void *ctx, wlc_iem_parse_data_t *data);
static uint wlc_fbt_calc_md_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbt_write_md_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_fbt_parse_md_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_fbt_scan_parse_md_ie(void *ctx, wlc_iem_parse_data_t *data);
static int wlc_fbt_scan_parse_ccx_ext_cap_ie(void *ctx, wlc_iem_parse_data_t *data);
static uint wlc_fbt_calc_ft_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbt_write_ft_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_fbt_parse_ft_ie(void *ctx, wlc_iem_parse_data_t *data);
static uint wlc_fbt_calc_rde_ie_len(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_fbt_write_rde_ie(void *ctx, wlc_iem_build_data_t *data);
static int wlc_fbt_parse_rde_ie(void *ctx, wlc_iem_parse_data_t *data);
#endif /* STA && FBT_STA */

#ifdef WLFBT_STA_OVERRIDES
static bool
wlc_fbt_write_overridable_ie(void *ctx, uint16 param_type,
	wlc_iem_build_data_t *data);

static bool
wlc_fbt_calc_overridable_ie_len(void *ctx, wlc_iem_calc_data_t *data,
	uint16 param_type, uint8 *length);

static void wlc_fbt_bsscfg_free_overrides(bss_fbt_priv_t *fbt_bss_priv);
static bss_fbt_priv_t *wlc_fbt_get_bsscfg(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg);

static int wlc_fbt_cubby_field_alloc_set(wlc_fbt_priv_t *fbt_priv,
	void *value_in, uint16 value_in_len,
	uint8 **field_ptr_out, uint8 *field_len_out);

static int wlc_fbt_param_set(wlc_fbt_info_t *fbt_info, wl_fbt_params_t *fbt_params,
	wlc_bsscfg_t *bsscfg);

static int wlc_fbt_param_get(wlc_fbt_info_t *fbt_info, wl_fbt_params_t *fbt_params,
	wlc_bsscfg_t *bsscfg);

static uint8*
wlc_ft_override_get(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg,
	uint16 param_type, uint8 **to_ptr, uint8 *to_len);

static bool
wlc_ft_override_get_len(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg,
uint16 param_type, uint8 *to_len);

static bool
wlc_ft_override_get_buf(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg,
uint16 param_type, uint8 *to_ptr, uint8 max_len);

static void
wlc_ft_param_type_to_pointers(wlc_bss_fbt_overrides_t *fbt_bss_overrides,
	uint16 param_type, uint8 ***to_ptr, uint8 **to_len);

static bool
wlc_fbt_exclude_mde_override(void *ctx, wlc_bsscfg_t *cfg,
	uint16 frame_type);

static bss_fbt_priv_t*
wlc_fbt_param_get_bss_priv(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *bsscfg);
#else
#define wlc_fbt_write_overridable_ie(a, b, c) (FALSE)
#define wlc_fbt_calc_overridable_ie_len(a, b, c, d) (FALSE)
#define wlc_ft_override_get_len(a, b, c, d) (FALSE)
#define wlc_ft_override_get_buf(a, b, c, d, e) (FALSE)
#define wlc_fbt_exclude_mde_override(a, b, c) (TRUE)
#define wlc_fbt_bsscfg_free_overrides(a)
#define wlc_fbt_param_get_bss_priv(a, b) (NULL)
#endif /* WLFBT_STA_OVERRIDES */

enum {
	IOV_ALLOW_FBTOVERDS = 0,
	IOV_FBT_CAP = 1,
	IOV_FBT = 2,				/* Enable Disable FBT per BSS */
	IOV_FBT_AP = 3,				/* Enable Disable FBT per AP */
	IOV_FBT_MDID = 4,			/* Mobility domain Identifier per BSS */
	IOV_FBT_R0KH_ID = 5,		/* R0 Key holder Identifier (nas identifer for this BSS) */
	IOV_FBT_R1KH_ID = 6,		/* R1 Key holder Identifier. (MAC address of this BSS) */
	IOV_FBT_REASSOC_TIME = 7,	/* Reassociation deadline interval per BSS */
	IOV_FBT_AUTH_RESP = 8,		/* Send FBT Authentication Response */
	IOV_FBT_DS_ADD_STA = 9,		/* Application handles FBT over DS and sets this ioctl */
	IOV_FBT_ACT_RESP = 10,		/* Send FBT Action Response Frame */
#ifdef WLFBT_STA_OVERRIDES
	IOV_FBT_PARAM = 11,			/* Override various FBT parameters for STA */
#endif /* WLFBT_STA_OVERRIDES */
#if WL_FT_DBG_IOVARS_ENAB
	IOV_FBT_MSGLEVEL = 12,		/* Add or remove extra debug output for FBT */
#endif /* WL_FT_DBG_IOVARS_ENAB */
	IOV_LAST
};

static const bcm_iovar_t fbt_iovars[] = {
	{"fbtoverds", IOV_ALLOW_FBTOVERDS, 0, IOVT_UINT32, 0},
	{"fbt_cap", IOV_FBT_CAP, (IOVF_OPEN_ALLOW), IOVT_UINT32, 0},
	{"fbt", IOV_FBT, 0, IOVT_INT32, 0},
	{"fbt_ap", IOV_FBT_AP, IOVF_BSSCFG_AP_ONLY, IOVT_INT32, 0},
	{"fbt_mdid", IOV_FBT_MDID, IOVF_BSSCFG_AP_ONLY, IOVT_UINT16, 0},
	{"fbt_r0kh_id", IOV_FBT_R0KH_ID, IOVF_BSSCFG_AP_ONLY, IOVT_BUFFER, 0},
	{"fbt_r1kh_id", IOV_FBT_R1KH_ID, IOVF_BSSCFG_AP_ONLY, IOVT_BUFFER, 0},
	{"fbt_reassoc_time", IOV_FBT_REASSOC_TIME, IOVF_BSSCFG_AP_ONLY, IOVT_UINT32, 0},
	{"fbt_auth_resp", IOV_FBT_AUTH_RESP, IOVF_SET_UP|IOVF_BSSCFG_AP_ONLY, IOVT_BUFFER, 0},
	{"fbt_ds_add_sta", IOV_FBT_DS_ADD_STA, IOVF_SET_UP|IOVF_BSSCFG_AP_ONLY, IOVT_BUFFER, 0},
	{"fbt_act_resp", IOV_FBT_ACT_RESP, IOVF_SET_UP|IOVF_BSSCFG_AP_ONLY, IOVT_BUFFER, 0},
#ifdef WLFBT_STA_OVERRIDES
	{"fbt_param", IOV_FBT_PARAM, IOVF_BSSCFG_STA_ONLY, IOVT_BUFFER,
	sizeof(wl_fbt_params_t) - 1},
#endif /* WLFBT_STA_OVERRIDES */
#if WL_FT_DBG_IOVARS_ENAB
	{"fbt_msglevel", IOV_FBT_MSGLEVEL, (0), IOVT_UINT32, 0},
#endif /* WL_FT_DBG_IOVARS_ENAB */
	{NULL, 0, 0, 0, 0}
};

/* FBT Over-the-DS Action frame IEs' order */
static const uint8 BCMINITDATA(fbt_ie_tags)[] = {
	DOT11_MNG_RSN_ID,
	DOT11_MNG_MDIE_ID,
	DOT11_MNG_FTIE_ID,
};

/* FBT RIC IEs' order in reassoc request frame */
static const uint8 BCMINITDATA(ric_ie_tags)[] = {
	DOT11_MNG_RDE_ID,
	DOT11_MNG_VS_ID
};

#define WLC_FBT_RDE_STA_STATUS_DEFAULT 0

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

#ifdef WLRSDB
typedef struct {
	void *cfg;
} wlc_fbt_info_copy_t;

#define FBT_INFO_COPY_SIZE	sizeof(wlc_fbt_info_copy_t)
static int
wlc_fbt_info_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len);
static int
wlc_fbt_info_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len);
#else
#define FBT_INFO_COPY_SIZE	0
#define wlc_fbt_info_get NULL
#define wlc_fbt_info_set NULL
#endif /* WLRSDB */

/* Allocate fbt context, squirrel away the passed values,
 * and return the context handle.
 */
wlc_fbt_info_t *
BCMATTACHFN(wlc_fbt_attach)(wlc_info_t *wlc)
{
	wlc_fbt_info_t *fbt_info;
	wlc_fbt_priv_t	*fbt_priv;

#ifdef FBT_STA
	uint16 mdiefstbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ) | FT2BMP(FC_AUTH);
	uint16 ftiefstbmp = FT2BMP(FC_REASSOC_REQ) | FT2BMP(FC_AUTH);
	uint16 scanfstbmp = FT2BMP(WLC_IEM_FC_SCAN_BCN) | FT2BMP(WLC_IEM_FC_SCAN_PRBRSP);
	uint16 ftfstbmp = FT2BMP(FC_ASSOC_RESP) | FT2BMP(FC_REASSOC_RESP) | FT2BMP(FC_AUTH);
	uint16 rsniefstbmp = FT2BMP(FC_AUTH);
#endif /* FBT_STA */
#ifdef AP
	uint16 apmdiefstbmp = FT2BMP(FC_BEACON) | FT2BMP(FC_PROBE_RESP) |
		FT2BMP(FC_ASSOC_RESP) | FT2BMP(FC_REASSOC_RESP);
	uint16 apftiefstbmp = FT2BMP(FC_ASSOC_RESP) | FT2BMP(FC_REASSOC_RESP);
	uint16 apassocreqbmp = FT2BMP(FC_ASSOC_REQ) | FT2BMP(FC_REASSOC_REQ);
	uint16 apauthftiefstbmp = FT2BMP(FC_AUTH);
#endif /* AP */

	WL_TRACE(("wl%d: wlc_fbt_attach\n", wlc->pub->unit));

	if (!(fbt_info = (wlc_fbt_info_t *)MALLOCZ(wlc->osh, sizeof(wlc_fbt_info_t)))) {
		WL_ATTACH_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	bzero((char *)fbt_info, sizeof(wlc_fbt_info_t));

	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	fbt_priv->wlc = wlc;
	fbt_priv->pub = wlc->pub;
	fbt_priv->wl = wlc->wl;
	fbt_priv->osh = wlc->osh;
	fbt_priv->bss_fbt_priv_offset = OFFSETOF(bss_fbt_info_t, bss_priv);

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((fbt_info->mod_pub.cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_fbt_info_t *),
		NULL /* wlc_fbt_init */, wlc_fbt_bsscfg_deinit, wlc_fbt_bsscfg_dump,
		(void *)fbt_info)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          UNIT(fbt_priv), __FUNCTION__));
		goto err;
	}

#if defined(STA) && defined(FBT_STA)
	/* assoc join-start/done callback */
	if (wlc_bss_assoc_state_register(wlc, (bss_assoc_state_fn_t)wlc_fbt_handle_joinproc,
		fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bss_assoc_state_register() failed\n",
		          UNIT(fbt_priv), __FUNCTION__));
		goto err;
	}
#endif /* STA && FBT_STA */

	if (wlc_bsscfg_updown_register(wlc, wlc_fbt_bsscfg_updown_callbk, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_up_down_register() failed\n",
			UNIT(fbt_priv), __FUNCTION__));
		goto err;
	}

#ifdef BCMSUP_PSK
	if (SUP_ENAB(wlc->pub)) {
		if (wlc_sup_up_down_register(wlc, wlc_fbt_sup_updown_callbk, fbt_info) != BCME_OK) {
			WL_ERROR(("wl%d: %s: wlc_sup_up_down_register() failed\n",
			 UNIT(fbt_priv), __FUNCTION__));
			goto err;
		}
	}
#endif /* BCMSUP_PSK */

	/* register module */
	if (wlc_module_register(wlc->pub, fbt_iovars, "fbt", fbt_info, wlc_fbt_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: auth wlc_module_register() failed\n", UNIT(fbt_priv)));
		goto err;
	}
#ifdef AP
	fbt_info->fbt_timer = wl_init_timer(wlc->wl, wlc_fbtap_reassociation_timer,
	                                    fbt_info, "fbt");

	if (fbt_info->fbt_timer == NULL) {
		WL_ERROR(("fbt_timer init failed\n"));
		goto err;
	}

	if (wlc_scb_state_upd_register(wlc,
		(bcm_notif_client_callback)wlc_fbtap_scb_state_upd, (void*)fbt_info)
		!= BCME_OK) {
		WL_ERROR(("fbt scb state upd register failed\n"));
		goto err;
	}

	/* reserve cubby in the scb container for per-scb private data */
	fbt_info->scbh = wlc_scb_cubby_reserve(wlc, sizeof(wlc_fbt_scb_cubby_t),
		wlc_fbt_scb_init, wlc_fbt_scb_deinit, wlc_fbt_scb_dump, (void*)fbt_info);

	if (fbt_info->scbh < 0) {
		WL_ERROR(("wl%d: %s: wlc_scb_cubby_reserve() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* Disable FBT when in AP mode. Enable through iovar */
	wlc->pub->_fbt = FALSE;
#endif /* AP */
	/* sort calc_len/build callbacks for Action frame */
	(void)wlc_ier_sort_cbtbl(wlc->ier_fbt, fbt_ie_tags, sizeof(fbt_ie_tags));

	/* sort calc_len/build callbacks for FBT RIC(Resource Request) registry */
	(void)wlc_ier_sort_cbtbl(wlc->ier_ric, ric_ie_tags, sizeof(ric_ie_tags));

	/* calc/build */
	/* authreq */
#ifdef WLFBT_STA_OVERRIDES
	rsniefstbmp |= FT2BMP(FC_REASSOC_REQ);
#endif /* WLFBT_STA_OVERRIDES */

#ifdef FBT_STA
	if (wlc_iem_add_build_fn_mft(wlc->iemi, rsniefstbmp, DOT11_MNG_RSN_ID,
	      wlc_fbt_auth_calc_rsn_ie_len, wlc_fbt_auth_write_rsn_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, rsn in assocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* authreq/assocreq/reassocreq */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, mdiefstbmp, DOT11_MNG_MDIE_ID,
	      wlc_fbt_calc_md_ie_len, wlc_fbt_write_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, md ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* authreq/reassocreq */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, ftiefstbmp, DOT11_MNG_FTIE_ID,
	      wlc_fbt_calc_ft_ie_len, wlc_fbt_write_ft_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, ft ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* ftreq */
	if (wlc_ier_add_build_fn(wlc->ier_fbt, DOT11_MNG_RSN_ID,
	      wlc_fbt_auth_calc_rsn_ie_len, wlc_fbt_auth_write_rsn_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_ier_add_build_fn failed, rsn ie in ftreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* ftreq */
	if (wlc_ier_add_build_fn(wlc->ier_fbt, DOT11_MNG_MDIE_ID,
	      wlc_fbt_calc_md_ie_len, wlc_fbt_write_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_ier_add_build_fn failed, mdie in ftreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* ftreq */
	if (wlc_ier_add_build_fn(wlc->ier_fbt, DOT11_MNG_FTIE_ID,
	      wlc_fbt_calc_ft_ie_len, wlc_fbt_write_ft_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_ier_add_build_fn failed, ftie in ftreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* reassocreq */
	/* reassocresp */
	if (wlc_ier_add_build_fn(wlc->ier_ric, DOT11_MNG_RDE_ID,
	      wlc_fbt_calc_rde_ie_len, wlc_fbt_write_rde_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_ier_add_build_fn failed, rde ie in reassocreq\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* parse */
	/* bcn/prbrsp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, scanfstbmp, DOT11_MNG_MDIE_ID,
	                             wlc_fbt_scan_parse_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, md in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* bcn/prbrsp */
	if (wlc_iem_vs_add_parse_fn_mft(wlc->iemi, scanfstbmp, WLC_IEM_VS_IE_PRIO_CCX_EXT_CAP,
	                             wlc_fbt_scan_parse_ccx_ext_cap_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, ccx ext cap ie in scan\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* reassocresp */
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_REASSOC_RESP, DOT11_MNG_RSN_ID,
	                             wlc_fbt_parse_rsn_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, rsn ie */\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* authresp/assocresp/reassocresp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, ftfstbmp, DOT11_MNG_MDIE_ID,
	                         wlc_fbt_parse_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, md ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* authresp/assocresp/reassocresp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, ftfstbmp, DOT11_MNG_FTIE_ID,
	                         wlc_fbt_parse_ft_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, ft ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* reassocresp */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, FC_REASSOC_RESP, DOT11_MNG_RDE_ID,
	                                wlc_fbt_parse_rde_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_vs_add_parse_fn failed, rde ie in reassocresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* ftresp */
	if (wlc_ier_add_parse_fn(wlc->ier_fbt, DOT11_MNG_RSN_ID,
	                        wlc_fbt_parse_rsn_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, rsn ie in ftresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* ftresp */
	if (wlc_ier_add_parse_fn(wlc->ier_fbt, DOT11_MNG_MDIE_ID,
	                        wlc_fbt_parse_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, md ie in ftresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* ftresp */
	if (wlc_ier_add_parse_fn(wlc->ier_fbt, DOT11_MNG_FTIE_ID,
	                        wlc_fbt_parse_ft_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_add_parse_fn failed, ft ie in ftresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
#endif /* FBT_STA */
#ifdef AP
	/* parse */
	/* assocreq/reassocreq */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, apassocreqbmp, DOT11_MNG_RSN_ID,
	                             wlc_fbtap_parse_rsn_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, rsn ie */\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* assocreq/reassocreq */
	if (wlc_iem_add_parse_fn_mft(wlc->iemi, apassocreqbmp, DOT11_MNG_MDIE_ID,
	                         wlc_fbtap_parse_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, md ie\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* auth */
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_AUTH, DOT11_MNG_RSN_ID,
	                             wlc_fbtap_auth_parse_rsn_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, rsn ie */\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_AUTH, DOT11_MNG_MDIE_ID,
	                             wlc_fbtap_parse_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, md ie */\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_AUTH, DOT11_MNG_FTIE_ID,
	                             wlc_fbtap_auth_parse_ft_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, ft ie */\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* reassocreq */
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_REASSOC_REQ, DOT11_MNG_FTIE_ID,
	                             wlc_fbtap_parse_ft_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, ft ie */\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* reassocreq */
	if (wlc_iem_add_parse_fn(wlc->iemi, FC_REASSOC_REQ, DOT11_MNG_RDE_ID,
	                             wlc_fbtap_parse_rde_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s wlc_iem_add_parse_fn failed, rde ie */\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* build */
	/* reassocresp */
	if (wlc_iem_add_build_fn(wlc->iemi, FC_REASSOC_RESP, DOT11_MNG_RSN_ID,
	      wlc_fbtap_calc_rsn_ie_len, wlc_fbtap_write_rsn_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn_mft failed, rsn ie in reassocresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* bcn/prbrsp/assocresp/reassocresp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, apmdiefstbmp, DOT11_MNG_MDIE_ID,
	      wlc_fbtap_calc_md_ie_len, wlc_fbtap_write_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn_mft failed, md ie in bcn\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* assocresp/reassocresp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, apftiefstbmp, DOT11_MNG_FTIE_ID,
	      wlc_fbtap_calc_ft_ie_len, wlc_fbtap_write_ft_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn_mft failed, ft ie in assocresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* authresp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, apauthftiefstbmp, DOT11_MNG_RSN_ID,
	      wlc_fbtap_auth_calc_rsn_ie_len, wlc_fbtap_auth_write_rsn_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn_mft failed, rsn ie in authresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* authresp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, apauthftiefstbmp, DOT11_MNG_MDIE_ID,
	      wlc_fbtap_auth_calc_md_ie_len, wlc_fbtap_auth_write_md_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn_mft failed, md ie in authresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	/* authresp */
	if (wlc_iem_add_build_fn_mft(wlc->iemi, apauthftiefstbmp, DOT11_MNG_FTIE_ID,
	      wlc_fbtap_auth_calc_ft_ie_len, wlc_fbtap_auth_write_ft_ie, fbt_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn_mft failed, ft ie in authresp\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}
#endif /* AP */

	return fbt_info;
err:
	wlc_fbt_detach(fbt_info);
	return NULL;
}

void
BCMATTACHFN(wlc_fbt_detach)(wlc_fbt_info_t *fbt_info)
{
	wlc_fbt_priv_t *fbt_priv;

	if (!fbt_info)
		return;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	WL_TRACE(("wl%d: wlc_fbt_detach\n", UNIT(fbt_priv)));

#if defined(STA) && defined(FBT_STA)
	wlc_bss_assoc_state_unregister(fbt_priv->wlc, wlc_fbt_handle_joinproc, fbt_info);
#endif /* STA && FBT_STA */
#ifdef BCMSUP_PSK
	if (SUP_ENAB(fbt_priv->wlc->pub))
		wlc_sup_up_down_unregister(fbt_priv->wlc, wlc_fbt_sup_updown_callbk, fbt_info);
#endif /* BCMSUP_PSK */
	wlc_bsscfg_updown_unregister(fbt_priv->wlc, wlc_fbt_bsscfg_updown_callbk, fbt_info);

#ifdef AP
	/* free FBT timer */
	if (fbt_info->fbt_timer) {
		wl_free_timer(fbt_priv->wlc->wl, fbt_info->fbt_timer);
	}
#endif /* AP */
	wlc_module_unregister(fbt_priv->pub, "fbt", fbt_info);
	MFREE(fbt_priv->osh, fbt_info, sizeof(wlc_fbt_info_t));
}

static int
wlc_fbt_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)handle;
	wlc_fbt_priv_t	*fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc_info_t *wlc = fbt_priv->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
	bool bool_val = FALSE;

#ifndef WLFBT_DISABLED
	int32 *ret_int_ptr;
	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;
#endif /* WLFBT_DISABLED */
	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	bool_val = (int_val != 0) ? TRUE : FALSE;
	(void)bool_val;

	switch (actionid) {
#ifndef WLFBT_DISABLED
	case IOV_GVAL(IOV_ALLOW_FBTOVERDS):
		{
			*ret_int_ptr = ((bsscfg->flags & WLC_BSSCFG_ALLOW_FTOVERDS) != 0);
			break;
		}

	case IOV_SVAL(IOV_ALLOW_FBTOVERDS):
		{
			if (bool_val)
				bsscfg->flags |= WLC_BSSCFG_ALLOW_FTOVERDS;
			else
				bsscfg->flags &= ~WLC_BSSCFG_ALLOW_FTOVERDS;
#ifdef AP
			if (BSSCFG_AP(bsscfg)) {
				wlc_fbt_fbtoverds_enable(wlc, bsscfg, bool_val);
			}
#endif /* AP */
			break;
		}

	case IOV_GVAL(IOV_FBT_CAP):
		{
			if (wlc->pub) {
				*ret_int_ptr = WLFBT_CAP(wlc->pub);
			} else {
				err = BCME_UNSUPPORTED;
			}
			break;
		}

	case IOV_GVAL(IOV_FBT):
		{
			*ret_int_ptr = WLFBT_ENAB(wlc->pub) && BSSCFG_IS_FBT(bsscfg);
			break;
		}
	case IOV_SVAL(IOV_FBT):
		{
			if (WLFBT_CAP(wlc->pub)) { // check capability
				wlc->pub->_fbt = (uint16) int_val; // toggle fbt
				bsscfg->flags2 &= ~(WLC_BSSCFG_FL2_FBT_MASK); // clear
				if (int_val == 0) { // disabled
					break;
				}
				/* wpa_auth must be WPA2 capable before enabling fbt */
				if (bsscfg->WPA_auth & WPA2_AUTH_UNSPECIFIED) {
					bsscfg->flags2 |= WLC_BSSCFG_FL2_FBT_1X;
				} else if (bsscfg->WPA_auth & WPA2_AUTH_PSK) {
					bsscfg->flags2 |= WLC_BSSCFG_FL2_FBT_PSK;
				} else {
					err = BCME_BADARG;
				}
			} else {
				err = BCME_UNSUPPORTED;
			}
			break;
		}
#ifdef AP
	case IOV_GVAL(IOV_FBT_AP):
		{
			*ret_int_ptr = BSSCFG_IS_FBT(bsscfg);
			break;
		}
	case IOV_SVAL(IOV_FBT_AP):
		{
			uint16 ft_mode;
			ft_mode = (uint16) int_val;

			if (ft_mode) {
				if (!WLFBT_ENAB(wlc->pub)) {
					err = BCME_ERROR;
					break;
				}
				/* wpa_auth must be WPA2 capable before enabling fbt_ap */
				if (bsscfg->WPA_auth & WPA2_AUTH_UNSPECIFIED) {
					bsscfg->flags2 = bsscfg->flags2 &
						~(WLC_BSSCFG_FL2_FBT_1X|WLC_BSSCFG_FL2_FBT_PSK);
					bsscfg->flags2 |= WLC_BSSCFG_FL2_FBT_1X;
				}
				else if (bsscfg->WPA_auth & WPA2_AUTH_PSK) {
					bsscfg->flags2 = bsscfg->flags2 &
						~(WLC_BSSCFG_FL2_FBT_1X|WLC_BSSCFG_FL2_FBT_PSK);
					bsscfg->flags2 |= WLC_BSSCFG_FL2_FBT_PSK;
				}
				else {
					err = BCME_BADARG;
				}
			}
			else {
				bsscfg->flags2 = bsscfg->flags2 &
					~(WLC_BSSCFG_FL2_FBT_1X|WLC_BSSCFG_FL2_FBT_PSK);
			}
			if (err != BCME_OK)
				break;
			break;
		}
	case IOV_GVAL(IOV_FBT_MDID):
		{
			*ret_int_ptr = bsscfg->fbt_mdid;
			break;
		}
	case IOV_SVAL(IOV_FBT_MDID):
		{
			uint16 mdid;
			mdid = (uint16) int_val;
			if (mdid == bsscfg->fbt_mdid)
				break;
			bsscfg->fbt_mdid = mdid;
			if (!WLFBT_ENAB(wlc->pub)) /* global mode is disabled return */
				break;
			break;
		}
	case IOV_GVAL(IOV_FBT_R0KH_ID):
		{
			if (alen < strlen(bsscfg->fbt_r0kh_id) + 1)
				return BCME_BUFTOOSHORT;
			strncpy(arg, bsscfg->fbt_r0kh_id,
					(strlen(bsscfg->fbt_r0kh_id) + 1));
			break;
		}
	case IOV_SVAL(IOV_FBT_R0KH_ID):
		{
			int slen;
			/* find strlen, with string either null terminated or 'len' terminated */
			for (slen = 0; slen < alen && ((char*)arg)[slen] != '\0'; slen++)
				;
			if (slen >= FBT_R0KH_ID_LEN)
				return BCME_BADLEN;
			memset(bsscfg->fbt_r0kh_id, 0, FBT_R0KH_ID_LEN);
			strncpy(bsscfg->fbt_r0kh_id, arg, slen);
			bsscfg->fbt_r0kh_id_len = slen;
			break;
		}
	case IOV_GVAL(IOV_FBT_R1KH_ID):
		{
			if (alen < ETHER_ADDR_LEN)
				return BCME_BUFTOOSHORT;
			memcpy((uint8 *)arg, bsscfg->fbt_r1kh_id, ETHER_ADDR_LEN);
			break;
		}
	case IOV_SVAL(IOV_FBT_R1KH_ID):
		{
			if (alen != ETHER_ADDR_LEN)
				return BCME_BADLEN;
			memcpy(bsscfg->fbt_r1kh_id, arg, ETHER_ADDR_LEN);
			break;
		}
	case IOV_GVAL(IOV_FBT_REASSOC_TIME):
		{
			*ret_int_ptr = bsscfg->fbt_reassoc_time;
			break;
		}
	case IOV_SVAL(IOV_FBT_REASSOC_TIME):
		{
			bsscfg->fbt_reassoc_time = *(uint *)arg;
			break;
		}
#endif /* WLFBT_DISABLED */
#endif /* AP */
#ifdef AP
	case IOV_SVAL(IOV_FBT_AUTH_RESP):
		{
			struct scb *scb;
			struct ether_addr sta_mac;
#ifdef BCMDBG
			char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */
			wlc_fbt_auth_resp_t *fbt_auth_resp;
			fbt_auth_resp = (wlc_fbt_auth_resp_t *)arg;
			memcpy(sta_mac.octet, fbt_auth_resp->macaddr, ETHER_ADDR_LEN);

			scb = wlc_scbfind(wlc, bsscfg, &sta_mac);
			if (scb == NULL) {
				WL_FBT(("wl%d: %s: FBT auth response for unknown MAC %s\n",
					wlc->pub->unit, __FUNCTION__,
					bcm_ether_ntoa((struct ether_addr *)&fbt_auth_resp->macaddr,
					eabuf)));
				return BCME_ERROR;
			}
			err = wlc_fbtap_process_auth_resp(wlc, fbt_info, scb, fbt_auth_resp);
			break;
		}
	case IOV_SVAL(IOV_FBT_DS_ADD_STA):
		{
			wlc_fbt_auth_resp_t *fbt_auth_resp;
			fbt_auth_resp = (wlc_fbt_auth_resp_t *)arg;
			err = wlc_fbtap_process_ds_auth_resp(wlc, fbt_info, bsscfg, fbt_auth_resp);
			break;
		}
	case IOV_SVAL(IOV_FBT_ACT_RESP):
		{
			wlc_fbt_action_resp_t *fbt_act_resp;
			fbt_act_resp = (wlc_fbt_action_resp_t *)arg;
			err = wlc_fbtap_process_action_resp(wlc, fbt_info, bsscfg, fbt_act_resp);
			break;
		}
#endif /* AP */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}
	return err;
}
#if defined(STA) && defined(FBT_STA)
void
wlc_fbt_clear_ies(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	wlc_fbt_free_ies(fbt_info, cfg);

	if (fbt_bss_priv->wpa) {
		wlc_wpapsk_free(fbt_bss_priv->wlc, fbt_bss_priv->wpa);
	}
	if (fbt_bss_priv->wpa_info) {
		fbt_bss_priv->wpa_info->pmk_len = 0;
	}
}

void
wlc_fbt_calc_fbt_ptk(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	wpapsk_t *wpa;
	wpapsk_info_t *wpa_info;
	uchar pmkr0[PMK_LEN], pmkr1[PMK_LEN];

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (!fbt_bss_priv)
		return;

	if (fbt_bss_priv->ftie) {
		wpa = fbt_bss_priv->wpa;
		wpa_info = fbt_bss_priv->wpa_info;

		/* calc PMK-R0 */
		wpa_calc_pmkR0(BSS_SSID(fbt_bss_priv), BSS_SSID_LEN(fbt_bss_priv),
			fbt_bss_priv->mdid,
			fbt_bss_priv->r0khid->data, fbt_bss_priv->r0khid->len,
			&CUR_EA(fbt_bss_priv),	wpa_info->pmk, (uint)wpa_info->pmk_len,
			pmkr0, fbt_bss_priv->pmkr0name);

		/* calc PMK-R1 */
		wpa_calc_pmkR1((struct ether_addr *)fbt_bss_priv->r1khid->data,
			&CUR_EA(fbt_bss_priv), pmkr0, PMK_LEN, fbt_bss_priv->pmkr0name,
			pmkr1, fbt_bss_priv->pmkr1name);

		/* calc PTK */
		wpa_calc_ft_ptk(&PEER_EA(fbt_bss_priv), &CUR_EA(fbt_bss_priv),
			wpa->anonce, wpa->snonce, pmkr1, PMK_LEN,
			wpa->eapol_mic_key, (uint)wpa->ptk_len);
#ifdef BCMDBG
		wlc_fbt_dump_fbt_keys(fbt_bss_priv, pmkr0, pmkr1);
#endif // endif
	}
}
#endif /* STA && FBT_STA */

#ifdef BCMDBG
#if defined(STA) && defined(FBT_STA)
static void
wlc_fbt_dump_fbt_keys(bss_fbt_priv_t *fbt_bss_priv, uchar *pmkr0, uchar *pmkr1)
{
	if (WL_WSEC_ON()) {
		uchar *ptk;
		int i;

		if (!fbt_bss_priv)
			return;

		prhex("PMK", fbt_bss_priv->wpa_info->pmk, (uint)fbt_bss_priv->wpa_info->pmk_len);

		prhex("PMK-R0", pmkr0, 32);

		printf("R0KHID len %d : \n", fbt_bss_priv->r0khid->len);
		prhex("R0KHID", fbt_bss_priv->r0khid->data, fbt_bss_priv->r0khid->len);

		printf("R1KHID len %d : \n", fbt_bss_priv->r1khid->len);
		prhex("R1KHID", fbt_bss_priv->r1khid->data, fbt_bss_priv->r1khid->len);

		prhex("PMK-R0name", fbt_bss_priv->pmkr0name, 16);

		prhex("PMK-R1", pmkr1, 32);

		prhex("PMK-R1name", fbt_bss_priv->pmkr1name, 16);

		prhex("Anonce", fbt_bss_priv->wpa->anonce, 32);
		prhex("Snonce", fbt_bss_priv->wpa->snonce, 32);

		printf("BSSID : \n");
		for (i = 0; i < 6; i++) {
			printf(" 0x%2x ", PEER_EA(fbt_bss_priv).octet[i]);
		}
		printf("\n");

		ptk = (uchar *)fbt_bss_priv->wpa->eapol_mic_key;
		prhex("PTK", ptk, WPA_MIC_KEY_LEN);
	}
}
#endif /* STA && FBT_STA */

static void
wlc_fbt_bsscfg_dump(void *context, wlc_bsscfg_t *bsscfg, struct bcmstrbuf *b)
{
#ifdef AP
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)context;

	WL_ERROR(("%s: cfgh: %p scbh %d\n", __FUNCTION__,
		FBT_BSSCFG_CUBBY_LOC(fbt_info, bsscfg), fbt_info->scbh));
#endif /* AP */
}
#endif /* BCMDBG */

#if defined(STA) && defined(FBT_STA)
bool
wlc_fbt_is_cur_mdid(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, wlc_bss_info_t *bi)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	return ((fbt_bss_priv->mdie_len != 0) && (bi->flags & WLC_BSS_FBT) &&
		(bi->mdid == fbt_bss_priv->mdid));
}
#endif /* STA && FBT_STA */

/* Calculate the RIC IE endptr and IE count. */
static bool
wlc_fbt_parse_ric(uint8 *pbody, int len, uint8 **ricend, int *ric_ie_count)
{
	bool found = FALSE;
	uint8 *ricptr;
	uint8 *_ricend = NULL;
	uint8 *end = NULL;
	int _ric_ie_count = 0;

	ricptr = pbody;
	end = pbody + len;
	while (ricptr < end) {
		bcm_tlv_t *ie = (bcm_tlv_t*)ricptr;
		int ie_len = TLV_HDR_LEN + ie->len;
		ricptr += ie_len;
		if (ie->id == DOT11_MNG_RDE_ID) {
			int i;
			dot11_rde_ie_t *rde = (dot11_rde_ie_t*)ie;

			/* Include RDE in MIC calculations. */
			_ric_ie_count += 1;
			/* Include protected elements too. */
			_ric_ie_count += rde->rd_count;
			for (i = 0; i < rde->rd_count; i++) {
				bcm_tlv_t *ie = (bcm_tlv_t*)ricptr;

				ricptr += TLV_HDR_LEN + ie->len;
				_ricend = ricptr;
			}
			/* Set return val. */
			found = TRUE;
		}
	}

	if (found) {
		*ricend = _ricend;
		*ric_ie_count = _ric_ie_count;
	}
	return found;
}

/* Populates the MIC and MIC control fields of the supplied FTIE. */
static bool
wlc_fbt_ft_calc_mic(bss_fbt_priv_t *fbt_bss_priv, wlc_bsscfg_t *cfg, struct scb *scb,
        dot11_ft_ie_t *ftie, bcm_tlv_t *mdie, bcm_tlv_t *rsnie, uint8 *ricdata,
        int ricdata_len, int ric_ie_count, uint8 trans_seq_nbr)
{
	int total_len = 0;
	uint8 *micdata;
	uint8 *pos;
	uint8 mic[PRF_OUTBUF_LEN];
	uint prot_ie_len = 3 + ric_ie_count;
#ifdef AP
	wlc_fbt_scb_t *fbt_scb = NULL;

	if (BSSCFG_AP(cfg)) {
		ASSERT(scb != NULL);
		fbt_scb = FBT_SCB(fbt_bss_priv->wlc->fbt, scb);
		if (fbt_scb == NULL)
			return FALSE;
	}

#endif /* AP */

	/* See 802.11r 11A.8.4 & 11A.8.5 for details. */
	/* Total expected size of buffer needed to calculate the MIC. */
	total_len += 2 * ETHER_ADDR_LEN;	/* AP & STA MAC addresses. */
	total_len += 1;				/* Transaction sequence number byte. */
	total_len += TLV_HDR_LEN + rsnie->len;
	total_len += TLV_HDR_LEN + mdie->len;
	total_len += TLV_HDR_LEN + ftie->len;
	total_len += ricdata_len;

	micdata = MALLOC(fbt_bss_priv->osh, total_len);
	if (micdata == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
		return FALSE;
	}
	pos = micdata;

	/* calc mic */
	if (BSSCFG_AP(cfg)) {
		bcopy((uint8 *)&PEER_EA(fbt_bss_priv), pos, ETHER_ADDR_LEN);
		pos += ETHER_ADDR_LEN;
		bcopy((uint8 *)&CUR_EA(fbt_bss_priv), pos, ETHER_ADDR_LEN);
		pos += ETHER_ADDR_LEN;
	}
	else {
		bcopy((uint8 *)&CUR_EA(fbt_bss_priv), pos, ETHER_ADDR_LEN);
		pos += ETHER_ADDR_LEN;
		bcopy((uint8 *)&PEER_EA(fbt_bss_priv), pos, ETHER_ADDR_LEN);
		pos += ETHER_ADDR_LEN;
	}

	/* Transaction sequence number. The value is dependent on whether we're called for an
	 * (re)association request, (re)association response, or other.
	 */
	*pos++ = trans_seq_nbr;

	bcopy(rsnie, pos, rsnie->len + TLV_HDR_LEN);
	pos += rsnie->len + TLV_HDR_LEN;

	bcopy(mdie, pos, mdie->len + TLV_HDR_LEN);
	pos += mdie->len + TLV_HDR_LEN;

	/* Prepare the FTIE. Set protected frame count in MIC control and zero the MIC field. */
	ftie->mic_control = htol16(prot_ie_len << 8);
	bzero(ftie->mic, sizeof(ftie->mic));

	bcopy(ftie, pos, ftie->len + TLV_HDR_LEN);
	pos += ftie->len + TLV_HDR_LEN;

	/* Add any RIC IEs to MIC data. */
	if (ricdata && ricdata_len) {
		/* Include RDE and counted frames in protected IE calculations. */
		bcopy(ricdata, pos, ricdata_len);
		pos += ricdata_len;
	}

#ifdef AP
	if (BSSCFG_AP(cfg)) {
		aes_cmac_calc(micdata, pos - micdata, fbt_scb->ptk.kck,
			EAPOL_WPA_KEY_MIC_LEN, mic, EAPOL_WPA_KEY_MIC_LEN);
	}
	else
#endif /* AP */
	aes_cmac_calc(micdata, pos - micdata, fbt_bss_priv->wpa->eapol_mic_key,
		EAPOL_WPA_KEY_MIC_LEN, mic, EAPOL_WPA_KEY_MIC_LEN);
	bcopy(mic, &ftie->mic[0], EAPOL_WPA_KEY_MIC_LEN);
	MFREE(fbt_bss_priv->osh, micdata, total_len);

	return TRUE;
}

#if defined(STA) && defined(FBT_STA)
static bool
wlc_fbt_auth_build_rsnie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, uint8 *pbody)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	wpa_pmkid_list_t *wpa_pmkid;
	wpapsk_t *wpa;

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	wpa = fbt_bss_priv->wpa;

	bcopy(wpa->sup_wpaie, pbody, wpa->sup_wpaie_len);
	if (wpa->sup_wpaie_len != 0) {
		/* Add pmkr0name to rsnie */
		pbody[1] += sizeof(wpa_pmkid_list_t);
		pbody += wpa->sup_wpaie_len;
		wpa_pmkid = (wpa_pmkid_list_t *)pbody;
		wpa_pmkid->count.low = 1;
		wpa_pmkid->count.high = 0;
		bcopy(fbt_bss_priv->pmkr0name, &wpa_pmkid->list[0], WPA2_PMKID_LEN);
	}
	return TRUE;
}

static bool
wlc_fbt_auth_build_mdie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, uint8 *pbody)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	/* Add mdie */
	/* XXX This is actually the mdie from the AP we did the inital auth from.
	 * Should we update this from the beacon/prb_resp of the AP we are roaming to.
	 */
	if (fbt_bss_priv->mdie != NULL) {
		bcopy(fbt_bss_priv->mdie, pbody, fbt_bss_priv->mdie_len);
	}
	return TRUE;
}

static bool
wlc_fbt_auth_build_ftie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, uint8 *pbody)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	/* Add FTIE with snonce, r0kh-id */
	if (fbt_bss_priv->r0khid != NULL) {
		dot11_ft_ie_t *ftie = (dot11_ft_ie_t *)pbody;
		wpapsk_t *wpa = fbt_bss_priv->wpa;

		bzero(ftie, sizeof(dot11_ft_ie_t));
		ftie->id = DOT11_MNG_FTIE_ID;
		ftie->len = sizeof(dot11_ft_ie_t) + fbt_bss_priv->r0khid->len;
		++wpa->snonce[0];
		bcopy(wpa->snonce, ftie->snonce, EAPOL_WPA_KEY_NONCE_LEN);
		pbody += sizeof(dot11_ft_ie_t);
		ASSERT((fbt_bss_priv->ftie_len >= sizeof(dot11_ft_ie_t)) &&
			(fbt_bss_priv->ftie_len < 255));
		bcopy(fbt_bss_priv->r0khid, pbody, TLV_HDR_LEN + fbt_bss_priv->r0khid->len);
	}
	return TRUE;
}

/* Save a copy of RSN IE from Assoc req */
static bool
wlc_fbt_find_rsnie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	/* Do some initialisation that would normally be done by wpa_start()
	 * This is for the case where the external supplicant is performing the 4-way handshake but
	 * we'd still like to do reassociations within the ESS.
	 */
	if (fbt_bss_priv) {
		wpapsk_t *wpa = fbt_bss_priv->wpa;
		uint8 *sup_ies, *auth_ies;
		uint sup_ies_len, auth_ies_len;
		uchar *sup_wpaie;

		wlc_find_sup_auth_ies(NULL, cfg, &sup_ies, &sup_ies_len,
			&auth_ies, &auth_ies_len);

		sup_wpaie = (uchar *)bcm_parse_tlvs(sup_ies, sup_ies_len, DOT11_MNG_RSN_ID);
		if (sup_wpaie == NULL) {
			WL_ERROR(("wl%d: %s: STA WPA IE not found in sup_ies with len %d\n",
				UNIT(fbt_bss_priv), __FUNCTION__, sup_ies_len));
			return FALSE;
		}
		if (wpa->sup_wpaie)
			MFREE(fbt_bss_priv->osh, wpa->sup_wpaie, wpa->sup_wpaie_len);

		/* Save copy of STA's WPA IE */
		wpa->sup_wpaie_len = (uint16) (sup_wpaie[1] + TLV_HDR_LEN);
		wpa->sup_wpaie = (uchar *) MALLOC(fbt_bss_priv->osh, wpa->sup_wpaie_len);
		if (!wpa->sup_wpaie) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
			return FALSE;
		}
		bcopy((char *)sup_wpaie, (char *)wpa->sup_wpaie, wpa->sup_wpaie_len);

		wlc_getrand(fbt_bss_priv->wlc, wpa->snonce, EAPOL_WPA_KEY_NONCE_LEN);
		wpa->ptk_len = AES_PTK_LEN;
		wpa->tk_len = AES_TK_LEN;
		wpa->desc = WPA_KEY_DESC_V2;
	}

	return TRUE;
}

/* Parse auth resp frame for mdie */
static bool
wlc_fbt_auth_parse_mdie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, uint8 *body, int body_len)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	dot11_mdid_ie_t *mdie = (dot11_mdid_ie_t *)body;

	if (!fbt_bss) {
		return FALSE;
	}
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (fbt_bss_priv->mdie_len) {
		return TRUE;
	}

	if ((BCM_TLV_SIZE(mdie) != sizeof(*mdie)) || (body_len < (int)sizeof(*mdie))) {
		WL_ERROR(("wl%d.%d: %s:MDIE: fail\n",
			UNIT(fbt_bss_priv), WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		return FALSE;
	}

	fbt_bss_priv->mdie_len = mdie->len + TLV_HDR_LEN;
	fbt_bss_priv->mdie = MALLOC(fbt_bss_priv->osh, fbt_bss_priv->mdie_len);
	if (!fbt_bss_priv->mdie) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		   UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
		fbt_bss_priv->mdie_len = 0;
		return FALSE;
	}
	memcpy(fbt_bss_priv->mdie, mdie, fbt_bss_priv->mdie_len);
	fbt_bss_priv->mdid = ltoh16_ua(&mdie->mdid);
	return TRUE;
}

/* Parse auth resp frame for ftie */
static bool
wlc_fbt_auth_parse_ftie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, uint8 *body, int body_len)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	dot11_ft_ie_t *ftie = (dot11_ft_ie_t *)body;
	wpapsk_t *wpa;
	uchar *tlvs;
	uint tlv_len;

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	wpa = fbt_bss_priv->wpa;

	/* Compare the received SNonce with the current SNonce and overwrite the
	 * current FT IE only if they match. Else return error for frame to be discarded.
	 */
	if (cfg->auth_atmptd == DOT11_FAST_BSS && fbt_bss_priv->ftie &&
		memcmp(fbt_bss_priv->wpa->snonce, ftie->snonce, EAPOL_WPA_KEY_NONCE_LEN)) {
		WL_ERROR(("wl%d.%d: %s:FTIE: Received SNonce does not match current SNonce\n",
			UNIT(fbt_bss_priv), WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		return DOT11_SC_INVALID_SNONCE;
	}
	/* ensure minimal ft ie length */
	if ((ftie->len + TLV_HDR_LEN) < sizeof(dot11_ft_ie_t)) {
		WL_ERROR(("wl%d.%d: %s:FTIE: Received ft ie size %d too short\n",
			UNIT(fbt_bss_priv), WLC_BSSCFG_IDX(cfg), __FUNCTION__,
			ftie->len + TLV_HDR_LEN));
		return DOT11_SC_FAILURE;
	}

	if (fbt_bss_priv->ftie)
		MFREE(fbt_bss_priv->osh, fbt_bss_priv->ftie, fbt_bss_priv->ftie_len);

	fbt_bss_priv->ftie_len = ftie->len + TLV_HDR_LEN;
	fbt_bss_priv->ftie = MALLOC(fbt_bss_priv->osh, fbt_bss_priv->ftie_len);
	if (!fbt_bss_priv->ftie) {
		fbt_bss_priv->ftie_len = 0;
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
		return FALSE;
	}
	bcopy(ftie, fbt_bss_priv->ftie, fbt_bss_priv->ftie_len);

	tlvs = (uchar *)((uintptr)fbt_bss_priv->ftie + sizeof(dot11_ft_ie_t));
	tlv_len = fbt_bss_priv->ftie_len - sizeof(dot11_ft_ie_t);
	fbt_bss_priv->r1khid = bcm_parse_tlvs(tlvs, tlv_len, DOT11_FBT_SUBELEM_ID_R1KH_ID);
	fbt_bss_priv->r0khid = bcm_parse_tlvs(tlvs, tlv_len, DOT11_FBT_SUBELEM_ID_R0KH_ID);
	if (!fbt_bss_priv->r0khid || !fbt_bss_priv->r1khid) {
		WL_ERROR(("wl%d.%d: %s:FTIE: Received ft ie is missing r0/r khid\n",
		UNIT(fbt_bss_priv), WLC_BSSCFG_IDX(cfg), __FUNCTION__));
		return DOT11_SC_FAILURE;
	}
	bcopy(ftie->anonce, wpa->anonce, sizeof(wpa->anonce));
	wlc_fbt_calc_fbt_ptk(fbt_info, cfg);
	return TRUE;
}

static bool
wlc_fbt_reassoc_build_ftie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg,
   uint8 *buf, int buflen, bcm_tlv_t *mdie, bcm_tlv_t *rsnie,
   uint8 *ricdata, int ricdata_len, int ric_ie_count)
{
	bool mic_ok = FALSE;
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (fbt_bss_priv->ftie && fbt_bss_priv->ftie_len) {
		uint8 *ftptr;

		ftptr = MALLOC(fbt_bss_priv->osh, fbt_bss_priv->ftie_len);
		if (ftptr == NULL) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			   UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
			return FALSE;
		} else {
			bcopy(fbt_bss_priv->ftie, ftptr, fbt_bss_priv->ftie_len);
			/* Calculate MIC */
			if (wlc_fbt_ft_calc_mic(fbt_bss_priv, cfg, NULL, (dot11_ft_ie_t*)ftptr,
				mdie, rsnie, ricdata, ricdata_len, ric_ie_count,
				FT_MIC_ASSOC_REQUEST_TSN)) {
				mic_ok = TRUE;
				bcm_copy_tlv_safe(ftptr, buf, buflen);
			} else {
				mic_ok = FALSE;
			}

			MFREE(fbt_bss_priv->osh, ftptr, fbt_bss_priv->ftie_len);
		}
	}
	return mic_ok;
}
#endif /* STA && FBT_STA */

/*
 * wlc_fbt_reassoc_validate_ftie: Checks the validity of the FTIE (if provided) in the reassociation
 * request response frame.
 *
 * Returns TRUE if the reassociation request response frame is good, FALSE otherwise.
 * A FALSE return will mean that the frame contained an FTIE but was missing either an MDIE or an
 * RSNIE (shouldn't really happen) or that the FT MIC was invalid. In either case, the calling
 * function should discard the reassociation request response frame.
 */
static bool
wlc_fbt_reassoc_validate_ftie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t* cfg, struct scb *scb,
	uint8 *ricie, int ricie_len, dot11_ft_ie_t *ftie, bcm_tlv_t *mdie, bcm_tlv_t *rsnie,
	uint8 trans_seq_nbr)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	bool is_valid = TRUE;

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (ftie != NULL) {
		uint8 *ricend = NULL;
		int ric_ie_count = 0;
		dot11_ft_ie_t *myftie;

		if (mdie == NULL) {
			WL_ERROR(("%s: MDIE not found and required\n", __FUNCTION__));
			return FALSE;
		}
		if (rsnie == NULL) {
			WL_ERROR(("%s: RSNIE not found and required\n", __FUNCTION__));
			return FALSE;
		}

		/* Copy received FTIE into own buffer as MIC calculation will modify it. */
		if (BCM_TLV_SIZE(ftie) < sizeof(dot11_ft_ie_t)) {
			return FALSE; /* not meet fixed lenth, hence return */
		}

		myftie = MALLOC(fbt_bss_priv->osh, ftie->len + TLV_HDR_LEN);
		if (myftie == NULL) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			   UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
			return FALSE;
		}
		if (ricie != NULL) {
			wlc_fbt_parse_ric(ricie, ricie_len, &ricend, &ric_ie_count);
		}
		/* Copy received FTIE into own buffer as MIC calculation will modify it. */
		memcpy(myftie, ftie, ftie->len + TLV_HDR_LEN);
		if (wlc_fbt_ft_calc_mic(fbt_bss_priv, cfg, scb, myftie, mdie,
				rsnie, ricie, (int)(ricend - ricie), ric_ie_count, trans_seq_nbr)) {
			/* Now get the calculated MIC and compare to that in ftie. */
			if (memcmp(myftie->mic, ftie->mic, EAPOL_WPA_KEY_MIC_LEN)) {
				/* MICs are different. */
				is_valid = FALSE;
				if (WL_ERROR_ON()) {
					WL_ERROR(("%s: FT-MICs do not match\n", __FUNCTION__));
					prhex("Recv MIC", ftie->mic, EAPOL_WPA_KEY_MIC_LEN);
					prhex("Calc MIC", myftie->mic, EAPOL_WPA_KEY_MIC_LEN);
				}
			}
		} else {
			is_valid = FALSE; /* mic calc failed */
		}
		MFREE(fbt_bss_priv->osh, myftie, ftie->len + TLV_HDR_LEN);
	}
	return is_valid;
}

#if defined(STA) && defined(FBT_STA)
/* Looks for 802.11r RIC response TLVs. */
static bool
wlc_fbt_reassoc_parse_rdeie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t* cfg, uint8 *tlvs, int tlvs_len)
{
	bool accepted = TRUE;
	bcm_tlv_t *ric_ie;

	ric_ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RDE_ID);
	while (ric_ie && accepted) {
		dot11_rde_ie_t *rdeptr = (dot11_rde_ie_t*)ric_ie;
		accepted = (rdeptr->status == 0);
		if (accepted) {
			/* May contain more than one RDE, so look for more. */
			tlvs += DOT11_MNG_RDE_IE_LEN;
			tlvs_len -= DOT11_MNG_RDE_IE_LEN;
			ric_ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RDE_ID);
		}
	}
	return accepted;
}

uint8 *
wlc_fbt_get_pmkr1name(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return NULL;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	return (fbt_bss_priv->pmkr1name);
}

static bool
wlc_fbt_process_reassoc_resp(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	wlc_assoc_t *as = cfg->assoc;
	dot11_ft_ie_t *ftie;
	uint8 *tlvs;
	uint tlv_len;
	dot11_gtk_ie_t *gtk;
	wpapsk_t *wpa;

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	wpa = fbt_bss_priv->wpa;

	tlvs = (uint8 *)&as->resp[1];
	if (as->resp_len <= DOT11_ASSOC_RESP_FIXED_LEN) {
		return FALSE; /* not meet fixed lenth, hence return */
	}
	tlv_len = as->resp_len - DOT11_ASSOC_RESP_FIXED_LEN;
	ftie = (dot11_ft_ie_t *)bcm_parse_tlvs(tlvs, tlv_len, DOT11_MNG_FTIE_ID);
	if (ftie && ((ftie->len + TLV_HDR_LEN) >= sizeof(dot11_ft_ie_t))) {
		tlvs = (uint8 *)((uintptr)ftie + sizeof(dot11_ft_ie_t));
		tlv_len = ftie->len - sizeof(dot11_ft_ie_t) + TLV_HDR_LEN;
		/* plumb the keys here and set the scb authorized */
		gtk = (dot11_gtk_ie_t *)bcm_parse_tlvs(tlvs, tlv_len, DOT11_FBT_SUBELEM_ID_GTK);
		if (gtk && (gtk->len >= DOT11_FBT_SUBELEM_GTK_MIN_LEN) &&
		   (gtk->len <= MIN(DOT11_FBT_SUBELEM_GTK_MAX_LEN,
		   (sizeof(wpa->gtk) + AKW_BLOCK_LEN + DOT11_FBT_SUBELEM_GTK_FIXED_LEN)))) {
			/* extract and plumb GTK */
			if ((gtk->key_len > sizeof(wpa->gtk)) ||
				aes_unwrap(WPA_ENCR_KEY_LEN, wpa->eapol_encr_key,
				gtk->len - DOT11_FBT_SUBELEM_GTK_FIXED_LEN,
				&gtk->data[0], wpa->gtk)) {
				WL_WSEC(("FBT reassoc: GTK decrypt failed\n"));
				return FALSE;
			}

			wpa->gtk_len = gtk->key_len;
			wpa->gtk_index = ltoh16_ua(&gtk->key_info) & 0x3;
			wlc_wpa_plumb_gtk(fbt_bss_priv->wlc, fbt_bss_priv->cfg, wpa->gtk,
				wpa->gtk_len, wpa->gtk_index, wpa->mcipher, gtk->rsc, TRUE);

			wlc_wpa_plumb_tk(fbt_bss_priv->wlc, fbt_bss_priv->cfg,
				(uint8*)wpa->temp_encr_key,
				wpa->tk_len, wpa->ucipher, &PEER_EA(fbt_bss_priv));

			wlc_ioctl(fbt_bss_priv->wlc, WLC_SCB_AUTHORIZE, &PEER_EA(fbt_bss_priv),
				ETHER_ADDR_LEN, fbt_bss_priv->cfg->wlcif);

			return TRUE;
		}
	}

	return FALSE;
}

uint16
wlc_fbt_getlen_eapol(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return 0;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	return (sizeof(wpa_pmkid_list_t) + fbt_bss_priv->ftie_len +	sizeof(dot11_mdid_ie_t));
}

/* Insert IEs in msg2 for 4-way eapol handshake */
void
wlc_fbt_addies(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, eapol_wpa_key_header_t *wpa_key)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	wpa_pmkid_list_t *pmkid;
	wpapsk_t * wpa;

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	wpa = fbt_bss_priv->wpa;

	wpa_key->data[1] += sizeof(wpa_pmkid_list_t);

	pmkid = (wpa_pmkid_list_t *)&wpa_key->data[wpa->sup_wpaie_len];
	pmkid->count.low = 1;
	pmkid->count.high = 0;
	bcopy(fbt_bss_priv->pmkr1name, &pmkid->list[0], WPA2_PMKID_LEN);
	bcopy(fbt_bss_priv->mdie, &pmkid[1], sizeof(dot11_mdid_ie_t));
	bcopy(fbt_bss_priv->ftie, (uint8 *)&pmkid[1] + sizeof(dot11_mdid_ie_t),
		fbt_bss_priv->ftie_len);
}

static bool
wlc_fbt_parse_associe(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	wlc_assoc_t *as = cfg->assoc;
	bcm_tlv_t *mdie, *ftie;
	int ies_len;
	uchar *tlvs;
	uint tlv_len;
	uchar * assoc_ies = (uchar *)&as->resp[1];

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	ies_len = as->resp_len - sizeof(struct dot11_assoc_resp);
	BSS_FBT_INI_FBT(fbt_info, cfg) = FALSE;

	mdie = bcm_parse_tlvs(assoc_ies, ies_len, DOT11_MNG_MDIE_ID);
	if (BCM_TLV_SIZE(mdie) != sizeof(dot11_mdid_ie_t)) {
		return FALSE;
	}
	fbt_bss_priv->mdie_len = mdie->len + TLV_HDR_LEN;
	fbt_bss_priv->mdie = MALLOC(fbt_bss_priv->osh, fbt_bss_priv->mdie_len);
	if (!fbt_bss_priv->mdie) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			fbt_bss_priv->pub->unit, __FUNCTION__,
			MALLOCED(fbt_bss_priv->osh)));
		fbt_bss_priv->mdie_len = 0;
		return FALSE;
	}
	memcpy(fbt_bss_priv->mdie, mdie, fbt_bss_priv->mdie_len);
	fbt_bss_priv->mdid = ltoh16_ua(&mdie->data[0]);
	ftie = bcm_parse_tlvs(assoc_ies, ies_len, DOT11_MNG_FTIE_ID);
	if (BCM_TLV_SIZE(ftie) < sizeof(dot11_ft_ie_t)) {
		return FALSE;
	}
	fbt_bss_priv->ftie_len = ftie->len + TLV_HDR_LEN;
	fbt_bss_priv->ftie = MALLOC(fbt_bss_priv->osh, fbt_bss_priv->ftie_len);
	if (!fbt_bss_priv->ftie) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			fbt_bss_priv->pub->unit, __FUNCTION__,
			MALLOCED(fbt_bss_priv->osh)));
		fbt_bss_priv->ftie_len = 0;
		return FALSE;
	}
	memcpy(fbt_bss_priv->ftie, ftie, fbt_bss_priv->ftie_len);
	tlvs = (uchar *)((uintptr)fbt_bss_priv->ftie + sizeof(dot11_ft_ie_t));
	tlv_len = fbt_bss_priv->ftie_len - sizeof(dot11_ft_ie_t);
	fbt_bss_priv->r1khid = bcm_parse_tlvs(tlvs, tlv_len, DOT11_FBT_SUBELEM_ID_R1KH_ID);
	fbt_bss_priv->r0khid = bcm_parse_tlvs(tlvs, tlv_len, DOT11_FBT_SUBELEM_ID_R0KH_ID);
	if (!fbt_bss_priv->r1khid || !fbt_bss_priv->r0khid) {
		return FALSE;
	}

	BSS_FBT_INI_FBT(fbt_info, cfg) = TRUE;
	return TRUE;
}
#endif /* STA && FBT_STA */

void
wlc_fbt_set_ea(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, struct ether_addr *ea)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = NULL;

	if (fbt_info)
		fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	else
		WL_ERROR(("%s: fbt_info is null\n", __FUNCTION__));

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	bcopy(ea, &fbt_bss_priv->peer_ea, ETHER_ADDR_LEN);
}

static void
wlc_fbt_wpa_free(bss_fbt_priv_t *fbt_bss_priv)
{
	if (fbt_bss_priv->wpa) {
		MFREE(fbt_bss_priv->osh, fbt_bss_priv->wpa, sizeof(*fbt_bss_priv->wpa));
		fbt_bss_priv->wpa = NULL;
	}
	if (fbt_bss_priv->wpa_info) {
		MFREE(fbt_bss_priv->osh, fbt_bss_priv->wpa_info, sizeof(*fbt_bss_priv->wpa_info));
		fbt_bss_priv->wpa_info = NULL;
	}
}

static void
wlc_fbt_bsscfg_updown_callbk(void *ctx, bsscfg_up_down_event_data_t *evt)
{
#ifndef WLFBT_DISABLED
	bss_fbt_info_t *fbt_bsscfg;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	fbt_bsscfg = FBT_BSSCFG_CUBBY(fbt_info, evt->bsscfg);
#endif /* WLFBT_DISABLED */
#ifdef WLFBT
	if (!evt->up) {
#if defined(STA) && defined(FBT_STA)
		wlc_fbt_clear_ies(ctx, evt->bsscfg);
#endif // endif
	}

	else if (BSSCFG_AP(evt->bsscfg) && evt->up) {
		wlc_fbt_bsscfg_init(ctx, evt->bsscfg);
#ifndef WLFBT_DISABLED
		fbt_bsscfg = FBT_BSSCFG_CUBBY(fbt_info, evt->bsscfg);
		fbt_bsscfg->mdid = evt->bsscfg->fbt_mdid;
		strncpy(fbt_bsscfg->fbt_r0kh_id, evt->bsscfg->fbt_r0kh_id,
				evt->bsscfg->fbt_r0kh_id_len);
		fbt_bsscfg->fbt_r0kh_id_len = evt->bsscfg->fbt_r0kh_id_len;
		memcpy(fbt_bsscfg->fbt_r1kh_id, evt->bsscfg->fbt_r1kh_id, ETHER_ADDR_LEN);
		fbt_bsscfg->reassoc_time = evt->bsscfg->fbt_reassoc_time;
		fbt_bsscfg->fbt_over_ds = evt->bsscfg->fbt_over_ds;
#endif /* WLFBT_DISABLED */
	}
#else
	if (!evt->up)
		wlc_fbt_clear_ies(ctx, evt->bsscfg);
#endif /* WLFBT */
}

static bss_fbt_priv_t *
wlc_fbt_bsscfg_cubby_init(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	wlc_fbt_priv_t	*fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	bss_fbt_info_t **pfbt_cfg = FBT_BSSCFG_CUBBY_LOC(fbt_info, cfg);
	bss_fbt_info_t *fbt_bss = NULL;
	bss_fbt_priv_t *fbt_bss_priv = NULL;

	if (!(fbt_bss = (bss_fbt_info_t *)MALLOCZ(fbt_priv->osh, sizeof(bss_fbt_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(fbt_priv), __FUNCTION__, MALLOCED(fbt_priv->osh)));
	}

	fbt_bss_priv = (bss_fbt_priv_t *)((uint8 *)fbt_bss + BSS_PRIV_OFFSET(fbt_info));

	fbt_bss_priv->m_handle = fbt_info;
	fbt_bss_priv->cfg = cfg;
	fbt_bss_priv->wlc = fbt_priv->wlc;
	fbt_bss_priv->osh = fbt_priv->osh;
	fbt_bss_priv->wl = fbt_priv->wl;
	fbt_bss_priv->pub = fbt_priv->pub;

#ifdef WLFBT
#ifdef AP
	fbt_bss->fbt_info = fbt_info;
	fbt_bss->reassoc_time = FBT_REASSOC_TIME_DEF;
	fbt_bss->fbt_timer_set = FALSE;
#endif /* AP */
#endif /* WLFBT */
	*pfbt_cfg = fbt_bss;
	return fbt_bss_priv;
}

static bss_fbt_priv_t *
wlc_fbt_get_bsscfg(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = NULL;

	fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	if (fbt_bss == NULL)
		fbt_bss_priv = wlc_fbt_bsscfg_cubby_init(fbt_info, cfg);
	else
		fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	return fbt_bss_priv;
}

/* Create fbt bsscfg when idsup comes up (sup_wpa=1) and use
 * supplicant wpa/wpa_info elements.
 * If idsup is disabled/not used, create fbt bsscfg at the end of first
 * association and use local fbt wpa/wpa_info elements.
 */
static void
wlc_fbt_bsscfg_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_fbt_priv_t	*fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	bss_fbt_priv_t *fbt_bss_priv = NULL;

	fbt_bss_priv = wlc_fbt_get_bsscfg(fbt_info, cfg);
	if (fbt_bss_priv == NULL)
		goto err;

	/* Use fbt wpa/wpa_info only if idsup is not being used. */
	if (fbt_bss_priv->use_sup_wpa == FALSE) {
		/* Check if wpa already exists as this is called for every JOIN_START */
		if (!fbt_bss_priv->wpa) {
			fbt_bss_priv->wpa = MALLOCZ(fbt_priv->osh, sizeof(wpapsk_t));
			if (fbt_bss_priv->wpa == NULL) {
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				  UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
				goto err;
			}
		}

		/* Check if wpa-info already exists */
		if (!fbt_bss_priv->wpa_info) {
			fbt_bss_priv->wpa_info = MALLOCZ(fbt_priv->osh, sizeof(wpapsk_info_t));
			if (fbt_bss_priv->wpa_info == NULL) {
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				  UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
				goto err;
			}
		}
	}
	return;
err:
	wlc_fbt_bsscfg_deinit(fbt_info, cfg);
}

static
void wlc_fbt_bsscfg_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	bss_fbt_info_t **pfbt_cfg = FBT_BSSCFG_CUBBY_LOC(fbt_info, cfg);

	if (!fbt_bss)
		return;

	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	wlc_fbt_free_ies(fbt_info, cfg);

	if (fbt_bss_priv->use_sup_wpa == FALSE)
		wlc_fbt_wpa_free(fbt_bss_priv);

	MFREE(fbt_bss_priv->osh, fbt_bss, sizeof(bss_fbt_info_t));
	*pfbt_cfg = NULL;
}

#if defined(STA) && defined(FBT_STA)
static void
wlc_fbt_handle_joindone(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	bss_fbt_priv_t *fbt_bss_priv;

	if (!fbt_bss)
		return;

	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	ASSERT(fbt_bss_priv);

	if (cfg->WPA_auth & WPA2_AUTH_FT) {
		if (cfg->auth_atmptd == DOT11_FAST_BSS) {

			if (WLC_FT_PARAM_OVERRIDE_ACTIVE(fbt_info,
					fbt_bss_priv) == FALSE) {

			/* plumbs keys from response frame for FBT-lite mode */
			wlc_fbt_process_reassoc_resp(fbt_info, cfg);
			}
			/* else overrides were enabled, the outside supplicant should
			* process the reassoc response, so no-op
			*/
		} else {
			WL_WSEC(("wl%d: FBT adopt bss \n",
				WLCWLUNIT(fbt_bss_priv->wlc)));

			if (!fbt_bss_priv->use_sup_wpa)
				wlc_fbt_clear_ies(fbt_info, cfg);
			else
				wlc_fbt_free_ies(fbt_info, cfg);

			/* Assoc response does not contain RSN IE. So save the RSN IE from
			 * assoc req for initial association. This is required for FBT-lite
			 * mode where idsup is disabled.
			 */
			wlc_fbt_find_rsnie(fbt_info, cfg);
			/* Parse assoc resp for mdie and ftie for initial fbt association */
			wlc_fbt_parse_associe(fbt_info, cfg);
			wlc_fbt_set_ea(fbt_info, cfg, &cfg->BSSID);

			/* Update beacon RSN IE AKM saved in supplicant for FBT adaptive mode */
			if (cfg->current_bss->wpa2.flags & RSN_FLAGS_FBT) {
				wlc_fbt_upd_authie(fbt_info, cfg);
			}

			if (!fbt_bss_priv->use_sup_wpa) {
				fbt_bss_priv->wpa->WPA_auth = cfg->WPA_auth;
				fbt_bss_priv->wpa_info->wlc = fbt_bss_priv->wlc;
			}
		}
	} else {
		if (WLC_FT_PARAM_OVERRIDE_ACTIVE(fbt_info, fbt_bss_priv)) {

			WL_ASSOC(("wl%d.%d: %s: clearing bsscfg IE overrides\n",
				fbt_info->mod_priv.wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
				__FUNCTION__));
			wlc_fbt_bsscfg_free_overrides(fbt_bss_priv);
		}
	}
}

static void
wlc_fbt_handle_joinproc(void *ctx, bss_assoc_state_data_t *evt_data)
{
	if (evt_data) {
		if (evt_data->state == AS_JOIN_INIT) {
			wlc_fbt_bsscfg_init(ctx, evt_data->cfg);
		} else if (evt_data->state == AS_JOIN_ADOPT) {
			wlc_fbt_handle_joindone(ctx, evt_data->cfg);
		}
	}
}
#endif /* STA && FBT_STA */

static void
wlc_fbt_free_ies(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (fbt_bss_priv->mdie != NULL) {
		MFREE(fbt_bss_priv->osh, fbt_bss_priv->mdie, fbt_bss_priv->mdie_len);
		fbt_bss_priv->mdie = NULL;
		fbt_bss_priv->mdie_len = 0;
	}
	if (fbt_bss_priv->ftie != NULL) {
		MFREE(fbt_bss_priv->osh, fbt_bss_priv->ftie, fbt_bss_priv->ftie_len);
		fbt_bss_priv->ftie = NULL;
		fbt_bss_priv->ftie_len = 0;
	}

	if (WLC_FT_PARAM_OVERRIDE_ACTIVE(fbt_info, fbt_bss_priv)) {
		wlc_fbt_bsscfg_free_overrides(fbt_bss_priv);
	}

	BSS_FBT_INI_FBT(fbt_info, cfg) = FALSE;
}

#if defined(STA) && defined(FBT_STA)
static uint
wlc_fbt_auth_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	uint8 ie_len = 0;

	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	bss_fbt_priv_t *fbt_bss_priv;

	if (!BSSCFG_STA(cfg)) {
		goto exit;
	}

	if (fbt_info == NULL) {
		goto exit;
	}

	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (!BSSCFG_IS_FBT(cfg)) {
		goto exit;
	}

	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	/* see if override is present first */
	if (WLC_FT_PARAM_OVERRIDE_ACTIVE(fbt_info, fbt_bss_priv) &&
			wlc_fbt_calc_overridable_ie_len(ctx, data, WL_FBT_PARAM_TYPE_RSNIE,
			&ie_len)) {

		goto exit;
	}

	/* above overrides may be for auth and reassoc frames, while the below code only
	* runs for select auth frames
	*/
	if ((cfg->WPA_auth & WPA2_AUTH_FT) && (cfg->auth_atmptd == DOT11_FAST_BSS)) {
		ie_len = (fbt_bss_priv->wpa->sup_wpaie_len + sizeof(wpa_pmkid_list_t));
	}
exit:
	return ie_len;
}

/* RSN IE in auth req for fast transition */
static int
wlc_fbt_auth_write_rsn_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_fbt_priv_t *fbt_priv;
	wlc_bsscfg_t *cfg = data->cfg;

	if (!BSSCFG_STA(cfg))
		return BCME_OK;
	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (BSSCFG_IS_FBT(bsscfg) && (cfg->WPA_auth & WPA2_AUTH_FT) &&
		(cfg->auth_atmptd == DOT11_FAST_BSS))
		wlc_fbt_auth_build_rsnie(fbt_info, cfg, data->buf);

	return BCME_OK;
}

static int
wlc_fbt_parse_rsn_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;

	if (!BSSCFG_STA(cfg))
		return BCME_OK;
	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	ASSERT(ftpparm != NULL);

	if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT) &&
		(data->ft == FC_REASSOC_RESP)) {
		/* Save the RSN IE for MIC calculation */
		ftpparm->assocresp.wpa2_ie = data->ie;
	}
	return BCME_OK;
}

static uint
wlc_fbt_calc_md_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	bss_fbt_priv_t *fbt_bss_priv;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_cbparm_t *ftcbparm;
	uint ielen = 0;

	if (!BSSCFG_STA(cfg))
		return 0;
	if (!fbt_info)
		return 0;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
		switch (data->ft) {
		case FC_AUTH:
		case FC_ACTION:
			fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
			if (cfg->auth_atmptd == DOT11_FAST_BSS) {
				ielen = fbt_bss_priv->mdie_len;
			}
			break;
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ: {
			wlc_bss_info_t *bi;
			uint8 *tlvs = NULL;
			uint tlvs_len = 0;
			bcm_tlv_t *md_ie;

			ftcbparm = data->cbparm->ft;
			ASSERT(ftcbparm != NULL);

			bi = ftcbparm->assocreq.target;

			/* find the MD IE */
			if (bi->wpa2.flags & RSN_FLAGS_FBT && bi->bcn_prb != NULL) {
				tlvs = (uint8 *)bi->bcn_prb + sizeof(struct dot11_bcn_prb);
				ASSERT(bi->bcn_prb_len >= sizeof(struct dot11_bcn_prb));
				tlvs_len = bi->bcn_prb_len - sizeof(struct dot11_bcn_prb);

				/* look for an MD IE in saved bcn/prb, and verify the length */
				md_ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_MDIE_ID);
				if (md_ie != NULL &&
				    md_ie->len == DOT11_MDID_IE_DATA_LEN) {

					/* save the pointer to the MD IE contained in
					 * bi->bcn_prb
					 */
					ftcbparm->assocreq.md_ie = (uint8 *)md_ie;
					/* returned length includes the TLV header */
					ielen = TLV_HDR_LEN + md_ie->len;
				}
			}
			break;
		}
		default:
			break;
		}
	}
	return ielen;
}

static int
wlc_fbt_write_md_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_cbparm_t *ftcbparm;

	if (!BSSCFG_STA(cfg))
		return BCME_OK;
	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
		switch (data->ft) {
		case FC_AUTH:
		case FC_ACTION:
			if (cfg->auth_atmptd == DOT11_FAST_BSS) {
				wlc_fbt_auth_build_mdie(fbt_info, cfg, data->buf);
			}
			break;
		case FC_ASSOC_REQ:
		case FC_REASSOC_REQ:
			ftcbparm = data->cbparm->ft;
			ASSERT(ftcbparm != NULL);
			if (ftcbparm->assocreq.target->wpa2.flags & RSN_FLAGS_FBT &&
				(ftcbparm->assocreq.md_ie != NULL))
				bcm_copy_tlv(ftcbparm->assocreq.md_ie, data->buf);
			break;
		default:
			break;
		}
	}
	return BCME_OK;
}

static int
wlc_fbt_parse_md_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;

	if (!BSSCFG_STA(cfg))
		return BCME_OK;
	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	ASSERT(ftpparm != NULL);

	if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
		switch (data->ft) {
		case FC_AUTH:
		case FC_ACTION:
			if (cfg->auth_atmptd == DOT11_FAST_BSS) {
				if (!wlc_fbt_auth_parse_mdie(fbt_info, cfg, data->ie,
						data->ie_len)) {
					ftpparm->auth.status = DOT11_SC_FAILURE;
					return BCME_ERROR;
				}
			}
			break;
		case FC_ASSOC_RESP:
			/* FBT-lite mode: pulls FTIE & MDIE from assocresp */
			if (!wlc_fbt_auth_parse_mdie(fbt_info, cfg, data->ie, data->ie_len)) {
				return BCME_ERROR;
			}
			break;
		case FC_REASSOC_RESP:
			ftpparm->assocresp.md_ie = data->ie;
		default:
			break;
		}
	}
	return BCME_OK;
}

/* Parse MD IE from the beacon/probe resp */
static int
wlc_fbt_scan_parse_md_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;
	wlc_info_t *wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (!BSSCFG_IS_FBT(cfg))
		return BCME_OK;

	if (data->ie != NULL) {
		dot11_mdid_ie_t * mdie = (dot11_mdid_ie_t *)data->ie;

		if (BCM_TLV_SIZE(mdie) != sizeof(dot11_mdid_ie_t)) {
			return BCME_BADLEN;
		}

		bi->mdid = ltoh16_ua(&mdie->mdid);
		bi->flags |= WLC_BSS_FBT;
		if ((mdie->cap & FBT_MDID_CAP_OVERDS) &&
			(cfg->flags & WLC_BSSCFG_ALLOW_FTOVERDS)) {
			bi->flags2 |= WLC_BSS_OVERDS_FBT;
		}
	}
	return BCME_OK;
}

/* Parse CCX Extended capability IE from the beacon/probe resp */
static int
wlc_fbt_scan_parse_ccx_ext_cap_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	ccx_ext_cap_ie_t *ccx_ext_cap_ie = (ccx_ext_cap_ie_t *)data->ie;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	wlc_bss_info_t *bi = ftpparm->scan.result;
	wlc_info_t *wlc;
	wlc_bsscfg_t *cfg = data->cfg;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	/* If FBT AKM suites already present, don't process
	 * CCX Ext Cap IE.
	 */
	if (!BSSCFG_IS_FBT(cfg) || (bi->wpa2.flags & RSN_FLAGS_FBT))
		return BCME_OK;

	if (ccx_ext_cap_ie == NULL)
		return BCME_OK;

	/* CCX Ext cap IE is present and FBT AKM suites not enabled,
	 * decide FBT capability based on FBT support bit in this IE.
	 * bit6 = 0, target bss does not support FBT, no action required.
	 * bit6 = 1, target bss supports FBT, set RSN_FLAGS_FBT flag.
	 */
	if (BCM_TLV_SIZE(ccx_ext_cap_ie) != sizeof(ccx_ext_cap_ie_t)) {
		return BCME_BADLEN;
	}
	if (ccx_ext_cap_ie->cap & CCX_CAP_FBT) {
		WL_INFORM(("wl%d: FBT support enabled in CCX Cap IE\n", wlc->pub->unit));
		bi->wpa2.flags |= RSN_FLAGS_FBT;
	}
	return BCME_OK;
}

static uint
wlc_fbt_calc_ft_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_cbparm_t *ftcbparm;
	bss_fbt_priv_t *fbt_bss_priv;
	wlc_info_t *wlc;
	uint ielen = 0;

	if (!BSSCFG_STA(cfg))
		return 0;
	if (!fbt_info)
		return 0;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
		switch (data->ft) {
		case FC_AUTH:
		case FC_ACTION:
			if (cfg->auth_atmptd == DOT11_FAST_BSS)
				ielen = TLV_HDR_LEN + sizeof(dot11_ft_ie_t) +
					fbt_bss_priv->r0khid->len;
			break;
		case FC_REASSOC_REQ:
			ftcbparm = data->cbparm->ft;
			ASSERT(ftcbparm != NULL);
			/* Calculate the length of FT IE and the length of RIC IEs
			 * using RIC registry.
			 */
			if ((ftcbparm->assocreq.target->wpa2.flags & RSN_FLAGS_FBT) &&
			    ftcbparm->assocreq.md_ie != NULL) {
				ielen = fbt_bss_priv->ftie_len +
				  wlc_cac_calc_ric_len(wlc->cac, cfg);
			}
			break;
		default:
			break;
		}
	}
	return ielen;
}

static int
wlc_fbt_write_ft_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	bss_fbt_priv_t *fbt_bss_priv;
	wlc_info_t *wlc;

	if (!BSSCFG_STA(cfg))
		return BCME_OK;
	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	wlc = fbt_priv->wlc;

	if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
		switch (data->ft) {
		case FC_AUTH:
		case FC_ACTION:
			if (cfg->auth_atmptd == DOT11_FAST_BSS)
				wlc_fbt_auth_build_ftie(fbt_info, cfg, data->buf);
			break;
		case FC_REASSOC_REQ: {
			uint ricies_len;
			uint8 *ricies = NULL;
			int ricie_count = 0;
			wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
			ASSERT(ftcbparm != NULL);

			if ((ftcbparm->assocreq.target->wpa2.flags & RSN_FLAGS_FBT) &&
			    ftcbparm->assocreq.wpa2_ie != NULL &&
			    ftcbparm->assocreq.md_ie != NULL) {
				/* Build the IEs in RIC request using RIC registry first
				 * and then build the FT IE as the RIC IEs have to be
				 * included for calculating the MIC which is part of FT IE.
				 */
				ricies_len = wlc_cac_calc_ric_len(wlc->cac, cfg);
				/* Point to the start of RIC buffer in reassoc req frame */
				if (ricies_len)
					ricies = data->buf + fbt_bss_priv->ftie_len;

				/* Build RIC IEs */
				wlc_cac_write_ric(wlc->cac, cfg, ricies, &ricie_count);

				/* Calculate MIC and generate FT IE */
				wlc_fbt_reassoc_build_ftie(fbt_info, cfg, data->buf,
					data->buf_len, (bcm_tlv_t *)ftcbparm->assocreq.md_ie,
					(bcm_tlv_t *)ftcbparm->assocreq.wpa2_ie,
					ricies, ricies_len, ricie_count);
			}
			break;
		}
		default:
			break;
		}
	}
	return BCME_OK;
}

static int
wlc_fbt_parse_ft_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;

	if (!BSSCFG_STA(cfg))
		return BCME_OK;
	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	ASSERT(ftpparm != NULL);

	if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
		switch (data->ft) {
		case FC_AUTH:
		case FC_ACTION:
			if (cfg->auth_atmptd == DOT11_FAST_BSS) {
				if (!wlc_fbt_auth_parse_ftie(fbt_info, cfg, data->ie, data->ie_len))
					ftpparm->auth.status = DOT11_SC_FAILURE;
			}
			break;
		case FC_ASSOC_RESP:
			/* FBT-lite mode: pull FTIE & MDIE from assocresp */
			if (!wlc_fbt_auth_parse_ftie(fbt_info, cfg, data->ie, data->ie_len)) {
				ftpparm->assocresp.status = DOT11_SC_FAILURE;
			}
			break;
		case FC_REASSOC_RESP: {
			uint8 *tlvs;
			uint tlvs_len;
			wlc_assoc_t *as = cfg->assoc;
			uint8 *ricie = NULL;

			/* Check if RDE IE is present in reassoc response */
			tlvs = (uint8 *)&as->resp[1];
			tlvs_len = as->resp_len - DOT11_ASSOC_RESP_FIXED_LEN;
			ricie = (uint8*)bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RDE_ID);
			if (ricie != NULL) {
				/* RDE IE is present, save FT IE to do MIC calculation
				 * while processing RDE IE.
				 */
				ftpparm->assocresp.ft_ie = data->ie;
			}
			else {
				/* RDE IE is absent. Do the MIC calculation here without RDE */
#ifdef WLFBT
				if (!wlc_fbt_reassoc_validate_ftie(fbt_info, cfg, NULL, NULL, 0,
					(dot11_ft_ie_t *)data->ie,
					(bcm_tlv_t *)ftpparm->assocresp.md_ie,
					(bcm_tlv_t *)ftpparm->assocresp.wpa2_ie,
					FT_MIC_REASSOC_RESPONSE_TSN)) {
#else
				if (!wlc_fbt_reassoc_validate_ftie(fbt_info, cfg, NULL, 0,
					(dot11_ft_ie_t *)data->ie,
					(bcm_tlv_t *)ftpparm->assocresp.md_ie,
					(bcm_tlv_t *)ftpparm->assocresp.wpa2_ie)) {
#endif /* WLFBT */
					WL_ERROR(("wl%d: %s failed, ignoring reassoc response\n",
					   UNIT(fbt_priv), __FUNCTION__));
					return BCME_ERROR;
				}
			}
			/* Update FTIE to handle M1 message received without FT
			 * initial association during unicast key rotation.
			 */
			if (!wlc_fbt_update_ftie(fbt_info, cfg))
					WL_ERROR(("wl%d: %s: FTIE update failed \n",
						UNIT(fbt_priv), __FUNCTION__));
			break;
		}
	    default:
			break;
		}
	}
	return BCME_OK;
}

static uint
wlc_fbt_calc_rde_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	uint len = 0;

	if (!fbt_info)
		return 0;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (BSSCFG_IS_FBT(data->cfg))
		len = DOT11_MNG_RDE_IE_LEN;
	return len;
}

/* Write RDE IE in reassoc req */
static int
wlc_fbt_write_rde_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_bsscfg_t *cfg = data->cfg;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (BSSCFG_IS_FBT(cfg)) {
		dot11_rde_ie_t* rdptr;
		ftcbparm->fbtric.rde_count += 1;
		/* RDE. */
		rdptr = (dot11_rde_ie_t*)data->buf;
		rdptr->id = DOT11_MNG_RDE_ID;
		rdptr->length = DOT11_MNG_RDE_IE_LEN - TLV_HDR_LEN;
		rdptr->rde_id = ftcbparm->fbtric.rde_count;
		rdptr->rd_count = 1;
#ifdef WLFBT
		if (BSSCFG_STA(cfg))
			rdptr->status = htol16(0);	/* Always 0 for STA. */
		else {
			rdptr->status = htol16(ftcbparm->fbtric.status);
			rdptr->rde_id = ftcbparm->fbtric.rde_id;
		}
#else
		rdptr->status = htol16(0);	/* Always 0 for STA. */
#endif /* WLFBT */
	}
	return BCME_OK;
}

static int
wlc_fbt_parse_rde_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_fbt_priv_t *fbt_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_info_t *wlc;

#ifdef WLFBT
	if (!BSSCFG_STA(cfg))
		return BCME_OK;
#endif /* WLFBT */
	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	wlc = fbt_priv->wlc;

	if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT) &&
		(data->ft == FC_REASSOC_RESP)) {
		bool accepted;
		wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
		ASSERT(ftpparm != NULL);

		/* Do the MIC calculation to validate FT IE when RDE IE is present */
#ifdef WLFBT
		if (wlc_fbt_reassoc_validate_ftie(fbt_info, cfg, NULL, data->ie,
			data->ie_len, (dot11_ft_ie_t *)ftpparm->assocresp.ft_ie,
			(bcm_tlv_t *)ftpparm->assocresp.md_ie,
			(bcm_tlv_t *)ftpparm->assocresp.wpa2_ie,
			FT_MIC_REASSOC_RESPONSE_TSN)) {
#else
		if (wlc_fbt_reassoc_validate_ftie(fbt_info, cfg, data->ie,
			data->ie_len, (dot11_ft_ie_t *)ftpparm->assocresp.ft_ie,
			(bcm_tlv_t *)ftpparm->assocresp.md_ie,
			(bcm_tlv_t *)ftpparm->assocresp.wpa2_ie)) {
#endif /* WLFBT */
			/* Process any RIC responses. */
			if (CAC_ENAB(wlc->pub)) {
				accepted = wlc_fbt_reassoc_parse_rdeie(fbt_info,
				        cfg, data->ie, data->ie_len);
				if (accepted == FALSE) {
					WL_ERROR(("wl%d: %s: RIC request denied\n",
					   wlc->pub->unit, __FUNCTION__));
					ftpparm->assocresp.status = DOT11_SC_FAILURE;
				}
			}
		} else {
			WL_ERROR(("wl%d: %s failed, ignoring reassoc response.\n",
			   wlc->pub->unit, __FUNCTION__));
			return BCME_ERROR;
		}
	}
	return BCME_OK;
}

/* Over-the-DS FBT Request Action frame */
void *
wlc_fbt_send_overds_req(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, struct ether_addr *ea,
	struct scb *scb, bool short_preamble)
{
	wlc_info_t *wlc;
	wlc_txq_info_t *qi;
	void *p = NULL;
	uint8 *pbody;
	dot11_ft_req_t *ft_req;
	uint16 fc;
	int body_len;
	wlc_bsscfg_t *parent;
	wlc_fbt_priv_t *fbt_priv;

	if (scb) {
	  parent = SCB_BSSCFG(scb);
	}
	else {
	  ASSERT(0);   /* Force debug images to crash to assist debugging */
	  return NULL; /* Error handing for release images */
	}

	if (!fbt_info)
		return NULL;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	ASSERT(cfg != NULL);

	/* assert:  (status == success) -> (scb is not NULL) */
	ASSERT(scb != NULL);

	/* calc IEs' length */
	body_len = DOT11_FT_REQ_FIXED_LEN;

	/* get a packet */
	fc = FC_ACTION;
	body_len += wlc_ier_calc_len(wlc->ier_fbt, parent, FC_ACTION, NULL);

	if ((p = wlc_frame_get_mgmt(wlc, fc, &cfg->BSSID, &cfg->cur_etheraddr,
	                            &cfg->BSSID, body_len, &pbody)) == NULL) {
		WL_ERROR(("wl%d: %s: wlc_frame_get_mgmt failed\n",
		          wlc->pub->unit, __FUNCTION__));
#ifdef STA
		/* XXX PR84212: p can only be NULL if packet alloc failed, give
		 * up now rather calling wlc_auth_tx_complete() to simulate frame
		 * wlc_auth_tx_complete(wlc, TX_STATUS_NO_ACK, (void *)(uintptr)cfg->ID);
		 */
#endif /* STA */
		return NULL;
	}

	ft_req = (dot11_ft_req_t*)pbody;
	ft_req->category = DOT11_ACTION_CAT_FBT;
	ft_req->action = DOT11_FT_ACTION_FT_REQ;
	bcopy(&cfg->cur_etheraddr, ft_req->sta_addr, ETHER_ADDR_LEN);
	bcopy(&cfg->target_bss->BSSID, ft_req->tgt_ap_addr, ETHER_ADDR_LEN);

	pbody += DOT11_FT_REQ_FIXED_LEN;
	body_len -= DOT11_FT_REQ_FIXED_LEN;

	/* build IEs */
	if (wlc_ier_build_frame(wlc->ier_fbt, parent, FC_ACTION, NULL,
	                        pbody, body_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_build_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		PKTFREE(wlc->osh, p, TRUE);
		return NULL;
	}

	/*
	  Send using the bsscfg queue so that the FT Req will go out on the current
	  associated channel, not the roam target channel.
	 */
	qi = cfg->wlcif->qi;

	if (wlc_queue_80211_frag(wlc, p, qi, scb, scb->bsscfg,
		short_preamble, NULL, WLC_LOWEST_SCB_RSPEC(scb)))
		return p;

	return NULL;
}
#endif /* STA && FBT_STA */

#if defined(STA) && defined(FBT_STA)
static void
wlc_fbt_auth_nhdlr_cb(void *ctx, wlc_iem_nhdlr_data_t *data)
{
	if (WL_INFORM_ON()) {
		printf("%s: no parser\n", __FUNCTION__);
		prhex("IE", data->ie, data->ie_len);
	}
}

static uint8
wlc_fbt_auth_vsie_cb(void *ctx, wlc_iem_pvsie_data_t *data)
{
	wlc_info_t *wlc = (wlc_info_t *)ctx;

	return wlc_iem_vs_get_id(wlc->iemi, data->ie);
}

/* FT Response Action frame */
void
wlc_fbt_recv_overds_resp(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg,
     struct dot11_management_header *hdr, uint8 *body, uint body_len)
{
	wlc_info_t *wlc;
	wlc_assoc_t *as = cfg->assoc;
	wlc_bss_info_t *target_bss = cfg->target_bss;
	uint16 resp_status;
#ifdef BCMDBG_ERR
	char eabuf[ETHER_ADDR_STR_LEN], *sa = bcm_ether_ntoa(&hdr->sa, eabuf);
#endif /* BCMDBG_ERR */
	dot11_ft_res_t* ft_resp = (dot11_ft_res_t*)body;
	struct scb *scb;
	bool ft_band_changed = TRUE;
	wlc_iem_upp_t upp;
	wlc_iem_ft_pparm_t ftpparm;
	wlc_iem_pparm_t pparm;
	wlc_fbt_priv_t *fbt_priv;

	if (!fbt_info)
		return;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	WL_TRACE(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	/* Is this request for an AP config or a STA config? */
	if (!BSSCFG_STA(cfg) || (!cfg->BSS)) {
		WL_ASSOC(("wl%d: %s: FC_ACTION FT Response: unknown bsscfg _ap %d"
			" bsscfg->BSS %d\n", WLCWLUNIT(wlc), __FUNCTION__,
			cfg->_ap, cfg->BSS));
		return;  /* We couldn't match the incoming frame to a BSS config */
	}

	/* ignore ft_resp frames from other stations */
	if ((ft_resp->action != DOT11_FT_ACTION_FT_RES) ||
		(as->state != AS_SENT_FTREQ) ||
		bcmp((char*)&hdr->sa, (char*)&cfg->current_bss->BSSID, ETHER_ADDR_LEN) ||
		bcmp((char*)&ft_resp->tgt_ap_addr, (char*)&target_bss->BSSID, ETHER_ADDR_LEN) ||
		!wlc_scbfind(wlc, cfg, &cfg->current_bss->BSSID) ||
		!(scb = wlc_scbfind(wlc, cfg, &target_bss->BSSID))) {
		WL_ERROR(("wl%d.%d: unsolicited FT response from %s",
		          wlc->pub->unit, WLC_BSSCFG_IDX(cfg), sa));
		WL_ERROR((" for  %s\n", bcm_ether_ntoa((struct ether_addr *)&ft_resp->tgt_ap_addr,
			eabuf)));

		wlc_auth_complete(cfg, WLC_E_STATUS_UNSOLICITED, &target_bss->BSSID, 0, 0);
		return;
	}

	resp_status = ltoh16(ft_resp->status);

	/* authentication error */
	if (resp_status != DOT11_SC_SUCCESS) {
		wlc_auth_complete(cfg, WLC_E_STATUS_FAIL, &target_bss->BSSID, resp_status,
			DOT11_FAST_BSS);
		return;
	}

	body += DOT11_FT_RES_FIXED_LEN;
	body_len -= DOT11_FT_RES_FIXED_LEN;

	/* prepare IE mgmt calls */
	bzero(&upp, sizeof(upp));
	upp.notif_fn = wlc_fbt_auth_nhdlr_cb;
	upp.vsie_fn = wlc_fbt_auth_vsie_cb;
	upp.ctx = wlc;
	bzero(&ftpparm, sizeof(ftpparm));
	ftpparm.auth.status = (uint16)resp_status;
	bzero(&pparm, sizeof(pparm));
	pparm.ft = &ftpparm;

	/* parse IEs */
	if (wlc_ier_parse_frame(wlc->ier_fbt, cfg, FC_ACTION, &upp, &pparm,
	                        body, body_len) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_ier_parse_frame failed\n",
		          wlc->pub->unit, __FUNCTION__));
		return;
	}
	resp_status = ftpparm.auth.status;

	if (resp_status != DOT11_SC_SUCCESS) {
		wlc_auth_complete(cfg, WLC_E_STATUS_FAIL, &target_bss->BSSID, resp_status,
			DOT11_FAST_BSS);
		return;
	}

	{
		/* authentication success */
		wlcband_t *band;
		chanspec_t chanspec = target_bss->chanspec;
		bool switch_scb_band;

		/* Set switch_scb_band before wlc_set_chanspec has a chance to change
		 * wlc->band->bandunit.
		 */
		switch_scb_band = wlc->band->bandunit != CHSPEC_WLCBANDUNIT(chanspec);

		band = wlc->bandstate[CHSPEC_IS2G(chanspec) ? BAND_2G_INDEX : BAND_5G_INDEX];

		/* Sanitize user setting for 8080MHz against current settings
		 * Reduce an 8080MHz chanspec to 80MHz if needed.
		 */
		if (CHSPEC_IS8080_UNCOND(chanspec) &&
		    (!VHT_ENAB_BAND(wlc->pub, band->bandtype) ||
		     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_160MHZ) ||
		     !WL_BW_CAP_160MHZ(band->bw_cap))) {
			/* select the 80MHz primary channel in case 80 is allowed */
			chanspec = wf_chspec_primary80_chspec(chanspec);
		}

		/* Sanitize user setting for 160MHz against current settings
		 * Reduce an 160MHz chanspec to 80MHz if needed.
		 */
		if (CHSPEC_IS160_UNCOND(chanspec) &&
		    (!VHT_ENAB_BAND(wlc->pub, band->bandtype) ||
		     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_160MHZ) ||
		     !WL_BW_CAP_160MHZ(band->bw_cap))) {
			/* select the 80MHz primary channel in case 80 is allowed */
			chanspec = wf_chspec_primary80_chspec(chanspec);
		}

		/* Sanitize user setting for 80MHz against current settings
		 * Reduce an 80MHz chanspec to 40MHz if needed.
		 */
		if (CHSPEC_IS80_UNCOND(chanspec) &&
		    (!VHT_ENAB_BAND(wlc->pub, band->bandtype) ||
		     (wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) & WLC_NO_80MHZ) ||
		     !WL_BW_CAP_80MHZ(band->bw_cap))) {
			/* select the 40MHz primary channel in case 40 is allowed */
			chanspec = wf_chspec_primary40_chspec(chanspec);
		}

		/* Check if we're going to change bands */
		if (CHSPEC_BAND(cfg->current_bss->chanspec) == CHSPEC_BAND(chanspec))
			ft_band_changed = FALSE;

		/* Convert a 40MHz AP channel to a 20MHz channel if we are not in NMODE or
		 * the locale does not allow 40MHz
		 * or the band is not configured for 40MHz operation
		 * Note that the unconditional version of the CHSPEC_IS40 is used so that
		 * code compiled without MIMO support will still recognize and convert
		 * a 40MHz chanspec.
		 */
		if (CHSPEC_IS40_UNCOND(chanspec) &&
			(!N_ENAB(wlc->pub) ||
			(wlc_channel_locale_flags_in_band(wlc->cmi, band->bandunit) &
			WLC_NO_40MHZ) ||
		         !WL_BW_CAP_40MHZ(band->bw_cap) ||
			(BAND_2G(band->bandtype) && WLC_INTOL40_DET(wlc, cfg)))) {
			uint channel;
			channel = wf_chspec_ctlchan(chanspec);
			chanspec = CH20MHZ_CHSPEC(channel);
		}

		/* Change the radio channel to match the target_bss */
		if ((WLC_BAND_PI_RADIO_CHANSPEC != chanspec)) {
			/* clear the quiet bit on the dest channel */
			wlc_clr_quiet_chanspec(wlc->cmi, chanspec);
			WL_CHANSW(("time=%uus old=%d new=%d reason=%d dwelltime=%dms",
			           R_REG(wlc->osh, &wlc->regs->tsf_timerlow),
			           WLC_BAND_PI_RADIO_CHANSPEC, chanspec,
			           CHANSW_FBT, -1));

			wlc_suspend_mac_and_wait(wlc);
			wlc_set_chanspec(wlc, chanspec);
#ifdef WLMCHAN
			if (MCHAN_ENAB(wlc->pub)) {
				wlc_mchan_set_priq(wlc->mchan, cfg);
			}
#endif // endif
			wlc_enable_mac(wlc);
		}

		wlc_rate_lookup_init(wlc, &target_bss->rateset);

		{wlc_phy_t *pi = WLC_PI(wlc);
		if (WLCISACPHY(wlc->band)) {
			wlc_full_phy_cal(wlc, cfg, PHY_PERICAL_JOIN_BSS);
			wlc_phy_interference_set(pi, TRUE);
			wlc_phy_acimode_noisemode_reset(pi,
				chanspec, FALSE, TRUE, FALSE);
		}
		else if ((WLCISNPHY(wlc->band) || WLCISHTPHY(wlc->band)) &&
			ft_band_changed) {
			wlc_full_phy_cal(wlc, cfg, PHY_PERICAL_JOIN_BSS);
			wlc_phy_interference_set(pi, TRUE);
			wlc_phy_acimode_noisemode_reset(pi,
				chanspec, FALSE, TRUE, FALSE);
		}}
		/* The scb for target_bss was created in wlc_join_BSS on the channel for the
		 * current_bss. We may need to switch the target_bss scb to the new band if we've
		 * successfully performed an FT over-the-DS reassoc.
		 */
		if (switch_scb_band) {
			wlc_scb_switch_band(wlc, scb, wlc->band->bandunit, cfg);
			wlc_rateset_filter(&wlc->band->hw_rateset /* src */,
				&scb->rateset /* dst */, FALSE,
				WLC_RATES_CCK_OFDM, RATE_MASK, BSS_N_ENAB(wlc, scb->bsscfg));
			wlc_scb_ratesel_init(wlc, scb);
		}
	}
	wlc_auth_complete(cfg, WLC_E_STATUS_SUCCESS, &target_bss->BSSID, resp_status,
		DOT11_FAST_BSS);

}

/* Save the akm type being used for the current association */
void
wlc_fbt_save_current_akm(wlc_fbt_info_t *fbt_info, const wlc_bsscfg_t *cfg,
	const wlc_bss_info_t *bi)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (cfg->WPA_auth & WPA2_AUTH_UNSPECIFIED) {
		if ((cfg->WPA_auth & WPA2_AUTH_FT) && (bi->wpa2.flags & RSN_FLAGS_FBT))
			fbt_bss_priv->current_akm_type = RSN_AKM_FBT_1X;
		else
			fbt_bss_priv->current_akm_type = RSN_AKM_UNSPECIFIED;
	}
	else if (cfg->WPA_auth & WPA2_AUTH_PSK) {
		if ((cfg->WPA_auth & WPA2_AUTH_FT) && (bi->wpa2.flags & RSN_FLAGS_FBT))
			fbt_bss_priv->current_akm_type = RSN_AKM_FBT_PSK;
		 else
			fbt_bss_priv->current_akm_type = RSN_AKM_PSK;
	}
}

/* Reset the current state of the variable */
void
wlc_fbt_reset_current_akm(wlc_fbt_info_t *fbt_info, const wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!(cfg->WPA_auth & WPA2_AUTH_FT))
		return;

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (fbt_bss_priv)
		fbt_bss_priv->current_akm_type = RSN_AKM_NONE;

	return;
}
#endif /* STA && FBT_STA */

#if defined(STA) && defined(FBT_STA)
/* Fast transition allowed only if the target AP supports the same FT AKM
 * suites as the currently associated AP.
 */
bool
wlc_fbt_is_fast_reassoc(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg, wlc_bss_info_t *bi)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);

	if (!(cfg->WPA_auth & WPA2_AUTH_FT))
		return FALSE;

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	if (!wlc_fbt_is_cur_mdid(fbt_info, cfg, bi))
		return FALSE;

	if (((cfg->WPA_auth & WPA2_AUTH_UNSPECIFIED) &&
		(fbt_bss_priv->current_akm_type == RSN_AKM_FBT_1X)) ||
		((cfg->WPA_auth & WPA2_AUTH_PSK) &&
		(fbt_bss_priv->current_akm_type == RSN_AKM_FBT_PSK))) {
		WL_WSEC(("wl%d: JOIN: Fast bss transition\n", WLCWLUNIT(fbt_bss_priv->wlc)));
		return TRUE;
	}
	else
		return FALSE;
}

/* Check if the AKM selected for the current bss is supported by the target AP */
bool
wlc_fbt_akm_match(wlc_fbt_info_t *fbt_info, const wlc_bsscfg_t *cfg, const wlc_bss_info_t *bi)
{
	wlc_fbt_priv_t *fbt_priv;
	bool akm_match = FALSE;
	int i;

	if (!fbt_info)
		return FALSE;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (bcmwpa_is_rsn_auth(cfg->WPA_auth) && (bi->wpa2.flags & RSN_FLAGS_SUPPORTED)) {
		if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT)) {
			for (i = 0; i < bi->wpa2.acount && (akm_match == FALSE); i++) {
				if (((bi->wpa2.auth[i] == RSN_AKM_FBT_1X) &&
					(cfg->WPA_auth & WPA2_AUTH_UNSPECIFIED)) ||
					((bi->wpa2.auth[i] == RSN_AKM_FBT_PSK) &&
					(cfg->WPA_auth & WPA2_AUTH_PSK))) {
					WL_ASSOC(("wl%d: JOIN: FBT AKM match\n",
						WLCWLUNIT(fbt_priv->wlc)));
					akm_match = TRUE;
				}
			}
		}
		else if (!(cfg->WPA_auth & WPA2_AUTH_FT)) {
			for (i = 0; i < bi->wpa2.acount && (akm_match == FALSE); i++) {
				if (((bi->wpa2.auth[i] == RSN_AKM_UNSPECIFIED) &&
					(cfg->WPA_auth & WPA2_AUTH_UNSPECIFIED)) ||
					((bi->wpa2.auth[i] == RSN_AKM_PSK) &&
					(cfg->WPA_auth & WPA2_AUTH_PSK))) {
					WL_ASSOC(("wl%d: JOIN: WPA AKM match\n",
						WLCWLUNIT(fbt_priv->wlc)));
					akm_match = TRUE;
				}
#ifdef BCMCCX
				else if (CCX_ENAB(fbt_priv->wlc->pub) &&
					(cfg->WPA_auth & WPA2_AUTH_CCKM) &&
					((bi->wpa2.auth[i] == RSN_AKM_NONE) ||
					(bi->wpa2.auth[i] == RSN_AKM_UNSPECIFIED))) {
					WL_ASSOC(("wl%d: JOIN: CCKM AKM match\n",
						WLCWLUNIT(fbt_priv->wlc)));
					akm_match = TRUE;
				}
#endif /* BCMCCX */
			}
		}
	}
	return akm_match;
}
#endif /* STA && FBT_STA */

#ifdef BCMSUP_PSK
static
void wlc_fbt_sup_updown_callbk(void *ctx, sup_init_event_data_t *evt)
{
	if (evt->up)
		wlc_fbt_sup_init(ctx, evt);
	else
		wlc_fbt_sup_deinit(ctx, evt->bsscfg);
}

static
void wlc_fbt_sup_init(void *ctx, sup_init_event_data_t *evt)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	bss_fbt_priv_t *fbt_bss_priv = NULL;

	fbt_bss_priv = wlc_fbt_get_bsscfg(fbt_info, evt->bsscfg);
	if (fbt_bss_priv) {
		/* If sup comes up, free up FBT local wpa & wpa_info elements. */
		if (fbt_bss_priv->use_sup_wpa == FALSE) {
			wlc_fbt_wpa_free(fbt_bss_priv);
			fbt_bss_priv->use_sup_wpa = TRUE;
		}
		fbt_bss_priv->wpa = evt->wpa;
		fbt_bss_priv->wpa_info = evt->wpa_info;
	} else {
		wlc_fbt_sup_deinit(fbt_info, evt->bsscfg);
	}
}

static
void wlc_fbt_sup_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	bss_fbt_priv_t *fbt_bss_priv;

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	if (fbt_bss_priv->use_sup_wpa == TRUE) {
		/* free wpapsk elements before removing reference to wpa/wpa_info
		 */
		wlc_wpapsk_free(fbt_bss_priv->wlc, fbt_bss_priv->wpa);
		fbt_bss_priv->wpa_info->pmk_len = 0;

		/* wpa/wpa_info are owned by sup module so only remove fbt reference to
		 * them.
		 */
		fbt_bss_priv->wpa = NULL;
		fbt_bss_priv->wpa_info = NULL;
		fbt_bss_priv->use_sup_wpa = FALSE;
	}
}
#endif /* BCMSUP_PSK */

#if defined(STA) && defined(FBT_STA)
int
wlc_fbt_set_pmk(wlc_fbt_info_t *fbt_info, struct wlc_bsscfg *cfg, wsec_pmk_t *pmk, bool assoc)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss;

	if (fbt_info == NULL || cfg == NULL || pmk == NULL) {
		WL_ERROR(("%s: missing fbt_info or bsscfg or pmk\n", __FUNCTION__));
		return BCME_BADARG;
	}

	fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	if (fbt_bss == NULL) {
		WL_ERROR(("%s: fbt_bss cubby not set\n", __FUNCTION__));
		return BCME_BADARG;
	}

	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	ASSERT(fbt_bss_priv);

	return wlc_set_pmk(fbt_bss_priv->wlc, fbt_bss_priv->wpa_info, fbt_bss_priv->wpa, cfg, pmk,
		assoc);
}

void
wlc_fbt_get_kck_kek(wlc_fbt_info_t *fbt_info, struct wlc_bsscfg *cfg, uint8 *key)
{
	bss_fbt_priv_t *fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	ASSERT(fbt_bss_priv && fbt_bss_priv->wpa && key);
	memcpy(key, fbt_bss_priv->wpa->eapol_mic_key, WPA_MIC_KEY_LEN);
	memcpy(key + WPA_MIC_KEY_LEN, fbt_bss_priv->wpa->eapol_encr_key, WPA_ENCR_KEY_LEN);
}
#endif /* STA && FBT_STA */

bool
wlc_fbt_enabled(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_info_t *fbt_cfg;

	fbt_cfg = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	if (fbt_cfg == NULL) {
		return FALSE;
	}

#ifdef AP
	if (BSSCFG_AP(cfg)) {
		return BSSCFG_IS_FBT(cfg);
	}
#endif /* AP */

	return TRUE;
}

#ifdef AP
bool
wlc_fbt_fbtoverds_flag(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	wlc_fbt_info_t *fbt_info = wlc->fbt;
	bss_fbt_info_t *fbt_bsscfg;

	fbt_bsscfg = FBT_BSSCFG_CUBBY(fbt_info, bsscfg);
	if (fbt_bsscfg == NULL) {
		return FALSE;
	}

	return fbt_bsscfg->fbt_over_ds != 0;
}

#ifndef WLFBT_DISABLED
bool
wlc_fbt_fbtoverds_enable(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, bool fbt_over_ds)
{

	if (!bsscfg) {
		return FALSE;
	}

	if (fbt_over_ds == bsscfg->fbt_over_ds)
		return TRUE;
	bsscfg->fbt_over_ds = fbt_over_ds;
	return TRUE;
}
#endif /* WLFBT_DISABLED */
static int
wlc_fbt_scb_init(void *context, struct scb *scb)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)context;
	wlc_fbt_priv_t	*fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc_info_t *wlc = fbt_priv->wlc;
	wlc_bsscfg_t *bsscfg = scb->bsscfg;
	wlc_fbt_scb_cubby_t *fbt_scb_cubby;
	wlc_fbt_scb_t *fbt_scb;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, bsscfg)) {

		fbt_scb = (wlc_fbt_scb_t *)MALLOCZ(fbt_priv->osh, sizeof(wlc_fbt_scb_t));
		if (!fbt_scb)
		{
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(fbt_priv), __FUNCTION__, MALLOCED(fbt_priv->osh)));
			return BCME_NOMEM;
		}
		fbt_scb_cubby = FBT_SCB_CUBBY(fbt_info, scb);
		fbt_scb_cubby->cubby = fbt_scb;
		memset(fbt_scb, 0, sizeof(wlc_fbt_scb_cubby_t));
		memcpy(fbt_scb->macaddr.octet, scb->ea.octet, ETHER_ADDR_LEN);
	}

	return BCME_OK;
}

static void
wlc_fbt_scb_deinit(void *context, struct scb *scb)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)context;
	wlc_fbt_priv_t	*fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc_info_t *wlc = fbt_priv->wlc;
	wlc_fbt_scb_t *fbt_scb = FBT_SCB(fbt_info, scb);
	wlc_fbt_scb_cubby_t *fbt_scb_cubby = FBT_SCB_CUBBY(fbt_info, scb);

	if (!fbt_scb)
		return;
	if (fbt_scb->auth_resp_ies) {
		MFREE(wlc->osh, fbt_scb->auth_resp_ies, fbt_scb->auth_resp_ielen);
		fbt_scb->auth_resp_ielen = 0;
		fbt_scb->auth_resp_ies = NULL;
	}
	wlc_fbtap_free_fbties(wlc, &(fbt_scb->fbties_assoc));
	MFREE(fbt_priv->osh, fbt_scb, sizeof(wlc_fbt_scb_t));
	fbt_scb_cubby->cubby = NULL;
}

#ifdef BCMDBG
static void
wlc_fbt_scb_dump(void *context, struct scb *scb, struct bcmstrbuf *b)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)context;
	char eabuf[ETHER_ADDR_STR_LEN];
	wlc_fbt_scb_t *fbt_scb = FBT_SCB(fbt_info, scb);

	if (fbt_scb == NULL)
		return;

	bcm_bprintf(b, "FBT Etheraddr %s\n", bcm_ether_ntoa(&scb->ea, eabuf));
	bcm_bprhex(b, "\t\tPMK_R1Name: ", TRUE, fbt_scb->pmk_r1_name, WPA2_PMKID_LEN);
	bcm_bprhex(b, "\t\tptk: ", TRUE, fbt_scb->ptk.tk1, WPA2_PMKID_LEN);
	bcm_bprhex(b, "\t\tgtk: ", TRUE, fbt_scb->gtk.key, fbt_scb->gtk.key_len);
	if (fbt_scb->auth_resp_ies) {
		bcm_bprhex(b, "\t\tIEs: ", TRUE, fbt_scb->auth_resp_ies, fbt_scb->auth_resp_ielen);
	}
	bcm_bprintf(b, "Auth Time:%d\n", fbt_scb->auth_time);
}
#endif /* BCMDBG */

static int
wlc_fbtap_process_auth_resp(wlc_info_t *wlc, wlc_fbt_info_t *fbt_info,
	struct scb *scb, wlc_fbt_auth_resp_t *fbt_auth_resp)
{
	wlc_bsscfg_t *bsscfg = scb->bsscfg;
	wlc_fbt_scb_t *fbt_scb = FBT_SCB(fbt_info, scb);

	ASSERT(scb != NULL);

	if (!(SCB_AUTHENTICATING(scb))) {
		WL_ERROR(("wl%d: %s: Sending Deauth scb state not pending \n",
			wlc->pub->unit, __FUNCTION__));
		goto deauth;
	}

	if (fbt_scb->auth_resp_ies) {
		MFREE(wlc->osh, fbt_scb->auth_resp_ies, fbt_scb->auth_resp_ielen);
		fbt_scb->auth_resp_ielen = 0;
		fbt_scb->auth_resp_ies = NULL;
	}

	if (fbt_auth_resp->status != DOT11_SC_SUCCESS) {
		WL_ERROR(("wl%d: %s: Sending Deauth status %d \n",
			wlc->pub->unit, __FUNCTION__, fbt_auth_resp->status));
		wlc_sendauth(bsscfg, &scb->ea, &bsscfg->BSSID, scb,
			DOT11_FAST_BSS, 2, fbt_auth_resp->status,
			NULL, DOT11_CAP_SHORT);
		goto deauth;
	}

	if (fbt_auth_resp->ie_len < (sizeof(dot11_ft_ie_t) +
		sizeof(dot11_mdid_ie_t) + sizeof(wpa_rsn_ie_fixed_t))) {
		goto deauth;
	}

	fbt_scb->auth_resp_ies = (uint8 *) MALLOCZ(wlc->osh, fbt_auth_resp->ie_len);
	WL_FBT(("wl%d: %s: Recvd auth len %d \n", wlc->pub->unit, __FUNCTION__,
			fbt_scb->auth_resp_ielen));

	if (fbt_scb->auth_resp_ies == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, fbt_auth_resp->ie_len));
		wlc_sendauth(bsscfg, &scb->ea, &bsscfg->BSSID, scb,
			DOT11_FAST_BSS, 2, fbt_auth_resp->status,
			NULL, DOT11_CAP_SHORT);
		goto deauth;
	}
	fbt_scb->auth_resp_ielen = fbt_auth_resp->ie_len;
	bcopy(&(fbt_auth_resp->ies[0]), fbt_scb->auth_resp_ies, fbt_scb->auth_resp_ielen);

	bcopy(&(fbt_auth_resp->pmk_r1_name), fbt_scb->pmk_r1_name, WPA2_PMKID_LEN);
	bcopy(&(fbt_auth_resp->ptk), &(fbt_scb->ptk), sizeof(wpa_ptk_t));
	bcopy(&(fbt_auth_resp->gtk), &(fbt_scb->gtk), sizeof(wpa_gtk_t));
#if defined(BCMDBG)
	if (WL_FBT_ON()) {
		prhex("FBT AUTH RESP ies", fbt_scb->auth_resp_ies, fbt_auth_resp->ie_len);
		prhex("pmk r1 name in iov", fbt_scb->pmk_r1_name, 16);
		prhex("pmk ptk kck iov", fbt_scb->ptk.kck, 16);
		prhex("pmk ptk kek iov", fbt_scb->ptk.kek, 16);
		prhex("pmk ptk tk1 iov", fbt_scb->ptk.tk1, 16);
		prhex("pmk ptk tk1 iov", fbt_scb->ptk.tk2, 16);
		prhex("pmk gtk in iov", fbt_scb->gtk.key, 32);
	}
#endif // endif

	fbt_scb->status = fbt_auth_resp->status;

	wlc_scb_clearstatebit_bsscfg(scb, PENDING_AUTH, WLC_BSSCFG_IDX(bsscfg));
	wlc_scb_setstatebit_bsscfg(scb, AUTHENTICATED, WLC_BSSCFG_IDX(bsscfg));
	WL_FBT(("wl%d: %s:  sending auth resp success %d \n", wlc->pub->unit, __FUNCTION__,
		fbt_scb->auth_resp_ielen));
	wlc_ap_sendauth(wlc->ap, bsscfg, scb, DOT11_FAST_BSS, 2,
		fbt_auth_resp->status, NULL, DOT11_CAP_SHORT, TRUE);

	return BCME_OK;
deauth:
	wlc_scbfree(wlc, scb);
	return BCME_ERROR;
}

static int
wlc_fbtap_process_ds_auth_resp(wlc_info_t *wlc, wlc_fbt_info_t *fbt_info,
	wlc_bsscfg_t *bsscfg, wlc_fbt_auth_resp_t *fbt_auth_resp)
{
	char eabuf[ETHER_ADDR_STR_LEN];
	struct ether_addr ea;
	char *sa;
	uint8 *tlvs;
	uint tlvs_len;
	wlc_fbt_ies_t fbties;
	wlc_fbt_scb_t *fbt_scb;
	uint status = DOT11_SC_SUCCESS;
	struct scb *scb = NULL;
	uint32 reassoc_deadline = 0;
	bss_fbt_info_t *fbt_bsscfg = FBT_BSSCFG_CUBBY(fbt_info, bsscfg);
	int err = BCME_OK;

	memcpy(ea.octet, fbt_auth_resp->macaddr, ETHER_ADDR_LEN);
	sa = bcm_ether_ntoa(&ea, eabuf);

	wlc_scbfind_delete(wlc, bsscfg, (struct ether_addr *)&fbt_auth_resp->macaddr);

	tlvs = fbt_auth_resp->ies;
	tlvs_len = fbt_auth_resp->ie_len;

	wlc_fbtap_parse_fbties(wlc, bsscfg, tlvs, tlvs_len, &fbties);

	if ((status = wlc_fbtap_validate_auth_fbties(wlc, fbt_bsscfg, fbties, sa)) !=
		DOT11_SC_SUCCESS) {
		err = BCME_NOMEM;
		status = DOT11_SC_FAILURE;
		goto send_result;
	}

	/* allocate an scb */
	if (!(scb = wlc_scblookup(wlc, bsscfg, (struct ether_addr *) &ea))) {
		WL_ERROR(("wl%d: %s: out of scbs for %s\n", wlc->pub->unit, __FUNCTION__,
			sa));
		err = BCME_NOMEM;
		status = DOT11_SC_FAILURE;
		goto send_result;
	}

	fbt_scb = FBT_SCB(fbt_info, scb);

	if (fbt_scb->auth_resp_ies) {
		MFREE(wlc->osh, fbt_scb->auth_resp_ies, fbt_scb->auth_resp_ielen);
		fbt_scb->auth_resp_ielen = 0;
		fbt_scb->auth_resp_ies = NULL;
	}

	if (fbt_auth_resp->ie_len < (sizeof(dot11_ft_ie_t) +
		sizeof(dot11_mdid_ie_t) + sizeof(wpa_rsn_ie_fixed_t))) {
		status = DOT11_SC_FAILURE;
		goto send_result;
	}

	fbt_scb->auth_resp_ies = (uint8 *) MALLOCZ(wlc->osh, fbt_auth_resp->ie_len);

	if (fbt_scb->auth_resp_ies == NULL) {
		err = BCME_NOMEM;
		status = DOT11_SC_FAILURE;
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, fbt_auth_resp->ie_len));
		goto send_result;
	}

	fbt_scb->auth_resp_ielen = fbt_auth_resp->ie_len;
	bcopy(fbt_auth_resp->pmk_r1_name, fbt_scb->pmk_r1_name, WPA2_PMKID_LEN);
	bcopy(&(fbt_auth_resp->ptk), &(fbt_scb->ptk), sizeof(wpa_ptk_t));
	bcopy(&(fbt_auth_resp->gtk), &(fbt_scb->gtk), sizeof(wpa_gtk_t));
	bcopy(fbt_auth_resp->ies, fbt_scb->auth_resp_ies, fbt_scb->auth_resp_ielen);

send_result:
	if (status == DOT11_SC_SUCCESS) {
		fbt_scb->auth_time = OSL_SYSUPTIME();
		reassoc_deadline = ((fbt_bsscfg->reassoc_time) * DOT11_TU_TO_US)/1000;
		wlc_fbtap_add_timer(wlc, scb, reassoc_deadline);
		wlc_scb_setstatebit_bsscfg(scb, AUTHENTICATED, WLC_BSSCFG_IDX(bsscfg));
		wlc_scb_clearstatebit_bsscfg(scb, PENDING_AUTH, WLC_BSSCFG_IDX(bsscfg));
		scb->auth_alg = DOT11_FAST_BSS;
		wlc_bss_mac_event(wlc, bsscfg, WLC_E_AUTH_IND, &scb->ea, WLC_E_STATUS_SUCCESS,
				DOT11_SC_SUCCESS, scb->auth_alg, 0, 0);
	}
	else {
		if (scb) {
			wlc_scbfree(wlc, scb);
		}
	}
	wlc_fbtap_free_fbties(wlc, &fbties);
	return err;
}

static void
wlc_fbtap_add_timer(wlc_info_t *wlc, struct scb *scb, uint timeval)
{
	wlc_fbt_info_t *fbt_info = wlc->fbt;
	wlc_bsscfg_t *bsscfg = scb->bsscfg;
	bss_fbt_info_t *fbt_bsscfg;

	fbt_bsscfg = FBT_BSSCFG_CUBBY(fbt_info, bsscfg);
	if (fbt_bsscfg->fbt_timer_set)
		return;
	wl_add_timer(wlc->wl, fbt_info->fbt_timer, timeval, 0);
	fbt_bsscfg->fbt_timer_set = TRUE;
}

static int
wlc_fbtap_parse_rde_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_fbt_priv_t *fbt_priv;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	struct scb *scb = ftpparm->assocreq.scb;
	wlc_fbt_scb_t *fbt_scb;
	int len, ric_elem_count;
	uint8 *ricend;
	bool found = FALSE;

	if (!BSSCFG_AP(cfg))
		return BCME_OK;

	if (!scb)
		return BCME_OK;

	if (!fbt_info)
		return BCME_OK;

	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	ASSERT(ftpparm != NULL);

	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, cfg) && scb) {
		fbt_scb = FBT_SCB(fbt_info, scb);

		/* In intial association only mdid is validated */
		if (scb->auth_alg == DOT11_OPEN_SYSTEM) {
			return BCME_OK;
		}

		found = wlc_fbt_parse_ric(data->ie, data->ie_len, &ricend, &ric_elem_count);
		if (found && fbt_scb) {
			len = ricend - data->ie;
			if (fbt_scb->fbties_assoc.ricie) {
				MFREE(wlc->osh, fbt_scb->fbties_assoc.ricie,
					fbt_scb->fbties_assoc.ric_len);
				fbt_scb->fbties_assoc.ric_len = 0;
				fbt_scb->fbties_assoc.ric_elem_count = 0;
				fbt_scb->fbties_assoc.ricie = NULL;
			}
			fbt_scb->fbties_assoc.ricie = (uint8 *) MALLOC(wlc->osh, len);
			memcpy(fbt_scb->fbties_assoc.ricie, data->ie, len);
			fbt_scb->fbties_assoc.ric_len = len;
			fbt_scb->fbties_assoc.ric_elem_count = ric_elem_count;
		}

		wlc_fbt_set_ea(fbt_info, cfg, &scb->ea);

		/* Do the MIC calculation to validate FT IE when RDE IE is present */
		if (!wlc_fbt_reassoc_validate_ftie(fbt_info, cfg, scb, data->ie,
			data->ie_len, (dot11_ft_ie_t *)ftpparm->assocreq.ft_ie,
			(bcm_tlv_t *)ftpparm->assocreq.md_ie,
			(bcm_tlv_t *)ftpparm->assocreq.wpa2_ie,
			FT_MIC_ASSOC_REQUEST_TSN)) {
			WL_ERROR(("wl%d: %s failed, ignoring reassoc request\n",
			   wlc->pub->unit, __FUNCTION__));
			ftpparm->assocreq.status = DOT11_SC_INVALID_FTIE;
			return BCME_ERROR;
		}
	}

	return BCME_OK;
}

static int
wlc_fbtap_parse_ft_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_fbt_priv_t *fbt_priv;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	struct scb *scb = ftpparm->assocreq.scb;
	wlc_fbt_scb_t *fbt_scb;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	uint8 *ptr;
	dot11_ft_ie_t *fbtieptr;
	bcm_tlv_t *ie;
	int len = 0;
	dot11_ft_ie_t *fbt_auth;
	wlc_fbt_ies_t fbties_assoc;
	wlc_fbt_ies_t fbties_auth;
	uint8 *tlvs_auth;
	uint tlvs_auth_len;

	if (!BSSCFG_AP(cfg))
		return BCME_OK;

	if (!scb)
		return BCME_OK;

	if (!fbt_info)
		return BCME_OK;

	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	ASSERT(ftpparm != NULL);

	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, cfg)) {
		ie = (bcm_tlv_t *)data->ie;

		/* bail if we don't have fixed length of ft ie */
		if (ie && ((ie->len + TLV_HDR_LEN) < sizeof(dot11_ft_ie_t))) {
			return BCME_OK;
		}

		memset(&fbties_assoc, 0, sizeof(fbties_assoc));

		fbtieptr = (dot11_ft_ie_t *)ie;

		/* Parse the optional sub elements and update the scb */
		 len = fbtieptr->len + TLV_HDR_LEN - sizeof(dot11_ft_ie_t);
		ptr = (uint8 *)ie + sizeof(dot11_ft_ie_t);

		/* parse optional FTIE elements */
		while (len > TLV_HDR_LEN) {
			/* PMKR1 Key Holder */
			if (ptr[DOT11_SUBELEM_ID_OFF] == DOT11_FBT_SUBELEM_ID_R1KH_ID) {
				ie = bcm_parse_tlvs(ptr, len, DOT11_FBT_SUBELEM_ID_R1KH_ID);
				if (ie == NULL || (ie->len != DOT11_FBT_SUBELEM_R1KH_LEN)) {
					break;
				}
				fbties_assoc.r1kh_id = ie->data;
				fbties_assoc.r1kh_id_len = ie->len;
			}
			/* GTK */
			else if (ptr[DOT11_SUBELEM_ID_OFF] == DOT11_FBT_SUBELEM_ID_GTK) {
				ie = bcm_parse_tlvs(ptr, len, DOT11_FBT_SUBELEM_ID_GTK);
				if (ie == NULL || (ie->len < DOT11_FBT_SUBELEM_GTK_MIN_LEN) ||
					(ie->len > DOT11_FBT_SUBELEM_GTK_MAX_LEN)) {
					break;
				}
				fbties_assoc.gtk = ie->data;
				fbties_assoc.gtk_len = ie->len;
			}
			/* PMKR0 Key Holder */
			else if (ptr[DOT11_SUBELEM_ID_OFF] == DOT11_FBT_SUBELEM_ID_R0KH_ID) {
				ie = bcm_parse_tlvs(ptr, len, DOT11_FBT_SUBELEM_ID_R0KH_ID);
				if (ie == NULL || (ie->len < DOT11_FBT_SUBELEM_R0KH_MIN_LEN) ||
					(ie->len > DOT11_FBT_SUBELEM_R0KH_MAX_LEN)) {
					break;
				}
				fbties_assoc.r0kh_id = ie->data;
				fbties_assoc.r0kh_id_len = ie->len;
			}
			/* IGTK Holder */
			else if (ptr[DOT11_SUBELEM_ID_OFF] == DOT11_FBT_SUBELEM_ID_IGTK) {
				ie = bcm_parse_tlvs(ptr, len, DOT11_FBT_SUBELEM_ID_IGTK);
				if (ie == NULL || (ie->len != DOT11_FBT_SUBELEM_IGTK_LEN)) {
					break;
				}
				fbties_assoc.igtk = ie->data;
				fbties_assoc.igtk_len = ie->len;
			}
			len = len - ie->len - TLV_HDR_LEN;
			ptr += ie->len + TLV_HDR_LEN;
		}

		/* In intial association only mdid is validated */
		if (scb->auth_alg == DOT11_OPEN_SYSTEM) {
			return BCME_OK;
		}

		/* Get FBT IEs sent in FBT Authentication Response */
		fbt_scb = FBT_SCB(fbt_info, scb);
		tlvs_auth = fbt_scb->auth_resp_ies;
		tlvs_auth_len = fbt_scb->auth_resp_ielen;

		memset(&fbties_auth, 0, sizeof(wlc_fbt_ies_t));
		wlc_fbtap_parse_fbties(wlc, cfg, tlvs_auth, tlvs_auth_len, &fbties_auth);

		fbt_auth = (dot11_ft_ie_t *)fbties_auth.ftie;
		if ((memcmp(fbt_auth->snonce, fbtieptr->snonce, sizeof(fbt_auth->snonce)) != 0)||
			(memcmp(fbt_auth->anonce, fbtieptr->anonce,
			sizeof(fbt_auth->anonce) != 0))) {
			ftpparm->assocreq.status = DOT11_SC_INVALID_FTIE;
			WL_FBT(("wl%d: %s: Recvd assoc frame FBT w/invalid SNONCE or ANONCE "
				"from sta %s\n", wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(&scb->ea, eabuf)));
			wlc_fbtap_free_fbties(wlc, &fbties_auth);
			return BCME_ERROR;
		}
		if ((fbties_auth.r0kh_id_len != fbties_assoc.r0kh_id_len) ||
			(fbties_auth.r1kh_id_len != fbties_assoc.r1kh_id_len)) {
			ftpparm->assocreq.status = DOT11_SC_INVALID_FTIE;
			WL_FBT(("wl%d: %s: Recvd assoc frame FBT w/invalid r0kh or r1kh "
				"from sta %s\n", wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(&scb->ea, eabuf)));
			wlc_fbtap_free_fbties(wlc, &fbties_auth);
			return BCME_ERROR;
		}
		if ((fbties_assoc.r0kh_id != NULL) &&
			(fbties_auth.r0kh_id != NULL) &&
			((memcmp(fbties_auth.r0kh_id, fbties_assoc.r0kh_id,
			fbties_assoc.r0kh_id_len) != 0) ||
			(memcmp(fbties_auth.r1kh_id, fbties_assoc.r1kh_id,
			fbties_assoc.r1kh_id_len) != 0))) {
			ftpparm->assocreq.status = DOT11_SC_INVALID_FTIE;
			WL_FBT(("wl%d: %s: Recvd assoc frame FBT w/invalid r0kh or r1kh "
				"from sta %s\n", wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(&scb->ea, eabuf)));
			wlc_fbtap_free_fbties(wlc, &fbties_auth);
			return BCME_ERROR;
		}
		wlc_fbtap_free_fbties(wlc, &fbties_auth);

		if (data->ft == FC_REASSOC_REQ) {
			uint8 *tlvs;
			uint tlvs_len;
			uint8 *ricie = NULL;

			/* Check if RDE IE is present in reassoc request */
			tlvs = data->buf;
			tlvs_len = data->buf_len;
			ricie = (uint8*)bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RDE_ID);
			if (ricie != NULL) {
				/* RDE IE is present, save FT IE for MIC calculation
				 * while processing RDE IE.
				 */
				ftpparm->assocreq.ft_ie = data->ie;
			}
			else {
				wlc_fbt_set_ea(fbt_info, cfg, &scb->ea);
				/* RDE IE is absent. Do the MIC calculation here without RDE */
				if (!wlc_fbt_reassoc_validate_ftie(fbt_info, cfg, scb, NULL, 0,
					(dot11_ft_ie_t *)data->ie,
					(bcm_tlv_t *)ftpparm->assocreq.md_ie,
					(bcm_tlv_t *)ftpparm->assocreq.wpa2_ie,
					FT_MIC_ASSOC_REQUEST_TSN)) {
					WL_ERROR(("wl%d: %s failed, ignoring reassoc request\n",
					   wlc->pub->unit, __FUNCTION__));
					ftpparm->assocreq.status = DOT11_SC_INVALID_FTIE;
					return BCME_ERROR;
				}
			}
		}
	}

	return BCME_OK;
}

static int
wlc_fbtap_auth_parse_ft_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_fbt_priv_t *fbt_priv;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	struct scb *scb = ftpparm->auth.scb;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif
	uint8 *ptr;
	dot11_ft_ie_t *fbtieptr;
	bcm_tlv_t *ie;
	int len = 0;
	wlc_fbt_ies_t fbties;
	uint reassoc_deadline;
	bss_fbt_info_t *fbt_bsscfg;
	wlc_fbt_scb_t *fbt_scb;
	wl_event_fbt_t *pevent;
	uint16 event_len;

	if (!BSSCFG_AP(cfg))
		return BCME_OK;

	if (!scb)
		return BCME_OK;

	if (!fbt_info)
		return BCME_OK;

	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	ASSERT(ftpparm != NULL);

	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, cfg)) {

		/* In initial association only mdid is validated */
		if (ftpparm->auth.alg == DOT11_OPEN_SYSTEM) {
			return BCME_OK;
		}

		fbt_bsscfg = FBT_BSSCFG_CUBBY(fbt_info, cfg);
		ie = (bcm_tlv_t *)data->ie;

		/* bail if we don't have fixed length of ft ie */
		if (ie && ((ie->len + TLV_HDR_LEN) < sizeof(dot11_ft_ie_t))) {
			return BCME_OK;
		}

		memset(&fbties, 0, sizeof(fbties));

		fbtieptr = (dot11_ft_ie_t *)ie;

		/* Parse the optional sub elements and update the scb */
		len = fbtieptr->len + TLV_HDR_LEN - sizeof(dot11_ft_ie_t);
		ptr = (uint8 *)ie + sizeof(dot11_ft_ie_t);

		/* parse optional FTIE elements */
		while (len > TLV_HDR_LEN) {
			/* PMKR1 Key Holder */
			if (ptr[0] == DOT11_FBT_SUBELEM_ID_R1KH_ID) {
				ie = bcm_parse_tlvs(ptr, len, DOT11_FBT_SUBELEM_ID_R1KH_ID);
				if (ie == NULL || (ie->len != DOT11_FBT_SUBELEM_R1KH_LEN)) {
					break;
				}
				fbties.r1kh_id = ie->data;
				fbties.r1kh_id_len = ie->len;
			}
			/* GTK */
			else if (ptr[0] == DOT11_FBT_SUBELEM_ID_GTK) {
				ie = bcm_parse_tlvs(ptr, len, DOT11_FBT_SUBELEM_ID_GTK);
				if (ie == NULL || (ie->len < DOT11_FBT_SUBELEM_GTK_MIN_LEN) ||
					(ie->len > DOT11_FBT_SUBELEM_GTK_MAX_LEN)) {
					break;
				}
				fbties.gtk = ie->data;
				fbties.gtk_len = ie->len;
			}
			/* PMKR0 Key Holder */
			else if (ptr[0] == DOT11_FBT_SUBELEM_ID_R0KH_ID) {
				ie = bcm_parse_tlvs(ptr, len, DOT11_FBT_SUBELEM_ID_R0KH_ID);
				if (ie == NULL || (ie->len < DOT11_FBT_SUBELEM_R0KH_MIN_LEN) ||
					(ie->len > DOT11_FBT_SUBELEM_R0KH_MAX_LEN)) {
					break;
				}
				fbties.r0kh_id = ie->data;
				fbties.r0kh_id_len = ie->len;
			}
			/* IGTK Holder */
			else if (ptr[0] == DOT11_FBT_SUBELEM_ID_IGTK) {
				ie = bcm_parse_tlvs(ptr, len, DOT11_FBT_SUBELEM_ID_IGTK);
				if (ie == NULL || (ie->len != DOT11_FBT_SUBELEM_IGTK_LEN)) {
					break;
				}
				fbties.igtk = ie->data;
				fbties.igtk_len = ie->len;
			}
			len = len - ie->len - TLV_HDR_LEN;
			ptr += ie->len + TLV_HDR_LEN;
		}

		if ((ftpparm->auth.alg == DOT11_FAST_BSS) &&
			(fbties.r0kh_id == NULL)) {
			ftpparm->auth.status = DOT11_SC_INVALID_FTIE;
			WL_FBT(("wl%d: %s: Recvd auth frame FBT w/invalid r0kh "
				"from sta %s\n", wlc->pub->unit, __FUNCTION__,
				bcm_ether_ntoa(&scb->ea, eabuf)));
			return BCME_ERROR;
		}

		if (ftpparm->auth.alg == DOT11_FAST_BSS) {
			wlc_scb_setstatebit_bsscfg(scb, PENDING_AUTH,
			                           WLC_BSSCFG_IDX(cfg));
			event_len = sizeof(*pevent) + data->buf_len;
			if (!(pevent = (wl_event_fbt_t *) MALLOCZ(wlc->osh, event_len))) {
				WL_FBT(("wl%d: %s: out of mem, malloced %d bytes\n",
					wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
				return BCME_ERROR;
			}

			/* Send event data in little endian */
			pevent->version = htol16(WL_EVENT_FBT_VER_1);
			pevent->length = htol16(event_len);
			pevent->type = htol16(WL_E_FBT_TYPE_FBT_OTA_AUTH);
			pevent->data_offset = htol16(sizeof(*pevent));

			if (data->buf_len > 0 && data->buf != NULL) {
				memcpy(pevent->data, data->buf, data->buf_len);
			}
			/* Notify application, send all IEs to application */
			wlc_bss_mac_event(wlc, cfg, WLC_E_FBT, &scb->ea,
				WLC_E_STATUS_SUCCESS, 0, 0,
				(void *)pevent, event_len);
			MFREE(wlc->osh, pevent, event_len);
			fbt_scb = FBT_SCB(fbt_info, scb);
			fbt_scb->auth_time = OSL_SYSUPTIME();
			reassoc_deadline = ((fbt_bsscfg->reassoc_time) * DOT11_TU_TO_US)/1000;
			WL_FBT(("wl%d: %s: starting reassoc timer %d for sta %s\n",
			wlc->pub->unit, __FUNCTION__, reassoc_deadline,
			bcm_ether_ntoa(&scb->ea, eabuf)));

			wlc_fbtap_add_timer(wlc, scb, reassoc_deadline);
		}
	}

	return BCME_OK;
}

enum {	RSNE_OFFSETS_VER_INDEX = 0,
	RSNE_OFFSETS_GDCS_INDEX,
	RSNE_OFFSETS_PCS_INDEX,
	RSNE_OFFSETS_AKMS_INDEX,
	RSNE_OFFSETS_CAP_INDEX,
	RSNE_OFFSETS_PMKID_INDEX,
	RSNE_OFFSETS_GMCS_INDEX,
	RSNE_OFFSETS_MAX_INDEX
};

static int
wlc_fbtap_validate_rsn_ie(bcm_tlv_t *ie, uint8 *offsets)
{
	uint8 *ptr, *ptr_inc;
	int len = 0;
	uint16 count = 0;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *mgmt;
	wpa_pmkid_list_t *pkid;

	if (!ie) {
		return BCME_BADARG;
	}
	if (ie->id != DOT11_MNG_RSN_ID) {
		return BCME_IE_NOTFOUND;
	}
	len = (int)(ie->len + TLV_HDR_LEN);
	ptr_inc = ptr = (uint8 *)ie;
	/* for rsn max len check */
	if (len > TLV_BODY_LEN_MAX) {
		return BCME_BADLEN;
	}
	/* for rsn min len check */
	if ((len - (ptr_inc - ptr)) < (int)sizeof(wpa_rsn_ie_fixed_t)) {
		return BCME_BADLEN;
	}
	offsets[RSNE_OFFSETS_VER_INDEX] = TLV_HDR_LEN;

	/* for rsn group data cipher suite */
	ptr_inc += sizeof(wpa_rsn_ie_fixed_t);
	if (!(len - (ptr_inc - ptr))) {
		return BCME_OK; /* only have upto ver */
	}
	if ((len - (ptr_inc - ptr)) < (int)sizeof(wpa_suite_mcast_t)) {
		return BCME_BADLEN;
	}
	offsets[RSNE_OFFSETS_GDCS_INDEX] = (uint8)(ptr_inc - ptr);

	/* for rsn pairwise cipher suite */
	ptr_inc += sizeof(wpa_suite_mcast_t);
	if (!(len - (ptr_inc - ptr))) {
		return BCME_OK;
	}
	if ((len - (ptr_inc - ptr)) < (int)sizeof(ucast->count)) {
		return BCME_BADLEN;
	}
	offsets[RSNE_OFFSETS_PCS_INDEX] = (uint8)(ptr_inc - ptr);
	ucast = (wpa_suite_ucast_t *)ptr_inc;
	count = ltoh16_ua(&ucast->count);

	/* for rsn AKM authentication */
	ptr_inc += count * WPA_SUITE_LEN + sizeof(ucast->count);
	if (!(len - (ptr_inc - ptr))) {
		return BCME_OK;
	}
	if (!count) {
		return BCME_BADLEN;
	}
	if ((len - (ptr_inc - ptr)) < (int)sizeof(mgmt->count)) {
		return BCME_BADLEN;
	}
	offsets[RSNE_OFFSETS_AKMS_INDEX] = (uint8)(ptr_inc - ptr);
	mgmt = (wpa_suite_auth_key_mgmt_t *)ptr_inc;
	count = ltoh16_ua(&mgmt->count);

	/* for rsn capabilities */
	ptr_inc += count * WPA_SUITE_LEN + sizeof(mgmt->count);
	if (!(len - (ptr_inc - ptr))) {
		return BCME_OK;
	}
	if (!count) {
		return BCME_BADLEN;
	}
	if ((len - (ptr_inc - ptr)) < RSN_CAP_LEN) {
		return BCME_BADLEN;
	}
	offsets[RSNE_OFFSETS_CAP_INDEX] = (uint8)(ptr_inc - ptr);

	/* for rsn PMKID */
	ptr_inc += RSN_CAP_LEN;
	if (!(len - (ptr_inc - ptr))) {
		return BCME_OK;
	}
	if ((len - (ptr_inc - ptr)) < (int)sizeof(pkid->count)) {
		return BCME_BADLEN;
	}
	offsets[RSNE_OFFSETS_PMKID_INDEX] = (uint8)(ptr_inc - ptr);
	pkid = (wpa_pmkid_list_t*)ptr_inc;
	count = ltoh16_ua(&pkid->count);

	/* for rsn group management cipher suite */
	ptr_inc += count * WPA2_PMKID_LEN + sizeof(pkid->count);
	if (!(len - (ptr_inc - ptr))) {
		return BCME_OK;
	}
	if (!count) {
		return BCME_BADLEN;
	}
	if ((len - (ptr_inc - ptr)) != (int)sizeof(wpa_suite_mcast_t)) {
		return BCME_BADLEN;
	}
	offsets[RSNE_OFFSETS_GMCS_INDEX] = (uint8)(ptr_inc - ptr);

	return BCME_OK;
}

static int
wlc_fbtap_parse_rsn_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_fbt_priv_t *fbt_priv;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	struct scb *scb = ftpparm->assocreq.scb;
	wlc_fbt_scb_t *fbt_scb;
	uint8 *ptr;
	wpa_suite_auth_key_mgmt_t *mgmt;
	uint32 WPA_auth = cfg->WPA_auth;
	wpa_pmkid_list_t *pmk;
	int count = 0;
	uint8 offsets[RSNE_OFFSETS_MAX_INDEX] = {0};

	if (!BSSCFG_AP(cfg))
		return BCME_OK;

	if (!scb)
		return BCME_OK;

	if (!fbt_info)
		return BCME_OK;

	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	ASSERT(ftpparm != NULL);

	wlc = fbt_priv->wlc;
	fbt_scb = FBT_SCB(fbt_info, scb);

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, cfg) && fbt_scb) {
#ifdef BCMDBG
		char eabuf[ETHER_ADDR_STR_LEN], *sa = bcm_ether_ntoa(&scb->ea, eabuf);
#endif // endif
		/* In initial association only mdid is validated */
		if (scb->auth_alg == DOT11_OPEN_SYSTEM) {
			return BCME_OK;
		}

		int len_status = wlc_fbtap_validate_rsn_ie((bcm_tlv_t *)data->ie, offsets);
		if (len_status != BCME_OK) {
			/* update frame's status, to avoid ASSERT(status != DOT11_SC_SUCCESS)
			 * in wlc_ap_process_assocreq routine
			 */
			ftpparm->assocreq.status = DOT11_SC_INVALID_RSNIE;
			return len_status;
		}
		ptr = (uint8 *)data->ie;

		/* Check the AKM */
		if (offsets[RSNE_OFFSETS_AKMS_INDEX]) {
			mgmt = (wpa_suite_auth_key_mgmt_t *)&ptr[offsets[RSNE_OFFSETS_AKMS_INDEX]];
			count = ltoh16_ua(&mgmt->count);
			if ((scb->auth_alg == DOT11_FAST_BSS) &&
				((count != 1) ||
				!((memcmp(mgmt->list[0].oui, WPA2_OUI, DOT11_OUI_LEN) == 0) &&
				(((mgmt->list[0].type == RSN_AKM_FBT_1X) &&
				(WPA_auth & WPA2_AUTH_UNSPECIFIED)) ||
				((mgmt->list[0].type == RSN_AKM_FBT_PSK) &&
				(WPA_auth & WPA2_AUTH_PSK)))))) {
				WL_ERROR(("wl%d: %s: bad AKM in WPA2 IE.\n",
					wlc->pub->unit, __FUNCTION__));
				ftpparm->assocreq.status = DOT11_SC_INVALID_AKMP;
				return BCME_ERROR;
			}
		}

		if ((scb->auth_alg == DOT11_FAST_BSS) &&
			(!(data->ft == FC_REASSOC_REQ))) {
			WL_FBT(("wl%d: %s: Recvd assoc frame for FBT auth from sta %s\n",
				wlc->pub->unit, __FUNCTION__, sa));
			ftpparm->assocreq.status = DOT11_SC_FAILURE;
			return BCME_ERROR;
		}

		if (offsets[RSNE_OFFSETS_PMKID_INDEX]) {
			pmk = (wpa_pmkid_list_t *)&ptr[offsets[RSNE_OFFSETS_PMKID_INDEX]];
			if (pmk->count.low != 0 || pmk->count.high != 0) {
				if (memcmp(pmk->list, fbt_scb->pmk_r1_name, WPA2_PMKID_LEN) != 0) {
					WL_FBT(("wl%d: %s: Recvd assoc frame FBT "
						"with invalid PMKID from sta %s\n",
						wlc->pub->unit, __FUNCTION__, sa));
					ftpparm->assocreq.status = DOT11_SC_INVALID_PMKID;
					return BCME_BAD_IE_DATA;
				}
			}
		}

		/* save wpa2 ie for MIC calc */
		ftpparm->assocreq.wpa2_ie = data->ie;
	}

	return BCME_OK;
}

static int
wlc_fbtap_auth_parse_rsn_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_info_t *wlc;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_fbt_priv_t *fbt_priv;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	struct scb *scb = ftpparm->auth.scb;
	wlc_fbt_scb_t *fbt_scb;
	uint8 *ptr;
	wpa_suite_auth_key_mgmt_t *mgmt;
	uint32 WPA_auth = cfg->WPA_auth;
	wpa_pmkid_list_t *pmk = NULL;
	int count = 0;
	uint8 offsets[RSNE_OFFSETS_MAX_INDEX] = {0};

	if (!BSSCFG_AP(cfg))
		return BCME_OK;

	if (!scb)
		return BCME_OK;

	if (!fbt_info)
		return BCME_OK;

	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	ASSERT(ftpparm != NULL);

	wlc = fbt_priv->wlc;
	fbt_scb = FBT_SCB(fbt_info, scb);

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, cfg) && fbt_scb) {
#ifdef BCMDBG
		char eabuf[ETHER_ADDR_STR_LEN], *sa = bcm_ether_ntoa(&scb->ea, eabuf);
#endif // endif
		int len_status;
		/* In initial association only mdid is validated */
		if (ftpparm->auth.alg == DOT11_OPEN_SYSTEM) {
			return BCME_OK;
		}

		len_status = wlc_fbtap_validate_rsn_ie((bcm_tlv_t *)data->ie, offsets);
		if (len_status != BCME_OK) {
			return len_status;
		}
		ptr = (uint8 *)data->ie;

		/* Check the AKM */
		if (offsets[RSNE_OFFSETS_AKMS_INDEX]) {
			mgmt = (wpa_suite_auth_key_mgmt_t *)&ptr[offsets[RSNE_OFFSETS_AKMS_INDEX]];
			count = ltoh16_ua(&mgmt->count);
			if ((count != 1) ||
				!((memcmp(mgmt->list[0].oui, WPA2_OUI, DOT11_OUI_LEN) == 0) &&
				(((mgmt->list[0].type == RSN_AKM_FBT_1X) &&
				(WPA_auth & WPA2_AUTH_UNSPECIFIED)) ||
				((mgmt->list[0].type == RSN_AKM_FBT_PSK) &&
				(WPA_auth & WPA2_AUTH_PSK))))) {
				WL_ERROR(("wl%d: %s: bad AKM in WPA2 IE.\n",
					wlc->pub->unit, __FUNCTION__));
				ftpparm->auth.status = DOT11_SC_INVALID_AKMP;
				return BCME_ERROR;
			}
		}

		if (offsets[RSNE_OFFSETS_PMKID_INDEX]) {
			pmk = (wpa_pmkid_list_t *)&ptr[offsets[RSNE_OFFSETS_PMKID_INDEX]];
			if (pmk->count.low == 0 && pmk->count.high == 0) {
				pmk = NULL;
			}
		}

		if ((ftpparm->auth.alg == DOT11_FAST_BSS) &&
			(pmk == NULL)) {
			WL_FBT(("wl%d: %s: FBT auth from sta %s has invalid pmkid\n",
				wlc->pub->unit, __FUNCTION__, sa));
			ftpparm->auth.status = DOT11_SC_INVALID_PMKID;
			return BCME_ERROR;
		}

	}

	return BCME_OK;
}

static int
wlc_fbtap_parse_md_ie(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *cfg = data->cfg;
	wlc_iem_ft_pparm_t *ftpparm = data->pparm->ft;
	struct scb *scb = NULL;
	dot11_mdid_ie_t mdie;
	bss_fbt_info_t *fbt_bsscfg;
	wlc_fbt_priv_t *fbt_priv;

	if (!BSSCFG_AP(cfg))
		return BCME_OK;

	if (data->ft == FC_AUTH) {
		scb = ftpparm->auth.scb;
	}
	else {
		scb = ftpparm->assocreq.scb;
	}

	if (!scb)
		return BCME_OK;

	if (!fbt_info)
		return BCME_OK;

	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	if (data->ie == NULL)
		return BCME_OK;

	ASSERT(ftpparm != NULL);

	if (WLFBT_ENAB(fbt_priv->wlc->pub) && wlc_fbt_enabled(fbt_info, cfg)) {
		fbt_bsscfg = FBT_BSSCFG_CUBBY(fbt_info, cfg);
		if (BCM_TLV_SIZE((bcm_tlv_t *)(data->ie)) != sizeof(dot11_mdid_ie_t)) {
			return BCME_BADLEN;
		}
		memcpy(&mdie, data->ie, sizeof(dot11_mdid_ie_t));

		if (ltoh16(mdie.mdid) != fbt_bsscfg->mdid) {
#ifdef BCMDBG
			char eabuf[ETHER_ADDR_STR_LEN], *sa = bcm_ether_ntoa(&scb->ea, eabuf);
#endif // endif
			WL_FBT(("wl%d: %s: MDID %d Invalid MDID %d from sta %s\n",
			fbt_priv->wlc->pub->unit, __FUNCTION__,
			fbt_bsscfg->mdid, ltoh16(mdie.mdid), sa));
			if (data->ft == FC_AUTH) {
				ftpparm->auth.status = DOT11_SC_INVALID_MDID;
			}
			else {
				ftpparm->assocreq.status = DOT11_SC_INVALID_MDID;
			}
			return BCME_ERROR;
		}

		if (data->ft != FC_AUTH) {
			/* save md ie for MIC calc */
			ftpparm->assocreq.md_ie = data->ie;
		}
	}

	return BCME_OK;
}

static void
wlc_fbtap_parse_fbties(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	uchar *tlvs, uint tlvs_len, wlc_fbt_ies_t *fbt_ies)
{
	bcm_tlv_t *ie;
	dot11_mdid_ie_t *mdieptr;
	dot11_ft_ie_t *fbtieptr;
	int len, ric_elem_count;
	uint8 *ptr;
	uint8 *ricend = NULL;
	bool found = FALSE;
	wpa_pmkid_list_t *pmk;

	memset(fbt_ies, 0, sizeof(wlc_fbt_ies_t));
	ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_MDIE_ID);
	if (BCM_TLV_SIZE(ie) == sizeof(dot11_mdid_ie_t)) {
		mdieptr = (dot11_mdid_ie_t *)ie;
		memcpy(&(fbt_ies->mdie), mdieptr, sizeof(dot11_mdid_ie_t));
	}

	ie = (bcm_tlv_t *)bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RSN_ID);
	if (ie != NULL) {
		uint8 offsets[RSNE_OFFSETS_MAX_INDEX] = {0};
		int err = wlc_fbtap_validate_rsn_ie(ie, offsets);
		if (err != BCME_OK) {
			return;
		}
		fbt_ies->rsnie = (uint8 *) MALLOC(wlc->osh, ie->len + TLV_HDR_LEN);
		if (fbt_ies->rsnie == NULL) {
			WL_FBT(("wl%d: %s: Unable to allocate memory for RSNIE \n",
				wlc->pub->unit, __FUNCTION__));
			return;
		}
		memcpy(fbt_ies->rsnie, ie, ie->len + TLV_HDR_LEN);
		fbt_ies->rsnie_len = ie->len + TLV_HDR_LEN;

		if (offsets[RSNE_OFFSETS_PMKID_INDEX]) {
			ptr = (uint8 *)ie;
			pmk = (wpa_pmkid_list_t *)&ptr[offsets[RSNE_OFFSETS_PMKID_INDEX]];
			if (pmk->count.low != 0 || pmk->count.high != 0) {
				fbt_ies->pmkid = (uint8 *) MALLOC(wlc->osh, WPA2_PMKID_LEN);
				fbt_ies->pmkid_len = WPA2_PMKID_LEN;
				memcpy(fbt_ies->pmkid, pmk->list, WPA2_PMKID_LEN);
			}
		}
	}

	ie = (bcm_tlv_t *) bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_FTIE_ID);
	if (ie && ((ie->len + TLV_HDR_LEN) >= sizeof(dot11_ft_ie_t))) {
		fbt_ies->ftie = (uint8 *) MALLOC(wlc->osh, ie->len + TLV_HDR_LEN);
		fbtieptr = (dot11_ft_ie_t *)ie;
		memcpy(fbt_ies->ftie, fbtieptr, fbtieptr->len + TLV_HDR_LEN);
		fbt_ies->ftie_len = fbtieptr->len + TLV_HDR_LEN;

		/* Parse the optional sub elements and update the scb */
		len = fbt_ies->ftie_len + TLV_HDR_LEN - sizeof(dot11_ft_ie_t);
		ptr = (uint8 *)ie + sizeof(dot11_ft_ie_t);

		/* parse optional FTIE elements */
		while (len > TLV_HDR_LEN) {
			/* PMKR1 Key Holder */
			if (ptr[0] == DOT11_FBT_SUBELEM_ID_R1KH_ID) {
				ie = bcm_parse_tlvs(ptr, len, DOT11_FBT_SUBELEM_ID_R1KH_ID);
				if (ie == NULL || (ie->len != DOT11_FBT_SUBELEM_R1KH_LEN)) {
					break;
				}
				fbt_ies->r1kh_id = (uint8 *) MALLOC(wlc->osh, ie->len);
				if (fbt_ies->r1kh_id != NULL) {
					memcpy(fbt_ies->r1kh_id, ie->data, ie->len);
					fbt_ies->r1kh_id_len = ie->len;
				}
			}
			/* GTK */
			else if (ptr[0] == DOT11_FBT_SUBELEM_ID_GTK) {
				ie = bcm_parse_tlvs(ptr, len, DOT11_FBT_SUBELEM_ID_GTK);
				if (ie == NULL || (ie->len < DOT11_FBT_SUBELEM_GTK_MIN_LEN) ||
				    (ie->len > DOT11_FBT_SUBELEM_GTK_MAX_LEN)) {
					break;
				}
				fbt_ies->gtk = (uint8 *) MALLOC(wlc->osh, ie->len);
				if (fbt_ies->gtk != NULL) {
					memcpy(fbt_ies->gtk, ie->data, ie->len);
					fbt_ies->gtk_len = ie->len;
				}
			}
			/* PMKR0 Key Holder */
			else if (ptr[0] == DOT11_FBT_SUBELEM_ID_R0KH_ID) {
				ie = bcm_parse_tlvs(ptr, len, DOT11_FBT_SUBELEM_ID_R0KH_ID);
				if (ie == NULL || (ie->len < DOT11_FBT_SUBELEM_R0KH_MIN_LEN) ||
				    (ie->len > DOT11_FBT_SUBELEM_R0KH_MAX_LEN)) {
					break;
				}
				fbt_ies->r0kh_id = (uint8 *) MALLOC(wlc->osh, ie->len);
				if (fbt_ies->r0kh_id != NULL) {
					memcpy(fbt_ies->r0kh_id, ie->data, ie->len);
					fbt_ies->r0kh_id_len = ie->len;
				}
			}
			/* IGTK Holder */
			else if (ptr[0] == DOT11_FBT_SUBELEM_ID_IGTK) {
				ie = bcm_parse_tlvs(ptr, len, DOT11_FBT_SUBELEM_ID_IGTK);
				if (ie == NULL || (ie->len != DOT11_FBT_SUBELEM_IGTK_LEN))
					break;
				fbt_ies->igtk = (uint8 *) MALLOC(wlc->osh, ie->len);
				if (fbt_ies->igtk != NULL) {
					memcpy(fbt_ies->igtk, ie->data, ie->len);
					fbt_ies->igtk_len = ie->len;
				}
			}
			len = len - ie->len - TLV_HDR_LEN;
			ptr += (ie->len + TLV_HDR_LEN);
		}
	}

	ie = bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RDE_ID);
	if (ie != NULL) {
		found = wlc_fbt_parse_ric((uint8 *)ie, tlvs_len - ((uint8 *)ie - tlvs),
			&ricend, &ric_elem_count);
		if (found) {
			/* ie = (bcm_tlv_t *)bcm_parse_tlvs(tlvs, tlvs_len, DOT11_MNG_RDE_ID); */
			len = ricend - (uint8 *)ie;
			fbt_ies->ricie = (uint8 *) MALLOC(wlc->osh, len);
			memcpy(fbt_ies->ricie, ie, len);
			fbt_ies->ric_len = len;
			fbt_ies->ric_elem_count = ric_elem_count;
		}
	}
}

static void
wlc_fbtap_free_fbties(wlc_info_t *wlc, wlc_fbt_ies_t *fbties)
{
	if (fbties->ftie) {
		MFREE(wlc->osh, fbties->ftie, fbties->ftie_len);
		fbties->ftie_len = 0;
	}
	if (fbties->r0kh_id) {
		MFREE(wlc->osh, fbties->r0kh_id, fbties->r0kh_id_len);
		fbties->r0kh_id_len = 0;
	}
	if (fbties->r1kh_id) {
		MFREE(wlc->osh, fbties->r1kh_id, fbties->r1kh_id_len);
		fbties->r1kh_id_len = 0;
	}
	if (fbties->gtk) {
		MFREE(wlc->osh, fbties->gtk, fbties->gtk_len);
		fbties->gtk_len = 0;
	}
	if (fbties->pmkid) {
		MFREE(wlc->osh, fbties->pmkid, fbties->pmkid_len);
		fbties->pmkid_len = 0;
	}
	if (fbties->igtk) {
		MFREE(wlc->osh, fbties->igtk, fbties->igtk_len);
		fbties->igtk_len = 0;
	}
	if (fbties->ricie) {
		MFREE(wlc->osh, fbties->ricie, fbties->ric_len);
		fbties->ric_len = 0;
	}
	if (fbties->rsnie) {
		MFREE(wlc->osh, fbties->rsnie, fbties->rsnie_len);
		fbties->rsnie_len = 0;
	}
}

static void
wlc_fbtap_reassociation_timer(void *arg)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *) arg;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	uint cur_time = OSL_SYSUPTIME();
	uint least_reassoc_timer = 0, i;
	struct scb *scb;
	struct scb *next_scb = NULL;
	struct scb_iter scbiter;
	wlc_bsscfg_t *bsscfg;
	wlc_fbt_scb_t *fbt_scb;
	int reassoc_deadline;
	bss_fbt_info_t *fbt_bsscfg;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif // endif

	if (!fbt_info)
		return;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	FOREACH_UP_AP(wlc, i, bsscfg)
	{
		fbt_bsscfg = FBT_BSSCFG_CUBBY(fbt_info, bsscfg);

		if (!fbt_bsscfg || !BSSCFG_IS_FBT(bsscfg))
			continue;
		reassoc_deadline = ((fbt_bsscfg->reassoc_time) * DOT11_TU_TO_US)/1000;
		FOREACH_BSS_SCB(wlc->scbstate, &scbiter, bsscfg, scb)
		{
			if (SCB_AUTHENTICATING(scb) || SCB_AUTHENTICATED(scb)) {
				fbt_scb = FBT_SCB(fbt_info, scb);
				if (!fbt_scb)
					continue;
				if (SCB_ASSOCIATED(scb)) {
					fbt_scb->auth_time = 0;
					continue;
				}
				if ((fbt_scb->auth_time != 0) &&
				(cur_time - fbt_scb->auth_time > reassoc_deadline)) {
					WL_FBT(("wl%d: %s: Reassoc timeout for sta %s\n",
					wlc->pub->unit, __FUNCTION__,
					bcm_ether_ntoa(&scb->ea, eabuf)));
					wlc_scbfree(wlc, scb);
				}
				else if (fbt_scb->auth_time != 0) {
					if (fbt_scb->auth_time < least_reassoc_timer) {
						least_reassoc_timer = fbt_scb->auth_time;
						next_scb = scb;
					}
					if (least_reassoc_timer == 0) {
						least_reassoc_timer = fbt_scb->auth_time;
						next_scb = scb;
					}
				}
			}
		} /* SCB Iterator */
		fbt_bsscfg->fbt_timer_set = FALSE;
		if (least_reassoc_timer) {
			least_reassoc_timer = cur_time - least_reassoc_timer;
			wlc_fbtap_add_timer(wlc, next_scb, least_reassoc_timer);
		}
		least_reassoc_timer = 0;
	} /* BSSCFG Iterator */
}

static int
wlc_fbtap_validate_auth_fbties(wlc_info_t *wlc, bss_fbt_info_t *fbt_bsscfg,
	wlc_fbt_ies_t fbties, char *sa)
{
	if (ltoh16(fbties.mdie.mdid) != fbt_bsscfg->mdid) {
		WL_FBT(("wl%d: %s: MDID %d Invalid MDIE %d from sta %s\n", wlc->pub->unit,
			__FUNCTION__, fbt_bsscfg->mdid, ltoh16(fbties.mdie.mdid), sa));
		return DOT11_SC_INVALID_MDID;
	}

	if (fbties.r0kh_id == NULL) {
		WL_FBT(("wl%d: %s: Invalid FBTIE from sta %s\n", wlc->pub->unit,
			__FUNCTION__, sa));
		return DOT11_SC_INVALID_FTIE;
	}

	if (fbties.pmkid == NULL) {
		WL_FBT(("wl%d: %s: Invalid RSNIE from sta %s\n", wlc->pub->unit,
			__FUNCTION__, sa));
		return DOT11_SC_INVALID_PMKID;
	}

	return DOT11_SC_SUCCESS;
}

static uint
wlc_fbtap_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	struct scb *scb = ftcbparm->assocresp.scb;
	wlc_fbt_scb_t *fbt_scb;
	uint8 buf[257];
	uint8 *cp = buf;
	uint ielen = 0;

	if (!BSSCFG_AP(bsscfg))
		return 0;

	if (!fbt_info)
		return 0;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, bsscfg) && scb) {
		fbt_scb = FBT_SCB(fbt_info, scb);

		if (scb->auth_alg == DOT11_FAST_BSS && fbt_scb) {
			cp = wlc_fbt_write_rsn_ie_safe(wlc, bsscfg, cp, sizeof(buf));
			ielen = cp - buf + sizeof(wpa_pmkid_list_t);
		}
	}

	return ielen;
}

static int
wlc_fbtap_write_rsn_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	uint8 *cp = data->buf;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	struct scb *scb = ftcbparm->assocresp.scb;
	wlc_fbt_scb_t *fbt_scb;
	wpa_pmkid_list_t *wpa_pmkid;
	bcm_tlv_t *wpa2ie;
	int count = 0;

	if (!BSSCFG_AP(bsscfg))
		return BCME_OK;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, bsscfg) && scb) {
		fbt_scb = FBT_SCB(fbt_info, scb);
		wpa2ie = (bcm_tlv_t *)cp;

		if (scb->auth_alg == DOT11_FAST_BSS && fbt_scb) {
			cp = wlc_fbt_write_rsn_ie_safe(wlc, bsscfg, cp, data->buf_len);
			wpa_pmkid = (wpa_pmkid_list_t *)cp;
			count = 1;
			wpa_pmkid->count.low = (uint8)count;
			wpa_pmkid->count.high = (uint8)(count>>8);
			bcopy(fbt_scb->pmk_r1_name, &wpa_pmkid->list[0], WPA2_PMKID_LEN);
			cp += sizeof(wpa_pmkid_list_t);
			wpa2ie->len = cp - data->buf - TLV_HDR_LEN;

			/* save wpa2 ie for MIC calc */
			ftcbparm->assocresp.wpa2_ie = data->buf;
		}
	}

	return BCME_OK;
}

static uint
wlc_fbtap_calc_md_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	uint ielen = 0;

	if (!BSSCFG_AP(bsscfg))
		return 0;

	if (!fbt_info)
		return 0;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, bsscfg)) {
		/* bcn/prbresp/assocresp/reassocresp */
		ielen = sizeof(dot11_mdid_ie_t);
	}

	return ielen;
}

static int
wlc_fbtap_write_md_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	dot11_mdid_ie_t *mdie = (dot11_mdid_ie_t *)data->buf;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	bss_fbt_info_t *fbt_cfg;

	if (!BSSCFG_AP(bsscfg))
		return BCME_OK;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, bsscfg)) {
		fbt_cfg = FBT_BSSCFG_CUBBY(fbt_info, bsscfg);
		memset(mdie, 0, sizeof(dot11_mdid_ie_t));
		/* bcn/prbresp/assocresp/reassocresp */
		mdie->id = DOT11_MNG_MDIE_ID;
		mdie->len = sizeof(mdie->mdid) + sizeof(mdie->cap);
		mdie->mdid = htol16(fbt_cfg->mdid);
		//if (bsscfg->flags & WLC_BSSCFG_ALLOW_FTOVERDS)
		if (fbt_cfg->fbt_over_ds) //MIST changes
			mdie->cap |= FBT_MDID_CAP_OVERDS;
		if (fbt_cfg->fbt_res_req_cap)
			mdie->cap |= FBT_MDID_CAP_RRP;
		/* assocresp/reassocresp */
		if (data->ft == FC_REASSOC_RESP || data->ft == FC_ASSOC_RESP) {
			/* save md ie for MIC calc */
			ftcbparm->assocresp.md_ie = data->buf;
		}
	}

	return BCME_OK;
}

static uint
wlc_fbtap_auth_calc_rsn_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	struct scb *scb = ftcbparm->auth.scb;
	uint ielen = 0;
	wlc_fbt_scb_t *fbt_scb;
	bcm_tlv_t *ie;

	if (!BSSCFG_AP(bsscfg))
		return 0;

	if (!fbt_info)
		return 0;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, bsscfg)) {
		if ((scb != NULL) &&
			(ftcbparm->auth.alg == DOT11_FAST_BSS) &&
			(ftcbparm->auth.seq == 2)) {
			fbt_scb = FBT_SCB(fbt_info, scb);
			if (fbt_scb && fbt_scb->auth_resp_ies) {
				ie = bcm_parse_tlvs(fbt_scb->auth_resp_ies,
					fbt_scb->auth_resp_ielen, DOT11_MNG_RSN_ID);
				if (ie) {
					ielen = ie->len + TLV_HDR_LEN;
				}
			}
		}
	}

	return ielen;
}

static int
wlc_fbtap_auth_write_rsn_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	struct scb *scb = ftcbparm->auth.scb;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	wlc_fbt_scb_t *fbt_scb;
	bcm_tlv_t *ie;

	if (!BSSCFG_AP(bsscfg))
		return BCME_OK;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, bsscfg)) {
		if ((scb != NULL) &&
			(ftcbparm->auth.alg == DOT11_FAST_BSS) &&
			(ftcbparm->auth.seq == 2)) {
			fbt_scb = FBT_SCB(fbt_info, scb);
			if (fbt_scb && fbt_scb->auth_resp_ies) {
				ie = bcm_parse_tlvs(fbt_scb->auth_resp_ies,
					fbt_scb->auth_resp_ielen, DOT11_MNG_RSN_ID);
				if (ie) {
					memcpy(data->buf, ie, (ie->len + TLV_HDR_LEN));
				}
			}
		}
	}

	return BCME_OK;
}

static uint
wlc_fbtap_auth_calc_md_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	struct scb *scb = ftcbparm->auth.scb;
	uint ielen = 0;
	wlc_fbt_scb_t *fbt_scb;
	bcm_tlv_t *ie;

	if (!BSSCFG_AP(bsscfg))
		return 0;

	if (!fbt_info)
		return 0;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, bsscfg)) {
		if ((scb != NULL) &&
			(ftcbparm->auth.alg == DOT11_FAST_BSS) &&
			(ftcbparm->auth.seq == 2)) {
			fbt_scb = FBT_SCB(fbt_info, scb);
			if (fbt_scb && fbt_scb->auth_resp_ies) {
				ie = bcm_parse_tlvs(fbt_scb->auth_resp_ies,
					fbt_scb->auth_resp_ielen, DOT11_MNG_MDIE_ID);
				if (ie) {
					ielen = ie->len + TLV_HDR_LEN;
				}
			}
		}
	}

	return ielen;
}

static int
wlc_fbtap_auth_write_md_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	struct scb *scb = ftcbparm->auth.scb;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	wlc_fbt_scb_t *fbt_scb;
	bcm_tlv_t *ie;

	if (!BSSCFG_AP(bsscfg))
		return BCME_OK;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, bsscfg)) {
		if ((scb != NULL) &&
			(ftcbparm->auth.alg == DOT11_FAST_BSS) &&
			(ftcbparm->auth.seq == 2)) {
			fbt_scb = FBT_SCB(fbt_info, scb);
			if (fbt_scb && fbt_scb->auth_resp_ies) {
				ie = bcm_parse_tlvs(fbt_scb->auth_resp_ies,
					fbt_scb->auth_resp_ielen, DOT11_MNG_MDIE_ID);
				if (BCM_TLV_SIZE(ie) != sizeof(dot11_mdid_ie_t)) {
					return BCME_BADLEN;
				}
				memcpy(data->buf, ie, (ie->len + TLV_HDR_LEN));
			}
		}
	}

	return BCME_OK;
}

static uint
wlc_fbtap_auth_calc_ft_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	struct scb *scb = ftcbparm->auth.scb;
	uint ielen = 0;
	wlc_fbt_scb_t *fbt_scb;
	bcm_tlv_t *ie;

	if (!BSSCFG_AP(bsscfg))
		return 0;

	if (!fbt_info)
		return 0;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, bsscfg)) {
		if ((scb != NULL) &&
			(ftcbparm->auth.alg == DOT11_FAST_BSS) &&
			(ftcbparm->auth.seq == 2)) {
			fbt_scb = FBT_SCB(fbt_info, scb);
			if (fbt_scb && fbt_scb->auth_resp_ies) {
				ie = bcm_parse_tlvs(fbt_scb->auth_resp_ies,
					fbt_scb->auth_resp_ielen, DOT11_MNG_FTIE_ID);
				if (ie) {
					ielen = ie->len + TLV_HDR_LEN;
				}
			}
		}
	}

	return ielen;
}

static int
wlc_fbtap_auth_write_ft_ie(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	struct scb *scb = ftcbparm->auth.scb;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	wlc_fbt_scb_t *fbt_scb;
	bcm_tlv_t *ie;

	if (!BSSCFG_AP(bsscfg))
		return BCME_OK;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, bsscfg)) {
		if ((scb != NULL) &&
			(ftcbparm->auth.alg == DOT11_FAST_BSS) &&
			(ftcbparm->auth.seq == 2)) {
			fbt_scb = FBT_SCB(fbt_info, scb);
			if (fbt_scb && fbt_scb->auth_resp_ies) {
				ie = bcm_parse_tlvs(fbt_scb->auth_resp_ies,
					fbt_scb->auth_resp_ielen, DOT11_MNG_FTIE_ID);
				if (ie) {
					memcpy(data->buf, ie, (ie->len + TLV_HDR_LEN));
				}
			}
		}
	}

	return BCME_OK;
}

static uint
wlc_fbtap_calc_ft_ie_len(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	bss_fbt_info_t *fbt_bsscfg;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	struct scb *scb = ftcbparm->assocresp.scb;
	bool reassoc = (data->ft == FC_REASSOC_RESP);
	uint ielen = 0;
	uint padding, key_len;
	wlc_fbt_scb_t *fbt_scb;
	uint8 key_data[EAPOL_WPA_MAX_KEY_SIZE];

	if (!BSSCFG_AP(bsscfg))
		return 0;

	if (!fbt_info)
		return 0;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (!WLFBT_ENAB(wlc->pub) || !wlc_fbt_enabled(fbt_info, bsscfg) ||
		scb == NULL) {
		return 0;
	}

	fbt_bsscfg = FBT_BSSCFG_CUBBY(fbt_info, bsscfg);

	ASSERT(fbt_bsscfg != NULL);

	fbt_scb = FBT_SCB(fbt_info, scb);

	ASSERT(fbt_scb != NULL);

	if (scb->auth_alg == DOT11_OPEN_SYSTEM) {
		ielen += sizeof(dot11_ft_ie_t);
		/* sub element R1KH_ID */
		ielen += ETHER_ADDR_LEN + TLV_HDR_LEN;
		/* sub element R0KH_ID */
		ielen += fbt_bsscfg->fbt_r0kh_id_len + TLV_HDR_LEN;
		return ielen;
	}

	if (scb->auth_alg == DOT11_FAST_BSS && !reassoc) {
		return 0;
	}

	ielen += sizeof(dot11_ft_ie_t);

	/* copy sub element R1KH_ID */
	ielen += ETHER_ADDR_LEN + TLV_HDR_LEN;

	/* copy sub element GTK. 11A.8.5 802.11r
	The Key field shall be padded before encrypting if the key length is less
	than 16 octets or if it is not a multiple of 8.
	*/

	padding = fbt_scb->gtk.key_len % 8;
	if (padding)
		padding = 8 - padding;
	if (fbt_scb->gtk.key_len + padding < 16)
		padding += 8;

	memset(key_data, 0, sizeof(key_data));
	bcopy(fbt_scb->gtk.key, key_data, fbt_scb->gtk.key_len);
	key_len = fbt_scb->gtk.key_len;

	if (padding) {
		key_data[fbt_scb->gtk.key_len] = 0xdd;
		key_len += padding;
	}

	/* aes wrap increases key size by 8 bytes. */
	ielen += sizeof(uint16) /* key_info */ + 8 /* rsc */ +
		sizeof(uint8) + key_len + 8;
	ielen += TLV_HDR_LEN;

	/* copy sub element R0KH_ID */
	ielen += fbt_bsscfg->fbt_r0kh_id_len + TLV_HDR_LEN;

	if (scb->auth_alg == DOT11_FAST_BSS) {
		if (fbt_scb->fbties_assoc.ricie && fbt_scb->fbties_assoc.ric_len) {
			ielen += fbt_scb->fbties_assoc.ric_len;
		}
	}

	return ielen;
}

static int
wlc_fbtap_write_ft_ie(void *ctx, wlc_iem_build_data_t *data)
{
	bss_fbt_priv_t *fbt_bss_priv;
	wlc_fbt_info_t *fbt_info = (wlc_fbt_info_t *)ctx;
	wlc_bsscfg_t *bsscfg = data->cfg;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	bss_fbt_info_t *fbt_bsscfg;
	wlc_iem_ft_cbparm_t *ftcbparm = data->cbparm->ft;
	struct scb *scb = ftcbparm->assocresp.scb;
	bool reassoc = (data->ft == FC_REASSOC_RESP);
	dot11_ft_ie_t *fbtie = (dot11_ft_ie_t *)data->buf;
	uint8 *cp = data->buf;
	dot11_ft_ie_t *fbtie_scb = NULL;
	uint8 *ptr;
	bcm_tlv_t *tlv;
	uint padding, key_len;
	wlc_fbt_scb_t *fbt_scb;
	uint8 key_data[EAPOL_WPA_MAX_KEY_SIZE];
	uint8 encrypted_key_data[EAPOL_WPA_KEY_DATA_LEN];
	dot11_gtk_ie_t *gtk_ie;
	uint8 *ricdata = NULL;
	uint ric_ie_count = 0, ricdata_len = 0;
	wlc_fbt_ies_t fbties_auth;
	uint8 *tlvs_auth;
	uint tlvs_auth_len;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */
	union {
		int index;
		uint8 rsc[EAPOL_WPA_KEY_RSC_LEN];
	} u;

	if (!BSSCFG_AP(bsscfg))
		return BCME_OK;

	if (!fbt_info)
		return BCME_OK;
	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);
	wlc = fbt_priv->wlc;

	if (!WLFBT_ENAB(wlc->pub) || !wlc_fbt_enabled(fbt_info, bsscfg) ||
		scb == NULL) {
		return BCME_OK;
	}

	fbt_bsscfg = FBT_BSSCFG_CUBBY(fbt_info, bsscfg);

	ASSERT(fbt_bsscfg != NULL);

	fbt_scb = FBT_SCB(fbt_info, scb);

	ASSERT(fbt_scb != NULL);

	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, bsscfg);

	memset(fbtie, 0, sizeof(dot11_ft_ie_t));
	fbtie->id = DOT11_MNG_FTIE_ID;
	if (scb->auth_alg == DOT11_OPEN_SYSTEM) {
		ptr = cp + sizeof(dot11_ft_ie_t);
		tlv = (bcm_tlv_t *)ptr;

		/* copy sub element R1KH_ID */
		tlv->id = DOT11_FBT_SUBELEM_ID_R1KH_ID;
		tlv->len = ETHER_ADDR_LEN;
		bcopy(fbt_bsscfg->fbt_r1kh_id, tlv->data, ETHER_ADDR_LEN);
		ptr += tlv->len + TLV_HDR_LEN;
		tlv = (bcm_tlv_t *)ptr;

		/* copy sub element R0KH_ID */
		tlv->id = DOT11_FBT_SUBELEM_ID_R0KH_ID;
		tlv->len = fbt_bsscfg->fbt_r0kh_id_len;
		bcopy(fbt_bsscfg->fbt_r0kh_id, tlv->data, fbt_bsscfg->fbt_r0kh_id_len);
		ptr += tlv->len + TLV_HDR_LEN;

		fbtie->len = ptr - cp - TLV_HDR_LEN;
		cp += fbtie->len + TLV_HDR_LEN;
		return BCME_OK;
	}

	if (scb->auth_alg == DOT11_FAST_BSS && !reassoc) {
		return BCME_OK;
	}

	tlvs_auth = fbt_scb->auth_resp_ies;
	tlvs_auth_len = fbt_scb->auth_resp_ielen;

	memset(&fbties_auth, 0, sizeof(wlc_fbt_ies_t));
	wlc_fbtap_parse_fbties(wlc, bsscfg, tlvs_auth, tlvs_auth_len, &fbties_auth);

	fbtie_scb = (dot11_ft_ie_t *)fbties_auth.ftie;
	if (fbtie_scb) {
		bcopy(fbtie_scb->anonce, fbtie->anonce, EAPOL_WPA_KEY_NONCE_LEN);
		bcopy(fbtie_scb->snonce, fbtie->snonce, EAPOL_WPA_KEY_NONCE_LEN);
	}
	ptr = cp + sizeof(dot11_ft_ie_t);
	tlv = (bcm_tlv_t *)ptr;

	/* copy sub element R1KH_ID */
	tlv->id = DOT11_FBT_SUBELEM_ID_R1KH_ID;
	tlv->len = ETHER_ADDR_LEN;
	bcopy(fbt_bsscfg->fbt_r1kh_id, tlv->data, ETHER_ADDR_LEN);
	ptr += tlv->len + TLV_HDR_LEN;
	tlv = (bcm_tlv_t *)ptr;

	/* copy sub element R0KH_ID */
	tlv->id = DOT11_FBT_SUBELEM_ID_R0KH_ID;
	tlv->len = fbt_bsscfg->fbt_r0kh_id_len;
	bcopy(fbties_auth.r0kh_id, tlv->data, fbties_auth.r0kh_id_len);
	ptr += tlv->len + TLV_HDR_LEN;

	/* Free fbties_auth */
	wlc_fbtap_free_fbties(wlc, &fbties_auth);

	/* copy sub element GTK. 11A.8.5 802.11r
	The Key field shall be padded before encrypting if the key length is less
	than 16 octets or if it is not a multiple of 8.
	*/

	padding = fbt_scb->gtk.key_len % 8;
	if (padding)
		padding = 8 - padding;
	if (fbt_scb->gtk.key_len + padding < 16)
		padding += 8;

	memset(key_data, 0, sizeof(key_data));
	bcopy(fbt_scb->gtk.key, key_data, fbt_scb->gtk.key_len);
	key_len = fbt_scb->gtk.key_len;

	if (padding) {
		key_data[fbt_scb->gtk.key_len] = 0xdd;
		key_len += padding;
	}

	if (aes_wrap(sizeof(fbt_scb->ptk.kek), fbt_scb->ptk.kek,
		key_len, key_data, encrypted_key_data)) {
		WL_FBT(("wl%d: %s: GTK Key encrypt failed for %s\n",
			wlc->pub->unit, __FUNCTION__,
			bcm_ether_ntoa(&fbt_scb->macaddr, eabuf)));
		return BCME_OK;
	}
	gtk_ie = (dot11_gtk_ie_t *) ptr;

	gtk_ie->id = DOT11_FBT_SUBELEM_ID_GTK;
	/* aes wrap increases key size by 8 bytes. */
	gtk_ie->len = sizeof(gtk_ie->key_info) + sizeof(gtk_ie->rsc) +
		sizeof(gtk_ie->key_len) + key_len + 8;
	htol16_ua_store(fbt_scb->gtk.idx & WPA2_GTK_INDEX_MASK, &(gtk_ie->key_info));
	gtk_ie->key_len = fbt_scb->gtk.key_len;

	u.index = fbt_scb->gtk.idx;
	if (wlc_ioctl(wlc, WLC_GET_KEY_SEQ, &u, sizeof(gtk_ie->rsc), bsscfg->wlcif) != 0) {
		WL_FBT(("wl%d: %s: Failed to get Key Sequence. Setting to 0 \n",
			wlc->pub->unit, __FUNCTION__));
		memset(gtk_ie->rsc, 0, sizeof(gtk_ie->rsc));
	}
	bcopy(u.rsc, gtk_ie->rsc, EAPOL_WPA_KEY_RSC_LEN);
	bcopy(encrypted_key_data, gtk_ie->data, key_len + 8);
	ptr += gtk_ie->len + TLV_HDR_LEN;

	fbtie->len = ptr - cp - TLV_HDR_LEN;
	cp += fbtie->len + TLV_HDR_LEN;

	/* handle RDE IE */
	if (scb->auth_alg == DOT11_FAST_BSS) {
		if (fbt_scb->fbties_assoc.ricie && fbt_scb->fbties_assoc.ric_len) {
			ricdata = cp;
			cp = wlc_fbtap_write_ric_ie(wlc, bsscfg, &fbt_scb->fbties_assoc, scb, cp,
				&ricdata_len, &ric_ie_count);
			if (cp == ricdata || ricdata_len == 0) {
				ricdata = NULL;
			}
		}

		if ((ftcbparm->assocresp.md_ie != NULL) &&
			(ftcbparm->assocresp.wpa2_ie != NULL)) {
			if (!wlc_fbt_ft_calc_mic(fbt_bss_priv, bsscfg, scb, (dot11_ft_ie_t *)fbtie,
				(bcm_tlv_t *)ftcbparm->assocresp.md_ie,
				(bcm_tlv_t *)ftcbparm->assocresp.wpa2_ie,
				ricdata, ricdata_len, ric_ie_count, FT_MIC_REASSOC_RESPONSE_TSN)) {
					return BCME_ERROR;
				}
		}
	}

	return BCME_OK;
}

static uint8 *
wlc_fbtap_write_ric_ie(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg,
	wlc_fbt_ies_t *fbties_assoc, struct scb *scb, uint8 *cp,
	uint *ricdata_len, uint *ric_ie_count)
{
	uint8 *ricptr, *start;
	uint8 *end = NULL;
	uint _ric_ie_count = 0, count = 0;
	bool found = FALSE;
	uint status = DOT11_SC_FAILURE;
	wlc_iem_ft_cbparm_t ftcbparm;
	wlc_iem_cbparm_t cbparm;
	int cp_len = 0;

	/* prepare IE mgmt calls */
	bzero(&ftcbparm, sizeof(ftcbparm));
	ftcbparm.fbtric.ts_count = 0;
	ftcbparm.fbtric.rde_count = 0;
	bzero(&cbparm, sizeof(cbparm));
	cbparm.ft = &ftcbparm;

	start = cp;
	ricptr = fbties_assoc->ricie;
	end = ricptr + fbties_assoc->ric_len;
	while (ricptr < end) {
		bcm_tlv_t *ie = (bcm_tlv_t*)ricptr;
		int ie_len = TLV_HDR_LEN + ie->len;
		ricptr += ie_len;
		if (ie->id == DOT11_MNG_RDE_ID) {
			int i;
			dot11_rde_ie_t *rde = (dot11_rde_ie_t*)ie;

			/* Include RDE in MIC calculations. */
			_ric_ie_count += 1;
			count = rde->rd_count;

			for (i = 0; i < count; i++) {
				bcm_tlv_t *ie = (bcm_tlv_t*)ricptr;
				if (!found) {
					/* if tspec is accepted 1 resource descriptor is sent
						Refer 802.11r 11A.11.3.2 for state machine
					*/
					status = wlc_cac_ap_write_ricdata(wlc, bsscfg, scb,
						(uint8 *)ie, ie->len + TLV_HDR_LEN, &ftcbparm);
					if (!status) {
						ftcbparm.fbtric.rde_id = rde->rde_id;
						/* length of a single Resource request in RIC */
						cp_len = wlc_ier_calc_len(wlc->ier_ric, bsscfg,
							0, NULL);
						if (wlc_ier_build_frame(wlc->ier_ric, bsscfg,
							WLC_IEM_FC_UNK, &cbparm, cp, cp_len)
							!= BCME_OK) {
							WL_ERROR(("wl%d: %s: wlc_ier_build_frame "
							"failed\n", wlc->pub->unit, __FUNCTION__));
							status = DOT11_SC_FAILURE;
							return cp;
						}
						/* Point to the next Resource request in RIC */
						cp += cp_len;
						found = TRUE;

						/* Include protected elements too. */
						_ric_ie_count += 1;
					}
				}
				ricptr += TLV_HDR_LEN + ie->len;
			}

			found = FALSE;
			status = DOT11_SC_FAILURE;
		}
	}
	*ric_ie_count = _ric_ie_count;
	*ricdata_len = cp - start;
	return cp;
}

static int
wlc_fbtap_process_action_resp(wlc_info_t *wlc, wlc_fbt_info_t *fbt_info,
	wlc_bsscfg_t *bsscfg, wlc_fbt_action_resp_t *fbt_action_resp)
{
	int plen;
	void *p;
	struct ether_addr ea;
	uint8 *pbody;
	wlc_txq_info_t *qi;
	struct scb *scb;
	int err = BCME_OK;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	bcopy(fbt_action_resp->macaddr, ea.octet, ETHER_ADDR_LEN);
	scb = wlc_scbfind(wlc, bsscfg, &ea);
	if (scb == NULL) {
		WL_FBT(("wl%d: %s: scb not found to send FBT Action resp %s\n",
			wlc->pub->unit, __FUNCTION__,
			bcm_ether_ntoa((struct ether_addr *)&fbt_action_resp->macaddr, eabuf)));
		err = BCME_BADARG;
		return err;
	}
	/* Allocate FBT Action Response Frame */
	plen = fbt_action_resp->data_len;

	p = wlc_frame_get_mgmt(wlc, FC_ACTION, &ea, &bsscfg->cur_etheraddr,
		&bsscfg->BSSID, plen, (uint8 **) &pbody);
	if (p == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n", wlc->pub->unit,
			__FUNCTION__, MALLOCED(wlc->osh)));
		err = BCME_NOMEM;
		return err;
	}
	memcpy(pbody, &fbt_action_resp->data[0], fbt_action_resp->data_len);

#if defined(BCMDBG)
	if (WL_FBT_ON()) {
		prhex("fbt_resp", pbody, plen);
	}
#endif /* defined(BCMDBG) */
		/*
		  Send using the bsscfg queue the FBT resp will go out on the current
		  channel
		 */
	qi = bsscfg->wlcif->qi;

	if (!wlc_queue_80211_frag(wlc, p, qi, scb, scb->bsscfg, FALSE, NULL, 0)) {
	WL_ERROR(("wl%d: %s: wlc_queue_80211_frag failed\n", wlc->pub->unit, __FUNCTION__));
		return BCME_ERROR;
	}

	return err;
}

void
wlc_fbt_recv_overds_req(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *bsscfg,
	struct dot11_management_header *hdr, void *body, uint body_len)
{
	wlc_info_t *wlc = bsscfg->wlc;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN], *sa = bcm_ether_ntoa(&hdr->sa, eabuf);
#endif // endif
	dot11_ft_req_t *fbt_req = (dot11_ft_req_t *)body;
	struct scb *scb;
	struct ether_addr sta_mac;
	wl_event_fbt_t *pevent;
	uint16 event_len = sizeof(*pevent) + body_len;

	WL_FBT(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	/* Is this request for an AP config or a STA config? */
	if (!BSSCFG_AP(bsscfg)) {
		WL_FBT(("wl%d: %s: FC_ACTION FBT Response: unknown bsscfg _ap %d\n",
		WLCWLUNIT(wlc), __FUNCTION__, bsscfg->_ap));
		return; /* We couldn't match the incoming frame to a BSS config */
	}

	if (!wlc_fbt_fbtoverds_flag(wlc, bsscfg)) {
		WL_FBT(("wl%d: %s FBT Action Req from sta %s FBT over ds not enabled \n",
			wlc->pub->unit, __FUNCTION__, sa));
		return;
	}

	memcpy(sta_mac.octet, fbt_req->sta_addr, ETHER_ADDR_LEN);
	scb = wlc_scbfind(wlc, bsscfg, &sta_mac);

	if (scb == NULL) {
		WL_FBT(("wl%d: %s FBT Action Req from unknown sta %s \n",
			wlc->pub->unit, __FUNCTION__, sa));
		return;
	}

	if (!(pevent = (wl_event_fbt_t *) MALLOCZ(wlc->osh, event_len))) {
		WL_FBT(("wl%d: %s: out of mem, malloced %d bytes\n",
				wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return;
	}

	pevent->version = WL_EVENT_FBT_VER_1;
	pevent->length = event_len;
	pevent->type = WL_E_FBT_TYPE_FBT_OTD_AUTH;
	pevent->data_offset = sizeof(*pevent);

	if (body_len > 0 && body != NULL) {
		memcpy(pevent->data, body, body_len);
	}

	/* Notify application, send all IEs to application */
	wlc_bss_mac_event(wlc, bsscfg, WLC_E_FBT, &scb->ea,
		WLC_E_STATUS_SUCCESS, 0, 0,
		(void *)pevent, event_len);

	MFREE(wlc->osh, pevent, event_len);
}

static uint8 *
wlc_fbt_write_rsn_ie_safe(wlc_info_t *wlc, wlc_bsscfg_t *cfg, uint8 *buf, int buflen)
{
	/* Infrastructure WPA info element */
	uint WPA_len = 0;	/* tag length */
	bcm_tlv_t *wpa2ie = (bcm_tlv_t *)buf;
	wpa_suite_mcast_t *mcast;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *auth;
	uint16 count;
	uint8 *cap;
	uint8 *orig_buf = buf;
	int totlen;
	uint16 WPA_auth = cfg->WPA_auth;
	uint wsec = cfg->wsec;
	int wpacap = 0;

	if (!bcmwpa_includes_rsn_auth(WPA_auth) || ! WSEC_ENABLED(wsec))
		return buf;

	WL_WSEC(("wl%d: %s: adding RSN IE, wsec = 0x%x\n", wlc->pub->unit, __FUNCTION__, wsec));

	/* perform length check */
	/* if buffer too small, return untouched buffer */
	totlen = (int)(&wpa2ie->data[WPA2_VERSION_LEN] - &wpa2ie->id) +
		WPA_SUITE_LEN + WPA_IE_SUITE_COUNT_LEN;
	BUFLEN_CHECK_AND_RETURN(totlen, buflen, orig_buf);
	buflen -= totlen;

	/* fixed portion */
	wpa2ie->id = DOT11_MNG_RSN_ID;
	wpa2ie->data[0] = (uint8)WPA2_VERSION;
	wpa2ie->data[1] = (uint8)(WPA2_VERSION>>8);
	WPA_len = WPA2_VERSION_LEN;

	/* multicast suite */
	mcast = (wpa_suite_mcast_t *)&wpa2ie->data[WPA2_VERSION_LEN];

	bcopy(WPA2_OUI, mcast->oui, DOT11_OUI_LEN);
	mcast->type = wlc_wpa_mcast_cipher(wlc, cfg);

	WPA_len += WPA_SUITE_LEN;

	/* unicast suite list */
	ucast = (wpa_suite_ucast_t *)&mcast[1];
	count = 0;

	WPA_len += WPA_IE_SUITE_COUNT_LEN;

	if (WSEC_AES_ENABLED(wsec)) {
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);

		bcopy(WPA2_OUI, ucast->list[count].oui, DOT11_OUI_LEN);
		ucast->list[count++].type = WPA_CIPHER_AES_CCM;
		WPA_len += WPA_SUITE_LEN;
		buflen -= WPA_SUITE_LEN;
	}
	if (WSEC_TKIP_ENABLED(wsec)) {
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);

		bcopy(WPA2_OUI, ucast->list[count].oui, DOT11_OUI_LEN);
		ucast->list[count++].type = WPA_CIPHER_TKIP;
		WPA_len += WPA_SUITE_LEN;
		buflen -= WPA_SUITE_LEN;
	}
	ASSERT(count);
	ucast->count.low = (uint8)count;
	ucast->count.high = (uint8)(count>>8);

	/* authenticated key management suite list */
	auth = (wpa_suite_auth_key_mgmt_t *)&ucast->list[count];
	count = 0;

	/* length check */
	/* if buffer too small, return untouched buffer */
	BUFLEN_CHECK_AND_RETURN(WPA_IE_SUITE_COUNT_LEN, buflen, orig_buf);

	WPA_len += WPA_IE_SUITE_COUNT_LEN;
	buflen -= WPA_IE_SUITE_COUNT_LEN;

	if (WPA_auth & WPA2_AUTH_UNSPECIFIED) {
		if (BSSCFG_IS_FBT_1X(cfg)) {
			/* length check */
			/* if buffer too small, return untouched buffer */
			BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
			bcopy(WPA2_OUI, auth->list[count].oui, DOT11_OUI_LEN);
			auth->list[count++].type = RSN_AKM_FBT_1X;
			WPA_len += WPA_SUITE_LEN;
			buflen -= WPA_SUITE_LEN;
		}
	}
	if (WPA_auth & WPA2_AUTH_PSK) { /* always publish PSK */
		/* length check */
		/* if buffer too small, return untouched buffer */
		BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
		bcopy(WPA2_OUI, auth->list[count].oui, DOT11_OUI_LEN);
		auth->list[count++].type = RSN_AKM_PSK;
		WPA_len += WPA_SUITE_LEN;
		buflen -= WPA_SUITE_LEN;
		if (BSSCFG_IS_FBT_PSK(cfg)) { /* additionally publish FT-PSK */
			/* length check */
			/* if buffer too small, return untouched buffer */
			BUFLEN_CHECK_AND_RETURN(WPA_SUITE_LEN, buflen, orig_buf);
			bcopy(WPA2_OUI, auth->list[count].oui, DOT11_OUI_LEN);
			auth->list[count++].type = RSN_AKM_FBT_PSK;
			WPA_len += WPA_SUITE_LEN;
			buflen -= WPA_SUITE_LEN;
		}
	}

	ASSERT(count);
	auth->count.low = (uint8)count;
	auth->count.high = (uint8)(count>>8);

	/* WPA capabilities */
	cap = (uint8 *)&auth->list[count];
	/* length check */
	/* if buffer too small, return untouched buffer */
	BUFLEN_CHECK_AND_RETURN(WPA_CAP_LEN, buflen, orig_buf);
	wlc_iovar_op(wlc, "wpa_cap", NULL, 0, &wpacap, sizeof(wpacap), IOV_GET, cfg->wlcif);
	wpacap = htol16(wpacap);
	memcpy(cap, &wpacap, WPA_CAP_LEN);

	WPA_len += WPA_CAP_LEN;
	buflen -= WPA_CAP_LEN;

	/* update tag length */
	wpa2ie->len = (uint8)WPA_len;

	if (WPA_len)
		buf += TLV_HDR_LEN + WPA_len;

	return (buf);
}

static void wlc_fbtap_scb_state_upd(wlc_fbt_info_t *fbt_info,
        scb_state_upd_data_t *data)
{
	wlc_bsscfg_t *bsscfg;
	scb_t *scb;
	wlc_fbt_priv_t *fbt_priv;
	wlc_info_t *wlc;
	wlc_fbt_scb_t *fbt_scb = NULL;

	scb = data->scb;

	ASSERT(scb != NULL);

	bsscfg = SCB_BSSCFG(scb);

	if (!BSSCFG_AP(bsscfg))
		return;

	fbt_priv = WLC_FBT_PRIV_INFO(fbt_info);

	wlc = fbt_priv->wlc;

	fbt_scb = FBT_SCB(wlc->fbt, scb);

	if (fbt_scb == NULL)
		return;
	if (WLFBT_ENAB(wlc->pub) && wlc_fbt_enabled(fbt_info, bsscfg)) {
		if ((scb != NULL) &&
			(scb->auth_alg == DOT11_FAST_BSS && fbt_scb) &&
			(SCB_ASSOCIATED(scb))) {
			wlc_ioctl(wlc, WLC_SCB_AUTHORIZE, &(fbt_scb->macaddr),
					ETHER_ADDR_LEN, bsscfg->wlcif);
		}
	}

	return;
}
#endif /* AP */

#if defined(STA) && defined(FBT_STA)
/* This function is to update FTIE in M2 appropriately if AP initiates 4-way handshake
 * after PTK timeout following a fast transition.
 */
static bool
wlc_fbt_update_ftie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_priv_t *fbt_bss_priv;
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
#if defined(FT_REKEY_FTIE_REORDER_WAR)
	uint8 *r0khid, *r1khid;
	uint16 r0khid_len, r1khid_len;
#endif /* FT_REKEY_FTIE_REORDER_WAR */
	uint8 *pbody;

	if (!fbt_bss)
		return FALSE;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);
	pbody = fbt_bss_priv->ftie;

	/* If M1 comes after fast transition, MIC, Snonce and Anonce in FTIE are non-zero.
	 * So clear out the fields.
	 */
	pbody += TLV_HDR_LEN;
	memset(pbody, 0, sizeof(dot11_ft_ie_t) - TLV_HDR_LEN);

#if defined(FT_REKEY_FTIE_REORDER_WAR)
	/* WAR: 802.11r spec does not specify the order in which R0KH-ID and R1KH-ID should be
	 * present in FT IE. However some APs expect the order to be R0KH-ID followed by R1KH-ID
	 * and report invalid FTIE in M2 error otherwise. Forcing the order as R0KHID before R1KHID.
	 * The WAR can be removed when the issue is fixed on the AP side.
	 */
	if (fbt_bss_priv->r0khid == NULL || fbt_bss_priv->r1khid == NULL)
		return FALSE;

	r0khid_len = fbt_bss_priv->r0khid->len + TLV_HDR_LEN;
	r0khid = MALLOC(fbt_bss_priv->osh, r0khid_len);
	if (!r0khid) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		   UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
		return FALSE;
	}
	r1khid_len = fbt_bss_priv->r1khid->len + TLV_HDR_LEN;
	r1khid = MALLOC(fbt_bss_priv->osh, r1khid_len);
	if (!r1khid) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		   UNIT(fbt_bss_priv), __FUNCTION__, MALLOCED(fbt_bss_priv->osh)));
		MFREE(fbt_bss_priv->osh, r0khid, r0khid_len);
		return FALSE;
	}

	memcpy(r0khid, fbt_bss_priv->r0khid, r0khid_len);
	memcpy(r1khid, fbt_bss_priv->r1khid, r1khid_len);

	pbody += sizeof(dot11_ft_ie_t) - TLV_HDR_LEN;
	fbt_bss_priv->r0khid = (bcm_tlv_t *)pbody;
	memcpy(pbody, r0khid, r0khid_len);

	pbody += r0khid_len;
	fbt_bss_priv->r1khid = (bcm_tlv_t *)pbody;
	memcpy(pbody, r1khid, r1khid_len);

	MFREE(fbt_bss_priv->osh, r0khid, r0khid_len);
	MFREE(fbt_bss_priv->osh, r1khid, r1khid_len);
#endif /* FT_REKEY_FTIE_REORDER_WAR */
	return TRUE;
}
#endif /* STA && FBT_STA */

#ifdef WLRSDB
static int
wlc_fbt_info_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len)
{
	wlc_fbt_info_copy_t *cp = (wlc_fbt_info_copy_t *)data;
	cp->cfg = cfg;
	return BCME_OK;
}

static int
wlc_fbt_info_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len)
{
	wlc_fbt_info_copy_t *cp = (wlc_fbt_info_copy_t *)data;
	wlc_bsscfg_t *from_cfg, *to_cfg;

	from_cfg = cp->cfg;
	to_cfg = cfg;

	if (!from_cfg || !to_cfg)
		return BCME_ERROR;

	if (FBT_BSSCFG_CUBBY(from_cfg->wlc->fbt, from_cfg)) {
		bss_fbt_priv_t *from_fbt_bss_priv = NULL;
		bss_fbt_priv_t *to_fbt_bss_priv = NULL;

		from_fbt_bss_priv = wlc_fbt_get_bsscfg(from_cfg->wlc->fbt, from_cfg);
		if (!from_fbt_bss_priv)
			return BCME_ERROR;
		to_fbt_bss_priv = wlc_fbt_get_bsscfg(to_cfg->wlc->fbt, to_cfg);
		if (!to_fbt_bss_priv)
			return BCME_ERROR;
		to_fbt_bss_priv->use_sup_wpa  = from_fbt_bss_priv->use_sup_wpa;
		wlc_fbt_bsscfg_init(to_cfg->wlc->fbt, to_cfg);

		if (FBT_BSSCFG_CUBBY(to_cfg->wlc->fbt, to_cfg)) {
			/* clone bss_fbt_pub */
			bss_fbt_info_t *from_fbt_bss =
			FBT_BSSCFG_CUBBY(from_cfg->wlc->fbt, from_cfg);
			bss_fbt_info_t *to_fbt_bss =
				FBT_BSSCFG_CUBBY(to_cfg->wlc->fbt, to_cfg);

			if (!from_fbt_bss || !to_fbt_bss) {
				WL_ERROR(("FBT BSSCFG cubby not found\n"));
				return BCME_ERROR;
			}

			to_fbt_bss->bss_pub.ini_fbt =
				from_fbt_bss->bss_pub.ini_fbt;

			/* Clone bss_fbt_priv  */
			memcpy(to_fbt_bss_priv->wpa, from_fbt_bss_priv->wpa, sizeof(wpapsk_t));
			if (from_fbt_bss_priv->wpa->auth_wpaie) {
				to_fbt_bss_priv->wpa->auth_wpaie =
				(uchar *)MALLOCZ(to_fbt_bss_priv->osh,
				from_fbt_bss_priv->wpa->auth_wpaie_len);

				if (to_fbt_bss_priv->wpa->auth_wpaie != NULL)
					memcpy(to_fbt_bss_priv->wpa->auth_wpaie,
					from_fbt_bss_priv->wpa->auth_wpaie,
					from_fbt_bss_priv->wpa->auth_wpaie_len);
			}
			if (from_fbt_bss_priv->wpa->sup_wpaie) {
				to_fbt_bss_priv->wpa->sup_wpaie =
				(uchar *)MALLOCZ(to_fbt_bss_priv->osh,
				from_fbt_bss_priv->wpa->sup_wpaie_len);

				if (to_fbt_bss_priv->wpa->sup_wpaie != NULL)
					memcpy(to_fbt_bss_priv->wpa->sup_wpaie,
					from_fbt_bss_priv->wpa->sup_wpaie,
					from_fbt_bss_priv->wpa->sup_wpaie_len);
			}

			memcpy(&to_fbt_bss_priv->peer_ea,
				&from_fbt_bss_priv->peer_ea, sizeof(struct ether_addr));

			to_fbt_bss_priv->mdie_len  = from_fbt_bss_priv->mdie_len;
			if (from_fbt_bss_priv->mdie_len) {
				to_fbt_bss_priv->mdie =
				(uchar *)MALLOCZ(to_fbt_bss_priv->osh,
				from_fbt_bss_priv->mdie_len);

				if (to_fbt_bss_priv->mdie != NULL)
					memcpy(to_fbt_bss_priv->mdie, from_fbt_bss_priv->mdie,
						from_fbt_bss_priv->mdie_len);

			}
			to_fbt_bss_priv->ftie_len  = from_fbt_bss_priv->ftie_len;
			if (from_fbt_bss_priv->ftie_len) {
				to_fbt_bss_priv->ftie =
				(uchar *)MALLOCZ(to_fbt_bss_priv->osh,
				from_fbt_bss_priv->ftie_len);
				if (to_fbt_bss_priv->ftie != NULL)
					memcpy(to_fbt_bss_priv->ftie, from_fbt_bss_priv->ftie,
					from_fbt_bss_priv->ftie_len);

			}
			if (from_fbt_bss_priv->r0khid && from_fbt_bss_priv->r1khid) {
				uchar *tlvs;
				uint tlv_len;
				tlvs = (uchar *)((uintptr)to_fbt_bss_priv->ftie +
					sizeof(dot11_ft_ie_t));
				tlv_len = to_fbt_bss_priv->ftie_len - sizeof(dot11_ft_ie_t);
				to_fbt_bss_priv->r0khid = bcm_parse_tlvs(tlvs, tlv_len, 3);
				to_fbt_bss_priv->r1khid = bcm_parse_tlvs(tlvs, tlv_len, 1);
			}

			memcpy(to_fbt_bss_priv->pmkr0name, from_fbt_bss_priv->pmkr0name,
				WPA2_PMKID_LEN);
			memcpy(to_fbt_bss_priv->pmkr1name, from_fbt_bss_priv->pmkr1name,
				WPA2_PMKID_LEN);
			to_fbt_bss_priv->current_akm_type  = from_fbt_bss_priv->current_akm_type;
			to_fbt_bss_priv->use_sup_wpa  = from_fbt_bss_priv->use_sup_wpa;
		}
	}
	return BCME_OK;
}
#endif /* WLRSDB */

#if defined(STA) && defined(FBT_STA)
static void
wlc_fbt_upd_authie(wlc_fbt_info_t *fbt_info, wlc_bsscfg_t *cfg)
{
	bss_fbt_info_t *fbt_bss = FBT_BSSCFG_CUBBY(fbt_info, cfg);
	bss_fbt_priv_t *fbt_bss_priv;
	bcm_tlv_t *wpa2ie;
	wpa_suite_ucast_t *ucast;
	wpa_suite_auth_key_mgmt_t *auth;
	uint16 ucast_count, akm_count;
	ccx_ext_cap_ie_t *ccx_ext_cap_ie;
	uint8 type = CCX_EXT_CAP_IE_TYPE;
	uint8 *auth_ies, *sup_ies;
	uint auth_ies_len, sup_ies_len;
	uint i;
	int len;

	if (!fbt_bss)
		return;
	fbt_bss_priv = FBT_BSSCFG_CUBBY_PRIV(fbt_info, cfg);

	/* Get the beacon IEs */
	wlc_find_sup_auth_ies(fbt_bss_priv->wlc->idsup, cfg, &sup_ies, &sup_ies_len,
			&auth_ies, &auth_ies_len);

	/* Get the CCX extended capability IE from the beacon */
	ccx_ext_cap_ie = (ccx_ext_cap_ie_t *)bcm_find_vendor_ie(auth_ies, auth_ies_len,
		CISCO_AIRONET_OUI, &type, sizeof(type));

	/* If FT adaptive mode is enabled, update the non-FT AKM suite to FT in beacon RSN IE */
	if (ccx_ext_cap_ie && (ccx_ext_cap_ie->cap & CCX_CAP_FBT)) {
		WL_WSEC(("wl%d: FBT adaptive mode, update supplicant auth_ie \n",
				WLCWLUNIT(fbt_bss_priv->wlc)));

		/* Update fbt_bss_priv->wpa->auth_wpaie which points to suppicant auth_ie */
		wpa2ie = (bcm_tlv_t *)fbt_bss_priv->wpa->auth_wpaie;
		len = wpa2ie->len;

		/* length for mcast suite */
		len -= (WPA2_VERSION_LEN + WPA_SUITE_LEN);

		/* Check for unicast suite(s) */
		if (len < WPA_IE_SUITE_COUNT_LEN) {
			WL_INFORM(("wl%d: no unicast suite\n", fbt_bss_priv->wlc->pub->unit));
			return;
		}
		/* unicast suite list */
		ucast = (wpa_suite_ucast_t *)&wpa2ie->data[WPA2_VERSION_LEN +
			sizeof(wpa_suite_mcast_t)];
		ucast_count = ltoh16_ua(&ucast->count);
		len -= (WPA_IE_SUITE_COUNT_LEN + (ucast_count * WPA_SUITE_LEN));

		if (len < 0) {
			WL_INFORM(("wl%d: Bad length for ucast suite\n",
			   fbt_bss_priv->wlc->pub->unit));
			return;
		}
		/* Check for auth key management suite(s) */
		if (len < WPA_IE_SUITE_COUNT_LEN) {
			WL_INFORM(("wl%d: no auth key mgmt suite\n", fbt_bss_priv->wlc->pub->unit));
			return;
		}
		/* authenticated key management suite list */
		auth = (wpa_suite_auth_key_mgmt_t *)&ucast->list[ucast_count];
		akm_count = ltoh16_ua(&auth->count);
		len -= WPA_IE_SUITE_COUNT_LEN;
		for (i = 0; i < akm_count && len >= WPA_SUITE_LEN;
			 i++, len -= WPA_SUITE_LEN) {
			if (!bcmp(auth->list[i].oui, WPA2_OUI, DOT11_OUI_LEN)) {
				if (auth->list[i].type == RSN_AKM_UNSPECIFIED) {
					auth->list[i].type = RSN_AKM_FBT_1X;
				}
				else if (auth->list[i].type == RSN_AKM_PSK) {
					auth->list[i].type = RSN_AKM_FBT_PSK;
				}
			}
		}
	}
}
#endif /* STA && FBT_STA */
