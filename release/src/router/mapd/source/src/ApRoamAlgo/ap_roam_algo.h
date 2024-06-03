void ap_roam_algo_select_steer_candidate(struct mapd_global *global, struct client *arr_cand_list[MAX_STA_SEEN], int *cand_cnt);
void ap_roam_algo_find_candidate_channels_single(struct mapd_global *global, uint32_t client_id);
void ap_roam_algo_determine_sap_bss(struct mapd_global *global, uint32_t client_id);
void ap_roam_algo_chk_preassoc_str(struct mapd_global *global, struct client *client_info);
#ifdef SUPPORT_MULTI_AP
void ap_roam_algo_find_measurement_channel_multi(struct mapd_global *pGlobal_dev,
				uint32_t client_id);
#endif
void ap_roam_algo_get_ch_ol_safety_th(struct own_1905_device *own_device, u8 channel,
		u8 *ol_th, u8 *ch_safety_th);
int ap_roam_algo_disable_post_assoc_strng(struct mapd_global *global, u8 disable);
int ap_roam_algo_disable_pre_assoc_strng(struct mapd_global *global, u8 disable);
STEERING_STATE ap_roam_algo_handle_chan_meas_complete(struct mapd_global *global,
                struct client *cli, int *status_code);
int ap_roam_algo_disable_post_assoc_strng_by_type(struct mapd_global *global,
                u8 disable, STEERING_METHOD_TYPE str_method);
u8 ap_roam_algo_get_bss_channel(
#ifdef SUPPORT_MULTI_AP
		struct bss_info_db * bss,
#else
		struct mapd_bss * bss,
#endif
		struct client *map_client);
void ap_roam_algo_determine_sap_bss_cent_str(struct mapd_global *global, uint32_t client_id);

#ifdef VENDOR1_FEATURE_EXTEND
unsigned int vendor1_calc_steer_action_prohibit_time(struct mapd_global *global,	struct client *cli,
	unsigned int prohibit_time, const char* pszFuncName);
#endif //VENDOR1_FEATURE_EXTEND