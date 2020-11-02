/*
 * wlc_sup_ccx.c -- module source for leap supplicant.
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
 * $Id: wlc_sup_ccx.c 530688 2015-01-30 19:02:09Z $
 */

#include <wlc_cfg.h>

#ifndef	STA
#error "STA must be defined for wlc_sup_ccx.c"
#endif /* STA */

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
#include <bcmcrypto/bcmccx.h>
#ifdef	BCMSUP_PSK
#include <bcmcrypto/passhash.h>
#include <bcmcrypto/sha1.h>
#endif /* BCMSUP_PSK */
#if defined(BCMSUP_PSK) || defined(WLFBT)
#include <bcmcrypto/prf.h>
#endif /* BCMSUP_PSK || WLFBT */
#include <proto/802.11.h>
#include <proto/802.11_ccx.h>
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
#include <wlc_wpa.h>
#include <wlc_sup.h>
#include <wlc_sup_ccx.h>

/* msg retry settings: */
#define LEAP_MAX_RETRY		3		/* no. of retries */
#define LEAP_TIMER_MSECS	30000		/* msecs between retries */
#define LEAP_START_DELAY	500		/* msecs delay for 1st START */
#define LEAP_HELD_DELAY		30000		/* msecs delay between retries */

typedef struct wlc_ccxsup_priv {
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_pub_t *pub;			/* pointer to wlc public portion */
	void *wl;			/* per-port handle */
	osl_t *osh;			/* PKT* stuff wants this */

	uint16 bss_ccxsup_priv_offset;	/* offset of priv cubby in bsscfg */
} wlc_ccxsup_priv_t;

struct wlc_ccxsup_info {
	wlc_ccxsup_pub_t	mod_pub;
	wlc_ccxsup_priv_t	mod_priv;
};

/* ioctl table */
static const wlc_ioctl_cmd_t wlc_ccxsup_ioctls[] = {
	{WLC_SET_LEAP_LIST, WLC_IOCF_BSSCFG_STA_ONLY, sizeof(wl_leap_list_t)},
	{WLC_GET_LEAP_LIST, WLC_IOCF_BSSCFG_STA_ONLY, sizeof(wl_leap_list_t)},
};

/* type for saving rougue AP reports */
typedef struct {
	bool   valid;			/* flags used entry */
	struct ether_addr ap_mac;	/* rogue MAC address */
	uint16 reason;			/* auth failure reason code */
	uint8  ap_name_len;		/* length of rogue AP name */
	uint8  ap_name[32];		/* rogue AP name */
} wlc_ccx_rogue_t;

#define LEAP_ROGUE_NUM		20	/* No. of rogue report table entries */

typedef enum {
	CCX_IDLE = 0,
	CCX_INIT,
	CCX_STARTED,
	CCX_IDREQD,
	CCX_IDED,
	CCX_CHALLENGED,
	CCX_CHALLENGING,
	CCX_NOTIFIED,
	CCX_HELD,
	CCX_AUTHENTICATED,
	CCX_KEYED
} leapsup_status_t;

typedef struct {
	/* database to save rogue APs */
	wlc_ccx_rogue_t	rogue_ap[LEAP_ROGUE_NUM];

	/* The remaining fields are specific to an authentication and
	 * are cleared when the authentication fails or is known to end.
	 */
	leapsup_status_t status;
	bool auth_pending;
	unsigned char type;
	unsigned char id;
	uint8 start_count;		/* eapol start counter */
	uint8 retry_count;		/* ID request counter */
	uint8 challenge_count;		/* challenge retry counter */
	uint16 name_len;
	uint8 name[LEAP_USER_MAX+LEAP_DOMAIN_MAX+1];
	uint8 replay[EAPOL_KEY_REPLAY_LEN];
	uchar pwhash[CCX_PW_HASH_LEN];
	struct wl_timer *timer;		/* interval timer cookie */
	struct ether_addr last_bssid;	/* last bssid */
	uint8 last_ssid[32];		/* last ssid */
	uint8 last_ssid_len;

	/* Keep the following 5 items in order so they can be fed to
	 * MD5 from this buffer.
	 */
	uchar pwhashhash[CCX_PW_HASH_LEN];
	uchar net_challenge[LEAP_CHALLENGE_LEN];
	uchar net_response[LEAP_RESPONSE_LEN];
	uchar peer_challenge[LEAP_CHALLENGE_LEN];
	uchar peer_response[LEAP_RESPONSE_LEN];

	/* Keep the following two items contiguous */
	uchar key_iv[EAPOL_KEY_IV_LEN];
	uchar session_key[CCX_SESSION_KEY_LEN];
} leap_sup_t;

typedef struct bss_ccxsup_priv {
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_pub_t *pub;			/* pointer to wlc public portion */
	void *wl;			/* per-port handle */
	osl_t *osh;			/* PKT* stuff wants this */
	wlc_bsscfg_t *cfg;		/* pointer to sup's bsscfg */
	wlc_ccxsup_info_t *m_handle;
	wpapsk_t *wpa;
	wpapsk_info_t *wpa_info;

	/* LEAP parameters poked down via WLC_SET_LEAP_LIST ioctl */
	wl_leap_list_t *leap_list;
	/* LEAP list parameters for current BSS */
	wl_leap_info_t *leap_info;
	leap_sup_t *leap;		/* volatile, initialized in set_sup */
#if defined(BCMSUP_PSK)
	uint32 rn;	/* reassociation request number (refreshed per session key) */
	/* fields of CCKM key hierarchy */
	uint8 key_refresh_key[CCKM_KRK_LEN];
	uint8 base_transient_key[CCKM_BTK_LEN];
#endif // endif
} bss_ccxsup_priv_t;

struct bss_ccxsup_info {
	bss_ccxsup_priv_t	bss_priv;
};
typedef struct bss_ccxsup_info bss_ccxsup_info_t;

/* wlc_ccxsup_info_priv_t offset in module states */
static uint16 wlc_ccxsup_info_priv_offset = OFFSETOF(wlc_ccxsup_info_t, mod_priv);

#define WLC_CCXSUP_PRIV_INFO(ccxsup_info)	((wlc_ccxsup_priv_t *)((uint8 *)ccxsup_info + \
	wlc_ccxsup_info_priv_offset))

#define CCXSUP_BSSCFG_CUBBY_LOC(ccxsup, cfg) \
	((bss_ccxsup_info_t **)BSSCFG_CUBBY(cfg, (ccxsup)->mod_pub.cfgh))
#define CCXSUP_BSSCFG_CUBBY(ccxsup, cfg) (*CCXSUP_BSSCFG_CUBBY_LOC(ccxsup, cfg))
#define BSS_PRIV_OFFSET(ccxsup)	((WLC_CCXSUP_PRIV_INFO(ccxsup))->bss_ccxsup_priv_offset)
#define CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup, cfg) \
	((bss_ccxsup_priv_t *)((uint8 *)(CCXSUP_BSSCFG_CUBBY(ccxsup, cfg))+ \
	BSS_PRIV_OFFSET(ccxsup)))

/* Simplify maintenance of references to driver `common' items. */
#define UNIT(ptr)	((ptr)->pub->unit)
#define CUR_EA(ptr)	((ptr)->cfg->cur_etheraddr)
#define BSS_EA(ptr)	((ptr)->cfg->BSSID)
#define BSS_SSID(ptr)	((ptr)->cfg->current_bss->SSID)
#define BSS_SSID_LEN(ptr)	((ptr)->cfg->current_bss->SSID_len)
#define OSH(ptr)	((ptr)->osh)

/*
 * Cisco Client Extensions (CCX) LEAP supplicant.
 */
/*
 * Internal inferfaces:
 *
 */

static void wlc_leap_rogue_clear(bss_ccxsup_priv_t *ccxsup_bss_priv);
static void wlc_leap_ddp_rogue_rpt(bss_ccxsup_priv_t *ccxsup_bss_priv);
static void wlc_leapsup_sendeap(bss_ccxsup_priv_t *ccxsup_bss_priv);
static void wlc_leapsup_timer(void *arg);
static void wlc_leapsup_timer_callback(bss_ccxsup_priv_t *ccxsup_bss_priv);
static void wlc_ccxsup_bss_updn(void *ctx, bsscfg_up_down_event_data_t *evt);
static void wlc_ccxsup_up_down_callbk(void *ctx, sup_init_event_data_t *evt);
static int wlc_ccxsup_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif);
/* Handle WLC_[GS]ET_LEAP_LIST ioctls */
static int
wlc_ccx_set_leap_list(bss_ccxsup_priv_t *ccxsup_bss_priv, bss_ccxsup_info_t *ccxsup_bss,
	wlc_bsscfg_t *cfg, void *pl);
static int
wlc_ccx_get_leap_list(bss_ccxsup_priv_t *ccxsup_bss_priv, bss_ccxsup_info_t *ccxsup_bss,
	wlc_bsscfg_t *cfg, void *pl);

static const uint8 ccx_rogue_snap[CCX_DDP_LLC_SNAP_LEN] =
	{ 0xAA, 0xAA, 0x03, 0x00, 0x40, 0x96, 0x00, 0x00 };

wlc_ccxsup_info_t *
BCMATTACHFN(wlc_ccxsup_attach)(wlc_info_t *wlc)
{
	wlc_ccxsup_info_t *ccxsup_info;
	wlc_ccxsup_priv_t	*ccxsup_priv;

	WL_TRACE(("wl%d: wlc_ccxsup_attach\n", wlc->pub->unit));

	if (!(ccxsup_info = (wlc_ccxsup_info_t *)MALLOC(wlc->osh, sizeof(wlc_ccxsup_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	bzero((char *)ccxsup_info, sizeof(wlc_ccxsup_info_t));
	ccxsup_priv = WLC_CCXSUP_PRIV_INFO(ccxsup_info);
	ccxsup_priv->wlc = wlc;
	ccxsup_priv->pub = wlc->pub;
	ccxsup_priv->wl = wlc->wl;
	ccxsup_priv->osh = wlc->osh;
	ccxsup_priv->bss_ccxsup_priv_offset = OFFSETOF(bss_ccxsup_info_t, bss_priv);

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((ccxsup_info->mod_pub.cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_ccxsup_info_t *),
		NULL /* wlc_sup_init */, wlc_ccxsup_deinit, NULL,
		(void *)ccxsup_info)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
			UNIT(ccxsup_priv), __FUNCTION__));
			goto err;
	}

	if (wlc_sup_up_down_register(wlc, wlc_ccxsup_up_down_callbk, ccxsup_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_sup_up_down_register() failed\n",
			UNIT(ccxsup_priv), __FUNCTION__));
		goto err;
	}

	/* bsscfg up/down callback */
	if (wlc_bsscfg_updown_register(wlc, wlc_ccxsup_bss_updn, ccxsup_info) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_updown_register() failed\n",
			UNIT(ccxsup_priv), __FUNCTION__));
		goto err;
	}

	if (wlc_module_add_ioctl_fn(wlc->pub, ccxsup_info, wlc_ccxsup_doioctl,
		ARRAYSIZE(wlc_ccxsup_ioctls), wlc_ccxsup_ioctls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_add_ioctl_fn() failed\n", wlc->pub->unit,
			__FUNCTION__));
		goto err;
	}

	return ccxsup_info;

err:
	wlc_ccxsup_detach(ccxsup_info);
	return NULL;
}

void
BCMATTACHFN(wlc_ccxsup_detach)(wlc_ccxsup_info_t *ccxsup_info)
{
	wlc_ccxsup_priv_t *ccxsup_priv;

	if (!ccxsup_info)
		return;

	ccxsup_priv = WLC_CCXSUP_PRIV_INFO(ccxsup_info);

	WL_TRACE(("wl%d: wlc_ccxsup_detach\n", UNIT(ccxsup_priv)));

	wlc_module_remove_ioctl_fn(ccxsup_priv->wlc->pub, ccxsup_info);
	wlc_bsscfg_updown_unregister(ccxsup_priv->wlc, wlc_ccxsup_bss_updn, ccxsup_info);
	wlc_sup_up_down_unregister(ccxsup_priv->wlc, wlc_ccxsup_up_down_callbk, ccxsup_info);
	/* wlc_module_unregister(ccxsup_priv->pub, "idsup", ccxsup_info); */
	MFREE(ccxsup_priv->osh, ccxsup_info, sizeof(wlc_ccxsup_info_t));
}

static int
wlc_ccxsup_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	wlc_ccxsup_info_t *ccxsup_info = (wlc_ccxsup_info_t *)ctx;
	wlc_ccxsup_priv_t	*ccxsup_priv;

	bss_ccxsup_priv_t *ccxsup_bss_priv;
	bss_ccxsup_info_t *ccxsup_bss;

	wlc_info_t *wlc;

	wlc_bsscfg_t *bsscfg;
	int val, *pval;
	int err = BCME_OK;

	if (ccxsup_info == NULL)
		return BCME_ERROR;

	ccxsup_priv = WLC_CCXSUP_PRIV_INFO(ccxsup_info);
	wlc = ccxsup_priv->wlc;

	/* default argument is generic integer */
	pval = (int *) arg;

	/* This will prevent the misaligned access */
	if (pval && (uint32)len >= sizeof(val))
		bcopy(pval, &val, sizeof(val));
	else
		val = 0;

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);

	ASSERT(bsscfg != NULL);
	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, bsscfg);
	ccxsup_bss = CCXSUP_BSSCFG_CUBBY(ccxsup_info, bsscfg);

	if (BSSCFG_AP(bsscfg)) {
		err = BCME_BADARG;
		goto exit;
	}

	switch (cmd) {
	case WLC_SET_LEAP_LIST:
		err = wlc_ccx_set_leap_list(ccxsup_bss_priv, ccxsup_bss, bsscfg, pval);
		break;
	case WLC_GET_LEAP_LIST:
		ASSERT(pval != NULL);
		err = wlc_ccx_get_leap_list(ccxsup_bss_priv, ccxsup_bss, bsscfg, pval);
		break;
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

exit:	return err;
}

static
void wlc_ccxsup_up_down_callbk(void *ctx, sup_init_event_data_t *evt)
{
	if (evt->up)
		wlc_ccxsup_init(ctx, evt);
	else
		wlc_ccxsup_deinit(ctx, evt->bsscfg);
}

int
wlc_ccxsup_init(void *ctx, sup_init_event_data_t *evt)
{
	wlc_ccxsup_info_t *ccxsup_info = (wlc_ccxsup_info_t *)ctx;
	wlc_ccxsup_priv_t	*ccxsup_priv;
	wlc_info_t *wlc;
	bss_ccxsup_info_t **pccxsup_bss;
	bss_ccxsup_info_t *ccxsup_bss = NULL;
	bss_ccxsup_priv_t *ccxsup_bss_priv = NULL;

	if (ccxsup_info == NULL)
		return BCME_ERROR;

	ccxsup_priv = WLC_CCXSUP_PRIV_INFO(ccxsup_info);
	wlc = ccxsup_priv->wlc;
	pccxsup_bss = CCXSUP_BSSCFG_CUBBY_LOC(ccxsup_info, evt->bsscfg);

	WL_TRACE(("wl%d: wlc_ccxsup_init\n", UNIT(ccxsup_priv)));

	if (BSSCFG_AP(evt->bsscfg))
		return BCME_NOTSTA;
	if (!(ccxsup_bss = (bss_ccxsup_info_t *)MALLOC(ccxsup_priv->osh,
		sizeof(bss_ccxsup_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(ccxsup_priv), __FUNCTION__, MALLOCED(ccxsup_priv->osh)));
			goto err;
	}
	bzero(ccxsup_bss, sizeof(bss_ccxsup_info_t));
	*pccxsup_bss = ccxsup_bss;

	ccxsup_bss_priv = (bss_ccxsup_priv_t *)((uint8 *)ccxsup_bss + BSS_PRIV_OFFSET(ccxsup_info));

	ccxsup_bss_priv->m_handle = wlc->ccxsup;
	ccxsup_bss_priv->cfg = evt->bsscfg;
	ccxsup_bss_priv->wlc = wlc;
	ccxsup_bss_priv->osh = ccxsup_priv->osh;
	ccxsup_bss_priv->wl = ccxsup_priv->wl;
	ccxsup_bss_priv->pub = ccxsup_priv->pub;
	ccxsup_bss_priv->wpa = evt->wpa;
	ccxsup_bss_priv->wpa_info = evt->wpa_info;

	if (CCX_ENAB(wlc->pub)) {
		if (!(ccxsup_bss_priv->leap = MALLOC(ccxsup_bss_priv->osh, sizeof(leap_sup_t)))) {
			WL_ERROR(("wl%d: wlc_ccxsup_init: out of memory, malloced %d bytes\n",
				UNIT(ccxsup_bss_priv), MALLOCED(ccxsup_bss_priv->osh)));
			goto err;
		}
		bzero(ccxsup_bss_priv->leap, sizeof(leap_sup_t));
		if (!(ccxsup_bss_priv->leap_info =
			MALLOC(ccxsup_bss_priv->osh, sizeof(wl_leap_info_t)))) {
			WL_ERROR(("wl%d: wlc_sup_init: out of memory, malloced %d bytes\n",
				UNIT(ccxsup_bss_priv), MALLOCED(ccxsup_bss_priv->osh)));
			goto err;
		}
		bzero(ccxsup_bss_priv->leap_info, sizeof(wl_leap_info_t));
		if (!(ccxsup_bss_priv->leap->timer =
			wl_init_timer(ccxsup_bss_priv->wl, wlc_leapsup_timer,
			ccxsup_bss_priv, "leap"))) {
			WL_ERROR(("wl%d: wlc_sup_init: wl_init_timer for supp timer failed\n",
				UNIT(ccxsup_bss_priv)));
			goto err;
		}
	}

	return BCME_OK;
err:
	if (ccxsup_bss)
		wlc_ccxsup_deinit(ccxsup_info, evt->bsscfg);
	return BCME_ERROR;
}

void
wlc_ccxsup_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_ccxsup_info_t *ccxsup_info = (wlc_ccxsup_info_t *)ctx;
	bss_ccxsup_info_t **pccxsup_bss;
	bss_ccxsup_info_t *ccxsup_bss;
	bss_ccxsup_priv_t *ccxsup_bss_priv;

	if (ccxsup_info == NULL)
		return;

	pccxsup_bss = CCXSUP_BSSCFG_CUBBY_LOC(ccxsup_info, cfg);
	ccxsup_bss = *pccxsup_bss;
	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);

	if (ccxsup_bss != NULL) {
		WL_TRACE(("wl%d: wlc_ccxsup_deinit\n", UNIT(ccxsup_bss_priv)));
#if defined(BCMINTSUP)
		if (ccxsup_bss_priv->leap) {
			if (ccxsup_bss_priv->leap->timer) {
				wl_free_timer(ccxsup_bss_priv->wl, ccxsup_bss_priv->leap->timer);
			}
			MFREE(ccxsup_bss_priv->osh, ccxsup_bss_priv->leap,
				sizeof(leap_sup_t));
		}

		if (ccxsup_bss_priv->leap_info)
			MFREE(ccxsup_bss_priv->osh, ccxsup_bss_priv->leap_info,
				sizeof(wl_leap_info_t));

		/* toss the leap list if there is one */
		if (ccxsup_bss_priv->leap_list != NULL) {
			MFREE(ccxsup_bss_priv->osh, ccxsup_bss_priv->leap_list,
				ccxsup_bss_priv->leap_list->buflen);
		}
#endif	/* BCMINTSUP */
		MFREE(ccxsup_bss_priv->osh, ccxsup_bss, sizeof(bss_ccxsup_info_t));
		*pccxsup_bss = NULL;
	}
}

static void
wlc_ccxsup_bss_updn(void *ctx, bsscfg_up_down_event_data_t *evt)
{
	wlc_ccxsup_info_t *ccxsup_info = (wlc_ccxsup_info_t *)ctx;
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	bss_ccxsup_info_t *ccxsup_bss;

	if (ccxsup_info == NULL)
		return;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, evt->bsscfg);
	ccxsup_bss = CCXSUP_BSSCFG_CUBBY(ccxsup_info, evt->bsscfg);

	if (!evt->up) {
		if (ccxsup_bss != NULL) {
			if (CCX_ENAB(ccxsup_bss_priv->pub) && (ccxsup_bss_priv->leap != NULL))
				wl_del_timer(ccxsup_bss_priv->wl, ccxsup_bss_priv->leap->timer);
		}
	}
}

static void
wlc_leapsup_timer(void *arg)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv = (bss_ccxsup_priv_t *)arg;
	wlc_info_t *wlc = ccxsup_bss_priv->wlc;

	if (!wlc->pub->up) {
		return;
	}

	if (DEVICEREMOVED(wlc)) {
		WL_ERROR(("wl%d: %s: dead chip\n", UNIT(ccxsup_bss_priv), __FUNCTION__));
		wl_down(wlc->wl);
		return;
	}
	wlc_leapsup_timer_callback(ccxsup_bss_priv);
}

static void
wlc_leap_rogue_clear(bss_ccxsup_priv_t *ccxsup_bss_priv)
{
	wlc_ccx_rogue_t *rogue = ccxsup_bss_priv->leap->rogue_ap;

	WL_TRACE(("wl%d: wlc_leap_rogue_clear: clear rogue list\n", UNIT(ccxsup_bss_priv)));
	do {
		rogue->valid = FALSE;
		rogue->ap_name_len = 0;
	} while (++rogue < ccxsup_bss_priv->leap->rogue_ap + LEAP_ROGUE_NUM);
}

static void
wlc_leap_ddp_rogue_rpt(bss_ccxsup_priv_t *ccxsup_bss_priv)
{
	uint len;
	wlc_ccx_rogue_t *rogue;
	ccx_ddp_pkt_t *ddp_hdr;
	osl_t *osh, *p;
#if defined(BCMDBG) || defined(WLMSG_WSEC)
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG || WLMSG_WSEC */

	WL_TRACE(("wl%d: wlc_leap_ddp_rogue_rpt: report rogue AP\n", UNIT(ccxsup_bss_priv)));

	osh = OSH(ccxsup_bss_priv);

	for (rogue = ccxsup_bss_priv->leap->rogue_ap + LEAP_ROGUE_NUM - 1;
	     rogue >= ccxsup_bss_priv->leap->rogue_ap;
	     rogue--) {
		if (!rogue->valid)
			continue;

		len = CCX_DDP_PKT_LEN;
		if ((p = PKTGET(osh, len + TXOFF, TRUE)) == NULL) {
			WL_ERROR(("wl%d: %s: pktget error for len %d\n",
			          UNIT(ccxsup_bss_priv), __FUNCTION__, len));
			WLCNTINCR(ccxsup_bss_priv->pub->_cnt->txnobuf);
			return;
		}
		ASSERT(ISALIGNED(PKTDATA(osh, p), sizeof(uint32)));

		/* reserve TXOFF bytes of headroom */
		PKTPULL(osh, p, TXOFF);
		PKTSETLEN(osh, p, len);

		ddp_hdr = (ccx_ddp_pkt_t *)PKTDATA(osh, p);
		bcopy(&BSS_EA(ccxsup_bss_priv), &ddp_hdr->eth.ether_dhost, ETHER_ADDR_LEN);
		bcopy(&CUR_EA(ccxsup_bss_priv), &ddp_hdr->eth.ether_shost, ETHER_ADDR_LEN);
		ddp_hdr->eth.ether_type = hton16(DOT11_LLC_SNAP_HDR_LEN + CCX_DDP_MSG_LEN);

		bcopy(ccx_rogue_snap, &ddp_hdr->snap, CCX_DDP_LLC_SNAP_LEN);

		ddp_hdr->msg_len = hton16(CCX_DDP_MSG_LEN);
		ddp_hdr->msg_type = 0x40;
		ddp_hdr->fcn_code = 0x8e;
		bcopy(&BSS_EA(ccxsup_bss_priv), &ddp_hdr->dest_mac, ETHER_ADDR_LEN);
		bcopy(&CUR_EA(ccxsup_bss_priv), &ddp_hdr->src_mac, ETHER_ADDR_LEN);
		ddp_hdr->fail_reason = hton16(rogue->reason);
		bcopy(&rogue->ap_mac, &ddp_hdr->rogue_mac, ETHER_ADDR_LEN);
		if (rogue->ap_name_len > 0)
			bcopy(rogue->ap_name, ddp_hdr->rogue_name, rogue->ap_name_len);
		else
			ddp_hdr->rogue_name[0] = '\0';

		WL_WSEC(("wl%d: wlc_leap_ddp_rogue_rpt: reporting rogue AP %s\n",
			UNIT(ccxsup_bss_priv), bcm_ether_ntoa(&rogue->ap_mac, eabuf)));
		/* send it out */
		rogue->valid = FALSE;
		wlc_sendpkt(ccxsup_bss_priv->wlc, p, ccxsup_bss_priv->cfg->wlcif);
	}
}

/* Return whether the given SSID has a match in the LEAP list. */
bool
wlc_ccx_leap_ssid(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg, uchar SSID[], int len)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	bss_ccxsup_info_t *ccxsup_bss = CCXSUP_BSSCFG_CUBBY(ccxsup_info, cfg);
	wl_leap_info_t *li;
	int i = 0;

	if (ccxsup_bss == NULL)
		return FALSE;

	/* Check that there is a list and that it has an entry */
	if ((ccxsup_bss_priv->leap_list == NULL) || (ccxsup_bss_priv->leap_list->count == 0))
	    return FALSE;

	/* If SSID matches one in the leap list, the matching list
	 * item is current leap info.
	 */
	li = ccxsup_bss_priv->leap_list->leap_info;
	do {
		if ((len == (int) li->ssid.SSID_len) && !bcmp(SSID, li->ssid.SSID, len)) {
			WL_WSEC(("wl%d: ssid match in leap list\n", UNIT(ccxsup_bss_priv)));
			/* Need to work from a copy of the matching item
			 * because the list could change at any time.
			 */
			bcopy(li, ccxsup_bss_priv->leap_info, sizeof(wl_leap_info_t));

			/* Update the supplicant structure with the user info as well */
			/* Don't use domain if it isn't furnished. */
			if (ccxsup_bss_priv->leap_info->domain_len != 0) {
				bcopy(ccxsup_bss_priv->leap_info->domain,
					ccxsup_bss_priv->leap->name,
					ccxsup_bss_priv->leap_info->domain_len);
				ccxsup_bss_priv->leap->name[ccxsup_bss_priv->leap_info->domain_len]
					= '\\';
				i = ccxsup_bss_priv->leap_info->domain_len + 1;
			}
			bcopy(ccxsup_bss_priv->leap_info->user,
				&ccxsup_bss_priv->leap->name[i],
				ccxsup_bss_priv->leap_info->user_len);
			ccxsup_bss_priv->leap->name_len = ccxsup_bss_priv->leap_info->user_len + i;
			return TRUE;
		}
	} while (++li < (ccxsup_bss_priv->leap_list->leap_info +
		ccxsup_bss_priv->leap_list->count));
	return FALSE;
}

/* Compose and send an EAP packet.  How to compose it is determined
 * from the supplicant's LEAP context.
 */
static void
wlc_leapsup_sendeap(bss_ccxsup_priv_t *ccxsup_bss_priv)
{
	uint16 len;
	void *p = NULL;
	uint timeout = 0;	/* no timeout by default */
	eapol_header_t *eapol_hdr = NULL;
	eap_header_t *eap_hdr;
	leap_challenge_t *leap_chall;
	leap_response_t *leap_resp;
	osl_t *osh = OSH(ccxsup_bss_priv);

	len = (uint16) ccxsup_bss_priv->leap->name_len;
	switch (ccxsup_bss_priv->leap->status) {
	case CCX_STARTED:
		if ((p = wlc_eapol_pktget(ccxsup_bss_priv->wlc, ccxsup_bss_priv->cfg,
			&BSS_EA(ccxsup_bss_priv), EAPOL_HEADER_LEN)) == NULL)
			break;
		eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
		eapol_hdr->type = EAPOL_START;
		eapol_hdr->length = 0;
		WL_WSEC(("wl%d: wlc_leapsup_sendeap: sending EAPOL START %d\n",
			UNIT(ccxsup_bss_priv), ccxsup_bss_priv->leap->start_count));
		timeout = LEAP_TIMER_MSECS;	/* send START again when timeout */
		ccxsup_bss_priv->leap->start_count ++;
		break;

	case CCX_IDED:
		if (ccxsup_bss_priv->leap->type == EAP_IDENTITY)
			len += EAP_HEADER_LEN + 1;
		else
			len += EAP_HEADER_LEN + 2;
		if ((p = wlc_eapol_pktget(ccxsup_bss_priv->wlc, ccxsup_bss_priv->cfg,
			&BSS_EA(ccxsup_bss_priv), EAPOL_HEADER_LEN + len)) == NULL)
			break;
		/* Fill in an identity response */
		eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
		eapol_hdr->type = EAP_PACKET;
		eapol_hdr->length = hton16(len);
		eap_hdr = (eap_header_t *) eapol_hdr->body;
		/* Use id from the request and remember it. */
		eap_hdr->code = EAP_RESPONSE;
		eap_hdr->id = (uchar) ccxsup_bss_priv->leap->id;
		eap_hdr->length = hton16(len);
		if (ccxsup_bss_priv->leap->type == EAP_IDENTITY) {
			eap_hdr->type = EAP_IDENTITY;
			bcopy(ccxsup_bss_priv->leap->name, eap_hdr->data,
				ccxsup_bss_priv->leap->name_len);
			WL_WSEC(("wl%d: wlc_leapsup_sendeap: sending EAP identity response\n",
				UNIT(ccxsup_bss_priv)));
		}
		else {
			eap_hdr->type = EAP_NAK;
			eap_hdr->data[0] = EAP_LEAP;
			WL_WSEC(("wl%d: wlc_leapsup_sendeap: sending EAP NAK response\n",
				UNIT(ccxsup_bss_priv)));
		}
		timeout = LEAP_TIMER_MSECS;	/* send START again when timeout */
		break;

	case CCX_CHALLENGED:
		len += EAP_HEADER_LEN + LEAP_RESPONSE_HDR_LEN;
		if ((p = wlc_eapol_pktget(ccxsup_bss_priv->wlc, ccxsup_bss_priv->cfg,
			&BSS_EA(ccxsup_bss_priv), EAPOL_HEADER_LEN + len)) == NULL)
			break;
		/* Fill in a LEAP response */
		eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
		eapol_hdr->type = EAP_PACKET;
		eapol_hdr->length = hton16(len);
		eap_hdr = (eap_header_t *) eapol_hdr->body;
		eap_hdr->code = EAP_RESPONSE;
		eap_hdr->id = (uchar) ccxsup_bss_priv->leap->id;
		eap_hdr->length = hton16(len);
		eap_hdr->type = EAP_LEAP;
		leap_resp = (leap_response_t *) eap_hdr->data;
		leap_resp->version = LEAP_VERSION;
		leap_resp->reserved = 0;
		leap_resp->resp_len = LEAP_RESPONSE_LEN;

		/* password hash deferred until it's needed in a msg */
		bcm_ccx_hashpwd(ccxsup_bss_priv->leap_info->password,
		                ccxsup_bss_priv->leap_info->password_len,
		                ccxsup_bss_priv->leap->pwhash, ccxsup_bss_priv->leap->pwhashhash);

		/* cobble challenge response */
		bcm_ccx_leap_response(ccxsup_bss_priv->leap->pwhash,
			ccxsup_bss_priv->leap->peer_challenge,
			leap_resp->response);
		bcopy(leap_resp->response, ccxsup_bss_priv->leap->peer_response, LEAP_RESPONSE_LEN);
		bcopy(ccxsup_bss_priv->leap->name, leap_resp->username,
			ccxsup_bss_priv->leap->name_len);
		WL_WSEC(("wl%d: wlc_leapsup_sendeap: sending LEAP challenge response\n",
		         UNIT(ccxsup_bss_priv)));
		timeout = LEAP_TIMER_MSECS;	/* send START again when timeout */
		break;

	case CCX_CHALLENGING:
		len += EAP_HEADER_LEN + LEAP_CHALLENGE_HDR_LEN;
		if ((p = wlc_eapol_pktget(ccxsup_bss_priv->wlc, ccxsup_bss_priv->cfg,
			&BSS_EA(ccxsup_bss_priv), EAPOL_HEADER_LEN + len)) == NULL)
			break;
		/* Fill in a client challenge */
		eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
		eapol_hdr->type = EAP_PACKET;
		eapol_hdr->length = hton16(len);
		eap_hdr = (eap_header_t *) eapol_hdr->body;
		eap_hdr->code = EAP_REQUEST;
		eap_hdr->id = (uchar) ccxsup_bss_priv->leap->id;
		eap_hdr->length = hton16(len);
		eap_hdr->type = EAP_LEAP;
		leap_chall = (leap_challenge_t *) eap_hdr->data;
		leap_chall->version = LEAP_VERSION;
		leap_chall->reserved = 0;
		leap_chall->chall_len = LEAP_CHALLENGE_LEN;

		/* cobble a challenge */
		wlc_getrand(ccxsup_bss_priv->wlc, ccxsup_bss_priv->leap->net_challenge,
		            LEAP_CHALLENGE_LEN);
		bcopy(ccxsup_bss_priv->leap->net_challenge,
			leap_chall->challenge, LEAP_CHALLENGE_LEN);
		bcopy(ccxsup_bss_priv->leap->name, leap_chall->username,
			ccxsup_bss_priv->leap->name_len);
		WL_WSEC(("wl%d: wlc_leapsup_sendeap: sending LEAP challenge request %d\n",
		         UNIT(ccxsup_bss_priv), ccxsup_bss_priv->leap->challenge_count));
		timeout = LEAP_TIMER_MSECS;	/* send challenge again when timeout */
		ccxsup_bss_priv->leap->challenge_count ++;
		break;

	case CCX_NOTIFIED:
		if ((p = wlc_eapol_pktget(ccxsup_bss_priv->wlc, ccxsup_bss_priv->cfg,
			&BSS_EA(ccxsup_bss_priv), EAPOL_HEADER_LEN)) == NULL)
			break;
		eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
		eapol_hdr->type = EAP_PACKET;
		eapol_hdr->length = hton16(EAP_HEADER_LEN);
		eap_hdr = (eap_header_t *) eapol_hdr->body;
		eap_hdr->code = EAP_RESPONSE;
		eap_hdr->id = (uchar) ccxsup_bss_priv->leap->id;
		eap_hdr->length = hton16(EAP_HEADER_LEN + 1);
		eap_hdr->type = EAP_NOTIFICATION;
		WL_WSEC(("wl%d: wlc_leapsup_sendeap: sending EAP notification response\n",
		         UNIT(ccxsup_bss_priv)));
		break;

	case CCX_IDREQD:
	case CCX_AUTHENTICATED:
	/* Fall through -- shouldn't be sending in this state. */
	default:
		WL_ERROR(("wl%d: %s: unexpected CCX status %d\n",
		          UNIT(ccxsup_bss_priv), __FUNCTION__, ccxsup_bss_priv->leap->status));
		break;
	}
	if (p != NULL) {
		wlc_sendpkt(ccxsup_bss_priv->wlc, p, ccxsup_bss_priv->cfg->wlcif);
		if (timeout)
			wl_add_timer(ccxsup_bss_priv->wl, ccxsup_bss_priv->leap->timer, timeout, 0);
	}
}

static void
wlc_leapsup_timer_callback(bss_ccxsup_priv_t *ccxsup_bss_priv)
{
	bss_ccxsup_info_t *ccxsup_bss = CCXSUP_BSSCFG_CUBBY(ccxsup_bss_priv->m_handle,
		ccxsup_bss_priv->cfg);

	if (ccxsup_bss == NULL)
		return;

	switch (ccxsup_bss_priv->leap->status) {
	case CCX_STARTED:
	case CCX_HELD:
		ccxsup_bss_priv->leap->status = CCX_STARTED;
		/* IEEE Std 802.1X-2001 Sect 8.5.10 */
		if (ccxsup_bss_priv->leap->start_count < LEAP_MAX_RETRY)
			wlc_leapsup_sendeap(ccxsup_bss_priv);
		else {
			WL_WSEC(("wl%d: wlc_leapsup_timer_callback: reach max # EAPOL START count,"
			         " move to state %d\n",
			         UNIT(ccxsup_bss_priv), CCX_AUTHENTICATED));
			ccxsup_bss_priv->leap->status = CCX_AUTHENTICATED;
			ccxsup_bss_priv->leap->auth_pending = FALSE;
			/* bump it to trigger timeout status */
			ccxsup_bss_priv->leap->start_count = LEAP_MAX_RETRY + 1;
		}
		break;
	case CCX_IDED:
	case CCX_CHALLENGED:
		/* IEEE Std 802.1X-2001 Sect 8.5.10 */
		WL_WSEC(("wl%d: wlc_leapsup_timer_callback: timeout in state %d, move to state"
		         " %d\n",
		         UNIT(ccxsup_bss_priv), ccxsup_bss_priv->leap->status, CCX_STARTED));
		ccxsup_bss_priv->leap->status = CCX_STARTED;
		wlc_leapsup_sendeap(ccxsup_bss_priv);
		break;
	case CCX_CHALLENGING:
		/* RFC2284 2.2.1 */
		if (ccxsup_bss_priv->leap->challenge_count < LEAP_MAX_RETRY)
			wlc_leapsup_sendeap(ccxsup_bss_priv);
		else {
			WL_WSEC(("wl%d: wlc_leapsup_timer_callback: reach max # LEAP challenge"
			         " count, move to state %d\n",
			         UNIT(ccxsup_bss_priv), CCX_IDLE));
			ccxsup_bss_priv->leap->auth_pending = FALSE;
			ccxsup_bss_priv->leap->status = CCX_IDLE;
		}
		break;
	default:
		WL_ERROR(("wl%d: %s: wrong state %d\n",
		          UNIT(ccxsup_bss_priv), __FUNCTION__, ccxsup_bss_priv->leap->status));
		break;
	}
}

bool
wlc_ccx_authenticated(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	bss_ccxsup_info_t *ccxsup_bss;

	if (ccxsup_info == NULL)
		return FALSE;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	ccxsup_bss = CCXSUP_BSSCFG_CUBBY(ccxsup_info, cfg);

	if (ccxsup_bss == NULL)
		return FALSE;

	return ccxsup_bss_priv->leap->status == CCX_KEYED;
}

#define EAP_REJECT	"Rejected\n\r"
#define EAP_REJECT_LEN	(sizeof(EAP_REJECT) - 1)

/* Dispose of a received EAP message. */
/* auth_pending is only ever FALSE if a rogue report has been made.
 * That looks bogus, but the point of it is to make it look as though
 * authentication is in progress until a longer, higher-level timer can
 * report a LEAP timeout.
 */
static void
wlc_leapsup_recveap(bss_ccxsup_priv_t *ccxsup_bss_priv, eap_header_t *rx_eap_hdr)
{
	wlc_ccxsup_info_t *ccxsup_info = ccxsup_bss_priv->m_handle;
	uint16 len;
	leap_challenge_t *leap_chall;
	leap_response_t *leap_resp;
	leapsup_status_t status;

	/* This much of the length is common, so hoist the load. */
	len = ccxsup_bss_priv->leap->name_len;

	/* What kind of EAP is this? */
	switch (rx_eap_hdr->code) {
	case EAP_REQUEST:

		ccxsup_bss_priv->leap->type = rx_eap_hdr->type;

		/* Is it an identity request? */
		if (rx_eap_hdr->type == EAP_IDENTITY) {

			WL_WSEC(("wl%d: %s: received EAP identity request\n",
				UNIT(ccxsup_bss_priv), __FUNCTION__));

			/* Identity request makes sense only early in
			 * authentication or late for reauthentication.
			 */
			if ((ccxsup_bss_priv->leap->status > CCX_IDED) &&
			    (ccxsup_bss_priv->leap->status < CCX_KEYED)) {
				WL_ERROR(("wl%d: %s: unexpected EAP identity"
				          " request\n",
				          UNIT(ccxsup_bss_priv), __FUNCTION__));
				break;
			}
			(void) wl_del_timer(ccxsup_bss_priv->wl, ccxsup_bss_priv->leap->timer);
			ccxsup_bss_priv->leap->id = rx_eap_hdr->id;
			/* Check if we have the identity info to respond */
			if (ccxsup_bss_priv->leap_info->user_len) {
				ccxsup_bss_priv->leap->status = CCX_IDED;
				/*
				 * make sure this counter doesn't get wrapped around but
				 * still reflects 'timeout' condition
				 */
				if (ccxsup_bss_priv->leap->retry_count <= LEAP_MAX_RETRY)
					ccxsup_bss_priv->leap->retry_count ++;
				wlc_leapsup_sendeap(ccxsup_bss_priv);
			}
			else
				ccxsup_bss_priv->leap->status = CCX_IDREQD;
			/* reset START counter per IEEE 802.1X Std Sect 8.5.10 */
			ccxsup_bss_priv->leap->start_count = 0;

		} else if (rx_eap_hdr->type == EAP_LEAP) {

			WL_WSEC(("wl%d: %s: received LEAP challenge request\n",
			         UNIT(ccxsup_bss_priv), __FUNCTION__));

			leap_chall = (leap_challenge_t *)rx_eap_hdr->data;
			/* Check LEAP version and how much is there */
			if ((ntoh16(rx_eap_hdr->length) < EAP_HEADER_LEN +
			     LEAP_CHALLENGE_HDR_LEN) ||
			    (leap_chall->version != LEAP_VERSION)) {
				WL_ERROR(("wl%d: %s: bad LEAP challenge length %d"
					" or version %d\n",
					UNIT(ccxsup_bss_priv), __FUNCTION__,
					ntoh16(rx_eap_hdr->length),
					leap_chall->version));
				break;
			}

			/* check state and that this is for our username. */
			if (ccxsup_bss_priv->leap->status != CCX_IDED) {
				WL_ERROR(("wl%d: %s: unexpected LEAP challenge\n",
				          UNIT(ccxsup_bss_priv), __FUNCTION__));
				break;
			}
			(void) wl_del_timer(ccxsup_bss_priv->wl, ccxsup_bss_priv->leap->timer);
			/* save the challenge */
			bcopy(leap_chall->challenge, ccxsup_bss_priv->leap->peer_challenge,
				LEAP_CHALLENGE_LEN);
			ccxsup_bss_priv->leap->id = rx_eap_hdr->id;
			ccxsup_bss_priv->leap->status = CCX_CHALLENGED;
			wlc_leapsup_sendeap(ccxsup_bss_priv);

		} else if (rx_eap_hdr->type == EAP_NOTIFICATION) {

			WL_WSEC(("wl%d: %s: received EAP notification\n",
			         UNIT(ccxsup_bss_priv), __FUNCTION__));

			if (rx_eap_hdr->id != ccxsup_bss_priv->leap->id) {
				WL_ERROR(("wl%d: %s: EAP notication id %d,"
				          " expected id %d\n",
				          UNIT(ccxsup_bss_priv), __FUNCTION__, rx_eap_hdr->id,
				          ccxsup_bss_priv->leap->id));
			}

			wl_del_timer(ccxsup_bss_priv->wl, ccxsup_bss_priv->leap->timer);
			len = ntoh16(rx_eap_hdr->length);
			/* RFC2284 3.2 says we *must* respond. */
			status = ccxsup_bss_priv->leap->status;
			ccxsup_bss_priv->leap->status = CCX_NOTIFIED;
			wlc_leapsup_sendeap(ccxsup_bss_priv);
			ccxsup_bss_priv->leap->status = status;
			/* check for rejection notification */
			if ((len > (EAP_HEADER_LEN + EAP_REJECT_LEN)) &&
			    !bcmp(rx_eap_hdr->data, EAP_REJECT, EAP_REJECT_LEN)) {
				WL_WSEC(("wl%d: %s: EAP reject notice\n",
				         UNIT(ccxsup_bss_priv), __FUNCTION__));
				wlc_ccx_rogueap_update(ccxsup_info, ccxsup_bss_priv->cfg,
					(uint16)CCX_ROGUE_CHAN_FROM_AP,
				                       &BSS_EA(ccxsup_bss_priv));
				ccxsup_bss_priv->leap->status = CCX_IDLE;
				ccxsup_bss_priv->leap->auth_pending = FALSE;
				break;
			}
			/* What other notification is expected here? */
			WL_ERROR(("wl%d: %s: Unexpected EAP request notication\n",
			          UNIT(ccxsup_bss_priv), __FUNCTION__));
		} else {

			WL_WSEC(("wl%d: %s: received EAP type %d request\n",
				UNIT(ccxsup_bss_priv), __FUNCTION__, rx_eap_hdr->type));

			/* check state */
			if ((ccxsup_bss_priv->leap->status != CCX_IDED)) {
				WL_ERROR(("wl%d: %s: unexpected EAP request\n",
				          UNIT(ccxsup_bss_priv), __FUNCTION__));
				break;
			}
			(void) wl_del_timer(ccxsup_bss_priv->wl, ccxsup_bss_priv->leap->timer);
			/* NAK the request */
			ccxsup_bss_priv->leap->id = rx_eap_hdr->id;
			wlc_leapsup_sendeap(ccxsup_bss_priv);
		}
		break;

	case EAP_RESPONSE:
		WL_WSEC(("wl%d: %s: received LEAP challenge response\n",
		         UNIT(ccxsup_bss_priv), __FUNCTION__));

		/* Check that the response is long enough, is LEAP and
		 * has the right username.
		 */
		leap_resp = (leap_response_t *)rx_eap_hdr->data;
		if ((ntoh16(rx_eap_hdr->length) < EAP_HEADER_LEN + LEAP_RESPONSE_HDR_LEN + len) ||
		    (rx_eap_hdr->type != EAP_LEAP) ||
		    (leap_resp->version != LEAP_VERSION) ||
		    (leap_resp->resp_len != LEAP_RESPONSE_LEN) ||
		    (bcmp(leap_resp->username, ccxsup_bss_priv->leap->name, len))) {
			WL_ERROR(("wl%d: %s: unexpected EAP response",
			          UNIT(ccxsup_bss_priv), __FUNCTION__));
			wlc_ccx_rogueap_update(ccxsup_info, ccxsup_bss_priv->cfg,
				CCX_ROGUE_CHAN_TO_AP, &BSS_EA(ccxsup_bss_priv));
			ccxsup_bss_priv->leap->auth_pending = FALSE;
			break;
		}
		(void) wl_del_timer(ccxsup_bss_priv->wl, ccxsup_bss_priv->leap->timer);
		/* check the response text */
		bcm_ccx_leap_response(ccxsup_bss_priv->leap->pwhashhash,
		                      ccxsup_bss_priv->leap->net_challenge,
		                      ccxsup_bss_priv->leap->net_response);
		if (bcmp(ccxsup_bss_priv->leap->net_response, leap_resp->response,
			LEAP_RESPONSE_LEN)) {
			WL_WSEC(("wl%d: %s: challenge response failure; reporting"
				" as rogue AP\n",
				UNIT(ccxsup_bss_priv), __FUNCTION__));
			wlc_ccx_rogueap_update(ccxsup_info, ccxsup_bss_priv->cfg,
				(uint16)CCX_ROGUE_CHAN_TO_AP, &BSS_EA(ccxsup_bss_priv));
			ccxsup_bss_priv->leap->status = CCX_IDLE;
			ccxsup_bss_priv->leap->auth_pending = FALSE;
			break;
		}

		bcm_ccx_session_key(ccxsup_bss_priv->leap->pwhashhash, CCX_PW_HASH_LEN +
		                    LEAP_CHALLENGE_LEN + LEAP_RESPONSE_LEN +
		                    LEAP_CHALLENGE_LEN + LEAP_RESPONSE_LEN,
		                    ccxsup_bss_priv->leap->session_key);
		if (ccxsup_bss_priv->leap->status < CCX_AUTHENTICATED)
			ccxsup_bss_priv->leap->status = CCX_AUTHENTICATED;
#ifdef	BCMSUP_PSK
		if (BSS_SUP_TYPE(ccxsup_bss_priv->wlc->idsup, ccxsup_bss_priv->cfg)
			== SUP_LEAP_WPA) {
			wsec_pmk_t pmk;

			pmk.key_len = CCX_SESSION_KEY_LEN;
			pmk.flags = 0;
			bcopy(ccxsup_bss_priv->leap->session_key, pmk.key, CCX_SESSION_KEY_LEN);
			wlc_ioctl(ccxsup_bss_priv->wlc, WLC_SET_WSEC_PMK, &pmk, sizeof(wsec_pmk_t),
			          ccxsup_bss_priv->cfg->wlcif);
		}
#endif /* BCMSUP_PSK */
		break;

	case EAP_SUCCESS:
		WL_WSEC(("wl%d: %s: received EAP success\n", UNIT(ccxsup_bss_priv), __FUNCTION__));

		/* Check how much is there, whether the id is the expected
		 * one, and if we're in the right state
		 */
		if ((ntoh16(rx_eap_hdr->length) < EAP_HEADER_LEN) ||
		    /* WAR: ignore ID check since some AS sends EAP_SUCCESS
		     * with a different ID
		     */
		    /* (rx_eap_hdr->id != sup->leap->id) || */
		    /* check the current state to make sure we won't respond
		     * to unexpected EAP_SUCCESS
		     */
		    (ccxsup_bss_priv->leap->status == CCX_AUTHENTICATED) ||
		    (ccxsup_bss_priv->leap->status != CCX_CHALLENGED)) {
			WL_ERROR(("wl%d: %s: unexpected EAP success",
			          UNIT(ccxsup_bss_priv), __FUNCTION__));
			break;
		}

		/* Plausible success means it's our turn to send the
		 * client challenge.
		 */
		(void) wl_del_timer(ccxsup_bss_priv->wl, ccxsup_bss_priv->leap->timer);
		ccxsup_bss_priv->leap->id = rx_eap_hdr->id;
		ccxsup_bss_priv->leap->status = CCX_CHALLENGING;
		ccxsup_bss_priv->leap->challenge_count = 0;
		ccxsup_bss_priv->leap->retry_count = 0;
		ccxsup_bss_priv->leap->start_count = 0;
		wlc_leapsup_sendeap(ccxsup_bss_priv);
		break;

	case EAP_FAILURE:
		WL_WSEC(("wl%d: %s: received EAP failure\n", UNIT(ccxsup_bss_priv), __FUNCTION__));

		(void) wl_del_timer(ccxsup_bss_priv->wl, ccxsup_bss_priv->leap->timer);
		if (ccxsup_bss_priv->leap->status == CCX_IDLE)
			break;			/* We know already */
		WL_WSEC(("wl%d: %s: EAP failure\n", UNIT(ccxsup_bss_priv), __FUNCTION__));
		if (ccxsup_bss_priv->leap->status <  CCX_CHALLENGING) {
			/* This is *supposed* to be the clue for for a rogue
			 * report.  It seems not to be reliable, though.
			 */
			wlc_ccx_rogueap_update(ccxsup_info, ccxsup_bss_priv->cfg,
			(uint16)CCX_ROGUE_CHAN_FROM_AP, &BSS_EA(ccxsup_bss_priv));
			ccxsup_bss_priv->leap->auth_pending = FALSE;
			ccxsup_bss_priv->leap->status = CCX_IDLE;
		}
		break;

	default:
		WL_ERROR(("wl%d: %s: unexpected EAP code %d\n",
		          UNIT(ccxsup_bss_priv), __FUNCTION__, (int) rx_eap_hdr->code));
		break;
	}
	return;
}

/* Initiate LEAP authentication.
 * Called after successful association.
 * Return boolean indicating whether authentication is in progress.  That
 * flag is used to recognize an authentication timeout.
 */
bool
wlc_leapsup_start(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	bss_ccxsup_info_t *ccxsup_bss;
	bool last_failed = FALSE;
	uint delay_time;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	if (ccxsup_info == NULL)
		return FALSE;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	ccxsup_bss = CCXSUP_BSSCFG_CUBBY(ccxsup_info, cfg);

	if (ccxsup_bss == NULL) {
		WL_ERROR(("%s: called with NULL supplicant info addr\n", __FUNCTION__));
		return FALSE;
	}
	WL_TRACE(("wl%d: wlc_leapsup_start: EAPOL start for %s\n",
	          UNIT(ccxsup_bss_priv), bcm_ether_ntoa(&BSS_EA(ccxsup_bss_priv), eabuf)));

	/* Make a clean start */
	wl_del_timer(ccxsup_bss_priv->wl, ccxsup_bss_priv->leap->timer);
	/* Save last failure status for same ssid */
	if (ccxsup_bss_priv->leap->status != CCX_INIT &&
		ccxsup_bss_priv->leap->status < CCX_AUTHENTICATED &&
		ccxsup_bss_priv->leap->last_ssid_len == BSS_SSID_LEN(ccxsup_bss_priv) &&
		!bcmp(BSS_SSID(ccxsup_bss_priv), ccxsup_bss_priv->leap->last_ssid,
		ccxsup_bss_priv->leap->last_ssid_len))
		last_failed = TRUE;
	/* Clear the authentication state. */
	/* leap_clear_auth_state(lsc); */
	ccxsup_bss_priv->leap->status = CCX_STARTED;
	ccxsup_bss_priv->leap->challenge_count = 0;
	/* default delay time */
	delay_time = LEAP_START_DELAY;
	/* reset the counters only when trying a different BSS */
	if (bcmp(&ccxsup_bss_priv->leap->last_bssid, &BSS_EA(ccxsup_bss_priv),
		sizeof(struct ether_addr))) {
		bcopy(&BSS_EA(ccxsup_bss_priv), &ccxsup_bss_priv->leap->last_bssid,
			sizeof(struct ether_addr));
		bcopy(BSS_SSID(ccxsup_bss_priv), ccxsup_bss_priv->leap->last_ssid,
			BSS_SSID_LEN(ccxsup_bss_priv));
		ccxsup_bss_priv->leap->last_ssid_len = BSS_SSID_LEN(ccxsup_bss_priv);
		ccxsup_bss_priv->leap->start_count = 0;
		ccxsup_bss_priv->leap->retry_count = 0;
	}
	if (last_failed) {
		ccxsup_bss_priv->leap->status = CCX_HELD;
		delay_time = LEAP_HELD_DELAY;
		WL_WSEC(("wl%d: wlc_leapsup_start: hold a while after failure\n",
			UNIT(ccxsup_bss_priv)));
	}
	wl_add_timer(ccxsup_bss_priv->wl, ccxsup_bss_priv->leap->timer, delay_time, 0);
	ccxsup_bss_priv->leap->auth_pending = (ccxsup_bss_priv->leap->status > CCX_IDLE);

	return (ccxsup_bss_priv->leap->auth_pending);
}

/* Dispatch LEAP supplicant's received EAPOL packets.
 * Packets related to authentication are passed to wlc_leapsup_recveap().
 * EAPOL key messages are handled in this function.
 *
 * Return boolean indicating whether the received packet was used.
 */
bool
wlc_ccx_leapsup(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg, eapol_header_t *rx_hdr)
{
	eap_header_t *eap_hdr;
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	bss_ccxsup_info_t *ccxsup_bss;

	if (ccxsup_info == NULL)
		return FALSE;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	ccxsup_bss = CCXSUP_BSSCFG_CUBBY(ccxsup_info, cfg);

	if (ccxsup_bss  == NULL) {
		WL_ERROR(("%s: called with NULL supplicant info addr\n", __FUNCTION__));
		return FALSE;
	} else
		WL_TRACE(("wl%d: %s: started\n", UNIT(ccxsup_bss_priv), __FUNCTION__));

	if (rx_hdr == NULL) {
		WL_WSEC(("wl%d: %s: passed NULL EAPOL pkt\n",
		         UNIT(ccxsup_bss_priv), __FUNCTION__));
		return FALSE;
	}

	if (rx_hdr->type == EAP_PACKET) {
		WL_WSEC(("wl%d: wlc_ccx_leapsup: received EAP packet\n",
		         UNIT(ccxsup_bss_priv)));
		eap_hdr = (eap_header_t *) rx_hdr->body;
		/* Don't accept request if in held time */
		if (ccxsup_bss_priv->leap->status != CCX_HELD)
			wlc_leapsup_recveap(ccxsup_bss_priv, eap_hdr);
		else {
			WL_WSEC(("wl%d: wlc_ccx_leapsup: discard eap packet in held time\n",
			         UNIT(ccxsup_bss_priv)));
			goto done;
		}

	} else if (rx_hdr->type == EAPOL_KEY) {
		eapol_key_header_t *eapol_key;
		bool report_rogues = FALSE;
		wl_wsec_key_t *key;

		WL_WSEC(("wl%d: wlc_ccx_leapsup: received EAPOL_KEY packet\n",
		         UNIT(ccxsup_bss_priv)));

		/* Don't accept a key if not authenticated. */
		if ((ccxsup_bss_priv->leap->status < CCX_AUTHENTICATED) ||
		    (ntoh16(rx_hdr->length) < EAPOL_KEY_HEADER_LEN)) {
			WL_WSEC(("wl%d: wlc_ccx_leapsup: unexpected EAPOL key\n",
			         UNIT(ccxsup_bss_priv)));
			goto done;
		}
		eapol_key = (eapol_key_header_t *) rx_hdr->body;

		if (!(key = MALLOC(ccxsup_bss_priv->osh, sizeof(wl_wsec_key_t)))) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				UNIT(ccxsup_bss_priv), __FUNCTION__,
				MALLOCED(ccxsup_bss_priv->osh)));
			return FALSE;
		}

		bzero(key, sizeof(wl_wsec_key_t));
		key->len = ntoh16(eapol_key->length);
		switch (key->len) {
		case WEP1_KEY_SIZE:
			key->algo = CRYPTO_ALGO_WEP1;
			break;
		case WEP128_KEY_SIZE:
			key->algo = CRYPTO_ALGO_WEP128;
			break;
		default:
			WL_WSEC(("wl%d: wlc_ccx_leapsup: ignoring EAPOL key with unexpected length"
			         " %d\n",
			         UNIT(ccxsup_bss_priv), key->len));
			MFREE(ccxsup_bss_priv->osh, key, sizeof(wl_wsec_key_t));
			goto done;
			break;
		}
		key->index = eapol_key->index & EAPOL_KEY_INDEX_MASK;

		if (eapol_key->index & EAPOL_KEY_UNICAST) {

			/* Unicast key message doesn't have a key It only
			 * tells how much of the session key to use and
			 * which index to pick.
			 */
			bcopy(ccxsup_bss_priv->leap->session_key, key->data, key->len);
			key->flags |= WL_PRIMARY_KEY;
			/* LEAP complete */
			report_rogues = ccxsup_bss_priv->leap->auth_pending;
			ccxsup_bss_priv->leap->auth_pending = FALSE;
			ccxsup_bss_priv->leap->status = CCX_KEYED;

		} else {
			rc4_ks_t ks;

			/* It's an encrypted broadcast key */
			bcopy(eapol_key->key, key->data, key->len);
			bcopy(eapol_key->iv, ccxsup_bss_priv->leap->key_iv, EAPOL_KEY_IV_LEN);
			prepare_key(ccxsup_bss_priv->leap->key_iv,
			            EAPOL_KEY_IV_LEN+CCX_SESSION_KEY_LEN, &ks);
			rc4(key->data, key->len, &ks);
		}
		wlc_ioctl(ccxsup_bss_priv->wlc, WLC_SET_KEY, key, sizeof(wl_wsec_key_t),
			ccxsup_bss_priv->cfg->wlcif);
		MFREE(ccxsup_bss_priv->osh, key, sizeof(wl_wsec_key_t));

		/* Authorize scb for data */
#ifdef BCMINTSUP
		wlc_ioctl(ccxsup_bss_priv->wlc, WLC_SCB_AUTHORIZE, &BSS_EA(ccxsup_bss_priv),
		          ETHER_ADDR_LEN, ccxsup_bss_priv->cfg->wlcif);
#else
		AUTHORIZE(ccxsup_bss_priv->cfg);
#endif // endif

		/* leap is done, unicast key is in place, report Rogue AP list */
		if (report_rogues) {
			wlc_leap_ddp_rogue_rpt(ccxsup_bss_priv);
			wlc_leap_rogue_clear(ccxsup_bss_priv);
		}
	}
done:
	WL_TRACE(("wl%d: wlc_ccx_leapsup: done\n", UNIT(ccxsup_bss_priv)));
	return TRUE;
}

void
wlc_ccx_rogue_timer(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg, struct ether_addr *ap_mac)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	bss_ccxsup_info_t *ccxsup_bss;

	if (ccxsup_info == NULL)
		return;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	ccxsup_bss = CCXSUP_BSSCFG_CUBBY(ccxsup_info, cfg);

	if ((ccxsup_bss == NULL) ||
		(BSS_SUP_TYPE(ccxsup_bss_priv->wlc->idsup, cfg) != SUP_LEAP) ||
	    (ccxsup_bss_priv->leap->status >= CCX_AUTHENTICATED))
		return;

	wlc_ccx_rogueap_update(ccxsup_info, cfg, (uint16)CCX_ROGUE_LEAP_TIMEOUT, ap_mac);
	ccxsup_bss_priv->leap->auth_pending = FALSE;
}

void
wlc_ccx_rogueap_update(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg, uint16 reason,
	struct ether_addr *ap_mac)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	bss_ccxsup_info_t *ccxsup_bss;
	uint indx;
	wlc_ccx_rogue_t *rogue = NULL;
#ifdef BCMDBG
	char eabuf[ETHER_ADDR_STR_LEN];
#endif /* BCMDBG */

	if (ccxsup_info == NULL)
		return;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	ccxsup_bss = CCXSUP_BSSCFG_CUBBY(ccxsup_info, cfg);

	if (ccxsup_bss == NULL)
		return;

	WL_TRACE(("wl%d: wlc_ccx_rogueap_update: add rogue AP %s, reason %d\n",
		UNIT(ccxsup_bss_priv), bcm_ether_ntoa(ap_mac, eabuf), reason));

	/* if found, return; if not found, add to the list */
	for (indx = 0; indx < LEAP_ROGUE_NUM; indx++) {
		rogue = &ccxsup_bss_priv->leap->rogue_ap[indx];
		if (!rogue->valid)
			break;

		if (!bcmp(&ap_mac->octet, &rogue->ap_mac, ETHER_ADDR_LEN))
			return;
	}

	if (indx >= LEAP_ROGUE_NUM)	/* out of space */
		return;

	rogue->valid = TRUE;
	bcopy(&ap_mac->octet, &rogue->ap_mac, ETHER_ADDR_LEN);
	rogue->reason = reason;

	/* AP name is undocumented in Cisco IE, set to NULL for now */
	rogue->ap_name_len = 0;
	rogue->ap_name[0] = '\0';
}

/* Do the work for the WLC_SET_LEAP_LIST ioctl */
static int
wlc_ccx_set_leap_list(bss_ccxsup_priv_t *ccxsup_bss_priv, bss_ccxsup_info_t *ccxsup_bss,
	wlc_bsscfg_t *cfg, void *pl)
{
	wl_leap_list_t *ll = (wl_leap_list_t *)pl;
	int n;

	/* Must have a leapsup context and a parameter ptr. */
	if ((ccxsup_bss == NULL) || (ll == NULL))
		return BCME_BADARG;

	/* Check that buflen is sufficient for list count. */
	n = (ll->count > 0) ? ll->count - 1 : 0;
	if (ll->buflen < sizeof(wl_leap_list_t) - sizeof(wl_leap_info_t) +
	    (n * sizeof(wl_leap_info_t))) {
		WL_ERROR(("wl%d: %s: leap info buflen inconsistent\n",
		          UNIT(ccxsup_bss_priv), __FUNCTION__));
		return BCME_BUFTOOSHORT;
	}

	/* If there's one already, toss it. */
	if (ccxsup_bss_priv->leap_list != NULL) {
		MFREE(ccxsup_bss_priv->osh, ccxsup_bss_priv->leap_list,
			ccxsup_bss_priv->leap_list->buflen);
	}

	ccxsup_bss_priv->leap_list = (wl_leap_list_t *) MALLOC(ccxsup_bss_priv->osh, ll->buflen);
	if (ccxsup_bss_priv->leap_list == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          UNIT(ccxsup_bss_priv), __FUNCTION__, MALLOCED(ccxsup_bss_priv->osh)));
		return BCME_NOMEM;
	}
	bcopy(ll, ccxsup_bss_priv->leap_list, ll->buflen);

	/* Check if we are waiting for user identity information */
	if (ccxsup_bss_priv->leap->status == CCX_IDREQD) {
		if (wlc_ccx_leap_ssid(ccxsup_bss_priv->m_handle, ccxsup_bss_priv->cfg,
			ccxsup_bss_priv->leap_info->ssid.SSID,
			ccxsup_bss_priv->leap_info->ssid.SSID_len)) {
			if (ccxsup_bss_priv->leap_info->user_len) {
				ccxsup_bss_priv->leap->status = CCX_IDED;
				wlc_leapsup_sendeap(ccxsup_bss_priv);
			}
		}
	}

	return 0;
}

/* Do the work for the WLC_GET_LEAP_LIST ioctl */
static int
wlc_ccx_get_leap_list(bss_ccxsup_priv_t *ccxsup_bss_priv, bss_ccxsup_info_t *ccxsup_bss,
	wlc_bsscfg_t *cfg, void *pl)
{
	wl_leap_list_t *ll = (wl_leap_list_t *)pl;

	/* Must have a leapsup context with a leap list. */
	if ((ccxsup_bss == NULL) || (ccxsup_bss_priv->leap_list == NULL) ||
	    (ll->buflen < ccxsup_bss_priv->leap_list->buflen))
		return BCME_BADARG;

	/* special case zero-count list */
	ll->count = ccxsup_bss_priv->leap_list->count;
	if (ccxsup_bss_priv->leap_list->count == 0)
		return 0;

	/* Caller must provide enough buffer for the list */
	if (ll->buflen <  sizeof(wl_leap_list_t) +
	    ((ccxsup_bss_priv->leap_list->count - 1) * sizeof(wl_leap_info_t)))
		return BCME_BUFTOOSHORT;

	ll->buflen = ccxsup_bss_priv->leap_list->buflen;
	ll->version = ccxsup_bss_priv->leap_list->version;
	bcopy(&ccxsup_bss_priv->leap_list->leap_info, &(ll->leap_info),
	      ccxsup_bss_priv->leap_list->count * sizeof(wl_leap_info_t));
	return 0;
}

void
wlc_ccx_sup_init(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg, int sup_type)
{

	/*
	 * clear the last bssid so that the related counters can be reset
	 * at the next assoc/reassoc when wlc_set_sup/wlc_leapsup_start
	 * function is called.
	 */
	bss_ccxsup_priv_t *ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	bss_ccxsup_info_t *ccxsup_bss = CCXSUP_BSSCFG_CUBBY(ccxsup_info, cfg);

	if (!ccxsup_bss)
		return;

#ifdef BCMSUP_PSK
	ASSERT(sup_type == SUP_LEAP || sup_type == SUP_LEAP_WPA);
#else
	ASSERT(sup_type == SUP_LEAP);
#endif /* BCMSUP_PSK */
	bzero(&ccxsup_bss_priv->leap->last_bssid, sizeof(struct ether_addr));
	/* delete leap timer */
	wl_del_timer(ccxsup_bss_priv->wl, ccxsup_bss_priv->leap->timer);
	/* reset leap status */
	ccxsup_bss_priv->leap->status = CCX_INIT;
}

#if defined(BCMSUP_PSK)
static void
wlc_cckm_calc_krk_btk(bss_ccxsup_priv_t *ccxsup_bss_priv)
{
	uchar *data, *prf_buff;
	const char prefix[] = "Fast-Roam Generate Base Key";
	int data_len = 0;

	if (!(data = MALLOC(ccxsup_bss_priv->osh, WPA_KEY_DATA_LEN_128))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(ccxsup_bss_priv), __FUNCTION__,  MALLOCED(ccxsup_bss_priv->osh)));
		return;
	}
	if (!(prf_buff = MALLOC(ccxsup_bss_priv->osh, PRF_OUTBUF_LEN))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(ccxsup_bss_priv), __FUNCTION__,  MALLOCED(ccxsup_bss_priv->osh)));
		MFREE(ccxsup_bss_priv->osh, data, WPA_KEY_DATA_LEN_128);
		return;
	}

	/* create the data portion (BSSID | STA-ID | SNonce | ANonce) */
	bcopy(&BSS_EA(ccxsup_bss_priv), &data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	bcopy(&CUR_EA(ccxsup_bss_priv), &data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	bcopy(&ccxsup_bss_priv->wpa->snonce, &data[data_len], EAPOL_WPA_KEY_NONCE_LEN);
	data_len += EAPOL_WPA_KEY_NONCE_LEN;
	bcopy(&ccxsup_bss_priv->wpa->anonce, &data[data_len], EAPOL_WPA_KEY_NONCE_LEN);
	data_len += EAPOL_WPA_KEY_NONCE_LEN;
#if defined(BCMDBG) || defined(WLMSG_WSEC)
	if (WL_WSEC_ON()) {
		prhex("KRK/BTK gen input", data, data_len);
	}
#endif /* BCMDBG || WLMSG_WSEC */

	/* generate the KRK/BTK */
	ASSERT(strlen(prefix) + data_len + 1 <= PRF_MAX_I_D_LEN);
	fPRF(ccxsup_bss_priv->wpa_info->pmk, ccxsup_bss_priv->wpa_info->pmk_len,
		(const uchar *)prefix, strlen(prefix),
		data, data_len, prf_buff, CCKM_KRK_LEN + CCKM_BTK_LEN);
	bcopy(prf_buff, ccxsup_bss_priv->key_refresh_key, CCKM_KRK_LEN);
	bcopy(prf_buff + CCKM_KRK_LEN, ccxsup_bss_priv->base_transient_key, CCKM_BTK_LEN);
#if defined(BCMDBG) || defined(WLMSG_WSEC)
	if (WL_WSEC_ON()) {
		prhex("KRK looks like", ccxsup_bss_priv->key_refresh_key, CCKM_KRK_LEN);
		prhex("BTK looks like", ccxsup_bss_priv->base_transient_key, CCKM_BTK_LEN);
	}
#endif /* BCMDBG || WLMSG_WSEC */

#if defined(BCMEXTCCX)
	/* driver needs this for cckm reassoc */
	PUSH_KRK_TO_WLDRIVER(ccxsup_bss_priv->cfg, ccxsup_bss_priv->key_refresh_key, CCKM_KRK_LEN);
#endif // endif
	MFREE(ccxsup_bss_priv->osh, data, WPA_KEY_DATA_LEN_128);
	MFREE(ccxsup_bss_priv->osh, prf_buff, PRF_OUTBUF_LEN);
}

static void
wlc_cckm_calc_ptk(bss_ccxsup_priv_t *ccxsup_bss_priv)
{
	uchar *data, *prf_buff;
	int data_len = 0;
	uint32 RN = htol32(ccxsup_bss_priv->rn);

	if (!(data = MALLOC(ccxsup_bss_priv->osh, WPA_KEY_DATA_LEN_128))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(ccxsup_bss_priv), __FUNCTION__,  MALLOCED(ccxsup_bss_priv->osh)));
		return;
	}
	if (!(prf_buff = MALLOC(ccxsup_bss_priv->osh, PRF_OUTBUF_LEN))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			UNIT(ccxsup_bss_priv), __FUNCTION__,  MALLOCED(ccxsup_bss_priv->osh)));
		MFREE(ccxsup_bss_priv->osh, data, WPA_KEY_DATA_LEN_128);
		return;
	}

	/* create the data portion (RN | BSSID) */
	bcopy(&RN, &data[data_len], sizeof(ccxsup_bss_priv->rn));
	data_len += sizeof(ccxsup_bss_priv->rn);
	bcopy(&BSS_EA(ccxsup_bss_priv), &data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
#if defined(BCMDBG) || defined(WLMSG_WSEC)
	if (WL_WSEC_ON()) {
		prhex("PTK gen input", data, data_len);
	}
#endif /* BCMDBG || WLMSG_WSEC */
	/* generate the PTK */
	ASSERT(data_len + 1 <= PRF_MAX_I_D_LEN);
	fPRF(ccxsup_bss_priv->base_transient_key, CCKM_BTK_LEN, NULL, 0, data, data_len,
		prf_buff, ccxsup_bss_priv->wpa->ptk_len);
	bcopy(prf_buff, ccxsup_bss_priv->wpa->eapol_mic_key, ccxsup_bss_priv->wpa->ptk_len);
#if defined(BCMDBG) || defined(WLMSG_WSEC)
	if (WL_WSEC_ON()) {
		prhex("PTK looks like", ccxsup_bss_priv->wpa->eapol_mic_key,
			ccxsup_bss_priv->wpa->ptk_len);
	}
#endif /* BCMDBG || WLMSG_WSEC */
	MFREE(ccxsup_bss_priv->osh, data, WPA_KEY_DATA_LEN_128);
	MFREE(ccxsup_bss_priv->osh, prf_buff, PRF_OUTBUF_LEN);
}
#endif /* BCMSUP_PSK && (BCMCCX || BCMEXTCCX) */

#if defined(BCMINTSUP)
void
wlc_cckm_gen_reassocreq_IE(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg,
	cckm_reassoc_req_ie_t *cckmie, uint32 tsf_h,
	uint32 tsf_l, struct ether_addr *bssid, wpa_ie_fixed_t *rsnie)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	bss_ccxsup_info_t *ccxsup_bss;
	uint32 rn;
	uint32 timestamp[2];

	if (ccxsup_info == NULL)
		return;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	ccxsup_bss = CCXSUP_BSSCFG_CUBBY(ccxsup_info, cfg);

	if (ccxsup_bss == NULL)
		return;
	timestamp[0] = htol32(tsf_l);
	timestamp[1] = htol32(tsf_h);

	/* load timestamp from bcn_prb (< 1s) */
	bcopy(timestamp, cckmie->timestamp, DOT11_MNG_TIMESTAMP_LEN);

	/* increment and load RN */
	rn = ++ccxsup_bss_priv->rn;
	rn = htol32(rn);
	bcopy(&rn, &cckmie->rn, sizeof(rn));

	/* calculate and load MIC */
	wlc_cckm_calc_reassocreq_MIC(cckmie, bssid, rsnie,
		&ccxsup_bss_priv->cfg->cur_etheraddr, ccxsup_bss_priv->rn,
		ccxsup_bss_priv->key_refresh_key, ccxsup_bss_priv->wpa->WPA_auth);
}
#endif /* BCMINTSUP && BCMCCX */

bool
wlc_sup_getleapauthpend(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	bool auth_pending = FALSE;

	if (ccxsup_info) {
		ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);

		/* check NULL pointer which may happen to open security AP connection */
		if (ccxsup_bss_priv && ccxsup_bss_priv->leap)
			auth_pending = ccxsup_bss_priv->leap->auth_pending;
	}
	return auth_pending;
}

/* Get the CCKM reassoc response IE from an association reponse */
static bool
wlc_cckm_reassoc(bss_ccxsup_priv_t *ccxsup_bss_priv, cckm_reassoc_resp_ie_t *cckm_ie)
{
	uint32 rn;
	uint len;

	len = (int)cckm_ie->len;
#if defined(BCMDBG) || defined(WLMSG_WSEC)
	if (WL_WSEC_ON()) {
		prhex("CCKM IE?", (uchar *)cckm_ie, cckm_ie->len + TLV_HDR_LEN);
	}
#endif /* BCMDBG || WLMSG_WSEC */
	if (bcmp(cckm_ie->oui, CISCO_AIRONET_OUI, DOT11_OUI_LEN) ||
	    (cckm_ie->oui_type != (uint8)CCKM_OUI_TYPE)) {
		WL_WSEC(("wl%d: wlc_cckm_reassoc: OUI (%02x:%02x:%02x:%02x) mismatch in CCKM"
		         " reassoc IE\n",
		         UNIT(ccxsup_bss_priv), cckm_ie->oui[0], cckm_ie->oui[1],
		         cckm_ie->oui[2], cckm_ie->oui_type));
		return FALSE;
	}

	len = ltoh16_ua(&cckm_ie->gtklen) + sizeof(cckm_reassoc_resp_ie_t) - 1;
	if (len != (uint)(cckm_ie->len + TLV_HDR_LEN)) {
		WL_WSEC(("wl%d: wlc_cckm_reassoc: CCKM reassoc IE length = %d, should be %d\n",
		         UNIT(ccxsup_bss_priv), cckm_ie->len, len - TLV_HDR_LEN));
		return FALSE;
	}

	rn = ltoh32_ua(&cckm_ie->rn);
	if (rn != ccxsup_bss_priv->rn) {
		WL_WSEC(("wl%d: wlc_cckm_reassoc: CCKM reassociation request number = %d, should be"
		         " %d\n",
		         UNIT(ccxsup_bss_priv), rn, ccxsup_bss_priv->rn));
		return FALSE;
	}

	return TRUE;
}

static void
wlc_cckm_calc_reassocresp_MIC(bss_ccxsup_priv_t *ccxsup_bss_priv,
	cckm_reassoc_resp_ie_t *cckmie, uint8 *obuf)
{
	uchar data[128], hash_buf[20];
	int data_len = 0;

	/* create the data portion */
	bcopy(&CUR_EA(ccxsup_bss_priv), &data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	bcopy(ccxsup_bss_priv->wpa->auth_wpaie, &data[data_len],
		ccxsup_bss_priv->wpa->auth_wpaie_len);
	data_len += ccxsup_bss_priv->wpa->auth_wpaie_len;
	bcopy(&cckmie->rn, &data[data_len], sizeof(cckmie->rn));
	data_len += sizeof(cckmie->rn);
	data[data_len++] = cckmie->ucast_idx;
	data[data_len++] = cckmie->mcast_idx;
	bcopy(cckmie->rsc, &data[data_len], CCKM_RSC_LEN);
	data_len += CCKM_RSC_LEN;
	bcopy(&cckmie->gtklen, &data[data_len], sizeof(cckmie->gtklen));
	data_len += sizeof(cckmie->gtklen);
	bcopy(cckmie->egtk, &data[data_len], cckmie->gtklen);
	data_len += cckmie->gtklen;

	/* generate the MIC */
	if (ccxsup_bss_priv->wpa->WPA_auth == WPA2_AUTH_CCKM)
		hmac_sha1(data, data_len, ccxsup_bss_priv->wpa->eapol_mic_key,
			WPA_MIC_KEY_LEN, hash_buf);
	else
		hmac_md5(data, data_len, ccxsup_bss_priv->wpa->eapol_mic_key,
			WPA_MIC_KEY_LEN, hash_buf);
	bcopy(hash_buf, obuf, CCKM_MIC_LEN);
}

static bool
wlc_cckm_decr_gtk(bss_ccxsup_priv_t *ccxsup_bss_priv, uint8 *egtk, uint16 egtk_len)
{
	unsigned char data[256] = {0}, encrkey[32];
	unsigned char egtkbuf[64];
	rc4_ks_t rc4key;
	uint16 len = ltoh16(egtk_len);
	uint encrkeylen = 0;

	WL_WSEC(("CCKM: decrypting gtk: egtk_len = %d\n", len));

	if (ccxsup_bss_priv->wpa->WPA_auth == WPA2_AUTH_CCKM) {
		if (aes_unwrap(WPA_ENCR_KEY_LEN, ccxsup_bss_priv->wpa->eapol_encr_key,
			(size_t)len, egtk, egtkbuf)) {
			WL_ERROR(("CCKM: decrypt GTK failure\n"));
			return FALSE;
		}
	} else {
		/* decrypt the gtk using RC4 */
		bcopy(&ccxsup_bss_priv->rn, encrkey, sizeof(ccxsup_bss_priv->rn));
		encrkeylen += sizeof(ccxsup_bss_priv->rn);
		bcopy(ccxsup_bss_priv->wpa->eapol_encr_key, &encrkey[encrkeylen], WPA_ENCR_KEY_LEN);
		encrkeylen += WPA_ENCR_KEY_LEN;

		prepare_key(encrkey, encrkeylen, &rc4key);
		rc4(data, 256, &rc4key); /* dump 256 bytes */
		bcopy(egtk, egtkbuf, len);
		rc4(egtkbuf, len, &rc4key);
	}
	bcopy(egtkbuf, ccxsup_bss_priv->wpa->gtk, ccxsup_bss_priv->wpa->gtk_len);
	return TRUE;
}

bool
wlc_cckm_reassoc_resp(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	bss_ccxsup_info_t *ccxsup_bss;
	wlc_info_t *wlc;
	wlc_assoc_t *as = cfg->assoc;
	uint8 *IEs;
	uint IEs_len;
	cckm_reassoc_resp_ie_t *cckmie;
	uint8 MIC[CCKM_MIC_LEN];

	if (ccxsup_info == NULL)
		return FALSE;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	ccxsup_bss = CCXSUP_BSSCFG_CUBBY(ccxsup_info, cfg);

	if (ccxsup_bss == NULL)
		return FALSE;
	wlc = ccxsup_bss_priv->wlc;

	/* check for CCKM fast roam success */
	IEs = (uint8 *)&as->resp[1];
	IEs_len = as->resp_len - sizeof(struct dot11_assoc_resp);
	if (!(cckmie = (cckm_reassoc_resp_ie_t *)
	      bcm_parse_tlvs(IEs, IEs_len, DOT11_MNG_CCKM_REASSOC_ID))) {
		WL_WSEC(("wl%d: wlc_cckm_reassoc_resp: no CCKM reassoc IE.  Association"
			" response IEs length %d bytes\n", UNIT(ccxsup_bss_priv), IEs_len));
		return FALSE;
	}

	/* validate the CCKM IE */
	if (!wlc_cckm_reassoc(ccxsup_bss_priv, cckmie))
		return FALSE;

	/* process the CCKM IE */

	/* derive the PTK */
	wlc_cckm_calc_ptk(ccxsup_bss_priv);

	/* validate cckmie->MIC */
	wlc_cckm_calc_reassocresp_MIC(ccxsup_bss_priv, cckmie, MIC);
	if (bcmp(MIC, cckmie->mic, CCKM_MIC_LEN)) {
		WL_ERROR(("CCKM: reassoc success, but MIC failure!  What now?\n"));
		return FALSE;
	}

	/* plumb the pairwise key */
	wlc_wpa_plumb_tk(wlc, cfg, (uint8*)ccxsup_bss_priv->wpa->temp_encr_key,
		ccxsup_bss_priv->wpa->tk_len,
		ccxsup_bss_priv->wpa->ucipher, &cfg->current_bss->BSSID);

	/* decrypt and plumb the group key */
	if (!wlc_cckm_decr_gtk(ccxsup_bss_priv, cckmie->egtk, cckmie->gtklen))
		return FALSE;
	/* plumb GTK */
	wlc_wpa_plumb_gtk(wlc, cfg, ccxsup_bss_priv->wpa->gtk, ccxsup_bss_priv->wpa->gtk_len,
		cckmie->mcast_idx, ccxsup_bss_priv->wpa->mcipher, cckmie->rsc, FALSE);

	/*
	 * partially reset supplicant state:
	 * - reset WPA replay counter
	 * - XXX - anything else?
	 */
	bzero(ccxsup_bss_priv->wpa->replay, EAPOL_KEY_REPLAY_LEN);

	return TRUE;
}
#ifdef BCMEXTCCX
void
wlc_cckm_set_assoc_resp(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg, uint8 *src, int len)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	struct dot11_assoc_resp *dst = ccxsup_bss_priv->wlc->assoc_resp;

	if (ccxsup_info == NULL)
		return FALSE;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);

	if (len < sizeof(struct dot11_assoc_resp) || src == NULL) {
		DEBUG_PRINTF(("%s: bad args:  assoc_resp %p len %d\n",
			__FUNCTION__, src, len));
		return;
	}
	if (dst != NULL)
		MFREE(ccxsup_bss_priv->osh, dst, ccxsup_bss_priv->wlc->assoc_resp_len);

	dst = MALLOC(ccxsup_bss_priv->osh, len);
	if (dst == NULL) {
		return;
	}
	bcopy(src, dst, len);
	ccxsup_bss_priv->wlc->assoc_resp_len = len;
	ccxsup_bss_priv->wlc->assoc_resp = dst;
}

void
wlc_cckm_set_rn(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg, int rn)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	bss_ccxsup_info_t *ccxsup_bss;

	if (ccxsup_info == NULL)
		return FALSE;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	ccxsup_bss = CCXSUP_BSSCFG_CUBBY(ccxsup_info, cfg);

	if (!ccxsup_bss)
		return;
	ccxsup_bss_priv->rn = rn;
}
#endif /* BCMEXTCCX */

sup_auth_status_t
wlc_ccxsup_get_auth_status(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;

	if (ccxsup_info == NULL)
		return FALSE;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	/*
	* 1. timeout when we have tried EAPOL Start N times without going any further
	* 2. timeout when we have tried to authenticate N times without getting
	*	 EAP success or EAP failure
	*/
	if ((ccxsup_bss_priv->leap->start_count > LEAP_MAX_RETRY &&
		ccxsup_bss_priv->leap->status == CCX_AUTHENTICATED) ||
		(ccxsup_bss_priv->leap->retry_count > LEAP_MAX_RETRY &&
		ccxsup_bss_priv->leap->status < CCX_AUTHENTICATED &&
		ccxsup_bss_priv->leap->status > CCX_IDLE)) {
		WL_WSEC(("wl%d: TIMEOUT, start %d retry %d status %d\n",
			UNIT(ccxsup_bss_priv),
			ccxsup_bss_priv->leap->start_count, ccxsup_bss_priv->leap->retry_count,
			ccxsup_bss_priv->leap->status));
		return WLC_SUP_TIMEOUT;
	}

	switch (ccxsup_bss_priv->leap->status) {
		case CCX_IDLE:
		case CCX_HELD:
			return WLC_SUP_DISCONNECTED;
		case CCX_STARTED:
			return WLC_SUP_CONNECTING;
		case CCX_IDREQD:
			return WLC_SUP_IDREQUIRED;
		case CCX_IDED:
		case CCX_CHALLENGED:
		case CCX_CHALLENGING:
		case CCX_NOTIFIED:
			return WLC_SUP_AUTHENTICATING;
		case CCX_AUTHENTICATED:
			return WLC_SUP_AUTHENTICATED;
		case CCX_KEYED:
			return WLC_SUP_KEYED;
		default:
			return WLC_SUP_DISCONNECTED;
	}
}

uint16
wlc_ccxsup_get_cipher(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg, wpapsk_t *wpa,
	uint16 key_info, uint16 key_len)
{
	uint16 cipher = CRYPTO_ALGO_OFF;

	if (ccxsup_info == NULL)
		return CRYPTO_ALGO_OFF;

	switch (key_len) {
		case AES_KEY_SIZE:
			/* CCKM: ucast CKIP/MMH and CKIP come in as AES */
			if (wpa->WPA_auth == WPA_AUTH_CCKM && (key_info & WPA_KEY_PAIRWISE)) {
				switch (wpa->ucipher) {
					case CRYPTO_ALGO_CKIP_MMH:
					case CRYPTO_ALGO_CKIP:
						cipher = wpa->ucipher;
						break;
					default:
						break;
				}
			}
			break;
		case WEP128_KEY_SIZE:
			if (!(key_info & WPA_KEY_PAIRWISE)) {
				/* CCKM: mcast CKIP/MMH, CKIP, and WEP/MMH come in as WEP */
				if (wpa->WPA_auth == WPA_AUTH_CCKM) {
					switch (wpa->mcipher) {
						case CRYPTO_ALGO_CKIP_MMH:
						case CRYPTO_ALGO_CKIP:
						case CRYPTO_ALGO_WEP_MMH:
							cipher = wpa->mcipher;
							break;
						default:
							break;
					}
				}
			} else if (wpa->WPA_auth == WPA_AUTH_CCKM) {
				/* CCKM: ucast WEP is legal */
				/* CCKM: ucast WEP/MMH comes in as WEP */
				if (wpa->ucipher == CRYPTO_ALGO_WEP_MMH)
					cipher = wpa->ucipher;
				else
					/* This fails downstream if not WEP128... */
					cipher = CRYPTO_ALGO_WEP128;
			}
			break;
		case WEP1_KEY_SIZE:
			if (!(key_info & WPA_KEY_PAIRWISE)) {
				/* CCKM: mcast CKIP/MMH, CKIP, and WEP/MMH come in as WEP */
				if (wpa->WPA_auth == WPA_AUTH_CCKM) {
					switch (wpa->mcipher) {
					case CRYPTO_ALGO_CKIP_MMH:
					case CRYPTO_ALGO_CKIP:
					case CRYPTO_ALGO_WEP_MMH:
						cipher = wpa->mcipher;
						break;
					default:
						break;
					}
				}
			} else if (wpa->WPA_auth == WPA_AUTH_CCKM) {
				/* CCKM: ucast WEP is legal */
				/* CCKM: ucast WEP/MMH comes in as WEP */
				if (wpa->ucipher == CRYPTO_ALGO_WEP_MMH)
					cipher = wpa->ucipher;
				else
					/* This fails downstream if not WEP1... */
					cipher = CRYPTO_ALGO_WEP1;
			}
			break;
		default:
			break;
		}
	return cipher;
}

uint16
wlc_ccxsup_handle_joinstart(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg, uint16 sup_type)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	wlc_info_t *wlc;

	if (ccxsup_info == NULL)
		return sup_type;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	wlc = ccxsup_bss_priv->wlc;

	if ((wlc->ccx->leap_on =
		wlc_ccx_leap_ssid(ccxsup_info, cfg, cfg->SSID, cfg->SSID_len))) {
		if (wlc_assoc_iswpaenab(cfg, TRUE))
			sup_type = SUP_LEAP_WPA;
		else
			sup_type = SUP_LEAP;
		wlc_ccx_sup_init(ccxsup_info, cfg, sup_type);
	}
	return sup_type;
}

void
wlc_ccxsup_handle_wpa_eapol_msg1(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg, uint16 key_info)
{
	char mic_key_buf[16];
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	wpapsk_t *wpa;

	if (ccxsup_info == NULL)
		return;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	wpa = ccxsup_bss_priv->wpa;

	/* compute KRK and BTK from NSK */
	wlc_cckm_calc_krk_btk(ccxsup_bss_priv);

	/* compute PTK from BTK */
	wlc_cckm_calc_ptk(ccxsup_bss_priv);

	/* overwrite 802.1x MIC key with KRK for message 2 */
	bcopy(wpa->eapol_mic_key, mic_key_buf, WPA_MIC_KEY_LEN);
	bcopy(ccxsup_bss_priv->key_refresh_key, wpa->eapol_mic_key, CCKM_KRK_LEN);

	/* send message 2 */
	if (wlc_wpa_sup_sendeapol(ccxsup_bss_priv->wlc->idsup, ccxsup_bss_priv->cfg,
		(key_info & PMSG2_MATCH_FLAGS), PMSG2)) {
		wpa->state = WPA_SUP_STAKEYSTARTP_WAIT_M3;
	} else {
		WL_WSEC(("wl%d: wlc_wpa_sup_eapol: send message 2 failed"
				 "(ccx)\n", UNIT(ccxsup_bss_priv)));
		wlc_wpa_send_sup_status(ccxsup_bss_priv->wlc->idsup, ccxsup_bss_priv->cfg,
			WLC_E_SUP_SEND_FAIL);
	}
	/* restore 802.1x MIC Key */
	bcopy(mic_key_buf, wpa->eapol_mic_key, WPA_MIC_KEY_LEN);

}

void
wlc_ccxsup_send_leap_rogue_report(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;

	if (ccxsup_info == NULL)
		return;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);

	if (ccxsup_bss_priv->leap->auth_pending) {
		ccxsup_bss_priv->leap->auth_pending = FALSE;
		wlc_leap_ddp_rogue_rpt(ccxsup_bss_priv);
		wlc_leap_rogue_clear(ccxsup_bss_priv);
	}
}

void
wlc_ccxsup_set_leap_state_keyed(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;

	if (ccxsup_info == NULL)
		return;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);

	ccxsup_bss_priv->leap->status = CCX_KEYED;
}

void
wlc_ccxsup_init_cckm_rn(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;

	if (ccxsup_info == NULL)
		return;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);

	ccxsup_bss_priv->rn = 1;
}

void
wlc_ccxsup_start_negotimer(wlc_ccxsup_info_t *ccxsup_info, wlc_bsscfg_t *cfg)
{
	bss_ccxsup_priv_t *ccxsup_bss_priv;
	wlc_info_t *wlc;

	if (ccxsup_info == NULL)
		return;

	ccxsup_bss_priv = CCXSUP_BSSCFG_CUBBY_PRIV(ccxsup_info, cfg);
	wlc = ccxsup_bss_priv->wlc;

	if (CCX_ENAB(wlc->pub) && wlc->ccx->leap_on) {
		/* start LEAP negotiation timer */
		wlc->ccx->leap_start = wlc->pub->now;
		ccxsup_bss_priv->leap->auth_pending = TRUE;
	}
}
