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
#ifndef _RDP_DRV_HASH_H_
#define _RDP_DRV_HASH_H_

#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_hash_ag.h"

#define HASH_MAX_TABLES_NUM                 7
#define HASH_MAX_CTX_SIZE                   18
#define HASH_NUM_OF_ENGINES                 4
#define HASH_NUM_OF_EFFECTIVE_ENGINES       1
#define HASH_INVALID_IDX	                0xFFFF
#define HASH_NUM_ENTRIES_PER_ENGINE         1536
#define HASH_CAN_NUM_ENTRIES                64
#define HASH_CRC_BYTE_SIZE                  8
#define HASH_KEY_SIZE                       60
#define CONTEXT_ENTRY_MIN_SIZE              3
#define CAM_READ_CMD                        0
#define CAM_WRITE_CMD                       1
#define CAM_SEARCH_CMD                      2
#define CAM_VALID_SET_CMD                   3

/* default values */
#define HASH_PAD_VALUE_H                    0x5555555
#define HASH_PAD_VALUE_L                    0x55555555

/* Macros */
#define CONSTRUCT_KEY(high, mid, low) \
    ((((uint64_t)(high))<<28) | (((uint64_t)(mid))<<12) | low)

#define VERIFY_TABLE_ID(table_id) \
        (((table_id) < 1) || ((table_id) > 7))

#define VERIFY_TBL_INIT_DATA(int_ctx_size, ext_ctx_size, search_depth) \
        (((int_ctx_size) % 3 == 0) && ((int_ctx_size) % 3 == 0) && ((search_depth) >= 1) && ((search_depth) <= 16))

#define GET_TABLE_ID(table_id) \
        ((table_id) - 1)

#define UNITE_HIGH_LOW(high, low) \
        (((uint64_t)(high) << 32) + (low))

#define UNITE_HIGH_LOW_60(high, low) \
        (((uint64_t)(high) << 28) + ((uint64_t)(low)))

#define GET_TBL_SIZE(tbl_size_cfg) \
        (((tbl_size_cfg) == 7) ? (1536) : (((uint16_t)1) << ((tbl_size_cfg) + 4)))

#define GET_INDEX(crc, depth, tbl_size, base_addr) \
        ((((crc) + (depth)) % (tbl_size)) + (base_addr))

#define GET_ENTRY_IDX_PER_ENGINE(idx, eng) \
        ((idx << 2) | (eng & 0x3))

#define GET_ENG_FROM_INDEX(idx) \
        ((idx) & 0x3)

#define GET_IDX_FROM_INDEX(idx) \
        ((idx) >> 2)

#define GET_CONTEXT_INDEX(ctx_base_addr, match_idx, match_eng, hash_base_addr, ext_ctx_chunks_num) \
        (ctx_base_addr) + (((match_idx) - (hash_base_addr) + (match_eng)) * ((ext_ctx_chunks_num)))

#define ALIGN_CTX_SIZE_TO_WORD(size)   (4 * (size / 4 + size % 4 ? 1 : 0))

#define IS_SAME_HASH_IDX(hash_idx0, hash_idx1, hash_idx2, hash_idx3) ( (hash_idx0 == hash_idx1) \
                                                                       || (hash_idx0 == hash_idx2) \
                                                                       || (hash_idx0 == hash_idx3)  ? hash_idx0 : HASH_INVALID_IDX)
typedef enum
{
    HASH_MISS,
    HASH_MATCH_IN_TABLE,
    RESERVED,
    HASH_MATCH_IN_CAM,
} hash_match_t;

typedef struct hash_result
{
        hash_match_t                    match;
        uint16_t                        match_index;
        uint16_t                        context_addr;
        uint16_t                        first_free_idx;
        uint16_t                        duplicate_idx;
} hash_result_t;

typedef struct hash_tbl_init_input
{
        uint8_t                        int_ctx_size;
        uint8_t                        ext_ctx_size;
        uint8_t                        search_depth_per_engine;
} hash_tbl_init_input_t;

typedef struct hash_config
{
        uint32_t                        tbl_num;
        hash_lkup_tbl_cfg_tbl_cfg       tbl_cfg[HASH_MAX_TABLES_NUM];
        hash_mask                       mask[HASH_MAX_TABLES_NUM];
        hash_lkup_tbl_cfg_cntxt_cfg     ctx_cfg[HASH_MAX_TABLES_NUM];
        hash_tbl_init_input_t           tbl_init[HASH_MAX_TABLES_NUM];
        uint16_t                        cam_base_addr;
} hash_config_t;

typedef enum
{
    hash_max_16_entries_per_engine,
    hash_max_32_entries_per_engine,
    hash_max_64_entries_per_engine,
    hash_max_128_entries_per_engine,
    hash_max_256_entries_per_engine,
    hash_max_512_entries_per_engine,
    hash_max_1k_entries_per_engine,
    hash_max_1_5k_entries_per_engine,
} tbl_size_e;

bdmf_error_t drv_hash_find(uint8_t table_id, uint8_t *key , hash_result_t *hash_res);
bdmf_error_t drv_hash_key_get(uint8_t table_id, uint16_t idx , uint8_t *key);
bdmf_error_t drv_hash_rule_add(uint32_t table_id, uint8_t *key, uint8_t *ext_ctx, uint8_t *int_ctx);
bdmf_error_t drv_hash_rule_insert(uint32_t table_id, uint16_t idx, uint8_t *key_p, uint8_t *ext_ctx, uint8_t *int_ctx);
bdmf_error_t drv_hash_rule_remove_key(uint32_t table_id, uint8_t *key);
bdmf_error_t drv_hash_rule_remove_index(uint16_t idx);
bdmf_error_t drv_hash_set_aging(uint16_t idx, bdmf_boolean *prev_age);
bdmf_error_t drv_hash_modify_context(uint8_t table_id, uint16_t idx, uint8_t *ext_ctx, uint8_t *int_ctx);
bdmf_error_t drv_hash_get_context(uint16_t idx, uint8_t table_id, uint8_t *ext_ctx, uint8_t *int_ctx);
bdmf_error_t drv_hash_init(hash_config_t *cfg);

#ifdef USE_BDMF_SHELL
void drv_hash_cli_init(bdmfmon_handle_t driver_dir);
void drv_hash_cli_exit(bdmfmon_handle_t driver_dir);
#endif
#endif
