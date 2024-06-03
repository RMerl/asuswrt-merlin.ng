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
#ifndef __DATA_DEF_H__
#define __DATA_DEF_H__

#include <linux/if_ether.h>
#include <net/if.h>
#include "os.h"

#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */



/*commands that are only defined between 1905.1 daemon and 1905.1 library*/
enum {
	WAPP_AP_METRICS_RSP_INFO=0xC0,
	WAPP_LINK_METRICS_RSP_INFO,
	WAPP_CHANNEL_SELECTION_RSP_INFO,
	WAPP_CHANNEL_PREFERENCE_REPORT_INFO,
	MANAGEMENT_1905_COMMAND,
	WAPP_NOTIFY_WIRELESS_BH_LINK_DOWN,
	WAPP_GET_OWN_TOPOLOGY_RSP,
	GET_SWITCH_STATUS,
	WAPP_NOTIFY_CH_BW_INFO,
};

/*control command that are used to control 1905 daemon*/
enum {
	MANAGE_SET_ROLE,
	MANAGE_SET_LOG_LEVEL,
	MANAGE_GET_LOCAL_DEVINFO,
	MANAGE_SET_BSS_CONF,
};

enum BAND {
	CONFIG_BAND_2G = 0x01,
	CONFIG_BAND_5GL = 0x02,
	CONFIG_BAND_5GH_CAP = 0x04
};

/*WSC Encryption type, defined in wsc 2.0.2 p.114*/
enum ENCRYPTION_TYPE {
	CONFIG_ENCRYP_NONE = 0x0001,
	CONFIG_ENCRYP_WEP = 0x0002,
	CONFIG_ENCRYP_TKIP = 0x0004,
	CONFIG_ENCRYP_AES = 0x0008
};

/*WSC Authentication type, defined in wsc 2.0.2 p.105*/
enum AUTH_MODE {
	CONFIG_AUTH_OPEN = 0x0001,
	CONFIG_AUTH_WPA_PERSONAL = 0x0002,
	CONFIG_AUTH_SHARED = 0x0004,
	CONFIG_AUTH_WPA_ENTERPRISE = 0x0008,
	CONFIG_AUTH_WPA2_ENTERPRISE = 0x0010,
	CONFIG_AUTH_WPA2_PERSONAL = 0x0020
};

enum BSS_CONFIG_OPERATION {
	BSS_ADD,
	BSS_RESET_ADD,
};

/**
  * @identifier: radio unique identifier of the radio
  * @transmit_power_limit: transmit power limit, this field is coded as a 2s comlement signed
  * interger in units of decibels relative to 1mW(dBm)
  */
struct GNU_PACKED transmit_power_limit
{
	unsigned char identifier[ETH_ALEN];
	unsigned char transmit_power_limit;
};



#define MAX_CH_NUM 13
#define MAX_ELEMNT_NUM 4
#define MAX_OP_CLASS_NUM 64
#define MAX_OP_CLASS_NUM_20M 6
#define MAX_LINK_NUM 2
#define MAX_INF_NUM 20



/**
  * @identifier: radio unique identifier of the radio for witch channel preferences are reported
  * @op_class_num: operation class number
  * @opinfo: a pointer to struct prefer_info
  */
struct GNU_PACKED ch_prefer_lib
{
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class_num;
	struct prefer_info opinfo[MAX_OP_CLASS_NUM];
};


/**
  * @identifier: radio unique identifier of the radio for witch radio operation restrictions are reported
  * @op_class_num: operation class number
  * @opinfo: a pointer to struct restrict_info
  */
struct GNU_PACKED restriction_lib
{
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class_num;
	struct restrict_info opinfo[MAX_OP_CLASS_NUM];
};

/**
  * @bssid: bssid of a bss operated by the multi-ap agent for which the metrics are reported
  * @ch_util: channel utilization as mesured by the radio operating the bss
  * @assoc_sta_cnt: indicates the total number of STAs currently associated with the BSS
  * @esp: estimated service parameters information field.
  */
struct GNU_PACKED ap_metrics_info_lib {
	unsigned char bssid[ETH_ALEN];
	unsigned char ch_util;
	unsigned short assoc_sta_cnt;
	unsigned char valid_esp_count;
	struct esp_info esp[12];
};



struct GNU_PACKED tx_link_metrics_sub {
	unsigned char mac_address[ETH_ALEN];
	unsigned char mac_address_neighbor[ETH_ALEN];
	/*Table 6-18��IEEE 1905.1 transmitter link metrics*/
	unsigned short intftype;
	unsigned char ieee80211_bridgeflg;
	unsigned int packetErrors;
	unsigned int transmittedPackets;
	unsigned short macThroughputCapacity;
	unsigned short linkAvailability;
	unsigned short phyRate;
};

struct GNU_PACKED tx_link_metrics {
	unsigned char almac[ETH_ALEN];
	unsigned char neighbor_almac[ETH_ALEN];
	unsigned short link_pair_cnt;
	struct tx_link_metrics_sub metrics[2];
};

struct GNU_PACKED rx_link_metrics_sub {
	unsigned char mac_address[ETH_ALEN];
	unsigned char mac_address_neighbor[ETH_ALEN];
	/*Table 6-18��IEEE 1905.1 transmitter link metrics*/
	unsigned short intftype;
	unsigned int packetErrors;
	unsigned int packetsReceived;
	unsigned char rssi;
};

struct GNU_PACKED rx_link_metrics {
	unsigned char almac[ETH_ALEN];
	unsigned char neighbor_almac[ETH_ALEN];
	unsigned short link_pair_cnt;
	struct rx_link_metrics_sub metrics[2];
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
* @rx_pkts: Estimated number of packets transmitted by the Transmitter of the link on
  * the same measurement period
  * @rssi: If the media type of the link is IEEE 802.11, then this value is the estimated RSSI
  */
struct GNU_PACKED link_stat_rsp {
	unsigned int pkt_errs_tx;
	unsigned int tx_pkts;
	unsigned short mac_tp_cap;
	unsigned short link_avail;
	unsigned short phyrate;
	unsigned int pkt_errs_rx;
	unsigned int rx_pkts;
	unsigned char rssi;

};


struct GNU_PACKED beacon_metrics_rsp_lib {
	unsigned char sta_mac[ETH_ALEN];
	unsigned char status;
	unsigned char bcn_rpt_num;
	unsigned short rpt_len;
	unsigned char rpt[0];
};



/**
  * @identifier: radio unique identifier of an AP radio for which steering policies are being provided.
  * @steer_policy: 0x00 means agent initiated steering disallowed; 0x01 means agent initiated RSSI-based steering mandated; 0x02 means agent initiated RSSI-based steering allowed; 0x03 ~ 0xff are reserved values.
  * @ch_util_thres: channel utilization threshold.
  * @rssi_thres: RSSI steering threshold.
  */
struct GNU_PACKED lib_steer_radio_policy
{
	unsigned char identifier[ETH_ALEN];
	unsigned char steer_policy;
	unsigned char ch_util_thres;
    unsigned char rssi_thres;
};

/**
  * @identifier: radio unique identifier.
  * @rssi_thres: STA metrics reporting RSSI threshold (dBm). 0 means do not report STA metrics based on RSSI threshold; 1~220 means RSSI threshold value; 221 ~ 255 are reserved values.
   * @rssi_margin: STA metrics reporting RSSI hysteresis margin override. 0 means use agent's implementation-specific default RSSI hysteresis margin; 1~255 means RSSI hysteresis margin value.
   * @ ch_util_thres: AP metrics channel utilization reporting threshold. 0 means do not report AP metrics based on channel utilization threshold; 1~255 means AP metrics channel utilization reporting threshold.
  * @ traffic_inclusion: associated STA traffic stats inclusion policy. 0 means do not include associated STA traffic stats TLV in AP metrics response; 1 means Include associated STA traffic stats TLV in AP metrics response; 2 ~ 255 are reserved values.
  * @ metrics_inclusion: associated STA link metrics inclusion policy. 0 means do not include associated STA link metrics TLV in AP metrics response; 1 means include associated STA link metrics TLV in AP metrics response; 2 ~ 255 are reserved values.
  */
struct GNU_PACKED lib_metrics_radio_policy
{
	unsigned char identifier[ETH_ALEN];
	unsigned char rssi_thres;
    unsigned char rssi_margin;
	unsigned char ch_util_thres;
    unsigned char traffic_inclusion;
    unsigned char metrics_inclusion;
};

#ifdef MAP_R2

/**
  * @report_switch: Indicates whether Multi-AP Agent should report unsuccessful association attempts of client STAs to the Multi-AP Controller
	   0: Do not report unsuccessful association attempts
	   1: Report unsuccessful association attempts
  * @rssi_thres: Maximum rate for reporting unsuccessful association attempts (in attempts per minute)
  */
struct GNU_PACKED lib_unsuccess_assoc_policy
{
	unsigned char report_switch;
	unsigned char report_rate;
};
#endif // #ifdef MAP_R2


/**
  * @bssid:  6 bytes of bss MAC address that the STAs specified in the request are currently associated with.
  * @mode: request mode. If bit 7 set to 1, it means request is a steering mandate to trigger steering for specific client STA(s). If bit 7 set to 0, it means request is a steering opportunity. BTM disassociation Imminent bit shown with bit 6 set. BTM abridged bit shown with bit 5 set. bit 0-4 are reserved.
  * @window: steering opportunity window. Time period in seconds (from reception of the steering request message) for which the request is valid. If bit 7 of @mode is 1, then this field is ignored.
  * @timer: BTM disassociation timer. Time period in TUs of the disassociation timer in the BTM request.
  * @sta_cnt: STA list count. If @sta_cnt set to 0, it mean steering request applies to all associated STAs in the BSS per policy setting. Otherwise, it means steering request applies to specific STAs specified by STA MAC address(es).
  * @sta_list: STA MAC address for which the steering request applies. Ignore when @sta_cnt is set to 0.
  */
struct GNU_PACKED lib_steer_request {
	unsigned char bssid[ETH_ALEN];
	unsigned char mode;
	unsigned char disassoc_imminent;
	unsigned char abridged;
	unsigned short window;
	unsigned short timer;
	unsigned char sta_cnt;
	unsigned char sta_list[0];
};

/**
  * @bssid: target BSSID. Indicates a target BSSID for steering. Wildcard BSSID is represented by FF:FF:FF:FF:FF:FF.
  * @op_class: target BSS operating class. If the target BSSID is set to "Wildcard BSSID", the value of this field is ignored by the receiver.
  * @channel: target BSS channel number for channel on which the target BSS is transmitting Beacon frames. If the target BSSID is set to "Wildcard BSSID", the value of this field is ignored by the receiver.
  */
struct GNU_PACKED lib_target_bssid_info {
	unsigned char bssid[ETH_ALEN];
	unsigned char op_class;
	unsigned char channel;
};

struct GNU_PACKED ch_sel_rsp_info {
	unsigned char radio_indentifier[ETH_ALEN];
	unsigned char rsp_code;
};


struct GNU_PACKED unassoc_sta_link_metrics_query_sub{
	unsigned char channel;
	unsigned char sta_num;
	unsigned char sta_mac[ETH_ALEN * 64];
};

struct GNU_PACKED unassoc_sta_link_metrics_query {
	unsigned char op_class;
	unsigned char ch_num;
	struct unassoc_sta_link_metrics_query_sub unassoc_link_query_sub[0];
};

struct GNU_PACKED device_info_sub {
	unsigned char mac[ETH_ALEN];
	unsigned short media_type;
};
struct GNU_PACKED device_info {
	unsigned char al_mac[ETH_ALEN];
	unsigned char inf_num;
	struct device_info_sub inf_info[0];
};

struct GNU_PACKED bridge_cap_sub {
	char br_name[IFNAMSIZ];
	unsigned char inf_num;
	unsigned char inf_mac[MAX_INF_NUM * ETH_ALEN];
};
struct GNU_PACKED bridge_cap {
	unsigned char br_num;
	struct bridge_cap_sub bridge_info[0];
};

struct GNU_PACKED supported_srv {
	unsigned char srv_num;
	unsigned char srv[0];
};

/**
  * @cnt: the count of info
  * @almac: indicates the al mac address of which device this config is applied to, note that FF:FF:FF:FF:FF:FF 
  * is the wild card mac which means that this field will not be took into account when selects the bss info for other MAP agent
  * @band: indicates which band this config is used for
  	0x01-2.4G  
  	0x02-5GL 
  	0x04-5GH
  	0x03-both 2.4G and 5GL
  	0x05-both 2.4G and 5GH
  	0x06-both 5GL and 5GH
  	0x07-both 2.4G, 5GL and 5GH
  * @ssid: ssid of this config
  * @encry_type: indicates the encryption mode of this bss config
  * @auth_mode: indicates the auth mode of this bss config
  * @key: indicates the key of this bss config
  * @wfa_vendor_extension: indicates the vendor extension ie included in WSC M2 message, 
  * BIT_BH_BSS(bit6) means backhaul bss, BIT_FH_BSS(bit5) means fronthaul bss, 
  * BIT_TEAR_DOWN(bit4) means this radio should be teared down
  */
struct GNU_PACKED bss_config_info {
	unsigned char cnt;
	struct GNU_PACKED config_info {
		unsigned char almac[ETH_ALEN];
		unsigned char band;
		char ssid[33];
		enum ENCRYPTION_TYPE encry_type;
		enum AUTH_MODE auth_mode;
		unsigned char key[65];
		unsigned char wfa_vendor_extension;
		unsigned char hidden_ssid;
	} info[0];
};


#ifdef MAP_R2

/*channel scan feature*/
struct GNU_PACKED scan_opcap
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
};
struct GNU_PACKED scan_capability_lib
{
	unsigned char identifier[ETH_ALEN];
	unsigned char boot_only;
	unsigned char scan_impact;
	unsigned int min_scan_interval;
	unsigned char op_class_num;
	struct scan_opcap opcap[0];
};

struct GNU_PACKED scan_request_lib
{
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class_num;
	struct scan_opcap opcap[MAX_OP_CLASS_NUM_20M];
};

#define BW_MAX 10
#define TS_MAX 30

struct GNU_PACKED scan_neighbor {
	unsigned char bssid[ETH_ALEN];
	unsigned char ssid_len;
	unsigned char ssid[32];
	unsigned char rssi;
	unsigned char bw_len;
	unsigned char bw[BW_MAX];
	unsigned char present;
	unsigned char utilization; //valid when bit 7 of present is set
	unsigned short sta_cnt; //valid when bit 6 of present is set
};

struct GNU_PACKED scan_result_lib
{
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class;
	unsigned char channel;
	unsigned char scan_status;
	unsigned char ts_len; //valid when scan_status set to 0
	unsigned char ts_str[TS_MAX]; //valid when scan_status set to 0
	unsigned char utilization; //valid when scan_status set to 0
	unsigned char noise; //valid when scan_status set to 0
	unsigned int agg_scan_dur; //valid when scan_status set to 0
	unsigned char scan_type; //valid when scan_status set to 0
	unsigned short neighbor_cnt; //valid when scan_status set to 0
	struct scan_neighbor neighbor[0]; //valid when scan_status set to 0
};


struct GNU_PACKED tunneled_msg_tlv
{
	unsigned int payload_len;
	unsigned char payload[0];
};



struct GNU_PACKED tunneled_message_lib
{
	unsigned char sta_mac[ETH_ALEN];
	unsigned char proto_type;
	unsigned char num_tunneled_tlv;
	struct tunneled_msg_tlv tunneled_msg_tlv[0];
};


struct GNU_PACKED assoc_status {	
	unsigned char bssid[ETH_ALEN];
	unsigned char status;
};

struct GNU_PACKED assoc_notification_tlv {
	unsigned char bssid_num;
	struct assoc_status status[0];	
};

struct GNU_PACKED assoc_notification_lib {
	unsigned char assoc_notification_tlv_num;
	struct assoc_notification_tlv notification_tlv[0];	
};


struct GNU_PACKED lib_target_bssid_info_R2 {
	unsigned char bssid[ETH_ALEN];
	unsigned char op_class;
	unsigned char channel;
	unsigned char reason_code; 
}; 

struct GNU_PACKED lib_steer_request_R2 {
	unsigned char bssid[ETH_ALEN];
	unsigned char mode;
	unsigned char disassoc_imminent;
	unsigned char abridged;
	unsigned short window;
	unsigned short timer;
	unsigned char sta_cnt;
	unsigned char sta_list[0]; 
};

struct GNU_PACKED cac_opcap
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[0]; //MAX of 5G Channels
};

struct GNU_PACKED cac_type
{
	unsigned char cac_mode;
	unsigned char cac_interval[3];
	unsigned char op_class_num;
	struct cac_opcap opcap[0];
};


struct GNU_PACKED cac_cap
{
	unsigned char identifier[ETH_ALEN];
	unsigned char cac_type_num;
	struct cac_type type[0];
};


struct GNU_PACKED cac_capability_lib
{
	unsigned char country_code[2];
	unsigned char radio_num;
	struct cac_cap cap[0];
}; 



struct GNU_PACKED cac_req {
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class_num;
	unsigned char ch_num;
	unsigned char cac_method;
	unsigned char cac_action;
};
struct GNU_PACKED cac_request_lib {
	unsigned char num_radio;
	struct cac_req req[0];
}; 


#endif
#define MAX_RADIO_NUM 3

struct GNU_PACKED cac_completion_report_opcap
{
	unsigned char op_class;		// This field shall be 0, if radar was not detected.
	unsigned char ch_num;		// This field shall be 0, if radar was not detected.
};

struct GNU_PACKED cac_completion_status_lib
{
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class;
	unsigned char channel;
	unsigned char cac_status;
	unsigned char op_class_num;	// This field shall be 0, if radar was not detected.
	struct cac_completion_report_opcap opcap[0];
};


struct GNU_PACKED cac_completion_report_lib
{
	unsigned char radio_num;
	struct cac_completion_status_lib cac_completion_status[MAX_RADIO_NUM];
};


#ifdef MAP_R2
#define MAX_CLASS_CHANNEL 68		//Total Operating class/channel pairs possible
#define MAX_CLASS_CHAN_NON_ALLOWED 52	//Non Allowed Operating class - Channel possible
struct GNU_PACKED cac_allowed_channel
{
	unsigned char op_class;    
	unsigned char ch_num;	    
	unsigned short cac_interval;	// This field shall be 0, for Non-DFS Channels
};
struct GNU_PACKED cac_non_allowed_channel
{
	unsigned char op_class;    
	unsigned char ch_num;	    
	unsigned short remain_interval; 
};
struct GNU_PACKED cac_ongoing_channel
{
	unsigned char op_class;    
	unsigned char ch_num;	    
	unsigned int remain_interval; 
};

struct GNU_PACKED cac_status_report_lib
{
	unsigned char allowed_channel_num;    	 
	unsigned char non_allowed_channel_num;    
	unsigned char ongoing_cac_channel_num; 
	struct cac_allowed_channel allowed_channel[MAX_CLASS_CHANNEL];         
	struct cac_non_allowed_channel non_allowed_channel[MAX_CLASS_CHAN_NON_ALLOWED];
	struct cac_ongoing_channel cac_ongoing_channel[0];
};


struct GNU_PACKED cac_term {
        unsigned char identifier[ETH_ALEN];
        unsigned char op_class_num;
        unsigned char ch_num;
};

struct GNU_PACKED cac_terminate_lib {
        unsigned char num_radio;
        struct cac_term term_tlv[0];
};

struct GNU_PACKED ap_extended_metrics_lib {
	unsigned char bssid[ETH_ALEN];
	unsigned int uc_tx;
	unsigned int uc_rx;
	unsigned int mc_tx;
	unsigned int mc_rx;
	unsigned int bc_tx;
	unsigned int bc_rx;
};


struct GNU_PACKED radio_metrics_lib {
	unsigned char identifier[ETH_ALEN];
	unsigned char noise;
	unsigned char transmit;
	unsigned char receive_self;
	unsigned char receive_other;
};


struct GNU_PACKED ch_util_lib {
	unsigned char ch_num;
	unsigned int edcca;
};

struct GNU_PACKED ap_ext_metrics_info_lib {
        unsigned char bssid[ETH_ALEN];
        u32 uc_tx;
        u32 uc_rx;
        u32 mc_tx;
        u32 mc_rx;
        u32 bc_tx;
        u32 bc_rx;
};

struct GNU_PACKED extended_metrics_info {
	unsigned char bssid[ETH_ALEN];
	unsigned int last_data_ul_rate;
	unsigned int last_data_dl_rate;
	unsigned int utilization_rx;
	unsigned int utilization_tx;
};


struct GNU_PACKED sta_extended_metrics_lib {
	unsigned char sta_mac[ETH_ALEN];
	unsigned char extended_metric_cnt;
	struct extended_metrics_info metric_info[0];
};

#endif // #ifdef MAP_R2
struct stats_tlv {
	unsigned char	mac[ETH_ALEN];
	unsigned int	bytes_sent;
	unsigned int	bytes_received;
	unsigned int 	packets_sent;
	unsigned int 	packets_received;
	unsigned int 	tx_packets_errors;
	unsigned int 	rx_packets_errors;
	unsigned int 	retransmission_count;
	unsigned int 	ul_rate;
	unsigned int 	dl_rate;
	unsigned int 	last_bytes_sent;
	unsigned int 	last_bytes_recieved;
	struct os_time 	last_traffic_stats_time;
};
#endif /*__DATA_DEF_H__*/

