/*
* <:copyright-BRCM:2013-2015:proprietary:standard
* 
*    Copyright (c) 2013-2015 Broadcom 
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
 :>
*/


#include "rdpa_ucast_ex.h"
#include "rdpa_int.h"

typedef struct
{
    uint32_t natc_control;
    rdd_fc_context_t rdd_ip_flow;
} natc_result_entry_t;



/**** L3 Unicast connection and context functions ****/

int rdd_connection_entry_add(rdd_ip_flow_t *add_connection, rdpa_traffic_dir direction)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t keyword[NATC_MAX_ENTRY_LEN] = {};
    natc_result_entry_t context_entry = {};
    uint8_t *ip_flow = (uint8_t *)&context_entry.rdd_ip_flow;
#if defined(USE_NATC_VAR_CONTEXT_LEN)
    uint8_t hash_keyword[NATC_MAX_ENTRY_LEN] = {};
#endif
    uint8_t tbl_idx = rdd_natc_l2l3_ucast_dir_to_tbl_idx(direction);

#if defined(USE_NATC_VAR_CONTEXT_LEN)
    /* + 2 for non-command list fields */
    rc = rdd_ip_class_key_entry_var_size_ctx_compose(add_connection->lookup_entry, keyword, add_connection->lookup_entry->lookup_port, hash_keyword,
        (rdd_ctx_size) add_connection->context_entry.fc_ucast_flow_context_entry.command_list_length_64 + 2);
#else
    rc = rdd_ip_class_key_entry_compose(add_connection->lookup_entry, keyword, add_connection->lookup_entry->lookup_port);
#endif
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Generating ip flow key entry failed, error %d\n", rc);

    add_connection->context_entry.fc_ucast_flow_context_entry.valid = 1;
    add_connection->context_entry.fc_ucast_flow_context_entry.connection_direction = direction;

    memcpy(ip_flow, &add_connection->context_entry, sizeof(rdd_fc_context_t));

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(ip_flow, FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_OFFSET);
#endif

#if defined(USE_NATC_VAR_CONTEXT_LEN)
    rc = drv_natc_key_result_entry_var_size_ctx_add(tbl_idx, hash_keyword, keyword, (uint8_t *)&context_entry,
        &add_connection->entry_index);
#else
    rc = drv_natc_key_result_entry_add(tbl_idx, keyword, (uint8_t *)&context_entry, &add_connection->entry_index);
#endif
    if (rc == BDMF_ERR_NOENT)
    {
       /* Hash table is full. Skip flow_adding but don't report an error. restore entry_index to INVALID */
       BDMF_TRACE_INFO("Adding connection to nat cache failed due to no_ent, rc = %d\n", rc);
       return BDMF_ERR_IGNORE;
    }
    else if (rc)
       BDMF_TRACE_RET(BDMF_ERR_PERM, "Adding connection to nat cache failed %d\n", rc);

    add_connection->entry_index = build_rdd_flow_id_for_natc(add_connection->entry_index, tbl_idx);

    return 0;
}

int rdd_connection_entry_delete(bdmf_index flow_entry_index)
{
    int rc = BDMF_ERR_OK;
    natc_flow_id_t natc_flow_id = {.word = flow_entry_index};

    rc = drv_natc_entry_delete(natc_flow_id.tbl_idx, natc_flow_id.natc_idx, 1, 1);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "IP flow deletion failed, error=%d\n", rc);

    return 0;
}

int rdd_connection_entry_get(rdpa_traffic_dir unused, uint32_t entry_index,
    rdpa_ip_flow_key_t *nat_cache_lkp_entry, bdmf_index *flow_entry_index)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t keyword[NATC_MAX_ENTRY_LEN] = {};
    uint8_t sub_tbl_id;
    bdmf_boolean valid = 0;
    natc_flow_id_t natc_flow_id = {.word = entry_index};

    rc = drv_natc_key_entry_get(natc_flow_id.tbl_idx, natc_flow_id.natc_idx, &valid, keyword);

    if (!valid)
        rc = BDMF_ERR_NOENT;

    if (rc)
        return rc;

    rdd_ip_class_key_entry_decompose(nat_cache_lkp_entry, &sub_tbl_id, keyword);

    if (sub_tbl_id != NATC_SUB_TBL_IDX_L3)
        return BDMF_ERR_NOENT;

    nat_cache_lkp_entry->lookup_port = nat_cache_lkp_entry->ingress_if;
    nat_cache_lkp_entry->dir = rdd_natc_tbl_idx_to_dir(natc_flow_id.tbl_idx);

    *flow_entry_index = entry_index;

    return 0;
}

int rdd_connection_entry_search(rdd_ip_flow_t *get_connection, rdpa_traffic_dir direction,
    bdmf_index *entry_index)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t keyword[NATC_MAX_ENTRY_LEN] = {};
    uint32_t hash_idx;
    uint8_t tbl_idx = rdd_natc_l2l3_ucast_dir_to_tbl_idx(direction);
    uint32_t natc_idx;

    rc = rdd_ip_class_key_entry_compose(get_connection->lookup_entry, keyword, get_connection->lookup_entry->lookup_port);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Generating ip flow key entry failed, error %d\n", rc);

    rc = drv_natc_key_idx_get(tbl_idx, keyword, &hash_idx, &natc_idx);

    *entry_index = build_rdd_flow_id_for_natc(natc_idx, tbl_idx);

    return rc;
}

int rdd_context_entry_get(bdmf_index flow_entry_index, rdd_fc_context_t *context)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    natc_result_entry_t context_entry = {};
    uint8_t *ip_flow = (uint8_t *)&context_entry.rdd_ip_flow;
    natc_flow_id_t natc_flow_id = {.word = flow_entry_index};

    rc = drv_natc_result_entry_get(natc_flow_id.tbl_idx, natc_flow_id.natc_idx, (uint8_t *)&context_entry);
    if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Retrieving context entry failed, error %d tbl_idx = %d natc_idx = %d bdmf_idx %ld\n", 
                       rc, natc_flow_id.tbl_idx, natc_flow_id.natc_idx, flow_entry_index);
    }

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(ip_flow, FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_OFFSET);
#endif

    if (!context_entry.rdd_ip_flow.fc_ucast_flow_context_entry.valid)
        rc = BDMF_ERR_NOENT;

    memcpy(context, ip_flow, sizeof(rdd_fc_context_t));

    return rc;
}

int rdd_context_entry_modify(rdd_fc_context_t *context, bdmf_index flow_entry_index)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    natc_result_entry_t context_entry = {};
    uint8_t *ip_flow = (uint8_t *)&context_entry.rdd_ip_flow;
    natc_flow_id_t natc_flow_id = {.word = flow_entry_index};

    memcpy(ip_flow, context, sizeof(rdd_fc_context_t));

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(ip_flow, FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_OFFSET);
#endif

    rc = drv_natc_result_entry_modify(natc_flow_id.tbl_idx, natc_flow_id.natc_idx, (uint8_t *)&context_entry);

    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Adding context entry failed, error %d\n", rc);

    return rc;
}

int rdd_context_entry_flwstat_get(bdmf_index flow_entry_index, rdd_fc_context_t *context_entry)
{
    return rdd_context_entry_get(flow_entry_index, context_entry);
}

int rdd_flow_counters_get(bdmf_index flow_entry_index, uint32_t *stat_packets, uint32_t *stat_bytes)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint64_t packets = 0;
    uint64_t bytes = 0;
    natc_flow_id_t natc_flow_id = {.word = flow_entry_index};

    rc = drv_natc_entry_counters_get(natc_flow_id.tbl_idx, natc_flow_id.natc_idx, &packets, &bytes);
    if (rc == BDMF_ERR_OK)
    {
        *stat_packets = (uint32_t)packets;
        *stat_bytes = (uint32_t)bytes;
    }

    return rc;
}

/**** L2 Unicast connection and context functions ****/

int rdd_l2_connection_entry_add(rdd_l2_flow_t *add_connection, rdpa_traffic_dir direction)
{
    bdmf_error_t rc;
    uint8_t keyword[NATC_MAX_ENTRY_LEN] = {};
    rdpa_l2_flow_key_exclude_fields_t l2_flow_key_exclude = RDPA_L2_KEY_EXCLUDE_FIELDS;
    natc_result_entry_t context_entry = {};
    uint8_t *ip_flow = (uint8_t *)&context_entry.rdd_ip_flow;
#if defined(USE_NATC_VAR_CONTEXT_LEN)
    uint8_t hash_keyword[NATC_MAX_ENTRY_LEN] = {};
    uint8_t tbl_idx = rdd_natc_l2l3_ucast_dir_to_tbl_idx(direction);

    rc = rdd_l2_flow_key_var_size_ctx_compose(l2_flow_key_exclude, add_connection->lookup_entry,
        keyword, hash_keyword, (rdd_ctx_size)
        add_connection->context_entry.fc_ucast_flow_context_entry.command_list_length_64 + 2); /* +2 for non-command list fields */
#else
    rc = rdd_l2_flow_key_compose(l2_flow_key_exclude, add_connection->lookup_entry, keyword);
#endif
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Generating L2 flow key entry failed, error %d\n", rc);

    add_connection->context_entry.fc_ucast_flow_context_entry.valid = 1;
    add_connection->context_entry.fc_ucast_flow_context_entry.connection_direction = direction;
    memcpy(ip_flow, &add_connection->context_entry, sizeof(rdd_fc_context_t));

#ifdef FIRMWARE_LITTLE_ENDIAN
    SWAPBYTES(ip_flow, FC_UCAST_FLOW_CONTEXT_ENTRY_COMMAND_LIST_OFFSET);
#endif

#if defined(USE_NATC_VAR_CONTEXT_LEN)
    rc = drv_natc_key_result_entry_var_size_ctx_add(tbl_idx, hash_keyword, keyword, (uint8_t *)&context_entry, &add_connection->entry_index);
#else
    rc = drv_natc_key_result_entry_add(tbl_idx, keyword, (uint8_t *)&context_entry, &add_connection->entry_index);
#endif
    if (rc == BDMF_ERR_NOENT)
    {
        /* Hash table is full. Skip flow_adding but don't report an error. restore entry_index to INVALID */
        BDMF_TRACE_INFO("Adding connection to nat cache failed due to no_ent, rc = %d\n", rc);
        return BDMF_ERR_IGNORE;
    }
    else if (rc)
    {
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Adding connection to nat cache failed %d\n", rc);
    }

    add_connection->entry_index = build_rdd_flow_id_for_natc(add_connection->entry_index, tbl_idx);

    return 0;
}

int rdd_l2_connection_entry_delete(bdmf_index flow_entry_index)
{
    return rdd_connection_entry_delete(flow_entry_index);
}

int rdd_l2_connection_entry_get(rdpa_traffic_dir unused, uint32_t entry_index,
    rdpa_l2_flow_key_t *nat_cache_lkp_entry, bdmf_index *flow_entry_index)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t keyword[NATC_MAX_ENTRY_LEN] = {};
    uint8_t sub_tbl_id;
    bdmf_boolean valid = 0;
    natc_flow_id_t natc_flow_id = {.word = entry_index};

    rc = drv_natc_key_entry_get(natc_flow_id.tbl_idx, natc_flow_id.natc_idx, &valid, keyword);

    if (!valid)
        rc = BDMF_ERR_NOENT;

    if (rc)
        return rc;

    rdd_l2_flow_key_decompose(nat_cache_lkp_entry, &sub_tbl_id, keyword);

    if (sub_tbl_id != NATC_SUB_TBL_IDX_L2)
        return BDMF_ERR_NOENT;

    nat_cache_lkp_entry->dir = rdd_natc_tbl_idx_to_dir(natc_flow_id.tbl_idx);

    *flow_entry_index = entry_index;

    return 0;
}

int rdd_l2_connection_entry_search(rdd_l2_flow_t *get_connection, rdpa_traffic_dir direction,
    bdmf_index *entry_index)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t keyword[NATC_MAX_ENTRY_LEN] = {};
    uint32_t hash_idx;
    rdpa_l2_flow_key_exclude_fields_t l2_flow_key_exclude = RDPA_L2_KEY_EXCLUDE_FIELDS;
    uint8_t tbl_idx = rdd_natc_l2l3_ucast_dir_to_tbl_idx(direction);
    uint32_t natc_idx;

    rc = rdd_l2_flow_key_compose(l2_flow_key_exclude, get_connection->lookup_entry, keyword);
    if (rc)
        BDMF_TRACE_RET(BDMF_ERR_PERM, "Generating ip flow key entry failed, error %d\n", rc);

    rc = drv_natc_key_idx_get(tbl_idx, keyword, &hash_idx, &natc_idx);

    *entry_index = build_rdd_flow_id_for_natc(natc_idx, tbl_idx);

    return rc;
}

int rdd_l2_context_entry_get(bdmf_index flow_entry_index, rdd_fc_context_t *context)
{
    return rdd_context_entry_get(flow_entry_index, context);
}

int rdd_l2_context_entry_flwstat_get(bdmf_index flow_entry_index, rdd_fc_context_t *context_entry)
{
    return rdd_context_entry_get(flow_entry_index, context_entry);
}

int rdd_l2_context_entry_modify(rdd_fc_context_t *context_entry, bdmf_index entry_index)
{
    return rdd_context_entry_modify(context_entry, entry_index);
}

int rdd_l2_flow_counters_get(bdmf_index flow_entry_index, uint32_t *packets, uint32_t *bytes)
{
    return rdd_flow_counters_get(flow_entry_index, packets, bytes);
}

