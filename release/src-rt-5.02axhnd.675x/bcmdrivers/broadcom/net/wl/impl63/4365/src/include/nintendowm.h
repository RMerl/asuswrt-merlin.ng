/*
 *
 *   _____________________________________________________________________
 *  |                                                                     |
 *  |  Copyright (C) 2003 Mitsumi electric co,. ltd. All Rights Reserved. |
 *  |  This software contains the valuable trade secrets of Mitsumi.      |
 *  |  The software is protected under copyright laws as an unpublished   |
 *  |  work of Mitsumi. Notice is for informational purposes only and     |
 *  |  does not imply publication. The user of this software may make     |
 *  |  copies of the software for use with parts manufactured by Mitsumi  |
 *  |  or under license from Mitsumi and for no other use.                |
 *  |_____________________________________________________________________|
 *
 *
 */

/*
 * Nintendo library functions header file
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
 *
 * $Id: nintendowm.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef	_NINTENDOWM_H
#define _NINTENDOWM_H

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif // endif

/* This marks the start of a packed structure section. */
#include <packed_section_start.h>

/*
 *  Confirm calculators
 *
 */
#define	NWM_RSV	(12/2)
#define	CalcReqMsgLength(_x_)	((sizeof(_x_) - NWM_RSV*2 - sizeof(nwm_cmdhdr_t) + 1) / 2)
#define	CalcCfmMsgLength(_x_)	((sizeof(_x_) -  sizeof(nwm_cmdhdr_t) + 1) / 2)
#define	CalcIndMsgLength(_x_)	((sizeof(_x_) -  NWM_RSV*2 - sizeof(nwm_cmdhdr_t) + 1) / 2)
#define	CalcIndMsgLength2(_x_, _y_)	((sizeof(_x_) -  \
			NWM_RSV*2 - sizeof(nwm_cmdhdr_t) + _y_ + 1) / 2)
#define	WL_CalcHeaderLength(_len_)	((_len_+1)/2)
#define	WL_CalcConfirmPointer(_p_)	((uintptr)_p_ + \
		((nwm_cmd_req_t*)_p_)->header.length*2 + 12+4)

#define  NWM_CMD_RESERVED_CFM_MINSIZE       1
#define	NWM_CMD_RESVD_REQ_MINSIZE			0

#define NWM_MLME_RESET_REQ_MINSIZE	CalcReqMsgLength(nwm_mlme_reset_req_t)
#define NWM_MLME_SCAN_REQ_MINSIZE   CalcReqMsgLength(nwm_mlme_scan_req_t)
#define NWM_MLME_JOIN_REQ_MINSIZE   CalcReqMsgLength(nwm_mlme_join_req_t)
#define NWM_MLME_START_REQ_MINSIZE	CalcReqMsgLength(nwm_mlme_startreq_t)
#define NWM_MLME_DISASSOC_REQ_MINSIZE	CalcReqMsgLength(nwm_mlme_disassoc_req_t)
#define NWM_MLME_MEASCHAN_REQ_MINSIZE	CalcReqMsgLength(nwm_mlme_measchan_req_t)

#define NWM_MLME_RESET_CFM_MINSIZE	CalcCfmMsgLength(nwm_mlme_reset_cfm_t)
#define NWM_MLME_SCAN_CFM_MINSIZE	CalcCfmMsgLength(nwm_mlme_scan_cfm_t)
#define NWM_MLME_JOIN_CFM_MINSIZE	CalcCfmMsgLength(nwm_mlme_join_cfm_t)
#define NWM_MLME_START_CFM_MINSIZE	CalcCfmMsgLength(nwm_mlme_startcfm_t)
#define NWM_MLME_DISASSOC_CFM_MINSIZE	CalcCfmMsgLength(nwm_mlme_disassoc_cfm_t)
#define NWM_MLME_MEASCHAN_CFM_MINSIZE	CalcCfmMsgLength(nwm_mlme_measchan_cfm_t)

/* MA */
#define	MA_DATA_REQ_MINSIZE			CalcReqMsgLength(nwm_ma_data_req_t)
#define NWM_MA_KEYDATA_REQ_MINSIZE	CalcReqMsgLength(nwm_ma_keydata_req_t)
/* zero length data frames are ok */
#define NWM_MA_MP_REQ_MINSIZE	    (CalcReqMsgLength(nwm_ma_mp_req_t) - 1)
#define NWM_MA_TESTDATA_REQ_MINSIZE	CalcReqMsgLength(nwm_ma_testdata_req_t)
#define NWM_MA_CLRDATA_REQ_MINSIZE	CalcReqMsgLength(nwm_ma_clrdata_req_t)

#define NWM_MA_CLRDATA_CFM_MINSIZE	CalcCfmMsgLength(nwm_ma_clrdata_cfm_t)

/* PARAMSET */
#define NWM_SET_ALL_REQ_MINSIZE		CalcReqMsgLength(nwm_set_all_req_t)
#define NWM_SET_MACADRS_REQ_MINSIZE	CalcReqMsgLength(nwm_set_macadrs_req_t)
#define NWM_SET_RETRY_REQ_MINSIZE	CalcReqMsgLength(nwm_set_retrylimit_req_t)
#define NWM_SET_ENCH_REQ_MINSIZE	CalcReqMsgLength(nwm_set_enablechannel_req_t)
#define NWM_SET_MODE_REQ_MINSIZE	CalcReqMsgLength(nwm_set_mode_req_t)
#define NWM_SET_RATESET_REQ_MINSIZE	CalcReqMsgLength(nwm_set_rateset_req_t)
#define NWM_SET_WEPMODE_REQ_MINSIZE	CalcReqMsgLength(nwm_set_wepmode_req_t)
#define NWM_SET_WEPKEYID_REQ_MINSIZE	CalcReqMsgLength(nwm_set_wepkeyid_req_t)
#define NWM_SET_WEPKEY_REQ_MINSIZE	CalcReqMsgLength(nwm_set_wepkey_req_t)
#define NWM_SET_BCN_TYPE_REQ_MINSIZE	CalcReqMsgLength(nwm_set_beacontype_req_t)
#define NWM_SET_RES_BC_SSID_REQ_MINSIZE	CalcReqMsgLength(nwm_set_proberes_req_t)
#define NWM_SET_BCN_LOST_REQ_MINSIZE	CalcReqMsgLength(nwm_set_beaconlostth_req_t)
#define NWM_SET_ACT_ZONE_REQ_MINSIZE	CalcReqMsgLength(nwm_set_activezone_req_t)
#define NWM_SET_SSID_MASK_REQ_MINSIZE	CalcReqMsgLength(nwm_set_ssidmask_req_t)
#define NWM_SET_PREAMBLE_TYPE_REQ_MINSIZE	CalcReqMsgLength(nwm_set_preambletype_req_t)
#define NWM_SET_AUTHALGO_REQ_MINSIZE	CalcReqMsgLength(nwm_set_authalgo_req_t)
#define NWM_SET_CCA_EDTH_REQ_MINSIZE	CalcReqMsgLength(nwm_set_cca_mode_eeth_req_t)
#define NWM_SET_LIFE_TIME_REQ_MINSIZE	CalcReqMsgLength(nwm_set_lifetime_req_t)
#define NWM_SET_MAX_CONN_REQ_MINSIZE	CalcReqMsgLength(nwm_set_maxconn_req_t)
#define NWM_SET_TXANT_REQ_MINSIZE		CalcReqMsgLength(nwm_set_txant_req_t)
#define NWM_SET_DIVERSITY_REQ_MINSIZE	CalcReqMsgLength(nwm_set_diversity_req_t)
#define NWM_SET_BCNTXRXIND_REQ_MINSIZE	CalcReqMsgLength(nwm_set_beaconsendrecvind_req_t)
#define NWM_SET_INTERFERENCE_REQ_MINSIZE		CalcReqMsgLength(nwm_set_interference_req_t)

#define NWM_SET_BSSID_REQ_MINSIZE		CalcReqMsgLength(nwm_set_bssid_req_t)
#define NWM_SET_BCN_PERIOD_REQ_MINSIZE	CalcReqMsgLength(nwm_set_beaconperiod_req_t)
#define NWM_SET_DTIM_PERIOD_REQ_MINSIZE	CalcReqMsgLength(nwm_set_dtimperiod_req_t)
#define NWM_SET_GAME_INFO_REQ_MINSIZE   CalcReqMsgLength(nwm_set_gameinfo_req_t)
#define NWM_SET_VBLANK_TSF_REQ_MINSIZE	(CalcReqMsgLength(nwm_set_vblanktsf_req_t))
#define NWM_SET_MACLIST_REQ_MINSIZE		(CalcReqMsgLength(nwm_set_maclist_req_t))
#define NWM_SET_RTSTHRESH_REQ_MINSIZE	(CalcReqMsgLength(nwm_set_rtsthresh_req_t))
#define NWM_SET_FRAGTHRESH_REQ_MINSIZE	(CalcReqMsgLength(nwm_set_fragthresh_req_t))
#define NWM_SET_PMK_KEY_REQ_MINSIZE		(CalcReqMsgLength(nwm_set_pmk_req_t))
#define NWM_SET_EEROM_REQ_MINSIZE		(CalcReqMsgLength(nwm_set_eerom_req_t))
#define NWM_SET_TXPWR_REQ_MINSIZE		(CalcReqMsgLength(nwm_set_txpwr_req_t))
#define NWM_SET_MCAST_REQ_MINSIZE		(CalcReqMsgLength(nwm_set_mcast_rate_req_t))

#define NWM_SET_ALL_CFM_MINSIZE			CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_MACADRS_CFM_MINSIZE		CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_RETRY_CFM_MINSIZE		CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_ENCH_CFM_MINSIZE		CalcCfmMsgLength(nwm_set_enablechannel_cfm_t)
#define NWM_SET_MODE_CFM_MINSIZE		CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_RATESET_CFM_MINSIZE		CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_WEPMODE_CFM_MINSIZE		CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_WEPKEYID_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_WEPKEY_CFM_MINSIZE		CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_BCN_TYPE_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_RES_BC_SSID_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_BCN_LOST_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_ACT_ZONE_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_SSID_MASK_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_PREAMBLE_TYPE_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_AUTHALGO_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_CCA_EDTH_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_LIFE_TIME_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)

#define NWM_SET_BSSID_CFM_MINSIZE		CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_SSID_CFM_MINSIZE		CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_BCN_PERIOD_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_DTIM_PERIOD_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_LSN_INT_CFM_MINSIZE		CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_GAME_INFO_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_VBLANK_TSF_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_MACLIST_CFM_MINSIZE	    CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_RTSTHRESH_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_FRAGTHRESH_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_PMK_CFM_MINSIZE	        CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_EEROM_CFM_MINSIZE		CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_MAX_CONN_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_TXPWR_CFM_MINSIZE		CalcCfmMsgLength(nwm_set_cfm_t)
#define NWM_SET_MCAST_CFM_MINSIZE		CalcCfmMsgLength(nwm_set_cfm_t)
#define	NWM_PARAMSET_TXANT_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define	NWM_PARAMSET_DIVERSITY_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define	NWM_PARAMSET_BCNTXRXIND_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)
#define	NWM_PARAMSET_INTERFERENCE_CFM_MINSIZE	CalcCfmMsgLength(nwm_set_cfm_t)

/* PARAMGET */
#define NWM_GET_ALL_REQ_MINSIZE			CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_MACADRS_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_RETRY_REQ_MINSIZE			CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_ENCH_REQ_MINSIZE			CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_MODE_REQ_MINSIZE			CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_RATESET_REQ_MINSIZE			CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_WEPMODE_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_WEPKEYID_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_BCN_TYPE_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_RES_BC_SSID_REQ_MINSIZE	CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_BCN_LOST_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_ACT_ZONE_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_SSID_MASK_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_PREAMBLE_TYPE_REQ_MINSIZE	CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_AUTHALGO_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_CCA_EDTH_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_MAX_CONN_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define	PARAMGET_TXANT_REQ_MINSIZE			CalcReqMsgLength(nwm_get_req_t)
#define	PARAMGET_DIVERSITY_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_BCNTXRXIND_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define	PARAMGET_INTERFERENCE_REQ_MINSIZE			CalcReqMsgLength(nwm_get_req_t)

#define NWM_GET_BSSID_REQ_MINSIZE			CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_SSID_REQ_MINSIZE			CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_BCN_PERIOD_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_DTIM_PERIOD_REQ_MINSIZE	CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_LSN_INT_REQ_MINSIZE		CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_GAME_INFO_REQ_MINSIZE	    CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_VBLANK_TSF_REQ_MINSIZE	    CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_MACLIST_REQ_MINSIZE	    CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_RTSTHRESH_REQ_MINSIZE	    CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_FRAGTHRESH_REQ_MINSIZE	    CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_EEROM_REQ_MINSIZE	        CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_RSSI_REQ_MINSIZE	        CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_TXPWR_REQ_MINSIZE     		CalcReqMsgLength(nwm_get_req_t)
#define NWM_GET_MCAST_REQ_MINSIZE     		CalcReqMsgLength(nwm_get_req_t)

#define NWM_GET_ALL_CFM_MINSIZE		CalcCfmMsgLength(nwm_get_all_cfm_t)
#define NWM_GET_MACADRS_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_macadrs_cfm_t)
#define NWM_GET_RETRY_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_retrylimit_cfm_t)
#define NWM_GET_ENCH_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_enablechannel_cfm_t)
#define NWM_GET_MODE_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_mode_cfm_t)
#define NWM_GET_RATESET_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_rateset_cfm_t)
#define NWM_GET_WEPMODE_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_wepmode_cfm_t)
#define NWM_GET_WEPKEYID_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_wepkeyid_cfm_t)
#define NWM_GET_BCN_TYPE_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_beacontype_cfm_t)
#define NWM_GET_RES_BC_SSID_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_proberes_cfm_t)
#define NWM_GET_BCN_LOST_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_beaconlostth_cfm_t)
#define NWM_GET_ACT_ZONE_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_activezone_cfm_t)
#define NWM_GET_SSID_MASK_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_ssidmask_cfm_t)
#define NWM_GET_PREAMBLE_TYPE_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_preambletype_cfm_t)
#define NWM_GET_AUTHALGO_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_authalgo_cfm_t)
#define NWM_GET_CCA_EDTH_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_ccamode_edth_cfm_t)
#define NWM_GET_MAX_CONN_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_maxconn_cfm_t)
#define NWM_GET_TXANT_CFM_MINSIZE		CalcCfmMsgLength(nwm_get_txant_cfm_t)
#define NWM_GET_DIVERSITY_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_diversity_cfm_t)
#define NWM_GET_BCNTXRXIND_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_beaconsendrecvind_cfm_t)
#define NWM_GET_INTERFERENCE_CFM_MINSIZE		CalcCfmMsgLength(nwm_get_interference_cfm_t)

#define NWM_GET_BSSID_CFM_MINSIZE		CalcCfmMsgLength(nwm_get_bssid_cfm_t)
#define NWM_GET_SSID_CFM_MINSIZE		CalcCfmMsgLength(nwm_get_ssid_cfm_t)
#define NWM_GET_BCN_PERIOD_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_beaconperiod_cfm_t)
#define NWM_GET_DTIM_PERIOD_CFM_MINSIZE	CalcCfmMsgLength(nwm_get_dtimperiod_cfm_t)
#define NWM_GET_GAME_INFO_CFM_MINSIZE   CalcCfmMsgLength(nwm_get_gameinfo_cfm_t)
#define NWM_GET_VBLANK_TSF_CFM_MINSIZE   CalcCfmMsgLength(nwm_get_vblanktsf_cfm_t)
#define NWM_GET_MACLIST_CFM_MINSIZE   CalcCfmMsgLength(nwm_get_maclist_req_t)
#define NWM_GET_RTSTHRESH_CFM_MINSIZE   CalcCfmMsgLength(nwm_get_rtsthresh_cfm_t)
#define NWM_GET_FRAGTHRESH_CFM_MINSIZE   CalcCfmMsgLength(nwm_get_fragthresh_cfm_t)
#define NWM_GET_EEROM_CFM_MINSIZE        CalcCfmMsgLength(nwm_get_eerom_cfm_t)
#define NWM_GET_RSSI_CFM_MINSIZE        CalcCfmMsgLength(nwm_get_rssi_cfm_t)
#define NWM_GET_RSSI_INFO_CFM_MINSIZE	(CalcCfmMsgLength(nwm_get_rssi_cfm_t))
#define NWM_GET_TXPWR_CFM_MINSIZE   	CalcCfmMsgLength(nwm_get_txpwr_cfm_t)
#define NWM_GET_MCAST_CFM_MINSIZE   	CalcCfmMsgLength(nwm_get_mcast_rate_cfm_t)

/* DEVICE */
#define NWM_DEV_SHUTDOWN_REQ_MINSIZE	CalcReqMsgLength(nwm_dev_shutdown_req_t)
#define NWM_DEV_IDLE_REQ_MINSIZE	CalcReqMsgLength(nwm_dev_idle_req_t)
#define NWM_DEV_CLASS1_REQ_MINSIZE	CalcReqMsgLength(nwm_dev_class1_req_t)
#define NWM_DEV_REBOOT_REQ_MINSIZE	CalcReqMsgLength(nwm_dev_reboot_req_t)
#define NWM_DEV_CLR_WLINFO_REQ_MINSIZE		CalcReqMsgLength(nwm_dev_clrinfo_req_t)
#define NWM_DEV_GET_VERINFO_REQ_MINSIZE		CalcReqMsgLength(nwm_dev_getverinfo_req_t)
#define NWM_DEV_GET_WLINFO_REQ_MINSIZE		CalcReqMsgLength(nwm_dev_getinfo_req_t)
#define NWM_DEV_GET_STATE_REQ_MINSIZE		CalcReqMsgLength(nwm_dev_getstate_req_t)
#define NWM_DEV_TEST_SIGNAL_REQ_MINSIZE		CalcReqMsgLength(nwm_dev_testsignal_req_t)

#define NWM_DEV_SHUTDOWN_CFM_MINSIZE		CalcCfmMsgLength(nwm_dev_shutdown_cfm_t)
#define NWM_DEV_IDLE_CFM_MINSIZE			CalcCfmMsgLength(nwm_dev_idle_cfm_t)
#define NWM_DEV_CLASS1_CFM_MINSIZE			CalcCfmMsgLength(nwm_dev_class1_cfm_t)
#define NWM_DEV_REBOOT_CFM_MINSIZE			CalcCfmMsgLength(nwm_dev_reboot_cfm_t)
#define NWM_DEV_CLR_WLINFO_CFM_MINSIZE		CalcCfmMsgLength(nwm_dev_clrinfo_cfm_t)
#define NWM_DEV_GET_VERINFO_CFM_MINSIZE		CalcCfmMsgLength(nwm_dev_getverinfo_cfm_t)
#define NWM_DEV_GET_WLINFO_CFM_MINSIZE		CalcCfmMsgLength(nwm_dev_getinfo_cfm_t)
#define NWM_DEV_GET_STATE_CFM_MINSIZE		CalcCfmMsgLength(nwm_dev_getstate_cfm_t)
#define NWM_DEV_TEST_SIGNAL_CFM_MINSIZE		CalcCfmMsgLength(nwm_dev_testsignal_cfm_t)

#define MAX_NITRO_STA_ALLOWED   15    /* Max num of nitro STA allowed per BSS */

typedef BWL_PRE_PACKED_STRUCT struct {
	uint16		code;
	uint16		length;
} BWL_POST_PACKED_STRUCT nwm_cmdhdr_t;

/* MLME-Reset.Request/Confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				mib;
} BWL_POST_PACKED_STRUCT nwm_mlme_reset_req_t;

/* MLME-Reset.Confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
} BWL_POST_PACKED_STRUCT nwm_mlme_reset_cfm_t;

/* BSSDescription */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16		length;
	uint16		rssi;
	uint16		bssid[3];
	uint16		ssidlength;
	uint8		ssid[32];
	uint16		capainfo;
	struct {
		uint16	basic;
		uint16	support;
	} rateset;
	uint16		beaconPeriod;
	uint16		dtimPeriod;
	uint16		channel;
	uint16		cfpPeriod;
	uint16		cfpMaxDuration;
	uint16		info_elt_len;
	uint16		info_elt[1];
} BWL_POST_PACKED_STRUCT nwm_bss_desc_t;

#define NWM_BSSDESC_FIXED_SIZE	(sizeof(nwm_bss_desc_t) - sizeof(uint16))
#define NWM_GAME_INFO_ELEMENT_ID	0xDD

/* MLME-Scan.Request/Confirm */
#define NWM_MAX_CHANNELS				(NBITS(uint16))	/* a vector */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				bssid[3];
	uint16				ssid_length;
	uint16				ssid[16];
	uint16				scan_type;
	uint16				chvector;
	uint16				max_channel_time;
} BWL_POST_PACKED_STRUCT nwm_mlme_scan_req_t;

#define NWM_SCAN_REQ_SIZE	(sizeof(nwm_mlme_scan_req_t))

/* MLME-Scan.Confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultcode;

} BWL_POST_PACKED_STRUCT nwm_mlme_scan_cfm_t;

#define NWM_SCAN_CFM_SIZE	(sizeof(nwm_mlme_scan_cfm_t))

/* MLME-Join.Request/Confirm/Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				rsv;
	nwm_bss_desc_t		bssdesc;
} BWL_POST_PACKED_STRUCT nwm_mlme_join_req_t;

/* MLME-Join.Confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultcode;
} BWL_POST_PACKED_STRUCT nwm_mlme_join_cfm_t;

typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;
	uint16				resultcode;
	uint16				bssid[3];
	uint16				aid;
} BWL_POST_PACKED_STRUCT nwm_mlme_join_ind_t;

#define NWM_JOIN_REQ_LEN	(sizeof(nwm_mlme_join_req_t) + sizeof(nwm_mlme_join_cfm_t))

#define NWM_JOIN_IND_LENGTH		CalcIndMsgLength(nwm_mlme_join_ind_t)
#define NWM_JOIN_IND_SIZE		(sizeof(nwm_mlme_join_ind_t))
#define NWM_AID_MASK		0x7ff	/* Only want the "true" aid value */

/* MLME-DisAssociate.Request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				mac_addr[3];
	uint16				reason;
} BWL_POST_PACKED_STRUCT nwm_mlme_disassoc_req_t;

/* MLME-DisAssociate.Confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultcode;
} BWL_POST_PACKED_STRUCT nwm_mlme_disassoc_cfm_t;

#define NWM_MAX_GAMEINFO_LENGTH		(128)		/* bytes */

/* MLME-Start.Request/Confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				ssidlength;
	uint16				ssid[16];
	uint16				beaconperiod;
	uint16				dtimperiod;
	uint16				channel;
	uint16				basic_rateset;
	uint16				supp_rateset;
	uint16				gameinfolength;
	uint16				gameinfo[1];
} BWL_POST_PACKED_STRUCT nwm_mlme_startreq_t;

#define NWM_STARTREQ_FIXED_SIZE		(sizeof(nwm_mlme_startreq_t) - sizeof(uint16))

/* MLME-Start.Confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
} BWL_POST_PACKED_STRUCT nwm_mlme_startcfm_t;

/* MLME-MeasureChannel.Request/Confirm/Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				reserved1;
	uint16				testmode;
	uint16				reserved2;
	uint16				measure_time;
	uint16				chvector;
} BWL_POST_PACKED_STRUCT nwm_mlme_measchan_req_t;

#define NWM_MEASCHAN_REQ_SIZE	(sizeof(nwm_mlme_measchan_req_t))
#define NWM_MEASTYPE_CCA	0
#define NWM_MEASTYPE_RPI	1

/* MLME-MeasureChannel.Confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
} BWL_POST_PACKED_STRUCT nwm_mlme_measchan_cfm_t;

#define NWM_MEASCHAN_CFM_SIZE		(sizeof(nwm_mlme_measchan_cfm_t))

/* MLME-Associate.Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				mac_addr[3];
	uint16				aid;
	uint16				ssid_len;
	uint16				ssid[16];
} BWL_POST_PACKED_STRUCT nwm_mlme_assoc_ind_t;

#define NWM_ASSOC_IND_LENGTH	CalcIndMsgLength(nwm_mlme_assoc_ind_t)
#define NWM_ASSOC_IND_SIZE		(sizeof(nwm_mlme_assoc_ind_t))

/* MLME-DisAssociate.Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				mac_addr[3];
	uint16				reason_code;
} BWL_POST_PACKED_STRUCT nwm_mlme_disassoc_ind_t;

#define NWM_DISASSOC_IND_LENGTH		CalcIndMsgLength(nwm_mlme_disassoc_ind_t)
#define NWM_DISASSOC_IND_SIZE		(sizeof(nwm_mlme_disassoc_ind_t))

/* MLME-BeaconLost.Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				peer_mac[3];
} BWL_POST_PACKED_STRUCT nwm_mlme_beaconlost_ind_t;

#define NWM_BCNLOST_IND_LENGTH		CalcIndMsgLength(nwm_mlme_beaconlost_ind_t)
#define NWM_BCNLOST_IND_SIZE	(sizeof(nwm_mlme_beaconlost_ind_t))

/* MLME-BeaconSend.Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;
} BWL_POST_PACKED_STRUCT nwm_mlme_beaconsend_ind_t;

#define NWM_BCNSEND_IND_LENGTH	CalcIndMsgLength(nwm_mlme_beaconsend_ind_t)
#define NWM_BCNSEND_IND_SIZE	(sizeof(nwm_mlme_beaconsend_ind_t))

/* MLME-BeaconRecv.Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				reserved1[3];
	uint16				game_info_length;

	uint16				reserved2[3];
	uint16				rate;
	uint16				rssi;

	uint16				reserved3[4];
	uint16				reserved4[3];

	uint16				src_mac_adrs[3];
	uint16				vblank_tsf;
	uint16				game_info[1];
} BWL_POST_PACKED_STRUCT nwm_mlme_beaconrecv_ind_t;

#define NWM_BCNRECV_IND_FIXED_LEN	\
	(CalcIndMsgLength(nwm_mlme_beaconrecv_ind_t) - 1)

#define NWM_BCNRECV_IND_FIXED_SIZE	(sizeof(nwm_mlme_beaconrecv_ind_t)\
		- sizeof(uint16))
#define NWM_BCNRCV_IND_SIZE		(sizeof(nwm_mlme_beaconrecv_ind_t))

/* MA-Data.Request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;
	uint16				frameid;
	uint16				dstea[3];
	uint16				srcea[3];
	uint16				proto;
	uint16				frame[1];
} BWL_POST_PACKED_STRUCT nwm_ma_data_req_t;

#define NWM_HDR_TOP					(NWM_RSV * 2 + sizeof(nwm_cmdhdr_t))
#define NWM_MASENDREQ_FIXED_SIZE	(sizeof(nwm_ma_data_req_t) - sizeof(uint16))
/* offset to ethernet frame from MA-Data.send requerst */
#define NWM_SEND_REQ_HDR	(NWM_HDR_TOP + sizeof(uint16))

/* MA-KeyData.Request */
typedef BWL_PRE_PACKED_STRUCT struct nwm_mphdr {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				frameid;
	uint16				reserved;
} BWL_POST_PACKED_STRUCT nwm_mphdr_t;
#define NWM_MP_REQ_HDR_SIZE			(sizeof(nwm_mphdr_t))

typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_mphdr_t			keycmdhdr;
	uint16				length;
	uint16				wmheader;
	uint16				keydata[1];
} BWL_POST_PACKED_STRUCT nwm_ma_keydata_req_t;
#define NWM_MAKEY_DATAREQ_FIXED_SIZE	(sizeof(nwm_ma_keydata_req_t) - 2)

/* MA-MP.Request */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_mphdr_t			mpcmdhdr;
	uint16				resume;
	uint16				retrylimit;
	uint16				txop;
	uint16				pollbitmap;
	uint16				tmptt;
	uint16				currtsf;
	uint16				datalength;
	uint16				wmheader;
	uint16				data[1];
} BWL_POST_PACKED_STRUCT nwm_ma_mp_req_t;

#define NWM_MA_MPREQ_FIXED_SIZE	(sizeof(nwm_ma_mp_req_t) - 2)
#define NWM_MPIND_HDR_SIZE		(sizeof(nwm_mpind_hdr_t))
#define NWM_INDHDR_FIXED_SIZE	(sizeof(nwm_top_indhdr_t))
#define NWM_NIN_IND_MAGIC	(0xFEEDFACE)

/* Keydata header attached to Keydata response from each child */
typedef BWL_PRE_PACKED_STRUCT struct wl_mpkey_data {
	uint16        length;        /* length of data */
	uint8         rate;          /* rate of the recvd packet */
	uint8         rssi;          /* rssi of the recvd packet */
	uint16        aid;           /* aid of the child that sent the response */
	uint16        noResponse;    /* set if no response is received from the child */
	uint8         cdata[1];      /* Keydata received from the child */
} BWL_POST_PACKED_STRUCT nwm_mpkey_data_t;
#define NWM_MPKEYDATA_FIXED_SIZE	(sizeof(nwm_mpkey_data_t) - 1)
#define NWM_MPKEY_FIXED_SIZE	(sizeof(nwm_mpkey_t) - sizeof(nwm_mpkey_data_t))
#define NWM_MPKEY_SUBHDR_SIZE	(NWM_MPKEY_FIXED_SIZE - NWM_INDHDR_FIXED_SIZE)

/* MA-TestData.Request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint8		frame[1];
} BWL_POST_PACKED_STRUCT nwm_ma_testdata_req_t;

/* MA-ClrData.Request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				flag;
} BWL_POST_PACKED_STRUCT nwm_ma_clrdata_req_t;

/* MA-ClrData.Confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
} BWL_POST_PACKED_STRUCT nwm_ma_clrdata_cfm_t;

/* Common to all indications */
typedef BWL_PRE_PACKED_STRUCT struct wl_nwmindhdr {
	uint32		  magic;		/* marker for indications */
	uint16		  reserved1;	/* unused */
	uint16		  reserved2;	/* unused */
	uint16		  reserved3;	/* unused */
	uint16		  reserved4;	/* unused */
	uint16		  indcode;		/* indication type */
	uint16		  indlength;	/* length of following in uint16 words */
} BWL_POST_PACKED_STRUCT nwm_top_indhdr_t;

/* MLME-Scan Complete Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_top_indhdr_t	header;
	uint16				status;
	uint16				bss_desc_count;
	nwm_bss_desc_t		bss_desc_list[1];
} BWL_POST_PACKED_STRUCT nwm_scanind_t;
/* the hard way to figure out there are two uint16 fields left over! */
#define NWM_SCANIND_FIXED_SIZE		(sizeof(nwm_scanind_t) - sizeof(nwm_bss_desc_t))
#define NWM_SCANIND_FIXED_LEN		(NWM_SCANIND_FIXED_SIZE - NWM_INDHDR_FIXED_SIZE)

/* MA-Data.Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_top_indhdr_t		header;
	uint16					pad;		/* need for 4 byte alignment */
} BWL_POST_PACKED_STRUCT nwm__madataind_hdr_t;
#define NWM_MADATA_RCV_HDR_SIZE		(sizeof(nwm__madataind_hdr_t))

/* MLME-MeasureChannel.Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_top_indhdr_t	header;

	uint16				result_code;
	uint16				type;
	uint16				chvector;
	BWL_PRE_PACKED_STRUCT union {
		uint16			cca_results[16];
		uint16			rpi_results[16][8];
	} BWL_POST_PACKED_STRUCT u;
} BWL_POST_PACKED_STRUCT nwm_mlme_measchan_ind_t;
#define NWM_MEASCHAN_IND_FIXED_LEN		(3 * sizeof(uint16))
#define NWM_MEASCHAN_IND_FIXED_SIZE	(sizeof(nwm_top_indhdr_t) - NWM_MEASCHAN_IND_FIXED_LEN)
/* MA-MP and MA-MPACK Indications */

/* Header attached to MP and MPACK frames before sending up */
typedef BWL_PRE_PACKED_STRUCT struct wl_mpind_hdr {

	nwm_top_indhdr_t nwmindhdr;	/* Nintendo indication header */

	uint8         pad_1[6];
	uint16        length;        /* Length of the frame */
	uint16        keytxsts;      /* status of the Keydata txq */
	uint16        mpacktimestamp; /* timestamp of expected MPACK frame */
	uint16        timestamp;     /* timestamp of this frame */
	uint8         rate;          /* rate of recvd frame */
	uint8         rssi;          /* rssi of recvd frame */
	uint8         pad_2[4];
} BWL_POST_PACKED_STRUCT nwm_mpind_hdr_t;

/* MA-MPEND.Indication */

/* Key header attached to MPEND indication frame */
typedef BWL_PRE_PACKED_STRUCT struct wl_mpkey {
	nwm_top_indhdr_t nwmindhdr;	/* Nintendo indication header */

	uint16        bitmap;        /* bit set indicates there was no response fromthat child */
	uint16        currtsf;       /* bits 6-21 of tsf_l reg when ind is sent up */
	uint16        count;         /* Number of responses in this frame */
	uint16        resp_maxlen;   /* Max length of expected response */
	uint16        txCount;       /* Num of MP sequence tries */
	nwm_mpkey_data_t data[1];
} BWL_POST_PACKED_STRUCT nwm_mpkey_t;

/* MA-FATALERR.Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_top_indhdr_t		header;

	uint16				errcode;
	uint16				data[1];
} BWL_POST_PACKED_STRUCT nwm_ma_fatalerr_ind_t;

#define NWM_FATALERR_IND_FIXED_LENGTH	(sizeof(nwm_ma_fatalerr_ind_t) - sizeof(uint16))
#define NWM_FATALERR_INDLENGTH			(sizeof(uint16))

typedef BWL_PRE_PACKED_STRUCT struct {
	uint16			frameid;
	uint16			errcode;
} BWL_POST_PACKED_STRUCT nwm_ma_fatalerr_tx_ind_t;

/* MA-Timeout Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_top_indhdr_t		header;
	uint16					errcode;
	uint16					data[1];
} BWL_POST_PACKED_STRUCT nwm_ma_timouterr_ind_t;

#define NWM_TIMEOUTERR_IND_FIXED_LENGTH		(sizeof(nwm_ma_timouterr_ind_t) - sizeof(uint16))

/* MA-Channel_Use.Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_top_indhdr_t		header;
	uint16					channel;
	uint16					on_off;
} BWL_POST_PACKED_STRUCT nwm_ma_channel_use_ind_t;
#define NWM_MA_CHANNEL_USE_SIZE		(sizeof(nwm_ma_channel_use_ind_t))
#define NWM_CHANNEL_USE_IND_LENGTH	(NWM_MA_CHANNEL_USE_SIZE - NWM_INDHDR_FIXED_SIZE)

/* MA-IAPP.Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_top_indhdr_t		header;
	uint16					frame[1];
} BWL_POST_PACKED_STRUCT nwm_ma_iapp_ind_t;

#define NWM_MA_IAPP_FIXED_SIZE		(sizeof(nwm_ma_iapp_ind_t) - sizeof(uint16))

/* Set all (really set many) */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				reserved1[3];
	uint16				shortRetryLimit;
	uint16				reserved2[2];
	uint16				opmode;
	uint16				reserved3;
	uint16				wepmode;
	uint16				wepKeyId;
	uint16				wepKey[4][10];
	uint16				beacontype;
	uint16				proberes;
	uint16				beaconlost_thr;
	uint16				reserved4;
	uint16				ssidmask[16];
	uint16				preambletype;
	uint16				authalgo;
	uint16				longRetryLimit;
	uint16				wpa_passphrase_len;
	uint16				wpa_passphrase[32];
} BWL_POST_PACKED_STRUCT nwm_set_all_req_t;

/* MAC */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				macaddr[3];
} BWL_POST_PACKED_STRUCT nwm_set_macadrs_req_t;

/* Retry: short and long */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16			shortRetryLimit;
	uint16			longRetryLimit;
} BWL_POST_PACKED_STRUCT nwm_set_retrylimit_req_t;

/* Set country */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16			countrystring[2];
} BWL_POST_PACKED_STRUCT nwm_set_enablechannel_req_t;

/* Mode Set */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				mode;
} BWL_POST_PACKED_STRUCT nwm_set_mode_req_t;

/* Rate request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				supprate;
	uint16				basicrate;
} BWL_POST_PACKED_STRUCT nwm_set_rateset_req_t;

/* Consult Broadcom-Nintendo specification for details on these */
#define NWM_RATESET_VECTOR_DISALLOWED	0xf000
#define NWM_RATSET_NITRO_ALLOWED		0x0003

/* WEP mode */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				wepmode;
} BWL_POST_PACKED_STRUCT nwm_set_wepmode_req_t;

/* Set default WEP key index */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				wepKeyId;
} BWL_POST_PACKED_STRUCT nwm_set_wepkeyid_req_t;

/* Set WEP keys */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				wepKey[4][10];
} BWL_POST_PACKED_STRUCT nwm_set_wepkey_req_t;

/* Set beacon type: broadcast SSID (or don't) */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				beacontype;
} BWL_POST_PACKED_STRUCT nwm_set_beacontype_req_t;

/* Set probe response parameters: respond (or don't) to broadcast probe request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				proberes;
} BWL_POST_PACKED_STRUCT nwm_set_proberes_req_t;

/* Beacon lost threshold */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				beaconlost_thr;
} BWL_POST_PACKED_STRUCT nwm_set_beaconlostth_req_t;

#define NWM_BCN_THRESH_MAX	255

/* ActiveZone */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				active_zone_time;
} BWL_POST_PACKED_STRUCT nwm_set_activezone_req_t;

/* Set SSID mask */
#define NWM_SSIDMASK_LEN	32
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				ssidmask[NWM_SSIDMASK_LEN/2];
} BWL_POST_PACKED_STRUCT nwm_set_ssidmask_req_t;

/* Set preamble type */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				type;
} BWL_POST_PACKED_STRUCT nwm_set_preambletype_req_t;

/* Authentication Algorithm */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				type;
} BWL_POST_PACKED_STRUCT nwm_set_authalgo_req_t;

/* LifeTime */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				Mtime;
	uint16				Ntime;
	uint16				frameLifeTime;
} BWL_POST_PACKED_STRUCT nwm_set_lifetime_req_t;

/* Max Connectable Child */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				count;
} BWL_POST_PACKED_STRUCT nwm_set_maxconn_req_t;

/* Tx Antenna use */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				useantenna;
} BWL_POST_PACKED_STRUCT nwm_set_txant_req_t;

/* Antenna Diversity */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				diversity;
	uint16				useantenna;
} BWL_POST_PACKED_STRUCT nwm_set_diversity_req_t;

/* BeaconSend/Recv.ind */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				enable_send;
	uint16				enable_recv;
} BWL_POST_PACKED_STRUCT nwm_set_beaconsendrecvind_req_t;

/* Interference mode use */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				interference;
} BWL_POST_PACKED_STRUCT nwm_set_interference_req_t;

/* Set BSSID */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16					bssid[3];
} BWL_POST_PACKED_STRUCT nwm_set_bssid_req_t;

/* Beacon Period */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				beaconPeriod;
} BWL_POST_PACKED_STRUCT nwm_set_beaconperiod_req_t;

/* DTIM Period */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				dtimPeriod;
} BWL_POST_PACKED_STRUCT nwm_set_dtimperiod_req_t;

/* Game Info */
#define NWM_MAX_GAMEINFO_LEN		128 /* bytes */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				game_info_length;
	uint16				gameinfo[1];
} BWL_POST_PACKED_STRUCT nwm_set_gameinfo_req_t;

/* Vblank_tsf */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				vblank;

} BWL_POST_PACKED_STRUCT nwm_set_vblanktsf_req_t;

/* MAC list */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				enableflag;
	uint16				count;
	uint16				macentry[3];
} BWL_POST_PACKED_STRUCT nwm_set_maclist_req_t;

/* RTS Threshold */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				rtsthresh;

} BWL_POST_PACKED_STRUCT nwm_set_rtsthresh_req_t;

/* Frag Threshold */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				fragthresh;

} BWL_POST_PACKED_STRUCT nwm_set_fragthresh_req_t;

/* PMK */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				keylen;
	uint16				key[32];

} BWL_POST_PACKED_STRUCT nwm_set_pmk_req_t;

/* Set country code in eerom */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				country_code[2];
} BWL_POST_PACKED_STRUCT nwm_set_eerom_req_t;

/* tx power */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				txpwr;
} BWL_POST_PACKED_STRUCT nwm_set_txpwr_req_t;

/* multicast rate */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				mcast_rate;
} BWL_POST_PACKED_STRUCT nwm_set_mcast_rate_req_t;

/* Parameter set confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
} BWL_POST_PACKED_STRUCT nwm_set_cfm_t;

/* Get top level */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;
} BWL_POST_PACKED_STRUCT nwm_get_req_t;

/* Get all (really get many) */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				macaddr[3];
	uint16				shortRetryLimit;
	uint16				enableChannel;
	uint16				channel;
	uint16				opmode;
	uint16				reserved1;
	uint16				wepmode;
	uint16				wepKeyId;
	uint16				beacontype;
	uint16				proberes;
	uint16				beaconlost_thr;
	uint16				reserved;
	uint16				ssidmask[16];
	uint16				preambletype;
	uint16				authalgo;
	uint16				longRetryLimit;
} BWL_POST_PACKED_STRUCT nwm_get_all_cfm_t;

/* Get MAC */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				macaddr[3];
} BWL_POST_PACKED_STRUCT nwm_get_macadrs_cfm_t;

/* Get retry limits */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16			shortRetryLimit;
	uint16			longRetryLimit;
} BWL_POST_PACKED_STRUCT nwm_get_retrylimit_cfm_t;

/* Get available channels (for "this" locale) */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				enableChannel;
	uint16				channel;
	uint16				country[WLC_CNTRY_BUF_SZ/2];
} BWL_POST_PACKED_STRUCT nwm_get_enablechannel_cfm_t;

/* set the country code and return the channel list */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
	uint16				enableChannel;

} BWL_POST_PACKED_STRUCT nwm_set_enablechannel_cfm_t;

#define NWM_ALLOWED_CHANNEL_VECTOR		(0x7ffe)			/* valid bits */
#define NWM_UNALLOWED_CHANNEL_VECTOR	(0x8001)			/* invalid bits */

/* Get operating mode */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				mode;
} BWL_POST_PACKED_STRUCT nwm_get_mode_cfm_t;

/* Get ratesets */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				supprate;
	uint16				basicrate;
} BWL_POST_PACKED_STRUCT nwm_get_rateset_cfm_t;

/* Get WEP mode */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				wepmode;
} BWL_POST_PACKED_STRUCT nwm_get_wepmode_cfm_t;

/* Get WEP default key index */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				wepKeyId;
} BWL_POST_PACKED_STRUCT nwm_get_wepkeyid_cfm_t;

/* Get Beacon type */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultcode;

	uint16				beacontype;
} BWL_POST_PACKED_STRUCT nwm_get_beacontype_cfm_t;

/*
 * Get probe response type
 *
 */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultcode;

	uint16				proberes;
} BWL_POST_PACKED_STRUCT nwm_get_proberes_cfm_t;

/* Get Beacon lost threshold */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				beaconlost_thr;
} BWL_POST_PACKED_STRUCT nwm_get_beaconlostth_cfm_t;

/* Get ActiveZone */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				active_zone_time;
} BWL_POST_PACKED_STRUCT nwm_get_activezone_cfm_t;

/* Get SSID mask */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				ssidmask[NWM_SSIDMASK_LEN/2];
} BWL_POST_PACKED_STRUCT nwm_get_ssidmask_cfm_t;

/* Get Preamble type */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				type;
} BWL_POST_PACKED_STRUCT nwm_get_preambletype_cfm_t;

/* Authentication Algorithm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				type;
} BWL_POST_PACKED_STRUCT nwm_get_authalgo_cfm_t;

/* Max Connectable Child */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				count;
} BWL_POST_PACKED_STRUCT nwm_get_maxconn_cfm_t;

/* Tx Antenna use */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				useantenna;
} BWL_POST_PACKED_STRUCT nwm_get_txant_cfm_t;

/* Antenna Diversity */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				diversity;
	uint16				useantenna;
} BWL_POST_PACKED_STRUCT nwm_get_diversity_cfm_t;

/* Get BeaconSend/Recv.ind */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				enable_send;
	uint16				enable_recv;
} BWL_POST_PACKED_STRUCT nwm_get_beaconsendrecvind_cfm_t;

/* Interference Mode use */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				interference;
} BWL_POST_PACKED_STRUCT nwm_get_interference_cfm_t;

/* Get BSSID */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				bssid[3];
} BWL_POST_PACKED_STRUCT nwm_get_bssid_cfm_t;

/* Get SSID */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				ssidLength;
	uint16				ssid[16];
} BWL_POST_PACKED_STRUCT nwm_get_ssid_cfm_t;

/* Get Beacon Period */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				beaconPeriod;
} BWL_POST_PACKED_STRUCT nwm_get_beaconperiod_cfm_t;

/*
 * Get DTIM Period
 *
 */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				dtimPeriod;
} BWL_POST_PACKED_STRUCT nwm_get_dtimperiod_cfm_t;

/* Get Game Info */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				game_info_length;
	uint16				gameinfo[1];
} BWL_POST_PACKED_STRUCT nwm_get_gameinfo_cfm_t;

/* Vblank_tsf */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				vblank;

} BWL_POST_PACKED_STRUCT nwm_get_vblanktsf_cfm_t;

/* MAC list */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				enableflag;
	uint16				count;
	uint16				macentry[3];
} BWL_POST_PACKED_STRUCT nwm_get_maclist_req_t;

/* RTS Threshold */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				rtsthresh;

} BWL_POST_PACKED_STRUCT nwm_get_rtsthresh_cfm_t;

/* Frag Threshold */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				fragthresh;

} BWL_POST_PACKED_STRUCT nwm_get_fragthresh_cfm_t;

/* Get country code from eerom */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
	uint16				country_code[2];
} BWL_POST_PACKED_STRUCT nwm_get_eerom_cfm_t;

/* RSSI, RxRate, TxRate, etc */
typedef BWL_PRE_PACKED_STRUCT struct rssi_info {
	uint16			RSSI;
	uint16			TxRate;
	uint16			RxRate;
	uint16			macaddr[3];
} BWL_POST_PACKED_STRUCT nwm_rssi_info_t;
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				TSSI;
	uint16				count;
	struct rssi_info info[1];

} BWL_POST_PACKED_STRUCT nwm_get_rssi_cfm_t;

/* tx power */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				txpwr;

} BWL_POST_PACKED_STRUCT nwm_get_txpwr_cfm_t;

/* multicast rate */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				mcast_rate;

} BWL_POST_PACKED_STRUCT nwm_get_mcast_rate_cfm_t;

/* IDLE request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;
} BWL_POST_PACKED_STRUCT nwm_dev_idle_req_t;

/* IDLE confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
} BWL_POST_PACKED_STRUCT nwm_dev_idle_cfm_t;

/* CLASS1 request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;
} BWL_POST_PACKED_STRUCT nwm_dev_class1_req_t;

/* CLASS1 confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
} BWL_POST_PACKED_STRUCT nwm_dev_class1_cfm_t;

/* Restart request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;
} BWL_POST_PACKED_STRUCT nwm_dev_reboot_req_t;

/* Restart confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
} BWL_POST_PACKED_STRUCT nwm_dev_reboot_cfm_t;

/* Clear wireless counters */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;
} BWL_POST_PACKED_STRUCT nwm_dev_clrinfo_req_t;

/* clear data confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
} BWL_POST_PACKED_STRUCT nwm_dev_clrinfo_cfm_t;

/* Get version request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;
} BWL_POST_PACKED_STRUCT nwm_dev_getverinfo_req_t;

#define NWM_VER_STRLEN		80
/*  Get version confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16		version_str[NWM_VER_STRLEN/2]; /* null terminated string */
	uint32		vendorid;	/* pci vendor id */
	uint32		deviceid;	/* device id of chip */
	uint32		radiorev;	/* radio revision */
	uint32		chiprev;	/* chip revision */
	uint32		corerev;	/* core revision */
	uint32		boardid;	/* board identifier( usu. PCI sub-device id) */
	uint32		boardvendor;	/* board vendor usu. PCI sub-vendor id) */
	uint32		boardrev;	/* board revision */
	uint32		driverrev;	/* driver version */
	uint32		ucoderev;	/* microcode version */
	uint32		bus;		/* bus type */
	uint32		chipnum;	/* chip number */

} BWL_POST_PACKED_STRUCT nwm_dev_getverinfo_cfm_t;

/* Get wireless counters request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;
} BWL_POST_PACKED_STRUCT nwm_dev_getinfo_req_t;

#define NWM_NFIFO		6
typedef struct {
	uint16	version;	/* see definition of WL_CNT_T_VERSION */
	uint16	length;		/* length of entire structure */

	/* transmit stat counters */
	uint32	txframe;	/* tx data frames */
	uint32	txbyte;		/* tx data bytes */
	uint32	txretrans;	/* tx mac retransmits */
	uint32	txerror;	/* tx data errors */
	uint32	txctl;		/* tx management frames */
	uint32	txprshort;	/* tx short preamble frames */
	uint32	txserr;		/* tx status errors */
	uint32	txnobuf;	/* tx out of buffers errors */
	uint32	txnoassoc;	/* tx discard because we're not associated */
	uint32	txrunt;		/* tx runt frames */
	uint32	txchit;		/* tx header cache hit (fastpath) */
	uint32	txcmiss;	/* tx header cache miss (slowpath) */

	/* transmit chip error counters */
	uint32	txuflo;		/* tx fifo underflows */
	uint32	txphyerr;	/* tx phy errors (indicated in tx status) */
	uint32	txphycrs;

	/* receive stat counters */
	uint32	rxframe;	/* rx data frames */
	uint32	rxbyte;		/* rx data bytes */
	uint32	rxerror;	/* rx data errors */
	uint32	rxctl;		/* rx management frames */
	uint32	rxnobuf;	/* rx out of buffers errors */
	uint32	rxnondata;	/* rx non data frames in the data channel errors */
	uint32	rxbadds;	/* rx bad DS errors */
	uint32	rxbadcm;	/* rx bad control or management frames */
	uint32	rxfragerr;	/* rx fragmentation errors */
	uint32	rxrunt;		/* rx runt frames */
	uint32	rxgiant;	/* rx giant frames */
	uint32	rxnoscb;	/* rx no scb error */
	uint32	rxbadproto;	/* rx invalid frames */
	uint32	rxbadsrcmac;	/* rx frames with Invalid Src Mac */
	uint32	rxbadda;	/* rx frames tossed for invalid da */
	uint32	rxfilter;	/* rx frames filtered out */

	/* receive chip error counters */
	uint32	rxoflo;		/* rx fifo overflow errors */
	uint32	rxuflo[NWM_NFIFO];	/* rx dma descriptor underflow errors */

	uint32	d11cnt_txrts_off;	/* d11cnt txrts value when reset d11cnt */
	uint32	d11cnt_rxcrc_off;	/* d11cnt rxcrc value when reset d11cnt */
	uint32	d11cnt_txnocts_off;	/* d11cnt txnocts value when reset d11cnt */

	/* misc counters */
	uint32	dmade;		/* tx/rx dma descriptor errors */
	uint32	dmada;		/* tx/rx dma data errors */
	uint32	dmape;		/* tx/rx dma descriptor protocol errors */
	uint32	reset;		/* reset count */
	uint32	tbtt;		/* cnts the TBTT int's */
	uint32	txdmawar;
	uint32	pkt_callback_reg_fail;	/* callbacks register failure */

	/* MAC counters: 32-bit version of d11.h's macstat_t */
	uint32	txallfrm;	/* total number of frames sent, incl. Data, ACK, RTS, CTS,
				 * Control Management (includes retransmissions)
				 */
	uint32	txrtsfrm;	/* number of RTS sent out by the MAC */
	uint32	txctsfrm;	/* number of CTS sent out by the MAC */
	uint32	txackfrm;	/* number of ACK frames sent out */
	uint32	txdnlfrm;	/* Not used */
	uint32	txbcnfrm;	/* beacons transmitted */
	uint32	txfunfl[8];	/* per-fifo tx underflows */
	uint32	txtplunfl;	/* Template underflows (mac was too slow to transmit ACK/CTS
				 * or BCN)
				 */
	uint32	txphyerror;	/* Transmit phy error, type of error is reported in tx-status for
				 * driver enqueued frames
				 */
	uint32	rxfrmtoolong;	/* Received frame longer than legal limit (2346 bytes) */
	uint32	rxfrmtooshrt;	/* Received frame did not contain enough bytes for its frame type */
	uint32	rxinvmachdr;	/* Either the protocol version != 0 or frame type not
				 * data/control/management
				 */
	uint32	rxbadfcs;	/* number of frames for which the CRC check failed in the MAC */
	uint32	rxbadplcp;	/* parity check of the PLCP header failed */
	uint32	rxcrsglitch;	/* PHY was able to correlate the preamble but not the header */
	uint32	rxstrt;		/* Number of received frames with a good PLCP
				 * (i.e. passing parity check)
				 */
	uint32	rxdfrmucastmbss; /* Number of received DATA frames with good FCS and matching RA */
	uint32	rxmfrmucastmbss; /* number of received mgmt frames with good FCS and matching RA */
	uint32	rxcfrmucast;	/* number of received CNTRL frames with good FCS and matching RA */
	uint32	rxrtsucast;	/* number of unicast RTS addressed to the MAC (good FCS) */
	uint32	rxctsucast;	/* number of unicast CTS addressed to the MAC (good FCS) */
	uint32	rxackucast;	/* number of ucast ACKS received (good FCS) */
	uint32	rxdfrmocast;	/* number of received DATA frames (good FCS and not matching RA) */
	uint32	rxmfrmocast;	/* number of received MGMT frames (good FCS and not matching RA) */
	uint32	rxcfrmocast;	/* number of received CNTRL frame (good FCS and not matching RA) */
	uint32	rxrtsocast;	/* number of received RTS not addressed to the MAC */
	uint32	rxctsocast;	/* number of received CTS not addressed to the MAC */
	uint32	rxdfrmmcast;	/* number of RX Data multicast frames received by the MAC */
	uint32	rxmfrmmcast;	/* number of RX Management multicast frames received by the MAC */
	uint32	rxcfrmmcast;	/* number of RX Control multicast frames received by the MAC
				 * (unlikely to see these)
				 */
	uint32	rxbeaconmbss;	/* beacons received from member of BSS */
	uint32	rxdfrmucastobss; /* number of unicast frames addressed to the MAC from
				  * other BSS (WDS FRAME)
				  */
	uint32	rxbeaconobss;	/* beacons received from other BSS */
	uint32	rxrsptmout;	/* Number of response timeouts for transmitted frames
				 * expecting a response
				 */
	uint32	bcntxcancl;	/* transmit beacons canceled due to receipt of beacon (IBSS) */
	uint32	rxf0ovfl;	/* Number of receive fifo 0 overflows */
	uint32	rxf1ovfl;	/* Number of receive fifo 1 overflows (obsolete) */
	uint32	rxf2ovfl;	/* Number of receive fifo 2 overflows (obsolete) */
	uint32	txsfovfl;	/* Number of transmit status fifo overflows (obsolete) */
	uint32	pmqovfl;	/* Number of PMQ overflows */
	uint32	rxcgprqfrm;	/* Number of received Probe requests that made it into
				 * the PRQ fifo
				 */
	uint32	rxcgprsqovfl;	/* Rx Probe Request Que overflow in the AP */
	uint32	txcgprsfail;	/* Tx Probe Response Fail. AP sent probe response but did
				 * not get ACK
				 */
	uint32	txcgprssuc;	/* Tx Probe Response Success (ACK was received) */
	uint32	prs_timeout;	/* Number of probe requests that were dropped from the PRQ
				 * fifo because a probe response could not be sent out within
				 * the time limit defined in M_PRS_MAXTIME
				 */
	uint32	rxnack;		/* Number of NACKS received (Afterburner) */
	uint32	frmscons;	/* Number of frames completed without transmission because of an
				 * Afterburner re-queue
				 */
	uint32	txnack;		/* Number of NACKs transmitted (Afterburner) */
	uint32	txglitch_nack;	/* obsolete */
	uint32	txburst;	/* obsolete */

	/* 802.11 MIB counters, pp. 614 of 802.11 reaff doc. */
	uint32	txfrag;		/* dot11TransmittedFragmentCount */
	uint32	txmulti;	/* dot11MulticastTransmittedFrameCount */
	uint32	txfail;		/* dot11FailedCount */
	uint32	txretry;	/* dot11RetryCount */
	uint32	txretrie;	/* dot11MultipleRetryCount */
	uint32	rxdup;		/* dot11FrameduplicateCount */
	uint32	txrts;		/* dot11RTSSuccessCount */
	uint32	txnocts;	/* dot11RTSFailureCount */
	uint32	txnoack;	/* dot11ACKFailureCount */
	uint32	rxfrag;		/* dot11ReceivedFragmentCount */
	uint32	rxmulti;	/* dot11MulticastReceivedFrameCount */
	uint32	rxcrc;		/* dot11FCSErrorCount */
	uint32	txfrmsnt;	/* dot11TransmittedFrameCount (bogus MIB?) */
	uint32	rxundec;	/* dot11WEPUndecryptableCount */

	/* WPA2 counters (see rxundec for DecryptFailureCount) */
	uint32	tkipmicfaill;	/* TKIPLocalMICFailures */
	uint32	tkipcntrmsr;	/* TKIPCounterMeasuresInvoked */
	uint32	tkipreplay;	/* TKIPReplays */
	uint32	ccmpfmterr;	/* CCMPFormatErrors */
	uint32	ccmpreplay;	/* CCMPReplays */
	uint32	ccmpundec;	/* CCMPDecryptErrors */
	uint32	fourwayfail;	/* FourWayHandshakeFailures */
	uint32	wepundec;	/* dot11WEPUndecryptableCount */
	uint32	wepicverr;	/* dot11WEPICVErrorCount */
	uint32	decsuccess;	/* DecryptSucceswlsCount */
	uint32	tkipicverr;	/* TKIPICVErrorCount */
	uint32	wepexcluded;	/* dot11WEPExcludedCount */

	uint32	txchanrej;	/* Tx frames suppressed due to channel rejection */
} nwm_wl_cnt_t;

/* Nitro protocol stats */
typedef struct wlc_nitro_cnt {
	uint32        txnitro;       /* Nitro frames (MP or Keydata) transmitted */
	uint32        txnitro_fail;  /* Nitro frames transmission fail */
	uint32        txqfull;       /* Child: nitro txq is full */
	uint32        txnullkeydata; /* Child: TX NULL Keydata frames */
	uint32        rxmp;          /* MP frames received */
	uint32        rxkeydata;     /* Keydata frames received */
	uint32        rxnullkeydata; /* NULL Keydata frames received */
	uint32        rxbadnitro;    /* Bad Nitro frames received */
	uint32        rxdupnitro;    /* Duplicate Nitro frames received */
	uint32        rxmpack;       /* MPACK frames received */
	uint32        istatus;       /* Intermediate Status */
	uint32        retrans;       /* Retransmissions */
	uint32        rxkeyerr[MAX_NITRO_STA_ALLOWED];	/* Key response error per sta */
} wlc_nitro_cnt_t;
/* wl pktcnt stats */
typedef struct nwm_get_pktcnt {
	uint32          rx_good_pkt;    /* good rx pkts */
	uint32          rx_bad_pkt;             /* bad rx pkts */
	uint32          tx_good_pkt;    /* good tx pkts */
	uint32          tx_bad_pkt;             /* bad tx pkts */
	uint32          rx_ocast_good_pkt; /* othercast rx */
} nwm_get_pktcnt_t;

/* Get wireless counters confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				rsv1;
	/* from Broadcom wlioctl.h */
	nwm_wl_cnt_t			counter;
	/* from wlc_nitro.h */
	wlc_nitro_cnt_t		nitro;
	/* also from Broadcom wlioctl.h */
	nwm_get_pktcnt_t	pktcounters;
} BWL_POST_PACKED_STRUCT nwm_dev_getinfo_cfm_t;

/* Get state request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;
} BWL_POST_PACKED_STRUCT nwm_dev_getstate_req_t;

/* Get state confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;

	uint16				state;
} BWL_POST_PACKED_STRUCT nwm_dev_getstate_cfm_t;

/* Test signal request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;

	uint16				control;
	uint16				signal;
	uint16				rate;
	uint16				channel;
} BWL_POST_PACKED_STRUCT nwm_dev_testsignal_req_t;

/* test signal confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
} BWL_POST_PACKED_STRUCT nwm_dev_testsignal_cfm_t;

#define NWM_TEST_FREQ_ACC		0
#define NWM_TEST_SUPP_CARRIER	1
#define NWM_TEST_EVM			2

#define NWM_RATE_1MBPS		1
#define NWM_RATE_2MBPS		2
#define NWM_RATE_5_5MBPS	3
#define NWM_RATE_11MBPS		4

/* Request */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;
	uint16				buf[2];
} BWL_POST_PACKED_STRUCT nwm_cmd_req_t;
#define NWM_NOARG_CMD_REQ_SIZE		0

#define NWM_NOARG_CMD_CFM_SIZE		1

/* Confirm */
typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
	uint16				buf[2];
} BWL_POST_PACKED_STRUCT nwm_cmd_cfm_t;

typedef BWL_PRE_PACKED_STRUCT struct {
	nwm_cmdhdr_t		header;
	uint16				resultCode;
} BWL_POST_PACKED_STRUCT nwm_cmd_cfm_top_t;
#define NWM_CFM_TOP		(sizeof(nwm_cmd_cfm_t))

/* Indication */
typedef BWL_PRE_PACKED_STRUCT struct {
	uint16				wlRsv[NWM_RSV];
	nwm_cmdhdr_t		header;
	uint16				buf[2];
} BWL_POST_PACKED_STRUCT nwm_cmd_ind_t;

/* MLME-Request */

#define	NWM_CMDCODE_MLME_RESET				(NWM_CMDGCODE_MLME  | 0x00)
#define	NWM_CMDCODE_MLME_PWRMGT				(NWM_CMDGCODE_MLME  | 0x01)
#define	NWM_CMDCODE_MLME_SCAN				(NWM_CMDGCODE_MLME  | 0x02)
#define	NWM_CMDCODE_MLME_JOIN				(NWM_CMDGCODE_MLME  | 0x03)
#define	NWM_CMDCODE_MLME_AUTH				(NWM_CMDGCODE_MLME  | 0x04)
#define	NWM_CMDCODE_MLME_DEAUTH				(NWM_CMDGCODE_MLME  | 0x05)
#define	NWM_CMDCODE_MLME_ASSOC				(NWM_CMDGCODE_MLME  | 0x06)
#define	NWM_CMDCODE_MLME_REASSOC			(NWM_CMDGCODE_MLME  | 0x07)
#define	NWM_CMDCODE_MLME_DISASSOC			(NWM_CMDGCODE_MLME  | 0x08)
#define	NWM_CMDCODE_MLME_START				(NWM_CMDGCODE_MLME  | 0x09)
#define	NWM_CMDCODE_MLME_MEASCH				(NWM_CMDGCODE_MLME  | 0x0A)
/* #define	WL_CMDCODE_MLME_BCLOST			(NWM_CMDGCODE_MLME  | 0x0B) */

/* MLME-Indication */
#define	NWM_CMDCODE_MLME_SCANCMPLT_IND		(NWM_CMDGCODE_MLME  | 0x82)
#define	NWM_CMDCODE_MLME_JOINCMPLT_IND		(NWM_CMDGCODE_MLME  | 0x83)
#define	NWM_CMDCODE_MLME_AUTH_IND			(NWM_CMDGCODE_MLME  | 0x84)
#define	NWM_CMDCODE_MLME_DEAUTH_IND			(NWM_CMDGCODE_MLME  | 0x85)
#define	NWM_CMDCODE_MLME_ASSOC_IND			(NWM_CMDGCODE_MLME  | 0x86)
#define	NWM_CMDCODE_MLME_REASSOC_IND		(NWM_CMDGCODE_MLME  | 0x87)
#define	NWM_CMDCODE_MLME_DISASSOC_IND		(NWM_CMDGCODE_MLME  | 0x88)
/* #define	NWM_CMDCODE_MLME_MEASCH_IND		(NWM_CMDGCODE_MLME  | 0x89) */
#define	NWM_CMDCODE_MLME_MEASCH_IND			(NWM_CMDGCODE_MLME  | 0x8A)
#define	NWM_CMDCODE_MLME_BCLOST_IND			(NWM_CMDGCODE_MLME  | 0x8B)
#define	NWM_CMDCODE_MLME_BCSEND_IND			(NWM_CMDGCODE_MLME  | 0x8C)
#define	NWM_CMDCODE_MLME_BCRECV_IND			(NWM_CMDGCODE_MLME  | 0x8D)

/* MA-Request */
#define	NWM_CMDCODE_MA_DATA					(NWM_CMDGCODE_MA    | 0x00)
#define	NWM_CMDCODE_MA_KEY					(NWM_CMDGCODE_MA    | 0x01)
#define	NWM_CMDCODE_MA_MP					(NWM_CMDGCODE_MA    | 0x02)
#define	NWM_CMDCODE_MA_TESTDATA				(NWM_CMDGCODE_MA    | 0x03)
#define	NWM_CMDCODE_MA_CLRDATA				(NWM_CMDGCODE_MA    | 0x04)

#define NWM_IS_PKT_SEND_CMD(code)	((code == NWM_CMDCODE_MA_DATA) || \
		(code == NWM_CMDCODE_MA_KEY) || (code == NWM_CMDCODE_MA_MP))

/* MA-Indicate */
#define	NWM_CMDCODE_MA_DATA_IND				(NWM_CMDGCODE_MA    | 0x80)
/* #define	NWM_CMDCODE_MA_KEY_IND			(NWM_CMDGCODE_MA    | 0x81) */
#define	NWM_CMDCODE_MA_MP_IND				(NWM_CMDGCODE_MA    | 0x82)
/* #define	NWM_CMDCODE_MA_TESTDATA_IND		(NWM_CMDGCODE_MA    | 0x83) */
#define	NWM_CMDCODE_MA_MPEND_IND			(NWM_CMDGCODE_MA    | 0x84)
#define	NWM_CMDCODE_MA_MPACK_IND			(NWM_CMDGCODE_MA    | 0x85)
#define	NWM_CMDCODE_MA_FATAL_ERR_IND		(NWM_CMDGCODE_MA    | 0x86)
#define	NWM_CMDCODE_MA_CHANNEL_USE_IND		(NWM_CMDGCODE_MA    | 0x90)
#define	NWM_CMDCODE_MA_IAPP_IND				(NWM_CMDGCODE_MA    | 0x91)

/* Set */
#define	NWM_CMDCODE_PARAM_SET_ALL			(NWM_CMDGCODE_PARAM | 0x00)
#define	NWM_CMDCODE_PARAM_SET_MAC_ADRS		(NWM_CMDGCODE_PARAM | 0x01)
#define	NWM_CMDCODE_PARAM_SET_RETRY_LIMIT	(NWM_CMDGCODE_PARAM | 0x02)
#define	NWM_CMDCODE_PARAM_SET_ENABLECHANNEL	(NWM_CMDGCODE_PARAM | 0x03)
#define	NWM_CMDCODE_PARAM_SET_MODE			(NWM_CMDGCODE_PARAM | 0x04)
#define	NWM_CMDCODE_PARAM_SET_RATESET		(NWM_CMDGCODE_PARAM | 0x05)
#define	NWM_CMDCODE_PARAM_SET_WEP_MODE		(NWM_CMDGCODE_PARAM | 0x06)
#define	NWM_CMDCODE_PARAM_SET_WEP_KEYID		(NWM_CMDGCODE_PARAM | 0x07)
#define	NWM_CMDCODE_PARAM_SET_WEP_KEY		(NWM_CMDGCODE_PARAM | 0x08)
#define	NWM_CMDCODE_PARAM_SET_BEACON_TYPE	(NWM_CMDGCODE_PARAM | 0x09)
#define	NWM_CMDCODE_PARAM_SET_PROBE_RES		(NWM_CMDGCODE_PARAM | 0x0A)
#define	NWM_CMDCODE_PARAM_SET_BEACON_LOST	(NWM_CMDGCODE_PARAM | 0x0B)
#define	NWM_CMDCODE_PARAM_SET_ACTIVE_ZONE	(NWM_CMDGCODE_PARAM | 0x0C)
#define	NWM_CMDCODE_PARAM_SET_SSID_MASK		(NWM_CMDGCODE_PARAM | 0x0D)
#define	NWM_CMDCODE_PARAM_SET_PREAMBLE_TYPE	(NWM_CMDGCODE_PARAM | 0x0E)
#define	NWM_CMDCODE_PARAM_SET_AUTHALGO		(NWM_CMDGCODE_PARAM | 0x0F)
#define	NWM_CMDCODE_PARAM_SET_CCAMODE		(NWM_CMDGCODE_PARAM | 0x10)
#define	NWM_CMDCODE_PARAM_SET_LIFETIME		(NWM_CMDGCODE_PARAM | 0x11)
#define	NWM_CMDCODE_PARAM_SET_MAXCONN		(NWM_CMDGCODE_PARAM | 0x12)
#define	NWM_CMDCODE_PARAM_SET_TXANT			(NWM_CMDGCODE_PARAM | 0x13)
#define	NWM_CMDCODE_PARAM_SET_DIVERSITY		(NWM_CMDGCODE_PARAM | 0x14)
#define	NWM_CMDCODE_PARAM_SET_BCNTXRX_IND	(NWM_CMDGCODE_PARAM | 0x15)
#define	NWM_CMDCODE_PARAM_SET_INTERFERENCE		(NWM_CMDGCODE_PARAM | 0x16)

#define	NWM_CMDCODE_PARAM_SET_BSSID			(NWM_CMDGCODE_PARAM | 0x40)
#define	NWM_CMDCODE_PARAM_SET_SSID			(NWM_CMDGCODE_PARAM | 0x41)
#define	NWM_CMDCODE_PARAM_SET_BEACON_PERIOD	(NWM_CMDGCODE_PARAM | 0x42)
#define	NWM_CMDCODE_PARAM_SET_DTIM_PERIOD	(NWM_CMDGCODE_PARAM | 0x43)
#define	NWM_CMDCODE_PARAM_SET_LISTEN_INT		(NWM_CMDGCODE_PARAM | 0x44)
#define	NWM_CMDCODE_PARAM_SET_GAME_INFO		(NWM_CMDGCODE_PARAM | 0x45)
#define	NWM_CMDCODE_PARAM_SET_VBLANK_TSF		(NWM_CMDGCODE_PARAM | 0x46)
#define	NWM_CMDCODE_PARAM_SET_MACLIST		(NWM_CMDGCODE_PARAM | 0x47)
#define	NWM_CMDCODE_PARAM_SET_RTS_THRESH		(NWM_CMDGCODE_PARAM | 0x48)
#define	NWM_CMDCODE_PARAM_SET_FRAG_THRESH	(NWM_CMDGCODE_PARAM | 0x49)
#define	NWM_CMDCODE_PARAM_SET_PMK			(NWM_CMDGCODE_PARAM | 0x4A)
#define	NWM_CMDCODE_PARAM_SET_EEROM_INFO		(NWM_CMDGCODE_PARAM | 0x4B)
/* 0x4C reserved */
#define	NWM_CMDCODE_PARAM_SET_TXPWR			(NWM_CMDGCODE_PARAM | 0x4D)
#define	NWM_CMDCODE_PARAM_SET_MCAST_RATE	(NWM_CMDGCODE_PARAM | 0x4E)

/* Get */
#define	NWM_CMDCODE_PARAM_GET_ALL			(NWM_CMDGCODE_PARAM | 0x80)
#define	NWM_CMDCODE_PARAM_GET_MAC_ADRS		(NWM_CMDGCODE_PARAM | 0x81)
#define	NWM_CMDCODE_PARAM_GET_RETRY_LIMIT	(NWM_CMDGCODE_PARAM | 0x82)
#define	NWM_CMDCODE_PARAM_GET_ENABLECHANNEL	(NWM_CMDGCODE_PARAM | 0x83)
#define	NWM_CMDCODE_PARAM_GET_MODE			(NWM_CMDGCODE_PARAM | 0x84)
#define	NWM_CMDCODE_PARAM_GET_RATESET		(NWM_CMDGCODE_PARAM | 0x85)
#define	NWM_CMDCODE_PARAM_GET_WEP_MODE		(NWM_CMDGCODE_PARAM | 0x86)
#define	NWM_CMDCODE_PARAM_GET_WEP_KEYID		(NWM_CMDGCODE_PARAM | 0x87)
#define	NWM_CMDCODE_PARAM_GET_WEP_KEY		(NWM_CMDGCODE_PARAM | 0x88)
#define	NWM_CMDCODE_PARAM_GET_BEACON_TYPE	(NWM_CMDGCODE_PARAM | 0x89)
#define	NWM_CMDCODE_PARAM_GET_PROBE_RES		(NWM_CMDGCODE_PARAM | 0x8A)
#define	NWM_CMDCODE_PARAM_GET_BEACON_LOST	(NWM_CMDGCODE_PARAM | 0x8B)
#define	NWM_CMDCODE_PARAM_GET_ACTIVE_ZONE	(NWM_CMDGCODE_PARAM | 0x8C)
#define	NWM_CMDCODE_PARAM_GET_SSID_MASK		(NWM_CMDGCODE_PARAM | 0x8D)
#define	NWM_CMDCODE_PARAM_GET_PREAMBLE_TYPE	(NWM_CMDGCODE_PARAM | 0x8E)
#define	NWM_CMDCODE_PARAM_GET_AUTHALGO		(NWM_CMDGCODE_PARAM | 0x8F)
#define	NWM_CMDCODE_PARAM_GET_CCAMODE		(NWM_CMDGCODE_PARAM | 0x90)
/* #define	WL_CMDCODE_PARAM_GET_LIFETIME	(NWM_CMDGCODE_PARAM | 0x91) */
#define	NWM_CMDCODE_PARAM_GET_MAXCONN		(NWM_CMDGCODE_PARAM | 0x92)
#define	NWM_CMDCODE_PARAM_GET_TXANT			(NWM_CMDGCODE_PARAM | 0x93)
#define	NWM_CMDCODE_PARAM_GET_DIVERSITY		(NWM_CMDGCODE_PARAM | 0x94)
#define	NWM_CMDCODE_PARAM_GET_BCNTXRX_IND	(NWM_CMDGCODE_PARAM | 0x95)
#define	NWM_CMDCODE_PARAM_GET_INTERFERENCE			(NWM_CMDGCODE_PARAM | 0x96)

#define	NWM_CMDCODE_PARAM_GET_BSSID			(NWM_CMDGCODE_PARAM | 0xC0)
#define	NWM_CMDCODE_PARAM_GET_SSID			(NWM_CMDGCODE_PARAM | 0xC1)
#define	NWM_CMDCODE_PARAM_GET_BEACON_PERIOD	(NWM_CMDGCODE_PARAM | 0xC2)
#define	NWM_CMDCODE_PARAM_GET_DTIM_PERIOD	(NWM_CMDGCODE_PARAM | 0xC3)
#define	NWM_CMDCODE_PARAM_GET_LISTEN_INT		(NWM_CMDGCODE_PARAM | 0xC4)
#define	NWM_CMDCODE_PARAM_GET_GAME_INFO		(NWM_CMDGCODE_PARAM | 0xC5)
#define	NWM_CMDCODE_PARAM_GET_VBLANK_TSF		(NWM_CMDGCODE_PARAM | 0xC6)
#define	NWM_CMDCODE_PARAM_GET_MACLIST		(NWM_CMDGCODE_PARAM | 0xC7)
#define	NWM_CMDCODE_PARAM_GET_RTS_THRESH		(NWM_CMDGCODE_PARAM | 0xC8)
#define	NWM_CMDCODE_PARAM_GET_FRAG_THRESH	(NWM_CMDGCODE_PARAM | 0xC9)
#define	NWM_CMDCODE_PARAM_GET_EEROM_INFO		(NWM_CMDGCODE_PARAM | 0xCB)
#define	NWM_CMDCODE_PARAM_GET_RSSI_INFO		(NWM_CMDGCODE_PARAM | 0xCC)
#define	NWM_CMDCODE_PARAM_GET_TXPWR			(NWM_CMDGCODE_PARAM | 0xCD)
#define	NWM_CMDCODE_PARAM_GET_MCAST_RATE	(NWM_CMDGCODE_PARAM | 0xCE)

/* Device commands */
#define	NWM_CMDCODE_DEV_SHUTDOWN				(NWM_CMDGCODE_DEV   | 0x01)
#define	NWM_CMDCODE_DEV_IDLE					(NWM_CMDGCODE_DEV   | 0x02)
#define	NWM_CMDCODE_DEV_CLASS1				(NWM_CMDGCODE_DEV   | 0x03)
#define	NWM_CMDCODE_DEV_RESTART				(NWM_CMDGCODE_DEV   | 0x04)
#define	NWM_CMDCODE_DEV_INIT_WLCOUNTERS			(NWM_CMDGCODE_DEV   | 0x05)
#define	NWM_CMDCODE_DEV_GET_VERINFO			(NWM_CMDGCODE_DEV   | 0x06)
#define	NWM_CMDCODE_DEV_GET_INFO				(NWM_CMDGCODE_DEV   | 0x07)
#define	NWM_CMDCODE_DEV_GET_STATE			(NWM_CMDGCODE_DEV   | 0x08)
#define	NWM_CMDCODE_DEV_TEST_SIGNAL			(NWM_CMDGCODE_DEV   | 0x09)

/* Bases */
#define	NWM_CMDGCODE_MLME					0x0000
#define	NWM_CMDGCODE_MA						0x0100
#define	NWM_CMDGCODE_PARAM					0x0200
#define	NWM_CMDGCODE_DEV						0x0300

/* Masks */
#define	NWM_CMDSCODE_MASK		0x00FF	/* invalid bits */
#define	NWM_CMDGCODE_MASK		0xFF00

/* Fixups for the "gaps" in the set/get tables */
#define	NWM_SET_STR_OFST					(0x00)
#define	NWM_SET2_STR_OFST					(0x40)
#define	NWM_GET_STR_OFST					(0x80)
#define	NWM_GET2_STR_OFST					(0xC0)

/* return codes */
/* Command results */
#define	NWM_CMDRES_SUCCESS					0x00
#define NWM_CMDRES_OPERATING				0x80
#define	NWM_CMDRES_STATE_WRONG				0x01
#define	NWM_CMDRES_REQUEST_BUSY				0x02
#define	NWM_CMDRES_NOT_SUPPORT				0x03
#define	NWM_CMDRES_LENGTH_ERR				0x04
#define	NWM_CMDRES_INVALID_PARAM			0x05
#define	NWM_CMDRES_REFUSE					0x06
#define	NWM_CMDRES_TIMEOUT					0x07
#define	NWM_CMDRES_NOT_ENOUGH_MEM			0x08
#define	NWM_CMDRES_NOT_ENOUGH_PARAM			0x09
#define	NWM_CMDRES_NOT_CLASS3_STA_FRAME		0x0A
#define	NWM_CMDRES_ILLEGAL_MODE				0x0B
#define	NWM_CMDRES_FAILURE					0x0C
#define	NWM_CMDRES_CONFIRM_CODE_ERR			0x0D
#define	NWM_CMDRES_FLASH_ERR				0x0E

/* Reasons */
#define NWM_RSN_RESERVED						0x00
#define NWM_RSN_UNSPECIFIED					0x01
#define NWM_RSN_PREV_AUTH_INVALID			0x02
#define NWM_RSN_DEAUTH_LEAVING				0x03
#define NWM_RSN_INACTIVE						0x04
#define NWM_RSN_UNABLE_HANDLE				0x05
#define NWM_RSN_RX_CLASS2_FROM_NONAUTH_STA	0x06
#define NWM_RSN_RX_CLASS3_FROM_NONASSOC_STA	0x07
#define NWM_RSN_DISASSOC_LEAVING			0x08
#define NWM_RSN_ASSOC_STA_NOTAUTHED			0x09
#define NWM_RSN_NO_ENTRY					0x13

/* Status results */
#define NWM_STS_SUCCESS						0x00
#define NWM_STS_UNSPECIFIED					0x01
/* 0x02 --> 0x09 reserved */
#define NWM_STS_NOT_SUPPORT_CAPABILITY		0x0A
#define NWM_STS_REASSOC_INABILITY			0x0B
#define NWM_STS_OUT_OF_STANDARD				0x0C
#define NWM_STS_NOT_SUPPORT_AUTH_ALGORITHM	0x0D
#define NWM_STS_OUT_OF_AUTH_SEQ_NUM			0x0E
#define NWM_STS_CHALLENGE_FAILURE			0x0F
#define NWM_STS_AUTH_TIMEOUT				0x10
#define NWM_STS_ASSOC_UNABLE_HANDLE			0x11
#define NWM_STS_INVALID_BASICRATESET		0x12
#define NWM_STS_NO_ENTRY					0x13

/* Fatal error indication codes */
#define	NWM_TX_ENQUEUE_ERR					0x00
#define	NWM_KEYDATAREQ_FAILURE				0x01
#define	NWM_MPREQ_FAILURE					0x02

/* cmd error mask */
#define	WL_CMDRES_ERR_MASK			0x7F

/* Other useful stuff */
/* These are for encapsulating requests, valid for majority of "simple"
 * commands by virtue of padding req and cfm fields
 */
#define NWM_MIN_REQ_SIZE		(sizeof(nwm_cmd_req_t))
#define NWM_MIN_CFM_SIZE		(sizeof(nwm_cmd_cfm_t))
#define	NWM_MIN_PKT_SIZE	(NWM_MIN_REQ_SIZE + NWM_MIN_CFM_SIZE)

/* absolute barebones sizes for encapsulating requests */
#define NWM_MIN_GET_REQ_SIZE	(sizeof(nwm_get_req_t))
#define NWM_MIN_SET_CFM_SIZE	(sizeof(nwm_set_cfm_t))
#define NWM_MIN_ENCAP_SIZE		(NWM_MIN_GET_REQ_SIZE + NWM_MIN_SET_CFM_SIZE)

/*
 * 0000h: Test Mode
 * 0001h: Parent Mode
 * 0002h: Child Mode
 * 0003h:  STA Infrastructure Mode  (default)
 * 0004h:  STA Ad hoc mode
 * 0005h: AP mode
 * 0006h: Travel Router
 *
 *
 */

/* Operating modes */
#define NWM_NIN_TESTMODE				0
#define NWM_NIN_NITRO_PARENT_MODE		1
#define NWM_NIN_NITRO_CHILD_MODE		2
#define NWM_NIN_STA_INFRA_MODE			3
#define NWM_NIN_STA_IBSS_MODE			4
#define NWM_NIN_AP_MODE					5
#define NWM_NIN_URE_MODE				6
#define NWM_NIN_MODE_UNKNOWN			7

#define NWM_IS_AP_PARENT(mode) 		((mode == NWM_NIN_NITRO_PARENT_MODE) || \
		(mode == NWM_NIN_AP_MODE))

/* Security values */
#define NWM_NIN_NO_ENCRYPTION	0
#define NWM_NIN_WEP_40_BIT		1
#define NWM_NIN_WEP_104_BIT		2
#define NWM_NIN_WSEC_RESERVED	3
#define NWM_NIN_WPA_PSK_TKIP	4
#define NWM_NIN_WPA2_PSK_AES	5
#define NWM_NIN_WPA_PSK_AES		6

/* Operating states */
#define NWM_NIN_IDLE	0x0010
#define NWM_NIN_CLASS1	0x0020
#define NWM_NIN_CLASS2	0x0030
#define NWM_NIN_CLASS3	0x0040

/* convenience for channel cmd manipulations */
#define NWM_MAX_CHANNEL				14
#define NWM_MIN_CHANNEL				1

#define NWM_NIN_MAGIC	0xdeadbeef

/* IOPOS related items */
#if defined(TARGETENV_linux)
#define NWM_SIOCDEVPRIVATE		(SIOCDEVPRIVATE)
#else
#define NWM_SIOCDEVPRIVATE	(1)
#endif // endif

#define NWM_SIOCDEVPRIVATE_NINTENDO		(NWM_SIOCDEVPRIVATE + 1)
#define NWM_SIOCDEVPRIVATE_IOPOS		(NWM_SIOCDEVPRIVATE + 2)

#define WL_IFNAME_STRING	"/dev/wl"

/* commands for NWM_SIOCDEVPRIVATE_IOPOS */
#define NWM_IOPOS_SET_SENDUP_QID		0

#define WLNIN_LISTEN_QSIZE				5

/* taken from bcmdefs.h */
#define NWM_PKT_HDROOM		180

typedef struct nwm_iop_ioctl {
	char ifname[32];
	int  sendup_qid;
} nwm_iop_ioctl_t;

/* This marks the end of a packed structure section. */
#include <packed_section_end.h>

#endif	/* _NINTENDOWM_H */
