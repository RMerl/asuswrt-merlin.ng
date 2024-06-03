/*
    <:copyright-BRCM:2013-2016:DUAL/GPL:standard
    
       Copyright (c) 2013-2016 Broadcom 
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

rdpa_cpu_reason cpu_reason_to_cpu_per_port_reason_index(rdpa_cpu_reason reason)
{
    switch (reason)
    {
    case rdpa_cpu_rx_reason_mcast:
        return 0;
    case rdpa_cpu_rx_reason_bcast:
        return 1;
    case rdpa_cpu_rx_reason_unknown_da:
        return 2;
    default:
        break;
    }

    return 3;
}

void _rdd_runner_flow_header_descriptor_init(uint32_t private_memory_offset, uint32_t descriptor_addr,
    uint32_t thread_number, uint32_t packet_headroom_size, uint32_t ih_buffer_id)
{
    uint32_t *runner_flow_header_descriptor_ptr;
    uint32_t runner_flow_header_descriptor[2];

    runner_flow_header_descriptor_ptr = (uint32_t *)(DEVICE_ADDRESS(private_memory_offset) + descriptor_addr);

    runner_flow_header_descriptor[0] = (1 << 5) + (thread_number << 6);
    runner_flow_header_descriptor[1] = ((128 - packet_headroom_size) << 5) + (ih_buffer_id << 20);

    MWRITE_32(((uint8_t *)runner_flow_header_descriptor_ptr + 0), runner_flow_header_descriptor[0]);
    MWRITE_32(((uint8_t *)runner_flow_header_descriptor_ptr + 4), runner_flow_header_descriptor[1]);
}

void rdd_cpu_rx_meter_config(rdd_cpu_rx_meter cpu_meter, uint16_t average_rate, uint16_t burst_size, rdpa_traffic_dir direction)
{
    RDD_CPU_RX_METER_TABLE_DTS *table;
    RDD_CPU_RX_METER_ENTRY_DTS *entry;
    static uint32_t api_first_time_call[2] = {1, 1};

    if (direction == rdpa_dir_ds)
        table = (RDD_CPU_RX_METER_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_CPU_RX_METER_TABLE_ADDRESS);
    else
        table = (RDD_CPU_RX_METER_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_CPU_RX_METER_TABLE_ADDRESS);

    entry = &(table->entry[cpu_meter]);

    burst_size = rdd_budget_to_alloc_unit(burst_size, RDD_CPU_RX_METER_TIMER_PERIOD, 0);

    RDD_CPU_RX_METER_ENTRY_BUDGET_LIMIT_WRITE(burst_size, entry);

    average_rate = rdd_budget_to_alloc_unit(average_rate, RDD_CPU_RX_METER_TIMER_PERIOD, 0);

    RDD_CPU_RX_METER_ENTRY_ALLOCATED_BUDGET_WRITE(average_rate, entry);

    if (api_first_time_call[direction])
    {
        rdd_timer_task_config(direction, RDD_CPU_RX_METER_TIMER_PERIOD, CPU_RX_METER_BUDGET_ALLOCATE_CODE_ID);

        api_first_time_call[direction] = 0;
    }
}

static void _rdd_cpu_reason_and_src_emac_to_rx_meter_cfg(rdpa_cpu_reason reason, rdd_cpu_rx_meter meter,
    rdd_emac_id_t src_port, bdmf_boolean is_set)
{
    RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_DTS *table;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS *entry;
    uint32_t cpu_reason_per_port_index;

    table = RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_PTR();

    cpu_reason_per_port_index = cpu_reason_to_cpu_per_port_reason_index(reason);

    entry = &(table->entry[cpu_reason_per_port_index][src_port]);
    if (is_set)
        RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE(meter, entry);
    else
    {
        rdd_cpu_rx_meter curr_meter;

        RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_READ(curr_meter, entry);

        if (curr_meter == meter)
            RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE(CPU_RX_METER_DISABLE, entry);
    }
}

static void _rdd_cpu_reason_to_cpu_rx_meter_us(rdpa_cpu_reason reason, rdd_cpu_rx_meter meter, uint32_t src_port_mask)
{
    RDD_US_CPU_REASON_TO_METER_TABLE_DTS *table;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS *entry;

    if (reason == rdpa_cpu_rx_reason_mcast || reason == rdpa_cpu_rx_reason_bcast ||
        reason == rdpa_cpu_rx_reason_unknown_da)
    {
        rdd_emac_id_t emac_id;

        /* set for ports within mask, unset for rest */
        for (emac_id = RDD_EMAC_ID_START; emac_id < RDD_EMAC_ID_COUNT; emac_id++)
            _rdd_cpu_reason_and_src_emac_to_rx_meter_cfg(reason, meter, emac_id, src_port_mask & RDD_EMAC_PORT_TO_VECTOR(emac_id, 0));
    }
    else
    {
        table = RDD_US_CPU_REASON_TO_METER_TABLE_PTR();
        entry = &(table->entry[reason]);

        RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE(meter, entry);
    }
}

static void _rdd_cpu_reason_to_cpu_rx_meter_ds(rdpa_cpu_reason reason, rdd_cpu_rx_meter meter)
{
    RDD_DS_CPU_REASON_TO_METER_TABLE_DTS *table;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS *entry;

    table = RDD_DS_CPU_REASON_TO_METER_TABLE_PTR();
    entry = &(table->entry[reason]);

    RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE(meter, entry);
}

int rdd_cpu_reason_to_cpu_rx_meter(rdpa_cpu_reason reason, rdd_cpu_rx_meter meter, rdpa_traffic_dir dir, uint32_t src_port_mask)
{
    /* check the validity of the input parameters - CPU-RX reason */
    if (reason >= RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE)
        return BDMF_ERR_PARM;

    if (dir == rdpa_dir_ds)
        _rdd_cpu_reason_to_cpu_rx_meter_ds(reason, meter);
    else
        _rdd_cpu_reason_to_cpu_rx_meter_us(reason, meter, src_port_mask);

    return 0;
}

bdmf_error_t rdd_vport_pm_counters_get(rdd_vport_id_t vport, bdmf_boolean counters_clear, rdd_vport_pm_counters_t *pm_counters)
{
    rdd_vport_pm_counters_t  *pm_counters_buffer_ptr;
    bdmf_error_t rdd_error;
    unsigned long flags;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    /* read pm counters of a single port and reset its value */
    rdd_error = rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_BRIDGE_PORT_PM_COUNTERS_GET, PICO_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, vport, 0, 0, 1);

    if (rdd_error)
    {
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        return rdd_error;
    }

    pm_counters_buffer_ptr = (rdd_vport_pm_counters_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + PM_COUNTERS_BUFFER_ADDRESS);

    if (vport == RDD_WAN0_VPORT)
    {
        pm_counters->rx_valid = swap4bytes(pm_counters_buffer_ptr->rx_valid);
        pm_counters->tx_valid = swap4bytes(pm_counters_buffer_ptr->tx_valid);
    }
    else
    {
        pm_counters->rx_valid = 0;
        pm_counters->tx_valid = 0;
    }
    pm_counters->error_rx_bpm_congestion = swap2bytes(pm_counters_buffer_ptr->error_rx_bpm_congestion);
    pm_counters->bridge_filtered_packets = swap2bytes(pm_counters_buffer_ptr->bridge_filtered_packets);
    pm_counters->bridge_tx_packets_discard = swap2bytes(pm_counters_buffer_ptr->bridge_tx_packets_discard);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    return 0;
}

int rdd_various_counters_get(rdpa_traffic_dir dir, uint32_t mask, bdmf_boolean clear_counters, rdd_various_counters_t *various_counters)
{
    uint32_t ingress_filter_idx;
    uint32_t l4_filter_idx;
    uint32_t counters_group;
    bdmf_error_t rc = BDMF_ERR_OK;

    counters_group = (dir == rdpa_dir_ds) ? DOWNSTREAM_VARIOUS_PACKETS_GROUP : UPSTREAM_VARIOUS_PACKETS_GROUP;

    if (mask & CONNECTION_ACTION_DROP_COUNTER_MASK)
    {
        rc = rdd_2_bytes_counter_get(counters_group, CONNECTION_ACTION_DROP_COUNTER_OFFSET, &various_counters->connection_action_drop);

        if (rc)
            goto exit;
    }

    if (mask & TPID_DETECT_DROP_COUNTER_MASK)
    {
        rc = rdd_2_bytes_counter_get(counters_group, TPID_DETECT_DROP_COUNTER_OFFSET, &various_counters->tpid_detect_drop);

        if (rc)
            goto exit;
    }

#ifdef UNDEF
     if (mask & ETHERNET_FLOW_ACTION_DROP_COUNTER_MASK)
     {
         rc = rdd_2_bytes_counter_get(counters_group, ETHERNET_FLOW_DROP_ACTION_COUNTER_OFFSET, &various_counters->eth_flow_action_drop);

         if (rc)
             goto exit;
     }

     if (mask & VLAN_SWITCHING_DROP_COUNTER_MASK)
     {
         rc = rdd_2_bytes_counter_get(counters_group, VLAN_SWITCHING_DROP_COUNTER_OFFSET, &various_counters->vlan_switching_drop);

         if (rc)
             goto exit;
     }

     if (mask & SA_LOOKUP_FAILURE_DROP_COUNTER_MASK)
     {
         rc = rdd_2_bytes_counter_get(counters_group, SA_LOOKUP_FAILURE_DROP_COUNTER_OFFSET, &various_counters->sa_lookup_failure_drop);

         if (rc)
             goto exit;
     }

     if (mask & DA_LOOKUP_FAILURE_DROP_COUNTER_MASK)
     {
         rc = rdd_2_bytes_counter_get(counters_group, DA_LOOKUP_FAILURE_DROP_COUNTER_OFFSET, &various_counters->da_lookup_failure_drop);

         if (rc)
             goto exit;
     }

     if (mask & SA_ACTION_DROP_COUNTER_MASK)
     {
         rc = rdd_2_bytes_counter_get(counters_group, SA_ACTION_DROP_COUNTER_OFFSET, &various_counters->sa_action_drop);

         if (rc)
             goto exit;
     }

     if (mask & DA_ACTION_DROP_COUNTER_MASK)
     {
         rc = rdd_2_bytes_counter_get(counters_group, DA_ACTION_DROP_COUNTER_OFFSET, &various_counters->da_action_drop);

         if (rc)
             goto exit;
     }

     if (mask & FORWARDING_MATRIX_DISABLED_DROP_COUNTER_MASK)
     {
         rc = rdd_2_bytes_counter_get(counters_group, FORWARDING_MATRIX_DISABLED_DROP_COUNTER_OFFSET, &various_counters->forwarding_matrix_disabled_drop);

         if (rc)
             goto exit;
     }
     if (mask & INVALID_SUBNET_IP_DROP_COUNTER_MASK)
     {
         rc = rdd_2_bytes_counter_get(counters_group, INVALID_SUBNET_IP_DROP_COUNTER_OFFSET, &various_counters->invalid_subnet_ip_drop);

         if (rc)
             goto exit;
     }
#endif

    if (mask & INGRESS_FILTERS_DROP_COUNTER_MASK)
    {
        for (ingress_filter_idx = 0; ingress_filter_idx < RDD_FILTER_LAST; ingress_filter_idx++)
        {
            rc = rdd_2_bytes_counter_get(counters_group, INGRESS_FILTER_DROP_SUB_GROUP_OFFSET + ingress_filter_idx,
                &various_counters->ingress_filters_drop[ingress_filter_idx]);

            if (rc)
                goto exit;
        }
    }

    if (mask & IP_VALIDATION_FILTER_DROP_COUNTER_MASK)
    {
        for (ingress_filter_idx = 0; ingress_filter_idx < 2; ingress_filter_idx++)
        {
            rc = rdd_2_bytes_counter_get(counters_group, INGRESS_FILTER_IP_VALIDATION_GROUP_OFFSET + ingress_filter_idx,
                &various_counters->ip_validation_filter_drop[ingress_filter_idx]);

            if (rc)
                goto exit;
        }
    }

    if (mask & L4_FILTERS_DROP_COUNTER_MASK)
    {
        for (l4_filter_idx = 0; l4_filter_idx <= RDD_LAYER4_FILTER_UNKNOWN; l4_filter_idx++)
        {
            rc = rdd_2_bytes_counter_get(counters_group, LAYER4_FILTER_DROP_SUB_GROUP_OFFSET + l4_filter_idx,
                &various_counters->layer4_filters_drop[l4_filter_idx]);

            if (rc)
                goto exit;
        }
    }

    if (dir == rdpa_dir_ds)
    {
#ifdef UNDEF
        if (mask & INVALID_LAYER2_PROTOCOL_DROP_COUNTER_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, INVALID_LAYER2_PROTOCOL_DROP_COUNTER_OFFSET, &various_counters->invalid_layer2_protocol_drop);

            if (rc)
                goto exit;
        }

        if (mask & FIREWALL_DROP_COUNTER_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, FIREWALL_DROP_COUNTER_OFFSET, &various_counters->firewall_drop);

            if (rc)
                goto exit;
        }

        if (mask & DST_MAC_NON_ROUTER_DROP_COUNTER_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, DST_MAC_NON_ROUTER_COUNTER_OFFSET, &various_counters->dst_mac_non_router_drop);

            if (rc)
                goto exit;
        }

        if (mask & EMAC_LOOPBACK_DROP_COUNTER_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, EMAC_LOOPBACK_DROP_COUNTER, &various_counters->emac_loopback_drop);

            if (rc)
                goto exit;
        }

        if (mask & IPTV_LAYER3_DROP_COUNTER_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, IPTV_LAYER3_DROP_COUNTER_OFFSET, &various_counters->iptv_layer3_drop);

            if (rc)
                goto exit;
        }

        if (mask & DOWNSTREAM_POLICERS_DROP_COUNTER_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, DOWNSTREAM_POLICERS_DROP_COUNTER_OFFSET, &various_counters->downstream_policers_drop);

            if (rc)
                goto exit;
        }

         if (mask & DUAL_STACK_LITE_CONGESTION_DROP_COUNTER_MASK)
         {
            rc = rdd_2_bytes_counter_get(counters_group, DUAL_STACK_LITE_CONGESTION_DROP_COUNTER_OFFSET, &various_counters->dual_stack_lite_congestion_drop);

            if (rc)
                goto exit;
        }

        rc = rdd_2_bytes_counter_get(counters_group, DOWNSTREAM_PARALLEL_PROCESSING_NO_SLAVE_WAIT_OFFSET, &various_counters->ds_parallel_processing_no_avialable_slave);

        if (rc)
            goto exit;

        rc = rdd_2_bytes_counter_get(counters_group, DOWNSTREAM_PARALLEL_PROCESSING_REORDER_WAIT_OFFSET, &various_counters->ds_parallel_processing_reorder_slaves);

        if (rc)
            goto exit;

        if (mask & ABSOLUTE_ADDRESS_LIST_OVERFLOW_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, ABSOLUTE_ADDRESS_LIST_OVERFLOW_OFFSET, &various_counters->absolute_address_list_overflow_drop);

            if (rc)
                goto exit;
        }

        if (mask & WLAN_MCAST_COPY_FAILED_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, WLAN_MCAST_COPY_FAILED_OFFSET, &various_counters->wlan_mcast_copy_failed_drop);

            if (rc)
                goto exit;
        }

        if (mask & WLAN_MCAST_OVERFLOW_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, WLAN_MCAST_OVERFLOW_OFFSET, &various_counters->wlan_mcast_overflow_drop);

            if (rc)
                goto exit;
        }

        if (mask & WLAN_MCAST_DROP_COUNTER_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, WLAN_MCAST_DROP_COUNTER_OFFSET, &various_counters->wlan_mcast_drop);

            if (rc)
                goto exit;
        }

#endif
    }
    else
    {
        if (mask & ACL_OUI_DROP_COUNTER_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, ACL_OUI_DROP_COUNTER_OFFSET, &various_counters->acl_oui_drop);

            if (rc)
                goto exit;
        }

#ifdef UNDEF
        if (mask & ACL_L2_DROP_COUNTER_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, ACL_LAYER2_DROP_COUNTER_OFFSET, &various_counters->acl_l2_drop);

            if (rc)
                goto exit;
        }

        if (mask & ACL_L3_DROP_COUNTER_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, ACL_LAYER3_DROP_COUNTER_OFFSET, &various_counters->acl_l3_drop);

            if (rc)
                goto exit;
        }

        if (mask & LOCAL_SWITCHING_CONGESTION_COUNTER_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, LAN_ENQUEUE_CONGESTION_COUNTER_OFFSET, &various_counters->local_switching_congestion);

            if (rc)
                goto exit;
        }

        if (mask & EPON_DDR_QUEUEU_DROP_COUNTER_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, EPON_DDR_QUEUES_COUNTER_OFFSET, &various_counters->us_ddr_queue_drop);

            if (rc)
                goto exit;
        }

        if (mask & DHD_IH_CONGESTION_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, DHD_IH_CONGESTION_OFFSET, &various_counters->dhd_ih_congestion_drop);

            if (rc)
                goto exit;
        }

        if (mask & DHD_MALLOC_FAILED_MASK)
        {
            rc = rdd_2_bytes_counter_get(counters_group, DHD_MALLOC_FAILED_OFFSET, &various_counters->dhd_malloc_failed_drop);

            if (rc)
                goto exit;
        }
#endif
    }

exit:
    return rc;
}

int rdd_cpu_rx_meter_drop_counter_get(rdd_cpu_rx_meter cpu_meter, rdpa_traffic_dir dir, uint32_t *drop_counter)
{
    int rdd_error;
    uint16_t drop_counter_val;

    rdd_error = rdd_2_bytes_counter_get(CPU_RX_METERS_DROPPED_PACKETS_GROUP, dir * CPU_RX_METERS_DROPPED_PACKETS_UPSTREAM_OFFSET + cpu_meter, &drop_counter_val);
    *drop_counter = (uint32_t)drop_counter_val;

    return rdd_error;
}

int rdd_cpu_reason_to_cpu_rx_queue(rdpa_cpu_reason cpu_reason, rdd_cpu_rx_queue queue_id, rdpa_traffic_dir direction)
{
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS *table;
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_DTS *entry;
    uint8_t cpu_queue;

    /* check the validity of the input parameters - CPU-RX reason */
    if (cpu_reason >= RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE)
        return BDMF_ERR_PARM;

    /* check the validity of the input parameters - CPU-RX queue-id */
    if (queue_id > CPU_RX_QUEUE_LAST)
        return BDMF_ERR_PARM;

    if (direction == rdpa_dir_ds)
    {
        table = (RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) +
            DS_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_ADDRESS);
    }
    else
    {
        table = (RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_1_OFFSET) +
            US_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_ADDRESS - sizeof(RUNNER_COMMON));
    }

    entry = &(table->entry[cpu_reason]);
    cpu_queue = queue_id;

    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_CPU_RX_QUEUE_WRITE(cpu_queue, entry);

    return 0;
}

