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
#include "rdd_fc.h"
#include "rdd_ip_flow.h"

uint32_t g_free_context_entries_number;
uint32_t g_free_context_entries_head;
uint32_t g_free_context_entries_tail;
uint32_t *g_free_connection_context_entries;

extern uint32_t g_runner_tables_offset;
extern uint32_t g_ddr_runner_base_addr;
extern bdmf_fastlock int_lock_irq;
#if defined(FIRMWARE_INIT)
uint32_t g_context_table_ptr;
#endif

extern int rdd_ip_flow_entry_get(rdd_module_t *module, uint32_t ip_flow_index,
    rdpa_ip_flow_key_t *tuple_entry, uint32_t *context_index);

void rdd_context_entry_free(uint32_t index)
{
    g_free_connection_context_entries[g_free_context_entries_tail++] = index;
    g_free_context_entries_tail = g_free_context_entries_tail % RDD_CONTEXT_TABLE_SIZE;
    g_free_context_entries_number++;
}

void rdd_fc_context_init(void)
{
    uint32_t i;

    g_free_connection_context_entries = (uint32_t *)bdmf_alloc(RDD_CONTEXT_TABLE_SIZE * sizeof(uint32_t));
    g_free_context_entries_number = 0;
    g_free_context_entries_head = 0;
    g_free_context_entries_tail = 0;

    for (i = 0; i < RDD_CONTEXT_TABLE_SIZE; i++)
        rdd_context_entry_free(i);
}

void rdd_fc_context_entry_read(rdd_fc_context_t *ctx, RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *entry)
{
    uint8_t wifi_ssid;
    uint32_t i;

    memset(ctx, 0, sizeof(rdd_fc_context_t));

    RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_DIRECTION_READ(ctx->conn_dir, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_ACTIONS_VECTOR_READ(ctx->actions_vector, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_FWD_ACTION_READ(ctx->fwd_action, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_CPU_REASON_READ(ctx->trap_reason, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUES_MODE_READ(ctx->service_queue_enabled, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUE_READ(ctx->service_queue_id, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_QOS_MAPPING_MODE_READ(ctx->qos_method, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_IP_VERSION_READ(ctx->ip_version, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_NAT_PORT_READ(ctx->nat_port, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_NAT_IP_READ(ctx->nat_ip.addr.ipv4, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DS_LITE_HEADER_INDEX_READ(ctx->ds_lite_hdr_index, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_OUTER_VID_OFFSET_READ(ctx->ovid_offset, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_OUTER_PBIT_REMAP_ACTION_READ(ctx->opbit_action, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_INNER_PBIT_REMAP_ACTION_READ(ctx->ipbit_action, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DSCP_VALUE_READ(ctx->dscp_value, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_ECN_VALUE_READ(ctx->ecn_value, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_POLICER_ID_READ(ctx->policer_id, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_VIRTUAL_EGRESS_PORT_READ(ctx->vir_egress_port, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_PHYSICAL_EGRESS_PORT_READ(ctx->phy_egress_port, entry);

    if (ctx->vir_egress_port == RDD_WIFI_VPORT)
    {
        RDD_FLOW_CACHE_CONTEXT_ENTRY_IS_WFD_READ(ctx->wfd.nic_ucast.is_wfd, entry);

        if (ctx->wfd.nic_ucast.is_wfd)
        {
            uint16_t wl_metadata;
            RDD_FLOW_CACHE_CONTEXT_ENTRY_WFD_IDX_READ(ctx->wfd.nic_ucast.wfd_idx, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_WL_METADATA_READ(wl_metadata, entry);
            
            if (wl_metadata & (1 << 13))
            {
                ctx->wfd.nic_ucast.is_chain = 1;
                ctx->wfd.nic_ucast.priority = (wl_metadata & 0xf00) >> 8;
                ctx->wfd.nic_ucast.chain_idx = wl_metadata & 0xff;
            }
            else
            {
                ctx->wfd.dhd_ucast.priority = (wl_metadata & 0x1c00) >> 10;
                ctx->wfd.dhd_ucast.flowring_idx = wl_metadata & 0x3ff;
            }
        }
        else
        {
            RDD_FLOW_CACHE_CONTEXT_ENTRY_PRIORITY_READ(ctx->rnr.priority, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_FLOW_RING_ID_READ(ctx->rnr.flowring_idx, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_FLOW_PRIO_READ(ctx->rnr.flow_prio, entry);
        }
    }
    else
    {
        RDD_FLOW_CACHE_CONTEXT_ENTRY_TRAFFIC_CLASS_READ(ctx->traffic_class, entry);
        RDD_FLOW_CACHE_CONTEXT_ENTRY_WAN_FLOW_INDEX_READ(ctx->wan_flow_index, entry);
        RDD_FLOW_CACHE_CONTEXT_ENTRY_RATE_CONTROLLER_READ(ctx->rate_controller, entry);
    }

    RDD_FLOW_CACHE_CONTEXT_ENTRY_WIFI_SSID_READ(wifi_ssid, entry);

    /* In DHD TX Post descriptor, wifi_ssid expected to be subunit */
    if (ctx->vir_egress_port == RDD_WIFI_VPORT && !ctx->wfd.nic_ucast.is_wfd)
        ctx->wifi_ssid = wifi_ssid + ctx->rnr.radio_idx * WL_NUM_OF_SSID_PER_UNIT;

    RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_OFFSET_READ(ctx->l2_hdr_offset, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_SIZE_READ(ctx->l2_hdr_size, entry);
    ctx->l2_hdr_size += RDD_LAYER2_HEADER_MINIMUM_LENGTH;
    RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_NUMBER_OF_TAGS_READ(ctx->l2_hdr_number_of_tags, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_IP_CHECKSUM_DELTA_READ(ctx->ip_checksum_delta, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_L4_CHECKSUM_DELTA_READ(ctx->l4_checksum_delta, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_VALID_PACKETS_COUNTER_READ(ctx->valid_cnt.packets, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_VALID_BYTES_COUNTER_READ(ctx->valid_cnt.bytes, entry);

    for (i = 0; i < RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_NUMBER; i++)
        RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_READ(ctx->l2_header[i], entry, i);
}

void rdd_fc_context_entry_write(rdd_fc_context_t *ctx, RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *entry)
{
    uint32_t l2_hdr_offset;
    uint8_t wifi_ssid;
    uint32_t i;

    memset(entry, 0, sizeof(RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS));

    RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_DIRECTION_WRITE(ctx->conn_dir, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_ACTIONS_VECTOR_WRITE(ctx->actions_vector, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_FWD_ACTION_WRITE(ctx->fwd_action, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_CPU_REASON_WRITE(ctx->trap_reason - rdpa_cpu_rx_reason_udef_0, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUES_MODE_WRITE(ctx->service_queue_enabled, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUE_WRITE(ctx->service_queue_id, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_QOS_MAPPING_MODE_WRITE(ctx->qos_method, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_IP_VERSION_WRITE(ctx->ip_version, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_NAT_PORT_WRITE(ctx->nat_port, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_NAT_IP_WRITE(ctx->nat_ip.addr.ipv4, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DS_LITE_HEADER_INDEX_WRITE(ctx->ds_lite_hdr_index, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_OUTER_VID_OFFSET_WRITE(ctx->ovid_offset, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_OUTER_PBIT_REMAP_ACTION_WRITE(ctx->opbit_action, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_INNER_PBIT_REMAP_ACTION_WRITE(ctx->ipbit_action, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DSCP_VALUE_WRITE(ctx->dscp_value, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_ECN_VALUE_WRITE(ctx->ecn_value, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_POLICER_ID_WRITE(ctx->policer_id, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_VIRTUAL_EGRESS_PORT_WRITE(ctx->vir_egress_port, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_PHYSICAL_EGRESS_PORT_WRITE(ctx->phy_egress_port, entry);

    l2_hdr_offset = ctx->l2_hdr_offset;
    RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_OFFSET_WRITE(l2_hdr_offset, entry);
    ctx->l2_hdr_size -= RDD_LAYER2_HEADER_MINIMUM_LENGTH;
    RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_SIZE_WRITE(ctx->l2_hdr_size, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_NUMBER_OF_TAGS_WRITE(ctx->l2_hdr_number_of_tags, entry);

    wifi_ssid = ctx->wifi_ssid;

    if (ctx->vir_egress_port == RDD_WIFI_VPORT)
    {
        RDD_FLOW_CACHE_CONTEXT_ENTRY_IS_WFD_WRITE(ctx->wfd.nic_ucast.is_wfd, entry);
        if (ctx->wfd.nic_ucast.is_wfd)
        {
            uint16_t wl_metadata = 0;
            /* In the nic mode and in the dhd this fields are on the same places so we could 
               just use one of the union */
            RDD_FLOW_CACHE_CONTEXT_ENTRY_WFD_IDX_WRITE(ctx->wfd.nic_ucast.wfd_idx, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_WFD_PRIO_WRITE(ctx->wfd.nic_ucast.wfd_prio, entry);

            if (ctx->wfd.nic_ucast.is_chain)
            {
                wl_metadata = (1 << 13) | ctx->wfd.nic_ucast.chain_idx;
                wl_metadata |= ctx->wfd.nic_ucast.priority << 8;
            }
            else
            {
                wl_metadata = ctx->wfd.dhd_ucast.flowring_idx;
                wl_metadata |= ctx->wfd.dhd_ucast.priority << 10; 
            }

            RDD_FLOW_CACHE_CONTEXT_ENTRY_WL_METADATA_WRITE(wl_metadata, entry);
        }
        else
        {
            RDD_FLOW_CACHE_CONTEXT_ENTRY_PRIORITY_WRITE(ctx->rnr.priority, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_FLOW_RING_ID_WRITE(ctx->rnr.flowring_idx, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_RADIO_IDX_WRITE(ctx->rnr.radio_idx, entry);
            RDD_FLOW_CACHE_CONTEXT_ENTRY_FLOW_PRIO_WRITE(ctx->rnr.flow_prio, entry);

            /* In DHD TX Post descriptor, wifi_ssid expected to be subunit */
            wifi_ssid %= WL_NUM_OF_SSID_PER_UNIT;
        }
    }
    else
    {
        RDD_FLOW_CACHE_CONTEXT_ENTRY_TRAFFIC_CLASS_WRITE(ctx->traffic_class, entry);
        RDD_FLOW_CACHE_CONTEXT_ENTRY_WAN_FLOW_INDEX_WRITE(ctx->wan_flow_index, entry);
        RDD_FLOW_CACHE_CONTEXT_ENTRY_RATE_CONTROLLER_WRITE(ctx->rate_controller, entry);
    }

    RDD_FLOW_CACHE_CONTEXT_ENTRY_WIFI_SSID_WRITE(ctx->wifi_ssid, entry);

    RDD_FLOW_CACHE_CONTEXT_ENTRY_IP_CHECKSUM_DELTA_WRITE(ctx->ip_checksum_delta, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_L4_CHECKSUM_DELTA_WRITE(ctx->l4_checksum_delta, entry);

    for (i = 0; i < RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_NUMBER; i++)
        RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_WRITE(ctx->l2_header[i], entry, i);
}

int rdd_context_entry_add(rdpa_ip_flow_key_t *key, void *ctx, uint32_t ip_flow_index, uint32_t *context_index)
{
    RDD_CONTEXT_TABLE_DTS *context_tbl;
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *context_entry;
    rdd_fc_context_t *fc_ctx = (rdd_fc_context_t *)ctx;

    if (g_free_context_entries_number > RDD_RESERVED_CONTEXT_ENTRIES)
    {
        *context_index = g_free_connection_context_entries[g_free_context_entries_head++];
        g_free_context_entries_head = g_free_context_entries_head % RDD_CONTEXT_TABLE_SIZE;
        g_free_context_entries_number--;
    }
    else
    {
        return BDMF_ERR_NOENT;
    }

    context_tbl = (RDD_CONTEXT_TABLE_DTS *)CONTEXT_TABLE_PTR;
    context_entry = &(context_tbl->entry[*context_index]);

    fc_ctx->conn_dir = key->dir;
    fc_ctx->ip_version = key->dst_ip.family;

    rdd_connection_checksum_delta_calc(key, fc_ctx);

    rdd_fc_context_entry_write(fc_ctx, context_entry);

    return 0;
}

static RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *_rdd_context_entry_get(uint32_t context_index)
{
    RDD_CONTEXT_TABLE_DTS *context_table;

    context_table = (RDD_CONTEXT_TABLE_DTS *)CONTEXT_TABLE_PTR;
    return &(context_table->entry[context_index]);
}

int rdd_context_entry_get(uint32_t context_index, void *context_entry)
{
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *entry;
    rdd_fc_context_t *fc_ctx = (rdd_fc_context_t *)context_entry;

    entry = _rdd_context_entry_get(context_index);

    if (!entry)
        return BDMF_ERR_NOENT;

    /* It's possible that we just need to validate the context entry exist, but not really need the context itself */
    if (fc_ctx)
        rdd_fc_context_entry_read(fc_ctx, entry);

    return 0;
}

int rdd_context_entry_modify(uint32_t context_index, void *context_entry)
{
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *entry;
    rdd_fc_context_t *fc_ctx = (rdd_fc_context_t *)context_entry;
    unsigned long flags;

    entry = _rdd_context_entry_get(context_index);
    if (!entry)
        return BDMF_ERR_NOENT;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);
    rdd_fc_context_entry_write(fc_ctx, entry);
    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);

    return 0;
}

int rdd_clean_context_aging_get(uint32_t context_index, uint8_t *aging_status)
{
    RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS *entry;
    unsigned long flags;

    entry = _rdd_context_entry_get(context_index);
    if (!entry)
        return BDMF_ERR_NOENT;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_ACTIVITY_STATUS_READ(*aging_status, entry);
    RDD_FLOW_CACHE_CONTEXT_ENTRY_ACTIVITY_STATUS_WRITE(0, entry);
    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);

    return 0;
}

int rdd_ds_lite_tunnel_cfg(rdpa_ds_lite_tunnel_id dual_stack_lite_tunnel_id, bdmf_ipv6_t *ipv6_src_ip,
    bdmf_ipv6_t *ipv6_dst_ip)
{
    RDD_DUAL_STACK_LITE_TABLE_DTS *dual_stack_lite_table;
    RDD_DUAL_STACK_LITE_ENTRY_DTS *dual_stack_lite_entry;
    uint32_t i;

    if (dual_stack_lite_tunnel_id > rdpa_ds_lite_max_tunnel_id)
        return BDMF_ERR_PARM;

    dual_stack_lite_table = RDD_DUAL_STACK_LITE_TABLE_PTR();

    dual_stack_lite_entry = &(dual_stack_lite_table->entry[dual_stack_lite_tunnel_id]);

    for (i = 0; i < 16; i++)
    {
        RDD_DUAL_STACK_LITE_ENTRY_SRC_IP_WRITE(ipv6_src_ip->data[i], dual_stack_lite_entry, i);
        RDD_DUAL_STACK_LITE_ENTRY_DST_IP_WRITE(ipv6_dst_ip->data[i], dual_stack_lite_entry, i);
    }

    return 0;
}

