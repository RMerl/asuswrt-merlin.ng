/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2018, Mediatek, Inc.
 * *
 * * All rights reserved. Mediatek's source code is an unpublished work and the
 * * use of a copyright notice does not imply otherwise. This source code
 * * contains confidential trade secret material of Ralink Tech. Any attemp
 * * or participation in deciphering, decoding, reverse engineering or in any
 * * way altering the source code is stricitly prohibited, unless the prior
 * * written consent of Mediatek, Inc. is obtained.
 * ***************************************************************************
 *
 *  Module Name:
 *  1905 tlv parsor
 *
 *  Abstract:
 *  1905 tlv parsor
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Kapil.Gupta 2018/05/02    First implementation of the 1905 tlv parsor
 * */

#include "includes.h"
#ifdef __linux__
#include <fcntl.h>
#endif				/* __linux__ */

#include "common.h"
#include <sys/un.h>

#include "interface.h"
#include "./../1905_local_lib/data_def.h"
#include "client_db.h"
#include "mapd_i.h"
#include "topologySrv.h"
#include "eloop.h"
#include "tlv_parsor.h"
#include "ap_est.h"
#include "ch_planning.h"
#include "network_optimization.h"
#include "mapd_user_iface.h"
#include "utils/list.h"
#include "ctrl_iface.h"
#include "chan_mon.h"
#include "ap_cent_str.h"
#ifdef CENT_STR
#include "1905_map_interface.h"
#endif



void mapd_user_eth_client_leave(
	struct mapd_global *global,
	unsigned char *sta_mac, unsigned char *bssid)
{
	struct mapd_user_iface_eth_client_event *client_notif = NULL;
	struct mapd_user_event *user_event = NULL;
	struct ctrl_iface_global_priv *priv = global->ctrl_iface;

	user_event = (struct mapd_user_event *)os_zalloc(sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_iface_eth_client_event));
	if (!user_event) {
		err("mem alloc failed");
		return;
	}
	os_memset(user_event, 0, sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_iface_eth_client_event));

	user_event->event_id = ETHERNET_CLIENT_LEAVE_NOTIF;
	client_notif = (struct mapd_user_iface_eth_client_event *)user_event->event_body;

	os_memcpy(client_notif->sta_mac, sta_mac, ETH_ALEN);
	os_memcpy(client_notif->almac, bssid, ETH_ALEN);
	if (!dl_list_empty(&priv->ctrl_dst)) {
		mapd_ctrl_iface_send(global,
							priv->sock,
							&priv->ctrl_dst,
							(const char *)user_event, sizeof(struct mapd_user_event) +
							sizeof(struct mapd_user_iface_eth_client_event),
							priv);
	}
	os_free(user_event);
}
void mapd_user_eth_client_join(
	struct mapd_global *global,
	unsigned char *sta_mac, unsigned char *bssid)
{
	struct mapd_user_iface_eth_client_event *client_notif = NULL;
	struct mapd_user_event *user_event = NULL;
	struct ctrl_iface_global_priv *priv = global->ctrl_iface;

	user_event = (struct mapd_user_event *)os_zalloc(sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_iface_eth_client_event));
	if (!user_event) {
		err("mem alloc failed");
		return;
	}
	os_memset(user_event, 0, sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_iface_eth_client_event));

	user_event->event_id = ETHERNET_CLIENT_JOIN_NOTIF;
	client_notif = (struct mapd_user_iface_eth_client_event *)user_event->event_body;

	os_memcpy(client_notif->sta_mac, sta_mac, ETH_ALEN);
	os_memcpy(client_notif->almac, bssid, ETH_ALEN);
	if (!dl_list_empty(&priv->ctrl_dst)) {
		mapd_ctrl_iface_send(global,
							priv->sock,
							&priv->ctrl_dst,
							(const char *)user_event, sizeof(struct mapd_user_event) +
							sizeof(struct mapd_user_iface_eth_client_event),
							priv);
	}
	os_free(user_event);
}

void mapd_user_wireless_client_join(
	struct mapd_global *global,
	unsigned char *sta_mac, unsigned char *bssid, char *ssid)
{
	struct mapd_user_iface_wireless_client_event *client_notif = NULL;
	struct mapd_user_event *user_event = NULL;
	struct ctrl_iface_global_priv *priv = global->ctrl_iface;

	user_event = (struct mapd_user_event *)os_zalloc(sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_iface_wireless_client_event));
	if (!user_event) {
		err("mem alloc failed");
		return;
	}
	os_memset(user_event, 0, sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_iface_wireless_client_event));

	user_event->event_id = WIRELESS_CLIENT_JOIN_NOTIF;
	client_notif = (struct mapd_user_iface_wireless_client_event *)user_event->event_body;

	os_memcpy(client_notif->sta_mac, sta_mac, ETH_ALEN);
	os_memcpy(client_notif->bssid, bssid, ETH_ALEN);
	os_memcpy(client_notif->ssid, ssid, strlen(ssid));
	if (!dl_list_empty(&priv->ctrl_dst)) {
		mapd_ctrl_iface_send(global,
							priv->sock,
							&priv->ctrl_dst,
							(const char *)user_event, sizeof(struct mapd_user_event) +
							sizeof(struct mapd_user_iface_wireless_client_event),
							priv);
	}
	os_free(user_event);
}


void mapd_user_wireless_client_leave(
	struct mapd_global *global,
	unsigned char *sta_mac, unsigned char *bssid, char *ssid)
{
	struct mapd_user_iface_wireless_client_event *client_notif = NULL;
	struct mapd_user_event *user_event = NULL;
	struct ctrl_iface_global_priv *priv = global->ctrl_iface;

	user_event = (struct mapd_user_event *)os_zalloc(sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_iface_wireless_client_event));
	if (!user_event) {
		err("mem alloc failed");
		return;
	}
	os_memset(user_event, 0, sizeof(struct mapd_user_event) +
		sizeof(struct mapd_user_iface_wireless_client_event));

	user_event->event_id = WIRELESS_CLIENT_LEAVE_NOTIF;
	client_notif = (struct mapd_user_iface_wireless_client_event *)user_event->event_body;

	os_memcpy(client_notif->sta_mac, sta_mac, ETH_ALEN);
	os_memcpy(client_notif->bssid, bssid, ETH_ALEN);
	os_memcpy(client_notif->ssid, ssid, strlen(ssid));
	if (!dl_list_empty(&priv->ctrl_dst)) {
		mapd_ctrl_iface_send(global,
							priv->sock,
							&priv->ctrl_dst,
							(const char *)user_event, sizeof(struct mapd_user_event) +
							sizeof(struct mapd_user_iface_wireless_client_event),
							priv);
	}
	os_free(user_event);
}

/**
* @brief Fn to get tlv lenght
*
* @param buf msg buff
*
* @return tlv lenght
*/
int get_cmdu_tlv_length(unsigned char *buf)
{
	unsigned char *temp_buf = buf;
	int length;

	temp_buf += 1;		//shift to length field

	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	return (length + 3);
}

/**
* @brief Parse backhaul sterring tlv
*
* @param buf msg buff
* @param ctx own 1905 device ctx
*
* @return tlv lenght parsed if success else -1
*/
int parse_backhaul_steering_request_tlv(unsigned char *buf,
		struct own_1905_device *ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;

	temp_buf = buf;

	if ((*temp_buf) == BACKHAUL_STEERING_REQUEST_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	//shift to tlv value field
	temp_buf += 2;

	memcpy(ctx->bsteer_req.backhaul_mac, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	memcpy(ctx->bsteer_req.target_bssid, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	ctx->bsteer_req.oper_class = *temp_buf++;
	ctx->bsteer_req.channel = *temp_buf++;

	debug("backhaul_mac(%02x:%02x:%02x:%02x:%02x:%02x)"
			"target_bssid(%02x:%02x:%02x:%02x:%02x:%02x)"
			"operating class=%d, channel=%d",
			PRINT_MAC(ctx->bsteer_req.backhaul_mac),
			PRINT_MAC(ctx->bsteer_req.target_bssid),
			ctx->bsteer_req.oper_class, ctx->bsteer_req.channel);
	return (length + 3);
}

/**
* @brief Fn to update global steering params
*
* @param ctx own 1905 device ctx
* @param policy steering policy
*
* @return 0 if success else -1
*/
extern int8_t NOISE_OFFSET_BY_CH_WIDTH[];
int topo_srv_update_global_steer_params(struct own_1905_device *ctx, struct radio_policy_db *policy)
{
	struct radio_info_db *radio;
	struct steer_params *cli_steer = &ctx->cli_steer_params;

	radio = topo_srv_get_radio(topo_srv_get_1905_device(ctx, NULL), policy->identifier);
	if (!radio) {
		err("failed to found radio");
		return -1;
	}
	if (get_band_from_channel(radio->channel[0]) == BAND_5GH) {
		cli_steer->CUOverloadTh_5G_H = policy->ch_util_thres;
	} else if (get_band_from_channel(radio->channel[0]) == BAND_5GL) {
		cli_steer->CUOverloadTh_5G_L = policy->ch_util_thres;
	} else if (get_band_from_channel(radio->channel[0]) == BAND_2G) {
		cli_steer->CUOverloadTh_2G = policy->ch_util_thres;
	}
	cli_steer->LowRSSIAPSteerEdge_RE = policy->rssi_thres - NOISE_OFFSET_BY_CH_WIDTH[0];
	policy->rssi_thres = cli_steer->LowRSSIAPSteerEdge_RE;
	return 0;
}

/**
* @brief Fn to get radio sterring policy
*
* @param ctx own 1905 device ctx
* @param identifier radio identifier
* @param radio_policy steering policy to be filled
*
* @return 0 if sccess else -1
*/
int topo_srv_get_radio_steer_policy(struct own_1905_device *ctx, unsigned char *identifier, int8_t *radio_policy)
{
	struct radio_policy_db *policy;

	SLIST_FOREACH(policy, &ctx->map_policy.spolicy.radio_policy_head, radio_policy_entry) {
		if (os_memcmp(policy->identifier, identifier, ETH_ALEN) == 0) {
			*radio_policy = policy->steer_policy;
			return 0;
		}
	}

	return -1;
}
/**
* @brief Fn to get rssi threshould by steer policy
*
* @param ctx own 1905 device ctx
* @param identifier radio identifier
* @param rssi rssi threshould to be filled
*
* @return 0 if success else -1
*/
int topo_srv_get_rssi_th_by_policy(struct own_1905_device *ctx, struct mapd_radio_info *radio_info , int8_t *rssi)
{
	struct radio_policy_db *policy;

	SLIST_FOREACH(policy, &ctx->map_policy.spolicy.radio_policy_head, radio_policy_entry) {
		if (memcmp(policy->identifier, radio_info->identifier, ETH_ALEN) == 0) {
			*rssi = policy->rssi_thres + ap_est_get_noise_offset(ctx->back_ptr,radio_info->radio_idx);
			return 0;
		}
	}

	return -1;
}

/**
* @brief Fn to parse steer policy tlv
*
* @param buf msg buf
* @param ctx own 1905 device ctx
*
* @return -1 if error else tlv lenght
*/
int parse_steering_policy_tlv(unsigned char *buf, struct own_1905_device *ctx)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	int i =0;
	struct sta_db *sta = NULL;
	struct radio_policy_db *radio_policy = NULL;
	struct steer_policy *spolicy = &ctx->map_policy.spolicy;

	temp_buf = buf;

	if((*temp_buf) == STEERING_POLICY_TYPE) {
		temp_buf++;
	}
	else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	spolicy->local_disallow_count = *temp_buf++;
	for (i = 0; i < spolicy->local_disallow_count; i++) {
		sta = (struct sta_db *)os_malloc(sizeof(struct sta_db));
		if (!sta) {
			err("alloc struct sta_db for local_disallow_head fail");
			return -1;
		}
		memcpy(sta->mac, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
		SLIST_INSERT_HEAD(&spolicy->local_disallow_head, sta, sta_entry);
	}
	spolicy->btm_disallow_count = *temp_buf++;
	for (i = 0; i < spolicy->btm_disallow_count; i++) {
		sta = (struct sta_db *)os_malloc(sizeof(struct sta_db));
		if (!sta) {
			err("alloc struct sta_db for btm_disallow_head fail");
			return -1;
		}
		memcpy(sta->mac, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
		SLIST_INSERT_HEAD(&spolicy->btm_disallow_head, sta, sta_entry);
	}

	spolicy->radios = *temp_buf++;
	for (i = 0; i < spolicy->radios; i++) {
		radio_policy = (struct radio_policy_db *)os_malloc(sizeof(struct radio_policy_db));
		if (!radio_policy) {
			err("alloc struct radio_policy_db fail");
			return -1;
		}
		memcpy(radio_policy->identifier, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
		radio_policy->steer_policy = *temp_buf++;
		radio_policy->ch_util_thres = *temp_buf++;
		radio_policy->rssi_thres = *temp_buf++;
		radio_policy->rssi_thres = rcpi_to_rssi(radio_policy->rssi_thres);
		SLIST_INSERT_HEAD(&spolicy->radio_policy_head, radio_policy, radio_policy_entry);
		topo_srv_update_global_steer_params(ctx, radio_policy);
	}
	return (length+3);
}

/**
* @brief Fn to parse metric policy tlv
*
* @param buf msg buf
* @param ctx own 1905 device ctx
*
* @return -1 if error else tlv lenght
*/
int parse_metric_reporting_policy_tlv(unsigned char *buf,
		struct own_1905_device *ctx)
{
	unsigned char *temp_buf;
	unsigned short length = 0, i =0;
	struct metric_policy_db *metric_policy = NULL;
	struct metrics_policy *mpolicy = &ctx->map_policy.mpolicy;

	temp_buf = buf;

	if((*temp_buf) == METRIC_REPORTING_POLICY_TYPE) {
		temp_buf++;
	}
	else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	mpolicy->report_interval = *temp_buf++;
	mpolicy->radio_num = *temp_buf++;
	for (i = 0; i < mpolicy->radio_num; i++) {
		metric_policy = (struct metric_policy_db *)os_malloc(sizeof(struct metric_policy_db));
		if (!metric_policy) {
			err("alloc struct metric_policy_db fail");
			return -1;
		}
		memcpy(metric_policy->identifier, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
		metric_policy->rssi_thres = *temp_buf++;
		metric_policy->hysteresis_margin = *temp_buf++;
		metric_policy->ch_util_thres = *temp_buf++;
		metric_policy->sta_stats_inclusion = (*temp_buf & 0x80) >> 7;
		metric_policy->sta_metrics_inclusion = (*temp_buf & 0x40) >> 6;
		temp_buf += 1;
		SLIST_INSERT_HEAD(&mpolicy->policy_head, metric_policy, policy_entry);
	}
	return (length+3);
}

int parse_client_steering_btm_report_tlv(struct own_1905_device * ctx, struct _1905_map_device *dev, unsigned char *buf)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;
	unsigned char client_mac[ETH_ALEN] = {0};
	struct client* cli = NULL;
	unsigned char btm_status = 0;
		
	temp_buf = buf;


	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	//skip bssid field
	temp_buf += ETH_ALEN;

	//sta mac address
	os_memcpy(client_mac, temp_buf, ETH_ALEN);

	cli = client_db_get_client_from_sta_mac(ctx->back_ptr, client_mac);	
	temp_buf += ETH_ALEN;

	btm_status = *temp_buf;
#ifdef CENT_STR
	if(ctx->cent_str_en && cli ) {
		if(btm_status > 0 ) {
		debug(" btm fail report status:%d\n",btm_status);	
		 steer_fsm_trigger(ctx->back_ptr, cli->client_id, BTM_FAILURE, NULL);
		} 
		else {
		debug("btm success report status:%d\n",btm_status);			
		 steer_fsm_trigger(ctx->back_ptr, cli->client_id, BTM_SUCCESS, NULL);
		} 
	}	
#endif
	return (length+3);
}

int parse_client_steering_btm_report_message(struct own_1905_device *ctx, struct _1905_map_device *dev,
        unsigned char *buf)
{
	int length =0;
	unsigned char *temp_buf;
	temp_buf = buf;

	while(1)
	{
		if(*temp_buf == STEERING_BTM_REPORT_TYPE)
		{
			length = parse_client_steering_btm_report_tlv(ctx, dev, temp_buf + 1);

			if(length < 0)
			{
				err("error client steering btm report tlv\n");
				return -1;
			}
			temp_buf += length;
		} else if(*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}

	return 0;
}

/**
* @brief Fn to parse ap metrics policy tlv
*
* @param buf msg buf
* @param ctx own 1905 device ctx
*
* @return -1 if error else tlv lenght
*/
int parse_ap_metrics_query_tlv(unsigned char *buf, struct own_1905_device *ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;
	int i = 0;
	struct bss_db *bss = NULL, *tmp_bss = NULL;
	int bss_cnt = 0;

	temp_buf = buf;

	if ((*temp_buf) == AP_METRICS_QUERY_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	//shift to tlv value field
	temp_buf += 2;

	/*bssid num */
	temp_buf += 1;

	bss_cnt = (length - 1) / ETH_ALEN;
	debug("bss count is %d", bss_cnt);
	bss = SLIST_FIRST(&ctx->metric_entry.metrics_query_head);
	while (bss) {
		tmp_bss = SLIST_NEXT(bss, bss_entry);
		free(bss);
		bss = tmp_bss;
	}
	SLIST_INIT(&ctx->metric_entry.metrics_query_head);
	for (i = 0; i < bss_cnt; i++) {
		bss = (struct bss_db *)os_malloc(sizeof(struct bss_db));
		if (!bss) {
			err("alloc struct bss_db fail");
			return -1;
		}
		memcpy(bss->bssid, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
		debug("bssid(%02x:%02x:%02x:%02x:%02x:%02x) ",
			PRINT_MAC(bss->bssid));
		SLIST_INSERT_HEAD(&(ctx->metric_entry.metrics_query_head),
				bss, bss_entry);
	}
	return (length + 3);
}
#ifdef MAP_R2
int parse_ap_radio_identifier_tlv(unsigned char *buf, struct own_1905_device *ctx, unsigned short band_idx)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;
	//int i = 0;
	//struct bss_db *bss = NULL/*, *tmp_bss = NULL*/;
	//int bss_cnt = 0;
	temp_buf = buf;

	if ((*temp_buf) == AP_RADIO_IDENTIFIER_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));
	mapd_hexdump(MSG_OFF, "parse_ap_radio_identifier_tlv", buf, length+3);
	//shift to tlv value field
	temp_buf += 2;
	os_memcpy(ctx->metric_entry.radio_id[band_idx].identifier, temp_buf, ETH_ALEN);
	printf(MACSTR, MAC2STR(ctx->metric_entry.radio_id[band_idx].identifier));
	temp_buf += ETH_ALEN;
	return (length + 3);
}
int dump_ch_scan_info(struct radio_info_db *radio, char** reply_buf, size_t buf_len)
{
	struct scan_result_tlv *scan_result;
	unsigned char exist = 0;
	char *pos, *end;
	int ret = 0;
	s32 temp_ch_score;
	pos = *reply_buf;
	end = pos + buf_len;

	ret = os_snprintf(pos, end - pos, "{\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"identifier\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n", PRINT_MAC(radio->identifier));
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	//err("Radio ID "MACSTR"", MAC2STR(radio->identifier));
	ret = os_snprintf(pos, end - pos, "\"Channel scan result\":[\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	SLIST_FOREACH(scan_result, &radio->first_scan_result, next_scan_result)
	{
		ret = os_snprintf(pos, end - pos, "{\n");
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
		ret = os_snprintf(pos, end - pos, "\"Channel Number\":\"%u\",\n", scan_result->channel);
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
		ret = os_snprintf(pos, end - pos, "\"Channel Util\":\"%u\",\n", scan_result->utilization);
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
		ret = os_snprintf(pos, end - pos, "\"Channel Neighbour\":\"%u\",\n", scan_result->neighbor_num);
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
#if 0
		ret = os_snprintf(pos, end - pos, "\"Channel EDCCA\":\"%u\",\n", scan_result->cu_distribution.edcca_airtime);
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
#endif
		if(scan_result->ch_score <= -65535){
			/*keeping a high negative value for filtered out channel , so that graph formation for demo is ok*/
			temp_ch_score = -100;
		} else {
			temp_ch_score = scan_result->ch_score;
		}
		ret = os_snprintf(pos, end - pos, "\"Channel Condition\":\"%d\"", temp_ch_score);
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
		//always("Channel Number=%u\n", scan_result->channel);
		//always("Channel Util=%u\n", scan_result->utilization);
		//always("Channel Noise=%u\n", scan_result->noise);
		//always("Channel Condition=%u\n", scan_result->score);
		/*always("OperatingClass=%u\n", scan_result->oper_class);
		always("ScanStatus=%u\n", scan_result->scan_status);
		always("ChannelScanType=%u\n", scan_result->scan_type);
		always("Timestamp=%s\n", scan_result->timestamp);
		always("AggScanDuration=%u\n", scan_result->agg_scan_duration);
		always("cu_dis ch_num=%u\n", scan_result->cu_distribution.ch_num);
		always("cu_dis edcca airtime =%u\n", scan_result->cu_distribution.edcca_airtime);
		SLIST_FOREACH(nb_info,
			&scan_result->first_neighbor_info, next_neighbor_info)
		{
			always("NeighborBSSID=(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(nb_info->bssid));
			always("NeighborSSID=%s\n", nb_info->ssid);
			always("NeighborRCPI=%u\n", nb_info->RCPI);
			always("NeighborBW=%s\n", nb_info->ch_bw);
			if (nb_info->cu_stacnt_present & 0x80)
				always("NeighborCU: %u\n", nb_info->cu);
			if (nb_info->cu_stacnt_present & 0x40)
				always("NeighborSTACount: %u\n", nb_info->sta_cnt);
		}*/
		ret = os_snprintf(pos, end - pos, "},\n");
		if (ret < 0 || ret >= end - pos)
			return -1;
		pos += ret;
		exist = 1;
	}
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "]");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\n},\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	*reply_buf = pos;
	return 0;
}

int dump_score_info(struct score_info *cumulative_score, char* reply_buf, size_t buf_len, struct own_1905_device *ctx)
{
	char *pos, *end;
	int ret = 0;

	pos = reply_buf;
	end = pos + buf_len;

	ret = os_snprintf(pos, end - pos, "{\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"Channel Number\":\"%d\",\n", cumulative_score->channel);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Channel Score\":\"%d\",\n", cumulative_score->avg_score);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Channel Rank\":\"%d\"", cumulative_score->ch_rank);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
#if 0 
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		//err("Radio ID "MACSTR"",MAC2STR(radio->identifier));
		dump_ch_scan_info(radio, &pos, end - pos);
		exist = 1;
	}
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "],\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	exist = 1;
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
#endif
	ret = os_snprintf(pos, end - pos, "\n},\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	return pos - reply_buf;
}

int dump_dev_scan_info( struct _1905_map_device *dev, char* reply_buf, size_t buf_len, struct own_1905_device *ctx)
{
	struct radio_info_db *radio;
	struct _1905_device *_1905_info = &dev->_1905_info;
	unsigned char exist = 0;
	char *pos, *end;
	int ret = 0;
	int cnt = 0;

	pos = reply_buf;
	end = pos + buf_len;

	ret = os_snprintf(pos, end - pos, "{\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	ret = os_snprintf(pos, end - pos, "\"AL MAC\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\n", PRINT_MAC(_1905_info->al_mac_addr));
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	//err("DEVALMAC "MACSTR"",MAC2STR(dev->_1905_info.al_mac_addr));
	cnt = 0;
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		cnt++;
	}
	ret = os_snprintf(pos, end - pos, "\"Number of radios\":\"%d\",\n", cnt);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	ret = os_snprintf(pos, end - pos, "\"Radio Info\":[\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		//err("Radio ID "MACSTR"",MAC2STR(radio->identifier));
		dump_ch_scan_info(radio, &pos, end - pos);
		exist = 1;
	}
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "],\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	exist = 1;
	if (exist == 1) {
		pos -= 2;
		exist = 0;
	}
	ret = os_snprintf(pos, end - pos, "\n},\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	return pos - reply_buf;
}
#ifdef MAP_R2
int dump_ch_plan_score_info(struct own_1905_device *own_dev, char *buf, char* reply_buf, size_t buf_Len)
{
	char *pos, *end;
	int ret = 0;
	int exist = 0;
	int len = 0, total_len = 0;
	pos = reply_buf;
	end = pos + buf_Len;
	ret = os_snprintf(pos, end - pos, "{\n\"Channel Planning Score Information\":[ \n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	debug("*********ScoreTable****************");
	struct score_info *cumulative_score = NULL;
	SLIST_FOREACH(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score,next_ch_score) {
		debug("CH %d , avg score %d, rank %d", cumulative_score->channel, cumulative_score->avg_score, cumulative_score->ch_rank);
		len = dump_score_info(cumulative_score,pos, end - pos, own_dev);
		pos += len;
		exist = 1;
		if (len < 0) {
			break;
		} else {
			if (total_len + len > buf_Len - 3) {
				break;
			}
			else
				total_len += len;
		}
	}
#if 0
	while (dev) {
		if (dev->in_network == 0) {
			dev = topo_srv_get_next_1905_device(own_dev, dev);
			continue;
		}
		//err("DEVALMAC "MACSTR"",MAC2STR(dev->_1905_info.al_mac_addr));
		len = dump_dev_scan_info(dev, pos, end - pos, own_dev);
		//dump_dev_scan_info(dev);
		dev = topo_srv_get_next_1905_device(own_dev, dev);
		pos += len;
		exist = 1;
		if (len < 0) {
			break;
		} else {
			if (total_len + len > buf_Len - 3) {
				break;
			}
			else
				total_len += len;
		}
	}
#endif
	if (exist == 1)
		pos -= 2; // for truncating extra comma character in topology dump

	ret = os_snprintf(pos, end - pos, "]\n}\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	total_len  = pos - reply_buf;

	return total_len;
}
#endif
int dump_all_dev_scan_info(struct own_1905_device *own_dev, char *buf, char* reply_buf, size_t buf_Len)
{
	struct _1905_map_device *dev;
	dev = topo_srv_get_1905_device(own_dev, NULL);
	char *pos, *end;
	int ret = 0;
	int exist = 0;
	int len = 0, total_len = 0, i = 0;
	pos = reply_buf;
	end = pos + buf_Len;
	ret = os_snprintf(pos, end - pos, "{\n\"Channel Scan Information\":[ \n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;
	while (dev) {
		if (dev->in_network == 0) {
			dev = topo_srv_get_next_1905_device(own_dev, dev);
			continue;
		}
		//err("DEVALMAC "MACSTR"",MAC2STR(dev->_1905_info.al_mac_addr));
		len = dump_dev_scan_info(dev, pos, end - pos, own_dev);
		//dump_dev_scan_info(dev);
		dev = topo_srv_get_next_1905_device(own_dev, dev);
		pos += len;
		exist = 1;
		if (len < 0) {
			break;
		} else {
			if (total_len + len > buf_Len - 3) {
				break;
			}
			else
				total_len += len;
		}
	}
	if (exist == 1)
		pos -= 2; // for truncating extra comma character in topology dump

	ret = os_snprintf(pos, end - pos, "]\n}\n");
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	if (dev == NULL)
		ret = 0;//removing end character
	else
		ret = os_snprintf(pos, end - pos, "%03d", i);
	if (ret < 0 || ret >= end - pos)
		return -1;
	pos += ret;

	total_len  = pos - reply_buf;

	return total_len;
}

int insert_new_ch_scan_report_info(
		struct mapd_global * global,
		struct _1905_map_device *dev,
		unsigned char *buf,
		unsigned short len)
{
	unsigned char *pos = buf;
	struct radio_info_db *radio = NULL;
	struct own_1905_device *ctx = &global->dev;
	u8 i;

	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		if (ctx->user_triggered_scan == FALSE &&
				(radio->dev_ch_plan_info.dev_ch_plan_state != CHPLAN_STATE_SCAN_ONGOING &&
				global->dev.ch_planning_R2.ch_plan_state != CHPLAN_STATE_SCAN_ONGOING)) {
			continue;
		}
		if (os_memcmp(radio->identifier, pos, ETH_ALEN) == 0) {
			struct scan_result_tlv *res = NULL;
			u8 *pos_backup = pos;
			pos_backup += ETH_ALEN+1;
			struct scan_result_tlv *new_scan_result = NULL;
			//err("existing entries in scan result, check if channel entry exists");
			SLIST_FOREACH(res, &(radio->first_scan_result), next_scan_result) {
				//err("Scan results for channel %d", res->channel);
				//err("Util %d, NBnum %d score %d", res->utilization, res->neighbor_num, res->ch_score);
				//err("pos_backup channel %d",*pos_backup);
				if (*pos_backup == res->channel){
					//err("channel found");
					new_scan_result = res;
					break;
				}
			}
			if(!new_scan_result){
				new_scan_result = os_zalloc(sizeof(struct scan_result_tlv));
			}
			if (!new_scan_result) {
				err("alloc memory fail");
				return -1;
			}
			//SLIST_INSERT_HEAD(&(radio->first_scan_result), new_scan_result, next_scan_result);
			//SLIST_INIT(&(new_scan_result->first_neighbor_info));
			os_memcpy(new_scan_result->radio_id, pos, ETH_ALEN);
			pos += ETH_ALEN;
			new_scan_result->oper_class = *pos++;
			new_scan_result->channel = *pos++;
			new_scan_result->scan_status = *pos++;
			if (new_scan_result->scan_status != SCAN_SUCCESS) {
				debug("new_scan_result->channel %d", new_scan_result->channel);
				debug("scan fail, status: %u", new_scan_result->scan_status);
				if (new_scan_result->scan_status == OP_CLASS_CHAN_NOT_SUPP) {					
					os_free(new_scan_result);
					goto SCAN_FAIL;
				} else {			
					os_free(new_scan_result);
					ch_planning_scan_restart_due_to_failure(global);
					return -1;
				}
			}
			SLIST_INSERT_HEAD(&(radio->first_scan_result), new_scan_result, next_scan_result);
			SLIST_INIT(&(new_scan_result->first_neighbor_info));
			new_scan_result->timestamp_len = *pos++;
			os_memcpy(new_scan_result->timestamp, pos, new_scan_result->timestamp_len);
			pos += new_scan_result->timestamp_len;
			new_scan_result->utilization = *pos++;
			new_scan_result->noise = *pos++;
			new_scan_result->neighbor_num = (((*pos) << 8) | (*(pos + 1)));
			err("Add scan report for channel %d, Util %d NB %d",
				new_scan_result->channel,
				new_scan_result->utilization,
				new_scan_result->neighbor_num);
			if(new_scan_result->neighbor_num)
			{
				pos += 2;
				for (i = 0; i < new_scan_result->neighbor_num; i++) {
					struct nb_info *nb_info = os_zalloc(sizeof(struct nb_info));
					u8 tmp_len = 0;
					if(!nb_info) {
						err("alloc memory fail");
						os_free(new_scan_result);
						return -1;
					}
					SLIST_INSERT_HEAD(&(new_scan_result->first_neighbor_info), nb_info, next_neighbor_info);
					os_memcpy(nb_info->bssid, pos, ETH_ALEN);
					pos += ETH_ALEN;
					tmp_len = *pos++;
					if(tmp_len < MAX_LEN_OF_SSID)
						nb_info->ssid_len = tmp_len;
					else
						nb_info->ssid_len = MAX_LEN_OF_SSID;
					os_memcpy(nb_info->ssid, pos, nb_info->ssid_len);
					err("Neigh SSID %s, BSSID "MACSTR"", nb_info->ssid, MAC2STR(nb_info->bssid));
					pos += tmp_len;
					nb_info->RCPI = *pos++;
					tmp_len = 0;
					tmp_len = *pos++;
					if(tmp_len < MAX_CH_BW_LEN)
						nb_info->ch_bw_len = tmp_len;
					else
						nb_info->ch_bw_len = MAX_CH_BW_LEN;
					os_memcpy(nb_info->ch_bw, pos, nb_info->ch_bw_len);
					pos += tmp_len;
					nb_info->cu_stacnt_present = *pos++;
					if (nb_info->cu_stacnt_present & 0x80)
						nb_info->cu = *pos++;
					if (nb_info->cu_stacnt_present & 0x40) {
						nb_info->sta_cnt = *pos;
						pos += 2;
					}
				}
			}
SCAN_FAIL:
			//dump_ch_scan_info(radio); 
			topo_srv_update_ch_plan_scan_done(&global->dev,dev,radio);
			break;
			}
	}
	return 0;

}
int insert_new_ch_net_opt_report_info(struct _1905_map_device *dev, unsigned char *buf,
                struct net_opt_scan_report_event **scan_resp , unsigned int *resp_len)
{
	unsigned char a = 0;
	unsigned char *pos = buf;
	u16 neighbor_num = 0;
	struct net_opt_scan_report_event *temp = NULL;
	struct net_opt_scan_result_event *result = NULL;
	u8 *buf_result = NULL;

	*resp_len = *resp_len + sizeof(struct net_opt_scan_result_event);
	temp = os_realloc(*scan_resp,*resp_len);
	if(!temp)
	   return -1;

	buf_result	= (u8*)temp->scan_result;
	for (a = 0; a < temp->scan_result_num; a++)
	{
	  result = (struct net_opt_scan_result_event *)buf_result;
	  buf_result = buf_result + sizeof(struct net_opt_scan_result_event) +
			(result->neighbor_num * sizeof(struct neighbor_info));
	}
	result = (struct net_opt_scan_result_event *)buf_result;
	os_memcpy(result->radio_id, pos, ETH_ALEN);
	pos += ETH_ALEN;
	result->oper_class = *pos;
	pos++;
	result->channel = *pos;
	pos++;
	result->scan_status = *pos;
	pos++;
	err("scan_status: %u", result->scan_status);
	if (result->scan_status == 0) {
		result->timestamp_len = *pos;
		pos++;
		os_memcpy(result->timestamp, pos,
						result->timestamp_len);
		pos += result->timestamp_len;
		result->utilization = *pos++;
		result->noise = *pos++;
		result->neighbor_num = (((*pos) << 8) | (*(pos + 1)));
		pos += 2;
		if (result->neighbor_num) {
			*resp_len = *resp_len + (result->neighbor_num)*sizeof(struct neighbor_info);
			temp = os_realloc(temp, *resp_len);
			if(!temp)
				return -1;
			buf_result = (u8*)temp->scan_result;
			for (a = 0; a < temp->scan_result_num; a++) {
				result = (struct net_opt_scan_result_event *)buf_result;
				buf_result = buf_result + sizeof(struct net_opt_scan_result_event) +
					(result->neighbor_num * sizeof(struct neighbor_info));
			}
			result = (struct net_opt_scan_result_event *)buf_result;
		}
		for (neighbor_num = 0; neighbor_num < result->neighbor_num; neighbor_num++) {
			os_memcpy(result->nb_info[neighbor_num].bssid, pos, ETH_ALEN);
			pos += ETH_ALEN;
			result->nb_info[neighbor_num].ssid_len = *pos++;
			os_memset(result->nb_info[neighbor_num].ssid, 0, MAX_LEN_OF_SSID);
			os_memcpy(result->nb_info[neighbor_num].ssid, pos,
				result->nb_info[neighbor_num].ssid_len);
			pos += result->nb_info[neighbor_num].ssid_len;
			result->nb_info[neighbor_num].RCPI = *pos++;
			result->nb_info[neighbor_num].ch_bw_len = *pos++;
			os_memcpy(result->nb_info[neighbor_num].ch_bw, pos,
				result->nb_info[neighbor_num].ch_bw_len);
			pos += result->nb_info[neighbor_num].ch_bw_len;
			result->nb_info[neighbor_num].cu_stacnt_present = *pos++;
			if (result->nb_info[neighbor_num].cu_stacnt_present & 0x80)
				result->nb_info[neighbor_num].cu = *pos++;
			if (result->nb_info[neighbor_num].cu_stacnt_present & 0x40) {
				result->nb_info[neighbor_num].sta_cnt = *pos;
				pos += 2;
			}
		}
		temp->scan_result_num++;
		*scan_resp = temp;
		return 0;
	} else  {
		os_free(temp);
		*scan_resp = NULL;
		return -1;
	}
}

int parse_ch_scan_timestamp_tlv(struct mapd_global * global, struct _1905_map_device *dev, unsigned char *buf)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == TIMESTAMP_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));
	return (length+3);
}

int parse_ch_scan_result_tlv(struct mapd_global * global, struct _1905_map_device *dev, unsigned char *buf,
        struct net_opt_scan_report_event **scan_resp , unsigned int *resp_len)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == CHANNEL_SCAN_RESULT_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;
	if(dev->network_opt_per1905.network_opt_device_state == NETOPT_STATE_DATA_COLLECTION_ONGOING) {
		if (0 > insert_new_ch_net_opt_report_info(dev, temp_buf, scan_resp, resp_len)) {
			debug("insert_new_ch_net_opt_report_info");
			return -1;
		}
	}
	else {
		/*insert new channel preference info*/
		if (0 > insert_new_ch_scan_report_info(global, dev, temp_buf, length)) {
			debug("insert_new_ch_scan_report_info fail");
			return -1;
		}
	}
	return (length+3);
}
int parse_ch_scan_cu_tlv(
	struct mapd_global * global,
	struct _1905_map_device *dev,
	unsigned char *buf)
{
	unsigned char *temp_buf = buf;
	unsigned short length = 0, temp_len = 0;
	unsigned char func_type = 0;
	struct ch_util_lib ch_util;
	struct radio_info_db *radio = NULL;
	struct scan_result_tlv *result = NULL;
	struct own_1905_device *ctx = &global->dev;

	//calculate tlv length
	length = get_cmdu_tlv_length(temp_buf);

	//shift to tlv value field
	temp_buf += 3;
	temp_len += 3;
	if (os_memcmp(temp_buf, MTK_OUI, OUI_LEN)) {
		err("OUI not mtk_oui; do not handle!\n");
		return length;
	}
	temp_buf += OUI_LEN;
	func_type = *temp_buf++;
	debug("func_type %d, "MACSTR"", func_type, MAC2STR(dev->_1905_info.al_mac_addr));
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		if(ctx->user_triggered_scan == FALSE &&
			(radio->dev_ch_plan_info.dev_ch_plan_state != CHPLAN_STATE_SCAN_ONGOING &&
			global->dev.ch_planning_R2.ch_plan_state != CHPLAN_STATE_SCAN_ONGOING)) {
				continue;
		}
		if (func_type == FUNC_VENDOR_CHANNEL_UTIL_RSP) {
			while(temp_len < (length - 4)) {/*-4 is done to ignore OUI Len and FUNC_TYPE LEN)*/
				err("temp_len %d", temp_len);
				ch_util.ch_num = *temp_buf;
				temp_buf ++;
				temp_len ++;
				ch_util.edcca = (u32) *temp_buf;
				temp_buf += 4;
				temp_len += 4;
				debug("channel num %d, edcca %d", ch_util.ch_num, ch_util.edcca);
				radio = topo_srv_get_radio_by_band(dev,ch_util.ch_num);
				if(!radio)
					return length;
				SLIST_FOREACH(result,&radio->first_scan_result,next_scan_result) {
					if(ch_util.ch_num == result->channel) {
						result->cu_distribution.edcca_airtime = ch_util.edcca;
						result->cu_distribution.ch_num = ch_util.ch_num;
						break;
					}
				}
			}
		}
	}
	err(" ");
	return length;
}

int parse_channel_scan_report_message(struct mapd_global *global,
	unsigned char *buf, struct _1905_map_device *dev)
{
	int length =0;
	unsigned char *temp_buf;
	unsigned char integrity = 0;
	struct net_opt_scan_report_event *scan_resp = NULL;
	unsigned int resp_len = 0;

	temp_buf = buf;

	if(dev->network_opt_per1905.network_opt_device_state == NETOPT_STATE_DATA_COLLECTION_ONGOING)
	{
		resp_len = sizeof(struct net_opt_scan_report_event);
		scan_resp = (struct net_opt_scan_report_event*)os_zalloc(resp_len);
		if(scan_resp ==NULL) {
			err("alloc scan_resp fail");
			return -1;
		}
	}
	while(1) {
		if (*temp_buf == TIMESTAMP_TYPE) {
			integrity |= 0x1;
			length = parse_ch_scan_timestamp_tlv(global, dev, temp_buf);
			if(length < 0) {
				err("error timestamp tlv");
				goto PARSE_FAIL;
			}
			temp_buf += length;
		} else if (*temp_buf == CHANNEL_SCAN_RESULT_TYPE) {
			integrity |= 0x2;
			length = parse_ch_scan_result_tlv(global, dev, temp_buf, &scan_resp, &resp_len);
			if(length < 0) {
				err("error channel scan result tlv");
				goto PARSE_FAIL;
			}
			temp_buf += length;
		} else if (*temp_buf == VENDOR_SPECIFIC_TLV_TYPE) {
			integrity |= 0x2;
			length = parse_ch_scan_cu_tlv(global, dev, temp_buf);
			if(length < 0) {
				err("error channel scan result CU tlv");
				goto PARSE_FAIL;
			}
			temp_buf += length;
		} else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	//dump_dev_scan_info(dev);
PARSE_FAIL:
	if(dev->network_opt_per1905.network_opt_device_state == NETOPT_STATE_DATA_COLLECTION_ONGOING) {
		if(scan_resp) {
			dev->net_opt_scan_report = scan_resp;
			dev->network_opt_per1905.network_opt_device_state = NETOPT_STATE_DATA_COLLECTION_COMPLETE;
		}
	}
	/*check integrity*/
	if(integrity != 0x3) {
		err("incomplete channel scan report 0x%x 0x3\n",
			integrity);
		return -1;
	}
	if (length < 0) {
		return -1;
	}

	return 0;
}
int insert_new_radio_metrics(
		struct mapd_global * global,
		struct _1905_map_device *dev,
		unsigned char *buf, unsigned short len)
{
	unsigned char *pos = buf;
	struct radio_info_db *radio = NULL;
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		if (os_memcmp(radio->identifier, pos, ETH_ALEN) == 0) {
			os_memcpy(radio->radio_metrics.ra_id, pos, ETH_ALEN);
			pos += ETH_ALEN;
			radio->radio_metrics.cu_noise = *pos++;
			radio->radio_metrics.cu_tx = *pos++;
			radio->radio_metrics.cu_rx = *pos++;
			radio->radio_metrics.cu_other = *pos++;
			break;
		}
	}
#ifdef MAP_R2
	err(" other dev ch %d,  cu tx %d, cu rx %d",radio->channel[0], radio->radio_metrics.cu_tx , radio->radio_metrics.cu_rx );
	topo_srv_update_ch_planning_info(&global->dev,dev,NULL,radio,0);
#endif
	return 0;
}


int insert_new_ap_extended_metrics(
		struct mapd_global * global, struct _1905_map_device *dev, unsigned char *buf, unsigned short len)
{
	unsigned char *pos = buf;
	struct bss_info_db *bss = NULL;

	SLIST_FOREACH(bss, &dev->first_bss, next_bss) {
		if (os_memcmp(bss->bssid, pos, ETH_ALEN) == 0) {
			pos += ETH_ALEN;
			bss->uc_tx = ((*pos) << 24) | ((*(pos + 1)) << 16) |
						((*(pos + 2)) << 8) | (*(pos + 3));
			pos += 4;
			bss->uc_rx = ((*pos) << 24) | ((*(pos + 1)) << 16) |
						((*(pos + 2)) << 8) | (*(pos + 3));
			pos += 4;
			bss->mc_tx = ((*pos) << 24) | ((*(pos + 1)) << 16) |
						((*(pos + 2)) << 8) | (*(pos + 3));
			pos += 4;
			bss->mc_rx = ((*pos) << 24) | ((*(pos + 1)) << 16) |
						((*(pos + 2)) << 8) | (*(pos + 3));
			pos += 4;
			bss->bc_tx = ((*pos) << 24) | ((*(pos + 1)) << 16) |
						((*(pos + 2)) << 8) | (*(pos + 3));
			pos += 4;
			bss->bc_rx = ((*pos) << 24) | ((*(pos + 1)) << 16) |
						((*(pos + 2)) << 8) | (*(pos + 3));
			pos += 4;
			break;
		}
	}
	return 0;

}


int parse_assoc_ap_extended_metrics_tlv(struct own_1905_device * ctx, struct _1905_map_device *dev, unsigned char *buf)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == AP_EXTENDED_METRIC_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}
	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	if (0 > insert_new_ap_extended_metrics((struct mapd_global *)ctx->back_ptr, dev, temp_buf, length)) {
		debug("insert_new_ap_metrics_info fail");
		return -1;
	}

	return (length+3);
}
int parse_radio_metrics_tlv(struct own_1905_device * ctx, struct _1905_map_device *dev, unsigned char *buf)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == RADIO_METRIC_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}
	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	if (0 > insert_new_radio_metrics((struct mapd_global *)ctx->back_ptr, dev, temp_buf, length)) {
		debug("insert_new_ap_metrics_info fail");
		return -1;
	}

	return (length+3);
}
int insert_new_sta_ext_metrics(
		struct mapd_global * global, struct _1905_map_device *dev, unsigned char *buf, unsigned short len)
{
	unsigned char *pos = buf;
	struct associated_clients *sta = NULL;

	SLIST_FOREACH(sta, &dev->assoc_clients, next_client) {
		if (os_memcmp(sta->client_addr, pos, ETH_ALEN) == 0) {
			pos += ETH_ALEN;
			pos += sizeof(unsigned char) + ETH_ALEN;
			sta->sta_ext_info.last_data_ul_rate = ((*pos) << 24) | ((*(pos + 1)) << 16) |
													((*(pos + 2)) << 8) | (*(pos + 3));
			pos += 4;
			sta->sta_ext_info.last_data_dl_rate = ((*pos) << 24) | ((*(pos + 1)) << 16) |
													((*(pos + 2)) << 8) | (*(pos + 3));
			pos += 4;
			sta->sta_ext_info.utilization_rx = ((*pos) << 24) | ((*(pos + 1)) << 16) |
												((*(pos + 2)) << 8) | (*(pos + 3));
			pos += 4;
			sta->sta_ext_info.utilization_tx = ((*pos) << 24) | ((*(pos + 1)) << 16) |
												((*(pos + 2)) << 8) | (*(pos + 3));
			pos += 4;
			break;
		}
	}
	return 0;

}

int parse_assoc_sta_ext_metrics_tlv(struct own_1905_device * ctx, struct _1905_map_device *dev, unsigned char *buf)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == ASSOCIATED_STA_EXTENDED_LINK_METRIC_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}
	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	if (0 > insert_new_sta_ext_metrics((struct mapd_global *)ctx->back_ptr, dev, temp_buf, length)) {
		debug("insert_new_ap_metrics_info fail");
		return -1;
	}

	return (length+3);
}

int parse_assoc_status_notification_type_tlv(struct own_1905_device *ctx, struct _1905_map_device *dev, unsigned char *buf)
{
    unsigned char *temp_buf;
    unsigned short length = 0;
	int i, bss_num = 0;
	struct _1905_map_device *tmp_dev = NULL;
	struct bss_info_db *bss = NULL;

    temp_buf = buf;

    if((*temp_buf) == ASSOCIATION_STATUS_NOTIFICATION_TYPE) {
        err("assoc status nofification type:%02x \n", *temp_buf);
        temp_buf++;
    }
    else {
        return -1;
    }

    //calculate tlv length
    length = (*temp_buf);
    length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

    temp_buf += 2;

	bss_num = *temp_buf;
	temp_buf++;

	for (i = 0; i < bss_num; i++) {
		SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
			SLIST_FOREACH(bss, &dev->first_bss, next_bss) {
				if (os_memcmp(bss->bssid, temp_buf, ETH_ALEN) == 0) {
					temp_buf += ETH_ALEN;
					if (*temp_buf == 0)
						bss->status = 1;
					else
						bss->status = 0;
					temp_buf++;
				}
			}
		}
	}

    return (length+3);
}

int parse_assoc_status_notification_message(struct own_1905_device *ctx, struct _1905_map_device *dev, unsigned char *buf)
{
	int length =0;
	unsigned char *temp_buf;
	unsigned int integrity = 0;
	temp_buf = buf;

	while(1)
	{
		if(*temp_buf == ASSOCIATION_STATUS_NOTIFICATION_TYPE)
		{
			length = parse_assoc_status_notification_type_tlv(ctx, dev, temp_buf);

			if(length < 0)
			{
				err("error assoc status notification type tlv\n");
				return -1;
			}
			integrity |= (1 << 0);
			temp_buf += length;
		} else if(*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
		/*check integrity*/
	if(integrity != 1) {
		err("error when check tunneled message tlvs\n");
		return -1;
	}
	return 0;
}
int parse_client_disassciation_stats_message(struct own_1905_device *ctx, struct _1905_map_device *dev, unsigned char *buf)
{
	int length =0;
	unsigned char *temp_buf;
	unsigned int integrity = 0;
	temp_buf = buf;

	while(1)
	{
		/* One STA MAC Address TLV */
		if(*temp_buf == STA_MAC_ADDRESS_TYPE)
		{
			integrity |= (1 << 0);
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
		/* One Disassociation Reason Code TLV */
		else if (*temp_buf == DISASSOCIATION_REASON_CODE_TYPE) {
			integrity |= (1 << 1);
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
		/* One Associated STA Traffic Stats TLV */
		else if (*temp_buf == ASSOC_STA_TRAFFIC_STATS_TYPE) {
			integrity |= (1 << 2);
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		} else if(*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
		/*check integrity*/
	if(integrity != 0x7) {
		err("error when check client disassciation stats tlvs\n");
		return -1;
	}
	return 0;
}
int parse_bh_sta_report(struct own_1905_device *ctx, struct _1905_map_device *dev, unsigned char *buf)
{
	int length =0;
	unsigned char *temp_buf;
	unsigned int integrity = 0;
	temp_buf = buf;

	while(1)
	{
		if (*temp_buf == BACKHAUL_STA_RADIA_CAPABILITY_TYPE) {
			integrity |= (1 << 0);
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		} else if(*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}

	if (integrity == 0x0) {
		err("error when check backhaul sta cap report tlvs\n");
		return -1;
	}
	return 0;
}
int parse_cu_tlv(
	struct own_1905_device * ctx,
	struct _1905_map_device *dev,
	unsigned char *buf)
{
	unsigned char *temp_buf = buf;
	u8 temp_len = 0;
	unsigned short length = 0;
	unsigned char func_type = 0;
	struct ch_util_lib ch_util;
	struct radio_info_db *radio = NULL;

	//calculate tlv length
	length = get_cmdu_tlv_length(temp_buf);

	//shift to tlv value field
	temp_buf += 3;
	temp_len += 3;
	
	if (os_memcmp(temp_buf, MTK_OUI, OUI_LEN)) {
		err("OUI not mtk_oui; do not handle!\n");
		return length;
	}
	temp_buf += OUI_LEN;
	func_type = *temp_buf++;//FUNC_LEN is 1


	if (func_type == FUNC_VENDOR_CHANNEL_UTIL_RSP) {
		while(temp_len < (length - (OUI_LEN+1))) {
			ch_util.ch_num = *temp_buf++;
			temp_len++;
			ch_util.edcca = (u32)*temp_buf;
			temp_buf +=4;
			temp_len += 4;
			radio = topo_srv_get_radio_by_channel(dev,ch_util.ch_num);
			if(!radio)
				return length;
			debug("ch_util_channel is %d",ch_util.ch_num);
			radio->cu_distribution.edcca_airtime = ch_util.edcca;
			radio->cu_distribution.ch_num = ch_util.ch_num;
			debug(" other dev ");
			topo_srv_update_ch_planning_info(ctx,dev,NULL,radio,1);
		}
		
	}
	return length;
}

int parse_tunneled_message_type_tlv(struct own_1905_device *ctx, unsigned char *buf, u8 *tunnel_type)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == TUNNELED_MESSAGE_TYPE) {
		err("Tunneled Msg type:%02x \n", *temp_buf);
		temp_buf++;
	}
	else {
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	temp_buf += 2;

	*tunnel_type = *temp_buf++;

	return (length+3);
}

int parse_source_info_tlv(struct own_1905_device *ctx, unsigned char *buf, uint32_t *client_id)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;
	u8 mac[ETH_ALEN] = {0};
	struct mapd_global *global = ctx->back_ptr;
	u8 already_seen = 0;

	temp_buf = buf;

	if((*temp_buf) == SOURCE_INFO_TYPE) {
		err("Source Info type:%02x \n", *temp_buf);
		temp_buf++;
	}
	else {
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	temp_buf += 2;

	os_memcpy(mac, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	*client_id = client_db_track_add(global, mac, &already_seen);

	return (length+3);
}

int parse_tunneled_type_tlv(struct own_1905_device *ctx, unsigned char *buf, struct client *cli, u8 tunnel_type, struct _1905_map_device *dev)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	temp_buf = buf;
	struct associated_clients *sta = NULL;

	if((*temp_buf) == TUNNELED_TYPE) {
		err("Tunneled type:%02x \n", *temp_buf);
		temp_buf++;
	}
	else {
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	temp_buf += 2;
#if 0
	if (tunnel_type < MAX_TUNNEL_TYPE) {
		if (cli->tunnel_info[tunnel_type].tunneled_payload != NULL &&
			cli->tunnel_info[tunnel_type].tunnel_payload_len != length)
			cli->tunnel_info[tunnel_type].tunneled_payload =
			os_realloc(cli->tunnel_info[tunnel_type].tunneled_payload, length);
		else 
			cli->tunnel_info[tunnel_type].tunneled_payload = os_malloc(length);
		os_memcpy(cli->tunnel_info[tunnel_type].tunneled_payload, temp_buf, length);
	}
#endif
#ifdef CENT_STR
	if (ctx->cent_str_en && (tunnel_type == ASSOC_REQ_NORMAL || tunnel_type == ASSOC_REQ_REASSOC)) {
		parse_assoc_req((struct mapd_global *)ctx->back_ptr, cli->mac_addr,
			cli->bssid, temp_buf, length, cli->current_chan, tunnel_type);
		sta = topo_srv_get_associate_client(ctx, dev, cli->mac_addr);
		if (sta) {
			os_get_time(&(sta->stat_db.last_traffic_stats_time));
			sta->stat_db.last_bytes_recieved = 0;
			sta->stat_db.last_bytes_sent = 0;
		}
	}
#endif
	return (length+3);
}


int parse_tunneled_message(struct own_1905_device *ctx, unsigned char *buf, struct _1905_map_device *dev)
{
	int length =0;
	unsigned char *temp_buf = NULL;
	unsigned int integrity = 0;
	temp_buf = buf;
	struct client *cli = NULL;
	u8 tunnel_type = 255;
	uint32_t cli_id = 0;

	while(1)
	{
		if(*temp_buf == SOURCE_INFO_TYPE)
		{
			length = parse_source_info_tlv(ctx, temp_buf, &cli_id);

			if(length < 0)
			{
				err("error source info tlv\n");
				return -1;
			}
			integrity |= (1 << 0);
			temp_buf += length;
		} else if(*temp_buf == TUNNELED_MESSAGE_TYPE) {
			cli = client_db_get_client_from_client_id((struct mapd_global *)ctx->back_ptr, cli_id);
			if (cli)
				length = parse_tunneled_message_type_tlv(ctx, temp_buf, &tunnel_type);
			else
				length = get_cmdu_tlv_length(temp_buf);

			if(length < 0)
			{
				err("error tunneled message type tlv\n");
				return -1;
			}
			integrity |= (1 << 0);
			temp_buf += length;
		}  else if(*temp_buf == TUNNELED_TYPE) {
			cli = client_db_get_client_from_client_id((struct mapd_global *)ctx->back_ptr, cli_id);
			if (cli && tunnel_type >= 0 && tunnel_type < MAX_TUNNEL_TYPE)
				length = parse_tunneled_type_tlv(ctx, temp_buf, cli, tunnel_type, dev);
			else
				length = get_cmdu_tlv_length(temp_buf);

			if(length < 0)
			{
				err("error tunneled type tlv\n");
				return -1;
			}
			integrity |= (1 << 0);
			temp_buf += length;
		} else if(*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
		/*check integrity*/
	if(integrity != 1) {
		err("error when check tunneled message tlvs\n");
		return -1;
	}
	return 0;
}
int parse_1905_ack_message(struct own_1905_device *ctx, struct _1905_map_device *dev, unsigned char *buf)
{
	int length = 0;
	unsigned char *temp_buf;

	temp_buf = buf;

	while (1) {
		if (*temp_buf == ERROR_CODE_TYPE) {
			length = get_cmdu_tlv_length(temp_buf);

			if(length < 0) {
				err("error error code tlv \n");
				return -1;
			}
			temp_buf += length;
		}
		else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			/*ignore extra tlv*/
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}

	return 0;
}


#endif
int parse_ap_metrics_response_message(struct own_1905_device *ctx, struct _1905_map_device *dev,
        unsigned char *buf)
{
	int length =0;
	unsigned char *temp_buf;

	temp_buf = buf;
	while(1) {
		if (*temp_buf == AP_METRICS_TYPE) {
			length = parse_ap_metrics_tlv(ctx, dev, temp_buf);
			if(length < 0) {
				err("error ap metrics tlv");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == ASSOC_STA_TRAFFIC_STATS_TYPE) {
			length = parse_assoc_sta_traffic_stats_tlv(ctx, dev, temp_buf);
			if(length < 0) {
				err("error assoc sta traffic stats tlv");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == ASSOC_STA_LINK_METRICS_TYPE) {
			length = parse_assoc_sta_link_metrics_tlv(ctx, dev, temp_buf);
			if(length < 0) {
				err("error assoc sta link metrics tlv");
				return -1;
			}
			temp_buf += length;
		}
#ifdef MAP_R2
		/* Zero or more Radio Metrics TLVs */
		else if (*temp_buf == RADIO_METRIC_TYPE) {
			length = parse_radio_metrics_tlv(ctx, dev, temp_buf);
			if(length < 0) {
				err("error radio metrics tlv");
				return -1;
			}
			temp_buf += length;
		}
		else if (*temp_buf == TLV_802_11_VENDOR_SPECIFIC) {
			length = parse_cu_tlv(ctx, dev, temp_buf);
			if(length < 0) {
				err("error cu tlv");
				return -1;
			}
			temp_buf += length;
		}
		/* One or more AP Extended Metrics TLVs */
		else if (*temp_buf == AP_EXTENDED_METRIC_TYPE) {
			length = parse_assoc_ap_extended_metrics_tlv(ctx, dev, temp_buf);
			if(length < 0) {
				err("error assoc ap extended metric tlv");
				return -1;
			}
			temp_buf += length;
		}
		/* Zero or more Associated STA Extended Link Metrics TLVs */
		else if (*temp_buf == ASSOCIATED_STA_EXTENDED_LINK_METRIC_TYPE) {
			length = parse_assoc_sta_ext_metrics_tlv(ctx, dev, temp_buf);
			if(length < 0) {
				err("error assoc sta extended link metric tlv");
				return -1;
			}
			temp_buf += length;
		}
#endif
		else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	return 0;
}


/**
* @brief Fn to parse link metric query tlv
*
* @param buf msg buffer
* @param target target for the link metric
* @param type rx/tx/rx-tx
*
* @return -1 if error else tlv lenght
*/
int parse_link_metric_query_type_tlv(unsigned char *buf,
		unsigned char *target, unsigned char *type)
{
	unsigned char *temp_buf;
	int length = 0;

	temp_buf = buf;

	if ((*temp_buf) == LINK_METRICS_QUERY_TLV_TYPE)
		temp_buf++;
	else {
		return -1;
	}

	/*calculate tlv length */
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));
	/*shift to tlv value field */
	temp_buf += 2;

	/*set target mac = 0 if query all neighbors */
	if (*temp_buf == QUERY_ALL_NEIGHBOR) {
		memset(target, 0, ETH_ALEN);
		temp_buf += 1;

		/*add for IOT issue with BRCM */
		if (length == 8)
			temp_buf += ETH_ALEN;

	} else if (*temp_buf == QUERY_SPECIFIC_NEIGHBOR) {
		temp_buf += 1;
		memcpy(target, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
	} else {
		debug("reserve valus %d", *temp_buf);
		return -1;
	}

	if (*temp_buf == TX_METRICS_ONLY)
		*type = TX_METRICS_ONLY;
	else if (*temp_buf == RX_METRICS_ONLY)
		*type = RX_METRICS_ONLY;
	else if (*temp_buf == BOTH_TX_AND_RX_METRICS)
		*type = BOTH_TX_AND_RX_METRICS;

	return (length + 3);
}

/**
* @brief Fn to insert channel preference
*
* @param dev 1905 map device pointer
* @param buf msg buffer
* @param len msg len
*
* @return 0 if success else error
*/
int insert_new_ch_prefer_info(
	struct own_1905_device *ctx,
	struct _1905_map_device * dev,
	unsigned char *buf,
	unsigned short len)
{
	struct radio_ch_prefer *chcap = NULL;
	struct prefer_info_db *prefer = NULL;
	unsigned char *pos = buf;
	unsigned short check_len = 0, prefer_len = 0;
	unsigned char i = 0, j = 0, op_class = 0, ch_num = 0;

	debug("insert_new_ch_prefer_info");
	if (len < 7) {
		err("length error less than 7");
		return -1;
	}

	struct radio_info_db *radio = topo_srv_get_radio(dev, buf);
	if (!radio) {
		err("something worng here");
		return -1;
	}
	chcap = &radio->chan_preferance;
	pos += ETH_ALEN;
	check_len += ETH_ALEN;
	chcap->op_class_num = *pos;
	pos += 1;
	check_len += 1;
	chcap->is_valid = 1;

	for (i = 0; i < chcap->op_class_num; i++) {
		op_class = *pos;
		pos += 1;
		check_len += 1;
		ch_num = *pos;
		pos += 1;
		check_len += 1;
		prefer =
			(struct prefer_info_db *)
			os_malloc(sizeof(struct prefer_info_db));
		if (!prefer) {
			err("alloc struct prefer_info_db fail");
			return -1;
		}
		memset(prefer, 0, prefer_len);
		prefer->op_class = op_class;
		prefer->ch_num = ch_num;
		if (prefer->ch_num) {
			memcpy(prefer->ch_list, pos, ch_num);
			debug("opclass %d, chnum %d, ch[0] %d",
				prefer->op_class,
				prefer->ch_num,
				prefer->ch_list[0]);
			pos += ch_num;
			check_len += ch_num;
			/*bit 4~7 */
			prefer->perference = *pos >> 4;
			/*bit 0~3 */
			prefer->reason = *pos & 0x0f;
			pos += 1;
			check_len += 1;
		}else {
			/*if no ch_list under preference channel, make all channel to highest prefernce channel as MAP SEPC*/
			debug("ch num is 0 for this opclass %d",prefer->op_class);
			get_op_class_channel_list(op_class,prefer);
			/*bit 4~7 */
			prefer->perference = 0xf;
			/*bit 0~3 */
			prefer->reason = 0;
			pos += 1;
			check_len += 1;
		}
		if ((ctx->device_role == DEVICE_ROLE_CONTROLLER))
		{
			int i;
			for (i = 0; i < prefer->ch_num; i++)
			{
				ch_planning_add_ch_to_prefered_list(ctx,
					prefer->ch_list[i],
					radio,
					prefer->ch_list[i] > 14,
					prefer->perference,
					prefer->reason);
			}

		}

		SLIST_INSERT_HEAD(&(chcap->prefer_info_head), prefer,
				prefer_info_entry);
		debug("insert struct prefer_info_db");
		debug("opclass=%d, ch_num=%d perference=%d, reason=%d",
				prefer->op_class, prefer->ch_num,
				prefer->perference, prefer->reason);
		debug("ch_list: ");
		for (j = 0; j < prefer->ch_num; j++) {
			debug("%d ", prefer->ch_list[j]);
		}
	}
	if (len != check_len) {
		err("length mismatch len(%d) != check_len(%d)",
				len, check_len);
		delete_agent_ch_prefer_info(ctx, dev);
		return -1;
	}

	return 0;

}

/**
* @brief Fn to parse channel preferce tlv
*
* @param buf msg buffer
* @param dev 1905 map device pointer
*
* @return -1 if error else tlv length
*/
int parse_channel_preference_tlv(
	struct own_1905_device *ctx,
	unsigned char *buf,
	struct _1905_map_device * dev)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if ((*temp_buf) == CH_PREFERENCE_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	//shift to tlv value field
	temp_buf += 2;

	/*insert new channel preference info */
	if (0 > insert_new_ch_prefer_info(ctx, dev, temp_buf, length)) {
		debug("insert_new_ch_prefer_info fail");
		return -1;
	}
	dev->ch_preference_available = TRUE;
	return (length + 3);
}

/**
* @brief Fn to parse assoc sta link metrics
*
* @param buf msg buffer
* @param ctx own 1905 device ctx
*
* @return -1 if error else tlv lenght
*/
int parse_associated_sta_link_metrics_query_tlv(unsigned char *buf,
		struct own_1905_device *ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == STA_MAC_ADDRESS_TYPE) {
		temp_buf++;
	}
	else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	memcpy(ctx->metric_entry.assoc_sta, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	return (length+3);
}

/**
* @brief Fn to parse assoc link metrics tlv
*
* @param ctx own 1905 device pointer
* @param buf msg buffer
*
* @return -1 if error else tlv lenght
*/
int parse_associated_sta_link_metrics_query_message(struct own_1905_device *ctx,
		unsigned char *buf)
{
	int length = 0;
	unsigned char *temp_buf;
	unsigned char integrity = 0;

	temp_buf = buf;

	while (1) {
		if (*temp_buf == STA_MAC_ADDRESS_TYPE) {
			integrity |= 0x1;
			length =
				parse_associated_sta_link_metrics_query_tlv
				(temp_buf, ctx);
			if (length < 0) {
				err("error associated sta link metrics query tlv");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	/*check integrity */
	if (integrity != 0x1) {
		err("no sta mac address tlv");
		return -1;
	}

	return 0;
}

/**
* @brief Fn to insert new channel restriction info
*
* @param dev 1905 map device
* @param buf msg buffer
* @param len msg lenght
*
* @return 0 if success else error
*/
int insert_new_radio_oper_restrict(struct _1905_map_device * dev, unsigned char *buf, unsigned short len)
{
	struct oper_restrict *restriction = NULL;
	struct restrict_db *restrict_var = NULL;
	unsigned char *pos = buf;
	unsigned short check_len = 0, prefer_len = 0;
	unsigned char i = 0, j = 0, op_class = 0, ch_num = 0;

	debug("enter");
	if (len < 7) {
		err("length error less than 7");
		return -1;
	}

	struct radio_info_db *radio = topo_srv_get_radio(dev, pos);
	if (!radio) {
		err("something worng here");
		return -1;
	}
	restriction = &radio->chan_restrict;
	pos += ETH_ALEN;
	check_len += ETH_ALEN;
	restriction->op_class_num = *pos++;
	check_len += 1;
	restriction->is_valid = 1;

	debug("insert struct oper_restrict_db");
	for(i = 0; i < restriction->op_class_num; i++) {
		op_class = *pos++;
		check_len += 1;
		ch_num = *pos++;
		check_len += 1;
		restrict_var = (struct restrict_db *)os_malloc(sizeof(struct restrict_db));
		if (!restrict_var) {
			err("alloc struct restrict_db fail");
			return -1;
		}
		memset(restrict_var, 0 , prefer_len);
		restrict_var->op_class = op_class;
		restrict_var->ch_num = ch_num;
		for (j = 0; j < restrict_var->ch_num; j++) {
			restrict_var->ch_list[j] = *pos++;
			restrict_var->min_fre_sep[j] = *pos++;
			check_len += 2;
		}
		SLIST_INSERT_HEAD(&restriction->restrict_head, restrict_var, restrict_entry);
		debug("insert struct restrict_db");
		debug("opclass=%d, ch_num=%d", restrict_var->op_class, restrict_var->ch_num);
		debug("ch_list: ");
		for (j = 0; j < restrict_var->ch_num; j++) {
			debug("%d ", restrict_var->ch_list[j]);
		}
		debug("min_fre_sep_list: ");
		for (j = 0; j < restrict_var->ch_num; j++) {
			debug("%d ", restrict_var->min_fre_sep[j]);
		}
	}

	if (len != check_len) {
		err("length mismatch len(%d) != check_len(%d)",
				len, check_len);
		delete_agent_ch_prefer_info(NULL, dev);
		return -1;
	}

	return 0;
}

/**
* @brief Fn to parse radio operating restriction tlv
*
* @param buf msg buffer
* @param dev 1905 map device pointer
*
* @return -1 if error else tlv lenght
*/
int parse_radio_operation_restriction_tlv(unsigned char *buf, struct _1905_map_device *dev)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if ((*temp_buf) == RADIO_OPERATION_RESTRICTION_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	//shift to tlv value field
	temp_buf += 2;

	/*insert new channel preference info */
	if (0 >
			insert_new_radio_oper_restrict(dev, temp_buf,
				length)) {
		err("insert_new_radio_oper_restrict fail");
		return -1;
	}

	return (length + 3);
}

/**
* @brief Fn to add new ap metrics info
*
* @param ctx own 1905 device ctx
* @param dev 1905 map device pointer
* @param buf msg buffer
* @param len msg leng
*
* @return 0 if success else -1
*/
int insert_new_ap_metrics_info(
		struct own_1905_device * ctx, struct _1905_map_device *dev, unsigned char *buf, unsigned short len)
{
	unsigned char *pos = buf;
	unsigned short check_len = 0;
	struct esp_db *esp = NULL;
	struct mapd_global *global = ctx->back_ptr;
	struct bss_info_db *mrsp = topo_srv_get_bss_by_bssid(ctx, dev, buf);
	if (!mrsp) {
		err("should have never happened");
		return -1;
	}
	delete_exist_ap_metrics_info(ctx, buf);

	pos += ETH_ALEN;
	check_len += ETH_ALEN;
	mrsp->ch_util = *pos++;
	check_len += 1;
	mrsp->assoc_sta_cnt = (((*pos)<<8)|(*(pos+1)));
	pos += 2;
	check_len += 2;
	/*skip esp indicator*/
	pos += 1;
	check_len += 1;
	mrsp->esp_cnt = 0;
	SLIST_INIT(&mrsp->esp_head);

	debug("insert struct mrsp_db");
	debug("bssid(%02x:%02x:%02x:%02x:%02x:%02x) ch_util=%d assoc_sta_cnt=%d",
			PRINT_MAC(mrsp->bssid), mrsp->ch_util, mrsp->assoc_sta_cnt);
	while(check_len < len) {
		esp = (struct esp_db *)os_zalloc(sizeof(struct esp_db));
		if (!esp) {
			err("alloc struct esp_db fail");
			return -1;
		}
		memset(esp, 0 , sizeof(struct esp_db));
		if (global->params.Certification) {
			esp->ac = (*(pos + 2)) & 0x03;
			esp->format = ((*(pos + 2)) & 0x18) >> 3;
			esp->ba_win_size = ((*(pos + 2)) & 0xe0) >> 5;
			esp->e_air_time_fraction = *(pos + 1);
			esp->ppdu_dur_target = *pos;
		} else {
			esp->ac = (*pos) & 0x03;
			esp->format = ( (*pos) & 0x18) >> 3;
			esp->ba_win_size = ((*pos) & 0xe0) >> 5;
			esp->e_air_time_fraction = *(pos + 1);
			esp->ppdu_dur_target = *(pos + 2);
		}
		SLIST_INSERT_HEAD(&mrsp->esp_head, esp, esp_entry);
		pos += 3;
		check_len += 3;
		mrsp->esp_cnt++;
		debug("insert struct esp_db");
		debug("ac=%d, format=%d ba_win_size=%d, e_air_time_fraction=%d, ",
				esp->ac, esp->format, esp->ba_win_size, esp->e_air_time_fraction);
		debug("ppdu_dur_target=%d esp_cnt=%d", esp->ppdu_dur_target, mrsp->esp_cnt);
	}
	topo_srv_update_channel_info(ctx, mrsp);
#ifdef MAP_R2
	debug("other_dev ");
	topo_srv_update_ch_planning_info(ctx,dev,mrsp,NULL,0);
#endif
#ifdef CENT_STR
	if(ctx->cent_str_en && ctx->device_role == DEVICE_ROLE_CONTROLLER)
		cent_str_cu_monitor(ctx,mrsp);
#endif
	return 0;

}

int insert_new_sta_metrics_info(
		struct own_1905_device * ctx, struct _1905_map_device *dev, unsigned char *buf, unsigned short len)
{
	unsigned char mac[ETH_ALEN] = {0};
	unsigned char bssid[ETH_ALEN] = {0};
	unsigned char *p = NULL;
	struct associated_clients *client = NULL;
#ifdef CENT_STR			
	struct client *cli = NULL;
#endif			
	p = buf;
	os_memcpy(mac, p, ETH_ALEN);
	p += ETH_ALEN;
	p++; //for bss cnt
	os_memcpy(bssid, p, ETH_ALEN);
	p += ETH_ALEN;
	SLIST_FOREACH(client, &dev->assoc_clients, next_client) {
		debug("metric bss mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(bssid));
		debug("metric cli mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(mac));
		debug("cli mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(client->client_addr));
		debug("bssid mac: %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(client->bss->bssid));
		if(!memcmp(client->client_addr, mac, ETH_ALEN) && !memcmp(client->bss->bssid, bssid, ETH_ALEN)) {
			client->time_delta = (((*p)<<24)|((*(p+1))<<16)|((*(p+2))<<8)|(*(p+3)));
			p+=4;
			client->erate_downlink = (((*p)<<24)|((*(p+1))<<16)|((*(p+2))<<8)|(*(p+3)));
			p+=4;
			client->erate_uplink = (((*p)<<24)|((*(p+1))<<16)|((*(p+2))<<8)|(*(p+3)));
			p+=4;
			client->rssi_uplink = rcpi_to_rssi(*p);
			p++;
#ifdef CENT_STR
			uint32_t cid = client_db_get_cid_from_mac((struct mapd_global *)ctx->back_ptr, client->client_addr);
			cli = client_db_get_client_from_sta_mac(ctx->back_ptr, client->client_addr);
			struct bss_info_db *bss = topo_srv_get_bss_by_bssid(ctx, dev, bssid);
			if(cli && bss && bss->radio){
			cli->dl_phy_rate = client->erate_uplink;
			client_db_set_ul_rssi((struct mapd_global *)ctx->back_ptr, cid, client->rssi_uplink, bss->radio->band, 0);
			if ((client->is_bh_link != 1) && link_metrics_mon_rcpi_at_controller(client, ctx)) {
				cent_str_select_on_demand_str_method(ctx,cli);
				
			}
			}
#endif			
			debug("time delta: %d", client->time_delta);
			debug("erate downlink: %u", client->erate_downlink);
			debug("erate uplink: %u", client->erate_uplink);
			debug("rssi uplink: %d", (char)client->rssi_uplink);
			break;
		}
	}

	return 0;

}
/**
* @brief Fn to add new tx link metrics info
*
* @param dev 1905 map device pointer
* @param buf msg buffer
* @param len msg len
*
* @return 0 if success else -1
*/
int insert_new_tx_link_metrics_info(
		struct _1905_map_device *dev, unsigned char *buf, unsigned short len)
{
	unsigned char *pos = buf;
	struct map_neighbor_info *neighbor_info;
	struct backhaul_link_info *bh_info = NULL;
	unsigned char *neighbor_dev = buf + ETH_ALEN;
	struct list_neighbor *neighbor_list;
	unsigned char found = 0;
	int bh_cnt = 0, i = 0;

	neighbor_list = &dev->neighbors_entry;


	SLIST_FOREACH(neighbor_info, neighbor_list, next_neighbor)
	{
		if(!os_memcmp(neighbor_info->n_almac, neighbor_dev, ETH_ALEN)) {
			info("neighbor dev(%02x:%02x:%02x:%02x:%02x:%02x) found",
					PRINT_MAC(neighbor_dev));
			break;
		}
	}

	if (!neighbor_info) {
		/* Discard this info, will be filled in next infra metrics */
		err("topo server doesn't have (%02x:%02x:%02x:%02x:%02x:%02x) with neighbor(%02x:%02x:%02x:%02x:%02x:%02x)",
					PRINT_MAC(dev->_1905_info.al_mac_addr) ,PRINT_MAC(neighbor_dev));
		return -1;
	} else {
		pos += 2 * ETH_ALEN;
	}
	if ((len - 12) % 29 != 0) {
		err("tlv length error");
		return -1;
	} else
		bh_cnt = (len - 12) / 29;

	for (i = 0; i < bh_cnt; i++) {
		found = 0;
		SLIST_FOREACH(bh_info, &neighbor_info->bh_head, next_bh) {
			if (os_memcmp(bh_info->connected_iface_addr, pos, ETH_ALEN) == 0&&
				os_memcmp(bh_info->neighbor_iface_addr, pos + ETH_ALEN, ETH_ALEN) == 0) {
				info("found connected_iface_addr=%02x:%02x:%02x:%02x:%02x:%02x , neighbor_iface_addr %02x:%02x:%02x:%02x:%02x:%02x",
				PRINT_MAC(bh_info->connected_iface_addr), PRINT_MAC(bh_info->neighbor_iface_addr));
				found = 1;
				break;
			}
		}
		if (found == 0) {
			/* Discard this info, will be filled in next infra metrics */
			err("topo server doesn't have (%02x:%02x:%02x:%02x:%02x:%02x) with neighbor(%02x:%02x:%02x:%02x:%02x:%02x)",
					PRINT_MAC(dev->_1905_info.al_mac_addr) ,PRINT_MAC(neighbor_dev));
			err("connected(%02x:%02x:%02x:%02x:%02x:%02x) n_iface(%02x:%02x:%02x:%02x:%02x:%02x)",
					PRINT_MAC(pos), PRINT_MAC((pos + ETH_ALEN)));
			return -1;
		}

		memcpy(bh_info->connected_iface_addr, pos, ETH_ALEN);
		pos += ETH_ALEN;
		memcpy(bh_info->neighbor_iface_addr, pos, ETH_ALEN);
		pos += ETH_ALEN;
		bh_info->tx.iface_type = ((*pos) << 8) | (*(pos + 1));
		pos += 2;
		bh_info->tx.is_80211_bridge = *pos;
		pos += 1;
		bh_info->tx.pkt_err = ((*pos) << 24) | ((*(pos + 1)) << 16) |
			((*(pos + 2)) << 8) | (*(pos + 3));
		pos += 4;
		bh_info->tx.tx_packet = ((*pos) << 24) | ((*(pos + 1)) << 16) |
			((*(pos + 2)) << 8) | (*(pos + 3));
		pos += 4;
		bh_info->tx.mac_throughput = ((*pos) << 8) | (*(pos + 1));
		pos += 2;
		bh_info->tx.link_availability = ((*pos) << 8) | (*(pos + 1));
		pos += 2;
		bh_info->tx.phy_rate = ((*pos) << 8) | (*(pos + 1));
		pos += 2;
		os_get_time(&bh_info->last_update);

		info("insert struct tx_metric_db");
		info("mac(%02x:%02x:%02x:%02x:%02x:%02x) ne mac(%02x:%02x:%02x:%02x:%02x:%02x)",
				PRINT_MAC(bh_info->connected_iface_addr),
				PRINT_MAC(bh_info->neighbor_iface_addr));
		info("intf_type=%d, bridge_flag=%d error_packet=%d, ",
				bh_info->tx.iface_type, bh_info->tx.is_80211_bridge, bh_info->tx.pkt_err);
		info("tx_packets=%d, mac_tpcap=%d linkavl=%d, phyrate=%d",
				bh_info->tx.tx_packet, bh_info->tx.mac_throughput, bh_info->tx.link_availability, bh_info->tx.phy_rate);
	}

	return 0;
}

/**
* @brief Fn to parse basic radio cap tlv
*
* @param buf msg buffer
* @param dev 1905 map device ptr
* @param bcap rdio basic cap
*
* @return -1 if error else tlv lenght
*/
int parse_basic_radio_cap_tlv(unsigned char *buf, struct _1905_map_device *dev, struct ap_radio_basic_cap *bcap)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	struct radio_basic_cap *opcap;
	int i;

	temp_buf = buf;

	if((*temp_buf) == AP_RADIO_BASIC_CAPABILITY_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;
	memcpy(bcap->identifier, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	bcap->max_bss_num = *temp_buf;
	temp_buf++;
	bcap->op_class_num = *temp_buf;
	temp_buf++;
	for (i = 0; i < bcap->op_class_num; i++) {
		opcap = &bcap->opcap[i];
		opcap->op_class = *temp_buf;
		temp_buf++;
		opcap->max_tx_pwr = *temp_buf;
		temp_buf++;
		opcap->non_operch_num = *temp_buf;
		temp_buf++;
		memcpy(opcap->non_operch_list, temp_buf, opcap->non_operch_num);
		temp_buf += opcap->non_operch_num;
	}

	return (length+3);

}

/**
* @brief Fn to parse ap cap tlv
*
* @param buf msg buffer
* @param dev 1905 map device pointer
* @param cap ap capability
*
* @return -1 if error else tlv lenght
*/
int parse_ap_cap_tlv(unsigned char *buf, struct _1905_map_device *dev, struct ap_capability *cap)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	unsigned char val;

	temp_buf = buf;

	if((*temp_buf) == AP_CAPABILITY_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	val = *temp_buf;

	cap->sta_report_on_cop = val & 0x20;
	cap->sta_report_not_cop = val & 0x40;
	cap->rssi_steer = val & 0x80;

	return (length+3);
}

/**
* @brief Fn to parse ap ht caps tlv
*
* @param buf msg buffer
* @param cap ap ht caps tlv
*
* @return -1 if error else tlv lenght
*/
int parse_ap_ht_cap_tlv(unsigned char *buf, struct ap_ht_capability *cap)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	unsigned char val;

	temp_buf = buf;

	if((*temp_buf) == AP_HT_CAPABILITY_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	memcpy(cap->identifier, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	val = *temp_buf;

	cap->ht_40 = (val & 0x02) >> 1;
	cap->sgi_40 = (val & 0x04) >>2;
	cap->sgi_20 = (val & 0x08) >>3;
	cap->rx_stream = (val & 0x30)>>4;
	cap->tx_stream = (val & 0xC0)>>6;

	return (length+3);
}

/**
* @brief Fn to parse ap vht caps tlbv
*
* @param buf msg buffer
* @param cap ap vht caps
*
* @return -1 if error else tlv length
*/
int parse_ap_vht_cap_tlv(unsigned char *buf, struct ap_vht_capability *cap)
{
	unsigned char *temp_buf, val;
	unsigned short length = 0, mcs;

	temp_buf = buf;

	if((*temp_buf) == AP_VHT_CAPABILITY_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	temp_buf += 2;

	memcpy(cap->identifier, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	mcs = (*temp_buf);
	mcs = (mcs << 8) & 0xFF00;
	mcs = mcs |(*(temp_buf+1));

	temp_buf += 2;
	cap->vht_tx_mcs = mcs;

	mcs = (*temp_buf);
	mcs = (mcs << 8) & 0xFF00;
	mcs = mcs |(*(temp_buf+1));

	temp_buf += 2;
	cap->vht_rx_mcs = mcs;

	val = *temp_buf;
	temp_buf += 1;
	cap->tx_stream = (val & 0xE0) >> 5;
	cap->rx_stream = (val & 0x1C) >> 2;
	cap->sgi_80 = val & 0x02 >> 1;
	cap->sgi_160 = val & 0x01;

	val = *temp_buf;
	cap->vht_8080 = 0x80 >> 7;
	cap->vht_160 = 0x40 >> 6;
	cap->su_beamformer = 0x20 >> 5;
	cap->mu_beamformer = 0x10 >> 4;
	//TODO band
	//cap->band =

	return (length+3);
}

/**
* @brief Fn to parse ap he caps tlv
*
* @param buf msg buffer
* @param bcap ap he caps
*
* @return -1 if error else tlv length
*/
int parse_ap_he_cap_tlv(unsigned char *buf, struct ap_he_capability *bcap)
{
	unsigned char *temp_buf;
	unsigned short length, val = 0;

	temp_buf = buf;

	if((*temp_buf) == AP_HE_CAPABILITY_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	memcpy(bcap->identifier, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	//MCS Length
	bcap->he_mcs_len = *temp_buf;
	temp_buf++;

	//Supported MCS
	memcpy(bcap->he_mcs, temp_buf, bcap->he_mcs_len);
	temp_buf += bcap->he_mcs_len;

	//Spatial streams and BW support
	val = *temp_buf;
	bcap->tx_stream = (val & 0xE0) >> 5;
	bcap->rx_stream = (val & 0x1C) >> 2;
	bcap->he_8080 = (val & 0x02) >> 1;
	bcap->he_160 = val & 0x01;
	temp_buf++;

	val = *temp_buf;
	bcap->su_bf_cap = (val & 0x80) >> 7;
	bcap->mu_bf_cap= (val & 0x40) >> 6;
	bcap->ul_mu_mimo_cap = (val & 0x20) >> 5;
	bcap->ul_mu_mimo_ofdma_cap = (val & 0x10) >> 4;
	bcap->dl_mu_mimo_ofdma_cap = (val & 0x08) >> 3;
	bcap->ul_ofdma_cap = (val & 0x04) >> 2;
	bcap->dl_ofdma_cap = (val & 0x02) >> 1;
	return (length+3);
}
#ifdef MAP_R2
int parse_ap_cac_cap_tlv(
	unsigned char *buf,
	struct _1905_map_device *dev)
{
	struct cac_capability cac_cap;
	struct cac_cap_tlv cap;
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;
	int i,j,k,l;
	struct radio_info_db *radio = NULL;

	temp_buf = buf;

	if((*temp_buf) == CAC_CAPABILITIES_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	cac_cap.country_code[0] =  *temp_buf;
	temp_buf ++;
	cac_cap.country_code[1] = *temp_buf;
	temp_buf++;
	cac_cap.radio_num = *temp_buf;
	temp_buf++;

	for (i = 0; i < cac_cap.radio_num; i++) {
		os_memcpy(cap.identifier, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;
		cap.cac_type_num = *temp_buf;
		temp_buf++;
		radio = topo_srv_get_radio(dev, cap.identifier);
		if(!radio) {
			err("dev radio not found some error");
			return -1;
		}
		os_memcpy(radio->cac_cap.identifier, cap.identifier, ETH_ALEN);
		radio->cac_cap.cac_type_num = cap.cac_type_num;
		topo_srv_clear_cac_cap_db(radio);
		SLIST_INIT(&radio->cac_cap.cac_capab_head);
		for(j = 0 ; j < radio->cac_cap.cac_type_num ; j++)
		{
			struct cac_cap_db *cac = os_zalloc(sizeof(struct cac_cap_db));
			if(!cac) {
				err("mem alloc fail");
				return -1;
			}
			SLIST_INSERT_HEAD(&radio->cac_cap.cac_capab_head, cac, cac_cap_entry);
			SLIST_INIT(&cac->cac_opcap_head);
			cac->cac_mode = *temp_buf;
			err("CAC Mode: %d", cac->cac_mode);
			temp_buf++;
			cac->cac_interval[0] = *temp_buf;
			temp_buf++;
			cac->cac_interval[1] = *temp_buf;
			temp_buf++;
			cac->cac_interval[2] = *temp_buf;
			temp_buf++;
			cac->op_class_num = *temp_buf;
			temp_buf++;
			for(k = 0 ; k < cac->op_class_num ; k++) {
				struct cac_opcap_db *opcap = os_zalloc(sizeof(struct cac_opcap_db));
				if(!opcap) {
					err("mem alloc fail");
					os_free(cac);
					return -1;
				}
				SLIST_INSERT_HEAD(&cac->cac_opcap_head, opcap, cac_opcap_entry);
				opcap->op_class = *temp_buf;
				temp_buf++;
				opcap->ch_num = *temp_buf;
				temp_buf++;
				for (l = 0; l < opcap->ch_num; l++) {
					opcap->ch_list[l] = *temp_buf;
					temp_buf++;
				}
			}
		}
	}
	return (length+3);
}

int parse_ap_scan_cap_tlv(
	unsigned char *buf,
	struct _1905_map_device *dev,
	struct channel_scan_capab *ch_scan_cap)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	struct channel_body *ch_body;
	int i,j,k;

	temp_buf = buf;

	if((*temp_buf) == CHANNEL_SCAN_CAPABILITY_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;
	ch_scan_cap->radio_num = *temp_buf;
	temp_buf++;
	for (i = 0; i < ch_scan_cap->radio_num; i++) {
		os_memcpy(ch_scan_cap->radio_scan_params[i].radio_id, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;

		ch_scan_cap->radio_scan_params[i].boot_scan_only= (*temp_buf)& 0x80;//bit 7
		ch_scan_cap->radio_scan_params[i].scan_impact= *temp_buf & 0x60; //bit 5 and 6
		temp_buf++;

		ch_scan_cap->radio_scan_params[i].min_scan_interval = (u32)*temp_buf;
		temp_buf += 4;

		ch_scan_cap->radio_scan_params[i].oper_class_num = *temp_buf;
		temp_buf++;

		for (j = 0; j < ch_scan_cap->radio_scan_params[i].oper_class_num; j++) {
			ch_body = &ch_scan_cap->radio_scan_params[i].ch_body[j];
			ch_body->oper_class = *temp_buf;
			temp_buf++;
			ch_body->ch_list_num = *temp_buf;
			temp_buf++;
			for (k = 0; k < ch_body->ch_list_num; k++) {
				ch_body->ch_list[k] = *temp_buf;
				temp_buf++;
			}
		}
	}

	return (length+3);

}
#endif
int parse_cac_completion_tlv(struct own_1905_device *ctx,
	unsigned char *buf, struct _1905_map_device *dev)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	struct radio_info_db *radio = NULL;
	struct cac_completion_report cac_rep;
	u8 i, j;
	temp_buf = buf;
#if 0
	struct os_reltime rem_time = {0};
#endif

	if ((*temp_buf) == CAC_COMPLETION_REPORT_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	//shift to tlv value field
	temp_buf += 2;
	mapd_hexdump(MSG_OFF, "MAPD parse_cac_completion_tlv", temp_buf, length);
	cac_rep.radio_num = *temp_buf;
	temp_buf ++;
	for(i=0; i < cac_rep.radio_num; i++) {
		radio = topo_srv_get_radio(dev, temp_buf);
		if(radio)
		{
			os_memcpy(radio->cac_comp_status.identifier, temp_buf, ETH_ALEN);
			temp_buf+=ETH_ALEN;
			radio->cac_comp_status.op_class = *temp_buf;
			temp_buf++;
			radio->cac_comp_status.channel = *temp_buf;
			temp_buf++;
			radio->cac_comp_status.cac_status = *temp_buf;
			temp_buf++;
			radio->cac_comp_status.op_class_num = *temp_buf;
			temp_buf++;
			topo_srv_clear_cac_completion_status(radio);
			struct cac_completion_opcap_db *opcap;
			SLIST_INIT(&radio->cac_comp_status.cac_completion_opcap_head);
			for (j = 0; j < radio->cac_comp_status.op_class_num; j++)
			{
				opcap = os_zalloc(sizeof(struct cac_completion_opcap_db));
				if(!opcap) {
					err("mem alloc fail");
					return -1;
				}
				SLIST_INSERT_HEAD(&radio->cac_comp_status.cac_completion_opcap_head,opcap,opcap_db_next);
				opcap->op_class = *temp_buf;
				temp_buf++;
				opcap->ch_num = *temp_buf;
				temp_buf++;
			}
			if (radio->cac_comp_status.cac_status == CAC_SUCCESSFUL && ctx->network_optimization.network_optimization_enabled) {
				radio->cac_channel = 0;
				radio->cac_enable = 0;
#if 0
				eloop_get_remaining_timeout(&rem_time,trigger_net_opt,ctx,NULL);
				if (rem_time.sec < ctx->network_optimization.post_cac_trigger_time) {
					eloop_cancel_timeout(trigger_net_opt,ctx,NULL);
					network_opt_reset(ctx->back_ptr);
					eloop_register_timeout(ctx->network_optimization.post_cac_trigger_time, 0, trigger_net_opt, ctx, NULL);
				}
#endif
			}
#ifdef MAP_R2
			ch_planning_handle_cac_response2(ctx, dev, radio);
#endif
		}
	}
	return (length + 3);
}
#ifdef MAP_R2
int parse_metric_collection_intv_tlv(unsigned char *buf, struct _1905_map_device *dev)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	temp_buf = buf;

	if ((*temp_buf) == METRIC_COLLECTION_INTERVAL_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	//shift to tlv value field
	temp_buf += 2;
	dev->metric_rep_interval = ((*temp_buf) << 24) | ((*(temp_buf + 1)) << 16) |
			((*(temp_buf + 2)) << 8) | (*(temp_buf + 3));
	temp_buf += 4;
	return (length + 3);
}
int parse_r2_cap_tlv(unsigned char *buf, struct _1905_map_device *dev)
{
	unsigned char *temp_buf;
	unsigned short length = 0;
	temp_buf = buf;

	if ((*temp_buf) == R2_AP_CAPABILITY_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	//shift to tlv value field
	temp_buf += 2;
	temp_buf += 2; // reserved field
	mapd_hexdump(MSG_OFF,"R2 AP cap", temp_buf, length);
	if (*temp_buf & 0x80)
		dev->byte_cnt_unit = 2;
	else if (*temp_buf & 0x40)
		dev->byte_cnt_unit = 1;
	err("Byte cnt unit: %d", dev->byte_cnt_unit);
	return (length + 3);
}

#endif

/**
* @brief Fn to add new traffic stats
*
* @param ctx own 1905 device ctx
* @param traffic_stats sta_traffic_stats to be added
*
* @return 0 if success else -1
*/

int insert_new_traffic_stats_tlv(struct own_1905_device *ctx, struct stat_info *traffic_stats, struct _1905_map_device *dev)
{
	struct associated_clients *cli = NULL;
	struct client *sta = client_db_get_client_from_sta_mac((struct mapd_global *)ctx->back_ptr,traffic_stats->mac);
	struct os_time now, sub;
	unsigned int diff_ul = 0, diff_dl = 0, ul_rate = 0, dl_rate = 0;
	int found = 0;
	long int bytes_check = 0;

	os_get_time(&now);
	SLIST_FOREACH(cli, &dev->assoc_clients, next_client) {
		if (os_memcmp(cli->client_addr, traffic_stats->mac, ETH_ALEN) == 0) {
			found = 1;
			break;
		}
	}
	if (found == 1) {
		cli->stat_db.bytes_sent = traffic_stats->bytes_sent;
		cli->stat_db.bytes_received = traffic_stats->bytes_received;
		cli->stat_db.packets_sent = traffic_stats->packets_sent;
		cli->stat_db.packets_received = traffic_stats->packets_received;
		cli->stat_db.tx_packets_errors = traffic_stats->tx_packets_errors;
		cli->stat_db.rx_packets_errors = traffic_stats->rx_packets_errors;
		cli->stat_db.retransmission_count = traffic_stats->retransmission_count;
		if (cli->stat_db.last_bytes_recieved > cli->stat_db.bytes_received)
			diff_ul = cli->stat_db.bytes_received + 0xFFFFFFFF - cli->stat_db.last_bytes_recieved + 1;
		else
			diff_ul = cli->stat_db.bytes_received - cli->stat_db.last_bytes_recieved;
		if (cli->stat_db.last_bytes_sent > cli->stat_db.bytes_sent)
			diff_ul = cli->stat_db.bytes_sent + 0xFFFFFFFF - cli->stat_db.last_bytes_sent + 1;
		else
			diff_dl = cli->stat_db.bytes_sent - cli->stat_db.last_bytes_sent;
		os_time_sub(&now, &(cli->stat_db.last_traffic_stats_time), &sub);
		if (sub.sec > 0) {
			dl_rate = diff_dl/sub.sec;
			ul_rate = diff_ul/sub.sec;
		}
#ifdef MAP_R2
		if (ctx->map_version == DEV_TYPE_R2 && dev->map_version == DEV_TYPE_R2) {
			if (dev->byte_cnt_unit == 2) {
				cli->stat_db.dl_rate = dl_rate << 3;
				cli->stat_db.ul_rate = ul_rate << 3;
				bytes_check = (dl_rate + ul_rate) * 1024 *1024;
			} else if (dev->byte_cnt_unit == 1) {
				cli->stat_db.dl_rate = dl_rate >> 7;
				cli->stat_db.ul_rate = ul_rate >> 7;
				bytes_check = (dl_rate + ul_rate) * 1024;
			} else {
				cli->stat_db.dl_rate = dl_rate >> 17;
				cli->stat_db.ul_rate = ul_rate >> 17;
				bytes_check = dl_rate + ul_rate;
			}
		} else {
#endif
		cli->stat_db.dl_rate = dl_rate >> 17;
		cli->stat_db.ul_rate = ul_rate >> 17;
		bytes_check = dl_rate + ul_rate;
#ifdef MAP_R2
		}
#endif
		cli->stat_db.last_bytes_recieved = cli->stat_db.bytes_received;
		cli->stat_db.last_bytes_sent = cli->stat_db.bytes_sent;
		cli->stat_db.last_traffic_stats_time = now;
	}

	if (sta && found == 1) {
		sta->ul_rate = cli->stat_db.ul_rate;
		sta->dl_rate = cli->stat_db.dl_rate;
		if (bytes_check > (ctx->cli_steer_params.ActivityThreshold))
			sta->curr_activity_state = 1;
		else
			sta->curr_activity_state = 0;
	}

	return 0;
}

int parse_assoc_sta_traffic_stats_tlv(struct own_1905_device * ctx, struct _1905_map_device *dev, unsigned char *buf)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == ASSOC_STA_TRAFFIC_STATS_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}
	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	if (0 > insert_new_traffic_stats_tlv(ctx,(struct stat_info *) temp_buf, dev)) {
		debug("insert_new_sta_traffic_stats fail");
		return -1;
	}
	return (length+3);
}

int parse_assoc_sta_link_metrics_tlv(struct own_1905_device * ctx, struct _1905_map_device *dev, unsigned char *buf)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == ASSOC_STA_LINK_METRICS_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}
	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;

	if (0 > insert_new_sta_metrics_info(ctx, dev, temp_buf, length)) {
		debug("insert_new_ap_metrics_info fail");
		return -1;
	}

	return (length+3);
}
/**
* @brief Fn to parse ap metrics tlv
*
* @param ctx own 1905 device ctx
* @param dev 1905 map device pointer
* @param buf msg buffer
*
* @return -1 if error else tlv length
*/
int parse_ap_metrics_tlv(struct own_1905_device * ctx, struct _1905_map_device *dev, unsigned char *buf)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == AP_METRICS_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	//shift to tlv value field
	temp_buf += 2;
	/*insert new channel preference info*/
	if (0 > insert_new_ap_metrics_info(ctx, dev, temp_buf, length)) {
		debug("insert_new_ap_metrics_info fail");
		return -1;
	}

	return (length+3);
}

/**
* @brief Fn to parse rx link metrics tlv
*
* @param buf msg buffer
* @param dev 1905 map device pointer
*
* @return -1 if error else tlv length
*/
int parse_tx_link_metrics_tlv(unsigned char *buf,
		struct _1905_map_device *dev)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if ((*temp_buf) == TRANSMITTER_LINK_METRIC_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	//shift to tlv value field
	temp_buf += 2;

	/*insert new tx link info */
	if (0 > insert_new_tx_link_metrics_info(dev, temp_buf,
				length)) {
		debug("insert_new_tx_link_metrics_info fail");
		return -1;
	}

	return (length + 3);
}

/**
* @brief Fn to insert rx link metrics query in ctx
*
* @param dev 1905 map device
* @param buf msg buffer
* @param len msg length
*
* @return 0 if success else -1
*/
int insert_new_rx_link_metrics_info(
		struct _1905_map_device *dev, unsigned char *buf, unsigned short len)
{
	unsigned char *pos = buf;
	struct map_neighbor_info *neighbor_info;
	struct backhaul_link_info *bh_info = NULL;
	unsigned char *neighbor_dev = buf + 6;
	struct list_neighbor *neighbor_list;
	unsigned char found = 0;
	int bh_cnt = 0, i = 0;

	neighbor_list = &dev->neighbors_entry;

	SLIST_FOREACH(neighbor_info, neighbor_list, next_neighbor)
	{
		if(!os_memcmp(neighbor_info->n_almac, neighbor_dev, ETH_ALEN)) {
			debug("neighbor dev(%02x:%02x:%02x:%02x:%02x:%02x) found",
					PRINT_MAC(neighbor_dev));
			break;
		}
	}

	if (!neighbor_info) {
		/* Discard this info, will be filled in next infra metrics */
		err("topo server doesn't have (%02x:%02x:%02x:%02x:%02x:%02x) with neighbor(%02x:%02x:%02x:%02x:%02x:%02x)",
					PRINT_MAC(dev->_1905_info.al_mac_addr) ,PRINT_MAC(neighbor_dev));
		return -1;
	} else {
		pos += 2 * ETH_ALEN;
	}
	if ((len - 12) % 23 != 0) {
		err("tlv length error");
		return -1;
	} else
		bh_cnt = (len - 12) / 23;

	for (i = 0; i < bh_cnt; i++) {
		found = 0;
		SLIST_FOREACH(bh_info, &neighbor_info->bh_head, next_bh) {
			if (os_memcmp(bh_info->connected_iface_addr, pos, ETH_ALEN) == 0&&
				os_memcmp(bh_info->neighbor_iface_addr, pos + ETH_ALEN, ETH_ALEN) == 0) {
				found = 1;
				break;
			}
		}
		if (found == 0) {
			/* Discard this info, will be filled in next infra metrics */
			err("topo server doesn't have (%02x:%02x:%02x:%02x:%02x:%02x) with neighbor(%02x:%02x:%02x:%02x:%02x:%02x)",
					PRINT_MAC(dev->_1905_info.al_mac_addr) ,PRINT_MAC(neighbor_dev));
			err("connected(%02x:%02x:%02x:%02x:%02x:%02x) n_iface(%02x:%02x:%02x:%02x:%02x:%02x)",
					PRINT_MAC(pos), PRINT_MAC((pos + ETH_ALEN)));
			return -1;
		}

		memcpy(bh_info->connected_iface_addr, pos, ETH_ALEN);
		pos += ETH_ALEN;
		memcpy(bh_info->neighbor_iface_addr, pos, ETH_ALEN);
		pos += ETH_ALEN;

		bh_info->rx.iface_type = ((*pos) << 8) | (*(pos + 1));
		pos += 2;
		bh_info->rx.pkt_err = ((*pos) << 24) | ((*(pos + 1)) << 16) |
			((*(pos + 2)) << 8) | (*(pos + 3));
		pos += 4;
		bh_info->rx.pkt_received = ((*pos) << 24) | ((*(pos + 1)) << 16) |
			((*(pos + 2)) << 8) | (*(pos + 3));
		pos += 4;
		bh_info->rx.rssi = rcpi_to_rssi(*pos);
		pos += 1;
		os_get_time(&bh_info->last_update);
		debug("insert struct rx_metric_db");
		debug("mac(%02x:%02x:%02x:%02x:%02x:%02x)",
				PRINT_MAC(bh_info->connected_iface_addr));
		debug("connect mac(%02x:%02x:%02x:%02x:%02x:%02x)",
				PRINT_MAC(bh_info->neighbor_iface_addr));
		debug("intf_type=%d, error_packet=%d, ",
				bh_info->rx.iface_type, bh_info->rx.pkt_err);
		debug("rx_packets=%d, rssi=%d",
				bh_info->rx.pkt_received, bh_info->rx.rssi);
	}


	return 0;
}


/**
* @brief Fn to parse rx link metrics tlv
*
* @param buf msg buffer
* @param dev 1905 map device
*
* @return -1 if error else tlv length
*/
int parse_rx_link_metrics_tlv(unsigned char *buf, struct _1905_map_device *dev)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if ((*temp_buf) == RECEIVER_LINK_METRIC_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	//shift to tlv value field
	temp_buf += 2;

	/*insert new channel preference info */
	if (0 > insert_new_rx_link_metrics_info(dev, temp_buf,
				length)) {
		err("insert_new_rx_link_metrics_info fail");
		return -1;
	}

	return (length + 3);
}

/**
* @brief Fn to parse beacon metrics query tlv
*
* @param buf msg buffer
* @param beacon beacon_metrics_query struct to be filled
*
* @return tlv lenght if success else -1
*/
int parse_beacon_metrics_query_tlv(unsigned char *buf,
		struct beacon_metrics_query **beacon)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0, query_len = 0;
	struct beacon_metrics_query *bcn_query = NULL;
	unsigned char bssid[ETH_ALEN] = { 0 };
	unsigned char sta_mac[ETH_ALEN] = { 0 };
	unsigned char ssid[33] = { 0 };
	unsigned char opclass = 0, ch = 0, rpt_detail = 0;
	unsigned char ssid_len = 0, num_chrep = 0;
	unsigned char i = 0;
	struct ap_chn_rpt *chn_rpt = NULL;

	temp_buf = buf;

	if ((*temp_buf) == BEACON_METRICS_QUERY_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	//shift to tlv value field
	temp_buf += 2;

	memcpy(sta_mac, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	/*opclass */
	opclass = *temp_buf++;
	/*channel number */
	ch = *temp_buf++;
	/*bssid */
	memcpy(bssid, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	/*Reporting Detail value */
	rpt_detail = *temp_buf++;
	/*ssid len & ssid */
	ssid_len = *temp_buf++;
	memcpy(ssid, temp_buf, ssid_len);
	temp_buf += ssid_len;
	/*Number of AP Channel Reports */
	num_chrep = *temp_buf++;

	query_len = sizeof(struct beacon_metrics_query) +
		num_chrep * sizeof(struct ap_chn_rpt);
	debug("query_len=%d", query_len);
	bcn_query = (struct beacon_metrics_query *)os_malloc(query_len);
	if (!bcn_query) {
		err("alloc struct beacon_metrics_query fail");
		return -1;
	}

	memset(bcn_query, 0, query_len);
	memcpy(bcn_query->sta_mac, sta_mac, ETH_ALEN);
	bcn_query->oper_class = opclass;
	bcn_query->ch = ch;
	memcpy(bcn_query->bssid, bssid, ETH_ALEN);
	bcn_query->rpt_detail_val = rpt_detail;
	bcn_query->ssid_len = ssid_len;
	memcpy(bcn_query->ssid, ssid, ssid_len);
	bcn_query->ap_ch_rpt_num = num_chrep;
	chn_rpt = bcn_query->rpt;
	for (i = 0; i < num_chrep; i++) {
		/*ap channel report info */
		chn_rpt->ch_rpt_len = *temp_buf++;
		chn_rpt->oper_class = *temp_buf++;
		memcpy(chn_rpt->ch_list, temp_buf, chn_rpt->ch_rpt_len - 1);
		temp_buf += chn_rpt->ch_rpt_len - 1;
		chn_rpt++;
	}
	/*element IDs */
	bcn_query->elemnt_num = *temp_buf++;
	if (bcn_query->elemnt_num > MAX_ELEMNT_NUM)
		bcn_query->elemnt_num = MAX_ELEMNT_NUM;
	memcpy(bcn_query->elemnt_list, temp_buf, bcn_query->elemnt_num);

	*beacon = bcn_query;

	return (length + 3);
}

/**
* @brief Fn to parse unassoc sta link metrics query tlv
*
* @param buf msg buff
* @param unlink unlink_metrics_query to be filled
*
* @return tlv length if success else -1
*/
int parse_unassociated_sta_link_metrics_query_tlv(unsigned char *buf, struct unlink_metrics_query
		*unlink)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0, query_len = 0;
	struct unlink_metrics_query *unlink_query = NULL;
	unsigned char opclass = 0, num_sta = 0, num_ch = 0;
	unsigned char channels[MAX_CH_NUM] = { 0 };
	unlink_query = unlink;

	temp_buf = buf;

	if ((*temp_buf) == UNASSOC_STA_LINK_METRICS_QUERY_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	//shift to tlv value field
	temp_buf += 2;

	opclass = *temp_buf++;
	num_ch = *temp_buf++;
	memcpy(channels, temp_buf, num_ch);
	temp_buf += num_ch;
	num_sta = *temp_buf++;

	query_len = sizeof(struct unlink_metrics_query) + num_sta * ETH_ALEN;
	debug("query_len=%d", query_len);
	//unlink_query = (struct unlink_metrics_query *)os_zalloc(sizeof(query_len));
	if (!unlink_query) {
		err(" alloc struct unlink_metrics_query fail");
		return -1;
	}

	unlink_query->oper_class = opclass;
	unlink_query->ch_num = num_ch;
	if (unlink_query->ch_num == 0 || num_ch > 13) {
		err("invalid ch num(0)");
		return -1;
	}
	memcpy(unlink_query->ch_list, channels, num_ch);
	unlink_query->sta_num = num_sta;
	if (unlink_query->sta_num == 0) {
		err("invalid sta num(0)");
		return -1;
	}
	memcpy(unlink_query->sta_list, temp_buf,
			unlink_query->sta_num * ETH_ALEN);

	//*unlink = unlink_query;

	return (length + 3);
}

/**
* @brief Fn to parse device info tlv
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
* @param dev 1905 map device ptr
*
* @return -1 if error else tlv length
*/
int parse_device_info_type_tlv(struct own_1905_device *ctx, unsigned char *buf,
		struct _1905_map_device *dev)
{
	unsigned char *temp_buf;
	int length = 0;
	unsigned char itfs_num = 0;
	unsigned char vs_info_len = 0;
	int i = 0;
	unsigned char mac[ETH_ALEN];
	unsigned char topo_bss_num = 0;
	char tmp = 0;

	temp_buf = buf;

	if ((*temp_buf) == DEVICE_INFO_TLV_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));

	//shift to tlv value field
	temp_buf += 2;

	//skip AL MAC address field
	memcpy(dev->_1905_info.al_mac_addr, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	//get the amount of local interface
	itfs_num = *temp_buf;
	temp_buf += 1;
	struct iface_info *iface;

	for (i = 0; i < itfs_num; i++) {
		iface = topo_srv_get_iface(dev, temp_buf);
		memcpy(mac, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;

		if (!iface) {
			iface = (struct iface_info *)os_zalloc(sizeof(struct iface_info));
			if (!iface) {
				assert(0);
				return -1;
			}
			iface->p1905_device = dev;
			iface->radio = NULL;
			SLIST_INSERT_HEAD(&dev->_1905_info.first_iface,
					iface, next_iface);
		}

		memcpy(iface->iface_addr, mac, ETH_ALEN);

		iface->media_type = (*temp_buf);
		iface->media_type = (iface->media_type << 8) & 0xFF00;
		iface->media_type |= (*(temp_buf + 1));
		iface->valid = TRUE;
		temp_buf += 2;

		vs_info_len = (*temp_buf);
		temp_buf += 1;
#ifdef STOP_BEACON_FEATURE
		if ((iface->media_type < IEEE802_11_GROUP) &&
			(ctx->beacon_enable == 1)){
				system("wappctrl ra0 map set_beacon_state en");
		}
#endif
		debug("vs_info_len = %d",vs_info_len);
		if (vs_info_len > 0) {
			if (vs_info_len == 10) {
				memcpy(&iface->media_info, temp_buf, vs_info_len);
				if (*((char *)(&iface->media_info) + 6) == 0x00) {
					topo_bss_num++;
				}

				if (*((char *)(&iface->media_info) + 6) == 0x40)
				{
					//TODO count station interface
				}
				//! iface bandwidth
				tmp =  *(char *)((char *)(&iface->media_info) + 7);
				iface->media_info.ap_channel_band = (u8)tmp;
				if (iface->media_info.ap_channel_band == 0x2)
				{
					//! it is a 80Mhz iface, assume primary channel as central 
					// channel - 6
					iface->channel_freq_idx = 
						(*((char *)(&iface->media_info) + 8)) - 6;
				} else {
					//! it is a 40/20Mhz channel, assume primary channel as 
					// central freq
					iface->channel_freq_idx = 
						*((char *)(&iface->media_info) + 8);
				}
			}
		}
		temp_buf += vs_info_len;
	}

	topo_srv_remove_all_invalid_iface(ctx, dev);
	return (length + 3);
}

/**
* @brief Fn to parse almac type tlv
*
* @param buf msg buffer
* @param al_mac almac to be filled
*
* @return -1 if error else tlv length
*/
int parse_al_mac_addr_type_tlv(unsigned char *buf,unsigned char *al_mac)
{
	unsigned char *temp_buf;
	temp_buf = buf;

	if((*temp_buf) == AL_MAC_ADDR_TLV_TYPE)
		temp_buf++;
	else
	{
		return -1;
	}

	if(*temp_buf == 0x0 && *(temp_buf+1) == 0x6)
		temp_buf +=2;
	else
	{
		return -1;
	}

	memcpy(al_mac,temp_buf,ETH_ALEN);
#define TOTAL_AL_MAC_ADDR_TLV_LENGTH 9
	return TOTAL_AL_MAC_ADDR_TLV_LENGTH;
}

/**
* @brief Fn to parse client assoc event tlv
*
* @param buf msg buffer
* @param evt topo_notification to be filled
*
* @return -1 if error else tlv length
*/
int parse_client_assoc_event_tlv(struct own_1905_device *ctx, unsigned char *buf, struct topo_notification *evt)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;
	unsigned char *pos =NULL;
	struct client_assoc *assoc;
	struct bss_info_db *bss_info = NULL;
	struct mapd_global *global = ctx->back_ptr;
	temp_buf = buf;

	if((*temp_buf) == CLIENT_ASSOCIATION_EVENT_TYPE)
		temp_buf++;
	else
		return -1;


	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));
	if (length != 13) {
		debug("length(%d) error", length);
		return -1;
	}
	temp_buf+=2;

	assoc = &evt->assoc[evt->assoc_cnt];
	evt->assoc_cnt++;
	memcpy(assoc->sta_addr, temp_buf, ETH_ALEN);
	debug("client(%02x:%02x:%02x:%02x:%02x:%02x) ", PRINT_MAC(temp_buf));
	temp_buf += ETH_ALEN;
	memcpy(assoc->bssid, temp_buf, ETH_ALEN);
	pos = temp_buf;
	temp_buf += ETH_ALEN;
	assoc->is_joined = *temp_buf;

	bss_info = topo_srv_get_bss_by_bssid(ctx, NULL, assoc->bssid);
	if (assoc->is_joined) {
		 if (bss_info) {
#if 0
			if (is_local_adm_ether_addr(assoc->sta_addr))
				always("inform local wifi APCLI join\n");
			else
#endif
			always("inform local wifi APCLI/STA join\n");
			mapd_user_wireless_client_join(global,
				assoc->sta_addr, assoc->bssid,
				(char *)bss_info->ssid);
		}
	} else {
		 if (bss_info) {
#if 0
			if (is_local_adm_ether_addr(assoc->sta_addr))
				always("inform local wifi APCLI left\n");
			else
#endif				
			always("inform local wifi APCLI/STA left\n");
			mapd_user_wireless_client_leave(
				global,
				assoc->sta_addr, assoc->bssid,
				(char *)bss_info->ssid);
		}
	}

	info("%s the ", (*temp_buf) ? "join" : "left");
	info("bss(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(pos));

	return (length+3);

}

/**
* @brief Fn to parse topology event
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
* @param evt topology notification event
*
* @return 0 if success else -1
*/
int parse_topology_notification_evt(struct own_1905_device *ctx, unsigned char *buf, struct topo_notification *evt)
{
	int length =0;
	unsigned char *temp_buf;
	unsigned int integrity = 0;
	unsigned int right_integrity = 0x1;

	temp_buf = buf;
	evt->assoc_cnt = 0;
	while (1) {
		if (*temp_buf == AL_MAC_ADDR_TLV_TYPE) {
			integrity |= (1<<0);
			length=parse_al_mac_addr_type_tlv(temp_buf, evt->al_mac);

			if (length < 0) {
				err("error al mac tlv ");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == CLIENT_ASSOCIATION_EVENT_TYPE) {
			length = parse_client_assoc_event_tlv(ctx, temp_buf, evt);

			if(length < 0) {
				err("error client_assoc_event tlv");
				return -1;
			}

			temp_buf += length;
		} else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			err("wrong TLV in topology notification messagn");
			/*ignore extra tlv*/
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}

	/*check integrity*/
	if (integrity != right_integrity) {
		err("incomplete topology notification 0x%x 0x%x",
				integrity, right_integrity);
		return -1;
	}

	return 0;
}


/**
* @brief Fn to parse bridge capability tlv
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
* @param dev 1905 map device pointer
*
* @return tlv length if success else -1
*/
int parse_bridge_capability_type_tlv(struct own_1905_device *ctx, unsigned char *buf,
		struct _1905_map_device *dev)
{
	unsigned char *temp_buf;
	int length = 0;
	unsigned char br_num = 0;
	int i = 0;
	unsigned char br_itfs_num = 0;
	struct _1905_device *_1905_info = &dev->_1905_info;
	struct _1905_bridge *tmp_br, *br = SLIST_FIRST(&_1905_info->first_bridge);

	temp_buf = buf;

	if ((*temp_buf) == BRIDGE_CAPABILITY_TLV_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));
	//shift to tlv value field
	temp_buf += 2;

	//the field of total number of bridge tuple
	br_num = *temp_buf;
	temp_buf++;

	while (br) {
		tmp_br = SLIST_NEXT(br, next_bridge);
		if (br->interface_mac_tuple)
			free(br->interface_mac_tuple);
		free(br);
		br = tmp_br;
	}
	SLIST_INIT(&_1905_info->first_bridge);
	debug("br_num = %d",br_num);
	//loop br_num times to store the bridge tuple information
	for (i = 0; i < br_num; i++) {
		br_itfs_num = *temp_buf;
		temp_buf++;

		// debug("br_itfs_num = %d",br_itfs_num);

		br = (struct _1905_bridge *)
			os_malloc(sizeof(struct _1905_bridge));
		if(br == NULL) {
			err("alloc br fail");
			return -1;
		}
		os_memset(br, 0, sizeof(struct _1905_bridge));
		br->interface_count = br_itfs_num;

		if (br_itfs_num > 0)
			br->interface_mac_tuple =
				(unsigned char *)os_malloc(br_itfs_num * ETH_ALEN);

		if(br->interface_mac_tuple == NULL) {
			err("alloc interface_mac_tuple fail");
			return -1;
		}
		os_memset(br->interface_mac_tuple, 0, br_itfs_num * ETH_ALEN);
		memcpy(br->interface_mac_tuple, temp_buf,
				(br_itfs_num * ETH_ALEN));
		SLIST_INSERT_HEAD(&dev->_1905_info.first_bridge, br, next_bridge);

		temp_buf += br_itfs_num * ETH_ALEN;

		//debug("br->interface_count = %d",br->interface_count);
	}

	return (length + 3);
}

struct map_neighbor_info *topo_srv_create_insert_neighbor_entry(struct own_1905_device *ctx,
			struct _1905_map_device *dev, unsigned char *al_mac,
			unsigned char *own_iface, unsigned char *n_iface)
{
	struct backhaul_link_info *bh;
	struct map_neighbor_info *neighbor = os_zalloc(sizeof(struct map_neighbor_info));
	//Remove entry from client DB
	uint32_t client_id;
	u8 already_seen = 2;
	struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
	
	if (neighbor == NULL) {
		err("alloc memory fail");
		assert(0);
		return NULL;
	}
	memcpy(neighbor->n_almac, al_mac, ETH_ALEN);
	SLIST_INSERT_HEAD(&(dev->neighbors_entry), neighbor, next_neighbor);

	SLIST_INIT(&neighbor->bh_head);
	/* Since we are allcating it, there is a link available */
	bh = os_zalloc(sizeof(struct backhaul_link_info));
	if (!bh) {
		err("failed to allocate memory for bh info");
		assert(0);
		return NULL;
	}
	bh->is_valid = 1;
	SLIST_INSERT_HEAD(&neighbor->bh_head, bh, next_bh);

	/* update interface info if available */
	if (own_iface)
		os_memcpy(bh->connected_iface_addr, own_iface, ETH_ALEN);
	if (n_iface)
		os_memcpy(bh->neighbor_iface_addr, n_iface, ETH_ALEN);
	err("neighbor almac (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(neighbor->n_almac));
	err("connected_iface_addr (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bh->connected_iface_addr));
	err("neighbor_iface addr (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bh->neighbor_iface_addr));
	neighbor->insert_new_link = 1;

	if (n_iface) {		
		client_id = client_db_track_add(mapd_ctx, bh->neighbor_iface_addr, &already_seen);
		if (client_id == (uint32_t)-1) {
			 mapd_printf(MSG_ERROR, "No more room to accomodate" MACSTR
						", that has joined a BSS on another device", MAC2STR(bh->neighbor_iface_addr));
		}
	}

	return neighbor;
}

/**
* @brief Fn to parse 1905 neighbor device tlv
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
* @param dev 1905 map device pointer
*
* @return -1 if error else tlv length
*/
struct _1905_map_device *topo_srv_create_1905_device(struct own_1905_device *ctx, unsigned char *almac);
int parse_p1905_neighbor_device_type_tlv(struct own_1905_device *ctx, unsigned char *buf,
		struct _1905_map_device *dev)
{
	unsigned char *temp_buf;
	int length = 0;
	struct iface_info *iface;
	unsigned char local_mac[ETH_ALEN];
	unsigned char al_mac[ETH_ALEN];
	unsigned char exist = 0;
	unsigned char new_db = 1;
	int tuple_num = 0;
#ifdef REMOVE
	unsigned char up_device_invalid = FALSE;
#endif
	struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
	int i = 0;
	struct map_neighbor_info *neighbor, *tmp_neighbor;
	struct _1905_map_device *tmp_dev;
	struct _1905_map_device * own_1905_map_device = topo_srv_get_1905_device(ctx, NULL);
	char skip  = 0;
	struct bh_link_entry *bh_entry = NULL;

	if (own_1905_map_device != dev) {
		own_1905_map_device = NULL;
	}
	temp_buf = buf;

	if ((*temp_buf) == P1905_NEIGHBOR_DEV_TLV_TYPE) {
		temp_buf++;
	} else {
		err("should not go here");
		return -1;
	}

	/* calculate tlv length */
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));
	/*shift to tlv value field */
	temp_buf += 2;

	memcpy(local_mac, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	/* tlv value has one local mac address field(6 bytes) and several
	 * (al mac address field(6 bytes) + 802.1 exist field(1 byte)),
	 * so tlv (value length - 6 ) % 7 must be zero
	 */
	if (((length - 6) % 7) == 0) {
		tuple_num = (length - 6) / 7;
		info("tuple_num = %d",tuple_num);
	} else {
		err("ignore p1905.1 neighbor tlv");
		return (length + 3);
	}

	SLIST_FOREACH(iface, &(dev->_1905_info.first_iface), next_iface) {
		if (!os_memcmp(local_mac, iface->iface_addr, ETH_ALEN)) {
			/*it means it has map neighbor device, so it is backhaul interface*/
			if (tuple_num > 0)
				iface->is_map_if = 1;
			exist = 1;
			break;
		}
	}

	if (exist == 0) {
		err("interface not found");
		return (length + 3);
	}

	if (own_1905_map_device && tuple_num > 0 &&
		iface->media_type >= IEEE802_11_GROUP &&
		iface->media_info.role == 0x04) {
		SLIST_FOREACH(bh_entry, &ctx->bh_link_head, next_bh_link) {
				if (!mapd_ctx->params.Certification && !os_memcmp(local_mac, bh_entry->mac_addr, ETH_ALEN) &&
					bh_entry->bh_assoc_state == WAPP_APCLI_DISASSOCIATED) {
						err("apcli-"MACSTR" link down!!! drop stale own rsp",
										PRINT_MAC(local_mac));
						return (length + 3);
				}
		}
	}

	for (i = 0; i < tuple_num; i++) {
		skip = 0;
		memcpy(al_mac, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;

		/*do not use the topology rsp of my neighbor device to update all the link related
		*to me. it may be incorrect due to the asynchronous updating neighbor time between
		*two directly connected device
		*/
		if (!os_memcmp(ctx->al_mac, al_mac, ETH_ALEN)) {
			skip = 1;
			debug("skip neighbor(%02x:%02x:%02x:%02x:%02x:%02x) "
					"from dev(%02x:%02x:%02x:%02x:%02x:%02x)",
					PRINT_MAC(al_mac),
					PRINT_MAC(dev->_1905_info.al_mac_addr));

		}

        new_db = 1;
 		SLIST_FOREACH(neighbor, &(dev->neighbors_entry), next_neighbor) {
 			if (!os_memcmp(al_mac, neighbor->n_almac, ETH_ALEN)) {
 				new_db = 0;
				break;
            }
        }

		if (skip) {
			/*update link info*/
			if (!new_db) {
				topo_srv_update_neighbor_entry(ctx, neighbor, local_mac, skip);
				tmp_dev = topo_srv_get_1905_device(ctx, al_mac);
				SLIST_FOREACH(tmp_neighbor, &tmp_dev->neighbors_entry, next_neighbor)
				{
					if (!os_memcmp(tmp_neighbor->n_almac, dev->_1905_info.al_mac_addr, ETH_ALEN))
						break;
				}
				if (tmp_neighbor)
					topo_srv_update_neighbor_entry_peer(ctx, tmp_neighbor, local_mac, skip);
			}
			temp_buf += 1;
			continue;
		}

		if (new_db) {
			err("new_db!!! dev(%02x:%02x:%02x:%02x:%02x:%02x) "
				"has neighbor(%02x:%02x:%02x:%02x:%02x:%02x)",
				PRINT_MAC(dev->_1905_info.al_mac_addr),
				PRINT_MAC(al_mac));
			neighbor = topo_srv_create_insert_neighbor_entry(ctx, dev, al_mac, local_mac, NULL);
			tmp_dev = topo_srv_get_1905_device(ctx, al_mac);
			if (!tmp_dev) {
				tmp_dev = topo_srv_create_1905_device(ctx, al_mac);
				neighbor->neighbor = tmp_dev;
				/* Add one entry in its neighbor and bh for ourself */
				tmp_neighbor = topo_srv_create_insert_neighbor_entry(ctx, tmp_dev,
							dev->_1905_info.al_mac_addr, NULL, local_mac);
				tmp_neighbor->neighbor = dev;
			} else {
				tmp_neighbor = SLIST_FIRST(&tmp_dev->neighbors_entry);
				while (tmp_neighbor) {
					if (!os_memcmp(tmp_neighbor->n_almac, dev->_1905_info.al_mac_addr, ETH_ALEN))
						break;
					tmp_neighbor = SLIST_NEXT(tmp_neighbor, next_neighbor);
				}
				if (!tmp_neighbor) {
					tmp_neighbor = topo_srv_create_insert_neighbor_entry(ctx, tmp_dev,
							dev->_1905_info.al_mac_addr, NULL, local_mac);
					tmp_neighbor->neighbor = dev;
				}
			}
		} else {
           debug("exist db!!! dev(%02x:%02x:%02x:%02x:%02x:%02x) "
               "has neighbor(%02x:%02x:%02x:%02x:%02x:%02x)",
               PRINT_MAC(dev->_1905_info.al_mac_addr),
               PRINT_MAC(al_mac));

			tmp_dev = topo_srv_get_1905_device(ctx, al_mac);
			/* there is only one backhaul info, TODO modify it for multiple links */
			if(tmp_dev) {
				SLIST_FOREACH(tmp_neighbor, &tmp_dev->neighbors_entry, next_neighbor)
				{
					if (!os_memcmp(tmp_neighbor->n_almac, dev->_1905_info.al_mac_addr, ETH_ALEN))
						break;
				}
				if (!tmp_neighbor) {
					tmp_neighbor = topo_srv_create_insert_neighbor_entry(ctx, tmp_dev,
							dev->_1905_info.al_mac_addr, NULL, local_mac);
					tmp_neighbor->neighbor = dev;
					err("this should be an error condition");
				}
               /*update bh link of dev which is the neighbor of the device reporting topology response*/
               if (topo_srv_update_neighbor_entry_peer(ctx, tmp_neighbor, local_mac, skip) < 0)
                   return (length + 3);

               /*update bh link of dev which report the topology response*/
               if (topo_srv_update_neighbor_entry(ctx, neighbor, local_mac, skip) < 0)
                   return (length + 3);
			} else
				err("this is a critical condition");
		   }
		/* TODO handle bh link info for multiple links */
		if (!tmp_dev) {
			err("something is very wrong");
			return (length + 3);
		}
		neighbor->neighbor = tmp_dev;
		neighbor->neighbor->in_network = 1;
		neighbor->is_valid = 1;
		debug("neighbor in network (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(neighbor->n_almac));
		debug("is valid 1 for %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(neighbor->neighbor->_1905_info.al_mac_addr));
		os_get_time(&neighbor->neighbor->last_seen);
		/* Assumption, AP is always upstream device when operating as cli, correct? */
		if ((iface->media_type >= 0x0100) &&(iface->media_info.role == 0x4)) {
			err("updated upstream device");
			dev->upstream_device = tmp_dev;
		}

		if (((*temp_buf) & 0x80) == 0x80)
			neighbor->ieee_802_1_bridge_exist = 1;
		else
			neighbor->ieee_802_1_bridge_exist = 0;

		temp_buf += 1;
	}
	return (length + 3);
}

/**
* @brief Fn to parse non 1905 neighbor device tlv
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
* @param dev 1905 map device
*
* @return -1 if error else tlv length
*/
int parse_non_p1905_neighbor_device_type_tlv(struct own_1905_device *ctx, unsigned char *buf,
		struct _1905_map_device *dev)
{
	unsigned char *temp_buf;
	int length = 0;
	int tuple_num = 0;
	unsigned char local_mac[ETH_ALEN];
	unsigned char neighbor_mac[ETH_ALEN];
	struct iface_info *iface;
	struct connected_clients *client;

	unsigned char exist = 0;
	unsigned char new_db = 1;
	int i = 0;

	temp_buf = buf;

	if ((*temp_buf) == NON_P1905_NEIGHBOR_DEV_TLV_TYPE)
		temp_buf++;
	else {
		return -1;
	}

	/*calculate tlv length */
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));
	/*shift to tlv value field */
	temp_buf += 2;

	/*get local interface field */
	memcpy(local_mac, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;

	/*calculate how many non-1905.1 device */
	if (((length - 6) % 6) == 0) {
		tuple_num = (length - 6) / 6;
		//debug("tuple_num = %d",tuple_num);
	} else {
		//debug("ignore non p1905.1 neighbor tlv");
		return (length + 3);
	}

	SLIST_FOREACH(iface, &(dev->_1905_info.first_iface), next_iface) {
		if (!os_memcmp(local_mac, iface->iface_addr, ETH_ALEN)) {
			exist = 1;
			break;
		}
	}

	if (exist) {
		for (i = 0; i < tuple_num; i++) {
			memcpy(neighbor_mac, temp_buf, ETH_ALEN);
			temp_buf += ETH_ALEN;

			new_db = 1;
			/* maybe we don't need to do this check because the old topology
			 * response db will be deleted when we get a new topology response
			 * message.
			 */

			SLIST_FOREACH(client, &(dev->wlan_clients),
					next_client) {
				if (!os_memcmp(neighbor_mac, client->client_addr,
						 ETH_ALEN)) {
					new_db = 0;
					break;
				}
			}

			if (new_db) {
				client = (struct connected_clients *)os_malloc(sizeof
								(struct connected_clients));
				memcpy(client->client_addr, neighbor_mac,
						ETH_ALEN);
				memcpy(client->_1905_iface_addr, local_mac, ETH_ALEN);
				client->is_bh_link = 0;
				SLIST_INSERT_HEAD(&(dev->wlan_clients),
						client, next_client);

				client->is_APCLI = 0;
				if (iface->media_type < IEEE802_11_GROUP) {
					struct mapd_global *global = ctx->back_ptr;
					mapd_user_eth_client_join(global,
						client->client_addr, dev->_1905_info.al_mac_addr);
				}
			}
			client->entry_valid = TRUE;
		}
	}
	return (length + 3);
}

extern void mapd_start_wired_iface_monitor(void *eloop_ctx, void *timeout_ctx);

/**
* @brief Fn to parse supported service tlv
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
* @param dev 1905 map device
*
* @return -1 if error else tlv length
*/
int parse_supported_service_tlv(struct own_1905_device *ctx, unsigned char *buf,
		struct _1905_map_device *dev)
{

	unsigned char *temp_buf;
	int length = 0;
	temp_buf = buf;

	if ((*temp_buf) == SUPPORTED_SERVICE_TLV_TYPE)
		temp_buf++;
	else {
		return -1;
	}

	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));
	temp_buf += 2;

	/* as per specs, List of supported service + type(agent/controller/agent+controller)
	 * so the length can be either 2 or 3
	 */
	if (length == 2) {
		temp_buf++;
		dev->supported_services = *temp_buf;
		debug("support service %s",
				dev->supported_services ? "agent" : "controller");
		if (dev->supported_services)
			dev->device_role = DEVICE_ROLE_AGENT;
		else
			dev->device_role = DEVICE_ROLE_CONTROLLER;
	} else if (length == 3) {
		temp_buf++;
		dev->supported_services = 2;
       /*need update device role here*/
		dev->device_role = DEVICE_ROLE_CONTRAGENT;
		debug("support service agent and controller");
	} else {
		return (length + 3);
	}

	return (length + 3);
}

/**
* @brief Fn to parse ap operation type bss
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
* @param dev 1905 map device
*
* @return  -1 if error else tlv length
*/
int parse_ap_operational_bss_type_tlv(struct own_1905_device *ctx, unsigned char *buf,
		struct _1905_map_device *dev)
{
	unsigned char *temp_buf = NULL;
	int count = 0, i = 0, bss_count = 0, j = 0;
	int length = 0;
	struct radio_info_db *radio  = NULL;
	struct bss_info_db *bss = NULL;

	if(!ctx || !buf || !dev) {
		err(" invaild input params\n");
		return -1;
	}

	temp_buf = buf;

	if ((*temp_buf) == AP_OPERATIONAL_BSS_TYPE)
		temp_buf++;
	else {
		return -1;
	}

	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));
	temp_buf += 2;

	count = *temp_buf;
	temp_buf++;

	for (i = 0; i < count; i++) {
		radio = topo_srv_get_radio(dev, temp_buf);
		if (!radio) {
			radio = os_zalloc(sizeof(*radio));
			if (!radio) {
				err("mem allocation failed");
				return -1;
			}
			memcpy(radio->identifier, temp_buf, ETH_ALEN);
			radio->parent_1905 = dev;
			SLIST_INSERT_HEAD(&(dev->first_radio), radio, next_radio);
			debug("new radio interface");
			SLIST_INIT(&(radio->link_estimate_cb_head));
			SLIST_INIT(&radio->chan_preferance.prefer_info_head);
			SLIST_INIT(&radio->chan_restrict.restrict_head);
#ifdef MAP_R2
			SLIST_INIT(&radio->first_scan_result);
			SLIST_INIT(&radio->cac_cap.cac_capab_head);
			SLIST_INIT(&radio->cac_comp_status.cac_completion_opcap_head);
#endif
			radio->bh_priority = 1;
		}
		topo_srv_mark_all_oper_bss_invalid(ctx, dev, radio);
		temp_buf += ETH_ALEN;
		bss_count = *temp_buf;
		temp_buf++;
		for (j = 0; j < bss_count; j++) {
			bss = topo_srv_get_bss_by_bssid(ctx, dev, temp_buf);
			if (!bss) {
				bss = os_malloc(sizeof(*bss));
				if (!bss) {
					err("mem allocation failed");
					return -1;
				}
				os_memset(bss, 0, sizeof(*bss));
				memcpy(bss->bssid, temp_buf, ETH_ALEN);
				SLIST_INIT(&bss->esp_head);
				SLIST_INSERT_HEAD(&(dev->first_bss), bss, next_bss);
#ifdef ACL_CTRL
				dl_list_init(&bss->acl_cli_list);
#endif
			}
			temp_buf += ETH_ALEN;
			bss->ssid_len = 0;
			bss->ssid_len = *temp_buf;
			temp_buf++;
			bss->radio = radio;
			bss->valid = TRUE;
			os_memset(bss->ssid, 0, MAX_SSID_LEN);
			memcpy(bss->ssid, temp_buf, bss->ssid_len);
			temp_buf += bss->ssid_len;
			debug("bssid(%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bss->bssid));
			debug("ssid %s", bss->ssid);
		}
	}

	return (length + 3);
}
void clear_all_client_info(struct own_1905_device *ctx, struct _1905_map_device *dev)
{
	struct associated_clients *client = NULL, *next_cli = NULL;
	struct connected_clients *conn_client = NULL, *next_conn_client = NULL;
	client = SLIST_FIRST(&dev->assoc_clients);
	while (client) {
		next_cli = SLIST_NEXT(client, next_client);
		mapd_printf(MSG_INFO, "%s deleted assoc sta=%02x:%02x:%02x:%02x:%02x:%02x",
				__FUNCTION__, PRINT_MAC(client->client_addr));
		SLIST_REMOVE(&dev->assoc_clients, client, associated_clients, next_client);
		os_free(client);
		client = next_cli;
	}
	SLIST_INIT(&dev->assoc_clients);
	conn_client = SLIST_FIRST(&dev->wlan_clients);
	while(conn_client) {
		struct bss_info_db *bss = NULL;
		bss = topo_srv_get_bss_by_bssid(ctx, dev, conn_client->_1905_iface_addr); //removing wireless clients
		next_conn_client = SLIST_NEXT(conn_client, next_client);
		if(bss == NULL) {
#if 1
			conn_client->entry_valid = FALSE;
#else
			mapd_printf(MSG_INFO, "%s deleted eth sta=%02x:%02x:%02x:%02x:%02x:%02x",
				__FUNCTION__, PRINT_MAC(conn_client->client_addr));
			SLIST_REMOVE(&dev->wlan_clients, conn_client, connected_clients, next_client);
			os_free(conn_client);
#endif
		}
		conn_client = next_conn_client;
	}
	return;
}
void clear_invalid_client_info(struct own_1905_device *ctx, struct _1905_map_device *dev)
{
	struct connected_clients *conn_client = NULL, *next_conn_client = NULL;

	conn_client = SLIST_FIRST(&dev->wlan_clients);
	while(conn_client) {
		struct bss_info_db *bss = NULL;
		bss = topo_srv_get_bss_by_bssid(ctx, dev, conn_client->_1905_iface_addr); //removing wireless clients
		next_conn_client = SLIST_NEXT(conn_client, next_client);
		if((bss == NULL) && (conn_client->entry_valid == FALSE)) {
			struct mapd_global *global = ctx->back_ptr;

			mapd_printf(MSG_INFO, "%s deleted eth sta=%02x:%02x:%02x:%02x:%02x:%02x",
				__FUNCTION__, PRINT_MAC(conn_client->client_addr));
			mapd_user_eth_client_leave(global,
				conn_client->client_addr, dev->_1905_info.al_mac_addr);
			SLIST_REMOVE(&dev->wlan_clients, conn_client, connected_clients, next_client);
			os_free(conn_client);
		}
		conn_client = next_conn_client;
	}
	return;
}

/**
* @brief Fn to parse ap associated clients tlv
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
* @param dev 1905 map device
*
* @return -1 if error else tlv length
*/
int parse_ap_associated_clients_type_tlv(struct own_1905_device *ctx, unsigned char *buf,
		struct _1905_map_device *dev)
{
	unsigned char *temp_buf;
	int length = 0;
	u8 bss_cnt = 0, j = 0;
	u16 sta_cnt = 0, i = 0;
	u8 *bssid = NULL;
	struct associated_clients *assoc_client = NULL;
	struct bss_info_db *bss = NULL;

	temp_buf = buf;

	if ((*temp_buf) == AP_ASSOCIATED_CLIENTS_TYPE)
		temp_buf++;
	else {
		return -1;
	}

	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));
	temp_buf += 2;

	bss_cnt = *temp_buf;
	temp_buf++;
	for (j = 0; j < bss_cnt; j++) {
		bssid = temp_buf;
		temp_buf += ETH_ALEN;

		bss = topo_srv_get_bss_by_bssid(ctx, dev, bssid);
		if (!bss) {
			bss = os_malloc(sizeof(*bss));
			if (!bss) {
				err("mem allocation failed");
				return -1;
			}
			os_memset(bss, 0, sizeof(*bss));
			SLIST_INIT(&bss->esp_head);
			memcpy(bss->bssid, bssid, ETH_ALEN);
			err("inserting into dev list %p", dev);
			SLIST_INSERT_HEAD(&(dev->first_bss), bss, next_bss);
#ifdef ACL_CTRL
			dl_list_init(&bss->acl_cli_list);
#endif
			bss->valid = TRUE;
		}
		sta_cnt = *((u16*)temp_buf);
		sta_cnt = be_to_host16(sta_cnt);
		temp_buf += 2;
		bss->assoc_sta_cnt = sta_cnt;
		for (i = 0; i < sta_cnt; i++) {
			assoc_client = (struct associated_clients*)os_malloc(sizeof(struct associated_clients));
			assoc_client->bss = bss;
			os_memcpy(assoc_client->client_addr, temp_buf, ETH_ALEN);
			temp_buf += ETH_ALEN;
			assoc_client->last_assoc_time = *((u16*)temp_buf);
			assoc_client->last_assoc_time = be_to_host16(assoc_client->last_assoc_time);
			assoc_client->is_bh_link = 0;
			assoc_client->is_APCLI = 0;
			temp_buf += 2;
			SLIST_INSERT_HEAD(&dev->assoc_clients, assoc_client, next_client);

			mapd_printf(MSG_INFO, "%s insert assoc sta=%02x:%02x:%02x:%02x:%02x:%02x",
				__FUNCTION__, PRINT_MAC(assoc_client->client_addr));
#ifdef CENT_STR
			if (ctx->cent_str_en && ctx->device_role == DEVICE_ROLE_CONTROLLER) {
				struct mapd_global *global = (struct mapd_global *)ctx->back_ptr;
				struct client *cli = client_db_get_client_from_sta_mac(global, assoc_client->client_addr);
				if (cli && cli->bssid == NULL)
					map_1905_Send_Client_Capability_Query_Message(global->_1905_ctrl, (char *)dev->_1905_info.al_mac_addr, bssid, assoc_client->client_addr);
			}
#endif
		}
	}
	/*TODO do we need to parse this tlv further? */
	return (length + 3);
}

/**
* @brief Fn to parse radio basic caps tlv
*
* @param buf msg buffer
* @param identifier radio identifier
* @param max_bss_num max bss number count
* @param band_cap band caps
*
* @return -1 if error else tlv length
*/
int parse_ap_radio_basic_cap_tlv(unsigned char *buf,
		unsigned char *identifier,
		unsigned char *max_bss_num,
		unsigned char *band_cap)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;
	unsigned char opnum = 0, i = 0, non_opch_num = 0;
	unsigned char opclass = 0;
	unsigned char cap = 0;

	temp_buf = buf;

	if ((*temp_buf) == AP_RADIO_BASIC_CAPABILITY_TYPE)
		temp_buf++;
	else
		return -1;

	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));
	temp_buf += 2;

	/*here only parse identifier of ap_radio_basic_cap_tlv */
	memcpy(identifier, temp_buf, 6);
	temp_buf += 6;
	*max_bss_num = *temp_buf++;
	/*Number of operating classes */
	opnum = *temp_buf++;
	*band_cap = 0;
	for (i = 0; i < opnum; i++) {
		opclass = *temp_buf;
		temp_buf += 2;
		non_opch_num = *temp_buf++;
		cap = get_bandcap(opclass, non_opch_num, temp_buf);
		*band_cap |= cap;
		temp_buf += non_opch_num;
	}
	debug("this m1 bandcap=%d", *band_cap);

	debug("radio_id(%02x:%02x:%02x:%02x:%02x:%02x)",
			PRINT_MAC(identifier));

	return (length + 3);
}

/**
* @brief Fn to parse backhaul steer request msg
*
* @param ctx own 1905 device ctx
* @param buf msg buffer
*
* @return 0 if success else -1
*/
int parse_backhaul_steering_request_message(struct own_1905_device *ctx,
		unsigned char *buf)
{
	int length = 0;
	unsigned char *temp_buf;
	unsigned char integrity = 0;

	temp_buf = buf;

	while (1) {
		if (*temp_buf == BACKHAUL_STEERING_REQUEST_TYPE) {
			integrity |= 0x1;
			length =
				parse_backhaul_steering_request_tlv(temp_buf, ctx);
			if (length < 0) {
				err("error backhaul steering request tlv");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	/*check integrity */
	if (integrity != 0x1) {
		err("no backhaul steering request tlv");
		return -1;
	}

	return 0;
}

int topo_srv_update_neighbor_entry(struct own_1905_device *ctx,
	struct map_neighbor_info *neighbor, unsigned char *local_mac, char skip)
{
	struct backhaul_link_info *bh = NULL;
	unsigned char zero_mac[ETH_ALEN] = {0};
	struct iface_info *niface = NULL, *ciface = NULL;

	//Remove entry from client DB
	uint32_t client_id;
	u8 already_seen = 2;
	struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;

	SLIST_FOREACH(bh, &neighbor->bh_head, next_bh)
	{
		if (!os_memcmp(bh->connected_iface_addr, local_mac, ETH_ALEN)) {
			bh->is_valid = 1;
			debug("skip(%d)", skip);
			debug("neighbor almac (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(neighbor->n_almac));
			debug("connected_iface_addr (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bh->connected_iface_addr));
			debug("neighbor_iface addr (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bh->neighbor_iface_addr));
			client_id = client_db_track_add(mapd_ctx, bh->neighbor_iface_addr, &already_seen);
			if (client_id == (uint32_t)-1) {
				 mapd_printf(MSG_ERROR, "No more room to accomodate" MACSTR
							", that has joined a BSS on another device", MAC2STR(bh->neighbor_iface_addr));
			}
			return 0;
		}
	}

	ciface = topo_srv_find_iface_by_mac(ctx, local_mac);
	if (!ciface) {
		err("err condition! cannot find connected iface("MACSTR")",
			MAC2STR(local_mac));
		return -1;
	}


	SLIST_FOREACH(bh, &neighbor->bh_head, next_bh)
	{
		/*check if need update connected_iface_addr of exist link  */
		if (os_memcmp(bh->neighbor_iface_addr, zero_mac, ETH_ALEN)) {
			niface = topo_srv_find_iface_by_mac(ctx, bh->neighbor_iface_addr);
			if (!niface) {
				err("err condition! cannot find iface("MACSTR")",
					MAC2STR(bh->neighbor_iface_addr));
				return -1;
			}
			/*if neighbor interface is ap apcli interface and its bssid equal to local_mac*/
			if ((niface->media_type >= IEEE802_11_GROUP)
				&& (niface->media_info.role == 0x04)) {
				if (!os_memcmp(niface->media_info.network_membership, local_mac, ETH_ALEN)) {
					os_memcpy(bh->connected_iface_addr, local_mac, ETH_ALEN);
					bh->is_valid = 1;
					break;
				}
			/*if neighbor interface is ap interface and local_mac is apcli interface and
			*equal to apcli's bssid
			*/
			} else if ((niface->media_type >= IEEE802_11_GROUP)
						&& (niface->media_info.role == 0x00)) {
				if ((ciface->media_type >= IEEE802_11_GROUP)
					&& (ciface->media_info.role == 0x04) &&
					!os_memcmp(ciface->media_info.network_membership, bh->neighbor_iface_addr, ETH_ALEN)) {
					os_memcpy(bh->connected_iface_addr, local_mac, ETH_ALEN);
					bh->is_valid = 1;
					break;
				}
			/*if neighbor interface and local_mac is eth interface*/
			} else if (niface->media_type < ieee_802_11_b) {
				if (ciface->media_type < ieee_802_11_b) {
					os_memcpy(bh->connected_iface_addr, local_mac, ETH_ALEN);
					bh->is_valid = 1;
					break;
				}
			}
		}
	}

	if (!bh && !skip) {
		bh = os_zalloc(sizeof(struct backhaul_link_info));
		if (!bh) {
			err("failed to allocate memory for bh info");
			assert(0);
			return -1;
		}
		bh->is_valid = 1;
		SLIST_INSERT_HEAD(&neighbor->bh_head, bh, next_bh);
		os_memcpy(bh->connected_iface_addr, local_mac, ETH_ALEN);
		neighbor->insert_new_link = 1;
	}

	if (bh) {
		client_id = client_db_track_add(mapd_ctx, bh->neighbor_iface_addr, &already_seen);
		if (client_id == (uint32_t)-1) {
			 mapd_printf(MSG_ERROR, "No more room to accomodate" MACSTR
						", that has joined a BSS on another device", MAC2STR(bh->neighbor_iface_addr));
		}
		err("skip(%d)", skip);
		err("neighbor almac (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(neighbor->n_almac));
		err("connected_iface_addr (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bh->connected_iface_addr));
		err("neighbor_iface addr (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bh->neighbor_iface_addr));
	}

	return 0;
}

#ifdef MAP_R2
int parse_unsuccessful_association_policy_tlv(unsigned char *buf,
		struct own_1905_device *ctx)
{
	unsigned char *temp_buf;
	unsigned short length = 0/*, i =0*/;
	struct unsuccessful_association_policy *assoc_failed_policy = &ctx->map_policy.assoc_failed_policy;
	temp_buf = buf;
	int report_rate = 0;
	
	debug("@@@@parse_unsuccessful_association_policy_tlv @@@@\n");
	if((*temp_buf) == UNSUCCESSFUL_ASSOCIATION_POLICY_TYPE) {
		temp_buf++;
	}
	else {
		err("should not go here");
		return -1;
	}

	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));
	mapd_hexdump(MSG_OFF, "parse_unsuccessful_association_policy_tlv", buf, length+3);
	//err("@@@@parse_unsuccessful_association_policy_tlv @@@@\n");
	//printf("@@@@parse_unsuccessful_association_policy_tlv %d @@@@\n", __LINE__);

	//shift to tlv value field
	temp_buf += 2;
	
	//assoc_failed_policy = (struct unsuccessful_association_policy *)os_malloc(sizeof(struct unsuccessful_association_policy));
	//if (!assoc_failed_policy) {
		//err("alloc struct unsuccessful_association_policy fail");
	//	return -1;
	//}
	if (*temp_buf && 0x80)
		assoc_failed_policy->report_unsuccessful_association = 1;
	else
		assoc_failed_policy->report_unsuccessful_association = 0;
	temp_buf += 1;
	//assoc_failed_policy->max_supporting_rate = *temp_buf;
	report_rate = (*((unsigned int *)temp_buf));

	assoc_failed_policy->max_supporting_rate = host_to_be32(report_rate);
	temp_buf += 4;
	
	//printf("@@@@parse_unsuccessful_association_policy_tlv %d length = %d @@@@\n", __LINE__, length);
	
	printf("@@@@assoc_failed_policy->supporting rate = %d @@@@\n",ctx->map_policy.assoc_failed_policy.max_supporting_rate);
	return length+3;
}
int parse_map_version_tlv(unsigned char *buf,
	unsigned char *almac, unsigned char *profile)
{
	unsigned char *temp_buf;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == MULTI_AP_VERSION_TYPE) {
		temp_buf++;
	}
	else {
		return -1;
	}

	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);

	//shift to tlv value field
	temp_buf += 2;
	*profile = *temp_buf;

	err("peer(%02x:%02x:%02x:%02x:%02x:%02x) multi-ap profile=%02x\n",
		PRINT_MAC(almac), *profile);

	return (length+3);
}

#endif

int topo_srv_update_neighbor_entry_peer(struct own_1905_device *ctx,
	struct map_neighbor_info *neighbor, unsigned char *local_mac, char skip)
{
	struct backhaul_link_info *bh = NULL;
	unsigned char zero_mac[ETH_ALEN] = {0};
	struct iface_info *niface = NULL, *ciface = NULL;

	SLIST_FOREACH(bh, &neighbor->bh_head, next_bh)
	{
		if (!os_memcmp(bh->neighbor_iface_addr, local_mac, ETH_ALEN)) {
			debug("skip(%d)", skip);
			debug("neighbor almac (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(neighbor->n_almac));
			debug("connected_iface_addr (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bh->connected_iface_addr));
			debug("neighbor_iface addr (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bh->neighbor_iface_addr));
			return 0;
		}
	}

	niface = topo_srv_find_iface_by_mac(ctx, local_mac);
	if (!niface) {
		err("err condition! cannot find connected iface("MACSTR")",
			MAC2STR(local_mac));
		return -1;
	}


	SLIST_FOREACH(bh, &neighbor->bh_head, next_bh)
	{
		/*check if need update neighbor_iface_addr of exist link  */
		if (os_memcmp(bh->connected_iface_addr, zero_mac, ETH_ALEN)) {
			ciface = topo_srv_find_iface_by_mac(ctx, bh->connected_iface_addr);
			if (!ciface) {
				err("err condition! cannot find iface("MACSTR")",
					MAC2STR(bh->connected_iface_addr));
				return -1;
			}
			/*if connected interface is ap apcli interface and its bssid equal to local_mac*/
			if ((ciface->media_type >= IEEE802_11_GROUP)
				&& (ciface->media_info.role == 0x04)) {
				if (!os_memcmp(ciface->media_info.network_membership, local_mac, ETH_ALEN)) {
					os_memcpy(bh->neighbor_iface_addr, local_mac, ETH_ALEN);
					break;
				}
			/*if connected interface is ap interface and local_mac is apcli interface and
			*equal to apcli's bssid
			*/
			} else if ((ciface->media_type >= IEEE802_11_GROUP)
						&& (ciface->media_info.role == 0x00)) {
				if (((niface->media_type >= IEEE802_11_GROUP)
					&& (niface->media_info.role == 0x04)) &&
					!os_memcmp(niface->media_info.network_membership, bh->connected_iface_addr, ETH_ALEN)) {
					os_memcpy(bh->neighbor_iface_addr, local_mac, ETH_ALEN);
					break;
				}
			/*if connected interface and local_mac is eth interface*/
			} else if (ciface->media_type < ieee_802_11_b) {
				if (niface->media_type < ieee_802_11_b) {
					os_memcpy(bh->neighbor_iface_addr, local_mac, ETH_ALEN);
					break;
				}
			}
		}
	}

	if (!bh && !skip) {
		bh = os_zalloc(sizeof(struct backhaul_link_info));
		if (!bh) {
			err("failed to allocate memory for bh info");
			assert(0);
			return -1;
		}
		bh->is_valid = 1;
		SLIST_INSERT_HEAD(&neighbor->bh_head, bh, next_bh);
		os_memcpy(bh->neighbor_iface_addr, local_mac, ETH_ALEN);
		neighbor->insert_new_link = 1;
	}

	if (bh) {
		debug("skip(%d)", skip);
		debug("neighbor almac (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(neighbor->n_almac));
		debug("connected_iface_addr (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bh->connected_iface_addr));
		debug("neighbor_iface addr (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bh->neighbor_iface_addr));
	}

	return 0;
}
int parse_backhaul_steering_rsp_tlv(unsigned char *buf,
	struct own_1905_device *ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;
	struct backhaul_steer_rsp *steer_rsp;
	temp_buf = buf;
	if((*temp_buf) == BACKHAUL_STEERING_RESPONSE_TYPE) {
		temp_buf++;
	} else {
		return -1;
	}
	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	//length = be2cpu16(length);
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));
	temp_buf += 2;
	steer_rsp = (struct backhaul_steer_rsp *)temp_buf;
	err("bh mac"MACSTR" ", MAC2STR(steer_rsp->backhaul_mac));
	err("target bssid"MACSTR" ", MAC2STR(steer_rsp->target_bssid));
	err("status %d",steer_rsp->status);
	if (steer_rsp->status == 0)
		{
			err("success bh steer \n");
		}
	else
		{
			err("bh steer fail\n");
		}
#if 0 
	memcpy(temp_buf, steer_rsp->backhaul_mac, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;
	memcpy(temp_buf, steer_rsp->target_bssid, ETH_ALEN);
	temp_buf += ETH_ALEN;
	total_length += ETH_ALEN;
	*temp_buf++ = steer_rsp->status;
	total_length += 1;
#endif
	return (length+3);
}
int parse_error_code_tlv(unsigned char *buf,
	struct own_1905_device *ctx)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;
	temp_buf = buf;
	if((*temp_buf) == ERROR_CODE_TYPE) {
		temp_buf++;
	}
	else {
		return -1;
	}
	//calculate tlv length
	length = *(unsigned short *)temp_buf;
	length = (length << 8) & 0xFF00;
	length = length | (*(temp_buf + 1));
	if (length != 7) {
		return -1;
	}
	return (length+3);
}
int parse_backhaul_steering_rsp_message(struct own_1905_device *ctx,
	unsigned char *buf)
{
	int length =0;
	unsigned char *temp_buf;
	unsigned char integrity = 0;
	temp_buf = buf;
	while(1) {
		if (*temp_buf == BACKHAUL_STEERING_RESPONSE_TYPE) {
			integrity |= 0x1;
			length = parse_backhaul_steering_rsp_tlv(temp_buf, ctx);
			if(length < 0) {
				err("error backhaul steering response tlv\n");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == ERROR_CODE_TYPE) {
			length = parse_error_code_tlv(temp_buf, ctx);
			if(length < 0) {
				err("error error code tlv\n");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	if(integrity != 0x1) {
		err("no backhaul steering response tlv\n");
		return -1;
	}
	return 0;
}
#ifdef CENT_STR

int parse_client_info_tlv(unsigned char *buf,
	struct own_1905_device *ctx, unsigned char *bssid, unsigned char *sta_mac)
{
	unsigned char *temp_buf = NULL;
	unsigned short length = 0;

	temp_buf = buf;

	if((*temp_buf) == CLIENT_INFO_TYPE)
		temp_buf++;
	else
		return -1;

	length = *(unsigned short *)temp_buf;
	length = be_to_host16(length);
	if (length != 12) {
		err("length(%d) error\n", length);
		return -1;
	}
	temp_buf+=2;

	debug("BSSID (%02x:%02x:%02x:%02x:%02x:%02x) ", PRINT_MAC(temp_buf));
	os_memcpy(bssid, temp_buf, ETH_ALEN);
	temp_buf += ETH_ALEN;
	debug("client (%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(temp_buf));
	os_memcpy(sta_mac, temp_buf, ETH_ALEN);
	return (length+3);

}

int parse_client_capability_report_tlv(struct own_1905_device *ctx,
	unsigned char *buf, unsigned char *assoc_req, unsigned int assoc_len)
{
    unsigned char *temp_buf = NULL;
    unsigned short length = 0;

    temp_buf = buf;
    if(!temp_buf){
		err("buf is null");
		return -1;
    }
    if((*temp_buf) == CLIENT_CAPABILITY_REPORT_TYPE) {
        temp_buf++;
    }
    else {
        return -1;
    }

    //calculate tlv length
    length = *(unsigned short *)temp_buf;
    length = be_to_host16(length);
	temp_buf += 2;
	if (length <= 1) {
		err("assoc length is invalid");
		return -1;
	}
	if(length - sizeof(unsigned char) > assoc_len)
		os_memcpy(assoc_req, temp_buf + 1, assoc_len);
	else
		os_memcpy(assoc_req, temp_buf + 1, length - sizeof(unsigned char));

    return (length+3);
}

int parse_client_capability_report_message(struct own_1905_device *ctx,
		unsigned char *buf, unsigned char *temp_bssid, unsigned char *temp_sta,
		unsigned char *assoc_req, unsigned int *assoc_len)
{
	int length =0, ass_len = 0;
	unsigned char *temp_buf;
	temp_buf = buf;
	ass_len = *assoc_len;
	while(1)
	{
		if(*temp_buf == CLIENT_INFO_TYPE)
		{
			length = parse_client_info_tlv(temp_buf, ctx, temp_bssid, temp_sta);
			if(length < 0)
			{
				err("error client info tlv\n");
				return -1;
			}
			temp_buf += length;
		} else if (*temp_buf == CLIENT_CAPABILITY_REPORT_TYPE) {
			length = parse_client_capability_report_tlv(ctx, temp_buf, assoc_req, ass_len);
			if(length < 0)
			{
				err("error client capability report tlv\n");
				return -1;
			}
			err("length %d",length );
			*assoc_len = length - 4;
			temp_buf += length;
		} else if (*temp_buf == ERROR_CODE_TYPE) {
			length = parse_error_code_tlv(temp_buf, ctx);
			if(length < 0)
			{
				err( "error in error code tlv\n");
				return -1;
			}
			temp_buf += length;
		} else if(*temp_buf == END_OF_TLV_TYPE) {
			break;
		} else {
			length = get_cmdu_tlv_length(temp_buf);
			temp_buf += length;
		}
	}
	return 0;
}
#endif
