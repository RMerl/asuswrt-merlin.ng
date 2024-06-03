#ifndef CLIENT_DB_H
#define CLIENT_DB_H

#include "list.h"
#include "steer_fsm.h"

#include "interface.h"
#include <sys/queue.h>

/* Blacklist reason */
#define BL_MAP_ASSOC_CONTROL BIT(0)
#define BL_STEER_ALGO BIT(1)

/* Capabilities */
/* HT/VHT Cap */

#define VHT_HT_CAPABLE 0 //Both VHT/HT cap
#define NON_VHT_CAPABLE 1 // Only HT
#define LEGACY_CLIENT 2 // Legacy
#define HE_VHT_HT_CAPABLE 3 // HE/VHT/HT cap
#define HE_HT_CAPABLE 4 // HE/VHT/HT cap

#define CAP_11K_SUPPORTED BIT(0)
#define CAP_11V_SUPPORTED BIT(1)
#define CAP_11R_SUPPORTED BIT(2)
#define CAP_MBO_SUPPORTED BIT(3)

/* Bands */
#define IEEE_NUM_BANDS 2
#define BAND_2G_IDX 0
#define BAND_5G_IDX 1
#define BAND_2G_SUPPORTED BIT(0)
#define BAND_5G_SUPPORTED BIT(1)

/* Element IDs */
#define WLAN_EID_SUPP_RATES 1
#define WLAN_EID_EXT_SUPP_RATES 50
#define WLAN_EID_SUPPORTED_CHANNELS 36
#define WLAN_EID_HT_CAP 45
#define WLAN_EID_MDE	54
#define WLAN_EID_VHT_CAP 191
#define WLAN_EID_MULTI_BAND 158
#define WLAN_EID_RRM_ENABLED_CAPABILITIES 70
#define WLAN_EID_EXT_CAPAB 127
#define IE_WLAN_EXTENSION 255
#define EID_EXT_HE_CAPS 35


/* IEEE80211 Channels */
#define MAX_NUM_CHANNELS_2G 14   /* 1 to 14 */
#define MAX_NUM_CHANNELS_5GL 8   /* 36 to 64 */
#define MAX_NUM_CHANNELS_5GH1 12 /* 100 to 144 */
#define MAX_NUM_CHANNELS_5GH2 5  /* 149 to 165 */
#define MAX_NUM_CHANNELS MAX_NUM_CHANNELS_2G + MAX_NUM_CHANNELS_5GL +\
    MAX_NUM_CHANNELS_5GH1 + MAX_NUM_CHANNELS_5GH2

/* Using bitmask for representing supported channel numbers */
#define MAX_CHAN_BITMAP		5
#ifdef MAP_R2
#define MAX_NOT_PREFER_CH_NUM 16
#endif
    

#ifndef MAX_NUM_OF_RADIO
#define MAX_NUM_OF_RADIO 6
#endif
#ifndef MAX_NUM_OF_BSS_PER_RADIO
#define MAX_NUM_OF_BSS_PER_RADIO 16
#endif
#ifndef MAX_NUM_BSS
#define MAX_NUM_BSS (MAX_NUM_OF_BSS_PER_RADIO * MAX_NUM_OF_RADIO)
#endif
#ifdef MAP_R2
#define MAX_TUNNEL_TYPE 5
#endif

/* Client is not present in persistent DB */
#define NOT_IN_DB 0
/* Client is already present or needs to be added in persistent DB */
#define IN_DB 1
/* Client is already present and needs to be added from persistent DB */
#define IN_DB_DEL 2
struct mapd_global;
struct mapd_bss;

enum max_bw {
	BW_20,
	BW_40,
	BW_80,
	BW_160,
	BW_10,
	BW_5,
	BW_8080
};

enum phy_mode {
	MODE_CCK,
	MODE_OFDM,
	MODE_HTMIX,
	MODE_HTGREENFIELD,
	MODE_VHT,
	MODE_HE
};

typedef enum coarse_phymode
{
	LEGACY_MODE,
	HT_MODE,
	VHT_MODE,
	HE_MODE
} coarse_phy_mode;

enum target_steering_type {
	NO_STEERING,
	FORCED_STEERING,
	BTM_IDLE_STEERING,
	BTM_ACTIVE_STEERING
};
enum steering_result {
	STEERING_NOT_ATTEMPTED,
	STEERING_FAILED,
	STEERING_SUCCESS
};

struct cli_rssi {
	struct dl_list rssi_entry;
	int8_t rssi;
	uint8_t channel;
	uint8_t bssid[ETH_ALEN];
};

struct num_he_sp_streams {
	uint8_t bw80_streams;
	uint8_t bw160_streams;
	uint8_t bw8080_streams;
};
struct phy_capab{
	uint8_t num_sp_streams;
	enum max_bw max_bw[IEEE_NUM_BANDS];
	enum coarse_phymode phy_mode[IEEE_NUM_BANDS];
	struct num_he_sp_streams num_he_spstr;
};

enum btm_csbc_state {
	CSBC_BTM_UNKNOWN,
	CSBC_BTM_DISALLOWED,
	CSBC_BTM_IDLE_ACTIVE_UNFRIENDLY,
	CSBC_BTM_IDLE_ACTIVE_FRIENDLY,
	CSBC_BTM_ACTIVE_ALLOWED
};

enum force_str_csbc_state {
	CSBC_FORCED_ALLOWED,
	CSBC_FORCED_DISALLOWED
};
enum association_request_format{
	ASSOC_REQ_NORMAL,
	ASSOC_REQ_REASSOC,
	ASSOC_REQ_AMBIGUOUS,
};
enum band_support{
	BAND_SUPPORT_AMBIGUOUS,
	BAND_SUPPORT_SINGLE,
	BAND_SUPPORT_DUAL,
};
struct csbc_data {
    uint8_t BCu;
    uint8_t BCi;
    uint8_t BCa;
    uint8_t consec_btm_act_fail_cnt;
    enum btm_csbc_state btm_state;
    enum force_str_csbc_state force_str_state;
};
#ifdef SUPPORT_MULTI_AP
struct coord_state_data {
	COORDINATION_STATE cli_coordination_state;
	u8 rfs_retries;
	u8 tsq_retries;
	u8 transaction_id;
	struct coord_req_dev_list *current_coord_dev;
	SLIST_HEAD(list_rfs_req, coord_req_dev_list) map_coord_dev_head;
	SLIST_HEAD(list_assoc_ctrl, _1905_map_device) map_1905_dev_assoc_control_head;
	u8 rfs_timer_running;
	u8 tsq_timer_running;
	COMPLETE_STATUS_CODE steer_complete_reason;

	u8 auto_clr_blacklist;
	struct os_reltime rfs_req_timestamp; /*time when rfs req was received. Need to clear blacklist after timeout*/
	struct os_reltime assoc_cntrl_req_timestamp; /*time when rfs req was received. Need to clear blacklist after timeout*/
};
#ifdef MAP_R2
struct tunneled_info {
	u8 tunnel_payload_len;
	u8 *tunneled_payload;
};
#endif
#endif
struct bss_air_mon_report
{
	u8 al_mac[ETH_ALEN];
	u8 bssid[ETH_ALEN];
	s8 rssi;
	u32 delta_time;
	u8 report_recieved;
	SLIST_ENTRY(bss_air_mon_report) air_mon_bss_entry;
};

struct meas_channel {
	u8 channel;
	u8 op_class;
};

struct meas_state_data {
	MEASUREMENT_STATE  cli_measurement_state;
	uint8_t meas_chan_cnt;
	struct meas_channel measurement_channels[MAX_NUM_CHANNELS];
	/* Number of 11k retries on single channel */
	uint8_t measurement_retries_11k;
	uint8_t curr_measurement_chan_idx;
	struct dl_list dl_rssi_list;

	u8 air_mon_timer_running;
	u8 air_mon_sent_cnt;
	u8 air_mon_rx_cnt;
	SLIST_HEAD(air_mon_list, bss_air_mon_report) air_mon_bss_head;
};

struct exec_mon_state_data {
	uint8_t target_bssid[ETH_ALEN];
	enum target_steering_type target_str;
};

struct remote_cli_data {
	uint8_t steer_type;
};

struct cli_steer_stats {
	/* Local Stats */
	uint16_t steer_attempts_f[MAX_NUM_STR_METHODS];
	uint16_t steer_attempts_btm[MAX_NUM_STR_METHODS];
	uint16_t steer_succ_cnt_f[MAX_NUM_STR_METHODS];
	uint16_t steer_fail_cnt_f[MAX_NUM_STR_METHODS];
	uint16_t steer_succ_cnt_btm[MAX_NUM_STR_METHODS];
	uint16_t steer_fail_cnt_btm[MAX_NUM_STR_METHODS];
	/* Cumulative Remote Stats */
	uint32_t rem_steer_succ_cnt_f;
	uint32_t rem_steer_fail_cnt_f;
	uint32_t rem_steer_succ_cnt_btm;
	uint32_t rem_steer_fail_cnt_btm;
	/* 11k Stats */
	uint32_t num_11k;
	uint32_t num_11k_succ;
#ifdef CENT_STR
uint16_t steer_band_steer_success_cnt;
uint16_t steer_band_steer_fail_cnt;
#endif	
	};

struct client {
	struct dl_list sta_seen_entry;
	struct dl_list assoc_sta_entry;
	struct dl_list unassoc_client_entry[MAX_NUM_OF_RADIO];
	uint8_t mac_addr[ETH_ALEN];
	uint32_t client_id; //-1 if not in use
	STEERING_STATE cli_steer_state;
	STEERING_METHOD_TYPE cli_steer_method;
	struct meas_state_data meas_data;
#ifdef SUPPORT_MULTI_AP
	struct coord_state_data coord_data;
#endif
	struct exec_mon_state_data exec_mon_data;
	EXEC_MONITOR_STATE cli_exec_mon_state;
	struct csbc_data csbc_data;
	uint8_t phy_cap_known[IEEE_NUM_BANDS]; //0: Unknown; 1: From PREQ; 2:From Assoc
	struct phy_capab phy_capab;
	struct remote_cli_data remote_data;
	uint8_t known_channels[MAX_CHAN_BITMAP];
	uint8_t chan_id_tries[MAX_NUM_OF_RADIO];
	struct os_reltime chan_id_trigger_ts;
	uint8_t known_bands;
	uint8_t current_chan;
	uint8_t bssid[ETH_ALEN];
	/* if the client is connected to a remote bssid */
	uint8_t is_remote;
	/* Track Activity State */
	uint8_t curr_activity_state;
	uint8_t activity_state;
	uint8_t consec_idle_count;
	/* Avg DL tp in Mbps */
	uint16_t dl_rate;
	/* Avg UL tp in Mbps */
	uint16_t ul_rate;
	/* Absoulute Tx packet Count */
	uint32_t tx_count;
	/* Absoulute Rx packet Count */
	uint32_t rx_count;
	uint32_t rx_success_pkt_cnt;
	uint32_t tx_success_pkt_cnt;
	uint32_t capab;
	uint8_t ht_vht_he_cap; //Used for pre-assoc
	uint16_t curr_air_time; //in percentage
	uint8_t auth_deny_count;
	uint8_t auth_deny_max;
	
	/* The TS at which this client was selected as a potential candidate for steering */
	struct os_reltime steer_cand_ts;
	/* The TS at which 11k on a channel failed consecutivey for MAX_11K_RETRIES */
	struct os_reltime failureTs11K;
	struct os_reltime null_frame_trigger_ts;

	uint8_t radio_idx; //Radio idx of the radio to which this client is connected
	int8_t ul_rssi[MAX_NUM_OF_RADIO];
	struct os_reltime rssi_ts[MAX_NUM_OF_RADIO];//TS at which RSSI on the conected BSS was updated
	uint32_t dl_phy_rate; //current DL Phy rate :
	struct cli_steer_stats steer_stats;
	uint8_t force_airmon;
	/* Tracks all the clients whose persistent attributes have chaged */
	uint8_t dirty;
	uint32_t str_mthds_failed_in_dec;
	u8 steer_retry_time;
	u8 steer_retry_step;
	/* 0: Not in DB; 1: In DB; 1: Marked for deletion */
	u8 in_db;
#ifdef MAP_R2
#ifdef SUPPORT_MULTI_AP
	struct tunneled_info tunnel_info[MAX_TUNNEL_TYPE];
#endif
	u8 np_channels[MAX_NOT_PREFER_CH_NUM];
	u8 np_pref;
	u8 np_reason;
#endif
	u8 rssi_based_rcpi;
#ifdef VENDOR1_FEATURE_EXTEND
	unsigned char vendor1_almac[ETH_ALEN];
	s8 vendor1_rssi_own;			//for vendor1 log - current ap downlink rssi from beacon response
	s8 vendor1_rssi_candidate;		//for vendor1 log - target ap downlink rssi from beacon response
#endif //VENDOR1_FEATURE_EXTEND
#ifdef CENT_STR
	u8 dual_band;
#endif
};

struct bl_client {
	struct client *cli;
	u32 bl_reason;
	struct dl_list list_entry;
	struct dl_list map_dev_list;
};

struct map_dev {
	u8 al_mac[ETH_ALEN];
	u32 duration;
	struct dl_list map_dev_entry;
};

#define STR_METHOD_FAILED_IN_DECISION(cli, method) ((cli->str_mthds_failed_in_dec & BIT(method)))

uint32_t client_db_get_cid_from_mac(struct mapd_global *global, u8 *mac_addr);
uint32_t client_db_track_add(struct mapd_global *global, u8 *mac_addr, u8 *already_seen); //Prakhar
void client_db_remove_from_assoc_list(struct mapd_global *global , uint32_t client_id);
Boolean client_db_is_sta_bl_on_bss(struct mapd_global *global,
                struct client *cli, struct mapd_bss *target_bss);
void client_db_init(struct client *cli_db);
void client_db_set_dl_phy_rate(struct mapd_global *global, uint32_t client_id,
            uint32_t dl_phy_rate);
uint32_t client_db_get_dl_phy_rate(struct mapd_global *global, uint32_t client_id);
void client_db_set_ul_rssi(struct mapd_global *global, uint32_t client_id,
        uint8_t ul_rssi, uint8_t radio_idx, Boolean is_preq);
int8_t client_db_get_ul_rssi(struct mapd_global *global, uint32_t client_id,
        uint8_t radio_idx);
void client_db_set_bssid(struct mapd_global *global, uint8_t client_id, u8 *bssid);
u8 * client_db_get_bssid(struct mapd_global *global, uint8_t client_id);
void client_db_set_curr_channel(struct mapd_global *global, uint8_t client_id, u8 channel);
void client_db_set_radio_idx(struct mapd_global *global, uint8_t client_id, u8 radio_idx);
u8 client_db_get_curr_channel(struct mapd_global *global, uint8_t client_id);
void client_db_clear_post_assoc_params(struct mapd_global *global, uint8_t client_id);
struct dl_list * client_db_get_assoc_list_entry(struct mapd_global *global,
		uint8_t client_id);
struct dl_list * client_db_get_bl_sta_per_bss_list_entry(struct mapd_global *global,
		uint8_t client_id, uint8_t bss_idx, uint8_t radio_idx);
struct dl_list * client_db_get_bl_sta_per_dev_list_entry(struct mapd_global *global,
        uint8_t client_id);
void client_db_update_from_ies(struct mapd_global *global, uint8_t client_id,
		const u8 *start, size_t len, uint8_t channel);
struct client *client_db_get_client_from_sta_mac(struct mapd_global *global,
		unsigned char *sta_mac);
struct client * client_db_get_client_from_client_id(struct mapd_global *global,
		uint8_t client_id);
STEERING_STATE client_db_get_cli_steer_state(struct mapd_global *global, uint8_t client_id);
void client_db_set_cli_steer_state(struct mapd_global *global, int client_id, STEERING_STATE next_state);
void client_db_set_known_channels(struct mapd_global *global, uint8_t client_id,
        uint8_t channel);
uint32_t client_db_add_client_to_db(struct mapd_global *global, u8 *mac_addr);
void client_db_update_cli_ht_vht_cap(struct mapd_global *global, uint32_t client_id, 
				const u8 *start, size_t len, uint8_t band_idx);
void client_db_set_phy_capab(struct mapd_global *global, uint32_t  client_id, uint8_t source,
                         uint8_t band_idx, enum max_bw bw, uint8_t nss, coarse_phy_mode phy_mode);
void client_db_set_capab(struct mapd_global *global, uint32_t client_id,
                         u8 bBtm, u8 bRrm, u8 bMbo);
void client_db_set_he_phy_capab(struct mapd_global *global, uint32_t  client_id, struct map_he_nss *nss_he);
#endif

