/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2011, Mediatek, Inc.
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
 *  AP Estimator
 *
 *  Abstract:
 *  AP Estimator Headers
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Neelansh.M   2018/05/02     First implementation of the ap est. Module
 * */

#ifndef AP_EST_H
#define AP_EST_H

#define MAX_11K_RETRIES 4

#ifdef SUPPORT_MULTI_AP

#define AIR_MON_TIMEOUT		30

struct GNU_PACKED unassoc_sta_link_metric
{
	u8 cli_mac[ETH_ALEN];
	u8 channel;
	u32 delta_time;
	s8 rssi;
};

struct GNU_PACKED  unassoc_link_metric_rsp
{
	u8 op_class;
	u8 sta_num;
	struct unassoc_sta_link_metric sta_link_metric[0];
};
struct GNU_PACKED  unassoc_link_metric_tlv_rsp{
	u8 al_mac[ETH_ALEN];
	u8 type;
	u16 length;
	struct unassoc_link_metric_rsp unasssoc_link_rsp;
};

s8 ap_est_update_remote_non_serving_rssi(struct mapd_global *global,
        struct client *cli, struct bss_info_db *cand_bss_1,
		struct bss_info_db *cand_bss_2,s8 cand_bss_1_rssi);
#endif

void ap_est_update_non_serving_rssi(struct mapd_global *global, struct client *cli,
				uint8_t actual_radio_idx, uint8_t desired_radio_idx);
uint16_t ap_est_get_airtime(struct mapd_global *global, uint16_t ul_rate,
    uint16_t dl_rate, const uint16_t dl_phy_rate);
void ap_est_handle_11k_report(struct mapd_global *global, u8 *mac_addr,
    uint8_t isSuccess, u8 *bssid, u8 channel, u8 rcpi,u8 islastreport);
Boolean ap_est_trigger_11k_channel_measurement(struct mapd_global *global,
    struct client *cli);
uint16_t ap_est_map_dl_rssi_to_phyrate(struct mapd_global *global, struct mapd_radio_info *radio_info,
	struct client *cli, int8_t rssi);
int8_t ap_est_get_noise_offset(struct mapd_global *global, uint8_t radio_idx);
Boolean ap_est_trigger_channel_measurement(struct mapd_global *pGlobal_dev,int client_id, DECISION_DATA *data);
void ap_est_11k_cleanup(struct mapd_global *global, struct client *cli);
#ifdef SUPPORT_MULTI_AP
void ap_est_handle_unassoc_sta_link_metric_rsp(struct mapd_global *pGlobal_dev,
						struct unassoc_link_metric_tlv_rsp *msg,
						u32 msg_len);
Boolean ap_est_trigger_air_mon(struct mapd_global *pGlobal_dev,int client_id);
int8_t ap_est_update_non_serving_rssi_for_bh(struct mapd_global *global,
         struct bh_link_entry *bh_entry, uint8_t cur_band, uint8_t desired_band);
#endif

#ifdef CENT_STR
uint16_t ap_est_map_dl_rssi_to_phyrate_1905_radio(struct mapd_global *global, struct radio_info_db *target_radio,
		struct client *cli, int8_t rssi);

int8_t ap_est_get_noise_offset_by_1905_radio(struct mapd_global *global, struct radio_info_db *target_radio);

void ap_est_trigger_11k_channel_measurement_cent_str(struct mapd_global *global,
				struct client *cli);
Boolean ap_est_trigger_air_mon_cent_str(struct mapd_global *pGlobal_dev,int client_id);
void ap_est_handle_own_unassoc_sta_link_metric_rsp(struct mapd_global *pGlobal_dev,
						struct unassoc_link_metric_rsp *unassoc_link_metric_msg,
						u32 msg_len);

int8_t ap_est_update_non_serving_rssi_cent_str(struct mapd_global *global,
         struct client *cli, uint8_t cur_band, uint8_t desired_band);

int topo_srv_get_rssi_th_by_policy_1905_radio(struct own_1905_device *ctx,struct radio_info_db *radio_info , int8_t *rssi);

void topo_srv_trigger_beacon_metrics_query(struct mapd_global *global, struct _1905_map_device *dev,u8 *sta_mac,
				u8 *assoc_bssid, u8 ssid_len, u8 *ssid,
				u8 channel, u8 op_class, u8 *bssid,
				u8 rpt_detail, u8 num_elem, char *elem_list,
				u8 num_chrep, struct ap_chn_rpt *chan_rpt);
unsigned char  get_1905_radio_max_ch_util(struct radio_info_db *target);
struct bss_info_db * get_same_ess_on_1905_radio(struct _1905_map_device *target_1905_device,struct radio_info_db *target_radio_info, struct bss_info_db *curr_bss);



#endif

#endif
