/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
       All Rights Reserved
    
    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:
    
       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.
    
    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    
    :>
*/

#include "rdp_subsystem_common.h"
#include "rdd_common.h"
#include "rdp_drv_natc.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_data_structures_auto.h"

#define POLLING_TIME_OUT  1000

static uint8_t g_hash_mode;
uint32_t total_length_bytes[6] = {48, 64, 80, 96, 112, 128};
natc_tbl_config_t g_natc_tbl_cfg[NATC_MAX_TABLES_NUM] = {};
uint32_t g_natc_tbls_num = 0;

#ifndef _CFE_ 
bdmf_fastlock nat_cache_lock;
#ifndef RDP_SIM


static int drv_natc_eng_key_result_write(uint32_t eng_idx, natc_eng_key_result * key_result)
{
    uint32_t i;
    natc_eng_key_result key_result_swapped = {};

    key_result_swapped.data[3] = key_result->data[0];
    key_result_swapped.data[2] = key_result->data[1];
    key_result_swapped.data[1] = key_result->data[2];
    key_result_swapped.data[0] = key_result->data[3];
    for (i = 0; i < (ARRAY_LENGTH(key_result->data) - 4); i++)
        key_result_swapped.data[4 + i] = key_result->data[ARRAY_LENGTH(key_result->data) - 1 - i];

    memcpy(key_result, &key_result_swapped, ARRAY_LENGTH(key_result->data) * sizeof(uint32_t));

    for (i = 0; i < ARRAY_LENGTH(key_result->data); i++)
    {
        key_result->data[i] = swap4bytes(key_result->data[i]);
    }

    return ag_drv_natc_eng_key_result_set(eng_idx, 0, key_result);
}

static int drv_natc_eng_command_write(uint32_t eng_idx, nat_command_t command)
{
    int time_out = POLLING_TIME_OUT;
    natc_eng_command_status command_status = {};

    command_status.command = command;
    command_status.busy = 1;
    ag_drv_natc_eng_command_status_set(eng_idx, &command_status);

    while (time_out > 0)
    {
        ag_drv_natc_eng_command_status_get(eng_idx, &command_status);
        if (!command_status.busy)
        {
            if (command_status.error)
                return BDMF_ERR_NORES;            
            if (command == natc_cmd_lookup && command_status.miss)
                return BDMF_ERR_NOENT;
            return BDMF_ERR_OK;
        }
        time_out--;
    }
    bdmf_trace("ERROR: drv_natc_command_write:  command = 0x%08x engine 0x%08x\n", command, eng_idx);
    BUG();
    return BDMF_ERR_INTERNAL;
}
#endif
#endif

static int drv_natc_eng_command_submit(nat_command_t command, natc_eng_key_result *keyword, uint32_t mask,
    uint32_t *hit_count, uint32_t *byte_count)
{
#if !defined(_CFE_) && (!defined(RDP_SIM) || defined(XRDP_EMULATION))
    int rc;
    uint8_t eng_idx = command;

    if (command == natc_cmd_add)
        return BDMF_ERR_NOT_SUPPORTED;

    rc = ag_drv_natc_eng_key_mask_set(eng_idx, mask);
    if (rc)
        goto exit;

    rc = drv_natc_eng_key_result_write(eng_idx, keyword);
    rc = rc ? rc : drv_natc_eng_command_write(eng_idx, command);
    if (rc)
        goto exit;

    if (command == natc_cmd_lookup)
    {
       if (hit_count)
           rc = ag_drv_natc_eng_hit_count_get(eng_idx, hit_count);
       if (byte_count)
           rc = rc ? rc : ag_drv_natc_eng_byte_count_get(eng_idx, byte_count);
    }

exit:
    return rc;
#else
    return BDMF_ERR_OK;
#endif
}


static inline int drv_natc_key_entry_compare(uint32_t tbl_idx, uint8_t *key_0, uint8_t *key_1)
{
    natc_tbl_config_t *tbl_config = &g_natc_tbl_cfg[tbl_idx];
    uint8_t i;

    for (i = 0; i < tbl_config->key_len; i++)
    {
        if (key_0[i] != key_1[i])
            return BDMF_ERR_OK;
    }

    return BDMF_ERR_PARM;
}

static void drv_natc_rolling_xor(uint32_t key_len, uint8_t *keyword, uint32_t *hash_idx)
{
    uint8_t i;
    uint32_t *key = (uint32_t *)keyword;

    *hash_idx = swap4bytes(HASH_FUNCTION_INIT_VAL);
    for (i = 0; i < (key_len / sizeof(uint32_t)); i++, key++)
        *hash_idx ^= swap4bytes(*key);

    *hash_idx = (*hash_idx >> 16) ^ (*hash_idx & 0xffff);
}

static inline void drv_natc_key_mask(uint8_t key_len, uint32_t mask, uint8_t *masked_key, uint8_t *key)
{
    int i;

    for (i = key_len; i > 0; --i)
    {
        if (mask & (1 << (i - 1)))
            masked_key[key_len - i] = 0;
        else
            masked_key[key_len - i] = key[key_len - i];
    }
}

static inline void drv_natc_hash_function(uint8_t tbl_idx, uint32_t mask, uint8_t *keyword, uint32_t *hash_idx)
{
    natc_tbl_config_t *tbl_config = &g_natc_tbl_cfg[tbl_idx];
    uint8_t masked_key[MAX_NATC_ENTRY_LEN] = {};

    drv_natc_key_mask(tbl_config->key_len, mask, masked_key, keyword);

    if (g_hash_mode == hash_mode_rolling_xor)
    {
        drv_natc_rolling_xor(tbl_config->key_len, masked_key, hash_idx);
    }
    else
    {
        *hash_idx = rdd_crc_bit_by_bit_natc((const uint8_t *)masked_key, tbl_config->key_len, 0);

        if (g_hash_mode == hash_mode_crc32_low)
            *hash_idx = *hash_idx & 0xffff;
        else if (g_hash_mode == hash_mode_crc32_high)
            *hash_idx = (*hash_idx >> 16);
        else
            *hash_idx = (*hash_idx >> 16) ^ (*hash_idx & 0xffff);
    }
}

static inline bdmf_boolean drv_natc_valid_bit_get(uint8_t *key_entry)
{
    uint8_t *lkp_key_ptr;
    RDD_NAT_CACHE_LKP_ENTRY_DTS lkp_key;

    lkp_key_ptr = (uint8_t *)&lkp_key;

    memcpy(lkp_key_ptr, key_entry, sizeof(RDD_NAT_CACHE_LKP_ENTRY_DTS));
#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(lkp_key_ptr, sizeof(RDD_NAT_CACHE_LKP_ENTRY_DTS));
#endif
    return lkp_key.valid;
}

#define NATC_KEY_ENTRY(tbl_config, entry_idx) \
    ((uint8_t *)(tbl_config->vir_addr.key + (tbl_config->key_len * entry_idx)))
#define NATC_RES_ENTRY(tbl_config, entry_idx) \
    ((uint8_t *)(tbl_config->vir_addr.res + (tbl_config->res_len * entry_idx)))
static inline int drv_natc_tbl_config_get(uint8_t tbl_idx, uint32_t entry_idx, natc_tbl_config_t **tbl_config)
{
    if (tbl_idx >= g_natc_tbls_num)
        return BDMF_ERR_RANGE;

    *tbl_config = &g_natc_tbl_cfg[tbl_idx];

    if (entry_idx >= ((*tbl_config)->res_tbl_size / (*tbl_config)->res_len))
        return BDMF_ERR_RANGE;

    return 0;
}

int drv_natc_key_entry_get(uint8_t tbl_idx, uint32_t entry_idx, bdmf_boolean *valid, uint8_t *key)
{
    natc_tbl_config_t *tbl_config;
    uint8_t key_entry[MAX_NATC_ENTRY_LEN] = {};
    int rc;

    rc = drv_natc_tbl_config_get(tbl_idx, entry_idx, &tbl_config);
    if (rc)
        return rc;

    MREAD_BLK_8(key_entry, NATC_KEY_ENTRY(tbl_config, entry_idx), tbl_config->key_len);

#ifndef XRDP_EMULATION
    *valid = drv_natc_valid_bit_get(key_entry);

    if (*valid)
        memcpy(key, key_entry, tbl_config->key_len);
    else
        rc = BDMF_ERR_NOENT;
#else
    *valid = 0;
    memcpy(key, key_entry, tbl_config->key_len);
#endif

    /* if found on cache, statistics are there as well */
    return rc;
}

/* this function finds the next empty hash key entry */
static int drv_natc_find_empty_hash_key_entry(uint8_t tbl_idx, uint8_t *keyword, uint32_t *hash_idx)
{
    uint8_t key_entry[MAX_NATC_ENTRY_LEN] = {};
    uint32_t depth, key_table_entry_num;
    natc_tbl_config_t *tbl_config;
    bdmf_boolean valid = 0;
    int empty_hash_index = -1;
    bdmf_error_t rc = BDMF_ERR_OK;

    if (tbl_idx >= g_natc_tbls_num)
        return BDMF_ERR_RANGE;
    tbl_config = &g_natc_tbl_cfg[tbl_idx];

    key_table_entry_num = (tbl_config->key_tbl_size / tbl_config->key_len);

    for (depth = 0; depth < RDD_NAT_CACHE_LOOKUP_DEPTH_SIZE; depth++)
    {
        drv_natc_key_entry_get(tbl_idx, (*hash_idx + depth) % key_table_entry_num, &valid, key_entry);
        if (!valid)
        {
            if (empty_hash_index < 0)
                empty_hash_index = (*hash_idx + depth) % key_table_entry_num;
            continue;
        }
        if (drv_natc_key_entry_compare(tbl_idx, keyword, key_entry))
        {
            empty_hash_index = (*hash_idx + depth) % key_table_entry_num;
            rc = BDMF_ERR_ALREADY;
            break;
        }
    }

    if (empty_hash_index < 0)
        return BDMF_ERR_NOENT;

    *hash_idx = empty_hash_index;

    return rc;
}

int drv_natc_key_idx_get(uint8_t tbl_idx, uint8_t *keyword, uint32_t *hash_idx)
{
    if (tbl_idx >= g_natc_tbls_num)
        return BDMF_ERR_RANGE;

    drv_natc_hash_function(tbl_idx, g_natc_tbl_cfg[tbl_idx].mask, keyword, hash_idx);

    return drv_natc_find_empty_hash_key_entry(tbl_idx, keyword, hash_idx);
}

int drv_natc_result_entry_get(uint8_t tbl_idx, uint32_t entry_idx, uint8_t *res)
{
    natc_tbl_config_t *tbl_config;
    int rc;

    rc = drv_natc_tbl_config_get(tbl_idx, entry_idx, &tbl_config);
    if (rc)
        return rc;

    bdmf_fastlock_lock(&nat_cache_lock);

    memcpy(res, NATC_RES_ENTRY(tbl_config, entry_idx), tbl_config->res_len);

    bdmf_fastlock_unlock(&nat_cache_lock);

    /* if found on cache, statistics are there as well */
    return BDMF_ERR_OK;
}

int drv_natc_entry_counters_get(uint8_t tbl_idx, uint32_t entry_idx, uint32_t *hit_count, uint32_t *byte_count)
{
    natc_eng_key_result key_result = {};
    bdmf_boolean valid = 0;
    bdmf_error_t rc = BDMF_ERR_OK;
    natc_tbl_config_t *tbl_config = NULL;
    uint8_t *tbl_res;

    bdmf_fastlock_lock(&nat_cache_lock);

    rc = drv_natc_key_entry_get(tbl_idx, entry_idx, &valid, (uint8_t *)key_result.data);
    if (rc)
    {
        bdmf_fastlock_unlock(&nat_cache_lock);
        return rc;
    }

    rc = drv_natc_tbl_config_get(tbl_idx, entry_idx, &tbl_config);

    rc = rc ? rc : drv_natc_eng_command_submit(natc_cmd_lookup, &key_result, tbl_config->mask, hit_count, byte_count);

    if (rc == BDMF_ERR_NOENT)
    {
        /* Entry not in cache, read from DDR */
        tbl_res = NATC_RES_ENTRY(tbl_config, entry_idx);
        RDD_NATC_COUNTERS_ENTRY_HIT_COUNTER_READ(*hit_count, tbl_res);
        RDD_NATC_COUNTERS_ENTRY_BYTES_COUNTER_READ(*byte_count, tbl_res);
        rc  = 0;
    }

    bdmf_fastlock_unlock(&nat_cache_lock);
    return rc;
}

int drv_natc_result_entry_add(uint8_t tbl_idx, uint32_t entry_idx, uint8_t *result)
{
    natc_tbl_config_t *tbl_config;
    bdmf_error_t rc = BDMF_ERR_OK;

    rc = drv_natc_tbl_config_get(tbl_idx, entry_idx, &tbl_config);
    if (rc)
        return rc;

    MWRITE_BLK_8(NATC_RES_ENTRY(tbl_config, entry_idx), result, tbl_config->res_len);

    return rc;
}

int drv_natc_key_entry_add(uint8_t tbl_idx, uint32_t entry_idx, uint8_t *key)
{
    natc_tbl_config_t *tbl_config;
    bdmf_error_t rc = BDMF_ERR_OK;

    rc = drv_natc_tbl_config_get(tbl_idx, entry_idx, &tbl_config);
    if (rc)
        return rc;

    MWRITE_BLK_8(NATC_KEY_ENTRY(tbl_config, entry_idx), key, tbl_config->key_len);

    return rc;
}

int drv_natc_key_result_entry_add(uint8_t tbl_idx, uint8_t *key, uint8_t *result, uint32_t *hash_idx)
{
    bdmf_error_t rc = BDMF_ERR_OK;

    bdmf_fastlock_lock(&nat_cache_lock);

    rc = drv_natc_key_idx_get(tbl_idx, key, hash_idx);

    if (rc == BDMF_ERR_NOENT || rc == BDMF_ERR_ALREADY)
        rc = BDMF_ERR_PARM;

    rc = rc ? rc : drv_natc_result_entry_add(tbl_idx, *hash_idx, result);
    rc = rc ? rc : drv_natc_key_entry_add(tbl_idx, *hash_idx, key);

    bdmf_fastlock_unlock(&nat_cache_lock);

    return rc;
}

int drv_natc_entry_delete(uint8_t tbl_idx, uint32_t entry_idx)
{
    natc_eng_key_result del_key_result = {};
    natc_tbl_config_t *tbl_config;
    uint8_t *key, *result;
    bdmf_error_t rc = BDMF_ERR_OK;

    rc = drv_natc_tbl_config_get(tbl_idx, entry_idx, &tbl_config);
    if (rc)
        return rc;

    key = NATC_KEY_ENTRY(tbl_config, entry_idx);
    result = NATC_RES_ENTRY(tbl_config, entry_idx);

    bdmf_fastlock_lock(&nat_cache_lock);

    if (!drv_natc_valid_bit_get(key))
    {
        bdmf_trace("Delete failed - Invalid DDR entry\n");
        bdmf_fastlock_unlock(&nat_cache_lock);
        return BDMF_ERR_PARM;
    }

#ifndef XRDP_EMULATION
    MREAD_BLK_8(&del_key_result, key, tbl_config->key_len);
#endif

    MEMSET(key, 0, tbl_config->key_len);
    MEMSET(result, 0, tbl_config->res_len);

    /* look for the entry in the NAT cache internal memory if its there delete it */
    rc = drv_natc_eng_command_submit(natc_cmd_del, &del_key_result, tbl_config->mask, NULL, NULL);

    bdmf_fastlock_unlock(&nat_cache_lock);

    return rc == BDMF_ERR_INTERNAL ? rc : BDMF_ERR_OK;
}

int drv_natc_tbl_ctrl_set(uint8_t tbl_idx, tbl_control_t *tbl_ctrl)
{
    if (tbl_idx >= g_natc_tbls_num)
        return BDMF_ERR_RANGE;

    return ag_drv_natc_table_control_set(tbl_idx, (tbl_ctrl->key_len << 1) | tbl_ctrl->non_cacheable);
}

int drv_natc_tbl_ctrl_get(uint8_t tbl_idx, tbl_control_t *tbl_ctrl)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t table_control = 0;

    if (tbl_idx >= g_natc_tbls_num)
        return BDMF_ERR_RANGE;

    rc = ag_drv_natc_table_control_get(tbl_idx, &table_control);
    if (!rc)
    {
        tbl_ctrl->key_len = (table_control & 0x2) >> 0x1;
        tbl_ctrl->non_cacheable = (table_control & 0x1);
    }

    return rc;
}


int drv_natc_init(natc_config_t *cfg)
{
    uint8_t tbl_idx;
    uint32_t addr_hi, addr_lo;
#ifndef RDP_SIM
    uint32_t cache_entry_idx;
    natc_indir_data indir_data = {};
#endif
    bdmf_error_t rc = BDMF_ERR_OK;

    g_natc_tbls_num = cfg->tbl_num;
    for (tbl_idx = 0; tbl_idx < g_natc_tbls_num; tbl_idx++)
    {
        rc = rc ? rc : drv_natc_tbl_ctrl_set(tbl_idx, &cfg->tbl_cntrl[tbl_idx]);
        GET_ADDR_HIGH_LOW(addr_hi, addr_lo, cfg->tbl_cfg[tbl_idx].phy_addr.key);
        rc = rc ? rc : ag_drv_natc_cfg_key_addr_set(tbl_idx, addr_lo >> 3, addr_hi);
        GET_ADDR_HIGH_LOW(addr_hi, addr_lo, cfg->tbl_cfg[tbl_idx].phy_addr.res);
        rc = rc ? rc : ag_drv_natc_cfg_res_addr_set(tbl_idx, addr_lo >> 3, addr_hi);

        /* initialize ddr table */
#ifndef XRDP_EMULATION
        MEMSET((uint8_t *)cfg->tbl_cfg[tbl_idx].vir_addr.key, 0, cfg->tbl_cfg[tbl_idx].key_tbl_size);
        MEMSET((uint8_t *)cfg->tbl_cfg[tbl_idx].vir_addr.res, 0, cfg->tbl_cfg[tbl_idx].res_tbl_size);
#endif
        memcpy(&g_natc_tbl_cfg[tbl_idx], &cfg->tbl_cfg[tbl_idx], sizeof(natc_tbl_config_t));
    }

#ifndef RDP_SIM
    /* init cache entries and their stats */
    for (cache_entry_idx = 0; cache_entry_idx < NATC_CACHE_ENTRIES_NUM; cache_entry_idx++)
    {
        ag_drv_natc_indir_data_set(0, &indir_data);
        ag_drv_natc_indir_addr_set(1, cache_entry_idx);
    }
#endif

    rdd_crc_init();

    /* assume ddr and nat hash modes are the same */
    g_hash_mode = cfg->ctrl_status.nat_hash_mode;
    return rc ? rc : ag_drv_natc_ctrl_status_set(&cfg->ctrl_status);
}



/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/

#ifdef USE_BDMF_SHELL
#define TABLE_CTRL_KEY_LEN_GET(tbl_ctrl)       (NATC_TABLE_KEY_16B * (1 + ((tbl_ctrl & 0x2) >> 0x1)))
#define TABLE_CTRL_NON_CACHEABLE_GET(tbl_ctrl) (tbl_ctrl & 0x1)

static int _drv_natc_tbl_ctrl_cli_get(bdmf_session_handle session, uint8_t tbl_idx)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t tbl_ctrl = 0;

    rc = ag_drv_natc_table_control_get(tbl_idx, &tbl_ctrl);
    if (!rc)
    {
        bdmf_session_print(session, "key length:   %d\n", TABLE_CTRL_KEY_LEN_GET(tbl_ctrl));
        bdmf_session_print(session, "non cachable: %d\n", TABLE_CTRL_NON_CACHEABLE_GET(tbl_ctrl));
    }
    return rc;
}

int drv_natc_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t tbl_idx;
    static uint32_t natc_tbl_cfg[] = {cli_natc_cfg_key_addr, cli_natc_cfg_res_addr};
    static uint32_t natc_status[] = {cli_natc_ctrl_status};
    bdmf_error_t rc = BDMF_ERR_OK;

    bdmf_session_print(session, "NATC configurations:\n");
    for (tbl_idx = 0; tbl_idx < g_natc_tbls_num; tbl_idx++)
    {
        natc_tbl_config_t *cfg = &g_natc_tbl_cfg[tbl_idx];
        bdmf_session_print(session, "\nTable [%d]:\n", tbl_idx);
        bdmf_session_print(session, "\nVir: key_tbl = 0x%lx\tres_tbl = 0x%lx\t ", (long unsigned int)(uintptr_t)cfg->vir_addr.key, (long unsigned int)(uintptr_t)cfg->vir_addr.res);
        rc = rc ? rc : _drv_natc_tbl_ctrl_cli_get(session, tbl_idx);
        HAL_CLI_IDX_PRINT_LIST(session, natc_cfg, natc_tbl_cfg, tbl_idx);
    }
    HAL_CLI_PRINT_LIST(session, natc, natc_status);
    return rc;
}

int drv_natc_cli_cache_dump(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t print_cache_idx, cache_entry_idx, i;
    int time_out = POLLING_TIME_OUT;
    natc_indir_data indir_data = {};

    /* init cache entries and their stats */
    for (cache_entry_idx = 0; cache_entry_idx < NATC_CACHE_ENTRIES_NUM; cache_entry_idx++)
    {
        print_cache_idx = 1;
        rc = ag_drv_natc_indir_addr_set(0, cache_entry_idx);
        if (rc)
            return rc;
        while (time_out > 0)
            time_out--;
        rc = ag_drv_natc_indir_data_get(0, &indir_data);
        if (rc)
            return rc;
        for (i = 0; i < ARRAY_LENGTH(indir_data.data); i++)
        {
            if (indir_data.data[i] == 0)
                continue;
            if (print_cache_idx)
            {
                bdmf_trace("cache entry[%d]:\n", cache_entry_idx);
                print_cache_idx = 0;
            }
            bdmf_trace("[%d] 0x%08x\n", i, indir_data.data[i]);
        }
    }
    return rc;
}

static natc_ctrs_natc_ctrs g_natc_ctrs = {};

int drv_natc_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t tbl_idx, prcnt;
    uint32_t miss_diff, hit_diff;
    uint64_t natc_total_access;
    natc_ctrs_natc_ctrs natc_ctrs = {};
    static uint32_t natc_ctrs_cli[] = {cli_natc_ctrs_natc_ctrs};
    bdmf_error_t rc = BDMF_ERR_OK;

    bdmf_session_print(session, "NATC debug stats:\n\r");
    for (tbl_idx = 0; !rc && tbl_idx < g_natc_tbls_num; tbl_idx++)
    {
        bdmf_session_print(session, "\nTable [%d]:\n", tbl_idx);
        HAL_CLI_IDX_PRINT_LIST(session, natc_ctrs, natc_ctrs_cli, tbl_idx);

        rc = ag_drv_natc_ctrs_natc_ctrs_get(tbl_idx, &natc_ctrs);
        g_natc_ctrs.ddr_request_count = natc_ctrs.ddr_request_count - g_natc_ctrs.ddr_request_count;
        miss_diff = natc_ctrs.cache_miss_count - g_natc_ctrs.cache_miss_count;
        hit_diff = natc_ctrs.cache_hit_count - g_natc_ctrs.cache_hit_count;
        natc_total_access = hit_diff + miss_diff;

        memcpy((uint8_t *)&g_natc_ctrs, (uint8_t *)&natc_ctrs, sizeof(natc_ctrs_natc_ctrs));

        prcnt = ((100 * hit_diff) / natc_total_access);
        bdmf_session_print(session, "Hit ratio: %3u%%\n", prcnt);
        prcnt = ((100 * miss_diff) / natc_total_access);
        bdmf_session_print(session, "Miss ratio: %3u%%\n", prcnt);
    }

    return rc;
}

static bdmfmon_handle_t natc_dir;
void drv_natc_cli_init(bdmfmon_handle_t driver_dir)
{
    if ((natc_dir = bdmfmon_dir_find(driver_dir, "natc"))!=NULL)
        return;
    natc_dir = bdmfmon_dir_add(driver_dir, "natc", "natc driver", BDMF_ACCESS_ADMIN, NULL);

    ag_drv_natc_cli_init(natc_dir);
    ag_drv_natc_cfg_cli_init(natc_dir);
    ag_drv_natc_eng_cli_init(natc_dir);
    ag_drv_natc_indir_cli_init(natc_dir);
    ag_drv_natc_ctrs_cli_init(natc_dir);

    BDMFMON_MAKE_CMD_NOPARM(natc_dir, "cfg_get", "natc configuration", (bdmfmon_cmd_cb_t)drv_natc_cli_config_get);
    BDMFMON_MAKE_CMD_NOPARM(natc_dir, "dbg_get", "natc configuration", (bdmfmon_cmd_cb_t)drv_natc_cli_debug_get);
    BDMFMON_MAKE_CMD_NOPARM(natc_dir, "cd", "print non-zero cache entries", (bdmfmon_cmd_cb_t)drv_natc_cli_cache_dump);
}

void drv_natc_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (natc_dir)
    {
        bdmfmon_token_destroy(natc_dir);
        natc_dir = NULL;
    }
}

#endif /* USE_BDMF_SHELL */

