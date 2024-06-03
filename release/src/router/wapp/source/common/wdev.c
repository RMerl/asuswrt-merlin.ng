/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * Copyright  (C) 2019-2020  MediaTek Inc. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdlib.h>
#include <stdio.h>
#include "wdev.h"
#include "driver_wext.h"
#include "interface.h"
#include "wps.h"
#ifdef DPP_SUPPORT
#include "dpp/dpp_wdev.h"
#include "gas_query.h"
#include "gas_server.h"
#endif /*DPP_SUPPORT*/

extern unsigned short prev_1905_msg;
struct wapp_dev* wapp_dev_list_lookup_by_radio(struct wifi_app *wapp, char* ra_identifier)
{
	struct wapp_dev *wdev, *target_wdev = NULL;
	unsigned char wdev_identifier[ETH_ALEN];
	struct dl_list *dev_list;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev && wdev->radio) {			
			MAP_GET_RADIO_IDNFER(wdev->radio, wdev_identifier);
			if(!os_memcmp(wdev_identifier, ra_identifier, ETH_ALEN) &&
				(wdev->dev_type == WAPP_DEV_TYPE_AP)) {
					target_wdev = wdev;
					break;
			}
		}
	}

	return target_wdev;
}

/* 
should lookup wdev by addr and wdev type, 
due to WDS has the same addr with AP main inf 
*/
struct wapp_dev* wapp_dev_list_lookup_by_mac_and_type(struct wifi_app *wapp, const u8 *mac_addr, const u8 wdev_type)
{
	struct wapp_dev *wdev, *target_wdev = NULL;
	struct dl_list *dev_list;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev && (os_memcmp(wdev->mac_addr, mac_addr, MAC_ADDR_LEN) == 0) && wdev->dev_type == wdev_type) {
			target_wdev = wdev;
			break;
		}
	}

	return target_wdev;
}

struct wapp_dev* wapp_dev_list_lookup_by_ifindex(struct wifi_app *wapp, const u32 ifindex)
{
	struct wapp_dev *wdev, *target_wdev = NULL;
	struct dl_list *dev_list;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev && wdev->ifindex == ifindex) {
			target_wdev = wdev;
			break;
		}
	}

	return target_wdev;
}

struct wapp_dev* wapp_dev_list_lookup_by_ifname(struct wifi_app *wapp, const char *ifname)
{
	struct wapp_dev *wdev, *target_wdev = NULL;
	struct dl_list *dev_list;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev && strcmp(ifname, (char *) wdev->ifname) == 0) {
			target_wdev = wdev;
			break;
		}
	}

	return target_wdev;
}

int wapp_dev_create(
	struct wifi_app *wapp,
	char 			*iface,
	u32				if_idx,
	u8				*mac_addr)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !iface || !mac_addr) {
		//DBGPRINT(RT_DEBUG_OFF, "warning: entry is not NULL!\n");
		return WAPP_INVALID_ARG;
	}

	wdev = os_zalloc(sizeof(struct wapp_dev));

	if (!wdev) {
		//DBGPRINT(RT_DEBUG_OFF, "Creating entry failed!\n");
		return WAPP_RESOURCE_ALLOC_FAIL;
	}

	os_memset(wdev, 0, sizeof(struct wapp_dev));

	dl_list_init(&wdev->list);
	os_memcpy(wdev->mac_addr, mac_addr, MAC_ADDR_LEN);
	os_memcpy(wdev->ifname, iface, IFNAMSIZ);
	wdev->ifindex = if_idx;
	dl_list_add_tail(&wapp->dev_list, &wdev->list);

	DBGPRINT(RT_DEBUG_TRACE, 
		"new wdev: %s if_idx = %u, mac_addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
		wdev->ifname, wdev->ifindex, PRINT_MAC(wdev->mac_addr));

	wapp_drv_support_version_check(wapp, wdev->ifname);

	wapp_query_wdev(wapp, iface);

	return WAPP_SUCCESS;
}

int wapp_dev_del(
struct wifi_app	*wapp,
struct wapp_dev	*wdev,
const u8		*mac_addr,
const u32		wdev_type)
{
	struct wapp_dev *del_wdev = NULL;
	//struct wdev_ops *ops = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp) {
		return WAPP_INVALID_ARG;
	}

	if (wdev) {
		del_wdev = wdev;
		goto entry_del;
	}

	if (mac_addr) {
		del_wdev = wapp_dev_list_lookup_by_mac_and_type(wapp, mac_addr, wdev_type);
		if (del_wdev)
			goto entry_del;
		else
			return WAPP_LOOKUP_ENTRY_NOT_FOUND;
	}

	/* at least one of sta or mac_addr should not be NULL */
	return WAPP_UNEXP;

entry_del:

	if (del_wdev->p_dev != NULL) {
		if (wdev && wdev->ops->wdev_del)
			wdev->ops->wdev_del(wapp, wdev);
		else
			DBGPRINT_RAW(RT_DEBUG_ERROR, "Error! No wdev_del ops.\n");
		return WAPP_NOT_INITIALIZED;
	}

	dl_list_del(&del_wdev->list);
	os_free(del_wdev);

	return WAPP_SUCCESS;
}

int wapp_clear_dev_list(
struct wifi_app	*wapp)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp) {
		return WAPP_INVALID_ARG;
	}

	if (!dl_list_empty(&wapp->dev_list)) {
		struct wapp_dev *wdev, *wdev_tmp;

		dl_list_for_each_safe(wdev, wdev_tmp,
						&wapp->dev_list, struct wapp_dev, list) {
			wapp_dev_del(wapp, wdev, wdev->mac_addr, wdev->dev_type);
		}
	}

	return WAPP_SUCCESS;
}

int wapp_show_devinfo(struct wapp_dev *wdev)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wdev) {
		return WAPP_INVALID_ARG;
	}
	
	DBGPRINT_RAW(RT_DEBUG_OFF, "dev_type: %u\n", wdev->dev_type);
	DBGPRINT_RAW(RT_DEBUG_OFF, "ifindex: %u\n", wdev->ifindex);
	DBGPRINT_RAW(RT_DEBUG_OFF, "ifname: %s\n", wdev->ifname);
	if (wdev->radio)
	DBGPRINT_RAW(RT_DEBUG_OFF, "radio index: %u\n", wdev->radio->index);
	DBGPRINT_RAW(RT_DEBUG_OFF, "mac_addr: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(wdev->mac_addr));

#ifdef MAP_SUPPORT
	DBGPRINT_RAW(RT_DEBUG_OFF, "i_am_fh_bss: %d\n", wdev->i_am_fh_bss);
	DBGPRINT_RAW(RT_DEBUG_OFF, "i_am_bh_bss: %d\n", wdev->i_am_bh_bss);
#endif /* MAP_SUPPORT */

#if 0
	DBGPRINT_RAW(RT_DEBUG_OFF, "bssid: %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(sta->bssid));
	DBGPRINT_RAW(RT_DEBUG_OFF, "cell_data_cap: %u\n", sta->cell_data_cap);
	DBGPRINT_RAW(RT_DEBUG_OFF, "no_none_pref_ch: %u\n", sta->no_none_pref_ch);
	DBGPRINT_RAW(RT_DEBUG_OFF, "trans_reason: %u\n", sta->trans_reason);
	DBGPRINT_RAW(RT_DEBUG_OFF, "disassoc_imnt: %u\n", sta->disassoc_imnt);
	DBGPRINT_RAW(RT_DEBUG_OFF, "akm: 0x%x\n", sta->akm);
	DBGPRINT_RAW(RT_DEBUG_OFF, "cipher: 0x%x\n", sta->cipher);
	DBGPRINT_RAW(RT_DEBUG_OFF, "non_pref_ch_list: (ch/pref/reason)\n");
	if (!dl_list_empty(&sta->non_pref_ch_list))
		dl_list_for_each(npc_entry, &sta->non_pref_ch_list, struct non_pref_ch_entry, list) {
		DBGPRINT_RAW(RT_DEBUG_OFF, "%u/%u/%u ", npc_entry->npc.ch, npc_entry->npc.pref, npc_entry->npc.reason_code);
	} else
		DBGPRINT_RAW(RT_DEBUG_OFF, "ch list is empty: %u", sta->cell_data_cap);
	DBGPRINT_RAW(RT_DEBUG_OFF, "\n");
#endif
	DBGPRINT_RAW(RT_DEBUG_OFF, "---\n");
	return WAPP_SUCCESS;
}

void wapp_show_dev_list(struct wifi_app *wapp)
{
	struct wapp_dev *wdev_tmp;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	DBGPRINT_RAW(RT_DEBUG_OFF, "wdev_list:\n");
	DBGPRINT_RAW(RT_DEBUG_OFF, "==============================\n");

	if (!dl_list_empty(&wapp->dev_list)){
		dl_list_for_each(wdev_tmp, &wapp->dev_list, struct wapp_dev, list)
			wapp_show_devinfo(wdev_tmp);
	}

	return;
}

void wdev_query_rsp_handle(struct wifi_app *wapp, wapp_event_data *event_data)
{
	wapp_dev_info *dev_info;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	dev_info = &event_data->dev_info;

	DBGPRINT_RAW(RT_DEBUG_OFF,
				"wdev_query_rsp_handle: dev_type = %u, mac = %02x:%02x:%02x:%02x:%02x:%02x, ifname = %s\n",
				dev_info->dev_type,
				PRINT_MAC(dev_info->mac_addr),
				dev_info->ifname);

	switch (dev_info->dev_type)
	{
		case WAPP_DEV_TYPE_AP:
			wdev_ap_create(wapp, dev_info);
		break;
		
		case WAPP_DEV_TYPE_STA:
		case WAPP_DEV_TYPE_APCLI:
			wdev_sta_create(wapp, dev_info);
		break;
		
		default:
		break;
	}
#ifdef DPP_SUPPORT
	dpp_conf_init(wapp, dev_info);
#endif /* DPP_SUPPORT */
}

void wdev_ht_cap_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_ht_cap *ht_cap_rcvd)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_ht_cap *ht_cap;
		ht_cap = &ap->ht_cap;
		os_memcpy(ht_cap, ht_cap_rcvd, sizeof(wdev_ht_cap));
		DBGPRINT_RAW(RT_DEBUG_OFF,
				"ht_cap (%u):\n"
				"\t tx_stream = %u, rx_stream = %u, sgi_20 = %u, sgi_40 = %u, ht_40 = %u\n",
				ifindex,
				ht_cap->tx_stream,
				ht_cap->rx_stream,
				ht_cap->sgi_20,
				ht_cap->sgi_40,
				ht_cap->ht_40);
	}
}


void wdev_vht_cap_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_vht_cap *vht_cap_rcvd)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_vht_cap *vht_cap;
		vht_cap = &ap->vht_cap;
		os_memcpy(vht_cap, vht_cap_rcvd, sizeof(wdev_vht_cap));
		DBGPRINT_RAW(RT_DEBUG_OFF,
				"vht_cap (%u):\n"
				"\t sup_tx_mcs[]= %02x %02x sup_rx_mcs[] = %u %u\n"
				"\t tx_stream = %u, rx_stream = %u\n"
				"\t sgi_80 = %u, sgi_160 = %u\n"
				"\t vht160 = %u, vht8080 = %u\n"
				"\t su_bf = %u, mu_bf = %u\n",
				ifindex,
				vht_cap->sup_tx_mcs[0], vht_cap->sup_tx_mcs[1],
				vht_cap->sup_rx_mcs[0], vht_cap->sup_rx_mcs[1],
				vht_cap->tx_stream,
				vht_cap->rx_stream,
				vht_cap->sgi_80,
				vht_cap->sgi_160,
				vht_cap->vht_160,
				vht_cap->vht_8080,
				vht_cap->su_bf,
				vht_cap->mu_bf);
	}
}

void wdev_misc_cap_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_misc_cap *misc_cap)
{
	struct wapp_dev *wdev = NULL;
	//wdev_misc_cap *misc_cap = &event_data->misc_cap;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	/* TODO: use wdev hooking funcion? */
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;

		ap->max_num_of_cli = misc_cap->max_num_of_cli;
		ap->max_num_of_bss = misc_cap->max_num_of_bss;
		ap->num_of_bss = misc_cap->num_of_bss;
		ap->max_num_of_block_cli = \
			(misc_cap->max_num_of_block_cli < BLOCK_LIST_NUM) ? misc_cap->max_num_of_block_cli : BLOCK_LIST_NUM;

		if (ap->client_table == NULL) {
			wdev_ap_client_table_create(wapp, ap);
		}

		wdev_ap_block_list_init(wapp, ap);

		DBGPRINT_RAW(RT_DEBUG_OFF,
				"misc_cap (%u):\n"
				"\t max_num_of_cli = %u\n"
				"\t max_num_of_bss = %u\n"
				"\t num_of_bss     = %u\n"
				"\t max_num_of_block_cli = %u\n",
				ifindex,
				ap->max_num_of_cli,
				ap->max_num_of_bss,
				ap->num_of_bss,
				ap->max_num_of_block_cli);
	}
}

void wdev_cli_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_sta *sta = NULL;
	struct ap_dev *ap = NULL;
#ifdef MAP_SUPPORT
	struct wapp_sta temp_sta;
#endif

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	/* TODO: use wdev hooking funcion? */
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		wapp_client_info *cli = &event_data->cli_info;
		ap = (struct ap_dev *)wdev->p_dev;
		sta = wdev_ap_client_list_lookup(wapp, ap, cli->mac_addr);
		if (!sta && cli->sta_status== WAPP_STA_CONNECTED)
			wapp_client_create(wapp, ap, &sta);

		if (sta) {
#if 1 //MBO
			dl_list_init(&sta->non_pref_ch_list);
#endif
			if (cli->sta_status == WAPP_STA_CONNECTED) {
				if (sta->sta_status != WAPP_STA_CONNECTED) {
					ap->num_of_assoc_cli++;
#ifdef MAP_R2
					wapp_set_mbo_allow_disallow(wapp);
#endif
				}
				wapp_fill_client_info(wapp, cli, sta);
#ifdef MAP_R2
	//			printf("MAP R2 sta ext params\n");
				sta->ext_sta_metrics.sta_info.last_data_dl_rate = cli->ext_metric_info.sta_info.last_data_dl_rate;
				sta->ext_sta_metrics.sta_info.last_data_ul_rate = cli->ext_metric_info.sta_info.last_data_ul_rate;
				sta->ext_sta_metrics.sta_info.utilization_rx = cli->ext_metric_info.sta_info.utilization_rx;
				sta->ext_sta_metrics.sta_info.utilization_tx = cli->ext_metric_info.sta_info.utilization_tx;
				//printf("dl rate: %d, ul rate: %d\n", sta->ext_sta_metrics.sta_info.last_data_dl_rate, sta->ext_sta_metrics.sta_info.last_data_ul_rate);
				//printf("rx rate: %d, tx rate: %d\n", sta->ext_sta_metrics.sta_info.utilization_rx, sta->ext_sta_metrics.sta_info.utilization_tx);
				//printf("dl rate: %d, ul rate: %d\n", cli->ext_metric_info.sta_info.last_data_dl_rate, cli->ext_metric_info.sta_info.last_data_ul_rate);
				//printf("rx rate: %d, tx rate: %d\n", cli->ext_metric_info.sta_info.utilization_rx, cli->ext_metric_info.sta_info.utilization_tx);
#endif
			}
			else if (cli->sta_status == WAPP_STA_DISCONNECTED) {
				if (sta->sta_status == WAPP_STA_CONNECTED) {
					ap->num_of_assoc_cli--;
#ifdef MAP_R2
					wapp_set_mbo_allow_disallow(wapp);
#endif
				}
				sta->sta_status = WAPP_STA_DISCONNECTED;
				if (sta->beacon_report) {
					free(sta->beacon_report);
					sta->beacon_report = NULL;
				}
			}
#if 0
			DBGPRINT_RAW(RT_DEBUG_OFF, "cli_info: (%u)\n", ifindex);
			wdev_show_wapp_sta_info(sta);
#endif
		}


#ifdef MAP_SUPPORT
		if (!sta) {
			sta = &temp_sta;
			sta->sta_status = WAPP_STA_DISCONNECTED;
		}
		map_send_one_assoc_sta_msg(wapp, sta);
#endif
	}
}


void wdev_cli_list_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	if (!wapp)
		return;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	DBGPRINT_RAW(RT_DEBUG_TRACE, "cli_list_rsp: (%u)\n", ifindex);

	os_get_time(&wdev->cli_list_last_update_time);
#ifdef MAP_SUPPORT
	map_send_assoc_sta_msg(wapp);
#endif

}


void wdev_apcli_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_sta *sta = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		wapp_client_info *cli = &event_data->cli_info;
		sta = (struct wapp_sta *)wdev->p_dev;
		if (sta) {
			wapp_fill_client_info(wapp, cli, sta);
			DBGPRINT_RAW(RT_DEBUG_OFF, "apcli_info: (%u)\n", ifindex);
			wdev_show_wapp_sta_info(sta);
#ifdef MAP_SUPPORT
			map_send_one_assoc_sta_msg(wapp, sta);
#endif
		}
	}
}


void wdev_cli_join_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	unsigned short old_msg;
	struct wapp_sta *sta = NULL;	

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	old_msg = prev_1905_msg;
	/* reset previous 1905 msg type */
	prev_1905_msg = 0;
	wdev_cli_query_rsp_handle(wapp, ifindex, event_data);
#ifdef MAP_SUPPORT
	char buf[MAX_EVT_BUF_LEN];
	wapp_client_info *cli = &event_data->cli_info;
	char assoc_buffer[512] = {0};	


	os_memcpy(&assoc_buffer[0], cli->mac_addr, MAC_ADDR_LEN);	
	
	driver_wext_get_assoc_req_frame(wapp->drv_data, wdev->ifname,assoc_buffer,cli->assoc_req_len);

	sta = wdev_ap_client_list_lookup(wapp, (struct ap_dev *)wdev->p_dev, cli->mac_addr);

	if(sta){
		sta->assoc_req_len = cli->assoc_req_len;
		os_memcpy(sta->assoc_req,&assoc_buffer[0],sta->assoc_req_len);
		DBGPRINT(RT_DEBUG_TRACE, "%s:Got Assoc request\n",__func__);
		hex_dump_dbg("Assoc Req", (UCHAR *)sta->assoc_req,sta->assoc_req_len);		
	}
	
	
	map_send_assoc_cli_msg(wapp, cli->bssid, cli->mac_addr, STA_JOIN, buf);
#endif
	/* restore previous 1905 msg type */
	prev_1905_msg = old_msg;
#ifdef MAP_R2
	wdev_send_tunnel_assoc_req(wapp, ifindex, cli->assoc_req_len, cli->mac_addr, cli->IsReassoc, (u8 *)assoc_buffer);
#endif
}


void wdev_cli_leave_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_sta *sta = NULL;
	struct ap_dev *ap = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	/* TODO: use wdev hooking funcion? */
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		wapp_client_info *cli = &event_data->cli_info;
#ifdef MAP_SUPPORT
		char buf[MAX_EVT_BUF_LEN];
		map_send_assoc_cli_msg(wapp, wdev->mac_addr, cli->mac_addr, STA_LEAVE, buf);
#endif
		ap = (struct ap_dev *)wdev->p_dev;
		sta = wdev_ap_client_list_lookup(wapp, (struct ap_dev *) wdev->p_dev, cli->mac_addr);
		if (sta) {
			if (sta->sta_status == WAPP_STA_CONNECTED) {
				ap->num_of_assoc_cli--;
#ifdef MAP_R2
				wapp_set_mbo_allow_disallow(wapp);
#endif
				sta->sta_status = WAPP_STA_DISCONNECTED;
				if (sta->beacon_report) {
					free(sta->beacon_report);
					sta->beacon_report = NULL;
				}
			}
		}
	}
}
#ifdef MAP_R2
int mbo_set_assoc_disallow( struct wifi_app *wapp, char *iface, int value)
{
	struct mbo_cfg *mbo = wapp->mbo;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__); 

	mbo->assoc_disallow_reason = value;

	/* mbo assoc disallow */
	mbo_param_setting(wapp, (const char *)iface, PARAM_MBO_AP_ASSOC_DISALLOW, wapp->mbo->assoc_disallow_reason);
	
#ifdef MAP_R2
	/* Send assoc status notification to controller/agents */
	map_send_assoc_notification(wapp, (const char *)iface, wapp->mbo->assoc_disallow_reason);
#endif	
	
	return MBO_SUCCESS;
}

void wdev_sta_disassoc_stats_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	/* TODO: use wdev hooking funcion? */
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		wapp_client_info *cli = &event_data->cli_info;
		printf("event cli info: %d\n", event_data->cli_info.disassoc_reason);
		char buf[MAX_EVT_BUF_LEN];
		map_send_sta_disassoc_stats_msg(wapp, wdev->mac_addr, cli, buf);
	}
}
void wapp_set_mbo_allow_disallow(struct wifi_app *wapp)
{
	int tot_cnt = 0;
	struct ap_dev * ap = NULL;
	struct dl_list *dev_list = NULL;
	struct wapp_dev *wdev = NULL;

	dev_list = &wapp->dev_list;
	dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
		if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
			ap = (struct ap_dev *)wdev->p_dev;
			tot_cnt += ap->num_of_assoc_cli;
		}
	}
	if (tot_cnt >= wapp->map->max_client_cnt && wapp->mbo->assoc_disallow_reason == 0) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: MAX no of client cnt reached disallowing\n", __func__);
		wdev = NULL;
		dev_list = &wapp->dev_list;
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
			if (wdev->dev_type == WAPP_DEV_TYPE_AP)
				mbo_set_assoc_disallow(wapp, wdev->ifname, 2);
		}
	} else if (tot_cnt <= wapp->map->max_client_cnt && wapp->mbo->assoc_disallow_reason == 2) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: Reduced no of client, allowing\n", __func__);
		wdev = NULL;
		dev_list = &wapp->dev_list;
		dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
			if (wdev->dev_type == WAPP_DEV_TYPE_AP)
				mbo_set_assoc_disallow(wapp, wdev->ifname, 0);
		}
	}
}
#endif
void wdev_cli_probe_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct probe_info *info;
	wapp_probe_info *probe = &event_data->probe_info;
	int send_pkt_len = 0;
	char* buf = NULL;

	/* TODO varify conf status for controller */
	if ((wapp->is_bs20_attached == FALSE) &&
	    (wapp->map->conf != MAP_CONN_STATUS_CONF)) {
		DBGPRINT(RT_DEBUG_TRACE, "%s: device is unconfigued, ignore probes\n", __func__);
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	info = wapp_probe_lookup(wapp, probe->mac_addr);
	if (info == NULL) {
		info = wapp_probe_create(wapp, probe->mac_addr);
	}

	//Update probe info in wapp
	info->rssi = probe->rssi;
	info->channel = probe->channel;
	os_memcpy(info->preq, probe->preq, probe->preq_len);
	os_get_time(&info->last_update_time);

	//Send to other daemon
	send_pkt_len = sizeof(struct probe);
	buf = os_zalloc(send_pkt_len);
	if (buf) {
		DBGPRINT(RT_DEBUG_TRACE, "Sending %s\n", __func__);
		os_memcpy(buf, probe, send_pkt_len);
		wapp_send_1905_msg(wapp, WAPP_UPDATE_PROBE_INFO, send_pkt_len, buf);
		os_free(buf);
	}
}


void wdev_chn_list_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_chn_info *chn_list_rcvd)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_chn_info *chn_list;
		chn_list = &ap->ch_info;
		os_memcpy(chn_list, chn_list_rcvd, sizeof(wdev_chn_info));
		if (ap->isActive)
			wapp_radio_update_ch(wapp, wdev->radio, chn_list->op_ch);


		DBGPRINT_RAW(RT_DEBUG_OFF,
				"chn_list: (%u)\n"
				"\t op_class = %u\n"
				"\t op_ch = %u\n"
				"\t band = %u \n"
				"\t ch_list_num = %u\n"
				"\t non_op_chn_num = %d\n"
				"\t dl_mcs = %u\n",
				ifindex,
				chn_list->op_class,
				chn_list->op_ch,
				chn_list->band,
				chn_list->ch_list_num,
				chn_list->non_op_chn_num,
				chn_list->dl_mcs);
	}
}

void wdev_op_class_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_op_class_info *op_class_rcvd)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_op_class_info *op_class;
		op_class = &ap->op_class;
		os_memcpy(op_class, op_class_rcvd, sizeof(wdev_op_class_info));


		DBGPRINT_RAW(RT_DEBUG_OFF,
				"op_class: (%u)\n"
				"\t num_of_op_class = %u \n",
				ifindex,
				op_class->num_of_op_class);

		int i = 0;
		for(i = 0; i < op_class->num_of_op_class; i++) {
			printf("\t opClass = %u, chn_num = %u\n", op_class->opClassInfo[i].op_class, op_class->opClassInfo[i].num_of_ch);
		}
	}
}

void wapp_on_boot_scan(void *eloop_data, void *user_ctx);

void wdev_bss_info_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_bss_info *bss_info_rcvd)
{
	struct wapp_dev *wdev = NULL;
	u8 idx = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_bss_info *bss_info;
		bss_info = os_zalloc(sizeof(wdev_bss_info));

		if (!bss_info) {
			return;
		}
		os_memcpy(bss_info, bss_info_rcvd, sizeof(wdev_bss_info));

		DBGPRINT_RAW(RT_DEBUG_OFF,
				"bss_info: (%u)\n"
				"\t table idx = %u\n"
				"\t ssid = %s\n"
				"\t SsidLen = %u\n"
				"\t map role = %u\n"
				"\t Bssid = [%02x][%02x][%02x][%02x][%02x][%02x]\n"
				"\t Identifier = [%02x][%02x][%02x][%02x][%02x][%02x]\n",
				ifindex,
				wapp->map->bss_tbl_idx,
				bss_info->ssid,
				bss_info->SsidLen,
				bss_info->map_role,
				PRINT_MAC(bss_info->bssid),
				PRINT_MAC(bss_info->if_addr));

		os_memcpy(&ap->bss_info, bss_info, sizeof(wdev_bss_info));
		idx = wapp->map->bss_tbl_idx;
		wapp->map->op_bss_table[idx] = *bss_info;
		wapp->map->bss_tbl_idx++;

		os_free(bss_info);
		bss_info = NULL;
#ifdef MAP_R2
		if(wapp->map->MapMode == 4){// perform onboot scan only for certification
			if(eloop_is_timeout_registered(wapp_on_boot_scan, wapp,NULL))
				eloop_cancel_timeout(wapp_on_boot_scan, wapp,NULL);
			eloop_register_timeout(15,0,wapp_on_boot_scan, wapp, NULL);
		}
#endif
	}
}


void wdev_ap_metric_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_ap_metric *ap_metrics_rcvd)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_ap_metric *ap_metrics;
		ap_metrics = &ap->ap_metrics;
		os_memcpy(ap_metrics, ap_metrics_rcvd, sizeof(wdev_ap_metric));
		DBGPRINT_RAW(RT_DEBUG_TRACE, "Ap_metric: (%u)\n", ifindex);
		wdev_ap_show_ap_metric(wapp, ap);
#ifdef MAP_SUPPORT
		map_send_ap_metric_msg(wapp, ap);
#endif
	}
}
#ifdef MAP_R2
void wdev_radio_metric_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
		map_send_radio_metric_msg(wapp, event_data, ifindex);
}
#endif

void wdev_ch_util_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_radio *ra = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		DBGPRINT(RT_DEBUG_INFO, "WAPP got WAPP_CH_UTIL_QUERY_RSP, ch_util: %d\n", event_data->ch_util);
		ra = wdev->radio;
		ra->metric_policy.ch_util_prev = ra->metric_policy.ch_util_current;
		ra->metric_policy.ch_util_current = event_data->ch_util;
	}
}

void wdev_ap_config_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		wapp->map->rssi_steer = event_data->ap_conf.rssi_steer;
		wapp->map->sta_report_not_cop = event_data->ap_conf.sta_report_not_cop;
		wapp->map->sta_report_on_cop = event_data->ap_conf.sta_report_on_cop;

		DBGPRINT_RAW(RT_DEBUG_OFF,
				"Ap_conf: (%u)\n"
				"\t rssi_steer = %u\n"
				"\t sta_report_not_cop = %u\n"
				"\t sta_report_on_cop = %u\n",
				ifindex,
				wapp->map->rssi_steer,
				wapp->map->sta_report_not_cop,
				wapp->map->sta_report_on_cop);
	}
}

void wdev_bcn_report_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct beacon_metrics_rsp *bcn_rpt = NULL;
	struct wapp_sta *sta = NULL;
	struct ap_dev *ap = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {

		ap = (struct ap_dev *)wdev->p_dev;
		sta = wdev_ap_client_list_lookup(wapp, ap, event_data->bcn_rpt_info.sta_addr);
		if (!sta || sta->sta_status != WAPP_STA_CONNECTED)
			return;
		if (sta->beacon_report == NULL) {
			sta->beacon_report = (struct beacon_metrics_rsp *)malloc(sizeof(struct beacon_metrics_rsp) + MAX_BEACON_REPORT_LEN);
			os_memset(sta->beacon_report, 0, sizeof(*sta->beacon_report));
			os_memcpy(sta->beacon_report->sta_mac, event_data->bcn_rpt_info.sta_addr, MAC_ADDR_LEN);
		}
		bcn_rpt = sta->beacon_report;
		if ((bcn_rpt->rpt_len + event_data->bcn_rpt_info.bcn_rpt_len) > MAX_BEACON_REPORT_LEN)
			return;
		if(event_data->bcn_rpt_info.last_fragment)
			bcn_rpt->bcn_rpt_num++;
		os_memcpy(&bcn_rpt->rpt[bcn_rpt->rpt_len],
			event_data->bcn_rpt_info.bcn_rpt, event_data->bcn_rpt_info.bcn_rpt_len);
		bcn_rpt->rpt_len += event_data->bcn_rpt_info.bcn_rpt_len;

		DBGPRINT_RAW(RT_DEBUG_OFF,
			" get bcn rpt from %02x:%02x:%02x:%02x:%02x:%02x"
			" len: %d"
			" accumulate bcn rpt num: %d\n"
			" accumulate bcn rpt len: %d\n",
			PRINT_MAC(event_data->bcn_rpt_info.sta_addr),
			event_data->bcn_rpt_info.bcn_rpt_len,
			bcn_rpt->bcn_rpt_num,
			bcn_rpt->rpt_len);
	}
}


void wdev_bcn_report_complete_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
#if defined(MAP_SUPPORT)
	char buf[MAX_EVT_BUF_LEN];
	int send_pkt_len = 0;
#endif

	struct wapp_dev *wdev = NULL;
	struct wapp_sta *sta = NULL;
	struct ap_dev *ap = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		ap = (struct ap_dev *)wdev->p_dev;
		sta = wdev_ap_client_list_lookup(wapp, ap, event_data->bcn_rpt_info.sta_addr);
		if (!sta || sta->sta_status != WAPP_STA_CONNECTED)
			return;
		if (sta->beacon_report) {
			DBGPRINT_RAW(RT_DEBUG_OFF,
				" get bcn rpt complete from %02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(event_data->bcn_rpt_info.sta_addr));
#if defined(MAP_SUPPORT)
			send_pkt_len = sizeof(struct beacon_metrics_rsp) + sizeof(unsigned char) * sta->beacon_report->rpt_len;
			os_memcpy(buf, sta->beacon_report, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_BEACON_METRICS_REPORT, send_pkt_len, buf);
#endif
			free(sta->beacon_report);
			sta->beacon_report = NULL;
		}
	}
}

void wdev_bssload_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct bssload_info bssload_info;
	int send_pkt_len = 0;
	char* buf = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		os_memcpy(&ap->bssload, &event_data->bssload_info, sizeof(wapp_bssload_info));

		DBGPRINT_RAW(RT_DEBUG_OFF,
			" sta_cnt: %d"
			" ch_util: %d\n"
			" AvalAdmCap: %x\n",
			event_data->bssload_info.sta_cnt,
			event_data->bssload_info.ch_util,
			event_data->bssload_info.AvalAdmCap);

		os_memcpy(&bssload_info, &ap->bssload, sizeof(struct bssload_info));
		send_pkt_len = sizeof(struct bssload_info);
		buf = os_zalloc(send_pkt_len);
		if (buf) {
			os_memcpy(buf, &bssload_info, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_STA_BSSLOAD, send_pkt_len, buf);
			os_free(buf);
		}
	}
}

void wdev_mnt_info_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char* buf = NULL;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp)
		return;

#ifdef MAP_SUPPORT
	if (!dl_list_empty(&wapp->sta_mntr_list)){
		air_monitor_packet_handle(wapp, ifindex, event_data);
		return;
	}
#endif

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wapp_mnt_info *mnt_info;

		mnt_info = &ap->mnt;
		os_memcpy(mnt_info, &event_data->mnt_info, sizeof(wapp_mnt_info));

		send_pkt_len = sizeof(wapp_mnt_info);
		buf = os_zalloc(send_pkt_len);
		if (buf) {
			os_memcpy(buf, mnt_info, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_NAC_INFO, send_pkt_len, buf);
			os_free(buf);
		}
	}
}

void wdev_sta_rssi_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	wapp_client_info *cli = NULL;
	struct wapp_sta* sta = NULL;
	int send_pkt_len = 0;
	char* buf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		cli = &event_data->cli_info;

		sta = wdev_ap_client_list_lookup_for_all_bss(wapp, cli->mac_addr);
		if (sta) {
			sta->uplink_rssi = cli->uplink_rssi;
			DBGPRINT_RAW(RT_DEBUG_OFF,
						"sta->uplink_rssi: %d", sta->uplink_rssi);

			send_pkt_len = MAC_ADDR_LEN + sizeof(u8);
			buf = os_zalloc(send_pkt_len);
			if (buf) {
				os_memcpy(buf, sta->mac_addr, MAC_ADDR_LEN);
				os_memcpy(buf + MAC_ADDR_LEN, &sta->uplink_rssi, sizeof(u8));
				wapp_send_1905_msg(wapp, WAPP_STA_RSSI, send_pkt_len, buf);
				os_free(buf);
			}
		}
	}
}

void wdev_cli_active_change_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	wapp_client_info *cli = NULL;
	struct wapp_sta* sta = NULL;
	struct ap_dev *ap = NULL;
	int send_pkt_len = 0;
	wapp_sta_status_info *sta_status = NULL;
	DBGPRINT(RT_DEBUG_TRACE, RED("%s \n"), __func__);

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		cli = &event_data->cli_info;
		ap = (struct ap_dev *)wdev->p_dev;
		sta = wdev_ap_client_list_lookup(wapp, ap, cli->mac_addr);

		if (sta) {
			sta->stat = cli->status;
			DBGPRINT_RAW(RT_DEBUG_OFF, RED("sta->stat: %d"), sta->stat);

			send_pkt_len = sizeof(wapp_sta_status_info);
			sta_status = (wapp_sta_status_info *)os_zalloc(send_pkt_len);
			if (sta_status) {
				os_memcpy(sta_status->sta_mac, sta->mac_addr, MAC_ADDR_LEN);
				sta_status->status = sta->stat;
				wapp_send_1905_msg(wapp, WAPP_STA_STAT, send_pkt_len, (char *)sta_status);
				os_free(sta_status);
			}
		} else
			DBGPRINT(RT_DEBUG_WARN, RED("Null sta\n"));
	}
}

void wdev_chn_change_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	struct wapp_dev *wdev_temp = NULL;
	struct dl_list *dev_list = NULL;
	u8 new_ch = 0;
	u8 op_class = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	dev_list = &wapp->dev_list;
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_chn_info *chn_list;

		chn_list = &ap->ch_info;
		chn_list->op_ch = event_data->ch_change_info.new_ch;
		chn_list->op_class = event_data->ch_change_info.op_class;
		printf("ch: %u op_class: %u\n", ap->ch_info.op_ch, ap->ch_info.op_class);
		if (wdev->radio)
			wdev->radio->op_ch = chn_list->op_ch;

		map_operating_channel_info(wapp);
	} else if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		new_ch = event_data->ch_change_info.new_ch;
		op_class = event_data->ch_change_info.op_class;
		wapp_radio_update_ch(wapp, wdev->radio, new_ch);
		dl_list_for_each(wdev_temp, dev_list, struct wapp_dev, list) {
			if (wdev->radio == wdev_temp->radio) {
				if (wdev_temp->dev_type == WAPP_DEV_TYPE_AP) {
					struct ap_dev * ap = (struct ap_dev *)wdev_temp->p_dev;
					ap->ch_info.op_ch = new_ch;
					 ap->ch_info.op_class = op_class;
				}
			}
		}
	}
}

void wdev_csa_event_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct dl_list *dev_list;
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char* buf = NULL;
	u8 ruid[MAC_ADDR_LEN];
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		MAP_GET_RADIO_IDNFER(wdev->radio, ruid);

		if (wdev->radio->op_ch != event_data->csa_info.new_channel) {
			wdev_chn_info *chn_list;
			struct csa_info_rsp csa_info;

			wapp_radio_update_ch(wapp, wdev->radio, event_data->csa_info.new_channel);

			//update op_ch of ap wdev under this ruid
			dev_list = &wapp->dev_list;
			dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
				if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP) {
					u8 wdev_identifier[MAC_ADDR_LEN];
					MAP_GET_RADIO_IDNFER(wdev->radio, wdev_identifier);
					if (!os_memcmp(wdev_identifier, ruid, MAC_ADDR_LEN)) {
						struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;

						ap->ch_info.op_ch = event_data->csa_info.new_channel;
						chn_list = &ap->ch_info;
						chn_list->op_ch = event_data->csa_info.new_channel;
					}
				}
			}

			os_memcpy(csa_info.ruid, ruid, MAC_ADDR_LEN);
			csa_info.new_ch = event_data->csa_info.new_channel;
			send_pkt_len = sizeof(struct csa_info_rsp);
			buf = os_zalloc(send_pkt_len);
			if (buf) {
				os_memcpy(buf, &csa_info, send_pkt_len);
				wapp_send_1905_msg(wapp, WAPP_CSA_INFO, send_pkt_len, buf);
				os_free(buf);
			}
		}
	}
	wapp->csa_new_channel = event_data->csa_info.new_channel;
	if(wapp->map->quick_ch_change == TRUE)
		wapp->cli_assoc_info.current_channel = event_data->csa_info.new_channel;
	map_operating_channel_info(wapp);
}

void wdev_apcli_rssi_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char* buf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		wapp_apcli_association_info *apcli_info = &event_data->apcli_association_info;

		send_pkt_len = sizeof(char);
		buf = os_zalloc(send_pkt_len);
		if (buf) {
			os_memcpy(buf, &apcli_info->rssi, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_APCLI_UPLINK_RSSI, send_pkt_len, buf);
			os_free(buf);
		}
	}
}

void wdev_bss_stat_change_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char *buf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		ap->isActive = event_data->bss_state_info.bss_state;

		//Send to other daemon
		struct bss_state_info bss_stat_info;
		bss_stat_info.bss_state = event_data->bss_state_info.bss_state;
		bss_stat_info.interface_index = event_data->bss_state_info.interface_index;
		send_pkt_len = sizeof(struct bss_state_info);
		os_alloc_mem(NULL, (UCHAR **)&buf, send_pkt_len);
		if (buf) {
			os_memcpy(buf, &bss_stat_info, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_BSS_STAT_CHANGE, send_pkt_len, buf);
			os_free(buf);
		}
	}
}

void wdev_bssload_crossing_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char* buf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct bssload_crossing_info bssload_crossing_info;

		bssload_crossing_info.bssload = event_data->bssload_crossing_info.bssload;
		bssload_crossing_info.bssload_high_thrd = event_data->bssload_crossing_info.bssload_high_thrd;
		bssload_crossing_info.bssload_low_thrd = event_data->bssload_crossing_info.bssload_low_thrd;
		bssload_crossing_info.interface_index = event_data->bssload_crossing_info.interface_index;

		send_pkt_len = sizeof(struct bssload_crossing_info);
		os_alloc_mem(NULL, (UCHAR **)&buf, send_pkt_len);
		if (buf) {
			os_memcpy(buf, &bssload_crossing_info, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_BSS_LOAD_CROSSING, send_pkt_len, buf);
			os_free(buf);
		}
	}
}

void wdev_update_bh_link_info(struct wifi_app *wapp, u32 ifindex, u8 action)
{
	struct wapp_dev *wdev = NULL;
	struct bh_link *bh_links = NULL, *bh_links_tmp = NULL;
	u8 is_exist = 0 , num = 0;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (!dl_list_empty(&(wapp->map->bh_link_list))) {
		dl_list_for_each_safe(bh_links, bh_links_tmp, &wapp->map->bh_link_list, struct bh_link, list) {
			if(!os_memcmp(bh_links->bssid, wdev->mac_addr, MAC_ADDR_LEN))  {
				is_exist = 1;
				break;
			}
		}
	}
	
	if (action == 0 && is_exist == 1) { // disassoc case

		if(wapp->map->bh_link_num > 0) {
			DBGPRINT(RT_DEBUG_OFF, "remove link mac: %02X:%02X:%02X:%02X:%02X:%02X \n",PRINT_MAC(bh_links->bssid));
			dl_list_del(&bh_links->list);
			os_free(bh_links);
			bh_links = NULL;
			wapp->map->bh_link_num --;
		}
	} else if (action == 1 && !is_exist) { // assoc case
		/*create New link*/
		bh_links = NULL;
		bh_links = (struct bh_link *)os_zalloc(sizeof(struct bh_link));
		if(bh_links == NULL) {
			DBGPRINT(RT_DEBUG_OFF, "%s Memory Alloc Fail\n", __func__);
			return;
		}
		DBGPRINT(RT_DEBUG_OFF, "add link mac: %02X:%02X:%02X:%02X:%02X:%02X \n",PRINT_MAC(wdev->mac_addr));
		os_memcpy(bh_links->bssid, wdev->mac_addr, MAC_ADDR_LEN);
		dl_list_add_tail(&(wapp->map->bh_link_list), &bh_links->list);
		wapp->map->bh_link_num ++;
	}

	DBGPRINT(RT_DEBUG_OFF, "action:%d, is_exist:%d, bh_link_num:%d \n", action, is_exist, wapp->map->bh_link_num);

	num = 0;
	bh_links = NULL;
	if (!dl_list_empty(&(wapp->map->bh_link_list))) {
		dl_list_for_each_safe(bh_links, bh_links_tmp, &wapp->map->bh_link_list, struct bh_link, list) {
			num++;
			DBGPRINT(RT_DEBUG_OFF, " link mac (num:%d): %02X:%02X:%02X:%02X:%02X:%02X \n", num, PRINT_MAC(wdev->mac_addr));
		}
	}
}

void wdev_apcli_assoc_stat_change_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL, *ap_wdev;
	int i;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		// Send to other daemon
		struct apcli_association_info *apcli_stat_info = &wapp->cli_assoc_info;
		apcli_stat_info->interface_index = event_data->apcli_association_info.interface_index;
		apcli_stat_info->apcli_assoc_state = event_data->apcli_association_info.apcli_assoc_state;
		apcli_stat_info->peer_map_enable = event_data->apcli_association_info.PeerMAPEnable;
		apcli_stat_info->current_channel = wdev->radio->op_ch;

		if (!wapp->wsc_configs_pending) {
			if ( wapp->map->quick_ch_change == TRUE || wapp->csa_notif_received != TRUE) {
					wapp_send_1905_msg(wapp, WAPP_APCLI_ASSOC_STAT_CHANGE, sizeof(struct apcli_association_info),
					(char *)&wapp->cli_assoc_info);
					wdev_update_bh_link_info(wapp, ifindex, wapp->cli_assoc_info.apcli_assoc_state);
			} else {
				wapp->link_change_notif_pending = TRUE;
			}

			if (wapp->map->TurnKeyEnable && apcli_stat_info->apcli_assoc_state == WAPP_APCLI_DISASSOCIATED) {
				eloop_cancel_timeout(map_config_state_check,wapp,NULL);
				if (wdev && wdev->dev_type == WAPP_DEV_TYPE_STA) {
					char local_command[128];
					os_memset(local_command, 0, sizeof(local_command));
					os_snprintf(local_command, sizeof(local_command), "iwpriv %s set ApCliEnable=0",
						wdev->ifname);
					system(local_command);
				}
			} else {
				for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
					/* This command will go twice in case of single chip DBDC */
					if (wapp->radio[i].adpt_id) {
						char idfr[MAC_ADDR_LEN];
						char cmd[256];
						struct wapp_radio *ra = &wapp->radio[i];
						MAP_GET_RADIO_IDNFER(ra, idfr);
						ap_wdev = wapp_dev_list_lookup_by_radio(wapp, idfr);
						os_memset(cmd, 0, 256);
						sprintf(cmd, "iwpriv %s set APProxyRefresh=1;", ap_wdev->ifname);
						system(cmd);
					}
				}
			}
		} else {
			if (apcli_stat_info->apcli_assoc_state == WAPP_APCLI_DISASSOCIATED) {
				stop_con_cli_wps(wapp, NULL);
			}
		}

	}
}
void wdev_apcli_assoc_stat_change_handle_vendor10(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL, *ap_wdev;
	int i;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;
	if (wdev->dev_type == WAPP_DEV_TYPE_STA) {
		// Send to other daemon
		struct apcli_association_info *apcli_stat_info = &wapp->cli_assoc_info;
		apcli_stat_info->interface_index = event_data->apcli_association_info.interface_index;
		apcli_stat_info->apcli_assoc_state = event_data->apcli_association_info.apcli_assoc_state;
		apcli_stat_info->current_channel = wdev->radio->op_ch;
		for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		/* This command will go twice in case of single chip DBDC */
			if (wapp->radio[i].adpt_id) {
					char idfr[MAC_ADDR_LEN];
					char cmd[256];
					struct wapp_radio *ra = &wapp->radio[i];
					MAP_GET_RADIO_IDNFER(ra, idfr);
					ap_wdev = wapp_dev_list_lookup_by_radio(wapp, idfr);
					os_memset(cmd, 0, 256);
					if(event_data->apcli_association_info.apcli_assoc_state == WAPP_APCLI_DISASSOCIATED)
						sprintf(cmd, "iwpriv %s set V10Converter=0;", ap_wdev->ifname);
					else
						sprintf(cmd, "iwpriv %s set V10Converter=1;", ap_wdev->ifname);

					system(cmd);
				}
			}
		}
}



#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#ifdef MAP_R2
void assoc_fail_count_timer(void *eloop_ctx, void *timeout_ctx)
{
	struct wifi_app *wapp = (struct wifi_app *)eloop_ctx;
	wapp->map->assoc_fail_rep_count =0;
}
#endif

void wdev_sta_cnnct_rej_handle(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char *buf = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct sta_cnnct_rej_info sta_cnnct_rej_info;

		sta_cnnct_rej_info.interface_index = event_data->sta_cnnct_rej_info.interface_index;
		os_memcpy(sta_cnnct_rej_info.sta_mac, event_data->sta_cnnct_rej_info.sta_mac, MAC_ADDR_LEN);
		os_memcpy(sta_cnnct_rej_info.bssid, event_data->sta_cnnct_rej_info.bssid, MAC_ADDR_LEN);
		sta_cnnct_rej_info.cnnct_fail.connect_stage = event_data->sta_cnnct_rej_info.cnnct_fail.connect_stage;
		sta_cnnct_rej_info.cnnct_fail.reason = event_data->sta_cnnct_rej_info.cnnct_fail.reason;
#ifdef MAP_R2
		sta_cnnct_rej_info.assoc_status_code = event_data->sta_cnnct_rej_info.assoc_status_code;
		sta_cnnct_rej_info.assoc_reason_code = event_data->sta_cnnct_rej_info.assoc_reason_code;
		//code to check
		printf("### %d %s assoc_status_code = %d: %d \n", __LINE__, __func__, sta_cnnct_rej_info.assoc_status_code,
																		sta_cnnct_rej_info.assoc_reason_code);
		printf("### %d %s report_unsuccessful_association = %d \n", __LINE__, __func__, wapp->map->assoc_failed_policy.report_unsuccessful_association);
		printf("### %d %s max_supporting_rate = %d \n", __LINE__, __func__, wapp->map->assoc_failed_policy.max_supporting_rate);

		wapp->map->assoc_fail_rep_count++;
		if (wapp->map->assoc_failed_policy.report_unsuccessful_association
			&& (wapp->map->assoc_failed_policy.max_supporting_rate > wapp->map->assoc_fail_rep_count)
			)
			sta_cnnct_rej_info.send_failed_assoc_frame = 1;
		else
			sta_cnnct_rej_info.send_failed_assoc_frame = 0;
		printf("### %d %s send_failed_assoc_frame = %d \n", __LINE__, __func__, sta_cnnct_rej_info.send_failed_assoc_frame);

		if(!eloop_is_timeout_registered(assoc_fail_count_timer,wapp,NULL)) {
			eloop_register_timeout(60,0,assoc_fail_count_timer,wapp,NULL );
		}
#endif
		send_pkt_len = sizeof(struct sta_cnnct_rej_info);
		os_alloc_mem(NULL, (UCHAR **)&buf, send_pkt_len);
		if (buf) {
			os_memcpy(buf, &sta_cnnct_rej_info, send_pkt_len);
			wapp_send_1905_msg(wapp, WAPP_STA_CNNCT_REJ_INFO, send_pkt_len, buf);
			os_free(buf);
		}
	}
}

#define L1_PROFILE_PATH "/etc/wireless/l1profile.dat"
int wapp_read_l1_profile_file(char *ifname, int *radio_index, int *band_index)
{
	FILE *file;
	char buf[256], *pos, *token;
	char tmpbuf[256];
	char test_buf[256];
	int line = 0, i = 0;
	char ra_band_idx = 0;


	file = fopen(L1_PROFILE_PATH, "r");
	if (!file) {
		printf("open l1 profile file (%s) fail\n", L1_PROFILE_PATH);
		return -1;
	}


	for (i = 0; i < 3; i++) {
		sprintf(test_buf, "INDEX%d_apcli_ifname", i);
		line = 0;
		os_memset(buf, 0, 256);
		os_memset(tmpbuf, 0, 256);
		ra_band_idx = 0;
		while (wapp_config_get_line(buf, sizeof(buf), file, &line, &pos)) {
			os_snprintf(tmpbuf, sizeof(tmpbuf), "%s", pos);
			token = strtok(pos, "=");
			if (token != NULL) {
				if (os_strcmp(token, test_buf) == 0) {
					token = strtok(NULL, ";");
					while (token != NULL && ra_band_idx < 2) {
						if (os_strcmp(token, ifname) == 0) {
							*radio_index = i;
							*band_index = ra_band_idx;
							fclose(file);
							return 0;
						}
						ra_band_idx++;
						token = strtok(NULL, ";");
					}
				}
			}
		}
	}

	fclose(file);
	return -1;
}

#ifdef DPP_SUPPORT
struct wapp_dev* wapp_dev_list_lookup_by_radio_and_type(struct wifi_app *wapp, char* ra_identifier, const u8 wdev_type)
{
        struct wapp_dev *wdev, *target_wdev = NULL;
        unsigned char wdev_identifier[ETH_ALEN];
        struct dl_list *dev_list;

        DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
        dev_list = &wapp->dev_list;

        dl_list_for_each(wdev, dev_list, struct wapp_dev, list){
                if (wdev && wdev->radio) {
                        MAP_GET_RADIO_IDNFER(wdev->radio, wdev_identifier);
                        if(!os_memcmp(wdev_identifier, ra_identifier, ETH_ALEN) &&
                                (wdev->dev_type == wdev_type)) {
                                        target_wdev = wdev;
                                        break;
                        }
                }
        }

        return target_wdev;
}

void wdev_get_dpp_action_frame(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	char dpp_frame_event[2048] = {0};
	int frame_len = 2048;
	u32 frm_id;
	if (!wapp)
		return;
	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;
	frm_id = event_data->wapp_dpp_frame_id_no;
	os_memcpy(dpp_frame_event, (char*)&(frm_id), sizeof(u32));
	DBGPRINT(RT_DEBUG_TRACE,"[%s] frame id number = %d\n", __func__, frm_id);
	driver_wext_get_dpp_frame(wapp->drv_data, wdev->ifname, dpp_frame_event ,frame_len);
	wdev_handle_dpp_action_frame(wapp, wdev, (struct wapp_dpp_action_frame *)dpp_frame_event);
}

void wdev_handle_dpp_action_frame(struct wifi_app *wapp,
	struct wapp_dev *wdev, struct wapp_dpp_action_frame *frame)
{
	if (frame->is_gas) {
		if (wapp->dpp->dpp_configurator_supported)
			gas_server_rx(wapp->dpp->gas_server, wdev, wdev->mac_addr,
					(u8 *)frame->src, wdev->mac_addr,
					10, frame->frm, frame->frm_len, frame->chan);
		else
			gas_query_rx(wapp->dpp->gas_query_ctx, wdev->mac_addr, (u8 *)frame->src, (u8 *)frame->src,
					10, frame->frm, frame->frm_len, frame->chan);

	} else
		wapp_dpp_rx_action(wapp, wdev, (u8 *)frame->src,
				frame->frm, frame->frm_len - 4, frame->chan);
}

void wdev_handle_dpp_frm_tx_status(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	struct dpp_tx_status *tx_status = wapp_dpp_get_status_info_from_sq(wapp, event_data->tx_status.seq_no);

	if (!tx_status) {
		DBGPRINT(RT_DEBUG_ERROR, "%s: failed to get status for sq=%u \n", __func__, event_data->tx_status.seq_no);
		return;
	}
	if (tx_status->is_gas_frame) {
		if (wapp->dpp->dpp_configurator_supported)
			gas_server_tx_status(wapp->dpp->gas_server, tx_status->dst,
				NULL, 0, event_data->tx_status.tx_success ? 0 : 1);
		else
			gas_query_tx_status(wapp, 0, tx_status->dst,
				NULL, NULL, NULL, 0, event_data->tx_status.tx_success ? 0 : 1);
	} else
		wapp_dpp_tx_status(wapp, tx_status->dst,
				NULL, 0, event_data->tx_status.tx_success ? 0 : 1);
}
#endif /*DPP_SUPPORT*/

void wdev_handle_wsc_eapol_notif(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	stop_con_ap_wps(wapp, wdev);
}

void wdev_handle_wsc_eapol_end_notif(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	wapp_device_status *device_status = &wapp->map->device_status;
	device_status->status_fhbss = STATUS_FHBSS_WPS_SUCCESSFULL;
	eloop_cancel_timeout(map_wps_timeout, wapp, device_status);
	wapp_send_1905_msg(
		wapp,
		WAPP_DEVICE_STATUS,
		sizeof(wapp_device_status),
		(char *)device_status);
}
void wdev_handle_scan_complete_notif(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	/*Ignore! if scan is not triggered by wapp*/
	if(!wdev->wapp_triggered_scan) {
		return;
	}
	wdev->wapp_triggered_scan = FALSE;
	map_get_scan_result(wapp, wdev);
}

void wdev_enable_pmf(struct wifi_app *wapp, struct wapp_dev *wdev)
{
	char cmd[100];

	os_memset(cmd, 0, 100);
	if (wdev->dev_type == WAPP_DEV_TYPE_AP)
		sprintf(cmd, "iwpriv %s set PMFMFPC=1", wdev->ifname);
	else
		sprintf(cmd, "iwpriv %s set ApCliPMFMFPC=1", wdev->ifname);
	system(cmd);
	DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
}

void wdev_bh_sta_reset_default(struct wifi_app *wapp, struct wapp_dev *wdev)
{
	char cmd[100];

	if (!wapp)
		return;

	memset(cmd, 0, 100);
	sprintf(cmd, "iwpriv %s set ApCliEnable=0;", wdev->ifname);
	system(cmd);

	memset(cmd, 0, 100);
	sprintf(cmd,"iwpriv %s set ApCliSsid=MAP_APCLI_UNCONF", wdev->ifname);
	DBGPRINT(RT_DEBUG_ERROR, "<CMD:%s> \n",cmd);
	system(cmd);

	memset(cmd, 0, 100);
	sprintf(cmd, "iwpriv %s set ApCliWPAPSK=12345678", wdev->ifname);
	system(cmd);

	memset(cmd, 0, 100);
	sprintf(cmd, "iwpriv %s set ApCliAuthMode=OPEN", wdev->ifname);
	DBGPRINT(RT_DEBUG_ERROR, "<CMD:%s> \n",cmd);
	system(cmd);

	os_memset(cmd, 0, 100);
	sprintf(cmd, "iwpriv %s set ApCliPMFMFPC=0;", wdev->ifname);
	system(cmd);
	DBGPRINT(RT_DEBUG_ERROR, "<CMD:%s> \n",cmd);

	memset(cmd, 0, 100);
	sprintf(cmd, "iwpriv %s set ApCliEncrypType=NONE", wdev->ifname);
	system(cmd);
	DBGPRINT(RT_DEBUG_ERROR, "<CMD:%s> \n",cmd);

	return;
}

#ifdef MAP_R2
void wdev_bh_sta_connect_wsc_profile(struct wifi_app *wapp, struct wapp_dev *wdev, wsc_apcli_config *cli_conf)
{
	char *auth_str[] = {
		"OPEN",
		"WPAPSK",
		"SHARED",
		"WPA",
		"WPA2",
		"WPA2PSK",
		"WPA2PSKWPA3PSK"
	};
	char *encryp_str[] = {
		"NONE",
		"WEP",
		"TKIP",
		"AES",
	};
	int i = 0, j = 0;
	unsigned short authmode = cli_conf->AuthType;
	unsigned short encryptype = cli_conf->EncrType;
	char cmd[200];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	memset(cmd, 0, 200);
	os_snprintf(cmd, sizeof(cmd),"iwpriv %s set ApCliEnable=0;", wdev->ifname);
	system(cmd);

	memset(cmd, 0, 200);

	if (cli_conf->SsidLen <= MAX_LEN_OF_SSID)
		cli_conf->ssid[cli_conf->SsidLen] = '\0';
	else
		cli_conf->ssid[MAX_LEN_OF_SSID] = '\0';

	os_snprintf(cmd, sizeof(cmd),"iwpriv %s set ApCliSsid=%s", wdev->ifname, cli_conf->ssid);
	DBGPRINT(RT_DEBUG_ERROR, "<CMD:%s> \n",cmd);
	system(cmd);
	DBGPRINT(RT_DEBUG_ERROR, "<%s> \n",cmd);
	/* this validation is invalid in case of WPA2+WPA */
	if ((authmode < WSC_AUTHTYPE_OPEN || authmode > (WSC_AUTHTYPE_WPA2PSK | WSC_AUTHTYPE_SAE)) ||
		(encryptype < WSC_ENCRTYPE_NONE || encryptype > WSC_ENCRTYPE_AES)) {
		DBGPRINT(RT_DEBUG_TRACE, "%s, invalid sec_info auth=%d enc=%d\n",
				__func__, authmode, encryptype);
		return;
	}

	if (cli_conf->KeyLength) {
		memset(cmd, 0, 200);
		if (cli_conf->KeyLength < 64)
			cli_conf->Key[cli_conf->KeyLength] = '\0';
		else 
			cli_conf->Key[63] = '\0';
		os_snprintf(cmd, sizeof(cmd),"iwpriv %s set ApCliWPAPSK=%s", wdev->ifname, cli_conf->Key);
		system(cmd);
	}
#ifdef MAP_R2
	/* Make it mixed mode */
	if (wapp->map->map_version == 2 && authmode == WSC_AUTHTYPE_WPA2PSK)
		authmode = authmode << 1;
#endif
	DBGPRINT(RT_DEBUG_ERROR,"setting wpa3\n");
	while (authmode) {
		authmode = authmode >> 1;
		i++;
	}
	i--;
	memset(cmd, 0, 200);
	os_snprintf(cmd, sizeof(cmd),"iwpriv %s set ApCliAuthMode=%s", wdev->ifname, auth_str[i]);
	DBGPRINT(RT_DEBUG_ERROR, "<CMD:%s> \n",cmd);
	system(cmd);
	if (strcmp(auth_str[i], "WPA2PSKWPA3PSK") == 0) {
		os_memset(cmd, 0, 200);
		os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliPMFMFPC=1;", wdev->ifname);
		system(cmd);
		DBGPRINT(RT_DEBUG_ERROR,"%s\n", cmd);
	}

	while (encryptype) {
		encryptype = encryptype >> 1;
		j++;
	}
	j--;
	
	memset(cmd, 0, 200);
	os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliEncrypType=%s", wdev->ifname, encryp_str[j]);
	system(cmd);

	memset(cmd, 0, 200);
	os_snprintf(cmd, sizeof(cmd), "iwpriv %s set ApCliEnable=1;", wdev->ifname);
	system(cmd);

	return;

}
#endif

void wdev_handle_wsc_config_write(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	wsc_apcli_config_msg *apcli_config_msg;
	char wsc_profile_buffer[512] = {0};
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;
//	send oid here to fill up the wsc profile credentials from driver
	driver_wext_get_wsc_profiles(wapp->drv_data, wdev->ifname,wsc_profile_buffer,&send_pkt_len);
	apcli_config_msg = (wsc_apcli_config_msg *)wsc_profile_buffer;
	DBGPRINT(RT_DEBUG_ERROR, "In WAPP , profile count is %d ,send_pkt_len %d \n",apcli_config_msg->profile_count,send_pkt_len );
	if(wapp->wsc_save_bh_profile == TRUE) {
		write_configs(wapp, apcli_config_msg->apcli_config, 0, NULL);
		wapp->wsc_save_bh_profile = FALSE;
	}
	if(wapp->wps_on_controller_cli==1) {
		write_configs(wapp,apcli_config_msg->apcli_config, 0, NULL);
		wapp->wps_on_controller_cli=0;
	}
	wapp->wsc_configs_pending = FALSE;
	if(wapp->map->TurnKeyEnable) {
		wapp_send_1905_msg(wapp, WAPP_MAP_BH_CONFIG, send_pkt_len,(char *) apcli_config_msg);
	}
#ifdef MAP_R2
        else {
		/*4.14.2_BH5GL_FH24G: we have received WPS profiles from driver. Need to trigger connection on the bh_sta configured.*/
		struct wapp_dev *bsta_wdev =  wapp->map->bh_wifi_dev;
		char cmd[100];
		DBGPRINT(RT_DEBUG_ERROR, "<ProfileSSID: %s> \n",apcli_config_msg->apcli_config[0].ssid);
		if(bsta_wdev == NULL ||apcli_config_msg->profile_count == 0)
			return;
		// Stop WPS on APCLI interface
		
		DBGPRINT(RT_DEBUG_ERROR, "Calling WSC stop \n");
		os_memset(cmd, 0, sizeof(cmd));
		os_snprintf(cmd,sizeof(cmd),
			"iwpriv %s set WscStop=1;",
			wdev->ifname);
		system(cmd);
		// TODO: what if channel of the BSS is different.
		/*Only trigger connection on the bh_wdev configured through the command.*/
		wdev_bh_sta_connect_wsc_profile(wapp, bsta_wdev, &apcli_config_msg->apcli_config[0]);
		/*Disable APCLI on other interfaces*/
		{
			struct wapp_dev *tmp_wdev;
			struct dl_list *dev_list;

			DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
			dev_list = &wapp->dev_list;

			dl_list_for_each(tmp_wdev, dev_list, struct wapp_dev, list){
				if (tmp_wdev &&  tmp_wdev->dev_type == WAPP_DEV_TYPE_STA && tmp_wdev != bsta_wdev) {
					memset(cmd, 0, 100);
					sprintf(cmd, "iwpriv %s set ApCliEnable=0;", tmp_wdev->ifname);
					system(cmd);
				}
	}
}

	}
#endif
}

void wdev_handle_map_vend_ie_evt(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char *buf = NULL;
	struct map_vendor_ie *map_ie;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	map_ie = (struct map_vendor_ie *)event_data;
	send_pkt_len = sizeof(struct map_vendor_ie);
	os_alloc_mem(NULL, (UCHAR **)&buf, send_pkt_len);
	os_memcpy(buf, map_ie, send_pkt_len);
	wapp_send_1905_msg(wapp, WAPP_MAP_VEND_IE_CHANGED, send_pkt_len, buf);
	os_free(buf);
}

void wdev_handle_a4_entry_missing_notif(struct wifi_app *wapp, u32 ifindex,
	wapp_event_data *event_data)
{
	uint32_t dstip = (uint32_t)event_data->a4_missing_entry_ip;

	if (dstip == 0 || dstip == 0xffffffff) {
        DBGPRINT(RT_DEBUG_ERROR, "Invalid source IP\n");
        return;
    }

	test_arping(wapp->map->br.arp_sock, wapp->map->br.ifindex,
		wapp->map->br.mac_addr, (uint32_t)wapp->map->br.ip,
		dstip);
}

#ifdef WIFI_MD_COEX_SUPPORT
extern int map_send_ap_oper_bss_msg(
	struct wifi_app *wapp, unsigned char *addr, char *evt_buf, int* len_buf);
/*below channel list is defined by modem*/
unsigned char channel_24G[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
unsigned char channel_5G0[] = {36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80,
							   84, 88, 92, 96, 100, 104, 108, 112, 116, 120,
							   124, 128, 132, 136, 140, 144};
unsigned char channel_5G1[] = {149, 153, 157, 161, 165, 169, 173, 177, 181};
	
void wdev_handle_unsafe_ch_event(struct wifi_app *wapp,
	wapp_event_data *event_data)
{
	struct unsafe_channel_notif_s *unsafe_ch = &event_data->unsafe_ch_notif;
	unsigned char radio_id[MAC_ADDR_LEN];
	unsigned int event_size = 0, i = 0;
	char *channel_prefer_info = NULL;
	struct evt * evt_tlv = (struct evt *)channel_prefer_info;

	channel_prefer_info = os_zalloc(1024);
	if (channel_prefer_info == NULL) {
		return;
	}

	wapp->map->off_ch_scan_state.ch_scan_state = CH_SCAN_IDLE;
	evt_tlv = (struct evt *)channel_prefer_info;

	DBGPRINT(RT_DEBUG_ERROR, "bitmap([0~3]=%08x,%08x,%08x,%08x)\n",
		unsafe_ch->ch_bitmap[0], unsafe_ch->ch_bitmap[1], unsafe_ch->ch_bitmap[2],
		unsafe_ch->ch_bitmap[3]);
	/*align with modem, bit 0 is not used for 2.4G band*/
	unsafe_ch->ch_bitmap[0] = unsafe_ch->ch_bitmap[0] >> 1; 
	/*update channel_operable_status*/
	for (i = 0; i< (sizeof(channel_5G0) / sizeof(unsigned char)); i++) {
		if(i < (sizeof(channel_24G) / sizeof(unsigned char)))
			update_primary_ch_status(channel_24G[i], ((1 << i) & unsafe_ch->ch_bitmap[0]) ? 1 : 0);
		if(i < (sizeof(channel_5G0) / sizeof(unsigned char)))
			update_primary_ch_status(channel_5G0[i], ((1 << i) & unsafe_ch->ch_bitmap[1]) ? 1 : 0);
		if(i < (sizeof(channel_5G1) / sizeof(unsigned char)))
			update_primary_ch_status(channel_5G1[i], ((1 << i) & unsafe_ch->ch_bitmap[2]) ? 1 : 0);
	}

	dump_operable_channel();
	
	/*build channel preference for each radio*/
	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		if (wapp->radio[i].adpt_id != 0) {
			MAP_GET_RADIO_IDNFER(((struct wapp_radio *)&wapp->radio[i]), radio_id);
			event_size = map_build_chn_pref(
				wapp, radio_id, channel_prefer_info);
			if (event_size) {
				wapp_send_1905_msg(wapp, WAPP_CHANNLE_PREFERENCE, event_size,
					(char *)evt_tlv->buffer);
			}
			os_sleep(0, 10);
		}
	}
	os_free(channel_prefer_info);
}

void wdev_handle_band_status_change(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	struct band_status_change *band_status_change = &event_data->band_status;
	unsigned char radio_id[MAC_ADDR_LEN];
	struct wapp_dev *wdev = NULL;
	int event_size = 0;
	char *event_buf = NULL;
	
	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;
	
	event_buf = os_malloc(3072);

	if (!event_buf) {
		DBGPRINT(RT_DEBUG_ERROR, "%s fail to alloc memory\n", __func__);
		return;
	}

	wdev->radio->operatable = band_status_change->status;

	MAP_GET_RADIO_IDNFER(wdev->radio, radio_id);
	DBGPRINT(RT_DEBUG_ERROR, "radio(%02x:%02x:%02x:%02x:%02x:%02x)- %s operatable by modem\n",
		PRINT_RA_IDENTIFIER(radio_id), !wdev->radio->operatable ? "not" : "");

	/*update operational bss information when current radio's status changing*/
	map_send_ap_oper_bss_msg(wapp,
		(unsigned char *)&radio_id,
		event_buf, &event_size);

	if (0 > map_1905_send(wapp, event_buf, event_size)) {
		DBGPRINT(RT_DEBUG_TRACE, "%s  send fail \n", __func__);
		os_free(event_buf);
		return;
	}

	DBGPRINT(RT_DEBUG_ERROR, "radio(%02x:%02x:%02x:%02x:%02x:%02x)-update oper BSS\n",
		PRINT_RA_IDENTIFIER(radio_id));

	os_free(event_buf);
}
#endif

void wdev_handle_radar(struct wifi_app *wapp,
	u32 ifindex,
	wapp_event_data *event_data)
{
	struct radar_notif_s *radar_notif = &event_data->radar_notif;
	unsigned char radio_id[MAC_ADDR_LEN];
	struct wapp_dev *wdev = NULL;
	unsigned int event_size = 0;
	char *channel_prefer_info = NULL;
	struct evt * evt_tlv = (struct evt *)channel_prefer_info;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	channel_prefer_info = os_zalloc(1024);
	if (channel_prefer_info == NULL) {
		return;
	}

	wapp->map->off_ch_scan_state.ch_scan_state = CH_SCAN_IDLE;
	evt_tlv = (struct evt *)channel_prefer_info;

	MAP_GET_RADIO_IDNFER(wdev->radio, radio_id);

	if (radar_notif->status) {
		DBGPRINT(RT_DEBUG_ERROR, "RADAR Present, channel = %d!!!\n", radar_notif->channel);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, "RADAR Absent, channel = %d!!!\n", radar_notif->channel);
	}
	update_primary_ch_status(radar_notif->channel, radar_notif->status);
	event_size = map_build_chn_pref(
	wapp, radio_id, channel_prefer_info);
	if (event_size) {
		wapp_send_1905_msg(wapp, WAPP_CHANNLE_PREFERENCE, event_size,
			(char *)evt_tlv->buffer);
	}
	os_free(channel_prefer_info);
}
void wdev_process_wsc_scan_comp(struct wifi_app *wapp, 
	u32 ifindex, 
	wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	
	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "wdev==NULL!!!\n");
		return;
	}

	wdev->wsc_scan_info.bss_count = event_data->wsc_scan_info.bss_count;
	os_memcpy(wdev->wsc_scan_info.Uuid,
		event_data->wsc_scan_info.Uuid,
		sizeof(event_data->wsc_scan_info.Uuid));
	/*
		results for WPS scan on wapp->wsc_trigger_wdev are available now,
		call wps_ctrl_run_cli_wps to check if WPS needs to be executed on next BH CLI
	*/
	wapp->wsc_trigger_wdev = wps_ctrl_run_cli_wps(wapp, wapp->wsc_trigger_wdev);
	/*
		if wps_ctrl_run_cli_wps returns a NULL WDEV, we have executed scan on all
		available BH CLI. Now we process scan results of all CLIs together.
	*/
	if (wapp->wsc_trigger_wdev == NULL) {
		printf("will process scan results now\n");
		wps_ctrl_process_scan_results(wapp);
	}
}

#ifdef MAP_R2
void map_fill_last_scan_time(struct wifi_app *wapp, u8 radio_idx)
{
	//u8 *radio_id = wapp->map->ch_scan_req->body[radio_idx].radio_id;
	// TODO: Raghav
}
// handle channel scan compelete event from driver.
void wapp_fill_ch_bw_str(struct neighbor_info *dst, 
	wdev_ht_cap ht_cap,
	wdev_vht_cap vht_cap);
u8 rssi_to_rcpi(signed char rssi);

void wdev_send_tunnel_assoc_req(struct wifi_app *wapp, u32 ifindex, u16 assoc_len, u8 *mac_addr, u8 isReassoc, u8 *assoc_buffer)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	struct tunneled_msg_tlv *tunelled_tlv=NULL;
	u8 num_payload_tlv = 1;
	u8 proto_type = 0;
	DBGPRINT(RT_DEBUG_OFF, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;
//	send oid here to fill up the wsc profile credentials from driver
	

	if(isReassoc)
		proto_type = 1;
	
	send_pkt_len = sizeof(struct tunneled_msg_tlv) + assoc_len - LENGTH_802_11;
	tunelled_tlv = os_zalloc(send_pkt_len);
	if(tunelled_tlv == NULL) {
		DBGPRINT(RT_DEBUG_ERROR,"memory alloc fail %s", __func__);
		return;
	}

	tunelled_tlv->payload_len = assoc_len - LENGTH_802_11;
	os_memcpy(&tunelled_tlv->payload[0],assoc_buffer + LENGTH_802_11, assoc_len - LENGTH_802_11);
	
	map_build_and_send_tunneled_message(wapp, mac_addr, proto_type, num_payload_tlv, tunelled_tlv);
	
	
}
#endif // MAP_R2

void wdev_handle_scan_results(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char *buf = NULL;
	struct wapp_scan_info *scan_info = NULL;
	int scan_done = 1;
#ifdef DPP_SUPPORT
	int i;
#endif /* DPP_SUPPORT */

	if (!wapp)
		goto End;
	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		goto End;

	if (!wdev->scan_cookie)
		goto End;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s for %s\n", __func__, wdev->ifname);

	scan_info = &event_data->scan_info;
	scan_info->interface_index = ifindex;
	if (scan_info->more_bss) {
		scan_done = 0;
		wapp_query_scan_result(wapp, wdev, 1);
	}
	send_pkt_len = sizeof(struct scan_bss_info)* scan_info->bss_count + sizeof(u8) + sizeof(u8) +
		sizeof(unsigned int);
	os_alloc_mem(NULL, (UCHAR **)&buf, send_pkt_len);
	os_memcpy(buf, scan_info, send_pkt_len);
#ifdef DPP_SUPPORT
	for (i = 0; i < scan_info->bss_count; i++) {
		struct bss_info_scan_result *scan_result = os_malloc(sizeof(*scan_result));
		scan_result->bss = scan_info->bss[i];
		dl_list_add_tail(&(wapp->scan_results_list), &scan_result->list);
	}
#endif /* DPP_SUPPORT */
	wapp_send_1905_msg(wapp, WAPP_SCAN_RESULT, send_pkt_len, buf);
	if (scan_done) {
#ifdef DPP_SUPPORT
		wapp_handle_dpp_scan(wapp, wdev);
		DBGPRINT(RT_DEBUG_OFF, "scan_done: %s\n", __func__);
#endif /* DPP_SUPPORT */
		wapp_send_1905_msg(wapp, WAPP_SCAN_DONE, sizeof(int), (char *)&(wdev->scan_cookie));
	}

End:
	os_free(buf);
}

int wapp_issue_scan_request(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev)
{
	char cmd[256];
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}
	/*Flag to check if scan is triggered from wapp*/
	wdev->wapp_triggered_scan = TRUE;
	sprintf(cmd, "iwpriv %s set SiteSurvey=;", wdev->ifname);
	DBGPRINT(RT_DEBUG_ERROR,"1-%s\n", cmd);
	ret = system(cmd);

	return ret;
}

/* need to set SSID in the end to apply sec settings */
int wdev_set_sec_and_ssid(

	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	struct sec_info *sec,
	char *ssid)
{
	char cmd[MAX_CMD_MSG_LEN];
	int ret;
	struct ap_dev * ap = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev || !sec) {
		return WAPP_INVALID_ARG;
	}

	ap = (struct ap_dev *)wdev->p_dev;

    DBGPRINT_RAW(RT_DEBUG_OFF,
		"set sec_info:\n"
		"\t ifname = %s\n"
		"\t auth = %s\n"
		"\t encryp = %s\n"
		"\t passphrases = %s\n",
		wdev->ifname,
		sec->auth,
		sec->encryp,
		sec->psphr);

	os_memset(cmd, 0, MAX_CMD_MSG_LEN);
	sprintf(cmd, "iwpriv %s set AuthMode=%s;", wdev->ifname, sec->auth);	
	printf("1-%s\n", cmd);
	ret = system(cmd);

        os_memset(cmd,0,MAX_CMD_MSG_LEN);
        sprintf(cmd, "wifi_config_save %s AuthMode %s", wdev->ifname, sec->auth);
        ret = system(cmd);
        DBGPRINT(RT_DEBUG_OFF," [%s] Send AuthMode Command: %s, ret = %d\n", __func__, cmd, ret);


	os_memset(cmd, 0, MAX_CMD_MSG_LEN);
	sprintf(cmd, "iwpriv %s set EncrypType=%s;", wdev->ifname, sec->encryp);
	printf("2-%s\n", cmd);
	ret = system(cmd);
        os_memset(cmd,0,MAX_CMD_MSG_LEN);
        sprintf(cmd, "wifi_config_save %s EncrypType %s", wdev->ifname, sec->encryp);
        ret = system(cmd);
        DBGPRINT(RT_DEBUG_OFF," [%s] Send EncrypType Command: %s, ret = %d\n", __func__, cmd, ret);

	if (os_strcmp(sec->encryp, "WEP") == 0) {
			os_memset(cmd, 0, MAX_CMD_MSG_LEN);
			sprintf(cmd, "iwpriv %s set DefaultKeyID=1;", wdev->ifname);
			//printf("3.1-%s\n", cmd);
			ret = system(cmd);

			os_memset(cmd,0,MAX_CMD_MSG_LEN);
			sprintf(cmd, "wifi_config_save %s DefaultKeyID 1", wdev->ifname);
			ret = system(cmd);
			DBGPRINT(RT_DEBUG_OFF," [%s] Send DefaultKeyID Command: %s, ret = %d\n", __func__, cmd, ret);

			if((os_strlen(sec->psphr)!=13)&&(os_strlen(sec->psphr)!=5)&&(os_strlen(sec->psphr)!=10)&&(os_strlen(sec->psphr)!=26)) {
				DBGPRINT(RT_DEBUG_ERROR, "%s in WEP key len should be 13 or 5 ascii OR 10 or 26 in valid hex\n", __func__);
				return WAPP_INVALID_ARG;
			}
			os_memset(cmd, 0, MAX_CMD_MSG_LEN);
			sprintf(cmd, "iwpriv %s set Key1=%s;", wdev->ifname,sec->psphr);
			//printf("3.2-%s\n", cmd);
			ret = system(cmd);

			os_memset(cmd,0,MAX_CMD_MSG_LEN);
			sprintf(cmd, "wifi_config_save %s Key1 %s", wdev->ifname, sec->psphr);
			ret = system(cmd);
			DBGPRINT(RT_DEBUG_OFF," [%s] Send Key1 Command: %s, ret = %d\n", __func__, cmd, ret);
	}
	else if (os_strcmp(sec->encryp, "WPAPSKWPA2PSK") == 0) {
		DBGPRINT(RT_DEBUG_OFF," [%s] Security is mixed mode\n", __func__);
		os_memset(cmd, 0, MAX_CMD_MSG_LEN);
		sprintf(cmd, "iwpriv %s set DefaultKeyID=2;", wdev->ifname);
		ret = system(cmd);

		os_memset(cmd,0,MAX_CMD_MSG_LEN);
		sprintf(cmd, "wifi_config_save %s DefaultKeyID 2", wdev->ifname);
		ret = system(cmd);
		DBGPRINT(RT_DEBUG_OFF," [%s] Send DefaultKeyID Command: %s, ret = %d\n", __func__, cmd, ret);

		wdev_set_psk(wapp, wdev, (char *)sec->psphr);
	}
	else {
		wdev_set_psk(wapp, wdev, (char *)sec->psphr);
	}

	/* Need hook funcion */
	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		driver_wext_set_ssid(wapp->drv_data, wdev->ifname, ssid ? ssid : ap->bss_info.ssid);
	}

	if (ret != -1 && ssid) {
		
		os_strncpy(ap->bss_info.ssid, ssid, MAX_LEN_OF_SSID);
		os_memcpy(&wdev->sec, sec, sizeof(struct sec_info));
	}

	return WAPP_SUCCESS;
}

int wdev_set_quick_ch(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	int ch)
{
	char cmd[256] = {0};
	struct dl_list *dev_list;
	struct wapp_dev *wdev_temp;
	dev_list = &wapp->dev_list;
	int ret = 0;
	unsigned int bss_coex_buffer = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		DBGPRINT(RT_DEBUG_ERROR, "%s invalid_argument\n", __func__);
		return WAPP_INVALID_ARG;
	}

	if (ch == 0) {
		DBGPRINT_RAW(RT_DEBUG_OFF,
		"channel is 0, return\n");
		return WAPP_INVALID_ARG;
	}
		
	if (ch == wdev->radio->op_ch) {
		return WAPP_SUCCESS;
	}
	DBGPRINT_RAW(RT_DEBUG_OFF,
		"set ch:\n"
		"\t ifname = %s\n"
		"\t ch = %d\n",
		wdev->ifname, ch);

	dl_list_for_each(wdev_temp, dev_list, struct wapp_dev, list) {
		if (wdev->radio == wdev_temp->radio)
			if (wdev_temp->dev_type == WAPP_DEV_TYPE_AP) {
				struct ap_dev * ap = (struct ap_dev *)wdev_temp->p_dev;
				ap->ch_info.op_ch = ch;
			}
	}

	/* Disable coex scan for this channel, since this is a configuration/auth phase */
	driver_wext_get_bss_coex(wapp->drv_data, wdev->ifname,
			(void *)&bss_coex_buffer);
	if (bss_coex_buffer)
		wdev_set_bss_coex(wapp, wdev, FALSE);

	// TODO move this code outside of MAP in driver
	os_memset(cmd,0,sizeof(cmd));
	sprintf(cmd, "iwpriv %s set MapChannel=%d;", wdev->ifname, ch);
	system(cmd);
	
        DBGPRINT(RT_DEBUG_OFF," [%s] Send Channel Command: %s, ret = %d\n", __func__, cmd, ret);
	wapp_radio_update_ch(wapp, wdev->radio, ch);

	/* Enable coex scan */
	if (bss_coex_buffer)
		wdev_set_bss_coex(wapp, wdev, TRUE);

	return WAPP_SUCCESS;
}

int wdev_set_ch(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	int ch,
	unsigned char op_class)
{
	char cmd[256];
	struct dl_list *dev_list;
	struct wapp_dev *wdev_temp;
	dev_list = &wapp->dev_list;
	int ret = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}

	if (ch == 0) {
		DBGPRINT_RAW(RT_DEBUG_OFF,
		"channel is 0, return\n");
		return WAPP_INVALID_ARG;
	}

	if (ch == wdev->radio->op_ch) {
		return WAPP_SUCCESS;
	}

	DBGPRINT_RAW(RT_DEBUG_OFF,
		"set ch:\n"
		"\t ifname = %s\n"
		"\t ch = %d\n",
		wdev->ifname, ch);
#ifdef MAP_SUPPORT
	if(wapp->map->quick_ch_change) {
#ifndef MAP_R2
		sprintf(cmd, "iwpriv %s set MapChannel=%d;", wdev->ifname, ch);
#else
		printf("dev role in wapp is %d", wapp->map->my_map_dev_role);
		sprintf(cmd, "iwpriv %s set MapChannel=%d:%d:%d", wdev->ifname, ch, !wdev->cac_not_required, wapp->map->my_map_dev_role);
		wdev->cac_not_required = 0;
#endif
	} else
#endif
	sprintf(cmd, "iwpriv %s set Channel=%d;", wdev->ifname, ch);

	system(cmd);
	
	DBGPRINT_RAW(RT_DEBUG_OFF, "%s\n", cmd);
        os_memset(cmd,0,sizeof(cmd));
        sprintf(cmd, "wifi_config_save %s Channel %d", wdev->ifname, ch);
        ret = system(cmd);
        DBGPRINT(RT_DEBUG_OFF," [%s] Send Channel Command: %s, ret = %d\n", __func__, cmd, ret);
	wapp_radio_update_ch(wapp, wdev->radio, ch);
	dl_list_for_each(wdev_temp, dev_list, struct wapp_dev, list) {
		if (wdev->radio == wdev_temp->radio)
			if (wdev_temp->dev_type == WAPP_DEV_TYPE_AP) {
				struct ap_dev * ap = (struct ap_dev *)wdev_temp->p_dev;
				ap->ch_info.op_ch = ch;
				 if (op_class != 0)
					ap->ch_info.op_class = op_class;
			}
	}
	map_operating_channel_info(wapp);

	return WAPP_SUCCESS;
}


int wdev_set_bss_coex(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	char enable)
{
	char cmd[256];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}
	os_memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "iwpriv %s set HtBssCoex=%d;", wdev->ifname, enable);
	system(cmd);
	return WAPP_SUCCESS;
}

#ifdef DPP_SUPPORT
char * wdev_get_dpp_driver_auth_type(enum dpp_akm akm)
{
	switch (akm) {
	case DPP_AKM_DPP:
		return "DPP";
	case DPP_AKM_PSK:
		return "WPA2PSK";
	case DPP_AKM_SAE:
		return "WPA3PSK";
	case DPP_AKM_PSK_SAE:
		return "WPA2PSKWPA3PSK";
	case DPP_AKM_SAE_DPP:
		return "DPPWPA3PSK";
	case DPP_AKM_PSK_SAE_DPP:
		return "DPPWPA3PSKWPA2PSK";
	default:
		return "WPA2PSK";
	}
}

int wdev_enable_apcli_iface(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	int enable)
{
	char cmd[100] = {0};

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}


	if (wdev->dev_type != WAPP_DEV_TYPE_STA) {
		DBGPRINT(RT_DEBUG_TRACE, "%s incorrect iface type =%d\n", __func__, wdev->dev_type);
		return -1;
	}	

	sprintf(cmd, "iwpriv %s set ApCliEnable=%d", wdev->ifname, enable);
	system(cmd);

	return WAPP_SUCCESS;
}

int wdev_set_dpp_akm(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	enum dpp_akm akm)
{
	char cmd[100];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}

	memset(cmd, 0, 100);

	if (wdev->dev_type == WAPP_DEV_TYPE_AP)
		sprintf(cmd, "iwpriv %s set AuthMode=%s", wdev->ifname,
			wdev_get_dpp_driver_auth_type(akm));
	else
		sprintf(cmd, "iwpriv %s set ApCliAuthMode=%s", wdev->ifname,
			wdev_get_dpp_driver_auth_type(akm));

	system(cmd);

	memset(cmd, 0, 100);
	if (wdev->dev_type == WAPP_DEV_TYPE_AP)
		sprintf(cmd, "iwpriv %s set EncrypType=AES", wdev->ifname);
	else
		sprintf(cmd, "iwpriv %s set ApCliEncrypType=AES", wdev->ifname);

	system(cmd);

	return WAPP_SUCCESS;
}
#endif /*DPP_SUPPORT*/

int wdev_set_ssid(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	char *ssid)
{

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev || !ssid) {
		return WAPP_INVALID_ARG;
	}

	DBGPRINT_RAW(RT_DEBUG_OFF,
		"set ssid:\n"
		"\t ifname = %s\n"
		"\t ssid = %s\n",
		wdev->ifname, ssid);
	wapp->drv_ops->drv_set_ssid(wapp->drv_data, wdev->ifname, ssid);

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev * ap = (struct ap_dev *)wdev->p_dev;
		os_strncpy(ap->bss_info.ssid, ssid, MAX_LEN_OF_SSID);
	}

	return WAPP_SUCCESS;
}
int wdev_set_psk(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	char *psk)
{

	if (!wapp || !wdev || !psk) {
		return WAPP_INVALID_ARG;
	}

	DBGPRINT_RAW(RT_DEBUG_OFF,
		"set psk:\n"
		"\t ifname = %s\n"
		"\t psk = %s\n",
		wdev->ifname, psk);
	wapp->drv_ops->drv_set_psk(wapp->drv_data, wdev->ifname, psk);

	return WAPP_SUCCESS;
}

int wdev_set_radio_onoff(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	int onoff)
{
	char cmd[256];

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev)
		return WAPP_INVALID_ARG;

	if (!wdev->radio)
		return WAPP_NOT_INITIALIZED;

	if (wdev->radio->onoff == onoff)
		return WAPP_SUCCESS;

#if 1
	if (onoff == RADIO_ON) {
		DBGPRINT_RAW(RT_DEBUG_OFF,
			GRN(
			"set radio:\n"
			"\t ifname = %s\n"
			"\t onoff = %d\n"),
			wdev->ifname, onoff);

	} else {
		DBGPRINT_RAW(RT_DEBUG_OFF,
			RED(
			"set radio:\n"
			"\t ifname = %s\n"
			"\t onoff = %d\n"),
			wdev->ifname, onoff);
	}
#endif

	sprintf(cmd, "iwpriv %s set RadioOn=%d;", wdev->ifname, onoff);
	system(cmd);
	wdev->radio->onoff = onoff;

	return WAPP_SUCCESS;
}

int wdev_set_bss_role(struct wapp_dev *wdev, unsigned char map_vendor_extension)
{
	char cmd[256] = {0};
	struct ap_dev *ap = NULL;
	BOOLEAN need_disconnect = FALSE;
	
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wdev) {
		return WAPP_INVALID_ARG;
	}

	ap = (struct ap_dev *)wdev->p_dev;

	if ((wdev->i_am_fh_bss && !(map_vendor_extension & (1<<MAP_ROLE_FRONTHAUL_BSS)))
		|| (wdev->i_am_bh_bss && !(map_vendor_extension & (1<<MAP_ROLE_BACKHAUL_BSS))))
	{
		need_disconnect = TRUE;
	}
	wdev->i_am_fh_bss = (map_vendor_extension & (1<<MAP_ROLE_FRONTHAUL_BSS))?1:0;
	wdev->i_am_bh_bss = (map_vendor_extension & (1<<MAP_ROLE_BACKHAUL_BSS))?1:0;

	/* This info will be sent back to mapd */
	ap->bss_info.map_role = map_vendor_extension;

	DBGPRINT_RAW(RT_DEBUG_OFF,"%s-[%s] i_am_fh_bss %d , i_am_bh_bss %d\n", 
		__func__,wdev->ifname, wdev->i_am_fh_bss, wdev->i_am_bh_bss);

	/* Reset previous values for bh and fh bss */
	if (!wdev->i_am_fh_bss) {
		sprintf(cmd, "iwpriv %s set fhbss=0;", wdev->ifname);
		system(cmd);
		os_memset(cmd, 0, 256);
	}
	sprintf(cmd, "iwpriv %s set bhbss=0;", wdev->ifname);
	system(cmd);
	os_memset(cmd, 0, 256);

	if(wdev->i_am_fh_bss) {
		sprintf(cmd, "iwpriv %s set fhbss=1;", wdev->ifname);
		system(cmd);
	}
	if(wdev->i_am_bh_bss) {
		sprintf(cmd, "iwpriv %s set bhbss=1;", wdev->ifname);
		system(cmd);
	}

	if (need_disconnect) {
		sprintf(cmd, "iwpriv %s set DisConnectAllSta=;", wdev->ifname);
		system(cmd);
	}
	return WAPP_SUCCESS;
}
int wdev_set_hidden_ssid(
	struct wapp_dev *wdev,
	unsigned char hidden_ssid)
{
	char cmd[256] = {0};
	struct ap_dev *ap = NULL;
        int ret = 0;
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
  
	if (!wdev) {
		return WAPP_INVALID_ARG;
	}
  
	ap = (struct ap_dev *)wdev->p_dev;
	ap->bss_info.hidden_ssid = hidden_ssid;

	DBGPRINT_RAW(RT_DEBUG_OFF,
		"set ifname = %s\n"
		"\t hidessid = %d\n",
		wdev->ifname, hidden_ssid);

	sprintf(cmd, "iwpriv %s set HideSSID=%d;", wdev->ifname, hidden_ssid);
	system(cmd);

        os_memset(cmd,0,sizeof(cmd));
        sprintf(cmd, "wifi_config_save %s HideSSID %d", wdev->ifname, hidden_ssid);
        ret = system(cmd);
        DBGPRINT(RT_DEBUG_OFF," [%s] Send HideSSID Command: %s, ret = %d\n", __func__, cmd, ret);

	return WAPP_SUCCESS;
}
void wdev_he_cap_query_rsp_handle(struct wifi_app *wapp, u32 ifindex, wdev_he_cap *he_cap_rcvd)
{
	struct wapp_dev *wdev = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);

	if (!wdev)
		return;

	if (wdev->dev_type == WAPP_DEV_TYPE_AP) {
		struct ap_dev *ap = (struct ap_dev *)wdev->p_dev;
		wdev_he_cap *he_cap;
		he_cap = &ap->he_cap;
		os_memcpy(he_cap, he_cap_rcvd, sizeof(wdev_he_cap));
		DBGPRINT_RAW(RT_DEBUG_OFF,
				"he_cap (%u):\n"
				"\t tx_stream = %u, rx_stream = %u, he_160 = %u, he_8080 = %u, he_mcs_len = %u\n",
				ifindex,
				he_cap->tx_stream,
				he_cap->rx_stream,
				he_cap->he_160,
				he_cap->he_8080,
				he_cap->he_mcs_len)
	}
}
void wdev_handle_cac_period(struct wifi_app *wapp, u32 ifindex, wapp_event_data *event_data)
{
	struct wapp_dev *wdev = NULL;
	int send_pkt_len = 0;
	char* buf = NULL;

	if (!wapp)
		return;

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, ifindex);
	if (!wdev)
		return;

	send_pkt_len = sizeof(struct _wapp_cac_info);
	buf = os_zalloc(send_pkt_len);
	if (buf) {
		os_memcpy(buf, &event_data->cac_info, send_pkt_len);
		DBGPRINT(RT_DEBUG_TRACE, "%s cac_enable:%d \n", __func__, event_data->cac_info.ret);
		wapp_send_1905_msg(wapp, WAPP_CAC_PERIOD_ENABLE, send_pkt_len, buf);
		os_free(buf);
	}
}
