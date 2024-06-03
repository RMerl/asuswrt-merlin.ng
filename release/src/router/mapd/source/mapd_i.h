#ifndef mapd_I_H
#define mapd_I_H

#include "utils/bitfield.h"
#include "utils/list.h"
#include <sys/queue.h>
#include "metrics.h"
//#include "topologySrv.h"
//#include "ch_planning.h"
//#include "chan_mon.h"



#ifdef CENT_STR
#define CENT_STR_1_MIN 60
#endif


#ifndef GNU_PACKED
#define GNU_PACKED  __attribute__ ((packed))
#endif /* GNU_PACKED */
#ifdef SUPPORT_MULTI_AP
#define OUI_LEN		3
extern u8 MTK_OUI[OUI_LEN];
/*
 * Forward declarations of private structures used within the ctrl_iface
 * backends. Other parts of mapd do not have access to data stored in
 * these structures.
 */
struct _1905_map_device;
#endif
#define TRIBAND 3
#ifdef MAP_R2
#define STEERING_R2 1  
#endif
#define MAX_NUM_OF_RADIO 6
#define MAX_NUM_OF_BSS_PER_RADIO 16
#define MAX_NUM_BSS (MAX_NUM_OF_BSS_PER_RADIO * MAX_NUM_OF_RADIO) //Maximum number of BSSs per device
#ifdef CONFIG_SUPPORT_OPENWRT
#define MAX_STA_SEEN 128
#else
/* Limited NVRAM on LSDK. */
#define MAX_STA_SEEN 100
#endif

#define MIN_BL_FAIL_CNT 2

#define WMODE_CAP_AC(_x)        (((_x) & (WMODE_AC)) != 0)
#define WMODE_CAP_N(_x)         (((_x) & (WMODE_GN | WMODE_AN)) != 0)
#define WMODE_CAP_AX(_x) ((_x) & (WMODE_AX_24G | WMODE_AX_5G))
#define STEER_ENABLE 1
#define STEER_DISABLE 0
#define NON_MAP_ENABLE 1
#define NON_MAP_DISABLE 0
#ifdef SUPPORT_MULTI_AP
#define REMOTE_WPS_VENDOR_LEN 8
#endif
#define TLV_802_11_VENDOR_SPECIFIC	11
#define MAX_FILE_PATH_LENGTH 128
#define MIN_STEER_RETRY_TIME 5 // seconds
#define MAX_STEER_RETRY_TIME 30 // seconds
#define STEER_RETRY_STEP_CNT 3
#ifdef SUPPORT_MULTI_AP
#define FUNC_RFS_REQ 	3
#define FUNC_RFS_RSP	4
#define FUNC_TSQ_REQ	5
#define FUNC_TSQ_RSP	6
#define FUNC_VENDOR_OUI	7
#define FUNC_VENDOR_CHAN_REPORT 8
#define FUNC_VENDOR_TRIGER_WPS 9
#define FUNC_VENDOR_SET_TX_POWER_PERCENTAGE	 10
#define FUNC_VENDOR_OFF_CH_SCAN_REQ			11
#define FUNC_VENDOR_OFF_CH_SCAN_RESP		12
#define FUNC_VENDOR_NET_OPT_SCAN_RESP 13
#define FUNC_VENDOR_NET_OPT_SCAN_REQ  14
#define FUNC_CAC_START 15
#define FUNC_BH_PRIORITY_INFO 16
#ifdef ACL_CTRL
#define FUNC_VENDOR_ACL_CTRL  17
#endif
#endif
#define MAX_BH_OL_STEER_COUNT 20
#define MAX_BH_OL_FORBID_TIME 300 // seconds
#define MAX_ALLOWED_SEC_LINK_SCAN 5
#define DEFAULT_BH_STEER_TIMEOUT  120
#define SEC_BH_LINK_RESTORE	  110
#ifdef MAP_R2
#define DFS_CH_CLEAR_INDICATION 0xB
#define FUNC_VENDOR_CHANNEL_UTIL_RSP 15
#endif
enum map_device_type {
	DEV_TYPE_UNKNOWN=0,
	DEV_TYPE_R1,
	DEV_TYPE_R2,
	DEV_TYPE_CLIENT
};

#define MAX_SET_BSS_INFO_NUM 26
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

#ifdef MAP_R2
enum map_steer_reason_code {
	STEER_REASON_UNSPECIFIED=0,
	STEER_REASON_PREMIUM_AP=9,

};

#endif
/**
 * struct mapd_params - Parameters for mapd_init()
 */
struct mapd_params {
	/**
	 * daemonize - Run %mapd in the background
	 */
	int daemonize;

	/**
	 * pid_file - Path to a PID (process ID) file
	 *
	 * If this and daemonize are set, process ID of the background process
	 * will be written to the specified file.
	 */
	char *pid_file;
#ifdef CONFIG_CLIENT_DB_FILE
	char * clientDBname;
#endif

	/**
	 * mapd_debug_level - Debugging verbosity level (e.g., MSG_INFO)
	 */
	int mapd_debug_level;

	/**
	 * mapd_debug_timestamp - Whether to include timestamp in debug messages
	 */
	int mapd_debug_timestamp;

	/**
	 * ctrl_interface - Global ctrl_iface path/parameter
	 */
	char *ctrl_interface;
    char *ctrl_interface_group;

	/**
	 * mapd_debug_file_path - Path of debug file or %NULL to use stdout
	 */
	const char *mapd_debug_file_path;
#ifdef SUPPORT_MULTI_AP
	unsigned char Certification;			/*for Certification*/
#endif
	/**
	 * mapd_debug_syslog - Enable log output through syslog
	 */
	int mapd_debug_syslog;

	/**
	 * mapd_debug_tracing - Enable log output through Linux tracing
	 */
	int mapd_debug_tracing;
	int core_dump;
};

struct steer_params {
	uint8_t CUOverloadTh_2G;
	uint8_t CUOverloadTh_5G_L;
	uint8_t CUOverloadTh_5G_H;
	uint8_t CUSafetyTh_2G;
	uint8_t CUSafetyTh_5G_L;
	uint8_t CUSafetyh_5G_H;
	int8_t MinRSSIOverload;
	int8_t RSSISteeringEdge_DG;
	int8_t RSSISteeringEdge_UG;
	uint32_t MCSCrossingThreshold_DG;
	uint32_t MCSCrossingThreshold_UG;
	int8_t RSSICrossingThreshold_DG;
	int8_t RSSICrossingThreshold_UG;
	uint32_t phy_scal_factx100;
	Boolean PHYBasedSelection;
	uint8_t RSSIAgeLim; //in seconds
	uint8_t RSSIAgeLim_preAssoc; //in seconds
	uint8_t RSSIMeasureSamples;
	uint32_t ForceStrBlockTime;
	uint32_t BTMStrBlockTime;
	uint32_t ForceStrForbidTime;
	uint32_t BTMStrForbidTime;
	uint32_t StrForbidTimeJoin;
	uint32_t BTMStrTimeout;
	uint32_t ForceStrTimeout;
	uint8_t MinSteerRetryTime;
	uint8_t MaxSteerRetryTime;
	uint8_t SteerRetryStep;
#ifdef SUPPORT_MULTI_AP
	//MultiAPSteering thresholds
	
	int8_t LowRSSIAPSteerEdge_root; 
	int8_t LowRSSIAPSteerEdge_RE;
	int8_t MinRssiIncTh_Root;
	int8_t MinRssiIncTh_RE;
	int8_t MinRssiIncTh_Peer;
#endif
	u8 CUAvgPeriod;
	uint32_t MaxClientOverloaded;
#ifdef SUPPORT_MULTI_AP
	uint8_t single_steer;
#endif
	uint32_t ActivityThreshold; //ACtivity Threshold in bytes per second
	uint8_t StartInActive;
#ifdef SUPPORT_MULTI_AP
	int8_t force_roam_rssi_th;
#endif
	/* Resets CSBC Unfriendly State when STA joins/roams on its own */
	uint8_t reset_btm_csbc_at_join;
#ifdef SUPPORT_MULTI_AP
	// CLI steer rssi monitoring threshould
	int8_t cli_rssi_threshold;
#endif
	uint8_t prohibitTime11K;
	/* Stering Control */
	uint8_t disable_pre_assoc_strng;
	uint8_t disable_post_assoc_strng;
	uint8_t ForcedRssiUpdate;
    uint8_t disable_offloading;
#ifdef SUPPORT_MULTI_AP
	uint8_t disable_nolmultiap;
#endif
    uint8_t disable_active_ug;
    uint8_t disable_active_dg;
    uint8_t disable_idle_ug;
    uint8_t disable_idle_dg;
	/* Prohibit Timer per device */
	unsigned int  GlobalProhibitTime;
	uint8_t idle_count_th;
#ifdef CENT_STR	
	uint8_t cent_str_max_steer_cand;
	uint8_t cent_str_max_bs_fail;
	uint8_t	cent_str_max_ol_steer_cand;
	uint8_t	cent_str_max_ug_steer_cand;
	uint8_t	cent_str_cu_mon_time;
	uint8_t	cent_str_cu_mon_prohibit_time;
#endif
};

struct mapd_bss {
	struct dl_list bss_entry;
	uint8_t ssid_len;
	uint8_t ssid[33];
	uint8_t bssid[ETH_ALEN];
	uint8_t channel;
	uint8_t assoc_sta_cnt;
	uint8_t bss_idx; // local index maintained within system
	uint8_t radio_idx;
	/* List of all clients blacklisted by this BSS */
	struct dl_list bl_sta_list;
	/* List of all clients connected to this BSS */
	struct dl_list assoc_sta_list;
	
	u8 steer_req_len; // length 0 means not valid.
	struct os_time steer_req_timestamp;
#ifdef SUPPORT_MULTI_AP
	u32 mandate_steer_done_bitmap;
	struct steer_request *_1905_steer_req_msg;
#endif
};
#ifdef SUPPORT_MULTI_AP
struct metric_report_policy_params {
	unsigned char RadioBand;
	unsigned char MetricPolicyRcpi;
	unsigned char MetricPolicyHys;
	unsigned char MetricPolicyMetricsInclusion;
	unsigned char MetricPolicyTrafficInclusion;
	unsigned char MetricPolicyChUtilThres;
};

struct metric_report_policy {
	u8 report_interval;
	struct metric_report_policy_params policy_params[3];
};

#ifdef MAP_R2
struct GNU_PACKED unsuccessful_association_policy {
	unsigned char report_unsuccessful_association;
	u32 max_supporting_rate;
};
#endif

struct map_1905_device;
#endif
#ifndef MAX_NODES
#define MAX_NODES 50
#endif

struct client;

struct GNU_PACKED vht_cap {
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

struct GNU_PACKED ht_cap {
	unsigned char tx_stream;
	unsigned char rx_stream;
	unsigned char sgi_20;
	unsigned char sgi_40;
	unsigned char ht_40;
	unsigned char band;
};

struct GNU_PACKED he_cap
{
	unsigned char he_mcs_len;
	unsigned char he_mcs;
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

struct GNU_PACKED he_mac_capinfo {
	u32 mac_capinfo_1;
	u16 mac_capinfo_2;
};

struct GNU_PACKED he_phy_capinfo {
	u32 phy_capinfo_1;
	u32 phy_capinfo_2;
	u8 phy_capinfo_3;
	u8 phy_capinfo_4;
	u8 phy_capinfo_5;
};

struct GNU_PACKED he_txrx_mcs_nss {
	u16 max_rx_mcs_nss;
	u16 max_tx_mcs_nss;
};

struct GNU_PACKED he_cap_ie {
	struct he_mac_capinfo mac_cap;
	struct he_phy_capinfo phy_cap;
	struct he_txrx_mcs_nss txrx_mcs_nss;
};

/* HE PHY Capabilities Information field */
/* Channel Width Set subfield */
enum he_channel_width_set {
	SUPP_40M_CW_IN_24G_BAND = 1,
	SUPP_40M_80M_CW_IN_5G_BAND = (1 << 1),
	SUPP_160M_CW_IN_5G_BAND = (1 << 2),
	SUPP_160M_8080M_CW_IN_5G_BAND = (1 << 3),
	SUPP_20MSTA_RX_242TONE_RU_IN_24G_BAND = (1 << 4),
	SUPP_20MSTA_RX_242TONE_RU_IN_5G_BAND = (1 << 5)
};



#define MAX_CHANNEL_BLOCKS 4
struct mapd_radio_info {
	unsigned char identifier[ETH_ALEN];
	unsigned char op_class;
	uint8_t channel;
	signed char tx_power;
	uint8_t radio_idx;
	uint8_t ch_util;
	struct ht_cap ht_capab;
	struct vht_cap vht_capab;
	uint32_t bss_bitmap;
	/* List of all BSSs on this radio */
	WIFI_MODE wireless_mode;
	struct dl_list bss_list;
#ifdef SUPPORT_MULTI_AP
	uint8_t CuThCrossSend;
#endif
	struct he_cap he_capab;
	uint32_t cu_ol_count;
#ifdef MAP_R2
	struct radio_metrics_info radio_metrics;
	u8 bootup_run;
#endif
	unsigned long last_byte_count;
};

struct mac_addr_node {
	u8 addr[ETH_ALEN];
	SLIST_ENTRY(mac_addr_node) next_node;
};

struct GNU_PACKED sta_db {
        unsigned char mac[ETH_ALEN];
        SLIST_ENTRY(sta_db) sta_entry;
};
#ifdef SUPPORT_MULTI_AP
#define AGENT_STEER_DISALLOWED		0x00
#define AGENT_STEER_RSSI_MANDATED	0x01
#define AGENT_STEER_RSSI_ALLOWD		0x02

struct GNU_PACKED radio_policy_db {
        unsigned char identifier[ETH_ALEN];
        unsigned char steer_policy;
        unsigned char ch_util_thres;
        unsigned char rssi_thres;
        SLIST_ENTRY(radio_policy_db) radio_policy_entry;
};

struct GNU_PACKED steer_policy {
        unsigned char local_disallow_count;
        SLIST_HEAD(list_head_local_steer, sta_db) local_disallow_head;
        unsigned char btm_disallow_count;
        SLIST_HEAD(list_head_btm_steer, sta_db) btm_disallow_head;
        unsigned char radios;
        SLIST_HEAD(list_head_radio_policy, radio_policy_db) radio_policy_head;
};

struct GNU_PACKED metric_policy_db {
        unsigned char identifier[ETH_ALEN];
        unsigned char rssi_thres;
        unsigned char hysteresis_margin;
        unsigned char ch_util_thres;
        unsigned char sta_stats_inclusion;
        unsigned char sta_metrics_inclusion;
        SLIST_ENTRY(metric_policy_db) policy_entry;
};

struct GNU_PACKED metrics_policy {
        unsigned char report_interval;
        unsigned char radio_num;
        SLIST_HEAD(list_head_metric_policy, metric_policy_db) policy_head;
};
#ifdef MAP_R2
struct GNU_PACKED ch_scan_policy {
	u8 rep_independent_scan;
};
#endif
struct GNU_PACKED policy_config {
        struct steer_policy spolicy;
        struct metrics_policy mpolicy;
#ifdef MAP_R2
		struct ch_scan_policy scan_policy;
		struct unsuccessful_association_policy assoc_failed_policy;
#endif
};

struct GNU_PACKED tlv_head
{
	u8 tlv_type;
	u16 tlv_len;
	u8 oui[OUI_LEN];
	u8 func_type;
};

enum man_rr_state
{
	RR_NONE,
	STEER_REQ_TRIGGERED,
	SILENT_PERIOD
};

struct rr_steer_controller
{
	u8 can_trigger_steer_req;
	struct os_time rr_steer_req_timestamp; // time when last steer_req was sent.
	struct os_time rr_silent_period_timestamp;
	struct _1905_map_device *p_current_1905_rr; // current 1905 device which has steer opportunity
	enum man_rr_state rr_state;
	u16 opp_window;
	u16 silent_window;
	u16 btm_timer;
	
};


struct controller
{
	struct rr_steer_controller rr_control;
	struct metric_report_policy ap_metric_policy;
	struct steer_policy cli_steer_policy;
};


struct agent
{
	struct dl_list Cli_steer_list;
	//struct ap_metric_report_policy Ap_metric_report_policy;
	struct policy_config map_policy;
	struct mapd_bss *steer_req_bss; // bss on which steer req is valid. NULL otherwise
};

struct topology_channel {
	unsigned char channel_no;
	unsigned char channel_util;
	SLIST_ENTRY(topology_channel) next_channel;
};
#endif
struct steer_cands {
	struct client *steer_cand;
	u8 steer_cand_home_bssid[ETH_ALEN];
	SLIST_ENTRY(steer_cands) next_cand;
};

#ifdef CENT_STR
struct cent_steer_cands {
	struct client *steer_cand;
	u8 steer_cand_home_bssid[ETH_ALEN];
	STAILQ_ENTRY(cent_steer_cands) next_cand;
};
#endif

#ifdef SUPPORT_MULTI_AP
#define MIN_POSSIBLE_BH_PRIORITY 3
#define MAX_POSSIBLE_BH_PRIORITY 1
struct scan_bss_list {
	struct bss_info bss;
	struct os_time time;
	struct bh_link_entry *bh_entry;
	unsigned long long estimated_rate;
	unsigned long long estimated_score;	
	SLIST_ENTRY(scan_bss_list) next_bss;
};

#define DEVICE_ROLE_INVALID -1
#define DEVICE_ROLE_UNCONFIGURED 0 
#define DEVICE_ROLE_CONTROLLER 1
#define DEVICE_ROLE_AGENT 2
#define DEVICE_ROLE_CONTRAGENT 3

struct prefered_ch_radio_info_db
{
	struct radio_info_db *radio;
	SLIST_ENTRY(prefered_ch_radio_info_db) next_pref_ch_radio;
};
struct prefered_ch_cb {
	unsigned char ch_num;
	unsigned char radio_count;
	unsigned char preference;
	unsigned char reason;
	unsigned char possible_op_class;
	signed int ch_score;
	SLIST_HEAD(list_prefered_channel_radios, prefered_ch_radio_info_db) first_radio; /**< list of radios attached to it */
	SLIST_ENTRY(prefered_ch_cb) next_prefered_ch;
};
struct operating_ch_cb {
	unsigned char ch_num;
	unsigned char radio_count;
	SLIST_HEAD(list_co_channel_radios, radio_info_db) first_radio; /**< list of radios attached to it */
	SLIST_ENTRY(operating_ch_cb) next_operating_ch;
};

struct ch_distribution_cb {
	unsigned char operating_ch_count;
	unsigned char prefered_ch_count;
	unsigned char user_prefered_ch;
	unsigned char  user_prefered_ch_HighBand;
	SLIST_HEAD(list_operating_channels, operating_ch_cb) first_operating_ch; /**< list of radios attached to it */
	SLIST_HEAD(list_prefered_channels, prefered_ch_cb) first_prefered_ch; /**< list of radios attached to it */
};

#define CHANNEL_PLANNING_IDLE				0
#define CHANNEL_SELECTION_REQ_SENT			1
#define CHANNEL_SELECTION_RSP_RECEIVED		2
#define CHANNEL_PLANNING_CAC_START			3
#define CHANNEL_PLANNING_CAC_FINISH			4



struct ch_planning_cb {
	void * current_ch_planning_dev;
	unsigned char ch_planning_state;
	unsigned char ch_planning_enabled;
	struct ch_distribution_cb ch_ditribution_2g;
	struct ch_distribution_cb ch_ditribution_5g;
	unsigned long last_byte_count;
	struct os_time last_high_byte_count_ts;
	unsigned long ChPlanningIdleByteCount;
	unsigned long ChPlanningIdleTime;
	u8 need_restart_ch_plan;
};
#if 0 
struct network_optmization_rr_control {
	enum man_rr_state ntwrk_opt_rr_state;
	u8 current_transaction_id;
	u32 wait_time;
	struct os_time ntwrk_change_ts;
	struct _1905_map_device *curr_rr_1905_device;
};
#endif
struct GNU_PACKED ntwrk_opt_req_tlv {
	struct tlv_head tlv;
	u8 transaction_id;
	u8 almac[ETH_ALEN];
	u8 req_type;
};

#define MAX_OFF_CH_SCAN_CH 	8
#define SCAN_MODE_BAND 		1
#define SCAN_MODE_CH		0
#define SCAN_2G				BIT(0)
#define SCAN_5GL			BIT(1)
#define SCAN_5GH			BIT(2)
struct GNU_PACKED off_ch_scan_req_msg_s {
	unsigned char mode;
	unsigned char band;
	unsigned char ch_list[MAX_OFF_CH_SCAN_CH];
	unsigned char bw;
};

struct GNU_PACKED off_ch_scan_req_tlv {
	struct tlv_head tlv;
	u32 transaction_id;
	struct off_ch_scan_req_msg_s scan_msg;
};

struct GNU_PACKED net_opt_scan_resp_tlv {
	struct tlv_head tlv;
	u32 transaction_id;
	struct net_opt_scan_report_event off_ch_scan_rep;
};


struct GNU_PACKED off_ch_scan_resp_tlv {
	struct tlv_head tlv;
	u32 transaction_id;
	struct off_ch_scan_report_event off_ch_scan_rep;
};

struct GNU_PACKED cac_start_info {
	u8 cac_enable;
	u8 cac_channel;
	u8 cac_timer;
};

struct GNU_PACKED cac_start_tlv {
	struct tlv_head tlv;
	struct cac_start_info cac_start;
};
struct GNU_PACKED bh_priority_tlv {
	u8 radio_id[ETH_ALEN];
	u8 bh_priority;
};

struct GNU_PACKED bh_priority_msg {
	struct tlv_head tlv;
	struct bh_priority_tlv bh_tlv[MAX_NUM_OF_RADIO];
};

#ifdef ACL_CTRL
typedef enum {
	ACL_ADD = 0,
	ACL_DEL,
	ACL_FLUSH,
	ACL_POLICY_0,
	ACL_POLICY_1,
	ACL_POLICY_2,
	ACL_SHOW,
} ACL_CMD_TYPE;

typedef enum {
	ACL_FUNC_ALL_DEV,
	ACL_FUNC_DEV,
	ACL_FUNC_BSSID,
	ACL_FUNC_REQ,
	ACL_FUNC_RSP,
	ACL_FUNC_MAX,
}ACL_FUNC_TYPE;

struct acl_cli{
	uint8_t mac_addr[ETH_ALEN];
	struct dl_list list_entry;
};

struct GNU_PACKED acl_ctrl_tlv {
	struct tlv_head tlv;
	struct acl_ctrl acl_info;
};

#define ACL_CTRL_TLV_LEN	sizeof(struct acl_ctrl_tlv)
#endif /*ACL_CTRL*/

#define BH_STATE_DEFAULT	 		0
#define BH_STATE_WIFI_BOOTUP 		1
#define BH_STATE_WIFI_LINK_FAIL 	2
#define BH_STATE_WIFI_BH_STEER 		3
#define BH_STATE_ETHERNET_PLUGGED	4
#define BH_STATE_ETHERNET_UPLUGGED	5
#define BH_STATE_WIFI_LINKUP		6
#define BH_STATE_WIFI_BAND_SWITCHED		7

#define BH_SUBSTATE_IDLE			0
#define BH_SUBSTATE_SCAN_DONE		1
#define BH_SUBSTATE_LINKDOWN_WAIT	2
#define BH_SUBSTATE_CONNECT_WAIT	3
#define BH_SUBSTATE_DUP_LINKDOWN_WAIT	4

/* No Loop Detected */
#define BH_SUBSTATE_NO_LINK_C 0
#define BH_SUBSTATE_W_LINK_C 1
#define BH_SUBSTATE_E_LINK_C 2
/* Loop Detected(ETH LOOP: No Action for now) */
#define BH_SUBSTATE_LOOP_DETECTED 3
/* Loop Detected and waiting for ApCli Link Down */
#define BH_SUBSTATE_LOOP_LINK_DOWN_WAIT 4
/* Loop Detected and waiting for Peer MAP dev to take Action */
#define BH_SUBSTATE_LOOP_PEER_ACTION_WAIT 5

#define MBH_COMPLETED 0
#define MBH_DISABLED 1
#define MBH_NOT_ALLOWED 2
#define MBH_TRIGGERED 3
#define MBH_PENDING 4

enum device_config_status {
	DEVICE_UNCONFIGURED,
	DEVICE_CONFIG_ONGOING,
	DEVICE_CONFIGURED,
};
#ifdef MAP_R2
enum de_state {
	OFF,
	SINGLE_AGENT,
	ALL_AGENTS,
};
#endif
struct bh_priority_info
{
	unsigned char priority;
	unsigned char priority_bkp;
	unsigned char attempt_allowed;
	unsigned char scan_triggered;
	unsigned char need_rssi_consider;
};

struct bh_link_entry
{
	unsigned char type;
	unsigned char ifname[IFNAMSIZ]; /*  */
	unsigned char mac_addr[ETH_ALEN];
	unsigned char bssid[ETH_ALEN];
	unsigned rssi_monitor;
	unsigned bh_assoc_state;
	unsigned bh_channel;
	char rssi;
	unsigned long scan_cookie;
	struct bh_priority_info priority_info;
	struct bss_info bss;
	u8 mbh_blocked;
	unsigned char radio_identifier[ETH_ALEN];
	SLIST_HEAD(list_scan, scan_bss_list) scan_bss_list_head;
	SLIST_ENTRY(bh_link_entry) next_bh_link;
};
#endif
#if 0 
struct network_optimization_cb {
	unsigned char network_optimization_state;
	unsigned char last_1905_backhaul_device[ETH_ALEN];
	unsigned int last_1905_device_score;	
	u32 ntwrk_opt_score_margin;
	struct bh_link_entry *ntwrk_opt_selected_bh_entry;
	struct scan_bss_list ntwrk_opt_selected_bss;
	struct ntwrk_opt_req_tlv network_optimization_req;
};
#endif
struct off_ch_scan_cb_s {
	unsigned int off_ch_token;
	struct _1905_map_device *_1905_dev_ptr;
};

struct agent_list {
	unsigned char almac[ETH_ALEN];
	SLIST_ENTRY(agent_list) next_agent;
};

struct blacklisted_ap_list {
	unsigned char bssid[ETH_ALEN];
	unsigned char fail_cnt;
	struct os_time time;
	SLIST_ENTRY(blacklisted_ap_list) next_bl_ap;
};
#define REASON_NOT_REQUIRED 0
#define REASON_RSSI_VARIATION 1
#define REASON_TOPOLOGY_CHANGE 2
#define REASON_ENABLED_BY_USER 3
#define REASON_TRY_AGAIN 4
#define REALIZATION_DONE 1
#define REALIZATION_ONGOING 2
#define TRIGGER_NEW_REALIZATION 3

#ifdef SUPPORT_MULTI_AP
typedef enum network_opt_state 
{
	NETOPT_STATE_IDLE,//0 main   
	NETOPT_STATE_DATA_COLLECTION_ONGOING,// 1main
	NETOPT_STATE_DATA_COLLECTION_COMPLETE,//2sub
	NETOPT_STATE_NEED_TO_ESTIMATE,//3sub
	NETOPT_STATE_OPT_NET_NEED_TO_REALIZE,//4main
	NETOPT_STATE_OPT_NET_REALIZATION_ONGOING,//5main
	NETOPT_STATE_OPT_NET_BHSTEER_DONE,//6sub
	NETOPT_STATE_COMPLETE,//7sub
} NETOPT_STATE;
struct optimized_network
{
	struct _1905_map_device *opt_net_dev;
	SLIST_ENTRY(optimized_network) next_opt_net_dev;
};

//-----------------------------------------------------------
#ifdef MAP_R2
/*All the below structures are for use in channel planning R2 Turnkey at the controller*/
typedef enum ch_planning_R2_state
{
	CHPLAN_STATE_IDLE, // 0
	CHPLAN_STATE_MONITOR,// 1
	CHPLAN_STATE_SCAN_ONGOING,// 2
	CHPLAN_STATE_SCAN_COMPLETE,// 3
	CHPLAN_STATE_CAC_ONGOING,// 4
	CHPLAN_STATE_CAC_TIMEOUT,// 5
	CHPLAN_STATE_CH_CHANGE_TRIGGERED,// 6
} CHPLAN_STATE;

struct Ch_threshold{
	u8 band;
	u8 ch_util_threshold;
	u8 edcca_threshold;
	u8 obss_load_threshold;
};

struct affected_agent_info
{
	struct _1905_map_device *affected_dev;
	SLIST_ENTRY(affected_agent_info) next_affected_agent;
};

struct monitor_ch_info
{
	u8 channel_num;
	u8 trigger_status;
	SLIST_HEAD(affected_agent_list, affected_agent_info) first_affected_agent;
	SLIST_ENTRY(monitor_ch_info) next_monitor_ch;
};

struct scan_list_info
{
	u8 al_mac[ETH_ALEN];
	u8 radio_identifier[ETH_ALEN];
	u8 trigger_done;
	SLIST_ENTRY(scan_list_info) next_scan_list;
};

struct score_info
{
	u8 channel;
	s32 total_score;
	s32 avg_score;
	u8 ch_rank;
	u8 dev_count;
	SLIST_ENTRY(score_info) next_ch_score;
};

struct grp_score_info
{
	u8 grp_channel_num;
	u8 grp_channel_list[8];
	s32 grp_total_avg_score;
	u8 grp_rank;
	u8 best_ch;
	s32 best_ch_score;
	SLIST_ENTRY(grp_score_info) next_grp_score;
};

struct ch_planning_R2_control{
	u8 ch_plan_enable;
	u8 ch_plan_metric_policy_interval;
	u32 ch_monitor_prohibit_wait_time;//secs
	struct os_time ch_monitor_start_ts;
	CHPLAN_STATE ch_plan_state;
	u16 min_score_inc; //in percentage
	struct Ch_threshold ch_plan_thres[MAX_NUM_OF_RADIO];
	u8 skip_edcca_check;
	SLIST_HEAD(monitor_ch_list, monitor_ch_info) first_monitor_ch;
	/*list of agents to whom scan req is triggered*/
	SLIST_HEAD(scan_list, scan_list_info) first_scan_list;
	/*channel num vs. cumulative score*/
	SLIST_HEAD(score_table, score_info) first_ch_score;
	u8 CAC_on_channel;
	u8 cac_ongoing;
	u8 force_trigger;
	u8 retry_count;
	u16 ch_monitor_timeout;//CHANNEL_MONITOR_TIMEOUT;
	struct ch_prefer_lib *ch_prefer_for_ch_select;//backup for setting dfs channel 
	u8 ch_prefer_count;
	/*channel group vs. group_byBW score*/
	u8 ch_plan_enable_bw;
	SLIST_HEAD(grp_score_table, grp_score_info) first_grp_score;
	u8 grp_bw;
};


/*per device per radio channel monitor stage related info*/
struct dev_ch_monitor{
	u32 avg_cu_monitor;
	u32 avg_obss_load;
	u32 avg_myTxAirtime;
	u32 avg_myRxAirtime;
	u8 count_cu_util;
	u8 count_radio_metric;
	u32 avg_edcca;
	u8 count_edcca_cu_tlv;
};

/*per device per radio ch plan control */
struct ch_plan_R2_1905dev{
	CHPLAN_STATE dev_ch_plan_state;
	struct os_time last_report_timestamp;
	struct dev_ch_monitor dev_ch_monitor_info;
};

typedef enum handle_pending_task
{
	TASK_USER_TRIGGERED_SCAN,
	TASK_NETWORK_OPT_TRIGGER,
	TASK_CHANNEL_PLANNING_TRIGGER,
} task_type;

struct task_info
{
	u8 task_type;
	struct monitor_ch_info *ch_info;
	struct channel_scan_req *scan_req;
	u8 almac[ETH_ALEN];
	SLIST_ENTRY(task_info) next_task;
};

#endif

//--------------------------------------------------------------

//for controller ctx (owndev)
struct network_optimization_control {
	unsigned char network_optimization_enabled;
	NETOPT_STATE network_opt_state;
	unsigned char NetOptReason;
	u32 data_collection_wait_time;
	u32 bh_steer_wait_time;
	u32 wait_time;
	unsigned int connect_wait_time; 
	unsigned int disconnect_wait_time;
	unsigned int post_cac_trigger_time;
	struct os_time ntwrk_change_ts;
	struct os_time trigger_netopt_ts;
	SLIST_HEAD(opt_topology_list, optimized_network) first_opt_net_dev;
	struct _1905_map_device *max_score_dev;
	unsigned int scan_2g_allow;
	unsigned int scan_5gl_allow;
	unsigned int scan_5gh_allow;
	unsigned char prefer_5G_bh;
	unsigned char prefer_5G_bh_try_cnt_user;
	unsigned char prefer_5G_bh_try_cnt_curr;
};
struct new_bh {
	struct bh_link_entry *new_selected_bh_entry;
	struct scan_bss_list new_selected_bss;
};
#endif

struct ch_util_params {
	unsigned char bh_switch_cu_en;
	uint32_t BHOLSteerCountTh;
	uint32_t BHOLForbidTime;
};

struct own_1905_device {
#ifdef SUPPORT_MULTI_AP
	u8 al_mac[ETH_ALEN];
	/* policy configuration parameters */
	struct policy_config map_policy;
	/* List of all clients blacklisted by this device */
	struct controller controller_context;
	struct agent agent_context;
	SLIST_HEAD(list_topology, _1905_map_device) _1905_dev_head;
	struct metrics_info metric_entry;
	/*backhaul steering request & response*/
	struct backhaul_steer_request bsteer_req;
	struct backhaul_steer_rsp bsteer_rsp;
	SLIST_HEAD(list_bh_links, bh_link_entry) bh_link_head;
	SLIST_HEAD(list_topology_channel, topology_channel) channel_head;
//	SLIST_HEAD(list_steer_cands, steer_cands) steer_cands_head;
	struct channel_report *chan_report;
	/* 0: unconfigured 1: controller 2: agent */
	int device_role;
	unsigned char lan_iface[IFNAMSIZ];
	unsigned char wan_iface[IFNAMSIZ];
	u8 rfs_transaction_id;
	unsigned char current_connect_priority;
	unsigned int bh_config_count;
	struct wsc_apcli_config bh_configs[MAX_NUM_OF_RADIO];
	unsigned char bh_ready_expected;
	unsigned char bh_priority_2g;
	unsigned char bh_priority_5gl;
	unsigned char bh_priority_5gh;
	struct ch_planning_cb ch_planning;
	//unsigned char conf_flag;
	enum device_config_status config_status;
	Boolean scan_triggered;
	u8 bh_steer_bssid[ETH_ALEN];
	unsigned char bh_steer_channel;
	unsigned char current_bh_state;
	/* Duplicate Link Maint */
	struct bh_link_entry *bh_dup_entry;
	struct bh_link_entry *primary_link;
	unsigned char num_wifi_itfs;
	struct local_interface **wifi_itfs;
	struct local_interface *eth_itf;

	unsigned char current_bh_substate;
	struct bh_link_entry *failed_link_bh_entry;
	struct bh_link_entry *bh_steer_bh_entry;
	unsigned char link_fail_single_channel_scan_count;
	unsigned char channel_planning_initial_timeout;	
	
	/* Network Optimization Feature*/
	//unsigned char network_optimization_enabled;
	//unsigned char network_optimization_enable_by_user;
	//unsigned char network_optimization_blocked;
	//unsigned int network_optimization_periodicity;
	struct bh_link_entry *last_connected_bh_entry;
	/*struct network_optimization_cb ntwrk_opt;*/
	
	/*  Network Optimization round robin control*/
	//struct network_optmization_rr_control network_optmization_rr;

	unsigned char upstream_device_present;
	/*dhcp_ctl*/
	unsigned char dhcp_ctl_enable;
	struct dhcp_ctl_req dhcp_req;
	char ipbuf[64];
	/* Dual Bh Feature*/
	unsigned char dual_bh_en;
	unsigned char load_balance_en;
	/*Auto BH Switch*/
	unsigned char auto_bh_switch;
	/* Track BH Loop */
	unsigned char bh_loop_state;
	unsigned int band_switch_time;
	struct off_ch_scan_cb_s off_ch_scan_cb;
	unsigned char ThirdPartyConnection;
	unsigned char enhanced_logging;
	unsigned char Restart_ch_planning_radar;
  	unsigned char ConnectThirdPartyVend;
	unsigned char own_new_DevRole;
	unsigned char auto_role_detect; /*1: ping state; 2: send search state*/
	SLIST_HEAD(list_agent, agent_list) a_list;
	SLIST_HEAD(list_bl_ap, blacklisted_ap_list) bl_ap_list;
	unsigned char conn_attempted_mac[ETH_ALEN];
	unsigned int bl_timeout;
	Boolean wsc_save_bh_profile;
	/*Centralized Network Optimization*/
	struct network_optimization_control network_optimization;
	struct new_bh new_bh_info;
	struct off_ch_scan_req_msg_s *net_opt_scan_msg ;
	unsigned char nw_opt_triggered_5G;
	unsigned char nw_opt_triggered_5G_in_process;
#ifdef MAP_R2
	struct channel_scan_capab *scan_capab;	
	struct ap_r2_capability *r2_ap_capab;
	u8 scan_capab_len;
	u8 r2_ap_capab_len;
#ifdef DFS_CAC_R2
	struct cac_capability_lib *cac_capab;
	u8 cac_capab_len;
	unsigned char Restart_ch_planning_radar_on_agent;
	u8 radar_channel_agent;
#endif
#endif
	wapp_device_status device_status;
	signed char rssi_threshold_2g;
	signed char rssi_threshold_5g;
#ifdef ACL_CTRL
	/* ACL CTRL list for all network */
	u8 acl_policy;
	struct dl_list acl_cli_list;
#endif
#endif
	struct steer_params cli_steer_params;
	u8 steer_cand_home_bssid[ETH_ALEN];
	char SetSteer;
	struct client *steer_cand ;//client for which steering is in progress
	/* Array representing client database */
	struct client client_db[MAX_STA_SEEN];
	/* Array of Radio interfaces */
	struct mapd_radio_info dev_radio_info[MAX_NUM_OF_RADIO];
	/* List of clients sorted by last seen */
	struct dl_list sta_seen_list; //struct client
	/* back pointer to global mapd */
	void *back_ptr;
	SLIST_HEAD(list_steer_cands, steer_cands) steer_cands_head;
	STAILQ_HEAD(list_cent_steer_cands, cent_steer_cands) cent_steer_cands_head;	
	/* Band switch by cu ol*/
	struct ch_util_params bh_cu_params;
	u8     max_allowed_scan;
	u8     sec_link_scan_cnt;
	u32    bh_steer_timeout;
	Boolean sec_bh_link_restore;
	unsigned char cac_enable;
	unsigned char cac_channel;
#ifdef STOP_BEACON_FEATURE
	unsigned char beacon_enable;
#endif
#ifdef SUPPORT_MULTI_AP
#ifdef MAP_R2
	u8	de_state;
	u8	de_cnt;
	struct ch_planning_R2_control ch_planning_R2;
	u8 user_triggered_scan;
#endif
#endif
	struct chnList dfs_info_ch_list[16];
#ifdef MAP_R2
	SLIST_HEAD(list_pending_tasks, task_info) task_list_head;
	u8 user_triggered_cac;
	u8 map_version;
	u8 dedicated_radio;
	u8 force_ch_change;
#endif
	unsigned char non_map_ap_enable;
#ifdef CENT_STR	
	uint8_t cent_str_en;
	struct _1905_map_device *p_cent_str_curr_1905_rr; // current 1905 device which has steer opportunity	
	SLIST_HEAD(cu_monitor_ch_list, cent_str_cu_mon_ch_info) cent_str_cu_mon_ch_list;
#endif	
	u8 ap_metric_rep_intv;
#ifdef MAP_R2
	u8 div_ch_planning;
#endif
	u8 need_to_update_wts;
	struct set_config_bss_info bss_config[MAX_SET_BSS_INFO_NUM];
};

/**
 * struct mapd_global - Internal, global data.
 *
 * This structure is initialized by calling mapd_init() when starting
 * %mapd.
 */

struct mapd_global {
	struct mapd_params params;
	struct ctrl_iface_global_priv *ctrl_iface;
	struct wapp_usr_intf_ctrl *wapp_ctrl;
#ifdef SUPPORT_MULTI_AP
	struct _1905_context *_1905_ctrl;
#endif
	struct own_1905_device dev;
	char wapp_wintf_status;
};
#ifdef SUPPORT_MULTI_AP
extern char g_map_cfg_path[MAX_FILE_PATH_LENGTH];
extern char g_map_1905_cfg_path[MAX_FILE_PATH_LENGTH];
#endif
extern char g_mapd_strng_path[MAX_FILE_PATH_LENGTH];

struct mapd_global * mapd_init(struct mapd_params *params);
int mapd_run(struct mapd_global *global);
void mapd_deinit(struct mapd_global *global);
struct mapd_bss * mapd_get_bss_from_bssid(struct mapd_global *global, unsigned char *bssid);
struct mapd_bss * mapd_get_bss_from_mac(struct mapd_global *global, u8 *mac_addr);
struct mapd_radio_info * get_radio_info_by_radio_id(struct mapd_global *global,
				                unsigned char *radio_id);
struct mapd_radio_info * mapd_get_radio_info_from_bss(struct mapd_global *global, struct mapd_bss *bss);
unsigned char *mapd_get_ssid_from_bssid(struct mapd_global *global, unsigned char *bssid);
void bss_init(struct mapd_radio_info *radio_info, unsigned char *bssid, unsigned char *ssid,u8 ssid_len, uint8_t bss_idx);
void bss_deinit(struct mapd_radio_info *radio_info, struct mapd_bss *bss);
void mapd_add_client_to_bss_bl_sta_list(struct mapd_global *global,
            u8 client_id, u8* bssid);
void mapd_add_client_to_bss_assoc_list(struct mapd_global *global,
            u8 client_id, u8* bssid);
void mapd_handle_radio_channel_change(struct mapd_radio_info *radio_info, uint8_t new_channel);
void mapd_handle_ap_metrics_info(struct mapd_global *global, u8 *bssid, u8 ch_util,
				unsigned short assoc_sta_cnt);
void mapd_radio_init(uint8_t radio_idx, struct mapd_radio_info *radio_info, uint8_t channel,
                uint8_t op_class, signed char tx_power, u8 *identifier);
void mapd_radio_deinit(struct mapd_global *global, struct mapd_radio_info *radio_info);
void mapd_init_radio_interface(struct mapd_radio_info *radio_info);
void reset_bss_idx_in_bitmap(struct mapd_radio_info *radio_info, uint8_t idx);
uint8_t get_free_bss_idx_in_bitmap(struct mapd_radio_info *radio_info);
uint8_t mapd_get_radio_idx_from_bssid(struct mapd_global *global, u8 *bssid);
uint8_t mapd_get_channel_from_bssid(struct mapd_global *global, u8 *bssid);
int mapd_get_mib(struct mapd_global *global, char *reply, int reply_size);
int mapd_get_mib_sta(struct mapd_global *global, char *buf, int  buf_len, int client_id);
int mapd_get_mib_options(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len);
#ifdef SUPPORT_MULTI_AP
enum steer_opp_allow mapd_is_steering_opp_recvd(struct mapd_global *global, u8 *bssid, u8 *mac_addr);
Boolean mapd_is_mandate_on(struct mapd_global *global, u8 *cli_mac_addr);
#endif
TRIGGER_TYPE mapd_get_trigger_from_steer_method(struct mapd_global *global,
                STEERING_METHOD_TYPE steer_method);
#ifdef SUPPORT_MULTI_AP
u8 *mapd_get_target_mandate_bssid(struct mapd_global *global,uint8_t client_id);
int map_1905_controller_found(struct own_1905_device *dev);
#endif
int map_get_info_from_wapp(struct own_1905_device *ctx,
        unsigned short msgtype, unsigned short waitmsgtype, unsigned char *bssid,
        unsigned char *stamac, void *data, int datalen);
struct mapd_radio_info * mapd_get_radio_from_channel(struct mapd_global *global, u8 channel);
void mapd_handle_traffic_stats(struct mapd_global *global,
				struct sta_traffic_stats *stats_arr);
#ifdef SUPPORT_MULTI_AP
int map_start_auto_role_detection_srv(struct own_1905_device *dev);
void mapd_send_steering_complete(struct mapd_global *global, struct mapd_bss *bss);
#ifdef CENT_STR
void mapd_cent_str_send_steering_complete(struct mapd_global *global,struct mapd_bss *bss);
#endif
#endif
void mapd_handle_stub(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len);
#ifdef SUPPORT_MULTI_AP
int mapd_set_enrollee_bh(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len);
int mapd_set_bss_role(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len);
int mapd_trigger_wps(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len);
#endif
int mapd_get_client_db(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len);

int newline_terminated(const char *buf, size_t buflen);
void skip_line_end(FILE *stream);
char * mapd_config_get_line(char *s, int size, FILE *stream, int *line,
				  char **_pos);
#ifdef SUPPORT_MULTI_AP
int mapd_Get_Bh_ConnectionStatus(struct own_1905_device *ctx, char *buf, size_t buf_Len);
int mapd_Get_Bh_ConnectionType(struct own_1905_device *ctx, char *buf, size_t buf_Len);
int  mapd_bh_steer(struct mapd_global *global, char *cmd_buf);
int mapd_mandate_steer(struct mapd_global *global, char *cmd_buf);
int mapd_Set_RssiTh(struct mapd_global *global, char *cmd_buf);
int mapd_trigger_ap_selection_bh(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len);
#endif
void Write_Steer_Status(char *status);
#ifdef SUPPORT_MULTI_AP
int mapd_Set_ChUtilTh(struct mapd_global *global, char *cmd_buf);
int mapd_send_config_renew(struct mapd_global *global);
void mapd_renew(struct mapd_global *global);
int mapd_Get_BH_interfaceAP(struct mapd_global *global,char *buf, size_t buf_Len);
int mapd_Get_FH_interfaceAP(struct mapd_global *global,char *buf, size_t buf_Len);
int mapd_set_bh_priority(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len);
/*DHCP_CTL*/
void map_dhcp_poll_timeout(void *eloop_ctx, void *timeout_ctx);
void map_register_dhcp_timer(struct own_1905_device *dev);
int mapd_get_bh_config(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len);
#endif
int mapd_reset_csbc(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len);
void os_sleep(os_time_t sec, os_time_t usec);
#ifdef SUPPORT_MULTI_AP
void map_update_device_role_as_controller(struct own_1905_device *dev);
void map_start_auto_role_detection(void *eloop_ctx, void *timeout_ctx);
#endif
void map_1905_poll_timeout(void *eloop_ctx, void *timeout_ctx);
#ifdef SUPPORT_MULTI_AP
int mapd_set_metric_policy_param (struct mapd_global *global, char *cmd_buf);

#ifdef MAP_R2
#define CH_SCAN_TIMEOUT 300
#define CH_SCAN_RETRIGGER_TIMEOUT 150
int trigger_metric_msg(struct mapd_global *global, char *cmd_buf);
int trigger_cac_msg (struct mapd_global *global, char *cmd_buf);
int trigger_ch_sel_msg (struct mapd_global *global, char *cmd_buf);
void Send_Failed_assoc_message(struct _1905_context *ctx, struct own_1905_device *own_dev, u8 *sta_mac_address,
		u16 assoc_sts_cd, u16 assoc_reason_code);
int mapd_get_de_stats(struct mapd_global *global, char *cmd_buf);
void insert_into_task_list(struct own_1905_device *own_dev,
	u8 task_type,
	struct monitor_ch_info *ch_info,
	struct channel_scan_req *scan_req,
	u8 *almac);
void handle_task_completion(
	struct own_1905_device *own_dev);
void find_and_remove_pending_task(
	struct own_1905_device *own_dev,
	u8 pending_task_type);
void own_dev_get_metric_info(
	struct mapd_global *global,
	struct radio_info_db *radio);
int map_cmd_ch_scan_req_demo( struct mapd_global *global, char *cmd_buf);
int map_cmd_ch_plan_R2_demo( struct mapd_global *global, char *cmd_buf);
int trigger_bh_sta_query (struct mapd_global *global, char *cmd_buf);
#endif

#endif
 Boolean is_chan_supported(u8 *known_channels, u8 channel);
int mapd_set_bh_switch_cu_en(struct mapd_global *global, char *cmd_buf);
int mapd_set_cu_maxcount_thresh(struct mapd_global *global, char *cmd_buf);
int mapd_set_bh_cu_forbidtime_thresh(struct mapd_global *global, char *cmd_buf);
void mapd_get_all_ap_metrics_info(struct mapd_global *global);
void mapd_reset_first_seen_for_all_dev(
	struct own_1905_device *ctx, u8 value);

#ifdef ACL_CTRL
int mapd_set_acl_ctrl(struct mapd_global *global, char *cmd_buf, char *buf, int buf_len);
int handle_acl_ctrl_msg(struct mapd_global *pGlobal_dev, struct acl_ctrl_tlv *acl_ctrl, struct _1905_map_device *p1905_device);
void map_sync_acl_info(void *eloop_ctx, void *timeout_ctx);
void mapd_acl_sync_new_agent_info(struct own_1905_device *ctx, struct _1905_map_device *p1905_device);
#endif /*ACL_CTRL*/
#endif
