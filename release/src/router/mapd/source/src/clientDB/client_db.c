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

                Module Name: clientDB
                client_db.c

                Abstract:
				Client DB get and set handlers.

                Revision History:
                Who        			 When          What
                ---------------   	 ----------    -----------------------------------------
                Neelansh Mittal   	 2018/04/20    Add header files
*/

#include "includes.h"
#include "common.h"
#include "steer_fsm.h"
#include "list.h"
#include "client_db.h"
#include "mapd_i.h"
#ifdef SUPPORT_MULTI_AP
#include "./../1905_local_lib/data_def.h"
#include "topologySrv.h"
#endif
#include "client_mon.h"
#include "steer_action.h"
#include <assert.h>
#include "db.h"

/* Deinit Client: Called from during oldest client eviction
 * Remove this client from all lists*/
void client_db_deinit_client(struct mapd_global *global, struct client *cli)
{
#ifdef MAP_R2
	int i;
#endif
    if(cli->client_id == (uint32_t)-1) {
        err("No client linked to this client_db entry--return");
        return;
    }
#ifdef MAP_R2
	for (i = 0;i < MAX_TUNNEL_TYPE; i++) {
		if (cli->tunnel_info[i].tunneled_payload != NULL) {
			os_free(cli->tunnel_info[i].tunneled_payload);
			cli->tunnel_info[i].tunneled_payload = NULL;
		}
	}
#endif
    /* Remove from Sta seen list(always present, if client_id != -1) */
    dl_list_del(&cli->sta_seen_entry);
    /* Remove from Assoc list(if present) */
    client_db_remove_from_assoc_list(global, cli->client_id);
	/* Remove all BLs */
	client_mon_force_unblock_cli_on_all_bss(global, cli);
	/* Cancel all timers */
	steer_action_clear_all_timers(global, cli);
}

/* Init client: Called from MAPD init and during oldest client eviction */
void client_db_init_client(struct client *cli)
{
	os_memset(cli, 0, sizeof(struct client));
	/* Initialize non-zero attributes */
	cli->client_id = -1;
	cli->radio_idx = -1;
	cli->auth_deny_max = 5; //CHANGE ME
}

uint32_t client_db_get_cid_from_mac(struct mapd_global *global, u8 *mac_addr)
{
	uint32_t i = 0;

	for(i = 0; i < MAX_STA_SEEN; i++) {
		if(!os_memcmp(mac_addr, &global->dev.client_db[i].mac_addr[0], ETH_ALEN))
			return i;
    }
	return (uint32_t)-1;
}

uint32_t client_db_add_client_to_db(struct mapd_global *global, u8 *mac_addr)
{
	struct client *cli = NULL, *evict_cli = NULL;
    uint32_t i = 0;

	for(i = 0; i < MAX_STA_SEEN; i++) {
		cli = &global->dev.client_db[i];
		if (cli->client_id == (uint32_t)-1) {
			//printf("Found an un-unused client db entry at%d\n", i);
			os_memcpy(&cli->mac_addr[0], mac_addr, ETH_ALEN);
			cli->client_id = i;
			/* Add to last seen list */
			dl_list_add_tail(&global->dev.sta_seen_list, &cli->sta_seen_entry);
			return i;
		}
	}
	/* Evict the oldest unassociated entry */
    dl_list_for_each(cli, &global->dev.sta_seen_list, struct client, sta_seen_entry) {
        if(cli != NULL && !is_zero_ether_addr(cli->bssid))
            continue;
		if (cli->cli_steer_state >= CLI_STATE_STEER_DECISION) //Steering in Progress for this client
			continue;
		if (is_cli_bl_map_assoc_control(global, cli))
			continue;
		/* This client can be evicted. Try to evict a not in_db client. If all are in db,
		 * then evict the oldest one */
		if (evict_cli == NULL)
			evict_cli = cli;
		if (cli->in_db)
			continue;
		evict_cli = cli;
		break;
	}

	if (evict_cli != NULL) {
		uint32_t evicted_client_id = evict_cli->client_id;
		u8 in_db = 0;

		if (evict_cli->in_db)
			in_db = 1;
		mapd_printf(MSG_INFO, "Evicting %d(in_db=%d) " MACSTR, evicted_client_id, in_db, MAC2STR(evict_cli->mac_addr));
		client_db_deinit_client(global, evict_cli);
		/* Init client */
		client_db_init_client(evict_cli);
		evict_cli->client_id = evicted_client_id; //Init makes client_id = (uint8_t) - 1;
		os_memcpy(&evict_cli->mac_addr[0], mac_addr, ETH_ALEN);
		if (in_db)
			evict_cli->in_db = IN_DB_DEL; // Mark for Delete from persistent DB
		/* Add to sta_seen list */
		dl_list_add_tail(&global->dev.sta_seen_list, &evict_cli->sta_seen_entry);
		//XXX TODO: Call Update
		return evict_cli->client_id;
	}

	mapd_printf(MSG_INFO, "No room; Full and associated");
    return (uint32_t)-1; //FULL and all associated
}

void client_db_init(struct client *cli_db)
{
		uint16_t i = 0;
		for (i = 0; i < MAX_STA_SEEN; i++) {
			client_db_init_client(&cli_db[i]);
		}
}

uint32_t client_db_track_add(struct mapd_global *global, u8 *mac_addr,
				u8 *already_seen)
{
	uint32_t client_id = 0;
    struct client *cli = NULL;
	u8 old_seen = *already_seen;

    dl_list_for_each(cli, &global->dev.sta_seen_list, struct client, sta_seen_entry)
    {
        if (!os_memcmp(&cli->mac_addr[0], mac_addr, ETH_ALEN)) {
            debug("Already seen client\n");
            *already_seen = 1;
            break;
        }
    }
    if(*already_seen == 1){
        /* Move the most recent entry to the end of the sta_seen list */
        dl_list_del(&cli->sta_seen_entry);
		if (old_seen != 2)
	        dl_list_add_tail(&global->dev.sta_seen_list, &cli->sta_seen_entry);
		else if (old_seen == 2)
			cli->client_id = -1;
        return cli->client_id;
    }
    /* Seeing this client for the first time */
	if (old_seen != 2) {
    	client_id = client_db_add_client_to_db(global, mac_addr);
	}
    return client_id;
}

struct client *client_db_get_client_from_sta_mac(struct mapd_global *global, unsigned char *sta_mac)
{
	uint32_t  idx=0;
	for(idx = 0; idx < MAX_STA_SEEN; ++idx)
	{
		if (!os_memcmp(global->dev.client_db[idx].mac_addr, sta_mac, ETH_ALEN))
			return &global->dev.client_db[idx];
	}

	return NULL;
}

Boolean client_db_is_sta_bl_on_bss(struct mapd_global *global,
				struct client *cli, struct mapd_bss *bss)
{
	struct bl_client *bl_sta = NULL;

	dl_list_for_each(bl_sta, &bss->bl_sta_list, struct bl_client,
			list_entry)
	{
		if (bl_sta->cli == cli) {
			return TRUE;
		}
	}
	return FALSE;
}

static uint8_t get_max_nss_by_htcap_ie_mcs(const u8 *cap_mcs)
{
	u8 index, nss = 0;
	u8 bitmask;

	index = 32;
	do {
		index--;
		nss = index / 8;
		bitmask = (1 << (index - (nss * 8)));

		if (cap_mcs[nss] & bitmask)
			break;
	} while (index);

	return nss + 1;
}

void client_db_set_radio_idx(struct mapd_global *global, uint8_t client_id, u8 radio_idx)
{
	struct client *cli = &global->dev.client_db[client_id];
	cli->radio_idx = radio_idx;
}

void client_db_set_known_channels(struct mapd_global *global, uint8_t client_id,
        uint8_t channel)
{
	struct client *cli = &global->dev.client_db[client_id];
	uint8_t chan_idx = chan_to_idx(channel);
	uint8_t arr_idx = 0;

	if (chan_idx <= -1 || chan_idx >= MAX_NUM_CHANNELS) {
		return;
		mapd_ASSERT(0);
	}

	arr_idx = chan_idx / 8; //15 ;arr_idx = 1

	if(cli->known_channels[arr_idx] & BIT(chan_idx % 8)) {
		return;
	} else {
		cli->known_channels[arr_idx] |= BIT(chan_idx % 8);
		mapd_printf(MSG_DEBUG, MACSTR " new channel=%d",
				MAC2STR(cli->mac_addr), channel);
		/* Mark for Update to Non-volatile */
		cli->dirty = 1;
	}
}
void parse_and_set_supp_channels(const u8 *start, size_t len)
{
    /* Parse and populate known_channels */
}

#define HT_CAP_IE_SIZE 16
#define VHT_CAP_IE_SIZE 12
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK 0x0C
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ 0x04
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ 0x08

#define HE_CAP_IE_SIZE 22
#define IEEE80211_HE_CAP_SUPP_CHAN_WIDTH_MASK 0xFE
#define IEEE_80211_HE_CAP_SUPP_CHAN_WIDTH_2G_20_40MHZ 0x1
#define IEEE_80211_HE_CAP_SUPP_CHAN_WIDTH_5G_40_80MHZ 0x2
#define IEEE_80211_HE_CAP_SUPP_CHAN_WIDTH_5G_160MHZ 0x4
#define IEEE_80211_HE_CAP_SUPP_CHAN_WIDTH_5G_160_80PLUS80MHZ 0x8
#define IEEE_80211_HE_CAP_SUPP_CHAN_WIDTH_5G 0xE

#define DOT11AX_PHY_CAP_CH_WIDTH_SET_SHIFT 1
#define DOT11AX_PHY_CAP_CH_WIDTH_SET_MASK (0x7F << 1)
#define GET_DOT11AX_CH_WIDTH(phy_cap)\
	(((phy_cap) & DOT11AX_PHY_CAP_CH_WIDTH_SET_MASK) >> DOT11AX_PHY_CAP_CH_WIDTH_SET_SHIFT)

/* max_mcs_nss */
#define DOT11AX_MAX_STREAM 8
#define DOT11AX_MCS_NSS_MASK 0x3

u8 he_support_max_mcs(u16 mcs_map, u8 nss_m1)
{
	return ((mcs_map >> (nss_m1 << 1)) & DOT11AX_MCS_NSS_MASK);
}


u8 peer_max_bw_cap(u8 ch_width_set)
{
	u8 ch_width = BW_20;

	if (ch_width_set & SUPP_40M_CW_IN_24G_BAND)
		ch_width = BW_40;
	if (ch_width_set & SUPP_40M_80M_CW_IN_5G_BAND) {
		ch_width = BW_80;
		if (ch_width_set & SUPP_160M_CW_IN_5G_BAND)
			ch_width = BW_160;
		if (ch_width_set & SUPP_160M_8080M_CW_IN_5G_BAND)
			ch_width = BW_8080;
	}
	return ch_width;
}



void client_db_update_cli_ht_vht_cap(struct mapd_global *global, uint32_t client_id, 
const u8 *start, size_t len, uint8_t band_idx)
{
	size_t left = len;
	const u8 *pos = start;
	const u8 *ext_he_ptr;
	int8_t is_ht = 0;
	int8_t is_vht = 0;
	int8_t is_he = 0;
	struct client *cli = NULL;
	const u8 *supported_mcs_set = NULL;
	int ht_bw = BW_20, vht_bw = BW_20;
	struct he_cap_ie *he_cap = NULL;
	struct he_txrx_mcs_nss *mcs_nss_160 = NULL;
	struct he_txrx_mcs_nss *mcs_nss_8080 = NULL;
	int new_nss = 0, new_max_bw = BW_20, new_phy_mode = LEGACY_MODE;
	int he_bw = BW_20;
	int he_nss = 0;
	u8 idx = 0,nss = 0;
	struct map_he_nss *nss_he = os_zalloc(sizeof(struct map_he_nss));
	u8 ext_id = 0;
	cli = &global->dev.client_db[client_id];

	while (left >= 2) {
		u8 id, elen;

		id = *pos++;
		elen = *pos++;
		left -= 2;

		if (elen > left) {
			mapd_printf(MSG_MSGDUMP, "IEEE 802.11 element "
					"parse failed (id=%d elen=%d "
					"left=%lu)",
					id, elen, (unsigned long) left);
			break;
		}
		mapd_printf(MSG_MSGDUMP, "left: %d EID=%d ELEN=%d", (int)left, id, elen);

		switch (id) {
			case WLAN_EID_HT_CAP:
				if (elen < HT_CAP_IE_SIZE)
					break;
				supported_mcs_set = pos + 3;
				new_nss = get_max_nss_by_htcap_ie_mcs(&supported_mcs_set[0]);
				ht_bw = ((*pos) & ((unsigned char)BIT(1))) ? BW_40 : BW_20;
				is_ht = 1;
				mapd_printf(MSG_DEBUG, "WLAN_EID_HT_CAP");
				break;
			case WLAN_EID_VHT_CAP:
				if (elen < VHT_CAP_IE_SIZE)
					break;
				is_vht = 1;
				switch ((*pos) & IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK) {
						case IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ:
								vht_bw = BW_160;
								break;
						case IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ:
								vht_bw = BW_8080;
								break;
						default:
								vht_bw = BW_80;
								break;
				}
				mapd_printf(MSG_MSGDUMP, "WLAN_EID_VHT_CAP");
				break;
			case IE_WLAN_EXTENSION:
				ext_id = *pos;

				switch (ext_id) {
					case EID_EXT_HE_CAPS: 
				if (elen < HE_CAP_IE_SIZE) {
					mapd_printf(MSG_ERROR, "IE less than HE_CAP_IE_SIZE ");
					break;
				}
						 he_cap = (struct he_cap_ie *)((u8*)pos + 2);
				//Extract BW.
						if(band_idx == BAND_2G_IDX)
							he_bw = peer_max_bw_cap(GET_DOT11AX_CH_WIDTH(he_cap->phy_cap.phy_capinfo_1) & IEEE_80211_HE_CAP_SUPP_CHAN_WIDTH_2G_20_40MHZ);
						else
							he_bw = peer_max_bw_cap(GET_DOT11AX_CH_WIDTH(he_cap->phy_cap.phy_capinfo_1) & IEEE_80211_HE_CAP_SUPP_CHAN_WIDTH_5G);
				//Extract NSS
				for(idx = 0; idx < DOT11AX_MAX_STREAM; idx ++) {
							nss = he_support_max_mcs(he_cap->txrx_mcs_nss.max_rx_mcs_nss,idx);
							if (nss == 0x3)
						continue;
							nss_he->nss_80= idx+1;
							he_nss = nss_he->nss_80;
				}
						if (he_bw == BW_160) {
							ext_he_ptr = pos + sizeof(struct he_cap_ie);
							mcs_nss_160 = (struct he_txrx_mcs_nss *)ext_he_ptr;
							for(idx = 0; idx < DOT11AX_MAX_STREAM; idx ++) {
								nss = he_support_max_mcs(mcs_nss_160->max_rx_mcs_nss,idx);
								if (nss == 0x3)
									continue;
								nss_he->nss_160 = idx+1;
							}
							he_nss = nss_he->nss_160;
						}
						if (he_bw == BW_8080) {
							ext_he_ptr = pos + sizeof(struct he_cap_ie) + sizeof(struct he_txrx_mcs_nss);
							mcs_nss_8080 = (struct he_txrx_mcs_nss *)ext_he_ptr;
							for(idx = 0; idx < DOT11AX_MAX_STREAM; idx ++) {
								nss = he_support_max_mcs(mcs_nss_8080->max_rx_mcs_nss,idx);
								if (nss == 0x3)
									continue;
								nss_he->nss_8080 = idx+1;
							}
							he_nss = nss_he->nss_8080;
						}
				is_he = 1;
				mapd_printf(MSG_MSGDUMP, "WLAN_EID_HE_CAP");
						break;
						default:
							break;
				}
				break;
			default:
				mapd_printf(MSG_MSGDUMP, "IEEE 802.11 element parse "
						"ignored unknown element (id=%d elen=%d)",
						id, elen);
				break;
		}
		left -= elen;
		pos += elen;
	}

	if(is_he == 1 && is_vht == 1 && is_ht == 1) {
			cli->ht_vht_he_cap = HE_VHT_HT_CAPABLE;
			new_phy_mode = HE_MODE;
			new_max_bw = he_bw;
			new_nss = he_nss;
	} else if (is_vht == 0 && is_ht == 1 && is_he == 1) {
		cli->ht_vht_he_cap = HE_HT_CAPABLE;
		new_max_bw = he_bw;
		new_phy_mode = HE_MODE;
		new_nss = he_nss;
	} else if (is_vht == 0 && is_ht == 1) {
		cli->ht_vht_he_cap = NON_VHT_CAPABLE;
		new_max_bw = ht_bw;
		new_phy_mode = HT_MODE;
	} else if(is_vht == 0 && is_ht == 0) {
		cli->ht_vht_he_cap = LEGACY_CLIENT;
		new_max_bw = BW_20; 
		new_phy_mode = LEGACY_MODE;
	}
	else {
		new_phy_mode = VHT_MODE;
		if (band_idx == BAND_5G_IDX) //VHT and 5G
			new_max_bw = vht_bw;
		else if(is_ht) //VHT, HT and 2G
			new_max_bw = ht_bw;
		else 
			new_max_bw = BW_20;
	}

	if (left) {
		mapd_printf(MSG_MSGDUMP, "%s: Parse failed", __func__);
	}

	client_db_set_phy_capab(global, client_id, 1, band_idx, new_max_bw, new_nss,
					new_phy_mode);
	if(nss_he && is_he == 1) {
		client_db_set_he_phy_capab(global, client_id, nss_he);
	}
	os_free(nss_he);
	return;
}
#if 0
void client_db_update_from_ies(struct mapd_global *global, uint8_t client_id,
const u8 *start, size_t len, uint8_t channel)
{
	size_t left = len;
	const u8 *pos = start;
	const u8 *supported_mcs_set = NULL;
	const u8 *ext_capab_oct3 = NULL;
	int8_t ht_bw = -1;
	int8_t vht_bw = -1;
	uint8_t i = 0;
	struct client *cli = NULL;
	enum max_bw orig_max_bw ;
	enum phy_mode orig_phy_mode, new_phy_mode = 0;
	u8 band_idx = 0;



	cli = &global->dev.client_db[client_id];

	if (channel > 14) {
		band_idx = BAND_5G_IDX;
	} else {
		band_idx = BAND_2G_IDX;
	}
	/* Store the orig capabs */
	orig_max_bw = cli->phy_capab.max_bw[band_idx];
	orig_phy_mode = cli->phy_capab.phy_mode;


	mapd_hexdump(MSG_DEBUG, "preq/assoc_ies", start, len);
	while (left >= 2) {
		u8 id, elen;

		id = *pos++;
		elen = *pos++;
		left -= 2;

		if (elen > left) {
			mapd_printf(MSG_DEBUG, "IEEE 802.11 element "
					"parse failed (id=%d elen=%d "
					"left=%lu)",
					id, elen, (unsigned long) left);
			break;
		}
		mapd_printf(MSG_DEBUG, "left: %d EID=%d ELEN=%d", (int)left, id, elen);

		switch (id) {
			case WLAN_EID_SUPPORTED_CHANNELS:
			    parse_and_set_supp_channels(pos, elen);
			    mapd_printf(MSG_DEBUG, "WLAN_EID_SUPPORTED_CHANNELS IE");
				break;
			case WLAN_EID_SUPP_RATES:
				for (i = 0; i < elen; i++){
					if ((pos[i] & 0x7f) == 12) {
						if (new_phy_mode < MODE_HTMIX)
							new_phy_mode = MODE_OFDM;
						break;
					}
				}
				mapd_printf(MSG_DEBUG, "WLAN_EID_SUPP_RATES");
				break;
			case WLAN_EID_EXT_SUPP_RATES:
				for (i = 0; i < elen; i++){
					if ((pos[i] & 0x7f) == 12) {
						if (new_phy_mode < MODE_HTMIX)
							new_phy_mode = MODE_OFDM;
						break;
					}
				}
				mapd_printf(MSG_DEBUG, "WLAN_EID_EXT_SUPP_RATES");
				break;
			case WLAN_EID_HT_CAP:
				if (elen < HT_CAP_IE_SIZE)
					break;
				supported_mcs_set = pos + 3;
				cli->phy_capab.num_sp_streams =
					get_max_nss_by_htcap_ie_mcs(&supported_mcs_set[0]);
				ht_bw = ((*pos) & ((unsigned char)BIT(1))) ? BW_40 : BW_20;
				if (new_phy_mode < MODE_VHT) {
					new_phy_mode = ((*pos) & ((unsigned char)BIT(4))) ?
						MODE_HTGREENFIELD : MODE_HTMIX;
				}
				mapd_printf(MSG_DEBUG, "WLAN_EID_HT_CAP");
				break;
			case WLAN_EID_VHT_CAP:
				if (elen < VHT_CAP_IE_SIZE)
					break;
				new_phy_mode = MODE_VHT;
				switch ((*pos) & IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK) {
					case IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ:
						vht_bw = BW_160;
						break;
					case IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ:
						vht_bw = BW_8080;
						break;
					default:
						vht_bw = BW_80;
						break;
				}
				mapd_printf(MSG_DEBUG, "WLAN_EID_VHT_CAP");
				break;
			case WLAN_EID_MULTI_BAND:
				cli->capab |= CAP_MBO_SUPPORTED;
				mapd_printf(MSG_DEBUG, "WLAN_EID_MULTI_BAND");
				break;
			case WLAN_EID_RRM_ENABLED_CAPABILITIES:
				cli->capab |= CAP_11K_SUPPORTED;
				mapd_printf(MSG_DEBUG, "WLAN_EID_RRM_ENABLED_CAPABILITIES");
				break;
			case WLAN_EID_MDE:
				cli->capab |= CAP_11R_SUPPORTED;
				mapd_printf(MSG_DEBUG, "WLAN_EID_MDE");
				break;
			case WLAN_EID_EXT_CAPAB:
				if (elen > 3) {
				    ext_capab_oct3 = pos + 2;
					if((*ext_capab_oct3) & BIT(3))
						cli->capab |= CAP_11V_SUPPORTED;
				}
				mapd_printf(MSG_DEBUG, "WLAN_EID_EXT_CAPAB");
				break;
			default:
				mapd_printf(MSG_MSGDUMP, "IEEE 802.11 element parse "
						"ignored unknown element (id=%d elen=%d)",
						id, elen);
				break;
		}
		left -= elen;
		pos += elen;
	}

	if (vht_bw != -1) {
		cli->phy_capab.max_bw[band_idx] = vht_bw;
		new_phy_mode = MODE_VHT;
	}
	else if (ht_bw != -1)
		cli->phy_capab.max_bw[band_idx] = ht_bw;
	else {// NON-HT 
		cli->phy_capab.max_bw[band_idx] = BW_20;
	}

	if (left) {
		mapd_printf(MSG_DEBUG, "%s: Parse failed", __func__);
	}
	cli->phy_capab.phy_mode = new_phy_mode;
	mapd_printf(MSG_DEBUG, "Dump client info retrived from IEs");
	mapd_printf(MSG_DEBUG, "Channel =%d Capab=%d NSS=%d max_bw=%d PhyMode=%d", channel, cli->capab,
			cli->phy_capab.num_sp_streams, cli->phy_capab.max_bw[band_idx], cli->phy_capab.phy_mode);
	/* Update non-volatile db*/
	if ((orig_max_bw != cli->phy_capab.max_bw[band_idx]) ||
		(orig_phy_mode != cli->phy_capab.phy_mode))
		cli->dirty = 1;

	return;
}
#endif

void client_db_remove_from_assoc_list(struct mapd_global *global , uint32_t client_id)
{
   	struct client *cli = &global->dev.client_db[client_id];
	if(cli == NULL)
		return;
	if ((cli->assoc_sta_entry.next != NULL) || (cli->assoc_sta_entry.prev != NULL))
			dl_list_del(&cli->assoc_sta_entry);
}

void client_db_set_dl_phy_rate(struct mapd_global *global, uint32_t client_id, 
			uint32_t dl_phy_rate)
{
	struct client *cli = &global->dev.client_db[client_id];
	cli->dl_phy_rate = dl_phy_rate;
}

uint32_t client_db_get_dl_phy_rate(struct mapd_global *global, uint32_t client_id)
{
	struct client *cli = &global->dev.client_db[client_id];
	return cli->dl_phy_rate;
}

void client_db_set_ul_rssi(struct mapd_global *global, uint32_t client_id,
		uint8_t ul_rssi, uint8_t radio_idx, Boolean is_preq)
{
	struct client *cli = &global->dev.client_db[client_id];
	if (ul_rssi) {
		cli->ul_rssi[radio_idx] = ul_rssi;
		if (is_preq)
			os_get_reltime(&cli->rssi_ts[radio_idx]);
	}
}

int8_t client_db_get_ul_rssi(struct mapd_global *global, uint32_t client_id,
		uint8_t radio_idx)
{
	struct client *cli = &global->dev.client_db[client_id];
	return cli->ul_rssi[radio_idx];
}
void client_db_set_bssid(struct mapd_global *global, uint8_t client_id, u8 *bssid)
{
	struct client *cli = &global->dev.client_db[client_id];
	os_memcpy(cli->bssid, bssid, ETH_ALEN);
}

u8 * client_db_get_bssid(struct mapd_global *global, uint8_t client_id)
{
	struct client *cli = &global->dev.client_db[client_id];
	return &cli->bssid[0];
}

void client_db_set_curr_channel(struct mapd_global *global, uint8_t client_id, u8 channel)
{
	struct client *cli = &global->dev.client_db[client_id];
	cli->current_chan = channel;
}

u8 client_db_get_curr_channel(struct mapd_global *global, uint8_t client_id)
{
	struct client *cli = &global->dev.client_db[client_id];
	return cli->current_chan;
}

struct dl_list * client_db_get_assoc_list_entry(struct mapd_global *global,
		uint8_t client_id)
{
	struct client *cli = &global->dev.client_db[client_id];
	return &cli->assoc_sta_entry;
}

void client_db_clear_post_assoc_params(struct mapd_global *global, uint8_t client_id)
{
	struct client *cli = NULL;
	mapd_printf(MSG_INFO, "Clearing post assoc params for the client %d\n", client_id);
	cli = &global->dev.client_db[client_id];

	os_memset(cli->bssid, 0, ETH_ALEN);
	cli->current_chan = 0;
	cli->dl_rate = cli->ul_rate = 0;
	cli->tx_count = cli->rx_count = 0;
	cli->activity_state = 0;
	cli->curr_air_time = 0;
	cli->auth_deny_count = 0;
	cli->radio_idx = -1;
	os_memset((u8 *)&cli->ul_rssi[0], 0, MAX_NUM_OF_RADIO);
	os_memset((u8 *)&cli->rssi_ts[0], 0, MAX_NUM_OF_RADIO * sizeof(struct os_reltime));
	cli->dl_phy_rate = 0;
	cli->force_airmon = 0;
	cli->str_mthds_failed_in_dec = 0;
#ifdef MAP_R2
	os_memset((u8 *)&cli->np_channels[0], 0, MAX_NOT_PREFER_CH_NUM);
	cli->np_pref = 255;
	cli->np_reason = 0;
#endif
}

struct client * client_db_get_client_from_client_id(struct mapd_global *global,
			uint8_t client_id)
{
	struct client *cli = NULL;
	if((client_id >= 0)||(client_id < MAX_STA_SEEN))
	{
		cli = &global->dev.client_db[client_id];
		return cli;
	}
	else
		return NULL;
}

STEERING_STATE client_db_get_cli_steer_state(struct mapd_global *global, uint8_t client_id)
{
	struct client *cli = NULL;

	cli = &global->dev.client_db[client_id];
	return cli->cli_steer_state;
}

void client_db_set_cli_steer_state(struct mapd_global *global, int client_id, STEERING_STATE next_state)
{
	struct client *cli = NULL;

	cli = &global->dev.client_db[client_id];
	cli->cli_steer_state = next_state;
}

void client_db_set_capab(struct mapd_global *global, uint32_t client_id,
                         u8 bBtm, u8 bRrm, u8 bMbo)
{
	struct client *cli = NULL;

	cli = &global->dev.client_db[client_id];
	if (bBtm)
		cli->capab |= CAP_11V_SUPPORTED;
	else
		cli->capab &= ~CAP_11V_SUPPORTED;

	if (bRrm)
		cli->capab |= CAP_11K_SUPPORTED;
	else
		cli->capab &= ~CAP_11K_SUPPORTED;

	if (bMbo)
		cli->capab |= CAP_MBO_SUPPORTED;
	else
		cli->capab &= ~CAP_MBO_SUPPORTED;
}

void client_db_set_phy_capab(struct mapd_global *global, uint32_t  client_id, uint8_t source,
                         uint8_t band_idx, enum max_bw bw, uint8_t nss, coarse_phy_mode phy_mode)
{
	struct client *cli = NULL;

	cli = &global->dev.client_db[client_id];
	cli->phy_cap_known[band_idx] = source;
	if (cli->phy_capab.max_bw[band_idx] < bw) {
		cli->phy_capab.max_bw[band_idx] = bw;
		cli->dirty = 1;
	}
	if (cli->phy_capab.num_sp_streams < nss) {
		cli->phy_capab.num_sp_streams = nss;
		cli->dirty = 1;
	}
	if (cli->phy_capab.phy_mode[band_idx] < phy_mode) {
		cli->phy_capab.phy_mode[band_idx] = phy_mode;
		cli->dirty = 1;
	}
}
void client_db_set_he_phy_capab(struct mapd_global *global, uint32_t  client_id, struct map_he_nss *nss_he) {	
	struct client *cli = NULL;	
	cli = &global->dev.client_db[client_id];
	if(nss_he == NULL)
		return;
	if (cli->phy_capab.num_he_spstr.bw80_streams < nss_he->nss_80) {
		cli->phy_capab.num_he_spstr.bw80_streams = nss_he->nss_80;
		cli->dirty = 1;
	}
	if (cli->phy_capab.num_he_spstr.bw160_streams < nss_he->nss_160) {
		cli->phy_capab.num_he_spstr.bw160_streams = nss_he->nss_160;
		cli->dirty = 1;
	}
	if (cli->phy_capab.num_he_spstr.bw8080_streams < nss_he->nss_8080) {
		cli->phy_capab.num_he_spstr.bw8080_streams = nss_he->nss_8080;
		cli->dirty = 1;
	}
}
