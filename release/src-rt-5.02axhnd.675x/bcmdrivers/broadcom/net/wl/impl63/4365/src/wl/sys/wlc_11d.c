/**
 * 802.11d (support for additional regulatory domains) module source file
 * Broadcom 802.11abgn Networking Device Driver
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
 * $Id: wlc_11d.c 708017 2017-06-29 14:11:45Z $
 */

/**
 * @file
 * @brief
 * A device may implement "multi-domain" operation, meaning it can operate in regulatory compliance
 * (using conforming channels, tx power) w/several domains. Thus it must find the domain and any
 * relevant information in order to operate. The Country IE allows a STA to get this info
 * automatically from an AP; in this case the STA should only do passive scanning until it obtains
 * the information from an AP. If the capability is disabled, (default) the STA use built-in
 * defaults, modified by client commands; but still should not transmit unless it has a valid
 * country code. The 802.11d specification also defines IEs for Frequency Hopping info
 * (Parameters and Pattern), and a Request IE which a STA may use (in a Probe Request, for example)
 * to ask for specific IEs to be included in the response from the AP.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [WlDriver11DH]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifdef WL11D

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_channel.h>
#include <wlc.h>
#include <wlc_scan.h>
#include <wlc_cntry.h>
#include <wlc_11d.h>
#include <phy_utils_api.h>

/* IOVar table */
/* No ordering is imposed */
enum {
	IOV_AUTOCOUNTRY_DEFAULT,
	IOV_AUTOCOUNTRY,
	IOV_LAST
};

static const bcm_iovar_t wlc_11d_iovars[] = {
	{"autocountry_default", IOV_AUTOCOUNTRY_DEFAULT, (0), IOVT_BUFFER, 0},
#ifdef STA
	{"autocountry", IOV_AUTOCOUNTRY, (0), IOVT_BOOL, 0},
#endif /* STA */
	{NULL, 0, 0, 0, 0}
};

/* ioctl table */
static const wlc_ioctl_cmd_t wlc_11d_ioctls[] = {
	{WLC_SET_REGULATORY, 0, sizeof(int)},
	{WLC_GET_REGULATORY, 0, sizeof(int)}
};

/* Country module info */
struct wlc_11d_info {
	wlc_info_t *wlc;
	char autocountry_default[WLC_CNTRY_BUF_SZ];	/* initial country for 802.11d
							 * auto-country mode
							 */
	bool autocountry_adopted_from_ap;	/* whether the current locale is adopted from
						 * country IE of a associated AP
						 */
	bool awaiting_cntry_info;		/* still waiting for country ie in 802.11d mode */
};

/* local functions */
/* module */
static int wlc_11d_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
#ifdef BCMDBG
static int wlc_11d_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif
static int wlc_11d_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif);

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* module */
wlc_11d_info_t *
BCMATTACHFN(wlc_11d_attach)(wlc_info_t *wlc)
{
	wlc_11d_info_t *m11d;

	if ((m11d = MALLOCZ(wlc->osh, sizeof(wlc_11d_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	m11d->wlc = wlc;

#ifdef BCMDBG
	if (wlc_dump_register(wlc->pub, "11d", wlc_11d_dump, m11d) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_dumpe_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif

	if (wlc_module_register(wlc->pub, wlc_11d_iovars, "11d", m11d, wlc_11d_doiovar,
	                        NULL, NULL, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

	if (wlc_module_add_ioctl_fn(wlc->pub, m11d, wlc_11d_doioctl,
	                            ARRAYSIZE(wlc_11d_ioctls), wlc_11d_ioctls) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_add_ioctl_fn() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}

#ifdef PCOEM_LINUXSTA
	/* enable regulatory */
	wlc->pub->_11d = TRUE;
#endif // endif

	return m11d;

	/* error handling */
fail:
	wlc_11d_detach(m11d);
	return NULL;
}

void
BCMATTACHFN(wlc_11d_detach)(wlc_11d_info_t *m11d)
{
	wlc_info_t *wlc;

	if (m11d == NULL)
		return;

	wlc = m11d->wlc;

	wlc_module_remove_ioctl_fn(wlc->pub, m11d);
	wlc_module_unregister(wlc->pub, "11d", m11d);

	MFREE(wlc->osh, m11d, sizeof(wlc_11d_info_t));
}

static int
wlc_11d_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_11d_info_t *m11d = (wlc_11d_info_t *)ctx;
	wlc_info_t *wlc = m11d->wlc;
	int err = BCME_OK;
	int32 int_val = 0;
	int32 *ret_int_ptr;
	bool bool_val;

	/* convenience int and bool vals for first 8 bytes of buffer */
	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	/* convenience int ptr for 4-byte gets (requires int aligned arg) */
	ret_int_ptr = (int32 *)arg;
	BCM_REFERENCE(ret_int_ptr);

	bool_val = (int_val != 0) ? TRUE : FALSE;
	BCM_REFERENCE(bool_val);

	/* Do the actual parameter implementation */
	switch (actionid) {
	case IOV_GVAL(IOV_AUTOCOUNTRY_DEFAULT):
		if ((uint)len <= strlen(m11d->autocountry_default) + 1) {
			err = BCME_BUFTOOSHORT;
			break;
		}
		strncpy(arg, m11d->autocountry_default, len - 1);
		break;

	case IOV_SVAL(IOV_AUTOCOUNTRY_DEFAULT): {
		clm_country_t unused;
		char country_abbrev[WLC_CNTRY_BUF_SZ];
		int slen;

		/* find strlen, with string either null terminated or 'len' terminated */
		for (slen = 0; slen < len && ((char*)arg)[slen] != '\0'; slen++)
			;
		if (slen >= WLC_CNTRY_BUF_SZ) {
			err = BCME_BUFTOOLONG;
			break;
		}
		/* copy country code from arg avoiding overruns and null terminating */
		bzero(country_abbrev, WLC_CNTRY_BUF_SZ);
		strncpy(country_abbrev, (char*)arg, slen);

		WL_REGULATORY(("wl%d:%s(): set IOV_AUTOCOUNTRY_DEFAULT %s\n",
			wlc->pub->unit, __FUNCTION__, country_abbrev));

		if (slen == 0 || wlc_country_lookup(wlc, country_abbrev, &unused) !=
			CLM_RESULT_OK) {
			err = BCME_BADARG;
			break;
		}

		if (!wlc_is_sromccode_autocountrysupported(wlc)) {
			WL_ERROR(("wl%d:%s(): set IOV_AUTOCOUNTRY_DEFAULT: failed "
				"%s not part of autocountry_default list\n",
				wlc->pub->unit, __FUNCTION__, m11d->autocountry_default));
			err = BCME_UNSUPPORTED;
			break;
		}

		strncpy(m11d->autocountry_default, country_abbrev, WLC_CNTRY_BUF_SZ - 1);
		break;
	}

#ifdef STA
	case IOV_GVAL(IOV_AUTOCOUNTRY):
		*ret_int_ptr = (int)wlc->pub->_autocountry;
		break;

	case IOV_SVAL(IOV_AUTOCOUNTRY): {
		bool autocountry;
		bool awaiting_cntry_info;

		if (!wlc_is_sromccode_autocountrysupported(wlc)) {
			WL_ERROR(("wl%d:%s(): set IOV_AUTOCOUNTRY: failed "
				"%s not part of autocountry_default list\n",
				wlc->pub->unit, __FUNCTION__, m11d->autocountry_default));
			err = BCME_UNSUPPORTED;
			break;
		}

		if (wlc->pub->associated) {
			err = BCME_ASSOCIATED;
			break;
		}

		if (SCAN_IN_PROGRESS(wlc->scan))
			wlc_scan_abort(wlc->scan, WLC_E_STATUS_ABORT);

		autocountry = wlc->pub->_autocountry;
		awaiting_cntry_info = m11d->awaiting_cntry_info;

		wlc->pub->_autocountry = bool_val;

		m11d->awaiting_cntry_info = bool_val;

		if (bool_val) {
			WL_INFORM(("wl%d:%s(): IOV_AUTOCOUNTRY is TRUE, re-init to "
				"autocountry_defualt %s\n", wlc->pub->unit, __FUNCTION__,
				m11d->autocountry_default));
			/* Re-init channels and locale to Auto Country default */
			err = wlc_set_countrycode(wlc->cmi, m11d->autocountry_default);
		} else {
			WL_INFORM(("wl%d:%s(): IOV_AUTOCOUNTRY is FALSE,\n",
			           wlc->pub->unit, __FUNCTION__));
			err = wlc_cntry_use_default(wlc->cntry);
		}

		if (err) {
			WL_ERROR(("wl%d:%s(): IOV_AUTOCOUNTRY  %d failed\n",
				wlc->pub->unit, __FUNCTION__, bool_val));
			wlc->pub->_autocountry = autocountry;
			m11d->awaiting_cntry_info = awaiting_cntry_info;
			break;
		}

		m11d->autocountry_adopted_from_ap = FALSE;

		break;
	}
#endif /* STA */

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

static int
wlc_11d_doioctl(void *ctx, int cmd, void *arg, int len, struct wlc_if *wlcif)
{
	wlc_11d_info_t *m11d = (wlc_11d_info_t *)ctx;
	wlc_info_t *wlc = m11d->wlc;
	int val = 0, *pval;
	int err = BCME_OK;
	bool bool_val;

	/* default argument is generic integer */
	pval = (int *)arg;

	/* This will prevent the misaligned access */
	if (pval && (uint32)len >= sizeof(val))
		bcopy(pval, &val, sizeof(val));

	bool_val = (val != 0) ? TRUE : FALSE;

	switch (cmd) {
	case WLC_SET_REGULATORY:
		wlc->pub->_11d = bool_val;
		break;

	case WLC_GET_REGULATORY:
		*pval = (int)wlc->pub->_11d;
		break;

	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}

#ifdef BCMDBG
static int
wlc_11d_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_11d_info_t *m11d = (wlc_11d_info_t *)ctx;
	wlc_info_t *wlc = m11d->wlc;

	bcm_bprintf(b, "reg_domain:%d\n", WL11D_ENAB(wlc));
	bcm_bprintf(b, "autocountry:%d autocountry_def:%s adopted_from_ap:%d awaiting:%d\n",
	            WLC_AUTOCOUNTRY_ENAB(wlc), m11d->autocountry_default,
	            m11d->autocountry_adopted_from_ap, m11d->awaiting_cntry_info);

	return BCME_OK;
}
#endif /* BCMDBG */

#ifdef STA
/* Determine if the country channel information is compatible with the current association.
 * Returns TRUE if the country setting includes the channel of the currently associated AP
 * or IBSS, FALSE otherwise.
 */
static int
wlc_11d_compatible_country(wlc_11d_info_t *m11d, const char *country_abbrev)
{
	wlc_info_t *wlc = m11d->wlc;
	chanvec_t channels;
	chanspec_t chanspec;

	/* should only be called if associated to an AP */
	ASSERT(wlc->pub->associated);
	if (!wlc->pub->associated)
		return TRUE;

	chanspec = wlc->home_chanspec;
	ASSERT(!wf_chspec_malformed(chanspec));

	if (wlc_channel_get_chanvec(wlc, country_abbrev,
		(CHSPEC_IS2G(chanspec) ? WLC_BAND_2G : WLC_BAND_5G), &channels) == FALSE)
		return FALSE;

	if (CHSPEC_IS8080(chanspec)) {
		uint ch;
		int i;
		bool compat = TRUE;

		ch = wf_chspec_primary80_channel(chanspec);

		/* check each 20MHz sub-channel in the first 80MHz channel */
		ch = ch - CH_40MHZ_APART + CH_10MHZ_APART;

		for (i = 0; i < 4; i++, ch += CH_20MHZ_APART) {
			if (!isset(channels.vec, ch)) {
				compat = FALSE;
				break;
			}
		}

		ch = wf_chspec_secondary80_channel(chanspec);
		/* well-formed 80p80 channel should have valid secondary channel */
		ASSERT(ch > 0);

		/* check each 20MHz sub-channel in the 80MHz channel */
		ch = ch - CH_40MHZ_APART + CH_10MHZ_APART;

		for (i = 0; i < 4; i++, ch += CH_20MHZ_APART) {
			if (!isset(channels.vec, ch)) {
				compat = FALSE;
				break;
			}
		}

		return compat;
	}
	else if (CHSPEC_IS160(chanspec)) {
		uint ch;
		int i;
		bool compat = TRUE;

		/* check each 20MHz sub-channel in the 160MHz channel */
		ch = CHSPEC_CHANNEL(chanspec) - CH_80MHZ_APART + CH_10MHZ_APART;

		for (i = 0; i < 8; i++, ch += CH_20MHZ_APART) {
			if (!isset(channels.vec, ch)) {
				compat = FALSE;
				break;
			}
		}
		return compat;
	}
	else if (CHSPEC_IS80(chanspec)) {
		uint ch;
		int i;
		bool compat = TRUE;

		/* check each 20MHz sub-channel in the 80MHz channel */
		ch = CHSPEC_CHANNEL(chanspec) - CH_40MHZ_APART + CH_10MHZ_APART;

		for (i = 0; i < 4; i++, ch += CH_20MHZ_APART) {
			if (!isset(channels.vec, ch)) {
				compat = FALSE;
				break;
			}
		}
		return compat;
	} else if (CHSPEC_IS40(chanspec))
		return isset(channels.vec, LOWER_20_SB(CHSPEC_CHANNEL(chanspec))) &&
		        isset(channels.vec, UPPER_20_SB(CHSPEC_CHANNEL(chanspec)));
	else
		return isset(channels.vec, CHSPEC_CHANNEL(chanspec));
}

void
wlc_11d_scan_complete(wlc_11d_info_t *m11d, int status)
{
	wlc_info_t *wlc = m11d->wlc;
	wlcband_t *band;
	wlc_bss_info_t *bi;
	bcm_tlv_t *country_ie;
	bcm_tlv_t *best_country_ie = NULL;
	char best_country_abbrev[WLC_CNTRY_BUF_SZ];
	uint8 *tags;
	uint tag_len;
	uint i, j;
	char country_abbrev[WLC_CNTRY_BUF_SZ];
	chanvec_t supported_channels;
	chanvec_t channels, unused;
	int8 ie_tx_pwr[MAXCHANNEL];
	clm_country_t country_new;
	clm_result_t result = CLM_RESULT_ERR;
	clm_country_locales_t locale_new;
	int a_max, b_max;
	int a_band, b_band;
	int err;
#if defined(BCMDBG) || defined(WLMSG_INFORM)
	char eabuf[ETHER_ADDR_STR_LEN];
	char ssidbuf[SSID_FMT_BUF_LEN];
	wlc_bss_info_t *best_bi = NULL;
#endif	/* BCMDBG || WLMSG_INFORM */

	/* This routine should only be called if we are still looking for country information. */
	if (!m11d->awaiting_cntry_info)
		return;

	/* get all the phy supported channels */
	bzero(&supported_channels, sizeof(chanvec_t));
	band = wlc->band;
	for (i = 0; i < NBANDS(wlc); i++, band = wlc->bandstate[OTHERBANDUNIT(wlc)]) {
		phy_utils_chanspec_band_validch((phy_info_t *)WLC_PI_BANDUNIT(wlc, band->bandunit),
			band->bandtype, &channels);
		for (j = 0; j < sizeof(chanvec_t); j++)
			supported_channels.vec[j] |= channels.vec[j];
		if (!IS_MBAND_UNLOCKED(wlc))
			break;
	}

	/* Walk the list of APs, finding the ones with Country IEs.
	 * Keep the best choice so far as we go
	 */
	a_max = b_max = 0;
	for (i = 0; i < wlc->scan_results->count; i++) {

		a_band = b_band = 0;

		bi = wlc->scan_results->ptrs[i];
		ASSERT(bi != NULL);

		/* skip IBSS bcn/prb, or scan results with no bcn/prb */
		if (bi->infra == 0 || !bi->bcn_prb || (bi->bcn_prb_len <= DOT11_BCN_PRB_LEN))
			continue;

		tag_len = bi->bcn_prb_len - sizeof(struct dot11_bcn_prb);
		tags = (uint8 *)bi->bcn_prb + sizeof(struct dot11_bcn_prb);

		/* skip if no Country IE */
		country_ie = bcm_parse_tlvs(tags, tag_len, DOT11_MNG_COUNTRY_ID);
		if (!country_ie)
			continue;

		bzero(ie_tx_pwr, sizeof(ie_tx_pwr));
		bzero(channels.vec, sizeof(channels.vec));
		err = wlc_cntry_parse_country_ie(wlc->cntry, country_ie, country_abbrev,
		                                 &channels, ie_tx_pwr);
		if (err) {
			WL_REGULATORY(("wl%d: %s: skipping malformed Country IE on "
			               "AP %s \"%s\" channel %d\n",
			               wlc->pub->unit, __FUNCTION__,
			               bcm_ether_ntoa(&bi->BSSID, eabuf),
			               (wlc_format_ssid(ssidbuf, bi->SSID, bi->SSID_len), ssidbuf),
			               wf_chspec_ctlchan(bi->chanspec)));
			continue;
		}

		/* skip if the Country IE does not have a known country code */
		result = wlc_country_lookup_ext(wlc, country_abbrev, &country_new);
		if (result != CLM_RESULT_OK) {
			WL_REGULATORY(("wl%d: %s: skipping unknown country code \"%s\" from "
			               "AP %s \"%s\" channel %d\n",
			               wlc->pub->unit, __FUNCTION__,
			               country_abbrev, bcm_ether_ntoa(&bi->BSSID, eabuf),
			               (wlc_format_ssid(ssidbuf, bi->SSID, bi->SSID_len), ssidbuf),
			               wf_chspec_ctlchan(bi->chanspec)));
			continue;
		}

		/* count the channels in each band */
		result = wlc_get_locale(country_new, &locale_new);
		if (result == CLM_RESULT_OK) {
#ifdef BAND2G
			wlc_locale_get_channels(&locale_new, CLM_BAND_2G, &channels, &unused);
			for (j = 0; j < sizeof(chanvec_t); j++)
				channels.vec[j] &= supported_channels.vec[j];
			b_band = bcm_bitcount(channels.vec, sizeof(chanvec_t));
#endif // endif
#ifdef BAND5G
			wlc_locale_get_channels(&locale_new, CLM_BAND_5G, &channels, &unused);
			for (j = 0; j < sizeof(chanvec_t); j++)
				channels.vec[j] &= supported_channels.vec[j];
			a_band = bcm_bitcount(channels.vec, sizeof(chanvec_t));
#endif // endif
		}
		WL_REGULATORY(("wl%d: %s: AP %s \"%s\" with Country IE: "
		               "\"%s\" %d 2GHz, %d 5GHz channels\n",
		               wlc->pub->unit, __FUNCTION__,
		               bcm_ether_ntoa(&bi->BSSID, eabuf),
		               (wlc_format_ssid(ssidbuf, bi->SSID, bi->SSID_len), ssidbuf),
		               country_abbrev, b_band, a_band));

		/* Pick the best by most channels */
		if ((a_band + b_band) > (a_max + b_max)) {
			if (best_country_ie)
				WL_REGULATORY(("wl%d: %s: Country IE \"%s\" with "
				               "%d 2GHz, %d 5GHz channels preferred over "
				               "\"%s\" with %d 2GHz, %d 5GHz channels.\n",
				               wlc->pub->unit, __FUNCTION__,
				               country_abbrev, a_band, b_band,
				               best_country_abbrev, a_max, b_max));
			a_max = a_band;
			b_max = b_band;
			best_country_ie = country_ie;
			strncpy(best_country_abbrev, country_abbrev,
			        sizeof(best_country_abbrev) - 1);
#if defined(BCMDBG) || defined(WLMSG_INFORM)
			best_bi = bi;
#endif // endif
		}
	}

	if (!best_country_ie)
		return;

	/* keep only the 2 char ISO code for our country setting,
	 * dropping the Indoor/Outdoor/Either specification in the 3rd char
	 */
	best_country_abbrev[2] = '\0';

	if (wlc->pub->associated &&
	    !wlc_11d_compatible_country(wlc->m11d, best_country_abbrev)) {
		WL_REGULATORY(("wl%d: %s: Not adopting best choice Country \"%s\" from "
			"AP %s \"%s\" channel %d since it is incompatible with our "
			"current association on channel %d\n",
			wlc->pub->unit, __FUNCTION__, best_country_abbrev,
			bcm_ether_ntoa(&best_bi->BSSID, eabuf),
			(wlc_format_ssid(ssidbuf, best_bi->SSID, best_bi->SSID_len), ssidbuf),
			wf_chspec_ctlchan(best_bi->chanspec),
			wf_chspec_ctlchan(wlc->home_chanspec)));
		return;
	}

	/* Adopt the best choice */
	WL_INFORM(("wl%d: %s: Adopting Country IE \"%s\" from AP %s \"%s\" channel %d\n",
	           wlc->pub->unit, __FUNCTION__, best_country_abbrev,
	           bcm_ether_ntoa(&best_bi->BSSID, eabuf),
	           (wlc_format_ssid(ssidbuf, best_bi->SSID, best_bi->SSID_len), ssidbuf),
	           wf_chspec_ctlchan(best_bi->chanspec)));

	wlc_set_countrycode(wlc->cmi, best_country_abbrev);
	m11d->awaiting_cntry_info = FALSE;
	/* The current channel could be chatty in the new country code */
	if (wlc->pub->up && !wlc_quiet_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC))
		wlc_mute(wlc, OFF, 0);
}

void
wlc_11d_adopt_country(wlc_11d_info_t *m11d, char *country_str, bool adopt_country)
{
	wlc_info_t *wlc = m11d->wlc;
	char country_abbrev[WLC_CNTRY_BUF_SZ];
	clm_country_t country_new;
	clm_result_t result;

	if (!WLC_AUTOCOUNTRY_ENAB(wlc))
		return;

	if (!(m11d->awaiting_cntry_info ||
	      adopt_country ||
	      !m11d->autocountry_adopted_from_ap))
		return;

	WL_REGULATORY(("wl%d: %s: Adopting Country from associated AP\n",
	               wlc->pub->unit, __FUNCTION__));

	/* create the 2 char country code from the ISO country code */
	strncpy(country_abbrev, country_str, sizeof(country_abbrev) - 1);
	country_abbrev[2] = '\0';

	result = wlc_country_lookup_ext(wlc, country_str, &country_new);
	if (result != CLM_RESULT_OK) {
		WL_REGULATORY(("wl%d: Ignoring associated AP's Country \"%s\" since "
		               "no match was found in built-in table\n",
		               wlc->pub->unit, country_str));
	}
	if (!wlc_11d_compatible_country(wlc->m11d, country_abbrev)) {
		WL_REGULATORY(("wl%d: Ignoring associated AP's Country \"%s\" "
		               "since it is incompatible with the associated channel %d\n",
		               wlc->pub->unit, country_str,
		               wf_chspec_ctlchan(wlc->home_chanspec)));
	}

	WL_INFORM(("wl%d: setting regulatory information from built-in "
	           "country \"%s\" matching associated AP's Country\n",
	           wlc->pub->unit, country_abbrev));

	wlc_set_countrycode(wlc->cmi, country_abbrev);
	m11d->awaiting_cntry_info = FALSE;
	/* The current channel could be chatty in the new country code */
	if (!wlc_quiet_chanspec(wlc->cmi, WLC_BAND_PI_RADIO_CHANSPEC))
		wlc_mute(wlc, OFF, 0);

	if (adopt_country)
		m11d->autocountry_adopted_from_ap = TRUE;
}
#endif /* STA */

void
wlc_11d_reset_all(wlc_11d_info_t *m11d)
{
	m11d->awaiting_cntry_info = FALSE;
}

/* accessors */
bool
wlc_11d_autocountry_adopted(wlc_11d_info_t *m11d)
{
	return m11d->autocountry_adopted_from_ap;
}

void
wlc_11d_set_autocountry_default(wlc_11d_info_t *m11d, const char *country_abbrev)
{
	strncpy(m11d->autocountry_default, country_abbrev, WLC_CNTRY_BUF_SZ - 1);
}

const char *
wlc_11d_get_autocountry_default(wlc_11d_info_t *m11d)
{
	return m11d->autocountry_default;
}
#endif /* WL11D */
