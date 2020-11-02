/**
 * @file
 * @brief
 * wlc_auth.c -- driver-resident authenticator.
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
 * $Id: wlc_auth.c 782389 2019-12-18 06:56:56Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#if defined(BCMAUTH_PSK)

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
#include <bcmcrypto/passhash.h>
#include <bcmcrypto/prf.h>
#include <bcmcrypto/sha1.h>
#include <proto/802.11.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_export.h>
#include <wlc_scb.h>
#include <wlc_keymgmt.h>
#include <wlc_wpa.h>
#include <wlc_sup.h>
#include <wlc_auth.h>
#include <bcm_mpool_pub.h>
#ifdef WLBTAMP
#include <proto/802.11_bta.h>
#endif /* WLBTAMP */

#ifdef MFP
#include <wlc_mfp.h>
#define AUTHMFP(a) ((a)->wlc->mfp)
#endif // endif

#define AUTH_WPA2_RETRY		3	/* number of retry attempts */
#define AUTH_WPA2_RETRY_TIMEOUT	1000	/* 1 sec retry timeout */
#define AUTH_WPA2_GTK_ROTATION_TIME 0

/* PRF() expects to write its result sloppily. */
#define PRF_RESULT_LEN	80

#define GMK_LEN			32
#define KEY_COUNTER_LEN		32

#define AUTH_FLAG_GTK_PLUMBED	0x1		/* GTK has been plumbed into h/w */
#define AUTH_FLAG_PMK_PRESENT	0x2		/* auth->psk contains pmk */
#define AUTH_FLAG_REKEY_ENAB	0x4		/* enable rekeying request */

/* GTK plumbing index values */
#define GTK_ID_1	1
#define GTK_ID_2	2

/* Toggle GTK index.  Indices 1 - 3 are usable; spec recommends 1 and 2. */
#define GTK_NEXT_ID(auth)	((auth)->gtk_id == GTK_ID_1 ? GTK_ID_2 : GTK_ID_1)

/* determine which key descriptor version to use */
#define KEY_DESC(auth, scb) (SCB_SHA256(scb) ? WPA_KEY_DESC_V3 : (\
	!WSEC_TKIP_ENABLED(scb->wsec) ? WPA_KEY_DESC_V2 :  \
		WPA_KEY_DESC_V1))
/* Authenticator top-level structure hanging off bsscfg */
struct authenticator {
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_bsscfg_t *bsscfg;		/* pointer to auth's bsscfg */
	wlc_auth_info_t *auth_info;	/* pointer to parent module */
	struct scb *scb_auth_in_prog;	/* pointer to SCB actively being authorized */

	uint16 flags;			/* operation flags */

	/* mixed-mode WPA/WPA2 is not supported */
	int auth_type;			/* authenticator discriminator */

	/* global passphrases used for cobbling pairwise keys */
	ushort psk_len;			/* len of pre-shared key */
	uchar  psk[WSEC_MAX_PSK_LEN];	/* saved pre-shared key */

	/* group key stuff */
	uint8 gtk[TKIP_KEY_SIZE];		/* group transient key */
	uint8 gtk_id;
	uint8 gtk_key_index;
	ushort gtk_len;		/* Group (mcast) key length */
	uint8 global_key_counter[KEY_COUNTER_LEN];	/* global key counter */
	uint8 initial_gkc[KEY_COUNTER_LEN];		/* initial GKC value */
	uint8 gnonce[EAPOL_WPA_KEY_NONCE_LEN];	/* AP's group key nonce */
	uint8 gmk[GMK_LEN];			/* group master key */
	uint8 gtk_rsc[8];
	struct wl_timer *gtk_timer;
	uint32 gtk_interval;
};

/* skeletion structure since authenticator_t hangs off the bsscfg, rather than wlc */
struct wlc_auth_info {
	wlc_info_t *wlc;
	int scb_handle;			/* scb cubby for per-STA data */
	bcm_mp_pool_h wpa_mpool_h;	/* Memory pool for 'wpapsk_t' */
	bcm_mp_pool_h wpa_info_mpool_h;	/* Memory pool for 'wpapsk_info_t' */
};

/* scb cubby */
typedef struct scb_auth {
	uint16 flags;			/* operation flags */
	wpapsk_t *wpa;			/* volatile, initialized in set_auth */
	wpapsk_info_t *wpa_info;		/* persistent wpa related info */
} scb_auth_t;

struct auth_cubby {
	scb_auth_t *cubby;
};

/* iovar table */
enum {
#ifdef BCMDBG
	IOV_AUTH_PTK_M1,		/* send PTK M1 pkt */
	IOV_AUTH_REKEY_INIT,	/* enable grp rekey. must be done before any connection */
	IOV_AUTH_GTK_M1,		/* send GTK M1 pkt */
	IOV_AUTH_GTK_BAD_M1,	/* send bad GTK M1 pkt */
#endif /* BCMDBG */
	_IOV_AUTHENTICATOR_DUMMY	/* avoid empty enum */
};

static const bcm_iovar_t auth_iovars[] = {
#ifdef BCMDBG
	{"auth_ptk_m1", IOV_AUTH_PTK_M1, IOVF_BSSCFG_AP_ONLY, IOVT_BUFFER, ETHER_ADDR_LEN},
	{"auth_rekey_init", IOV_AUTH_REKEY_INIT, IOVF_BSSCFG_AP_ONLY, IOVT_BOOL, 0},
	{"auth_gtk_m1", IOV_AUTH_GTK_M1, IOVF_BSSCFG_AP_ONLY, IOVT_BUFFER, ETHER_ADDR_LEN},
	{"auth_gtk_bad_m1", IOV_AUTH_GTK_BAD_M1, IOVF_BSSCFG_AP_ONLY, IOVT_BUFFER, ETHER_ADDR_LEN},
#endif /* BCMDBG */
	{NULL, 0, 0, 0, 0}
};

#define SCB_AUTH_INFO(auth, scb) ((SCB_CUBBY((scb), (auth)->scb_handle)))
#define SCB_AUTH_CUBBY(auth, scb) (((struct auth_cubby *)SCB_AUTH_INFO(auth, scb))->cubby)

#define AUTH_IN_PROGRESS(auth) (((auth)->scb_auth_in_prog != NULL))

static bool wlc_auth_wpapsk_start(authenticator_t *auth, uint8 *sup_ies, uint sup_ies_len,
                                  uint8 *auth_ies, uint auth_ies_len, struct scb *scb);
static bool wlc_wpa_auth_sendeapol(authenticator_t *auth, uint16 flags,
                                   wpa_msg_t msg, struct scb *scb);
static bool wlc_wpa_auth_recveapol(authenticator_t *auth, eapol_header_t *eapol,
                                   bool encrypted, struct scb *scb);

static void wlc_auth_gen_gtk(authenticator_t *auth, wlc_bsscfg_t *bsscfg);
static void wlc_auth_initialize_gkc(authenticator_t *auth);
static void wlc_auth_incr_gkc(authenticator_t *auth);
static void wlc_auth_initialize_gmk(authenticator_t *auth);
static int wlc_auth_set_ssid(authenticator_t *auth, uchar ssid[], int ssid_len, struct scb *scb);

/* scb stuff */
static int wlc_auth_scb_init(void *context, struct scb *scb);
static void wlc_auth_scb_deinit(void *context, struct scb *scb);
static int wlc_auth_prep_scb(authenticator_t *auth, struct scb *scb);
static void wlc_auth_cleanup_scb(wlc_info_t *wlc, struct scb *scb);

/* module stuff */
static int wlc_auth_down(void *handle);
static int wlc_auth_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
                            void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif);
#ifdef BCMDBG
static int wlc_auth_dump(authenticator_t *auth, struct bcmstrbuf *b);
#endif /* BCMDBG */

static void wlc_auth_endassoc(authenticator_t *auth);
void initialize_gmk(authenticator_t *auth);

static void wlc_auth_gtk_timer(void *arg);

/* Break a lengthy passhash algorithm into smaller pieces. It is necessary
 * for dongles with under-powered CPUs.
 */
static void
wlc_auth_wpa_passhash_timer(void *arg)
{
	authenticator_t *auth = (authenticator_t *)arg;
	wlc_info_t *wlc = auth->wlc;
	struct scb *scb = auth->scb_auth_in_prog;
	wpapsk_info_t *wpa_info;
	scb_auth_t *auth_cubby;

	if (scb == NULL) {
		WL_ERROR(("wl%d: %s: SCB is null\n", wlc->pub->unit,  __FUNCTION__));
		return;
	}
	auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
	wpa_info = auth_cubby->wpa_info;

	if (wpa_info == NULL) {
		WL_ERROR(("wl%d: %s: wpa_info is null\n", wlc->pub->unit,  __FUNCTION__));
		return;
	}

	if (do_passhash(&wpa_info->passhash_states, 256) == 0) {
		WL_WSEC(("wl%d: %s: Passhash is done\n", wlc->pub->unit, __FUNCTION__));
		get_passhash(&wpa_info->passhash_states, wpa_info->pmk, PMK_LEN);
		wpa_info->pmk_len = PMK_LEN;
			wlc_auth_join_complete(auth, &scb->ea, FALSE);
		return;
	}

	WL_TRACE(("wl%d: %s: passhash is in progress\n", wlc->pub->unit, __FUNCTION__));
	wl_add_timer(wlc->wl, wpa_info->passhash_timer, 0, 0);
}

static void
wlc_auth_retry_timer(void *arg)
{
	uint16 flags;
	struct scb *scb = (struct scb *)arg;
	wlc_bsscfg_t *bsscfg;
	authenticator_t *auth;
	scb_auth_t *auth_cubby;
	wpapsk_t *wpa;

	if (scb == NULL) {
		WL_ERROR(("%s: scb is null\n", __FUNCTION__));
		return;
	}

	if ((bsscfg = SCB_BSSCFG(scb)) == NULL) {
		WL_ERROR(("%s: bsscfg is null\n", __FUNCTION__));
		return;
	}

	if ((auth = bsscfg->authenticator) == NULL || auth->auth_info == NULL) {
		if (auth)
			WL_ERROR(("%s: auth->auth_info is NULL\n", __FUNCTION__));
		else
			WL_ERROR(("%s: authenticator is NULL \n", __FUNCTION__));
		return;
	}

	if ((auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb)) == NULL) {
		WL_ERROR(("%s: cubby is null\n", __FUNCTION__));
		return;
	}

	if ((wpa = auth_cubby->wpa) == NULL) {
		WL_ERROR(("%s: NULL wpa\n", __FUNCTION__));
		return;
	}

	WL_WSEC(("wl%d: retry timer fired in state %d\n",
		auth->wlc->pub->unit, wpa->state));

	flags = KEY_DESC(auth, scb);
	switch (wpa->state) {
	case WPA_AUTH_PTKINITNEGOTIATING:
		if (wpa->retries++ >= AUTH_WPA2_RETRY) {
			/* Send deauth when 4-way handshake time out */
			if (auth->scb_auth_in_prog)
				wlc_wpa_senddeauth(auth->bsscfg,
					(char *)&auth->scb_auth_in_prog->ea, DOT11_RC_4WH_TIMEOUT);
			wlc_auth_endassoc(auth);
			break;
		}
		/* Bump up replay counter for retried frame(as suggested by Std)
		 * XXX: This is known to cause some issues. If we decide
		 * not to bump up the replay counter, then have to make
		 * a corresponding change in the supplicant where it can
		 * accept the retried frame
		 */
		wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);
		wlc_wpa_auth_sendeapol(auth, flags, PMSG1, scb);
		break;

	case WPA_AUTH_PTKINITDONE:
		if (wpa->retries++ >= AUTH_WPA2_RETRY) {
			wlc_auth_endassoc(auth);
			break;
		}
		/* Bump up replay counter for retried frame(as suggested by Std)
		 * XXX: This is known to cause some issues. If we decide
		 * not to bump up the replay counter, then have to make
		 * a corresponding change in the supplicant where it can
		 * accept the retried frame
		 */
		wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);
		wlc_wpa_auth_sendeapol(auth, flags, PMSG3, scb);
		break;

	case WPA_AUTH_REKEYNEGOTIATING:
		if (wpa->retries++ >= AUTH_WPA2_RETRY) {
			wlc_auth_endassoc(auth);
			break;
		}
		wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);
		if (wpa->WPA_auth == WPA_AUTH_PSK)
			wlc_wpa_auth_sendeapol(auth, flags, GMSG1, scb);
		else
			wlc_wpa_auth_sendeapol(auth, flags, GMSG_REKEY, scb);
		break;

	default:
		break;
	}
}

static int
wlc_auth_encr_key_data(authenticator_t *auth, wpapsk_t *wpa,
	eapol_wpa_key_header_t *wpa_key, uint16 flags,
	uint eapol_ver)
{
	uint8 scrbuf[32];
	rc4_ks_t *rc4key;
	bool enc_status;
	int ret = 0;

	rc4key = MALLOC(auth->wlc->osh, sizeof(rc4_ks_t));

	if (!rc4key) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
			auth->wlc->pub->unit, __FUNCTION__,  MALLOCED(auth->wlc->osh)));
		ret = -1;
		goto err;
	}

	/* encrypt key data field */
	if (eapol_ver == 2) {
		WL_WSEC(("%s(): EAPOL hdr version is 2, set key iv to 0\n",
			__FUNCTION__));
		memset((uchar*)wpa_key->iv, 0, 16);
	}
	else
		bcopy((uchar*)&auth->global_key_counter[KEY_COUNTER_LEN-16],
			(uchar*)wpa_key->iv, 16);

	/* Pass single buffer for data and encryption key arguments
	 * used for discarding data
	 */
	enc_status = wpa_encr_key_data(wpa_key, flags, wpa->eapol_encr_key,
		NULL, NULL, scrbuf, rc4key);

	if (!enc_status) {
		WL_ERROR(("wl%d: %s: error encrypting key "
		          "data\n", auth->wlc->pub->unit, __FUNCTION__));
		ret = -1;
	}

err:
	if (rc4key)
		MFREE(auth->wlc->osh, rc4key, sizeof(rc4_ks_t));

	return ret;
}

static int
wlc_auth_plumb_gtk(authenticator_t *auth, uint32 cipher)
{
	auth->gtk_key_index = (uint8)wlc_wpa_plumb_gtk(auth->wlc, auth->bsscfg,
		auth->gtk, auth->gtk_len, auth->gtk_id,
		cipher, NULL, 1);
	if (auth->gtk_key_index == (uint8) (-1)) {
		WL_ERROR(("wl%d: %s: invalid gtk_key_index \n",
			auth->wlc->pub->unit,  __FUNCTION__));
		auth->gtk_key_index = 0;
		return -1;
	}

	auth->flags |= AUTH_FLAG_GTK_PLUMBED;

	/* start the update timer */
	if (auth->gtk_interval) {
		auth->flags |= AUTH_FLAG_REKEY_ENAB;
		if (!auth->gtk_timer)
			auth->gtk_timer = wl_init_timer(auth->wlc->wl, wlc_auth_gtk_timer,
				auth, "auth_gtk_rotate");
		if (!auth->gtk_timer)
			WL_ERROR(("wl%d: %s: gtk timer alloc failed\n",
				auth->wlc->pub->unit, __FUNCTION__));
		else
			wl_add_timer(auth->wlc->wl, auth->gtk_timer,
				auth->gtk_interval, 0);
	}

	return 0;
}

static void
wlc_auth_new_gtk(authenticator_t *auth)
{
	if (!(auth->flags & AUTH_FLAG_GTK_PLUMBED))
		wlc_auth_initialize_gmk(auth);

	wlc_auth_gen_gtk(auth, auth->bsscfg);
	auth->gtk_id = GTK_NEXT_ID(auth);

#ifdef MFP
	if (WLC_MFP_ENAB(auth->wlc->pub))
		wlc_mfp_gen_igtk(AUTHMFP(auth), auth->bsscfg,
			auth->gmk, GMK_LEN);
#endif // endif
	return;
}

static void
wlc_auth_gtk_timer(void *arg)
{
	authenticator_t *auth = (authenticator_t *)arg;
	struct scb *scb = NULL;
	scb_auth_t *auth_cubby;
	wpapsk_t *wpa;
	struct scb_iter scbiter;
	int num_scbs = 0;
	uint16 flags = 0;

	memset(&scbiter, 0, sizeof(scbiter));

	/* GTK rekey timer expired.  Set all SCBs associated with this BSS to
	 * WPA_REKEYNEGOTIATING state.  Generate and send new GTK to all SCBs
	 */
	if (!auth || !auth->wlc || !auth->bsscfg) {
		if (auth)
			WL_ERROR(("%s: NULL ptr auth->wlc: %p, auth->bsscfg: %p\n",
				__FUNCTION__, auth->wlc, auth->bsscfg));
		else
			WL_ERROR(("%s: NULL ptr auth\n", __FUNCTION__));
		return;
	}

	FOREACH_BSS_SCB(auth->wlc->scbstate, &scbiter, auth->bsscfg, scb) {
		if (!SCB_AUTHORIZED(scb)) { /* Skip SCBs that are not authorized */
			continue;
		}
		auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
		if (auth_cubby == NULL) {
			WL_ERROR(("%s: wpa is null is for scb %p\n", __FUNCTION__, scb));
			continue;
		}

		if ((wpa = auth_cubby->wpa) == NULL) {
			WL_ERROR(("%s: wpa is null is for scb %p\n", __FUNCTION__, scb));
			continue;
		}

		/* Cobble the GTK for first SCB found */
		if (!num_scbs)
			wlc_auth_new_gtk(auth);

		wpa->state = WPA_AUTH_REKEYNEGOTIATING;
		flags = KEY_DESC(auth, scb);
		wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);
		wlc_wpa_auth_sendeapol(auth, flags, GMSG_REKEY, scb);
		num_scbs++;
	}

	if (num_scbs) {
		/* Start timer for the next GTK rotation */
		if (auth->gtk_interval)
			wl_add_timer(auth->wlc->wl, auth->gtk_timer,
				auth->gtk_interval, 0);
	} else {
		/* No active SCBs on this BSS */
		auth->flags &= ~AUTH_FLAG_GTK_PLUMBED;
	}
}

/* return 0 when succeeded, 1 when passhash is in progress, -1 when failed */
int
wlc_auth_set_ssid(authenticator_t *auth, uchar ssid[], int len, struct scb* scb)
{
	wpapsk_info_t *wpa_info;
	scb_auth_t *auth_cubby;

	if (auth == NULL) {
		WL_WSEC(("wlc_auth_set_ssid: called with NULL auth\n"));
		return -1;
	}

	if (scb == NULL) {
		WL_ERROR(("wl%d: %s: SCB is null\n", auth->wlc->pub->unit,  __FUNCTION__));
		return -1;
	}

	auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
	wpa_info = auth_cubby->wpa_info;

	if (auth->psk_len == 0) {
		WL_WSEC(("%s: called with NULL psk\n", __FUNCTION__));
		return 0;
	} else if (wpa_info->pmk_len != 0) {
		WL_WSEC(("%s: called with non-NULL pmk\n", __FUNCTION__));
		return 0;
	}
	return wlc_wpa_cobble_pmk(wpa_info, (char *)auth->psk, auth->psk_len, ssid, len);
}

/* Allocate authenticator context, squirrel away the passed values,
 * and return the context handle.
 */
wlc_auth_info_t *
BCMATTACHFN(wlc_auth_attach)(wlc_info_t *wlc)
{
	wlc_auth_info_t *auth_info;

	WL_TRACE(("wl%d: wlc_auth_attach\n", wlc->pub->unit));

	if (!(auth_info = (wlc_auth_info_t *)MALLOCZ(wlc->osh, sizeof(wlc_auth_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	/* Create memory pool for 'wpapsk_t' data structs. */
	if (bcm_mpm_create_heap_pool(wlc->mem_pool_mgr, sizeof(wpapsk_t),
	                             "a_wpa", &auth_info->wpa_mpool_h) != BCME_OK) {
		WL_ERROR(("wl%d: bcm_mpm_create_heap_pool failed\n", wlc->pub->unit));
		goto err;
	}

	/* Create memory pool for 'wpapsk_info_t' data structs. */
	if (bcm_mpm_create_heap_pool(wlc->mem_pool_mgr, sizeof(wpapsk_info_t),
	                             "a_wpai", &auth_info->wpa_info_mpool_h) != BCME_OK) {
		WL_ERROR(("wl%d: bcm_mpm_create_heap_pool failed\n", wlc->pub->unit));
		goto err;
	}

	/* XXX Authenticator hangs off the bsscfg, rather than off wlc, cannot allocate/deallocate
	 * anything in scb init/deinit functions since the authenticator context (and scb handle)
	 * is tossed before the scbs are freed.
	 */

	auth_info->scb_handle =
	        wlc_scb_cubby_reserve(wlc, sizeof(struct auth_cubby), wlc_auth_scb_init,
	                              wlc_auth_scb_deinit, NULL, (void*) auth_info);

	if (auth_info->scb_handle < 0) {
		WL_ERROR(("wl%d: wlc_scb_cubby_reserve() failed\n", wlc->pub->unit));
		goto err;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, auth_iovars, "auth", auth_info, wlc_auth_doiovar,
		NULL, NULL, wlc_auth_down)) {
		WL_ERROR(("wl%d: auth wlc_module_register() failed\n", wlc->pub->unit));
		goto err;
	}

	auth_info->wlc = wlc;

#ifdef BCMDBG
	wlc_dump_register(wlc->pub, "auth", (dump_fn_t)wlc_auth_dump, (void *)auth_info);
#endif // endif
	return auth_info;

err:
	wlc_auth_detach(auth_info);
	return NULL;
}

authenticator_t *
wlc_authenticator_attach(wlc_info_t *wlc, wlc_bsscfg_t *cfg)
{
	authenticator_t *auth;

	WL_TRACE(("wl%d: %s\n", wlc->pub->unit, __FUNCTION__));

	if (!(auth = (authenticator_t *)MALLOCZ(wlc->osh, sizeof(authenticator_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	auth->wlc = wlc;
	auth->bsscfg = cfg;
	auth->auth_info = wlc->authi;
	auth->gtk_id = GTK_ID_1;
	auth->gtk_interval = AUTH_WPA2_GTK_ROTATION_TIME;

	return auth;
}

/* detach the authenticator_t struct from the wlc_auth_info_t struct */
/* exists to preserve scb handle, which is needed to deinit the scbs */

void
wlc_authenticator_detach(authenticator_t *auth)
{
	if (auth == NULL)
		return;

	MFREE(auth->wlc->osh, auth, sizeof(authenticator_t));
	auth = NULL;
}

/* down and clean the authenticator structure which hangs off bsscfg */
void
wlc_authenticator_down(authenticator_t *auth)
{
	if (auth == NULL)
		return;

	wlc_keymgmt_reset(auth->wlc->keymgmt, auth->bsscfg, NULL);

	if (auth->gtk_timer) {
		wl_del_timer(auth->wlc->wl, auth->gtk_timer);
		wl_free_timer(auth->wlc->wl, auth->gtk_timer);
		auth->gtk_timer = NULL;
	}

	auth->flags &= ~AUTH_FLAG_GTK_PLUMBED;
	auth->gtk_id = GTK_ID_1;
	auth->gtk_key_index = 0;
	auth->gtk_len = 0;

	memset(auth->gtk, 0, sizeof(auth->gtk));
	memset(auth->global_key_counter, 0, sizeof(auth->global_key_counter));
	memset(auth->initial_gkc, 0, sizeof(auth->initial_gkc));
	memset(auth->gtk_rsc, 0, sizeof(auth->gtk_rsc));
	memset(auth->gmk, 0, sizeof(auth->gmk));
	memset(auth->gnonce, 0, sizeof(auth->gnonce));
}

/* Toss authenticator context */
void
BCMATTACHFN(wlc_auth_detach)(wlc_auth_info_t *auth_info)
{
	if (!auth_info)
		return;

	WL_TRACE(("wl%d: wlc_auth_detach\n", auth_info->wlc->pub->unit));

	wlc_module_unregister(auth_info->wlc->pub, "auth", auth_info);

	if (auth_info->wpa_info_mpool_h != NULL) {
		bcm_mpm_delete_heap_pool(auth_info->wlc->mem_pool_mgr,
		                         &auth_info->wpa_info_mpool_h);
	}

	if (auth_info->wpa_mpool_h != NULL) {
		bcm_mpm_delete_heap_pool(auth_info->wlc->mem_pool_mgr, &auth_info->wpa_mpool_h);
	}

	MFREE(auth_info->wlc->osh, auth_info, sizeof(wlc_auth_info_t));
	auth_info = NULL;
}

/* JRK TBD */
static int
wlc_auth_scb_init(void *context, struct scb *scb)
{
	wlc_auth_info_t *auth_info = (wlc_auth_info_t *)context;
	struct auth_cubby *cubby_info = SCB_AUTH_INFO(auth_info, scb);

	ASSERT(cubby_info);

	cubby_info->cubby = (scb_auth_t *)MALLOCZ(auth_info->wlc->osh, sizeof(scb_auth_t));

	if (cubby_info->cubby == NULL)
		return BCME_NOMEM;

	return BCME_OK;
}

static void
wlc_auth_scb_deinit(void *context, struct scb *scb)
{
	struct auth_cubby *cubby_info;
	int idx;
	wlc_auth_info_t *auth_info = (wlc_auth_info_t *)context;
	wlc_bsscfg_t *bsscfg;

	wlc_auth_cleanup_scb(auth_info->wlc, scb);

	FOREACH_BSS(auth_info->wlc, idx, bsscfg) {
		authenticator_t *auth = (authenticator_t *)bsscfg->authenticator;
		if (auth && auth->scb_auth_in_prog == scb)
			auth->scb_auth_in_prog = NULL;
	}

	cubby_info = SCB_AUTH_INFO(auth_info, scb);

	if ((cubby_info != NULL) && (cubby_info->cubby != NULL)) {
		MFREE(auth_info->wlc->osh, cubby_info->cubby, sizeof(scb_auth_t));
		cubby_info->cubby = NULL;
	}

	return;
}

static int
wlc_auth_down(void *handle)
{
	return 0;
}

static int
wlc_auth_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
#ifdef BCMDBG
	wlc_auth_info_t *auth_info = (wlc_auth_info_t*)handle;
	wlc_info_t *wlc = auth_info->wlc;
	authenticator_t *auth;
	wlc_bsscfg_t *bsscfg;
	struct scb *scb;
	int32 int_val = 0;
	int err = BCME_OK;

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);
	auth = bsscfg->authenticator;
	ASSERT(auth != NULL);

	switch (actionid) {
	case IOV_SVAL(IOV_AUTH_PTK_M1):
		scb = wlc_scbfind(wlc, bsscfg, (struct ether_addr*)a);
		if (!(scb && SCB_AUTHORIZED(scb) && !AUTH_IN_PROGRESS(auth))) {
			err = BCME_ERROR;
			break;
		}
		/* restart 4-way handshake */
		wlc_auth_cleanup_scb(wlc, scb);
		auth->flags &= ~AUTH_FLAG_GTK_PLUMBED;
		wlc_auth_join_complete(auth, &scb->ea, TRUE);
		break;
	case IOV_SVAL(IOV_AUTH_REKEY_INIT): {
		if (int_val) {
			if (auth->flags & AUTH_FLAG_REKEY_ENAB)
				/* already enabled */
				break;
			if (wlc_bss_assocscb_getcnt(wlc, bsscfg)) {
				/* must be enabled before association */
				err = BCME_ASSOCIATED;
				break;
			}
			/* enable tx M1 */
			auth->flags |= AUTH_FLAG_REKEY_ENAB;
		} else {	/* disable tx M1 */
			struct scb_iter scbiter;
			if (!(auth->flags & AUTH_FLAG_REKEY_ENAB))
				/* already disabled */
				break;
			if (AUTH_IN_PROGRESS(auth)) {
				err = BCME_BUSY;
				break;
			}
			/* clean up scb */
			FOREACHSCB(wlc->scbstate, &scbiter, scb) {
				if (scb->bsscfg == bsscfg)
					wlc_auth_cleanup_scb(wlc, scb);
			}
			auth->flags &= ~AUTH_FLAG_REKEY_ENAB;
		}
		break;
	}
	case IOV_SVAL(IOV_AUTH_GTK_M1):
	case IOV_SVAL(IOV_AUTH_GTK_BAD_M1): {
		uint16 flags;
		wpapsk_t *wpa;
		scb_auth_t *auth_cubby;
		if (!(auth->flags & AUTH_FLAG_REKEY_ENAB)) {
			err = BCME_EPERM;
			break;
		}
		scb = wlc_scbfind(wlc, bsscfg, (struct ether_addr*)a);
		if (!(scb && SCB_ASSOCIATED(scb) && !AUTH_IN_PROGRESS(auth))) {
			err = BCME_ERROR;
			break;
		}

		if (!bcmwpa_is_rsn_auth(bsscfg->WPA_auth) ||
			wlc_bss_assocscb_getcnt(wlc, bsscfg) != 1) {
			err = BCME_UNSUPPORTED;
			break;
		}
		/* rekey */
		flags = KEY_DESC(auth, scb);
		if (actionid == IOV_SVAL(IOV_AUTH_GTK_BAD_M1))
			/* set for negtive test */
			flags |= WPA_KEY_ERROR;
		auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
		wpa = auth_cubby->wpa;
		wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);
		/* If test is positive generate a new key */
		if (!(flags & WPA_KEY_ERROR)) {
			wlc_auth_new_gtk(auth);
			if (wlc_auth_plumb_gtk(auth, wpa->mcipher) == -1) {
				err = BCME_ERROR;
				break;
			}
		}
		err = wlc_wpa_auth_sendeapol(auth, flags, GMSG_REKEY, scb);
		break;
	}
	default:
		if (P2P_ENAB(wlc->pub))
			break;

		WL_ERROR(("wl%d: %s: iovar %s get/set requires p2p enabled\n",
		          wlc->pub->unit, __FUNCTION__, name));
		return BCME_ERROR;
	}

	return err;
#else
	return 0;
#endif	/* BCMDBG */
}

int
wlc_auth_set_pmk(authenticator_t *auth, wsec_pmk_t *pmk)
{
	if (auth == NULL || pmk == NULL) {
		if (auth) {
			WL_WSEC(("wl%d: %s: missing required parameter\n",
				auth->wlc->pub->unit, __FUNCTION__));
		} else {
			WL_WSEC(("%s: null auth\n", __FUNCTION__));
		}
		return BCME_BADARG;
	}

	auth->flags &= ~AUTH_FLAG_PMK_PRESENT;

	if (pmk->flags & WSEC_PASSPHRASE) {
		if (pmk->key_len < WSEC_MIN_PSK_LEN ||
		    pmk->key_len > WSEC_MAX_PSK_LEN) {
			return BCME_BADARG;
		}  else if (pmk->key_len == WSEC_MAX_PSK_LEN) {
			wpapsk_info_t psk_info; /* temp variable to hold the cobbled pmk */
			if (wlc_wpa_cobble_pmk(&psk_info, (char *)pmk->key,
			                       pmk->key_len, NULL, 0) == 0) {
				bcopy(&psk_info.pmk, auth->psk, PMK_LEN);
				auth->flags |= AUTH_FLAG_PMK_PRESENT;
				return BCME_OK;
			}
		}
	} else if (pmk->key_len == PMK_LEN) {
		auth->flags |= AUTH_FLAG_PMK_PRESENT;
	} else
		return BCME_BADARG;

	bcopy((char*)pmk->key, auth->psk, pmk->key_len);
	auth->psk_len = pmk->key_len;

	return BCME_OK;
}

static bool
wlc_wpa_auth_recveapol(authenticator_t *auth, eapol_header_t *eapol, bool encrypted,
                       struct scb *scb)
{
	uint16 flags;
	wlc_info_t *wlc = auth->wlc;
	eapol_wpa_key_header_t *body = (eapol_wpa_key_header_t *)eapol->body;
	wpapsk_t *wpa;
	wpapsk_info_t *wpa_info;
	scb_auth_t *auth_cubby;
	uint16 key_info, wpaie_len;

	if (scb == NULL) {
		WL_ERROR(("wl%d: %s: SCB is null\n", auth->wlc->pub->unit,  __FUNCTION__));
		return FALSE;
	}

	WL_WSEC(("wl%d: %s: received EAPOL_WPA_KEY packet.\n",
	         wlc->pub->unit, __FUNCTION__));

	auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
	wpa = auth_cubby->wpa;
	wpa_info = auth_cubby->wpa_info;

	if (wpa == NULL || wpa_info == NULL) {
		WL_ERROR(("wl%d: %s: wpa or wpa_info is null\n",
			auth->wlc->pub->unit,  __FUNCTION__));
		return FALSE;
	}

	flags = KEY_DESC(auth, scb);
	switch (wpa->WPA_auth) {
	case WPA2_AUTH_PSK:
	case WPA_AUTH_PSK:
		break;
	default:
		WL_ERROR(("wl%d: %s: unexpected... %d\n",
			wlc->pub->unit, __FUNCTION__, wpa->WPA_auth));
		ASSERT(0);
		return FALSE;
	}

	key_info = ntoh16_ua(&body->key_info);

	/* check for replay */
	if (wpa_array_cmp(MAX_ARRAY, body->replay, wpa->replay, EAPOL_KEY_REPLAY_LEN) ==
	    wpa->replay) {
#ifdef BCMDBG
		uchar *g = body->replay, *s = wpa->replay;
		WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: ignoring replay "
			"(got %02x%02x%02x%02x%02x%02x%02x%02x"
			" last saw %02x%02x%02x%02x%02x%02x%02x%02x)\n",
			wlc->pub->unit,
			g[0], g[1], g[2], g[3], g[4], g[5], g[6], g[7],
			s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]));
#endif /* BCMDBG */
		return TRUE;
	}

	switch (wpa->state) {
	case WPA_AUTH_PTKINITNEGOTIATING:

		WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: processing message 2\n",
			wlc->pub->unit));

		if (((key_info & PMSG2_REQUIRED) != PMSG2_REQUIRED) ||
		    ((key_info & PMSG2_PROHIBITED) != 0)) {
			WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: incorrect key_info 0x%x\n",
				wlc->pub->unit, key_info));
			return TRUE;
		}

		/* Save snonce and produce PTK */
		bcopy((char *)body->nonce, wpa->snonce, sizeof(wpa->anonce));

#ifdef MFP
		if (WLC_MFP_ENAB(auth->wlc->pub) && SCB_SHA256(scb)) {
			kdf_calc_ptk(&scb->ea, &auth->bsscfg->cur_etheraddr,
				wpa->anonce, wpa->snonce, wpa_info->pmk,
				wpa_info->pmk_len, wpa->eapol_mic_key, wpa->ptk_len);
		} else
#endif // endif
		if (!memcmp(&scb->ea, &auth->bsscfg->cur_etheraddr, ETHER_ADDR_LEN)) {
			/* something is wrong -- toss; this shouldn't happen */
			WL_WSEC(("wl%d: %s: invalid eapol; identical mac addrs; discard\n",
				auth->wlc->pub->unit, __FUNCTION__));
			return TRUE;
		} else {
			wpa_calc_ptk(&scb->ea, &auth->bsscfg->cur_etheraddr,
				wpa->anonce, wpa->snonce, wpa_info->pmk,
				wpa_info->pmk_len, wpa->eapol_mic_key, wpa->ptk_len);
		}

		/* check message MIC */

		/* ensure we have the MIC field */
		if ((key_info & WPA_KEY_MIC) &&
			(ntoh16(eapol->length) < OFFSETOF(eapol_wpa_key_header_t, data_len))) {
			WL_WSEC(("wl%d: %s: bad eapol - short frame, no mic.\n",
				WLCWLUNIT(wlc), __FUNCTION__));
			WLCNTINCR(wlc->pub->_cnt->rxrunt);
			return TRUE; /* consume it */
		}

		if ((key_info & WPA_KEY_MIC) &&
		    !wpa_check_mic(eapol, key_info & (WPA_KEY_DESC_V1|WPA_KEY_DESC_V2),
		                   wpa->eapol_mic_key)) {
			/* 802.11-2007 clause 8.5.3.2 - silently discard MIC failure */
			WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: MIC failure, discarding pkt\n",
			         wlc->pub->unit));
			return TRUE;
		}

		/* check the IE */
		if (ntoh16(eapol->length) < OFFSETOF(eapol_wpa_key_header_t, data)) {
			WL_WSEC(("wl%d: %s: bad eapol - short frame, no data_len.\n",
				WLCWLUNIT(wlc), __FUNCTION__));
			WLCNTINCR(wlc->pub->_cnt->rxrunt);
			return TRUE; /* consume it */
		}

		wpaie_len = ntoh16_ua(&body->data_len);
		if (ntoh16(eapol->length) < (wpaie_len +
			OFFSETOF(eapol_wpa_key_header_t, data))) {
			WL_WSEC(("wl%d: %s: bad eapol - short frame, no wpa ie.\n",
				WLCWLUNIT(wlc), __FUNCTION__));
			WLCNTINCR(wlc->pub->_cnt->rxrunt);
			return TRUE; /* consume it */
		}

		if (!wpaie_len || wpaie_len != wpa->sup_wpaie_len ||
		    bcmp(body->data, wpa->sup_wpaie, wpaie_len) != 0) {
			WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: wpaie does not match\n",
				wlc->pub->unit));
			wlc_wpa_senddeauth(auth->bsscfg, (char *)&scb->ea,
				DOT11_RC_WPA_IE_MISMATCH);
			return TRUE;
		}

		/* clear older timer */
		wpa->retries = 0;
		wl_del_timer(auth->wlc->wl, wpa_info->retry_timer);

		/* if MIC was okay, increment counter */
		wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);

		/* send msg 3 */
		wlc_wpa_auth_sendeapol(auth, flags, PMSG3, scb);

		break;

	case WPA_AUTH_PTKINITDONE:

		WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: processing message 4\n",
			wlc->pub->unit));

		if (((key_info & PMSG4_REQUIRED) != PMSG4_REQUIRED) ||
		    ((key_info & PMSG4_PROHIBITED) != 0)) {
			WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: incorrect key_info 0x%x\n",
				wlc->pub->unit, key_info));
			return TRUE;
		}

		/* check message MIC */
		if ((key_info & WPA_KEY_MIC) &&
		    !wpa_check_mic(eapol, key_info & (WPA_KEY_DESC_V1|WPA_KEY_DESC_V2),
		                   wpa->eapol_mic_key)) {
			/* 802.11-2007 clause 8.5.3.4 - silently discard MIC failure */
			WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: MIC failure, discarding pkt\n",
			         wlc->pub->unit));
			return TRUE;
		}

		/* clear older timer */
		wpa->retries = 0;
		wl_del_timer(auth->wlc->wl, wpa_info->retry_timer);

		/* Plumb paired key */
		wlc_wpa_plumb_tk(wlc, auth->bsscfg, (uint8*)wpa->temp_encr_key,
			wpa->tk_len, wpa->ucipher, &scb->ea);

		if (wpa->WPA_auth == WPA2_AUTH_PSK)
			wpa->state = WPA_AUTH_KEYUPDATE;
		else if (wpa->WPA_auth == WPA_AUTH_PSK) {
			WL_WSEC(("wl%d: %s: Moving into WPA_AUTH_REKEYNEGOTIATING\n",
			         wlc->pub->unit, __FUNCTION__));
			wpa->state = WPA_AUTH_REKEYNEGOTIATING;
			/* create the GTK */
			if (!(auth->flags & AUTH_FLAG_GTK_PLUMBED)) {
				wlc_auth_new_gtk(auth);
				if (wlc_auth_plumb_gtk(auth, wpa->mcipher) == -1) {
					WL_ERROR(("%s: Failed to plumb GTK\n",
						__FUNCTION__));
					break;
				}
			}
			wpa_incr_array(wpa->replay, EAPOL_KEY_REPLAY_LEN);
			wlc_wpa_auth_sendeapol(auth, flags, GMSG1, scb);
		}
		if (wpa->WPA_auth == WPA2_AUTH_PSK || wpa->WPA_auth == WPA_AUTH_PSK)
			wlc_ioctl(wlc, WLC_SCB_AUTHORIZE, &scb->ea, ETHER_ADDR_LEN,
			          auth->bsscfg->wlcif);

		/* can support a new authorization now */
		if (wpa->WPA_auth == WPA2_AUTH_PSK)
			wlc_auth_endassoc(auth);
		break;

	case WPA_AUTH_REKEYNEGOTIATING:

		WL_WSEC(("wl%d: %s: Processing group key message 2\n", wlc->pub->unit,
		         __FUNCTION__));

		/* check the MIC */

		/* ensure we have the MIC field */
		if ((key_info & WPA_KEY_MIC) &&
			(ntoh16(eapol->length) < OFFSETOF(eapol_wpa_key_header_t, data_len))) {
			WL_WSEC(("wl%d: %s: bad eapol - short frame, no mic.\n",
				WLCWLUNIT(wlc), __FUNCTION__));
			WLCNTINCR(wlc->pub->_cnt->rxrunt);
			return TRUE; /* consume it */
		}

		if ((key_info & WPA_KEY_MIC) &&
		    !wpa_check_mic(eapol, key_info & (WPA_KEY_DESC_V1|WPA_KEY_DESC_V2),
		                   wpa->eapol_mic_key)) {
			/* 802.11-2007 clause 8.5.3.4 - silently discard MIC failure */
			WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: GMSG1 - MIC failure, "
			         "discarding pkt\n",
			         wlc->pub->unit));
			return TRUE;
		}

		/* clear older timer */
		wpa->retries = 0;
		wl_del_timer(auth->wlc->wl, wpa_info->retry_timer);
		wpa->state = WPA_AUTH_KEYUPDATE;
		wlc_auth_endassoc(auth);
		break;

	default:
		WL_WSEC(("wl%d: wlc_wpa_auth_recveapol: unexpected state\n",
			wlc->pub->unit));
	}

	return TRUE;
}

static int
auth_get_group_rsc(authenticator_t *auth, uint8 *buf, int indx)
{
	union {
		int index;
		uint8 rsc[EAPOL_WPA_KEY_RSC_LEN];
	} u;

	u.index = indx;
	if (wlc_ioctl(auth->wlc, WLC_GET_KEY_SEQ, &u, sizeof(u), auth->bsscfg->wlcif) != 0)
		return -1;

	bcopy(u.rsc, buf, EAPOL_WPA_KEY_RSC_LEN);

	return 0;
}

static int
wlc_auth_insert_gtk(authenticator_t *auth, eapol_header_t *eapol, uint16 *data_len)
{
	eapol_wpa_key_header_t *body = (eapol_wpa_key_header_t *)eapol->body;
	eapol_wpa2_encap_data_t *data_encap;
	uint16 len = *data_len;
	eapol_wpa2_key_gtk_encap_t *gtk_encap;

	if (auth_get_group_rsc(auth, &auth->gtk_rsc[0], auth->gtk_key_index)) {
		/* Don't use what we don't have. */
		memset(auth->gtk_rsc, 0, sizeof(auth->gtk_rsc));
	}

	/* insert GTK into eapol message */
	/*	body->key_len = htons(wpa->gtk_len); */
	/* key_len is PTK len, gtk len is implicit in encapsulation */
	data_encap = (eapol_wpa2_encap_data_t *) (body->data + len);
	data_encap->type = DOT11_MNG_PROPR_ID;
	data_encap->length = (EAPOL_WPA2_ENCAP_DATA_HDR_LEN - TLV_HDR_LEN) +
	        EAPOL_WPA2_KEY_GTK_ENCAP_HDR_LEN + auth->gtk_len;
	bcopy(WPA2_OUI, data_encap->oui, DOT11_OUI_LEN);
	data_encap->subtype = WPA2_KEY_DATA_SUBTYPE_GTK;
	len += EAPOL_WPA2_ENCAP_DATA_HDR_LEN;
	gtk_encap = (eapol_wpa2_key_gtk_encap_t *) (body->data + len);
	gtk_encap->flags = (auth->gtk_id << WPA2_GTK_INDEX_SHIFT) & WPA2_GTK_INDEX_MASK;
	bcopy(auth->gtk, gtk_encap->gtk, auth->gtk_len);
	len += auth->gtk_len + EAPOL_WPA2_KEY_GTK_ENCAP_HDR_LEN;

	/* copy in the gtk rsc */
	bcopy(auth->gtk_rsc, body->rsc, sizeof(body->rsc));

	/* return the adjusted data len */
	*data_len = len;

	return (auth->gtk_len + EAPOL_WPA2_KEY_GTK_ENCAP_HDR_LEN + EAPOL_WPA2_ENCAP_DATA_HDR_LEN);
}

/* Build and send an EAPOL WPA key message */
static bool
wlc_wpa_auth_sendeapol(authenticator_t *auth, uint16 flags, wpa_msg_t msg, struct scb *scb)
{
	wlc_info_t *wlc = auth->wlc;
	wpapsk_t *wpa;
	wpapsk_info_t *wpa_info;
	scb_auth_t *auth_cubby;
	uint16 len, key_desc, data_len, buf_len;
	void *p = NULL;
	eapol_header_t *eapol_hdr = NULL;
	eapol_wpa_key_header_t *wpa_key = NULL;
	uchar mic[PRF_OUTBUF_LEN];
	bool add_mic = FALSE;

	if (scb == NULL) {
		WL_ERROR(("wl%d: %s: SCB is null\n", auth->wlc->pub->unit,  __FUNCTION__));
		return FALSE;
	}

	auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
	wpa = auth_cubby->wpa;
	wpa_info = auth_cubby->wpa_info;

	len = EAPOL_HEADER_LEN + EAPOL_WPA_KEY_LEN;
	switch (msg) {
	case PMSG1:		/* pair-wise msg 1 */
		if ((p = wlc_eapol_pktget(wlc, auth->bsscfg, &scb->ea, len)) == NULL)
			break;
		eapol_hdr = (eapol_header_t *) PKTDATA(wlc->osh, p);
		eapol_hdr->length = hton16(EAPOL_WPA_KEY_LEN);
		wpa_key = (eapol_wpa_key_header_t *) eapol_hdr->body;
		bzero((char *)wpa_key, EAPOL_WPA_KEY_LEN);
		hton16_ua_store((flags | PMSG1_REQUIRED), (uint8 *)&wpa_key->key_info);
		hton16_ua_store(wpa->tk_len, (uint8 *)&wpa_key->key_len);
		wlc_getrand(wlc, wpa->anonce, EAPOL_WPA_KEY_NONCE_LEN);
		bcopy(wpa->anonce, wpa_key->nonce, EAPOL_WPA_KEY_NONCE_LEN);
		/* move to next state */
		wpa->state = WPA_AUTH_PTKINITNEGOTIATING;
		WL_WSEC(("wl%d: wlc_wpa_auth_sendeapol: sending message 1\n",
			wlc->pub->unit));
		break;

	case PMSG3:		/* pair-wise msg 3 */
		if (wpa->WPA_auth == WPA2_AUTH_PSK) {
#ifdef WLBTAMP
			if (BSS_BTA_ENAB(wlc, auth->bsscfg))
				flags |= PMSG3_BRCM_REQUIRED;
			else
#endif /* WLBTAMP */
				flags |= PMSG3_WPA2_REQUIRED;
		} else if (wpa->WPA_auth == WPA_AUTH_PSK) {
			flags |= PMSG3_REQUIRED;
		}
		else
			/* nothing else supported for now */
			ASSERT(0);

		data_len = wpa->auth_wpaie_len;
		len += data_len;
		buf_len = 0;
		if (wpa->WPA_auth == WPA2_AUTH_PSK && (flags & WPA_KEY_ENCRYPTED_DATA)) {
			if (!(auth->flags & AUTH_FLAG_GTK_PLUMBED)) {
				/* Cobble the key and plumb it. */
				wlc_auth_new_gtk(auth);
				if (wlc_auth_plumb_gtk(auth, wpa->mcipher) == -1) {
					WL_ERROR(("%s: failed to plumb gtk\n",
						__FUNCTION__));
					break;
				}
			}
#ifdef MFP
			if (WLC_MFP_ENAB(auth->wlc->pub)) {
				wlc_key_t *igtk;
				wlc_key_info_t key_info;
				igtk = wlc_keymgmt_get_bss_tx_key(auth->wlc->keymgmt,
					auth->bsscfg, TRUE, &key_info);
				if (igtk != NULL && key_info.key_len > 0) {
					buf_len += key_info.key_len;
					buf_len += EAPOL_WPA2_KEY_IGTK_ENCAP_HDR_LEN + 8
						+ EAPOL_WPA2_ENCAP_DATA_HDR_LEN;
				} else {
					WL_WSEC(("wl%d.%d: %s: no igtk for tx\n",
						WLCWLUNIT(auth->wlc), WLC_BSSCFG_IDX(auth->bsscfg),
						__FUNCTION__));
				}
			}
#endif /* MFP */

			/* add 8+8 for extra aes bytes and possible padding */
			buf_len += auth->gtk_len + EAPOL_WPA2_KEY_GTK_ENCAP_HDR_LEN + 8
				+ EAPOL_WPA2_ENCAP_DATA_HDR_LEN;
			/* The encryption result has to be 8-byte aligned */
			if (data_len % AKW_BLOCK_LEN)
				buf_len += (AKW_BLOCK_LEN - (data_len % AKW_BLOCK_LEN));
		}

		buf_len += len;

		if ((p = wlc_eapol_pktget(wlc, auth->bsscfg, &scb->ea, buf_len)) == NULL)
			break;

		eapol_hdr = (eapol_header_t *) PKTDATA(wlc->osh, p);
		eapol_hdr->length = EAPOL_WPA_KEY_LEN;
		wpa_key = (eapol_wpa_key_header_t *) eapol_hdr->body;
		bzero((char *)wpa_key, EAPOL_WPA_KEY_LEN);

		hton16_ua_store(flags, (uint8 *)&wpa_key->key_info);
		hton16_ua_store(wpa->tk_len, (uint8 *)&wpa_key->key_len);
		bcopy(wpa->anonce, wpa_key->nonce, EAPOL_WPA_KEY_NONCE_LEN);
		bcopy((char *)wpa->auth_wpaie, (char *)wpa_key->data,
			wpa->auth_wpaie_len);
		wpa_key->data_len = hton16(data_len);
		if (wpa->WPA_auth == WPA2_AUTH_PSK && (flags & WPA_KEY_ENCRYPTED_DATA)) {
			wlc_auth_insert_gtk(auth, eapol_hdr, &data_len);
			wpa_key->data_len = hton16(data_len);

#ifdef MFP
			if (WLC_MFP_ENAB(auth->wlc->pub) && SCB_MFP(scb)) {
				wlc_mfp_insert_igtk(AUTHMFP(auth), auth->bsscfg,
					eapol_hdr, &data_len);
				wpa_key->data_len = hton16(data_len);
			}
#endif // endif

			/* encrypt key data field */
			if (wlc_auth_encr_key_data(auth, wpa, wpa_key, flags,
				eapol_hdr->version) == -1)
			{
				WL_ERROR(("wl%d: %s: error encrypting key "
				          "data\n", wlc->pub->unit, __FUNCTION__));
				PKTFREE(wlc->osh, p, FALSE);
				p = NULL;
				break;
			}
		} /* if (flags & WPA_KEY_ENCRYPTED_DATA) */
		/* encr algorithm might change the data_len, so pick up the update */
		eapol_hdr->length += ntoh16_ua((uint8 *)&wpa_key->data_len);
		eapol_hdr->length = hton16(eapol_hdr->length);
		add_mic = TRUE;
		wpa->state = WPA_AUTH_PTKINITDONE;
		WL_WSEC(("wl%d: %s: sending message 3\n",
		         wlc->pub->unit, __FUNCTION__));
		break;

	case GMSG1: /* WPA_REKEYNEGOTIATING */ {
		int key_index;
		ASSERT(wpa->WPA_auth == WPA_AUTH_PSK);
		flags |= GMSG1_REQUIRED;
		key_index = (auth->gtk_id << WPA_KEY_INDEX_SHIFT) & WPA_KEY_INDEX_MASK;
		flags |= key_index;
		WL_WSEC(("auth->gtk_id is %u, key_index %u\n", auth->gtk_id, key_index));
		data_len = 0;
		/* make sure to pktget the EAPOL frame length plus the length of the body */
		len += auth->gtk_len;

		if (flags & WPA_KEY_DESC_V2_OR_V3)
			len += 8;

		if ((p = wlc_eapol_pktget(wlc, auth->bsscfg, &scb->ea, len)) == NULL)
			break;

		eapol_hdr = (eapol_header_t *) PKTDATA(wlc->osh, p);
		eapol_hdr->length = EAPOL_WPA_KEY_LEN;
		wpa_key = (eapol_wpa_key_header_t *) eapol_hdr->body;
		bzero((char *)wpa_key, len - EAPOL_HEADER_LEN);
		hton16_ua_store(flags, (uint8 *)&wpa_key->key_info);
		hton16_ua_store(auth->gtk_len, (uint8 *)&wpa_key->key_len);
		bcopy(auth->gnonce, wpa_key->nonce, EAPOL_WPA_KEY_NONCE_LEN);
		bcopy(&auth->global_key_counter[KEY_COUNTER_LEN-16], wpa_key->iv, 16);
		bcopy(auth->gtk_rsc, wpa_key->rsc, sizeof(wpa_key->rsc));

		data_len = auth->gtk_len;
		bcopy(auth->gtk, wpa_key->data, data_len);
		wpa_key->data_len = hton16(data_len);

		if (wlc_auth_encr_key_data(auth, wpa, wpa_key,
			(flags&(WPA_KEY_DESC_V1|WPA_KEY_DESC_V2)),
			eapol_hdr->version) == -1)
		{
			WL_ERROR(("%s: failed to encrypt key data\n", __FUNCTION__));
			PKTFREE(wlc->osh, p, FALSE);
			p = NULL;
			break;
		}
		eapol_hdr->length += ntoh16_ua((uint8 *)&wpa_key->data_len);
		eapol_hdr->length = hton16(eapol_hdr->length);
		add_mic = TRUE;
		wpa->state = WPA_AUTH_REKEYNEGOTIATING;
		WL_WSEC(("wl%d: wlc_wpa_auth_sendeapol: sending message G1\n",
			wlc->pub->unit));
		break;
	}

	case GMSG_REKEY: {	/* sending gtk rekeying request */
#ifdef BCMDBG
		bool neg_test;

		neg_test = (flags & WPA_KEY_ERROR) ? TRUE : FALSE;
		flags &= ~WPA_KEY_ERROR;
#endif /* BCMDBG */

		if (wpa->WPA_auth != WPA2_AUTH_PSK)
			return FALSE;

		flags |= (GMSG1_REQUIRED | WPA_KEY_ENCRYPTED_DATA);

		data_len = wpa->auth_wpaie_len;
		len += data_len;
		buf_len = 0;

#ifdef MFP
		if (WLC_MFP_ENAB(auth->wlc->pub)) {
			buf_len += AES_TK_LEN;
			buf_len += EAPOL_WPA2_KEY_IGTK_ENCAP_HDR_LEN + 8 +
				EAPOL_WPA2_ENCAP_DATA_HDR_LEN;
		}
#endif // endif
		/* add 8+8 for extra aes bytes and possible padding */
		buf_len += auth->gtk_len + EAPOL_WPA2_KEY_GTK_ENCAP_HDR_LEN + 8 +
			EAPOL_WPA2_ENCAP_DATA_HDR_LEN;
		/* The encryption result has to be 8-byte aligned */
		if (data_len % AKW_BLOCK_LEN)
			buf_len += (AKW_BLOCK_LEN - (data_len % AKW_BLOCK_LEN));

		buf_len += len;

		if ((p = wlc_eapol_pktget(wlc, auth->bsscfg, &scb->ea, buf_len)) == NULL)
			break;

		eapol_hdr = (eapol_header_t *) PKTDATA(wlc->osh, p);
		eapol_hdr->length = EAPOL_WPA_KEY_LEN;
		wpa_key = (eapol_wpa_key_header_t *) eapol_hdr->body;
		bzero((char *)wpa_key, EAPOL_WPA_KEY_LEN);

		hton16_ua_store(flags, (uint8 *)&wpa_key->key_info);
		hton16_ua_store(wpa->tk_len, (uint8 *)&wpa_key->key_len);
		bcopy(wpa->anonce, wpa_key->nonce, EAPOL_WPA_KEY_NONCE_LEN);
		bcopy((char *)wpa->auth_wpaie, (char *)wpa_key->data, wpa->auth_wpaie_len);
		wpa_key->data_len = hton16(data_len);

#ifdef BCMDBG
		if (!neg_test) {
#endif /* BCMDBG */
			/* for negtive test not include gtk encapsulation */
			wlc_auth_insert_gtk(auth, eapol_hdr, &data_len);
			wpa_key->data_len = hton16(data_len);
#ifdef BCMDBG
		}
#endif /* BCMDBG */

#ifdef MFP
		if (WLC_MFP_ENAB(auth->wlc->pub)) {
			wlc_mfp_insert_igtk(AUTHMFP(auth), auth->bsscfg, eapol_hdr, &data_len);
			wpa_key->data_len = hton16(data_len);
		}
#endif // endif

		if (wlc_auth_encr_key_data(auth, wpa, wpa_key, flags, eapol_hdr->version) == -1) {
			WL_ERROR(("%s: failed to encrypt key data\n", __FUNCTION__));
			PKTFREE(wlc->osh, p, FALSE);
			p = NULL;
			break;
		}

		/* encr algorithm might change the data_len, so pick up the update */
		eapol_hdr->length += ntoh16_ua((uint8 *)&wpa_key->data_len);
		eapol_hdr->length = hton16(eapol_hdr->length);
		add_mic = TRUE;
#ifdef BCMDBG
		if (!neg_test)
#endif /* BCMDBG */
			wpa->state = WPA_AUTH_REKEYNEGOTIATING;
		WL_WSEC(("wl%d: %s: sending group rekeying message\n",
		         wlc->pub->unit, __FUNCTION__));
		break;
	}

	default:
		WL_ERROR(("wl%d: %s: unexpected message type %d\n",
		         wlc->pub->unit, __FUNCTION__, msg));
		break;
	}

	if (p != NULL) {
		/* do common message fields here; make and copy MIC last. */
		eapol_hdr->type = EAPOL_KEY;
		if (wpa->WPA_auth == WPA2_AUTH_PSK)
			wpa_key->type = EAPOL_WPA2_KEY;
		else
			wpa_key->type = EAPOL_WPA_KEY;
		bcopy((char *)wpa->replay, (char *)wpa_key->replay,
		      EAPOL_KEY_REPLAY_LEN);
		key_desc = flags & (WPA_KEY_DESC_V1 |  WPA_KEY_DESC_V2);
		if (add_mic) {
			if (!wpa_make_mic(eapol_hdr, key_desc, wpa->eapol_mic_key,
				mic)) {
				WL_WSEC(("wl%d: %s: MIC generation failed\n",
				         wlc->pub->unit, __FUNCTION__));
				return FALSE;
			}
			bcopy(mic, wpa_key->mic, EAPOL_WPA_KEY_MIC_LEN);
		}

#ifdef EXT_STA
		/* for vista, mark the pkt as exempt */
		if (WLEXTSTA_ENAB(wlc->pub) && (msg == PMSG1 || msg == PMSG3))
		    WLPKTFLAG_EXEMPT_SET(WLPKTTAG(p), WSEC_EXEMPT_ALWAYS);
#endif /* EXT_STA */

#ifdef WLBTAMP
		if (BSS_BTA_ENAB(wlc, auth->bsscfg)) {
			struct ether_header *eh;
			struct dot11_llc_snap_header *lsh;

			/*
			 * re-encap packet w/ BT-SIG LLC/SNAP header and security prot ID:
			 * Step 1: convert Ethernet to 802.3 per 802.1H
			 * Step 2: replace RFC1042 SNAP header with BT-SIG encap header
			 * Step 3: replace ether_type with BT-SIG security prot ID
			 */
			eh = (struct ether_header *)PKTDATA(wlc->osh, p);
			wlc_ether_8023hdr(wlc, wlc->osh, eh, p);

			eh = (struct ether_header *)PKTDATA(wlc->osh, p);
			lsh = (struct dot11_llc_snap_header *)&eh[1];
			bcopy(BT_SIG_SNAP_MPROT, (char *)lsh, DOT11_LLC_SNAP_HDR_LEN - 2);

			lsh->type = hton16((uint16)BTA_PROT_SECURITY);
		}
#endif /* WLBTAMP */

		wlc_sendpkt(wlc, p, auth->bsscfg->wlcif);

		/* start the retry timer */
		wl_add_timer(wlc->wl, wpa_info->retry_timer, AUTH_WPA2_RETRY_TIMEOUT, 0);

		return TRUE;
	}
	return FALSE;
}

static bool
wlc_auth_wpapsk_start(authenticator_t *auth, uint8 *sup_ies, uint sup_ies_len,
                      uint8 *auth_ies, uint auth_ies_len, struct scb *scb)
{
	wpapsk_info_t *wpa_info;
	wpapsk_t *wpa;
	scb_auth_t *auth_cubby;
	uint16 flags;
	bool ret = TRUE;

	if (sup_ies == NULL) {
		return FALSE;
	}
	if (scb == NULL) {
		WL_ERROR(("wl%d: %s: SCB is null\n", auth->wlc->pub->unit,  __FUNCTION__));
		return FALSE;
	}

	/* find the cubby */
	auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
	wpa = auth_cubby->wpa;
	wpa_info = auth_cubby->wpa_info;

	wlc_wpapsk_free(auth->wlc, wpa);

	wpa->state = WPA_AUTH_INITIALIZE;
	/*
	 * XXX: you can't just inherit all the wpa_auth capabilities of the bsscfg,
	 * need to pick the right wpa auth for the current association
	 * and go from there
	*/
	{
		wpa_ie_fixed_t *ie = (wpa_ie_fixed_t *)sup_ies;
		if (ie->tag == DOT11_MNG_RSN_ID)
			wpa->WPA_auth = auth->bsscfg->WPA_auth & WPA2_AUTH_PSK;
		else
			wpa->WPA_auth = auth->bsscfg->WPA_auth & WPA_AUTH_PSK;
	}

	if (!wlc_wpapsk_start(auth->wlc, wpa, sup_ies, sup_ies_len,
		auth_ies, auth_ies_len)) {
		WL_ERROR(("wl%d: wlc_wpapsk_start() failed\n",
		        auth->wlc->pub->unit));
		return FALSE;
	}

	if ((auth->auth_type == AUTH_WPAPSK) && (wpa_info->pmk_len == 0)) {
		WL_WSEC(("wl%d: %s: no PMK material found\n",
		         auth->wlc->pub->unit, __FUNCTION__));
		return FALSE;
	}

	/* clear older timer */
	wpa->retries = 0;
	wl_del_timer(auth->wlc->wl, wpa_info->retry_timer);

	wpa->state = WPA_AUTH_PTKSTART;
	flags = KEY_DESC(auth, scb);
	wlc_wpa_auth_sendeapol(auth, flags, PMSG1, scb);

	return ret;
}

bool
wlc_set_auth(authenticator_t *auth, int auth_type, uint8 *sup_ies, uint sup_ies_len,
             uint8 *auth_ies, uint auth_ies_len, struct scb *scb)
{
	bool ret = TRUE;

	if (auth == NULL) {
		WL_WSEC(("wlc_set_auth called with NULL auth context\n"));
		return FALSE;
	}

	/* sanity */
	/* ASSERT(auth->auth_type == AUTH_UNUSED); */

	if (auth_type == AUTH_WPAPSK) {
		auth->auth_type = auth_type;
		ret = wlc_auth_wpapsk_start(auth, sup_ies, sup_ies_len, auth_ies, auth_ies_len,
		                            scb);
	} else {
		WL_ERROR(("wl%d: %s: unexpected auth type %d\n",
		         auth->wlc->pub->unit, __FUNCTION__, auth_type));
		return FALSE;
	}
	return ret;
}

/* Dispatch EAPOL to authenticator.
 * Return boolean indicating whether it should be freed or sent up.
 */
bool
wlc_auth_eapol(authenticator_t *auth, eapol_header_t *eapol_hdr, bool encrypted, struct scb *scb)
{
	if (!auth) {
		/* no unit to report if this happens */
		WL_ERROR(("%s: called with NULL auth\n", __FUNCTION__));
		return FALSE;
	}

	if ((eapol_hdr->type == EAPOL_KEY) && (auth->auth_type == AUTH_WPAPSK)) {
		eapol_wpa_key_header_t *body;

		/* ensure we have all of fixed eapol_wpa_key_header_t fields */
		if (ntoh16(eapol_hdr->length) < OFFSETOF(eapol_wpa_key_header_t, mic)) {
			WL_WSEC(("wl%d: %s: bad eapol - header too small.\n",
				WLCWLUNIT(auth->wlc), __FUNCTION__));
			WLCNTINCR(auth->wlc->pub->_cnt->rxrunt);
			return FALSE;
		}

		body = (eapol_wpa_key_header_t *)eapol_hdr->body;
		if (body->type == EAPOL_WPA2_KEY || body->type == EAPOL_WPA_KEY) {
			wlc_wpa_auth_recveapol(auth, eapol_hdr, encrypted, scb);
			return TRUE;
		}
	}

	return FALSE;
}

void
wlc_auth_join_complete(authenticator_t *auth, struct ether_addr *ea, bool initialize)
{
	wlc_info_t *wlc = auth->wlc;
	wlc_bsscfg_t *bsscfg = auth->bsscfg;
	struct scb *scb;
	wpapsk_info_t *wpa_info;
	scb_auth_t *auth_cubby;
	uint auth_ies_len;
	uint8 *auth_ies;
	bool stat = 0;

	scb = wlc_scbfindband(wlc, bsscfg, ea,
	                      CHSPEC_WLCBANDUNIT(bsscfg->current_bss->chanspec));
	if (!scb) {
#ifdef BCMDBG_ERR
		char eabuf[ETHER_ADDR_STR_LEN];
		WL_ERROR(("wl%d: %s: scb not found for ea %s\n",
		          wlc->pub->unit, __FUNCTION__, bcm_ether_ntoa(ea, eabuf)));
#endif // endif
		return;
	}

	/* can only support one STA authenticating at a time */
	if (initialize && AUTH_IN_PROGRESS(auth)) {
		WL_ERROR(("wl%d: %s: Authorization blocked for current authorization in progress\n",
		         auth->wlc->pub->unit, __FUNCTION__));
		return;
	}

	if (initialize) {
		if (wlc_auth_prep_scb(auth, scb) != BCME_OK)
			return;
	}

	auth_cubby = SCB_AUTH_CUBBY(auth->auth_info, scb);
	wpa_info = auth_cubby->wpa_info;

	if (!(auth->flags & AUTH_FLAG_GTK_PLUMBED))
		wlc_auth_initialize_gkc(auth);

	/* init per scb WPA_auth */
	scb->WPA_auth = bsscfg->WPA_auth;

	auth->scb_auth_in_prog = scb;

	/* auth->psk is pmk */
	if (auth->flags & AUTH_FLAG_PMK_PRESENT) {
		bcopy(auth->psk, wpa_info->pmk, PMK_LEN);
		wpa_info->pmk_len = PMK_LEN;
	}

	/* kick off authenticator */
	if (wpa_info->pmk_len == PMK_LEN) {
		auth_ies_len = wlc->pub->bcn_tmpl_len;
		if ((auth_ies = (uint8 *)MALLOC(wlc->osh, auth_ies_len)) == NULL) {
			WL_ERROR(("wl%d: %s: out of mem, %d bytes malloced\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
			return;
		}

		wlc_bcn_prb_body(wlc, FC_PROBE_RESP, SCB_BSSCFG(scb),
		                 auth_ies, (int *)&auth_ies_len, FALSE);
		stat = wlc_set_auth(auth, AUTH_WPAPSK,
		                    scb->wpaie, scb->wpaie_len,
		                    (uint8 *)auth_ies + sizeof(struct dot11_bcn_prb),
		                    auth_ies_len - sizeof(struct dot11_bcn_prb),
		                    auth->scb_auth_in_prog);
		if (!stat) {
			WL_ERROR(("wl%d: %s: 4-way handshake config problem\n",
			          wlc->pub->unit, __FUNCTION__));
		}

		auth_ies_len = wlc->pub->bcn_tmpl_len;
		MFREE(wlc->osh, (void *)auth_ies, auth_ies_len);
	}
	/* derive pmk from psk */
	else {
			wlc_auth_set_ssid(bsscfg->authenticator,
			                  (uchar *)&bsscfg->SSID, bsscfg->SSID_len, scb);
	}
}

static void
wlc_auth_gen_gtk(authenticator_t *auth, wlc_bsscfg_t *bsscfg)
{
	unsigned char data[256], prf_buff[PRF_RESULT_LEN];
	unsigned char prefix[] = "Group key expansion";
	int data_len = 0;

	/* Select a mcast cipher: only support wpa for now, otherwise change alg field */
	switch (WPA_MCAST_CIPHER(bsscfg->wsec, 0)) {
	case WPA_CIPHER_TKIP:
		WL_WSEC(("%s: TKIP\n", __FUNCTION__));
		auth->gtk_len = TKIP_TK_LEN;
		break;
	case WPA_CIPHER_AES_CCM:
		WL_WSEC(("%s: AES\n",  __FUNCTION__));
		auth->gtk_len = AES_TK_LEN;
		break;
	default:
		WL_WSEC(("%s: not supported multicast cipher\n", __FUNCTION__));
		return;
	}
	WL_WSEC(("%s: gtk_len %d\n", __FUNCTION__, auth->gtk_len));

	/* create the the data portion */
	bcopy((char*)&bsscfg->cur_etheraddr, (char*)&data[data_len], ETHER_ADDR_LEN);
	data_len += ETHER_ADDR_LEN;
	bcopy(auth->global_key_counter, auth->gnonce, EAPOL_WPA_KEY_NONCE_LEN);
	wlc_auth_incr_gkc(auth);
	bcopy((char*)&auth->gnonce, (char*)&data[data_len], EAPOL_WPA_KEY_NONCE_LEN);
	data_len += EAPOL_WPA_KEY_NONCE_LEN;

	/* generate the GTK */
	fPRF(auth->gmk, sizeof(auth->gmk), prefix, strlen((char *)prefix),
	    data, data_len, prf_buff, auth->gtk_len);
	memcpy(auth->gtk, prf_buff, auth->gtk_len);

	/* The driver clears the IV when it gets a new key, so
	 * clearing RSC should be consistent with that, right?
	 */
	memset(auth->gtk_rsc, 0, sizeof(auth->gtk_rsc));

	WL_WSEC(("%s: done\n", __FUNCTION__));
}

/* generate the initial global_key_counter */
static void
wlc_auth_initialize_gkc(authenticator_t *auth)
{
	wlc_info_t *wlc = auth->wlc;
	unsigned char buff[32], prf_buff[PRF_RESULT_LEN];
	unsigned char prefix[] = "Init Counter";

	wlc_getrand(wlc, &buff[0], 16);
	wlc_getrand(wlc, &buff[16], 16);

	/* Still not exactly right, but better. */
	fPRF(buff, sizeof(buff), prefix, strlen((char *)prefix),
	    (unsigned char *) &auth->bsscfg->cur_etheraddr, ETHER_ADDR_LEN,
	    prf_buff, KEY_COUNTER_LEN);
	memcpy(auth->global_key_counter, prf_buff, KEY_COUNTER_LEN);
	memcpy(auth->initial_gkc, auth->global_key_counter, KEY_COUNTER_LEN);
}

static void
wlc_auth_incr_gkc(authenticator_t *auth)
{
	wpa_incr_array(auth->global_key_counter, KEY_COUNTER_LEN);

	/* if key counter is now equal to the original one, reset it */
	if (!bcmp(auth->global_key_counter, auth->initial_gkc, KEY_COUNTER_LEN))
		wlc_auth_initialize_gmk(auth);
}

static void
wlc_auth_initialize_gmk(authenticator_t *auth)
{
	wlc_info_t *wlc = auth->wlc;
	unsigned char *gmk = (unsigned char *)auth->gmk;

	wlc_getrand(wlc, &gmk[0], 16);
	wlc_getrand(wlc, &gmk[16], 16);
}

#ifdef BCMDBG
static int
wlc_auth_dump(authenticator_t *auth, struct bcmstrbuf *b)
{
	return 0;
}
#endif /* BCMDBG */

static int
wlc_auth_prep_scb(authenticator_t *auth, struct scb *scb)
{
	wlc_auth_info_t *auth_info = auth->auth_info;
	struct auth_cubby *cubby_info = SCB_AUTH_INFO(auth_info, scb);

	if (!(cubby_info->cubby->wpa = bcm_mp_alloc(auth_info->wpa_mpool_h))) {
		WL_ERROR(("wl%d: %s: bcm_mp_alloc() of wpa failed\n",
		          auth->wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	bzero(cubby_info->cubby->wpa, sizeof(wpapsk_t));

	if (!(cubby_info->cubby->wpa_info = bcm_mp_alloc(auth_info->wpa_info_mpool_h))) {
		WL_ERROR(("wl%d: %s: bcm_mp_alloc() of wpa_info failed\n",
		          auth->wlc->pub->unit, __FUNCTION__));
		goto err;
	}
	bzero(cubby_info->cubby->wpa_info, sizeof(wpapsk_info_t));
	cubby_info->cubby->wpa_info->wlc = auth->wlc;

	if (!(cubby_info->cubby->wpa_info->passhash_timer =
	      wl_init_timer(auth->wlc->wl, wlc_auth_wpa_passhash_timer, auth,
	                    "passhash"))) {
		WL_ERROR(("wl%d: %s: passhash timer "
		          "failed\n", auth->wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	if (!(cubby_info->cubby->wpa_info->retry_timer =
	      wl_init_timer(auth->wlc->wl, wlc_auth_retry_timer, scb, "auth_retry"))) {
		WL_ERROR(("wl%d: %s: retry timer failed\n",
		          auth->wlc->pub->unit, __FUNCTION__));
		goto err;
	}

	return BCME_OK;

err:
	if (cubby_info->cubby->wpa_info) {
		if (cubby_info->cubby->wpa_info->passhash_timer) {
			wl_free_timer(auth->wlc->wl, cubby_info->cubby->wpa_info->passhash_timer);
			cubby_info->cubby->wpa_info->passhash_timer = NULL;
		}
		if (cubby_info->cubby->wpa_info->retry_timer) {
			wl_free_timer(auth->wlc->wl, cubby_info->cubby->wpa_info->retry_timer);
			cubby_info->cubby->wpa_info->retry_timer = NULL;
		}
		bcm_mp_free(auth_info->wpa_info_mpool_h, cubby_info->cubby->wpa_info);
		cubby_info->cubby->wpa_info = NULL;
	}

	if (cubby_info->cubby->wpa) {
		bcm_mp_free(auth_info->wpa_mpool_h, cubby_info->cubby->wpa);
		cubby_info->cubby->wpa = NULL;
	}
	return BCME_NOMEM;
}

static void
wlc_auth_endassoc(authenticator_t *auth)
{
	WL_TRACE(("Wl%d: %s: ENTER\n", auth->wlc->pub->unit, __FUNCTION__));

	if (!auth->scb_auth_in_prog)
		return;

	if (!(auth->flags & AUTH_FLAG_REKEY_ENAB))
		wlc_auth_cleanup_scb(auth->wlc, auth->scb_auth_in_prog);
	auth->scb_auth_in_prog = NULL;
}

static void
wlc_auth_cleanup_scb(wlc_info_t *wlc, struct scb *scb)
{
	struct auth_cubby *cubby_info;

	WL_TRACE(("wl%d: %s: Freeing SCB data at 0x%p\n", wlc->pub->unit, __FUNCTION__, scb));
	cubby_info = SCB_AUTH_INFO(wlc->authi, scb);

	if ((cubby_info == NULL) || (cubby_info->cubby == NULL))
		return;

	if (cubby_info->cubby->wpa) {
		wlc_wpapsk_free(wlc, cubby_info->cubby->wpa);
		bcm_mp_free(wlc->authi->wpa_mpool_h, cubby_info->cubby->wpa);
		cubby_info->cubby->wpa = NULL;
	}

	if (cubby_info->cubby->wpa_info) {
		if (cubby_info->cubby->wpa_info->passhash_timer) {
			wl_del_timer(wlc->wl, cubby_info->cubby->wpa_info->passhash_timer);
			wl_free_timer(wlc->wl, cubby_info->cubby->wpa_info->passhash_timer);
			cubby_info->cubby->wpa_info->passhash_timer = NULL;
		}
		if (cubby_info->cubby->wpa_info->retry_timer) {
			wl_del_timer(wlc->wl, cubby_info->cubby->wpa_info->retry_timer);
			wl_free_timer(wlc->wl, cubby_info->cubby->wpa_info->retry_timer);
			cubby_info->cubby->wpa_info->retry_timer = NULL;
		}
		bcm_mp_free(wlc->authi->wpa_info_mpool_h, cubby_info->cubby->wpa_info);
		cubby_info->cubby->wpa_info = NULL;
	}
}

void wlc_auth_tkip_micerr_handle(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg)
{
	if (wlc_keymgmt_tkip_cm_enabled(wlc->keymgmt, bsscfg)) {
		struct scb *scb;
		struct scb_iter scbiter;

		/* deauth all client */
		FOREACHSCB(wlc->scbstate, &scbiter, scb) {
			if (SCB_ASSOCIATED(scb) && SCB_BSSCFG(scb) == bsscfg) {
					WL_TRACE(("\n   DEAUTH: %02x:%02x:%02x:%02x:%02x:%02x!",
						scb->ea.octet[0], scb->ea.octet[1],
						scb->ea.octet[2],
						scb->ea.octet[3], scb->ea.octet[4],
						scb->ea.octet[5]));

					wlc_scb_set_auth(wlc, SCB_BSSCFG(scb), scb,
						FALSE, AUTHENTICATED, DOT11_RC_AUTH_INVAL);
				}
			}

		}
}
#endif /* BCMAUTH_PSK */
