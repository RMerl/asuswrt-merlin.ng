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
 *  Steer Exec
 *
 *  Abstract:
 *  Steering Orchestration module
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Neelansh.M   2018/05/02     First implementation of the steer exec Module
 * */
#include "includes.h"
#include "common.h"
#include "steer_fsm.h"
#include "list.h"
#include "client_db.h"
#include "mapd_i.h"
#include "db.h"
#ifdef SUPPORT_MULTI_AP
#include "./../1905_local_lib/data_def.h"
#endif
#include "chan_mon.h"
#include "./../1905_local_lib/data_def.h"
#ifdef SUPPORT_MULTI_AP
#include "topologySrv.h"
#endif
#include "client_mon.h"
//#include <sys/queue.h>
#include "eloop.h"
#include "steer_action.h"
#include "ap_est.h"
#include "eloop.h"
#include "wapp_if.h"
#include <assert.h>
#include <sys/un.h>
#ifdef SUPPORT_MULTI_AP
#include "1905_map_interface.h"
#include "ch_planning.h"
#endif
#include "ap_roam_algo.h"
#ifdef CENT_STR
#include "ap_cent_str.h"
#endif

struct steer_cands *get_client_from_steer_cand_list(struct own_1905_device *ctx, struct client *cli)
{
	struct steer_cands *cand = NULL;
	if (SLIST_EMPTY(&(ctx->steer_cands_head)))
		return NULL;
	cand = SLIST_FIRST(&(ctx->steer_cands_head));
	while(cand != NULL) {
		if (cli->client_id == cand->steer_cand->client_id)
			return cand;
		cand = SLIST_NEXT(cand, next_cand);
	}
	return NULL;
}

#ifdef CENT_STR
struct cent_steer_cands *get_client_from_cent_steer_cand_list(struct own_1905_device *ctx, struct client *cli)
{
	struct cent_steer_cands *cand = NULL;
	if (STAILQ_EMPTY(&(ctx->cent_steer_cands_head)))
		return NULL;
	cand = STAILQ_FIRST(&(ctx->cent_steer_cands_head));
	while(cand != NULL) {
		if (cli->client_id == cand->steer_cand->client_id)
			return cand;
		cand = STAILQ_NEXT(cand, next_cand);
	}
	return NULL;
}

#endif


/* We will check if the prohibit timoeut is registered wherever we need to
 * take care of the prohibit timoeut existence. Thus nothing needs to be done in the
 * timoeut handler because the timeout has already expired
 */
void steer_action_handle_prohibit_timeout(void *eloop_ctx, void *timeout_ctx)
{
	struct client *cli = (struct client *)timeout_ctx;

	mapd_printf(MSG_INFO, "Prohibit Timer expired for %d " MACSTR,
					cli->client_id, MAC2STR(cli->mac_addr));
}

void steer_action_handle_global_prohibit_timeout(void *eloop_ctx, void *timeout_ctx)
{
	mapd_printf(MSG_INFO, "****** Glboal Prohibit Timer expired *******");
}

#ifndef SUPPORT_MULTI_AP
/* In case of BS standalone, after active steering is done
 * a cooldown period is required to get the actual picture of the n/w
 * This timeout is to provide the desired cooldown period of 60sec.
 * In case of MAP enabled this is already handled.
 * nothing needs to be done in the timoeut handler because the
 * timeout has already expired.
 */
void steer_action_handle_steer_blackout_timeout(void *eloop_ctx, void *timeout_ctx)
{
        struct client *cli = (struct client *)timeout_ctx;

        mapd_printf(MSG_INFO, "Steer blackout Timer expired for %d " MACSTR,
                                        cli->client_id, MAC2STR(cli->mac_addr));
}
#endif



void client_disconn_trigger(struct mapd_global *global) {
	struct client *cli = NULL;
	uint32_t i = 0;

	/*During reset all clients will be disconnected so trigger leave*/
	for(i = 0; i < MAX_STA_SEEN; i++) {
		cli = &global->dev.client_db[i];
		if (cli->client_id == (uint32_t)-1)
			continue;

		if(cli->cli_steer_state == CLI_STATE_INVALID)
			continue;
		if(!is_zero_ether_addr(cli->bssid)){
			err("State:%d ,Trigger client leave "MACSTR,cli->cli_steer_state,MAC2STR(cli->mac_addr));
			client_mon_handle_local_leave(global,cli->mac_addr,cli->bssid);
		}
	}

	return;
}

