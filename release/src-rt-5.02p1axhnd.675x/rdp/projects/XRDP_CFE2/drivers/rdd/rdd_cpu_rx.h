/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
:>
*/

#ifndef _RDD_CPU_RX_H_
#define _RDD_CPU_RX_H_

#include "bdmf_errno.h"
#include "rdd.h"

#define HOST_BUFFER_SIZE                    2048
#define RING_INTERRUPT_THRESHOLD_MAX   ((1 << 16) - 1)
#define CPU_RING_SIZE_32_RESOLUTION 5
#define CPU_RING_SIZE_64_RESOLUTION 6

typedef enum
{
    DATA_RING_ID_FIRST = 0,
    DATA_RING_ID_LAST = ( RDD_CPU_RING_DESCRIPTORS_TABLE_SIZE - 1 ),
    FEED_RING_ID,
    FEED_RCYCLE_RING_ID,
    TX_RCYCLE_RING_ID,
    RING_ID_LAST = TX_RCYCLE_RING_ID,
    RING_ID_NUM_OF
} ring_id_t;



static inline void rdd_cpu_inc_feed_ring_write_idx(uint32_t delta)
{
#ifndef RDP_SIM
    RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_DTS *entry;
#endif
    uint32_t write_idx, size_of_ring;

#ifndef RDP_SIM
    entry = RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_PTR(get_runner_idx(cfe_core_runner_image));
    RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_READ(write_idx, entry);
    RDD_CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_READ(size_of_ring, entry);
#else
    RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_READ_G(write_idx, RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
    RDD_CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_READ_G(size_of_ring, RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
#endif

    RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_WRITE_G(((write_idx + delta) % (size_of_ring << CPU_RING_SIZE_64_RESOLUTION)), RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
}

static inline RDD_CPU_RING_DESCRIPTOR_DTS *get_recycle_ring_entry(uint32_t ring_id)
{
    if (ring_id == FEED_RCYCLE_RING_ID)
        return (RDD_CPU_RING_DESCRIPTOR_DTS *)RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_PTR(get_runner_idx(cfe_core_runner_image));
    return (RDD_CPU_RING_DESCRIPTOR_DTS *)RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_PTR(get_runner_idx(cfe_core_runner_image));
}

static inline int rdd_cpu_inc_read_idx(uint32_t ring_id, uint8_t type, uint32_t delta)
{
#ifndef RDP_SIM
    RDD_CPU_RING_DESCRIPTOR_DTS *entry;
#endif
    uint32_t read_idx, size_of_ring;

    if (type == CPU_IF_RDD_FEED)
        return BDMF_ERR_INTERNAL;

    if (type == CPU_IF_RDD_DATA)
    {
#ifndef RDP_SIM
        entry = ((RDD_CPU_RING_DESCRIPTOR_DTS *)RDD_CPU_RING_DESCRIPTORS_TABLE_PTR(get_runner_idx(cfe_core_runner_image))) + ring_id;
        RDD_CPU_RING_DESCRIPTOR_READ_IDX_READ(read_idx, entry);
        RDD_CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_READ(size_of_ring, entry);
#else
        /* XXX: Fix simulator to use work with non READ_G/WRITE_G macros */
        RDD_CPU_RING_DESCRIPTOR_READ_IDX_READ_G(read_idx, RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR, ring_id);
        RDD_CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_READ_G(size_of_ring, RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR,
            ring_id);
#endif
        RDD_CPU_RING_DESCRIPTOR_READ_IDX_WRITE_G(((read_idx + delta) % (size_of_ring << CPU_RING_SIZE_32_RESOLUTION)), RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR, ring_id);
    }
    else
    {
#ifndef RDP_SIM
        RDD_CPU_RING_DESCRIPTOR_DTS *entry = get_recycle_ring_entry(ring_id);

        RDD_CPU_RING_DESCRIPTOR_READ_IDX_READ(read_idx, entry);
        RDD_CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_READ(size_of_ring, entry);
        RDD_CPU_RING_DESCRIPTOR_READ_IDX_WRITE(((read_idx + delta) % (size_of_ring << 5)), entry);
#else
        RDD_CPU_RING_DESCRIPTOR_READ_IDX_READ_G(read_idx, RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
        RDD_CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_READ_G(size_of_ring, RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
        RDD_CPU_RING_DESCRIPTOR_READ_IDX_WRITE_G(((read_idx + delta) % (size_of_ring << CPU_RING_SIZE_32_RESOLUTION)), RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
#endif
    }

    return BDMF_ERR_OK;
}


static inline int rdd_cpu_get_write_idx(uint32_t ring_id, uint8_t type, uint32_t *write_idx)
{
#ifndef RDP_SIM
    RDD_CPU_RING_DESCRIPTOR_DTS *entry;
#endif


#ifndef RDP_SIM
    if (type == CPU_IF_RDD_DATA)
        entry = ((RDD_CPU_RING_DESCRIPTOR_DTS *)RDD_CPU_RING_DESCRIPTORS_TABLE_PTR(get_runner_idx(cfe_core_runner_image))) + ring_id;
    else if (type == CPU_IF_RDD_FEED)
        entry = (RDD_CPU_RING_DESCRIPTOR_DTS *)RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_PTR(get_runner_idx(cfe_core_runner_image));
    else
        entry = get_recycle_ring_entry(ring_id);

    RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_READ(*write_idx, entry);
#else
    /* XXX: Fix simulator to use work with non READ_G/WRITE_G macros */
    if (type == CPU_IF_RDD_DATA)
        RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_READ_G(*write_idx, RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR, ring_id);
    else if (type == CPU_IF_RDD_FEED)
        RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_READ_G(*write_idx, RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
    else if (type == CPU_IF_RDD_RECYCLE)
        RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_READ_G(*write_idx, RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
#endif

    return BDMF_ERR_OK;
}

static inline int rdd_cpu_get_read_idx(uint32_t ring_id, uint8_t type, uint32_t *read_idx)
{
#ifndef RDP_SIM
    RDD_CPU_RING_DESCRIPTOR_DTS *entry;
#endif

#ifndef RDP_SIM
    if (type == CPU_IF_RDD_DATA)
        entry = ((RDD_CPU_RING_DESCRIPTOR_DTS *)RDD_CPU_RING_DESCRIPTORS_TABLE_PTR(get_runner_idx(cfe_core_runner_image))) + ring_id;
    else if (type == CPU_IF_RDD_RECYCLE)
        entry = get_recycle_ring_entry(ring_id);
    else
        entry = ((RDD_CPU_RING_DESCRIPTOR_DTS *)RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_PTR(get_runner_idx(cfe_core_runner_image)));

    RDD_CPU_RING_DESCRIPTOR_READ_IDX_READ(*read_idx, entry);
#else
    /* XXX: Fix simulator to use work with non READ_G/WRITE_G macros */
    if (type == CPU_IF_RDD_DATA)
        RDD_CPU_RING_DESCRIPTOR_READ_IDX_READ_G(*read_idx, RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR, ring_id);
    else if (type == CPU_IF_RDD_RECYCLE)
        RDD_CPU_RING_DESCRIPTOR_READ_IDX_READ_G(*read_idx, RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
    else
        RDD_CPU_RING_DESCRIPTOR_READ_IDX_READ_G(*read_idx, RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_ADDRESS_ARR, 0);
#endif

    return BDMF_ERR_OK;
}



void rdd_cpu_rx_init(void);

#endif /* _RDD_CPU_RX_H_ */

