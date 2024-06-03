/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef __IEEE80211V_DEFS_H__
#define __IEEE80211V_DEFS_H__

#define WIDE_BANDWIDTH_CHANNEL 0x06
#define BSS_TRANSITION_CANDIDATE_PREF 0x03
#define SKIP_EID_LEN 2

struct wifi_app *wapp;
struct wapp_dev *wdev;

enum wnm_ie_eid {
	IE_TIME_ADVERTISEMENT = 69,
	IE_TIME_ZONE = 98,
};

enum {
	BTM_STA_STEERING = 0,
	BTM_DISASSOC_STA,
	BTM_AP_TERMINATE,
	BTM_STA_STEER_TO_CELL,
	BTM_QUERY_RSP,
};

enum transition_query_reason {
    UNSPECIFIED,
    EXCESSIVE_FRAME_LOSS_RATE,
    EXCESSIVE_DELAY,
    INSUFFICIENT_QOS_CAPACITY,
    FIRST_ASSOCIATION_TO_ESS,
    LOAD_BALANCING,
    BETTER_AP_FOUND,
    DEAUTHENTICATED_OR_DISASSOCIATED_FROM_PREVIOUS_AP,
    AP_FAILED_IEEE8021X_EAP_AUTH,
    AP_FAILED_FOUR_WAY_HANDSHAKE,
    RECEIVED_TOO_MANY_REPLAY_COUNTER_FAILURE,
    RECEIVED_TOO_MANY_DATA_MIC_FAILURE,
    EXCEEDED_MAXIMUM_NUMBER_OF_RETRANSMISSIONS,
    RECEIVED_TOO_MANY_BROADCAST_DISASSOCIATIONS,
    RECEIVED_TOO_MANY_BROADCAST_DEAUTHENTICATION,
    PREVIOUS_TRANSITION_FAILED,
    LOW_RSSI,
    ROAM_FROM_A_NON_80211_SYSTEM,
    RECEIVED_BSS_TRANSITION_REQ_FRAME,
    PREFERRED_BSS_TRANSITION_CANDIDATE_LIST_INCLUDED,
    LEAVING_ESS,
};

/* IEEE Std 802.11-2012 - Table 8-253 */
enum bss_trans_mgmt_status_code {
	WNM_BSS_TM_ACCEPT = 0,
	WNM_BSS_TM_REJECT_UNSPECIFIED,
	WNM_BSS_TM_REJECT_INSUFFICIENT_BEACON,
	WNM_BSS_TM_REJECT_INSUFFICIENT_CAPABITY,
	WNM_BSS_TM_REJECT_UNDESIRED,
	WNM_BSS_TM_REJECT_DELAY_REQUEST,
	WNM_BSS_TM_REJECT_STA_CANDIDATE_LIST_PROVIDED,
	WNM_BSS_TM_REJECT_NO_SUITABLE_CANDIDATES,
	WNM_BSS_TM_REJECT_LEAVING_ESS,
};

enum neighbor_report_subelement_id {
    RESERVED,
    TSF_INFORMATION,
    CONDENSED_COUNTRY_STRING,
    BSS_TRANSITION_CANDIDATE_PREFERENCE,
    BSS_TERMINATION_DURATION,
    BEARING,
    MEASUREMENT_PILOT_TRANSMISSION_INFO = 66,
    RPM_ENABLE_CAPABILITIES = 70,
    MULTIPLE_BSSID,
    VENDOR_SPECIFIC_FIELD = 221,
};

enum btm_req_mode_bit_map {
    CAND_LIST_INCLUDED_BIT_MAP,
	ABIDGED_BIT_MAP,
	DISASSOC_IMNT_BIT_MAP,
	BSS_TERM_INCLUDED_BIT_MAP,
	ESS_DISASSOC_IMNT_BIT_MAP,
};

struct time_advertisement_element {
	u8 eid;
	u8 length;
	u8 timing_capabilities;
	/*
 	 * Following are Time value, TIme Error, and Time Update Counter
 	 */ 
	u8 variable[0];
} __attribute__ ((packed));

struct time_zone_element {
	u8 eid;
	u8 length;
	/*
 	 * Following are Time Zone
 	 */
	u8 variable[0]; 
} __attribute__ ((packed));

struct btm_payload {
	union {
		struct {
			u8 btm_query_reason;
			/*
 			 * Following are BSS Transition Candidates List Entries
 			 */
			u8 variable[0]; 
		} __attribute__ ((packed)) btm_query;

		struct {
			u8 request_mode;
			u16 disassociation_timer;
			u8 validity_interval;
			/* 
 			 * Following are BSS Termination Duration, Session Information URL,
 			 * and BSS Transition Candidates List Entries
 			 */
			u8 variable[0];
		} __attribute__((packed)) btm_req;

		struct {
			u8 status_code;
			u8 bss_termination_delay;
			/*
 			 * Following are Target BSSID, and BSS Transition Candidates List Entries
 			 */
			u8 variable[0];
		} __attribute__ ((packed)) btm_rsp;
	}u;
} __attribute__ ((packed));

struct neighbor_report_subelement {
	u8 subelement_id;
	u8 length;
	union {
		struct {
			u8 preference;
		} __attribute__ ((packed)) bss_transition_candi_preference;

		struct {
			u64 bss_termination_tsf;
			u16 duration;
		} __attribute__ ((packed)) bss_termination_duration;

		struct {
			u16 bearing;
			u32 distance;
			u16 relative_height;
		} __attribute__ ((packed)) bearing;
	}u;
} __attribute__ ((packed));

struct neighbor_report_element {
	u8 eid;
	u8 length;
	u8 bssid[6];
	u32 bss_info;
	u8 regulatory_class;
	u8 channel_number;
	u8 phy_type;
	/*
 	 * Following are Optional sub elements
 	 */
	u8 variable[0];
} __attribute__ ((packed));

struct bss_transition_candi_preference_unit {
	struct dl_list list;
	u8 preference;
};

struct btm_cfg {
#if 1 /* for test purpose */	
	u8 bss_termination_onoff;
	u8 disassoc_imnt;			/* disassoc imnt bit in btm, for test purpose */
	u8 nebor_bssid[6];	
	u8 nebor_op_class;
	u8 nebor_op_ch;
	u8 nebor_pref;
#endif
};


/* MBO UCC CMD */
#define TEST_EVENT_STA_STEERING(_btm) \
        (_btm->disassoc_imnt == 0 && _btm->bss_termination_onoff == 0)
#define TEST_EVENT_STA_DISASSOC(_btm) \
        (_btm->disassoc_imnt == 1 && _btm->bss_termination_onoff == 0)
#define TEST_EVENT_BSS_TERM(_btm) \
        (_btm->bss_termination_onoff == 1)
#define TEST_EVENT_STA_TO_CELL(_btm) \
        (0) 

int wapp_send_btm_req_by_case(
    struct wifi_app *wapp,      
    const char *iface,
    const u8 *mac_addr,                                                             
    u8  trans_case);

int wapp_send_btm_req(struct wifi_app *wapp,
						 const char *iface,
						 const u8 *peer_mac_addr,
						 const char *btm_req,
						 size_t btm_req_len);

int wapp_send_btm_rsp(struct wifi_app *wapp,
		struct wapp_dev *wdev,
		const char *iface,
		enum bss_trans_mgmt_status_code status,
		u8 bss_term_delay, const u8 *peer_mac_addr,
		const u8 *target_bssid, u8 ChNum);

size_t wapp_build_btm_req(
	u8 req_mode,
	u16 disassoc_timer,
	u8 vad_intvl,
	struct neighbor_report_subelement *bss_term_dur,
	char *url,
	size_t	 url_len,
	char *cand_list,
	size_t cand_list_len,
	char *btm_req_buf);

int wapp_parse_nr_elem(
		struct wifi_app *wapp, void *nr_elem);

int wapp_event_btm_req(struct wifi_app *wapp,
		const char *iface,
		const u8 *peer_mac_addr,
		const char *btm_req,
		size_t btm_req_len);

int wapp_event_btm_rsp(struct wifi_app *wapp,
						  const char *iface,
						  const u8 *peer_mac_addr,
						  const char *btm_rsp,
						  size_t btm_rsp_len);
						  
int wapp_event_btm_query(struct wifi_app *wapp,
						    const char *iface,
						    const u8 *peer_addr,
						    const char *btm_query,
						    size_t btm_query_len);

int wapp_send_wnm_notify_req(struct wifi_app *wapp, const char *iface,
						 const char *peer_mac_addr,
						 const char *wnm_req,
						 size_t wnm_req_len,
						 int	type);

int wapp_send_reduced_nr_list_by_inf(
    struct wifi_app *wapp,      
    const char *iface);

int wapp_send_reduced_nr_list(struct wifi_app *wapp,
						 const char *iface,
						 const char *reduced_nr_list,
						 size_t reduced_nr_list_len);

size_t wapp_build_reduced_nr_list(
	char *cand_list,
	size_t cand_list_len,
	char *reduced_nr_list_buf);

#endif /* __IEEE80211V_DEFS_H__ */
