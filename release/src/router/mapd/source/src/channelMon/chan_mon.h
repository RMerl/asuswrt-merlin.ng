#include "mapd_i.h"
#include "topologySrv.h"

#ifndef CHAN_MON_H
#define CHAN_MON_H

#define OUI_LEN		3

#define CLI_CAP_11K (BIT(0))
#define CLI_CAP_11V (BIT(1))
#define CLI_CAP_MBO (BIT(2))


#define STA_INFO_REQ_TLV_LEN		10
#define STA_INFO_RSP_TLV_LEN		11
#ifdef SUPPORT_MULTI_AP
#define FUNC_STA_INFO_REQ 	1
#define FUNC_STA_INFO_RSP	2

#define STEER_OPP_MIN_TIME 20
#define STEER_MANDATE 1
#define STEER_OPP 0

#define MIN_2G_CH 1
#define MAX_2G_CH 14

enum steer_opp_allow {
	STEER_OPP_INVALID,
	STEER_OPP_VALID,
	STEER_OPP_TIME_INSUFF,
	STEER_OPP_TIME_EXPIRE
};
#endif
enum op_bw{
	BW20,
	BW40PLUS,
	BW40MINUS,
	BW80,
	BW2160,
	BW160,
	BW80P80
};

struct oper_class_map {
    u8 op_class;
    u8 min_chan;
    u8 max_chan;
    u8 inc;
    enum op_bw bw;
};
#ifdef SUPPORT_MULTI_AP
struct sta_info_req_tlv
{
	u8 tlv_type;
	u16 tlv_len;
	u8 mtk_oui[OUI_LEN];
	u8 func_type;
	u8 cli_mac[ETH_ALEN];
};

struct sta_info_rsp_tlv
{
	u8 tlv_type;
	u16 tlv_len;
	u8 mtk_oui[OUI_LEN];
	u8 func_type;
	u8 cli_mac[ETH_ALEN];
	u8 band_standard_cap; /* BIT0: 2.4G, BIT1: 5G, BIT2: 11k, BIT3: 11v, BIT4: MBO*/
};
#endif

#if 0
struct bss
{
	struct dl_list list_node;
	char bssid;
	int channel;
	struct bss_steer_policy steer_policy;
	struct dl_list assoc_client_list_head;
/*Steer Req*/
	Boolean steer_window_valid;
	struct lib_steer_request steer_req_msg;
	Boolean b_steer_triggered; // to be set when steering was triggered.
	u32 steer_req_timestamp; // time when steer req was received.
};
#endif

#if 1
#if 0
/**
  * @target_bssid: indicates a target BSSID for steering. wildcard BSSID is represented by ff:ff:ff:ff:ff:ff
  * @op_class: target BSS operating class
  * @channel: target BSS channel number for channel on which the target BSS is transmitting beacon frames  
  * @sta_mac: sta mac address for which the steering request applies. if sta_count of struct steer_request
  * is 0, then this field is invalid
  */
struct GNU_PACKED target_bssid_info {
	unsigned char target_bssid[ETH_ALEN];
	unsigned char op_class;
	unsigned char channel;
	unsigned char sta_mac[ETH_ALEN];
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
struct GNU_PACKED map_steer_request {
	unsigned char assoc_bssid[ETH_ALEN];
	unsigned char request_mode;
	unsigned char btm_disassoc_immi;
	unsigned char btm_abridged;
	unsigned short steer_window;
	unsigned short btm_disassoc_timer;
	unsigned char sta_count;
	unsigned char target_bssid_count;
	struct target_bssid_info info[0];
};

#endif

#ifdef SUPPORT_MULTI_AP
struct GNU_PACKED map_lib_steer_request {
	unsigned char bssid[ETH_ALEN];
	unsigned char mode;
	unsigned short window;
	unsigned short timer;
	unsigned char sta_cnt;
    unsigned char sta_list[0];
};


struct GNU_PACKED map_lib_target_bssid_info {
	unsigned char tbss_cnt;
    struct lib_target_bssid_info bss_info[0];
};

#ifdef MAP_R2

struct GNU_PACKED map_lib_target_bssid_info_R2 {
	unsigned char tbss_cnt;
    struct lib_target_bssid_info_R2 bss_info[0];
};

#endif

#endif
#endif /* #if 1 */




#if 0
struct own_1905_device
{
	char al_mac[MAC_ADDR_LEN];
	struct controller controller_context;
	struct agent agent_context;
	struct dl_list bss_list_head; 
	struct unassoc_clients[MAX_NUM_CHANNELS]
	struct dl_list Blacklist_sta_list;
	struct steer_params cli_steer_params;
	struct 1905_map_device *Nodes_pointers[MAX_NODES]; // topo db
	struct client assoc_client_db[MAX_STA_NUM];
	Dl_list unassoc_client_db_head;
	dl_list steer_action_cli_list; //list of clients whose steering is ongoing == STEER_EXEC_MON
	struct _1905_context *_1905_ctx;
	UINT8 CUAvgPeriod;
};
#endif
void chan_mon_set_util(struct mapd_global *global, uint8_t radio_idx,
			uint8_t ch_util);
uint8_t chan_mon_get_util(struct mapd_global *global, u8 radio_idx);
uint8_t chan_mon_get_ol_th(struct mapd_global *global, u8 radio_idx);
#ifdef SUPPORT_MULTI_AP
void chan_mon_rr_trigger_handler(struct mapd_global *pGlobal_dev);
int chan_mon_handle_steering_req(struct mapd_global *pGlobal_dev, u8 *buf, size_t len);
#endif
void chan_mon_set_util(struct mapd_global *global, uint8_t radio_idx,
			uint8_t ch_util);
uint8_t chan_mon_get_util(struct mapd_global *global, u8 radio_idx);
uint8_t chan_mon_get_ol_th(struct mapd_global *global, u8 radio_idx);
int chan_mon_get_bw_from_op_class(u8 op_class);
#ifdef SUPPORT_MULTI_AP
void chan_mon_check_steer_triggered(struct mapd_global *global, u8 *buf, u8 len);
//Boolean chan_mon_is_radio_ol(struct mapd_global *global, struct mapd_radio_info *ra_info);
void mapd_update_controller_steer_policy(struct mapd_global *pGlobal_dev);
void steer_msg_update_policy_config(struct mapd_global *pGlobal_dev, struct _1905_map_device *_1905_device);
void chan_mon_handle_steer_complete(struct own_1905_device *own_device, struct _1905_map_device *_1905_device);
void steer_msg_handle_vendor_specific_msg(struct mapd_global *pGlobal_dev,
			struct _1905_map_device *_1905_device, struct tlv_head *tlv);
void chan_mon_update_rr_ctrl_trigger_info(struct mapd_global *pGlobal_dev);
#endif
unsigned char is_channel_in_opclass(unsigned char operclass, unsigned char bandwidth, unsigned char channel);
int chan_mon_get_op_class_frm_channel(u8 channel, u8 bandwidth);
int chan_mon_get_20m_op_class_frm_channel(u8 channel);
#ifdef SUPPORT_MULTI_AP
void update_client_coordination_state_for_assoc_control(struct mapd_global *global, struct cli_assoc_control *assoc_cntrl, u8 len);

void chan_mon_fill_steer_req_data(struct mapd_bss *curr_own_bss,
		struct lib_steer_request *steer_req_msg,
#ifdef MAP_R2
		struct map_lib_target_bssid_info *bss_info,
		struct lib_steer_request_R2 *steer_req_msg_r2,
		struct map_lib_target_bssid_info_R2 *bss_info_r2
#else
		struct map_lib_target_bssid_info *bss_info
#endif
		);
#endif
u8 chan_mon_get_safety_th(struct mapd_global *global, u8 radio_idx);
Boolean chan_mon_is_radio_ol(struct mapd_global *global, struct mapd_radio_info *ra_info);

#ifdef CENT_STR
Boolean chan_mon_is_1905_radio_safety_ol(struct mapd_global *global,struct radio_info_db *target_radio);
Boolean chan_mon_is_1905_radio_ol(struct mapd_global *global, struct radio_info_db *target_radio);
u8 chan_mon_get_ol_th_by_chan(struct mapd_global *global, u8 channel);
u8 chan_mon_get_safety_th_by_chan(struct mapd_global *global, u8 channel);
void chan_mon_create_steer_req_mandate(
	char sta_cnt,
	char *sta_mac_list,
	char bss_cnt,
	char *cli_bssid,
	struct bss_info_db map_bss[],
	struct lib_steer_request **steer_req_msg,
#ifdef MAP_R2
	struct map_lib_target_bssid_info **bss_info,
	struct lib_steer_request_R2 **steer_req_msg_r2,
	struct map_lib_target_bssid_info_R2 **bss_info_r2,
	char steer_reason_code
#else
	struct map_lib_target_bssid_info **bss_info
#endif

	);

#endif
#endif
