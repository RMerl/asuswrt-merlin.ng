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
#include "rdp_drv_hash.h"
#include "rdd_crc.h"
#include "rdp_common.h"
#include "XRDP_AG.h"

#define POLLING_TIME_OUT  1000

hash_config_t g_hash_cfg;

static inline uint16_t _get_hash_table_index(uint16_t crc, uint16_t depth, uint8_t table_id)
{
    uint16_t tbl_size = GET_TBL_SIZE(g_hash_cfg.tbl_cfg[table_id].tbl_size);
    return ((crc + depth) % tbl_size) + g_hash_cfg.tbl_cfg[table_id].hash_base_addr;
}

static inline uint64_t _uint8_arr_to_uint64(uint8_t *src, uint32_t byte_len)
{
    uint32_t i;
    uint64_t res = 0;

    if (byte_len > 8)
        byte_len = 8;
    for (i = 0; i < byte_len; i++)
    {
        res <<= 8;
        res |= src[i];
    }
    return res;
}

static inline uint8_t _get_mask_byte_len(uint64_t mask)
{
    uint8_t i = 1;

    while ((mask >> i*8) != 0)
        i++;
    return i;
}

static void _copy_byte_by_byte(uint8_t *dst, uint64_t src, uint8_t size)
{
    uint32_t i;

    for (i = 0; i < size; i++)
    {
        dst[size - i - 1] = src & 0xFF;
        src >>= 8;
    }
}
/************************************************************************************
 * This function regulates context to be word aligned and swaps BE to LE:           *
 * For Instance:                                                                    *
 * for 3  bytes internal context:                                                   *
 *  A B C ->  A B C 0 -> 0 C B A                                                    *
 * for 6  bytes internal context:                                                   *
 *  A B C D E F -> A B C D E F 0 0 -> D C B A 0 0 F E                               *
 ************************************************************************************/
static void _drv_hash_align_and_swap(uint8_t *unaligned_ctx, uint8_t *aligned_ctx, uint8_t unaligned_size)
{
    uint8_t aligned_size = ALIGN_CTX_SIZE_TO_WORD(unaligned_size);

    memset(aligned_ctx, 0, aligned_size);
    memcpy(aligned_ctx + aligned_size - unaligned_size, unaligned_ctx, unaligned_size);
    SWAPBYTES(aligned_ctx, aligned_size);
}

static void _drv_hash_unalign_and_swap(uint8_t *unaligned_ctx, uint8_t *aligned_ctx, uint8_t unaligned_size)
{
    uint8_t aligned_size = ALIGN_CTX_SIZE_TO_WORD(unaligned_size);

    SWAPBYTES(aligned_ctx, aligned_size);
    memset(unaligned_ctx, 0, unaligned_size);
    memcpy(unaligned_ctx, aligned_ctx + aligned_size - unaligned_size, unaligned_size);
}

static void _drv_hash_copy_swap_key(uint8_t *key, uint8_t *swapped_key, uint8_t key_size)
{
    memcpy(swapped_key, key, key_size);
    SWAPBYTES(swapped_key, key_size);
}

void _drv_hash_calc_base_addrs(uint8_t tbl_idx, hash_config_t *cfg)
{
    uint32_t prev_entries, first_hash_idx = 0, ctx_base_addr = 0;
    uint16_t base_addr_count = 0;
    uint8_t prev_tbl;
    int tbl;

    for (prev_tbl = 0; prev_tbl < tbl_idx; prev_tbl++)
        base_addr_count += GET_TBL_SIZE(cfg->tbl_cfg[prev_tbl].tbl_size);
    cfg->tbl_cfg[tbl_idx].hash_base_addr = base_addr_count;

    for (tbl = 0; tbl < tbl_idx; tbl++)
    {
        prev_entries = GET_TBL_SIZE(cfg->tbl_cfg[tbl].tbl_size) * HASH_NUM_OF_ENGINES;
        first_hash_idx += prev_entries;
        ctx_base_addr += prev_entries * (cfg->tbl_init[tbl].ext_ctx_size / 6);
    }
    cfg->ctx_cfg[tbl_idx].base_address = ctx_base_addr;
    cfg->ctx_cfg[tbl_idx].first_hash_idx = first_hash_idx;
}

bdmf_error_t drv_hash_init(hash_config_t *cfg)
{
    uint8_t tbl_idx;
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t eng, entry_idx, key_11_0 = 0, key_27_12 = 0, key_59_28 = 0;

    /* general configuration */
    for (tbl_idx = 0; !rc && (tbl_idx < cfg->tbl_num); tbl_idx++)
    {
        cfg->tbl_cfg[tbl_idx].direct_lkup_en = 0;   /* direct lookup is not supported in A0 */
        cfg->tbl_cfg[tbl_idx].hash_type = 0;        /* hash_type can only be 0 in A0 */

        /* verify input is valid */
        if (!VERIFY_TBL_INIT_DATA(cfg->tbl_init[tbl_idx].int_ctx_size, cfg->tbl_init[tbl_idx].ext_ctx_size, cfg->tbl_init[tbl_idx].search_depth_per_engine))
            return BDMF_ERR_PARM;
        /* convert input to cfg values */
        cfg->tbl_cfg[tbl_idx].int_ctx_size = cfg->tbl_init[tbl_idx].int_ctx_size / 3;
        cfg->ctx_cfg[tbl_idx].ext_ctx_size = cfg->tbl_init[tbl_idx].ext_ctx_size / 3;
        cfg->tbl_cfg[tbl_idx].max_hop = cfg->tbl_init[tbl_idx].search_depth_per_engine - 1;

        /* calc base_addr */
        _drv_hash_calc_base_addrs(tbl_idx, cfg);
        /* calculation: 1536 lines of HLKUP ram * 2 lines of context ram(3B * 4engines) = 3072. */
        cfg->cam_base_addr = 3072;

        rc = rc ? rc : ag_drv_hash_lkup_tbl_cfg_tbl_cfg_set(tbl_idx, cfg->tbl_cfg);
        rc = rc ? rc : ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(tbl_idx, cfg->ctx_cfg);
        rc = rc ? rc : ag_drv_hash_mask_set(tbl_idx, cfg->mask);
        rc = rc ? rc : ag_drv_hash_cam_base_addr_set(cfg->cam_base_addr);
    }

    /* initializing Hash key RAM */
    for (entry_idx = 0; !rc && entry_idx < HASH_NUM_ENTRIES_PER_ENGINE; entry_idx++)
    {
        for (eng = 0; eng < HASH_NUM_OF_ENGINES; eng++)
        {
            rc = rc ? rc : ag_drv_hash_ram_eng_low_set(GET_ENTRY_IDX_PER_ENGINE(entry_idx, eng), 1, 0, key_11_0, key_27_12);
            rc = rc ? rc : ag_drv_hash_ram_eng_high_set(GET_ENTRY_IDX_PER_ENGINE(entry_idx, eng), key_59_28);
        }
    }
#if defined(RDP_SIM) && !defined(XRDP_EMULATION)
    rc = rc ? rc : ag_drv_hash_key_padding_set(HASH_PAD_VALUE_H, HASH_PAD_VALUE_L);
#endif

    memcpy(&g_hash_cfg, cfg, sizeof(hash_config_t));
    return rc;
}

#if !defined(_CFE_) && !defined(RDP_SIM)
static bdmf_error_t _drv_hash_cam_wait_indirect(void)
{
    int time_out = POLLING_TIME_OUT;
    bdmf_boolean is_done = 0;
    bdmf_error_t rc = BDMF_ERR_OK;

    while ((time_out > 0) && !rc && !is_done)
    {
        rc = ag_drv_hash_cam_indirect_op_done_get(&is_done);
        time_out--;
    }

    if (!rc && (time_out <= 0))
        return BDMF_ERR_INTERNAL;

    return rc;
}
#endif

static bdmf_error_t _drv_hash_cam_search(uint8_t *key_p, hash_result_t *hash_res)
{
#if !defined(_CFE_) && !defined(RDP_SIM)
    uint8_t cam_idx;
    uint16_t cam_base_addr;
    uint64_t key;
    bdmf_boolean cam_match;
    bdmf_error_t rc;

    key = _uint8_arr_to_uint64(key_p, 8);

    rc = ag_drv_hash_cam_indirect_key_in_set(0, (hash_cam_indirect_key_in *)&key);
    rc = rc ? rc : ag_drv_hash_cam_indirect_op_set(CAM_SEARCH_CMD);
    if (rc)
        return rc;

    _drv_hash_cam_wait_indirect();

    rc = rc ? rc : ag_drv_hash_cam_indirect_rslt_get(&cam_match, &cam_idx);
    if (rc)
        return rc;
    if (cam_match && !rc)
    {
        hash_res->match = HASH_MATCH_IN_CAM;
        rc = rc ? rc : ag_drv_hash_cam_base_addr_get(&cam_base_addr);
        hash_res->match_index = cam_base_addr + cam_idx;
    }
    return rc;
#else
    return BDMF_ERR_INTERNAL;
#endif
}

bdmf_error_t drv_hash_find(uint8_t table_id, uint8_t *key_p, hash_result_t *hash_res)
{
    uint8_t eng, cfg, tbl_idx, key_to_crc[8];
    uint16_t en_idx, eng_hash[4], key_11_0, key_27_12;
    uint32_t depth;
    uint64_t mask, padding, key, entry_key;
    uint32_t pad_l, pad_h, key_59_28;
    bdmf_boolean skp;
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t swapped_key_p[8] = {};

    if (VERIFY_TABLE_ID(table_id))
            return BDMF_ERR_PARM;
    tbl_idx = GET_TABLE_ID(table_id);

    hash_res->match = HASH_MISS;
    hash_res->first_free_idx = HASH_INVALID_IDX;
    hash_res->duplicate_idx = HASH_INVALID_IDX;
    mask = UNITE_HIGH_LOW(g_hash_cfg.mask[tbl_idx].maskh, g_hash_cfg.mask[tbl_idx].maskl);
    _drv_hash_copy_swap_key(key_p, swapped_key_p, 8);
    key = _uint8_arr_to_uint64(swapped_key_p, 8);
    key &= mask;
    ag_drv_hash_key_padding_get(&pad_l, &pad_h);
    padding = UNITE_HIGH_LOW_60(pad_h, pad_l);
    padding &= (~mask);
    key |= padding;
    _copy_byte_by_byte(key_to_crc, key, HASH_CRC_BYTE_SIZE);
    /* this calculates the initial hash for each engine */
    for (eng = 0; eng < HASH_NUM_OF_ENGINES; eng++)
        eng_hash[eng] = rdd_crc_bit_by_bit_hash(&key_to_crc[0], HASH_KEY_SIZE, HASH_CRC_BYTE_SIZE, (crc16_polynom_t)eng);

    /* performing Hash table search for each engine */
    for (depth = 0; depth < g_hash_cfg.tbl_init[tbl_idx].search_depth_per_engine; depth++)
    {
        for (eng = 0; eng < HASH_NUM_OF_EFFECTIVE_ENGINES; eng++)
        {
            /* getting the index to search in each engine RAM */
            en_idx = _get_hash_table_index(eng_hash[eng], depth, tbl_idx);

            /* extracting the key from RAM */
            rc = rc ? rc : ag_drv_hash_ram_eng_low_get(GET_ENTRY_IDX_PER_ENGINE(en_idx, eng), &skp, &cfg, &key_11_0, &key_27_12);
            rc = rc ? rc : ag_drv_hash_ram_eng_high_get(GET_ENTRY_IDX_PER_ENGINE(en_idx, eng), &key_59_28);

            /* if entry is free and no free entry was found earlier mark entry as first available entry */
            if (!rc)
            {
                if ((skp && hash_res->first_free_idx == HASH_INVALID_IDX) || cfg == 0)
                {
                    hash_res->first_free_idx = GET_ENTRY_IDX_PER_ENGINE(en_idx, eng);
                    /* if we reached an invalid entry- exit search with MISS */
                    if (cfg == 0)
                    {
#ifdef BCM6858_A0
                        /* BCM6858_A0 workaround, if a key index is the same for two or more engines, skip the entry at depth 0 */
                        if (depth == 0)
                        {
                            uint16_t same_hash_idx = IS_SAME_HASH_IDX(_get_hash_table_index(eng_hash[0], depth, tbl_idx)
                                    , _get_hash_table_index(eng_hash[1], depth, tbl_idx)
                                    , _get_hash_table_index(eng_hash[2], depth, tbl_idx)
                                    , _get_hash_table_index(eng_hash[3], depth, tbl_idx));

                            if (same_hash_idx != HASH_INVALID_IDX)
                            {
                                hash_res->duplicate_idx = hash_res->first_free_idx;
                                hash_res->first_free_idx = HASH_INVALID_IDX;
                                continue;
                            }
                        }
#endif

                        hash_res->match = HASH_MISS;
                        return rc;
                    }
                }
                else if (!skp)
                {
                    /* otherwise - if key matches entry key return match in hash table */
                    entry_key = CONSTRUCT_KEY(key_59_28, key_27_12, key_11_0) & mask;
                    entry_key |= padding;
                    if (entry_key == key)
                    {
                        hash_res->match = HASH_MATCH_IN_TABLE;
                        hash_res->match_index = GET_ENTRY_IDX_PER_ENGINE(en_idx, eng);
                        hash_res->context_addr = GET_CONTEXT_INDEX(g_hash_cfg.ctx_cfg[tbl_idx].base_address, hash_res->match_index,
                                en_idx, g_hash_cfg.tbl_cfg[tbl_idx].hash_base_addr, g_hash_cfg.ctx_cfg[tbl_idx].ext_ctx_size);
                        return rc;
                    }
                }
            }
        }
    }
    /* If not found - Try searching CAM */
    if (g_hash_cfg.tbl_cfg[tbl_idx].cam_en)
        rc = rc ? rc : _drv_hash_cam_search(key_p, hash_res);
    hash_res->match = hash_res->match ? hash_res->match : HASH_MISS;
    return rc;
}

bdmf_error_t _drv_hash_write_external_context(uint16_t ctx_idx, uint8_t ctx_chunks_num, uint8_t *ctx)
{
    uint8_t i, j;
    uint32_t ctx_chunk = 0;
    bdmf_error_t rc;

    for (i = 0; i < ctx_chunks_num; i++)
    {
        /* cntxt_size is actually the number of 3B chunks of context */
        ctx_chunk = 0;
        for (j = 0; j < CONTEXT_ENTRY_MIN_SIZE; j++)
        {
            ctx_chunk <<= 8;
            ctx_chunk |= ctx[j];
        }
        rc = ag_drv_hash_context_ram_context_3b_data_set(ctx_idx + i, ctx_chunk);
        if (rc)
            return rc;
    }
    return 0;
}

bdmf_error_t _drv_hash_write_internal_context_to_key(uint8_t table_id, uint8_t *int_ctx, uint16_t *key_27_12,
    uint32_t *key_59_28)
{
    uint8_t i, tbl_idx, ctx_size;
    uint64_t mask, key_copy, internal_context = 0;
    uint8_t *aligned_int_ctx;
    tbl_idx = table_id - 1;
    ctx_size = g_hash_cfg.tbl_init[tbl_idx].int_ctx_size;
    mask = UNITE_HIGH_LOW(g_hash_cfg.mask[tbl_idx].maskh, g_hash_cfg.mask[tbl_idx].maskl);
    key_copy = CONSTRUCT_KEY(*key_59_28, *key_27_12, 0);
    key_copy &= mask;

    aligned_int_ctx = bdmf_alloc(ALIGN_CTX_SIZE_TO_WORD(ctx_size));
    if (!aligned_int_ctx)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to allocate aligned ctx size");
    _drv_hash_align_and_swap(int_ctx, aligned_int_ctx, ctx_size);

    for (i = 0; i < ctx_size; i++)
        internal_context |= ((uint64_t)aligned_int_ctx[i]) << (60 - ((i+1) * 8));

    internal_context &= ~mask;
    key_copy |= internal_context;
    key_copy >>= 12;
    *key_27_12 = (uint16_t)(key_copy & 0xFFFF);
    key_copy >>= 16;
    *key_59_28 = (uint32_t)key_copy;
    bdmf_free(aligned_int_ctx);
    return BDMF_ERR_OK;
}

bdmf_error_t _drv_hash_write_key_and_internal_context(uint8_t table_id, uint16_t idx, uint8_t *key_p, uint8_t *int_ctx)
{
    bdmf_error_t rc;
    bdmf_boolean skp;
    uint8_t  cfg, tbl_idx;
    uint16_t key_11_0, key_27_12;
    uint32_t key_59_28;
    uint64_t key = 0;
    uint8_t swapped_key_p[8];

    if (VERIFY_TABLE_ID(table_id))
        return BDMF_ERR_PARM;
    tbl_idx = GET_TABLE_ID(table_id);

    /* verify the entry is empty */
    rc = ag_drv_hash_ram_eng_low_get(idx, &skp, &cfg, &key_11_0, &key_27_12);
 
    /* write_internal_context */
    if (cfg && !skp && !rc)
        return BDMF_ERR_ALREADY;
    if (rc)
        return rc;
    _drv_hash_copy_swap_key(key_p, swapped_key_p, 8);
    key = _uint8_arr_to_uint64(swapped_key_p, 8);
    key_11_0 = (uint16_t)key & 0xFFF;
    key_27_12 = (uint16_t)(key >> 12) & 0xFFFF;
    key_59_28 = (uint32_t)(key >> 28);

    if (g_hash_cfg.tbl_cfg[tbl_idx].int_ctx_size)
        rc = _drv_hash_write_internal_context_to_key(table_id, int_ctx, &key_27_12, &key_59_28);

    /* write key + internal context to SRAM */
    rc = rc ? rc : ag_drv_hash_ram_eng_low_set(idx, 0, table_id, key_11_0, key_27_12);
    rc = rc ? rc : ag_drv_hash_ram_eng_high_set(idx, key_59_28);

    return rc;
}

bdmf_error_t drv_hash_rule_insert(uint32_t table_id, uint16_t idx, uint8_t *key_p, uint8_t *ext_ctx, uint8_t *int_ctx)
{
    uint8_t tbl_idx;
    uint16_t ctx_index;
    bdmf_error_t rc;

    if (VERIFY_TABLE_ID(table_id))
        return BDMF_ERR_PARM;
    tbl_idx = GET_TABLE_ID(table_id);

    /* reset aging */
    rc = ag_drv_hash_aging_ram_aging_set(idx, 0);

    /* write external context */
    if (g_hash_cfg.ctx_cfg[tbl_idx].ext_ctx_size)
    {
        ctx_index = GET_CONTEXT_INDEX(g_hash_cfg.ctx_cfg[tbl_idx].base_address, idx, GET_ENG_FROM_INDEX(idx),
                    g_hash_cfg.tbl_cfg[tbl_idx].hash_base_addr, g_hash_cfg.ctx_cfg[tbl_idx].ext_ctx_size);
        rc = rc ? rc : _drv_hash_write_external_context(ctx_index, g_hash_cfg.ctx_cfg[tbl_idx].ext_ctx_size, ext_ctx);
    }

    /* write key with internal context (if exists) */
    rc = rc ? rc : _drv_hash_write_key_and_internal_context(table_id, idx, key_p, int_ctx);
    return rc;
}

bdmf_error_t _drv_hash_cam_insert(uint8_t *key_p, uint64_t *mask, uint8_t *int_ctx, uint8_t *ext_ctx, uint8_t ext_ctx_size, uint8_t int_ctx_size)
{
#if !defined(_CFE_) && !defined(RDP_SIM)
    uint64_t key;
    uint32_t ctx_idx;
    uint8_t *united_context, cam_idx = 0;
    bdmf_boolean valid = 1;
    bdmf_error_t rc = BDMF_ERR_OK;

    while (cam_idx < 64 && valid) /* searching CAM for an invalid (=free) entry */
    {
        rc = rc ? rc : ag_drv_hash_cam_indirect_addr_set(0, cam_idx);
        rc = rc ? rc : ag_drv_hash_cam_indirect_op_set(CAM_READ_CMD);
        rc = rc ? rc : _drv_hash_cam_wait_indirect();
        rc = rc ? rc : ag_drv_hash_cam_indirect_vlid_out_get(&valid);
    }
    if (cam_idx >= 64)
        return BDMF_ERR_NOMEM;

    /* writing key1 part (the key itself) */
    key = _uint8_arr_to_uint64(key_p, 8);
    rc = rc ? rc : ag_drv_hash_cam_indirect_addr_set(1, cam_idx);
    rc = rc ? rc : ag_drv_hash_cam_indirect_key_in_set(0, (hash_cam_indirect_key_in *)&key);
    rc = rc ? rc : ag_drv_hash_cam_indirect_op_set(CAM_WRITE_CMD);
    rc = rc ? rc : _drv_hash_cam_wait_indirect();

    /* writing key0 part (the mask) */
    rc = rc ? rc : ag_drv_hash_cam_indirect_addr_set(0, cam_idx);
    rc = rc ? rc : ag_drv_hash_cam_indirect_key_in_set(0, (hash_cam_indirect_key_in *)mask);
    rc = rc ? rc : ag_drv_hash_cam_indirect_op_set(CAM_WRITE_CMD);
    rc = rc ? rc : _drv_hash_cam_wait_indirect();

    /* setting valid bit */
    rc = rc ? rc : ag_drv_hash_cam_indirect_vlid_in_set(1);
    rc = rc ? rc : ag_drv_hash_cam_indirect_op_set(CAM_VALID_SET_CMD);
    rc = rc ? rc : _drv_hash_cam_wait_indirect();

    /* write context */
    ctx_idx = g_hash_cfg.cam_base_addr + cam_idx*6; /* CAM context size is always 18B */
    united_context = bdmf_alloc(int_ctx_size + ext_ctx_size);
    if (united_context == NULL)
        return BDMF_ERR_INTERNAL;
    memcpy(ext_ctx, united_context, ext_ctx_size);
    united_context += ext_ctx_size;
    memcpy(int_ctx, united_context, int_ctx_size);
    rc = rc ? rc : _drv_hash_write_external_context(ctx_idx, int_ctx_size + ext_ctx_size, united_context);
    bdmf_free(united_context);
    return rc;
#else
    return BDMF_ERR_INTERNAL;
#endif
}

bdmf_error_t drv_hash_rule_add(uint32_t table_id, uint8_t *key_p, uint8_t *ext_ctx, uint8_t *int_ctx)
{
    bdmf_error_t rc;
    uint8_t tbl_idx;
    uint64_t mask;
    hash_result_t hash_res = {};

    if (VERIFY_TABLE_ID(table_id))
        BDMF_TRACE_RET(BDMF_ERR_PARM, "wrong table id: %d\n", table_id);
    tbl_idx = GET_TABLE_ID(table_id);
    rc = drv_hash_find(table_id, key_p, &hash_res);
    if (rc)
        return rc;
    if (hash_res.match != HASH_MISS)
        return BDMF_ERR_ALREADY;
    if (hash_res.duplicate_idx != HASH_INVALID_IDX)
    {
        drv_hash_rule_remove_index(hash_res.duplicate_idx);
    }
    if (hash_res.first_free_idx == HASH_INVALID_IDX) /* this means no free place in hash table was found */
    {
        if (g_hash_cfg.tbl_cfg[tbl_idx].cam_en)
        {
            mask = UNITE_HIGH_LOW(g_hash_cfg.mask[tbl_idx].maskh, g_hash_cfg.mask[tbl_idx].maskl);
            return _drv_hash_cam_insert(key_p, &mask, ext_ctx, int_ctx,
                    g_hash_cfg.tbl_init[tbl_idx].ext_ctx_size, g_hash_cfg.tbl_init[tbl_idx].int_ctx_size);
        }
        else
            return BDMF_ERR_NOMEM;
    }
    return drv_hash_rule_insert(table_id, hash_res.first_free_idx, key_p, ext_ctx, int_ctx);
}

bdmf_error_t drv_hash_rule_remove_key(uint32_t table_id, uint8_t *key_p)
{
    uint16_t idx;
    hash_result_t hash_res;
    bdmf_error_t rc;

    rc = drv_hash_find(table_id, key_p, &hash_res);
    if (!rc && hash_res.match == HASH_MISS)
            return BDMF_ERR_ALREADY;

    idx = hash_res.match_index;
    rc = rc ? rc : ag_drv_hash_ram_eng_low_set(idx, 1, table_id, 0, 0);
    rc = rc ? rc : ag_drv_hash_ram_eng_high_set(idx, 0);

    return rc;
}

bdmf_error_t drv_hash_rule_remove_index(uint16_t idx)
{
    bdmf_error_t rc;

    /* skp is set to 1 in case there are additional entries in hash with depth greater than the current
     * cfg is set to 0 in order to invalidate the entry
     */
    rc = ag_drv_hash_ram_eng_low_set(idx, 1, 0, 0, 0);
    rc = rc ? rc : ag_drv_hash_ram_eng_high_set(idx, 1);

    return rc;
}

bdmf_error_t drv_hash_set_aging(uint16_t idx, bdmf_boolean *prev_age)
{
    bdmf_error_t rc;
    rc = ag_drv_hash_aging_ram_aging_get(idx, prev_age);
    rc = rc ? rc : ag_drv_hash_aging_ram_aging_set(idx, 1);

    return rc;
}

bdmf_error_t drv_hash_modify_context(uint8_t table_id, uint16_t idx, uint8_t *ext_ctx, uint8_t *int_ctx)
{
    uint32_t key_59_28;
    uint16_t ctx_idx, key_27_12, key_11_0;
    uint8_t cfg, int_ctx_size, ext_ctx_size, tbl_idx;
    bdmf_boolean skp;
    bdmf_error_t rc = BDMF_ERR_OK;

    if (VERIFY_TABLE_ID(table_id))
        return BDMF_ERR_PARM;
    tbl_idx = GET_TABLE_ID(table_id);

    int_ctx_size = g_hash_cfg.tbl_init[tbl_idx].int_ctx_size;
    ext_ctx_size = g_hash_cfg.ctx_cfg[tbl_idx].ext_ctx_size;

    if (int_ctx_size)
    {
        rc = ag_drv_hash_ram_eng_low_get(idx, &skp, &cfg, &key_11_0, &key_27_12);
        rc = rc ? rc : ag_drv_hash_ram_eng_high_get(idx, &key_59_28);
        if (rc)
            return rc;
        _drv_hash_write_internal_context_to_key(table_id, int_ctx, &key_27_12, &key_59_28);
        rc = ag_drv_hash_ram_eng_low_set(idx, skp, cfg, key_11_0, key_27_12);
        rc = rc ? rc : ag_drv_hash_ram_eng_high_set(idx, key_59_28);
        if (rc)
            return rc;
    }
    if (ext_ctx_size)
    {
        ctx_idx = GET_CONTEXT_INDEX(g_hash_cfg.ctx_cfg[tbl_idx].base_address, idx, GET_ENG_FROM_INDEX(idx),
                g_hash_cfg.tbl_cfg[tbl_idx].hash_base_addr, ext_ctx_size);
        rc = _drv_hash_write_external_context(ctx_idx, ext_ctx_size, ext_ctx);
    }
    return rc;
}

bdmf_error_t drv_hash_get_context(uint16_t idx, uint8_t table_id, uint8_t *ext_ctx, uint8_t *int_ctx)
{
    /*
     * Does NOT support partial internal context hack (needs to be managed by the layer above)
     * context needs to be allocated the right size when given as argument - use drv_hash_ctx_alloc
     */
    uint8_t cfg, tbl_idx, ctx_size, aligned_ctx_size;
    uint8_t *aligned_int_ctx;
    uint16_t ctx_idx, i, key_11_0, key_27_12;
    uint32_t ctx_chunk, key_59_28;
    bdmf_boolean skp;
    uint64_t mask, internal_context = 0;
    bdmf_error_t rc = BDMF_ERR_OK;

    if (VERIFY_TABLE_ID(table_id))
        return BDMF_ERR_PARM;
    tbl_idx = GET_TABLE_ID(table_id);

    ctx_idx = GET_CONTEXT_INDEX(g_hash_cfg.ctx_cfg[tbl_idx].base_address, idx, GET_ENG_FROM_INDEX(idx),
            g_hash_cfg.tbl_cfg[tbl_idx].hash_base_addr, g_hash_cfg.ctx_cfg[tbl_idx].ext_ctx_size);
    /* get external context */
    for (i = 0; i < g_hash_cfg.ctx_cfg[tbl_idx].ext_ctx_size; i++)
    {
        rc = ag_drv_hash_context_ram_context_3b_data_get(ctx_idx + i, &ctx_chunk);
        if (rc)
            return rc;
        _copy_byte_by_byte(ext_ctx, ctx_chunk, CONTEXT_ENTRY_MIN_SIZE);
        ext_ctx += CONTEXT_ENTRY_MIN_SIZE;
    }

    /* get internal context */
    rc = ag_drv_hash_ram_eng_low_get(idx, &skp, &cfg, &key_11_0, &key_27_12);
    rc = rc ? rc : ag_drv_hash_ram_eng_high_get(idx, &key_59_28);
    if (rc)
        return rc;
    ctx_size = g_hash_cfg.tbl_init[tbl_idx].int_ctx_size;
    aligned_ctx_size = ALIGN_CTX_SIZE_TO_WORD(ctx_size);
    aligned_int_ctx = bdmf_alloc(aligned_ctx_size);
    if (!aligned_int_ctx)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to allocate aligned ctx size");
    mask = UNITE_HIGH_LOW(g_hash_cfg.mask[tbl_idx].maskh, g_hash_cfg.mask[tbl_idx].maskl);
    internal_context = CONSTRUCT_KEY(key_59_28, key_27_12, 0);
    internal_context &= ~mask;
    internal_context >>= (60 - (ctx_size  * 8));
    memset(aligned_int_ctx, 0, aligned_ctx_size);
    _copy_byte_by_byte(aligned_int_ctx, internal_context, ctx_size);
    _drv_hash_unalign_and_swap(int_ctx, aligned_int_ctx, ctx_size);
    bdmf_free(aligned_int_ctx);
    return rc;
}

bdmf_error_t drv_hash_key_get(uint8_t table_id, uint16_t idx , uint8_t *key)
{
    bdmf_error_t rc;
    bdmf_boolean skp;
    uint8_t cfg, tbl_idx;
    uint16_t key_11_0, key_27_12;
    uint32_t key_59_28;
    uint64_t mask, key64 = 0;
    uint8_t read_key_p[8] = {};

    if (VERIFY_TABLE_ID(table_id))
        return BDMF_ERR_PARM;
    /* A0 workaround for hash hw bug */
    if (GET_ENG_FROM_INDEX(idx) >= HASH_NUM_OF_EFFECTIVE_ENGINES)
        return BDMF_ERR_NOENT;
    tbl_idx = GET_TABLE_ID(table_id);

    rc = ag_drv_hash_ram_eng_low_get(idx, &skp, &cfg, &key_11_0, &key_27_12);
    if (!cfg || skp)
        return BDMF_ERR_NOENT;
    rc = rc ? rc : ag_drv_hash_ram_eng_high_get(idx, &key_59_28);
    if (rc)
        return rc;
    mask = UNITE_HIGH_LOW(g_hash_cfg.mask[tbl_idx].maskh, g_hash_cfg.mask[tbl_idx].maskl);
    key64 = CONSTRUCT_KEY(key_59_28, key_27_12, key_11_0) & mask;
    _copy_byte_by_byte(read_key_p, key64, 8);
    _drv_hash_copy_swap_key(read_key_p, key, 8);
    return 0;
}

/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/

#ifdef USE_BDMF_SHELL

static int drv_hash_cli_find(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    hash_result_t hash_res = {};
    RDD_IPTV_HASH_LKP_ENTRY_DTS hash_key = {};
    bdmf_error_t rc;
    uint32_t table_id = parm[parameter_index++].value.unumber; /* input */

    hash_key.addr_high = parm[parameter_index++].value.unumber; /* input */
    hash_key.dst_ip_or_mac_low = parm[parameter_index++].value.unumber; /* input */

    rc =  drv_hash_find(table_id, (uint8_t *)&hash_key, &hash_res);
    if (rc)
        return rc;
    if (hash_res.match)
    {
        bdmf_session_print(session, "match index     : %d\n\r", hash_res.match_index);
        bdmf_session_print(session, "context address : 0x%x\n\r", hash_res.context_addr);
        bdmf_session_print(session, "First free idx  : %d\n\r", hash_res.first_free_idx);
    }
    else
        bdmf_session_print(session, "No match !!!\n\r");

    return 0;
}

static int drv_hash_cli_add(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    RDD_IPTV_HASH_LKP_ENTRY_DTS hash_key = {};
    RDD_IPTV_HASH_RESULT_ENTRY_DTS hash_ctx = {}; 
    bdmf_error_t rc;
    uint32_t table_id = parm[parameter_index++].value.unumber; /* input */

    hash_key.addr_high = parm[parameter_index++].value.unumber; /* input */
    hash_key.dst_ip_or_mac_low = parm[parameter_index++].value.unumber; /* input */
    hash_ctx.ctx_idx = parm[parameter_index++].value.unumber; /* input */

    rc = drv_hash_rule_add(table_id, (uint8_t *)&hash_key, NULL, (uint8_t *)&hash_ctx);
    if (rc)
    {
        bdmf_session_print(session, "Can't add entry. rc : %d\n\r", rc);
        return rc;
    }
    else
        bdmf_session_print(session, "Entry added\n\r");

    return 0;
}
 
static int drv_hash_cli_remove_key(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    RDD_IPTV_HASH_LKP_ENTRY_DTS hash_key = {};
    bdmf_error_t rc;
    uint32_t table_id = parm[parameter_index++].value.unumber; /* input */

    hash_key.addr_high = parm[parameter_index++].value.unumber; /* input */
    hash_key.dst_ip_or_mac_low = parm[parameter_index++].value.unumber; /* input */

    rc = drv_hash_rule_remove_key(table_id, (uint8_t *)&hash_key);
    if (rc)
    {
        bdmf_session_print(session, "Can't remove entry. rc : %d\n\r", rc);
        return rc;
    }
    else
        bdmf_session_print(session, "Entry removed\n\r");

    return 0;
}

static int drv_hash_cli_get_context(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    RDD_IPTV_HASH_RESULT_ENTRY_DTS int_ctx = {}; 
    RDD_IPTV_HASH_RESULT_ENTRY_DTS ext_ctx = {}; 
    bdmf_error_t rc;
    uint32_t table_id = parm[parameter_index++].value.unumber; /* input */
    uint32_t entry_idx = parm[parameter_index++].value.unumber; /* input */

    rc = drv_hash_get_context(entry_idx, table_id, (uint8_t *)&ext_ctx, (uint8_t *)&int_ctx);
    if (rc)
    {
        bdmf_session_print(session, "Can't get entry context. rc : %d\n\r", rc);
        return rc;
    }
    else
    {
        bdmf_session_print(session, "Entry internal context: %d\n\r", int_ctx.ctx_idx);
        bdmf_session_print(session, "Entry external context: %d\n\r", ext_ctx.ctx_idx);
    }

    return 0;
}

static int drv_hash_cli_get_key(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    RDD_IPTV_HASH_LKP_ENTRY_DTS hash_key = {};
    bdmf_error_t rc;
    uint32_t table_id = parm[parameter_index++].value.unumber; /* input */
    uint32_t entry_idx = parm[parameter_index++].value.unumber; /* input */

    rc = drv_hash_key_get(table_id, entry_idx , (uint8_t *)&hash_key);
    if (rc)
    {
        bdmf_session_print(session, "Can't get entry key. rc : %d\n\r", rc);
        return rc;
    }
    else
    {
        bdmf_session_print(session, "Key entry: %d\n\r", hash_key.dst_ip_or_mac_low);
    }

    return 0;
}

static bdmfmon_handle_t hash_dir;

void drv_hash_cli_init(bdmfmon_handle_t driver_dir)
{
    hash_dir = ag_drv_hash_cli_init(driver_dir);

    BDMFMON_MAKE_CMD(hash_dir, "hash_find", "find entry in hash", drv_hash_cli_find,
        BDMFMON_MAKE_PARM("table", "table id", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_RANGE("key_high", "hash key high (4 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffffff),
        BDMFMON_MAKE_PARM_RANGE("key_low", "hash key low (4 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffffff));

    BDMFMON_MAKE_CMD(hash_dir, "hash_add", "add entry in hash", drv_hash_cli_add,
        BDMFMON_MAKE_PARM("table", "table id", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_RANGE("key_high", "hash key high (4 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffffff),
        BDMFMON_MAKE_PARM_RANGE("key_low", "hash key low (4 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffffff),
        BDMFMON_MAKE_PARM_RANGE("ctx", "ctx (10 bits)", BDMFMON_PARM_HEX, 0, 0, 0x3ff));

    BDMFMON_MAKE_CMD(hash_dir, "hash_remove_key", "remove entry in hash using key", drv_hash_cli_remove_key,
        BDMFMON_MAKE_PARM("table", "table id", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_RANGE("key_high", "hash key high (4 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffffff),
        BDMFMON_MAKE_PARM_RANGE("key_low", "hash key low (4 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffffff));

    BDMFMON_MAKE_CMD(hash_dir, "hash_get_context", "get entry context", drv_hash_cli_get_context,
        BDMFMON_MAKE_PARM("table", "table id", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("index", "entry index", BDMFMON_PARM_NUMBER, 0));

    BDMFMON_MAKE_CMD(hash_dir, "hash_get_key", "get entry key", drv_hash_cli_get_key,
        BDMFMON_MAKE_PARM("table", "table id", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("index", "entry index", BDMFMON_PARM_NUMBER, 0));
}

void drv_hash_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (hash_dir)
    {
        bdmfmon_token_destroy(hash_dir);
        hash_dir = NULL;
    }
}
#endif
