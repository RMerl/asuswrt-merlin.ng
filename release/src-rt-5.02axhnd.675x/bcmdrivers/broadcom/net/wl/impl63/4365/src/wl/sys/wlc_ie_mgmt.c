/*
 * IE management module source - this is a top-level function
 * to manage building and/or parsing of IEs in the following
 * management frame types:
 *
 *	FC_ASSOC_REQ
 *	FC_ASSOC_RESP
 *	FC_REASSOC_REQ
 *	FC_REASSOC_RESP
 *	FC_PROBE_REQ
 *	FC_PROBE_RESP
 *	FC_BEACON
 *	FC_DISASSOC
 *	FC_AUTH
 *	FC_DEAUTH
 *
 * Building IEs in a frame should have been very simple by just calling registered
 * callbacks one by one, but unfortunately we must support user supplied IEs list
 * (was for assocreq frame only, but now is extended to all frame types, since we
 * are handling it 'generically' for all above frame types) as overrides, which adds
 * lots of complexity logically. The following details the handling of these IEs:
 *
 * 1. Non Vendor Specific IEs in the user supplied IEs list:
 *
 * - Copy IEs that don't have any registered callbacks in locations agreed by the
 *   user
 * - Copy IEs that have registered callbacks at relatively same position as the
 *   registered callbacks if the user decides to keep those IEs intact
 * - Pass IEs that have registered callbacks to the callbacks at relatively same
 *   position as the registered callbacks in case the callbacks want to use these
 *   IEs as templates to build their own versions if the user decides to modify
 *   these IEs
 *
 * 2. Vendor Specific IEs in the user supplied IEs list:
 *
 * - Query the user's callback for IEs' priorities
 * - Copy IEs that don't have any registered callbacks in locations agreed by the
 *   user
 * - Copy IEs that have registered callbacks at relatively same position as the
 *   registered callbacks
 *
 * Regardless if there is an user supplied IEs list or not the IEs' building process
 * is guided by the callbacks' order in the callback table. The registered callbacks
 * are invoked when there is no user supplied IEs list or there is no user supplied
 * IE found in the user supplied IEs list found for the registered callback's tag.
 * These callbacks can then write the expected IEs in the frame when invoked.
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
 * $Id: wlc_ie_mgmt.c 708017 2017-06-29 14:11:45Z $
 */

/* XXX: Define wlc_cfg.h to be the first header file included as some builds
 * get their feature flags thru this file.
 */
#include <wlc_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <wlioctl.h>
#include <d11.h>
#include <wlc_rate.h>
#include <wlc_pub.h>
#include <wlc_bsscfg.h>
#include <wlc.h>
#include <wl_dbg.h>
#include <wlc_ie_mgmt_lib.h>
#include <wlc_ie_mgmt_dbg.h>
#include <wlc_ie_mgmt.h>
#ifdef IEM_TEST
#include <wlc_ie_mgmt_test.h>
#endif // endif
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
#include <wlc_ie_mgmt_vs.h>
#endif // endif

/*
 * module info
 */
/* module private states - fixed portion */
struct wlc_iem_info {
	wlc_info_t *wlc;
	uint16 iem_info_len;		/* iem_info_t struct size, for MFREE */
	bool init;			/* module has been initialized */
	wlc_iem_uiel_t *uiel;
	wlc_iem_upp_t *upp;
	/* byte offsets into iem_info_t struct */
	uint16 build_cbs;		/* # 'calc_len'/'build' callbacks */
	uint16 build_tag_offset;	/* calc_len/build callback tables offset */
	uint16 build_cb_offset;
	uint16 build_ft_offset;		/* calc_len/build callback indices offset */
	uint16 vs_build_cbs;		/* # VS 'calc'/'build' callbacks */
	uint16 vs_build_prio_offset;	/* VS calc_len/build callback tables offset */
	uint16 vs_build_cb_offset;
	uint16 vs_build_ft_offset;	/* VS calc_len/build callback indices offset */
	uint16 parse_cbs;		/* # 'parse' callbacks */
	uint16 parse_tag_offset;	/* parse callback tables offset */
	uint16 parse_cb_offset;
	uint16 parse_ft_offset;		/* parse callback indices offset */
	uint16 vs_parse_cbs;		/* # 'VS parse' callbacks */
	uint16 vs_parse_id_offset;	/* VS parse callback tables offset */
	uint16 vs_parse_cb_offset;
	uint16 vs_parse_ft_offset;	/* VS parse callback indices offset */
};

/* module private states - entire structure */
/* typedef struct {
 *	wlc_iem_info_t iem -- module private states
 *	uint8 build_tag[max_ie_build_cbs] -- 'build' tag table
 *	wlc_iem_cbe_t build_cb[max_ie_build_cbs] -- 'build' callback table
 *	uint16 build_ft[IEM_NUM_FT + 1] - 'build' ft table
 *	uint8 vs_build_prio[max_vs_ie_build_cbs] -- 'vs build' prio table
 *	wlc_iem_cbe_t vs_build_cb[max_vs_ie_build_cbs] -- 'vs build' callback table
 *	uint16 vs_build_ft[IEM_NUM_FT + 1] - 'vs build' ft table
 *	uint8 parse_tag[max_ie_parse_cbs] -- 'parse' tag table
 *	wlc_iem_pe_t parse_cb[max_ie_parse_cbs] -- 'parse' callback table
 *	uint16 parse_ft[IEM_NUM_FT + 1] - 'parse' ft table
 *	uint8 vs_parse_id[max_vs_ie_parse_cbs] -- 'vs parse' id table
 *	wlc_iem_pe_t vs_parse_cb[max_vs_ie_parse_cbs] -- 'vs parse' callback table
 *	uint16 parse_ft[IEM_NUM_FT + 1] - 'vs parse' ft table
 * } wlc_iem_mem_t;
 * Notes:
 * - parallel tables
 *   parallel tables 'tag'/'prio' save a bit memory by eliminating padding
 * - 'build' and 'parse' tables relations
 *   ft table     tag table callback table
 *   +------+     +-------+ +-------+--
 *   | ft 0 | --> | tag a | | cb... |
 *   +------+     +-------+ +-------+--
 *   | ...  |     | ...   | |  ...  |
 *   +------+     +-------+ +-------+--
 *   | ...  |     | tab x | | cb... |
 *   +------+     +-------+ +-------+--
 *   | ft n | -   | ...   | |  ...  |
 *   +------+  |  +-------+ +-------+--
 *              > | tag A | | cb... |
 *                +-------+ +-------+--
 *                | ...   | |  ...  |
 *                +-------+ +-------+--
 *                | tag X | | cb... |
 *                +-------+ +-------+--
 * - 'vs build' related tables
 *   ft table     prio table callback table
 *   +------+     +--------+ +-------+--
 *   | ft 0 | --> | prio a | | cb... |
 *   +------+     +--------+ +-------+--
 *   | ...  |     | ...    | |  ...  |
 *   +------+     +--------+ +-------+--
 *   | ...  |     | prio x | | cb... |
 *   +------+     +--------+ +-------+--
 *   | ft n | -   | ...    | |  ...  |
 *   +------+  |  +--------+ +-------+--
 *              > | prio A | | cb... |
 *                +--------+ +-------+--
 *                | ...    | |  ...  |
 *                +--------+ +-------+--
 *                | prio X | | cb... |
 *                +--------+ +-------+--
 * - xxxx_ft[] size
 *   The tables are 1 size bigger than it needs to be, which allows us to compute
 *   the number of callbacks in table i by using xxxx_ft[i + 1] - xxxx_ft[i].
 */

/* table access macros */
#define BUILD_TAG_TBL(iem)	(uint8 *)((uintptr)(iem) + (iem)->build_tag_offset)
#define BUILD_CB_TBL(iem)	(wlc_iem_cbe_t *)((uintptr)(iem) + (iem)->build_cb_offset)
#define BUILD_FT_TBL(iem)	(uint16 *)((uintptr)(iem) + (iem)->build_ft_offset)
#define VS_BUILD_PRIO_TBL(iem)	(uint8 *)((uintptr)(iem) + (iem)->vs_build_prio_offset)
#define VS_BUILD_CB_TBL(iem)	(wlc_iem_cbe_t *)((uintptr)(iem) + (iem)->vs_build_cb_offset)
#define VS_BUILD_FT_TBL(iem)	(uint16 *)((uintptr)(iem) + (iem)->vs_build_ft_offset)
#define PARSE_TAG_TBL(iem)	(uint8 *)((uintptr)(iem) + (iem)->parse_tag_offset)
#define PARSE_CB_TBL(iem)	(wlc_iem_pe_t *)((uintptr)(iem) + (iem)->parse_cb_offset)
#define PARSE_FT_TBL(iem)	(uint16 *)((uintptr)(iem) + (iem)->parse_ft_offset)
#define VS_PARSE_ID_TBL(iem)	(uint8 *)((uintptr)(iem) + (iem)->vs_parse_id_offset)
#define VS_PARSE_CB_TBL(iem)	(wlc_iem_pe_t *)((uintptr)(iem) + (iem)->vs_parse_cb_offset)
#define VS_PARSE_FT_TBL(iem)	(uint16 *)((uintptr)(iem) + (iem)->vs_parse_ft_offset)

/* position callback entries for type 'ft' */
#define TBL_POS_GET(fttbl, ft, cur, next) {	\
		uint16 subtype = FT2FST(ft);	\
		ASSERT(subtype < IEM_NUM_FT);	\
		cur = (fttbl)[subtype];		\
		next = (fttbl)[subtype + 1];	\
	}

/* shift callback entries from 'next' (for type 'ft' + 1 onwards) down by 1 */
#define TBL_POS_SHIFT(ttbl, fntbl, cbs, next, fttbl, ft) {		\
		uint n;							\
		if ((next) < (cbs)) {					\
			uint mentries = (cbs) - (next);			\
			memmove(&(ttbl)[(next) + 1], &(ttbl)[next],	\
			        sizeof((ttbl)[0]) * mentries);		\
			memmove(&(fntbl)[(next) + 1], &(fntbl)[next],	\
			        sizeof((fntbl)[0]) * mentries);		\
		}							\
		for (n = FT2FST(ft) + 1; n < IEM_NUM_FT + 1; n ++) {	\
			(fttbl)[n] ++;					\
		}							\
	}

/*
 * IE tags tables
 */
/* Beacon - IEEE 802.11 spec Table 8-20 */
static const uint8 BCMINITDATA(ie_tags_bcn)[] = {
	/* order name id */
	/* 4 */	DOT11_MNG_SSID_ID,
	/* 5 */ DOT11_MNG_RATES_ID,
	/* 6 */ DOT11_MNG_FH_PARMS_ID,
	/* 7 */ DOT11_MNG_DS_PARMS_ID,
	/* 8 */ DOT11_MNG_CF_PARMS_ID,
	/* 9 */ DOT11_MNG_IBSS_PARMS_ID,
	/* 10 */ DOT11_MNG_TIM_ID,
	/* 11 */ DOT11_MNG_COUNTRY_ID,
	/* 12 */ DOT11_MNG_HOPPING_PARMS_ID,
	/* 13 */ DOT11_MNG_HOPPING_TABLE_ID,
	/* 14 */ DOT11_MNG_PWR_CONSTRAINT_ID,
	/* 15 */ DOT11_MNG_CHANNEL_SWITCH_ID,
	/* 16 */ DOT11_MNG_QUIET_ID,
	/* 17 */ DOT11_MNG_IBSS_DFS_ID,
	/* 18 */ DOT11_MNG_TPC_REPORT_ID,
	/* 19 */ DOT11_MNG_ERP_ID,
	/* 20 */ DOT11_MNG_EXT_RATES_ID,
	/* 21 */ DOT11_MNG_RSN_ID,
	/* 22 */ DOT11_MNG_QBSS_LOAD_ID,
	/* 23 */ DOT11_MNG_EDCA_PARAM_ID,
	/* 24 */ DOT11_MNG_QOS_CAP_ID,
	/* 25 */ DOT11_MNG_AP_CHREP_ID,
	/* 26 */ DOT11_MNG_BSS_AVR_ACCESS_DELAY_ID,
	/* 27 */ DOT11_MNG_ANTENNA_ID,
	/* 28 */ DOT11_MNG_BSS_AVAL_ADMISSION_CAP_ID,
#ifndef BCMWAPI_WAI
	/* 29 */ DOT11_MNG_BSS_AC_ACCESS_DELAY_ID,
#endif // endif
	/* 30 */ DOT11_MNG_MEASUREMENT_PILOT_TX_ID,
	/* 31 */ DOT11_MNG_MULTIPLE_BSSID_ID,
	/* 32 */ DOT11_MNG_RRM_CAP_ID,
	/* 33 */ DOT11_MNG_MDIE_ID,
	/* 34 */ /* DOT11_MNG_DSE_LOC_ID */ 58,
	/* 35 */ DOT11_MNG_EXT_CSA_ID,
	/* 36 */ DOT11_MNG_REGCLASS_ID,
	/* 37 */ DOT11_MNG_HT_CAP,
	/* 38 */ DOT11_MNG_HT_ADD,
	/* 39 */ DOT11_MNG_HT_BSS_COEXINFO_ID,
	/* 40 */ DOT11_MNG_HT_OBSS_ID,
	/* 41 */ DOT11_MNG_EXT_CAP_ID,
	/* 42 */ DOT11_MNG_FMS_DESCR_ID,
	/* 43 */ /* DOT11_MNG_QOS_TRF_CAP_ID */ 89,
	/* 44 */ DOT11_MNG_TIME_ADVERTISE_ID,
	/* 45 */ DOT11_MNG_INTERWORKING_ID,
	/* 46 */ DOT11_MNG_ADVERTISEMENT_ID,
	/* 47 */ DOT11_MNG_ROAM_CONSORT_ID,
	/* 48 */ /* DOT11_MNG_EAID_ID */ 112,
	/* 60 */ DOT11_MNG_VHT_CAP_ID,
	/* 61 */ DOT11_MNG_VHT_OPERATION_ID,
	/* 62 */ DOT11_MNG_VHT_TRANSMIT_POWER_ENVELOPE_ID,
	/* 63 */ DOT11_MNG_CHANNEL_SWITCH_WRAPPER_ID,
	/* 66 */ DOT11_MNG_OPER_MODE_NOTIF_ID,
#ifdef BCMWAPI_WAI
	/* XX */ DOT11_MNG_WAPI_ID,
#endif // endif
	/* Last */ DOT11_MNG_VS_ID
};

/* Probe Request - IEEE 802.11 spec Table 8-26 */
static const uint8 BCMINITDATA(ie_tags_prq)[] = {
	/* order name id */
	/* 1 */	DOT11_MNG_SSID_ID,
	/* 2 */ DOT11_MNG_RATES_ID,
	/* 3 */ DOT11_MNG_REQUEST_ID,
	/* 4 */ DOT11_MNG_EXT_RATES_ID,
	/* 5 */ DOT11_MNG_DS_PARMS_ID,
	/* 6 */ DOT11_MNG_REGCLASS_ID,
	/* 7 */ DOT11_MNG_HT_CAP,
	/* 8 */ DOT11_MNG_HT_BSS_COEXINFO_ID,
	/* 9 */ DOT11_MNG_EXT_CAP_ID,
	/* 10 */ /* DOT11_MNG_SSID_LIST_ID */ 84,
	/* 11 */ DOT11_MNG_CHANNEL_USAGE,
	/* 12 */ DOT11_MNG_INTERWORKING_ID,
	/* 17 */ DOT11_MNG_VHT_CAP_ID,
	/* Last */ DOT11_MNG_VS_ID
};

/* Probe Response - IEEE 802.11 spec Table 8-27 */
static const uint8 BCMINITDATA(ie_tags_prs)[] = {
	/* order name id */
	/* 4 */	DOT11_MNG_SSID_ID,
	/* 5 */ DOT11_MNG_RATES_ID,
	/* 6 */ DOT11_MNG_FH_PARMS_ID,
	/* 7 */ DOT11_MNG_DS_PARMS_ID,
	/* 8 */ DOT11_MNG_CF_PARMS_ID,
	/* 9 */ DOT11_MNG_IBSS_PARMS_ID,
	/* 10 */ DOT11_MNG_COUNTRY_ID,
	/* 11 */ DOT11_MNG_HOPPING_PARMS_ID,
	/* 12 */ DOT11_MNG_HOPPING_TABLE_ID,
	/* 13 */ DOT11_MNG_PWR_CONSTRAINT_ID,
	/* 14 */ DOT11_MNG_CHANNEL_SWITCH_ID,
	/* 15 */ DOT11_MNG_QUIET_ID,
	/* 16 */ DOT11_MNG_IBSS_DFS_ID,
	/* 17 */ DOT11_MNG_TPC_REPORT_ID,
	/* 18 */ DOT11_MNG_ERP_ID,
	/* 19 */ DOT11_MNG_EXT_RATES_ID,
	/* 20 */ DOT11_MNG_RSN_ID,
	/* 21 */ DOT11_MNG_QBSS_LOAD_ID,
	/* 22 */ DOT11_MNG_EDCA_PARAM_ID,
	/* 23 */ DOT11_MNG_MEASUREMENT_PILOT_TX_ID,
	/* 24 */ DOT11_MNG_MULTIPLE_BSSID_ID,
	/* 25 */ DOT11_MNG_RRM_CAP_ID,
	/* 26 */ DOT11_MNG_AP_CHREP_ID,
	/* 27 */ DOT11_MNG_BSS_AVR_ACCESS_DELAY_ID,
	/* 28 */ DOT11_MNG_ANTENNA_ID,
	/* 29 */ DOT11_MNG_BSS_AVAL_ADMISSION_CAP_ID,
#ifndef BCMWAPI_WAI
	/* 30 */ DOT11_MNG_BSS_AC_ACCESS_DELAY_ID,
#endif // endif
	/* 31 */ DOT11_MNG_MDIE_ID,
	/* 32 */ /* DOT11_MNG_DSE_LOC_ID */ 58,
	/* 33 */ DOT11_MNG_EXT_CSA_ID,
	/* 34 */ DOT11_MNG_REGCLASS_ID,
	/* 35 */ DOT11_MNG_HT_CAP,
	/* 36 */ DOT11_MNG_HT_ADD,
	/* 37 */ DOT11_MNG_HT_BSS_COEXINFO_ID,
	/* 38 */ DOT11_MNG_HT_OBSS_ID,
	/* 39 */ DOT11_MNG_EXT_CAP_ID,
	/* 40 */ /* DOT11_MNG_QOS_TRF_CAP_ID */ 89,
	/* 41 */ DOT11_MNG_CHANNEL_USAGE,
	/* 42 */ DOT11_MNG_TIME_ADVERTISE_ID,
	/* 43 */ DOT11_MNG_TIME_ZONE_ID,
	/* 44 */ DOT11_MNG_INTERWORKING_ID,
	/* 45 */ DOT11_MNG_ADVERTISEMENT_ID,
	/* 46 */ DOT11_MNG_ROAM_CONSORT_ID,
	/* 47 */ /* DOT11_MNG_EAID_ID */ 112,
	/* 60 */ DOT11_MNG_VHT_CAP_ID,
	/* 61 */ DOT11_MNG_VHT_OPERATION_ID,
	/* 62 */ DOT11_MNG_VHT_TRANSMIT_POWER_ENVELOPE_ID,
	/* 64 */ DOT11_MNG_CHANNEL_SWITCH_WRAPPER_ID,
	/* 67 */ DOT11_MNG_OPER_MODE_NOTIF_ID,
#ifdef BCMWAPI_WAI
	/* XX */ DOT11_MNG_WAPI_ID,
#endif // endif
	/* Last - 1 */ DOT11_MNG_VS_ID
	/* Last - n */ /* Requested elements */
};

/* Authentication - IEEE 802.11 spec Table 8-28 */
static const uint8 BCMINITDATA(ie_tags_auth)[] = {
	/* order name id */
	/* 4 */ DOT11_MNG_CHALLENGE_ID,
	/* 5 */ DOT11_MNG_RSN_ID,
	/* 6 */ DOT11_MNG_MDIE_ID,
	/* 7 */ DOT11_MNG_FTIE_ID,
	/* 8 */ DOT11_MNG_FT_TI_ID,
	/* 9 */ /* DOT11_MNG_RDE_ID */ 57,
	/* Last */ DOT11_MNG_VS_ID
};

/* De-authentication - IEEE 802.11 spec Table 8-30 */
static const uint8 BCMINITDATA(ie_tags_deauth)[] = {
	/* order name id */
	/* 2 - (Last - 1) */ DOT11_MNG_VS_ID
};

/* Association Request - IEEE 802.11 spec Table 8-22 */
static const uint8 BCMINITDATA(ie_tags_assocreq)[] = {
	/* order name id */
	/* 3 */	DOT11_MNG_SSID_ID,
	/* 4 */ DOT11_MNG_RATES_ID,
	/* 5 */ DOT11_MNG_EXT_RATES_ID,
	/* 6 */ DOT11_MNG_PWR_CAP_ID,
	/* 7 */ DOT11_MNG_SUPP_CHANNELS_ID,
	/* 8 */ DOT11_MNG_RSN_ID,
	/* 9 */ DOT11_MNG_QOS_CAP_ID,
	/* 10 */ DOT11_MNG_RRM_CAP_ID,
	/* 11 */ DOT11_MNG_MDIE_ID,
	/* 12 */ DOT11_MNG_REGCLASS_ID,
	/* 13 */ DOT11_MNG_HT_CAP,
	/* 14 */ DOT11_MNG_HT_BSS_COEXINFO_ID,
	/* 15 */ DOT11_MNG_EXT_CAP_ID,
	/* 16 */ /* DOT11_MNG_QOS_TRF_CAP_ID */ 89,
	/* 17 */ DOT11_MNG_TIMBC_REQ_ID,
	/* 18 */ DOT11_MNG_INTERWORKING_ID,
	/* 22 */ DOT11_MNG_VHT_CAP_ID,
	/* 23 */ DOT11_MNG_OPER_MODE_NOTIF_ID,
#ifdef BCMWAPI_WAI
	/* XX */ DOT11_MNG_WAPI_ID,
#endif // endif
#ifdef BCMCCX
	/* XX */ DOT11_MNG_AIRONET_ID,
#endif // endif
	/* Last */ DOT11_MNG_VS_ID,
};

/* Re-association Request - IEEE 802.11 spec Table 8-24 */
static const uint8 BCMINITDATA(ie_tags_reassocreq)[] = {
	/* order name id */
	/* 4 */	DOT11_MNG_SSID_ID,
	/* 5 */ DOT11_MNG_RATES_ID,
	/* 6 */ DOT11_MNG_EXT_RATES_ID,
	/* 7 */ DOT11_MNG_PWR_CAP_ID,
	/* 8 */ DOT11_MNG_SUPP_CHANNELS_ID,
	/* 9 */ DOT11_MNG_RSN_ID,
	/* 10 */ DOT11_MNG_QOS_CAP_ID,
	/* 11 */ DOT11_MNG_RRM_CAP_ID,
	/* 12 */ DOT11_MNG_MDIE_ID,
	/* 13 */ DOT11_MNG_FTIE_ID,
	/* 14 */ DOT11_MNG_RDE_ID,
	/* 15 */ DOT11_MNG_REGCLASS_ID,
	/* 16 */ DOT11_MNG_HT_CAP,
	/* 17 */ DOT11_MNG_HT_BSS_COEXINFO_ID,
	/* 18 */ DOT11_MNG_EXT_CAP_ID,
	/* 19 */ /* DOT11_MNG_QOS_TRF_CAP_ID */ 89,
	/* 20 */ DOT11_MNG_TIMBC_REQ_ID,
	/* 21 */ DOT11_MNG_FMS_REQ_ID,
	/* 22 */ DOT11_MNG_DMS_REQUEST_ID,
	/* 23 */ DOT11_MNG_INTERWORKING_ID,
	/* 28 */ DOT11_MNG_VHT_CAP_ID,
	/* 29 */ DOT11_MNG_OPER_MODE_NOTIF_ID,
#ifdef BCMWAPI_WAI
	/* XX */ DOT11_MNG_WAPI_ID,
#endif // endif
#ifdef BCMCCX
	/* XX */ DOT11_MNG_AIRONET_ID,
	/* XX */ DOT11_MNG_CCKM_REASSOC_ID,
#endif // endif
	/* Last */ DOT11_MNG_VS_ID,
};

/* Association Response - IEEE 802.11 spec Table 8-23 */
static const uint8 BCMINITDATA(ie_tags_assocresp)[] = {
	/* order name id */
	/* 4 */ DOT11_MNG_RATES_ID,
	/* 5 */ DOT11_MNG_EXT_RATES_ID,
	/* 6 */ DOT11_MNG_EDCA_PARAM_ID,
	/* 7 */ DOT11_MNG_RCPI_ID,
	/* 8 */ DOT11_MNG_RSNI_ID,
	/* 9 */ DOT11_MNG_RRM_CAP_ID,
	/* 10 */ DOT11_MNG_MDIE_ID,
	/* 11 */ DOT11_MNG_FTIE_ID,
	/* 12 */ /* DOT11_MNG_DSE_LOC_ID */ 58,
	/* 13 */ DOT11_MNG_FT_TI_ID,
	/* 14 */ DOT11_MNG_HT_CAP,
	/* 15 */ DOT11_MNG_HT_ADD,
	/* 16 */ DOT11_MNG_HT_BSS_COEXINFO_ID,
	/* 17 */ DOT11_MNG_HT_OBSS_ID,
	/* 18 */ DOT11_MNG_EXT_CAP_ID,
	/* 19 */ DOT11_MNG_BSS_MAX_IDLE_PERIOD_ID,
	/* 20 */ DOT11_MNG_TIMBC_RESP_ID,
	/* 21 */ DOT11_MNG_QOS_MAP_ID,
	/* 27 */ DOT11_MNG_VHT_CAP_ID,
	/* 28 */ DOT11_MNG_VHT_OPERATION_ID,
	/* 29 */ DOT11_MNG_OPER_MODE_NOTIF_ID,
#ifdef BCMWAPI_WAI
	/* XX */ DOT11_MNG_WAPI_ID,
#endif // endif
	/* Last */ DOT11_MNG_VS_ID
};

/* Re-association Response - IEEE 802.11 spec Table 8-25 */
static const uint8 BCMINITDATA(ie_tags_reassocresp)[] = {
	/* order name id */
	/* 4 */ DOT11_MNG_RATES_ID,
	/* 5 */ DOT11_MNG_EXT_RATES_ID,
	/* 6 */ DOT11_MNG_EDCA_PARAM_ID,
	/* 7 */ DOT11_MNG_RCPI_ID,
	/* 8 */ DOT11_MNG_RSNI_ID,
	/* 9 */ DOT11_MNG_RRM_CAP_ID,
	/* 10 */ DOT11_MNG_RSN_ID,
	/* 11 */ DOT11_MNG_MDIE_ID,
	/* 12 */ DOT11_MNG_FTIE_ID,
	/* 13 */ DOT11_MNG_RDE_ID,
	/* 14 */ /* DOT11_MNG_DSE_LOC_ID */ 58,
	/* 15 */ DOT11_MNG_FT_TI_ID,
	/* 16 */ DOT11_MNG_HT_CAP,
	/* 17 */ DOT11_MNG_HT_ADD,
	/* 18 */ DOT11_MNG_HT_BSS_COEXINFO_ID,
	/* 19 */ DOT11_MNG_HT_OBSS_ID,
	/* 20 */ DOT11_MNG_EXT_CAP_ID,
	/* 21 */ DOT11_MNG_BSS_MAX_IDLE_PERIOD_ID,
	/* 22 */ DOT11_MNG_TIMBC_RESP_ID,
	/* 23 */ DOT11_MNG_FMS_RESP_ID,
	/* 24 */ DOT11_MNG_DMS_RESPONSE_ID,
	/* 25 */ DOT11_MNG_QOS_MAP_ID,
	/* 31 */ DOT11_MNG_VHT_CAP_ID,
	/* 32 */ DOT11_MNG_VHT_OPERATION_ID,
	/* 33 */ DOT11_MNG_OPER_MODE_NOTIF_ID,
#ifdef BCMWAPI_WAI
	/* XX */ DOT11_MNG_WAPI_ID,
#endif // endif
	/* Last */ DOT11_MNG_VS_ID
};

/* Disassociation - IEEE 802.11 spec Table 8-21 */
static const uint8 BCMINITDATA(ie_tags_disassoc)[] = {
	/* order name id */
	/* 2 - (Last - 1) */ DOT11_MNG_VS_ID
};

#ifdef IEM_TEST
/* Test */
static const uint8 BCMINITDATA(ie_tags_test)[] = {
	/* order name id */
	/* 0 */	DOT11_MNG_SSID_ID,
	/* 1 */ DOT11_MNG_RATES_ID,
	/* 2 */ DOT11_MNG_EXT_RATES_ID,
	/* 3 */ DOT11_MNG_EXT_CAP_ID,
	/* Last */ DOT11_MNG_VS_ID
};
#endif /* IEM_TEST */

/*
 * # of Frame Types supported
 */
/* IE tags tables info - sorted by FC_XXXX */
typedef struct {
	const uint8 *tags;
	uint16 cnt;
} wlc_iem_tag_t;
static const wlc_iem_tag_t BCMINITDATA(ie_tags)[] = {
	/* FC_ASSOC_REQ	0 */ {ie_tags_assocreq, ARRAYSIZE(ie_tags_assocreq)},
	/* FC_ASSOC_RESP 1 */ {ie_tags_assocresp, ARRAYSIZE(ie_tags_assocresp)},
	/* FC_REASSOC_REQ 2 */ {ie_tags_reassocreq, ARRAYSIZE(ie_tags_reassocreq)},
	/* FC_REASSOC_RESP 3 */ {ie_tags_reassocresp, ARRAYSIZE(ie_tags_reassocresp)},
	/* FC_PROBE_REQ 4 */ {ie_tags_prq, ARRAYSIZE(ie_tags_prq)},
	/* FC_PROBE_RESP 5 */ {ie_tags_prs, ARRAYSIZE(ie_tags_prs)},
/* F */	/* WLC_IEM_FC_SCAN_PRBRSP 6 */ {ie_tags_prs, ARRAYSIZE(ie_tags_prs)},
/* F */	/* WLC_IEM_FC_SCAN_BCN 7 */ {ie_tags_bcn, ARRAYSIZE(ie_tags_bcn)},
	/* FC_BEACON 8 */ {ie_tags_bcn, ARRAYSIZE(ie_tags_bcn)},
/* F */	/* WLC_IEM_FC_AP_BCN 9 */ {ie_tags_bcn, ARRAYSIZE(ie_tags_bcn)},
	/* FC_DISASSOC 10 */ {ie_tags_disassoc, ARRAYSIZE(ie_tags_disassoc)},
	/* FC_AUTH 11 */ {ie_tags_auth, ARRAYSIZE(ie_tags_auth)},
	/* FC_DEAUTH 12 */ {ie_tags_deauth, ARRAYSIZE(ie_tags_deauth)},
#ifdef IEM_TEST
/* F */	/* WLC_IEM_FC_TEST 13 */ {ie_tags_test, ARRAYSIZE(ie_tags_test)},
#endif // endif
};
#define IEM_NUM_FT ARRAYSIZE(ie_tags)

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
/* IE tags tables names - sorted by FC_XXXX */
/* PLEASE KEEP THIS TABLE IN SYNC WITH ABOVE 'ie_tags' */
static const char *ie_tags_names[] = {
	"AssocReq",
	"AssocResp",
	"ReassocReq",
	"ReassocResp",
	"ProbeReq",
	"ProbeResp",
	"ScanPrbRsp",
	"ScanBcn",
	"Bcn",
	"APBcn",
	"Disassoc",
	"Auth",
	"Deauth",
#ifdef IEM_TEST
	"Test",
#endif // endif
};
#endif /* BCMDBG || BCMDBG_DUMP */

/*
 * IOVar table
 */
enum {
	IOV_IEM_DBG,
	IOV_IEM_TEST,
	IOV_LAST
};

#if defined(BCMDBG)
static const bcm_iovar_t wlc_iem_iovars[] = {
	{"iem_dbg", IOV_IEM_DBG, (0), IOVT_UINT32, 0},
	{"iem_test", IOV_IEM_TEST, (0), IOVT_UINT32, 0},
	{NULL, 0, 0, 0, 0}
};
#else
#define wlc_iem_iovars NULL
#endif // endif

/*
 * Debug
 */
#define WLCUNIT(iem) (iem)->wlc->pub->unit

/*
 * Local function declarations
 */
/* module entries */
#if defined(BCMDBG)
static int wlc_iem_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 aid, const char *name,
	void *p, uint plen, void *a, int alen, int vs, struct wlc_if *wlcif);
#else
#define wlc_iem_doiovar NULL
#endif // endif
static int wlc_iem_init(void *ctx);
#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int wlc_iem_dump(void *ctx, struct bcmstrbuf *b);
#endif // endif

/* Vendor Specific IE proxy */
static uint wlc_iem_vs_calc_len_cb(void *ctx, wlc_iem_calc_data_t *data);
static int wlc_iem_vs_build_cb(void *ctx, wlc_iem_build_data_t *data);
static int wlc_iem_vs_parse_cb(void *ctx, wlc_iem_parse_data_t *data);

/*
 * module attach/detach
 */
wlc_iem_info_t *
BCMATTACHFN(wlc_iem_attach)(wlc_info_t *wlc)
{
	wlc_iem_info_t *iem;
	uint16 len;
	uint fst;
	int cid;

	/* module private data length (fixed struct + variable strcuts) */
	len = (uint16)sizeof(wlc_iem_info_t);	/* fixed */
	len = (uint16)ALIGN_SIZE(len, sizeof(uint8));	/* build tag */
	IEM_ATTACH(("build_tag: %u\n", len));
	len += (uint16)(sizeof(uint8) * wlc->pub->tunables->max_ie_build_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* build cb */
	IEM_ATTACH(("build_cb: %u\n", len));
	len += (uint16)(sizeof(wlc_iem_cbe_t) * wlc->pub->tunables->max_ie_build_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint16));	/* build ft */
	IEM_ATTACH(("build_ft: %u\n", len));
	len += (uint16)(sizeof(uint16) * (IEM_NUM_FT + 1));
	len = (uint16)ALIGN_SIZE(len, sizeof(uint8));	/* vs build prio */
	IEM_ATTACH(("vs_build_prio: %u\n", len));
	len += (uint16)(sizeof(uint8) * wlc->pub->tunables->max_vs_ie_build_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* vs_build cb */
	IEM_ATTACH(("vs_build_cb: %u\n", len));
	len += (uint16)(sizeof(wlc_iem_cbe_t) * wlc->pub->tunables->max_vs_ie_build_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint16));	/* vs build ft */
	IEM_ATTACH(("vs_build_ft: %u\n", len));
	len += (uint16)(sizeof(uint16) * (IEM_NUM_FT + 1));
	len = (uint16)ALIGN_SIZE(len, sizeof(uint8));	/* parse tag */
	IEM_ATTACH(("parse_tag: %u\n", len));
	len += (uint16)(sizeof(uint8) * wlc->pub->tunables->max_ie_parse_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* parse cb */
	IEM_ATTACH(("parse_cb: %u\n", len));
	len += (uint16)(sizeof(wlc_iem_pe_t) * wlc->pub->tunables->max_ie_parse_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint16));	/* parse ft */
	IEM_ATTACH(("parse_ft: %u\n", len));
	len += (uint16)(sizeof(uint16) * (IEM_NUM_FT + 1));
	len = (uint16)ALIGN_SIZE(len, sizeof(uint8));	/* vs parse tag */
	IEM_ATTACH(("vs_parse_id: %u\n", len));
	len += (uint16)(sizeof(uint8) * wlc->pub->tunables->max_vs_ie_parse_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* vs parse cb */
	IEM_ATTACH(("vs_parse_cb: %u\n", len));
	len += (uint16)(sizeof(wlc_iem_pe_t) * wlc->pub->tunables->max_vs_ie_parse_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint16));	/* vs parse ft */
	IEM_ATTACH(("vs_parse_ft: %u\n", len));
	len += (uint16)(sizeof(uint16) * (IEM_NUM_FT + 1));

	/* allocate module private data */
	if ((iem = MALLOCZ(wlc->osh, len)) == NULL) {
		WL_ERROR(("wl%d: %s: out of mem, malloced %d bytes\n",
		          wlc->pub->unit, __FUNCTION__, MALLOCED(wlc->osh)));
		goto fail;
	}
	iem->wlc = wlc;
	iem->iem_info_len = len;

	/* suballocate variable size arrays */
	len = (uint16)sizeof(wlc_iem_info_t);	/* fixed */
	len = (uint16)ALIGN_SIZE(len, sizeof(uint8));	/* build tag */
	IEM_ATTACH(("build_tag: %u\n", len));
	iem->build_tag_offset = len;
	len += (uint16)(sizeof(uint8) * wlc->pub->tunables->max_ie_build_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* build cb */
	IEM_ATTACH(("build_cb: %u\n", len));
	iem->build_cb_offset = len;
	len += (uint16)(sizeof(wlc_iem_cbe_t) * wlc->pub->tunables->max_ie_build_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint16));	/* build ft */
	IEM_ATTACH(("build_ft: %u\n", len));
	iem->build_ft_offset = len;
	len += (uint16)(sizeof(uint16) * (IEM_NUM_FT + 1));
	len = (uint16)ALIGN_SIZE(len, sizeof(uint8));	/* vs build prio */
	IEM_ATTACH(("vs_build_prio: %u\n", len));
	iem->vs_build_prio_offset = len;
	len += (uint16)(sizeof(uint8) * wlc->pub->tunables->max_vs_ie_build_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* vs build cb */
	IEM_ATTACH(("vs_build_cb: %u\n", len));
	iem->vs_build_cb_offset = len;
	len += (uint16)(sizeof(wlc_iem_cbe_t) * wlc->pub->tunables->max_vs_ie_build_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint16));	/* vs build ft */
	IEM_ATTACH(("vs_build_ft: %u\n", len));
	iem->vs_build_ft_offset = len;
	len += (uint16)(sizeof(uint16) * (IEM_NUM_FT + 1));
	len = (uint16)ALIGN_SIZE(len, sizeof(uint8));	/* parse tag */
	IEM_ATTACH(("parse_tag: %u\n", len));
	iem->parse_tag_offset = len;
	len += (uint16)(sizeof(uint8) * wlc->pub->tunables->max_ie_parse_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* parse cb */
	IEM_ATTACH(("parse_cb: %u\n", len));
	iem->parse_cb_offset = len;
	len += (uint16)(sizeof(wlc_iem_pe_t) * wlc->pub->tunables->max_ie_parse_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint16));	/* parse ft */
	IEM_ATTACH(("parse_ft: %u\n", len));
	iem->parse_ft_offset = len;
	len += (uint16)(sizeof(uint16) * (IEM_NUM_FT + 1));
	len = (uint16)ALIGN_SIZE(len, sizeof(uint8));	/* vs parse tag */
	IEM_ATTACH(("vs_parse_id: %u\n", len));
	iem->vs_parse_id_offset = len;
	len += (uint16)(sizeof(uint8) * wlc->pub->tunables->max_vs_ie_parse_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(void *));	/* vs parse cb */
	IEM_ATTACH(("vs_parse_cb: %u\n", len));
	iem->vs_parse_cb_offset = len;
	len += (uint16)(sizeof(wlc_iem_pe_t) * wlc->pub->tunables->max_vs_ie_parse_cbs);
	len = (uint16)ALIGN_SIZE(len, sizeof(uint16));	/* vs parse ft */
	IEM_ATTACH(("vs_parse_ft: %u\n", len));
	iem->vs_parse_ft_offset = len;
	len += (uint16)(sizeof(uint16) * (IEM_NUM_FT + 1));

	/* sanity check */
	ASSERT(len == iem->iem_info_len);

	/* register Vendor Specific IE calc_len/build proxy */
	for (fst = 0; fst < IEM_NUM_FT; fst ++) {
		if (ie_tags[fst].tags == NULL)
			continue;
		if ((cid = wlc_iem_add_build_fn(iem, FST2FT(fst), DOT11_MNG_VS_ID,
		            wlc_iem_vs_calc_len_cb, wlc_iem_vs_build_cb, iem)) < 0) {
			WL_ERROR(("wl%d: %s: wlc_iem_add_build_fn failed, fst %d err %d\n",
			          wlc->pub->unit, __FUNCTION__, fst, cid));
			goto fail;
		}
	}

	/* register Vendor Specific IE parse proxy */
	for (fst = 0; fst < IEM_NUM_FT; fst ++) {
		if (ie_tags[fst].tags == NULL)
			continue;
		if ((cid = wlc_iem_add_parse_fn(iem, FST2FT(fst), DOT11_MNG_VS_ID,
		            wlc_iem_vs_parse_cb, iem)) < 0) {
			WL_ERROR(("wl%d: %s: wlc_iem_add_parse_fn failed, fst %d err %d\n",
			          wlc->pub->unit, __FUNCTION__, fst, cid));
			goto fail;
		}
	}

#ifdef IEM_TEST
	if (wlc_iem_test_register_fns(iem) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_iem_test_register_fns failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	}
#endif // endif

	/* register a wlc_init callback to signal that
	 * the client callback registration should be blocked...
	 */
	if (wlc_module_register(wlc->pub, wlc_iem_iovars, "iem", iem,
	                        wlc_iem_doiovar,
	                        NULL, wlc_iem_init, NULL) != BCME_OK) {
		WL_ERROR(("wl%d: %s: wlc_module_register() failed\n",
		          wlc->pub->unit, __FUNCTION__));
		goto fail;
	};

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
	/* register dump routine */
	wlc_dump_register(wlc->pub, "iem", wlc_iem_dump, (void *)iem);
	wlc_dump_register(wlc->pub, "iem_vs", wlc_iem_vs_dump, (void *)iem);
#endif // endif

	return iem;

fail:
	wlc_iem_detach(iem);
	return NULL;
}

void
BCMATTACHFN(wlc_iem_detach)(wlc_iem_info_t *iem)
{
	wlc_info_t *wlc;
	uint16 len;

	if (iem == NULL)
		return;

	wlc = iem->wlc;
	len = iem->iem_info_len;

	wlc_module_unregister(wlc->pub, "iem", iem);

	MFREE(wlc->osh, iem, len);
}

#if defined(BCMDBG)
static int
wlc_iem_doiovar(void *ctx, const bcm_iovar_t *vi, uint32 aid, const char *name,
	void *p, uint plen, void *a, int alen, int vs, struct wlc_if *wlcif)
{
	wlc_iem_info_t *iem = (wlc_iem_info_t *)ctx;
	wlc_info_t *wlc = iem->wlc;
	int32 int_val = 0;
	int err = BCME_OK;
	wlc_bsscfg_t *cfg;

	if (plen >= (int)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	cfg = wlc_bsscfg_find_by_wlcif(wlc, wlcif);
	ASSERT(cfg != NULL);

	switch (aid) {
	case IOV_SVAL(IOV_IEM_DBG):
		iem_msg_level = (uint)int_val;
		break;
#ifdef IEM_TEST
	case IOV_SVAL(IOV_IEM_TEST): {
		uint saved_msglevel = iem_msg_level;
		iem_msg_level = -1;
		switch (int_val) {
		case 0:
			err = wlc_iem_test_build_frame(wlc, cfg);
			break;
		case 1:
			err = wlc_iem_test_parse_frame(wlc, cfg);
			break;
		}
		iem_msg_level = saved_msglevel;
		break;
	}
#endif /* IEM_TEST */
	default:
		err = BCME_UNSUPPORTED;
		break;
	}

	return err;
}
#endif /* BCMDBG */

/* Sort all calc_len/build callback table entries */
static int
BCMINITFN(wlc_iem_sort_cbtbl)(wlc_iem_info_t *iem)
{
	uint8 *build_tag = BUILD_TAG_TBL(iem);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(iem);
	uint16 *build_ft = BUILD_FT_TBL(iem);
	uint16 fst;

	/* Note: on dongle, wlc_iem_sort_cbtbl and some tables
	 * may not be available after wl init sequence...
	 */
	if (iem->init)
		return BCME_UNSUPPORTED;

	ASSERT(build_ft != NULL);
	ASSERT(build_tag != NULL);
	ASSERT(build_cb != NULL);

	/* sort for all frame types */
	for (fst = 0; fst < IEM_NUM_FT; fst ++) {
		uint16 cur, next;
		uint16 ft;
		uint8 *build_tag1;
		wlc_iem_cbe_t *build_cb1;

		ft = FST2FT(fst);
		TBL_POS_GET(build_ft, ft, cur, next);

		IEM_TRACE(("wl%d: %s: ft 0x%04x, cur %u, next %u\n",
		           WLCUNIT(iem), __FUNCTION__, ft, cur, next));

		/* early bailout */
		if (next - cur == 0)
			continue;

		/* point to the beginning of the tables */
		build_tag1 = &build_tag[cur];
		build_cb1 = &build_cb[cur];

		/* sort based on the ie_tags table */
		wlc_ieml_sort_cbtbl(build_tag1, build_cb1, next - cur,
		                    ie_tags[fst].tags, ie_tags[fst].cnt);
	}

	return BCME_OK;
}

/* wlc_init callback */
static int
BCMINITFN(wlc_iem_init)(void *ctx)
{
	wlc_iem_info_t *iem = (wlc_iem_info_t *)ctx;

	/* do the rest only for the first time of bringing up the driver */
	if (iem->init) {
		WL_INFORM(("wl%d: %s: wl is already up..\n", WLCUNIT(iem), __FUNCTION__));
		return BCME_OK;
	}

	/* sort build_cb_tbl following tags' order in by IE tags table */
	wlc_iem_sort_cbtbl(iem);

	iem->init = TRUE;

	return BCME_OK;
}

#if defined(BCMDBG) || defined(BCMDBG_DUMP)
static int
wlc_iem_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_iem_info_t *iem = (wlc_iem_info_t *)ctx;
	wlc_info_t *wlc = iem->wlc;
	uint16 cur, next;
	uint16 fst, ft;
	uint16 i;

	/* table offsets */
	bcm_bprintf(b, "build_tag_offset %u build_cb_offset %u build_ft_offset %u\n",
	            iem->build_tag_offset, iem->build_cb_offset, iem->build_ft_offset);
	bcm_bprintf(b, "vs_build_prio_offset %u vs_build_cb_offset %u vs_build_ft_offset %u\n",
	            iem->vs_build_prio_offset, iem->vs_build_cb_offset, iem->vs_build_ft_offset);
	bcm_bprintf(b, "parse_tag_offset %u parse_cb_offset %u parse_ft_offset %u\n",
	            iem->parse_tag_offset, iem->parse_cb_offset, iem->parse_ft_offset);
	bcm_bprintf(b, "vs_parse_id_offset %u vs_parse_cb_offset %u vs_parse_ft_offset %u\n",
	            iem->vs_parse_id_offset, iem->vs_parse_cb_offset, iem->vs_parse_ft_offset);

	/* states */
	bcm_bprintf(b, "init: %u\n", iem->init);

	/* tables */
#ifndef DONGLEBUILD
	bcm_bprintf(b, "tags tables: %u\n", IEM_NUM_FT);
	for (fst = 0; fst < IEM_NUM_FT; fst ++) {
		uint16 ti;

		if (ie_tags[fst].tags == NULL)
			continue;

		ft = FST2FT(fst);

		bcm_bprintf(b, "  frame type: 0x%04x(%s), total tags: %u\n",
		            ft, ie_tags_names[fst], ie_tags[fst].cnt);

		for (ti = 0; ti < ie_tags[fst].cnt; ti ++) {
			bcm_bprintf(b, "    idx %u: tag %u\n", ti, ie_tags[fst].tags[ti]);
		}
	}
#endif /* DONGLEBUILD */
	bcm_bprintf(b, "build callbacks: %u of %u\n",
	            iem->build_cbs, wlc->pub->tunables->max_ie_build_cbs);
	for (fst = 0; fst < IEM_NUM_FT; fst ++) {
		uint8 *build_tag = BUILD_TAG_TBL(iem);
		wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(iem);
		uint16 *build_ft = BUILD_FT_TBL(iem);

#ifndef DONGLEBUILD
		if (ie_tags[fst].tags == NULL)
			continue;
#endif // endif

		ft = FST2FT(fst);
		TBL_POS_GET(build_ft, ft, cur, next);

		bcm_bprintf(b, "  frame type: 0x%04x(%s), total callbacks %u\n",
		            ft, ie_tags_names[fst], next - cur);

		for (i = cur; i < next; i ++) {
			bcm_bprintf(b, "    idx %u(%u): tag %u, calc %p, build %p, ctx %p\n",
			            i - cur, i, build_tag[i],
			            build_cb[i].calc, build_cb[i].build, build_cb[i].ctx);
		}
	}
	bcm_bprintf(b, "vs build callbacks: %u of %u\n",
	            iem->vs_build_cbs, wlc->pub->tunables->max_vs_ie_build_cbs);
	for (fst = 0; fst < IEM_NUM_FT; fst ++) {
		uint8 *build_prio = VS_BUILD_PRIO_TBL(iem);
		wlc_iem_cbe_t *build_cb = VS_BUILD_CB_TBL(iem);
		uint16 *build_ft = VS_BUILD_FT_TBL(iem);

#ifndef DONGLEBUILD
		if (ie_tags[fst].tags == NULL)
			continue;
#endif // endif

		ft = FST2FT(fst);
		TBL_POS_GET(build_ft, ft, cur, next);

		bcm_bprintf(b, "  frame type: 0x%04x(%s), total callbacks %u\n",
		            ft, ie_tags_names[fst], next - cur);

		for (i = cur; i < next; i ++) {
			bcm_bprintf(b, "    idx %u(%u): prio %u, calc %p, build %p, ctx %p\n",
			            i - cur, i, build_prio[i],
			            build_cb[i].calc, build_cb[i].build, build_cb[i].ctx);
		}
	}
	bcm_bprintf(b, "parse callbacks: %u of %u\n",
	            iem->parse_cbs, wlc->pub->tunables->max_ie_parse_cbs);
	for (fst = 0; fst < IEM_NUM_FT; fst ++) {
		uint8 *parse_tag = PARSE_TAG_TBL(iem);
		wlc_iem_pe_t *parse_cb = PARSE_CB_TBL(iem);
		uint16 *parse_ft = PARSE_FT_TBL(iem);

#ifndef DONGLEBUILD
		if (ie_tags[fst].tags == NULL)
			continue;
#endif // endif

		ft = FST2FT(fst);
		TBL_POS_GET(parse_ft, ft, cur, next);

		bcm_bprintf(b, "  frame type: 0x%04x(%s), total callbacks %u\n",
		            ft, ie_tags_names[fst], next - cur);

		for (i = cur; i < next; i ++) {
			bcm_bprintf(b, "    idx %u(%u): tag %u, parse %p, ctx %p\n",
			            i - cur, i, parse_tag[i],
			            parse_cb[i].parse, parse_cb[i].ctx);
		}
	}
	bcm_bprintf(b, "vs parse callbacks: %u of %u\n",
	            iem->vs_parse_cbs, wlc->pub->tunables->max_vs_ie_parse_cbs);
	for (fst = 0; fst < IEM_NUM_FT; fst ++) {
		uint8 *parse_id = VS_PARSE_ID_TBL(iem);
		wlc_iem_pe_t *parse_cb = VS_PARSE_CB_TBL(iem);
		uint16 *parse_ft = VS_PARSE_FT_TBL(iem);

#ifndef DONGLEBUILD
		if (ie_tags[fst].tags == NULL)
			continue;
#endif // endif

		ft = FST2FT(fst);
		TBL_POS_GET(parse_ft, ft, cur, next);

		bcm_bprintf(b, "  frame type: 0x%04x(%s), total callbacks %u\n",
		            ft, ie_tags_names[fst], next - cur);

		for (i = cur; i < next; i ++) {
			bcm_bprintf(b, "    idx %u(%u): tag %u, parse %p, ctx %p\n",
			            i - cur, i, parse_id[i],
			            parse_cb[i].parse, parse_cb[i].ctx);
		}
	}

	return BCME_OK;
}
#endif /* BCMDBG || BCMDBG_DUMP */

/* 'calc_len//build' callback pair registration - for non Vendor Specific IE
 * with tag 'tag'
 */
int
BCMATTACHFN(wlc_iem_add_build_fn)(wlc_iem_info_t *iem, uint16 ft, uint8 tag,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx)
{
	wlc_info_t *wlc = iem->wlc;
	uint8 *build_tag = BUILD_TAG_TBL(iem);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(iem);
	uint16 *build_ft = BUILD_FT_TBL(iem);
	uint16 cur, next;
	uint16 fst;
	uint i;

	fst = FT2FST(ft);
	ASSERT(fst < IEM_NUM_FT);

	if (iem->build_cbs >= wlc->pub->tunables->max_ie_build_cbs) {
		WL_ERROR(("wl%d: %s: too many entries\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NORESOURCE;
	}

	ASSERT(calc_fn != NULL);
	ASSERT(build_fn != NULL);

	IEM_TRACE(("wl%d: %s: ft 0x%x, tag %u, calc %p, build %p, ctx %p\n",
	           WLCUNIT(iem), __FUNCTION__, ft, tag, calc_fn, build_fn, ctx));

	/* make sure we know about the tag */
	for (i = 0; i < ie_tags[fst].cnt; i ++) {
		if (ie_tags[fst].tags[i] == tag)
			break;
	}
	if (i == ie_tags[fst].cnt) {
		WL_ERROR(("wl%d: %s: tag %u not found\n", wlc->pub->unit, __FUNCTION__, tag));
		return BCME_NOTFOUND;
	}

	ASSERT(build_ft != NULL);
	ASSERT(build_tag != NULL);
	ASSERT(build_cb != NULL);

	/* get the frame type specific table position */
	TBL_POS_GET(build_ft, ft, cur, next);

	IEM_TRACE(("wl%d: %s: ttl %u, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, iem->build_cbs, cur, next));

	/* move the entries of other frame types down by 1 */
	TBL_POS_SHIFT(build_tag, build_cb, iem->build_cbs, next, build_ft, ft);

	/* point to the beginning of the frame type specific table */
	build_tag = &build_tag[cur];
	build_cb = &build_cb[cur];

	wlc_ieml_add_build_fn(build_tag, build_cb, next - cur, calc_fn, build_fn, ctx, tag);

	iem->build_cbs ++;

	return BCME_OK;
}

/* 'calc_len//build' callback pair registration - for non Vendor Specific IE
 * with tag 'tag' and for multiple frame types.
 */
int
BCMATTACHFN(wlc_iem_add_build_fn_mft)(wlc_iem_info_t *iem, uint16 fstbmp, uint8 tag,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx)
{
	uint16 fst;
	int err = BCME_OK;

	for (fst = 0; fst < IEM_NUM_FT; fst ++) {
		if (!(fstbmp & (1 << fst)))
			continue;
		if ((err = wlc_iem_add_build_fn(iem, FST2FT(fst), tag,
		                                calc_fn, build_fn, ctx)) != BCME_OK)
			break;
	}

	return err;
}

/* 'calc_len/build' callback registration for Vendor Specific IE with priority 'prio' */
int
BCMATTACHFN(wlc_iem_vs_add_build_fn)(wlc_iem_info_t *iem, uint16 ft, uint8 prio,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx)
{
	wlc_info_t *wlc = iem->wlc;
	uint8 *build_prio = VS_BUILD_PRIO_TBL(iem);
	wlc_iem_cbe_t *build_cb = VS_BUILD_CB_TBL(iem);
	uint16 *build_ft = VS_BUILD_FT_TBL(iem);
	uint16 cur, next;

	if (iem->vs_build_cbs >= wlc->pub->tunables->max_vs_ie_build_cbs) {
		WL_ERROR(("wl%d: %s: too many entries\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NORESOURCE;
	}

	ASSERT(calc_fn != NULL);
	ASSERT(build_fn != NULL);

	IEM_TRACE(("wl%d: %s: ft 0x%x, prio %u, calc %p, build %p, ctx %p\n",
	           WLCUNIT(iem), __FUNCTION__, ft, prio, calc_fn, build_fn, ctx));

	ASSERT(build_ft != NULL);
	ASSERT(build_prio != NULL);
	ASSERT(build_cb != NULL);

	/* get the frame type specific table's position */
	TBL_POS_GET(build_ft, ft, cur, next);

	IEM_TRACE(("wl%d: %s: ttl %u, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, iem->vs_build_cbs, cur, next));

	/* move the entries of other frame types down by 1 */
	TBL_POS_SHIFT(build_prio, build_cb, iem->vs_build_cbs, next, build_ft, ft);

	/* point to the beginning of the table */
	build_prio = &build_prio[cur];
	build_cb = &build_cb[cur];

	wlc_ieml_add_build_fn(build_prio, build_cb, next - cur, calc_fn, build_fn, ctx, prio);

	iem->vs_build_cbs ++;

	return BCME_OK;
}

/* 'calc_len//build' callback pair registration - for non Vendor Specific IE
 * with tag 'tag' and for multiple frame types.
 */
int
BCMATTACHFN(wlc_iem_vs_add_build_fn_mft)(wlc_iem_info_t *iem, uint16 fstbmp, uint8 prio,
	wlc_iem_calc_fn_t calc_fn, wlc_iem_build_fn_t build_fn, void *ctx)
{
	uint16 fst;
	int err = BCME_OK;

	for (fst = 0; fst < IEM_NUM_FT; fst ++) {
		if (!(fstbmp & (1 << fst)))
			continue;
		if ((err = wlc_iem_vs_add_build_fn(iem, FST2FT(fst), prio,
		                                   calc_fn, build_fn, ctx)) != BCME_OK)
			break;
	}

	return err;
}

/* 'parse' callback registration for non Vendor Specific IE with tag 'tag' */
int
BCMATTACHFN(wlc_iem_add_parse_fn)(wlc_iem_info_t *iem, uint16 ft, uint8 tag,
	wlc_iem_parse_fn_t parse_fn, void *ctx)
{
	wlc_info_t *wlc = iem->wlc;
	uint8 *parse_tag = PARSE_TAG_TBL(iem);
	wlc_iem_pe_t *parse_cb = PARSE_CB_TBL(iem);
	uint16 *parse_ft = PARSE_FT_TBL(iem);
	uint16 cur, next;

	if (iem->parse_cbs >= wlc->pub->tunables->max_ie_parse_cbs) {
		WL_ERROR(("wl%d: %s: too many entries\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NORESOURCE;
	}

	ASSERT(parse_fn != NULL);

	IEM_TRACE(("wl%d: %s: ft 0x%x, tag %u, parse %p, ctx %p\n",
	           WLCUNIT(iem), __FUNCTION__, ft, tag, parse_fn, ctx));

	ASSERT(parse_ft != NULL);
	ASSERT(parse_tag != NULL);
	ASSERT(parse_cb != NULL);

	/* get the frame type specific table's position */
	TBL_POS_GET(parse_ft, ft, cur, next);

	IEM_TRACE(("wl%d: %s: ttl %u, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, iem->parse_cbs, cur, next));

	/* move the entries of other frame types down by 1 */
	TBL_POS_SHIFT(parse_tag, parse_cb, iem->parse_cbs, next, parse_ft, ft);

	/* point to the beginning of the table */
	parse_tag = &parse_tag[cur];
	parse_cb = &parse_cb[cur];

	wlc_ieml_add_parse_fn(parse_tag, parse_cb, next - cur, parse_fn, ctx, tag);

	iem->parse_cbs ++;

	return BCME_OK;
}

/* 'parse' callback registration for non Vendor Specific IE with tag 'tag'
 * for multiple frame types
 */
int
BCMATTACHFN(wlc_iem_add_parse_fn_mft)(wlc_iem_info_t *iem, uint16 fstbmp, uint8 tag,
	wlc_iem_parse_fn_t parse_fn, void *ctx)
{
	uint16 fst;
	int err = BCME_OK;

	for (fst = 0; fst < IEM_NUM_FT; fst ++) {
		if (!(fstbmp & (1 << fst)))
			continue;
		if ((err = wlc_iem_add_parse_fn(iem, FST2FT(fst), tag,
		                                parse_fn, ctx)) != BCME_OK)
			break;
	}

	return err;
}

/* 'parse' callback registration for Vendor Specific IE with ID 'id' */
int
BCMATTACHFN(wlc_iem_vs_add_parse_fn)(wlc_iem_info_t *iem, uint16 ft, uint8 id,
	wlc_iem_parse_fn_t parse_fn, void *ctx)
{
	wlc_info_t *wlc = iem->wlc;
	uint8 *parse_id = VS_PARSE_ID_TBL(iem);
	wlc_iem_pe_t *parse_cb = VS_PARSE_CB_TBL(iem);
	uint16 *parse_ft = VS_PARSE_FT_TBL(iem);
	uint16 cur, next;

	if (iem->vs_parse_cbs >= wlc->pub->tunables->max_vs_ie_parse_cbs) {
		WL_ERROR(("wl%d: %s: too many entries\n", wlc->pub->unit, __FUNCTION__));
		return BCME_NORESOURCE;
	}

	ASSERT(parse_fn != NULL);

	IEM_TRACE(("wl%d: %s: ft 0x%x, id %u, parse %p, ctx %p\n",
	           WLCUNIT(iem), __FUNCTION__, ft, id, parse_fn, ctx));

	ASSERT(parse_ft != NULL);
	ASSERT(parse_id != NULL);
	ASSERT(parse_cb != NULL);

	/* get the frame specific table's position */
	TBL_POS_GET(parse_ft, ft, cur, next);

	IEM_TRACE(("wl%d: %s: ttl %u, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, iem->vs_parse_cbs, cur, next));

	/* move the entries of other frame types down by 1 */
	TBL_POS_SHIFT(parse_id, parse_cb, iem->vs_parse_cbs, next, parse_ft, ft);

	/* point to the beginning of the table */
	parse_id = &parse_id[cur];
	parse_cb = &parse_cb[cur];

	wlc_ieml_add_parse_fn(parse_id, parse_cb, next - cur, parse_fn, ctx, id);

	iem->vs_parse_cbs ++;

	return BCME_OK;
}

/* 'parse' callback registration for Vendor Specific IE with ID 'id'
 * for multiple frame types.
 */
int
BCMATTACHFN(wlc_iem_vs_add_parse_fn_mft)(wlc_iem_info_t *iem, uint16 fstbmp, uint8 id,
	wlc_iem_parse_fn_t parse_fn, void *ctx)
{
	uint16 fst;
	int err = BCME_OK;

	for (fst = 0; fst < IEM_NUM_FT; fst ++) {
		if (!(fstbmp & (1 << fst)))
			continue;
		if ((err = wlc_iem_vs_add_parse_fn(iem, FST2FT(fst), id,
		                                   parse_fn, ctx)) != BCME_OK)
			break;
	}

	return err;
}

/* Calculate IEs' length in a frame */
uint
wlc_iem_calc_len(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm)
{
	uint8 *build_tag = BUILD_TAG_TBL(iem);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(iem);
	uint16 *build_ft = BUILD_FT_TBL(iem);
	uint16 cur, next;
	uint len;

	ASSERT(build_ft != NULL);
	ASSERT(build_tag != NULL);
	ASSERT(build_cb != NULL);

	TBL_POS_GET(build_ft, ft, cur, next);

	IEM_TRACE(("wl%d: %s: ft 0x%04x, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, ft, cur, next));

	build_tag = &build_tag[cur];
	build_cb = &build_cb[cur];

	/* Pass it to Vendor Specific IE proxy */
	ASSERT(iem->uiel == NULL);
	iem->uiel = uiel;

	len = wlc_ieml_calc_len(cfg, ft, build_tag, TRUE, build_cb, next - cur,
	                        uiel, cbparm);

	iem->uiel = NULL;

	return len;
}

/*
 * Calculate a particular non Vendor Specific IE's length in a frame.
 *
 * Inputs:
 * - ft: Frame type as defined by FC_XXXX in 802.11.h
 * - tag: non Vendor Specific IE's tag
 * - cbparm: Callback parameters
 *
 * Outputs:
 * - len: IE's length in bytes
 *
 * A negative return value indicates an error (BCME_XXXX).
 */
uint
wlc_iem_calc_ie_len(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	uint8 tag, wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm)
{
	uint8 *build_tag = BUILD_TAG_TBL(iem);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(iem);
	uint16 *build_ft = BUILD_FT_TBL(iem);
	uint16 cur, next;

	ASSERT(build_ft != NULL);
	ASSERT(build_tag != NULL);
	ASSERT(build_cb != NULL);

	TBL_POS_GET(build_ft, ft, cur, next);

	IEM_TRACE(("wl%d: %s: ft 0x%04x, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, ft, cur, next));

	build_tag = &build_tag[cur];
	build_cb = &build_cb[cur];

	return wlc_ieml_calc_ie_len(cfg, ft, build_tag, TRUE, build_cb, next - cur,
	                            tag, uiel, cbparm);
}

/*
 * Calculate a particular Vendor Specific IE's length in a frame.
 *
 * Inputs:
 * - ft: Frame type as defined by FC_XXXX in 802.11.h
 * - tag: non Vendor Specific IE's tag
 * - cbparm: Callback parameters
 *
 * Outputs:
 * - len: IE's length in bytes
 *
 * A negative return value indicates an error (BCME_XXXX).
 */
uint
wlc_iem_vs_calc_ie_len(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	uint8 prio, wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm)
{
	uint8 *build_prio = VS_BUILD_PRIO_TBL(iem);
	wlc_iem_cbe_t *build_cb = VS_BUILD_CB_TBL(iem);
	uint16 *build_ft = VS_BUILD_FT_TBL(iem);
	uint16 cur, next;

	ASSERT(build_ft != NULL);
	ASSERT(build_prio != NULL);
	ASSERT(build_cb != NULL);

	TBL_POS_GET(build_ft, ft, cur, next);

	IEM_TRACE(("wl%d: %s: ft 0x%04x, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, ft, cur, next));

	build_prio = &build_prio[cur];
	build_cb = &build_cb[cur];

	return wlc_ieml_calc_ie_len(cfg, ft, build_prio, FALSE, build_cb, next - cur,
	                            prio, uiel, cbparm);
}

/* Write IEs in a frame */
int
wlc_iem_build_frame(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm, uint8 *buf, uint buf_len)
{
	uint8 *build_tag = BUILD_TAG_TBL(iem);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(iem);
	uint16 *build_ft = BUILD_FT_TBL(iem);
	uint16 cur, next;
	int err;

	ASSERT(build_ft != NULL);
	ASSERT(build_tag != NULL);
	ASSERT(build_cb != NULL);

	TBL_POS_GET(build_ft, ft, cur, next);

	IEM_TRACE(("wl%d: %s: ft 0x%04x, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, ft, cur, next));

	build_tag = &build_tag[cur];
	build_cb = &build_cb[cur];

	/* Pass it to Vendor Specific IE proxy */
	ASSERT(iem->uiel == NULL);
	iem->uiel = uiel;

	err = wlc_ieml_build_frame(cfg, ft, build_tag, TRUE, build_cb, next - cur,
	                           uiel, cbparm, buf, buf_len);

	iem->uiel = NULL;

	return err;
}

/*
 * Write a particular non Vendor Specific IE in a frame.
 *
 * Inputs:
 * - ft: Frame type as defined by FC_XXXX in 802.11.h
 * - tag: non Vendor Specific IE's tag
 * - cbparm: Callback parameters
 * - buf: buffer pointer
 * - buf_len: buffer length
 *
 * Outputs:
 * - buf: IE written in the buffer
 *
 * A negative return value indicates an error (BCME_XXXX).
 */
int
wlc_iem_build_ie(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	uint8 tag, wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm, uint8 *buf, uint buf_len)
{
	uint8 *build_tag = BUILD_TAG_TBL(iem);
	wlc_iem_cbe_t *build_cb = BUILD_CB_TBL(iem);
	uint16 *build_ft = BUILD_FT_TBL(iem);
	uint16 cur, next;

	ASSERT(build_ft != NULL);
	ASSERT(build_tag != NULL);
	ASSERT(build_cb != NULL);

	TBL_POS_GET(build_ft, ft, cur, next);

	IEM_TRACE(("wl%d: %s: ft 0x%04x, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, ft, cur, next));

	build_tag = &build_tag[cur];
	build_cb = &build_cb[cur];

	return wlc_ieml_build_ie(cfg, ft, build_tag, TRUE, build_cb, next - cur,
	                         tag, uiel, cbparm, buf, buf_len);
}

/*
 * Write a particular Vendor Specific IE in a frame.
 *
 * Inputs:
 * - ft: Frame type as defined by FC_XXXX in 802.11.h
 * - prio: Vendor Specific IE's prio/id as defined by wlc_ie_mgmt_vs.h
 * - cbparm: Callback parameters
 * - buf: buffer pointer
 * - buf_len: buffer length
 *
 * Outputs:
 * - buf: IE written in the buffer
 *
 * A negative return value indicates an error (BCME_XXXX).
 */
int
wlc_iem_vs_build_ie(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	uint8 prio, wlc_iem_uiel_t *uiel, wlc_iem_cbparm_t *cbparm, uint8 *buf, uint buf_len)
{
	uint8 *build_prio = VS_BUILD_PRIO_TBL(iem);
	wlc_iem_cbe_t *build_cb = VS_BUILD_CB_TBL(iem);
	uint16 *build_ft = VS_BUILD_FT_TBL(iem);
	uint16 cur, next;

	ASSERT(build_ft != NULL);
	ASSERT(build_prio != NULL);
	ASSERT(build_cb != NULL);

	TBL_POS_GET(build_ft, ft, cur, next);

	IEM_TRACE(("wl%d: %s: ft 0x%04x, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, ft, cur, next));

	build_prio = &build_prio[cur];
	build_cb = &build_cb[cur];

	return wlc_ieml_build_ie(cfg, ft, build_prio, FALSE, build_cb, next - cur,
	                         prio, uiel, cbparm, buf, buf_len);
}

/* Calculate Vendor Specific IEs length in a frame */
static uint
wlc_iem_vs_calc_len_cb(void *ctx, wlc_iem_calc_data_t *data)
{
	wlc_iem_info_t *iem = (wlc_iem_info_t *)ctx;
	uint8 *build_prio = VS_BUILD_PRIO_TBL(iem);
	wlc_iem_cbe_t *build_cb = VS_BUILD_CB_TBL(iem);
	uint16 *build_ft = VS_BUILD_FT_TBL(iem);
	uint16 cur, next;

	TBL_POS_GET(build_ft, data->ft, cur, next);

	IEM_TRACE(("wl%d: %s: ft 0x%04x, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, data->ft, cur, next));

	build_prio = &build_prio[cur];
	build_cb = &build_cb[cur];

	return wlc_ieml_calc_len(data->cfg, data->ft, build_prio, FALSE, build_cb, next - cur,
	                         iem->uiel, data->cbparm);
}

/* Build Vendor Specific IEs in a frame */
static int
wlc_iem_vs_build_cb(void *ctx, wlc_iem_build_data_t *data)
{
	wlc_iem_info_t *iem = (wlc_iem_info_t *)ctx;
	uint8 *build_prio = VS_BUILD_PRIO_TBL(iem);
	wlc_iem_cbe_t *build_cb = VS_BUILD_CB_TBL(iem);
	uint16 *build_ft = VS_BUILD_FT_TBL(iem);
	uint16 cur, next;

	TBL_POS_GET(build_ft, data->ft, cur, next);

	IEM_TRACE(("wl%d: %s: ft 0x%04x, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, data->ft, cur, next));

	build_prio = &build_prio[cur];
	build_cb = &build_cb[cur];

	return wlc_ieml_build_frame(data->cfg, data->ft, build_prio, FALSE, build_cb, next - cur,
	                            iem->uiel, data->cbparm, data->buf, data->buf_len);
}

/* Traverse IEs in 'buf' and invoke callbacks registered for these IEs */
int
wlc_iem_parse_frame(wlc_iem_info_t *iem, wlc_bsscfg_t *cfg, uint16 ft,
	wlc_iem_upp_t *upp, wlc_iem_pparm_t *pparm, uint8 *buf, uint buf_len)
{
	uint8 *parse_tag = PARSE_TAG_TBL(iem);
	wlc_iem_pe_t *parse_cb = PARSE_CB_TBL(iem);
	uint16 *parse_ft = PARSE_FT_TBL(iem);
	uint16 cur, next;
	int err;

	ASSERT(parse_ft != NULL);
	ASSERT(parse_tag != NULL);
	ASSERT(parse_cb != NULL);

	TBL_POS_GET(parse_ft, ft, cur, next);

	IEM_TRACE(("wl%d: %s: ft 0x%04x, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, ft, cur, next));

	parse_tag = &parse_tag[cur];
	parse_cb = &parse_cb[cur];

	/* Pass it to Vendor Specific IE proxy */
	ASSERT(iem->upp == NULL);
	iem->upp = upp;

	err = wlc_ieml_parse_frame(cfg, ft, parse_tag, TRUE, parse_cb, next - cur,
	                           upp, pparm, buf, buf_len);

	iem->upp = NULL;

	return err;
}

/* Parse Vendor Specific IE pointed by data->ie pointer */
static int
wlc_iem_vs_parse_cb(void *ctx, wlc_iem_parse_data_t *data)
{
	wlc_iem_info_t *iem = (wlc_iem_info_t *)ctx;
	uint8 *parse_id = VS_PARSE_ID_TBL(iem);
	wlc_iem_pe_t *parse_cb = VS_PARSE_CB_TBL(iem);
	uint16 *parse_ft = VS_PARSE_FT_TBL(iem);
	uint16 cur, next;
	uint8 *buf;
	uint buf_len;

	ASSERT(parse_ft != NULL);
	ASSERT(parse_id != NULL);
	ASSERT(parse_cb != NULL);

	TBL_POS_GET(parse_ft, data->ft, cur, next);

	IEM_TRACE(("wl%d: %s: ft 0x%04x, cur %u, next %u\n",
	           WLCUNIT(iem), __FUNCTION__, data->ft, cur, next));

	parse_id = &parse_id[cur];
	parse_cb = &parse_cb[cur];

	/* let's parse all IEs of the same tag at once starting from this one */
	if (data->ie != NULL) {
		buf = data->ie;
		buf_len = data->buf_len - (uint)(buf - data->buf);
	}
	else {
		buf = NULL;
		buf_len = 0;
	}

	return wlc_ieml_parse_frame(data->cfg, data->ft, parse_id, FALSE, parse_cb, next - cur,
	                            iem->upp, data->pparm, buf, buf_len);
}
