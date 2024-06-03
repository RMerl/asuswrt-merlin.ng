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

#ifndef __MAP_H__
#define __MAP_H__

struct wifi_app;
struct ap_dev;
struct wapp_sta;

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
#include <sys/socket.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "rt_config.h"

#include "wapp_cmm.h"
#include "interface.h"
#include "off_ch_scan.h"
#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */
#define MAX_SET_BSS_INFO_NUM 26

#define MAX_LEN_OF_SSID 32
#define MAC_ADDR_LEN    6
#define MAP_CONF_TIMEOUT 5
#define MAP_MAX_STEER_COUNT 3
#define MAP_MAX_RADIO 3
#define MAX_NUM_OF_RADIO 6
#define MAX_RADIO_DBDC 2
#define MAP_CONF_PER_RADIO_TIMEOUT 20
#if 1 /* for WTS */
#define MAP_TMP_FILE "/tmp/map_cfg.txt"
#define MAC_ADDR_TMP_FILE "/tmp/map_macaddr.txt"
#define MAP_1905_CFG_FILE "/etc/map/1905d.cfg"
#define DPP_CFG_FILE "/etc/dpp_cfg.txt"

/* wireless interface info for 1905 */
#define MAP_WIFI_INFO_FILE "/etc/wifi_info.txt"  
#endif

#define MAP_DEFAULT_SSID "MAP-AP-Unconfig"

#if 1
// TODO: use profile to config this
#define MAP_DEFAULT_ETH_BH	"eth2"
#define MAP_DEFAULT_WIFI_BH	"apcli0"
#endif

/*WSC Encryption type, defined in wsc 2.0.2 p.114*/
#define ENCRYP_NONE 0x0001
#define ENCRYP_WEP 0x0002
#define ENCRYP_TKIP 0x0004
#define ENCRYP_AES 0x0008
#define ENCRYP_AES_TKIP (ENCRYP_TKIP | ENCRYP_AES)


/*WSC Authentication type, defined in wsc 2.0.2 p.105*/
#define AUTH_OPEN 0x0001
#define AUTH_WPA_PERSONAL 0x0002
#define AUTH_SHARED 0x0004
#define AUTH_WPA_ENTERPRISE 0x0008
#define AUTH_WPA2_ENTERPRISE 0x0010
#define AUTH_WPA2_PERSONAL 0x0020

#define BIT(n)                          ((UINT32) 1 << (n))

#define DEVICE_ROLE_INVALID -1
#define DEVICE_ROLE_UNCONFIGURED 0
#define DEVICE_ROLE_CONTROLLER 1
#define DEVICE_ROLE_AGENT 2
#ifdef AUTOROLE_NEGO
#define PACKET_TYPE_QUERY 1
#define PACKET_TYPE_RESPONSE 2
#endif // AUTOROLE_NEGO
enum MAP_ROLE {
    MAP_CONTROLLER = 0,
    MAP_AGENT,
};
typedef enum {
	RADIO_24G,
	RADIO_5GL,
	RADIO_5GH,
	RADIO_5G
} MAP_RADIO_SUPT_TYPE;
enum wps_band {
	WPS_BAND_24G,
	WPS_BAND_5GL,
	WPS_BAND_5GH,
	WPS_BAND_5G
};

typedef enum {
    MAP_CONF_UNCONF,
	MAP_CONF_WAIT_RSP,
	MAP_CONF_CONFED,
	MAP_CONF_STOP, /* give up or teared down */
} MAP_CONFIG_STATE;

enum {
	MAP_CONN_STATUS_UNCONF,
	MAP_CONN_STATUS_CONF
	};

typedef enum {
	MAP_BH_ETH,
	MAP_BH_WIFI,
} MAP_BH_TYPE;

enum _bandWidth{
	BW_20 = 0, // 20MHz
	BW_40,
	BW_80,
	BW_160,
	BW_10,
	BW_5,
	BW_8080,
};
enum _vht_bandWidth{
	bw_20_40 = 0,
	bw_80,
	bw_160,
	bw_8080,
};


enum _band{
	BAND_24G,
	BAND_5G
};

enum _ch_pref_val{
	PREF_SCORE_0 = 0,
	PREF_SCORE_1,
	PREF_SCORE_2,
	PREF_SCORE_3,
	PREF_SCORE_4,
	PREF_SCORE_5,
	PREF_SCORE_6,
	PREF_SCORE_7,
	PREF_SCORE_8,
	PREF_SCORE_9,
	PREF_SCORE_10,
	PREF_SCORE_11,
	PREF_SCORE_12,
	PREF_SCORE_13,
	PREF_SCORE_14
};

typedef enum {
	NON_DRIVER_PARAM,
	DRIVER_PARAM
} param_type;

struct map_evt_msg {
	u32 ifindex;
	u8  evtType;
	u8  evtLen;
} __attribute__ ((packed));

struct GNU_PACKED map_radio_identifier {
	u32 adpt_id;
	u8  card_id;
	u8 ra_id;
};

struct map_conf_state {
	u8 state;
	u8 elapsed_time;
};

struct map_metric_policy {
	unsigned char sta_rssi_thres;
	unsigned char sta_hysteresis_margin;
	unsigned char ch_util_thres;
	unsigned char ch_util_current;
	unsigned char ch_util_prev;
};
typedef struct GNU_PACKED _wsc_apcli_config_wrapper {
                unsigned char config_valid;
                unsigned char raid[8];
                wsc_apcli_config apcli_config;
}wsc_apcli_config_wrapper;

struct br_dev {
	char mac_addr[MAC_ADDR_LEN];
	int arp_sock;
	int ifindex;
	int ip;
};
#ifdef MAP_R2
#define MAX_TS_LEN 30
#define MAX_CH_BW_LEN 10
#define MAX_CH_NUM 13
#define MAX_OP_CLASS_NUM 6

#ifdef DFS_CAC_R2
/* The definition of Radar detection duration region */
#define CE		0
#define FCC		1
#define JAP		2
#define JAP_W53	3
#define JAP_W56	4
#define CHN		5
#define MAX_RD_REGION 6
#endif

struct GNU_PACKED r2_ap_cap {
	unsigned char max_total_num_sp_rules;
	unsigned char reserved1;

	unsigned char rsv_bit0_3:4;
	unsigned char enhanced_sp_flag:1;
	unsigned char basic_sp_flag:1;
	unsigned char byte_counter_units:2; /*0: bytes, 1: kibibytes (KiB), 2: mebibytes (MiB), 3: reserved*/

	unsigned char max_total_num_vid;
};

struct GNU_PACKED channel_scan_master_list{
	u8 scan_result_num;
	struct dl_list scan_result_list;  //struct scan_result_tlv
};
struct GNU_PACKED enqueue_msg_info{
	u8 enqueue_pending_msg;
	u8 ignore_req_too_soon;
	unsigned short msg_len;
	char *msg_body_ptr;	 
};


struct GNU_PACKED channel_scan_capab {
	u8 radio_num;
	struct radio_scan_capab radio_scan_params[MAP_MAX_RADIO];
};


#ifdef DFS_CAC_R2

enum cac_state {
	CAC_IDLE,
	CAC_ONGOING,
	CAC_DONE
};

struct cac_radio_state {
	u8 radio_id[MAC_ADDR_LEN];
	u8 id;
	u8 prev_ch;
	u8 op_class;
	enum cac_state state_cac;
};


#endif

#define MAX_AWAY_CHANNEL 5

#define MAX_NUM_OF_CHANNELS           59 // 14 channels @2.4G +  12@UNII(lower/middle) + 16@HiperLAN2 + 11@UNII(upper) + 0@Japan + 1 as NULL termination


struct msg_channel_list {
	CHANNEL_INFO CHANNELLIST[60];
};

#ifdef DFS_CAC_R2
typedef struct _OFFCHANNEL_CAC_MSG {
u8   Action;
u8 ifrn_name[32];
u32 ifIndex;
union {
				CHANNEL_INFO channel_data;
				OFFCHANNEL_SCAN_PARAM offchannel_param;
				OPERATING_INFO operating_ch_info;
				SORTED_CHANNEL_LIST sorted_channel_list;
} data;
} OFFCHANNEL_CAC_MSG, *POFFCHANNEL_CAC_MSG;


#endif

struct GNU_PACKED unsuccessful_association_policy {
	unsigned char report_unsuccessful_association;
	u32 max_supporting_rate;
};

struct failed_assoc_sta_info {
	u8 mac_addr[MAC_ADDR_LEN];
	struct os_time now;
	u16 failed_counter;
};

#ifdef DFS_CAC_R2
struct cac_state_ctrl {
	struct cac_radio_state radio_state[MAP_MAX_RADIO];
};
#endif

struct assoc_status {	
	u8 bssid[MAC_ADDR_LEN];
	u8 status;
};

struct assoc_notification_tlv {
	u8 bssid_num;
	struct assoc_status status[0];	
};

struct assoc_notification_lib {
	u8 assoc_notification_tlv_num;
	struct assoc_notification_tlv notification_tlv[0];	
};
struct tunneled_msg_tlv {
	unsigned int payload_len;
	unsigned char payload[0];
 };

struct tunneled_message_lib {
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char proto_type;
	unsigned char num_tunneled_tlv;
	unsigned char tunneled_msg_tlv[0];	
};

#ifdef DFS_CAC_R2

struct GNU_PACKED cac_tlv {
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char op_class_num;
	unsigned char ch_num;
	unsigned char cac_method;
	unsigned char cac_action;
};

struct GNU_PACKED cac_req {
	unsigned char num_radio;
	struct cac_tlv body[3];		
};


struct GNU_PACKED cac_term {
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char op_class_num;
	unsigned char ch_num;	
};

struct GNU_PACKED cac_terminate {
	unsigned char num_radio;
	struct cac_term term_tlv[0];		
};

#define MAX_RADIO_NUM 3
#define WAPP_SET_CAC_STOP 0

#endif
#endif
#define RADAR_DETECTED  1
#define CAC_SUCCESSFUL  0
#define CAC_FAILURE     5
struct GNU_PACKED cac_completion_report_opcap
{
	unsigned char op_class;    // This field shall be 0, if radar was not detected.
	unsigned char ch_num;	    // This field shall be 0, if radar was not detected.
};

struct GNU_PACKED cac_completion_status_lib
{
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char op_class;
	unsigned char channel;
	unsigned char cac_status;
	unsigned char op_class_num;    // This field shall be 0, if radar was not detected.
	struct cac_completion_report_opcap opcap[0];
};


struct GNU_PACKED cac_completion_report_lib
{
	unsigned char radio_num;
	struct cac_completion_status_lib cac_completion_status[0];
	//struct cac_report_opcap opcap[0];         // This field shall be 0, if radar was not detected.
};
#ifdef MAP_R2
#ifdef DFS_CAC_R2

struct GNU_PACKED cac_opcap
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[16]; //MAX of 5G Channels
};

struct GNU_PACKED cac_type
{
	unsigned char cac_mode;
	unsigned char cac_interval[3];
	unsigned char op_class_num;
	struct cac_opcap opcap[16];
};

struct GNU_PACKED cac_cap
{	
	unsigned char identifier[ETH_ALEN];
	unsigned char cac_type_num;
	struct cac_type type[2];
};

struct GNU_PACKED cac_capability_lib
{
	unsigned char country_code[2];
	unsigned char radio_num;
	struct cac_cap cap[MAP_MAX_RADIO];
};

struct GNU_PACKED cac_run_list
{
	unsigned char ch_num;
	unsigned char ch_list[16]; //MAX of 5G Channels
	struct os_time last_cac_time[16];
};
struct GNU_PACKED cac_driver_opcap
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[16]; //MAX of 5G Channels
	unsigned short cac_time[16];
	unsigned int last_cac_time[16];
	unsigned short non_occupancy_remain[16];
};

struct GNU_PACKED cac_driver_capab
{
	unsigned char country_code[2];
	unsigned char rdd_region;
	unsigned char op_class_num;
	struct cac_driver_opcap opcap[16];
	unsigned char active_cac;
	unsigned char ch_num;
	u32 cac_remain_time;
	u8 cac_mode;
};


struct GNU_PACKED cac_opcapa
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[0]; //MAX of 5G Channels
};

struct GNU_PACKED cac_tlv_type
{
	unsigned char cac_mode;
	unsigned char cac_interval[3];
	unsigned char op_class_num;
	struct cac_opcapa opcap[0];
};

struct GNU_PACKED cac_capa
{	
	unsigned char identifier[ETH_ALEN];
	unsigned char cac_type_num;
	struct cac_tlv_type type[0];
};

struct GNU_PACKED cac_lib
{
	unsigned char country_code[2];
	unsigned char radio_num;
	struct cac_capa cap[0];
};
#define MAX_CLASS_CHANNEL 68 //Total Operating class/channel pairs possible
#define MAX_CLASS_CHAN_NON_ALLOWED 52 //Non Allowed Operating class - Channel possible
#define DFS_AVAILABLE_LIST_CH_NUM 30/*MAX_NUM_OF_CHANNELS*/
struct GNU_PACKED cac_allowed_channel
{
	u8 op_class;
	u8 ch_num;
	u16 cac_interval;  // This field shall be 0, for Non-DFS Channels
};
struct GNU_PACKED cac_non_allowed_channel
{
	u8 op_class;
	u8 ch_num;    
	u16 remain_interval;
};
struct GNU_PACKED cac_ongoing_channel
{
	u8 op_class;
	u8 ch_num;
	u32 remain_interval;
};
struct GNU_PACKED cac_status_report_lib
{
	u8 allowed_channel_num;
	u8 non_allowed_channel_num;
	u8 ongoing_cac_channel_num; 
	struct cac_allowed_channel allowed_channel[MAX_CLASS_CHANNEL];
	struct cac_non_allowed_channel non_allowed_channel[MAX_CLASS_CHAN_NON_ALLOWED];
	struct cac_ongoing_channel cac_ongoing_channel[0];
};
typedef struct _DFS_REPORT_AVALABLE_CH_LIST {
	u8 Channel;
	u8 RadarHitCnt;
} DFS_REPORT_AVALABLE_CH_LIST, *PDFS_REPORT_AVALABLE_CH_LIST;
typedef struct _NOP_REPORT_CH_LIST {
	u8 Channel;
	u8 Bw;
	u16 NonOccupancy;
} NOP_REPORT_CH_LIST, *PNOP_REPORT_CH_LIST;

union dfs_zero_wait_msg {
	struct _aval_channel_list_msg{
		u8 Action;
		u8 Bw80TotalChNum;
		u8 Bw40TotalChNum;
		u8 Bw20TotalChNum;
		DFS_REPORT_AVALABLE_CH_LIST Bw80AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
		DFS_REPORT_AVALABLE_CH_LIST Bw40AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
		DFS_REPORT_AVALABLE_CH_LIST Bw20AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
	} aval_channel_list_msg;

	struct _nop_of_channel_list_msg{
		u8 Action;
		u8 NOPTotalChNum;
		NOP_REPORT_CH_LIST NopReportChList[DFS_AVAILABLE_LIST_CH_NUM];
	} nop_of_channel_list_msg;

	struct _set_monitored_ch_msg{
		u8 Action;
		u8 SyncNum;
		u8 Channel;
		u8 Bw;
		u8 doCAC;
	} set_monitored_ch_msg;

	struct _zerowait_dfs_ctrl_msg{
		u8 Action;
		u8 Enable;
	} zerowait_dfs_ctrl_msg;

	struct _nop_force_set_msg{
		u8 Action;
		u8 Channel;
		u8 Bw;
		u16 NOPTime;
	} nop_force_set_msg;

	struct _assign_next_target{
		u8 Channel;
		u8 Bw;
		u16 CacValue;
	} assign_next_target;

	struct _target_ch_show{
		u8 mode;
	} target_ch_show;
};

#endif

#endif

struct bh_link {
	struct dl_list list; 
	u8 bssid[MAC_ADDR_LEN];
};

struct map_info {
	char iface[IFNAMSIZ + 1];
	char map_cfg[128];
	char map_user_cfg[128];

	u8 bss_tbl_idx;
	wdev_bss_info    *op_bss_table;
	struct sockaddr_un sock_addr;
	u8 sta_report_on_cop;	//Support Unassociated STA link metric report on current operating Bss
	u8 sta_report_not_cop;	//Support Unassociated STA link metric report on currently non operating Bss
	u8 rssi_steer;			//Support Agent-initiated Rssi-based steering

	struct wapp_dev *bh_wifi_dev;
	u8 bh_link_ready;
	u8 ctrler_found;
	u32 bh_link_num;
	struct dl_list bh_link_list;
	struct wapp_radio *conf_ongoing_radio;
	u8 conf_ongoing_radio_idx;
	u8 ongoing_conf_retry_times;
	u8 radio_band_options[MAP_MAX_RADIO]; /* options for radio band. can be 24G,5GL,5GH, or 5G */
#if 1 /* for WTS*/
	u8 ctrl_alid[MAC_ADDR_LEN];
	u8 agnt_alid[MAC_ADDR_LEN];
	u8 is_ctrler;
	u8 is_agnt;
	u8 is_root;
	u8 fh_radio_supt; /* 0 - 2.4GHz only, 1 - 2.4GHz and 5G Low, 2 - 2.4GHz, 5G Low, 5G High */
	u8 bh_sta_radio;
	u8 fh_24g_bssid[MAC_ADDR_LEN];
	u8 fh_5g1_bssid[MAC_ADDR_LEN];
	u8 fh_5g2_bssid[MAC_ADDR_LEN];
	u8 ht_24g_supt; /*0 - not supported, 1 - supported*/
	u8 he_24g_supt; 
	u8 ht_5g1_supt; 
	u8 vht_5g1_supt; 
	u8 he_5g1_supt; 
	u8 ht_5g2_supt; 
	u8 vht_5g2_supt; 
	u8 he_5g2_supt; 
	u8 bh_type; /* eth or wifi */
	wapp_device_status device_status;
#endif
	u8 MapMode;
	u8 TurnKeyEnable;
	u8 quick_ch_change; /*Raghav: Quick channel change feature. Currently only implemented in MT7615 driver.*/
	u8 conf;
	char br_iface[IFNAMSIZ + 1];
	struct br_dev br;
	char enable_wps_toggle_5GL_5GH;
	char WPS_Fh_Fail;
	char g_LastWPS_ran_on;
	wsc_apcli_config_wrapper apcli_configs[MAX_NUM_OF_RADIO];
	struct off_ch_scan_req_s *off_ch_scan_req;
	struct off_ch_scan_report *off_ch_scan_rep;
	struct off_ch_scan_capab *off_ch_scan_capab;
	u8 off_ch_scan_prohibit; 
	u8 max_off_scan_interval; // TODO: fill.
	u16 off_ch_scan_capab_len;
	u8 off_ch_scan_req_len;
	u8 off_ch_scan_policy;
	struct off_ch_scan_state_ctrl off_ch_scan_state;
	u8 my_map_dev_role;
#ifdef AUTOROLE_NEGO
	int nego_role_send_sock;
#endif //AUTOROLE_NEGO
#ifdef MAP_R2
	//struct channel_scan_req *ch_scan_req;
	//struct channel_scan_report *ch_scan_rep;
	//struct channel_scan_capab *scan_capab;
	struct channel_scan_master_list ch_scan_master_list;
	u8 ch_scan_prohibit; 
	u8 max_scan_interval; // TODO: fill.
	//u16 scan_capab_len;
	u8 scan_req_len;
	u8 f_scan_req;
	struct enqueue_msg_info msg_info;
	u8 scan_policy;
	//struct ch_scan_state_ctrl ch_scan_state;
	struct unsuccessful_association_policy assoc_failed_policy;
	struct r2_ap_cap r2_ap_capab;
	u16 assoc_fail_rep_count;
#ifdef DFS_CAC_R2
	struct cac_req cac_req;
	struct cac_tlv *cac_tlv;
	u16 cac_req_len;
	u8 cac_req_ongoing;
	u8 num_cac_req;
	u8 cac_radio_ongoing;
	struct cac_state_ctrl cac_state;
	struct cac_capability_lib *cac_capab;
	u16 cac_capab_len;
	u16 cac_capab_final_len;
	u16 cac_op_class_total;
	struct cac_run_list cac_list;
#endif
	unsigned char br_name[IFNAMSIZ];
#ifdef MAP_R2
	u32 metric_rep_intv;
	u8 max_client_cnt;
#endif
#endif
	u8 map_version;
	u8 wps_after_scan;
};

int map_ctrl_interface_cmd_handle(
	struct wifi_app *wapp,
	const char *iface,
	u8 argc,
	char **argv);

int map_radio_tear_down(
	struct wifi_app *wapp,
	char *idtfer);

int map_bh_ready(
	struct wifi_app *wapp,
	u8 bh_type,
	u8 *ifname,
	u8 *mac_addr,
	u8 *bssid);

void map_periodic_exec(
	struct wifi_app *wapp);

int map_ctrl_interface_cmd_handle(
	struct wifi_app *wapp,
	const char *iface,
	u8 argc,
	char **argv);

int map_build_chn_pref(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf);

int map_build_ra_op_restrict(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf);

int map_build_ap_cap(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf);

int map_build_ap_ra_basic_cap(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf);

int map_build_ap_ht_cap(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf);

int map_build_ap_vht_cap(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf);

int map_build_ap_he_cap(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf);

int map_build_ap_op_bss(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf);

int map_build_assoc_cli(
	struct wifi_app *wapp,
	unsigned char *bss_addr,
	unsigned char *sta_addr,
	unsigned char stat,
	char *evt_buf);

int map_build_ap_metric(
	struct wifi_app *wapp,
	struct ap_dev *ap,
	char *evt_buf
);

#ifdef MAP_R2
int map_build_radio_metric(
	struct wifi_app *wapp,
	wapp_event_data *event_data,
	char *evt_buf,
	u32 ifindex
);
#endif

int map_build_wifi_tx_link_stats(
	struct wifi_app *wapp,
	struct wapp_sta *sta,
	char *evt_buf);

int map_build_eth_tx_link_stats(
	struct wifi_app *wapp,
	char *msg_buf,
	char *evt_buf);

int map_build_wifi_rx_link_stats(
	struct wifi_app *wapp,
	struct wapp_sta *sta,
	char *evt_buf);

int map_build_eth_rx_link_stats(
	struct wifi_app *wapp,
	char *msg_buf,
	char *evt_buf);

int map_build_assoc_sta_traffic_stats(
	struct wifi_app *wapp,
	char *evt_buf,
	u8 *radio_id
);

int map_build_one_assoc_sta_traffic_stats(
	struct wifi_app *wapp,
	struct wapp_sta *sta,
	char *evt_buf);

int map_build_assoc_sta_link_metric(
	struct wifi_app *wapp,
	char *evt_buf,
	u8 *radio_id
);

int map_build_assoc_sta_tp_metric(
	struct wifi_app *wapp,
	char *evt_buf,
	u8 *radio_id
);

int map_build_one_assoc_sta_link_metric(
	struct wifi_app *wapp,
	struct wapp_sta *sta,
	char *evt_buf
);

int map_build_unassoc_sta_link_metrics(
	struct wifi_app *wapp,
	char *msg_buf,
	char *evt_buf);

int map_send_ap_ht_capability_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf);

int map_send_ap_vht_capability_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf);

int map_send_ap_he_capability_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf);

#ifdef MAP_R2
int map_send_channel_scan_capability_msg(
	struct wifi_app *wapp, char *evt_buf, int* len_buf);
int map_build_assoc_sta_ext_metric(
	struct wifi_app *wapp, char *evt_buf, u8 *radio_id);
int map_build_one_assoc_sta_ext_link_metric(
	struct wifi_app *wapp, struct wapp_sta *sta, char *evt_buf);
int map_build_disassoc_stats(
	struct wifi_app *wapp,
	unsigned char *bss_addr,
	wapp_client_info *cli_info,
	char *evt_buf);
int map_send_r2_ap_capability_msg(
	struct wifi_app *wapp, char *evt_buf, int* len_buf);
#endif

void map_event_bh_sta_wap_done(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void map_event_str_sta_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void map_config_state_check(
	void *eloop_data,
	void *user_ctx);

//MAP initialize
int map_init(struct map_info *map);
void map_bss_table_release(struct map_info *map);
void map_reset_conf_sm(struct map_info *map);
int map_btm_rsp_action(
        struct wifi_app *wapp,
        struct wapp_dev *wdev,
        const u8 *peer_mac_addr,
        struct btm_payload *btm_resp);
int map_update_radio_band(
        struct wifi_app *wapp,
        struct wapp_radio *ra,
        u8 ch);
/**
 * hwaddr_aton - Convert ASCII string to MAC address (colon-delimited format)
 * @txt: MAC address as a string (e.g., "00:11:22:33:44:55")
 * @addr: Buffer for the MAC address (ETH_ALEN = 6 bytes)
 * Returns: 0 on success, -1 on failure (e.g., string not a MAC address)
 */
int hwaddr_aton(const char *txt, u8 *addr);

int hex2num(char c);

#define MAP_CONF_STATE_SET(_conf, _state) \
{\
	_conf->state = _state; \
	_conf->elapsed_time = 0; \
}

#define IS_CONF_STATE(_conf, _state) (_conf->state == _state)

#if 1  /* for sigma test, disable adpt_id */
#define MAP_GET_RADIO_IDNFER(_radio, _idnfer) \
{\
	struct map_radio_identifier *idfer = NULL; \
	idfer = (struct map_radio_identifier *) _idnfer; \
	idfer->ra_id = _radio->radio_id; \
	idfer->card_id = _radio->card_id; \
	memset(&idfer->adpt_id,0,sizeof(idfer->adpt_id)); \
}
#else
#define MAP_GET_RADIO_IDNFER(_radio, _idnfer) \
{\
	struct map_radio_identifier *idfer = NULL; \
	idfer = (struct map_radio_identifier *) _idnfer; \
	idfer->ra_id = _radio->radio_id; \
	idfer->card_id = _radio->card_id; \
	idfer->adpt_id = _radio->adpt_id; \
}
#endif

#define IS_MAP_CH_24G(__ch) \
	(__ch <= 14)

#define IS_MAP_CH_5GL(__ch) \
	(__ch >= 36 && __ch <= 64)

#define IS_MAP_CH_5GH(__ch) \
	(__ch >= 100)

#define IS_MAP_CH_5G(__ch) \
	(__ch >= 36)

#define IS_OP_CLASS_24G(__op_class) \
	(__op_class > 80 && __op_class <= 84)

#define IS_OP_CLASS_5GL(__op_class) \
	(__op_class > 110 && __op_class <= 120)

#ifdef WAPP_160BW
#define IS_OP_CLASS_5GH(__op_class) \
	(__op_class > 120 && __op_class <= 129)
#else
#define IS_OP_CLASS_5GH(__op_class) \
	(__op_class > 120 && __op_class <= 128)
#endif

#define IS_OP_CLASS_5G(__op_class) \
	(IS_OP_CLASS_5GL(__op_class) || IS_OP_CLASS_5GH(__op_class))

#define OP_CLASS_MATCH_RADIO_CONF(_radio_band, _op_class) \
	((_radio_band == RADIO_24G && IS_OP_CLASS_24G(_op_class)) || \
	 (_radio_band == RADIO_5GL && IS_OP_CLASS_5GL(_op_class)) || \
	 (_radio_band == RADIO_5GH && IS_OP_CLASS_5GH(_op_class)) || \
	 (_radio_band == RADIO_5G && IS_OP_CLASS_5G(_op_class)))

/* spec v171027 */
enum MAPRole {
	MAP_ROLE_TEARDOWN = 4,
	MAP_ROLE_FRONTHAUL_BSS = 5,
	MAP_ROLE_BACKHAUL_BSS = 6,
	MAP_ROLE_BACKHAUL_STA = 7,
};
void read_system_command_output(char *system_command, char *output_buffer);
void update_primary_ch_status(unsigned char channel, unsigned char status);
#ifdef WIFI_MD_COEX_SUPPORT
void dump_operable_channel();
#endif
int save_map_parameters(struct wifi_app *wapp,char *param, char *value, param_type type);
int get_map_parameters(struct map_info *map, char *param, char *value, param_type type, size_t val_len);
void wapp_reset_backhaul_config(struct wifi_app *wapp, struct wapp_dev *wappdev);
struct wapp_dev* wapp_dev_list_lookup_by_band_and_type(struct wifi_app *wapp, int band, int target_dev_type);
struct wapp_dev* wapp_dev_list_lookup_by_band_and_type_for_cert(struct wifi_app *wapp, int band, int target_dev_type);
#ifdef DPP_SUPPORT
void map_get_scan_result(void *eloop_ctx, void *timeout_ctx);
#endif /*DPP_SUPPORT*/
#endif

