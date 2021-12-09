/*
    <:copyright-BRCM:2014:DUAL/GPL:standard

       Copyright (c) 2014 Broadcom
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

#ifndef _RDD_COMMON_H
#define _RDD_COMMON_H

#include "rdd_defs.h"
#include "rdp_platform.h"
#include "rdd.h"
typedef enum
{
    WANBIT,
    LANBIT,
    ERRORBIT
} wan_or_lan_pd_bit_t;

#if defined(__LP64__) || defined(_LP64)
/* 64 bit*/
#define GET_ADDR_HIGH_LOW(msb_addr, lsb_addr, phys_ring_address) \
    lsb_addr = phys_ring_address & 0xFFFFFFFF; \
    msb_addr = phys_ring_address >> 32;
#else
/* 32 bit */
#define GET_ADDR_HIGH_LOW(msb_addr, lsb_addr, phys_ring_address) \
    lsb_addr = ((uint32_t)phys_ring_address & 0xFFFFFFFF); \
    msb_addr = 0;
#endif

#define PORT_OR_WAN_FLOW_TO_TX_FLOW(port , dir) (dir == rdpa_dir_us \
    ? WANBIT << RDD_LOG2_NUM_OF_WAN_FLOWS | port \
    : LANBIT << RDD_LOG2_NUM_OF_WAN_FLOWS | port)

#define RUNNER_CORE_CONTEXT_ADDRESS(rnr_idx)    DEVICE_ADDRESS(RU_BLK(RNR_CNTXT).addr[rnr_idx] + RU_REG_OFFSET(RNR_CNTXT, MEM_ENTRY))
#define PACKET_BUFFER_PD_PTR(base_addr, task_number) (base_addr + (task_number * sizeof(RDD_PACKET_BUFFER_DTS)))

#define LAYER2_HEADER_COPY_ROUTINE_ARRAY(var, image, prefix) \
    uint16_t var[] = { \
        [0] = ADDRESS_OF(image, prefix##_14_bytes_8_bytes_align), \
        [1] = ADDRESS_OF(image, prefix##_18_bytes_8_bytes_align), \
        [2] = ADDRESS_OF(image, prefix##_22_bytes_8_bytes_align), \
        [3] = ADDRESS_OF(image, prefix##_26_bytes_8_bytes_align), \
        [4] = ADDRESS_OF(image, prefix##_30_bytes_8_bytes_align), \
        [5] = ADDRESS_OF(image, prefix##_14_bytes_4_bytes_align), \
        [6] = ADDRESS_OF(image, prefix##_18_bytes_4_bytes_align), \
        [7] = ADDRESS_OF(image, prefix##_22_bytes_4_bytes_align), \
        [8] = ADDRESS_OF(image, prefix##_26_bytes_4_bytes_align), \
        [9] = ADDRESS_OF(image, prefix##_30_bytes_4_bytes_align), \
        [10] = ADDRESS_OF(image, prefix##_14_bytes_8_bytes_align), \
        [11] = ADDRESS_OF(image, prefix##_18_bytes_8_bytes_align), \
        [12] = ADDRESS_OF(image, prefix##_22_bytes_8_bytes_align), \
        [13] = ADDRESS_OF(image, prefix##_26_bytes_8_bytes_align), \
        [14] = ADDRESS_OF(image, prefix##_30_bytes_8_bytes_align), \
        [15] = ADDRESS_OF(image, prefix##_14_bytes_4_bytes_align), \
        [16] = ADDRESS_OF(image, prefix##_18_bytes_4_bytes_align), \
        [17] = ADDRESS_OF(image, prefix##_22_bytes_4_bytes_align), \
        [18] = ADDRESS_OF(image, prefix##_26_bytes_4_bytes_align), \
        [19] = ADDRESS_OF(image, prefix##_30_bytes_4_bytes_align), \
        [20] = ADDRESS_OF(image, prefix##_14_bytes_8_bytes_align), \
        [21] = ADDRESS_OF(image, prefix##_18_bytes_8_bytes_align), \
        [22] = ADDRESS_OF(image, prefix##_22_bytes_8_bytes_align), \
        [23] = ADDRESS_OF(image, prefix##_26_bytes_8_bytes_align), \
        [24] = ADDRESS_OF(image, prefix##_30_bytes_8_bytes_align), \
        [25] = ADDRESS_OF(image, prefix##_14_bytes_4_bytes_align), \
        [26] = ADDRESS_OF(image, prefix##_18_bytes_4_bytes_align), \
        [27] = ADDRESS_OF(image, prefix##_22_bytes_4_bytes_align), \
        [28] = ADDRESS_OF(image, prefix##_26_bytes_4_bytes_align), \
        [29] = ADDRESS_OF(image, prefix##_30_bytes_4_bytes_align), \
    }

#define ANY_VID                          0xFFFF

/* RX APIs*/
void rdd_rx_flow_cfg(uint32_t flow_index, rdd_flow_dest destination, rdd_rdd_vport vport, uint32_t cntr_id);
void rdd_mac_type_cfg(rdd_mac_type wan_mac_type);

/* TX APIs*/
void rdd_tx_flow_enable(uint16_t port_or_wan_flow, rdpa_traffic_dir dir, bdmf_boolean enable);
void rdd_fpm_pool_number_mapping_cfg(uint16_t fpm_base_token_size);


#endif /* _RDD_COMMON_H */
