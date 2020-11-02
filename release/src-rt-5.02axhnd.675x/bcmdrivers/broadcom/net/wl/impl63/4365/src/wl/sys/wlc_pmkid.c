/*
 * wlc_pmkid.c - PMKID module handling file
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
 * $Id: wlc_pmkid.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * Used for WPA(2) pre-authentication.  Preauth is effectively a background task where the
 * supplicant makes connections with different in-range WPA access points (even though there could
 * be an active and valid WPA connection with a particular AP) so that when a roam event takes
 * place, the WPA handshake is not necessary due to the fact that the pmk is already known.
 */

#include <wlc_cfg.h>

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <wlc_types.h>
#include <bcmwpa.h>
#if defined(BCMSUP_PSK) || defined(WLFBT) || defined(EXT_STA)
#include <bcmcrypto/prf.h>
#endif /* BCMSUP_PSK || WLFBT || EXT_STA */
#ifdef	BCMSUP_PSK
#include <bcmcrypto/passhash.h>
#include <bcmcrypto/sha1.h>
#include <wlc_sup.h>
#endif /* BCMSUP_PSK */

#include <d11.h>
#include <wlc_rate.h>
#include <siutils.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_assoc.h>
#include <proto/eapol.h>
#include <bcmwpa.h>
#include <wlc_wpa.h>
#include <wlc_pmkid.h>
#include <bcmcrypto/prf.h>
#include <bcmcrypto/rc4.h>

#ifdef WL_OKC
#include <wlc_okc.h>
#endif // endif

static int wlc_pmkid_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif);
static int wlc_pmkid_bss_init(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_pmkid_bss_deinit(void *ctx, wlc_bsscfg_t *cfg);
static void wlc_pmkid_event(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg);

enum {
	IOV_PMKID_INFO,
	IOV_LAST
};

static const bcm_iovar_t pmkid_iovars[] = {
	{"pmkid_info", IOV_PMKID_INFO,
	(0), IOVT_BUFFER, sizeof(uint)
	},
	{NULL, 0, 0, 0, 0}
};

struct wlc_pmkid_info {
	/* references to driver `common' things */
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_pub_t *pub;			/* pointer to wlc public portion */
	osl_t *osh;			/* PKT* stuff wants this */
	int	cfgh;
};

typedef struct bss_pmkid_info {
	/* PMKID caching */
	pmkid_cand_t	pmkid_cand[MAXPMKID];	/* PMKID candidate list */
	uint		npmkid_cand;	/* num PMKID candidates */
	pmkid_t		pmkid[MAXPMKID];	/* PMKID cache */
	uint		npmkid;		/* num cached PMKIDs */
} bss_pmkid_info_t;

#define PMKID_BSSCFG_CUBBY_LOC(pmkid, cfg) ((bss_pmkid_info_t **)BSSCFG_CUBBY(cfg, (pmkid)->cfgh))
#define PMKID_BSSCFG_CUBBY(pmkid, cfg) (*PMKID_BSSCFG_CUBBY_LOC(pmkid, cfg))

#define UNIT(ptr)	((ptr)->pub->unit)

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

wlc_pmkid_info_t *
BCMATTACHFN(wlc_pmkid_attach)(wlc_info_t *wlc)
{
	wlc_pmkid_info_t *pmkid_info;

	WL_TRACE(("wl%d: wlc_pmkid_attach\n", wlc->pub->unit));

	if (!(pmkid_info = (wlc_pmkid_info_t *)MALLOCZ(wlc->osh, sizeof(wlc_pmkid_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	pmkid_info->wlc = wlc;
	pmkid_info->pub = wlc->pub;
	pmkid_info->osh = wlc->osh;

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((pmkid_info->cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_pmkid_info_t *),
	                wlc_pmkid_bss_init, wlc_pmkid_bss_deinit, NULL,
	                (void *)pmkid_info)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
		          UNIT(pmkid_info), __FUNCTION__));
		goto err;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, pmkid_iovars, "pmkid", pmkid_info, wlc_pmkid_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: pmkid wlc_module_register() failed\n", UNIT(pmkid_info)));
		goto err;
	}
	return pmkid_info;
err:
	wlc_pmkid_detach(pmkid_info);
	return NULL;
}

/* Toss pmkid context */
void
BCMATTACHFN(wlc_pmkid_detach)(wlc_pmkid_info_t *pmkid_info)
{
	if (!pmkid_info)
		return;

	WL_TRACE(("wl%d: wlc_pmkid_detach\n", UNIT(pmkid_info)));
	wlc_module_unregister(pmkid_info->pub, "pmkid", pmkid_info);
	MFREE(pmkid_info->osh, pmkid_info, sizeof(wlc_pmkid_info_t));
}

static int
wlc_pmkid_bss_init(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_pmkid_info_t *pmkid_info = (wlc_pmkid_info_t *)ctx;
	bss_pmkid_info_t **pbss_pmkid = PMKID_BSSCFG_CUBBY_LOC(pmkid_info, cfg);
	bss_pmkid_info_t *bss_pmkid;
	int err = BCME_OK;

	if (!(bss_pmkid = (bss_pmkid_info_t *)MALLOCZ(pmkid_info->osh, sizeof(bss_pmkid_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n", UNIT(pmkid_info),
			__FUNCTION__, MALLOCED(pmkid_info->osh)));
		err = BCME_NOMEM;
		*pbss_pmkid = NULL;
		return err;
	}

	*pbss_pmkid = bss_pmkid;
	return BCME_OK;
}

static void
wlc_pmkid_bss_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_pmkid_info_t *pmkid_info = (wlc_pmkid_info_t *)ctx;
	bss_pmkid_info_t **pbss_pmkid = PMKID_BSSCFG_CUBBY_LOC(pmkid_info, cfg);
	bss_pmkid_info_t *bss_pmkid;
	if (pbss_pmkid) {
		bss_pmkid = *pbss_pmkid;
		if (bss_pmkid) {
			MFREE(pmkid_info->osh, bss_pmkid, sizeof(bss_pmkid_info_t));
			*pbss_pmkid = NULL;
		}
	}
}

static int
wlc_pmkid_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_pmkid_info_t *pmkid_info = (wlc_pmkid_info_t *)handle;
	wlc_info_t *wlc = pmkid_info->wlc;
	bss_pmkid_info_t *bss_pmkid;
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
	int32 *ret_int_ptr;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bss_pmkid = PMKID_BSSCFG_CUBBY(pmkid_info, bsscfg);
	ASSERT(bss_pmkid != NULL);

	/* Do the actual parameter implementation */
	switch (actionid) {
		case IOV_GVAL(IOV_PMKID_INFO):
			if (alen < (int)(sizeof(int_val) + bss_pmkid->npmkid*sizeof(pmkid_t)))
				return BCME_BUFTOOSHORT;
			*ret_int_ptr = bss_pmkid->npmkid;
			arg = (char*)arg + sizeof(uint32);
			bcopy(bss_pmkid->pmkid, arg, bss_pmkid->npmkid*sizeof(pmkid_t));
			break;

		case IOV_SVAL(IOV_PMKID_INFO): {
			pmkid_list_t *pmkid = (pmkid_list_t *)arg;
			uint i, npmkid;

			bcopy(&pmkid->npmkid, &npmkid, sizeof(uint));

			if (npmkid > MAXPMKID)
				return BCME_BADLEN;

			/* Full cache is always plumbed, so clear what's already there */
			bzero(bss_pmkid->pmkid, sizeof(bss_pmkid->pmkid));

			/* Fill cache */
			for (i = 0; i < npmkid; i++) {
				bcopy(&pmkid->pmkid[i], &bss_pmkid->pmkid[i], sizeof(pmkid_t));
#ifdef BCMDBG
				if (WL_WSEC_ON()) {
					char eabuf[ETHER_ADDR_STR_LEN];
					uint j;

					WL_WSEC(("wl%d: PMKID[%d]: %s = ", UNIT(pmkid_info), i,
						bcm_ether_ntoa(&pmkid->pmkid[i].BSSID,
						eabuf)));
					for (j = 0; j < WPA2_PMKID_LEN; j++)
						WL_WSEC(("%02x ", pmkid->pmkid[i].PMKID[j]));
					WL_WSEC(("\n"));
				}
#endif /* BCMDBG */
			}
			bss_pmkid->npmkid = npmkid;
			break;
		}

		default:
			err = BCME_UNSUPPORTED;
			break;
	}
	return err;
}

/* Gets called when PMKID candidates are found. If idsup is active,
 * candidate bssid is compared with in-driver PMKIDs stored and
 * for matching of BSSID, PMKID would be used
 * in RSN IE of assoc request. If idsup is not active an event
 * is triggered to host to indicate about PMKID candidates
 */

void
wlc_pmkid_cache_req(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg)
{
#ifdef BCMSUP_PSK
	bss_pmkid_info_t *bss_pmkid = PMKID_BSSCFG_CUBBY(pmkid_info, cfg);
	uint8			temp_pmkid[WPA2_PMKID_LEN];
	uint i, k;

	/*
	 * for each element in driver's candidate list
	 * - find matching entries in supplicant's PMKID store
	 * - install in driver's PMKID cache
	 */
	if (SUP_ENAB(pmkid_info->pub) &&
		BSS_SUP_ENAB_WPA(pmkid_info->wlc->idsup, cfg)) {
		if (!bss_pmkid->npmkid_cand)
			return;
		k = 0;
		for (i = 0; i < bss_pmkid->npmkid_cand; i++) {
			pmkid_cand_t *pmkid_cand = &bss_pmkid->pmkid_cand[i];

			if (wlc_sup_find_pmkid(pmkid_info->wlc->idsup, cfg,
				&pmkid_cand->BSSID, temp_pmkid)) {
				bcopy(&pmkid_cand->BSSID,
					&bss_pmkid->pmkid[k].BSSID,
					ETHER_ADDR_LEN);
				bcopy(temp_pmkid,
					&bss_pmkid->pmkid[k].PMKID,
					WPA2_PMKID_LEN);
				k++;
			}
		}
		bss_pmkid->npmkid = k;
	}
	else
#endif /* BCMSUP_PSK */
		wlc_pmkid_event(pmkid_info, cfg);
}

/*
* pmkid context is cleared to start afresh the PMKID handling process
*/
void
wlc_pmkid_clear_store(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg)
{
	bss_pmkid_info_t *bss_pmkid = PMKID_BSSCFG_CUBBY(pmkid_info, cfg);
	bss_pmkid->npmkid_cand = bss_pmkid->npmkid = 0;
#ifdef BCMSUP_PSK
	if (pmkid_info->wlc->idsup)
		wlc_sup_clear_pmkid_store(pmkid_info->wlc->idsup, cfg);
#endif /* BCMSUP_PSK */
}

/* Used to indicate PMKID candidate event to the host
*/
static void
wlc_pmkid_event(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg)
{
	bss_pmkid_info_t *bss_pmkid = PMKID_BSSCFG_CUBBY(pmkid_info, cfg);
	wlc_info_t *wlc = pmkid_info->wlc;
	wlc_event_t *e;
	uint32 i, pmkid_list_len = bss_pmkid->npmkid_cand * sizeof(pmkid_cand_t) + sizeof(uint32);
	struct {
		pmkid_cand_list_t pmkids;
		pmkid_cand_t foo[MAXPMKID-1];
	} pmkid_list;
	pmkid_cand_t *cand = (pmkid_cand_t *)&pmkid_list.pmkids.pmkid_cand[0];

#ifdef WLLMAC
	/* LMAC doesn't use these event */
	if (LMAC_ENAB(wlc->pub))
		return;
#endif // endif

	bzero((char*)&pmkid_list, pmkid_list_len);

	e = wlc_event_alloc(wlc->eventq);
	if (e == NULL) {
		WL_ERROR(("wl%d: %s wlc_event_alloc failed\n", UNIT(pmkid_info), __FUNCTION__));
		return;
	}

	e->event.event_type = WLC_E_PMKID_CACHE;

	/* creat a copy of the pmkid cand list, store the list elements in network order */
	pmkid_list.pmkids.npmkid_cand = hton32(bss_pmkid->npmkid_cand);

	for (i = 0; i < bss_pmkid->npmkid_cand; i++) {
		bcopy(&bss_pmkid->pmkid_cand[i].BSSID.octet[0], &cand->BSSID.octet[0],
			ETHER_ADDR_LEN);
		cand->preauth = bss_pmkid->pmkid_cand[i].preauth;
		cand++;
	}

	/* point the event data at it */
	e->event.datalen = pmkid_list_len;
	e->data = MALLOC(pmkid_info->osh, e->event.datalen);
	if (e->data == NULL) {
		wlc_event_free(wlc->eventq, e);
		WL_ERROR(("wl%d: %s MALLOC failed\n", UNIT(pmkid_info), __FUNCTION__));
		return;
	}

	bcopy(&pmkid_list, e->data, e->event.datalen);

	wlc_event_if(wlc, cfg, e, NULL);
	wlc_process_event(wlc, e);
}

/* List of possible PMKID candidates would be prepared at the end of assocscan.
*
*/
void
wlc_pmkid_prep_list(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg,
struct ether_addr *bssid, uint8 wpa2_flags)
{
	bss_pmkid_info_t *bss_pmkid = PMKID_BSSCFG_CUBBY(pmkid_info, cfg);
	uint j, mark;

	mark = MAXPMKID;
	/* already in candidate list? */
	for (j = 0; j < bss_pmkid->npmkid_cand; j++) {
		if (bcmp((void *)bssid, (void *)&bss_pmkid->pmkid_cand[j].BSSID,
		         ETHER_ADDR_LEN) == 0) {
			WL_WSEC(("already in candidate list\n"));
			mark = j;
			break;
		}
	}

	/* already in candidate list at the end, move on */
	if (mark != MAXPMKID && mark == (bss_pmkid->npmkid_cand - 1))
		return;

	/* not already in candidate list, add */
	if (mark == MAXPMKID) {
		WL_WSEC(("add to candidate list\n"));

		if (bss_pmkid->npmkid_cand == (MAXPMKID - 1)) {
			WL_WSEC(("wl%d: wlc_pmkid_build_cand_list(): no room..."
				 "replace oldest\n", pmkid_info->wlc->pub->unit));
			mark = 0;
		} else
			mark = bss_pmkid->npmkid_cand++;
	}

	/* bubble each item up, overwriting item at mark */
	for (j = mark + 1; j < bss_pmkid->npmkid_cand; j++)
		bss_pmkid->pmkid_cand[j - 1] = bss_pmkid->pmkid_cand[j];

	/* new or updated entry gets added at or moved to end of list */
	bcopy(bssid->octet, bss_pmkid->pmkid_cand[bss_pmkid->npmkid_cand - 1].BSSID.octet,
	      ETHER_ADDR_LEN);
	bss_pmkid->pmkid_cand[bss_pmkid->npmkid_cand - 1].preauth =
		(((wpa2_flags & RSN_FLAGS_PREAUTH) != 0) ? 1 : 0);
}

/* Gets called when RSN IE of assoc request has to be populated with PMKID
* since the driver has PMKID stored. Pointer to PMKID location of RSN IE of
* assoc request is passed and this function puts PMKIDs if present in the frame
* and returns the length of pbody pointer to be incremented to move to next
* element
*/
uint16
wlc_pmkid_putpmkid(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg,
	struct ether_addr *bssid, bcm_tlv_t *wpa2_ie, uint8 *fbt_pmkid, uint32 WPA_auth)
{
	bss_pmkid_info_t *bss_pmkid = PMKID_BSSCFG_CUBBY(pmkid_info, cfg);
	uint8 added_len = 0;
	uint i;
	uint8 *pmkid[MAXPMKID];
	uint npmkid = 0;
	if (fbt_pmkid) {
		npmkid = 1;
		pmkid[0] = fbt_pmkid;
	} else if (WPA_auth & WPA2_AUTH_UNSPECIFIED) {
		for (i = 0; i < bss_pmkid->npmkid; i++) {
			if (bcmp((void *)bssid,
				(void *)&bss_pmkid->pmkid[i].BSSID, ETHER_ADDR_LEN) == 0) {
#ifdef BCMDBG
				if (WL_WSEC_ON()) {
					char eabuf[ETHER_ADDR_STR_LEN];
					uint j;

					WL_WSEC(("wl%d: PMKID cache hit: %s = ",
						UNIT(pmkid_info),
						bcm_ether_ntoa(bssid,
						eabuf)));
					for (j = 0; j < WPA2_PMKID_LEN; j++)
						WL_WSEC(("%02x ",
							bss_pmkid->pmkid[i].PMKID[j]));
					WL_WSEC(("\n"));
				}
#endif /* BCMDBG */
				pmkid[npmkid++] = bss_pmkid->pmkid[i].PMKID;
			}
		}
#ifdef WL_OKC
		/* if we don't have any pmkid for target bss, we have to create pmkid based on
		*  WLC_OKC_INFO(wlc).pmk which is the pmk created from external supplicant.
		*/
			if (cfg->associated && (npmkid == 0)&&
				OKC_ENAB(cfg->wlc->pub)&&
				WLC_OKC_INFO(cfg->wlc)->pmk_len) {
				int index = 0;
				wlc_calc_pmkid_for_okc(cfg->wlc, cfg, bssid, &index);
				if (index != -1) {
					pmkid[npmkid++] = bss_pmkid->pmkid[index].PMKID;
				}

			}
#endif /* WL_OKC */
	}

	if (npmkid) {
		wpa_pmkid_list_t *pmkid_list;

		pmkid_list =
			(wpa_pmkid_list_t *)&wpa2_ie->data[wpa2_ie->len];

		/* fill in PMKID count */
		pmkid_list->count.low = (uint8)npmkid;
		pmkid_list->count.high = (uint8)(npmkid >> 8);
		wpa2_ie->len += 2;
		added_len += 2;

		/* fill in PMKID list */
		for (i = 0; i < npmkid; i++) {
#ifdef BCMDBG
			if (WL_WSEC_ON()) {
				uint j;

				WL_WSEC(("wl%d: try PMKID: ", UNIT(pmkid_info)));
				for (j = 0; j < WPA2_PMKID_LEN; j++)
					WL_WSEC(("%02x ", pmkid[i][j]));
				WL_WSEC(("\n"));
			}
#endif /* BCMDBG */

			bcopy(pmkid[i], &pmkid_list->list[i], WPA2_PMKID_LEN);
			wpa2_ie->len += WPA2_PMKID_LEN;
			added_len += WPA2_PMKID_LEN;
		}
	}
	return added_len;
}
#ifdef WL_OKC
void
wpa_calc_pmkid_for_okc(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg, struct ether_addr *auth_ea,
	struct ether_addr *sta_ea, uint8 *pmk, uint pmk_len, uint8 *data, uint8 *digest, int *index)
{
	/* PMKID = HMAC-SHA1-128(PMK, "PMK Name" | AA | SPA) */
	int i = 0;
#if defined(BCMDBG) || defined(WLMSG_WSEC)
	char eabuf1[ETHER_ADDR_STR_LEN];
	char eabuf2[ETHER_ADDR_STR_LEN];
#endif // endif
	bss_pmkid_info_t *bss_pmkid = PMKID_BSSCFG_CUBBY(pmkid_info, cfg);

	pmkid_t *pmkid = NULL;
	/* Overwrite existing PMKID for this BSSID */
	for (i = 0; i < bss_pmkid->npmkid; i++) {
		pmkid = &bss_pmkid->pmkid[i];
		if (!bcmp(auth_ea, (char *)&pmkid->BSSID, ETHER_ADDR_LEN))
			break;
	}

	/* Add new PMKID to store if no existing PMKID was found */
	if (i == bss_pmkid->npmkid) {
		if (bss_pmkid->npmkid == MAXPMKID) {
			WL_WSEC(("%s: can't calculate PMKID - no room in"
						" the inn\n", __FUNCTION__));
			pmkid = &bss_pmkid->pmkid[bss_pmkid->npmkid - 1];
			*index = bss_pmkid->npmkid - 1;
		} else {
			*index = bss_pmkid->npmkid;
			pmkid = &bss_pmkid->pmkid[bss_pmkid->npmkid++];
		}
	} else
		*index = i;
#if defined(BCMINTSUP) && defined(BCMSUP_PSK)
	if (!SUP_ENAB(pmkid_info->wlc->pub) ||
		!BSS_SUP_ENAB_WPA(pmkid_info->wlc->idsup, cfg) ||
		wlc_sup_set_pmkid(pmkid_info->wlc->idsup, cfg,
		pmk, (ushort)pmk_len, auth_ea, pmkid->PMKID)) {
#endif // endif
	if ((cfg->WPA_auth & (WPA2_AUTH_1X_SHA256 | WPA2_AUTH_PSK_SHA256)) &&
	    (cfg->current_bss->wpa2.flags & RSN_FLAGS_SHA256))
		kdf_calc_pmkid(auth_ea, sta_ea, pmk, pmk_len, pmkid->PMKID, data, digest);
	else
		wpa_calc_pmkid(auth_ea, sta_ea, pmk, pmk_len, pmkid->PMKID, data, digest);
#if defined(BCMINTSUP) && defined(BCMSUP_PSK)
	}
#endif // endif
	bcopy((uint8 *)auth_ea, (uint8 *)&pmkid->BSSID, ETHER_ADDR_LEN);
	WL_WSEC(("auth_ea address : %s , sta_ea address : %s", bcm_ether_ntoa(auth_ea, eabuf1),
		bcm_ether_ntoa(sta_ea, eabuf2)));
#ifdef BCMDBG
	WL_WSEC(("PMK: "));
	for (i = 0; i < 32; i++)
		WL_WSEC(("%02X ", pmk[i]));
	WL_WSEC(("\n"));
#endif // endif
	WL_WSEC(("PMKID: "));
	for (i = 0; i < WPA2_PMKID_LEN; i++)
		WL_WSEC(("%02x ", pmkid->PMKID[i]));
	WL_WSEC(("\n"));
}
#endif /* WL_OKC */
