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

#include <stdlib.h>
#include <stdio.h>
#ifdef OPENWRT_SUPPORT
#include <libdatconf.h>
#endif
#include "wapp_cmm.h"
#include "driver_wext.h"
#include "interface.h"
#include "map_test.h"
#include "wps.h"
#include "dhcp_ctl.h"

#define MAX_NUM_DEPENDENT_CHANNELS			4
struct channel_operable_status_s {
	unsigned char channel_num;
	unsigned char operable_status;
};
struct channel_dependency_list_s {
	unsigned char channel_num;
	unsigned char opclass;	/* regulatory class */
	unsigned char dependent_channel_set[MAX_NUM_DEPENDENT_CHANNELS];	/* max 13 channels, use 0 as terminator */
};

struct channel_operable_status_s channel_operable_status[] = {
#ifdef WIFI_MD_COEX_SUPPORT
{1, 1},
{2, 1},
{3, 1},
{4, 1},
{5, 1},
{6, 1},
{7, 1},
{8, 1},
{9, 1},
{10, 1},
{11, 1},
{12, 1},
{13, 1},
{14, 1},
#endif
{36, 1},
{40, 1},
{44, 1},
{48, 1},
{52, 1},
{56, 1},
{60, 1},
{64, 1},
{100, 1},
{104, 1},
{108, 1},
{112, 1},
{116, 1},
{120, 1},
{124, 1},
{128, 1},
{132, 1},
{136, 1},
{140, 1},
{144, 1},
{149, 1},
{153, 1},
{157, 1},
{161, 1},
{165, 1},
{169, 1},
};

struct channel_dependency_list_s channel_dependency_list[] = {
	{36,	116	,{40}},
	{40,	117	,{36}},
	{44,	116	,{48}},
	{48,	117	,{44}},
	{52,	119	,{56}},
	{56,	120	,{52}},
	{60,	119	,{64}},
	{64,	120	,{60}},
	{100,	122	,{104}},
	{104,	123	,{100}},
	{108,	122	,{112}},
	{112,	123	,{108}},
	{116,	122	,{120}},
	{120,	123	,{116}},
	{124,	122	,{128}},
	{128,	123	,{124}},
	{132,	122	,{136}},
	{136,	123	,{132}},
	{140,	122	,{144}},
	{144,	123	,{140}},
	{149,	126	,{153}},
	{153,	127	,{149}},
	{157,	126	,{161}},
	{161,	127	,{157}},
	{42 ,	128	,{36		,40		,44		,48}},
	{58 ,	128	,{52		,56		,60		,64}},
	{106,	128	,{100	,104	,108	,112}},
	{122,	128	,{116	,120	,124	,128}},
	{138,	128	,{132	,136	,140	,144}},
	{155,	128	,{149	,153	,157	,161}},
};
/* MAP Command API */
int map_cmd_show_help( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int map_cmd_show_param( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int map_cmd_set_bh_type( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int map_cmd_set_alid( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int map_cmd_send_1905( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int map_cmd_reset_default( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int map_cmd_set_dev_config( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int map_cmd_trigger_wps( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int map_cmd_get_macaddr_by_ssid_ruid( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
int map_cmd_1905_req( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
#ifdef MAP_R2
int map_cmd_ch_scan_req( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
#ifdef DFS_CAC_R2
int map_cmd_cac_status( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
#endif
#endif
#ifdef STOP_BEACON_FEATURE
int map_cmd_set_beacon_state( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
#endif

/*************************************************
       map related function
**************************************************/
int hex2num(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}


int hwaddr_aton(const char *txt, u8 *addr)
{
	int i;

	for (i = 0; i < 6; i++) {
		int a, b;

		a = hex2num(*txt++);
		if (a < 0)
			return -1;
		b = hex2num(*txt++);
		if (b < 0)
			return -1;
		*addr++ = (a << 4) | b;
		if (i < 5 && *txt++ != ':')
			return -1;
	}

	return 0;
}

int map_send_reset_msg(struct wifi_app *wapp)
{
	struct evt *wapp_event;
	char buf[50]= {0};
	int send_pkt_len = 0;

	DBGPRINT(RT_DEBUG_ERROR, "%s\n", __func__);
	wapp_event = (struct evt *)buf;
	wapp_event->type = WAPP_MAP_RESET;
	wapp_event->length = 0;

	send_pkt_len = sizeof(*wapp_event) + wapp_event->length;

	if(0 > map_1905_send(wapp, buf, send_pkt_len)) {
		printf("send map reset event\n");
		return -1;
	}

	memset(buf, 0, send_pkt_len);
	return 0;
}

void map_bss_table_release(struct map_info *map)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (map->op_bss_table) {
		os_free(map->op_bss_table);
		map->op_bss_table = NULL;
	}
}

void map_bss_table_init(struct map_info *map)
{
	u32 mem_size;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	mem_size = (sizeof(wdev_bss_info) * 16);	//HW_BEACON_MAX_NUM
	map->op_bss_table = os_zalloc(mem_size);

	if (!map->op_bss_table) {
		DBGPRINT_RAW(RT_DEBUG_ERROR, "ERROR! num_of_bss is zero.\n");
	}
}

void map_reset_conf_sm(struct map_info *map)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	map->conf_ongoing_radio = NULL;
	map->conf_ongoing_radio_idx = 0;
	map->ongoing_conf_retry_times = 0;
	map->conf = MAP_CONN_STATUS_UNCONF;
}

int map_read_config_file(struct map_info *map)
{
	FILE *file;
	char buf[256], *pos, *token, *token1;
	char tmpbuf[256];
	int line = 0, i = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!map) {
		printf("\033[1;31m %s, %u \033[0m\n", __FUNCTION__, __LINE__);  /* Haipin Debug Print (R)*/
		return WAPP_INVALID_ARG;
	}

	file = fopen(MAP_1905_CFG_FILE, "r");

	if (!file) {
		printf("\033[1;31m %s, %u \033[0m\n", __FUNCTION__, __LINE__);  /* Haipin Debug Print (R)*/
		DBGPRINT(RT_DEBUG_ERROR, "open MAP cfg file (%s) fail\n", MAP_1905_CFG_FILE);
		return WAPP_NOT_INITIALIZED;
	}

	os_memset(buf, 0, 256);
	os_memset(tmpbuf, 0, 256);

	while (wapp_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
		os_strcpy(tmpbuf, pos);
		token = strtok(pos, "=");

		if (token != NULL) {
			if (os_strcmp(token, "map_controller_alid") == 0) {
				token = strtok(NULL, "");
				i = 0;
				token1 = strtok(token, ":");

				while (token1 != NULL) {
					AtoH(token1, (char *) &map->ctrl_alid[i], 1);
					i++;
					if (i >= MAC_ADDR_LEN)
						break;
					token1 = strtok(NULL, ":");
				}
				DBGPRINT(RT_DEBUG_TRACE, "ctrl_alid = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(map->ctrl_alid));
			} else if (os_strcmp(token, "map_agent_alid") == 0) {
				token = strtok(NULL, "");
				i = 0;
				token1 = strtok(token, ":");

				while (token1 != NULL) {
					AtoH(token1, (char *) &map->agnt_alid[i], 1);
					i++;
					if (i >= MAC_ADDR_LEN)
						break;
					token1 = strtok(NULL, ":");
				}
				DBGPRINT(RT_DEBUG_TRACE, "agnt_alid = %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(map->agnt_alid));
			} else if (os_strcmp(token, "bh_type") == 0) {
				token = strtok(NULL, "");
				if (os_strcmp(token, "eth") == 0) {
					map->bh_type = MAP_BH_ETH;
				} else if (os_strcmp(token, "wifi") == 0) {
					map->bh_type = MAP_BH_WIFI;
				} else
					printf("\033[1;31m %s, %u \033[0m\n", __FUNCTION__, __LINE__);	/* Haipin Debug Print (R)*/

				DBGPRINT(RT_DEBUG_TRACE, "bh_type = %s\n", token);
			} else if (os_strcmp(token, "map_controller") == 0) {
				// TODO: "if map_controller= (empty), it will have error"
				token = strtok(NULL, "");
				map->is_ctrler = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "is_ctrler = %u\n", map->is_ctrler);
			} else if (os_strcmp(token, "map_agent") == 0) {
				token = strtok(NULL, "");
				map->is_agnt = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "is_agnt = %u\n", map->is_agnt);
			}  else if (os_strcmp(token, "map_root") == 0) {
				token = strtok(NULL, "");
				map->is_root = atoi(token);
				DBGPRINT(RT_DEBUG_TRACE, "is_root = %u\n", map->is_root);
			}  else if (os_strcmp(token, "br_inf") == 0) {
				token = strtok(NULL, "");
				strcpy(map->br_iface, token);
				DBGPRINT(RT_DEBUG_TRACE, "bridge interface is = %s\n", map->br_iface);
			}  else if (os_strcmp(token, "map_ver") == 0) {
				token = strtok(NULL, "");
				map->map_version = 1;
				if (os_strcmp(token, "R1") == 0)
					map->map_version = 1;
				else if (os_strcmp(token, "R2") == 0)
					map->map_version = 2;
				DBGPRINT(RT_DEBUG_TRACE, "map_ver = %d\n", map->map_version);
			} else if (os_strcmp(token, "radio_band") == 0) {
				u8 ra_band_idx = 0;
				token = strtok(NULL, ";");
				while (token != NULL && ra_band_idx < MAP_MAX_RADIO) {
					if (os_strcmp(token, "24G") == 0)
						map->radio_band_options[ra_band_idx] = RADIO_24G;
					else if (os_strcmp(token, "5GL") == 0)
						map->radio_band_options[ra_band_idx] = RADIO_5GL;
					else if (os_strcmp(token, "5GH") == 0)
						map->radio_band_options[ra_band_idx] = RADIO_5GH;
					else if (os_strcmp(token, "5G") == 0)
						map->radio_band_options[ra_band_idx] = RADIO_5G;

					DBGPRINT(RT_DEBUG_TRACE, "radio_band_options[%u] = %u\n",
								ra_band_idx,
								map->radio_band_options[ra_band_idx]);
					ra_band_idx++;
					token = strtok(NULL, ";");
				}
			}
		}
	}

	fclose(file);
	return WAPP_SUCCESS;
}
#ifdef MAP_R2

void map_r2_cap_init(struct r2_ap_cap *r2_cap) 
{
	memset(r2_cap, 0, sizeof(struct r2_ap_cap));

	r2_cap->byte_counter_units = 0x01;
	r2_cap->max_total_num_sp_rules = 6;
	r2_cap->basic_sp_flag = 1;
	r2_cap->enhanced_sp_flag = 1;
	r2_cap->max_total_num_vid = 2;
}
#endif
int map_init(struct map_info *map)
{
	u8 fk_mac[] = {0x00,0x0c,0x43,0x11,0x22,0x33};
	char value[10] = {0};
#ifdef DFS_CAC_R2
	int i=0;
#endif

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	map->radio_band_options[0] = RADIO_24G;
	map->radio_band_options[1] = RADIO_5GL;
	map->radio_band_options[2] = RADIO_5GH;

#if 1
	if (map_read_config_file(map) != WAPP_SUCCESS)
		printf("\033[1;31m %s, %u \033[0m\n", __FUNCTION__, __LINE__);  /* Haipin Debug Print (R)*/
#endif
	disable_dhcp_client(map->br_iface);
#if 1 /* temp for WTS*/
	map->fh_radio_supt = RADIO_24G;
	map->bh_sta_radio = RADIO_5GL;
	COPY_MAC_ADDR(map->fh_24g_bssid, fk_mac);
	COPY_MAC_ADDR(map->fh_5g1_bssid, fk_mac);
	COPY_MAC_ADDR(map->fh_5g2_bssid, fk_mac);
	map->ht_24g_supt = 1; /*0 - not supported, 1 - supported*/
	map->he_24g_supt = 0;
	map->ht_5g1_supt = 1;
	map->vht_5g1_supt = 1;
	map->he_5g1_supt = 0;
	map->ht_5g2_supt = 0;
	map->vht_5g2_supt = 0;
	map->he_5g2_supt = 0;
#endif
	/*Read TurnKey Enable From Nvram*/
	get_map_parameters(map, "MapMode", value, DRIVER_PARAM, sizeof(value));
	map->MapMode = strtol(value, NULL, 10);
	if(!strcmp(value,"1"))
		map->TurnKeyEnable = 1;
	else
		map->TurnKeyEnable = 0;

	map->sta_report_on_cop = TRUE;
	map->sta_report_not_cop = FALSE;
	map->rssi_steer = FALSE;

	map_bss_table_init(map);
	map_reset_conf_sm(map);
	map->conf = MAP_CONN_STATUS_UNCONF;
#ifdef MAP_R2
	//map->scan_capab = NULL;
//	map->ch_scan_req = NULL;
//	map->ch_scan_rep = NULL;
	
	//map->scan_capab_len = 0;
//	map->scan_req_len = 0;
	map->scan_policy = 0;
//	os_memset((void *)&map->ch_scan_state,0, sizeof(struct ch_scan_state_ctrl));
	map_r2_cap_init(&map->r2_ap_capab);
#ifdef DFS_CAC_R2
	map->cac_capab = NULL;
	map->cac_tlv = NULL;
	
	os_memset((void *)&map->cac_state,0, sizeof(struct cac_state_ctrl));
	for (i=0; i < MAP_MAX_RADIO; i++)
		map->cac_state.radio_state[i].state_cac = CAC_DONE;
	
#endif
#endif

	dl_list_init(&map->bh_link_list);
	map->bh_link_num = 0;
	return 0;
}


int map_build_wifi_tx_link_stats(
	struct wifi_app *wapp, struct wapp_sta *sta, char *evt_buf)
{
	struct evt *map_event = NULL;
	struct tx_link_stat_rsp *tx_link = NULL;
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (sta->sta_status == WAPP_STA_CONNECTED) {
		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_TX_LINK_STATISTICS;
		tx_link = (struct tx_link_stat_rsp *) map_event->buffer;
		tx_link->pkt_errs = sta->tx_packets_errors;
		tx_link->tx_pkts = sta->packets_sent;
		tx_link->mac_tp_cap = sta->downlink;
		tx_link->link_avail = sta->link_availability;
		tx_link->phyrate= 0;
		tx_link->tx_tp = sta->tx_tp;
		map_event->length = sizeof(struct tx_link_stat_rsp);
		send_pkt_len = sizeof(struct evt) + map_event->length;
	}
	return send_pkt_len;
}


int map_build_eth_tx_link_stats(
	struct wifi_app *wapp, char *msg_buf, char *evt_buf)
{
	struct evt *map_event = NULL;
	struct tx_link_stat_rsp *tx_link = NULL;
	struct wapp_conf *conf;
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_TX_LINK_STATISTICS;
	tx_link = (struct tx_link_stat_rsp *) map_event->buffer;

	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		DBGPRINT(RT_DEBUG_TRACE, "####conf->iface(%s) iface(%s)\n",conf->iface, "ra0");
		if (os_strcmp(conf->iface, "ra0") == 0) {
			break;
		}
	}
	tx_link->pkt_errs = 0;
	tx_link->tx_pkts = 1000;
	tx_link->mac_tp_cap = conf->metrics.dl_load;
	tx_link->link_avail = 100;
	tx_link->phyrate = conf->metrics.dl_speed;
	tx_link->tx_tp = 0;
	map_event->length = sizeof(struct tx_link_stat_rsp);
	send_pkt_len = sizeof(struct evt) + map_event->length;
	return send_pkt_len;
}

int map_build_wifi_rx_link_stats(
	struct wifi_app *wapp, struct wapp_sta *sta, char *evt_buf)
{
	struct evt *map_event = NULL;
	struct rx_link_stat_rsp *rx_link = NULL;
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (sta->sta_status == WAPP_STA_CONNECTED) {
		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_RX_LINK_STATISTICS;
		rx_link = (struct rx_link_stat_rsp *) map_event->buffer;
		rx_link->pkt_errs = sta->rx_packets_errors;
		rx_link->rx_pkts = sta->packets_received;
		rx_link->rssi = sta->uplink_rssi;
		rx_link->rx_tp = sta->rx_tp;
		map_event->length = sizeof(struct rx_link_stat_rsp);
		send_pkt_len = sizeof(struct evt) + map_event->length;
	}
	return send_pkt_len;

}


int map_build_eth_rx_link_stats(
	struct wifi_app *wapp, char *msg_buf, char *evt_buf)
{
	struct evt *map_event = NULL;
	struct rx_link_stat_rsp *rx_link = NULL;
	struct wapp_conf *conf;
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_RX_LINK_STATISTICS;
	rx_link = (struct rx_link_stat_rsp *) map_event->buffer;

	dl_list_for_each(conf, &wapp->conf_list, struct wapp_conf, list) {
		DBGPRINT(RT_DEBUG_TRACE, "####conf->iface(%s) iface(%s)\n",conf->iface, "ra0");
		if (os_strcmp(conf->iface, "ra0") == 0) {
			break;
		}
	}
	rx_link->pkt_errs = 0;
	rx_link->rx_pkts = 0;
	rx_link->rssi = 100;
	rx_link->rx_tp = 0;
	map_event->length = sizeof(struct rx_link_stat_rsp);
	send_pkt_len = sizeof(struct evt) + map_event->length;
	return send_pkt_len;
}

#ifdef MAP_R2
long int cal_power(int c)
{
	if (c == 0)
		return 1;
	return 1024 * cal_power(--c);
}
#endif

int map_build_assoc_sta_traffic_stats(
	struct wifi_app *wapp, char *evt_buf, u8 *radio_id)
{
	struct dl_list *dev_list;
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	struct wapp_sta *sta = NULL;
	struct evt *map_event = NULL;
	struct sta_traffic_stats *traffic_stats = NULL;
	struct stat_info * stats = NULL;
	int send_pkt_len = 0;
	int i;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_ALL_ASSOC_STA_TRAFFIC_STATS;
	traffic_stats = (struct sta_traffic_stats *) map_event->buffer;

	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
			ap = (struct ap_dev *)wdev->p_dev;
			MAP_GET_RADIO_IDNFER(wdev->radio, traffic_stats->identifier);
			/* Report only for the requested radio id */
			if (os_memcmp(traffic_stats->identifier, radio_id, ETH_ALEN))
				continue;
			for (i = 0; i < CLIENT_TABLE_SIZE; i++) {
				sta = ap->client_table[i];
				if (sta && sta->sta_status == WAPP_STA_CONNECTED) {
					stats = &traffic_stats->stats[traffic_stats->sta_cnt];
					COPY_MAC_ADDR(stats->mac, sta->mac_addr);
					stats->bytes_sent = sta->bytes_sent;
					stats->bytes_received = sta->bytes_received;
					stats->packets_sent = sta->packets_sent;
					stats->packets_received = sta->packets_received;
					stats->tx_packets_errors = sta->tx_packets_errors;
					stats->rx_packets_errors = sta->rx_packets_errors;
					stats->retransmission_count = sta->retransmission_count;
					stats->is_APCLI = sta->is_APCLI;
					traffic_stats->sta_cnt++;
				}
			}
		}
	}

	os_memcpy(traffic_stats->identifier, radio_id, ETH_ALEN);
	map_event->length = sizeof(struct sta_traffic_stats) + traffic_stats->sta_cnt * sizeof(struct stat_info);
	send_pkt_len = sizeof(struct evt) + map_event->length;
	return send_pkt_len;
}

int map_build_one_assoc_sta_traffic_stats(
	struct wifi_app *wapp, struct wapp_sta *sta, char *evt_buf)
{
	struct evt *map_event = NULL;
	struct stat_info *stats = NULL;
#ifdef MAP_R2
	unsigned char byte_cnt = 0;
#endif
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
#ifdef MAP_R2
	if (wapp->map->map_version > 1)
		byte_cnt = wapp->map->r2_ap_capab.byte_counter_units;
#endif
	if (sta->sta_status == WAPP_STA_CONNECTED) {
		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_ONE_ASSOC_STA_TRAFFIC_STATS;
		stats = (struct stat_info *) map_event->buffer;

		COPY_MAC_ADDR(stats->mac, sta->mac_addr);
#ifdef MAP_R2
		// TODO: should depend on whether the controller is R2 or not.
		stats->bytes_sent = sta->bytes_sent/cal_power(byte_cnt);
		stats->bytes_received = sta->bytes_received/cal_power(byte_cnt);
#else
		stats->bytes_sent = sta->bytes_sent;
		stats->bytes_received = sta->bytes_received;
#endif
		stats->packets_sent = sta->packets_sent;
		stats->packets_received = sta->packets_received;
		stats->tx_packets_errors = sta->tx_packets_errors;
		stats->rx_packets_errors = sta->rx_packets_errors;
		stats->retransmission_count = sta->retransmission_count;

		map_event->length = sizeof(struct stat_info);
		send_pkt_len = sizeof(struct evt) + map_event->length;
	}

	return send_pkt_len;
}

int map_build_assoc_sta_tp_metric(
	struct wifi_app *wapp, char *evt_buf, u8 *radio_id)
{
	struct dl_list *dev_list;
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	struct wapp_sta *sta = NULL;
	struct evt *map_event = NULL;
	struct sta_tp_metrics *tp_metrics_info = NULL;
	struct tp_metrics * tp_metric = NULL;
	int send_pkt_len = 0;
	int i;
	DBGPRINT(RT_DEBUG_TRACE, "%s TRACE_NM\n", __func__);

	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_ALL_ASSOC_TP_METRICS;
	tp_metrics_info = (struct sta_tp_metrics *) map_event->buffer;

	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
			ap = (struct ap_dev *)wdev->p_dev;
			MAP_GET_RADIO_IDNFER(wdev->radio, tp_metrics_info->identifier);
			if (os_memcmp(tp_metrics_info->identifier, radio_id, ETH_ALEN))
				continue;
			for (i = 0; i < CLIENT_TABLE_SIZE; i++) {
				sta = ap->client_table[i];
				if (sta && sta->sta_status == WAPP_STA_CONNECTED) {
					tp_metric = &tp_metrics_info->info[tp_metrics_info->sta_cnt];
					COPY_MAC_ADDR(tp_metric->mac, sta->mac_addr);
					COPY_MAC_ADDR(tp_metric->bssid, sta->bssid);
					tp_metric->tx_tp = sta->tx_tp;
					tp_metric->rx_tp = sta->rx_tp;
					tp_metric->is_APCLI = sta->is_APCLI;
					tp_metrics_info->sta_cnt++;
				}
			}
		}
	}

	map_event->length = sizeof(struct sta_tp_metrics) + tp_metrics_info->sta_cnt * sizeof(struct tp_metrics);
	send_pkt_len = sizeof(struct evt) + map_event->length;
	return send_pkt_len;
}
int map_build_assoc_sta_link_metric(
	struct wifi_app *wapp, char *evt_buf, u8 *radio_id)
{
	struct dl_list *dev_list;
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	struct wapp_sta *sta = NULL;
	struct evt *map_event = NULL;
	struct sta_link_metrics *sta_metrics = NULL;
	struct link_metrics * metric = NULL;
	struct os_time now, delta;
	int send_pkt_len = 0;
	int i;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_ALL_ASSOC_STA_LINK_METRICS;
	sta_metrics = (struct sta_link_metrics *) map_event->buffer;

	os_get_time(&now);

	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
			ap = (struct ap_dev *)wdev->p_dev;
			MAP_GET_RADIO_IDNFER(wdev->radio, sta_metrics->identifier);
			if (os_memcmp(sta_metrics->identifier, radio_id, ETH_ALEN))
				continue;
			for (i = 0; i < CLIENT_TABLE_SIZE; i++) {
				sta = ap->client_table[i];
				if (sta && sta->sta_status == WAPP_STA_CONNECTED) {
					metric = &sta_metrics->info[sta_metrics->sta_cnt];
					COPY_MAC_ADDR(metric->mac, sta->mac_addr);
					COPY_MAC_ADDR(metric->bssid, sta->bssid);
					metric->erate_downlink = sta->downlink;
					metric->erate_uplink = sta->uplink;
					metric->rssi_uplink = sta->uplink_rssi;
					metric->is_APCLI = sta->is_APCLI;
					os_time_sub(&now, &sta->last_update_time, &delta);
					metric->time_delta = delta.sec * 1000 + delta.usec / 1000;
					sta_metrics->sta_cnt++;
				}
			}
		}
	}

	map_event->length = sizeof(struct sta_link_metrics) + sta_metrics->sta_cnt * sizeof(struct link_metrics);
	send_pkt_len = sizeof(struct evt) + map_event->length;
	return send_pkt_len;
}

#ifdef MAP_R2
int map_build_assoc_sta_ext_metric(
	struct wifi_app *wapp, char *evt_buf, u8 *radio_id)
{
	struct dl_list *dev_list;
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	struct wapp_sta *sta = NULL;
	struct evt *map_event = NULL;
	struct ext_sta_link_metrics *sta_metrics = NULL;
	struct ext_link_metrics * metric = NULL;
	int send_pkt_len = 0;
	int i;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_ALL_ASSOC_STA_EXTENDED_LINK_METRICS;
	sta_metrics = (struct ext_sta_link_metrics *) map_event->buffer;


	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
			ap = (struct ap_dev *)wdev->p_dev;
			MAP_GET_RADIO_IDNFER(wdev->radio, sta_metrics->identifier);
			if (os_memcmp(sta_metrics->identifier, radio_id, ETH_ALEN))
				continue;
			for (i = 0; i < CLIENT_TABLE_SIZE; i++) {
				sta = ap->client_table[i];
				if (sta && sta->sta_status == WAPP_STA_CONNECTED) {
					DBGPRINT(RT_DEBUG_TRACE, "WAPP_ALL_ASSOC_STA_EXTENDED_LINK_METRICS to MAPD\n");
					metric = &sta_metrics->info[sta_metrics->sta_cnt];
					COPY_MAC_ADDR(metric->mac, sta->mac_addr);
					COPY_MAC_ADDR(metric->bssid, sta->bssid);
					metric->last_data_dl_rate = sta->ext_sta_metrics.sta_info.last_data_dl_rate;
					metric->last_data_ul_rate = sta->ext_sta_metrics.sta_info.last_data_ul_rate;
					metric->utilization_rx = sta->ext_sta_metrics.sta_info.utilization_rx;
					metric->utilization_tx = sta->ext_sta_metrics.sta_info.utilization_tx;
					//printf("dl rate: %d, ul rate: %d\n", sta->ext_sta_metrics.sta_info.last_data_dl_rate, sta->ext_sta_metrics.sta_info.last_data_ul_rate);
					//printf("rx rate: %d, tx rate: %d\n", sta->ext_sta_metrics.sta_info.utilization_rx, sta->ext_sta_metrics.sta_info.utilization_tx);
					//printf("dl rate: %d, ul rate: %d\n", metric->last_data_dl_rate, metric->last_data_ul_rate);
					//printf("rx rate: %d, tx rate: %d\n", metric->utilization_rx, metric->utilization_tx);
					sta_metrics->sta_cnt++;
				}
			}
		}
	}

	map_event->length = sizeof(struct ext_sta_link_metrics) + sta_metrics->sta_cnt * sizeof(struct ext_link_metrics);
	send_pkt_len = sizeof(struct evt) + map_event->length;
	//printf("send pkt len: %d\n", send_pkt_len);
	return send_pkt_len;
}
int map_build_one_assoc_sta_ext_link_metric(
	struct wifi_app *wapp, struct wapp_sta *sta, char *evt_buf)
{
	struct evt *map_event = NULL;
	struct ext_link_metrics * metric = NULL;
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	if (sta->sta_status == WAPP_STA_CONNECTED) {
		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_ONE_ASSOC_STA_EXTENDED_LINK_METRICS;
		metric = (struct ext_link_metrics *) map_event->buffer;
		printf("sending one assoc info to mapd\n");
		COPY_MAC_ADDR(metric->mac, sta->mac_addr);
		COPY_MAC_ADDR(metric->bssid, sta->bssid);
		metric->last_data_dl_rate = sta->ext_sta_metrics.sta_info.last_data_dl_rate;
		metric->last_data_ul_rate = sta->ext_sta_metrics.sta_info.last_data_ul_rate;
		metric->utilization_rx = sta->ext_sta_metrics.sta_info.utilization_rx;
		metric->utilization_tx = sta->ext_sta_metrics.sta_info.utilization_tx;
		map_event->length = sizeof(struct ext_link_metrics);
		send_pkt_len = sizeof(struct evt) + map_event->length;
	}
	return send_pkt_len;
}

#endif

int map_build_one_assoc_sta_link_metric(
	struct wifi_app *wapp, struct wapp_sta *sta, char *evt_buf)
{
	struct evt *map_event = NULL;
	struct link_metrics * metric = NULL;
	struct os_time now, delta;
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (sta->sta_status == WAPP_STA_CONNECTED) {
		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_ONE_ASSOC_STA_LINK_METRICS;
		metric = (struct link_metrics *) map_event->buffer;
		os_get_time(&now);

		COPY_MAC_ADDR(metric->mac, sta->mac_addr);
		COPY_MAC_ADDR(metric->bssid, sta->bssid);
		metric->erate_downlink = sta->downlink;
		metric->erate_uplink = sta->uplink;
		metric->rssi_uplink = sta->uplink_rssi;
		os_time_sub(&now, &sta->last_update_time, &delta);
		metric->time_delta = delta.sec * 1000 + delta.usec / 1000;
		map_event->length = sizeof(struct link_metrics);
		send_pkt_len = sizeof(struct evt) + map_event->length;
	}
	return send_pkt_len;
}


int map_build_unassoc_sta_link_metrics(
	struct wifi_app *wapp, char *msg_buf, char *evt_buf)
{
	struct unlink_metrics_query *query;
	struct evt *map_event = NULL;
	struct unlink_metrics_rsp * metric = NULL;
	struct unlink_rsp_sta *info;
	struct probe_info * probe;
	struct os_time now, delta;
	int send_pkt_len = 0;
	int i, j;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	query = (struct unlink_metrics_query *)msg_buf;
	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_UNASSOC_STA_LINK_METRICS;
	metric = (struct unlink_metrics_rsp *) map_event->buffer;
	metric->oper_class = query->oper_class;

	os_get_time(&now);

	for (i = 0; i < query->sta_num; i++) {
		probe = wapp_probe_lookup(wapp, query->sta_list + i*MAC_ADDR_LEN);
		if (probe) {
			//check channel
			for (j = 0; j < query->ch_num; j++) {
				if (probe->channel == query->ch_list[j])
					break;
			}
			if (j < query->ch_num) { //found channel
				info = &metric->info[metric->sta_num];
				COPY_MAC_ADDR(info->mac, probe->mac_addr);
				info->ch = probe->channel;
				os_time_sub(&now, &probe->last_update_time, &delta);
				info->time_delta = delta.sec * 1000 + delta.usec / 1000;
				info->uplink_rssi = probe->rssi;
				metric->sta_num++;
			}
		}
	}

	map_event->length = sizeof(struct unlink_metrics_rsp) + metric->sta_num * sizeof(struct unlink_rsp_sta);
	send_pkt_len = sizeof(struct evt) + map_event->length;
	return send_pkt_len;
}


int map_build_ap_metric(
	struct wifi_app *wapp, struct ap_dev *ap, char *evt_buf)
{
	wdev_ap_metric *ap_metrics = NULL;
	struct evt *map_event = NULL;
	struct ap_metrics_info *metrics = NULL;
	int send_pkt_len = 0;
	int i;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	ap_metrics = &ap->ap_metrics;
	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_AP_METRICS_INFO;
	metrics = (struct ap_metrics_info *)map_event->buffer;
	COPY_MAC_ADDR(metrics->bssid, ap_metrics->bssid);
	metrics->ch_util = ap_metrics->cu;
	metrics->assoc_sta_cnt = ap->num_of_assoc_cli;
	metrics->valid_esp_count = AC_NUM;
#ifdef MAP_R2
	metrics->ext_ap_metric.bc_rx = ap_metrics->ext_ap_metric.bc_rx;
	metrics->ext_ap_metric.bc_tx = ap_metrics->ext_ap_metric.bc_tx;
	metrics->ext_ap_metric.mc_rx = ap_metrics->ext_ap_metric.mc_rx;
	metrics->ext_ap_metric.mc_tx = ap_metrics->ext_ap_metric.mc_tx;
	metrics->ext_ap_metric.uc_rx = ap_metrics->ext_ap_metric.uc_rx;
	metrics->ext_ap_metric.uc_tx = ap_metrics->ext_ap_metric.uc_tx;
#endif
	for (i = 0; i < AC_NUM; i++) {
		metrics->esp[i].ac = ap_metrics->ESPI_AC[i][0] & 0x03;
		metrics->esp[i].format = (ap_metrics->ESPI_AC[i][0] & 0x18) >> 3;
		metrics->esp[i].ba_win_size = (ap_metrics->ESPI_AC[i][0] & 0xe0) >> 5;
		metrics->esp[i].e_air_time_fraction = ap_metrics->ESPI_AC[i][1];
		metrics->esp[i].ppdu_dur_target = ap_metrics->ESPI_AC[i][2];
	}

	map_event->length = sizeof(struct ap_metrics_info) + metrics->valid_esp_count*sizeof(struct esp_info);
	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}
#ifdef MAP_R2
int map_build_radio_metric(
	struct wifi_app *wapp, wapp_event_data *event_data, char *evt_buf, u32 ifindex)
{
	wdev_radio_metric *radio_metrics = NULL;
	struct evt *map_event = NULL;
	struct radio_metrics_info *metrics = NULL;
	int send_pkt_len = 0;
	struct wapp_dev *wdev = NULL;
	u8 ra_identifier[ETH_ALEN];
	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	MAP_GET_RADIO_IDNFER(wdev->radio,ra_identifier);
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	//printf("###WAPP### %s ###\n", __func__);
	radio_metrics = &event_data->radio_metrics;	
	//printf("###WAPP### %s ###\n", __func__);
	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_RADIO_METRICS_INFO;
	metrics = (struct radio_metrics_info *)map_event->buffer;
	metrics->cu_noise = radio_metrics->cu_noise;
	metrics->cu_tx = radio_metrics->cu_tx;
	metrics->cu_rx = radio_metrics->cu_rx;
	metrics->cu_other = radio_metrics->cu_other;
	metrics->edcca = radio_metrics->edcca;
	//printf("###WAPP### %s ### map_event->type = %d\n", __func__, map_event->type);
	//printf("###WAPP### %s ### metrics->cu_noise = %d\n", __func__, metrics->cu_noise);
	//printf("###WAPP### %s ### metrics->cu_tx = %d\n", __func__, metrics->cu_tx);
	//printf("###WAPP### %s ### metrics->cu_rx = %d\n", __func__, metrics->cu_rx);
	//printf("###WAPP### %s ### metrics->cu_other = %d\n", __func__, metrics->cu_other);

	COPY_MAC_ADDR(metrics->ra_id, ra_identifier);

	DBGPRINT(RT_DEBUG_TRACE,
	"idfer	=	%02x%02x%02x%02x%02x%02x\n",
	PRINT_RA_IDENTIFIER(ra_identifier)
	);

	
	DBGPRINT(RT_DEBUG_TRACE,
	"idfer	=	%02x%02x%02x%02x%02x%02x\n",
	PRINT_RA_IDENTIFIER(metrics->ra_id)
	);
	map_event->length = sizeof(struct radio_metrics_info);
	send_pkt_len = sizeof(*map_event) + map_event->length;
	
	return send_pkt_len;
}
#endif

int set_ch_pref_val(
	struct wifi_app *wapp,
	wdev_chn_info *ch_info)
{
//TBD
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	return PREF_SCORE_14;
}

BOOLEAN is_primary_channel_operable(unsigned char channel)
{
	int i = 0;

	for (i = 0; i < sizeof(channel_operable_status)/sizeof(channel_operable_status[0]); i++) {
		if (channel_operable_status[i].channel_num == channel) {
			return channel_operable_status[i].operable_status;
		}
	}
	return TRUE;
}

void update_primary_ch_status(unsigned char channel, unsigned char status)
{
	int i = 0;

	for (i = 0; i < sizeof(channel_operable_status)/sizeof(channel_operable_status[0]); i++) {
		if (channel_operable_status[i].channel_num == channel) {
			channel_operable_status[i].operable_status = status;
		}
	}
}
BOOLEAN is_ch_operable(unsigned char channel, unsigned char opclass)
{
	int i = 0;
	int j = 0;
	if (is_primary_channel_operable(channel)) {
		for (i = 0; i < sizeof(channel_dependency_list)/sizeof(channel_dependency_list[0]);
			i++) {
			if (channel_dependency_list[i].channel_num == channel &&
				channel_dependency_list[i].opclass == opclass)
			{
				for (j = 0; j < MAX_NUM_DEPENDENT_CHANNELS; j++) {
					if (!is_primary_channel_operable(
						channel_dependency_list[i].dependent_channel_set[j])) {
						return FALSE;
					}
				}
				return TRUE;
			}
		}
	} else {
		return FALSE;
	}
	return TRUE;
}

#ifdef WIFI_MD_COEX_SUPPORT
void dump_operable_channel()
{
	int i = 0;

	for (i = 0; i < sizeof(channel_operable_status)/sizeof(channel_operable_status[0]); i++) {
		DBGPRINT(RT_DEBUG_ERROR, "channel=%d operable=%d\n", channel_operable_status[i].channel_num,
			channel_operable_status[i].operable_status);
	}
}
#endif

int map_build_chn_pref(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf)
{
	u8 i = 0, j = 0;
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	struct evt *map_event = NULL;
	int send_pkt_len = 0;
	struct ch_prefer *ch_pref = NULL;
	struct prefer_info *op_info = NULL;
	wdev_op_class_info *op_class = NULL;
	unsigned char channel_count = 0;
	unsigned char opclass_count = 0;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)addr);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return 0;
	}

	if(wdev->dev_type == WAPP_DEV_TYPE_AP) {
		ap = (struct ap_dev *)wdev->p_dev;
		op_class = &ap->op_class;

		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_CHANNLE_PREFERENCE;

		ch_pref = (struct ch_prefer*)  map_event->buffer;
		if (wdev->radio) {
			MAP_GET_RADIO_IDNFER(wdev->radio, ch_pref->identifier);
		}

		op_info = ch_pref->opinfo;
		for (i = 0; i < op_class->num_of_op_class; i++) {
			channel_count = 0;
			if (OP_CLASS_MATCH_RADIO_CONF(*wdev->radio->radio_band, op_class->opClassInfo[i].op_class)) {

				for (j = 0; j < op_class->opClassInfo[i].num_of_ch; j++) {
					if (is_ch_operable(op_class->opClassInfo[i].ch_list[j],
						op_class->opClassInfo[i].op_class)) {
						op_info->ch_list[channel_count] =
							op_class->opClassInfo[i].ch_list[j];
						channel_count++;
					}
				}
				if (channel_count) {
					op_info->op_class = op_class->opClassInfo[i].op_class;
					op_info->ch_num = channel_count;
					op_info->perference = PREF_SCORE_14;
					op_info->reason = 0;
					opclass_count++;
					op_info++;

				}
			}
		}

		for (i = 0; i < op_class->num_of_op_class; i++) {
			channel_count = 0;
			if (OP_CLASS_MATCH_RADIO_CONF(*wdev->radio->radio_band, op_class->opClassInfo[i].op_class)) {

				for (j = 0; j < op_class->opClassInfo[i].num_of_ch; j++) {
					if (!is_ch_operable(op_class->opClassInfo[i].ch_list[j],
						op_class->opClassInfo[i].op_class)) {
						op_info->ch_list[channel_count] =
							op_class->opClassInfo[i].ch_list[j];
						channel_count++;
					}
				}
				if (channel_count) {
					op_info->op_class = op_class->opClassInfo[i].op_class;
					op_info->ch_num = channel_count;
					op_info->perference = PREF_SCORE_0;
					op_info->reason = 0x7;//! RADAR Detected
					opclass_count++;
					op_info++;
				}
			}
		}

		ch_pref->op_class_num = opclass_count;

	}
	map_event->length = sizeof(struct ch_prefer)
						+ sizeof(struct prefer_info) * opclass_count;

	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}


int map_build_ra_op_restrict(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf)
{
	u8 i = 0, j = 0;
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	struct evt *map_event = NULL;
	int send_pkt_len = 0;
	struct restriction *rest = NULL;
	struct restrict_info *opinfo = NULL;
	wdev_op_class_info *op_class = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)addr);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return 0;
	}

	if(wdev->dev_type == WAPP_DEV_TYPE_AP) {
		ap = (struct ap_dev *)wdev->p_dev;
		op_class = &ap->op_class;

		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_RADIO_OPERATION_RESTRICTION;
		map_event->length = sizeof(struct restriction)
							+ (op_class->num_of_op_class * sizeof(struct restrict_info));

		rest = (struct restriction *) map_event->buffer;
		if (wdev->radio) {
			MAP_GET_RADIO_IDNFER(wdev->radio, rest->identifier);
		}
		rest->op_class_num = op_class->num_of_op_class;

		opinfo = rest->opinfo;
		for(i = 0; i < op_class->num_of_op_class; i++) {
			opinfo->op_class = op_class->opClassInfo[i].op_class;
			opinfo->ch_num = op_class->opClassInfo[i].num_of_ch;
			for(j = 0; j < opinfo->ch_num; j++) {
				opinfo->ch_list[j] = op_class->opClassInfo[i].ch_list[j];
				opinfo->fre_separation[j] = 0;
			}
			opinfo++;
		}
	}

	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}


int map_build_ap_op_bss(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf)
{
	int i = 0;
	struct wapp_dev *wdev = NULL;
	struct evt *map_event = NULL;
	int send_pkt_len = 0;

	struct oper_bss_cap *op_bss_cap = NULL;
	struct op_bss_cap *cap = NULL;
	unsigned char wdev_identifier[ETH_ALEN];
	struct dl_list *dev_list;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dev_list = &wapp->dev_list;

	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_OPERBSS_REPORT;
	op_bss_cap = (struct oper_bss_cap *) map_event->buffer;

	os_memcpy(op_bss_cap->identifier, addr, ETH_ALEN);
	cap = op_bss_cap->cap;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
		if(wdev && wdev->radio) {
			struct map_conf_state *conf_state = &wdev->radio->conf_state;

			if(IS_CONF_STATE(conf_state, MAP_CONF_STOP))
				continue;
#ifdef WIFI_MD_COEX_SUPPORT
			/*all bss has been stopped if no operatable channel on current radio*/
			if (!wdev->radio->operatable) {
				DBGPRINT(RT_DEBUG_OFF, "itf_mac(%02x:%02x:%02x:%02x:%02x:%02x)'s radio is not operatable\n",
							PRINT_MAC(wdev->mac_addr));
				continue;
			}
#endif
			MAP_GET_RADIO_IDNFER(wdev->radio, wdev_identifier);
			if(!os_memcmp(wdev_identifier, (char *)addr, ETH_ALEN) &&
				(wdev->dev_type == WAPP_DEV_TYPE_AP)) {
					struct ap_dev * ap = (struct ap_dev *)wdev->p_dev;
					if (ap->isActive == WAPP_BSS_START) {
						if (wdev->radio->radio_band)
							op_bss_cap->band = *(wdev->radio->radio_band);
						os_memcpy(cap->bssid, wdev->mac_addr, MAC_ADDR_LEN);
						cap->ssid_len = strlen(ap->bss_info.ssid);
						cap->map_vendor_extension = ap->bss_info.map_role;
						wdev->i_am_fh_bss = (ap->bss_info.map_role & (1<<MAP_ROLE_FRONTHAUL_BSS))?1:0;
						wdev->i_am_bh_bss = (ap->bss_info.map_role & (1<<MAP_ROLE_BACKHAUL_BSS))?1:0;
						os_memcpy(cap->ssid, ap->bss_info.ssid, cap->ssid_len);
						cap->auth_mode = ap->bss_info.auth_mode;
						cap->enc_type = ap->bss_info.enc_type;
						cap->key_len = ap->bss_info.key_len;
						os_memcpy(cap->key, ap->bss_info.key, ap->bss_info.key_len);
						DBGPRINT(RT_DEBUG_TRACE, "%s opbss(%s)\n",
							__func__, ap->bss_info.ssid);
						DBGPRINT(RT_DEBUG_TRACE, "itf_mac(%02x:%02x:%02x:%02x:%02x:%02x)\n",
							PRINT_MAC(wdev->mac_addr));
						i++;
						cap++;
					}
			}
		}
	}
	op_bss_cap->oper_bss_num = i;
	map_event->length = OPER_BSS_CAP_LEN + (sizeof(struct op_bss_cap) * i);

	DBGPRINT(RT_DEBUG_INFO, "%s oper_bss_num=%d, band=%d\n", __func__, op_bss_cap->oper_bss_num, op_bss_cap->band);
	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}

int map_build_ap_ra_basic_cap(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf)
{
	int i = 0;//, j = 0;
	int send_pkt_len = 0;
	struct ap_dev *ap = NULL;
	struct wapp_dev *wdev = NULL;
	struct evt *map_event = NULL;
	struct ap_radio_basic_cap *apRaCap = NULL;
	struct radio_basic_cap *opcap = NULL;
	wdev_op_class_info *op_class = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)addr);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return 0;
	}
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		u8 num_map_op_class = 0;
		u8 radio_band = 0;
		ap = (struct ap_dev *)wdev->p_dev;

		op_class = &ap->op_class;
		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_RADIO_BASIC_CAP;


		apRaCap = (struct ap_radio_basic_cap *)map_event->buffer;
		if (wdev->radio) {
			MAP_GET_RADIO_IDNFER(wdev->radio, apRaCap->identifier);
			if (wdev->radio->radio_band)
				radio_band = *wdev->radio->radio_band;
		}
		apRaCap->max_bss_num = ap->max_num_of_bss;
		apRaCap->band = 0; //Un-used
		apRaCap->wireless_mode = wdev->wireless_mode;

		opcap = apRaCap->opcap;
		for (i = 0; i < op_class->num_of_op_class; i++) {
			if (OP_CLASS_MATCH_RADIO_CONF(radio_band, op_class->opClassInfo[i].op_class))
			{
			opcap->op_class = op_class->opClassInfo[i].op_class;
			opcap->max_tx_pwr = ap->pwr.tx_pwr;
			opcap->non_operch_num = 0;
			//for(j = 0; j < ch_info->non_op_chn_num; j++) {
			//    opcap->non_operch_list[j] = ch_info->non_op_ch_list[j];
			//}
			opcap++;
				num_map_op_class++;
			}
		}
		map_event->length = sizeof(struct ap_radio_basic_cap)
					+ (num_map_op_class* sizeof(struct radio_basic_cap));
		apRaCap->op_class_num = num_map_op_class;

	}

	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}


int map_build_ap_cap(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf)
{
	struct evt *map_event = NULL;
	struct ap_capability *ap_cap = NULL;
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_AP_CAPABILITY;
	map_event->length = sizeof(struct ap_capability);
	ap_cap = (struct ap_capability *)map_event->buffer;

	if (wapp->map->sta_report_on_cop)
		ap_cap->sta_report_on_cop = 1;
	if (wapp->map->sta_report_not_cop)
		ap_cap->sta_report_not_cop = 1;
	if (wapp->map->rssi_steer)
		ap_cap->rssi_steer = 1;

	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}

int map_build_ap_ht_cap(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf)
{
	struct wapp_dev *wdev = NULL;
	struct evt *map_event = NULL;
	struct ap_dev *ap = NULL;
	wdev_ht_cap *ht_cap = NULL;
	int send_pkt_len = 0;
	struct ap_ht_capability *map_ht_cap = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)addr);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return 0;
	}

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		ap = (struct ap_dev *)wdev->p_dev;
		ht_cap = &ap->ht_cap;

		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_AP_HT_CAPABILITY;
		map_event->length = sizeof(struct ap_ht_capability);
		map_ht_cap = (struct ap_ht_capability *)map_event->buffer;
		if (wdev->radio) {
			MAP_GET_RADIO_IDNFER(wdev->radio, map_ht_cap->identifier);
		}

		switch (ht_cap->tx_stream) {
			case 1:
				map_ht_cap->tx_stream = 0;
				break;
			case 2:
				map_ht_cap->tx_stream = 1;
				break;
			case 3:
				map_ht_cap->tx_stream = 2;
				break;
			case 4:
				map_ht_cap->tx_stream = 3;
				break;
			default:
				printf("%s: - Unknow HT Tx Spatial Stream %d!! \n", __func__,ht_cap->rx_stream);
				map_ht_cap->tx_stream = 0;  //TODO, check why
				break;
		}

		switch (ht_cap->rx_stream) {
			case 1:
				map_ht_cap->rx_stream = 0;
				break;
			case 2:
				map_ht_cap->rx_stream = 1;
				break;
			case 3:
				map_ht_cap->rx_stream = 2;
				break;
			case 4:
				map_ht_cap->rx_stream = 3;
				break;
			default:
				printf("%s: - Unknow HT Rx Spatial Stream!! %d\n", __func__,ht_cap->rx_stream);
				map_ht_cap->rx_stream = 0;  //TODO, check why
				break;
		}

		map_ht_cap->sgi_20 = (ht_cap->sgi_20) ? 1 : 0;
		map_ht_cap->sgi_40 = (ht_cap->sgi_40) ? 1 : 0;
		map_ht_cap->ht_40 = (ht_cap->ht_40) ? 1 : 0;
	}

	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}


int map_build_ap_vht_cap(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf)
{
	struct wapp_dev *wdev = NULL;
	struct evt *map_event = NULL;
	struct ap_dev *ap = NULL;
	wdev_vht_cap *vht_cap = NULL;
	int send_pkt_len = 0;
	struct ap_vht_capability *map_vht_cap;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)addr);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return 0;
	}

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		ap = (struct ap_dev *)wdev->p_dev;
		vht_cap = &ap->vht_cap;

		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_AP_VHT_CAPABILITY;
		map_event->length = sizeof(struct ap_vht_capability);
		map_vht_cap = (struct ap_vht_capability *)map_event->buffer;

		if (wdev->radio) {
			MAP_GET_RADIO_IDNFER(wdev->radio, map_vht_cap->identifier);
		}
		os_memcpy(&map_vht_cap->vht_tx_mcs, vht_cap->sup_tx_mcs, sizeof(unsigned short));
		os_memcpy(&map_vht_cap->vht_rx_mcs, vht_cap->sup_rx_mcs, sizeof(unsigned short));

		switch (vht_cap->tx_stream) {
			case 1:
				map_vht_cap->tx_stream = 0;
				break;
			case 2:
				map_vht_cap->tx_stream = 1;
				break;
			case 3:
				map_vht_cap->tx_stream = 2;
				break;
			case 4:
				map_vht_cap->tx_stream = 3;
				break;
			case 5:
				map_vht_cap->tx_stream = 4;
				break;
			case 6:
				map_vht_cap->tx_stream = 5;
				break;
			case 7:
				map_vht_cap->tx_stream = 6;
				break;
			case 8:
				map_vht_cap->tx_stream = 7;
				break;
			default:
				printf("%s: - Unknow VHT Tx Spatial Stream!! \n", __FUNCTION__);
				break;
		}

		switch (vht_cap->rx_stream) {
			case 1:
				map_vht_cap->rx_stream = 0;
				break;
			case 2:
				map_vht_cap->rx_stream = 1;
				break;
			case 3:
				map_vht_cap->rx_stream = 2;
				break;
			case 4:
				map_vht_cap->rx_stream = 3;
				break;
			case 5:
				map_vht_cap->rx_stream = 4;
				break;
			case 6:
				map_vht_cap->rx_stream = 5;
				break;
			case 7:
				map_vht_cap->rx_stream = 6;
				break;
			case 8:
				map_vht_cap->rx_stream = 7;
				break;
			default:
				printf("%s: - Unknow VHT Rx Spatial Stream!! \n", __FUNCTION__ );
				break;
		}

		map_vht_cap->sgi_80 = (vht_cap->sgi_80) ? 1 : 0;
		map_vht_cap->sgi_160 = (vht_cap->sgi_160) ? 1 : 0;
		map_vht_cap->vht_8080 = (vht_cap->vht_8080) ? 1 : 0;
		map_vht_cap->vht_160 = (vht_cap->vht_160) ? 1 : 0;
		map_vht_cap->su_beamformer = (vht_cap->su_bf) ? 1 : 0;
		map_vht_cap->mu_beamformer = (vht_cap->su_bf) ? 1 : 0;
	}

	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}


int map_build_assoc_cli(
	struct wifi_app *wapp,
	unsigned char *bss_addr,
	unsigned char *sta_addr,
	unsigned char stat,
	char *evt_buf)
{
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	struct wapp_sta *sta = NULL;
	struct evt *map_event = NULL;
	struct client_association_event *assoc_evt;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	int send_pkt_len = 0;

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bss_addr, WAPP_DEV_TYPE_AP);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return 0;
	}

	ap = (struct ap_dev *)wdev->p_dev;

	sta = wdev_ap_client_list_lookup(wapp, ap, sta_addr);
	if (sta) {
		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_CLIENT_NOTIFICATION;
		assoc_evt = (struct client_association_event *)map_event->buffer;
		assoc_evt->map_assoc_evt.assoc_evt = stat;
		os_memcpy(assoc_evt->map_assoc_evt.sta_mac, sta->mac_addr, MAC_ADDR_LEN);
		os_memcpy(assoc_evt->map_assoc_evt.bssid, sta->bssid, MAC_ADDR_LEN);
		assoc_evt->map_assoc_evt.assoc_time = sta->assoc_time;
		assoc_evt->map_assoc_evt.is_APCLI = sta->is_APCLI;
		os_memcpy(&assoc_evt->cli_caps, &sta->cli_caps, sizeof(struct map_priv_cli_cap));

		printf("\033[1;36m %s, %u!!!!!!!!!! stat 0x%x assoc_evt->assoc_evt 0x%x\033[0m\n", __FUNCTION__, __LINE__,stat,assoc_evt->map_assoc_evt.assoc_evt);
		if (stat == STA_LEAVE) {
			map_event->length = sizeof(struct client_association_event);
			assoc_evt->map_assoc_evt.assoc_req_len = 0;
		} else {
			map_event->length = sizeof(struct client_association_event) +  sta->assoc_req_len;
			assoc_evt->map_assoc_evt.assoc_req_len =  sta->assoc_req_len;
			os_memcpy(assoc_evt->map_assoc_evt.assoc_req, sta->assoc_req, sta->assoc_req_len);
		}
		send_pkt_len = sizeof(*map_event) + map_event->length;
	}

	return send_pkt_len;
}

#ifdef MAP_R2
int map_build_disassoc_stats(
	struct wifi_app *wapp,
	unsigned char *bss_addr,
	wapp_client_info *cli_info,
	char *evt_buf)
{
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;
	struct wapp_sta *sta = NULL;
	struct evt *map_event = NULL;
	struct client_disassociation_stats_event *disassoc_stats_evt;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);
	int send_pkt_len = 0;

	wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, bss_addr, WAPP_DEV_TYPE_AP);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return 0;
	}

	ap = (struct ap_dev *)wdev->p_dev;

	sta = wdev_ap_client_list_lookup(wapp, ap, cli_info->mac_addr);
	if (sta) {
		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_DISASSOC_STATS_EVT;
		disassoc_stats_evt = (struct client_disassociation_stats_event *)map_event->buffer;
		os_memcpy(disassoc_stats_evt->mac_addr, sta->mac_addr, ETH_ALEN);
		disassoc_stats_evt->reason_code = cli_info->disassoc_reason;
		map_event->length = sizeof(struct client_disassociation_stats_event);
		printf("disassoc_stats_evt->reason_code: %d\n", disassoc_stats_evt->reason_code);
		send_pkt_len = sizeof(*map_event) + map_event->length;
	}

	return send_pkt_len;
}

#endif

int map_get_config(
	struct wifi_app *wapp,
	struct wapp_radio *ra)
{
	struct wps_get_config *conf = NULL;
	u8 num_of_dev = 0, i;
	u16 data_len = 0;
	char *data = NULL;
	struct wapp_dev *wdev;
	struct dl_list *dev_list;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	/* alid ra_id mac bss info rf band */
	data_len = sizeof(struct wps_get_config);

	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev && wdev->radio == ra && wdev->dev_type == WAPP_DEV_TYPE_AP) {
			num_of_dev++;
		}
	}

	data_len += (num_of_dev * sizeof(inf_info));

	data = os_zalloc(data_len);
	if (data == NULL) {
		DBGPRINT_RAW(RT_DEBUG_OFF, "%s: buf alloc failed",__FUNCTION__);
		return WAPP_RESOURCE_ALLOC_FAIL;
	}

	conf = (struct wps_get_config *) data;
	conf->band = (ra->op_ch > 14) ? BAND_5G : BAND_24G;
	printf(BLUE("%s, for band %d.\n"), __FUNCTION__, conf->band);
	conf->dev_type = WAPP_DEV_TYPE_AP;
	conf->num_of_inf = num_of_dev;
	MAP_GET_RADIO_IDNFER(ra, conf->identifier);

	i = 0;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (i > num_of_dev) {
			DBGPRINT_RAW(RT_DEBUG_OFF, "%s: unexpected error!",__FUNCTION__);
			os_free(data);
			return WAPP_UNEXP;
		}

		if (wdev && wdev->radio == ra && wdev->dev_type == WAPP_DEV_TYPE_AP) {
			os_memcpy(conf->inf_data[i].ifname, wdev->ifname, IFNAMSIZ);
			os_memcpy(conf->inf_data[i].mac_addr, wdev->mac_addr, MAC_ADDR_LEN);
			i++;
		}
	}

	wapp_send_1905_msg(
		wapp,
		WAPP_GET_WSC_CONF,
		data_len,
		data);

	os_free(data);
	return WAPP_SUCCESS;
}

int map_radio_tear_down(
	struct wifi_app *wapp,
	char *idtfer)
{
	struct wapp_dev *wdev = NULL;
	struct map_conf_state *conf_stat = NULL;
	struct dl_list *dev_list;
	struct wapp_radio *ra;
	u8 idfr[MAC_ADDR_LEN];
	int i;
	struct ap_dev * ap = NULL;
#ifdef HOSTAPD_MAP_SUPPORT
	char cmd[MAX_CMD_MSG_LEN] = {0};
	struct wireless_setting unConf_wifi_profile = {0};
#endif
	struct wireless_setting config;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp || !idtfer) {
		printf(RED("%s, %u\n"), __FUNCTION__, __LINE__);
		return WAPP_INVALID_ARG;
	}

	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		ra = &wapp->radio[i];
		if (ra->adpt_id) {
			MAP_GET_RADIO_IDNFER(ra, idfr);
			if(!os_memcmp(idfr, idtfer, ETH_ALEN))
				break;
		}
	}

	if (i == MAX_NUM_OF_RADIO) {
		DBGPRINT(RT_DEBUG_ERROR, RED("%s, %u\n"), __func__, __LINE__);
		return WAPP_INVALID_ARG;
	}

	/*update wps bh info*/
	DBGPRINT(RT_DEBUG_ERROR, "%s disable current radio(%02x:%02x:%02x:%02x:%02x:%02x) in wps bh info\n",
		__func__, PRINT_MAC(idtfer));
	memset(&config, 0, sizeof(config));
	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
		/*check wdev with ap cap in the same radio of wireless setting*/
		if (wdev && wdev->radio == ra && wdev->dev_type == WAPP_DEV_TYPE_AP) {
			ap = (struct ap_dev *)wdev->p_dev;
			if (ap->bss_info.map_role & BIT_BH_BSS) {
				os_memcpy(config.mac_addr, ap->bss_info.if_addr, MAC_ADDR_LEN);
				os_memcpy(config.Ssid, ap->bss_info.ssid, ap->bss_info.SsidLen);
				config.AuthMode = ap->bss_info.auth_mode;
				config.EncrypType = ap->bss_info.enc_type;
				os_memcpy(config.WPAKey, ap->bss_info.key, ap->bss_info.key_len);
				config.map_vendor_extension = BIT_TEAR_DOWN;
				config.hidden_ssid = ap->bss_info.hidden_ssid;
				DBGPRINT(RT_DEBUG_ERROR, "%s ssid(%s) key(%s)\n",
					__func__, config.Ssid, config.WPAKey);
				break;
			}
		}
	}
	if (wdev)
		set_bh_wsc_profile(wapp, &config);

	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
		if (wdev && wdev->radio == ra && wdev->dev_type == WAPP_DEV_TYPE_AP) {
			ap = (struct ap_dev *)wdev->p_dev;
			ap->isActive = WAPP_BSS_STOP;
			wdev_set_ssid(wapp, wdev, "MAP-UNCONF");
			wapp_set_bss_stop(wapp, wdev->ifname);
#ifdef HOSTAPD_MAP_SUPPORT
			os_memcpy(unConf_wifi_profile.Ssid, "MAP-UNCONF", os_strlen("MAP-UNCONF"));
			wapp_set_hapd_wifi_profile(wapp, wdev, &unConf_wifi_profile, 0, FALSE);
			/*Bringdown hostapd for this interface*/
			memset(cmd,0,sizeof(cmd));
			os_snprintf(cmd,sizeof(cmd), "hostapd_cli -i%s disable", wdev->ifname);
			system(cmd);
#endif
			DBGPRINT(RT_DEBUG_TRACE, "%s bss stop %s\n", __func__, wdev->ifname);
		}
	}
	conf_stat = &ra->conf_state;
	MAP_CONF_STATE_SET(conf_stat, MAP_CONF_STOP);

	return WAPP_SUCCESS;
}

int map_bh_ready(
	struct wifi_app *wapp,
	u8 bh_type,
	u8 *ifname,
	u8 *mac_addr,
	u8 *bssid)
{
	struct bh_link_info bh;
	wapp_device_status *device_status = &wapp->map->device_status;
#if 1  //need to be able to repond to AP Capability Query in AP AutoConfigSearch stage
	int i = 0, len = 0;
	char *map_evt_buf = (char*)os_malloc(MAX_EVT_BUF_LEN * sizeof(char));
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!map_evt_buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s Alloc memory failed !!!!!\n", __func__);
		return WAPP_RESOURCE_ALLOC_FAIL;
	}
#if 0
	if (wapp->map->bh_link_ready) {
		os_free(map_evt_buf);
		return MAP_ERROR;
	}
#endif
	for (i = 0; i < MAX_NUM_OF_RADIO; i++)
	{
		struct wapp_radio *ra = &wapp->radio[i];
		u8 id[MAC_ADDR_LEN];

		if (!ra->adpt_id)
			continue;

		MAP_GET_RADIO_IDNFER(ra, id);
		map_send_ap_ht_capability_msg(wapp, id, map_evt_buf, &len);
		if (0 > map_1905_send(wapp, map_evt_buf, len)) {
			DBGPRINT(RT_DEBUG_ERROR, "%s send ap ht capability msg fail\n", __func__);
			os_free(map_evt_buf);
			return MAP_ERROR;
		}
		memset(map_evt_buf,0, MAX_EVT_BUF_LEN * sizeof(char));
		map_send_ap_vht_capability_msg(wapp, id, map_evt_buf, &len);
		if (0 > map_1905_send(wapp, map_evt_buf, len)) {
			DBGPRINT(RT_DEBUG_ERROR, "%s send ap vht capability msg fail\n", __func__);
			os_free(map_evt_buf);
			return MAP_ERROR;
		}
	}
	os_free(map_evt_buf);
	map_evt_buf = NULL;
#endif
	wapp->map->bh_link_ready = 1;

	bh.type = bh_type;
	if (bh.type == MAP_BH_ETH) {
		int i;
		struct map_conf_state *conf_stat = NULL;
		for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
			conf_stat = &wapp->radio[i].conf_state;
			MAP_CONF_STATE_SET(conf_stat, MAP_CONF_UNCONF);
		}
		wapp->map->ongoing_conf_retry_times = 0;
		map_reset_conf_sm(wapp->map);
		wapp_soft_reset_scan_states(wapp);
	}
	os_memcpy(bh.ifname, ifname, IFNAMSIZ);
	os_memcpy(bh.mac_addr, mac_addr, MAC_ADDR_LEN);
	os_memcpy(bh.bssid, bssid, MAC_ADDR_LEN);
	DBGPRINT(RT_DEBUG_ERROR,"\033[1;36m %s linked to %02X:%02X:%02X:%02X:%02X:%02X \033[0m\n", __func__,PRINT_MAC(bh.bssid));
	if (device_status->status_bhsta != STATUS_BHSTA_CONFIGURED) {
		if(bh.type != MAP_BH_ETH)
			device_status->status_bhsta = STATUS_BHSTA_BH_READY;
		else
			device_status->status_bhsta = STATUS_BHSTA_UNCONFIGURED;
		device_status->status_fhbss = STATUS_FHBSS_UNCONFIGURED;
		wapp_send_1905_msg(
			wapp,
			WAPP_DEVICE_STATUS,
			sizeof(wapp_device_status),
			(char *)device_status);
	}
	return wapp_send_1905_msg(
		wapp,
		WAPP_MAP_BH_READY,
		sizeof (struct bh_link_info),
		(char *) &bh);
}

void map_conf_state_check(
	struct wifi_app *wapp)
{
	int ret;
	struct map_info *map = wapp->map;
	struct wapp_radio *ra = NULL;
	struct map_conf_state *conf_stat = NULL;
	unsigned char identifier[6] = {0};
	int conf_flag = 0;
	int i = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	/* if backhaul link is ready and controller is found,
	   check for any unconfig radio and try to get config */

	if (map->bh_link_ready && map->ctrler_found)
	{
		if (map->conf_ongoing_radio_idx >= MAX_NUM_OF_RADIO) {
			map->conf_ongoing_radio_idx = 0;
		}

		map->conf_ongoing_radio = &wapp->radio[map->conf_ongoing_radio_idx];
		ra = map->conf_ongoing_radio;

		/* non active radio , try next radio */
		if (!ra->adpt_id) {
			struct map_conf_state *conf_state = NULL;
			conf_flag = 0;
			map->conf_ongoing_radio_idx++;
			if (wapp->map->conf == MAP_CONN_STATUS_CONF)
				return;
			for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
				conf_state = &wapp->radio[i].conf_state;
				if ((wapp->radio[i].adpt_id) && (IS_CONF_STATE(conf_state, MAP_CONF_UNCONF)
					|| IS_CONF_STATE(conf_state, MAP_CONF_WAIT_RSP)))
					conf_flag = 1;
			}
			if(conf_flag != 1) {
				wapp_device_status *device_status = &map->device_status;
				eloop_cancel_timeout(map_wps_timeout, wapp, device_status);
				wapp->map->conf = MAP_CONN_STATUS_CONF;
				device_status->status_fhbss = STATUS_FHBSS_CONFIGURED;
				device_status->status_bhsta = STATUS_BHSTA_CONFIGURED;
				wapp_send_1905_msg(
					wapp,
					WAPP_DEVICE_STATUS,
					sizeof(wapp_device_status),
					(char *)device_status);
			}
			return;
		}

		MAP_GET_RADIO_IDNFER(ra, identifier);
		conf_stat = &ra->conf_state;

		DBGPRINT(RT_DEBUG_TRACE, "identifier(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(identifier));
		DBGPRINT(RT_DEBUG_TRACE, "state(%d)\n", conf_stat->state);
		DBGPRINT(RT_DEBUG_TRACE, "conf_ongoing_radio_idx(%d)\n" ,map->conf_ongoing_radio_idx);
		DBGPRINT(RT_DEBUG_TRACE, "ongoing_conf_retry_times(%d)\n" ,map->ongoing_conf_retry_times);
		DBGPRINT(RT_DEBUG_TRACE, "elapsed_time(%d)\n" ,conf_stat->elapsed_time);

		if (IS_CONF_STATE(conf_stat, MAP_CONF_UNCONF)) {
			ret = map_get_config(wapp, ra);
			if (ret == WAPP_SUCCESS) {
				/* retry 4 times, then give up , try next radio */
				if(map->ongoing_conf_retry_times > 3) {
					wapp_device_status *device_status = &map->device_status;
					MAP_CONF_STATE_SET(conf_stat, MAP_CONF_STOP);
					printf("\033[1;32m %s, %u give up radio %d.\033[0m\n", __FUNCTION__, __LINE__,map->conf_ongoing_radio_idx);
					map->conf_ongoing_radio_idx++;
					map->ongoing_conf_retry_times = 0;
					device_status->status_bhsta = STATUS_BHSTA_CONFIG_PENDING;
					device_status->status_fhbss = STATUS_FHBSS_UNCONFIGURED;
					wapp_send_1905_msg(
						wapp,
						WAPP_DEVICE_STATUS,
						sizeof(wapp_device_status),
						(char *)device_status);
					for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
						ra = &wapp->radio[i];
						MAP_CONF_STATE_SET((&ra->conf_state), MAP_CONF_UNCONF);
					}
					if(wapp->map->TurnKeyEnable) {
						map_reset_conf_sm(wapp->map);
					}
					wapp->map->bh_link_ready = 0;
					if(wapp->map->is_agnt)
						wapp->map->ctrler_found = 0;
					device_status->status_fhbss = STATUS_FHBSS_UNCONFIGURED;
					device_status->status_bhsta = STATUS_BHSTA_UNCONFIGURED;
					wapp_send_1905_msg(
					wapp,
					WAPP_DEVICE_STATUS,
					sizeof(wapp_device_status),
					(char *)device_status);
					wapp_soft_reset_scan_states(wapp);
				} else {
					MAP_CONF_STATE_SET(conf_stat, MAP_CONF_WAIT_RSP);
				}
			} else {
			}
		} else if (IS_CONF_STATE(conf_stat, MAP_CONF_WAIT_RSP)) {
			if (conf_stat->elapsed_time > MAP_CONF_TIMEOUT) {
				/* retry */
				MAP_CONF_STATE_SET(conf_stat, MAP_CONF_UNCONF);
				map->ongoing_conf_retry_times++;
			} else {
				conf_stat->elapsed_time++;
			}
		} else if (IS_CONF_STATE(conf_stat, MAP_CONF_STOP)) {
			/* 1905 will auto retry m1 if it got renew, will be confed in map_config_wireless_setting_msg */
			printf("\033[1;32m %s, %u this radio %d was gave up, do nothing.\033[0m\n", __FUNCTION__, __LINE__,map->conf_ongoing_radio_idx);
			map->conf_ongoing_radio_idx++;
			map->ongoing_conf_retry_times = 0;
		} else if (IS_CONF_STATE(conf_stat, MAP_CONF_CONFED)){
			map->conf_ongoing_radio_idx++;
			map->ongoing_conf_retry_times = 0;
		}

	}
}

void map_block_list_state_check(
	struct wifi_app *wapp)
{
	struct wapp_dev *wdev;
	struct dl_list *dev_list;
	struct ap_dev *ap = NULL;
	int i = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
			ap = (struct ap_dev *)wdev->p_dev;
			for (i = 0; i < ap->max_num_of_block_cli; i++) {
				if (ap->block_table[i].valid_period > 0) {
					ap->block_table[i].valid_period--;
					if (ap->block_table[i].valid_period == 0) {
						DBGPRINT(RT_DEBUG_INFO, "Free assoc ctrl sta(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(ap->block_table[i].mac_addr));
						//Delete ACL list
#ifdef ACL_CTRL
						map_blacklist_system_cmd(wdev, ap->block_table[i].mac_addr, UNBLOCK);
#else
						map_acl_system_cmd(wdev, ap->block_table[i].mac_addr, ACL_DEL);
#endif
						//Clear acl list in wapp
						os_memset(ap->block_table[i].mac_addr, 0, MAC_ADDR_LEN);
						ap->num_of_block_cli--;
					}
				}
			}
		}
	}
}

void map_link_metric_check(
	struct wifi_app *wapp)
{
	int i = 0;
	struct wapp_radio *ra = NULL;
	char idfr[MAC_ADDR_LEN];
	struct wapp_dev *wdev = NULL;
	static int cnt = 0, tot_cnt = 0;
	static int send_above_threshold = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	for (i = 0; i < MAX_NUM_OF_RADIO; i++)
	{
		ra = &wapp->radio[i];
		if (ra->adpt_id) {
			MAP_GET_RADIO_IDNFER(ra, idfr);
			if (ra->metric_policy.ch_util_thres > 0) {

#if 1 /* work around 4.7.6: do not check real channel utilization,
		 just send link metric report for every 5 sec*/
				//if (cnt % 5 == 0) {
				if (tot_cnt == 10) {
					if (cnt >= 7 && send_above_threshold == 0) {
						wapp_send_1905_msg(
								wapp,
								WAPP_AP_LINK_METRIC_REQ,
								ETH_ALEN,
								idfr);
						send_above_threshold = 1;
					} else if (cnt <= 5 && send_above_threshold == 1) {
						wapp_send_1905_msg(
								wapp,
								WAPP_AP_LINK_METRIC_REQ,
								ETH_ALEN,
								idfr);
						send_above_threshold = 0;
					}
					cnt = 0;
					tot_cnt = 0;
				}
				//}
#else
				/*check if the channel utilization cross the threshold*/
				if ((ra->metric_policy.ch_util_current > ra->metric_policy.ch_util_thres &&
					 ra->metric_policy.ch_util_prev < ra->metric_policy.ch_util_thres) ||
					(ra->metric_policy.ch_util_current < ra->metric_policy.ch_util_thres &&
					 ra->metric_policy.ch_util_prev > ra->metric_policy.ch_util_thres)) {
					DBGPRINT(RT_DEBUG_OFF,
						"send WAPP_AP_LINK_METRIC_REQ to 1905, curr:%d, thres: %d, prev: %d\n",
						ra->metric_policy.ch_util_current,
						ra->metric_policy.ch_util_thres,
						ra->metric_policy.ch_util_prev);
					wapp_send_1905_msg(
								wapp,
								WAPP_AP_LINK_METRIC_REQ,
								0,
								NULL);
				}
#endif
				/*query channel utilization again*/
				wdev = wapp_dev_list_lookup_by_radio(wapp, idfr);
				if (wdev) {
					wapp_query_wdev_by_req_id(wapp, wdev->ifname, WAPP_CH_UTIL_QUERY_REQ);
				}
				if (ra->metric_policy.ch_util_current > ra->metric_policy.ch_util_thres)
					cnt++;
				tot_cnt++;
			}
		}
	}
}

void map_periodic_exec(
	struct wifi_app *wapp)
{
//	struct map_info *map = wapp->map;
//	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	//map_conf_state_check(wapp);
	map_block_list_state_check(wapp);
	map_link_metric_check(wapp);

	/* search backhaul wdev */
#if 0
	if (map->bh_wifi_dev == NULL) {
		wdev = wapp_dev_list_lookup_by_ifname(
					wapp,
					MAP_DEFAULT_WIFI_BH);

		if (wdev)
			map->bh_wifi_dev = wdev;
	}
#endif
}

int map_reset_default(
		struct wifi_app *wapp)
{
	int i, ret = 0;
	struct wapp_dev *wdev = NULL, *main_dev = NULL;
	struct wapp_radio *ra = NULL;
	struct dl_list *dev_list;
	struct sec_info sec;
	wapp_device_status *device_status = &wapp->map->device_status;


	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wapp\n", __func__);
		return WAPP_INVALID_ARG;
	}

	dev_list = &wapp->dev_list;
	os_memset(&sec, 0, sizeof(sec));
#if 1
	os_strncpy(sec.auth, "OPEN", 4);
	os_strncpy(sec.encryp, "NONE", 4);
	os_strncpy(sec.psphr, "12345678", 8);
#endif
	save_map_parameters(wapp,"BhProfile0Valid", "0", NON_DRIVER_PARAM);

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
		if (wdev && (wdev->dev_type == WAPP_DEV_TYPE_APCLI || wdev->dev_type == WAPP_DEV_TYPE_STA)) {
			wdev_bh_sta_reset_default(wapp, wdev);
			continue;
		}
	}

	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
//#ifdef MAP_R2
		struct wapp_dev *ap_wdev = NULL;
//#endif
		ra = &wapp->radio[i];

		if (ra->adpt_id == 0)
			continue;

		dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
			eloop_cancel_timeout(bh_steering_ready_timeout, wapp, wdev);
#if 1 /* now, only for ap_dev */
#ifdef MAP_R2
			ts_bh_set_default_8021q(wdev, 0, 0);
			ts_bh_set_all_vid(wdev, 0, NULL);
			ts_fh_set_vid(wdev, 4096);
#endif
#if 0
			if (wdev && wdev->dev_type != WAPP_DEV_TYPE_AP) {
				wdev_bh_sta_reset_default(wapp, wdev);
				continue;
                        }
#endif
#endif

			if (wdev->radio == ra) {
//#ifdef MAP_R2
				struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
				ap_wdev = wdev;
				ap->isActive = WAPP_BSS_START;
				wapp_set_bss_start(wapp, wdev->ifname);
//#endif
				if (!main_dev)
					main_dev = wdev;
				ret += wdev_set_sec_and_ssid(wapp, wdev, &sec, MAP_DEFAULT_SSID);
				//sleep(3);
			}

		}
//#ifdef MAP_R2
		if (ap_wdev)
			wapp_reset_map_params(wapp, ap_wdev);
//#endif
		

#if 0
		/* Only need to set channel once for each band */
		if (main_dev && main_dev->radio) {
			ra = main_dev->radio;
#if 0 /* in some cases, UCC will not trigger controller send channel req and 5GH cases will fail */
			if (ra->op_ch > 14)
				ret += wdev_set_ch(wapp, main_dev, 36);
			else
				ret += wdev_set_ch(wapp, main_dev, 6);
#endif

#if 1
			ret += wdev_set_radio_onoff(wapp, main_dev, RADIO_ON);
#endif
			main_dev = NULL;
		}
#endif
		MAP_CONF_STATE_SET((&ra->conf_state), MAP_CONF_UNCONF);
	}
	if(wapp->map->TurnKeyEnable) {
		map_reset_conf_sm(wapp->map);
		eloop_cancel_timeout(map_config_state_check,wapp,NULL);
	}
	wapp_reset_scan_states(wapp);
	wapp->map->bh_link_ready = 0;
	if(wapp->map->is_agnt)
		wapp->map->ctrler_found = 0;
	device_status->status_fhbss = STATUS_FHBSS_UNCONFIGURED;
	device_status->status_bhsta = STATUS_BHSTA_UNCONFIGURED;
	wapp_send_1905_msg(
		wapp,
		WAPP_DEVICE_STATUS,
		sizeof(wapp_device_status),
		(char *)device_status);
	map_send_reset_msg(wapp);

	return ret;
}

void map_show_param(
	struct wifi_app *wapp)
{
	struct map_info *map = NULL;
	FILE *file;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wapp\n", __func__);
		return;
	}

	map = wapp->map;

	file = fopen(MAP_TMP_FILE, "w");

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, "open MAP tmp file (%s) fail\n", MAP_TMP_FILE);
		return;
	}
#if 0
	DBGPRINT_RAW(RT_DEBUG_OFF,
		"map param:\n"
		"\t alid              %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\t fh_24g_bssid 	  %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\t fh_5g1_bssid 	  %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\t fh_5g2_bssid 	  %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\t fh_radio_supt 	  %u\n"
		"\t bh_sta_radio 	  %u\n"
		"\t ht_24g_supt       %u\n"
		"\t he_24g_supt       %u\n"
		"\t ht_5g1_supt       %u\n"
		"\t vht_5g1_supt      %u\n"
		"\t he_5g1_supt       %u\n"
		"\t vht_5g2_supt      %u\n"
		"\t ht_5g2_supt       %u\n"
		"\t he_5g2_supt       %u\n"
		"\t bh_type           %s\n",
		PRINT_MAC(map->alid),
		PRINT_MAC(map->fh_24g_bssid),
		PRINT_MAC(map->fh_5g1_bssid),
		PRINT_MAC(map->fh_5g2_bssid),
		map->fh_radio_supt,
		map->bh_sta_radio,
		map->ht_24g_supt,
		map->he_24g_supt,
		map->ht_5g1_supt,
		map->vht_5g1_supt,
		map->he_5g1_supt,
		map->ht_5g2_supt,
		map->vht_5g2_supt,
		map->he_5g2_supt,
		map->bh_type == MAP_BH_ETH ? "eth" : "wifi");
#else
	fprintf(file,
		"map param:\n"
		"\t is_ctrler             %u\n"
		"\t is_agnt               %u\n"
		"\t is_root 		      %u\n"
		"\t ctrl_alid             %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\t agnt_alid             %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\t fh_24g_bssid 	      %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\t fh_5g1_bssid 	      %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\t fh_5g2_bssid 	      %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\t fh_radio_supt 	      %u\n"
		"\t bh_wifi_dev 	      %s\n"
		"\t bh_type(%u)           %s\n"
		"\t radio_band_options    %u-%u-%u\n",
		map->is_ctrler,
		map->is_agnt,
		map->is_root,
		PRINT_MAC(map->ctrl_alid),
		PRINT_MAC(map->agnt_alid),
		PRINT_MAC(map->fh_24g_bssid),
		PRINT_MAC(map->fh_5g1_bssid),
		PRINT_MAC(map->fh_5g2_bssid),
		map->fh_radio_supt,
		map->bh_wifi_dev == NULL? "NONE":map->bh_wifi_dev->ifname,
		map->bh_type, map->bh_type == MAP_BH_ETH ? "eth" : "wifi",
		map->radio_band_options[0],
		map->radio_band_options[1],
		map->radio_band_options[2]);
#endif

	fclose(file);

	map_send_wireless_inf_info(wapp, TRUE, 0);
}

int map_btm_rsp_action(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	const u8 *peer_mac_addr,
	struct btm_payload *btm_resp)
{
	char buf[128] = {0};
	struct cli_steer_btm_event btm_evt;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

#if 0
	hex_dump("rsp_data", (unsigned char*) btm_rps, btm_rsp_len);

	DBGPRINT(RT_DEBUG_OFF, BLUE("%s target bssid: %02x:%02x:%02x:%02x:%02x:%02x\n"), __FUNCTION__, PRINT_MAC(btm_rps->u.btm_rsp.variable));
#endif
	DBGPRINT(RT_DEBUG_OFF, BLUE("%s peer_mac_addr: %02x:%02x:%02x:%02x:%02x:%02x\n"), __func__, PRINT_MAC(peer_mac_addr));

	os_memset(&btm_evt,0,sizeof(btm_evt));
	/*send client steering BTM report*/
	COPY_MAC_ADDR(&btm_evt.bssid, wdev->mac_addr);
	COPY_MAC_ADDR(&btm_evt.sta_mac, peer_mac_addr);
	btm_evt.status = btm_resp->u.btm_rsp.status_code;
	if (btm_resp->u.btm_rsp.status_code == 0) /* target bssid is present only if status code is 0 */
		COPY_MAC_ADDR(&btm_evt.tbssid, btm_resp->u.btm_rsp.variable);
	wapp_send_cli_steer_btm_report_msg(wapp, buf, 128, &btm_evt);

	return WAPP_SUCCESS;
}

int map_update_radio_band(
	struct wifi_app *wapp,
	struct wapp_radio *ra,
	u8 ch)
{
	int i, j;
	u8 used = FALSE;
	struct map_info *map = wapp->map;

	if (!ra)
		return WAPP_INVALID_ARG;

	/* release the ocuppied entry */
	ra->radio_band = NULL;

	/* Check single band first */
	for ( i = 0; i < MAP_MAX_RADIO; i++)
	{
		if ((map->radio_band_options[i] == RADIO_24G && IS_MAP_CH_24G(ra->op_ch)) ||
			(map->radio_band_options[i] == RADIO_5GL && IS_MAP_CH_5GL(ra->op_ch)) ||
			(map->radio_band_options[i] == RADIO_5GH && IS_MAP_CH_5GH(ra->op_ch)))
		{
			used = FALSE;
			/* found one option match the radio channel,
			   check if other radio is using this one */
			for (j = 0; j < MAP_MAX_RADIO; j++)
			{
				if (wapp->radio[j].radio_band == &map->radio_band_options[i])
				{
					used = TRUE;
					break;
				}
			}

			if (used == FALSE)
			{
				ra->radio_band = &map->radio_band_options[i];
				break;
			}
		}
	}

	/* If not found, check again for 5G full band options */
	if (ra->radio_band == NULL) {
		for ( i = 0; i < MAP_MAX_RADIO; i++)
		{
			if ( map->radio_band_options[i] == RADIO_5G && IS_MAP_CH_5G(ra->op_ch))
			{
				used = FALSE;
				/* found one option match the radio channel,
			   	check if other radio is using this one */
				for (j = 0; j < MAP_MAX_RADIO; j++)
				{
					if (wapp->radio[j].radio_band == &map->radio_band_options[i])
					{
						used = TRUE;
						break;
					}
				}

				if (used == FALSE)
				{
					ra->radio_band = &map->radio_band_options[i];
					break;
				}
			}
		}
	}
	if (ra->radio_band == NULL)
		DBGPRINT(RT_DEBUG_OFF, RED("%s(): no valid band options!\n"), __func__);
	return WAPP_SUCCESS;
}

struct wapp_ctrl_cmd map_cmd[] = {
	{"show_param",        map_cmd_show_param,                      "show param for wts DEV_GET_PARAMETER"},
	{"get_macaddr",       map_cmd_get_macaddr_by_ssid_ruid,        "get macaddr by matching ssid and radioID"},
	{"set_bh_type",       map_cmd_set_bh_type,                     "set backhaul type: [arg1]:eth or wifi"},
	{"set_alid",          map_cmd_set_alid,                        "[arg1]:ALid. set local ALid"},
	{"dev_send_1905",     map_cmd_send_1905,                       "dev data to 1905"},
	{"reset_default",     map_cmd_reset_default,                   "reset default (unconf)"},
	{"dev_set_config",    map_cmd_set_dev_config,                  "notify dev setting config to 1905"},
	{"trigger_wps",		  map_cmd_trigger_wps,                     "trigger ap or apcli do wps (currently from UCC)"},
#if 0 /* TODO */
	{"set_devrole",       wapp_cmd_wdev_ht_cap_query,			   "[arg1]:interface. query ht_cap of the interface"},
	{"set_bh_type",       wapp_cmd_wdev_ht_cap_query,			   "[arg1]:interface. query ht_cap of the interface"},
	{"set_bh_if",         wapp_cmd_wdev_ht_cap_query,			   "[arg1]:interface. query ht_cap of the interface"},
#endif /* TODO*/
	{"help",              map_cmd_show_help,                    "show this help"},
	{"wps_connect",       wps_connect,                      	"wps_connect"},
	{"0x0001",				toponotify,                  		"topology notification"},
	{"0x8018",				high_layer_data,                    "send high layer data"},
	{"operbss",				operbss,                      		"operational bss info"},
	{"0x8006",				ch_select_req,                      "channel selection request message"},
	{"0x8014",				cli_steer_req,                      "client steering request message"},
	{"0x8004",				ch_prefer_query,                    "channel preference query message"},
	{"0x8003",				map_policy_config_req,              "multi-ap policy config request message"},
	{"0x8016",				cli_assoc_cntrl_req,                "client association control request message"},
/*commen*/
	{"discovery",			discovery,                      	"topology discovery"},
	{"query",				topoquery,                      	"topology query"},
	{"notification",		toponotify,                  		"topology notification"},
/*steering*/
	{"steer_complete",		steering_completed,                 "steering completed message"},
	{"btm_report",			btm_report,                 		"client steering btm report message"},
	{"steer_mand",			steer_mand,                 		"Steering Request message in 4.8.1"},
	{"steer_oppo",			steer_oppo,                 		"Steering Request message in 4.8.3"},
	{"steer_policy",		steer_policy,                 		"Steering Policy message in 4.8.1"},
	{"steer_policy_rssi",	steer_policy_rssi,                 	"Steering Policy message in 4.8.4"},
	{"assoc_cntrl",			assoc_cntrl,                 		"Client Association Control Request message in 4.8.5"},
	{"backhaul_steer",		backhaul_steer,                 	"Backhaul Steering Request message in 4.9.1"},
/*metrics*/
	{"metrics_all_neighbor",		metrics_all_neighbor,       "link metrics query message in 4.7.1"},
	{"metrics_specific_neighbor",	metrics_specific_neighbor,  "link metrics query message in 4.7.2"},
	{"metrics_query",				metrics_query,  			"ap metrics query message in 4.7.4"},
	{"metric_report_policy",		metric_report_policy,  		"metrics report policy message in 4.7.5"},
	{"sta_link_metric_query",		sta_link_metric_query,  	"associated sta link metrics query message in 4.7.7"},
	{"sta_unlink_metric_query",		sta_unlink_metric_query,  	"unassociated sta link metrics query message in 4.7.8"},
	{"1905_req",            map_cmd_1905_req,                   "[arg1] req_id [arg2] req_value - send req to 1905 "},
#ifdef MAP_R2
	{"ch_scan",            map_cmd_ch_scan_req,                   "[arg1] req_id [arg2] req_value - send req to 1905 "},
#endif	
#ifdef DFS_CAC_R2
	{"cac_status",            map_cmd_cac_status,                   "[arg1] req_id [arg2] req_value - send cac status to 1905 "},
#endif
#ifdef STOP_BEACON_FEATURE
	{"set_beacon_state",       map_cmd_set_beacon_state,                     "set beacon state: [arg1]:en or dis"},
#endif
	{NULL,                map_cmd_show_help,	NULL}
};


int map_ctrl_interface_cmd_handle(
	struct wifi_app *wapp,
	const char *iface,
	u8 argc,
	char **argv)
{
	int i, ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!argv[0]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	for (i = 0; map_cmd[i].cmd != NULL; i++)
	{
		if (os_strncmp(map_cmd[i].cmd, argv[0], os_strlen(argv[0])) == 0) {
			ret = map_cmd[i].cmd_proc(wapp, iface, argc, argv);
			if (ret != WAPP_SUCCESS)
				DBGPRINT(RT_DEBUG_ERROR, "cmd [%s] failed. ret = %d\n",	map_cmd[i].cmd, ret);
			break;
		}
	}

	return WAPP_SUCCESS;
}

int map_cmd_show_help( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 i = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	printf("\033[1;36m available cmds: \033[0m\n");
	for(i=0;(map_cmd[i].cmd != NULL);i++){
		printf("\033[1;36m %20s  \t -  %s\033[0m\n",map_cmd[i].cmd,map_cmd[i].help);
	}

	return WAPP_SUCCESS;
}


int map_cmd_show_param( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	map_show_param(wapp);

	return WAPP_SUCCESS;
}

int map_cmd_get_macaddr_by_ssid_ruid( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	u8 idnfer[MAC_ADDR_LEN];
	u8 id[MAC_ADDR_LEN];
	u8 ssid[MAX_LEN_OF_SSID];
	struct dl_list *dev_list;
	struct wapp_dev *wdev = NULL;
	FILE *file=NULL;
	BOOLEAN found = FALSE;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (argc != 3) {
		printf("%s: cmd parameter error! need 3 but %d\n", __func__, argc);
		return WAPP_INVALID_ARG;
	}
	printf("\033[1;36m %s SSID [%s] RUID:%s\033[0m\n", __FUNCTION__, argv[1], argv[2]);
	os_strncpy((char *)ssid, argv[1], sizeof(ssid));
	AtoH(argv[2], (char *) idnfer, MAC_ADDR_LEN);

	printf("\033[1;36m %s SSID [%s] RUID: %02x:%02x:%02x:%02x:%02x:%02x\033[0m\n", __FUNCTION__, ssid, PRINT_RA_IDENTIFIER(idnfer));

	file = fopen(MAC_ADDR_TMP_FILE, "w");

	if (!file) {
		printf("\033[1;32m %s, %u \033[0m\n", __FUNCTION__, __LINE__);  /* Haipin Debug Print (G)*/

		DBGPRINT(RT_DEBUG_ERROR, "open MAP tmp file (%s) fail\n", MAC_ADDR_TMP_FILE);
		return WAPP_RESOURCE_ALLOC_FAIL;
	}

	if (os_strcmp(argv[1], "BHSTA") == 0) {
		dev_list = &wapp->dev_list;
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
			if (wdev && wdev->radio) {
				MAP_GET_RADIO_IDNFER(wdev->radio, id);
				if (wdev && (wdev->dev_type == WAPP_DEV_TYPE_STA) &&
					(os_memcmp(id, idnfer, MAC_ADDR_LEN) == 0)) {
						printf("[%s] HWAddr %02x:%02x:%02x:%02x:%02x:%02x\n",wdev->ifname,PRINT_MAC(wdev->mac_addr));
						found = TRUE;
						break;
				}
			}
		}
	} else {
		dev_list = &wapp->dev_list;
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
			if (wdev && wdev->radio) {
				MAP_GET_RADIO_IDNFER(wdev->radio, id);
				if (wdev && (wdev->dev_type == WAPP_DEV_TYPE_AP) &&
					(os_memcmp(id, idnfer, MAC_ADDR_LEN) == 0)) {
						struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
						if (os_memcmp(ssid, ap->bss_info.ssid, os_strlen((char *)ssid)) == 0) {
							printf("[%s] len %zu [%s] HWAddr %02x:%02x:%02x:%02x:%02x:%02x\n",
								ap->bss_info.ssid, os_strlen((char *)ssid), wdev->ifname, PRINT_MAC(wdev->mac_addr));
							found = TRUE;
							break;
						}
				}
			}
		}
	}

	if(found) {
		fprintf(file, "\t HWAddr              %02x:%02x:%02x:%02x:%02x:%02x\n",PRINT_MAC(wdev->mac_addr));
	} else {
		printf("HWAddr N/A\n");
		fprintf(file,
			"HWAddr N/A\n");
	}

	fclose(file);
	return WAPP_SUCCESS;
}


int map_cmd_set_bh_type( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct map_info *map = wapp->map;
	struct wapp_dev *wdev = NULL;
	struct dl_list *dev_list;
	u8 mac_addr[MAC_ADDR_LEN];
	u8 idnfer[MAC_ADDR_LEN];
	u8 id[MAC_ADDR_LEN];
	u8 dev_found = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	printf("\033[1;36m %s = type = %s\033[0m\n", __FUNCTION__, argv[1]);

	if (os_strcmp(argv[1], "eth") == 0) {
		map->bh_type = MAP_BH_ETH;
		wapp_get_mac_addr_by_ifname(MAP_DEFAULT_ETH_BH, mac_addr);
		if (wapp->map->TurnKeyEnable) {
			struct bh_type_info bh;

			bh.type = MAP_BH_ETH;
			wapp_send_1905_msg(
				wapp,
				WAPP_SET_BH_TYPE,
				sizeof (struct bh_type_info),
				(char *) &bh);
		}
		map_bh_ready(wapp, MAP_BH_ETH, (u8 *) MAP_DEFAULT_ETH_BH, mac_addr, mac_addr);
	} else if (os_strcmp(argv[1], "wifi") == 0){
		if (!wapp->map->TurnKeyEnable) {
			if (argv[2] == NULL) {
				DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
				return WAPP_INVALID_ARG;
			}
			AtoH(argv[2], (char *) idnfer, MAC_ADDR_LEN);

			printf("\033[1;36m %s idnfer = %02x%02x%02x%02x%02x%02x\033[0m\n", __FUNCTION__, PRINT_MAC(idnfer));
			dev_list = &wapp->dev_list;
			dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
				if (wdev && wdev->radio) {
					MAP_GET_RADIO_IDNFER(wdev->radio, id);
					if ((wdev->dev_type == WAPP_DEV_TYPE_STA) &&
						(os_memcmp(id, idnfer, MAC_ADDR_LEN) == 0)) {
							dev_found = 1;
							break;
					}
				}
			}

			map->bh_type = MAP_BH_WIFI;
			if (dev_found == 1) {
				map->bh_wifi_dev = wdev;
				printf("bh_wifi_dev  %s\n", map->bh_wifi_dev->ifname);
			} else {
				printf(RED("%s: dev not found use default %s\n"),
						__FUNCTION__, map->bh_wifi_dev->ifname);
			}

		} else {
			struct bh_type_info bh;

			bh.type = MAP_BH_WIFI;
			wapp_send_1905_msg(
				wapp,
				WAPP_SET_BH_TYPE,
				sizeof (struct bh_type_info),
				(char *) &bh);
		}
	} else {
		printf("\033[1;36m %s: unknown bh type %s\033[0m\n", __FUNCTION__, argv[1]);
	}

	return WAPP_SUCCESS;
}


int map_cmd_set_alid( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct map_info *map = wapp->map;
	u8 alid[MAC_ADDR_LEN];
	char *token;
	int i;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!argv[1] || !argv[2]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	printf("\033[1;36m %s ALid = %s\033[0m\n", __FUNCTION__, argv[1]);

	i = 0;
	token = strtok(argv[1], ":");

	while (token != NULL) {
		AtoH(token, (char *) &alid[i], 1);
		i++;
		if (i >= MAC_ADDR_LEN)
			break;
		token = strtok(NULL, ":");
	}

	if (os_strcmp(argv[2], "c") == 0)
		COPY_MAC_ADDR(map->ctrl_alid, alid);
	else
		COPY_MAC_ADDR(map->agnt_alid, alid);

	printf("\033[1;36m %s alid = %02x:%02x:%02x:%02x:%02x:%02x\033[0m\n", __FUNCTION__, PRINT_MAC(alid));

	return WAPP_SUCCESS;
}

int map_cmd_reset_default( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	return map_reset_default(wapp);
}

int map_cmd_send_1905( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct evt *wapp_event;
	u8 buffer[256];
	int send_pkt_len = 0;
	int role;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!argv[1] || !argv[2]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	wapp_event = (struct evt *)buffer;
	wapp_event->type = WAPP_1905_READ_1905_TLV_REQUEST;
	wapp_event->length = strlen(argv[1]);
	os_strncpy((char *)wapp_event->buffer, argv[1], strlen(argv[1]));
	send_pkt_len = sizeof(struct evt) + wapp_event->length;

	role = hex2num(*argv[2]);

	if (role == 0) { /*send to controller*/
		if (0 > map_1905_send_controller(wapp, (char *)buffer, send_pkt_len)) {
			DBGPRINT(RT_DEBUG_ERROR, "%s map_1905_send fail\n", __func__);
			return MAP_ERROR;
		}
	} else { /*send to agent*/
		if (0 > map_1905_send(wapp, (char *)buffer, send_pkt_len)) {
			DBGPRINT(RT_DEBUG_ERROR, "%s map_1905_send fail\n", __func__);
			return MAP_ERROR;
		}
	}
	DBGPRINT_RAW(RT_DEBUG_OFF,
				"%s\t argv[1] = %s	len=%zu	event->buffer=%s argv[2] = %d\n",
				 __FUNCTION__,argv[1],strlen(argv[1]), wapp_event->buffer, role);

	memset(buffer, 0, send_pkt_len);
	return WAPP_SUCCESS;
}

int map_cmd_set_dev_config( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct evt *wapp_event;
	u8 buffer[256];
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	wapp_event = (struct evt *)buffer;
	wapp_event->type = WAPP_1905_READ_BSS_CONF_REQUEST;
	wapp_event->length = strlen(argv[1]);
	os_strncpy((char *)wapp_event->buffer, argv[1], strlen(argv[1]));
	send_pkt_len = sizeof(struct evt) + wapp_event->length;

	if (0 > map_1905_send_controller(wapp, (char *)buffer, send_pkt_len)) {
		DBGPRINT(RT_DEBUG_ERROR, "%s map_1905_send fail\n", __func__);
		return MAP_ERROR;
	}
	DBGPRINT_RAW(RT_DEBUG_OFF,
				"%s\t argv[1] = %s	len=%zu  event->buffer=%s\n",
				 __FUNCTION__,argv[1],strlen(argv[1]), wapp_event->buffer);

	memset(buffer, 0, send_pkt_len);
	return WAPP_SUCCESS;
}

struct wapp_dev* wapp_dev_list_lookup_by_band_and_type_for_cert(struct wifi_app *wapp, int band, int target_dev_type)
{
	struct wapp_dev *wdev = NULL, *target_wdev = NULL;
	struct dl_list *dev_list;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){

		if (wdev && wdev->radio && 
			((IS_MAP_CH_5GL(wdev->radio->op_ch) && band == WPS_BAND_5GL)
			|| (IS_MAP_CH_5GH(wdev->radio->op_ch) && band == WPS_BAND_5GH) 
			|| (IS_MAP_CH_24G(wdev->radio->op_ch) && band == WPS_BAND_24G))
			&& wdev->dev_type == target_dev_type) {
			target_wdev = wdev;
			break;
		}
	}


	if (target_wdev)
		DBGPRINT(RT_DEBUG_ERROR,"\033[1;36m %s, got target_wdev [%s], ifidx %d \033[0m\n", __FUNCTION__, target_wdev->ifname, target_wdev->ifindex); 
	return target_wdev;
}

struct wapp_dev* wapp_dev_list_lookup_by_band_and_type(struct wifi_app *wapp, int band, int target_dev_type)
{
	struct wapp_dev *wdev = NULL, *target_wdev = NULL;
	struct dl_list *dev_list;
	int radio_count = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s band: %d\n", __func__, band);

	dev_list = &wapp->dev_list;
	radio_count = get_valid_radio_count(wapp);

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (radio_count == 3) {
			if (wdev && wdev->radio &&
				((IS_MAP_CH_5GL(wdev->radio->op_ch) && band == WPS_BAND_5GL)
				|| (IS_MAP_CH_5GH(wdev->radio->op_ch) && band == WPS_BAND_5GH) 
				|| (IS_MAP_CH_24G(wdev->radio->op_ch) && band == WPS_BAND_24G))
				&& wdev->dev_type == target_dev_type) {
				target_wdev = wdev;
				break;
			}
		} else if (radio_count == 2) {
			if (wdev && wdev->radio &&
				((IS_MAP_CH_5G(wdev->radio->op_ch) && band == WPS_BAND_5G) 
				|| (IS_MAP_CH_24G(wdev->radio->op_ch) && band == WPS_BAND_24G))
				&& wdev->dev_type == target_dev_type) {
				target_wdev = wdev;
				break;
			}
		}
	}


	if (target_wdev)
		DBGPRINT(RT_DEBUG_ERROR,"\033[1;36m %s, got target_wdev [%s], ifidx %d \033[0m\n", __FUNCTION__, target_wdev->ifname, target_wdev->ifindex); 
	return target_wdev;
}


struct wapp_dev* wapp_dev_list_lookup_fhbss_by_band_and_type(struct wifi_app *wapp, int band, int target_dev_type)
{
	struct wapp_dev *wdev, *target_wdev = NULL;
	struct dl_list *dev_list;
	int radio_count = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	struct ap_dev *ap;

	dev_list = &wapp->dev_list;
	radio_count = get_valid_radio_count(wapp);

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		ap = (struct ap_dev *)wdev->p_dev;

		if (ap && (ap->isActive != WAPP_BSS_START))
			continue;
		if (radio_count == 2) {
			if (wdev && wdev->radio &&
				((IS_MAP_CH_5G(wdev->radio->op_ch) && band == WPS_BAND_5G)
				|| (IS_MAP_CH_24G(wdev->radio->op_ch) && band == WPS_BAND_24G))
				&& wdev->dev_type == target_dev_type && wdev->i_am_fh_bss) {
				target_wdev = wdev;
				break;
			}
		} else {
			if (wdev && wdev->radio &&
				((IS_MAP_CH_5GL(wdev->radio->op_ch) && band == WPS_BAND_5GL)
				|| (IS_MAP_CH_5GH(wdev->radio->op_ch) && band == WPS_BAND_5GH)
				|| (IS_MAP_CH_24G(wdev->radio->op_ch) && band == WPS_BAND_24G))
				&& wdev->dev_type == target_dev_type && wdev->i_am_fh_bss) {
				target_wdev = wdev;
				break;
			}
		}
	}


	if (target_wdev)
		printf("\033[1;36m %s, got target_wdev [%s], ifidx %d \033[0m\n", __FUNCTION__, target_wdev->ifname, target_wdev->ifindex);
	return target_wdev;
}
int map_cmd_trigger_wps( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct map_info *map = wapp->map;
	char cmd[1024];
	char cmd_bk[1024];
	int wps_mode = 0;
	int target_band = 0;
	struct wapp_dev *target_wdev = NULL;
	int radio_count = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!argv[1] || !argv[2]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}
	radio_count = get_valid_radio_count(wapp);
	/*
	  * argv[1] : mode
	  * argv[2] : band
	  * argv[3] : trigger_role bh/fh (enrollee/registrar)
	  */

	memset(cmd,0,sizeof(cmd));

	//printf("\033[1;36m %s, argv[1] [%s], argv[2] [%s] argv[3] [%s]\033[0m\n", __FUNCTION__,argv[1],argv[2],argv[3]);

	if (os_strcmp(argv[1], "PBC") == 0) {
		wps_mode = 2;
	} else {
		DBGPRINT_RAW(RT_DEBUG_ERROR, "%s UNKNOWN WPS MODE [%s]\n", __FUNCTION__,argv[1]);
		return WAPP_INVALID_ARG;
	}

	if (os_strcmp(argv[2], "24G") == 0) {
		target_band = WPS_BAND_24G;
	} else if (os_strcmp(argv[2], "5GL") == 0){
		if (radio_count == 2)
			target_band = WPS_BAND_5G;
		else
			target_band = WPS_BAND_5GL;
	} else if (os_strcmp(argv[2], "5GH") == 0){
		if (radio_count == 2)
			target_band = WPS_BAND_5G;
		else
			target_band = WPS_BAND_5GH;
	} else
		DBGPRINT_RAW(RT_DEBUG_ERROR, "%s UNKNOWN WPS BAND [%s]\n", __FUNCTION__,argv[2]);

#if 1 /* ucc don't have argv[3], judge role by bh_link_ready */
	if(map->bh_link_ready || map->ctrler_found) { /* bh ready won't come in case of device operating as controller */
		/*I got bh link, trigger first fh bss as wps registrar*/
		target_wdev = wapp_dev_list_lookup_fhbss_by_band_and_type(wapp, target_band, WAPP_DEV_TYPE_AP);

		if(target_wdev == NULL) {
			DBGPRINT_RAW(RT_DEBUG_OFF, "%s cant find wdev for [%s] type WAPP_DEV_TYPE_AP\n",
					 __FUNCTION__,target_band?"5G":"2G");
			return WAPP_INVALID_ARG;
		} else {
			os_snprintf(cmd,sizeof(cmd),
				"iwpriv %s set WscConfMode=4;iwpriv %s set WscMode=%d;iwpriv %s set WscConfStatus=2;iwpriv %s set WscGetConf=1",
				target_wdev->ifname,target_wdev->ifname,wps_mode,target_wdev->ifname,target_wdev->ifname);
		}
	} else {
		/*I don't have bh link, trigger wps as enrollee*/
#ifdef MAP_R2
		target_wdev = wapp_dev_list_lookup_by_band_and_type(wapp, target_band, WAPP_DEV_TYPE_STA);
#else
		target_wdev = map->bh_wifi_dev;
#endif
		if(target_wdev == NULL) {
			DBGPRINT_RAW(RT_DEBUG_OFF, "%s cant find wdev for [%s] type WAPP_DEV_TYPE_APCLI\n",
					 __FUNCTION__,target_band?"5G":"2G");
		} else {
			os_snprintf(cmd,sizeof(cmd),
				"iwpriv %s set ApCliEnable=1;iwpriv %s set WscConfMode=1;iwpriv %s set WscMode=%d;iwpriv %s set WscGetConf=1",
				target_wdev->ifname,target_wdev->ifname,target_wdev->ifname,wps_mode,target_wdev->ifname);
		}
	}


	DBGPRINT_RAW(RT_DEBUG_OFF, "\033[1;36m cmd [%s] \033[0m\n", cmd);
	system(cmd);

	memset(cmd_bk,0,sizeof(cmd_bk));
	os_snprintf(cmd_bk,sizeof(cmd_bk), "echo \"%s\" > /tmp/wps_dbg",cmd);
	system(cmd_bk);
#else /* normal usage, judge by argv[3] : TODO */

	if (os_strcmp(argv[3], "bh") == 0) {
		//trigger wps as enrollee
	} else if (os_strcmp(argv[3], "fh") == 0) {
		//trigger wps as registrar
	} else {
		DBGPRINT_RAW(RT_DEBUG_ERROR,
				"%s UNKNOWN WPS ROLE [%s]\n", __FUNCTION__,argv[3]);
		return WAPP_INVALID_ARG;
	}
#endif
	return WAPP_SUCCESS;
}

#ifdef MAP_R2
u8 mapd_get_channel_scan_capab_from_driver(struct wifi_app *wapp);

#if 1
int map_cmd_ch_scan_req( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{

#if 0
#if 1

	struct off_ch_scan_req_s*scan_req = NULL;
	u8 buf[8000] = {0};
	u32 len;
	u8 radio_id_5GH[6] = {0,0,0,0,1,0};
	u8 radio_id_5GL[6] = {0,0,0,0,3,0};
	u8 radio_id_2G[6] = {0,0,0,0,2,0};
#endif
#if 1
	scan_req = (struct channel_scan_req *)buf;
	printf("%s %d\n", __func__, __LINE__);
	mapd_get_channel_scan_capab_from_driver(wapp);

	scan_req->fresh_scan= 0x80;
	scan_req->radio_num = 3;
	os_memcpy(scan_req->body[0].radio_id, radio_id_2G, 6);
	scan_req->body[0].oper_class_num = 1;
	scan_req->body[0].ch_body[0].oper_class = 81;
	scan_req->body[0].ch_body[0].ch_list_num = 0;
#if 1
	os_memcpy(scan_req->body[1].radio_id, radio_id_5GL, 6);
	scan_req->body[1].oper_class_num = 2;
	scan_req->body[1].ch_body[0].oper_class = 115;
	scan_req->body[1].ch_body[0].ch_list_num = 0;
	
	scan_req->body[1].ch_body[1].oper_class = 118;
	scan_req->body[1].ch_body[1].ch_list_num = 0;

	os_memcpy(scan_req->body[2].radio_id, radio_id_5GH, 6);
	scan_req->body[2].oper_class_num = 2;
	scan_req->body[2].ch_body[0].oper_class = 121;
	scan_req->body[2].ch_body[0].ch_list_num = 0;

	scan_req->body[2].ch_body[1].oper_class = 125;
	scan_req->body[2].ch_body[1].ch_list_num = 0;

#endif
	len = sizeof(struct channel_scan_req) + 3*sizeof(struct scan_body);
	printf("%s: scanReqLen = %d\n",__func__,len);
	map_receive_channel_scan_req(wapp, (char *)buf,(u16)len);
#endif
	// fill send channel scan req
#endif
return WAPP_SUCCESS;
}
#endif

#ifdef DFS_CAC_R2
int map_send_cac_status_msg(
	struct wifi_app *wapp, char *addr, char *evt_buf, int* len_buf);
int map_cmd_cac_status( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	char buf[8000] = {0};
	int len;
	map_send_cac_status_msg(wapp, NULL, buf, &len);
	// fill send channel scan req
	return WAPP_SUCCESS;
}


#endif
#endif
int map_cmd_1905_req( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct wapp_1905_req req;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!argv[1] || !argv[2]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	req.id = atoi(argv[1]);
	req.value = atoi(argv[2]);

	printf(BLUE("%s: id = %u, value = %u\n"), __func__, req.id, req.value);

	map_1905_req(wapp, &req);
	return WAPP_SUCCESS;

}

void map_event_bh_sta_wap_done(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct map_info *map = NULL;
	wapp_device_status *device_status = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	map = wapp->map;
	device_status = &wapp->map->device_status;

	printf(BLUE("%s \n"), __func__);

	if(event_data->bhsta_info.peer_map_enable)
	{
		if (map->bh_type == MAP_BH_WIFI &&
			wdev == map->bh_wifi_dev) {
			map_bh_ready(wapp, MAP_BH_WIFI, (u8 *)wdev->ifname, wdev->mac_addr, event_data->bhsta_info.connected_bssid);
		}	else if (map->bh_wifi_dev == NULL)
		{
			map_bh_ready(wapp, MAP_BH_WIFI, (u8 *)wdev->ifname, wdev->mac_addr, event_data->bhsta_info.connected_bssid);
		}
	}else {
			/* Connection to Non-MAP AP*/
			device_status->status_bhsta = STATUS_BHSTA_WPS_NORMAL_CONFIGURED;
			device_status->status_fhbss = STATUS_FHBSS_UNCONFIGURED;
			wapp_send_1905_msg(
			wapp,
			WAPP_DEVICE_STATUS,
			sizeof(wapp_device_status),
			(char *)device_status);
	}
}

void map_event_str_sta_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	wdev_steer_sta *str_sta = NULL;
	struct wapp_sta *sta = NULL;
	char cand_list[128] = {0};
	char buf[128] = {0};
	u8 btm_neighbor_report_header[2] = {0};
	size_t btm_req_len = 0, cand_list_len = 0;
	u8 frame_pos = 0, req_mode = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		str_sta = &event_data->str_sta;
		DBGPRINT(RT_DEBUG_INFO, "Rssi str_sta[%02x][%02x][%02x][%02x][%02x][%02x]\n", PRINT_MAC(str_sta->mac_addr));
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;

		sta = wdev_ap_client_list_lookup(wapp, ap, str_sta->mac_addr);
		if (!sta) {
			DBGPRINT(RT_DEBUG_ERROR, "%s null sta\n", __FUNCTION__);
			return;
		}

		if (sta->bLocalSteerDisallow == FALSE) {
			if ((sta->bBSSMantSupport == TRUE)
				&& (sta->bBTMSteerDisallow == FALSE)) {
				DBGPRINT(RT_DEBUG_INFO, "%s Rssi Steering 11v Sta\n", __func__);
				req_mode = (1 << ABIDGED_BIT_MAP) | (1 << DISASSOC_IMNT_BIT_MAP);
				u16 append_entry_len = NEIGHBOR_REPORT_IE_SIZE; /* append Bssid ~ CandidatePref */
				/* element ID: 52, len: 16 */
				btm_neighbor_report_header[0] = IE_RRM_NEIGHBOR_REP;
				btm_neighbor_report_header[1] = append_entry_len;

				os_memcpy(&cand_list[frame_pos], &btm_neighbor_report_header, sizeof(btm_neighbor_report_header));
				frame_pos += sizeof(btm_neighbor_report_header);
				cand_list_len += sizeof(btm_neighbor_report_header);

				// fill neighbor report info
				os_memcpy(&cand_list[frame_pos], &wapp->daemon_nr_list.NRInfo[0], append_entry_len);
				cand_list_len += append_entry_len;
				//hex_dump_dbg("entry", (u8 *)frame_pos, append_entry_len);

				btm_req_len = wapp_build_btm_req(req_mode, 10 * 10, 200, //Validate interval
												NULL, NULL, 0, cand_list, cand_list_len, buf);

#ifndef KV_API_SUPPORT
				wapp_send_btm_req(wapp, (char*)wdev->ifname, str_sta->mac_addr, buf, btm_req_len);
#else
				wapp_send_btm_req_11kv_api(wapp, wdev->ifname, sta->mac_addr, buf, btm_req_len);
#endif /* KV_API_SUPPORT */
			} else {
				DBGPRINT(RT_DEBUG_INFO, "%s Rssi Steering Legacy Sta\n", __func__);
				map_trigger_deauth(wapp, wdev->ifname, str_sta->mac_addr);
			}

			if (++sta->steered_count > MAP_MAX_STEER_COUNT)
				sta->bLocalSteerDisallow = TRUE;
		}
	}

}

int save_map_parameters(struct wifi_app *wapp,char *param, char *value, param_type type)
{
#ifdef OPENWRT_SUPPORT
	struct kvc_context *dat_ctx = NULL;
	int ret = 0;
	const char *file = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s param(%s) value(%s)\n",__func__, param, value);

	if (type == NON_DRIVER_PARAM) {
		dat_ctx = dat_load((const char *)wapp->map->map_user_cfg);
		if (!dat_ctx) {
			DBGPRINT(RT_DEBUG_ERROR, "load file(%s) fail\n", wapp->map->map_user_cfg);
			ret = -1;
			goto out;
		}
		ret = kvc_set(dat_ctx, (const char *)param, (const char *)value);
		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, "set param(%s) fail\n", param);
			goto out;
		}
		ret = kvc_commit(dat_ctx);
		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, "write param(%s) fail\n", param);
			goto out;
		}
		/*IN Map Certification Easymesh will not be triggered so we need to reset the params in mapd_cfg*/
		if(!wapp->map->TurnKeyEnable) {
			kvc_unload(dat_ctx);
			dat_ctx = dat_load((const char *)wapp->map->map_cfg);
			if (!dat_ctx) {
				DBGPRINT(RT_DEBUG_ERROR, "load file(%s) fail\n", wapp->map->map_cfg);
				ret = -1;
				goto out;
			}
			ret = kvc_set(dat_ctx, (const char *)param, (const char *)value);
			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, "set param(%s) fail\n", param);
				goto out;
			}
			ret = kvc_commit(dat_ctx);
			if (ret) {
				DBGPRINT(RT_DEBUG_ERROR, "write param(%s) fail\n", param);
				goto out;
			}
		}
	} else {
		file = get_dat_path_by_ord(0);
		if (!file) {
			DBGPRINT(RT_DEBUG_ERROR, "invalid file!!! type(%d)\n", type);
			return -1;
		}
		dat_ctx = dat_load((const char *)file);
		if (!dat_ctx) {
			DBGPRINT(RT_DEBUG_ERROR, "load file(%s) fail\n", file);
			ret = -1;
			goto out;
		}

		ret = kvc_set(dat_ctx, (const char *)param, (const char *)value);
		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, "set param(%s) fail\n", param);
			goto out;
		}
		ret = kvc_commit(dat_ctx);
		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, "write param(%s) fail\n", param);
			goto out;
		}
	}
out:
	if (file && (type == DRIVER_PARAM))
		free_dat_path(file);
	if (dat_ctx)
		kvc_unload(dat_ctx);
	if (ret)
		return -1;
	else
		return 0;
#else
	char cmd_buffer[256] = {0};

	os_snprintf(cmd_buffer, sizeof(cmd_buffer),"nvram_set 2860 %s %s &", param, value);
	system(cmd_buffer);
	DBGPRINT(RT_DEBUG_INFO, "get_map_parameter %s = %s\n", param, value);

	return 0;
#endif
}

int get_map_parameters(struct map_info *map, char* param, char* value, param_type type, size_t val_len)
{
#ifdef OPENWRT_SUPPORT
	const char *tmp_value = NULL;
	unsigned int len = 0;
	struct kvc_context *dat_ctx = NULL;
	const char *file = NULL;
	int ret = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s param(%s)\n",__func__, param);

	os_memset(value, 0, val_len);

	if (type == NON_DRIVER_PARAM)
		file = map->map_cfg;
	else
		file = get_dat_path_by_ord(0);

	if (!file) {
		DBGPRINT(RT_DEBUG_ERROR, "invalid file!!! type(%d)\n", type);
		return -1;
	}

	dat_ctx = dat_load(file);
	if (!dat_ctx) {
		DBGPRINT(RT_DEBUG_ERROR, "load file(%s) fail\n", file);
		ret = -1;
		goto out;
	}

	tmp_value = kvc_get(dat_ctx, (const char *)param);
	if (!tmp_value) {
		DBGPRINT(RT_DEBUG_ERROR, "get param(%s) fail\n", param);
		ret = -1;
		goto out;
	}
	len = os_min(os_strlen(tmp_value), val_len - 1);
	os_memcpy(value, tmp_value, len);

	DBGPRINT(RT_DEBUG_TRACE, "%s value(%s)\n",__func__, value);
out:
	if (file && (type == DRIVER_PARAM))
		free_dat_path(file);
	if (dat_ctx)
		kvc_unload(dat_ctx);

	return ret;
#else
	char cmd_buffer[256] = {0};
	char tmp_buffer[350] = {0};
	char temp_file[] = "/tmp/system_command_output";
	FILE *fp;

	os_memset(value, 0, val_len);
	os_snprintf(cmd_buffer, sizeof(cmd_buffer),"nvram_get 2860 %s > %s &", param, temp_file);
	if ((fp = popen(temp_file, "r")) == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, "Error opening pipe!\n");
		return -1;
	}
	fscanf(fp, "%349s", tmp_buffer);

	os_memcpy(value, tmp_buffer, val_len);
	DBGPRINT(RT_DEBUG_INFO, "get_map_parameter %s = %s\n", param, value);

	if(pclose(fp)) {
		DBGPRINT(RT_DEBUG_ERROR, "Command not found or exited with error status\n");
		return -1;
	}

	return 0;
#endif
}

int map_build_ap_he_cap(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf)
{
	struct wapp_dev *wdev = NULL;
	struct evt *map_event = NULL;
	struct ap_dev *ap = NULL;
	wdev_he_cap *he_cap = NULL;
	int send_pkt_len = 0;
	struct ap_he_capability *map_he_cap;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	wdev = wapp_dev_list_lookup_by_radio(wapp, (char *)addr);
	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s null wdev\n", __func__);
		return 0;
	}

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		ap = (struct ap_dev *)wdev->p_dev;
		he_cap = &ap->he_cap;

		map_event = (struct evt *)evt_buf;
		map_event->type = WAPP_AP_HE_CAPABILITY;
		map_event->length = sizeof(struct ap_he_capability);
		map_he_cap = (struct ap_he_capability *)map_event->buffer;

		if (wdev->radio) {
			MAP_GET_RADIO_IDNFER(wdev->radio, map_he_cap->identifier);
		}

		switch (he_cap->tx_stream) {
			case 1:
				map_he_cap->tx_stream = 0;
				break;
			case 2:
				map_he_cap->tx_stream = 1;
				break;
			case 3:
				map_he_cap->tx_stream = 2;
				break;
			case 4:
				map_he_cap->tx_stream = 3;
				break;
			case 5:
				map_he_cap->tx_stream = 4;
				break;
			case 6:
				map_he_cap->tx_stream = 5;
				break;
			case 7:
				map_he_cap->tx_stream = 6;
				break;
			case 8:
				map_he_cap->tx_stream = 7;
				break;
			default:
				printf("%s: - Unknow HE Tx Spatial Stream!! \n", __FUNCTION__);
				break;
		}

		switch (he_cap->rx_stream) {
			case 1:
				map_he_cap->rx_stream = 0;
				break;
			case 2:
				map_he_cap->rx_stream = 1;
				break;
			case 3:
				map_he_cap->rx_stream = 2;
				break;
			case 4:
				map_he_cap->rx_stream = 3;
				break;
			case 5:
				map_he_cap->rx_stream = 4;
				break;
			case 6:
				map_he_cap->rx_stream = 5;
				break;
			case 7:
				map_he_cap->rx_stream = 6;
				break;
			case 8:
				map_he_cap->rx_stream = 7;
				break;
			default:
				printf("%s: - Unknow HE Rx Spatial Stream!! \n", __FUNCTION__ );
				break;
		}
		map_he_cap->he_160 = he_cap->he_160;
		map_he_cap->he_8080 = he_cap->he_8080;
		map_he_cap->he_mcs_len = he_cap->he_mcs_len;
		os_memcpy(map_he_cap->he_mcs, he_cap->he_mcs, he_cap->he_mcs_len);
		map_he_cap->su_bf_cap = he_cap->su_bf_cap;
		map_he_cap->mu_bf_cap = he_cap->mu_bf_cap;
		map_he_cap->dl_mu_mimo_ofdma_cap = he_cap->dl_mu_mimo_ofdma_cap;
		map_he_cap->dl_ofdma_cap = he_cap->dl_ofdma_cap;
		map_he_cap->ul_mu_mimo_ofdma_cap = he_cap->ul_mu_mimo_ofdma_cap;
		map_he_cap->ul_mu_mimo_cap = he_cap->ul_mu_mimo_cap;
		map_he_cap->ul_ofdma_cap = he_cap->ul_ofdma_cap;
	}

	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}
void wapp_reset_backhaul_config(struct wifi_app *wapp, struct wapp_dev *wappdev)
{
	struct wapp_dev *wdev;
	struct dl_list *dev_list;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	char cmd[256] = {0};
	 /* This if condition should only be called during wapp init*/
	if (wappdev && wappdev->dev_type == WAPP_DEV_TYPE_AP) {
		sprintf(cmd, "iwpriv %s set fhbss=0;", wappdev->ifname);
		system(cmd);
		sprintf(cmd, "iwpriv %s set fhbss=1;", wappdev->ifname);
		system(cmd);
		os_memset(cmd, 0, 256);
	} else {
		dev_list = &wapp->dev_list;
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
			if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
				sprintf(cmd, "iwpriv %s set fhbss=0;", wdev->ifname);
				system(cmd);
				os_memset(cmd, 0, 256);
				if (wdev->i_am_fh_bss) {
					sprintf(cmd, "iwpriv %s set fhbss=1;", wdev->ifname);
					system(cmd);
					os_memset(cmd, 0, 256);
				}
			}
		}
	}
}
#ifdef MAP_R2
int map_build_metric_reporting_info(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf)
{
	struct evt *map_event = NULL;
	u32 *metric_rep_interval = 0;
	int send_pkt_len = 0;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);

	map_event = (struct evt *)evt_buf;
	map_event->type = WAPP_METRIC_REP_INTERVAL_CAP;
	map_event->length = sizeof(u32);
	metric_rep_interval = (u32 *)map_event->buffer;

	*metric_rep_interval = wapp->map->metric_rep_intv;
	DBGPRINT(RT_DEBUG_OFF, "map_build_metric_reporting_info %d\n", *metric_rep_interval);

	send_pkt_len = sizeof(*map_event) + map_event->length;
	return send_pkt_len;
}

#endif
#ifdef STOP_BEACON_FEATURE
int map_cmd_set_beacon_state( struct wifi_app *wapp, const char *iface, u8 argc, char **argv)
{
	struct wapp_dev *ap_wdev = NULL;
	char idfr[MAC_ADDR_LEN];
	char cmd[256];
	u8 i;

	DBGPRINT(RT_DEBUG_TRACE, "%s \n", __func__);

	if (!argv[1]) {
		DBGPRINT(RT_DEBUG_ERROR, "Invalid argument\n");
		return WAPP_INVALID_ARG;
	}

	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		if (wapp->radio[i].adpt_id) {
			struct wapp_radio *ra = &wapp->radio[i];
			MAP_GET_RADIO_IDNFER(ra, idfr);
			ap_wdev = wapp_dev_list_lookup_by_radio(wapp, idfr);
			os_memset(cmd, 0, 256);
			if (os_strcmp(argv[1], "en") == 0) {
				sprintf(cmd, "iwpriv %s set V10Converter=1;", ap_wdev->ifname); }
			else {
				sprintf(cmd, "iwpriv %s set V10Converter=0;", ap_wdev->ifname); }

			system(cmd);
		}
	}
	return WAPP_SUCCESS;
}
#endif
