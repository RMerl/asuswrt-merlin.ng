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
                steer_fsm.c

                Abstract:
                State Machine engine containing the different transitions and data passing mechanisms

                Revision History:
                Who         When          What
                --------    ----------    -----------------------------------------
                Anirudh.S   2018/04/19		First implementation of the state machine
*/

#include "includes.h"
#include "common.h"
#include "eloop.h"
#include "client_db.h"
#include "mapd_i.h"
#ifdef SUPPORT_MULTI_AP
#include "../../1905_local_lib/data_def.h"
#include "topologySrv.h"
#endif
#include "steer_fsm.h"
#include "ap_roam_algo.h"
#include "steer_action.h"
#include "mapd_i.h"
#include "ap_est.h"
#include "client_mon.h"

u8 bcast_mac[ETH_ALEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

/* STATE MACHINE CORE FUNCTIONALITY */
/* ***************************************************************************** */

void steer_fsm_step_state(struct mapd_global *global, int client_id,
			  STEERING_STATE next_state, void *priv)
{
	STEERING_STATE previous_state =
			client_db_get_cli_steer_state(global, client_id);

	struct client *cli = client_db_get_client_from_client_id(global, client_id);

	if (!cli) {
		mapd_printf(MSG_ERROR, "Invalid client ID: %d", client_id);
		mapd_ASSERT(0);
		return;
	}

	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Prev State = %s Next"
			" State = %s", client_id, MAC2STR(cli->mac_addr),
			get_steer_state_str(previous_state),
			get_steer_state_str(next_state));

	client_db_set_cli_steer_state(global, client_id, next_state);
	switch (next_state)
	{
		case CLI_STATE_INVALID:
			steer_fsm_invalid_state_handler(global, client_id,
					previous_state, priv);
			break;
		case CLI_STATE_IDLE :
			steer_fsm_idle_state_handler(global, client_id,
						     previous_state, priv);
			break;
		case CLI_STATE_STEER_DECISION:
			steer_fsm_decision_state_handler(global, client_id,
							 previous_state, priv);
			break;
		case CLI_STATE_STEER_PREPARATION:
			steer_fsm_preparation_state_handler(global, client_id,
							    previous_state, priv);
			break;
		case CLI_STATE_STEER_EXEC_MONITOR:
			steer_fsm_exec_monitor_state_handler(global,
							     client_id,
							     previous_state,
							     priv);
			break;
		case CLI_STATE_STEER_COMPLETE:
			steer_fsm_complete_state_handler(global, client_id,
							 previous_state, priv);
			break;
		default:
			mapd_printf(MSG_ERROR, "Invalid next state");
			break;
	}


}

void steer_fsm_trigger(struct mapd_global *global, int client_id,
		       TRIGGER_TYPE trigger, void *user_data)
{

	STEERING_STATE current_state = client_db_get_cli_steer_state(global,
			client_id);
	struct client *cli = client_db_get_client_from_client_id(global, client_id);

	if (!cli) {
		mapd_printf(MSG_ERROR, "Invalid client ID: %d", client_id);
		mapd_ASSERT(0);
		return;
	}

	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Curr State = %s"
					" Trigger = %s", client_id, MAC2STR(cli->mac_addr),
					get_steer_state_str(current_state),
					get_steer_trigger_str(trigger));

	switch(current_state)
	{
		case CLI_STATE_INVALID:
			steer_fsm_invalid_trigger_handler(global, client_id,
					                               trigger, user_data);
			break;
		case CLI_STATE_IDLE :
			steer_fsm_idle_trigger_handler(global, client_id,
						       trigger, user_data);
			break;
		case CLI_STATE_STEER_DECISION:
			steer_fsm_decision_trigger_handler(global, client_id,
							   trigger, user_data);
			break;
		case CLI_STATE_STEER_PREPARATION:
			steer_fsm_preparation_trigger_handler(global,
							      client_id,
							      trigger, user_data);
			break;
		case CLI_STATE_STEER_EXEC_MONITOR:
			steer_fsm_exec_monitor_trigger_handler(global,
							       client_id, trigger,
							       user_data);
			break;
		case CLI_STATE_STEER_COMPLETE:
			steer_fsm_complete_trigger_handler(global, client_id,
							   trigger, user_data);
			break;
		default:
			mapd_printf(MSG_ERROR, "Invalid current state");
			break;

	}
}

static void steer_fsm_clear_meas_data(struct mapd_global *global, struct client *cli)
{
	struct cli_rssi *tmp = NULL, *next = NULL;
#ifdef SUPPORT_MULTI_AP
	struct bss_air_mon_report *air_mon_bss = NULL, *air_mon_bss_tmp = NULL;
#endif
	mapd_printf(MSG_DEBUG, " ");

	/* Cancel any timeouts that could be registered during DECISION state */
	ap_est_11k_cleanup(global, cli);

	/* Free dynamic dl_rssi list objects as part of 11k meas */
	if ((cli->meas_data.dl_rssi_list.next != 0) &&
		(cli->meas_data.dl_rssi_list.prev != 0)) { //List is inited
			dl_list_for_each_safe(tmp, next, &cli->meas_data.dl_rssi_list,
							struct cli_rssi, rssi_entry)
			{
					dl_list_del(&tmp->rssi_entry);
					os_free(tmp);
			}
	}
#ifdef SUPPORT_MULTI_AP
	/* Free AirMon Data collected as part of AirMon meas */
	if(!SLIST_EMPTY(&(cli->meas_data.air_mon_bss_head)))
	{
		/*delete all p1905.1 device in this interface*/
		air_mon_bss =\
		SLIST_FIRST(&(cli->meas_data.air_mon_bss_head));
		while(air_mon_bss)
		{
			air_mon_bss_tmp =\
			SLIST_NEXT(air_mon_bss, air_mon_bss_entry);

			SLIST_REMOVE(&(cli->meas_data.air_mon_bss_head),\
			air_mon_bss, bss_air_mon_report,\
			air_mon_bss_entry);
			free(air_mon_bss);
			air_mon_bss = air_mon_bss_tmp;
		}
	}
#endif
	os_memset(&cli->meas_data, 0, sizeof(struct meas_state_data));
}
#ifdef SUPPORT_MULTI_AP
/*
Clear RFS List
*/
static void steer_fsm_clear_rfs_data(struct mapd_global *global, struct client *map_client)
{
	struct coord_req_dev_list *rfs_req_dev = NULL, *rfs_req_dev_tmp = NULL;

	mapd_printf(MSG_DEBUG, " ");
	if(!SLIST_EMPTY(&(map_client->coord_data.map_coord_dev_head)))
	{
		/*delete all p1905.1 device in this interface*/
		rfs_req_dev =\
		SLIST_FIRST(&(map_client->coord_data.map_coord_dev_head));
		while(rfs_req_dev)
		{
			rfs_req_dev_tmp =\
			SLIST_NEXT(rfs_req_dev, coord_req_dev_entry);
			SLIST_REMOVE(&(map_client->coord_data.map_coord_dev_head),\
				rfs_req_dev,coord_req_dev_list,\
											coord_req_dev_entry);
			free(rfs_req_dev);
			rfs_req_dev = rfs_req_dev_tmp;
		}
	}
	map_client->coord_data.rfs_timer_running = 0;
	map_client->coord_data.tsq_timer_running = 0;
	map_client->coord_data.steer_complete_reason = 0;

	if (map_client->coord_data.cli_coordination_state != COORD_STATE_RFS_RECEIVED
		&& map_client->coord_data.cli_coordination_state != COORD_STATE_ASSOC_CONTROL_RECEIVED)
		os_memset(&map_client->coord_data, 0, sizeof(struct coord_state_data));

}
#endif
/*Clean Up Memory Allocated for Sm */
void clear_sm_data(struct mapd_global *global, struct client *cli)
{
	steer_fsm_clear_meas_data(global, cli);
#ifdef SUPPORT_MULTI_AP
	steer_fsm_clear_rfs_data(global, cli);
#endif
}

/* ***************************************************************************** */

/* STATE MACHINE NON-TRIGGER STATE HANDLERS */
/* ***************************************************************************** */

void steer_fsm_invalid_state_handler(struct mapd_global *global, int client_id,
		STEERING_STATE previous_state, void *data)
{
	struct mapd_bss *bss;
	struct client *cli;
	struct steer_cands *cand = NULL;
	INVALID_DATA *data_rx = (INVALID_DATA *)data;
	struct own_1905_device *ctx = &global->dev;
#ifdef CENT_STR
struct cent_steer_cands *cent_str_cand = NULL;

#endif

	cli = client_db_get_client_from_client_id(global, client_id);

	if (!cli) {
		mapd_printf(MSG_ERROR, "Invalid client ID: %d", client_id);
		mapd_ASSERT(0);
		return;
	}

	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Prev State = %s",
				client_id, MAC2STR(cli->mac_addr),
				get_steer_state_str(previous_state));

	switch(previous_state)
	{

		case CLI_STATE_IDLE :
			/* Steering not initiated for the client but it is no longer connected
			 * to us
			 */
			break;
		case CLI_STATE_STEER_DECISION :
			steer_fsm_clear_meas_data(global, cli);
			cand = get_client_from_steer_cand_list(ctx, cli);
			if (cand != NULL) {
				cand->steer_cand = NULL;
				SLIST_REMOVE(&(ctx->steer_cands_head), cand, steer_cands, next_cand);
				os_free(cand);
			}
#ifdef CENT_STR
			if(global->dev.cent_str_en){
				cent_str_cand = get_client_from_cent_steer_cand_list(ctx, cli);
				if (cent_str_cand != NULL) {
					struct client *cli = cent_str_cand->steer_cand;
					err("Invalid state Remove client state:%d client:"MACSTR,cli->cli_steer_state,MAC2STR(cli->mac_addr));
					cent_str_cand->steer_cand = NULL;
					STAILQ_REMOVE(&(ctx->cent_steer_cands_head), cent_str_cand, cent_steer_cands, next_cand);
					os_free(cent_str_cand);
				}
			}
#endif
			break;
		case CLI_STATE_STEER_COMPLETE :
			cand = get_client_from_steer_cand_list(ctx, cli);
			if (cand != NULL) {
				bss = mapd_get_bss_from_mac(global, cand->steer_cand_home_bssid);
				cand->steer_cand = NULL;
				SLIST_REMOVE(&(ctx->steer_cands_head), cand, steer_cands, next_cand);
				os_free(cand);
#ifdef SUPPORT_MULTI_AP
				if(global->dev.cli_steer_params.single_steer) {
#ifdef CENT_STR
					if (global->dev.cent_str_en) {
						err("Steer is complete");
						mapd_cent_str_send_steering_complete(global, bss);
					} else
#endif
					mapd_send_steering_complete(global, bss);
				}
#endif
			}
#ifdef CENT_STR
			if(global->dev.cent_str_en) {
				cent_str_cand = get_client_from_cent_steer_cand_list(ctx, cli);

				if (cent_str_cand != NULL) {
					struct client *cli = cent_str_cand->steer_cand;
					err("Complete state Remove client state:%d client:"MACSTR,cli->cli_steer_state,MAC2STR(cli->mac_addr));
					cent_str_cand->steer_cand = NULL;
					STAILQ_REMOVE(&(ctx->cent_steer_cands_head), cent_str_cand, cent_steer_cands, next_cand);
					os_free(cent_str_cand);
				}
			}
#endif
			break;
		default:
			mapd_printf(MSG_ERROR, "Transitioning to the invalid state incorrectly!"
					"Something is wrong");
			mapd_ASSERT(0);
	}
	if(cli){
		if(data_rx != NULL)
#ifdef SUPPORT_MULTI_AP
			steer_action_clear_bl_on_peers(global, cli, data_rx->exit_code);
#else
			mapd_printf(MSG_DEBUG, "data_rx->exit_code:%d \n", data_rx->exit_code);
#endif

		clear_sm_data(global, cli);
	}

}



void steer_fsm_idle_state_handler(struct mapd_global *global, int client_id,
				  STEERING_STATE previous_state, void *data)
{
	IDLE_DATA *data_rx = (IDLE_DATA *) data;
	struct mapd_bss *bss;
	struct client *cli = NULL;
	struct steer_cands *cand = NULL;
	struct own_1905_device *ctx = &global->dev;
#ifdef CENT_STR
struct cent_steer_cands *cent_str_cand = NULL;

#endif

	cli = client_db_get_client_from_client_id(global, client_id);

	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Prev State = %s",
				client_id, MAC2STR(cli->mac_addr),
				get_steer_state_str(previous_state));

	if(data_rx == NULL) {
		mapd_printf(MSG_INFO, "NULL data passed as state data");
	}
	else {
		mapd_printf(MSG_INFO, "The return to idle status code is %d",
				data_rx->exit_code);
		if (data_rx->exit_code != IDLE_SUCCESS)
			mapd_printf(MSG_INFO, "Idle state reached without success");
	}

	switch(previous_state)
	{

		case CLI_STATE_INVALID :
			cli->steer_retry_time = global->dev.cli_steer_params.MinSteerRetryTime;
			cli->steer_retry_step = 0;
			/*  When a client connects, wait for atleast StrForbidTimeJoin seconds before
				considering for steer. */
			if (eloop_replenish_timeout(global->dev.cli_steer_params.StrForbidTimeJoin,
							0, steer_action_handle_prohibit_timeout, global, cli) == -1) {
				if (global->dev.cli_steer_params.StrForbidTimeJoin > 0)
					eloop_register_timeout(global->dev.cli_steer_params.StrForbidTimeJoin,
							0, steer_action_handle_prohibit_timeout,
							global, cli);
			}
			break;
		case CLI_STATE_STEER_PREPARATION :
		case CLI_STATE_STEER_DECISION :
			cand = get_client_from_steer_cand_list(ctx, cli);
			if (cand != NULL) {
				{
				bss = mapd_get_bss_from_mac(global, cand->steer_cand_home_bssid);
				if (previous_state == CLI_STATE_STEER_DECISION) {
					mapd_printf(MSG_INFO, "%s attempt could not go past "
								"DECISION state",
								str_method_str(cli->cli_steer_method));
					cli->str_mthds_failed_in_dec |= BIT(cli->cli_steer_method);
				}
				cand->steer_cand = NULL;
				SLIST_REMOVE(&(ctx->steer_cands_head), cand, steer_cands, next_cand);
				os_free(cand);
#ifdef SUPPORT_MULTI_AP
				if(global->dev.cli_steer_params.single_steer) {
#ifdef CENT_STR
					if (global->dev.cent_str_en) {
						err("Steer is complete");
						mapd_cent_str_send_steering_complete(global, bss);
					} else
#endif
					mapd_send_steering_complete(global, bss);
				}
#endif
			}
			}
#ifdef CENT_STR
			if(ctx->cent_str_en){
				cent_str_cand = get_client_from_cent_steer_cand_list(ctx, cli);

				if(cent_str_cand){
					if (previous_state == CLI_STATE_STEER_DECISION) {
						mapd_printf(MSG_INFO, "%s attempt could not go past "
									"DECISION state",
									str_method_str(cli->cli_steer_method));
						cli->str_mthds_failed_in_dec |= BIT(cli->cli_steer_method);
					}
					struct client *cli = cent_str_cand->steer_cand;
					err("Idle state from desc/prep Remove client state:%d client:"MACSTR,cli->cli_steer_state,MAC2STR(cli->mac_addr));

					cent_str_cand->steer_cand = NULL;
					STAILQ_REMOVE(&(ctx->cent_steer_cands_head), cent_str_cand, cent_steer_cands, next_cand);
					os_free(cent_str_cand);

				}
			}
#endif
			break;
		case CLI_STATE_STEER_COMPLETE :
			cand = get_client_from_steer_cand_list(ctx, cli);
			if (cand != NULL) {
				bss = mapd_get_bss_from_mac(global, cand->steer_cand_home_bssid);
				cand->steer_cand = NULL;
				SLIST_REMOVE(&(ctx->steer_cands_head), cand, steer_cands, next_cand);
				os_free(cand);
#ifdef SUPPORT_MULTI_AP
				if(global->dev.cli_steer_params.single_steer) {
#ifdef CENT_STR
					if (global->dev.cent_str_en) {
						err("Steer is complete");
						mapd_cent_str_send_steering_complete(global, bss);
					} else
#endif
					mapd_send_steering_complete(global, bss);
				}
#endif
			}
#ifdef CENT_STR
		if(ctx->cent_str_en) {
			cent_str_cand = get_client_from_cent_steer_cand_list(ctx, cli);
			if (cent_str_cand != NULL) {
				struct client *cli = cent_str_cand->steer_cand;
				err("Idle state from Complete state Remove client state:%d client:"MACSTR,cli->cli_steer_state,MAC2STR(cli->mac_addr));

				cent_str_cand->steer_cand = NULL;
				STAILQ_REMOVE(&(ctx->cent_steer_cands_head), cent_str_cand, cent_steer_cands, next_cand);
				os_free(cent_str_cand);
			}
		}
#endif
			break;
		default:
			mapd_printf(MSG_ERROR, "Transitioning to the idle state incorrectly!"
					"Something is wrong");
			mapd_ASSERT(0);
	}
	if(cli){
		if(data_rx != NULL)
#ifdef SUPPORT_MULTI_AP
			steer_action_clear_bl_on_peers(global, cli, data_rx->exit_code);
#else
			mapd_printf(MSG_DEBUG, "data_rx->exit_code:%d \n", data_rx->exit_code);
#endif

		clear_sm_data(global, cli);
	}
}

static void steer_fsm_init_meas_data(struct mapd_global *global,
				struct client *cli, uint8_t is_multi)
{
	mapd_printf(MSG_DEBUG, "Client(%d) " MACSTR " is_multi=%d",
					cli->client_id, MAC2STR(cli->mac_addr), is_multi);

	os_memset(&cli->meas_data, 0, sizeof(struct meas_state_data));
	dl_list_init(&cli->meas_data.dl_rssi_list);
	SLIST_INIT(&(cli->meas_data.air_mon_bss_head));
	if (is_multi) {
#ifdef SUPPORT_MULTI_AP
		ap_roam_algo_find_measurement_channel_multi(global, cli->client_id);
#endif
	} else {
		ap_roam_algo_find_candidate_channels_single(global, cli->client_id);
	}
}

void steer_fsm_decision_state_handler(struct mapd_global *global,
				      int client_id, STEERING_STATE previous_state,
				      void *data)
{

	//switch statement on the basis of previous state is again unnecessary here
	DECISION_DATA *data_rx = (DECISION_DATA *) data;
	PREPARATION_DATA data_tx_prep;
	IDLE_DATA data_tx_idle ;
	struct client *cli = &global->dev.client_db[client_id];
#ifdef SUPPORT_MULTI_AP
	u8 bss[ETH_ALEN];
#endif
	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Prev State = %s SM= %s",
					client_id, MAC2STR(cli->mac_addr),
					get_steer_state_str(previous_state),
					str_method_str(data_rx->steer_method));

	/* Note the TS */
	os_get_reltime(&cli->steer_cand_ts);

	if (previous_state == CLI_STATE_STEER_DECISION) {
		mapd_printf(MSG_INFO, "***RE-ENTERED ForceAirMon=%d SteerMethod=%s****",
				   cli->force_airmon, str_method_str(cli->cli_steer_method));
		steer_fsm_clear_meas_data(global, cli);
	}

	switch(data_rx->steer_method)
	{
#ifdef SUPPORT_MULTI_AP
		case MANDATE :
			/* XXX:wildcard will be taken care of using one of the other triggers
			we will not tell the state machine that a wildcard mandate req
			has been obtained
			*/
			{
				os_memcpy(bss, mapd_get_target_mandate_bssid(global, client_id), ETH_ALEN);
				os_memcpy(cli->exec_mon_data.target_bssid, bss, ETH_ALEN);
				make_preparation_data(&data_tx_prep, bss);
				steer_fsm_step_state(global, client_id,
						     CLI_STATE_STEER_PREPARATION,
						     (void *) &data_tx_prep);
				break;
			}
		case OFFLOADING :
		case NOL_MULTIAP :
			steer_fsm_init_meas_data(global, cli, 1);
			if (cli->meas_data.meas_chan_cnt < 1) {
					mapd_printf(MSG_ERROR, "**** No Alternative BSSs(overloaded bss / "
									"ESS mismatch / chan not supp ****");
					make_idle_data(&data_tx_idle,
									IDLE_BSS_NOT_FOUND);
					steer_fsm_step_state(global, cli->client_id,
									CLI_STATE_IDLE,
									(void*) &data_tx_idle);
					return;
			}
			if (ap_est_trigger_channel_measurement(global, client_id, data_rx) == FALSE) {
					mapd_printf(MSG_ERROR, "Could not start channel meas");
					make_idle_data(&data_tx_idle,
									IDLE_CHAN_MEAS_FAILED);
					steer_fsm_step_state(global, cli->client_id,
									CLI_STATE_IDLE,
									(void*) &data_tx_idle);
			}
			mapd_printf(MSG_INFO, "Channel meas started succesfully(waiting for result)");
			break;
#endif
		case IDLE_STANDALONE_DG:
		case IDLE_STANDALONE_UG:
		case IDLE_5GH_TO_5GL:
		case IDLE_5GL_TO_5GH:
				/* XXX: We could split the following function for the different cases
				  we have caused it to fall through, leading to no redundant
				  checking of steer method again in ap_roam_algo_determine_sap_bss
				 */
#ifdef CENT_STR
			if(global->dev.cent_str_en)
				ap_roam_algo_determine_sap_bss_cent_str(global, client_id);
			else
#endif
				ap_roam_algo_determine_sap_bss(global, client_id);

			if (is_zero_ether_addr(cli->exec_mon_data.target_bssid))
			{
				mapd_printf(MSG_INFO, "No suitable BSS found, going to the idle state\n");
				make_idle_data(&data_tx_idle,
								IDLE_BSS_NOT_FOUND);
				steer_fsm_step_state(global, client_id,
								CLI_STATE_IDLE,
								(void*) &data_tx_idle);
			}
			else
			{
				make_preparation_data(&data_tx_prep, cli->exec_mon_data.target_bssid);
				steer_fsm_step_state(global, client_id,
						CLI_STATE_STEER_PREPARATION,
						(void *) &data_tx_prep);
			}


			break;
		case ACTIVE_STANDALONE_DG:
		case ACTIVE_STANDALONE_UG:
		case ACTIVE_5GL_TO_5GH:
		case ACTIVE_5GH_TO_5GL:
#ifndef SUPPORT_MULTI_AP
		case OFFLOADING:
#endif
			steer_fsm_init_meas_data(global, cli, 0);
#ifndef SUPPORT_MULTI_AP
			mapd_ASSERT(cli->meas_data.meas_chan_cnt < 1);
#endif
			if (ap_est_trigger_11k_channel_measurement(global, cli) == FALSE) {
					mapd_printf(MSG_ERROR, "Could not start channel meas");
					make_idle_data(&data_tx_idle,
									IDLE_CHAN_MEAS_FAILED);
					steer_fsm_step_state(global, cli->client_id,
									CLI_STATE_IDLE,
									(void*) &data_tx_idle);
			}
			break;
		default :
			mapd_printf(MSG_ERROR, "Invalid Steer method");
	}
}
void steer_fsm_preparation_state_handler(struct mapd_global *global,
					 int client_id,
					 STEERING_STATE previous_state, void *data)
{
	EXEC_MONITOR_DATA data_tx_exec_mon;
	struct client *map_client = client_db_get_client_from_client_id(global,client_id);

	if(map_client == NULL) {
		mapd_printf(MSG_ERROR, "Invalid client ID: %d", client_id);
		mapd_ASSERT(0);
		return;
	}

	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Prev State = %s",
				client_id, MAC2STR(map_client->mac_addr),
				get_steer_state_str(previous_state));

	/* Clean decision State Data */
	if (previous_state == CLI_STATE_STEER_DECISION) {
		steer_fsm_clear_meas_data(global, map_client);
		map_client->str_mthds_failed_in_dec = 0;
	}

	/* This function decides the type of steering to be triggerd */
	steer_action_check_steer_type(global, client_id);
#ifdef SUPPORT_MULTI_AP
	if (steer_action_steer_prep(global, client_id) == FALSE)
#endif
	{
		make_exec_monitor_data(&data_tx_exec_mon,map_client->exec_mon_data.target_bssid);
		steer_fsm_step_state(global, client_id,
				     CLI_STATE_STEER_EXEC_MONITOR,
				     (void*) &data_tx_exec_mon);
	}
}

void steer_fsm_exec_monitor_state_handler(struct mapd_global *global,
					  int client_id,
					  STEERING_STATE previous_state, void *data)
{
	struct mapd_radio_info *radio_info;
	struct mapd_bss *bss;
	//NM_REV Use clientDB API
	struct client *cli = &global->dev.client_db[client_id];
	uint8_t idx = 0, num_bss = 0;
	uint8_t exclude_bss_list[MAX_NUM_OF_RADIO * 16][ETH_ALEN] = {{0}};

	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Prev State = %s",
				client_id, MAC2STR(cli->mac_addr),
				get_steer_state_str(previous_state));

	//perform blacklist on our own system BSS
	for(idx = 0; idx < MAX_NUM_OF_RADIO; ++idx)
	{
		radio_info = &global->dev.dev_radio_info[idx];
		if (radio_info->radio_idx == (uint8_t)-1)
			continue;
		dl_list_for_each(bss, &radio_info->bss_list, struct mapd_bss, bss_entry)
		{
			if((!os_memcmp(bss->bssid, cli->exec_mon_data.target_bssid, ETH_ALEN))||
					(!os_memcmp(bss->bssid, cli->bssid, ETH_ALEN)))
			{
				os_memcpy(exclude_bss_list[num_bss++], bss->bssid, ETH_ALEN);
			}
		}
	}
	mapd_printf(MSG_INFO, "START Prepare for steering on self");
	for (idx = 0; idx < num_bss; idx++)
			mapd_printf(MSG_INFO, "Exclude " MACSTR, MAC2STR(exclude_bss_list[idx]));

	client_mon_bl_sta_for_all_bss(global, cli, num_bss, exclude_bss_list, 1);
	mapd_printf(MSG_INFO, "Prepare DONE for steering on self");
	steer_action_steer_client(global, client_id);

}

void steer_fsm_complete_state_handler(struct mapd_global *global,
				      int client_id,
				      STEERING_STATE previous_state, void *data)
{
	struct client *cli = &global->dev.client_db[client_id];
	COMPLETE_STATE_DATA *data_rx = (COMPLETE_STATE_DATA*) data;

	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Prev State = %s Reason_code = %d",
				client_id, MAC2STR(cli->mac_addr),
				get_steer_state_str(previous_state),
				data_rx->reason_code);

	switch(data_rx->reason_code)
	{
		case COMPLETE_SUCCESS :
#ifdef SUPPORT_MULTI_AP
		case COMPLETE_FAIL_MANDATE :
		case COMPLETE_FAIL_REJ_ANOTHER_DEVICE :
#endif
		case COMPLETE_FAIL_AUTHDENYMAX_EXCEEDED:
		case COMPLETE_FAIL_LOW_RSSI_ON_TARGET:
		case COMPLETE_FAIL_STA_REJECTED_BTM_REQUEST:
		case COMPLETE_FAIL_BTM_RESPONSE_TIMEOUT:
		case COMPLETE_FAIL_WRONG_BSS:
		case COMPLETE_FAIL_ASSOC_TIMEOUT:
		case COMPLETE_FAIL_UNEXPECTED_TRIGGER:
			steer_action_handle_coord_steer_complete(global, client_id,data_rx);
#ifdef CENT_STR
			if(global->dev.cent_str_en){
				if(data_rx->reason_code != COMPLETE_SUCCESS && cli->cli_steer_method != NOL_MULTIAP){
					cli->steer_stats.steer_band_steer_fail_cnt++;
				} else if(data_rx->reason_code == COMPLETE_SUCCESS && cli->cli_steer_method != NOL_MULTIAP){
					cli->steer_stats.steer_band_steer_success_cnt++;
				}
			}
#endif
			break;
		default :
			mapd_printf(MSG_ERROR, "Invalid Reason code");


	}
}
/* ***************************************************************************** */

/* STATE MACHINE TRIGGER STATE HANDLERS */
/* ***************************************************************************** */
void steer_fsm_invalid_trigger_handler(struct mapd_global *global, int client_id,
		TRIGGER_TYPE trigger, void *user_data)
{
	struct client *cli = client_db_get_client_from_client_id(global, client_id);

	if (!cli) {
		mapd_printf(MSG_ERROR, "Invalid client ID: %d", client_id);
		mapd_ASSERT(0);
		return;
	}

	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Trigger = %s",
				client_id, MAC2STR(cli->mac_addr),
				get_steer_trigger_str(trigger));

	switch(trigger)
	{
		case CLIENT_ASSOCIATED :
			steer_fsm_step_state(global, client_id, CLI_STATE_IDLE, NULL);
			break;
		default :
		mapd_printf(MSG_ERROR, "unexpected trigger obtained for a client in the invalid state");
	}

}

void steer_fsm_idle_trigger_handler(struct mapd_global *global, int client_id,
				    TRIGGER_TYPE trigger, void *user_data)
{
	struct client *cli = client_db_get_client_from_client_id(global, client_id);

	if (!cli) {
		mapd_printf(MSG_ERROR, "Invalid client ID: %d", client_id);
		mapd_ASSERT(0);
		return;
	}

	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Trigger = %s",
				client_id, MAC2STR(cli->mac_addr),
				get_steer_trigger_str(trigger));


	DECISION_DATA data_tx_decision;

	switch(trigger)
	{
#ifdef SUPPORT_MULTI_AP
		case MANDATE_TRIGGER :
		       make_decision_data(&data_tx_decision, MANDATE);
		       break;
#endif
		case OFFLOADING_TRIGGER :
		       make_decision_data(&data_tx_decision, OFFLOADING);
		       break;
#ifdef SUPPORT_MULTI_AP
		case NOL_MULTIAP_TRIGGER :
		       make_decision_data(&data_tx_decision, NOL_MULTIAP);
		       break;
#endif
		case IDLE_STANDALONE_DG_TRIGGER :
		       make_decision_data(&data_tx_decision, IDLE_STANDALONE_DG);
		       break;
		case IDLE_STANDALONE_UG_TRIGGER :
		       make_decision_data(&data_tx_decision, IDLE_STANDALONE_UG);
		       break;
		case ACTIVE_STANDALONE_DG_TRIGGER :
		       make_decision_data(&data_tx_decision, ACTIVE_STANDALONE_DG);
		       break;
		case ACTIVE_STANDALONE_UG_TRIGGER :
			   make_decision_data(&data_tx_decision, ACTIVE_STANDALONE_UG);
		       break;
		case IDLE_STANDALONE_5GL_TO_5GH_TRIGGER :
		       make_decision_data(&data_tx_decision, IDLE_5GL_TO_5GH);
		       break;
		case IDLE_STANDALONE_5GH_TO_5GL_TRIGGER :
		       make_decision_data(&data_tx_decision, IDLE_5GH_TO_5GL);
		       break;
		case ACTIVE_STANDALONE_5GL_TO_5GH_TRIGGER :
		       make_decision_data(&data_tx_decision, ACTIVE_5GL_TO_5GH);
		       break;
		case ACTIVE_STANDALONE_5GH_TO_5GL_TRIGGER :
		       make_decision_data(&data_tx_decision, ACTIVE_5GH_TO_5GL);
		       break;
#ifdef SUPPORT_MULTI_AP
		case RFS_SUCCESS :
		{
			RFS_RSP_SM_DATA *rfs_rsp_data = user_data;
			steer_action_handle_rfs_rsp(global,client_id,&rfs_rsp_data->cli_rfs_rsp,rfs_rsp_data->map_1905_device);
			return;
		}
		case REMOTE_TOPOLOGY_NOTIFICATION:
			steer_action_handle_remote_topo_notif(global, client_id);
			return;
#endif
		case CLIENT_ASSOCIATED:
			return;
		case CLIENT_DISSOCIATED:
			steer_fsm_step_state(global, client_id, CLI_STATE_INVALID, NULL);
			return;
		case AUTH_DENY_MAX_REACHED:
			return;
		case BTM_SUCCESS :
		case BTM_FAILURE:
			return;
		default :
			mapd_printf(MSG_ERROR, "Invalid trigger");
			return;
	}
	steer_fsm_step_state(global, client_id, CLI_STATE_STEER_DECISION,
			     (void *) &data_tx_decision);
}

void steer_fsm_decision_trigger_handler(struct mapd_global *global,
					int client_id, TRIGGER_TYPE trigger,
					void *user_data)
{
	PREPARATION_DATA data_tx_preparation;
	DECISION_DATA data_tx_decision;
	IDLE_DATA data_tx_idle;
#if 0
	STEERING_METHOD_TYPE steer_method;
#endif
	struct client *cli = &global->dev.client_db[client_id];
	int next_state = 0;
	int status_code = 0;

	if (!cli) {
		mapd_printf(MSG_ERROR, "Invalid client ID: %d", client_id);
		mapd_ASSERT(0);
		return;
	}

	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Trigger = %s",
				client_id, MAC2STR(cli->mac_addr),
				get_steer_trigger_str(trigger));

	switch (trigger)
	{
#ifdef SUPPORT_MULTI_AP
		case MANDATE_TRIGGER :
			cli->meas_data.cli_measurement_state = MEAS_STATE_IDLE;
			make_idle_data(&data_tx_idle, IDLE_MANDATE_RECEIVED);
			steer_fsm_step_state(global, client_id,
					     CLI_STATE_IDLE,
					     (void*) &data_tx_decision);
			break;
#endif
		case CHAN_MEASUREMENT_COMPLETE:
			next_state = ap_roam_algo_handle_chan_meas_complete(global, cli, &status_code);
			cli->meas_data.cli_measurement_state = MEAS_STATE_IDLE;
			switch (next_state) {
				case CLI_STATE_IDLE:
						make_idle_data(&data_tx_idle,
										status_code);
						steer_fsm_step_state(global,
										client_id,
										CLI_STATE_IDLE,
										(void *) &data_tx_idle);
						break;
				case CLI_STATE_STEER_PREPARATION:
						make_preparation_data(
										&data_tx_preparation,
										cli->exec_mon_data.target_bssid);
						steer_fsm_step_state(global,
										client_id,
										CLI_STATE_STEER_PREPARATION,
										(void*)  &data_tx_preparation);
						break;
				case CLI_STATE_STEER_DECISION:
						make_decision_data(&data_tx_decision,
										cli->cli_steer_method);
						steer_fsm_step_state(global,
										client_id,
										CLI_STATE_STEER_DECISION,
										(void *) &data_tx_decision);
						break;
				default:
						mapd_ASSERT(0);
			}
			break;
		case CHAN_MEASUREMENT_FAIL:
			err("Chan Meas Failed");
			cli->meas_data.cli_measurement_state = MEAS_STATE_IDLE;
			make_idle_data(&data_tx_idle,
							IDLE_CHAN_MEAS_FAILED);
			steer_fsm_step_state(global, cli->client_id,
							CLI_STATE_IDLE,
									(void*) &data_tx_idle);
			break;

		case CHAN_MEASUREMENT_DISALLOWED:
			err("Chan Meas disallowed as radar/channel planning/network opt is ongoing");
			cli->meas_data.cli_measurement_state = MEAS_STATE_IDLE;
			make_idle_data(&data_tx_idle,
							IDLE_CHAN_MEAS_DISALLOWED);
			steer_fsm_step_state(global, cli->client_id,
							CLI_STATE_IDLE,
									(void*) &data_tx_idle);
			break;
#ifdef SUPPORT_MULTI_AP
		case REMOTE_TOPOLOGY_NOTIFICATION:
			{

				steer_action_handle_remote_topo_notif(global, client_id);
			}
			break;
		case RFS_SUCCESS:
		{
			RFS_RSP_SM_DATA *rfs_rsp_data = user_data;
			steer_action_handle_rfs_rsp(global,client_id,&rfs_rsp_data->cli_rfs_rsp,rfs_rsp_data->map_1905_device);
		}
			break;
#endif
		case CLIENT_ASSOCIATED:
			make_idle_data(&data_tx_idle, IDLE_UNEXPECTED_TRIGGER);
			steer_fsm_step_state(global, client_id, CLI_STATE_IDLE, (void *)&data_tx_idle);
			break;
		case CLIENT_DISSOCIATED:
			steer_fsm_step_state(global, client_id, CLI_STATE_INVALID, NULL);
			break;
		default :
			mapd_printf(MSG_ERROR, "Invalid Trigger");

	}

}


#if 1  // Raghav

void steer_fsm_preparation_trigger_handler(struct mapd_global *global,
					   int client_id, TRIGGER_TYPE trigger,
					   void *user_data)
{
	COMPLETE_STATE_DATA data_tx_complete;
	struct client *cli = &global->dev.client_db[client_id];

	if (!cli) {
		mapd_printf(MSG_ERROR, "Invalid client ID: %d", client_id);
		mapd_ASSERT(0);
		return;
	}

	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Trigger = %s",
				client_id, MAC2STR(cli->mac_addr),
				get_steer_trigger_str(trigger));

	switch(trigger)
	{
#ifdef SUPPORT_MULTI_AP
		case MANDATE_TRIGGER :
			make_complete_data(&data_tx_complete, COMPLETE_FAIL_MANDATE);
			steer_fsm_step_state(global, client_id,
					     CLI_STATE_STEER_COMPLETE,
					     (void*) &data_tx_complete);
			break;
		case RFS_FAILURE :
		case RFS_SUCCESS :
			{
				RFS_RSP_SM_DATA *rfs_rsp_data = user_data;
				steer_action_handle_rfs_rsp(global,client_id,&rfs_rsp_data->cli_rfs_rsp,rfs_rsp_data->map_1905_device);

			//This function must also be responsible for the state
			// transition to EXEC_MONITOR state when all RFS responses
			// have been obtained correctly
			}
			break;

		case RFS_TIMEOUT :

			steer_action_rfs_rsp_timeout(global,client_id);

			//This function must also be responsible for the timeout
			// handling and accounting for a limited no. of retries

			break;
		case REMOTE_TOPOLOGY_NOTIFICATION:
			{

				steer_action_handle_remote_topo_notif(global, client_id);
			}
			break;
#endif
		case CLIENT_ASSOCIATED:
		case CLIENT_DISSOCIATED:
			make_complete_data(&data_tx_complete, COMPLETE_FAIL_UNEXPECTED_TRIGGER);
			steer_fsm_step_state(global, client_id, CLI_STATE_STEER_COMPLETE, (void *)&data_tx_complete);
			break;
		default :
			mapd_printf(MSG_ERROR, "Invalid Trigger");
	}
}
#endif


void steer_fsm_exec_monitor_trigger_handler(struct mapd_global *global,
					    int client_id, TRIGGER_TYPE trigger,
					    void *user_data)
{
	COMPLETE_STATE_DATA data_tx_complete;
	struct client *cli = &global->dev.client_db[client_id];
#ifdef SUPPORT_MULTI_AP
	char status_success[]="BTM Success";
#endif
	char status_fail[]="BTM Fail";

	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Trigger = %s",
				client_id, MAC2STR(cli->mac_addr),
				get_steer_trigger_str(trigger));

	switch(trigger)
	{
		case BTM_SUCCESS :
			break;
		case BTM_FAILURE:
			steer_action_handle_steer_fail(global, cli);
			client_mon_bl_sta_for_all_bss(global, cli, 0, NULL, FALSE);
			make_complete_data(&data_tx_complete, COMPLETE_FAIL_STA_REJECTED_BTM_REQUEST);
			steer_fsm_step_state(global, client_id,
					     CLI_STATE_STEER_COMPLETE,
					     (void*) &data_tx_complete);
			break;
		case BTM_TIMEOUT:
			break;
		case CLIENT_ASSOCIATED :
#ifdef SUPPORT_MULTI_AP
		case REMOTE_TOPOLOGY_NOTIFICATION :
			if (!os_memcmp(cli->bssid, cli->exec_mon_data.target_bssid, ETH_ALEN))
			{
				steer_action_handle_steer_success(global, cli);
				os_memset(cli->exec_mon_data.target_bssid, 0, ETH_ALEN);
				client_mon_bl_sta_for_all_bss(global, cli, 0, NULL, FALSE);
				make_complete_data(&data_tx_complete, COMPLETE_SUCCESS);
				steer_fsm_step_state(global, client_id,CLI_STATE_STEER_COMPLETE,
					     (void *) &data_tx_complete);
				Write_Steer_Status(status_success);
			}
			else
			{
				steer_action_handle_steer_fail(global, cli);
				//TODO : Change below to take care fo remote leave
				client_mon_bl_sta_for_all_bss(global, cli, 0, NULL, FALSE);
				make_complete_data(&data_tx_complete, COMPLETE_FAIL_WRONG_BSS);
				steer_fsm_step_state(global, client_id,
						CLI_STATE_STEER_COMPLETE,
						(void*) &data_tx_complete);
				Write_Steer_Status(status_fail);
			}

			break;
#endif
		case CLIENT_ASSOCIATION_TIMEOUT :
			steer_action_handle_steer_fail(global, cli);
			client_mon_bl_sta_for_all_bss(global, cli, 0, NULL, FALSE); //XXX: Move this to complete?
			make_complete_data(&data_tx_complete, COMPLETE_FAIL_ASSOC_TIMEOUT);
			steer_fsm_step_state(global, client_id,
					     CLI_STATE_STEER_COMPLETE,
					     (void*) &data_tx_complete);
			Write_Steer_Status(status_fail);
			break;
		case CLIENT_DISSOCIATED:
			mapd_printf(MSG_INFO, "*****Client got dissociated****");
			//TODO transit to complete?
			break;
		case AUTH_DENY_MAX_REACHED:
			steer_action_handle_steer_fail(global, cli);
			client_mon_bl_sta_for_all_bss(global, cli, 0, NULL, FALSE);  //XXX: Move this to complete ?
			make_complete_data(&data_tx_complete, COMPLETE_FAIL_AUTHDENYMAX_EXCEEDED);
			steer_fsm_step_state(global, client_id,
							CLI_STATE_STEER_COMPLETE,
							(void*) &data_tx_complete);
			Write_Steer_Status(status_fail);
			break;
		default :
			mapd_printf(MSG_ERROR, "Invalid Trigger");
	}
}

void steer_fsm_complete_trigger_handler(struct mapd_global *global,
					 int client_id, TRIGGER_TYPE trigger,
					 void *user_data)
{
	IDLE_DATA data_tx_idle;
	struct client *cli = &global->dev.client_db[client_id];

	if (!cli) {
		mapd_printf(MSG_ERROR, "Invalid client ID: %d", client_id);
		mapd_ASSERT(0);
		return;
	}

	mapd_printf(MSG_INFO, "Client(%d) " MACSTR " Trigger = %s",
				client_id, MAC2STR(cli->mac_addr),
				get_steer_trigger_str(trigger));

	switch(trigger)
	{
#ifdef SUPPORT_MULTI_AP
		case TSQ_SUCCESS :
		case TSQ_FAILURE :
		{
			TSQ_RSP_SM_DATA *tsq_rsp = user_data;

			steer_action_handle_tsq_rsp(global,client_id, tsq_rsp->map_1905_device, &(tsq_rsp->cli_tsq_rsp));
		}
		break;
		case TSQ_TIMEOUT :
			steer_action_tsq_rsp_timeout(global, client_id);
			break;
		case RFS_SUCCESS:
		{
			RFS_RSP_SM_DATA *rfs_rsp_data = user_data;
			steer_action_handle_rfs_rsp(global,client_id,&rfs_rsp_data->cli_rfs_rsp,rfs_rsp_data->map_1905_device);
		}
			break;
		case REMOTE_TOPOLOGY_NOTIFICATION:
			{

				steer_action_handle_remote_topo_notif(global, client_id);
			}
			break;
#endif
		case CLIENT_ASSOCIATED:
			make_idle_data(&data_tx_idle, IDLE_UNEXPECTED_TRIGGER);
			steer_fsm_step_state(global, client_id, CLI_STATE_IDLE, (void *)&data_tx_idle);
			break;
		case CLIENT_DISSOCIATED:
			steer_fsm_step_state(global, client_id, CLI_STATE_INVALID, NULL);
			break;
		default:
			mapd_printf(MSG_ERROR, "Invalid Trigger");
	}
}



/* ***************************************************************************** */

/* HELPER FUNCTIONS FOR THE STATE MACHINE DATA PASSING */
/* ***************************************************************************** */


void make_idle_data(IDLE_DATA *data, IDLE_STATUS_CODE exit_code)
{
	data->exit_code = exit_code;
}

void make_decision_data(DECISION_DATA *data, STEERING_METHOD_TYPE steer_method)
{
	data->steer_method = steer_method;
}

void make_preparation_data(PREPARATION_DATA *data, u8 *bssid)
{
	memcpy(data->bss, bssid, ETH_ALEN);
}

void make_exec_monitor_data(EXEC_MONITOR_DATA *data, u8 *bssid)
{
	memcpy(data->bss, bssid,ETH_ALEN);
}


void make_complete_data(COMPLETE_STATE_DATA *data, COMPLETE_STATUS_CODE reason_code)
{
	data->reason_code = reason_code;
}


/* ***************************************************************************** */


/* GENERIC HELPER FUNCTIONS */
/******************************************************************************* */

const char *get_steer_state_str(STEERING_STATE steer_state)
{
	switch(steer_state)
	{
		case CLI_STATE_INVALID:
			return "CLI_STATE_INVALID";
		case CLI_STATE_IDLE:
			return "CLI_STATE_IDLE";
		case CLI_STATE_STEER_DECISION:
			return "CLI_STATE_STEER_DECISION";
		case CLI_STATE_STEER_PREPARATION:
			return "CLI_STATE_STEER_PEPARATION";
		case CLI_STATE_STEER_EXEC_MONITOR:
			return "CLI_STATE_STEER_EXEC_MONITOR";
		case CLI_STATE_STEER_COMPLETE:
			return "CLI_STATE_STEER_COMPLETE";
		default:
			return "Unrecognized steering state";
	}
}

const char *get_steer_trigger_str(TRIGGER_TYPE type)
{
	switch(type)
	{
    case OFFLOADING_TRIGGER:
		return "OFFLOADING_TRIGGER";
    case ACTIVE_STANDALONE_DG_TRIGGER:
		return "ACTIVE_STANDALONE_DG_TRIGGER";
    case IDLE_STANDALONE_UG_TRIGGER:
		return "IDLE_STANDALONE_UG_TRIGGER";
	case IDLE_STANDALONE_DG_TRIGGER:
		return "IDLE_STANDALONE_DG_TRIGGER";
    case ACTIVE_STANDALONE_UG_TRIGGER:
		return "ACTIVE_STANDALONE_UG_TRIGGER";
#ifdef SUPPORT_MULTI_AP
    case NOL_MULTIAP_TRIGGER:
		return "NOL_MULTIAP_TRIGGER";
#endif
    case IDLE_STANDALONE_5GL_TO_5GH_TRIGGER:
		return "IDLE_STANDALONE_5GL_TO_5GH_TRIGGER";
    case IDLE_STANDALONE_5GH_TO_5GL_TRIGGER:
		return "IDLE_STANDALONE_5GH_TO_5GL_TRIGGER";
    case ACTIVE_STANDALONE_5GL_TO_5GH_TRIGGER:
		return "ACTIVE_STANDALONE_5GL_TO_5GH_TRIGGER";
    case ACTIVE_STANDALONE_5GH_TO_5GL_TRIGGER:
		return "ACTIVE_STANDALONE_5GH_TO_5GL_TRIGGER";
    case CHAN_MEASUREMENT_COMPLETE:
		return "CHAN_MEASUREMENT_COMPLETE";
    case CHAN_MEASUREMENT_FAIL:
		return "CHAN_MEASUREMENT_FAIL";
#ifdef SUPPORT_MULTI_AP
    case RFS_SUCCESS:
		return "RFS_SUCCESS";
    case RFS_FAILURE:
		return "RFS_FAILURE";
    case RFS_TIMEOUT:
		return "RFS_TIMEOUT";
#endif
	case BTM_SUCCESS:
		return "BTM_SUCCESS";
    case BTM_FAILURE:
		return "BTM_FAILURE";
    case BTM_TIMEOUT:
		return "BTM_FAILURE";
#ifdef SUPPORT_MULTI_AP
    case REMOTE_TOPOLOGY_NOTIFICATION:
		return "REMOTE_TOPOLOGY_NOTIFICATION";
#endif
    case CLIENT_ASSOCIATED:
		return "CLIENT_ASSOCIATED";
    case CLIENT_DISSOCIATED:
		return "CLIENT_DISSOCIATED";
    case CLIENT_ASSOCIATION_TIMEOUT:
		return "CLIENT_ASSOCIATION_TIMEOUT";
    case AUTH_DENY_MAX_REACHED:
		return "AUTH_DENY_MAX_REACHED";
#ifdef SUPPORT_MULTI_AP
    case TSQ_SUCCESS:
		return "TSQ_SUCCESS";
    case TSQ_FAILURE:
		return "TSQ_FAILURE";
    case TSQ_TIMEOUT:
		return "TSQ_TIMEOUT";
#endif
	default:
		return "Unrecognized steering trigger";
	}
}


const char *str_method_str(STEERING_METHOD_TYPE steer_method)
{
	switch (steer_method) {
#ifdef SUPPORT_MULTI_AP
	    case MANDATE:
			return "MANDATE";
#endif
		case ACTIVE_STANDALONE_DG:
			return "ACTIVE_STANDALONE_DG";
		case IDLE_STANDALONE_DG:
			return "IDLE_STANDALONE_DG";
#ifdef SUPPORT_MULTI_AP
		case NOL_MULTIAP:
			return "NOL_MULTIAP";
#endif
		case OFFLOADING:
			return "OFFLOADING";
		case ACTIVE_STANDALONE_UG:
			return "ACTIVE_STANDALONE_UG";
		case IDLE_STANDALONE_UG:
			return "IDLE_STANDALONE_UG";
		case ACTIVE_5GL_TO_5GH:
			return "ACTIVE_5GL_TO_5GH";
		case ACTIVE_5GH_TO_5GL:
			return "ACTIVE_5GH_TO_5GL";
		case IDLE_5GL_TO_5GH:
			return "IDLE_5GL_TO_5GH";
		case IDLE_5GH_TO_5GL:
			return "IDLE_5GH_TO_5GL";
		case NONE:
			return "NONE";
		default:
			return "ERR";
	}
	return "ERR";
}

