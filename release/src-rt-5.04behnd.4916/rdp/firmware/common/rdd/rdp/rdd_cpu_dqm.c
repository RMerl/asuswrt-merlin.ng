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

#define _RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE  RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE

extern uint32_t g_cpu_message_queue_write_ptr[2];

#ifdef FIRMWARE_INIT
extern unsigned int cpu_rx_ring_base_addr_ptr;
#endif

#if !defined(RDD_BASIC)
#if !defined(FIRMWARE_INIT)
#endif /* !defined(FIRMWARE_INIT) */
#endif /* !defined(RDD_BASIC) */

void rdd_cpu_rx_init(void)
{
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS *cpu_reason_to_cpu_rx_queue_table_ptr;
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_DTS *cpu_reason_to_cpu_rx_queue_entry_ptr;
    RDD_DS_CPU_REASON_TO_METER_TABLE_DTS *ds_cpu_reason_to_meter_table_ptr;
    RDD_US_CPU_REASON_TO_METER_TABLE_DTS *us_cpu_reason_to_meter_table_ptr;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS *cpu_reason_to_meter_entry_ptr;
    RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_DTS *cpu_reason_and_vport_to_meter_table_ptr;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS *cpu_reason_and_vport_to_meter_entry_ptr;
    rdd_vport_id_t vport;
    uint32_t cpu_reason_per_port_index;
    uint8_t cpu_reason;

    /* Note: CPU RX Ingress queue setup is now done as part of rdd_init*/

    cpu_reason_to_cpu_rx_queue_table_ptr  = (RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + DS_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_ADDRESS);

    /* set reason_direct_flow to queue_0 for backward compatibility */
    cpu_reason_to_cpu_rx_queue_entry_ptr = &(cpu_reason_to_cpu_rx_queue_table_ptr->entry[rdpa_cpu_rx_reason_direct_flow]);
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_CPU_RX_QUEUE_WRITE(CPU_RX_QUEUE_ID_0, cpu_reason_to_cpu_rx_queue_entry_ptr);


    ds_cpu_reason_to_meter_table_ptr = RDD_DS_CPU_REASON_TO_METER_TABLE_PTR();
    us_cpu_reason_to_meter_table_ptr = RDD_US_CPU_REASON_TO_METER_TABLE_PTR();

    for (cpu_reason = rdpa_cpu_rx_reason_oam; cpu_reason < rdpa_cpu_reason__num_of; cpu_reason++)
    {
        cpu_reason_to_meter_entry_ptr = &(ds_cpu_reason_to_meter_table_ptr->entry[cpu_reason]);

        RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE(CPU_RX_METER_DISABLE, cpu_reason_to_meter_entry_ptr);

        cpu_reason_to_meter_entry_ptr = &(us_cpu_reason_to_meter_table_ptr->entry[cpu_reason]);

        RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE(CPU_RX_METER_DISABLE, cpu_reason_to_meter_entry_ptr);

        if ((cpu_reason != rdpa_cpu_rx_reason_mcast) && (cpu_reason != rdpa_cpu_rx_reason_bcast) && (cpu_reason != rdpa_cpu_rx_reason_unknown_da))
            continue;

        cpu_reason_and_vport_to_meter_table_ptr = RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_PTR();

        cpu_reason_per_port_index = cpu_reason_to_cpu_per_port_reason_index(cpu_reason);

        for (vport = RDD_VPORT_ID_0; vport < RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_SIZE2; vport++)
        {
            cpu_reason_and_vport_to_meter_entry_ptr = &(cpu_reason_and_vport_to_meter_table_ptr->entry[cpu_reason_per_port_index][vport]);

            RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE(CPU_RX_METER_DISABLE, cpu_reason_and_vport_to_meter_entry_ptr);
        }
    }
}

void rdd_cpu_tx_init(void)
{
    uint16_t *rx_dispatch_fifo_read_ptr;

    /* dispatch fifo read pointer initialize */
    rx_dispatch_fifo_read_ptr = (uint16_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_CPU_RX_DISPATCH_FIFO_READ_PTR_ADDRESS);

    MWRITE_16(rx_dispatch_fifo_read_ptr, US_CPU_TX_BBH_DESCRIPTORS_ADDRESS);

    _rdd_runner_flow_header_descriptor_init(RUNNER_PRIVATE_0_OFFSET, DS_RUNNER_FLOW_HEADER_DESCRIPTOR_ADDRESS,
        CPU_TX_RGW_DS_FORWARD_THREAD_NUMBER, RDD_DS_IH_PACKET_HEADROOM_OFFSET, RDD_RUNNER_FLOW_RUNNER_A_IH_BUFFER);

    _rdd_runner_flow_header_descriptor_init(RUNNER_PRIVATE_1_OFFSET, US_RUNNER_FLOW_HEADER_DESCRIPTOR_ADDRESS,
        CPU_TX_RGW_US_FORWARD_THREAD_NUMBER, RDD_US_IH_PACKET_HEADROOM_OFFSET, RDD_RUNNER_FLOW_RUNNER_B_IH_BUFFER);
#if defined(CONFIG_BCM_PKTRUNNER_GSO)
    bdmf_gso_desc_pool_create(RUNNER_MAX_GSO_DESC);
#endif
}

void rdd_dqm_init(uint32_t dqm_id)
{
    RDD_DQM_DESCRIPTORS_TABLE_DTS *table;
    RDD_DQM_DESCRIPTOR_DTS *descr;

    table = (RDD_DQM_DESCRIPTORS_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + DQM_DESCRIPTORS_TABLE_ADDRESS);

    descr = &(table->entry[dqm_id]);

    RDD_DQM_DESCRIPTOR_AVAILABLE_PACKET_COUNTER_WRITE(0, descr);
    RDD_DQM_DESCRIPTOR_DROP_COUNTER_WRITE(0, descr);
}

int rdd_cpu_rx_queue_discard_get(rdd_cpu_rx_queue ring_id, uint16_t *number_of_packets)
{
    RDD_DQM_DESCRIPTORS_TABLE_DTS *table;
    RDD_DQM_DESCRIPTOR_DTS *descr;
    unsigned long flags;

    /* check the validity of the input parameters - CPU-RX queue index */
    if (ring_id >= RDD_DQM_DESCRIPTORS_TABLE_SIZE)
        return BDMF_ERR_PARM;

    table = (RDD_DQM_DESCRIPTORS_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + DQM_DESCRIPTORS_TABLE_ADDRESS);

    descr = &(table->entry[ring_id]);

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    RDD_DQM_DESCRIPTOR_DROP_COUNTER_READ(*number_of_packets, descr);
    RDD_DQM_DESCRIPTOR_DROP_COUNTER_WRITE(0, descr);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    return 0;
}

int _rdd_cpu_message_send(rdd_cpu_message_type_t msg_type, rdd_cluster_t runner_cluster,
    uint32_t parameter_1, uint32_t parameter_2, uint32_t parameter_3, bdmf_boolean is_wait)
{
    RUNNER_REGS_CFG_CPU_WAKEUP runner_cpu_wakeup_register;
    RDD_CPU_MESSAGE_DESCRIPTOR_DTS *cpu_message_descriptor_ptr;
    uint32_t cpu_message_descriptor_valid;

    if (runner_cluster == RDD_CLUSTER_0)
        cpu_message_descriptor_ptr = (RDD_CPU_MESSAGE_DESCRIPTOR_DTS *)(RDD_CLUSTER_0_CPU_MESSAGE_QUEUE_PTR()) +
            g_cpu_message_queue_write_ptr[RDD_CLUSTER_0];
    else
        cpu_message_descriptor_ptr = (RDD_CPU_MESSAGE_DESCRIPTOR_DTS *)(RDD_CLUSTER_1_CPU_MESSAGE_QUEUE_PTR()) +
            g_cpu_message_queue_write_ptr[RDD_CLUSTER_1];

    /* if the descriptor is valid then the CPU message queue is full and the message will not be sent */
    RDD_CPU_MESSAGE_DESCRIPTOR_VALID_READ(cpu_message_descriptor_valid, cpu_message_descriptor_ptr);

    if (cpu_message_descriptor_valid)
        return BDMF_ERR_TOO_MANY_REQS;

    RDD_CPU_MESSAGE_DESCRIPTOR_TYPE_WRITE(msg_type, cpu_message_descriptor_ptr);

    switch (msg_type)
    {
    case RDD_CPU_MESSAGE_FLUSH_WAN_TX_QUEUE:
        RDD_CPU_MESSAGE_DESCRIPTOR_FLUSH_WAN_TX_QUEUE_TX_QUEUE_PTR_WRITE(parameter_1, cpu_message_descriptor_ptr);
        break;

    case RDD_CPU_MESSAGE_FLUSH_LAN_TX_QUEUE:
        RDD_CPU_MESSAGE_DESCRIPTOR_FLUSH_LAN_TX_QUEUE_EMAC_WRITE(parameter_1, cpu_message_descriptor_ptr);
        RDD_CPU_MESSAGE_DESCRIPTOR_FLUSH_LAN_TX_QUEUE_QUEUE_WRITE(parameter_2, cpu_message_descriptor_ptr);
        break;

    case RDD_CPU_MESSAGE_RX_FLOW_PM_COUNTERS_GET:
    case RDD_CPU_MESSAGE_TX_FLOW_PM_COUNTERS_GET:
        RDD_CPU_MESSAGE_DESCRIPTOR_FLOW_PM_COUNTERS_GET_FLOW_WRITE(parameter_1, cpu_message_descriptor_ptr);
        break;

    case RDD_CPU_MESSAGE_BRIDGE_PORT_PM_COUNTERS_GET:
        RDD_CPU_MESSAGE_DESCRIPTOR_BRIDGE_PORT_PM_COUNTERS_GET_SRC_BRIDGE_PORT_WRITE(parameter_1, cpu_message_descriptor_ptr);
        break;

    case RDD_CPU_MESSAGE_PM_COUNTER_GET:
        RDD_CPU_MESSAGE_DESCRIPTOR_PM_COUNTER_GET_GROUP_WRITE(parameter_1, cpu_message_descriptor_ptr);
        RDD_CPU_MESSAGE_DESCRIPTOR_PM_COUNTER_GET_COUNTER_WRITE(parameter_2, cpu_message_descriptor_ptr);
        RDD_CPU_MESSAGE_DESCRIPTOR_PM_COUNTER_GET_IS_4_BYTES_WRITE(parameter_3, cpu_message_descriptor_ptr);
        break;

    default:
        break;
    }

    RDD_CPU_MESSAGE_DESCRIPTOR_VALID_WRITE(1, cpu_message_descriptor_ptr);

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    if (runner_cluster == RDD_CLUSTER_0)
    {
        g_cpu_message_queue_write_ptr[runner_cluster]++;
        g_cpu_message_queue_write_ptr[runner_cluster] &= ~RDD_CLUSTER_0_CPU_MESSAGE_QUEUE_SIZE;
    }
    else
    {
        g_cpu_message_queue_write_ptr[runner_cluster]++;
        g_cpu_message_queue_write_ptr[runner_cluster] &= ~RDD_CLUSTER_1_CPU_MESSAGE_QUEUE_SIZE;
    }

    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.urgent_req = 0;

    runner_cpu_wakeup_register.req_trgt = CPU_MESSAGE_THREAD_NUMBER / 32;
    runner_cpu_wakeup_register.thread_num = CPU_MESSAGE_THREAD_NUMBER % 32;

    if (runner_cluster == RDD_CLUSTER_0)
        RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);
    else
        RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);

#if !defined(BDMF_SYSTEM_SIM)
    if (is_wait)
    {
        /* wait for the cpu message thread to finish the current message */
        uint32_t break_out_counter = 10000;

        while (--break_out_counter)
        {
            RDD_CPU_MESSAGE_DESCRIPTOR_VALID_READ(cpu_message_descriptor_valid,
                ((volatile RDD_CPU_MESSAGE_DESCRIPTOR_DTS *)cpu_message_descriptor_ptr));

            if (cpu_message_descriptor_valid == 0)
                break;
        }

        if (break_out_counter == 0)
            BDMF_TRACE_RET(BDMF_ERR_INTERNAL, "CPU -> Runner message timeout\n");
    }
#endif
    return BDMF_ERR_OK;
}

bdmf_error_t rdd_flow_pm_counters_get(uint32_t flow_id, rdd_flow_pm_counters_type_t  flow_pm_counters_type, bdmf_boolean counters_clear, rdd_flow_pm_counters_t *pm_counters)
{
    rdd_flow_pm_counters_t *pm_counters_buffer_ptr;
    bdmf_error_t rdd_error;
    unsigned long flags;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    /* read pm counters of a single port and reset its value */
    rdd_error = _rdd_cpu_message_send(flow_pm_counters_type, RDD_CLUSTER_0, flow_id, 0, 0, 1);

    if (rdd_error)
    {
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        return rdd_error;
    }

    pm_counters_buffer_ptr = (rdd_flow_pm_counters_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + PM_COUNTERS_BUFFER_ADDRESS);

    pm_counters->good_rx_packet = pm_counters_buffer_ptr->good_rx_packet;
    pm_counters->good_rx_bytes = pm_counters_buffer_ptr->good_rx_bytes;
    pm_counters->good_tx_packet = pm_counters_buffer_ptr->good_tx_packet;
    pm_counters->good_tx_bytes = pm_counters_buffer_ptr->good_tx_bytes;

    pm_counters->error_rx_packets_discard = pm_counters_buffer_ptr->error_rx_packets_discard;
    pm_counters->error_tx_packets_discard = pm_counters_buffer_ptr->error_tx_packets_discard;

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    return 0;
}

int rdd_iptv_rx_pm_counters_get(rdd_vport_pm_counters_t *pm_counters)
{
    int rdd_error;
    uint32_t counter;

    rdd_error = rdd_4_bytes_counter_get(WAN_BRIDGE_PORT_GROUP, WAN_IPTV_RX_VALID_SUB_GROUP_OFFSET, &counter);

    if (!rdd_error)
    {
        pm_counters->rx_valid = counter;
        pm_counters->tx_valid = 0;
    }
    return rdd_error;
}

#if defined(CONFIG_BCM_PKTRUNNER_GSO)
int rdd_gso_counters_get(RDD_GSO_COUNTERS_ENTRY_DTS *gso_counters_ptr)
{
    RDD_GSO_CONTEXT_ENTRY_DTS   *gso_context_ptr_tmp;
    unsigned long               flags;

    gso_context_ptr_tmp = (RDD_GSO_CONTEXT_ENTRY_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_GSO_CONTEXT_TABLE_ADDRESS);

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    RDD_GSO_CONTEXT_ENTRY_RX_PACKETS_READ(gso_counters_ptr->rx_packets, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_RX_OCTETS_READ(gso_counters_ptr->rx_octets, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_TX_PACKETS_READ(gso_counters_ptr->tx_packets, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_TX_OCTETS_READ(gso_counters_ptr->tx_octets, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_DROPPED_PACKETS_READ(gso_counters_ptr->dropped_packets, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_DROPPED_NO_BPM_BUFFER_READ(gso_counters_ptr->dropped_no_bpm_buffer, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_DROPPED_PARSE_FAILED_READ(gso_counters_ptr->dropped_parse_failed, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_DROPPED_LINEAR_LENGTH_INVALID_READ(gso_counters_ptr->dropped_linear_length_invalid, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_QUEUE_FULL_READ(gso_counters_ptr->queue_full, gso_context_ptr_tmp);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);

    return BDMF_ERR_OK;
}

int rdd_gso_context_get(RDD_GSO_CONTEXT_ENTRY_DTS *gso_context_ptr)
{
    RDD_GSO_CONTEXT_ENTRY_DTS   *gso_context_ptr_tmp;
    unsigned long               flags;

    gso_context_ptr_tmp = (RDD_GSO_CONTEXT_ENTRY_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_GSO_CONTEXT_TABLE_ADDRESS);

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    RDD_GSO_CONTEXT_ENTRY_RX_BBH_DESCRIPTOR_0_READ(gso_context_ptr->rx_bbh_descriptor_0, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_RX_BBH_DESCRIPTOR_1_READ(gso_context_ptr->rx_bbh_descriptor_1, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_TX_BBH_DESCRIPTOR_0_READ(gso_context_ptr->tx_bbh_descriptor_0, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_TX_BBH_DESCRIPTOR_1_READ(gso_context_ptr->tx_bbh_descriptor_1, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_SUMMARY_READ(gso_context_ptr->summary, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_IP_HEADER_OFFSET_READ(gso_context_ptr->ip_header_offset, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_IP_HEADER_LENGTH_READ(gso_context_ptr->ip_header_length, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_IP_TOTAL_LENGTH_READ(gso_context_ptr->ip_total_length, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_IP_ID_READ(gso_context_ptr->ip_id, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_IP_FRAGMENT_OFFSET_READ(gso_context_ptr->ip_fragment_offset, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_IP_FLAGS_READ(gso_context_ptr->ip_flags, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_IP_PROTOCOL_READ(gso_context_ptr->ip_protocol, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_IPV4_CSUM_READ(gso_context_ptr->ipv4_csum, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_PACKET_HEADER_LENGTH_READ(gso_context_ptr->packet_header_length, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_SEG_COUNT_READ(gso_context_ptr->seg_count, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_NR_FRAGS_READ(gso_context_ptr->nr_frags, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_FRAG_INDEX_READ(gso_context_ptr->frag_index, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_TCP_UDP_HEADER_OFFSET_READ(gso_context_ptr->tcp_udp_header_offset, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_TCP_UDP_HEADER_LENGTH_READ(gso_context_ptr->tcp_udp_header_length, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_TCP_UDP_TOTAL_LENGTH_READ(gso_context_ptr->tcp_udp_total_length, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_TCP_SEQUENCE_READ(gso_context_ptr->tcp_sequence, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_TCP_FLAGS_READ(gso_context_ptr->tcp_flags, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_VERSION_READ(gso_context_ptr->version, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_TCP_UDP_CSUM_READ(gso_context_ptr->tcp_udp_csum, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_MSS_READ(gso_context_ptr->mss, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_MSS_ADJUST_READ(gso_context_ptr->mss_adjust, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_SEG_LENGTH_READ(gso_context_ptr->seg_length, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_SEG_BYTES_LEFT_READ(gso_context_ptr->seg_bytes_left, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_MAX_CHUNK_LENGTH_READ(gso_context_ptr->max_chunk_length, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_CHUNK_BYTES_LEFT_READ(gso_context_ptr->chunk_bytes_left, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_PAYLOAD_BYTES_LEFT_READ(gso_context_ptr->payload_bytes_left, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_PAYLOAD_PTR_READ(gso_context_ptr->payload_ptr, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_PAYLOAD_LENGTH_READ(gso_context_ptr->payload_length, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_LINEAR_LENGTH_READ(gso_context_ptr->linear_length, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_TX_PACKET_PTR_READ(gso_context_ptr->tx_packet_ptr, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_TX_PACKET_LENGTH_READ(gso_context_ptr->tx_packet_length, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_UDP_FIRST_PACKET_LENGTH_READ(gso_context_ptr->udp_first_packet_length, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_UDP_FIRST_PACKET_PTR_READ(gso_context_ptr->udp_first_packet_ptr, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_UDP_FIRST_PACKET_BUFFER_NUMBER_READ(gso_context_ptr->udp_first_packet_buffer_number, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_BPM_BUFFER_NUMBER_READ(gso_context_ptr->bpm_buffer_number, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_PACKET_LENGTH_READ(gso_context_ptr->packet_length, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_IPV6_IP_ID_READ(gso_context_ptr->ipv6_ip_id, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_AUTH_STATE_3_READ(gso_context_ptr->auth_state_3, gso_context_ptr_tmp);
    RDD_GSO_CONTEXT_ENTRY_DEBUG_0_READ(gso_context_ptr->debug_0, gso_context_ptr_tmp);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);

    return BDMF_ERR_OK;
}

int rdd_gso_desc_get(RDD_GSO_DESC_ENTRY_DTS *gso_desc_ptr)
{
    RDD_GSO_DESC_ENTRY_DTS  *gso_desc_ptr_tmp;
    unsigned long           flags;
#if 0
    int                     nr_frags;
#endif

    gso_desc_ptr_tmp = (RDD_GSO_DESC_ENTRY_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_GSO_DESC_TABLE_ADDRESS);

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    RDD_GSO_DESC_ENTRY_DATA_READ(gso_desc_ptr->data, gso_desc_ptr_tmp);
    RDD_GSO_DESC_ENTRY_LEN_READ(gso_desc_ptr->len, gso_desc_ptr_tmp);
    RDD_GSO_DESC_ENTRY_LINEAR_LEN_READ(gso_desc_ptr->linear_len, gso_desc_ptr_tmp);
    RDD_GSO_DESC_ENTRY_MSS_READ(gso_desc_ptr->mss, gso_desc_ptr_tmp);
    RDD_GSO_DESC_ENTRY_IS_ALLOCATED_READ(gso_desc_ptr->is_allocated, gso_desc_ptr_tmp);
    RDD_GSO_DESC_ENTRY_NR_FRAGS_READ(gso_desc_ptr->nr_frags, gso_desc_ptr_tmp);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);

    return BDMF_ERR_OK;
}
#endif /* CONFIG_BCM_PKTRUNNER_GSO */
