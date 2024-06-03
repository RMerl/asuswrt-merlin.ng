#ifndef AP_CENT_STR_H
#define AP_CENT_STR_H

#ifdef SUPPORT_MULTI_AP
#ifdef CENT_STR
#define MAX_CENT_STEER_CAND 5
#define MAX_BS_FAIL 	5
#define MAX_OL_STEER_CAND 1
#define MAX_UG_STEER_CAND 1
#define MAX_CU_MONITOR_TIME 30
#define MAX_CU_MONITOR_PROHIBIT_TIME 60


struct cent_str_cu_mon_ch_info
{
	u8 channel_num;
	struct radio_info_db *radio;
	SLIST_ENTRY(cent_str_cu_mon_ch_info) next_mon_ch;
};


void client_mon_chk_post_assoc_cent_str(struct mapd_global *global);
void cent_str_select_on_demand_str_method(struct own_1905_device *ctx,struct client *cli);
void cent_str_rr_steer_cand_selection(void *eloop_ctx, void *timeout_ctx);
Boolean cent_str_bs_max_cnt_reached(struct client *cli, struct mapd_global *global);
Boolean is_chplan_netopt_ongoing(struct mapd_global *global);
void steer_handle_chan_plan_net_opt_trigger(struct mapd_global *global);
void cent_str_cu_monitor(struct own_1905_device * ctx,
	struct bss_info_db *bss);
void cent_str_cu_monitor_timeout(void *eloop_ctx, void *timeout_ctx);
void cent_str_cu_mon_remove_chan_list(
	struct own_1905_device *own_dev);

#endif /*CENT_STEER*/
#endif /*SUPPORT_MULTI_AP*/
#endif
