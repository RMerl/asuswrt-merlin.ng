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

#ifdef CC_RDD_CPU_SPDSVC_DEBUG
bdmf_sysb  g_spdsvc_setup_sysb_ptr = NULL;
#endif /*CC_RDD_CPU_SPDSVC_DEBUG*/
#if !defined(FIRMWARE_INIT)
cpu_tx_skb_free_indexes_cache_t g_cpu_tx_skb_free_indexes_cache;
uint32_t  g_cpu_tx_pending_free_indexes_counter = 0;
static uint8_t g_dummy_read;
#endif /*!FIRMWARE_INIT*/
uint32_t   g_cpu_tx_abs_packet_limit = 0;
uint16_t  *g_free_skb_indexes_fifo_table = NULL;
uint8_t  **g_cpu_tx_skb_pointers_reference_array = NULL;
rdd_phys_addr_t *g_cpu_tx_data_pointers_reference_array = NULL;
rdd_phys_addr_t g_free_skb_indexes_fifo_table_physical_address = 0;
rdd_phys_addr_t g_free_skb_indexes_fifo_table_physical_address_last_idx = 0;

#if defined(CONFIG_BCM_PKTRUNNER_GSO)
uint32_t  g_cpu_tx_no_free_gso_desc_counter = 0;
uint32_t  g_cpu_tx_sent_abs_gso_packets_counter = 0;
uint32_t  g_cpu_tx_sent_abs_gso_bytes_counter = 0;
#endif

uint32_t  g_cpu_tx_skb_free_indexes_release_ptr = 0;
uint32_t  g_cpu_tx_released_skb_counter = 0;
uint32_t  g_cpu_tx_no_free_skb_counter = 0;
uint32_t  g_cpu_tx_sent_abs_packets_counter = 0;

extern uint8_t g_broadcom_switch_mode;
extern rdd_bridge_port_t g_broadcom_switch_physical_port;
extern rdd_wan_physical_port_t g_wan_physical_port;
extern rdd_wan_tx_pointers_table_t *wan_tx_pointers_table;

uint32_t  g_cpu_tx_queue_write_ptr[4];
uint32_t  g_cpu_tx_queue_free_counter[4] = { 0, 0, 0, 0 };
uint32_t  g_cpu_tx_queue_abs_data_ptr_write_ptr[4];

#ifdef FIRMWARE_INIT
extern uint8_t *cpu_rx_ring_base;
#endif

#if defined(CONFIG_DHD_RUNNER)
extern bdmf_boolean is_dhd_enabled[];
#endif

void rdd_cpu_rx_init(void)
{
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS *cpu_reason_to_cpu_rx_queue_table_ptr;
    RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_DTS *cpu_reason_to_cpu_rx_queue_entry_ptr;
    RDD_DS_CPU_REASON_TO_METER_TABLE_DTS *ds_cpu_reason_to_meter_table_ptr;
    RDD_US_CPU_REASON_TO_METER_TABLE_DTS *us_cpu_reason_to_meter_table_ptr;
    RDD_CPU_REASON_TO_METER_ENTRY_DTS *cpu_reason_to_meter_entry_ptr;
    uint8_t cpu_reason;
#if defined(FIRMWARE_INIT)
    RDD_CPU_RX_DESCRIPTOR_DTS *cpu_rx_descriptor_ptr;
    uint32_t host_buffer_address;
    uint32_t i;

    /* Init Rings */
    cpu_rx_descriptor_ptr  = (RDD_CPU_RX_DESCRIPTOR_DTS *)cpu_rx_ring_base;

    host_buffer_address = SIMULATOR_DDR_RING_OFFSET +
        RDD_RING_DESCRIPTORS_TABLE_SIZE * SIMULATOR_DDR_RING_NUM_OF_ENTRIES * sizeof(RDD_CPU_RX_DESCRIPTOR_DTS);

    for (i = 0; i < RDD_RING_DESCRIPTORS_TABLE_SIZE * 10;
        i++, cpu_rx_descriptor_ptr++, host_buffer_address += RDD_PACKET_BUFFER_SIZE)
    {
        RDD_CPU_RX_DESCRIPTOR_HOST_DATA_BUFFER_POINTER_WRITE(host_buffer_address, cpu_rx_descriptor_ptr);
    }
#endif /*FIRMWARE_INIT*/

    /* Note: CPU RX Ingress queue setup is now done as part of rdd_init*/

    cpu_reason_to_cpu_rx_queue_table_ptr  = (RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + DS_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_ADDRESS);

    /* set cpu reason_direct_flow to queue_0 for backward compatibility */ 
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
    }
}

#if !defined(FIRMWARE_INIT)
extern cpu_tx_skb_free_indexes_cache_t g_cpu_tx_skb_free_indexes_cache;

static inline int rdd_cpu_tx_skb_free_cache_initialize(void)
{
    g_cpu_tx_skb_free_indexes_cache.write = 0;
    g_cpu_tx_skb_free_indexes_cache.read = 0;
    g_cpu_tx_skb_free_indexes_cache.count = 0;
    g_cpu_tx_skb_free_indexes_cache.data = (uint16_t *)CACHED_MALLOC_ATOMIC(sizeof(RDD_FREE_SKB_INDEXES_FIFO_ENTRY_DTS) * g_cpu_tx_abs_packet_limit);

    if (g_cpu_tx_skb_free_indexes_cache.data == NULL)
        return BDMF_ERR_NOMEM;
    return BDMF_ERR_OK;
}

#if !defined(RDD_BASIC)
static int rdd_cpu_tx_free_skb_timer_config(void)
{
    static uint32_t               api_first_time_call = 1;

    if (api_first_time_call)
    {
        rdd_timer_task_config(rdpa_dir_us, FREE_SKB_INDEX_TIMER_PERIOD, FREE_SKB_INDEX_ALLOCATE_CODE_ID);
        rdd_timer_task_config(rdpa_dir_ds, FREE_SKB_INDEX_TIMER_PERIOD, FREE_SKB_INDEX_ALLOCATE_CODE_ID);

        api_first_time_call = 0;
    }

    return BDMF_ERR_OK;
}
#endif /*!defined(RDD_BASIC)*/
#endif /*!defined(FIRMWARE_INIT)*/

void rdd_cpu_tx_init(void)
{
    uint16_t *rx_dispatch_fifo_read_ptr;

    /* dispatch fifo read pointer initialize */
    rx_dispatch_fifo_read_ptr = (uint16_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_CPU_RX_DISPATCH_FIFO_READ_PTR_ADDRESS);

    MWRITE_16(rx_dispatch_fifo_read_ptr, US_CPU_TX_BBH_DESCRIPTORS_ADDRESS);

    g_cpu_tx_queue_write_ptr[FAST_RUNNER_A] = CPU_TX_FAST_QUEUE_ADDRESS;
    g_cpu_tx_queue_write_ptr[FAST_RUNNER_B] = CPU_TX_FAST_QUEUE_ADDRESS;
    g_cpu_tx_queue_write_ptr[PICO_RUNNER_A] = CPU_TX_PICO_QUEUE_ADDRESS;
    g_cpu_tx_queue_write_ptr[PICO_RUNNER_B] = CPU_TX_PICO_QUEUE_ADDRESS;
    g_cpu_tx_queue_abs_data_ptr_write_ptr[FAST_RUNNER_A] = DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS;
    g_cpu_tx_queue_abs_data_ptr_write_ptr[FAST_RUNNER_B] = US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS;
    g_cpu_tx_queue_abs_data_ptr_write_ptr[PICO_RUNNER_A] = DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS;
    g_cpu_tx_queue_abs_data_ptr_write_ptr[PICO_RUNNER_B] = US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS;

    {
        int ii;
        uint32_t  *ih_buffer_bbh_ptr;
        uint16_t *gso_queue_ptr_ptr;

        _rdd_runner_flow_header_descriptor_init(RUNNER_PRIVATE_0_OFFSET, DS_RUNNER_FLOW_HEADER_DESCRIPTOR_ADDRESS,
                                                CPU_TX_FAST_THREAD_NUMBER, RDD_DS_IH_PACKET_HEADROOM_OFFSET, RDD_RUNNER_FLOW_RUNNER_A_IH_BUFFER);

        for (ii = 0; ii < 3; ii++)
        {
            _rdd_runner_flow_header_descriptor_init(RUNNER_PRIVATE_1_OFFSET, US_RUNNER_FLOW_HEADER_DESCRIPTOR_ADDRESS + ii*2*sizeof(uint32_t),
                                                    0 /*thread number*/, RDD_US_IH_PACKET_HEADROOM_OFFSET, RDD_RUNNER_FLOW_RUNNER_B_IH_BUFFER);
        }

        ih_buffer_bbh_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + IH_BUFFER_BBH_POINTER_ADDRESS);

        MWRITE_32(ih_buffer_bbh_ptr, ((BBH_PERIPHERAL_IH << 16) | (RDD_IH_BUFFER_BBH_ADDRESS + RDD_RUNNER_B_IH_BUFFER_BBH_OFFSET)));

        gso_queue_ptr_ptr = (uint16_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + GSO_PICO_QUEUE_PTR_ADDRESS);
        MWRITE_16(gso_queue_ptr_ptr, GSO_PICO_QUEUE_ADDRESS);

#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
        rdd_cpu_tx_skb_free_cache_initialize();
        rdd_initialize_skb_free_indexes_cache();
        rdd_cpu_tx_free_skb_timer_config();
#endif
    }
#if defined(CONFIG_BCM_PKTRUNNER_GSO)
    bdmf_gso_desc_pool_create(RUNNER_MAX_GSO_DESC);
#endif
}

int rdd_cpu_rx_interrupt_coalescing_config(uint32_t ring_id, uint32_t timeout_us, uint32_t max_packet_count)
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_TIMER_TARGET           runner_timer_target_register;
    RUNNER_REGS_CFG_CPU_WAKEUP             runner_cpu_wakeup_register;
#endif
    RDD_INTERRUPT_COALESCING_CONFIG_TABLE_DTS *ic_table_ptr;
    RDD_INTERRUPT_COALESCING_CONFIG_DTS       *ic_entry_ptr;
    RDD_INTERRUPT_COALESCING_TIMER_CONFIG_DTS *ic_timer_config_table_ptr;
    static uint32_t                           api_first_time_call = 1;

    if (ring_id > RDD_INTERRUPT_COALESCING_CONFIG_TABLE_SIZE)
        return BDMF_ERR_RANGE;

    ic_table_ptr = RDD_INTERRUPT_COALESCING_CONFIG_TABLE_PTR();
    ic_entry_ptr =  &(ic_table_ptr->entry[ring_id]);

    RDD_INTERRUPT_COALESCING_CONFIG_CONFIGURED_TIMEOUT_WRITE(timeout_us, ic_entry_ptr);
    RDD_INTERRUPT_COALESCING_CONFIG_CONFIGURED_MAX_PACKET_COUNT_WRITE(max_packet_count, ic_entry_ptr);

    if (api_first_time_call)
    {
        ic_timer_config_table_ptr = (RDD_INTERRUPT_COALESCING_TIMER_CONFIG_DTS *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + INTERRUPT_COALESCING_TIMER_CONFIG_TABLE_ADDRESS);
#if defined(FIRMWARE_INIT)
        RDD_INTERRUPT_COALESCING_TIMER_CONFIG_TIMER_PERIOD_WRITE(10, ic_timer_config_table_ptr);
#else
        RDD_INTERRUPT_COALESCING_TIMER_CONFIG_TIMER_PERIOD_WRITE(INTERRUPT_COALESCING_TIMER_PERIOD, ic_timer_config_table_ptr);

        RUNNER_REGS_0_CFG_TIMER_TARGET_READ(runner_timer_target_register);
        runner_timer_target_register.timer_4_6 = RUNNER_REGS_CFG_TIMER_TARGET_TIMER_4_6_PICO_CORE_VALUE;
        RUNNER_REGS_0_CFG_TIMER_TARGET_WRITE(runner_timer_target_register);

        /* activate the interrupt coalescing task */
        runner_cpu_wakeup_register.req_trgt = CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER / 32;
        runner_cpu_wakeup_register.thread_num = CPU_RX_INTERRUPT_COALESCING_THREAD_NUMBER % 32;
        RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);
#endif /*FIRMWARE_INIT*/
        api_first_time_call = 0;
    }
    return 0;
}

void rdd_ring_init(uint32_t ring_id, uint8_t unused0, rdd_phys_addr_t ring_address, uint32_t number_of_entries,
    uint32_t size_of_entry, uint32_t interrupt_id, uint32_t unused1, bdmf_phys_addr_t unused2, uint8_t unused3)
{
    RDD_RING_DESCRIPTORS_TABLE_DTS *table;
    RDD_RING_DESCRIPTOR_DTS *descr;

    table = RDD_RING_DESCRIPTORS_TABLE_PTR();

    descr = &(table->entry[ring_id]);

    RDD_RING_DESCRIPTOR_ENTRIES_COUNTER_WRITE(0, descr);
    RDD_RING_DESCRIPTOR_SIZE_OF_ENTRY_WRITE(size_of_entry, descr);
    RDD_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_WRITE(number_of_entries, descr);
    RDD_RING_DESCRIPTOR_INTERRUPT_ID_WRITE(1 << interrupt_id, descr);
    RDD_RING_DESCRIPTOR_RING_POINTER_WRITE(ring_address, descr);
}

int rdd_ring_destroy(uint32_t  ring_id)
{
    int rdd_error = BDMF_ERR_OK;
    unsigned long flags = 0;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

#if !defined(FIRMWARE_INIT)
    rdd_error = rdd_cpu_tx_send_message(RDD_CPU_TX_MESSAGE_RING_DESTROY, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, ring_id, 0, 0, 1);
#endif
    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);

    return rdd_error;
}

int rdd_cpu_rx_queue_discard_get(rdd_cpu_rx_queue ring_id, uint16_t *number_of_packets)
{
    RDD_RING_DESCRIPTORS_TABLE_DTS *table;
    RDD_RING_DESCRIPTOR_DTS *descr;
    unsigned long flags;

    /* check the validity of the input parameters - CPU-RX queue index */
    if (ring_id >= RDD_RING_DESCRIPTORS_TABLE_SIZE)
    {
        return BDMF_ERR_PARM;
    }

    table = RDD_RING_DESCRIPTORS_TABLE_PTR();

    descr = &(table->entry[ring_id]);

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    RDD_RING_DESCRIPTOR_DROP_COUNTER_READ(*number_of_packets, descr);
    RDD_RING_DESCRIPTOR_DROP_COUNTER_WRITE(0, descr);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    return 0;
}

int rdd_cpu_tx_send_message(rdd_cpu_tx_message_type_t msg_type, rdd_runner_index_t runner_index,
    uint32_t sram_base, uint32_t parameter_1, uint32_t parameter_2, uint32_t parameter_3, bdmf_boolean is_wait)
{
    RUNNER_REGS_CFG_CPU_WAKEUP  runner_cpu_wakeup_register;
    RDD_CPU_TX_DESCRIPTOR_DTS *cpu_tx_descriptor_ptr;
#if !defined(BDMF_SYSTEM_SIM)
    uint32_t cpu_tx_descriptor_valid;
#endif

    cpu_tx_descriptor_ptr = (RDD_CPU_TX_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(sram_base) + g_cpu_tx_queue_write_ptr[runner_index]);

    if (g_cpu_tx_queue_free_counter[runner_index] == 0)
    {
        rdd_get_tx_descriptor_free_count(runner_index, cpu_tx_descriptor_ptr);

        if (g_cpu_tx_queue_free_counter[runner_index] == 0)
            return BDMF_ERR_TOO_MANY_REQS;
    }

    RDD_CPU_TX_MESSAGE_DESCRIPTOR_COMMAND_WRITE(RDD_CPU_TX_COMMAND_MESSAGE, cpu_tx_descriptor_ptr);
    RDD_CPU_TX_MESSAGE_DESCRIPTOR_MESSAGE_TYPE_WRITE(msg_type, cpu_tx_descriptor_ptr);

    switch (msg_type)
    {
    case RDD_CPU_TX_MESSAGE_FLUSH_WAN_TX_QUEUE:
        RDD_CPU_TX_DESCRIPTOR_TX_QUEUE_PTR_WRITE(parameter_1, cpu_tx_descriptor_ptr);
        break;

    case RDD_CPU_TX_MESSAGE_FLUSH_ETH_QUEUE:
        RDD_CPU_TX_DESCRIPTOR_EMAC_WRITE(parameter_1, cpu_tx_descriptor_ptr);
        RDD_CPU_TX_DESCRIPTOR_QUEUE_WRITE(parameter_2, cpu_tx_descriptor_ptr);
        break;
#ifdef CONFIG_DHD_RUNNER
    case RDD_CPU_TX_MESSAGE_DHD_MESSAGE: /*==RDD_CPU_TX_MESSAGE_MIRRORING_MODE_CONFIG:*/
        if (is_dhd_enabled[parameter_2 >> 14] && runner_index == PICO_RUNNER_A)
        {
            RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DHD_MSG_TYPE_WRITE(parameter_1, cpu_tx_descriptor_ptr);
            RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_FLOW_RING_ID_WRITE((parameter_2 & ~0xc000), cpu_tx_descriptor_ptr);
            RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_RADIO_IDX_WRITE((parameter_2 >> 14) & 0x3, cpu_tx_descriptor_ptr);
            RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_READ_IDX_VALID_WRITE((parameter_2 >> 31) & 0x1, cpu_tx_descriptor_ptr);
            RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_READ_IDX_WRITE((parameter_2 >> 16) & 0x3FF, cpu_tx_descriptor_ptr);
            RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DISABLED_WRITE(parameter_3, cpu_tx_descriptor_ptr);
            break;
        }
        if (runner_index == FAST_RUNNER_B)
#else
    case RDD_CPU_TX_MESSAGE_MIRRORING_MODE_CONFIG:
#endif
        {
            RDD_CPU_TX_DESCRIPTOR_EMAC_WRITE(parameter_1, cpu_tx_descriptor_ptr);
        }
        break;

#if defined(UNDEF) || defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    case RDD_CPU_TX_MESSAGE_RELEASE_SKB_BUFFERS:
        RDD_CPU_TX_DESCRIPTOR_EMAC_WRITE(parameter_1, cpu_tx_descriptor_ptr);
        break;
#endif
    case RDD_CPU_TX_MESSAGE_FLOW_PM_COUNTERS_GET:
    case RDD_CPU_TX_MESSAGE_RX_FLOW_PM_COUNTERS_GET:
    case RDD_CPU_TX_MESSAGE_TX_FLOW_PM_COUNTERS_GET:
        RDD_CPU_TX_DESCRIPTOR_FLOW_WRITE(parameter_1, cpu_tx_descriptor_ptr);
        break;

    case RDD_CPU_TX_MESSAGE_BRIDGE_PORT_PM_COUNTERS_GET:
    case RDD_CPU_TX_MESSAGE_SEND_XON_FRAME:
        RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_WRITE(parameter_1, cpu_tx_descriptor_ptr);
        break;

    case RDD_CPU_TX_MESSAGE_PM_COUNTER_GET:
        RDD_CPU_TX_DESCRIPTOR_GROUP_WRITE(parameter_1, cpu_tx_descriptor_ptr);
        RDD_CPU_TX_DESCRIPTOR_COUNTER_WRITE(parameter_2, cpu_tx_descriptor_ptr);
        RDD_CPU_TX_DESCRIPTOR_COUNTER_4_BYTES_WRITE(parameter_3, cpu_tx_descriptor_ptr);
        break;

    case RDD_CPU_TX_MESSAGE_IPTV_MAC_COUNTER_GET:/*==RDD_CPU_TX_MESSAGE_INVALIDATE_CONTEXT_INDEX_CACHE_ENTRY:*/
        if ((runner_index == PICO_RUNNER_A) || (runner_index == PICO_RUNNER_B))
        {
            RDD_CPU_TX_DESCRIPTOR_IPTV_MAC_IDX_WRITE(parameter_1, cpu_tx_descriptor_ptr);
        }
        else /*Fast->RDD_CPU_TX_MESSAGE_INVALIDATE_CONTEXT_INDEX_CACHE_ENTRY:*/
        {
            RDD_CPU_TX_DESCRIPTOR_CONTEXT_INDEX_WRITE(parameter_1, cpu_tx_descriptor_ptr);
        }
        break;
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    case RDD_CPU_TX_MESSAGE_UPDATE_PD_POOL_QUOTA:
        RDD_CPU_TX_UPDATE_PD_POOL_QUOTA_MESSAGE_DESCRIPTOR_GUARANTEED_FREE_COUNT_INCR_WRITE(parameter_1, cpu_tx_descriptor_ptr);
        RDD_CPU_TX_UPDATE_PD_POOL_QUOTA_MESSAGE_DESCRIPTOR_GUARANTEED_FREE_COUNT_DELTA_WRITE(parameter_2, cpu_tx_descriptor_ptr);
        break;
#else
    case RDD_CPU_TX_MESSAGE_ACTIVATE_TCONT:
        RDD_CPU_TX_DESCRIPTOR_TCONT_INDEX_WRITE(parameter_1, cpu_tx_descriptor_ptr);
        break;
#endif /*DSL*/
    case RDD_CPU_TX_MESSAGE_RING_DESTROY:
        RDD_CPU_TX_DESCRIPTOR_QUEUE_WRITE(parameter_1, cpu_tx_descriptor_ptr);
        break;

    default:
        break;
    }

    RDD_CPU_TX_MESSAGE_DESCRIPTOR_VALID_WRITE(1, cpu_tx_descriptor_ptr);

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[runner_index] += RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[runner_index] &= RDD_CPU_TX_QUEUE_SIZE_MASK;
    g_cpu_tx_queue_free_counter[runner_index]--;

    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    if (runner_index == FAST_RUNNER_A || runner_index == FAST_RUNNER_B)
    {
        runner_cpu_wakeup_register.req_trgt = CPU_TX_FAST_THREAD_NUMBER / 32;
        runner_cpu_wakeup_register.thread_num = CPU_TX_FAST_THREAD_NUMBER % 32;
        runner_cpu_wakeup_register.urgent_req = 0;
    }
    else
    {
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
        runner_cpu_wakeup_register.req_trgt = CPU_TX_PICO_THREAD_NUMBER / 32;
        runner_cpu_wakeup_register.thread_num = CPU_TX_PICO_THREAD_NUMBER % 32;
#endif
        runner_cpu_wakeup_register.urgent_req = 0;
    }

    if (runner_index == FAST_RUNNER_A || runner_index == PICO_RUNNER_A)
    {
        RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);
    }
    else
    {
        RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);
    }

#if !defined(BDMF_SYSTEM_SIM)
    if (is_wait)
    {
        /* wait for the cpu tx thread to finish the current message */
        do
        {
            RDD_CPU_TX_MESSAGE_DESCRIPTOR_VALID_READ(cpu_tx_descriptor_valid, ((volatile RDD_CPU_TX_DESCRIPTOR_DTS *)cpu_tx_descriptor_ptr));
        } while (cpu_tx_descriptor_valid == 1);
    }
#endif
    return 0;
}

bdmf_error_t rdd_flow_pm_counters_get(uint32_t flow_id, rdd_flow_pm_counters_type_t  flow_pm_counters_type, bdmf_boolean counters_clear, rdd_flow_pm_counters_t *pm_counters)
{
    rdd_flow_pm_counters_t *pm_counters_buffer_ptr;
    bdmf_error_t rdd_error;
    unsigned long flags;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    /* read pm counters of a single port and reset its value */
    rdd_error = rdd_cpu_tx_send_message(flow_pm_counters_type, FAST_RUNNER_A, RUNNER_PRIVATE_0_OFFSET, flow_id, 0, 0, 1);

    if (rdd_error)
    {
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        return rdd_error;
    }

    pm_counters_buffer_ptr = (rdd_flow_pm_counters_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + PM_COUNTERS_BUFFER_ADDRESS);

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    pm_counters->good_rx_packet = swap4bytes(pm_counters_buffer_ptr->good_rx_packet);
    pm_counters->good_rx_bytes = swap4bytes(pm_counters_buffer_ptr->good_rx_bytes);
    pm_counters->good_tx_packet = swap4bytes(pm_counters_buffer_ptr->good_tx_packet);
    pm_counters->good_tx_bytes = swap4bytes(pm_counters_buffer_ptr->good_tx_bytes);

    pm_counters->error_rx_packets_discard = swap2bytes(pm_counters_buffer_ptr->error_rx_packets_discard);
    pm_counters->error_tx_packets_discard = swap2bytes(pm_counters_buffer_ptr->error_tx_packets_discard);
#else
    pm_counters->good_rx_packet = pm_counters_buffer_ptr->good_rx_packet;
    pm_counters->good_rx_bytes = pm_counters_buffer_ptr->good_rx_bytes;
    pm_counters->good_tx_packet = pm_counters_buffer_ptr->good_tx_packet;
    pm_counters->good_tx_bytes = pm_counters_buffer_ptr->good_tx_bytes;

    pm_counters->error_rx_packets_discard = pm_counters_buffer_ptr->error_rx_packets_discard;
    pm_counters->error_tx_packets_discard = pm_counters_buffer_ptr->error_tx_packets_discard;
#endif /*DSL*/
    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    return 0;
}

int rdd_cpu_tx_write_eth_packet(uint8_t                    *packet_ptr,
                                  uint32_t                   packet_size,
                                  rdd_emac_id_t              emac_id,
                                  uint8_t                    wifi_ssid,
                                  rdd_tx_queue_id_t          queue_id)
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_CPU_WAKEUP  runner_cpu_wakeup_register;
    uint8_t                    *packet_ddr_ptr;
#endif
    RDD_CPU_TX_DESCRIPTOR_DTS   *cpu_tx_descriptor_ptr;
    uint32_t                    cpu_tx_descriptor;
    uint32_t                    bpm_buffer_number;
    uint8_t                     cpu_tx_descriptor_valid;
    unsigned long               flags;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    cpu_tx_descriptor_ptr = (RDD_CPU_TX_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + g_cpu_tx_queue_write_ptr[2]);

    /* if the descriptor is valid then the CPU-TX queue is full and the packet should be dropped */
    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_READ(cpu_tx_descriptor_valid, cpu_tx_descriptor_ptr);

    if (cpu_tx_descriptor_valid)
    {
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        return BDMF_ERR_TOO_MANY_REQS; /*Was: QUEUE_FULL*/
    }

#if !defined(FIRMWARE_INIT)
    if (fi_bl_drv_bpm_req_buffer(DRV_BPM_SP_SPARE_0, (uint32_t *)&bpm_buffer_number) != DRV_BPM_ERROR_NO_ERROR)
    {
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        return BDMF_ERR_NOMEM; /*Was: BPM_ALLOC_FAIL*/
    }

    packet_ddr_ptr = g_runner_ddr_base_addr + bpm_buffer_number * RDD_RUNNER_PACKET_BUFFER_SIZE + g_ddr_packet_headroom_size + RDD_PACKET_DDR_OFFSET;

    /* copy the packet from the supplied DDR buffer */
    MWRITE_BLK_8(packet_ddr_ptr, packet_ptr, packet_size);

    g_dummy_read = *((packet_ddr_ptr) + packet_size - 1);
#else
    bpm_buffer_number = 0;
#endif

    /* write CPU-TX descriptor and validate it */
    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_L_WRITE((g_ddr_packet_headroom_size + RDD_PACKET_DDR_OFFSET) / 2) |
                        RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_L_WRITE(bpm_buffer_number) |
                        RDD_CPU_TX_DESCRIPTOR_1588_INDICATION_L_WRITE(0);

    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr + 4, cpu_tx_descriptor);

    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_COMMAND_L_WRITE(RDD_CPU_TX_COMMAND_EGRESS_PORT_PACKET) |
                        RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_L_WRITE(packet_size + 4) |
                        RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_L_WRITE(SPARE_0_SRC_PORT) |
                        RDD_CPU_TX_DESCRIPTOR_EMAC_L_WRITE(emac_id) |
                        RDD_CPU_TX_DESCRIPTOR_QUEUE_L_WRITE(queue_id) |
                        RDD_CPU_TX_DESCRIPTOR_VALID_L_WRITE(1);

    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr, cpu_tx_descriptor);

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[2] += RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[2] &= RDD_CPU_TX_QUEUE_SIZE_MASK;

#if !defined(FIRMWARE_INIT)
    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.req_trgt = CPU_TX_PICO_THREAD_NUMBER / 32;
    runner_cpu_wakeup_register.thread_num = CPU_TX_PICO_THREAD_NUMBER % 32;
    runner_cpu_wakeup_register.urgent_req = 0;

    RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);
#endif

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    return BDMF_ERR_OK;
}

int rdd_cpu_tx_write_gpon_packet(uint8_t                              *packet_ptr,
                                   uint32_t                             packet_size,
                                   uint32_t                             upstream_gem_flow,
                                   rdd_wan_channel_id_t                 wan_channel,
                                   rdd_rate_cntrl_id_t                  rate_controller,
                                   rdd_tx_queue_id_t                    queue,
                                   uint8_t                              exclusive_packet)
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_CPU_WAKEUP     runner_cpu_wakeup_register;
    uint8_t                       *packet_ddr_ptr;
#endif
    RDD_CPU_TX_DESCRIPTOR_DTS      *cpu_tx_descriptor_ptr;
    rdd_wan_tx_pointers_entry_t    *wan_tx_pointers_entry_ptr;
    uint32_t                       cpu_tx_descriptor;
    uint32_t                       bpm_buffer_number;
    uint8_t                        cpu_tx_descriptor_valid;
    unsigned long                  flags;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    wan_tx_pointers_entry_ptr = &(wan_tx_pointers_table->entry[wan_channel][rate_controller][queue]);

    cpu_tx_descriptor_ptr = (RDD_CPU_TX_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + g_cpu_tx_queue_write_ptr[1]);

    /* if the descriptor is valid then the CPU-TX queue is full and the packet should be dropped */
    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_READ(cpu_tx_descriptor_valid, cpu_tx_descriptor_ptr);

    if (cpu_tx_descriptor_valid)
    {
        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        return BDMF_ERR_TOO_MANY_REQS;
    }

#if !defined(FIRMWARE_INIT)
    if (fi_bl_drv_bpm_req_buffer(DRV_BPM_SP_EMAC0, (uint32_t *)&bpm_buffer_number) != DRV_BPM_ERROR_NO_ERROR)
    {
        if (exclusive_packet) 
        {
            if (fi_bl_drv_bpm_req_buffer(DRV_BPM_SP_SPARE_0, (uint32_t *)&bpm_buffer_number) != DRV_BPM_ERROR_NO_ERROR)
            {
                bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
                return BDMF_ERR_NOMEM;
            }
        }
        else
        {
            bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
            return BDMF_ERR_NOMEM;
        }
    }

    packet_ddr_ptr = g_runner_ddr_base_addr + bpm_buffer_number * RDD_RUNNER_PACKET_BUFFER_SIZE + g_ddr_packet_headroom_size + RDD_PACKET_DDR_OFFSET;

    /* copy the packet from the supplied DDR buffer */
    MWRITE_BLK_8(packet_ddr_ptr, packet_ptr, packet_size);

    g_dummy_read = *(packet_ddr_ptr + packet_size - 1);
#else
    bpm_buffer_number = 0;
#endif

    /* write CPU-TX descriptor and validate it */
    cpu_tx_descriptor = 
        RDD_CPU_TX_DESCRIPTOR_TX_QUEUE_PTR_L_WRITE(((wan_tx_pointers_entry_ptr->wan_tx_queue_ptr - WAN_TX_QUEUES_TABLE_ADDRESS) / sizeof(RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS))) | 
        RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_L_WRITE(bpm_buffer_number) | 
        RDD_CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_L_WRITE((g_ddr_packet_headroom_size + RDD_PACKET_DDR_OFFSET) / 2);

    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr + 4, cpu_tx_descriptor);

    cpu_tx_descriptor = RDD_CPU_TX_DESCRIPTOR_COMMAND_L_WRITE(RDD_CPU_TX_COMMAND_EGRESS_PORT_PACKET) |
                        RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_L_WRITE(packet_size + 4) |
                        RDD_CPU_TX_DESCRIPTOR_SRC_BRIDGE_PORT_L_WRITE(SPARE_0_SRC_PORT) |
                        RDD_CPU_TX_DESCRIPTOR_US_GEM_FLOW_L_WRITE(upstream_gem_flow) |
                        RDD_CPU_TX_DESCRIPTOR_VALID_L_WRITE(1);

    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr, cpu_tx_descriptor);

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[1] += RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[1] &= RDD_CPU_TX_QUEUE_SIZE_MASK;

#if !defined(FIRMWARE_INIT)
    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.req_trgt = CPU_TX_FAST_THREAD_NUMBER / 32;
    runner_cpu_wakeup_register.thread_num = CPU_TX_FAST_THREAD_NUMBER % 32;
    runner_cpu_wakeup_register.urgent_req = 0;

    RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);
#endif

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    return BDMF_ERR_OK;
}

static inline uint32_t rdd_spdsvc_kbps_to_tokens(uint32_t kbps)
{
    return (uint32_t)(((1000/8) * kbps) / SPDSVC_TIMER_HZ);
}

static inline uint32_t rdd_spdsvc_mbs_to_bucket_size(uint32_t mbs)
{
    uint32_t bucket_size = mbs;

    if (bucket_size < SPDSVC_BUCKET_SIZE_MIN)
    {
        bucket_size = SPDSVC_BUCKET_SIZE_MIN;
    }

    return bucket_size;
}

static inline RDD_SPDSVC_CONTEXT_ENTRY_DTS *rdd_spdsvc_get_context_ptr(rdpa_traffic_dir direction)
{
    if (direction == rdpa_dir_us)
    {
        return (RDD_SPDSVC_CONTEXT_ENTRY_DTS *)
            (DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_SPDSVC_CONTEXT_TABLE_ADDRESS);
    }
    else
    {
        return (RDD_SPDSVC_CONTEXT_ENTRY_DTS *)
            (DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + DS_SPDSVC_CONTEXT_TABLE_ADDRESS);
    }
}

static int f_rdd_spdsvc_config(uint32_t kbps,
                                uint32_t mbs,
                                uint32_t copies,
                                uint32_t total_length,
                                rdpa_traffic_dir direction)
{
    RDD_SPDSVC_CONTEXT_ENTRY_DTS *spdsvc_context_ptr = rdd_spdsvc_get_context_ptr(direction);
    RDD_SPDSVC_CONTEXT_ENTRY_DTS spdsvc_context;

    RDD_SPDSVC_CONTEXT_ENTRY_BBH_DESCRIPTOR_0_WRITE(0, spdsvc_context_ptr);
    RDD_SPDSVC_CONTEXT_ENTRY_BBH_DESCRIPTOR_1_WRITE(0, spdsvc_context_ptr);
    RDD_SPDSVC_CONTEXT_ENTRY_SKB_FREE_INDEX_WRITE(0xFFFF, spdsvc_context_ptr);

    spdsvc_context.tokens = rdd_spdsvc_kbps_to_tokens(kbps);
    spdsvc_context.bucket_size = rdd_spdsvc_mbs_to_bucket_size(spdsvc_context.tokens + mbs);

    RDD_SPDSVC_CONTEXT_ENTRY_COPIES_IN_TRANSIT_WRITE(0, spdsvc_context_ptr);
    RDD_SPDSVC_CONTEXT_ENTRY_TOTAL_COPIES_WRITE(copies, spdsvc_context_ptr);
    RDD_SPDSVC_CONTEXT_ENTRY_TOTAL_LENGTH_WRITE(total_length, spdsvc_context_ptr);
    RDD_SPDSVC_CONTEXT_ENTRY_TOKENS_WRITE(spdsvc_context.tokens, spdsvc_context_ptr);
    RDD_SPDSVC_CONTEXT_ENTRY_BUCKET_SIZE_WRITE(spdsvc_context.bucket_size, spdsvc_context_ptr);

    RDD_SPDSVC_CONTEXT_ENTRY_TX_QUEUE_DISCARDS_WRITE(0, spdsvc_context_ptr);
    RDD_SPDSVC_CONTEXT_ENTRY_TX_QUEUE_WRITES_WRITE(0, spdsvc_context_ptr);
    RDD_SPDSVC_CONTEXT_ENTRY_TX_QUEUE_READS_WRITE(0, spdsvc_context_ptr);

#if defined(CC_RDD_CPU_SPDSVC_DEBUG) && !defined(FIRMWARE_INIT)
    __rdd_cpu_trace("\n%s: kbps %u (tokens %u), mbs %u (bucket_size %u), copies %u, total_length %u, direction %s\n\n",
                    __FUNCTION__, kbps, spdsvc_context.tokens, mbs, spdsvc_context.bucket_size, copies, total_length,
                    (direction == rdpa_dir_us) ? "US" : "DS");
#endif
    return BDMF_ERR_OK;
}

#define RDD_CPU_SPDSVC_IS_RUNNING(__copies_in_transit, __total_copies)  \
    (((__copies_in_transit) > 4) || (__total_copies))

int rdd_spdsvc_config(uint32_t kbps,
                       uint32_t mbs,
                       uint32_t copies,
                       uint32_t total_length)
{
    RDD_SPDSVC_CONTEXT_ENTRY_DTS *spdsvc_context_ptr;
    uint32_t copies_in_transit;
    uint32_t total_copies;
    int ret;

    spdsvc_context_ptr = rdd_spdsvc_get_context_ptr(rdpa_dir_us);
    RDD_SPDSVC_CONTEXT_ENTRY_COPIES_IN_TRANSIT_READ(copies_in_transit, spdsvc_context_ptr);
    RDD_SPDSVC_CONTEXT_ENTRY_TOTAL_COPIES_READ(total_copies, spdsvc_context_ptr);

    if (RDD_CPU_SPDSVC_IS_RUNNING(copies_in_transit, total_copies))
    {
        spdsvc_context_ptr = rdd_spdsvc_get_context_ptr(rdpa_dir_ds);
        RDD_SPDSVC_CONTEXT_ENTRY_COPIES_IN_TRANSIT_READ(copies_in_transit, spdsvc_context_ptr);
        RDD_SPDSVC_CONTEXT_ENTRY_TOTAL_COPIES_READ(total_copies, spdsvc_context_ptr);

        if (RDD_CPU_SPDSVC_IS_RUNNING(copies_in_transit, total_copies))
        {
            return BDMF_ERR_NORES;
        }
    }

    ret = f_rdd_spdsvc_config(kbps, mbs, copies, total_length, rdpa_dir_us);
    if (ret == BDMF_ERR_OK)
    {
        ret = f_rdd_spdsvc_config(kbps, mbs, copies, total_length, rdpa_dir_ds);
    }

    return ret;
}

static int f_rdd_spdsvc_get_tx_result(uint8_t *running_p,
                                       uint32_t *tx_packets_p,
                                       uint32_t *tx_discards_p,
                                       rdpa_traffic_dir direction)
{
    RDD_SPDSVC_CONTEXT_ENTRY_DTS *spdsvc_context_ptr = rdd_spdsvc_get_context_ptr(direction);
    RDD_SPDSVC_CONTEXT_ENTRY_DTS spdsvc_context;

    RDD_SPDSVC_CONTEXT_ENTRY_COPIES_IN_TRANSIT_READ(spdsvc_context.copies_in_transit, spdsvc_context_ptr);
    RDD_SPDSVC_CONTEXT_ENTRY_TOTAL_COPIES_READ(spdsvc_context.total_copies, spdsvc_context_ptr);
    RDD_SPDSVC_CONTEXT_ENTRY_TX_QUEUE_READS_READ(spdsvc_context.tx_queue_reads, spdsvc_context_ptr);
    RDD_SPDSVC_CONTEXT_ENTRY_TX_QUEUE_DISCARDS_READ(spdsvc_context.tx_queue_discards, spdsvc_context_ptr);

    *running_p &= RDD_CPU_SPDSVC_IS_RUNNING(spdsvc_context.copies_in_transit, spdsvc_context.total_copies) ? 1 : 0;
    *tx_packets_p += spdsvc_context.tx_queue_reads;
    *tx_discards_p += spdsvc_context.tx_queue_discards;

    return BDMF_ERR_OK;
}

int rdd_spdsvc_get_tx_result(uint8_t *running_p,
                              uint32_t *tx_packets_p,
                              uint32_t *tx_discards_p)
{
    int ret;

    *running_p = 1;
    *tx_packets_p = 0;
    *tx_discards_p = 0;

    ret = f_rdd_spdsvc_get_tx_result(running_p, tx_packets_p, tx_discards_p, rdpa_dir_us);
    if (ret == BDMF_ERR_OK)
    {
        ret = f_rdd_spdsvc_get_tx_result(running_p, tx_packets_p, tx_discards_p, rdpa_dir_ds);
    }

    return ret;
}

#if defined(CONFIG_BCM_SPDSVC_SUPPORT) && !defined(RDD_BASIC)
static void rdd_spdsvc_initialize_structs(rdpa_traffic_dir direction)
{
    RDD_SPDSVC_CONTEXT_ENTRY_DTS *spdsvc_context_ptr = rdd_spdsvc_get_context_ptr(direction);

    memset(spdsvc_context_ptr, 0, sizeof(RDD_SPDSVC_CONTEXT_ENTRY_DTS));

#if defined(FIRMWARE_INIT)
    RDD_SPDSVC_CONTEXT_ENTRY_TIMER_PERIOD_WRITE(100, spdsvc_context_ptr);

    rdd_spdsvc_config(100000, 2000, 10, 1514);
#else /*FIRMWARE_INIT*/
#if defined(CC_RDD_CPU_SPDSVC_DEBUG)
    __rdd_cpu_trace("\n\tspdsvc_context_ptr %p\n\n", spdsvc_context_ptr);
#endif
#ifdef RUNNER_FWTRACE
    RDD_SPDSVC_CONTEXT_ENTRY_TIMER_PERIOD_WRITE((SPDSVC_TIMER_PERIOD*(1000/TIMER_PERIOD_NS)), spdsvc_context_ptr);
#else
    RDD_SPDSVC_CONTEXT_ENTRY_TIMER_PERIOD_WRITE(SPDSVC_TIMER_PERIOD, spdsvc_context_ptr);

    RDD_SPDSVC_CONTEXT_ENTRY_SKB_FREE_INDEX_WRITE(0xFFFF, spdsvc_context_ptr);
#endif /*FWTRACE*/
#endif /*FIRMWARE_INIT*/
}

int rdd_spdsvc_initialize(void)
{
#if !defined(FIRMWARE_INIT)
    RUNNER_REGS_CFG_TIMER_TARGET           runner_timer_target_register;
    RUNNER_REGS_CFG_CPU_WAKEUP             runner_cpu_wakeup_register;
#endif
    static uint32_t                        api_first_time_call = 1;

    if (api_first_time_call)
    {
        rdd_spdsvc_initialize_structs(rdpa_dir_us);
        rdd_spdsvc_initialize_structs(rdpa_dir_ds);

#if !defined(FIRMWARE_INIT)
        RUNNER_REGS_1_CFG_TIMER_TARGET_READ(runner_timer_target_register);
        runner_timer_target_register.timer_5_7 = RUNNER_REGS_CFG_TIMER_TARGET_TIMER_5_7_MAIN_CORE_VALUE;
        RUNNER_REGS_1_CFG_TIMER_TARGET_WRITE(runner_timer_target_register);

        /* activate the speed service tasks */
        runner_cpu_wakeup_register.req_trgt = US_SPDSVC_THREAD_NUMBER / 32;
        runner_cpu_wakeup_register.thread_num = US_SPDSVC_THREAD_NUMBER % 32;
        RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);

        runner_cpu_wakeup_register.req_trgt = DS_SPDSVC_THREAD_NUMBER / 32;
        runner_cpu_wakeup_register.thread_num = DS_SPDSVC_THREAD_NUMBER % 32;
        RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);
#endif /*FIRMWARE_INIT*/
        api_first_time_call = 0;
    }

    return BDMF_ERR_OK;
}
#endif /*defined(CONFIG_BCM_SPDSVC_SUPPORT) && !defined(RDD_BASIC)*/

int rdd_cso_counters_get(rdd_cso_counters_entry_t *cso_counters_ptr)
{
    rdd_cso_counters_entry_t   *cso_context_ptr_tmp;
    unsigned long               flags;

    cso_context_ptr_tmp = (rdd_cso_counters_entry_t *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_CSO_CONTEXT_TABLE_ADDRESS);

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    RDD_CSO_CONTEXT_ENTRY_GOOD_CSUM_PACKETS_READ(cso_counters_ptr->good_csum_packets, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_NO_CSUM_PACKETS_READ(cso_counters_ptr->no_csum_packets, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_BAD_IPV4_HDR_CSUM_PACKETS_READ(cso_counters_ptr->bad_ipv4_hdr_csum_packets, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_BAD_TCP_UDP_CSUM_PACKETS_READ(cso_counters_ptr->bad_tcp_udp_csum_packets, cso_context_ptr_tmp);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
    return BDMF_ERR_OK;
}

int rdd_cso_context_get(RDD_CSO_CONTEXT_ENTRY_DTS *cso_context_ptr)
{
    RDD_CSO_CONTEXT_ENTRY_DTS   *cso_context_ptr_tmp;
    unsigned long               flags;

    cso_context_ptr_tmp = (RDD_CSO_CONTEXT_ENTRY_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET) + US_CSO_CONTEXT_TABLE_ADDRESS);

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    RDD_CSO_CONTEXT_ENTRY_SUMMARY_READ(cso_context_ptr->summary, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_PACKET_LENGTH_READ(cso_context_ptr->packet_length, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_LINEAR_LENGTH_READ(cso_context_ptr->linear_length, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_PACKET_HEADER_LENGTH_READ(cso_context_ptr->packet_header_length, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_IP_PROTOCOL_READ(cso_context_ptr->ip_protocol, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_IP_HEADER_OFFSET_READ(cso_context_ptr->ip_header_offset, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_IP_HEADER_LENGTH_READ(cso_context_ptr->ip_header_length, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_IP_TOTAL_LENGTH_READ(cso_context_ptr->ip_total_length, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_IPV4_CSUM_READ(cso_context_ptr->ipv4_csum, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_TCP_UDP_HEADER_OFFSET_READ(cso_context_ptr->tcp_udp_header_offset, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_TCP_UDP_HEADER_LENGTH_READ(cso_context_ptr->tcp_udp_header_length, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_TCP_UDP_TOTAL_LENGTH_READ(cso_context_ptr->tcp_udp_total_length, cso_context_ptr);
    RDD_CSO_CONTEXT_ENTRY_TCP_UDP_CSUM_READ(cso_context_ptr->tcp_udp_csum, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_MAX_CHUNK_LENGTH_READ(cso_context_ptr->max_chunk_length, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_CHUNK_BYTES_LEFT_READ(cso_context_ptr->chunk_bytes_left, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_NR_FRAGS_READ(cso_context_ptr->nr_frags, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_FRAG_INDEX_READ(cso_context_ptr->frag_index, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_FRAG_LEN_READ(cso_context_ptr->frag_len, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_FRAG_DATA_READ(cso_context_ptr->frag_data, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_GOOD_CSUM_PACKETS_READ(cso_context_ptr->good_csum_packets, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_NO_CSUM_PACKETS_READ(cso_context_ptr->no_csum_packets, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_BAD_IPV4_HDR_CSUM_PACKETS_READ(cso_context_ptr->bad_ipv4_hdr_csum_packets, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_BAD_TCP_UDP_CSUM_PACKETS_READ(cso_context_ptr->bad_tcp_udp_csum_packets, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_FAIL_CODE_READ(cso_context_ptr->fail_code, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_DMA_SYNC_READ(cso_context_ptr->dma_sync, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_SEG_LENGTH_READ(cso_context_ptr->seg_length, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_SEG_BYTES_LEFT_READ(cso_context_ptr->seg_bytes_left, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_PAYLOAD_LENGTH_READ(cso_context_ptr->payload_length, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_PAYLOAD_BYTES_LEFT_READ(cso_context_ptr->payload_bytes_left, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_PAYLOAD_PTR_READ(cso_context_ptr->payload_ptr, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_DDR_PAYLOAD_OFFSET_READ(cso_context_ptr->ddr_payload_offset, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_DDR_SRC_ADDRESS_READ(cso_context_ptr->ddr_src_address, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_SAVED_IH_BUFFER_NUMBER_READ(cso_context_ptr->saved_ih_buffer_number, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_SAVED_CSUM32_RET_ADDR_READ(cso_context_ptr->saved_csum32_ret_addr, cso_context_ptr_tmp);
    RDD_CSO_CONTEXT_ENTRY_SAVED_R16_READ(cso_context_ptr->saved_r16, cso_context_ptr_tmp);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);

    return BDMF_ERR_OK;
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
