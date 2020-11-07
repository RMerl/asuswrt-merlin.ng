/*
   <:copyright-BRCM:2014-2016:DUAL/GPL:standard
   
      Copyright (c) 2014-2016 Broadcom 
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

#include "rdd.h"
#include "bdmf_system.h"
#include "rdd_tuple_lkp.h"
#include "rdd_l4_filters.h"
#include "rdd_ip_flow.h"

extern uint8_t* g_ddr_runner_base_addr;
extern uint32_t g_ddr_runner_base_addr_phys;
extern uint32_t g_runner_tables_offset;

#if defined(FIRMWARE_INIT)
extern uint32_t ds_tuple_lkp_table_ptr;
extern uint32_t us_tuple_lkp_table_ptr;
extern uint32_t g_context_table_ptr;
#endif

extern int rdd_context_entry_add(rdpa_ip_flow_key_t *key, void *ctx, uint32_t ip_flow_index, uint32_t *context_index);
extern void rdd_context_entry_free(uint32_t index);
extern int rdd_get_ip_flow_index_by_context_index(uint32_t context_index, uint32_t *ip_flow_index);

extern bdmf_fastlock int_lock_irq;

int rdd_tuple_lkp_init(const rdd_module_t *module)
{
    RDD_TUPLE_LKP_CFG_DTS cfg_entry = {};
    int i;
    uint32_t ddr_tables_base = g_ddr_runner_base_addr + g_runner_tables_offset;
    uint32_t tuple_lkp_table = ((rdd_tuple_lkp_params_t *)module->params)->dir == rdpa_dir_ds ?
        ddr_tables_base + DS_TUPLE_LKP_TABLE_ADDRESS : ddr_tables_base + US_TUPLE_LKP_TABLE_ADDRESS;

    ((rdd_tuple_lkp_params_t *)module->params)->tuple_lkp_table =
        ((rdd_tuple_lkp_params_t *)module->params)->dir == rdpa_dir_ds ?
        DS_TUPLE_LKP_TABLE_PTR : US_TUPLE_LKP_TABLE_PTR;

    cfg_entry.res_offset = module->res_offset;
    cfg_entry.context_size = module->context_size;
    cfg_entry.context_offset = module->context_offset;
    cfg_entry.counter_cam_semaphore_address = ((rdd_tuple_lkp_params_t *)module->params)->semaphore_address;
    cfg_entry.context_index_cam_free_index = 0;
    cfg_entry.lookup_table_ptr = RDD_RSV_VIRT_TO_PHYS(g_ddr_runner_base_addr, g_ddr_runner_base_addr_phys,
        tuple_lkp_table);
    cfg_entry.context_table_ptr = RDD_RSV_VIRT_TO_PHYS(g_ddr_runner_base_addr, g_ddr_runner_base_addr_phys,
        ddr_tables_base + CONTEXT_TABLE_ADDRESS);
    cfg_entry.context_index_cam_ptr = ((rdd_tuple_lkp_params_t *)module->params)->context_index_cam_address;
    cfg_entry.counter_table_ptr = ((rdd_tuple_lkp_params_t *)module->params)->counter_table_address;

    MWRITE_GROUP_BLOCK_32(module->group, module->cfg_ptr, (uint32_t *)&cfg_entry, sizeof(RDD_TUPLE_LKP_CFG_DTS));

    for (i = 0; i < ((rdd_tuple_lkp_params_t *)(module->params))->context_index_cam_size; i++)
    {
        GROUP_MWRITE_I_16(module->group, ((rdd_tuple_lkp_params_t *)(module->params))->context_index_cam_ptr,
            CAM_STOP_VALUE, i);
    }

    rdd_l4_filters_init(((rdd_tuple_lkp_params_t *)module->params)->dir);

    return 0;
}

static inline int _rdd_tuple_lkp_ipv6_addr_crc_calc(bdmf_ip_t *ip, uint32_t *crc)
{
#ifndef FIRMWARE_INIT
    int rc;
    uint32_t *ipv6_buffer_ptr;
    unsigned long flags;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    ipv6_buffer_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + HASH_BUFFER_ADDRESS);

    MWRITE_BLK_8(ipv6_buffer_ptr, ip->addr.ipv6.data, 16);

    rc = rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_IPV6_CRC_GET, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, 0, 0, 0, 1);
        
    if (rc)
    {
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        return rc;
    }

    *crc = *(volatile uint32_t *)ipv6_buffer_ptr;

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
#else
    *crc = ip->addr.ipv4;
#endif
    return 0;
}

static inline int tuple_lkp_key_cmp(RDD_TUPLE_LKP_ENTRY_DTS *entry, rdd_ip_flow_key *key)
{
    rdd_ip_flow_key _key;

    RDD_TUPLE_LKP_ENTRY_PROTOCOL_READ(_key.prot, entry);
    RDD_TUPLE_LKP_ENTRY_KEY_EXTEND_READ(_key.family, entry);
    RDD_TUPLE_LKP_ENTRY_SRC_PORT_READ(_key.src_port, entry);
    RDD_TUPLE_LKP_ENTRY_DST_PORT_READ(_key.dst_port, entry);
    RDD_TUPLE_LKP_ENTRY_SRC_IP_READ(_key.src_ip, entry);
    RDD_TUPLE_LKP_ENTRY_DST_IP_READ(_key.dst_ip, entry);

    if (_key.prot == key->prot && _key.family == key->family &&
        _key.src_port == key->src_port && _key.dst_port == key->dst_port &&
        _key.src_ip == key->src_ip && _key.dst_ip == key->dst_ip)
    {
        return 0;
    }
    return BDMF_ERR_NOENT;
}

static inline int _rdd_tuple_lkp_find_free_entry(uint32_t hash_index, RDD_TUPLE_LKP_TABLE_DTS *tuple_table,
    rdd_ip_flow_key *key, uint32_t *index)
{
    RDD_TUPLE_LKP_ENTRY_DTS *tuple_entry;
    uint32_t valid, i;

    for (i = 0; i < RDD_TUPLE_LKP_TABLE_BUCKET_SIZE; i++)
    {
        tuple_entry = &tuple_table->entry[hash_index + i];

        RDD_TUPLE_LKP_ENTRY_VALID_READ(valid, tuple_entry);

        if (!valid)
            break;

        /* if entry is valid, check if it matches entry being added */
        if (!tuple_lkp_key_cmp(tuple_entry, key))
           BDMF_TRACE_RET(BDMF_ERR_ALREADY, "Lookup entry exists\n");
    }

    *index = i;

    return 0;
}

static inline int _rdd_tuple_lkp_find_entry(uint32_t hash_index, RDD_TUPLE_LKP_TABLE_DTS *tuple_table,
    rdd_ip_flow_key *key, uint32_t *index)
{
    RDD_TUPLE_LKP_ENTRY_DTS *tuple_entry;
    uint32_t valid, i;

    for (i = 0; i < RDD_TUPLE_LKP_TABLE_BUCKET_SIZE; i++)
    {
        tuple_entry = &tuple_table->entry[hash_index + i];

        RDD_TUPLE_LKP_ENTRY_VALID_READ(valid, tuple_entry);

        if (!valid)
            continue;

        /* if entry is valid, check if it matches */
        if (!tuple_lkp_key_cmp(tuple_entry, key))
        {
            *index = i;
            return 0;
        }
    }

    return BDMF_ERR_NOENT;
}

static inline int _rdd_ip_flow_key_get(rdpa_ip_flow_key_t *key, rdd_ip_flow_key *rdd_key)
{
    int rc;
    uint32_t crc;

    rdd_key->prot = key->prot;
    rdd_key->src_port = key->src_port;
    rdd_key->dst_port = key->dst_port;
    rdd_key->family = key->dst_ip.family;

    if (key->dst_ip.family == bdmf_ip_family_ipv4)
    {
        rdd_key->src_ip = key->src_ip.addr.ipv4;
        rdd_key->dst_ip = key->dst_ip.addr.ipv4;
    }
    else
    {
        rc = _rdd_tuple_lkp_ipv6_addr_crc_calc(&key->src_ip, &crc);

        if (rc)
            return rc;

        rdd_key->src_ip = crc;

        rc = _rdd_tuple_lkp_ipv6_addr_crc_calc(&key->dst_ip, &crc);

        if (rc)
            return rc;

        rdd_key->dst_ip = crc;
    }

    return 0;
}

static inline uint32_t _rdd_tuple_lkp_crc_get(rdd_ip_flow_key *key)
{
    uint8_t entry_bytes[12];

    entry_bytes[0] = (key->src_port >> 8) & 0xFF;
    entry_bytes[1] = key->src_port & 0xFF;
    entry_bytes[2] = (key->dst_port >> 8) & 0xFF;
    entry_bytes[3] = key->dst_port & 0xFF;
    entry_bytes[4] = (key->src_ip >> 24) & 0xFF;
    entry_bytes[5] = (key->src_ip >> 16) & 0xFF;
    entry_bytes[6] = (key->src_ip >> 8) & 0xFF;
    entry_bytes[7] = key->src_ip & 0xFF;
    entry_bytes[8] = (key->dst_ip >> 24) & 0xFF;
    entry_bytes[9] = (key->dst_ip >> 16) & 0xFF;
    entry_bytes[10] = (key->dst_ip >> 8) & 0xFF;
    entry_bytes[11] = key->dst_ip & 0xFF;

    return crcbitbybit(entry_bytes, 12, 0, key->prot, RDD_CRC_TYPE_32);
}

int rdd_ip_flow_add(rdd_module_t *module, rdpa_ip_flow_key_t *key, void *context, uint32_t *context_index)
{
    RDD_TUPLE_LKP_TABLE_DTS *tuple_table;
    RDD_TUPLE_LKP_ENTRY_DTS *tuple_entry;
    rdd_ip_flow_key rdd_key;
    uint32_t tuple_index, hash_index, match_index;
    uint32_t prev_bucket_last_entry_index, bucket_overflow_counter, overflow_bit;
    int rc;
    unsigned long flags = 0;

    rc = _rdd_ip_flow_key_get(key, &rdd_key);

    if (rc)
        return rc;

    hash_index = (_rdd_tuple_lkp_crc_get(&rdd_key) &
        (RDD_TUPLE_LKP_TABLE_SIZE / RDD_TUPLE_LKP_TABLE_BUCKET_SIZE - 1)) * RDD_TUPLE_LKP_TABLE_BUCKET_SIZE;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    tuple_table = (RDD_TUPLE_LKP_TABLE_DTS *)((rdd_tuple_lkp_params_t *)module->params)->tuple_lkp_table;

    /* try to find free slot in matched bucket */
    rc = _rdd_tuple_lkp_find_free_entry(hash_index, tuple_table, &rdd_key, &match_index);

    if (rc)
        goto exit;

    if (match_index == RDD_TUPLE_LKP_TABLE_BUCKET_SIZE)
    {
        /* try to find free slot in consecutive bucket */
        hash_index = (hash_index + RDD_TUPLE_LKP_TABLE_BUCKET_SIZE) & (RDD_TUPLE_LKP_TABLE_SIZE - 1);

        rc = _rdd_tuple_lkp_find_free_entry(hash_index, tuple_table, &rdd_key, &match_index);

        if (rc)
            goto exit;

        if (match_index == RDD_TUPLE_LKP_TABLE_BUCKET_SIZE)
        {
            bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "Can't add tuple, bucket full\n");
        }

        overflow_bit = 0x1;
    }
    else
    {
        overflow_bit = 0;
    }

    tuple_index = hash_index + match_index;

    rc = rdd_context_entry_add(key, context, tuple_index, context_index);

    if (rc)
    {
        BDMF_TRACE_ERR("Failed to add context entry, err %d\n", rc);
        goto exit;
    }

    if (overflow_bit)
    {
        /* increment bucket_overflow_counter in the last entry of the previous bucket */
        prev_bucket_last_entry_index = hash_index == 0 ? RDD_TUPLE_LKP_TABLE_SIZE - 1 : hash_index - 1;

        tuple_entry = &(tuple_table->entry[prev_bucket_last_entry_index]);

        RDD_TUPLE_LKP_ENTRY_BUCKET_OVERFLOW_COUNTER_READ(bucket_overflow_counter, tuple_entry);
        bucket_overflow_counter++;
        RDD_TUPLE_LKP_ENTRY_BUCKET_OVERFLOW_COUNTER_WRITE(bucket_overflow_counter, tuple_entry);
    }

    /* write key */
    tuple_entry = &(tuple_table->entry[tuple_index]);

    RDD_TUPLE_LKP_ENTRY_DST_IP_WRITE(rdd_key.dst_ip, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_SRC_IP_WRITE(rdd_key.src_ip, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_DST_PORT_WRITE(rdd_key.dst_port, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_SRC_PORT_WRITE(rdd_key.src_port, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_PROTOCOL_WRITE(rdd_key.prot, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_KEY_EXTEND_WRITE(rdd_key.family, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_CONTEXT_INDEX_WRITE(*context_index, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_OVERFLOW_WRITE(overflow_bit, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_VALID_WRITE(0x1, tuple_entry);

    rc = 0;

exit:
    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    return rc;
}

int rdd_ip_flow_delete(rdd_module_t *module, uint32_t context_index)
{
    RDD_TUPLE_LKP_TABLE_DTS *tuple_table;
    RDD_TUPLE_LKP_ENTRY_DTS *tuple_entry, *prev_tuple_entry;
    uint32_t ip_flow_index, prev_ip_flow_index;
    uint32_t valid, ip_flow_context_index, overflow, bucket_overflow_counter;
    int rc;
    unsigned long flags;

    rc = rdd_get_ip_flow_index_by_context_index(context_index, &ip_flow_index);

    if (rc)
        return rc;

    tuple_table = (RDD_TUPLE_LKP_TABLE_DTS *)((rdd_tuple_lkp_params_t *)module->params)->tuple_lkp_table;
    tuple_entry = &(tuple_table->entry[ip_flow_index]);

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    RDD_TUPLE_LKP_ENTRY_VALID_READ(valid, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_CONTEXT_INDEX_READ(ip_flow_context_index, tuple_entry);

    if (!valid || ip_flow_context_index != context_index)
    {
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        return BDMF_ERR_NOENT;
    }

    RDD_TUPLE_LKP_ENTRY_OVERFLOW_READ(overflow, tuple_entry);

    if (overflow)
    {
        /* decrement bucket_overflow_counter in the last entry of the previous bucket */
        prev_ip_flow_index = (ip_flow_index < RDD_TUPLE_LKP_TABLE_BUCKET_SIZE) ? RDD_TUPLE_LKP_TABLE_SIZE - 1 :
            (ip_flow_index & ~(RDD_TUPLE_LKP_TABLE_BUCKET_SIZE - 1)) - 1;

        prev_tuple_entry = &(tuple_table->entry[prev_ip_flow_index]);

        RDD_TUPLE_LKP_ENTRY_BUCKET_OVERFLOW_COUNTER_READ(bucket_overflow_counter, prev_tuple_entry);
        bucket_overflow_counter--;
        RDD_TUPLE_LKP_ENTRY_BUCKET_OVERFLOW_COUNTER_WRITE(bucket_overflow_counter, prev_tuple_entry);
    }

    RDD_TUPLE_LKP_ENTRY_VALID_WRITE(0, tuple_entry);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);

    rdd_context_entry_free(context_index);

    return 0;
}

int rdd_ip_flow_find(rdd_module_t *module, rdpa_ip_flow_key_t *key, uint32_t *context_index)
{
    RDD_TUPLE_LKP_TABLE_DTS *tuple_table;
    RDD_TUPLE_LKP_ENTRY_DTS *tuple_entry;
    rdd_ip_flow_key rdd_key;
    uint32_t hash_index, match_index;
    int rc;
    unsigned long flags = 0;

    rc = _rdd_ip_flow_key_get(key, &rdd_key);

    if (rc)
        return rc;

    hash_index = _rdd_tuple_lkp_crc_get(&rdd_key) &
        (RDD_TUPLE_LKP_TABLE_SIZE / RDD_TUPLE_LKP_TABLE_BUCKET_SIZE - 1) * RDD_TUPLE_LKP_TABLE_BUCKET_SIZE;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    tuple_table = (RDD_TUPLE_LKP_TABLE_DTS *)((rdd_tuple_lkp_params_t *)module->params)->tuple_lkp_table;

    /* try to find free slot in matched bucket */
    rc = _rdd_tuple_lkp_find_entry(hash_index, tuple_table, &rdd_key, &match_index);

    if (rc == BDMF_ERR_NOENT)
    {
        /* try to find free slot in consecutive bucket */
        hash_index = (hash_index + RDD_TUPLE_LKP_TABLE_BUCKET_SIZE) & (RDD_TUPLE_LKP_TABLE_SIZE - 1);

        rc = _rdd_tuple_lkp_find_entry(hash_index, tuple_table, &rdd_key, &match_index);

        if (rc)
            goto exit;
    }

    tuple_entry = &tuple_table->entry[hash_index + match_index];

    RDD_TUPLE_LKP_ENTRY_CONTEXT_INDEX_READ(*context_index, tuple_entry);

    rc = 0;

exit:
    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    return rc;
}

int rdd_ip_flow_get(rdd_module_t *module, uint32_t ip_flow_index, rdpa_ip_flow_key_t *key, uint32_t *context_index)
{
    RDD_TUPLE_LKP_TABLE_DTS *tuple_table;
    RDD_TUPLE_LKP_ENTRY_DTS *tuple_entry;
    uint32_t valid;

    tuple_table = (RDD_TUPLE_LKP_TABLE_DTS *)((rdd_tuple_lkp_params_t *)module->params)->tuple_lkp_table;
    tuple_entry = &tuple_table->entry[ip_flow_index];

    RDD_TUPLE_LKP_ENTRY_VALID_READ(valid, tuple_entry);

    if (!valid)
        return BDMF_ERR_NOENT;

    RDD_TUPLE_LKP_ENTRY_PROTOCOL_READ(key->prot, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_SRC_PORT_READ(key->src_port, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_DST_PORT_READ(key->dst_port, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_SRC_IP_READ(key->src_ip.addr.ipv4, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_DST_IP_READ(key->dst_ip.addr.ipv4, tuple_entry);
    RDD_TUPLE_LKP_ENTRY_KEY_EXTEND_READ(key->dst_ip.family, tuple_entry);
    key->src_ip.family = key->dst_ip.family;
    RDD_TUPLE_LKP_ENTRY_CONTEXT_INDEX_READ(*context_index, tuple_entry);

    return 0;
}
