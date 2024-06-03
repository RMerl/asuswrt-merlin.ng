/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#include <stdio.h>
#include <string.h>
#include "itc_rpc.h"
#include "ba_svc.h"

#define RPC_SERVICE_VER_BA_CPU_ID						0
#define RPC_SERVICE_VER_BA_CPU_NAME						0
#define RPC_SERVICE_VER_BA_RUN_STATE_ID					0
#define RPC_SERVICE_VER_BA_RUN_STATE_NAME				0
#define RPC_SERVICE_VER_BA_GET_RUN_STATE				0
#define RPC_SERVICE_VER_BA_NOTIFY_RUN_STATE				0
#define RPC_SERVICE_VER_BA_REQUEST_RUN_STATE			0
#define RPC_SERVICE_VER_BA_REQUEST_RUN_STATE_RESPONSE	0
#define RPC_SERVICE_VER_BA_SET_RUN_STATE				0
#define RPC_SERVICE_VER_BA_XPORT_SET_PWR				0
#define RPC_SERVICE_VER_BA_GET_SMCBL_VER        		0
#define RPC_SERVICE_VER_BA_GET_SMCBL_VER_HASH   		0
#define RPC_SERVICE_VER_BA_GET_SMCOS_VER        		0
#define RPC_SERVICE_VER_BA_GET_SMCOS_VER_HASH   		0
#define RPC_SERVICE_VER_BA_RPRT_BOOT_SUCCESS            0
#define RPC_SERVICE_VER_BA_SVC_GET_BOOT_FAIL_CNT        0

#define BA_SVC_RPC_REQUEST_TIMEOUT	1 /* sec */

uint32_t ba_cpu_all;
uint32_t ba_cpu_rg;
uint32_t ba_cpu_cm;
uint32_t ba_cpu_gfap;
uint32_t ba_cpu_bne;
uint32_t ba_cpu_tpmi;
uint32_t ba_rs_off;
uint32_t ba_rs_reset;
uint32_t ba_rs_boot;
uint32_t ba_rs_shutdown;
uint32_t ba_rs_running;
uint32_t ba_rs_ready;

typedef uint8_t(*get_retcode_t)(rpc_msg *);

/* BA service helpers */
static inline int ba_svc_request(rpc_msg *msg, get_retcode_t cb)
{
	int rc = 0;
	rc = rpc_send_request_timeout(RPC_TUNNEL_ARM_SMC_NS,
		msg, BA_SVC_RPC_REQUEST_TIMEOUT);
	if (rc) 
	{
		printf("ba_svc: rpc_send_request failure (%d)\n", rc);
		rpc_dump_msg(msg);
		goto done;
	}
	if (cb)
    {
        rc = cb(msg);
        if (rc)
            printf("%s:%d : ERROR: rpc_send_request failure (%d)\n",__FUNCTION__,__LINE__, rc);
    }

done:
	return rc;
}

static inline int ba_svc_message(rpc_msg *msg)
{
	int rc = 0;
	rc = rpc_send_message(RPC_TUNNEL_ARM_SMC_NS, msg);
	if (rc) {
		printf("ba_svc: rpc_send_message failure (%d)\n", rc);
		rpc_dump_msg(msg);
	}
	return rc;
}

int ba_svc_boot_secondary(uint32_t cpu_mask, uint32_t vector)
{
	int rc = 0;
	rpc_msg msg;

	rpc_msg_init(&msg, RPC_SERVICE_BA, BA_SVC_BOOT_FROM_ADDR, 0, 0, 0, 0);
	msg.data[0] = 0;
	msg.data[1] = vector;
	msg.data[2] = cpu_mask;
	rc = ba_svc_message(&msg);
	return rc;
}
	
/* BA service calls */
int ba_svc_cpu_id(char *cpu_name, uint32_t *cpu_id)
{
	struct ba_msg ba_msg;
	rpc_msg *msg = (rpc_msg *)&ba_msg;
	int rc;

	rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_CPU_ID,
			RPC_SERVICE_VER_BA_CPU_ID, 0, 0, 0);
	strncpy(ba_msg.name, cpu_name, sizeof(ba_msg.name));
	rc = ba_svc_request(msg, ba_svc_msg_get_retcode);
	*cpu_id = ba_msg.cpu_id;
	return rc;
}

int ba_svc_cpu_name(uint32_t cpu_id, char *cpu_name)
{
	struct ba_msg ba_msg;
	rpc_msg *msg = (rpc_msg *)&ba_msg;
	int rc;

	if (cpu_id > 0xff)
		return -1;
	rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_CPU_NAME,
			RPC_SERVICE_VER_BA_CPU_NAME, cpu_id, 0, 0);
	rc = ba_svc_request(msg, ba_svc_msg_get_retcode);
	memcpy(cpu_name, ba_msg.name, sizeof(ba_msg.name));
	return rc;
}

int ba_svc_run_state_id(char *rs_name, uint32_t *rs_id)
{
	struct ba_msg ba_msg;
	rpc_msg *msg = (rpc_msg *)&ba_msg;
	int rc;

	rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_RUN_STATE_ID,
			RPC_SERVICE_VER_BA_RUN_STATE_ID, 0, 0, 0);
	strncpy(ba_msg.name, rs_name, sizeof(ba_msg.name));
	rc = ba_svc_request(msg, ba_svc_msg_get_retcode);
	*rs_id = ba_msg.rs_id;
	if (rc) {
		printf("%s failure, retcode %d\n", __func__, rc);
		rpc_dump_msg(msg);
	}
	return rc;
}

int ba_svc_run_state_name(uint32_t rs_id, char *rs_name)
{
	struct ba_msg ba_msg;
	rpc_msg *msg = (rpc_msg *)&ba_msg;
	int rc;

	if (rs_id > 0xff)
		return -1;
	rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_RUN_STATE_NAME,
			RPC_SERVICE_VER_BA_RUN_STATE_NAME, rs_id, 0, 0);
	rc = ba_svc_request(msg, ba_svc_msg_get_retcode);
	memcpy(rs_name, ba_msg.name, sizeof(ba_msg.name));
	if (rc) {
		printf("%s failure, retcode %d\n", __func__, rc);
		rpc_dump_msg(msg);
	}
	return rc;
}

int ba_svc_get_run_state(uint32_t cpu_id, uint32_t *rs_id)
{
	struct ba_msg ba_msg;
	rpc_msg *msg = (rpc_msg *)&ba_msg;
	int rc;

	rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_GET_RUN_STATE,
			RPC_SERVICE_VER_BA_GET_RUN_STATE, cpu_id, 0, 0);
	rc = ba_svc_request(msg, ba_svc_msg_get_retcode);
	*rs_id = ba_msg.rs_id;
	if (rc) {
		printf("%s failure, retcode %d\n", __func__, rc);
		rpc_dump_msg(msg);
	}
	return rc;
}

int ba_svc_notify_run_state(uint32_t cpu_id, uint32_t rs_id)
{
	struct ba_msg ba_msg;
	rpc_msg *msg = (rpc_msg *)&ba_msg;
	int rc;

	if (cpu_id > 0xff || rs_id > 0xff)
		return -1;
	rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_NOTIFY_RUN_STATE,
			RPC_SERVICE_VER_BA_NOTIFY_RUN_STATE, 0, 0, 0);
	ba_msg.rs_id = (uint8_t)rs_id;
	ba_msg.cpu_id = (uint8_t)cpu_id;
	rc = ba_svc_message(msg);
	return rc;
}

int ba_svc_request_run_state(uint32_t cpu_id, uint32_t rs_id, bool be_rude)
{
	struct ba_msg ba_msg;
	rpc_msg *msg = (rpc_msg *)&ba_msg;
	int rc;

	if (cpu_id > 0xff || rs_id > 0xff)
		return -1;
	rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_REQUEST_RUN_STATE,
			RPC_SERVICE_VER_BA_REQUEST_RUN_STATE, 0, 0, 0);
	ba_msg.cpu_id = (uint8_t)cpu_id;
	ba_msg.rs_id = (uint8_t)rs_id;
	ba_msg.be_rude = be_rude ? 1 : 0;
	rc = ba_svc_message(msg);
	return rc;
}

int ba_svc_request_run_state_response(uint32_t cpu_id, uint32_t rs_id,
	enum ba_req_rs_rsp response)
{
	struct ba_msg ba_msg;
	rpc_msg *msg = (rpc_msg *)&ba_msg;
	int rc;

	rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_REQUEST_RUN_STATE_RESPONSE,
			RPC_SERVICE_VER_BA_REQUEST_RUN_STATE_RESPONSE, 0, 0, 0);
	ba_msg.cpu_id = (uint8_t)cpu_id;
	ba_msg.rs_id = (uint8_t)rs_id;
	ba_msg.response = response;
	rc = ba_svc_message(msg);
	return rc;
}

/* BA service handlers */
int ba_svc_run_state_notification(enum rpc_tunnel_idx tunnel, rpc_msg *msg)
{
	/* TODO: handle GFAP run state notification changes here */

	return 0;
}

int ba_svc_run_state_requested(enum rpc_tunnel_idx tunnel, rpc_msg *msg)
{
	struct ba_msg *ba_msg = (struct ba_msg *)msg;

	/* TODO: stop using resources of CPU that is rebooting first */

	ba_svc_request_run_state_response(ba_msg->cpu_id,
					  ba_msg->rs_id,
					  BA_SVC_RESPONSE_READY);
	return 0;
}

int ba_svc_set_run_state(enum rpc_tunnel_idx tunnel, rpc_msg *msg)
{
	struct ba_msg *ba_msg = (struct ba_msg *)msg;

	/* TODO: send out each state below current until hit target state */

	ba_svc_notify_run_state(ba_cpu_rg, ba_msg->rs_id);
	return 0;
}

rpc_function ba_service_table[BA_SVC_FUNC_MAX] =
{
	{ NULL,					RPC_SERVICE_VER_BA_CPU_ID },
	{ NULL,					RPC_SERVICE_VER_BA_CPU_NAME },
	{ NULL,					RPC_SERVICE_VER_BA_RUN_STATE_ID },
	{ NULL,					RPC_SERVICE_VER_BA_RUN_STATE_NAME },
	{ NULL,					RPC_SERVICE_VER_BA_GET_RUN_STATE },
	{ ba_svc_run_state_notification,	RPC_SERVICE_VER_BA_NOTIFY_RUN_STATE },
	{ ba_svc_run_state_requested,		RPC_SERVICE_VER_BA_REQUEST_RUN_STATE },
	{ NULL,					RPC_SERVICE_VER_BA_REQUEST_RUN_STATE_RESPONSE },
	{ ba_svc_set_run_state,			RPC_SERVICE_VER_BA_SET_RUN_STATE },
};

int ba_svc_init(void)
{
	int rc = 0;

	rc = rpc_register_functions(RPC_SERVICE_BA, ba_service_table,
			       BA_SVC_FUNC_MAX);
	if (rc) goto done;
	rc = ba_svc_cpu_id(BA_SVC_CPU_ALL, &ba_cpu_all);
	if (rc) {
		printf("BA: Failure getting CPU ALL ID!\n");
		goto done;
	}
	rc = ba_svc_cpu_id(BA_SVC_CPU_RG, &ba_cpu_rg);
	if (rc) {
		printf("BA: Failure getting RG ID!\n");
		goto done;
	}
	rc = ba_svc_cpu_id(BA_SVC_CPU_CM, &ba_cpu_cm);
	if (rc) ba_cpu_cm = INVALID_ID;
	rc = ba_svc_cpu_id(BA_SVC_CPU_GFAP, &ba_cpu_gfap);
	if (rc) {
		printf("BA: Failure getting GFAP ID!\n");
		goto done;
	}
	rc = ba_svc_cpu_id(BA_SVC_CPU_BNE, &ba_cpu_bne);
	if (rc) ba_cpu_bne = INVALID_ID;
	rc = ba_svc_cpu_id(BA_SVC_CPU_TPMI, &ba_cpu_tpmi);
	if (rc) ba_cpu_tpmi = INVALID_ID;
	rc = ba_svc_run_state_id(BA_SVC_RS_OFF, &ba_rs_off);
	if (rc) goto done;
	rc = ba_svc_run_state_id(BA_SVC_RS_RESET, &ba_rs_reset);
	if (rc) goto done;
	rc = ba_svc_run_state_id(BA_SVC_RS_BOOT, &ba_rs_boot);
	if (rc) goto done;
	rc = ba_svc_run_state_id(BA_SVC_RS_SHUTDOWN, &ba_rs_shutdown);
	if (rc) goto done;
	rc = ba_svc_run_state_id(BA_SVC_RS_RUNNING, &ba_rs_running);
	if (rc) goto done;
	rc = ba_svc_run_state_id(BA_SVC_RS_READY, &ba_rs_ready);
	if (rc) goto done;

	rc = ba_svc_notify_run_state(ba_cpu_rg, ba_rs_running);
	if (rc) goto done;

done:
	if (rc)
		printf("Failure initializing boot assist service!\n");
	return rc;
}

int ba_xport_set_state( uint8_t port_id, uint8_t enable)
{
	int ret = 0;
	struct ba_msg ba_msg;
	rpc_msg *msg = (rpc_msg *)&ba_msg;

	rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_XPORT_SET_PWR, RPC_SERVICE_VER_BA_XPORT_SET_PWR, 0, 0, 0);
	
	ba_msg.cpu_id = port_id;
	ba_msg.rs_id = enable;

	ret = ba_svc_request(msg, ba_svc_msg_get_retcode);
	if (ret)
	{
		printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
		return -1;
	}

	return ret;
}

int ba_get_smcbl_ver(smcbl_ver_t  *smcbl_ver)
{
    int ret = 0;
    struct ba_msg ba_msg;
    rpc_msg *msg = (rpc_msg *)&ba_msg;

    rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_GET_SMCBL_VER, RPC_SERVICE_VER_BA_GET_SMCBL_VER, 0, 0, 0);
    ret = ba_svc_request(msg, NULL);
    if (ret)
    {
        printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
        return -1;
    }

    smcbl_ver->smcbl_major_ver  = ((msg->data[0] >> 16) & 0xffff);
    smcbl_ver->smcbl_minor_ver  = ((msg->data[0]) & 0xffff);
    smcbl_ver->smcbl_rev        = (msg->data[1]);
    smcbl_ver->ponbl_major_ver  = ((msg->data[2] >> 16) & 0xffff);
    smcbl_ver->ponbl_minor_ver  = ((msg->data[2]) & 0xffff);

    memset(msg , 0 , sizeof(struct ba_msg));
    rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_GET_SMCBL_VER_HASH, RPC_SERVICE_VER_BA_GET_SMCBL_VER_HASH, 0, 0, 0);
    ret = ba_svc_request(msg, NULL);
    if (ret)
    {
        printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
        return -1;
    }

    strncpy(&smcbl_ver->smcbl_ver_hash[0], (char*)&(msg->data[0]), HASH_SHORT_SIZE);

    printf("SMC_BL v.%d.%d.%d.%d.%d (%s)\n", smcbl_ver->smcbl_major_ver, 
                                             smcbl_ver->smcbl_minor_ver,
                                             smcbl_ver->smcbl_rev,
                                             smcbl_ver->ponbl_major_ver,
                                             smcbl_ver->ponbl_minor_ver,
                                             smcbl_ver->smcbl_ver_hash);
    return ret;
}

int ba_get_smcos_ver(smcos_ver_t  *smcos_ver)
{
    int ret = 0;
    struct ba_msg ba_msg;
    rpc_msg *msg = (rpc_msg *)&ba_msg;

    rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_GET_SMCOS_VER, RPC_SERVICE_VER_BA_GET_SMCOS_VER, 0, 0, 0);
    ret = ba_svc_request(msg, NULL);
    if (ret)
    {
        printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
        return -1;
    }

    smcos_ver->smcos_major_ver  = ((msg->data[0] >> 16 ) & 0xffff);
    smcos_ver->smcos_minor_ver  = ((msg->data[0]) & 0xffff);
    smcos_ver->smcos_rev        = ((msg->data[1] >> 16) & 0xffff);
    smcos_ver->pon_major_ver    = ((msg->data[1] ) & 0xffff);
    smcos_ver->pon_minor_ver    = ((msg->data[2] >> 16 ) & 0xffff);
    smcos_ver->pon_patch_ver    = ((msg->data[2]) & 0xffff);

    memset(msg , 0 , sizeof(struct ba_msg));
    rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_GET_SMCOS_VER_HASH, RPC_SERVICE_VER_BA_GET_SMCOS_VER_HASH, 0, 0, 0);
    ret = ba_svc_request(msg, NULL);
    if (ret)
    {
        printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
        return -1;
    }

    strncpy(&smcos_ver->smcos_ver_hash[0], (char*)&(msg->data[0]), HASH_SHORT_SIZE);

    printf("SMC_OS v%d.%d.%d.%d.%d.%d (%s)\n", smcos_ver->smcos_major_ver, 
                                               smcos_ver->smcos_minor_ver,
                                               smcos_ver->smcos_rev,
                                               smcos_ver->pon_major_ver,
                                               smcos_ver->pon_minor_ver,
                                               smcos_ver->pon_patch_ver,
                                               smcos_ver->smcos_ver_hash);
    return ret;
}

int bcm_rpc_ba_report_boot_success(uint32_t flags)
{
    int ret = 0;
    struct ba_msg ba_msg;
    rpc_msg *msg = (rpc_msg *)&ba_msg;

#ifdef DEBUG
    printf("[%s:%d]\n",__FUNCTION__, __LINE__);
#endif
    rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_RPRT_BOOT_SUCCESS, RPC_SERVICE_VER_BA_RPRT_BOOT_SUCCESS, flags, 0, 0);

    ret = ba_svc_request(msg, NULL);
    if (ret)
    {
        printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
        return -1;
    }

    return ret;
}

int bcm_rpc_ba_get_boot_fail_cnt(void)
{
    int ret = 0;
    struct ba_msg ba_msg;
    rpc_msg *msg = (rpc_msg *)&ba_msg;

    rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_GET_BOOT_FAIL_CNT, RPC_SERVICE_VER_BA_SVC_GET_BOOT_FAIL_CNT, 0, 0, 0);
    ret = ba_svc_request(msg, NULL);
    if (ret)
    {
        printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
        return -1;
    }

    return msg->data[0];
}

