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

#ifndef _RDD_CPU_H
#define _RDD_CPU_H

#include "rdd.h"
#include "rdd_data_structures.h"
#include "rdd_runner_proj_defs.h"
#include "rdpa_cpu.h"

extern bdmf_fastlock int_lock_irq;
extern rdd_wan_tx_pointers_table_t *wan_tx_pointers_table;

extern uint32_t g_ddr_packet_headroom_size;
extern uint32_t g_cpu_tx_queue_write_ptr[4];
extern uint32_t g_cpu_tx_queue_free_counter[4];
extern uint8_t *g_runner_ddr_base_addr;
extern uint32_t g_cpu_tx_queue_abs_data_ptr_write_ptr[4];
extern uint32_t g_cpu_tx_released_skb_counter;
extern uint32_t g_cpu_tx_skb_free_indexes_head_ptr;
extern uint32_t g_cpu_tx_abs_packet_limit;
extern uint32_t g_cpu_tx_sent_abs_packets_counter;
extern uint32_t g_cpu_tx_skb_free_indexes_release_ptr;
extern uint32_t g_cpu_tx_skb_free_indexes_counter;
extern uint32_t g_cpu_tx_no_free_skb_counter;
extern uint8_t **g_cpu_tx_skb_pointers_reference_array;
extern uint16_t *g_free_skb_indexes_fifo_table;
extern rdd_phys_addr_t *g_cpu_tx_data_pointers_reference_array;

#if defined(CONFIG_DHD_RUNNER)
extern bdmf_boolean is_dhd_enabled[];
#endif

#if !defined(FIRMWARE_INIT)
#if defined(CONFIG_BCM_PKTRUNNER_GSO)
extern uint32_t  g_cpu_tx_no_free_gso_desc_counter;
extern uint32_t  g_cpu_tx_sent_abs_gso_packets_counter;
extern uint32_t  g_cpu_tx_sent_abs_gso_bytes_counter;
#endif /*CONFIG_BCM_PKTRUNNER_GSO*/

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
extern rdd_wan_tx_pointers_table_t *wan_tx_pointers_table;

/*#define CC_RDD_CPU_SPDSVC_DEBUG*/
typedef struct {
    uint32_t write;
    uint32_t read;
    uint32_t count;
    uint16_t *data;
} cpu_tx_skb_free_indexes_cache_t;

extern cpu_tx_skb_free_indexes_cache_t g_cpu_tx_skb_free_indexes_cache;
extern rdd_phys_addr_t g_free_skb_indexes_fifo_table_physical_address;
extern uint32_t  g_cpu_tx_pending_free_indexes_counter;
#if defined(CC_RDD_CPU_SPDSVC_DEBUG)
extern bdmf_sysb  g_spdsvc_setup_sysb_ptr;
#endif /*CC_RDD_CPU_SPDSVC_DEBUG*/
#endif /*DSL*/
#endif /*!FIRMWARE_INIT*/

void rdd_cpu_rx_init(void);
void rdd_cpu_tx_init(void);
int rdd_cpu_reason_to_cpu_rx_queue(rdpa_cpu_reason cpu_reason, rdd_cpu_rx_queue queue_id, rdpa_traffic_dir direction);
void rdd_cpu_rx_meter_config(rdd_cpu_rx_meter cpu_meter, uint16_t average_rate, uint16_t burst_size, rdpa_traffic_dir dir);
int rdd_cpu_reason_to_cpu_rx_meter(rdpa_cpu_reason reason, rdd_cpu_rx_meter meter, rdpa_traffic_dir dir, uint32_t src_port_mask);
void rdd_ring_init(uint32_t ring_id, uint8_t unused0, rdd_phys_addr_t ring_address,
    uint32_t number_of_entries, uint32_t size_of_entry, uint32_t interrupt_id, uint32_t unused1);
bdmf_error_t rdd_ring_destroy(uint32_t  ring_id);
int rdd_cpu_rx_interrupt_coalescing_config(uint32_t ring_id, uint32_t timeout_us, uint32_t max_packet_count);
int rdd_cpu_rx_queue_discard_get(rdd_cpu_rx_queue ring_id, uint16_t *number_of_packets);
int rdd_cpu_tx_send_message(rdd_cpu_tx_message_type_t msg_type, rdd_runner_index_t runner_index,
    uint32_t sram_base, uint32_t parameter_1, uint32_t parameter_2, uint32_t parameter_3, bdmf_boolean is_wait);
int rdd_flow_pm_counters_get(uint32_t flow_id, rdd_flow_pm_counters_type_t  flow_pm_counters_type, bdmf_boolean counters_clear, rdd_flow_pm_counters_t *pm_counters);
int rdd_vport_pm_counters_get(rdd_vport_id_t vport, bdmf_boolean counters_clear, rdd_vport_pm_counters_t *pm_counters);
int rdd_various_counters_get(rdpa_traffic_dir dir, uint32_t mask, bdmf_boolean clear_counters, rdd_various_counters_t *various_counters);
int rdd_cpu_rx_meter_drop_counter_get(rdd_cpu_rx_meter cpu_meter, rdpa_traffic_dir direction, uint32_t *drop_counter);
int rdd_iptv_rx_pm_counters_get(rdd_vport_pm_counters_t *pm_counters);

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
int rdd_cpu_tx_write_eth_packet(uint8_t                    *packet_ptr,
                                  uint32_t                   packet_size,
                                  rdd_emac_id_t              emac_id,
                                  uint8_t                    wifi_ssid,
                                  rdd_tx_queue_id_t          queue_id);
int rdd_cpu_tx_write_gpon_packet(uint8_t                              *packet_ptr,
                                   uint32_t                             packet_size,
                                   uint32_t                             upstream_gem_flow,
                                   rdd_wan_channel_id_t                 wan_channel,
                                   rdd_rate_cntrl_id_t                  rate_controller,
                                   rdd_tx_queue_id_t                    queue,
                                   uint8_t                              exclusive_packet);
int rdd_cpu_tx_send_packet_to_lan_bridge(uint8_t   *packet_ptr,
                                           uint32_t  packet_size,
                                           uint32_t  downstream_gem_flow);
int rdd_cpu_tx_send_packet_to_lan_interworking(uint8_t                   *packet_ptr,
                                                 uint32_t                  packet_size,
                                                 uint32_t                  downstream_gem_flow,
                                                 rdd_emac_id_t             emac_id,
                                                 uint8_t                   wifi_ssid);
int rdd_spdsvc_config(uint32_t kbps,
                       uint32_t mbs,
                       uint32_t copies,
                       uint32_t total_length);
int rdd_spdsvc_get_tx_result(uint8_t *running_p,
                              uint32_t *tx_packets_p,
                              uint32_t *tx_discards_p);
int rdd_spdsvc_initialize(void);

int rdd_cso_counters_get(rdd_cso_counters_entry_t *cso_counters_ptr);
int rdd_cso_context_get(RDD_CSO_CONTEXT_ENTRY_DTS *cso_context_ptr);
#endif /*DSL*/

#if defined(CONFIG_BCM_PKTRUNNER_GSO)
int rdd_gso_counters_get(RDD_GSO_COUNTERS_ENTRY_DTS *gso_counters_ptr);
int rdd_gso_context_get(RDD_GSO_CONTEXT_ENTRY_DTS *gso_context_ptr);
int rdd_gso_desc_get(RDD_GSO_DESC_ENTRY_DTS *gso_desc_ptr);
#endif

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
static inline void __skb_free_indexes_cache_write(cpu_tx_skb_free_indexes_cache_t *cache_p, uint32_t data)
{
    cache_p->data[cache_p->write] = data;

    if (cache_p->write == (g_cpu_tx_abs_packet_limit-1))
    {
        cache_p->write = 0;
    }
    else
    {
        cache_p->write++;
    }
    
    cache_p->count++;
}

static inline void rdd_initialize_skb_free_indexes_cache(void)
{
    uint32_t free_index = 0;

    for (free_index = 0; free_index < g_cpu_tx_abs_packet_limit; ++free_index)
    {
        __skb_free_indexes_cache_write(&g_cpu_tx_skb_free_indexes_cache, free_index);
    }
}
#endif /*!defined(FIRMWARE_INIT) && !defined(RDD_BASIC)*/
#endif /*DSL*/

static inline void rdd_get_tx_descriptor_free_count(rdd_runner_index_t runner_index, RDD_CPU_TX_DESCRIPTOR_DTS *cpu_tx_descriptor_ptr)
{
    RDD_CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_DTS *cpu_tx_queue_read_table_ptr;
    uint8_t *cpu_tx_queue_read_entry_ptr;
    uint8_t cpu_tx_queue_read_idx;

    cpu_tx_queue_read_table_ptr = RDD_CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_PTR();

    cpu_tx_queue_read_entry_ptr = (uint8_t *)&cpu_tx_queue_read_table_ptr->entry[runner_index];

    MREAD_8(cpu_tx_queue_read_entry_ptr, cpu_tx_queue_read_idx);

    {
        uint8_t read = cpu_tx_queue_read_idx;
        uint8_t write = (g_cpu_tx_queue_write_ptr[runner_index] & ~CPU_TX_DESCRIPTOR_ADDRESS_MASK) / RDD_CPU_TX_DESCRIPTOR_SIZE;
        uint32_t free;

        if (write >= read)
        {
            free = RDD_CPU_TX_QUEUE_SIZE - write + read - 1;
        }
        else
        {
            /* write wraparound */
            free = read - write - 1;
        }

        g_cpu_tx_queue_free_counter[runner_index] = free;
    }
}

static inline int32_t rdd_vport_to_class_id(rdd_vport_id_t vport)
{
    switch (vport)
    {
    case RDD_WAN0_VPORT:
        return RDD_IH_WAN_BRIDGE_LOW_CLASS;

    case RDD_LAN0_VPORT:
        return RDD_IH_LAN_EMAC0_CLASS;

    case RDD_LAN1_VPORT:
        return RDD_IH_LAN_EMAC1_CLASS;

    case RDD_LAN2_VPORT:
        return RDD_IH_LAN_EMAC2_CLASS;

    case RDD_LAN3_VPORT:
        return RDD_IH_LAN_EMAC3_CLASS;

    case RDD_LAN4_VPORT:
        return RDD_IH_LAN_EMAC4_CLASS;

    case RDD_WIFI_VPORT:
        return RDD_IH_PCI_CLASS;

    default:
        return 0;
    }

    return 0;
}

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
#define RDD_CPU_TX_MAX_PENDING_FREE_INDEXES  64

static inline uint32_t __skb_free_indexes_cache_nbr_of_entries(cpu_tx_skb_free_indexes_cache_t *cache_p)
{
    return cache_p->count;
}

#define __is_skb_free_indexes_cache_not_empty(_cache_p)       \
    __skb_free_indexes_cache_nbr_of_entries((_cache_p))

static inline uint32_t __skb_free_indexes_cache_read(cpu_tx_skb_free_indexes_cache_t *cache_p)
{
    uint32_t data;

    data = cache_p->data[cache_p->read];

    if (cache_p->read == (g_cpu_tx_abs_packet_limit-1))
    {
        cache_p->read = 0;
    }
    else
    {
        cache_p->read++;
    }
    
    cache_p->count--;

    return data;
}

static inline int rdd_release_free_skb(uint32_t max_pending_free_indexes)
{
#if !(defined(FIRMWARE_INIT) || defined(BDMF_SYSTEM_SIM)) /* DSL changed expression */

    /* BULK FREE: In order to reduce the number of Runner SRAM reads and increase CPU
       performance, SKB recycling is deferred until the number of buffers pending
       transmission is higher than the provided max_pending_free_indexes */

    if (unlikely(g_cpu_tx_pending_free_indexes_counter > max_pending_free_indexes))
    {
        uint32_t prev_cpu_tx_pending_free_indexes_counter = g_cpu_tx_pending_free_indexes_counter;
        uint32_t *free_indexes_fifo_tail_ptr;
        uint32_t loaded = 0;
        uint32_t tx_skb_free_indexes_tail_address;
        uint64_t free_indexes_64 = 0;
        uint16_t *free_indexes_array = (uint16_t *)&free_indexes_64;

        /* read free indexes fifo tail ptr */
        free_indexes_fifo_tail_ptr = (uint32_t *)(DEVICE_ADDRESS(RUNNER_COMMON_0_OFFSET) + FREE_SKB_INDEXES_DDR_FIFO_TAIL_ADDRESS);

        MREAD_32(free_indexes_fifo_tail_ptr, tx_skb_free_indexes_tail_address);
        /*The free_indexes_64_addr read below is an uncached normal memory read, which on ARM can be 
         *speculatively issued out of order before the above fifo tail pointer read.
         *This creates a race condition where ARM can read from the tail of the FIFO before the Runner has
         *completed writing to it.
         *To guarantee correct ordering, we place a read barrier after the tail pointer read.
         */
        rmb();

        tx_skb_free_indexes_tail_address = (tx_skb_free_indexes_tail_address -
                g_free_skb_indexes_fifo_table_physical_address) >> 1;

        while (g_cpu_tx_skb_free_indexes_release_ptr != tx_skb_free_indexes_tail_address)
        {
            uint32_t release_ptr_index;
            uint16_t free_index;

            release_ptr_index = g_cpu_tx_skb_free_indexes_release_ptr & (uint32_t)(0x3);

            if (!release_ptr_index || !loaded)
            {
                volatile uint64_t *free_indexes_64_addr = (volatile uint64_t *)(&g_free_skb_indexes_fifo_table[
                        (g_cpu_tx_skb_free_indexes_release_ptr & (uint32_t)(~0x3))]);
                free_indexes_64 = (uint64_t)*free_indexes_64_addr;
                loaded = 1;
            }

            free_index = (uint16_t)swap2bytes(free_indexes_array[release_ptr_index]) & ~RDD_CPU_TX_SKB_INDEX_OWNERSHIP_BIT_MASK;

#if defined(CC_RDD_CPU_SPDSVC_DEBUG)
            if ((bdmf_sysb)g_cpu_tx_skb_pointers_reference_array[free_index] == g_spdsvc_setup_sysb_ptr)
            {
                __rdd_cpu_trace("Speed Service Generator: OFF (%p, 0x%x)", g_spdsvc_setup_sysb_ptr, free_index);
                g_spdsvc_setup_sysb_ptr = NULL;
            }
#endif /*SPDSVC_DEBUG*/
            if (likely(g_cpu_tx_skb_pointers_reference_array[free_index]))
            {
                bdmf_sysb_free((bdmf_sysb)g_cpu_tx_skb_pointers_reference_array[free_index]);
                __skb_free_indexes_cache_write(&g_cpu_tx_skb_free_indexes_cache, free_index);
                g_cpu_tx_data_pointers_reference_array[free_index] = 0;
                g_cpu_tx_skb_pointers_reference_array[free_index] = 0;
                g_cpu_tx_released_skb_counter++;
                g_cpu_tx_pending_free_indexes_counter--;
            }
#if !(defined(FIRMWARE_INIT) || defined(BDMF_SYSTEM_SIM)) /* DSL changed expression */
            else
            {
                printk(KERN_ERR "ERR !! RDD : rdd_release_free_skb() : Duplicate Buffer free requet : index = %d\n", free_index);
            }
#endif

            g_cpu_tx_skb_free_indexes_release_ptr = (g_cpu_tx_skb_free_indexes_release_ptr + 1) % g_cpu_tx_abs_packet_limit;
        }

        if (prev_cpu_tx_pending_free_indexes_counter != g_cpu_tx_pending_free_indexes_counter)
        {
            /* We have freed one or more SKBs: make sure the updates of the data pointers table reached the DDR */
            WMB();
        }
    }
#endif /*!(defined(FIRMWARE_INIT) || defined(BDMF_SYSTEM_SIM))*/
    return BDMF_ERR_OK;
}

static inline bdmf_sysb rdd_cpu_return_free_index(uint16_t free_index)
{
    unsigned long flags;
    bdmf_sysb sysb;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    sysb = (bdmf_sysb)(g_cpu_tx_skb_pointers_reference_array[free_index]);

    if (likely(g_cpu_tx_skb_pointers_reference_array[free_index]))
    {
        __skb_free_indexes_cache_write(&g_cpu_tx_skb_free_indexes_cache, free_index);

        g_cpu_tx_data_pointers_reference_array[free_index] = 0;

        WMB();
        g_cpu_tx_skb_pointers_reference_array[free_index] = 0;
        g_cpu_tx_released_skb_counter++;
        g_cpu_tx_pending_free_indexes_counter--;
    }
#if !(defined(FIRMWARE_INIT) || defined(BDMF_SYSTEM_SIM)) /* DSL changed expression */
    else
    {
        printk(KERN_ERR "ERR !! RDD : rdd_cpu_return_free_index() : Duplicate free requet : index = %d\n", free_index);
    }
#endif


    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);

    return sysb;
}
#endif /*!defined(FIRMWARE_INIT) && !defined(RDD_BASIC)*/
/*
 * NOTE: This function must be called with int_lock_irq locked.
 */
static inline int alloc_cpu_tx_descriptor(rdd_runner_index_t descriptor_queue_table_idx,
                                          RDD_CPU_TX_DESCRIPTOR_DTS  **cpu_tx_descriptor_ptr,
                                          uint32_t                   *descriptor_number,
                                          uint16_t                   *skb_free_index)
{
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
    if (descriptor_queue_table_idx == FAST_RUNNER_A || descriptor_queue_table_idx == PICO_RUNNER_A)
        *cpu_tx_descriptor_ptr = (RDD_CPU_TX_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET +
                                                                              g_cpu_tx_queue_write_ptr[descriptor_queue_table_idx]));
    else
        *cpu_tx_descriptor_ptr = (RDD_CPU_TX_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET +
                                                                              g_cpu_tx_queue_write_ptr[descriptor_queue_table_idx]));

    if (g_cpu_tx_queue_free_counter[descriptor_queue_table_idx] == 0)
    {
        rdd_get_tx_descriptor_free_count(descriptor_queue_table_idx, *cpu_tx_descriptor_ptr);

        if (g_cpu_tx_queue_free_counter[descriptor_queue_table_idx] == 0)
        {
            g_cpu_tx_released_skb_counter++;
            /* skb_free() will be called by caller upon error */
            return BDMF_ERR_TOO_MANY_REQS;
        }
    }

    /* get skb pointer list free index */
    if (__is_skb_free_indexes_cache_not_empty(&g_cpu_tx_skb_free_indexes_cache))
    {
        *skb_free_index = __skb_free_indexes_cache_read(&g_cpu_tx_skb_free_indexes_cache);
        g_cpu_tx_pending_free_indexes_counter++;
    }
    else
    {
        g_cpu_tx_no_free_skb_counter++;
        /* skb_free() will be called by caller upon error */
        return BDMF_ERR_NORES;
    }

    *descriptor_number = (g_cpu_tx_queue_write_ptr[descriptor_queue_table_idx] &
                          RDD_CPU_TX_DESCRIPTOR_NUMBER_MASK) / RDD_CPU_TX_DESCRIPTOR_SIZE;
    
    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[descriptor_queue_table_idx] += RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[descriptor_queue_table_idx] &= RDD_CPU_TX_QUEUE_SIZE_MASK;
    g_cpu_tx_queue_free_counter[descriptor_queue_table_idx]--;
#endif /*!defined(FIRMWARE_INIT) && !defined(RDD_BASIC)*/
    return BDMF_ERR_OK;
}

static inline int rdd_cpu_tx_write_gpon_packet_from_abs_address(bdmf_sysb                            packet_ptr,
                                                                  uint32_t                           packet_size,
                                                                  uint32_t                           upstream_gem_flow,
                                                                  rdd_wan_channel_id_t               wan_channel,
                                                                  rdd_rate_cntrl_id_t                rate_controller,
                                                                  rdd_tx_queue_id_t                  queue,
                                                                  rdpa_cpu_tx_extra_info_t           extra_info)
{
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
    RUNNER_REGS_CFG_CPU_WAKEUP     runner_cpu_wakeup_register;
    RDD_CPU_TX_DESCRIPTOR_DTS      *cpu_tx_descriptor_ptr;
    uint16_t                       free_index = 0xFFFF;
    unsigned long                  flags;
    rdd_wan_tx_pointers_entry_t   *wan_tx_pointers_entry_ptr;
    rdd_runner_index_t             cpu_tx_descriptor_queue_table_idx;
    uint32_t                       cpu_tx_descriptor = 0;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    cpu_tx_descriptor_queue_table_idx = FAST_RUNNER_B;
    cpu_tx_descriptor_ptr = (RDD_CPU_TX_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_1_OFFSET + g_cpu_tx_queue_write_ptr[cpu_tx_descriptor_queue_table_idx]));

    if (g_cpu_tx_queue_free_counter[cpu_tx_descriptor_queue_table_idx] == 0)
    {
        rdd_get_tx_descriptor_free_count(cpu_tx_descriptor_queue_table_idx, cpu_tx_descriptor_ptr);

        if (g_cpu_tx_queue_free_counter[cpu_tx_descriptor_queue_table_idx] == 0)
        {
            g_cpu_tx_released_skb_counter++;

            bdmf_fastlock_unlock_irq(&int_lock_irq, flags);

            /* skb_free() will be called by caller upon error */
            return BDMF_ERR_TOO_MANY_REQS;
        }
    }

    /* get skb pointer list free index */
    if (__is_skb_free_indexes_cache_not_empty(&g_cpu_tx_skb_free_indexes_cache))
    {
        free_index = __skb_free_indexes_cache_read(&g_cpu_tx_skb_free_indexes_cache);

        g_cpu_tx_pending_free_indexes_counter++;
    }
    else
    {
        g_cpu_tx_no_free_skb_counter++;

        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);

        /* skb_free() will be called by caller upon error */
        return BDMF_ERR_NORES;
    }

    /* save packet_ptr ptr and data ptr */
    g_cpu_tx_skb_pointers_reference_array[free_index] = (uint8_t *)packet_ptr;

    g_cpu_tx_data_pointers_reference_array[free_index] = swap4bytes(RDD_VIRT_TO_PHYS(bdmf_sysb_data(packet_ptr)));
    g_cpu_tx_sent_abs_packets_counter++;

    /* write CPU-TX descriptor Word 1: Egress enqueue */
    RDD_CPU_TX_DESCRIPTOR_ABS_SKB_INDEX_L_WRITE(cpu_tx_descriptor, free_index & 0x3FFF);

    wan_tx_pointers_entry_ptr = &(wan_tx_pointers_table->entry[wan_channel][rate_controller][queue]);
    RDD_CPU_TX_DESCRIPTOR_US_FAST_TX_QUEUE_L_WRITE(cpu_tx_descriptor, 
                                                   ((wan_tx_pointers_entry_ptr->wan_tx_queue_ptr - WAN_TX_QUEUES_TABLE_ADDRESS) / 
                                                   sizeof(RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS)));

    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr + 4, cpu_tx_descriptor);

    cpu_tx_descriptor = 0;

    RDD_CPU_TX_DESCRIPTOR_CORE_PACKET_LENGTH_L_WRITE(cpu_tx_descriptor, packet_size + 4);
    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_L_WRITE(cpu_tx_descriptor, 1);

    if (extra_info.is_spdsvc_setup_packet)
    {
#if defined(CC_RDD_CPU_SPDSVC_DEBUG)
        g_spdsvc_setup_sysb_ptr = packet_ptr;
        __rdd_cpu_trace("Speed Service Generator ON (%p, 0x%x)", g_spdsvc_setup_sysb_ptr, free_index);
#endif
        RDD_CPU_TX_DESCRIPTOR_CORE_COMMAND_L_WRITE(cpu_tx_descriptor, RDD_CPU_TX_COMMAND_SPDSVC_PACKET);
    }
    else
    {
        RDD_CPU_TX_DESCRIPTOR_CORE_COMMAND_L_WRITE(cpu_tx_descriptor, RDD_CPU_TX_COMMAND_ABSOLUTE_ADDRESS_PACKET);
    }
    RDD_CPU_TX_DESCRIPTOR_CORE_TX_QUEUE_L_WRITE(cpu_tx_descriptor, 0);
    RDD_CPU_TX_DESCRIPTOR_US_FAST_UPSTREAM_GEM_FLOW_L_WRITE(cpu_tx_descriptor, upstream_gem_flow);

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[cpu_tx_descriptor_queue_table_idx] += RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[cpu_tx_descriptor_queue_table_idx] &= RDD_CPU_TX_QUEUE_SIZE_MASK;
    g_cpu_tx_queue_free_counter[cpu_tx_descriptor_queue_table_idx]--;

    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.req_trgt =  CPU_TX_FAST_THREAD_NUMBER >> 5;
    runner_cpu_wakeup_register.thread_num =  CPU_TX_FAST_THREAD_NUMBER & 0x1f;
    runner_cpu_wakeup_register.urgent_req = 0;

    rdd_release_free_skb(RDD_CPU_TX_MAX_PENDING_FREE_INDEXES);

    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr, cpu_tx_descriptor);

    RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
#endif
    return BDMF_ERR_OK;
}


static inline int rdd_cpu_tx_write_eth_packet_from_abs_address(bdmf_sysb                  packet_ptr,
                                                               uint32_t                   packet_size,
                                                               rdd_emac_id_t              emac_id,
                                                               rdd_tx_queue_id_t          queue_id,
                                                               rdpa_cpu_tx_extra_info_t   extra_info)
{
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
    RUNNER_REGS_CFG_CPU_WAKEUP     runner_cpu_wakeup_register;
    RDD_CPU_TX_DESCRIPTOR_DTS      *cpu_tx_descriptor_ptr;
    uint16_t                       free_index = 0xFFFF;
    unsigned long                  flags;
    rdd_runner_index_t             cpu_tx_descriptor_queue_table_idx;
    uint32_t                       cpu_tx_descriptor = 0;
    void                           *packet_data_ptr;
#if defined(CONFIG_BCM_PKTRUNNER_GSO)
    uint32_t                       is_gso_pkt = 0;
#endif

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    cpu_tx_descriptor_queue_table_idx = PICO_RUNNER_A;
    cpu_tx_descriptor_ptr = (RDD_CPU_TX_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET + g_cpu_tx_queue_write_ptr[cpu_tx_descriptor_queue_table_idx]));

    if (g_cpu_tx_queue_free_counter[cpu_tx_descriptor_queue_table_idx] == 0)
    {
        rdd_get_tx_descriptor_free_count(cpu_tx_descriptor_queue_table_idx, cpu_tx_descriptor_ptr);

        if (g_cpu_tx_queue_free_counter[cpu_tx_descriptor_queue_table_idx] == 0)
        {
            g_cpu_tx_released_skb_counter++;

            bdmf_fastlock_unlock_irq(&int_lock_irq, flags);

            /* skb_free() will be called by caller upon error */
            return BDMF_ERR_TOO_MANY_REQS;
        }
    }

    /* get skb pointer list free index */
    if (__is_skb_free_indexes_cache_not_empty(&g_cpu_tx_skb_free_indexes_cache))
    {
        free_index = __skb_free_indexes_cache_read(&g_cpu_tx_skb_free_indexes_cache);

        g_cpu_tx_pending_free_indexes_counter++;
    }
    else
    {
        g_cpu_tx_no_free_skb_counter++;

        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        /* skb_free() will be called by caller upon error */
        return BDMF_ERR_NORES;
    }

#if defined(CONFIG_BCM_PKTRUNNER_GSO)
    /* if needed create a GSO Descriptor */
    packet_data_ptr = bdmf_sysb_gso_data(packet_ptr, &is_gso_pkt); 

    if (unlikely(!packet_data_ptr))
    {
        g_cpu_tx_no_free_gso_desc_counter++;

        /*release the skbfree index */
        __skb_free_indexes_cache_write(&g_cpu_tx_skb_free_indexes_cache, free_index);
        g_cpu_tx_pending_free_indexes_counter--;

        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
        /* skb_free() will be called by caller upon error */
        return BDMF_ERR_NORES;
    }
#else
    packet_data_ptr = bdmf_sysb_data(packet_ptr);
#endif

    /* save packet_ptr ptr and data ptr */
    g_cpu_tx_skb_pointers_reference_array[free_index] = (uint8_t *)packet_ptr;

    g_cpu_tx_data_pointers_reference_array[free_index] = swap4bytes(RDD_VIRT_TO_PHYS(packet_data_ptr));

    g_cpu_tx_sent_abs_packets_counter++;

    /* write CPU-TX descriptor Word 1: Egress enqueue */
    RDD_CPU_TX_DESCRIPTOR_ABS_SKB_INDEX_L_WRITE(cpu_tx_descriptor, free_index & 0x3FFF);
    RDD_CPU_TX_DESCRIPTOR_CORE_LAG_PORT_PTI_L_WRITE(cpu_tx_descriptor, extra_info.lag_port);
    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr + 4, cpu_tx_descriptor);

    cpu_tx_descriptor = 0;

    RDD_CPU_TX_DESCRIPTOR_CORE_PACKET_LENGTH_L_WRITE(cpu_tx_descriptor, packet_size + 4);
    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_L_WRITE(cpu_tx_descriptor, 1);
    RDD_CPU_TX_DESCRIPTOR_CORE_EMAC_L_WRITE(cpu_tx_descriptor, emac_id);

    if (extra_info.is_spdsvc_setup_packet)
    {
#if defined(CC_RDD_CPU_SPDSVC_DEBUG)
        g_spdsvc_setup_sysb_ptr = packet_ptr;
        __rdd_cpu_trace("Speed Service Generator ON (%p, 0x%x)", g_spdsvc_setup_sysb_ptr, free_index);
#endif
        RDD_CPU_TX_DESCRIPTOR_CORE_COMMAND_L_WRITE(cpu_tx_descriptor, RDD_CPU_TX_COMMAND_SPDSVC_PACKET);
    }
    else
    {
        RDD_CPU_TX_DESCRIPTOR_CORE_COMMAND_L_WRITE(cpu_tx_descriptor, RDD_CPU_TX_COMMAND_ABSOLUTE_ADDRESS_PACKET);
    }

    RDD_CPU_TX_DESCRIPTOR_CORE_TX_QUEUE_L_WRITE(cpu_tx_descriptor,  queue_id);

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[cpu_tx_descriptor_queue_table_idx] += RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[cpu_tx_descriptor_queue_table_idx] &= RDD_CPU_TX_QUEUE_SIZE_MASK;
    g_cpu_tx_queue_free_counter[cpu_tx_descriptor_queue_table_idx]--;

    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.req_trgt =  CPU_TX_PICO_THREAD_NUMBER >> 5;
    runner_cpu_wakeup_register.thread_num =  CPU_TX_PICO_THREAD_NUMBER & 0x1f;
    runner_cpu_wakeup_register.urgent_req = 0;

    rdd_release_free_skb(RDD_CPU_TX_MAX_PENDING_FREE_INDEXES);

#if defined(CONFIG_BCM_PKTRUNNER_GSO)
    if (is_gso_pkt)
    {
        /* update is_gso_pkt information in TX Descriptor */
        cpu_tx_descriptor = cpu_tx_descriptor | RDD_CPU_TX_DESCRIPTOR_GSO_PKT_L_WRITE(is_gso_pkt);

        /* update gso packet count & byte count */
        g_cpu_tx_sent_abs_gso_packets_counter++;
        g_cpu_tx_sent_abs_gso_bytes_counter += packet_size;

        /*update packet_size*/
        packet_size = sizeof(runner_gso_desc_t);
    }
#endif
    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr, cpu_tx_descriptor);

    RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
#endif
    return BDMF_ERR_OK;
}

static inline int rdd_cpu_tx_write_offload_packet_from_abs_address(bdmf_sysb                  packet_ptr,
                                                                     uint32_t                   packet_size,
                                                                     rdd_tx_queue_id_t          queue_id)
{
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC)
    RUNNER_REGS_CFG_CPU_WAKEUP     runner_cpu_wakeup_register;
    RDD_CPU_TX_DESCRIPTOR_DTS      *cpu_tx_descriptor_ptr;
    uint16_t                       free_index = 0xFFFF;
    unsigned long                  flags;
    rdd_runner_index_t             cpu_tx_descriptor_queue_table_idx;
    uint32_t                       descriptor_number;
    uint32_t                       cpu_tx_descriptor;
    void                           *packet_data_ptr;
    RDD_FOUR_BYTES_DTS             *cpu_tx_abs_descriptor_ptr;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

    cpu_tx_descriptor_queue_table_idx = FAST_RUNNER_A;
    cpu_tx_descriptor_ptr = (RDD_CPU_TX_DESCRIPTOR_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET + g_cpu_tx_queue_write_ptr[cpu_tx_descriptor_queue_table_idx]));

    if (g_cpu_tx_queue_free_counter[cpu_tx_descriptor_queue_table_idx] == 0)
    {
        rdd_get_tx_descriptor_free_count(cpu_tx_descriptor_queue_table_idx, cpu_tx_descriptor_ptr);

        if (g_cpu_tx_queue_free_counter[cpu_tx_descriptor_queue_table_idx] == 0)
        {
            g_cpu_tx_released_skb_counter++;

            bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
            /* skb_free() will be called by caller upon error */
            return BDMF_ERR_TOO_MANY_REQS;
        }
    }

    /* get skb pointer list free index */
    if (__is_skb_free_indexes_cache_not_empty(&g_cpu_tx_skb_free_indexes_cache))
    {
        free_index = __skb_free_indexes_cache_read(&g_cpu_tx_skb_free_indexes_cache);

        g_cpu_tx_pending_free_indexes_counter++;
    }
    else
    {
        g_cpu_tx_no_free_skb_counter++;

        bdmf_fastlock_unlock_irq(&int_lock_irq, flags);

        /* skb_free() will be called by caller upon error */
        return BDMF_ERR_NORES;
    }

    packet_data_ptr = bdmf_sysb_data(packet_ptr);

    /* save packet_ptr ptr and data ptr */
    g_cpu_tx_skb_pointers_reference_array[free_index] = (uint8_t *)packet_ptr;

    g_cpu_tx_data_pointers_reference_array[free_index] = swap4bytes(RDD_VIRT_TO_PHYS(packet_data_ptr));

    /* save data pointer also in CPU Tx queue */
    descriptor_number = (g_cpu_tx_queue_write_ptr[cpu_tx_descriptor_queue_table_idx] & RDD_CPU_TX_DESCRIPTOR_NUMBER_MASK) / RDD_CPU_TX_DESCRIPTOR_SIZE;

    cpu_tx_abs_descriptor_ptr = (RDD_FOUR_BYTES_DTS *)(DEVICE_ADDRESS(RUNNER_PRIVATE_0_OFFSET) + 
                                g_cpu_tx_queue_abs_data_ptr_write_ptr[cpu_tx_descriptor_queue_table_idx] + 
                                (descriptor_number * RDD_CPU_TX_ABS_DATA_PTR_DESCRIPTOR_SIZE));

    MWRITE_32((uint8_t *)cpu_tx_abs_descriptor_ptr,  (uint32_t)(RDD_VIRT_TO_PHYS(packet_data_ptr)));

    g_cpu_tx_sent_abs_packets_counter++;

    /* write CPU-TX descriptor Word 1: Egress enqueue */
    cpu_tx_descriptor = 0;

    RDD_CPU_TX_DESCRIPTOR_ABS_SKB_INDEX_L_WRITE(cpu_tx_descriptor, free_index & 0x3FFF);

    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr + 4, cpu_tx_descriptor);

    /* write CPU-TX descriptor Word 0: Egress enqueue */
    cpu_tx_descriptor = 0;

    RDD_CPU_TX_DESCRIPTOR_CORE_PACKET_LENGTH_L_WRITE(cpu_tx_descriptor, packet_size + 4);
    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_L_WRITE(cpu_tx_descriptor, 1);
    RDD_CPU_TX_DESCRIPTOR_CORE_COMMAND_L_WRITE(cpu_tx_descriptor, RDD_CPU_TX_COMMAND_ABSOLUTE_ADDRESS_PACKET);
    RDD_CPU_TX_DESCRIPTOR_CORE_TX_QUEUE_L_WRITE(cpu_tx_descriptor,  queue_id);

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[cpu_tx_descriptor_queue_table_idx] += RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[cpu_tx_descriptor_queue_table_idx] &= RDD_CPU_TX_QUEUE_SIZE_MASK;
    g_cpu_tx_queue_free_counter[cpu_tx_descriptor_queue_table_idx]--;

    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.req_trgt =  CPU_TX_FAST_THREAD_NUMBER >> 5;
    runner_cpu_wakeup_register.thread_num =  CPU_TX_FAST_THREAD_NUMBER & 0x1f;
    runner_cpu_wakeup_register.urgent_req = 0;

    rdd_release_free_skb(RDD_CPU_TX_MAX_PENDING_FREE_INDEXES);

    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr, cpu_tx_descriptor);

    RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
#endif
    return BDMF_ERR_OK;
}
#endif /*DSL*/

static inline int rdd_cpu_tx(rdd_cpu_tx_args_t *args, void *buffer, uint32_t skb_pkt_length)
{
#if !defined(FIRMWARE_INIT) && !defined(RDD_BASIC) && !defined(RUNNER_CPU_DQM_TX)
    RUNNER_REGS_CFG_CPU_WAKEUP runner_cpu_wakeup_register;
    RDD_CPU_TX_DESCRIPTOR_DTS *cpu_tx_descriptor_ptr;
    rdd_wan_tx_pointers_entry_t *wan_tx_pointers_entry_ptr;
    rdd_runner_index_t cpu_tx_descriptor_queue_table_idx;
    unsigned long flags;
    uint16_t free_index = 0;
    uint32_t cpu_tx_descriptor = 0;
    uint32_t src_port = args->direction.ds.emac_id;

    bdmf_fastlock_lock_irq(&int_lock_irq, flags);

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    rdd_release_free_skb(RDD_CPU_TX_MAX_PENDING_FREE_INDEXES);
#endif

    if (rdpa_dir_ds == args->traffic_dir)
        cpu_tx_descriptor_queue_table_idx = (rdd_cpu_tx_mode_egress_enq == args->mode) ? PICO_RUNNER_A : FAST_RUNNER_A;
    else /* if (rdpa_dir_us == args->traffic_dir) */
        cpu_tx_descriptor_queue_table_idx = (rdd_cpu_tx_mode_egress_enq == args->mode) ? FAST_RUNNER_B : PICO_RUNNER_B;

    cpu_tx_descriptor_ptr = (RDD_CPU_TX_DESCRIPTOR_DTS *)(DEVICE_ADDRESS((rdpa_dir_ds == args->traffic_dir ?
        RUNNER_PRIVATE_0_OFFSET : RUNNER_PRIVATE_1_OFFSET)) + (g_cpu_tx_queue_write_ptr[cpu_tx_descriptor_queue_table_idx]));

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
    if (unlikely(g_cpu_tx_queue_free_counter[cpu_tx_descriptor_queue_table_idx] == 0))
    {
        rdd_get_tx_descriptor_free_count(cpu_tx_descriptor_queue_table_idx, cpu_tx_descriptor_ptr);

        if (g_cpu_tx_queue_free_counter[cpu_tx_descriptor_queue_table_idx] == 0)
        {
            g_cpu_tx_released_skb_counter++;

            bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
            return BDMF_ERR_TOO_MANY_REQS;
        }
    }

    /* get skb pointer list free index (ABS only) */
    if (rdd_host_buffer == args->buffer_type)
    {
        /* get skb pointer list free index */
        if (__is_skb_free_indexes_cache_not_empty(&g_cpu_tx_skb_free_indexes_cache))
        {
            free_index = __skb_free_indexes_cache_read(&g_cpu_tx_skb_free_indexes_cache);

            g_cpu_tx_pending_free_indexes_counter++;
        }
        else
        {
            g_cpu_tx_no_free_skb_counter++;
            bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
            return BDMF_ERR_NORES;
        }

        /* save buffer ptr and data ptr */
        g_cpu_tx_skb_pointers_reference_array[free_index] = (uint8_t *)buffer;
        g_cpu_tx_data_pointers_reference_array[free_index] = swap4bytes(RDD_VIRT_TO_PHYS(bdmf_sysb_data(buffer)));
        g_cpu_tx_sent_abs_packets_counter++;
    }
#endif
    /* write CPU-TX descriptor Word 1: bridge/interwork/Egress enqueue common */
    RDD_CPU_TX_DESCRIPTOR_ABS_SKB_INDEX_L_WRITE(cpu_tx_descriptor, rdd_runner_buffer == args->buffer_type ?
        ((bdmf_pbuf_t *)buffer)->bpm_bn : (free_index & 0x3FFF));

    /* Egress enqueue case */
    if (rdd_cpu_tx_mode_egress_enq == args->mode)
    {
        if (rdd_runner_buffer == args->buffer_type)
            RDD_CPU_TX_DESCRIPTOR_CORE_PAYLOAD_OFFSET_L_WRITE(cpu_tx_descriptor, ((bdmf_pbuf_t *)buffer)->offset);

        if (rdpa_dir_ds == args->traffic_dir)
        {
            if (RDD_EMAC_ID_WIFI == args->direction.ds.emac_id)
            {
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#if defined(CONFIG_DHD_RUNNER)
                src_port = PCI_0_SRC_PORT;
                if (is_dhd_enabled[args->wan_flow >> 14])
                {
                    RDD_CPU_TX_DHD_DESCRIPTOR_FLOW_RING_ID_L_WRITE(cpu_tx_descriptor, args->wan_flow & ~0xc000);
                    RDD_CPU_TX_DHD_DESCRIPTOR_RADIO_IDX_L_WRITE(cpu_tx_descriptor, args->wan_flow >> 14);
                    RDD_CPU_TX_DHD_DESCRIPTOR_SSID_L_WRITE(cpu_tx_descriptor, args->wifi_ssid);
                    RDD_CPU_TX_DHD_DESCRIPTOR_SSID_MULTICAST_L_WRITE(cpu_tx_descriptor, 0); /* expandable, it is 0 anyway */
                }
                else
#endif /*CONFIG_DHD_RUNNER*/
                {
                    RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI_SSID_L_WRITE(cpu_tx_descriptor, args->wifi_ssid);
                    RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI_SSID_MULTICAST_L_WRITE(cpu_tx_descriptor, 0); /* expandable, it is 0 anyway */
                }

#else /*!DSL:*/
                RDD_CPU_TX_DHD_DESCRIPTOR_FLOW_RING_ID_L_WRITE(cpu_tx_descriptor, args->wan_flow);
                RDD_CPU_TX_DHD_DESCRIPTOR_SSID_L_WRITE(cpu_tx_descriptor, args->wifi_ssid);
#endif /*DSL*/
            }
        }
        else /* if (rdpa_dir_us == args->traffic_dir) */
        {
            wan_tx_pointers_entry_ptr = &(wan_tx_pointers_table->entry
                [args->direction.us.wan_channel][args->direction.us.rate_controller][args->direction.us.queue]);
            RDD_CPU_TX_DESCRIPTOR_US_FAST_TX_QUEUE_L_WRITE(cpu_tx_descriptor,
                ((wan_tx_pointers_entry_ptr->wan_tx_queue_ptr - WAN_TX_QUEUES_TABLE_ADDRESS) / sizeof(RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS)));
        }
    }
#if !defined(WL4908)
    /* bridge/interwork case */
    else
    {
        RDD_CPU_TX_DESCRIPTOR_CORE_IH_CLASS_L_WRITE(cpu_tx_descriptor, rdd_vport_to_class_id(rdpa_dir_ds == args->traffic_dir ?
            RDD_WAN0_VPORT : args->direction.us.src_bridge_port));
        RDD_CPU_TX_DESCRIPTOR_CORE_SSID_L_WRITE(cpu_tx_descriptor, (rdd_cpu_tx_mode_interworking == args->mode) || (rdpa_dir_us == args->traffic_dir) ?
            args->wifi_ssid : 0);
        RDD_CPU_TX_DESCRIPTOR_CORE_ABS_FLAG_L_WRITE(cpu_tx_descriptor, (rdd_host_buffer == args->buffer_type) ? 1 : 0);
    }
#endif

    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr + 4, cpu_tx_descriptor);
    cpu_tx_descriptor = 0;

    /* bridge/interwork/Egress enqueue common part */
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#if defined(CONFIG_DHD_RUNNER)
    if ((is_dhd_enabled[args->wan_flow >> 14]) && (rdd_cpu_tx_mode_egress_enq == args->mode) && (RDD_EMAC_ID_WIFI == args->direction.ds.emac_id) &&
        (rdpa_dir_ds == args->traffic_dir))
        RDD_CPU_TX_DESCRIPTOR_CORE_PACKET_LENGTH_L_WRITE(cpu_tx_descriptor, (rdd_host_buffer == args->buffer_type) ? skb_pkt_length  : ((bdmf_pbuf_t *)buffer)->length);
    else
#endif /*CONFIG_DHD_RUNNER*/
#endif /*DSL*/
        RDD_CPU_TX_DESCRIPTOR_CORE_PACKET_LENGTH_L_WRITE(cpu_tx_descriptor, (rdd_host_buffer == args->buffer_type) ?
            skb_pkt_length + 4 : ((bdmf_pbuf_t *)buffer)->length + 4);

    RDD_CPU_TX_DESCRIPTOR_CORE_VALID_L_WRITE(cpu_tx_descriptor, 1);

    if ((rdpa_dir_ds == args->traffic_dir) && (rdd_cpu_tx_mode_full != args->mode))
        RDD_CPU_TX_DESCRIPTOR_CORE_EMAC_L_WRITE(cpu_tx_descriptor, args->direction.ds.emac_id);

    /* bridge/interwork case */
    if (rdd_cpu_tx_mode_egress_enq != args->mode)
    {
        RDD_CPU_TX_DESCRIPTOR_CORE_COMMAND_L_WRITE(cpu_tx_descriptor, (rdd_cpu_tx_mode_full == args->mode) ?
            RDD_CPU_TX_COMMAND_BRIDGE_PACKET : RDD_CPU_TX_COMMAND_INTERWORKING_PACKET);

        if (args->traffic_dir == rdpa_dir_us)
            RDD_CPU_TX_DESCRIPTOR_CORE_SRC_BRIDGE_PORT_L_WRITE(cpu_tx_descriptor, args->direction.us.src_bridge_port);
    }

    /* Egress enqueue case */
    else
    {
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
        if (args->is_spdsvc_setup_packet)
        {
#if defined(CC_RDD_CPU_SPDSVC_DEBUG)
            g_spdsvc_setup_sysb_ptr = buffer;
            __rdd_cpu_trace("Speed Service Generator ON (%p, 0x%x)", g_spdsvc_setup_sysb_ptr, free_index);
#endif
            RDD_CPU_TX_DESCRIPTOR_CORE_COMMAND_L_WRITE(cpu_tx_descriptor, RDD_CPU_TX_COMMAND_SPDSVC_PACKET);
        }
        else
#endif /*DSL*/
        {
            RDD_CPU_TX_DESCRIPTOR_CORE_COMMAND_L_WRITE(cpu_tx_descriptor, rdd_host_buffer == args->buffer_type ?
                RDD_CPU_TX_COMMAND_ABSOLUTE_ADDRESS_PACKET : RDD_CPU_TX_COMMAND_EGRESS_PORT_PACKET);
        }

        RDD_CPU_TX_DESCRIPTOR_CORE_TX_QUEUE_L_WRITE(cpu_tx_descriptor, rdpa_dir_ds == args->traffic_dir ? args->direction.ds.queue_id : 0);

        if (rdd_runner_buffer == args->buffer_type)
            RDD_CPU_TX_DESCRIPTOR_CORE_SRC_BRIDGE_PORT_L_WRITE(cpu_tx_descriptor, (args->buffer_type == rdd_runner_buffer) ? RDD_CPU_VPORT : 0);

        if (rdpa_dir_us == args->traffic_dir)
            RDD_CPU_TX_DESCRIPTOR_US_FAST_UPSTREAM_GEM_FLOW_L_WRITE(cpu_tx_descriptor, args->wan_flow);
    }
    
    if ((rdpa_dir_ds == args->traffic_dir) && (rdd_cpu_tx_mode_full != args->mode))
    {
        RDD_CPU_TX_DESCRIPTOR_CORE_EMAC_L_WRITE(cpu_tx_descriptor, src_port);
    }

    MWRITE_32((uint8_t *)cpu_tx_descriptor_ptr, cpu_tx_descriptor);

    /* increment and wrap around if needed the write pointer of the CPU-TX queue */
    g_cpu_tx_queue_write_ptr[cpu_tx_descriptor_queue_table_idx] += RDD_CPU_TX_DESCRIPTOR_SIZE;
    g_cpu_tx_queue_write_ptr[cpu_tx_descriptor_queue_table_idx] &= RDD_CPU_TX_QUEUE_SIZE_MASK;
    g_cpu_tx_queue_free_counter[cpu_tx_descriptor_queue_table_idx]--;

    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    runner_cpu_wakeup_register.req_trgt = (cpu_tx_descriptor_queue_table_idx <= FAST_RUNNER_B ? CPU_TX_FAST_THREAD_NUMBER : CPU_TX_PICO_THREAD_NUMBER) >> 5;
    runner_cpu_wakeup_register.thread_num = (cpu_tx_descriptor_queue_table_idx <= FAST_RUNNER_B ? CPU_TX_FAST_THREAD_NUMBER : CPU_TX_PICO_THREAD_NUMBER) & 0x1f;
    runner_cpu_wakeup_register.urgent_req = 0;

    if (rdpa_dir_ds == args->traffic_dir)
        RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);
    else /* if (rdpa_dir_us == args->traffic_dir) */
        RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE(runner_cpu_wakeup_register);

    bdmf_fastlock_unlock_irq(&int_lock_irq, flags);
#endif
    return 0;
}

/*rdd_cpu internal functions*/
rdpa_cpu_reason cpu_reason_to_cpu_per_port_reason_index(rdpa_cpu_reason reason);
void _rdd_runner_flow_header_descriptor_init(uint32_t private_memory_offset, uint32_t descriptor_addr,
                                             uint32_t thread_number, uint32_t packet_headroom_size, uint32_t ih_buffer_id);

#endif /* _RDD_CPU_H */
