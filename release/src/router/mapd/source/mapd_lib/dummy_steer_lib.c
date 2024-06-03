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

u8 ZERO_MAC_ADDR[ETH_ALEN] = {0,0,0,0,0,0};

/*
 * @brief
 * Attempts to find a suitable client (from within the MAP network) for steering.
 * This function does not check if a steering is being attempted currently,
 * it is the responsibility of the caller to ensure that it is only called when 
 * no post assoc steering is in progress
 *
 * @param global: Global device pointer
 * @param arr_cand_list: Candidate List of STA suitable for steering in array format
 * @param cand_cnt: Number of Candidates in the “arr_cand_list”
 *
 */
void ap_roam_algo_select_steer_candidate(struct mapd_global *global, struct client *arr_cand_list[MAX_STA_SEEN], int *cand_cnt)
{

}

/*
 * @brief : Find target AP channel for which 11k measurement should be done 
 * for a candidate STA on the same device.
 *
 * @param global : Global device pointer
 * @param client_id : client id of the client in question
 */
void ap_roam_algo_find_candidate_channels_single(struct mapd_global *global, uint32_t client_id)
{

}

/*
 * @brief
 * determine Determine the optimal BSS without performing measurement for a client on the same device. 
 * Should only be called for clients which doesn’t require channel measurement
 *
 * @param global: global device pointer
 * @param client_id : client id of the client in question
 */
void ap_roam_algo_determine_sap_bss(struct mapd_global *global, uint32_t client_id)
{

}

/*
 * @brief
 * Check if pre-assoc steering should be initiated for a client.
 *
 * @param global : Global device pointer
 * @param client_info: Pointer to struct client for which pre-assoc steering required to check.
 */
void ap_roam_algo_chk_preassoc_str(struct mapd_global *global, struct client *client_info)
{

}

/*
 * @brief: It handles the case when channel measurement is completed for a particular client.
 * @param global: IN Global device pointer
 * @param client_info: IN Pointer to struct client for which channel measurement is completed.
 * @param status_code: OUT Fills the status code as per the next steering state returned.
 * @return: Returns the next steering state defined in enum “STEERING_STATE” as per the status of channel measurement
 */

STEERING_STATE ap_roam_algo_handle_chan_meas_complete(struct mapd_global *global, struct client *cli, int *status_code)
{
  /* Return next Steering state */
	return CLI_STATE_INVALID;
}

/*
 * @brief Find channel (for 11k measurement) for a candidate STA on the Multi-AP network
 *
 * @param pGlobal_dev pointer to the global structure
 * @param client_id client for which measurement need to be made.
 */
void ap_roam_algo_find_measurement_channel_multi(struct mapd_global *pGlobal_dev, uint32_t client_id)
{

}
/*
 * @brief This function manages the codition when steering is failed.
 * @param pGlobal_dev pointer to the global structure
 * @param client pointer to the client structure for which steering is failed.
 */
void steer_action_handle_steer_fail(struct mapd_global *global, struct client *cli)
{

}


/*
 * @brief: This function is used to handle the scenario when steering is successful.
 *
 * @@param global: pointer to the global structure
 * @param cli: Pointer to struct client for which steer is triggered
 */
void steer_action_handle_steer_success(struct mapd_global *global, struct client *cli)
{

}

/*
 * @brief This function execute Steering.
 *
 * @param global: pointer to the global structure
 * @param client_id: Client ID corresponding to client for which steer action is required
 */

void steer_action_steer_client(struct mapd_global *global, uint8_t client_id)
{

}

/*
 * @brief This function controls steering coordination.
 *
 * @param pGlobal_dev pointer to the global structure
 * @param client_id client for which preperation is done.
 *
 * @return TRUE if rfs req was sent. Else return FALSE
 */
u8 steer_action_steer_prep(struct mapd_global *pGlobal_dev, u32 client_id)
{
	return TRUE;
}

/*
 * @brief This function handles the scenario when client’s steering is completed (Success/failure both cases).
 *
 * @param pGlobal_dev pointer to the global structure
 * @param client_id client for which steering is complete
 * @param complete_data steering result (success/fail)
 */
void steer_action_handle_coord_steer_complete(struct mapd_global *pGlobal_dev, u32 client_id, COMPLETE_STATE_DATA *complete_data)
{

}

/*
 * @brief This function handles remote topology notification for a client. 
 * If steering of this client is ongoing, then change the state of this client and stop steering process.
 *
 * @param pGlobal_dev pointer to the global struct.
 * @param client_id client for which topology notification has come.
 */
void steer_action_handle_remote_topo_notif(struct mapd_global *pGlobal_dev, int client_id)
{

}

/*
 * @brief: Clears the black list on peers.
 * @param global: pointer to the global struct.
 * @param client_id: client for which topology notification has come
 * @param reason_code: non-zero value corresponding to status code mentioned for INVALID/IDLE state.
 *
 */  

void steer_action_clear_bl_on_peers(struct mapd_global *global, struct client *map_client, unsigned int reason_code)
{

}

/*
 * @brief: This function used to clear timers used for steering.
 * @param global: pointer to the global struct.
 * @param cli: pointer to struct client for which tall timers needs to be clear
 */

void steer_action_clear_all_timers(struct mapd_global *global, struct client *cli)
{

}

/*
 * @brief This function takes decision on the target steering for particular client
 *
 * @param global: pointer to the global struct
 * @param client_id: Pointer to struct client for which newly joined
 */
void steer_action_check_steer_type(struct mapd_global *global, uint32_t client_id)
{

}


/* ******************************************************************
* These functions with dummy body are meant to handle compile errors
* when CONFIG_MULTIAP_MTK_LIB macro set to 'n' in mapd_config file.
* Customers who are not using MTK's LIB shouldn't refer these functions.
* These functions are part of proprietary MTK's algorithms.
********************************************************************/
const int8_t NOISE_OFFSET_BY_CH_WIDTH[] = {
    -94, //20 Mhz
    -91, //40 Mhz
    -88, //80 Mhz
    -85, //160 Mhz
};
int8_t ap_est_get_noise_offset(struct mapd_global *global, uint8_t radio_idx)
{
	return NOISE_OFFSET_BY_CH_WIDTH[0];
}
void ap_est_update_non_serving_rssi(struct mapd_global *global, struct client *cli, uint8_t actual_radio_idx, uint8_t desired_radio_idx)
{

}
uint16_t ap_est_get_airtime(struct mapd_global *global, uint16_t ul_rate, uint16_t dl_rate, const uint16_t dl_phy_rate)
{
	return 0;
}
int steer_action_reset_csbc(struct mapd_global *global, uint32_t client_id, u8 skip_force_csbc_reset)
{
	return 0;
}
int steer_action_print_csbc_stats(struct mapd_global *global, struct client *cli, char *pos, char *end)
{
	return 0;
}
void steer_action_sta_join(struct mapd_global *global, struct client *cli, u8 is_remote)
{

}
void steer_action_csbc_init_from_db(struct mapd_global *global, struct client *cli)
{

}
void steer_action_tsq_rsp_timeout(struct mapd_global *pGlobal_dev, int client_id)
{

}
void steer_action_handle_tsq_rsp(struct mapd_global *pGlobal_dev,int client_id, struct _1905_map_device *map_1905_device, struct tsq_rsp_tlv *cli_tsq_rsp)
{

}
void steer_action_handle_tsq_req(struct mapd_global *pGlobal_dev, struct tsq_req_tlv *cli_tsq_req, struct _1905_map_device *map_1905_device)
{

}
int ap_roam_algo_disable_pre_assoc_strng(struct mapd_global *global, u8 disable)
{
	return 0;
}
int ap_roam_algo_disable_post_assoc_strng(struct mapd_global *global, u8 disable)
{
	return 0;
}
int ap_roam_algo_disable_post_assoc_strng_by_type(struct mapd_global *global, u8 disable, STEERING_METHOD_TYPE str_method)
{
	return 0;
}
void steer_action_rfs_rsp_timeout(struct mapd_global *pGlobal_dev, int client_id)
{

}
void steer_action_handle_rfs_rsp(struct mapd_global *pGlobal_dev,int client_id, struct rfs_rsp_tlv *cli_rfs_rsp, struct _1905_map_device *map_1905_device)
{

}
void steer_action_handle_rfs_req(struct mapd_global *pGlobal_dev, struct rfs_req_tlv *rfs_req, struct _1905_map_device *map_1905_device)
{

}

#ifdef SUPPORT_MULTI_AP
Boolean steer_action_is_steer_allowed(struct mapd_global *global, uint8_t client_id)
#else
Boolean steer_action_is_steer_allowed(struct mapd_global *global, uint8_t client_id, uint8_t is_steer_method_DG)
#endif
{
	return 0;
}

int steer_action_is_btm_active_str_allowed(struct mapd_global *global, struct client *cli)
{
	return 0;
}

int steer_action_is_btm_idle_str_allowed(struct mapd_global *global, struct client *cli)
{
	return 0;
}

int steer_action_is_force_str_allowed(struct mapd_global *global, struct client *cli)
{
	return 0;
}

void ap_roam_algo_get_ch_ol_safety_th(struct own_1905_device *own_device, u8 channel, u8 *ol_th, u8 *ch_safety_th)
{

}
int8_t ap_est_update_non_serving_rssi_for_bh(struct mapd_global *global,
	struct bh_link_entry *bh_entry, uint8_t cur_band, uint8_t desired_band)
{
	return 0;
}
#ifdef CENT_STR
void ap_roam_algo_find_best_bss_map_cent_str(struct mapd_global *pGlobal_dev, int client_id)
{

}
void ap_roam_algo_determine_best_bss_post_measurement_cent_str(struct mapd_global *global,
		uint32_t client_id)
{

}
void ap_roam_algo_determine_sap_bss_cent_str(struct mapd_global *global, uint32_t client_id)
{

}
int8_t ap_est_update_non_serving_rssi_cent_str(struct mapd_global *global,
         struct client *cli, uint8_t cur_band, uint8_t desired_band)
{

	return 0;
}

int8_t ap_est_get_noise_offset_by_1905_radio(struct mapd_global *global, struct radio_info_db *target_radio)
{
	return 0;
}

int topo_srv_get_rssi_th_by_policy_1905_radio(struct own_1905_device *ctx,struct radio_info_db *radio_info , int8_t *rssi)
{
	return 0;
}
#endif
