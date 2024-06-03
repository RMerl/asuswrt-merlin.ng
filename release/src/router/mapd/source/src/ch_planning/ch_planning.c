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
 *  Channel planning *
 *  Abstract:
 *  Channel Planning
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Hasan 2018/05/02    First implementation channel planning * */

#include "includes.h"
#ifdef __linux__
#include <fcntl.h>
#endif				/* __linux__ */

#include "common.h"
#include <sys/un.h>
#include "interface.h"
#include "./../1905_local_lib/data_def.h"
#include "1905_map_interface.h"
#include "client_db.h"
#include "mapd_i.h"
#include "topologySrv.h"
#include "eloop.h"
#include "tlv_parsor.h"
#include "1905_if.h"
#include "wapp_if.h"
#include "mapd_debug.h"
#include "chan_mon.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "apSelection.h"
#include "ch_planning.h"
extern u8 ZERO_MAC_ADDR[ETH_ALEN];

#ifdef MAP_160BW
u8 grp_bw_160[2][8] = {
	{36,40,44,48,52,56,60,64},
	{100,104,108,112,116,120,124,128}
};
#endif
u8 grp_bw_80[MAX_BW_80_BLOCK][4] = {
	{36,40,44,48},
	{52,56,60,64},
	{100,104,108,112},
	{116,120,124,128},
	{132,136,140,144},
	{149,153,157,161}
};
u8 grp_bw_40[MAX_BW_40_BLOCK][2] = {
	{36,40},
	{44,48},
	{52,56},
	{60,64},
	{100,104},
	{108,112},
	{116,120},
	{124,128},
	{132,136},
	{140,144},
	{149,153},
	{157,161}
};

struct vht_ch_layout {
       u8 ch_low_bnd;
       u8 ch_up_bnd;
       u8 cent_freq_idx;
};

static struct vht_ch_layout vht_ch_80M[] = {
       {36, 48, 42},
       {52, 64, 58},
       {100, 112, 106},
       {116, 128, 122},
       {132, 144, 138},
       {149, 161, 155},
       {0, 0, 0},
};

unsigned char get_primary_channel(unsigned char channel)
{
       int i, ch_size;
       struct vht_ch_layout *vht;

       ch_size = sizeof(vht_ch_80M) / sizeof(struct vht_ch_layout);
       for (i = 0; i < ch_size; i++) {
               vht = &vht_ch_80M[i];
               if (vht->cent_freq_idx == channel) {
                       return vht->ch_low_bnd;
               }
       }
       return channel;
}

unsigned int is_valid_primary_ch_80M_160M(unsigned char ch, unsigned char center_ch, unsigned char op)
{
	int offset = 0;

	offset = ch - center_ch;
	if (op == 128) {
		if ((abs(offset) == 6) || (abs(offset) == 2))
			return 1;
		else
			return 0;
	}
#ifdef MAP_160BW
	else if (op == 129) {
		if ((abs(offset) == 14) || (abs(offset) == 10) ||
			(abs(offset) == 6) || (abs(offset) == 2)) {
			return 1;
		} else {
			return 0;
		}
	}
#endif
	return 0;
}

/**
* @brief Fn to get number of radios on an operating channel
* @param ch_distribution chanel_planning global struct
* @param channel taregt channel number
*/

unsigned char ch_planning_get_num_radio_on_operating_channel(
	struct ch_distribution_cb *ch_distribution,
	unsigned char channel)
{
	struct operating_ch_cb *operating_ch= NULL;
	SLIST_FOREACH(operating_ch, &(ch_distribution->first_operating_ch), next_operating_ch) {
		if (operating_ch->ch_num == channel)
		{
			return operating_ch->radio_count;
		}
	}
	return 0;
}

/**
* @brief Fn to get number of radios on an prefered channel
* @param ch_distribution chanel_planning global struct
* @param channel taregt channel number
*/
unsigned char ch_planning_get_num_radio_on_prefered_channel(
	struct own_1905_device *ctx,
	unsigned char channel)
{
	//err("%s\n", __FUNCTION__);
	struct ch_distribution_cb *ch_distribution = NULL;
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct prefered_ch_cb *prefered_ch= NULL;

	if (channel > 14)
	{
		//err("ch_ditribution_5g\n");
		ch_distribution = &ch_planning->ch_ditribution_5g;
	} else {
		//err("ch_ditribution_2g\n");
		ch_distribution = &ch_planning->ch_ditribution_2g;
	}
	SLIST_FOREACH(prefered_ch, &(ch_distribution->first_prefered_ch), next_prefered_ch) {
		if (prefered_ch->ch_num == channel)
		{
			return prefered_ch->radio_count;
		}
	}
	return 0;
}
/**
* @brief Fn to insert a radio in operating channel list
* @param ch_distribution chanel_planning global struct
* @param channel taregt channel number
*/

void ch_planning_insert_into_ch_operating(
	struct own_1905_device *ctx,
	struct operating_ch_cb *operating_ch,
	struct ch_distribution_cb *ch_distribution)
{
	struct operating_ch_cb *operating_ch_temp = NULL;
	struct operating_ch_cb *previous_operating_ch= NULL;

//	err("%s\n", __FUNCTION__);
	SLIST_FOREACH(operating_ch_temp, &(ch_distribution->first_operating_ch), next_operating_ch) {
		if (operating_ch_temp->radio_count > operating_ch->radio_count)
		{
			debug("Radio count on Channel %d is higher than on Channel %d\n",
				operating_ch_temp->ch_num, operating_ch->ch_num);
		} else {
			if (operating_ch_temp->radio_count <
				operating_ch->radio_count)
			{
				if (previous_operating_ch != NULL) {
					SLIST_INSERT_AFTER(previous_operating_ch,
						operating_ch,
						next_operating_ch);
					debug("Add after Channel number = %d\n",
					previous_operating_ch->ch_num);
				} else {
					debug("Insert into head\n");
					SLIST_INSERT_HEAD(
						&(ch_distribution->first_operating_ch),
						operating_ch,
						next_operating_ch);
				}
				break;
			}
		}
		previous_operating_ch = operating_ch_temp;
	}
	if (operating_ch_temp == NULL)
	{
		debug("Channel %d not inserted yet, insert in the tail\n",
			operating_ch->ch_num);
		if (previous_operating_ch != NULL) {
			debug("Insert after %d\n", previous_operating_ch->ch_num);
			SLIST_INSERT_AFTER(previous_operating_ch,
				operating_ch,
				next_operating_ch);
		} else {
			debug("Insert in the head\n");
			SLIST_INSERT_HEAD(
				&(ch_distribution->first_operating_ch),
				operating_ch,
				next_operating_ch);
		}
	}

}
#ifdef MAP_R2
void ch_planning_allow_ch_sync(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{
	struct radio_info_db *radio = NULL;
	struct mapd_global *global = ctx->back_ptr;
	//struct _1905_map_device *_1905_device = topo_srv_get_1905_device(ctx, NULL);
	/*if any radio's bootup run is yet incomplete then wait before allowing sync*/
	uint8_t op_channel_idx;
	for (op_channel_idx = 0; op_channel_idx < MAX_NUM_OF_RADIO; op_channel_idx++) {
		struct mapd_radio_info *radio_info = &global->dev.dev_radio_info[op_channel_idx];
		if (radio_info->radio_idx == (uint8_t)-1)
			continue;
		if(radio_info->bootup_run != 1)
			return;
	}
	err("global state %d", ctx->ch_planning_R2.ch_plan_state);
	if(ctx->ch_planning_R2.force_trigger == 1 ||
		ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_CH_CHANGE_TRIGGERED) {
		err("return as no need to sync all radios, only the ones on which ch planning has been triggered");
		return;
	}

	/*Otherwise allow sync , it indicates that a new device has entered network*/
	err("for new dev"MACSTR"", MAC2STR(dev->_1905_info.al_mac_addr));
#if 0
	SLIST_FOREACH(radio, &(dev->first_radio), next_radio) {
		ch_planning_update_all_dev_state((u8)CHPLAN_STATE_CH_CHANGE_TRIGGERED,radio->channel[0],ctx);
	}
	ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
	mapd_restart_channel_plannig(global);
#endif
	SLIST_FOREACH(radio, &(dev->first_radio), next_radio) {
		radio->dev_ch_plan_info.dev_ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
		err("Channel_planning triggered for %u", radio->channel[0]);
	}
}
#endif
/**
* @brief Fn to select device for channel planning
* @param ctx own global structure
*/
struct _1905_map_device *ch_planning_get_target_dev(struct own_1905_device *ctx)
{
	struct _1905_map_device *dev = NULL;
	struct os_time current_time;
	struct mapd_global *mapd_global_ptr = ctx->back_ptr;
	/* wait for 2 mins before starting channel planning */
	os_get_time(&current_time);
	SLIST_FOREACH(dev, &(ctx->_1905_dev_head), next_1905_device) {

		if (dev->in_network &&
			!dev->ch_preference_available)
		{
			map_1905_Send_Channel_Preference_Query_Message(mapd_global_ptr->_1905_ctrl,
				(char *)dev->_1905_info.al_mac_addr);
		}

		if (dev->in_network &&
			dev->ch_preference_available &&
			dev->radio_mapping_completed &&
			((current_time.sec - dev->first_seen.sec)
			> ctx->channel_planning_initial_timeout) &&
			!dev->channel_planning_completed)
		{
#ifdef MAP_R2
			if(ctx->ch_planning_R2.ch_plan_enable == TRUE) {
				ch_planning_allow_ch_sync(ctx, dev);
			}
#endif
			return dev;
		}
	}
	return NULL;
}

unsigned char ch_planning_get_centre_freq_ch(unsigned char channel, unsigned char op)
{
#ifdef MAP_160BW
	if (op == 129) {
		if (channel >= 36 && channel <= 64)
			return 50;
		else if (channel >= 100 && channel <= 128)
			return 114;
	} else
#endif
	{
		if (channel >= 36 && channel <= 48)
			return 42;
		else if (channel >= 52 && channel <= 64)
			return 58;
		else if (channel >= 100 && channel <= 112)
			return 106;
		else if (channel >= 116 && channel <= 128)
			return 122;
		else if (channel >= 132 && channel <= 144)
			return 138;
		else if (channel >= 149 && channel <= 161)
			return 155;
	}
	return 0;
}

void ch_planning_get_channel_block(unsigned char channel, unsigned char channel_block[8], unsigned char op, int maxbw)
{
	u8 *grp_list = NULL;
	int i = 0;
#ifdef MAP_160BW
	if (maxbw == BW_160 || op == 129) {
		grp_list = &grp_bw_160[0][0];
		if (channel >= 36 && channel <= 64) {
			os_memcpy(&channel_block[0], (grp_list), 8);
		} else if (channel >= 100 && channel <= 128) {
			os_memcpy(&channel_block[0], (grp_list + 8), 8);
		}
	} else
#endif
	{
		if (maxbw == BW_80) {
			grp_list = &grp_bw_80[0][0];
			for (i = 0;i < MAX_BW_80_BLOCK;i++) {
				if (channel == grp_bw_80[i][0] || channel == grp_bw_80[i][1]
					|| channel == grp_bw_80[i][2] || channel == grp_bw_80[i][3]) {
					channel_block[0] = grp_bw_80[i][0];
					channel_block[1] = grp_bw_80[i][1];
					channel_block[2] = grp_bw_80[i][2];
					channel_block[3] = grp_bw_80[i][3];
					break;
				}
			}
		} else if(maxbw == BW_40) {
			for (i = 0;i < MAX_BW_40_BLOCK;i++) {
				if (channel == grp_bw_40[i][0] || channel == grp_bw_40[i][1]) {
					channel_block[0] = grp_bw_40[i][0];
					channel_block[1] = grp_bw_40[i][1];
					break;
				}
			}
		} else {
			channel_block[0] = channel;
		}
	}
}

unsigned int ch_planning_check_channel_available(struct own_1905_device *ctx,
	unsigned char channel)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	struct radio_info_db *radio_info = NULL;
	struct prefer_info_db *prefer_db = NULL;
	int i = 0;

	while (dev != NULL) {
		if (dev->in_network) {
			radio_info = topo_srv_get_radio(dev, NULL);
				while (radio_info) {
					SLIST_FOREACH(prefer_db,
						&radio_info->chan_preferance.prefer_info_head, prefer_info_entry) {
						for(i = 0; i < prefer_db->ch_num; i++)
						{
							if (prefer_db->ch_list[i] == channel) {
								return TRUE;
							}
						}
					}
					radio_info = topo_srv_get_next_radio(dev,
						radio_info);
				}
		}
		dev = topo_srv_get_next_1905_device(ctx, dev);
	}
	return FALSE;
}

unsigned int ch_planning_get_operable_blocks(struct own_1905_device *ctx,
	unsigned char channel)
{
	unsigned int ret = 0;
#ifdef MAP_160BW
	unsigned char channel_block[8] = {0};
#else
	unsigned char channel_block[4] = {0};
#endif
	int i = 0, loop = 4;
	struct _1905_map_device *dev = NULL;
	struct radio_info_db *radio_info = NULL;
	int maxbw = BW_20, bw = BW_20;
	struct _1905_map_device *_1905_dev = NULL;
	SLIST_FOREACH(_1905_dev, &ctx->_1905_dev_head, next_1905_device) {
		bw = ch_planning_get_dev_bw_from_channel(_1905_dev, channel);
		if (maxbw < bw)
			maxbw = bw;
	}

	dev = topo_srv_get_1905_device(ctx, NULL);
	radio_info = topo_srv_get_radio_by_band(dev, channel);
	if (channel > 14) {
#ifdef MAP_160BW
		if (radio_info->operating_class == 129)
			loop = 8;
#endif
		ch_planning_get_channel_block(channel, channel_block, radio_info->operating_class, maxbw);
		for (i = 0; i < loop; i++) {
			if (ch_planning_check_channel_available(ctx, channel_block[i]))
				ret++;
		}
	} else {
		ret = 2;
	}
	return ret;
}

#ifdef WIFI_MD_COEX_SUPPORT
unsigned int ch_planning_check_channel_for_dev_operable_wrapper(struct _1905_map_device *dev,
	unsigned char channel)
{
	unsigned int ret = TRUE;
	unsigned char channel_block[4] = {0};
	int i = 0;

	if (channel > 14) {
		ch_planning_get_channel_block(channel, channel_block);
		for (i = 0; i < 4; i++) {
			//! check individual block
			ret = ch_planning_check_channel_operable_for_dev(dev, channel_block[i]);
			if (ret == FALSE)
				break;
		}
	} else {
		ret = ch_planning_check_channel_operable_for_dev(dev, channel);
	}
	return ret;
}

unsigned int ch_planning_check_channel_operable_for_dev(struct _1905_map_device *dev,
	unsigned char channel)
{
	struct radio_info_db *radio_info = NULL;
	struct prefer_info_db *prefer_db = NULL;
	int i = 0;
	//! loop for each device
	if (dev->in_network) {
		radio_info = topo_srv_get_radio(dev, NULL);
			//!  check each radio
			while (radio_info) {
				SLIST_FOREACH(prefer_db,
					&radio_info->chan_preferance.prefer_info_head, prefer_info_entry) {
					for(i = 0; i < prefer_db->ch_num; i++)
					{
						//! channel is present and preference is set to 0
						//! channel is not operable
						if (prefer_db->ch_list[i] == channel &&
							prefer_db->perference == 0)
							return FALSE;
					}
				}
				radio_info = topo_srv_get_next_radio(dev,
					radio_info);
			}
	}
	//! either channel is not present in preference list or not
	//! marked as 0 preference by any device
	return TRUE;
}
#endif

unsigned int ch_planning_check_channel_operable_wrapper(struct own_1905_device *ctx,
	unsigned char channel)
{
	unsigned int ret = TRUE;
#ifdef MAP_160BW
	unsigned char channel_block[8] = {0};
#else
	unsigned char channel_block[4] = {0};
#endif
	int i = 0, loop = 4;
	struct _1905_map_device *dev = NULL;
	struct radio_info_db *radio_info = NULL;
	int maxbw = BW_20, bw = BW_20;
		struct _1905_map_device *_1905_dev = NULL;
		SLIST_FOREACH(_1905_dev, &ctx->_1905_dev_head, next_1905_device) {
			bw = ch_planning_get_dev_bw_from_channel(_1905_dev, channel);
			if (maxbw < bw)
				maxbw = bw;
	}

	dev = topo_srv_get_1905_device(ctx, NULL);
	radio_info = topo_srv_get_radio_by_band(dev, channel);

	//! if it is a 5G channel then all 20Mhz blocks should be operable
	if (channel > 14) {
#ifdef MAP_160BW
		if (radio_info->operating_class == 129)
			loop = 8;
#endif
		ch_planning_get_channel_block(channel, channel_block, radio_info->operating_class, maxbw);
		for (i = 0; i < loop; i++) {
#ifdef MAP_160BW
			if (channel_block[i] == 0)
				continue;
#endif
			//! check individual block
			ret = ch_planning_check_channel_operable(ctx, channel_block[i], maxbw);
			if (ret == FALSE)
				break;
		}
	}
	return ret;
}

unsigned int ch_planning_check_channel_operable(struct own_1905_device *ctx,
	unsigned char channel, int maxbw)
{
	struct _1905_map_device *dev = topo_srv_get_1905_device(ctx, NULL);
	struct radio_info_db *radio_info = NULL;
	struct prefer_info_db *prefer_db = NULL;
	int i = 0;

	while (dev != NULL) {
		//! loop for each device
		if (dev->in_network) {
			radio_info = topo_srv_get_radio(dev, NULL);
				//!  check each radio
				while (radio_info) {
					SLIST_FOREACH(prefer_db,
						&radio_info->chan_preferance.prefer_info_head, prefer_info_entry) {
						for(i = 0; i < prefer_db->ch_num; i++)
						{
							//! channel is present and preference is set to 0
							//! channel is not operable
							if (maxbw >= chan_mon_get_bw_from_op_class(prefer_db->op_class) &&
								prefer_db->ch_list[i] == channel &&
								prefer_db->perference == 0) {
								debug("prefer ch nop: %d", prefer_db->ch_list[i]);
								return FALSE;
							}
						}
					}
					radio_info = topo_srv_get_next_radio(dev,
						radio_info);
				}
		}
		dev = topo_srv_get_next_1905_device(ctx, dev);
	}
	//! either channel is not present in preference list or not
	//! marked as 0 preference by any device
	return TRUE;
}

/**
* @brief Fn to find best channel for a radio
* @param ch_distribution chanel_planning global struct
* @param radio
* @return best channel for the radio
* 	return value 0 means radio is already on best possible channel
*	return value negative means channel planning cannot be performed for the radio currently
*	return type should always be signed int to avoid treating channel > 128 as negative value
*/
signed int ch_planning_select_best_channel(
	struct own_1905_device *ctx,
	struct radio_info_db *radio,
	struct _1905_map_device *dev
	)
{
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct prefered_ch_cb *prefered_channel = NULL;
	struct prefered_ch_cb *current_prefered_channel = NULL;
	struct prefered_ch_cb *new_prefered_channel = NULL;
	struct prefered_ch_radio_info_db *prefered_ch_radio = NULL;
	struct _1905_map_device *cnt_dev = NULL;
	struct radio_info_db *cnt_radio = NULL;
	unsigned char centre_freq_ch = 0;

	if (radio->channel[0] == 0)
	{
		err("Operating channel not known yet\n");
		return -1;
	}
	if (radio->chan_preferance.op_class_num == 0)
	{
		err("Channel preference not known yet\n");
		return -1;
	}


	if (radio->channel[0] > 14)
	{
		info("5G\n");
		ch_distribution = &ch_planning->ch_ditribution_5g;
	} else {
		info("2.4G\n");
		ch_distribution = &ch_planning->ch_ditribution_2g;
	}
	ch_planning_update_ch_score(ctx, ch_distribution);
	SLIST_FOREACH(current_prefered_channel,
		&(ch_distribution->first_prefered_ch),
		next_prefered_ch)
	{
		if (current_prefered_channel->ch_num == radio->channel[0])
			break;
	}

	if (current_prefered_channel == NULL)
	{
		goto bail_out;
	}
	if (!ch_planning_check_channel_operable_wrapper(ctx,current_prefered_channel->ch_num)) {
		current_prefered_channel->ch_score = -1;
	}
	SLIST_FOREACH(prefered_channel,
		&(ch_distribution->first_prefered_ch),
		next_prefered_ch) {
		if (!ch_planning_check_channel_operable_wrapper(ctx,prefered_channel->ch_num)) {
			prefered_channel->ch_score = -1;
		}
#ifndef MAP_160BW
		if (prefered_channel->ch_num == 50 ||
			prefered_channel->ch_num == 114 ||
			prefered_channel->ch_num == 165 ||
			prefered_channel->ch_num == 169)
		{
			always("160 Mhz Channel, skip %d\n",
				prefered_channel->ch_num);
			continue;
		}
#endif
		centre_freq_ch = ch_planning_get_centre_freq_ch(prefered_channel->ch_num, radio->operating_class);
		if (prefered_channel->ch_num == centre_freq_ch)
		{
			always("Not suitable candidate as  %d is a centre freq in 80Mhz\n",
				prefered_channel->ch_num);
			continue;
		}
		if (current_prefered_channel == prefered_channel)
		{
			continue;
		}
		if (prefered_channel->ch_score == -1)
		{
			always("Continue as channel %d is currently not operable\n",
				prefered_channel->ch_num);
			continue;
		}
		if (current_prefered_channel->ch_score > prefered_channel->ch_score)
		{
			continue;
		}
		SLIST_FOREACH(prefered_ch_radio,
			&(prefered_channel->first_radio),
			next_pref_ch_radio) {
			if ((prefered_ch_radio->radio == radio))
			{
				if (radio->operating_channel == NULL ||
					radio->chan_preferance.op_class_num == 0) {
					err("Channel Distribution not complete yet\n");
					return -1;
				}
				if (new_prefered_channel == NULL ||
					new_prefered_channel->ch_score < prefered_channel->ch_score)
				{
					new_prefered_channel = prefered_channel;
					err("Radio operating on %d, score = %x,possible better channel %d, score = %x\n",
						radio->channel[0], current_prefered_channel->ch_score,
						prefered_channel->ch_num,
						new_prefered_channel->ch_score);
					//return (signed int)(prefered_channel->ch_num);
				}
				break;
			}
		}
	}

	if (new_prefered_channel &&
		(new_prefered_channel->ch_score > current_prefered_channel->ch_score)) {
		if (dev->device_role == DEVICE_ROLE_CONTROLLER) {
			cnt_dev = topo_srv_get_1905_device(ctx, NULL);
			if (cnt_dev) {
				cnt_radio = topo_srv_get_radio_by_band(cnt_dev, radio->channel[0]);
				if (cnt_radio)
					cnt_radio->prev_channel = new_prefered_channel->ch_num;
			}
		}
		err("Best possible option = %d", new_prefered_channel->ch_num);
		return new_prefered_channel->ch_num;
	} else if (new_prefered_channel &&
		(new_prefered_channel->ch_score == current_prefered_channel->ch_score) &&
		dev->device_role == DEVICE_ROLE_AGENT) {
		cnt_dev = topo_srv_get_1905_device(ctx, NULL);
		if (cnt_dev) {
			cnt_radio = topo_srv_get_radio_by_band(cnt_dev, radio->channel[0]);
			if (cnt_radio) {
				if(cnt_radio->channel[0] == radio->channel[0]){
					err("Radio already on best possible channel %d\n", cnt_radio->channel[0]);
					return 0;
				} else if (cnt_radio->channel[0] != cnt_radio->prev_channel) {
					return cnt_radio->prev_channel;
				}
				err("New radio channel: %u", cnt_radio->channel[0]);
				return cnt_radio->channel[0];
			}
		}
	} else {
		err("Radio already on best possible channel\n");
	}
	return 0;
bail_out:
	err("Prefered channel list not present for the device yet\n");
	return -1;
}
#ifdef MAP_R2
u8 ch_planning_check_CAC_success(
	struct own_1905_device *ctx,
	u8 channel)
{
	/*if any dev's radio has done cac successful for this channel then we return 1*/
	struct radio_info_db *radio = NULL;
	struct _1905_map_device *_1905_dev = NULL;
	SLIST_FOREACH(_1905_dev, &ctx->_1905_dev_head, next_1905_device) {
		radio = topo_srv_get_radio_by_band(_1905_dev, channel);
		if(!radio){
			err("radio not found for dev");
			continue;
		}
		if (radio->cac_comp_status.cac_status == CAC_SUCCESSFUL &&
			channel == radio->cac_comp_status.channel) {
			err("CAC success found on ch %d at dev"MACSTR"", channel, MAC2STR(_1905_dev->_1905_info.al_mac_addr));
			return 1;
		}
	}
return 0;
}
#endif
u8 ch_planning_all_pref_done(
              struct ch_prefer_lib *ch_prefer)
{
       u8 j=0;
       for (j = 0; j < ch_prefer->op_class_num; j++) {
               if(ch_prefer->opinfo[j].perference == 0)
                       return 0;
       }
       return 1;
}
void dump_ch_prefer_info(
	struct ch_prefer_lib *ch_prefer)
{
	u8 i = 0;
	for (i = 0; i < ch_prefer->op_class_num; i++) {
		 err("opclass: %u, chnum: %u,ch_list %u, pref: %u",
		 		ch_prefer->opinfo[i].op_class,
                ch_prefer->opinfo[i].ch_num,
                ch_prefer->opinfo[i].ch_list[0],
                ch_prefer->opinfo[i].perference);
	}

}
/*Re-define the func of ch_planning_ch_selection_prefer_data.
This is because there is IOT issue of Channel Preference TLV with Qualcomm Agent.
According to R1 spec, we should not only insert the preferred Channel but also need insert all other Channels with lower preference value*/

/*
@ [input] channel: the best channel which Multi-AP Agent is asked to operated on
@ [input] radio: the data struct of radio of Multi-AP Agent
@ [output] ch_prefer: Channel Preference data which is used to construct the Channel Preference TLV.
*/
void ch_planning_ch_selection_prefer_data(
	struct own_1905_device *ctx,
	unsigned char channel,
	struct radio_info_db *radio,
	struct ch_prefer_lib *ch_prefer)
{
	struct prefer_info_db *prefer_info = NULL;
	unsigned char num_of_op_class = 0; //num of operating class withing one radio unique
	int i = 0, j = 0, offset = 0;
	unsigned char op_class_match = FALSE; //boolean value

	os_memcpy(ch_prefer->identifier, radio->identifier, ETH_ALEN);

	SLIST_FOREACH(prefer_info, &(radio->chan_preferance.prefer_info_head), prefer_info_entry) {
	//This loop is insert the preferred channel with higher preference value including 20M, 40M, 80M of channel with op class

		if (prefer_info->op_class <= 127)
		{
			for (i = 0; i < prefer_info->ch_num; i++)
			{
				if (channel	== prefer_info->ch_list[i])
				{
					ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
					ch_prefer->opinfo[num_of_op_class].ch_num = 1;
					ch_prefer->opinfo[num_of_op_class].ch_list[0] = channel;
					if(prefer_info->perference == 15)
						ch_prefer->opinfo[num_of_op_class].perference = 14;
					else
						ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;//14;
#ifdef MAP_R2
					u8 check_cac = ch_planning_check_CAC_success(ctx, channel);
					if(check_cac) {
						ch_prefer->opinfo[num_of_op_class].reason = DFS_CH_CLEAR_INDICATION;
						err("CAC successful, clear indication not to CAC %d", channel);
					}
#endif

					num_of_op_class++;
					if(num_of_op_class == MAX_OP_CLASS_NUM)
					{
						err("[ERROR]number of OP class is too big, can't insert Channel Preference anymore.");
						goto finish;
					}
				}
			}
		}
		else if(prefer_info->op_class == 128 && radio->band != BAND_2G)
		{
			for (i = 0; i < prefer_info->ch_num; i++)
			{
				offset = channel - prefer_info->ch_list[i];
				if (abs(offset) <= 6) //the channel is within the 80M coverage
				{
					ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
					ch_prefer->opinfo[num_of_op_class].ch_num = 1;
					ch_prefer->opinfo[num_of_op_class].ch_list[0] = prefer_info->ch_list[i];
					if(prefer_info->perference == 15)
						ch_prefer->opinfo[num_of_op_class].perference = 14;
					else
						ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;//14;
#ifdef MAP_R2
					u8 check_cac = ch_planning_check_CAC_success(ctx, channel);
					if(check_cac) {
						ch_prefer->opinfo[num_of_op_class].reason = DFS_CH_CLEAR_INDICATION;
						err("CAC successful, clear indication not to CAC %d", channel);
					}
#endif
					num_of_op_class++;
					ch_prefer->opinfo[num_of_op_class].op_class = chan_mon_get_op_class_frm_channel(channel, BW_20);
					ch_prefer->opinfo[num_of_op_class].ch_num = 1;
					ch_prefer->opinfo[num_of_op_class].ch_list[0] = channel;
					ch_prefer->opinfo[num_of_op_class].reason = ch_prefer->opinfo[num_of_op_class - 1].reason;
					ch_prefer->opinfo[num_of_op_class].perference = ch_prefer->opinfo[num_of_op_class - 1].perference;

					num_of_op_class++;
					if(num_of_op_class == MAX_OP_CLASS_NUM)
					{
						err("[ERROR]number of OP class is too big, can't insert Channel Preference anymore.");
						goto finish;
					}
				}
			}

		}
#ifdef MAP_160BW
		else if(prefer_info->op_class == 129 && radio->band != BAND_2G) {
			for (i = 0; i < prefer_info->ch_num; i++)
			{
				offset = channel - prefer_info->ch_list[i];
				if (abs(offset) <= 14) //the channel is within the 160M coverage
				{
					ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
					ch_prefer->opinfo[num_of_op_class].ch_num = 1;
					ch_prefer->opinfo[num_of_op_class].ch_list[0] = prefer_info->ch_list[i];
					if(prefer_info->perference == 15)
						ch_prefer->opinfo[num_of_op_class].perference = 14;
					else
						ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;//14;
#ifdef MAP_R2
					u8 check_cac = ch_planning_check_CAC_success(ctx, channel);
					if(check_cac) {
						ch_prefer->opinfo[num_of_op_class].reason = DFS_CH_CLEAR_INDICATION;
						err("CAC successful, clear indication not to CAC %d", channel);
					}
#endif
					num_of_op_class++;
					ch_prefer->opinfo[num_of_op_class].op_class = chan_mon_get_op_class_frm_channel(channel, BW_20);
					ch_prefer->opinfo[num_of_op_class].ch_num = 1;
					ch_prefer->opinfo[num_of_op_class].ch_list[0] = channel;
					ch_prefer->opinfo[num_of_op_class].reason = ch_prefer->opinfo[num_of_op_class - 1].reason;
					ch_prefer->opinfo[num_of_op_class].perference = ch_prefer->opinfo[num_of_op_class - 1].perference;

					num_of_op_class++;
					if(num_of_op_class == MAX_OP_CLASS_NUM)
					{
						err("[ERROR]number of OP class is too big, can't insert Channel Preference anymore.");
						goto finish;
					}
				}
			}
		}
#endif
	} // end of SLIST_FOREACH(prefer_info, &(radio->chan_preferance.prefer_info_head), prefer_info_entry)

	SLIST_FOREACH(prefer_info, &(radio->chan_preferance.prefer_info_head), prefer_info_entry) {
	//This loop is insert all the channels except preferred channel to assign lower preference value

		op_class_match = FALSE;
		if (prefer_info->op_class <= 127)
		{
			op_class_match = TRUE;
			ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
			ch_prefer->opinfo[num_of_op_class].ch_num = prefer_info->ch_num;
			if(radio->band == BAND_2G)
				ch_prefer->opinfo[num_of_op_class].perference = 0;
			else
				ch_prefer->opinfo[num_of_op_class].perference = 1; //should at least one non-DFS channel as operable

			if(prefer_info->reason){
				if(prefer_info->perference == 15)
					ch_prefer->opinfo[num_of_op_class].perference = 14;
				else
					ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;
				ch_prefer->opinfo[num_of_op_class].reason = prefer_info->reason;
			}

			i = 0; //index for prefer_info->ch_list[]
			j = 0; //index for ch_prefer->opinfo[].ch_list[]
			while (i < prefer_info->ch_num)
			{	//ch_list should not contain the input channel
				if (channel	!= prefer_info->ch_list[i])
				{
					ch_prefer->opinfo[num_of_op_class].ch_list[j] = prefer_info->ch_list[i];
					j++;
				}
				else
				{
					ch_prefer->opinfo[num_of_op_class].ch_num--;
				}

				i++;
			}
		}
		else if(prefer_info->op_class == 128 && radio->band != BAND_2G)
		{
			op_class_match = TRUE;
			ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
			ch_prefer->opinfo[num_of_op_class].ch_num = prefer_info->ch_num;
			ch_prefer->opinfo[num_of_op_class].perference = 1;
			if(prefer_info->reason){
				if(prefer_info->perference == 15)
					ch_prefer->opinfo[num_of_op_class].perference = 14;
				else
					ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;
				ch_prefer->opinfo[num_of_op_class].reason = prefer_info->reason;
			}

			i = 0; //index for prefer_info->ch_list[]
			j = 0; //index for ch_prefer->opinfo[].ch_list[]
			while (i < prefer_info->ch_num)
			{
				offset = channel - prefer_info->ch_list[i];
				if (abs(offset) > 6) //the channel is within the 80M coverage
				{
					ch_prefer->opinfo[num_of_op_class].ch_list[j] = prefer_info->ch_list[i];
					j++;
				}
				else
				{
					ch_prefer->opinfo[num_of_op_class].ch_num--;
				}
				i++;
			}
		}
		else if(prefer_info->op_class == 129 && radio->band != BAND_2G)
		{
			op_class_match = TRUE;
			ch_prefer->opinfo[num_of_op_class].op_class = prefer_info->op_class;
			ch_prefer->opinfo[num_of_op_class].ch_num = prefer_info->ch_num;
			ch_prefer->opinfo[num_of_op_class].perference = 1;
			if(prefer_info->reason){
				if(prefer_info->perference == 15)
					ch_prefer->opinfo[num_of_op_class].perference = 14;
				else
					ch_prefer->opinfo[num_of_op_class].perference = prefer_info->perference;
				ch_prefer->opinfo[num_of_op_class].reason = prefer_info->reason;
			}

			i = 0; //index for prefer_info->ch_list[]
			j = 0; //index for ch_prefer->opinfo[].ch_list[]
			while (i < prefer_info->ch_num)
			{
				offset = channel - prefer_info->ch_list[i];
				if (abs(offset) > 14) //the channel is within the 160M coverage
				{
					ch_prefer->opinfo[num_of_op_class].ch_list[j] = prefer_info->ch_list[i];
					j++;
				}
				else
				{
					ch_prefer->opinfo[num_of_op_class].ch_num--;
				}
				i++;
			}
		}

		if(op_class_match){
			num_of_op_class++;
			if(num_of_op_class == MAX_OP_CLASS_NUM)
			{
				err("[ERROR]number of OP class is too big, can't insert Channel Preference anymore.");
				goto finish;
			}
		}
	} // end of SLIST_FOREACH(prefer_info, &(radio->chan_preferance.prefer_info_head), prefer_info_entry)
finish:
	ch_prefer->op_class_num = num_of_op_class;

	//dump_ch_prefer_info(ch_prefer);
}
/**
* @brief Fn timeout handler for channel selection request
*/

void ch_planning_timeout_handler(void * eloop_ctx, void *user_ctx)
{
	struct own_1905_device *ctx = eloop_ctx;
	struct _1905_map_device *dev = user_ctx;

	if (ctx->ch_planning.current_ch_planning_dev == dev)
	{
		err("ch_planning_timeout for %02x:%02x:%02x:%02x:%02x:%02x\n",
			PRINT_MAC(dev->_1905_info.al_mac_addr));
		//dev->channel_planning_completed = TRUE;
		ctx->ch_planning.current_ch_planning_dev = NULL;
		ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
#ifdef MAP_R2
		if(ctx->ch_planning_R2.ch_plan_enable == TRUE &&
			ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_CH_CHANGE_TRIGGERED) {
			err("operating ch report did not come but reset states"MACSTR"", MAC2STR(dev->_1905_info.al_mac_addr));
			ch_planning_handle_ch_selection_rsp(ctx,dev);
		} else {
			ch_planning_trigger_net_opt_post_ch_plan(ctx);
		}
#endif
	}
}
/**
* @brief Fn decide channel score
* @param ch_distribution_cb channel planning global
* @param channel target channel
*/

void ch_planning_update_ch_score(
	struct own_1905_device *ctx,
	struct ch_distribution_cb *ch_distribution)
{
	char operating_count = 0;
	struct prefered_ch_cb *prefered_ch;
	unsigned int channel_operable = 0;
	signed int operable_blocks = 0;
	SLIST_FOREACH(prefered_ch, &ch_distribution->first_prefered_ch, next_prefered_ch){
		channel_operable = ch_planning_check_channel_operable_wrapper(ctx, prefered_ch->ch_num);
		//! radar is not present, score = user_prefered << 24| prefered_count << 16 | operating_count << 8 | operable blocks
		if (channel_operable) {
#ifdef MAP_R2
			if(ctx->ch_planning_R2.ch_plan_enable == TRUE) {
				if(ctx->ch_planning_R2.ch_plan_enable_bw && (prefered_ch->ch_num > 14)) {
					/*If map R2 and the feature is enabled then use ch group Rank instead of operating count*/
					operating_count = ch_planning_get_grp_rank(ctx, prefered_ch->ch_num);
					err(" channel %d grp rank %d", prefered_ch->ch_num, operating_count);
				} else {
					/*If map R2 then use channel Rank instead of operating count*/
					operating_count = ch_planning_get_ch_rank(ctx, prefered_ch->ch_num);
				}
			} else {
				operating_count = ch_planning_get_num_radio_on_operating_channel(ch_distribution,
				prefered_ch->ch_num);
			}
#else
			operating_count = ch_planning_get_num_radio_on_operating_channel(ch_distribution,
				prefered_ch->ch_num);
#endif
			operable_blocks = ch_planning_get_operable_blocks(ctx, prefered_ch->ch_num);
			prefered_ch->ch_score = (prefered_ch->radio_count << 8) | operating_count;
			if (prefered_ch->ch_num == ch_distribution->user_prefered_ch) {
				debug("update normal score MSB set %d\n",prefered_ch->ch_num);
				prefered_ch->ch_score |= 1 << 16;
			}
			else if(prefered_ch->ch_num == ch_distribution->user_prefered_ch_HighBand) {
				debug("update high band score MSB set %d\n",prefered_ch->ch_num);
				prefered_ch->ch_score |= 1 << 16;
			}
			prefered_ch->ch_score = (prefered_ch->ch_score << 8) | operable_blocks;
		} else {
			//! radar found, score is -1
			prefered_ch->ch_score = -1;
		}
	}
	//ch_planning_show_ch_distribution(ctx);

}

/**
* @brief Fn to add radio to prefered channel list
* @param ch_distribution_cb channel planning global
* @param radio target radio
* @param prefered_ch element
*/

void ch_planning_add_radio_to_prefered_ch(
	struct own_1905_device *ctx,
	struct ch_distribution_cb *ch_distribution,
	struct radio_info_db *radio,
	struct prefered_ch_cb *prefered_ch)
{
	struct prefered_ch_radio_info_db *prefered_ch_radio =NULL;
	prefered_ch_radio =
		os_zalloc(sizeof(struct prefered_ch_radio_info_db));
	prefered_ch_radio->radio = radio;
	SLIST_INSERT_HEAD(
		&(prefered_ch->first_radio),
		prefered_ch_radio,
		next_pref_ch_radio);
	prefered_ch->radio_count++;
	//ch_planning_update_ch_score(ctx, ch_distribution, prefered_ch->ch_num);
}

/**
* @brief Fn to add a channel to prefered list
* @param ctx own 1905 global
* @param channel target channel
* @param radio target radio
* @param band
* @param preference
* @param reason for preference
*/

void ch_planning_add_ch_to_prefered_list(
	struct own_1905_device *ctx,
	unsigned char channel,
	struct radio_info_db *radio,
	unsigned char band,
	unsigned char preference,
	unsigned char reason)
{

	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct prefered_ch_cb *prefered_ch= NULL;
	struct prefered_ch_radio_info_db *prefered_ch_radio =NULL;
	//err("%s\n", __FUNCTION__);
	if (band)
	{
		//err("ch_ditribution_5g\n");
		ch_distribution = &ch_planning->ch_ditribution_5g;
	} else {
		//err("ch_ditribution_2g\n");
		ch_distribution = &ch_planning->ch_ditribution_2g;
	}
	SLIST_FOREACH(prefered_ch, &(ch_distribution->first_prefered_ch), next_prefered_ch) {
		if (prefered_ch->ch_num == channel)
		{
			SLIST_FOREACH(prefered_ch_radio,
				&(prefered_ch->first_radio), next_pref_ch_radio) {
				if (prefered_ch_radio->radio == radio) {
					break;
				}
			}
			if (prefered_ch_radio == NULL) {
				ch_planning_add_radio_to_prefered_ch(ctx, ch_distribution,
					radio, prefered_ch);
			}
			break;
		}
	}
	if (prefered_ch == NULL)
	{
		//err("This channel not currently predent in prefered list\n");
		prefered_ch = os_zalloc(sizeof(struct prefered_ch_cb));
		SLIST_INIT(&(prefered_ch->first_radio));
		//err("Allocate a new prefered channel and add radio to it\n");
		prefered_ch->ch_num = channel;
		if (ch_distribution->prefered_ch_count == 0)
		{
			SLIST_INIT(&(ch_distribution->first_prefered_ch));
		}
		SLIST_INSERT_HEAD(
			&(ch_distribution->first_prefered_ch),
			prefered_ch,
			next_prefered_ch);
		ch_planning_add_radio_to_prefered_ch(ctx, ch_distribution,
			radio, prefered_ch);
		ch_distribution->prefered_ch_count++;
	}
	//ch_planning_update_ch_score(ctx, ch_distribution, channel);
}
/**
* @brief Fn to remove channel from prefered list
* @param ctx own 1905 global
* @param channel target channel
* @param radio target radio
* @param band
*/

void ch_planning_remove_ch_from_prefered_list(
	struct own_1905_device *ctx,
	unsigned char channel,
	struct radio_info_db *radio,
	unsigned char band)
{
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct prefered_ch_cb *prefered_ch= NULL;
	struct prefered_ch_cb *prefered_ch_temp= NULL;
	struct prefered_ch_radio_info_db *prefered_ch_radio_temp = NULL;

	//err("%s\n", __FUNCTION__);
	if (band)
	{
		//err("ch_ditribution_5g\n");
		ch_distribution = &ch_planning->ch_ditribution_5g;
	} else {
		//err("ch_ditribution_2g\n");
		ch_distribution = &ch_planning->ch_ditribution_2g;
	}
	SLIST_FOREACH(prefered_ch, &(ch_distribution->first_prefered_ch), next_prefered_ch) {
		if (prefered_ch->ch_num == channel)
		{
			//err("Channel %d is present in prefered list\n", channel);
			SLIST_FOREACH(prefered_ch_radio_temp,
				&(prefered_ch->first_radio),
				next_pref_ch_radio) {
				if (prefered_ch_radio_temp->radio == radio)
				{
					//err("remove radio from the list\n");
					SLIST_REMOVE(&(prefered_ch->first_radio),
					prefered_ch_radio_temp,
					prefered_ch_radio_info_db,
					next_pref_ch_radio);
					os_free(prefered_ch_radio_temp);
					prefered_ch->radio_count--;
					break;
				}
			}
			if (prefered_ch->radio_count == 0) {
				//err("No more radios prefer channel %d\n", prefered_ch->ch_num);
				prefered_ch_temp = SLIST_NEXT(prefered_ch, next_prefered_ch);
				SLIST_REMOVE(&(ch_distribution->first_prefered_ch),
				prefered_ch,
				prefered_ch_cb,
				next_prefered_ch);
				os_free(prefered_ch);
				prefered_ch = prefered_ch_temp;
				ch_distribution->prefered_ch_count--;
				if (prefered_ch == NULL)
				{
					break;
				}
			}
		}
	}

	//ch_planning_update_ch_score(ctx, ch_distribution, channel);
}
/**
* @brief Fn to remove channel from operating list
* @param ctx own 1905 global
* @param channel target channel
* @param radio target radio
* @param band
*/

void ch_planning_remove_ch_from_operating_list(
	struct own_1905_device *ctx,
	unsigned char channel,
	struct radio_info_db *radio,
	unsigned char band)
{
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct operating_ch_cb *operating_ch= NULL;
	struct operating_ch_cb *operating_ch_temp = NULL;
	struct radio_info_db *radio_tmp = NULL;

	err("%s\n", __FUNCTION__);
	if (band)
	{
		//err("ch_ditribution_5g\n");
		ch_distribution = &ch_planning->ch_ditribution_5g;
	} else {
		//err("ch_ditribution_2g\n");
		ch_distribution = &ch_planning->ch_ditribution_2g;
	}
	SLIST_FOREACH(operating_ch,
		&(ch_distribution->first_operating_ch),
		next_operating_ch) {
		if (operating_ch->ch_num == channel)
		{
			err("Channel %d is present in operating list\n",
				channel);
			SLIST_FOREACH(radio_tmp,
				&(operating_ch->first_radio),
				next_co_ch_radio) {
				if (radio_tmp == radio)
				{
					err("remove radio from the list\n");
					SLIST_REMOVE(&(operating_ch->first_radio),
					radio_tmp,
					radio_info_db,
					next_co_ch_radio);
					operating_ch->radio_count--;
					//ch_planning_update_ch_score(ctx, ch_distribution, channel);
					break;
				}
			}
			if (operating_ch->radio_count == 0) {
				err("No more radios prefer channel %d\n",
					operating_ch->ch_num);
				operating_ch_temp = SLIST_NEXT(operating_ch, next_operating_ch);
				SLIST_REMOVE(&(ch_distribution->first_operating_ch),
				operating_ch,
				operating_ch_cb,
				next_operating_ch);
				os_free(operating_ch);
				operating_ch = operating_ch_temp;
				ch_distribution->prefered_ch_count--;
				if (operating_ch == NULL)
				{
					break;
				}
			}
		}
	}
	if (operating_ch != NULL)
	{
		err("Channel %d still exist, remove from list temporarily\n",
			operating_ch->ch_num);
		SLIST_REMOVE(&ch_distribution->first_operating_ch,
		operating_ch,
		operating_ch_cb,
		next_operating_ch);
		ch_planning_insert_into_ch_operating(ctx,
			operating_ch,
			ch_distribution);
	}
}
Boolean channel_validInList(struct own_1905_device *ctx,struct radio_info_db *radio, unsigned int channel)
{
	struct prefer_info_db *prefer_db;
	int j;
	if (radio->operating_channel == NULL ||
		radio->chan_preferance.op_class_num == 0) {
		err("Channel Distribution not complete yet\n");
		return FALSE;
	}
	SLIST_FOREACH(prefer_db,
		&radio->chan_preferance.prefer_info_head,
		prefer_info_entry) {
		for(j = 0; j < prefer_db->ch_num; j++)
		{
			if(channel==prefer_db->ch_list[j])
			{
				return TRUE;
			}
		}
	}
	err("channel %d not found in preferred list of radio %d",channel,radio->band);
	return FALSE;
}
/**
* @brief Fn to execute channel planning
* @param ctx own 1905 global
* @param dev target dev
*/

u8 ch_planning_find_max_pref_index(
	struct own_1905_device *ctx,
	struct ch_prefer_lib *ch_prefer)
{
	//aim is to keep the BW of device same ( so choose opclass accordingly)

	u8 max_pref_index = 0, j = 0;
	u8 max_opclass_pref = ch_prefer->opinfo[0].perference;
	u8 curr_opclass_pref = ch_prefer->opinfo[0].perference;
	u8 curr_bw = BW_20, max_bw = BW_20;
	u8 curr_op_bw = BW_20;
	struct radio_info_db *radio = NULL;
	struct _1905_map_device * _1905_device = topo_srv_get_1905_device(ctx,NULL);
	radio = topo_srv_get_radio(_1905_device,ch_prefer->identifier);
	if(!radio){
		err("radio is NULL");
	} else {
		curr_op_bw = chan_mon_get_bw_from_op_class(radio->operating_class);
		err("current operating BW %d, opclass %d, channel %d",
			curr_op_bw, radio->operating_class, radio->channel[0]);
	}
	 for (j = 0; j < ch_prefer->op_class_num; j++) {
		 curr_opclass_pref = ch_prefer->opinfo[j].perference;
		 if(curr_opclass_pref > max_opclass_pref) {
			 max_opclass_pref = ch_prefer->opinfo[j].perference;
			 max_pref_index = j;
		 } else if (curr_opclass_pref == max_opclass_pref) {
		 	curr_bw = chan_mon_get_bw_from_op_class(ch_prefer->opinfo[j].op_class);
			max_bw = chan_mon_get_bw_from_op_class(ch_prefer->opinfo[max_pref_index].op_class);
			err("curr_bw %d curr opclass %d", curr_bw, ch_prefer->opinfo[j].op_class);
			if(curr_bw == curr_op_bw) {
				 max_opclass_pref = ch_prefer->opinfo[j].perference;
				 max_pref_index = j;
			}
	 	}
	 }
	 err("found new ch to set %d",ch_prefer->opinfo[max_pref_index].ch_list[0]);
	return max_pref_index;
}
void ch_planning_trigger_net_opt_post_ch_plan(
	struct own_1905_device *ctx)
{
	struct _1905_map_device *dev = NULL;
	u8 pending = 0;
	u8 count = 0;
	SLIST_FOREACH(dev, &(ctx->_1905_dev_head), next_1905_device) {
		debug("dev role %d dev->in_network %d,dev->channel_planning_completed %d ",
			dev->device_role,
			dev->in_network,
			dev->channel_planning_completed);
		if (dev->in_network &&
			!dev->channel_planning_completed){
				pending = 1;
				break;
			}
	}
	if(pending == 0) {
		count = get_net_opt_dev_count((struct mapd_global *)ctx->back_ptr);
		if (count > 1) {
			err("all dev R1 channel planning is complete , safe to run network opt");
				eloop_register_timeout(ctx->network_optimization.wait_time,
					0, trigger_net_opt, ctx, NULL);
		}

	}

}
#ifdef MAP_R2
void ch_planning_send_select(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char ch_prefer_count,
	struct ch_prefer_lib *ch_prefer)
{
	struct mapd_global *global = ctx->back_ptr;
	struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(ctx, NULL);
	u8 max_pref_index = 0;
	if (own_1905_node != dev) {
		err("change on agent\n");
		map_1905_Send_Channel_Selection_Request_Message(
			global->_1905_ctrl,
			(char *)dev->_1905_info.al_mac_addr,
			ch_prefer_count,
			ch_prefer,
			0,
			NULL);
		eloop_register_timeout(5, 0, ch_planning_timeout_handler,
			ctx,dev);
	} else {
		err("Channel Planning on controller\n");
		struct channel_setting *setting = NULL;
		int i = 0;
		setting = os_zalloc(512);
		if(!setting){
			err("alloc fail ");
			return;
		}
		setting->ch_set_num = ch_prefer_count;
		while (i < setting->ch_set_num) {
			max_pref_index = ch_planning_find_max_pref_index(ctx, ch_prefer);
			setting->chinfo[i].channel = ch_prefer[i].opinfo[max_pref_index].ch_list[0];
			setting->chinfo[i].op_class = ch_prefer[i].opinfo[max_pref_index].op_class;
			os_memcpy(setting->chinfo[i].identifier, ch_prefer[i].identifier, ETH_ALEN);
			setting->chinfo[i].reason_code = ch_prefer[i].opinfo[max_pref_index].reason;
			err("Reason code: %d opclass %d channel %d", ch_prefer[i].opinfo[max_pref_index].reason,
				ch_prefer[i].opinfo[max_pref_index].op_class,
				ch_prefer[i].opinfo[max_pref_index].ch_list[0]);

			/* Using i+1 index for storing primary CH info at setting->chinfo */
			if ((ch_prefer[i].opinfo[max_pref_index].op_class == 128)
#ifdef MAP_160BW
			|| (ch_prefer[i].opinfo[max_pref_index].op_class == 129)
#endif
			) {
				unsigned char next_channel;

				next_channel = ch_prefer[i].opinfo[max_pref_index+1].ch_list[0];
				if (is_valid_primary_ch_80M_160M(next_channel, ch_prefer[i].opinfo[max_pref_index].ch_list[0],
										ch_prefer[i].opinfo[max_pref_index].op_class)) {
					setting->chinfo[i+1].channel = next_channel;
					setting->chinfo[i+1].op_class = ch_prefer[i].opinfo[max_pref_index+1].op_class;
					os_memcpy(setting->chinfo[i+1].identifier, ch_prefer[i].identifier, ETH_ALEN);
					setting->chinfo[i+1].reason_code = ch_prefer[i].opinfo[max_pref_index+1].reason;
					err("Primary channel Reason code: %d opclass %d channel %d",
						ch_prefer[i].opinfo[max_pref_index+1].reason,
						ch_prefer[i].opinfo[max_pref_index+1].op_class,
						ch_prefer[i].opinfo[max_pref_index+1].ch_list[0]);
					/* Since we just added one extra entry, increment the index by 1 to make sure it will not get overwritten in next iteration */
					i++;
					setting->ch_set_num++;
				}
			}
			i++;
		}
		wlanif_issue_wapp_command(ctx->back_ptr, WAPP_USER_SET_CHANNEL_SETTING, 0,
			NULL, NULL, setting, 512, 0, 0, 0);
		os_free(setting);
		dev->channel_planning_completed = TRUE;
		ctx->Restart_ch_planning_radar = 0;
		ctx->Restart_ch_planning_radar_on_agent = 0;
		ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
		ctx->ch_planning.current_ch_planning_dev = NULL;
	}
	if(ctx->ch_planning_R2.ch_plan_enable == TRUE)
		ch_planning_handle_ch_selection_rsp(ctx,dev);
	else {
		ch_planning_trigger_net_opt_post_ch_plan(ctx);
	}
}
#endif
//! channel array in this function as input helps to
//! helps to differentiate between convergent and
//! divergent ch planning.
//! for divergent planning, chnnl array will comprise of non zero values
//! for convergent these value will be all zeros,
//! best channel will be picked internally
void ch_planning_exec(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned int channel[])
{

#ifdef MAP_R2
	ch_planning_exec_R2(ctx, dev, channel);
#else
	struct radio_info_db *radio;
	unsigned char channel_planning_req = FALSE,avoid_ChPlanning_radio = FALSE;
	signed int best_channel = 0;
	unsigned int force_channel = 0;
	unsigned char ch_prefer_count = 0;
	struct ch_prefer_lib *ch_prefer;
	struct mapd_global *global = ctx->back_ptr;
	struct bh_link_entry *bh_entry = NULL;
	u8	max_pref_index = 0;

	ch_prefer = os_zalloc(sizeof(struct ch_prefer_lib) * 3);
	SLIST_FOREACH(radio, &(dev->first_radio), next_radio) {
		if((DEVICE_ROLE_CONTROLLER == dev->device_role) &&
			(global->dev.ThirdPartyConnection) && (!ctx->Restart_ch_planning_radar)) {
			avoid_ChPlanning_radio = FALSE;
			SLIST_FOREACH(bh_entry, &(ctx->bh_link_head), next_bh_link) {
				if((bh_entry->bh_channel == radio->channel[0] &&
					bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED) ||
 					(ctx->current_bh_state != BH_STATE_WIFI_LINKUP && ctx->bh_config_count > 0 )) {
					//! it is a divergent ch planning case and current radio
					//! is a controller radio with CLI connected to
					//! third party AP. channel planning cannot be executed on it
					avoid_ChPlanning_radio = TRUE;
				}
			}
			if(avoid_ChPlanning_radio) {
				err("Avoid Ch planning on this radio");
				continue;
			}
		}
		//! channel planning is executed only on the radio, whose APCLI interface
		//! is not connected to uplink device
		if (!radio->uplink_bh_present){
			//! convergent channel planing case
			if(!(channel[0]||channel[1]||channel[2])){
				//! find best possible channel for the radio
				best_channel = ch_planning_select_best_channel(ctx, radio, dev);
				err("bestchannel %d", best_channel);
				if(best_channel < 0){
					os_free(ch_prefer);
					return;
				} else if (best_channel > 0 && ch_planning_is_ch_dfs(ctx, best_channel)) {
					start_netopt_timer(ctx, best_channel);
				}
			}
			//! divergent channel planning cases
			else if(channel_validInList(ctx,radio,channel[0])) {
					force_channel = channel[0];
					global->dev.ch_planning.ch_planning_enabled = 0;
			}
			else if(channel_validInList(ctx,radio,channel[1])) {
					force_channel = channel[1];
					global->dev.ch_planning.ch_planning_enabled = 0;
			}
			else if(channel_validInList(ctx,radio,channel[2])) {
					force_channel = channel[3];
					global->dev.ch_planning.ch_planning_enabled = 0;
			}
			if (best_channel > 0) {
				//! prepare channel selection request data for current radio
				ch_planning_ch_selection_prefer_data(ctx, best_channel,
					radio, &ch_prefer[ch_prefer_count]);
				ch_prefer_count++;
				err(" ");
				channel_planning_req = TRUE;
			}
			if (force_channel>0) {
				//! prepare channel selection request data for current radio

				ch_planning_ch_selection_prefer_data(ctx, force_channel,
					radio, &ch_prefer[ch_prefer_count]);
				ch_prefer_count++;
				channel_planning_req = TRUE;
			}
		}
	}//radio loop ends here
	if (channel_planning_req)
	{
		ctx->ch_planning.ch_planning_state =
			CHANNEL_SELECTION_REQ_SENT;
		ctx->ch_planning.current_ch_planning_dev = dev;

		struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(ctx, NULL);
		if (own_1905_node != dev) {
			err("change on agent\n");
			map_1905_Send_Channel_Selection_Request_Message(
				global->_1905_ctrl,
				(char *)dev->_1905_info.al_mac_addr,
				ch_prefer_count,
				ch_prefer,
				0,
				NULL);
			eloop_register_timeout(3, 0, ch_planning_timeout_handler,
				ctx,dev);
		} else {
			err("Channel Planning on controller\n");
			struct channel_setting *setting = NULL;
			int i = 0;
			setting = os_malloc(512);

			setting->ch_set_num = ch_prefer_count;
			for (i = 0; i < setting->ch_set_num; i++) {
				max_pref_index = ch_planning_find_max_pref_index(ctx, ch_prefer);
				setting->chinfo[i].channel = ch_prefer[i].opinfo[max_pref_index].ch_list[0];
				setting->chinfo[i].op_class = ch_prefer[i].opinfo[max_pref_index].op_class;
				os_memcpy(setting->chinfo[i].identifier, ch_prefer[i].identifier, ETH_ALEN);
				setting->chinfo[i].reason_code = ch_prefer[i].opinfo[max_pref_index].reason;
				err("Reason code: %d opclass %d channel %d",
					ch_prefer[i].opinfo[max_pref_index].reason,
					ch_prefer[i].opinfo[max_pref_index].op_class,
					ch_prefer[i].opinfo[max_pref_index].ch_list[0]);

				/* Using i+1 index for primary CH info for setting->chinfo */
				if ((i < setting->ch_set_num-1) && ((ch_prefer[i].opinfo[max_pref_index].op_class == 128)
#ifdef MAP_160BW
					|| (ch_prefer[i].opinfo[max_pref_index].op_class == 129)
#endif
				)) {
                     setting->chinfo[i+1].channel = ch_prefer[i].opinfo[max_pref_index+1].ch_list[0];
                                	setting->chinfo[i+1].op_class = ch_prefer[i].opinfo[max_pref_index+1].op_class;
                                	os_memcpy(setting->chinfo[i+1].identifier, ch_prefer[i].identifier, ETH_ALEN);
                                	setting->chinfo[i+1].reason_code = ch_prefer[i].opinfo[max_pref_index+1].reason;
                                	err("Primary CH Reason code: %d opclass %d channel %d",
                                        	ch_prefer[i].opinfo[max_pref_index+1].reason,
                                        	ch_prefer[i].opinfo[max_pref_index+1].op_class,
                                        	ch_prefer[i].opinfo[max_pref_index+1].ch_list[0]);
					i++;
					setting->ch_set_num++;
				}

			}
			wlanif_issue_wapp_command(ctx->back_ptr, WAPP_USER_SET_CHANNEL_SETTING, 0,
				NULL, NULL, setting, 512, 0, 0, 0);
			os_free(setting);

			dev->channel_planning_completed = TRUE;
			ctx->Restart_ch_planning_radar = 0;
			ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
			ctx->ch_planning.current_ch_planning_dev = NULL;
		}

	} else if (best_channel == 0){
		dev->channel_planning_completed = TRUE;
		ctx->Restart_ch_planning_radar = 0;
	}
	os_free(ch_prefer);
#endif
}
#ifdef MAP_R2
void ch_planning_exec_R2(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned int channel[])
{
	struct radio_info_db *radio;
	unsigned char channel_planning_req = FALSE,avoid_ChPlanning_radio = FALSE;
	signed int best_channel = 0;
	unsigned int force_channel = 0;
	unsigned char ch_prefer_count = 0;
	struct ch_prefer_lib *ch_prefer;
	struct mapd_global *global = ctx->back_ptr;
	struct bh_link_entry *bh_entry = NULL;
	struct mapd_radio_info *own_radio = NULL;
	u8 dfs_status = 0;
	ch_prefer = os_zalloc(sizeof(struct ch_prefer_lib) * 3);
	SLIST_FOREACH(radio, &(dev->first_radio), next_radio) {
		if(ctx->ch_planning_R2.ch_plan_enable == TRUE) {
			if (DEVICE_ROLE_CONTROLLER == dev->device_role){
				own_radio = mapd_get_radio_from_channel(global,radio->channel[0]);
				err("channel %d, bootup_run %d", radio->channel[0],own_radio->bootup_run);
				if(own_radio->bootup_run == 0)
					continue;
			}
		err("radio->ch %d, state %d,force_channel %d", radio->channel[0], radio->dev_ch_plan_info.dev_ch_plan_state, force_channel);
			if(radio->dev_ch_plan_info.dev_ch_plan_state != CHPLAN_STATE_CH_CHANGE_TRIGGERED){
				continue;
			}
	}else {
		if(ctx->div_ch_planning == 0 && (best_channel > 0 || force_channel > 0)){
			err("continue as need to switch to this channel first ");
			continue;
		}
	}
	if((DEVICE_ROLE_CONTROLLER == dev->device_role) &&
		(global->dev.ThirdPartyConnection) && (!ctx->Restart_ch_planning_radar)) {
		avoid_ChPlanning_radio = FALSE;
		SLIST_FOREACH(bh_entry, &(ctx->bh_link_head), next_bh_link) {
			if((bh_entry->bh_channel == radio->channel[0] &&
				bh_entry->bh_assoc_state == WAPP_APCLI_ASSOCIATED) ||
				(ctx->current_bh_state != BH_STATE_WIFI_LINKUP && ctx->bh_config_count > 0 )) {
				//! it is a divergent ch planning case and current radio
				//! is a controller radio with CLI connected to
				//! third party AP. channel planning cannot be executed on it
				avoid_ChPlanning_radio = TRUE;
			}
		}
		if(avoid_ChPlanning_radio) {
			err("Avoid Ch planning on this radio");
			continue;
		}
	}
		//! channel planning is executed only on the radio, whose APCLI interface
		//! is not connected to uplink device
		if (!radio->uplink_bh_present){
			//! convergent channel planing case
			if(!(channel[0] || channel[1] || channel[2])){
				//! find best possible channel for the radio
				best_channel = ch_planning_select_best_channel(ctx, radio, dev);
					err("bestchannel %d", best_channel);

				if(best_channel == 0){
					if(ctx->ch_planning_R2.ch_plan_enable == TRUE) {
						ch_planning_handle_ch_selection_rsp(ctx,dev);
					} else {
						ch_planning_trigger_net_opt_post_ch_plan(ctx);
					}
				}
				if(best_channel < 0){
					if(ctx->ch_planning_R2.ch_plan_enable == TRUE) {
						ch_planning_handle_ch_selection_rsp(ctx,dev);
					} else {
						ch_planning_trigger_net_opt_post_ch_plan(ctx);
					}

					os_free(ch_prefer);
					return;
				}
			}
			//! divergent channel planning cases
			else if(channel_validInList(ctx,radio,channel[0])) {
				force_channel = channel[0];
				global->dev.ch_planning.ch_planning_enabled = 0;
			}
			else if(channel_validInList(ctx,radio,channel[1])) {
				force_channel = channel[1];
				global->dev.ch_planning.ch_planning_enabled = 0;
			}
			else if(channel_validInList(ctx,radio,channel[2])) {
				force_channel = channel[2];
				global->dev.ch_planning.ch_planning_enabled = 0;
			}
			err("force ch: %d", force_channel);
			u8 dedicated_radio =0;
			if (best_channel > 0){
				dedicated_radio = ch_planning_check_controller_cac_cap(&global->dev, best_channel, DEDICATED_RADIO);
			} else if (force_channel > 0) {
				dedicated_radio = ch_planning_check_controller_cac_cap(&global->dev, force_channel, DEDICATED_RADIO);
			}
			global->dev.dedicated_radio = dedicated_radio;
			err("dedicated_radio %d", dedicated_radio);
			if((ctx->ch_planning_R2.ch_plan_enable == TRUE || (global->dev.dedicated_radio)) &&
				(best_channel > 14 || force_channel > 14)) {
				/*check if ch is DFS*/
				if(best_channel > 0)
					dfs_status = ch_planning_check_dfs(global,best_channel);
				else if (force_channel > 0)
					dfs_status = ch_planning_check_dfs(global,force_channel);
				if(dfs_status == 1) {
					if (best_channel > 0)
						start_netopt_timer(&global->dev, best_channel);
					else if (force_channel > 0)
						start_netopt_timer(&global->dev, force_channel);
					err("ch selection will be issued after cac complete");
					if (ctx->force_ch_change == 0) {
						os_free(ch_prefer);
						return;
					} else {
						err("dfs cac to be checked, moving to other radio");
						continue;
					}
				} else {
					err("dfs_status %d", dfs_status);
				}
			} else {
				dfs_status = 2;
			}

			if (!global->dev.dedicated_radio && dfs_status == 2) {
				if (best_channel > 14 && ch_planning_is_ch_dfs(&global->dev, best_channel) &&
					(is_CAC_Success(global, best_channel) == 0)) {
						start_netopt_timer(&global->dev, best_channel);
				} else if (force_channel > 14 && ch_planning_is_ch_dfs(&global->dev, force_channel) &&
					(is_CAC_Success(global, force_channel) == 0)) {
						start_netopt_timer(&global->dev, force_channel);
				}
			}

			if (best_channel > 0) {
				//! prepare channel selection request data for current radio
				ch_planning_ch_selection_prefer_data(ctx, best_channel,
					radio, &ch_prefer[ch_prefer_count]);
				ch_prefer_count++;
				channel_planning_req = TRUE;
			}
			if (force_channel > 0) {
				//! prepare channel selection request data for current radio
				ch_planning_ch_selection_prefer_data(ctx, force_channel,
					radio, &ch_prefer[ch_prefer_count]);
				ch_prefer_count++;
				channel_planning_req = TRUE;
			}
		}
		else {
			err("UPLINK BH present , ch update to happen via CSA, clean state");
			if(ctx->ch_planning_R2.ch_plan_enable == TRUE) {
				ch_planning_handle_ch_selection_rsp(ctx,dev);
			} else {
				ch_planning_trigger_net_opt_post_ch_plan(ctx);
			}
		}
	}//radio loop ends here
	err("channel_planning_req %d, best_channel %d,force_channel %d ", channel_planning_req, best_channel,force_channel );
	if (channel_planning_req)
	{
		ctx->ch_planning_R2.ch_plan_state = CHPLAN_STATE_CH_CHANGE_TRIGGERED;
		err("Send Channel Selection Request\n");
		ctx->ch_planning.ch_planning_state =
			CHANNEL_SELECTION_REQ_SENT;
		ctx->ch_planning.current_ch_planning_dev = dev;
		if(dfs_status == 2)
		{
			err("since it is not DFS ch so jump directly");
			ch_planning_send_select(ctx, dev, ch_prefer_count, ch_prefer);
			if(global->dev.force_ch_change) {
				global->dev.ch_planning.ch_planning_enabled = 0;
				global->dev.force_ch_change = 0;
			}
		} else if(dfs_status == 0) {
			err("send select dfs");
			if(best_channel > 0) {
				ch_planning_R2_send_select_dfs(ctx, dev, ch_prefer_count, ch_prefer, best_channel);
				if (!(is_CAC_Success(global, best_channel)))
					start_netopt_timer(&global->dev, best_channel);
			} else if(force_channel > 0) {
				err(" global->dev.force_ch_change %d", global->dev.force_ch_change);
				ch_planning_R2_send_select_dfs(ctx, dev, ch_prefer_count, ch_prefer, force_channel);
				if(global->dev.force_ch_change) {
					global->dev.ch_planning.ch_planning_enabled = 0;
					global->dev.force_ch_change = 0;
				}
			}
		}
	} else if (best_channel == 0){
		dev->channel_planning_completed = TRUE;
		ctx->Restart_ch_planning_radar = 0;
		if(ctx->ch_planning_R2.ch_plan_enable == FALSE) {
			ch_planning_trigger_net_opt_post_ch_plan(ctx);
		}
	}
	os_free(ch_prefer);
}
#endif

void ch_planning_show_ch_distribution(
	struct own_1905_device *ctx)
{
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct operating_ch_cb *operating_ch = NULL;
	struct prefered_ch_cb *prefered_ch = NULL;
	struct prefered_ch_radio_info_db *prefered_ch_radio = NULL;
	struct radio_info_db *radio = NULL;

	always("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
	always("Operating Channel distribution on 5G\n");
	ch_distribution = &ch_planning->ch_ditribution_5g;
	always("\tNumber of Channels = %d\n",
		ch_distribution->operating_ch_count);
	SLIST_FOREACH(operating_ch,
		&(ch_distribution->first_operating_ch),
		next_operating_ch) {
		always("\t\tChannel Number = %d\n", operating_ch->ch_num);
		always("\t\tNumber of Radios = %d\n", operating_ch->radio_count);
		SLIST_FOREACH(radio,
			&(operating_ch->first_radio),
			next_co_ch_radio) {
			always("\t\t\tDev ALMAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(radio->parent_1905->_1905_info.al_mac_addr));
			always("\t\t\tRadio ID = %02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(radio->identifier));
		}
	}
	always("Operating Channel distribution on 2G\n");
	ch_distribution = &ch_planning->ch_ditribution_2g;
	always("\tNumber of Channels = %d\n",
		ch_distribution->operating_ch_count);
	SLIST_FOREACH(operating_ch,
		&(ch_distribution->first_operating_ch),
		next_operating_ch) {
		always("\t\tChannel Number = %d\n", operating_ch->ch_num);
		always("\t\tNumber of Radios = %d\n", operating_ch->radio_count);
		SLIST_FOREACH(radio,
			&(operating_ch->first_radio),
			next_co_ch_radio) {
			always("\t\t\tDev ALMAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(radio->parent_1905->_1905_info.al_mac_addr));
			always("\t\t\tRadio ID = %02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(radio->identifier));
		}
	}

	always("Prefered Channel distribution on 5G\n");
	ch_distribution = &ch_planning->ch_ditribution_5g;
	always("\tNumber of Channels = %d\n",
		ch_distribution->prefered_ch_count);
	SLIST_FOREACH(prefered_ch,
		&(ch_distribution->first_prefered_ch),
		next_prefered_ch) {
		always("\t\tChannel Number = %d\n", prefered_ch->ch_num);
		always("\t\tchannel Score = %x\n", prefered_ch->ch_score);
		SLIST_FOREACH(prefered_ch_radio,
			&(prefered_ch->first_radio),
			next_pref_ch_radio) {
			always("\t\t\tDev ALMAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(prefered_ch_radio->radio->parent_1905->_1905_info.al_mac_addr));
		}
	}

	always("Prefered Channel distribution on 2G\n");
	ch_distribution = &ch_planning->ch_ditribution_2g;
	always("\tNumber of Channels = %d\n",
		ch_distribution->prefered_ch_count);
	SLIST_FOREACH(prefered_ch,
		&(ch_distribution->first_prefered_ch),
		next_prefered_ch) {
		always("\t\tChannel Number = %d\n", prefered_ch->ch_num);
		always("\t\tchannel Score = %x\n", prefered_ch->ch_score);
		SLIST_FOREACH(prefered_ch_radio,
			&(prefered_ch->first_radio),
			next_pref_ch_radio) {
			always("\t\t\tDev ALMAC = %02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(prefered_ch_radio->radio->parent_1905->_1905_info.al_mac_addr));
		}
	}
	always(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
}
/**
* @brief Fn to operating channel distribution
* @param ctx own 1905 global
* @param dev 1905 map deice
* @param radio target radio
* @param channel*/

void ch_planning_update_ch_ditribution(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	struct radio_info_db *radio,
	unsigned char channel,
 	unsigned char op_class
)
{
	struct ch_planning_cb *ch_planning = &ctx->ch_planning;
	struct ch_distribution_cb *ch_distribution = NULL;
	struct operating_ch_cb *operating_ch;


	if (ctx->device_role != DEVICE_ROLE_CONTROLLER)
	{
		return;
	}
	if (radio->channel[0] > 14)
	{
		//err("ch_ditribution_5g\n");
		ch_distribution = &ch_planning->ch_ditribution_5g;
	} else {
		//err("ch_ditribution_2g\n");
		ch_distribution = &ch_planning->ch_ditribution_2g;
	}

	/*dump channel distribution to see if radio still exist in operating list, when radio->operating_channel = NULL*/
	if (radio->operating_channel == NULL) {
		err("radio_ptr:%p operating channel: NULL, dump Channel Distribution\n", radio);
		ch_planning_show_ch_distribution(ctx);
	}

	if (radio->operating_channel != NULL) {
		//err("Unique channel was previousy allocated to device\n");
		operating_ch = radio->operating_channel;
		if (operating_ch->ch_num == channel)
		{
			//err("already on previously announced channel\n");
			return;
		} else {
			err("remove radio from previous unique channel\n");
			operating_ch->radio_count--;
			//ch_planning_update_ch_score(ctx, ch_distribution, operating_ch->ch_num);
			SLIST_REMOVE(&operating_ch->first_radio,
			radio,
			radio_info_db,
			next_co_ch_radio);
			if (operating_ch->radio_count == 0)
			{
				err("No more radios in current unique list\n");
				err("remove from channel distribution\n");
				SLIST_REMOVE(&ch_distribution->first_operating_ch,
				operating_ch,
				operating_ch_cb,
				next_operating_ch);
				os_free(operating_ch);
				ch_distribution->operating_ch_count--;
			}
		}
	}
	operating_ch = NULL;
	SLIST_FOREACH(operating_ch,
		&(ch_distribution->first_operating_ch),
		next_operating_ch) {
		if (operating_ch->ch_num == channel)
		{
			err("A unique channel exist for current channel req\n");
			SLIST_INSERT_HEAD(
				&(operating_ch->first_radio),
				radio,
				next_co_ch_radio);
			radio->operating_channel = operating_ch;
			radio->operating_class = op_class;
			operating_ch->radio_count++;
			//ch_planning_update_ch_score(ctx, ch_distribution, operating_ch->ch_num);
			SLIST_REMOVE(&(ch_distribution->first_operating_ch),
			operating_ch,
			operating_ch_cb,
			next_operating_ch);
			ch_planning_insert_into_ch_operating(ctx,
				operating_ch,
				ch_distribution);
			break;
		}
	}
	if (operating_ch == NULL)
	{
		err("allocate a new unique channel, channel NUM = %d, opclass %d\n", channel, op_class);
		operating_ch = os_zalloc(sizeof(struct operating_ch_cb));
		operating_ch->ch_num = channel;
		SLIST_INIT(&(operating_ch->first_radio));
		SLIST_INSERT_HEAD(
			&(operating_ch->first_radio),
			radio,
			next_co_ch_radio);
		operating_ch->radio_count++;
		radio->operating_channel = operating_ch;
		radio->operating_class = op_class;
		if (ch_distribution->operating_ch_count == 0)
		{
			SLIST_INIT(&(ch_distribution->first_operating_ch));
		}
		ch_planning_insert_into_ch_operating(ctx,
			operating_ch,
			ch_distribution);
		ch_distribution->operating_ch_count++;
		//ch_planning_update_ch_score(ctx, ch_distribution, operating_ch->ch_num);
	}
	//ch_planning_show_ch_distribution(ctx);
}
/**
* @brief Fn to handle operating channel report
* @param ctx own 1905 global
* @param dev target 1905 map device
* @param buff
*/

unsigned char * ch_planning_handle_oper_ch_tlv(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char *buff)
{
	unsigned char * radio_id = buff;
	unsigned char *curr_pointer = buff +ETH_ALEN;
	unsigned char i = 0;
	unsigned char num_oper_class = *curr_pointer;
	unsigned char oper_class = 0;
	unsigned char oper_channel = 0;
	struct radio_info_db *radio =
		topo_srv_get_radio(dev, radio_id);

#if 0
	mapd_hexdump(MSG_ERROR, "ch_planning_handle_operating_channel_report",
		buff, 32);
#endif
	if (radio == NULL)
	{
		radio = os_zalloc(sizeof(*radio));
		if (!radio) {
			err("mem allocation failed");
			return NULL;
		}
		os_memcpy(radio->identifier, radio_id, ETH_ALEN);
		radio->parent_1905 = dev;
		err("%s, create new radio %p, parent device = %p\n",
			__FUNCTION__, radio, radio->parent_1905);
		SLIST_INSERT_HEAD(&(dev->first_radio), radio, next_radio);
		SLIST_INIT(&(radio->link_estimate_cb_head));
		SLIST_INIT(&radio->chan_preferance.prefer_info_head);
		SLIST_INIT(&radio->chan_restrict.restrict_head);
#ifdef MAP_R2
		SLIST_INIT(&radio->first_scan_result);
		SLIST_INIT(&radio->cac_cap.cac_capab_head);
		SLIST_INIT(&radio->cac_comp_status.cac_completion_opcap_head);
#endif
	}
	curr_pointer++;
	for (i = 0; i < num_oper_class; i++)
	{
		oper_class = *curr_pointer;
		curr_pointer++;
		oper_channel = *curr_pointer;
		curr_pointer++;
		err("Channel = %d, OperClass = %d\n", oper_channel, oper_class);
		radio->operating_class = oper_class;
		os_memset(radio->channel, 0,sizeof(radio->channel));
		radio->channel[0] = oper_channel;
		/* getting Primary CH for 80MHz BW */
        if (((oper_class == 128)
#ifdef MAP_160BW
			|| (oper_class == 129)
#endif
		) && (i < (num_oper_class-1))) {
			unsigned char next_channel;

			next_channel = *(curr_pointer + 1);
			if (is_valid_primary_ch_80M_160M(next_channel, oper_channel, oper_class)) {
				radio->channel[0] = next_channel;
				err("Primary Ch %d, op: %u",radio->channel[0], oper_class);
				curr_pointer+=2;
			}
		}
		mapd_fill_secondary_channels(radio->channel, oper_class, 0);
		radio->band = get_band_from_channel(radio->channel[0]);
		ch_planning_update_ch_ditribution(ctx, dev,
			radio, oper_channel, oper_class);
	}
	return curr_pointer;
}
void ch_planning_handle_operating_channel_report(
	struct own_1905_device *ctx,
	unsigned char *buff,
	int len)
{
	unsigned char *peer_al_mac = buff;
	unsigned char *curr_pointer = buff + ETH_ALEN;
	struct _1905_map_device *dev = topo_srv_get_1905_device(
		ctx, peer_al_mac);

	while (dev)
	{
		if (*curr_pointer == OPERATING_CHANNEL_REPORT_TYPE)
		{
			curr_pointer += 1;//increment a byte for TLV type
			curr_pointer += 2;//! we do not need the legth for now.
			curr_pointer = ch_planning_handle_oper_ch_tlv(ctx, dev, curr_pointer);
			if (curr_pointer == NULL)
				return;
			curr_pointer += 1; //! we are not handling TX power
		} else {
			break;
		}
	}

	if (ctx->ch_planning.current_ch_planning_dev == dev)
	{
		err("Operating Channel Report Received, planning completed\n");
		ctx->ch_planning.current_ch_planning_dev = NULL;
		ctx->ch_planning.ch_planning_state = CHANNEL_PLANNING_IDLE;
#ifdef MAP_R2
		if(ctx->ch_planning_R2.ch_plan_enable == TRUE &&
			ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_CH_CHANGE_TRIGGERED) {
			err("operating ch report has come ch switch complete for"MACSTR"", MAC2STR(dev->_1905_info.al_mac_addr));
			ch_planning_handle_ch_selection_rsp(ctx,dev);
		}else {
			ch_planning_trigger_net_opt_post_ch_plan(ctx);
		}
#endif
	}
}

/**
* @brief Fn periodic function for channel planning
* @param ctx own 1905 global
*/

void mapd_perform_channel_planning(struct own_1905_device *ctx)
{
	struct _1905_map_device *dev = NULL;
	unsigned int channel[3]={0,0,0};
#ifdef MAP_R2
	/*Do not restart channel planning based on 30 min idle time if R2 ch planning is enabled*/
	if(ctx->ch_planning_R2.ch_plan_enable == FALSE) {
		if(ch_planning_is_MAP_net_idle(ctx)){
			mapd_restart_channel_plannig(ctx->back_ptr);
			struct ch_planning_cb *p_ch_planning = &ctx->ch_planning;
			os_get_time(&p_ch_planning->last_high_byte_count_ts);
		}
	}
#else
	struct os_time now;
	struct ch_planning_cb *p_ch_planning = &ctx->ch_planning;
	if (p_ch_planning->last_high_byte_count_ts.sec == 0)
	{
		os_get_time(&p_ch_planning->last_high_byte_count_ts);
	}
	os_get_time(&now);

	if (now.sec - p_ch_planning->last_high_byte_count_ts.sec >
		p_ch_planning->ChPlanningIdleTime)
	{
		err("due to CH planning idle timeout restart ch planning");
		mapd_restart_channel_plannig(ctx->back_ptr);
		os_get_time(&p_ch_planning->last_high_byte_count_ts);
	}
#endif
#ifdef MAP_R2
	if (ctx->device_role == DEVICE_ROLE_CONTROLLER &&
		(ctx->ch_planning_R2.ch_plan_enable == TRUE)){
		struct os_time current_time;
		os_get_time(&current_time);
		dev = topo_srv_get_1905_device(ctx,NULL);//controller bootup
		if((current_time.sec - dev->first_seen.sec)
			> ctx->channel_planning_initial_timeout){
			ch_planning_R2_bootup_handling(ctx);
		}
	}
#endif
	if ((ctx->device_role == DEVICE_ROLE_CONTROLLER) &&
		(ctx->ch_planning.current_ch_planning_dev == NULL) &&
		(ctx->ch_planning.ch_planning_state == CHANNEL_PLANNING_IDLE))
	{
		//while(1)
		{
			dev = ch_planning_get_target_dev(ctx);
			if (dev != NULL)
			{
				err("err Need perform channel planning for %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(dev->_1905_info.al_mac_addr));
#ifdef MAP_R2
/*To Prevent the old channel planning logic to run simultaneously when
ch planing R2 algo is running*/
				if((ctx->ch_planning_R2.ch_plan_enable == TRUE) &&
					(ctx->ch_planning_R2.ch_plan_state != CHPLAN_STATE_IDLE &&
					ctx->ch_planning_R2.ch_plan_state != CHPLAN_STATE_CH_CHANGE_TRIGGERED)) {
					debug("Ch planning R2 ongoing, avoid R1 right now");
					return;
				}
				if (ctx->div_ch_planning == 1) {
					ctx->ch_planning.ch_planning_enabled = 0;
					if (ctx->ch_planning_R2.ch_plan_state == CHPLAN_STATE_CH_CHANGE_TRIGGERED && ctx->ch_planning_R2.CAC_on_channel) {
						channel[0] = ctx->ch_planning_R2.CAC_on_channel;
						ctx->force_ch_change = 1;
					}
				}
#endif
				ch_planning_exec(ctx, dev, channel);
			} else {
				//err("no dev for planning\n");
			}
		}
	}
}

#ifdef MAP_R2
void ch_planning_reset_user_preff_ch(struct mapd_global *global)
{
	struct own_1905_device *ctx = &global->dev;
	struct ch_distribution_cb *ch_distribution;
	ch_distribution = &ctx->ch_planning.ch_ditribution_5g;
	ch_distribution->user_prefered_ch_HighBand = 0;
	ch_distribution->user_prefered_ch = 0;
	ch_distribution = &ctx->ch_planning.ch_ditribution_2g;
	ch_distribution->user_prefered_ch_HighBand = 0;
	ch_distribution->user_prefered_ch = 0;
}
#endif
int ch_planning_set_user_preff_ch(struct mapd_global *global,
	u8 channel)
{
	struct own_1905_device *ctx = &global->dev;
	struct _1905_map_device *dev = topo_srv_get_next_1905_device(ctx, NULL);
	struct ch_distribution_cb *ch_distribution;
	if (channel > 14)
		ch_distribution = &ctx->ch_planning.ch_ditribution_5g;
	else
		ch_distribution = &ctx->ch_planning.ch_ditribution_2g;
	if(check_is_triband(dev)&&(isChan5GH(channel))) {
		ch_distribution->user_prefered_ch_HighBand = channel;
	} else {
		ch_distribution->user_prefered_ch = channel;
		//ch_planning_update_ch_score(ctx,ch_distribution, prev_prefered_ch);
	}

	//ch_planning_update_ch_score(ctx, ch_distribution, channel);
	err(" channel %d", channel);
	mapd_restart_channel_plannig(global);
	return 0;
}

/**
* @brief Fn to set txpower percentage for a given MAP device
* @param ctx own 1905 global
* @param dev target dev
* @param unsigned char band
* @param unsigned char txpower_percentage
*/
void ch_planning_set_txpower_percentage(
	struct own_1905_device *ctx,
	struct _1905_map_device *dev,
	unsigned char band,
	unsigned char txpower_percentage)
{
	struct _1905_map_device *own_1905_node = topo_srv_get_next_1905_device(ctx, NULL);
	struct tx_power_percentage_setting *tx_power_setting = NULL;

	err("Set Tx Power percentage\n");

	if(txpower_percentage > 100) {
		err("Invalid txpower_percentage setting %d\n", txpower_percentage);
		return;
	}
	if(!((band == BAND_24G) || (band == BAND_5G))) {
		err("Invalid band setting %d!!\n", band);
		return;
	}
	if (own_1905_node != dev) {
		err("Set Tx Power percentage on agent\n");
		ch_planning_send_txpower_percentage_msg(ctx, dev->_1905_info.al_mac_addr, band, txpower_percentage);
	} else {
		err("Set Tx Power percentage on controller\n");
		tx_power_setting = os_zalloc(sizeof(struct tx_power_percentage_setting));
		if(tx_power_setting == NULL) {
			err("tx_power setting:Memory allocation failed\n");
			return;
		}
		tx_power_setting->bandIdx = band;
		tx_power_setting->tx_power_percentage = txpower_percentage;

		wlanif_issue_wapp_command(ctx->back_ptr, WAPP_USER_SET_TX_POWER_PERCENTAGE, 0,
			NULL, NULL, tx_power_setting, sizeof(struct tx_power_percentage_setting), 0, 0, 0);
		os_free(tx_power_setting);
	}

	return;
}

/**
* @brief Fn to send txpower percentage for a given MAP device
* @param ctx own 1905 global
* @param unsigned char *al_mac_addr
* @param unsigned char bandIdx
* @param unsigned char txpower_percentage
*/
void ch_planning_send_txpower_percentage_msg(
	struct own_1905_device *ctx,
	unsigned char *al_mac_addr,
	unsigned char bandIdx,
	unsigned char txpower_percentage)
{
	struct mapd_global *mapd_ctx = (struct mapd_global *)ctx->back_ptr;
	struct tx_power_percentage_tlv *txpower_percent_tlv = NULL;

	if (txpower_percentage > 100){
		err("Invalid txpower_percentage setting %d\n",txpower_percentage);
		return;
	}
	if(!((bandIdx == BAND_24G) || (bandIdx == BAND_5G))) {
		err("Invalid band setting %d!!\n", bandIdx);
		return;
	}
	debug("sending vendor tx power percentage tlv");
	txpower_percent_tlv = os_zalloc(sizeof(struct tx_power_percentage_tlv));

	if(txpower_percent_tlv == NULL) {
		err("txpower_percent_tlv is NULL\n");
		return;
	}

	txpower_percent_tlv->tlv_type = TLV_802_11_VENDOR_SPECIFIC;
	txpower_percent_tlv->tlv_len = host_to_be16(TX_POWER_PERCENTAGE_TLV_LEN);
	os_memcpy(txpower_percent_tlv->mtk_oui, MTK_OUI, OUI_LEN);
	txpower_percent_tlv->func_type = FUNC_VENDOR_SET_TX_POWER_PERCENTAGE;
	txpower_percent_tlv->bandIdx = bandIdx;
	txpower_percent_tlv->tx_power_percentage = txpower_percentage;

	map_1905_Send_Vendor_Specific_Message(mapd_ctx->_1905_ctrl, (char *)al_mac_addr,
						(char *)txpower_percent_tlv, sizeof(struct tx_power_percentage_tlv));
	os_free(txpower_percent_tlv);

	return;
}

/**
* @brief Fn to handle txpower percentage from the controller
* @param ctx own 1905 global
* @param tx_power_percentage_tlv *txpower_percent_tlv
*/
void ch_planning_handle_tx_power_percentage_msg(
	struct mapd_global *pGlobal_dev,
	struct tx_power_percentage_tlv *txpower_percent_tlv)
{
	struct tx_power_percentage_setting *tx_power_setting = NULL;
	struct own_1905_device *ctx = &pGlobal_dev->dev;

	if(ctx == NULL) {
		mapd_printf(MSG_ERROR,"own dev ctx is NULL");
		return;
	}

	if(txpower_percent_tlv == NULL) {
		mapd_printf(MSG_ERROR,"txpower_percent_tlv is null");
		return;
	}

	tx_power_setting = os_zalloc(sizeof(struct tx_power_percentage_setting));
	if(tx_power_setting == NULL) {
		mapd_printf(MSG_ERROR,"tx_power setting:Memory allocation failed\n");
		return;
	}

	tx_power_setting->bandIdx = txpower_percent_tlv->bandIdx;
	tx_power_setting->tx_power_percentage = txpower_percent_tlv->tx_power_percentage;
	always("%s:tx_power_setting->bandIdx = %d, tx_power_setting->tx_power_percentage = %d\n",__func__, tx_power_setting->bandIdx, tx_power_setting->tx_power_percentage);
	wlanif_issue_wapp_command(ctx->back_ptr, WAPP_USER_SET_TX_POWER_PERCENTAGE, 0,
		NULL, NULL, tx_power_setting, sizeof(struct tx_power_percentage_setting), 0, 0, 0);
	os_free(tx_power_setting);

	return;
}
int off_ch_scan_exec(struct own_1905_device *ctx,
					char *buf,
					unsigned char *reply, unsigned char bandwidth)
{
	unsigned char almac[6]= {0};
	int i = 0;
	unsigned char band = 0;
	unsigned char mode = 0;
	unsigned char bw = 0;
	unsigned char scan_ch_list[MAX_OFF_CH_SCAN_CH] = {0};
	struct _1905_map_device *target_1905=NULL;
	int reply_len=1;
	char * ptmp = NULL;
	char * pvalue = NULL;
	//! ptmp points to comand string
	ptmp = strtok_r(buf, " ", &buf);
	//! ptmp points to ALMAC
	ptmp = strtok_r(buf, " ", &buf);
	pvalue = strtok_r(ptmp, ":", &ptmp);
	for(i = 0;(pvalue && (i< ETH_ALEN)); i++, pvalue = strtok_r(ptmp, ":", &ptmp)) {
		almac[i] = strtol(pvalue, &pvalue, 0x10);
	}
	bw = bandwidth;
	ptmp = strtok_r(buf, " ", &buf);
	mode= strtol(ptmp, &ptmp, 0x10);
	if (mode == SCAN_MODE_BAND) {
		//! ptmp points to band
		ptmp = strtok_r(buf, " ", &buf);
		if(!ptmp)  {
			err("miss the specified band\n");
			return -1;
		}
		band = strtol(ptmp, &ptmp, 0x10);
	} else if (mode == SCAN_MODE_CH){
		ptmp = strtok_r(buf, " ", &buf);
		i = 0;
		while (ptmp && i < sizeof(scan_ch_list)) {
			scan_ch_list[i] = strtol(ptmp, &ptmp, 10);
			ptmp = strtok_r(buf, " ", &buf);
			i++;
		}
	}
	target_1905 = topo_srv_get_1905_device(ctx, almac);
	if (!target_1905) {
		err("device with given almac not found\n");
		reply_len=-1;
	} else if((!target_1905->in_network)) {
		err("device is not connected\n");
		reply_len=-1;
	} else {
		os_memcpy(reply, "OK\n", 3);
		reply_len = 3;
	}

	if (target_1905) {
		if (target_1905->off_ch_scan_report) {
			os_free(target_1905->off_ch_scan_report);
			target_1905->off_ch_scan_report = NULL;
		}
		send_off_ch_scan_req(ctx->back_ptr, target_1905, mode, band, scan_ch_list, bw, 1);
	}
	return reply_len;
}

void mapd_fill_secondary_channels(unsigned char *channel,
	unsigned char op_class, unsigned char bw)
{
	int i = 0;
	int j = 1;
	unsigned char centre_freq = ch_planning_get_centre_freq_ch(channel[0], op_class);

	if ((op_class > 127) || (bw == 2)) {
		for (i = 0; i < 4; i++) {
			if (channel[0] == ((centre_freq - 6) + (i*4))) {
				continue;
			} else {
				channel[j] = ((centre_freq - 6) + (i*4));
				j++;
			}
		}
	}
}
void mapd_fill_secondary_channels_for_1905_dev(struct own_1905_device *ctx,
	struct _1905_map_device *dev)
{
	struct radio_info_db *radio = topo_srv_get_next_radio(
		dev, NULL);
	struct bss_info_db *bss_info;
	struct iface_info *iface;
	while (radio) {
		bss_info = NULL;
		iface = NULL;
		bss_info = topo_srv_get_next_bss(dev, NULL);
		while (bss_info) {
			if (bss_info->radio == radio) {
				break;
			}
			bss_info = topo_srv_get_next_bss(dev, bss_info);
		}

		if (bss_info) {
			iface = topo_srv_get_next_iface(dev, NULL);
			while (iface) {
				if (!os_memcmp(iface->iface_addr, bss_info->bssid,
					ETH_ALEN)) {
					break;
				}
				iface = topo_srv_get_next_iface(dev, iface);
			}
		}
		if (iface) {
			os_memset(radio->channel, 0,
				sizeof(radio->channel));
			radio->channel[0] = iface->channel_freq_idx;
			radio->operating_class =
				chan_mon_get_op_class_frm_channel(iface->channel_freq_idx,
				iface->media_info.ap_channel_band);
			mapd_fill_secondary_channels(radio->channel, 0,
				iface->media_info.ap_channel_band);
			radio->band = get_band_from_channel(radio->channel[0]);
		}
		radio = topo_srv_get_next_radio(dev, radio);
	}
}

void start_netopt_timer(struct own_1905_device *own_dev,u8 channel)
{
	u8 i;
	u8 count = 0;
	for (i = 0; i < 16; i++) {
		debug("ch %d pref 0x%x",own_dev->dfs_info_ch_list[i].channel,
			own_dev->dfs_info_ch_list[i].pref);
		if(channel == own_dev->dfs_info_ch_list[i].channel) {
			if(own_dev->dfs_info_ch_list[i].pref &(OP_DISALLOWED_DUE_TO_DFS)) {
				count = get_net_opt_dev_count((struct mapd_global *)own_dev->back_ptr);
				if (count > 1) {
					if (eloop_is_timeout_registered(trigger_net_opt,(void *)own_dev , NULL))
						eloop_cancel_timeout(trigger_net_opt, own_dev, NULL);
					eloop_register_timeout((own_dev->network_optimization.post_cac_trigger_time + own_dev->dfs_info_ch_list[i].cac_timer),
							0, trigger_net_opt, own_dev, NULL);
					err("Channel is DFS, netopt timer : %d", (own_dev->network_optimization.post_cac_trigger_time + own_dev->dfs_info_ch_list[i].cac_timer));
				}
			}
		}
	}
}

Boolean ch_planning_is_ch_dfs(
	struct own_1905_device *own_dev,u8 channel)
{
	u8 i;
	for (i = 0; i < 16; i++) {
		debug("ch %d pref 0x%x",own_dev->dfs_info_ch_list[i].channel,
			own_dev->dfs_info_ch_list[i].pref);
		if(channel == own_dev->dfs_info_ch_list[i].channel) {
			if(own_dev->dfs_info_ch_list[i].pref &(OP_DISALLOWED_DUE_TO_DFS)) {
				return TRUE;
			}
		}
	}
	return FALSE;
}

#ifdef MAP_R2
void ch_scan_req_timeout(void * eloop_ctx, void *user_ctx)
{
	struct mapd_global *ctx = eloop_ctx;
	struct own_1905_device *own_dev = &ctx->dev;

	if(own_dev->user_triggered_scan == 1) {
		own_dev->user_triggered_scan = 0;
		handle_task_completion(own_dev);
	}
}
#endif
