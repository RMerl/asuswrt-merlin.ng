/**
 * @file
 * @brief
 * Exposed interfaces of wlc_okc.c (Opportunistic Key Caching).
 * This code is for external supplicant to handle fast-roaming properly.
 * Currently what is implemented is OKC(WL_OKC) and RCC(WLRCC).
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
 * $Id: wlc_okc.c 708017 2017-06-29 14:11:45Z $
 *
 */

/**
 * @file
 * @brief
 * XXX Twiki: [OKC]
 */

#include <wlc_cfg.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmendian.h>
#include <wlioctl.h>
#include <sbhndpio.h>
#include <sbhnddma.h>
#include <hnddma.h>
#include <d11.h>
#include <bcmwpa.h>
#include <wlc_rate.h>
#include <wlc_key.h>
#include <wlc_channel.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc_pio.h>
#include <wlc.h>
#include <wlc_okc.h>
#include <wlioctl.h>
#include <proto/802.11.h>
#include <bcmcrypto/prf.h>
#include <bcmcrypto/rc4.h>
#include <wlc_pmkid.h>
#if defined(BCMINTSUP) && defined(BCMSUP_PSK)
#include <wlc_sup.h>
#endif // endif

#if !defined(WL_OKC) && !defined(WLRCC)
#error "WL_OKC nor WLRCC is defined!!!"
#endif // endif

/* iovar table */
enum {
#ifdef WL_OKC
	IOV_OKC_ENABLE,        /* enable/disable okc feature */
	IOV_OKC_INFO_PMK,       /* pmk for specfic AP  */
#endif // endif
#ifdef WLRCC
	IOV_RCC_MODE,
	IOV_RCC_CHANNELS
#endif // endif
};

static const bcm_iovar_t okc_rcc_iovars[] = {
#ifdef WL_OKC
	{"okc_enable", IOV_OKC_ENABLE, 0, IOVT_BOOL, 0},
	{"okc_info_pmk", IOV_OKC_INFO_PMK, 0, IOVT_BUFFER, 0},
#endif // endif
#ifdef WLRCC
	{"roamscan_mode", IOV_RCC_MODE, 0, IOVT_BOOL, 0},
	{"roamscan_channels", IOV_RCC_CHANNELS, 0, IOVT_BUFFER, 0},
#endif // endif
	{NULL, 0, 0, 0, 0}
};

#ifdef WLRCC
typedef struct {
	int n;
	chanspec_t channels[MAX_ROAM_CHANNEL];
} channel_list;
#endif // endif

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

/* handle okc related iovars */
static int
wlc_okc_rcc_doiovar(void *context, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *p, uint plen, void *a, int alen, int vsize, struct wlc_if *wlcif)
{
#ifdef WL_OKC
	okc_info_t *okc_info = (okc_info_t *)context;
	wlc_info_t *wlc = okc_info->wlc;
#else
/* in case of WLRCC only */
	wlc_info_t *wlc = (wlc_info_t *)context;
#endif // endif
	wlc_bsscfg_t *bsscfg;
	int32 int_val = 0;
	int err = BCME_OK;

	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	/* all iovars require okc being enabled */
	switch (actionid) {
#ifdef WL_OKC
		case IOV_GVAL(IOV_OKC_ENABLE):
			*((uint32*)a) = wlc->pub->_okc;
			break;

		case IOV_SVAL(IOV_OKC_ENABLE):
			if (BSSCFG_STA(bsscfg))
				wlc->pub->_okc = (int_val  == 1);
			break;

		case IOV_SVAL(IOV_OKC_INFO_PMK): {

			uint8 tmp[PMK_LEN];

			/* if okc is disabled, just do nothing */
			if (!OKC_ENAB(wlc->pub)) {
				break;
			}

			bzero(tmp, PMK_LEN);

			bzero(WLC_OKC_INFO(wlc)->pmk, PMK_LEN);
			bcopy((uint8*)a, WLC_OKC_INFO(wlc)->pmk, PMK_LEN);
			if (bcmp(WLC_OKC_INFO(wlc)->pmk, tmp, PMK_LEN)) {
				WLC_OKC_INFO(wlc)->pmk_len = PMK_LEN;
			}
			else
				WLC_OKC_INFO(wlc)->pmk_len = 0;

			/* clear PMKID cache */
			wlc_pmkid_clear_store(wlc->pmkid_info, bsscfg);
#if defined(BCMINTSUP) && defined(BCMSUP_PSK)
			if (SUP_ENAB(wlc->pub) && BSSCFG_STA(bsscfg) &&
				BSS_SUP_ENAB_WPA(wlc->idsup, bsscfg)) {
				wsec_pmk_t pmk;
				char pmkstr[WSEC_MAX_PSK_LEN + 1];
				int i;
				char *c = pmkstr;

				for (i = 0; i < (WSEC_MAX_PSK_LEN/2); i++) {
					sprintf(c, "%02x", ((uint8 *)a)[i]);
					c += 2;
				}
				pmk.key_len = WSEC_MAX_PSK_LEN;
				pmk.flags = WSEC_PASSPHRASE;
				bcopy(pmkstr, pmk.key, WSEC_MAX_PSK_LEN);
				err = wlc_sup_set_pmk(wlc->idsup, bsscfg, &pmk, bsscfg->associated);
			}
#endif /* BCMINTSUP && BCMSUP_PSK */

			break;
		}
#endif /* WL_OKC */
#ifdef WLRCC
		case IOV_GVAL(IOV_RCC_MODE):
			if (WLRCC_ENAB(wlc->pub)) {
				if (BSSCFG_STA(bsscfg))
					*((uint32 *)a) = bsscfg->roam->rcc_mode;
				else
					*((uint32 *)a) = 0;
			} else {
				err = BCME_UNSUPPORTED;
			}
			break;
		case IOV_SVAL(IOV_RCC_MODE):
			if (WLRCC_ENAB(wlc->pub) && BSSCFG_STA(bsscfg)) {
				if ((int_val == 1) || (int_val == 0)) {
					bsscfg->roam->rcc_mode = (int_val == 1)?
						RCC_MODE_FORCE : int_val;
					if (bsscfg->roam->rcc_mode == RCC_MODE_FORCE) {
						bsscfg->roam->cache_valid = FALSE;
					}
				} else
					err = BCME_BADARG;
			} else {
				err = BCME_UNSUPPORTED;
			}
			break;
		case IOV_GVAL(IOV_RCC_CHANNELS):
			if (WLRCC_ENAB(wlc->pub) && BSSCFG_STA(bsscfg)) {
				channel_list *list = (channel_list *)a;

				list->n = bsscfg->roam->n_rcc_channels;
				memcpy(list->channels, bsscfg->roam->rcc_channels,
					bsscfg->roam->n_rcc_channels * sizeof(chanspec_t));
			} else {
				err = BCME_UNSUPPORTED;
			}
			break;
		/* set RCC channel to specified channels */
		case IOV_SVAL(IOV_RCC_CHANNELS):
			if (WLRCC_ENAB(wlc->pub) && BSSCFG_STA(bsscfg)) {
				channel_list *list;
				list = (channel_list *)a;
				if (list->n < 1) {
					err = BCME_BADARG;
					break;
				}
				bsscfg->roam->n_rcc_channels = list->n;
				memcpy(bsscfg->roam->rcc_channels, list->channels,
					list->n * sizeof(chanspec_t));
				bsscfg->roam->rcc_valid = TRUE;
				bsscfg->roam->cache_valid = FALSE;
			} else {
				err = BCME_UNSUPPORTED;
			}
			break;
#endif /* WLRCC */
		default:
			err = BCME_UNSUPPORTED;
			break;

	}

	return err;
}

#ifdef WL_OKC
int wlc_calc_pmkid_for_okc(wlc_info_t *wlc, wlc_bsscfg_t *cfg, struct ether_addr *BSSID, int *index)
{
#define WPA_KEY_DATA_LEN_128	128	/* allocation size of 128 for temp data pointer. */
	uint8 data[WPA_KEY_DATA_LEN_128];
	uint8 digest[PRF_OUTBUF_LEN];

	if (ETHER_ISNULLADDR(BSSID)) {
		WL_WSEC(("wlc_calc_pmkid_for_okc: can't calculate PMKID - NULL BSSID\n"));
		*index = -1;
		return -1;
	}

	if (WLC_OKC_INFO(wlc)->pmk_len == 0) {
		WL_ERROR(("%s: pmk is not valid\n", __FUNCTION__));
		*index = -1;
		return -1;
	}

	wpa_calc_pmkid_for_okc(wlc->pmkid_info, cfg, BSSID, &cfg->cur_etheraddr,
		WLC_OKC_INFO(wlc)->pmk, (uint)WLC_OKC_INFO(wlc)->pmk_len, data, digest, index);

	return 0;
}
#endif /* WL_OKC */

#ifdef WLRCC

void rcc_update_channel_list(wlc_roam_t *roam, wlc_ssid_t *ssid,
	wl_join_assoc_params_t *assoc_param)
{
	int32 n;

	n = assoc_param->chanspec_num > MAX_ROAM_CHANNEL ?
		MAX_ROAM_CHANNEL : assoc_param->chanspec_num;
	memcpy(roam->rcc_channels, assoc_param->chanspec_list, n * sizeof(chanspec_t));
	roam->n_rcc_channels = n;
	roam->rcc_valid = TRUE;
}

void rcc_add_chanspec(wlc_roam_t *roam, uint8 ch)
{
	int i;

	/* store channels as 20MHZ */
	if (roam->n_rcc_channels >= MAX_ROAM_CHANNEL)
		return;

	for (i = 0; i < roam->n_rcc_channels; i++) {
		if (CHSPEC_CHANNEL(roam->rcc_channels[i]) == ch)
			return;
	}

	WL_ROAM(("RCC:[%d] CH%d(%x) added\n", roam->n_rcc_channels, ch,
		roam->rcc_channels[roam->n_rcc_channels]));
	roam->rcc_channels[roam->n_rcc_channels++] = CH20MHZ_CHSPEC(ch);
}

void rcc_update_from_join_targets(wlc_roam_t *roam, wlc_bss_list_t *targets)
{
	int i;
	uint8 ctl_channel;

	for (i = 0; i < targets->count; i++) {
		ctl_channel = wf_chspec_ctlchan(targets->ptrs[i]->chanspec);
		if (ctl_channel > 0)
			rcc_add_chanspec(roam, ctl_channel);
	}
	if (roam->n_rcc_channels > 0)
		roam->rcc_valid = TRUE;
}

#endif /* WLRCC */

/* module attach/detach */
void *
BCMATTACHFN(wlc_okc_attach)(wlc_info_t *wlc)
{
	void *context;
#ifdef WL_OKC
	okc_info_t *okc_info;

	/* sanity check */

	/* module states */
	if ((okc_info = (okc_info_t *)MALLOCZ(wlc->osh, sizeof(okc_info_t))) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
			wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}

	okc_info->wlc = wlc;
	context = okc_info;
#else
	/* In case of WLRCC only */
	context = wlc;
#endif /* WL_OKC */

	if (wlc_module_register(wlc->pub, okc_rcc_iovars, "okc", context,
		wlc_okc_rcc_doiovar, NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
			wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
	return context;

fail:
	/* error handling */
#ifdef WL_OKC
	if (okc_info != NULL) {
		MFREE(wlc->osh, okc_info, sizeof(okc_info_t));
	}
#endif // endif
	return NULL;
}

void
BCMATTACHFN(wlc_okc_detach)(void *hdl)
{
#ifdef WL_OKC
	okc_info_t *okc_info = (okc_info_t *)hdl;
	wlc_info_t *wlc = okc_info->wlc;
#else
	wlc_info_t *wlc = (wlc_info_t *)hdl;
#endif // endif
	wlc_module_unregister(wlc->pub, "okc", hdl);
#ifdef WL_OKC
	MFREE(wlc->osh, okc_info, sizeof(okc_info_t));
#endif // endif
}
