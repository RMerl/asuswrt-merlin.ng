/*
 * WLC OID module  of
 * Broadcom 802.11bang Networking Device Driver
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
 * $Id: wlc_hostoid.c 708017 2017-06-29 14:11:45Z $*
 *
 */

/**
 * @file
 * @brief
 * NDIS related feature. Moves OID processing code to host to save dongle RAM.
 */

/**
 * @file
 * @brief
 * XXX Twiki: [HostOidProcessing]
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>

#ifndef WLC_HOSTOID
#error "Cannot use this file without WLC_HOSTOID defined"
#endif /* WLC_HOSTOID */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <proto/802.11.h>
#include <d11.h>
#include <proto/eap.h>
#include <proto/eapol.h>
#include <wlc_rate.h>
#include <wlc_keymgmt.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wlc_phy_hal.h>
#include <wl_export.h>
#include <wlc_scan.h>
#include <wlc_hostoid.h>
#include <wlc_ap.h>
#include <bcmendian.h>
#include <wlc_assoc.h>
#ifdef BCMSUP_PSK
#include <wlc_sup.h>
#endif // endif
#include <wlc_pmkid.h>
#include <wlc_ht.h>
#include <wlc_prot_n.h>

static int wlc_hostoid_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif);
enum {
	IOV_DEFAULT_BSS_INFO,
#ifdef WLCNT
	IOV_STATISTICS,
#endif /* WLCNT */
	IOV_SET_NMODE,
	IOV_KEY_ABSENT,
	IOV_SCAN_IN_PROGRESS,
	IOV_BCN_PRB,
	IOV_PMKID_EVENT,
	IOV_PROT_N_MODE_RESET,
	IOV_LAST        /* In case of a need to check max ID number */
};

const bcm_iovar_t hostoid_iovars[] = {
	{"default_bss_info", IOV_DEFAULT_BSS_INFO,
	(0), IOVT_BUFFER, sizeof(wl_bss_config_t)
	},
#ifdef WLCNT
	{"statistics", IOV_STATISTICS,
	(0), IOVT_BUFFER, sizeof(wl_cnt_info_t)
	},
#endif /* WLCNT */
	{"set_nmode", IOV_SET_NMODE,
	(0), IOVT_UINT32, 0
	},
	{"key_absent", IOV_KEY_ABSENT,
	(0), IOVT_BOOL, 0
	},
	{"scan_in_progress", IOV_SCAN_IN_PROGRESS,
	(0), IOVT_BOOL, 0
	},
	{"bcn_prb", IOV_BCN_PRB,
	(0), IOVT_BUFFER, sizeof(int)
	},
	{"pmkid_event", IOV_PMKID_EVENT,
	(0), IOVT_UINT32, 0
	},
	{"prot_n_mode_reset", IOV_PROT_N_MODE_RESET,
	(0), IOVT_UINT32, 0
	},
	{NULL, 0, 0, 0, 0}
};

/* This includes the auto generated ROM IOCTL/IOVAR patch handler C source file (if auto patching is
 * enabled). It must be included after the prototypes and declarations above (since the generated
 * source file may reference private constants, types, variables, and functions).
 */
#include <wlc_patch.h>

int
BCMATTACHFN(wlc_hostoid_attach)(wlc_info_t *wlc)
{
	wlc_pub_t *pub = wlc->pub;
	if (wlc_module_register(pub, hostoid_iovars, "hostoid", wlc, wlc_hostoid_doiovar,
	                        NULL, NULL, NULL)) {
		WL_ERROR(("wl%d: hostoid wlc_module_register() failed\n", pub->unit));
		return BCME_ERROR;
	}

	return BCME_OK;
}

int
BCMATTACHFN(wlc_hostoid_detach)(wlc_info_t *wlc)
{
	wlc_module_unregister(wlc->pub, "hostoid", wlc);
	return BCME_OK;
}

static int
wlc_hostoid_doiovar(void *hdl, const bcm_iovar_t *vi, uint32 actionid, const char *name,
	void *params, uint p_len, void *arg, int len, int val_size, struct wlc_if *wlcif)
{
	wlc_info_t * wlc = (wlc_info_t *)hdl;
	int err = BCME_OK;
	int32 int_val = 0;
	wlc_bsscfg_t *bsscfg;
	int32 *ret_int_ptr;

	if ((err = wlc_iovar_check(wlc, vi, arg, len, IOV_ISSET(actionid), wlcif)) != 0)
		return err;
	bsscfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(bsscfg != NULL);

	if (p_len >= (int)sizeof(int_val))
		bcopy(params, &int_val, sizeof(int_val));

	ret_int_ptr = (int32 *)arg;

	switch (actionid) {

		case IOV_GVAL(IOV_DEFAULT_BSS_INFO): {
			wl_bss_config_t default_bss;
			bzero(&default_bss, sizeof(wl_bss_config_t));
			default_bss.beacon_period = (uint32)wlc->default_bss->beacon_period;
			default_bss.beacon_period = (uint32)wlc->default_bss->atim_window;
			default_bss.chanspec = (uint32)wlc->default_bss->chanspec;
			bcopy(&default_bss, (wl_bss_config_t *)arg, sizeof(wl_bss_config_t));
			break;
		}

		case IOV_SVAL(IOV_DEFAULT_BSS_INFO): {
			wl_bss_config_t *default_bss = (wl_bss_config_t *)arg;
			wlc->default_bss->beacon_period = (uint16)default_bss->beacon_period;
			wlc->default_bss->atim_window = (uint16)default_bss->atim_window;
			wlc->default_bss->chanspec = (chanspec_t)default_bss->chanspec;
			break;
		}

#ifdef WLCNT
		case IOV_GVAL(IOV_STATISTICS): {
			wlc_statsupd(wlc);
			err = wlc_get_all_cnts(wlc, arg, len);
			break;
		}
#endif /* WLCNT */

#ifdef WL11N
		case IOV_SVAL(IOV_SET_NMODE):
			err = wlc_set_nmode(wlc->hti, int_val);
			break;
#endif /* WL11N */

		case IOV_GVAL(IOV_KEY_ABSENT): {
			wlc_key_info_t key_info;
			wlc_keymgmt_get_key_by_addr(wlc->keymgmt,
				bsscfg, &bsscfg->BSSID, WLC_KEY_FLAG_NONE, &key_info);
			*ret_int_ptr = (key_info.algo == CRYPTO_ALGO_OFF);
			break;
		}

		case IOV_GVAL(IOV_SCAN_IN_PROGRESS): {
			*ret_int_ptr = ANY_SCAN_IN_PROGRESS(wlc->scan);
			break;
		}

		case IOV_GVAL(IOV_BCN_PRB):
			if ((uint)len < sizeof(uint32) + bsscfg->current_bss->bcn_prb_len) {
				return BCME_BUFTOOSHORT;
			}
			bcopy(&bsscfg->current_bss->bcn_prb_len, (char*)arg, sizeof(uint32));
			bcopy((char*)bsscfg->current_bss->bcn_prb, (char*)arg + sizeof(uint32),
				bsscfg->current_bss->bcn_prb_len);
#if defined(BCMDBG)
			printf("bcn_prb_len: %d\n", bsscfg->current_bss->bcn_prb_len);
			prhex("bcn_prb", (void*)bsscfg->current_bss->bcn_prb,
				bsscfg->current_bss->bcn_prb_len);
#endif /* BCMDBG */
			break;

		case IOV_SVAL(IOV_PMKID_EVENT): {
				wlc_pmkid_cache_req(wlc->pmkid_info, bsscfg);
			break;
		}

		case IOV_SVAL(IOV_PROT_N_MODE_RESET): {
			wlc_prot_n_mode_reset(wlc->prot_n, (bool)int_val);
			break;
		}

		default:
			err = BCME_UNSUPPORTED;
			break;
	}

	return err;
}
