#ifndef _CH_PLANNING_H_
#define _CH_PLANNING_H_
#include "topologySrv.h"
#define TX_POWER_PERCENTAGE_TLV_LEN 6

#define BAND_24G					0
#define BAND_5G						1
#define MAX_BW_80_BLOCK 			6
#define MAX_BW_40_BLOCK 			12
enum off_ch_scan_status_code {
	SCAN_SUCCESS,
	OP_CLASS_CHAN_NOT_SUPP,
	REQ_TOO_SOON,
	RADIO_BUSY,
	SCAN_INCOMPLETE_LESS_TIME,
	ABORT_SCAN
};
struct GNU_PACKED tx_power_percentage_tlv {
	u8 tlv_type;
	u16 tlv_len;
	u8 mtk_oui[OUI_LEN];
	u8 func_type;
	u8 bandIdx;
	u8 tx_power_percentage;
};
void ch_planning_get_channel_block(unsigned char channel, unsigned char *channel_block, unsigned char op, int maxbw);


void ch_planning_timeout_handler(void * eloop_ctx, void *user_ctx);
void ch_planning_add_ch_to_prefered_list(
	struct own_1905_device *ctx,
	unsigned char channel, 
	struct radio_info_db *radio,
	unsigned char band,
	unsigned char preference,
	unsigned char reason);
void ch_planning_remove_ch_from_operating_list(
	struct own_1905_device *ctx,
	unsigned char channel, 
	struct radio_info_db *radio,
	unsigned char band);

void ch_planning_show_ch_distribution(
	struct own_1905_device *ctx);
void ch_planning_update_ch_ditribution(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct radio_info_db *radio,
	unsigned char channel,
	unsigned char op_class
);
void ch_planning_add_ch_to_prefered_list(
	struct own_1905_device *ctx,
	unsigned char channel, 
	struct radio_info_db *radio,
	unsigned char band,
	unsigned char preference,
	unsigned char reason);
void ch_planning_remove_ch_from_prefered_list(
	struct own_1905_device *ctx,
	unsigned char channel, 
	struct radio_info_db *radio,
	unsigned char band);
void ch_planning_timeout_handler(void * eloop_ctx, void *user_ctx);
void ch_planning_handle_operating_channel_report(
	struct own_1905_device *ctx,
	unsigned char *buff,
	int len);
void mapd_restart_channel_plannig(struct mapd_global *global);
int _1905_update_channel_pref_report(struct own_1905_device *ctx
	, struct cac_completion_report * report	
#ifdef DFS_CAC_R2
	, struct cac_status_report * status_report
#endif
	);
void mapd_perform_channel_planning(struct own_1905_device *ctx);
int ch_planning_set_user_preff_ch(struct mapd_global *global,
	u8 channel);
void ch_planning_exec(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned int channel[]);
void ch_planning_set_txpower_percentage(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char band,
	unsigned char txpower_percentage);
void ch_planning_send_txpower_percentage_msg(
	struct own_1905_device *ctx,
	unsigned char *al_mac_addr,
	unsigned char bandIdx,
	unsigned char txpower_percentage);
void ch_planning_handle_tx_power_percentage_msg(
        struct mapd_global *pGlobal_dev,
        struct tx_power_percentage_tlv *txpower_percent_tlv);
unsigned char ch_planning_get_centre_freq_ch(unsigned char channel, unsigned char op);
unsigned int ch_planning_check_channel_operable(struct own_1905_device *ctx,
	unsigned char channel, int maxbw);
#ifdef WIFI_MD_COEX_SUPPORT
unsigned int ch_planning_check_channel_operable_for_dev(struct _1905_map_device *dev,
	unsigned char channel);
#endif
int off_ch_scan_exec(struct own_1905_device *ctx,
					char *buf,
					unsigned char *reply, unsigned char bandwidth);
void dump_net_opt_off_ch_scan_rep(struct net_opt_scan_report_event *scan_rep_evt);
void dump_off_ch_scan_rep(struct off_ch_scan_report_event *scan_rep_evt);

#ifdef SUPPORT_MULTI_AP
void ch_planning_update_ch_score(
	struct own_1905_device *ctx,
	struct ch_distribution_cb *ch_distribution);
#endif
unsigned int ch_planning_check_channel_operable_wrapper(struct own_1905_device *ctx,
	unsigned char channel);
#ifdef WIFI_MD_COEX_SUPPORT
unsigned int ch_planning_check_channel_for_dev_operable_wrapper(struct _1905_map_device *dev,
	unsigned char channel);
#endif
void mapd_fill_secondary_channels(unsigned char *channel,
	unsigned char op_class, unsigned char bw);
void mapd_fill_secondary_channels_for_1905_dev(struct own_1905_device *ctx,
	struct _1905_map_device *dev);
Boolean ch_planning_is_ch_dfs(
	struct own_1905_device *own_dev,u8 channel);
void trigger_net_opt(void *eloop_ctx, void *timeout_ctx);
void start_netopt_timer(
	struct own_1905_device *own_dev,
	u8 channel);
unsigned char get_primary_channel(
       unsigned char channel);
u8 ch_planning_find_max_pref_index(
	struct own_1905_device *ctx,
	struct ch_prefer_lib *ch_prefer);
void ch_planning_trigger_net_opt_post_ch_plan(
	struct own_1905_device *ctx);
u8 is_mixed_network(
	struct own_1905_device *ctx,
	Boolean ignore_edcca);

#ifdef MAP_R2
void ch_planning_update_state_scan_done(
	struct own_1905_device *own_dev,
	struct _1905_map_device *dev,
	struct radio_info_db *radio);
void ch_planning_handle_metric_report(
	struct own_1905_device * ctx,
	struct _1905_map_device *dev,
	struct bss_info_db *bss,
	struct radio_info_db *radio,
	u8 cu_tlv_update,
	u8 force_monitor);
void ch_planning_remove_all_scan_results(
	struct _1905_map_device *dev);
void ch_planning_remove_radio_scan_results(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev);
void ch_planning_exec_R2(
        struct own_1905_device *ctx,
        struct _1905_map_device *dev,
        unsigned int channel[]);

void ch_planning_handle_ch_scan_rep(struct mapd_global *global);


void ch_scan_req_timeout(void * eloop_ctx, void *user_ctx);
#define POSITIVE(n) ((n) < 0 ? 0 - (n) : (n))
#define MAX_BSS_NUM_2G 255//8 //Rayden mail 19Feb , do not filter for 2.4G based on neighbour num
#define MAX_BSS_NUM_5G 45
#define BSS_OVERHEAD_5G 56  //0.56
#define BSS_OVERHEAD_2G 322 //3.22
#define FILTER_OUT 1
#define FILTER_IN 0
#define SNR_FILTER_THRESHOLD 7
#define EDCCA_FILTER_THRESHOLD 50

void ch_planning_send_scan_req(
	struct own_1905_device *ctx,
	struct _1905_map_device *_1905_device,
	struct radio_info_db *radio);
#ifdef SUPPORT_MULTI_AP
void ch_planning_select_best_channel_R2(
	struct mapd_global *global,
	struct monitor_ch_info *ch_info);
#endif
void channel_monitor_timeout(void *eloop_ctx, void *timeout_ctx);
struct _1905_map_device * ch_planning_find_agent_for_CAC(
	struct mapd_global *global,
	unsigned char channel, unsigned int *cac_method);
void ch_planning_trigger_cac_msg (
	struct mapd_global *global,
	struct _1905_map_device *_1905_dev,
	unsigned char channel, unsigned int cac_method) ;
void ch_planning_ch_change_trigger(
	struct mapd_global *global,
	unsigned char channel);

void ch_planning_R2_reset(
	struct own_1905_device * ctx,
	struct radio_info_db *reset_radio);
Boolean ch_planning_all_dev_select_done(
	struct own_1905_device *own_dev,
	struct radio_info_db *radio);
void ch_planning_R2_init(struct own_1905_device * ctx);
int get_default_radio_policy(
	struct own_1905_device *own_dev,
	struct radio_info_db *radio,
	struct lib_steer_radio_policy *radio_policy);
void ch_planning_own_dev_get_metric_timeout(
	void *eloop_ctx, void *timeout_ctx);
void ch_planning_remove_scan_list(
	struct own_1905_device *own_dev);
#ifdef SUPPORT_MULTI_AP
void ch_planning_update_ap_metric_policy(
	struct own_1905_device *ctx,
	struct monitor_ch_info *monitor_ch,
	u8 metric_policy_interval);
#endif
void dump_ch_planning_info(struct own_1905_device *own_dev, u8 get_scan_results);
void ch_planning_handle_controller_scan_result(
	struct own_1905_device *ctx,
	struct net_opt_scan_report_event *scan_rep_evt);
void ch_planning_handle_ch_selection_rsp(
	struct own_1905_device *own_dev,struct _1905_map_device *peer_1905);
void ch_planning_update_all_dev_state(
	u8 state,
	u8 channel,
	struct own_1905_device *own_dev);
u8 ch_planning_get_ch_rank(
	struct own_1905_device *own_dev,
	u8 channel);
void ch_planning_R2_force_trigger(
	struct mapd_global *global,
	u8 channel);
void ch_planning_R2_bootup_handling(
	struct own_1905_device *ctx);
u8 ch_planning_is_MAP_net_idle(
	struct own_1905_device * ctx);
void ch_planning_send_select(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char ch_prefer_count,
	struct ch_prefer_lib *ch_prefer);
void ch_planning_trigger_cac_msg2 (
	struct mapd_global *global,
	struct _1905_map_device *_1905_dev,
	u8 channel, unsigned int cac_method);
void ch_planning_handle_cac_response2(
	struct own_1905_device *own_dev,
	struct _1905_map_device *dev,
	struct radio_info_db *radio);
u8 ch_planning_check_dfs(
	struct mapd_global *global,
	u8 channel);
int check_ongoing_CAC(
	struct own_1905_device *ctx,
	struct radio_info_db *radio);
u8 is_CAC_Success(
	struct mapd_global *global,
	u8 channel);
void ch_planning_R2_send_select_dfs(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char ch_prefer_count,
	struct ch_prefer_lib *ch_prefer,
	u8 channel);
void ch_planning_reset_user_preff_ch(struct mapd_global *global);
void ch_planning_restore_policy(
	struct own_1905_device * ctx,
	struct radio_info_db *reset_radio);

u8 ch_planning_check_controller_cac_cap(
	struct own_1905_device *own_dev,
	u8 channel,
	u8 cac_mode);
void ch_planning_trigger_cac_on_cont(struct mapd_global *global, u8 channel);
void ch_planning_handle_cac_failure(struct mapd_global * global, struct radio_info_db *radio,
							struct cac_completion_report * report, struct cac_status_report * status_report);
void ch_planning_handle_cac_success_for_cont(struct mapd_global * global, struct radio_info_db *radio,
							struct cac_completion_report * report);
u8 ch_planning_get_grp_rank(
	struct own_1905_device *own_dev,
	u8 channel);
u16 ch_planning_calc_weight( u8 band, u16 neighbor_num);
u16 ch_planning_calc_OBSS(struct own_1905_device *own_dev,
	struct _1905_map_device *_1905_dev,
	struct radio_info_db *radio,
	struct scan_result_tlv *scan_res);
void ch_planning_calc_score(
	struct own_1905_device *own_dev,
	struct _1905_map_device *_1905_dev,
	struct radio_info_db *radio,
	struct scan_result_tlv *scan_res);
void ch_scan_timeout(void *eloop_ctx, void *timeout_ctx);
void ch_planning_R2_bootup_handling_restart(void * eloop_ctx,void * timeout_ctx);
void ch_planning_scan_restart_due_to_failure(struct mapd_global *global);
u8 ch_planning_check_scan_result(struct own_1905_device *own_dev,
	struct monitor_ch_info *ch_info);

#define CHANNEL_MONITOR_PROHIBIT_TIME 900 //15 min
#define CHANNEL_MONITOR_TIMEOUT 300
#define CHANNEL_CAC_TIMEOUT 120
#define RADAR_DETECTED 0x01
#define CAC_SUCCESSFUL 0x00
#define CAC_FAILURE 0x05
typedef enum cac_mode 
{
	CONTINUOUS,
	DEDICATED_RADIO,
	REDUCED_MIMO,
} CAC_MODE;

#define MIN_SCORE_INCREMENT_DEFAULT 10
#define DEFAULT_METRIC_REPORTING_INTERVAL 10

//sonal test Todo need to review threshold values during rdut
#define CH_PLAN_DEFAULT_CH_UTIL_TH_2G   70
#define CH_PLAN_DEFAULT_EDCCA_TH_2G     35
#define CH_PLAN_DEFAULT_OBSS_TH_2G      60

#define CH_PLAN_DEFAULT_CH_UTIL_TH_5G   80
#define CH_PLAN_DEFAULT_EDCCA_TH_5G     35
#define CH_PLAN_DEFAULT_OBSS_TH_5G      60

#define EDCCA_THRESHOLD 35
#define OBSS_THRESHOLD 60
#define BKLOAD_THRESHOLD 65
#define MIN_SAMPLE_COUNT 1
typedef enum trigger_status
{
	TRIGGER_FALSE,
	TRIGGER_TRUE,
	TRIGGER_PENDING,
} TRIGGER_STATUS;

#define NON_PREF 0
#endif
int get_net_opt_dev_count(struct mapd_global *pGlobal_dev);
unsigned int is_valid_primary_ch_80M_160M(unsigned char ch, unsigned char center_ch, unsigned char op);
#define OP_DISALLOWED_DUE_TO_DFS (BIT(0)|BIT(1)|BIT(2))
int ch_planning_get_dev_bw_from_channel(
	struct _1905_map_device *_1905_dev,
	u8 channel);
#endif
