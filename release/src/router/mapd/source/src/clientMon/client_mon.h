uint8_t client_mon_handle_local_join(struct mapd_global *global, u8 *mac_addr, u8 *bssid,
				u8 valid, u8 bBtm, u8 bRrm, u8 bMbo,
				int nss, enum phy_mode phy_mode, enum max_bw bw, struct map_he_nss *nss_he);
void client_mon_handle_local_leave(struct mapd_global *global, u8 *mac_addr, u8 *leaving_bssid);
#ifdef SUPPORT_MULTI_AP
void client_mon_handle_remote_join(struct mapd_global *global, u8 *mac_addr,
					 u8 *bssid, u8 *al_mac);
void client_mon_handle_remote_leave(struct mapd_global *global, struct client_assoc *assoc_notif);
void 	client_mon_handle_topo_notification(struct mapd_global *global,struct topo_notification *evt);
#endif
void client_mon_handle_link_metrics(struct mapd_global *global, u8 *sta_mac,
				uint32_t dl_phy_rate, int8_t ul_rssi, u8 *bssid);
void client_mon_chk_post_assoc_str(struct mapd_global *global);
void client_mon_get_assoc_stats(struct mapd_global *global);
void client_mon_handle_client_preq(struct mapd_global *global, uint8_t *mac_addr,
		uint8_t channel, uint8_t rssi, uint8_t preq_len, uint8_t *preq,
		struct os_reltime now);
void client_mon_handle_activity_change(struct mapd_global *global,
		uint32_t client_id, uint8_t act_stat);
void client_mon_bl_sta_for_bss(struct mapd_global *global, struct client *cli,
		u8 num_bss, struct mapd_bss **bss_arr, Boolean blacklist);
#ifdef MAP_R2
void client_mon_handle_auth_rej(struct mapd_global *global, wapp_sta_cnnct_rej_info *rej_cnnct_info);
#else
void client_mon_handle_auth_rej(struct mapd_global *global, u8 *mac_addr);
#endif
void client_mon_handle_traffic_stats(struct mapd_global *global, u8 *mac_addr,
                u32 tx_bytes, u32 rx_bytes, u32 tx_pkts, u32 rx_pkts, u32 tx_errs,
                u32 rx_errs);
void client_mon_handle_tp_metrics(struct mapd_global *global, u8 *sta_mac,
                                uint32_t tx_tp, uint32_t rx_tp, u8 *bssid);
void client_mon_cli_db_maintenance(struct mapd_global *global);

void client_mon_bl_sta_for_all_bss(struct mapd_global *global, struct client *cli,
		u8 num_bssid, u8 excluding_bssid_arr[][ETH_ALEN], Boolean blacklist);

Boolean chan_mon_is_radio_ol(struct mapd_global *global, struct mapd_radio_info *ra_info);
#ifdef SUPPORT_MULTI_AP
void client_mon_handle_assoc_control(struct mapd_global *global, struct cli_assoc_control *assoc_cntrl, u8 len, u8 *al_mac);
#endif
void client_mon_unblock_cli_on_bss(struct mapd_global *global,
        struct mapd_bss *bss, struct client *cli, u32 reason, u8 *al_mac);
void client_mon_force_unblock_cli_on_all_bss(struct mapd_global *global, struct client *cli);
Boolean is_cli_bl_map_assoc_control(struct mapd_global *global, struct client *cli);
void client_mon_block_cli_on_bss(struct mapd_global *global,
		struct mapd_bss *bss, struct client *cli, u32 reason, u32 duration, u8 *al_mac);
