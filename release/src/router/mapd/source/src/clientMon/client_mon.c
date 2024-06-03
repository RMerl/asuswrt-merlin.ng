/*
***************************************************************************
*  Mediatek Inc.
* 4F, No. 2 Technology 5th Rd.
* Science-based Industrial Park
* Hsin-chu, Taiwan, R.O.C.
*
* (c) Copyright 2002-2011, Mediatek, Inc.
*
* All rights reserved. Mediatek's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of Ralink Tech. Any attemp
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of Mediatek, Inc. is obtained.
***************************************************************************

                Module Name:
                Channel Monitor

                Abstract:
                Per client monitoring

                Revision History:
                Who         When          What
                --------    ----------    -----------------------------------------
                Neelansh.M   2018/05/02     First implementation of the client monitor Module
*/
#include "includes.h"
#include "common.h"
#include "steer_fsm.h"
#include "list.h"
#include "client_db.h"
#include "mapd_i.h"
#include "./../1905_local_lib/data_def.h"
#include "chan_mon.h"
#include "wapp_if.h"
#ifdef SUPPORT_MULTI_AP
#include "topologySrv.h"
#endif
#include "client_mon.h"
#include "ap_roam_algo.h"
#include "ap_est.h"
#include "steer_action.h"
#ifdef SUPPORT_MULTI_AP
#include "tlv_parsor.h"
#include "mapd_user_iface.h"
#endif
#include <assert.h>
#include "1905_map_interface.h"

void client_mon_handle_activity_change(struct mapd_global *global, uint32_t client_id, uint8_t act_stat)
{

	struct client *cli = &global->dev.client_db[client_id];
	cli->activity_state = act_stat;
}


/**
 * @brief : Handling a received probe request, update the clientDB according to the
 * probe and initiate pre assoc steering if required. Called from wlanif
 *
 * @param global : global device pointer
 * @param mac_addr : mac address of the client for which probe is received 
 * @param channel : channel on which the probe request is received
 * @param rssi : RSSI at which the probe is received 
 * @param preq_len : length of the probe request
 * @param preq : pointer to the probe request frame 
 * @param now : current time at which the event is received, used for timestamping
 */
void client_mon_handle_client_preq(struct mapd_global *global, uint8_t *mac_addr,
		uint8_t channel, uint8_t rssi, uint8_t preq_len, uint8_t *preq,
		struct os_reltime now)

{
	uint8_t idx = 0;
	struct mapd_radio_info *radio_info;
	struct client *cli;
	u8 already_seen = 0;
	uint32_t client_id;
	unsigned char *bssid;
	struct mapd_radio_info *preq_ra_info = NULL;
	uint8_t band_idx = 0;

	/* Do not add locally administered mac addresses:
	 * Stations use Random mac addresses while scanning */
#if 0
	if (is_local_adm_ether_addr(mac_addr))
		return ;
#endif
	if (mapd_get_radio_from_channel(global, channel) == NULL) {
		debug("ERROR ********************** Invalid channel=%d*********************", channel);
		return;
	}
		

	/* Add to client DB: client_db_track_add */
	client_id = client_db_track_add(global, mac_addr, &already_seen);
	if (client_id == (uint32_t)-1) {
		 mapd_printf(MSG_ERROR, "No more room to accomodate" MACSTR
					", that is discovered", MAC2STR(mac_addr));
		 return;
	}
	if (already_seen != 1) {
		 mapd_printf(MSG_DEBUG, "New Client discovered"
					MACSTR, MAC2STR(mac_addr));
	}

	cli = &global->dev.client_db[client_id];

	/* Update known bands and known channels */
	if(channel < 14) {
		cli->known_bands |= BAND_2G_SUPPORTED;
		band_idx = BAND_2G_IDX;
	} else {
		cli->known_bands |= BAND_5G_SUPPORTED;
		band_idx = BAND_5G_IDX;
	}

	/* Update PHY CAPs */
	if (preq_len > 24 && preq_len <= PREQ_IE_LEN)
		client_db_update_cli_ht_vht_cap(global, client_id, preq + 24, preq_len - 24, band_idx);
	else {
		mapd_printf(MSG_ERROR, "************* Invalid Preq length ***********");
		//assert(0);
		return;
	}
	
	/* Sure to support the channel on which preq is rx */
	client_db_set_known_channels(global, client_id, channel);

	bssid = client_db_get_bssid(global, client_id);
	/* Not connected and Remote Steering not in Progress */
	if (is_zero_ether_addr(bssid) 
#ifdef SUPPORT_MULTI_AP
		&& cli->coord_data.cli_coordination_state == COORD_STATE_IDLE
#endif
	) {
		preq_ra_info = mapd_get_radio_from_channel(global, channel);
		if(preq_ra_info == NULL) {
			err("not got channel from radio");
			return;
			}
		/* Update RSSI on the channel on which preq is recvd */
		client_db_set_ul_rssi(global, cli->client_id, rssi, preq_ra_info->radio_idx, 1);	

		/* Use past learning */
		for (idx = 0; idx < MAX_NUM_OF_RADIO; ++idx) 
		{
			radio_info = &global->dev.dev_radio_info[idx];
			if (radio_info->radio_idx == (uint8_t)-1)
				continue;
			if (radio_info->radio_idx == preq_ra_info->radio_idx)
				continue;
			if (!is_chan_supported(cli->known_channels, radio_info->channel))
				continue;
			/* Estimate if the client is known to operate on this radio idx and
			 * if the RSSI is too old or unknown */
			if (os_reltime_expired(&now, &cli->rssi_ts[idx],
							global->dev.cli_steer_params.RSSIAgeLim_preAssoc)) {
				mapd_printf(MSG_DEBUG, "Unassoc client =%d is known to operate"
								"on %d channel and RSSI is too old -- Estimate its RSSI",
								cli->client_id, radio_info->channel);
				ap_est_update_non_serving_rssi(global,
									cli,  preq_ra_info->radio_idx, idx);
				mapd_printf(MSG_DEBUG, "Estimated RSSI =%d", cli->ul_rssi[idx]);
			}
		}
		/* Trigger pre-asoc str on preq recv(rssi-update) */
#ifdef CENT_STR
		if(!global->dev.cent_str_en)
#endif
		ap_roam_algo_chk_preassoc_str(global, cli);
	}
}
static coarse_phy_mode phy_mode_mapper(enum phy_mode phy_mode)
{
	switch(phy_mode) {
		case MODE_CCK:
		case MODE_OFDM:
			return LEGACY_MODE;
		case MODE_HTMIX:
		case MODE_HTGREENFIELD:
			return HT_MODE;
		case MODE_VHT:
			return VHT_MODE;
		case MODE_HE:
			return HE_MODE;
		default:
			return HT_MODE;
	}
}

/**
 * @brief : Called when new assoc is received (from wlanif) or when link metrics are
 * received for a client not in the DB. Add the client to clientDB, remove from
 * unassoc lists, add to assoc list, remove blacklists, reset statistics and update
 * info from the assoc IEs
 *
 * @param global : global device pointer
 * @param mac_addr : mac address of the client for which assoc received
 * @param bssid : BSSID on which connected
 * @param assoc_req : pointer to the assoc request
 * @param assoc_req_len : length of the assoc request
 *
 * @return 
 */
uint8_t client_mon_handle_local_join(struct mapd_global *global, u8 *mac_addr, u8 *bssid,
				u8 valid, u8 bBtm, u8 bRrm, u8 bMbo,
				int nss, enum phy_mode phy_mode, enum max_bw bw, struct map_he_nss *nss_he)
{
	 uint32_t client_id;
	 u8 *old_bssid;
	 u8 already_seen = 0;
	 uint8_t channel = 0;
	 struct client *cli = NULL;
	 /* Add to client DB: client_db_track_add */
	 client_id = client_db_track_add(global, mac_addr, &already_seen);
	 if (client_id == (uint32_t)-1) {
		  mapd_printf(MSG_ERROR, "No more room to accomodate" 
						  MACSTR ", that is leaving a BSS on my device", MAC2STR(mac_addr));
		  return -1;
	 }
	 if (already_seen != 1) {
		  mapd_printf(MSG_ERROR, "Assoc for a client that has not been seen yet"
					 MACSTR, MAC2STR(mac_addr));
	 }
	 cli = client_db_get_client_from_client_id(global, client_id);
	 cli->in_db = IN_DB;
	 if (already_seen != 1)
		 cli->dirty = 1;

	 /* Remove all BLs due to STEER_ALGO*/
	 client_mon_bl_sta_for_all_bss(global, cli, 0, NULL, FALSE);

	 old_bssid = client_db_get_bssid(global, client_id);
	 if (!is_zero_ether_addr(old_bssid)) {
		  mapd_printf(MSG_ERROR, "Got local join for client" MACSTR
					 ", that has not left me/remote BSS ", MAC2STR(mac_addr));
		  client_db_remove_from_assoc_list(global, client_id);
		  if (os_memcmp(old_bssid, bssid, ETH_ALEN)) {
				  mapd_printf(MSG_ERROR, "New bssid " MACSTR
								  " .Issue local disconnect on old bssid " MACSTR,
								  MAC2STR(bssid), MAC2STR(old_bssid));
				  wlanif_deauth_sta(global, cli->mac_addr, old_bssid);
		  }
	 }
	 /* Clear PostAssoc Params */
	 client_db_clear_post_assoc_params(global, client_id);
	 /* Add to local assoc List */
	 mapd_add_client_to_bss_assoc_list(global, client_id, bssid);
	 client_db_set_bssid(global, client_id, bssid);
	 /* Client is connected to me -- It is no longer remote */
	 cli->is_remote = 0;
	 /* Update current Channel */
	 channel = mapd_get_channel_from_bssid(global, bssid);
	 client_db_set_curr_channel(global, client_id, channel);
	 /* Update known_channels */
	 client_db_set_known_channels(global, client_id, channel);
	 /* Update current radio_idx */
	 client_db_set_radio_idx(global, client_id,
					 mapd_get_radio_idx_from_bssid(global, bssid));

	 /* Update Caps */
	 if (valid) {
		 client_db_set_capab(global, client_id, bBtm, bRrm, bMbo);
		 client_db_set_phy_capab(global, client_id, 2,
						 (channel > 14 ? BAND_5G_IDX : BAND_2G_IDX),
						 bw, nss, phy_mode_mapper(phy_mode));
		 if(phy_mode_mapper(phy_mode) == HE_MODE) {
			 mapd_printf(MSG_ERROR, "NSS_HE80: %d NSS_HE160: %d NSS_HE8080: %d\n", nss_he->nss_80, nss_he->nss_160, nss_he->nss_8080);
			client_db_set_he_phy_capab(global, client_id, nss_he);
		 }
		 steer_action_sta_join(global, cli, 0);
	 }

	/* Trigger FSM */
	 mapd_printf(MSG_INFO, "Trigger FSM (CLIENT_ASSOCIATED)");
	 steer_fsm_trigger(global, client_id, CLIENT_ASSOCIATED, NULL);
	 return client_id;
}

/**
 * @brief : In the event that a client leaves one of the BSS on our system, remove
 * the client from assoc lists(if required), and trigger the state machine for the
 * client
 *
 * @param global : global device pointer
 * @param mac_addr : mac adress of the client leaving
 */
void client_mon_handle_local_leave(struct mapd_global *global, u8 *mac_addr,
				u8 *leaving_bssid)
{
	 uint32_t client_id = 0;
	 u8 already_seen = 0;
	 u8 *curr_bssid = NULL;
	 struct client *cli = NULL;
#ifdef SUPPORT_MULTI_AP
	struct bss_info_db *bss_info = NULL;
#endif

	 /* Add to client DB: client_db_track_add */
	 /* XXX : Why is the following required here? */
	 client_id = client_db_track_add(global, mac_addr, &already_seen);
	 if (client_id == (uint32_t)-1) {
		  mapd_printf(MSG_ERROR, "No more room to accomodate" MACSTR
					 ", that is leaving a BSS on my device", MAC2STR(mac_addr));
		  return;
	 }

	 curr_bssid = client_db_get_bssid(global, client_id);
	 cli = client_db_get_client_from_client_id(global, client_id);
	 cli->in_db = IN_DB;
	curr_bssid = client_db_get_bssid(global, client_id);

	 /* Stale Local Leave */
	 if (!curr_bssid || os_memcmp(curr_bssid, leaving_bssid, ETH_ALEN)) {
		 mapd_printf(MSG_OFF, "Stale Local leave");
		 return;
	 }
#ifdef SUPPORT_MULTI_AP
	 bss_info = topo_srv_get_bss_by_bssid(&global->dev, NULL, leaving_bssid);
	 if (bss_info) {
			 always("inform local wifi sta leave: ssid: %s\n", bss_info->ssid);
			 mapd_user_wireless_client_leave(global, mac_addr,
							 leaving_bssid, (char *)bss_info->ssid);
	 }
#endif
	 if (already_seen != 1) {
		  mapd_printf(MSG_ERROR, "Dissaoc for a client that has not been seen yet"
					 MACSTR, MAC2STR(mac_addr));
		  mapd_printf(MSG_INFO, "Trigger FSM (CLIENT_DISSOCIATED)");
		  steer_fsm_trigger(global, client_id, CLIENT_DISSOCIATED, NULL);
		  cli->dirty = 1;
		  return;
	 }
	 /* Remove from assoc list */
	 client_db_remove_from_assoc_list(global, client_id);
	 /* Trigger FSM */
	 mapd_printf(MSG_INFO, "Trigger FSM (CLIENT_DISSOCIATED)");
	 steer_fsm_trigger(global, client_id, CLIENT_DISSOCIATED, NULL);
	 /* Clear PostAssoc Tmp params */
	 client_db_clear_post_assoc_params(global, client_id);
}
#ifdef SUPPORT_MULTI_AP
/**
 * @brief : Handle
 *
 * @param global
 * @param mac_addr
 * @param bssid
 * @param assoc_req
 * @param assoc_req_len
 */
void client_mon_handle_remote_join(struct mapd_global *global, u8 *mac_addr,
		  u8 *bssid, u8 * al_mac)
{
	 uint32_t client_id;
	 u8 already_seen = 0;
	 u8 *old_bssid = NULL;
	 struct client *cli = NULL;

	 /* Add to client DB: client_db_track_add */
	 client_id = client_db_track_add(global, mac_addr, &already_seen);
	 if (client_id == (uint32_t)-1) {
		  mapd_printf(MSG_ERROR, "No more room to accomodate" MACSTR
					 ", that has joined a BSS on another device", MAC2STR(mac_addr));
		  return;
	 }
	 cli = client_db_get_client_from_client_id(global, client_id);

	 cli->in_db = IN_DB;
	 if (already_seen != 1) {
		  mapd_printf(MSG_ERROR, "Remote JOIN client notific for a client that has not"
		  			 "been seen yet" MACSTR, MAC2STR(mac_addr));
		  cli->dirty = 1;

	 }
	 /* Remove all BLs due to STEER_ALGO*/
	 client_mon_bl_sta_for_all_bss(global, cli, 0, NULL, FALSE);

	 old_bssid = client_db_get_bssid(global, client_id);
	 if (!is_zero_ether_addr(old_bssid)) {
		  mapd_printf(MSG_ERROR, "Got remote client JOIN notific for client" MACSTR
					 ", that has not left a local/remote bss ", MAC2STR(mac_addr));
		  client_db_remove_from_assoc_list(global, client_id);
	 }
	 /* Clear PostAssoc Tmp params */
	 client_db_clear_post_assoc_params(global, client_id);
	 /* Update BSSID */
	 cli->is_remote = 1;
	 client_db_set_bssid(global, client_id, bssid);

#if 0
	 //TODO 
	 //channel = get_From_topo_server(bssid);
	 /* Update from Assoc IEs */
	 if(assoc_req_len > 28)
		 client_db_update_from_ies(global, client_id, assoc_req + 28,
				 assoc_req_len -28, channel);
#endif
#ifndef CENT_STR
	steer_action_sta_join(global, cli, 1);
	/* Trigger FSM */
	 mapd_printf(MSG_INFO, "Trigger FSM (REMOTE_TOPOLOGY_NOTIFICATION)");
	 steer_fsm_trigger(global, client_id, REMOTE_TOPOLOGY_NOTIFICATION, NULL);
#else
	if (!global->dev.cent_str_en || global->dev.device_role == DEVICE_ROLE_AGENT) {
		steer_action_sta_join(global, cli, 1);
		/* Trigger FSM */
		 mapd_printf(MSG_INFO, "Trigger FSM (REMOTE_TOPOLOGY_NOTIFICATION)");
		 steer_fsm_trigger(global, client_id, REMOTE_TOPOLOGY_NOTIFICATION, NULL);
	}		
#endif
}

/**
 * @brief 
 *
 * @param global
 * @param mac_addr
 */
void client_mon_handle_remote_leave(struct mapd_global *global, struct client_assoc *assoc_notif)
{
	 uint32_t client_id;
	 u8 *old_bssid;
	 u8 already_seen = 0;
	 struct client *cli = NULL;

	 /* Add to client DB: client_db_track_add */
	 client_id = client_db_track_add(global, assoc_notif->sta_addr, &already_seen);
	 if (client_id == (uint32_t)-1) {
		  mapd_printf(MSG_ERROR, "No more room to accomodate" MACSTR
					 ", that has left a remote BSS", MAC2STR(assoc_notif->sta_addr));
		  return;
	 }

	 cli = client_db_get_client_from_client_id(global, client_id);
	 cli->in_db = IN_DB;

	 if (already_seen != 1) {
		  mapd_printf(MSG_ERROR, "Remote client LEAVE notific for a client that has not"
		  			 "been seen yet" MACSTR, MAC2STR(assoc_notif->sta_addr));
		  cli->dirty = 1;
		  return;
	 }
	 /* Remove from local assoc list(just in-case we missed both local leave and remote join) */
	
	 if(cli && cli->is_remote == 0) {
		debug("Delayed remote topology notification... STA is already connected to us.ignore");
		return;
	 }
	 old_bssid = client_db_get_bssid(global, client_id);
#ifdef CENT_STR
	if (global->dev.cent_str_en && os_memcmp(old_bssid, assoc_notif->bssid, ETH_ALEN) == 0) {
#endif
	 if (!is_zero_ether_addr(old_bssid)) {
		  mapd_printf(MSG_ERROR, "Got remote client LEAVE notific for client" MACSTR
					 ", that has not left a local/remote bss ", MAC2STR(assoc_notif->sta_addr));
		  client_db_remove_from_assoc_list(global, client_id);
	 }

	 /* DO NOT Remove all BLs: Remote Steering would get affected */
	 /* Trigger FSM */
	 mapd_printf(MSG_INFO, "Trigger FSM (REMOTE_TOPOLOGY_NOTIFICATION)");
//	 steer_fsm_trigger(global, client_id, REMOTE_TOPOLOGY_NOTIFICATION, NULL);
#ifdef CENT_STR
	 if((global->dev.cent_str_en && global->dev.device_role == DEVICE_ROLE_CONTROLLER))
		 steer_fsm_trigger(global, client_id, CLIENT_DISSOCIATED, NULL);
#endif
	 /* Clear PostAssoc Tmp params */
	 client_db_clear_post_assoc_params(global, client_id);
	 cli->is_remote = 0;
#ifdef CENT_STR
	}
#endif
}



void client_mon_handle_topo_notification(struct mapd_global *global,struct topo_notification *evt)
{
	u8 i;
	for (i = 0; i < evt->assoc_cnt; i++) {
		/* Ignore BACKHAUL STAs */
		/* All all STA/Backhaul STA as well*/
		/* Remove Backhaul STA using topology response */
#if 0
		if (is_local_adm_ether_addr(evt->assoc[i].sta_addr))
			return ;
#endif
#ifdef MAP_R2
		struct _1905_map_device *_1905_dev = topo_srv_get_1905_device(&global->dev, evt->al_mac);
#endif
		if(evt->assoc[i].is_joined) {
#ifdef CENT_STR
			if (!global->dev.cent_str_en || global->dev.device_role == DEVICE_ROLE_AGENT)
#endif
			client_mon_handle_remote_join(global,evt->assoc[i].sta_addr, evt->assoc[i].bssid, evt->al_mac);
#ifdef CENT_STR
			else{
#ifdef MAP_R2
				if (global->dev.map_version == DEV_TYPE_R2 && _1905_dev && _1905_dev->map_version == DEV_TYPE_R2) {
					struct bss_info_db *bss = topo_srv_get_bss(_1905_dev, (char *)evt->assoc[i].bssid);
					if (bss) {
						client_mon_handle_remote_join(global, evt->assoc[i].sta_addr, evt->assoc[i].bssid, _1905_dev->_1905_info.al_mac_addr);
						uint32_t client_id = client_db_get_cid_from_mac(global, evt->assoc[i].sta_addr);
						if (client_id != (uint32_t)-1)
							client_db_set_curr_channel(global, client_id, bss->radio->channel[0]);
					}
					err("MAP R2 devices, Will get tunneled Msg");
				} else
#endif
				map_1905_Send_Client_Capability_Query_Message(global->_1905_ctrl,(char *) evt->al_mac, evt->assoc[i].bssid, evt->assoc[i].sta_addr);
				
			}
#endif
		} else {
			client_mon_handle_remote_leave(global, &evt->assoc[i]);
		}
	}
}
#endif

void client_mon_handle_link_metrics(struct mapd_global *global, u8 *sta_mac,
				uint32_t dl_phy_rate, int8_t ul_rssi, u8 *bssid)
{
	uint8_t radio_idx = 0;
	struct client *cli = NULL;
	uint8_t client_id = client_db_get_cid_from_mac(global, sta_mac);

	if (client_id == (uint8_t)-1) {
		client_id = client_mon_handle_local_join(global, sta_mac,
						bssid, 0, 0, 0, 0, 0, 0, 0, NULL);
		if (client_id == (uint8_t)-1) {
			mapd_printf(MSG_ERROR, "new assoc seen-->But no more room");
			return;
		}
		mapd_printf(MSG_WARNING, "new assoc seen-->assigned_id=%d",
						client_id);
	}

	cli = client_db_get_client_from_client_id(global, client_id);

	if (cli && os_memcmp(cli->bssid, bssid, ETH_ALEN)) {
		mapd_printf(MSG_DEBUG, "Err: Different bssid(present in 2 wapp wdev tables)"
						MACSTR, MAC2STR(cli->mac_addr));
		return;
	}
	if (cli && cli->cli_steer_state > CLI_STATE_IDLE) {
		mapd_printf(MSG_DEBUG, "Steering in progress for " MACSTR,
						MAC2STR(cli->mac_addr));
		return;
	}

	radio_idx = mapd_get_radio_idx_from_bssid(global, bssid);
	if(radio_idx > MAX_NUM_OF_RADIO) {
		mapd_printf(MSG_ERROR, "radio_idx (%d) invalid", radio_idx);
		return;
	}
	client_db_set_dl_phy_rate(global, client_id, dl_phy_rate);
	client_db_set_ul_rssi(global, client_id, ul_rssi, radio_idx, FALSE);
}
void client_mon_handle_tp_metrics(struct mapd_global *global, u8 *sta_mac,
				uint32_t tx_tp, uint32_t rx_tp, u8 *bssid)
{
	uint32_t client_id = client_db_get_cid_from_mac(global, sta_mac);
	struct client *cli = NULL;

	if (client_id == (uint32_t)-1) {
		client_id = client_mon_handle_local_join(global, sta_mac,
						bssid, 0, 0, 0, 0, 0, 0, 0, NULL);
		if (client_id == (uint32_t)-1) {
			mapd_printf(MSG_ERROR, "new assoc seen-->But no more room");
			return;
		}
		mapd_printf(MSG_WARNING, "new assoc seen-->assigned_id=%d",
						client_id);
	}

	cli = client_db_get_client_from_client_id(global, client_id);

	if (os_memcmp(cli->bssid, bssid, ETH_ALEN)) {
		mapd_printf(MSG_DEBUG, "Err: Different bssid(present in 2 wapp wdev tables)"
						MACSTR, MAC2STR(cli->mac_addr));
		return;
	}
	if (cli->cli_steer_state > CLI_STATE_IDLE) {
		mapd_printf(MSG_DEBUG, "Steering in progress for " MACSTR,
						MAC2STR(cli->mac_addr));
		return;
	}

	cli->dl_rate = (u16)((u32)tx_tp >> 17); //Convert to Mbps
	cli->ul_rate = (u16)((u32)rx_tp >> 17);
	/* Update the Activity Status */
	if (tx_tp + rx_tp > global->dev.cli_steer_params.ActivityThreshold)
		cli->curr_activity_state = 1;
	else
		cli->curr_activity_state = 0;
}

void client_mon_get_assoc_stats(struct mapd_global *global)
{
	uint8_t i = 0;
	struct mapd_radio_info *ra_info = NULL;
	struct mapd_bss *bss = NULL;
	struct client *cli = NULL;
	uint32_t rssi_refresh_num_clients = 0;
	struct os_reltime now;

	os_get_reltime(&now);

	for(i = 0; i < MAX_NUM_OF_RADIO; i++) {
		ra_info = &global->dev.dev_radio_info[i];
		if (ra_info->radio_idx == (uint8_t)-1)
			continue;
		/* Get Assoc STA Stats */
		wlanif_get_all_assoc_sta_link_metrics(global, ra_info->identifier); 
#ifdef MAP_R2
		wlanif_get_all_assoc_sta_ext_link_metrics(global, ra_info->identifier);
#endif
		/* Get STA TP matrics */
		wlanif_get_all_assoc_sta_tp_metrics(global, ra_info->identifier);
		/* Get Traffic stats for all STAs */
		wlanif_get_assoc_sta_traffic_stats(global, ra_info->identifier);
		dl_list_for_each(bss, &ra_info->bss_list, struct mapd_bss, bss_entry) {
			dl_list_for_each(cli, &bss->assoc_sta_list, struct client, assoc_sta_entry) {
				cli->curr_air_time = ap_est_get_airtime(global, cli->ul_rate,
								cli->dl_rate, cli->dl_phy_rate);
				/* Monitor RSSI freshness */
				if (global->dev.cli_steer_params.ForcedRssiUpdate) {
						if ((os_reltime_expired(&now, &cli->rssi_ts[cli->radio_idx],
												global->dev.cli_steer_params.RSSIAgeLim)) &&
							(os_reltime_expired(&now, &cli->null_frame_trigger_ts, 30)) &&
							(rssi_refresh_num_clients * global->dev.cli_steer_params.RSSIMeasureSamples < 50)) { //only 50 in 1 burst
								/* Trigger NULL frame */
								wlanif_trigger_null_frames(global, cli->mac_addr, cli->bssid,
												global->dev.cli_steer_params.RSSIMeasureSamples);
								os_get_reltime(&cli->null_frame_trigger_ts);
								rssi_refresh_num_clients++;
						}
				}
			}
		}
	}
}

void client_mon_chk_post_assoc_str(struct mapd_global *global)
{
	struct steer_cands *cand = NULL;
	struct own_1905_device *ctx = &global->dev;
	struct client *arr_cand_list[MAX_STA_SEEN];
	int cand_cnt = 0, i;

	if(global->dev.SetSteer==STEER_DISABLE){
		mapd_printf(MSG_INFO, "Steering is disabled");
		return;
	}

	/* Check if post-assoc steering is disabled */
	if (global->dev.cli_steer_params.disable_post_assoc_strng)
		return;

	/* Steering already in progress */
	cand = NULL;
#ifdef SUPPORT_MULTI_AP
	SLIST_FOREACH(cand, &ctx->steer_cands_head, next_cand) {
	if (cand->steer_cand->cli_steer_method == MANDATE)
		return;
	}
#endif
	ap_roam_algo_select_steer_candidate(global, arr_cand_list, &cand_cnt);
	for (i = 0 ; i < cand_cnt ; i++) {
		cand = NULL;
		mapd_printf(MSG_OFF, "Trigger Steering for client =%d "MACSTR,
					arr_cand_list[i]->client_id, MAC2STR(arr_cand_list[i]->mac_addr));
		/* Once steering is complete, Below 2 shall be reset 
		 * Also, send steer complete when only_one_in_win is set*/
		cand = os_zalloc(sizeof(*cand));
		cand->steer_cand = arr_cand_list[i];
		os_memcpy(cand->steer_cand_home_bssid, arr_cand_list[i]->bssid, ETH_ALEN);
		SLIST_INSERT_HEAD(&ctx->steer_cands_head, cand, next_cand);
		steer_fsm_trigger(global, arr_cand_list[i]->client_id,
					mapd_get_trigger_from_steer_method(global,
							arr_cand_list[i]->cli_steer_method), NULL);
	}
}

/**
 * @brief: Install/Uninstall BL for this client on all BSSs except, excluding_bssids 
 * @param global
 * @param cli
 * @param num_bssid
 * @param excluding_bssid_arr[][ETH_ALEN]
 * @param blacklist
 */
void client_mon_bl_sta_for_all_bss(struct mapd_global *global, struct client *cli,
		u8 num_bssid, u8 excluding_bssid_arr[][ETH_ALEN], Boolean blacklist)

{
	uint8_t idx = 0, j = 0;
	struct mapd_bss *bss = NULL;
	uint8_t num_bss = 0;
	struct mapd_bss *bss_arr[MAX_NUM_BSS];
	struct mapd_radio_info *radio_info = NULL;


	for (idx = 0; idx < MAX_NUM_OF_RADIO; ++idx)
	{
		radio_info = &global->dev.dev_radio_info[idx];
		if (radio_info->radio_idx == (uint8_t)-1)
			continue;
		dl_list_for_each(bss, &radio_info->bss_list, struct mapd_bss, bss_entry)
		{
			for (j = 0; j < num_bssid; j++) {
				u8 *bssid = (u8 *)excluding_bssid_arr[j];
				if (!os_memcmp(bss->bssid, bssid, ETH_ALEN))
					break;
			}
			if (j < num_bssid){
				mapd_printf(MSG_INFO, "Excl BSSID" MACSTR, MAC2STR(bss->bssid));
			} else {
				bss_arr[num_bss++] = bss;
			}
		}
	}
	client_mon_bl_sta_for_bss(global, cli, num_bss, bss_arr, blacklist);
}

Boolean is_cli_bl_map_assoc_control(struct mapd_global *global, struct client *cli)
{
	int i = 0;
	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		struct mapd_radio_info *ra_info = NULL;
		ra_info = &global->dev.dev_radio_info[i];
		if (ra_info->radio_idx == (uint8_t)-1)
			continue;
		struct mapd_bss *bss = NULL;
		dl_list_for_each(bss, &ra_info->bss_list, struct mapd_bss, bss_entry) {
				struct bl_client *bl_sta = NULL;
				dl_list_for_each(bl_sta, &bss->bl_sta_list, struct bl_client,
								list_entry) {
						if (bl_sta->cli != cli)
							continue;
						if (bl_sta->bl_reason & BL_MAP_ASSOC_CONTROL)
							return TRUE;
				}
		}
	}
	return FALSE;
}

void client_mon_force_unblock_cli_on_all_bss(struct mapd_global *global, struct client *cli)
{
	int i = 0;

	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		struct mapd_radio_info *ra_info = NULL;
		struct mapd_bss *bss = NULL;

		ra_info = &global->dev.dev_radio_info[i];
		if (ra_info->radio_idx == (uint8_t)-1)
			continue;

		dl_list_for_each(bss, &ra_info->bss_list, struct mapd_bss, bss_entry) {
			struct bl_client *bl_sta_curr = NULL, *bl_sta_next = NULL;
			dl_list_for_each_safe(bl_sta_curr, bl_sta_next, &bss->bl_sta_list,
							struct bl_client, list_entry) {
				struct map_dev *map_dev_curr = NULL, *map_dev_next = NULL;
				if (bl_sta_curr->cli != cli)
					continue;
				dl_list_for_each_safe(map_dev_curr, map_dev_next, &bl_sta_curr->map_dev_list,
								struct map_dev, map_dev_entry) {
					dl_list_del(&map_dev_curr->map_dev_entry);
					os_free(map_dev_curr);
				}
				if (wlanif_bl_sta_for_bss(global, bl_sta_curr->cli->mac_addr,
										bss->bssid, 0) == 0) {
					mapd_printf(MSG_DEBUG," %d(" MACSTR
								") UNBLACKLISTED ON BSSID=" MACSTR,
								bl_sta_curr->cli->client_id,
								MAC2STR(bl_sta_curr->cli->mac_addr),
								MAC2STR(bss->bssid));
				} else {
					mapd_printf(MSG_ERROR, "%d(" MACSTR ") UN-BLACKLISTING ON BSSID="
								MACSTR " FAILED", cli->client_id,
								MAC2STR(cli->mac_addr),
								MAC2STR(bss->bssid));
				}
				dl_list_del(&bl_sta_curr->list_entry);
				os_free(bl_sta_curr);
			}
		}
	}
}


void client_mon_block_check(struct mapd_global *global)
{
	int i = 0;

	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		struct mapd_radio_info *ra_info = NULL;
		struct mapd_bss *bss = NULL;
		ra_info = &global->dev.dev_radio_info[i];
		if (ra_info->radio_idx == (uint8_t)-1)
			continue;
		dl_list_for_each(bss, &ra_info->bss_list, struct mapd_bss, bss_entry) {
				struct bl_client *bl_sta = NULL, *bl_sta_next = NULL;
				dl_list_for_each_safe(bl_sta, bl_sta_next, &bss->bl_sta_list, struct bl_client,
								list_entry) {
						struct map_dev *tmp = NULL, *next = NULL;
						if (!(bl_sta->bl_reason & BL_MAP_ASSOC_CONTROL))
							continue;
						dl_list_for_each_safe(tmp, next, &bl_sta->map_dev_list,
										struct map_dev, map_dev_entry) {
								if (tmp->duration <= 0) //Blocked till unblocked
									continue;
								tmp->duration--;
								if (tmp->duration == 0) {
									mapd_printf(MSG_INFO, "BL done by al_mac " MACSTR
													" timed out for " MACSTR
													" on " MACSTR, MAC2STR(tmp->al_mac),
													MAC2STR(bl_sta->cli->mac_addr),
													MAC2STR(bss->bssid));
									client_mon_unblock_cli_on_bss(global, bss,
													bl_sta->cli,
													BL_MAP_ASSOC_CONTROL,
													tmp->al_mac);
								}
						}
				}
		}
	}
}
/**
 * @brief : This function is used to deduce the channels supported by a connected
 * STA. Should only be called for clients that support Active Beacon Requests.
 *
 * @param global : global device pointer
 * @param cli : struct client
 */
static int client_mon_chan_id(struct mapd_global *global, struct client *cli)
{
	u8 chan_id_bssid[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	struct mapd_radio_info *ra_info = NULL;
	uint8_t i = 0;

	for (i = 0; i < MAX_NUM_OF_RADIO; i++) {
		ra_info = &global->dev.dev_radio_info[i];
		if (ra_info->radio_idx == (uint8_t)-1)
			continue;

		if (is_chan_supported(cli->known_channels, ra_info->channel)) {
			continue;
		}
		/* Already tried; Treat this channel as unsupported */
		if (cli->chan_id_tries[i] >= 3)
			continue;

		/* Trigger an Unaccounted(response would be ignored) active meas req
		 * This will hopefuly trigger probe req from the STA */
		mapd_printf(MSG_ERROR, "Trigger Channel ID on Channel %d for " MACSTR, ra_info->channel,
					MAC2STR(cli->mac_addr));
		wlanif_beacon_metrics_query(global, cli->mac_addr, cli->bssid,
						0, NULL,
						ra_info->channel,
						chan_mon_get_op_class_frm_channel(ra_info->channel, BW_20),
						chan_id_bssid,
						0, 0, NULL, 0, NULL);
		cli->chan_id_tries[i]++;
		return 1;
	}
	return 0;
}

/**
 * @brief : Periodic maintenance of the clientDB, called from the periodic thread
 *
 * @param global : Global device pointer
 */
void client_mon_cli_db_maintenance(struct mapd_global *global)
{
	struct client *cli = NULL;
	int i;
	struct os_reltime now;

	os_get_reltime(&now);

	for(i = 0; i < MAX_STA_SEEN; i++) {
		cli = &global->dev.client_db[i];
		if (cli->client_id == (uint32_t)-1) {
			continue;
		}

		/* Use Active Meas Request to identify channels supported by the STA */
		if (global->params.Certification == 0 && (!is_zero_ether_addr(cli->bssid)) &&
			(cli->cli_steer_state == CLI_STATE_IDLE) &&
			(cli->capab & CLI_CAP_11K) &&
			(os_reltime_expired(&now, &cli->chan_id_trigger_ts, 10)) &&
			(client_mon_chan_id(global, cli)))
			 cli->chan_id_trigger_ts = now;
#ifdef SUPPORT_MULTI_AP
		// check rfs_req_timestamp;
		if(cli->coord_data.cli_coordination_state == COORD_STATE_RFS_RECEIVED 
			&& os_reltime_expired(&now, &cli->coord_data.rfs_req_timestamp, TSQ_REQ_TIMEOUT_VAL)) {
			mapd_printf(MSG_ERROR,"rfs req received but no tsq req. Clear blacklist");
			client_mon_bl_sta_for_all_bss(global, cli, 0, NULL, FALSE);
			cli->coord_data.cli_coordination_state = COORD_STATE_IDLE;
			cli->coord_data.rfs_req_timestamp.sec = 0;
			cli->coord_data.rfs_req_timestamp.usec = 0;
		}
		if(cli->coord_data.cli_coordination_state == COORD_STATE_ASSOC_CONTROL_RECEIVED
			&& os_reltime_expired(&now, &cli->coord_data.assoc_cntrl_req_timestamp, ASSOC_BLOCK_DURATION)) {
			mapd_printf(MSG_OFF, "sta mac: " MACSTR", reset COORD_STATE_IDLE ", MAC2STR(cli->mac_addr));
			cli->coord_data.cli_coordination_state = COORD_STATE_IDLE;
			cli->coord_data.assoc_cntrl_req_timestamp.sec = 0;
			cli->coord_data.assoc_cntrl_req_timestamp.usec = 0;
		}
#endif
		/* Activity State */
		if (cli->curr_activity_state == 1) {
			cli->activity_state = 1; //mark active
			cli->consec_idle_count = 0;
		} else {
			cli->consec_idle_count++;
			if(cli->consec_idle_count >= global->dev.cli_steer_params.idle_count_th) {
				cli->activity_state = 0; //mark idle
				cli->consec_idle_count = 0;
			}
		}

	}
	/* User config Block Check */
	client_mon_block_check(global);
}

#ifdef MAP_R2
void client_mon_handle_auth_rej(struct mapd_global *global, wapp_sta_cnnct_rej_info *rej_cnnct_info)
{
	struct client *cli = client_db_get_client_from_sta_mac(global, rej_cnnct_info->sta_mac);

	if (cli == NULL) {
		mapd_printf(MSG_ERROR, "Auth Rej for client " MACSTR
						" not in DB", MAC2STR(rej_cnnct_info->sta_mac));
		return;
	}
	if(rej_cnnct_info->cnnct_fail.connect_stage <= WAPP_ASSOC) {
	//Increment reject count stored in the DB
		cli->auth_deny_count ++;
		mapd_printf(MSG_INFO, "Auth Rejected count=%d for client=%d " MACSTR,
					cli->auth_deny_count, cli->client_id, MAC2STR(rej_cnnct_info->sta_mac));
		// If the reject count exceeds the max reject count for this client, remove BL
		if (cli->auth_deny_count >= cli->auth_deny_max) {
				if (cli->cli_steer_state >= CLI_STATE_IDLE) {
						mapd_printf(MSG_INFO, "Trigger FSM (AUTH_DENY_MAX_REACHED)");
						steer_fsm_trigger(global, cli->client_id, AUTH_DENY_MAX_REACHED, NULL);
				} else {
						 /* Remove all BLs due to STEER_ALGO*/
						 client_mon_bl_sta_for_all_bss(global, cli, 0, NULL, FALSE);
				}

		}
	}
#ifdef MAP_R2
	debug("###assoc_status_code = %d ####\n",
		rej_cnnct_info->assoc_status_code);
	debug(MACSTR, MAC2STR(rej_cnnct_info->sta_mac));
	if (rej_cnnct_info->send_failed_assoc_frame == 1) {
		struct own_1905_device *dev = &global->dev;
		Send_Failed_assoc_message(global->_1905_ctrl, dev, rej_cnnct_info->sta_mac,
								rej_cnnct_info->assoc_status_code,
								rej_cnnct_info->assoc_reason_code);
	}
#endif //MAPR2
}

#else
/**
 * @brief : Handle auth reject event received via the wlanif 
 *
 * @param global : global device pointer
 * @param mac_addr: mac address of the client for which auth rejection is done
 */
//XXX: TODO: Handle auth rejects for multi-ap ?
void client_mon_handle_auth_rej(struct mapd_global *global, u8 *mac_addr)
{
	struct client *cli = client_db_get_client_from_sta_mac(global, mac_addr);

	if (cli == NULL) {
		mapd_printf(MSG_ERROR, "Auth Rej for client " MACSTR
						" not in DB", MAC2STR(mac_addr));
		return;
	}
	//Increment reject count stored in the DB
	cli->auth_deny_count ++;
	mapd_printf(MSG_INFO, "Auth Rejected count=%d for client=%d " MACSTR,
				cli->auth_deny_count, cli->client_id, MAC2STR(mac_addr));
	// If the reject count exceeds the max reject count for this client, remove BL
	if (cli->auth_deny_count >= cli->auth_deny_max) {
			if (cli->cli_steer_state >= CLI_STATE_IDLE) {
					mapd_printf(MSG_INFO, "Trigger FSM (AUTH_DENY_MAX_REACHED)");
					steer_fsm_trigger(global, cli->client_id, AUTH_DENY_MAX_REACHED, NULL);
			} else {
					 /* Remove all BLs due to STEER_ALGO*/
					 client_mon_bl_sta_for_all_bss(global, cli, 0, NULL, FALSE);
			}

	}
}
#endif
void client_mon_unblock_cli_on_bss(struct mapd_global *global,
		struct mapd_bss *bss, struct client *cli, u32 reason, u8 *al_mac)
{
	struct bl_client *bl_sta = NULL;

	dl_list_for_each(bl_sta, &bss->bl_sta_list, struct bl_client,
			list_entry) {
		if (bl_sta->cli == cli) {
#ifdef SUPPORT_MULTI_AP
				if (reason == BL_MAP_ASSOC_CONTROL) {
						struct map_dev *tmp = NULL, *next = NULL;
						if (!al_mac)
							return;
						/* The reasaon is MAP_ASSOC_CONTROL */
						dl_list_for_each_safe(tmp, next, &bl_sta->map_dev_list,
										struct map_dev, map_dev_entry) {
								if (!(os_memcmp(al_mac, tmp->al_mac, ETH_ALEN))) {
										dl_list_del(&tmp->map_dev_entry);
										os_free(tmp);
										break;
								}
						}
						if (!dl_list_empty(&bl_sta->map_dev_list))
								return;
				}
#endif
				if (bl_sta->bl_reason  == reason) {
						/* I am the only reason */
					if (wlanif_bl_sta_for_bss(global, cli->mac_addr, bss->bssid, 0) == 0) {
						mapd_printf(MSG_DEBUG," %d(" MACSTR
									") UNBLACKLISTED ON BSSID=" MACSTR,
									cli->client_id, MAC2STR(cli->mac_addr),
									MAC2STR(bss->bssid));
						dl_list_del(&bl_sta->list_entry);
						os_free(bl_sta);
					} else {
						mapd_printf(MSG_ERROR, "%d(" MACSTR ") UN-BLACKLISTING ON BSSID="
									MACSTR " FAILED", cli->client_id,
									MAC2STR(cli->mac_addr),
									MAC2STR(bss->bssid));
						/* Failed ; try again after 10 seconds */
#ifdef SUPPORT_MULTI_AP
						if (bl_sta->bl_reason == BL_MAP_ASSOC_CONTROL) {
								struct map_dev *map_device = NULL;
								map_device = (struct map_dev *)os_malloc(sizeof(struct map_dev));
								if (map_device) {
								os_memset(map_device, 0, sizeof(struct map_dev));
								map_device->duration = 10;
								os_memcpy(map_device->al_mac, al_mac, ETH_ALEN);
								dl_list_add(&bl_sta->map_dev_list, &map_device->map_dev_entry);
								}
						}
#endif
					}
			} else if(bl_sta->bl_reason) {
				/* Multiple reasons  */
				bl_sta->bl_reason &= ~reason;
			}
			return;
		}
	}
	/* Not in list */
	return ;
}

void client_mon_block_cli_on_bss(struct mapd_global *global,
		struct mapd_bss *bss, struct client *cli, u32 reason, u32 duration, u8 *al_mac)
{
	struct bl_client *bl_sta = NULL;

	dl_list_for_each(bl_sta, &bss->bl_sta_list, struct bl_client,
			list_entry) {
		/* Already Blacklisted */
		if (bl_sta->cli == cli) {
			bl_sta->bl_reason  |= reason;
#ifdef SUPPORT_MULTI_AP
			if (reason == BL_MAP_ASSOC_CONTROL) {
					struct map_dev *map_device = NULL;
					dl_list_for_each(map_device, &bl_sta->map_dev_list, struct map_dev,
									map_dev_entry) {
							if (!os_memcmp(map_device->al_mac, al_mac, ETH_ALEN)) {
									map_device->duration = duration;
									return;
							}
					}
					map_device = (struct map_dev *)os_malloc(sizeof(struct map_dev));
					if (!map_device) {
						mapd_printf(MSG_ERROR, "OOM: Could not allocate memory");
					}
					if(map_device) {
					os_memset(map_device, 0, sizeof(struct map_dev));
					map_device->duration = duration;
					os_memcpy(map_device->al_mac, al_mac, ETH_ALEN);
					dl_list_add(&bl_sta->map_dev_list, &map_device->map_dev_entry);
					}
			}
#endif
			return;
		}
	}
	/* Not in List */
	if (wlanif_bl_sta_for_bss(global, cli->mac_addr, bss->bssid, 1) == 0) {
		mapd_printf(MSG_DEBUG, "%d(" MACSTR
					") BLACKLISTED ON BSSID=" MACSTR,
					cli->client_id, MAC2STR(cli->mac_addr),
					MAC2STR(bss->bssid));
		bl_sta = (struct bl_client *)malloc(sizeof(struct bl_client));
		if(bl_sta == NULL) {
			mapd_printf(MSG_ERROR,"memory alloc fail");
			return;
		}
		os_memset(bl_sta, 0, sizeof(struct bl_client));
		bl_sta->cli = cli;
		bl_sta->bl_reason |= reason;
		dl_list_add(&bss->bl_sta_list, &bl_sta->list_entry);
		dl_list_init(&bl_sta->map_dev_list);
#ifdef SUPPORT_MULTI_AP
		if (reason == BL_MAP_ASSOC_CONTROL) {
				struct map_dev *map_device = NULL;
				map_device = (struct map_dev *)malloc(sizeof(struct map_dev));
				if (!map_device) {
					mapd_printf(MSG_ERROR, "OOM: Could not allocate memory");
					return;
				}
				os_memset(map_device, 0, sizeof(struct map_dev));
				map_device->duration = duration;
				os_memcpy(map_device->al_mac, al_mac, ETH_ALEN);
				dl_list_add(&bl_sta->map_dev_list, &map_device->map_dev_entry);
		}
#endif
	} else {
		mapd_printf(MSG_ERROR, "%d(" MACSTR ") BLACKLISTING ON BSSID="
					MACSTR " FAILED", cli->client_id,
					MAC2STR(cli->mac_addr),
					MAC2STR(bss->bssid));
	}
}


/* OK */
/**
 * @brief : Function to blacklist/unblacklist a stations on a given list of BSS.
 * Internally calls the wlanif function to BL/UnBL the station on a single BSS by
 * iterating through the list.
 *
 * @param global: The global device pointer
 * @param cli: client to be blacklisted/unblacklisted
 * @param num_bss: number of BSS on which to be blacklisted/unblacklisted
 * @param bss_arr : Pointer to the BSS array
 * @param blacklist: 1 for Blacklisting, 0 for unblacklisting
 */
void client_mon_bl_sta_for_bss(struct mapd_global *global, struct client *cli,
		u8 num_bss, struct mapd_bss **bss_arr,
		Boolean blacklist)
{
	uint8_t i = 0;
	struct mapd_bss *bss = NULL;

	mapd_printf(MSG_MSGDUMP, "ENTERED");
	//Iterate through the BSS list
	for (i = 0; i < num_bss; i++) {
		bss = (struct mapd_bss *)bss_arr[i];
#ifdef SUPPORT_MULTI_AP
		if (blacklist) {
			client_mon_block_cli_on_bss(global, bss, cli, BL_STEER_ALGO, 0, global->dev.al_mac);
		} else {
			client_mon_unblock_cli_on_bss(global, bss, cli, BL_STEER_ALGO, global->dev.al_mac);
		}
#else
		/* In BS there is no use of al_mac */
		if (blacklist) {
                        client_mon_block_cli_on_bss(global, bss, cli, BL_STEER_ALGO, 0, NULL);
                } else {
                        client_mon_unblock_cli_on_bss(global, bss, cli, BL_STEER_ALGO, NULL);
                }
#endif
	}
}

void client_mon_handle_traffic_stats(struct mapd_global *global, u8 *mac_addr,
				u32 tx_bytes, u32 rx_bytes, u32 tx_pkts, u32 rx_pkts, u32 tx_errs,
				u32 rx_errs)
{
	struct client *cli = NULL;
	uint32_t tx_success = 0;
	uint32_t rx_success = 0;
	uint32_t client_id;
	unsigned char *bssid;

	cli = client_db_get_client_from_sta_mac(global, mac_addr);
	if (cli == NULL) {
		mapd_printf(MSG_DEBUG, MACSTR " is not in DB",
						MAC2STR(mac_addr));
		return;
	}
	if (cli->cli_steer_state > CLI_STATE_IDLE) {
		mapd_printf(MSG_DEBUG, "Steering in progress for " MACSTR,
						MAC2STR(cli->mac_addr));
		return;
	}

	client_id = cli->client_id;
	if (client_id == (uint32_t)-1) {
		mapd_printf(MSG_ERROR, "client_id is not valid!");
		return;
	}

	bssid = client_db_get_bssid(global, client_id);
	if (is_zero_ether_addr(bssid)) {
		mapd_printf(MSG_INFO, "Zero bssid!");
		return;
	}

	if (cli->radio_idx == (uint8_t)-1) {
		mapd_printf(MSG_ERROR, "invalid radio_idx!");
		return;
	}

	mapd_printf(MSG_MSGDUMP, MACSTR ": %d %d %d %d %d %d",
					MAC2STR(cli->mac_addr), tx_bytes, rx_bytes, tx_pkts, rx_pkts,
					tx_errs, rx_errs);

	/* Update Tx and Rx Packet Count, and update RSSI ts */
	tx_success = tx_pkts - tx_errs;
	rx_success = rx_pkts - rx_errs;
	/* Update RSSI TS */
	if (((tx_success - cli->tx_success_pkt_cnt > 0) ||
		 (rx_success - cli->rx_success_pkt_cnt > 0)) &&
		(cli->ul_rssi[cli->radio_idx] != 0))
		os_get_reltime(&cli->rssi_ts[cli->radio_idx]);
	/* Update Counter */
	cli->tx_success_pkt_cnt = tx_success;
	cli->rx_success_pkt_cnt = rx_success;
}
#ifdef SUPPORT_MULTI_AP
void client_mon_handle_assoc_control(struct mapd_global *global, struct cli_assoc_control *assoc_cntrl, u8 len, u8 *al_mac)
{
	u32 client_id;
	int i = 0;
	struct client *cli = NULL;
	struct mapd_bss *bss = NULL;
	u8 already_seen = 0;
	debug("valid period: %d bssid:"MACSTR" al_mac:"MACSTR, assoc_cntrl->valid_period, MAC2STR(assoc_cntrl->bssid), MAC2STR(al_mac));
	assoc_cntrl->valid_period = be_to_host16(assoc_cntrl->valid_period);
	bss = mapd_get_bss_from_mac(global, assoc_cntrl->bssid);

	if (!bss)
		return;

	for (i = 0; i < assoc_cntrl->sta_list_count; i++) {
		client_id = client_db_track_add(global, &assoc_cntrl->sta_mac[ETH_ALEN * i],
						&already_seen);
			if (client_id == (uint32_t)-1) {
				mapd_printf(MSG_ERROR, "No more room to accomodate " MACSTR ,
								MAC2STR(&assoc_cntrl->sta_mac[ETH_ALEN * i]));
				return;
			}
			cli = &global->dev.client_db[client_id];
			cli->in_db = IN_DB;
			if (already_seen != 1) {
				mapd_printf(MSG_DEBUG, "New Client discovered"
								MACSTR, MAC2STR(&assoc_cntrl->sta_mac[ETH_ALEN * i]));
				cli->dirty = 1;
			}
			if (!assoc_cntrl->assoc_control) //block
				client_mon_block_cli_on_bss(global, bss, cli,
								BL_MAP_ASSOC_CONTROL, assoc_cntrl->valid_period,
								al_mac);
			else
				client_mon_unblock_cli_on_bss(global, bss, cli,
								BL_MAP_ASSOC_CONTROL, al_mac);
	}
}
#endif
