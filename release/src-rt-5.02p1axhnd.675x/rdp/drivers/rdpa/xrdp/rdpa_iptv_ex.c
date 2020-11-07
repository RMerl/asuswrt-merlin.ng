/*
 * <:copyright-BRCM:2015:proprietary:standard
 * 
 *    Copyright (c) 2015 Broadcom 
 *    All Rights Reserved
 * 
 *  This program is the proprietary software of Broadcom and/or its
 *  licensors, and may only be used, duplicated, modified or distributed pursuant
 *  to the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied), right
 *  to use, or waiver of any kind with respect to the Software, and Broadcom
 *  expressly reserves all rights in and to the Software and all intellectual
 *  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 * 
 *  Except as expressly set forth in the Authorized License,
 * 
 *  1. This program, including its structure, sequence and organization,
 *     constitutes the valuable trade secrets of Broadcom, and you shall use
 *     all reasonable efforts to protect the confidentiality thereof, and to
 *     use this information only in connection with your use of Broadcom
 *     integrated circuit products.
 * 
 *  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *     PERFORMANCE OF THE SOFTWARE.
 * 
 *  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *     LIMITED REMEDY.
 * :> 
 */

#include "rdpa_iptv_ex.h"
#include "rdd_ic_common.h"
#include "rdd_iptv.h"
#include "rdd_data_structures_auto.h"
#include "rdp_drv_rnr.h"
#include "rdp_drv_hash.h"
#include "rdp_drv_qm.h"
#include "rdp_drv_proj_cntr.h"
#include "rdpa_port_int.h"
#include "rdpa_system_ex.h"

#if !defined(BCM63158)
#include "rdp_drv_natc_counters.h"
#endif

#define MAC_ADDR_LOW(addr)     ((uint64_t)(addr) & 0xFFFFFFFF)
#define MAC_ADDR_HIGH(addr)    ((uint64_t)(addr) >> 32)
#define IPV4_MCAST_MAC_ADDR_PREFIX  (0x01005E000000)
#define IPV4_MCAST_MAC_ADDR_MASK    (0xFFFFFF800000)
#define IPV6_MCAST_MAC_ADDR_PREFIX  (0x333300000000)
#define IPV6_MCAST_MAC_ADDR_MASK    (0xFFFF00000000)

#if !defined(BCM63158)
extern struct bdmf_object *iptv_object;
#endif

#define END_OF_LIST ((uint32_t)-1)
static rdpa_iptv_stat_t accumulative_iptv_stat = {};

#define UPDATE_BUF_LENGTH_LEFT(pbuf, size_get, size_left) \
    do {                                                  \
        pbuf += size_get;                                 \
        size_left -= size_get;                            \
    } while (0)

typedef struct
{
    uint32_t counter_id;
    uint32_t offset;
} rdpa_iptv_cntrs_and_offsets_t;

static rdpa_iptv_cntrs_and_offsets_t iptv_cntrs_and_offsets[] =
{
    {COUNTER_IPTV_HASH_LKP_MISS_DROP, offsetof(rdpa_iptv_stat_t, iptv_lkp_miss_drop)},
    {COUNTER_IPTV_INVALID_CTX_ENTRY_DROP, offsetof(rdpa_iptv_stat_t, iptv_invalid_ctx_entry_drop)},
    {COUNTER_IPTV_SRC_IP_VID_LKP_MISS_DROP, offsetof(rdpa_iptv_stat_t, iptv_src_ip_vid_lkp_miss_drop)},
    {COUNTER_IPTV_FPM_ALLOC_NACK_DROP, offsetof(rdpa_iptv_stat_t, iptv_fpm_alloc_nack_drop)},
    {COUNTER_IPTV_FIRST_REPL_DISP_ALLOC_NACK_DROP, offsetof(rdpa_iptv_stat_t, iptv_first_repl_disp_nack_drop)},
    {COUNTER_IPTV_EXCEPTION_DROP, offsetof(rdpa_iptv_stat_t, iptv_exception_drop)},
    {COUNTER_IPTV_OTHER_REPL_DISP_ALLOC_NACK_DROP, offsetof(rdpa_iptv_stat_t, iptv_other_repl_disp_nack_drop)},
#ifdef BCM6858
    {COUNTER_IPTV_CONGESTION_DROP, offsetof(rdpa_iptv_stat_t, iptv_congestion_drop)},
#endif
    {END_OF_LIST, END_OF_LIST}
};

static void mcast_control_ip_filter_set(void)
{
    rnr_quad_parser_ip0 ipv4_filter, ipv6_filter;
    uint32_t quad_idx;

    /* Ipv4 multicast range - 224.0.0.0 to 224.0.0.255, mask = 255.255.255.0.
     * Runner expects to receive in host-order */
    ipv4_filter.ip_address = 0xe0000000;
    ipv4_filter.ip_address_mask = 0xFFFFFF00;
    ipv4_filter.ip_filter0_dip_en = 1;
    ipv4_filter.ip_filter0_valid = 1;

    /* Ipv6 multicast range - FF02:: to FF02:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF,
     * mask = FFFF::. By default, MS DW is used. */
    ipv6_filter.ip_address = 0xff020000;
    ipv6_filter.ip_address_mask = 0xffff0000;
    ipv6_filter.ip_filter0_dip_en = 1;
    ipv6_filter.ip_filter0_valid = 1;

    for (quad_idx = 0; quad_idx < NUM_OF_RNR_QUAD; quad_idx++) 
    {
        ag_drv_rnr_quad_parser_ip0_set(quad_idx, &ipv4_filter);
        ag_drv_rnr_quad_parser_ip1_set(quad_idx, &ipv6_filter);
    }
}

int rdpa_iptv_post_init_ex(void)
{
    uint16_t fpm_min_pool_size;
    bdmf_error_t rc;
#if !defined(BCM63158)
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
#endif
    rc = drv_qm_fpm_min_pool_get(&fpm_min_pool_size);
    if (rc)
        BDMF_TRACE_RET(rc, "Failed to get FPM min pool size\n");

    BDMF_TRACE_DBG("fpm min pool size is: %d\n", fpm_min_pool_size);
    rc = rdd_iptv_ctx_table_ddr_init(fpm_min_pool_size);
    if (rc)
        BDMF_TRACE_RET(rc, "Failed to initialize FPM pool\n");
#if !defined(BCM63158)
    rdd_multicast_filter_cfg(iptv_cfg->mcast_prefix_filter);
    rdd_iptv_status_cfg(1);
#endif
    mcast_control_ip_filter_set();
    memset(&accumulative_iptv_stat, 0, sizeof(rdpa_iptv_stat_t));

    return 0;
}

void rdpa_iptv_destroy_ex(void)
{
    rdd_iptv_ctx_table_ddr_destroy();
#if !defined(BCM63158)
    rdd_iptv_status_cfg(0);
#endif
}

int rdpa_iptv_cfg_rdd_update_ex(iptv_drv_priv_t *iptv_cfg, iptv_drv_priv_t *new_iptv_cfg, bdmf_boolean post_init)
{
    bdmf_error_t rc = BDMF_ERR_OK;

    /* Change only if not equal or init time */
    if (!rc && (iptv_cfg->lookup_method != new_iptv_cfg->lookup_method || post_init))
        rdd_iptv_lkp_method_set(new_iptv_cfg->lookup_method);

    return rc;
}

int rdpa_iptv_rdd_entry_search_ex(rdpa_iptv_channel_key_t *key, uint32_t *index)
{
    RDD_IPTV_HASH_LKP_ENTRY_DTS hash_key = {}; 
    RDD_IPTV_HASH_RESULT_ENTRY_DTS hash_ctx = {};
    uint32_t ctx_idx = 0;
    uint32_t head_ctx_idx = 0;
    hash_result_t hash_res = {};
    bdmf_error_t rc;
    bdmf_boolean skp;
    uint8_t cfg;

#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT)
    /* update rx_if from rdpa_if to rdd_vport here. */
    key->rx_if = (rdpa_if)rdpa_port_rdpa_if_to_vport(key->rx_if);
#endif
    /* lookup destination ip / mac in hash */
    rdd_iptv_hash_key_entry_compose(key, &hash_key);
    rc = drv_hash_find(HASH_TABLE_IPTV, (uint8_t *)&hash_key, &hash_res);
    if (rc)
        return rc;

    if (hash_res.match == HASH_MISS)
        return BDMF_ERR_NOENT;

    /* extract rdd iptv ctx index from hash result */
    rc = drv_hash_get_context(hash_res.match_index, HASH_TABLE_IPTV, NULL, (uint8_t *)&hash_ctx, &skp, &cfg);
    rdd_iptv_hash_result_entry_decompose(hash_ctx, &head_ctx_idx);
    if (rc)
        return rc;

    /* further lookup source ip and / or vid */
    rc = rdd_iptv_result_entry_find(key, head_ctx_idx, &ctx_idx);
    if (rc != BDMF_ERR_NOENT)
        *index = IPTV_CHANNEL_INDEX_GET(hash_res.match_index, ctx_idx);

    return rc;
}

int _rdpa_iptv_hash_key_get(uint32_t channel_index, rdd_iptv_entry_t *rdd_iptv_entry)
{
    int rc;
    RDD_IPTV_HASH_LKP_ENTRY_DTS hash_key = {};

    /* get dest ip / mac */
    rc = drv_hash_key_get(HASH_TABLE_IPTV, IPTV_KEY_INDEX_GET(channel_index), (uint8_t *)&hash_key);
    if (rc)
        return rc;

    rdd_iptv_hash_key_entry_decompose(hash_key, &rdd_iptv_entry->key);
    return 0;
}

int rdpa_iptv_rdd_entry_get_ex(uint32_t channel_index, rdd_iptv_entry_t *rdd_iptv_entry)
{
    int rc;
#if !defined(BCM63158)
    uint8_t ic_ctx[RDD_IPTV_RULE_BASED_RESULT_RULE_NUMBER] = {};
#else
    uint8_t *ic_ctx = (uint8_t *)&rdd_iptv_entry->gpe;
#endif
    rc = _rdpa_iptv_hash_key_get(channel_index, rdd_iptv_entry);
    if (rc)
        return rc;

    /* get src ip / mac and vid*/
    rc = rdd_iptv_result_entry_get(IPTV_CTX_INDEX_GET((uint32_t)channel_index), rdd_iptv_entry, ic_ctx);
    if (rc)
        return rc;

#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT)
#if defined(BCM_PON_XRDP) /* XXX may not needed */
    rdd_iptv_entry->key.rx_if = rdpa_port_rdpa_if_to_vport(rdd_iptv_entry->key.rx_if);
#else
    rdd_iptv_entry->key.rx_if = rdpa_port_vport_to_rdpa_if((rdd_vport_id_t)rdd_iptv_entry->key.rx_if);
#endif
#endif
#if !defined(BCM63158)
    rdd_ic_result_entry_decompose(ic_ctx, &rdd_iptv_entry->ic_context, NULL);
#endif
    return rc;
}

#if !defined(BCM63158)
static void _rdpa_iptv_wlan_to_host_check_modify(rdd_vport_id_t vport_vector, rdpa_forward_action *action)
{
    iptv_drv_priv_t *iptv_cfg = (iptv_drv_priv_t *)bdmf_obj_data(iptv_object);
    uint64_t wlan_ports_mask;
    
    if (!iptv_cfg->wlan_to_host)
        return;

    wlan_ports_mask = (1LL << rdpa_port_rdpa_if_to_vport(rdpa_if_wlan0)) |
        (1LL << rdpa_port_rdpa_if_to_vport(rdpa_if_wlan1)) |
        (1LL << rdpa_port_rdpa_if_to_vport(rdpa_if_wlan2));
    if (vport_vector & wlan_ports_mask)
        *action = rdpa_forward_action_host;
}
#endif

int rdpa_iptv_rdd_entry_add_ex(rdd_iptv_entry_t *rdd_iptv_entry, uint32_t *channel_idx)
{
    uint32_t ctx_idx, head_ctx_idx, cntr_id = 0;
    uint16_t key_idx;
    hash_result_t hash_res = {};
#if !defined(BCM63158)
    rdd_ic_context_t rdd_ic_ctx = {};
    uint8_t ic_ctx[RDD_IPTV_RULE_BASED_RESULT_RULE_NUMBER] = {};
#else
    uint8_t *ic_ctx = (uint8_t *)&rdd_iptv_entry->gpe;
#endif
    RDD_IPTV_HASH_LKP_ENTRY_DTS hash_key = {};
    RDD_IPTV_HASH_RESULT_ENTRY_DTS hash_ctx = {};
    bdmf_error_t rc;


    rdpa_cntr_id_alloc(CNTR_GROUP_IPTV_NATC, &cntr_id);
#if !defined(BCM63158)
    BDMF_TRACE_DBG("NAT CACHE alloc counter for key index=%d\n", (int)cntr_id);

    /* get ic context */
    rc = rdpa_ic_rdd_context_get(rdpa_dir_ds, rdd_iptv_entry->ic_context, &rdd_ic_ctx);
    if (rc)
        goto exit;

    /* Required explicitly for IPTV action vector */
    rdd_ic_ctx.forw_mode = rdpa_forwarding_mode_pkt;

    /* override action if wlan_to_host flag is set and there is wlan port in egress */
    _rdpa_iptv_wlan_to_host_check_modify(rdd_iptv_entry->egress_port_vector, &rdd_ic_ctx.action);

    rdd_ic_result_entry_compose(rdd_iptv_entry->ic_context, &rdd_ic_ctx, ic_ctx);
#endif
#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT)
#ifndef BCM_PON_XRDP
    /* update rx_if from rdpa_if to rdd_vport here.*/
    rdd_iptv_entry->key.rx_if = (rdpa_if)rdpa_port_rdpa_if_to_vport(rdd_iptv_entry->key.rx_if);
#endif

#endif
    /* lookup destination ip / mac in hash */
    rdd_iptv_hash_key_entry_compose(&rdd_iptv_entry->key, &hash_key);
    rc = drv_hash_find(HASH_TABLE_IPTV, (uint8_t *)&hash_key, &hash_res);
    if (rc)
        goto exit;

    /* check if first entry in linked list */
    if (hash_res.match == HASH_MISS)
    {
        /* add entry to DDR */
        rc = rdd_iptv_result_entry_add(rdd_iptv_entry, ic_ctx, NULL, cntr_id, &ctx_idx);
        head_ctx_idx = ctx_idx;
        if (rc)
            goto exit;

#ifdef CONFIG_BCM_CACHE_COHERENCY
        dma_wmb();
#endif
        /* add hash key + result */
        rdd_iptv_hash_result_entry_compose(ctx_idx, &hash_ctx);
        rc = drv_hash_rule_add(HASH_TABLE_IPTV, (uint8_t *)&hash_key, NULL, (uint8_t *)&hash_ctx, &key_idx);
        if (rc)
        {
            rdd_iptv_result_entry_delete(ctx_idx, &head_ctx_idx, &cntr_id);
            goto exit;
        }
    }
    else
    {
        bdmf_boolean skp;
        uint8_t cfg;

        key_idx = hash_res.match_index;
        rc = drv_hash_get_context(hash_res.match_index, HASH_TABLE_IPTV, NULL, (uint8_t *)&hash_ctx, &skp, &cfg);
        rdd_iptv_hash_result_entry_decompose(hash_ctx, &head_ctx_idx);
        rc = rc ? rc : rdd_iptv_result_entry_add(rdd_iptv_entry, ic_ctx, &head_ctx_idx, cntr_id, &ctx_idx);
        if (rc)
            goto exit;

#ifdef CONFIG_BCM_CACHE_COHERENCY
        dma_wmb();
#endif
        /* added index is now the head of the linked list */
        if (head_ctx_idx == ctx_idx)
        {
            rdd_iptv_hash_result_entry_compose(ctx_idx, &hash_ctx);
            rc = drv_hash_modify_context(HASH_TABLE_IPTV, key_idx, NULL, (uint8_t *)&hash_ctx);
        }
    }

exit:
    if (!rc)
    {
        *channel_idx = IPTV_CHANNEL_INDEX_GET(key_idx, ctx_idx);
        RDD_BTRACE("** Added channel, Key index in HASH %d, Context index %d, result encoded index %d\n",
            key_idx, ctx_idx, *channel_idx);
    }
    else
        rdpa_cntr_id_dealloc(CNTR_GROUP_IPTV_NATC, NONE_CNTR_SUB_GROUP_ID, cntr_id);
    return rc;
}

int rdpa_iptv_result_entry_modify(uint32_t channel_idx, rdd_iptv_entry_t *rdd_iptv_entry)
{
#if !defined(BCM63158)
    uint8_t ic_ctx[RDD_IPTV_RULE_BASED_RESULT_RULE_NUMBER] = {};
    rdd_ic_context_t rdd_ic_ctx = {};
#else
    uint8_t *ic_ctx = (uint8_t *)&rdd_iptv_entry->gpe;
#endif
    bdmf_error_t rc = BDMF_ERR_OK;

#if !defined(BCM63158)
    /* get ic context */
    rc = rdpa_ic_rdd_context_get(rdpa_dir_ds, rdd_iptv_entry->ic_context, &rdd_ic_ctx);
    if (rc)
        return rc;

    /* Required explicitly for IPTV action vector */
    rdd_ic_ctx.forw_mode = rdpa_forwarding_mode_pkt;
    rdd_ic_result_entry_compose(rdd_iptv_entry->ic_context, &rdd_ic_ctx, ic_ctx);
#endif
#if defined(CONFIG_RUNNER_IPTV_LKUP_KEY_INCLUDE_SRC_PORT)
    /* update rx_if from rdpa_if to rdd_vport here. */
    rdd_iptv_entry->key.rx_if = (rdpa_if)rdpa_port_rdpa_if_to_vport(rdd_iptv_entry->key.rx_if);
#endif
    rdd_iptv_result_entry_modify(rdd_iptv_entry, ic_ctx, IPTV_CTX_INDEX_GET(channel_idx));

    return rc;
}

int rdpa_iptv_rdd_entry_delete_ex(uint32_t channel_idx)
{
    uint32_t head_ctx_idx, next_ctx_idx, cntr_id = 0;
    RDD_IPTV_HASH_RESULT_ENTRY_DTS hash_ctx = {};
    bdmf_error_t rc;
    bdmf_boolean skp;
    uint8_t cfg;

    /* get head ctx index from hash */
    rc = drv_hash_get_context(IPTV_KEY_INDEX_GET(channel_idx), HASH_TABLE_IPTV, NULL, (uint8_t *)&hash_ctx, &skp, &cfg);
    if (rc)
        return rc;
    rdd_iptv_hash_result_entry_decompose(hash_ctx, &head_ctx_idx);

    /* first element in the DDR ctx list need the update the hash result with the new head ctx idx */
    if (head_ctx_idx == IPTV_CTX_INDEX_GET(channel_idx))
    {
        rc = rdd_iptv_result_entry_next_idx_get(IPTV_CTX_INDEX_GET(channel_idx), &next_ctx_idx);
        if (rc)
            return rc;

        /* no more elements */
        if (next_ctx_idx == IPTV_CTX_ENTRY_IDX_NULL)
            rc = drv_hash_rule_remove_index(HASH_TABLE_IPTV, IPTV_KEY_INDEX_GET(channel_idx));
        else
        {
            rdd_iptv_hash_result_entry_compose(next_ctx_idx, &hash_ctx);
            rc = drv_hash_modify_context(HASH_TABLE_IPTV, IPTV_KEY_INDEX_GET(channel_idx), NULL, (uint8_t *)&hash_ctx);
        }
    }

    rc = rc ? rc : rdd_iptv_result_entry_delete(IPTV_CTX_INDEX_GET(channel_idx), &head_ctx_idx, &cntr_id);
    if (!rc)
        rdpa_cntr_id_dealloc(CNTR_GROUP_IPTV_NATC, NONE_CNTR_SUB_GROUP_ID, cntr_id);
    return rc;
}

#if !defined(BCM63158)
int rdpa_iptv_ic_result_add_ex(mcast_result_entry_t *entry)
{
    rdd_ic_context_t ic_ctx = {};
    bdmf_error_t rc;

    rc = rdpa_map_to_rdd_classifier(rdpa_dir_ds, &entry->mcast_result, &ic_ctx, 1, 1, RDPA_IC_TYPE_FLOW, 0);
    return rc ? rc : rdpa_ic_rdd_context_cfg(rdpa_dir_ds, entry->mcast_result_idx, &ic_ctx);
}

void rdpa_iptv_ic_result_delete_ex(uint32_t mcast_result_idx, rdpa_traffic_dir dir)
{
    rdpa_ic_result_delete(mcast_result_idx, dir);
}
#endif

int rdpa_iptv_stat_read_ex(struct bdmf_attr *ad, bdmf_index index, void *val, uint32_t size)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};
    rdpa_iptv_stat_t *stat = (rdpa_iptv_stat_t *)val;
    int idx = 0;

    memset(stat, 0, sizeof(rdpa_iptv_stat_t));
    rc = drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_RX_IPTV_VALID_PKT, cntr_arr);
    if (rc)
    {
        cntr_arr[0] = 0;
        BDMF_TRACE_ERR("Error reading counter GENERAL_COUNTER_RX_IPTV_VALID_PKT, rc %d\n", rc);
    }
    rdpa_common_update_cntr_results_uint32(&(stat->rx_valid_pkt), &(accumulative_iptv_stat.rx_valid_pkt), 0, cntr_arr[0]);

    while (iptv_cntrs_and_offsets[idx].counter_id != END_OF_LIST)
    {
        uint32_t counter_id = iptv_cntrs_and_offsets[idx].counter_id;
        uint16_t cntr_val;
        int rc_inner;

        rc_inner = drv_cntr_various_counter_get(counter_id, &cntr_val);
        if (rc_inner)
        {
            rc = rc_inner;
            BDMF_TRACE_ERR("Error reading counter %d, rc %d\n", counter_id, rc);
            cntr_val = 0; /* will set to resulting statistic the latest good known*/
                          /* accumulated value*/
        }
        rdpa_common_update_cntr_results_uint32(stat, &accumulative_iptv_stat,
            iptv_cntrs_and_offsets[idx].offset, (uint32_t)cntr_val);

        idx++;
    }

    drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_RX_IPTV_VALID_BYTES, cntr_arr);
    if (rc)
    {
        cntr_arr[0] = 0;
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "can't read RX_IPTV_VALID_BYTES counter, error = %d\n", rc);
    }
    
    rdpa_common_update_cntr_results_uint32(&(stat->rx_valid_bytes), &(accumulative_iptv_stat.rx_valid_bytes), 0, cntr_arr[0]);

    rc = drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_IPTV_TOTAL_DROP_PKT, cntr_arr);
    if (rc)
    {
        cntr_arr[0] = 0;
        BDMF_TRACE_ERR("Error reading counter GENERAL_COUNTER_IPTV_TOTAL_DROP_PKT, rc %d\n", rc);
    }
    rdpa_common_update_cntr_results_uint32(&(stat->discard_pkt), &(accumulative_iptv_stat.discard_pkt), 0, cntr_arr[0]);

    rc = drv_cntr_counter_read(CNTR_GROUP_GENERAL, GENERAL_COUNTER_IPTV_TOTAL_DROP_BYTES, cntr_arr);
    if (rc)
    {
        cntr_arr[0] = 0;
        BDMF_TRACE_ERR("Error reading counter GENERAL_COUNTER_IPTV_TOTAL_DROP_BYTES, rc %d\n", rc);
    }
    rdpa_common_update_cntr_results_uint32(&(stat->discard_bytes), &(accumulative_iptv_stat.discard_bytes), 0, cntr_arr[0]);

    stat->rx_crc_error_pkt = 0;

    return rc;
}

/* "stat" attribute "write" callback */
int iptv_attr_stat_write_ex(struct bdmf_object *mo, struct bdmf_attr *ad, bdmf_index index, const void *val,
    uint32_t size)
{
    bdmf_error_t rc = BDMF_ERR_OK;

    memset(&accumulative_iptv_stat, 0, sizeof(rdpa_iptv_stat_t));
    return rc;
}

bdmf_error_t rdpa_iptv_channel_rdd_pm_stat_get_ex(bdmf_index channel_index, rdpa_stat_t *pm_stat)
{
#ifndef RDP_SIM
    int rc;
    uint32_t cntr_id = 0;
    uint32_t rx_cntr_arr[MAX_NUM_OF_COUNTERS_PER_READ] = {};

    rc = rdd_iptv_cntr_idx_get(IPTV_CTX_INDEX_GET(channel_index), &cntr_id);
    rc = rc ? rc : drv_cntr_counter_read(CNTR_GROUP_IPTV_NATC, cntr_id, rx_cntr_arr);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Failed to read iptv counter of channel 0x%x, rdd error %d\n",
            (uint32_t)channel_index, rc);
    }

    pm_stat->packets = rx_cntr_arr[0];
    pm_stat->bytes = rx_cntr_arr[1];

#else
    memset(pm_stat, 0, sizeof(rdpa_stat_t));
#endif
    return BDMF_ERR_OK;
}


#define _VLAN_ACTION_FMT_  "_vlan_action={vlan_action/dir=ds,index="
bdmf_error_t rdpa_iptv_channel_mcast_result_val_to_str_ex(const void *val, char *sbuf,
    uint32_t _size, rdd_ic_context_t *rdd_classify_ctx, rdpa_ports ports)
{
    rdpa_ic_result_t *mcast_result = (rdpa_ic_result_t *)val;
    int size = (int)_size;
    rdpa_if port = 0;
    int size_get = 0, size_left = size;
    char *pbuf = sbuf;

    size_get = snprintf(pbuf, size_left, "{qos_method=%s,forward_action=%s,tc=%d",
              bdmf_attr_get_enum_text_hlp(&rdpa_qos_method_enum_table, mcast_result->qos_method),
              bdmf_attr_get_enum_text_hlp(&rdpa_forward_action_enum_table, mcast_result->action),
              rdd_classify_ctx->priority);

    UPDATE_BUF_LENGTH_LEFT(pbuf, size_get, size_left);

    if (mcast_result->dscp_remark)
    {
        size_get = snprintf(pbuf, size_left, ",dscp_remark=%d,dscp_val=%d", mcast_result->dscp_remark, mcast_result->dscp_val);
        UPDATE_BUF_LENGTH_LEFT(pbuf, size_get, size_left);
    }

    if (mcast_result->policer)
    {
        size_get = snprintf(pbuf, size_left, ",policer={%s}", mcast_result->policer->name);
        UPDATE_BUF_LENGTH_LEFT(pbuf, size_get, size_left);
    }
    
    if (mcast_result->action_vec & ~(1 << rdpa_ic_act_none))
    {
        uint32_t idx = 0;
        bdmf_boolean first_action = 1;

        size_get = snprintf(pbuf, size_left, ",action_vector={");
        UPDATE_BUF_LENGTH_LEFT(pbuf, size_get, size_left);
        while  (rdpa_ic_act_vect_enum_table.values[idx].name != NULL)
        {
            if (mcast_result->action_vec & (1 << rdpa_ic_act_vect_enum_table.values[idx].val))
            {
                if (!first_action)
                {
                    size_get = snprintf(pbuf, size_left, "+");
                    UPDATE_BUF_LENGTH_LEFT(pbuf, size_get, size_left);
                }
                else
                    first_action = 0;

                size_get = snprintf(pbuf, size_left, "%s", bdmf_attr_get_enum_text_hlp(&rdpa_ic_act_vect_enum_table,
                    rdpa_ic_act_vect_enum_table.values[idx].val));
                UPDATE_BUF_LENGTH_LEFT(pbuf, size_get, size_left);
            }
            idx++;
        }
        size_get = snprintf(pbuf, size_left, "}");
        UPDATE_BUF_LENGTH_LEFT(pbuf, size_get, size_left);
    }

    if (rdd_classify_ctx->service_queue_mode)
        size_get = snprintf(pbuf, size_left, ",service_queue_id=%d,per_port_vlan_actions={", egress_tm_svcq_queue_id_get(rdd_classify_ctx->service_queue));
    else
        size_get = snprintf(pbuf, size_left, ",service_queue_id=disable,per_port_vlan_actions={");

    while (ports)
    {
        port = rdpa_port_get_next(&ports);
        UPDATE_BUF_LENGTH_LEFT(pbuf, size_get, size_left);

        /* We assume enough space for vlan action index (up to 128, i.e. 3 chars), and rdpa_if name (up to 5 chars).
         * Anyway, we don't print more then size_left chars, so in case there will be no enough space we will fail
         * either in next iteration or when adding command suffix out of the loop */
        if (size_left > sizeof(_VLAN_ACTION_FMT_) + 10)
        {
            size_get = snprintf(pbuf, size_left, "%s" _VLAN_ACTION_FMT_ "%d},",
                bdmf_attr_get_enum_text_hlp(&rdpa_if_enum_table, port),
                rdd_classify_ctx->vlan_command_id.ds_vlan_command[port-rdpa_if_lan0]);
        }
        else
            return BDMF_ERR_INTERNAL; /* not enough space in buffer to print there */
    }
    size_get -= 1; /*to override last ',' at the end of the line*/
    UPDATE_BUF_LENGTH_LEFT(pbuf, size_get, size_left);
    if (size_left > 2)
        size_get = snprintf(pbuf, size_left, "}}");
    else
        return BDMF_ERR_INTERNAL; /*not enough space in buffer to print "}}" */

    UPDATE_BUF_LENGTH_LEFT(pbuf, size_get, size_left);
    /*if were printed the same amount of characters or above then buffer length*/
    /*    report ERROR*/
    if (size_left <= 0)
    	return BDMF_ERR_INTERNAL;

    return BDMF_ERR_OK;
}

bdmf_error_t rdpa_iptv_lkp_miss_action_write_ex(rdpa_forward_action new_lookup_miss_action)
{
    return rdd_iptv_lkp_miss_action_cfg(new_lookup_miss_action);
}
