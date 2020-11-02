/*
 * Broadcom 802.11 Networking Device Driver
 * Management Frame Protection (MFP)
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
 * $Id: wlc_mfp.h 766553 2018-08-07 05:25:24Z $
 *
 * This file provides the software interface to MFP functionality - first
 * defined as 802.11w and incorporated into IEEE 802.11/2012. The
 * functionality includes
 *		WLC Module Interface
 *			Attach/Detach
 *			MFP related IOVARs
 *		Integrity GTK (IGTK) support
 * 		Tx/Rx of protected management frames
 *		SA Query
 */

/**
 * XXX
 * Twiki: [ProtectedManagementFrames]
 */

#ifndef _wlc_mfp_h_
#define _wlc_mfp_h_

#ifdef MFP
#include <proto/802.11.h>
#include <proto/eapol.h>
#include <wlc_types.h>

/* whether frame kind is relevant to MFP */
#define IS_MFP_FC(fk) (fk == FC_DEAUTH || fk == FC_DISASSOC || fk == FC_ACTION)

/* Maximum number of SA query timesouts before disassociating */
#define WLC_MFP_SA_QUERY_MAX_TIMEOUTS 25

/* SA query timeout in milliseconds */
#define WLC_MFP_SA_QUERY_TIMEOUT_MS 200

/* association come back time interval - in TUs - must be greater
 * than SA query timeout
 * Add tiny margin over SA Query timeout and convert from ms to TU.
 * (Multiply millisec by 1000 for microsec and divide by 1024 to get TUs)
 * See IEEE 802.11 (2012) dot11AssociationSAQueryRetryTimeout
 * and dot11AssociationSAQueryMaximumTimeout
 */
#define WLC_MFP_COMEBACK_TIE_TU (((WLC_MFP_SA_QUERY_TIMEOUT_MS+1)*1000)>>10)

/* wlc module support */

wlc_mfp_info_t* wlc_mfp_attach(wlc_info_t *wlc);

void wlc_mfp_detach(wlc_mfp_info_t *mfp);

/* note: mfp uses igtk that is maintained by the keymgmt module */

/* extract igtk from eapol */
bool wlc_mfp_extract_igtk(const wlc_mfp_info_t *mfp, wlc_bsscfg_t *bsscfg,
	const eapol_header_t* eapol);

/* insert igtk in eapol and adjust data_len (offset in/out)
 * and return added length
 */
int wlc_mfp_insert_igtk(const wlc_mfp_info_t *mfp,
	const wlc_bsscfg_t *bsscfg, eapol_header_t *eapol, uint16 *data_len);

/* generate igtk for bss and return its length. master key is maintained
 * by the authenticator - used to seed the IGTK using PRF (sha256)
 */
uint16 wlc_mfp_gen_igtk(wlc_mfp_info_t *mfp, wlc_bsscfg_t *bsscfg,
	uint8* master_key, uint32 master_key_len);

/* sa query */

/* handle a received SA query - action is either a request or response */
void wlc_mfp_handle_sa_query(wlc_mfp_info_t *mfp, scb_t *scb,
	uint action_id, const struct dot11_management_header *hdr,
	const uint8 *body, int body_len);

/* start a SA query - used to validate disassoc, deauth */
void wlc_mfp_start_sa_query(wlc_mfp_info_t *mfp,
	const wlc_bsscfg_t *bsscfg, scb_t *scb);

/* Allocate a pkt and set MFP flag if necessary. Also handles
 * the case if BCMCCX and CCX_SDK are defined along with MFP
 */
void* wlc_mfp_frame_get_mgmt(wlc_mfp_info_t *mfp, uint16 fc,
	uint8 cat /* action only */,
    const struct ether_addr *da, const struct ether_addr *sa,
    const struct ether_addr *bssid, uint body_len, uint8 **pbody);

/* rx */

/* receive protected frame; return false to discard */
bool wlc_mfp_rx(wlc_mfp_info_t *mfp, const wlc_bsscfg_t *bsscfg,
	scb_t *scb, d11rxhdr_t *rxh,
	struct dot11_management_header *hdr, void *p);

/* misc utils */

/* check if mfp is needed based on rsn caps and wsec config. true if there
 * is no mismatch w/ mfp setting in enable_mfp
 */
bool wlc_mfp_check_rsn_caps(const wlc_mfp_info_t *mfp, wlc_bsscfg_t *cfg,
	uint16 rsn, bool *enable_mfp);

/* translate between wsec (config) bits and rsn caps */
uint8 wlc_mfp_get_rsn_caps(const wlc_mfp_info_t *mfp,  wlc_bsscfg_t *cfg);
uint8 wlc_mfp_rsn_caps_to_flags(const wlc_mfp_info_t *mfp, uint8 flags);

/* reset igtk(s) */
void wlc_mfp_reset_igtk(wlc_mfp_info_t *mfp, wlc_bsscfg_t *bsscfg);

int wlc_mfp_igtk_update(const wlc_mfp_info_t *mfp, wlc_bsscfg_t *bsscfg, int key_len,
	uint16 key_id, uint8 *pn, uint8 *key);

bool mfp_get_bip(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, wpa_suite_t *bip);

#endif /* MFP */
#endif	/* !_wlc_mfp_h_ */
