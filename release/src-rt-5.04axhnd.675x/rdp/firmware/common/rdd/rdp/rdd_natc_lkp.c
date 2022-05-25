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
#include "rdd_natc_lkp.h"
#include "rdd_l4_filters.h"
#include "rdd_ip_flow.h"
#include "rdp_natcache.h"

extern uint32_t g_runner_nat_cache_key_ptr;
extern uint32_t g_runner_nat_cache_context_ptr;

volatile uint32_t *result_regs_addr[NAT_CACHE_SEARCH_ENGINES_NUM];
volatile uint32_t *status_regs_addr[NAT_CACHE_SEARCH_ENGINES_NUM];
volatile uint32_t *hit_count_reg_addr[NAT_CACHE_SEARCH_ENGINES_NUM];
volatile uint32_t *byte_count_reg_addr[NAT_CACHE_SEARCH_ENGINES_NUM];

bdmf_fastlock nat_cache_lock;
extern bdmf_fastlock cpu_message_lock;

int rdd_nat_cache_init(const rdd_module_t *module)
{
    RDD_NAT_CACHE_CFG_DTS cfg_entry = {};
    uint32_t *indirect_addr_regs_addr;
    uint32_t *indirect_data_regs_addr;
    uint32_t nat_cache_entry_idx;
    uint32_t nat_cache_data_word_idx;
    uint32_t register_value;

    cfg_entry.res_offset = module->res_offset;
    cfg_entry.context_offset = module->context_offset + 4;

    MWRITE_GROUP_BLOCK_32(module->group, module->cfg_ptr, (void *)&cfg_entry, sizeof(RDD_NAT_CACHE_CFG_DTS));

    rdd_l4_filters_init(((natc_params_t *)module->params)->dir);

    result_regs_addr[0] = (uint32_t *)NATCACHE_RDP_NAT0_KEY_RESULT_0_17;
    result_regs_addr[1] = (uint32_t *)NATCACHE_RDP_NAT1_KEY_RESULT_0_17;
    result_regs_addr[2] = (uint32_t *)NATCACHE_RDP_NAT2_KEY_RESULT_0_17;
    result_regs_addr[3] = (uint32_t *)NATCACHE_RDP_NAT3_KEY_RESULT_0_17;

    status_regs_addr[0] = (uint32_t *)NATCACHE_RDP_NAT0_COMMAND_STATUS;
    status_regs_addr[1] = (uint32_t *)NATCACHE_RDP_NAT1_COMMAND_STATUS;
    status_regs_addr[2] = (uint32_t *)NATCACHE_RDP_NAT2_COMMAND_STATUS;
    status_regs_addr[3] = (uint32_t *)NATCACHE_RDP_NAT3_COMMAND_STATUS;

    hit_count_reg_addr[0] = (uint32_t *)NATCACHE_RDP_NAT0_HIT_COUNT;
    hit_count_reg_addr[1] = (uint32_t *)NATCACHE_RDP_NAT1_HIT_COUNT;
    hit_count_reg_addr[2] = (uint32_t *)NATCACHE_RDP_NAT2_HIT_COUNT;
    hit_count_reg_addr[3] = (uint32_t *)NATCACHE_RDP_NAT3_HIT_COUNT;

    byte_count_reg_addr[0] = (uint32_t *)NATCACHE_RDP_NAT0_BYTE_COUNT;
    byte_count_reg_addr[1] = (uint32_t *)NATCACHE_RDP_NAT1_BYTE_COUNT;
    byte_count_reg_addr[2] = (uint32_t *)NATCACHE_RDP_NAT2_BYTE_COUNT;
    byte_count_reg_addr[3] = (uint32_t *)NATCACHE_RDP_NAT3_BYTE_COUNT;

    indirect_addr_regs_addr = (uint32_t *)NATCACHE_RDP_INDIRECT_ADDRESS;
    indirect_data_regs_addr = (uint32_t *)NATCACHE_RDP_INDIRECT_DATA;

    for (nat_cache_entry_idx = 0; nat_cache_entry_idx < 1024; nat_cache_entry_idx++)
    {
       for (nat_cache_data_word_idx = 0; nat_cache_data_word_idx < 20; nat_cache_data_word_idx++)
       {
           register_value = 0;
           WRITE_32(indirect_data_regs_addr + nat_cache_data_word_idx, register_value);
       }

       register_value = (1 << 10) | nat_cache_entry_idx;
       WRITE_32(indirect_addr_regs_addr, register_value);
    }

    return BDMF_ERR_OK;
}

void rdd_tuple_entry_add(rdpa_ip_flow_key_t *tuple_entry, uint32_t ipv6_src_ip_crc, uint32_t ipv6_dst_ip_crc,
    RDD_NAT_CACHE_LKP_ENTRY_DTS *key_entry_ptr)
{
    if (tuple_entry->dst_ip.family == bdmf_ip_family_ipv4)
    {
        RDD_NAT_CACHE_LKP_ENTRY_DST_IP_WRITE(tuple_entry->dst_ip.addr.ipv4, key_entry_ptr);
        RDD_NAT_CACHE_LKP_ENTRY_SRC_IP_WRITE(tuple_entry->src_ip.addr.ipv4, key_entry_ptr);
    }
    else
    {
        RDD_NAT_CACHE_LKP_ENTRY_DST_IP_WRITE(ipv6_dst_ip_crc, key_entry_ptr);
        RDD_NAT_CACHE_LKP_ENTRY_SRC_IP_WRITE(ipv6_src_ip_crc, key_entry_ptr);
    }

    RDD_NAT_CACHE_LKP_ENTRY_DST_PORT_WRITE(tuple_entry->dst_port, key_entry_ptr);
    RDD_NAT_CACHE_LKP_ENTRY_SRC_PORT_WRITE(tuple_entry->src_port, key_entry_ptr);
    RDD_NAT_CACHE_LKP_ENTRY_PROTOCOL_WRITE(tuple_entry->prot, key_entry_ptr);
    RDD_NAT_CACHE_LKP_ENTRY_KEY_EXTEND_WRITE(tuple_entry->dst_ip.family, key_entry_ptr);
    RDD_NAT_CACHE_LKP_ENTRY_VALID_WRITE(1, key_entry_ptr);
}

void rdd_tuple_entry_get(RDD_NAT_CACHE_LKP_ENTRY_DTS *tuple_entry_ptr, rdpa_ip_flow_key_t *tuple_entry)
{
    RDD_NAT_CACHE_LKP_ENTRY_DST_IP_READ(tuple_entry->dst_ip.addr.ipv4, tuple_entry_ptr);
    RDD_NAT_CACHE_LKP_ENTRY_SRC_IP_READ(tuple_entry->src_ip.addr.ipv4, tuple_entry_ptr);
    RDD_NAT_CACHE_LKP_ENTRY_DST_PORT_READ(tuple_entry->dst_port, tuple_entry_ptr);
    RDD_NAT_CACHE_LKP_ENTRY_SRC_PORT_READ(tuple_entry->src_port, tuple_entry_ptr);
    RDD_NAT_CACHE_LKP_ENTRY_PROTOCOL_READ(tuple_entry->prot, tuple_entry_ptr);
    RDD_NAT_CACHE_LKP_ENTRY_KEY_EXTEND_READ(tuple_entry->dst_ip.family, tuple_entry_ptr);
}

int rdd_nat_cache_entry_compare(RDD_NAT_CACHE_LKP_ENTRY_DTS *key_entry, rdpa_ip_flow_key_t *tuple_entry,
    uint32_t ipv6_src_ip_crc, uint32_t ipv6_dst_ip_crc)
{
    uint8_t key_extend, protocol;
    uint16_t src_port, dst_port;
    bdmf_ip_t src_ip, dst_ip;

    /* if entry is valid, check if it matches entry being added */
    RDD_NAT_CACHE_LKP_ENTRY_KEY_EXTEND_READ(key_extend, key_entry);
    RDD_NAT_CACHE_LKP_ENTRY_PROTOCOL_READ(protocol, key_entry);
    RDD_NAT_CACHE_LKP_ENTRY_SRC_PORT_READ(src_port, key_entry);
    RDD_NAT_CACHE_LKP_ENTRY_DST_PORT_READ(dst_port, key_entry);
    RDD_NAT_CACHE_LKP_ENTRY_SRC_IP_READ(src_ip.addr.ipv4, key_entry);
    RDD_NAT_CACHE_LKP_ENTRY_DST_IP_READ(dst_ip.addr.ipv4, key_entry);

    return ((protocol == tuple_entry->prot) &&
        (key_extend == tuple_entry->dst_ip.family) &&
        (src_port == tuple_entry->src_port) &&
        (dst_port == tuple_entry->dst_port) &&
        (((tuple_entry->dst_ip.family == bdmf_ip_family_ipv4) &&
        (src_ip.addr.ipv4 == tuple_entry->src_ip.addr.ipv4) &&
        (dst_ip.addr.ipv4 == tuple_entry->dst_ip.addr.ipv4)) ||
        (((tuple_entry->dst_ip.family == bdmf_ip_family_ipv6) &&
        (src_ip.addr.ipv4 == ipv6_src_ip_crc) &&
        (dst_ip.addr.ipv4 == ipv6_dst_ip_crc)))));
}

void rdd_nat_cache_entry_alloc(bdmf_error_t *rc, uint32_t *hash_idx, rdpa_ip_flow_key_t *tuple_entry,
    uint32_t ipv6_src_ip_crc, uint32_t ipv6_dst_ip_crc)
{
    RDD_NAT_CACHE_LKP_ENTRY_DTS *key_entry;
    uint32_t depth, entry_valid_bit, empty_hash_index, table_idx;
    bdmf_boolean entry_found = 0;

    table_idx = *hash_idx;

    for (depth = 0; depth < RDD_NAT_CACHE_LOOKUP_DEPTH_SIZE; depth++)
    {
        key_entry = &((RDD_NAT_CACHE_TABLE_DTS *)(NAT_CACHE_TABLE_PTR))->entry[table_idx];

        RDD_NAT_CACHE_LKP_ENTRY_VALID_READ(entry_valid_bit, key_entry);

        if (!entry_valid_bit)
        {
            if (!entry_found)
            {
                empty_hash_index = table_idx;
                entry_found = 1;
            }
        }
        else if (rdd_nat_cache_entry_compare(key_entry, tuple_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc))
        {
            *rc = BDMF_ERR_ALREADY;
            return;
        }

        table_idx = (table_idx + 1) % RDD_NAT_CACHE_TABLE_SIZE;
    }

    if (!entry_found)
        *rc = BDMF_ERR_NORES;

    *hash_idx = empty_hash_index;
}

void rdd_natc_validate_connection(bdmf_error_t *rc, rdpa_ip_flow_key_t *tuple_entry,
    rdd_fc_context_t *context_entry, rdpa_traffic_dir dir)
{
    if (tuple_entry->dst_ip.family != bdmf_ip_family_ipv4)
    {
        if (context_entry->actions_vector & rdpa_fc_action_nat)
            *rc = BDMF_ERR_PARM;
        else if (dir == rdpa_dir_ds && (context_entry->actions_vector & rdpa_fc_action_tunnel))
            *rc = BDMF_ERR_PARM;
    }

    if ((context_entry->vir_egress_port != RDD_WIFI_VPORT) && (context_entry->wifi_ssid != 0))
        *rc = BDMF_ERR_PARM;
}

void rdd_connection_hash_function(uint32_t *hash_idx, uint8_t *connection_entry)
{
    *hash_idx = rdd_crc_bit_by_bit_natc(connection_entry, sizeof(RDD_NAT_CACHE_LKP_ENTRY_DTS), 0);
    *hash_idx = (*hash_idx >> 16) ^ (*hash_idx & 0xffff);
}

void rdd_connection_key_compose(bdmf_error_t *rc, uint8_t *connection_entry, rdpa_ip_flow_key_t *tuple_entry,
    uint32_t *ipv6_src_ip_crc, uint32_t *ipv6_dst_ip_crc)
{
    uint32_t crc_init_value;
#if !defined(FIRMWARE_INIT)
    uint32_t *ipv6_crc_buffer_ptr;
    int rdd_error;
#endif

    connection_entry[0] = (1 << 7); /* valid bit */
    connection_entry[1] = 0;
    connection_entry[2] = tuple_entry->dst_ip.family;
    connection_entry[3] = tuple_entry->prot;
    connection_entry[4] = (tuple_entry->src_port >> 8) & 0xFF;
    connection_entry[5] = tuple_entry->src_port & 0xFF;
    connection_entry[6] = (tuple_entry->dst_port >> 8) & 0xFF;
    connection_entry[7] = tuple_entry->dst_port & 0xFF;

    if (tuple_entry->dst_ip.family == bdmf_ip_family_ipv4)
    {
        connection_entry[8] = (tuple_entry->src_ip.addr.ipv4 >> 24) & 0xFF;
        connection_entry[9] = (tuple_entry->src_ip.addr.ipv4 >> 16) & 0xFF;
        connection_entry[10] = (tuple_entry->src_ip.addr.ipv4 >> 8) & 0xFF;
        connection_entry[11] = tuple_entry->src_ip.addr.ipv4 & 0xFF;
        connection_entry[12] = (tuple_entry->dst_ip.addr.ipv4 >> 24) & 0xFF;
        connection_entry[13] = (tuple_entry->dst_ip.addr.ipv4 >> 16) & 0xFF;
        connection_entry[14] = (tuple_entry->dst_ip.addr.ipv4 >> 8) & 0xFF;
        connection_entry[15] = tuple_entry->dst_ip.addr.ipv4 & 0xFF;
    }
    else
    {
/* XXX: cpu tx is temporary not supported */
#if !defined(FIRMWARE_INIT)
        ipv6_crc_buffer_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + IPV6_CRC_BUFFER_ADDRESS);

        bdmf_fastlock_lock(&cpu_message_lock);

        MWRITE_BLK_8(ipv6_crc_buffer_ptr, tuple_entry->src_ip.addr.ipv6.data, 16);

        rdd_error = _rdd_cpu_message_send(RDD_CPU_MESSAGE_IPV6_CRC_GET, RDD_CLUSTER_0, 0, 0, 0, 1);

        if (rdd_error)
        {
            bdmf_fastlock_unlock(&cpu_message_lock);
            return;
        }
        *ipv6_src_ip_crc = swap4bytes(*(volatile uint32_t *)ipv6_crc_buffer_ptr);
#else
        crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);
        *ipv6_src_ip_crc = rdd_crc_bit_by_bit(tuple_entry->src_ip.addr.ipv6.data, 16, 0, crc_init_value, RDD_CRC_TYPE_32);
#endif
        connection_entry[8] = (*ipv6_src_ip_crc >> 24) & 0xFF;
        connection_entry[9] = (*ipv6_src_ip_crc >> 16) & 0xFF;
        connection_entry[10] = (*ipv6_src_ip_crc >> 8) & 0xFF;
        connection_entry[11] = (*ipv6_src_ip_crc >> 0) & 0xFF;
/* XXX: cpu tx is temporary not supported */
#if !defined(FIRMWARE_INIT)
        MWRITE_BLK_8(ipv6_crc_buffer_ptr, tuple_entry->dst_ip.addr.ipv6.data, 16);

        rdd_error = _rdd_cpu_message_send(RDD_CPU_MESSAGE_IPV6_CRC_GET, RDD_CLUSTER_0, 0, 0, 0, 1);

        if (rdd_error)
        {
            bdmf_fastlock_unlock(&cpu_message_lock);
            return;
        }
        *ipv6_dst_ip_crc = swap4bytes(*(volatile uint32_t *)ipv6_crc_buffer_ptr);
#else
        crc_init_value = rdd_crc_init_value_get(RDD_CRC_TYPE_32);
        *ipv6_dst_ip_crc = rdd_crc_bit_by_bit(tuple_entry->dst_ip.addr.ipv6.data, 16, 0, crc_init_value, RDD_CRC_TYPE_32);
#endif
        connection_entry[12] = (*ipv6_dst_ip_crc >> 24) & 0xFF;
        connection_entry[13] = (*ipv6_dst_ip_crc >> 16) & 0xFF;
        connection_entry[14] = (*ipv6_dst_ip_crc >> 8) & 0xFF;
        connection_entry[15] = (*ipv6_dst_ip_crc >> 0) & 0xFF;

#if !defined(FIRMWARE_INIT)
        bdmf_fastlock_unlock(&cpu_message_lock);
#endif
    }
}

void rdd_connection_entry_hash_idx_calc(bdmf_error_t *rc, uint32_t *hash_idx, rdpa_ip_flow_key_t *tuple_entry,
    uint32_t *ipv6_src_ip_crc, uint32_t *ipv6_dst_ip_crc)
{
    uint8_t connection_entry[sizeof(RDD_NAT_CACHE_LKP_ENTRY_DTS)] = {0};

    rdd_connection_key_compose(rc, connection_entry, tuple_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc);
    rdd_connection_hash_function(hash_idx, connection_entry);
}

int rdd_ip_flow_add(rdd_module_t *module, rdpa_ip_flow_key_t *key, void *context, uint32_t *context_index)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    rdd_fc_context_t *fc_ctx = (rdd_fc_context_t *)context;
    uint32_t hash_idx, ipv6_src_ip_crc = 0, ipv6_dst_ip_crc = 0;

    rdd_natc_validate_connection(&rc, key, fc_ctx, ((natc_params_t *)(module->params))->dir);

    if (rc)
        goto exit;

    rdd_connection_entry_hash_idx_calc(&rc, &hash_idx, key, &ipv6_src_ip_crc, &ipv6_dst_ip_crc);

    if (rc)
        goto exit;

    bdmf_fastlock_lock(&nat_cache_lock);

    rdd_nat_cache_entry_alloc(&rc, &hash_idx, key, ipv6_src_ip_crc, ipv6_dst_ip_crc);

    if (rc)
        goto unlock;

#if !defined(FIRMWARE_INIT)
    if (key->dst_ip.family != bdmf_ip_family_ipv4)
        fc_ctx->nat_ip.addr.ipv4 = *(uint32_t *)&(key->dst_ip.addr.ipv6.data[12]); /*TODO: verify ipv4*/
#endif
    fc_ctx->ip_version = key->dst_ip.family;
    fc_ctx->conn_index = hash_idx;
    fc_ctx->conn_dir = ((natc_params_t *)(module->params))->dir;
    rdd_connection_checksum_delta_calc(key, fc_ctx);
    rdd_fc_context_entry_write(fc_ctx, &((RDD_CONTEXT_TABLE_DTS *)(CONTEXT_TABLE_PTR))->entry[hash_idx]);
    rdd_tuple_entry_add(key, ipv6_src_ip_crc, ipv6_dst_ip_crc, &((RDD_NAT_CACHE_TABLE_DTS *)(NAT_CACHE_TABLE_PTR))->entry[hash_idx]);

    /* NAT cache workaround: the context table is wrap around at 64K while the key table is continuous */
    if (hash_idx < RDD_NAT_CACHE_EXTENSION_TABLE_SIZE)
        rdd_tuple_entry_add(key, ipv6_src_ip_crc, ipv6_dst_ip_crc, &((RDD_NAT_CACHE_TABLE_DTS *)(NAT_CACHE_TABLE_PTR))->entry[RDD_NAT_CACHE_TABLE_SIZE + hash_idx]);

    *context_index = hash_idx;

unlock:

    bdmf_fastlock_unlock(&nat_cache_lock);

exit:

    return rc;
}

int rdd_ip_flow_get(rdd_module_t *module, uint32_t ip_flow_index, rdpa_ip_flow_key_t *key, uint32_t *context_index)
{
    uint8_t valid_bit = 0;
    RDD_NAT_CACHE_LKP_ENTRY_DTS *natc_tuple_entry;

    if (ip_flow_index >= RDD_NAT_CACHE_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    bdmf_fastlock_lock(&nat_cache_lock);

    natc_tuple_entry = &((RDD_NAT_CACHE_TABLE_DTS *)(NAT_CACHE_TABLE_PTR))->entry[ip_flow_index];
    RDD_NAT_CACHE_LKP_ENTRY_VALID_READ(valid_bit, natc_tuple_entry);

    if (!valid_bit)
    {
        bdmf_fastlock_unlock(&nat_cache_lock);
        return BDMF_ERR_NOENT;
    }

    rdd_tuple_entry_get(natc_tuple_entry, key);
    *context_index = ip_flow_index;

    bdmf_fastlock_unlock(&nat_cache_lock);

    return BDMF_ERR_OK;
}

void rdd_tuple_entry_idx_get(bdmf_error_t *rc, uint32_t *hash_idx, rdpa_ip_flow_key_t *tuple_entry,
    uint32_t ipv6_src_ip_crc, uint32_t ipv6_dst_ip_crc)
{
    RDD_NAT_CACHE_LKP_ENTRY_DTS *key_entry;
    uint32_t depth, entry_valid_bit, table_idx;

    table_idx = *hash_idx;

    for (depth = 0; depth < RDD_NAT_CACHE_LOOKUP_DEPTH_SIZE; key_entry++, depth++)
    {
        key_entry = &((RDD_NAT_CACHE_TABLE_DTS *)(NAT_CACHE_TABLE_PTR))->entry[table_idx];

        RDD_NAT_CACHE_LKP_ENTRY_VALID_READ(entry_valid_bit, key_entry);

        if (entry_valid_bit && rdd_nat_cache_entry_compare(key_entry, tuple_entry, ipv6_src_ip_crc, ipv6_dst_ip_crc))
        {
            *hash_idx = table_idx;
            return;
        }

        table_idx = (table_idx + 1) % RDD_NAT_CACHE_TABLE_SIZE;
    }

    *rc = BDMF_ERR_NOENT;
}

int rdd_ip_flow_find(rdd_module_t *module, rdpa_ip_flow_key_t *key, uint32_t *context_index)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t hash_idx;
    uint32_t ipv6_src_ip_crc = 0, ipv6_dst_ip_crc = 0;

    rdd_connection_entry_hash_idx_calc(&rc, &hash_idx, key, &ipv6_src_ip_crc, &ipv6_dst_ip_crc);

    if (rc)
        goto exit;

    bdmf_fastlock_lock(&nat_cache_lock);

    rdd_tuple_entry_idx_get(&rc, &hash_idx, key, ipv6_src_ip_crc, ipv6_dst_ip_crc);

    if (rc)
        goto unlock;

    *context_index = hash_idx;

unlock:

    bdmf_fastlock_unlock(&nat_cache_lock);

exit:

    return rc;
}

void rdd_nat_cache_write_key_result_regs(uint32_t natc_engine_idx, uint32_t *keyword)
{
    uint32_t key_value;

    key_value = swap4bytes(*(uint32_t *)(keyword + 3));
    WRITE_32(result_regs_addr[natc_engine_idx] + 0, key_value);

    key_value = swap4bytes(*(uint32_t *)(keyword + 2));
    WRITE_32(result_regs_addr[natc_engine_idx] + 1, key_value);

    key_value = swap4bytes(*(uint32_t *)(keyword + 1));
    WRITE_32(result_regs_addr[natc_engine_idx] + 2, key_value);

    key_value = swap4bytes(*(uint32_t *)(keyword + 0));
    WRITE_32(result_regs_addr[natc_engine_idx] + 3, key_value);
}

int rdd_nat_cache_write_command(uint32_t natc_engine_idx, uint32_t command)
{
    uint32_t status_reg;
    uint32_t status_mask;
    uint8_t break_out_counter = 200;

    if (command == natc_lookup)
        status_mask = (NATC_STATUS_BUSY_BIT | NATC_STATUS_MISS_BIT);
    else if (command == natc_del)
        status_mask = NATC_STATUS_BUSY_BIT;
    else
        status_mask = (NATC_STATUS_BUSY_BIT | NATC_STATUS_ERROR_BIT);

    /* write command and wait for not busy */
    command |= NATC_STATUS_BUSY_BIT;

    WRITE_32(status_regs_addr[natc_engine_idx], command);

    while (--break_out_counter)
    {
        READ_32(status_regs_addr[natc_engine_idx], status_reg);

        if ((status_reg & status_mask) == 0)
            return BDMF_ERR_OK;
    }

#if !defined(FIRMWARE_INIT)
    BDMF_TRACE_INFO("rdd_nat_cache_write_command(): status register = 0x%0x\n", status_reg);
#endif
    return BDMF_ERR_INTERNAL;
}

int rdd_nat_cache_submit_command(uint32_t command, uint32_t *keyword, uint32_t *context, uint32_t *hit_count, uint32_t *byte_count)
{
    uint32_t natc_engine_idx = command;
    int rc;

    rdd_nat_cache_write_key_result_regs(natc_engine_idx, keyword);

    rc = rdd_nat_cache_write_command(natc_engine_idx, command);

    if ((rc == BDMF_ERR_OK) && (command == natc_lookup))
    {
       if (hit_count)
           READ_32(hit_count_reg_addr[natc_engine_idx], *hit_count);

       if (byte_count)
           READ_32(byte_count_reg_addr[natc_engine_idx], *byte_count);
    }

    return rc;
}

int rdd_ip_flow_counters_get(uint32_t entry_index, uint32_t *hit_count, uint32_t *byte_count)
{
    RDD_NAT_CACHE_LKP_ENTRY_DTS *nat_cache_lookup_entry_ptr, nat_cache_lookup_entry;
#if !defined(FIRMWARE_INIT)
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *nat_cache_context_entry_ptr;
    bdmf_error_t rc;
#endif
    if (entry_index >= RDD_NAT_CACHE_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    bdmf_fastlock_lock(&nat_cache_lock);

    nat_cache_lookup_entry_ptr = &((RDD_NAT_CACHE_TABLE_DTS *)(NAT_CACHE_TABLE_PTR))->entry[entry_index];

    memcpy(&nat_cache_lookup_entry, nat_cache_lookup_entry_ptr, sizeof(RDD_NAT_CACHE_LKP_ENTRY_DTS));

#if !defined(FIRMWARE_INIT)
    /* look for the entry in the NAT cache internal memory, if found then statistics is also there */
    rc = rdd_nat_cache_submit_command(natc_lookup, (uint32_t *)&nat_cache_lookup_entry, NULL, hit_count, byte_count);

    /* if its not in the NAT cache then it was copied to the DDR */
    if (rc != BDMF_ERR_OK)
    {
        nat_cache_context_entry_ptr = &((RDD_CONTEXT_TABLE_DTS *)(CONTEXT_TABLE_PTR))->entry[entry_index];

        RDD_FLOW_CACHE_CONTEXT_ENTRY_STATUS_0_READ(*hit_count, nat_cache_context_entry_ptr);
        RDD_FLOW_CACHE_CONTEXT_ENTRY_STATUS_1_READ(*byte_count, nat_cache_context_entry_ptr);
    }
#endif

    bdmf_fastlock_unlock(&nat_cache_lock);

    return BDMF_ERR_OK;
}

int rdd_ip_flow_delete(rdd_module_t *module, uint32_t entry_index)
{
    RDD_NAT_CACHE_LKP_ENTRY_DTS *nat_cache_lookup_entry_ptr, nat_cache_lookup_entry;
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *nat_cache_context_entry_ptr;
    bdmf_error_t rc = BDMF_ERR_OK;

    if (entry_index >= RDD_NAT_CACHE_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    bdmf_fastlock_lock(&nat_cache_lock);

    /* NAT cache workaround: the context table is wrap around at 64K while the key table is continuous */
    if (entry_index < RDD_NAT_CACHE_EXTENSION_TABLE_SIZE) {
        nat_cache_lookup_entry_ptr = &((RDD_NAT_CACHE_TABLE_DTS *)(NAT_CACHE_TABLE_PTR))->entry[RDD_NAT_CACHE_TABLE_SIZE + entry_index];

        memset(nat_cache_lookup_entry_ptr, 0, sizeof(RDD_NAT_CACHE_LKP_ENTRY_DTS));
    }

    nat_cache_lookup_entry_ptr = &((RDD_NAT_CACHE_TABLE_DTS *)(NAT_CACHE_TABLE_PTR))->entry[entry_index];

    memcpy(&nat_cache_lookup_entry, nat_cache_lookup_entry_ptr, sizeof(RDD_NAT_CACHE_LKP_ENTRY_DTS));

    memset(nat_cache_lookup_entry_ptr, 0, sizeof(RDD_NAT_CACHE_LKP_ENTRY_DTS));

    nat_cache_context_entry_ptr = &((RDD_CONTEXT_TABLE_DTS *)(CONTEXT_TABLE_PTR))->entry[entry_index];

    memset(nat_cache_context_entry_ptr, 0, sizeof(RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS));

#if !defined(FIRMWARE_INIT)
    rc = rdd_nat_cache_submit_command(natc_del, (uint32_t *)&nat_cache_lookup_entry, NULL, NULL, NULL);
#endif

    bdmf_fastlock_unlock(&nat_cache_lock);

    return rc;
}
