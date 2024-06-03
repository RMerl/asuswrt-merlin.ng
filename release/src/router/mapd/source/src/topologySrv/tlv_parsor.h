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
 *  1905 tlv parsor
 *
 *  Abstract:
 *  1905 tlv parsor
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Kapil.Gupta 2018/05/02    First implementation of the 1905 tlv parsor
 * */

#ifndef tlv_parsor_H
#define tlv_parsor_H

#include "topologySrv.h"

#define IEEE802_11_band_2P4G 0x00
#define IEEE802_11_band_5GL  0x01
#define IEEE802_11_band_5GH  0x02

#define BAND_UNKNOWN  0x00
#define BAND_2G 0x01
#define BAND_5GL 0x02
#define BAND_5GH 0x03

#define BAND_INVALID_CAP 0x00
#define BAND_2G_CAP 0x01
#define BAND_5GL_CAP  0x02
#define BAND_5GH_CAP  0x04
#define BAND_5G_CAP  0x06

#define BSS_2G_CAP 0x01
#define BSS_5GL_CAP  0x02
#define BSS_5GH_CAP  0x04
#define BSS_5G_CAP  0x06

#define SERVICE_CONTROLLER 0x00
#define SERVICE_AGENT 0x01

/*Multi-AP tlv type*/
#define SUPPORTED_SERVICE_TLV_TYPE 0x80
#define SEARCHED_SERVICE_TLV_TYPE 0x81
#define AP_RADIO_IDENTIFIER_TYPE 0x82
#define AP_OPERATIONAL_BSS_TYPE 0x83
#define AP_ASSOCIATED_CLIENTS_TYPE 0x84
#define AP_RADIO_BASIC_CAPABILITY_TYPE 0x85
#define AP_HT_CAPABILITY_TYPE 0x86
#define AP_VHT_CAPABILITY_TYPE 0x87
#define AP_HE_CAPABILITY_TYPE 0x88
#define STEERING_POLICY_TYPE 0x89
#define METRIC_REPORTING_POLICY_TYPE 0x8A
#define CH_PREFERENCE_TYPE 0x8B
#define RADIO_OPERATION_RESTRICTION_TYPE 0x8C
#define TRANSMIT_POWER_LIMIT_TYPE 0x8D
#define CH_SELECTION_RESPONSE_TYPE 0x8E
#define OPERATING_CHANNEL_REPORT_TYPE 0x8F

#define CLIENT_INFO_TYPE 0x90
#define CLIENT_CAPABILITY_REPORT_TYPE 0x91
#define CLIENT_ASSOCIATION_EVENT_TYPE 0x92
#define AP_METRICS_QUERY_TYPE 0x93
#define AP_METRICS_TYPE 0x94
#define STA_MAC_ADDRESS_TYPE 0x95
#define ASSOC_STA_LINK_METRICS_TYPE 0x96
#define UNASSOC_STA_LINK_METRICS_QUERY_TYPE 0x97
#define UNASSOC_STA_LINK_METRICS_RSP_TYPE 0x98
#define BEACON_METRICS_QUERY_TYPE 0x99
#define BEACON_METRICS_RESPONSE_TYPE 0x9A
#define STEERING_REQUEST_TYPE 0x9B
#define STEERING_BTM_REPORT_TYPE 0x9C
#define CLI_ASSOC_CONTROL_REQUEST_TYPE 0x9D
#define BACKHAUL_STEERING_REQUEST_TYPE 0x9E
#define BACKHAUL_STEERING_RESPONSE_TYPE 0x9F
#define HIGH_LAYER_DATA_TYPE 0xA0
#define AP_CAPABILITY_TYPE 0xA1
#define ASSOC_STA_TRAFFIC_STATS_TYPE 0xA2
#define ERROR_CODE_TYPE 0xA3
#ifdef MAP_R2
#define CHANNEL_SCAN_REPORTING_POLICY_TYPE 0xA4
#define CHANNEL_SCAN_CAPABILITY_TYPE 0xA5
#define CHANNEL_SCAN_REQUEST_TYPE 0xA6
#define CHANNEL_SCAN_RESULT_TYPE 0xA7
#define TIMESTAMP_TYPE 0xA8
#ifdef DFS_CAC_R2
#define CAC_REQ_RESULT_TYPE 0xAD
#define CAC_TERMINATE_TYPE  0xAE
#define CAC_COMPLETION_REPORT_TYPE 0xAF
#define CAC_STATUS_REPORT_TYPE 0xB1
#endif
#endif
#define CAC_COMPLETION_REPORT_TYPE 0xAF
#ifdef MAP_R2
#define R2_AP_CAPABILITY_TYPE 0xB4
#define SOURCE_INFO_TYPE 0xC0
#define TUNNELED_MESSAGE_TYPE 0xC1
#define TUNNELED_TYPE 0xC2
#define STEERING_REQUEST_TYPE_R2 0xC3  
#define UNSUCCESSFUL_ASSOCIATION_POLICY_TYPE 0xC4
#define METRIC_COLLECTION_INTERVAL_TYPE 0xC5
#define CAC_CAPABILITIES_TYPE 0xB2
#define MULTI_AP_VERSION_TYPE								0xB3
#define RADIO_METRIC_TYPE 0xC6
#define AP_EXTENDED_METRIC_TYPE 0xC7
#define ASSOCIATED_STA_EXTENDED_LINK_METRIC_TYPE 0xC8
#define ASSOCIATION_STATUS_NOTIFICATION_TYPE 0xBF
#define DISASSOCIATION_REASON_CODE_TYPE 0xCA
#define BACKHAUL_STA_RADIA_CAPABILITY_TYPE 0xCB
#endif


/*Multi-AP tlv length*/
#define SUPPORTED_SERVICE_LENGTH 2
#define SUPPORTED_SERVICE2_LENGTH 3
#define SEARCHED_SERVICE_LENGTH 2
#define AP_RADIO_IDENTIFIER_LENGTH 6
#define AP_CAPABILITY_LENGTH 1
#define AP_HT_CAPABILITY_LENGTH 7
#define AP_VHT_CAPABILITY_LENGTH 12
#define CLIENT_ASSOCIATION_EVENT_LENGTH 13
#define CLIENT_INFO_LENGTH 12
#define TRANSMIT_POWER_LIMIT_LENGTH 7

#define END_OF_TLV_TYPE 0
#define AL_MAC_ADDR_TLV_TYPE 1
#define MAC_ADDR_TLV_TYPE 2
#define DEVICE_INFO_TLV_TYPE 3
#define BRIDGE_CAPABILITY_TLV_TYPE 4
#define NON_P1905_NEIGHBOR_DEV_TLV_TYPE 6
#define P1905_NEIGHBOR_DEV_TLV_TYPE 7
#define LINK_METRICS_QUERY_TLV_TYPE 8
#define TRANSMITTER_LINK_METRIC_TYPE 9
#define RECEIVER_LINK_METRIC_TYPE 10
#define VENDOR_SPECIFIC_TLV_TYPE 11
#define RESULT_CODE_TLV_TYPE 12
#define SEARCH_ROLE_TLV_TYPE 13
#define AUTO_CONFIG_FREQ_BAND_TLV_TYPE 14
#define SUPPORT_ROLE_TLV_TYPE 15
#define SUPPORT_FREQ_BAND_TLV_TYPE 16
#define WSC_TLV_TYPE 17
#define PUSH_BUTTON_EVENT_NOTIFICATION_TYPE 18
#define PUSH_BUTTON_JOIN_NOTIFICATION_TYPE 19
#define SUPPORTED_SERVICE_TLV_TYPE_CHECK 20
#define AP_RADIO_IDENTIFIER_TYPE_CHECK 21
#define AP_RADIO_BASIC_CAPABILITY_TYPE_CHECK 22
#define CLIENT_ASSOCIATION_EVENT_TYPE_CHECK 23
#define CLIENT_INFO_TYPE_CHECK 24
#define CH_PREFERENCE_TYPE_CHECK 25
#define TRANSMIT_POWER_LIMIT_TYPE_CHECK 26
#define STEERING_REQUEST_TYPE_CHECK 27
#define STEERING_POLICY_TYPE_CHECK 28
#define METRIC_REPORTING_POLICY_TYPE_CHECK 29
#define CLI_ASSOC_CONTROL_REQUEST_TYPE_CHECK 30
#define AP_METRICS_QUERY_TYPE_CHECK 31
#ifdef MAP_R2
#define UNSUCCESSFUL_ASSOCIATION_POLICY_TYPE_CHECK 2
#endif
#define RADIO_OPERATION_RESTRICTION_TYPE_CHECK 1

/*CMDU tlv length(fixed part)*/
#define END_OF_TLV_LENGTH 0
#define AL_MAC_ADDR_TLV_LENGTH 6
#define MAC_ADDR_TLV_LENGTH 6
#define LINK_METRIC_RESULT_CODE_LENGTH 1
#define PUSH_BUTTON_JOIN_NOTIFICATION_LENGTH 20
#define BACKHAUL_STEERING_RESPONSE_LENGTH 13

#define SEARCH_ROLE_LENGTH 1
#define AUTOCONFIG_FREQ_BAND_LENGTH 1
#define SUPPORTED_ROLE_LENGTH 1
#define SUPPORTED_FREQ_BAND_LENGTH 1

/*link metrics query target*/
#define QUERY_ALL_NEIGHBOR 0
#define QUERY_SPECIFIC_NEIGHBOR 1

/*link metrics query type*/
#define TX_METRICS_ONLY 0
#define RX_METRICS_ONLY 1
#define BOTH_TX_AND_RX_METRICS 2

/*for searched role tlv and support role tlv use*/
#define ROLE_REGISTRAR 0x00

/*for auto freq band tlv and supported freq band tlv use*/
#define IEEE802_11_band_2P4G 0x00
#define IEEE802_11_band_5G   0x01
#define IEEE802_11_band_60G  0x02

struct wfa_subelements_attr {
	unsigned char attribute;
	unsigned char attribute_length;
	unsigned char attribute_value[0];
};
#define DEFAULT_CONF_STATUS 0
#define WAPP_EVT_BH_READY 1
#define WAPP_EVT_COMPLETED 2
#define WAPP_EVT_NOT_COMPLETED 3
#define WAPP_EVT_TRIGGER_WPS 4
#define WAPP_EVT_FAILED_WPS 5

#define SWAP16(x) \
    ((u16) (\
               (((u16) (x) & (u16) 0x00ffU) << 8) | \
               (((u16) (x) & (u16) 0xff00U) >> 8)))

/*use flags below to check integrity of received message*/
const static unsigned int check_integrity[] = {
	(1 << AL_MAC_ADDR_TLV_TYPE) | (1 << MAC_ADDR_TLV_TYPE),	//topology discovery
	(1 << AL_MAC_ADDR_TLV_TYPE),	//topology notification
	0,			//topology query
	(1 << DEVICE_INFO_TLV_TYPE) | (1 << SUPPORTED_SERVICE_TLV_TYPE_CHECK),	//topology response
	(1 << VENDOR_SPECIFIC_TLV_TYPE),	//vendor specific
	(1 << LINK_METRICS_QUERY_TLV_TYPE),	//link metrics query
	(1 << TRANSMITTER_LINK_METRIC_TYPE) | (1 << RECEIVER_LINK_METRIC_TYPE),	//link mtrics response
	(1 << AL_MAC_ADDR_TLV_TYPE) | (1 << SEARCH_ROLE_TLV_TYPE) | (1 << AUTO_CONFIG_FREQ_BAND_TLV_TYPE),	//ap autoconfig search  
	(1 << SUPPORT_ROLE_TLV_TYPE) | (1 << SUPPORT_FREQ_BAND_TLV_TYPE) | (1 << SUPPORTED_SERVICE_TLV_TYPE_CHECK),	//ap autoconfig response
#if defined(SUPPORT_AP_ENROLLE)
	(1 << WSC_TLV_TYPE) | (1 << AP_RADIO_IDENTIFIER_TYPE_CHECK),	//ap autoconfig WSC
#elif defined(SUPPORT_AP_REGISTRAR)
	(1 << WSC_TLV_TYPE) | (1 << AP_RADIO_BASIC_CAPABILITY_TYPE_CHECK),	//ap autoconfig WSC
#endif
	(1 << AL_MAC_ADDR_TLV_TYPE) | (1 << SUPPORT_ROLE_TLV_TYPE) | (1 << SUPPORT_FREQ_BAND_TLV_TYPE),	//ap autoconfig renew
	(1 << AL_MAC_ADDR_TLV_TYPE) | (1 << PUSH_BUTTON_EVENT_NOTIFICATION_TYPE),	//PB event notification
	(1 << AL_MAC_ADDR_TLV_TYPE) | (1 << PUSH_BUTTON_JOIN_NOTIFICATION_TYPE),	//PB join notification
	(1 << CLIENT_INFO_TYPE_CHECK),	//client capability query
	(1 << CH_PREFERENCE_TYPE_CHECK) | (1 << TRANSMIT_POWER_LIMIT_TYPE_CHECK),	//channel selection request
};

int get_cmdu_tlv_length(unsigned char *buf);

int parse_device_info_type_tlv(struct own_1905_device *ctx, unsigned char *buf, struct _1905_map_device *dev);

int parse_bridge_capability_type_tlv(struct own_1905_device *ctx, unsigned char *buf, struct _1905_map_device *dev);

int parse_p1905_neighbor_device_type_tlv(struct own_1905_device *ctx, unsigned char *buf, struct _1905_map_device *dev);

int parse_non_p1905_neighbor_device_type_tlv(struct own_1905_device *ctx, unsigned char *buf,
					     struct _1905_map_device *dev);

int parse_supported_service_tlv(struct own_1905_device *ctx, unsigned char *buf, struct _1905_map_device *dev);

int parse_ap_operational_bss_type_tlv(struct own_1905_device *ctx, unsigned char *buf, struct _1905_map_device *dev);

int parse_ap_associated_clients_type_tlv(struct own_1905_device *ctx, unsigned char *buf, struct _1905_map_device *dev);

int parse_channel_preference_tlv(struct own_1905_device *ctx,unsigned char *buf, struct _1905_map_device *dev);

int parse_radio_operation_restriction_tlv(unsigned char *buf, struct _1905_map_device *dev);

int parse_tx_link_metrics_tlv(unsigned char *buf, struct _1905_map_device *dev);

int parse_rx_link_metrics_tlv(unsigned char *buf, struct _1905_map_device *dev);

int parse_unassociated_sta_link_metrics_query_tlv(unsigned char *buf, struct unlink_metrics_query
						  *unlink);

int parse_beacon_metrics_query_tlv(unsigned char *buf, struct beacon_metrics_query **beacon);

int parse_steering_policy_tlv(unsigned char *buf, struct own_1905_device *ctx);

int parse_metric_reporting_policy_tlv(unsigned char *buf, struct own_1905_device *ctx);

int parse_ap_metrics_query_tlv(unsigned char *buf, struct own_1905_device *ctx);

int parse_associated_sta_link_metrics_query_message(struct own_1905_device *ctx, unsigned char *buf);

int parse_backhaul_steering_request_message(struct own_1905_device *ctx, unsigned char *buf);

int parse_map_policy_config_request_message(struct own_1905_device *ctx, unsigned char *buf);

int parse_ap_metrics_tlv(struct own_1905_device *ctx, struct _1905_map_device *dev, unsigned char *buf);

int parse_link_metric_query_type_tlv(unsigned char *buf, unsigned char *target, unsigned char *type);

int append_link_metric_info(unsigned char *ap_tx_link, unsigned char *ap_rx_link, unsigned char *sta_tx_link,
			    unsigned char *sta_rx_link, struct _1905_map_device *dev,
			    struct map_neighbor_info *neighbor, int *is_ap);

int parse_ap_cap_tlv(unsigned char *buf, struct _1905_map_device *dev, struct ap_capability *cap);
int parse_basic_radio_cap_tlv(unsigned char *buf, struct _1905_map_device *dev, struct ap_radio_basic_cap *bcap);
int parse_ap_ht_cap_tlv(unsigned char *buf, struct ap_ht_capability *cap);
int parse_ap_vht_cap_tlv(unsigned char *buf, struct ap_vht_capability *cap);
int parse_ap_he_cap_tlv(unsigned char *buf, struct ap_he_capability *bcap);
int parse_topology_notification_evt(struct own_1905_device *ctx, unsigned char *buf, struct topo_notification *evt);
int parse_ap_metrics_response_message(struct own_1905_device *ctx, struct _1905_map_device *dev,
        unsigned char *buf);
int parse_assoc_sta_link_metrics_tlv(struct own_1905_device * ctx, struct _1905_map_device *dev, unsigned char *buf);
int parse_assoc_sta_traffic_stats_tlv(struct own_1905_device * ctx, struct _1905_map_device *dev, unsigned char *buf);
void clear_all_client_info(struct own_1905_device *ctx, struct _1905_map_device *dev);
void mapd_user_eth_client_leave(
	struct mapd_global *global,
	unsigned char *sta_mac, unsigned char *bssid);
void clear_invalid_client_info(struct own_1905_device *ctx, struct _1905_map_device *dev);
int topo_srv_update_neighbor_entry_peer(struct own_1905_device *ctx,
   struct map_neighbor_info *neighbor, unsigned char *local_mac, char skip);
int topo_srv_update_neighbor_entry(struct own_1905_device *ctx,
   struct map_neighbor_info *neighbor, unsigned char *local_mac, char skip);
int parse_backhaul_steering_rsp_message(struct own_1905_device *ctx,
	unsigned char *buf);
int parse_client_steering_btm_report_message(struct own_1905_device *ctx, struct _1905_map_device *dev,
        unsigned char *buf);
int parse_client_capability_report_message(struct own_1905_device *ctx,
		unsigned char *buf, unsigned char *temp_bssid, unsigned char *temp_sta,
		unsigned char *assoc_req, unsigned int *assoc_len);

#ifdef MAP_R2
int parse_unsuccessful_association_policy_tlv(unsigned char *buf, struct own_1905_device *ctx);
int parse_ap_radio_identifier_tlv(unsigned char *buf, struct own_1905_device *ctx, unsigned short band_idx);
int parse_channel_scan_report_message(struct mapd_global *global,
	unsigned char *buf, struct _1905_map_device *dev);
int parse_assoc_status_notification_message(struct own_1905_device *ctx, struct _1905_map_device *dev, unsigned char *buf);
int parse_client_disassciation_stats_message(struct own_1905_device *ctx, struct _1905_map_device *dev, unsigned char *buf);
int parse_tunneled_message(struct own_1905_device *ctx, unsigned char *buf, struct _1905_map_device *dev);
int parse_cac_completion_tlv(struct own_1905_device *ctx,
	unsigned char *buf, struct _1905_map_device *dev);
void ch_planning_handle_cac_response(
	struct own_1905_device *own_dev,
	struct _1905_map_device *dev,
	struct radio_info_db *radio);
int parse_ap_scan_cap_tlv(
	unsigned char *buf,
	struct _1905_map_device *dev,
	struct channel_scan_capab *ch_scan_cap);
int topo_srv_update_dev_ch_scan_cap(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct channel_scan_capab *ch_scan_cap);
int parse_ap_cac_cap_tlv(
	unsigned char *buf,
	struct _1905_map_device *dev);
int parse_metric_collection_intv_tlv(unsigned char *buf, struct _1905_map_device *dev);
int parse_r2_cap_tlv(unsigned char *buf, struct _1905_map_device *dev);
int parse_bh_sta_report(struct own_1905_device *ctx, struct _1905_map_device *dev, unsigned char *buf);
int parse_1905_ack_message(struct own_1905_device *ctx, struct _1905_map_device *dev, unsigned char *buf);
int parse_map_version_tlv(unsigned char *buf,
	unsigned char *almac, unsigned char *profile);
#endif
int parse_cac_completion_tlv(struct own_1905_device *ctx,
	unsigned char *buf, struct _1905_map_device *dev);
int parse_cli_assoc_control_request_message(struct own_1905_device *ctx,
	unsigned char *buf, u8 len);
int parse_cli_assoc_control_request_tlv(unsigned char *buf,
	struct own_1905_device *ctx, u8 *al_mac);
#endif
