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
 *  Channel planning R2*
 *  Abstract:
 *  Channel Planning
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  * */

#include "includes.h"
#ifdef __linux__
#include <fcntl.h>
#endif				/* __linux__ */

#include "common.h"
#include <sys/un.h>
#include "interface.h"
#include "./../1905_local_lib/data_def.h"
#include "1905_map_interface.h"
#include "client_db.h"
#include "mapd_i.h"
#include "topologySrv.h"
#include "eloop.h"
#include "tlv_parsor.h"
#include "1905_if.h"
#include "wapp_if.h"
#include "mapd_debug.h"
#include "chan_mon.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "apSelection.h"
#include "ch_planning.h"


#ifdef MAP_R2 
void ch_planning_R2_bootup_handling_restart(
	void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *ctx = eloop_ctx;
	uint8_t radio_idx = 0;
	struct mapd_radio_info *radio_info = NULL;
	err("trigger scan again")
	for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++) {
		radio_info = &ctx->dev_radio_info[radio_idx];
		if (radio_info->radio_idx == (uint8_t)-1)
			continue;
		if (radio_info->bootup_run == 3)
			radio_info->bootup_run = 0;
	}
}
u8 find_radio_num(
	struct own_1905_device *ctx,
	struct radio_info_db *radio)
{
	u8 radio_num = 0;
	for(radio_num = 0;radio_num < MAX_NUM_OF_RADIO; radio_num++) {
		if (radio->band == ctx->ch_planning_R2.ch_plan_thres[radio_num].band)
			break;
	}
	return radio_num;
}

void ch_planning_get_avg (struct radio_info_db *radio)
{
	struct dev_ch_monitor *monitor_info =
		&radio->dev_ch_plan_info.dev_ch_monitor_info;
	if ((monitor_info->count_cu_util < MIN_SAMPLE_COUNT)
		||((monitor_info->count_radio_metric < MIN_SAMPLE_COUNT))) {
		err("count less than min sample count avoid this radio in analysis");
		monitor_info->avg_cu_monitor = 0;
		monitor_info->avg_edcca = 0;
		monitor_info->avg_obss_load = 0;
		monitor_info->avg_myTxAirtime = 0;
		monitor_info->avg_myRxAirtime = 0;
		return;
	}
	monitor_info->avg_cu_monitor =
		(monitor_info->avg_cu_monitor /
		monitor_info->count_cu_util);

	monitor_info->avg_edcca =
		(monitor_info->avg_edcca /
			monitor_info->count_edcca_cu_tlv);

	monitor_info->avg_obss_load =
		(monitor_info->avg_obss_load /
			monitor_info->count_radio_metric);

	monitor_info->avg_myTxAirtime =
		(monitor_info->avg_myTxAirtime /
			monitor_info->count_radio_metric);

	monitor_info->avg_myRxAirtime =
		(monitor_info->avg_myRxAirtime /
			monitor_info->count_radio_metric);
}
s32 CalculateCompareOBSS(
	struct own_1905_device *ctx,
	struct _1905_map_device *_1905_device,
	struct monitor_ch_info *ch_info)
{
	u32 TotalMapTxRxAirtime = 0;
	s32 CompareObss = 0;
	struct radio_info_db *temp_radio = NULL;
	struct _1905_map_device *temp_1905_device = NULL;
	temp_1905_device = topo_srv_get_1905_device(ctx, NULL);
	while(temp_1905_device) {
		if(temp_1905_device == _1905_device) {
			temp_1905_device = topo_srv_get_next_1905_device(ctx, temp_1905_device);
			continue;
		}
		temp_radio = topo_srv_get_radio_by_channel(temp_1905_device, ch_info->channel_num);
		if(temp_radio) {
			TotalMapTxRxAirtime = TotalMapTxRxAirtime +
			(temp_radio->dev_ch_plan_info.dev_ch_monitor_info.avg_myTxAirtime +
				temp_radio->dev_ch_plan_info.dev_ch_monitor_info.avg_myRxAirtime);
			err("for dev "MACSTR" avg_myTxAirtime %d avg_myRxAirtime %d",MAC2STR(temp_1905_device->_1905_info.al_mac_addr),
				temp_radio->dev_ch_plan_info.dev_ch_monitor_info.avg_myTxAirtime,
				temp_radio->dev_ch_plan_info.dev_ch_monitor_info.avg_myRxAirtime);
		}
		
		temp_1905_device = topo_srv_get_next_1905_device(ctx,temp_1905_device);
	}
	err("for all dev  TotalMapTxRxAirtime %d", TotalMapTxRxAirtime);
	temp_radio = topo_srv_get_radio_by_channel(_1905_device,ch_info->channel_num);
	if(temp_radio) {
		err("avg_obss_load %d",temp_radio->dev_ch_plan_info.dev_ch_monitor_info.avg_obss_load);
		CompareObss = (temp_radio->dev_ch_plan_info.dev_ch_monitor_info.avg_obss_load
				- TotalMapTxRxAirtime);
	}
	return CompareObss;
}
void dump_ch_planning_update_data(struct radio_info_db *radio)
{
	struct dev_ch_monitor *dev_monitor_info =
			&radio->dev_ch_plan_info.dev_ch_monitor_info;
	always("avg_cu_monitor %d\n",dev_monitor_info->avg_cu_monitor);
	always("avg_edcca %d\n",dev_monitor_info->avg_edcca);
	always("avg_obss_load %d\n",dev_monitor_info->avg_obss_load);
	always("avg_myTxAirtime %d\n",dev_monitor_info->avg_myTxAirtime);
	always("avg_myRxAirtime %d\n",dev_monitor_info->avg_myRxAirtime);
	always("count ch_util %d\n",dev_monitor_info->count_cu_util);
	always("count edcca tlv %d\n",dev_monitor_info->count_edcca_cu_tlv);
	always("count radio metric tlv %d\n",dev_monitor_info->count_radio_metric);
}

void ch_planning_all_dev_avg(struct own_1905_device *ctx,
	struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *_1905_device = NULL;
	struct radio_info_db *radio = NULL;

	_1905_device = topo_srv_get_1905_device(ctx, NULL);
	while(_1905_device) {
		radio =
			topo_srv_get_radio_by_channel(_1905_device, ch_info->channel_num);
		if(radio) {
			ch_planning_get_avg(radio);
			err("For dev "MACSTR"", MAC2STR(_1905_device->_1905_info.al_mac_addr));
			dump_ch_planning_update_data(radio);
		}
	_1905_device = topo_srv_get_next_1905_device(ctx, _1905_device);
	}
	

}
u32 ch_planning_get_max_edcca_airtime(
	struct own_1905_device *ctx,
	struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *_1905_device = NULL;
	struct radio_info_db *radio = NULL;
	u32 Max_EdccaAirtime = 0;
	_1905_device = topo_srv_get_1905_device(ctx, NULL);
	while(_1905_device) {
		radio = topo_srv_get_radio_by_channel(_1905_device, ch_info->channel_num);
		if(radio) {
			if(radio->dev_ch_plan_info.dev_ch_monitor_info.avg_edcca > Max_EdccaAirtime)
				Max_EdccaAirtime = radio->dev_ch_plan_info.dev_ch_monitor_info.avg_edcca;
		}
		_1905_device = topo_srv_get_next_1905_device(ctx, _1905_device);
	}
	return Max_EdccaAirtime;
}

u8 ch_planning_is_trigger_req(
	struct own_1905_device *ctx,
	struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *_1905_device = NULL;
	u8 need_trigger = TRIGGER_FALSE;
	u32 Max_EdccaAirtime = 0;
	struct radio_info_db *radio = NULL;
	u32 EdccaThreshold = EDCCA_THRESHOLD;
	u32 CompareEdcca = 0;
	s32 ObssThreshold = OBSS_THRESHOLD;
	s32 CompareObss = 0 ;
	s32 BkLoadThreshold = BKLOAD_THRESHOLD, CompareBkLoad = 0;
	u8 skip = ctx->ch_planning_R2.skip_edcca_check;
	u8 radio_num = 0;
	_1905_device = topo_srv_get_1905_device(ctx, NULL);
	radio = topo_srv_get_radio_by_channel(_1905_device,ch_info->channel_num);
	if(!radio){
		err("radio is NULL");
	}
	else {
		radio_num = find_radio_num(ctx, radio);
		EdccaThreshold = ctx->ch_planning_R2.ch_plan_thres[radio_num].edcca_threshold;
		ObssThreshold = (s32)ctx->ch_planning_R2.ch_plan_thres[radio_num].obss_load_threshold;
		
	}
	err("Trigger thresholds are EDCCA %d,OBSS %d",EdccaThreshold, ObssThreshold);
	debug("skip %d",skip);
	if (skip == 0) {
		//sonal test later when correct edcca value comes
		Max_EdccaAirtime = ch_planning_get_max_edcca_airtime(ctx, ch_info);
		CompareEdcca = Max_EdccaAirtime;
		if(CompareEdcca > EdccaThreshold) {
			err(" There is strong non-WIFI interference, launch scan mandate");
			need_trigger = TRIGGER_TRUE;
			return need_trigger;
		}
	}

	while(_1905_device) {
		CompareObss = CalculateCompareOBSS(ctx, _1905_device, ch_info);
		if(CompareObss < 0) {
			err("for this DEV ASSUMPTION that ObssAirtime_A2 > (MyTxAirtime_A1 + MyRxAirtime_A1) is wrong");
		}
		err("CompareObss %d", CompareObss);
		CompareBkLoad = CompareEdcca + CompareObss;
		err("CompareBkLoad %d", CompareBkLoad);
		/*If any agent exceeds threshold ,
		then also we will go for trigger ch planning on that radio*/
		if(CompareObss > ObssThreshold) {
			err("There is large WiFi traffic not belongs to MyMAP Network");
			need_trigger = TRIGGER_TRUE;
		} else if ((CompareBkLoad > BkLoadThreshold) && (skip == 0)) {
			err("There is large background load not belongs to MyMAP Network");
			need_trigger = TRIGGER_TRUE;
		}
		if(need_trigger)
			break;
		_1905_device = topo_srv_get_next_1905_device(ctx, _1905_device);
	}
	return need_trigger;
}
u8 ch_planning_check_trigger(
	struct own_1905_device *ctx,
	struct monitor_ch_info *ch_info)
{
	u8 need_trigger = TRIGGER_FALSE;

	/*Check TRIGGER is to be made PENDING*/
	if((ctx->network_optimization.network_opt_state != NETOPT_STATE_IDLE) ||
			(ctx->ch_planning_R2.ch_plan_state >= CHPLAN_STATE_SCAN_ONGOING) ||
			(ctx->user_triggered_scan == 1)) {
		err("DEFER CH PLANNING");
		need_trigger = TRIGGER_PENDING;
		return need_trigger;
	}
	/*Check TRIGGER is to be TRUE or FALSE*/
	/*Part 1 -> Take AVG of all devices , monitored radio*/
	err(".....Take Monitored value AVG.....");
	ch_planning_all_dev_avg(ctx, ch_info);

	always("....Compare Monitor Avg with Threshold......\n");
	/*Part 2 ->Calculate and compare with thresholds*/
	need_trigger = ch_planning_is_trigger_req(ctx, ch_info);
	err(".....need_trigger chplanning Result %d.....",need_trigger);
	return need_trigger;
}
u32 get_scan_wait_time(
	struct _1905_map_device *_1905_device,
	struct radio_info_db *radio)
{
/*can't trigger scan if last scan for any agent was earlier than its min_scan_interval*/
	u32 scan_wait_time = 0;
	struct os_time now;
	os_get_time(&now);
	if(now.sec > (radio->dev_ch_plan_info.last_report_timestamp.sec
				+ radio->radio_scan_params.min_scan_interval)) {
		scan_wait_time = 0;
	} else {
		scan_wait_time = radio->radio_scan_params.min_scan_interval
				- (now.sec - radio->dev_ch_plan_info.last_report_timestamp.sec);
	}
	return scan_wait_time;
}
void ch_planning_scan_wait_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct _1905_map_device *dev = (struct _1905_map_device *)timeout_ctx;
	struct own_1905_device *ctx = (struct own_1905_device *)eloop_ctx;
	struct radio_info_db *temp_radio = NULL;
	struct monitor_ch_info *ch_info = NULL;

	SLIST_FOREACH(ch_info, &ctx->ch_planning_R2.first_monitor_ch, next_monitor_ch) {
		if(ch_info->channel_num == temp_radio->channel[0]) {
			temp_radio = topo_srv_get_radio_by_channel(dev, ch_info->channel_num);
			break;
		}
	}
	if(temp_radio)
		ch_planning_send_scan_req(ctx, dev, temp_radio);

}
void ch_scan_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct monitor_ch_info *ch_info = (struct monitor_ch_info *)timeout_ctx;
	struct own_1905_device *own_dev = (struct own_1905_device *)eloop_ctx;

	/*check if more than 50% agents have sent scan report.
	If yes, then trigger channel planning logic. find the agent count that support this radio
	number of agents that support that radio compare with total num of scan results
	*/
	if (ch_planning_check_scan_result(own_dev, ch_info)) {
		err("HALF scan reports received go for best channel selection");
		ch_planning_select_best_channel_R2(own_dev->back_ptr, ch_info);
		/*still some agents have not sent scan results , so clean complete scan list entry */
		ch_planning_remove_scan_list(own_dev);
		own_dev->ch_planning_R2.retry_count = 0;
	} else {
		if (own_dev->ch_planning_R2.retry_count == 1) {
			/* If we reach here it means we have not received more than 50% result
			after two attempts, need to proceed with whatever result we have */
			ch_planning_select_best_channel_R2(own_dev->back_ptr, ch_info);
			ch_planning_remove_scan_list(own_dev);
			own_dev->ch_planning_R2.retry_count = 0;
		} else {
			err(" re-trigger channel monitor on ch %d", ch_info->channel_num);
			/* re-trigger channel monitor.*/
			own_dev->ch_planning_R2.retry_count++;
			own_dev->ch_planning_R2.ch_plan_state = CHPLAN_STATE_MONITOR;
			ch_planning_update_all_dev_state((u8)CHPLAN_STATE_MONITOR,ch_info->channel_num,own_dev);
			/* if 5min scan timedout and we have not received 50% of scan result
			then retrigger scan immediately only one more time */
			eloop_register_timeout(0, 0, channel_monitor_timeout, own_dev, ch_info);
			/*change policy for AP metric reporting for all devices on that channel*/
			ch_planning_update_ap_metric_policy(own_dev, ch_info, 0);
			err("update policy done");
		}
	}
}

void ch_scan_timeout_mtk(void *eloop_ctx, void *timeout_ctx)
{
	struct monitor_ch_info *ch_info = (struct monitor_ch_info *)timeout_ctx;
	struct own_1905_device *own_dev = (struct own_1905_device *)eloop_ctx;

	/*check if more than 50% agents have sent scan report.
	If yes, then trigger channel planning logic. find the agent count that support this radio
	number of agents that support that radio compare with total num of scan results
	*/
	err(" ");
	if (ch_planning_check_scan_result(own_dev, ch_info)) {
		err("HALF scan reports received go for best channel selection");
		ch_planning_select_best_channel_R2(own_dev->back_ptr, ch_info);
		/*still some agents have not sent scan results , so clean complete scan list entry */
		ch_planning_remove_scan_list(own_dev);
		eloop_cancel_timeout(ch_scan_timeout, own_dev, ch_info);
		own_dev->ch_planning_R2.retry_count = 0;
	}

}

void ch_planning_remove_radio_scan_results(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{
	struct scan_result_tlv *scan_list_entry = NULL;
	struct nb_info *neigh = NULL;
	struct radio_info_db *radio = NULL;
	/*If user triggered scan is true remove all scan results ,
	if ch planning algo triggers then only delete for particular radio*/
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		if(ctx->user_triggered_scan == FALSE &&
			radio->dev_ch_plan_info.dev_ch_plan_state != CHPLAN_STATE_SCAN_ONGOING)
			continue;
		while (!SLIST_EMPTY(&(radio->first_scan_result))) {
			scan_list_entry = SLIST_FIRST(&(radio->first_scan_result));
			while (!SLIST_EMPTY(&(scan_list_entry->first_neighbor_info))) {
				neigh = SLIST_FIRST(&(scan_list_entry->first_neighbor_info));
				SLIST_REMOVE_HEAD(&(scan_list_entry->first_neighbor_info), next_neighbor_info);
				os_free(neigh);
			}
			SLIST_REMOVE_HEAD(&(radio->first_scan_result), next_scan_result);
			os_free(scan_list_entry);
		}
		if (SLIST_EMPTY(&(radio->first_scan_result))) {
			SLIST_INIT(&(radio->first_scan_result));
		}
	}
}

void ch_planning_remove_all_scan_results(
	struct _1905_map_device *dev)
{
	struct scan_result_tlv *scan_list_entry = NULL;
	struct nb_info *neigh = NULL;
	struct radio_info_db *radio = NULL;
	SLIST_FOREACH(radio, &dev->first_radio, next_radio) {
		while (!SLIST_EMPTY(&(radio->first_scan_result))) {
			scan_list_entry = SLIST_FIRST(&(radio->first_scan_result));
			while (!SLIST_EMPTY(&(scan_list_entry->first_neighbor_info))) {
				neigh = SLIST_FIRST(&(scan_list_entry->first_neighbor_info));
				SLIST_REMOVE_HEAD(&(scan_list_entry->first_neighbor_info), next_neighbor_info);
				os_free(neigh);
			}
			SLIST_REMOVE_HEAD(&(radio->first_scan_result), next_scan_result);
			os_free(scan_list_entry);
		}
	}
}
void ch_planning_remove_scanlist_entry(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
	struct scan_list_info *scan_list;
	SLIST_FOREACH(scan_list, &ctx->ch_planning_R2.first_scan_list, next_scan_list) {
		if(os_memcmp(scan_list->al_mac, dev->_1905_info.al_mac_addr, ETH_ALEN) == 0 &&
			os_memcmp(scan_list->radio_identifier, radio->identifier, ETH_ALEN) == 0) {
			err("dev ALMAC"MACSTR"",MAC2STR(dev->_1905_info.al_mac_addr));
			SLIST_REMOVE(&(ctx->ch_planning_R2.first_scan_list),
				scan_list,
				scan_list_info,
				next_scan_list);
			os_free(scan_list);
			if(SLIST_EMPTY(&(ctx->ch_planning_R2.first_scan_list)))
				break;
		}
	}
}
void ch_planning_insert_scanlist_entry(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
	struct scan_list_info *scan_list = os_zalloc(sizeof(struct scan_list_info));
	if (scan_list == NULL) {
		err("alloc memory fail");
		return;
	}
	os_memcpy(scan_list->al_mac, dev->_1905_info.al_mac_addr, ETH_ALEN);
	os_memcpy(scan_list->radio_identifier, radio->identifier, ETH_ALEN);
	scan_list->trigger_done = 0;
	SLIST_INSERT_HEAD(&(ctx->ch_planning_R2.first_scan_list),
		scan_list, next_scan_list);
}

void ch_planning_update_state_scan_done(
	struct own_1905_device *own_dev,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
	if(radio->dev_ch_plan_info.dev_ch_plan_state == CHPLAN_STATE_SCAN_ONGOING) {
		os_get_time(&radio->dev_ch_plan_info.last_report_timestamp);
		radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_SCAN_COMPLETE;
		ch_planning_remove_scanlist_entry(own_dev, dev, radio);
	}
}
void ch_planning_clear_score_table(
	struct own_1905_device *own_dev,
	struct monitor_ch_info *ch_info)
{
	struct score_info *cumulative_score = NULL;
	struct radio_info_db *radio_monitor = NULL, *radio = NULL;
	struct _1905_map_device *_1905_dev = NULL;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	radio_monitor = topo_srv_get_radio_by_band(_1905_dev,ch_info->channel_num);
	debug("monitor radio band %d", radio_monitor->band);
	SLIST_FOREACH(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score,next_ch_score) {
		radio = topo_srv_get_radio_by_band(_1905_dev,cumulative_score->channel);
		if(radio->band != radio_monitor->band){
			continue;
		}
		cumulative_score->total_score = 0;
		cumulative_score->dev_count = 0;
		cumulative_score->avg_score = 0;
		cumulative_score->ch_rank = 0;
	}
}
void ch_planning_remove_agg_score_table(
	struct own_1905_device *own_dev)
{
	struct score_info *cumulative_score = NULL;
	err(" ");
	while (!SLIST_EMPTY(&(own_dev->ch_planning_R2.first_ch_score))) {
		cumulative_score = SLIST_FIRST(&(own_dev->ch_planning_R2.first_ch_score));
		SLIST_REMOVE_HEAD(&(own_dev->ch_planning_R2.first_ch_score),
			next_ch_score);
		os_free(cumulative_score);
	}
}
void ch_planning_remove_selective_ch_monitor_info(
	struct own_1905_device *own_dev,
	struct radio_info_db *reset_radio)
{
	struct monitor_ch_info *ch_info = NULL;
	struct affected_agent_info *agent = NULL;
	SLIST_FOREACH(ch_info,
		&(own_dev->ch_planning_R2.first_monitor_ch), next_monitor_ch) {
		if(ch_info->channel_num != reset_radio->channel[0])
		{
			continue;
		}
		if(ch_info->trigger_status == TRIGGER_TRUE) {
			debug("ch scan timeout cancel for ch_info ch  %d", ch_info->channel_num);
			eloop_cancel_timeout(ch_scan_timeout, own_dev, ch_info);
			eloop_cancel_timeout(ch_scan_timeout_mtk, own_dev, ch_info);
			break;
		}
		eloop_cancel_timeout(channel_monitor_timeout, own_dev, ch_info);
		while (!SLIST_EMPTY(&(ch_info->first_affected_agent))) {
			agent = SLIST_FIRST(&(ch_info->first_affected_agent));
			SLIST_REMOVE_HEAD(&(ch_info->first_affected_agent),
				next_affected_agent);
			os_free(agent);
		}
		SLIST_REMOVE(&(own_dev->ch_planning_R2.first_monitor_ch),
				ch_info,
				monitor_ch_info,
				next_monitor_ch);
		os_free(ch_info);
		if(SLIST_EMPTY(&(own_dev->ch_planning_R2.first_monitor_ch)))
			break;
	}
}

void ch_planning_remove_ch_monitor_info_list(
	struct own_1905_device *own_dev,
	struct radio_info_db *reset_radio)
{
	struct monitor_ch_info *ch_info = NULL;
	struct affected_agent_info *agent = NULL;
	if(reset_radio != NULL) {
		ch_planning_remove_selective_ch_monitor_info(own_dev,reset_radio);
	}
	while (!SLIST_EMPTY(&(own_dev->ch_planning_R2.first_monitor_ch))) {
		ch_info = SLIST_FIRST(&(own_dev->ch_planning_R2.first_monitor_ch));
		if(ch_info->trigger_status == TRIGGER_TRUE) {
			debug("ch scan timeout cancel for ch_info ch  %d", ch_info->channel_num);
			eloop_cancel_timeout(ch_scan_timeout, own_dev, ch_info);
			eloop_cancel_timeout(ch_scan_timeout_mtk, own_dev, ch_info);
			break;
		}
		eloop_cancel_timeout(channel_monitor_timeout, own_dev, ch_info);
		while (!SLIST_EMPTY(&(ch_info->first_affected_agent))) {
			agent = SLIST_FIRST(&(ch_info->first_affected_agent));
			SLIST_REMOVE_HEAD(&(ch_info->first_affected_agent),
				next_affected_agent);
			os_free(agent);
		}
		SLIST_REMOVE_HEAD(&(own_dev->ch_planning_R2.first_monitor_ch),
			next_monitor_ch);
		os_free(ch_info);
	}
}
void ch_planning_remove_scan_list(
	struct own_1905_device *own_dev)
{
	struct scan_list_info *scan_list = NULL;
	while (!SLIST_EMPTY(&(own_dev->ch_planning_R2.first_scan_list))) {
		scan_list = SLIST_FIRST(&(own_dev->ch_planning_R2.first_scan_list));
		SLIST_REMOVE_HEAD(&(own_dev->ch_planning_R2.first_scan_list),
			next_scan_list);
		os_free(scan_list);
	}
}

void ch_planning_insert_ch_score_table(
	struct own_1905_device *own_dev, u8 channel)
{
	struct score_info *cumulative_score, *entry;
	/*insert only if not already inserted*/
	SLIST_FOREACH(entry, &(own_dev->ch_planning_R2.first_ch_score), next_ch_score) {
		if (entry->channel == channel) {
			debug("no need to insert ch already present");
			return;
		}
	}
	cumulative_score = os_zalloc(sizeof(struct score_info));
	if (cumulative_score == NULL) {
		err("alloc memory fail");
		assert(0);
		return ;
	}
	cumulative_score->channel = channel;
	cumulative_score->total_score = 0;
	cumulative_score->dev_count = 0;
	cumulative_score->ch_rank = 0;
	SLIST_INSERT_HEAD(&(own_dev->ch_planning_R2.first_ch_score),
		cumulative_score, next_ch_score);
}

void ch_planning_remove_ch_score_table(
	struct own_1905_device *own_dev, u8 channel)
{
	struct score_info *cumulative_score ;
	SLIST_FOREACH(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score) {
		if(cumulative_score->channel == channel) {
			SLIST_REMOVE(&(own_dev->ch_planning_R2.first_ch_score),
				cumulative_score,
				score_info,
				next_ch_score);
			os_free(cumulative_score);
			return;
		}
	}
}
struct score_info* ch_planning_find_ch_max_score(
	struct own_1905_device *own_dev)
{
	struct score_info *ch_score_temp = NULL, *max_ch_score = NULL;
	SLIST_FOREACH(ch_score_temp,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score) {
		if(!max_ch_score)
			max_ch_score = ch_score_temp;
		if(ch_score_temp->total_score > max_ch_score->total_score)
			max_ch_score = ch_score_temp;
	}
	if(max_ch_score)
		err("max score channel %d, score %d",max_ch_score->channel, max_ch_score->total_score);
	return max_ch_score;
}
void dump_ch_planning_score_info(
	struct own_1905_device *own_dev)
{
	struct score_info *ch_score = NULL;
	always("******* Total Channel Score Table************\n");
	SLIST_FOREACH(ch_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score) {
		always("channel %d, total score %d, avg score %d Rank %d\n", ch_score->channel,
			ch_score->total_score, ch_score->avg_score, ch_score->ch_rank);
	}
}

struct score_info* ch_planning_find_current_ch_score_info(
	struct own_1905_device *own_dev,
	struct radio_info_db *current_radio )
{
	struct score_info *ch_score = NULL, *current_working_ch_info = NULL;
	SLIST_FOREACH(ch_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score){
		if(ch_score->channel == current_radio->channel[0]) {
			current_working_ch_info = ch_score;
			return current_working_ch_info;
		}
	}
	return NULL;
}

u16 ch_planning_calc_weight( u8 band, u16 neighbor_num)
{
	u16 weight = 0;
	u16 weight_5G[MAX_BSS_NUM_5G] = {
		0, 18, 28, 36, 42, 47, 51, 54, 57, 60,
		62, 65, 67, 69, 71, 72, 74, 75, 77, 78,
		79, 81, 82, 83, 84, 85, 86, 87, 88, 89,
		90, 91, 91, 92, 93, 94, 94, 95, 96, 96,
		97, 98, 98, 99, 100};
	u16 weight_2G[MAX_BSS_NUM_2G] = {0, 33, 52, 66, 77, 86, 93, 100};

	if (band == BAND_2G){
		if (neighbor_num >= MAX_BSS_NUM_2G) {
			weight = weight_2G[MAX_BSS_NUM_2G - 1];
		}else {
			weight = weight_2G[neighbor_num - 1];
		}
	} else{
		if (neighbor_num >= MAX_BSS_NUM_5G) {
			weight = weight_5G[MAX_BSS_NUM_5G - 1];
		}else {
			weight = weight_5G[neighbor_num - 1];
		}
	}
	return weight;
}
u16 ch_planning_calc_OBSS(struct own_1905_device *own_dev,
	struct _1905_map_device *_1905_dev,
	struct radio_info_db *radio,
	struct scan_result_tlv *scan_res)
{
/*need to remove the my MAP network contribution from OBSS time*/
/*Calculate ObssAirtime based on the following formula
	ObssTime-A1 = ObssTimeA1 - (Tx/Rx Time C +Tx/Rx Time A2 etc.)*/
	//struct _1905_map_device *other_map_dev = NULL;
	//struct radio_info_db *other_mapdev_radio = NULL;
	u8 OBSS = 0;
	//u32 OtherDevTxRxAirTime = 0;

	if (radio->channel[0] != scan_res->channel) {
		debug("My MAP Network is not on this channel , so can use obss time directly");
		OBSS = scan_res->utilization;
		return OBSS;
	}
	return scan_res->utilization;//return utilization for all channels
#if 0
	OBSS = radio->radio_metrics.cu_other;
	SLIST_FOREACH(other_map_dev, &own_dev->_1905_dev_head, next_1905_device) {
	/* Don't subtract your own tx rx air time as it is already not present in obss airtime*/
		if(other_map_dev == _1905_dev)
			continue;
		other_mapdev_radio =
			topo_srv_get_radio_by_band(other_map_dev,radio->channel[0]);
		if(!other_mapdev_radio)
			return 0;
		err("cu_tx %d,cu_rx %d",other_mapdev_radio->radio_metrics.cu_tx,
			other_mapdev_radio->radio_metrics.cu_rx);
		
		OtherDevTxRxAirTime = OtherDevTxRxAirTime +
			other_mapdev_radio->radio_metrics.cu_tx +
			other_mapdev_radio->radio_metrics.cu_rx;
	}
	if(OBSS > OtherDevTxRxAirTime) {
		OBSS = OBSS - OtherDevTxRxAirTime;
		err("OBSS %d", OBSS);
		return OBSS;
	}else {
		err("ASSUMPTION that ObssAirtime_A2 > (MyTxAirtime_A1 + MyRxAirtime_A1) is wrong");
		return scan_res->utilization;
	}
#endif
}

void channel_cac_timeout2(void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *own_dev = (struct own_1905_device *)eloop_ctx;
	struct _1905_map_device *_1905_dev, *new_1905_dev;
	struct radio_info_db *radio;
	u8 channel = own_dev->ch_planning_R2.CAC_on_channel;
	unsigned int cac_method = 0;
	err("CAC_on_channel %d ", own_dev->ch_planning_R2.CAC_on_channel);

/*Find the device on which CAC timed out and mark its state as CAC_TIMEOUT*/
	SLIST_FOREACH(_1905_dev, &own_dev->_1905_dev_head, next_1905_device) {
		SLIST_FOREACH(radio, &_1905_dev->first_radio, next_radio) {
			if(radio->dev_ch_plan_info.dev_ch_plan_state == CHPLAN_STATE_CAC_ONGOING)
				radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CAC_TIMEOUT;
		}
	}

/*Find a new agent to trigger CAC on*/
	new_1905_dev = ch_planning_find_agent_for_CAC(own_dev->back_ptr, channel, &cac_method);

/*Trigger CAC on new agent*/
	if(new_1905_dev) {
		err(" trigger CAC again on new agent "MACSTR"",MAC2STR(new_1905_dev->_1905_info.al_mac_addr));
		ch_planning_trigger_cac_msg2(own_dev->back_ptr, new_1905_dev, channel, cac_method);
	} else {
		err("channel switch to ch %d trigger immediately as no agent found", channel);
		own_dev->ch_planning_R2.ch_plan_state =
				CHPLAN_STATE_CH_CHANGE_TRIGGERED;
		ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED,channel,own_dev);
		own_dev->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
	}
	start_netopt_timer(own_dev, channel);
}
#if 0
void channel_cac_timeout(void *eloop_ctx, void *timeout_ctx)
{
	u8 channel = *(u8 *)timeout_ctx;
	struct own_1905_device *own_dev = (struct own_1905_device *)eloop_ctx;
	struct _1905_map_device *_1905_dev, *new_1905_dev;
	struct radio_info_db *radio;
	unsigned int cac_method = 0;
	err(" ");
/*Find the device on which CAC timed out and mark its state as CAC_TIMEOUT*/
	SLIST_FOREACH(_1905_dev, &own_dev->_1905_dev_head, next_1905_device) {
		SLIST_FOREACH(radio, &_1905_dev->first_radio, next_radio) {
			if(radio->dev_ch_plan_info.dev_ch_plan_state == CHPLAN_STATE_CAC_ONGOING)
				radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CAC_TIMEOUT;
		}
	}

/*Find a new agent to trigger CAC on*/
	new_1905_dev = ch_planning_find_agent_for_CAC(own_dev->back_ptr, channel, &cac_method);

/*Trigger CAC on new agent*/
	if(new_1905_dev) {
		err(" trigger CAC again on new agent "MACSTR"",MAC2STR(new_1905_dev->_1905_info.al_mac_addr));
		ch_planning_trigger_cac_msg(own_dev->back_ptr, new_1905_dev, channel, cac_method);
	} else {
		err("channel switch to ch %d trigger immediately as no agent found", channel);
		ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED,channel,own_dev);
		own_dev->ch_planning_R2.ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
		ch_planning_set_user_preff_ch(own_dev->back_ptr,channel);
	}
}
#endif
void ch_planning_blacklist_ch(
	struct own_1905_device *own_dev, u8 channel, u8 op)
{
	struct score_info *cumulative_score ;
#ifdef MAP_160BW
	unsigned char channel_block[8] = {0};
#else
	unsigned char channel_block[4] = {0};
#endif
	int maxbw = BW_20, bw = BW_20;
	struct _1905_map_device *_1905_dev = NULL;
	SLIST_FOREACH(_1905_dev, &own_dev->_1905_dev_head, next_1905_device) {
		bw = ch_planning_get_dev_bw_from_channel(_1905_dev, channel);
		if (maxbw < bw)
			maxbw = bw;
	}
	ch_planning_get_channel_block(channel, channel_block, op, maxbw);
	//err("ch blk to blacklist is %d, %d, %d, %d", channel_block[0], channel_block[1], channel_block[2], channel_block[3]);
	SLIST_FOREACH(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score) {
#ifdef MAP_160BW
		if (op == 129) {
			if(cumulative_score->channel == channel_block[0] ||
				cumulative_score->channel == channel_block[1] ||
				cumulative_score->channel == channel_block[2] ||
				cumulative_score->channel == channel_block[3] ||
				cumulative_score->channel == channel_block[4] ||
				cumulative_score->channel == channel_block[5] ||
				cumulative_score->channel == channel_block[6] ||
				cumulative_score->channel == channel_block[7]) {
				cumulative_score->avg_score = -65535;
				cumulative_score->ch_rank = 0;
				err("blacklist channel %d due to radar",cumulative_score->channel);
				//return;
			}
		} else
#endif
		{
			if(cumulative_score->channel == channel_block[0] ||
				cumulative_score->channel == channel_block[1] ||
				cumulative_score->channel == channel_block[2] ||
				cumulative_score->channel == channel_block[3]) {
				cumulative_score->avg_score = -65535;
				cumulative_score->ch_rank = 0;
				err("blacklist channel %d due to radar",cumulative_score->channel);
				//return;
			}
		}
	}
}

void ch_planning_radar_detected2(
	struct own_1905_device *own_dev,
	struct radio_info_db *radio)
{
	err("remove channel from list and get next best channel ");
	ch_planning_blacklist_ch(own_dev,
		radio->cac_comp_status.channel, radio->operating_class);
	/*need to go back to ch planning R1 logic to find new best channel*/
	own_dev->ch_planning_R2.ch_plan_state =
		CHPLAN_STATE_CH_CHANGE_TRIGGERED;
	radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
	own_dev->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
	if (own_dev->div_ch_planning == 1)
		own_dev->ch_planning_R2.CAC_on_channel = 0;

}
#if 0
void ch_planning_radar_detected(
	struct own_1905_device *own_dev,
	struct radio_info_db *radio)
{
	struct score_info *best_channel_info, *current_working_ch_info;

	err("remove channel from list and get next best channel ");
	ch_planning_remove_ch_score_table(own_dev,
		radio->cac_comp_status.channel);

	best_channel_info = ch_planning_find_ch_max_score(own_dev);

	if(!best_channel_info) {
		err("NO best channel found");
		return;
	}
	if (best_channel_info->channel != radio->channel[0]) {

		err("current channel is different from best channel ");
		current_working_ch_info =
			ch_planning_find_current_ch_score_info(own_dev, radio);

		if(!current_working_ch_info) {
			err("NO current working channel info found");
			return;
		}

		//check score margin
		if (best_channel_info->total_score >
			(current_working_ch_info->total_score +
				own_dev->ch_planning_R2.min_score_inc)) {
			err("need to trigger ch switch");
			ch_planning_ch_change_trigger(own_dev->back_ptr,
				best_channel_info->channel);
		}
	}

}
#endif
void controller_dfs_ch_set(void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *ctx = (struct own_1905_device *)eloop_ctx;
	struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(ctx, NULL);
	struct _1905_map_device *tmp_dev = NULL;
	err(".......controller dfs ch select timeout......");
	//ctx->ch_planning_R2.ch_prefer_for_ch_select;
err("ctx->ch_planning_R2.ch_prefer_count %d ", ctx->ch_planning_R2.ch_prefer_count);
	err(" chnum %d",ctx->ch_planning_R2.ch_prefer_for_ch_select->opinfo[0].ch_list[0]);
	ch_planning_send_select(ctx,
		own_1905_node,
		ctx->ch_planning_R2.ch_prefer_count,
		ctx->ch_planning_R2.ch_prefer_for_ch_select);
	SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
		err("clearing ch sel request");
		tmp_dev->ch_sel_req_given = 0;
	}
	os_free(ctx->ch_planning_R2.ch_prefer_for_ch_select);
}

void ch_planning_R2_send_select_dfs(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char ch_prefer_count,
	struct ch_prefer_lib *ch_prefer,
	u8 channel)
{
	struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(ctx, NULL);
	struct _1905_map_device *tmp_dev = NULL, *tmp_dev_2 = NULL;
	int max_hop = -1;
	debug("R2.ch_plan_enable %d, state %d, ctx->dedicated_radio %d",ctx->ch_planning_R2.ch_plan_enable,  ctx->ch_planning_R2.ch_plan_state, ctx->dedicated_radio);
	/*since our dev needs to jump to a dfs channel , so all the devices that are connected below 
	it in the tree (or have max hop count more that the dev), need to first be switched to the dfs channel ,
	otherwise they may mistakenly perform CAC due to Channel switch announcement of their uplink*/
	if((ctx->ch_planning_R2.ch_plan_enable == TRUE || ctx->dedicated_radio)&&
		ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_CH_CHANGE_TRIGGERED) {
		SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
			if (tmp_dev == own_1905_node)
				continue;
			err("MAC:"MACSTR, MAC2STR(tmp_dev->_1905_info.al_mac_addr));
			max_hop = -1;
			SLIST_FOREACH(tmp_dev_2, &ctx->_1905_dev_head, next_1905_device) {
				if (tmp_dev_2->ch_sel_req_given == 1)
					continue;
				if (max_hop <= tmp_dev_2->root_distance)
					max_hop = tmp_dev_2->root_distance;
			}
			SLIST_FOREACH(tmp_dev_2, &ctx->_1905_dev_head, next_1905_device) {
				if (tmp_dev_2 == own_1905_node) {
					err("skip for controller");
					continue;
				}
				if (max_hop == tmp_dev_2->root_distance && tmp_dev_2->ch_sel_req_given == 0) {
					err("sending ch sel request to:"MACSTR, MAC2STR(tmp_dev_2->_1905_info.al_mac_addr));
					struct radio_info_db *temp_radio = topo_srv_get_radio_by_band(tmp_dev_2,channel);
					if(temp_radio)
						temp_radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
					ch_planning_send_select(ctx, tmp_dev_2, ch_prefer_count, ch_prefer);
					tmp_dev_2->ch_sel_req_given = 1;
				}
			}
		}
		ctx->ch_planning_R2.ch_prefer_count = ch_prefer_count;
		ctx->ch_planning_R2.ch_prefer_for_ch_select = os_zalloc(sizeof(struct ch_prefer_lib) * 3);
		if(!ctx->ch_planning_R2.ch_prefer_for_ch_select) {
			err("alloc fail");
			return;
		}
		struct ch_prefer_lib *con_prefer = ctx->ch_planning_R2.ch_prefer_for_ch_select;
		u8 i = 0;
		err(" count %d ", ctx->ch_planning_R2.ch_prefer_count);
		while (i < ch_prefer_count) {
			u8 max_pref_index = ch_planning_find_max_pref_index(ctx, ch_prefer);
			err("Reason code: %d opclass %d channel %d", ch_prefer[i].opinfo[max_pref_index].reason, 
				ch_prefer[i].opinfo[max_pref_index].op_class, 
				ch_prefer[i].opinfo[max_pref_index].ch_list[0]);
			os_memcpy(con_prefer->identifier,ch_prefer->identifier,ETH_ALEN);
			con_prefer->op_class_num = ch_prefer->op_class_num;
			con_prefer->opinfo[i].op_class = ch_prefer->opinfo[max_pref_index].op_class;
			con_prefer->opinfo[i].ch_num = ch_prefer->opinfo[max_pref_index].ch_num;
			con_prefer->opinfo[i].ch_list[0] = ch_prefer->opinfo[max_pref_index].ch_list[0];
			con_prefer->opinfo[i].perference = ch_prefer->opinfo[max_pref_index].perference;
			con_prefer->opinfo[i].reason = ch_prefer->opinfo[max_pref_index].reason;
			err(" chlist %d ", ctx->ch_planning_R2.ch_prefer_for_ch_select->opinfo[i].ch_list[0]);

			/* Using i+1 index for storing primary CH info at con_prefer->opinfo */
			if ((ch_prefer->opinfo[max_pref_index].op_class == 128)
#ifdef MAP_160BW
				|| (ch_prefer->opinfo[max_pref_index].op_class == 129)
#endif
			) {
				unsigned char next_channel;

				next_channel = ch_prefer->opinfo[max_pref_index+1].ch_list[0];
				if (is_valid_primary_ch_80M_160M(next_channel, ch_prefer->opinfo[max_pref_index].ch_list[0],
										ch_prefer->opinfo[max_pref_index].op_class)) {
					con_prefer->opinfo[i+1].ch_num = 1;
					con_prefer->opinfo[i+1].ch_list[0] = ch_prefer->opinfo[max_pref_index+1].ch_list[0];
					con_prefer->opinfo[i+1].op_class = ch_prefer->opinfo[max_pref_index+1].op_class;
					//os_memcpy(con_prefer->opinfo[i+1].identifier, ch_prefer->identifier, ETH_ALEN);
					con_prefer->opinfo[i+1].reason = ch_prefer->opinfo[max_pref_index+1].reason;
					err("DFS opclass %d channel %d",
						ch_prefer->opinfo[max_pref_index+1].op_class,
						ch_prefer->opinfo[max_pref_index+1].ch_list[0]);
					/* Since we just added one extra entry, increment the index by 1 to make sure it will not get overwritten in next iteration */
					i++;
				}
			}
			i++;
		}
		err(" ");
		eloop_register_timeout(3, 0,
			controller_dfs_ch_set, ctx, NULL);

	}
}

void ch_planning_handle_cac_response2(
	struct own_1905_device *own_dev,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
	struct _1905_map_device *_1905_device;
	unsigned int cac_method = 0;
	err(" ");
	if (own_dev->user_triggered_cac == 0) {
		if((own_dev->ch_planning_R2.ch_plan_state != CHPLAN_STATE_CAC_ONGOING) ||
			(radio->dev_ch_plan_info.dev_ch_plan_state != CHPLAN_STATE_CAC_ONGOING) ||
			(own_dev->ch_planning.ch_planning_state != CHANNEL_PLANNING_CAC_START))
		{
			err(" ");
			return;
		}
	}
	if (radio->cac_comp_status.channel != own_dev->ch_planning_R2.CAC_on_channel) {
		err("Return, CAC req on %u, CAC success for %u\n", own_dev->ch_planning_R2.CAC_on_channel,
			radio->cac_comp_status.channel);
		return;
	}
	err("cancel cac timeouton ch %d",own_dev->ch_planning_R2.CAC_on_channel);
	eloop_cancel_timeout(channel_cac_timeout2,
		own_dev, NULL);

	if(radio->cac_comp_status.cac_status == RADAR_DETECTED) {
		err("Radar found on best channel");
		if (own_dev->user_triggered_cac == 0)
			ch_planning_radar_detected2(own_dev,radio);
	} else if (radio->cac_comp_status.cac_status == CAC_SUCCESSFUL) {
		err("set this channel %d",radio->cac_comp_status.channel);
		if (own_dev->user_triggered_cac == 0) {
			own_dev->ch_planning_R2.ch_plan_state =
				CHPLAN_STATE_CH_CHANGE_TRIGGERED;
			own_dev->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
			ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED,radio->channel[0],own_dev);
			err(" own_dev->force_ch_change %d", own_dev->force_ch_change);
			if(own_dev->force_ch_change || own_dev->div_ch_planning) {
				err("R1 ch planning is %d, make it 1 here", own_dev->ch_planning.ch_planning_enabled);
				own_dev->ch_planning.ch_planning_enabled = 1;
				mapd_restart_channel_plannig(own_dev->back_ptr);
			}
		}
	} else {
		err("CAC failed trigger CAC on different agent.");
		/*current agent is not good ,mark it */
		if (own_dev->user_triggered_cac == 0) {
			radio->dev_ch_plan_info.dev_ch_plan_state =
				CHPLAN_STATE_CAC_TIMEOUT;

			_1905_device = ch_planning_find_agent_for_CAC(own_dev->back_ptr,
				radio->cac_comp_status.channel, &cac_method);

			if (_1905_device) {
				ch_planning_trigger_cac_msg2(
				own_dev->back_ptr,
				_1905_device,
				radio->cac_comp_status.channel, cac_method);
			} else {
				err("channel switch to ch %d trigger immediately as no agent found", 
							radio->cac_comp_status.channel);
							own_dev->ch_planning_R2.ch_plan_state =
						CHPLAN_STATE_CH_CHANGE_TRIGGERED;
				ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED,
							radio->cac_comp_status.channel,
							own_dev);
				own_dev->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
			}

			start_netopt_timer(own_dev, radio->cac_comp_status.channel);
		}
	}
}
#if 0
void ch_planning_handle_cac_response(
	struct own_1905_device *own_dev,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
	struct _1905_map_device *_1905_device;
	unsigned int cac_method = 0;
	err(" ");
	if (own_dev->user_triggered_cac == 0)
		if((own_dev->ch_planning_R2.ch_plan_state != CHPLAN_STATE_CAC_ONGOING) ||
			(radio->dev_ch_plan_info.dev_ch_plan_state != CHPLAN_STATE_CAC_ONGOING))
		{
			return;
		}
	eloop_cancel_timeout(channel_cac_timeout,
		own_dev, &radio->cac_comp_status.channel);

	if(radio->cac_comp_status.cac_status == RADAR_DETECTED) {
		err("Radar found on best channel");
		if (own_dev->user_triggered_cac == 0)
			ch_planning_radar_detected(own_dev,radio);
	} else if (radio->cac_comp_status.cac_status == CAC_SUCCESSFUL) {
		err("set this channel %d",radio->cac_comp_status.channel);
		if (own_dev->user_triggered_cac == 0) {
			own_dev->ch_planning_R2.ch_plan_state =
				CHPLAN_STATE_CH_CHANGE_TRIGGERED;
			ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED,
				radio->cac_comp_status.channel,own_dev);
			ch_planning_set_user_preff_ch(own_dev->back_ptr,
				radio->cac_comp_status.channel);
		}
	} else {
		err("CAC failed trigger CAC on different agent.");
		/*current agent is not good ,mark it */
		if (own_dev->user_triggered_cac == 0) {
			radio->dev_ch_plan_info.dev_ch_plan_state =
				CHPLAN_STATE_CAC_TIMEOUT;

			_1905_device = ch_planning_find_agent_for_CAC(own_dev->back_ptr,
				radio->cac_comp_status.channel, &cac_method);

			if (_1905_device)
				ch_planning_trigger_cac_msg(
				own_dev->back_ptr,
				_1905_device,
				radio->cac_comp_status.channel, cac_method);

		}
	}
}
#endif
u8 is_channel_in_cac_cap(
	u8 channel,
	struct cac_cap_db *cap)
{
	struct cac_opcap_db *opcap = NULL;
	u8 i = 0;
	int bw = BW_20;
	SLIST_FOREACH(opcap, &cap->cac_opcap_head, cac_opcap_entry) {
		bw = chan_mon_get_bw_from_op_class(opcap->op_class);
		for(i = 0; i < opcap->ch_num; i++) {
			if (bw <= BW_40) {
				if(opcap->ch_list[i] == channel)
					return 1;
			} else {
				if (opcap->ch_list[i] ==
						ch_planning_get_centre_freq_ch(channel, opcap->op_class)) {
					return 1;
				}
			}
		}
	}
	err("channel not found in cac cap");
	return 0;
}

void ch_planning_trigger_cac_msg2 (
	struct mapd_global *global,
	struct _1905_map_device *_1905_dev,
	u8 channel, unsigned int cac_method)
{
	struct own_1905_device *ctx = &global->dev;
	struct radio_info_db *radio = NULL;
	struct cac_request *req = NULL;
	int total_cac_time = 0;
	struct cac_cap_db *cap = NULL;
	//int band;
	err(" ");
	/*CAC is to be done on the new best channel*/
	req = os_zalloc(sizeof(struct cac_request) + sizeof(struct cac_tlv));
	if(!req) {
		err("malloc fail");
		return;
	}
	req->num_radio = 1;
	radio = topo_srv_get_radio_by_band(_1905_dev, channel);
#if 0 
	band = get_band(_1905_dev, channel);
	err("channel %d, band %d",channel, band);
	SLIST_FOREACH(radio, &_1905_dev->first_radio, next_radio) {
		err("radio->band %d", radio->band);
		if(radio->band == band)
			break;
	}
#endif
	if (radio == NULL) {
		err("radio is NULL");
		os_free(req);
		return;
	}
	err("radio->band %d", radio->band);

	os_memcpy(req->tlv[0].identifier, radio->identifier, ETH_ALEN);
	req->tlv[0].cac_action = 0x01;
	req->tlv[0].cac_method = cac_method;
	req->tlv[0].ch_num = channel;
	req->tlv[0].op_class_num = radio->operating_class;
	err("send cac req to agent with ALMAC "MACSTR" ",MAC2STR(_1905_dev->_1905_info.al_mac_addr));
	err("cac action: %d, cac method: %d, ch num: %d, op class: %d", req->tlv[0].cac_action, req->tlv[0].cac_method, req->tlv[0].ch_num, req->tlv[0].op_class_num);
	if (_1905_dev->map_version != DEV_TYPE_R2) {
		err("agent is not R2");
		os_free(req);
		return;
	}
	map_1905_Send_CAC_Request(
		global->_1905_ctrl,
		(char *)_1905_dev->_1905_info.al_mac_addr,
		req);
	radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CAC_ONGOING;
	global->dev.ch_planning_R2.ch_plan_state = CHPLAN_STATE_CAC_ONGOING;
	ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_CAC_START;
	os_free(req);
	global->dev.ch_planning_R2.CAC_on_channel = channel;
	err("start channel cac timeout2 on channel %d", channel);
	SLIST_FOREACH(cap, &radio->cac_cap.cac_capab_head, cac_cap_entry) {
		if (is_channel_in_cac_cap(channel, cap) == 1) {
			total_cac_time = cap->cac_interval[0]*power(16,4) + cap->cac_interval[1]*power(16,2) + cap->cac_interval[2];
			break;
		}
	}
	err("Total cac time: %d", total_cac_time);
	eloop_register_timeout(total_cac_time + 20,
			0,
			channel_cac_timeout2,
			ctx,
			NULL);
	return;
}

#if 0
void ch_planning_trigger_cac_msg (
	struct mapd_global *global,
	struct _1905_map_device *_1905_dev,
	u8 channel, unsigned int cac_method)
{
	struct own_1905_device *ctx = &global->dev;
	struct radio_info_db *radio = NULL;
	struct cac_request *req = NULL;
	int band;
	err(" ");
	/*CAC is to be done on the new best channel*/
	req = os_zalloc(sizeof(struct cac_request) + sizeof(struct cac_tlv));
	if(!req) {
		err("malloc fail");
		return;
	}
	req->num_radio = 1;
	band = get_band(_1905_dev, channel);
	SLIST_FOREACH(radio, &_1905_dev->first_radio, next_radio) {
		if(radio->band == band)
			break;
	}
	if (radio == NULL) {
		err("radio is NULL");
		os_free(req);
		return;
	}

	os_memcpy(req->tlv[0].identifier, radio->identifier, ETH_ALEN);
	req->tlv[0].cac_action = 0x01;
	req->tlv[0].cac_method = cac_method;
	req->tlv[0].ch_num = channel;
	req->tlv[0].op_class_num = radio->operating_class;
	err("send cac req to agent with ALMAC "MACSTR" ",MAC2STR(_1905_dev->_1905_info.al_mac_addr));
	err("cac action: %d, cac method: %d, ch num: %d, op class: %d", req->tlv[0].cac_action, req->tlv[0].cac_method, req->tlv[0].ch_num, req->tlv[0].op_class_num);
	if (_1905_dev->map_version == DEV_TYPE_R2)
		map_1905_Send_CAC_Request(
			global->_1905_ctrl,
			(char *)_1905_dev->_1905_info.al_mac_addr,
			req);
	radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CAC_ONGOING;
	global->dev.ch_planning_R2.ch_plan_state = CHPLAN_STATE_CAC_ONGOING;
	os_free(req);
	global->dev.ch_planning_R2.CAC_on_channel = channel;
	eloop_register_timeout(CHANNEL_CAC_TIMEOUT,
		0,
		channel_cac_timeout,
		ctx,
		(void *)&channel);
	return;
}
#endif

struct _1905_map_device *ch_planning_find_agent_cac_mode(
	struct own_1905_device *own_dev,
	u8 channel,
	u8 cac_mode)
{
	struct radio_info_db *radio = NULL;
	struct cac_cap_db *cap = NULL;
	struct _1905_map_device *_1905_dev = NULL;
	struct _1905_map_device *temp_map_dev = NULL;
	int max_hop = -1;

	SLIST_FOREACH(_1905_dev, &own_dev->_1905_dev_head, next_1905_device) {
		if(_1905_dev->device_role != DEVICE_ROLE_AGENT)
			continue;
		if(_1905_dev->map_version != DEV_TYPE_R2) {
			err("this is agent is not R2 cap , so don't choose this agent");
			continue;
		}
		if(!_1905_dev->in_network) {
			err("agent not in network ");
			continue;
		}
		radio = topo_srv_get_radio_by_band(_1905_dev,channel);
		if (radio->dev_ch_plan_info.dev_ch_plan_state ==
			CHPLAN_STATE_CAC_TIMEOUT) {
			continue;
		}
		SLIST_FOREACH(cap, &radio->cac_cap.cac_capab_head, cac_cap_entry) {
			if (is_channel_in_cac_cap(channel, cap) == 0)
				continue;

			if(cap->cac_mode == cac_mode) {
				if (max_hop <= _1905_dev->root_distance) {
					max_hop = _1905_dev->root_distance;
					temp_map_dev = _1905_dev;
				}
			}
		}
	}
	return temp_map_dev;
}
struct _1905_map_device * ch_planning_find_agent_for_CAC(
	struct mapd_global *global,
	u8 channel, unsigned int *cac_method)
{
/*select best Agent/Radio for executing DFS
Dedicated Radio > Reduced MIMO > continuous */
	struct own_1905_device *own_dev = &global->dev;
	struct _1905_map_device *agent = NULL;
	*cac_method = DEDICATED_RADIO;
	agent = ch_planning_find_agent_cac_mode(own_dev,channel, DEDICATED_RADIO);
	if (!agent) {
		*cac_method = REDUCED_MIMO;
		agent = ch_planning_find_agent_cac_mode(own_dev, channel, REDUCED_MIMO);
	}
	if (!agent) {
		*cac_method = CONTINUOUS;
		agent = ch_planning_find_agent_cac_mode(own_dev, channel, CONTINUOUS);
	}

	return agent;
	
}

u8 is_CAC_Success(
	struct mapd_global *global,
	u8 channel)
{
	struct own_1905_device *own_dev = &global->dev;
	struct _1905_map_device *_1905_dev = NULL;
	struct radio_info_db *radio = NULL;

	SLIST_FOREACH(_1905_dev, &own_dev->_1905_dev_head, next_1905_device) {
		radio = topo_srv_get_radio_by_band(_1905_dev,channel);
		if (radio){
			if((radio->cac_comp_status.channel == channel) &&
				(radio->cac_comp_status.cac_status == CAC_SUCCESSFUL)) {
				err("CAC has already been done and success on ch %d",channel);
				return 1;
			}
		}
	}
err("CAC not done , so do");
return 0;
}
u8 ch_planning_check_controller_cac_cap(
	struct own_1905_device *own_dev,
	u8 channel,
	u8 cac_mode)
{
	struct radio_info_db *radio = NULL;
	struct cac_cap_db *cap = NULL;
	struct _1905_map_device *_1905_dev = NULL;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);

	radio = topo_srv_get_radio_by_band(_1905_dev,channel);
	if(!radio) {
		err("radio not found");
		return 0;
	}
	err("radio channel %d, identifier "MACSTR", cac_type_num:%d", radio->channel[0], MAC2STR(radio->cac_cap.identifier), radio->cac_cap.cac_type_num);
	//mapd_hexdump(MSG_OFF,"in ch_planning_check_controller_cac_cap CAC CAP FROM WAPP",&radio->cac_cap, 50); 
	SLIST_FOREACH(cap, &radio->cac_cap.cac_capab_head, cac_cap_entry) {
		err("cap->cac_mode %d", cap->cac_mode);
		if(cap->cac_mode == cac_mode) {
			err("controller supports dedicated radio CAC");
			return 1;
		}
	}
	return 0;
}
void ch_planning_handle_cac_failure(struct mapd_global * global, struct radio_info_db *radio,
						struct cac_completion_report * report, struct cac_status_report * status_report)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_map_device *dev = topo_srv_get_controller_device(ctx);

	if (dev == NULL)
		return;

	if (ctx->device_role == DEVICE_ROLE_CONTROLLER) {
		err("CAC_FAILURE\n");
		eloop_cancel_timeout(channel_cac_timeout2, ctx, NULL);
		radio->cac_comp_status.cac_status = CAC_FAILURE;
		ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
		ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_IDLE;
		dev->channel_planning_completed = TRUE;
		ch_planning_R2_reset(ctx, NULL);
	}
	if (ctx->device_role != DEVICE_ROLE_CONTROLLER) {
		err("agent send cac failure info to controller");
		if (ctx->map_version == DEV_TYPE_R2)
			_1905_update_channel_pref_report(ctx, report, status_report);
		else
			_1905_update_channel_pref_report(ctx, report, NULL);

	};
}
void ch_planning_handle_cac_success_for_cont(struct mapd_global * global, struct radio_info_db *radio,
						struct cac_completion_report * report)
{
	struct own_1905_device *ctx = &global->dev;

	err("radio ch state: %d, r2_chan_state: %d, R1_chan_state: %d\n",
		radio->dev_ch_plan_info.dev_ch_plan_state, ctx->ch_planning_R2.ch_plan_state, ctx->ch_planning.ch_planning_state);
	if ((global->dev.ch_planning_R2.ch_plan_state == CHPLAN_STATE_CAC_ONGOING) &&
		((ctx->ch_planning.ch_planning_state == CHANNEL_PLANNING_CAC_START) || ctx->div_ch_planning == 1)) {

		if (report->cac_completion_status[0].channel != ctx->ch_planning_R2.CAC_on_channel) {
			err("Return, CAC req on %u, CAC success for %u\n", ctx->ch_planning_R2.CAC_on_channel,
				report->cac_completion_status[0].channel);
			return;
		}
		err("cancel cac timeouton ch %d",ctx->ch_planning_R2.CAC_on_channel);
		eloop_cancel_timeout(channel_cac_timeout2,ctx, NULL);

		ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
		ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
		ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED,radio->channel[0],&global->dev);

		err("R1 ch planning is %d, make it 1 here", global->dev.ch_planning.ch_planning_enabled);
		global->dev.ch_planning.ch_planning_enabled = 1;
		mapd_restart_channel_plannig(global);

		radio->cac_enable = CAC_SUCCESSFUL;
		radio->cac_channel = 0;
		radio->cac_comp_status.cac_status = CAC_SUCCESSFUL;
		radio->cac_comp_status.channel = report->cac_completion_status[0].channel;//ctx->ch_planning_R2.CAC_on_channel;
	}
}

void ch_planning_trigger_cac_on_cont(struct mapd_global *global, u8 channel)
{
	struct cac_request *cac = NULL;
	struct _1905_map_device *_1905_dev = NULL;
	struct own_1905_device *ctx = &global->dev;
	struct radio_info_db *radio = NULL;
	u8 *buff = NULL;
	u8 i = 0;
	u16 length = 0;
	unsigned int total_cac_time = 0;
	struct cac_cap_db *cap = NULL;

	err("Triggering CAC on controller\n");

	buff = os_zalloc(sizeof(struct cac_request) + sizeof(struct cac_tlv));
	if(!buff) {
		err("malloc fail");
		return;
	}

	_1905_dev = topo_srv_get_1905_device(ctx, NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev, channel);

	if (radio == NULL) {
		err("radio is NULL");
		os_free(buff);
		return;
	}
	cac = (struct cac_request *)buff;
	cac->num_radio = 1;

	for(i=0; i < cac->num_radio; i++) {
		os_memcpy(&cac->tlv[i].identifier,radio->identifier,ETH_ALEN);

		cac->tlv[i].op_class_num = radio->operating_class;
		cac->tlv[i].ch_num = channel;
		err("channel %d",cac->tlv[i].ch_num);
		cac->tlv[i].cac_method = 0x01;
		err("cac method: %d", cac->tlv[i].cac_method);
		cac->tlv[i].cac_action = 0x01;
		err("cac action: %d", cac->tlv[i].cac_action);
	}

	length = sizeof(struct cac_request) + cac->num_radio*sizeof(struct cac_tlv);
	mapd_hexdump(MSG_OFF, "NEW CAC_REQ", buff, length);
	wlanif_issue_wapp_command(global, WAPP_USER_SET_CAC_REQ,
			0, NULL, 0, buff, length, 0, 0, 0);

	radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CAC_ONGOING;
	global->dev.ch_planning_R2.ch_plan_state = CHPLAN_STATE_CAC_ONGOING;
	ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_CAC_START;
	global->dev.ch_planning_R2.CAC_on_channel = channel;
	err("start channel cac timeout2 on channel %d", channel);
	SLIST_FOREACH(cap, &radio->cac_cap.cac_capab_head, cac_cap_entry) {
		if (is_channel_in_cac_cap(channel, cap) == 1) {
			total_cac_time = cap->cac_interval[0]*power(16,4) + cap->cac_interval[1]*power(16,2) + cap->cac_interval[2];
			break;
		}
	}
	err("Total cac time: %d", total_cac_time);
	eloop_register_timeout(total_cac_time + 20,
		0,
		channel_cac_timeout2,
		ctx,
		NULL);
	os_free(buff);
}

void switch_temporary_channel(struct mapd_global *global)
{
	struct prefer_info_db *prefer_info = NULL;
	struct channel_setting *setting = NULL;
	struct _1905_map_device *_1905_device = topo_srv_get_1905_device(&global->dev, NULL);
	struct radio_info_db *radio = topo_srv_get_radio_by_band(_1905_device, 36);
	int i = 0;
	setting = os_zalloc(512);
	if(!setting){
		err("alloc fail ");
		return;
	}

	err(" on controller\n");
	SLIST_FOREACH(prefer_info,
		&(radio->chan_preferance.prefer_info_head),
		prefer_info_entry) {
		int i =0;
		for (i = 0; i < prefer_info->ch_num; i++) {
			if (36 == prefer_info->ch_list[i])
				break;
		}
		if (36 == prefer_info->ch_list[i])
			break;
	}
	if(!prefer_info) {
		err("return");
		os_free(setting);
		return;
	}
	setting->ch_set_num = 1;
	setting->chinfo[i].channel = 36;
	setting->chinfo[i].op_class = prefer_info->op_class;
	os_memcpy(setting->chinfo[i].identifier, radio->identifier, ETH_ALEN);
	setting->chinfo[i].reason_code = 0;

	wlanif_issue_wapp_command(global, WAPP_USER_SET_CHANNEL_SETTING, 0,
		NULL, NULL, setting, 512, 0, 0, 0);
	os_free(setting);
}

u8 ch_planning_check_dfs(
	struct mapd_global *global,
	u8 channel)
{
	struct _1905_map_device *_1905_device;
	unsigned int cac_method = 0;
	if (ch_planning_is_ch_dfs(&global->dev,channel)) {
		if(is_CAC_Success(global,channel) == 1){
			err("CAC is already done for this channel, so can switch directly");
			return 0;
		} else if(global->dev.Restart_ch_planning_radar_on_agent) {
			//may cause link break
			err("Current channel on controller is non-operable , so need to switch immediately");
			switch_temporary_channel(global);
		}
		if (global->dev.map_version == DEV_TYPE_R2) {
			if (ch_planning_check_controller_cac_cap(&global->dev, channel, DEDICATED_RADIO)) {
				err("trigger cac on controller third radio");
				ch_planning_trigger_cac_on_cont(global, channel);
				return 1;
			} else {
				err("need to get CAC done by some agent");
				_1905_device = ch_planning_find_agent_for_CAC(global, channel, &cac_method);
				if (_1905_device) {
					err("trigger cac on agent "MACSTR "",MAC2STR( _1905_device->_1905_info.al_mac_addr));
					ch_planning_trigger_cac_msg2(global, _1905_device, channel, cac_method);
					return 1;
				}
			}
			err("directly switch controller channel");
			return 0;
		} else {
			if (ch_planning_check_controller_cac_cap(&global->dev, channel, DEDICATED_RADIO)) {
				err("trigger cac on controller third radio");
				ch_planning_trigger_cac_on_cont(global, channel);
				return 1;
			} else {
				err("directly switch controller channel");
				return 0;
			}
		}
	} else {
		return 2;
	}
}
#if 0
void ch_planning_ch_change_trigger(
	struct mapd_global *global,
	u8 channel)
{
	struct _1905_map_device *_1905_device;
	unsigned int cac_method = 0;
	if (ch_planning_is_ch_dfs(&global->dev,channel)) {
		err("need to get CAC done by some agent");
		_1905_device = ch_planning_find_agent_for_CAC(global, channel, &cac_method);
		if (_1905_device) {
			err("trigger cac on agent "MACSTR "",MAC2STR( _1905_device->_1905_info.al_mac_addr));
			ch_planning_trigger_cac_msg(global, _1905_device, channel, cac_method);
			return;
		} else {
			err("no agent found to do CAC %d", channel);
		}
	}
	err("channel switch to ch %d trigger immediately", channel);
	ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED,channel,&global->dev);
	global->dev.ch_planning_R2.ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
	ch_planning_set_user_preff_ch(global,channel);
}
#endif
u8 ch_planning_filter_channel(
	struct radio_info_db *radio,
	struct scan_result_tlv *scan_res)
{
	u8 band = BAND_2G;
	u8 band_max_bss;
	band = get_band_from_channel(scan_res->channel);
	if (band == BAND_2G) {
		band_max_bss = MAX_BSS_NUM_2G;
	} else {
		band_max_bss = MAX_BSS_NUM_5G;
	}
	if(scan_res->neighbor_num > band_max_bss) {
		err("NeighboringBSSNum %d is over MaxBSSNum",scan_res->neighbor_num );
		return FILTER_OUT;
	}
	return FILTER_IN;
}
void ch_planning_agg_score(
	struct own_1905_device *own_dev,
	struct scan_result_tlv *scan_res)
{
	struct score_info *cumulative_score = NULL;
	SLIST_FOREACH(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score) {
		if(cumulative_score->channel == scan_res->channel &&
			cumulative_score->total_score != -65535) {
			cumulative_score->total_score =
				cumulative_score->total_score + scan_res->ch_score;
			cumulative_score->dev_count ++;
			break;
		}
	}
}
void ch_planning_avg_score(
	struct own_1905_device *own_dev)
{
	struct score_info *cumulative_score = NULL;
	SLIST_FOREACH(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score) {
		if(cumulative_score->dev_count !=0 )
			cumulative_score->avg_score =
				cumulative_score->total_score / cumulative_score->dev_count;
	}
}
u8 ch_planning_get_ch_rank(
	struct own_1905_device *own_dev,
	u8 channel)
{
	struct score_info *cumulative_score = NULL;
	SLIST_FOREACH(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score) {
		if(channel == cumulative_score->channel) {
			return cumulative_score->ch_rank;
		}
	}
	return 0;
}
u8 ch_planning_min_score_margin_check(
	struct own_1905_device *own_dev, 
	struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *_1905_dev = NULL;
	struct radio_info_db *radio = NULL, *temp_radio = NULL;
	struct score_info *cumulative_score = NULL, *max_score_info = NULL, *current_op_ch_info = NULL;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev,ch_info->channel_num);
	u32 per_diff = 0;
	if(!radio) {
		err("radio not found skip score cal");
		return 0;
	}
	err(" check for radio with ch %d", radio->channel[0]);
	SLIST_FOREACH(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score) {
		temp_radio = topo_srv_get_radio_by_band(_1905_dev,cumulative_score->channel);
		if(!temp_radio || temp_radio->band != radio->band)
			continue;
		if(!max_score_info)
			max_score_info = cumulative_score;
		if(cumulative_score->ch_rank > max_score_info->ch_rank) {
			max_score_info = cumulative_score;
		}
		if(cumulative_score->channel == radio->channel[0]) {
			current_op_ch_info = cumulative_score;
		}
	}
	if(!current_op_ch_info){
		err("no current op ch info found in score table");
		return 0;
	}
	err("max score info is ch %d, score %d, rank %d", max_score_info->channel, max_score_info->avg_score, max_score_info->ch_rank);
	err("current ch score info is ch %d, score %d, rank %d", current_op_ch_info->channel, current_op_ch_info->avg_score, current_op_ch_info->ch_rank);
	if(current_op_ch_info->avg_score <= 0) {
		err("current channel is not operable");
		return 0;
	}

	if(max_score_info->channel == current_op_ch_info->channel ||
		max_score_info->avg_score == current_op_ch_info->avg_score) {
		err("give higher rank to current channel");
		current_op_ch_info->ch_rank = max_score_info->ch_rank + 1;
		return 1;
	}
	per_diff = ((POSITIVE(max_score_info->avg_score - current_op_ch_info->avg_score))*100)/(current_op_ch_info->avg_score);
	err("percentage difference of max with current %d, margin %d", per_diff, own_dev->ch_planning_R2.min_score_inc);
	if(per_diff <= own_dev->ch_planning_R2.min_score_inc){
		err("still give highest rank to current channel");
		current_op_ch_info->ch_rank = max_score_info->ch_rank + 1;
		return 1;
	}
	return 0;
}
u8 ch_planning_modify_rank_DFS(
	struct own_1905_device *own_dev, 
	struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *_1905_dev = NULL;
	struct radio_info_db *radio = NULL, *temp_radio = NULL;
	struct score_info *cumulative_score = NULL, *max_score_info_non_DFS = NULL, *max_score_info_DFS = NULL;
	u8 check_DFS = 0;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev,ch_info->channel_num);
	u32 per_diff = 0;
	if(!radio) {
		err("radio not found skip score cal");
		return 0;
	}
	debug(" check for radio with ch %d", radio->channel[0]);
	/*find max non-DFS Ranked entry*/
	SLIST_FOREACH(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score) {
		temp_radio = topo_srv_get_radio_by_band(_1905_dev,cumulative_score->channel);
		if(!temp_radio || temp_radio->band != radio->band)
			continue;
		check_DFS = 0;
		check_DFS = ch_planning_is_ch_dfs(own_dev, cumulative_score->channel);
		debug("check_DFS %d, cumulative_score->channel %d", check_DFS, cumulative_score->channel);
		if(!check_DFS) {
			if(!max_score_info_non_DFS)
				max_score_info_non_DFS = cumulative_score;
			if(cumulative_score->ch_rank > max_score_info_non_DFS->ch_rank) {
				max_score_info_non_DFS = cumulative_score;
			}
		} else {
			if(!max_score_info_DFS)
				max_score_info_DFS = cumulative_score;
			if(cumulative_score->ch_rank > max_score_info_DFS->ch_rank) {
				max_score_info_DFS = cumulative_score;
			}
		}
	}
	
	if(!max_score_info_DFS || !max_score_info_non_DFS){
		debug("no max info found");
		return 0;
	}
	debug("DFS max score info is ch %d, score %d, rank %d", max_score_info_DFS->channel, max_score_info_DFS->avg_score, max_score_info_DFS->ch_rank);
	debug("NON-DFS max score info is ch %d, score %d, rank %d", max_score_info_non_DFS->channel, max_score_info_non_DFS->avg_score, max_score_info_non_DFS->ch_rank);
	
	if(max_score_info_non_DFS->avg_score <= 0) {
		debug("max non-DFS channel is not operable");
		return 0;
	}

	if(max_score_info_DFS->ch_rank < max_score_info_non_DFS->ch_rank) {
		debug("non-DFS is already better than DFS , so no need for rank change");
		return 0;
	} 

	per_diff = ((POSITIVE(max_score_info_DFS->avg_score - max_score_info_non_DFS->avg_score))*100)/(max_score_info_non_DFS->avg_score);
	debug("percentage difference of max with current %d", per_diff);
	if(per_diff <= own_dev->ch_planning_R2.min_score_inc){
		debug("give highest rank to non-DFS channel");
		max_score_info_non_DFS->ch_rank = max_score_info_DFS->ch_rank + 1;
		return 1;
	}
	return 0;
}


void ch_planning_update_channel_rank(
	struct own_1905_device *own_dev, 
	struct monitor_ch_info *ch_info)
{
/*calculate the channel rank based on avg score Rank 1 is lowest */
	struct score_info *cumulative_score = NULL, *current_ch = NULL, *last_current_ch = NULL;
	u8 current_rank = 1, is_current_ch_max = 0;
	
	while(1) {
		current_ch = NULL;
		SLIST_FOREACH(cumulative_score,
					&own_dev->ch_planning_R2.first_ch_score, next_ch_score) {
			if(cumulative_score->ch_rank == 0){
				current_ch = cumulative_score;
				debug("curr ch %d",current_ch->channel);
				break;
			}
		}
		if(current_ch == NULL){
			err("all dev rank done\n");
			break;
		}
		SLIST_FOREACH(cumulative_score,
			&own_dev->ch_planning_R2.first_ch_score, next_ch_score) {
			if(cumulative_score->ch_rank == 0){
				if (cumulative_score->avg_score <= current_ch->avg_score){
					current_ch = cumulative_score;
				}
			}
		}
		if(last_current_ch){
			if(last_current_ch->avg_score == current_ch->avg_score){
				current_ch->ch_rank = last_current_ch->ch_rank;
				debug("ch %d rank %d",current_ch->channel, current_ch->ch_rank);
				continue;
			}
		}
		last_current_ch = current_ch;
		current_ch->ch_rank = current_rank;
		debug("ch %d rank %d",current_ch->channel, current_ch->ch_rank);
		current_rank ++;
	}
/*give higher rank to current radio's channel if its score is not less that certain percentage from
the max rank score , so that there is no need to switch , as not much improvement*/
//find the channel with max rank , compare its score with current channel's score
	is_current_ch_max = ch_planning_min_score_margin_check(own_dev, ch_info);

	if(is_current_ch_max == 0) {
	/*means a new channel will be chosen , so in this case give more preference to non-DFS channel over DFS channel*/
		ch_planning_modify_rank_DFS(own_dev, ch_info);
	}

//only for test  -> avoid setting dfs channel for sometime
#if 0
	err(" ");
	SLIST_FOREACH(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score, next_ch_score) {
		if(ch_planning_is_ch_dfs(own_dev,cumulative_score->channel)){
			cumulative_score->ch_rank = 0;
		}
	}
	err(" ");
#endif
}
#endif
int ch_planning_get_dev_bw_from_channel(
	struct _1905_map_device *_1905_dev,
	u8 channel)
{
	int contr_bw = BW_20;
	struct radio_info_db *radio = NULL;
	radio = topo_srv_get_radio_by_band(_1905_dev,channel);
	if(!radio) {
		err("radio not found skip grouping");
		return -1;
	}
	info("channel[0] %dopclass is %d",
		radio->channel[0], radio->operating_class);
	contr_bw = chan_mon_get_bw_from_op_class(radio->operating_class);
	info("controller's BW is %d", contr_bw);
	return contr_bw;
}
#ifdef MAP_R2
void dump_ch_group_by_bw_info(
	struct own_1905_device *own_dev)
{
	struct grp_score_info *grp_score = NULL;
	u8 i=0;
	err("******* Group Score Table************\n");
	SLIST_FOREACH(grp_score,
		&own_dev->ch_planning_R2.first_grp_score, next_grp_score) {
		for(i=0;i<grp_score->grp_channel_num;i++) {
			err("channel %d",grp_score->grp_channel_list[i]);
		}
		err("grp_total_avg_score %d, grp rank %d", grp_score->grp_total_avg_score,grp_score->grp_rank);
		debug("best ch %d, best ch score %d", grp_score->best_ch, grp_score->best_ch_score);
	}
}

void ch_planning_form_ch_grps(struct own_1905_device *own_dev)
{

//create groups based on BW
	struct grp_score_info *grp_score = NULL;
	u8 grp_num = 12,grp_channel_num = 2;
	u8 grp_bw_160[2][8] = { 
		 {36,40,44,48,52,56,60,64},
		 {100,104,108,112,116,120,124,128}
		};
	u8 grp_bw_80[6][4] = {
		 {36,40,44,48},
		 {52,56,60,64},
		 {100,104,108,112},
		 {116,120,124,128},
		 {132,136,140,144},
		 {149,153,157,161}
		};
	u8 grp_bw_40[12][2] = {
		 {36,40},
		 {44,48},
		 {52,56},
		 {60,64},
		 {100,104},
		 {108,112},
		 {116,120},
		 {124,128},
		 {132,136},
		 {140,144},
		 {149,153},
		 {157,161}
		};
	u8 *grp_list = NULL;
	u8 i = 0, j = 0;
	struct score_info *cumulative_score = NULL;
	if(own_dev->ch_planning_R2.grp_bw == BW_160) {
		grp_num = 2;
		grp_channel_num = 8;
		grp_list = &grp_bw_160[0][0];
	}else if (own_dev->ch_planning_R2.grp_bw == BW_80) {
		grp_num = 6;
		grp_channel_num = 4;
		grp_list = &grp_bw_80[0][0];
	}else if (own_dev->ch_planning_R2.grp_bw == BW_40) {
		grp_num = 12;
		grp_channel_num = 2;
		grp_list = &grp_bw_40[0][0];
	}
	debug("grp_num %d grp_channel_num %d grp_list %p grp_bw_40 %p grp_bw_80 %p, grp_bw_160 %p", 
		grp_num, grp_channel_num, grp_list,
		grp_bw_40,
		grp_bw_80,
		grp_bw_160);
	for(i = 0; i < grp_num; i++) {
		grp_score = os_zalloc(sizeof(struct grp_score_info));
		if (grp_score == NULL) {
			err("alloc memory fail");
			assert(0);
			return;
		}
		grp_score->grp_channel_num = grp_channel_num;
		grp_score->grp_rank = 0;
		grp_score->best_ch = 0;
		grp_score->best_ch_score = 0;
		if(grp_list == NULL) {
			err("grp_list is NULL");
			os_free(grp_score);
			return;
		}
		os_memcpy(&grp_score->grp_channel_list[0],(grp_list+(i*grp_channel_num)),grp_channel_num);
		err("grp list ch[0] %d,ch[1] %d ch[2] %d ch[3] %d ", 
			grp_score->grp_channel_list[0],
			grp_score->grp_channel_list[1],
			grp_score->grp_channel_list[2],
			grp_score->grp_channel_list[3]);
		for(j = 0; j < grp_channel_num; j++) {
			SLIST_FOREACH(cumulative_score,
				&own_dev->ch_planning_R2.first_ch_score,
				next_ch_score) {
				//err(" ");
				//err("grp_score->grp_channel_list[j] %d , cumulative_score->channel %d", 
				//	grp_score->grp_channel_list[j], 
				//	cumulative_score->channel);				
				if(grp_score->grp_channel_list[j] == cumulative_score->channel) {
					grp_score->grp_total_avg_score =
						grp_score->grp_total_avg_score + cumulative_score->avg_score;
					debug("add score for ch %d total_avg_score %d", 
						grp_score->grp_channel_list[j],
						grp_score->grp_total_avg_score);
					break;
				}	
				
			}
			
			if(!cumulative_score) {
				continue;
			}
			if (grp_score->best_ch == 0 ) {
				grp_score->best_ch = grp_score->grp_channel_list[j];
				grp_score->best_ch_score = cumulative_score->avg_score;
			} else if(grp_score->best_ch_score < cumulative_score->avg_score) {
				grp_score->best_ch = grp_score->grp_channel_list[j];
				grp_score->best_ch_score = cumulative_score->avg_score;
			} else if(grp_score->best_ch_score == cumulative_score->avg_score) {
				struct _1905_map_device *_1905_device = topo_srv_get_1905_device(own_dev,NULL);
				struct radio_info_db *temp_radio = topo_srv_get_radio_by_band(_1905_device,cumulative_score->channel);
				if(temp_radio) {
					if(temp_radio->channel[0] == cumulative_score->channel) {
						grp_score->best_ch = grp_score->grp_channel_list[j];
						grp_score->best_ch_score = cumulative_score->avg_score;
					}
				}
			}
		}
		err("grp_score->grp_total_avg_score %d", grp_score->grp_total_avg_score);
		if(own_dev->ch_planning_R2.grp_bw == BW_160 ||
			own_dev->ch_planning_R2.grp_bw == BW_80) {
			//best channel has to be always the primary channel 
			grp_score->best_ch = grp_score->grp_channel_list[0];
		}
		err("total grp avg score %d, best channel %d",
			grp_score->grp_total_avg_score, grp_score->best_ch);
		SLIST_INSERT_HEAD(&(own_dev->ch_planning_R2.first_grp_score),
			grp_score, next_grp_score);
	}
	
	//dump_ch_group_by_bw_info(own_dev);
}


void ch_planning_remove_grp_score_table(
	struct own_1905_device *own_dev)
{
	struct grp_score_info *grp_score = NULL;
	err(" ");
	while (!SLIST_EMPTY(&(own_dev->ch_planning_R2.first_grp_score))) {
		grp_score = SLIST_FIRST(&(own_dev->ch_planning_R2.first_grp_score));
		SLIST_REMOVE_HEAD(&(own_dev->ch_planning_R2.first_grp_score),
			next_grp_score);
		os_free(grp_score);
	}
	err(" ");
}
u8 ch_planning_get_grp_rank(
	struct own_1905_device *own_dev,
	u8 channel)
{
	struct grp_score_info *grp_score = NULL;
	u8 i = 0;
	SLIST_FOREACH(grp_score,
		&own_dev->ch_planning_R2.first_grp_score, next_grp_score) {
		for(i = 0; i < grp_score->grp_channel_num; i++) {
			if(channel == grp_score->grp_channel_list[i]) {
				if(channel == grp_score->best_ch) {
					return (grp_score->grp_rank+1);
				}
				return grp_score->grp_rank;
			}
		}
	}
	return 0;
}
u8 ch_planning_all_grp_rank_done(
	struct own_1905_device *own_dev)
{
	struct grp_score_info *grp_score = NULL;
	SLIST_FOREACH(grp_score,
			&own_dev->ch_planning_R2.first_grp_score, next_grp_score) {
			err("dump grp wth ch %d, rank %d",grp_score->best_ch, grp_score->grp_rank);
		if(grp_score->grp_rank == 0)
			return 0;
	}
	return 1;
}
u8 ch_planning_grp_score_margin_check(
	struct own_1905_device *own_dev, 
	u8 channel)
{
	struct _1905_map_device *_1905_dev = NULL;
	struct radio_info_db *radio = NULL, *temp_radio = NULL;
	struct grp_score_info *grp_score = NULL, *max_score_info = NULL, *current_op_grp_info = NULL;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev,channel);
	u32 per_diff = 0;
	u8 i = 0;
	if(!radio) {
		err("radio not found skip score cal");
		return 0;
	}
	err(" check for radio with ch %d", radio->channel[0]);
	
	SLIST_FOREACH(grp_score,
		&own_dev->ch_planning_R2.first_grp_score, next_grp_score) {
		temp_radio = topo_srv_get_radio_by_band(_1905_dev,channel);
		if(!temp_radio || temp_radio->band != radio->band)
			continue;
		if(!max_score_info)
			max_score_info = grp_score;
		if(grp_score->grp_rank > max_score_info->grp_rank) {
			max_score_info = grp_score;
		}
		for (i = 0; i < grp_score->grp_channel_num; i++) {
			if(grp_score->grp_channel_list[i] == radio->channel[0]) {
				current_op_grp_info = grp_score;
				break;
			}
		}
	}
	if(!current_op_grp_info){
		err("no current op grp info found in score table");
		return 0;
	}
	err("max score info is ch %d, score %d, rank %d", max_score_info->grp_channel_list[0], max_score_info->grp_total_avg_score, max_score_info->grp_rank);
	err("current grp score info is ch %d, score %d, rank %d", current_op_grp_info->grp_channel_list[0], current_op_grp_info->grp_total_avg_score, current_op_grp_info->grp_rank);
	if(current_op_grp_info->grp_total_avg_score <= 0) {
		err("current grp is not operable");
		return 0;
	}

	if(max_score_info->grp_channel_list[0]== current_op_grp_info->grp_channel_list[0] ||
		max_score_info->grp_total_avg_score == current_op_grp_info->grp_total_avg_score) {
		err("give higher rank to current group");
		current_op_grp_info->grp_rank = max_score_info->grp_rank + 1;
		return 1;
	}
	per_diff = ((POSITIVE(max_score_info->grp_total_avg_score - current_op_grp_info->grp_total_avg_score))*100)/(current_op_grp_info->grp_total_avg_score);
	err("percentage difference of max with current %d, margin %d", per_diff, own_dev->ch_planning_R2.min_score_inc);
	if(per_diff <= own_dev->ch_planning_R2.min_score_inc){
		err("still give highest rank to current grp");
		current_op_grp_info->grp_rank = max_score_info->grp_rank + 1;
		return 1;
	}
	return 0;
}
u8 ch_planning_modify_grp_rank_DFS(
	struct own_1905_device *own_dev, 
	u8 channel)
{
	struct _1905_map_device *_1905_dev = NULL;
	struct radio_info_db *radio = NULL, *temp_radio = NULL;
	struct grp_score_info *cumulative_score = NULL, *max_score_info_non_DFS = NULL, *max_score_info_DFS = NULL;
	u8 check_DFS = 0;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev,channel);
	u32 per_diff = 0;
	if(!radio) {
		err("radio not found skip score cal");
		return 0;
	}
	err(" check for radio with ch %d", radio->channel[0]);
	/*find max non-DFS Ranked entry*/
	SLIST_FOREACH(cumulative_score,
		&own_dev->ch_planning_R2.first_grp_score, next_grp_score) {
		temp_radio = topo_srv_get_radio_by_band(_1905_dev,cumulative_score->grp_channel_list[0]);
		if(!temp_radio || temp_radio->band != radio->band)
			continue;
		check_DFS = 0;
		check_DFS = ch_planning_is_ch_dfs(own_dev, cumulative_score->grp_channel_list[0]);
		err("check_DFS %d, cumulative_score->channel %d", check_DFS, cumulative_score->grp_channel_list[0]);
		if(!check_DFS) {
			if(!max_score_info_non_DFS)
				max_score_info_non_DFS = cumulative_score;
			if(cumulative_score->grp_rank > max_score_info_non_DFS->grp_rank) {
				max_score_info_non_DFS = cumulative_score;
			}
		} else {
			if(!max_score_info_DFS)
				max_score_info_DFS = cumulative_score;
			if(cumulative_score->grp_rank > max_score_info_DFS->grp_rank) {
				max_score_info_DFS = cumulative_score;
			}
		}
	}
	
	if(!max_score_info_DFS || !max_score_info_non_DFS){
		err("no max info found");
		return 0;
	}
	err("DFS max score info is ch %d, score %d, rank %d", max_score_info_DFS->grp_channel_list[0], max_score_info_DFS->grp_total_avg_score, max_score_info_DFS->grp_rank);
	err("NON-DFS max score info is ch %d, score %d, rank %d", max_score_info_non_DFS->grp_channel_list[0], max_score_info_non_DFS->grp_total_avg_score, max_score_info_non_DFS->grp_rank);
	
	if(max_score_info_non_DFS->grp_total_avg_score <= 0) {
		err("max non-DFS channel is not operable");
		return 0;
	}

	if(max_score_info_DFS->grp_rank < max_score_info_non_DFS->grp_rank) {
		err("non-DFS is already better than DFS , so no need for rank change");
		return 0;
	} 

	per_diff = ((POSITIVE(max_score_info_DFS->grp_total_avg_score - max_score_info_non_DFS->grp_total_avg_score))*100)/(max_score_info_non_DFS->grp_total_avg_score);
	err("percentage difference of max with current %d", per_diff);
	if(per_diff <= own_dev->ch_planning_R2.min_score_inc){
		err("give highest rank to non-DFS channel");
		max_score_info_non_DFS->grp_rank = max_score_info_DFS->grp_rank + 1;
		return 1;
	}
	return 0;
}

void ch_planning_update_grp_rank(
	struct own_1905_device *own_dev, 
	u8 channel)
{
	struct grp_score_info *cumulative_score = NULL, *current_ch = NULL, *last_current_ch = NULL;
	u8 current_rank = 1,is_current_grp_max = 0;;
	
	while(1) {
		current_ch = NULL;
		SLIST_FOREACH(cumulative_score,
					&own_dev->ch_planning_R2.first_grp_score, next_grp_score) {
			if(cumulative_score->grp_rank == 0){
				current_ch = cumulative_score;
				debug("curr ch %d",current_ch->best_ch);
				break;
			}
		}
		if(current_ch == NULL){
			err("all dev rank done\n");
			break;
		}
		SLIST_FOREACH(cumulative_score,
			&own_dev->ch_planning_R2.first_grp_score, next_grp_score) {
			if(cumulative_score->grp_rank == 0){
				if (cumulative_score->grp_total_avg_score<= current_ch->grp_total_avg_score){
					current_ch = cumulative_score;
				}
			}
		}
		if(last_current_ch){
			if(last_current_ch->grp_total_avg_score == current_ch->grp_total_avg_score){
				current_ch->grp_rank = last_current_ch->grp_rank;
				debug("ch %d rank %d",current_ch->best_ch, current_ch->grp_rank);
				continue;
			}
		}
		last_current_ch = current_ch;
		current_ch->grp_rank = current_rank;
		debug("ch %d rank %d",current_ch->best_ch, current_ch->grp_rank);
		current_rank ++;
	}


//after update group ranks dump
	//err("after update group rank, dump group by bw info");
	//dump_ch_group_by_bw_info(own_dev);

	is_current_grp_max = ch_planning_grp_score_margin_check(own_dev,channel);
	err("after score margin check dump group by bw info, is_current_grp_max %d", is_current_grp_max);
	
	if(is_current_grp_max == 0) {
		ch_planning_modify_grp_rank_DFS(own_dev,channel);
	}
	dump_ch_group_by_bw_info(own_dev);
return;
}

void ch_planning_group_ch_by_bw(
	struct own_1905_device *own_dev,
	struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *_1905_dev = NULL;
	//struct radio_info_db *radio = NULL, *temp_radio = NULL;
	//struct score_info *cumulative_score = NULL, *max_score_info = NULL, *current_op_ch_info = NULL;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	
	if(ch_info->channel_num <=14) {
		err("this feature is not supported for 2.4G");	
		return;
	}
	//find the current BW of controller 
	own_dev->ch_planning_R2.grp_bw = ch_planning_get_dev_bw_from_channel(_1905_dev, ch_info->channel_num);	

	if(own_dev->ch_planning_R2.grp_bw == BW_20) {
		err("no need for BW based grouping as grp bw is 20MHz");
		return;
	}
	err(" ");

	//based on this BW make link list of score with bw
	//clean-up the existing list if any
	ch_planning_remove_grp_score_table(own_dev);
	if (SLIST_EMPTY(&(own_dev->ch_planning_R2.first_grp_score))) {
		SLIST_INIT(&own_dev->ch_planning_R2.first_grp_score);
	}
	//create channel groups based on BW
	ch_planning_form_ch_grps(own_dev);
	err(" ");
	//update the rank of each ch group 
	ch_planning_update_grp_rank(own_dev, ch_info->channel_num);
	err(" ");
}

u8 ch_planning_is_ch_switch_req(
	struct own_1905_device *own_dev,
	struct monitor_ch_info *ch_info,
	struct score_info *best_channel_info)
{
	struct radio_info_db *temp_radio= NULL;
	u16 min_score_inc = own_dev->ch_planning_R2.min_score_inc;
	struct score_info *current_working_ch_info = NULL;

	/*current working radio info*/
	temp_radio =
		topo_srv_get_radio_by_channel(
		SLIST_FIRST(&own_dev->_1905_dev_head),
		ch_info->channel_num);
	if(!temp_radio) {
		err("radio not found with channel %d",ch_info->channel_num);
		return 0;
	}
	debug("best_channel %d,radio->channel %d",
		best_channel_info->channel, temp_radio->channel[0]);
	if (best_channel_info->channel != temp_radio->channel[0]) {
		err("current channel is different from best channel ");
		current_working_ch_info =
			ch_planning_find_current_ch_score_info(own_dev, temp_radio);
		if(!current_working_ch_info) {
			err("current working ch_info not found ");
			return 0;
		}
		err("current_working ch %d, score %d", current_working_ch_info->channel,
			current_working_ch_info->total_score);
		/*check score margin*/
		if (best_channel_info->total_score >
			current_working_ch_info->total_score + min_score_inc) {
			err("need to trigger ch switch");
			return 1;
		} else {
			err("no need to trigger ch change as score margin not met");
			return 0;
		}
	} else {
		err("no need to trigger ch change as score of current ch is the best");
	}
	return 0;
}
void ch_planning_select_best_channel_R2(
	struct mapd_global *global,
	struct monitor_ch_info *ch_info)
{
	struct own_1905_device *own_dev = &global->dev;
	struct _1905_map_device *_1905_dev = NULL;
	struct radio_info_db *radio = NULL;
	struct scan_result_tlv *scan_res = NULL;
	//struct score_info *best_channel_info = NULL;
	u8 filter_status;//, trigger_status;
	if(!ch_info) {
		err("ch_info is NULL BEWARE ");
		_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
		radio = topo_srv_get_radio_by_band(_1905_dev,ch_info->channel_num);
		ch_planning_R2_reset(own_dev,radio);
		handle_task_completion(own_dev);
		return;
	}
	ch_planning_clear_score_table(own_dev, ch_info);
	info("##### Score Calculation start #####");
	SLIST_FOREACH(_1905_dev, &own_dev->_1905_dev_head, next_1905_device) {
		//radio = topo_srv_get_radio_by_channel(_1905_dev,ch_info->channel_num);
		radio = topo_srv_get_radio_by_band(_1905_dev,ch_info->channel_num);
		if(!radio) {
			err("radio not found skip score cal");
			continue;
		}
		info("dev ALMAC : "MACSTR"",
			MAC2STR(_1905_dev->_1905_info.al_mac_addr));
		SLIST_FOREACH(scan_res, &radio->first_scan_result, next_scan_result) {
			/*ch_planning_filter_unqualified_channels*/
			filter_status = ch_planning_filter_channel(radio, scan_res);
			if (filter_status == FILTER_OUT) {
				info("filter_out ch %d at this dev", scan_res->channel);
				scan_res->ch_score = 0;
				struct score_info *cumulative_score = NULL;
				SLIST_FOREACH(cumulative_score,
					&own_dev->ch_planning_R2.first_ch_score, next_ch_score) {
					if(cumulative_score->channel == scan_res->channel) {
						cumulative_score->total_score = -65535;
					}
				}
				continue;
			}
			/*channel score calculation*/
			ch_planning_calc_score(own_dev,_1905_dev,radio,scan_res);
			/*keep aggregating scores from all agents*/
			ch_planning_agg_score(own_dev,scan_res);
		} //all scan result loop
	}//all 1905 device loop

	ch_planning_avg_score(own_dev);
	ch_planning_update_channel_rank(own_dev, ch_info);
	dump_ch_planning_score_info(own_dev);
	if(own_dev->ch_planning_R2.ch_plan_enable_bw){
		ch_planning_group_ch_by_bw(own_dev, ch_info);
	}
	own_dev->ch_planning_R2.ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
	_1905_dev = topo_srv_get_1905_device(own_dev,NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev,ch_info->channel_num);
	ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED,radio->channel[0],own_dev);
	mapd_restart_channel_plannig(global);
}

u8 ch_planning_check_scan_result(struct own_1905_device *own_dev,
			struct monitor_ch_info *ch_info)
{
	struct _1905_map_device *dev = NULL;
	struct radio_info_db *temp_radio = NULL;
	struct scan_list_info *list = NULL;
	u8 count = 0, pending = 0, received = 0;

	err("for ch %d",ch_info->channel_num);
	SLIST_FOREACH(dev, &own_dev->_1905_dev_head, next_1905_device) {
		temp_radio = topo_srv_get_radio_by_channel(dev, ch_info->channel_num);
		if(temp_radio)
			count++;
	}

	SLIST_FOREACH(list, &own_dev->ch_planning_R2.first_scan_list, next_scan_list) {
		pending ++;
	}
	/*find number of scan results received (count - pending)*/
	received = count - pending;
	err("Received %d, count %d, pending %d", received,count,pending);
	if(received >= (count/2)) {
		return 1;
	} else
		return 0;

}

void ch_planning_handle_ch_scan_rep(struct mapd_global *global)
{
	struct monitor_ch_info *ch_info, *tmp_ch_info;
	struct own_1905_device *ctx = &global->dev;
	struct os_reltime rem_time = {0};
	Boolean timedout = 0;

	debug("global->dev.ch_planning_R2.ch_plan_state %d ", global->dev.ch_planning_R2.ch_plan_state);
	if(global->dev.ch_planning_R2.ch_plan_state != CHPLAN_STATE_SCAN_ONGOING) {
		err("scan is not ongoing , so return");
		return;
	}
	if(SLIST_EMPTY(&global->dev.ch_planning_R2.first_scan_list)) { 
		err("All scan report were received success");
		/* all scan report were received success*/
		global->dev.ch_planning_R2.ch_plan_state = CHPLAN_STATE_SCAN_COMPLETE;
		SLIST_FOREACH(ch_info,
			&global->dev.ch_planning_R2.first_monitor_ch, next_monitor_ch) {
			if(ch_info->trigger_status == TRIGGER_TRUE) {
				debug("ch scan timeout cancel for ch_info ch  %d", ch_info->channel_num);
				eloop_cancel_timeout(ch_scan_timeout, &global->dev, ch_info);
				eloop_cancel_timeout(ch_scan_timeout_mtk, &global->dev, ch_info);
				break;
			}
		}
	//	dump_ch_planning_info(&global->dev, 0);
		ch_planning_select_best_channel_R2(global, ch_info);
	} else {
			/* If non mtk agent in the network then check
			ch_scan_timeout_mtk is timedout and we have more than 50% result */
			if (is_mixed_network(ctx, 0)) {
				SLIST_FOREACH(tmp_ch_info,
						&global->dev.ch_planning_R2.first_monitor_ch, next_monitor_ch) {
					if((tmp_ch_info->trigger_status == TRIGGER_TRUE)) {
						eloop_get_remaining_timeout(&rem_time,ch_scan_timeout_mtk,ctx,tmp_ch_info);
						timedout = (rem_time.sec == 0) ? 1 : 0;
						break;
					}
				}
				err("remaining time: %lu, timedout: %d", rem_time.sec, timedout);
				if (timedout == 1) {
					err("Mixed network and timedout");
					SLIST_FOREACH(ch_info,
							&global->dev.ch_planning_R2.first_monitor_ch, next_monitor_ch) {
						/* trigger is TRUE and we have received half of scan result */
						if((ch_info->trigger_status == TRIGGER_TRUE) &&
								ch_planning_check_scan_result(ctx, ch_info)) {
							err("ch scan timeout cancel for ch_info ch	%d", ch_info->channel_num);
							global->dev.ch_planning_R2.ch_plan_state = CHPLAN_STATE_SCAN_COMPLETE;
							eloop_cancel_timeout(ch_scan_timeout, &global->dev, ch_info);
							ch_planning_select_best_channel_R2(global, ch_info);
							/*still some agents have not sent scan results , so clean complete scan list entry */
							ch_planning_remove_scan_list(ctx);
							break;
						}
					}
				}
			}
	}
}
void ch_planning_get_channel_list(
	struct own_1905_device *own_dev,
	u8 *channel_arr)
{
	struct score_info *entry;
	u8 i = 0;
	SLIST_FOREACH(entry,&(own_dev->ch_planning_R2.first_ch_score),next_ch_score) {
		channel_arr[i] = entry->channel;
		err("channel list %d", channel_arr[i]);
		i++;
	}
}

#if 0
u8 is_channel_preferred_by_all(
	struct own_1905_device *ctx,
	u8 channel)
{
	u8 total_dev_count = 0;
	u8 status = 0;
	struct radio_info_db *radio = NULL;
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct prefered_ch_cb *prefered_channel = NULL;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct _1905_map_device *_1905_dev = NULL;
	if (channel > 14)
	{
		info("5G\n");
		ch_distribution = &ch_planning->ch_ditribution_5g;
	} else {
		info("2.4G\n");
		ch_distribution = &ch_planning->ch_ditribution_2g;
	}
	SLIST_FOREACH(_1905_dev, &ctx->_1905_dev_head, next_1905_device) {
		radio = topo_srv_get_radio_by_band(_1905_dev, channel);
		if(radio)
			total_dev_count ++;
	}
	SLIST_FOREACH(prefered_channel,
		&(ch_distribution->first_prefered_ch),
		next_prefered_ch) {
		if(prefered_channel->ch_num == channel)
			break;
	}
	if(prefered_channel->radio_count == total_dev_count) {
		return 1;
	}
return 0;
}
#endif
void ch_planning_send_scan_req_radio(
	struct mapd_global *global,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
	struct channel_scan_req *scan_req = NULL;
	struct prefer_info_db *prefer_db = NULL;
	u8 buf[3000] = {0};
	u8 i, opidx = 0;
	//u8 scan_ch_list[MAX_CH_NUM] = {0};
	u16 length = 0;

	scan_req = (struct channel_scan_req *)buf;
	scan_req->fresh_scan = 0x80;
	scan_req->radio_num = 0;
	os_memcpy(scan_req->body[0].radio_id, radio->identifier, ETH_ALEN);
	scan_req->body[0].oper_class_num = 0;
	debug("radio ID "MACSTR"", MAC2STR(radio->identifier));
	SLIST_FOREACH(prefer_db,
		&radio->chan_preferance.prefer_info_head, prefer_info_entry) {
		if (prefer_db->perference == 0)
			continue;
		/*we only support scan on 20MHz opclass */
		if((radio->band > BAND_2G) && (prefer_db->op_class != 118 &&
							prefer_db->op_class != 115 &&
							prefer_db->op_class != 125 &&
							prefer_db->op_class != 121))
			continue;
		if((radio->band == BAND_2G) && (prefer_db->op_class != 81 &&
							prefer_db->op_class != 82 ))
			continue;
		scan_req->body[0].ch_body[opidx].oper_class = prefer_db->op_class;
		debug("oper_class %d", scan_req->body[0].ch_body[opidx].oper_class);
		scan_req->body[0].ch_body[opidx].ch_list_num = 0;
		debug("ch_list_num %d", scan_req->body[0].ch_body[opidx].ch_list_num);
		for(i = 0; i < prefer_db->ch_num; i++) {
#if 0 
			is_intersect = 0;
			is_intersect = is_channel_preferred_by_all(prefer_db->ch_list[i]);
			if(is_intersect == 0) {
				err("don't add ch %d as not common",prefer_db->ch_list[i]);
				continue;
			}
#endif
			if (prefer_db->ch_list[i] == 50 ||
				prefer_db->ch_list[i] == 114 ||
				prefer_db->ch_list[i] == 165 ||
				prefer_db->ch_list[i] == 169) {
				debug("160 Mhz Channel, skip %d\n",
					prefer_db->ch_list[i]);
				continue;
			}
			scan_req->body[0].ch_body[opidx].ch_list_num++;
			scan_req->body[0].ch_body[opidx].ch_list[i] = prefer_db->ch_list[i];
			debug("scan list %d", prefer_db->ch_list[i]);
			ch_planning_insert_ch_score_table(
				&global->dev,prefer_db->ch_list[i]);
		}
		scan_req->body[0].oper_class_num ++;
		opidx ++;
	}
	scan_req->radio_num++;
	if (dev->device_role == DEVICE_ROLE_CONTROLLER) {
		//ch_planning_get_channel_list(&global->dev, scan_ch_list);
		/*send scan request to owndev */
		//send_off_ch_scan_req(global, dev, SCAN_MODE_CH, 0, scan_ch_list, 0,0);
		length = sizeof(struct channel_scan_req) + scan_req->radio_num*sizeof(struct scan_body);
		scan_req->neighbour_only = 2;//NB_ALL;
		mapd_hexdump(MSG_OFF, "MAPD SCAN_REQ", buf, length);
		map_get_info_from_wapp(&global->dev, WAPP_USER_SET_CHANNEL_SCAN_REQ, 0, NULL, NULL, (void *)buf, length);
	} else {
		err("Send scan req to other dev"MACSTR"", MAC2STR(dev->_1905_info.al_mac_addr));
		map_1905_Send_Channel_Scan_Request_Message(
			global->_1905_ctrl,
			(char *)dev->_1905_info.al_mac_addr,
			scan_req->fresh_scan,
			scan_req->radio_num,
			(unsigned char *)scan_req->body);
	}
}

void ch_planning_controller_fill_scan_results(
	struct own_1905_device *ctx,
	struct _1905_map_device *own_1905_device,
	struct net_opt_scan_report_event *scan_rep_evt)
{
	u8 i = 0, j = 0;
	struct radio_info_db *radio = NULL;
	u8 * buf = NULL;
	buf = (u8 *)scan_rep_evt->scan_result;
	debug(" scan_rep_evt->scan_result_num %d", scan_rep_evt->scan_result_num);
	for(i = 0; i < scan_rep_evt->scan_result_num; i++) {
		struct net_opt_scan_result_event *rep = (struct net_opt_scan_result_event *)buf;
		debug("scan res num %d , channel %d", i, rep->channel);
		radio = topo_srv_get_radio(own_1905_device,rep->radio_id);
		if(!radio) {
			err("radio not found");
			continue;
		}
		struct scan_result_tlv *new_scan_result = os_zalloc(sizeof(struct scan_result_tlv));
		if (!new_scan_result) {
			err("alloc memory fail");
			return ;
		}
		//SLIST_INSERT_HEAD(&(radio->first_scan_result), new_scan_result, next_scan_result);
		//SLIST_INIT(&(new_scan_result->first_neighbor_info));
		os_memcpy(new_scan_result->radio_id,rep->radio_id, ETH_ALEN);
		new_scan_result->oper_class = rep->oper_class;
		new_scan_result->channel = rep->channel;
		new_scan_result->scan_status = rep->scan_status;
		debug("Scan status %d",new_scan_result->scan_status);
		if (new_scan_result->scan_status != SCAN_SUCCESS){
			debug("new_scan_result->channel %d", new_scan_result->channel);
			err("scan fail");
			buf +=sizeof(struct net_opt_scan_result_event);
			if (new_scan_result->scan_status == OP_CLASS_CHAN_NOT_SUPP) {					
				os_free(new_scan_result);
				continue;
			} else {		
				os_free(new_scan_result);
				ch_planning_scan_restart_due_to_failure(ctx->back_ptr);
				return;
			}
		}
		SLIST_INSERT_HEAD(&(radio->first_scan_result), new_scan_result, next_scan_result);
		SLIST_INIT(&(new_scan_result->first_neighbor_info));
		debug(" new_scan_result->channel %d", new_scan_result->channel);
		new_scan_result->timestamp_len = rep->timestamp_len;
		os_memcpy(new_scan_result->timestamp, rep->timestamp, new_scan_result->timestamp_len);
		new_scan_result->utilization = rep->utilization;
		new_scan_result->noise = rep->noise;
		new_scan_result->neighbor_num = rep->neighbor_num;
		info("Add scan report for channel %d, Util %d NB %d",
				new_scan_result->channel,
				new_scan_result->utilization,
				new_scan_result->neighbor_num);
		debug("neighbor_num %d", new_scan_result->neighbor_num);
		for (j = 0; j < new_scan_result->neighbor_num; j++) {
			struct nb_info *nb_info = os_zalloc(sizeof(struct nb_info));
			struct neighbor_info *src_nb = &rep->nb_info[j];
			if(!nb_info) {
				err("alloc memory fail");
				os_free(new_scan_result);
				return;
			}
			SLIST_INSERT_HEAD(&(new_scan_result->first_neighbor_info), 
				nb_info, next_neighbor_info);
			os_memcpy(nb_info->bssid, src_nb->bssid, ETH_ALEN);
			nb_info->ssid_len = src_nb->ssid_len;
			os_memcpy(nb_info->ssid, src_nb->ssid, nb_info->ssid_len);
			info("neigh %s",src_nb->ssid);
			nb_info->RCPI = src_nb->RCPI;
			nb_info->ch_bw_len = src_nb->ch_bw_len;
			os_memcpy(nb_info->ch_bw,src_nb->ch_bw, nb_info->ch_bw_len);
			nb_info->cu_stacnt_present = src_nb->cu_stacnt_present;
			if (nb_info->cu_stacnt_present & 0x80)
				nb_info->cu = src_nb->cu;
			if (nb_info->cu_stacnt_present & 0x40) {
				nb_info->sta_cnt = src_nb->sta_cnt;
			}
		}
		new_scan_result->cu_distribution.ch_num = rep->channel;
		new_scan_result->cu_distribution.edcca_airtime = rep->edcca;
		buf +=sizeof(struct net_opt_scan_result_event)+ (rep->neighbor_num*sizeof(struct neighbor_info));
		topo_srv_update_ch_plan_scan_done(ctx,own_1905_device,radio);
	}
}

void ch_planning_handle_controller_scan_result(
	struct own_1905_device *ctx,
	struct net_opt_scan_report_event *scan_rep_evt)
{
	struct _1905_map_device *own_1905_mapdevice = topo_srv_get_next_1905_device(ctx, NULL);
	err(" ");
	//ch_planning_remove_all_scan_results(own_1905_mapdevice);//sonal test 
	ch_planning_remove_radio_scan_results(ctx, own_1905_mapdevice);
	ch_planning_controller_fill_scan_results(ctx, own_1905_mapdevice, scan_rep_evt);
	ch_planning_handle_ch_scan_rep(ctx->back_ptr);
	err(" ");
}
void ch_planning_send_scan_req(
	struct own_1905_device *ctx,
	struct _1905_map_device *_1905_device,
	struct radio_info_db *radio)
{
	u8 is_profile2 = 0;
	radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_SCAN_ONGOING;
	ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_SCAN_ONGOING;
	if(_1905_device->device_role == DEVICE_ROLE_CONTROLLER) {
		debug("cancel monitor timer for radio with channel %d", radio->channel[0]);
		/*cancel timer for own dev metric reporting from wapp */
		eloop_cancel_timeout(ch_planning_own_dev_get_metric_timeout, ctx, radio);
	} else {
		debug("restore policy as now going to trigger scan");
		/*Update policy to restore original setting after monitor timeout when scan is triggered*/
		steer_msg_update_policy_config(ctx->back_ptr, _1905_device);
	}

	/*Add check here to send scan req only to a MAP R2 dev that supports fresh scan
	otherwise update state of dev as scan done*/
	is_profile2 = (_1905_device->map_version == DEV_TYPE_R2);//is_profile2_dev(ctx->back_ptr,_1905_device->_1905_info.al_mac_addr);
	err("is_profile2 %d, _1905_device->map_version %d", is_profile2, _1905_device->map_version);
	err("for dev with ALMAC "MACSTR"bootOnly scan %d",MAC2STR(_1905_device->_1905_info.al_mac_addr),
		radio->radio_scan_params.boot_scan_only);
	if((_1905_device->map_version != DEV_TYPE_R2) ||
		(_1905_device->map_version == DEV_TYPE_R2 && radio->radio_scan_params.boot_scan_only == 1) ||
		(!_1905_device->in_network)) {
		err("Scan is not allowed %d, %d ", is_profile2, radio->radio_scan_params.boot_scan_only);
		radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_SCAN_COMPLETE;
		return;
	}
	ch_planning_insert_scanlist_entry(ctx, _1905_device, radio);
	ch_planning_send_scan_req_radio(ctx->back_ptr, _1905_device, radio);

}

void ch_planning_scan_trigger(
	struct own_1905_device *ctx,
	struct monitor_ch_info *ch_info)
{
	struct radio_info_db *radio = NULL;
	u32 scan_wait_timeout = 0;
	struct _1905_map_device *_1905_device = NULL;
	err(" for ch %d",ch_info->channel_num);
	SLIST_FOREACH( _1905_device,&ctx->_1905_dev_head,next_1905_device) {
		//radio = topo_srv_get_radio_by_channel(_1905_device, ch_info->channel_num);
		radio = topo_srv_get_radio_by_band(_1905_device, ch_info->channel_num);
		if(!radio){
			err("radio not found can't give scan req");
			continue;
		}
		if(radio->dev_ch_plan_info.dev_ch_plan_state >=
			CHPLAN_STATE_SCAN_ONGOING) {
			err("ch planning is already ongoing , defer scan");
			return;
		}
		scan_wait_timeout = get_scan_wait_time(_1905_device, radio);
		if(scan_wait_timeout) {
			eloop_register_timeout(scan_wait_timeout,
				0,
				ch_planning_scan_wait_timeout,
				ctx,
				_1905_device);
			return;
		} else {
			ch_planning_send_scan_req(ctx, _1905_device, radio);
		}
	}

	/* Check for 3rd party agent, if yes then scan timeout will be 5min */
		if (!is_mixed_network(ctx, 0)) {
			err("all mtk devices");
			eloop_register_timeout(8, 0, ch_scan_timeout, ctx, ch_info);
		} else {
			err("mixed devices");
			eloop_register_timeout(8, 0, ch_scan_timeout_mtk, ctx, ch_info);
			eloop_register_timeout(300, 0, ch_scan_timeout, ctx, ch_info);
		}
}

void ch_planning_remove_monitor_ch(
	struct own_1905_device *own_dev,
	struct monitor_ch_info *ch_info)
{
	struct monitor_ch_info *temp_ch_info;
	struct affected_agent_info *agent = NULL;
	SLIST_FOREACH(temp_ch_info,
		&own_dev->ch_planning_R2.first_monitor_ch, next_monitor_ch) {
		if(ch_info->channel_num == temp_ch_info->channel_num) {
			/*remove all the list of affected agents that we had saved earlier */
			while (!SLIST_EMPTY(&(temp_ch_info->first_affected_agent))) {
				agent = SLIST_FIRST(&(temp_ch_info->first_affected_agent));
				SLIST_REMOVE_HEAD(&(temp_ch_info->first_affected_agent),
					next_affected_agent);
				os_free(agent);
			}
			SLIST_REMOVE(&(own_dev->ch_planning_R2.first_monitor_ch),
				temp_ch_info,
				monitor_ch_info,
				next_monitor_ch);
			os_free(temp_ch_info);
			return;
		}
	}
}
u8 is_mixed_network(struct own_1905_device *ctx, Boolean ignore_edcca)
{
	struct _1905_map_device *dev = NULL;

	if (ignore_edcca)
		return 1;//sonal test Sonal hard code as edcca value is coming 0 always right now 
	else {
		SLIST_FOREACH(dev, &ctx->_1905_dev_head, next_1905_device) {
			if(dev->vendor != VENDOR_MEDIATEK)
				return 1;
		}
	}
	return 0;
}

void channel_monitor_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct monitor_ch_info *ch_info = (struct monitor_ch_info *)timeout_ctx;
	struct own_1905_device *ctx = (struct own_1905_device *)eloop_ctx;
	err(".......Monitor timeout channel %d......", ch_info->channel_num);

	ctx->ch_planning_R2.skip_edcca_check = is_mixed_network(ctx, 1);

	if(ctx->ch_planning_R2.force_trigger == 1) {
		err("CH planning force trigger");
		ch_info->trigger_status = TRIGGER_TRUE;
	} else {
		ch_info->trigger_status = ch_planning_check_trigger(ctx, ch_info);
		debug("Ch planning trigger status %d", ch_info->trigger_status);
	}
	
	err("after monitor timeout start monitor prohibit time");
	os_get_time(&ctx->ch_planning_R2.ch_monitor_start_ts);
	if(ch_info->trigger_status == TRIGGER_TRUE) {
		err("CH planning scan trigger");
		ch_planning_scan_trigger(ctx, ch_info);
	} else if(ch_info->trigger_status == TRIGGER_FALSE) {
		err("CH planning remove monitor ch");
		ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_IDLE;
		ch_planning_update_all_dev_state((u8)CHPLAN_STATE_IDLE,ch_info->channel_num,ctx);
		struct _1905_map_device *_1905_device = NULL;
		struct radio_info_db *reset_radio = NULL;
		_1905_device = topo_srv_get_1905_device(ctx, NULL);
		reset_radio = topo_srv_get_radio_by_band(_1905_device, ch_info->channel_num);
		ch_planning_restore_policy(ctx, reset_radio);
		ch_planning_remove_monitor_ch(ctx, ch_info);
	} else if (ch_info->trigger_status == TRIGGER_PENDING) {
		err("CH planning insert into pending list");
		insert_into_task_list(ctx, TASK_CHANNEL_PLANNING_TRIGGER,
			ch_info, NULL, NULL);
	}
}
u8 ch_planning_search_monitor_ch(
	struct own_1905_device * ctx,
	struct radio_info_db *radio)
{
	struct monitor_ch_info *ch = NULL;
	if(!radio) {
		debug("radio is NULL, ignore search");
		return 0;
	}
	SLIST_FOREACH(ch,
		&(ctx->ch_planning_R2.first_monitor_ch), next_monitor_ch) {
		if (radio->channel[0] == ch->channel_num) {
			return 1;
		}
	}
	debug("this channel is not in monitor list %d", radio->channel[0]);
	return 0;
}
void ch_planning_own_dev_get_metric_timeout(
	void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *ctx = (struct own_1905_device *)eloop_ctx;
	struct radio_info_db *radio = (struct radio_info_db *)timeout_ctx;
	u8 metric_policy_interval =
		ctx->ch_planning_R2.ch_plan_metric_policy_interval;
	own_dev_get_metric_info(ctx->back_ptr, radio);
	eloop_register_timeout(metric_policy_interval,
		0, ch_planning_own_dev_get_metric_timeout, ctx, radio);
}

void ch_planning_update_ap_metric_policy(
	struct own_1905_device *ctx,
	struct monitor_ch_info *monitor_ch,
	u8 metric_policy_interval)
{
	/*change policy for AP metric reporting for all devices on that channel*/
	struct _1905_map_device *tmp_dev = NULL;
	struct radio_info_db *tmp_radio = NULL;
	struct lib_steer_radio_policy *radio_policy = NULL;
	struct lib_metrics_radio_policy *metrics_policy = NULL;
	struct mapd_global *global = ctx->back_ptr;
	if(metric_policy_interval == 0) {
		metric_policy_interval =
			ctx->ch_planning_R2.ch_plan_metric_policy_interval;
	}
	struct lib_unsuccess_assoc_policy *assoc_policy = os_zalloc(sizeof(struct lib_unsuccess_assoc_policy));

	radio_policy = os_zalloc(sizeof(struct lib_steer_radio_policy));
	metrics_policy = os_zalloc(sizeof(struct lib_metrics_radio_policy));
	if(metrics_policy == NULL || radio_policy == NULL) {
		if(metrics_policy)
			os_free(metrics_policy);
		if(radio_policy)
			os_free(radio_policy);
		err("memory alloc fail, can't update policy");
		if(assoc_policy)
			os_free(assoc_policy);
		return;
	}
	if(assoc_policy == NULL) {
		os_free(metrics_policy);
		os_free(radio_policy);
		mapd_ASSERT(0);
		return;
	}
	SLIST_FOREACH(tmp_dev, &ctx->_1905_dev_head, next_1905_device) {
		tmp_radio = topo_srv_get_radio_by_channel(tmp_dev, monitor_ch->channel_num);
		if(!tmp_radio) {
			err("failed to find radio on this temp dev ");
			continue;
		}
		if(tmp_dev->device_role == DEVICE_ROLE_CONTROLLER) {
			err("start metric rep timer for owndev tmp_radio ch  %d", tmp_radio->channel[0]);
			/*start timer for own_dev's info fetching from WAPP*/
			eloop_register_timeout(metric_policy_interval,
				0, ch_planning_own_dev_get_metric_timeout, ctx, tmp_radio);
			continue;
		}
		if(get_default_radio_policy(ctx, tmp_radio, radio_policy) != 0) {
			err("get radio_policy fail, can't update policy");
			if(metrics_policy)
				os_free(metrics_policy);
			if(radio_policy)
				os_free(radio_policy);
			if(assoc_policy)
				os_free(assoc_policy);
			return;
		}
		os_memcpy(metrics_policy->identifier, tmp_radio->identifier, ETH_ALEN);
		debug("send command to agent");
		assoc_policy->report_switch = 1;
		assoc_policy->report_rate = 10;
		/*no need to fill other metric policy contents, as reporting will be done as per interval*/
		if (tmp_dev->map_version == DEV_TYPE_R2)
			map_1905_Send_MAP_Policy_Request_Message (
				global->_1905_ctrl,
				(char *)tmp_dev->_1905_info.al_mac_addr,
				0,
				NULL,
				0,
				NULL,
				1,
				radio_policy,
				metric_policy_interval,
				1,
				metrics_policy,
				1,
				0,
				1,
				assoc_policy);
		else
			map_1905_Send_MAP_Policy_Request_Message (
				global->_1905_ctrl,
				(char *)tmp_dev->_1905_info.al_mac_addr,
				0,
				NULL,
				0,
				NULL,
				1,
				radio_policy,
				metric_policy_interval,
				1,
				metrics_policy,
				0,
				0,
				0,
				NULL);
	}
	if(metrics_policy) {
		os_free(metrics_policy);
	}
	if(radio_policy){
		os_free(radio_policy);
	}
	if (assoc_policy)
		os_free(assoc_policy);
}

void ch_planning_add_monitor_ch(
	struct own_1905_device * ctx,
	struct _1905_map_device *dev,
	struct radio_info_db *radio)
{
/* check list if channel is already present ,
if not then add new list item and start timer for this channel's monitor
check list if affected agents has dev alid match , if not then add */
	struct monitor_ch_info *ch = NULL;
	struct monitor_ch_info *new_ch_info = NULL;
	struct affected_agent_info *affected_agent = NULL;
	u8 new_ch = 1, new_agent = 1;
	if(!radio) {
		err("radio is NULL , can't add ERROR");
		return ;
	}
	if(ch_planning_search_monitor_ch(ctx,radio)) {
		new_ch = 0;
	}
	if(new_ch) {
	
		new_ch_info = os_zalloc(sizeof(struct monitor_ch_info));
		affected_agent = NULL;
		if (new_ch_info == NULL) {
			err("alloc memory fail");
			assert(0);
			return ;
		}
		new_ch_info->channel_num = radio->channel[0];
		SLIST_INSERT_HEAD(&(ctx->ch_planning_R2.first_monitor_ch),
			new_ch_info, next_monitor_ch);
		SLIST_INIT(&new_ch_info->first_affected_agent);
		err("......Start Monitoring Channel %d.....", radio->channel[0]);

		affected_agent = os_zalloc(sizeof(struct affected_agent_info));
		if (affected_agent == NULL) {
			err("alloc memory fail");
			os_free(new_ch_info);
			assert(0);
			return ;
		}
		affected_agent->affected_dev = dev;
		SLIST_INSERT_HEAD(&new_ch_info->first_affected_agent,
			affected_agent, next_affected_agent);

		/*start timer for  this channel's monitor*/
		debug("start moni timeout for channel %d ", radio->channel[0]);
		eloop_register_timeout(ctx->ch_planning_R2.ch_monitor_timeout, 0,
			channel_monitor_timeout, ctx, new_ch_info);

		ch_planning_update_all_dev_state((u8)CHPLAN_STATE_MONITOR,new_ch_info->channel_num,ctx);
		/*change policy for AP metric reporting for all devices on that channel*/
		ch_planning_update_ap_metric_policy(ctx, new_ch_info, 0);
		debug("update policy done");
	} else {
		affected_agent = NULL;
		new_agent = 1;
		SLIST_FOREACH(ch, &(ctx->ch_planning_R2.first_monitor_ch), next_monitor_ch) {
			if (ch->channel_num == radio->channel[0])
				break;
		}
		if (!ch) {
			err("monitor ch is null some error");
			return;
		}
		SLIST_FOREACH(affected_agent,
			&(ch->first_affected_agent), next_affected_agent) {
			if (os_memcmp(affected_agent->affected_dev->_1905_info.al_mac_addr,
				dev->_1905_info.al_mac_addr, ETH_ALEN) == 0) {
				new_agent = 0;
				break;
			}
		}
		if(new_agent) {
			affected_agent = os_zalloc(sizeof(struct affected_agent_info));
			if (!affected_agent) {
				err("failed to allocate memory for affected_agent");
				assert(0);
				return;
			}
			affected_agent->affected_dev = dev;
			SLIST_INSERT_HEAD(&ch->first_affected_agent,
				affected_agent, next_affected_agent);
			debug("add new agent");
		}
	}
}
void ch_planning_monitor_threshold_init(struct own_1905_device *ctx)
{
	struct Ch_threshold *thresh = NULL;

	thresh = &ctx->ch_planning_R2.ch_plan_thres[0];
	thresh->band = BAND_2G;
	if(thresh->ch_util_threshold == 0)
		thresh->ch_util_threshold =
			CH_PLAN_DEFAULT_CH_UTIL_TH_2G;
	if(thresh->edcca_threshold == 0)
		thresh->edcca_threshold =
			CH_PLAN_DEFAULT_EDCCA_TH_2G;
	if(thresh->obss_load_threshold == 0)
		thresh->obss_load_threshold =
			CH_PLAN_DEFAULT_OBSS_TH_2G;

	thresh = &ctx->ch_planning_R2.ch_plan_thres[1];
		thresh->band = BAND_5GL;
		if(thresh->ch_util_threshold == 0)
			thresh->ch_util_threshold =
				CH_PLAN_DEFAULT_CH_UTIL_TH_5G;
		if(thresh->edcca_threshold == 0)
			thresh->edcca_threshold =
				CH_PLAN_DEFAULT_EDCCA_TH_5G;
		if(thresh->obss_load_threshold == 0)
			thresh->obss_load_threshold =
				CH_PLAN_DEFAULT_OBSS_TH_5G;

	thresh = &ctx->ch_planning_R2.ch_plan_thres[2];
	thresh->band = BAND_5GH;
	if(thresh->ch_util_threshold == 0)
		thresh->ch_util_threshold =
			CH_PLAN_DEFAULT_CH_UTIL_TH_5G;
	if(thresh->edcca_threshold == 0)
		thresh->edcca_threshold =
			CH_PLAN_DEFAULT_EDCCA_TH_5G;
	if(thresh->obss_load_threshold == 0)
		thresh->obss_load_threshold =
			CH_PLAN_DEFAULT_OBSS_TH_5G;
}

void ch_planning_R2_init(struct own_1905_device * ctx)
{

	ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_IDLE;
	if(ctx->ch_planning_R2.min_score_inc == 0)
		ctx->ch_planning_R2.min_score_inc = MIN_SCORE_INCREMENT_DEFAULT;

	if(ctx->ch_planning_R2.ch_plan_metric_policy_interval == 0)
		ctx->ch_planning_R2.ch_plan_metric_policy_interval =
			DEFAULT_METRIC_REPORTING_INTERVAL;

	if(ctx->ch_planning_R2.ch_monitor_timeout == 0)
		ctx->ch_planning_R2.ch_monitor_timeout =
			CHANNEL_MONITOR_TIMEOUT;
	if(ctx->ch_planning_R2.ch_monitor_prohibit_wait_time == 0)
		ctx->ch_planning_R2.ch_monitor_prohibit_wait_time =
			CHANNEL_MONITOR_PROHIBIT_TIME;

	ch_planning_monitor_threshold_init(ctx);
	SLIST_INIT(&ctx->ch_planning_R2.first_monitor_ch);
	if (SLIST_EMPTY(&(ctx->ch_planning_R2.first_scan_list))) {
		SLIST_INIT(&ctx->ch_planning_R2.first_scan_list);
	}
	if (SLIST_EMPTY(&(ctx->ch_planning_R2.first_ch_score))) {
		SLIST_INIT(&ctx->ch_planning_R2.first_ch_score);
	}
	
	
}
void ch_planning_remove_radio_scan_result(
	struct radio_info_db *radio)
{
	struct scan_result_tlv *scan_result = NULL;
	struct nb_info *neigh = NULL;
	while (!SLIST_EMPTY(&(radio->first_scan_result))) {
		scan_result = SLIST_FIRST(&(radio->first_scan_result));
		while (!SLIST_EMPTY(&(scan_result->first_neighbor_info))) {
			neigh = SLIST_FIRST(&(scan_result->first_neighbor_info));
			SLIST_REMOVE_HEAD(&(scan_result->first_neighbor_info),
				next_neighbor_info);
			os_free(neigh);
		}
		SLIST_REMOVE_HEAD(&(radio->first_scan_result), next_scan_result);
		os_free(scan_result);
	}
}
void ch_planning_remove_all_dev_radio_scan_results(
	struct own_1905_device * ctx,
	struct radio_info_db *reset_radio)
{
	struct _1905_map_device *_1905_device = NULL;
	struct radio_info_db *radio = NULL;
	SLIST_FOREACH(_1905_device, &(ctx->_1905_dev_head), next_1905_device) {
		SLIST_FOREACH(radio, &(_1905_device->first_radio), next_radio) {
			if((reset_radio && (radio->band == reset_radio->band)) ||
				(reset_radio == NULL)) {
				err("Reset radio state");
				radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_IDLE;
				eloop_cancel_timeout(ch_planning_own_dev_get_metric_timeout, ctx, radio);
				//ch_planning_remove_radio_scan_result(radio);//sonal temp comment , don't want to delete scan results
				//SLIST_INIT(&radio->first_scan_result);//sonal temp comment , don't want to delete scan results
				os_memset(&radio->dev_ch_plan_info.dev_ch_monitor_info,
					0, sizeof(struct dev_ch_monitor));
				//break;
			}
		}
	}
}
u8 ch_planning_num_monitor_ch(
	struct own_1905_device *own_dev)
{
	struct monitor_ch_info *temp_ch_info;
	u8 num_monitor_ch = 0;
	SLIST_FOREACH(temp_ch_info,
		&own_dev->ch_planning_R2.first_monitor_ch, next_monitor_ch) {
		num_monitor_ch++;
	}
	return num_monitor_ch;
}

void ch_planning_restore_policy(
	struct own_1905_device * ctx,
	struct radio_info_db *reset_radio)
{
	struct radio_info_db *radio = reset_radio, *temp_radio = NULL;
	struct _1905_map_device *_1905_device = NULL;
	u8 num_monitor_ch = ch_planning_num_monitor_ch(ctx);
	SLIST_FOREACH(_1905_device, &(ctx->_1905_dev_head), next_1905_device) {
		if(_1905_device->device_role == DEVICE_ROLE_CONTROLLER) {
			if(radio) {
				/*cancel timer for own dev metric reporting from wapp */
				eloop_cancel_timeout(ch_planning_own_dev_get_metric_timeout, ctx, radio);
			} else {
				SLIST_FOREACH(temp_radio,&(_1905_device->first_radio),next_radio){
					/*cancel timer for own dev metric reporting from wapp */
					eloop_cancel_timeout(ch_planning_own_dev_get_metric_timeout, ctx, temp_radio);
				}
			}
		} else if(num_monitor_ch < 2){
			err("restore policy");
			/*Update policy to restore original setting after monitor timeout when scan is triggered*/
			steer_msg_update_policy_config(ctx->back_ptr, _1905_device);
		}
	}
}
void ch_planning_R2_reset(
	struct own_1905_device * ctx,
	struct radio_info_db *reset_radio)
{
/*if reset radio is NULL , then reset for all radios*/
	err(" ");
	ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_IDLE;
	//ch_planning_remove_agg_score_table(ctx);// make persistent
	ch_planning_restore_policy(ctx, reset_radio);
	ch_planning_remove_scan_list(ctx);
	ch_planning_remove_ch_monitor_info_list(ctx, reset_radio);
	ch_planning_remove_all_dev_radio_scan_results(ctx,reset_radio);
	if(!reset_radio){
		find_and_remove_pending_task(ctx,TASK_CHANNEL_PLANNING_TRIGGER);
	}
	if(ctx->ch_planning_R2.CAC_on_channel)
		eloop_cancel_timeout(channel_cac_timeout2,
			ctx, &ctx->ch_planning_R2.CAC_on_channel);
	ctx->ch_planning_R2.CAC_on_channel = 0;
	ctx->ch_planning_R2.force_trigger = 0;
	ch_planning_R2_init(ctx);
}

void ch_planning_update_data(
	struct bss_info_db *bss,
	struct radio_info_db *radio,
	u8 cu_tlv_update)
{
	struct dev_ch_monitor *dev_monitor_info =
		&radio->dev_ch_plan_info.dev_ch_monitor_info;
	debug("radio channel %d", radio->channel[0]);
	if(bss) {
		dev_monitor_info->avg_cu_monitor =
			(dev_monitor_info->avg_cu_monitor +
			bss->ch_util);
		dev_monitor_info->count_cu_util++;
		debug("add ch %d bss->ch_util %d, count %d",radio->channel[0], bss->ch_util, dev_monitor_info->count_cu_util);
	} else {
		if(cu_tlv_update == 1) {
			dev_monitor_info->avg_edcca =
				(dev_monitor_info->avg_edcca +
				radio->cu_distribution.edcca_airtime);
			dev_monitor_info->count_edcca_cu_tlv++;
			debug("add edcca %d, count %d", radio->cu_distribution.edcca_airtime, dev_monitor_info->count_edcca_cu_tlv);
		} else {
			dev_monitor_info->avg_obss_load =
				(dev_monitor_info->avg_obss_load +
				radio->radio_metrics.cu_other);

			dev_monitor_info->avg_myTxAirtime =
				(dev_monitor_info->avg_myTxAirtime +
				radio->radio_metrics.cu_tx);

			dev_monitor_info->avg_myRxAirtime =
				(dev_monitor_info->avg_myRxAirtime +
				radio->radio_metrics.cu_rx);
			dev_monitor_info->count_radio_metric++;
			debug("add cu_other %d, count %d", radio->radio_metrics.cu_other, dev_monitor_info->count_radio_metric);
		}
	}
	//dump_ch_planning_update_data(radio);
}
u8 ch_planning_compare_monitor_thresh(
	struct own_1905_device * ctx,
	struct bss_info_db *bss,
	struct radio_info_db *radio,
	u8 cu_tlv_update)
{
	u8 need_to_monitor = 0;
	u8 radio_num = 0;
	u32  util_per = 0;
	if(bss) {
		/*check based on channel util threshold*/
		radio_num = find_radio_num(ctx,bss->radio);
		debug("ch_util %d",bss->ch_util);
		util_per = (u32)(bss->ch_util) * 100;/*calculate percentage*/
		util_per = util_per / 255;
		if(util_per >
			ctx->ch_planning_R2.ch_plan_thres[radio_num].ch_util_threshold)
		{
			debug("CH monitoring on ch %d, bss->ch_util %d",bss->radio->channel[0], bss->ch_util);
			need_to_monitor = 1;
		}
		radio = bss->radio;
		debug("BSSID "MACSTR"bss radio ch %d",MAC2STR(bss->bssid), radio->channel[0]);
	} else if(radio && (cu_tlv_update == 1)) {
			/*check based on EDCCA thresh*/
			radio_num = find_radio_num(ctx, radio);
			debug("edcca_airtime %d",  radio->cu_distribution.edcca_airtime);
			if(radio->cu_distribution.edcca_airtime >
				ctx->ch_planning_R2.ch_plan_thres[radio_num].edcca_threshold)
			{
				debug("moni trigger due to edcca bad %d", radio->cu_distribution.edcca_airtime);
				need_to_monitor = 1;
			}
			debug("EDCCA radio ch %d", radio->channel[0]);
	} else if(radio && (cu_tlv_update == 0)) {
		/*check based on OBSS thresh*/
		radio_num = find_radio_num(ctx, radio);
		debug("OBSS %d", radio->radio_metrics.cu_other);
		if(radio->radio_metrics.cu_other >
			ctx->ch_planning_R2.ch_plan_thres[radio_num].obss_load_threshold)
		{
			debug("moni trigger due to obss bad %d", radio->radio_metrics.cu_other);
			need_to_monitor = 1;
		}
		debug("OBSS radio ch %d", radio->channel[0]);
	}
	return need_to_monitor;
}

u8 ch_planning_is_MAP_net_idle(
	struct own_1905_device * ctx)
{
	u8 is_idle = 0;
	struct os_time now;
	struct ch_planning_cb *p_ch_planning = &ctx->ch_planning;

	if (p_ch_planning->last_high_byte_count_ts.sec == 0) {
		os_get_time(&p_ch_planning->last_high_byte_count_ts);
	}
	os_get_time(&now);
	if (now.sec - p_ch_planning->last_high_byte_count_ts.sec >
		p_ch_planning->ChPlanningIdleTime) {
		is_idle = 1;
	} else {
		is_idle = 0;
	}
	debug("is_idle %d", is_idle);
return is_idle;
}

u8 ch_planning_is_monitor_prohibit_over(
	struct own_1905_device * ctx)
{
	struct os_time now;
	os_get_time(&now);
	if(ctx->ch_planning_R2.ch_monitor_start_ts.sec == 0) {
		err(" initialize monitor timer");
		os_get_time(&ctx->ch_planning_R2.ch_monitor_start_ts);
	}
	if((now.sec > ctx->ch_planning_R2.ch_monitor_start_ts.sec +
			ctx->ch_planning_R2.ch_monitor_prohibit_wait_time)) {
		err("start monitor time update ");
		os_get_time(&ctx->ch_planning_R2.ch_monitor_start_ts);
		return 1;
	}
	return 0;
}
u8 ch_planning_need_monitor(
	struct own_1905_device * ctx,
	struct bss_info_db *bss,
	struct radio_info_db *radio,
	u8 cu_tlv_update)
{
/*	Need to monitor will be true only when 
	the monitor thresholds exceed 
	AND 
	Map network is in IDLE state
	AND
	MONITOR prohibit time has elapsed*/
	u8 need_to_monitor_1 = 0, need_to_monitor_2 = 0, need_to_monitor_3 = 0;

	/*Compare with Thresholds*/
	need_to_monitor_1 = ch_planning_compare_monitor_thresh(ctx,bss,radio,cu_tlv_update);
	if(!need_to_monitor_1)
		return 0;
	/*Check Network is in IDLE state*/
	need_to_monitor_2 = ch_planning_is_MAP_net_idle(ctx);
	if(!need_to_monitor_2)
		return 0;
	/*Check Monitoring prohibit timer has expired*/
	need_to_monitor_3 = ch_planning_is_monitor_prohibit_over(ctx);
	err(" threshmon %d, MAP idle mon %d, prohibit mon %d", need_to_monitor_1, need_to_monitor_2, need_to_monitor_3);
	if(!need_to_monitor_3)
		return 0;
	return 1;
}
void ch_planning_handle_metric_report(
	struct own_1905_device * ctx,
	struct _1905_map_device *dev,
	struct bss_info_db *bss,
	struct radio_info_db *radio,
	u8 cu_tlv_update,
	u8 force_ch_planning_monitor)
{
	u8 need_to_monitor = 0;
	debug("dev ALMAC"MACSTR" ",MAC2STR(dev->_1905_info.al_mac_addr));
	if(force_ch_planning_monitor)
		need_to_monitor = 1;
	else
		need_to_monitor = ch_planning_need_monitor(ctx,bss,radio,cu_tlv_update);
	if(!radio){
		radio = bss->radio;
	}
	if(need_to_monitor) {
		if(ctx->ch_planning_R2.ch_plan_state < CHPLAN_STATE_MONITOR)
			ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_MONITOR;
		ch_planning_add_monitor_ch(ctx, dev, radio);
	}
	
	if(ch_planning_search_monitor_ch(ctx, radio)) {
		/*need to keep storing channel plan data per device*/
		ch_planning_update_data(bss, radio, cu_tlv_update);
	}
}
Boolean ch_planning_all_dev_select_done(
	struct own_1905_device *own_dev,
	struct radio_info_db *radio)
{
	struct radio_info_db *temp_radio = NULL;
	struct _1905_map_device *_1905_device = NULL;
	if(!radio) {
		err("radio is NULL");
		return FALSE;
	}
	SLIST_FOREACH(_1905_device, &(own_dev->_1905_dev_head), next_1905_device) {
		err("ALMAC %02x:%02x:%02x:%02x:%02x:%02x\n", MAC2STR(_1905_device->_1905_info.al_mac_addr));
		if (!(_1905_device->in_network))
			continue;
		temp_radio = topo_srv_get_radio_by_band(_1905_device,radio->channel[0]);
		if(temp_radio &&
			temp_radio->dev_ch_plan_info.dev_ch_plan_state != CHPLAN_STATE_IDLE) {
			err("FALSE");
			return FALSE;
		}
	}
	err("TRUE");
	return TRUE;
}
void ch_planning_update_all_dev_state(
	u8 state,
	u8 channel,
	struct own_1905_device *own_dev)
{
	struct radio_info_db *temp_radio = NULL;
	struct _1905_map_device *_1905_device = NULL;
	err("channel %d , state %d", channel, state);
	SLIST_FOREACH(_1905_device, &(own_dev->_1905_dev_head), next_1905_device) {
		temp_radio = topo_srv_get_radio_by_band(_1905_device,channel);
		if(temp_radio) {
			err(" ");
			temp_radio->dev_ch_plan_info.dev_ch_plan_state = state;
		}
	}
}
void ch_planning_handle_ch_selection_rsp(
	struct own_1905_device *own_dev,struct _1905_map_device *peer_1905)
{
	struct radio_info_db *radio = NULL;
	struct mapd_radio_info *own_radio = NULL;
	struct os_reltime rem_time = {0};
	u8 count = 0;
	err(" ");
	SLIST_FOREACH(radio,&(peer_1905->first_radio),next_radio)
	{
		if(radio->dev_ch_plan_info.dev_ch_plan_state == CHPLAN_STATE_CH_CHANGE_TRIGGERED)
		{
			err("ch %d dev_ch_plan_state %d",radio->channel[0],radio->dev_ch_plan_info.dev_ch_plan_state);
			radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_IDLE;
			break;
		}
	}
	err(" ");
	if(ch_planning_all_dev_select_done(own_dev,radio))
	{
		struct radio_info_db *temp_radio = NULL;
		struct _1905_map_device *_1905_device = topo_srv_get_1905_device(own_dev,NULL);
		temp_radio = topo_srv_get_radio_by_band(_1905_device,radio->channel[0]);
		if(temp_radio)
			own_radio = mapd_get_radio_from_channel(own_dev->back_ptr,temp_radio->channel[0]);
		if(own_radio && own_radio->bootup_run == 2){
			err("ownradio bootup run complete for ch %d", radio->channel[0]);
			own_radio->bootup_run = 1;
		}
		dump_ch_planning_info(own_dev, 0);
		err("ch change done on all devices");
		count = get_net_opt_dev_count((struct mapd_global *)own_dev->back_ptr);
		if (count > 1) {
			eloop_get_remaining_timeout(&rem_time,trigger_net_opt,own_dev,NULL);
			if (rem_time.sec == 0) {
				eloop_register_timeout(own_dev->network_optimization.wait_time,
						0, trigger_net_opt, own_dev, NULL);
			}
		}
		ch_planning_R2_reset(own_dev,radio);
		handle_task_completion(own_dev);
	}
}

void dump_ch_planning_info(
	struct own_1905_device *own_dev,
	u8 cmd_param)
{
	struct _1905_map_device *peer_1905 = NULL;
	struct radio_info_db *radio = NULL;
	struct monitor_ch_info *ch_info = NULL;
	u8 i;
	err("global ch_plan_enable %d", own_dev->ch_planning_R2.ch_plan_enable);
	err("global ch_plan_state %d", own_dev->ch_planning_R2.ch_plan_state);
	err("force trigger %d",own_dev->ch_planning_R2.force_trigger);
	debug("global Threshold information");
	for (i = 0; i< 2; i++)
	{
		debug("global ch_util_threshold for band %d is %d", i, own_dev->ch_planning_R2.ch_plan_thres[i].ch_util_threshold);
		debug("global edcca_threshold for band %d is %d", i, own_dev->ch_planning_R2.ch_plan_thres[i].edcca_threshold);
		debug("global obss_load_threshold for band %d is %d", i, own_dev->ch_planning_R2.ch_plan_thres[i].obss_load_threshold);
	}
	uint8_t radio_idx = 0;
	for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++) {
		struct mapd_radio_info *radio_info = &own_dev->dev_radio_info[radio_idx];
		if (radio_info->radio_idx == (uint8_t)-1)
			continue;
		err("ownradio ch %d radio_info->bootup_run state %d",radio_info->channel, radio_info->bootup_run);
	}

	err("************monitor info*************");
	SLIST_FOREACH(ch_info, &own_dev->ch_planning_R2.first_monitor_ch, next_monitor_ch) {
		err("chinfo channel_num %d", ch_info->channel_num);
		err("chinfo scan trigger_status %d", ch_info->trigger_status);
	}
	err("*********CHScoreTable****************");
	struct score_info *cumulative_score = NULL;
	SLIST_FOREACH(cumulative_score,
		&own_dev->ch_planning_R2.first_ch_score,next_ch_score) {
		err("CH %d , avg score %d, rank %d", cumulative_score->channel, cumulative_score->avg_score, cumulative_score->ch_rank);
	}
	if(own_dev->ch_planning_R2.ch_plan_enable_bw) {
		err("*********GroupScoreTable****************");
		struct grp_score_info *grp_score = NULL;
		SLIST_FOREACH(grp_score,
			&own_dev->ch_planning_R2.first_grp_score,next_grp_score) {
			err("CH grp [%d, %d, %d, %d, %d, %d, %d, %d], avg score %d, rank %d",
				grp_score->grp_channel_list[0],grp_score->grp_channel_list[1],
				grp_score->grp_channel_list[2],grp_score->grp_channel_list[3],
				grp_score->grp_channel_list[4],grp_score->grp_channel_list[5],
				grp_score->grp_channel_list[6],grp_score->grp_channel_list[7],
				grp_score->grp_total_avg_score,grp_score->grp_rank);
		}
		
	} 
	err("**************1905 dev info************");
	SLIST_FOREACH(peer_1905, &(own_dev->_1905_dev_head), next_1905_device) {
		err("DEV Role %d, DEV ALMAC"MACSTR"",peer_1905->device_role, MAC2STR(peer_1905->_1905_info.al_mac_addr));
		SLIST_FOREACH(radio, &(peer_1905->first_radio), next_radio) {
			err("channel %d, state %d", radio->channel[0],
				radio->dev_ch_plan_info.dev_ch_plan_state);
		}
	}

	if(cmd_param == 2) {
		//want to dump all dev ch prefer info 
		err("**************1905 dev CH PREFER info************");
		struct radio_ch_prefer *ch_prefer = NULL;
		struct prefer_info_db *prefer_db = NULL;
		u8 bw = 0, i = 0;
		SLIST_FOREACH(peer_1905, &(own_dev->_1905_dev_head), next_1905_device) {
			err("DEV Role %d, DEV ALMAC"MACSTR"",
				peer_1905->device_role, MAC2STR(peer_1905->_1905_info.al_mac_addr));
			SLIST_FOREACH(radio, &(peer_1905->first_radio), next_radio) {
				err("current channel %d, opclass %d", radio->channel[0], radio->operating_class);
				ch_prefer = &radio->chan_preferance;
				SLIST_FOREACH(prefer_db, &ch_prefer->prefer_info_head, prefer_info_entry) {
					bw = chan_mon_get_bw_from_op_class(prefer_db->op_class);
					err("opclass %d,bw %d pref %d", prefer_db->op_class,bw, prefer_db->perference);
					for(i=0; i<prefer_db->ch_num;i++) {
						err("ch %d",prefer_db->ch_list[i]);
					}
				}				
			}
		}
	}
	if(cmd_param != 1)
		return;
	struct scan_result_tlv *res = NULL;
	err("*************Scan results dump***********");
	SLIST_FOREACH(peer_1905, &(own_dev->_1905_dev_head), next_1905_device) {
		err("^^^^^^DEV ALMAC^^^^^"MACSTR"",MAC2STR(peer_1905->_1905_info.al_mac_addr));
		SLIST_FOREACH(radio, &(peer_1905->first_radio), next_radio) {
			err("------radio channel %d--------", radio->channel[0]);
			SLIST_FOREACH(res, &(radio->first_scan_result), next_scan_result) {
				err("Scan results for channel %d", res->channel);
				err("Util %d, NBnum %d score %d", res->utilization, res->neighbor_num, res->ch_score);
				debug("EDCCA %d, ch %d",res->cu_distribution.edcca_airtime, res->cu_distribution.ch_num);
			}
		}
	}
	
}

void ch_planning_R2_force_trigger(
	struct mapd_global *global,
	u8 channel)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_map_device *_1905_dev = topo_srv_get_1905_device(&global->dev, NULL);
	struct radio_info_db *radio = NULL;
	struct monitor_ch_info *new_ch_info = NULL;
	global->dev.ch_planning_R2.ch_plan_enable = TRUE;
	ch_planning_R2_reset(&global->dev,NULL);
	radio = topo_srv_get_radio_by_band(_1905_dev, channel);

	/*Add channel to monitor list*/
	new_ch_info = os_zalloc(sizeof(struct monitor_ch_info));
	if (new_ch_info == NULL) {
		err("alloc memory fail");
		assert(0);
		return ;
	}
	new_ch_info->channel_num = radio->channel[0];
	SLIST_INSERT_HEAD(&(ctx->ch_planning_R2.first_monitor_ch),
		new_ch_info, next_monitor_ch);
	SLIST_INIT(&new_ch_info->first_affected_agent);

	/*set force trigger flag*/
	ctx->ch_planning_R2.force_trigger = 1;
	ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_MONITOR;
	ch_planning_update_all_dev_state((u8)CHPLAN_STATE_MONITOR,new_ch_info->channel_num,ctx);
	/*Go for scan start*/
	eloop_register_timeout(0, 0,
			channel_monitor_timeout, ctx, new_ch_info);

}
void ch_planning_R2_bootup_handling(
	struct own_1905_device *ctx)
{
	uint8_t radio_idx = 0;
	for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++) {
		struct mapd_radio_info *radio_info = &ctx->dev_radio_info[radio_idx];
		if (radio_info->radio_idx == (uint8_t)-1)
			continue;
		if(radio_info->bootup_run != 1 && radio_info->bootup_run != 3 &&
			ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_IDLE &&
			ctx->user_triggered_scan == FALSE &&
			ctx->network_optimization.network_opt_state == NETOPT_STATE_IDLE) {
			err("At bootup force trigger ch planning on ch %d", radio_info->channel);
			radio_info->bootup_run = 2;
			ch_planning_R2_force_trigger(ctx->back_ptr, radio_info->channel);
			if(ctx->ch_planning_R2.ch_monitor_start_ts.sec == 0) {
				err(" initialize prohibit monitor timer");
				os_get_time(&ctx->ch_planning_R2.ch_monitor_start_ts);
			}
		}
	}
}

int check_ongoing_CAC(struct own_1905_device *ctx, struct radio_info_db *radio)
{
	struct _1905_map_device *dev = NULL;
	struct cac_cap_db *cap = NULL;
	u8 non_dedicated = 0;

	if(ctx->ch_planning_R2.ch_plan_enable == FALSE && radio->band == BAND_2G) {
		return 0;
	}

	dev = topo_srv_get_1905_device(ctx, NULL);

	SLIST_FOREACH(cap, &radio->cac_cap.cac_capab_head, cac_cap_entry) {
		if(cap->cac_mode == CONTINUOUS) {
			non_dedicated = 1;
			break;
		}
	}

	debug("radio_channel: %u, status:%u", radio->cac_comp_status.channel,
			radio->cac_comp_status.cac_status);
	debug("cac_ongoing_channel: %u, cac_ongoing: %u", ctx->ch_planning_R2.CAC_on_channel,
			ctx->ch_planning_R2.cac_ongoing);
	if ((non_dedicated == 1) && (ctx->ch_planning_R2.cac_ongoing == 1) &&
			(radio == topo_srv_get_radio_by_band(dev, ctx->ch_planning_R2.CAC_on_channel))) {
		return 1;
	}

	return 0;
}
void ch_planning_scan_restart_due_to_failure(struct mapd_global *global)
{
	struct own_1905_device *own_dev = &global->dev;
	struct _1905_map_device *dev = topo_srv_get_1905_device(own_dev,NULL);
	struct monitor_ch_info *ch_info = NULL;
	uint8_t radio_idx = 0;	
	if ((global->dev.user_triggered_scan == FALSE)) {
		for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++) {
			struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[radio_idx];
			if (radio_info->radio_idx == (uint8_t)-1)
				continue;
			if (radio_info->bootup_run != 1)
				radio_info->bootup_run = 3; //Special condition to run scan after 150 seconds on particular radio
			SLIST_FOREACH(ch_info,
				&global->dev.ch_planning_R2.first_monitor_ch, next_monitor_ch) {
				if((ch_info->trigger_status == TRIGGER_TRUE)
					&& (get_band(dev, ch_info->channel_num) == get_band(dev, radio_info->channel))) {
					radio_info->bootup_run = 3;
					break;
				}
			}
		}
	} else {
		global->dev.user_triggered_scan = FALSE;
		err ("User triggered scan while agent is not ready. User Retrigger command after 150 seconds")
	}
	ch_planning_R2_reset(&global->dev, NULL);
	eloop_register_timeout(CH_SCAN_RETRIGGER_TIMEOUT, 0, ch_planning_R2_bootup_handling_restart, (void *)&global->dev, NULL);
	return;
}

#endif/*MAP_R2*/



