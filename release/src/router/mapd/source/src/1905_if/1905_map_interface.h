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
 *  1905 wrapper
 *
 *  Abstract:
 *  1905 wrapper
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Kapil.Gupta 2018/05/02    First implementation of the 1905 wrapper
 * */

#ifndef _1905_MAP_INTERFACE_H
#define _1905_MAP_INTERFACE_H

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif				/* GNU_PACKED */
#include <linux/if_ether.h>
#include <net/if.h>

#define IN
#define OUT
#ifdef SUPPORT_1905
#include "_1905_interface_ctrl.h"
#ifdef MAP_R2
#include "./../1905_local_lib/data_def.h"
#endif
/**
 * map_1905_Init - Open a control interface to 1905.1 library
 * @daemon: current daemon name
 * @local_path: Path for UNIX domain sockets;
 * Returns: Pointer to abstract control interface data or %NULL on failure
 *
 * This function is used to open a control interface to wapp..
 */
static inline struct _1905_context *map_1905_Init(const char *local_path)
{
	return _1905_Init(local_path);
}

static inline Boolean is_1905_present()
{
	return TRUE;
}

/**
 * map_1905_Deinit - Close a control interface to 1905.1 library
 * @ctrl: Control interface data from _1905_init()
 *
 * This function is used to close a control interface.
 */
static inline void map_1905_Deinit(struct _1905_context *ctrl)
{
	return _1905_Deinit(ctrl);
}


/**
 * map_1905_receive - Receive a pending control interface message
 * @ctrl: Control interface data from _1905_init()
 * @reply: Buffer for the message data
 * @reply_len: Length of the reply buffer
 * Returns: 0 on success, -1 on failure
 *
 * This function will receive a pending control interface message. The received
 * response will be written to reply and reply_len is set to the actual length
 * of the reply.
 */
static inline int map_1905_Receive(struct _1905_context *ctrl, char *reply, size_t * reply_len)
{
	return _1905_Receive(ctrl, reply, reply_len);
}

static inline int map_1905_interface_ctrl_pending(struct _1905_context *ctrl, struct timeval *tv)
{
	return _1905_interface_ctrl_pending(ctrl, tv);
}

static inline int map_1905_interface_ctrl_get_fd(struct _1905_context *ctrl)
{
	return _1905_interface_ctrl_get_fd(ctrl);
}

static inline int map_1905_Set_Role(IN struct _1905_context *ctx, enum MAP_ROLE role)
{
	return _1905_Set_Role(ctx, role);
}

static inline int map_1905_Set_Radio_Basic_Cap(IN struct _1905_context *ctx, IN struct ap_radio_basic_cap *cap)
{
	return _1905_Set_Radio_Basic_Cap(ctx, cap);
}

static inline int map_1905_Set_Channel_Preference_Report_Info(IN struct _1905_context *ctx, IN int ch_prefer_cnt,
		IN struct ch_prefer_lib *ch_prefers,
		IN int restriction_cnt,
		IN struct restriction_lib *restrictions,
		IN struct cac_completion_report  *cac_rep
#ifdef DFS_CAC_R2
		,
		IN struct cac_status_report  *cac_status_rep
#endif
		)
{
	return _1905_Set_Channel_Preference_Report_Info(ctx, ch_prefer_cnt, ch_prefers, restriction_cnt, restrictions, (struct cac_completion_report_lib *)cac_rep
#ifdef DFS_CAC_R2
		, (struct cac_status_report_lib *)cac_status_rep
#endif
		);
}

static inline int map_1905_Set_Channel_Selection_Rsp_Info(IN struct _1905_context *ctx,
		IN struct ch_sel_rsp_info *rsp_info, IN int rsp_cnt)
{
	return _1905_Set_Channel_Selection_Rsp_Info(ctx, rsp_info, rsp_cnt);
}

static inline int map_1905_Set_Ap_Cap(IN struct _1905_context *ctx, IN struct ap_capability *cap)
{
	return _1905_Set_Ap_Cap(ctx, cap);
}

static inline int map_1905_Set_Ap_Ht_Cap(IN struct _1905_context *ctx, IN struct ap_ht_capability *cap)
{
	return _1905_Set_Ap_Ht_Cap(ctx, cap);
}

static inline int map_1905_Set_Ap_Vht_Cap(IN struct _1905_context *ctx, IN struct ap_vht_capability *cap)
{
	return _1905_Set_Ap_Vht_Cap(ctx, cap);
}

static inline int map_1905_Set_Ap_He_Cap(IN struct _1905_context *ctx, IN struct ap_he_capability *cap)
{
	return _1905_Set_Ap_He_Cap(ctx, cap);
}

static inline int map_1905_Set_Operbss_Cap(IN struct _1905_context *ctx, IN struct oper_bss_cap *cap)
{
	return _1905_Set_Operbss_Cap(ctx, cap);
}

#ifdef MAP_R2
static inline int map_1905_Set_Channel_Scan_Cap(IN struct _1905_context *ctx, IN struct radio_scan_capab *cap)
{
	return _1905_Set_Channel_Scan_Cap(ctx, (struct scan_capability_lib *)cap);
}

static inline int map_1905_Set_R2_AP_Cap(IN struct _1905_context *ctx, IN struct ap_r2_capability *cap)
{
	return _1905_Set_R2_AP_Cap(ctx, (struct ap_r2_capability *)cap);
}


static inline int map_1905_Send_Channel_Scan_Report_Message (
		IN struct _1905_context* ctx, char* almac,
		IN unsigned char ts_len, IN unsigned char *ts_str,
		IN int scan_res_cnt, IN unsigned char *scan_res, IN unsigned char *ch_util_info, IN unsigned char ch_util_len)

{
	return _1905_Send_Channel_Scan_Report_Message(ctx, almac, ts_len, ts_str, scan_res_cnt, scan_res, ch_util_info, ch_util_len);
}

static inline int map_1905_Send_Channel_Scan_Request_Message (
	IN struct _1905_context* ctx, char* almac,
	IN unsigned char fresh_scan, IN int radio_cnt,
	IN unsigned char *scan_req) 
{
	return _1905_Send_Channel_Scan_Request_Message(ctx, almac, fresh_scan, radio_cnt, (struct scan_request_lib *)scan_req);
}

static inline int map_1905_Send_Assoc_Status_Notification_Message(
		IN struct _1905_context * ctx,char * almac,
	    IN struct assoc_notification * assoc_notification)

{
	return _1905_Send_Assoc_Status_Notification_Message(ctx, almac, (struct assoc_notification_lib *)assoc_notification);

}

static inline int map_1905_Send_Tunneled_Message (
		IN struct _1905_context * ctx,char * almac,
	    IN struct tunneled_msg * tunneled_message)
{
	return _1905_Send_Tunneled_Message(ctx, almac, (struct tunneled_message_lib *)tunneled_message);
}

#ifdef DFS_CAC_R2
static inline int map_1905_Set_CAC_Cap(IN struct _1905_context *ctx, IN struct cac_capability * cap, IN unsigned short len)
{
	return _1905_Set_CAC_Cap(ctx, (struct cac_capability_lib *)cap, (int)len);
}

static inline int map_1905_Send_CAC_Request(IN struct _1905_context *ctx, char *almac, IN struct cac_request * req)
{

	return _1905_Send_CAC_Request_Message(ctx, almac, (struct cac_request_lib *)req);
}

static inline int map_1905_Send_CAC_Terminate(IN struct _1905_context *ctx, char *almac, IN struct cac_terminate * term)
{
	return _1905_Send_CAC_Terminate_Message(ctx, almac, (struct cac_terminate_lib *)term);
}


#endif
static inline int map_1905_Send_Metric_collection_interval_cap(IN struct _1905_context *ctx, IN u32 *met_rep_intv)
{
	//printf("map_1905_Send_Metric_collection_interval_cap %d\n", *met_rep_intv);
	//return 0;//_1905_Send_Metric_collection_interval_cap(ctx, met_rep_intv); SPS
	return _1905_Set_Metric_collection_interval(ctx, *met_rep_intv);
}
static inline int map_1905_send_bh_sta_cap_query(IN struct _1905_context *ctx, char *almac)
{
	return _1905_Send_BH_Sta_Cap_Query(ctx, almac);
}

#endif

static inline int map_1905_Set_Cli_Steer_BTM_Report_Info(IN struct _1905_context *ctx,
		IN struct cli_steer_btm_event *info)
{
	return _1905_Set_Cli_Steer_BTM_Report_Info(ctx, info);
}

static inline int map_1905_Set_Steering_Complete_Info(IN struct _1905_context *ctx)
{
	return _1905_Set_Steering_Complete_Info(ctx);
}

static inline int map_1905_Set_Read_Bss_Conf_Request(IN struct _1905_context *ctx)
{
	return _1905_Set_Read_Bss_Conf_Request(ctx);
}

static inline int map_1905_Set_Read_Bss_Conf_and_Renew(IN struct _1905_context *ctx, IN unsigned char local_only)
{
	return _1905_Set_Read_Bss_Conf_and_Renew(ctx, local_only);
}

static inline int map_1905_Set_Read_Bss_Conf_and_Renew_v2(IN struct _1905_context *ctx, IN unsigned char local_only)
{
	return _1905_Set_Read_Bss_Conf_and_Renew_v2(ctx, local_only);
}

#ifdef MAP_R2
static inline int map_1905_Set_Ap_Metric_Rsp_Info(IN struct _1905_context *ctx, IN struct ap_metrics_info_lib *info,
		IN int ap_metrics_info_cnt, IN struct stat_info *sta_states,
		IN int sta_states_cnt, IN struct link_metrics *sta_metrics,
		IN int sta_metrics_cnt, IN struct ap_extended_metrics_lib *ext_ap_metrics,
		IN int ext_ap_met_cnt, IN struct sta_extended_metrics_lib *ext_sta_metric,
		IN int ext_sta_met_cnt, IN struct radio_metrics_lib *info_radio,
		IN int radio_metrics_info_cnt, IN struct ch_util_lib *ch_util, IN int ch_util_len)
{
	printf("\next_ap_met_cnt: %d, ext_sta_met_cnt: %d, radio cnt: %d, ap metric cnt: %d, sta metrics cnt: %d, ch_util_len %d\n",
		ext_ap_met_cnt, ext_sta_met_cnt, radio_metrics_info_cnt, ap_metrics_info_cnt, sta_metrics_cnt, ch_util_len);
	return _1905_Set_Ap_Metric_Rsp_Info(ctx, info, ap_metrics_info_cnt, sta_states, sta_states_cnt, sta_metrics,
			sta_metrics_cnt, ext_ap_metrics, ext_ap_met_cnt, info_radio, radio_metrics_info_cnt,
			(struct sta_extended_metrics_lib *)ext_sta_metric, ext_sta_met_cnt, (unsigned char *)ch_util, ch_util_len);
}
#else
static inline int map_1905_Set_Ap_Metric_Rsp_Info(IN struct _1905_context *ctx, IN struct ap_metrics_info_lib *info,
		IN int ap_metrics_info_cnt, IN struct stat_info *sta_states,
		IN int sta_states_cnt, IN struct link_metrics *sta_metrics,
		IN int sta_metrics_cnt)
{
	return _1905_Set_Ap_Metric_Rsp_Info(ctx, info, ap_metrics_info_cnt, sta_states, sta_states_cnt, sta_metrics,
			sta_metrics_cnt);
}

#endif

static inline int map_1905_Set_Bh_Steer_Rsp_Info(IN struct _1905_context *ctx, IN struct backhaul_steer_rsp *info)
{
	return _1905_Set_Bh_Steer_Rsp_Info(ctx, info);
}

// TODO: add logic for error code tlv
#ifdef MAP_R2
static inline int map_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(IN struct _1905_context *ctx, IN unsigned char info_cnt,
		IN struct link_metrics *info, IN unsigned char ext_info_cnt, IN struct sta_extended_metrics_lib *ext_info, IN unsigned char *sta_mac, IN unsigned char Reason)
{
	return _1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(ctx, info_cnt, info, sta_mac, Reason, ext_info_cnt, ext_info);
}

#else

static inline int map_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(IN struct _1905_context *ctx, IN int info_cnt,
		IN struct link_metrics *info, IN unsigned char *sta_mac, IN unsigned char Reason)
{
	return _1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(ctx, info_cnt, info, sta_mac, Reason);
}

#endif

static inline int map_1905_Set_Link_Metrics_Rsp_Info(IN struct _1905_context *ctx, IN int tx_metrics_cnt,
		IN struct tx_link_metrics *tx_metrics, IN int rx_metrics_cnt,
		IN struct rx_link_metrics *rx_metrics)
{
	return _1905_Set_Link_Metrics_Rsp_Info(ctx, tx_metrics_cnt, tx_metrics, rx_metrics_cnt, rx_metrics);
}

static inline int map_1905_Set_Unassoc_Sta_Link_Metric_Rsp_Info(IN struct _1905_context *ctx,
		IN struct unlink_metrics_rsp *info)
{
	return _1905_Set_Unassoc_Sta_Link_Metric_Rsp_Info(ctx, info);
}

static inline int map_1905_Set_Operating_Channel_Report_Info(IN struct _1905_context *ctx,
		IN struct channel_report *info)
{
	return _1905_Set_Operating_Channel_Report_Info(ctx, info);
}

static inline int map_1905_Set_Beacon_Metrics_Report_Info(IN struct _1905_context *ctx,
		IN struct beacon_metrics_rsp_lib *info)
{
	return _1905_Set_Beacon_Metrics_Report_Info(ctx, info);
}

static inline int map_1905_Set_Sta_Notification_Info(IN struct _1905_context *ctx,
		IN struct client_association_event *info)
{
	return _1905_Set_Sta_Notification_Info(ctx, info);
}
#ifdef MAP_R2
static inline int map_1905_Send_disassoc_sta_stats_message(IN struct _1905_context *ctx,
		IN u8 *al_mac, IN u16 reason_code, IN struct stat_info *stats)
{
	printf("client disassoc evt to 1905");
	return _1905_Send_Client_Disassociation_Stats_Message(ctx, (char *)al_mac, reason_code, stats);
}
#endif

static inline int map_1905_Set_Bh_Ready(IN struct _1905_context *ctx, struct bh_link_info *bh_info)
{
	return _1905_Set_Bh_Ready(ctx, bh_info);
}

static inline int map_1905_Get_Wsc_Config(IN struct _1905_context *ctx, struct wps_get_config *config)
{
	return _1905_Get_Wsc_Config(ctx, config);
}

static inline int map_1905_Set_Read_1905_Tlv_Req(IN struct _1905_context *ctx, char *file_path, int len)
{
	return _1905_Set_Read_1905_Tlv_Req(ctx, file_path, len);
}

static inline int map_1905_Send_Topology_Query_Message(IN struct _1905_context *ctx, char *almac)
{
	return _1905_Send_Topology_Query_Message(ctx, almac);
}

static inline int map_1905_Send_Link_Metric_Query_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char neighbor, char *neighbor_almac,
		unsigned char metrics)
{
	return _1905_Send_Link_Metric_Query_Message(ctx, almac, neighbor, neighbor_almac, metrics);
}

static inline int map_1905_Send_AP_Autoconfig_Search_Message(IN struct _1905_context *ctx,
		unsigned char band)
{
	return _1905_Send_AP_Autoconfig_Search_Message(ctx, band);
}

static inline int map_1905_Send_AP_autoconfig_Renew_Message(IN struct _1905_context *ctx,
		unsigned char band)
{
	return _1905_Send_AP_autoconfig_Renew_Message(ctx, band);
}

static inline int map_1905_Send_Vendor_Specific_Message(IN struct _1905_context *ctx, char *almac,
		char *vend_spec_tlv, unsigned short len)
{
	return _1905_Send_Vendor_Specific_Message(ctx, almac, vend_spec_tlv, len);
}

static inline int map_1905_Send_AP_Capability_Query_Message(IN struct _1905_context *ctx, char *almac)
{
	return _1905_Send_AP_Capability_Query_Message(ctx, almac);
}

static inline int map_1905_Send_MAP_Policy_Request_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char steer_disallow_sta_cnt,
		char *steer_disallow_sta_list,
		unsigned char btm_disallow_sta_cnt,
		char *btm_disallow_sta_list,
		unsigned char radio_cnt_steer,
		struct lib_steer_radio_policy *steering_policy,
		unsigned char ap_rep_interval,
		unsigned char radio_cnt_metrics,
		struct lib_metrics_radio_policy *metrics_policy
#ifdef MAP_R2
		, unsigned char scan_rep_include
		, unsigned char scan_rep_policy
		, unsigned char assoc_policy_include
		, struct lib_unsuccess_assoc_policy *assoc_policy
#endif
                )
{
	// SPS Hack
#ifdef MAP_R2
	return _1905_Send_MAP_Policy_Request_Message(ctx, (char *)almac, steer_disallow_sta_cnt, steer_disallow_sta_list,
			btm_disallow_sta_cnt, btm_disallow_sta_list, radio_cnt_steer,
			steering_policy, ap_rep_interval, radio_cnt_metrics,
			metrics_policy, scan_rep_include, scan_rep_policy, assoc_policy_include, assoc_policy);
#else
	return _1905_Send_MAP_Policy_Request_Message(ctx, (char *)almac, steer_disallow_sta_cnt, steer_disallow_sta_list,
			btm_disallow_sta_cnt, btm_disallow_sta_list, radio_cnt_steer,
			steering_policy, ap_rep_interval, radio_cnt_metrics,
			metrics_policy);
#endif

}

static inline int map_1905_Send_Channel_Preference_Query_Message(IN struct _1905_context *ctx, char *almac)
{
	return _1905_Send_Channel_Preference_Query_Message(ctx, almac);
}

static inline int map_1905_Send_Channel_Selection_Request_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char ch_prefer_cnt,
		struct ch_prefer_lib *prefer,
		unsigned char transmit_power_cnt,
		struct transmit_power_limit *power_limit)
{
	return _1905_Send_Channel_Selection_Request_Message(ctx, almac, ch_prefer_cnt, prefer, transmit_power_cnt,
			power_limit);
}

static inline int map_1905_Send_Client_Capability_Query_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char *bssid, unsigned char *sta_mac)
{
	return _1905_Send_Client_Capability_Query_Message(ctx, almac, bssid, sta_mac);
}

#ifdef MAP_R2
static inline int map_1905_Send_AP_Metrics_Query_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char bssid_cnt, unsigned char *bssid_list, unsigned char radio_cnt, unsigned char *radio_list)
{
	return _1905_Send_AP_Metrics_Query_Message(ctx, almac, bssid_cnt, bssid_list, radio_cnt, radio_list);
}
#else
static inline int map_1905_Send_AP_Metrics_Query_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char bssid_cnt, unsigned char *bssid_list)
{
	return _1905_Send_AP_Metrics_Query_Message(ctx, almac, bssid_cnt, bssid_list);
}
#endif

static inline int map_1905_Send_Associated_STA_Link_Metrics_Query_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char *sta_mac)
{
	return _1905_Send_Associated_STA_Link_Metrics_Query_Message(ctx, almac, sta_mac);
}

static inline int map_1905_Send_Unassociated_STA_Link_Metrics_Query_Message(IN struct _1905_context *ctx,
		char *almac,
		struct unassoc_sta_link_metrics_query
		*query)
{
	return _1905_Send_Unassociated_STA_Link_Metrics_Query_Message(ctx, almac, query);
}

static inline int map_1905_Send_Beacon_Metrics_Query_Message(IN struct _1905_context* ctx, char* almac,
		struct beacon_metrics_query *query)
{
	return _1905_Send_Beacon_Metrics_Query_Message(ctx, almac, query);
}

static inline int map_1905_Send_Combined_Infrastructure_Metrics_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char ap_metrics_cnt,
		struct GNU_PACKED ap_metrics_info_lib *
		metrics_info, unsigned char bh_link_cnt,
		struct tx_link_metrics *tx_metrics_bh_ap,
		struct tx_link_metrics *tx_metrics_bh_sta,
		struct rx_link_metrics *rx_metrics_bh_ap,
		struct rx_link_metrics *rx_metrics_bh_sta)
{
	return _1905_Send_Combined_Infrastructure_Metrics_Message(ctx, almac, ap_metrics_cnt, metrics_info,
			bh_link_cnt, tx_metrics_bh_ap, tx_metrics_bh_sta,
			rx_metrics_bh_ap, rx_metrics_bh_sta);
}

static inline int map_1905_Send_Client_Steering_Request_Message(IN struct _1905_context *ctx, char *almac,
		struct lib_steer_request *request,
		unsigned char tbss_cnt,
		struct lib_target_bssid_info *info
#ifdef MAP_R2
		, struct lib_steer_request_R2 *request_R2,
		unsigned char tbss_cnt_R2,
		struct lib_target_bssid_info_R2 *info_R2
#endif
		)
		
{
#ifdef MAP_R2
	return _1905_Send_Client_Steering_Request_Message(ctx, almac, request, tbss_cnt, info,
		request_R2, tbss_cnt_R2, info_R2);
#else	
	return _1905_Send_Client_Steering_Request_Message(ctx, almac, request, tbss_cnt, info);
#endif
}

static inline int map_1905_Send_Client_Association_Control_Request_Message(IN struct _1905_context *ctx,
		char *almac, unsigned char *bssid,
		unsigned char control,
		unsigned short valid_time,
		unsigned char sta_cnt,
		unsigned char *sta_list)
{
	return _1905_Send_Client_Association_Control_Request_Message(ctx, almac, bssid, control, valid_time, sta_cnt,
			sta_list);
}

static inline int map_1905_Send_Higher_Layer_Date_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char protocol, unsigned short len,
		unsigned char *payload)
{
	return _1905_Send_Higher_Layer_Data_Message(ctx, almac, protocol, len, payload);
}

static inline int map_1905_Send_Backhaul_Steering_Request_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char *sta_mac, unsigned char *bssid,
		unsigned char opclass, unsigned char channel)
{
	return _1905_Send_Backhaul_Steering_Request_Message(ctx, almac, sta_mac, bssid, opclass, channel);
}

static inline int map_1905_Get_Local_Devinfo(IN struct _1905_context *ctx, OUT struct device_info *dev_info,
		OUT struct bridge_cap *br_cap, OUT struct supported_srv *srv)
{
	return _1905_Get_Local_Devinfo(ctx, dev_info, br_cap, srv);
}
static inline int map_1905_Set_Wi_Bh_Link_Down(IN struct _1905_context *ctx, struct bh_link_info *bh_info)
{
	return _1905_Set_Wi_Bh_Link_Down(ctx, bh_info);
}
static inline int map_1905_Set_Channel_BandWidth(IN struct _1905_context *ctx,
	struct channel_bw_info *ch_bw_info)
{
	return _1905_Set_ch_bw_info(ctx, ch_bw_info);
}

static inline int map_1905_Set_Wireless_Interface_Info(IN struct _1905_context *ctx, IN struct interface_info_list_hdr *info)
{
	return _1905_Set_Wireless_Interface_Info(ctx, info);
}

#ifdef MAP_R2
static inline int map_1905_Send_Failed_Assoc_message(IN struct _1905_context *ctx, char *almac, u8 *sta_mac_address,
		u16 assoc_sts_cd, u16 assoc_reason_code)
{
	return _1905_Send_Failed_Connection_message(ctx, (char *)almac, (char *)sta_mac_address, assoc_sts_cd, assoc_reason_code);
}
#endif
static inline int map_1905_Clear_Switch_Table(IN struct _1905_context *ctx)
{
	return _1905_clear_switch_table(ctx);
}

#else

enum MAP_ROLE {
    MAP_CONTROLLER = 0,
    MAP_AGENT,
};

struct GNU_PACKED _1905_context
{
    char* name;
    int s;
    struct sockaddr_un local;
    struct sockaddr_un dest;
};

/**
 * map_1905_Init - Open a control interface to 1905.1 library
 * @daemon: current daemon name
 * @local_path: Path for UNIX domain sockets;
 * Returns: Pointer to abstract control interface data or %NULL on failure
 *
 * This function is used to open a control interface to wapp..
 */
static inline struct _1905_context *map_1905_Init(const char *local_path)
{
	return NULL;
}


/**
 * map_1905_Deinit - Close a control interface to 1905.1 library
 * @ctrl: Control interface data from _1905_init()
 *
 * This function is used to close a control interface.
 */
static inline void map_1905_Deinit(struct _1905_context *ctrl)
{
	return;
}


/**
 * map_1905_receive - Receive a pending control interface message
 * @ctrl: Control interface data from _1905_init()
 * @reply: Buffer for the message data
 * @reply_len: Length of the reply buffer
 * Returns: 0 on success, -1 on failure
 *
 * This function will receive a pending control interface message. The received
 * response will be written to reply and reply_len is set to the actual length
 * of the reply.
 */
static inline int map_1905_Receive(struct _1905_context *ctrl, char *reply, size_t * reply_len)
{
	return 0;
}

static inline int map_1905_interface_ctrl_pending(struct _1905_context *ctrl, struct timeval *tv)
{
	return 0;
}

static inline int map_1905_interface_ctrl_get_fd(struct _1905_context *ctrl)
{
	return 0;
}

static inline int map_1905_Set_Radio_Basic_Cap(IN struct _1905_context *ctx, IN struct ap_radio_basic_cap *cap)
{
	return 0;
}

static inline int map_1905_Set_Role(IN struct _1905_context *ctx, enum MAP_ROLE role)
{
	return 0;
}

static inline int map_1905_Set_Channel_Preference_Report_Info(IN struct _1905_context *ctx, IN int ch_prefer_cnt,
		IN struct ch_prefer_lib *ch_prefers,
		IN int restriction_cnt,
		IN struct restriction_lib *restrictions)
{
	return 0;
}

static inline int map_1905_Set_Channel_Selection_Rsp_Info(IN struct _1905_context *ctx,
		IN struct ch_sel_rsp_info *rsp_info, IN int rsp_cnt)
{
	return 0;
}

static inline int map_1905_Set_Ap_Cap(IN struct _1905_context *ctx, IN struct ap_capability *cap)
{
	return 0;
}

static inline int map_1905_Set_Ap_Ht_Cap(IN struct _1905_context *ctx, IN struct ap_ht_capability *cap)
{
	return 0;
}

static inline int map_1905_Set_Ap_Vht_Cap(IN struct _1905_context *ctx, IN struct ap_vht_capability *cap)
{
	return 0;
}

static inline int map_1905_Set_Ap_He_Cap(IN struct _1905_context *ctx, IN struct ap_he_capability *cap)
{
	return 0;
}

static inline int map_1905_Set_Operbss_Cap(IN struct _1905_context *ctx, IN struct oper_bss_cap *cap)
{
	return 0;
}
#ifdef MAP_R2
static inline int map_1905_Set_Channel_Scan_Cap(IN struct _1905_context *ctx, IN struct radio_scan_capab *cap)
{
	//return _1905_Set_Operbss_Cap(ctx, cap);
	// TODO: Raghav: Once 1905 is ready
	return 0;
}
static inline int map_1905_Set_R2_AP_Cap(IN struct _1905_context *ctx, IN struct ap_r2_capability *cap)
{
	return 0;
}
static inline int map_1905_Send_Channel_Scan_Report_Message (
		IN struct _1905_context* ctx, char* almac,
		IN unsigned char ts_len, IN unsigned char *ts_str,
		IN int scan_res_cnt, IN unsigned char *scan_res, IN unsigned char *ch_util_info, IN unsigned char ch_util_len)
{
	return 0;//_1905_Send_Channel_Scan_Report_Message(ctx, almac, ts_len, ts_str, scan_res_cnt, scan_res);
}
static inline int map_1905_Send_Channel_Scan_Request_Message (
	IN struct _1905_context* ctx, char* almac,
	IN unsigned char fresh_scan, IN int radio_cnt,
	IN unsigned char *scan_req) 
{
	return 0;//_1905_Send_Channel_Scan_Request_Message(ctx, almac, fresh_scan, radio_cnt, scan_req);
}

static inline int map_1905_Send_Assoc_Status_Notification_Message(
		IN struct _1905_context * ctx,char * almac,
	    IN struct assoc_notification *assoc_notification)

{
	return 0;

}

static inline int map_1905_Send_Tunneled_Message (
		IN struct _1905_context * ctx,char * almac,
	    IN struct tunneled_msg *tunneled_message)
{
	return 0;
}

#ifdef DFS_CAC_R2
static inline int map_1905_Set_CAC_Cap(
	IN struct _1905_context *ctx,
	IN struct cac_capability *cap,
	IN int len)
{
	return 0;
}
static inline int map_1905_Send_CAC_Request(IN struct _1905_context *ctx, char *almac, IN struct cac_request * req)
{
	return 0;
}

static inline int map_1905_Send_CAC_Terminate(IN struct _1905_context *ctx, char *almac, IN struct cac_terminate * term)
{
	return 0;
}

static inline int map_1905_Send_Metric_collection_interval_cap(IN struct _1905_context *ctx, IN u32 *met_rep_intv)
{
	return 0;
}
static inline int map_1905_send_bh_sta_cap_query(IN struct _1905_context *ctx, char *almac)
{
	return 0;
}

#endif
#endif
static inline int map_1905_Set_Cli_Steer_BTM_Report_Info(IN struct _1905_context *ctx,
		IN struct cli_steer_btm_event *info)
{
	return 0;
}

static inline int map_1905_Set_Steering_Complete_Info(IN struct _1905_context *ctx)
{
	return 0;
}

static inline int map_1905_Set_Read_Bss_Conf_Request(IN struct _1905_context *ctx)
{
	return 0;
}

static inline int map_1905_Set_Read_Bss_Conf_and_Renew(IN struct _1905_context *ctx, IN unsigned char local_only)
{
	return 0;
}

static inline int map_1905_Set_Read_Bss_Conf_and_Renew_v2(IN struct _1905_context *ctx, IN unsigned char local_only)
{
	return 0;
}


#ifdef MAP_R2
static inline int map_1905_Set_Ap_Metric_Rsp_Info(IN struct _1905_context *ctx, IN struct ap_metrics_info_lib *info,
		IN int ap_metrics_info_cnt, IN struct stat_info *sta_states,
		IN int sta_states_cnt, IN struct link_metrics *sta_metrics,
		IN int sta_metrics_cnt, IN struct ap_ext_metrics_info_lib *ext_ap_metrics,
		IN int ext_ap_met_cnt, IN struct sta_extended_metrics_lib *ext_sta_metric,
		IN int ext_sta_met_cnt, IN struct radio_metrics_lib *info_radio,
		IN int radio_metrics_info_cnt, struct ch_util_lib *ch_util, IN int ch_util_len)
{
	return 0;
}
#else

static inline int map_1905_Set_Ap_Metric_Rsp_Info(IN struct _1905_context *ctx, IN struct ap_metrics_info_lib *info,
		IN int ap_metrics_info_cnt, IN struct stat_info *sta_states,
		IN int sta_states_cnt, IN struct link_metrics *sta_metrics,
		IN int sta_metrics_cnt)
{
	return 0;
}
#endif
static inline int map_1905_Set_Bh_Steer_Rsp_Info(IN struct _1905_context *ctx, IN struct backhaul_steer_rsp *info)
{
	return 0;
}

#ifdef MAP_R2
static inline int map_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(IN struct _1905_context *ctx, IN unsigned char info_cnt,
		IN struct link_metrics *info, IN unsigned char ext_info_cnt, IN struct sta_extended_metrics_lib *ext_info, IN unsigned char *sta_mac, IN unsigned char Reason)
{
	return 0;//_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(ctx, info_cnt, info,ext_info_cnt, ext_info, sta_mac, Reason);
}

#else

static inline int map_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(IN struct _1905_context *ctx, IN int info_cnt,
		IN struct link_metrics *info, IN unsigned char *sta_mac, IN unsigned char Reason)
{
	return 0;
}
#endif

static inline int map_1905_Set_Link_Metrics_Rsp_Info(IN struct _1905_context *ctx, IN int tx_metrics_cnt,
		IN struct tx_link_metrics *tx_metrics, IN int rx_metrics_cnt,
		IN struct rx_link_metrics *rx_metrics)
{
	return 0;
}

static inline int map_1905_Set_Unassoc_Sta_Link_Metric_Rsp_Info(IN struct _1905_context *ctx,
		IN struct unlink_metrics_rsp *info)
{
	return 0;
}

static inline int map_1905_Set_Operating_Channel_Report_Info(IN struct _1905_context *ctx,
		IN struct channel_report *info)
{
	return 0;
}

static inline int map_1905_Set_Beacon_Metrics_Report_Info(IN struct _1905_context *ctx,
		IN struct beacon_metrics_rsp_lib *info)
{
	return 0;
}

static inline int map_1905_Set_Sta_Notification_Info(IN struct _1905_context *ctx,
		IN struct client_association_event *info)
{
	return 0;
}
#ifdef MAP_R2
static inline int map_1905_Send_disassoc_sta_stats_message(IN struct _1905_context *ctx,
		IN u8 *al_mac, IN u16 reason_code, IN struct stat_info *stats)
{
	return 0;
}
#endif


static inline int map_1905_Set_Bh_Ready(IN struct _1905_context *ctx, struct bh_link_info *bh_info)
{
	return 0;
}

static inline int map_1905_Get_Wsc_Config(IN struct _1905_context *ctx, struct wps_get_config *config)
{
	return 0;
}

static inline int map_1905_Set_Read_1905_Tlv_Req(IN struct _1905_context *ctx, char *file_path, int len)
{
	return 0;
}

static inline int map_1905_Send_Topology_Query_Message(IN struct _1905_context *ctx, char *almac)
{
	return 0;
}

static inline int map_1905_Send_Link_Metric_Query_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char neighbor, char *neighbor_almac,
		unsigned char metrics)
{
	return 0;
}

static inline int map_1905_Send_AP_Autoconfig_Search_Message(IN struct _1905_context *ctx,
		unsigned char band)
{
	return 0;
}

static inline int map_1905_Send_AP_autoconfig_Renew_Message(IN struct _1905_context *ctx,
		unsigned char band)
{
	return 0;
}

static inline int map_1905_Send_Vendor_Specific_Message(IN struct _1905_context *ctx, char *almac,
		char *vend_spec_tlv, unsigned short len)
{
	return 0;
}

static inline int map_1905_Send_AP_Capability_Query_Message(IN struct _1905_context *ctx, char *almac)
{
	return 0;
}

static inline int map_1905_Send_MAP_Policy_Request_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char steer_disallow_sta_cnt,
		char *steer_disallow_sta_list,
		unsigned char btm_disallow_sta_cnt,
		char *btm_disallow_sta_list,
		unsigned char radio_cnt_steer,
		struct lib_steer_radio_policy *steering_policy,
		unsigned char ap_rep_interval,
		unsigned char radio_cnt_metrics,
		struct lib_metrics_radio_policy *metrics_policy
#ifdef MAP_R2
		, unsigned char scan_rep_include
		, unsigned char scan_rep_policy
		, unsigned char assoc_policy_include
		, struct lib_unsuccess_assoc_policy *assoc_policy
#endif
)
{
	return 0;
}

static inline int map_1905_Send_Channel_Preference_Query_Message(IN struct _1905_context *ctx, char *almac)
{
	return 0;
}

static inline int map_1905_Send_Channel_Selection_Request_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char ch_prefer_cnt,
		struct ch_prefer_lib *prefer,
		unsigned char transmit_power_cnt,
		struct transmit_power_limit *power_limit)
{
	return 0;
}

static inline int map_1905_Send_Client_Capability_Query_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char *bssid, unsigned char *sta_mac)
{
	return 0;
}
#ifdef MAP_R2
static inline int map_1905_Send_AP_Metrics_Query_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char bssid_cnt, unsigned char *bssid_list, unsigned char radio_cnt, unsigned char *radio_list)
{
	return 0;
}
#else
static inline int map_1905_Send_AP_Metrics_Query_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char bssid_cnt, unsigned char *bssid_list)
{
	return 0;
}
#endif
static inline int map_1905_Send_Associated_STA_Link_Metrics_Query_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char *sta_mac)
{
	return 0;
}

static inline int map_1905_Send_Unassociated_STA_Link_Metrics_Query_Message(IN struct _1905_context *ctx,
		char *almac,
		struct unassoc_sta_link_metrics_query
		*query)
{
	return 0;
}

static inline int map_1905_Send_Beacon_Metrics_Query_Message(IN struct _1905_context* ctx, char* almac,
		struct beacon_metrics_query *query)
{

	return 0;
}
static inline int map_1905_Send_Combined_Infrastructure_Metrics_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char ap_metrics_cnt,
		struct GNU_PACKED ap_metrics_info_lib *
		metrics_info, unsigned char bh_link_cnt,
		struct tx_link_metrics *tx_metrics_bh_ap,
		struct tx_link_metrics *tx_metrics_bh_sta,
		struct rx_link_metrics *rx_metrics_bh_ap,
		struct rx_link_metrics *rx_metrics_bh_sta)
{
	return 0;
}

static inline int map_1905_Send_Client_Steering_Request_Message(IN struct _1905_context *ctx, char *almac,
		struct lib_steer_request *request,
		unsigned char tbss_cnt,
		struct lib_target_bssid_info *info
#ifdef MAP_R2
		, struct lib_steer_request_R2 *request_R2,
		unsigned char tbss_cnt_R2,
		struct lib_target_bssid_info_R2 *info_R2
#endif
		)
{
	return 0;
}

static inline int map_1905_Send_Client_Association_Control_Request_Message(IN struct _1905_context *ctx,
		char *almac, unsigned char *bssid,
		unsigned char control,
		unsigned short valid_time,
		unsigned char sta_cnt,
		unsigned char *sta_list)
{
	return 0;
}

static inline int map_1905_Send_Higher_Layer_Date_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char protocol, unsigned short len,
		unsigned char *payload)
{
	return 0;
}

static inline int map_1905_Send_Backhaul_Steering_Request_Message(IN struct _1905_context *ctx, char *almac,
		unsigned char *sta_mac, unsigned char *bssid,
		unsigned char opclass, unsigned char channel)
{
	return 0;
}

static inline Boolean is_1905_present()
{
	return FALSE;
}

static inline int map_1905_Get_Local_Devinfo(IN struct _1905_context *ctx, OUT struct device_info *dev_info,
		OUT struct bridge_cap *br_cap, OUT struct supported_srv *srv)
{
	return 0xff;
}
#ifdef MAP_R2
static inline int map_1905_Send_Failed_Assoc_message(IN struct _1905_context *ctx, char *almac, u8 *sta_mac_address,
		u16 assoc_sts_cd, u16 assoc_reason_code)
{
	return 0;
}
#endif

static inline int map_1905_Set_Wi_Bh_Link_Down(IN struct _1905_context *ctx, struct bh_link_info *bh_info)
{
	return FALSE;
}

static inline int map_1905_Set_Channel_BandWidth(IN struct _1905_context *ctx,
	struct channel_bw_info *ch_bw_info)
{
	return FALSE;
}

static inline int map_1905_Set_Wireless_Interface_Info(IN struct _1905_context *ctx, IN struct interface_info_list_hdr *info)
{
	return 0;
}

static inline int map_1905_Clear_Switch_Table(IN struct _1905_context *ctx)
{
	return FALSE;
}

#endif
#endif				/*  */
