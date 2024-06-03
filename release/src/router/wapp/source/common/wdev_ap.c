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


int wdev_ap_del(struct wifi_app	*wapp, struct wapp_dev	*wdev);


struct wdev_ops wdev_ap_ops = {
	.wdev_del = wdev_ap_del,
};
void wdev_handle_nop_channels(struct nop_channel_list_s *nop_channels)
{
	int i = 0;
	for (i = 0; i < nop_channels->channel_count; i ++)
	{
		update_primary_ch_status(nop_channels->channel_list[i],
			FALSE);
	}
}

int wdev_ap_create(
	struct wifi_app *wapp,
	wapp_dev_info *dev_info)
{
	struct wapp_dev *wdev = NULL;
	struct ap_dev *ap = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !dev_info) {
		return WAPP_INVALID_ARG;
	}

	wdev = wapp_dev_list_lookup_by_ifindex(wapp, dev_info->ifindex);

	if (wdev == NULL)
		goto new_wdev;
	else if (wdev->p_dev == NULL)
		goto new_bss;
	else if (wdev->dev_type != WAPP_DEV_TYPE_AP) {
		DBGPRINT_RAW(RT_DEBUG_OFF, "Error! dev_type = %u\n", wdev->dev_type);
		/* check duplicate mac_address? */
		return WAPP_UNEXP;
	} else {
		/* already exsist */
		return WAPP_SUCCESS;
	}

new_wdev:
	if (wapp_dev_create(wapp, (char *) dev_info->ifname, dev_info->ifindex, dev_info->mac_addr) != WAPP_SUCCESS)
		DBGPRINT_RAW(RT_DEBUG_OFF, "Warning! wdev create failed.\n");
	return WAPP_SUCCESS;

new_bss:
	wdev->p_dev = os_zalloc(sizeof(struct ap_dev));

	if (!wdev->p_dev)
		return WAPP_RESOURCE_ALLOC_FAIL;

	ap = wdev->p_dev;
	wdev->dev_type = WAPP_DEV_TYPE_AP;
	os_memset(ap, 0, sizeof(struct ap_dev));
	/*set the default value*/
	ap->isActive = dev_info->dev_active;
	wdev->ops = &wdev_ap_ops;
	wdev->radio = wapp_radio_update_or_create(wapp,
						dev_info->adpt_id,
						dev_info->radio_id);
	if (!wdev->radio) {
		DBGPRINT_RAW(RT_DEBUG_OFF, "Warning! radio not found.\n");
	}
	wdev->wireless_mode = dev_info->wireless_mode;
	wapp_reset_backhaul_config(wapp, wdev);

	/* get max table size to build client table */
#if 0	
	wapp_query_wdev_by_req_id(wapp, (char *)dev_info->ifname, WAPP_MISC_CAP_QUERY_REQ);
	wapp_query_wdev_by_req_id(wapp, (char *)dev_info->ifname, WAPP_HT_CAP_QUERY_REQ);
	wapp_query_wdev_by_req_id(wapp, (char *)dev_info->ifname, WAPP_VHT_CAP_QUERY_REQ);
	wapp_query_wdev_by_req_id(wapp, (char *)dev_info->ifname, WAPP_CHN_LIST_QUERY_REQ);
	wapp_query_wdev_by_req_id(wapp, (char *)dev_info->ifname, WAPP_OP_CLASS_QUERY_REQ);
	wapp_query_wdev_by_req_id(wapp, (char *)dev_info->ifname, WAPP_BSS_INFO_QUERY_REQ);
	wapp_query_wdev_by_req_id(wapp, (char *)dev_info->ifname, WAPP_AP_METRIC_QUERY_REQ);
#else
	wdev_ht_cap ht_cap;
	wdev_vht_cap vht_cap;
	wdev_misc_cap misc_cap;
	wdev_chn_info chn_list;
	wdev_op_class_info op_class;
	wdev_bss_info bss_info;
	wdev_ap_metric ap_metrics;
	struct nop_channel_list_s nop_channels;
	u32 chip_id = 0;
	wdev_he_cap he_cap;
	os_memset(&he_cap, 0, sizeof(wdev_he_cap));
	os_memset(&ht_cap, 0, sizeof(wdev_ht_cap));
	os_memset(&vht_cap, 0, sizeof(wdev_vht_cap));
	os_memset(&misc_cap, 0, sizeof(wdev_misc_cap));
	os_memset(&chn_list, 0, sizeof(wdev_chn_info));
	os_memset(&op_class, 0, sizeof(wdev_op_class_info));
	os_memset(&bss_info, 0, sizeof(wdev_bss_info));
	os_memset(&ap_metrics, 0, sizeof(wdev_ap_metric));
	os_memset(&nop_channels, 0, sizeof(struct nop_channel_list_s));

	wapp_get_misc_cap(wapp, (char *)dev_info->ifname, (char *)&misc_cap , sizeof(wdev_misc_cap));
	wdev_misc_cap_query_rsp_handle(wapp, dev_info->ifindex, &misc_cap);

	wapp_get_ht_cap(wapp, (char *)dev_info->ifname, (char *)&ht_cap , sizeof(wdev_ht_cap));
	wdev_ht_cap_query_rsp_handle(wapp, dev_info->ifindex, &ht_cap);

	wapp_get_vht_cap(wapp, (char *)dev_info->ifname, (char *)&vht_cap , sizeof(wdev_vht_cap));
	wdev_vht_cap_query_rsp_handle(wapp, dev_info->ifindex, &vht_cap);

	wapp_get_he_cap(wapp, (char *)dev_info->ifname, (char *)&he_cap , sizeof(wdev_he_cap));
	wdev_he_cap_query_rsp_handle(wapp, dev_info->ifindex, &he_cap);

	wapp_get_chan_list(wapp, (char *)dev_info->ifname, (char *)&chn_list , sizeof(wdev_chn_info));
	wdev_chn_list_query_rsp_handle(wapp, dev_info->ifindex, &chn_list);

	wapp_get_nop_channels(wapp, (char *)dev_info->ifname, (char *)&nop_channels,
			sizeof(nop_channels));
	wdev_handle_nop_channels(&nop_channels);

	wapp_get_op_class(wapp, (char *)dev_info->ifname, (char *)&op_class , sizeof(wdev_op_class_info));
	wdev_op_class_query_rsp_handle(wapp, dev_info->ifindex, &op_class);
	
	wapp_get_bss_info(wapp, (char *)dev_info->ifname, (char *)&bss_info , sizeof(wdev_bss_info));
	wdev_bss_info_query_rsp_handle(wapp, dev_info->ifindex, &bss_info);
	
	wapp_get_ap_metrics(wapp, (char *)dev_info->ifname, (char *)&ap_metrics , sizeof(wdev_ap_metric));
	wdev_ap_metric_query_rsp_handle(wapp, dev_info->ifindex, &ap_metrics);
	wapp_get_chip_id(wapp, (char *)dev_info->ifname, (char*)&chip_id , sizeof(chip_id));
	wdev->radio->chip_id = chip_id;
#endif
	wapp_set_ap_config(wapp, wdev);
	wdev->valid = 1;
	return WAPP_SUCCESS;
}

int wdev_ap_del(
	struct wifi_app *wapp,
	struct wapp_dev *wdev)
{
	struct ap_dev *ap = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}

	ap = (struct ap_dev *) wdev->p_dev;

	/* free client list */
	wdev_ap_client_table_release(wapp, ap);

	/* free ap */
	os_free(ap);
	wdev->p_dev = NULL;

	return WAPP_SUCCESS;
}


int wdev_ap_update(
	struct wifi_app *wapp,
	struct ap_dev *ap,
	u8 action,
	u32 len,
	void *data)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap) {
		return WAPP_INVALID_ARG;
	}

	switch(action)
	{
#if 0
		case MBO_CDC_UPDATE:
		{
			if (len == sizeof(u8))
				sta->cell_data_cap = *((u8 *) data);
			break;
		}

		case MBO_NPC_APPEND:
		{
			if (len == sizeof( struct non_pref_ch)){
				struct non_pref_ch_entry *npc_entry;

				npc_entry = os_zalloc(sizeof(struct non_pref_ch_entry));
				os_memcpy(&npc_entry->npc, data, len);
				dl_list_add_tail(&sta->non_pref_ch_list, &npc_entry->list);
			}
			
			break;
		}

		case MBO_BSSID_UPDATE:
		{
			if (len == MAC_ADDR_LEN){
				os_memcpy(&sta->bssid, data, len);
			}
			
			break;
		}

		case MBO_AKM_UPDATE:
		{
			if (len == sizeof(u32))
				sta->akm = *((u32 *) data);
			break;
		}
		
		case MBO_CIPHER_UPDATE:
		{
			if (len == sizeof(u32))
				sta->cipher = *((u32 *) data);
			break;
		}	
#endif
		default:
			break;
	}

	return WAPP_SUCCESS;
}

int wdev_ap_block_list_init(
	struct wifi_app *wapp,
	struct ap_dev *ap)
{
	int i = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap)
		return WAPP_INVALID_ARG;

	if (ap->max_num_of_block_cli) {
		for (i = 0; i < ap->max_num_of_block_cli; i++)
			os_memset(ap->block_table[i].mac_addr, 0, MAC_ADDR_LEN);
	} else {
		DBGPRINT_RAW(RT_DEBUG_ERROR, "ERROR! max_num_of_block_cli is zero.\n");
		return WAPP_INVALID_ARG;
	}

	return WAPP_SUCCESS;
}

int wapp_del_block_sta(
	struct ap_dev *ap,
	unsigned char *sta_addr)
{
	int i = 0;
	int num = ap->max_num_of_block_cli;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	for (i = 0; i < num; i++) {
		if (!os_memcmp(ap->block_table[i].mac_addr, sta_addr, MAC_ADDR_LEN)) {
			os_memset(ap->block_table[i].mac_addr, 0, MAC_ADDR_LEN);
			ap->block_table[i].valid_period = 0;
			ap->num_of_block_cli--;
			break;
		}
	}

	return WAPP_SUCCESS;
}

int wapp_add_block_sta(
	struct wifi_app *wapp,
	struct ap_dev *ap,
	u8 *sta_addr,
	u16 valid_period)
{
	struct wapp_block_sta *block_sta = NULL;
	int i = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap) {
		return WAPP_INVALID_ARG;
	}

	if (ap->num_of_block_cli >= ap->max_num_of_block_cli) {
		DBGPRINT(RT_DEBUG_ERROR, "Cli number  is more than table size\n");
		return WAPP_RESOURCE_ALLOC_FAIL; 
	}

	/* Find out the First Avail Entry */
	for (i = 0; i < ap->max_num_of_block_cli; i++)
	{
		if (is_all_zero_mac(ap->block_table[i].mac_addr)) {
			block_sta = &ap->block_table[i];
			break;
		}
	}
	if (!block_sta) {
		DBGPRINT(RT_DEBUG_ERROR, "Cli number  is more than table size\n");
		return WAPP_RESOURCE_ALLOC_FAIL; 
	}

	os_memcpy(block_sta->mac_addr, sta_addr, MAC_ADDR_LEN);
	block_sta->valid_period = valid_period;

	ap->num_of_block_cli++;
	return WAPP_SUCCESS;
}

struct wapp_block_sta* wdev_ap_block_list_lookup(
	struct wifi_app *wapp,
	struct ap_dev *ap,
	u8 *mac_addr)
{
	int i;
	struct wapp_block_sta *block_sta = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap || !mac_addr)
		return block_sta;

	for (i = 0; i < ap->max_num_of_block_cli; i++)
	{
		if (os_memcmp(ap->block_table[i].mac_addr, mac_addr, MAC_ADDR_LEN) == 0) {
			block_sta = &ap->block_table[i];
			break;
		}
	}

	return block_sta;
}


int wdev_ap_client_table_create(
struct wifi_app *wapp,
struct ap_dev	*ap)
{
	u32 mem_size;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap) {
		return WAPP_INVALID_ARG;
	}

	mem_size = (sizeof(struct wapp_sta *) * CLIENT_TABLE_SIZE);
	ap->client_table = os_zalloc(mem_size);
	if (!ap->client_table) {
		return WAPP_RESOURCE_ALLOC_FAIL;
	}
	os_memset(ap->client_table, 0, mem_size);

	return WAPP_SUCCESS;
}

int wdev_ap_client_table_release(
struct wifi_app *wapp,
struct ap_dev	*ap)
{
	int i;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap) {
		return WAPP_INVALID_ARG;
	}

	if (!ap->client_table) {
		return WAPP_SUCCESS;
	}

	/* free sta entries */
	for (i = 0; i < CLIENT_TABLE_SIZE; i++)
	{
		if (ap->client_table[i] != NULL) {
			os_free(ap->client_table[i]);
			ap->client_table[i] = NULL;
		}
	}
	
	/* free table itself */
	os_free(ap->client_table);
	ap->client_table = NULL;

	return WAPP_SUCCESS;
}

int wapp_client_create(
	struct wifi_app *wapp,
	struct ap_dev	*ap,
	struct wapp_sta **new_sta)
{
	int i;
	struct wapp_sta *sta_temp = NULL;
	*new_sta = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap) {
		return WAPP_INVALID_ARG;
	}

	if (!ap->client_table) {
		return WAPP_NOT_INITIALIZED;
	}

	for (i = 0; i < CLIENT_TABLE_SIZE; i++)
	{
		/* if there is an old entry, use it */
		sta_temp = ap->client_table[i];
		if (sta_temp && (sta_temp->sta_status == WAPP_STA_INVALID)) {
			*new_sta = sta_temp;
			break;
		} else if (!sta_temp) {
			/* alloc a new entry */
			ap->client_table[i] = os_zalloc(sizeof(struct wapp_sta));
			*new_sta = ap->client_table[i];
			break;
		}
	}

	if ((*new_sta) == NULL) {
		/* find the oldest one*/
		*new_sta = ap->client_table[0];
		for (i = 1; i < CLIENT_TABLE_SIZE; i++) {
			sta_temp = ap->client_table[i];
			if (os_time_before(&sta_temp->last_update_time, &(*new_sta)->last_update_time))
				*new_sta = sta_temp;
		}
		os_memset(*new_sta, 0, sizeof(struct wapp_sta));	
		
	}
	else {
		ap->num_of_clients++;
	}

	return WAPP_SUCCESS;
}

int wapp_client_clear(
	struct wifi_app *wapp,
	struct ap_dev	*ap,
	u8 *mac_addr)
{
	struct wapp_sta *sta = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	sta = wdev_ap_client_list_lookup(wapp, ap, mac_addr);

	if (sta) {
		os_memset(sta->mac_addr, 0, MAC_ADDR_LEN);
		sta->sta_status = WAPP_STA_INVALID;
		sta->assoc_time = 0;
		if (sta->beacon_report) {
			free(sta->beacon_report);
			sta->beacon_report = NULL;
		}
		/* reuse the memory, don't free. */
	}

	ap->num_of_clients--;

	return WAPP_SUCCESS;
}


void wapp_fill_client_info(
	struct wifi_app *wapp,
	wapp_client_info *cli_info,
	struct wapp_sta *sta)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	COPY_MAC_ADDR(sta->mac_addr, cli_info->mac_addr);
	COPY_MAC_ADDR(sta->bssid, cli_info->bssid);
	sta->sta_status = cli_info->sta_status;
	sta->assoc_time = cli_info->assoc_time;
	sta->assoc_req_len = cli_info->assoc_req_len;
	if (sta->assoc_req_len > ASSOC_REQ_LEN_MAX)
	sta->assoc_req_len = ASSOC_REQ_LEN_MAX;
	os_memcpy(&sta->cli_caps, &cli_info->cli_caps, sizeof(struct map_cli_cap));
	sta->downlink = cli_info->downlink;
	sta->uplink = cli_info->uplink;
	sta->uplink_rssi = cli_info->uplink_rssi;
	sta->bBSSMantSupport = (cli_info->cli_caps.btm_capable == 1 ? TRUE : FALSE);
	sta->bLocalSteerDisallow = cli_info->bLocalSteerDisallow;
	sta->bBTMSteerDisallow = cli_info->bBTMSteerDisallow;
	os_get_time(&sta->last_update_time);

	/*traffic stats*/
	sta->bytes_sent = cli_info->bytes_sent;
	sta->bytes_received = cli_info->bytes_received;
	sta->packets_sent = cli_info->packets_sent;
	sta->packets_received = cli_info->packets_received;
	sta->tx_packets_errors = cli_info->tx_packets_errors;
	sta->rx_packets_errors = cli_info->rx_packets_errors;
	sta->retransmission_count = cli_info->retransmission_count;
	sta->link_availability = cli_info->link_availability;
	sta->tx_tp = cli_info->tx_tp;
	sta->rx_tp = cli_info->rx_tp;
	sta->is_APCLI = cli_info->is_APCLI;
}

struct wapp_sta*  wdev_ap_client_list_lookup_for_all_bss(
	struct wifi_app *wapp,
	const u8 *mac_addr)
{
	struct wapp_sta *cli = NULL;
	struct wapp_dev *wdev;
	struct dl_list *dev_list;
	struct ap_dev *ap = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !mac_addr)
		return cli;

	dev_list = &wapp->dev_list;

	dl_list_for_each(wdev, dev_list, struct wapp_dev, list) {
		if (wdev && wdev->dev_type == WAPP_DEV_TYPE_AP)
			ap = (struct ap_dev *) wdev->p_dev;
		else
			continue;

		cli = wdev_ap_client_list_lookup(wapp, ap, mac_addr);
		if (cli)
		{
			return cli;
		}
	}

	return cli;
}

struct wapp_sta*  wdev_ap_client_list_lookup(
	struct wifi_app *wapp,
	struct ap_dev	*ap,
	const u8 *mac_addr)
{
	int i;
	struct wapp_sta *cli = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap || !mac_addr)
		return cli;

	if (!ap->client_table)
		return cli;

	for (i = 0; i < CLIENT_TABLE_SIZE; i++)
	{
		cli = ap->client_table[i];
		if (cli && (os_memcmp(cli->mac_addr, mac_addr, MAC_ADDR_LEN) == 0))
			break;
	}

	return cli;
}

int wdev_show_wapp_sta_info(
	struct wapp_sta *cli)
{
	struct non_pref_ch_entry *npc_entry;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!cli)
		return WAPP_INVALID_ARG;

	DBGPRINT_RAW(RT_DEBUG_OFF,
		"\t mac_addr = %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\t bssid = %02x:%02x:%02x:%02x:%02x:%02x\n"
		"\t status = %u\n"
		"\t assoc_time = %u\n"
		"\t downlink = %u\n"
		"\t uplink = %u\n"
		"\t rssi = %d\n"
		"\t bytes_sent = %u\n"
		"\t bytes_received = %u\n"
		"\t packets_sent = %u\n"
		"\t packets_received = %u\n"
		"\t tx_packets_errors = %u\n"
		"\t rx_packets_errors = %u\n"
		"\t retransmission_count = %u\n"
		"\t link_availability = %u\n"
		"\t last_update_time = %lu Sec\n"
		"\t bBSSMantSupport = %u\n"
		"\t bLocalSteerDisallow = %u\n"
		"\t bBTMSteerDisallow = %u\n",
		PRINT_MAC(cli->mac_addr),
		PRINT_MAC(cli->bssid),
		cli->sta_status,
		cli->assoc_time,
		cli->downlink,
		cli->uplink,
		cli->uplink_rssi,
		cli->bytes_sent,
		cli->bytes_received,
		cli->packets_sent,
		cli->packets_received,
		cli->tx_packets_errors,
		cli->rx_packets_errors,
		cli->retransmission_count,
		cli->link_availability,
		cli->last_update_time.sec,
		cli->bBSSMantSupport,
		cli->bLocalSteerDisallow,
		cli->bBTMSteerDisallow);
	DBGPRINT_RAW(RT_DEBUG_OFF, "non_pref_ch_list: (ch/pref/reason)\n");
		if (!dl_list_empty(&cli->non_pref_ch_list))
			dl_list_for_each(npc_entry, &cli->non_pref_ch_list, struct non_pref_ch_entry, list) {
			DBGPRINT_RAW(RT_DEBUG_OFF, "%u/%u/%u ", npc_entry->npc.ch, npc_entry->npc.pref, npc_entry->npc.reason_code);
		} else
			DBGPRINT_RAW(RT_DEBUG_OFF, "ch list is empty:");
		DBGPRINT_RAW(RT_DEBUG_OFF, "\n");

	return WAPP_SUCCESS;
}

int wdev_ap_show_block_list(
	struct wifi_app *wapp,
	struct ap_dev	*ap)
{
	int i = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap) {
		return WAPP_INVALID_ARG;
	}

	DBGPRINT_RAW(RT_DEBUG_OFF, "Block List:\n");
	for (i = 0; i < ap->max_num_of_block_cli; i++) {
		if (is_all_zero_mac(ap->block_table[i].mac_addr)) {
			continue;
		}
		DBGPRINT_RAW(RT_DEBUG_OFF, "(%d) sta: %02x:%02x:%02x:%02x:%02x:%02x\n", i, PRINT_MAC(ap->block_table[i].mac_addr));
	}

	return WAPP_SUCCESS;
}

int wdev_ap_show_cli_list(
	struct wifi_app *wapp,
	struct ap_dev	*ap)
{
	int i;
	struct wapp_sta *cli;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap) {
		return WAPP_INVALID_ARG;
	}

	if (!ap->client_table) {
		return WAPP_NOT_INITIALIZED;
	}

	for (i = 0; i < CLIENT_TABLE_SIZE; i++)
	{
		cli = ap->client_table[i];
		if (cli) {
			DBGPRINT_RAW(RT_DEBUG_OFF, "cli_info:\n");
			wdev_show_wapp_sta_info(cli);
		}
	}

	return WAPP_SUCCESS;
}

int wdev_ap_show_chn_list(
	struct wifi_app *wapp,
	struct ap_dev	*ap)
{
	int j = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap) {
		return WAPP_INVALID_ARG;
	}

    DBGPRINT_RAW(RT_DEBUG_OFF,
		"chn_list_info:\n"
		"\t op_ch =    %u\n"
		"\t op_class = %u\n"
		"\t band =   %u\n"
		"\t ch_list_num =   %u\n"
		"\t non_op_chn_num =   %d\n",
		ap->ch_info.op_ch,
		ap->ch_info.op_class,
		ap->ch_info.band,
		ap->ch_info.ch_list_num,
		ap->ch_info.non_op_chn_num);

	for(j = 0; j < ap->ch_info.ch_list_num; j++) {
		printf("channel = %d \n", ap->ch_info.ch_list[j].channel);
	}
	return WAPP_SUCCESS;
}

int wdev_ap_show_bss_info(
	struct wifi_app *wapp,
	struct ap_dev	*ap)
{
    int i = 0;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap) {
		return WAPP_INVALID_ARG;
	}

    printf("ap->num_of_bss = %d\n", ap->num_of_bss);
    for(i = 0; i < ap->num_of_bss; i++) {
		DBGPRINT_RAW(RT_DEBUG_OFF,
			"op_bss_info[%d]: \n"
			"\t Bssid = [%02x][%02x][%02x][%02x][%02x][%02x]\n"
			"\t ssid = %s\n"
			"\t SsidLen = %u\n",
			i,
			PRINT_MAC(wapp->map->op_bss_table[i].bssid),
			wapp->map->op_bss_table[i].ssid,
			wapp->map->op_bss_table[i].SsidLen);
    }
	return WAPP_SUCCESS;
}

int wdev_ap_show_ap_metric(
	struct wifi_app *wapp,
	struct ap_dev	*ap)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap) {
		return WAPP_INVALID_ARG;
	}

    DBGPRINT_RAW(RT_DEBUG_TRACE,
		"\t Bssid = [%02x][%02x][%02x][%02x][%02x][%02x]\n"
		"\t CU = %u\n"
		"\t ESPI_AC(BE) = [%02x][%02x][%02x]\n"
		"\t ESPI_AC(BK) = [%02x][%02x][%02x]\n"
		"\t ESPI_AC(VI) = [%02x][%02x][%02x]\n"
		"\t ESPI_AC(VO) = [%02x][%02x][%02x]\n",
		ap->ap_metrics.bssid[0], ap->ap_metrics.bssid[1], ap->ap_metrics.bssid[2],
		ap->ap_metrics.bssid[3], ap->ap_metrics.bssid[4], ap->ap_metrics.bssid[5],
		ap->ap_metrics.cu,
		ap->ap_metrics.ESPI_AC[0][0], ap->ap_metrics.ESPI_AC[0][1], ap->ap_metrics.ESPI_AC[0][2],
		ap->ap_metrics.ESPI_AC[1][0], ap->ap_metrics.ESPI_AC[1][1], ap->ap_metrics.ESPI_AC[1][2],
		ap->ap_metrics.ESPI_AC[2][0], ap->ap_metrics.ESPI_AC[2][1], ap->ap_metrics.ESPI_AC[2][2],
		ap->ap_metrics.ESPI_AC[3][0], ap->ap_metrics.ESPI_AC[3][1], ap->ap_metrics.ESPI_AC[3][2]);

	return WAPP_SUCCESS;
}

int wdev_ap_show_ap_info(
	struct wifi_app *wapp,
	struct wapp_dev *wdev,
	struct ap_dev	*ap)
{
	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !ap) {
		return WAPP_INVALID_ARG;
	}

	DBGPRINT_RAW(RT_DEBUG_OFF,
		"ap_info:\n"
		"\t wireless_mode =		%u\n"
		"\t num_of_cli =		%u\n"
		"\t num_of_assoc_cli =	%u\n"
		"\t max_num_of_cli =        %u\n"
		"\t max_num_of_bss =        %u\n"
		"\t num_of_bss =            %u\n"
		"\t max_num_of_block_cli =  %u\n"
		"\t num_of_block_cli =      %u\n"
		"\t pwr =                   %d\n",
		wdev->wireless_mode,
		ap->num_of_clients,
		ap->num_of_assoc_cli,
		ap->max_num_of_cli,
		ap->max_num_of_bss,
		ap->num_of_bss,
		ap->max_num_of_block_cli,
		ap->num_of_block_cli,
		ap->pwr.tx_pwr);

	return WAPP_SUCCESS;
}

int wdev_ap_set_txpwr_limit(
	struct wifi_app *wapp,
	struct wapp_dev	*wdev,
	char pwr_limit)
{
	struct ap_dev *ap = NULL;

	DBGPRINT(RT_DEBUG_TRACE, "%s\n", __func__);
	if (!wapp || !wdev) {
		return WAPP_INVALID_ARG;
	}

	if (wdev->dev_type != WAPP_DEV_TYPE_AP)
		return WAPP_INVALID_ARG;

	ap = (struct ap_dev *) wdev->p_dev;

	if (ap->pwr.pwr_limit == pwr_limit) {
		printf("%s: same pwr limit %d dB, do nothing\n", __func__, pwr_limit);
	} else {
		ap->pwr.pwr_limit = pwr_limit;
#if 1 // use pwr percentage as emp sol. nned to find another way to control per-bss / per-channel pwr
		if (pwr_limit < 6) { /*Used Lowest Power Percentage.*/
			wapp_set_tx_power_percentage(wapp, wdev, 6);
		} else if (pwr_limit < 9) {
			wapp_set_tx_power_percentage(wapp, wdev, 10);
		} else if (pwr_limit < 12) {
			wapp_set_tx_power_percentage(wapp, wdev, 25);
		} else if (pwr_limit < 14) {
			wapp_set_tx_power_percentage(wapp, wdev, 50);
		} else if (pwr_limit < 15) {
			wapp_set_tx_power_percentage(wapp, wdev, 75);
		} else {
			wapp_set_tx_power_percentage(wapp, wdev, 100);
		}
#endif
		printf(BLUE("%s: change pwr limit to %d dB\n"), __func__, pwr_limit);
	}

	return WAPP_SUCCESS;
}


