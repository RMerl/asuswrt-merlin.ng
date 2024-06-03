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
                Network Optmization

                Abstract:
                Network Optmization in MAP network

                Revision History:
                Who         When          What
                --------    ----------    -----------------------------------------
                Avishad.V   2018/05/02		First implementation of the Netowork Optimization feature
*/
#include "includes.h"
#include "common.h"
#include "steer_fsm.h"
#include "list.h"
#include "client_db.h"
#include "mapd_i.h"
#include "db.h"
#include "./../1905_local_lib/data_def.h"
#include "chan_mon.h"
#include "./../1905_local_lib/data_def.h"
#include "topologySrv.h"
#include "client_mon.h"
	//#include <sys/queue.h>
#include "eloop.h"
#include "steer_action.h"
#include "ap_est.h"
#include "eloop.h"
#include "wapp_if.h"
#include <assert.h>
#include <sys/un.h>
#include "1905_map_interface.h"
#include "network_optimization.h"
#ifdef WIFI_MD_COEX_SUPPORT
#include "ch_planning.h"
#endif


/*set / reset Network optimziation */
void update_ntwrk_opt_in_dat_file(unsigned char value)
{
	char cmd[200];
	memset(cmd,0,sizeof(cmd));

#ifndef CONFIG_SUPPORT_OPENWRT
	os_snprintf(cmd, sizeof(cmd), "nvram_set 2860 NetworkOptimizationEnabled %d &",value);
	system((const char *)cmd);
#else
	os_snprintf(cmd, sizeof(cmd), "wificonf -f %s set NetworkOptimizationEnabled %d",
		g_map_cfg_path, value);
	system((const char *)cmd);
#endif
}

unsigned char is_all_dev_state_done(struct own_1905_device *dev)
{
	struct _1905_map_device *_1905_dev = NULL;
	unsigned char state = REALIZATION_DONE; //if all are complete
	struct os_time now;
	os_get_time(&now);
	SLIST_FOREACH(_1905_dev, &dev->_1905_dev_head, next_1905_device) {
		err("ALMAC "MACSTR" network_opt_device_state %d ",MAC2STR(_1905_dev->_1905_info.al_mac_addr),_1905_dev->network_opt_per1905.network_opt_device_state);
		if (!(_1905_dev->in_network) &&
			!(_1905_dev->network_opt_per1905.network_opt_device_state == NETOPT_STATE_OPT_NET_REALIZATION_ONGOING)) {
			_1905_dev->network_opt_per1905.network_opt_device_state = NETOPT_STATE_COMPLETE;
				continue;
		}
		if((_1905_dev->network_opt_per1905.network_opt_device_state == NETOPT_STATE_OPT_NET_REALIZATION_ONGOING) ||
			(_1905_dev->network_opt_per1905.network_opt_device_state == NETOPT_STATE_OPT_NET_BHSTEER_DONE))
		{	
			if (now.sec > (_1905_dev->network_opt_per1905.bh_steer_start_ts.sec + 
						dev->network_optimization.bh_steer_wait_time)) {
				err("BHSteer timeout, mark complete");
				_1905_dev->network_opt_per1905.network_opt_device_state = NETOPT_STATE_COMPLETE;
				continue;
			} else {
				state = REALIZATION_ONGOING;
				break;
			}
		} else if (_1905_dev->network_opt_per1905.network_opt_device_state == NETOPT_STATE_OPT_NET_NEED_TO_REALIZE){
			err("Trigger new realization");
			state = TRIGGER_NEW_REALIZATION;
		}
	}
	return state;
}
struct _1905_map_device * find_est_dev_least_hop_cnt(struct own_1905_device *dev)
{
	struct _1905_map_device *least_hop_1905 = NULL;
	struct optimized_network *opt_1905_device = NULL;

	if(SLIST_EMPTY(&(dev->network_optimization.first_opt_net_dev)))
	{ 
		err("opt network list is empty");
		network_opt_reset(dev->back_ptr);
		dev->network_optimization.NetOptReason = REASON_TRY_AGAIN;
#ifdef MAP_R2
		handle_task_completion(dev);
#endif
		return NULL;
	}
	SLIST_FOREACH( opt_1905_device,&dev->network_optimization.first_opt_net_dev, next_opt_net_dev)
	{
		if(opt_1905_device->opt_net_dev->network_opt_per1905.network_opt_device_state == NETOPT_STATE_OPT_NET_NEED_TO_REALIZE)
		{	
			info("least hop count dev init");
			least_hop_1905 = opt_1905_device->opt_net_dev;
			break;
		}
	}

	if (!least_hop_1905)
		return NULL;

	SLIST_FOREACH( opt_1905_device,&dev->network_optimization.first_opt_net_dev, next_opt_net_dev)
	{
		if(opt_1905_device->opt_net_dev->network_opt_per1905.est_hop_count < least_hop_1905->network_opt_per1905.est_hop_count)
		{ 
			if(opt_1905_device->opt_net_dev->network_opt_per1905.network_opt_device_state == NETOPT_STATE_OPT_NET_NEED_TO_REALIZE)
			{ 
				least_hop_1905 = opt_1905_device->opt_net_dev;
			}
		}	
	}

	return least_hop_1905;
}
unsigned char is_eth_BH(struct _1905_map_device *_1905_device)
{
	unsigned char is_eth = 0;
	struct backhaul_link_info *bh_entry = NULL;
	struct map_neighbor_info *neighbor = NULL;
	struct iface_info *iface = NULL;
	if (_1905_device->upstream_device == NULL)
		return 0;
	SLIST_FOREACH(neighbor, &_1905_device->neighbors_entry, next_neighbor) {
		if(os_memcmp(neighbor->n_almac, _1905_device->upstream_device->_1905_info.al_mac_addr,ETH_ALEN) ==0)
		{
			break;
		}
	}
	if (neighbor) {	
	SLIST_FOREACH(bh_entry, &neighbor->bh_head, next_bh) {
		if(bh_entry) {
			iface = topo_srv_get_iface(_1905_device , bh_entry->connected_iface_addr);
			if(!iface)
				return -1;
			if(iface->media_type == ieee_802_3_ab || iface->media_type == ieee_802_3_u) {
				is_eth = 1;
				break;
			}
		}
	}
		}
return is_eth;
}
void Optimized_network_realization(struct mapd_global *global)
{
	struct own_1905_device *dev = &global->dev;
	struct _1905_map_device *_1905_dev = NULL, *target_1905 = NULL;
	struct bss_info_db *target_bss = NULL;
	struct radio_info_db *max_radio = NULL;
	struct N_O_link_estimate_cb *max_link = NULL;
	struct iface_info *iface = NULL;
//find the first device on which BH steer needs to be triggered
	while (1) {
		_1905_dev = find_est_dev_least_hop_cnt(dev);
		if (_1905_dev == NULL) {
			err("1905 dev is NULL return and move to netopt idle state");
			return;
		}
		if (_1905_dev->device_role == DEVICE_ROLE_CONTROLLER)
		{	
			_1905_dev->network_opt_per1905.network_opt_device_state = NETOPT_STATE_COMPLETE;
			err("controller state update as complete");
			continue;
		}
		if (!_1905_dev->in_network) {
			_1905_dev->network_opt_per1905.network_opt_device_state = NETOPT_STATE_COMPLETE;
			err("device is not in network");
			continue;
		}
		if (is_eth_BH(_1905_dev)) {
			_1905_dev->network_opt_per1905.network_opt_device_state = NETOPT_STATE_COMPLETE;
			err("ethernet BH device mark state as complete");
			continue;
		}
		if (_1905_dev->network_opt_per1905.network_opt_device_state == NETOPT_STATE_COMPLETE)
		{	
			err("dev state is already realization done, no need for BH steer");
			continue;
		}
		if (_1905_dev->network_opt_per1905.network_opt_device_state == NETOPT_STATE_OPT_NET_NEED_TO_REALIZE)
		{
			_1905_dev->network_opt_per1905.network_opt_device_state = NETOPT_STATE_OPT_NET_REALIZATION_ONGOING;
			err("realize for _1905_dev-> almac "MACSTR"",MAC2STR(_1905_dev->_1905_info.al_mac_addr));
			break;
		}
	}
	if (_1905_dev->network_opt_per1905.max_score_radio)
	{
		max_radio = _1905_dev->network_opt_per1905.max_score_radio;
		if (max_radio->network_opt_1905dev_radio.max_score_link)
		{
			max_link = max_radio->network_opt_1905dev_radio.max_score_link;
		}
		else
		{
			err("max score link is null ");
			network_opt_reset(global);
			dev->network_optimization.NetOptReason = REASON_TRY_AGAIN;
#ifdef MAP_R2
			handle_task_completion(dev);
#endif
			return;
		}
	}
	else
		{
			err("max score radio is NULL ");
			network_opt_reset(global);
			dev->network_optimization.NetOptReason = REASON_TRY_AGAIN;
#ifdef MAP_R2
			handle_task_completion(dev);
#endif
			return;
		}
//find the target 1905 BSSID 
	if (_1905_dev->network_opt_per1905.est_upstream_device)
		target_1905=_1905_dev->network_opt_per1905.est_upstream_device;
	else {
		err("no valid upstream dev present");
		network_opt_reset(global);
		dev->network_optimization.NetOptReason = REASON_TRY_AGAIN;
#ifdef MAP_R2
		handle_task_completion(dev);
#endif
		return;
	}
// if the current upstream device is the target upstream then 
//check if the _1905 dev in opt network under consideration is connected to upstream with same radio now as per the max link
//directly move state to realization done for this dev , no need to BH steer 
	if ((_1905_dev->network_opt_per1905.est_upstream_device == _1905_dev->upstream_device) &&
		(max_radio->uplink_bh_present))
	{
		err("current upstream is same as est upstream return ");
		_1905_dev->network_opt_per1905.network_opt_device_state = NETOPT_STATE_COMPLETE;
		return;
	}		

	target_bss = topo_srv_get_bss_by_bssid(&global->dev, target_1905, max_link->peer_mac);
	if(target_bss == NULL) {
		err("target bss not found");
		network_opt_reset(global);
		dev->network_optimization.NetOptReason = REASON_TRY_AGAIN;
#ifdef MAP_R2
		handle_task_completion(dev);
#endif
		return ;
	}
//find the relevant apcli mac address
	SLIST_FOREACH(iface, &(_1905_dev->_1905_info.first_iface), next_iface) {
		if ((iface->media_type >= ieee_802_11_b && iface->media_type <= ieee_802_11_ax)
			&& iface->media_info.role == 0x4){
				if(get_band(_1905_dev, max_radio->channel[0]) == get_band(_1905_dev, iface->channel_freq_idx))
				{	info("iface->is_map_if %d", iface->is_map_if);
					info("iface->media_type  %d", iface->media_type );
					info("iface->media_info.role  %d", iface->media_info.role );
					info("iface->channel_freq_idx %d", iface->channel_freq_idx);
					info("max_radio->channel[0] %d ",max_radio->channel[0]);
					info("selected agent %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(iface->iface_addr));
				    break;
				}
		}
	}
	if (iface == NULL)
	{
		err("iface is null");
		network_opt_reset(global);
		dev->network_optimization.NetOptReason = REASON_TRY_AGAIN;
#ifdef MAP_R2
		handle_task_completion(dev);
#endif
		return;
	}
	err("#### Network Opt BH steer command data #### ");
	err("max_radio->identifier "MACSTR" ",MAC2STR(max_radio->identifier));
	err("_1905_dev->_1905_info.al_mac_addr "MACSTR" ",MAC2STR(_1905_dev->_1905_info.al_mac_addr));
	err("iface->iface_addr "MACSTR" ",MAC2STR(iface->iface_addr));
	err("max_link->peer_mac "MACSTR" ",MAC2STR(max_link->peer_mac));
	err("class %d ,channel %d", target_bss->radio->operating_class,target_bss->radio->channel[0]);
	os_get_time(&_1905_dev->network_opt_per1905.bh_steer_start_ts);
//give BH steer command
	map_1905_Send_Backhaul_Steering_Request_Message(global->_1905_ctrl, 
		(char *)_1905_dev->_1905_info.al_mac_addr,
		(unsigned char *)iface->iface_addr,
		(unsigned char *)max_link->peer_mac,
		target_bss->radio->operating_class,
		target_bss->radio->channel[0]);
	dev->network_optimization.network_opt_state = NETOPT_STATE_OPT_NET_REALIZATION_ONGOING;
}
void dump_max_score_device_info(struct _1905_map_device *max_score_dev)
{

	err("max device almac %02x:%02x:%02x:%02x:%02x:%02x ", PRINT_MAC(max_score_dev->_1905_info.al_mac_addr));
	err("max radio identifier %02x:%02x:%02x:%02x:%02x:%02x ", PRINT_MAC(max_score_dev->network_opt_per1905.max_score_radio->identifier));
	err("max radio channel %d ", max_score_dev->network_opt_per1905.max_score_radio->channel[0]);
	if(max_score_dev->network_opt_per1905.max_score_radio->network_opt_1905dev_radio.max_score_link)
	{
		err("peer_MAC or BSSID "MACSTR" ", MAC2STR(max_score_dev->network_opt_per1905.max_score_radio->network_opt_1905dev_radio.max_score_link->peer_mac));
		err("PEER ALMAC "MACSTR" ", MAC2STR(max_score_dev->network_opt_per1905.max_score_radio->network_opt_1905dev_radio.max_score_link->dev_almac));
		err("max_score	of link is %d ",max_score_dev->network_opt_per1905.max_score_radio->network_opt_1905dev_radio.max_score_link->estimated_score);
		err("latched est max_score	of link is %d ",max_score_dev->network_opt_per1905.est_max_score );
	}
	err("estimated hop count %d ", max_score_dev->network_opt_per1905.est_hop_count);
}
void dump_est_opt_net(struct own_1905_device *dev)
{
	struct optimized_network *opt_1905_device = NULL;
	if(SLIST_EMPTY(&(dev->network_optimization.first_opt_net_dev)))
	{
		err("opt network link list is empty");
		return;
	}
	SLIST_FOREACH( opt_1905_device,&dev->network_optimization.first_opt_net_dev, next_opt_net_dev)
	{
		err("device %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(opt_1905_device->opt_net_dev->_1905_info.al_mac_addr));
		err("estimated hop_count %d",opt_1905_device->opt_net_dev->network_opt_per1905.est_hop_count);
		err("estimated max_score  %d",opt_1905_device->opt_net_dev->network_opt_per1905.est_max_score);
		if(opt_1905_device->opt_net_dev->network_opt_per1905.est_upstream_device) {
			err("upstream device %02x:%02x:%02x:%02x:%02x:%02x", PRINT_MAC(opt_1905_device->opt_net_dev->network_opt_per1905.est_upstream_device->_1905_info.al_mac_addr));
		} else {
			err ("upstream is NULL");
		}
	}
}
void add_dev_to_opt_net(
	struct _1905_map_device *_1905_dev,
	struct own_1905_device *dev)
{
	struct optimized_network *opt_1905_device = NULL;
	opt_1905_device = 	os_zalloc(sizeof(struct optimized_network));
	opt_1905_device->opt_net_dev = _1905_dev;
	SLIST_INSERT_HEAD(
		&(dev->network_optimization.first_opt_net_dev),
		opt_1905_device,
		next_opt_net_dev);
}

unsigned char is_link_already_present(struct N_O_link_estimate_cb  *link,struct _1905_map_device *_1905_device)
{
	struct _1905_map_device *upstream_dev = _1905_device->upstream_device;
	err("link ALMAC "MACSTR"", MAC2STR(link->dev_almac));
	err("upstream_dev ALMAC "MACSTR"", MAC2STR(upstream_dev->_1905_info.al_mac_addr));
	if (os_memcmp(link->dev_almac,upstream_dev->_1905_info.al_mac_addr,ETH_ALEN) == 0)
	{
		return 1;
	}
	return 0;
}
void find_link_with_max_score(struct own_1905_device *dev)
{
	struct _1905_map_device *_1905_device = NULL;
	struct radio_info_db *radio = NULL;
	struct N_O_link_estimate_cb  *link = NULL;
	struct _1905_map_device *max_score_dev= NULL;
	struct radio_info_db *max_score_radio = NULL;
	struct N_O_link_estimate_cb *max_score_link= NULL;
	struct _1905_map_device *upstream_dev = NULL;
	err("********find_link_with_max_score**********");
	_1905_device = topo_srv_get_next_1905_device(dev, NULL); /*Get own device struct.*/
	while(_1905_device) {
		if(_1905_device->network_opt_per1905.network_opt_device_state != NETOPT_STATE_NEED_TO_ESTIMATE)
		{
			_1905_device = topo_srv_get_next_1905_device(dev, _1905_device);
			info("continue");
			continue;
		}
		info("loop 1905 dev for ALMAC "MACSTR"", MAC2STR(_1905_device->_1905_info.al_mac_addr));
		radio = topo_srv_get_radio(_1905_device, NULL);
		max_score_radio = NULL;
		_1905_device->network_opt_per1905.max_score_radio = NULL;
		while (radio) {
			info("loop radio ID "MACSTR"",MAC2STR(radio->identifier));
			max_score_link = NULL;
			radio->network_opt_1905dev_radio.max_score_link = NULL;
			SLIST_FOREACH(link, &(radio->link_estimate_cb_head), link_estimate_cb_entry)
			{
				if (max_score_link == NULL) {
					max_score_link = link;
					continue;
				}
				if(link->estimated_score > max_score_link->estimated_score){
					max_score_link = link;
				}	
				else if (link->estimated_score == max_score_link->estimated_score)
				{
					info("link score is the same");
					if(is_link_already_present(link, _1905_device)){
						info("the current link is already present , so update max link with current link");
						max_score_link = link;
					}			
				}
			}
			radio->network_opt_1905dev_radio.max_score_link = max_score_link;
			if (radio->network_opt_1905dev_radio.max_score_link) {
				info("final max_score_link almac %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(max_score_link->dev_almac));
				info("final max_score_link  score %d", max_score_link->estimated_score);
				if (max_score_radio == NULL) {
					max_score_radio = radio;
				} else if(radio->network_opt_1905dev_radio.max_score_link->estimated_score > 
					(max_score_radio->network_opt_1905dev_radio.max_score_link->estimated_score)) {
					max_score_radio = radio;
				}
				else if(radio->network_opt_1905dev_radio.max_score_link->estimated_score == 
						max_score_radio->network_opt_1905dev_radio.max_score_link->estimated_score) {
					info("radio score same");
					if(radio->uplink_bh_present) {
						info("radio scores same , so update this max radio as uplink bh is present");
						max_score_radio = radio;
					}
				}
			}
			radio = topo_srv_get_next_radio(_1905_device, radio);
		}
		_1905_device->network_opt_per1905.max_score_radio = max_score_radio;
		if (max_score_radio != NULL){
			if (max_score_dev == NULL) {
				max_score_dev = _1905_device;
			} else if(max_score_radio->network_opt_1905dev_radio.max_score_link->estimated_score > 
				(max_score_dev->network_opt_per1905.max_score_radio->network_opt_1905dev_radio.max_score_link->estimated_score)) {
					max_score_dev = _1905_device;
			} 
		}
		_1905_device = topo_srv_get_next_1905_device(dev, _1905_device);
	}
	if (max_score_dev) {
		info("final max score dev almac "MACSTR"",MAC2STR(max_score_dev->_1905_info.al_mac_addr));
		if (max_score_dev->network_opt_per1905.max_score_radio->network_opt_1905dev_radio.max_score_link) {
			info("update max_score dev Upstream ");
			upstream_dev = topo_srv_get_1905_device(dev,max_score_dev->network_opt_per1905.max_score_radio->network_opt_1905dev_radio.max_score_link->dev_almac);
			max_score_dev->network_opt_per1905.est_upstream_device = upstream_dev; /**< pointer to upstream device */
			max_score_dev->network_opt_per1905.network_opt_device_state = NETOPT_STATE_OPT_NET_NEED_TO_REALIZE;
			max_score_dev->network_opt_per1905.est_hop_count = 
				max_score_dev->network_opt_per1905.max_score_radio->network_opt_1905dev_radio.max_score_link->estimated_hop_count;
			max_score_dev->network_opt_per1905.est_max_score = 
				max_score_dev->network_opt_per1905.max_score_radio->network_opt_1905dev_radio.max_score_link->estimated_score;
			max_score_dev->network_opt_per1905.est_max_uplink_rate= 
				max_score_dev->network_opt_per1905.max_score_radio->network_opt_1905dev_radio.max_score_link->estimated_rate;
		}
		else
		{
			info("keep existing upstream dev ");
			upstream_dev = max_score_dev->upstream_device;
			max_score_dev->network_opt_per1905.est_upstream_device = upstream_dev; /**< pointer to upstream device */
			max_score_dev->network_opt_per1905.network_opt_device_state = NETOPT_STATE_OPT_NET_NEED_TO_REALIZE;
			max_score_dev->network_opt_per1905.est_hop_count = upstream_dev->network_opt_per1905.est_hop_count;//estimate_hop_count(dev, max_score_dev);
		}
		add_dev_to_opt_net(max_score_dev,dev);
		dump_max_score_device_info(max_score_dev);
	}
}
void clear_estimated_scores(struct own_1905_device *dev)
{
	struct radio_info_db *radio = NULL;
	struct _1905_map_device *_1905_device = NULL;
	struct N_O_link_estimate_cb  *link = NULL;
	_1905_device = topo_srv_get_next_1905_device(dev, NULL); /*Get own device struct.*/
	while(_1905_device) {
		radio = topo_srv_get_radio(_1905_device, NULL);
		while (radio) {
			if(!SLIST_EMPTY(&(radio->link_estimate_cb_head)))
			{ 
				SLIST_FOREACH(link, &(radio->link_estimate_cb_head), link_estimate_cb_entry)
				{
					link->estimated_score = 0;
				}
			}
			radio = topo_srv_get_next_radio(_1905_device, radio);
		}
		_1905_device = topo_srv_get_next_1905_device(dev, _1905_device);
	}
}
void remove_all_dev_from_opt_net(
	struct own_1905_device *dev)
{
	struct optimized_network *opt_1905_device = NULL;
	while (!SLIST_EMPTY(&(dev->network_optimization.first_opt_net_dev))) {			/* List Deletion. */
			opt_1905_device = SLIST_FIRST(&(dev->network_optimization.first_opt_net_dev));
			SLIST_REMOVE_HEAD(&(dev->network_optimization.first_opt_net_dev), next_opt_net_dev);
			free(opt_1905_device);
	}
}
void remove_all_entry_link_est_db(
	struct _1905_map_device *_1905_device)
{
	struct radio_info_db *radio = NULL;
	struct N_O_link_estimate_cb *link = NULL;
	radio = topo_srv_get_next_radio(_1905_device, radio);
	while (radio) {
		while (!SLIST_EMPTY(&(radio->link_estimate_cb_head))) {			/* List Deletion. */
				link = SLIST_FIRST(&(radio->link_estimate_cb_head));
				SLIST_REMOVE_HEAD(&(radio->link_estimate_cb_head), link_estimate_cb_entry);
				os_free(link);
		}
		radio = topo_srv_get_next_radio(_1905_device, radio);
	}
}

unsigned int find_max_uplink_rate(struct _1905_map_device *_1905_device)
{
	unsigned int est_max_score =0;
	struct radio_info_db *radio = NULL;
	radio = topo_srv_get_radio(_1905_device, NULL);
	while (radio) {
		if (radio->uplink_rate > est_max_score)
			est_max_score = radio->uplink_rate;
		radio = topo_srv_get_next_radio(_1905_device, radio);
	}
	return est_max_score;
}

void network_opt_init(struct mapd_global *pGlobal_dev)
{
	struct own_1905_device *dev = &pGlobal_dev->dev;
	dev->network_optimization.NetOptReason = REASON_NOT_REQUIRED;
	if (dev->network_optimization.wait_time == 0 )
		dev->network_optimization.wait_time = 45;
	if (dev->network_optimization.data_collection_wait_time == 0 )
		dev->network_optimization.data_collection_wait_time = 300;
	if (dev->network_optimization.bh_steer_wait_time == 0 )
		dev->network_optimization.bh_steer_wait_time = 60;
	if (dev->network_optimization.disconnect_wait_time == 0)
		dev->network_optimization.disconnect_wait_time = 45;
	if (dev->network_optimization.connect_wait_time == 0)
		dev->network_optimization.connect_wait_time = 45;
	if (dev->network_optimization.post_cac_trigger_time== 0)
		dev->network_optimization.post_cac_trigger_time = 25;
	err("conn %d dis %d",dev->network_optimization.connect_wait_time,dev->network_optimization.disconnect_wait_time);
	dev->network_optimization.network_opt_state = NETOPT_STATE_IDLE; 
}
void network_opt_reset(struct mapd_global *pGlobal_dev)
{
	struct own_1905_device *dev = &pGlobal_dev->dev;
	struct _1905_map_device *_1905_device = NULL;
	dev->network_optimization.NetOptReason = REASON_NOT_REQUIRED;
	dev->network_optimization.network_opt_state = NETOPT_STATE_IDLE;
	err(" ");
	_1905_device = topo_srv_get_next_1905_device(dev, NULL); /*Get own device struct.*/
	while(_1905_device) {
			_1905_device->network_opt_per1905.network_opt_device_state =
				NETOPT_STATE_IDLE;
			_1905_device->network_opt_per1905.data_col_retry_count = 0;
			remove_all_entry_link_est_db(_1905_device);
		if (_1905_device->net_opt_scan_report)
			os_free(_1905_device->net_opt_scan_report);

		_1905_device->net_opt_scan_report = NULL;
		_1905_device = topo_srv_get_next_1905_device(&pGlobal_dev->dev,_1905_device);
	}
	reset_last_uplink_rate(pGlobal_dev);
	remove_all_dev_from_opt_net(dev);
	os_get_time(&dev->network_optimization.trigger_netopt_ts);
}
void dump_link_estimate_db_per_dev(struct _1905_map_device *_1905_device)
{
	struct	N_O_link_estimate_cb *NO_cb = NULL;
	struct radio_info_db *radio = NULL;
	signed char Link_RSSI = 0;
	radio = topo_srv_get_radio(_1905_device, NULL);
	while (radio) {		
		err("radio->identifier:"MACSTR"\n", MAC2STR(radio->identifier));
		if(!SLIST_EMPTY(&(radio->link_estimate_cb_head))) {
			SLIST_FOREACH(NO_cb, &radio->link_estimate_cb_head, link_estimate_cb_entry) {
				err("ALMAC:"MACSTR"\n", MAC2STR(NO_cb->dev_almac));
				err("STA_MAC/BSSID:"MACSTR"\n", MAC2STR(NO_cb->peer_mac));
				err("RCPI = %d\n ",  NO_cb->rcpi);
				Link_RSSI = rcpi_to_rssi(NO_cb->rcpi);
				err("Link RSSI is %d", Link_RSSI);
			}
		}
		else
		{
			err("SLIST is empty for this radio ");
		}
		radio = topo_srv_get_next_radio(_1905_device, radio);
	}
}
struct	N_O_link_estimate_cb *topo_srv_get_next_N_O_link(struct radio_info_db *radio,struct N_O_link_estimate_cb *NO_cb)
{
	struct list_link_estimate_cb *NO_cb_list;
	if (!radio) {
		err("radio is null");
		return NULL;
	}
	NO_cb_list = &radio->link_estimate_cb_head;
	if (SLIST_EMPTY(NO_cb_list))
		return NULL;
	if (!NO_cb)
		return SLIST_FIRST(NO_cb_list);
	return SLIST_NEXT(NO_cb, link_estimate_cb_entry);
}
struct	N_O_link_estimate_cb *find_existing_entry_in_cb(struct radio_info_db *radio,struct neighbor_info *nb_info)
{
	struct	N_O_link_estimate_cb *NO_cb = NULL;
	do {
		NO_cb = topo_srv_get_next_N_O_link(radio, NO_cb);
		if (!NO_cb) {
			info("failed to find existing NO_cb");
			return NULL;
		}
		if (os_memcmp(NO_cb->peer_mac, nb_info->bssid, ETH_ALEN) == 0)
			return NO_cb;
	} while (NO_cb);
	return NULL;
}

int is_that_a_bh_nb_info(struct mapd_global *global, char *ssid)
{
	int a;
	for (a = 0; a < MAX_SET_BSS_INFO_NUM; a++) {
		if ((os_strcmp(global->dev.bss_config[a].ssid, ssid) == 0)
			&& (global->dev.bss_config[a].wfa_vendor_extension & BSS_BH)) {
			return TRUE;
		}
	}
	return FALSE;
}

int Update_link_estimation_db_per_dev(struct own_1905_device *ctx,struct _1905_map_device *_1905_device)
{
	int i=0,j=0; 
	struct	N_O_link_estimate_cb *NO_cb = NULL;
	struct _1905_map_device *peer_map_device = NULL;
	struct radio_info_db *radio= NULL;
	u8 *buf = NULL;
	struct mapd_global *global = ctx->back_ptr;
	if(_1905_device->net_opt_scan_report == NULL)
	{
		err("no off channel scan report received for dev with almac"MACSTR"",MAC2STR(_1905_device->_1905_info.al_mac_addr));
		return -1;
	}
	buf = (u8 *)_1905_device->net_opt_scan_report->scan_result; 
	for(i=0;i<_1905_device->net_opt_scan_report->scan_result_num;i++)
	{	
		struct net_opt_scan_result_event *scan_result_evt = (struct net_opt_scan_result_event *)buf;
		radio= topo_srv_get_radio(_1905_device, scan_result_evt->radio_id);
		if(radio == NULL)
		{
			err("radio not found");
			continue;
		}
#ifdef WIFI_MD_COEX_SUPPORT
		/*to check if the channel is operable when doing network optimization*/
		if (ch_planning_check_channel_for_dev_operable_wrapper(_1905_device, scan_result_evt->channel))
#endif
		for(j=0;j<scan_result_evt->neighbor_num;j++)
		{	
			struct neighbor_info *nb_info = &scan_result_evt->nb_info[j];
			peer_map_device = topo_srv_get_1905_by_bssid(ctx,nb_info->bssid);
			if(!peer_map_device) {
				err("peer mapdev not found continue\n");
				continue;
			}
			/*Check if the ssid supports bh as given by controller*/
			if (!is_that_a_bh_nb_info(global, (char *)nb_info->ssid))
				continue;
			NO_cb = find_existing_entry_in_cb(radio,nb_info);
			if(!NO_cb)//bssid not already present in the link_estimated_db_list
			{
				NO_cb = os_zalloc(sizeof(*NO_cb));
				if (!NO_cb) {
					err("mem allocation failed");
					return -1;
				}
				os_memcpy(NO_cb->peer_mac, &nb_info->bssid, ETH_ALEN);
				os_memcpy(NO_cb->dev_almac,peer_map_device->_1905_info.al_mac_addr,ETH_ALEN);
				SLIST_INSERT_HEAD(&(radio->link_estimate_cb_head), NO_cb, link_estimate_cb_entry);
			}
			NO_cb->rcpi=nb_info->RCPI;//for all entries (existing and new) update RSSI
		}
		buf += sizeof(struct net_opt_scan_result_event)+ (scan_result_evt->neighbor_num*sizeof(struct neighbor_info));
	}
	dump_link_estimate_db_per_dev(_1905_device);
	return 0;
}
void Update_link_estimation_db (struct mapd_global *pGlobal_dev)
{
	struct _1905_map_device *_1905_device = NULL;
	struct own_1905_device *ctx = &pGlobal_dev->dev;
	_1905_device = topo_srv_get_1905_device(ctx, NULL); 
	while(_1905_device) {
		if(_1905_device->device_role != DEVICE_ROLE_CONTROLLER) 
		{
			err("for dev with almac"MACSTR"\n",MAC2STR(_1905_device->_1905_info.al_mac_addr));
			Update_link_estimation_db_per_dev(ctx,_1905_device);
		}
		_1905_device = topo_srv_get_next_1905_device(&pGlobal_dev->dev,_1905_device);
	}
}
void network_opt_get_unique_channel_list(struct own_1905_device *ctx,
	unsigned char *channel_arr)
{
	struct _1905_map_device *_1905_device = NULL;
	struct radio_info_db *radio = NULL;
	unsigned char channel_count = 0;
	struct bss_info_db *bss = NULL;
	reset_net_opt_allowed(ctx);
	_1905_device = topo_srv_get_1905_device(ctx,NULL);
	if (_1905_device) {
		/* For every bss */
		SLIST_FOREACH(bss, &(_1905_device->first_bss), next_bss) {
			net_opt_check_bh_scan_allowed(ctx, bss);
		}
	}

	while (_1905_device) {
		radio = topo_srv_get_radio(_1905_device,NULL);
		while (radio) {
			if((radio->channel[0] <= 14 && !ctx->network_optimization.scan_2g_allow) || 
				(isChan5GL(radio->channel[0])&& !ctx->network_optimization.scan_5gl_allow) ||
				(isChan5GH(radio->channel[0])&& !ctx->network_optimization.scan_5gh_allow)) {
				radio = topo_srv_get_next_radio(_1905_device, radio);
				continue;
			}
			int i = 0;
			while (i < channel_count) {
				if (channel_arr[i] == radio->channel[0]) {
					break;
				}
				i++;
			}
			if (i == channel_count) {
				err("add ch %d ", radio->channel[0]);
				channel_arr[i] = radio->channel[0];
			}
			channel_count++;
			radio = topo_srv_get_next_radio(_1905_device, radio);
		}
		_1905_device = topo_srv_get_next_1905_device(ctx,_1905_device);
	}
}
void network_opt_data_collection(struct mapd_global *pGlobal_dev)
{
	struct _1905_map_device *_1905_device = NULL;
	struct own_1905_device *dev = &pGlobal_dev->dev;
	unsigned char scan_ch_list[MAX_OFF_CH_SCAN_CH] = {0};
	err("N.O. data collection");
	network_opt_get_unique_channel_list(dev,scan_ch_list);
	_1905_device = topo_srv_get_next_1905_device(dev, NULL); /*Get own device struct.*/
	while(_1905_device) {
		if (!_1905_device->in_network) {
			_1905_device = topo_srv_get_next_1905_device(&pGlobal_dev->dev,_1905_device);
			continue;
		}
		if (_1905_device->device_role == DEVICE_ROLE_CONTROLLER) {
			_1905_device->network_opt_per1905.network_opt_device_state = NETOPT_STATE_DATA_COLLECTION_COMPLETE;
	} else {
			//avoid sending off channel scan req here if the BH of 1905 device is ethernet.
			if (is_eth_BH(_1905_device) == 0){
				_1905_device->network_opt_per1905.network_opt_device_state= NETOPT_STATE_DATA_COLLECTION_ONGOING;
				_1905_device->network_opt_per1905.data_col_retry_count = 0;
				os_get_time(&_1905_device->network_opt_per1905.data_collection_start_ts);
#ifdef MAP_R2
			if (dev->map_version == DEV_TYPE_R2 && _1905_device->map_version == DEV_TYPE_R2) {
				err("Send R2 device Channel Scan Request to %02x:%02x:%02x:%02x:%02x:%02x",
						MAC2STR(_1905_device->_1905_info.al_mac_addr));
				/*Wait for 120 ms so that no two devices scan same channel at the same time
				if two devices scan at the same time on same channel, they won't receive
				each other's beacon*/
				os_sleep(0,120000);
				send_channel_scan_req(pGlobal_dev,_1905_device, scan_ch_list);
			}
			else
				send_off_ch_scan_req(pGlobal_dev, _1905_device, SCAN_MODE_CH, 0, scan_ch_list, 0,0);
#else
				send_off_ch_scan_req(pGlobal_dev, _1905_device, SCAN_MODE_CH, 0, scan_ch_list, 0,0);
#endif
			}
			else {
				_1905_device->network_opt_per1905.network_opt_device_state = NETOPT_STATE_DATA_COLLECTION_COMPLETE;
				err("ethernet BH so don't do off channel scan ");
			}	
		}
		_1905_device = topo_srv_get_next_1905_device(&pGlobal_dev->dev,_1905_device);
	}
}
void retry_data_collection(struct mapd_global *pGlobal_dev,struct _1905_map_device * _1905_device)
{
	unsigned char scan_ch_list[MAX_OFF_CH_SCAN_CH] = {0};
	network_opt_get_unique_channel_list(&pGlobal_dev->dev,
		scan_ch_list);
#ifdef MAP_R2
	if (pGlobal_dev->dev.map_version == DEV_TYPE_R2 && _1905_device->map_version == DEV_TYPE_R2) {
		send_channel_scan_req(pGlobal_dev,_1905_device, scan_ch_list);
	} else
#endif
	send_off_ch_scan_req(pGlobal_dev, _1905_device, SCAN_MODE_CH, 0, scan_ch_list, 0,0);
}
unsigned char is_net_opt_data_collection_complete(struct mapd_global *pGlobal_dev)
{
	struct own_1905_device *dev = &pGlobal_dev->dev;
	struct _1905_map_device *_1905_device = NULL;	
	_1905_device = topo_srv_get_next_1905_device(dev, NULL); /*Get own device struct.*/
	while(_1905_device) {
		if (_1905_device->device_role != DEVICE_ROLE_CONTROLLER) {
			if(_1905_device->network_opt_per1905.network_opt_device_state == NETOPT_STATE_DATA_COLLECTION_ONGOING) {
				info("Not complete yet");
				dev->network_optimization.network_opt_state = NETOPT_STATE_DATA_COLLECTION_ONGOING;
				return 0;
				break;
			}
		}
		_1905_device = topo_srv_get_next_1905_device(&pGlobal_dev->dev,_1905_device);
	}
	return 1;//complete
}
unsigned char periodic_network_opt_checking(struct mapd_global *pGlobal_dev)
{
	struct own_1905_device *dev = &pGlobal_dev->dev;
	struct os_time now;
	struct _1905_map_device *_1905_device = NULL;	
	unsigned char is_Data_collection_complete = 0;
	os_get_time(&now);
	_1905_device = topo_srv_get_next_1905_device(dev, NULL); /*Get own device struct.*/
	while(_1905_device) {
			if(_1905_device->network_opt_per1905.network_opt_device_state == NETOPT_STATE_DATA_COLLECTION_ONGOING) {
				if (now.sec > (_1905_device->network_opt_per1905.data_collection_start_ts.sec + 
						dev->network_optimization.data_collection_wait_time)) {
						if(_1905_device->network_opt_per1905.data_col_retry_count < 5){
							_1905_device->network_opt_per1905.data_col_retry_count++;
							os_get_time(&_1905_device->network_opt_per1905.data_collection_start_ts);
							err("retry data collection , report not received");
							retry_data_collection(pGlobal_dev,_1905_device);
						}
						else {
							err("retry count expired , put state as data collection complete");
							_1905_device->network_opt_per1905.network_opt_device_state = NETOPT_STATE_DATA_COLLECTION_COMPLETE;
							_1905_device->network_opt_per1905.data_col_retry_count = 0;
					}
				}
			}
		_1905_device = topo_srv_get_next_1905_device(&pGlobal_dev->dev,_1905_device);
	}	
	is_Data_collection_complete = is_net_opt_data_collection_complete(pGlobal_dev);
	info("is_Data_collection_complete %d",is_Data_collection_complete);
	return is_Data_collection_complete;
}
/*
* Check if Network Optimization is Required
*/
void is_network_opt_required_rate(struct mapd_global *pGlobal_dev)
{
	struct _1905_map_device *_1905_device = NULL;
	struct own_1905_device *ctx = &pGlobal_dev->dev;
	struct radio_info_db *radio = NULL;
	unsigned char count_threshold = 5;
	_1905_device = topo_srv_get_next_1905_device(ctx, NULL); /*Get own device struct.*/
	while(_1905_device) {
		if (_1905_device->device_role != DEVICE_ROLE_CONTROLLER) {
			radio = topo_srv_get_radio(_1905_device, NULL);
			while(radio) {
				if(radio->network_opt_1905dev_radio.rate_deviate_count > count_threshold) {
					ctx->network_optimization.NetOptReason = REASON_RSSI_VARIATION;
					err("device radio uplink rate deviates for more than 5mins , reset last uplink rate");
					radio->network_opt_1905dev_radio.last_uplink_rate = 0;
					radio->network_opt_1905dev_radio.rate_deviate_count = 0;
					break;
				}	
				radio = topo_srv_get_next_radio(_1905_device,radio);
			}
		}
		if (ctx->network_optimization.NetOptReason == REASON_RSSI_VARIATION)
			break;
		_1905_device = topo_srv_get_next_1905_device(ctx,_1905_device);
	}
		return;
}
unsigned char is_all_dev_idle(struct own_1905_device *dev)
{
	struct _1905_map_device *_1905_device = NULL;	
	unsigned char is_idle =1;
	_1905_device = topo_srv_get_next_1905_device(dev, NULL); /*Get own device struct.*/
	while(_1905_device) {
		if ((_1905_device->network_opt_per1905.network_opt_device_state != NETOPT_STATE_IDLE ))
		{
			is_idle = 0;
			break;
		}
		_1905_device = topo_srv_get_next_1905_device(dev,_1905_device);
	}

	return is_idle;
}
void reset_last_uplink_rate(struct mapd_global *pGlobal_dev)
{
	struct _1905_map_device *_1905_device = NULL;
	struct own_1905_device *ctx = &pGlobal_dev->dev;
	struct radio_info_db *radio = NULL;
	_1905_device = topo_srv_get_next_1905_device(ctx, NULL); /*Get own device struct.*/
	while(_1905_device) {
		if (_1905_device->device_role != DEVICE_ROLE_CONTROLLER) {
			radio = topo_srv_get_radio(_1905_device, NULL);
			while(radio) {
				info("reset last uplink rate %d  to 0",radio->network_opt_1905dev_radio.last_uplink_rate);
				radio->network_opt_1905dev_radio.last_uplink_rate = 0;
				radio->network_opt_1905dev_radio.rate_deviate_count = 0;
				radio = topo_srv_get_next_radio(_1905_device,radio);
			}
		}
		_1905_device = topo_srv_get_next_1905_device(ctx,_1905_device);
	}
}
int is_network_opt_required(struct mapd_global *pGlobal_dev)
{
	struct own_1905_device *dev = &pGlobal_dev->dev;
	struct os_time now;
	unsigned int count = 0;
	struct _1905_map_device *_1905_device = NULL;	
	int status = REASON_NOT_REQUIRED;
	if(is_all_dev_idle(dev) == 0) {
		err("all 1905 devs are not idle so REASON_NOT_REQUIRED");
		status = REASON_NOT_REQUIRED;
			return status;
	}
	_1905_device = topo_srv_get_next_1905_device(dev, NULL); /*Get own device struct.*/
	while(_1905_device) {
		if (_1905_device->in_network)
		{
			info("In Network ALMAC" MACSTR, MAC2STR(_1905_device->_1905_info.al_mac_addr));
			count++;
		}
		_1905_device = topo_srv_get_next_1905_device(dev,_1905_device);
	}
	if(count < MINIMUM_AGENTS_COUNT_FOR_NETWORK_OPTIMIZATION) {
		status = REASON_NOT_REQUIRED;
		dev->network_optimization.NetOptReason = status;
		info("no agent present, don't run Net Opt");
		return status;
	}
	if (dev->network_optimization.NetOptReason == REASON_ENABLED_BY_USER) {
		err("REASON_ENABLED_BY_USER");
		status = dev->network_optimization.NetOptReason;
		return status;
	}
	os_get_time(&now);
	if (dev->network_optimization.NetOptReason == REASON_TRY_AGAIN && (now.sec > dev->network_optimization.trigger_netopt_ts.sec + dev->network_optimization.wait_time)) {
		err("REASON_TRY_AGAIN");
		status = dev->network_optimization.NetOptReason;
		return status;
	}
	if (dev->network_optimization.NetOptReason == REASON_TOPOLOGY_CHANGE) {
		//err("dev->network_optmization.ntwrk_change_ts.sec %ld",dev->network_optimization.ntwrk_change_ts.sec);
		//err("dev->network_optmization.wait_time %d",dev->network_optimization.wait_time);
		if((now.sec > dev->network_optimization.ntwrk_change_ts.sec +
			dev->network_optimization.wait_time)) {
			err("now %ld",now.sec);
			err("REASON_TOPOLOGY_CHANGE");
			status = dev->network_optimization.NetOptReason;
			return status;
	} else {
			info("Wait Time %ld", now.sec);
			status = REASON_NOT_REQUIRED; //as wait time is not over
			return status;
		}
	}
	//RSSI comparison in current topology with the last captured one , 
	//if it varies more than uplink rate margin 
	//for more than count allowed then only trigger NetOpt
	is_network_opt_required_rate (pGlobal_dev);
	if (dev->network_optimization.NetOptReason == REASON_RSSI_VARIATION)
	{
		err("REASON_RSSI_VARIATION");
		status = REASON_RSSI_VARIATION;
		reset_last_uplink_rate(pGlobal_dev);
	}

	return status;
}

int get_net_opt_dev_count(struct mapd_global *pGlobal_dev)
{
	struct own_1905_device *dev = &pGlobal_dev->dev;
	unsigned int count = 0;
	struct _1905_map_device *_1905_device = NULL;	
	_1905_device = topo_srv_get_next_1905_device(dev, NULL); /*Get own device struct.*/
	while(_1905_device) {
		if (_1905_device->in_network)
		{
			info("In Network ALMAC" MACSTR, MAC2STR(_1905_device->_1905_info.al_mac_addr));
			count++;
		}
		_1905_device = topo_srv_get_next_1905_device(dev,_1905_device);
	}
	return count;
}

void ntwrk_opt_device_conn_disconnect_handle(struct own_1905_device *ctx, unsigned char IsConnect, struct _1905_map_device *_1905_dev)
{

	err("Connect %d ",IsConnect);
#ifdef STOP_BEACON_FEATURE
	if((ctx->device_role != DEVICE_ROLE_CONTROLLER)) {

		if(!IsConnect) {
			err("Disabling Beacon");
			system("wappctrl ra0 map set_beacon_state disable");
			ctx->beacon_enable = 0;
		} else {
			ctx->beacon_enable = 1;
		}
		return;
	}
#else
	 if((ctx->device_role != DEVICE_ROLE_CONTROLLER))
		return;
#endif

	if(ctx->network_optimization.network_optimization_enabled == 0){
		return;
	}

	if(IsConnect == DEVICE_CONNECT) {
		ctx->network_optimization.wait_time = ctx->network_optimization.connect_wait_time;
	} else {
		ctx->network_optimization.wait_time = ctx->network_optimization.disconnect_wait_time;
	}

	os_get_time(&ctx->network_optimization.ntwrk_change_ts);
	err("Network Connect Disconnect state %d timestamp  %ld ", IsConnect, ctx->network_optimization.ntwrk_change_ts.sec);
	err("for neighbor(%02x:%02x:%02x:%02x:%02x:%02x) ",PRINT_MAC(_1905_dev->_1905_info.al_mac_addr));
		if ((_1905_dev->network_opt_per1905.network_opt_device_state == NETOPT_STATE_OPT_NET_REALIZATION_ONGOING) ) {
			if(IsConnect)
			{
				err("Net Opt complete ");
				_1905_dev->network_opt_per1905.network_opt_device_state = NETOPT_STATE_COMPLETE;
			}
		} else {
			err("topo changed ");
			network_opt_reset(ctx->back_ptr);
			ctx->network_optimization.NetOptReason = REASON_TOPOLOGY_CHANGE;
			if(ctx->network_optimization.prefer_5G_bh)
				ctx->network_optimization.prefer_5G_bh_try_cnt_curr = 0;
		}	
}
void Update_controller_state(struct own_1905_device *dev )
{
	struct _1905_map_device *_1905_device = NULL;	
	_1905_device = topo_srv_get_next_1905_device(dev, NULL); /*Get own device struct.*/
	while(_1905_device) {
		if (_1905_device->device_role == DEVICE_ROLE_CONTROLLER)
		{
			_1905_device->network_opt_per1905.network_opt_device_state = NETOPT_STATE_COMPLETE;
		}
		_1905_device = topo_srv_get_next_1905_device(dev,_1905_device);
	}
}

void trigger_net_opt(void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *dev= eloop_ctx;
	struct _1905_map_device *_1905_dev = topo_srv_get_1905_device(dev, NULL);
	struct radio_info_db *radio = NULL;
	if(dev->network_optimization.network_optimization_enabled){
		dev->network_optimization.NetOptReason = REASON_TOPOLOGY_CHANGE;
		if(dev->network_optimization.prefer_5G_bh)
			dev->network_optimization.prefer_5G_bh_try_cnt_curr = 0;
	}
	while (_1905_dev) {
		radio = topo_srv_get_radio(_1905_dev, NULL);
		while (radio) {
			radio->cac_channel = 0;
			radio->cac_enable = 0;
			radio->cac_timer = 0;
			radio = topo_srv_get_next_radio(_1905_dev, radio);
		}
		_1905_dev = topo_srv_get_next_1905_device(dev, _1905_dev);
	}
}

void trigger_5G_net_opt(void *eloop_ctx, void *timeout_ctx)
{
	struct own_1905_device *dev= eloop_ctx;
	struct _1905_map_device *map_dev;
	struct radio_info_db *radio = NULL;

	always("Nw opt trigger timer function");
	SLIST_FOREACH(map_dev, &(dev->_1905_dev_head), next_1905_device) {
		do {
			radio = topo_srv_get_next_radio(map_dev, radio);
			if (radio) {
				if ((radio->channel[0] > 14) && (radio->uplink_bh_present == TRUE)){
					err("do nothing, already on 5G");
					return;
					}
				}
			}while(radio);
	}

	if(dev->network_optimization.network_optimization_enabled){
		dev->network_optimization.NetOptReason = REASON_TOPOLOGY_CHANGE;
		dev->network_optimization.prefer_5G_bh_try_cnt_curr = 0;
	}
	dev->nw_opt_triggered_5G = FALSE;
	dev->nw_opt_triggered_5G_in_process = TRUE;
}

void network_optimization_state_handler(struct mapd_global *global)
{
	struct own_1905_device *dev = &global->dev;
	NETOPT_STATE netopt_state;
	int status;
	unsigned char is_Data_collection_complete = 0;
	unsigned char all_dev_state = 0;
	netopt_state = dev->network_optimization.network_opt_state;
	info("netopt_state %d", netopt_state);
	switch(netopt_state)
	{
	 case NETOPT_STATE_IDLE:
			status = is_network_opt_required(global);
			info("is_network_opt_required status %d\n", status);
			if(status != REASON_NOT_REQUIRED) {
#ifdef MAP_R2
				if((dev->ch_planning_R2.ch_plan_enable == TRUE) &&
					((dev->ch_planning_R2.ch_plan_state != CHPLAN_STATE_IDLE) ||
						(dev->user_triggered_scan == TRUE)))
				{ 
					dev->network_optimization.network_optimization_enabled = 0;
					network_opt_reset(global);
					dev->network_optimization.NetOptReason = REASON_ENABLED_BY_USER;
					insert_into_task_list(dev,TASK_NETWORK_OPT_TRIGGER, NULL, NULL, NULL);
					break;
				}
#endif
				network_opt_reset(global);
				dev->network_optimization.network_opt_state = NETOPT_STATE_DATA_COLLECTION_ONGOING;
				network_opt_data_collection(global);
				err("triggered collection");
			}
			break;
	case NETOPT_STATE_DATA_COLLECTION_ONGOING:
			info("NETOPT_STATE_DATA_COLLECTION_ONGOING");
			is_Data_collection_complete = periodic_network_opt_checking(global);
			if(is_Data_collection_complete) {
				Update_link_estimation_db(global);
				Optimized_Link_estimation_algo(global);
				dev->network_optimization.network_opt_state = NETOPT_STATE_OPT_NET_NEED_TO_REALIZE;
			} else {
				break;
			}
	 case NETOPT_STATE_OPT_NET_NEED_TO_REALIZE:
			//send BH steer command per device
			Optimized_network_realization(global);
			dev->network_optimization.network_opt_state = NETOPT_STATE_OPT_NET_REALIZATION_ONGOING;
			break;
	 case NETOPT_STATE_OPT_NET_REALIZATION_ONGOING:
	 		//if all devices are having state done , then move global state done.
			all_dev_state = is_all_dev_state_done(dev) ;
		 	if (all_dev_state == REALIZATION_DONE){
				err("all devices BH steer complete\n");
	 			network_opt_reset(global);
				dev->network_optimization.network_opt_state = NETOPT_STATE_IDLE;
#ifdef MAP_R2
				handle_task_completion(&global->dev);
#endif
				if(dev->nw_opt_triggered_5G_in_process == TRUE) {
					dev->nw_opt_triggered_5G_in_process = FALSE;
				}
		 	} else if (all_dev_state == TRIGGER_NEW_REALIZATION){
		 		Optimized_network_realization(global);
				dev->network_optimization.network_opt_state = NETOPT_STATE_OPT_NET_REALIZATION_ONGOING;
		 	}
			break;
	 default:
	 		err("invalid state for network optimization");
	}
}
