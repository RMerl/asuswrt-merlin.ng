/*
 * IE management module Frame Type specific structures.
 *
 * Used to communicate between IE management module users (caller and callbacks).
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
 * $Id: wlc_ie_mgmt_ft.h 730443 2017-11-07 08:52:23Z $
 */

#ifndef _wlc_ie_mgmt_ft_h_
#define _wlc_ie_mgmt_ft_h_

#include <typedefs.h>
#include <bcmutils.h>
#include <wlc_types.h>
#include <wlioctl.h>
#include <bcmwifi_channels.h>

/*
 * 'calc_len' and 'build' parameters.
 *
 * Passed from the caller of either wlc_ieml_calc_len or wlc_ieml_build_frame
 * APIs to the registered 'calc_len' and/or 'build' callbacks.
 *
 * Please add any new fields at the end of each type struct.
 */
union wlc_iem_ft_cbparm {
	/* for auth calc/build */
	struct {
		int alg;		/* auth algo */
		int seq;		/* sequence # */
		struct scb *scb;	/* maybe NULL when 'status' isn't DOT11_SC_SUCCESS */
		uint8 *challenge;	/* challenge text for seq 3 when 'alg' is shared key */
		uint16 status;		/* Output: Status Code */
	} auth;
	/* for (re)assocreq calc/build */
	struct {
		scb_t *scb;
		wlc_bss_info_t *target;	/* Association Target */
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
		uint8 *md_ie;		/* Mobility Domain IE */
		uint8 *wpa_ie;		/* Output: WPA IE */
		uint8 *wpa2_ie;		/* Output: WPA2 IE */
		uint8 narrow_bw;	/* need drop current bw to narrow bw. */
	} assocreq;
	/* for (re)assocresp calc/build */
	struct {
		scb_t *scb;
		uint8 *mcs;		/* Preferred MCS */
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
		uint status;
#ifdef WLFBT
		uint8 *md_ie;		/* Mobility Domain IE */
		uint8 *wpa2_ie;		/* WPA2 IE */
#endif /* WLFBT */
	} assocresp;
	/* for prbreq calc/build */
	struct {
		uint8 *mcs;		/* Preferred MCS */
		const uint8 *ssid;
		uint8 ssid_len;
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
	} prbreq;
	/* for bcn/prbrsp calc/build */
	struct {
		uint8 *mcs;		/* Preferred MCS */
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
		uint8 *tim_ie;		/* Output: TIM IE */
	} bcn;
	/* for CS wrapper IE */
	struct {
		chanspec_t chspec;	/* new chanspec */
	} csw;
	/* for TDLS Setup frames */
	struct {
		scb_t *scb;
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
		uint8 *cap;	/* Extended Capabilities */
		chanspec_t chspec;	/* chanspec on which the STA-AP connection runs on */
		uint8 *ft_ie;	/* Output: FT IE pointer */
		uint8 action;
		bool ht_op_ie;
		bool vht_op_ie;
	} tdls;
	/* for TDLS Discovery frames */
	struct {
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
		uint8 *linkid_ie;	/* Link ID */
		uint8 *cap;	/* Extended Capabilities */
		uint8 ext_cap_len;	/* Extended Capabilities len */
	} disc;
	struct {
		int rde_count;	/* RDE IE count in RIC */
		int ts_count;	/* WME TSPEC IE count in RIC */
		uint8 *ts;	/* TSPEC list */
#ifdef WLFBT
		uint8 rde_id;   /* RDE Identifier from station */
		uint16 status;  /* status code for each TSPEC. AP Mode only */
#endif /* WLFBT */
	} fbtric;
};

/*
 * 'parse' parameters.
 *
 * Passed from the wlc_ieml_parse_frame API to the registered 'parse' callbacks.
 *
 * Please add any new fields at the end of each type struct.
 */
union wlc_iem_ft_pparm {
	/* for auth parse */
	struct {
		int alg;	/* auth algo */
		int seq;	/* sequence # */
		scb_t *scb;
		uint8 *challenge;	/* Output: Seq #2 Challenge text */
		uint16 status;	/* Output: Status Code */
	} auth;
	/* for (re)assocreq parse */
	struct {
		scb_t *scb;
		wlc_rateset_t *sup;	/* Supported Rates */
		wlc_rateset_t *ext;	/* Extended Supported Rates */
		uint8 *ht_cap_ie;	/* Output: HT Capability IE */
		uint8 *vht_cap_ie;	/* Output: VHT Capability IE */
		uint8 *vht_op_ie;	/* Output: VHT Operation IE */
		uint8 vht_ratemask;	/* Output: VHT BRCM Ratemask */
		uint8 *wps_ie;	/* Output: WPS IE */
		uint16 status;	/* Output: Status Code */
#ifdef WLFBT
		uint8 *md_ie;		/* Mobility Domain IE */
		uint8 *wpa2_ie;		/* WPA2 IE */
		uint8 *ft_ie;		/* FBT FT IE */
#endif /* WLFBT */
		wlc_supp_channels_t *supp_chan;	/* Supported Channels */
	} assocreq;
	/* for (re)assocresp parse */
	struct {
		scb_t *scb;
		uint16 status;	/* Output: Status Code */
		uint8 *md_ie;		/* Mobility Domain IE */
		uint8 *wpa2_ie;		/* WPA2 IE */
		uint8 *ft_ie;		/* FBT FT IE */
	} assocresp;
	/* for bcn parse in bcn proc (for Infra STA)
	 * when the bcn is from the associated AP
	 */
	struct {
		scb_t *scb;
		uint8 chan;	/* DS channel or rx channel */
		uint16 cap;
		bool erp;
	} bcn;
	/* for bcn/prbrsp parse during scan
	 * when the bcn/prbrsp is parsed by wlc_recv_parse_bcn_prb
	 */
	struct {
		wlc_bss_info_t *result;
		bool cap_bw_40;	/* Output: 40Mhz Capable */
		bool op_bw_any;	/* Output: Any Bandwidth */
		uint8 chan;	/* DS channel or rx channel */
	} scan;
	/* for TDLS Setup frames parse */
	struct {
		wlc_bss_info_t *result;
	} tdls;
};

#endif /* _wlc_ie_mgmt_ft_h_ */
