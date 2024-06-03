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

/* this header file is uesed to define the message between wapp and 1905 deamon
 */
#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "types.h"
#include <sys/socket.h>
#include "linux/if.h"
#include "wapp_cmm_type.h"
#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */

#define MAX_CH_NUM 13
#define MAX_ELEMNT_NUM 4
#define MAX_HE_MCS_LEN 12
#define WSC_APCLI_CONFIG_MSG_USE_SAVED_PROFILE 998
#define WSC_APCLI_CONFIG_MSG_SAVE_PROFILE 999

/*WAPP EVENT*/
enum {
	WAPP_MAP_MIN_CMD=0x0,
	/*add new mapcmd below this line*/

	WAPP_MAP_BH_READY=0x01,
	WAPP_WPS_CONFIG_STATUS,
	WAPP_RADIO_BASIC_CAP,
	WAPP_CHANNLE_PREFERENCE,
	WAPP_RADIO_OPERATION_RESTRICTION,
	WAPP_DISCOVERY,
	WAPP_CLIENT_NOTIFICATION,
	WAPP_AP_CAPABILITY,
	WAPP_AP_HT_CAPABILITY,
	WAPP_AP_VHT_CAPABILITY,
	WAPP_AP_OP_BSS=0x0C,
	WAPP_ASSOC_CLI,
	WAPP_CHN_SEL_RSP,
	WAPP_TOPOQUERY,
	WAPP_OPERBSS_REPORT,
	WAPP_CLI_CAPABILITY_REPORT,
	WAPP_1905_CMDU_REQUEST,
	WAPP_CLI_STEER_BTM_REPORT,
	WAPP_STEERING_COMPLETED,
	WAPP_1905_READ_BSS_CONF_REQUEST,  /* for wts controller bss_info ready case */
	WAPP_1905_READ_BSS_CONF_AND_RENEW,
	WAPP_AP_METRICS_INFO,
	WAPP_GET_WSC_CONF,
	WAPP_1905_READ_1905_TLV_REQUEST, /* for wts send 1905 data ready case */
	WAPP_BACKHAUL_STEER_RSP,
	WAPP_ALL_ASSOC_STA_TRAFFIC_STATS,
	WAPP_ONE_ASSOC_STA_TRAFFIC_STATS,
	WAPP_ALL_ASSOC_STA_LINK_METRICS,
	WAPP_ONE_ASSOC_STA_LINK_METRICS,
	WAPP_TX_LINK_STATISTICS,
	WAPP_RX_LINK_STATISTICS,
	WAPP_UNASSOC_STA_LINK_METRICS,
	WAPP_OPERATING_CHANNEL_REPORT,
	WAPP_BEACON_METRICS_REPORT,
	WAPP_1905_REQ,
	WAPP_AP_LINK_METRIC_REQ,
	WAPP_OPERATING_CHANNEL_INFO,
	WAPP_AP_CAPABLILTY_QUERY=0x61,
	WAPP_CLI_CAPABLILTY_QUERY,
	WAPP_CH_SELECTION_REQUEST,
	WAPP_CLI_STEER_REQUEST,
	WAPP_CH_PREFER_QUERY,
	WAPP_POLICY_CONFIG_REQUEST,
	WAPP_CLI_ASSOC_CNTRL_REQUEST,
	WAPP_OP_CHN_RPT=0x8F,
	WAPP_STA_RSSI,
	WAPP_STA_STAT,
	WAPP_NAC_INFO,
	WAPP_STA_BSSLOAD,
	WAPP_TXPWR_CHANGE,
	WAPP_CSA_INFO,
	WAPP_APCLI_UPLINK_RSSI,
	WAPP_BSS_STAT_CHANGE,
	WAPP_BSS_LOAD_CROSSING,
	WAPP_APCLI_ASSOC_STAT_CHANGE,
	WAPP_STA_CNNCT_REJ_INFO,
	WAPP_UPDATE_PROBE_INFO,
	WAPP_WIRELESS_INF_INFO,
	WAPP_SCAN_RESULT,
	WAPP_SCAN_DONE,
	WAPP_MAP_VEND_IE_CHANGED,
	WAPP_ALL_ASSOC_TP_METRICS,
	WAPP_AIR_MONITOR_REPORT,
	WAPP_MAP_BH_CONFIG,
	WAPP_DEVICE_STATUS,
	WAPP_BRIDGE_IP,
	WAPP_SET_BH_TYPE,
	WAPP_OFF_CH_SCAN_REPORT,
	WAPP_NET_OPT_SCAN_REPORT,
#ifdef AUTOROLE_NEGO
	WAPP_MAP_NEGO_ROLE_RESP,
#endif //AUTOROLE_NEGO
	WAPP_AP_HE_CAPABILITY,
#ifdef MAP_R2
	WAPP_CHANNEL_SCAN_CAPAB,  //MAP R2
	WAPP_CHANNEL_SCAN_REPORT, // MAP R2
	WAPP_ASSOC_STATUS_NOTIFICATION, //MAP R2
	WAPP_TUNNELED_MESSAGE, //MAP R2
#ifdef DFS_CAC_R2
	WAPP_CAC_CAPAB,
#endif
#endif
	WAPP_CAC_COMPLETION_REPORT,
#ifdef MAP_R2
#ifdef DFS_CAC_R2
	WAPP_CAC_STATUS_REPORT,
#endif
	WAPP_METRIC_REP_INTERVAL_CAP,  // MAP R2
	WAPP_DISASSOC_STATS_EVT, // MAP R2
	WAPP_ALL_ASSOC_STA_EXTENDED_LINK_METRICS,
	WAPP_ONE_ASSOC_STA_EXTENDED_LINK_METRICS,
	WAPP_RADIO_METRICS_INFO, //MAP R2
	WAPP_R2_AP_CAP,
#endif
	WAPP_CH_LIST_DFS_INFO,
#ifdef MAP_R2
	WAPP_MBO_STA_PREF_CH_LIST,
#endif
	WAPP_CAC_PERIOD_ENABLE,
	WAPP_SEND_DPP_MSG,
	WAPP_MAP_RESET,
	WAPP_WTS_CONFIG,
	/*add new map event before this line*/
	WAPP_MAP_MAX_EVENT,
};

/*wapp user req command*/
enum {
	SET_REGITER_EVENT = 0x01,

	WAPP_USER_MIN_CMD = 0x80,
	/*add wapp user cmd below this line*/

	WAPP_USER_GET_CLI_CAPABILITY_REPORT = 0x81,
	WAPP_USER_SET_WIRELESS_SETTING,
	WAPP_USER_GET_RADIO_BASIC_CAP,
	WAPP_USER_GET_AP_CAPABILITY,
	WAPP_USER_GET_AP_HT_CAPABILITY,
	WAPP_USER_GET_AP_VHT_CAPABILITY,
	WAPP_USER_GET_SUPPORTED_SERVICE,
	WAPP_USER_GET_SEARCHED_SERVICE,
	WAPP_USER_GET_ASSOCIATED_CLIENT,
	WAPP_USER_GET_RA_OP_RESTRICTION,
	WAPP_USER_GET_CHANNEL_PREFERENCE,
	WAPP_USER_SET_CHANNEL_SETTING,
	WAPP_USER_GET_OPERATIONAL_BSS,
	WAPP_USER_SET_STEERING_SETTING,
	WAPP_USER_SET_ASSOC_CNTRL_SETTING,
	WAPP_USER_SET_LOCAL_STEER_DISALLOW_STA,
	WAPP_USER_SET_BTM_STEER_DISALLOW_STA,
	WAPP_USER_SET_RADIO_CONTROL_POLICY,
	WAPP_USER_GET_AP_METRICS_INFO,
	WAPP_USER_MAP_CONTROLLER_FOUND,
	WAPP_USER_SET_BACKHAUL_STEER,
	WAPP_USER_SET_METIRCS_POLICY,
	WAPP_USER_GET_ASSOC_STA_TRAFFIC_STATS,
	WAPP_USER_GET_ONE_ASSOC_STA_TRAFFIC_STATS,
	WAPP_USER_GET_ASSOC_STA_LINK_METRICS,
	WAPP_USER_GET_ONE_ASSOC_STA_LINK_METRICS,
	WAPP_USER_GET_TX_LINK_STATISTICS,
	WAPP_USER_GET_RX_LINK_STATISTICS,
	WAPP_USER_GET_UNASSOC_STA_LINK_METRICS,
	WAPP_USER_SET_RADIO_TEARED_DOWN,
	WAPP_USER_SET_BEACON_METRICS_QRY,
	WAPP_USER_SET_TOPOLOGY_INFO,
	WAPP_USER_SET_RADIO_RENEW,
	WAPP_USER_GET_OPERATING_CHANNEL_INFO,
	WAPP_USER_GET_AP_HE_CAPABILITY,
	WAPP_USER_FLUSH_ACL,
	WAPP_USER_GET_BSSLOAD,
	WAPP_USER_GET_RSSI_REQ,
	WAPP_USER_SET_WHPROBE_REQ,
	WAPP_USER_SET_NAC_REQ,
	WAPP_USER_SET_DEAUTH_STA,
	WAPP_USER_SET_VENDOR_IE,
	WAPP_USER_GET_APCLI_RSSI_REQ,
	WAPP_USER_GET_WIRELESS_INF_INFO,
	WAPP_USER_SET_ADDITIONAL_BH_ASSOC,
	WAPP_USER_ISSUE_SCAN_REQ,
	WAPP_USER_GET_SCAN_RESULT,
	WAPP_USER_SEND_NULL_FRAMES,
	WAPP_USER_GET_ALL_ASSOC_TP_METRICS,
	WAPP_USER_SET_AIR_MONITOR_REQUEST,

	WAPP_USER_SET_ENROLLEE_BH,
	WAPP_USER_SET_BSS_ROLE,
	WAPP_USER_TRIGGER_WPS,
	WAPP_USER_ISSUE_APCLI_DISCONNECT,
	WAPP_USER_SET_BH_WIRELESS_SETTING,
	WAPP_USER_SET_BH_PRIORITY,
	WAPP_USER_SET_BH_CONNECT_REQUEST,
	WAPP_USER_GET_BH_WIRELESS_SETTING,
	WAPP_USER_SET_GARP_REQUEST,
	WAPP_USER_SET_DHCP_CTL_REQUEST,
	WAPP_USER_GET_BRIDGE_IP_REQUEST,
	WAPP_USER_SET_BRIDGE_DEFAULT_IP_REQUEST,
	WAPP_USER_SET_TX_POWER_PERCENTAGE,
	WAPP_USER_SET_OFF_CH_SCAN_REQ,
	WAPP_USER_SET_NET_OPT_SCAN_REQ,
#ifdef AUTOROLE_NEGO
	WAPP_NEGOTIATE_ROLE,
#endif //AUTOROLE_NEGO
	WAPP_UPDATE_MAP_DEVICE_ROLE,
	WAPP_USER_SET_AVOID_SCAN_CAC,
#ifdef MAP_R2
	WAPP_USER_GET_SCAN_CAP, //MAP_R2
	WAPP_USER_SET_CHANNEL_SCAN_REQ, //MAP_R2
	WAPP_USER_SET_CHANNEL_SCAN_POLICY, //MAP_R2
#ifdef DFS_CAC_R2
	WAPP_USER_SET_CAC_REQ,
	WAPP_USER_SET_CAC_TERMINATE_REQ,
	WAPP_USER_GET_CAC_CAP, //MAP_R2
	WAPP_USER_GET_CAC_STATUS,
#endif
	WAPP_USER_GET_METRIC_REP_INTERVAL_CAP, //MAP R2
	WAPP_USER_GET_ASSOC_STA_EXTENDED_LINK_METRICS, //MAP R2
	WAPP_USER_GET_ONE_ASSOC_STA_EXTENDED_LINK_METRICS,
	WAPP_USER_GET_RADIO_METRICS_INFO, //MAP R2
	WAPP_USER_SET_UNSUCCESSFUL_ASSOC_POLICY, //MAP R2
	WAPP_USER_GET_R2_AP_CAP, //MAP R2
	WAPP_USER_SET_TRAFFIC_SEPARATION_SETTING, //MAP_R2
	WAPP_USER_SET_TRANSPARENT_VLAN_SETTING, //MAP_R2
#endif
	WAPP_USER_GET_WTS_CONFIG,
#ifdef ACL_CTRL
	WAPP_USER_SET_ACL_CNTRL_SETTING,
#endif
	/*add wapp user cmd above this line*/
	WAPP_USER_MAX_CMD,
};

typedef enum {
	SHOW_1905_REQ,
	SET_1905_DBG_LV_REQ,
} WAPP_1905_REQ_ID;

typedef enum {
	SYNC,
	ASYNC,
} CMD_EVENT_TYPE;

#define STATUS_BHSTA_UNCONFIGURED		0
#define STATUS_BHSTA_BH_READY			1
#define STATUS_BHSTA_CONFIGURED			2
#define STATUS_BHSTA_CONFIG_PENDING		3
#define STATUS_BHSTA_WPS_TRIGGERED		4
#define STATUS_BHSTA_WPS_FAILED			5
#define STATUS_BHSTA_WPS_NORMAL_CONFIGURED		6

#define STATUS_FHBSS_UNCONFIGURED		0
#define STATUS_FHBSS_WPS_TRIGGERED		1
#define STATUS_FHBSS_CONFIGURED			2
#define STATUS_FHBSS_WPS_FAILED			3
#define STATUS_FHBSS_WPS_SUCCESSFULL	4

#ifdef DPP_SUPPORT
#define SUPPORTED_SERVICE_TLV_TYPE 0x80
#define MULTI_AP_VERSION_TYPE 0xB3
#define AKM_SUITE_TLV_TYPE 0xCC
#define AP_RADIO_BASIC_CAPABILITY_TYPE 0x85
#define BACKHAUL_STATION_RADIO_CAP_TYPE 0xCB
#define R2_CAP_TLV_TYPE 0xB4
#define AP_RADIO_ADVANCE_CAP_TLV 0xBE

#define SUPPORTED_SERVICE_LENGTH 2
#define SUPPORTED_SERVICE2_LENGTH 3
#define SERVICE_CONTROLLER 0x00
#define SERVICE_AGENT 0x01
#endif /*DPP_SUPPORT*/

typedef struct GNU_PACKED _wapp_device_status {
	unsigned int status_fhbss;
	unsigned int status_bhsta;
}wapp_device_status;

struct GNU_PACKED cmd_to_wapp
{
	unsigned short type;
	unsigned char role;             /*0-indicate this message is from agent, 1-indicate this message is from controller*/
	unsigned short length;
	unsigned char band;
	unsigned char bssAddr[6];
	unsigned char staAddr[6];
	unsigned char body[0];
};

struct GNU_PACKED msg
{
	unsigned short type;
	unsigned short length;
	unsigned char buffer[0];
};

struct _map_cmd_event
{
	unsigned short cmd;
	unsigned short event;
	CMD_EVENT_TYPE type;
	unsigned char (*match_func)(struct msg*, struct cmd_to_wapp* );
};


struct GNU_PACKED wapp_1905_req
{
	unsigned char  id;
	unsigned short value;
};

typedef struct GNU_PACKED _inf_info{
	unsigned char ifname[IFNAMSIZ];
	unsigned char mac_addr[MAC_ADDR_LEN];
} inf_info;
/**
  *@type: backhaul type, 0-eth; 1-wifi
  *@ifname: backhaul inf name (with link ready)
  *@mac_addr: mac addr of the backhaul interface
  */
struct GNU_PACKED bh_link_info
{
	unsigned char type;
	unsigned char ifname[IFNAMSIZ]; /*  */
	unsigned char mac_addr[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
};

#ifdef DPP_SUPPORT
struct GNU_PACKED akm_suite {
	unsigned char oui[3];
	unsigned char oui_type;
};

struct GNU_PACKED akm_suite_caps
{
	unsigned char bhsta_akm_cnt;
	unsigned char bhap_akm_cnt;
	unsigned char fhap_akm_cnt;
	struct akm_suite bhsta_akm[10];
	struct akm_suite bhap_akm[10];
	struct akm_suite fhap_akm[10];
};

struct GNU_PACKED radio_bhsta_caps
{
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char is_sta_mac;
	unsigned char sta_mac[MAC_ADDR_LEN];
};

struct GNU_PACKED r2_ap_caps
{
	unsigned short max_sp_rule_cnt;
	unsigned short max_adv_sp_rule_cnt;
	unsigned short max_destination_addr;
	unsigned char byte_count_unit;
	unsigned char max_vid;
	unsigned char eth_edge_iface_cnt;	
	unsigned char addr[5][MAC_ADDR_LEN];
};

struct GNU_PACKED radio_adv_caps
{
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char ts_rules_support;
};
#endif /*DPP_SUPPORT*/
/**
*@type: backhaul type, 0-eth; 1-wifi
*/
struct GNU_PACKED bh_type_info
{
	unsigned char type;
};

/**
  *@identifier: radio unique identifier of the radio requesting config settings
  *@band: 0x00-2.4g 0x01-5g
  *@dev_type: 1-wifi ap; 2-wifi sta
  *@num_of_inf: number of interface under this radio for getting conf
  *@inf_info: ifname, e.g ra0/ra1/apcli0; mac_addr: mac addr of this interface
  */

struct GNU_PACKED wps_get_config
{
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char band;
	unsigned char dev_type;
	unsigned char num_of_inf;
	inf_info      inf_data[0];
};
/**
  *@identifier: radio unique identifier of the radio for which capabilities are reported
  *@op_class:
  *@channel:
  *@power:
  */
struct GNU_PACKED ch_config_info
{
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char ifname[IFNAMSIZ];
	unsigned char op_class;
	unsigned char channel;
	signed char power;
	unsigned char reason_code;
};

struct GNU_PACKED channel_setting
{
	unsigned char almac[MAC_ADDR_LEN];
	unsigned char ch_set_num;
	struct ch_config_info chinfo[0];
};

struct GNU_PACKED ch_rep_info {
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char op_class;
	unsigned char channel;
	signed char tx_power;
};

struct GNU_PACKED channel_report
{
	unsigned char ch_rep_num;
	struct ch_rep_info info[0];
};

struct GNU_PACKED tx_power_percentage_setting {
	unsigned char bandIdx;
	unsigned char tx_power_percentage;
};

/**
  * this structure belongs to struct ap_radio_basic_cap
  * @op_class: specifying the global operating class in which the subsequent channel list is valid
  * @max_tx_pwr: maximum transmit power EIRP that this radio is capable of transmitting in the
  * current regulatory domain for the operating class
  * @non_operch_num: number of statically non-operable channels in the operating class
  * @non_operch_list: statically non-operable channels in the operating class
  */
struct GNU_PACKED radio_basic_cap
{
	unsigned char op_class;
	unsigned char max_tx_pwr;
	unsigned char non_operch_num;
	unsigned char non_operch_list[MAX_CH_NUM];
};
/**
  * @identifier: radio unique identifier of the radio for which capabilities are reported
  * @max_bss_num: maximum number of bss supported by this radio
  * @op_class_num: operation class number
  * @opcap: a pointer to struct radio_basic_cap
  */
struct GNU_PACKED ap_radio_basic_cap
{
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char max_bss_num;
	unsigned char band;
	int wireless_mode;
	unsigned char op_class_num;
	struct radio_basic_cap opcap[0];
};

/**
  * @bssid: MAC address of local interface(equal to BSSID) operating on the radio
  */
struct GNU_PACKED op_bss_cap
{
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char ssid_len;
	unsigned char ssid[33];
	unsigned char map_vendor_extension;
	unsigned int auth_mode;
	unsigned int enc_type;
	unsigned char key_len;
	unsigned char key[64 + 1];
};
/**
  * @identifier: radio unique identifier of a radio
  * @identifier: the band of the radio
  * @oper_bss_num: number of bss(802.11 local interfaces) currently operating on the radio
  */
struct GNU_PACKED oper_bss_cap
{
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char oper_bss_num;
	unsigned char band;
	struct op_bss_cap cap[0];
};

#define BIT_BH_STA (1U << (7))
#define BIT_BH_BSS (1U << (6))
#define BIT_FH_BSS (1U << (5))
#define BIT_TEAR_DOWN (1U << (4))

#define OPER_BSS_CAP_LEN 8
/**
  * @ifname: the radio interface name to be config
  * @num: number of wireless setting, can be 1~16
  * @Ssid: the ssid of the bss
  * @AuthMode:
  * @EncrypType:
  * @WPAKey:
  * @map_vendor_extension: map vendor extentsion in M2, 1 byte
  */
struct GNU_PACKED wireless_setting {
	unsigned char mac_addr[MAC_ADDR_LEN];
	unsigned char	Ssid[32 + 1];
	unsigned short	AuthMode;
	unsigned short	EncrypType;
	unsigned char	WPAKey[64 + 1];
	unsigned char map_vendor_extension;    /*store MAP controller's Muiti-AP Vendor Extesion value in M2*/
	unsigned char hidden_ssid;
};

struct GNU_PACKED wsc_config {
	unsigned char num;
	struct wireless_setting setting[0];
};
struct GNU_PACKED bh_priority{
	unsigned char priority;
	unsigned char bh_mac[6];
};

/**
  * @sta_report_on_cop(0 or 1): cap of unassociated sta link metrics reporting on the channels
  * its BSSs arecurrently operating on
  * @sta_report_not_cop(0 or 1): cap of unassociated sta link metrics reporting on the channels
  * its BSSs are not currently operating on
  * @rssi_steer(0 or 1): cap of Agent-initiated RSSI-based Steering
  */
struct GNU_PACKED ap_capability
{
	unsigned char sta_report_on_cop;
	unsigned char sta_report_not_cop;
	unsigned char rssi_steer;
};

/**
  * @identifier: radio unique identifier of the radio for witch HT capabilities are reported
  * @tx_stream: maximum number of supported Tx spatial streams
  * @rx_stream: maximum number of supported Rx spatial streams
  * @sgi_20: short GI support for 20 MHz
  * @sgi_40: short GI support for 40 MHz
  * @ht_40: HT support for 40MHz
  */
struct GNU_PACKED ap_ht_capability
{
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char sgi_20;
	unsigned char sgi_40;
	unsigned char ht_40;
	unsigned char band;
};

/**
  * @identifier: radio unique identifier of the radio for witch HT capabilities are reported
  * @vht_tx_mcs: supported VHT Tx MCS; set to Tx VHT MCS Map field
  * @vht_rx_mcs: supported VHT Rx MCS; set to Rx VHT MCS Map field
  * @tx_stream: maximum number of supported Tx spatial streams
  * @rx_stream: maximum number of supported Rx spatial streams
  * @sgi_80: short GI support for 80 MHz
  * @sgi_160: short GI support for 160 MHz and 80+80MHz
  * @vht_8080: VHT support for 80+80MHz
  * @vht_160: VHT support for 160MHz
  * @su_beamformer: SU Beamformer capable
  * @mu_beamformer: MU Beamformer capable
  */
struct GNU_PACKED ap_vht_capability
{
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned short vht_tx_mcs;
	unsigned short vht_rx_mcs;
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char sgi_80;
	unsigned char sgi_160;
	unsigned char vht_8080;
	unsigned char vht_160;
	unsigned char su_beamformer;
	unsigned char mu_beamformer;
	unsigned char band;
};

struct GNU_PACKED ap_he_capability
{
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char he_mcs_len;
	unsigned char he_mcs[MAX_HE_MCS_LEN];
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char he_8080;
	unsigned char he_160;
	unsigned char su_bf_cap;
	unsigned char mu_bf_cap;
	unsigned char ul_mu_mimo_cap;
	unsigned char ul_mu_mimo_ofdma_cap;
	unsigned char dl_mu_mimo_ofdma_cap;
	unsigned char ul_ofdma_cap;
	unsigned char dl_ofdma_cap;
};

/**
  * this structure belongs to struct ch_prefer
  * @op_class: specifying the global operating class in which the subsequent channel list is valid
  * @ch_num: valid channel num belongs to op_class
  * @ch_list: channel list belongs to op_class
  * @perference: indicate a preference value for the channels in the channel list
  * @reason: indicate reason for the preference
  */
struct GNU_PACKED prefer_info
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char perference;
	unsigned char reason;
};
/**
  * @identifier: radio unique identifier of the radio for witch channel preferences are reported
  * @op_class_num: operation class number
  * @opinfo: a pointer to struct prefer_info
  */
struct GNU_PACKED ch_prefer
{
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char op_class_num;
	struct prefer_info opinfo[0];
};
/**
  * this structure belongs to struct restriction
  * @op_class: specifying the global operating class in which the subsequent channel list is valid
  * @ch_num: valid channel num belongs to op_class
  * @ch_list: channel list belongs to op_class
  * @fre_separation: the minimum frequency separation(in multiples of 10 MHz) that this radio
  * would require when operationg on the above channel number between the center frequency of
  * that channel and the center operating requency of another radio(operating simultaneous TX/RX)
  * of the agent
  */
struct GNU_PACKED restrict_info
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char fre_separation[MAX_CH_NUM];
};
/**
  * @identifier: radio unique identifier of the radio for witch radio operation restrictions are reported
  * @op_class_num: operation class number
  * @opinfo: a pointer to struct restrict_info
  */
struct GNU_PACKED restriction
{
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char op_class_num;
	struct restrict_info opinfo[0];
};

/**
  * @sta_mac: the MAC address of the client
  * @bssid: the BSSID of the BSS operated by the MAP agent for which the event has occurred
  * @assoc_evt: 1 for client has joined the BSS; 0 for client has left the BSS
  * @assoc_time: the time of the 802.11 client's last association to this Multi-AP device. unit jiffies.
  * @assoc_req: association request frame body
  */
struct GNU_PACKED map_client_association_event
{
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char assoc_evt;
	unsigned int assoc_time;
	unsigned char is_APCLI;
	unsigned short assoc_req_len;
	unsigned char assoc_req[0];
};
struct GNU_PACKED map_he_nss{
	u16 nss_80:2;
	u16 nss_160:2;
	u16 nss_8080:2;
};

struct GNU_PACKED map_priv_cli_cap
{
	u16 bw:2;
	u16 phy_mode:3;
	u16 nss:2;
	u16 btm_capable:1;
	u16 rrm_capable:1;
	u16 mbo_capable:1;
	struct map_he_nss nss_he;
};

struct GNU_PACKED client_association_event
{
  	struct map_priv_cli_cap cli_caps;
	struct map_client_association_event map_assoc_evt;
};
#ifdef MAP_R2
struct GNU_PACKED client_disassociation_stats_event
{
  	u8 mac_addr[MAC_ADDR_LEN];
	u16 reason_code;
};
#endif
/**
  * @bssid: the BSSID of a BSS
  * @sta_mac: the MAC address of the client
  */
struct GNU_PACKED client_info
{
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char sta_mac[MAC_ADDR_LEN];
};
/**
  * @result: result code for the client capability report message
  * @length: the length of frame body
  * @body: the frame body of the most recently received (re)association request frame
  * from this client. if result code is not equal to 0, this field is omitted
  */
struct GNU_PACKED client_capa_rep
{
	unsigned char result;
	unsigned short length;
	unsigned char body[0];
};


/**
  * @dest_al_mac: dest al mac
  * @type: 1905 frame type, e.g High layer Data Message 0x8018
  * @len: the length frame body, for High layer Data Message, it must not be 0,
  	       for other type of 1905 cmdu(e.g topology discovery, there's no frame content, it may be 0)
  * @body: frame body
  */
struct GNU_PACKED _1905_cmdu_request
{
	unsigned char dest_al_mac[MAC_ADDR_LEN];
	unsigned short type;
	unsigned short len;
	unsigned char body[0];
};

/**
  * @target_bssid: indicates a target BSSID for steering. wildcard BSSID is represented by ff:ff:ff:ff:ff:ff
  * @op_class: target BSS operating class
  * @channel: target BSS channel number for channel on which the target BSS is transmitting beacon frames
  * @sta_mac: sta mac address for which the steering request applies. if sta_count of struct steer_request
  * is 0, then this field is invalid
  */
struct GNU_PACKED target_bssid_info {
	unsigned char target_bssid[MAC_ADDR_LEN];
	unsigned char op_class;
	unsigned char channel;
	unsigned char sta_mac[MAC_ADDR_LEN];
#ifdef MAP_R2
	unsigned char reason;
#endif
};

/**
  * @assoc_bssid: unique identifier of the source BSS for whirch the steering request applies
  * (i.e. BSS that the STAs specified in the request are currently associated with)
  * @request_mode: 0: request is a steering opportunity; 1: request is a steering mandate to
  * trigger steering for specific client STA(s)
  * @btm_disassoc_immi: BTM Disassociation imminent bit
  * @btm_abridged: BTM Abridged bit
  * @steer_window: steering opportunity window. time period in seconds(from reception of the
  * steering request message)for whitch the request is valid.
  * @btm_disassoc_timer: BTM Disassociation timer. time period in TUs for disassociation timer
  * in BTM request
  * @sta_count: sta list count. k=0: steering request applies to all associated STAs in the BSS per
  * policy setting; k>0: steering request applies to specific STAs specific by STA MAC address(es)
  * @target_bssid_count: target BSSID list count. only valid when request_mode is set to 1.
  * m=1: the same target BSSID is indicated for all specified STAs;m=k: an individual target BSSID
  * is indicated for each apecified STA(in same order)
  * @steer_info: contains k sta mac address and m target bssid info
  */
struct GNU_PACKED steer_request {
	unsigned char assoc_bssid[MAC_ADDR_LEN];
	unsigned char request_mode;
	unsigned char btm_disassoc_immi;
	unsigned char btm_abridged;
	unsigned short steer_window;
	unsigned short btm_disassoc_timer;
	unsigned char sta_count;
	unsigned char target_bssid_count;
#ifdef MAP_R2
	unsigned char steering_type; //0-Legacy, 1-R2
#endif
	struct target_bssid_info info[0];
};

/**
  * @bssid: unique identifier of the source BSS for which the steering BTM report applies
  * @sta_mac: sta mac address for which the steering BTM report applies
  * @status: indicates the value of the BTM status code as reported by the STA in the BTM response
  * @tbssid: indicates the value of the target BSSID field(if present)in the BTM response received
  * from the STA. Note: this indicates the BSSID that the STA intends to roam to, whicg may not
  * align with the target BSSID specified in the BTM request.
  */
struct GNU_PACKED cli_steer_btm_event {
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char status;
	unsigned char tbssid[MAC_ADDR_LEN];
};

/**
  * @bssid: unique identifier of the BSS for which the client blocking request applies
  * @assoc_control: indicates if the request is to block or unblock the indicated STAs from associating
  * @valid_period: time period in seconds(from reception of the client association control request message)
  * for which a blocking request is valid
  * @sta_list_count: indicateing one or more STAs  for which the client association control request applies
  * @sta_mac: sta mac address for which he client association control request applies
  */
struct GNU_PACKED cli_assoc_control {
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char assoc_control;
	unsigned short valid_period;
	unsigned char sta_list_count;
	unsigned char sta_mac[0];
};

struct GNU_PACKED local_disallow_sta_head {
	unsigned char sta_cnt;
	unsigned char sta_list[0];
};
struct GNU_PACKED btm_disallow_sta_head {
	unsigned char sta_cnt;
	unsigned char sta_list[0];
};
struct GNU_PACKED radio_policy_head {
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char policy;
	unsigned char ch_ultil_thres;
	unsigned char rssi_thres;
};
struct GNU_PACKED radio_policy {
	unsigned char radio_cnt;
	struct radio_policy_head radio[0];
};

struct GNU_PACKED metric_policy_head {
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char rssi_thres;
	unsigned char hysteresis_margin;
	unsigned char ch_util_thres;
	unsigned char sta_stats_inclusion;
	unsigned char sta_metrics_inclusion;
};
struct GNU_PACKED metric_policy {
	unsigned char report_interval;
	unsigned char policy_cnt;
	struct metric_policy_head policy[0];
};


/**
  * @backhaul_mac: the mac address of associated backhaul station operated by the multi-ap agent
  * @target_bssid: the bssid of the target BSS
  * @oper_class: operating class
  * @channel: channel number on which beacon frames are being transmitted by the target BSS
  */
struct GNU_PACKED backhaul_steer_request {
	unsigned char backhaul_mac[MAC_ADDR_LEN];
	unsigned char target_bssid[MAC_ADDR_LEN];
	unsigned char oper_class;
	unsigned char channel;
};


struct GNU_PACKED backhaul_steer_request_extended {
	unsigned char target_ssid[32];
	unsigned char ssid_len;
	struct backhaul_steer_request request;
};
struct GNU_PACKED backhaul_connect_request {
	unsigned char backhaul_mac[MAC_ADDR_LEN];
	unsigned char target_bssid[MAC_ADDR_LEN];
	unsigned char target_ssid[33];
	unsigned short AuthType;
	unsigned short EncrType;
	unsigned char Key[64];
	unsigned char oper_class;
	unsigned char channel;
};

/**
  * @backhaul_mac: the mac address of associated backhaul station operated by the multi-ap agent
  * @target_bssid: the bssid of the target BSS
  * @status: status code
  */
struct GNU_PACKED backhaul_steer_rsp {
	unsigned char backhaul_mac[MAC_ADDR_LEN];
	unsigned char target_bssid[MAC_ADDR_LEN];
	unsigned char status;
};

/*link metric collection*/
struct GNU_PACKED esp_info {
	unsigned char ac;
	unsigned char format;
	unsigned char ba_win_size;
	unsigned char e_air_time_fraction;
	unsigned char ppdu_dur_target;
};

#ifdef MAP_R2
struct GNU_PACKED extended_ap_metrics {
	u32 uc_tx;
	u32 uc_rx;
	u32 mc_tx;
	u32 mc_rx;
	u32 bc_tx;
	u32 bc_rx;
};
#endif

/**
  * @bssid: bssid of a bss operated by the multi-ap agent for which the metrics are reported
  * @ch_util: channel utilization as mesured by the radio operating the bss
  * @assoc_sta_cnt: indicates the total number of STAs currently associated with the BSS
  * @esp: estimated service parameters information field.
  */
struct GNU_PACKED ap_metrics_info {
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char ch_util;
	unsigned short assoc_sta_cnt;
	unsigned char valid_esp_count;
#ifdef MAP_R2
	struct extended_ap_metrics ext_ap_metric;
#endif
	struct esp_info esp[0];
};
#ifdef MAP_R2
struct GNU_PACKED radio_metrics_info {
	u8 ra_id[MAC_ADDR_LEN];
	u8 cu_noise;
	u8 cu_tx;
	u8 cu_rx;
	u8 cu_other;
	u32 edcca;
};
#endif

/**
  * @mac: MAC address of the associated STA
  * @bytes_sent: raw counter of the number of bytes sent to the associated STA
  * @bytes_received: raw counter of number of bytes received from the associated STA
  * @packets_sent: raw counter of the number of packets successfully sent to the associated STA
  * @packets_received: raw counter of the number of packets received from the associated STA
  * during the measurement window
  * @tx_packets_errors: raw counter of the number of packets which could not be transmitted to
  * the associated STA due to errors
  * @rx_packets_errors: raw counter of the number of packets which were received in error from
  * the associated STA
  * @retransmission_count: raw counter of the number of packets sent with the retry flag set to
  * the associated STA
  */
struct GNU_PACKED stat_info {
	unsigned char mac[MAC_ADDR_LEN];
	unsigned int bytes_sent;
	unsigned int bytes_received;
	unsigned int packets_sent;
	unsigned int packets_received;
	unsigned int tx_packets_errors;
	unsigned int rx_packets_errors;
	unsigned int retransmission_count;
	unsigned char is_APCLI;
};

/**
  * @identifier: radio unique identifier of the radio for which sta are associated
  * @sta_cnt: the total number of sta which traffic stats are reported
  * @stats: sta traffic stats info
  */
struct GNU_PACKED sta_traffic_stats {
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char sta_cnt;
	struct stat_info stats[0];
};

/**
  * @mac: MAC address of the associated STA
  * @bssid: BSSID of the BSS for which the STA is associated
  * @time_delta: The time delta in ms between the time at which the earliest measurement that
  * contributed to the data rate estimates were made, and the time at which this report was sent
  * @erate_downlink: Estimated MAC Data Rate in downlink (in Mb/s)
  * @erate_uplink: Estimated MAC Data Rate in uplink (in Mb/s)
  * @rssi_uplink: Measured uplink RSSI for STA (dBm)
  */
struct GNU_PACKED link_metrics {
	unsigned char mac[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned int time_delta;
	unsigned int erate_downlink;
	unsigned int erate_uplink;
	unsigned char rssi_uplink;
	unsigned char is_APCLI;
};

/**
  * @identifier: radio unique identifier of the radio for which sta are associated
  * @sta_cnt: the total number of sta which link metrics are reported
  * @info: sta link metrics info
  */
struct GNU_PACKED sta_link_metrics {
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char sta_cnt;
	struct link_metrics info[0];
};
#ifdef MAP_R2
struct GNU_PACKED ext_link_metrics {
	unsigned char mac[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
	u32 last_data_ul_rate;
	u32 last_data_dl_rate;
	u32 utilization_rx;
	u32 utilization_tx;
};

struct GNU_PACKED ext_sta_link_metrics {
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char sta_cnt;
	struct ext_link_metrics info[0];
};
#endif
/**
  * a struct to query tx or rx link metrics info of 1905
  * @media_type: wifi or eth interface
  * @local_if: MAC address of an interface in the receiving IEEE 1905.1 AL, which connects to
  * an interface in the neighbor IEEE 1905.1 AL
  * @neighbor_if: MAC address of an IEEE 1905.1 interface in a neighbor IEEE 1905.1 device
  * which connects to an IEEE 1905.1 interface in the receiving IEEE 1905.1 device
  */
struct GNU_PACKED link_stat_query {
	unsigned char media_type;
	unsigned char local_if[MAC_ADDR_LEN];
	unsigned char neighbor_if[MAC_ADDR_LEN];
};

/**
  * @pkt_errs: wifi or eth interface
  * @local_if: Estimated number of lost packets on the transmit side of the link during the
  * measurement period
  * @tx_pkts: Estimated number of packets transmitted by the Transmitter of the link on
  * the same measurement period
  * @mac_tp_cap: The maximum MAC throughput of the Link estimated at the transmitter
  * and expressed in Mb/s
  * @link_avail: The estimated average percentage of time that the link is available for data
  * transmission
  * @phyrate: If the media type of the link is IEEE 802.3, then IEEE 1901 or MoCA 1.1
  * This value is the PHY rate estimated at the transmitter of the link expressed in Mb/s;
  * otherwise, it is set to 0xFFFF.
  */
struct GNU_PACKED tx_link_stat_rsp {
	unsigned int pkt_errs;
	unsigned int tx_pkts;
	unsigned short mac_tp_cap;
	unsigned short link_avail;
	unsigned short phyrate;
	unsigned int tx_tp;
};

/**
  * @pkt_errs: wifi or eth interface
  * @local_if: Estimated number of lost packets on the receive side of the link during the
  * measurement period
  * @rx_pkts: Estimated number of packets transmitted by the Transmitter of the link on
  * the same measurement period
  * @rssi: If the media type of the link is IEEE 802.11, then this value is the estimated RSSI
  * in dB at the receive side of the Link expressed in dB; otherwise, it is set to 0xFF.
  */
struct GNU_PACKED rx_link_stat_rsp {
	unsigned int pkt_errs;
	unsigned int rx_pkts;
	unsigned char rssi;
	unsigned int rx_tp;
};

struct GNU_PACKED unlink_metrics_query {
	unsigned char oper_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char sta_num;
	unsigned char sta_list[0];
};

struct GNU_PACKED unlink_rsp_sta {
	unsigned char mac[MAC_ADDR_LEN];
	unsigned char ch;
	unsigned int time_delta;
	signed char uplink_rssi;
};

struct GNU_PACKED unlink_metrics_rsp {
	unsigned char oper_class;
	unsigned char sta_num;
	struct unlink_rsp_sta info[0];
};

struct GNU_PACKED ap_chn_rpt {
	unsigned char ch_rpt_len;
	unsigned char oper_class;
	unsigned char ch_list[MAX_CH_NUM];
};

struct GNU_PACKED beacon_metrics_query {
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char oper_class;
	unsigned char ch;
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char rpt_detail_val;
	unsigned char ssid_len;
	unsigned char ssid[33];
	unsigned char elemnt_num;
	unsigned char elemnt_list[MAX_ELEMNT_NUM];
	unsigned char ap_ch_rpt_num;
	struct ap_chn_rpt rpt[0];
};


struct GNU_PACKED beacon_metrics_rsp {
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char reserved;
	unsigned char bcn_rpt_num;
	unsigned short rpt_len;
	unsigned char rpt[0];
};

struct GNU_PACKED topo_info {
	unsigned char almac[MAC_ADDR_LEN];
	unsigned char bssid_num;
	unsigned char bssid[0];
};

struct GNU_PACKED csa_info_rsp {
	unsigned char ruid[MAC_ADDR_LEN];
	unsigned char new_ch;
};

struct GNU_PACKED interface_info {
	unsigned char if_name[IFNAMSIZ];
	unsigned char if_role[6]; /* wiap,wista */
	unsigned char if_ch;      /* channel */
	unsigned char if_phymode[3];  /* n,ac */
	unsigned char if_mac_addr[MAC_ADDR_LEN];
	unsigned char identifier[MAC_ADDR_LEN];/*belong to which radio*/

};

struct GNU_PACKED interface_info_list_hdr {
	unsigned char interface_count;
	struct interface_info if_info[0];
};

struct GNU_PACKED bh_assoc_wireless_setting {
	unsigned char   bh_mac_addr[MAC_ADDR_LEN];
	unsigned char	target_ssid[33];
	unsigned char 	target_bssid[MAC_ADDR_LEN];
	unsigned short  auth_mode;
	unsigned short	encryp_type;
	unsigned char	wpa_key[65];
	unsigned char   target_channel;
};

typedef struct GNU_PACKED wapp_sta_activity_status {
	uint8_t status;
	unsigned char sta_mac[MAC_ADDR_LEN];
} wapp_sta_status_info;

struct GNU_PACKED tp_metrics {
	unsigned char mac[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned int tx_tp;
	unsigned int rx_tp;
	unsigned char is_APCLI;
};

struct GNU_PACKED sta_tp_metrics {
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char sta_cnt;
	struct tp_metrics info[0];
};

struct GNU_PACKED bssload_info {
	unsigned short sta_cnt;
	unsigned char ch_util;
	unsigned short AvalAdmCap;
};

struct GNU_PACKED CONNECT_FAILURE_REASON {
	unsigned char	connect_stage;
	unsigned short	reason;
} ;

struct GNU_PACKED sta_cnnct_rej_info {
	unsigned int interface_index;
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
	struct CONNECT_FAILURE_REASON cnnct_fail;
#ifdef MAP_R2
	unsigned short assoc_status_code;
	unsigned short assoc_reason_code;
	unsigned short send_failed_assoc_frame;
#endif
};

struct GNU_PACKED bssload_crossing_info {
	unsigned int interface_index;
	unsigned char bssload_high_thrd;
	unsigned char bssload_low_thrd;
	unsigned char bssload;
};

struct GNU_PACKED bss_state_info {
	unsigned int interface_index;
	unsigned char bss_state;
};

struct GNU_PACKED apcli_association_info {
	unsigned int interface_index;
	unsigned char apcli_assoc_state;
	char rssi;
	unsigned char current_channel;
	unsigned char peer_map_enable;
};

#define PREQ_IE_LEN 128
struct GNU_PACKED probe {
	unsigned char mac_addr[MAC_ADDR_LEN];
	unsigned char channel;
	char rssi;
	unsigned char preq_len;
	unsigned char preq[PREQ_IE_LEN];
};

struct GNU_PACKED enrollee_bh {
	unsigned char if_type;    				/*0-eth 1-wireless*/
	unsigned char mac_address[MAC_ADDR_LEN];    /*mac address of the interface*/
};

struct GNU_PACKED bss_role {
	unsigned char bssid[MAC_ADDR_LEN];			/*bssid*/
	unsigned char role;                     /*the bss type, could be BIT_BH_BSS, BIT_FH_BSS and BIT_BH_BSS|BIT_FH_BSS*/
};

struct GNU_PACKED trigger_wps_param {
	unsigned char if_mac[MAC_ADDR_LEN];         /*interface mac address on which the wps trggers on */
	unsigned char mode;                     /*WPS mode, 2-PBC */
};
struct GNU_PACKED garp_src_addr
{
	unsigned char addr[6];
};

struct GNU_PACKED garp_dev_addr
{
	unsigned char addr[6];
};

/*if you need change this value, need keep align with mapd*/
#define MAX_BHDEV_CNT 10

struct GNU_PACKED garp_req_s
{
	unsigned char dev_cnt;
	unsigned int sta_count;
	struct garp_dev_addr dev_addr_list[MAX_BHDEV_CNT];
	struct garp_src_addr mac_addr_list[0];
};

struct GNU_PACKED dhcp_ctl_req
{
	unsigned int dhcp_server_enable;
	unsigned int dhcp_client_enable;
};

#ifdef MAP_R2
#define MAX_VLAN_NUM 	48
#define VLAN_N_VID		4095


struct GNU_PACKED ts_fh_config {
	unsigned char itf_mac[6];
	unsigned short vid;
};

struct GNU_PACKED ts_fh_bss_setting {
	unsigned char itf_num;
	struct ts_fh_config fh_configs[0];
};

struct GNU_PACKED ts_common_setting {
	unsigned short primary_vid;
	unsigned char primary_pcp;
	unsigned char policy_vid_num;
	unsigned short policy_vids[MAX_VLAN_NUM];
};

struct GNU_PACKED ts_setting {
	struct ts_common_setting common_setting;
	struct ts_fh_bss_setting fh_bss_setting;
};

struct GNU_PACKED trans_vlan_config {
	unsigned char trans_vid_num;
	unsigned short vids[128];
};

struct GNU_PACKED trans_vlan_setting {
	struct trans_vlan_config trans_vlan_configs;
	unsigned char apply_itf_num;
	unsigned char apply_itf_mac[0];
};
#endif

#ifdef KV_API_SUPPORT
#define MAX_CANDIDATE_NUM 5
#define OP_LEN 16
#define CH_LEN 30
#define REQ_LEN 30
#define MAC_ADDR_LEN 6
#define SSID_LEN 33
#define URL_LEN 40


#define OID_GET_SET_TOGGLE		0x8000
#define	OID_GET_SET_FROM_UI		0x4000
#define OID_802_11_WNM_COMMAND  0x094A
#define OID_802_11_WNM_EVENT	0x094B
#define OID_802_11_RRM_COMMAND  0x094C
#define OID_802_11_RRM_EVENT	0x094D

enum rrm_cmd_subid {
	OID_802_11_RRM_CMD_ENABLE = 0x01,
	OID_802_11_RRM_CMD_CAP,
	OID_802_11_RRM_CMD_SEND_BEACON_REQ,
	OID_802_11_RRM_CMD_QUERY_CAP,
	OID_802_11_RRM_CMD_SET_BEACON_REQ_PARAM,
	OID_802_11_RRM_CMD_SEND_NEIGHBOR_REPORT,
	OID_802_11_RRM_CMD_SET_NEIGHBOR_REPORT_PARAM,
	OID_802_11_RRM_CMD_HANDLE_NEIGHBOR_REQUEST_BY_DAEMON,
};

typedef struct GNU_PACKED rrm_command_s {
	unsigned char command_id;
	unsigned int command_len;
	unsigned char command_body[0];
} rrm_command_t, *p_rrm_command_t;

/** @peer_address: mandatory; sta to send beacon request frame;
	@num_rpt: optional; number of repetitions;
	@regclass: only mandatory when channel is set to 0; operating class;
	@channum: mandatory; channel number;
	@random_ivl: optional; randomization interval; unit ms;
	the upper bound of the random delay to be used prior to make measurement;
	@duration: optional; measurement duration; unit ms;
	@bssid: optional;
	@mode: optional; measurement mode;
	As default value 0 is a valid value in spec, so here need remap the value and the meaning;
	1 for passive mode; 2 for active mode; 3 for beacon table;
	@req_ssid: optional; subelement SSID;
	@timeout: optional; unit s;
	@rep_conditon: optional; subelement Beacon Reporting Information;
	@ref_value: optional; subelement Beacon Reporting Information;
	condition for report to be issued;
	driver will send timeout event after timeout value if no beacon report received;
	@detail: optional; subelement Reporting Detail;
	As default value 0 is a valid value in spec, so here need remap the value and the meaning;
	1 for no fixed length fields or elements;
	2 for all fixed length fields and any requested elements in the request IE;
	3 for all fixed length fields and elements;
	@op_class_len:  mandatory only when channel is set to 255;
	@op_class_list: subelement Ap Channel Report;
	@ch_list_len: mandatory only when channel is set to 255;
	@ch_list: subelement Ap Channel Report;
	if you want use all the channels in operating classes then use default value
	otherwise specify all channels you want sta to do measurement
	@request_len: optional;
	@request: subelement Request; only vaild when you specify request IDs
*/
typedef struct GNU_PACKED bcn_req_info_s {
	unsigned char peer_address[6];
	unsigned short num_rpt;
	unsigned char regclass;
	unsigned char channum;
	unsigned short random_ivl;
	unsigned short duration;
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char mode;
	unsigned char req_ssid_len;
	unsigned char req_ssid[SSID_LEN];
	unsigned int timeout;
	unsigned char rep_conditon;
	unsigned char ref_value;
	unsigned char detail;
	unsigned char op_class_len;
	unsigned char op_class_list[OP_LEN];
	unsigned char ch_list_len;
	unsigned char ch_list[CH_LEN];
	unsigned char request_len;
	unsigned char request[REQ_LEN];
} bcn_req_info, *p_bcn_req_info;


enum wnm_cmd_subid {
	OID_802_11_WNM_CMD_ENABLE = 0x01,
	OID_802_11_WNM_CMD_CAP,
	OID_802_11_WNM_CMD_SEND_BTM_REQ,
	OID_802_11_WNM_CMD_QUERY_BTM_CAP,
	OID_802_11_WNM_CMD_SEND_BTM_REQ_IE,
	OID_802_11_WNM_CMD_SET_BTM_REQ_PARAM,
};

struct GNU_PACKED wnm_command {
	unsigned char command_id;
	unsigned char command_len;
	unsigned char command_body[0];
};


/**
	@channum: mandatory; channel number;
	@phytype: optional; PHY type;
	@regclass: optional; operating class;
	@capinfo: optional; Same as AP's Capabilities Information field in Beacon;
	@bssid: mandatory;
	@preference: not used in neighbor report; optional in btm request;
		indicates the network preference for BSS transition to the BSS listed in this
		BSS Transition Candidate List Entries; 0 is a valid value in spec, but here
		need remap its meaning to not include preference IE in neighbor report
		response frame;
	@is_ht:  optional; High Throughput;
	@is_vht: optional; Very High Throughput;
	@ap_reachability: optional; indicates whether the AP identified by this BSSID is
		reachable by the STA that requested the neighbor report. For example,
		the AP identified by this BSSID is reachable for the exchange of
		preauthentication frames;
	@security: optional;  indicates whether the AP identified by this BSSID supports
		the same security provisioning as used by the STA in its current association;
	@key_scope: optional; indicates whether the AP indicated by this BSSID has the
		same authenticator as the AP sending the report;
	@Mobility: optional; indicate whether the AP represented by this BSSID is
		including an MDE in its Beacon frames and that the contents of that MDE are
 		identical to the MDE advertised by the AP sending the report;
*/
struct GNU_PACKED nr_info {
	unsigned char channum;
	unsigned char phytype;
	unsigned char regclass;
	unsigned short capinfo;
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char preference;
	unsigned char is_ht;
	unsigned char is_vht;
	unsigned char ap_reachability;
	unsigned char security;
	unsigned char key_scope;
	unsigned char mobility;
};


/**
	@sta_mac: mandatory; mac of sta sending the frame;
	@dialogtoken: optional; dialog token;
	@reqmode: optional; request mode;
	@disassoc_timer: optional; the time(TBTTs) after which the AP will issue
		a Disassociation frame to this STA;
	@valint: optional;  the number of beacon transmission times (TBTTs) until
		the BSS transition candidate list is no longer valid;
	@timeout: optional; driver will send timeout event after timeout value
		if no beacon report received; unit s;
	@TSF: optional; BSS Termination TSF;
	@duration: optional; number of minutes for which the BSS is not present;
	@url_len: optional;
	@url: optional; only vaild when you specify url;
	@num_candidates: mandatory; num of candidates;
	@candidates: mandatory; request mode; the num of candidate is no larger
		than 5;
*/
typedef struct GNU_PACKED btm_reqinfo_s {
	unsigned char sta_mac [MAC_ADDR_LEN];
	unsigned char dialogtoken;
	unsigned char reqmode;
	unsigned short disassoc_timer;
	unsigned char valint;
	unsigned int timeout;
	unsigned long long TSF;
	unsigned short duration;
	unsigned char url_len;
	unsigned char url[URL_LEN];
	unsigned char num_candidates;
	struct nr_info candidates[0];
}btm_reqinfo_t, *p_btm_reqinfo_t;

typedef struct GNU_PACKED btm_req_ie_data_s {
	unsigned int ifindex;
	unsigned char peer_mac_addr[6];
	unsigned char dialog_token;
	unsigned int timeout;
	unsigned int btm_req_len;
	unsigned char btm_req[0];
}btm_req_ie_data_t, *p_btm_req_ie_data_t;
#endif /* KV_API_SUPPORT */

#ifdef DPP_SUPPORT
struct GNU_PACKED dpp_bootstrap_uri_info {
	unsigned char identifier[MAC_ADDR_LEN];
	unsigned char local_intf_mac[MAC_ADDR_LEN];
	unsigned char sta_mac[MAC_ADDR_LEN];
};

struct GNU_PACKED dpp_tlv_opclass_info {
	unsigned char opclass;
	unsigned char channel_num;
	unsigned char channel[4];
};

struct GNU_PACKED dpp_tlv_info {
	unsigned char final_dest_flag;
	unsigned char chn_list_flag;
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char opclass_num;
	struct dpp_tlv_opclass_info opclass[10];
};

struct GNU_PACKED dpp_msg
{
	unsigned char almac[MAC_ADDR_LEN];
	unsigned char frame_type;
	struct dpp_tlv_info dpp_info;
	unsigned short payload_len;
	unsigned char payload[0];
};
#endif /*DPP_SUPPORT*/
#ifdef ACL_CTRL
struct GNU_PACKED acl_ctrl {
	unsigned char type;
	unsigned char cmd;
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char sta_list_count;
	unsigned char sta_mac[0];
};
#endif /*ACL_CTRL*/
#endif /*INTERFACE_H*/
