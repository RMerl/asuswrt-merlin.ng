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

#ifndef _1905_INTERFACE_CTRL_H
#define _1905_INTERFACE_CTRL_H

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */
#include <linux/if_ether.h>
#include <net/if.h>


#define IN
#define OUT
#define INOUT

enum {
	/*for map agent*/
	_1905_SET_WIRELESS_SETTING_EVENT =0x0001,
	_1905_RECV_CHANNEL_PREFERENCE_QUERY_EVENT,
	_1905_RECV_CHANNEL_SELECTION_REQ_EVENT,
	_1905_RECV_STEERING_REQUEST_EVENT,
	_1905_RECV_CLIENT_ASSOC_CNTRL_SETTING_EVENT,
	_1905_RECV_POLICY_CONFIG_REQUEST_EVENT,
	_1905_RECV_AP_METRICS_QUERY_EVENT,
	_1905_RECV_BACKHAUL_STEER_REQ_EVENT,
	_1905_RECV_ASSOC_STA_LINK_METRICS_QUERY_EVENT,
	_1905_RECV_LINK_METRICS_QUERY,
	_1905_RECV_UNASSOC_STA_LINK_METRICS_QUERY_EVENT,
	_1905_SET_RADIO_TEARED_DOWN_EVENT,
	_1905_RECV_BEACON_METRICS_QUERY_EVENT,
	_1905_MAP_CONTROLLER_FOUND_EVENT,
	_1905_AUTOCONFIG_RENEW_EVENT,
	_1905_AUTOCONFIG_SEARCH_EVENT,
	_1905_AUTOCONFIG_RSP_EVENT,
	/*for map controller*/
	_1905_RECV_TOPOLOGY_RSP_EVENT,
	_1905_RECV_TOPOLOGY_NOTIFICATION_EVENT,
	_1905_RECV_CHANNEL_PREFERENCE_REPORT_EVENT,
	_1905_RECV_CLI_STEER_BTM_REPORT_EVENT,
	_1905_RECV_STEER_COMPLETE_EVENT,
	_1905_RECV_LINK_METRICS_RSP_EVENT,
	_1905_RECV_AP_METRICS_RSP_EVENT,
	_1905_RECV_ASSOC_STA_LINK_METRICS_RSP_EVENT,
	_1905_RECV_UNASSOC_STA_LINK_METRICS_RSP_EVENT,
	_1905_RECV_BCN_METRICS_RSP_EVENT,
	_1905_RECV_BACKHAUL_STEER_RSP_EVENT,
	_1905_RECV_COMBINED_INFRASTRUCTURE_METRICS_EVENT,
	_1905_RECV_VENDOR_SPECIFIC_MESSAGE_EVENT,
	_1905_RECV_AP_CAPABILITY_REPORT_EVENT,
	_1905_RECV_CH_SELECTION_RSP_EVENT,
	_1905_RECV_OPERATING_CH_REPORT_EVENT,
	_1905_RECV_CLIENT_CAPABILITY_REPORT_EVENT,
	_1905_RECV_HIGHER_LAYER_DATA_EVENT,
	_1905_RECV_COMBINED_INFRASTRUCTURE_METRICS_QUERY_EVENT,
	/*common*/
	_1905_SWITCH_STATUS,
#ifdef MAP_R2
	/*channel scan feature*/
	_1905_RECV_CHANNEL_SCAN_REQ_EVENT,
	_1905_RECV_CHANNEL_SCAN_REP_EVENT,
	_1905_RECV_TUNNELED_MESSAGE_EVENT,
	_1905_RECV_ASSOCIATION_STATUS_NOTIFICATION_EVENT,
	_1905_RECV_CAC_REQUEST_EVENT,
	_1905_RECV_CAC_TERMINATE_EVENT,
	_1905_RECV_CLIENT_DISASSOCIATION_STATS_EVENT,
	_1905_RECV_BACKHAUL_STA_CAP_REPORT_EVENT,
	_1905_RECV_FAILED_ASSOCIATION_EVENT,
#endif
#if defined(MAP_R2) || defined(CENT_STR)
	_1905_RECV_AP_METRICS_QUERY_PERIODIC_EVENT,
#endif
#ifdef MAP_R2
	_1905_SET_TRAFFIC_SEPARATION_SETTING_EVENT,
	_1905_SET_TRANSPARENT_VLAN_SETTING_EVENT,
#endif // #ifdef MAP_R2
	_1905_RECV_ERROR_CODE_EVENT,

};

enum MAP_ROLE {
	MAP_CONTROLLER = 0,
	MAP_AGENT,
};

/**
 * struct _1905_context - Internal structure
 *
 * This structure is used by the daemon to store internal data. Programs 
 * using the library should not touch this data directly. They can only use
 * the pointer to the data structure as an identifier for the control interface 
 * connection and use this as one of
 * the arguments for most of the control interface library functions.
 */

struct GNU_PACKED _1905_context
{
	char* name;
	int s;
	struct sockaddr_un local;
	struct sockaddr_un dest;
};

/**
 * _1905_Init - init the 1905.1 library
 * @local_path: Path for UNIX domain sockets;
 * Returns: Pointer to _1905_context or %NULL on failure
 *
 * This function is used to initilize the 1905.1 library.
 */

struct _1905_context * _1905_Init(IN const char *local_path);


/**
 * _1905_Deinit - deinit the 1905.1 library
 * @ctrl: _1905_context data from _1905_init()
 *
 * This function is used to deinitialize the 1905.1 library, usually in the user of
 * deinitialization function, it should call this function.
 */

void _1905_Deinit(IN struct _1905_context *ctrl);


/**
 * _1905_receive - Receive an event from 1905 library
 * @ctrl: _1905_context data from _1905_init()
 * @reply: Buffer for the message data
 * @reply_len: Length of the reply buffer
 * Returns: 0 on success, -1 on failure
 *
 * This function will receive an event from 1905 service. The received
 * event buffer will be written to reply and reply_len is set to the actual length
 * of the reply.
 * note: the reply_len is a INOUT arguement, for IN, it must equal to the size of receiving buffer,
 * for OUT, the reply_len will be set to the acture length of the event buffer.
 */
 
int _1905_Receive(IN struct _1905_context *ctrl, OUT char *reply, INOUT size_t *reply_len);

/**
 * _1905_interface_ctrl_pending - Check whether there are pending event messages
 * @ctrl: _1905_context data from _1905_init()
 * @tv: pending time, if tv equal to NULL, this function will block until any message received
 * Returns: 1 if there are pending messages, 0 if timeout, or -1 on error
 *
 * This function will check whether there are any pending control interface
 * message available to be received with _1905_Receive(). _1905_interface_ctrl_pending() is
 * only used for event messages.
 */
int _1905_interface_ctrl_pending(IN struct _1905_context *ctrl, IN struct timeval *tv);

/**
 * _1905_interface_ctrl_get_fd - Get file descriptor of _1905_context
 * @ctrl: _1905_context data from _1905_init()
 * Returns: File descriptor used for the connection
 *
 * This function can be used to get the file descriptor that is used for the
 * control interface connection. The returned value can be used, e.g., with
 * select() while waiting for multiple events.
 *
 * The returned file descriptor must not be used directly for sending or
 * receiving packets
 */
int _1905_interface_ctrl_get_fd(IN struct _1905_context *ctrl);

/**
 * _1905_Set_Role - set role of 1905 service
 * @ctx: _1905_context data from _1905_init()
 * @role: the role attempted to set to 1905 daemon
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to dynamically change role of 1905 service, note that only 
 * MAP agent-->MAP controller role changing is permitted
 *
 */
int _1905_Set_Role(IN struct _1905_context *ctx, IN enum MAP_ROLE role);

/**
 * _1905_Get_Local_Devinfo - get local information about 1905
 * @ctx: _1905_context data from _1905_init()
 * @dev_info: including local al mac address and local network device infomation
 * @br_cap: including number of local bridge and local network device belongs to bridge
 * @srv: including supported services of 1905 service
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to get local 1905 information which may be used to maintain the 1905
 * network topology, the corresponding infomation will be set to the dev_info, br_cap and srv,
 * so it must guarantee that the buffers that these three pointer pointing to is large enough to contain
 * the whole infomation.
 */
int _1905_Get_Local_Devinfo(IN struct _1905_context *ctx, OUT struct device_info *dev_info, 
	OUT struct bridge_cap *br_cap, OUT struct supported_srv *srv);

/**
 * _1905_Set_Radio_Basic_Cap - set capability to 1905 service
 * @ctx: _1905_context data from _1905_init()
 * @cap:  including the ap radio basic capability infomation
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to set the ap radio basic capability to 1905 service, usually before 
 * and after 1905 onboarding/autoconfig procedure, it is reqeust that calling this function to 
 * set this capability to 1905 service, and also it is permitted that calling this function when 
 * the user of 1905.1 library think it is neccessary to set this capability to 1905 service.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Set_Radio_Basic_Cap (IN struct _1905_context* ctx, 
	IN struct ap_radio_basic_cap * cap);

/**
 * _1905_Set_Ap_Cap - set capability to 1905 service
 * @ctx: _1905_context data from _1905_init()
 * @cap:  including the MAP ap capability infomation
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to set the MAP ap capability to 1905 service, usually before 
 * and after 1905 onboarding/autoconfig procedure, it is reqeust that calling this function to 
 * set this capability to 1905 service, and also it is permitted that calling this function when 
 * the user of 1905.1 library think it is neccessary to set this capability to 1905 service.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Set_Ap_Cap (IN struct _1905_context* ctx, 
	IN struct ap_capability *cap);

/**
 * _1905_Set_Ap_Cap - set capability to 1905 service
 * @ctx: _1905_context data from _1905_init()
 * @cap:  including the MAP ap ht capability infomation
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to set the MAP ap ht capability to 1905 service, usually before 
 * and after 1905 onboarding/autoconfig procedure, it is reqeust that calling this function to 
 * set this capability to 1905 service, and also it is permitted that calling this function when 
 * the user of 1905.1 library think it is neccessary to set this capability to 1905 service.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Set_Ap_Ht_Cap (IN struct _1905_context* ctx, 
	IN struct ap_ht_capability *cap);

/**
 * _1905_Set_Ap_Vht_Cap - set capability to 1905 service
 * @ctx: _1905_context data from _1905_init()
 * @cap:  including the MAP ap vht capability infomation
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to set the MAP ap vht capability to 1905 service, usually before 
 * and after 1905 onboarding/autoconfig procedure, it is reqeust that calling this function to 
 * set this capability to 1905 service, and also it is permitted that calling this function when 
 * the user of 1905.1 library think it is neccessary to set this capability to 1905 service.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Set_Ap_Vht_Cap (IN struct _1905_context* ctx,
	IN struct ap_vht_capability *cap);


/**
 * _1905_Set_Ap_He_Cap - set capability to 1905 service
 * @ctx: _1905_context data from _1905_init()
 * @cap:  including the MAP ap he capability infomation
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to set the MAP ap vht capability to 1905 service, usually before
 * and after 1905 onboarding/autoconfig procedure, it is reqeust that calling this function to
 * set this capability to 1905 service, and also it is permitted that calling this function when
 * the user of 1905.1 library think it is neccessary to set this capability to 1905 service.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Set_Ap_He_Cap (IN struct _1905_context* ctx,
	IN struct ap_he_capability *cap);



/**
 * _1905_Set_Operbss_Cap - set capability to 1905 service
 * @ctx: _1905_context data from _1905_init()
 * @cap:  including the MAP operational bss infomation
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to set the MAP operational bss to 1905 service, usually before 
 * and after 1905 onboarding/autoconfig procedure, it is reqeust that calling this function to 
 * set this capability to 1905 service, and also it is permitted that calling this function when 
 * the user of 1905.1 library think it is neccessary to set this capability to 1905 service.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Set_Operbss_Cap(IN struct _1905_context* ctx, 
	IN struct oper_bss_cap *cap);

/**
 * _1905_Set_Bh_Ready - notify backhaul link ready
 * @ctx: _1905_context data from _1905_init()
 * @info:  including the backhaul link infomation
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to notify 1905 service that the backhaul link is ready, then 1905
 * service will start to search the MAP controller by using 1905 autoconfig search cmdu to
 * find the MAP controller(it indicates that the autoconfig procedure starts), and when 1905 
 * service find the MAP controller, the user of 1905.1 library will receive 
 * _1905_MAP_CONTROLLER_FOUND_EVENT.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Set_Bh_Ready (IN struct _1905_context* ctx, struct bh_link_info *info);

/**
 * _1905_Set_Wi_Bh_Disconnect - notify the wireless backhaul link down
 * @ctx: _1905_context data from _1905_init()
 * @info:  including the backhaul link infomation
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to notify 1905 service that the wireless backhaul link is down, 1905 will update
 * its topology database of this wireless backhaul interface.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Set_Wi_Bh_Link_Down (IN struct _1905_context* ctx, struct bh_link_info *info);
int _1905_Set_ch_bw_info (IN struct _1905_context* ctx, struct channel_bw_info 
*info);

/**
 * _1905_Get_Own_Topo_Rsp - query own _1905_RECV_TOPOLOGY_RSP_EVENT event from 1905daemon
 * @ctx: _1905_context data from _1905_init()
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to query own _1905_RECV_TOPOLOGY_RSP_EVENT from 1905 service.
 */
int _1905_Get_Own_Topo_Rsp (IN struct _1905_context* ctx);

/**
 * _1905_Get_Wsc_Config - get the wireless setting from autoconfig procedure
 * @ctx: _1905_context data from _1905_init()
 * @config:  the band infomation of current radio that need to get the wireless setting
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to trigger the 1905 service to start the M1/M2 procedure to exchange
 * info. Usually when the user of 1905.1 library receives _1905_MAP_CONTROLLER_FOUND_EVENT,
 * it should call this function, and when 1905.1 service get the wireless setting from M2, the user of
 * 1905.1 library will receive the _1905_SET_WIRELESS_SETTING_EVENT.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Get_Wsc_Config (IN struct _1905_context* ctx, struct wps_get_config *config);

/**
 * _1905_Set_Channel_Preference_Report_Info - set the information of 
 * channel preference report
 * @ctx: _1905_context data from _1905_init()
 * @ch_prefer_cnt: the number of ch_prefers
 * @ch_prefers: channel preference information
 * @restriction_cnt: the number of restrictions
 * @restrictions: radio restriction restriction information
 * @cac_rep: cac completion report information, if pointer is NULL, no need to add in Channel preference Report.
 * @cac_status_rep: cac status report information, if pointer is NULL, no need to add in Channel preference Report.
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to fill the MAP channel preference report message, usually when 
 * the user of 1905.1 library receives _1905_RECV_CHANNEL_PREFERENCE_QUERY_EVENT,
 * it should call this functio to reply the channel preference report message.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Set_Channel_Preference_Report_Info (IN struct _1905_context* ctx,
	IN int ch_prefer_cnt, IN struct ch_prefer_lib * ch_prefers,
	IN int restriction_cnt, IN struct restriction_lib *restrictions
	, IN struct cac_completion_report_lib  *cac_rep
#ifdef MAP_R2
	, 
	IN struct cac_status_report_lib  *cac_status_rep
#endif
	);

/**
 * _1905_Set_Channel_Selection_Rsp_Info - set the information of 
 * channel selection response
 * @ctx: _1905_context data from _1905_init()
 * @rsp_info: channel selection response information
 * @rsp_cnt: the number of struct ch_sel_rsp_info
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to fill the MAP channel selection response message, usually when 
 * the user of 1905.1 library receives _1905_RECV_CHANNEL_SELECTION_REQ_EVENT,
 * it should call this function to reply the channel selection response message.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Set_Channel_Selection_Rsp_Info (IN struct _1905_context* ctx, 
	IN struct ch_sel_rsp_info *rsp_info, IN unsigned char rsp_cnt);

/**
 * _1905_Set_Operating_Channel_Report_Info - set the information of 
 * operating channel report
 * @ctx: _1905_context data from _1905_init()
 * @info: the infomation of operating channel
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to fill the MAP operating channel report message, usually when the
 * user of 1905.1 library receives _1905_RECV_CHANNEL_SELECTION_REQ_EVENT and reply the 
 * channel selection response indicating acceptance of MAP controller's requrst by calling 
 * _1905_Set_Channel_Selection_Rsp_Info, it should call this function to reply the operating 
 * channel report message.
 * note:this api is usually used when act as MAP agent
 */
int _1905_Set_Operating_Channel_Report_Info (IN struct _1905_context* ctx,
	IN struct channel_report *info);

/**
 * _1905_Set_Cli_Steer_BTM_Report_Info - set the information of 
 * client steering btm report message
 * @ctx: _1905_context data from _1905_init()
 * @info: client steering btm report
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to fill the MAP client steering btm report message, usually when 
 * the user of 1905.1 library receives _1905_RECV_STEERING_REQUEST_EVENT,
 * it should call this function to reply the client steering request if the MAP agent determines to
 * use btm reqeust method to steer client when MAP agent receives the btm response from the
 * station.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Set_Cli_Steer_BTM_Report_Info (IN struct _1905_context* ctx, 
	IN struct  cli_steer_btm_event *info);

/**
 * _1905_Set_Steering_Complete_Info - set the information of 
 * client steering complete message
 * @ctx: _1905_context data from _1905_init()
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to fill the MAP client steering complete message, usually when 
 * the user of 1905.1 library receives _1905_RECV_STEERING_REQUEST_EVENT,
 * it should call this function to reply the client steering request in which the Reuqest 
 * Mode bit set to zero indicating a Steering Opportunity.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Set_Steering_Complete_Info (IN struct _1905_context* ctx);

/**
 * _1905_Set_Ap_Metric_Rsp_Info - set the information of 
 * ap metrics response message
 * @ctx: _1905_context data from _1905_init()
 * @info: the infomation of ap metrics
 * @ap_metrics_info_cnt: the number of array the info pointing to
 * @sta_states: the information of associated station traffic stats
 * @sta_states_cnt: the number of arrary the sta_states pointing to
 * @sta_metrics: the information of associated station link metrics
 * @sta_metrics_cnt: the number of array the sta_metrics pointing to
 * @ap_extended_metrics: ap extended metrics
 * @ap_extended_metrics_cnt: num of ap extened metrics
 * @radio_metrics: radio metrics
 * @radio_metrics_cnt: num of radio metrics
 * @sta_extended_metrics: sta extended metrics
 * @sta_extended_metrics_cnt: num of sta extended metrics
 * @vs_tlv: any vendor specific tlv
 * @vs_len: length of vendor specific tlv
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to fill the MAP ap metcis response message, usually when 
 * the user of 1905.1 library receives _1905_RECV_AP_METRICS_QUERY_EVENT,
 * it should call this function to reply the ap metcis response message.
 * note:this api is usually used when act as MAP agent
 */
int _1905_Set_Ap_Metric_Rsp_Info (IN struct _1905_context* ctx, 
	IN struct ap_metrics_info_lib *info, IN int ap_metrics_info_cnt,
	IN struct stat_info *sta_states, IN int sta_states_cnt,
	IN struct link_metrics *sta_metrics, IN int sta_metrics_cnt
#ifdef MAP_R2
	, IN struct ap_extended_metrics_lib *ap_extended_metrics, IN int ap_extended_metrics_cnt,
	IN struct radio_metrics_lib *radio_metrics, IN int radio_metrics_cnt,
	IN struct sta_extended_metrics_lib *sta_extended_metrics, IN int sta_extended_metrics_cnt,
	IN unsigned char *vs_tlv, IN unsigned int vs_len
#endif // #ifdef MAP_R2
	);

/**
 * _1905_Set_Bh_Steer_Rsp_Info - set the information of 
 * backhaul steering response
 * @ctx: _1905_context data from _1905_init()
 * @info: the infomation of backhaul steering response
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to fill the MAP backhaul steering response message, usually when 
 * the user of 1905.1 library receives _1905_RECV_BACKHAUL_STEER_REQ_EVENT,
 * it should call this function to reply the backhaul steering response message.
 * note:this api is usually used when act as MAP agent
 */
int _1905_Set_Bh_Steer_Rsp_Info (IN struct _1905_context* ctx,
	IN struct backhaul_steer_rsp *info);

/**
 * _1905_Set_Link_Metrics_Rsp_Info - set the information of 
 * 1905 link metrics
 * @ctx: _1905_context data from _1905_init()
 * @tx_metrics_cnt: the number of array the tx_metrics pointing to
 * @tx_metrics: the infomation of transmitter link metrics
 * @rx_metrics_cnt: the number of array the rx_metrics pointing to
 * @rx_metrics: the infomation of receiver link metrics
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to fill the 1905.1 link merics message, usually when the user of 1905.1 
 * library receives _1905_RECV_LINK_METRICS_QUERY, it should call this function to reply
 * the 1905.1 link metrics response message.
 * note:this api is usually used when act as MAP agent
 */
int _1905_Set_Link_Metrics_Rsp_Info (IN struct _1905_context* ctx, 
	IN int tx_metrics_cnt, IN struct tx_link_metrics *tx_metrics, 
	IN int rx_metrics_cnt, IN struct rx_link_metrics *rx_metrics);


/**
 * _1905_Set_Assoc_Sta_Link_Metric_Rsp_Info - set the information of 
 * associated station link merics
 * @ctx: _1905_context data from _1905_init()
 * @info_cnt: the number of array the info pointing to
 * @info: the infomation of backhaul steering response
 * @sta_extended_metrics_cnt: num of sta extended metrics
 * @sta_extended_metrics: sta extended metrics
 * @sta_mac: sta mac
 * @reason: reason code
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to fill the MAP associated station link metrics response message, usually 
 * when the user of 1905.1 library receives _1905_RECV_ASSOC_STA_LINK_METRICS_QUERY_EVENT,
 * it should call this function to reply the associated station link metrics response message.
 * note:this api is usually used when act as MAP agent
 */
int _1905_Set_Assoc_Sta_Link_Metric_Rsp_Info (IN struct _1905_context* ctx,
	IN unsigned char info_cnt, IN struct link_metrics *info,
	IN unsigned char *sta_mac, IN unsigned char reason
#ifdef MAP_R2
	, IN unsigned char sta_extended_metrics_cnt,
	IN struct sta_extended_metrics_lib *sta_extended_metrics
#endif // #ifdef MAP_R2
);

/**
 * _1905_Set_Unassoc_Sta_Link_Metric_Rsp_Info - set the information of 
 * unassociated station link merics
 * @ctx: _1905_context data from _1905_init()
 * @info: the infomation of unassociated station link metrics
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to fill the MAP unassociated station link metrics response message, usually 
 * when the user of 1905.1 library receives _1905_RECV_UNASSOC_STA_LINK_METRICS_QUERY_EVENT,
 * it should call this function to reply the unassociated station link metrics response message.
 * note:this api is usually used when act as MAP agent
 */
int _1905_Set_Unassoc_Sta_Link_Metric_Rsp_Info (IN struct _1905_context* ctx,
	IN struct unlink_metrics_rsp *info);

/**
 * _1905_Set_Beacon_Metrics_Report_Info - set the information of 
 * beacon metrics report
 * @ctx: _1905_context data from _1905_init()
 * @info: the infomation of beacon metrics report
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to fill the MAP beacon metrics response message, usually when the user of 
 * 1905.1 library receives _1905_RECV_BEACON_METRICS_QUERY_EVENT, it should call this function
 * to reply the beacon metrics response message.
 * note:this api is usually used when act as MAP agent
 */
int _1905_Set_Beacon_Metrics_Report_Info (IN struct _1905_context* ctx, 
	IN struct beacon_metrics_rsp_lib *info);

/**
 * _1905_Set_Sta_Notification_Info - notify the 1905 daemon the station association event
 * @ctx: _1905_context data from _1905_init()
 * @info: the infomation of station association event
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to notify the 1905 service that a station has joined in current ap
 * note:this api is usually used when act as MAP agent or controller
 */
int _1905_Set_Sta_Notification_Info (IN struct _1905_context* ctx, 
	IN struct client_association_event *info);

/**
 * _1905_Set_Read_1905_Tlv_Req - notify the 1905 service to read the dev_send_1905 
 * file and send corresponding message
 * @ctx: _1905_context data from _1905_init()
 * @file_path: the file path of dev_send_1905 raw data
 * @len: length of file_path string
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to notify the 1905 service to read the dev_send_1905 file and send
 * the corresponding message
 * note:this api is usually used when act as MAP agent or controller in WFA Certification 
 */
int _1905_Set_Read_1905_Tlv_Req (IN struct _1905_context* ctx, IN char* file_path, IN int len);

/**
 * _1905_Set_Read_Bss_Conf_Request - set the information of 
 * bss configuration
 * @ctx: _1905_context data from _1905_init()
 *
 * This function is used to set the bss config file path to 1905.1 service, the bss config information 
 * is used to configurate the wireless setting of other MAP agents.
 * note:this api is usually used when act as MAP controller
 * note:this api is usually used when in WFA Certification mode
 */
int _1905_Set_Read_Bss_Conf_Request (IN struct _1905_context* ctx);

/**
 * _1905_Set_Read_Bss_Conf_and_Renew - set and update the information of bss configuration
 * @ctx: _1905_context data from _1905_init()
 * @local_only: if it is set, only the bss info of local interfaces will be updated.
 *    Otherwise, it will send renew message to agents to update their bss info
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to set the bss config file path to 1905.1 service, the bss config information
 * is used to configurate the wireless setting of the local and other MAP agents.
 * note:this api is usually used when act as MAP controller
 * note:this api is usually used when in WFA Certification mode
 */
int _1905_Set_Read_Bss_Conf_and_Renew (IN struct _1905_context *ctx, IN unsigned char local_only);

/**
 * _1905_Set_Read_Bss_Conf_and_Renew_v2 - set and update the information of bss configuration manually
 * @ctx: _1905_context data from _1905_init()
 * @local_only: if it is set, only the bss info of local interfaces will be updated.
 *    Otherwise, it will send renew message to agents to update their bss info
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to set the bss config file path to 1905.1 service, the bss config information
 * is used to configurate the wireless setting of the local and other MAP agents.
 * note:this api is usually used when act as MAP controller
 * note:this api is usually used when user update bss manually,  by modifying wts_bss_info_config, or changing the bss setting on web UI
 */
int _1905_Set_Read_Bss_Conf_and_Renew_v2 (IN struct _1905_context *ctx, IN unsigned char local_only);


/**
 * _1905_Set_Bss_Config - set the information of
 * bss configuration
 * @ctx: _1905_context data from _1905_init()
 * @info: pointer to bss config info
 * @oper: 
 	BSS_ADD - add new bss info to current bss database
 	BSS_RESET_ADD - remove all current bss info in database and add new bss info
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to set the bss config information to 1905.1 service, the bss config information 
 * is used to configurate the wireless setting of other MAP agents.
 * note:this api is usually used when act as MAP controller
 */
int _1905_Set_Bss_Config(IN struct _1905_context* ctx, IN struct bss_config_info* info, 
	IN enum BSS_CONFIG_OPERATION oper);

/**
 * _1905_Send_Topology_Query_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send topology query to specific 1905.1 device
 * note:this api is usually used when act as MAP agent or controller 
 */
int _1905_Send_Topology_Query_Message (IN struct _1905_context* ctx, IN char* almac);

/**
 * _1905_Send_AP_Autoconfig_Search_Message
 * @ctx: _1905_context data from _1905_init()
 * @band: Frequency band of the unconfigured interface requesting an autoconfiguration
 		0x00: 802.11 2.4 GHz
		0x01: 802.11 5 GHz
		0x02: 802.11 60 GHz
		0x03~0xFF: Reserved values
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send autoconfig search message to find the controller
 * note:this api is usually used when act as MAP agent
 */
int _1905_Send_AP_Autoconfig_Search_Message(IN struct _1905_context* ctx,
	IN unsigned char band);

/**
 * _1905_Send_AP_autoconfig_Renew_Message
 * @ctx: _1905_context data from _1905_init()
 * @band: Frequency band supported by the autoconfiguration process
 		0x00: 802.11 2.4 GHz
		0x01: 802.11 5 GHz
		0x02: 802.11 60 GHz
		0x03~0xFF: Reserved values
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send autoconfig renew message to agents to trigger autoconfig renew procedure
 * note:this api is usually used when act as MAP controller
 */
int _1905_Send_AP_autoconfig_Renew_Message (IN struct _1905_context* ctx,
	IN unsigned char band);

/**
 * _1905_Send_Link_Metric_Query_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address
 * @neighbor: 0x01 -query all neighbors
 *                  0x02 -query specific neighbor
 *                  0x03~0xff -not permitted
 * @neighbor_almac: the al mac address of the neighbor need to query, if neighbor is equal to 0x01, 
 * then this argument is ignored
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send link metrics query to specific 1905.1 device to query the link metrics
 * information of the specific neighbor/neighbors
 * note:this api is usually used when act as MAP agent or controller 
 */
int _1905_Send_Link_Metric_Query_Message (IN struct _1905_context* ctx, 
	IN char* almac, IN unsigned char  neighbor, IN char* neighbor_almac, IN unsigned char metrics);

/**
 * _1905_Send_Vendor_Specific_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address
 * @vend_spec_tlv: a pointer pointing to the vendor specific tvl buffer
 * @len: the length of the ved_sepc_tlv
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send 1905.1 vendor specific message to the specific 1905.1 device
 * note:this api is usually used when act as MAP agent or controller 
 */
int _1905_Send_Vendor_Specific_Message(IN struct _1905_context* ctx, 
	IN char* almac, IN char* vend_spec_tlv, IN unsigned short len);

/**
 * _1905_Send_AP_Capability_Query_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send ap capability query message to the specific map device to query its
 * ap capability
 * note:this api is usually used when act as controller 
 */
int _1905_Send_AP_Capability_Query_Message (IN struct _1905_context* ctx, IN char* almac);

/**
 * _1905_Send_MAP_Policy_Request_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * @steer_disallow_sta_cnt: the number of steering disallowed station
 * @steer_disallow_sta_list: a pointer pointing to mac address of the steering disallowed stations
 * @btm_disallow_sta_cnt: the number of btm disallowed station
 * @btm_disallow_sta_list: a pointer pointing to mac address of the btm disallowed stations
 * @radio_cnt_steer: Number of radios for which control policy is being indicated
 * @steering_policy: a pointer pointing to the lib_steer_radio_policy array which is used to fill the
 * steering policy tlv in map policy config request
 * @ap_rep_interval: AP Metrics Reporting Interval in seconds
 * @radio_cnt_metrics: Number of radios to which ap metrics policy is being applied
 * @metrics_policy: a pointer pointing to lib_metrics_radio_policy array which is used to fill the metrics
 * reporting policy tlv in map policy config request
 * @scan_rep_include: indicate if include a channel scan report policy
 * @scan_rep_policy: channel scan report policy
 * @unsuccess_assoc_policy_include: indicate if include a unsuccess assoc policy
 * @unsuccess_assoc_policy: unsuccess assoc policy
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to map policy config request to the specific map device to config the map policy
 * note:this api is usually used when act as controller 
 */
int _1905_Send_MAP_Policy_Request_Message (IN struct _1905_context* ctx, 
	IN char* almac, IN unsigned char steer_disallow_sta_cnt, IN char *steer_disallow_sta_list,
	IN unsigned char btm_disallow_sta_cnt, IN char *btm_disallow_sta_list, IN unsigned char radio_cnt_steer,
	IN struct lib_steer_radio_policy *steering_policy, IN unsigned char ap_rep_interval,
	IN unsigned char radio_cnt_metrics, IN struct lib_metrics_radio_policy *metrics_policy
#ifdef MAP_R2
	, unsigned char scan_rep_include, unsigned char scan_rep_policy,
	unsigned char unsuccess_assoc_policy_include, struct lib_unsuccess_assoc_policy *unsuccess_assoc_policy
#endif	//#ifdef MAP_R2
	);

/**
 * _1905_Send_Channel_Preference_Query_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send channel preference query message to the specific map device to query its
 * channel preference
 * note:this api is usually used when act as controller 
 */
int _1905_Send_Channel_Preference_Query_Message (IN struct _1905_context* ctx, IN char* almac);

/**
 * _1905_Send_Channel_Selection_Request_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * @ch_prefer_cnt: the arrary size the prefer pointing to
 * @prefer: a pointer pointing to the ch_prefer_lib arrary which is used to fill the channel preference tlv
 * in channel selection request
 * @transmit_power_cnt: the arrary size the power_limit pointing to
 * @power_limit: a pointer pointing to the transmit_power_limit arrary which is used to fill the transmit power
 * limit tlv
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send channel selection request message to the specific map device to request
 * target map device to select channel
 * note:this api is usually used when act as controller 
 */
int _1905_Send_Channel_Selection_Request_Message (IN struct _1905_context* ctx, 
	IN char* almac, unsigned char ch_prefer_cnt, IN struct ch_prefer_lib* prefer, 
	IN unsigned char transmit_power_cnt, IN struct transmit_power_limit* power_limit);

/**
 * _1905_Send_Client_Capability_Query_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * @bssid: the bssid of a bss that the station associated to
 * @sta_mac: the mac address of the station 
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send client capability query message to the specific map device to query
 * the capability of the station that associated to this device
 * note:this api is usually used when act as controller 
 */
int _1905_Send_Client_Capability_Query_Message (IN struct _1905_context* ctx,
	IN char* almac, IN unsigned char *bssid, IN unsigned char *sta_mac);

/**
 * _1905_Send_AP_Metrics_Query_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * @bssid_cnt: the number of the bssid arrary the bssid_list points to
 * @bssid_list: a pointer pointing to the bssid arrary for which the ap metrics should be reported
 * @radio_cnt: num of radio
 * @identifier_list: identifier for each radio
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send ap metrics query message to the specific map device to query
 * the ap metrics info of some bss the device is operating.
 * note:this api is usually used when act as controller 
 */
int _1905_Send_AP_Metrics_Query_Message (IN struct _1905_context* ctx,
	IN char* almac, IN unsigned char bssid_cnt, IN unsigned char *bssid_list
#ifdef MAP_R2	
	, unsigned char radio_cnt, unsigned char *identifier_list
#endif // #ifdef MAP_R2
	);

/**
 * _1905_Send_Associated_STA_Link_Metrics_Query_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * @sta_mac: the mac address of the station associated to this map device
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send associated station link metrics query message to the specific map 
 * device to query the station link metrics information
 * note:this api is usually used when act as controller 
 */
int _1905_Send_Associated_STA_Link_Metrics_Query_Message (IN struct _1905_context* ctx,
	IN char* almac, IN unsigned char *sta_mac);

/**
 * _1905_Send_Unassociated_STA_Link_Metrics_Query_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * @sta_mac: the mac address of the unassociated station
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send unassociated station link metrics query message to the specific map 
 * device to query the unassociated station link metrics information
 * note:this api is usually used when act as controller 
 */
int _1905_Send_Unassociated_STA_Link_Metrics_Query_Message (IN struct _1905_context* ctx, 
	IN char* almac, IN struct unassoc_sta_link_metrics_query *query);

/**
 * _1905_Send_Beacon_Metrics_Query_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * @query: the beacon metircs query information
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send beacon metrics query message to the specific map device
 * note:this api is usually used when act as controller 
 */
int _1905_Send_Beacon_Metrics_Query_Message (IN struct _1905_context* ctx, IN char* almac, 
	IN struct beacon_metrics_query *query);

/**
 * _1905_Send_Combined_Infrastructure_Metrics_Message
 * @ctx: _1905_context data from _1905_init()
 * @ap_metrics_cnt: the number of the array the metrics_info pointing to
 * @metrics_info: a pointer pointing to the ap_metrics_info_lib arrary which is used to fill the ap metrics tlv
 * in combined infrastructure metrics message
 * @bh_link_cnt: the backhaul link count between two map agents this message would like to report
 * @tx_metrics_bh_ap:  transmitter link metric information corresponding to the backhaul AP, note that the arrary size 
 * this point pointing to is equal to bh_link_cnt
 * @tx_metrics_bh_sta: transmitter link metric information corresponding to the backhaul STA, note that the arrary size 
 * this point pointing to is equal to bh_link_cnt
 * @rx_metrics_bh_ap: receiver link metric information corresponding to the backhaul AP, note that the arrary size 
 * this point pointing to is equal to bh_link_cnt
 * @rx_metrics_bh_sta: receiver link metric information corresponding to the backhaul STA, note that the arrary size 
 * this point pointing to is equal to bh_link_cnt
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send combined infrastructure metrics message to the specific map device
 * note:this api is usually used when act as controller 
 */
int _1905_Send_Combined_Infrastructure_Metrics_Message (IN struct _1905_context* ctx, 
	IN char* almac, IN unsigned char ap_metrics_cnt, IN struct GNU_PACKED ap_metrics_info_lib *metrics_info,
	IN unsigned char bh_link_cnt, IN struct tx_link_metrics* tx_metrics_bh_ap, 
	IN struct tx_link_metrics *tx_metrics_bh_sta, IN struct rx_link_metrics* rx_metrics_bh_ap,
	IN struct rx_link_metrics *rx_metrics_bh_sta);

/**
 * _1905_Send_Client_Steering_Request_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * @request: steering request information
 * @tbss_cnt: the starget bss count
 * @info: the pointer pointing to the arrary which indicate the target bss info
 * @request_R2: R2 steering request information, if not provided, pointer will be NULL.
 * @tbss_cnt_R2: the starget bss count
 * @info_R2: the pointer pointing to the arrary which indicate the target bss info, if not provided, pointer will be NULL.
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send client steering request  to the specific map device
 * note:this api is usually used when act as controller 
 */
int _1905_Send_Client_Steering_Request_Message (IN struct _1905_context* ctx, 
	IN char* almac, IN struct lib_steer_request *request, IN unsigned char tbss_cnt, 
	IN struct lib_target_bssid_info *info
#ifdef MAP_R2
	, struct lib_steer_request_R2 *request_R2,
	unsigned char tbss_cnt_R2, struct lib_target_bssid_info_R2 *info_R2
#endif // #ifdef MAP_R2
	);


/**
 * _1905_Send_Client_Association_Control_Request_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * @control: indicates if the request is to block or unblock the indicated STAs from associating.
 *			0x00: Block
 *			0x01: Unblock
 *			0x02-0xFF: Reserved
 * @valid_time: Time period in seconds (from reception of the Client Association Control Request
 * message) for which a blocking request is valid.
 * @sta_cnt: STA List Count Indicating one or more STA(s) for which Client Association Control 
 * request applies.
 * @sta_list: pointer pointing to the arrary of STA MAC address for which the Client Association
 * Control request applies
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send client association control request to the specific map device
 * note:this api is usually used when act as controller 
 */
int _1905_Send_Client_Association_Control_Request_Message (IN struct _1905_context* ctx,
	IN char* almac, IN unsigned char *bssid, IN unsigned char control, 
	IN unsigned short valid_time, IN unsigned char sta_cnt, IN unsigned char *sta_list);

/**
 * _1905_Send_Backhaul_Steering_Request_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * @sta_mac: The MAC address of the associated backhaul station operated by the Multi-AP Agent.
 * @bssid: The BSSID of the target BSS.
 * @opclass: operating class of target BSS.
 * @channel: Channel number on which Beacon frames are being transmitted by the target BSS.
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send higher layer data message  to the specific map device
 * note:this api is usually used when act as controller or agent
 */
int _1905_Send_Backhaul_Steering_Request_Message (IN struct _1905_context* ctx, IN char* almac,
	IN unsigned char *sta_mac, IN unsigned char *bssid, IN unsigned char opclass, IN unsigned char channel);

/**
 * _1905_Send_Higher_Layer_Date_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * @protocol: Higher layer protocol
 * @len: the length of the payload
 * @payload: a pointer pointing to the payload buffer
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send higher layer data message  to the specific map device
 * note:this api is usually used when act as controller or agent
 */
int _1905_Send_Higher_Layer_Data_Message (IN struct _1905_context* ctx, IN char* almac,
	IN unsigned char protocol, IN unsigned short len, IN unsigned char *payload);


#ifdef MAP_R2

/*channel scan feature*/
int _1905_Send_Channel_Scan_Request_Message (
	IN struct _1905_context* ctx, char* almac,
	IN unsigned char fresh_scan, IN int radio_cnt,
	IN struct scan_request_lib *scan_req);

int _1905_Send_Channel_Scan_Report_Message (
	IN struct _1905_context* ctx, char* almac,
	IN unsigned char ts_len, IN unsigned char *ts_str,
	IN int scan_res_cnt, IN unsigned char *scan_res,
	IN unsigned char *vs_tlv, IN unsigned int vs_len);

int _1905_Set_Channel_Scan_Cap (IN struct _1905_context* ctx,
	IN struct scan_capability_lib *scan_caps);

/**
 * _1905_Set_R2_AP_Cap
 * @ctx: _1905_context data from _1905_init()
 * @info: r2 capability info
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to set R2 cap to 1905
 * note:this api is usually used when act as controller or agent
 */
int _1905_Set_R2_AP_Cap(IN struct _1905_context* ctx, struct ap_r2_capability *info);


/**
 * _1905_Set_Metric_collection_interval
 * @ctx: _1905_context data from _1905_init()
 * @interval: metric collection interval
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to set metric collection interval to 1905
 * note:this api is usually used when act as controller or agent
 */
int _1905_Set_Metric_collection_interval(IN struct _1905_context* ctx, unsigned int interval);



/**
 * _1905_Send_Tunneled_Message - send Tunneled Message to 1905 service
 * @ctx: _1905_context data from _1905_init()
 *  @tunneled_msg: Tunneled message with payload.
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send the Tunneled message to 1905 service.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Send_Tunneled_Message (IN struct _1905_context* ctx, char* almac,
	IN struct tunneled_message_lib * tunneled_msg);

/**
 * _1905_Send_Assoc_Status_Notification_Request - send assoc notification to 1905 service
 * @ctx: _1905_context data from _1905_init()
 * @almac: almac address and will be NULL, if almac is NULL send to all agents and controller.
 * @assoc_notification_lib: assoc notification message
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send assoc notification message to the specific map device
 * note:this api is usually used when act as agent
 */ 
int _1905_Send_Assoc_Status_Notification_Message (IN struct _1905_context* ctx,
	char* almac, IN struct assoc_notification_lib *assoc_notification);


/**
 * _1905_Set_CAC_Cap - set CAC capability to 1905 service
 * @ctx: _1905_context data from _1905_init()
 * @cac_caps: cac capability information
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to set the cac capability to 1905 service, usually before 
 * and after 1905 onboarding/autoconfig procedure, it is reqeust that calling this function to 
 * set this capability to 1905 service, and also it is permitted that calling this function when 
 * the user of 1905.1 library think it is neccessary to set this capability to 1905 service.
 * note: this api is usually used when act as MAP agent
 */
int _1905_Set_CAC_Cap (IN struct _1905_context* ctx,
	IN struct cac_capability_lib * cac_caps, IN int len);


/**
 * _1905_Send_CAC_Request - send CAC request to 1905 service
 * @ctx: _1905_context data from _1905_init()
 * @radio_cnt: the number of radios upon which CAC is requested
 * @cac_req: CAC request information
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send channel CAC request to the specific map device
 * note:this api is usually used when act as controller 
 */
int _1905_Send_CAC_Request_Message (IN struct _1905_context* ctx, char* almac, 
	IN struct cac_request_lib * cac_req);


/**
 * _1905_Send_CAC_Terminate - send CAC termination to 1905 service
 * @ctx: _1905_context data from _1905_init()
 * @radio_cnt: the number of radios upon which CAC is requested
 * @scan_term: CAC Terminate information
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send channel CAC request to the specific map device
 * note:this api is usually used when act as controller 
 */
int _1905_Send_CAC_Terminate_Message (IN struct _1905_context* ctx, char* almac,
	IN struct cac_terminate_lib * cac_term);

/**
 * _1905_Send_Client_Disassociation_Stats_Message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * @reason_code: disassociation reason
 * @stat: traffic statistic information for the client
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send client disassociation stats message to controller
 * note:this api is usually used when act as agent 
 */
int _1905_Send_Client_Disassociation_Stats_Message(IN struct _1905_context* ctx, 
	char* almac, unsigned short reason_code, struct stat_info *stat);


/**
 * _1905_Send_Failed_Connection_message
 * @ctx: _1905_context data from _1905_init()
 * @almac: peer al mac address 
 * @sta_mac: sta mac address 
 * @status: association status code
 * @reason: disassciation reason code
 * Returns: 0-operation success, otherwise operation fail
 *
 * This function is used to send failed assoc message to controller 
 * note:this api is usually used when act as agent 
 */
int _1905_Send_Failed_Connection_message(IN struct _1905_context* ctx, 
	char* almac, char* sta_mac, unsigned short status, unsigned short reason);
int _1905_Send_BH_Sta_Cap_Query (IN struct _1905_context *ctx, char *almac);

#endif // #ifdef MAP_R2

int _1905_Set_Wireless_Interface_Info (IN struct _1905_context* ctx,
	IN struct interface_info_list_hdr *info);

int _1905_clear_switch_table (IN struct _1905_context* ctx);

#endif
