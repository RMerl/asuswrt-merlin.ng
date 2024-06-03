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
 *  Headers for Steering Orchestration module
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Neelansh.M   2018/05/02     First implementation of the steer exec Module
 * */

//extern u8 ZERO_MAC_ADDR[ETH_ALEN];
#ifdef SUPPORT_MULTI_AP
#define ASSOC_BLOCK 0
#define ASSOC_UNBLOCK 1
#define ASSOC_BLOCK_DURATION 60

#define RFS_REQ_TLV_LEN		sizeof(struct rfs_req_tlv) - 3 // + bss list
#define RFS_RSP_TLV_LEN		sizeof(struct rfs_rsp_tlv)
#define TSQ_REQ_TLV_LEN		sizeof(struct tsq_req_tlv)
#define TSQ_RSP_TLV_LEN		sizeof(struct tsq_rsp_tlv)



#define RFS_RSP_TIMEOUT		2 /*seconds*/
#define TSQ_TIMEOUT_VAL			2 /*seconds*/
#define TSQ_REQ_TIMEOUT_VAL 	60


#define RFS_RETRY_CNT	1
#define TSQ_RETRY_CNT	1
#endif

struct peer_csbc_data {
	u8 consecutive_legacy_steer_fail;
	u8 consecutive_btm_fail;
	u8 consecutive_active_btm_fail;
	enum btm_csbc_state btm_state;
	enum force_str_csbc_state force_str_state;
	u32 rem_btm_unfriendly_time;
	u32 rem_force_str_unfriendly_time;
};

enum csbc_fsm_event
{
	FORCED_IDLE_STEER_FAIL,
	FORCED_IDLE_STEER_SUCCESS,
	BTM_IDLE_STEER_FAIL,
	BTM_IDLE_STEER_SUCCESS,
	BTM_ACTIVE_STEER_FAIL,
	BTM_ACTIVE_STEER_SUCCESS,
	REMOTE_FORCED_IDLE_STEER_FAIL,
	REMOTE_FORCED_IDLE_STEER_SUCCESS,
	REMOTE_BTM_IDLE_STEER_FAIL,
	REMOTE_BTM_IDLE_STEER_SUCCESS,
	REMOTE_BTM_ACTIVE_STEER_FAIL,
	REMOTE_BTM_ACTIVE_STEER_SUCCESS,
	CSBC_LEGACY_UNFRIENDLY_TO,
	CSBC_BTM_UNFRIENDLY_TO,
	PEER_SYNC_CSBC,
	INIT_FROM_DB,
	STA_JOIN,
	REMOTE_STA_JOIN
};
#ifdef SUPPORT_MULTI_AP
enum tsq_abort_reason
{
	STEER_SUCCESS,
	REJECTED_BY_PEER,
	REJECTED_AUTH_DENY_MAX,
	REJECTED_LOW_RSSI,
	REJECTED_BTM_FAIL,
	REJECTED_BTM_TIMEOUT,
	REJECTED_ASSOC_TIMEOUT
};

struct GNU_PACKED rfs_req_tlv
{
	struct tlv_head tlv;
	u8 transaction_id;
	u8 cli_mac[6];
	u32 rem_btm_unfriendly_time;
	u32 rem_force_str_unfriendly_time;
	u8 consecutive_legacy_steer_fail;
	u8 consecutive_btm_fail;
	u8 consecutive_active_btm_fail;
	u8 steer_type;
	u8 csbc_btm_state;
	u8 csbc_force_str_state;
	u8 auto_clear_blacklist;
	u8 allowed_bss_list_cnt;
	u8 bss[0]; //MAC_ADDRESS
};

struct GNU_PACKED rfs_rsp_tlv
{
	struct tlv_head tlv;
	u8 transaction_id;
	u8 cli_mac[6];
	u8 status;
	u32 rem_prohibit_time;
	u32 rem_btm_unfriendly_time;
	u32 rem_force_str_unfriendly_time;
	u8 consecutive_legacy_steer_fail;
	u8 consecutive_btm_fail;
	u8 consecutive_active_btm_fail;
	u8 csbc_btm_state;
	u8 csbc_force_str_state;
};

struct GNU_PACKED tsq_req_tlv {
	struct tlv_head tlv;
	u8 transaction_id;
	u8 cli_mac[6];
	enum tsq_abort_reason abort_reason;
};

struct GNU_PACKED tsq_rsp_tlv {
	struct tlv_head tlv;
	u8 transaction_id;
	u8 cli_mac[6];
};

typedef struct  tsq_rsp_sm_data 
{
	struct _1905_map_device *map_1905_device;
	struct tsq_rsp_tlv cli_tsq_rsp;
}TSQ_RSP_SM_DATA;

typedef struct rfs_rsp_sm_data 
{
	struct _1905_map_device *map_1905_device;
	struct rfs_rsp_tlv cli_rfs_rsp;
}RFS_RSP_SM_DATA;


struct coord_req_dev_list
{
	u8 al_mac[ETH_ALEN];
	u8 transaction_id;
	SLIST_ENTRY(coord_req_dev_list) coord_req_dev_entry;
};

#define RFS_STATUS_SUCCESS 0
#define RFS_STATUS_PROHIBIT 1
#define RFS_STATUS_UNFRIENDLY 2

#define PREP_ABORT
#define PREP_CONTINUE
#define PREP_SUCCESS
#endif
#define STEER_TYPE_BTM_ACTIVE	3
#define STEER_TYPE_BTM_IDLE		2
#define STEER_TYPE_LEGACY		1
#define STEER_TYPE_NONE			0
#ifdef SUPPORT_MULTI_AP
void steer_action_handle_tsq_rsp(struct mapd_global *pGlobal_dev,int client_id,
								struct _1905_map_device *map_1905_device, struct tsq_rsp_tlv *cli_tsq_rsp);

void steer_action_tsq_rsp_timeout(struct mapd_global *pGlobal_dev, int client_id);

u8 steer_action_steer_prep(struct mapd_global *pGlobal_dev, u32 client_id);
#endif
void steer_action_handle_coord_steer_complete(struct mapd_global *pGlobal_dev,
													u32 client_id, COMPLETE_STATE_DATA *data);
#ifdef SUPPORT_MULTI_AP
void steer_action_handle_rfs_rsp(struct mapd_global *pGlobal_dev,int client_id,
				struct rfs_rsp_tlv *cli_rfs_rsp,
				struct _1905_map_device *map_1905_device);

void steer_action_rfs_rsp_timeout(struct mapd_global *pGlobal_dev, int client_id);

void steer_action_handle_rfs_req(struct mapd_global *pGlobal_dev, struct rfs_req_tlv *rfs_req,
															struct _1905_map_device *map_1905_device);
void steer_action_handle_tsq_req(struct mapd_global *pGlobal_dev,
								struct tsq_req_tlv *cli_tsq_req,
								struct _1905_map_device *map_1905_device);

Boolean steer_action_is_steer_allowed(struct mapd_global *global, uint8_t client_id);
#else
Boolean steer_action_is_steer_allowed(struct mapd_global *global, uint8_t client_id, uint8_t is_steer_method_DG);
#endif

int steer_action_is_btm_active_str_allowed(struct mapd_global *global, struct client *cli);
int steer_action_is_btm_idle_str_allowed(struct mapd_global *global, struct client *cli);
int steer_action_is_force_str_allowed(struct mapd_global *global, struct client *cli);



void steer_action_prohibit_timeout_handler(void *eloop_ctx, void *user_ctx);
void steer_action_handle_steer_fail(struct mapd_global *global, struct client *cli);
void steer_action_handle_steer_success(struct mapd_global *global, struct client *cli);
#ifdef SUPPORT_MULTI_AP
void steer_action_handle_remote_topo_notif(struct mapd_global *pGlobal_dev, int client_id);
#endif
void steer_action_steer_client(struct mapd_global *global, uint8_t client_id);
#ifdef SUPPORT_MULTI_AP
void steer_action_clear_bl_on_peers(struct mapd_global *global, struct client *map_client, unsigned int reason_code);
#endif
void steer_action_handle_prohibit_timeout(void *eloop_ctx, void *timeout_ctx);
void steer_action_handle_global_prohibit_timeout(void *eloop_ctx, void *timeout_ctx);

void steer_action_csbc_init_from_db(struct mapd_global *global, struct client *cli);
void steer_action_clear_all_timers(struct mapd_global *global, struct client *cli);
void steer_action_sta_join(struct mapd_global *global, struct client *cli, u8 is_remote);
int steer_action_print_csbc_stats(struct mapd_global *global, struct client *cli,
				char *pos, char *end);
void steer_action_check_steer_type(struct mapd_global *global, uint32_t client_id);
int steer_action_reset_csbc(struct mapd_global *global, uint32_t client_id,
				u8 skip_force_csbc_reset);

struct steer_cands *get_client_from_steer_cand_list(struct own_1905_device *ctx, struct client *cli);
struct cent_steer_cands *get_client_from_cent_steer_cand_list(struct own_1905_device *ctx, struct client *cli);
u8 is_op_class_2g(u8 op_class);
#ifdef CENT_STR
u8 is_op_class_5g(u8 op_class);
struct cent_steer_cands *get_client_from_cent_steer_cand_list(struct own_1905_device *ctx, struct client *cli);
#endif
void client_disconn_trigger(struct mapd_global *global);
struct steer_cands *get_client_from_steer_cand_list(struct own_1905_device *ctx, struct client *cli);
