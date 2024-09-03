/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom Ltd.
 */
#include <common.h>
#include <fdtdec.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <linux/errno.h>
#include <asm/unaligned.h>
#include <malloc.h>
#include "tpl_params.h"
#include "spl_env.h"
#include "bcm_secure.h"
#include "u-boot/rsa.h"
#include "u-boot/rsa-mod-exp.h"
#include <u-boot/sha256.h>
#include <uboot_aes.h>
#include <watchdog.h>
#include "mini-gmp/mini-gmp.h"
#include "mini-gmp/mini-mpq.h"
#include "ba_svc.h"

#if defined(CONFIG_SMC_BASED)

#define KS_KEY_NAME_HINT_BYTES 4

static int bcm_sec_smc_key_id_str_2_num(const char *str, uint32_t *num)
{
    if (KS_KEY_NAME_HINT_BYTES != strlen(str)) {
        return -1;
    }

    ((uint8_t *)num)[0] = str[3];
    ((uint8_t *)num)[1] = str[2];
    ((uint8_t *)num)[2] = str[1];
    ((uint8_t *)num)[3] = str[0];

    return 0;
}

int bcm_sec_smc_boot_state(bcm_sec_state_t* sec_state)
{
    /*
        Get Secure State supported for SMC based devices / Gen4 only,
            Implementation of Gen3 Secure State on SMC based devices will be considered later.
    */
    switch(bcm_rpc_ba_get_sec_state()) {
        /* Don't break current implementation, convert SMC Security State to TPL Security State */
        case SMC_SEC_STATE_UNSEC:
            *sec_state = SEC_STATE_UNSEC;
            break;
        case SMC_SEC_STATE_MFG:
            *sec_state = SEC_STATE_GEN3_MFG;
            break;
        default:
            *sec_state = SEC_STATE_GEN3_FLD;
            break;
    }

    return 0;
}

void bcm_sec_smc_get_root_aes_key_name_hint(const u8 * fit, u8** key)
{
    *key = bcm_util_get_fdt_prop_data((void *)fit, SEC_FIT_NODE_PATH, SEC_FIT_NODE_ROE, NULL);
}

u8* bcm_sec_smc_get_root_pub_key_name_hint(const u8 * fit)
{
    return bcm_util_get_fdt_prop_data((void *)fit, SEC_FIT_NODE_PATH, SEC_FIT_NODE_ROT, NULL);
}

bool bcm_sec_smc_is_active_pub_key_name_hint(void)
{
    if ((NULL == bcm_sec()->delg_cfg_obj) && (0 == memcmp(bcm_sec()->key.rsa_pub, SEC_FIT_NODE_ROT, sizeof(SEC_FIT_NODE_ROT)))) {
        return true;
    }

    return false;
}

void bcm_sec_smc_get_active_pub_key_info(const u8 * fit, u8** key, u32 *len)
{
#if defined (CONFIG_TPL_BUILD)
    static bool key_name_hint_preserved = false;
#endif // #if defined (CONFIG_TPL_BUILD)
    u8 *hint = bcm_util_get_fdt_prop_data((void *)fit, SEC_FIT_NODE_PATH, SEC_FIT_NODE_ROT, NULL);

    *key = 0;
    *len = 0;

    if (hint) {
#if defined (CONFIG_TPL_BUILD)
        if (false == key_name_hint_preserved) {
            memcpy(bcm_sec()->key.rsa_pub, SEC_FIT_NODE_ROT, sizeof(SEC_FIT_NODE_ROT));
            key_name_hint_preserved = true;
        }
#endif // #if defined (CONFIG_TPL_BUILD)
        if (true == bcm_sec_smc_is_active_pub_key_name_hint()) {
            *key = hint;
            if (bcm_sec_smc_get_key_size(hint, len)) {
                *len = 0;
            }
        } else {
            bcm_sec_get_active_pub_key_info(key, len);
        }
	}
}

int bcm_sec_smc_rsa_verify(const u8 *obj, u32 obj_len, const u8* sig, const u8 *pub)
{
    struct sec_rsa_sig_verify_descriptor __attribute__((aligned(8))) desc = {0};
    int rc = -1;

    if (bcm_sec_smc_key_id_str_2_num((const char *)pub, &(desc.key_name_hint))) {
        return -1;
    }

    desc.crypto_options = 0;
    desc.data_addr = (uint64_t)obj;
    desc.data_size = obj_len;
    desc.signature_addr = (uint64_t)sig;
    desc.signature_size = 0;

#if !defined (CONFIG_TPL_BUILD)
    flush_dcache_range((unsigned long)desc.data_addr, (unsigned long)((uint8_t *)(desc.data_addr) + desc.data_size));
    flush_dcache_range((unsigned long)desc.signature_addr, (unsigned long)((uint8_t *)(desc.signature_addr) + RSA4096_BYTES));
#endif // #if !defined (CONFIG_TPL_BUILD)

    rc = bcm_rpc_sec_rsa_sig_verify(&desc);

    return rc;
}

int bcm_sec_smc_aes_encrypt(const u8 *src, u8 *dst, u32 length, const u8 *key)
{
    struct sec_aes_crypt_descriptor __attribute__((aligned(8))) desc = {0};
    int rc = -1;

    if (bcm_sec_smc_key_id_str_2_num((const char *)key, &(desc.key_name_hint))) {
        return -1;
    }

    desc.crypto_options = SEC_AES_CRYPT_ENCRYPT;
    desc.data_src_addr = (uint64_t)src;
    desc.data_dst_addr = (uint64_t)dst;
    desc.data_size = length;

#if !defined (CONFIG_TPL_BUILD)
    flush_dcache_range((unsigned long)desc.data_src_addr, (unsigned long)((uint8_t *)(desc.data_src_addr) + desc.data_size));
#endif // #if !defined (CONFIG_TPL_BUILD)

    rc = bcm_rpc_sec_aes_crypt(&desc);

#if !defined (CONFIG_TPL_BUILD)
    invalidate_dcache_range((unsigned long)desc.data_dst_addr, (unsigned long)((uint8_t *)(desc.data_dst_addr) + desc.data_size));
#endif // #if !defined (CONFIG_TPL_BUILD)

    return rc;
}

int bcm_sec_smc_aes_decrypt(const u8 *src, u8 *dst, u32 length, const u8 *key)
{
    struct sec_aes_crypt_descriptor __attribute__((aligned(8))) desc = {0};
    int rc = -1;

    if (bcm_sec_smc_key_id_str_2_num((const char *)key, &(desc.key_name_hint))) {
        return -1;
    }

    desc.crypto_options = SEC_AES_CRYPT_DECRYPT;
    desc.data_src_addr = (uint64_t)src;
    desc.data_dst_addr = (uint64_t)dst;
    desc.data_size = length;

#if !defined (CONFIG_TPL_BUILD)
    flush_dcache_range((unsigned long)desc.data_src_addr, (unsigned long)((uint8_t *)(desc.data_src_addr) + desc.data_size));
#endif // #if !defined (CONFIG_TPL_BUILD)

    rc = bcm_rpc_sec_aes_crypt(&desc);

#if !defined (CONFIG_TPL_BUILD)
    invalidate_dcache_range((unsigned long)desc.data_dst_addr, (unsigned long)((uint8_t *)(desc.data_dst_addr) + desc.data_size));
#endif // #if !defined (CONFIG_TPL_BUILD)

    return rc;
}

int bcm_sec_smc_get_key_size(const u8 *key, u32 *size)
{
    uint32_t key_name_hint = 0;

    if (bcm_sec_smc_key_id_str_2_num((const char *)key, &key_name_hint)) {
        return -1;
    }

    return bcm_rpc_sec_get_key_size(key_name_hint, size);
}

int bcm_sec_smc_handle_ksm_certificate(const u8 *data, u32 size)
{
    return bcm_rpc_sec_handle_ksm_certificate(data, size);
}

int bcm_sec_smc_get_key_store_stats(u32 *version, u32 *entries, u32 *epoch)
{
    struct sks_stats __attribute__((aligned(8))) stats = {0};
    int rc = -1;

    rc = bcm_rpc_sec_get_sks_stats(&stats);

    if (SEC_RC_OK == rc) {
        if (version) *version = stats.version;
        if (entries) *entries = stats.entries;
        if (epoch) *epoch = stats.epoch;
    }

    return rc;
}

#endif // #if defined(CONFIG_SMC_BASED)
