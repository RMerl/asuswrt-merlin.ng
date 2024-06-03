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
 *  Dummy Lib
 *
 *  Abstract:
 *  This file is having the dummy placeholders for the API exposed 
 *  by MTK's MAP lib.
 *
 *  Revision History:
 *  Who          When          What
 *  --------     ----------    -----------------------------------------
 *  Amit Kumar   2019/10/14     Dummy placeholder definitions
 * */
#include "includes.h"
#include "common.h"
#include "client_db.h"
#include "mapd_i.h"
#ifdef SUPPORT_MULTI_AP
#include "topologySrv.h"
#endif
#include "steer_action.h"

/* forward Declarations */
struct client;
struct _1905_map_device;

#ifdef NO_SUPPORT_MULTIAP_MTK_LIB
/*
 * @brief: The API will process all the scan results available for Backhaul links and selects
 * the best link on which backhaul link should be formed.
 *
 * @param ctx: IN pointer to own_1905_device structure.
 * @param p_select_bss: OUT double pointer to point selected BSS entry.
 * @return: Backhaul link entry which is expected to form the connection
 *
 */
struct bh_link_entry * ap_selection_find_best_ap(struct own_1905_device *ctx, struct scan_bss_list **p_selected_bss)
{
	return NULL;
}
#endif
/* function Skeleton */
/*
 * @brief: API is currently used to set proprietary information in beacons. Proprietary information is defined by map_vendor_ie
 *
 * @param ctx: pointer to the own_1905_device.
 * @param bss_ie: pointer to map_vendor specific IE.
 * @param flag: flag for MTK proprietary purposes.
 * @return: Returns ????on success.
 */
int ap_selection_update_vend_ie(struct own_1905_device *ctx, struct map_vendor_ie *bss_ie, char flag)
{
	return 0;
}

/* ******************************************************************
* These functions with dummy body are meant to handle compile errors
* when CONFIG_MULTIAP_MTK_LIB macro set to 'n' in mapd_config file.
* Customers who are not using MTK's LIB shouldn't refer these functions.
* These functions are part of proprietary MTK's algorithms.
********************************************************************/
#ifdef NO_SUPPORT_MULTIAP_MTK_LIB
int estimate_bss_phyrate(struct own_1905_device *ctx, struct radio_info_db *radio, struct bss_info *bss)
{
	return 0;
}
void topo_srv_update_uplink_rate(struct own_1905_device *ctx, struct _1905_map_device *dev)
{

}
unsigned int estimate_radio_phyrate(struct own_1905_device *ctx,
	struct radio_info_db *radio,
	signed char RSSI,
	unsigned  int wireless_mode,
	unsigned int client_streams,
	unsigned int client_bandwidth)
{
	return 0;
}
#endif
void topo_srv_manage_bh_links(struct own_1905_device *ctx)
{

}
#ifdef SUPPORT_MULTI_AP

void Optimized_Link_estimation_algo(struct mapd_global *pGlobal_dev)
{
	return;
}
#endif
/* ******************************************************************
* These functions are part of proprietary MTK's algorithms user for MAP R2
********************************************************************/
#ifdef MAP_R2
void ch_planning_calc_score(
	struct own_1905_device *own_dev,
	struct _1905_map_device *_1905_dev,
	struct radio_info_db *radio,
	struct scan_result_tlv *scan_res)
{
	return;
}

#endif
void topo_serv_clear_visit_node(struct own_1905_device *ctx)
{

	return;
}

void hopcnt_to_controller(struct _1905_map_device *dev, char *hopcnt, char *term)
{

	return;
}


