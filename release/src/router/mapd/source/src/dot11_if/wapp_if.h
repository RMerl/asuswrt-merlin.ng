#ifndef wapp_if_H
#define wapp_if_H
typedef enum measurement_mode 
{
	PASSIVE,
	ACTIVE,
	BEACON_TABLE

} MEASUREMENT_MODE;


struct event_wrapper_s {
	int from;
	void * event;
};

struct wapp_usr_intf_ctrl *wapp_ctrl;
void wapp_close_connection(void);
int wapp_open_connection(const char *ctrl_path, struct mapd_global *global);
void wlanif_get_op_chan_info(struct mapd_global *global);
void wlanif_get_op_bss_info(struct mapd_global *global, unsigned char *identifier);
void wlanif_get_all_assoc_sta_link_metrics(struct mapd_global *global, unsigned char *identifier);
void wlanif_get_assoc_sta_traffic_stats(struct mapd_global *global, unsigned char *identifier);
int  wlanif_get_ap_metrics_info(struct mapd_global *global, u8 *bssid);
void wlanif_beacon_metrics_query(struct mapd_global *global, u8 *sta_mac,
                u8 *assoc_bssid, u8 ssid_len, u8 *ssid,
                u8 channel, u8 op_class, u8 *bssid,
                u8 rpt_detail, u8 num_elem, char *elem_list,
                u8 num_chrep, struct ap_chn_rpt *chan_rpt);
void wlanif_get_ap_cap(struct mapd_global *global, unsigned char *identifier);
int wlanif_issue_wapp_command(struct mapd_global *global, int msgtype,
		int waitmsgtype, unsigned char *bssid, unsigned char *stamac,
		void *data, int datalen, int from, int resp_expected, int cmd_role);
void wlanif_trigger_btm_req(struct mapd_global *global, u8 *mac_addr,
        u8 *curr_bssid, u8 *target_bssid, u8 target_op_chan, u8 target_op_class,
        u8 disassoc_imm, u8 btm_abridged, u16 btm_disassoc_timer);
void wlanif_process_wapp_events(struct mapd_global *global, char *buf, int length);
int wlanif_issue_wapp_command(struct mapd_global *global, int msgtype,
        int waitmsgtype, unsigned char *bssid, unsigned char *stamac,
        void *data, int datalen, int from, int resp_expected, int cmd_role);
int wlanif_bl_sta_for_bss(struct mapd_global *global, u8 *mac_addr,
        u8 *bssid, Boolean blacklist);
#ifdef ACL_CTRL
int wlanif_acl_ctrl_for_bss(struct mapd_global *global, u8 *sta_addr, u8 *bssid, u8 cmd);
#endif
void wlanif_register_wapp_events(struct mapd_global *global);
int wlanif_deauth_sta(struct mapd_global *global, u8 *mac_addr, u8 *bssid);
void wlanif_get_op_chan_info(struct mapd_global *global);
void wlanif_get_all_assoc_sta_tp_metrics(struct mapd_global *global, u8 *radio_id);
int wlanif_disconnect_apcli(struct mapd_global *global, unsigned char *intfname);
int wlanif_flush_bl_for_bss(struct mapd_global *global, u8 *bssid);
void wlanif_trigger_null_frames(struct mapd_global *global, u8 *mac_addr,
				u8 *bssid, u8 count);
int wapp_open_reconnection(struct mapd_global *global);
#ifdef SUPPORT_MULTI_AP
void topo_srv_parse_wapp_ap_metric_event(struct mapd_global * global,
	struct ap_metrics_info *minfo, unsigned short from);
void topo_srv_parse_wapp_all_sta_traffic_stats(struct mapd_global * global,
	struct sta_traffic_stats *stats, unsigned short from);
void topo_srv_parse_wapp_all_assoc_link_metric(struct mapd_global * global,
	struct sta_link_metrics *metrics, unsigned short from);
void topo_srv_parse_wapp_one_assoc_link_metric(struct mapd_global * global,
	struct link_metrics *metrics, unsigned short from);
void topo_srv_parse_wapp_unassoc_link_metric(struct mapd_global * global,
	struct unlink_metrics_rsp *unlink_metrics);
void topo_srv_parse_wapp_air_monitor_report(struct mapd_global * global,
	struct unlink_metrics_rsp *unlink_metrics);
void topo_srv_parse_wapp_cli_steer_btm_report(struct mapd_global * global,
	struct cli_steer_btm_event *evt);
void topo_srv_parse_wapp_cli_steering_completed(struct mapd_global * global,
	struct cli_steer_btm_event *evt);
void topo_srv_parse_wapp_read_bss_conf_request(struct mapd_global * global,
	char *file_path, unsigned int len);
void topo_srv_parse_wapp_operating_channel_report(struct mapd_global * global,
	unsigned char *buf);
void topo_srv_parse_wapp_beacon_metrics_report(struct mapd_global * global,
	struct beacon_metrics_rsp *evt);
void topo_srv_parse_wapp_client_notification(struct mapd_global * global,
	struct client_association_event_local *evt);
void topo_srv_parse_wapp_bh_ready(struct mapd_global * global,
	struct bh_link_info *bh_info);
void topo_srv_parse_wapp_1905_cmdu_request(struct mapd_global * global,
	struct _1905_cmdu_request *request);
void topo_srv_parse_wapp_radio_basic_cap(struct mapd_global * global,
	struct ap_radio_basic_cap *bcap);
void topo_srv_parse_wapp_radio_operation_restriction(struct mapd_global * global,
	struct restriction *restrict_var);
void topo_srv_parse_wapp_ht_capability(struct mapd_global * global,
	struct ap_ht_capability *cap);
void topo_srv_parse_wapp_vht_capability(struct mapd_global * global,
	struct ap_vht_capability *cap);
void topo_srv_parse_wapp_he_capability(struct mapd_global * global,
	struct ap_he_capability *cap);
void topo_srv_parse_wapp_channel_preferrence(struct mapd_global * global,
	struct ch_prefer *prefer);
void topo_srv_parse_wapp_bh_steer_resp(struct mapd_global * global,
	struct backhaul_steer_rsp *steer_info);
void topo_srv_parse_wapp_operating_channel_info(struct mapd_global * global,
	struct channel_report *chan_report);
void topo_srv_parse_wapp_ap_capability(struct mapd_global * global,
	struct ap_capability *ap_cap);
void topo_srv_parse_wapp_oper_bss_report(struct mapd_global * global,
	struct oper_bss_cap *oper_bss);
void topo_srv_parse_wapp_scan_result(struct mapd_global * global,
	struct wapp_scan_info *scan_results);
void topo_srv_parse_wapp_scan_done(struct mapd_global * global);
void topo_srv_parse_wapp_vend_ie_changed(struct mapd_global * global,
	struct map_vendor_ie *vendor_ie);
void topo_srv_parse_wapp_get_wsc_config(struct mapd_global * global,
	struct wps_get_config *wps_config);
int topo_srv_parse_wapp_ap_link_metirc_request(struct mapd_global * global,
	unsigned char *target_bssid);
void topo_srv_parse_wapp_assoc_state_changed(struct mapd_global * global,
	struct wapp_apcli_association_info *cli_assoc_info);
void topo_srv_parse_wapp_1905_read_tlv_req(struct mapd_global * global,
	char *tlv, int tlv_length);
void topo_srv_parse_wapp_device_status(struct mapd_global * global,
	wapp_device_status *device_status);
void topo_srv_parse_wapp_off_channel_scan_report(struct mapd_global * global,
	struct off_ch_scan_report_event *scan_rep_evt);
void topo_srv_parse_wapp_net_opt_scan_report(struct mapd_global * global,
	struct net_opt_scan_report_event *scan_rep_evt);
void dump_net_opt_off_ch_scan_rep(struct net_opt_scan_report_event *scan_rep_evt);
int find_bhlink_bw_ch(struct own_1905_device *ctx,struct radio_info_db *radio,struct bh_link_entry *bh_entry, struct channel_bw_info *ch_bw_info);
unsigned char ch_planning_get_centre_freq_ch(unsigned char channel, unsigned char op);
int wapp_get_all_wifi_interface_status(struct mapd_global *global);
void topo_srv_parse_wapp_cac_completion_report_wrapper(void *eloop_ctx, void *timeout_ctx);
#ifdef MAP_R2
void wlanif_get_all_assoc_sta_ext_link_metrics(struct mapd_global *global, u8 *radio_id);

void topo_srv_parse_wapp_radio_metric_event(struct mapd_global * global,
	struct radio_metrics_info *minfo, unsigned short from);


void topo_srv_parse_wapp_all_sta_extended_link_metrics(struct mapd_global * global,
	struct ext_sta_link_metrics *metrics);


void topo_srv_parse_wapp_one_sta_extended_link_metrics(struct mapd_global * global,
	struct ext_link_metrics *metrics);



void topo_srv_parse_wapp_dissassoc_stats(struct mapd_global * global,
	struct client_disassociation_stats_event *evt, unsigned short len, unsigned short from);


//WAPP_SCAN_CAPAB
void topo_srv_parse_wapp_scan_capab(struct mapd_global * global,
	struct channel_scan_capab *scan_cap, unsigned short cap_len);



void topo_srv_parse_wapp_r2_ap_cap(struct mapd_global * global,
	struct ap_r2_capability *r2_ap_capab, unsigned short cap_len);


/*WAPP_CHANNEL_SCAN_REPORT*/


void dump_ch_scan_rep_r2(struct channel_scan_report_event *scan_rep_evt);


void topo_srv_parse_wapp_ch_scan_report(struct mapd_global * global,
	struct net_opt_scan_report_event *scan_rep_evt, unsigned short rep_len);


/*WAPP_ASSOC_STATUS_NOTIFICATION*/
void topo_srv_parse_wapp_assoc_status_notif(struct mapd_global * global,
	struct assoc_notification *assoc);


/*WAPP_TUNNELED_MESSAGE*/
void topo_srv_parse_wapp_tunneled_msg(struct mapd_global * global,
	struct tunneled_msg * tunneled);


#ifdef DFS_CAC_R2
/*WAPP_CAC_CAPAB*/
void topo_srv_parse_wapp_cac_capab(struct mapd_global * global,
	struct cac_capability *cac_cap, unsigned short cap_len);

#endif
#endif
void topo_srv_parse_wapp_cac_completion_report(struct mapd_global * global,
	struct msg * wapp_event, unsigned short rep_len);
void topo_srv_parse_wapp_cac_periodic_enable(struct mapd_global * global,
	struct msg *wapp_event, struct own_1905_device *ctx);
#ifdef MAP_R2
#ifdef DFS_CAC_R2
void topo_srv_parse_wapp_cac_status_report(struct mapd_global * global,
	struct msg * wapp_event, unsigned short rep_len);
#endif
/*WAPP_METRIC_REP_INTERVAL_CAP*/
void topo_srv_parse_wapp_metric_rep_interval(struct mapd_global * global,
	u32 *interval);
#endif
void topo_srv_parse_ch_list_dfs_info(struct mapd_global * global,
	u8 *buf, unsigned short len);
#ifdef MAP_R2
void topo_srv_parse_r2_mbo_sta_non_pref_list(struct mapd_global * global,
	unsigned char *buf, unsigned short len);

#endif
#endif
void topo_srv_parse_wts_config(struct mapd_global * global,
	struct set_config_bss_info *bss_config, unsigned short config_length);
#endif
