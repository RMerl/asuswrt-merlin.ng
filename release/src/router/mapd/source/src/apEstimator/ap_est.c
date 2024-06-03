/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2011, Mediatek, Inc.
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
 *  AP Estimator
 *
 *  Abstract:
 *  AP Estimator
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Neelansh.M   2018/05/02     First implementation of the ap est. Module
 * */
#include "includes.h"
#include "interface.h"
#include "./../1905_local_lib/data_def.h"
#include "common.h"
#include "steer_fsm.h"
#include "list.h"
#include "client_db.h"
#include "mapd_i.h"
#include "chan_mon.h"
#ifdef SUPPORT_MULTI_AP
#include "topologySrv.h"
#include "tlv_parsor.h"
#endif
#include "client_mon.h"
#include "steer_action.h"
#include "ap_est.h"
#include "wapp_if.h"
#include "eloop.h"
#include <assert.h>

#include <sys/un.h>
#ifdef SUPPORT_MULTI_AP
#include "1905_map_interface.h"
#include "ch_planning.h"
#endif
#ifdef CENT_STR
#include "ap_cent_str.h"
#endif


static void ap_est_handle_11k_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct client *cli = (struct client *)timeout_ctx;
	u8 idx = cli->meas_data.curr_measurement_chan_idx;


	mapd_printf(MSG_ERROR, "11k Timedout for channel = %d",
					cli->meas_data.measurement_channels[idx].channel);

	if (cli->meas_data.cli_measurement_state != MEAS_STATE_11K_TRIGGERED) {
		mapd_printf(MSG_ERROR, "Timeout when not triggered!!");
		mapd_ASSERT(0);
	}




	if (cli->meas_data.measurement_retries_11k < MAX_11K_RETRIES) {
		cli->meas_data.measurement_retries_11k ++;
		mapd_printf(MSG_INFO, "11k Retry(%d) on channel = %d",
						 cli->meas_data.measurement_retries_11k,
						 cli->meas_data.measurement_channels[idx].channel);

		if (ap_est_trigger_11k_channel_measurement(global,cli) == FALSE) {
			mapd_printf(MSG_ERROR, "Channel meas Failed");
			steer_fsm_trigger(global, cli->client_id, CHAN_MEASUREMENT_FAIL, NULL);
		}
		return;
	}
	mapd_printf(MSG_INFO, "Retries exhausted for channel = %d",
					cli->meas_data.measurement_channels[idx].channel);

	cli->meas_data.measurement_retries_11k = 0;
	/* Move to next channel */
	if(cli->meas_data.curr_measurement_chan_idx < (cli->meas_data.meas_chan_cnt - 1)) {
			cli->meas_data.curr_measurement_chan_idx++;
			idx = cli->meas_data.curr_measurement_chan_idx;
			mapd_printf(MSG_INFO, "Triggering 11k for next channel = %d",
							cli->meas_data.measurement_channels[idx].channel);
		if (ap_est_trigger_11k_channel_measurement(global,cli) == FALSE) {
			mapd_printf(MSG_ERROR, "Channel meas Failed");
			steer_fsm_trigger(global, cli->client_id, CHAN_MEASUREMENT_FAIL, NULL);

		}
	} else {
		/* Trigger Complete */
			mapd_printf(MSG_INFO, "Channel meas completed");
			steer_fsm_trigger(global, cli->client_id, CHAN_MEASUREMENT_COMPLETE, NULL);
	}
}

/**
 * @brief : Call the beacon metrics query function within wlanif to initiate a
 * beacon request for a client
 *
 * @param global: global device pointer
 * @param cli: client for which beacon metrics query is being initiated
 */
Boolean ap_est_trigger_11k_channel_measurement(struct mapd_global *global,
				struct client *cli)
{
	u8 *ssid = NULL;
	u8 idx = cli->meas_data.curr_measurement_chan_idx;
	u8 wildcard[] = {0xff,0xff,0xff,0xff,0xff,0xff};
#ifdef CENT_STR
	struct _1905_map_device *dev = NULL;
	struct bss_info_db * bss_db = NULL;

	dev = topo_srv_get_1905_by_bssid(&global->dev,cli->bssid);


	if(!dev)
	 return FALSE;

	bss_db = topo_srv_get_bss_by_bssid(&global->dev, dev, cli->bssid);

	if(!bss_db)
	 return FALSE;


	if (global->dev.cent_str_en)
		ssid = bss_db->ssid;
	else
#endif
		ssid = mapd_get_ssid_from_bssid(global, cli->bssid);

	if (
#ifdef SUPPORT_MULTI_AP
		global->params.Certification ||
#endif
		idx <= cli->meas_data.meas_chan_cnt - 1) {
		mapd_printf(MSG_INFO, MACSTR " idx=%d SSID=%s Channel=%d OpClass=%d",
						MAC2STR(cli->mac_addr), idx, (char *)ssid,
						cli->meas_data.measurement_channels[idx].channel,
						cli->meas_data.measurement_channels[idx].op_class);
#ifdef CENT_STR
		if((!global->dev.cent_str_en) || 
			(global->dev.cent_str_en && ( dev->device_role == DEVICE_ROLE_CONTROLLER))) 
#endif
		wlanif_beacon_metrics_query(global, cli->mac_addr, cli->bssid, strlen((char *)ssid), ssid,
						cli->meas_data.measurement_channels[idx].channel,
						cli->meas_data.measurement_channels[idx].op_class, wildcard,
						0, 0, NULL, 0, NULL);
#ifdef CENT_STR
			else 
			topo_srv_trigger_beacon_metrics_query(global, dev, cli->mac_addr, cli->bssid, strlen((char *)ssid), ssid,
									cli->meas_data.measurement_channels[idx].channel,
									cli->meas_data.measurement_channels[idx].op_class, wildcard,
									0, 0, NULL, 0, NULL);

#endif
		cli->meas_data.cli_measurement_state = MEAS_STATE_11K_TRIGGERED;
		eloop_register_timeout(2, 0, ap_est_handle_11k_timeout, global, cli);
		cli->steer_stats.num_11k ++;
		return TRUE;
	} else {
		mapd_printf(MSG_ERROR, "No channels");
		mapd_ASSERT(0);
	}
	return FALSE;
}

/**
 * @brief : handle 11k report from driver (by noting the dl_rssi) called from the
 * wlanif
 *
 * @param global: Global device pointer
 * @param mac_addr: Mac Adress of the client from which report is received
 * @param isSuccess: If the channel measurement was successful, 1 means success
 * @param bssid: bssid on which dl_rssi has been received
 * @param rcpi: the dl_rcpi value received from the beacon report
 */
void ap_est_handle_11k_report(struct mapd_global *global, u8 *mac_addr,
		uint8_t isSuccess, u8 *bssid, u8 channel, u8 rcpi,u8 islastreport)
{
	struct cli_rssi *dl_rssi = NULL;
	struct client *cli = client_db_get_client_from_sta_mac(global, mac_addr);
	u8 idx = 0;
	uint8_t band_idx;
	int8_t curr_rssi = 0;
#ifdef CENT_STR
	struct _1905_map_device * own_device = NULL;
	struct associated_clients * client_dev = NULL;
	

#endif

	if (cli == NULL) {
		mapd_printf(MSG_ERROR, MACSTR " not in DB", MAC2STR(mac_addr));
		return;
	}


#ifdef CENT_STR
	if(global->dev.cent_str_en){


		own_device = topo_srv_get_1905_by_bssid(&global->dev, cli->bssid);

		if (!own_device) {
			mapd_printf(MSG_ERROR," own device is NULL");
			return;
		}
		
		client_dev = topo_srv_get_associate_client(&global->dev,own_device, cli->mac_addr);

		if (!client_dev) {
			mapd_printf(MSG_ERROR," client dev is NULL");
			return;
		}

	}
#endif

	idx = cli->meas_data.curr_measurement_chan_idx;

	if (cli->meas_data.cli_measurement_state != MEAS_STATE_11K_TRIGGERED) {
		mapd_printf(MSG_ERROR, "Stale 11k report/Channel_ID --ignore");
		return;
	}

	if(!isSuccess || (channel == 0)) {
		eloop_cancel_timeout(ap_est_handle_11k_timeout, global, cli);
		mapd_printf(MSG_ERROR, "11k Failed for channel = %d",
						cli->meas_data.measurement_channels[idx].channel);

#ifdef CENT_STR
			/*Disable steering if radar detected orchannel planning or network opt is ongoing*/
			if(global->dev.cent_str_en && is_chplan_netopt_ongoing(global)){
				steer_fsm_trigger(global, cli->client_id, CHAN_MEASUREMENT_DISALLOWED, NULL);
				return;
			}
#endif
		/* Move to Next Channel */
		if(cli->meas_data.curr_measurement_chan_idx < (cli->meas_data.meas_chan_cnt - 1)) {
			cli->meas_data.curr_measurement_chan_idx++;
			idx = cli->meas_data.curr_measurement_chan_idx;
			mapd_printf(MSG_INFO, "Triggering 11k for next channel = %d",
							cli->meas_data.measurement_channels[idx].channel);
			if(ap_est_trigger_11k_channel_measurement(global,cli) == FALSE){
				mapd_printf(MSG_ERROR, "Channel meas Fail");
				steer_fsm_trigger(global, cli->client_id, CHAN_MEASUREMENT_FAIL, NULL);
			}


		} else {
			mapd_printf(MSG_INFO, "Channel meas completed");
			steer_fsm_trigger(global, cli->client_id, CHAN_MEASUREMENT_COMPLETE, NULL);
		}
		return;
	}

	if(channel == 0) {
		mapd_printf(MSG_ERROR, "11k report for a 0 channel--ignore");
		return;
	}

	if (cli->meas_data.measurement_channels[idx].channel != channel) {
		mapd_printf(MSG_ERROR, "11k report for a different channel %d %d ",
									cli->meas_data.measurement_channels[idx].channel,channel);
	}

        if(channel > 14)
                band_idx = BAND_5G_IDX;
        else
                band_idx = BAND_2G_IDX;


	dl_rssi = (struct cli_rssi *)os_zalloc(sizeof(struct cli_rssi));
	if (dl_rssi == NULL) {
		mapd_printf(MSG_ERROR, "OOM");
		assert(0);
	}

	mapd_printf(MSG_INFO, MACSTR " RSSI=%d BSSID= " MACSTR, MAC2STR(mac_addr), (s8)rcpi, MAC2STR(bssid));
#ifdef VENDOR1_FEATURE_EXTEND
	mapd_printf(MSG_ERROR, "STA:" MACSTR " BSSID:" MACSTR " RCPI:%d(0x%x) RSSI:%d",
		MAC2STR(mac_addr), MAC2STR(bssid), rcpi, ((rcpi) & 0xff),
		rcpi_to_rssi(rcpi));
#endif //VENDOR1_FEATURE_EXTEND

#if 0
	/* 
	As per spec, a STA should awalys send RCPI in the Beacon Report.
	But its observed that most of the Non-HE STAs sending RSSI.
	Needs to  be revisted
	*/
        if(cli->phy_capab.phy_mode[band_idx] >= HE_MODE)
                dl_rssi->rssi = rcpi_to_rssi(rcpi);
        else
                dl_rssi->rssi = rcpi;

#endif
	dl_rssi->rssi = rcpi;
	curr_rssi = cli->ul_rssi[cli->radio_idx];
#ifdef CENT_STR
	if(global->dev.cent_str_en)
		curr_rssi = (signed char)client_dev->rssi_uplink;
#endif
	
	if(!os_memcmp(cli->bssid,bssid,ETH_ALEN)){

		if(rcpi > 220/*0 dbm*/
			|| (rcpi < 10 && (curr_rssi - rcpi_to_rssi(rcpi)) > 60)/*RCPI=-105dBm but RSSI=10 dBm*/
			|| (rcpi > 148 && (rcpi_to_rssi(rcpi) + dl_rssi->rssi - 2*curr_rssi) > 20) /*RCPI=-36 dBm  but RSSI = -108dBm*/ 
		)
			cli->rssi_based_rcpi = 1;
		else
			cli->rssi_based_rcpi = 0;
		

	}

	dl_rssi->channel = channel;
	os_memcpy(&dl_rssi->bssid[0], bssid, ETH_ALEN);
	dl_list_add(&cli->meas_data.dl_rssi_list, &dl_rssi->rssi_entry);
	if(islastreport) {
		eloop_cancel_timeout(ap_est_handle_11k_timeout, global, cli);
		mapd_printf(MSG_ERROR, "11k Success for channel = %d",
						cli->meas_data.measurement_channels[idx].channel);
		cli->steer_stats.num_11k_succ ++;
		if(!cli->rssi_based_rcpi) {
			struct cli_rssi *rssi_obj = NULL;

			dl_list_for_each(rssi_obj,&cli->meas_data.dl_rssi_list,struct cli_rssi,rssi_entry){
				rssi_obj->rssi = rcpi_to_rssi(rssi_obj->rssi);

			}
		}

#ifdef CENT_STR
		/*Disable steering if radar detected orchannel planning or network opt is ongoing*/
		if(global->dev.cent_str_en && is_chplan_netopt_ongoing(global)){
			steer_fsm_trigger(global, cli->client_id, CHAN_MEASUREMENT_DISALLOWED, NULL);
			return;
		}
#endif

		
		
		/* Move to next channel */
		if (cli->meas_data.curr_measurement_chan_idx < (cli->meas_data.meas_chan_cnt - 1)) {
			cli->meas_data.curr_measurement_chan_idx++;
			idx = cli->meas_data.curr_measurement_chan_idx;
			mapd_printf(MSG_INFO, "Triggering 11k for next channel = %d",
							cli->meas_data.measurement_channels[idx].channel);
			if (ap_est_trigger_11k_channel_measurement(global,cli) == FALSE) {
				mapd_printf(MSG_ERROR, "Channel meas Failed");
				steer_fsm_trigger(global, cli->client_id, CHAN_MEASUREMENT_FAIL, NULL);

			}

		} else {
			/* Trigger Complete */
			mapd_printf(MSG_INFO, "Channel meas completed");
			steer_fsm_trigger(global, cli->client_id, CHAN_MEASUREMENT_COMPLETE, NULL);
		}
	}
}
#ifdef SUPPORT_MULTI_AP
/**
* @brief timeout happened for air monitor response. trigger the state machine
*
* @param eloop_ctx pointer to the global structure
* @param user_ctx client for which air monitor timeout happened
*/
static void ap_est_air_mon_timeout(void *eloop_ctx, void *user_ctx)
{
	struct mapd_global *pGlobal_dev = eloop_ctx;
	struct client *map_client = user_ctx;


	mapd_printf(MSG_INFO, "Air Mon Timedout");
	mapd_printf(MSG_INFO, "Trigger FSM (CHAN_MEASUREMENT_COMPLETE)");

				steer_fsm_trigger(pGlobal_dev, map_client->client_id, CHAN_MEASUREMENT_COMPLETE, NULL);
}

/**
* @brief trigger air monitor for the client. send un-assoc link metric query to all devices that support it and wait for their response.
*
* @param pGlobal_dev pointer to the global structure
* @param client_id client for which air monitor see
*/
Boolean ap_est_trigger_air_mon(struct mapd_global *pGlobal_dev,int client_id)
{
	struct _1905_map_device *_1905_device = NULL;
	struct client *map_client = client_db_get_client_from_client_id(pGlobal_dev, client_id);

	if(map_client == NULL) {
		mapd_printf(MSG_ERROR,"invalid client id");
		assert(0);
	}
	
	_1905_device = topo_srv_get_next_1905_device(&pGlobal_dev->dev, NULL); /*Get own device struct.*/
	_1905_device = topo_srv_get_next_1905_device(&pGlobal_dev->dev, _1905_device);

	if(_1905_device == NULL) {
		mapd_printf(MSG_ERROR,"Multiple devices not present. Can't reach here.");
		return FALSE;
	}

	while(_1905_device) {
		struct bss_info_db *map_bss = topo_srv_get_next_bss(_1905_device, NULL);

		while (_1905_device->in_network && map_bss != NULL) {
			if(map_client->current_chan == map_bss->radio->channel[0] ||
				map_client->current_chan == map_bss->radio->channel[1] ||
				map_client->current_chan == map_bss->radio->channel[2] ||
				map_client->current_chan == map_bss->radio->channel[3] ) { // TODO: Raghav:check op class also ?
				struct unassoc_sta_link_metrics_query *metric_query = os_zalloc(sizeof(struct unassoc_sta_link_metrics_query ) + sizeof(struct unassoc_sta_link_metrics_query_sub));
				int ret = 0;
				struct bss_air_mon_report *air_mon_bss = os_zalloc(sizeof(struct bss_air_mon_report));
				if(air_mon_bss == NULL) {
					mapd_printf(MSG_ERROR,"%s: can't alloc mem\n", __func__);
					assert(0);
				}
				//os_memset(&metric_query,0,sizeof(struct unassoc_sta_link_metrics_query));
				os_memcpy(air_mon_bss->al_mac, _1905_device->_1905_info.al_mac_addr, ETH_ALEN);
				os_memcpy(air_mon_bss->bssid, map_bss->bssid, ETH_ALEN);
				air_mon_bss->report_recieved = 0;
				air_mon_bss->rssi = -110;
				metric_query->ch_num = 1;
				metric_query->op_class = map_bss->radio->operating_class;
				
				metric_query->unassoc_link_query_sub[0].channel = map_client->current_chan;
				metric_query->unassoc_link_query_sub[0].sta_num = 1;
				os_memcpy(&metric_query->unassoc_link_query_sub[0].sta_mac[0], map_client->mac_addr, ETH_ALEN);
				
				ret = map_1905_Send_Unassociated_STA_Link_Metrics_Query_Message(pGlobal_dev->_1905_ctrl,
						(char *)_1905_device->_1905_info.al_mac_addr, metric_query);
				os_free(metric_query);
				if(ret == 0) {
					SLIST_INSERT_HEAD(&map_client->meas_data.air_mon_bss_head, air_mon_bss,air_mon_bss_entry);
					// Make sure to free this memory after steering.
					map_client->meas_data.air_mon_sent_cnt++;
					break;
				}
				else
					os_free(air_mon_bss);
			}
			map_bss = topo_srv_get_next_bss(_1905_device, map_bss);
		}
		_1905_device = topo_srv_get_next_1905_device(&pGlobal_dev->dev,_1905_device);
	}
	
	if(!SLIST_EMPTY(&(map_client->meas_data.air_mon_bss_head))) {
		map_client->meas_data.cli_measurement_state = MEAS_STATE_AIR_MON_TRIGGERED;
		map_client->meas_data.air_mon_timer_running = TRUE;
		mapd_printf(MSG_ERROR, "Air Mon Triggered on channel %d", map_client->current_chan);
		eloop_register_timeout(AIR_MON_TIMEOUT, 0,ap_est_air_mon_timeout,pGlobal_dev,map_client);
	}
	else
		return FALSE;

	return TRUE;
}

#ifdef CENT_STR
Boolean ap_est_trigger_air_mon_cent_str(struct mapd_global *pGlobal_dev,int client_id)
{

	struct client *map_client = client_db_get_client_from_client_id(pGlobal_dev, client_id);
	struct _1905_map_device *cur_1905_device = NULL;
	struct _1905_map_device *_1905_device = NULL;
	struct bss_info_db *map_bss = NULL;
	int ret = 0;

	if(map_client == NULL) {
		mapd_printf(MSG_ERROR,"invalid client id");
		return FALSE;
	}

	cur_1905_device = topo_srv_get_1905_by_bssid(&pGlobal_dev->dev, map_client->bssid);


	if(cur_1905_device == NULL) {
		mapd_printf(MSG_ERROR,"current 1905 device is NULL");
		return FALSE;
	}

	
	_1905_device = topo_srv_get_next_1905_device(&pGlobal_dev->dev, NULL); /*Get own device struct.*/


	if(_1905_device == NULL) {
		mapd_printf(MSG_ERROR,"Multiple devices not present. Can't reach here.");
		return FALSE;
	}

	

	while(_1905_device) {
		
		if(_1905_device == cur_1905_device){
			_1905_device = topo_srv_get_next_1905_device(&pGlobal_dev->dev,_1905_device);
			continue;
		}
		
		map_bss = topo_srv_get_next_bss(_1905_device, NULL);

		while (_1905_device->in_network && map_bss != NULL) {
			if(map_client->current_chan == map_bss->radio->channel[0] ||
				map_client->current_chan == map_bss->radio->channel[1] ||
				map_client->current_chan == map_bss->radio->channel[2] ||
				map_client->current_chan == map_bss->radio->channel[3] ) { // TODO: Raghav:check op class also ?

				struct bss_air_mon_report *air_mon_bss = os_zalloc(sizeof(struct bss_air_mon_report));

				if(_1905_device->device_role == DEVICE_ROLE_AGENT) {

				struct unassoc_sta_link_metrics_query *metric_query = os_zalloc(sizeof(struct unassoc_sta_link_metrics_query ) + sizeof(struct unassoc_sta_link_metrics_query_sub));


				if(air_mon_bss == NULL) {
					mapd_printf(MSG_ERROR,"%s: can't alloc mem\n", __func__);
					assert(0);
				}
				//os_memset(&metric_query,0,sizeof(struct unassoc_sta_link_metrics_query));
				os_memcpy(air_mon_bss->al_mac, _1905_device->_1905_info.al_mac_addr, ETH_ALEN);
				os_memcpy(air_mon_bss->bssid, map_bss->bssid, ETH_ALEN);
				air_mon_bss->report_recieved = 0;
				air_mon_bss->rssi = -110;
				metric_query->ch_num = 1;
				metric_query->op_class = map_bss->radio->operating_class;
				
				metric_query->unassoc_link_query_sub[0].channel = map_client->current_chan;
				metric_query->unassoc_link_query_sub[0].sta_num = 1;
				os_memcpy(&metric_query->unassoc_link_query_sub[0].sta_mac[0], map_client->mac_addr, ETH_ALEN);

				
	
					ret = map_1905_Send_Unassociated_STA_Link_Metrics_Query_Message(pGlobal_dev->_1905_ctrl,
							(char *)_1905_device->_1905_info.al_mac_addr, metric_query);

				os_free(metric_query);
				} else {

					u8 unlink_query[50];
					struct unlink_metrics_query *p_unlink_query = (struct unlink_metrics_query *)unlink_query;	
					int query_len = 0;
					os_memcpy(air_mon_bss->al_mac, _1905_device->_1905_info.al_mac_addr, ETH_ALEN);
					os_memcpy(air_mon_bss->bssid, map_bss->bssid, ETH_ALEN);
					air_mon_bss->report_recieved = 0;
					air_mon_bss->rssi = -110;
					p_unlink_query->ch_num = 1;
					p_unlink_query->oper_class = map_bss->radio->operating_class;
					p_unlink_query->ch_list[0]= map_client->current_chan;
					p_unlink_query->sta_num = 1;
					os_memcpy(&p_unlink_query->sta_list[0], map_client->mac_addr, ETH_ALEN);				


					query_len = sizeof(struct unlink_metrics_query) + p_unlink_query->sta_num * ETH_ALEN;			

					ret = map_get_info_from_wapp(&pGlobal_dev->dev,
				
								   WAPP_USER_SET_AIR_MONITOR_REQUEST,
								   0, NULL, NULL,
								   (void *)p_unlink_query, query_len);

				}
				if(ret == 0) {
					SLIST_INSERT_HEAD(&map_client->meas_data.air_mon_bss_head, air_mon_bss,air_mon_bss_entry);
					// Make sure to free this memory after steering.
					map_client->meas_data.air_mon_sent_cnt++;
					break;
				}
				else
					os_free(air_mon_bss);
			}
			map_bss = topo_srv_get_next_bss(_1905_device, map_bss);
		}
		_1905_device = topo_srv_get_next_1905_device(&pGlobal_dev->dev,_1905_device);
	}
	
	if(!SLIST_EMPTY(&(map_client->meas_data.air_mon_bss_head))) {
		map_client->meas_data.cli_measurement_state = MEAS_STATE_AIR_MON_TRIGGERED;
		map_client->meas_data.air_mon_timer_running = TRUE;
		mapd_printf(MSG_ERROR, "Air Mon Triggered on channel %d", map_client->current_chan);
		eloop_register_timeout(AIR_MON_TIMEOUT, 0,ap_est_air_mon_timeout,pGlobal_dev,map_client);
	}
	else
		return FALSE;

	return TRUE;
}

void ap_est_handle_own_unassoc_sta_link_metric_rsp(struct mapd_global *pGlobal_dev,
						struct unassoc_link_metric_rsp *unassoc_link_metric_msg,
						u32 msg_len)
{
	struct _1905_map_device *_1905_device = NULL;
	struct client * map_client = NULL;
	u8 cli_cnt = 0, i=0;
	struct bss_air_mon_report *air_mon_report = NULL;
#ifdef CENT_STR	
	struct own_1905_device *dev = &pGlobal_dev->dev;	
#endif
	_1905_device = topo_srv_get_1905_device(&pGlobal_dev->dev,NULL);


	cli_cnt = unassoc_link_metric_msg->sta_num;

	if(cli_cnt > 1) {
		mapd_printf(MSG_ERROR,"Cli cnt: %d. Something is wrong\n", cli_cnt);
	}

	for(i=0; i < cli_cnt; i++) {
		u8 *cli_mac = unassoc_link_metric_msg->sta_link_metric[i].cli_mac;

		map_client = client_db_get_client_from_sta_mac(pGlobal_dev,cli_mac);

		if(map_client == NULL
		|| map_client->cli_steer_state != CLI_STATE_STEER_DECISION
		|| map_client->meas_data.cli_measurement_state != MEAS_STATE_AIR_MON_TRIGGERED)
			continue;

		if(!SLIST_EMPTY(&(map_client->meas_data.air_mon_bss_head)))
		{
			SLIST_FOREACH(air_mon_report, &(map_client->meas_data.air_mon_bss_head),
												air_mon_bss_entry)
			{
				if(!os_memcmp(air_mon_report->al_mac, _1905_device->_1905_info.al_mac_addr, ETH_ALEN)) {
					air_mon_report->rssi = rcpi_to_rssi(unassoc_link_metric_msg->sta_link_metric[i].rssi);
					air_mon_report->delta_time = unassoc_link_metric_msg->sta_link_metric[i].delta_time;
					if(air_mon_report->report_recieved  == 1)
						break;
					air_mon_report->report_recieved  = 1;
					err("AirMon result :BSS: %x:%x:%x:%x:%x:%x AL= %x:%x:%x:%x:%x:%x: RSSI: %d",PRINT_MAC(air_mon_report->bssid),
																							PRINT_MAC(air_mon_report->al_mac),
																							air_mon_report->rssi);
					map_client->meas_data.air_mon_rx_cnt++;
					break;
				}
			}
		}

#ifdef CENT_STR
			/*Disable steering if radar detected orchannel planning or network opt is ongoing*/
			if(dev->cent_str_en && is_chplan_netopt_ongoing(pGlobal_dev)){
				eloop_cancel_timeout(ap_est_air_mon_timeout, pGlobal_dev,map_client);
				steer_fsm_trigger(pGlobal_dev, map_client->client_id, CHAN_MEASUREMENT_DISALLOWED, NULL);
				return;					
			
			}
#endif


		if(map_client->meas_data.air_mon_rx_cnt == map_client->meas_data.air_mon_sent_cnt) {

			mapd_printf(MSG_INFO, "All air mon rsp received-->SUCCESS");
			eloop_cancel_timeout(ap_est_air_mon_timeout, pGlobal_dev,map_client);
			mapd_printf(MSG_INFO, "%s: Trigger FSM (CHAN_MEASUREMENT_COMPLETE)", __func__);
			steer_fsm_trigger(pGlobal_dev, map_client->client_id, CHAN_MEASUREMENT_COMPLETE, NULL);
		}
	}
}


void topo_srv_trigger_beacon_metrics_query(struct mapd_global *global, struct _1905_map_device *dev,u8 *sta_mac,
				u8 *assoc_bssid, u8 ssid_len, u8 *ssid,
				u8 channel, u8 op_class, u8 *bssid,
				u8 rpt_detail, u8 num_elem, char *elem_list,
				u8 num_chrep, struct ap_chn_rpt *chan_rpt)
{
	struct beacon_metrics_query *bcn_query = NULL;
	struct ap_chn_rpt *chn_rpt = NULL;
	uint8_t i = 0;


	if (!dev) {
		mapd_printf(MSG_ERROR, "dev is NULL");
		return;
	}

	mapd_printf(MSG_INFO, "Preparing to send Beacon metrics query");

	bcn_query = (struct beacon_metrics_query *)
			os_zalloc(sizeof(struct beacon_metrics_query) + num_chrep * sizeof(struct ap_chn_rpt));
	if (!bcn_query) {
		mapd_printf(MSG_ERROR, "FAILED OOM");
		return;
	}

	os_memcpy(bcn_query->sta_mac, sta_mac, ETH_ALEN);
	os_memcpy(bcn_query->bssid, bssid, ETH_ALEN);
	bcn_query->ssid_len = ssid_len;
	bcn_query->ch = channel;
	bcn_query->oper_class = op_class;
	os_memcpy(bcn_query->ssid, ssid, ssid_len);

	bcn_query->rpt_detail_val = rpt_detail;

	/*ap channel report info*/
	bcn_query->ap_ch_rpt_num = num_chrep;
	chn_rpt = bcn_query->rpt;
	for (i = 0; i < num_chrep; i++) {
			chn_rpt->ch_rpt_len = chan_rpt->ch_rpt_len;
			chn_rpt->oper_class = chan_rpt->oper_class;
			memcpy(chn_rpt->ch_list, chan_rpt->ch_list, chan_rpt->ch_rpt_len - 1);
			chn_rpt++;
	}

	bcn_query->elemnt_num = num_elem;
	if (num_elem > MAX_ELEMNT_NUM)
			bcn_query->elemnt_num = MAX_ELEMNT_NUM;
	os_memcpy(bcn_query->elemnt_list, elem_list, bcn_query->elemnt_num);

	map_1905_Send_Beacon_Metrics_Query_Message(global->_1905_ctrl,(char *)dev->_1905_info.al_mac_addr ,bcn_query);
	
	os_free(bcn_query);
}

#endif

#endif
/**
 * @brief : Trigger channel measurement (802.11k or air monitor as required), called
 * from the state machine when decision state is active for a client
 *
 * @param pGlobal_dev: Global device pointer
 * @param client_id: client ID of client for which measurement is to be triggered
 * @param data: Decision state specific data containing steering type
 */
Boolean ap_est_trigger_channel_measurement(struct mapd_global *pGlobal_dev,
		int client_id, DECISION_DATA *data)
{
	struct client *map_client = client_db_get_client_from_client_id(pGlobal_dev, client_id);

	if(map_client == NULL) {
		mapd_printf(MSG_ERROR,"invalid client id:%d", client_id);
		mapd_ASSERT(0);
	}
	if((map_client->capab & CLI_CAP_11K) &&
		(map_client->force_airmon != 1)) {
		return ap_est_trigger_11k_channel_measurement(pGlobal_dev, map_client);
#ifdef SUPPORT_MULTI_AP
	} else {
		/* Should reach here only when the clien is idle */
		/* For Legacy(non 11k) or 11k failed(force_airmon) idle clients */
		mapd_ASSERT(map_client->activity_state == 0);
		map_client->force_airmon = 0;
#ifdef CENT_STR
			if (pGlobal_dev->dev.cent_str_en)
				return ap_est_trigger_air_mon_cent_str(pGlobal_dev, client_id);	
			else
#endif		
		return ap_est_trigger_air_mon(pGlobal_dev, client_id);
#endif
	}
}
#ifdef SUPPORT_MULTI_AP
/**
 * @brief : handle unassoc sta link metrics
 *
 * @param pGlobal_dev
 * @param unassoc_link_metric_msg
 * @param msg_len
 */
void ap_est_handle_unassoc_sta_link_metric_rsp(struct mapd_global *pGlobal_dev,
						struct unassoc_link_metric_tlv_rsp *rsp,
						u32 msg_len)
{
	struct _1905_map_device *_1905_device = NULL;
	struct unassoc_link_metric_rsp *unassoc_link_metric_msg = NULL;
	struct client * map_client = NULL;
	u8 cli_cnt = 0, i=0;
	struct bss_air_mon_report *air_mon_report = NULL;
#ifdef CENT_STR	
	struct own_1905_device *dev = &pGlobal_dev->dev;	
#endif	

	unassoc_link_metric_msg = &rsp->unasssoc_link_rsp;
	_1905_device = topo_srv_get_1905_device(&pGlobal_dev->dev,rsp->al_mac);

	if(_1905_device == NULL) {
		mapd_printf(MSG_ERROR,"%s: Unknown 1905 device. Drop\n", __func__);
		return;
	}
	if(msg_len < sizeof(struct unassoc_link_metric_rsp)) {
		mapd_printf(MSG_ERROR,"%s: Invalid msg len\n", __func__);
	}
	
	cli_cnt = unassoc_link_metric_msg->sta_num;

	if(cli_cnt > 1) {
		mapd_printf(MSG_ERROR,"Cli cnt: %d. Something is wrong\n", cli_cnt);
	}

	for(i=0; i < cli_cnt; i++) {
		u8 *cli_mac = unassoc_link_metric_msg->sta_link_metric[i].cli_mac;

		map_client = client_db_get_client_from_sta_mac(pGlobal_dev,cli_mac);

		if(map_client == NULL
		|| map_client->cli_steer_state != CLI_STATE_STEER_DECISION
		|| map_client->meas_data.cli_measurement_state != MEAS_STATE_AIR_MON_TRIGGERED)
			continue;

		if(!SLIST_EMPTY(&(map_client->meas_data.air_mon_bss_head)))
		{
			SLIST_FOREACH(air_mon_report, &(map_client->meas_data.air_mon_bss_head),
												air_mon_bss_entry)
			{
				if(!os_memcmp(air_mon_report->al_mac, rsp->al_mac, ETH_ALEN)) {
					air_mon_report->rssi = rcpi_to_rssi(unassoc_link_metric_msg->sta_link_metric[i].rssi);
					air_mon_report->delta_time = unassoc_link_metric_msg->sta_link_metric[i].delta_time;
					if(air_mon_report->report_recieved  == 1)
						break;
					air_mon_report->report_recieved  = 1;
					err("AirMon result :BSS: %x:%x:%x:%x:%x:%x AL= %x:%x:%x:%x:%x:%x: RSSI: %d",PRINT_MAC(air_mon_report->bssid),
																							PRINT_MAC(air_mon_report->al_mac),
																							air_mon_report->rssi);
					map_client->meas_data.air_mon_rx_cnt++;
					break;
				}
			}
		}


#ifdef CENT_STR
					/*Disable steering if radar detected orchannel planning or network opt is ongoing*/
			if(dev->cent_str_en && is_chplan_netopt_ongoing(pGlobal_dev)){
					eloop_cancel_timeout(ap_est_air_mon_timeout, pGlobal_dev,map_client);
					steer_fsm_trigger(pGlobal_dev, map_client->client_id, CHAN_MEASUREMENT_DISALLOWED, NULL);
					return;
			}
#endif


		if(map_client->meas_data.air_mon_rx_cnt == map_client->meas_data.air_mon_sent_cnt) {

			mapd_printf(MSG_INFO, "All air mon rsp received-->SUCCESS");
			eloop_cancel_timeout(ap_est_air_mon_timeout, pGlobal_dev,map_client);
			mapd_printf(MSG_INFO, "%s: Trigger FSM (CHAN_MEASUREMENT_COMPLETE)", __func__);
			steer_fsm_trigger(pGlobal_dev, map_client->client_id, CHAN_MEASUREMENT_COMPLETE, NULL);
		}
	}
}
#endif
void ap_est_11k_cleanup(struct mapd_global *global, struct client *cli)
{
       eloop_cancel_timeout(ap_est_handle_11k_timeout, global, cli);
}

