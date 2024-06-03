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
 *  Wapp event
 *
 *  Abstract:
 *  wapp event handler
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Kapil.Gupta 2018/05/02    First implementation of the wapp event handler
 * */

#include "includes.h"
#ifdef __linux__
#include <fcntl.h>
#endif				/* __linux__ */

#include "common.h"
#include <sys/un.h>
#include "mapd_debug.h"

#include "interface.h"
#include "./../1905_local_lib/data_def.h"
#include "client_db.h"
#include "mapd_i.h"
#include "topologySrv.h"
#include "eloop.h"
#include "wapp_if.h"
#include "tlv_parsor.h"
#include "apSelection.h"
#include "1905_map_interface.h"
#include "ch_planning.h"
#include "network_optimization.h"

#ifdef MAP_R2

/**
* @brief Fn to update channel scan caps for an 1905 device
*
* @param ctx own 1905 device ctx
* @param dev 1905 device pointer
* @param pcap ht capability
*
* @return 0 if success else -1
*/
int topo_srv_update_ch_scan_cap(struct own_1905_device *ctx,size_t len , struct channel_scan_capab *pcap)
{
	if(ctx->scan_capab && ctx->scan_capab_len != len)
		ctx->scan_capab = os_realloc(ctx->scan_capab,len);
	else if(ctx->scan_capab == NULL)
		ctx->scan_capab = os_malloc(len);

	if(ctx->scan_capab == NULL)
		return -1;

	os_memset(ctx->scan_capab, 0, len);

	ctx->scan_capab_len = len;
	os_memcpy(ctx->scan_capab, pcap, len);
	return 0;
}
int topo_srv_update_r2_ap_cap(struct own_1905_device *ctx,size_t len , struct ap_r2_capability *pcap)
{
	if(ctx->r2_ap_capab && ctx->r2_ap_capab_len != len)
		ctx->r2_ap_capab = os_realloc(ctx->r2_ap_capab,len);
	else if(ctx->r2_ap_capab == NULL)
		ctx->r2_ap_capab = os_malloc(len);

	if(ctx->r2_ap_capab == NULL)
		return -1;

	os_memset(ctx->r2_ap_capab, 0, len);

	ctx->r2_ap_capab_len = len;
	os_memcpy(ctx->r2_ap_capab, pcap, len);
	return 0;
}
#endif

#ifdef DFS_CAC_R2
/**
* @brief Fn to update channel scan caps for an 1905 device
*
* @param ctx own 1905 device ctx
* @param dev 1905 device pointer
* @param pcap cac capability
*
* @return 0 if success else -1
*/
int topo_srv_update_cac_cap(struct own_1905_device *ctx,size_t len , struct cac_capability *pcap)
{
	struct _1905_map_device *own_dev = topo_srv_get_1905_device(ctx, NULL);
	struct radio_info_db *radio = NULL;
	struct cac_cap_tlv *p_cac_cap_in = NULL;
	struct cac_cap_type *p_cac_type_in = NULL;
	struct cac_cap_opcap *p_cac_opcap_in = NULL;
	int i = 0, j = 0, k = 0, l = 0;
	int offset_type, offset_opcap;

	if(ctx->cac_capab && ctx->cac_capab_len != len)
		ctx->cac_capab = os_realloc(ctx->cac_capab,len);
	else if(ctx->cac_capab == NULL)
		ctx->cac_capab = os_malloc(len);

	if(ctx->cac_capab == NULL)
		return -1;

	os_memset(ctx->cac_capab, 0, len);

	ctx->cac_capab_len = len;
	os_memcpy(ctx->cac_capab, pcap, len);
	p_cac_cap_in = pcap->cap;
	for (i = 0; i < pcap->radio_num; i++) {
	    radio = topo_srv_get_radio(own_dev, p_cac_cap_in->identifier);
	    if(!radio) {
	                    err("dev radio not found some error");
	                    return -1;
	    }
	    os_memcpy(radio->cac_cap.identifier, p_cac_cap_in->identifier, ETH_ALEN);
	    radio->cac_cap.cac_type_num = p_cac_cap_in->cac_type_num;
	    topo_srv_clear_cac_cap_db(radio);
	    SLIST_INIT(&radio->cac_cap.cac_capab_head);
	    p_cac_type_in = p_cac_cap_in->type;
	    offset_type = 0;
	    for (j = 0; j < radio->cac_cap.cac_type_num; j++) {
	        struct cac_cap_db *cac = os_zalloc(sizeof(struct cac_cap_db));
			if(!cac) {
				err("mem alloc fail");
				return -1;
			}
	        cac->cac_mode = p_cac_type_in->cac_mode;
	        err ("MY device CAC Mode = %d", cac->cac_mode);
	        os_memcpy(cac->cac_interval, p_cac_type_in->cac_interval, sizeof(p_cac_type_in->cac_interval));
	        cac->op_class_num = p_cac_type_in->op_class_num;

	        SLIST_INSERT_HEAD(&radio->cac_cap.cac_capab_head, cac, cac_cap_entry);
	        SLIST_INIT(&cac->cac_opcap_head);
	        p_cac_opcap_in = p_cac_type_in->opcap;
	        offset_opcap = 0;
			for (k = 0; k < cac->op_class_num; k++) {
			    struct cac_opcap_db *opcap = os_zalloc(sizeof(struct cac_opcap_db));
				if(!opcap) {
				    err("mem alloc fail");
				    os_free(cac);
				    return -1;
				}
			    opcap->op_class = p_cac_opcap_in->op_class;
			    opcap->ch_num = p_cac_opcap_in->ch_num;
				for (l = 0; l < opcap->ch_num; l++) {
				    opcap->ch_list[l] = p_cac_opcap_in->ch_list[l];
				}
			    SLIST_INSERT_HEAD(&cac->cac_opcap_head, opcap, cac_opcap_entry);
			    p_cac_opcap_in = (struct cac_cap_opcap *)((char *)p_cac_opcap_in + sizeof(struct cac_cap_opcap) + opcap->ch_num);
			    offset_opcap += sizeof(struct cac_cap_opcap) + opcap->ch_num;
			}
	        p_cac_type_in = (struct cac_cap_type *)((char *)p_cac_type_in + sizeof(struct cac_cap_type) + offset_opcap);
	        offset_type += sizeof(struct cac_cap_type) + offset_opcap;
	    }
		p_cac_cap_in = (struct cac_cap_tlv *)((char *)p_cac_cap_in + sizeof(struct cac_cap_tlv) + offset_type);
	}
	//mapd_hexdump(MSG_OFF,"CAC CAP FROM WAPP",&radio->cac_cap, 50);

	return 0;
}
#endif

/**
* @brief Fn to query ap metrics infro from wapp
*
* @param ctx own 1905 device ctx
*/
void topo_srv_get_ap_metrics_info(struct own_1905_device *ctx)
{
	struct bss_db *bss = NULL;

	SLIST_FOREACH(bss, &ctx->metric_entry.metrics_query_head, bss_entry) {
		wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_AP_METRICS_INFO,
					WAPP_AP_METRICS_INFO, bss->bssid, NULL, NULL, 0, 1, 1, 0);
	}
}
#ifdef MAP_R2
void topo_srv_get_radio_metrics_info(struct own_1905_device *ctx)
{
	int i;
	int band_idx = ctx->metric_entry.total_radio_band;
	debug("band idx %d\n", band_idx);
	for (i = 0; i < band_idx; i++){
		wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_RADIO_METRICS_INFO,
					WAPP_RADIO_METRICS_INFO, NULL, NULL, ctx->metric_entry.radio_id[i].identifier, ETH_ALEN, 1, 1, 0);
	}
}
void topo_srv_get_all_radio_metrics_info(struct own_1905_device *ctx)
{
	struct radio_info_db *radio_db = NULL;
	struct _1905_map_device *device = topo_srv_get_1905_device(ctx, NULL);
	SLIST_FOREACH(radio_db,&device->first_radio,next_radio) {
		debug("send wapp get radio cmd for channel %d ", radio_db->channel[0]);
		wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_RADIO_METRICS_INFO,
			WAPP_RADIO_METRICS_INFO, NULL, NULL, radio_db->identifier, ETH_ALEN, 1, 1, 0);
	}
}

#endif


void topo_srv_get_own_metrics_info(struct own_1905_device *ctx)
{
#if 0
       struct bss_info_db *bss = NULL;
       struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);

       SLIST_FOREACH(bss, &dev->first_bss, next_bss) {
		err("1905: bss mac metrics issue: (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bss->bssid));
                wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_AP_METRICS_INFO,
                                        WAPP_AP_METRICS_INFO, bss->bssid, NULL, NULL, 0, 1, 1, 0);
        }
#endif
	/*some bss may be released; use operating bss instead of all bss*/
	struct mapd_bss *bss = 0;
   	uint8_t radio_idx;
	struct mapd_global *global = (struct mapd_global *)ctx->back_ptr;

  	for (radio_idx = 0; radio_idx < MAX_NUM_OF_RADIO; radio_idx++) {
	   struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[radio_idx];
	   if (radio_info->radio_idx == (uint8_t)-1)
		   continue;
	   dl_list_for_each(bss, &radio_info->bss_list, struct mapd_bss, bss_entry) {
		   info("1905: bss mac metrics issue: (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(bss->bssid));
		   wlanif_issue_wapp_command(global, WAPP_USER_GET_AP_METRICS_INFO,
					   WAPP_AP_METRICS_INFO, bss->bssid, NULL, NULL, 0, 1, 1, 0);
	   }
  	}
 }

void topo_srv_get_own_link_metrics_info(struct own_1905_device *ctx)
{
	struct link_stat_query lsq;
	struct map_neighbor_info *neighbor;
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	struct iface_info *ifc_info = NULL;

	SLIST_FOREACH(neighbor, &dev->neighbors_entry, next_neighbor) {
		ctx->metric_entry.bh = SLIST_FIRST(&neighbor->bh_head);
		ifc_info = topo_srv_get_iface(dev, ctx->metric_entry.bh->connected_iface_addr);
		if(!ifc_info)
			continue;
		lsq.media_type = ifc_info->media_type;
		err("media type: %d\n", lsq.media_type);
		os_memcpy(lsq.local_if, ctx->metric_entry.bh->connected_iface_addr, ETH_ALEN);
		err("local if mac: (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(lsq.local_if));
		os_memcpy(lsq.neighbor_if, ctx->metric_entry.bh->neighbor_iface_addr, ETH_ALEN);
		err("neighbor if mac: (%02x:%02x:%02x:%02x:%02x:%02x)", PRINT_MAC(lsq.neighbor_if));
		wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_TX_LINK_STATISTICS,
			WAPP_TX_LINK_STATISTICS, ifc_info->iface_addr, NULL, &lsq, sizeof(struct link_stat_query), 0, 1, 0);
		wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_GET_RX_LINK_STATISTICS,
			WAPP_RX_LINK_STATISTICS, ifc_info->iface_addr, NULL, &lsq, sizeof(struct link_stat_query), 0, 1, 0);
		ctx->metric_entry.bh = NULL;
	}

}

/**
* @brief Fn to query assoc sta traffic stats from wapp
*
* @param ctx own 1905 device ctx
*/
void topo_srv_get_assoc_sta_traffic_stats(struct own_1905_device *ctx)
{
	struct metric_policy_db *policy = NULL;

	SLIST_FOREACH(policy, &ctx->map_policy.mpolicy.policy_head, policy_entry) {
		if (policy->sta_stats_inclusion) {
			wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr,
					       WAPP_USER_GET_ASSOC_STA_TRAFFIC_STATS,
					       WAPP_ALL_ASSOC_STA_TRAFFIC_STATS, NULL,
					NULL, (void *)policy->identifier, ETH_ALEN, 1, 1, 0);
		}
	}
}

/**
* @brief Fn to query all assoc sta traffic stats from wapp
*
* @param ctx own 1905 device ctx
*/
void topo_srv_get_all_assoc_sta_link_metrics(struct own_1905_device *ctx)
{
	struct metric_policy_db *policy = NULL;

	SLIST_FOREACH(policy, &ctx->map_policy.mpolicy.policy_head, policy_entry) {
		if (policy->sta_metrics_inclusion) {
			wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr,
					       WAPP_USER_GET_ASSOC_STA_LINK_METRICS,
					       WAPP_ALL_ASSOC_STA_LINK_METRICS, NULL,
					NULL, (void *)policy->identifier, ETH_ALEN, 1, 1, 0);
#ifdef MAP_R2
			wlanif_get_all_assoc_sta_ext_link_metrics((struct mapd_global *)ctx->back_ptr,
				(void *)policy->identifier);
#endif
		}
	}
}


/**
* @brief Fn to query radio caps from wapp
*
* @param ctx own 1905 device ctx
* @param identifier radio identifier
*/
void topo_srv_get_radio_capinfo(struct own_1905_device *ctx, unsigned char *identifier)
{
	/*get cap for the specific radio */
	map_get_info_from_wapp(ctx, WAPP_USER_GET_AP_CAPABILITY, WAPP_AP_CAPABILITY, identifier, NULL, NULL, 0);
	map_get_info_from_wapp(ctx, WAPP_USER_GET_RADIO_BASIC_CAP, WAPP_RADIO_BASIC_CAP, identifier, NULL, NULL, 0);
	map_get_info_from_wapp(ctx, WAPP_USER_GET_AP_HT_CAPABILITY, WAPP_AP_HT_CAPABILITY, identifier, NULL, NULL, 0);
	map_get_info_from_wapp(ctx, WAPP_USER_GET_AP_VHT_CAPABILITY, WAPP_AP_VHT_CAPABILITY, identifier, NULL, NULL, 0);
	map_get_info_from_wapp(ctx, WAPP_USER_GET_CHANNEL_PREFERENCE, WAPP_CHANNLE_PREFERENCE,
			       identifier, NULL, NULL, 0);
	map_get_info_from_wapp(ctx, WAPP_USER_GET_RA_OP_RESTRICTION, WAPP_RADIO_OPERATION_RESTRICTION,
			       identifier, NULL, NULL, 0);
	//! one possibility of reaching here is when we got config renew. we do not want to get BH profile when config renew got triggered on its own in ethernt plug in in API mode.
	if (ctx->current_bh_state != BH_STATE_DEFAULT)
		map_get_info_from_wapp(ctx, WAPP_USER_GET_BH_WIRELESS_SETTING, WAPP_MAP_BH_CONFIG, identifier, NULL, NULL, 0);
	map_get_info_from_wapp(ctx, WAPP_USER_GET_AP_HE_CAPABILITY, WAPP_AP_HE_CAPABILITY, identifier, NULL, NULL, 0);
#ifdef MAP_R2
	map_get_info_from_wapp(ctx, WAPP_USER_GET_METRIC_REP_INTERVAL_CAP, WAPP_METRIC_REP_INTERVAL_CAP, identifier, NULL, NULL, 0);
#endif
}

#ifdef MAP_R2
void dump_ch_scan_rep(struct net_opt_scan_report_event *scan_rep_evt);
void dump_tunneled(struct tunneled_msg *tunneled);
void dump_assoc(struct assoc_notification *assoc);
#endif


void topo_srv_parse_wapp_ap_metric_event(struct mapd_global * global,
	struct ap_metrics_info *minfo, unsigned short from)
{
	struct own_1905_device *ctx = &global->dev;
#ifdef MAP_R2
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
#endif
	if (from) {
		info("receive ap metrics info event from=%d", from);
		delete_exist_ap_metrics_info(ctx, minfo->bssid);
		insert_new_metrics_info(ctx, minfo);
	} else
		topo_srv_update_bss_chan_util(ctx, minfo);
#ifdef MAP_R2
	dev->de_done = 0;
#endif
}

void topo_srv_parse_wapp_all_sta_traffic_stats(struct mapd_global * global,
	struct sta_traffic_stats *stats, unsigned short from)
{
	struct own_1905_device *ctx = &global->dev;
	if (!from)
		return;

	err("receive all associated sta traffic stats event");
	delete_exist_traffic_stats_info(ctx, stats->identifier);
	insert_new_traffic_stats_info(ctx, stats);
}

void topo_srv_parse_wapp_all_assoc_link_metric(struct mapd_global * global,
	struct sta_link_metrics *metrics, unsigned short from)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	struct link_metrics *info = NULL;
	struct associated_clients *metrics_ctx = NULL;
	int i;

	debug("receive associated all assoc sta link metrics event");
	insert_new_link_metrics_info(_1905_ctrl,ctx, metrics);
#if 0
	if (from)
		map_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(_1905_ctrl, metrics->sta_cnt, metrics->info);
#endif
	if (is_1905_present())
		for (i = 0; i < metrics->sta_cnt; i++) {
			info = &metrics->info[i];
			metrics_ctx = topo_srv_get_associate_client(ctx, NULL, info->mac);
			if (!metrics_ctx)
				continue;

			info->rssi_uplink = rcpi_to_rssi(info->rssi_uplink);
		}
}


void topo_srv_parse_wapp_one_assoc_link_metric(struct mapd_global * global,
	struct link_metrics *metrics, unsigned short from)
{
	struct own_1905_device *ctx = &global->dev;
#ifndef MAP_R2
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
#endif

	update_one_sta_link_metrics_info(ctx, metrics);
	metrics->rssi_uplink = rssi_to_rcpi((signed char)metrics->rssi_uplink);
#ifndef MAP_R2
	if (from)
		map_1905_Set_Assoc_Sta_Link_Metric_Rsp_Info(_1905_ctrl, 1, metrics, NULL, 0);
#endif
	metrics->rssi_uplink = rcpi_to_rssi(metrics->rssi_uplink);
}

void topo_srv_parse_wapp_unassoc_link_metric(struct mapd_global * global,
	struct unlink_metrics_rsp *unlink_metrics)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	struct unlink_rsp_sta *info = NULL;
	int i;

	delete_exist_unlink_metrics_rsp(&ctx->metric_entry.unlink_info);
	update_unlink_metrics_rsp(&ctx->metric_entry.unlink_info, unlink_metrics);
	map_1905_Set_Unassoc_Sta_Link_Metric_Rsp_Info(_1905_ctrl, unlink_metrics);
	for (i = 0; i < unlink_metrics->sta_num; i++) {
		info = &unlink_metrics->info[i];
		info->uplink_rssi = rcpi_to_rssi(info->uplink_rssi);
	}
}

void topo_srv_parse_wapp_air_monitor_report(struct mapd_global * global,
	struct unlink_metrics_rsp *unlink_metrics)
{
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	int i;
	struct unlink_rsp_sta *info = NULL;

	for (i = 0; i < unlink_metrics->sta_num; i++) {
		info = &unlink_metrics->info[i];
		info->uplink_rssi = rssi_to_rcpi(info->uplink_rssi);
	}
	map_1905_Set_Unassoc_Sta_Link_Metric_Rsp_Info(_1905_ctrl, unlink_metrics);
}


void topo_srv_parse_wapp_cli_steer_btm_report(struct mapd_global * global,
	struct cli_steer_btm_event *evt)
{
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;

	map_1905_Set_Cli_Steer_BTM_Report_Info(_1905_ctrl, evt);
}

void topo_srv_parse_wapp_cli_steering_completed(struct mapd_global * global,
	struct cli_steer_btm_event *evt)
{
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;

	map_1905_Set_Steering_Complete_Info(_1905_ctrl);
}

void topo_srv_parse_wapp_read_bss_conf_request(struct mapd_global * global,
	char *file_path, unsigned int len)
{
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;

	map_1905_Set_Read_Bss_Conf_Request(_1905_ctrl);
}

void topo_srv_parse_wapp_operating_channel_report(struct mapd_global * global,
	unsigned char *buf)
{
	//struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	struct channel_report *chan_rpt = (struct channel_report *)buf;
	struct ch_rep_info *report_info;
	int report_cnt = 0;

	for (report_cnt = 0;report_cnt < chan_rpt->ch_rep_num; report_cnt++) {
			report_info = &chan_rpt->info[report_cnt];
			err("ch: %u, opclass: %u", report_info->channel, report_info->op_class);
	}
	err("send to 1905");

	//topo_srv_update_own_radio_info(ctx, buf);
	map_1905_Set_Operating_Channel_Report_Info(_1905_ctrl,
		(struct channel_report *)buf);
}


void topo_srv_parse_wapp_beacon_metrics_report(struct mapd_global * global,
	struct beacon_metrics_rsp *evt)
{
	struct beacon_metrics_rsp_lib *rsp;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;

	rsp = malloc(sizeof(*rsp) + evt->rpt_len);
	memcpy(rsp->sta_mac, evt->sta_mac, ETH_ALEN);
	rsp->status = evt->reserved;	//TODO check if same
	rsp->bcn_rpt_num = evt->bcn_rpt_num;
	rsp->rpt_len = evt->rpt_len;
	memcpy(rsp->rpt, evt->rpt, evt->rpt_len);

	/* TODO check whether this is needed in topology server */
	map_1905_Set_Beacon_Metrics_Report_Info(_1905_ctrl, rsp);
	free(rsp);
}


void topo_srv_parse_wapp_client_notification(struct mapd_global * global,
	struct client_association_event_local *evt)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	struct map_client_association_event_local *pevt = NULL;
	struct client_association_event *mod_evt = NULL;

	pevt = (struct map_client_association_event_local*)&evt->map_assoc_evt;
	if(topo_srv_update_assoc_client_info(ctx, pevt) == 0) {

		mod_evt = os_zalloc(sizeof(struct client_association_event) + evt->map_assoc_evt.assoc_req_len);
		if (mod_evt) {
			mod_evt->map_assoc_evt.assoc_evt = evt->map_assoc_evt.assoc_evt;
			os_memcpy(mod_evt->map_assoc_evt.sta_mac, evt->map_assoc_evt.sta_mac, MAC_ADDR_TLV_LENGTH);
			os_memcpy(mod_evt->map_assoc_evt.bssid, evt->map_assoc_evt.bssid, MAC_ADDR_TLV_LENGTH);
			mod_evt->map_assoc_evt.assoc_time = evt->map_assoc_evt.assoc_time;
			mod_evt->map_assoc_evt.assoc_req_len = evt->map_assoc_evt.assoc_req_len;
			os_memcpy(&mod_evt->cli_caps, &evt->cli_caps, sizeof(struct map_priv_cli_cap));

			if (evt->map_assoc_evt.assoc_req_len)
				os_memcpy(mod_evt->map_assoc_evt.assoc_req, evt->map_assoc_evt.assoc_req, evt->map_assoc_evt.assoc_req_len);

			map_1905_Set_Sta_Notification_Info(_1905_ctrl, mod_evt);
			os_free(mod_evt);
		}
		else
			always("map sta notification failed");

	}
}

void topo_srv_parse_wapp_bh_ready(struct mapd_global * global,
	struct bh_link_info *bh_info)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	struct bh_link_entry *bh_entry = NULL;

	err("got bh ready type=%d", bh_info->type);

	bh_info->trigger_autconf  = 1;
	if(global->params.Certification){
		map_1905_Set_Bh_Ready(_1905_ctrl, bh_info);
		return;
	}
	/* If ethernet start send directly to 1905 for onboarding */
	if (bh_info->type == 0) {
		map_1905_Set_Bh_Ready(_1905_ctrl, bh_info);
		//ctx->current_bh_state = BH_STATE_ETHERNET_PLUGGED;
		/*Notify app for eth onboarding start */
		mapd_send_onboardstatus_to_app((struct mapd_global *)ctx->back_ptr,
						ETH_ONBOARDING_STATE_START, bh_info->type);

		if (ctx->dhcp_ctl_enable && DEVICE_ROLE_AGENT == ctx->device_role) {
			if (!(0 == ctx->dhcp_req.dhcp_server_enable && 1 == ctx->dhcp_req.dhcp_client_enable)) {
				ctx->dhcp_req.dhcp_server_enable = 0;
				ctx->dhcp_req.dhcp_client_enable = 1;
				/*send command to wapp*/
				wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_DHCP_CTL_REQUEST,
					0, NULL, NULL, &ctx->dhcp_req, sizeof(struct dhcp_ctl_req), 0, 0, 0);
				/*timeout*/
				map_register_dhcp_timer(ctx);
			}
		}
		return;
	}
	if (ctx->bh_ready_expected == FALSE)
	{
		err("bh ready is not exepected");
		return;
	} else {
		err("receive WAPP_MAP_BH_READY");
		if (topo_srv_parse_backhaul_ready_evt(ctx, bh_info) < 0) {
			err(" failed to parse");
		}

		SLIST_FOREACH(bh_entry, &(ctx->bh_link_head), next_bh_link) {
			if (os_strcmp((char *)bh_entry->ifname, (char *)bh_info->ifname) == 0)
					break;
		}

		if (bh_info->type != 0) {
			topo_srv_update_upstream_device(ctx, NULL, bh_info->bssid);
		}


		if (ctx->ThirdPartyConnection) {
			if((DEVICE_ROLE_AGENT == ctx->device_role)||
				((DEVICE_ROLE_UNCONFIGURED == ctx->device_role) &&
				(!ctx->ConnectThirdPartyVend))) {
				map_1905_Set_Bh_Ready(_1905_ctrl, bh_info);
			}
		} else {
			if ((ctx->bh_dup_entry && bh_entry) && (ctx->bh_dup_entry == bh_entry)) {
				ctx->bh_dup_entry = NULL;
				always("BH redy for duplicate link; Skip autoconf");
				bh_info->trigger_autconf = 0;
			}
			map_1905_Set_Bh_Ready(_1905_ctrl, bh_info);
		}
		mapd_send_onboardstatus_to_app((struct mapd_global *)ctx->back_ptr,
						WIFI_ONBOARDING_STATE_START, bh_info->type);
		if (ctx->dhcp_ctl_enable) {
			if (!(0 == ctx->dhcp_req.dhcp_server_enable && 1 == ctx->dhcp_req.dhcp_client_enable)) {
				ctx->dhcp_req.dhcp_server_enable = 0;
				ctx->dhcp_req.dhcp_client_enable = 1;
				/*send command to wapp*/
				wlanif_issue_wapp_command((struct mapd_global *)ctx->back_ptr, WAPP_USER_SET_DHCP_CTL_REQUEST,
					0, NULL, NULL, &ctx->dhcp_req, sizeof(struct dhcp_ctl_req), 0, 0, 0);

				/*timeout, check ip result*/
				map_register_dhcp_timer(ctx);
			}
		}

		if (!bh_entry) {
			err("failed to get the bh entry");
		} else {
			memcpy(bh_entry->bssid, bh_info->bssid, ETH_ALEN);
			bh_entry->bh_assoc_state = WAPP_APCLI_ASSOCIATED;
			err("updated bssid of mac entry ");
		}
	}
}

void topo_srv_parse_wapp_1905_cmdu_request(struct mapd_global * global,
	struct _1905_cmdu_request *request)
{
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;

	map_1905_Send_Higher_Layer_Date_Message(_1905_ctrl, (char *)request->dest_al_mac, request->type,
						request->len, request->body);
}


void topo_srv_parse_wapp_radio_basic_cap(struct mapd_global * global,
	struct ap_radio_basic_cap *bcap)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;

	if (topo_srv_update_radio_basic_cap(ctx, NULL, bcap) < 0) {
		debug("update operation restriction fail");
	}
	map_1905_Set_Radio_Basic_Cap(_1905_ctrl, bcap);
}


void topo_srv_parse_wapp_radio_operation_restriction(struct mapd_global * global,
	struct restriction *restrict_var)
{
	struct own_1905_device *ctx = &global->dev;

	if (topo_srv_update_operation_restriction(ctx, NULL, restrict_var) < 0) {
		debug("update operation restriction fail");
	}
}

void topo_srv_parse_wapp_ht_capability(struct mapd_global * global,
	struct ap_ht_capability *cap)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;

	if (topo_srv_update_ap_ht_cap(ctx, NULL, cap) < 0) {
		err("update ap ht cap fail");
	}
	map_1905_Set_Ap_Ht_Cap(_1905_ctrl, cap);
}


void topo_srv_parse_wapp_vht_capability(struct mapd_global * global,
	struct ap_vht_capability *cap)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;

	if (topo_srv_update_ap_vht_cap(ctx, NULL, cap) < 0) {
		debug("update ap vht cap fail");
	}

	map_1905_Set_Ap_Vht_Cap(_1905_ctrl, cap);
}
void topo_srv_parse_wapp_he_capability(struct mapd_global * global,
	struct ap_he_capability *cap)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	if (topo_srv_update_ap_he_cap(ctx, NULL, cap) < 0) {
		debug("update ap he cap fail");
	}
	map_1905_Set_Ap_He_Cap(_1905_ctrl, cap);
}

void topo_srv_parse_wapp_channel_preferrence(struct mapd_global * global,
	struct ch_prefer *prefer)
{
	struct own_1905_device *ctx = &global->dev;

	if (topo_srv_update_channel_preference(ctx, NULL, prefer) < 0) {
		err("update ap vht cap fail");
	}
}

void topo_srv_parse_wapp_bh_steer_resp(struct mapd_global * global,
	struct backhaul_steer_rsp *steer_info)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;

	if (steer_info->status == 0) {
		struct _1905_map_device *tmp_dev = topo_srv_get_1905_by_iface_addr(ctx, steer_info->backhaul_mac);
		topo_srv_update_upstream_device(ctx, tmp_dev, steer_info->target_bssid);
	}
	map_1905_Set_Bh_Steer_Rsp_Info(_1905_ctrl, steer_info);
}


void topo_srv_parse_wapp_operating_channel_info(struct mapd_global * global,
	struct channel_report *chan_report)
{
	struct channel_report *tmp_chan_report;
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;

	/* update own radio info */
	topo_srv_update_own_radio_info(ctx, (unsigned char *)chan_report);
	/* latch this info, this will be needed later */
	tmp_chan_report = os_zalloc(sizeof(struct ch_rep_info) * chan_report->ch_rep_num + sizeof(unsigned char));
	os_memcpy(tmp_chan_report, chan_report, sizeof(struct ch_rep_info) * chan_report->ch_rep_num + sizeof(unsigned char));
	if (ctx->chan_report) {
		os_free(ctx->chan_report);
		ctx->chan_report = NULL;
	}
	ctx->chan_report = tmp_chan_report;
	/*TO be confirmed mandatory or optional*/
	if(global->params.Certification) {
		struct ch_sel_rsp_info rsp_info[MAX_CH_NUM];
		int i = 0;

		for (i = 0; i< tmp_chan_report->ch_rep_num; i ++) {
			os_memcpy(rsp_info[i].radio_indentifier, &tmp_chan_report->info[i].identifier, ETH_ALEN);
			rsp_info[i].rsp_code = 0;
			debug("\n"
				"\t\t radio_indentifier %2x:%02x:%02x:%02x:%02x:%02x\n"
				, PRINT_MAC(rsp_info[i].radio_indentifier));
		}

		map_1905_Set_Channel_Selection_Rsp_Info(_1905_ctrl, rsp_info, tmp_chan_report->ch_rep_num);
	}
	map_1905_Set_Operating_Channel_Report_Info(_1905_ctrl,
						   (struct channel_report *)chan_report);
}

void topo_srv_parse_wapp_ap_capability(struct mapd_global * global,
	struct ap_capability *ap_cap)
{
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	struct own_1905_device *ctx = &global->dev;

	map_1905_Set_Ap_Cap(_1905_ctrl, ap_cap);
	topo_srv_update_ap_cap(ctx, ap_cap);
}


void topo_srv_parse_wapp_oper_bss_report(struct mapd_global * global,
	struct oper_bss_cap *oper_bss)
{
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	struct own_1905_device *ctx = &global->dev;

	/* update own bss */
	topo_srv_update_own_bss_info(ctx,
		(unsigned char *)oper_bss);
	map_1905_Set_Operbss_Cap(_1905_ctrl, oper_bss);
}



void topo_srv_parse_wapp_scan_result(struct mapd_global * global,
	struct wapp_scan_info *scan_results)
{
	struct own_1905_device *ctx = &global->dev;

	ap_selection_parse_scan_result(ctx, scan_results);
}

void topo_srv_parse_wapp_scan_done(struct mapd_global * global)
{
	struct own_1905_device *ctx = &global->dev;

	ap_selection_issue_scan(ctx);
}

void topo_srv_parse_wapp_vend_ie_changed(struct mapd_global * global,
	struct map_vendor_ie *vendor_ie)
{
	struct own_1905_device *ctx = &global->dev;

	if (vendor_ie) {
		err(" connectivity_to_controller (%d)\n", vendor_ie->connectivity_to_controller);
		if (vendor_ie->connectivity_to_controller == 1)
		{
			struct radio_info_db *radio = NULL;
			unsigned char all_radios_status = TRUE;

			radio = topo_srv_get_radio(topo_srv_get_1905_device(ctx, NULL), NULL);
			while (radio) {
				if (radio->config_status == FALSE)
					all_radios_status = FALSE;
				radio = topo_srv_get_next_radio(topo_srv_get_1905_device(ctx, NULL), radio);
			}
			if (all_radios_status)
				ap_selection_update_vend_ie(ctx, vendor_ie, TRUE);
		} else if (vendor_ie->connectivity_to_controller == 0) {
			ap_selection_update_vend_ie(ctx, vendor_ie, TRUE);
		}
	}
}


void topo_srv_parse_wapp_get_wsc_config(struct mapd_global * global,
	struct wps_get_config *wps_config)
{
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;

	map_1905_Get_Wsc_Config(_1905_ctrl, wps_config);
}

int topo_srv_parse_wapp_ap_link_metirc_request(struct mapd_global * global,
	unsigned char *target_bssid)
{
	struct bss_db *bss = NULL, *tmp_bss = NULL;
	struct bss_info_db *topo_bss;
	struct radio_info_db *radio;
	struct own_1905_device *ctx = &global->dev;

	bss = SLIST_FIRST(&ctx->metric_entry.metrics_query_head);
	while (bss) {
		tmp_bss = SLIST_NEXT(bss, bss_entry);
		free(bss);
		bss = tmp_bss;
	}
	SLIST_INIT(&ctx->metric_entry.metrics_query_head);
	err("bssid(%02x:%02x:%02x:%02x:%02x:%02x) ",PRINT_MAC(target_bssid));
	radio = topo_srv_get_radio(topo_srv_get_1905_device(ctx, NULL), target_bssid);
	if (!radio) {
		err("failed to get radio");
		return -1;
	}
	topo_bss = topo_srv_get_next_bss(topo_srv_get_1905_device(ctx, NULL), NULL);
	while(topo_bss) {
		if (os_memcmp(radio->identifier, topo_bss->radio->identifier, ETH_ALEN) != 0) {
			topo_bss = topo_srv_get_next_bss(topo_srv_get_1905_device(ctx, NULL), topo_bss);
			continue;
		}
		bss = (struct bss_db *)os_malloc(sizeof(struct bss_db));
		if (!bss) {
			err("alloc struct bss_db fail");
			return -1;
		}
		memcpy(bss->bssid, topo_bss->bssid, ETH_ALEN);
		err("bssid(%02x:%02x:%02x:%02x:%02x:%02x) ",
				PRINT_MAC(bss->bssid));
		SLIST_INSERT_HEAD(&(ctx->metric_entry.metrics_query_head),
				bss, bss_entry);
		topo_bss = topo_srv_get_next_bss(topo_srv_get_1905_device(ctx, NULL), topo_bss);
	}
	topo_srv_get_ap_metrics_info(ctx);
	topo_srv_get_assoc_sta_traffic_stats(ctx);
	topo_srv_get_all_assoc_sta_link_metrics(ctx);
#if defined(MAP_R2) || defined(CENT_STR)
	_1905_if_send_ap_metric_rsp(ctx,0);
#else
	_1905_if_send_ap_metric_rsp(ctx);
#endif
	return 0;
}

void topo_srv_parse_wapp_assoc_state_changed(struct mapd_global * global,
	struct wapp_apcli_association_info *cli_assoc_info)
{
	struct own_1905_device *ctx = &global->dev;

	ap_selection_handle_cli_state_change(ctx, cli_assoc_info);
}

void topo_srv_parse_wapp_1905_read_tlv_req(struct mapd_global * global,
	char *tlv, int tlv_length)
{
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;

	if(global->params.Certification) {
		map_1905_Set_Read_1905_Tlv_Req(_1905_ctrl, tlv, tlv_length);
	}
}

void topo_srv_parse_wapp_device_status(struct mapd_global * global,
	wapp_device_status *device_status)
{
	os_memcpy(&global->dev.device_status, device_status, sizeof(wapp_device_status));
	always("FHBSS Status: %d, BHSTA Status: %d\n", global->dev.device_status.status_fhbss,
		global->dev.device_status.status_bhsta);

	/*Notify app for eth onboarding done*/
		if((global->dev.device_status.status_bhsta == STATUS_BHSTA_CONFIGURED) &&
			(global->dev.device_status.status_fhbss == STATUS_FHBSS_CONFIGURED)) {
			if (global->dev.device_role != DEVICE_ROLE_CONTROLLER)
				send_vs_bh_priority(&global->dev);
			if (global->dev.current_bh_state == BH_STATE_ETHERNET_PLUGGED) {
				mapd_send_onboardstatus_to_app(global,
						ETH_ONBOARDING_STATE_DONE, MAP_BH_ETH);
			} else if (global->dev.current_bh_state == BH_STATE_WIFI_LINKUP) {
				mapd_send_onboardstatus_to_app(global,
						WIFI_ONBOARDING_STATE_DONE, MAP_BH_WIFI);
			}
		}
}


void topo_srv_parse_wapp_off_channel_scan_report(struct mapd_global * global,
	struct off_ch_scan_report_event *scan_rep_evt)
{
	send_off_ch_scan_resp(global, scan_rep_evt);
}
void topo_srv_parse_wapp_net_opt_scan_report(struct mapd_global * global,
	struct net_opt_scan_report_event *scan_rep_evt)
{
	send_net_opt_scan_resp(global, scan_rep_evt);
}





/**
* @brief Fn to parse wapp event
*
* @param global mapd global
* @param buf msg buffer
* @param len msg length
* @param from whether it was request from 1905 or mapd
*
* @return -1 if error else 0
*/
void dump_off_ch_scan_rep(struct off_ch_scan_report_event *scan_rep_evt)
{
	int i;
	u8 * buf = NULL;
	buf = (u8 *)scan_rep_evt->scan_result;
	for(i=0; i< scan_rep_evt->scan_result_num;i++) {
		struct off_ch_scan_result_event *scan_result = (struct off_ch_scan_result_event *)buf;
		always("RESULT : %d--------------------->START\n", i);
		mapd_hexdump(MSG_OFF,"RadioID:",scan_result->radio_id,ETH_ALEN);
		always("Channel=%d\n",
				scan_result->channel);
		always("Utilization = %d,\n noise=%d\n", scan_result->utilization,
						scan_result->noise);
		always("Neighbor count %d\n", scan_result->neighbor_num);
		always("RESULT : %d--------------------->END\n\n", i);
		buf += sizeof(struct off_ch_scan_result_event);
	}
}
void dump_net_opt_off_ch_scan_rep(struct net_opt_scan_report_event *scan_rep_evt)
{
	int i,j;
	u8 * buf = NULL;
	buf = (u8 *)scan_rep_evt->scan_result;
	for(i=0; i< scan_rep_evt->scan_result_num;i++) {
		struct net_opt_scan_result_event *scan_result = (struct net_opt_scan_result_event *)buf;
		err("RESULT : %d--------------------->START\n", i);
		mapd_hexdump(MSG_OFF,"RadioID:",scan_result->radio_id,ETH_ALEN);
		err(" Channel=%d\n  \n",scan_result->channel);
		err("Utilization = %d,\n noise=%d\n", scan_result->utilization,
						scan_result->noise);
#ifdef MAP_R2
		err("edcca = %d,\n obss_time=%d\n", scan_result->edcca,
						scan_result->obss_time);
#endif
		err("Neighbor List %d-------------->\n", scan_result->neighbor_num);
		for (j=0;j<scan_result->neighbor_num;j++) {
			struct neighbor_info *nb_info = &scan_result->nb_info[j];
			err("BSSID:"MACSTR"\n", MAC2STR(nb_info->bssid));
			err("SSID : %s: %d\n RCPI = %d\n ", nb_info->ssid, nb_info->ssid_len, nb_info->RCPI);
			err("BW: %d %s\n", nb_info->ch_bw_len, nb_info->ch_bw);
			info("CUPresent=%d, CU=%d, STACNT=%d\n", nb_info->cu_stacnt_present,nb_info->cu, nb_info->sta_cnt);
		}
		err("RESULT : %d--------------------->END\n\n", i);
		buf += sizeof(struct net_opt_scan_result_event)+ (scan_result->neighbor_num*sizeof(struct neighbor_info));
	}
}

#ifdef MAP_R2

void topo_srv_parse_wapp_radio_metric_event(struct mapd_global * global,
	struct radio_metrics_info *minfo, unsigned short from)
{
	struct own_1905_device *ctx = &global->dev;
	if (from) {
		//mapd_hexdump(MSG_OFF, "WAPP_RADIO_METRICS_INFO", wapp_event->buffer, wapp_event->length);
		//info("receive radio metrics info event from=%d", from);

		insert_new_radio_metrics_info(ctx, minfo);
	}
}

void topo_srv_parse_wapp_all_sta_extended_link_metrics(struct mapd_global * global,
	struct ext_sta_link_metrics *metrics)
{
	struct own_1905_device *ctx = &global->dev;

	//mapd_hexdump(MSG_INFO,"WAPP_ALL_ASSOC_STA_EXTENDED_LINK_METRICS", wapp_event->buffer, wapp_event->length);
	debug("receive associated all assoc sta link metrics event");
	insert_new_ext_link_metrics_info(ctx, metrics);
}

void topo_srv_parse_wapp_one_sta_extended_link_metrics(struct mapd_global * global,
	struct ext_link_metrics *metrics)
{
	struct own_1905_device *ctx = &global->dev;

	//mapd_hexdump(MSG_OFF,"WAPP_ONE_ASSOC_STA_EXTENDED_LINK_METRICS", wapp_event->buffer, wapp_event->length);
	debug("receive one associated sta link metrics event");
	//metrics = (struct ext_link_metrics *)wapp_event->buffer;
	update_one_sta_link_ext_metrics_info(ctx, metrics);
}


void topo_srv_parse_wapp_dissassoc_stats(struct mapd_global * global,
	struct client_disassociation_stats_event *evt, unsigned short len, unsigned short from)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	struct traffic_stats_db *traffic_stats = NULL;
	struct stats_db *stats = NULL;
	struct stat_info *stat_buf = NULL;
	struct _1905_map_device *dev = topo_srv_get_controller_device(ctx);
	mapd_hexdump(MSG_OFF, "WAPP_DISASSOC_STATS_EVT",evt, len);
	SLIST_FOREACH(traffic_stats, &ctx->metric_entry.traffic_stats_head, traffic_stats_entry) {
		err("WAPP_DISASSOC_STATS_EVT");
		SLIST_FOREACH(stats, &traffic_stats->stats_head, stats_entry) {
			err("WAPP_DISASSOC_STATS_EVT");
			if (os_memcmp(stats->mac, evt->mac_addr, ETH_ALEN) == 0) {
				err("WAPP_DISASSOC_STATS_EVT");
				stat_buf = (struct stat_info *)os_malloc(sizeof(struct stat_info));
				os_memcpy(stat_buf, stats, sizeof(struct stat_info));
				map_1905_Send_disassoc_sta_stats_message(_1905_ctrl, dev->_1905_info.al_mac_addr, evt->reason_code, stat_buf);
				os_free(stat_buf);
				break;
			}
		}
	}
}

//WAPP_SCAN_CAPAB
void topo_srv_parse_wapp_scan_capab(struct mapd_global * global,
	struct channel_scan_capab *scan_cap, unsigned short cap_len)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	int i=0;
	err("------------->");
	excess_debug("receive channel scan capability event");

	if (topo_srv_update_ch_scan_cap(ctx,cap_len, scan_cap) < 0) {
		err("update scan cap fail");
	}
	// to be done for each radio separately
	mapd_hexdump(MSG_OFF,"ScanCapab",(u8 *)scan_cap, cap_len);
	for(i=0; i< scan_cap->radio_num;i++) {
		map_1905_Set_Channel_Scan_Cap(_1905_ctrl, &scan_cap->radio_scan_params[i]);
	}
}


void topo_srv_parse_wapp_r2_ap_cap(struct mapd_global * global,
	struct ap_r2_capability *r2_ap_capab, unsigned short cap_len)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	err("------------->");
	excess_debug("receive R2 AP capability event");

	if (topo_srv_update_r2_ap_cap(ctx, cap_len, r2_ap_capab) < 0) {
		err("update r2 ap capab fail");
	}
	mapd_hexdump(MSG_OFF,"R2ApCap",(u8 *)r2_ap_capab, cap_len);
		map_1905_Set_R2_AP_Cap(_1905_ctrl, r2_ap_capab);
}

/*WAPP_CHANNEL_SCAN_REPORT*/


void dump_ch_scan_rep_r2(struct channel_scan_report_event *scan_rep_evt)
{
	int i,j;
	u8 * buf = NULL;
	// print timestamp
	// TODO:
	buf = (u8 *)scan_rep_evt->scan_result;
	for(i=0; i< scan_rep_evt->scan_result_num;i++) {
		struct scan_result_event *scan_result = (struct scan_result_event *)buf;
		always("RESULT : %d--------------------->START\n", i);
		mapd_hexdump(MSG_OFF,"RadioID:",scan_result->radio_id,ETH_ALEN);
		always("OperClass = %d\n Channel=%d\n ScanStatus=%d \n",
				scan_result->oper_class,
				scan_result->channel,
				scan_result->scan_status);
		always("TimeStamp = %d: %s\n", scan_result->timestamp_len,
								scan_result->timestamp);
		always("Utilization = %d,\n noise=%d\n", scan_result->utilization,
						scan_result->noise);
		always("AggScanDur=%d, ScanType=%d\n", scan_result->agg_scan_duration, scan_result->scan_type);
		always("Neighbor List %d-------------->\n", scan_result->neighbor_num);
		for (j=0;j<scan_result->neighbor_num;j++) {
			struct neighbor_info *nb_info = &scan_result->nb_info[j];
			always("BSSID:"MACSTR"\n", MAC2STR(nb_info->bssid));
			always("SSID : %s: %d\n RCPI = %d\n ", nb_info->ssid, nb_info->ssid_len, nb_info->RCPI);
			always("BW: %d %s\n", nb_info->ch_bw_len, nb_info->ch_bw);
			always("CUPresent=%d, CU=%d, STACNT=%d\n", nb_info->cu_stacnt_present,nb_info->cu, nb_info->sta_cnt);
		}

		always("RESULT : %d--------------------->END\n\n", i);
		buf += sizeof(struct scan_result_event)+ scan_result->neighbor_num*sizeof(struct neighbor_info);
	}

}

void hex_dump_dbg(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen)
{
	unsigned char *pt;
	int x;


	pt = pSrcBufVA;
	printf("%s: %p, len = %d\n",str,  pSrcBufVA, SrcBufLen);

	for (x=0; x<SrcBufLen; x++) {
		if (x % 16 == 0)
			printf("0x%04x : ", x);
		printf("%02x ", ((unsigned char)pt[x]));
		if (x%16 == 15) printf("\n");
	}
	printf("\n");
}

void topo_srv_parse_wapp_ch_scan_report(struct mapd_global * global,
	struct net_opt_scan_report_event *scan_rep_evt, unsigned short rep_len)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	struct _1905_map_device *dev = topo_srv_get_controller_device(ctx);

	if (dev == NULL) {
		err("1905 dev is NULL");
		return;
	}

	if(ctx->device_role == DEVICE_ROLE_CONTROLLER) {
		err("Scan is done at controller %d", ctx->ch_planning_R2.ch_plan_state);
		if((ctx->ch_planning_R2.ch_plan_enable == TRUE &&
			ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_SCAN_ONGOING)||
			ctx->user_triggered_scan == TRUE) {
			//dump_net_opt_off_ch_scan_rep(scan_rep_evt);//test function
			ch_planning_handle_controller_scan_result(ctx,scan_rep_evt);
			if(ctx->user_triggered_scan == TRUE) {
				ctx->user_triggered_scan = 0;
				handle_task_completion(ctx);
			}
			return;
		}
	}
	struct net_opt_scan_result_event *scan_rep_evt_temp = &scan_rep_evt->scan_result[0];
	u8 *rep_evt = (u8 *)scan_rep_evt_temp;
	struct scan_result_lib *scan_result = os_zalloc(rep_len);
	if (scan_result == NULL) {
		err("Malloc failure");
		return;
	}
	u8 *scan_info = (u8 *)scan_result;
	u8 *scan_result_temp = (u8 *)scan_result;
	u8 buf[rep_len];
	u8 tlv_size = 0;
	int i,j;
	struct ch_util_lib *ch_util = NULL;
	u8 *ch_util_temp = (u8 *)ch_util;
	struct tlv_head *tlv = (struct tlv_head*)buf;
	unsigned char* p = NULL;

	os_memset(buf, 0, rep_len);
	tlv->tlv_type = TLV_802_11_VENDOR_SPECIFIC;
	os_memcpy(tlv->oui, MTK_OUI, OUI_LEN);
	tlv->func_type = FUNC_VENDOR_CHANNEL_UTIL_RSP;
	tlv->tlv_len = OUI_LEN + 4;//doing +1 for func type addition to len
	p = (unsigned char *)tlv + sizeof(struct tlv_head);
	ch_util = (struct ch_util_lib *)p;

	for (i = 0;i < scan_rep_evt->scan_result_num;i++) {
		//err("scan_result %p , scan_rep_evt_temp %p", scan_result,scan_rep_evt_temp);
		os_memcpy(scan_result->identifier, scan_rep_evt_temp->radio_id, ETH_ALEN);
		scan_result->op_class = scan_rep_evt_temp->oper_class;
		scan_result->channel = scan_rep_evt_temp->channel;
		scan_result->scan_status = scan_rep_evt_temp->scan_status;
		scan_result->ts_len = scan_rep_evt_temp->timestamp_len;
		os_memcpy(scan_result->ts_str , scan_rep_evt_temp->timestamp,
			scan_rep_evt_temp->timestamp_len);
		scan_result->utilization = scan_rep_evt_temp->utilization;
		scan_result->noise = scan_rep_evt_temp->noise;
		scan_result->agg_scan_dur = scan_rep_evt_temp->agg_scan_duration;
		scan_result->scan_type = scan_rep_evt_temp->scan_type;
		scan_result->neighbor_cnt = scan_rep_evt_temp->neighbor_num;
		err("channel %d neigh num %d ",scan_result->channel, scan_result->neighbor_cnt);
		for (j = 0; j < scan_result->neighbor_cnt; j++) {
			os_memcpy(&scan_result->neighbor[j], &scan_rep_evt_temp->nb_info[j],
				sizeof(struct neighbor_info));
		}

		scan_result_temp = (u8 *)scan_result;
		scan_result_temp += sizeof(struct scan_result_lib) +
			(scan_rep_evt_temp->neighbor_num * sizeof(struct neighbor_info));
		scan_result = (struct scan_result_lib *)scan_result_temp;
		//err("new scan_result %p ",scan_result);


		ch_util->ch_num = scan_rep_evt_temp->channel;
		ch_util->edcca = scan_rep_evt_temp->edcca;
		tlv->tlv_len += sizeof(struct ch_util_lib);
		ch_util_temp = (u8 *)ch_util;
		ch_util_temp += sizeof(struct ch_util_lib);
		ch_util = (struct ch_util_lib *)ch_util_temp;
		//err(" ch_util %p , size is %d ", ch_util, sizeof(struct ch_util_lib));

		rep_evt = (u8 *)scan_rep_evt_temp;
		rep_evt += sizeof(struct net_opt_scan_result_event) +
			(scan_rep_evt_temp->neighbor_num * sizeof(struct neighbor_info));
		scan_rep_evt_temp = (struct net_opt_scan_result_event *)rep_evt;
	}
	err(" ");
	tlv_size = tlv->tlv_len + 3;/*adding 3 bytes for TLV TYPE and TLV Len */
	tlv->tlv_len = host_to_be16(tlv->tlv_len);
	//err("tlv->tlv_len %d, tlv_size %d ", tlv->tlv_len,tlv_size);
	//dump_net_opt_off_ch_scan_rep(scan_rep_evt);//test function
	err(" going to send to 1905 ");

	map_1905_Send_Channel_Scan_Report_Message(_1905_ctrl,
			(char *)dev->_1905_info.al_mac_addr,
			scan_rep_evt->timestamp.timestamp_len, scan_rep_evt->timestamp.timestamp,
			scan_rep_evt->scan_result_num,(u8 *)scan_info,
			(u8 *)buf, tlv_size);
	err(" sent to 1905 already");
	//os_free(scan_result);
	os_free(scan_info);
	err("os free done ");
}

/*WAPP_ASSOC_STATUS_NOTIFICATION*/
void topo_srv_parse_wapp_assoc_status_notif(struct mapd_global * global,
	struct assoc_notification *assoc)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_map_device *own_dev = NULL;
	struct bss_info_db *bss;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	int i, j;
	if (!assoc)
		return;
	own_dev = topo_srv_get_1905_device(ctx, NULL);
	for (i = 0;i < assoc->notification_tlv_num;i++) {
		for (j = 0;j < assoc->tlv[i].bssid_num;j++) {
			SLIST_FOREACH(bss, &own_dev->first_bss, next_bss) {
				if (os_memcmp(assoc->tlv[i].tlv_status[j].bssid, bss->bssid, ETH_ALEN)== 0) {
					if (assoc->tlv[i].tlv_status[j].status == 0)
						bss->status = 1;
					else
						bss->status = 0;
				}
			}
		}
	}
	err("Raghav");
	map_1905_Send_Assoc_Status_Notification_Message(_1905_ctrl,
							NULL,
							assoc);
}
void insert_tunneled_own_info(struct own_1905_device *ctx, struct tunneled_msg * tunneled)
{
	struct mapd_global *global = ctx->back_ptr;
	struct _1905_map_device *own_dev = NULL;
	struct client *cli = NULL;

	own_dev = topo_srv_get_1905_device(ctx, NULL);
	dl_list_for_each(cli, &global->dev.sta_seen_list, struct client, sta_seen_entry) {
		if (os_memcmp(cli->mac_addr, tunneled->sta_mac, ETH_ALEN) == 0)
			break;
	}
	if (cli && tunneled->proto_type < MAX_TUNNEL_TYPE) {
		if (cli->tunnel_info[tunneled->proto_type].tunneled_payload != NULL &&
			cli->tunnel_info[tunneled->proto_type].tunnel_payload_len != tunneled->tlv[0].payload_len) {
			os_free(cli->tunnel_info[tunneled->proto_type].tunneled_payload);
			cli->tunnel_info[tunneled->proto_type].tunneled_payload =
			os_zalloc(tunneled->tlv[0].payload_len);
			err("\n");
			if (cli->tunnel_info[tunneled->proto_type].tunneled_payload != NULL)
				cli->tunnel_info[tunneled->proto_type].tunnel_payload_len = tunneled->tlv[0].payload_len;
			else
				cli->tunnel_info[tunneled->proto_type].tunnel_payload_len = 0;
		} else {
			err("\n");
			cli->tunnel_info[tunneled->proto_type].tunneled_payload = os_zalloc(tunneled->tlv[0].payload_len);
			if (cli->tunnel_info[tunneled->proto_type].tunneled_payload != NULL)
				cli->tunnel_info[tunneled->proto_type].tunnel_payload_len = tunneled->tlv[0].payload_len;
			else
				cli->tunnel_info[tunneled->proto_type].tunnel_payload_len = 0;
		}

		if (cli->tunnel_info[tunneled->proto_type].tunneled_payload != NULL) {
			os_memcpy(cli->tunnel_info[tunneled->proto_type].tunneled_payload, tunneled->tlv[0].payload, cli->tunnel_info[tunneled->proto_type].tunnel_payload_len);
			mapd_hexdump(MSG_OFF, "Own tunneled info", cli->tunnel_info[tunneled->proto_type].tunneled_payload, cli->tunnel_info[tunneled->proto_type].tunnel_payload_len);
		}
	}
}
/*WAPP_TUNNELED_MESSAGE*/
void topo_srv_parse_wapp_tunneled_msg(struct mapd_global * global,
	struct tunneled_msg * tunneled)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	struct _1905_map_device *dev = topo_srv_get_controller_device(ctx);

	if(dev != NULL) {
		if (ctx->device_role != DEVICE_ROLE_CONTROLLER &&
			ctx->map_version == DEV_TYPE_R2) {
			printf("\n prak tunnel msg final");
			//insert_tunneled_own_info(ctx, tunneled);
			map_1905_Send_Tunneled_Message (_1905_ctrl,
							(char *)dev->_1905_info.al_mac_addr,
							tunneled);
		}
	}
}

#ifdef DFS_CAC_R2
/*WAPP_CAC_CAPAB*/
void topo_srv_parse_wapp_cac_capab(struct mapd_global * global,
	struct cac_capability *cac_cap, unsigned short cap_len)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	//struct _1905_map_device *dev = topo_srv_get_controller_device(ctx);

	if (topo_srv_update_cac_cap(ctx, cap_len, cac_cap) < 0) {
		err("update cac cap fail");
	}
	// to be done for each radio separately
	mapd_hexdump(MSG_OFF,"CACCapab",(u8 *)cac_cap, cap_len);
	map_1905_Set_CAC_Cap(_1905_ctrl, cac_cap, cap_len);
}
#endif
#endif

void mark_radar_chn_non_operable (struct own_1905_device *ctx, unsigned char channel)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	struct radio_info_db *radio_info = NULL;
	struct radio_ch_prefer *ch_prefer;
	struct prefer_info_db *prefer_db = NULL;
	struct prefer_info_db *prefer_info_tmp = NULL;
	struct prefer_info_db *ptr = NULL;
	unsigned char preference = 0;
	unsigned char reason = 0x7;
	unsigned char num = 0;
	int i = 0;

	radio_info = topo_srv_get_radio(dev, NULL);
	if (!radio_info)
		return;
	prefer_info_tmp = os_zalloc(sizeof(*prefer_info_tmp));
	if (!prefer_info_tmp)
		return;
	//!  check each radio
	while (radio_info) {
		num = 0;
		ch_prefer = &radio_info->chan_preferance;

		SLIST_FOREACH(prefer_db,
			&radio_info->chan_preferance.prefer_info_head, prefer_info_entry) {
			for(i = 0; i < prefer_db->ch_num; i++) {
				if (prefer_db->ch_list[i] == channel) {
					prefer_db->ch_list[i] = prefer_db->ch_list[i+1];
					prefer_info_tmp[num].op_class = prefer_db->op_class;
					prefer_info_tmp[num].ch_num = 1;
					prefer_info_tmp[num].ch_list[0] = channel;
					prefer_info_tmp[num].perference = preference;
					prefer_info_tmp[num].reason = reason;
					num++;
					ptr = os_realloc(prefer_info_tmp, (num + 1)*sizeof(*prefer_info_tmp));
					if (!ptr) {
						if (prefer_info_tmp)
							os_free(prefer_info_tmp);
						return;
					}
					prefer_info_tmp = ptr;
				}
				if ((prefer_db->ch_list[i] == prefer_db->ch_list[i-1])
					&& i+1 != MAX_CH_NUM) {
					prefer_db->ch_list[i] = prefer_db->ch_list[i+1];
				}
			}
		}
		if (num) {
			radio_info->chan_preferance.op_class_num += num;
			for (i = 0; i < num; i++)
				SLIST_INSERT_HEAD(&ch_prefer->prefer_info_head,
				&prefer_info_tmp[i],
				prefer_info_entry);

			ch_planning_remove_ch_from_prefered_list(
				ctx,
				channel,
				radio_info,
				channel > 14);

			ch_planning_add_ch_to_prefered_list(ctx,
				channel,
				radio_info,
				channel > 14,
				preference,
				reason);
		}
		radio_info = topo_srv_get_next_radio(dev,
			radio_info);
	}
}
void topo_srv_parse_wapp_cac_completion_report(struct mapd_global * global,
	struct msg * wapp_event, unsigned short rep_len)
{
	struct own_1905_device *ctx = &global->dev;
	u16 report_len;
	int i;
	struct cac_completion_report * report = NULL;
#if 0
	struct os_reltime rem_time = {0};
#endif
#ifdef DFS_CAC_R2
	struct cac_status_report * status_report = NULL;
#endif
	struct _1905_map_device *dev = NULL;
	struct _1905_map_device *own_dev = NULL;
	struct radio_info_db *radio = NULL;

	dev = topo_srv_get_controller_device(ctx);
	if (dev == NULL) {
		err("controller dev is NULL");
		goto DONE;
	}

	own_dev = topo_srv_get_1905_device(&global->dev, NULL);
	if (own_dev == NULL) {
		err("1905 dev is NULL");
		goto DONE;
	}

	report = (struct cac_completion_report *)wapp_event->buffer;
	radio = topo_srv_get_radio_by_band(own_dev, report->cac_completion_status[0].channel);
	if (radio == NULL) {
		err("radio is NULL");
		goto DONE;
	}

	report_len = sizeof(struct cac_completion_report) + (report->radio_num* sizeof(struct cac_completion_status));
	for(i=0;i<report->radio_num;i++)
		report_len += (report->cac_completion_status[i].op_class_num *sizeof(struct cac_completion_opcap));
#ifdef DFS_CAC_R2
	if (rep_len - report_len > 0)
		status_report = (struct cac_status_report *)&wapp_event->buffer[report_len];
#endif
#ifdef MAP_R2
	err("CAC on channel: %u\n", ctx->ch_planning_R2.CAC_on_channel);
	if (ctx->device_role == DEVICE_ROLE_AGENT) {
		if (report->cac_completion_status[0].channel == ctx->ch_planning_R2.CAC_on_channel) {
			err(" ");
			ctx->ch_planning_R2.CAC_on_channel = 0;
			ctx->ch_planning_R2.cac_ongoing = 0;
		}
	}
#endif
	err("cac comp status: %d, channel: %d\n", report->cac_completion_status[0].cac_status,
				report->cac_completion_status[0].channel);
	switch (report->cac_completion_status[0].cac_status) {
		case CAC_SUCCESSFUL:
			if (radio->cac_enable != CAC_NOT_REQUIRED) {
				radio->cac_enable = CAC_SUCCESSFUL;
				radio->cac_channel = 0;
				ctx->cac_enable = 0;
				if(dev != NULL) {
					if (ctx->device_role != DEVICE_ROLE_CONTROLLER) {
#ifdef DFS_CAC_R2
						if (ctx->map_version == DEV_TYPE_R2)
							_1905_update_channel_pref_report(ctx, report, status_report);
						else
							_1905_update_channel_pref_report(ctx, report, NULL);
#else
						_1905_update_channel_pref_report(ctx, report);
#endif

					}else {
#ifdef DFS_CAC_R2
						ch_planning_handle_cac_success_for_cont(global, radio, report);
#endif
#if 0
						eloop_get_remaining_timeout(&rem_time,trigger_net_opt,ctx,NULL);
						if (rem_time.sec < ctx->network_optimization.post_cac_trigger_time) {
							eloop_cancel_timeout(trigger_net_opt, ctx, NULL);
							network_opt_reset(global);
							eloop_register_timeout(ctx->network_optimization.post_cac_trigger_time, 0, trigger_net_opt, ctx, NULL);
						}
#endif
					}
				}
			} else {
				radio->cac_enable = CAC_SUCCESSFUL;
				radio->cac_channel = 0;
				ctx->cac_enable = 0;
			}
			break;
		case RADAR_DETECTED:
			err("Radar detected\n");
			if (ctx->device_role != DEVICE_ROLE_CONTROLLER) {
				err("RADAR found, Send Channel preference report to controller\n");
				mark_radar_chn_non_operable(ctx, report->cac_completion_status[0].channel);
#ifdef DFS_CAC_R2
				if (ctx->map_version == DEV_TYPE_R2)
					_1905_update_channel_pref_report(ctx, report, status_report);
				else
					_1905_update_channel_pref_report(ctx, report, NULL);
#else
				_1905_update_channel_pref_report(ctx, report);
#endif
			}
			break;
#ifdef DFS_CAC_R2
		case CAC_FAILURE:
			ch_planning_handle_cac_failure(global, radio, report, status_report);
			break;
#endif
		default:
			break;
	}
DONE:
	if (wapp_event != NULL)
		os_free(wapp_event);
#if 0
	mapd_hexdump(MSG_OFF,"CACCompl",(u8 *)report, rep_len);
	printf("\n channel preference report");
	if(dev != NULL) {
		if (ctx->device_role != DEVICE_ROLE_CONTROLLER) {
			printf("\n before send report");
			_1905_update_channel_pref_report(ctx, report);
		}
	}
#endif
}
void topo_srv_parse_wapp_cac_completion_report_wrapper(void *eloop_ctx, void *timeout_ctx)
{
	struct mapd_global *global = (struct mapd_global *)eloop_ctx;
	struct msg *wapp_event = (struct msg *)timeout_ctx;
	err(" ");
	topo_srv_parse_wapp_cac_completion_report(global, wapp_event, wapp_event->length);
}

void topo_srv_parse_wapp_cac_periodic_enable(struct mapd_global * global,
	struct msg *wapp_event, struct own_1905_device *ctx)
{
	struct _1905_map_device *own_dev = NULL;

	global->dev.cac_enable = 1;
	own_dev = topo_srv_get_1905_device(&global->dev, NULL);
	struct radio_info_db *radio = NULL;
	if (!own_dev) {
		err("own 1905 dev is missing\n");
		return;
	}
	radio = topo_srv_get_radio_by_band(own_dev,(char)(*(wapp_event->buffer)));
	radio->cac_channel = (char)(*(wapp_event->buffer));
	radio->cac_enable = (char)(*(wapp_event->buffer + 1));
	radio->cac_timer = (char)(*(wapp_event->buffer + 2));
	if (radio->cac_enable == CAC_NOT_REQUIRED &&
		ctx->current_bh_substate == BH_SUBSTATE_CONNECT_WAIT) {
		eloop_cancel_timeout(issue_connect_timeout_handle, ctx, NULL);
		eloop_register_timeout(0, 0, issue_connect_timeout_handle, ctx, NULL);
	}
}
#ifdef MAP_R2
#ifdef DFS_CAC_R2
void topo_srv_parse_wapp_cac_status_report(struct mapd_global * global,
	struct msg * wapp_event, unsigned short rep_len)
{
	err("WAPP_CAC_STATUS: Len:%d------------->", wapp_event->length);
	struct cac_status_report * report = NULL;
	struct own_1905_device *ctx = &global->dev;
	struct _1905_map_device *dev = topo_srv_get_controller_device(ctx);
	report = (struct cac_status_report *)wapp_event->buffer;
	if(dev != NULL) {
		if (ctx->device_role != DEVICE_ROLE_CONTROLLER)	{
			if(!global->params.Certification) {
				wlanif_get_op_chan_info(ctx->back_ptr);
			}
			if (ctx->map_version == DEV_TYPE_R2)
				_1905_update_channel_pref_report(ctx, NULL, report);
			else
				_1905_update_channel_pref_report(ctx, NULL, NULL);
		}
	}
	if (wapp_event != NULL)
		os_free(wapp_event);
}

#endif
/*WAPP_METRIC_REP_INTERVAL_CAP*/
void topo_srv_parse_wapp_metric_rep_interval(struct mapd_global * global,
	u32 *interval)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_context *_1905_ctrl = global->_1905_ctrl;
	map_1905_Send_Metric_collection_interval_cap(_1905_ctrl, interval);
	topo_srv_update_metric_rep_intv_cap(ctx, interval);
}
#endif
void topo_srv_parse_ch_list_dfs_info(struct mapd_global * global,
	u8 *wapp_buf, unsigned short len)
{
	struct own_1905_device *ctx = &global->dev;
	struct chnList *ch_list;
	u8 *buf = wapp_buf;
	u8 i =0;
	for (i = 0; i <16; i++) {
		ch_list = (struct chnList *)buf;
		ctx->dfs_info_ch_list[i].channel = ch_list->channel;
		ctx->dfs_info_ch_list[i].pref = ch_list->pref;
		ctx->dfs_info_ch_list[i].cac_timer = ch_list->cac_timer;
		debug("ch %d, ,pref 0x%x, cac_timer %d", ch_list->channel, ch_list->pref, ch_list->cac_timer);
		buf += sizeof(struct chnList);
	}
}
#ifdef MAP_R2
void topo_srv_parse_r2_mbo_sta_non_pref_list(struct mapd_global * global,
	unsigned char *buf, unsigned short len)
{
	struct client *cli = NULL;
	struct non_pref_ch *client_pref = NULL;
	unsigned char *tmp_buf = buf;
	int length = 0, i = 0;

	debug("receive R2 MBO STA non pref ch list");
	//mapd_hexdump(MSG_OFF, "MAPD STA pref", buf, len);

	cli = client_db_get_client_from_sta_mac(global, tmp_buf);
	if(!cli) {
		debug("Client entry is null");
		return;
	}
	tmp_buf += ETH_ALEN;
	length = len - ETH_ALEN;
	length /= sizeof(struct non_pref_ch);
	for (i = 0;i < length; i++) {
		client_pref = (struct non_pref_ch *)tmp_buf;
		cli->np_channels[i] = client_pref->ch;
		cli->np_pref = client_pref->pref;
		cli->np_reason = client_pref->reason_code;
		tmp_buf += sizeof(struct non_pref_ch);
	}
}

#endif
void topo_srv_parse_wts_config(struct mapd_global * global,
	struct set_config_bss_info *bss_config, unsigned short config_length)
{
	os_memset(global->dev.bss_config,0, sizeof(struct set_config_bss_info) * MAX_SET_BSS_INFO_NUM);
	os_memcpy(global->dev.bss_config, bss_config, config_length);
	global->dev.need_to_update_wts = 0;
}
