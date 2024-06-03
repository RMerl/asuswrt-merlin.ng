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
 *  topology server
 *
 *  Abstract:
 *  topology server
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Kapil.Gupta 2018/05/02    First implementation of the 1905 topology server
 * */

#ifndef topo_srv_H
#define topo_srv_H

#include <sys/queue.h>
#include "interface.h"
#include "./../1905_local_lib/data_def.h"
#include "common.h"

#define MAX_MEDIA_INFO_LENGTH 10
#define BOOT_ONLY_SCAN 0x80
#define MAX_CH_NUM 13

//TODO move all these FUNC_ in sperate file for better reference
/*Interface Media Type(spec 6.4.7)*/
#define IEEE802_3_GROUP   0x0000
#define IEEE802_11_GROUP  0x0100
#define IEEE802_11AC_5G_GROUP   0x0105
#define IEEE802_11AX_GROUP   0x0108
#define IEEE1901_GROUP    0x0200
#define MOCA_GROUP        0x0300

#define ETH_ONBOARDING_STATE_START	0
#define ETH_ONBOARDING_STATE_DONE	1

#define WIFI_ONBOARDING_STATE_START	0
#define WIFI_ONBOARDING_STATE_DONE	1

#define MAP_FRONTHAUL_BSS	5
#define MAP_BACKHAUL_BSS	6
#define BSS_FH (1U << (MAP_FRONTHAUL_BSS))
#define BSS_BH (1U << (MAP_BACKHAUL_BSS))
#define RADAR_DETECTED 0x01
#define CAC_SUCCESSFUL 0x00
#define CAC_START 0x01
#define CAC_NOT_REQUIRED 0x02
#define IE_MEASUREMENT_REPORT           39
#ifdef CENT_STR
#define SUBTYPE_ASSOC_REQ				0
#define IE_RRM_EN_CAP					70
#define IE_WPA							221
#define IE_SUPP_CHANNELS				36
#define IE_EXT_CAPABILITY				127
#define IE_SUPP_REG_CLASS               59
#define MBO_OCE_OUI_0 					0x50
#define MBO_OCE_OUI_1 					0x6f
#define MBO_OCE_OUI_2 					0x9a
#define MBO_OCE_OUI_TYPE 				0x16
#define MBO_ATTR_STA_NOT_PREFER_CH_REP	2
#define MBO_ATTR_STA_CDC				3
#define MAX_LEN_OF_SUPPORTED_CHL		64
#endif
#define VERSION_MAPD 					"v2.0.2"
#define DEFAULT_ERROR_TLV_REASON		0
#define FAIL_CH_OP_CLASS				4
#define FAIL_TARGET_BSS					5
struct own_1905_device;
struct unlink_metrics_info;
struct metrics_policy;

struct mapd_user_onboarding_event
{
	unsigned int bh_type;
	unsigned int onboarding_start_stop;
};

enum interface_type {
	ieee_802_3_u = IEEE802_3_GROUP,
	ieee_802_3_ab,

	ieee_802_11_b = IEEE802_11_GROUP,
	ieee_802_11_g,
	ieee_802_11_a,
	ieee_802_11_n_2_4G,
	ieee_802_11_n_5G,
	ieee_802_11_ac,
	ieee_802_11_ad,
	ieee_802_11_af,
	ieee_802_11_ax,

	ieee_1901_wavelet = IEEE1901_GROUP,
	ieee_1901_FFT,

	moca_v1_1 = MOCA_GROUP,
};

enum _80211_iface_role {
	ieee_80211_ap,
	ieee_80211_sta = 4,
	ieee_80211_p2p_client = 8,
	ieee_80211_p2p_group_owner = 9,
	ieee_80211_ad_pcp = 10,
};

enum ring_action {
   no_ring,
   disconnect_wifi,
   do_nothing,
};

enum bh_status {
   no_cli_bh,
   one_cli_bh,
   multi_cli_bh,
   cli_eth_bh,
   wrong_status,
};

struct _80211_media_info {
	/* uplink bss is given through this */
	u8 network_membership[ETH_ALEN];
#if __BYTE_ORDER == __LITTLE_ENDIAN
	/* should be zero */
	u8 reserved:4;
	/* check enum _80211_iface_role */
	u8 role:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
	/* check enum _80211_iface_role */
	u8 role:4;
	/* should be zero */
	u8 reserved:4;
#endif
	u8 ap_channel_band;
	u8 ap_chan_central_freq_index1;
	u8 ap_chan_central_freq_index2;
};

struct client_assoc {
	unsigned char sta_addr[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	unsigned char is_joined;
};

struct topo_notification {
	unsigned char al_mac[ETH_ALEN];
	unsigned char assoc_cnt;
	struct client_assoc assoc[0];
};

struct iface_info {
	u8 iface_addr[ETH_ALEN];
	enum interface_type media_type;
	/* Media info is available for 802.11 type interface only */
	struct _80211_media_info media_info;
	/*to see whether it is map interface, which is used to see whether it is backhaul interface*/
	u8 is_map_if;
	void *radio;
	void *p1905_device;
	SLIST_ENTRY(iface_info) next_iface;
	unsigned char valid;
	unsigned char channel_freq_idx;
};

struct GNU_PACKED esp_db {
	unsigned char ac;
	unsigned char format;
	unsigned char ba_win_size;
	unsigned char e_air_time_fraction;
	unsigned char ppdu_dur_target;
	SLIST_ENTRY(esp_db) esp_entry;
};

struct stats_db {
	unsigned char mac[ETH_ALEN];
	unsigned int bytes_sent;
	unsigned int bytes_received;
	unsigned int packets_sent;
	unsigned int packets_received;
	unsigned int tx_packets_errors;
	unsigned int rx_packets_errors;
	unsigned int retransmission_count;
	SLIST_ENTRY(stats_db) stats_entry;
};

/* Table 6-18 IEEE 1905.1 transmitter link metrics */
struct backhaul_tx_link_info {
	enum interface_type iface_type;
	char is_80211_bridge;
	u32 pkt_err;
	u32 tx_packet;
	u16 mac_throughput;
	u16 link_availability;
	u16 phy_rate;
	u32 tx_tp;
};

/* Table 6-20 IEEE 1905.1 receiver link metrics */
struct backhaul_rx_link_info {
	enum interface_type iface_type;
	u32 pkt_err;
	u32 pkt_received;
	s8 rssi;
	u32 rx_tp;
};

struct backhaul_link_info {
	/**
	 * link_avail:
	 * 0x00: no link stats
	 * 0x01: Tx stats
	 * 0x02: Rx stats
	 * 0x03: Tx+Rx stats
	 */
	u8 link_avail;
	/* interface address using which neighbor is connected */
	u8 connected_iface_addr[ETH_ALEN];
	/* iface addrr of neighbor */
	u8 neighbor_iface_addr[ETH_ALEN];
	struct backhaul_tx_link_info tx;
	struct backhaul_rx_link_info rx;
	struct os_time last_update; /**< last update time in any link metrics resp or combined infrastructure metrics*/
	u8 is_valid;
	SLIST_ENTRY(backhaul_link_info) next_bh;
};

struct map_neighbor_info {
	u8 n_almac[ETH_ALEN];
	u8 ieee_802_1_bridge_exist;
	/* should be available only for 1905 device */
	//struct backhaul_link_info link_info;
	SLIST_HEAD(list_backhaul, backhaul_link_info) bh_head;
	/* Allocate this once we get report of this device */
	struct _1905_map_device *neighbor;
	/* next neighbor for the same device */
	SLIST_ENTRY(map_neighbor_info) next_neighbor;
	u8 is_valid;
	u8 insert_new_link;
};

struct bridge_tuple {
	u8 mac_addr[ETH_ALEN];
};

struct _1905_bridge {
	u8 interface_count;
	u8 *interface_mac_tuple;
	SLIST_ENTRY(_1905_bridge) next_bridge;
	//TODO
#if 0
	SLIST_HEAD(list_bride_tuple, bridge_tuple) first_tuple;
#endif
};

struct _1905_device {
	u8 al_mac_addr[ETH_ALEN];
	SLIST_HEAD(list_bridge, _1905_bridge) first_bridge;
	SLIST_HEAD(list_interface, iface_info) first_iface;
};

struct connected_clients {
	u8 _1905_iface_addr[ETH_ALEN];
	int link_info;
	u8 client_addr[ETH_ALEN];
	u8 bss_addr[ETH_ALEN];
	u8 entry_valid;
	u8 is_bh_link;
	u8 is_APCLI;
	SLIST_ENTRY(connected_clients) next_client;
};
#ifdef MAP_R2
struct sta_ext_info {
	u32 last_data_ul_rate;
	u32 last_data_dl_rate;
	u32 utilization_rx;
	u32 utilization_tx;
};
#endif
struct associated_clients {
	u8 client_addr[ETH_ALEN];
	struct bss_info_db *bss;
	u16 last_assoc_time;
	unsigned int time_delta;
	unsigned int erate_downlink;
	unsigned int erate_uplink;
	unsigned char rssi_uplink;
	unsigned char LastReportedUlRssi;
	unsigned char MonitorRcpi;
	Boolean is_bh_link;
#ifdef MAP_R2
	struct sta_ext_info sta_ext_info;
#endif
	u8 is_APCLI;
#ifdef CENT_STR
	struct stats_tlv stat_db;
#endif
	SLIST_ENTRY(associated_clients) next_client;
};

struct basic_cap_db {
	unsigned char op_class;
	unsigned char max_tx_pwr;
	unsigned char non_operch_num;
	unsigned char non_operch_list[MAX_CH_NUM];
	SLIST_ENTRY(basic_cap_db) basic_cap_entry;
};

struct ap_radio_basic_capability {
	unsigned char max_bss_num;
	unsigned char band;
	unsigned char op_class_num;
	SLIST_HEAD(list_head_bcap, basic_cap_db) bcap_head;
};

struct ht_caps {
	u8 valid;
	u8 reserved:1;
	u8 ht_40:1;
	u8 sgi_40:1;
	u8 sgi_20:1;
	u8 rx_stream:2;
	u8 tx_stream:2;
};

struct GNU_PACKED vht_caps {
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
	unsigned char valid;
};

struct he_caps {
	u8 he_160:1;
	u8 he_80plus80:1;
	u8 rx_spatial_streams:3;
	u8 tx_spatial_stream:3;
	u8 reserved:1;
	u8 dl_ofdma_supported:1;
	u8 ul_ofdma_supported:1;
	u8 dl_ofdma_plus_mu_mimo:1;
	u8 ul_ofdma_plus_mu_mimo:1;
	u8 ul_mi_mimo:1;
	u8 mu_beamformer:1;
	u8 su_beamformer:1;
	u8 valid:1;
};

/* Assumption: every bss in radio should have same capability */
struct radio_caps {
	u8 op_class;
	u8 max_tx_pwr;
	u8 non_oper_ch_num;
	u8 non_oper_ch_list[MAX_CH_NUM];
	struct ap_radio_basic_capability basic_caps;
	struct ht_caps ht_cap;
	struct vht_caps vht_cap;
	struct he_caps he_cap;
};

struct bss_info_db {
	unsigned char bssid[ETH_ALEN];
	unsigned char ssid_len;
	unsigned char ssid[32 + 1];
	unsigned int auth_mode;
	unsigned int enc_type;
	u8 key_len;
	u8 key[64 + 1];
	u8 map_vendor_extn;

	unsigned char ch_util;
	unsigned short assoc_sta_cnt;
	unsigned char esp_cnt;
	unsigned char hidden;
	SLIST_HEAD(list_esp, esp_db) esp_head;
	struct radio_info_db *radio;
	SLIST_ENTRY(bss_info_db) next_bss;
	unsigned char valid;
	// round robin:
	Boolean b_steer_triggered;  // variable to maintain if steering took place on this bss. Need to update on topology notification.
#ifdef MAP_R2
	u32 uc_tx;
	u32 uc_rx;
	u32 mc_tx;
	u32 mc_rx;
	u32 bc_tx;
	u32 bc_rx;
	u8	status;
#endif
#ifdef ACL_CTRL
	/* ACL CTRL list for this BSS */
	u8 acl_policy;
	struct dl_list acl_cli_list;
#endif
};

struct GNU_PACKED prefer_info_db {
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char perference;
	unsigned char reason;
	SLIST_ENTRY(prefer_info_db) prefer_info_entry;
};

struct GNU_PACKED restrict_db {
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM];
	unsigned char min_fre_sep[MAX_CH_NUM];
	SLIST_ENTRY(restrict_db) restrict_entry;
};

struct GNU_PACKED oper_restrict {
	char is_valid;
	unsigned char op_class_num;
	SLIST_HEAD(list_head_restrict, restrict_db) restrict_head;
};

struct GNU_PACKED radio_ch_prefer {
	char is_valid;
	signed char tx_power_limit;
	unsigned char op_class_num;
	SLIST_HEAD(list_head_prefer_info, prefer_info_db) prefer_info_head;
};
struct GNU_PACKED N_O_link_estimate_cb {
	unsigned char dev_almac[ETH_ALEN];
	unsigned char peer_mac[ETH_ALEN];
	unsigned int rcpi;
	unsigned int estimated_score;//unknown , to be filled in algo 
	unsigned int estimated_rate;
	unsigned int estimated_hop_count;
	SLIST_ENTRY(N_O_link_estimate_cb) link_estimate_cb_entry;
};

//Network opt structure for each radio of 1905 device in the topology
struct network_opt {
	unsigned char rate_deviate_count;
	unsigned int  last_uplink_rate;
	struct N_O_link_estimate_cb *max_score_link;
};
struct cac_completion_status_db
{
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class;
	unsigned char channel;
	unsigned char cac_status;
	unsigned char op_class_num;    // This field shall be 0, if radar was not detected.
	SLIST_HEAD(list_cac_completion_opcap, cac_completion_opcap_db) cac_completion_opcap_head;
};
struct cac_completion_opcap_db
{
	unsigned char op_class;    // This field shall be 0, if radar was not detected.
	unsigned char ch_num;       // This field shall be 0, if radar was not detected.
	SLIST_ENTRY(cac_completion_opcap_db) opcap_db_next;
};
#ifdef MAP_R2
struct non_pref_ch {
	u8 ch;
	u8 pref;
	u8 reason_code;
};

struct non_pref_ch_lib {
        u8 ch;
        u8 pref;
        u8 reason_code;
};

struct GNU_PACKED cu_distribution_tlv {
	u8 ch_num;
	u32 edcca_airtime;
};
struct GNU_PACKED nb_info {
	u8 bssid[ETH_ALEN];
	u8 ssid_len;
	u8 ssid[MAX_LEN_OF_SSID];
	u8 RCPI;
	u8 ch_bw_len;
	u8 ch_bw[MAX_CH_BW_LEN];
	u8 cu_stacnt_present; //bit7 : CU, bit-6 Stacnt
	u8 cu;
	u16 sta_cnt;
	SLIST_ENTRY(nb_info) next_neighbor_info;
};

struct GNU_PACKED scan_result_tlv {
	u8 radio_id[ETH_ALEN];
	u8 oper_class;
	u8 channel;
	u8 scan_status;
	u8 timestamp_len;
	u8 timestamp[TS_MAX];
	u8 utilization;//Utilization is coming normalized , max value is 255
	u8 noise;
	u32 agg_scan_duration;
	u8 scan_type;
	u16 neighbor_num;
	SLIST_HEAD(neighbor_info_list, nb_info) first_neighbor_info;
	SLIST_ENTRY(scan_result_tlv) next_scan_result;
	/*score data per agent per radio*/
	s32 ch_score;
	s16 snr;
	struct cu_distribution_tlv cu_distribution;
};

struct cac_opcap_db
{
	unsigned char op_class;
	unsigned char ch_num;
	unsigned char ch_list[MAX_CH_NUM]; //MAX of 5G Channels
	SLIST_ENTRY(cac_opcap_db) cac_opcap_entry;
};

struct cac_cap_db
{
	unsigned char cac_mode;
	unsigned char cac_interval[3];
	unsigned char op_class_num;
	SLIST_ENTRY(cac_cap_db) cac_cap_entry;
	SLIST_HEAD(list_head_cac_opcap, cac_opcap_db) cac_opcap_head;
};

struct cac_capability_db
{
	unsigned char identifier[ETH_ALEN];
	unsigned char cac_type_num;
	SLIST_HEAD(list_head_cac_cap, cac_cap_db) cac_capab_head;
};

struct  radio_metrics_info_db {
	u8 ra_id[ETH_ALEN];
	u8 cu_noise;
	u8 cu_tx;
	u8 cu_rx;
	u8 cu_other;
};
struct radio_scan_capab_db {
	unsigned char radio_id[ETH_ALEN];
	unsigned char boot_scan_only;
	unsigned char scan_impact;
	unsigned int min_scan_interval;
	unsigned char oper_class_num;
	struct channel_body ch_body[OP_CLASS_PER_RADIO];
};

#endif

/* AP capabilty report msg should be able to fatch everything, restriction: only for fronthaul AP */
struct radio_info_db {
	unsigned char identifier[ETH_ALEN];
	unsigned char band;
	u8 operating_class;
	u8 channel[MAX_CHANNEL_BLOCKS];
	u8 prev_channel;
	s8 power;
	uint8_t wireless_mode;
	struct radio_caps radio_capability;
	struct radio_ch_prefer chan_preferance;
	struct oper_restrict chan_restrict;
	unsigned char uplink_bh_present;
	struct operating_ch_cb *operating_channel;
	SLIST_ENTRY(radio_info_db) next_radio;
	SLIST_ENTRY(radio_info_db) next_co_ch_radio;
	struct _1905_map_device *parent_1905;
	/* Valid only for our own 1905 device */
	Boolean is_configured;
	unsigned int uplink_rate;
	struct network_opt network_opt_1905dev_radio;
	SLIST_HEAD(list_link_estimate_cb, N_O_link_estimate_cb) link_estimate_cb_head;
	char config_status;
	struct cac_completion_status_db cac_comp_status;
	u8 cac_channel;
	u8 cac_enable;
	u8 cac_timer;
	u8 send_cac_start_event;
	u8 bh_priority;
#ifdef MAP_R2
	SLIST_HEAD(scan_result_list, scan_result_tlv) first_scan_result;
	struct radio_scan_capab_db radio_scan_params;
	struct cac_capability_db cac_cap;
	struct radio_metrics_info_db radio_metrics;
#ifdef SUPPORT_MULTI_AP
	struct ch_plan_R2_1905dev dev_ch_plan_info;
	struct cu_distribution_tlv cu_distribution;
#endif
#endif
#ifdef CENT_STR
	u32 cent_str_avg_cu_monitor;
	u8 cent_str_count_cu_util;
#endif

};

enum map_vendor {
	VENDOR_QUALCOMM = 0,
	VENDOR_BROADCOM,
	VENDOR_MEDIATEK,
	VENDOR_MARVELL,
	VENDOR_INTEL,
	VENDOR_UNKNOWN = 0xff
};
struct est_dev_info {
	u8 al_mac_addr[ETH_ALEN];
	u32 max_relative_score;
};
#ifdef SUPPORT_MULTI_AP
struct network_opt_1905dev {
	NETOPT_STATE network_opt_device_state;
	unsigned char data_col_retry_count;
	struct os_time data_collection_start_ts;
	struct _1905_map_device *est_upstream_device; /**< pointer to upstream device */
	unsigned int est_max_score;
	unsigned int est_max_uplink_rate;
	unsigned int est_hop_count;
	struct radio_info_db *max_score_radio;
	struct os_time bh_steer_start_ts;
};
#endif
/**
* @brief 1905 map device info
*/
struct _1905_map_device {
	struct _1905_device _1905_info; /**< 1905 device info */
	/**
	 * bit map, could be root and controller both 
	 * agent = 0x00
	 * controller = 0x01
	 * root = 0x02
	 */
	int device_role; /**< device role */
	enum map_vendor vendor; /**< device vendor */
	int supported_services; /**< supported service set */
	int root_distance; /**< distance from root */
	struct os_time last_seen; /**< last seen in any topo resp */
	struct os_time first_seen; /**< first seen in any topo resp */
	char in_network; /**< if still part of the network */
	struct ap_capability ap_cap; /**< AP capabilities */
	SLIST_HEAD(list_radio, radio_info_db) first_radio; /**< list of radios attached to it */
	SLIST_HEAD(list_bss, bss_info_db) first_bss; /**< list of operation bss */
	/**
	 * can be derived using neighbout info ,
	 * either connected through wireless or wired,
	 * wireless will be in assoc client as well, if not then backhaul?
	 * assumption, only one device per ethernet interface
	 */
	struct _1905_map_device *upstream_device; /**< pointer to upstream device */

	struct bss_info_db *p_current_bss_rr;/**< current bss of 1905 device which has steer opportunity */

	long distance_updated; /**< cookie to check whether distance is updated */
	SLIST_HEAD(list_wlan_clients, connected_clients) wlan_clients; /**< list of wlan clients */
	SLIST_HEAD(list_associated_clients, associated_clients) assoc_clients; /**< list of associated clients from MAP Associated Clients TLV*/
	/* all the 1905 neighbors should be considered as child, including backhaul */
	SLIST_HEAD(list_neighbor, map_neighbor_info) neighbors_entry;/**< neighbor entry */
	SLIST_ENTRY(_1905_map_device) next_1905_device;/**< pointer to next 1905 device */
	SLIST_ENTRY(_1905_map_device) map_1905_dev_rfs_entry; /**< pointer to dev rfs device */
	SLIST_ENTRY(_1905_map_device) map_1905_dev_assoc_control_entry; /**< pointer to assoc control entry list*/
	unsigned char channel_planning_completed;
	unsigned char radio_mapping_completed;
	unsigned char ch_preference_available;
	struct _1905_map_device *secondary_root_dev;
	unsigned char is_valid;
	struct off_ch_scan_cb_s off_ch_scan_cb;
	struct off_ch_scan_report_event *off_ch_scan_report;
	struct net_opt_scan_report_event *net_opt_scan_report;
	u8 visited;
#ifdef SUPPORT_MULTI_AP
	struct network_opt_1905dev network_opt_per1905;
#endif
#ifdef MAP_R2
	u32 metric_rep_interval;
	u8	de_done;
	u8 ch_sel_req_given;
	u8 map_version;
	u8 byte_cnt_unit;
#endif
#ifdef ACL_CTRL
	u8 is_acl_sync_done;
#endif
};

struct channel_selection_chan_list {
	unsigned char channel;
	unsigned char reason_code:4;
	unsigned char preference:4;
};

struct ch_pref_sub {
	unsigned char op_class;
	unsigned char ch_num;
	struct channel_selection_chan_list chan_list[MAX_CH_NUM];
};

struct ch_pref_tlv {
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class_cnt;
	struct ch_pref_sub opclass[64];
};

#ifdef AUTOROLE_NEGO
struct GNU_PACKED dev_role_negotiate {
	unsigned char other_dev_role; 
	unsigned char other_dev_almac[ETH_ALEN];
};
#endif //AUTOROLE_NEGO
struct mbh_info {
    struct dl_list mbh_info_entry;
    u8 hop_cnt;
    u8 n_type;
    struct map_neighbor_info *neighbor;
};
#ifdef CENT_STR
typedef struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
	u16 Order:1;		/* Strict order expected */
	u16 Wep:1;		/* Wep data */
	u16 MoreData:1;	/* More data bit */
	u16 PwrMgmt:1;	/* Power management bit */
	u16 Retry:1;		/* Retry status bit */
	u16 MoreFrag:1;	/* More fragment bit */
	u16 FrDs:1;		/* From DS indication */
	u16 ToDs:1;		/* To DS indication */
	u16 SubType:4;	/* MSDU subtype */
	u16 Type:2;		/* MSDU type */
	u16 Ver:2;		/* Protocol version */
#else
	u16 Ver:2;		/* Protocol version */
	u16 Type:2;		/* MSDU type, refer to FC_TYPE_XX */
	u16 SubType:4;	/* MSDU subtype, refer to  SUBTYPE_XXX */
	u16 ToDs:1;		/* To DS indication */
	u16 FrDs:1;		/* From DS indication */
	u16 MoreFrag:1;	/* More fragment bit */
	u16 Retry:1;		/* Retry status bit */
	u16 PwrMgmt:1;	/* Power management bit */
	u16 MoreData:1;	/* More data bit */
	u16 Wep:1;		/* Wep data */
	u16 Order:1;		/* Strict order expected */
#endif	/* !RT_BIG_ENDIAN */
} FRAME_CONTROL, *PFRAME_CONTROL;

typedef struct GNU_PACKED _HEADER_802_11 {
	FRAME_CONTROL   FC;
	u16         Duration;
	u8           Addr1[6];
	u8           Addr2[6];
	u8		Addr3[6];
#ifdef RT_BIG_ENDIAN
	u16		Sequence:12;
	u16		Frag:4;
#else
	u16		Frag:4;
	u16		Sequence:12;
#endif /* !RT_BIG_ENDIAN */
	u8		Octet[0];
} HEADER_802_11, *PHEADER_802_11;

typedef struct GNU_PACKED _FRAME_802_11 {
	HEADER_802_11   Hdr;
	u8            Octet[1];
} FRAME_802_11, *PFRAME_802_11;
typedef struct GNU_PACKED _EID_STRUCT {
	u8   Eid;
	u8   Len;
	u8   Octet[1];
} EID_STRUCT, *PEID_STRUCT, BEACON_EID_STRUCT, *PBEACON_EID_STRUCT;
typedef union GNU_PACKED __RRM_EN_CAP_IE {
	struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
		u64 Reserved:28;
		u64 CIVICMeasureCap:1;
		u64 FTMRangeReportCapability:1;
		u64 AntennaInfoCap:1;
		u64 BssAvaiableAcmCap:1;
		u64 BssAvgAccessDelayCap:1;
		u64 RSNIMeasureCap:1;
		u64 RCPIMeasureCap:1;
		u64 NeighReportTSFOffsetCap:1;
		u64 MeasurePilotTxInfoCap:1;
		u64 MeasurePilotCap:3;
		u64 NotOperatingChMaxMeasureDuration:3;
		/*UINT64 RRMMibCap:1; */
		u64 OperatingChMaxMeasureDuration:3;
		u64 RRMMibCap:1;
		u64 APChannelReportCap:1;
		u64 TriggeredTransmitStreamCap:1;
		u64 TransmitStreamCap:1;
		u64 LCIAzimuthCap:1;
		u64 LCIMeasureCap:1;
		u64 StatisticMeasureCap:1;
		u64 NoiseHistogramMeasureCap:1;
		u64 ChannelLoadMeasureCap:1;
		u64 FrameMeasureCap:1;
		u64 BeaconMeasureReportCndCap:1;
		u64 BeaconTabMeasureCap:1;
		u64 BeaconActiveMeasureCap:1;
		u64 BeaconPassiveMeasureCap:1;
		u64 RepeatMeasureCap:1;
		u64 ParallelMeasureCap:1;
		u64 NeighborRepCap:1;
		u64 LinkMeasureCap:1;
#else
		u64 LinkMeasureCap:1;
		u64 NeighborRepCap:1;
		u64 ParallelMeasureCap:1;
		u64 RepeatMeasureCap:1;
		u64 BeaconPassiveMeasureCap:1;
		u64 BeaconActiveMeasureCap:1;
		u64 BeaconTabMeasureCap:1;
		u64 BeaconMeasureReportCndCap:1;
		u64 FrameMeasureCap:1;
		u64 ChannelLoadMeasureCap:1;
		u64 NoiseHistogramMeasureCap:1;
		u64 StatisticMeasureCap:1;
		u64 LCIMeasureCap:1;
		u64 LCIAzimuthCap:1;
		u64 TransmitStreamCap:1;
		u64 TriggeredTransmitStreamCap:1;
		u64 APChannelReportCap:1;
		u64 RRMMibCap:1;
		u64 OperatingChMaxMeasureDuration:3;
		u64 NotOperatingChMaxMeasureDuration:3;
		u64 MeasurePilotCap:3;
		u64 MeasurePilotTxInfoCap:1;
		u64 NeighReportTSFOffsetCap:1;
		u64 RCPIMeasureCap:1;
		u64 RSNIMeasureCap:1;
		u64 BssAvgAccessDelayCap:1;
		u64 BssAvaiableAcmCap:1;
		u64 AntennaInfoCap:1;
		u64 FTMRangeReportCapability:1;
		u64 CIVICMeasureCap:1;
		u64 Reserved:28;
#endif
	} field;
	u64 word;
} RRM_EN_CAP_IE, *PRRM_EN_CAP_IE;
typedef struct GNU_PACKED _EXT_CAP_INFO_ELEMENT {
#ifdef RT_BIG_ENDIAN
	u32	interworking:1;
	u32	TDLSChSwitchSupport:1; /* bit30: TDLS Channel Switching */
	u32	TDLSPeerPSMSupport:1; /* bit29: TDLS Peer PSM Support */
	u32	UAPSDBufSTASupport:1; /* bit28: Peer U-APSD Buffer STA Support */
	u32	utc_tsf_offset:1;
	u32	DMSSupport:1;
	u32	ssid_list:1;
	u32	channel_usage:1;
	u32	timing_measurement:1;
	u32	mbssid:1;
	u32	ac_sta_cnt:1;
	u32	qos_traffic_cap:1;
	u32	BssTransitionManmt:1;
	u32	tim_bcast:1;
	u32	WNMSleepSupport:1;/*bit 17*/
	u32	TFSSupport:1;/*bit 16*/
	u32	geospatial_location:1;
	u32	civic_location:1;
	u32	collocated_interference_report:1;
	u32	proxy_arp:1;
	u32	FMSSupport:1;/*bit 11*/
	u32	location_tracking:1;
	u32	mcast_diagnostics:1;
	u32	diagnostics:1;
	u32	event:1;
	u32	s_psmp_support:1;
	u32	rsv5:1;
	u32	psmp_cap:1;
	u32	rsv3:1;
	u32	ExtendChannelSwitch:1;
	u32	rsv1:1;
	u32	BssCoexistMgmtSupport:1;
#else
	u32	BssCoexistMgmtSupport:1;
	u32	rsv1:1;
	u32	ExtendChannelSwitch:1;
	u32	rsv3:1;
	u32	psmp_cap:1;
	u32	rsv5:1;
	u32	s_psmp_support:1;
	u32	event:1;
	u32	diagnostics:1;
	u32	mcast_diagnostics:1;
	u32	location_tracking:1;
	u32	FMSSupport:1;/*bit 11*/
	u32	proxy_arp:1;
	u32	collocated_interference_report:1;
	u32	civic_location:1;
	u32	geospatial_location:1;
	u32	TFSSupport:1;/*bit 16*/
	u32	WNMSleepSupport:1;/*bit 17*/
	u32	tim_bcast:1;
	u32	BssTransitionManmt:1;
	u32	qos_traffic_cap:1;
	u32	ac_sta_cnt:1;
	u32	mbssid:1;
	u32	timing_measurement:1;
	u32	channel_usage:1;
	u32	ssid_list:1;
	u32	DMSSupport:1;
	u32	utc_tsf_offset:1;
	u32	UAPSDBufSTASupport:1; /* bit28: Peer U-APSD Buffer STA Support */
	u32	TDLSPeerPSMSupport:1; /* bit29: TDLS Peer PSM Support */
	u32	TDLSChSwitchSupport:1; /* bit30: TDLS Channel Switching */
	u32	interworking:1;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
	u32 rsv63:1;
	u32 operating_mode_notification:1;
	u32 tdls_wider_bw:1;
	u32 rsv49:12;
	u32 utf8_ssid:1;
	u32 rsv47:1;
	u32 wnm_notification:1;
	u32 uapsd_coex:1;
	u32 id_location:1;
	u32 service_interval_granularity:3;
	u32 reject_unadmitted_frame:1;
	u32 TDLSChSwitchProhibited:1; /* bit39: TDLS Channel Switching Prohibited */
	u32 TDLSProhibited:1; /* bit38: TDLS Prohibited */
	u32 TDLSSupport:1; /* bit37: TDLS Support */
	u32 msgcf_cap:1;
	u32 rsv35:1;
	u32 sspn_inf:1;
	u32 ebr:1;
	u32 qosmap:1;
#else
	u32 qosmap:1;
	u32 ebr:1;
	u32 sspn_inf:1;
	u32 rsv35:1;
	u32 msgcf_cap:1;
	u32 TDLSSupport:1; /* bit37: TDLS Support */
	u32 TDLSProhibited:1; /* bit38: TDLS Prohibited */
	u32 TDLSChSwitchProhibited:1; /* bit39: TDLS Channel Switching Prohibited */
	u32 reject_unadmitted_frame:1;
	u32 service_interval_granularity:3;
	u32 id_location:1;
	u32 uapsd_coex:1;
	u32 wnm_notification:1;
	u32 rsv47:1;
	u32 utf8_ssid:1;
	u32 rsv49:12;
	u32 tdls_wider_bw:1;
	u32 operating_mode_notification:1;
	u32 rsv63:1;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
	u8 ftm_init:1;	/* bit71: FTM Initiator in 802.11mc D4.0*/
	u8 ftm_resp:1;	/* bit70: FTM responder */
	u8 rsv69:1;
	u8 rsv68:1;
	u8 rsv67:1;
	u8 rsv66:1;
	u8 rsv65:1;
	u8 rsv64:1;
#else
	u8 rsv64:1;
	u8 rsv65:1;
	u8 rsv66:1;
	u8 rsv67:1;
	u8 rsv68:1;
	u8 rsv69:1;
	u8 ftm_resp:1;	/* bit70: FTM responder */
	u8 ftm_init:1;	/* bit71: FTM Initiator in 802.11mc D4.0*/
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
	u8 rsv_79:1;
	u8 twt_responder_support:1;
	u8 twt_requester_support:1;
	u8 rsv76:1;
	u8 rsv75:1;
	u8 rsv74:1;
	u8 rsv73:1;
	u8 FILSCap:1;
#else
	u8 FILSCap:1;
	u8 rsv73:1;
	u8 rsv74:1;
	u8 rsv75:1;
	u8 rsv76:1;
	u8 twt_requester_support:1;
	u8 twt_responder_support:1;
	u8 rsv_79:1;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
	u8 rsv87:1;
	u8 rsv86:1;
	u8 rsv85:1;
	u8 rsv84:1;
	u8 rsv83:1;
	u8 sae_pwd_id_used_exclusively:1;
	u8 sae_pwd_id_in_use:1;
	u8 cmpl_non_txbssid:1;
#else
	u8 cmpl_non_txbssid:1;
	u8 sae_pwd_id_in_use:1;
	u8 sae_pwd_id_used_exclusively:1;
	u8 rsv83:1;
	u8 rsv84:1;
	u8 rsv85:1;
	u8 rsv86:1;
	u8 rsv87:1;
#endif /* RT_BIG_ENDIAN */
} EXT_CAP_INFO_ELEMENT, *PEXT_CAP_INFO_ELEMENT;

#endif
/* APIs */

struct radio_info_db *topo_srv_get_radio(struct _1905_map_device *_1905_device, unsigned char *identifier);
int delete_agent_ch_prefer_info(struct own_1905_device *ctx, struct _1905_map_device *dev);
struct iface_info *topo_srv_get_iface(struct _1905_map_device *device, u8 * mac);

int parse_backhaul_steering_request_message(struct own_1905_device *ctx, unsigned char *buf);

int parse_backhaul_steering_request_message(struct own_1905_device *ctx, unsigned char *buf);

int parse_backhaul_steering_request_message(struct own_1905_device *ctx, unsigned char *buf);

int topo_srv_update_wireless_setting(struct own_1905_device *ctx, unsigned char *msg_buf, int len);
int topo_srv_update_channel_setting(struct own_1905_device *ctx,
	unsigned char *msg_buf, int len, struct ch_sel_rsp_info *ch_sel_rsp);
int topo_srv_handle_metrics_query(struct own_1905_device *ctx, unsigned char *buf, int len, unsigned char periodic);

int topo_srv_handle_assoc_sta_metrics_query(struct own_1905_device *ctx, unsigned char *buf, int len);

int topo_srv_handle_link_metrics_query(struct own_1905_device *ctx, unsigned char *buf);

int topo_srv_handle_unassoc_sta_link_metrics_query(struct own_1905_device *ctx, unsigned char *buf);

int topo_srv_handle_beacon_metrics_query(struct own_1905_device *ctx, unsigned char *temp_buf);

int topo_srv_handle_all_topology_event(struct own_1905_device *ctx, unsigned char *msg_buf, int len);

int topo_srv_update_chan_preference(struct own_1905_device *ctx, unsigned char *msg_buf, int len);

int topo_srv_handle_client_steer_btm_report(struct own_1905_device *ctx, unsigned char *msg_buf, int len);

int topo_srv_handle_steer_complete(struct own_1905_device *ctx, unsigned char *msg_buf, int len);

int topo_srv_handle_link_metrics_rsp_event(struct own_1905_device *ctx, unsigned char *buf, int len);

int topo_srv_update_own_radio_info(struct own_1905_device *ctx, unsigned char *buf);

int topo_srv_update_own_bss_info(struct own_1905_device *ctx, unsigned char *buf);

int topo_srv_handle_assoc_link_metrics_rsp(struct own_1905_device *ctx, unsigned char *msg_buf, int len);

int topo_srv_handle_unassoc_link_metrics_rsp(struct own_1905_device *ctx, unsigned char *msg_buf, int len);

int topo_srv_handle_beacon_metrics_rsp_event(struct own_1905_device *ctx, unsigned char *msg_buf, int len);

int topo_srv_handle_backhaul_steer_rsp(struct own_1905_device *ctx, unsigned char *msg_buf, int len);

int delete_exist_ap_metrics_info(struct own_1905_device *ctx, unsigned char *bssid);

int insert_new_metrics_info(struct own_1905_device *ctx, struct ap_metrics_info *minfo);
#ifdef MAP_R2
int insert_new_radio_metrics_info(struct own_1905_device *ctx, struct radio_metrics_info *minfo);
#endif
int topo_srv_update_bss_chan_util(struct own_1905_device *ctx, struct ap_metrics_info *minfo);

int delete_exist_traffic_stats_info(struct own_1905_device *ctx, unsigned char *identifier);

int insert_new_traffic_stats_info(struct own_1905_device *ctx, struct sta_traffic_stats *traffic_stats);
#ifdef SUPPORT_MULTI_AP
int insert_new_link_metrics_info(struct _1905_context *_1905_ctrl,struct own_1905_device *ctx, struct sta_link_metrics *metrics_info);
#endif
int update_one_sta_link_metrics_info(struct own_1905_device *ctx, struct link_metrics *metrics);

int delete_exist_unlink_metrics_rsp(struct unlink_metrics_info *unlink_metrics);

int update_unlink_metrics_rsp(struct unlink_metrics_info *unlink_metrics_ctx,
		struct unlink_metrics_rsp *unlink_metrics);

int topo_srv_update_assoc_client_info(struct own_1905_device *ctx, struct map_client_association_event_local *cinfo);

int topo_srv_parse_backhaul_ready_evt(struct own_1905_device *ctx, struct bh_link_info *bh_info);

int topo_srv_update_radio_basic_cap(struct own_1905_device *ctx,
		struct _1905_map_device *dev, struct ap_radio_basic_cap *bcap);

int topo_srv_update_operation_restriction(struct own_1905_device *ctx,
		struct _1905_map_device *dev, struct restriction *restrict);

int topo_srv_update_ap_ht_cap(struct own_1905_device *ctx, struct _1905_map_device *dev, struct ap_ht_capability *pcap);

int topo_srv_update_ap_vht_cap(struct own_1905_device *ctx,
		struct _1905_map_device *dev, struct ap_vht_capability *pcap);
int topo_srv_update_ap_he_cap(struct own_1905_device *ctx,
		struct _1905_map_device *dev, struct ap_he_capability *pcap);
int topo_srv_update_channel_preference(struct own_1905_device *ctx,
		struct _1905_map_device *dev, struct ch_prefer *prefer);

void retrigger_ch_planning(void *eloop_ctx, void *timeout_ctx);

int get_bandcap(unsigned char opclass, unsigned char non_opch_num, unsigned char *non_opch);

unsigned short WscGetEncryType(unsigned int encryType);
unsigned short WscGetAuthType(unsigned int authType);

unsigned char check_invalid_channel(unsigned char op_class, unsigned char ch_num, struct channel_selection_chan_list *ch_list);
#ifdef SUPPORT_1905
unsigned short topo_srv_ap_metrics_rsp_message(struct own_1905_device *ctx,
		struct ap_metrics_info_lib **info, int *ap_metrics_info_cnt,
		struct stat_info **sta_stats, int *sta_stats_cnt,
					       struct link_metrics **sta_metrics, int *sta_metrics_cnt
#ifdef MAP_R2					       
					       , struct ap_extended_metrics_lib **ext_ap_metric, int *ext_ap_met_cnt,
					       struct sta_extended_metrics_lib **ext_sta_metric, int *ext_sta_met_cnt,
							struct radio_metrics_lib **info_radio, int *radio_metrics_info_cnt,
							struct ch_util_lib **ch_util, int *ch_util_cnt, unsigned char periodic
#endif
							);

unsigned short topo_srv_sta_metrics_rsp_message(struct own_1905_device *ctx,
				struct link_metrics **sta_metrics, unsigned char *sta_metrics_cnt
#ifdef MAP_R2
				, struct sta_extended_metrics_lib **ext_sta_metric, unsigned char *ext_sta_met_cnt
#endif
				);
#endif
int topo_srv_init_own_info(struct own_1905_device *ctx);

unsigned char topo_srv_get_channel_util(struct own_1905_device *ctx, unsigned char channel_no);
unsigned char topo_srv_get_max_congested_channel(struct own_1905_device *ctx);
struct radio_info_db *topo_srv_get_next_radio(struct _1905_map_device *device, struct radio_info_db *radio);
struct _1905_map_device *topo_srv_get_next_1905_device(struct own_1905_device
		*ctx, struct _1905_map_device
		*_1905_dev);

int topo_srv_get_root_distance(struct own_1905_device *ctx, struct _1905_map_device *_1905_dev);

int topo_srv_get_5g_bh_ap_bssid(struct own_1905_device *ctx, struct _1905_map_device *_1905_dev, unsigned char *bssid);
int topo_srv_get_2g_bh_ap_bssid(struct own_1905_device *ctx, struct _1905_map_device *_1905_dev, unsigned char *bssid);
struct _1905_map_device *topo_srv_get_1905_by_bssid(struct own_1905_device *ctx, unsigned char *ifaddr);
struct bss_info_db *topo_srv_get_bss_by_bssid(struct own_1905_device *ctx, struct _1905_map_device *dev,
		unsigned char *ifaddr);
struct _1905_map_device *topo_srv_get_upstream_device(struct _1905_map_device *_1905_dev);
struct _1905_map_device *topo_srv_get_1905_device(struct own_1905_device *ctx, u8 * al_mac);

struct _1905_map_device *topo_srv_get_controller_device(struct own_1905_device *ctx);

struct iface_info *topo_srv_get_interface(struct own_1905_device *ctx, struct _1905_map_device *_1905_dev,
		unsigned char *ifaddr);
int topo_srv_update_upstream_device(struct own_1905_device *ctx, struct _1905_map_device *tmp_dev,
		unsigned char *mac_addr);
int topo_srv_deinit_own_info(struct own_1905_device *ctx);
int topo_srv_parse_combined_infra_msg(struct own_1905_device *ctx, unsigned char *buf);
void topo_srv_get_radio_capinfo(struct own_1905_device *ctx, unsigned char *identifier);
unsigned short append_ap_metrics_info(unsigned char *pkt, struct bss_info_db *mrsp);
struct map_neighbor_info *topo_srv_get_neighbor(struct own_1905_device *ctx, struct _1905_map_device *tmp_dev,
		unsigned char *almac);
void topo_srv_get_ap_metrics_info(struct own_1905_device *ctx);
void topo_srv_get_assoc_sta_traffic_stats(struct own_1905_device *ctx);
void topo_srv_get_all_assoc_sta_link_metrics(struct own_1905_device *ctx);
#ifdef MAP_R2
void topo_srv_get_radio_metrics_info(struct own_1905_device *ctx);
void topo_srv_get_all_radio_metrics_info(struct own_1905_device *ctx);
int dump_ch_plan_score_info(struct own_1905_device *own_dev, char *buf, char* reply_buf, size_t buf_Len);
#endif
struct bss_info_db *topo_srv_get_next_bss(struct _1905_map_device *device, struct bss_info_db *bss);
int topo_srv_get_peer_relation(struct own_1905_device *ctx, unsigned char *almac);
struct bss_info_db *topo_srv_get_bss(struct _1905_map_device *_1905_device, char *bssid);
int topo_srv_prase_ap_cap_report(struct own_1905_device * ctx, unsigned char *buf);
int topo_srv_handle_topology_event(struct own_1905_device *ctx, unsigned char *buf, int len);
int delete_exist_metrics_policy(struct metrics_policy *mpolicy);
int parse_link_metrics_response_message(struct _1905_map_device *dev, unsigned char *buf);
int topo_srv_update_1905_ap_cap(struct _1905_map_device *dev, struct ap_capability *cap);
struct _1905_map_device *topo_srv_get_1905_by_iface_addr(struct own_1905_device *ctx, unsigned char *ifaddr);
int topo_srv_update_ap_cap(struct own_1905_device *ctx, struct ap_capability *cap);
int get_band_from_channel(int chan);
int topo_srv_update_channel_info(struct own_1905_device *ctx, struct bss_info_db *bss);
void topo_srv_start_1905_timer(void *eloop_ctx, void *timeout_ctx);
int topo_srv_start_combined_infra_metrics_srv(struct own_1905_device *ctx);
int topo_srv_deinit_topo_srv(struct own_1905_device *ctx);
Boolean topo_srv_is_btm_steer_disallowed(struct own_1905_device *ctx, unsigned char *mac);
Boolean topo_srv_is_local_steer_disallowed(struct own_1905_device *ctx, unsigned char *mac);
int topo_srv_get_radio_steer_policy(struct own_1905_device *ctx, unsigned char *identifier, int8_t *radio_policy);
int topo_srv_get_rssi_th_by_policy(struct own_1905_device *ctx, struct mapd_radio_info *radio_info, int8_t *rssi);
int topo_srv_parse_wapp_event(struct mapd_global *global, char *buf, int len, int from);
int topo_srv_dump_topology(struct own_1905_device *ctx);
void topo_srv_issue_scan(struct own_1905_device *ctx);
void topo_srv_update_1905_dev_vendor(struct own_1905_device *ctx, struct _1905_map_device *_1905_device, enum map_vendor vendor);
int topo_srv_send_vendor_oui_tlv(struct own_1905_device *ctx, unsigned char *al_mac_addr);
int topo_srv_send_vendor_chan_report_msg(struct own_1905_device *ctx, unsigned char *al_mac_addr);
int topo_srv_handle_ap_metrics_rsp(struct own_1905_device *ctx, unsigned char *buf, int len);
int topo_srv_dump_topology_v1(struct own_1905_device *ctx, char *buf, char* reply_buf, size_t buf_Len);
void topo_srv_update_device_role(struct own_1905_device *ctx, unsigned char *almac, int role);
struct radio_info_db *topo_srv_get_radio_by_channel(struct _1905_map_device *_1905_device, unsigned char chan);
struct radio_info_db *topo_srv_get_radio_by_band(struct _1905_map_device *_1905_device, unsigned char chan);
int topo_srv_cont_update_ap_metrics(struct own_1905_device *ctx);
int topo_srv_cont_update_link_metrics(struct own_1905_device *ctx);
void topo_srv_get_own_metrics_info(struct own_1905_device *ctx);
void topo_srv_get_own_link_metrics_info(struct own_1905_device *ctx);
int topo_srv_get_1905_dev_count(struct own_1905_device *ctx);
int topo_srv_issue_disconnect_if_local(struct own_1905_device *ctx, struct topo_notification *evt);
void duplicate_sta_check_for_1905_device(struct own_1905_device *ctx, struct _1905_map_device *dev);
void duplicate_sta_check_for_notification_evt(struct own_1905_device *ctx, struct topo_notification *evt);
int topo_srv_update_radio_info(struct own_1905_device *ctx,
	struct _1905_map_device *dev, struct channel_report *chan_rpt);
void topo_srv_attach_unmapped_bh_cli(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct bss_info_db *bss
	);
struct _1905_map_device *topo_srv_create_1905_device(struct own_1905_device *ctx, unsigned char *almac);
unsigned char topo_srv_wifi_uplink_exist(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct iface_info **uplink_iface,
	unsigned char *uplink_bssid);
int topo_srv_get_wireless_mode(int phy_mode);
int topo_srv_update_bss_role_for_controller(struct own_1905_device *dev);
void send_wapp_event_wireless_settings(struct own_1905_device *ctx,	struct bss_info_db *bss_bh);
void topo_serv_remove_remote_peers_recurse(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct _1905_map_device *exclude_dev);
void topo_srv_move_1905_off_nw (
	struct own_1905_device *ctx,
	struct _1905_map_device *dev);
int _1905_2_wapp_cert_channel_setting_event(struct own_1905_device *ctx, unsigned char *pkt, unsigned int pkt_len);
void wapp_set_metrics_policy_setting(struct own_1905_device *ctx, struct metrics_policy *mpolicy);
#ifdef SUPPORT_MULTI_AP
#ifdef MAP_R2
void wapp_set_unsuccessful_association_policy_setting(struct own_1905_device *ctx, struct unsuccessful_association_policy *assoc_failed_policy);
#endif
#endif
int topo_srv_dump_bh_all_info(struct own_1905_device *ctx);
int topo_srv_dump_sta_all_info(struct own_1905_device *ctx);
int mapd_set_acl_block(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len);
int mapd_set_scan_rssi_thresh(struct mapd_global *global, char *cmd_buf, int band);
#if defined(MAP_R2) || defined(CENT_STR)
int _1905_if_send_ap_metric_rsp(struct own_1905_device *own_dev, unsigned char periodic);
#else
int _1905_if_send_ap_metric_rsp(struct own_1905_device *own_dev);
#endif
void topo_srv_handle_local_leave(struct own_1905_device *ctx,
	unsigned char *mac_addr);
struct iface_info *topo_srv_get_next_iface(struct _1905_map_device *device, struct iface_info *iface);
void topo_srv_mark_all_iface_invalid(struct own_1905_device *ctx,
	struct _1905_map_device *dev);
void topo_srv_mark_all_oper_bss_invalid(struct own_1905_device *ctx,
	struct _1905_map_device *dev, struct radio_info_db *radio);
void topo_srv_remove_all_invalid_iface(struct own_1905_device *ctx,
	struct _1905_map_device *dev);
void topo_srv_remove_all_invalid_ap_iface(struct own_1905_device *ctx,
	struct _1905_map_device *dev);
void topo_srv_free_esp_record(struct bss_info_db *bss);
void topo_srv_remove_all_assoc_clients_for_bss(struct _1905_map_device *dev,
	struct bss_info_db *bss);
void topo_srv_update_rr_states_for_bss(struct own_1905_device *ctx,
	struct _1905_map_device *dev, struct bss_info_db *bss);
void topo_srv_remove_all_invalid_bss(struct own_1905_device *ctx,
	struct _1905_map_device *dev);

void topo_srv_update_1905_radio_capinfo(void *eloop_ctx, void *timeout_ctx);

void infra_metrics_srv_send_cb_infra_metrics(void *eloop_ctx, void *timeout_ctx);
int topo_srv_get_bssid_of_sta(struct own_1905_device *ctx, struct beacon_metrics_query *bcn_query, unsigned char *bssid);
struct associated_clients *topo_srv_get_associate_client(struct own_1905_device *ctx, struct _1905_map_device *dev, unsigned char *mac);
int topo_srv_update_own_device_config_status(struct own_1905_device *ctx);
int topo_srv_get_2g_bh_ap_channel(struct own_1905_device *ctx, struct _1905_map_device *_1905_dev, unsigned char *channel);
void topo_srv_get_5g_bh_ap_channel_by_band(struct own_1905_device *ctx, struct _1905_map_device *_1905_dev, unsigned char *channel, unsigned char band);
int topo_srv_get_5g_bh_ap_channel(struct own_1905_device *ctx, struct _1905_map_device *_1905_dev, unsigned char *channel);
Boolean check_is_triband(struct _1905_map_device *dev);
int get_band(struct _1905_map_device *dev,int chan);
void topo_srv_clear_ch_planning_states (
	struct own_1905_device *ctx,
	struct _1905_map_device *dev);
void topo_srv_ch_planning_on_agents (
	struct own_1905_device *ctx,
	struct _1905_map_device *dev);
#ifdef SUPPORT_MULTI_AP
void send_off_ch_scan_resp(struct mapd_global *pGlobal_dev,
	struct off_ch_scan_report_event *scan_rep_evt);
void send_net_opt_scan_resp(struct mapd_global *pGlobal_dev,
	struct net_opt_scan_report_event *scan_rep_evt);
void handle_off_ch_scan_req(struct mapd_global *pGlobal_dev,
	unsigned char *req,
	struct _1905_map_device *_1905_device_ptr,unsigned char type);
void handle_off_ch_scan_resp(struct mapd_global *pGlobal_dev,
	struct off_ch_scan_resp_tlv *resp,
	struct _1905_map_device *p1905_device);
void handle_net_opt_scan_resp(struct mapd_global *pGlobal_dev,
	struct net_opt_scan_resp_tlv *resp,
	struct _1905_map_device *p1905_device);
void send_off_ch_scan_req(struct mapd_global *pGlobal_dev,
	struct _1905_map_device *p1905_device, unsigned char mode,
	unsigned int band, unsigned char *list, unsigned char bw, unsigned char type);
#endif
void mapd_user_ch_pref_send(
	struct mapd_global *global,
	unsigned char *al_mac, unsigned char *identifier, struct prefer_info_db *prefer_db);
void mapd_send_onboardstatus_to_app(struct mapd_global *global, int onboard_status, unsigned char bh_type);
unsigned short append_ap_metrics_tlv(unsigned char *pkt, struct bss_info_db *mrsp);
void update_topology_info(struct _1905_map_device *child);
void topo_serv_update_own_topology_info(struct own_1905_device *ctx);
struct map_neighbor_info *topo_serv_find_cli_neighbor_by_mac(
   struct own_1905_device *ctx, struct _1905_map_device *dev,
   unsigned char *mac);
struct iface_info *topo_srv_find_iface_by_mac(
   struct own_1905_device *ctx, unsigned char *addr);
void topo_serv_remove_neighbor_peers(struct _1905_map_device *dev,
   struct _1905_map_device *ndev);
void topo_serv_remove_delete_link_peers(struct backhaul_link_info *bh,
   struct _1905_map_device *dev, struct _1905_map_device *ndev);
int lookup_iface_addr(char *iface_name, unsigned char *mac_addr);
void topo_srv_update_uplink_rate(struct own_1905_device *ctx, struct _1905_map_device *dev);
unsigned int estimate_radio_phyrate(struct own_1905_device *ctx, struct radio_info_db *radio,signed char RSSI,
	unsigned int wireless_mode, 
	unsigned int client_streams, unsigned int client_bandwidth);
unsigned int calculate_score(unsigned int estimated_rate, unsigned int uplink_rate,
	unsigned int hop_count);
unsigned int get_uplink_ethernet_hop_count(struct own_1905_device *ctx, struct _1905_map_device *dev);
void dump_uplink_rate(struct own_1905_device *ctx);
void sync_score_to_all_radios(struct own_1905_device *ctx, struct _1905_map_device *dev,
	struct map_neighbor_info *neighbor, unsigned char is_eth_bh,
	unsigned char wifi_bh_found, unsigned int score_24G,
	unsigned int score_5G);

#ifdef MAP_R2

int topo_srv_update_ch_scan_cap(struct own_1905_device *ctx,size_t len , struct channel_scan_capab *pcap);
int topo_srv_handle_ch_scan_req(struct own_1905_device *ctx, unsigned char *buf, int len);
#ifdef DFS_CAC_R2
int topo_srv_handle_cac_req(struct own_1905_device *ctx, unsigned char *buf, int len);
int topo_srv_handle_cac_terminate(struct own_1905_device *ctx, unsigned char *buf, int len);
void topo_srv_get_cac_statusinfo(struct own_1905_device *ctx);
#endif
int insert_new_ext_link_metrics_info(struct own_1905_device *ctx, struct ext_sta_link_metrics *metrics_info);
int update_one_sta_link_ext_metrics_info(struct own_1905_device *ctx, struct ext_link_metrics *metrics);
int topo_srv_update_metric_rep_intv_cap(struct own_1905_device *ctx, u32 *cap);
int topo_srv_update_r2_ap_cap(struct own_1905_device *ctx,size_t len , struct ap_r2_capability *pcap);
int mapd_send_ap_metric_msg(struct mapd_global *global);
int parse_ap_metrics_query_message(struct own_1905_device *ctx, unsigned char *buf);
int topo_srv_handle_ch_scan_report(struct mapd_global *global, unsigned char *buf, int len);
int update_policy_config(struct mapd_global *global, struct _1905_map_device *dev, int metric_inclusion);
int topo_srv_handle_assoc_status_notif_event(struct mapd_global *global, unsigned char *buf, int len);
int topo_srv_handle_disassoc_stats_event(struct mapd_global *global, unsigned char *buf, int len);
int dump_de_per_dev(struct own_1905_device *ctx, struct _1905_map_device *dev, char* reply_buf, size_t buf_len);
int dump_de(struct own_1905_device *ctx, char *buf, char* reply_buf, size_t buf_Len);
int dump_all_dev_scan_info(struct own_1905_device *own_dev, char *buf, char* reply_buf, size_t buf_Len);
int topo_srv_handle_tunneled_msg(struct mapd_global *global, unsigned char *buf, int len);
void topo_srv_update_ch_planning_info(
	struct own_1905_device * ctx,
	struct _1905_map_device *dev,
	struct bss_info_db *bss,
	struct radio_info_db *radio,
	u8 cu_tlv_update);
void topo_srv_update_ch_plan_scan_done(
	struct own_1905_device *own_dev,
	struct _1905_map_device *dev,
	struct radio_info_db *radio);
void topo_srv_clear_cac_cap_db(
	struct radio_info_db *radio);
void topo_srv_clear_cac_completion_status(
	struct radio_info_db *radio);
unsigned char is_profile2_dev(struct mapd_global *global,unsigned char *almac);
void de_stats_timeout(void * eloop_ctx, void *user_ctx);
int map_chk_candidate_bss(struct mapd_global *pGlobal_dev, struct client *map_client, struct bss_info_db *bss);
int topo_srv_handle_bh_sta_report(struct mapd_global *global, unsigned char *buf, int len);
int topo_srv_handle_ack_msg(struct mapd_global *global, unsigned char *buf, int len);
#endif
void topo_srv_clear_cac_completion_status(
	struct radio_info_db *radio);
int topo_srv_handle_client_cap_report(struct mapd_global *global, unsigned char *buf, unsigned short len);

void topo_srv_manage_bh_links(struct own_1905_device *ctx);
#ifdef SUPPORT_MULTI_AP
struct backhaul_link_info *topo_srv_get_bh_uplink_metrics_info(struct own_1905_device *ctx,
	struct bh_link_entry *bh_entry);
#endif
void hopcnt_to_controller(struct _1905_map_device *dev, char *hopcnt, char *term);
void topo_serv_clear_visit_node(struct own_1905_device *ctx);
void topo_srv_update_radio_config_status(struct own_1905_device *ctx, unsigned char *radio_id, char flag);
void net_opt_check_bh_scan_allowed(struct own_1905_device *ctx, struct bss_info_db *bss);
void reset_net_opt_allowed(struct own_1905_device *ctx);
int remove_duplicate_cli_single_bh(struct _1905_map_device *device, struct own_1905_device *ctx, unsigned char *ifaddr);
#ifdef SUPPORT_MULTI_AP
#ifdef MAP_R2
void reset_de_if_needed(struct own_1905_device *ctx);
int send_channel_scan_req( struct mapd_global *global, struct _1905_map_device *dev, unsigned char *scan_ch_list);
#endif
#endif
void send_vs_bh_priority(struct own_1905_device *ctx);
void handle_bh_priority_info_from_agent(struct mapd_global *pGlobal_dev,
	struct bh_priority_msg *bh_info,
	struct _1905_map_device *p1905_device);
void send_cac_start(struct mapd_global *pGlobal_dev);
#ifdef SUPPORT_MULTI_AP
void handle_cac_start_from_agent(struct mapd_global *pGlobal_dev,
	struct cac_start_tlv *cac_tlv,
	struct _1905_map_device *p1905_device);
#endif
int send_link_metrics_selective(struct associated_clients *metrics_ctx,struct own_1905_device *ctx);

#ifdef CENT_STR
int link_metrics_mon_rcpi_at_controller(struct associated_clients *metrics_ctx,struct own_1905_device *ctx);
int parse_assoc_req
(
	struct mapd_global *global,
	unsigned char *sta_mac,
	unsigned char *bssid,
	unsigned char *assoc_req,
	unsigned int assoc_len,
	unsigned char channel,
	unsigned char assoc_req_format
);

#endif
void retrigger_ch_planning_post_radar(void *eloop_ctx, void *timeout_ctx);
void topo_srv_update_wts_config(struct own_1905_device *ctx);
int get_op_class_channel_list(unsigned char op_class, struct prefer_info_db *perfer);
#ifdef ACL_CTRL
int topo_srv_dump_agent_info(struct own_1905_device *ctx);
#endif
#endif
