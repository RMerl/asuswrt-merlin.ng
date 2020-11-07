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

#define ALIGN_CTX_MAX_BUF_SIZE 32 /* Buffer for internal usage to make alighnments, should be bigger then max context
                                     size in both HASH and CAM */

typedef struct {
    int entries_in_hash;
    int entries_in_cam;
    int first_lkp_miss;
} hash_tbl_mgmt_t;

hash_config_t g_hash_cfg;
hash_tbl_mgmt_t hash_mgmt[8] = {};

/***************************/
/* Common helper functions */
/***************************/
static inline uint16_t _get_hash_table_index(uint16_t crc, uint16_t depth, uint8_t tbl_sw_idx)
{
    uint16_t tbl_size = GET_TBL_SIZE(g_hash_cfg.tbl_cfg[tbl_sw_idx].tbl_size);
    return ((crc + depth) % tbl_size) + g_hash_cfg.tbl_cfg[tbl_sw_idx].hash_base_addr;
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

static void _copy_byte_by_byte(uint8_t *dst, uint64_t src, uint8_t size)
{
    uint32_t i;

    for (i = 0; i < size; i++)
    {
        dst[size - i - 1] = src & 0xFF;
        src >>= 8;
    }
}

static void swap_words(uint8_t *buf, uint8_t size)
{
    int i;
    uint8_t tmp_buf[ALIGN_CTX_MAX_BUF_SIZE] = {}, *buf_p = buf;

    for (i = size / 4 - 1; i >= 0; i--, buf_p +=4)
        *((uint32_t *)tmp_buf + i) = *((uint32_t *)buf_p);

    memcpy(buf, tmp_buf, size);
}

/* This function regulates context to be word aligned and swaps BE to LE.                           
 * Resulted context will suit the hash context in resolution of 3 bytes, suffix will be zeroed. */
static void _drv_hash_align_and_swap(uint8_t *unaligned_ctx, uint8_t *aligned_ctx, uint8_t unaligned_size)
{
    uint8_t aligned_size = ALIGN_CTX_SIZE_TO_WORD(unaligned_size);
    uint8_t tmp_buf[ALIGN_CTX_MAX_BUF_SIZE] = {};

    /* First pad the context and swap bytes in each word. Example, for 6 bytes context: */
    /*   {0xaa 0xbb 0xcc 0xdd 0xee 0xff} -> {0xaa 0xbb 0xcc 0xdd 0xee 0xff 0x00 0x00} -> {0xdd 0xcc 0xbb 0xaa 0x00 0x00 0xff 0xee}  */
    memcpy(tmp_buf, unaligned_ctx, unaligned_size);
    SWAPBYTES(tmp_buf, aligned_size);

    /* Swap the words and get rid of zero prefix. */
    /*   {0xdd 0xcc 0xbb 0xaa 0x00 0x00 0xff 0xee} -> {0xff 0xee 0xdd 0xcc 0xbb 0xaa 0x00 0x00} */
    swap_words(tmp_buf, aligned_size);
    memcpy(aligned_ctx, tmp_buf + aligned_size - unaligned_size, unaligned_size);
}

/* This function does the opposize openration of _drv_hash_align_and_swap.
 * Result will suit stored hash context in resolution of 3 bytes. */
static void _drv_hash_unalign_and_swap(uint8_t *unaligned_ctx, uint8_t *aligned_ctx, uint8_t unaligned_size)
{
    uint8_t aligned_size = ALIGN_CTX_SIZE_TO_WORD(unaligned_size);

    /* Swap words. Example, for 6 bytes context: */
    /*   {0xff 0xee 0xdd 0xcc 0xbb 0xaa 0x00 0x00} -> {0xbb 0xaa 0x00 0x00 0xff 0xee 0xdd 0xcc} */
    swap_words(aligned_ctx, aligned_size);

    /* Swap bytes and get rid of prefix: */
    /*   {0xbb 0xaa 0x00 0x00 0xff 0xee 0xdd 0xcc} -> {0x00 0x00 0xaa 0xbb 0xcc 0xdd 0xee 0xff} -> {0xaa 0xbb 0xcc 0xdd 0xee 0xff} */
    SWAPBYTES(aligned_ctx, aligned_size);
    memcpy(unaligned_ctx, aligned_ctx + aligned_size - unaligned_size, unaligned_size);
}

static void _drv_hash_copy_swap_key(uint8_t *key, uint8_t *swapped_key, uint8_t key_size)
{
    memcpy(swapped_key, key, key_size);
    SWAPBYTES(swapped_key, key_size);
}

static uint64_t _drv_hash_get_key_padding(uint64_t mask)
{
    uint64_t padding;
    uint32_t pad_l, pad_h;

    ag_drv_hash_key_padding_get(&pad_h, &pad_l);
    padding = UNITE_HIGH_LOW_60(pad_h, pad_l);
    padding &= ~(mask);
    return padding;
}

static bdmf_error_t _drv_hash_write_ram_context(uint16_t ctx_idx, uint8_t ctx_chunks_num, uint8_t *ctx)
{
    uint8_t i, j;
    uint32_t ctx_chunk = 0;
    int rc;
    uint8_t aligned_ctx[CAM_CTX_SIZE] = {}; /* Context size in CAM is the max size of all possible contexts */

    RDD_BTRACE("Writing context to HASH RAM, ctx_idx %d, ctx_chunks_num %d\n", ctx_idx, ctx_chunks_num);
    RDD_BTRACE_BUF("Context", ctx, GET_CONTEXT_BYTE_SIZE(ctx_chunks_num));
    _drv_hash_align_and_swap(ctx, aligned_ctx, GET_CONTEXT_BYTE_SIZE(ctx_chunks_num));

    for (i = 0; i < ctx_chunks_num; i++)
    {
        /* cntxt_size is actually the number of 3B chunks of context */
        ctx_chunk = 0;
        for (j = 0; j < CONTEXT_ENTRY_MIN_SIZE; j++)
        {
            ctx_chunk <<= 8;
            ctx_chunk |= aligned_ctx[j+i*3];
        }

        if (((ctx_idx + i) % 2) == 0)
        {
            RDD_BTRACE("inserting external context 47_24 at context index 0x%04x, "
                "external context memory offset is 0x%05x, value 0x%x\n",
                (uint16_t)((ctx_idx + i) / 2), (uint16_t)((ctx_idx + i) << 2), ctx_chunk);
            rc = ag_drv_hash_context_ram_context_47_24_set((ctx_idx + i) / 2, ctx_chunk);
        }
        else
        {
            RDD_BTRACE("inserting external context 23_0 at context index 0x%04x, "
                "external context memory offset is 0x%05x, value 0x%x\n", 
                (uint16_t)((ctx_idx + i - 1) / 2), (uint16_t)((ctx_idx + i - 1) << 2), ctx_chunk);
            rc = ag_drv_hash_context_ram_context_23_0_set((ctx_idx + i - 1) / 2, ctx_chunk);
        }

        if (rc)
            return rc;
    }
    return 0;
}

static bdmf_error_t _drv_hash_read_ram_context(uint16_t ctx_idx, uint8_t ctx_chunks_num, uint8_t *ctx)
{
    uint8_t i;
    uint32_t ctx_chunk = 0;
    int rc;
    uint8_t aligned_ctx[CAM_CTX_SIZE] = {}, *_ctx;

    RDD_BTRACE("Reading context from HASH RAM, ctx_idx %d, ctx_chunks_num %d\n", ctx_idx, ctx_chunks_num);
    _ctx = aligned_ctx;
    for (i = 0; i < ctx_chunks_num; i++)
    {
        if (((ctx_idx + i) % 2) == 0)
            rc = ag_drv_hash_context_ram_context_47_24_get((ctx_idx + i) / 2, &ctx_chunk);
        else
            rc = ag_drv_hash_context_ram_context_23_0_get((ctx_idx + i - 1) / 2, &ctx_chunk);

        if (rc)
            return rc;
        _copy_byte_by_byte(_ctx, ctx_chunk, CONTEXT_ENTRY_MIN_SIZE);
        _ctx += CONTEXT_ENTRY_MIN_SIZE;
    }
    _drv_hash_unalign_and_swap(ctx, aligned_ctx, GET_CONTEXT_BYTE_SIZE(ctx_chunks_num));

    RDD_BTRACE_BUF("Context", ctx, GET_CONTEXT_BYTE_SIZE(ctx_chunks_num));

    return 0;
}

static inline uint64_t _drv_hash_get_table_mask(uint8_t table_id)
{
    uint8_t tbl_sw_idx = GET_SW_TABLE_ID(table_id);
    uint64_t mask;

    mask = UNITE_HIGH_LOW(g_hash_cfg.mask[tbl_sw_idx].maskh, g_hash_cfg.mask[tbl_sw_idx].maskl);
    RDD_BTRACE("Mask for table_id %d = 0x%llx\n", table_id, (long long unsigned int)mask);
    return mask;
}

static uint64_t _drv_hash_prepare_key_from_buf(uint8_t table_id, uint8_t *key_p)
{
    uint64_t key, mask;
    uint8_t swapped_key_p[8] = {};

    mask = _drv_hash_get_table_mask(table_id);

    _drv_hash_copy_swap_key(key_p, swapped_key_p, 8);
    key = _uint8_arr_to_uint64(swapped_key_p, 8);
    key &= mask;

    RDD_BTRACE("KEY From buf 0x%llx, mask 0x%llx\n", (long long unsigned int)key, (long long unsigned int)mask);
    RDD_BTRACE_BUF("Key_p", key_p, 8);

    return key;
}

/*******/
/* CAM */
/*******/
static uint32_t cam_context_base_addr_calc(hash_config_t *cfg)
{
    int i;
    uint32_t cam_base_addr = 0, table_offset;

    for (i = 0; i < cfg->tbl_num; i++)
    {
        /* Internal context is part of the key, so for CAM base calculation should take only external context size in
         * accound */
        table_offset = cfg->tbl_init[i].ext_ctx_size * GET_TBL_SIZE(cfg->tbl_cfg[i].tbl_size) * HASH_NUM_OF_ENGINES;
        RDD_BTRACE("Table %d, ext_ctx_size = %d, tbl_size = %d, num_of_engines = %d, table_offset %d (%d/6)\n", i,
            cfg->tbl_init[i].ext_ctx_size,
            GET_TBL_SIZE(cfg->tbl_cfg[i].tbl_size), HASH_NUM_OF_ENGINES, table_offset / 6, table_offset);
        table_offset /= 6;
        cam_base_addr += table_offset;
    }
    RDD_BTRACE("CAM Base context address = 0x%x (%d)\n", cam_base_addr, cam_base_addr);
    return cam_base_addr;
}

#ifndef _CFE_
static bdmf_error_t _drv_hash_cam_wait_indirect(void)
{
    int time_out = POLLING_TIME_OUT;
    bdmf_boolean is_done = 0;

    for (time_out = POLLING_TIME_OUT; time_out > 0; time_out--)
    {
        ag_drv_hash_cam_indirect_op_done_get(&is_done);
        if (is_done)
            break;
    }
    if (!is_done)
        return BDMF_ERR_INTERNAL;
    return 0;
}
#endif

uint64_t _drv_hash_encode_cam_key(uint8_t table_id, uint8_t *key_p, uint64_t *encode_mask)
{
    uint64_t _key, key;

    _key = _drv_hash_prepare_key_from_buf(table_id, key_p);
    _key = _key << 3 | (table_id & 0x7);
    key = (_key << 32) | (_key >> 32);
    RDD_BTRACE("Encode key (after swap words): 0x%llx\n", (long long unsigned int)key);

    if (encode_mask)
    {
        uint64_t mask = _drv_hash_get_table_mask(table_id);

        mask = (mask << 3) | 0x7;
        *encode_mask = mask & ~_key;
        *encode_mask = (*encode_mask << 32) | (*encode_mask >> 32);
        RDD_BTRACE("Encode mask (after swap words): 0x%llx\n", (long long unsigned int)*encode_mask);
    }

    return key;
}

void _drv_hash_decode_cam_key(uint64_t encode_key, uint8_t *table_id, uint8_t *key_p)
{
    uint64_t key, mask;
    uint8_t swapped_key_p[8] = {}, _table_id;

    key = (encode_key << 32) | (encode_key >> 32);
    _table_id = key & 0x7;
    if (table_id)
        *table_id = _table_id;
    key >>= 3; /* Exclude Table ID */

    /* Un-pad key */
    mask = _drv_hash_get_table_mask(_table_id);
    key &= ~_drv_hash_get_key_padding(mask);

    _copy_byte_by_byte(swapped_key_p, key, 8);
    _drv_hash_copy_swap_key(swapped_key_p, key_p, 8);
}

static void _drv_hash_cam_context_compose(uint8_t table_id, uint8_t *ext_ctx, uint8_t *int_ctx, uint8_t *cam_ctx)
{
    uint8_t tbl_sw_idx;
    int int_ctx_size, ext_ctx_size;

    tbl_sw_idx = GET_SW_TABLE_ID(table_id);
    int_ctx_size = GET_CONTEXT_BYTE_SIZE(g_hash_cfg.tbl_cfg[tbl_sw_idx].int_ctx_size);
    ext_ctx_size = GET_CONTEXT_BYTE_SIZE(g_hash_cfg.ctx_cfg[tbl_sw_idx].ext_ctx_size);

    /* Prepare single buffer with external and internal context. In CAM, first comes external, then internal. However,
     * since we swap when we write to RAM, first we push internal, and then external. */
    if (int_ctx_size)
        memcpy(cam_ctx, int_ctx, int_ctx_size);
    if (ext_ctx_size)
        memcpy(cam_ctx + int_ctx_size, ext_ctx, ext_ctx_size);
    RDD_BTRACE_BUF("Composed CAM context", cam_ctx, CAM_CTX_SIZE);
}

static void _drv_hash_cam_context_decompose(uint8_t table_id, uint8_t *cam_ctx, uint8_t *ext_ctx, uint8_t *int_ctx)
{
    uint8_t tbl_sw_idx;
    int int_ctx_size, ext_ctx_size;

    tbl_sw_idx = GET_SW_TABLE_ID(table_id);
    int_ctx_size = GET_CONTEXT_BYTE_SIZE(g_hash_cfg.tbl_cfg[tbl_sw_idx].int_ctx_size);
    ext_ctx_size = GET_CONTEXT_BYTE_SIZE(g_hash_cfg.ctx_cfg[tbl_sw_idx].ext_ctx_size);

    if (int_ctx_size)
        memcpy(int_ctx, cam_ctx, int_ctx_size);
    if (ext_ctx_size)
        memcpy(ext_ctx, cam_ctx + int_ctx_size, ext_ctx_size);
}

static int _drv_hash_cam_entry_read(uint16_t cam_idx, bdmf_boolean *valid, uint8_t *key_p, uint8_t *table_id)
{
    int rc;
    bdmf_boolean _valid;
    uint64_t key;

    rc = ag_drv_hash_cam_indirect_addr_set(0, cam_idx);
    rc = rc ? rc : ag_drv_hash_cam_indirect_op_set(CAM_READ_CMD);
    rc = rc ? rc : _drv_hash_cam_wait_indirect();
    rc = rc ? rc : ag_drv_hash_cam_indirect_vlid_out_get(&_valid);
    if (rc)
        return BDMF_ERR_INTERNAL; /* Validation failure, bad input */

    if (valid)
        *valid = _valid;
    if (!_valid)
        return BDMF_ERR_NOENT;

    if (!key_p && !table_id)
        return 0;

    ag_drv_hash_cam_indirect_key_out_get(0, (hash_cam_indirect_key_out *)&key);
    _drv_hash_decode_cam_key(key, table_id, key_p);

    RDD_BTRACE("Read Key from CAM IDX %d, value 0x%llx, table_id %d\n", cam_idx, (long long unsigned int)key,
        table_id ? *table_id : -1); 
    RDD_BTRACE_BUF("Decoded Key", key_p, 8);
    return 0;
}

static uint8_t _drv_hash_cam_get_free_idx(void)
{
    uint8_t cam_idx;
    int rc;

    for (cam_idx = 0; cam_idx < HASH_CAM_NUM_ENTRIES; cam_idx++)
    {
        rc = _drv_hash_cam_entry_read(cam_idx, NULL, NULL, NULL);
        if (rc)
        {
            if (rc == BDMF_ERR_NOENT)
                break; /* Empty slot found */
            return HASH_CAM_NUM_ENTRIES;
        }
    }
    return cam_idx;
}

#ifndef RDP_SIM
static bdmf_error_t _drv_hash_cam_search(uint8_t table_id, uint8_t *key_p, hash_result_t *hash_res)
{
#if !defined(_CFE_)
    uint8_t cam_idx;
    uint64_t key;
    bdmf_boolean cam_match;
    bdmf_error_t rc;

    RDD_BTRACE("Searching in CAM, table_id %d, Key:\n", table_id);
    RDD_BTRACE_BUF("Key_p", key_p, 8);

    /* It is safe to look for encoded (=masked key), as the CAM will mask anyway */
    key = _drv_hash_encode_cam_key(table_id, key_p, NULL);

    rc = ag_drv_hash_cam_indirect_key_in_set(0, (hash_cam_indirect_key_in *)&key);
    rc = rc ? rc : ag_drv_hash_cam_indirect_op_set(CAM_SEARCH_CMD);
    if (rc)
        return rc;

    _drv_hash_cam_wait_indirect();

    rc = ag_drv_hash_cam_indirect_rslt_get(&cam_match, &cam_idx);
    if (rc)
        return rc;

    if (cam_match)
    {
        hash_res->match = HASH_MATCH_IN_CAM;
        hash_res->match_index = GET_CAM_AGE_INDEX(cam_idx);
        hash_res->context_addr = GET_CAM_CONTEXT_INDEX(cam_idx);
    }
    else
    {
        /* XXX: Need to simplify and rewrite the logic here and in usage; upon miss should not fill first free index. */
        uint8_t cam_idx = _drv_hash_cam_get_free_idx();
        uint8_t tbl_sw_idx = GET_SW_TABLE_ID(table_id);

        hash_res->match = HASH_MISS;
        /* If empty slot not found in HASH, occupy one from CAM */
        if (cam_idx < HASH_CAM_NUM_ENTRIES && hash_res->first_free_idx == HASH_INVALID_IDX &&
           (!g_hash_cfg.tbl_init[tbl_sw_idx].cam_max_threshold ||
           hash_mgmt[tbl_sw_idx].entries_in_cam < g_hash_cfg.tbl_init[tbl_sw_idx].cam_max_threshold))
        {
            hash_res->first_free_idx = GET_CAM_AGE_INDEX(cam_idx);
        }
    }

    RDD_BTRACE("CAM lookup for encoded key 0x%llx %s, index %d, first_free_index %d\n", (long long unsigned int)key,
        hash_res->match == HASH_MISS ? "Failed" : "Succeeded", hash_res->match_index, hash_res->first_free_idx);

    return 0;
#else
    return BDMF_ERR_INTERNAL;
#endif
}
#endif

static bdmf_error_t _drv_hash_cam_key_get(uint8_t table_id, uint16_t idx, uint8_t *key_p)
{
    uint8_t _table_id, cam_idx;
    bdmf_boolean valid;
    int rc;
    
    cam_idx = GET_CAM_INDEX(idx);
    rc = _drv_hash_cam_entry_read(cam_idx, &valid, key_p, &_table_id);
    if (rc || !valid)
        return BDMF_ERR_NOENT; /* Failed to read from any reason */
    if (table_id != _table_id)
        return BDMF_ERR_INTERNAL; /* Incorrectly coded table_id */

    return 0;
}

static bdmf_error_t _drv_hash_cam_modify_context(uint8_t table_id, uint8_t cam_idx, uint8_t *ext_ctx, uint8_t *int_ctx)
{
    uint32_t cam_ctx_idx;
    uint8_t cam_ctx[CAM_CTX_SIZE] = {}, ext_ctx_size, int_ctx_size;
    int rc;

    int_ctx_size = g_hash_cfg.tbl_cfg[GET_SW_TABLE_ID(table_id)].int_ctx_size;
    ext_ctx_size = g_hash_cfg.ctx_cfg[GET_SW_TABLE_ID(table_id)].ext_ctx_size;

    cam_ctx_idx = GET_CAM_CONTEXT_INDEX(cam_idx);

    /* write context */
    _drv_hash_cam_context_compose(table_id, ext_ctx, int_ctx, cam_ctx);

    rc = _drv_hash_write_ram_context(cam_ctx_idx, ext_ctx_size + int_ctx_size, cam_ctx);

    /* reset aging */
    rc = rc ? rc : ag_drv_hash_aging_ram_aging_set(GET_CAM_AGE_INDEX(cam_idx), 0);

    RDD_BTRACE("Insert to CAM context index %d (CAM idx %d), rc = %d\n"
        "\tInternal context: int_ctx_size = %d, int_ctx[0] = 0x%x, int_ctx[1] = 0x%x, int_ctx[2] = 0x%x\n"
        "\tExternal context: ext_ctx_size = %d, ext_ctx[0] = 0x%x, ext_ctx[1] = 0x%x, ext_ctx[2] = 0x%x, "
        "ext_ctx[3] = 0x%x, ext_ctx[4] = 0x%x, ext_ctx[5] = 0x%x\n",
        cam_ctx_idx, cam_idx, rc, int_ctx_size,
        int_ctx_size > 0 ? int_ctx[0] : -1, int_ctx_size > 0 ? int_ctx[1] : -1, int_ctx_size > 0 ? int_ctx[2] : -1,
        ext_ctx_size,
        ext_ctx_size > 0 ? ext_ctx[0] : -1, ext_ctx_size > 0 ? ext_ctx[1] : -1, ext_ctx_size > 0 ? ext_ctx[2] : -1,
        ext_ctx_size > 1 ? ext_ctx[3] : -1, ext_ctx_size > 1 ? ext_ctx[4] : -1, ext_ctx_size > 1 ? ext_ctx[5] : -1);

    return rc;
}

/* Add new key and context to CAM */
static bdmf_error_t _drv_hash_cam_insert(uint8_t table_id, uint8_t *key_p, uint8_t *ext_ctx, uint8_t *int_ctx,
    uint16_t *key_idx)
{
#if !defined(_CFE_)
    uint64_t encode_key, encode_mask;
    uint8_t cam_idx, tbl_sw_idx;
    int rc;

    RDD_BTRACE("Adding to CAM, table_id %d, Key + Ext CTX + Int CTX:\n", table_id);
    RDD_BTRACE_BUF("Key", key_p, 8);
    tbl_sw_idx = GET_SW_TABLE_ID(table_id);
    RDD_BTRACE_BUF("Ext CTX", ext_ctx, g_hash_cfg.tbl_init[tbl_sw_idx].ext_ctx_size);
    RDD_BTRACE_BUF("Int CTX", int_ctx, g_hash_cfg.tbl_init[tbl_sw_idx].int_ctx_size);

    cam_idx = _drv_hash_cam_get_free_idx();
    if (cam_idx == HASH_CAM_NUM_ENTRIES)
        return BDMF_ERR_NOMEM; /* CAM table is full */

    /* HASH Key is encoded when stored in CAM */
    encode_key = _drv_hash_encode_cam_key(table_id, key_p, &encode_mask);

    /* writing key0 part (encode key) */
    rc = ag_drv_hash_cam_indirect_vlid_in_set(1);
    rc = rc ? rc : ag_drv_hash_cam_indirect_addr_set(0, cam_idx);
    rc = rc ? rc : ag_drv_hash_cam_indirect_key_in_set(0, (hash_cam_indirect_key_in *)&encode_key);
    rc = rc ? rc : ag_drv_hash_cam_indirect_op_set(CAM_WRITE_CMD);
    rc = rc ? rc : _drv_hash_cam_wait_indirect();

    /* writing key1 part (encode mask) */
    rc = rc ? rc : ag_drv_hash_cam_indirect_addr_set(1, cam_idx);
    rc = rc ? rc : ag_drv_hash_cam_indirect_key_in_set(0, (hash_cam_indirect_key_in *)&encode_mask);
    rc = rc ? rc : ag_drv_hash_cam_indirect_op_set(CAM_WRITE_CMD);
    rc = rc ? rc : _drv_hash_cam_wait_indirect();

    if (rc)
        return rc;

    *key_idx = GET_CAM_AGE_INDEX(cam_idx);
    rc = _drv_hash_cam_modify_context(table_id, cam_idx, ext_ctx, int_ctx);
    if (!rc)
        hash_mgmt[tbl_sw_idx].entries_in_cam++;
    return rc;
#else
    return BDMF_ERR_INTERNAL;
#endif
}

static bdmf_error_t _drv_hash_get_context_from_cam(uint16_t idx, uint8_t table_id, 
    uint8_t *ext_ctx, uint8_t *int_ctx, bdmf_boolean *skp, uint8_t *cfg)
{
    int rc, ext_ctx_size, int_ctx_size;
    uint8_t cam_idx, cam_ctx[CAM_CTX_SIZE] = {};
    bdmf_boolean valid = 0;
    uint16_t cam_ctx_idx; 

    cam_idx = GET_CAM_INDEX(idx);
    cam_ctx_idx = GET_CAM_CONTEXT_INDEX(cam_idx);
    rc = _drv_hash_cam_entry_read(cam_idx, &valid, NULL, NULL);
    *skp = !valid;

    RDD_BTRACE("Check CAM context for cam idx %d (idx %d), cam_ctx_idx %d, *skp (=VALID) = %d, rc = %d\n", cam_idx, idx,
        cam_ctx_idx, *skp, rc);

    if (rc)
        return rc;

    int_ctx_size = g_hash_cfg.tbl_cfg[GET_SW_TABLE_ID(table_id)].int_ctx_size;
    ext_ctx_size = g_hash_cfg.ctx_cfg[GET_SW_TABLE_ID(table_id)].ext_ctx_size;

    *cfg = table_id;

    /* get context */
    rc = _drv_hash_read_ram_context(cam_ctx_idx, int_ctx_size + ext_ctx_size, cam_ctx);
   
    if (rc)
        return rc;
  
    _drv_hash_cam_context_decompose(table_id, cam_ctx, ext_ctx, int_ctx);

    RDD_BTRACE("Get context from CAM, cam_ctx index %d (CAM idx %d), rc = %d\n"
        "\tInternal context: int_ctx_size = %d, int_ctx[0] = 0x%x, int_ctx[1] = 0x%x, int_ctx[2] = 0x%x\n"
        "\tExternal context: ext_ctx_size = %d, ext_ctx[0] = 0x%x, ext_ctx[1] = 0x%x, ext_ctx[2] = 0x%x, "
        "ext_ctx[3] = 0x%x, ext_ctx[4] = 0x%x, ext_ctx[5] = 0x%x\n",
        cam_ctx_idx, cam_idx, rc, int_ctx_size,
        int_ctx_size > 0 ? int_ctx[0] : -1, int_ctx_size > 1 ? int_ctx[1] : -1, int_ctx_size > 2 ? int_ctx[2] : -1,
        ext_ctx_size,
        ext_ctx_size > 0 ? ext_ctx[0] : -1, ext_ctx_size > 1 ? ext_ctx[1] : -1, ext_ctx_size > 2 ? ext_ctx[2] : -1,
        ext_ctx_size > 3 ? ext_ctx[3] : -1, ext_ctx_size > 4 ? ext_ctx[4] : -1, ext_ctx_size > 5 ? ext_ctx[5] : -1);

    return 0;
}

static bdmf_error_t _drv_hash_cam_remove_index(uint32_t table_id, uint16_t idx)
{
    bdmf_error_t rc;
    uint16_t cam_idx;

    cam_idx = GET_CAM_INDEX(idx);
    rc = ag_drv_hash_cam_indirect_addr_set(0, cam_idx);
    rc = rc ? rc : ag_drv_hash_cam_indirect_vlid_in_set(0);
    rc = rc ? rc : ag_drv_hash_cam_indirect_op_set(CAM_WRITE_CMD);
    rc = rc ? rc : _drv_hash_cam_wait_indirect();
    if (!rc)
        hash_mgmt[GET_SW_TABLE_ID(table_id)].entries_in_cam--;
    return rc;
}

int drv_hash_cam_max_threshold_set(uint32_t table_id, uint32_t cam_max_threshold)
{
    if (VERIFY_TABLE_ID(table_id) || cam_max_threshold > HASH_CAM_NUM_ENTRIES)
        return BDMF_ERR_PARM;

    g_hash_cfg.tbl_init[GET_SW_TABLE_ID(table_id)].cam_max_threshold = cam_max_threshold;
    return 0;
}

/********/
/* HASH */
/********/
static bdmf_error_t _drv_hash_write_internal_context_to_key(uint8_t table_id, uint8_t *int_ctx, uint16_t *key_27_12,
    uint32_t *key_59_28)
{
    uint8_t i, ctx_size;
    uint64_t mask, key_copy, internal_context = 0;
    uint8_t aligned_int_ctx[ALIGN_CTX_MAX_BUF_SIZE] = {};

    ctx_size = g_hash_cfg.tbl_init[GET_SW_TABLE_ID(table_id)].int_ctx_size;
    mask = _drv_hash_get_table_mask(table_id);
    key_copy = CONSTRUCT_KEY(*key_59_28, *key_27_12, 0);
    key_copy &= mask;

    _drv_hash_align_and_swap(int_ctx, aligned_int_ctx, ctx_size);

    for (i = 0; i < ctx_size; i++)
        internal_context |= ((uint64_t)aligned_int_ctx[i]) << (60 - ((i+1) * 8));

    internal_context &= ~mask;
    key_copy |= internal_context;
    key_copy >>= 12;
    *key_27_12 = (uint16_t)(key_copy & 0xFFFF);
    key_copy >>= 16;
    *key_59_28 = (uint32_t)key_copy;
    return BDMF_ERR_OK;
}

static bdmf_error_t _drv_hash_write_key_and_internal_context(uint8_t table_id, uint16_t abs_idx, uint8_t *key_p, uint8_t *int_ctx)
{
    bdmf_error_t rc;
    bdmf_boolean skp;
    uint8_t cfg;
    uint16_t key_11_0, key_27_12;
    uint32_t key_59_28;
    uint64_t key = 0;

    if (VERIFY_TABLE_ID(table_id))
        return BDMF_ERR_PARM;

    /* verify the entry is empty */
    rc = ag_drv_hash_ram_eng_low_get(abs_idx, &skp, &cfg, &key_11_0, &key_27_12);
 
    /* write_internal_context */
    if (cfg && !skp && !rc)
        return BDMF_ERR_ALREADY;
    if (rc)
        return rc;

    key = _drv_hash_prepare_key_from_buf(table_id, key_p); 
    RDD_BTRACE("Adding key to HASH: 0x%llx\n", (long long unsigned int)key);
    
    key_11_0 = (uint16_t)key & 0xFFF;
    key_27_12 = (uint16_t)(key >> 12) & 0xFFFF;
    key_59_28 = (uint32_t)(key >> 28);

    if (g_hash_cfg.tbl_cfg[GET_SW_TABLE_ID(table_id)].int_ctx_size)
        rc = _drv_hash_write_internal_context_to_key(table_id, int_ctx, &key_27_12, &key_59_28);

    /* write key + internal context to SRAM */
    rc = rc ? rc : ag_drv_hash_ram_eng_low_set(abs_idx, 0, table_id, key_11_0, key_27_12);
    rc = rc ? rc : ag_drv_hash_ram_eng_high_set(abs_idx, key_59_28);

    return rc;
}

static bdmf_error_t _drv_hash_rule_insert(uint32_t table_id, uint16_t idx, uint8_t *key_p, uint8_t *ext_ctx,
    uint8_t *int_ctx)
{
    uint8_t tbl_sw_idx;
    uint16_t ctx_index, abs_idx;
    bdmf_error_t rc;

    if (VERIFY_TABLE_ID(table_id))
        return BDMF_ERR_PARM;

    tbl_sw_idx = GET_SW_TABLE_ID(table_id);
    abs_idx = GET_ABS_INDEX_IN_HASH(idx, tbl_sw_idx);

    /* reset aging */
    rc = ag_drv_hash_aging_ram_aging_set(abs_idx, 0);

    /* write external context */
    if (g_hash_cfg.ctx_cfg[tbl_sw_idx].ext_ctx_size)
    {
        RDD_BTRACE("inserting external context for tbl_sw_idx: %d, base_address: 0x%04x (%d) "
            "abs_idx: 0x%04x (%d), idx_in_table: 0x%04x (%d), eng: %d, hash_base_addr : 0x%04x (%d)\n",
            tbl_sw_idx, g_hash_cfg.ctx_cfg[tbl_sw_idx].base_address, g_hash_cfg.ctx_cfg[tbl_sw_idx].base_address,
            abs_idx, abs_idx, GET_IDX_IN_TABLE_FROM_ABS_INDEX(abs_idx), GET_IDX_IN_TABLE_FROM_ABS_INDEX(abs_idx), 
            GET_ENG_FROM_ABS_INDEX(abs_idx),
            g_hash_cfg.tbl_cfg[tbl_sw_idx].hash_base_addr, g_hash_cfg.tbl_cfg[tbl_sw_idx].hash_base_addr);

        ctx_index = GET_CONTEXT_INDEX(g_hash_cfg.ctx_cfg[tbl_sw_idx].base_address,
            GET_IDX_IN_TABLE_FROM_ABS_INDEX(abs_idx), GET_ENG_FROM_ABS_INDEX(abs_idx),
            g_hash_cfg.tbl_cfg[tbl_sw_idx].hash_base_addr, g_hash_cfg.ctx_cfg[tbl_sw_idx].ext_ctx_size);

        RDD_BTRACE("inserting external context for table_id: %d at context index 0x%06x (%d), "
            "external context memory offset is 0x%06x (%d)\n", 
            table_id, ctx_index, ctx_index, ctx_index << 2, ctx_index << 2);
        rc = rc ? rc : _drv_hash_write_ram_context(ctx_index, g_hash_cfg.ctx_cfg[tbl_sw_idx].ext_ctx_size,
            ext_ctx);
    }

    if (g_hash_cfg.tbl_init[tbl_sw_idx].int_ctx_size)
    {
        RDD_BTRACE("inserting internal context for table_id: %d at absolute context index 0x%04x\n",
            table_id, abs_idx);
        RDD_BTRACE_BUF("Int CTX", int_ctx, g_hash_cfg.tbl_init[tbl_sw_idx].int_ctx_size); 
    }

    /* write key with internal context (if exists) */
    rc = rc ? rc : _drv_hash_write_key_and_internal_context(table_id, abs_idx, key_p, int_ctx);
    if (!rc)
        hash_mgmt[tbl_sw_idx].entries_in_hash++;
    return rc;
}

typedef struct {
    uint8_t tbl_sw_idx;
    uint8_t tbl_size;
    uint8_t rsv; /* In order to use u32_swap in kernel */
} hash_cfg_sort_elem_t;

static int lkup_tbl_cfg_compare(const void *elem1, const void *elem2)
{
    hash_cfg_sort_elem_t *_elem1 = (hash_cfg_sort_elem_t *)elem1;
    hash_cfg_sort_elem_t *_elem2 = (hash_cfg_sort_elem_t *)elem2;

    /* Compare to comply sort in descending order */
    if (_elem1->tbl_size > _elem2->tbl_size)
        return -1;
    if (_elem1->tbl_size < _elem2->tbl_size)
        return 1;
    return 0;
}

static void _drv_hash_calc_base_addresses(hash_config_t *cfg)
{
    hash_cfg_sort_elem_t sorted_tbl_indexes[HASH_MAX_TABLES_NUM] = {};
    int i;
    uint32_t entries, first_hash_idx = 0, ctx_base_addr = 0;
    uint16_t base_addr_count = 0;

    /* general configuration */
    cfg->cam_base_addr = cam_context_base_addr_calc(cfg);

    for (i = 0; i < cfg->tbl_num; i++)
    {
        sorted_tbl_indexes[i].tbl_sw_idx = i;
        sorted_tbl_indexes[i].tbl_size = cfg->tbl_cfg[i].tbl_size;
    }

    bdmf_sort(sorted_tbl_indexes, cfg->tbl_num, sizeof(hash_cfg_sort_elem_t), lkup_tbl_cfg_compare, NULL);
    
    /* Calculate base addresses */
    for (i = 0; i < cfg->tbl_num; i++)
    {
        int curr_idx = sorted_tbl_indexes[i].tbl_sw_idx;

        cfg->tbl_cfg[curr_idx].hash_base_addr = base_addr_count;
        cfg->ctx_cfg[curr_idx].base_address = ctx_base_addr;
        cfg->ctx_cfg[curr_idx].first_hash_idx = first_hash_idx;

        base_addr_count += GET_TBL_SIZE(cfg->tbl_cfg[curr_idx].tbl_size);
        entries = GET_TBL_SIZE(cfg->tbl_cfg[curr_idx].tbl_size) * HASH_NUM_OF_ENGINES;
        first_hash_idx += entries;
        ctx_base_addr += ((entries * cfg->tbl_init[curr_idx].ext_ctx_size) / 6);
    }
}

static bdmf_error_t _drv_hash_remove_index(uint32_t table_id, uint16_t idx)
{
    bdmf_error_t rc;
    uint8_t tbl_sw_idx;
    uint16_t abs_idx;
    
    tbl_sw_idx = GET_SW_TABLE_ID(table_id);
    abs_idx = GET_ABS_INDEX_IN_HASH(idx, tbl_sw_idx);

    rc = ag_drv_hash_ram_eng_low_set(abs_idx, 1, table_id, 0, 0);
    rc = rc ? rc : ag_drv_hash_ram_eng_high_set(abs_idx, 0);
    if (!rc)
        hash_mgmt[tbl_sw_idx].entries_in_hash--;
    return rc;
}

static bdmf_error_t _drv_hash_modify_context(uint8_t table_id, uint16_t idx, uint8_t *ext_ctx, uint8_t *int_ctx)
{
    uint32_t key_59_28;
    uint16_t abs_idx, ctx_idx, key_27_12, key_11_0;
    uint8_t cfg, int_ctx_size, ext_ctx_size, tbl_sw_idx;
    bdmf_boolean skp;
    bdmf_error_t rc = BDMF_ERR_OK;

    tbl_sw_idx = GET_SW_TABLE_ID(table_id);

    abs_idx = GET_ABS_INDEX_IN_HASH(idx, tbl_sw_idx);
    int_ctx_size = g_hash_cfg.tbl_init[tbl_sw_idx].int_ctx_size;
    ext_ctx_size = g_hash_cfg.ctx_cfg[tbl_sw_idx].ext_ctx_size;

    RDD_BTRACE("hash_modify_context, int_ctx[0]: %x  int_ctx[1]: %x  int_ctx[2]: %x ext_ctx[0]: %x  "
        "ext_ctx[1]: %x  ext_ctx[2]: %x\n",
        int_ctx_size ? int_ctx[0] : -1, int_ctx_size ? int_ctx[1] : -1, int_ctx_size ? int_ctx[2] : -1,
        ext_ctx_size ? ext_ctx[0] : -1, ext_ctx_size ? ext_ctx[1] : -1, ext_ctx_size ? ext_ctx[2] : -1);
        
    if (int_ctx_size)
    {
        rc = ag_drv_hash_ram_eng_low_get(abs_idx, &skp, &cfg, &key_11_0, &key_27_12);
        rc = rc ? rc : ag_drv_hash_ram_eng_high_get(abs_idx, &key_59_28);
        if (rc)
            return rc;
        _drv_hash_write_internal_context_to_key(table_id, int_ctx, &key_27_12, &key_59_28);
        rc = ag_drv_hash_ram_eng_low_set(abs_idx, skp, cfg, key_11_0, key_27_12);
        rc = rc ? rc : ag_drv_hash_ram_eng_high_set(abs_idx, key_59_28);
        if (rc)
            return rc;
    }
    if (ext_ctx_size)
    {
        ctx_idx = GET_CONTEXT_INDEX(g_hash_cfg.ctx_cfg[tbl_sw_idx].base_address,
            GET_IDX_IN_TABLE_FROM_ABS_INDEX(abs_idx), GET_ENG_FROM_ABS_INDEX(abs_idx),
            g_hash_cfg.tbl_cfg[tbl_sw_idx].hash_base_addr, ext_ctx_size);
        RDD_BTRACE("inserting external context for table_id: %d at context index 0x%04x, "
            "external context memory offset is 0x%05x\n", 
            table_id, ctx_idx, ctx_idx << 3);
        rc = _drv_hash_write_ram_context(ctx_idx, ext_ctx_size, ext_ctx);
    }
    return rc;
}

static bdmf_error_t _drv_hash_get_context_from_hash(uint16_t idx, uint8_t table_id, 
    uint8_t *ext_ctx, uint8_t *int_ctx, bdmf_boolean *skp, uint8_t *cfg)
{
    /*
     * Does NOT support partial internal context hack (needs to be managed by the layer above)
     * context needs to be allocated the right size when given as argument - use drv_hash_ctx_alloc
     */
    uint8_t tbl_sw_idx, ctx_size;
    uint8_t aligned_int_ctx[ALIGN_CTX_MAX_BUF_SIZE] = {};
    uint16_t abs_idx, ctx_idx, key_11_0, key_27_12;
    uint32_t key_59_28;
    uint64_t mask, internal_context = 0;
    bdmf_error_t rc = BDMF_ERR_OK;

    tbl_sw_idx = GET_SW_TABLE_ID(table_id);

    /* Align index to searched table if required */
    abs_idx = GET_ABS_INDEX_IN_HASH(idx, tbl_sw_idx);

    ctx_idx = GET_CONTEXT_INDEX(g_hash_cfg.ctx_cfg[tbl_sw_idx].base_address, GET_IDX_IN_TABLE_FROM_ABS_INDEX(abs_idx),
        GET_ENG_FROM_ABS_INDEX(abs_idx), g_hash_cfg.tbl_cfg[tbl_sw_idx].hash_base_addr,
        g_hash_cfg.ctx_cfg[tbl_sw_idx].ext_ctx_size);

    /* get external context */
    rc = _drv_hash_read_ram_context(ctx_idx, g_hash_cfg.ctx_cfg[tbl_sw_idx].ext_ctx_size, ext_ctx);
    if (rc)
        return rc;

    /* get internal context */
    rc = ag_drv_hash_ram_eng_low_get(abs_idx, skp, cfg, &key_11_0, &key_27_12);
    rc = rc ? rc : ag_drv_hash_ram_eng_high_get(abs_idx, &key_59_28);
    if (rc)
        return rc;
    ctx_size = g_hash_cfg.tbl_init[tbl_sw_idx].int_ctx_size;
    mask = _drv_hash_get_table_mask(table_id);
    internal_context = CONSTRUCT_KEY(key_59_28, key_27_12, 0);
    internal_context &= ~mask;
    internal_context >>= (60 - (ctx_size  * 8));
    _copy_byte_by_byte(aligned_int_ctx, internal_context, ctx_size);
    _drv_hash_unalign_and_swap(int_ctx, aligned_int_ctx, ctx_size);

    return rc;
}

static bdmf_error_t _drv_hash_key_get(uint8_t table_id, uint16_t idx , uint8_t *key)
{
    bdmf_error_t rc;
    bdmf_boolean skp;
    uint8_t cfg;
    uint16_t abs_idx, key_11_0, key_27_12;
    uint32_t key_59_28;
    uint64_t mask, key64 = 0;
    uint8_t read_key_p[8] = {};

    /* Align index to searched table if required */
    abs_idx = GET_ABS_INDEX_IN_HASH(idx, GET_SW_TABLE_ID(table_id));

    /* A0 workaround for hash hw bug */
    if (GET_ENG_FROM_ABS_INDEX(abs_idx) >= HASH_NUM_OF_EFFECTIVE_ENGINES)
        return BDMF_ERR_NOENT;

    rc = ag_drv_hash_ram_eng_low_get(abs_idx, &skp, &cfg, &key_11_0, &key_27_12);
    if (!cfg || skp)
        return BDMF_ERR_NOENT;

    rc = rc ? rc : ag_drv_hash_ram_eng_high_get(abs_idx, &key_59_28);
    if (rc)
        return rc;

    mask = _drv_hash_get_table_mask(table_id);
    key64 = CONSTRUCT_KEY(key_59_28, key_27_12, key_11_0) & mask;
    _copy_byte_by_byte(read_key_p, key64, 8);
    _drv_hash_copy_swap_key(read_key_p, key, 8);
    return 0;
}

/****************/
/* External API */
/****************/
bdmf_error_t drv_hash_init(hash_config_t *cfg)
{
#ifndef RDP_SIM
    uint8_t cam_idx = 0;
#endif
    uint8_t tbl_idx;
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t eng, entry_idx, key_11_0 = 0, key_27_12 = 0, key_59_28 = 0;

    _drv_hash_calc_base_addresses(cfg);

    for (tbl_idx = 0; !rc && (tbl_idx < cfg->tbl_num); tbl_idx++)
    {
        cfg->tbl_cfg[tbl_idx].direct_lkup_en = 0;
        cfg->tbl_cfg[tbl_idx].hash_type = 0;

        /* verify input is valid */
        if (!VERIFY_TBL_INIT_DATA(cfg->tbl_init[tbl_idx].int_ctx_size, cfg->tbl_init[tbl_idx].ext_ctx_size,
            cfg->tbl_init[tbl_idx].search_depth_per_engine))
        {
            return BDMF_ERR_PARM;
        }

        /* convert input to cfg values */
        cfg->tbl_cfg[tbl_idx].int_ctx_size = cfg->tbl_init[tbl_idx].int_ctx_size / 3;
        cfg->ctx_cfg[tbl_idx].ext_ctx_size = cfg->tbl_init[tbl_idx].ext_ctx_size / 3;
        cfg->tbl_cfg[tbl_idx].max_hop = cfg->tbl_init[tbl_idx].search_depth_per_engine - 1;

        rc = rc ? rc : ag_drv_hash_lkup_tbl_cfg_tbl_cfg_set(tbl_idx, &cfg->tbl_cfg[tbl_idx]);
        rc = rc ? rc : ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_set(tbl_idx, &cfg->ctx_cfg[tbl_idx]);
        rc = rc ? rc : ag_drv_hash_mask_set(tbl_idx, &cfg->mask[tbl_idx]);
    }
    rc = rc ? rc : ag_drv_hash_cam_base_addr_set(cfg->cam_base_addr);

    /* initializing Hash key RAM */
    for (entry_idx = 0; !rc && entry_idx < (HASH_RAM_ENG_HIGH_REG_RAM_CNT + 1) / HASH_NUM_OF_ENGINES; entry_idx++)
        for (eng = 0; eng < HASH_NUM_OF_ENGINES; eng++)
            rc = rc ? rc : ag_drv_hash_ram_eng_high_set(GET_ENTRY_IDX_PER_ENGINE(entry_idx, eng), key_59_28);

    for (entry_idx = 0; !rc && entry_idx < (HASH_RAM_ENG_LOW_REG_RAM_CNT + 1) / HASH_NUM_OF_ENGINES; entry_idx++)
        for (eng = 0; eng < HASH_NUM_OF_ENGINES; eng++)
            rc = rc ? rc : ag_drv_hash_ram_eng_low_set(GET_ENTRY_IDX_PER_ENGINE(entry_idx, eng), 1, 0, key_11_0, key_27_12);

    /* initializing Hash External Context RAM */
    for (entry_idx = 0; !rc && entry_idx <= HASH_CONTEXT_RAM_CONTEXT_47_24_REG_RAM_CNT; entry_idx++)
        rc = rc ? rc : ag_drv_hash_context_ram_context_47_24_set(entry_idx, 0);

    for (entry_idx = 0; !rc && entry_idx <= HASH_CONTEXT_RAM_CONTEXT_23_0_REG_RAM_CNT; entry_idx++)
        rc = rc ? rc : ag_drv_hash_context_ram_context_23_0_set(entry_idx, 0);

    /* initializing Hash Aging RAM (HASH and CAM) */
    for (entry_idx = 0; entry_idx < (HASH_RAM_ENG_HIGH_REG_RAM_CNT + 1 + HASH_CAM_NUM_ENTRIES); entry_idx++)
        rc = rc ? rc : ag_drv_hash_aging_ram_aging_set(entry_idx, 1);

#ifndef RDP_SIM
    /* initialize CAM */
    for (cam_idx = 0; cam_idx < HASH_CAM_NUM_ENTRIES; cam_idx++)
    {
        rc = rc ? rc : ag_drv_hash_cam_indirect_addr_set(0, cam_idx);
        rc = rc ? rc : ag_drv_hash_cam_indirect_vlid_in_set(0);
        rc = rc ? rc : ag_drv_hash_cam_indirect_op_set(CAM_WRITE_CMD);
        rc = rc ? rc : _drv_hash_cam_wait_indirect();
    }
#endif
#if defined(RDP_SIM) && !defined(XRDP_EMULATION)
    rc = rc ? rc : ag_drv_hash_key_padding_set(HASH_PAD_VALUE_H, HASH_PAD_VALUE_L);
#endif

    memcpy(&g_hash_cfg, cfg, sizeof(hash_config_t));

    return rc;
}

/* This flag used for debugging purpose. When set, lookup is done in CAM only. */
int in_cam_only = 0;

bdmf_error_t drv_hash_find(uint8_t table_id, uint8_t *key_p, hash_result_t *hash_res)
{
    uint8_t eng, cfg, tbl_sw_idx, key_to_crc[8];
    uint16_t en_idx, eng_hash[4], key_11_0, key_27_12;
    uint32_t depth;
    uint64_t mask, key, entry_key, padding;
    uint32_t key_59_28;
    bdmf_boolean skp;
    bdmf_error_t rc = BDMF_ERR_OK;

    if (VERIFY_TABLE_ID(table_id))
        return BDMF_ERR_PARM;

    tbl_sw_idx = GET_SW_TABLE_ID(table_id);

    hash_res->match = HASH_MISS;
    hash_res->first_free_idx = HASH_INVALID_IDX;
    hash_res->duplicate_idx = HASH_INVALID_IDX;

    key = _drv_hash_prepare_key_from_buf(table_id, key_p); 
    mask = _drv_hash_get_table_mask(table_id);
    /* Since we compare the key with padded values in the HASH table, add padding */
    padding = _drv_hash_get_key_padding(mask);
    key |= padding;

    _copy_byte_by_byte(key_to_crc, key, HASH_CRC_BYTE_SIZE);

    /* this calculates the initial hash for each engine */
    for (eng = 0; eng < HASH_NUM_OF_ENGINES; eng++)
    {
        eng_hash[eng] = rdd_crc_bit_by_bit_hash(&key_to_crc[0], HASH_KEY_SIZE, HASH_CRC_BYTE_SIZE,
            (crc16_polynom_t)eng);
    }

    RDD_BTRACE("Looking for Key in table %d, CAM lookup only %d\n", table_id, in_cam_only);
    RDD_BTRACE_BUF("Key", key_p, 8);

    /* performing Hash table search for each engine */
    for (depth = 0; !in_cam_only && depth < g_hash_cfg.tbl_init[tbl_sw_idx].search_depth_per_engine; depth++)
    {
        for (eng = 0; eng < HASH_NUM_OF_EFFECTIVE_ENGINES; eng++)
        {
            /* getting the index to search in each engine RAM */
            en_idx = _get_hash_table_index(eng_hash[eng], depth, tbl_sw_idx);

            /* extracting the key from RAM */
            rc = rc ? rc : ag_drv_hash_ram_eng_low_get(GET_ENTRY_IDX_PER_ENGINE(en_idx, eng), &skp, &cfg, &key_11_0,
                &key_27_12);
            rc = rc ? rc : ag_drv_hash_ram_eng_high_get(GET_ENTRY_IDX_PER_ENGINE(en_idx, eng), &key_59_28);

            /* if entry is free and no free entry was found earlier mark entry as first available entry */
            if (!rc)
            {
                if ((skp || (cfg == 0)) && hash_res->first_free_idx == HASH_INVALID_IDX)
                {
                    hash_res->first_free_idx = GET_INDEX_IN_TABLE(GET_ENTRY_IDX_PER_ENGINE(en_idx, eng), tbl_sw_idx);
                    /* if we reached an invalid entry- exit search with MISS */
                    if (cfg == 0)
                    {
#ifndef RDP_SIM
                        /* Try searching CAM */
                        if (g_hash_cfg.tbl_cfg[tbl_sw_idx].cam_en)
                            rc = rc ? rc : _drv_hash_cam_search(table_id, key_p, hash_res);
                        else
                            hash_res->match = HASH_MISS;
#endif
                        goto exit;
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
                        hash_res->match_index = GET_INDEX_IN_TABLE(GET_ENTRY_IDX_PER_ENGINE(en_idx, eng), tbl_sw_idx);
                        hash_res->context_addr = GET_CONTEXT_INDEX(g_hash_cfg.ctx_cfg[tbl_sw_idx].base_address,
                            GET_ENTRY_IDX_PER_ENGINE(en_idx, eng),
                            en_idx, g_hash_cfg.tbl_cfg[tbl_sw_idx].hash_base_addr,
                            g_hash_cfg.ctx_cfg[tbl_sw_idx].ext_ctx_size);
                        goto exit;
                    }
                }
            }
        }
    }
#ifndef RDP_SIM
    /* If not found - Try searching CAM */
    if (g_hash_cfg.tbl_cfg[tbl_sw_idx].cam_en)
        rc = rc ? rc : _drv_hash_cam_search(table_id, key_p, hash_res);
#endif

exit:
    if (hash_res->match == HASH_MISS && hash_res->first_free_idx == HASH_INVALID_IDX &&
        hash_mgmt[tbl_sw_idx].first_lkp_miss == 0)
    {
        hash_mgmt[tbl_sw_idx].first_lkp_miss = hash_mgmt[tbl_sw_idx].entries_in_hash +
            hash_mgmt[tbl_sw_idx].entries_in_cam + 1;
    }

    return rc;
}

bdmf_error_t drv_hash_rule_add(uint32_t table_id, uint8_t *key_p, uint8_t *ext_ctx, uint8_t *int_ctx,
    uint16_t *key_idx)
{
    bdmf_error_t rc;
    hash_result_t hash_res = {};

    if (VERIFY_TABLE_ID(table_id))
        return BDMF_ERR_PARM;

    rc = drv_hash_find(table_id, key_p, &hash_res);

    RDD_BTRACE("hash_add -> find result find result: %d, index: 0x%04x (%d), rc = %d\n",
        hash_res.match, hash_res.first_free_idx, hash_res.first_free_idx, rc);
    if (rc)
        return rc;
    if (hash_res.match != HASH_MISS)
        return BDMF_ERR_ALREADY;

    if (hash_res.duplicate_idx != HASH_INVALID_IDX)
    {
        RDD_BTRACE("***Hash add - encountered duplicate index***\n");
        drv_hash_rule_remove_index(table_id, hash_res.duplicate_idx);
    }
    
    if (hash_res.first_free_idx == HASH_INVALID_IDX) /* this means no free place in hash or cam table was found */
            return BDMF_ERR_NOMEM;

    if (IS_INDEX_IN_CAM(hash_res.first_free_idx))
        return _drv_hash_cam_insert(table_id, key_p, ext_ctx, int_ctx, key_idx);

    RDD_BTRACE("insert rule at table_id: %d is at index: %04x (%d), "
        "hash memory offset is 0x%05x  \n", 
        table_id, hash_res.first_free_idx, hash_res.first_free_idx, hash_res.first_free_idx << 3);
    *key_idx = hash_res.first_free_idx;
    return _drv_hash_rule_insert(table_id, hash_res.first_free_idx, key_p, ext_ctx, int_ctx);
}

bdmf_error_t drv_hash_key_get(uint8_t table_id, uint16_t idx , uint8_t *key)
{
    RDD_BTRACE("table_id = %d, idx = %d, key = %p\n", table_id, idx, key);

    if (VERIFY_TABLE_ID(table_id))
        return BDMF_ERR_PARM;

    if (IS_INDEX_IN_CAM(idx))
        return _drv_hash_cam_key_get(table_id, idx, key);

    return _drv_hash_key_get(table_id, idx, key);
}

bdmf_error_t drv_hash_get_context(uint16_t idx, uint8_t table_id, 
    uint8_t *ext_ctx, uint8_t *int_ctx, bdmf_boolean *skp, uint8_t *cfg)
{
    int rc;

    if (VERIFY_TABLE_ID(table_id))
        return BDMF_ERR_PARM;

    RDD_BTRACE("Get context from %s, IDX = %d, table_id = %d\n", IS_INDEX_IN_CAM(idx) ? "CAM" : "HASH",
        idx, table_id);

    if (IS_INDEX_IN_CAM(idx))
        rc = _drv_hash_get_context_from_cam(idx, table_id, ext_ctx, int_ctx, skp, cfg);
    else
        rc = _drv_hash_get_context_from_hash(idx, table_id, ext_ctx, int_ctx, skp, cfg);

    return rc;
}

bdmf_error_t drv_hash_modify_context(uint8_t table_id, uint16_t idx, uint8_t *ext_ctx, uint8_t *int_ctx)
{
    if (VERIFY_TABLE_ID(table_id))
        return BDMF_ERR_PARM;

    if (IS_INDEX_IN_CAM(idx))
        return _drv_hash_cam_modify_context(table_id, GET_CAM_INDEX(idx), ext_ctx, int_ctx);

    return _drv_hash_modify_context(table_id, idx, ext_ctx, int_ctx);
}

bdmf_error_t drv_hash_rule_remove_index(uint32_t table_id, uint16_t idx)
{
    if (IS_INDEX_IN_CAM(idx))
        return _drv_hash_cam_remove_index(table_id, idx);

    return _drv_hash_remove_index(table_id, idx);
}

bdmf_error_t drv_hash_set_aging(uint32_t table_id, uint16_t idx, bdmf_boolean *prev_age)
{
    bdmf_error_t rc;
    uint16_t age_idx;

    if (IS_INDEX_IN_CAM(idx))
        age_idx = idx;
    else
        age_idx = GET_ABS_INDEX_IN_HASH(idx, GET_SW_TABLE_ID(table_id));

    rc = ag_drv_hash_aging_ram_aging_get(age_idx, prev_age);
    rc = rc ? rc : ag_drv_hash_aging_ram_aging_set(age_idx, 1);
    RDD_BTRACE("Set aging: table_id %d, index %d, rc %d, prev_age %d\n", table_id, idx, rc, *prev_age);
    return rc;
}

bdmf_error_t drv_hash_rule_remove_key(uint32_t table_id, uint8_t *key_p)
{
    hash_result_t hash_res;
    bdmf_error_t rc;

    rc = drv_hash_find(table_id, key_p, &hash_res);
    if (!rc && hash_res.match == HASH_MISS)
        return BDMF_ERR_ALREADY;

    return drv_hash_rule_remove_index(table_id, hash_res.match_index);
}

/******************************************************************************/
/*                                                                            */
/* Driver shell functions                                                     */
/*                                                                            */
/******************************************************************************/

#ifdef USE_BDMF_SHELL

static int _drv_hash_cli_hash_key_to_table_key(uint32_t table_id, RDD_HASH_COMMAND_DTS hash_key, uint8_t *table_key)
{
    if (table_id == HASH_TABLE_IPTV)
    {
        RDD_IPTV_HASH_LKP_ENTRY_DTS iptv_hash_key = {};

        iptv_hash_key.dst_ip_or_mac_low = hash_key.key_0 + (hash_key.key_1 << 28);
        iptv_hash_key.addr_high = hash_key.key_1 >> 4;
        memcpy(table_key, (uint8_t*)&iptv_hash_key, sizeof(RDD_IPTV_HASH_LKP_ENTRY_DTS));
        return BDMF_ERR_OK;
    }
#if !defined(BCM63158)
    else
        if (table_id == HASH_TABLE_ARL)
        {
            RDD_BRIDGE_ARL_LKP_CMD_DTS arl_hash_key = {};

            arl_hash_key.mac_3_6 = hash_key.key_0 + (hash_key.key_1 << 28);
            arl_hash_key.mac_1_2 = hash_key.key_1 >> 4 & 0xFFFF;
            arl_hash_key.bridge_id = hash_key.key_1 >> 20;
            memcpy(table_key, (uint8_t *)&arl_hash_key, sizeof(RDD_BRIDGE_ARL_LKP_CMD_DTS));
            return BDMF_ERR_OK;
        }
        else 
            if (table_id == HASH_TABLE_BRIDGE_AND_VLAN_LKP)
            {
                uint32_t isolation_key = hash_key.key_1;
                memset(&table_key[0], 0,sizeof(uint32_t));
                memcpy(&table_key[4], (uint8_t *)&isolation_key, sizeof(uint32_t));
                return BDMF_ERR_OK;
            }
#endif
            else            
                return BDMF_ERR_INTERNAL;
}

static int drv_hash_cli_find(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    uint8_t table_key[8] = {};
    hash_result_t hash_res = {};
    RDD_HASH_COMMAND_DTS hash_key = {};
    /*RDD_IPTV_HASH_LKP_ENTRY_DTS hash_key = {};*/
    bdmf_error_t rc;
    uint32_t table_id = parm[parameter_index++].value.unumber; /* input */

    /*hash_key.addr_high = parm[parameter_index++].value.unumber;*/ /* input */
    /*hash_key.dst_ip_or_mac_low = parm[parameter_index++].value.unumber;*/ /* input */

    hash_key.key_0 = parm[parameter_index++].value.unumber; /* input */
    hash_key.key_1 = parm[parameter_index++].value.unumber; /* input */
    
    rc = _drv_hash_cli_hash_key_to_table_key(table_id, hash_key, table_key);
    if (rc)
    {
        bdmf_session_print(session, "Hash table number %d is not defined", table_id);
        return rc;
    }
    rc =  drv_hash_find(table_id, table_key, &hash_res);
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

static int drv_hash_cli_add_internal(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    RDD_HASH_COMMAND_DTS hash_key = {};
    hash_result_t hash_res = {};
    uint8_t int_ctx[16] = {};
    uint8_t table_key[8] = {}; 
    bdmf_error_t rc;
    uint8_t int_ctx_size;
    hash_lkup_tbl_cfg_tbl_cfg lkp_tbl_cfg;
    uint16_t key_idx;
    
    uint32_t table_id = parm[parameter_index++].value.unumber; /* input */
    rc = ag_drv_hash_lkup_tbl_cfg_tbl_cfg_get(GET_SW_TABLE_ID(table_id), &lkp_tbl_cfg);

    if (rc)
        return rc;
    if (!lkp_tbl_cfg.int_ctx_size)
    {
        bdmf_session_print(session, "Internal context size for table %d is 0\n\r", table_id);
        return BDMF_ERR_INVALID_OP;
    }

    int_ctx_size = lkp_tbl_cfg.int_ctx_size * 3;
    hash_key.key_0 = parm[parameter_index++].value.unumber; /* input */
    hash_key.key_1 = parm[parameter_index++].value.unumber; /* input */
    int_ctx[0] = parm[parameter_index].value.unumber >> 16; /* input */
    int_ctx[1] = parm[parameter_index].value.unumber >> 8 & 0xFF; 
    int_ctx[2] = parm[parameter_index++].value.unumber & 0xFF;
    if (int_ctx_size > 3)
    {
        int_ctx[3] = parm[parameter_index].value.unumber >> 16; /* input */
        int_ctx[4] = parm[parameter_index].value.unumber >> 8 & 0xFF;
        int_ctx[5] = parm[parameter_index++].value.unumber & 0xFF;
    }
    rc = _drv_hash_cli_hash_key_to_table_key(table_id, hash_key, table_key);
    if (rc)
        return rc;
    
    rc =  drv_hash_find(table_id, table_key, &hash_res);
    if (hash_res.match)
        rc = drv_hash_modify_context(table_id, hash_res.match_index, NULL, int_ctx);
    else
        rc = drv_hash_rule_add(table_id, table_key, NULL, int_ctx, &key_idx);
    if (rc)
    {
        bdmf_session_print(session, "Can't add entry. rc : %d\n\r", rc);
        return rc;
    }
    else
        bdmf_session_print(session, "Entry added, index %d\n\r", key_idx);

    return 0;
}

static int drv_hash_cli_add_external(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    RDD_HASH_COMMAND_DTS hash_key = {};
    uint8_t table_key[8] = {};
    hash_result_t hash_res = {};
    bdmf_error_t rc;
    uint8_t ext_ctx_size;
    uint8_t ext_ctx[16] = {};
    hash_lkup_tbl_cfg_cntxt_cfg context_cfg;
    int i;
    uint16_t key_idx;
    uint32_t table_id = parm[parameter_index++].value.unumber; /* input */    
    
    rc = ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get(GET_SW_TABLE_ID(table_id), &context_cfg);
    if (rc)
        return rc;
    ext_ctx_size = context_cfg.ext_ctx_size * 3;
    if (!ext_ctx_size)
    {
        bdmf_session_print(session, "External context size for table %d is 0\n\r", table_id);
        return BDMF_ERR_INVALID_OP;
    }
    if ((n_parms - 3) != context_cfg.ext_ctx_size)
    {
        bdmf_session_print(session, 
            "External context size for table %d is %d and is not compliant to the number of chunks in command\n\r", 
             table_id, ext_ctx_size);
        return BDMF_ERR_PARM;
    }

    hash_key.key_0 = parm[parameter_index++].value.unumber; /* input */
    hash_key.key_1 = parm[parameter_index++].value.unumber; /* input */
    for (i = 0; ext_ctx_size > 0; i+=3)
    {   
        ext_ctx_size -= 3;
        ext_ctx[i] = parm[parameter_index].value.unumber >> 16; /* input */
        ext_ctx[i+1] = parm[parameter_index].value.unumber >> 8 & 0xFF; 
        ext_ctx[i+2] = parm[parameter_index++].value.unumber & 0xFF;
    }
    rc = _drv_hash_cli_hash_key_to_table_key(table_id, hash_key, table_key);
    rc =  drv_hash_find(table_id, table_key, &hash_res);
    if (rc)
        return rc;
    if (hash_res.match)
        rc = drv_hash_modify_context(table_id, hash_res.match_index, ext_ctx, NULL);
    else
        rc = drv_hash_rule_add(table_id, table_key, ext_ctx, NULL, &key_idx);
    if (rc)
    {
        bdmf_session_print(session, "Can't add entry. rc : %d\n\r", rc);
        return rc;
    }
    else
        bdmf_session_print(session, "Entry added, index %d\n\r", key_idx);

    return 0;
}

static int drv_hash_cli_add_internal_external(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    hash_lkup_tbl_cfg_tbl_cfg lkp_tbl_cfg;
    hash_lkup_tbl_cfg_cntxt_cfg context_cfg;
    uint8_t parameter_index = 0;
    uint8_t ext_ctx_size;
    uint8_t int_ctx[16] = {}, ext_ctx[16] = {};
    RDD_HASH_COMMAND_DTS hash_key = {};
    uint8_t table_key[8] = {};
    hash_result_t hash_res = {};
    uint16_t key_idx;
    int i;
    bdmf_error_t rc;

    uint32_t table_id = parm[parameter_index++].value.unumber; /* input */

    rc = ag_drv_hash_lkup_tbl_cfg_tbl_cfg_get(GET_SW_TABLE_ID(table_id), &lkp_tbl_cfg);
    if (rc)
        return rc;

    rc = ag_drv_hash_lkup_tbl_cfg_cntxt_cfg_get(table_id, &context_cfg);
    if (rc)
        return rc;

    if (lkp_tbl_cfg.int_ctx_size == 0)
    {
        bdmf_session_print(session, "Internal context size for table %d is 0\n\r", table_id);
        return BDMF_ERR_INVALID_OP;
    }

    if (context_cfg.ext_ctx_size == 0)
    {
        bdmf_session_print(session, "External context size for table %d is 0\n\r", table_id);
        return BDMF_ERR_INVALID_OP;
    }

    ext_ctx_size = context_cfg.ext_ctx_size * 3;

    if ((n_parms - 3) != context_cfg.ext_ctx_size)
    {
        bdmf_session_print(session, 
            "External context size for table %d is %d and is not compliant to the number of chunks in command\n\r", 
             table_id, ext_ctx_size);
        return BDMF_ERR_PARM;
    }

    hash_key.key_0 = 0;
    hash_key.key_1 = parm[parameter_index++].value.unumber; /* input */

    int_ctx[0] = parm[parameter_index].value.unumber >> 16; /* input */
    int_ctx[1] = parm[parameter_index].value.unumber >> 8 & 0xFF; 
    int_ctx[2] = parm[parameter_index++].value.unumber & 0xFF;

    for (i = 0; ext_ctx_size > 0; i+=3)
    {   
        ext_ctx_size -= 3;
        ext_ctx[i] = parm[parameter_index].value.unumber >> 16; /* input */
        ext_ctx[i+1] = parm[parameter_index].value.unumber >> 8 & 0xFF; 
        ext_ctx[i+2] = parm[parameter_index++].value.unumber & 0xFF;
    }

    rc = _drv_hash_cli_hash_key_to_table_key(table_id, hash_key, table_key);
    if (rc)
        return rc;

    rc =  drv_hash_find(table_id, table_key, &hash_res);
    if (rc)
        return rc;

    if (hash_res.match)
        rc = drv_hash_modify_context(table_id, hash_res.match_index, ext_ctx, int_ctx);
    else
        rc = drv_hash_rule_add(table_id, table_key, ext_ctx, int_ctx, &key_idx);
    if (rc)
    {
        bdmf_session_print(session, "Can't add entry. rc : %d\n\r", rc);
        return rc;
    }
    else
        bdmf_session_print(session, "Entry added, index %d\n\r", key_idx);

    return 0;
}
 
static int drv_hash_cli_remove_key(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint8_t parameter_index = 0;
    uint8_t table_key[8] = {};
    RDD_HASH_COMMAND_DTS hash_key = {};
    bdmf_error_t rc;
    uint32_t table_id = parm[parameter_index++].value.unumber; /* input */

    hash_key.key_0 = parm[parameter_index++].value.unumber; /* input */
    hash_key.key_1 = parm[parameter_index++].value.unumber; /* input */
    rc = _drv_hash_cli_hash_key_to_table_key(table_id, hash_key, table_key);
    if (rc)
        return rc;

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

    rc = drv_hash_get_context(entry_idx, table_id, (uint8_t *)&ext_ctx, (uint8_t *)&int_ctx, NULL, NULL);
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
    uint8_t key_arr[8] = {};
    uint64_t key;

    bdmf_error_t rc;
    uint32_t table_id = parm[parameter_index++].value.unumber; /* input */
    uint32_t entry_idx = parm[parameter_index++].value.unumber; /* input */

    rc = drv_hash_key_get(table_id, entry_idx , key_arr);
    if (rc)
    {
        bdmf_session_print(session, "Can't get entry key. rc : %d\n\r", rc);
        return rc;
    }
    else
    {
        key = _uint8_arr_to_uint64(key_arr, 8);
        bdmf_session_print(session, "Key entry: 0x%16llx, low 0x%x, high 0x%x\n\r", (unsigned long long int)key,
            (uint32_t)(key & 0xffffffff), (uint32_t)(key >> 32));
    }

    return 0;
}

static int drv_hash_cli_cam_only_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    in_cam_only = (int)parm[0].value.unumber;
    return 0;
}

static int drv_hash_cli_tbl_mgmt_read(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint32_t table_id = (int)parm[0].value.unumber;

    table_id = GET_SW_TABLE_ID(table_id);
    bdmf_session_print(session, "Number of entries in HASH TBL_%d: %d\n", table_id,
        hash_mgmt[table_id].entries_in_hash);
    bdmf_session_print(session, "Number of entries in CAM  TBL_%d: %d\n", table_id, hash_mgmt[table_id].entries_in_cam);
    bdmf_session_print(session, "First lookup miss for     TBL_%d: %d\n", table_id, hash_mgmt[table_id].first_lkp_miss);
    bdmf_session_print(session, "CAM entries threshold for TBL_%d: %d\n", table_id,
        g_hash_cfg.tbl_init[table_id].cam_max_threshold);
    return 0;
}

static int drv_hash_cli_cam_max_thresh_set(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[],
    uint16_t n_parms)
{
    uint32_t table_id = parm[0].value.unumber; /* input */
    uint32_t max_thresh = parm[1].value.unumber; /* input */

    return drv_hash_cam_max_threshold_set(table_id, max_thresh);
}

static bdmfmon_handle_t hash_dir;

void drv_hash_cli_init(bdmfmon_handle_t driver_dir)
{
    hash_dir = ag_drv_hash_cli_init(driver_dir);

    BDMFMON_MAKE_CMD(hash_dir, "hash_find", "find entry in hash", drv_hash_cli_find,
        BDMFMON_MAKE_PARM("table", "table id", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_RANGE("key_high", "hash key high (4 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffffff),
        BDMFMON_MAKE_PARM_RANGE("key_low", "hash key low (4 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffffff));

    BDMFMON_MAKE_CMD(hash_dir, "hash_add_internal", "add/modify int ctx entry in hash", drv_hash_cli_add_internal,
        BDMFMON_MAKE_PARM("table", "table id", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_RANGE("key_high", "hash key high (4 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffffff),
        BDMFMON_MAKE_PARM_RANGE("key_low", "hash key low (4 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffffff),
        BDMFMON_MAKE_PARM_RANGE("ctx_hi", "ctx MSB chunk (3 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffff),
        BDMFMON_MAKE_PARM_RANGE("ctx_lo", "ctx LSB chunk (3 bytes)", BDMFMON_PARM_HEX, BDMFMON_PARM_FLAG_OPTIONAL, 0, 0xffffff));

    BDMFMON_MAKE_CMD(hash_dir, "hash_add_external", "add/modify ext ctx entry in hash", drv_hash_cli_add_external,
        BDMFMON_MAKE_PARM("table", "table id", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_RANGE("key_high", "hash key high (4 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffffff),
        BDMFMON_MAKE_PARM_RANGE("key_low", "hash key low (4 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffffff),
        BDMFMON_MAKE_PARM_RANGE("ctx_0", "ctx[2:0]  chunk (3 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffff),
        BDMFMON_MAKE_PARM_RANGE("ctx_1", "ctx[5:3] chunk (3 bytes)", BDMFMON_PARM_HEX, BDMFMON_PARM_FLAG_OPTIONAL, 0, 0xffffff),
        BDMFMON_MAKE_PARM_RANGE("ctx_2", "ctx[8:6] chunk (3 bytes)", BDMFMON_PARM_HEX, BDMFMON_PARM_FLAG_OPTIONAL, 0, 0xffffff),
        BDMFMON_MAKE_PARM_RANGE("ctx_3", "ctx[11:9] chunk (3 bytes)", BDMFMON_PARM_HEX, BDMFMON_PARM_FLAG_OPTIONAL, 0, 0xffffff));

    BDMFMON_MAKE_CMD(hash_dir, "hash_add_internal_external", "add/modify int and ext ctx entry in hash", drv_hash_cli_add_internal_external,
        BDMFMON_MAKE_PARM("table", "table id", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM_RANGE("key_low", "hash key low (4 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffffff),
        BDMFMON_MAKE_PARM_RANGE("ctx_int", "ctx chunk (3 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffff),
        BDMFMON_MAKE_PARM_RANGE("ctx_0", "ctx[2:0] chunk (3 bytes)", BDMFMON_PARM_HEX, 0, 0, 0xffffff),
        BDMFMON_MAKE_PARM_RANGE("ctx_1", "ctx[5:3] chunk (3 bytes)", BDMFMON_PARM_HEX, BDMFMON_PARM_FLAG_OPTIONAL, 0, 0xffffff),
        BDMFMON_MAKE_PARM_RANGE("ctx_2", "ctx[8:6] chunk (3 bytes)", BDMFMON_PARM_HEX, BDMFMON_PARM_FLAG_OPTIONAL, 0, 0xffffff),
        BDMFMON_MAKE_PARM_RANGE("ctx_3", "ctx[11:9] chunk (3 bytes)", BDMFMON_PARM_HEX, BDMFMON_PARM_FLAG_OPTIONAL, 0, 0xffffff));

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

    BDMFMON_MAKE_CMD(hash_dir, "cam_only_set", "Set Cam only lookup (debug)", drv_hash_cli_cam_only_set,
        BDMFMON_MAKE_PARM_RANGE("ena", "Enable:1, Disable:0", BDMFMON_PARM_NUMBER, 0, 0, 1));

    BDMFMON_MAKE_CMD(hash_dir, "hash_read_mgmt", "Read hash table management info", drv_hash_cli_tbl_mgmt_read,
        BDMFMON_MAKE_PARM_RANGE("table", "table id", BDMFMON_PARM_NUMBER, 0, 1, 8));

    BDMFMON_MAKE_CMD(hash_dir, "hash_cam_thresh_set", "Set CAM entries max threshold", drv_hash_cli_cam_max_thresh_set,
        BDMFMON_MAKE_PARM_RANGE("table", "table id", BDMFMON_PARM_NUMBER, 0, 1, 8),
        BDMFMON_MAKE_PARM_RANGE("thresh", "max threshold", BDMFMON_PARM_NUMBER, 0, 0, 64));
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
