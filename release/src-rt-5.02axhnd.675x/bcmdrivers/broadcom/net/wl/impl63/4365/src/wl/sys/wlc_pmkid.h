/*
 * Exposed interfaces of wlc_pmkid.c
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
 * $Id: wlc_pmkid.h 708017 2017-06-29 14:11:45Z $
 */

/** Used for WPA(2) pre-authentication */

extern wlc_pmkid_info_t * wlc_pmkid_attach(wlc_info_t *wlc);
extern void wlc_pmkid_detach(wlc_pmkid_info_t *pmkid_info);

/* Gets called when RSN IE of assoc request has to be populated with PMKID
 * since the driver has PMKID store
 */
extern uint16
wlc_pmkid_putpmkid(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg,
	struct ether_addr *bssid, bcm_tlv_t *wpa2_ie, uint8 *fbt_pmkid, uint32 WPA_auth);

/* Clears currently stored PMKIDs both in PMKID module as well as supplicant */
extern void
wlc_pmkid_clear_store(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg);

/* identify candidate & add to the candidate list */
extern void
wlc_pmkid_prep_list(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg,
	struct ether_addr *bssid, uint8 wpa2_flags);

/* if candidate is part of supplicant, store PMKID as part of module main list
 * so that it can be used while preparing (re)assoc req
 */
extern void
wlc_pmkid_cache_req(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg);
#ifdef WL_OKC
extern void
wpa_calc_pmkid_for_okc(wlc_pmkid_info_t *pmkid_info, wlc_bsscfg_t *cfg, struct ether_addr *auth_ea,
	struct ether_addr *sta_ea, uint8 *pmk, uint pmk_len, uint8 *data,
	uint8 *digest, int *index);
#endif // endif
