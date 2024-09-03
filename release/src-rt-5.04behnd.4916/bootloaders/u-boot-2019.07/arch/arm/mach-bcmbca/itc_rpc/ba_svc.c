/* SPDX-License-Identifier: GPL-2.0+
  *
  * Copyright 2019 Broadcom Ltd.
 */

#include <stdio.h>
#include <string.h>
#include <common.h>
#include "itc_rpc.h"
#include "ba_svc.h"

#define RPC_SERVICE_VER_BA_XPORT_SET_PWR                0
#define RPC_SERVICE_VER_BA_GET_SMCBL_VER                0
#define RPC_SERVICE_VER_BA_GET_SMCBL_VER_HASH           0
#define RPC_SERVICE_VER_BA_GET_SMCOS_VER                0
#define RPC_SERVICE_VER_BA_GET_SMCOS_VER_HASH           0
#define RPC_SERVICE_VER_BA_RPRT_BOOT_SUCCESS            0
#define RPC_SERVICE_VER_BA_SVC_GET_BOOT_FAIL_CNT        0
#define RPC_SERVICE_VER_BA_SVC_GET_SEC_STATE            0
#define RPC_SERVICE_VER_BA_SVC_SEC_HANDLE_CERTIFICATE   0
#define RPC_SERVICE_VER_BA_SVC_SEC_GET_SKS_STATS        0
#define RPC_SERVICE_VER_BA_SVC_SEC_AES_CRYPT            0
#define RPC_SERVICE_VER_BA_SVC_SEC_RSA_SIG_VERIFY       0
#define RPC_SERVICE_VER_BA_SVC_SEC_GET_KEY_SIZE         0

#define BA_SVC_RPC_REQUEST_TIMEOUT	1 /* sec */

#define SOTP_EXCHANGE_AREA_ADDR_START  0x00640000
#define SOTP_EXCHANGE_AREA_ADDR_END    0x0067FFFF

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
	
/* BA service handlers */
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

int bcm_rpc_ba_get_sec_state(void)
{
    int ret = 0;
    struct ba_msg ba_msg;
    rpc_msg *msg = (rpc_msg *)&ba_msg;

    rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_GET_SEC_STATE, RPC_SERVICE_VER_BA_SVC_GET_SEC_STATE, 0, 0, 0);
    ret = ba_svc_request(msg, NULL);
    if (ret)
    {
        printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
        return -1;
    }

    return msg->data[0];
}

int ba_get_dev_spec_key(void** dev_key, int* ek_size, int* iv_size)
{
    int ret = 0;
    struct ba_msg ba_msg;
    rpc_msg *msg = (rpc_msg *)&ba_msg;

    rpc_msg_init(msg, RPC_SERVICE_BA, BA_SVC_GET_DEV_SPEC_KEY, 0, 0, 0, 0);

    msg->data[0] = SOTP_EXCHANGE_AREA_ADDR_START;

    invalidate_dcache_range((unsigned long)msg->data[0], (unsigned long)msg->data[0] + SOTP_EXCHANGE_AREA_ADDR_END);

    ret = ba_svc_request(msg, NULL);
    if (ret)
    {
        printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
        return -1;
    }
    *dev_key = (void*)SOTP_EXCHANGE_AREA_ADDR_START;
    *ek_size = msg->data[1];
    *iv_size = msg->data[2];

    return 0;
}

int bcm_rpc_sec_handle_ksm_certificate(const uint8_t *certificate, uint32_t certificate_size)
    {
    int ret = 0;
    rpc_msg msg;

    rpc_msg_init(&msg, RPC_SERVICE_BA, BA_SVC_SEC_HANDLE_CERTIFICATE, RPC_SERVICE_VER_BA_SVC_SEC_HANDLE_CERTIFICATE, 0, 0, 0);
    msg.data[0] = 0;
    msg.data[1] = (unsigned long)certificate;
    msg.data[2] = certificate_size;

#if !defined (CONFIG_TPL_BUILD)
    flush_dcache_range((unsigned long)certificate, (unsigned long)((uint8_t *)certificate + certificate_size));
#endif // #if !defined (CONFIG_TPL_BUILD)

    ret = ba_svc_request(&msg, NULL);
    if (ret)
    {
        printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
        return -1;
    }

    return msg.data[0];
}

int bcm_rpc_sec_get_sks_stats(struct sks_stats *stats)
{
    int ret = 0;
    rpc_msg msg;

    rpc_msg_init(&msg, RPC_SERVICE_BA, BA_SVC_SEC_GET_SKS_STATS, RPC_SERVICE_VER_BA_SVC_SEC_GET_SKS_STATS, 0, 0, 0);
    msg.data[0] = 0;
    msg.data[1] = (unsigned long)stats;
    msg.data[2] = sizeof(*stats);

#if !defined (CONFIG_TPL_BUILD)
    invalidate_dcache_range((unsigned long)stats, (unsigned long)((uint8_t *)stats + sizeof(*stats)));
#endif // #if !defined (CONFIG_TPL_BUILD)

    ret = ba_svc_request(&msg, NULL);
    if (ret)
    {
        printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
        return -1;
    }

    return msg.data[0];
}

int bcm_rpc_sec_get_key_size(uint32_t key, uint32_t *size)
{
    int ret = 0;
    rpc_msg msg;

    rpc_msg_init(&msg, RPC_SERVICE_BA, BA_SVC_SEC_GET_KEY_SIZE, RPC_SERVICE_VER_BA_SVC_SEC_GET_KEY_SIZE, 0, 0, 0);

    msg.data[0] = 0;
    msg.data[1] = key;
    msg.data[2] = 0;

    ret = ba_svc_request(&msg, NULL);
    if (ret)
    {
        printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
        return -1;
    }

    *size = msg.data[2];

    return msg.data[0];
}

int bcm_rpc_sec_rsa_sig_verify(struct sec_rsa_sig_verify_descriptor *crypto_desc)
{
    int ret = 0;
    rpc_msg msg;

    rpc_msg_init(&msg, RPC_SERVICE_BA, BA_SVC_SEC_RSA_SIG_VERIFY, RPC_SERVICE_VER_BA_SVC_SEC_GET_SKS_STATS, 0, 0, 0);
    msg.data[0] = 0;
    msg.data[1] = (unsigned long)crypto_desc;
    msg.data[2] = sizeof(*crypto_desc);

#if !defined (CONFIG_TPL_BUILD)
    flush_dcache_range((unsigned long)crypto_desc, (unsigned long)((uint8_t *)crypto_desc + sizeof(*crypto_desc)));
#endif // #if !defined (CONFIG_TPL_BUILD)

    ret = ba_svc_request(&msg, NULL);
    if (ret)
    {
        printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
        return -1;
    }

    return msg.data[0];
}

int bcm_rpc_sec_aes_crypt(struct sec_aes_crypt_descriptor *crypto_desc)
{
    int ret = 0;
    rpc_msg msg;

    rpc_msg_init(&msg, RPC_SERVICE_BA, BA_SVC_SEC_AES_CRYPT, RPC_SERVICE_VER_BA_SVC_SEC_GET_SKS_STATS, 0, 0, 0);
    msg.data[0] = 0;
    msg.data[1] = (unsigned long)crypto_desc;
    msg.data[2] = sizeof(*crypto_desc);

#if !defined (CONFIG_TPL_BUILD)
    flush_dcache_range((unsigned long)crypto_desc, (unsigned long)((uint8_t *)crypto_desc + sizeof(*crypto_desc)));
#endif // #if !defined (CONFIG_TPL_BUILD)

    ret = ba_svc_request(&msg, NULL);
    if (ret)
    {
        printf("%s:%d : ERROR: ba_svc_request\n",__FUNCTION__, __LINE__);
        return -1;
    }

	return msg.data[0];
}
