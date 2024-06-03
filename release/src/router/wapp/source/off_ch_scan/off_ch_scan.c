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

#include "wapp_cmm.h"
#include "off_ch_scan.h"
#include "map.h"
#include "wps.h"

void map_build_and_send_off_ch_scan_rep(struct wifi_app *wapp);

/*Global operating classes*/
struct global_oper_class oper_class[] = {
	{0, 0, {0}},		/* Invlid entry */
	{81, 13, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}},
	{82, 1, {14}},
	{83, 9, {1, 2, 3, 4, 5, 6, 7, 8, 9}},
	{84, 9, {5, 6, 7, 8, 9, 10, 11, 12, 13}},
	{94, 2, {133, 137}},
	{95, 4, {132, 134, 136, 138}},
	{96, 8, {131, 132, 133, 134, 135, 136, 137, 138}},
	{101, 2, {21, 25}},
	{102, 5, {11, 13, 15, 17, 19}},
	{103, 10, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}},
	{104, 2, {184, 192}},
	{105, 2, {188, 196}},
	{106, 2, {191, 195}},
	{107, 5, {189, 191, 193, 195, 197}},
	{108, 10, {188, 189, 190, 191, 192, 193, 194, 195, 196, 197}},
	{109, 4, {184, 188, 192, 196}},
	{110, 7, {183, 184, 185, 186, 187, 188, 189}},
	{111, 8, {182, 183, 184, 185, 186, 187, 188, 189}},
	{112, 3, {8, 12, 16}},
	{113, 5, {7, 8, 9, 10, 11}},
	{114, 6, {6, 7, 8, 9, 10, 11}},
	{115, 4, {36, 40, 44, 48}},
	{116, 2, {36, 44}},
	{117, 2, {40, 48}},
	{118, 4, {52, 56, 60, 64}},
	{119, 2, {52, 60}},
	{120, 2, {56, 64}},
	{121, 12, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144}},
	{122, 6, {100, 108, 116, 124, 132, 140}},
	{123, 6, {104, 112, 120, 128, 136, 144}},
	{124, 4, {149, 153, 157, 161}},
	{125, 6, {149, 153, 157, 161, 165, 169}},
	{126, 2, {149, 157}},
	{127, 2, {153, 161}},
	{128, 6, {42, 58, 106, 122, 138, 155}},
	{129, 2, {50, 114}},
	{130, 6, {42, 58, 106, 122, 138, 155}},
	{0, 0, {0}}		/* end */
};

unsigned char opclass_2g[] = {
	81, 82, 83, 84, 101, 102, 103, 112, 113, 114, 0
};

unsigned char opclass_5gh[] = {
	94, 95, 96, 104, 105, 106, 107, 108, 109, 110, 111, 121, 122,
	123, 124, 125, 126, 127, 0
};

unsigned char opclass_5gl[] = {
	115, 116, 117, 118, 119, 120, 0
};

u8 Cfg80211_RadarChan[] = {
	52, 54, 56, 60, 62, 64, 100, 104,
};



static u8 IsRadarChannel(u8 ch)
{
	UINT idx = 0;

	for (idx = 0; idx < sizeof(Cfg80211_RadarChan); idx++) {
		if (Cfg80211_RadarChan[idx] == ch)
			return TRUE;
	}
	return FALSE;
}


// fill all the primary channels of a particular channel given the operating class.
u32 wapp_fill_ch_list(struct wifi_app *wapp, OFFCHANNEL_SCAN_PARAM *scan_param,u8 op_class, u8 chan)
{
	u8 num_primary_channels = 0;
	u8 i = 0;
	if(op_class == 128 || op_class == 130)
		num_primary_channels = 4; // 80MHz or 80+80MHz
	else if (op_class == 129)
		num_primary_channels = 8; //160MHz
	else
		num_primary_channels = 1; //20/40MHz

	
	for (i = 0; i < num_primary_channels; i++) {
		u8 primary_chan = chan - (2 * num_primary_channels - 2) + i * 4;
		scan_param->channel[i] = primary_chan;
		scan_param->scan_type[i] = IsRadarChannel(primary_chan)?0x80:0;
		scan_param->scan_time[i] = 120;
	}
	return num_primary_channels;
}

struct global_oper_class *wapp_get_global_op_class(u8 op_class)
{
	u8 i =1;
	while(oper_class[i].opclass != 0) {
		if(oper_class[i].opclass == op_class) {
			break;
		}
		i++;
	}
	if(oper_class[i].opclass == 0)
		return NULL;
	else
		return &oper_class[i];
}

u8 is_chan_op_class_supported(struct wifi_app *wapp, u8 ch, u8 op_class)
{
	struct off_ch_scan_capab *scan_capab = wapp->map->off_ch_scan_capab;
	u8 i,j,k, ch_num, *ch_list = NULL;
	struct global_oper_class *op = NULL;

	if(scan_capab == NULL)
		return FALSE;

	for (i=0; i< scan_capab->radio_num; i++) {

		for (j=0; j< scan_capab->radio_scan_params[i].oper_class_num; j++) {

			if(op_class == scan_capab->radio_scan_params[i].ch_body[j].oper_class) {
				if(scan_capab->radio_scan_params[i].ch_body[j].ch_list_num == 0) {
					op = wapp_get_global_op_class(op_class);
					if(op == NULL)
						return FALSE;
					ch_num = op->channel_num;
					ch_list = op->channel_set;
				}
				else {
					ch_num = scan_capab->radio_scan_params[i].ch_body[j].ch_list_num;
					ch_list = scan_capab->radio_scan_params[i].ch_body[j].ch_list;
				}

				for(k=0; k< ch_num; k++) {
					if(ch_list[k] == ch)
						return TRUE;
				}
			}
		}
	}
	return FALSE;
}

u8 map_is_off_ch_scan_allowed(struct wifi_app *wapp,
	struct wapp_dev *wdev,
	u8 ch,u8 op_class,
	u8 *status)
{
#ifdef MAP_R2
	struct os_time now;
	wapp_device_status *device_status = &wapp->map->device_status;
#endif
	// operating class not supported
	// channel not supported
	//request too soon
	//radio too busy
	//stored result not availiable
	*status = SCAN_SUCCESS;
#ifdef MAP_R2
	os_get_time(&now);
	if (wapp->map->msg_info.enqueue_pending_msg != 0 ) {
		printf("abort scan\n");
		*status = ABORT_SCAN;
	}
	else if ((wdev->radio->last_scan_time.sec + wdev->radio->min_scan_interval > now.sec) && (wapp->map->msg_info.ignore_req_too_soon == 0)) {
		*status = REQ_TOO_SOON;
	} 
	else if (is_chan_op_class_supported(wapp,ch, op_class) == FALSE) {
		*status = OP_CLASS_CHAN_NOT_SUPP;
	}
	else if ((wapp->map->wps_after_scan == 1)
		&& (device_status->status_fhbss == STATUS_FHBSS_WPS_TRIGGERED)){
		*status = RADIO_BUSY;
		wapp->map->wps_after_scan = 0;
		wps_ctrl_run_ap_wps(wapp);
		eloop_cancel_timeout(map_wps_timeout, wapp, device_status);
		eloop_register_timeout(WPS_TIMEOUT, 0, map_wps_timeout, wapp, device_status);
		wapp_send_1905_msg(
			wapp,
			WAPP_DEVICE_STATUS,
			sizeof(wapp_device_status),
			(char *)device_status);
	}
	else if (device_status->status_fhbss == STATUS_FHBSS_WPS_TRIGGERED) {
		*status = RADIO_BUSY;
	}
#endif
	return (*status == SCAN_SUCCESS)? TRUE: FALSE;
}
void wdev_timer_off_ch_scan_complete(void *eloop_data, void *user_ctx);

void map_handle_off_ch_scan_disallowed(struct wifi_app *wapp, struct wapp_dev *wdev,u8 ch, u8 op_class, u8 status)
{
	struct off_ch_scan_report *scan_rep = wapp->map->off_ch_scan_rep;
	struct off_ch_scan_result_tlv *scan_result = NULL;
	struct global_oper_class *op = NULL;

	if(status == OP_CLASS_CHAN_NOT_SUPP) {// scan on this channel is not supported

		scan_result = os_zalloc(sizeof(struct off_ch_scan_result_tlv));
		if(scan_result == NULL) {
			DBGPRINT(RT_DEBUG_OFF,"%s : cannot alloc mem\n", __func__);
			// TODO: error handling
			return;
		}
		dl_list_add(&scan_rep->scan_result_list, &scan_result->list);
		// fill the common fields
		MAP_GET_RADIO_IDNFER(wdev->radio,scan_result->radio_id);
		scan_result->oper_class = op_class;
		scan_result->channel = ch;
		scan_result->scan_status = status;
		wapp->map->off_ch_scan_state.curr_ch_idx++;
		wapp->map->off_ch_scan_rep->scan_result_num++;
	} else if ((status == REQ_TOO_SOON) || (status == ABORT_SCAN) || (status == RADIO_BUSY)) {

		// fill scan report for all channels and all operating classes for this radio
		struct off_ch_scan_body *radio = &wapp->map->off_ch_scan_req->body[wapp->map->off_ch_scan_state.curr_scan_radio_idx];
		u8 i,k,ch_num =0,*ch_list = NULL;

		for(i = 0; i < radio->oper_class_num; i++) {
			op_class = radio->ch_body[i].oper_class;
			ch_num = radio->ch_body[i].ch_list_num;
			ch_list = radio->ch_body[i].ch_list;

			if(radio->ch_body[i].ch_list_num ==0) {
				op = wapp_get_global_op_class(op_class);
				if(!op)
					break;
				ch_list = op->channel_set;
				ch_num = op->channel_num;
			}

			for (k = 0; k < ch_num; k++) {
				scan_result = os_zalloc(sizeof(struct off_ch_scan_result_tlv));
				if(scan_result == NULL) {
					DBGPRINT(RT_DEBUG_OFF,"%s : cannot alloc mem\n", __func__);
					// TODO: error handling
					return;
				}
				dl_list_add(&scan_rep->scan_result_list, &scan_result->list);
				// fill the common fields
				MAP_GET_RADIO_IDNFER(wdev->radio,scan_result->radio_id);
				scan_result->oper_class = op_class;
				scan_result->channel = ch_list[k];
				scan_result->scan_status = status;
				wapp->map->off_ch_scan_rep->scan_result_num++;
			}
		}

		wapp->map->off_ch_scan_state.curr_oper_class_idx = radio->oper_class_num - 1;
		wapp->map->off_ch_scan_state.curr_ch_idx = ch_num;
	}
	eloop_register_timeout(0,0,wdev_timer_off_ch_scan_complete, wapp, wdev);
}


/* Send BTM pkt content to driver */
int wapp_send_off_ch_scan_req(struct wifi_app *wapp,
						 const char *iface,
						 const char *scan_msg,
						 size_t msg_len)
{
	int ret;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	ret = wapp->drv_ops->drv_off_ch_scan_req(wapp->drv_data, iface, scan_msg, msg_len);

	return ret;
}

void write_timestamp(char *timestamp, u8*ts_len)
{
	time_t t;
	struct tm *tm;

	time(&t);
	tm = localtime(&t);
	/*
	DBGPRINT(RT_DEBUG_OFF, "%04u-%02u-%02u %02u:%02u:%02u %s: %d\n",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec, tm->tm_zone, (int)tm->tm_gmtoff);
		*/
	sprintf(timestamp, "%04u-%02u-%02uT%02u:%02u:%02u+00:00",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		tm->tm_hour, tm->tm_min, tm->tm_sec);
	*ts_len = strlen(timestamp);
	//DBGPRINT(RT_DEBUG_OFF,"Timestamp: %s : %d\n", timestamp, (unsigned int)*ts_len);
}

void fill_scan_BH_ssids(struct wifi_app *wapp,struct scan_BH_ssids *scan_ssids)
{
	u8 j = 0;
	for (j = 0; j < MAX_NUM_OF_RADIO && j < MAX_PROFILE_CNT; j++) {
		if (wapp->map->apcli_configs[j].config_valid) {
			scan_ssids->profile_cnt ++;
			os_memcpy(scan_ssids->scan_SSID_val[j].ssid,
				&wapp->map->apcli_configs[j].apcli_config.ssid,
				wapp->map->apcli_configs[j].apcli_config.SsidLen);
			scan_ssids->scan_SSID_val[j].SsidLen = wapp->map->apcli_configs[j].apcli_config.SsidLen;
		}
	}
}

void map_issue_off_ch_scan_req_v2(struct wifi_app *wapp)
{
	OFFCHANNEL_SCAN_MSG *scan_msg = NULL;
	struct off_ch_scan_req_s*scan_req = wapp->map->off_ch_scan_req;
	struct wapp_dev *wdev = NULL;
	struct off_ch_scan_state_ctrl *scan_state = &wapp->map->off_ch_scan_state;
	u8 op_class = 0, ch_num = 0;
	u8 ch_idx =0, a = 0;
	u8 status = 0;
	u8 *ch_list = scan_req->body[scan_state->curr_scan_radio_idx].ch_body[scan_state->curr_oper_class_idx].ch_list;
	struct global_oper_class *op = NULL;
	struct scan_BH_ssids *scan_ssids = os_zalloc(sizeof(struct scan_BH_ssids));
	if(scan_ssids == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,"Alloc fail %s %d", __func__, __LINE__);
		return;
	}
	wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)scan_req->body[scan_state->curr_scan_radio_idx].radio_id);

	if(!wdev) {
		DBGPRINT(RT_DEBUG_OFF,"%s %d\n", __func__, __LINE__);
		hex_dump("RadioID", scan_req->body[scan_state->curr_scan_radio_idx].radio_id,6);
		goto Error;
	}
#ifdef MAP_R2
	if(scan_req->neighbour_only == 1)
#endif
	{
	fill_scan_BH_ssids(wapp,scan_ssids);
	wapp_set_scan_BH_ssids(wapp, wdev, scan_ssids);
	}
	
	scan_msg = &scan_state->scan_msg;
	os_memset(scan_msg,0, sizeof(OFFCHANNEL_SCAN_MSG));

	scan_msg->Action = GET_OFFCHANNEL_INFO;
	os_memcpy(scan_msg->ifrn_name, wdev->ifname,sizeof(wdev->ifname));

	op_class = scan_req->body[scan_state->curr_scan_radio_idx].ch_body[scan_state->curr_oper_class_idx].oper_class;
	ch_num = scan_req->body[scan_state->curr_scan_radio_idx].ch_body[scan_state->curr_oper_class_idx].ch_list_num;
	ch_idx = scan_state->curr_ch_idx;

	if(ch_num == 0) { // all channels in the operating class
		op = wapp_get_global_op_class(op_class);

		if(op == NULL) {
			DBGPRINT(RT_DEBUG_OFF,"Requested opclass not found !!\n");
			map_handle_off_ch_scan_disallowed(wapp, wdev,0,op_class, OP_CLASS_CHAN_NOT_SUPP);
			goto Error;
		}
		ch_list = op->channel_set;
		ch_num = op->channel_num;
	}
	if (scan_state->ch_per_op_left == 0)
		scan_state->ch_per_op_left = ch_num;
/*Fill at max 5 channels to scan in one command*/
	if ((ch_num - ch_idx) >= MAX_AWAY_CHANNEL)
		scan_msg->data.offchannel_param.Num_of_Away_Channel = MAX_AWAY_CHANNEL;
	else
		scan_msg->data.offchannel_param.Num_of_Away_Channel = ch_num - ch_idx;

	scan_state->curr_ch_idx = ch_idx + 1;
	for (a = 0; a < scan_msg->data.offchannel_param.Num_of_Away_Channel; a++) {
		if(map_is_off_ch_scan_allowed(wapp, wdev, ch_list[ch_idx], op_class, &status) == FALSE) {
			DBGPRINT(RT_DEBUG_ERROR, "channel scan not allowed : %d %d %d\n", ch_list[ch_idx], op_class, status);
			map_handle_off_ch_scan_disallowed(wapp, wdev,ch_list[ch_idx],op_class, status);
			// avoid recursion here.
			goto Error;
		}
		scan_msg->data.offchannel_param.channel[a] = ch_list[ch_idx];
		scan_msg->data.offchannel_param.scan_type[a] = IsRadarChannel(ch_list[ch_idx])?0x80:0;
		scan_msg->data.offchannel_param.scan_time[a] = 120;
		ch_idx++;
	}
	scan_state->curr_chan_num = ch_list[ch_idx];
	scan_msg->data.offchannel_param.bw = scan_req->bw;
	// update index.
	write_timestamp(scan_state->last_scan_tm.timestamp, &(scan_state->last_scan_tm.timestamp_len));
	// issue command
	wapp_send_off_ch_scan_req(wapp, wdev->ifname,(const char *)scan_msg, sizeof(OFFCHANNEL_SCAN_MSG));	
Error:
	os_free(scan_ssids);
	return;
}


void map_issue_off_ch_scan_req(struct wifi_app *wapp)
{
	OFFCHANNEL_SCAN_MSG *scan_msg = NULL;
	struct off_ch_scan_req_s*scan_req = wapp->map->off_ch_scan_req;
	struct wapp_dev *wdev = NULL;
	struct off_ch_scan_state_ctrl *scan_state = &wapp->map->off_ch_scan_state;
	u8 op_class = 0, ch_num = 0;
	u8 ch_idx =0;
	u8 status = 0;
	u8 *ch_list = scan_req->body[scan_state->curr_scan_radio_idx].ch_body[scan_state->curr_oper_class_idx].ch_list;
	struct global_oper_class *op = NULL;
	struct scan_BH_ssids *scan_ssids = os_zalloc(sizeof(struct scan_BH_ssids));

	if(scan_ssids == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,"Alloc fail %s %d", __func__, __LINE__);
		return;
	}
	wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)scan_req->body[scan_state->curr_scan_radio_idx].radio_id);

	if(!wdev) {
		DBGPRINT(RT_DEBUG_OFF,"%s %d\n", __func__, __LINE__);
		hex_dump("RadioID", scan_req->body[scan_state->curr_scan_radio_idx].radio_id,6);
		goto Error;
	}
#ifdef MAP_R2
	if(scan_req->neighbour_only == 1)
#endif
	{
	fill_scan_BH_ssids(wapp,scan_ssids);
	wapp_set_scan_BH_ssids(wapp, wdev, scan_ssids);
	}

	scan_msg = &scan_state->scan_msg;
	os_memset(scan_msg,0, sizeof(OFFCHANNEL_SCAN_MSG));

	scan_msg->Action = GET_OFFCHANNEL_INFO;
	os_memcpy(scan_msg->ifrn_name, wdev->ifname,sizeof(wdev->ifname));

	op_class = scan_req->body[scan_state->curr_scan_radio_idx].ch_body[scan_state->curr_oper_class_idx].oper_class;
	ch_num = scan_req->body[scan_state->curr_scan_radio_idx].ch_body[scan_state->curr_oper_class_idx].ch_list_num;
	ch_idx = scan_state->curr_ch_idx;

	if(ch_num == 0) { // all channels in the operating class
		// fill all the primary channels of 1 channel for that opeating class, update the channel index, and issue scan.
		op = wapp_get_global_op_class(op_class);

		if(op == NULL) {
			DBGPRINT(RT_DEBUG_OFF,"Requested opclass not found !!\n");
			map_handle_off_ch_scan_disallowed(wapp, wdev,0,op_class, OP_CLASS_CHAN_NOT_SUPP);
			goto Error;
		}
		ch_list = op->channel_set;
	}

	if(map_is_off_ch_scan_allowed(wapp, wdev, ch_list[ch_idx], op_class, &status) == FALSE) {
		DBGPRINT(RT_DEBUG_ERROR, "channel scan not allowed : %d %d %d\n", ch_list[ch_idx], op_class, status);
		map_handle_off_ch_scan_disallowed(wapp, wdev,ch_list[ch_idx],op_class, status);
		// avoid recursion here.
		goto Error;
	}
	// should only fill one channel right now since only 20MHz OpClass is supported.
	scan_msg->data.offchannel_param.Num_of_Away_Channel = wapp_fill_ch_list(wapp,
														&scan_msg->data.offchannel_param,
														op_class,ch_list[ch_idx]);
	if(scan_msg->data.offchannel_param.Num_of_Away_Channel > 1)
		DBGPRINT(RT_DEBUG_ERROR, "> 1 channels given; ERROR!\n");

	scan_state->curr_chan_num = ch_list[ch_idx];
	scan_msg->data.offchannel_param.bw = scan_req->bw;
	// update index.
	ch_idx += 1;
	scan_state->curr_ch_idx = ch_idx;
	write_timestamp(scan_state->last_scan_tm.timestamp, &(scan_state->last_scan_tm.timestamp_len));
	// issue command
	wapp_send_off_ch_scan_req(wapp, wdev->ifname,(const char *)scan_msg, sizeof(OFFCHANNEL_SCAN_MSG));
Error:
	os_free(scan_ssids);
	return;
}
int check_radio_match(struct wifi_app *wapp, u8 *radio_id)
{
	struct off_ch_scan_req_s *scan_req = wapp->map->off_ch_scan_req;
	u8 i=0;
	if (scan_req == NULL)
		return TRUE;
	for (i=0;i< scan_req->radio_num; i++) {
		if(os_memcmp(scan_req->body[i].radio_id, radio_id, MAC_ADDR_LEN) == 0)
			return TRUE;
	}
	return FALSE;
}

void off_ch_scan_timeout(void *eloop_data, void *user_ctx)
{
	struct wifi_app *wapp = eloop_data;
	// Scan did not complete in x seconds. take action
	DBGPRINT(RT_DEBUG_OFF,"%s\n", __func__);
	map_build_and_send_off_ch_scan_rep(wapp);
	// TODO: raghav: clear scan data: free it.
	wapp->map->off_ch_scan_state.ch_scan_state = CH_SCAN_IDLE;
#ifdef MAP_R2
	wapp->map->f_scan_req = 0;
#endif
}
int map_get_net_opt_scan_rep_len(struct wifi_app *wapp)
{
	struct off_ch_scan_result_tlv	*scan_result = NULL;
	struct off_ch_scan_report *scan_rep = wapp->map->off_ch_scan_rep;
	u16 scan_rep_len;
	scan_rep_len = sizeof(struct net_opt_scan_report_event);

	dl_list_for_each(scan_result, &scan_rep->scan_result_list,
	struct off_ch_scan_result_tlv, list) {
		if((check_radio_match(wapp,scan_result->radio_id) == TRUE)
			&& (scan_result->neighbor_num
#ifdef MAP_R2
			|| (wapp->map->f_scan_req == 1)
#endif
			)
		)
			scan_rep_len += sizeof(struct net_opt_scan_result_event) +
			scan_result->neighbor_num*sizeof(struct neighbor_info);
	}
	return scan_rep_len;
}
int map_get_off_ch_scan_rep_len(struct wifi_app *wapp)
{
	struct off_ch_scan_report *scan_rep = wapp->map->off_ch_scan_rep;
	u16 scan_rep_len;

	scan_rep_len = sizeof(struct off_ch_scan_report_event) 
					+ (scan_rep->scan_result_num) * sizeof(struct off_ch_scan_result_event);
	return scan_rep_len;
}
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
void dump_off_ch_scan_rep(struct net_opt_scan_report_event *scan_rep_evt)
{
	int i,j;
	u8 * buf = NULL;
	buf = (u8 *)scan_rep_evt->scan_result;
	for(i=0; i< scan_rep_evt->scan_result_num;i++) {
		struct net_opt_scan_result_event *scan_result = (struct net_opt_scan_result_event *)buf;
		printf("RESULT : %d--------------------->START\n", i);
		hex_dump_dbg("RadioID:",scan_result->radio_id,MAC_ADDR_LEN);
		printf("Channel=%d\n \n",scan_result->channel);
		printf("Utilization = %d,\n noise=%d\n", scan_result->utilization,
						scan_result->noise);
#ifdef MAP_R2
		printf("edcca = %d,\n obss time=%d\n", scan_result->edcca,
						scan_result->obss_time);
#endif
		printf("Neighbor List %d-------------->\n", scan_result->neighbor_num);
		for (j=0;j<scan_result->neighbor_num;j++) {
			struct neighbor_info *nb_info = &scan_result->nb_info[j];
			printf("BSSID:"MACSTR"\n", MAC2STR(nb_info->bssid));
			printf("SSID : %s: %d\n RCPI = %d\n ", nb_info->ssid, nb_info->ssid_len, nb_info->RCPI);
			printf("BW: %d %s\n", nb_info->ch_bw_len, nb_info->ch_bw);
			printf("CUPresent=%d, CU=%d, STACNT=%d\n", nb_info->cu_stacnt_present,nb_info->cu, nb_info->sta_cnt);
		}
		printf("RESULT : %d--------------------->END\n\n", i);
		buf += sizeof(struct net_opt_scan_result_event)+ scan_result->neighbor_num*sizeof(struct neighbor_info);
	}
}
#ifdef MAP_R2
void check_pending_scan_req (void *eloop_data, void *user_ctx)
{
	struct wifi_app *wapp = (struct wifi_app*) user_ctx;
	if(wapp->map->msg_info.enqueue_pending_msg == WAPP_USER_SET_CHANNEL_SCAN_REQ)
	{
	//	printf("SSS check pending status\n");
		wapp->map->f_scan_req = 1;
		wapp->map->msg_info.enqueue_pending_msg = 0;//clean pending status
		wapp->map->msg_info.ignore_req_too_soon = 1;
		map_receive_off_channel_scan_req(wapp, wapp->map->msg_info.msg_body_ptr, wapp->map->msg_info.msg_len);
		if (wapp->map->off_ch_scan_state.ch_scan_state != CH_SCAN_ONGOING)
			wapp->map->f_scan_req = 0;
		if (wapp->map->msg_info.msg_body_ptr) {
			os_free(wapp->map->msg_info.msg_body_ptr);
			wapp->map->msg_info.msg_len = 0;
		}
	}
}
#endif
void map_build_and_send_net_opt_scan_rep(struct wifi_app *wapp)
{
	struct evt *map_event = NULL;
	struct net_opt_scan_report_event *scan_rep_evt = NULL;
	struct off_ch_scan_result_tlv *scan_result = NULL;
	struct off_ch_scan_report *scan_rep = wapp->map->off_ch_scan_rep;
	u16 evt_len;
	u16 scan_rep_len;
	u16 j;
	u8 *buf = NULL;

	if(scan_rep == NULL) {
		return;
	}

	if(scan_rep->scan_result_num == 0) {
		return;
	}

#ifdef MAP_R2
	if(wapp->map->off_ch_scan_state.ch_scan_state == CH_ONBOOT_SCAN_ONGOING)
		return;
#endif

	scan_rep_len = map_get_net_opt_scan_rep_len(wapp);
	evt_len = sizeof(struct evt) + scan_rep_len;
	map_event = os_zalloc(evt_len);
	if(map_event == NULL)
		goto Error;
	map_event->length = scan_rep_len;

#ifdef MAP_R2
	if(wapp->map->f_scan_req == 1)
		map_event->type = WAPP_CHANNEL_SCAN_REPORT;
	else
#endif
	map_event->type = WAPP_NET_OPT_SCAN_REPORT;
	scan_rep_evt = (struct net_opt_scan_report_event *)map_event->buffer;

#ifdef MAP_R2
	write_timestamp(scan_rep_evt->timestamp.timestamp, &scan_rep_evt->timestamp.timestamp_len);
#endif

	scan_rep_evt->scan_result_num = 0;//scan_rep->scan_result_num;
	buf = (u8 *)scan_rep_evt->scan_result;
	dl_list_for_each(scan_result, &scan_rep->scan_result_list,
	struct off_ch_scan_result_tlv, list) {
		struct net_opt_scan_result_event *scan_result_evt =
			(struct net_opt_scan_result_event *)buf;
		if(check_radio_match(wapp, scan_result->radio_id) == FALSE)
			continue;
		if (scan_result->neighbor_num != 0 
#ifdef MAP_R2
		|| wapp->map->f_scan_req == 1
#endif
		) {//only send valid scan results to map
			scan_rep_evt->scan_result_num++;
			os_memcpy(scan_result_evt->radio_id, scan_result->radio_id, MAC_ADDR_LEN);
#ifdef MAP_R2
			if(wapp->map->f_scan_req == 1) {
				scan_result_evt->oper_class = scan_result->oper_class;
				scan_result_evt->scan_status= scan_result->scan_status;
				scan_result_evt->timestamp_len= scan_result->timestamp_len;
				//os_memcpy(scan_result_evt->timestamp, scan_result->timestamp, scan_result->timestamp_len);
				scan_result_evt->agg_scan_duration= scan_result->agg_scan_duration;
				scan_result_evt->scan_type= scan_result->scan_type;
				scan_result_evt->rx_time = scan_result->rx_time;
				scan_result_evt->tx_time = scan_result->tx_time;
				scan_result_evt->obss_time = scan_result->obss_time;
				scan_result_evt->edcca = scan_result->edcca;
			}
#endif
			scan_result_evt->channel= scan_result->channel;
			scan_result_evt->utilization= scan_result->utilization;
			scan_result_evt->noise= scan_result->noise;
			scan_result_evt->neighbor_num= scan_result->neighbor_num;
			for (j=0;j<scan_result->neighbor_num;j++) {
				struct neighbor_info *nb_info_target = &scan_result_evt->nb_info[j];
				struct neighbor_info *nb_info_source = &scan_result->nb_info[j];
				os_memcpy(nb_info_target, nb_info_source, sizeof(struct neighbor_info));
			}
			buf += sizeof(struct net_opt_scan_result_event)+ (scan_result->neighbor_num * sizeof(struct neighbor_info));
		}
	}
		//dump_off_ch_scan_rep(scan_rep_evt);
#if 1
	if (0 > map_1905_send(wapp, (char *)map_event, evt_len)) {
		DBGPRINT(RT_DEBUG_ERROR, "%s send ch scan msg fail\n", __func__);
	}
#endif
#ifdef MAP_R2
	//printf("SSS report sent for last scan done \n");
	if (wapp->map->msg_info.ignore_req_too_soon)
		wapp->map->msg_info.ignore_req_too_soon = 0;
	//add check if another msg is enqueued for scan then it also needs to be handled
	eloop_register_timeout(1,0,check_pending_scan_req, NULL, wapp);
#endif
	Error:
		if(map_event)
			os_free(map_event);
		return;
}

void map_build_and_send_off_ch_scan_rep(struct wifi_app *wapp)
{
	struct evt *map_event = NULL;
	struct off_ch_scan_report_event *scan_rep_evt = NULL;
	struct off_ch_scan_result_tlv	*scan_result = NULL;
	struct off_ch_scan_report *scan_rep = wapp->map->off_ch_scan_rep;
	u16 evt_len;
	u16 scan_rep_len;
	//int i=0;
	u8 *buf = NULL;
	printf("%s", __func__);
#ifdef MAP_R2
	if(wapp->map->off_ch_scan_state.ch_scan_state == CH_ONBOOT_SCAN_ONGOING)
		return;
#endif
	scan_rep_len = map_get_off_ch_scan_rep_len(wapp);
	
	evt_len = sizeof(struct evt) + scan_rep_len;
	map_event = os_zalloc(evt_len);
	if(map_event == NULL)
		goto Error;
	map_event->length = scan_rep_len;
	map_event->type = WAPP_OFF_CH_SCAN_REPORT;

	scan_rep_evt = (struct off_ch_scan_report_event *)map_event->buffer;

// TODO: if the list is empty, then send the failure.

#if 1
	scan_rep_evt->scan_result_num = scan_rep->scan_result_num;
	buf = (u8 *)scan_rep_evt->scan_result;
	dl_list_for_each(scan_result, &scan_rep->scan_result_list, struct off_ch_scan_result_tlv, list)  {
		struct off_ch_scan_result_event *scan_result_evt = (struct off_ch_scan_result_event *)buf;
		os_memcpy(scan_result_evt->radio_id, scan_result->radio_id, MAC_ADDR_LEN);
		scan_result_evt->channel= scan_result->channel;
		scan_result_evt->utilization= scan_result->utilization;
		scan_result_evt->noise= scan_result->noise;
		scan_result_evt->neighbor_num= scan_result->neighbor_num;
		buf += sizeof(struct off_ch_scan_result_event);
	}
#endif
#if 1
	if (0 > map_1905_send(wapp, (char *)map_event, evt_len)) {
		DBGPRINT(RT_DEBUG_ERROR, "%s send ch scan msg fail\n", __func__);
	}
#endif
	Error:

		if(map_event)
			os_free(map_event);
		return;
}

// handle channel scan compelete event from driver.
void wdev_handle_off_ch_scan_complete(struct wifi_app *wapp, u32 ifindex)
{
	struct off_ch_scan_state_ctrl *scan_state = &wapp->map->off_ch_scan_state;
	struct off_ch_scan_req_s * ch_scan_req = wapp->map->off_ch_scan_req;
	struct wapp_dev *wdev = NULL;
	u8 ch_num = 0;
	u8 op_class = ch_scan_req->body[wapp->map->off_ch_scan_state.curr_scan_radio_idx].ch_body[wapp->map->off_ch_scan_state.curr_oper_class_idx].oper_class;
	struct global_oper_class *op = NULL;

	if(wapp->map->off_ch_scan_state.ch_scan_state == CH_SCAN_IDLE) {
		DBGPRINT(RT_DEBUG_OFF,"%s Scan Results received in invalid state\n", __func__);
		goto Error;
	}

	wdev = wapp_dev_list_lookup_by_ifindex(wapp,ifindex);
	if(!wdev) {
		DBGPRINT(RT_DEBUG_OFF,"wdev not found for this ifindex : %d\n", ifindex);
		goto Error;
	}

	if(ch_scan_req->body[scan_state->curr_scan_radio_idx].ch_body[scan_state->curr_oper_class_idx].ch_list_num == 0) {
		op = wapp_get_global_op_class(op_class);
		if(op == NULL)
			goto Error;
		ch_num = op->channel_num;
	} else {
		ch_num = ch_scan_req->body[wapp->map->off_ch_scan_state.curr_scan_radio_idx].ch_body[wapp->map->off_ch_scan_state.curr_oper_class_idx].ch_list_num;
	}
	//DBGPRINT(RT_DEBUG_OFF,"%s %d : %d %d\n", __func__, __LINE__, scan_state->curr_ch_idx, ch_num);

	if(scan_state->curr_ch_idx < ch_num) {// channels remaining on the current operating class
		printf("scan_state->curr_ch_idx %d, ch_num %d\n", scan_state->curr_ch_idx, ch_num);
		map_issue_off_ch_scan_req_v2(wapp);
		//map_issue_off_ch_scan_req(wapp);
// if all channels have been scanned for this operating class.
	} else if((scan_state->curr_oper_class_idx+1) < ch_scan_req->body[scan_state->curr_scan_radio_idx].oper_class_num) {
		// move to next operating class on the same radio.
		scan_state->curr_oper_class_idx++;
		scan_state->curr_ch_idx = 0;
		// issue next scan req.
		map_issue_off_ch_scan_req_v2(wapp);
		//map_issue_off_ch_scan_req(wapp);
	} else {
		// move to next radio.
		scan_state->curr_oper_class_idx = 0;
		scan_state->curr_ch_idx = 0;
		os_get_time(&wdev->radio->last_scan_time);

		if((scan_state->curr_scan_radio_idx+1) == ch_scan_req->radio_num) {
			// Scan complete. Send scan report.
			DBGPRINT(RT_DEBUG_OFF,"Scan Completed for all radios %d\n",wapp->map->off_ch_scan_req->neighbour_only);
			if (wapp->map->off_ch_scan_req->neighbour_only == FALSE)
			map_build_and_send_off_ch_scan_rep(wapp);
			else
				map_build_and_send_net_opt_scan_rep(wapp);
			eloop_cancel_timeout(off_ch_scan_timeout,wapp,NULL);
			scan_state->ch_scan_state = CH_SCAN_IDLE;
#ifdef MAP_R2
			wapp->map->f_scan_req = 0;
#endif
		} else {
			//map_fill_last_scan_time(wapp, scan_state->curr_scan_radio_idx);
			scan_state->curr_scan_radio_idx++;
			map_issue_off_ch_scan_req_v2(wapp);
//			map_issue_off_ch_scan_req(wapp);
		}
	}
	return;
	Error:
		// TODO: Error handling;
		return;
}

void wdev_timer_off_ch_scan_complete(void *eloop_data, void *user_ctx)
{
	struct wifi_app *wapp = eloop_data;
	struct wapp_dev *wdev = user_ctx;

	wdev_handle_off_ch_scan_complete(wapp, wdev->ifindex);
}

void wapp_clear_off_ch_scan_report(struct wifi_app *wapp)
{
	// free the report as well
	struct off_ch_scan_report *scan_rep = wapp->map->off_ch_scan_rep;
	struct off_ch_scan_result_tlv *scan_result = NULL, *scan_result_tmp = NULL;
	
	dl_list_for_each_safe(scan_result, scan_result_tmp, &scan_rep->scan_result_list, struct off_ch_scan_result_tlv, list)  {
		dl_list_del(&scan_result->list);
		if (scan_result->nb_info)
			os_free(scan_result->nb_info);
		os_free(scan_result);
	}
	os_memset(scan_rep, 0, sizeof(struct off_ch_scan_report));
}

int map_start_off_ch_scan(struct wifi_app *wapp)
{

	wapp->map->off_ch_scan_state.curr_scan_radio_idx = 0;
	wapp->map->off_ch_scan_state.curr_oper_class_idx =0;
	wapp->map->off_ch_scan_state.curr_ch_idx = 0;
	wapp->map->off_ch_scan_state.ch_per_op_left = 0;

	if(wapp->map->off_ch_scan_rep)
		wapp_clear_off_ch_scan_report(wapp);
	else {
		wapp->map->off_ch_scan_rep = os_zalloc(sizeof(struct off_ch_scan_report));
		if(wapp->map->off_ch_scan_rep == NULL)
			return MAP_ERROR;
	}
	dl_list_init(&wapp->map->off_ch_scan_rep->scan_result_list);

	//start scan.
	
	//DBGPRINT(RT_DEBUG_OFF,"%s %d\n", __func__, __LINE__);
	map_issue_off_ch_scan_req_v2(wapp);
	//map_issue_off_ch_scan_req(wapp);
	return MAP_SUCCESS;
}
int map_receive_off_channel_scan_req(
	struct wifi_app *wapp, char *msg_buf,unsigned short msg_len);
int get_valid_radio_count (struct wifi_app *wapp)
{
	int i = 0;
	int radio_count = 0;
	for (i = 0; i < MAX_NUM_OF_RADIO; i++)
	{
		if (wapp->radio[i].op_ch)
			radio_count++;
	}
	return radio_count;
}

unsigned char get_opclass_from_channel(unsigned char channel)
{
	if (channel <= 14)
	{
		return 81;
	} else if (channel <= 48) {
		return 115;
	} else if (channel <= 64) {
		return 118;
	} else if (channel <= 144) {
		return 121;
	} else if (channel <= 169) {
		return 125;
	}
	return 0;
}

void get_radio_id_by_channel(struct wifi_app *wapp, unsigned char channel,
	unsigned char *wdev_identifier)
{
	int radio_count = 0;
	struct dl_list *dev_list;
	radio_count = get_valid_radio_count(wapp);
	dev_list = &wapp->dev_list;
	struct wapp_dev *wdev = NULL;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev && wdev->radio) {
			if((wdev->dev_type == WAPP_DEV_TYPE_AP)) {
				if (radio_count == 3) {
					if (((wdev->radio->op_ch >= 100) && (channel >= 100)) ||
						((wdev->radio->op_ch >= 36) && (channel >= 36) &&
						(wdev->radio->op_ch < 100) && (channel < 100)) ||
						((wdev->radio->op_ch < 36) && (channel < 36))) {
						MAP_GET_RADIO_IDNFER(wdev->radio, wdev_identifier);
						break;
					}
				} else {
					if (((wdev->radio->op_ch >= 14) && (channel >= 14)) ||
						((wdev->radio->op_ch < 36) && (channel < 36))) {
						MAP_GET_RADIO_IDNFER(wdev->radio, wdev_identifier);
						break;
					}
				}
			}
		}
	}
}
void map_build_scan_body(struct wifi_app *wapp,
	struct off_ch_scan_req_s *off_ch_can_req, unsigned char *ch_list)
{
	unsigned int i = 0;
	unsigned int j =0;
	unsigned int k =0;
	unsigned char oper_class = 0;
	unsigned char raid[6] = {0};
	struct off_ch_scan_body *scan_body = off_ch_can_req->body;
	while ((i < MAX_OFF_CH_SCAN_CH) && ch_list[i] != 0)
	{
		oper_class = get_opclass_from_channel(ch_list[i]);
		get_radio_id_by_channel(wapp, ch_list[i], raid);
		j = 0;
		while (j < off_ch_can_req->radio_num)
		{
			if (!os_memcmp(scan_body[j].radio_id, raid, 6))
			{
				k = 0;
				while (k < scan_body[j].oper_class_num) {
					if (scan_body[j].ch_body[k].oper_class == oper_class) {
						scan_body[j].ch_body[k].ch_list[scan_body[j].ch_body[k].ch_list_num]
							= ch_list[i];
						scan_body[j].ch_body[k].ch_list_num++;
						break;
					}
					k++;
				}
				if (k == scan_body[j].oper_class_num) {
					scan_body[j].ch_body[k].oper_class = oper_class;
					scan_body[j].ch_body[k].ch_list[0] = ch_list[i];
					scan_body[j].ch_body[k].ch_list_num = 1;
					scan_body[j].oper_class_num++;
				}
				break;
			}
			j++;
		}
		if (j == off_ch_can_req->radio_num) {
			os_memcpy(scan_body[j].radio_id, raid, 6);
			scan_body[j].ch_body[0].oper_class = oper_class;
			scan_body[j].ch_body[0].ch_list[0] = ch_list[i];
			scan_body[j].ch_body[0].ch_list_num = 1;
			scan_body[j].oper_class_num++;
			off_ch_can_req->radio_num++;
		}
		i++;
	}
}
void map_prepare_off_channel_scan_req(struct wifi_app *wapp,
	char *msg, unsigned short type)
{
	struct wapp_dev *wdev = NULL;
	unsigned char wdev_identifier[6];
	struct dl_list *dev_list;
	unsigned char band = 0;
	struct off_ch_scan_req_msg_s *scan_msg = (struct off_ch_scan_req_msg_s *)msg;
	unsigned short scan_req_size = sizeof(struct off_ch_scan_req_s) +
		(sizeof(struct off_ch_scan_body) * 3);
	struct off_ch_scan_req_s *off_ch_can_req =
		(struct off_ch_scan_req_s *) os_zalloc(scan_req_size);
	struct off_ch_scan_body *scan_body = NULL;
	int map_band = 0;
	int radio_count = 0;
	unsigned int radio_2g_done = 0;
	unsigned int radio_5g_done = 0;
	unsigned int radio_5gh_done = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dev_list = &wapp->dev_list;
	off_ch_can_req->fresh_scan = 0x80;
	scan_body = off_ch_can_req->body;

	band = scan_msg->band;
	radio_count = get_valid_radio_count(wapp);

	if (band & 0x04)
		map_band = WPS_BAND_5GH;
	else if (band & 0x02) {
		if (radio_count == 2)
			map_band = WPS_BAND_5G;
		else
			map_band = WPS_BAND_5GL;
	} else if ((band & 0x01) || (band == 0))
		map_band = WPS_BAND_24G;

	wdev = wapp_dev_list_lookup_by_band_and_type(wapp, map_band, WAPP_DEV_TYPE_STA);
	if (wdev && wdev->wapp_triggered_scan == TRUE) {
		os_free(off_ch_can_req);
		return;
	}

	if (scan_msg->mode == SCAN_MODE_BAND) {
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
			if (wdev && wdev->radio) {
				MAP_GET_RADIO_IDNFER(wdev->radio, wdev_identifier);
				if((wdev->dev_type == WAPP_DEV_TYPE_AP)) {
					if (radio_count == 3) {
						if ((wdev->radio->op_ch >= 100) && (band & 0x04)
							&& !radio_5gh_done) {
							os_memcpy(scan_body->radio_id, wdev_identifier, 6);
							scan_body->oper_class_num = 2;
							scan_body->ch_body[0].oper_class = 121;//115;
							scan_body->ch_body[1].oper_class = 125;//118;
							scan_body++;
							radio_5gh_done = 1;
							off_ch_can_req->radio_num++;
						} else if ((wdev->radio->op_ch > 14) && (wdev->radio->op_ch < 100) && (band & 0x02)
						&& !radio_5g_done) {
							os_memcpy(scan_body->radio_id, wdev_identifier, 6);
							scan_body->oper_class_num = 2;
	 						scan_body->ch_body[0].oper_class = 115;//121;
							scan_body->ch_body[1].oper_class = 118;//125;
							scan_body++;
							radio_5g_done = 1;
							off_ch_can_req->radio_num++;
						} else if ((wdev->radio->op_ch < 14) && (band & 0x01)
							&& !radio_2g_done) {
							os_memcpy(scan_body->radio_id, wdev_identifier, 6);
							scan_body->oper_class_num = 1;
							scan_body->ch_body[0].oper_class = 81;
							scan_body++;
							radio_2g_done = 1;
							off_ch_can_req->radio_num++;
						}
					} else if (radio_count == 2){
						if ((wdev->radio->op_ch > 14) && (band & 0x06) &&
							!radio_5g_done) {
							os_memcpy(scan_body->radio_id, wdev_identifier, 6);
							scan_body->oper_class_num = 4;
							scan_body->ch_body[0].oper_class = 115;
							scan_body->ch_body[1].oper_class = 118;
							scan_body->ch_body[2].oper_class = 121;
							scan_body->ch_body[3].oper_class = 125;
							scan_body++;
							radio_5g_done = 1;
							off_ch_can_req->radio_num++;
						} else if ((wdev->radio->op_ch < 14) && (band & 0x01) &&
						!radio_2g_done){
							os_memcpy(scan_body->radio_id, wdev_identifier, 6);
							scan_body->oper_class_num = 1;
							scan_body->ch_body[0].oper_class = 81;
							scan_body++;
							radio_2g_done = 1;
							off_ch_can_req->radio_num++;
						}
					}
				}
			}
		}
	} else if (scan_msg->mode == SCAN_MODE_CH){
		map_build_scan_body(wapp, off_ch_can_req, scan_msg->ch_list);
	}
	off_ch_can_req->bw = scan_msg->bw;
	if (type == WAPP_USER_SET_NET_OPT_SCAN_REQ)
		off_ch_can_req->neighbour_only = TRUE;
	if (off_ch_can_req->radio_num)
		map_receive_off_channel_scan_req(wapp, (char *)off_ch_can_req,scan_req_size);
	os_free(off_ch_can_req);
}

#ifdef MAP_R2
void fill_off_ch_scan_req(struct wifi_app *wapp)
{
	u8 i = 0 , j = 0;
	char all_zero_mac[6] = {0};
	wapp->map->off_ch_scan_req->bw = 0;
	wapp->map->off_ch_scan_req->fresh_scan = 0x80;
	wapp->map->off_ch_scan_req->radio_num = wapp->map->off_ch_scan_capab->radio_num;
	wapp->map->off_ch_scan_req->neighbour_only = 2;

	for(i = 0; i < wapp->map->off_ch_scan_req->radio_num; i++) {
		j = 0;
		if (os_memcmp(wapp->map->off_ch_scan_req->body[i].radio_id,all_zero_mac,MAC_ADDR_LEN)!= 0) {
			wapp->map->off_ch_scan_req->body[i].oper_class_num = wapp->map->off_ch_scan_capab->radio_scan_params[i].oper_class_num;
			for(j = 0; j < wapp->map->off_ch_scan_req->body[i].oper_class_num; j++) {
				wapp->map->off_ch_scan_req->body[i].ch_body[j].oper_class = wapp->map->off_ch_scan_capab->radio_scan_params[i].ch_body[j].oper_class;
				wapp->map->off_ch_scan_req->body[i].ch_body[j].ch_list_num = 0;
			}
		}
	}
	wapp->map->f_scan_req = 1;
}
#endif

int map_receive_off_channel_scan_req(
	struct wifi_app *wapp, char *msg_buf,unsigned short msg_len)
{
	// handle the scan_scan_req, compare with capabilities
	// start the scan state machine if fresh_scan bit is set, else give the old results
	//create and send the scan command to driver
	// wait for scan report messages, start a single channel scan timer and complete channel scan timer.
	// scan state : wait for scan result;
	// Handle error cases.

	// current radio/current channel

	struct off_ch_scan_req_s *off_ch_scan_req = (struct off_ch_scan_req_s *)msg_buf;
#ifdef MAP_R2
	static u8 first_scan = 1;
#endif
	if(wapp->map->off_ch_scan_req != NULL)
		os_free(wapp->map->off_ch_scan_req);

	wapp->map->off_ch_scan_req = os_zalloc(msg_len);

	if(wapp->map->off_ch_scan_req == NULL)
		return MAP_ERROR;
	
	os_memcpy(wapp->map->off_ch_scan_req, msg_buf, msg_len);
	wapp->map->off_ch_scan_req_len = msg_len;

	if((off_ch_scan_req->fresh_scan & 0x80) == 0 && (wapp->map->off_ch_scan_rep != NULL) && (wapp->map->off_ch_scan_rep->scan_result_num !=0)) {
		// Give previous result
#ifdef MAP_R2
		if (wapp->map->f_scan_req == 1)
			map_build_and_send_net_opt_scan_rep(wapp);
		else
#endif
		map_build_and_send_off_ch_scan_rep(wapp);
	} else {
#ifdef MAP_R2
		if(first_scan == 0 && (off_ch_scan_req->fresh_scan & 0x80) == 0) {
			printf("Raghav: received scan req for old scan but no report present: ERROR!\n ");
			map_build_and_send_ch_scan_rep(wapp);
			return MAP_SUCCESS;
		}
		if((wapp->map->off_ch_scan_rep == NULL) &&((off_ch_scan_req->fresh_scan & 0x80) == 0))
		{
			printf("SSS received scan req for old scan but no report present: ERROR!\n ");
			fill_off_ch_scan_req(wapp);	
		}
#endif
		//perform fresh scan
		wapp->map->off_ch_scan_state.ch_scan_state = CH_SCAN_ONGOING;
		eloop_cancel_timeout(off_ch_scan_timeout, wapp, NULL);
		eloop_register_timeout(CH_SCAN_TIMEOUT, 0, off_ch_scan_timeout, wapp, NULL);
		if (map_start_off_ch_scan(wapp) == MAP_ERROR) {
			// TODO: Raghav : error handling
			DBGPRINT(RT_DEBUG_OFF,"%s %d\n", __func__, __LINE__);
		}
	}
	return MAP_SUCCESS;
}

u8 rssi_to_rcpi(signed char rssi)
{
	if (!rssi)
		return 255; /* not available */
	if (rssi < -110)
		return 0;
	if (rssi > 0)
		return 220;
	return (rssi + 110) * 2;
}

void wdev_handle_off_ch_scan_response(struct wifi_app *wapp, u32 ifindex, OFFCHANNEL_INFO *ch_info)
{
	// should be able to handle multiple events for a single channel, coz of size restrictions.
	// create a list of bss in scan report
	struct wapp_dev *wdev = NULL;
	struct off_ch_scan_result_tlv *scan_result = NULL;
	struct off_ch_scan_state_ctrl *scan_state = &wapp->map->off_ch_scan_state;
	u8 op_class = wapp->map->off_ch_scan_req->body[scan_state->curr_scan_radio_idx].ch_body[scan_state->curr_oper_class_idx].oper_class;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		goto End;

	if(wapp->map->off_ch_scan_state.ch_scan_state == CH_SCAN_IDLE) {
		DBGPRINT(RT_DEBUG_OFF,"Scan Results received in invalid state\n");
		goto End;
	}

	scan_result = os_zalloc(sizeof(struct off_ch_scan_result_tlv));

	//DBGPRINT(RT_DEBUG_OFF,"%s %d : %p\n", __func__, __LINE__, scan_result);

	if(scan_result == NULL) {
		DBGPRINT(RT_DEBUG_OFF,"%s : cannot alloc mem\n", __func__);
		// TODO: error handling
		return;
	}

	dl_list_add(&wapp->map->off_ch_scan_rep->scan_result_list, &scan_result->list);
	wapp->map->off_ch_scan_rep->scan_result_num++;

	// fill the common fields
	MAP_GET_RADIO_IDNFER(wdev->radio,scan_result->radio_id);
	scan_result->oper_class = op_class;
	scan_result->channel = ch_info->channel;
	scan_result->scan_status = SCAN_SUCCESS;
	if (ch_info->actual_measured_time)
		scan_result->utilization = ((ch_info->channel_busy_time)*255)/(ch_info->actual_measured_time*1000);
	else
		scan_result->utilization = ((ch_info->channel_busy_time)*255)/(120*1000);
	scan_result->noise = rssi_to_rcpi((s8)ch_info->NF);
	scan_result->scan_type = wapp->map->off_ch_scan_state.scan_msg.data.offchannel_param.scan_type[0];
	scan_result->scan_type  = IsRadarChannel(scan_result->channel)?0x0:0x80;//0 for passive , 1 for active 

	scan_result->agg_scan_duration= IsRadarChannel(scan_result->channel)?120:60; 
	scan_result->timestamp_len = wapp->map->off_ch_scan_state.last_scan_tm.timestamp_len;
	os_memcpy(scan_result->timestamp,wapp->map->off_ch_scan_state.last_scan_tm.timestamp, wapp->map->off_ch_scan_state.last_scan_tm.timestamp_len);

	wdev->scan_cookie = random();
#ifdef MAP_R2
	scan_result->rx_time = ch_info->rx_time;
	scan_result->tx_time = ch_info->tx_time;
	scan_result->obss_time = ch_info->obss_time;
	scan_result->edcca = ch_info->edcca;
#endif
	DBGPRINT(RT_DEBUG_OFF,"scan Completed for :%d\n",scan_result->channel);
#if 0
	wapp_query_scan_result(wapp, wdev,0);
#endif
	scan_state->ch_per_op_left --;
	DBGPRINT(RT_DEBUG_OFF,"scan_state->ch_per_op_left %d\n", scan_state->ch_per_op_left);
	DBGPRINT(RT_DEBUG_OFF,"scan_state->curr_ch_idx %d\n", scan_state->curr_ch_idx);

	if((scan_state->curr_ch_idx % MAX_AWAY_CHANNEL) == 0
		|| (scan_state->ch_per_op_left == 0)) {
		wapp_query_scan_result(wapp, wdev,0);
	} else {
		scan_state->curr_ch_idx++;
	}
	//DBGPRINT(RT_DEBUG_OFF,"%s %d\n", __func__, __LINE__);

	End:
		// TODO: error handling.
	return;
}

int wapp_event_offchannel_info(struct wifi_app *wapp, u8* buf)
{
	POFFCHANNEL_SCAN_MSG ch_scan_result = (POFFCHANNEL_SCAN_MSG)buf;
	u32 ifindex = if_nametoindex((const char *)ch_scan_result->ifrn_name);
	DBGPRINT(RT_DEBUG_OFF,"%s %d\n", __func__, wapp->map->off_ch_scan_state.ch_scan_state);
	if (wapp->map->off_ch_scan_state.ch_scan_state != CH_SCAN_IDLE) {
		DBGPRINT(RT_DEBUG_OFF,"Off channel scan resp in valid state\n");
		if(ch_scan_result->Action == OFFCHANNEL_INFO_RSP)
			wdev_handle_off_ch_scan_response(wapp, ifindex,
			&ch_scan_result->data.channel_data);
		else {
			DBGPRINT(RT_DEBUG_OFF,"Raghav:Invalid response from driver:%d\n", ch_scan_result->Action);
		}
	}
	
#ifdef MAP_R2
	ifindex = ch_scan_result->ifIndex;
	printf("in WAPP ,got DFS wala event wapp_event_offchannel_info %d from %s\n", ch_scan_result->Action, ch_scan_result->ifrn_name);
	if(ch_scan_result->Action == DFS_RADAR_HIT) {
		
		wdev_handle_cac_stop(wapp, ifindex, &ch_scan_result->data.operating_ch_info.channel, TRUE, TRUE); 
	}
#endif
	return 0;
}

void wapp_fill_ch_bw_str(struct neighbor_info *dst, 
	wdev_ht_cap ht_cap,
	wdev_vht_cap vht_cap)
{
	u8 ch_bw =0;
	if(ht_cap.ht_40 == 0)
		ch_bw = BW_20;
	else {
		if(vht_cap.vht_160 == 0)
			ch_bw = BW_40;
		else if(vht_cap.vht_160 == 1)
			ch_bw = BW_80;
		else if(vht_cap.vht_160 == 2)
			ch_bw = BW_160;
		if(vht_cap.vht_8080 == 1)
			ch_bw = BW_8080;
	}

	switch(ch_bw) {
	case BW_20:
		snprintf((char *)dst->ch_bw, sizeof(dst->ch_bw), "%s", "20MHz");
		dst->ch_bw_len = os_strlen("20MHz")+1;
		break;
	case BW_40:
		snprintf((char *)dst->ch_bw, sizeof(dst->ch_bw), "%s", "40MHz");
		dst->ch_bw_len = os_strlen("40MHz")+1;
		break;
	case BW_80:
		snprintf((char *)dst->ch_bw, sizeof(dst->ch_bw), "%s", "80MHz");
		dst->ch_bw_len = os_strlen("80MHz")+1;
		break;
	case BW_8080:
		snprintf((char *)dst->ch_bw, sizeof(dst->ch_bw), "%s", "80+80MHz");
		dst->ch_bw_len = os_strlen("80+80MHz")+1;
		break;
	case BW_160:
		snprintf((char *)dst->ch_bw, sizeof(dst->ch_bw), "%s", "160MHz");
		dst->ch_bw_len = os_strlen("160MHz")+1;
		break;
	default:
		break;
	}
}

u16 wapp_find_valid_neighbour_num(struct wifi_app *wapp,struct wapp_scan_info *scan_info,
	unsigned char scan_channel)
{
	u8 i,j;
	u16 neighbor_num =0;
	for (i=0; i<scan_info->bss_count;i++) {
		for (j = 0; j < MAX_NUM_OF_RADIO; j++) {
			if (wapp->map->apcli_configs[j].config_valid) {
				if ((os_memcmp(&scan_info->bss[i].Ssid,
					&wapp->map->apcli_configs[j].apcli_config.ssid,
					wapp->map->apcli_configs[j].apcli_config.SsidLen) == 0)
					&& (scan_channel == scan_info->bss[i].Channel))
				{
					printf("Valid SSIDnum is for %s\n",scan_info->bss[i].Ssid);
					neighbor_num ++;
				}
			}
		}
	}
	return neighbor_num;
}
u16 wapp_find_valid_neighbour_num_R2(struct wifi_app *wapp,struct wapp_scan_info *scan_info, unsigned char scan_channel)
{
	u8 i;
	u16 neighbor_num =0;
	printf("scan_info->bss_count %d\n", scan_info->bss_count);
	for (i=0; i<scan_info->bss_count;i++) {
		if(scan_channel == scan_info->bss[i].Channel) {
			DBGPRINT(RT_DEBUG_OFF,"channel scan info ssid %s, channel %d YES\n",scan_info->bss[i].Ssid, scan_info->bss[i].Channel);
			neighbor_num ++;
		} else {
			DBGPRINT(RT_DEBUG_OFF,"channel scan info ssid %s, channel %d NO\n",scan_info->bss[i].Ssid, scan_info->bss[i].Channel);
		}
	}
	return neighbor_num;
}
void wapp_fill_nb_info_R2(struct wifi_app *wapp,struct neighbor_info *dst,
struct wapp_scan_info *scan_info, unsigned char scan_channel)
{
	u8 i,is_valid =0,j;
	u8 is_valid_cnt =0;
	for (i=0; i<scan_info->bss_count;i++) {
		is_valid =0;
#ifdef MAP_R2
		if(wapp->map->off_ch_scan_req->neighbour_only == 2) {
			if(scan_channel == scan_info->bss[i].Channel) {
				is_valid = 1;
			}
		} else
#endif
		{
		for (j = 0; j < MAX_NUM_OF_RADIO; j++) {
			if (wapp->map->apcli_configs[j].config_valid) {
				 if (os_memcmp(&scan_info->bss[i].Ssid,
						&wapp->map->apcli_configs[j].apcli_config.ssid,
						wapp->map->apcli_configs[j].apcli_config.SsidLen) == 0)
				{
					is_valid =1;
					break;
				}
			}
		}
		}
		if (is_valid) {
			os_memcpy(dst[is_valid_cnt].bssid, scan_info->bss[i].Bssid, MAC_ADDR_LEN);
		if(scan_info->bss[i].SsidLen <= MAX_LEN_OF_SSID) {
				dst[is_valid_cnt].ssid_len = scan_info->bss[i].SsidLen;
				os_memcpy(dst[is_valid_cnt].ssid,
					scan_info->bss[i].Ssid, dst[is_valid_cnt].ssid_len);
		}
			dst[is_valid_cnt].RCPI = rssi_to_rcpi(scan_info->bss[i].Rssi);
			wapp_fill_ch_bw_str(&dst[is_valid_cnt],
				scan_info->bss[i].ht_cap,
				scan_info->bss[i].vht_cap);
#ifdef MAP_R2
			dst[is_valid_cnt].cu_stacnt_present = scan_info->bss[i].QbssLoad.bValid == 1? 0xc0:0;
			if(scan_info->bss[i].QbssLoad.bValid) {
			dst[is_valid_cnt].cu = scan_info->bss[i].QbssLoad.ChannelUtilization;
			dst[is_valid_cnt].sta_cnt= scan_info->bss[i].QbssLoad.StaNum;
			} else {
				dst[is_valid_cnt].cu = 0;
				dst[is_valid_cnt].sta_cnt= 0;
			}

#endif
			is_valid_cnt++;
		}
	}
}

void wapp_fill_nb_info(struct wifi_app *wapp,struct neighbor_info *dst,
struct wapp_scan_info *scan_info)
{
	u8 i,is_valid =0,j;
	u8 is_valid_cnt =0;
	for (i=0; i<scan_info->bss_count;i++) {
		is_valid =0;
#ifdef MAP_R2
		if(wapp->map->off_ch_scan_req->neighbour_only == 2)
			is_valid = 1;
		else
#endif
		{
		for (j = 0; j < MAX_NUM_OF_RADIO; j++) {
			if (wapp->map->apcli_configs[j].config_valid) {
				 if (os_memcmp(&scan_info->bss[i].Ssid,
						&wapp->map->apcli_configs[j].apcli_config.ssid,
						wapp->map->apcli_configs[j].apcli_config.SsidLen) == 0)
			 	{
			 		printf("Valid SSID is %s\n",wapp->map->apcli_configs[j].apcli_config.ssid);
				 	is_valid =1;
					break;
			 	}
			}
		}
		}
		if (is_valid) {
			os_memcpy(dst[is_valid_cnt].bssid, scan_info->bss[i].Bssid, MAC_ADDR_LEN);
		if(scan_info->bss[i].SsidLen <= MAX_LEN_OF_SSID) {
				dst[is_valid_cnt].ssid_len = scan_info->bss[i].SsidLen;
				os_memcpy(dst[is_valid_cnt].ssid, 
					scan_info->bss[i].Ssid, dst[is_valid_cnt].ssid_len);
		}
			dst[is_valid_cnt].RCPI = rssi_to_rcpi(scan_info->bss[i].Rssi);
			wapp_fill_ch_bw_str(&dst[is_valid_cnt],
				scan_info->bss[i].ht_cap,
				scan_info->bss[i].vht_cap);
#ifdef MAP_R2
			dst[is_valid_cnt].cu_stacnt_present = scan_info->bss[i].QbssLoad.bValid == 1? 0xc0:0;
			if(scan_info->bss[i].QbssLoad.bValid) {
			dst[is_valid_cnt].cu = scan_info->bss[i].QbssLoad.ChannelUtilization;
			dst[is_valid_cnt].sta_cnt= scan_info->bss[i].QbssLoad.StaNum;
			} else {
				dst[is_valid_cnt].cu = 0;
				dst[is_valid_cnt].sta_cnt= 0;
			}

#endif
			is_valid_cnt++;
		}
	}
}

void wapp_update_scan_master_list(struct wifi_app *wapp,struct off_ch_scan_result_tlv *scan_result)
{

}

void wdev_fill_net_opt_scan_results_v2(struct wifi_app *wapp,
	struct wapp_scan_info *scan_info, u8 is_last)
{
	struct off_ch_scan_result_tlv *scan_result = NULL;
	struct off_ch_scan_report *scan_rep = wapp->map->off_ch_scan_rep;	
	u8 tmp_nb_num, valid_entries;
	int size =0;

	if(scan_info->bss_count == 0) {
		goto End;
	}
	dl_list_for_each(scan_result, &scan_rep->scan_result_list, struct off_ch_scan_result_tlv, list) {
		tmp_nb_num = scan_result->neighbor_num;
#ifdef MAP_R2
		if(wapp->map->off_ch_scan_req->neighbour_only == 2) {
			if(scan_result->neighbor_num == 0) {
				scan_result->neighbor_num =
					wapp_find_valid_neighbour_num_R2(wapp, scan_info, scan_result->channel);
				if (scan_result->neighbor_num == 0)
					continue;
				size = scan_result->neighbor_num * sizeof(struct neighbor_info);
				scan_result->nb_info = (struct neighbor_info*)os_zalloc(size);
			} else {
				tmp_nb_num = scan_result->neighbor_num;
				valid_entries =
					wapp_find_valid_neighbour_num_R2(wapp, scan_info, scan_result->channel);
				if (valid_entries == 0)
					continue;
				scan_result->neighbor_num += valid_entries;
				size = scan_result->neighbor_num * sizeof(struct neighbor_info);
				scan_result->nb_info = os_realloc(scan_result->nb_info,size);
			}
			if(scan_result->nb_info == NULL)
				goto End;
			wapp_fill_nb_info_R2(wapp,&scan_result->nb_info[tmp_nb_num],
				scan_info, scan_result->channel);
		}
		else
#endif
		{
			if(scan_result->neighbor_num == 0) {
				scan_result->neighbor_num =
					wapp_find_valid_neighbour_num(wapp, scan_info, scan_result->channel);
				if (scan_result->neighbor_num == 0)
					continue;
				size = scan_result->neighbor_num * sizeof(struct neighbor_info);
				scan_result->nb_info = os_zalloc(size);
			} else {
				valid_entries =
					wapp_find_valid_neighbour_num(wapp, scan_info, scan_result->channel);
				if (valid_entries == 0)
					continue;
				scan_result->neighbor_num += valid_entries;
				size = scan_result->neighbor_num * sizeof(struct neighbor_info);
				scan_result->nb_info = os_realloc(scan_result->nb_info,size);
			}
			if(scan_result->nb_info == NULL)
				goto End;
			wapp_fill_nb_info(wapp,&scan_result->nb_info[tmp_nb_num], scan_info);
		}
		}
End:
	return;
}

void wdev_fill_net_opt_scan_results(struct wifi_app *wapp,
	struct wapp_scan_info *scan_info, u8 is_last)
{
	struct off_ch_scan_result_tlv *scan_result = dl_list_first(&(wapp->map->off_ch_scan_rep->scan_result_list),
		struct off_ch_scan_result_tlv, list);
	u8 tmp_nb_num, valid_entries;
	int i;
	int size =0;

	if(scan_result == NULL)
	{
		printf("ScanList  is NULL\n");
		return;
	}
	if(scan_result->neighbor_num == 0) {
		if(scan_info->bss_count == 0)
			goto End;
#ifdef MAP_R2
		if(wapp->map->off_ch_scan_req->neighbour_only == 2) {
			scan_result->neighbor_num = wapp_find_valid_neighbour_num_R2(wapp,scan_info, scan_result->channel);
		}
		else
#endif
		{
			scan_result->neighbor_num = wapp_find_valid_neighbour_num(wapp,scan_info, scan_result->channel);
		}
		if (scan_result->neighbor_num == 0)
			goto End;
		size = scan_result->neighbor_num*sizeof(struct neighbor_info);
		scan_result->nb_info = os_zalloc(scan_result->neighbor_num*sizeof(struct 
			neighbor_info));
		if(scan_result->nb_info == NULL)
			goto End;
#ifdef MAP_R2
		wapp_fill_nb_info_R2(wapp,scan_result->nb_info, scan_info, scan_result->channel);
#else
		wapp_fill_nb_info(wapp,scan_result->nb_info, scan_info);
#endif
	} else {
		tmp_nb_num = scan_result->neighbor_num;
#ifdef MAP_R2
		if(wapp->map->off_ch_scan_req->neighbour_only == 2) {
			//valid_entries = scan_info->bss_count;
			valid_entries = wapp_find_valid_neighbour_num_R2(wapp,scan_info, scan_result->channel);
		} else
#endif
		{
			valid_entries = wapp_find_valid_neighbour_num(wapp,scan_info, scan_result->channel);
		}
		if(scan_info->bss_count == 0)
			goto End;
		if (valid_entries == 0)
			goto End;
		scan_result->neighbor_num += valid_entries;
		size = scan_result->neighbor_num*sizeof(struct neighbor_info);
		scan_result->nb_info = os_realloc(scan_result->nb_info,size);
		if(scan_result->nb_info == NULL)
			goto End;
#ifdef MAP_R2
		wapp_fill_nb_info_R2(wapp,&scan_result->nb_info[tmp_nb_num], scan_info, scan_result->channel);
#else
		wapp_fill_nb_info(wapp,&scan_result->nb_info[tmp_nb_num], scan_info);
#endif
	}
End:
	if(is_last) {
		for(i=0; i<wapp->map->off_ch_scan_state.scan_msg.data.offchannel_param.Num_of_Away_Channel; i++) {
			scan_result->agg_scan_duration += wapp->map->off_ch_scan_state.scan_msg.data.offchannel_param.scan_time[i];
		}
		printf(" %s: %d\n", __func__, scan_result->neighbor_num);
		wapp_update_scan_master_list(wapp,scan_result);
	}
	return;
}

void wdev_handle_off_ch_scan_results(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_scan_info *scan_info = NULL;
	int scan_done = 1;

	if (!wapp)
		goto End;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		goto End;

	if (!wdev->scan_cookie)
		goto End;
	DBGPRINT(RT_DEBUG_TRACE, "%s for %s\n", __func__, wdev->ifname);

	scan_info = &event_data->scan_info;
	scan_info->interface_index = ifindex;
#if 0
	if (wapp->map->off_ch_scan_req->neighbour_only == FALSE) {
	scan_result->neighbor_num += scan_info->bss_count;
	}
#endif
	if (scan_info->more_bss) {
		scan_done = 0;
		wapp_query_scan_result(wapp, wdev, 1);
	}

	if(wapp->map->off_ch_scan_state.ch_scan_state != CH_SCAN_IDLE) { // scan result for R2 scan request
		if (wapp->map->off_ch_scan_req->neighbour_only != FALSE)
			wdev_fill_net_opt_scan_results_v2(wapp,scan_info, scan_done);
			/*wdev_fill_net_opt_scan_results(wapp,scan_info, scan_done);*/
		if (scan_done) {
			wdev_handle_off_ch_scan_complete(wapp,ifindex);
		}
	}
End:
	return;
}

