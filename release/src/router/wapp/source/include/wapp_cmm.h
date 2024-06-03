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

#ifndef __WAPP_CMM_H__
#define __WAPP_CMM_H__

#include "rt_config.h"
#include "os.h"
#include "ieee80211_defs.h"
#include "ieee80211v_defs.h"
#include "ieee80211u_defs.h"
#include "interface.h"
#include "mbo.h"
#include "oce.h"
#include "hotspot.h"
#include "rtmpiapp.h"
#include "iappdefs.h"
#include "wdev.h"
#include "map.h"
#include "map_1905.h"
#ifdef DPP_SUPPORT
#include "dpp.h"
#endif /*DPP_SUPPORT*/

#include "../wapp_usr_intf/wapp_usr_intf.h"

#define MAX_NUM_OF_RADIO 6
#define MAX_NUM_OF_CARD 3

#define PROBE_TABLE_SIZE		256
#define PROBE_HASH_BIT 			6
#define PROBE_HASH_SIZE			(1 << PROBE_HASH_BIT)
#define PROBE_HASH_INDEX(Addr)	(MAC_ADDR_HASH(Addr) & (PROBE_HASH_SIZE - 1))

#define RED(_text)  "\033[1;31m"_text"\033[0m"
#define GRN(_text)  "\033[1;32m"_text"\033[0m"
#define YLW(_text)  "\033[1;33m"_text"\033[0m"
#define BLUE(_text) "\033[1;36m"_text"\033[0m"

#define WAPP_VERSION  	"v2.0.2"
#ifdef OPENWRT_SUPPORT
#define WAPP_CONF_PATH "/etc"
#else
#define WAPP_CONF_PATH "/etc"
#endif /* OPENWRT_SUPPORT */
#ifdef MAP_R2
#define DFS_CH_CLEAR_INDICATION 0xB
#endif

typedef enum {
	WAPP_SUCCESS = 0,
	WAPP_INVALID_ARG,
	WAPP_RESOURCE_ALLOC_FAIL,
	WAPP_NOT_INITIALIZED,
	WAPP_LOOKUP_ENTRY_NOT_FOUND,
	WAPP_UNEXP,
} WAPP_ERR_CODE;

enum {
	RADIO_OFF = 0,
	RADIO_ON
};

typedef enum {
	TIME_BASED = 0,
	DATA_VOLUME_BASED,
	TIME_AND_DATA_VOLUME_BASED,
	UNLIMITED,
} ADVICE_OF_CHARGE_TYPE;
#ifdef MAP_R2
enum ZEROWAIT_ACT_CODE{
	ZERO_WAIT_DFS_ENABLE = 0,/*0*/
	INIT_AVAL_CH_LIST_UPDATE,
	MONITOR_CH_ASSIGN,
	NOP_FORCE_SET,
	PRE_ASSIGN_NEXT_TARGET,
	SHOW_TARGET_INFO,
	QUERY_AVAL_CH_LIST = 20,
	QUERY_NOP_OF_CH_LIST,

};
enum {
	RDD_BAND0 = 0,
	RDD_BAND1,
};
#define RDD_DEDICATED_RX 2
#endif

#ifndef DPP_SUPPORT
struct GNU_PACKED set_config_bss_info{
	unsigned char mac[ETH_ALEN];
	char oper_class[4];
	char ssid[33];
	unsigned short authmode;
	unsigned short encryptype;
	char key[65];
	unsigned char wfa_vendor_extension;
	unsigned char hidden_ssid;
	/* local */
	unsigned char operating_chan;
	unsigned char is_used;
};
#endif

struct wapp_conf {
	struct dl_list list;
	int hotspot_onoff;
	char iface[IFNAMSIZ + 1];
	char confname[256];
	int access_network_type;
	u8 interworking;
	int internet;
	int venue_group;
	int is_venue_group;
	int venue_type;
	int is_venue_type;
	char hessid[6];
	int is_hessid;
	u8 anqp_query;
	u8 mih_support;
	char *advertisement_proto;
	int advertisement_proto_num;

	u8 anqp_req_type;
	char hs_peer_mac[6];

	size_t calc_anqp_rsp_len;
	u8 have_anqp_capability_list;
	u8 query_anqp_capability_list;
	struct dl_list anqp_capability_list;

	u8 venue_name_nums;
	u8 have_venue_name;
	u8 query_venue_name;
	struct dl_list venue_name_list;

	u8 venue_url_nums;
	u8 have_venue_url;
	u8 query_venue_url;
	struct dl_list venue_url_list;
	
	char *t_c_filename;
	u8 have_t_c_filename;

	char *t_c_server_url;
	u8 have_t_c_server_url;

	char *t_c_timestamp;
	u8 have_t_c_timestamp;

	u8 osu_providers_nai_nums;
	u8 query_osu_providers_nai_list;
	u8 have_osu_providers_nai_list;
	struct dl_list osu_providers_nai_duple_list;

	u8 have_emergency_call_number;
	u8 query_emergency_call_number;
	struct dl_list emergency_call_number_list;
	
	u8 network_auth_type_nums;
	u8 have_network_auth_type;
	u8 query_network_auth_type;
	struct dl_list network_auth_type_list;
	
	u8 have_roaming_consortium_list;
	u8 query_roaming_consortium_list;
	struct dl_list oi_duple_list;
	
	u8 have_ip_address_type;
	u8 query_ip_address_type;
	u8 ipv4_address_type;
	u8 ipv6_address_type;

	u8 nai_realm_data_nums;
	u8 have_nai_realm_list;
	u8 query_nai_realm_list;
	struct dl_list nai_realm_list;

	u8 advice_of_charge_data_nums;
	u8 have_advice_of_charge;
	u8 query_advice_of_charge;
	struct dl_list advice_of_charge_list;

	u8 plmn_nums;
	u8 have_3gpp_network_info;
	u8 query_3gpp_network_info;
	u8 gud; /* generic container user data */
	u8 udhl; /* user data header length */
	struct dl_list plmn_list;

	u8 have_operating_class;
	u8 query_operating_class;
	struct dl_list operating_class_list;
	
	u8 have_ap_geospatial_location;
	u8 query_ap_geospatial_location;
	char *ap_geospatial_location;
	u16 ap_geospatial_location_len;
	
	u8 have_ap_civic_location;
	u8 query_ap_civic_location;
	char *ap_civic_location;
	u16 ap_civic_location_len;
	
	u8 have_ap_location_public_uri;
	u8 query_ap_location_public_uri;
	char *ap_location_public_uri;
	u16 ap_location_public_uri_len;

	u8 have_ap_neighbor_report;
	u8 query_ap_neighbor_report;

	u8 query_ap_cdcp;

	u16 anqp_domain_id;

	u8 have_domain_name_list;
	u8 query_domain_name_list;
	struct dl_list domain_name_list;
	
	u8 have_emergency_alert_uri;
	u8 query_emergency_alert_uri;
	char *emergency_alert_uri;
	u16 emergency_alert_uri_len;
	
	u8 have_emergency_nai;
	u8 query_emergency_nai;
	char *emergency_nai;
	u16 emergency_nai_len;

	/* Following are additional hotspot 2.0 elements */
	size_t calc_hs_anqp_rsp_len;
	size_t calc_hs_icon_file_len;
	u8 have_hs_capability_list;
	u8 query_hs_capability_list;
	struct dl_list hs_capability_list;

	u8 op_friendly_name_nums;
	u8 have_operator_friendly_name;
	u8 query_operator_friendly_name;
	struct dl_list operator_friendly_duple_list;

	u8 wan_metrics_nums;
	u8 have_wan_metrics;
	u8 query_wan_metrics;
	struct wan_metrics metrics;

	u8 proto_port_nums;
	u8 have_connection_capability_list;
	u8 query_connection_capability_list;
	struct dl_list connection_capability_list;

	u8 have_nai_home_realm_query;
	u8 query_nai_home_realm;
	size_t calc_hs_nai_home_realm_anqp_rsp_len;
	struct dl_list nai_home_realm_name_query_list;
	
	u8 osu_providers_list_nums;
	u8 have_osu_providers_list;
	u8 query_osu_providers_list;
	struct dl_list osu_providers_list;
	
	u8 have_anonymous_nai;
	int anonymous_nai_len;
	u8 *anonymous_nai;
	u8 query_anonymous_nai;
	struct dl_list anonymous_nai_list;
	
	u8 query_icon_binary_file;
	struct dl_list icon_file_list;

	u8 query_icon_metadata;
	u8 have_icon_metadata;
	struct dl_list icon_metadata_list;

	u8 preferred_candi_list_included;
	u8 abridged;
	u8 disassociation_imminent;
	u8 bss_termination_included;
	u8 ess_disassociation_imminent;
	u16 disassociation_timer;
	u8 validity_interval;
	u8 have_bss_termination_duration;
	u64 bss_termination_tsf;
	u16 bss_termination_duration; /* unit: 1minute */
	u8 have_session_info_url;
	int session_info_url_len;
	char *session_info_url;
	u8 have_bss_transition_candi_list;
	struct dl_list bss_transition_candi_list;

	u8 have_time_zone;
	int time_zone_len;
	char *time_zone;

	u8 DGAF_disabled;
	u8 proxy_arp;
	u8 l2_filter;
	u8 icmpv4_deny;
	u8 p2p_cross_connect_permitted;
	u32 mmpdu_size;
	u8 external_anqp_server_test;
	u16 gas_cb_delay;
	u8 hs2_openmode_test;
	
	u8 have_iconfile_path;
	int iconfile_path_len;
	u8 *iconfile_path;
		
	u16 nontransmitted_len;
	u8 *nontransmitted_profile;
	u8 legacy_osu_exist;
	size_t legacy_osu_ssidlen;
	u8 *legacy_osu_ssid;
	char osu_iface[IFNAMSIZ + 1];
	
	u8	qosmap_enable;
	u16 dscp_range[8];
	u16 dscp_exception[21];
	u8  dscp_field;
	
	u8  qload_mode;
	u8  qload_cu;
	u16 qload_sta_cnt;
	u8	icon_tag;

	size_t	calc_mbo_anqp_rsp_len;
	
};


typedef union GNU_PACKED _TBTT_INFO_HEADER
{
	struct GNU_PACKED
	{
#ifdef RT_BIG_ENDIAN
		u16 TbttInfoLength:8;
		u16 TbttInfoCount:4;
		u16 Reserved:1;
		u16 FilteredNrAP:1; 
		u16 TbttInfoType:2;
#else
		u16 TbttInfoType:2;
		u16 FilteredNrAP:1;
		u16 Reserved:1;
		u16 TbttInfoCount:4;
		u16 TbttInfoLength:8;
#endif
	} field;
	u32 word;
} TBTT_INFO_HEADER, *PTBTT_INFO_HEADER;

#define MONITOR_PACKET_COUNT 5

/*air Monitor */

/*Monitor state*/
enum {
	MONITOR_IDLE = 0,
	MONITOR_ONGOING
};
struct sta_mnt_stat{
	struct dl_list list;
	u8 sta_mac[MAC_ADDR_LEN]; 	/*Mac Address of the Station to be monitor*/
	u8 Channel;					/*Channel on which device to be monitored used in case of multiple channel list*/
	u8 mnt_state;				/* 1 monitor Ongoing 0 Idle*/
	s32 avg_rssi;				/*Running Rssi Average*/
	u8 mnt_cnt;					/*No of packets recived while monitoring*/
	s32 mnt_reference;			/*referenced by multiple request*/
};
struct wapp_sta {
	u8 mac_addr[MAC_ADDR_LEN];
	u8 bssid[MAC_ADDR_LEN];
	u8 sta_status; /* WAPP_STA_STATE */
	u16 assoc_time;
	u16 valid_period;
	u8 assoc_req[ASSOC_REQ_LEN_MAX];
	u16 assoc_req_len;
	struct map_cli_cap cli_caps;
	u16 downlink;
	u16 uplink;
	char uplink_rssi;
	u8 bBSSMantSupport;
	u8 bLocalSteerDisallow;
	u8 bBTMSteerDisallow;
	u8 stat; /* Idle or active */
	u8 assco_stat;
	u8 steered_count;
	struct os_time last_update_time;

	/*traffic stats*/
	u32 bytes_sent;
	u32 bytes_received;
	u32 packets_sent;
	u32 packets_received;
	u32 tx_packets_errors;
	u32 rx_packets_errors;
	u32 tx_tp;
	u32 rx_tp;
	u32 retransmission_count;
	u16 link_availability;

	/* ht_cap */
	/* vht_cap */
#if 1 //MBO STA
	u8 is_mbo_sta;
	u8 cell_data_cap;
	u8 no_none_pref_ch;			/* to indicate that the sta has no none-prefer ch */
	u8 trans_reason;			/* bss transition reason for this sta */
	u8 disassoc_imnt;
	u32 akm;
	u32 cipher;
	struct dl_list non_pref_ch_list;
#endif /* MBO STA */
#if 1 /* OCE STA */
	u8 is_oce_sta;
	u8 oce_cap;
	s8 deltaassocrssi;
#endif /* OCE STA */
	/*beacon report result*/
	struct beacon_metrics_rsp *beacon_report;
#ifdef MAP_R2
	wdev_extended_sta_metrics ext_sta_metrics;
#endif
	u8 is_APCLI;
};


struct wapp_radio {
	u8	index;
	u8	radio_id;
	u8	op_ch;
	u8	card_id;
	u32	adpt_id;
	u32	chip_id;
	u8  onoff;
#ifdef MAP_SUPPORT
	u8	*radio_band;
	struct map_conf_state conf_state;
	struct map_metric_policy metric_policy;
	struct os_time last_scan_time;
	u8 min_scan_interval;
#endif /* MAP_SUPPORT */
#ifdef DPP_SUPPORT
	int wireless_mode;
	struct radio_bhsta_caps bhsta_cap;
	struct radio_adv_caps adv_cap;
	unsigned char bcap_tlv[1000];
	int bcap_len;
	u8 ongoing_dpp_cnt;
#endif /*DPP_SUPPORT*/
#ifdef WIFI_MD_COEX_SUPPORT
	u8 operatable;	/*0-not operatable, 1-operatable, it is decided by modem*/
#endif
};

struct wifi_protocol {
	struct btm_cfg btm;
};

#define PREQ_IE_LEN 128
struct probe_info {
	u8 valid;
	u8 mac_addr[MAC_ADDR_LEN];
	u8 channel;
	u8 rssi;
	u8 preq[PREQ_IE_LEN];
	struct os_time last_update_time;
	struct probe_info *next;
};

#ifdef DPP_SUPPORT
struct dpp_global;
#endif /*DPP_SUPPORT*/


struct wifi_app {
	/* driver interface operation */
	const struct wapp_drv_ops *drv_ops;

	/* event operation */
	const struct wifi_app_event_ops *event_ops;

	/* driver interface data */
	void *drv_data;

	/* wapp control interface */
	struct wapp_ctrl_iface *w_ctrl_iface;

	/* dev list (fronthaul bss or backhaul sta) */
	struct dl_list dev_list;

	/* configuration list */
	struct dl_list conf_list;

	/* collect all protocol structure here */
	struct hotspot 	*hs;
	struct mbo_cfg 	*mbo;
	struct oce_cfg 	*oce;
#ifdef MAP_SUPPORT
    struct map_info *map;
#endif
	struct wapp_usr_intf_cli_ctrl infcli_ctrl;

	RTMP_IAPP		*IAPP_Ctrl_Block;

	/* 802.11v[BTM] 802.11u[ANQP] 802.11k[Neighbor Rept] 802.11r[FT] */
	struct wifi_protocol protocol;

	int version;

	int opmode;

	u8 drv_mode;

	struct wapp_radio radio[MAX_NUM_OF_RADIO];

	char iface[64];

	struct wapp_conf *wapp_default_config;
	/* neighbor report common pool */
	DAEMON_NR_LIST daemon_nr_list;

	/* probe req table*/
	struct probe_info probe_entry[PROBE_TABLE_SIZE];
	struct probe_info *probe_hash[PROBE_HASH_SIZE];
#ifdef MAP_SUPPORT
	struct dl_list air_monitor_query_list;
	struct dl_list sta_mntr_list;
#endif
	/*wdev for which WSC triggered last*/
	void *wsc_trigger_wdev;
	/**/
	BOOLEAN wsc_configs_pending;
	BOOLEAN is_bs20_attached;
	/* Taking wdev backup used during scan_req */
	struct wapp_dev *scan_wdev;
	unsigned char csa_notif_received;
	unsigned char csa_new_channel;
	unsigned char link_change_notif_pending;
	struct apcli_association_info cli_assoc_info;
	unsigned char wps_on_controller_cli;
#ifdef MAP_SUPPORT
	BOOLEAN wsc_save_bh_profile;
#endif
#ifdef DPP_SUPPORT
	struct r2_ap_caps r2_ap_cap;
	struct akm_suite_caps akm_caps;
	// kapil DPP
	struct dpp_global *dpp;
	struct dl_list scan_results_list; /* struct bss_info_scan_result */
#endif /*DPP_SUPPORT*/
};

/* nr_list_lookup_result */
#define NR_ENTRY_NOT_FOUND DAEMON_NEIGHBOR_REPORT_MAX_NUM

struct wapp_ctrl_cmd {
	char *cmd;
	int (*cmd_proc)( struct wifi_app *wapp, const char *iface, u8 argc, char **argv);
	char *help;
};

/* if_nametoindex is defined in net/if.h but that conflicts with linux/if.h */
#define PROC_NET_DEV "/proc/net/dev"
extern unsigned int if_nametoindex (const char *__ifname);
extern char *if_indextoname (unsigned int __ifindex, char *__ifname);

int get_mac_addr_by_ifname(char *_iface, u8 *mac_addr);

struct wapp_radio* wapp_radio_lookup(
	struct wifi_app *wapp, 
	u32	adpt_id,
	u8	ra_id);

struct wapp_radio* wapp_radio_create(
	struct wifi_app *wapp,
	u32 adpt_id,
	u8 ra_id);

struct wapp_radio* wapp_radio_update_or_create(
	struct wifi_app *wapp,
	u32 adpt_id,
	u8 radio_id);

int wapp_radio_update_ch(
	struct wifi_app *wapp,
	struct wapp_radio *ra,
	u8 ch);

struct probe_info * wapp_probe_lookup(
	struct wifi_app *wapp,
	u8 *mac_addr);

struct probe_info * wapp_probe_create(
	struct wifi_app *wapp,
	u8 *mac_addr);

int wapp_event_get_neighbor_report_list(
	struct wifi_app *wapp,
	const char *iface,
    const char *neighbor_list_req,
    size_t neighbor_list_len);

int nr_peer_btm_query_action(
	struct wifi_app *wapp,
	const char *btm_query,
	size_t btm_query_len);

int nr_list_lookup_by_mac_addr(
	struct wifi_app *wapp,
	u8* mac_addr);

int nr_entry_add(
	struct wifi_app *wapp,
	wapp_nr_info* nr_info);

int wapp_cmm_init(
	struct wifi_app *wapp,
	int drv_mode,
	int opmode,
	int version,
	struct hotspot *hs,
	struct mbo_cfg *mbo,
	struct oce_cfg *oce,
	struct map_info *map,
	struct _RTMP_IAPP *IAPP_ctrl_block);

int wapp_socket_and_ctrl_inf_init(
	struct wifi_app *wapp,
	int drv_mode,
	int opmode);

int wapp_get_mac_addr_by_ifname(
	char *ifname,
	u8 *mac_addr);

int wapp_get_wireless_interfaces(
	struct wifi_app *wapp);

void wapp_periodic_exec(
	void *eloop_data,
	void *user_ctx);

int wapp_query_wdev(
	struct wifi_app *wapp,
	const char *iface);

int wapp_query_wdev_by_req_id(
	struct wifi_app *wapp, 
	const char *iface,
	u8 req_id);

int wapp_query_cli(struct wifi_app *wapp,
	const char *iface,
	const u8 *mac_addr);

int wapp_set_bss_start(struct wifi_app *wapp,
	const char *iface);

int wapp_set_bssload_thrd(struct wifi_app *wapp,
	const char *iface,
	char *high_thrd,
	char *low_thrd);

int wapp_set_bss_stop(struct wifi_app *wapp,
	const char *iface);

int wapp_set_tx_power_percentage(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	u8 pwr_prctg);

int wapp_set_steering_policy(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	wdev_steer_policy *policy);

int wapp_set_ap_config(
	struct wifi_app *wapp,
	struct wapp_dev *wdev);

int wapp_query_bssload(
	struct wifi_app *wapp,
	struct wapp_dev *wdev);

int wapp_query_he_cap(
	struct wifi_app *wapp,
	struct wapp_dev *wdev);

int wapp_query_sta_rssi(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	const u8 *mac_addr);

int wapp_query_apcli_rssi(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	const u8 *mac_addr);

int wapp_query_scan_result(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	char more_data);

int wapp_send_null_frames(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	unsigned char *sta_addr,
	unsigned int count);

void wapp_event_handle(
	struct wifi_app *wapp,
	struct wapp_event *event);

int wapp_ctrl_interface_cmd_handle(
	struct wifi_app *wapp,
	const char *iface,
	u8 argc,
	char **argv);

void wdev_query_rsp_handle(
	struct wifi_app *wapp,
	union _wapp_event_data *event_data);

void wdev_ht_cap_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wdev_ht_cap *ht_cap);

void wdev_vht_cap_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wdev_vht_cap *vht_cap);

void wdev_misc_cap_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wdev_misc_cap *misc_cap);

void wdev_cli_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_cli_list_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_cli_join_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_cli_leave_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_cli_probe_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_apcli_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_chn_list_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wdev_chn_info *chn_list);

void wdev_op_class_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wdev_op_class_info *op_class);


void wdev_bss_info_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wdev_bss_info *bss_info);

void wdev_ap_metric_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wdev_ap_metric *ap_metrics);
#ifdef MAP_R2
void wdev_radio_metric_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);
#endif
void wdev_ch_util_query_rsp_handle(
	struct wifi_app *wapp, 
	u32 ifindex, 
	wapp_event_data *event_data);

void wdev_ap_config_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

char * wapp_config_get_line(
	char *s,
	int size,
	FILE *stream,
	int *line,
	char **_pos);

int wapp_init_ap_config(
	struct wifi_app *wapp,
	const char *confname);

int wapp_init_all_config(
	struct wifi_app *wapp,
	const char *confname);

int wapp_deinit_config(
	struct wifi_app *wapp,
	struct wapp_conf *conf);

int wapp_deinit_all_config(
	struct wifi_app *wapp);

int wapp_set_interworking_enable(
	struct wifi_app *wapp,
	const char *iface,
	char *enable);

void wdev_bcn_report_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_bcn_report_complete_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_bssload_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_mnt_info_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_sta_rssi_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_cli_active_change_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_chn_change_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_csa_event_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_apcli_rssi_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_bss_stat_change_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_bssload_crossing_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_apcli_assoc_stat_change_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_apcli_assoc_stat_change_handle_vendor10(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);
void wdev_sta_cnnct_rej_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

void wdev_handle_scan_results(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data);

void wdev_handle_map_vend_ie_evt(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data);
void wdev_handle_wsc_config_write(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data);
void wdev_handle_a4_entry_missing_notif(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data);
void wdev_handle_radar(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);

int wapp_set_ie(struct wifi_app *wapp, const char *iface, 
    char *ie, size_t ie_len);

int wapp_driver_version(struct wifi_app *wapp, 
	const char *iface, char *ver, size_t *len);
int wapp_wps_pbc_trigger(struct wifi_app *wapp,
	const char *iface, char *ver, size_t *len);

int wapp_drv_support_version_check(struct wifi_app *wapp, const char *iface);
int wapp_set_scan_BH_ssids(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	struct scan_BH_ssids *scan_ssids);

u8 is_all_zero_mac(u8 *mac_addr);

int wapp_get_misc_cap(struct wifi_app *wapp, const char *iface, char *buf, 
				       		  size_t buf_len);

int wapp_get_ht_cap(struct wifi_app *wapp, const char *iface, char *buf, 
				       		  size_t buf_len);

int wapp_get_vht_cap(struct wifi_app *wapp, const char *iface, char *buf, 
				       		  size_t buf_len);

int wapp_get_chan_list(struct wifi_app *wapp, const char *iface, char *buf, 
				       		  size_t buf_len);

int wapp_get_op_class(struct wifi_app *wapp, const char *iface, char *buf, 
				       		  size_t buf_len);

int wapp_get_bss_info(struct wifi_app *wapp, const char *iface, char *buf, 
				       		  size_t buf_len);

int wapp_get_ap_metrics(struct wifi_app *wapp, const char *iface, char *buf, 
				       		  size_t buf_len);

int wapp_get_chip_id(struct wifi_app *wapp, const char *iface, char *buf, 
				       		  size_t buf_len);
int wapp_get_nop_channels(struct wifi_app *wapp, const char *iface, char *buf, 
				       		  size_t buf_len);

int wapp_get_he_cap(struct wifi_app *wapp, const char *iface, char *buf, 
			       		  size_t buf_len);
int wapp_get_valid_radio_count (struct wifi_app *wapp);

#ifdef HOSTAPD_MAP_SUPPORT
/*fetch existing bh_wsc_profile from hostapd conf*/
int	wapp_get_hapd_wifi_profile(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	struct wireless_setting *wifi_profile,
	unsigned int bh_idx,	
	BOOLEAN is_backhaul_profile);

/*set new bh_wsc_profile to hostapd conf*/
int wapp_set_hapd_wifi_profile(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	struct wireless_setting *wifi_profile,
	unsigned int bh_idx,	
	BOOLEAN is_backhaul_bss);

#endif

int wapp_create_arp_socket(struct wifi_app *wapp);
int wapp_set_AvoidScanDuringCAC(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	unsigned char enable);

void wdev_he_cap_query_rsp_handle(
	struct wifi_app *wapp,
	u32 ifindex,
	wdev_he_cap *he_cap);

int get_parameters(
	char *name,
	char *param,
	char *value,
	param_type type,
	size_t val_len);

int set_parameters(
	char *name,
	char *param,
	char *value,
	param_type type);

int nr_peer_btm_req_action(
		struct wifi_app *wapp,
		const char *btm_req,
		size_t btm_req_len,
		enum bss_trans_mgmt_status_code *status);

#ifdef MAP_R2

int wapp_event_offchannel_info(struct wifi_app *wapp, u8* buf);
void wdev_handle_ch_scan_result(struct wifi_app *wapp, u32 ifindex, OFFCHANNEL_INFO *ch_info);
void wdev_handle_ch_scan_complete(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data);
int map_receive_channel_scan_req(
						struct wifi_app *wapp, char *msg_buf,unsigned short msg_len);
						struct global_oper_class *wapp_get_global_op_class(u8 op_class);
void wdev_timer_ch_scan_complete(void *eloop_data, void *user_ctx);
void ch_scan_timeout(void *eloop_data, void *user_ctx);
void failed_assoc_timeout(void *eloop_data, void *user_ctx);
int wapp_event_wnm_notify_req(struct wifi_app *wapp,
						 const char *iface,
						 const u8 *peer_mac_addr,
						 const char *wnm_req,
						 size_t wnm_req_len);
void wdev_send_tunnel_assoc_req(struct wifi_app *wapp, u32 ifindex, u16 assoc_len, u8 *mac_addr, u8 isReassoc, u8 *assoc_buffer);
#ifdef DFS_CAC_R2
int map_receive_cac_req(
						struct wifi_app *wapp, char *msg_buf,unsigned short msg_len);
int map_receive_cac_terminate_req(
						struct wifi_app *wapp, char *msg_buf,unsigned short msg_len);
int wapp_set_channel_monitor_assign(struct wifi_app *wapp, const char *iface,
				          char *msg, size_t msg_len);
int wapp_get_channel_avail_ch_list(struct wifi_app *wapp, const char *iface,
				          char *msg, size_t msg_len);

#endif
void wdev_sta_disassoc_stats_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data);
int map_build_metric_reporting_info(
						struct wifi_app *wapp, unsigned char *addr, char *evt_buf);
int map_build_r2_mbo_sta_ch_pref(
	struct wifi_app *wapp, unsigned char *buf, int buf_len);

#endif
void wdev_handle_cac_period(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data);
int wapp_read_wts_map_config(struct wifi_app *wapp, char *name,
		struct set_config_bss_info bss_info[], unsigned char max_bss_num,
		unsigned char *config_bss_num);
#ifdef WIFI_MD_COEX_SUPPORT
void wdev_handle_unsafe_ch_event(struct wifi_app *wapp,
	wapp_event_data *event_data);
void wdev_handle_band_status_change(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data);
#endif

#endif /* __WAPP_CMM_H__ */
