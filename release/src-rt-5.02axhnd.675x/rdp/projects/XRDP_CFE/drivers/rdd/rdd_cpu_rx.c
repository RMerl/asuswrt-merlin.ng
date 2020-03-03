/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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

#include "rdd_cpu_rx.h"

#ifndef RDP_SIM
#define FEED_RING_INTERRUPT_THRESHOLD    0x2000
#else
#define FEED_RING_INTERRUPT_THRESHOLD    0x200
#endif
#define RECYCLE_RING_INTERRUPT_THRESHOLD 256
#define RECYCLE_RING_INTERRUPT_COALESCING_DEFAULT_VALUE  500


#ifdef RDP_SIM
extern uint8_t *cpu_rx_ring_base_addr_ptr;
#endif

void rdd_cpu_if_init(void)
{
}

int rdd_ring_init(uint32_t ring_id, uint8_t type, bdmf_phys_addr_t phys_ring_address, uint32_t number_of_entries,
    uint32_t size_of_entry, uint32_t irq, uint32_t write_idx_init_val, bdmf_phys_addr_t phys_ring_index_addr, uint8_t high_prio_tc_thr)
{
    uint32_t addr_lo, addr_hi;
    int rc = BDMF_ERR_OK;

    if ((type > CPU_IF_RDD_LAST) || (ring_id > RING_ID_LAST))
        return BDMF_ERR_PARM;

    RDD_BTRACE("ring_id = %d, phys_ring_address = 0x%lx, number_of_entries = %d, size_of_entry = %d, irq = %d, write_idx_init_val = %d (not in use, "
        "ring_id used instead)\n",
        ring_id, (uintptr_t)phys_ring_address, number_of_entries, size_of_entry, irq, write_idx_init_val);

    if (type == CPU_IF_RDD_DATA)
    {
        RDD_CPU_RING_DESCRIPTOR_READ_IDX_WRITE_G(0, RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR, ring_id);
        RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_WRITE_G(0, RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR, ring_id);
        RDD_CPU_RING_DESCRIPTOR_SIZE_OF_ENTRY_WRITE_G(size_of_entry, RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR, ring_id);
        RDD_CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_WRITE_G((number_of_entries + 31) >> CPU_RING_SIZE_32_RESOLUTION, RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR,
            ring_id);
        RDD_CPU_RING_DESCRIPTOR_INTERRUPT_ID_WRITE_G(irq, RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR, ring_id);

        GET_ADDR_HIGH_LOW(addr_hi, addr_lo, phys_ring_address);
        RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_LOW_WRITE_G(addr_lo, RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR, ring_id);
        RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_HIGH_WRITE_G(addr_hi, RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR, ring_id);
    }
    else if (type == CPU_IF_RDD_RECYCLE)
    {
        RDD_CPU_RING_DESCRIPTOR_DTS *entry = get_recycle_ring_entry(ring_id);
        RDD_CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_DTS *index_addr;

        RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_WRITE(0, entry);
        RDD_CPU_RING_DESCRIPTOR_READ_IDX_WRITE(0, entry);

        /* XXX: change to size_of_entry when sw is fixed */
        RDD_CPU_RING_DESCRIPTOR_SIZE_OF_ENTRY_WRITE(8, entry);
        RDD_CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_WRITE((number_of_entries + 31) >> CPU_RING_SIZE_32_RESOLUTION, entry);
        RDD_CPU_RING_DESCRIPTOR_INTERRUPT_ID_WRITE(irq, entry);

        GET_ADDR_HIGH_LOW(addr_hi, addr_lo, phys_ring_address);
        RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_LOW_WRITE(addr_lo, entry);
        RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_HIGH_WRITE(addr_hi, entry);

        /* Update write index address */
        GET_ADDR_HIGH_LOW(addr_hi, addr_lo, phys_ring_index_addr);
        if (ring_id == FEED_RCYCLE_RING_ID)
            index_addr = RDD_CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_PTR(get_runner_idx(cpu_rx_runner_image));
        else
            index_addr = RDD_CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_PTR(get_runner_idx(cpu_tx_runner_image));
        RDD_DDR_ADDRESS_HIGH_WRITE(addr_hi, index_addr);
        RDD_DDR_ADDRESS_LOW_WRITE(addr_lo, index_addr);

        /* Update recycle coalescing thresholds. Same to all cores, can afford WRITE_G */
        RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_MAX_SIZE_WRITE_G(RECYCLE_RING_INTERRUPT_THRESHOLD,
            RDD_CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_ADDRESS_ARR, 0);
        RDD_CPU_INTERRUPT_COALESCING_ENTRY_TIMER_PERIOD_WRITE_G(RECYCLE_RING_INTERRUPT_COALESCING_DEFAULT_VALUE,
            RDD_CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_ADDRESS_ARR, 0);

        WMB();
#if !defined(G9991) && !defined(_CFE_REDUCED_XRDP_)
        /* After running slow-path performance tests decided not to do coalescing when recycling buffers from feed. Even
         * if will open it in future, in current implementation same coalescing task serves both data and recycle
         * queues, and for RX it is opened as part of data queues configuration. Should reconsider to use different
         * coalescing tasks data and recycle queues */
        if (ring_id == TX_RCYCLE_RING_ID)
        {
            rc = ag_drv_rnr_regs_cfg_cpu_wakeup_set(get_runner_idx(cpu_tx_runner_image),
                RECYCLE_INTERRUPT_COALESCING_THREAD_NUMBER);
        }
#endif
    }
    else if (type == CPU_IF_RDD_FEED)
    {
        RDD_CPU_RING_DESCRIPTOR_READ_IDX_WRITE_G(0, RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
        RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_WRITE_G(write_idx_init_val, RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
        RDD_CPU_RING_DESCRIPTOR_SIZE_OF_ENTRY_WRITE_G(size_of_entry, RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
        RDD_CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_WRITE_G((number_of_entries + 31) >> CPU_RING_SIZE_64_RESOLUTION,
            RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
        RDD_CPU_RING_DESCRIPTOR_INTERRUPT_ID_WRITE_G(irq, RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);

        GET_ADDR_HIGH_LOW(addr_hi, addr_lo, phys_ring_address);
        RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_LOW_WRITE_G(addr_lo, RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
        RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_HIGH_WRITE_G(addr_hi, RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
        RDD_BYTES_2_BITS_WRITE_G(FEED_RING_INTERRUPT_THRESHOLD, RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_ADDRESS_ARR, 0);
    }

    return rc;
}

int rdd_cpu_rx_ring_low_prio_set(uint32_t ring_id, uint8_t type, uint16_t threshold)
{
    if ((type > CPU_IF_RDD_LAST) || (ring_id > RING_ID_LAST))
        return BDMF_ERR_PARM;

    if (type == CPU_IF_RDD_DATA)
        RDD_CPU_RING_DESCRIPTOR_LOW_PRIORITY_THRESHOLD_WRITE_G(threshold, RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR, ring_id);
    else if (type == CPU_IF_RDD_FEED)
        RDD_CPU_RING_DESCRIPTOR_LOW_PRIORITY_THRESHOLD_WRITE_G(threshold, RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
    else if (type == CPU_IF_RDD_RECYCLE)
        RDD_CPU_RING_DESCRIPTOR_LOW_PRIORITY_THRESHOLD_WRITE_G(threshold, RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);

    return BDMF_ERR_OK;
}

int rdd_ring_destroy(uint32_t ring_id)
{
    RDD_BTRACE("ring_id = %d\n", ring_id);
    RDD_TRACE("This is stub, consider to implement\n");

    /* TODO */
    return 0;
}

void rdd_cpu_tc_to_rqx_init(uint8_t def_rxq_idx)
{
    int i;

    RDD_BTRACE("def_rxq_idx = %d\n", def_rxq_idx);

    for (i = 0; i < RDD_CPU_REASON_TO_TC_SIZE; i++)
    {
        GROUP_MWRITE_8(RDD_TC_TO_CPU_RXQ_ADDRESS_ARR, i, def_rxq_idx);
        GROUP_MWRITE_8(RDD_EXC_TC_TO_CPU_RXQ_ADDRESS_ARR, i, (uint8_t)BDMF_INDEX_UNASSIGNED);
    }
}

void rdd_cpu_vport_cpu_obj_init(uint8_t def_cpu_obj_idx)
{
    int i;

    RDD_BTRACE("def_cpu_obj_idx = %d\n", def_cpu_obj_idx);

    for (i = 0; i < RDD_VPORT_TO_CPU_OBJ_SIZE; i++)
        GROUP_MWRITE_8(RDD_VPORT_TO_CPU_OBJ_ADDRESS_ARR, i, def_cpu_obj_idx);
}

void rdd_cpu_rx_meters_init(void)
{
    int i;

    RDD_BTRACE("\n");

    /* Reason to meters configuration */
    for (i = 0; i < RDD_US_CPU_REASON_TO_METER_TABLE_SIZE; i++)
    {
        GROUP_MWRITE_8(RDD_US_CPU_REASON_TO_METER_TABLE_ADDRESS_ARR, i, CPU_RX_METER_DISABLE);
        GROUP_MWRITE_8(RDD_DS_CPU_REASON_TO_METER_TABLE_ADDRESS_ARR, i, CPU_RX_METER_DISABLE);
    }

    for (i = 0; i < RDD_CPU_REASON_AND_VPORT_TO_METER_TABLE_SIZE; i++)
        GROUP_MWRITE_8(RDD_CPU_REASON_AND_VPORT_TO_METER_TABLE_ADDRESS_ARR, i, CPU_RX_METER_DISABLE);

    for (i = 0; i < RDD_CPU_VPORT_TO_METER_TABLE_SIZE; i++)
        GROUP_MWRITE_8(RDD_CPU_VPORT_TO_METER_TABLE_ADDRESS_ARR, i, CPU_RX_METER_DISABLE);
}

