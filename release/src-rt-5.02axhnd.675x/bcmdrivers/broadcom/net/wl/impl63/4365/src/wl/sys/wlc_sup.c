/*
 * wlc_sup.c -- driver-resident supplicants.
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
 * $Id: wlc_sup.c 782389 2019-12-18 06:56:56Z $
 */

/**
 * @file
 * @brief
 * Internal WPA supplicant
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlanSwArchitectureIdsup]
 */

#ifdef BCMINTSUP
#include <wlc_cfg.h>
#endif /* BCMINTSUP */

#ifndef	STA
#error "STA must be defined for wlc_sup.c"
#endif /* STA */
#if !defined(BCMCCX) && !defined(BCMSUP_PSK) && !defined(WLFBT)
#error "BCMCCX and/or BCMSUP_PSK and/or WLFBT must be defined"
#endif /* !defined(BCMCCX) && !defined(BCMSUP_PSK) && !defined(WLFBT) */

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
#if defined(BCMSUP_PSK) || defined(WLFBT)
#include <bcmcrypto/prf.h>
#endif /* BCMSUP_PSK || WLFBT */
#ifdef	BCMSUP_PSK
#include <bcmcrypto/passhash.h>
#include <bcmcrypto/sha1.h>
#endif /* BCMSUP_PSK */
#include <proto/802.11.h>
#ifdef	BCMCCX
#include <proto/802.11_ccx.h>
#endif	/* BCMCCX */
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_led.h>
#include <wlc_rm.h>
#include <wlc_assoc.h>
#ifdef BCMCCX
#include <wlc_ccx.h>
#endif // endif
#include <wl_export.h>
#include <wlc_scb.h>
#if defined(BCMSUP_PSK) || defined(BCMCCX) || defined(WLFBT)
#include <wlc_wpa.h>
#endif /* BCMSUP_PSK || BCMCCX || WLFBT */
#include <wlc_sup.h>
#include <wlc_pmkid.h>
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
#ifdef BCMEXTCCX
#include <wlc_ccx.h>
#include <proto/802.11_ccx.h>
#endif // endif
#include "wpaif.h"
#include "wlc_sup.h"
#include "wlc_wpa.h"
#endif /* BCMINTSUP */
#if defined(WLFBT)
#include "wlc_fbt.h"
#endif // endif
#if defined(BCMCCX) && defined(BCMINTSUP)
#include <wlc_sup_ccx.h>
#endif // endif

#include <proto/802.11.h>
#include <wlc_keymgmt.h>

#ifdef MFP
#include <wlc_mfp.h>
#define SUPMFP(s) ((s)->wlc->mfp)
#endif // endif

#ifdef WLOFFLD
#include <bcm_ol_msg.h>
#endif // endif

#define SUP_CHECK_MCIPHER(sup) ((sup->wpa->mcipher != cipher) &&	\
	bcmwpa_is_wpa_auth(sup->wpa->WPA_auth))
#define SUP_CHECK_EAPOL(body) (body->type == EAPOL_WPA_KEY || body->type == EAPOL_WPA2_KEY)

#ifdef BCMCCX
#define SUP_CHECK_WPAPSK_SUP_TYPE(sup) (sup->sup_type == SUP_WPAPSK || \
	sup->sup_type == SUP_LEAP_WPA)
#else
#define SUP_CHECK_WPAPSK_SUP_TYPE(sup) (sup->sup_type == SUP_WPAPSK)
#endif // endif

#if defined(BCMCCX)
#ifdef BCMSUP_PSK
#define SUP_CHECK_AUTH_SUP_TYPE(sup) (sup->sup_type == SUP_LEAP || sup->sup_type == SUP_LEAP_WPA)
#else
#define SUP_CHECK_AUTH_SUP_TYPE(sup) (sup->sup_type == SUP_LEAP)
#endif /* BCMSUP_PSK */
#endif /* defined(BCMCCX) */

#if defined(BCMCCX)
#ifdef BCMSUP_PSK
#define CCX_CHECK_AUTH_SUP_TYPE(sup_type) (sup_type == SUP_LEAP || sup_type == SUP_LEAP_WPA)
#else
#define CCX_CHECK_AUTH_SUP_TYPE(sup_type) (sup_type == SUP_LEAP)
#endif /* BCMSUP_PSK */
#endif /* defined(BCMCCX) */

static int wlc_sup_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif);

enum {
	IOV_SUP_AUTH_STATUS,
	IOV_SUP_AUTH_STATUS_EXT,	/* Extended supplicant authentication status */
	IOV_SUP_M3SEC_OK,
	IOV_SUP_WPA,
	IOV_SUP_WPA2_EAPVER,
	IOV_SUP_WPA_TMO,
	IOV_LAST
};

static const bcm_iovar_t sup_iovars[] = {
	{"sup_auth_status", IOV_SUP_AUTH_STATUS,
	(0), IOVT_UINT32, 0
	},
	{"sup_auth_status_ext", IOV_SUP_AUTH_STATUS_EXT,
	(0), IOVT_UINT32, 0
	},
	{"sup_m3sec_ok", IOV_SUP_M3SEC_OK,
	(IOVF_RSDB_SET), IOVT_BOOL, 0
	},
	{"sup_wpa", IOV_SUP_WPA,
	(0), IOVT_BOOL, 0
	},
	{"sup_wpa2_eapver", IOV_SUP_WPA2_EAPVER,
	(IOVF_RSDB_SET), IOVT_BOOL, 0
	},
	{"sup_wpa_tmo", IOV_SUP_WPA_TMO,
	(0), IOVT_UINT32, 0
	},
	{NULL, 0, 0, 0, 0}
};

typedef struct {
	/* references to driver `common' things */
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_pub_t *pub;			/* pointer to wlc public portion */
	void *wl;			/* per-port handle */
	osl_t *osh;			/* PKT* stuff wants this */
	uint16 bss_sup_priv_offset;	/* offset of priv cubby in bsscfg */
	bool	sup_m3sec_ok;		/* to selectively allow incorrect bit in M3 */
	int sup_wpa2_eapver;	/* for choosing eapol version in M2 */
	bool	updn;			/* bsscfg up/down callbackk registered */
	bcm_notif_h			sup_up_down_notif_hdl; /* init notifier handle. */
} wlc_sup_priv_t;

/* skeletion structure since supplicant_t hangs off the bsscfg, rather than wlc */
struct wlc_sup_info {
	wlc_sup_pub_t mod_pub;
	wlc_sup_priv_t mod_priv;
};

/* wlc_sup_info_priv_t offset in module states */
static uint16 wlc_sup_info_priv_offset = OFFSETOF(wlc_sup_info_t, mod_priv);

#define WLC_SUP_PRIV_INFO(sup_info)		((wlc_sup_priv_t *) \
	((uint8 *)sup_info + wlc_sup_info_priv_offset))

/* Supplicant top-level structure hanging off bsscfg */
struct supplicant {
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_pub_t *pub;			/* pointer to wlc public portion */
	void *wl;			/* per-port handle */
	osl_t *osh;			/* PKT* stuff wants this */
	wlc_bsscfg_t *cfg;		/* pointer to sup's bsscfg */
	wlc_sup_info_t *m_handle;	/* module handle */

	struct ether_addr peer_ea;      /* peer's ea */

#if defined(BCMSUP_PSK)
	wpapsk_t *wpa;			/* volatile, initialized in set_sup */
	wpapsk_info_t *wpa_info;		/* persistent wpa related info */
	unsigned char ap_eapver;	/* eapol version from ap */
#endif	/* BCMSUP_PSK */

#if defined(BCMSUP_PSK)
	uint32 wpa_psk_tmo; /* 4-way handshake timeout */
	uint32 wpa_psk_timer_active;    /* 4-way handshake timer active */
	struct wl_timer *wpa_psk_timer; /* timer for 4-way handshake */
#endif	/* BCMSUP_PSK */
	uint		npmkid_sup;
	sup_pmkid_t pmkid_sup[SUP_MAXPMKID];
};

typedef struct supplicant supplicant_t;

struct bss_sup_info {
	sup_bss_pub_t bss_pub;
	supplicant_t bss_priv;
};
typedef struct bss_sup_info bss_sup_info_t;

#define SUP_BSSCFG_CUBBY_LOC(sup, cfg) ((bss_sup_info_t **)BSSCFG_CUBBY(cfg, (sup)->mod_pub.cfgh))
#define SUP_BSSCFG_CUBBY(sup, cfg) (*SUP_BSSCFG_CUBBY_LOC(sup, cfg))
#define BSS_PRIV_OFFSET(sup)	((WLC_SUP_PRIV_INFO(sup))->bss_sup_priv_offset)
#define SUP_BSSCFG_CUBBY_PRIV(sup, cfg) ((supplicant_t *)((uint8 *) \
	(*SUP_BSSCFG_CUBBY_LOC(sup, cfg)) + BSS_PRIV_OFFSET(sup)))
#define SUP_BSSCFG_CUBBY_PUB(sup, cfg) ((sup_bss_pub_t *)SUP_BSSCFG_CUBBY(sup, cfg))

/* Simplify maintenance of references to driver `common' items. */
#define UNIT(ptr)	((ptr)->pub->unit)
#define CUR_EA(ptr)	(((supplicant_t *)ptr)->cfg->cur_etheraddr)
#define PEER_EA(ptr)	(((supplicant_t *)ptr)->peer_ea)
#define BSS_EA(ptr)	(((supplicant_t *)ptr)->cfg->BSSID)
#define BSS_SSID(ptr)	(((supplicant_t *)ptr)->cfg->current_bss->SSID)
#define BSS_SSID_LEN(ptr)	(((supplicant_t *)ptr)->cfg->current_bss->SSID_len)
#define OSH(ptr)	((ptr)->osh)

static void wlc_sup_bss_updn(void *ctx, bsscfg_up_down_event_data_t *evt);

static bool
wlc_sup_retrieve_pmk(supplicant_t *sup, wlc_bsscfg_t *cfg, uint8 *data,
	struct ether_addr *bssid, uint8 *pmk, uint8 pmklen);

static uint32 wlc_sup_get_wpa_psk_tmo(struct supplicant *sup);
static void wlc_sup_set_wpa_psk_tmo(struct supplicant *sup, uint32 tmo);
/* Return the supplicant authentication status */
static sup_auth_status_t wlc_sup_get_auth_status(struct supplicant *sup);

/* Return the extended supplicant authentication status */
static sup_auth_status_t wlc_sup_get_auth_status_extended(struct supplicant *sup);

/* Initiate supplicant private context */
int wlc_sup_init(void *ctx, wlc_bsscfg_t *cfg);

/* Remove supplicant private context */
static void wlc_sup_deinit(void *ctx, wlc_bsscfg_t *cfg);
static
void wlc_sup_handle_joinproc(wlc_sup_info_t *sup_info, bss_assoc_state_data_t *evt_data);
void wlc_sup_handle_joinstart(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg);
void wlc_sup_handle_joindone(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg);

/* bsscfg cubby get/set config */
static uint16 wlc_sup_cubby_config_offset = OFFSETOF(wpapsk_info_t, psk_len);
#define SUP_CUBBY_CONFIG_DATA(wpainfo)  ((uint8*)(wpainfo) + wlc_sup_cubby_config_offset)
#define SUP_CUBBY_CONFIG_SIZE		(sizeof(wpapsk_info_t) - wlc_sup_cubby_config_offset)
static int wlc_sup_config_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len);
static int wlc_sup_config_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len);

#ifdef BCMSUP_PSK
static void wlc_wpa_psk_timer(void *arg);
static void *wlc_wpa_sup_prepeapol(supplicant_t *sup, uint16 flags, wpa_msg_t msg);
#endif // endif

/* Allocate supplicant context, squirrel away the passed values,
 * and return the context handle.
 */

wlc_sup_info_t *
BCMATTACHFN(wlc_sup_attach)(wlc_info_t *wlc)
{
	wlc_sup_info_t *sup_info;
	wlc_sup_priv_t	*sup_priv;

	WL_TRACE(("wl%d: wlc_sup_attach\n", wlc->pub->unit));

	if (!(sup_info = (wlc_sup_info_t *)MALLOCZ(wlc->osh, sizeof(wlc_sup_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	sup_priv = WLC_SUP_PRIV_INFO(sup_info);
	sup_priv->wlc = wlc;
	sup_priv->pub = wlc->pub;
	sup_priv->wl = wlc->wl;
	sup_priv->osh = wlc->osh;
	sup_priv->bss_sup_priv_offset = OFFSETOF(bss_sup_info_t, bss_priv);

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((sup_info->mod_pub.cfgh = wlc_bsscfg_cubby_reserve_ext(wlc, sizeof(bss_sup_info_t *),
		NULL /* wlc_sup_init */, wlc_sup_deinit, NULL,
		SUP_CUBBY_CONFIG_SIZE, wlc_sup_config_get, wlc_sup_config_set,
		(void *)sup_info)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          UNIT(sup_priv), __FUNCTION__));
		goto err;
	}

	/* bsscfg up/down callback */
	if (wlc_bsscfg_updown_register(wlc, wlc_sup_bss_updn, sup_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
		          UNIT(sup_priv), __FUNCTION__));
		goto err;
	}
	sup_priv->updn = TRUE;

	/* register assoc state notification callback */
	if (wlc_bss_assoc_state_register(wlc, (bss_assoc_state_fn_t)wlc_sup_handle_joinproc,
		sup_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bss_assoc_state_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* notifications done from idsup module */
	if (bcm_notif_create_list(wlc->notif, &sup_priv->sup_up_down_notif_hdl) != BCME_OK) {
		WL_ERROR(("wl%d: %s: bcm_notif_create_list sup_up_down_notif_hdl\n",
			wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	/* XXX Supplicant hangs off the bsscfg, rather than off wlc, cannot allocate/deallocate
	 * anything since the creation/deletion has to happen during bsscfg init/deinit
	 */

	/* register module */
	if (wlc_module_register(wlc->pub, sup_iovars, "idsup", sup_info, wlc_sup_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: auth wlc_module_register() failed\n", UNIT(sup_priv)));
		goto err;
	}
	return sup_info;
err:
	wlc_sup_detach(sup_info);
	return NULL;
}

/* Toss supplicant context */
void
BCMATTACHFN(wlc_sup_detach)(wlc_sup_info_t *sup_info)
{
	wlc_sup_priv_t *sup_priv;
	if (!sup_info)
		return;

	sup_priv = WLC_SUP_PRIV_INFO(sup_info);

	WL_TRACE(("wl%d: wlc_sup_detach\n", UNIT(sup_priv)));

	/* assoc join-start/done callback */
	if (wlc_bss_assoc_state_unregister(sup_priv->wlc,
		(bss_assoc_state_fn_t)wlc_sup_handle_joinproc, sup_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bss_assoc_state_unregister() failed\n",
		          UNIT(sup_priv), __FUNCTION__));
	}

	/* Delete event notification list for join start/done events. */
	if (sup_priv->sup_up_down_notif_hdl != NULL)
		bcm_notif_delete_list(&sup_priv->sup_up_down_notif_hdl);

	if (sup_priv->updn) {
		wlc_bsscfg_updown_unregister(sup_priv->wlc, wlc_sup_bss_updn, sup_info);
	}
	wlc_module_unregister(sup_priv->pub, "idsup", sup_info);
	MFREE(sup_priv->osh, sup_info, sizeof(wlc_sup_info_t));
}

static void
wlc_sup_bss_updn(void *ctx, bsscfg_up_down_event_data_t *evt)
{
	wlc_sup_info_t *sup_info = (wlc_sup_info_t *)ctx;

	if (!evt->up) {
		wlc_sup_down(sup_info, evt->bsscfg);
	}
}

static int
wlc_sup_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_sup_info_t *sup_info = (wlc_sup_info_t *)handle;
	wlc_sup_priv_t *sup_priv = WLC_SUP_PRIV_INFO(sup_info);
	supplicant_t *sup;
	bss_sup_info_t *sup_bss;
	sup_bss_pub_t	*supbsspub;
	wlc_info_t *wlc = sup_priv->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
	int32 *ret_int_ptr;
	bool bool_val, unhandled = FALSE;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);
	if (BSSCFG_AP(bsscfg)) {
		err = BCME_NOTSTA;
		goto exit;
	}
	sup_bss =  SUP_BSSCFG_CUBBY(sup_info, bsscfg);
	sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, bsscfg);
	supbsspub = (SUP_BSSCFG_CUBBY_PUB(sup_info, bsscfg));
	/* convenience int and bool vals for first 8 bytes of buffer */
	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0) ? TRUE : FALSE;

	switch (actionid) {
		case IOV_GVAL(IOV_SUP_M3SEC_OK):
			*ret_int_ptr = (int32)sup_priv->sup_m3sec_ok;
			break;
		case IOV_SVAL(IOV_SUP_M3SEC_OK):
			sup_priv->sup_m3sec_ok = bool_val;
			break;
		case IOV_GVAL(IOV_SUP_WPA2_EAPVER):
			*ret_int_ptr = (int32)sup_priv->sup_wpa2_eapver;
			break;
		case IOV_SVAL(IOV_SUP_WPA2_EAPVER):
			sup_priv->sup_wpa2_eapver = int_val;
			break;
		case IOV_GVAL(IOV_SUP_WPA):
			if (sup_bss)
				*ret_int_ptr = (int32)supbsspub->sup_enable_wpa;
			else
				*ret_int_ptr = 0;
			break;
		case IOV_SVAL(IOV_SUP_WPA):
			if (bool_val) {
				if (!sup_bss)
					err = wlc_sup_init(sup_info, bsscfg);
			}
			else {
				if (sup_bss) {
					wlc_ioctl(wlc, WLC_DISASSOC, NULL, 0, wlcif);
					wlc_sup_deinit(sup_info, bsscfg);
				}
			}
			break;
		default:
			unhandled  = TRUE;
			break;
	}

	if (unhandled) {
		if (!sup_bss) {
			err = BCME_NOTREADY;
			return err;
		}
		switch (actionid) {
			case IOV_GVAL(IOV_SUP_AUTH_STATUS):
				*((uint32 *)arg) = wlc_sup_get_auth_status(sup);
				break;
			case IOV_GVAL(IOV_SUP_AUTH_STATUS_EXT):
				*((uint32 *)arg) = wlc_sup_get_auth_status_extended(sup);
				break;
#ifdef BCMSUP_PSK
			case IOV_GVAL(IOV_SUP_WPA_TMO):
				*ret_int_ptr = (int32)wlc_sup_get_wpa_psk_tmo(sup);
				break;

			case IOV_SVAL(IOV_SUP_WPA_TMO):
				wlc_sup_set_wpa_psk_tmo(sup, (uint32) int_val);
				if (int_val == 0) {
					wlc_sup_wpa_psk_timer(sup_info, bsscfg, FALSE);
				}
				break;
#endif /* BCMSUP_PSK */
			default:
				err = BCME_UNSUPPORTED;
				break;
		}
	}
exit:	return err;
}

#if defined(BCMINTSUP) || defined(WLFBT)
/* Look for AP's and STA's IE list in probe response and assoc req */
void
wlc_find_sup_auth_ies(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg, uint8 **sup_ies,
	uint *sup_ies_len, uint8 **auth_ies, uint *auth_ies_len)
{
	wlc_assoc_t *as = cfg->assoc;
	wlc_bss_info_t *current_bss = cfg->current_bss;

	if ((current_bss->bcn_prb == NULL) ||
	    (current_bss->bcn_prb_len <= sizeof(struct dot11_bcn_prb))) {
		*auth_ies = NULL;
		*auth_ies_len = 0;
	} else {
		*auth_ies = (uint8 *)&current_bss->bcn_prb[1];
		*auth_ies_len = current_bss->bcn_prb_len - sizeof(struct dot11_bcn_prb);
	}

	if ((as->req == NULL) || as->req_len == 0) {
		*sup_ies = NULL;
		*sup_ies_len = 0;
	} else {

		*sup_ies = (uint8 *)&as->req[1];	/* position past hdr */
		*sup_ies_len = as->req_len;

		/* If this was a re-assoc, there's another ether addr to skip */
		if (as->req_is_reassoc) {
			*sup_ies_len -= ETHER_ADDR_LEN;
			*sup_ies += ETHER_ADDR_LEN;
		}
		*sup_ies_len -= sizeof(struct dot11_assoc_req);
	}
}
#endif /* BCMINTSUP || WLFBT */

#ifdef BCMSUP_PSK
/* Build and send an EAPOL WPA key message */
bool
wlc_wpa_sup_sendeapol(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg, uint16 flags, wpa_msg_t msg)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	void * p;
#ifdef BCMINTSUP
	wlc_info_t *wlc = sup->wlc;
#endif // endif

	p = wlc_wpa_sup_prepeapol(sup, flags, msg);

	if (p != NULL) {

#ifdef EXT_STA
		/* for vista, mark the pkt as exempt */
		if (WLEXTSTA_ENAB(wlc->pub) && msg != GMSG2)
			WLPKTFLAG_EXEMPT_SET(WLPKTTAG(p), WSEC_EXEMPT_ALWAYS);
#endif /* EXT_STA */
#ifdef WLBTAMP
		if (BSS_BTA_ENAB(wlc, sup->cfg)) {
			struct ether_header *eh;
			struct dot11_llc_snap_header *lsh;
			osl_t *osh;

			/*
			 * re-encap packet w/ BT-SIG LLC/SNAP header and security prot ID:
			 * Step 1: convert Ethernet to 802.3 per 802.1H
			 * Step 2: replace RFC1042 SNAP header with BT-SIG encap header
			 * Step 3: replace ether_type with BT-SIG security prot ID
			 */
			osh = OSH(sup);
			eh = (struct ether_header *)PKTDATA(osh, p);
			wlc_ether_8023hdr(wlc, osh, eh, p);

			eh = (struct ether_header *)PKTDATA(osh, p);
			lsh = (struct dot11_llc_snap_header *)&eh[1];
			bcopy(BT_SIG_SNAP_MPROT, lsh, DOT11_LLC_SNAP_HDR_LEN - 2);

			lsh->type = hton16((uint16)BTA_PROT_SECURITY);
		}
#endif /* WLBTAMP */
#ifdef BCMINTSUP
		wlc_sendpkt(wlc, p, sup->cfg->wlcif);
#else
		(void)SEND_PKT(sup->cfg, p);
#endif // endif
		return TRUE;
	}
	return FALSE;
}

static void *
wlc_wpa_sup_prepeapol(supplicant_t *sup, uint16 flags, wpa_msg_t msg)
{
	uint16 len, key_desc, fbt_len = 0;
	void *p = NULL;
	eapol_header_t *eapol_hdr = NULL;
	eapol_wpa_key_header_t *wpa_key = NULL;
	uchar mic[PRF_OUTBUF_LEN];
	osl_t *osh = OSH(sup);
	wpapsk_t *wpa = sup->wpa;

	BCM_REFERENCE(osh);

	len = EAPOL_HEADER_LEN + EAPOL_WPA_KEY_LEN;
	switch (msg) {
	case PMSG2:		/* pair-wise msg 2 */
		if (wpa->sup_wpaie == NULL)
			break;
#ifdef WLFBT
		if (BSSCFG_IS_FBT(sup->cfg) && (wpa->WPA_auth & WPA2_AUTH_FT) &&
			BSS_FBT_INI_FBT(sup->wlc->fbt, sup->cfg))
			fbt_len = wlc_fbt_getlen_eapol(sup->wlc->fbt, sup->cfg);
#endif /* WLFBT */

		len += wpa->sup_wpaie_len + fbt_len;
		if ((p = wlc_eapol_pktget(sup->wlc, sup->cfg, &PEER_EA(sup),
			len)) == NULL)
			break;
		eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
		eapol_hdr->length = hton16(len - EAPOL_HEADER_LEN);
		wpa_key = (eapol_wpa_key_header_t *) eapol_hdr->body;
		bzero(wpa_key, EAPOL_WPA_KEY_LEN);
		hton16_ua_store((flags | PMSG2_REQUIRED), (uint8 *)&wpa_key->key_info);
		hton16_ua_store(wpa->tk_len, (uint8 *)&wpa_key->key_len);
		bcopy(wpa->snonce, wpa_key->nonce, EAPOL_WPA_KEY_NONCE_LEN);
		wpa_key->data_len = hton16(wpa->sup_wpaie_len + fbt_len);
		bcopy(wpa->sup_wpaie, wpa_key->data, wpa->sup_wpaie_len);
#ifdef WLFBT
		if (BSSCFG_IS_FBT(sup->cfg) && (wpa->WPA_auth & WPA2_AUTH_FT) &&
			BSS_FBT_INI_FBT(sup->wlc->fbt, sup->cfg)) {
			wlc_fbt_addies(sup->wlc->fbt, sup->cfg, wpa_key);
		}
#endif /* WLFBT */
		WL_WSEC(("wl%d: wlc_wpa_sup_sendeapol: sending message 2\n",
			UNIT(sup)));
		break;

	case PMSG4:		/* pair-wise msg 4 */
		if ((p = wlc_eapol_pktget(sup->wlc, sup->cfg, &PEER_EA(sup),
			len)) == NULL)
			break;
		eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
		eapol_hdr->length = hton16(EAPOL_WPA_KEY_LEN);
		wpa_key = (eapol_wpa_key_header_t *) eapol_hdr->body;
		bzero(wpa_key, EAPOL_WPA_KEY_LEN);
		hton16_ua_store((flags | PMSG4_REQUIRED), (uint8 *)&wpa_key->key_info);
		hton16_ua_store(wpa->tk_len, (uint8 *)&wpa_key->key_len);
		WL_WSEC(("wl%d: wlc_wpa_sup_sendeapol: sending message 4\n",
			UNIT(sup)));
		break;

	case GMSG2:	       /* group msg 2 */
		if ((p = wlc_eapol_pktget(sup->wlc, sup->cfg, &PEER_EA(sup),
			len)) == NULL)
			break;
		eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
		eapol_hdr->length = hton16(EAPOL_WPA_KEY_LEN);
		wpa_key = (eapol_wpa_key_header_t *) eapol_hdr->body;
		bzero(wpa_key, EAPOL_WPA_KEY_LEN);
		hton16_ua_store((flags | GMSG2_REQUIRED), (uint8 *)&wpa_key->key_info);
		hton16_ua_store(wpa->gtk_len, (uint8 *)&wpa_key->key_len);
		break;

	case MIC_FAILURE:	/* MIC failure report */
		if ((p = wlc_eapol_pktget(sup->wlc, sup->cfg, &PEER_EA(sup),
			len)) == NULL)
			break;
		eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
		eapol_hdr->length = hton16(EAPOL_WPA_KEY_LEN);
		wpa_key = (eapol_wpa_key_header_t *) eapol_hdr->body;
		bzero(wpa_key, EAPOL_WPA_KEY_LEN);
		hton16_ua_store(flags, (uint8 *)&wpa_key->key_info);
		break;

	default:
		WL_WSEC(("wl%d: wlc_wpa_sup_sendeapol: unexpected message type %d\n",
		         UNIT(sup), msg));
		break;
	}

	if (p != NULL) {
		/* do common message fields here; make and copy MIC last. */
		eapol_hdr->type = EAPOL_KEY;
		if (bcmwpa_is_rsn_auth(wpa->WPA_auth))
			wpa_key->type = EAPOL_WPA2_KEY;
		else
			wpa_key->type = EAPOL_WPA_KEY;
		bcopy(wpa->replay, wpa_key->replay, EAPOL_KEY_REPLAY_LEN);
		/* If my counter is one greater than the last one of his I
		 * used, then a ">=" test on receipt works AND the problem
		 * of zero at the beginning goes away.  Right?
		 */
		wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);
		key_desc = flags & (WPA_KEY_DESC_V1 |  WPA_KEY_DESC_V2);
		if (!wpa_make_mic(eapol_hdr, key_desc, wpa->eapol_mic_key,
			mic)) {
			WL_WSEC(("wl%d: wlc_wpa_sup_sendeapol: MIC generation failed\n",
			         UNIT(sup)));
			return FALSE;
		}
		bcopy(mic, wpa_key->mic, EAPOL_WPA_KEY_MIC_LEN);
	}
	return p;
}

#if defined(BCMCCX) || defined(BCMSUP_PSK)
void
wlc_wpa_send_sup_status(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg, uint reason)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	bss_sup_info_t *sup_bss =  SUP_BSSCFG_CUBBY(sup_info, cfg);
	uint status;

	if (!sup_bss || ((SUP_BSSCFG_CUBBY_PUB(sup_info, cfg))->sup_type != SUP_WPAPSK))
		return;
	status = wlc_sup_get_auth_status_extended(sup);

	if (status != WLC_SUP_DISCONNECTED) {
#ifdef BCMINTSUP
		wlc_bss_mac_event(sup->wlc, sup->cfg, WLC_E_PSK_SUP,
		                  NULL, status, reason, 0, 0, 0);
#else
		wpaif_forward_mac_event_cb(sup->cfg, reason, status);
#endif /* BCMINTSUP */
	}
}
#endif /* defined(BCMCCX) || defined (BCMSUP_PSK) */

#if defined(WLWNM)
void
wlc_wpa_sup_gtk_update(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg,
	int index, int key_len, uint8 *key, uint8 *rsc)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	wpapsk_t *wpa = sup->wpa;

	key_len = MIN(key_len, sizeof(wpa->gtk));
	wpa->gtk_len = (ushort)key_len;
	wpa->gtk_index = (uint8)index;
	memcpy(wpa->gtk, key, key_len);

	WL_WNM(("+ WNM-Sleep GTK update id:%d\n", index));

	wlc_wpa_plumb_gtk(sup->wlc, cfg, wpa->gtk, wpa->gtk_len,
		index, wpa->mcipher, rsc, FALSE);
}
#endif /* WLWNM */

#if defined(BCMCCX) || defined(BCMSUP_PSK) || defined(BCMAUTH_PSK)
/* Defined in 802.11 protocol */
/* Ver+GroupDataCipher+PairwiseCipherCount+PairwiseCipherList+AKMCount+AKMList+CAP */
#define RSN_PCCNT_OFFSET	(2+4)
#define RSN_AKMCNT_OFFSET(pc_cnt)	(RSN_PCCNT_OFFSET+2+4*(pc_cnt))
#define RSN_PMKID_OFFSET(pc_cnt, akm_cnt)	(RSN_AKMCNT_OFFSET(pc_cnt)+2+4*(akm_cnt)+2)

/* Return TRUE if check is successful */
static bool
wlc_wpa_sup_check_rsn(supplicant_t *sup, bcm_tlv_t *rsn)
{
	bool ret = FALSE;
	bool fbt = FALSE;
	uint16 pc_cnt = 0;
	uint16 akm_cnt = 0;
	bcm_tlv_t *wpaie = (bcm_tlv_t*)sup->wpa->auth_wpaie;
#ifdef WLFBT
	/* initial FBT has an extra pmkr1name in msg3 */
	if (WLFBT_ENAB(sup->pub) && (sup->wpa->WPA_auth & WPA2_AUTH_FT))
		fbt = TRUE;
#endif // endif
	if (rsn) {
		if (fbt) {
			if (wpaie->len + sizeof(wpa_pmkid_list_t) == rsn->len) {
				/* WPA IE ends at RSN CAP */
				ret = !memcmp(wpaie->data, rsn->data, wpaie->len);
			} else if (wpaie->len + (uint8)WPA2_PMKID_LEN >= rsn->len) {
				int off = RSN_PCCNT_OFFSET;
				if (rsn->len < (off + sizeof(uint16))) {
					goto done;
				}

				/* WPA IE has more optional fields */
				pc_cnt = ltoh16_ua(&rsn->data[off]);
				off = RSN_AKMCNT_OFFSET(pc_cnt);
				if (rsn->len < (off + sizeof(uint16))) {
					goto done;
				}

				akm_cnt = ltoh16_ua(&rsn->data[off]);
				off = RSN_PMKID_OFFSET(pc_cnt, akm_cnt);
				ret = (rsn->len >= off) && !memcmp(wpaie->data, rsn->data, off);
			}
		} else {
			ret = (wpaie->len == rsn->len) &&
					!memcmp(wpaie->data, rsn->data, wpaie->len);
		}
	}

done:
	return ret;
}

static bool
wlc_wpa_sup_eapol(supplicant_t *sup, eapol_header_t *eapol, bool encrypted)
{
	wlc_sup_info_t *sup_info = sup->m_handle;
	eapol_wpa_key_header_t *body = (eapol_wpa_key_header_t *)eapol->body;
	uint16 key_info, key_len, data_len;
	uint16 cipher;
	uint16 prohibited, required;
	wpapsk_t *wpa = sup->wpa;
	wlc_sup_priv_t *sup_priv = WLC_SUP_PRIV_INFO(sup_info);
#ifdef BCMCCX
	sup_bss_pub_t *supbsspub = SUP_BSSCFG_CUBBY_PUB(sup_info, sup->cfg);
#endif // endif

	key_info = ntoh16_ua(&body->key_info);

	WL_WSEC(("wl%d: wlc_wpa_sup_eapol: received EAPOL_WPA_KEY packet, KI:%x\n",
		UNIT(sup), key_info));

	if ((key_info & WPA_KEY_PAIRWISE) && !(key_info & WPA_KEY_MIC)) {
		/* This is where cipher checks would be done for WDS.
		 * See what NAS' nsup does when that's needed.
		 */
	}

	/* check for replay */
	if (wpa_array_cmp(MAX_ARRAY, body->replay, wpa->replay, EAPOL_KEY_REPLAY_LEN) ==
	    wpa->replay) {
#if defined(BCMDBG) || defined(WLMSG_WSEC)
		uchar *g = body->replay, *s = wpa->replay;
		WL_WSEC(("wl%d: wlc_wpa_sup_eapol: ignoring replay "
				 "(got %02x%02x%02x%02x%02x%02x%02x%02x"
				 " last saw %02x%02x%02x%02x%02x%02x%02x%02x)\n", UNIT(sup),
				 g[0], g[1], g[2], g[3], g[4], g[5], g[6], g[7],
				 s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]));
#endif /* BCMDBG || WLMSG_WSEC */

		return TRUE;
	}
#if defined(BCMDBG) || defined(WLMSG_WSEC)
	{
		uchar *g = body->replay, *s = wpa->replay;
		WL_WSEC(("wl%d: wlc_wpa_sup_eapol: NO replay "
			"(got %02x%02x%02x%02x%02x%02x%02x%02x"
			" last saw %02x%02x%02x%02x%02x%02x%02x%02x)\n", UNIT(sup),
			g[0], g[1], g[2], g[3], g[4], g[5], g[6], g[7],
			s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]));
	}
#endif /* BCMDBG || WLMSG_WSEC */

	/* check message MIC */
	if (key_info & WPA_KEY_MIC) {
		if (ntoh16(eapol->length) < OFFSETOF(eapol_wpa_key_header_t, data_len)) {
			WL_WSEC(("wl%d: wlc_wpa_sup_eapol: missing MIC , discarding pkt\n",
				UNIT(sup)));
			return TRUE;
		}
		if (!wpa_check_mic(eapol, key_info & (WPA_KEY_DESC_V1|WPA_KEY_DESC_V2),
			wpa->eapol_mic_key)) {
			/* 802.11-2007 clause 8.5.3.3 - silently discard MIC failure */
			WL_WSEC(("wl%d: wlc_wpa_sup_eapol: MIC failure, discarding pkt\n",
				UNIT(sup)));
			return TRUE;
		}
	}

	/* check data length is okay */
	if (ntoh16(eapol->length) < OFFSETOF(eapol_wpa_key_header_t, data)) {
		WL_WSEC(("wl%d: wlc_wpa_sup_eapol: too short - no data len, toss pkt\n",
			UNIT(sup)));
		return TRUE;
	}

	data_len = ntoh16_ua(&body->data_len);
	if (ntoh16(eapol->length) < (OFFSETOF(eapol_wpa_key_header_t, data) + data_len)) {
		WL_WSEC(("wl%d: wlc_wpa_sup_eapol: not enough data - discarding pkt\n",
			UNIT(sup)));
		return TRUE;
	}

	/* if MIC was okay, save replay counter */
	/* last_replay is NOT incremented after transmitting a message */
	bcopy(body->replay, wpa->replay, EAPOL_KEY_REPLAY_LEN);
	bcopy(body->replay, wpa->last_replay, EAPOL_KEY_REPLAY_LEN);

	/* decrypt key data field */
	if (bcmwpa_is_rsn_auth(wpa->WPA_auth) &&
	    (key_info & WPA_KEY_ENCRYPTED_DATA)) {

		uint8 *data, *encrkey;
		rc4_ks_t *rc4key;
		bool decr_status;

		if (!(data = MALLOC(sup->osh, WPA_KEY_DATA_LEN_256))) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				UNIT(sup), __FUNCTION__,  MALLOCED(sup->osh)));
			wlc_wpa_send_sup_status(sup_info, sup->cfg, WLC_E_SUP_DECRYPT_KEY_DATA);
			return FALSE;
		}
		if (!(encrkey = MALLOC(sup->osh, WPA_MIC_KEY_LEN*2))) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				UNIT(sup), __FUNCTION__,  MALLOCED(sup->osh)));
			MFREE(sup->osh, data, WPA_KEY_DATA_LEN_256);
			wlc_wpa_send_sup_status(sup_info, sup->cfg, WLC_E_SUP_DECRYPT_KEY_DATA);
			return FALSE;
		}
		if (!(rc4key = MALLOC(sup->osh, sizeof(rc4_ks_t)))) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				UNIT(sup), __FUNCTION__,  MALLOCED(sup->osh)));
			MFREE(sup->osh, data, WPA_KEY_DATA_LEN_256);
			MFREE(sup->osh, encrkey, WPA_MIC_KEY_LEN*2);
			wlc_wpa_send_sup_status(sup_info, sup->cfg, WLC_E_SUP_DECRYPT_KEY_DATA);
			return FALSE;
		}

		decr_status = wpa_decr_key_data(body, key_info,
		                       wpa->eapol_encr_key, NULL, data, encrkey, rc4key);

		MFREE(sup->osh, data, WPA_KEY_DATA_LEN_256);
		MFREE(sup->osh, encrkey, WPA_MIC_KEY_LEN*2);
		MFREE(sup->osh, rc4key, sizeof(rc4_ks_t));

		if (!decr_status) {
			WL_WSEC(("wl%d: wlc_wpa_sup_eapol: decryption of key"
					"data failed\n", UNIT(sup)));
			wlc_wpa_send_sup_status(sup_info, sup->cfg, WLC_E_SUP_DECRYPT_KEY_DATA);
			return FALSE;
		}
	}

	key_len = ntoh16_ua(&body->key_len);
	cipher = CRYPTO_ALGO_OFF;

	if (bcmwpa_is_wpa_auth(wpa->WPA_auth) || (key_info & WPA_KEY_PAIRWISE)) {

		/* Infer cipher from message key_len.  Association shouldn't have
		 * succeeded without checking that the cipher is okay, so this is
		 * as good a way as any to find it here.
		 */
#if defined(BCMCCX) || defined(BCMEXTCCX)
		if (CCX_ENAB(sup->pub))
			cipher = wlc_ccxsup_get_cipher(sup->wlc->ccxsup,
				sup->cfg, wpa, key_info, key_len);
		if (cipher == CRYPTO_ALGO_OFF)
#endif // endif
		switch (key_len) {
		case TKIP_KEY_SIZE:
			cipher = CRYPTO_ALGO_TKIP;
			break;
		case AES_KEY_SIZE:
			cipher = CRYPTO_ALGO_AES_CCM;
			break;
		case WEP128_KEY_SIZE:
			if (!(key_info & WPA_KEY_PAIRWISE)) {
				cipher = CRYPTO_ALGO_WEP128;
						break;
			} else {
				WL_WSEC(("wl%d: wlc_wpa_sup_eapol: illegal use of ucast WEP128\n",
				         UNIT(sup)));
				wlc_wpa_send_sup_status(sup_info, sup->cfg,
					WLC_E_SUP_BAD_UCAST_WEP128);
				return FALSE;
			}
		case WEP1_KEY_SIZE:
			if (!(key_info & WPA_KEY_PAIRWISE)) {
				cipher = CRYPTO_ALGO_WEP1;
				break;
			} else {
				WL_WSEC(("wl%d: wlc_wpa_sup_eapol: illegal use of ucast WEP40\n",
				         UNIT(sup)));
				wlc_wpa_send_sup_status(sup_info, sup->cfg,
					WLC_E_SUP_BAD_UCAST_WEP40);
				return FALSE;
			}
		default:
			WL_WSEC(("wl%d: wlc_wpa_sup_eapol: unsupported key_len = %d\n",
			         UNIT(sup), key_len));
			wlc_wpa_send_sup_status(sup_info, sup->cfg, WLC_E_SUP_UNSUP_KEY_LEN);
			return FALSE;
		}
	}

	if (key_info & WPA_KEY_PAIRWISE) {
		if (wpa->ucipher != cipher) {
			WL_WSEC(("wl%d: wlc_wpa_sup_eapol: unicast cipher mismatch in pairwise key"
			         " message\n",
			         UNIT(sup)));
			wlc_wpa_send_sup_status(sup_info, sup->cfg, WLC_E_SUP_PW_KEY_CIPHER);
			return FALSE;
		}

		if (!(key_info & WPA_KEY_MIC)) {

			WL_WSEC(("wl%d: wlc_wpa_sup_eapol: processing message 1\n",
			         UNIT(sup)));

			/* Test message 1 key_info flags */
			prohibited = encrypted ? (PMSG1_PROHIBITED & ~WPA_KEY_SECURE)
				: PMSG1_PROHIBITED;
			required = encrypted ? (PMSG1_REQUIRED & ~WPA_KEY_SECURE) : PMSG1_REQUIRED;
			if (((key_info & required) != required) || ((key_info & prohibited) != 0)) {
				WL_WSEC(("wl%d: wlc_wpa_sup_eapol: unexpected key_info (0x%04x) in"
				         " WPA pairwise key message 1\n",
				         UNIT(sup), (uint)key_info));
				/*
				 * XXX - Should this return an error?
				 *       Msg3 check does...
				 */
			}
			wpa->state = WPA_SUP_STAKEYSTARTP_PREP_M2;

			if ((wpa->WPA_auth & WPA2_AUTH_UNSPECIFIED) &&
#ifdef WLFBT
				(!BSSCFG_IS_FBT(sup->cfg) || !(wpa->WPA_auth & WPA2_AUTH_FT)) &&
#endif /* WLFBT */
				TRUE) {
				eapol_wpa2_encap_data_t *data_encap;

				/* extract PMKID */
				data_len = ntoh16_ua(&body->data_len);
				data_encap = wpa_find_kde(body->data, data_len,
				                          WPA2_KEY_DATA_SUBTYPE_PMKID);
				if (data_encap && (data_encap->length >=
					(OFFSETOF(eapol_wpa2_encap_data_t, data)
						- TLV_HDR_LEN + WPA2_PMKID_LEN))) {

#if defined(BCMDBG) || defined(WLMSG_WSEC)
					if (WL_WSEC_ON()) {
						int i;
						WL_WSEC(("wl%d: PMKID received: ", UNIT(sup)));
						for (i = 0; i < WPA2_PMKID_LEN; i++)
							WL_WSEC(("0x%x ", data_encap->data[i]));
						WL_WSEC(("\n"));
					}
#endif /* BCMDBG || WLMSG_WSEC */

					if (!wlc_sup_retrieve_pmk(sup, sup->cfg,
						data_encap->data, &BSS_EA(sup),
						sup->wpa_info->pmk, PMK_LEN)) {
						sup->wpa_info->pmk_len = PMK_LEN;
					}
					else
						return TRUE;
				}
				else {
					return TRUE;
				}
			}
#if defined(BCMSUP_PSK)
			else if (wpa->WPA_auth & WPA2_AUTH_PSK) {
				/* If driver based roaming is enabled, PMK can get overwritten.
				 * Restore the PMK derived from PSK.
				 */
				if (sup->wpa_info->pmk_psk_len > 0) {
					WL_WSEC(("wl%d: %s: Restore PMK\n",
						UNIT(sup), __FUNCTION__));
					bcopy((char*)sup->wpa_info->pmk_psk, sup->wpa_info->pmk,
						sup->wpa_info->pmk_psk_len);
					sup->wpa_info->pmk_len = sup->wpa_info->pmk_psk_len;
				}
				else
					WL_WSEC(("wl%d: %s: PMK from PSK not saved\n",
						UNIT(sup), __FUNCTION__));
			}
#endif /* BCMSUP_PSK */
			if (sup->wpa_info->pmk_len == 0) {
				WL_WSEC(("wl%d: wlc_wpa_sup_eapol: No PMK available to compose"
				         " pairwise msg 2\n",
				         UNIT(sup)));
				return TRUE;
			}

			/* Save Anonce, generate Snonce, and produce PTK */
			bcopy(body->nonce, wpa->anonce, sizeof(wpa->anonce));
			wlc_getrand(sup->wlc, wpa->snonce, EAPOL_WPA_KEY_NONCE_LEN);
#if defined(BCMCCX) || defined(BCMEXTCCX)
			if (CCX_ENAB(sup->pub) && IS_CCKM_AUTH(wpa->WPA_auth)) {
				wlc_ccxsup_handle_wpa_eapol_msg1(sup->wlc->ccxsup,
					sup->cfg, key_info);
			} else
#endif /* BCMCCX || BCMEXTCCX */
			{
#if defined(WLFBT)
				if (BSSCFG_IS_FBT(sup->cfg) && (wpa->WPA_auth & WPA2_AUTH_FT) &&
					(BSS_FBT_INI_FBT(sup->wlc->fbt, sup->cfg)))
					wlc_fbt_calc_fbt_ptk(sup->wlc->fbt, sup->cfg);
				else
#endif /* WLFBT */

#ifdef MFP
				if (((wpa->WPA_auth & (WPA2_AUTH_PSK_SHA256 |
					WPA2_AUTH_1X_SHA256)) ||
					((wpa->WPA_auth & WPA2_AUTH_PSK) &&
					BSSCFG_IS_MFP_REQUIRED(sup->cfg))) &&
					(sup->cfg->current_bss->wpa2.flags & RSN_FLAGS_SHA256))
					kdf_calc_ptk(&PEER_EA(sup), &CUR_EA(sup),
					       wpa->anonce, wpa->snonce,
					       sup->wpa_info->pmk, (uint)sup->wpa_info->pmk_len,
					       wpa->eapol_mic_key, (uint)wpa->ptk_len);
				else
#endif // endif
				{
					if (!memcmp(&PEER_EA(sup), &CUR_EA(sup), ETHER_ADDR_LEN)) {
						/* something is wrong -- toss; invalid eapol */
						WL_WSEC(("wl%d:%s: toss msg; same mac\n",
							UNIT(sup), __FUNCTION__));
						return TRUE;
					} else {
						wpa_calc_ptk(&PEER_EA(sup), &CUR_EA(sup),
							wpa->anonce, wpa->snonce,
							sup->wpa_info->pmk,
							(uint)sup->wpa_info->pmk_len,
							wpa->eapol_mic_key,
							(uint)wpa->ptk_len);
					}
				}

				/* Send pair-wise message 2 */
				if (wlc_wpa_sup_sendeapol(sup_info, sup->cfg,
					(key_info & PMSG2_MATCH_FLAGS), PMSG2)) {
					wpa->state = WPA_SUP_STAKEYSTARTP_WAIT_M3;
				} else {
					WL_WSEC(("wl%d: wlc_wpa_sup_eapol: send message 2 failed\n",
					         UNIT(sup)));
					wlc_wpa_send_sup_status(sup_info, sup->cfg,
						WLC_E_SUP_SEND_FAIL);
				}
			}
		} else {
			WL_WSEC(("wl%d: wlc_wpa_sup_eapol: processing message 3\n",
				UNIT(sup)));

			/* Test message 3 key_info flags */
			prohibited = (encrypted || sup_priv->sup_m3sec_ok)
			        ? (PMSG3_PROHIBITED & ~WPA_KEY_SECURE)
				: PMSG3_PROHIBITED;
			required = encrypted ? (PMSG3_REQUIRED & ~WPA_KEY_SECURE) : PMSG3_REQUIRED;

			if (bcmwpa_is_rsn_auth(wpa->WPA_auth)) {
				prohibited = 0;
#ifdef WLBTAMP
				if (BSS_BTA_ENAB(sup->wlc, sup->cfg))
					required = PMSG3_BRCM_REQUIRED;
				else
#endif /* WLBTAMP */
					required = PMSG3_WPA2_REQUIRED;
			}

			if (((key_info & required) != required) ||
				((key_info & prohibited) != 0))
			{
				WL_WSEC(("wl%d: wlc_wpa_sup_eapol: unexpected key_info (0x%04x) in"
				         " WPA pairwise key message 3\n",
				         UNIT(sup), (uint)key_info));
				return TRUE;
			} else if (wpa->state < WPA_SUP_STAKEYSTARTP_PREP_M2 ||
			           wpa->state > WPA_SUP_STAKEYSTARTG_PREP_G2) {
				WL_WSEC(("wl%d: wlc_wpa_sup_eapol: unexpected 4-way msg 3 in state"
				         " %d\n",
				         UNIT(sup), wpa->state));
				/* don't accept msg3 unless it follows msg1 */
				return TRUE;
			}
			wpa->state = WPA_SUP_STAKEYSTARTP_PREP_M4;

			/* check anonce */
			if (bcmp(body->nonce, wpa->anonce, sizeof(wpa->anonce))) {
				WL_WSEC(("wl%d: wlc_wpa_sup_eapol: anonce in key message 3 doesn't"
				         " match anonce in key message 1, discarding pkt \n",
				         UNIT(sup)));
				return TRUE;
			}

			/* Test AP's WPA IE against saved one */
			data_len = ntoh16_ua(&body->data_len);
			if (bcmwpa_is_rsn_auth(wpa->WPA_auth)) {
				uint16 len;
				bcm_tlv_t *wpa2ie;
				wpa2ie = bcm_parse_tlvs(body->data, data_len, DOT11_MNG_RSN_ID);
				/* verify RSN IE */
				if (!wlc_wpa_sup_check_rsn(sup, wpa2ie)) {
					WL_WSEC(("wl%d: wlc_wpa_sup_eapol: WPA IE mismatch in key"
						" message 3\n", UNIT(sup)));
					wlc_wpa_send_sup_status(sup_info, sup->cfg,
						WLC_E_SUP_MSG3_IE_MISMATCH);
					/* should cause a deauth */
					wlc_wpa_senddeauth(sup->cfg, (char *)&PEER_EA(sup),
						DOT11_RC_WPA_IE_MISMATCH);
					return TRUE;
				}

				/* looking for second RSN IE.  deauth if presents */
				len = data_len - (uint16)((uint8*)wpa2ie - (uint8*)body->data);
				if (len > ((uint16)TLV_HDR_LEN + (uint16)wpa2ie->len) &&
					bcm_parse_tlvs((uint8*)wpa2ie + TLV_HDR_LEN + wpa2ie->len,
					len - (TLV_HDR_LEN + wpa2ie->len), DOT11_MNG_RSN_ID)) {
					WL_WSEC(("wl%d: wlc_wpa_sup_eapol: WPA IE contains more"
						" than one RSN IE in key message 3\n",
						UNIT(sup)));
					wlc_wpa_send_sup_status(sup_info, sup->cfg,
						WLC_E_SUP_MSG3_TOO_MANY_IE);
					/* should cause a deauth */
					wlc_wpa_senddeauth(sup->cfg,
					                   (char *)&PEER_EA(sup),
					                   DOT11_RC_WPA_IE_MISMATCH);
					return TRUE;
				}
			}
			else if ((wpa->auth_wpaie_len != data_len) ||
			         (bcmp(wpa->auth_wpaie, body->data,
			               wpa->auth_wpaie_len))) {
				WL_WSEC(("wl%d: wlc_wpa_sup_eapol: WPA IE mismatch in key message"
				         " 3\n",
				         UNIT(sup)));
				/* should cause a deauth */
				wlc_wpa_senddeauth(sup->cfg, (char *)&PEER_EA(sup),
				                   DOT11_RC_WPA_IE_MISMATCH);
				return TRUE;
			}

			if (wlc_wpa_sup_sendeapol(sup_info, sup->cfg,
				(key_info & PMSG4_MATCH_FLAGS), PMSG4)) {
				wpa->state = WPA_SUP_STAKEYSTARTG_WAIT_G1;
			} else {
				WL_WSEC(("wl%d: wlc_wpa_sup_eapol: send message 4 failed\n",
				         UNIT(sup)));
				wlc_wpa_send_sup_status(sup_info, sup->cfg, WLC_E_SUP_SEND_FAIL);
				return FALSE;
			}

			if (key_info & WPA_KEY_INSTALL)
				/* Plumb paired key */
				wlc_wpa_plumb_tk(sup->wlc, sup->cfg,
					(uint8*)wpa->temp_encr_key,
					wpa->tk_len, wpa->ucipher, &PEER_EA(sup));
			else {
				/* While INSTALL is in the `required' set this
				 * test is a tripwire for when that changes
				 */
				WL_WSEC(("wl%d: wlc_wpa_sup_eapol: INSTALL flag unset in 4-way msg"
				         " 3\n",
				         UNIT(sup)));
				wlc_wpa_send_sup_status(sup_info, sup->cfg,
					WLC_E_SUP_NO_INSTALL_FLAG);
				return FALSE;
			}

			if (bcmwpa_is_rsn_auth(wpa->WPA_auth) &&
#ifdef WLBTAMP
			    !BSS_BTA_ENAB(sup->wlc, sup->cfg) &&
#endif // endif
			    TRUE) {
				eapol_wpa2_encap_data_t *data_encap;
				eapol_wpa2_key_gtk_encap_t *gtk_kde;

				/* extract GTK */
				data_encap = wpa_find_gtk_encap(body->data, data_len);
				if (!data_encap || ((uint)(data_encap->length -
					EAPOL_WPA2_GTK_ENCAP_MIN_LEN) > sizeof(wpa->gtk))) {
					WL_WSEC(("wl%d: wlc_wpa_sup_eapol: encapsulated GTK missing"
					         " from message 3\n", UNIT(sup_priv)));
					wlc_wpa_send_sup_status(sup_info, sup->cfg,
						WLC_E_SUP_MSG3_NO_GTK);
					return FALSE;
				}
				/* note: data encap length checked during lookup above */
				wpa->gtk_len = data_encap->length - EAPOL_WPA2_GTK_ENCAP_MIN_LEN;
				gtk_kde = (eapol_wpa2_key_gtk_encap_t *)data_encap->data;
				wpa->gtk_index = (gtk_kde->flags & WPA2_GTK_INDEX_MASK) >>
				    WPA2_GTK_INDEX_SHIFT;
				bcopy(gtk_kde->gtk, wpa->gtk, wpa->gtk_len);

				/* plumb GTK */
				wlc_wpa_plumb_gtk(sup->wlc, sup->cfg, wpa->gtk,
					wpa->gtk_len, wpa->gtk_index, wpa->mcipher, body->rsc,
					gtk_kde->flags & WPA2_GTK_TRANSMIT);
#ifdef MFP
				if (WLC_MFP_ENAB(sup->wlc->pub))
					wlc_mfp_extract_igtk(SUPMFP(sup), sup->cfg, eapol);
#endif // endif
			}
			if (bcmwpa_is_rsn_auth(wpa->WPA_auth)) {
				wpa->state = WPA_SUP_KEYUPDATE;
#ifdef BCMCCX
				if (CCX_ENAB(sup->pub) && (supbsspub->sup_type == SUP_LEAP_WPA)) {
					/* mark node authenticated */
					wlc_ccxsup_set_leap_state_keyed(sup->wlc->ccxsup, sup->cfg);
				}
#endif /* BCMCCX */

				WL_WSEC(("wl%d: wlc_wpa_sup_eapol: WPA2 key update complete\n",
				         UNIT(sup)));
				wlc_wpa_send_sup_status(sup_info, sup->cfg, WLC_E_SUP_OTHER);

				/* Authorize scb for data */
#ifdef BCMINTSUP
				(void)wlc_ioctl(sup->wlc, WLC_SCB_AUTHORIZE,
					&PEER_EA(sup), ETHER_ADDR_LEN, sup->cfg->wlcif);

#ifdef WLFBT
				if (BSSCFG_IS_FBT(sup->cfg))
					BSS_FBT_INI_FBT(sup->wlc->fbt, sup->cfg) = FALSE;
#endif /* WLFBT */

#else /* BCMINTSUP */
				AUTHORIZE(sup->cfg);
#endif /* BCMINTSUP */

#ifdef BCMCCX
				if (CCX_ENAB(sup->pub) && (supbsspub->sup_type == SUP_LEAP_WPA)) {
					/* send LEAP rogue AP report */
					wlc_ccxsup_send_leap_rogue_report(sup->wlc->ccxsup,
						sup->cfg);
				}
#endif /* BCMCCX */
			} else
				wpa->state = WPA_SUP_STAKEYSTARTG_WAIT_G1;
		}

	} else {
		/* Pairwise flag clear; should be group key message. */
		if (wpa->state <  WPA_SUP_STAKEYSTARTG_WAIT_G1) {
			WL_WSEC(("wl%d: wlc_wpa_sup_eapol: unexpected group key msg1 in state %d\n",
			         UNIT(sup), wpa->state));
			return TRUE;
		}

		if (SUP_CHECK_MCIPHER(sup)) {
			WL_WSEC(("wl%d: wlc_wpa_sup_eapol: multicast cipher mismatch in group key"
			         " message\n",
			         UNIT(sup)));
			wlc_wpa_send_sup_status(sup_info, sup->cfg, WLC_E_SUP_GRP_KEY_CIPHER);
			return FALSE;
		}

		if ((key_info & GMSG1_REQUIRED) != GMSG1_REQUIRED) {
			WL_WSEC(("wl%d: wlc_wpa_sup_eapol: unexpected key_info (0x%04x)in"
				 "WPA group key message\n",
				 UNIT(sup), (uint)key_info));
			return TRUE;
		}

		wpa->state = WPA_SUP_STAKEYSTARTG_PREP_G2;
		if (bcmwpa_is_rsn_auth(wpa->WPA_auth)) {
			eapol_wpa2_encap_data_t *data_encap;
			eapol_wpa2_key_gtk_encap_t *gtk_kde;

			/* extract GTK */
			data_len = ntoh16_ua(&body->data_len);
			data_encap = wpa_find_gtk_encap(body->data, data_len);
			if (!data_encap || ((uint)(data_encap->length -
				EAPOL_WPA2_GTK_ENCAP_MIN_LEN) > sizeof(wpa->gtk))) {
				WL_WSEC(("wl%d: wlc_wpa_sup_eapol: encapsulated GTK missing from"
					" group message 1\n", UNIT(sup)));
				wlc_wpa_send_sup_status(sup_info, sup->cfg,
					WLC_E_SUP_GRP_MSG1_NO_GTK);
				return FALSE;
			}
			/* note: data encap length checked during lookup above */
			wpa->gtk_len = data_encap->length - EAPOL_WPA2_GTK_ENCAP_MIN_LEN;
			gtk_kde = (eapol_wpa2_key_gtk_encap_t *)data_encap->data;
			wpa->gtk_index = (gtk_kde->flags & WPA2_GTK_INDEX_MASK) >>
			    WPA2_GTK_INDEX_SHIFT;
			bcopy(gtk_kde->gtk, wpa->gtk, wpa->gtk_len);

			/* plumb GTK */
			wlc_wpa_plumb_gtk(sup->wlc, sup->cfg, wpa->gtk, wpa->gtk_len,
				wpa->gtk_index, wpa->mcipher, body->rsc,
				gtk_kde->flags & WPA2_GTK_TRANSMIT);
#ifdef MFP
			if (WLC_MFP_ENAB(sup->wlc->pub))
				wlc_mfp_extract_igtk(SUPMFP(sup), sup->cfg, eapol);
#endif // endif
		} else {

			uint8 *data, *encrkey;
			rc4_ks_t *rc4key;
			bool decr_status;

			if (!(data = MALLOC(sup->osh, WPA_KEY_DATA_LEN_256))) {
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
					UNIT(sup), __FUNCTION__,  MALLOCED(sup_priv->osh)));
				wlc_wpa_send_sup_status(sup_info, sup->cfg,
					WLC_E_SUP_GTK_DECRYPT_FAIL);
				return FALSE;
			}
			if (!(encrkey = MALLOC(sup->osh, WPA_MIC_KEY_LEN*2))) {
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
					UNIT(sup), __FUNCTION__,  MALLOCED(sup->osh)));
				MFREE(sup->osh, data, WPA_KEY_DATA_LEN_256);
				wlc_wpa_send_sup_status(sup_info, sup->cfg,
					WLC_E_SUP_GTK_DECRYPT_FAIL);
				return FALSE;
			}
			if (!(rc4key = MALLOC(sup->osh, sizeof(rc4_ks_t)))) {
				WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
					UNIT(sup), __FUNCTION__,  MALLOCED(sup->osh)));
				MFREE(sup->osh, data, WPA_KEY_DATA_LEN_256);
				MFREE(sup->osh, encrkey, WPA_MIC_KEY_LEN*2);
				wlc_wpa_send_sup_status(sup_info, sup->cfg,
					WLC_E_SUP_GTK_DECRYPT_FAIL);
				return FALSE;
			}

			decr_status = wpa_decr_gtk(body, key_info, wpa->eapol_encr_key,
			                  wpa->gtk, data, encrkey, rc4key);

			MFREE(sup->osh, data, WPA_KEY_DATA_LEN_256);
			MFREE(sup->osh, encrkey, WPA_MIC_KEY_LEN*2);
			MFREE(sup->osh, rc4key, sizeof(rc4_ks_t));

			if (!decr_status || (key_len > sizeof(wpa->gtk))) {
				WL_WSEC(("wl%d: wlc_wpa_sup_eapol: GTK decrypt failure\n",
				         UNIT(sup)));
				wlc_wpa_send_sup_status(sup_info, sup->cfg,
					WLC_E_SUP_GTK_DECRYPT_FAIL);
				return FALSE;
			}
			wpa->gtk_len = key_len;

			/* plumb GTK */
			wlc_wpa_plumb_gtk(sup->wlc, sup->cfg, wpa->gtk, wpa->gtk_len,
				(key_info & WPA_KEY_INDEX_MASK) >> WPA_KEY_INDEX_SHIFT,
				cipher, body->rsc, key_info & WPA_KEY_INSTALL);
		}

		/* send group message 2 */
		if (wlc_wpa_sup_sendeapol(sup_info, sup->cfg,
			(key_info & GMSG2_MATCH_FLAGS), GMSG2)) {
			wpa->state = WPA_SUP_KEYUPDATE;
#ifdef BCMCCX
			if (CCX_ENAB(sup->pub) && (supbsspub->sup_type == SUP_LEAP_WPA)) {
				/* mark node authenticated */
				wlc_ccxsup_set_leap_state_keyed(sup->wlc->ccxsup, sup->cfg);
			}
#endif /* BCMCCX */

			WL_WSEC(("wl%d: wlc_wpa_sup_eapol: key update complete\n",
			         UNIT(sup)));
			wlc_wpa_send_sup_status(sup_info, sup->cfg, WLC_E_SUP_OTHER);
		} else {
			WL_WSEC(("wl%d: wlc_wpa_sup_eapol: send grp msg 2 failed\n",
			         UNIT(sup)));
			wlc_wpa_send_sup_status(sup_info, sup->cfg, WLC_E_SUP_SEND_FAIL);
		}

		/* Authorize scb for data */
#ifdef BCMINTSUP
		(void)wlc_ioctl(sup->wlc, WLC_SCB_AUTHORIZE,
			&PEER_EA(sup), ETHER_ADDR_LEN, sup->cfg->wlcif);
#else
		AUTHORIZE(sup->cfg);
#endif // endif

#ifdef	BCMCCX
		if (CCX_ENAB(sup->pub) && (supbsspub->sup_type == SUP_LEAP_WPA)) {
			/* send LEAP rogue AP report */
			wlc_ccxsup_send_leap_rogue_report(sup->wlc->ccxsup, sup->cfg);

		}
#endif /* BCMCCX */

	}
	return TRUE;
}
#endif /* defined(BCMCCX) || defined(BCMSUP_PSK) || defined(BCMAUTH_PSK) */

#ifdef BCMINTSUP
/* Break a lengthy passhash algorithm into smaller pieces. It is necessary
 * for dongles with under-powered CPUs.
 */
static void
wlc_sup_wpa_passhash_timer(void *arg)
{
	supplicant_t *sup = (supplicant_t *)arg;
	wpapsk_info_t *info = sup->wpa_info;

	if (do_passhash(&info->passhash_states, 256) == 0) {
		WL_WSEC(("wl%d: passhash is done\n", UNIT(sup)));
		get_passhash(&info->passhash_states, info->pmk, PMK_LEN);
		info->pmk_len = PMK_LEN;

#if defined(BCMSUP_PSK)
		/* Store the PMK derived from PSK */
		if (sup->cfg->WPA_auth & WPA2_AUTH_PSK) {
			bcopy((char*)info->pmk, info->pmk_psk, info->pmk_len);
			info->pmk_psk_len = info->pmk_len;
		}
#endif /* BCMSUP_PSK */
		wlc_join_bss_prep(sup->cfg);
		return;
	}

	WL_WSEC(("wl%d: passhash is in progress\n", UNIT(sup)));
	wl_add_timer(info->wlc->wl, info->passhash_timer, 0, 0);
}
#endif /* BCMINTSUP */

static bool
wlc_sup_wpapsk_start(supplicant_t *sup, uint8 *sup_ies, uint sup_ies_len,
	uint8 *auth_ies, uint auth_ies_len)
{
	wlc_sup_info_t *sup_info = sup->m_handle;
	bool ret = TRUE;
	wpapsk_t *wpa;
	sup_bss_pub_t *supbsspub = SUP_BSSCFG_CUBBY_PUB(sup_info, sup->cfg);

	wpa = sup->wpa;

	wlc_wpapsk_free(sup->wlc, wpa);

	wpa->state = WPA_SUP_INITIALIZE;

	if (SUP_CHECK_WPAPSK_SUP_TYPE(supbsspub)) {
		wpa->WPA_auth = sup->cfg->WPA_auth;
	}

	if (!wlc_wpapsk_start(sup->wlc, wpa, sup_ies, sup_ies_len, auth_ies, auth_ies_len)) {
		WL_ERROR(("wl%d: wlc_wpapsk_start() failed\n",
			UNIT(sup)));
		return FALSE;
	}

	if ((supbsspub->sup_type == SUP_WPAPSK) && (sup->wpa_info->pmk_len == 0)) {
		WL_WSEC(("wl%d: wlc_sup_wpapsk_start: no PMK material found\n", UNIT(sup)));
		ret = FALSE;
	}

	return ret;
}

#if defined(BCMINTSUP)
/* return 0 when succeeded, 1 when passhash is in progress, -1 when failed */
int
wlc_sup_set_ssid(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg, uchar ssid[], int len)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	bss_sup_info_t *sup_bss = SUP_BSSCFG_CUBBY(sup_info, cfg);

	if (sup_bss == NULL) {
		WL_WSEC(("wlc_sup_set_ssid: called with NULL sup\n"));
		return -1;
	} else if (sup->wpa_info->psk_len == 0) {
		WL_WSEC(("wlc_sup_set_ssid: called with NULL psk\n"));
		return 0;
	} else if (sup->wpa_info->pmk_len != 0) {
		WL_WSEC(("wlc_sup_set_ssid: called with non-NULL pmk\n"));
		return 0;
	}
	return wlc_wpa_cobble_pmk(sup->wpa_info, (char *)sup->wpa_info->psk,
		sup->wpa_info->psk_len, ssid, len);
}
#endif /* BCMINTSUP */

bool
wlc_sup_send_micfailure(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg, bool ismulti)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	bss_sup_info_t *sup_bss = SUP_BSSCFG_CUBBY(sup_info, cfg);
	uint16 flags;

	if (sup_bss == NULL) {
		WL_WSEC(("wlc_sup_send_micfailure called with NULL supplicant\n"));
		return FALSE;
	}

	if ((SUP_BSSCFG_CUBBY_PUB(sup_info, cfg))->sup_type == SUP_UNUSED)
		return FALSE;

	flags = (uint16) (MIC_ERROR_REQUIRED | sup->wpa->desc);
	if (!ismulti)
		flags |= (uint16) WPA_KEY_PAIRWISE;
	WL_WSEC(("wl%d.%d: wlc_sup_send_micfailure: sending MIC failure report\n",
	         UNIT(sup), WLC_BSSCFG_IDX(cfg)));
	(void) wlc_wpa_sup_sendeapol(sup_info, sup->cfg, flags, MIC_FAILURE);
	return TRUE;
}
#endif	/* BCMSUP_PSK */

#if defined(BCMSUP_PSK)
int
wlc_sup_set_pmk(struct wlc_sup_info *sup_info, struct wlc_bsscfg *cfg, wsec_pmk_t *pmk, bool assoc)
{
	supplicant_t *sup;
	bss_sup_info_t *sup_bss;

	if (sup_info == NULL || cfg == NULL) {
		WL_ERROR(("%s: missing sup_info or bsscfg\n", __FUNCTION__));
		return BCME_BADARG;
	}

	sup_bss = SUP_BSSCFG_CUBBY(sup_info, cfg);
	if (sup_bss == NULL || pmk == NULL) {
		WL_WSEC(("%s: missing required parameter\n", __FUNCTION__));
		return BCME_BADARG;
	}

	sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);

	return wlc_set_pmk(sup->wlc, sup->wpa_info, sup->wpa, cfg, pmk, assoc);
}
#endif /* BCMSUP_PSK */

#if defined(BCMSUP_PSK) || defined(WLFBT)
int
wlc_set_pmk(wlc_info_t *wlc, wpapsk_info_t *info, wpapsk_t *wpa,
	struct wlc_bsscfg *cfg, wsec_pmk_t *pmk, bool assoc)
{
	/* Zero length means forget what's there now */
	if (pmk->key_len == 0)
		return BCME_OK;

	info->pmk_len = 0;
	info->psk_len = 0;

#if defined(BCMINTSUP) || defined(WLFBT)
	/* A key that needs hashing has to wait until we see the SSID */
	if (pmk->flags & WSEC_PASSPHRASE) {
		WL_WSEC(("wl%d: %s: saving raw PSK\n",
		         wlc->pub->unit, __FUNCTION__));

		if (pmk->key_len == WSEC_MAX_PSK_LEN) {
			info->psk_len = 0;
			/* this size must be legible hex and need not wait */
			if (wlc_wpa_cobble_pmk(info, (char *)pmk->key, pmk->key_len,
				NULL, 0) < 0) {
				return BCME_ERROR;
			} else {
				if (bcmwpa_includes_rsn_auth(wpa->WPA_auth) && assoc) {
					int ret = BCME_OK;
#ifdef BCMSUP_PSK
					if (SUP_ENAB(wlc->pub) &&
						BSS_SUP_ENAB_WPA(wlc->idsup, cfg) &&
						(wpa->WPA_auth & WPA2_AUTH_UNSPECIFIED))
						ret = wlc_sup_set_pmkid(wlc->idsup, cfg,
							&info->pmk[0], info->pmk_len, &cfg->BSSID,
							NULL);
#endif /* BCMSUP_PSK */
#ifdef WLFBT
					if (BSSCFG_IS_FBT(cfg) && (ret == BCME_OK) &&
						(cfg->WPA_auth & WPA2_AUTH_FT))
						wlc_fbt_calc_fbt_ptk(wlc->fbt, cfg);
#endif // endif
					if (ret != BCME_OK)
						return ret;
				}
#ifdef BCMSUP_PSK
				/* For driver based roaming, PSK can be shared by the supplicant
				 * only once during initialization. Save the PMK derived from
				 * the PSK separately so that it can be used for subsequent
				 * PSK associations without intervention from the supplicant.
				 */
				if ((wpa->WPA_auth & WPA2_AUTH_PSK) || !assoc) {
					WL_WSEC(("wl%d: %s: Save the PMK from PSK\n",
						wlc->pub->unit, __FUNCTION__));
					bcopy((char*)info->pmk, info->pmk_psk, info->pmk_len);
					info->pmk_psk_len = info->pmk_len;
				}
#endif /* BCMSUP_PSK */
				return BCME_OK;
			}
		} else if ((pmk->key_len >= WSEC_MIN_PSK_LEN) &&
		    (pmk->key_len < WSEC_MAX_PSK_LEN)) {
			bcopy((char*)pmk->key, info->psk, pmk->key_len);
			info->psk_len = pmk->key_len;
			return BCME_OK;
		}
		return BCME_ERROR;
	}
#endif /* BCMINTSUP || WLFBT */

	/* If it's not a passphrase it must be a proper PMK */
	if (pmk->key_len > TKIP_KEY_SIZE) {
		WL_WSEC(("wl%d: %s: unexpected key size (%d)\n",
		         wlc->pub->unit, __FUNCTION__, pmk->key_len));
		return BCME_BADARG;
	}

	bcopy((char*)pmk->key, info->pmk, pmk->key_len);
	info->psk_len = 0;
	info->pmk_len = pmk->key_len;
#ifdef BCMSUP_PSK
	/* In case PMK is directly passed by the supplicant for PSK associations */
	if ((wpa->WPA_auth & WPA2_AUTH_PSK) || !assoc) {
		WL_WSEC(("wl%d: %s: Save the PMK passed directly\n",
			wlc->pub->unit, __FUNCTION__));
		bcopy((char*)info->pmk, info->pmk_psk, info->pmk_len);
		info->pmk_psk_len = info->pmk_len;
	}
#endif /* BCMSUP_PSK */

#if defined(BCMCCX) && defined(BCMINTSUP)
	/* initialize CCKM reassociation request number */
	/* External supplicant gets its seq num from driver, not here */
	if (CCX_ENAB(wlc->pub))
		wlc_ccxsup_init_cckm_rn(wlc->ccxsup, cfg);
#endif /* BCMCCX && BCMINTSUP */

#if defined(BCMSUP_PSK)
	if (SUP_ENAB(wlc->pub) && BSS_SUP_ENAB_WPA(wlc->idsup, cfg) &&
	   (wpa->WPA_auth & WPA2_AUTH_UNSPECIFIED) && assoc)
		wlc_sup_set_pmkid(wlc->idsup, cfg, &info->pmk[0], info->pmk_len,
			&cfg->BSSID, NULL);
#endif // endif
#ifdef WLFBT
	if (BSSCFG_IS_FBT(cfg) && (cfg->WPA_auth & WPA2_AUTH_FT))
		wlc_fbt_calc_fbt_ptk(wlc->fbt, cfg);
#endif // endif

	return BCME_OK;
}
#endif /* BCMSUP_PSK || WLFBT */

#if defined(BCMCCX) || defined(BCMSUP_PSK)
void
wlc_sup_set_ea(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg, struct ether_addr *ea)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	bss_sup_info_t *sup_bss = SUP_BSSCFG_CUBBY(sup_info, cfg);

	if (sup_bss == NULL)
		return;

	bcopy(ea, &sup->peer_ea, ETHER_ADDR_LEN);
}

/* ARGSUSED */
bool
wlc_set_sup(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg, int sup_type,
	/* the following parameters are used only for PSK */
	uint8 *sup_ies, uint sup_ies_len, uint8 *auth_ies, uint auth_ies_len)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	sup_bss_pub_t	*supbsspub = (SUP_BSSCFG_CUBBY_PUB(sup_info, cfg));
	bool ret = TRUE;

	if (supbsspub == NULL) {
		WL_WSEC(("wlc_set_sup called with NULL sup context\n"));
		return FALSE;
	}
	supbsspub->sup_type = SUP_UNUSED;
#if defined(BCMSUP_PSK)
	sup->ap_eapver = 0;
#endif /* BCMSUP_PSK */

#if defined(BCMSUP_PSK)
	if (sup_type == SUP_WPAPSK) {
		supbsspub->sup_type = sup_type;
		ret = wlc_sup_wpapsk_start(sup, sup_ies, sup_ies_len, auth_ies, auth_ies_len);
	}
#endif	/* BCMSUP_PSK */

#ifdef	BCMCCX
	if (CCX_ENAB(sup->pub) && CCX_CHECK_AUTH_SUP_TYPE(sup_type)) {
		supbsspub->sup_type = sup_type;

#if	defined(BCMSUP_PSK)
		if (sup_type == SUP_LEAP_WPA) {
			return (wlc_sup_wpapsk_start(sup, sup_ies, sup_ies_len,
				auth_ies, auth_ies_len) &&
				wlc_leapsup_start(sup->wlc->ccxsup, sup->cfg));
	}
#endif	/* BCMSUP_PSK */
		if (sup_type == SUP_LEAP) {
			ret = wlc_leapsup_start(sup->wlc->ccxsup, sup->cfg);
		}
	}
#endif /* BCMCCX */

	/* If sup_type is still SUP_UNUSED, the passed type must be bogus */
	if (supbsspub->sup_type == SUP_UNUSED) {
		WL_WSEC(("wl%d: wlc_set_sup: unexpected supplicant type %d\n",
		         UNIT(sup), sup_type));
		return FALSE;
	}
	return ret;
}
#endif /* BCMCCX || BCMSUP_PSK */

#if defined(BCMCCX) || defined(BCMSUP_PSK)
#ifdef BCMSUP_PSK
/* Convert the basic supplicant state from internal to external format */
static sup_auth_status_t
wlc_sup_conv_auth_state(wpapsk_state_t state)
{
	switch (state) {
		case WPA_SUP_DISCONNECTED:
			return WLC_SUP_DISCONNECTED;
		case WPA_SUP_INITIALIZE:
			return WLC_SUP_AUTHENTICATED;
		case WPA_SUP_AUTHENTICATION:
		case WPA_SUP_STAKEYSTARTP_PREP_M2:
		case WPA_SUP_STAKEYSTARTP_WAIT_M3:
		case WPA_SUP_STAKEYSTARTP_PREP_M4:
		case WPA_SUP_STAKEYSTARTG_WAIT_G1:
		case WPA_SUP_STAKEYSTARTG_PREP_G2:
			return WLC_SUP_KEYXCHANGE;
		case WPA_SUP_KEYUPDATE:
			return WLC_SUP_KEYED;
		default:
			return WLC_SUP_DISCONNECTED;
	}
}
#endif  /* BCMSUP_PSK */

static sup_auth_status_t
wlc_sup_get_auth_status(supplicant_t *sup)
{
#ifdef BCMCCX
	if (CCX_ENAB(sup->pub) &&
		SUP_CHECK_AUTH_SUP_TYPE((SUP_BSSCFG_CUBBY_PUB(sup->m_handle, sup->cfg)))) {
		return wlc_ccxsup_get_auth_status(sup->wlc->ccxsup, sup->cfg);
	}
#endif  /* BCMCCX */

#ifdef BCMSUP_PSK
	if ((SUP_BSSCFG_CUBBY_PUB(sup->m_handle, sup->cfg))->sup_type == SUP_WPAPSK) {
		return wlc_sup_conv_auth_state(sup->wpa->state);
	}
#endif  /* BCMSUP_PSK */

	return WLC_SUP_DISCONNECTED;
}

#if defined(BCMSUP_PSK)
/* Convert the extended supplicant state from internal to external format */
static sup_auth_status_t
wlc_sup_conv_ext_auth_state(wpapsk_state_t state)
{
	switch (state) {
		case WPA_SUP_STAKEYSTARTP_WAIT_M1:
			return WLC_SUP_KEYXCHANGE_WAIT_M1;
		case WPA_SUP_STAKEYSTARTP_PREP_M2:
			return WLC_SUP_KEYXCHANGE_PREP_M2;
		case WPA_SUP_STAKEYSTARTP_WAIT_M3:
			return WLC_SUP_KEYXCHANGE_WAIT_M3;
		case WPA_SUP_STAKEYSTARTP_PREP_M4:
			return WLC_SUP_KEYXCHANGE_PREP_M4;
		case WPA_SUP_STAKEYSTARTG_WAIT_G1:
			return WLC_SUP_KEYXCHANGE_WAIT_G1;
		case WPA_SUP_STAKEYSTARTG_PREP_G2:
			return WLC_SUP_KEYXCHANGE_PREP_G2;
		default:
			return wlc_sup_conv_auth_state(state);
	}
}
#endif	/* BCMSUP_PSK */

/* Return the extended supplicant authentication state */
static sup_auth_status_t
wlc_sup_get_auth_status_extended(supplicant_t *sup)
{
	sup_auth_status_t	status = wlc_sup_get_auth_status(sup);
#if defined(BCMSUP_PSK)
#ifdef BCMINTSUP
	if (!sup->cfg->associated) {
		status = WLC_SUP_DISCONNECTED;
	}
	else
#endif /* BCMINTSUP */
	if (status == WLC_SUP_KEYXCHANGE) {
		status = wlc_sup_conv_ext_auth_state(sup->wpa->state);
	}
#endif	/* BCMSUP_PSK */
	return status;
}

/* Dispatch EAPOL to supplicant.
 * Return boolean indicating whether supplicant's use of message means
 * it should be freed or sent up.
 */
bool
wlc_sup_eapol(struct wlc_sup_info *sup_info, struct wlc_bsscfg *cfg, eapol_header_t *eapol_hdr,
	bool encrypted)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	sup_bss_pub_t	*supbsspub = (SUP_BSSCFG_CUBBY_PUB(sup_info, cfg));

	if (!supbsspub) {
		WL_ERROR(("wl%d:%s: called with NULL sup\n", WLCWLUNIT(sup->wlc), __FUNCTION__));
		return FALSE;
	}
	if (supbsspub->sup_type == SUP_UNUSED)
		return FALSE;

#ifdef	BCMSUP_PSK
#ifdef	REDO_THIS_STUFF
	if (eapol_hdr->type == EAPOL_KEY) {
		eapol_wpa_key_header_t *body;

		body = (eapol_wpa_key_header_t *)eapol_hdr->body;
		if (body->type == EAPOL_WPA_KEY) {
		}
	}
#endif /* REDO_THIS_STUFF */
	/* Save eapol version from the AP */
	sup->ap_eapver = eapol_hdr->version;
	/* If the supplicant is set to do a WPA key exchange and this is
	 * a WPA key message, send it to the code for WPA.
	 */
	if ((eapol_hdr->type == EAPOL_KEY) &&
		SUP_CHECK_WPAPSK_SUP_TYPE(supbsspub))
	 {
		eapol_wpa_key_header_t *body;
		if (ntoh16(eapol_hdr->length) < OFFSETOF(eapol_wpa_key_header_t, mic)) {
			WL_WSEC(("wl%d: %s: bad eapol - header too small.\n",
				WLCWLUNIT(sup->wlc), __FUNCTION__));
			WLCNTINCR(sup->wlc->pub->_cnt->rxrunt);
			return FALSE;
		}
		body = (eapol_wpa_key_header_t *)eapol_hdr->body;
		if (SUP_CHECK_EAPOL(body)) {
			(void) wlc_wpa_sup_eapol(sup, eapol_hdr, encrypted);
			return TRUE;
		}
	}
#endif	/* BCMSUP_PSK */

#ifdef	BCMCCX
	if (CCX_ENAB(sup->pub) && SUP_CHECK_AUTH_SUP_TYPE(supbsspub)) {
		bool ret;

		/* LEAP supplicant gets EAP_PACKET and EAPOL_KEY */
		ASSERT((eapol_hdr->type == EAP_PACKET) ||
		       (eapol_hdr->type == EAPOL_KEY));
		ret = wlc_ccx_leapsup(sup->wlc->ccxsup, sup->cfg, eapol_hdr);
		return ret;
	}
#endif	/* BCMCCX */

	/* Get here if no supplicant saw the message */

#ifdef BCMSUP_PSK
	/* Reset sup state and clear PMK on (re)auth (i.e., EAP Id Request) */
	if (supbsspub->sup_type == SUP_WPAPSK && eapol_hdr->type == EAP_PACKET) {
		eap_header_t *eap_hdr = (eap_header_t *)eapol_hdr->body;

		if (eap_hdr->type == EAP_IDENTITY &&
		    eap_hdr->code == EAP_REQUEST) {
			if (sup->wpa->WPA_auth & WPA_AUTH_UNSPECIFIED ||
#if defined(BCMCCX) || defined(BCMEXTCCX)
				IS_CCKM_AUTH(sup->wpa->WPA_auth) ||
#endif /* BCMCCX || BCMEXTCCX */
			    sup->wpa->WPA_auth & WPA2_AUTH_UNSPECIFIED) {
				WL_WSEC(("wl%d: wlc_sup_eapol: EAP-Identity Request received - "
				         "reset supplicant state and clear PMK\n", UNIT(sup)));
				sup->wpa->state = WPA_SUP_INITIALIZE;
				sup->wpa_info->pmk_len = 0;
			} else {
				WL_WSEC(("wl%d: wlc_sup_eapol: EAP-Identity Request ignored\n",
				         UNIT(sup)));
			}
		} else {
			WL_WSEC(("wl%d: wlc_sup_eapol: EAP packet ignored\n", UNIT(sup)));
		}
	}
#endif /* BCMSUP_PSK */

	return FALSE;
}
#endif  /* BCMCCX || BCMSUP_PSK */

#if defined(BCMSUP_PSK) || defined(BCMCCX)
int
wlc_sup_down(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	bss_sup_info_t *sup_bss = SUP_BSSCFG_CUBBY(sup_info, cfg);
	int callbacks = 0;

	if (sup_bss == NULL)
		return callbacks;

#if defined(BCMSUP_PSK) && defined(BCMINTSUP)
	if (!wl_del_timer(sup->wl, sup->wpa_info->passhash_timer))
		callbacks ++;
	if (!wl_del_timer(sup->wl, sup->wpa_psk_timer))
		callbacks ++;
#endif	/* BCMSUP_PSK && BCMINTSUP */
	return callbacks;
}

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* Allocate supplicant context, squirrel away the passed values,
 * and return the context handle.
 */
int
wlc_sup_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_sup_info_t *sup_info = (wlc_sup_info_t *)ctx;
	wlc_sup_priv_t	*sup_priv = WLC_SUP_PRIV_INFO(sup_info);

	wlc_info_t *wlc = sup_priv->wlc;

	bss_sup_info_t **psup_bss = SUP_BSSCFG_CUBBY_LOC(sup_info, cfg);
	bss_sup_info_t *sup_bss = NULL;
	supplicant_t *sup = NULL;
	sup_bss_pub_t *supbsspub;

	sup_init_event_data_t evt;

	WL_TRACE(("wl%d: wlc_sup_init\n", UNIT(sup_priv)));

#ifdef WLAWDL
	/* Don't bother allocating a sup structure for AWDL BSS */
	if (BSSCFG_AWDL(wlc, cfg))
		return NULL;
#endif /* WLAWDL */

	if (BSSCFG_AP(cfg))
		return BCME_NOTSTA;
	if (!(sup_bss = (bss_sup_info_t *)MALLOC(sup_priv->osh, sizeof(bss_sup_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          UNIT(sup_priv), __FUNCTION__, MALLOCED(sup_priv->osh)));
		goto err;
	}
	*psup_bss = sup_bss;
	bzero(sup_bss, sizeof(bss_sup_info_t));

	sup = (supplicant_t *)((uint8 *)sup_bss + BSS_PRIV_OFFSET(sup_info));
	supbsspub = (sup_bss_pub_t *)sup_bss;

	sup->m_handle = wlc->idsup;
	sup->cfg = cfg;
	sup->wlc = wlc;
	sup->osh = sup_priv->osh;
	sup->wl = sup_priv->wl;
	sup->pub = sup_priv->pub;

	supbsspub->sup_type = SUP_UNUSED;
	supbsspub->sup_enable_wpa = TRUE;

#if defined(BCMSUP_PSK) || defined(WLFBT)
	if (!(sup->wpa = MALLOC(sup_priv->osh, sizeof(wpapsk_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          UNIT(sup_priv), __FUNCTION__, MALLOCED(sup_priv->osh)));
		goto err;
	}
	bzero(sup->wpa, sizeof(wpapsk_t));
	if (!(sup->wpa_info = MALLOC(sup_priv->osh, sizeof(wpapsk_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          UNIT(sup_priv), __FUNCTION__, MALLOCED(sup_priv->osh)));
		goto err;
	}
	bzero(sup->wpa_info, sizeof(wpapsk_info_t));
	sup->wpa_info->wlc = wlc;
#endif /* BCMSUP_PSK */

#if defined(BCMSUP_PSK) && defined(BCMINTSUP)
	if (!(sup->wpa_info->passhash_timer =
	           wl_init_timer(sup_priv->wl, wlc_sup_wpa_passhash_timer, sup, "passhash"))) {
		WL_ERROR(("wl%d: %s: passhash timer failed\n",
		          UNIT(sup_priv), __FUNCTION__));
		goto err;
	}

	if (!(sup->wpa_psk_timer = wl_init_timer(sup_priv->wl, wlc_wpa_psk_timer, sup,
	                                         "wpapsk"))) {
		WL_ERROR(("wl%d: wlc_sup_init: wl_init_timer for wpa psk timer failed\n",
		          UNIT(sup_priv)));
		goto err;
	}
#endif	/* BCMSUP_PSK && BCMINTSUP */

	evt.bsscfg = cfg;
	evt.wpa = sup->wpa;
	evt.wpa_info = sup->wpa_info;
	evt.up = TRUE;
	bcm_notif_signal(sup_priv->sup_up_down_notif_hdl, &evt);
	return BCME_OK;
err:
	if (sup_bss)
		wlc_sup_deinit(sup_info, cfg);
	return BCME_ERROR;
}

/* Toss supplicant context */
static void
wlc_sup_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_sup_info_t *sup_info = (wlc_sup_info_t *)ctx;
	bss_sup_info_t **psup_bss = SUP_BSSCFG_CUBBY_LOC(sup_info, cfg);
	bss_sup_info_t *sup_bss = *psup_bss;
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	sup_init_event_data_t evt;

	if (sup_bss != NULL) {
		WL_TRACE(("wl%d: wlc_sup_deinit\n", UNIT(sup)));

#if defined(BCMSUP_PSK) && defined(BCMINTSUP)
		if (sup->wpa_psk_timer)
			wl_free_timer(sup->wl, sup->wpa_psk_timer);

		if (sup->wpa_info) {
			if (sup->wpa_info->passhash_timer)
				wl_free_timer(sup->wl, sup->wpa_info->passhash_timer);
			MFREE(sup->osh, sup->wpa_info, sizeof(wpapsk_info_t));
		}
#endif	/* BCMSUP_PSK && BCMINTSUP */

	evt.bsscfg = cfg;
	evt.wpa = NULL; /* MAY be NULL also */
	evt.wpa_info = NULL;
	evt.up = FALSE;
	bcm_notif_signal(WLC_SUP_PRIV_INFO(sup_info)->sup_up_down_notif_hdl, &evt);

#if defined(BCMSUP_PSK)
		if (sup->wpa) {
			wlc_wpapsk_free(sup->wlc, sup->wpa);
			MFREE(sup->osh, sup->wpa, sizeof(wpapsk_t));
		}
#endif	/* BCMSUP_PSK */

		MFREE(sup->osh, sup_bss, sizeof(bss_sup_info_t));
		*psup_bss = NULL;
	}
}
#endif  /* BCMCCX || BCMSUP_PSK */

#if defined(WOWL) && defined(BCMSUP_PSK)
#define PUTU32(ct, st) { \
		(ct)[0] = (uint8)((st) >> 24); \
		(ct)[1] = (uint8)((st) >> 16); \
		(ct)[2] = (uint8)((st) >>  8); \
		(ct)[3] = (uint8)(st); }

/* For Wake-on-wireless lan, broadcast key rotation feature requires a information like
 * KEK - KCK to be programmed in the ucode
 */
void *
wlc_sup_hw_wowl_init(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	bss_sup_info_t *sup_bss =  SUP_BSSCFG_CUBBY(sup_info, cfg);
	uint32 rk[4*(AES_MAXROUNDS+1)];
	int rounds;
	void *gtkp;
	int i;
	wlc_info_t *wlc;
	uint keyrc_offset, kck_offset, kek_offset;
	uint16 ram_base;

	if (!sup_bss)
		return NULL;

	BCM_REFERENCE(rounds);

	wlc = sup->wlc;
	ram_base = wlc_read_shm(wlc, M_AESTABLES_PTR) * 2;

	if (D11REV_LT(sup->wlc->pub->corerev, 40)) {
		keyrc_offset = M_KEYRC_LAST;
		kck_offset = M_KCK;
		kek_offset = M_KEK;
		if (!ram_base)
			ram_base = WOWL_TX_FIFO_TXRAM_BASE;
	}
	else {
		keyrc_offset = D11AC_M_KEYRC_LAST;
		kck_offset = D11AC_M_KCK;
		kek_offset = D11AC_M_KEK;
		if (!ram_base)
			ram_base = D11AC_WOWL_TX_FIFO_TXRAM_BASE;
	}

	/* Program last reply counter -- sup->wpa.last_replay. sup->wpa.replay is the expected
	 * value of next message while the ucode requires replay value from the last message
	 */
	wlc_copyto_shm(wlc, keyrc_offset, sup->wpa->last_replay, EAPOL_KEY_REPLAY_LEN);

	/* Prepare a dummy GTK MSG2 packet to program header for WOWL ucode */
	/* We don't care about the actual flag, we just need a dummy frame to create d11hdrs from */
	if (sup->wpa->ucipher != CRYPTO_ALGO_AES_CCM)
		gtkp = wlc_wpa_sup_prepeapol(sup, (WPA_KEY_DESC_V1), GMSG2);
	else
		gtkp = wlc_wpa_sup_prepeapol(sup, (WPA_KEY_DESC_V2), GMSG2);

	if (!gtkp)
		return NULL;

	/* Program KCK -- sup->wpa->eapol_mic_key */
	wlc_copyto_shm(wlc, kck_offset, sup->wpa->eapol_mic_key, WPA_MIC_KEY_LEN);

	/* Program KEK for WEP/TKIP (how do I find what's what) */
	/* Else program expanded key using rijndaelKeySetupEnc and program the keyunwrapping
	 * tables
	 */
	if (sup->wpa->ucipher != CRYPTO_ALGO_AES_CCM)
		wlc_copyto_shm(wlc, kek_offset, sup->wpa->eapol_encr_key, WPA_ENCR_KEY_LEN);
	else {
		rounds = rijndaelKeySetupEnc(rk, sup->wpa->eapol_encr_key,
		                             AES_KEY_BITLEN(WPA_ENCR_KEY_LEN));
		ASSERT(rounds == EXPANDED_KEY_RNDS);

		/* Convert the table to format that ucode expects */
		for (i = 0; i < (int)(EXPANDED_KEY_LEN/sizeof(uint32)); i++) {
			uint32 *v = &rk[i];
			uint8 tmp[4];

			PUTU32(tmp, rk[i]);

			*v = (uint32)*((uint32*)tmp);
		}

		/* Program the template ram with AES key unwrapping tables */
		wlc_write_shm(wlc, M_AESTABLES_PTR, ram_base);

		wlc_write_template_ram(wlc, ram_base,
		                       ARRAYSIZE(aes_xtime9dbe) * 2, (void *)aes_xtime9dbe);

		wlc_write_template_ram(wlc, ram_base +
		                       (ARRAYSIZE(aes_xtime9dbe) * 2),
		                       ARRAYSIZE(aes_invsbox) * 2,
		                       (void *)aes_invsbox);

		wlc_write_template_ram(wlc, ram_base +
		                       ((ARRAYSIZE(aes_xtime9dbe) + ARRAYSIZE(aes_invsbox)) * 2),
		                       EXPANDED_KEY_LEN, (void *)rk);
	}

	return gtkp;
}

/* Update the Supplicant's software state as the key could have rotated while driver was in
 * Wake mode
 */
void
wlc_sup_sw_wowl_update(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg)
{
	uint keyrc_offset;

	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	bss_sup_info_t *sup_bss =  SUP_BSSCFG_CUBBY(sup_info, cfg);

	if (D11REV_LT(sup->wlc->pub->corerev, 40))
		keyrc_offset = M_KEYRC_LAST;
	else
		keyrc_offset = D11AC_M_KEYRC_LAST;

	/* Update the replay counter from the AP */
	wlc_copyfrom_shm(sup->wlc, keyrc_offset, sup->wpa->last_replay, EAPOL_KEY_REPLAY_LEN);

	if (!sup_bss)
		return;

	/* Driver's copy of replay counter is one more than APs */
	bcopy(sup->wpa->last_replay, sup->wpa->replay, EAPOL_KEY_REPLAY_LEN);
	wpa_incr_array(sup->wpa->replay, EAPOL_KEY_REPLAY_LEN);
}
#endif /* WOWL && BCMSUP_PSK */

#ifdef BCMSUP_PSK
/* Get WPA-PSK Supplicant 4-way handshake timeout */
static uint32
wlc_sup_get_wpa_psk_tmo(supplicant_t *sup)
{
	return sup->wpa_psk_tmo;
}

/* Set WPA-PSK Supplicant 4-way handshake timeout */
static void
wlc_sup_set_wpa_psk_tmo(supplicant_t *sup, uint32 tmo)
{
	sup->wpa_psk_tmo = tmo;
	return;
}

static void
wlc_wpa_psk_timer(void *arg)
{
	supplicant_t *sup = (supplicant_t *)arg;
	sup_bss_pub_t *supbsspub = SUP_BSSCFG_CUBBY_PUB(sup->m_handle, sup->cfg);

	if (!supbsspub) {
		return;
	}

	sup->wpa_psk_timer_active = 0;

	if (!supbsspub->sup_enable_wpa ||
		(supbsspub->sup_type != SUP_WPAPSK) || !sup->wpa_psk_tmo) {
		return;
	}
	/* Report timeout event */
	if (sup->cfg->associated && sup->wpa->state != WPA_SUP_KEYUPDATE) {
		WL_WSEC(("wl%d: wlc_wpa_psk_timer: 4-way handshake timeout\n",
		         UNIT(sup)));
		wlc_wpa_send_sup_status(sup->m_handle, sup->cfg, WLC_E_SUP_WPA_PSK_TMO);
	}

	return;
}

void
wlc_sup_wpa_psk_timer(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg, bool start)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	sup_bss_pub_t *supbsspub = SUP_BSSCFG_CUBBY_PUB(sup_info, cfg);

	if (!supbsspub) {
		return;
	}

	if (!supbsspub->sup_enable_wpa ||
		(supbsspub->sup_type != SUP_WPAPSK) || !sup->wpa_psk_tmo) {
		return;
	}

	/* Stop timer */
	if (sup->wpa_psk_timer_active) {
		wl_del_timer(sup->wl, sup->wpa_psk_timer);
		sup->wpa_psk_timer_active = 0;
	}

	/* Start WPA PSK timer */
	if (start == TRUE) {
		wl_add_timer(sup->wl, sup->wpa_psk_timer, sup->wpa_psk_tmo, 0);
		sup->wpa_psk_timer_active = 1;
	}
	return;
}
#endif  /* BCMSUP_PSK */

static void
wlc_sup_handle_joinproc(wlc_sup_info_t *sup_info, bss_assoc_state_data_t *evt_data)
{
	if (evt_data->state == AS_JOIN_INIT)
		wlc_sup_handle_joinstart(sup_info, evt_data->cfg);
	else if (evt_data->state == AS_JOIN_ADOPT)
		wlc_sup_handle_joindone(sup_info, evt_data->cfg);
}

void
wlc_sup_handle_joinstart(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	sup_bss_pub_t	*supbsspub = (SUP_BSSCFG_CUBBY_PUB(sup_info, cfg));
	wlc_info_t *wlc;
#if defined(BCMDBG) || defined(WLMSG_WSEC)
	char *ssidbuf;
	const char *ssidstr;
#endif // endif

	if (!supbsspub || !supbsspub->sup_enable_wpa)
		return;
	wlc = sup->wlc;

#if defined(BCMDBG) || defined(WLMSG_WSEC)
	ssidbuf = (char *) MALLOC(wlc->osh, SSID_FMT_BUF_LEN);
	if (ssidbuf) {
		wlc_format_ssid(ssidbuf, cfg->SSID, cfg->SSID_len);
		ssidstr = ssidbuf;
	} else
		ssidstr = "???";
#endif // endif

	supbsspub->sup_type = SUP_UNUSED;

	if (wlc_assoc_iswpaenab(cfg, TRUE))
		supbsspub->sup_type = SUP_WPAPSK;

	/* check if network is configured for LEAP */
	WL_WSEC(("wl%d: wlc_join_start: checking %s for LEAP\n",
	         wlc->pub->unit, (cfg->SSID_len == 0) ? "<NULL SSID>" : ssidstr));
#if defined(BCMCCX) && defined(BCMINTSUP)
	if (CCX_ENAB(sup->pub))
		supbsspub->sup_type = wlc_ccxsup_handle_joinstart(wlc->ccxsup,
			sup->cfg, (uint16)supbsspub->sup_type);
#endif	/* BCMCCX && BCMINTSUP */

	/* clear PMKID cache */
	if (wlc_assoc_iswpaenab(cfg, FALSE)) {
		WL_WSEC(("wl%d: wlc_join_start: clearing PMKID cache and candidate list\n",
			wlc->pub->unit));
		wlc_pmkid_clear_store(wlc->pmkid_info, cfg);
	}

#if defined(BCMDBG) || defined(WLMSG_WSEC)
	if (ssidbuf != NULL)
		 MFREE(wlc->osh, (void *)ssidbuf, SSID_FMT_BUF_LEN);
#endif // endif
}

void
wlc_sup_handle_joindone(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	sup_bss_pub_t	*supbsspub = (SUP_BSSCFG_CUBBY_PUB(sup_info, cfg));
	wlc_info_t *wlc;

	if (!supbsspub || !supbsspub->sup_enable_wpa)
		return;
	wlc = sup->wlc;

	if (SUP_ENAB(wlc->pub) && (supbsspub->sup_type != SUP_UNUSED)) {
		bool sup_stat;
		bool fast_reassoc = FALSE;

#ifdef BCMCCX
		/* check for CCKM fast roam success */
		/* XXX - CCXv2: Is this the right way to handle reassoc success
		 * with reassoc resp IE MIC failure?
		 */
		if (CCX_ENAB(wlc->pub) && IS_CCKM_AUTH(cfg->WPA_auth) &&
			wlc_cckm_reassoc_resp(wlc->ccxsup, cfg)) {
			fast_reassoc = TRUE;
		} else
#endif // endif
		{
#if defined(WLFBT)
			if (BSSCFG_IS_FBT(cfg) && (cfg->auth_atmptd == DOT11_FAST_BSS)) {
				fast_reassoc = TRUE;
			}
#endif /* WLFBT */
		}

		wlc_sup_set_ea(wlc->idsup, cfg, &cfg->BSSID);
		if (!fast_reassoc) {
			uint8 *sup_ies, *auth_ies;
			uint sup_ies_len, auth_ies_len;

			WL_WSEC(("wl%d: wlc_adopt_bss: calling set sup\n",
				wlc->pub->unit));
			wlc_find_sup_auth_ies(wlc->idsup, cfg, &sup_ies, &sup_ies_len,
				&auth_ies, &auth_ies_len);
			sup_stat = wlc_set_sup(wlc->idsup, cfg, supbsspub->sup_type,
				sup_ies, sup_ies_len, auth_ies, auth_ies_len);
#ifdef BCMCCX
			if (CCX_ENAB(wlc->pub) && sup_stat) {
				wlc_ccxsup_start_negotimer(wlc->ccxsup, cfg);
			}
#else
			BCM_REFERENCE(sup_stat);
#endif /* BCMCCX */
		}
		/* Start WPA-PSK 4-way handshake timer */
		wlc_sup_wpa_psk_timer(wlc->idsup, cfg, TRUE);
	}
}

unsigned char
wlc_sup_geteaphdrver(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg)
{
	bss_sup_info_t *sup_bss =  SUP_BSSCFG_CUBBY(sup_info, cfg);
	wlc_sup_priv_t	*sup_priv = WLC_SUP_PRIV_INFO(sup_info);
	supplicant_t *sup;

	if (sup_priv->sup_wpa2_eapver == 1)
			return WPA_EAPOL_VERSION;
	else if (sup_bss) {
		sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
		if (sup_priv->sup_wpa2_eapver == -1 && sup->ap_eapver)
			return sup->ap_eapver;
	}
	return WPA2_EAPOL_VERSION;
}

/*
 * wlc_sup_up_down_register()
 *
 * This function registers a callback that will be invoked when join
 * start event occurs from assoc module.
 *
 * Parameters
 *    wlc       Common driver context.
 *    callback  Callback function  to invoke on join-start events.
 *    arg       Client specified data that will provided as param to the callback.
 * Returns:
 *    BCME_OK on success, else BCME_xxx error code.
 */
int BCMATTACHFN(wlc_sup_up_down_register)(struct wlc_info *wlc, sup_init_fn_t callback,
                               void *arg)
{
	wlc_sup_info_t *sup_info = wlc->idsup;
	wlc_sup_priv_t	*sup_priv = WLC_SUP_PRIV_INFO(sup_info);

	return (bcm_notif_add_interest(sup_priv->sup_up_down_notif_hdl,
	                               (bcm_notif_client_callback)callback,
	                               arg));
}

/*
 * wlc_sup_up_down_unregister()
 *
 * This function unregisters a bsscfg up/down event callback.
 *
 * Parameters
 *    wlc       Common driver context.
 *    callback  Callback function that was previously registered.
 *    arg       Client specified data that was previously registerd.
 * Returns:
 *    BCME_OK on success, else BCME_xxx error code.
 */
int BCMATTACHFN(wlc_sup_up_down_unregister)(struct wlc_info *wlc, sup_init_fn_t callback,
                                        void *arg)
{
	wlc_sup_info_t *sup_info = wlc->idsup;
	wlc_sup_priv_t	*sup_priv = WLC_SUP_PRIV_INFO(sup_info);

	return (bcm_notif_remove_interest(sup_priv->sup_up_down_notif_hdl,
	                                  (bcm_notif_client_callback)callback,
	                                  arg));
}

/* to retrieve PMK associated with PMKID.
*/
static bool
wlc_sup_retrieve_pmk(supplicant_t *sup, wlc_bsscfg_t *cfg, uint8 *data,
	struct ether_addr *bssid, uint8 *pmk, uint8 pmklen)
{
	uint i;

	for (i = 0; i < sup->npmkid_sup; i++) {
		if (!bcmp(data, sup->pmkid_sup[i].PMKID,
			WPA2_PMKID_LEN) &&
			!bcmp(bssid, &sup->pmkid_sup[i].BSSID,
			ETHER_ADDR_LEN)) {
			bcopy(sup->pmkid_sup[i].PMK, pmk, PMK_LEN);
			break;
		}
	}
	/* Check for npmkid_sup instead of npmkid else retrieving of pmkid fails */
	if (i == sup->npmkid_sup) {
		WL_WSEC(("wl%d: wlc_sup_retrieve_pmk: unrecognized"
				 " PMKID in WPA pairwise key message 1\n",
				 UNIT(sup)));
		return TRUE;
	}
	else
		return FALSE;
}

/* Gets called when 'set pmk' command is given. As finishing part
* of 'set pmk', if associated, PMKID is calculated for the associated BSSID
*/
int
wlc_sup_set_pmkid(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg, uint8 *pmk, ushort pmk_len,
	struct ether_addr *auth_ea, uint8 *pmkid_out)
{
	sup_pmkid_t *pmkid = NULL;
	uint i;
	uint8 *data, *digest;
	supplicant_t *sup;

	ASSERT(sup_info && cfg);
	sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	ASSERT(sup);

	if (ETHER_ISNULLADDR(auth_ea)) {
		WL_WSEC(("wl%d: wlc_sup_set_pmkid: can't calculate PMKID - NULL BSSID\n",
			UNIT(sup)));
		return BCME_BADARG;
	}

	/* Overwrite existing PMKID for the given auth_ea  */
	for (i = 0; i < sup->npmkid_sup; i++) {
		pmkid = &sup->pmkid_sup[i];
		if (!bcmp(auth_ea, &pmkid->BSSID, ETHER_ADDR_LEN))
			break;
	}

	/* Add new PMKID to store if no existing PMKID was found */
	if (i == sup->npmkid_sup) {
		/* match not found, add a new one or
		 * overwrite the last index when pmkid_sup is full
		 */
		if (sup->npmkid_sup < SUP_MAXPMKID) {
			pmkid = &sup->pmkid_sup[sup->npmkid_sup++];
		}
		bcopy(auth_ea, &pmkid->BSSID, ETHER_ADDR_LEN);
	}

	ASSERT(pmkid);
	if (!pmkid) {
		/* Unreachable by construction */
		return BCME_ERROR;
	}

	/* compute PMKID and add to supplicant store */
	bzero(pmkid->PMK, PMK_LEN);
	bcopy(pmk, pmkid->PMK, pmk_len);

	if (!(data = MALLOC(sup->osh, WPA_KEY_DATA_LEN_128))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(sup), __FUNCTION__,  MALLOCED(sup->osh)));
		return BCME_NOMEM;
	}
	if (!(digest = MALLOC(sup->osh, PRF_OUTBUF_LEN))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(sup), __FUNCTION__,  MALLOCED(sup->osh)));
		MFREE(sup->osh, data, WPA_KEY_DATA_LEN_128);
		return BCME_NOMEM;
	}

#ifdef MFP
	if (WLC_MFP_ENAB(sup->wlc->pub) && ((sup->cfg->wsec & MFP_SHA256) ||
		((sup->cfg->current_bss->wpa2.flags & RSN_FLAGS_SHA256))))
		kdf_calc_pmkid(auth_ea, &cfg->cur_etheraddr, pmk,
		(uint)pmk_len, pmkid->PMKID, data, digest);
	else
#endif // endif
	wpa_calc_pmkid(auth_ea, &cfg->cur_etheraddr, pmk,
	               (uint)pmk_len, pmkid->PMKID, data, digest);

	MFREE(sup->osh, data, WPA_KEY_DATA_LEN_128);
	MFREE(sup->osh, digest, PRF_OUTBUF_LEN);

	if (pmkid_out)
		bcopy(pmkid->PMKID, pmkid_out, WPA2_PMKID_LEN);
	return BCME_OK;
}

bool
wlc_sup_find_pmkid(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg,
struct ether_addr *bssid, uint8	*pmkid)
{
	uint	j;
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);

	for (j = 0; j < sup->npmkid_sup; j++) {
		if (!bcmp(bssid, &sup->pmkid_sup[j].BSSID,
			ETHER_ADDR_LEN)) {
			bcopy(sup->pmkid_sup[j].PMKID, pmkid, WPA2_PMKID_LEN);
			return TRUE;
		}
	}
	return FALSE;
}

void
wlc_sup_clear_pmkid_store(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	sup_bss_pub_t	*supbsspub = (SUP_BSSCFG_CUBBY_PUB(sup_info, cfg));

	if (!supbsspub || !supbsspub->sup_enable_wpa)
		return;

	sup->npmkid_sup = 0;
}

static int
wlc_sup_config_get(void *ctx, wlc_bsscfg_t *cfg, uint8 *data, int *len)
{
	wlc_sup_info_t 	*sup_info = (wlc_sup_info_t *)ctx;
	bss_sup_info_t 	*sup_bss = NULL;
	sup_bss_pub_t	*supbsspub = NULL;
	supplicant_t 	*sup = NULL;
	wpapsk_info_t 	*info = NULL;

	int config_len = SUP_CUBBY_CONFIG_SIZE;

	if (len == NULL) {
		WL_ERROR(("%s: Null len\n", __FUNCTION__));
		return BCME_ERROR;
	}

	ASSERT(cfg != NULL);
	if (BSSCFG_AP(cfg)) {
		*len = 0;
		return BCME_NOTSTA;
	}

	sup_bss =  SUP_BSSCFG_CUBBY(sup_info, cfg);
	sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);

	if (sup_bss == NULL || sup == NULL) {
		WL_INFORM(("%s: Sup not enabled, nothing to config\n", __FUNCTION__));
		*len = 0;
		return BCME_OK;
	}

	info = sup->wpa_info;
	supbsspub = (SUP_BSSCFG_CUBBY_PUB(sup_info, cfg));

	if (!supbsspub || !supbsspub->sup_enable_wpa) {
		WL_INFORM(("%s: Sup not enabled, nothing to config\n", __FUNCTION__));
		*len = 0;
		return BCME_OK;
	}
	if ((data == NULL) || (*len < config_len)) {
		WL_ERROR(("%s: Insufficient buffer data(%p) len(%d/%d)\n",
			__FUNCTION__, data, *len, config_len));
		*len = config_len;
		return BCME_BUFTOOSHORT;
	}
	if (data) {
		WL_WSEC(("wl%d.%d get cubby config info:%d, %d, %d \n",
			sup->wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
			info->psk_len, info->pmk_len, info->pmk_psk_len));
		*len = config_len;
		bcopy(SUP_CUBBY_CONFIG_DATA(info), data, config_len);
	}
	return BCME_OK;
}

static int
wlc_sup_config_set(void *ctx, wlc_bsscfg_t *cfg, const uint8 *data, int len)
{
	wlc_sup_info_t *sup_info = (wlc_sup_info_t *)ctx;
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	sup_bss_pub_t	*supbsspub = (SUP_BSSCFG_CUBBY_PUB(sup_info, cfg));
	wpapsk_info_t *info = sup->wpa_info;
	int config_len = SUP_CUBBY_CONFIG_SIZE;

	if ((data == NULL) || (len < config_len)) {
		WL_ERROR(("%s: data(%p) len(%d/%d)\n", __FUNCTION__, data, len, config_len));
		return BCME_ERROR;
	}
	if (!supbsspub || !supbsspub->sup_enable_wpa) {
		WL_ERROR(("%s: Sup not enabled, nothing to config\n", __FUNCTION__));
		return BCME_ERROR;
	}
	bcopy(data, SUP_CUBBY_CONFIG_DATA(info), SUP_CUBBY_CONFIG_SIZE);
	WL_WSEC(("wl%d.%d set cubby config info:%d, %d, %d \n",
		sup->wlc->pub->unit, WLC_BSSCFG_IDX(cfg),
		info->psk_len, info->pmk_len, info->pmk_psk_len));
	return BCME_OK;
}

#if defined(BCMSUP_PSK) && defined(WLFBT)
void
wlc_sup_clear_replay(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	ASSERT(sup);
	bzero(sup->wpa->replay, EAPOL_KEY_REPLAY_LEN);
	bzero(sup->wpa->last_replay, EAPOL_KEY_REPLAY_LEN);
	WL_WSEC(("Reset replay counter to 0\n"));
}
#endif /* BCMSUP_PSK && WLFBT */

#ifdef WLOFFLD
bool
wlc_wpa_sup_get_rekey_info(struct wlc_sup_info *sup_info,
	wlc_bsscfg_t *cfg, rsn_rekey_params * rsnkey)
{
	supplicant_t *sup = sup_info ? SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg) : NULL;
	wpapsk_t *wpa = sup ? sup->wpa : NULL;

	if (wpa && rsnkey && sup->cfg->associated &&
		(sup->wpa->state == WPA_SUP_KEYUPDATE)) {
		bcopy(wpa->eapol_encr_key, rsnkey->kek, WPA_ENCR_KEY_LEN);
		bcopy(wpa->eapol_mic_key, rsnkey->kck, WPA_MIC_KEY_LEN);
		bcopy(wpa->last_replay, rsnkey->replay_counter, EAPOL_KEY_REPLAY_LEN);
		return TRUE;
	}

	return FALSE;
}

bool
wlc_wpa_sup_set_rekey_info(struct wlc_sup_info *sup_info,
	wlc_bsscfg_t *cfg, rsn_rekey_params * rsnkey)
{
	supplicant_t *sup = sup_info ? SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg) : NULL;
	wpapsk_t *wpa = sup ? sup->wpa : NULL;

	if (wpa && rsnkey && sup->cfg->associated &&
		(sup->wpa->state == WPA_SUP_KEYUPDATE)) {
		bcopy(rsnkey->replay_counter, wpa->last_replay, EAPOL_KEY_REPLAY_LEN);

		bcopy(sup->wpa->last_replay, sup->wpa->replay, EAPOL_KEY_REPLAY_LEN);

		/* Driver's copy of replay counter is one more than APs */
		wpa_incr_array(sup->wpa->replay, EAPOL_KEY_REPLAY_LEN);

		return TRUE;
	}

	return FALSE;
}
#endif /* WOWL */
