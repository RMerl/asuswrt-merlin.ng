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

#ifndef __MAP_1905_H__
#define __MAP_1905_H__

#include "interface.h"

struct wifi_app;
struct map_info;

#include <stdint.h>
#include "types.h"
#include "os.h"
#include "list.h"
#include "util.h"
#include "driver.h"
#include "event.h"
#include "debug.h"
#include "eloop.h"
#include "rt_nl_copy.h"
#include "ctrl_iface_unix.h"
#include "ieee80211_defs.h"
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "rt_config.h"
#include "arp.h"

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */
#ifdef AUTOROLE_NEGO
#define PORT3			5003
#define TAG_LEN			4
#define PKT_TYPE_LEN	1
#define ROLE_LEN		1
#endif //AUTOROLE_NEGO
#define PRINT_RA_IDENTIFIER(addr) \
		addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]

#define BLOCK_LIST_NUM 128

//wapp command message len to driver
#define MAX_CMD_MSG_LEN 384

//Event buffer len to 1905
#define MAX_EVT_BUF_LEN 3072

#define MAP_SUCCESS 0
#define MAP_ERROR (-1)

#ifdef MAP_R2
#define STEERING_R2 1 
#endif

#define WPS_AUTH_OPEN 0x0001
#define WPS_AUTH_WPAPSK 0x0002
#define WPS_AUTH_WPA2PSK 0x0020
#define WPS_AUTH_SHARED 0x0004
#define WPS_AUTH_WPA 0x0008
#define WPS_AUTH_WPA2 0x0010
#ifdef MAP_SUPPORT		/* Add WPA3 Support with MAP_R1*/
#define WPS_AUTH_SAE	0x0040
#endif

#define WPS_AUTH_MIXED (WPS_AUTH_WPAPSK | WPS_AUTH_WPA2PSK)
#define WPS_ENCR_NONE 0x0001
#define WPS_ENCR_TKIP 0x0004
#define WPS_ENCR_AES 0x0008
#define WPS_ENCR_AESTKIP (WPS_ENCR_TKIP | WPS_ENCR_AES)

#ifdef AUTOROLE_NEGO
struct GNU_PACKED dev_role_negotiate {
	u8 other_dev_role;
	u8 other_dev_almac[MAC_ADDR_LEN];
};
#endif //AUTOROLE_NEGO

struct sta_info {
	unsigned char staAddr[MAC_ADDR_LEN];
};

//Message from 1905
struct GNU_PACKED msg_1905 {
	unsigned short type;
	unsigned char role;             /*0-indicate this message is from agent, 1-indicate this message is from controller*/
	unsigned short length;
	unsigned char band;
	UCHAR bssAddr[MAC_ADDR_LEN];
	UCHAR staAddr[MAC_ADDR_LEN];
	char body[0];
};

//Event to 1905
struct GNU_PACKED evt {
	unsigned short type;
	unsigned short length;
	unsigned char buffer[0];
};

#define AIR_MONITOR_RESPONSE_DONE_BIT		31
#define AIR_MONITOR_QUERY_TIMEOUT			1 //second
#define WPS_TIMEOUT							150 // 2.5 min
#define RRM_REQUEST_TIMEOUT					5
struct GNU_PACKED air_monitor_query_rsp{
	struct dl_list list;
	unsigned char almac[ETH_ALEN];
	unsigned int bitmap;
	struct unlink_metrics_rsp unlink_metric;
};

typedef enum {
	BH_SUCCESS = 0,
	BH_REJECT_FOR_NON_OP_CHN,	//Rejected because the backhaul station cannot operate on the channel specified.
	BH_REJECT_FOR_BAD_RSSI,		//Rejected because the target BSS signal is too weak or not found.
	BH_REJECT_BY_TARGET_BSS,	//Authentication or association Rejected by the target BSS
} BH_STATUS_CODE;

typedef enum {
	BLOCK = 0,
	UNBLOCK,
} ASSOC_CTRL;

#ifdef ACL_CTRL
typedef enum {
	BL_ADD = 0,
	BL_DEL,
	BL_FLUSH,
} BLACKLIST_CMD_TYPE;

typedef enum {
	ACL_ADD = 0,
	ACL_DEL,
	ACL_FLUSH,
	ACL_POLICY_0,
	ACL_POLICY_1,
	ACL_POLICY_2,
	ACL_SHOW,
} ACL_CMD_TYPE;
#else

typedef enum {
	ACL_ADD = 0,
	ACL_DEL,
	ACL_FLUSH,
} ACL_CMD_TYPE;
#endif /*ACL_CTRL*/

#define STA_LEAVE 0
#define STA_JOIN (1 << 7)

typedef enum {
	STEERING_OPPORTUNITY = 0,
	STEERING_MANDATE,
} BTM_REQUEST_MODE;

typedef enum {
	AGENT_INIT_STEER_DISALLOW = 0,
	AGENT_INIT_RSSI_STEER_MANDATE,
	AGENT_INIT_RSSI_STEER_ALLOW,
} STEERING_POLICY;

/*************************************************
        Export function
**************************************************/
int map_1905_socket_init(
	struct wifi_app *wapp);

int map_1905_socket_deinit(
	struct wifi_app *wapp);

int wapp_send_1905_msg(
	struct wifi_app *wapp,
	u16 msg_type,
	u16 data_len,
	char *data);

void wapp_iface_send(
	struct wifi_app* wapp,
	char* buf,
	size_t buf_len,
	char *dst_dameon);

int map_send_assoc_cli_msg(
	struct wifi_app *wapp,
	unsigned char *bss_addr,
	unsigned char *sta_addr,
	unsigned char stat,
	char *evt_buf);

int map_1905_send(
	struct wifi_app* wapp,
	char* buffer_send,
	int len_send);

#ifdef ACL_CTRL
void map_blacklist_system_cmd(
	struct wapp_dev *wdev,
	unsigned char *sta_addr,
	BLACKLIST_CMD_TYPE type);
#endif /*ACL_CTRL*/

void map_acl_system_cmd(
	struct wapp_dev *wdev,
	unsigned char *sta_addr,
	ACL_CMD_TYPE type);

int wapp_send_steering_completed_msg(
	struct wifi_app *wapp,
	char *buf,
	int* len_buf);

int wapp_send_cli_steer_btm_report_msg(
	struct wifi_app *wapp,
	char *buf,
	int max_len,
	struct cli_steer_btm_event *btm_evt);

int wapp_send_operbss_msg(
	struct wifi_app *wapp,
	char *buf,
	int max_len,
	unsigned char *identifier,
	unsigned char (*bssid)[6],
	unsigned char **ssid,
	unsigned char num);

int map_1905_send_controller(
	struct wifi_app* wapp,
	char* buffer_send,
	int len_send);

int map_send_one_assoc_sta_msg(
	struct wifi_app *wapp,
	struct wapp_sta *sta);

int map_send_assoc_sta_msg(
	struct wifi_app *wapp);

int map_send_ap_metric_msg(
	struct wifi_app *wapp,
	struct ap_dev *ap);

#ifdef MAP_R2
int map_send_radio_metric_msg(
	struct wifi_app *wapp,
	wapp_event_data *event_data,
	u32 ifindex);
#endif

void map_trigger_deauth(
	struct wifi_app *wapp,
	char *ifname,
	unsigned char *sta_addr);

void map_1905_req(
	struct wifi_app *wapp,
	struct wapp_1905_req *req);

int map_update_neighbor_bss(
	struct wifi_app *wapp,
	struct topo_info* top_info);

int map_operating_channel_info(
	struct wifi_app *wapp);

int map_config_bssload_thrd_setting_msg(
	struct wifi_app *wapp,
	const char *iface,
	char *high_thrd,
	char *low_thrd);

int map_send_wireless_inf_info(
	struct wifi_app *wapp,
	unsigned char write_to_conf,
	char send_to_1905);

int map_receive_addtional_bh_assoc_msg(
	struct wifi_app *wapp,
	char *msg_buf);
#ifdef MAP_SUPPORT
unsigned char air_monitor_entry_check(struct wifi_app *wapp,
	unsigned char *sta_mac, unsigned char channel);
void send_air_monitor_reponse(struct wifi_app *wapp, struct air_monitor_query_rsp  *metrics_rsp);
void air_monitor_query_timeout(void *eloop_data, void *user_ctx);
unsigned char check_for_monitor_complettion(struct air_monitor_query_rsp  *metrics_rsp);
int update_sta_rssi(struct wifi_app * wapp, struct wapp_dev *wdev, wapp_mnt_info *mnt_info);
void clear_monitor_list(struct wifi_app * wapp);
void clear_air_monitor_req_list(struct wifi_app * wapp);
void send_air_monitor_reponse_check(struct wifi_app * wapp, wapp_mnt_info *mnt_info);
void air_monitor_packet_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data);
void map_wps_timeout(void *eloop_data, void *user_ctx);
void bh_steering_ready_timeout(void *eloop_data, void *user_ctx);
#ifdef AUTOROLE_NEGO
void map_prepare_rawpacket(struct wifi_app *wapp,int map_dev_role);
void wapp_MapDevRoleNegotiation_init(struct wifi_app *wapp);
void send_DevRoleQuery_Response(unsigned int *almac,unsigned char packet_type, unsigned char my_dev_role, struct wifi_app *wapp);
#endif //AUTOROLE_NEGO
void wdev_handle_cac_stop(struct wifi_app *wapp, u32 ifindex, u8 *channel, u8 ret, int radar_status);
#ifdef MAP_R2
void map_send_assoc_notification(struct wifi_app *wapp, const char *iface, u8 assoc_disallow_reason);
void map_build_and_send_assoc_status_notification(struct wifi_app *wapp, struct assoc_notification_lib *assoc_notify,u8  bssid_num);
void map_send_btm_tunneled_message(struct wifi_app *wapp, const unsigned char *peer_addr,const char *btm_query, size_t btm_query_len);
void map_send_anqp_req_tunneled_message(struct wifi_app *wapp, const unsigned char *peer_mac_addr, const char *anqp_req, size_t anqp_req_len);
void map_send_wnm_tunneled_message(struct wifi_app *wapp, const unsigned char *peer_mac_addr, const char *wnm_req, size_t wnm_req_len);
void map_build_and_send_tunneled_message(struct wifi_app *wapp, const unsigned char *sta_mac, u8 proto_type, u8 num_payload_tlv, struct tunneled_msg_tlv *tlv);
#ifdef DFS_CAC_R2
int chan_mon_get_bw_from_op_class(u8 op_class);
void wdev_handle_cac_stop(struct wifi_app *wapp, u32 ifindex, u8 *channel, u8 ret, int radar_status);
int mapd_get_cac_capab_from_driver(struct wifi_app *wapp, unsigned char *addr);
int map_start_cac_req(struct wifi_app *wapp);
int map_receive_off_channel_scan_req(
        struct wifi_app *wapp, char *msg_buf,unsigned short msg_len);
int mapd_get_cac_status_from_driver(struct wifi_app *wapp, char *evt_buf, int* len_buf, u8 cac_completion);
#endif
int map_send_sta_disassoc_stats_msg(
	struct wifi_app *wapp, unsigned char *bss_addr, wapp_client_info *cli_info, char *evt_buf);
#endif
int chan_mon_get_vht_bw_from_op_class(u8 op_class);
int chan_mon_get_ht_bw_from_op_class(u8 op_class);
int set_bh_wsc_profile(struct wifi_app *wapp,
	struct wireless_setting *config);

#endif
int map_handle_get_wts_config(struct wifi_app *wapp, char *evt_buf, int* len_buf);
#endif

