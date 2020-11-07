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
#include "access_macros.h"
#include "rdd_common.h"
#include "rdp_drv_natc.h"
#include "rdd_natc.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_data_structures_auto.h"

#ifdef SW_CACHE_MISS_HANDLE
#ifndef G9991
#include "rdd_tuple_lkp.h"
#endif
#define DELETE_IDX(entry_idx) \
    ((entry_idx) + (NATC_LOOKUP_DEPTH_SIZE - ((entry_idx) % NATC_LOOKUP_DEPTH_SIZE)) % NATC_LOOKUP_DEPTH_SIZE) % (tbl_config->key_tbl_size / tbl_config->key_len)
#endif

#define POLLING_TIME_OUT  1000
#define NATC_NUM_OF_BITS_IN_HIT_COUNT   28
#define NATC_NUM_OF_BITS_IN_BYTE_COUNT  36

static uint8_t g_hash_mode;
#if defined(BCM63158)
uint32_t total_length_bytes[] = {48, 64, 80, 96, 112, 128, 144, 160};
#else
uint32_t total_length_bytes[6] = {48, 64, 80, 96, 112, 128};
#endif
natc_tbl_config_t g_natc_tbl_cfg[NATC_MAX_TABLES_NUM] = {};
uint32_t g_natc_tbls_num = 0;


#define NATC_KEY_ENTRY(tbl_config, entry_idx) \
    ((uint8_t *)(tbl_config->vir_addr.key + (tbl_config->key_len * entry_idx)))
#define NATC_RES_ENTRY(tbl_config, entry_idx) \
    ((uint8_t *)(tbl_config->vir_addr.res + (tbl_config->res_len * entry_idx)))
static inline int drv_natc_tbl_config_get(uint8_t tbl_idx, natc_tbl_config_t **tbl_config)
{
    if (tbl_idx >= g_natc_tbls_num)
        return BDMF_ERR_RANGE;

    *tbl_config = &g_natc_tbl_cfg[tbl_idx];

    return 0;
}

struct bdmfmon_enum_val natc_enum_table[] = {
    {"NATC_DS_TBL", 0},
#ifndef G9991
    {"NATC_US_TBL", 1},
    {"NATC_COUNTERS_TBL", 2},
#else
    {"NATC_COUNTERS_TBL", 1},
#endif
    {"NATC_VLAN_ACTION_TBL", 0},
    {NULL, 0},
};

#ifndef _CFE_
bdmf_fastlock nat_cache_lock;
#if !defined(RDP_SIM) || defined(XRDP_EMULATION)


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

static int drv_natc_eng_command_write(uint32_t eng_idx, uint8_t tbl_idx, nat_command_t command, uint8_t cache_flush)
{
    int time_out = POLLING_TIME_OUT;
    natc_eng_command_status command_status = {};

    memset(&command_status, 0 , sizeof(natc_eng_command_status));
    
    command_status.command = command;
    command_status.nat_tbl = tbl_idx;
    command_status.cache_flush = cache_flush;
   
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

static int drv_natc_eng_command_submit(nat_command_t command, uint8_t tbl_idx, uint32_t entry_idx, natc_eng_key_result *keyword, uint32_t mask,
    uint64_t *hit_count, uint64_t *byte_count)
{
#if !defined(_CFE_) && (!defined(RDP_SIM) || defined(XRDP_EMULATION))
    int rc;
#if !defined(DUAL_ISSUE)
    uint8_t eng_idx = command;	 
#else
    uint8_t eng_idx = 0;
#endif
    uint32_t hash_idx;  
    uint8_t *tbl_res;
    natc_tbl_config_t *tbl_config = NULL;
    uint32_t  hit_count_hw, byte_count_hw;

    if (command == natc_cmd_add)
        return BDMF_ERR_NOT_SUPPORTED;

    rc = drv_natc_eng_key_result_write(eng_idx, keyword);
    rc = rc ? rc : drv_natc_eng_command_write(eng_idx, tbl_idx, command, 0);
    if (rc)
        goto exit;

    if (command == natc_cmd_lookup)
    {
       /* flush counters to DDR and read from it */        
       rc = ag_drv_natc_eng_hash_get(eng_idx, &hash_idx);        
       
       rc = rc ? rc : drv_natc_tbl_config_get(tbl_idx, &tbl_config);
       
       if (!rc)
       {
           /* set key result to hash index for flush command */
           if (tbl_config->key_len == NATC_TABLE_KEY_16B)        
               keyword->data[3] = (hash_idx & 0x3ff) << 22;
           else
               keyword->data[7] = (hash_idx & 0x3ff) << 22;
       }
       
       rc = rc ? rc : ag_drv_natc_eng_key_result_set(eng_idx, 0, keyword);
       rc = rc ? rc : drv_natc_eng_command_write(eng_idx, tbl_idx, command, 1);
         
       if (!rc)
       {                      
          /* Entry was in cache and flushed to DDR - read from DDR */
          tbl_res = NATC_RES_ENTRY(tbl_config, entry_idx);
         
          RDD_NATC_COUNTERS_ENTRY_HIT_COUNTER_READ(hit_count_hw, tbl_res);
        
          if (hit_count)
            *hit_count = (uint64_t) hit_count_hw & ((1UL<< NATC_NUM_OF_BITS_IN_HIT_COUNT) - 1);        
             
          if (byte_count)
          {
            RDD_NATC_COUNTERS_ENTRY_BYTES_COUNTER_READ(byte_count_hw, tbl_res);
            *byte_count = (uint64_t) byte_count_hw << (NATC_NUM_OF_BITS_IN_BYTE_COUNT - 32);
            *byte_count |= ((uint64_t) hit_count_hw >> NATC_NUM_OF_BITS_IN_HIT_COUNT);
          }       
       }
    }

exit:
    return rc;
#else
    return BDMF_ERR_OK;
#endif
}


static inline int drv_natc_key_entry_compare(uint32_t tbl_idx, uint8_t *key_0, uint8_t *key_1, uint16_t mask)
{
    natc_tbl_config_t *tbl_config = &g_natc_tbl_cfg[tbl_idx];
    uint8_t i;

    for (i = 0; i < tbl_config->key_len; i++)
    {
        if ((key_0[i] != key_1[i]) && (!(mask & (1 << (tbl_config->key_len - i - 1)))))
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

static inline int drv_natc_hash_reduce(uint32_t *hash_idx, uint8_t natc_ddr_size)
{
    switch (natc_ddr_size)
    {
    case ddr_size_8k:
        *hash_idx = (*hash_idx & 0x1FFF) ^ ((*hash_idx >> 13) & 0x1FFF) ^ ((*hash_idx >> 26) & 0x3F);
        break;
    case ddr_size_16k:
        *hash_idx = (*hash_idx & 0x3FFF) ^ ((*hash_idx >> 14) & 0x3FFF) ^ ((*hash_idx >> 28) & 0xF);
        break;
    case ddr_size_32k:
        *hash_idx = (*hash_idx & 0x7FFF) ^ ((*hash_idx >> 15) & 0x7FFF) ^ ((*hash_idx >> 30) & 0x3);
        break;
    case ddr_size_64k:
        *hash_idx = (*hash_idx & 0xFFFF) ^ ((*hash_idx >> 16) & 0xFFFF);
        break;
    case ddr_size_128k:
        *hash_idx = (*hash_idx & 0x1FFFF) ^ ((*hash_idx >> 17) & 0x7FFF);
        break;
    case ddr_size_256k:
        *hash_idx = (*hash_idx & 0x3FFFF) ^ ((*hash_idx >> 18) & 0x3FFF);
        break;
    default:
        return -1;
    }
    
    return 0;
}

static inline int drv_natc_hash_function(uint8_t tbl_idx, uint32_t mask, uint8_t *keyword, uint32_t *hash_idx)
{
    natc_tbl_config_t *tbl_config = &g_natc_tbl_cfg[tbl_idx];
    uint8_t masked_key[NATC_MAX_ENTRY_LEN] = {};
    natc_ddr_cfg_natc_ddr_size ddr_size;
    uint8_t natc_ddr_size;
    uint8_t hash_shifts;
    uint32_t hash_mask;
    uint8_t* p_ddr_size;

    drv_natc_key_mask(tbl_config->key_len, mask, masked_key, keyword);
	
    ag_drv_natc_ddr_cfg_natc_ddr_size_get(&ddr_size); 
    p_ddr_size = (uint8_t*)&ddr_size.ddr_size_tbl0;
    natc_ddr_size = p_ddr_size[tbl_idx];

    if (g_hash_mode == hash_mode_rolling_xor)
    {
        drv_natc_rolling_xor(tbl_config->key_len, masked_key, hash_idx);
        drv_natc_hash_reduce(hash_idx, natc_ddr_size);
    }
    else
    {
        *hash_idx = rdd_crc_bit_by_bit_natc((const uint8_t *)masked_key, tbl_config->key_len, 0);

        if (g_hash_mode == hash_mode_crc32_low)
        {
            hash_mask = (1 << (13 + natc_ddr_size)) - 1;
            *hash_idx = *hash_idx & hash_mask;
        }
        else if (g_hash_mode == hash_mode_crc32_high)
        {
            hash_mask = (1 << (13 + natc_ddr_size)) - 1;
            hash_shifts = 19 - natc_ddr_size;
            *hash_idx = (*hash_idx >> hash_shifts) & hash_mask;
        }
        else
        {
            drv_natc_hash_reduce(hash_idx, natc_ddr_size);
        }
    }
       
    return 0;
}

static inline bdmf_boolean drv_natc_valid_bit_get(uint8_t *key_entry)
{
    uint8_t *lkp_key_ptr;
    RDD_NAT_CACHE_L3_LKP_ENTRY_DTS lkp_key;

    lkp_key_ptr = (uint8_t *)&lkp_key;

    MREAD_BLK_8(lkp_key_ptr, key_entry, sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));
#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(lkp_key_ptr, sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));
#endif
    return lkp_key.valid;
}

int drv_natc_key_entry_get(uint8_t tbl_idx, uint32_t entry_idx, bdmf_boolean *valid, uint8_t *key)
{
    natc_tbl_config_t *tbl_config;
    uint8_t key_entry[NATC_MAX_ENTRY_LEN] = {};
    int rc;

    rc = drv_natc_tbl_config_get(tbl_idx, &tbl_config);
    if (rc)
        return rc;

    MREAD_BLK_8(key_entry, NATC_KEY_ENTRY(tbl_config, entry_idx), tbl_config->key_len);

#if !defined(XRDP_EMULATION)
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
static int drv_natc_find_empty_hash_key_entry(uint8_t tbl_idx, uint8_t *keyword, uint32_t *hash_idx, uint32_t *empty_idx)
{
    uint8_t key_entry[NATC_MAX_ENTRY_LEN] = {};
    uint32_t depth, key_table_entry_num, key_bin_total_entry_num;
    natc_tbl_config_t *tbl_config;
    bdmf_boolean valid = 0;
    int empty_hash_index = -1;
    bdmf_error_t rc = BDMF_ERR_OK;

    if (tbl_idx >= g_natc_tbls_num)
        return BDMF_ERR_RANGE;
    tbl_config = &g_natc_tbl_cfg[tbl_idx];

    key_table_entry_num = tbl_config->tbl_entry_num;
    key_bin_total_entry_num = (tbl_config->key_tbl_size / tbl_config->key_len);

    for (depth = 0; depth < NATC_LOOKUP_DEPTH_SIZE; depth++)
    {
#ifdef SW_CACHE_MISS_HANDLE
        /* reserve modulo NATC_LOOKUP_DEPTH_SIZE entries for miss entries from runner */
        if (((*hash_idx + depth) % NATC_LOOKUP_DEPTH_SIZE) == 0)
            continue;
#endif
        if ((*hash_idx + depth) >= key_bin_total_entry_num)
        {
            /* hash + depth can't go beyond bin table, otherwise some error happens already */
            bdmf_trace("ERROR: drv_natc_find_empty_hash_key_entry: entry_index out of range\n");
            BUG();
            return BDMF_ERR_ALREADY;
        }

        drv_natc_key_entry_get(tbl_idx, (*hash_idx + depth), &valid, key_entry);
        if (!valid)
        {
            if (empty_hash_index < 0)
                empty_hash_index = (*hash_idx + depth);
            continue;
        }
        if (drv_natc_key_entry_compare(tbl_idx, keyword, key_entry, tbl_config->mask))
        {
            empty_hash_index = (*hash_idx + depth);
            rc = BDMF_ERR_ALREADY;
            break;
        }
    }

    if (empty_hash_index < 0)
        return BDMF_ERR_NOENT;

    /* In current impl, two flow tables, DS and US, are concatenated into 64K flow_index. That doesn't
       take bins into consideration. The actual flow_index range should (32K+7bins)*2. A thorough solution
       will be put in to support key+bin as well as more than 2 tables. In the meantime, limit the index
       assignment in the range of 32K, and don't use bin */
    if (empty_hash_index >= key_table_entry_num)
        return BDMF_ERR_NOENT;

    *empty_idx = empty_hash_index;

    return rc;
}

#ifdef SW_CACHE_MISS_HANDLE
static int _drv_natc_entry_delete(uint8_t tbl_idx, uint32_t delete_idx, natc_tbl_config_t *tbl_config, uint8_t *key, 
    uint32_t entry_idx, natc_eng_key_result *del_key_result)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_boolean valid = 0;
    uint8_t key_entry[NATC_MAX_ENTRY_LEN] = {};

    drv_natc_key_entry_get(tbl_idx, delete_idx, &valid, key_entry);

    if (valid && drv_natc_key_entry_compare(tbl_idx, key, key_entry, tbl_config->mask))
    {
        /* delete entry even for case add entry not succeed to prevent fw<->sw natc races */
        rc = drv_natc_entry_delete(tbl_idx, delete_idx, 0, 0);
    }
    /* delete entry from cache anyway - possible that it is not in DDR and in cache only */
    if (!rc)
    {
#ifndef XRDP_EMULATION
        MREAD_BLK_8(del_key_result, key, tbl_config->key_len);
#endif
        /* look for the entry in the NAT cache internal memory and try to delete it if its there */
        drv_natc_eng_command_submit(natc_cmd_del, tbl_idx, entry_idx, del_key_result, tbl_config->mask, NULL, NULL);
    }

    return rc;
}
#endif

int drv_natc_key_idx_get(uint8_t tbl_idx, uint8_t *keyword, uint32_t *hash_idx, uint32_t *entry_idx)
{
    int rc;

    if (tbl_idx >= g_natc_tbls_num)
        return BDMF_ERR_RANGE;

    rc = drv_natc_hash_function(tbl_idx, g_natc_tbl_cfg[tbl_idx].mask, keyword, hash_idx);
    if (rc)
        return rc;
    return drv_natc_find_empty_hash_key_entry(tbl_idx, keyword, hash_idx, entry_idx);
}

int drv_natc_result_entry_get(uint8_t tbl_idx, uint32_t entry_idx, uint8_t *res)
{
    natc_tbl_config_t *tbl_config;
    int rc;

    rc = drv_natc_tbl_config_get(tbl_idx, &tbl_config);
    if (rc)
        return rc;

    bdmf_fastlock_lock(&nat_cache_lock);

    MREAD_BLK_8(res, NATC_RES_ENTRY(tbl_config, entry_idx), tbl_config->res_len);

    bdmf_fastlock_unlock(&nat_cache_lock);

    /* if found on cache, statistics are there as well */
    return BDMF_ERR_OK;
}

int drv_natc_entry_counters_get(uint8_t tbl_idx, uint32_t entry_idx, uint64_t *hit_count, uint64_t *byte_count)
{
    natc_eng_key_result key_result = {};
    bdmf_boolean valid = 0;
    bdmf_error_t rc = BDMF_ERR_OK;
    natc_tbl_config_t *tbl_config = NULL;
    uint8_t *tbl_res;
    uint32_t hit_count_hw, byte_count_hw;

    bdmf_fastlock_lock(&nat_cache_lock);

    rc = drv_natc_key_entry_get(tbl_idx, entry_idx, &valid, (uint8_t *)key_result.data);
    if (rc)
    {
        bdmf_trace("failed in drv_natc_key_entry_get rc=%d tbl_idx=%d entry_idx=%d\n",(int)rc, tbl_idx, entry_idx);
        bdmf_fastlock_unlock(&nat_cache_lock);
        return rc;
    }

    rc = drv_natc_tbl_config_get(tbl_idx, &tbl_config);
    rc = rc ? rc : drv_natc_eng_command_submit(natc_cmd_lookup, tbl_idx, entry_idx, &key_result, tbl_config->mask, hit_count, byte_count);

    if (rc == BDMF_ERR_NOENT)
    {
        /* Entry not in cache, read from DDR */
        tbl_res = NATC_RES_ENTRY(tbl_config, entry_idx);
        
        RDD_NATC_COUNTERS_ENTRY_HIT_COUNTER_READ(hit_count_hw, tbl_res);
        
        if (hit_count)
           *hit_count = (uint64_t) hit_count_hw & ((1UL<< NATC_NUM_OF_BITS_IN_HIT_COUNT) - 1);        
           
        if (byte_count)
        {
          RDD_NATC_COUNTERS_ENTRY_BYTES_COUNTER_READ(byte_count_hw, tbl_res);
           *byte_count = (uint64_t) byte_count_hw << (NATC_NUM_OF_BITS_IN_BYTE_COUNT - 32);
           *byte_count |= ((uint64_t) hit_count_hw >> NATC_NUM_OF_BITS_IN_HIT_COUNT);
        }
                
        rc  = BDMF_ERR_OK;
    }

    bdmf_fastlock_unlock(&nat_cache_lock);
    return rc;
}

int drv_natc_result_entry_add(uint8_t tbl_idx, uint32_t entry_idx, uint8_t *result)
{
    natc_tbl_config_t *tbl_config;
    bdmf_error_t rc;

    rc = drv_natc_tbl_config_get(tbl_idx, &tbl_config);
    if (rc)
        return rc;

    MWRITE_BLK_8(NATC_RES_ENTRY(tbl_config, entry_idx), result, tbl_config->res_len);

    return rc;
}

int drv_natc_result_entry_modify(uint8_t tbl_idx, uint32_t entry_idx, uint8_t *result)
{
    natc_tbl_config_t *tbl_config;
    bdmf_error_t rc;
    bdmf_boolean valid = 0;
    natc_eng_key_result key_result = {};
    uint8_t *tbl_key, *tbl_res;
    int tbl_len;

    rc = drv_natc_tbl_config_get(tbl_idx, &tbl_config);
    if (rc)
        return rc;

    rc = drv_natc_key_entry_get(tbl_idx, entry_idx, &valid, (uint8_t *)key_result.data);
    if (rc)
        return rc;

    tbl_key = NATC_KEY_ENTRY(tbl_config, entry_idx);
    tbl_res = NATC_RES_ENTRY(tbl_config, entry_idx);
    tbl_len = tbl_config->res_len;

    /* move pointers ahead by 8 bytes to skip rewriting stats to 0 */
    result  += sizeof(RDD_NATC_COUNTERS_ENTRY_DTS);
    tbl_res += sizeof(RDD_NATC_COUNTERS_ENTRY_DTS);
    tbl_len -= sizeof(RDD_NATC_COUNTERS_ENTRY_DTS);

    MWRITE_BLK_8(tbl_res, result, tbl_len);

    /* if valid, flush flow from natc */
    if (valid)
    {
#if !defined(_CFE_) && (!defined(RDP_SIM) || defined(XRDP_EMULATION))
        /* flush stats from natc to ddr entry */
        drv_natc_eng_command_write(natc_cmd_lookup, tbl_idx, natc_cmd_lookup, 1);
#endif

        /* delete entry from natc */
#ifndef XRDP_EMULATION
        MREAD_BLK_8(&key_result, tbl_key, tbl_config->key_len);
#endif
        rc = drv_natc_eng_command_submit(natc_cmd_del, tbl_idx, entry_idx, &key_result, tbl_config->mask, NULL, NULL);
        rc = (rc == BDMF_ERR_INTERNAL) ? rc : BDMF_ERR_OK;
    }

    return rc;
}

int drv_natc_key_entry_add(uint8_t tbl_idx, uint32_t entry_idx, uint8_t *key)
{
    natc_tbl_config_t *tbl_config;
    bdmf_error_t rc = BDMF_ERR_OK;

    rc = drv_natc_tbl_config_get(tbl_idx, &tbl_config);
    if (rc)
        return rc;

    MWRITE_BLK_8(NATC_KEY_ENTRY(tbl_config, entry_idx), key, tbl_config->key_len);

    return rc;
}

int drv_natc_key_result_entry_var_size_ctx_add(uint8_t tbl_idx, uint8_t *hash_key, uint8_t *key, uint8_t *result, uint32_t *entry_idx)
{
    bdmf_error_t rc = BDMF_ERR_OK;
#ifdef SW_CACHE_MISS_HANDLE
    bdmf_error_t rc1 = BDMF_ERR_OK;
#ifndef G9991
    RDD_NAT_CACHE_L3_LKP_ENTRY_DTS lkp_key = {};
    uint8_t *lkp_key_ptr = (uint8_t *)&lkp_key;
#endif   
#endif
    natc_eng_key_result del_key_result = {};
    natc_tbl_config_t *tbl_config;
    uint32_t hash_idx;

    rc = drv_natc_tbl_config_get(tbl_idx, &tbl_config);

    if (rc)
        return rc;

    bdmf_fastlock_lock(&nat_cache_lock);

    /* hash_key does not include the variable size context field */
    rc = drv_natc_key_idx_get(tbl_idx, hash_key, &hash_idx, entry_idx);
#if defined(BCM63158)
    if (rc == BDMF_ERR_ALREADY)
        rc = BDMF_ERR_PARM;
#else
    if (rc == BDMF_ERR_NOENT || rc == BDMF_ERR_ALREADY)
        rc = BDMF_ERR_PARM;
#endif

    rc = rc ? rc : drv_natc_result_entry_add(tbl_idx, *entry_idx, result);
    rc = rc ? rc : drv_natc_key_entry_add(tbl_idx, *entry_idx, key);

    if (rc == 0)
    {
        g_natc_tbl_cfg[tbl_idx].count++;
    }

#ifdef SW_CACHE_MISS_HANDLE
    /* Check if there is no entry for flow miss and delete it if there is one */
#ifndef G9991
    MREAD_BLK_8(lkp_key_ptr, key, sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));
#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(lkp_key_ptr, sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));
#endif
    if (((tbl_idx == NATC_TBL_IDX_DS) || (tbl_idx == NATC_TBL_IDX_US)) && 
        (lkp_key.sub_table_id == NATC_SUB_TBL_IDX_L3) && 
        (lkp_key.vport == PROJ_DEFS_RDD_VPORT_ANY))
    {
        rdd_vport_id_t vport_idx;
        RDD_NAT_CACHE_L3_LKP_ENTRY_DTS keyword = {};
        uint8_t *keyword_ptr = (uint8_t *)&keyword;
        uint32_t keyword_idx;

        for (vport_idx = 0; vport_idx < RDD_VPORT_LAST; vport_idx++)
        {
            lkp_key.vport = vport_idx;
            MREAD_BLK_8(keyword_ptr, &lkp_key, sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));
#ifdef FIRMWARE_LITTLE_ENDIAN
            SWAPBYTES(keyword_ptr, sizeof(RDD_NAT_CACHE_L3_LKP_ENTRY_DTS));
#endif
            drv_natc_hash_function(tbl_idx, g_natc_tbl_cfg[tbl_idx].mask, keyword_ptr, &keyword_idx);

            if ((rc1 = _drv_natc_entry_delete(tbl_idx, DELETE_IDX(keyword_idx), tbl_config, keyword_ptr, *entry_idx, &del_key_result)))
            {
                bdmf_trace("Failed delete natc entry, error %d\n", rc);
                break;
            }
        }
    }
    else
#endif /* G9991 */
        rc1 = _drv_natc_entry_delete(tbl_idx, DELETE_IDX(hash_idx), tbl_config, key, *entry_idx, &del_key_result);
    
    bdmf_fastlock_unlock(&nat_cache_lock);
    return rc1 ? rc1 : rc;
#else
    /* remove HW natc cache miss entry */
#ifndef XRDP_EMULATION
    MREAD_BLK_8(&del_key_result, key, tbl_config->key_len);
#endif
    /* look for the entry in the NAT cache internal memory and try to delete it if its there */
    drv_natc_eng_command_submit(natc_cmd_del, tbl_idx, *entry_idx, &del_key_result, tbl_config->mask, NULL, NULL);
    bdmf_fastlock_unlock(&nat_cache_lock);
    return rc;
#endif
}

int drv_natc_key_result_entry_add(uint8_t tbl_idx, uint8_t *key, uint8_t *result, uint32_t *entry_idx)
{
    uint8_t hash_key[NATC_MAX_ENTRY_LEN] = {};

    memcpy(hash_key, key, sizeof(hash_key));
    return drv_natc_key_result_entry_var_size_ctx_add(tbl_idx, hash_key, key, result, entry_idx);
}

int drv_natc_entry_delete(uint8_t tbl_idx, uint32_t entry_idx,  uint8_t lock_req, uint8_t count_delete)
{
    natc_eng_key_result del_key_result = {};
    natc_tbl_config_t *tbl_config;
    uint8_t *key, *result;
    bdmf_error_t rc = BDMF_ERR_OK;

    rc = drv_natc_tbl_config_get(tbl_idx, &tbl_config);
    if (rc)
        return rc;

    key = NATC_KEY_ENTRY(tbl_config, entry_idx);
    result = NATC_RES_ENTRY(tbl_config, entry_idx);

    if (lock_req)
        bdmf_fastlock_lock(&nat_cache_lock);

    if (!drv_natc_valid_bit_get(key))
    {
        bdmf_trace("Delete failed - Invalid DDR entry\n");
        if (lock_req)
            bdmf_fastlock_unlock(&nat_cache_lock);
        return BDMF_ERR_PARM;
    }

#ifndef XRDP_EMULATION
    MREAD_BLK_8(&del_key_result, key, tbl_config->key_len);
#endif

    MEMSET(key, 0, tbl_config->key_len);
    MEMSET(result, 0, tbl_config->res_len);

    /* look for the entry in the NAT cache internal memory if its there delete it */
    rc = drv_natc_eng_command_submit(natc_cmd_del, tbl_idx, entry_idx, &del_key_result, tbl_config->mask, NULL, NULL);

    if (lock_req)
        bdmf_fastlock_unlock(&nat_cache_lock);

    if ((rc != BDMF_ERR_INTERNAL) && (count_delete))
    {
        g_natc_tbl_cfg[tbl_idx].count--;
    }
    return rc == BDMF_ERR_INTERNAL ? rc : BDMF_ERR_OK;
}

#if defined(USE_NATC_VAR_CONTEXT_LEN)
int drv_natc_tbl_ctrl_set(uint8_t tbl_idx, tbl_control_t *tbl_ctrl)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    natc_table_control table_control_reg;
    bdmf_boolean *table_control_field;

    if (tbl_idx >= g_natc_tbls_num)
        return BDMF_ERR_RANGE;

    rc = ag_drv_natc_table_control_get(&table_control_reg);
    if (!rc)
    {
        table_control_field = &table_control_reg.var_context_len_en_tbl0 - tbl_idx;
        table_control_field[0] = tbl_ctrl->var_context_len_en;

        table_control_field = &table_control_reg.non_cacheable_tbl0 - (tbl_idx * 2) - 1;
        table_control_field[0] = tbl_ctrl->key_len;
        table_control_field[1] = tbl_ctrl->non_cacheable;

        rc = ag_drv_natc_table_control_set(&table_control_reg);
    }

    return rc;
}

int drv_natc_tbl_ctrl_get(uint8_t tbl_idx, tbl_control_t *tbl_ctrl)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    natc_table_control table_control_reg;
    bdmf_boolean *table_control_field;

    if (tbl_idx >= g_natc_tbls_num)
        return BDMF_ERR_RANGE;

    rc = ag_drv_natc_table_control_get(&table_control_reg);
    if (!rc)
    {
        table_control_field = &table_control_reg.var_context_len_en_tbl0 - tbl_idx;
        tbl_ctrl->var_context_len_en = table_control_field[0];

        table_control_field = &table_control_reg.non_cacheable_tbl0 - (tbl_idx * 2) - 1;
        tbl_ctrl->key_len = table_control_field[0];
        tbl_ctrl->non_cacheable = table_control_field[1];
    }

    return rc;
}
#else
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
#endif

ddr_nat_table_size_t _natc_tbl_ddr_size_enum(uint8_t tbl_idx, natc_ddr_cfg_natc_ddr_size *ddr_size)
{
    switch (tbl_idx)
    {
        case 0:
            return ddr_size->ddr_size_tbl0;
        case 1:
            return ddr_size->ddr_size_tbl1;
        case 2:
            return ddr_size->ddr_size_tbl2;
        case 3:
            return ddr_size->ddr_size_tbl3;
        case 4:
            return ddr_size->ddr_size_tbl4;
        case 5:
            return ddr_size->ddr_size_tbl5;
        case 6:
            return ddr_size->ddr_size_tbl6;
        case 7:
            return ddr_size->ddr_size_tbl7;
        default:
            BDMF_TRACE_ERR("%s() : Invalid NATC table %d\n",__FUNCTION__,tbl_idx);
            break;
    }
    return 0;
}

total_length_t _natc_tbl_entry_size_enum(uint8_t tbl_idx, natc_ddr_cfg_total_len *total_len)
{
    switch (tbl_idx)
    {
        case 0:
            return total_len->total_len_tbl0;
        case 1:
            return total_len->total_len_tbl1;
        case 2:
            return total_len->total_len_tbl2;
        case 3:
            return total_len->total_len_tbl3;
        case 4:
            return total_len->total_len_tbl4;
        case 5:
            return total_len->total_len_tbl5;
        case 6:
            return total_len->total_len_tbl6;
        case 7:
            return total_len->total_len_tbl7;
        default:
            BDMF_TRACE_ERR("%s() : Invalid NATC table %d\n",__FUNCTION__,tbl_idx);
            break;
    }
    return 0;
}

#ifdef BCM_PON_XRDP
void drv_natc_cfg_build_tables(natc_config_t *cfg, natc_ddr_cfg_natc_ddr_size* ddr_size, natc_ddr_cfg_total_len* total_len, int table_base_size,
        void* table_base_addr_virt, bdmf_phys_addr_t table_base_addr_phys)
{
    int i;
    uint8_t* p_ddr_size = (uint8_t*)ddr_size;
    uint8_t* p_total_len = (uint8_t*)total_len;

    for (i = 0; i< natc_tbls_num; i++)
    {
       /* DS table */
       cfg->tbl_cfg[i].count = 0;
       cfg->tbl_cntrl[i].key_len = key_len_16B;
       cfg->tbl_cfg[i].mask = NATC_16BYTE_KEY_MASK;
       cfg->tbl_cfg[i].key_len = NATC_TABLE_KEY_16B + (NATC_TABLE_KEY_16B * cfg->tbl_cntrl[i].key_len);
       cfg->tbl_cfg[i].key_tbl_size = ((((table_base_size << p_ddr_size[i]) + ddr_8_bins_per_bucket) * cfg->tbl_cfg[i].key_len) + 0x1f) & ~0x1f;
       cfg->tbl_cfg[i].res_len = total_length_bytes[p_total_len[i]] - cfg->tbl_cfg[i].key_len;
       cfg->tbl_cfg[i].res_tbl_size = ((((table_base_size << p_ddr_size[i]) + ddr_8_bins_per_bucket) * cfg->tbl_cfg[i].res_len) + 0x1f) & ~0x1f;
       cfg->tbl_cfg[i].tbl_entry_num = table_base_size << p_ddr_size[i];
#ifndef XRDP_EMULATION
       if (i == 0)
           cfg->tbl_cfg[i].vir_addr.key = table_base_addr_virt;
       else
           cfg->tbl_cfg[i].vir_addr.key = (void *)((uint8_t *)cfg->tbl_cfg[i - 1].vir_addr.res + cfg->tbl_cfg[i - 1].res_tbl_size);
       cfg->tbl_cfg[i].vir_addr.res = (void *)((uint8_t *)cfg->tbl_cfg[i].vir_addr.key + cfg->tbl_cfg[i].key_tbl_size);
       cfg->tbl_cfg[i].phy_addr.key = RDD_RSV_VIRT_TO_PHYS(table_base_addr_virt, table_base_addr_phys, cfg->tbl_cfg[i].vir_addr.key);
       cfg->tbl_cfg[i].phy_addr.res = RDD_RSV_VIRT_TO_PHYS(table_base_addr_virt, table_base_addr_phys, cfg->tbl_cfg[i].vir_addr.res);
#else
       cfg->tbl_cfg[i].vir_addr.key = (uint32_t)bdmf_ioremap(cfg->tbl_cfg[i].phy_addr.key, cfg->tbl_cfg[i].key_tbl_size + cfg->tbl_cfg[i].res_tbl_size);
       cfg->tbl_cfg[i].vir_addr.res = cfg->tbl_cfg[i].vir_addr.key + cfg->tbl_cfg[i].key_tbl_size;
       cfg->tbl_cfg[i].phy_addr.key = cfg->tbl_cfg[i].vir_addr.key;
       cfg->tbl_cfg[i].phy_addr.res = cfg->tbl_cfg[i].vir_addr.res;
#endif
    }
}
#endif

int drv_natc_init(natc_config_t *cfg)
{
    uint8_t tbl_idx;
    uint32_t key_addr_hi, key_addr_lo, res_addr_hi, res_addr_lo;
#ifndef RDP_SIM
    uint32_t cache_entry_idx;
    natc_indir_data indir_data = {};
#endif
    bdmf_error_t rc = BDMF_ERR_OK;

    g_natc_tbls_num = cfg->tbl_num;

    if (g_natc_tbls_num > RDD_NATC_TBL_CFG_SIZE)
        return BDMF_ERR_RANGE;
        
    for (tbl_idx = 0; tbl_idx < g_natc_tbls_num; tbl_idx++)
    {
        rc = rc ? rc : drv_natc_tbl_ctrl_set(tbl_idx, &cfg->tbl_cntrl[tbl_idx]);
        GET_ADDR_HIGH_LOW(key_addr_hi, key_addr_lo, cfg->tbl_cfg[tbl_idx].phy_addr.key);
        rc = rc ? rc : ag_drv_natc_cfg_key_addr_set(tbl_idx, key_addr_lo >> 3, key_addr_hi);
        GET_ADDR_HIGH_LOW(res_addr_hi, res_addr_lo, cfg->tbl_cfg[tbl_idx].phy_addr.res);
        rc = rc ? rc : ag_drv_natc_cfg_res_addr_set(tbl_idx, res_addr_lo >> 3, res_addr_hi);

        rdd_natc_tbl_cfg(tbl_idx, key_addr_hi, key_addr_lo, res_addr_hi, res_addr_lo);

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

    g_hash_mode = cfg->ctrl_status.ddr_hash_mode;
    return rc ? rc : ag_drv_natc_ctrl_status_set(&cfg->ctrl_status);
}

int drv_natc_set_key_mask(uint16_t mask)
{
    uint16_t  tbl_idx;
    bdmf_error_t rc = BDMF_ERR_OK;

    for (tbl_idx = 0; tbl_idx < g_natc_tbls_num; tbl_idx++)
    {
      g_natc_tbl_cfg[tbl_idx].mask = mask;
    
      rc = ag_drv_natc_key_mask_tbl_key_mask_set(tbl_idx, mask);
    }

   return rc;
}


/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/

#ifdef USE_BDMF_SHELL
#define TABLE_CTRL_KEY_LEN_GET(tbl_ctrl)       (NATC_TABLE_KEY_SIZE * (1 + ((tbl_ctrl & 0x2) >> 0x1)))
#define TABLE_CTRL_NON_CACHEABLE_GET(tbl_ctrl) (tbl_ctrl & 0x1)

#if defined(USE_NATC_VAR_CONTEXT_LEN)
static int _drv_natc_tbl_ctrl_cli_get(bdmf_session_handle session, uint8_t tbl_idx)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    natc_table_control table_control_reg;
    bdmf_boolean *table_control_field;

    rc = ag_drv_natc_table_control_get(&table_control_reg);
    if (!rc)
    {
        table_control_field = &table_control_reg.var_context_len_en_tbl0 - tbl_idx;
        bdmf_session_print(session, "variable context length: %d\n", table_control_field[0]);

        table_control_field = &table_control_reg.non_cacheable_tbl0 - (tbl_idx * 2) - 1;
        bdmf_session_print(session, "key length:              %d\n", table_control_field[0]);
        bdmf_session_print(session, "non cachable:            %d\n", table_control_field[1]);
    }
    return rc;
}
#else
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
#endif

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

int drv_natc_cli_get_counters(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t rc;
    uint64_t packets;
    uint64_t bytes;
    uint32_t index = (uint16_t)parm[1].value.unumber;

    rc = drv_natc_entry_counters_get((uint16_t)parm[0].value.unumber, index, &packets, &bytes);
    bdmf_trace("natc counter for index%d packet=%d bytes=%d\n", index, (int)packets, (int)bytes);
    return rc;
}

int drv_natc_cli_set_mask(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint16_t  mask;
    uint16_t  tbl_idx;
    bdmf_error_t rc = BDMF_ERR_OK;

    tbl_idx = (uint16_t)parm[0].value.unumber;
    mask = (uint16_t)parm[1].value.unumber;
    
    g_natc_tbl_cfg[tbl_idx].mask = mask;
    
    rc = ag_drv_natc_key_mask_tbl_key_mask_set(tbl_idx, mask);

   return rc;
}

static natc_ctrs_natc_ctrs g_natc_ctrs = {};

static void __drv_natc_get_counters_from_ddr(uint8_t tbl_idx, uint32_t *sw_miss_cache_cnt, uint32_t *sw_hit_cnt)
{
    uint8_t key_entry[NATC_MAX_ENTRY_LEN] = {};
    bdmf_boolean valid;
    int i, rc;
    int _sw_miss_cache_cnt = 0, _sw_hit_cnt = 0;

    *sw_miss_cache_cnt = 0;
    *sw_hit_cnt = 0;
    for (i = 0; i < g_natc_tbl_cfg[tbl_idx].tbl_entry_num; i++)
    {
        rc = drv_natc_key_entry_get(tbl_idx, i, &valid, key_entry);
        if (rc)
            continue;
#ifdef SW_CACHE_MISS_HANDLE
        if (!(i % NATC_LOOKUP_DEPTH_SIZE))
        {
            _sw_miss_cache_cnt++;
        }
        else
#endif
        {
            _sw_hit_cnt++;
        }
    }
    *sw_miss_cache_cnt = _sw_miss_cache_cnt;
    *sw_hit_cnt = _sw_hit_cnt;
}

int drv_natc_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	uint64_t prcnt;
    uint32_t miss_diff, hit_diff;
    uint64_t natc_total_access;
    uint8_t tbl_idx;
    natc_ctrs_natc_ctrs natc_ctrs = {};
    static uint32_t natc_ctrs_cli[] = {cli_natc_ctrs_natc_ctrs};

    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t natc_sw_cache_miss_cnt, natc_sw_hit_cnt;

    bdmf_session_print(session, "NATC debug stats:\n\r");
#if !defined(NATC_UNIFIED_COUNTERS)
    for (tbl_idx = 0; !rc && tbl_idx < g_natc_tbls_num; tbl_idx++)
#endif
    {
#if !defined(NATC_UNIFIED_COUNTERS)
        bdmf_session_print(session, "\nTable [%d]:\n", tbl_idx);
        HAL_CLI_IDX_PRINT_LIST(session, natc_ctrs, natc_ctrs_cli, tbl_idx);
        rc = ag_drv_natc_ctrs_natc_ctrs_get(tbl_idx, &natc_ctrs);
#else
        HAL_CLI_PRINT_LIST(session, natc_ctrs, natc_ctrs_cli);
        rc = ag_drv_natc_ctrs_natc_ctrs_get(&natc_ctrs);
#endif
        g_natc_ctrs.ddr_request_count = natc_ctrs.ddr_request_count - g_natc_ctrs.ddr_request_count;
        miss_diff = natc_ctrs.cache_miss_count - g_natc_ctrs.cache_miss_count;
        hit_diff = natc_ctrs.cache_hit_count - g_natc_ctrs.cache_hit_count;
        natc_total_access = hit_diff + miss_diff;


        memcpy((uint8_t *)&g_natc_ctrs, (uint8_t *)&natc_ctrs, sizeof(natc_ctrs_natc_ctrs));

        if (natc_total_access > 0)
        {
			prcnt = 100 * hit_diff;
			do_div(prcnt, natc_total_access);
			bdmf_session_print(session, "Hit ratio: %3u%%\n", (uint32_t)prcnt);
			prcnt = 100 * miss_diff;
			do_div(prcnt, natc_total_access);
			bdmf_session_print(session, "Miss ratio: %3u%%\n", (uint32_t)prcnt);
        }
#if defined(NATC_UNIFIED_COUNTERS)
        for (tbl_idx = 0; !rc && tbl_idx < g_natc_tbls_num; tbl_idx++)
        {
             bdmf_session_print(session, "\nTable [%d]:\n", tbl_idx);
#else
        {
#endif
            __drv_natc_get_counters_from_ddr(tbl_idx, &natc_sw_cache_miss_cnt, &natc_sw_hit_cnt);
            bdmf_session_print(session, "Number of elements cached for SW miss: %d\n", natc_sw_cache_miss_cnt);
            bdmf_session_print(session, "Number of elements in DDR: %d\n", natc_sw_hit_cnt);
            bdmf_session_print(session, "Number of elements is SW NATC Table: %d\n", g_natc_tbl_cfg[tbl_idx].count);
        }
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
    ag_drv_natc_key_mask_cli_init(natc_dir);

    BDMFMON_MAKE_CMD_NOPARM(natc_dir, "cfg_get", "natc configuration", (bdmfmon_cmd_cb_t)drv_natc_cli_config_get);
    BDMFMON_MAKE_CMD_NOPARM(natc_dir, "dbg_get", "natc configuration", (bdmfmon_cmd_cb_t)drv_natc_cli_debug_get);
    BDMFMON_MAKE_CMD_NOPARM(natc_dir, "cd", "print non-zero cache entries", (bdmfmon_cmd_cb_t)drv_natc_cli_cache_dump);
    BDMFMON_MAKE_CMD(natc_dir, "mask",   "set natc mask", (bdmfmon_cmd_cb_t)drv_natc_cli_set_mask,
        BDMFMON_MAKE_PARM_ENUM("table index", "table index", natc_enum_table, 0),
        BDMFMON_MAKE_PARM("key mask", "key mask", BDMFMON_PARM_NUMBER, 0));
    BDMFMON_MAKE_CMD(natc_dir, "read_counters",   "read nat cache counters", (bdmfmon_cmd_cb_t)drv_natc_cli_get_counters,
            BDMFMON_MAKE_PARM_ENUM("table index", "table index", natc_enum_table, 0),
            BDMFMON_MAKE_PARM("index", "counter index", BDMFMON_PARM_NUMBER, 0));


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

