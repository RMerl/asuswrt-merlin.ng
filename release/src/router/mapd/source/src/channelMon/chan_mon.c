/*
***************************************************************************
*  Mediatek Inc.
* 4F, No. 2 Technology 5th Rd.
* Science-based Industrial Park
* Hsin-chu, Taiwan, R.O.C.
*
* (c) Copyright 2002-2011, Mediatek, Inc.
*
* All rights reserved. Mediatek's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of Ralink Tech. Any attemp
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of Mediatek, Inc. is obtained.
***************************************************************************

                Module Name:
                Channel Monitor

                Abstract:
                Track per radio(channel) info

                Revision History:
                Who         When          What
                --------    ----------    -----------------------------------------
                Neelansh.M   2018/05/02		First implementation of the channel monitor Module
*/
#include "includes.h"
#include "os.h"
#include "common.h"
#include <sys/un.h>
#include "steer_fsm.h"
#include "list.h"
#include "client_db.h"
#include "mapd_i.h"
#ifdef SUPPORT_MULTI_AP
#include "./../1905_local_lib/data_def.h"
#endif
#include "chan_mon.h"
#ifdef SUPPORT_MULTI_AP
#include "topologySrv.h"
#include "tlv_parsor.h"
#endif
#include "steer_action.h"
#include <assert.h>
#include "ap_roam_algo.h"
#ifdef SUPPORT_MULTI_AP
#include "1905_map_interface.h"
#include "network_optimization.h"
#endif
#ifdef CENT_STR
#include "wapp_if.h"
#endif
extern u8 bcast_mac[];
#define PRINT_MAC(a) a[0],a[1],a[2],a[3],a[4],a[5]

const static struct oper_class_map global_op_class[] = {
    {81, 1, 13, 1, BW20},
    {82, 14, 14, 1, BW20},
    {83, 1, 9, 1 ,BW40PLUS},
    {84, 5, 13, 1, BW40MINUS},
    {115, 36, 48, 4, BW20},
    {116, 36, 44, 8, BW40PLUS},
    {117, 40, 48, 8, BW40MINUS},
    {118, 52, 64, 4, BW20},
    {119, 52, 60, 8, BW40PLUS},
    {120, 56, 64, 8, BW40MINUS},
    {121, 100, 140, 4, BW20},
    {122, 100, 132, 8, BW40PLUS},
    {123, 104, 136, 8, BW40MINUS},
    {124, 149, 161, 4, BW20},
    {125, 149, 169, 4, BW20},
    {126, 149, 157, 8, BW40PLUS},
    {127, 153, 161, 8, BW40MINUS},
    {128, 36, 161, 4, BW80},
    {129, 50, 114, 16, BW160},
    {130, 36, 161, 4, BW80P80},
    {180, 1, 4, 1, BW2160},
    {0, 0, 0, 0, BW20}
};
#ifdef SUPPORT_MULTI_AP
Boolean chan_mon_trigger_round_robin_steer_req(struct mapd_global *);
void cli_mon_handle_cli_cap_update_rsp(struct mapd_global*pGlobal_dev, 
													struct sta_info_rsp_tlv *sta_info_rsp);
void cli_mon_handle_cli_cap_update_req(struct mapd_global*pGlobal_dev,
												struct sta_info_req_tlv *sta_info_req,
												struct _1905_map_device* _1905_device);
#endif

/**
 * @brief : set the channel utilization for a radio interface, and check for pre
 * assoc steering if there is a transition across the overload threshold
 *
 * @param global: global device pointer
 * @param radio_idx: identifier of the radio interface for which utlization is to be
 * set
 * @param ch_util: the vlaue of the channel utilization 
 */
void chan_mon_set_util(struct mapd_global *global, u8 radio_idx,
				u8 ch_util)
{
	struct mapd_radio_info *ra_info = &global->dev.dev_radio_info[radio_idx];
	uint8_t ol_th = chan_mon_get_ol_th(global, ra_info->radio_idx);

	if ((ra_info->ch_util < ol_th) && (ch_util >= ol_th)) {
		mapd_printf(MSG_INFO, "%s: Channel=%d is OL now", __func__,
						ra_info->channel);
		ra_info->ch_util = ch_util;
#ifdef CENT_STR
		if(!global->dev.cent_str_en)
#endif		
		ap_roam_algo_chk_preassoc_str(global, NULL);
	} else if ((ra_info->ch_util >= ol_th && ch_util < ol_th)) {
		mapd_printf(MSG_INFO, "%s: Channel=%d is NOL now", __func__,
						ra_info->channel);
		ra_info->ch_util = ch_util;
#ifdef CENT_STR
		if(!global->dev.cent_str_en)
#endif		
		ap_roam_algo_chk_preassoc_str(global, NULL);
	}
	ra_info->ch_util = ch_util;
}

u8 chan_mon_get_util(struct mapd_global *global, u8 radio_idx)
{
    struct mapd_radio_info *ra_info = &global->dev.dev_radio_info[radio_idx];
    return ra_info->ch_util;
}

/**
 * @brief : Return the overload threshold for a radio interface based on band
 * (possibly other conditions later)
 *
 * @param global: global device pointer
 * @param radio_idx: identifier of the radio interface for which threshold is
 * required
 *
 * @return : the threshold value as integer
 */
u8 chan_mon_get_ol_th(struct mapd_global *global, u8 radio_idx)
{
    struct mapd_radio_info *ra_info =  &global->dev.dev_radio_info[radio_idx];

    if (ra_info->channel >= 1 && ra_info->channel <= 14)
        return global->dev.cli_steer_params.CUOverloadTh_2G;
    else if(ra_info->channel >=36 && ra_info->channel <= 64)
        return global->dev.cli_steer_params.CUOverloadTh_5G_L;
    else
        return global->dev.cli_steer_params.CUOverloadTh_5G_H;
}
#ifdef SUPPORT_MULTI_AP
/**
* @brief create a message to send steering request with request mode as opportunity
*
* @param own_device pointer to global structure
* @param map_bss bss to which steering opportunity is given
* @param steer_req_msg steering req message formed by this function
* @param bss_info bss_info formed (not used.)
*/
void chan_mon_create_steer_req_opp(struct own_1905_device *own_device,struct bss_info_db *map_bss,
		struct lib_steer_request **steer_req_msg,
		struct map_lib_target_bssid_info **bss_info)
{
	struct lib_steer_request *tmp_steer_req = NULL;
	struct map_lib_target_bssid_info *tmp_bss_info = NULL;

	tmp_steer_req = os_malloc(sizeof (struct lib_steer_request));
	tmp_bss_info = os_malloc(sizeof (struct map_lib_target_bssid_info));

	
	if(tmp_steer_req == NULL || tmp_bss_info == NULL) {
		if (tmp_steer_req)
			os_free(tmp_steer_req);

		if (tmp_bss_info)
			os_free(tmp_bss_info);

		mapd_printf(MSG_ERROR,"%s: Cannot allocate memory\n", __func__);
		return;
	}
	
	os_memset(tmp_steer_req,0 ,(sizeof (struct lib_steer_request)));
	os_memset(tmp_bss_info,0 , (sizeof (struct map_lib_target_bssid_info)));

	*steer_req_msg = tmp_steer_req;
	*bss_info = tmp_bss_info;

	os_memcpy(tmp_steer_req->bssid, map_bss->bssid,ETH_ALEN);
	tmp_steer_req->mode = 0; // Opportunity
	tmp_steer_req->window = own_device->controller_context.rr_control.opp_window;
	tmp_steer_req->timer = 0;//;own_device->controller_context.rr_control.btm_timer;
	tmp_steer_req->disassoc_imminent = 0;
	tmp_steer_req->abridged = 0;
	tmp_steer_req->sta_cnt = 0;

	tmp_bss_info->tbss_cnt = 0;
}

/**
* @brief check if steering was triggered on the device to whome steering opportunity was given.
*
* @param own_device pointer to global structure
*
* @return TRUE if steering was triggered, else FALSE
*/
Boolean chan_mon_check_steering_triggered(struct own_1905_device *own_device)
{
	// check if steering took place when opportunity was given to this bss.
	if (own_device->controller_context.rr_control.p_current_1905_rr->p_current_bss_rr != NULL)
		return own_device->controller_context.rr_control.p_current_1905_rr->p_current_bss_rr->b_steer_triggered;
	return FALSE;
}

/**
* @brief initialize round robin steer request trigger information.
*
* @param pGlobal_dev pointer to global structure
*/
void chan_mon_update_rr_ctrl_trigger_info(struct mapd_global *pGlobal_dev)
{
	struct own_1905_device *own_device = &pGlobal_dev->dev;
	struct rr_steer_controller *rr_control = &own_device->controller_context.rr_control;
	rr_control->can_trigger_steer_req = TRUE;
	rr_control->rr_state = NONE;
	rr_control->opp_window = own_device->cli_steer_params.CUAvgPeriod;
	rr_control->silent_window = own_device->cli_steer_params.CUAvgPeriod;
}

/**
* @brief This is the core funciton which decides when to send round robin steering request and to whom. It maintiains the states
* NONE,STEER_REQ_TRIGGERED, SILENT_PERIOD. It maintains the 1905 device and the BSS to which steering request was triggered
*
* @param pGlobal_dev : pointer to the global structure
*/
void chan_mon_rr_trigger_handler(struct mapd_global *pGlobal_dev)
{
	struct own_1905_device *own_device = &pGlobal_dev->dev;
	struct rr_steer_controller *rr_control = &own_device->controller_context.rr_control;
	Boolean ret;
	struct os_time now;
	//struct mapd_bss *curr_own_bss = NULL;
	struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(&pGlobal_dev->dev, NULL);

	os_get_time(&now);

	if(!(own_1905_node->device_role == DEVICE_ROLE_CONTROLLER))
		return;

	if(rr_control->can_trigger_steer_req == FALSE) {
			if(rr_control->rr_state == STEER_REQ_TRIGGERED) {
				// check for steer req timeout, 
				// if time expired, then can trigger steer req:
				/* timer expiry will be valid only when opportunity is with agents and not controller (own device).
				In this case, opportunity will be forfieted only by steer complete.*/
				if(rr_control->p_current_1905_rr != own_1905_node && rr_control->p_current_1905_rr && !rr_control->p_current_1905_rr->in_network) {
					rr_control->rr_state = NONE;
					rr_control->can_trigger_steer_req = TRUE;
					rr_control->rr_steer_req_timestamp.sec = 0;
					rr_control->rr_steer_req_timestamp.usec = 0;
					rr_control->p_current_1905_rr->p_current_bss_rr = NULL;
				}
				else if (rr_control->p_current_1905_rr != own_1905_node && (now.sec > (rr_control->rr_steer_req_timestamp.sec + rr_control->opp_window))) { // check for cyclic timer

					debug("steer_opp window expired... Move to next\n");
					if(chan_mon_check_steering_triggered(own_device) == FALSE){
						// no steering took place, move to next bss
						rr_control->rr_state = NONE;
						rr_control->can_trigger_steer_req = TRUE;
						rr_control->rr_steer_req_timestamp.sec = 0;
						rr_control->rr_steer_req_timestamp.usec = 0;
					}
					else{
						rr_control->rr_state = SILENT_PERIOD;
						rr_control->rr_silent_period_timestamp.sec= now.sec;
					}
				}
			}
			else if (rr_control->rr_state == SILENT_PERIOD) {
				//check for silent period timeout
				//if time expired, then can trigger steer req
				
				//printf("Raghav: SteerTriggered.Silent %d: %d: %d\n", now.sec,
					//rr_control->rr_silent_period_timestamp.sec,rr_control->silent_window );
				if ((now.sec - rr_control->rr_silent_period_timestamp.sec) > rr_control->silent_window) { // check for cyclic timer
					rr_control->rr_state = NONE;
					rr_control->rr_silent_period_timestamp.sec = 0;
					rr_control->rr_silent_period_timestamp.usec = 0;
					rr_control->can_trigger_steer_req = TRUE;
				}
			}
	}

	if(rr_control->can_trigger_steer_req == TRUE) {
		ret = chan_mon_trigger_round_robin_steer_req(pGlobal_dev);
		if(ret == TRUE) {
			rr_control->can_trigger_steer_req = FALSE;
			rr_control->rr_state = STEER_REQ_TRIGGERED;
			rr_control->rr_steer_req_timestamp.sec = now.sec;
			rr_control->p_current_1905_rr->p_current_bss_rr->b_steer_triggered = FALSE;
		}
		else
		{
			err(" Trigger steer req failed\n");
			return;
		}
	}

}

/**
* @brief when client assoc control message is received, controller checks whether this message is from the bss to which steer request message was
*	sent. if it is from the same bss, then mark that steering took place. Based on this info, controller needs to start the silent period.
*
* @param global pointer to the global structure
* @param buf pointer to the client assoc control message.
* @param len : length of the message.
*/
void chan_mon_check_steer_triggered(struct mapd_global *global, u8 *buf, u8 len)
{
	struct _1905_map_device *_1905_device = topo_srv_get_1905_device(&global->dev,buf);
	struct _1905_map_device *own_1905_device = topo_srv_get_next_1905_device(&global->dev,NULL);
	
	if(_1905_device == NULL) {
		mapd_printf(MSG_ERROR,"Device not present in topo db\n");
		return;
	}

	if(own_1905_device->device_role == DEVICE_ROLE_CONTROLLER 
		&& global->dev.controller_context.rr_control.p_current_1905_rr == _1905_device
		&& global->dev.controller_context.rr_control.p_current_1905_rr->p_current_bss_rr != NULL)
		_1905_device->p_current_bss_rr->b_steer_triggered = TRUE;
}
/**
* @brief handle steer complete message from a 1905 device. based on whether steering took place on this device, controller should start silent period or 
* give opporturnity to another device.
*
* @param own_device pointer to the global struct
* @param _1905_device 190 device from which steering complete message is received.
*/
void chan_mon_handle_steer_complete(struct own_1905_device *own_device, struct _1905_map_device *_1905_device)
{
	struct rr_steer_controller *rr_control = &own_device->controller_context.rr_control;
	struct os_time now;

	if(rr_control->rr_state != STEER_REQ_TRIGGERED) {
		mapd_printf(MSG_ERROR,"steer complete received in wrong state... ignore");
		return;
	}
	os_get_time(&now);

	if(rr_control->p_current_1905_rr != _1905_device) {
		mapd_printf(MSG_ERROR,"steer complete received for wrong device... ignore");
		return;
	}

	if(chan_mon_check_steering_triggered(own_device) == FALSE){// steering did not take place at this bss, move to next
		rr_control->rr_state = NONE;
		rr_control->can_trigger_steer_req = TRUE;
		rr_control->rr_steer_req_timestamp.sec = 0;
		rr_control->rr_steer_req_timestamp.usec = 0;
	}
	else {
		rr_control->rr_state = SILENT_PERIOD;
		rr_control->rr_silent_period_timestamp.sec = now.sec;
	}
}
// Need to call when a new device is added in the topology.

/**
* @brief when a device becomes controller, it updates its own steering policy.
*
* @param pGlobal_dev pointer to the global structure.
*/
void mapd_update_controller_steer_policy(struct mapd_global *pGlobal_dev)
{
	struct own_1905_device *own_device = &pGlobal_dev->dev;
	struct _1905_map_device *_1905_device = topo_srv_get_next_1905_device(own_device,NULL);
	struct radio_info_db *radio = NULL;
	struct radio_policy_db *radio_policy;
	
	own_device->map_policy.spolicy.btm_disallow_count =0;
	own_device->map_policy.spolicy.local_disallow_count = 0;

	SLIST_FOREACH(radio, &_1905_device->first_radio, next_radio) {
		//topo_srv_dump_radio_info(radio);
		own_device->map_policy.spolicy.radios++;
	}

	SLIST_FOREACH(radio, &_1905_device->first_radio, next_radio) {
		radio_policy = (struct radio_policy_db *)malloc(sizeof(struct radio_policy_db));
		if (!radio_policy) {
				err("%s alloc struct radio_policy_db fail\n", __func__);
				return;
		}
		memcpy(radio_policy->identifier, radio->identifier, ETH_ALEN);
		radio_policy->steer_policy = AGENT_STEER_RSSI_ALLOWD;
		if(isChan5GH(radio->channel[0])) {
			radio_policy->ch_util_thres = pGlobal_dev->dev.cli_steer_params.CUOverloadTh_5G_H;
		} else if (isChan5GL(radio->channel[0])) {
			radio_policy->ch_util_thres = pGlobal_dev->dev.cli_steer_params.CUOverloadTh_5G_L;
		} else if(radio->channel[0] <= 14) {
			radio_policy->ch_util_thres = pGlobal_dev->dev.cli_steer_params.CUOverloadTh_2G;
		} else {
			err("Invalid Channel\n");
			os_free(radio_policy);
			return;
		}
		radio_policy->rssi_thres = pGlobal_dev->dev.cli_steer_params.LowRSSIAPSteerEdge_root;
		SLIST_INSERT_HEAD(&own_device->map_policy.spolicy.radio_policy_head, radio_policy, radio_policy_entry);
	}

	own_device->map_policy.mpolicy.report_interval = own_device->cli_steer_params.CUAvgPeriod;
	own_device->map_policy.mpolicy.radio_num = own_device->map_policy.spolicy.radios;
}

/**
* @brief send steering and metric reporting policy to device newly added to the network 
*
* @param pGlobal_dev pointer to the global structure
* @param _1905_device 1905 device to which policy update needs to be sent.
*/
extern int8_t NOISE_OFFSET_BY_CH_WIDTH[];
void steer_msg_update_policy_config(struct mapd_global *pGlobal_dev, struct _1905_map_device *_1905_device)
{
	// no need to add any steer policy as we don't have any
	// only configure ap metric reporting interval
	struct lib_steer_radio_policy *radio_policy;
	struct radio_info_db *radio;
	u8 radio_cnt = 0,band = 0;
	u8 i=0;
	struct lib_metrics_radio_policy *metrics_policy = NULL;
#ifdef MAP_R2
	struct lib_unsuccess_assoc_policy *assoc_policy = NULL;
#endif

	SLIST_FOREACH(radio, &_1905_device->first_radio, next_radio) {
		//topo_srv_dump_radio_info(radio);
		radio_cnt++;
	}
	radio_policy = os_malloc(sizeof(struct lib_steer_radio_policy) * radio_cnt);
	metrics_policy = os_malloc(sizeof(struct lib_metrics_radio_policy) * radio_cnt);
	if(metrics_policy == NULL || radio_policy == NULL) {
		mapd_ASSERT(0);
		if(metrics_policy) {
			os_free(metrics_policy);
		}
		if(radio_policy){
			os_free(radio_policy);
		}
		return;
	}
	os_memset(metrics_policy, 0, sizeof(struct lib_metrics_radio_policy) * radio_cnt);
	SLIST_FOREACH(radio, &_1905_device->first_radio, next_radio) {
		os_memcpy(radio_policy[i].identifier, radio->identifier,ETH_ALEN);
		radio_policy[i].steer_policy = AGENT_STEER_RSSI_ALLOWD;
#ifdef CENT_STR
		if (pGlobal_dev->dev.cent_str_en)
			radio_policy[i].steer_policy = AGENT_STEER_DISALLOWED;
#endif
		if(isChan5GH(radio->channel[0])) {
			band = BAND_5GH;
			radio_policy[i].ch_util_thres = pGlobal_dev->dev.cli_steer_params.CUOverloadTh_5G_H;
		} else if (isChan5GL(radio->channel[0])) {
			band = BAND_5GL;
			radio_policy[i].ch_util_thres = pGlobal_dev->dev.cli_steer_params.CUOverloadTh_5G_L;
		} else if(radio->channel[0] <= 14) {
			band = BAND_2G;
			radio_policy[i].ch_util_thres = pGlobal_dev->dev.cli_steer_params.CUOverloadTh_2G;
		} else {
			err("Invalid Channel\n");
			os_free(radio_policy);
			os_free(metrics_policy);
			return;
		}
		radio_policy[i].rssi_thres =rssi_to_rcpi( pGlobal_dev->dev.cli_steer_params.LowRSSIAPSteerEdge_RE + (NOISE_OFFSET_BY_CH_WIDTH[0]));
		os_memcpy(metrics_policy[i].identifier, radio->identifier, ETH_ALEN);
		metrics_policy[i].metrics_inclusion = pGlobal_dev->dev.controller_context.ap_metric_policy.policy_params[band-1].MetricPolicyMetricsInclusion;
		metrics_policy[i].traffic_inclusion = pGlobal_dev->dev.controller_context.ap_metric_policy.policy_params[band-1].MetricPolicyTrafficInclusion;
		metrics_policy[i].rssi_thres = pGlobal_dev->dev.controller_context.ap_metric_policy.policy_params[band-1].MetricPolicyRcpi;
		metrics_policy[i].rssi_margin = pGlobal_dev->dev.controller_context.ap_metric_policy.policy_params[band-1].MetricPolicyHys;
		metrics_policy[i].ch_util_thres = pGlobal_dev->dev.controller_context.ap_metric_policy.policy_params[band-1].MetricPolicyChUtilThres;
		debug("radio_policy[i].rssi_thres %d, metrics_policy[i].ch_util_thres %d\n", radio_policy[i].ch_util_thres, metrics_policy[i].ch_util_thres);
		i++;
	}
#ifdef MAP_R2
	err(" due to steer msg update ");
	assoc_policy = os_zalloc(sizeof(struct lib_unsuccess_assoc_policy));
	if(assoc_policy == NULL) {
		os_free(metrics_policy);
		os_free(radio_policy);
		mapd_ASSERT(0);
		return;
	}
	assoc_policy->report_rate = 10;
	assoc_policy->report_switch = 1;
	if(pGlobal_dev->dev.ch_planning_R2.ch_plan_enable && (pGlobal_dev->dev.ch_planning_R2.ch_plan_state != CHPLAN_STATE_IDLE))
		if (_1905_device->map_version == DEV_TYPE_R2)
			map_1905_Send_MAP_Policy_Request_Message (pGlobal_dev->_1905_ctrl, (char *)_1905_device->_1905_info.al_mac_addr,0,NULL,
				0, NULL, radio_cnt, radio_policy,pGlobal_dev->dev.ch_planning_R2.ch_plan_metric_policy_interval, radio_cnt, metrics_policy, 1, 0, 1, assoc_policy);
		else
			map_1905_Send_MAP_Policy_Request_Message (pGlobal_dev->_1905_ctrl, (char *)_1905_device->_1905_info.al_mac_addr,0,NULL,
				0, NULL, radio_cnt, radio_policy,pGlobal_dev->dev.ch_planning_R2.ch_plan_metric_policy_interval, radio_cnt, metrics_policy, 0, 0, 0, NULL);
	else
		if (_1905_device->map_version == DEV_TYPE_R2)
			map_1905_Send_MAP_Policy_Request_Message (pGlobal_dev->_1905_ctrl, (char *)_1905_device->_1905_info.al_mac_addr,0,NULL,
				0, NULL, radio_cnt, radio_policy,pGlobal_dev->dev.ap_metric_rep_intv, radio_cnt, metrics_policy, 1, 0, 1, assoc_policy);
		else
			map_1905_Send_MAP_Policy_Request_Message (pGlobal_dev->_1905_ctrl, (char *)_1905_device->_1905_info.al_mac_addr,0,NULL,
				0, NULL, radio_cnt, radio_policy,pGlobal_dev->dev.ap_metric_rep_intv, radio_cnt, metrics_policy, 0, 0, 0, NULL);
	if (assoc_policy)
		os_free(assoc_policy);
#else
	map_1905_Send_MAP_Policy_Request_Message (pGlobal_dev->_1905_ctrl, (char *)_1905_device->_1905_info.al_mac_addr,0,NULL,
	0, NULL, radio_cnt, radio_policy,pGlobal_dev->dev.ap_metric_rep_intv, radio_cnt, metrics_policy);
#endif
	os_free(radio_policy);
	os_free(metrics_policy);
}

/**
* @brief fill own structure for steering window for self. This is opportunity steering with window as CUAvgPeriod
* this functoin is called when controller wants to give self steering opportunity.
*
* @param curr_own_bss own bss to which steering opportunity is given
* @param steer_req_msg steer req message is stored in the bss.
* @param bss_info target bss info required in steering req message (not used)
*/
void chan_mon_fill_steer_req_data(struct mapd_bss *curr_own_bss,
		struct lib_steer_request *steer_req_msg,
#ifdef MAP_R2
		struct map_lib_target_bssid_info *bss_info,
		struct lib_steer_request_R2 *steer_req_msg_r2,
		struct map_lib_target_bssid_info_R2 *bss_info_r2
#else
		struct map_lib_target_bssid_info *bss_info
#endif
)
{
	struct steer_request *map_steer_req = NULL;
	int i = 0;
	unsigned char sta_count = 0, bssid_count = 0, count = 0;
	unsigned char *sta_mac_pos = NULL, *target_bssid_pos = NULL;
	struct target_bssid_info *info = NULL;
	unsigned char *target_bssid_start = NULL;

	if(curr_own_bss->_1905_steer_req_msg != NULL) {
		mapd_printf(MSG_ERROR,"SteerReqMsg Not null... something wrong\n");
		os_free(curr_own_bss->_1905_steer_req_msg);
		curr_own_bss->_1905_steer_req_msg = NULL;
		curr_own_bss->steer_req_len = 0;
		curr_own_bss->steer_req_timestamp.sec = 0;
		curr_own_bss->steer_req_timestamp.usec = 0;
	}

	if(steer_req_msg && bss_info) {
	sta_count = steer_req_msg->sta_cnt;
	bssid_count = bss_info->tbss_cnt;
	}
#ifdef MAP_R2
	if(steer_req_msg_r2 && bss_info_r2) {
		sta_count = steer_req_msg_r2->sta_cnt;
		bssid_count = bss_info_r2->tbss_cnt;
	}
#endif
	count = (bssid_count > sta_count) ? bssid_count : sta_count;

	curr_own_bss->steer_req_len = sizeof(struct steer_request) + (count * sizeof(struct target_bssid_info));

	curr_own_bss->_1905_steer_req_msg = os_malloc(curr_own_bss->steer_req_len);
	map_steer_req = curr_own_bss->_1905_steer_req_msg;
	if(curr_own_bss->_1905_steer_req_msg == NULL) {
		mapd_printf(MSG_ERROR,"%s:Cannot Alloc mem\n",__func__);
		curr_own_bss->steer_req_len = 0;
		return;
	}

	debug("got steering window for device");
	if(steer_req_msg && bss_info) {
	os_memcpy(map_steer_req->assoc_bssid,steer_req_msg->bssid, ETH_ALEN);
	map_steer_req->request_mode = steer_req_msg->mode;
	map_steer_req->btm_disassoc_immi = (steer_req_msg->timer > 0);
	map_steer_req->btm_disassoc_timer = steer_req_msg->timer;
	map_steer_req->btm_abridged = 0;
	map_steer_req->steer_window = steer_req_msg->window;
	map_steer_req->sta_count = steer_req_msg->sta_cnt;

	target_bssid_pos = (u8 *)bss_info->bss_info;
	sta_mac_pos = steer_req_msg->sta_list;
	}
#ifdef MAP_R2
	if(steer_req_msg_r2 && bss_info_r2) {
		os_memcpy(map_steer_req->assoc_bssid,steer_req_msg_r2->bssid, ETH_ALEN);
		map_steer_req->request_mode = steer_req_msg_r2->mode;
		map_steer_req->btm_disassoc_immi = (steer_req_msg_r2->timer > 0);
		map_steer_req->btm_disassoc_timer = steer_req_msg_r2->timer;
		map_steer_req->btm_abridged = 0;
		map_steer_req->steer_window = steer_req_msg_r2->window;
		map_steer_req->sta_count = steer_req_msg_r2->sta_cnt;

		map_steer_req->steering_type = 1;
		target_bssid_pos = (u8 *)bss_info_r2->bss_info;
		sta_mac_pos = steer_req_msg_r2->sta_list;
	}
#endif
	
	target_bssid_start = target_bssid_pos;
	info = map_steer_req->info;


	if (sta_count > bssid_count && bssid_count == 1) {
		/*all the specified sta roam to the same bssid*/
		for (i = 0; i < count; i++) {
			os_memcpy(info->target_bssid, target_bssid_pos, ETH_ALEN);
			target_bssid_pos += ETH_ALEN;
			info->op_class = *target_bssid_pos++;
			info->channel = *target_bssid_pos++;
			mapd_printf(MSG_DEBUG,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
				PRINT_MAC(info->target_bssid));
			mapd_printf(MSG_DEBUG,"op_class=%d, channel=%d\n",info->op_class, info->channel);
			os_memcpy(info->sta_mac, sta_mac_pos, ETH_ALEN);
			mapd_printf(MSG_DEBUG,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
				PRINT_MAC(info->sta_mac));
			sta_mac_pos += ETH_ALEN;
			target_bssid_pos = target_bssid_start;
			info++;
		}
	} else if (sta_count == bssid_count && sta_count > 0) {
		/*match sta & target bssid*/
		for (i = 0; i < count; i++) {
			os_memcpy(info->target_bssid, target_bssid_pos, ETH_ALEN);
			target_bssid_pos += ETH_ALEN;
			info->op_class = *target_bssid_pos++;
			info->channel = *target_bssid_pos++;
			mapd_printf(MSG_DEBUG,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
				PRINT_MAC(info->target_bssid));
			mapd_printf(MSG_DEBUG,"op_class=%d, channel=%d\n",info->op_class, info->channel);
			os_memcpy(info->sta_mac, sta_mac_pos, ETH_ALEN);
			mapd_printf(MSG_DEBUG,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
				PRINT_MAC(info->sta_mac));
			sta_mac_pos += ETH_ALEN;
			info++;
		}
	} else if (sta_count == 0 && bssid_count == 1) {
		/*all the associated sta roam to the same target bssid*/
		os_memcpy(info->target_bssid, target_bssid_pos, ETH_ALEN);
		target_bssid_pos += ETH_ALEN;
		info->op_class = *target_bssid_pos++;
		info->channel = *target_bssid_pos++;
		mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
			PRINT_MAC(info->target_bssid));
		mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
		mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
			PRINT_MAC(info->sta_mac));
	} else if (sta_count > 0 && bssid_count == 0) {
		/*for steering oppotunity test*/
		for (i = 0; i < count; i++) {
			os_memcpy(info->target_bssid,bcast_mac,ETH_ALEN);
			mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
				PRINT_MAC(info->target_bssid));
			mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
			os_memcpy(info->sta_mac, sta_mac_pos, ETH_ALEN);
			mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
				PRINT_MAC(info->sta_mac));
			sta_mac_pos += ETH_ALEN;
			info++;
		}
	} else if (sta_count == 0 && bssid_count == 0) {
		/*for steering oppotunity test*/
		mapd_printf(MSG_DEBUG,"sta_count=bssid_count==0, no need add any target_bssid_info\n");
	} else {
		mapd_printf(MSG_ERROR,"unkonwn case sta_count=%d, bssid_count=%d\n",
			sta_count, bssid_count);

		os_free(curr_own_bss->_1905_steer_req_msg);
		curr_own_bss->_1905_steer_req_msg = NULL;
		curr_own_bss->steer_req_len = 0;
		curr_own_bss->steer_req_timestamp.sec = 0;
		curr_own_bss->steer_req_timestamp.usec = 0;
		curr_own_bss->mandate_steer_done_bitmap = 0;
		return;
	}

	os_get_time(&curr_own_bss->steer_req_timestamp);
}

/**
* @brief Generic handling of vendor specific messages. RFS/TSQ/STA_INFO
*
* @param pGlobal_dev pointer to global structure
* @param buf vendor specific message contents
* @param len length of the message.
*/
void steer_msg_handle_vendor_specific_msg(struct mapd_global *pGlobal_dev,
			struct _1905_map_device *_1905_device, struct tlv_head *tlv)
{
	if(_1905_device == NULL) {
		mapd_printf(MSG_ERROR,"1905 dev not found");
		return;
	}

	if(tlv == NULL) {
		mapd_printf(MSG_ERROR,"tlv is null");
		return;
	}

	switch(tlv->func_type)
	{

		case FUNC_STA_INFO_REQ:
			cli_mon_handle_cli_cap_update_req(pGlobal_dev,(struct sta_info_req_tlv *)tlv,_1905_device);
			break;
		case FUNC_STA_INFO_RSP:
			cli_mon_handle_cli_cap_update_rsp(pGlobal_dev, (struct sta_info_rsp_tlv *)tlv);
			break;
		case FUNC_RFS_REQ:
			steer_action_handle_rfs_req(pGlobal_dev,(struct rfs_req_tlv *)tlv, _1905_device);
			break;
		case FUNC_RFS_RSP:
			{
				RFS_RSP_SM_DATA rfs_rsp_data;
				int client_id = -1;
				rfs_rsp_data.map_1905_device = _1905_device;
				os_memcpy(&rfs_rsp_data.cli_rfs_rsp,tlv, sizeof(struct rfs_rsp_tlv));
				client_id = client_db_get_cid_from_mac(pGlobal_dev, rfs_rsp_data.cli_rfs_rsp.cli_mac);
				mapd_printf(MSG_INFO, "%s: Trigger FSM (RFS_SUCCESS)", __func__);
				steer_fsm_trigger(pGlobal_dev,client_id,RFS_SUCCESS,&rfs_rsp_data);
			}
			break;
		case FUNC_TSQ_REQ:
			steer_action_handle_tsq_req(pGlobal_dev,(struct tsq_req_tlv *)tlv,_1905_device);
			break;
		case FUNC_TSQ_RSP:
			{
				TSQ_RSP_SM_DATA tsq_rsp_data;
				int client_id = -1;
				tsq_rsp_data.map_1905_device = _1905_device;
				os_memcpy(&tsq_rsp_data.cli_tsq_rsp,tlv, sizeof(struct tsq_rsp_tlv));
				mapd_printf(MSG_INFO, "%s: Trigger FSM (TSQ_SUCCESS)", __func__);
				client_id = client_db_get_cid_from_mac(pGlobal_dev, tsq_rsp_data.cli_tsq_rsp.cli_mac);
				steer_fsm_trigger(pGlobal_dev,client_id,TSQ_SUCCESS,&tsq_rsp_data);
			}
			break;
		default:
			return;
	}

}

/**
* @brief handle steering request message from the controller and store in the bss strucure.
*
* @param pGlobal_dev pointer to the global structure
* @param buf buffer containing steering request message
* @param len length of the buffer.
*
* @return 
*/
#ifdef MAP_R2
int chan_mon_handle_steering_req(struct mapd_global *pGlobal_dev, u8 *buf, size_t len)
{

	unsigned char *temp_buf;
	unsigned short length = 0;
	int i =0;
	unsigned char assoc_bssid[ETH_ALEN]/*, al_mac[ETH_ALEN]*/;
	unsigned char req_mode = 0, disassoc_immmi = 0,abridged = 0;
	unsigned char sta_count = 0, bssid_count = 0, count = 0;
	unsigned short steer_window = 0, disassoc_time = 0;
	unsigned char *sta_mac_pos = NULL, *target_bssid_pos = NULL;
	struct target_bssid_info *info = NULL;
	unsigned char *target_bssid_start = NULL;
	struct mapd_bss *own_bss = NULL;
	unsigned char steer_type = 0; 

	temp_buf = buf;
	if(((*temp_buf) == STEERING_REQUEST_TYPE) ||
		((*temp_buf) == STEERING_REQUEST_TYPE_R2)){
				
	}
	else {
		mapd_printf(MSG_ERROR,"%s should not go here: %x\n", __func__, *temp_buf);
		return -1;
	}
	//calculate tlv length
	
	while ((*temp_buf == STEERING_REQUEST_TYPE) ||
            (*temp_buf == STEERING_REQUEST_TYPE_R2)) { 


			if((*temp_buf) == STEERING_REQUEST_TYPE) {
			temp_buf = buf;
			temp_buf++;
			
			length = (*temp_buf);
			length = (length << 8) & 0xFF00;
			length = length |(*(temp_buf+1));
			if (length < 0) {
				err("error steering request tlv");
				return -1;
			}	
			//shift to tlv value field
			temp_buf += 2;

			os_memcpy(assoc_bssid, temp_buf, ETH_ALEN);
			temp_buf += ETH_ALEN;

			req_mode = (*temp_buf & 0x80) ? 1 : 0;
			disassoc_immmi = (*temp_buf & 0x40) ? 1 : 0;
			abridged = (*temp_buf & 0x20) ? 1 : 0;
			temp_buf += 1;

			if (req_mode == 0) {
				steer_window = *temp_buf;
				steer_window = (steer_window << 8) | (*(temp_buf + 1));
			}
				temp_buf += 2;
			//if (disassoc_immmi) {
				disassoc_time = *temp_buf;
				disassoc_time = (disassoc_time << 8) | (*(temp_buf + 1));
				temp_buf += 2;
			//}

			own_bss = mapd_get_bss_from_mac(pGlobal_dev, assoc_bssid);
			if(own_bss == NULL) {
				mapd_printf(MSG_ERROR,"%s own bss is null\n", __func__);
				return -1;
			}
			sta_count = *temp_buf++;
			sta_mac_pos = temp_buf;
			temp_buf += sta_count * ETH_ALEN;

			if (req_mode == 1) {
				bssid_count = *temp_buf++;
			} 

			count = (bssid_count > sta_count) ? bssid_count : sta_count;
			debug("got steering window msg for BSS:" MACSTR, MAC2STR(own_bss->bssid));

			if(own_bss->_1905_steer_req_msg != NULL) {
				mapd_printf(MSG_ERROR,"%s: existing steer request there... %d: %p\n",__func__,own_bss->steer_req_len,
																own_bss->_1905_steer_req_msg );
				os_free(own_bss->_1905_steer_req_msg);
				own_bss->_1905_steer_req_msg = NULL;
				own_bss->steer_req_len = 0;
				own_bss->steer_req_timestamp.sec = 0;
				own_bss->steer_req_timestamp.usec = 0;
			}
			
			own_bss->steer_req_len = sizeof(struct steer_request) +
				count * sizeof(struct target_bssid_info);
			own_bss->_1905_steer_req_msg = (struct steer_request *)os_malloc(own_bss->steer_req_len);

			if (!own_bss->_1905_steer_req_msg) {
				mapd_printf(MSG_ERROR,"%s alloc cli_steer_req fail\n", __func__);
				return -1;
			}

			os_memset(own_bss->_1905_steer_req_msg, 0, own_bss->steer_req_len);

			os_memcpy(own_bss->_1905_steer_req_msg->assoc_bssid, assoc_bssid, ETH_ALEN);
			own_bss->_1905_steer_req_msg->request_mode = req_mode;
			own_bss->_1905_steer_req_msg->btm_disassoc_immi = disassoc_immmi;
			own_bss->_1905_steer_req_msg->btm_abridged = abridged;
			own_bss->_1905_steer_req_msg->steer_window = steer_window;
			own_bss->_1905_steer_req_msg->btm_disassoc_timer = disassoc_time;
			own_bss->_1905_steer_req_msg->sta_count = sta_count;
			own_bss->_1905_steer_req_msg->target_bssid_count = bssid_count;		
			debug("%s request_mode=%d, btm_disassoc_immi=%d, "
				"btm_abridged=%d, steer_window=%d, btm_disassoc_timer=%d, "
				"sta_count=%d, target_bssid_count=%d, steering_type=%d\n", __func__,
				own_bss->_1905_steer_req_msg->request_mode,
				own_bss->_1905_steer_req_msg->btm_disassoc_immi,
				own_bss->_1905_steer_req_msg->btm_abridged,
				own_bss->_1905_steer_req_msg->steer_window,
				own_bss->_1905_steer_req_msg->btm_disassoc_timer,
				own_bss->_1905_steer_req_msg->sta_count,
				own_bss->_1905_steer_req_msg->target_bssid_count,own_bss->_1905_steer_req_msg->steering_type);

			target_bssid_pos = temp_buf;
			target_bssid_start = target_bssid_pos;

			info = own_bss->_1905_steer_req_msg->info;
			if (sta_count > bssid_count && bssid_count == 1) {
				/*all the specified sta roam to the same bssid*/
				for (i = 0; i < count; i++) {
					memcpy(info->target_bssid, target_bssid_pos, ETH_ALEN);
					target_bssid_pos += ETH_ALEN;
					info->op_class = *target_bssid_pos++;
					info->channel = *target_bssid_pos++;
					mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
						PRINT_MAC(info->target_bssid));
					mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
					memcpy(info->sta_mac, sta_mac_pos, ETH_ALEN);
					mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
						PRINT_MAC(info->sta_mac));
					sta_mac_pos += ETH_ALEN;
					target_bssid_pos = target_bssid_start;		
					info++;
				}
			} else if (sta_count == bssid_count && sta_count > 0) {
				/*match sta & target bssid*/
				for (i = 0; i < count; i++) {
					memcpy(info->target_bssid, target_bssid_pos, ETH_ALEN);
					target_bssid_pos += ETH_ALEN;
					info->op_class = *target_bssid_pos++;
					info->channel = *target_bssid_pos++;
					mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
						PRINT_MAC(info->target_bssid));
					mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
					memcpy(info->sta_mac, sta_mac_pos, ETH_ALEN);
					mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
						PRINT_MAC(info->sta_mac));
					sta_mac_pos += ETH_ALEN;
					info++;
				}
			} else if (sta_count == 0 && bssid_count == 1) {
				/*all the associated sta roam to the same target bssid*/
				memcpy(info->target_bssid, target_bssid_pos, ETH_ALEN);
				target_bssid_pos += ETH_ALEN;
				info->op_class = *target_bssid_pos++;
				info->channel = *target_bssid_pos++;
				mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
					PRINT_MAC(info->target_bssid));
				mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
				mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
					PRINT_MAC(info->sta_mac));
				
			} else if (sta_count > 0 && bssid_count == 0) {
				/*for steering oppotunity test*/
				for (i = 0; i < count; i++) {
					os_memcpy(info->target_bssid,bcast_mac,ETH_ALEN);
					mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
						PRINT_MAC(info->target_bssid));
					mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
					memcpy(info->sta_mac, sta_mac_pos, ETH_ALEN);
					mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
						PRINT_MAC(info->sta_mac));
					sta_mac_pos += ETH_ALEN;
					info++;
				}
			} else if (sta_count == 0 && bssid_count == 0) {
				/*for steering oppotunity test*/
				mapd_printf(MSG_DEBUG,"sta_count=bssid_count==0, no need add any target_bssid_info\n");
			} else {
				mapd_printf(MSG_ERROR,"unkonwn case sta_count=%d, bssid_count=%d\n",
					sta_count, bssid_count);
				return -1;
			}
			os_get_time(&own_bss->steer_req_timestamp);
			
			//length = length + 3;
		//	temp_buf = buf;
		//	temp_buf += length;

			length = length + 3;
			temp_buf = buf;
			temp_buf += length;
			buf = temp_buf;

		}else if (*temp_buf == STEERING_REQUEST_TYPE_R2) {

			temp_buf = buf;
			temp_buf++;
			length = (*temp_buf);
			length = (length << 8) & 0xFF00;
			length = length |(*(temp_buf+1));

			if (length < 0) {
				err("error steering request R2 tlv");
				return -1;
			}

			steer_type = STEERING_R2;	
			//shift to tlv value field
			temp_buf += 2;

			os_memcpy(assoc_bssid, temp_buf, ETH_ALEN);
			temp_buf += ETH_ALEN;

			req_mode = (*temp_buf & 0x80) ? 1 : 0;
			disassoc_immmi = (*temp_buf & 0x40) ? 1 : 0;
			abridged = (*temp_buf & 0x20) ? 1 : 0;
			temp_buf += 1;

			if (req_mode == 0) {
				steer_window = *temp_buf;
				steer_window = (steer_window << 8) | (*(temp_buf + 1));
			}
				temp_buf += 2;
			//if (disassoc_immmi) {
				disassoc_time = *temp_buf;
				disassoc_time = (disassoc_time << 8) | (*(temp_buf + 1));
				temp_buf += 2;
			//}

			own_bss = mapd_get_bss_from_mac(pGlobal_dev, assoc_bssid);
			if(own_bss == NULL) {
				mapd_printf(MSG_ERROR,"%s own bss is null\n", __func__);
				return -1;
			}

			sta_count = *temp_buf++;
			sta_mac_pos = temp_buf;
			temp_buf += sta_count * ETH_ALEN;

			if (req_mode == 1) {
				bssid_count = *temp_buf++;
			} 

			count = (bssid_count > sta_count) ? bssid_count : sta_count;
			err("got steering window msg for BSS:" MACSTR, MAC2STR(own_bss->bssid));

			if(own_bss->_1905_steer_req_msg != NULL) {
				mapd_printf(MSG_ERROR,"%s: existing steer request there... %d: %p\n",__func__,own_bss->steer_req_len,
																own_bss->_1905_steer_req_msg );
				os_free(own_bss->_1905_steer_req_msg);
				own_bss->_1905_steer_req_msg = NULL;
				own_bss->steer_req_len = 0;
				own_bss->steer_req_timestamp.sec = 0;
				own_bss->steer_req_timestamp.usec = 0;
			}
			
			own_bss->steer_req_len = sizeof(struct steer_request) +
				count * sizeof(struct target_bssid_info);
			own_bss->_1905_steer_req_msg = (struct steer_request *)os_malloc(own_bss->steer_req_len);

			if (!own_bss->_1905_steer_req_msg) {
				mapd_printf(MSG_ERROR,"%s alloc cli_steer_req fail\n", __func__);
				return -1;
			}

			os_memset(own_bss->_1905_steer_req_msg, 0, own_bss->steer_req_len);

			os_memcpy(own_bss->_1905_steer_req_msg->assoc_bssid, assoc_bssid, ETH_ALEN);
			own_bss->_1905_steer_req_msg->request_mode = req_mode;
			own_bss->_1905_steer_req_msg->btm_disassoc_immi = disassoc_immmi;
			own_bss->_1905_steer_req_msg->btm_abridged = abridged;
			own_bss->_1905_steer_req_msg->steer_window = steer_window;
			own_bss->_1905_steer_req_msg->btm_disassoc_timer = disassoc_time;
			own_bss->_1905_steer_req_msg->sta_count = sta_count;
			own_bss->_1905_steer_req_msg->target_bssid_count = bssid_count;		
			own_bss->_1905_steer_req_msg->steering_type = steer_type;//Prakhar
			
			err("%s request_mode=%d, btm_disassoc_immi=%d, "
				"btm_abridged=%d, steer_window=%d, btm_disassoc_timer=%d, "
				"sta_count=%d, target_bssid_count=%d, steering_type=%d\n", __func__,
				own_bss->_1905_steer_req_msg->request_mode,
				own_bss->_1905_steer_req_msg->btm_disassoc_immi,
				own_bss->_1905_steer_req_msg->btm_abridged,
				own_bss->_1905_steer_req_msg->steer_window,
				own_bss->_1905_steer_req_msg->btm_disassoc_timer,
				own_bss->_1905_steer_req_msg->sta_count,
				own_bss->_1905_steer_req_msg->target_bssid_count,own_bss->_1905_steer_req_msg->steering_type);

			target_bssid_pos = temp_buf;
			target_bssid_start = target_bssid_pos;

			info = own_bss->_1905_steer_req_msg->info;
			if (sta_count > bssid_count && bssid_count == 1) {
				/*all the specified sta roam to the same bssid*/
				for (i = 0; i < count; i++) {
					memcpy(info->target_bssid, target_bssid_pos, ETH_ALEN);
					target_bssid_pos += ETH_ALEN;
					info->op_class = *target_bssid_pos++;
					info->channel = *target_bssid_pos++;
					mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
						PRINT_MAC(info->target_bssid));
					mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
					memcpy(info->sta_mac, sta_mac_pos, ETH_ALEN);
					mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
						PRINT_MAC(info->sta_mac));
					sta_mac_pos += ETH_ALEN;
					if (steer_type == 1) {
						info->reason = *target_bssid_pos++; 
					}
					target_bssid_pos = target_bssid_start;		
					info++;
				}
			} else if (sta_count == bssid_count && sta_count > 0) {
				/*match sta & target bssid*/
				for (i = 0; i < count; i++) {
					memcpy(info->target_bssid, target_bssid_pos, ETH_ALEN);
					target_bssid_pos += ETH_ALEN;
					info->op_class = *target_bssid_pos++;
					info->channel = *target_bssid_pos++;
					mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
						PRINT_MAC(info->target_bssid));
					mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
					memcpy(info->sta_mac, sta_mac_pos, ETH_ALEN);
					mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
						PRINT_MAC(info->sta_mac));
					sta_mac_pos += ETH_ALEN;
					if (steer_type == 1) {
						info->reason = *target_bssid_pos++; 
					}
					info++;
				}
			} else if (sta_count == 0 && bssid_count == 1) {
				/*all the associated sta roam to the same target bssid*/
				memcpy(info->target_bssid, target_bssid_pos, ETH_ALEN);
				target_bssid_pos += ETH_ALEN;
				info->op_class = *target_bssid_pos++;
				info->channel = *target_bssid_pos++;
				mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
					PRINT_MAC(info->target_bssid));
				mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
				mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
					PRINT_MAC(info->sta_mac));
				if (steer_type == 1){
					info->reason = *target_bssid_pos++; //Prakhar
				}
			} else if (sta_count > 0 && bssid_count == 0) {
				/*for steering oppotunity test*/
				for (i = 0; i < count; i++) {
					os_memcpy(info->target_bssid,bcast_mac,ETH_ALEN);
					mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
						PRINT_MAC(info->target_bssid));
					mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
					memcpy(info->sta_mac, sta_mac_pos, ETH_ALEN);
					mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
						PRINT_MAC(info->sta_mac));
					sta_mac_pos += ETH_ALEN;
					if (steer_type == 1) {
						info->reason = *target_bssid_pos++; 
					}
					info++;
				}
			} else if (sta_count == 0 && bssid_count == 0) {
				/*for steering oppotunity test*/
				mapd_printf(MSG_DEBUG,"sta_count=bssid_count==0, no need add any target_bssid_info\n");
			} else {
				mapd_printf(MSG_ERROR,"unkonwn case sta_count=%d, bssid_count=%d\n",
					sta_count, bssid_count);
				return -1;
			}
			
			os_get_time(&own_bss->steer_req_timestamp);
			
			//length = length + 3;
			//temp_buf += length;

			length = length + 3;
			printf("\n len is %d",length);
			temp_buf = buf;
			temp_buf += length;
			buf = temp_buf;

		}	else if (*temp_buf == END_OF_TLV_TYPE) {
			printf("\n end");
			
			break;
		}
	
	}
	
	return (length);

}


#else

int chan_mon_handle_steering_req(struct mapd_global *pGlobal_dev, u8 *buf, size_t len)
{

	unsigned char *temp_buf;
	unsigned short length = 0;
	int i =0;
	unsigned char assoc_bssid[ETH_ALEN];
	unsigned char req_mode = 0, disassoc_immmi = 0,abridged = 0;
	unsigned char sta_count = 0, bssid_count = 0, count = 0;
	unsigned short steer_window = 0, disassoc_time = 0;
	unsigned char *sta_mac_pos = NULL, *target_bssid_pos = NULL;
	struct target_bssid_info *info = NULL;
	unsigned char *target_bssid_start = NULL;
	struct mapd_bss *own_bss = NULL;

	temp_buf = buf;

	if((*temp_buf) == STEERING_REQUEST_TYPE) {
		temp_buf++;
	}
	else {
		mapd_printf(MSG_ERROR,"%s should not go here: %x\n", __func__, *temp_buf);
		return -1;
	}
	//calculate tlv length
	length = (*temp_buf);
	length = (length << 8) & 0xFF00;
	length = length |(*(temp_buf+1));

	if( 1)
	{
		//shift to tlv value field
		temp_buf += 2;

		os_memcpy(assoc_bssid, temp_buf, ETH_ALEN);
		temp_buf += ETH_ALEN;

		req_mode = (*temp_buf & 0x80) ? 1 : 0;
		disassoc_immmi = (*temp_buf & 0x40) ? 1 : 0;
		abridged = (*temp_buf & 0x20) ? 1 : 0;
		temp_buf += 1;

		if (req_mode == 0) {
			steer_window = *temp_buf;
			steer_window = (steer_window << 8) | (*(temp_buf + 1));
		}
			temp_buf += 2;
		//if (disassoc_immmi) {
			disassoc_time = *temp_buf;
			disassoc_time = (disassoc_time << 8) | (*(temp_buf + 1));
			temp_buf += 2;
		//}

		own_bss = mapd_get_bss_from_mac(pGlobal_dev, assoc_bssid);
		if(own_bss == NULL) {
			mapd_printf(MSG_ERROR,"%s own bss is null\n", __func__);
			return -1;
		}

		sta_count = *temp_buf++;
		sta_mac_pos = temp_buf;
		temp_buf += sta_count * ETH_ALEN;

		if (req_mode == 1) {
			bssid_count = *temp_buf++;
		} 

		count = (bssid_count > sta_count) ? bssid_count : sta_count;
		info("got steering window msg for BSS:" MACSTR, MAC2STR(own_bss->bssid));

		if(own_bss->_1905_steer_req_msg != NULL) {
			mapd_printf(MSG_ERROR,"%s: existing steer request there... %d: %p\n",__func__,own_bss->steer_req_len,
															own_bss->_1905_steer_req_msg );
			os_free(own_bss->_1905_steer_req_msg);
			own_bss->_1905_steer_req_msg = NULL;
			own_bss->steer_req_len = 0;
			own_bss->steer_req_timestamp.sec = 0;
			own_bss->steer_req_timestamp.usec = 0;
		}
		
		own_bss->steer_req_len = sizeof(struct steer_request) +
			count * sizeof(struct target_bssid_info);
		own_bss->_1905_steer_req_msg = (struct steer_request *)os_malloc(own_bss->steer_req_len);

		if (!own_bss->_1905_steer_req_msg) {
			mapd_printf(MSG_ERROR,"%s alloc cli_steer_req fail\n", __func__);
			return -1;
		}

		os_memset(own_bss->_1905_steer_req_msg, 0, own_bss->steer_req_len);

		os_memcpy(own_bss->_1905_steer_req_msg->assoc_bssid, assoc_bssid, ETH_ALEN);
		own_bss->_1905_steer_req_msg->request_mode = req_mode;
		own_bss->_1905_steer_req_msg->btm_disassoc_immi = disassoc_immmi;
		own_bss->_1905_steer_req_msg->btm_abridged = abridged;
		own_bss->_1905_steer_req_msg->steer_window = steer_window;
		own_bss->_1905_steer_req_msg->btm_disassoc_timer = disassoc_time;
		own_bss->_1905_steer_req_msg->sta_count = sta_count;
		own_bss->_1905_steer_req_msg->target_bssid_count = bssid_count;
		info("%s request_mode=%d, btm_disassoc_immi=%d, "
			"btm_abridged=%d, steer_window=%d, btm_disassoc_timer=%d, "
			"sta_count=%d, target_bssid_count=%d\n", __func__,
			own_bss->_1905_steer_req_msg->request_mode,
			own_bss->_1905_steer_req_msg->btm_disassoc_immi,
			own_bss->_1905_steer_req_msg->btm_abridged,
			own_bss->_1905_steer_req_msg->steer_window,
			own_bss->_1905_steer_req_msg->btm_disassoc_timer,
			own_bss->_1905_steer_req_msg->sta_count,
			own_bss->_1905_steer_req_msg->target_bssid_count);

		target_bssid_pos = temp_buf;
		target_bssid_start = target_bssid_pos;

		info = own_bss->_1905_steer_req_msg->info;
		if (sta_count > bssid_count && bssid_count == 1) {
			/*all the specified sta roam to the same bssid*/
			for (i = 0; i < count; i++) {
				memcpy(info->target_bssid, target_bssid_pos, ETH_ALEN);
				target_bssid_pos += ETH_ALEN;
				info->op_class = *target_bssid_pos++;
				info->channel = *target_bssid_pos++;
				mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
					PRINT_MAC(info->target_bssid));
				mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
				memcpy(info->sta_mac, sta_mac_pos, ETH_ALEN);
				mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
					PRINT_MAC(info->sta_mac));
				sta_mac_pos += ETH_ALEN;
				target_bssid_pos = target_bssid_start;
				info++;
			}
		} else if (sta_count == bssid_count && sta_count > 0) {
			/*match sta & target bssid*/
			for (i = 0; i < count; i++) {
				memcpy(info->target_bssid, target_bssid_pos, ETH_ALEN);
				target_bssid_pos += ETH_ALEN;
				info->op_class = *target_bssid_pos++;
				info->channel = *target_bssid_pos++;
				mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
					PRINT_MAC(info->target_bssid));
				mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
				memcpy(info->sta_mac, sta_mac_pos, ETH_ALEN);
				mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
					PRINT_MAC(info->sta_mac));
				sta_mac_pos += ETH_ALEN;
				info++;

			}
		} else if (sta_count == 0 && bssid_count == 1) {
			/*all the associated sta roam to the same target bssid*/
			memcpy(info->target_bssid, target_bssid_pos, ETH_ALEN);
			target_bssid_pos += ETH_ALEN;
			info->op_class = *target_bssid_pos++;
			info->channel = *target_bssid_pos++;
			mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
				PRINT_MAC(info->target_bssid));
			mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
			mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
				PRINT_MAC(info->sta_mac));
		} else if (sta_count > 0 && bssid_count == 0) {
			/*for steering oppotunity test*/
			for (i = 0; i < count; i++) {
				os_memcpy(info->target_bssid,bcast_mac,ETH_ALEN);
				mapd_printf(MSG_ERROR,"target_bssid(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
					PRINT_MAC(info->target_bssid));
				mapd_printf(MSG_ERROR,"op_class=%d, channel=%d\n",info->op_class, info->channel);
				memcpy(info->sta_mac, sta_mac_pos, ETH_ALEN);
				mapd_printf(MSG_ERROR,"sta mac(%d)=%02x:%02x:%02x:%02x:%02x:%02x\n",i,
					PRINT_MAC(info->sta_mac));
				sta_mac_pos += ETH_ALEN;
				info++;

			}
		} else if (sta_count == 0 && bssid_count == 0) {
			/*for steering oppotunity test*/
			mapd_printf(MSG_DEBUG,"sta_count=bssid_count==0, no need add any target_bssid_info\n");
		} else {
			mapd_printf(MSG_ERROR,"unkonwn case sta_count=%d, bssid_count=%d\n",
				sta_count, bssid_count);
			return -1;
		}
	}
	os_get_time(&own_bss->steer_req_timestamp);
	
#ifdef CENT_STR
	if(pGlobal_dev->dev.cent_str_en && own_bss->_1905_steer_req_msg->request_mode == 1){
		struct client *arr_cand_list[10];
		int cand_cnt = 0;
		struct steer_cands *cand = NULL;

		ap_roam_algo_select_steer_candidate(pGlobal_dev, arr_cand_list, &cand_cnt);
		for (i = 0 ; i < cand_cnt ; i++) {
			cand = NULL;
			mapd_printf(MSG_OFF, "Trigger Steering for client =%d "MACSTR,
						arr_cand_list[i]->client_id, MAC2STR(arr_cand_list[i]->mac_addr));
			/* Once steering is complete, Below 2 shall be reset 
			 * Also, send steer complete when only_one_in_win is set*/
			cand = os_zalloc(sizeof(*cand));
			cand->steer_cand = arr_cand_list[i];
			os_memcpy(cand->steer_cand_home_bssid, arr_cand_list[i]->bssid, ETH_ALEN);
			SLIST_INSERT_HEAD(&pGlobal_dev->dev.steer_cands_head, cand, next_cand);
			steer_fsm_trigger(pGlobal_dev, arr_cand_list[i]->client_id,
						mapd_get_trigger_from_steer_method(pGlobal_dev,
								arr_cand_list[i]->cli_steer_method), NULL);
		}


	}
#endif

	return (length+3);

}
#endif

/**
* @brief determine the 1905 device and the corresponding bss to which steering req 
*  needs to be sent. if the device is own device then update own structures otherwise 
* send the message to the 1905 device.
*
* @param pGlobal_dev pointer to the global structure
*
* @return TRUE if steering request was sent. Else return false
*/
Boolean chan_mon_trigger_round_robin_steer_req(struct mapd_global*pGlobal_dev)
{
	struct own_1905_device *own_device = &pGlobal_dev->dev;
	struct rr_steer_controller *rr_control = &own_device->controller_context.rr_control;
	struct lib_steer_request *steer_req_msg = NULL;
	struct map_lib_target_bssid_info *bss_info = NULL;
	struct mapd_bss *curr_own_bss = NULL;
	struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(&pGlobal_dev->dev, NULL);
	Boolean ret = FALSE;
	
	if((rr_control->p_current_1905_rr == own_1905_node) && (rr_control->p_current_1905_rr->p_current_bss_rr != NULL)) {
		curr_own_bss = mapd_get_bss_from_mac(pGlobal_dev, rr_control->p_current_1905_rr->p_current_bss_rr->bssid);
		if (!curr_own_bss) {
			err("1chan_mon_trigger_round_robin_steer_req bssid " MACSTR "\n",
				MAC2STR(rr_control->p_current_1905_rr->p_current_bss_rr->bssid));
			rr_control->p_current_1905_rr->p_current_bss_rr = NULL;
		} else {
			if (curr_own_bss->_1905_steer_req_msg) {
				os_memset(curr_own_bss->_1905_steer_req_msg, 0, curr_own_bss->steer_req_len);
				os_free(curr_own_bss->_1905_steer_req_msg);
				curr_own_bss->_1905_steer_req_msg = NULL;
				curr_own_bss->steer_req_len = 0;
			}
		}
	}

	while(1)
	{
		rr_control->p_current_1905_rr = topo_srv_get_next_1905_device(&pGlobal_dev->dev,rr_control->p_current_1905_rr);


		if (rr_control->p_current_1905_rr == NULL) {
			rr_control->p_current_1905_rr = own_1905_node;
		}
		if(rr_control->p_current_1905_rr->in_network)
		{
			break;
		}
	}

	//Raghav: check if this device supports WLAN
	while (SLIST_EMPTY(&rr_control->p_current_1905_rr->first_radio)) {
		rr_control->p_current_1905_rr = topo_srv_get_next_1905_device(&pGlobal_dev->dev,rr_control->p_current_1905_rr);
		if (rr_control->p_current_1905_rr == NULL) {
			rr_control->p_current_1905_rr = own_1905_node;
		}
	}

	rr_control->p_current_1905_rr->p_current_bss_rr =
		topo_srv_get_next_bss(rr_control->p_current_1905_rr, rr_control->p_current_1905_rr->p_current_bss_rr);
	
	if(rr_control->p_current_1905_rr->p_current_bss_rr == NULL) {
		rr_control->p_current_1905_rr->p_current_bss_rr =
			topo_srv_get_next_bss(rr_control->p_current_1905_rr, NULL);
	}

	if(rr_control->p_current_1905_rr == own_1905_node) {
		curr_own_bss = mapd_get_bss_from_mac(pGlobal_dev, rr_control->p_current_1905_rr->p_current_bss_rr->bssid);
		if (curr_own_bss) {
			chan_mon_create_steer_req_opp(own_device,rr_control->p_current_1905_rr->p_current_bss_rr, &steer_req_msg, &bss_info);
			//os_memcpy(&curr_own_bss->steer_req_msg, &steer_req_msg, sizeof(steer_req_msg));
#ifdef MAP_R2
			chan_mon_fill_steer_req_data(curr_own_bss, steer_req_msg, bss_info,NULL,NULL);
#else
			chan_mon_fill_steer_req_data(curr_own_bss, steer_req_msg, bss_info);

#endif
			ret = TRUE;
		} else {
			err("2chan_mon_trigger_round_robin_steer_req bssid " MACSTR "\n",
				MAC2STR(rr_control->p_current_1905_rr->p_current_bss_rr->bssid));
			rr_control->p_current_1905_rr->p_current_bss_rr = NULL;
			ret = FALSE;
		}

	} else if(rr_control->p_current_1905_rr->p_current_bss_rr != NULL) {
	
		//steer_msg_update_policy_config(pGlobal_dev, rr_control->p_current_1905_rr);//remove from here , send policy when rx topo rsp
		chan_mon_create_steer_req_opp(own_device,rr_control->p_current_1905_rr->p_current_bss_rr, &steer_req_msg, &bss_info);
#ifdef MAP_R2
		map_1905_Send_Client_Steering_Request_Message(pGlobal_dev->_1905_ctrl,
												(char *)rr_control->p_current_1905_rr->_1905_info.al_mac_addr,
												steer_req_msg, 0, NULL, NULL, 0, NULL); //Prakhar
#else

		map_1905_Send_Client_Steering_Request_Message(pGlobal_dev->_1905_ctrl,
			(char *)rr_control->p_current_1905_rr->_1905_info.al_mac_addr,
			steer_req_msg, 0, NULL); 
#endif

		ret = TRUE;
	}

	if(steer_req_msg)
		os_free(steer_req_msg);

	if(bss_info)
		os_free(bss_info);

return ret;
}

/**
* @brief trigger sta info request message to get the capabilities of a client. this is called
* when sta is associated to our device.
* @param pGlobal_dev pointer to the global structure
* @param map_client client for which capability is requested
* @param al_mac mac of the device to which requst has to be sent.
*/
void cli_mon_trigger_cli_cap_update(struct mapd_global*pGlobal_dev, struct client *map_client, u8 *al_mac)
{
	struct sta_info_req_tlv sta_info_req;

	sta_info_req.tlv_type = TLV_802_11_VENDOR_SPECIFIC;
	sta_info_req.tlv_len = STA_INFO_REQ_TLV_LEN;
	os_memcpy(sta_info_req.mtk_oui, MTK_OUI,OUI_LEN);
	sta_info_req.func_type = FUNC_STA_INFO_REQ;
	os_memcpy(sta_info_req.cli_mac, map_client->mac_addr, ETH_ALEN);
	sta_info_req.tlv_len = host_to_be16(sta_info_req.tlv_len);

	map_1905_Send_Vendor_Specific_Message(pGlobal_dev->_1905_ctrl, (char *)al_mac,(char *)&sta_info_req,sizeof(sta_info_req));
}


/**
* @brief send sta_info request message to all the devices in the network to get the updated 
* capabilities of the client.
* @param global pointer to global structure
* @param map_client client for which sta info request is sent
*/
void cli_mon_trigger_cli_cap_update_to_all(struct mapd_global*global, struct client *map_client)
{
	struct _1905_map_device *_1905_device = NULL;

	_1905_device = topo_srv_get_next_1905_device(&global->dev,NULL);
	_1905_device = topo_srv_get_next_1905_device(&global->dev,_1905_device);

	while(_1905_device) {
		cli_mon_trigger_cli_cap_update(global, map_client, _1905_device->_1905_info.al_mac_addr);
		_1905_device = topo_srv_get_next_1905_device(&global->dev,_1905_device);
	}

}

/**
* @brief handle sta info request message and send the client info if available.
*
* @param pGlobal_dev pointer to the global structure
* @param sta_info_req sta_info request message received
* @param _1905_device 1905 device from which this message is received.
*/
void cli_mon_handle_cli_cap_update_req(struct mapd_global*pGlobal_dev,
												struct sta_info_req_tlv *sta_info_req,
												struct _1905_map_device* _1905_device)
{
	struct sta_info_rsp_tlv sta_info_rsp;
	struct client *map_client = NULL;

	os_memset(&sta_info_rsp, 0, sizeof(struct sta_info_rsp_tlv));
	map_client = client_db_get_client_from_sta_mac(pGlobal_dev,sta_info_req->cli_mac);
	if(map_client == NULL) {
		mapd_printf(MSG_ERROR,"%s: cli not found\n", __func__);
		return;
	}

	sta_info_rsp.tlv_type = TLV_802_11_VENDOR_SPECIFIC;
	sta_info_rsp.tlv_len = STA_INFO_RSP_TLV_LEN;
	os_memcpy(sta_info_rsp.mtk_oui, MTK_OUI,OUI_LEN);
	sta_info_rsp.func_type = FUNC_STA_INFO_REQ;
	os_memcpy(sta_info_rsp.cli_mac, map_client->mac_addr, ETH_ALEN);
	

	if(map_client->known_bands & BIT(0))
	sta_info_rsp.band_standard_cap |= BIT(0); //2.4G
	if(map_client->known_bands & BIT(1))
		sta_info_rsp.band_standard_cap |= BIT(1); //5G

	if(map_client->capab & CAP_11K_SUPPORTED)
	sta_info_rsp.band_standard_cap |= BIT(2); //11k
	if(map_client->capab & CAP_11V_SUPPORTED)
	sta_info_rsp.band_standard_cap |= BIT(3); //11v
	if(map_client->capab & CAP_MBO_SUPPORTED)
	sta_info_rsp.band_standard_cap |= BIT(4); //MBO
	sta_info_rsp.tlv_len = host_to_be16(sta_info_rsp.tlv_len);

	map_1905_Send_Vendor_Specific_Message(pGlobal_dev->_1905_ctrl, (char *)_1905_device->_1905_info.al_mac_addr, (char *)&sta_info_rsp, sizeof(sta_info_rsp));

}

/**
* @brief handle sta info response message and update the client capabilites
*
* @param pGlobal_dev pointer to the global structure
* @param sta_info_rsp sta info response message
*/
void cli_mon_handle_cli_cap_update_rsp(struct mapd_global*pGlobal_dev, 
													struct sta_info_rsp_tlv *sta_info_rsp)
{
	struct client *map_client = NULL;
	u8 band_standard_cap;
	map_client = client_db_get_client_from_sta_mac(pGlobal_dev,sta_info_rsp->cli_mac);

	if(map_client == NULL) {
		mapd_printf(MSG_ERROR,"%s: cli not found\n", __func__);
		return;
	}
	band_standard_cap = sta_info_rsp->band_standard_cap;

// Take the max value.
	if(band_standard_cap & BIT(0))
		map_client->known_bands |= BIT(0);
	if(band_standard_cap & BIT(1))
		map_client->known_bands |= BIT(1);

	if(band_standard_cap & BIT(2))
		map_client->capab |= BIT(0);
	if(band_standard_cap & BIT(3))
		map_client->capab |= BIT(1);
	if(band_standard_cap & BIT(4))
		map_client->capab |= BIT(2);

}
#endif
/**
 * @brief : check if a radio is overloaded by considering the criterions for
 * overloading
 *
 * @param global: global device pointer
 * @param ra_info: pointer to the radio structure which is to be checked for loading
 *
 * @return : 1 if radio is overloaded, 0 otherwise
 */
Boolean chan_mon_is_radio_ol(struct mapd_global *global, struct mapd_radio_info *ra_info)
{
	u8 ol_th = 0;
	uint32_t max_clients_th = 0;
	uint32_t clients_on_radio = 0;
	struct mapd_bss *bss = NULL;

	ol_th =  chan_mon_get_ol_th(global, ra_info->radio_idx);
	max_clients_th = global->dev.cli_steer_params.MaxClientOverloaded;

	/* Calculate the total num of clients */
	dl_list_for_each(bss, &ra_info->bss_list, struct mapd_bss, bss_entry) {
			clients_on_radio += dl_list_len(&bss->assoc_sta_list);
	}
	if((clients_on_radio >= max_clients_th) || (ra_info->ch_util > ol_th))
		return TRUE;
	return FALSE;
}

unsigned char is_channel_in_opclass(unsigned char operclass, unsigned char bandwidth, unsigned char channel) {
	u8 i=0;
	enum op_bw bw = BW20;
	switch(bandwidth)
	{
		case BW_20:
			bw =  BW20;
			break;
		case BW_40:
			bw = BW40PLUS;
			break;
		case BW_80:
			bw = BW80;
			break;
		case BW_160:
			bw = BW160;
			break;
		case BW_8080:
			bw = BW80P80; //XXX:We don't have the data for 160Mhz
			break;
		default:
			mapd_printf(MSG_ERROR, "BW not supp %d", bandwidth);
			mapd_ASSERT(0);
	}

	while (1) {
		if(global_op_class[i].op_class == 0) {
			mapd_printf(MSG_INFO, "Channel %d not belong to this opclass %d\n", channel, operclass);
			break;
		}
		if(bw == BW40PLUS) {
			if(global_op_class[i].bw != BW40PLUS && global_op_class[i].bw != BW40MINUS) {
				i++;
				continue;
			}
		} else {
			if(global_op_class[i].bw != bw) {
				i++;
				continue;
			}
		}
		if (global_op_class[i].op_class != operclass) {
			i++;
			continue;
		}
		if((channel >= global_op_class[i].min_chan) && (channel <= global_op_class[i].max_chan))
			return TRUE;
		i++;
	}
	return 0;

}

int chan_mon_get_op_class_frm_channel(u8 channel, u8 bandwidth)
{
	u8 i=0;
	enum op_bw bw = BW20;
	switch(bandwidth)
	{
		case BW_20:
			bw =  BW20;
			break;
		case BW_40:
			bw = BW40PLUS;
			break;
		case BW_80:
			bw = BW80;
			break;
		case BW_160:
			bw = BW160;
			break;
		case BW_8080:
			bw = BW80P80; //XXX:We don't have the data for 160Mhz
			break;
		default:
			mapd_printf(MSG_ERROR, "BW not supp %d", bandwidth);
			mapd_ASSERT(0);
	}

	while (1) {
		if(global_op_class[i].op_class == 0) {
			mapd_printf(MSG_ERROR, "op_class not found for channel %d\n", channel);
			break;
		}
		if(bw == BW40PLUS) {
			if(global_op_class[i].bw != BW40PLUS && global_op_class[i].bw != BW40MINUS) {
				i++;
				continue;
			}
		} else {
			if(global_op_class[i].bw != bw) {
				i++;
				continue;
			}
		}
		if((channel >= global_op_class[i].min_chan) && (channel <= global_op_class[i].max_chan))
			return global_op_class[i].op_class;
		i++;
	}
	return 0;
}


/**
 * @brief : mapping function from operating class to channel bandwidth using a
 * global op_class table
 *
 * @param op_class: operating class to be mapped to bandwidth
 *
 * @return : return the bandwidth (to be interpreted as enum max_bw)
 */
int chan_mon_get_bw_from_op_class(u8 op_class)
{
	const struct oper_class_map *op = &global_op_class[0];

	mapd_printf(MSG_DEBUG, "Op Class=%d", op_class);

	op = &global_op_class[0];
	while (op->op_class && op->op_class != op_class)
			op++;

	if (!op->op_class) {
		//mapd_printf(MSG_ERROR, "Op Class not found in Global OpClass Table, workaround!!!\n");
		return BW_20;
		//mapd_ASSERT(0);
		//return -1;
	}
	switch(op->bw)
	{
		case BW20:
			return BW_20;
		case BW40PLUS:
		case BW40MINUS:
			return BW_40;
		case BW80:
			return BW_80;
		case BW160:
#ifdef MAP_160BW
			return BW_160;
#else
			return BW_80;
#endif
		case BW80P80:
			return BW_80; //XXX:We don't have the data for 160Mhz
		case BW2160:
			mapd_printf(MSG_ERROR, "11ad opclass not supp");
			mapd_ASSERT(0);
			break;
		default:
			mapd_printf(MSG_ERROR, "opclass not supp");
			mapd_ASSERT(0);
			break;
	}
	return BW_20;
}
#ifdef SUPPORT_MULTI_AP
void update_client_coordination_state_for_assoc_control(struct mapd_global *global, struct cli_assoc_control *assoc_cntrl, u8 len)
{

		struct client *map_client = NULL;
		debug("assoc ctrl"MACSTR, MAC2STR(assoc_cntrl->sta_mac));
		map_client = client_db_get_client_from_sta_mac(global, assoc_cntrl->sta_mac);
		if(map_client){

			if(assoc_cntrl->assoc_control)
				map_client->coord_data.cli_coordination_state = COORD_STATE_IDLE;
			else {
				map_client->coord_data.cli_coordination_state = COORD_STATE_ASSOC_CONTROL_RECEIVED;
				os_get_reltime(&map_client->coord_data.assoc_cntrl_req_timestamp);
				mapd_printf(MSG_OFF, "CLi Id %d Coord State %d, assoc sec:%ld \n",map_client->client_id, map_client->coord_data.cli_coordination_state, 
					map_client->coord_data.assoc_cntrl_req_timestamp.sec);

			}
			mapd_printf(MSG_INFO, "CLi Id %d Coord State %d \n",map_client->client_id, map_client->coord_data.cli_coordination_state);
		}
}
#endif

u8 chan_mon_get_safety_th(struct mapd_global *global, u8 radio_idx)
{
    struct mapd_radio_info *ra_info =  &global->dev.dev_radio_info[radio_idx];

    if (ra_info->channel >= MIN_2G_CH && ra_info->channel <= MAX_2G_CH)
        return global->dev.cli_steer_params.CUSafetyTh_2G;
    else if(ra_info->channel >=36 && ra_info->channel <= 64)
        return global->dev.cli_steer_params.CUSafetyTh_5G_L;
    else
        return global->dev.cli_steer_params.CUSafetyh_5G_H;
}

void chan_mon_create_steer_req_mandate(
	char sta_cnt,
	char *sta_mac_list,
	char bss_cnt,
	char *cli_bssid,
	struct bss_info_db map_bss[],
	struct lib_steer_request **steer_req_msg,
#ifdef MAP_R2
	struct map_lib_target_bssid_info **bss_info,
	struct lib_steer_request_R2 **steer_req_msg_r2,
	struct map_lib_target_bssid_info_R2 **bss_info_r2,
	char steer_reason_code
#else
	struct map_lib_target_bssid_info **bss_info
#endif
	)
{
	struct lib_steer_request *tmp_steer_req = NULL;
	struct map_lib_target_bssid_info *tmp_bss_info = NULL;
#ifdef MAP_R2
	struct lib_steer_request_R2 *tmp_steer_req_r2 = NULL;
	struct map_lib_target_bssid_info_R2 *tmp_bss_info_r2 = NULL;
#endif
	int i=0;

	if(steer_req_msg && bss_info ){
	tmp_steer_req = os_malloc(sizeof (struct lib_steer_request) + (sta_cnt*ETH_ALEN));
	tmp_bss_info = os_malloc(sizeof (struct map_lib_target_bssid_info) +(bss_cnt*sizeof(struct lib_target_bssid_info)));
	if(tmp_steer_req == NULL || tmp_bss_info == NULL) {
		if (tmp_steer_req)
			os_free(tmp_steer_req);
		if (tmp_bss_info)
			os_free(tmp_bss_info);
		mapd_printf(MSG_ERROR,"%s: Cannot allocate memory\n", __func__);
		return;
	}
	os_memset(tmp_steer_req,0 ,(sizeof (struct lib_steer_request))+(sta_cnt*ETH_ALEN));
	os_memset(tmp_bss_info,0 , (sizeof (struct map_lib_target_bssid_info)+(bss_cnt*sizeof(struct lib_target_bssid_info))));
	if((sta_cnt>1)||(bss_cnt>1))
	{
		mapd_printf(MSG_ERROR,"sta_cnt and bss_cnt more than 1 not supported\n");
	}
	*steer_req_msg = tmp_steer_req;
	*bss_info = tmp_bss_info;
	os_memcpy(tmp_steer_req->bssid,cli_bssid ,ETH_ALEN);//give bssid of 1905 already connected with 
	tmp_steer_req->mode = 1; // Mandate
	tmp_steer_req->window = 0;
	tmp_steer_req->timer = 0;
	tmp_steer_req->disassoc_imminent = 0;
	tmp_steer_req->abridged = 0;
	tmp_steer_req->sta_cnt = sta_cnt; 
	os_memcpy(tmp_steer_req->sta_list,sta_mac_list ,sta_cnt*ETH_ALEN);//fill sta list 
	tmp_bss_info->tbss_cnt = bss_cnt;
	for(i=0;i<bss_cnt;i++){
	os_memcpy(tmp_bss_info->bss_info[i].bssid,map_bss[i].bssid ,ETH_ALEN);//give bssid of 1905 you want to connect with 
	tmp_bss_info->bss_info[i].op_class=map_bss[i].radio->operating_class;
	tmp_bss_info->bss_info[i].channel = map_bss[i].radio->channel[0];
	}
#if 1
	mapd_printf(MSG_ERROR,"%s agent_mac(%02x:%02x:%02x:%02x:%02x:%02x) , cli_mac(%02x:%02x:%02x:%02x:%02x:%02x)"
				"target_bssid(%02x:%02x:%02x:%02x:%02x:%02x)"
				"bss_cnt %d, operating class=%d, channel=%d\n", __func__,
				PRINT_MAC(tmp_steer_req->bssid),
				PRINT_MAC(tmp_steer_req->sta_list),
				PRINT_MAC(tmp_bss_info->bss_info[0].bssid),
				tmp_bss_info->tbss_cnt ,tmp_bss_info->bss_info[0].op_class, tmp_bss_info->bss_info[0].channel);
#endif
}
#ifdef MAP_R2
	if(steer_req_msg_r2 && bss_info_r2){
		tmp_steer_req_r2 = os_malloc(sizeof (struct lib_steer_request_R2) + (sta_cnt*ETH_ALEN));
		tmp_bss_info_r2 = os_malloc(sizeof (struct map_lib_target_bssid_info_R2) +(bss_cnt*sizeof(struct lib_target_bssid_info_R2)));
		if(tmp_steer_req_r2 == NULL || tmp_bss_info_r2 == NULL) {
			if (tmp_steer_req_r2)
				os_free(tmp_steer_req_r2);
			if (tmp_bss_info_r2)
				os_free(tmp_bss_info_r2);
			mapd_printf(MSG_ERROR,"%s: Cannot allocate memory\n", __func__);
			return;
		}
		os_memset(tmp_steer_req_r2,0 ,(sizeof (struct lib_steer_request_R2))+(sta_cnt*ETH_ALEN));
		os_memset(tmp_bss_info_r2,0 , (sizeof (struct map_lib_target_bssid_info_R2)+(bss_cnt*sizeof(struct lib_target_bssid_info_R2))));
		if((sta_cnt>1)||(bss_cnt>1))
		{
			mapd_printf(MSG_ERROR,"sta_cnt and bss_cnt more than 1 not supported\n");
		}
		*steer_req_msg_r2 = tmp_steer_req_r2;
		*bss_info_r2 = tmp_bss_info_r2;
		os_memcpy(tmp_steer_req_r2->bssid,cli_bssid ,ETH_ALEN);//give bssid of 1905 already connected with
		tmp_steer_req_r2->mode = 1; // Mandate
		tmp_steer_req_r2->window = 0;
		tmp_steer_req_r2->timer = 0;
		tmp_steer_req_r2->disassoc_imminent = 0;
		tmp_steer_req_r2->abridged = 0;
		tmp_steer_req_r2->sta_cnt = sta_cnt;
		os_memcpy(tmp_steer_req_r2->sta_list,sta_mac_list ,sta_cnt*ETH_ALEN);//fill sta list
		tmp_bss_info_r2->tbss_cnt = bss_cnt;
		for(i=0;i<bss_cnt;i++){
		os_memcpy(tmp_bss_info_r2->bss_info[i].bssid,map_bss[i].bssid ,ETH_ALEN);//give bssid of 1905 you want to connect with
		tmp_bss_info_r2->bss_info[i].op_class=map_bss[i].radio->operating_class;
		tmp_bss_info_r2->bss_info[i].channel = map_bss[i].radio->channel[0];
		tmp_bss_info_r2->bss_info[i].channel = map_bss[i].radio->channel[0];
		tmp_bss_info_r2->bss_info[i].reason_code = 0;
		}
#if 1
		mapd_printf(MSG_ERROR,"%s agent_mac(%02x:%02x:%02x:%02x:%02x:%02x) , cli_mac(%02x:%02x:%02x:%02x:%02x:%02x)"
					"target_bssid(%02x:%02x:%02x:%02x:%02x:%02x)"
					"bss_cnt %d, operating class=%d, channel=%d\n", __func__,
					PRINT_MAC(tmp_steer_req_r2->bssid),
					PRINT_MAC(tmp_steer_req_r2->sta_list),
					PRINT_MAC(tmp_bss_info_r2->bss_info[0].bssid),
					tmp_bss_info_r2->tbss_cnt ,tmp_bss_info_r2->bss_info[0].op_class, tmp_bss_info_r2->bss_info[0].channel);
#endif
	}

#endif
}


#ifdef CENT_STR
u8 chan_mon_get_safety_th_by_chan(struct mapd_global *global, u8 channel)
{
    
    if (channel >= MIN_2G_CH && channel <= MAX_2G_CH)
        return global->dev.cli_steer_params.CUSafetyTh_2G;
    else if(channel >=36 && channel <= 64)
        return global->dev.cli_steer_params.CUSafetyTh_5G_L;
    else
        return global->dev.cli_steer_params.CUSafetyh_5G_H;
}

u8 chan_mon_get_ol_th_by_chan(struct mapd_global *global, u8 channel)
{
    
    if (channel >= 1 && channel <= 14)
        return global->dev.cli_steer_params.CUOverloadTh_2G;
    else if(channel >=36 && channel <= 64)
        return global->dev.cli_steer_params.CUOverloadTh_5G_L;
    else
        return global->dev.cli_steer_params.CUOverloadTh_5G_H;
}

Boolean chan_mon_is_1905_radio_ol(struct mapd_global *global, struct radio_info_db *target_radio) {
	struct _1905_map_device * dev = NULL;
	struct bss_info_db *bss = NULL;
	uint32_t max_clients_th = 0;
	uint32_t clients_on_radio = 0;
	u8 radio_ol = 0;
	u8 ol_th = 0;
	
	
	if(!target_radio){
		printf("target radio is null");
		return FALSE;
	}

	dev = target_radio->parent_1905;	
	
	ol_th = chan_mon_get_ol_th_by_chan(global,target_radio->channel[0]);
	max_clients_th = global->dev.cli_steer_params.MaxClientOverloaded;
	
	SLIST_FOREACH(bss, &dev->first_bss, next_bss) {
	  if((bss->radio == target_radio) && bss->ch_util > ol_th){
			if(!radio_ol && bss->ch_util > ol_th)
				radio_ol = 1;
			clients_on_radio += bss->assoc_sta_cnt;	
	  }		
	}

   	if((clients_on_radio >= max_clients_th) || radio_ol)
		return TRUE;		
	
  return FALSE;
}

Boolean chan_mon_is_1905_radio_safety_ol(struct mapd_global *global,struct radio_info_db *target_radio) {
	struct _1905_map_device * dev = NULL;
	struct bss_info_db *bss = NULL;
	u8 saftey_th = 0;
	
	

	if(!target_radio) {
		printf("target radio is null");
		return FALSE;
	}
	
	dev = target_radio->parent_1905;
	
	saftey_th = chan_mon_get_safety_th_by_chan(global,target_radio->channel[0]);

	
	SLIST_FOREACH(bss, &dev->first_bss, next_bss) {
	  if((bss->radio == target_radio)) {
			if(bss->ch_util > saftey_th)
				return TRUE;
				
	  }		
	}

  return FALSE;
}

#endif

