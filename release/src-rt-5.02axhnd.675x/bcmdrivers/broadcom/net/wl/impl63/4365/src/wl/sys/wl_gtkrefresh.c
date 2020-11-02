/*
 * similar supplicant functions for GTK refresh in EXT_STA
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
 * $Id: wl_gtkrefresh.c 782389 2019-12-18 06:56:56Z $
 *
 */
#ifdef EXT_STA
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmwifi_channels.h>
#include <siutils.h>
#include <bcmendian.h>
#include <proto/802.1d.h>
#include <proto/802.11.h>
#include <proto/802.11e.h>
#ifdef MCAST_REGEN
#include <proto/bcmip.h>
#endif // endif
#include <proto/wpa.h>
#include <proto/vlan.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <pcicfg.h>
#include <bcmsrom.h>
#include <wlioctl.h>
#include <epivers.h>
#include <proto/eapol.h>
#include <bcmwpa.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <hndpmu.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc_channel.h>
#include <wlc.h>
#include <wlc_hw.h>
#include <wlc_hw_priv.h>
#include <wlc_bmac.h>
#include <wlc_apps.h>
#include <wlc_scb.h>
#include <wlc_security.h>
#ifdef STA
#include <wlc_wpa.h>
#endif /* STA */
#include <wlc_sup.h>
#include <wlc_pmkid.h>
#include <bcmcrypto/prf.h>
#include <proto/eap.h>

enum {
	IOV_GTK_KEYINFO,
	IOV_LAST
};

static const bcm_iovar_t gtk_iovars[] = {
	{"gtk_key_info", IOV_GTK_KEYINFO,
	(0), IOVT_BUFFER, sizeof(gtk_keyinfo_t)
	},
	{NULL, 0, 0, 0, 0}
};

typedef struct {
	/* copies of driver `common' things */
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_pub_t *pub;			/* pointer to wlc public portion */
	void *wl;			/* per-port handle */
	osl_t *osh;			/* PKT* stuff wants this */
	uint16 bss_sup_priv_offset;	/* offset of priv cubby in bsscfg */
	bool	sup_m3sec_ok;		/* to selectively allow incorrect bit in M3 */
	int sup_wpa2_eapver;	/* for choosing eapol version in M2 */
	bool	updn;			/* bsscfg up/down callbackk registered */
	bcm_notif_h			sup_up_down_notif_hdl; /* init notifier handle. */
	bcm_notif_h			pad; /* deinit notifier handle. */
} wlc_sup_priv_t;

/* skeletion structure since supplicant_t hangs off the bsscfg, rather than wlc */
struct wlc_sup_info {
	wlc_sup_pub_t mod_pub;
	wlc_sup_priv_t mod_priv;
};

#define WLC_SUP_PRIV_INFO(sup_info)		((wlc_sup_priv_t *) \
	((uint8 *)sup_info + OFFSETOF(wlc_sup_info_t, mod_priv)))

/* Supplicant top-level structure hanging off bsscfg */
struct supplicant {
	wlc_info_t *wlc;		/* pointer to main wlc structure */
	wlc_pub_t *pub;			/* pointer to wlc public portion */
	void *wl;			/* per-port handle */
	osl_t *osh;			/* PKT* stuff wants this */
	wlc_bsscfg_t *cfg;		/* pointer to sup's bsscfg */
	wlc_sup_info_t *m_handle;	/* module handle */

	struct ether_addr peer_ea;      /* peer's ea */

	wpapsk_t *wpa;			/* volatile, initialized in set_sup */
	wpapsk_info_t *wpa_info;		/* persistent wpa related info */
	unsigned char ap_eapver;	/* eapol version from ap */
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

/* Values for type parameter of wlc_set_sup() */
#define SUP_UNUSED	0 /* Supplicant unused */
#define SUP_WPAPSK	2 /* Used for WPA-PSK */

#define SUP_CHECK_EAPOL(body) (body->type == EAPOL_WPA_KEY || body->type == EAPOL_WPA2_KEY)

/* Simplify maintenance of references to driver `common' items. */
#define UNIT(ptr)	((ptr)->pub->unit)
#define CUR_EA(ptr)	(((supplicant_t *)ptr)->cfg->cur_etheraddr)
#define PEER_EA(ptr)	(((supplicant_t *)ptr)->peer_ea)
#define BSS_EA(ptr)	(((supplicant_t *)ptr)->cfg->BSSID)
#define OSH(ptr)	((ptr)->osh)

static int wlc_gtk_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif);
wlc_sup_info_t * wlc_gtk_attach(wlc_info_t *wlc);
void wlc_gtk_detach(wlc_sup_info_t *sup_info);
void wlc_gtk_deinit(void *ctx, wlc_bsscfg_t *cfg);
int wlc_gtk_init(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg);

wlc_sup_info_t *
wlc_gtk_attach(wlc_info_t *wlc)
{
	wlc_sup_info_t *sup_info;
	wlc_sup_priv_t	*sup_priv;

	if (!(sup_info = (wlc_sup_info_t *)MALLOC(wlc->osh, sizeof(wlc_sup_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return NULL;
	}

	bzero((char *)sup_info, sizeof(wlc_sup_info_t));
	sup_priv = WLC_SUP_PRIV_INFO(sup_info);
	sup_priv->wlc = wlc;
	sup_priv->pub = wlc->pub;
	sup_priv->wl = wlc->wl;
	sup_priv->osh = wlc->osh;
	sup_priv->bss_sup_priv_offset = OFFSETOF(bss_sup_info_t, bss_priv);

	/* reserve cubby in the bsscfg container for per-bsscfg private data */
	if ((sup_info->mod_pub.cfgh = wlc_bsscfg_cubby_reserve(wlc, sizeof(bss_sup_info_t *),
		NULL, wlc_gtk_deinit, NULL, (void *)sup_info)) < 0) {
		WL_ERROR(("wl%d: %s: wlc_bsscfg_cubby_reserve() failed\n",
			UNIT(sup_priv), __FUNCTION__));
		goto err;
	}

	/* register module */
	if (wlc_module_register(wlc->pub, gtk_iovars, "gtkref", sup_info, wlc_gtk_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: wlc_gtk_attach: wlc_module_register() failed\n", UNIT(sup_priv)));
		goto err;
	}

	return sup_info;

err:
	wlc_gtk_detach(sup_info);
	return NULL;
}

void
wlc_gtk_detach(wlc_sup_info_t *sup_info)
{
	wlc_sup_priv_t *sup_priv;
	if (!sup_info)
		return;

	sup_priv = WLC_SUP_PRIV_INFO(sup_info);

	WL_TRACE(("wl%d: %s\n", UNIT(sup_priv), __FUNCTION__));

	MFREE(sup_priv->osh, sup_info, sizeof(wlc_sup_info_t));
}

static int
wlc_gtk_doiovar(void *handle, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint plen, void *arg, int alen, int vsize, struct wlc_if *wlcif)
{
	wlc_sup_info_t *sup_info = (wlc_sup_info_t *)handle;
	wlc_sup_priv_t *sup_priv = WLC_SUP_PRIV_INFO(sup_info);
	supplicant_t *sup;
	wlc_info_t *wlc = sup_priv->wlc;
	wlc_bsscfg_t *bsscfg;
	int err = 0;
	int32 int_val = 0;
	int32 *ret_int_ptr;
	bool bool_val;
	gtk_keyinfo_t keyinfo;

	/* update bsscfg w/provided interface context */
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);
	if (BSSCFG_AP(bsscfg)) {
		err = BCME_NOTSTA;
		goto exit;
	}
	sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, bsscfg);
	/* convenience int and bool vals for first 8 bytes of buffer */
	if (plen >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;

	bool_val = (int_val != 0) ? TRUE : FALSE;
	BCM_REFERENCE(bool_val);
	BCM_REFERENCE(ret_int_ptr);

	switch (actionid) {
		case IOV_SVAL(IOV_GTK_KEYINFO):
			if (sup_info && bsscfg && (wlc_gtk_init(sup_info, bsscfg) != BCME_OK)) {
				err = BCME_ERROR;
				break;
			}
			bcopy(arg, &keyinfo, sizeof(gtk_keyinfo_t));
			bcopy(keyinfo.KCK, sup->wpa->eapol_mic_key, WPA_MIC_KEY_LEN);
			bcopy(keyinfo.KEK, sup->wpa->eapol_encr_key, WPA_ENCR_KEY_LEN);
			bcopy(keyinfo.ReplayCounter, sup->wpa->replay, EAPOL_KEY_REPLAY_LEN);
			bcopy(keyinfo.ReplayCounter, sup->wpa->last_replay, EAPOL_KEY_REPLAY_LEN);
			err = BCME_OK;
			break;
		case IOV_GVAL(IOV_GTK_KEYINFO):
			bzero(&keyinfo, sizeof(gtk_keyinfo_t));
			bcopy(sup->wpa->last_replay, keyinfo.ReplayCounter, EAPOL_KEY_REPLAY_LEN);
			bcopy(&keyinfo, arg, sizeof(gtk_keyinfo_t));
			wlc_gtk_deinit(sup_info, bsscfg);
			err = BCME_OK;
			break;
		default:
			break;
	}

exit:
	return err;
}

int
wlc_gtk_init(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg)
{
	wlc_sup_priv_t	*sup_priv = WLC_SUP_PRIV_INFO(sup_info);

	wlc_info_t *wlc = sup_priv->wlc;

	bss_sup_info_t **psup_bss = SUP_BSSCFG_CUBBY_LOC(sup_info, cfg);
	bss_sup_info_t *sup_bss = NULL;
	supplicant_t *sup = NULL;
	sup_bss_pub_t *supbsspub;

	WL_TRACE(("wl%d: %s\n", UNIT(sup_priv), __FUNCTION__));

	if (cfg->_ap)
		return BCME_NOTSTA;
	if (!(sup_bss = (bss_sup_info_t *)MALLOC(sup_priv->osh, sizeof(bss_sup_info_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
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

	supbsspub->sup_type = SUP_WPAPSK;
	supbsspub->sup_enable_wpa = TRUE;

	if (!(sup->wpa = MALLOC(sup_priv->osh, sizeof(wpapsk_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto err;
	}
	bzero(sup->wpa, sizeof(wpapsk_t));
	return BCME_OK;
err:
	if (sup_bss)
		wlc_gtk_deinit(sup_info, cfg);
	return BCME_ERROR;
}

/* Toss context */
void
wlc_gtk_deinit(void *ctx, wlc_bsscfg_t *cfg)
{
	wlc_sup_info_t *sup_info = (wlc_sup_info_t *)ctx;
	bss_sup_info_t **psup_bss;
	bss_sup_info_t *sup_bss;
	supplicant_t *sup = NULL;

	if (cfg == NULL || sup_info == NULL)
		return;

	psup_bss = SUP_BSSCFG_CUBBY_LOC(sup_info, cfg);
	sup_bss = *psup_bss;

	if (sup_bss == NULL)
		return;

	WL_TRACE(("wl%d: %s\n", UNIT(sup), __FUNCTION__));

	sup = &sup_bss->bss_priv;
	if (sup->wpa) {
		MFREE(sup->osh, sup->wpa, sizeof(wpapsk_t));
	}

	MFREE(sup->osh, sup_bss, sizeof(bss_sup_info_t));
	*psup_bss = NULL;
}

/* plumb the group key */
uint32
wlc_wpa_plumb_gtk(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint8 *gtk, uint32 gtk_len,
	uint32 key_index, uint32 cipher, uint8 *rsc, bool primary_key)
{
	wl_wsec_key_t *key;
	uint32 ret_index;

	WL_WSEC(("%s\n", __FUNCTION__));

	if (!(key = MALLOC(wlc->osh, sizeof(wl_wsec_key_t)))) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		return (uint32)(-1);
	}

	bzero(key, sizeof(wl_wsec_key_t));
	key->index = key_index;
	/* NB: wlc_insert_key() will re-infer key->algo from key_len */
	key->algo = cipher;
	key->len = gtk_len;
	bcopy(gtk, key->data, key->len);

	if (primary_key)
		key->flags |= WL_PRIMARY_KEY;

	/* Extract the Key RSC in an Endian independent format */
	key->iv_initialized = 1;
	if (rsc != NULL) {
		/* Extract the Key RSC in an Endian independent format */
		key->rxiv.lo = (((rsc[1] << 8) & 0xFF00) |
			(rsc[0] & 0x00FF));
		key->rxiv.hi = (((rsc[5] << 24) & 0xFF000000) |
			((rsc[4] << 16) & 0x00FF0000) |
			((rsc[3] << 8) & 0x0000FF00) |
			((rsc[2]) & 0x000000FF));
	} else {
		key->rxiv.lo = bsscfg->wpa_none_txiv.lo;
		key->rxiv.hi = bsscfg->wpa_none_txiv.hi;
	}

	WL_WSEC(("wl%d: %s: Group Key is stored as Low :0x%x,"
	         " High: 0x%x\n", wlc->pub->unit, __FUNCTION__, key->rxiv.lo, key->rxiv.hi));

	wlc_ioctl(wlc, WLC_SET_KEY, key, sizeof(wl_wsec_key_t), bsscfg->wlcif);
	if (key->index != key_index) {
		WL_WSEC(("%s(): key_index changed from %d to %d\n",
			__FUNCTION__, key_index, key->index));
	}

	ret_index = key->index;
	MFREE(wlc->osh, key, sizeof(wl_wsec_key_t));
	return ret_index;
}

static void
wlc_wpa_send_gtk_status(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg, uint reason)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	wlc_bss_mac_event(sup->wlc, sup->cfg, WLC_E_PSK_SUP,
	       NULL, WLC_SUP_KEYXCHANGE_PREP_G2, reason, 0, 0, 0);
}

/* Get an EAPOL packet and fill in some of the common fields */
static void *
wlc_gtk_eapol_pktget(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct ether_addr *da,
	uint len)
{
	osl_t *osh = wlc->osh;
	void *p;
	eapol_header_t *eapol_hdr;

	if ((p = PKTGET(osh, len + TXOFF, TRUE)) == NULL) {
		WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		WLCNTINCR(wlc->pub->_cnt->txnobuf);
		return (NULL);
	}
	ASSERT(ISALIGNED(PKTDATA(osh, p), sizeof(uint32)));

	/* reserve TXOFF bytes of headroom */
	PKTPULL(osh, p, TXOFF);
	PKTSETLEN(osh, p, len);

	/* fill in common header fields */
	eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
	bcopy(da, &eapol_hdr->eth.ether_dhost, ETHER_ADDR_LEN);
	bcopy(&bsscfg->cur_etheraddr, &eapol_hdr->eth.ether_shost, ETHER_ADDR_LEN);
	eapol_hdr->eth.ether_type = hton16(ETHER_TYPE_802_1X);
	eapol_hdr->version = WPA_EAPOL_VERSION;
	return p;
}

static void *
wlc_wpa_gtk_prepG2(supplicant_t *sup, uint16 flags, wpa_msg_t msg)
{
	uint16 len, key_desc;
	void *p = NULL;
	eapol_header_t *eapol_hdr = NULL;
	eapol_wpa_key_header_t *wpa_key = NULL;
	uchar mic[PRF_OUTBUF_LEN];
	osl_t *osh = OSH(sup);
	wpapsk_t *wpa = sup->wpa;

	len = EAPOL_HEADER_LEN + EAPOL_WPA_KEY_LEN;
	if ((p = wlc_gtk_eapol_pktget(sup->wlc, sup->cfg, &PEER_EA(sup),
		len)) == NULL)
		return NULL;

	eapol_hdr = (eapol_header_t *) PKTDATA(osh, p);
	eapol_hdr->length = hton16(EAPOL_WPA_KEY_LEN);
	wpa_key = (eapol_wpa_key_header_t *) eapol_hdr->body;
	bzero(wpa_key, EAPOL_WPA_KEY_LEN);
	hton16_ua_store((flags | GMSG2_REQUIRED), (uint8 *)&wpa_key->key_info);
	hton16_ua_store(wpa->gtk_len, (uint8 *)&wpa_key->key_len);

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
		WL_WSEC(("wl%d: %s: MIC generation failed\n",
		         UNIT(sup), __FUNCTION__));
		return FALSE;
	}
	bcopy(mic, wpa_key->mic, EAPOL_WPA_KEY_MIC_LEN);

	return p;
}

/* Build and send G2 message */
static bool
wlc_wpa_gtk_sendG2(wlc_sup_info_t *sup_info, wlc_bsscfg_t *cfg, uint16 flags, wpa_msg_t msg)
{
	supplicant_t *sup = SUP_BSSCFG_CUBBY_PRIV(sup_info, cfg);
	void * p;
	wlc_info_t *wlc = sup->wlc;

	p = wlc_wpa_gtk_prepG2(sup, flags, msg);
	if (p != NULL) {
		wlc_sendpkt(wlc, p, sup->cfg->wlcif);
		return TRUE;
	}
	return FALSE;
}

#ifdef MFP
static bool
wlc_wpa_sup_extract_igtk(supplicant_t *sup, eapol_header_t *eapol)
{
	eapol_wpa_key_header_t *body = (eapol_wpa_key_header_t *)eapol->body;
	uint16 data_len = ntoh16_ua(&body->data_len);
	wlc_bsscfg_t *bsscfg = sup->cfg;
	wsec_igtk_info_t *igtk = &bsscfg->igtk;

	eapol_wpa2_encap_data_t *data_encap;
	eapol_wpa2_key_igtk_encap_t *igtk_kde;

	/* extract IGTK */
	data_encap = wpa_find_igtk_encap(body->data, data_len);
	if (!data_encap) {
		WL_WSEC(("wl%d: wlc_wpa_sup_eapol: encapsulated IGTK missing"
			       " from message 3\n", UNIT(sup)));
		return FALSE;
	}
	WL_WSEC(("wlc_wpa_extract_igtk\n"));
	igtk->len = data_encap->length -
		((EAPOL_WPA2_ENCAP_DATA_HDR_LEN - TLV_HDR_LEN) +
		EAPOL_WPA2_KEY_IGTK_ENCAP_HDR_LEN);
	igtk_kde = (eapol_wpa2_key_igtk_encap_t *)data_encap->data;
	igtk->id = igtk_kde->key_id;
	igtk->ipn_lo = *(uint32 *)igtk_kde->ipn;
	igtk->ipn_hi = *(uint16 *)(igtk_kde->ipn + 4);
	WL_WSEC(("ipn hi and ipn low = %d, %d\n", igtk->ipn_lo, igtk->ipn_hi));
	bcopy(igtk_kde->key, igtk->key, AES_TK_LEN);
	return TRUE;
}
#endif /* MFP */

static bool
wlc_wpa_gtk_eapol(supplicant_t *sup, eapol_header_t *eapol, bool encrypted)
{
	wlc_sup_info_t *sup_info = sup->m_handle;
	eapol_wpa_key_header_t *body = (eapol_wpa_key_header_t *)eapol->body;
	uint16 key_info, key_len, data_len;
	uint16 cipher;
	wpapsk_t *wpa = sup->wpa;

	WL_WSEC(("wl%d: %s: received EAPOL_WPA_KEY packet\n",
	         UNIT(sup), __FUNCTION__));

	key_info = ntoh16_ua(&body->key_info);

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
		WL_WSEC(("wl%d: %s: ignoring replay "
				 "(got %02x%02x%02x%02x%02x%02x%02x%02x"
				 " last saw %02x%02x%02x%02x%02x%02x%02x%02x)\n",
				 UNIT(sup), __FUNCTION__,
				 g[0], g[1], g[2], g[3], g[4], g[5], g[6], g[7],
				 s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]));
#endif /* BCMDBG || WLMSG_WSEC */

		return TRUE;
	}
#if defined(BCMDBG) || defined(WLMSG_WSEC)
	{
		uchar *g = body->replay, *s = wpa->replay;
		WL_WSEC(("wl%d: %s: NO replay "
			"(got %02x%02x%02x%02x%02x%02x%02x%02x"
			" last saw %02x%02x%02x%02x%02x%02x%02x%02x)\n",
			UNIT(sup), __FUNCTION__,
			g[0], g[1], g[2], g[3], g[4], g[5], g[6], g[7],
			s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]));
	}
#endif /* BCMDBG || WLMSG_WSEC */

	/* check message MIC */
	if ((key_info & WPA_KEY_MIC) &&
		!wpa_check_mic(eapol, key_info & (WPA_KEY_DESC_V1|WPA_KEY_DESC_V2),
		wpa->eapol_mic_key)) {
		/* 802.11-2007 clause 8.5.3.3 - silently discard MIC failure */
		WL_WSEC(("wl%d: %s: MIC failure, discarding pkt\n",
		         UNIT(sup), __FUNCTION__));
		return TRUE;
	}

	/* if MIC was okay, save replay counter */
	/* last_replay is NOT incremented after transmitting a message */
	bcopy(body->replay, wpa->replay, EAPOL_KEY_REPLAY_LEN);
	bcopy(body->replay, wpa->last_replay, EAPOL_KEY_REPLAY_LEN);

	/* decrypt key data field */
	if (key_info & WPA_KEY_ENCRYPTED_DATA) {

		uint8 *data, *encrkey;
		rc4_ks_t *rc4key;
		bool decr_status;

		if (!(data = MALLOC(sup->osh, WPA_KEY_DATA_LEN_256))) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				UNIT(sup), __FUNCTION__, MALLOCED(sup->osh)));
			wlc_wpa_send_gtk_status(sup_info, sup->cfg, WLC_E_SUP_DECRYPT_KEY_DATA);
			return FALSE;
		}
		if (!(encrkey = MALLOC(sup->osh, WPA_MIC_KEY_LEN*2))) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				UNIT(sup), __FUNCTION__, MALLOCED(sup->osh)));
			MFREE(sup->osh, data, WPA_KEY_DATA_LEN_256);
			wlc_wpa_send_gtk_status(sup_info, sup->cfg, WLC_E_SUP_DECRYPT_KEY_DATA);
			return FALSE;
		}
		if (!(rc4key = MALLOC(sup->osh, sizeof(rc4_ks_t)))) {
			WL_ERROR(("wl%d: %s: out of memory, malloced %d bytes\n",
				UNIT(sup), __FUNCTION__, MALLOCED(sup->osh)));
			MFREE(sup->osh, data, WPA_KEY_DATA_LEN_256);
			MFREE(sup->osh, encrkey, WPA_MIC_KEY_LEN*2);
			wlc_wpa_send_gtk_status(sup_info, sup->cfg, WLC_E_SUP_DECRYPT_KEY_DATA);
			return FALSE;
		}

		decr_status = wpa_decr_key_data(body, key_info,
		                       wpa->eapol_encr_key, NULL, data, encrkey, rc4key);

		MFREE(sup->osh, data, WPA_KEY_DATA_LEN_256);
		MFREE(sup->osh, encrkey, WPA_MIC_KEY_LEN*2);
		MFREE(sup->osh, rc4key, sizeof(rc4_ks_t));

		if (!decr_status) {
			WL_WSEC(("wl%d: %s: decryption of key"
					"data failed\n", UNIT(sup), __FUNCTION__));
			wlc_wpa_send_gtk_status(sup_info, sup->cfg, WLC_E_SUP_DECRYPT_KEY_DATA);
			return FALSE;
		}
	}

	key_len = ntoh16_ua(&body->key_len);
	cipher = CRYPTO_ALGO_OFF;
	wpa->mcipher = CRYPTO_ALGO_OFF;

	if (!(key_info & WPA_KEY_PAIRWISE)) {
		/* Pairwise flag clear; should be group key message. */
		if ((key_info & GMSG1_REQUIRED) != GMSG1_REQUIRED) {
			WL_WSEC(("wl%d: %s: unexpected key_info (0x%04x)in"
				 "WPA group key message\n",
				 UNIT(sup), __FUNCTION__, (uint)key_info));
			return TRUE;
		}
		/* only for WPA2 */
		eapol_wpa2_encap_data_t *data_encap;
		eapol_wpa2_key_gtk_encap_t *gtk_kde;

		/* extract GTK */
		data_len = ntoh16_ua(&body->data_len);
		data_encap = wpa_find_gtk_encap(body->data, data_len);
		if (!data_encap || ((uint)(data_encap->length - EAPOL_WPA2_GTK_ENCAP_MIN_LEN)
			> sizeof(wpa->gtk))) {
			WL_WSEC(("wl%d: %s: encapsulated GTK missing from"
				" group message 1\n", UNIT(sup), __FUNCTION__));
			wlc_wpa_send_gtk_status(sup_info, sup->cfg,
				WLC_E_SUP_GRP_MSG1_NO_GTK);
			return FALSE;
		}
		/* note: data encap length checked during lookup above */
		wpa->gtk_len = data_encap->length - ((EAPOL_WPA2_ENCAP_DATA_HDR_LEN -
		                                          TLV_HDR_LEN) +
		                                         EAPOL_WPA2_KEY_GTK_ENCAP_HDR_LEN);
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
			wlc_wpa_sup_extract_igtk(sup, eapol);
#endif // endif

		/* send group message 2 */
		if (wlc_wpa_gtk_sendG2(sup_info, sup->cfg,
			(key_info & GMSG2_MATCH_FLAGS), GMSG2)) {
			WL_WSEC(("wl%d: %s: key update complete\n",
			         UNIT(sup), __FUNCTION__));
		} else {
			WL_WSEC(("wl%d: %s: send grp msg 2 failed\n",
			         UNIT(sup), __FUNCTION__));
			wlc_wpa_send_gtk_status(sup_info, sup->cfg, WLC_E_SUP_SEND_FAIL);
		}
	}
	return TRUE;
}

bool
wlc_gtk_refresh(wlc_info_t *wlc, struct scb *scb, struct wlc_frminfo *f)
{
	wlc_bsscfg_t                 *bsscfg;
	struct dot11_llc_snap_header *lsh;
	uchar                        *pframe;	/* pointer to body of frame */
	uint                         frame_len; /* length of the frame body */
	eapol_header_t               *ep;
	int                          wepbit = f->fc & FC_WEP;
	supplicant_t                 *sup_bsscfg_priv;

	ASSERT(scb != NULL);
	bsscfg = SCB_BSSCFG(scb);
	ASSERT(bsscfg != NULL);

	if (!bsscfg || !wlc->idsup ||
	 !(wlc->pub->wake_event_enable & (WAKE_EVENT_GTK_HANDSHAKE_ERROR_BIT |
		WAKE_EVENT_4WAY_HANDSHAKE_REQUEST_BIT))) {
		return FALSE;
	}

	eapol_wpa_key_header_t *body;
	uint16 key_info;
	bss_sup_info_t **psup_bss = SUP_BSSCFG_CUBBY_LOC(wlc->idsup, bsscfg);
	if (*psup_bss == NULL)
		return FALSE;

	if (!f->ismulti && !wepbit && !FC_SUBTYPE_ANY_NULL(f->subtype)) {
		wlc->pub->wake_event_enable &= ~((uint32)WAKE_EVENT_GTK_HANDSHAKE_ERROR_BIT);
		/* 4-way handshake detection */
		if (wlc->pub->wake_event_enable & WAKE_EVENT_4WAY_HANDSHAKE_REQUEST_BIT) {
			if (!bcmp(wlc_802_1x_hdr, f->pbody, DOT11_LLC_SNAP_HDR_LEN)) {
				ep =	(eapol_header_t *)
					(f->pbody + DOT11_LLC_SNAP_HDR_LEN - ETHER_HDR_LEN);
				body = (eapol_wpa_key_header_t *)ep->body;
				key_info = ntoh16_ua(&body->key_info);
				if ((ep->type == EAPOL_KEY) &&
				    (key_info & WPA_KEY_PAIRWISE)) {
					wlc_bss_eapol_event(wlc, bsscfg, &scb->ea,
						f->pbody + DOT11_LLC_SNAP_HDR_LEN,
						f->body_len - DOT11_LLC_SNAP_HDR_LEN);
					goto toss;
				}
			}
		}
	} else if (!f->ismulti && wepbit) {
		if (f->pbody[3] & DOT11_EXT_IV_FLAG) {
			pframe = (uchar*)f->pbody + 8;
			frame_len = f->body_len - 8;
		} else {
			pframe = (uchar*)f->pbody + 4;
			frame_len = f->body_len - 4;
		}

		lsh = (struct dot11_llc_snap_header *)pframe;
		if (lsh->dsap == 0xaa && lsh->ssap == 0xaa && lsh->ctl == 0x03 &&
			lsh->oui[0] == 0 && lsh->oui[1] == 0 &&
			((lsh->oui[2] == 0x00 &&
		      !(ntoh16(lsh->type) == 0x80f3 || ntoh16(lsh->type) == 0x8137)) ||
			(lsh->oui[2] == 0xf8 &&
		     (ntoh16(lsh->type) == 0x80f3 || ntoh16(lsh->type) == 0x8137)))) {
			if (ntoh16(lsh->type) == ETHER_TYPE_802_1X) {
				ep = (eapol_header_t *)(pframe +
				      DOT11_LLC_SNAP_HDR_LEN - ETHER_HDR_LEN);

				if (ntoh16(ep->length) > (frame_len - EAPOL_HEADER_LEN + 6))
					goto toss;

				sup_bsscfg_priv = SUP_BSSCFG_CUBBY_PRIV(wlc->idsup, bsscfg);
				/* Save eapol version from the AP */
				sup_bsscfg_priv->ap_eapver = ep->version;
				/* If this is  a WPA key message, send it to the code for WPA */
				if ((ep->type == EAPOL_KEY))
				{
					eapol_wpa_key_header_t *body;
					body = (eapol_wpa_key_header_t *)ep->body;
					if (SUP_CHECK_EAPOL(body)) {
						(void) wlc_wpa_gtk_eapol(sup_bsscfg_priv,
						                      ep, (wepbit != 0));
						goto toss;
					}
				}
			}
		}
	}

	return FALSE;

toss:
	if (WME_ENAB(wlc->pub)) {
		WLCNTINCR(wlc->pub->_wme_cnt->rx_failed[WME_PRIO2AC(PKTPRIO(f->p))].packets);
		WLCNTADD(wlc->pub->_wme_cnt->rx_failed[WME_PRIO2AC(PKTPRIO(f->p))].bytes,
			pkttotlen(wlc->osh,  f->p));
	}
	PKTFREE(wlc->osh, f->p, FALSE);
	return TRUE;
}
#endif /* EXT_STA */
