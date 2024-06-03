/*
 * WPA Supplicant / Control interface (shared code for all backends)
 * Copyright (c) 2004-2015, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */
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
 *  Control interface
 *
 *  Abstract:
 *  Control interface
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Neelansh.M   2018/05/02     Derived from WPA Supplicant / Control interface
 * */

#include "utils/includes.h"
#include "utils/common.h"
#include "utils/eloop.h"
#include "client_db.h"
#include "mapd_i.h"
#include "db.h"
#ifdef SUPPORT_MULTI_AP
#include "./../1905_local_lib/data_def.h"
#include "topologySrv.h"
#endif
#include "ap_roam_algo.h"
#include <sys/un.h>
#ifdef SUPPORT_MULTI_AP
#include "1905_map_interface.h"
#endif
#include "wapp_if.h"
#ifdef SUPPORT_MULTI_AP
#include "ch_planning.h"
#endif
#include "version.h"
#ifdef SUPPORT_MULTI_AP
#include "network_optimization.h"
#endif
#include "ctrl_iface.h"

#ifdef MAP_R2
#ifdef SUPPORT_1905

int map_1905_Send_Channel_Scan_Request_Message (
	IN struct _1905_context* ctx, char* almac,
	IN unsigned char fresh_scan, IN int radio_cnt,
	IN unsigned char *scan_req);


unsigned char is_profile2_dev(struct mapd_global *global,unsigned char *almac)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(&global->dev, almac);
	if (dev->map_version == DEV_TYPE_R2)
		return 1;
	return 0;
}



int map_cmd_ch_scan_req( struct mapd_global *global, u8 val)
{
#if 1

	struct channel_scan_req *scan_req = NULL;
	struct radio_info_db *radio = NULL;
	struct _1905_map_device *_1905_device = NULL;
	u8 buf[8000] = {0};
	u8 boot_only_scan = 0;
	u32 len;
	u8 val2 = 1;
	u8 radio_id_5GH[6] = {0,0,0,0,3,0};//{0,0,0,0,1,0};
	u8 radio_id_5GL[6] = {0,0,0,0,1,1};//{0,0,0,0,2,0};//{0,0,0,0,3,0};
	u8 radio_id_2G[6] = {0,0,0,0,1,0};//{0,0,0,0,2,0};
	u8 almac[6] = {0x00,0x0c,0x43,0x49,0x7C,0x82};
	if (val == 1) {
		err("scan at controller");
		 almac[0] = 0x00;
		 almac[1] = 0x0c;
		 almac[2] = 0x43;
		 almac[3] = 0x49;
		 almac[4] = 0x76;
		 almac[5] = 0xfa;
		}
#endif

//add valid check if the ALMAC in the scan request is for a profile2 type agent
	_1905_device = topo_srv_get_1905_device(&global->dev,almac);
	if (!_1905_device) {
		err("This 1905 device almac is not valid");
		return -1;
	} else if (_1905_device->map_version != DEV_TYPE_R2){
		err("this ALMAC does not belong to a profile2 MAP dev");
		return -1;
	}

#if 1
	scan_req = (struct channel_scan_req *)buf;
	struct scan_body *body_temp = scan_req->body;

	scan_req->fresh_scan= 0x80;//hardcode fresh scan
	val2 = 1;//for fresh scan 
//the on boot only capability for this ALMAC is 0 only then AP can send Fresh scan =1 command
	if ((scan_req->fresh_scan) && (boot_only_scan == 1)) {
		err("controller can't send fresh scan req to agent with boot only scan cap");
		return -1;
	}
//check each radio in comand if it is supported or not
	scan_req->radio_num = 0;
	radio = topo_srv_get_radio(_1905_device , radio_id_2G);
	if (!radio) {
		err("this radio ID is not present");
	} else {
		scan_req->radio_num = scan_req->radio_num + 1;
		
		#if 0 
		os_memcpy(scan_req->body[0].radio_id, radio_id_2G, 6);
		if(val2 == 1) {
			scan_req->body[0].oper_class_num = 1;
			scan_req->body[0].ch_body[0].oper_class = 81;
			scan_req->body[0].ch_body[0].ch_list_num = 0;
		}
		#endif
		os_memcpy(body_temp->radio_id, radio_id_2G, 6);
		if(val2 == 1) {
			body_temp->oper_class_num = 1;
			body_temp->ch_body[0].oper_class = 81;
			body_temp->ch_body[0].ch_list_num = 0;
		}
	}

	radio = topo_srv_get_radio(_1905_device , radio_id_5GL);
	if (!radio) {
		err("this radio ID is not present");
	} else {
		err(" ");
		scan_req->radio_num = scan_req->radio_num + 1;
		u8 *temp = (u8 *)body_temp;
		temp = temp + sizeof(struct scan_body);
		body_temp = (struct scan_body *)temp;
		os_memcpy(body_temp->radio_id, radio_id_5GL, 6);
		if(val2 == 1) {
			body_temp->oper_class_num = 2;
			body_temp->ch_body[0].oper_class = 115;
			body_temp->ch_body[0].ch_list_num = 0;
			body_temp->ch_body[1].oper_class = 118;
			body_temp->ch_body[1].ch_list_num = 0;
		}
		#if 0 
		os_memcpy(scan_req->body[1].radio_id, radio_id_5GL, 6);
		err("val2 %d", val2);
		if(val2 == 1) {
			err(" ");
			scan_req->body[1].oper_class_num = 2;
			scan_req->body[1].ch_body[0].oper_class = 115;
			scan_req->body[1].ch_body[0].ch_list_num = 0;
			scan_req->body[1].ch_body[1].oper_class = 118;
			scan_req->body[1].ch_body[1].ch_list_num = 0;
		}
		#endif
	}
	radio = topo_srv_get_radio(_1905_device , radio_id_5GH);
	if (!radio) {
		err("this radio ID is not present");
	} else {
		scan_req->radio_num = scan_req->radio_num + 1;
		os_memcpy(scan_req->body[2].radio_id, radio_id_5GH, 6);
		if(val2 == 1) {
			scan_req->body[2].oper_class_num = 2;
			scan_req->body[2].ch_body[0].oper_class = 121;
			scan_req->body[2].ch_body[0].ch_list_num = 0;
			scan_req->body[2].ch_body[1].oper_class = 125;
			scan_req->body[2].ch_body[1].ch_list_num = 0;
		}
	}
	u8 i;
	for (i =0;i<3;i++) {
		err("RAID "MACSTR"",MAC2STR(scan_req->body[i].radio_id));
		err("op_num %d", scan_req->body[i].oper_class_num);
		err("ch_body[0].oper_class %d", scan_req->body[i].ch_body[0].oper_class);
		err("ch_body[1].oper_class %d", scan_req->body[i].ch_body[1].oper_class);
	}
	
	len = sizeof(struct channel_scan_req) + 3*sizeof(struct scan_request_lib);
//	printf("%s: scanReqLen = %d\n",__func__,
	mapd_hexdump(MSG_OFF, "scan req", (u8*)buf, 1000);
	
	if(((global->dev.ch_planning_R2.ch_plan_state != CHPLAN_STATE_IDLE) ||
		(global->dev.network_optimization.network_opt_state != NETOPT_STATE_IDLE))&&
		(global->dev.device_role == DEVICE_ROLE_CONTROLLER))
	{
		err("need to add to pending task list, defer this scan");
		struct channel_scan_req *scan_req_dup = os_zalloc(len);
		os_memcpy(scan_req_dup,scan_req,len);
		insert_into_task_list(&global->dev,TASK_USER_TRIGGERED_SCAN,NULL,scan_req_dup,almac);
		os_free(scan_req_dup);
	}
	else
	{
		if(val == 1) {
			u16 length;
			err("scan command is meant for owndev  itself");
			length = sizeof(struct channel_scan_req) + scan_req->radio_num*sizeof(struct scan_body);
			err("Len = %d\n", length);
			scan_req->neighbour_only = 2;//NB_ALL;
			map_get_info_from_wapp(&global->dev, WAPP_USER_SET_CHANNEL_SCAN_REQ, 0, NULL, NULL, (void *)buf, length);
		} else {
			
			mapd_hexdump(MSG_OFF, "agent's scan req", (u8*)buf, 1000);
			err(" scan_req->radio_num %d",  scan_req->radio_num);
			map_1905_Send_Channel_Scan_Request_Message(global->_1905_ctrl,(char *)almac,scan_req->fresh_scan, scan_req->radio_num, (unsigned char *)scan_req->body);
		}
		global->dev.user_triggered_scan = 1;
		// Start a 5 mins timer
		eloop_register_timeout(CH_SCAN_TIMEOUT, 0, ch_scan_req_timeout, global, _1905_device);
	}
#endif
	return 0;
	// fill send channel scan req
}

#endif
#endif

/* MAPD Code */
static int mapd_ctrl_iface_set(struct mapd_global *global, char *cmd)
{
	char *value;
	int ret = 0;

	value = os_strchr(cmd, ' ');
	if (value == NULL)
		return -1;
	*value++ = '\0';

	mapd_printf(MSG_INFO, "'%s'='%s'", cmd, value);
	if (os_strcasecmp(cmd, "disable_pre_assoc_strng") == 0) {
		ret = ap_roam_algo_disable_pre_assoc_strng(global, atoi(value));
	} else if (os_strcasecmp(cmd, "disable_post_assoc_strng") == 0) {
			ret = ap_roam_algo_disable_post_assoc_strng(global, atoi(value));
	} else if (os_strcasecmp(cmd, "disable_offloading") == 0) {
			ret = ap_roam_algo_disable_post_assoc_strng_by_type(global, atoi(value),
							OFFLOADING);
#ifdef SUPPORT_MULTI_AP
	} else if (os_strcasecmp(cmd, "disable_nolmultiap") == 0) {
			ret = ap_roam_algo_disable_post_assoc_strng_by_type(global, atoi(value),
							NOL_MULTIAP);
#endif
	} else if (os_strcasecmp(cmd, "disable_active_ug") == 0) {
			ret = ap_roam_algo_disable_post_assoc_strng_by_type(global, atoi(value),
							ACTIVE_STANDALONE_UG);
	} else if (os_strcasecmp(cmd, "disable_active_dg") == 0) {
			ret = ap_roam_algo_disable_post_assoc_strng_by_type(global, atoi(value),
							ACTIVE_STANDALONE_DG);
	} else if (os_strcasecmp(cmd, "disable_idle_ug") == 0) {
			ret = ap_roam_algo_disable_post_assoc_strng_by_type(global, atoi(value),
							IDLE_STANDALONE_UG);
	} else if (os_strcasecmp(cmd, "disable_idle_dg") == 0) {
			ret = ap_roam_algo_disable_post_assoc_strng_by_type(global, atoi(value),
							IDLE_STANDALONE_DG);
#ifdef SUPPORT_MULTI_AP
	} else if (os_strcasecmp(cmd, "user_preferred_channel") == 0) {
		ret = ch_planning_set_user_preff_ch(global, atoi(value));
		global->dev.ch_planning.ch_planning_enabled = 1;
#ifdef MAP_R2
		u8 cmd_channel = atoi(value);
		struct mapd_radio_info *own_radio = NULL;
		struct _1905_map_device *_1905_dev = topo_srv_get_1905_device(&global->dev,NULL);
		struct radio_info_db *radio = topo_srv_get_radio_by_band(_1905_dev,cmd_channel);
		if(radio) {
			own_radio = mapd_get_radio_from_channel(global,radio->channel[0]);
			err("channel %d, bootup_run %d", radio->channel[0],own_radio->bootup_run);
			if(own_radio->bootup_run == 0)
				own_radio->bootup_run = 1;

			global->dev.ch_planning_R2.ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
			ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED,atoi(value),&global->dev);
		}
#endif
#endif
	} else if (os_strcasecmp(cmd, "StrForbidTimeJoin") == 0) {
		global->dev.cli_steer_params.StrForbidTimeJoin = atoi(value);
		mapd_printf(MSG_OFF, "StrForbidTimeJoin set to %d secs",
					global->dev.cli_steer_params.StrForbidTimeJoin);
	} else if (os_strcasecmp(cmd, "reset_btm_csbc_at_join") == 0) {
		global->dev.cli_steer_params.reset_btm_csbc_at_join = atoi(value);
		mapd_printf(MSG_OFF, "reset_btm_csbc_at_join set to %d",
						global->dev.cli_steer_params.reset_btm_csbc_at_join);
		if (global->dev.cli_steer_params.reset_btm_csbc_at_join)
			mapd_printf(MSG_OFF,
						"WARNING: ****** BTM CSBC would be reset at join************");
	} else if (os_strcasecmp(cmd, "log_level") == 0) {
			int log_level = atoi(value);
			if ((log_level >= MSG_EXCESSIVE) &&
				(log_level <= MSG_ERROR)) {
				mapd_debug_level = log_level;
				mapd_printf(MSG_ERROR, "Log Level set to =%d", log_level);
			} else {
				ret = -1;
				mapd_printf(MSG_ERROR, "Invalid log_level");
			}
#ifdef SUPPORT_MULTI_AP
	} else if (os_strcasecmp(cmd, "Network_Opt_Enable") == 0) {
					global->dev.network_optimization.network_optimization_enabled = atoi(value);
		mapd_printf(MSG_ERROR, "Ntwrk Opt En =%d",
			global->dev.network_optimization.network_optimization_enabled);
			if(global->dev.network_optimization.network_optimization_enabled == 1) {
				network_opt_reset(global);
				global->dev.network_optimization.NetOptReason = REASON_ENABLED_BY_USER;
			}
			else
			{
				network_opt_reset(global);
			}

			update_ntwrk_opt_in_dat_file(global->dev.network_optimization.network_optimization_enabled);
	} else if (os_strcasecmp(cmd, "third_party_connection") == 0) {
			global->dev.ThirdPartyConnection = atoi(value);
			if(global->dev.ThirdPartyConnection) {
				wlanif_issue_wapp_command(global, WAPP_USER_GET_BH_WIRELESS_SETTING,
					WAPP_MAP_BH_CONFIG, NULL, NULL, NULL, 0, 0, 1, 0);
			}
			mapd_printf(MSG_ERROR,"third party connection set %d\n",global->dev.ThirdPartyConnection );
			ret = 0;
#ifdef MAP_R2
#ifdef SUPPORT_1905
	} else if (os_strcasecmp(cmd, "ch_scan") == 0) {
		ret = map_cmd_ch_scan_req(global, atoi(value));
	} else if (os_strcasecmp(cmd, "force_ch_planning_R2_monitor") == 0) {
		if(global->dev.device_role == DEVICE_ROLE_CONTROLLER) {
			struct _1905_map_device *temp_dev = topo_srv_get_1905_device(&global->dev, NULL);
			u8 temp_channel = atoi(value) ;
			global->dev.ch_planning_R2.ch_plan_enable = TRUE;
			ch_planning_R2_reset(&global->dev,NULL);
			struct radio_info_db *radio = topo_srv_get_radio_by_band(temp_dev, temp_channel);
			ch_planning_handle_metric_report(&global->dev,temp_dev,NULL,radio,0,1);
		}
	} else if (os_strcasecmp(cmd, "force_ch_planning_R2") == 0) {
		if(global->dev.device_role == DEVICE_ROLE_CONTROLLER) {
			u8 temp_channel = atoi(value) ;
			struct mapd_radio_info *radio_info = NULL;
			struct radio_info_db * radio_db = NULL;
			struct _1905_map_device *_1905_dev = topo_srv_get_1905_device(&global->dev, NULL);
			if(_1905_dev)
				radio_db = topo_srv_get_radio_by_band(_1905_dev, temp_channel);
			if(radio_db)
				radio_info = get_radio_info_by_radio_id(global,radio_db->identifier);
			if(radio_info){
				err("bootup run force done for ch %d", radio_info->channel);
				radio_info->bootup_run = 1;
			}
			ch_planning_reset_user_preff_ch(global);
			ch_planning_R2_force_trigger(global, temp_channel);
		}
	}else if (os_strcasecmp(cmd, "get_ch_plan_info") == 0) {
		if(global->dev.device_role == DEVICE_ROLE_CONTROLLER) {
			u8 cmd_param = atoi(value);
			dump_ch_planning_info(&global->dev, cmd_param);
		}
	} else if (os_strcasecmp(cmd, "ch_planning_R2") == 0) {
		global->dev.ch_planning_R2.ch_plan_enable = atoi(value);
		ch_planning_R2_reset(&global->dev,NULL);
		if (global->dev.ch_planning_R2.ch_plan_enable == TRUE &&
			global->dev.ch_planning.ch_planning_enabled == FALSE){
			global->dev.ch_planning.ch_planning_enabled = TRUE;
			err("R1 ch planning is mandatory for R2 CH planning");
		}
#endif
#endif
#endif
	} else {
			mapd_printf(MSG_ERROR, "Invalid SET cmd");
		ret = -1;
	}

	/* Restore cmd to its original value to allow redirection */
	value[-1] = ' ';

	return ret;
}

char * mapd_global_ctrl_iface_process(struct mapd_global *global,
		char *buf, size_t *resp_len)
{
	char *reply = NULL;
	const int reply_size = 8092;
	int reply_len = 0;
	int level = MSG_DEBUG;
#ifdef SUPPORT_MULTI_AP
	FILE *fptr;
	char *dump_topo_rply = NULL;
	unsigned int dump_topo_len  = 0;
#endif
	always("I got a commapd %s\n", buf);
	if (os_strcmp(buf, "PING") == 0)
		level = MSG_EXCESSIVE;
	mapd_hexdump_ascii(level, "RX global ctrl_iface",
			(const u8 *) buf, os_strlen(buf));

	reply = os_malloc(reply_size);
	if (reply == NULL) {
		*resp_len = 1;
		return NULL;
	}

	os_memcpy(reply, "OK\n", 3);
	reply_len = 3;

	if (os_strcmp(buf, "PING") == 0) {
		os_memcpy(reply, "PONG\n", 5);
		reply_len = 5;
#ifdef SUPPORT_MULTI_AP
	} else if (os_strcmp(buf, "GET role") == 0) {
			*reply=(char)global->dev.device_role;
			reply_len = 1;

	}else if (os_strncmp(buf, "SELECT BEST AP", 14) == 0){
		wlanif_disconnect_apcli(global, NULL);
		*reply=0;
		reply_len = 1;
	} else if (os_strncmp(buf, "SET rssith ", 11) == 0){
	
		reply_len=mapd_Set_RssiTh(global, buf + 11);
		if(reply_len<0)
		{
			mapd_printf(MSG_ERROR, "error in setting rssi threshold\n");
		}

		always("reply=%d\n", reply_len);
	}else if (os_strncmp(buf, "MANDATE_STEER ", 14) == 0){
	
		always("mandate steering msg request \n");
		reply_len=mapd_mandate_steer(global, buf + 14);
			if(reply_len<0)
			{
				mapd_printf(MSG_ERROR, "error in sending mandate steer Req\n");
			}
			always("reply=%d\n", reply_len);
	}else if (os_strncmp(buf, "BHSTEER ", 8) == 0){
        always("back haul steering msg request \n");
		reply_len=mapd_bh_steer(global, buf + 8);
		if(reply_len<0)
		{
			mapd_printf(MSG_ERROR, "error in sending BH steer Req\n");
		}
		always("reply=%d\n", reply_len);
	}else if (os_strncmp(buf, "force_ch_switch ", strlen("force_ch_switch ")) == 0) {
		unsigned char almac[ETH_ALEN] = {0} ;
		unsigned int channel[3]={0,0,0};
		char *pTmp=NULL,*token=NULL;
		int i = 0;
		struct _1905_map_device *target_1905 = NULL, *own_dev;
		reply_len=1;
		if (hwaddr_aton(buf+strlen("force_ch_switch "), almac) < 0) {
			always("Return -1 ALMAC parsing error");
			reply_len = -1;
		}
		else {
			always("force_ch_switch ALMAC %02x:%02x:%02x:%02x:%02x:%02x\n",almac[0],almac[1],almac[2],almac[3],almac[4],almac[5]);
			pTmp = buf+strlen("force_ch_switch ")+(ETH_ALEN*3);
			while ((token = strtok_r(pTmp, " ", &pTmp))) { 
				printf("%s\n", token); 
				channel[i]=atoi((const char *)token);
				i++;
			}
			always(" channel1  %d channel2 %d channel3 %d\n", channel[0], channel[1], channel[2]);
		target_1905 = topo_srv_get_1905_device(&global->dev, almac);
		own_dev = topo_srv_get_1905_device(&global->dev, NULL);
		if (!target_1905) {
			err("device with given almac not found\n");
			reply_len=-1;
		} else if((!target_1905->in_network)&&(own_dev!=target_1905)) {
			err("device is not connected\n");
			reply_len=-1;
		} else {
#ifdef MAP_R2
			struct radio_info_db *temp_radio = 0;
			if (global->dev.div_ch_planning == 0 && own_dev->map_version == DEV_TYPE_R2) {
				err("Divergent channel planning not allowed");
				reply_len = -1;
			} else {

				if(channel[0] != 0) {
					temp_radio = topo_srv_get_radio_by_band(target_1905, channel[0]);
					temp_radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
					debug("chnage state to trigger ch change for radio with ch %d", temp_radio->channel[0]);
				}

				global->dev.force_ch_change = 1; 
#endif
			ch_planning_exec(&global->dev, target_1905, channel);
#ifdef MAP_R2
			}
#endif
			}
		}
	}else if (os_strncmp(buf, "set_txpower_percentage ", os_strlen("set_txpower_percentage ")) == 0) {
		char *ptmp = NULL, *pvalue = NULL;
		unsigned char almac[ETH_ALEN], band = 0;
		unsigned int txpower_percentage = 100, i = 0;
		struct _1905_map_device *target_1905 = NULL, *own_dev;
		reply_len = 1;
		//os_strcpy(cmd, buf);
		ptmp = buf + os_strlen("set_txpower_percentage ");
		ptmp = strtok(ptmp, " ");
		for(i = 0, pvalue = strtok(ptmp, ":"); (pvalue && (i< ETH_ALEN)); i++, pvalue = strtok(NULL, ":")) {
			almac[i] = strtol(pvalue, &pvalue, 0x10);
		}
		ptmp = buf + os_strlen("set_txpower_percentage ") + (ETH_ALEN*3);
		band = strtol(ptmp, &ptmp, 10);
		txpower_percentage = strtol(ptmp, NULL, 10);
		target_1905 = topo_srv_get_1905_device(&global->dev, almac);
		own_dev = topo_srv_get_1905_device(&global->dev, NULL);
		if (!target_1905) {
			err("device with given almac not found\n");
			reply_len = -1;
		} else if((!target_1905->in_network)&&(own_dev!=target_1905)) {
			err("device is not connected\n");
			reply_len = -1;
		} else {
			ch_planning_set_txpower_percentage(&global->dev, target_1905, band, txpower_percentage);
		}
	}else if (os_strncmp(buf, "BH_CONN_STATUS", 14) == 0){
		always("get bh connection status\n");
		reply_len = mapd_Get_Bh_ConnectionStatus(&global->dev, reply, reply_size);
		always("replylength =%d, reply =%d\n", reply_len, reply[0]);
	}else if (os_strncmp(buf, "BH_CONN_TYPE", 12) == 0){
		always("get bh connection type\n");
		reply_len = mapd_Get_Bh_ConnectionType(&global->dev, reply, reply_size);
		always("replylength =%d, reply =%d\n", reply_len, reply[0]);
#endif
	}else if (os_strncmp(buf, "STEER_EN_DIS ", 13) == 0){
			*reply=0;
			reply_len = 1;
			if(atoi(buf+13)==0){
				global->dev.SetSteer=STEER_DISABLE;
			}else{
				global->dev.SetSteer=STEER_ENABLE;
			}
			always("Set Steering g_SetSteer %d\n",global->dev.SetSteer );
#ifdef SUPPORT_MULTI_AP
	}else if (os_strncmp(buf, "SET CH_Utilth ", 14) == 0){
	    always("SET CH_Utilth cmd received\n");
		reply_len=mapd_Set_ChUtilTh(global, buf + 14);
		if(reply_len<0)
		{
			mapd_printf(MSG_ERROR, "error in setting channel utilization threshold\n");
		}
		always("reply=%d\n", reply_len);
	}else if (os_strcmp(buf,"CONFIG_RENEW") == 0){
		always("Issue Config renew Request from controller\n");
		reply_len = mapd_send_config_renew(global);
		always("reply=%d\n",reply_len);
	}else if (os_strcmp(buf, "GET_BH_if_AP") == 0){
		always("Get Backhaul interface AP\n");
		reply_len = mapd_Get_BH_interfaceAP(global, reply, reply_size);
		always("ctrl_iface %s , reply=%d\n", reply, reply_len);
	}else if (os_strcmp(buf, "GET_FH_if_AP") == 0){
		always("Get Fronthaul interface AP\n");
		reply_len = mapd_Get_FH_interfaceAP(global, reply, reply_size);
		always("ctrl_iface %s , reply=%d\n", reply, reply_len);
	}else if (os_strcmp(buf, "DUMP_TOPO") == 0) {
		if (topo_srv_dump_topology(&global->dev))
			reply_len = -1;
	} else if (os_strcmp(buf, "GET_SCAN_RESULTS") == 0) {
		topo_srv_issue_scan(&global->dev);
	} else if (os_strcmp(buf, "ENABLE_CHANNEL_PLANNING") == 0) {
		err("Enable Channel Planning\n");
		global->dev.ch_planning.ch_planning_enabled = 1;
#ifdef MAP_R2
		ch_planning_reset_user_preff_ch(global);
		global->dev.ch_planning_R2.ch_plan_enable = 1;
		ch_planning_R2_reset(&global->dev,NULL);
#endif
	} else if (os_strcmp(buf, "DISABLE_CHANNEL_PLANNING") == 0) {
		err("Disable Channel Planning\n");
		global->dev.ch_planning.ch_planning_enabled = 0;
#ifdef MAP_R2
		global->dev.ch_planning_R2.ch_plan_enable = 0;
		ch_planning_R2_reset(&global->dev,NULL);
#endif
#endif
	} else if (os_strcmp(buf, "SAVE_DB") == 0) {
		if (mapd_client_db_flush(global, 1))
			reply_len = -1;
	} else if (os_strncmp(buf, "SET ", 4) == 0){
		if (mapd_ctrl_iface_set(global, buf + 4))
			reply_len = -1;
	} else if (os_strncmp(buf, "RESET_CSBC ", 11) == 0){
		mapd_printf(MSG_OFF, "Got cmd to reset csbc");
		reply_len = mapd_reset_csbc(global, buf + 11, reply, reply_size);
	} else if (os_strncmp(buf, "MIB ",4) == 0){
		always("MIB cmd \n");
		reply_len = mapd_get_mib_options(global, buf, reply, reply_size);
		always("reply=%d\n", reply_len);
	} else if (os_strcmp(buf, "MIB") == 0){
		always("MIB cmd \n");
		reply_len = mapd_get_mib(global, reply, reply_size);
		always("reply=%d\n", reply_len);
	} else if (os_strncmp(buf, "STUB",4) == 0) {
		always("STUB from wapp \n");
		mapd_handle_stub(global, buf, reply, reply_size);
		always("reply=%d\n", reply_len);
	} else if(os_strcmp(buf, "TRIGGER_STR_CANDS") == 0) {
		int cand_cnt = 0;
		struct client *arr_cand_list[MAX_STA_SEEN];
		ap_roam_algo_select_steer_candidate(global, arr_cand_list, &cand_cnt);
		always("reply=%d\n", reply_len);
#ifdef SUPPORT_MULTI_AP
	} else if(os_strncmp(buf, "dump_topology_v1", 16) == 0) {
		char file_path[30];
		int j = 0;
		dump_topo_len = 15000;
		dump_topo_rply = os_malloc(dump_topo_len);
		if(dump_topo_rply == NULL) {
			always("Error!");
			*resp_len = 1;
			os_free(reply);
			return NULL;
		}
		memset(dump_topo_rply, '\0', dump_topo_len);
		memset(file_path, '\0', 30);
		if (os_strlen(buf) > (19 + sizeof(file_path))) {
			always("return NULL size of array insufficient");
			os_free(dump_topo_rply);
			os_free(reply);
			return NULL;
		} else {
			while(buf[j + 19] != '\0') {
			file_path[j] = buf[j + 19];
			j++;
		}
		fptr = fopen(file_path, "w");
   		if(fptr == NULL) {
			always("Error!");
			os_free(dump_topo_rply);
			os_free(reply);
			return NULL;
		}
		always("dump_topology_v1\n");
		reply_len = topo_srv_dump_topology_v1(&global->dev, buf, dump_topo_rply, dump_topo_len);
		always("reply=%d\n", reply_len);
		fprintf(fptr, "%s", dump_topo_rply);
		fclose(fptr);
		os_free(dump_topo_rply);
		}
	} else if(os_strncmp(buf, "set_enrollee_bh", 15) == 0) {
		always("set_enrollee_bh\n");
		reply_len = mapd_set_enrollee_bh(global, buf + 16, reply, reply_size);
		always("reply=%d\n", reply_len);
	} else if(os_strncmp(buf, "set_bss_role", 12) == 0) {		
		always("set_bss_role\n");
		reply_len = mapd_set_bss_role(global, buf + 13, reply, reply_size);
		always("reply=%d\n", reply_len);
	} else if(os_strncmp(buf, "trigger_wps", 11) == 0) {
		always("trigger_wps\n");
		reply_len = mapd_trigger_wps(global, buf + 12, reply, reply_size);
		always("reply=%d\n", reply_len);
	} else if(os_strncmp(buf, "ap_selection_bh_steer", 21) == 0) {
		/*todo*/
		always("bh steer ap selection test\n");
		reply_len = mapd_trigger_ap_selection_bh(global, buf + 22, reply, reply_size);
		always("reply=%d\n", reply_len);
#endif
	} else if(os_strncmp(buf, "get_client_DB", 13) == 0) {
		always("get_client_DB\n");
		reply_len = mapd_get_client_db(global, buf + 14, reply, reply_size);
		always("reply=%d\n", reply_len);
#ifdef SUPPORT_MULTI_AP
	} else if(os_strncmp(buf, "restart_ch_planning", strlen("restart_ch_planning")) == 0) {
		mapd_restart_channel_plannig(global);
	}
	else if (os_strcmp(buf, "BH_INFO") == 0) {
		if (topo_srv_dump_bh_all_info(&global->dev))
			reply_len = -1;
	} else if (os_strcmp(buf, "STA_MED_INFO") == 0) {
		if (topo_srv_dump_sta_all_info(&global->dev))
			reply_len = -1;
	} else if(os_strncmp(buf, "set_acl_block", 13) == 0) {
		err("Set ACl block");
		reply_len = mapd_set_acl_block(global, buf + 14, reply, reply_size);
		always("reply=%d\n", reply_len);
	}
#ifdef ACL_CTRL
	else if(os_strncmp(buf, "set_acl_ctrl", 12) == 0) {
		err("Set ACl ctrl");
		reply_len = mapd_set_acl_ctrl(global, buf + 13, reply, reply_size);
		always("reply=%d\n", reply_len);
	} else if(os_strcmp(buf, "dump_agent_info") == 0) {
		err("dump_agent_info");
		if (topo_srv_dump_agent_info(&global->dev))
			reply_len = -1;
	}
#endif /*ACL_CTRL*/
	else if (os_strncmp(buf, "renew", strlen("renew")) == 0) {
		always("wts_bss_info_config file has been updated. do renew procedure\n");
		mapd_renew(global);
	} else if (os_strcmp(buf, "CONN_STATUS") == 0) {
			os_memcpy(reply, &global->dev.device_status, sizeof(wapp_device_status));
			always("FHBSS Status: %d, BHSTA Status: %d\n", global->dev.device_status.status_fhbss,
				global->dev.device_status.status_bhsta);
			reply_len = sizeof(wapp_device_status);
	} else if (os_strncmp(buf, "scan_thresh_2g", strlen("scan_thresh_2g")) == 0) {
			reply_len = mapd_set_scan_rssi_thresh(global, buf + 15, 0);
	} else if (os_strncmp(buf, "scan_thresh_5g", strlen("scan_thresh_5g")) == 0) {
			reply_len = mapd_set_scan_rssi_thresh(global, buf + 15, 1);
#endif
	} else if (os_strncmp(buf, "version_info", strlen("version_info")) == 0) {
			always("Build date: %s\n", current_version);
			always("Version: %s\n", VERSION_MAPD);
			os_memcpy(reply, current_version, strlen(current_version));
			reply_len = os_strlen(current_version);
#ifdef SUPPORT_MULTI_AP
	} else if(os_strncmp(buf, "bh_priority", strlen("bh_priority")) == 0) {
		err("Set BH priority");
		reply_len = mapd_set_bh_priority(global, buf + 12, reply, reply_size);
		always("reply=%d\n", reply_len);
	} else if(os_strncmp(buf, "bh_priority", strlen("bh_priority")) == 0) {
		err("Set BH priority");
		reply_len = mapd_set_bh_priority(global, buf + 12, reply, reply_size);
		always("reply=%d\n", reply_len);
	} else if (os_strncmp(buf,"off_ch_scan_req", strlen("off_ch_scan_req")) == 0) {
		reply_len = off_ch_scan_exec(&global->dev, buf, (unsigned char *)reply, 0);
	} else if (os_strncmp(buf,"get_ch_scan_noise", strlen("get_ch_scan_noise")) == 0) {
		reply_len = off_ch_scan_exec(&global->dev, buf, (unsigned char *)reply, 1);
	} else if (os_strncmp(buf,"off_ch_scan_result", strlen("off_ch_scan_result")) == 0) {
		unsigned char almac[64]= {0};
		struct _1905_map_device *target_1905=NULL;
		reply_len=1;
		sscanf(buf+strlen("off_ch_scan_result "), "%02x:%02x:%02x:%02x:%02x:%02x", (unsigned int*)almac,
				(unsigned int*)(almac + 1), (unsigned int*)(almac + 2),
				(unsigned int*)(almac + 3), (unsigned int*)(almac + 4),
				(unsigned int*)(almac + 5));
		target_1905 = topo_srv_get_1905_device(&global->dev, almac);
		if (!target_1905) {
			err("device with given almac not found\n");
			reply_len=-1;
		} else {
			if (target_1905->off_ch_scan_report) {
				reply_len = sizeof(struct off_ch_scan_report_event) +
					(sizeof(struct off_ch_scan_result_event) *
					target_1905->off_ch_scan_report->scan_result_num);
				os_memcpy(reply, target_1905->off_ch_scan_report, reply_len);
				dump_off_ch_scan_rep(target_1905->off_ch_scan_report);
			} else {
				os_memcpy(reply, "WAITING\n", os_strlen("WAITING\n"));
				reply_len = os_strlen("WAITING\n");
			}
		}
	}else if(os_strncmp(buf, "read_bh_config", strlen("read_bh_config")) == 0) {
		mapd_get_bh_config(global, buf, NULL, 0);
	} else if(os_strncmp(buf, "SET_BL_TIMEOUT", strlen("SET_BL_TIMEOUT")) == 0) {
		global->dev.bl_timeout = atoi(buf+strlen("SET_BL_TIMEOUT "));
		always("bl_timeout set to: %d", global->dev.bl_timeout);
		reply_len = 0;
	} else if(os_strncmp(buf, "metric_policy_set", strlen("metric_policy_set")) == 0) {
		reply_len = mapd_set_metric_policy_param(global, buf + strlen("metric_policy_set "));
	} else if (os_strncmp(buf, "tx_higher_layer_data ", os_strlen("tx_higher_layer_data ")) == 0) {

		always("In mapd: tx_higher_layer_data\n");

		char cmd[4096];
		char *ptmp = NULL, *pvalue = NULL;
		char almac[ETH_ALEN];
		unsigned int i = 0, protocol = 0, payload_len = 0;
		unsigned char *payload_buff = NULL;
		reply_len=3;

		os_strcpy(cmd, buf);
		ptmp = cmd + os_strlen("tx_higher_layer_data ");
		ptmp = strtok(ptmp, " ");
		for(i = 0, pvalue = strtok(ptmp, ":"); (pvalue && (i< ETH_ALEN)); i++, pvalue = strtok(NULL, ":")) {
			almac[i] = strtol(pvalue, &pvalue, 0x10);
		}

		ptmp = buf + os_strlen("tx_higher_layer_data ") + (ETH_ALEN*3);
		protocol = strtol(ptmp, &ptmp, 10);
		payload_len = strtol(ptmp, &ptmp, 10);

		always("protocol: %d, payload_len: %d\n", protocol, payload_len);

		payload_buff = (unsigned char *)ptmp+1;

		map_1905_Send_Higher_Layer_Date_Message(global->_1905_ctrl, almac, protocol, payload_len, payload_buff);
		always("reply=%d\n", reply_len);
#endif
	} else if (os_strncmp(buf, "bh_switch_cu_en", strlen("bh_switch_cu_en")) == 0) {
		reply_len = mapd_set_bh_switch_cu_en(global, buf + strlen("bh_switch_cu_en") +1);
	} else if (os_strncmp(buf, "cu_ol_count_thresh", strlen("cu_ol_count_thresh")) == 0) {
		reply_len = mapd_set_cu_maxcount_thresh(global, buf + strlen("cu_ol_count_thresh") +1);
	} else if (os_strncmp(buf, "bh_ol_forbid_time", strlen("bh_ol_forbid_time")) == 0) {
		reply_len = mapd_set_bh_cu_forbidtime_thresh(global, buf + strlen("bh_ol_forbid_time") +1);
#ifdef MAP_R2
	} else if (os_strncmp(buf, "METRIC_MSG", strlen("METRIC_MSG")) == 0) {
		trigger_metric_msg(global, buf+strlen("METRIC_MSG "));
	} else if (os_strncmp(buf, "send_cac_req_ter", strlen("send_cac_req_ter")) == 0) {
		trigger_cac_msg(global, buf+strlen("send_cac_req_ter "));
	} else if (os_strncmp(buf, "ch_sel_req", strlen("ch_sel_req")) == 0) {
		trigger_ch_sel_msg(global, buf+strlen("ch_sel_req "));
	} else if (os_strncmp(buf, "get_de_stats", strlen("get_de_stats")) == 0){
		reply_len = mapd_get_de_stats(global, buf + strlen("get_de_stats "));
		if(reply_len<0) {
			mapd_printf(MSG_ERROR, "error in getting DE stats\n");
		}
		always("reply=%d\n", reply_len);
	} else if (os_strncmp(buf, "CH_SCAN ", 8) == 0){
		always("ch scan msg request \n");
		reply_len = map_cmd_ch_scan_req_demo(global, buf + 8);
		if(reply_len<0)
		{
			mapd_printf(MSG_ERROR, "error in sending channel scan Req\n");
		}
		always("reply=%d\n", reply_len);
	} else if (os_strncmp(buf, "CH_PLAN_R2 ", 11) == 0){
		always("ch planning R2 request \n");
		reply_len = map_cmd_ch_plan_R2_demo(global, buf + 11);
		if(reply_len<0)
		{
			mapd_printf(MSG_ERROR, "error in sending channel planning R2 Req\n");
		}
		always("reply=%d\n", reply_len);
	} else if(os_strncmp(buf, "get_ch_plan_score_dump ", strlen("get_ch_plan_score_dump ")) == 0) {
		char file_path[30];
		int j = 0;
		int extra_len = strlen("get_ch_plan_score_dump ");
		dump_topo_len = 15000;
		dump_topo_rply = os_malloc(dump_topo_len);
		if(dump_topo_rply == NULL) {
			always("Error!");
			*resp_len = 1;
			os_free(reply);
			return NULL;
		}
		memset(dump_topo_rply, '\0', dump_topo_len);
		memset(file_path, '\0', 30);
		if (os_strlen(buf) > (extra_len + sizeof(file_path))) {
			always("return NULL size of array insufficient");
			os_free(dump_topo_rply);
			os_free(reply);
			return NULL;
		} else {
			while(buf[j + extra_len] != '\0') {
			file_path[j] = buf[j + extra_len];
			j++;
		}
		fptr = fopen(file_path, "w");
   		if(fptr == NULL) {
			always("Error!");
			os_free(dump_topo_rply);
			os_free(reply);
			return NULL;
		}
		always("Dump Ch plan score info\n");
		reply_len = dump_ch_plan_score_info(&global->dev, buf, dump_topo_rply, dump_topo_len);
		always("reply=%d\n", reply_len);
		fprintf(fptr, "%s", dump_topo_rply);
		fclose(fptr);
		os_free(dump_topo_rply);
		}
	}else if(os_strncmp(buf, "get_ch_scan_stats_dump ", strlen("get_ch_scan_stats_dump ")) == 0) {
		char file_path[30];
		int j = 0;
		int extra_len = strlen("get_ch_scan_stats_dump ");
		dump_topo_len = 15000;
		dump_topo_rply = os_malloc(dump_topo_len);
		if(dump_topo_rply == NULL) {
			always("Error!");
			*resp_len = 1;
			os_free(reply);
			return NULL;
		}
		memset(dump_topo_rply, '\0', dump_topo_len);
		memset(file_path, '\0', 30);
		if (os_strlen(buf) > (extra_len + sizeof(file_path))) {
			always("return NULL size of array insufficient");
			os_free(dump_topo_rply);
			os_free(reply);
			return NULL;
		} else {
			while(buf[j + extra_len] != '\0') {
			file_path[j] = buf[j + extra_len];
			j++;
		}
		fptr = fopen(file_path, "w");
   		if(fptr == NULL) {
			always("Error!");
			os_free(dump_topo_rply);
			os_free(reply);
			return NULL;
		}
		always("Dump Ch Scan\n");
		reply_len = dump_all_dev_scan_info(&global->dev, buf, dump_topo_rply, dump_topo_len);
		always("reply=%d\n", reply_len);
		fprintf(fptr, "%s", dump_topo_rply);
		fclose(fptr);
		os_free(dump_topo_rply);
		}
	} else if(os_strncmp(buf, "get_de_dump ", strlen("get_de_dump ")) == 0) {
		char file_path[30];
		int j = 0;
		int extra_len = strlen("get_de_dump ");
		err("extra len: %d", extra_len);
		dump_topo_len = 15000;
		dump_topo_rply = os_malloc(dump_topo_len);
		if(dump_topo_rply == NULL) {
			always("Error!");
			*resp_len = 1;
			os_free(reply);
			return NULL;
		}
		memset(dump_topo_rply, '\0', dump_topo_len);
		memset(file_path, '\0', 30);
		if (os_strlen(buf) > (extra_len + sizeof(file_path))) {
			always("return NULL size of array insufficient");
			os_free(dump_topo_rply);
			os_free(reply);
			return NULL;
		} else {
			while(buf[j + extra_len] != '\0') {
				file_path[j] = buf[j + extra_len];
				j++;
			}
			err("file path: %s", file_path);
			fptr = fopen(file_path, "w");
   			if(fptr == NULL) {
				always("Error!");
				os_free(dump_topo_rply);
				os_free(reply);
				return NULL;
			}
			always("Dump DE info\n");
			reply_len = dump_de(&global->dev, buf, dump_topo_rply, dump_topo_len);
			always("reply=%d\n", reply_len);
			fprintf(fptr, "%s", dump_topo_rply);
			fclose(fptr);
			os_free(dump_topo_rply);
		}
	}
	else if (os_strncmp(buf, "send_bh_sta_query", strlen("send_bh_sta_query")) == 0) {
		trigger_bh_sta_query(global, buf+strlen("send_bh_sta_query "));
#endif
	} else {
		always("UNKNOWN COMMAND\n");
		os_memcpy(reply, "UNKNOWN COMMAND\n", 16);
		reply_len = 16;
	}

	if (reply_len < 0) {
		os_memcpy(reply, "FAIL\n", 5);
		reply_len = 5;
	}
#if 0 
	if (reply_len > 4090) {
		reply_len = 4090;
		always("Dump is overflowed (length>4096)");
        }
#endif
	*resp_len = reply_len;
	return reply;
}
