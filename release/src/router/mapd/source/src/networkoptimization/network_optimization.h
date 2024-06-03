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
#ifndef NETWORK_OPTIMIZATION_H
#define NETWORK_OPTIMIZATION_H

#include "mapd_i.h"
#include "apSelection.h"

#define MINIMUM_AGENTS_COUNT_FOR_NETWORK_OPTIMIZATION		2

#define NETWORK_OPTIMIZATION_FAILED			1
#define NETWORK_OPTIMIZATION_SUCCESS		2


#define DEVICE_CONNECT					1
#define DEVICE_DISCONNECT				0

#define NETWORK_OPTIMIZATION_FORCED_TRIGGER		1

enum map_network_optimization {
	NETWORK_OPTIMIZATION_IDLE = 0,
	NETWORK_OPTIMIZATION_TRIGGERED
};

#define IS_NTWRK_OPT_TRIGGERED(ctx) \
	(ctx->network_optimization_enabled && \
							ctx->ntwrk_opt.network_optimization_state == NETWORK_OPTIMIZATION_TRIGGERED)

#ifdef SUPPORT_MULTI_AP
struct GNU_PACKED ntwrk_opt_rsp_tlv {
	struct tlv_head tlv;
	u8 transaction_id;
	u8 almac[ETH_ALEN];
	u8 status;	/* 1 Fail 0 Success*/
};
#endif
void Optimized_Link_estimation_algo(struct mapd_global *pGlobal_dev);
unsigned int find_max_uplink_rate(struct _1905_map_device *_1905_device);
void add_dev_to_opt_net(
	struct _1905_map_device *_1905_dev,
	struct own_1905_device *dev);
void find_link_with_max_score(struct own_1905_device *dev);
void clear_estimated_scores(struct own_1905_device *dev);
unsigned char is_eth_BH(struct _1905_map_device *_1905_device);
void dump_est_opt_net(struct own_1905_device *dev);
void ntwrk_opt_device_conn_disconnect_handle(struct own_1905_device *ctx, unsigned char IsConnect, struct _1905_map_device *_1905_dev);
void update_ntwrk_opt_in_dat_file(unsigned char value);
void Update_link_estimation_db (struct mapd_global *pGlobal_dev);
void network_optimization_state_handler(struct mapd_global *global);
void network_opt_init(struct mapd_global *pGlobal_dev);
void network_opt_reset(struct mapd_global *pGlobal_dev);
void reset_last_uplink_rate(struct mapd_global *pGlobal_dev);
void channel_cac_timeout2(void *eloop_ctx, void *timeout_ctx);
void trigger_5G_net_opt(void *eloop_ctx, void *timeout_ctx);
void trigger_net_opt(void *eloop_ctx, void *timeout_ctx);


#endif
