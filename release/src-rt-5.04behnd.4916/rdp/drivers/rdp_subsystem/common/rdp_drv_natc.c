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

#include "data_path_init.h"

#include "rdp_subsystem_common.h"
#include "access_macros.h"
#include "rdd_common.h"
#include "rdp_drv_natc.h"
#include "rdd_natc.h"
#include "rdd_runner_proj_defs.h"
#include "rdd_data_structures_auto.h"
#if defined(BCM_PON_XRDP)
#include "rdp_drv_natc_counters.h"
#endif
#include "xrdp_autogens.h"

#ifdef SW_CACHE_MISS_HANDLE
#include "rdd_tuple_lkp.h"
#define DELETE_IDX(entry_idx) \
    ((entry_idx) + (NATC_LOOKUP_DEPTH_SIZE - ((entry_idx) % NATC_LOOKUP_DEPTH_SIZE)) % NATC_LOOKUP_DEPTH_SIZE) % (tbl_config->key_tbl_size / tbl_config->key_len)
#endif

#define POLLING_TIME_OUT  1000
#define NATC_NUM_OF_BITS_IN_HIT_COUNT   28
#define NATC_NUM_OF_BITS_IN_BYTE_COUNT  36

#if defined(RDP_SIM)
#define RDD_BTRACE_SIM bdmf_trace
#else
#define RDD_BTRACE_SIM RDD_BTRACE
#endif

static uint8_t g_hash_mode = hash_mode_crc32_low;
static uint8_t g_natc_hash_mode = hash_mode_crc32;
uint32_t total_length_bytes[] = {48, 64, 80, 96, 112, 128, 144, 160};
natc_tbl_config_t g_natc_tbl_cfg[NATC_MAX_TABLES_NUM] = {};
uint32_t g_natc_tbl_en_mask = 0;
#if (CHIP_VER >= RDP_GEN_100)
bdmf_error_t drv_natc_key_mask_set(uint8_t tbl_idx, uint8_t zero, const natc_key_mask *mask)
{
    bdmf_error_t (*mask_set_func)(uint32_t, uint32_t);
    int i;
    
    switch (tbl_idx)
    {
        case 0:
            mask_set_func = &ag_drv_nat_cache_natc_tbl0_key_mask_set;
        break;

        case 1:
            mask_set_func = &ag_drv_nat_cache_natc_tbl1_key_mask_set;
        break;

        case 2:
            mask_set_func = &ag_drv_nat_cache_natc_tbl2_key_mask_set;
        break;

        case 3:
            mask_set_func = &ag_drv_nat_cache_natc_tbl3_key_mask_set;
        break;
        
        case 4:
            mask_set_func = &ag_drv_nat_cache_natc_tbl4_key_mask_set;
        break;

        case 5:
            mask_set_func = &ag_drv_nat_cache_natc_tbl5_key_mask_set;
        break;

        case 6:
            mask_set_func = &ag_drv_nat_cache_natc_tbl6_key_mask_set;
        break;

        case 7:
            mask_set_func = &ag_drv_nat_cache_natc_tbl7_key_mask_set;
        break;
    }

    for (i = 0; i < 8; i++)
    {
        mask_set_func(i, mask->data[i]);
    }
    return BDMF_ERR_OK;
}

bdmf_error_t drv_natc_key_mask_get(uint8_t tbl_idx, uint8_t zero, natc_key_mask *mask)
{
    bdmf_error_t (*mask_get_func)(uint32_t, uint32_t*);
    int i;
    
    switch (tbl_idx)
    {
        case 0:
            mask_get_func = &ag_drv_nat_cache_natc_tbl0_key_mask_get;
        break;

        case 1:
            mask_get_func = &ag_drv_nat_cache_natc_tbl1_key_mask_get;
        break;

        case 2:
            mask_get_func = &ag_drv_nat_cache_natc_tbl2_key_mask_get;
        break;

        case 3:
            mask_get_func = &ag_drv_nat_cache_natc_tbl3_key_mask_get;
        break;
        
        case 4:
            mask_get_func = &ag_drv_nat_cache_natc_tbl4_key_mask_get;
        break;

        case 5:
            mask_get_func = &ag_drv_nat_cache_natc_tbl5_key_mask_get;
        break;

        case 6:
            mask_get_func = &ag_drv_nat_cache_natc_tbl6_key_mask_get;
        break;

        case 7:
            mask_get_func = &ag_drv_nat_cache_natc_tbl7_key_mask_get;
        break;
    }

    for (i = 0; i < 8; i++)
    {
        mask_get_func(i, &(mask->data[i]));
    }
    return BDMF_ERR_OK;
}

bdmf_error_t drv_natc_tbl_key_addr_set(uint8_t tbl_idx, uint32_t key_lo, uint8_t key_hi)
{
  switch (tbl_idx)
    {
        case 0:
            ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_lower_set(0, key_lo);
            ag_drv_nat_cache_natc_tbl0_ddr_key_base_address_upper_set(key_hi, 0);
        break;

        case 1:
            ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_lower_set(0, key_lo);
            ag_drv_nat_cache_natc_tbl1_ddr_key_base_address_upper_set(key_hi, 0);
        break;

        case 2:
            ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_lower_set(0, key_lo);
            ag_drv_nat_cache_natc_tbl2_ddr_key_base_address_upper_set(key_hi, 0);
        break;

        case 3:
            ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_lower_set(0, key_lo);
            ag_drv_nat_cache_natc_tbl3_ddr_key_base_address_upper_set(key_hi, 0);
        break;
        
        case 4:
            ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_lower_set(0, key_lo);
            ag_drv_nat_cache_natc_tbl4_ddr_key_base_address_upper_set(key_hi, 0);
        break;

        case 5:
            ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_lower_set(0, key_lo);
            ag_drv_nat_cache_natc_tbl5_ddr_key_base_address_upper_set(key_hi, 0);
        break;

        case 6:
            ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_lower_set(0, key_lo);
            ag_drv_nat_cache_natc_tbl6_ddr_key_base_address_upper_set(key_hi, 0);
        break;

        case 7:
            ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_lower_set(0, key_lo);
            ag_drv_nat_cache_natc_tbl7_ddr_key_base_address_upper_set(key_hi, 0);
        break;
    }  
    return 0;  
}

bdmf_error_t drv_natc_tbl_res_addr_set(uint8_t tbl_idx, uint32_t res_lo, uint8_t res_hi)
{
  switch (tbl_idx)
    {
        case 0:
            ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_lower_set(0, res_lo);
            ag_drv_nat_cache_natc_tbl0_ddr_result_base_address_upper_set(res_hi, 0);
        break;

        case 1:
            ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_lower_set(0, res_lo);
            ag_drv_nat_cache_natc_tbl1_ddr_result_base_address_upper_set(res_hi, 0);
        break;

        case 2:
            ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_lower_set(0, res_lo);
            ag_drv_nat_cache_natc_tbl2_ddr_result_base_address_upper_set(res_hi, 0);
        break;

        case 3:
            ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_lower_set(0, res_lo);
            ag_drv_nat_cache_natc_tbl3_ddr_result_base_address_upper_set(res_hi, 0);
        break;
        
        case 4:
            ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_lower_set(0, res_lo);
            ag_drv_nat_cache_natc_tbl4_ddr_result_base_address_upper_set(res_hi, 0);
        break;

        case 5:
            ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_lower_set(0, res_lo);
            ag_drv_nat_cache_natc_tbl5_ddr_result_base_address_upper_set(res_hi, 0);
        break;

        case 6:
            ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_lower_set(0, res_lo);
            ag_drv_nat_cache_natc_tbl6_ddr_result_base_address_upper_set(res_hi, 0);
        break;

        case 7:
            ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_lower_set(0, res_lo);
            ag_drv_nat_cache_natc_tbl7_ddr_result_base_address_upper_set(res_hi, 0);
        break;
    }  
    return 0;  
}


#endif

static hash_mode_t drv_natc_ddr_hash_mode(void)
{
    return g_hash_mode;
}

static hash_mode_t drv_natc_hash_mode(void)
{
    return g_natc_hash_mode;
}

#define NATC_KEY_ENTRY(tbl_config, entry_idx) \
    ((uint8_t *)(tbl_config->vir_addr.key + (tbl_config->key_len * entry_idx)))
#define NATC_RES_ENTRY(tbl_config, entry_idx) \
    ((uint8_t *)(tbl_config->vir_addr.res + (tbl_config->res_len * entry_idx)))
static inline int drv_natc_tbl_config_get(uint8_t tbl_idx, natc_tbl_config_t **tbl_config)
{
    if (tbl_idx >= RDD_NATC_TBL_CFG_SIZE)
        return BDMF_ERR_RANGE;

    *tbl_config = &g_natc_tbl_cfg[tbl_idx];

    return 0;
}

struct bdmfmon_enum_val natc_enum_table[] = {
    {"NATC_DS_TBL", 0},
    {"NATC_US_TBL", 1},
    {"NATC_COUNTERS_TBL", 2},
    {"NATC_VLAN_ACTION_TBL", 0},
    {NULL, 0},
};

struct bdmfmon_enum_val natc_hash_enum_table[] = {
    {"rolling_xor", hash_mode_rolling_xor},
    {"crc32", hash_mode_crc32},
    {"crc32_low", hash_mode_crc32_low},
    {"crc32_high", hash_mode_crc32_high},
#if CHIP_VER >= RDP_GEN_62    
    {"rss_key_0", hash_mode_rss_secret_key_0},
    {"rss_key_1", hash_mode_rss_secret_key_1},
    {"rss_key_2", hash_mode_rss_secret_key_2},
    {"rss_key_3", hash_mode_rss_secret_key_3},
    {"key_N_4", hash_mode_key_N_4},
#endif
    {NULL, 0},
};

bdmf_fastlock nat_cache_lock;

#if !defined(RDP_SIM)
static int drv_natc_eng_key_result_write(uint32_t eng_idx, natc_eng_key_result * key_result)
{
    uint32_t i;
    natc_eng_key_result key_result_swapped = {};
    int rc;

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

#if (CHIP_VER < RDP_GEN_100)
    rc = ag_drv_natc_eng_key_result_set(eng_idx, 0, key_result);
#else
    rc = ag_drv_nat_cache_nat0_key_result_set(0, key_result);
#endif
    return rc;
}

static int drv_natc_eng_command_write(uint32_t eng_idx, uint8_t tbl_idx, nat_command_t command, uint8_t cache_flush)
{
    int time_out = POLLING_TIME_OUT;
    natc_eng_command_status command_status = {};

    memset(&command_status, 0 , sizeof(natc_eng_command_status));
    
    command_status.command = command;
    command_status.nat_tbl = tbl_idx;
#if defined(NATC_COUNTER_BUG_WA)
    command_status.cache_flush = cache_flush;
#endif

#if (CHIP_VER < RDP_GEN_100)
    ag_drv_natc_eng_command_status_set(eng_idx, &command_status);
#else
    ag_drv_natc_eng_command_status_set(&command_status);
#endif

    while (time_out > 0)
    {
#if (CHIP_VER < RDP_GEN_100)
        ag_drv_natc_eng_command_status_get(eng_idx, &command_status);
#else
        ag_drv_natc_eng_command_status_get(&command_status);
#endif
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

#define MAX_ITERATION_CNT 100
static int drv_natc_eng_command_submit(nat_command_t command, uint8_t tbl_idx, uint32_t entry_idx, natc_eng_key_result *keyword,
    uint64_t *hit_count, uint64_t *byte_count)
{
#if !defined(RDP_SIM)
    int rc;
#if CHIP_VER < RDP_GEN_50
    uint8_t eng_idx = command;	 
#else
    uint8_t eng_idx = 0;
#endif

#if defined(NATC_COUNTER_BUG_WA)
    int iteration_cnt;
    uint32_t hash_idx;  
    uint8_t *tbl_res;
    natc_tbl_config_t *tbl_config = NULL;
    uint32_t  old_hit_count_hw, old_byte_count_hw;
#endif
    uint32_t  hit_count_hw, byte_count_hw;

    if (command == natc_cmd_add)
        return BDMF_ERR_NOT_SUPPORTED;

    rc = drv_natc_eng_key_result_write(eng_idx, keyword);
    rc = rc ? rc : drv_natc_eng_command_write(eng_idx, tbl_idx, command, 0);
    if (rc)
        goto exit;

    if (command == natc_cmd_lookup)
    {
#if defined(NATC_COUNTER_BUG_WA)
        /* flush counters to DDR and read from it */        
        rc = ag_drv_natc_eng_hash_get(eng_idx, &hash_idx);        

        rc = rc ? rc : drv_natc_tbl_config_get(tbl_idx, &tbl_config);
        if (rc)
            return rc;

        /* set key result to hash index for flush command */
        if (tbl_config->key_len == NATC_TABLE_KEY_16B)        
            keyword->data[3] = (hash_idx & 0x3ff) << 22;
        else
            keyword->data[7] = (hash_idx & 0x3ff) << 22;

        tbl_res = NATC_RES_ENTRY(tbl_config, entry_idx);
        RDD_NATC_COUNTERS_ENTRY_HIT_COUNTER_READ(old_hit_count_hw, tbl_res);
        RDD_NATC_COUNTERS_ENTRY_BYTES_COUNTER_READ(old_byte_count_hw, tbl_res);


#if (CHIP_VER < RDP_GEN_100)
        rc = rc ? rc : ag_drv_natc_eng_key_result_set(eng_idx, 0, keyword);
#else
        rc = rc ? rc : ag_drv_nat_cache_nat0_key_result_set(0, keyword);
#endif

        rc = rc ? rc : drv_natc_eng_command_write(eng_idx, tbl_idx, command, 1);

        iteration_cnt = MAX_ITERATION_CNT;
        if (!rc)
        {
            while(iteration_cnt > 0)
            {
                iteration_cnt--;
                RDD_NATC_COUNTERS_ENTRY_HIT_COUNTER_READ(hit_count_hw, tbl_res);
                RDD_NATC_COUNTERS_ENTRY_BYTES_COUNTER_READ(byte_count_hw, tbl_res);
                if (hit_count_hw != old_hit_count_hw)
                    break;
            };
        
            if (hit_count)
                *hit_count = (uint64_t) hit_count_hw & ((1UL<< NATC_NUM_OF_BITS_IN_HIT_COUNT) - 1);

            if (byte_count)
            {
                *byte_count = (uint64_t) byte_count_hw << (NATC_NUM_OF_BITS_IN_BYTE_COUNT - 32);
                *byte_count |= ((uint64_t) hit_count_hw >> NATC_NUM_OF_BITS_IN_HIT_COUNT);
            }
        }
#else
        if (hit_count)
#if (CHIP_VER < RDP_GEN_100) 
            rc = ag_drv_natc_eng_hit_count_get(eng_idx, &hit_count_hw);
#else
            rc = ag_drv_natc_eng_hit_count_get(&hit_count_hw);
#endif

        if (byte_count)
        {
#if (CHIP_VER < RDP_GEN_100) 
            rc = rc ? rc : ag_drv_natc_eng_byte_count_get(eng_idx, &byte_count_hw);
#else
            rc = rc ? rc : ag_drv_nat_cache_nat0_byte_count_get(&byte_count_hw);
#endif

            if (hit_count)
            {
                *hit_count = hit_count_hw & ((1UL << NATC_NUM_OF_BITS_IN_HIT_COUNT) - 1);
                *byte_count = (hit_count_hw >> NATC_NUM_OF_BITS_IN_HIT_COUNT);
                *byte_count &= ((1UL << (32 - NATC_NUM_OF_BITS_IN_HIT_COUNT)) - 1);
                *byte_count |= ((uint64_t)byte_count_hw << (NATC_NUM_OF_BITS_IN_BYTE_COUNT - 32));
            }
        }
#endif
    }

exit:
    return rc;
#else
    return BDMF_ERR_OK;
#endif
}


#if CHIP_VER < RDP_GEN_60
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
#else
static inline int drv_natc_key_entry_compare(uint32_t tbl_idx, uint8_t *key_0, uint8_t *key_1, natc_key_mask mask)
{
    natc_tbl_config_t *tbl_config = &g_natc_tbl_cfg[tbl_idx];
    int i;
    uint32_t *mask32 = mask.data;
    uint32_t *key0_32 = (uint32_t *)key_0;
    uint32_t *key1_32 = (uint32_t *)key_1;
    int len = (tbl_config->key_len / sizeof(uint32_t)) - 1;

    for (i = 0; i <= len; i++)
    {
        if ((key0_32[i] & ~swap4bytes(mask32[len - i])) != (key1_32[i] & ~swap4bytes(mask32[len - i])))
            return BDMF_ERR_OK;
    }

    return BDMF_ERR_PARM;
}
#endif

static void drv_natc_rolling_xor(uint32_t key_len, uint8_t *keyword, uint32_t *hash_idx)
{
    uint8_t i;
    uint32_t *key = (uint32_t *)keyword;

    *hash_idx = swap4bytes(HASH_FUNCTION_INIT_VAL);
    for (i = 0; i < (key_len / sizeof(uint32_t)); i++, key++)
        *hash_idx ^= swap4bytes(*key);
}

#if CHIP_VER < RDP_GEN_60
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
#else
static inline void drv_natc_key_mask(uint8_t key_len, natc_key_mask mask, uint8_t *masked_key, uint8_t *key)
{
    int i;
    uint32_t *mask32 = mask.data;
    uint32_t *masked_key32 = (uint32_t *)masked_key;
    uint32_t *key32 = (uint32_t *)key;
    int len = (key_len / sizeof(uint32_t)) - 1;

    for (i = 0; i <= len; i++)
    {
        masked_key32[i] = key32[i] & ~swap4bytes(mask32[len - i]);
    }
}
#endif

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

#if CHIP_VER < RDP_GEN_60
static inline int drv_natc_hash_function(uint8_t tbl_idx, uint32_t mask, uint8_t *keyword, uint32_t *hash_idx)
#else
static inline int drv_natc_hash_function(uint8_t tbl_idx, natc_key_mask mask, uint8_t *keyword, uint32_t *hash_idx)
#endif
{
    natc_tbl_config_t *tbl_config = &g_natc_tbl_cfg[tbl_idx];
    uint8_t masked_key[NATC_MAX_ENTRY_LEN] = {};
    natc_ddr_cfg_natc_ddr_size ddr_size;
    uint8_t natc_ddr_size;
    uint8_t hash_shifts;
    uint32_t hash_mask;

    drv_natc_key_mask(tbl_config->key_len, mask, masked_key, keyword);
    ag_drv_natc_ddr_cfg_natc_ddr_size_get(&ddr_size); 
    natc_ddr_size = (uint8_t)_natc_tbl_ddr_size_enum(tbl_idx, &ddr_size);

    if (g_hash_mode == hash_mode_rolling_xor)
    {
        drv_natc_rolling_xor(tbl_config->key_len, masked_key, hash_idx);
        drv_natc_hash_reduce(hash_idx, natc_ddr_size);
    }

#if CHIP_VER >= RDP_GEN_62    
    /* Key[N:4] is used as hash value. If DDR table size is 8K, N=16, 16K - N=17 ... 256K - N=21 */
    else if (g_hash_mode == hash_mode_key_N_4)
    {
        NAT_CACHE_COUNTER_LKP_ENTRY_STRUCT key_entry_swapped;
        memcpy((uint8_t *)&key_entry_swapped,keyword,sizeof(NAT_CACHE_COUNTER_LKP_ENTRY_STRUCT));

#ifdef FIRMWARE_LITTLE_ENDIAN
        SWAPBYTES(((uint8_t *)&key_entry_swapped), sizeof(NAT_CACHE_COUNTER_LKP_ENTRY_STRUCT));
#endif
        *hash_idx = key_entry_swapped.counter_key; 
    }
#endif
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

static inline bdmf_boolean drv_natc_valid_bit_get(uint8_t *key_entry, uint8_t tbl_idx)
{
    uint8_t *lkp_key_ptr;
    NAT_CACHE_L3_LKP_ENTRY_STRUCT lkp_key;

    lkp_key_ptr = (uint8_t *)&lkp_key;

    MREAD_BLK_8(lkp_key_ptr, key_entry, sizeof(NAT_CACHE_L3_LKP_ENTRY_STRUCT));
#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(lkp_key_ptr, sizeof(NAT_CACHE_L3_LKP_ENTRY_STRUCT));
#endif

#if defined(BCM6855)
    /* following strange check is because valid is swapped in gen6 in l3 key - HW bug */
    return (lkp_key.valid || lkp_key.protocol);
#else
    return lkp_key.valid;
#endif
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

    *valid = drv_natc_valid_bit_get(key_entry, tbl_idx);
    if (*valid)
        memcpy(key, key_entry, tbl_config->key_len);
    else
        rc = BDMF_ERR_NOENT;

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
    bdmf_error_t rc = BDMF_ERR_OK, rc1;

    if (tbl_idx >= RDD_NATC_TBL_CFG_SIZE)
        return BDMF_ERR_RANGE;

    if ((g_natc_tbl_en_mask & (0x1 << tbl_idx)) == 0)
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
            bdmf_trace("ERROR: drv_natc_find_empty_hash_key_entry: entry_index %d with depth %d out of range %d\n",*hash_idx, depth, key_bin_total_entry_num);
            BUG();
            return BDMF_ERR_ALREADY;
        }

        rc1 = drv_natc_key_entry_get(tbl_idx, (*hash_idx + depth), &valid, key_entry);
        if (rc1 || !valid)
        {
            if (empty_hash_index < 0)
                empty_hash_index = (*hash_idx + depth);
            continue;
        }
#if defined(USE_NATC_VAR_CONTEXT_LEN)
        /* Zero key ctx_var when comparing to hash_key */
        key_entry[NAT_CACHE_LKP_ENTRY_VAR_SIZE_CTX_VAR_SIZE_CTX_OFFSET] &= 0xF0;
#endif
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
    bdmf_error_t rc = BDMF_ERR_OK, rc1;
    bdmf_boolean valid = 0;
    uint8_t key_entry[NATC_MAX_ENTRY_LEN] = {};

    rc1 = drv_natc_key_entry_get(tbl_idx, delete_idx, &valid, key_entry);

    if (!rc1 && valid && drv_natc_key_entry_compare(tbl_idx, key, key_entry, tbl_config->mask))
    {
        /* delete entry even for case add entry not succeed to prevent fw<->sw natc races */
        rc = drv_natc_entry_delete(tbl_idx, delete_idx, 0);
    }
    /* delete entry from cache anyway - possible that it is not in DDR and in cache only */
    if (!rc)
    {
        MREAD_BLK_8(del_key_result, key, tbl_config->key_len);
        /* look for the entry in the NAT cache internal memory and try to delete it if its there */
        drv_natc_eng_command_submit(natc_cmd_del, tbl_idx, entry_idx, del_key_result, NULL, NULL);
    }

    return rc;
}
#endif

int drv_natc_key_idx_get(uint8_t tbl_idx, uint8_t *keyword, uint32_t *hash_idx, uint32_t *entry_idx)
{
    int rc;

    if (tbl_idx >= RDD_NATC_TBL_CFG_SIZE)
        return BDMF_ERR_RANGE;

    if ((g_natc_tbl_en_mask & (0x1 << tbl_idx)) == 0)
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
    rc = rc ? rc : drv_natc_eng_command_submit(natc_cmd_lookup, tbl_idx, entry_idx, &key_result, hit_count, byte_count);

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

    bdmf_fastlock_lock(&nat_cache_lock);
    tbl_key = NATC_KEY_ENTRY(tbl_config, entry_idx);
    tbl_res = NATC_RES_ENTRY(tbl_config, entry_idx);
    tbl_len = tbl_config->res_len;

    /* move pointers ahead by 8 bytes to skip rewriting stats to 0 */
    result  += sizeof(NATC_COUNTERS_ENTRY_STRUCT);
    tbl_res += sizeof(NATC_COUNTERS_ENTRY_STRUCT);
    tbl_len -= sizeof(NATC_COUNTERS_ENTRY_STRUCT);

    MWRITE_BLK_8(tbl_res, result, tbl_len);

    /* if valid, flush flow from natc */
    if (valid)
    {
#if defined(NATC_COUNTER_BUG_WA)
#if !defined(RDP_SIM)
#if CHIP_VER < RDP_GEN_50
        uint8_t eng_idx = natc_cmd_lookup;	 
#else
        uint8_t eng_idx = 0;
#endif
        /* flush stats from natc to ddr entry */
        drv_natc_eng_command_write(eng_idx, tbl_idx, natc_cmd_lookup, 1);
#endif
#else
        {
#if CHIP_VER < RDP_GEN_50
            uint8_t eng_idx = natc_cmd_lookup;	 
#else
#if ((CHIP_VER < RDP_GEN_100) || (!defined(RDP_SIM)))
            uint8_t eng_idx = 0;
#endif
#endif

            uint32_t hit_count;
            uint32_t byte_count;
 
            /* Read hit and byte counters from natc */
#if !defined(RDP_SIM)
            rc = drv_natc_eng_key_result_write(eng_idx, &key_result);
            rc = rc ? rc : drv_natc_eng_command_write(eng_idx, tbl_idx, natc_cmd_lookup, 0);
#endif
#if (CHIP_VER < RDP_GEN_100)
            rc = ag_drv_natc_eng_hit_count_get(eng_idx, &hit_count);
            rc = rc ? rc : ag_drv_natc_eng_byte_count_get(eng_idx, &byte_count);
#else
            rc = ag_drv_natc_eng_hit_count_get(&hit_count);
            rc = rc ? rc : ag_drv_nat_cache_nat0_byte_count_get(&byte_count);
#endif

            /* reset table address to the context begining */
            tbl_res -= sizeof(NATC_COUNTERS_ENTRY_STRUCT);

            SWAPBYTES(((uint8_t *)&hit_count), sizeof(hit_count));
            SWAPBYTES(((uint8_t *)&byte_count), sizeof(byte_count));

            /* store counters directly in modified entry in DDR */
            MWRITE_BLK_8(tbl_res, &hit_count, sizeof(hit_count));
            MWRITE_BLK_8(tbl_res + sizeof(hit_count), &byte_count, sizeof(byte_count));
        }

#endif
        /* delete entry from natc */
        MREAD_BLK_8(&key_result, tbl_key, tbl_config->key_len);
        rc = drv_natc_eng_command_submit(natc_cmd_del, tbl_idx, entry_idx, &key_result, NULL, NULL);
        rc = (rc == BDMF_ERR_INTERNAL) ? rc : BDMF_ERR_OK;
    }

    bdmf_fastlock_unlock(&nat_cache_lock);
    return rc;
}

int drv_natc_key_entry_add(uint8_t tbl_idx, uint32_t entry_idx, uint8_t *key)
{
    natc_tbl_config_t *tbl_config;
    bdmf_error_t rc = BDMF_ERR_OK;

    rc = drv_natc_tbl_config_get(tbl_idx, &tbl_config);
    if (rc)
        return rc;

    RDD_BTRACE("key Address 0x%p\n", NATC_KEY_ENTRY(tbl_config, entry_idx));
    RDD_BTRACE_SIM("key s 0x%08X%08X%08X%08X\n", ((uint32_t *)key)[0], ((uint32_t *)key)[1], ((uint32_t *)key)[2], ((uint32_t *)key)[3]);
    RDD_BTRACE("tbl_config->key_len(%d) * entry_idx(%d) = 0%0X\n", tbl_config->key_len, entry_idx, tbl_config->key_len * entry_idx);
#ifdef FIRMWARE_LITTLE_ENDIAN
    {
        uint32_t key_swapped[4];
         
        memcpy(key_swapped, key, sizeof(key_swapped));        
        SWAPBYTES(key_swapped, sizeof( key_swapped));
        RDD_BTRACE_SIM("key swapped 0x%08X%08X%08X%08X\n", key_swapped[0], key_swapped[1], key_swapped[2], key_swapped[3]);
    }
#endif
    MWRITE_BLK_8(NATC_KEY_ENTRY(tbl_config, entry_idx), key, tbl_config->key_len);

    return rc;
}

int drv_natc_key_result_entry_var_size_ctx_add(uint8_t tbl_idx, uint8_t *hash_key, uint8_t *key, uint8_t *result, uint32_t *entry_idx)
{
    bdmf_error_t rc = BDMF_ERR_OK;
#ifdef SW_CACHE_MISS_HANDLE
    bdmf_error_t rc1 = BDMF_ERR_OK;
    NAT_CACHE_L3_LKP_ENTRY_STRUCT lkp_key = {};
    uint8_t *lkp_key_ptr = (uint8_t *)&lkp_key; 
#endif
    natc_eng_key_result del_key_result = {};
    natc_tbl_config_t *tbl_config;
    uint32_t hash_idx;

    rc = drv_natc_tbl_config_get(tbl_idx, &tbl_config);

    if (rc)
        return rc;

    bdmf_fastlock_lock(&nat_cache_lock);
#if (CHIP_VER >= RDP_GEN_60) && !defined(CONFIG_RUNNER_FPI)
    {
        natc_key_mask key_mask;
        uint32_t key_word_idx, mask;
        uint32_t len = 3;
        
        rc = drv_natc_key_mask_get(tbl_idx, 0, &key_mask);
        for (key_word_idx = 0; key_word_idx < len + 1; key_word_idx++)
        {
            /* should swap the key and perform mask, but we swap the mask here for
             * easier computation */
            mask = key_mask.data[len - key_word_idx];
            mask = swap4bytes(mask);
            ((uint32_t *)hash_key)[key_word_idx] &= ~mask;
        }
    }
#endif
    /* hash_key does not include the variable size context field */
    rc = drv_natc_key_idx_get(tbl_idx, hash_key, &hash_idx, entry_idx);
    rc = rc ? rc : drv_natc_result_entry_add(tbl_idx, *entry_idx, result);
    rc = rc ? rc : drv_natc_key_entry_add(tbl_idx, *entry_idx, key);

    if (rc == 0)
    {
        g_natc_tbl_cfg[tbl_idx].count++;
    }

#ifdef SW_CACHE_MISS_HANDLE
    /* Check if there is no entry for flow miss and delete it if there is one */
    MREAD_BLK_8(lkp_key_ptr, key, sizeof(NAT_CACHE_L3_LKP_ENTRY_STRUCT));
#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(lkp_key_ptr, sizeof(NAT_CACHE_L3_LKP_ENTRY_STRUCT));
#endif
    bdmf_fastlock_unlock(&nat_cache_lock);
    rc1 = _drv_natc_entry_delete(tbl_idx, DELETE_IDX(hash_idx), tbl_config, key, *entry_idx, &del_key_result);
    return rc1 ? rc1 : rc;
#else
    /* remove HW natc cache miss entry */
    MREAD_BLK_8(&del_key_result, key, tbl_config->key_len);
    /* look for the entry in the NAT cache internal memory and try to delete it if its there */
    drv_natc_eng_command_submit(natc_cmd_del, tbl_idx, *entry_idx, &del_key_result, NULL, NULL);
    bdmf_fastlock_unlock(&nat_cache_lock);
    return rc;
#endif
}

int drv_natc_key_result_entry_add(uint8_t tbl_idx, uint8_t *key, uint8_t *result, uint32_t *entry_idx)
{
    uint8_t hash_key[NATC_MAX_ENTRY_LEN];
    /* TODO: This memcpy and variable allocation could be avoid if these two variable
       don't need different memory location in drv_natc_key_result_entry_var_size_ctx_add()
       for platforms without variable context len support */
    memcpy(hash_key, key, sizeof(hash_key));
    return drv_natc_key_result_entry_var_size_ctx_add(tbl_idx, hash_key, key, result, entry_idx);
}

int drv_natc_entry_delete(uint8_t tbl_idx, uint32_t entry_idx,  uint8_t count_delete)
{
    natc_eng_key_result del_key_result = {};
    natc_tbl_config_t *tbl_config;
    uint8_t *key;
    bdmf_error_t rc = BDMF_ERR_OK;

    rc = drv_natc_tbl_config_get(tbl_idx, &tbl_config);
    if (rc)
        return rc;

    key = NATC_KEY_ENTRY(tbl_config, entry_idx);

    bdmf_fastlock_lock(&nat_cache_lock);

    if (!drv_natc_valid_bit_get(key, tbl_idx))
    {
        bdmf_trace("Delete failed - Invalid DDR entry\n");
        bdmf_fastlock_unlock(&nat_cache_lock);
        return BDMF_ERR_PARM;
    }

    MREAD_BLK_8(&del_key_result, key, tbl_config->key_len);

    MEMSET(key, 0, tbl_config->key_len);

    /* look for the entry in the NAT cache internal memory if its there delete it */
    rc = drv_natc_eng_command_submit(natc_cmd_del, tbl_idx, entry_idx, &del_key_result, NULL, NULL);

    /* Do not zero the result in DDR.  This prevents a race condition where NAT Cache could
     * download a zero result for a flow hit on a firmware natc instruction. */

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

    if (tbl_idx >= RDD_NATC_TBL_CFG_SIZE)
        return BDMF_ERR_RANGE;

    rc = ag_drv_natc_table_control_get(&table_control_reg);
    if (!rc)
    {


        switch (tbl_idx)
        {
            case 0:
                table_control_reg.var_context_len_en_tbl0 = tbl_ctrl->var_context_len_en;
                table_control_reg.key_len_tbl0 = tbl_ctrl->key_len;
                table_control_reg.non_cacheable_tbl0 = tbl_ctrl->non_cacheable;
            break;

            case 1:
                table_control_reg.var_context_len_en_tbl1 = tbl_ctrl->var_context_len_en;
                table_control_reg.key_len_tbl1 = tbl_ctrl->key_len;
                table_control_reg.non_cacheable_tbl1 = tbl_ctrl->non_cacheable;
            break;

            case 2:
                table_control_reg.var_context_len_en_tbl2 = tbl_ctrl->var_context_len_en;
                table_control_reg.key_len_tbl2 = tbl_ctrl->key_len;
                table_control_reg.non_cacheable_tbl2 = tbl_ctrl->non_cacheable;
            break;

            case 3:
                table_control_reg.var_context_len_en_tbl3 = tbl_ctrl->var_context_len_en;
                table_control_reg.key_len_tbl3 = tbl_ctrl->key_len;
                table_control_reg.non_cacheable_tbl3 = tbl_ctrl->non_cacheable;
            break;

            case 4:
                table_control_reg.var_context_len_en_tbl4 = tbl_ctrl->var_context_len_en;
                table_control_reg.key_len_tbl4 = tbl_ctrl->key_len;
                table_control_reg.non_cacheable_tbl4 = tbl_ctrl->non_cacheable;
            break;

            case 5:
                table_control_reg.var_context_len_en_tbl5 = tbl_ctrl->var_context_len_en;
                table_control_reg.key_len_tbl5 = tbl_ctrl->key_len;
                table_control_reg.non_cacheable_tbl5 = tbl_ctrl->non_cacheable;
            break;

            case 6:
                table_control_reg.var_context_len_en_tbl6 = tbl_ctrl->var_context_len_en;
                table_control_reg.key_len_tbl6 = tbl_ctrl->key_len;
                table_control_reg.non_cacheable_tbl6 = tbl_ctrl->non_cacheable;
            break;

            case 7:
                table_control_reg.var_context_len_en_tbl7 = tbl_ctrl->var_context_len_en;
                table_control_reg.key_len_tbl7 = tbl_ctrl->key_len;
                table_control_reg.non_cacheable_tbl7 = tbl_ctrl->non_cacheable;
            break;
        }

        rc = ag_drv_natc_table_control_set(&table_control_reg);
    }

    return rc;
}

int drv_natc_tbl_ctrl_get(uint8_t tbl_idx, tbl_control_t *tbl_ctrl)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    natc_table_control table_control_reg;
    bdmf_boolean *table_control_field;

    if (tbl_idx >= RDD_NATC_TBL_CFG_SIZE)
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
    if (tbl_idx >= RDD_NATC_TBL_CFG_SIZE)
        return BDMF_ERR_RANGE;

    return ag_drv_natc_table_control_set(tbl_idx, (tbl_ctrl->key_len << 1) | tbl_ctrl->non_cacheable);
}

int drv_natc_tbl_ctrl_get(uint8_t tbl_idx, tbl_control_t *tbl_ctrl)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t table_control = 0;

    if (tbl_idx >= RDD_NATC_TBL_CFG_SIZE)
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

#if CHIP_VER < RDP_GEN_60
static rdpa_natc_table_name_t _natc_tbl_idx_to_name(uint8_t tbl_idx)
{
    switch (tbl_idx)
    {
    case NATC_TBL_IDX_DS:
        return rdpa_natc_table_name_ds;
    case NATC_TBL_IDX_US:
        return rdpa_natc_table_name_us;
    case NATC_TBL_IDX_MCAST:
        return rdpa_natc_table_name_mcast;
#if defined(BCM63158)
    case NATC_TBL_IDX_MCAST_WHITELIST:
        return rdpa_natc_table_name_mcast_whitelist;
#endif
    default:
        BDMF_TRACE_ERR("%s() : Invalid NATC table %d\n", __func__, tbl_idx);
        break;
    }
    return rdpa_natc_table_name_invalid;
}
#else /* CHIP_VER >= RDP_GEN_60) */
#if defined(CONFIG_RUNNER_FPI)
static rdpa_natc_table_name_t _natc_tbl_idx_to_name(uint8_t tbl_idx)
{
    switch (tbl_idx)
    {
    case FPI_NATC_TBL_IDX_L3L4:
        return rdpa_natc_table_name_l3_ucast;
    case FPI_NATC_TBL_IDX_L2:
        return rdpa_natc_table_name_l2_ucast;
    default:
        BDMF_TRACE_ERR("%s() : Invalid NATC table %d\n", __func__, tbl_idx);
        break;
    }
    return rdpa_natc_table_name_invalid;
}
#else
static rdpa_natc_table_name_t _natc_tbl_idx_to_name(uint8_t tbl_idx)
{
    switch (tbl_idx)
    {
    case NATC_TBL_IDX_L3:
        return rdpa_natc_table_name_l3_ucast;
    case NATC_TBL_IDX_L2:
        return rdpa_natc_table_name_l2_ucast;
    case NATC_TBL_IDX_MCAST:
        return rdpa_natc_table_name_mcast;
    case NATC_TBL_IDX_CT:
        return rdpa_natc_table_name_ct;
    default:
        BDMF_TRACE_ERR("%s() : Invalid NATC table %d\n", __func__, tbl_idx);
        break;
    }
    return rdpa_natc_table_name_invalid;
}
#endif
#endif

#if CHIP_VER < RDP_GEN_60 && !defined(BCM63158)
void drv_natc_cfg_build_tables(natc_config_t *cfg, natc_ddr_cfg_natc_ddr_size *ddr_size,
    natc_ddr_cfg_total_len *total_len, int table_base_size,
    void *table_base_addr_virt, bdmf_phys_addr_t table_base_addr_phys)
{
    int i;
    uint8_t* p_ddr_size = (uint8_t*)ddr_size;
    uint8_t* p_total_len = (uint8_t*)total_len;

    for (i = 0; i< natc_tbls_num; i++)
    {
        /* DS table */
        cfg->tbl_cfg[i].count = 0;
        cfg->tbl_cfg[i].table_name = _natc_tbl_idx_to_name(i);
        cfg->tbl_cntrl[i].key_len = key_len_16B;

        cfg->tbl_cfg[i].mask = (i != NATC_TBL_IDX_MCAST ) ? NATC_16BYTE_KEY_UCAST_MASK : NATC_16BYTE_KEY_MASK_FOR_MCAST;
        cfg->tbl_cfg[i].key_len = NATC_TABLE_KEY_16B + (NATC_TABLE_KEY_16B * cfg->tbl_cntrl[i].key_len);
        cfg->tbl_cfg[i].key_tbl_size = ((((table_base_size << p_ddr_size[i]) + ddr_8_bins_per_bucket) * cfg->tbl_cfg[i].key_len) + 0x1f) & ~0x1f;
        cfg->tbl_cfg[i].res_len = total_length_bytes[p_total_len[i]] - cfg->tbl_cfg[i].key_len;
        cfg->tbl_cfg[i].res_tbl_size = ((((table_base_size << p_ddr_size[i]) + ddr_8_bins_per_bucket) * cfg->tbl_cfg[i].res_len) + 0x1f) & ~0x1f;
        cfg->tbl_cfg[i].tbl_entry_num = table_base_size << p_ddr_size[i];
        if (i == 0)
            cfg->tbl_cfg[i].vir_addr.key = table_base_addr_virt;
        else
            cfg->tbl_cfg[i].vir_addr.key = (void *)((uint8_t *)cfg->tbl_cfg[i - 1].vir_addr.res + cfg->tbl_cfg[i - 1].res_tbl_size);
        cfg->tbl_cfg[i].vir_addr.res = (void *)((uint8_t *)cfg->tbl_cfg[i].vir_addr.key + cfg->tbl_cfg[i].key_tbl_size);
        cfg->tbl_cfg[i].phy_addr.key = RDD_RSV_VIRT_TO_PHYS(table_base_addr_virt, table_base_addr_phys, cfg->tbl_cfg[i].vir_addr.key);
        cfg->tbl_cfg[i].phy_addr.res = RDD_RSV_VIRT_TO_PHYS(table_base_addr_virt, table_base_addr_phys, cfg->tbl_cfg[i].vir_addr.res);
    }
}

#else /* 63158 or 6GEN */

/* Function to build/fill the natc table configuation parameters */
void drv_natc_init_build_table_cfg(natc_config_t *cfg, rdd_natc_tbl_idx tbl_idx, natc_table_cfg_t *tbl_cfg, dpi_params_t *p_dpi_cfg)
{
    cfg->tbl_cntrl[tbl_idx].key_len = tbl_cfg->key_len;
    cfg->tbl_cntrl[tbl_idx].var_context_len_en = tbl_cfg->var_cntx;
    cfg->tbl_cntrl[tbl_idx].non_cacheable = tbl_cfg->non_cacheable;

    cfg->tbl_cfg[tbl_idx].table_name = _natc_tbl_idx_to_name(tbl_idx);
    cfg->tbl_cfg[tbl_idx].key_len = NATC_TABLE_KEY_16B + (NATC_TABLE_KEY_16B * tbl_cfg->key_len);
    cfg->tbl_cfg[tbl_idx].key_tbl_size = ((((NATC_TABLE_BASE_SIZE_SIZE << tbl_cfg->tbl_size) + tbl_cfg->bins) * cfg->tbl_cfg[tbl_idx].key_len) + 0x1f) & ~0x1f;
    cfg->tbl_cfg[tbl_idx].res_len = _natc_tbl_entry_size_bytes(tbl_cfg->total_entry_len) - cfg->tbl_cfg[tbl_idx].key_len;
    cfg->tbl_cfg[tbl_idx].res_tbl_size = ((((NATC_TABLE_BASE_SIZE_SIZE << tbl_cfg->tbl_size) + tbl_cfg->bins) * cfg->tbl_cfg[tbl_idx].res_len) + 0x1f) & ~0x1f;
    cfg->tbl_cfg[tbl_idx].tbl_entry_num = NATC_TABLE_BASE_SIZE_SIZE << tbl_cfg->tbl_size;

#if defined(BCM63158)
    cfg->tbl_cfg[tbl_idx].mask = tbl_cfg->mask;
#else
    memcpy(&cfg->tbl_cfg[tbl_idx].mask, &tbl_cfg->mask, sizeof(natc_key_mask));
#endif

    cfg->tbl_cfg[tbl_idx].vir_addr.key = tbl_cfg->tbl_base_addr;
    cfg->tbl_cfg[tbl_idx].vir_addr.res = (void *)((uint8_t *)cfg->tbl_cfg[tbl_idx].vir_addr.key + cfg->tbl_cfg[tbl_idx].key_tbl_size);

#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
    if (tbl_idx == NATC_TBL_IDX_CT)
    {
        cfg->tbl_cfg[tbl_idx].phy_addr.key = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_ddos_tables_base_virt,
            p_dpi_cfg->rdp_ddr_ddos_tables_base_phys, cfg->tbl_cfg[tbl_idx].vir_addr.key);
        cfg->tbl_cfg[tbl_idx].phy_addr.res = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_ddos_tables_base_virt,
            p_dpi_cfg->rdp_ddr_ddos_tables_base_phys, cfg->tbl_cfg[tbl_idx].vir_addr.res);
    }
    else
#endif
    {
        cfg->tbl_cfg[tbl_idx].phy_addr.key = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_rnr_tables_base_virt,
            p_dpi_cfg->rdp_ddr_rnr_tables_base_phys, cfg->tbl_cfg[tbl_idx].vir_addr.key);
        cfg->tbl_cfg[tbl_idx].phy_addr.res = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_rnr_tables_base_virt,
            p_dpi_cfg->rdp_ddr_rnr_tables_base_phys, cfg->tbl_cfg[tbl_idx].vir_addr.res);
    }
}

#endif

#if !defined(ARRAY_SIZE)
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif
int drv_natc_tbl_database_get_tbl_size(natc_ddr_cfg_natc_ddr_size *ddr_size,
    natc_ddr_cfg_total_len *entry_len_array,
    uint32_t tbl_en_mask, uint32_t mem_avail)
{
    int idx, tbl_idx;
    uint32_t total_mem_reqd;
    uint32_t total_key_ctxt_len;
    ddr_nat_table_size_t tbl_ddr_size_enum;

#if CHIP_VER < RDP_GEN_60 /* 4GEN platforms */
    natc_ddr_cfg_natc_ddr_size ddr_size_db[] =
    {
        { .ddr_size_tbl0 = ddr_size_256k,
            .ddr_size_tbl1 = ddr_size_256k,
            .ddr_size_tbl2 = ddr_size_8k,
            .ddr_size_tbl3 = ddr_size_8k,
            .ddr_size_tbl4 = ddr_size_def,
            .ddr_size_tbl5 = ddr_size_def,
            .ddr_size_tbl6 = ddr_size_def,
            .ddr_size_tbl7 = ddr_size_def
        },
        { .ddr_size_tbl0 = ddr_size_128k,
            .ddr_size_tbl1 = ddr_size_128k,
            .ddr_size_tbl2 = ddr_size_8k,
            .ddr_size_tbl3 = ddr_size_8k,
            .ddr_size_tbl4 = ddr_size_def,
            .ddr_size_tbl5 = ddr_size_def,
            .ddr_size_tbl6 = ddr_size_def,
            .ddr_size_tbl7 = ddr_size_def
        },
        { .ddr_size_tbl0 = ddr_size_64k,
            .ddr_size_tbl1 = ddr_size_64k,
            .ddr_size_tbl2 = ddr_size_8k,
            .ddr_size_tbl3 = ddr_size_8k,
            .ddr_size_tbl4 = ddr_size_def,
            .ddr_size_tbl5 = ddr_size_def,
            .ddr_size_tbl6 = ddr_size_def,
            .ddr_size_tbl7 = ddr_size_def
        },
        /* Below is DEFAULT configuration -- anything
         * below * not recommended and blocked by
         * minimum * memory allocation during boot */
        { .ddr_size_tbl0 = ddr_size_32k,
            .ddr_size_tbl1 = ddr_size_32k,
            .ddr_size_tbl2 = ddr_size_8k,
            .ddr_size_tbl3 = ddr_size_8k,
            .ddr_size_tbl4 = ddr_size_def,
            .ddr_size_tbl5 = ddr_size_def,
            .ddr_size_tbl6 = ddr_size_def,
            .ddr_size_tbl7 = ddr_size_def
        },
        { .ddr_size_tbl0 = ddr_size_16k,
            .ddr_size_tbl1 = ddr_size_16k,
            .ddr_size_tbl2 = ddr_size_8k,
            .ddr_size_tbl3 = ddr_size_8k,
            .ddr_size_tbl4 = ddr_size_def,
            .ddr_size_tbl5 = ddr_size_def,
            .ddr_size_tbl6 = ddr_size_def,
            .ddr_size_tbl7 = ddr_size_def
        },
        { .ddr_size_tbl0 = ddr_size_8k,
            .ddr_size_tbl1 = ddr_size_8k,
            .ddr_size_tbl2 = ddr_size_8k,
            .ddr_size_tbl3 = ddr_size_8k,
            .ddr_size_tbl4 = ddr_size_def,
            .ddr_size_tbl5 = ddr_size_def,
            .ddr_size_tbl6 = ddr_size_def,
            .ddr_size_tbl7 = ddr_size_def
        },
    };


#else /* 6GEN */
    natc_ddr_cfg_natc_ddr_size ddr_size_db[] =
    {
        { .ddr_size_tbl0 = ddr_size_256k,
            .ddr_size_tbl1 = ddr_size_256k,
            .ddr_size_tbl2 = ddr_size_8k,
            .ddr_size_tbl3 = ddr_size_64k,
            .ddr_size_tbl4 = ddr_size_def,
            .ddr_size_tbl5 = ddr_size_def,
            .ddr_size_tbl6 = ddr_size_def,
            .ddr_size_tbl7 = ddr_size_def
        },
        { .ddr_size_tbl0 = ddr_size_128k,
            .ddr_size_tbl1 = ddr_size_128k,
            .ddr_size_tbl2 = ddr_size_8k,
            .ddr_size_tbl3 = ddr_size_64k,
            .ddr_size_tbl4 = ddr_size_def,
            .ddr_size_tbl5 = ddr_size_def,
            .ddr_size_tbl6 = ddr_size_def,
            .ddr_size_tbl7 = ddr_size_def
        },
        { .ddr_size_tbl0 = ddr_size_64k,
            .ddr_size_tbl1 = ddr_size_64k,
            .ddr_size_tbl2 = ddr_size_8k,
            .ddr_size_tbl3 = ddr_size_64k,
            .ddr_size_tbl4 = ddr_size_def,
            .ddr_size_tbl5 = ddr_size_def,
            .ddr_size_tbl6 = ddr_size_def,
            .ddr_size_tbl7 = ddr_size_def
        },
        /* Below is DEFAULT configuration -- anything
         * below * not recommended and blocked by
         * minimum * memory allocation during boot */
        { .ddr_size_tbl0 = ddr_size_32k,
            .ddr_size_tbl1 = ddr_size_32k,
            .ddr_size_tbl2 = ddr_size_8k,
            .ddr_size_tbl3 = ddr_size_64k,
            .ddr_size_tbl4 = ddr_size_def,
            .ddr_size_tbl5 = ddr_size_def,
            .ddr_size_tbl6 = ddr_size_def,
            .ddr_size_tbl7 = ddr_size_def
        },
        { .ddr_size_tbl0 = ddr_size_16k,
            .ddr_size_tbl1 = ddr_size_16k,
            .ddr_size_tbl2 = ddr_size_8k,
            .ddr_size_tbl3 = ddr_size_8k,
            .ddr_size_tbl4 = ddr_size_def,
            .ddr_size_tbl5 = ddr_size_def,
            .ddr_size_tbl6 = ddr_size_def,
            .ddr_size_tbl7 = ddr_size_def
        },
        { .ddr_size_tbl0 = ddr_size_8k,
            .ddr_size_tbl1 = ddr_size_8k,
            .ddr_size_tbl2 = ddr_size_8k,
            .ddr_size_tbl3 = ddr_size_8k,
            .ddr_size_tbl4 = ddr_size_def,
            .ddr_size_tbl5 = ddr_size_def,
            .ddr_size_tbl6 = ddr_size_def,
            .ddr_size_tbl7 = ddr_size_def
        },
    };
#endif /* 6GEN */

    for (idx = 0; idx < ARRAY_SIZE(ddr_size_db); idx++)
    {
        total_mem_reqd = 0;
        for (tbl_idx = 0; tbl_idx < RDD_NATC_TBL_CFG_SIZE; tbl_idx++)
        {
            if ((tbl_en_mask & (0x1 << tbl_idx)) == 0)
                continue;

#ifdef CONFIG_RUNNER_FPI
            /* FPI will share the same DDR memory for TBL_IDX#0-2, and they
             * have to be of the same size, so skip the counting for the
             * first 2 tables. */
            if (tbl_idx < FPI_NATC_TBL_IDX_L3L4)
                continue;
#elif defined(CONFIG_UNIFIED_FLOW_NATC_TBL)
            /* Layer2 and layer 3 tables share the same memory*/
            if (tbl_idx ==  NATC_TBL_IDX_L2)
                continue;
#endif

#if IS_ENABLED(CONFIG_BCM_HW_FIREWALL)
            /* NATC CT_tbl uses its own DDR memory, not in the calculation */
            if (tbl_idx ==  NATC_TBL_IDX_CT)
                continue;
#endif
            total_key_ctxt_len = _natc_tbl_entry_size_bytes(_natc_tbl_entry_size_enum(tbl_idx, entry_len_array));
            tbl_ddr_size_enum = _natc_tbl_ddr_size_enum(tbl_idx, &ddr_size_db[idx]);
            total_mem_reqd += _calc_natc_tbl_size(total_key_ctxt_len, NATC_DEFAULT_BIN_SIZE, tbl_ddr_size_enum);
        }
        if (total_mem_reqd <= mem_avail)
        {
            memcpy(ddr_size, &ddr_size_db[idx], sizeof(*ddr_size));
            bdmf_trace("DB-Idx = %d mem_avail = %u total_mem_reqd = %u\n", idx, mem_avail, total_mem_reqd);
            for (tbl_idx = 0; tbl_idx < RDD_NATC_TBL_CFG_SIZE; tbl_idx++)
            {
                if ((tbl_en_mask & (0x1 << tbl_idx)) == 0)
                    continue;

                bdmf_trace("Flow_Tbl[%d] Entries = %u\n", tbl_idx, _natc_tbl_ddr_size_num(_natc_tbl_ddr_size_enum(tbl_idx, ddr_size)));
            }
            return 0;
        }
    }
    BDMF_TRACE_ERR("NOT enough reserved memory for NATC Tables... minimum memory reqd = %u avail = %u\n", total_mem_reqd, mem_avail);
    return -1;
}

int drv_natc_init(natc_config_t *cfg)
{
    uint8_t tbl_idx;
    uint32_t key_addr_hi, key_addr_lo, res_addr_hi, res_addr_lo;
#if CHIP_VER<RDP_GEN_61 && (!defined(RDP_SIM)) 
    uint32_t cache_entry_idx;
    natc_indir_data indir_data = {};
#endif
    bdmf_error_t rc = BDMF_ERR_OK;

    g_natc_tbl_en_mask = cfg->tbl_en_mask;

    if (g_natc_tbl_en_mask & ~(((uint32_t)0x1 << RDD_NATC_TBL_CFG_SIZE) - 1))
        return BDMF_ERR_RANGE;
        
    for (tbl_idx = 0; tbl_idx < RDD_NATC_TBL_CFG_SIZE; tbl_idx++)
    {
        if ((g_natc_tbl_en_mask & (0x1 << tbl_idx)) == 0)
            continue;

        rc = rc ? rc : drv_natc_tbl_ctrl_set(tbl_idx, &cfg->tbl_cntrl[tbl_idx]);
        GET_ADDR_HIGH_LOW(key_addr_hi, key_addr_lo, cfg->tbl_cfg[tbl_idx].phy_addr.key);
        rc = rc ? rc : ag_drv_natc_cfg_key_addr_set(tbl_idx, key_addr_lo >> 3, key_addr_hi);
        GET_ADDR_HIGH_LOW(res_addr_hi, res_addr_lo, cfg->tbl_cfg[tbl_idx].phy_addr.res);
        rc = rc ? rc : ag_drv_natc_cfg_res_addr_set(tbl_idx, res_addr_lo >> 3, res_addr_hi);

        rdd_natc_tbl_cfg(tbl_idx, key_addr_hi, key_addr_lo, res_addr_hi, res_addr_lo);

        /* initialize ddr table */
        MEMSET((uint8_t *)cfg->tbl_cfg[tbl_idx].vir_addr.key, 0, cfg->tbl_cfg[tbl_idx].key_tbl_size);
        MEMSET((uint8_t *)cfg->tbl_cfg[tbl_idx].vir_addr.res, 0, cfg->tbl_cfg[tbl_idx].res_tbl_size);

        memcpy(&g_natc_tbl_cfg[tbl_idx], &cfg->tbl_cfg[tbl_idx], sizeof(natc_tbl_config_t));
    }

/* cache reset is not needed in newer chips */
#if CHIP_VER<RDP_GEN_61 && (!defined(RDP_SIM)) 
    /* init cache entries and their stats */
    for (cache_entry_idx = 0; cache_entry_idx < NATC_CACHE_ENTRIES_NUM; cache_entry_idx++)
    {
        ag_drv_natc_indir_data_set(0, &indir_data);
        ag_drv_natc_indir_addr_set(1, cache_entry_idx);
    }
#endif

    rdd_crc_init();
#if CHIP_VER < RDP_GEN_62
    g_hash_mode = cfg->ctrl_status.ddr_hash_mode;
#else
    g_hash_mode = drv_natc_ddr_hash_mode();
#endif
    return rc ? rc : ag_drv_natc_ctrl_status_set(&cfg->ctrl_status);
}

int drv_natc_cfg_get(natc_config_t *cfg)
{
    uint8_t tbl_idx;

    cfg->tbl_en_mask = g_natc_tbl_en_mask;

    for (tbl_idx = 0; tbl_idx < RDD_NATC_TBL_CFG_SIZE; tbl_idx++)
    {
        if ((g_natc_tbl_en_mask & (0x1 << tbl_idx)) == 0)
            continue;

        memcpy(&cfg->tbl_cfg[tbl_idx], &g_natc_tbl_cfg[tbl_idx], sizeof(natc_tbl_config_t));
    }

    return 0;
}

static void drv_natc_hash_mode_ctrl_status(natc_ctrl_status *ctrl_status)
{
    #if CHIP_VER >= RDP_GEN_62
    natc_ddr_cfg_ddr_hash_mode ddr_hash_mode;
    hash_mode_t ddr_mode = drv_natc_ddr_hash_mode();

    ddr_hash_mode.ddr_hash_mode_tbl0 = ddr_mode;
    ddr_hash_mode.ddr_hash_mode_tbl1 = ddr_mode;
    ddr_hash_mode.ddr_hash_mode_tbl2 = ddr_mode;
    ddr_hash_mode.ddr_hash_mode_tbl3 = ddr_mode;
    ddr_hash_mode.ddr_hash_mode_tbl4 = ddr_mode;
    ddr_hash_mode.ddr_hash_mode_tbl5 = ddr_mode;
    ddr_hash_mode.ddr_hash_mode_tbl6 = ddr_mode;
    ddr_hash_mode.ddr_hash_mode_tbl7 = ddr_mode;
    ag_drv_natc_ddr_cfg_ddr_hash_mode_set(&ddr_hash_mode);
#else
    ctrl_status->ddr_hash_mode = drv_natc_ddr_hash_mode();
#endif

    ctrl_status->nat_hash_mode = drv_natc_hash_mode();
}

static void drv_natc_hash_mode_cfg(natc_config_t *cfg)
{
    drv_natc_hash_mode_ctrl_status(&cfg->ctrl_status);
    cfg->ctrl_status.multi_hash_limit = multi_hash_limit_def;
}

#if CHIP_VER < RDP_GEN_60

int drv_natc_set_key_mask(uint16_t tbl_idx, uint16_t mask)
{
    if ((g_natc_tbl_en_mask & (0x1 << tbl_idx)) == 0)
        return 0;
    g_natc_tbl_cfg[tbl_idx].mask = mask;
    return ag_drv_natc_key_mask_tbl_key_mask_set(tbl_idx, mask);
}

#else /* 6GEN platforms */

#if CHIP_VER >= RDP_GEN_62
int drv_natc_flow_cntr_cfg(void)
{
    natc_flow_cntr_cntl flow_cntr_cntl = {};

#ifdef BCM6888_B0
    flow_cntr_cntl.flow_cntr_clr_on_rd_en = 0;
#endif
    flow_cntr_cntl.flow_cntr_en_tbl7 = 0;
    flow_cntr_cntl.flow_cntr_en_tbl6 = 0;
    flow_cntr_cntl.flow_cntr_en_tbl5 = 0;
    flow_cntr_cntl.flow_cntr_en_tbl4 = 0;
    flow_cntr_cntl.flow_cntr_en_tbl3 = 0;
    flow_cntr_cntl.flow_cntr_en_tbl2 = 0;
    flow_cntr_cntl.flow_cntr_en_tbl1 = 1;
    flow_cntr_cntl.flow_cntr_en_tbl0 = 1;

#ifdef PATH_STAT_WA
    /* table size - 1 - offset - external control size (1 word) */
    flow_cntr_cntl.context_offset = NATC_TABLE_SIZE_128 - 1 - FC_UCAST_FLOW_CONTEXT_ENTRY_PATHSTAT_IDX_OFFSET - sizeof(uint32_t) ;
#else
    flow_cntr_cntl.context_offset = FC_UCAST_FLOW_CONTEXT_ENTRY_PATHSTAT_IDX_OFFSET - sizeof(uint32_t);
#endif
    return ag_drv_natc_flow_cntr_cntl_set(&flow_cntr_cntl);
}
#endif

int drv_natc_set_key_mask(uint16_t tbl_idx, natc_key_mask *mask)
{
    if ((g_natc_tbl_en_mask & (0x1 << tbl_idx)) == 0)
        return 0;
    g_natc_tbl_cfg[tbl_idx].mask = *mask;
    return drv_natc_key_mask_set(tbl_idx, 0, mask);
}

#endif


/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/

#ifdef USE_BDMF_SHELL
#define TABLE_CTRL_KEY_LEN_GET(tbl_ctrl)       (NATC_TABLE_KEY_SIZE * (1 + ((tbl_ctrl & 0x2) >> 0x1)))
#define TABLE_CTRL_NON_CACHEABLE_GET(tbl_ctrl) (tbl_ctrl & 0x1)

#if (CHIP_VER < RDP_GEN_100) /* CRDP-TODO */ /* CLI interface names are not erlevant anymore - need to rewrite CLI code */
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
#endif

/* Simplified table from rdpa_natc_table_name_enum_table */
const char *drv_natc_table_name_str[rdpa_natc_table_name__num_of] =
{ "L3 Unicast", "L2 Unicast", "Multicast", "ConnTrack", "Downstream",
   "Upstream", "VLAN action", "Counter", "Multicast Whitelist"};

int drv_natc_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
#if (CHIP_VER < RDP_GEN_100) /* CRDP-TODO */
    uint8_t tbl_idx;
    static uint32_t natc_tbl_cfg[] = {cli_natc_cfg_key_addr, cli_natc_cfg_res_addr};
    static uint32_t natc_status[] = {cli_natc_ctrl_status};
    natc_tbl_config_t *cfg;
    bdmf_error_t rc = BDMF_ERR_OK;

    bdmf_session_print(session, "NATC configurations:\n");
    for (tbl_idx = 0; tbl_idx < RDD_NATC_TBL_CFG_SIZE; tbl_idx++)
    {
        if (g_natc_tbl_en_mask & (0x1 << tbl_idx))
        {
            cfg = &g_natc_tbl_cfg[tbl_idx];
            bdmf_session_print(session, "Table [%d]:\n", tbl_idx);
            if (cfg->table_name < rdpa_natc_table_name__num_of)
                bdmf_session_print(session, "Usage = %s\n",
                                   drv_natc_table_name_str[cfg->table_name]);
            bdmf_session_print(session, "Key Len = %d, Result Len =%d\n",
                               cfg->key_len, cfg->res_len);
            bdmf_session_print(session, "Number of Entry = %d\n",
                               cfg->tbl_entry_num);
            bdmf_session_print(session, "Key Table: Virt@%lx, Phy@%lx, Size %d byte\n",
                               (long unsigned int)(uintptr_t)cfg->vir_addr.key,
                               (long unsigned int)(uintptr_t)cfg->phy_addr.key,
                               cfg->key_tbl_size);
            bdmf_session_print(session, "Result Table: Virt@%lx, Phy@%lx, Size %d byte\n\n",
                               (long unsigned int)(uintptr_t)cfg->vir_addr.res,
                               (long unsigned int)(uintptr_t)cfg->phy_addr.res,
                               cfg->res_tbl_size);
            rc = rc ? rc : _drv_natc_tbl_ctrl_cli_get(session, tbl_idx);
#if CHIP_VER < RDP_GEN_60
            HAL_CLI_IDX_PRINT_LIST(session, natc_cfg, natc_tbl_cfg, tbl_idx);
#else
            HAL_CLI_IDX_PRINT_LIST(session, natc_tbl, natc_tbl_cfg, tbl_idx);
#endif
            bdmf_session_print(session, "\n");
        }
    }
    HAL_CLI_PRINT_LIST(session, natc, natc_status);
    return rc;
#else
    return 0;
#endif
}

int drv_natc_cli_cache_dump(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
/* TODO 6888 6837 6885*/
#if !defined(BCM6888) && !defined(BCM6837) && !defined(BCM68375) && !defined(BCM6885)
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
#else
    return 0;
#endif
}

int drv_natc_cli_hash_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_session_print(session, "\nnatc hash function = %s\nddr hash function = %s\n",
        bdmfmon_enum_stringval(natc_hash_enum_table, g_natc_hash_mode),
        bdmfmon_enum_stringval(natc_hash_enum_table, g_hash_mode));
    return BDMF_ERR_OK;
}

int drv_natc_cli_set_hash(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
#if (CHIP_VER < RDP_GEN_100) /* CRDP-TODO */
    bdmf_error_t rc;
    natc_ctrl_status ctrl_status = { 0 };
    uint8_t natc_hash_mode = g_natc_hash_mode;
    uint8_t ddr_hash_mode = g_hash_mode;

    rc = ag_drv_natc_ctrl_status_get(&ctrl_status);
    if (rc)
        return rc;

    g_hash_mode = parm[1].value.unumber;
    g_natc_hash_mode = parm[0].value.unumber;
    drv_natc_hash_mode_ctrl_status(&ctrl_status);

    rc = ag_drv_natc_ctrl_status_set(&ctrl_status);
    if (rc)
    {
        g_hash_mode = ddr_hash_mode;
        g_natc_hash_mode = natc_hash_mode;
    }

    return rc;
#else
    return 0;
#endif
}

int drv_natc_cli_get_counters(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t rc;
    uint64_t packets;
    uint64_t bytes;
    uint32_t index = (uint16_t)parm[1].value.unumber;

    rc = drv_natc_entry_counters_get((uint16_t)parm[0].value.unumber, index, &packets, &bytes);
    if (rc==BDMF_ERR_OK)
        bdmf_trace("natc counter for index%d packet=%u bytes=%u\n", index, (unsigned int)packets, (unsigned int)bytes);
    else
        bdmf_trace("ERROR =%d\n", rc);  
    return rc;
}

int drv_natc_cli_add_counter(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    bdmf_error_t rc;
#if defined(BCM_PON_XRDP)
    uint32_t entry_index = 0xFFFFFFFF;
    uint32_t sub_table_id = (uint32_t)parm[0].value.unumber;
    uint32_t counter_key = (uint16_t)(parm[1].value.unumber);

    rc = drv_natc_counters_add_new_counter(sub_table_id, &entry_index, counter_key);
    if (rc==BDMF_ERR_OK)
        bdmf_trace("natc counter add. sub_table_id=%d, key=%d ,index=%d\n", sub_table_id, counter_key, entry_index);
    else
        bdmf_trace("ERROR =%d\n", rc);  
#else
    rc = BDMF_ERR_NOT_SUPPORTED;
#endif
    return rc;
}

#if CHIP_VER < RDP_GEN_60
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
#else
int drv_natc_cli_set_mask(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    natc_key_mask *mask;
    uint16_t  tbl_idx;
    bdmf_error_t rc = BDMF_ERR_OK;

    tbl_idx = (uint16_t)parm[0].value.unumber;
    mask = (natc_key_mask *)parm[1].value.string;
    
    g_natc_tbl_cfg[tbl_idx].mask = *mask;
    
    rc = drv_natc_key_mask_set(tbl_idx, 0, mask);

   return rc;
}
#endif

void drv_natc_basic_cfg_init(natc_config_t *cfg, uint32_t tbl_en_mask)
{
    memset(cfg, 0, sizeof(natc_config_t));
    drv_natc_hash_mode_cfg(cfg);

    cfg->ctrl_status.age_timer_tick = timer_tick_packet;
    cfg->ctrl_status.age_timer = age_timer_256_ticks;
    cfg->ctrl_status.cache_update_on_reg_ddr_lookup = 1;
    cfg->ctrl_status.ddr_enable = 1;
    cfg->ctrl_status.natc_enable = 1;
#ifndef BCM6858
    cfg->ctrl_status.cache_update_on_ddr_miss = 1;
#endif
    cfg->tbl_en_mask = tbl_en_mask;
}

#if (CHIP_VER < RDP_GEN_100) /* CRDP-TODO */
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
#endif

int drv_natc_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
#if (CHIP_VER < RDP_GEN_100) /* CRDP-TODO */
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
    for (tbl_idx = 0; !rc && tbl_idx < RDD_NATC_TBL_CFG_SIZE; tbl_idx++)
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
        for (tbl_idx = 0; !rc && tbl_idx < RDD_NATC_TBL_CFG_SIZE; tbl_idx++)
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
#else
    return 0;
#endif
}

static bdmfmon_handle_t natc_dir;
void drv_natc_cli_init(bdmfmon_handle_t driver_dir)
{
    if ((natc_dir = bdmfmon_dir_find(driver_dir, "natc"))!=NULL)
        return;
    natc_dir = bdmfmon_dir_add(driver_dir, "natc", "natc driver", BDMF_ACCESS_ADMIN, NULL);
#if (CHIP_VER < RDP_GEN_100) /* CRDP-TODO */

    ag_drv_natc_cli_init(natc_dir);
    ag_drv_natc_cfg_cli_init(natc_dir);
    ag_drv_natc_eng_cli_init(natc_dir);
    
#if !defined(BCM6888) && !defined(BCM6837) && !defined(BCM68375)
    ag_drv_natc_indir_cli_init(natc_dir);
#endif
    ag_drv_natc_ctrs_cli_init(natc_dir);
#if CHIP_VER < RDP_GEN_60
    ag_drv_natc_key_mask_cli_init(natc_dir);
#else
    ag_drv_natc_key_cli_init(natc_dir);
#endif

#else
    ag_drv_nat_cache_cli_init(natc_dir);
#endif

    BDMFMON_MAKE_CMD_NOPARM(natc_dir, "cfg_get", "natc configuration", (bdmfmon_cmd_cb_t)drv_natc_cli_config_get);
    BDMFMON_MAKE_CMD_NOPARM(natc_dir, "dbg_get", "natc configuration", (bdmfmon_cmd_cb_t)drv_natc_cli_debug_get);
    BDMFMON_MAKE_CMD_NOPARM(natc_dir, "cd", "print non-zero cache entries", (bdmfmon_cmd_cb_t)drv_natc_cli_cache_dump);

    BDMFMON_MAKE_CMD_NOPARM(natc_dir, "hash_get", "natc and ddr hash functions", (bdmfmon_cmd_cb_t)drv_natc_cli_hash_get);
    BDMFMON_MAKE_CMD(natc_dir, "hash_set",   "set natc hashes", (bdmfmon_cmd_cb_t)drv_natc_cli_set_hash,
        BDMFMON_MAKE_PARM_ENUM("natc hash", "natc hash function", natc_hash_enum_table, 0),
        BDMFMON_MAKE_PARM_ENUM("ddr hash", "DDR hash function", natc_hash_enum_table, 0));

#if CHIP_VER < RDP_GEN_60
    BDMFMON_MAKE_CMD(natc_dir, "mask",   "set natc mask", (bdmfmon_cmd_cb_t)drv_natc_cli_set_mask,
        BDMFMON_MAKE_PARM_ENUM("table index", "table index", natc_enum_table, 0),
        BDMFMON_MAKE_PARM("key mask", "key mask", BDMFMON_PARM_NUMBER, 0));
#endif
    BDMFMON_MAKE_CMD(natc_dir, "read_counters",   "read nat cache counters", (bdmfmon_cmd_cb_t)drv_natc_cli_get_counters,
            BDMFMON_MAKE_PARM_ENUM("table index", "table index", natc_enum_table, 0),
            BDMFMON_MAKE_PARM("index", "counter index", BDMFMON_PARM_NUMBER, 0));

    BDMFMON_MAKE_CMD(natc_dir, "add_counter",   "add nat cache counter", (bdmfmon_cmd_cb_t)drv_natc_cli_add_counter,
            BDMFMON_MAKE_PARM_ENUM("table index", "table index", natc_enum_table, 0),
            BDMFMON_MAKE_PARM("key", "counter key", BDMFMON_PARM_NUMBER, 0));
}

void drv_natc_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (natc_dir)
    {
        bdmfmon_token_destroy(natc_dir);
        natc_dir = NULL;
    }
}

bdmf_boolean drv_natc_tos_mflows_get(void)
{
    bdmf_boolean enabled = 0;
    int rc = BDMF_ERR_OK;

#if CHIP_VER < RDP_GEN_60
    uint32_t key_mask = 0;
    rc = ag_drv_natc_key_mask_tbl_key_mask_get(NATC_TBL_IDX_DS, &key_mask);

    enabled = !(key_mask & NATC_16BYTE_KEY_TOS_MASK);
#else /* RDP_GEN_60 */
    natc_key_mask key_mask = NATC_16BYTE_KEY_MASK;
    rc = drv_natc_key_mask_get(NATC_TBL_IDX_L3, 0, &key_mask);

    enabled = !(key_mask.data[NATC_16BYTE_KEY_TOS_WORD_INDEX] & NATC_16BYTE_KEY_TOS_WORD_MASK);
#endif
    if (rc)
    {
        bdmf_trace("ERROR: %s:%u %s | err=%s (%d)\n", __FILE__, __LINE__, __FUNCTION__, bdmf_strerror(rc), rc);
    }

    return enabled;
}

#endif /* USE_BDMF_SHELL */

