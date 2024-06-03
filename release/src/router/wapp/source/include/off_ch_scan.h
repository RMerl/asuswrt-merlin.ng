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

#ifndef _OFF_CH_SCAN_H_
#define _OFF_CH_SCAN_H_

#include "stddef.h"
#include "list.h"
#include "types.h"
#include "os.h"
#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN				6
#endif

#ifndef MAX_LEN_OF_SSID
#define MAX_LEN_OF_SSID 32
#endif

#define MAX_AWAY_CHANNEL		5
#define CH_SCAN_TIMEOUT			60
#define MAX_TS_LEN				30
#define MAX_CH_BW_LEN			10
#define MAX_CH_NUM				13
#define MAX_OP_CLASS_NUM 		6


enum ASYNC_OFFCHANNEL_COMMAND_RSP {
	GET_OFFCHANNEL_INFO = 34,
	OFFCHANNEL_INFO_RSP,
	TRIGGER_DRIVER_CHANNEL_SWITCH,
	UPDATE_DRIVER_SORTED_CHANNEL_LIST,
	DFS_DRIVER_CHANNEL_SWITCH,
	DFS_RADAR_HIT,
	DFS_CHANNEL_NOP_CMPLT,
	DRIVER_CHANNEL_SWITCH_SUCCESSFUL
};
enum off_ch_scan_status_code {
	SCAN_SUCCESS,
	OP_CLASS_CHAN_NOT_SUPP,
	REQ_TOO_SOON,
	RADIO_BUSY,
	SCAN_INCOMPLETE_LESS_TIME,
	ABORT_SCAN
};

struct GNU_PACKED timestamp_tlv {
	u8 tlvType;
	u16 tlvLen;
	u8 timestamp_len;
	char timestamp[MAX_TS_LEN];
};


struct GNU_PACKED off_ch_scan_report {
	struct timestamp_tlv timestamp;
	u8 scan_result_num;
	struct dl_list scan_result_list;  //struct scan_result_tlv
};

struct GNU_PACKED neighbor_info {
	u8 bssid[MAC_ADDR_LEN];
	u8 ssid_len;
	u8 ssid[MAX_LEN_OF_SSID];
	u8 RCPI;
	u8 ch_bw_len;
	u8 ch_bw[MAX_CH_BW_LEN];
	u8 cu_stacnt_present; //bit7 : CU, bit-6 Stacnt
	u8 cu;
	u16 sta_cnt;
};

struct GNU_PACKED net_opt_scan_result_event {
	u8 radio_id[MAC_ADDR_LEN];
	u8 oper_class; //R2
	u8 channel;
	u8 scan_status; //R2
	u8 timestamp_len;//R2
	u8 timestamp[MAX_TS_LEN];//R2
	u8 utilization;
	u8 noise;
	u32 agg_scan_duration;//R2
	u8 scan_type;//R2
	u16 neighbor_num;
#ifdef MAP_R2
	u32 rx_time;
	u32 tx_time;
	u32 obss_time;
	u32 edcca;
#endif
	struct neighbor_info nb_info[0];//struct neighbor_info *nb_info;
};
struct GNU_PACKED off_ch_scan_result_event {
	u8 radio_id[MAC_ADDR_LEN];
	u8 channel;
	u8 utilization;
	u8 noise;
	u16 neighbor_num;
};
struct GNU_PACKED net_opt_scan_report_event {
#ifdef MAP_R2
	struct timestamp_tlv timestamp;
#endif
	u8 scan_result_num;
	struct net_opt_scan_result_event scan_result[0]; 
};

struct GNU_PACKED off_ch_scan_report_event {
	u8 scan_result_num;
	struct off_ch_scan_result_event scan_result[0];  //struct scan_result_tlv
};
struct global_oper_class {
	unsigned char opclass;	/* regulatory class */
	unsigned char channel_num;
	unsigned char channel_set[MAX_CH_NUM];	/* max 13 channels, use 0 as terminator */
};

struct GNU_PACKED off_ch_scan_result_tlv {
	struct dl_list list;
	u8 radio_id[MAC_ADDR_LEN];
	u8 oper_class;
	u8 channel;
	u8 scan_status;
	u8 timestamp_len;
	u8 timestamp[MAX_TS_LEN];
	u8 utilization;
	u8 noise;
	u32 agg_scan_duration;
	u8 scan_type;
	u16 neighbor_num;
#ifdef MAP_R2
	u32 rx_time;
	u32 tx_time;
	u32 obss_time;
	u32 edcca;
#endif
	struct neighbor_info *nb_info;
};

struct GNU_PACKED channel_body {
	u8 oper_class;
	u8 ch_list_num;
	u8 ch_list[MAX_CH_NUM];
};

struct GNU_PACKED off_ch_scan_body {
	u8 radio_id[MAC_ADDR_LEN];
	u8 oper_class_num;
	struct channel_body ch_body[MAX_OP_CLASS_NUM];
};

#define MAX_OFF_CH_SCAN_CH	8
#define SCAN_MODE_BAND		1
#define SCAN_MODE_CH		0
struct GNU_PACKED off_ch_scan_req_msg_s {
	unsigned char mode;
	unsigned char band;
	unsigned char ch_list[MAX_OFF_CH_SCAN_CH];
	unsigned char bw;
};

struct GNU_PACKED off_ch_scan_req_s {
	unsigned char bw;
	u8 fresh_scan; // 8: perform fresh scan, 0: return stored result
	u8 radio_num;
	u8 neighbour_only;
	struct off_ch_scan_body body[0];
};


struct off_ch_scan_radio_state {
	u8 radio_id[MAC_ADDR_LEN];
	struct os_time last_scan_req;
};

typedef struct GNU_PACKED _off_channel_info {
	u8	channel;
	u8	channel_idx;
	s32	NF;
	u32  rx_time;
	u32	tx_time;
	u32	obss_time;
	u32	channel_busy_time;
	u8	dfs_req;
	u8 	actual_measured_time;
#ifdef MAP_R2
	u32 edcca;
#endif
} OFFCHANNEL_INFO, *POFFCHANNEL_INFO;

typedef struct GNU_PACKED operating_info {
	u8 channel;
	u8 cfg_ht_bw;
	u8 cfg_vht_bw;
	u8 RDDurRegion;
	u8 region;
	u8 is4x4Mode;
	u8 vht_cent_ch2;
} OPERATING_INFO, *POPERATING_INFO;

typedef struct GNU_PACKED offchannel_param {
	u8 channel[MAX_AWAY_CHANNEL];
	u8 scan_type[MAX_AWAY_CHANNEL];
	u8 scan_time[MAX_AWAY_CHANNEL];
	u8 bw;
	u32 Num_of_Away_Channel;
} OFFCHANNEL_SCAN_PARAM, *POFFCHANNEL_SCAN_PARAM;

typedef struct GNU_PACKED sorted_list_info {
	u8 size;
	u8 SortedMaxChannelBusyTimeList[MAX_NUM_OF_WAPP_CHANNELS+1];
	u8 SortedMinChannelBusyTimeList[MAX_NUM_OF_WAPP_CHANNELS+1];

} SORTED_CHANNEL_LIST, *PSORTED_CHANNEL_LIST;

typedef struct GNU_PACKED _OFFCHANNEL_SCAN_MSG {
u8   Action;
u8 ifrn_name[32];
u32 ifIndex;
union {
				OFFCHANNEL_INFO channel_data;
				OFFCHANNEL_SCAN_PARAM offchannel_param;
				OPERATING_INFO operating_ch_info;
				SORTED_CHANNEL_LIST sorted_channel_list;
} data;
} OFFCHANNEL_SCAN_MSG, *POFFCHANNEL_SCAN_MSG;

enum off_ch_scan_state {
	CH_SCAN_IDLE,
	CH_SCAN_ONGOING,
	CH_ONBOOT_SCAN_ONGOING
};

#ifndef MAP_MAX_RADIO
#define MAP_MAX_RADIO 3
#endif
#define OP_CLASS_PER_RADIO 6

struct off_ch_scan_state_ctrl {
	enum off_ch_scan_state ch_scan_state;
	u8 curr_scan_radio_idx;
	u8 curr_oper_class_idx;
	u8 curr_ch_idx;
	u8 curr_chan_num;
	u8 ch_per_op_left;
	struct off_ch_scan_radio_state radio_state[MAP_MAX_RADIO];
	struct timestamp_tlv last_scan_tm;
	OFFCHANNEL_SCAN_MSG scan_msg;
};
struct GNU_PACKED radio_scan_capab {
	u8 radio_id[MAC_ADDR_LEN];
	u8 boot_scan_only;
	u8 scan_impact;
	u32 min_scan_interval;
	u8 oper_class_num;
	struct channel_body ch_body[OP_CLASS_PER_RADIO];
};

struct GNU_PACKED off_ch_scan_capab {
	u8 radio_num;
	struct radio_scan_capab radio_scan_params[MAP_MAX_RADIO];
};

typedef struct _channel_info {
	u8	channel;
	u8	channel_idx;
	s32	NF;
	u32  rx_time;
	u32	tx_time;
	u32	obss_time;
	u32	channel_busy_time;
	u8	dfs_req;
	u8 	actual_measured_time;
} CHANNEL_INFO, *PCHANNEL_INFO;

void map_prepare_off_channel_scan_req(struct wifi_app *wapp,
	char *band_ptr, unsigned short type);
void wdev_handle_off_ch_scan_results(struct wifi_app *wapp,
	u32 ifindex, wapp_event_data *event_data);
int wapp_event_offchannel_info(struct wifi_app *wapp, u8* buf);
int get_valid_radio_count (struct wifi_app *wapp);

#endif
