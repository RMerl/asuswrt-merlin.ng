/*
* <:copyright-BRCM:2013:DUAL/GPL:standard
* 
*    Copyright (c) 2013 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
* 
* :>
*/
#ifndef _RDPA_CPU_HELPER_H
#define _RDPA_CPU_HELPER_H

#ifndef CM3390
#include "rdp_cpu_ring_defs.h"
#else
#include "packing.h"
#include "rdp_drv_bpm.h"
#include "bdmf_system_common.h"
#include "rdpa_cpu.h"
#endif
#ifdef CONFIG_BCM96858
#include "rdd_data_structures_auto.h"
#include "rdd_runner_proj_defs.h"
#endif

#define SIZE_OF_RING_DESCRIPTOR sizeof(CPU_RX_DESCRIPTOR)

#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#define RUNNER_SOURCE_PORT_PCI  19
#else
#define RUNNER_SOURCE_PORT_PCI  8
#endif

extern rdpa_if map_rdd_to_rdpa_if[];

#ifdef CONFIG_BCM96858
extern rdpa_if rdd_vport_to_rdpa_if_map[];

static inline void decode_rnr_src(CPU_RX_DESCRIPTOR *rx_desc, rdpa_cpu_rx_info_t *rx_pd)
{
    rx_pd->src_port = rdd_vport_to_rdpa_if_map[rx_desc->wan.source_port];
    if (rx_desc->wan.is_src_wan)
        rx_pd->reason_data = rx_desc->wan.wan_flow_id;
    else if (rx_desc->cpu_vport.vport >= RDD_CPU_VPORT_FIRST && rx_desc->cpu_vport.vport <= RDD_CPU_VPORT_LAST)
        rx_pd->dest_ssid = rx_desc->cpu_vport.ssid;
}
#else
static inline rdpa_if rdpa_cpu_rx_srcport_to_rdpa_if(uint16_t rdd_srcport, int flow_id)
{
#ifndef BRCM_FTTDP
    /* Special case for wifi packets: if src_port is PCI then need to set
     * SSID */
    return (rdd_srcport == RUNNER_SOURCE_PORT_PCI) ? rdpa_if_ssid0 +
        flow_id : map_rdd_to_rdpa_if[rdd_srcport];
#else
    switch (rdd_srcport)
    {
    case 0:
        return rdpa_if_wan0;
    /* .. upto number-of-lan-ifs + 1 */
    case 1 ... rdpa_if_lan_max - rdpa_if_lan0 + 1 + 1:
        return rdpa_if_lan0 + rdd_srcport - 1;
    default:
        return rdpa_if_none;
    }
#endif
}
#endif

#ifdef CM3390

#pragma pack(push, 1)
typedef struct
{
    uint32_t offset:9         __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t rsv1:7           __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t priority:3       __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t src_subid:5      __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t src_port:5       __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t rsv2:2           __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t pd_size:1        __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t packet_size:12   __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t token_idx:16     __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t ddr:2            __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t rsv0:1           __PACKING_ATTRIBUTE_FIELD_LEVEL__;
    uint32_t valid:1          __PACKING_ATTRIBUTE_FIELD_LEVEL__;
} DOCSIS_PD;
#pragma pack(pop)

typedef void (*cpu_tx_dir_queue)(const rdpa_cpu_tx_info_t *info, void *raw_desc);

static inline int cpu_get_docsis_hdr_len(void)
{
    return 8;
}

static inline void cpu_tx_encode_docsis_hdr(void *docsis_hdr, uint32_t offset, uint32_t token, uint32_t wan_type, rdpa_if srcport)
{
    DOCSIS_PD *docsispd = (DOCSIS_PD *)docsis_hdr;
    uint32_t *swapptr = (uint32_t *)docsis_hdr;
    rdp_fpm_index fpm_idx = *(rdp_fpm_index *)&token;

    memset(docsispd, 0, sizeof(DOCSIS_PD));

    /* fill the docsis header */
    docsispd->pd_size = 0; /* always 8 */
    switch (srcport)
    {
        case rdpa_if_lan0:
            docsispd->src_port = RDD_LAN0_VPORT;
            break;
        case rdpa_if_lan1:
            docsispd->src_port = RDD_LAN1_VPORT;
            break;
        case rdpa_if_lan2:
            docsispd->src_port = RDD_LAN2_VPORT;
            break;
        case rdpa_if_lan3:
            docsispd->src_port = RDD_LAN3_VPORT;
            break;
        case rdpa_if_lan4:
            docsispd->src_port = RDD_LAN4_VPORT;
            break;

        case rdpa_if_ssid0 ... rdpa_if_ssid7:
            docsispd->src_port = RDD_WIFI_VPORT;
            docsispd->src_subid = srcport - rdpa_if_ssid0;
            break;

        case rdpa_if_ssid8 ... rdpa_if_ssid15:
            docsispd->src_port = RDD_WIFI_VPORT;
            docsispd->src_subid = srcport - rdpa_if_ssid8;
            break;

        default:
        case rdpa_if_cpu:
            docsispd->src_port = RDD_CPU_VPORT;
            break;
    }
    docsispd->ddr = fpm_idx.ddr;
    docsispd->valid = wan_type;
    docsispd->offset = offset + sizeof(DOCSIS_PD);
    docsispd->token_idx = fpm_idx.token_index;
    docsispd->packet_size = fpm_idx.token_size;

    *swapptr = swap4bytes(*swapptr);
    swapptr++;
    *swapptr = swap4bytes(*swapptr);
}

static void cpu_tx_ds_forward(const rdpa_cpu_tx_info_t *info, void *raw_desc)
{
    RDD_CPU_TX_DS_FORWARD_DESCRIPTOR_DTS *rnr_desc = (RDD_CPU_TX_DS_FORWARD_DESCRIPTOR_DTS *)raw_desc;
    rdp_fpm_index fpm_idx = *(rdp_fpm_index *)&info->data;

    memset(rnr_desc, 0, sizeof(RDD_CPU_TX_DS_FORWARD_DESCRIPTOR_DTS));

    rnr_desc->src_bridge_port = info->port == rdpa_if_wan0 ? RDD_WAN0_VPORT : RDD_WAN1_VPORT;
    rnr_desc->payload_offset = info->data_offset;
    rnr_desc->packet_length = info->data_size;
    rnr_desc->ih_class = info->port == rdpa_if_wan0 ? 8 : 9;
    rnr_desc->buffer_number = fpm_idx.token_index;
    rnr_desc->ddr_id = fpm_idx.ddr;
    rnr_desc->valid = 1;
}

static void cpu_tx_us_forward(const rdpa_cpu_tx_info_t *info, void *raw_desc)
{
    RDD_CPU_TX_US_FORWARD_DESCRIPTOR_DTS *rnr_desc = (RDD_CPU_TX_US_FORWARD_DESCRIPTOR_DTS *)raw_desc;
    rdp_fpm_index fpm_idx = *(rdp_fpm_index *)&info->data;

    memset(rnr_desc, 0, sizeof(RDD_CPU_TX_US_FORWARD_DESCRIPTOR_DTS));
    /* TODO:Convert switch to map */
    if (info->port >= rdpa_if_ssid0  && info->port <= rdpa_if_ssid15)
    {
        rnr_desc->src_bridge_port = RDD_WIFI_VPORT;
        rnr_desc->src_ssid = info->port - rdpa_if_ssid0;
        rnr_desc->ih_class = 2;
    }
    else
    {
        rnr_desc->src_bridge_port = info->port - rdpa_if_lan0 + RDD_VPORT_ID_1;
        rnr_desc->ih_class = info->port - rdpa_if_lan0 + 10;
    }
    rnr_desc->payload_offset = info->data_offset;
    rnr_desc->packet_length = info->data_size + 4; /*must padd the packet for the runner*/
    rnr_desc->buffer_number = fpm_idx.token_index;
    rnr_desc->ddr_id = fpm_idx.ddr;
    rnr_desc->valid = 1;
}

static void cpu_tx_ds_egress(const rdpa_cpu_tx_info_t *info, void *raw_desc)
{
    RDD_CPU_TX_DS_EGRESS_DESCRIPTOR_DTS *rnr_desc = (RDD_CPU_TX_DS_EGRESS_DESCRIPTOR_DTS *)raw_desc;
    rdp_fpm_index fpm_idx = *(rdp_fpm_index *)&info->data;

    memset(rnr_desc , 0 , sizeof(RDD_CPU_TX_DS_EGRESS_DESCRIPTOR_DTS));
    if (info->port >= rdpa_if_ssid0  && info->port <= rdpa_if_ssid15)
    {
        rnr_desc->emac = RDD_WIFI_VPORT;
        rnr_desc->dst_ssid = info->port - rdpa_if_ssid0;
    }
    else
    {
        rnr_desc->emac = info->port - rdpa_if_lan0 + RDD_LAN0_VPORT;
    }
    rnr_desc->tx_queue = info->x.lan.queue_id;
    rnr_desc->src_bridge_port = RDD_CPU_VPORT;
    rnr_desc->payload_offset = info->data_offset;
    rnr_desc->packet_length = info->data_size; /*must padd the packet for the runner*/
    rnr_desc->buffer_number = fpm_idx.token_index;
    rnr_desc->ddr_id = fpm_idx.ddr;
    rnr_desc->valid = 1;
}

static void cpu_tx_us_egress(const rdpa_cpu_tx_info_t *info, void *raw_desc)
{
    RDD_CPU_TX_US_EGRESS_DESCRIPTOR_DTS *rnr_desc = (RDD_CPU_TX_US_EGRESS_DESCRIPTOR_DTS *)raw_desc;
    rdp_fpm_index fpm_idx = *(rdp_fpm_index *)&info->data;

    memset(rnr_desc, 0, sizeof(RDD_CPU_TX_US_EGRESS_DESCRIPTOR_DTS));
    rnr_desc->wan_flow = info->port;
    rnr_desc->tx_queue = info->x.wan.queue_id;
    rnr_desc->payload_offset = info->data_offset;
    rnr_desc->packet_length = info->data_size + 4; /*must padd the packet for the runner*/
    rnr_desc->buffer_number = fpm_idx.token_index;
    rnr_desc->ddr_id = fpm_idx.ddr;
    rnr_desc->valid = 1;
}

static cpu_tx_dir_queue cpu_tx_array[2][2] =
{
    {cpu_tx_ds_egress, cpu_tx_us_egress},
    {cpu_tx_us_forward, cpu_tx_ds_forward}
};

inline int rdpa_cpu_tx_pd_set(const rdpa_cpu_tx_info_t *info, void *raw_desc)
{
    int dir = (info->port == rdpa_if_wan0) | (info->port == rdpa_if_wan1);
    cpu_tx_array[info->method][dir](info, raw_desc);

    return 0;
}

#define CMIM_INC_BM_MASK    0x8000
#define CMIM_PRIORITY_MASK  0x7000

/* in 3390 Cpu RX descriptor is different than Legacy RDP */
inline int rdpa_cpu_rx_pd_get(void *raw_desc /* Input */, rdpa_cpu_rx_info_t *rx_pd/* Output */)
{
    RDD_CPU_RX_DESCRIPTOR_DTS *p_desc = (RDD_CPU_RX_DESCRIPTOR_DTS *)raw_desc;
    RDD_CPU_RX_DESCRIPTOR_DTS rx_desc;
    rdp_fpm_index fpm_idx = {};

    /* copy descriptor to local cached memory, assuming arm memcpy is efficiany */
    memcpy(&rx_desc, p_desc, sizeof(RDD_CPU_RX_DESCRIPTOR_DTS));

    fpm_idx.token_size = rx_desc.packet_length;
    fpm_idx.ddr = rx_desc.ddr_id;
    fpm_idx.token_index = rx_desc.buffer_number;
    fpm_idx.token_valid = 1;

    rx_pd->data = *((uint32_t *)&fpm_idx);
    rx_pd->size = rx_desc.packet_length;
    rx_pd->src_port = rdpa_cpu_rx_srcport_to_rdpa_if(rx_desc.src_bridge_port, rx_desc.src_ssid);
    rx_pd->reason = (rdpa_cpu_reason) rx_desc.reason;
    rx_pd->data_offset = rx_desc.payload_offset;
    rx_pd->dest_ssid = rx_desc.wifi_cmim_union;
    rx_pd->cmim_priority = (rx_desc.wifi_cmim_union & CMIM_PRIORITY_MASK) >> 12;
    rx_pd->cmim_inc_bm = (rx_desc.wifi_cmim_union & CMIM_INC_BM_MASK) ? 1 : 0;
    rx_pd->cmim_if_bitmask = (rx_desc.wifi_cmim_union & CMIM_INC_BM_MASK) ? rx_desc.wifi_egress_params_cmim_union : 0;

    if (rx_pd->cmim_inc_bm)
        rx_pd->cmim_if_bitmask = rdpa_cpu_rx_srcport_to_rdpa_if(rx_pd->cmim_if_bitmask, 0);
    else
        rx_pd->cmim_if_bitmask = 0;

    return 0;
}
#endif

/** \addtogroup cpu_rx
 * @{
 */
#ifdef CONFIG_BCM96858

/** Translates raw packet to formated rdpa_cpu_rx_info_t structure.
 *
 * \param[in]   raw_desc           Raw packet descriptor.
 * \param[out]  rx_pd              Formated packet descriptor.
 * \return BDMF_ERR_NO_MORE if packet descriptor owned by runner subsystem. 
 */
inline int rdpa_cpu_rx_pd_get(void *raw_desc, rdpa_cpu_rx_info_t *rx_pd)
{
    CPU_RX_DESCRIPTOR rx_desc;
    register uint64_t dword0 asm ("x8");
    register uint64_t dword1 asm ("x9");

    /* Using this Aarch64 Assembly optimization to reduce uncached descriptor read time
     * ldnp: load pair of registers with non-temporal hint (Uncached) */

    __asm__("ldnp   %1, %2,[%0]" \
        :  "=r" (raw_desc), "=r" (dword0), "=r" (dword1) \
        : "0" (raw_desc));

    /* Swap two 32bit words in single instruction */
    *(uint64_t*)&rx_desc.word0 = swap4bytes64(dword0);
    *(uint64_t*)&rx_desc.word2 = swap4bytes64(dword1);

    if (rx_desc.abs.ownership == OWNERSHIP_HOST)
    {
        uintptr_t phys_ptr;

        phys_ptr = ((uintptr_t)rx_desc.abs.host_buffer_data_ptr_hi) << 32;
        phys_ptr |= rx_desc.abs.host_buffer_data_ptr_low;
        rx_pd->data = (void *)phys_to_virt(phys_ptr);

        rx_pd->size = rx_desc.abs.packet_length;

        /* The place of data_ofset is the same in all structures in this union we could use any.*/
        rx_pd->data_offset = rx_desc.wan.data_offset;

        decode_rnr_src(&rx_desc, rx_pd);

        rx_pd->reason = (rdpa_cpu_reason)rx_desc.wan.reason;
        rx_pd->wl_metadata = rx_desc.wl_metadata;

        return 0;
    }

    return BDMF_ERR_NO_MORE;
}

/** Resets the descriptor with a new data pointer and sets descriptor ownership to the runner subsystem.
 *
 * \param[in]   raw_desc           Raw packet descriptor.
 * \param[out]  data               New data pointer.
 */
inline void rdpa_cpu_ring_rest_desc(volatile void *__restrict__ raw_desc, void *__restrict__ data)
{
    /* in XRDP address is 48 bits, using 64bit register to store the descriptor at once */
    uint64_t *word0 = (uint64_t *)raw_desc;

    /* set 48bit address and clean ownership to runner */
    *word0 = swap4bytes64((uint64_t)data & 0x1FFFFFFFFFFFF);
}

/** Check if there is a valid packet in descriptor
 *
 * \param[in]   raw_desc            Raw packet descriptor.
 * \return true if there is a valid packet in descriptor.
 */
const inline int rdpa_cpu_ring_not_empty(const void *raw_desc)
{
    CPU_RX_DESCRIPTOR *rx_desc = (CPU_RX_DESCRIPTOR *)raw_desc;
    return rx_desc->abs.ownership == OWNERSHIP_HOST;
}

/** Check if there is a valid packet in descriptor
 *
 * \param[in]   raw_desc            Raw packet descriptor.
 * \return      Data pointer.
 */
const inline uintptr_t rdpa_cpu_ring_get_data_ptr(const void *raw_desc)
{
    CPU_RX_DESCRIPTOR *rx_desc = (CPU_RX_DESCRIPTOR *)raw_desc;
    return (uintptr_t)rx_desc->abs.host_buffer_data_ptr_hi << 32 |
            rx_desc->abs.host_buffer_data_ptr_low;
}
#elif !defined(CM3390)
inline int rdpa_cpu_rx_pd_get(void *raw_desc, rdpa_cpu_rx_info_t *rx_pd)
{
    CPU_RX_DESCRIPTOR rx_desc;
#if defined(__ARMEL__)
    register uint32_t w0 __asm__("r8");
    register uint32_t w1 __asm__("r9");
    register uint32_t w2 __asm__("r10");

    READ_RX_DESC(raw_desc);
#else
    CPU_RX_DESCRIPTOR *p_desc = (CPU_RX_DESCRIPTOR *)raw_desc;
#endif

    /* p_desc is in uncached mem so reading 32bits at a time into
     cached mem improves performance will be change to BurstBank read later*/
#if defined(__ARMEL__)
    rx_desc.word2 = swap4bytes(w2);
#elif defined(WL4908)
    rx_desc.word2 = swap4bytes(p_desc->word2);
#else
    rx_desc.word2 = p_desc->word2;
#endif
    if (rx_desc.word2 & 0x80000000)
    {
        rx_desc.word2 &= ~0x80000000;
        rx_pd->data = (void *)PHYS_TO_CACHED(rx_desc.word2);

#if defined(__ARMEL__)
        rx_desc.word0 = swap4bytes(w0);
#elif defined(WL4908)
        rx_desc.word0 = swap4bytes(p_desc->word0);
#else
        rx_desc.word0 = p_desc->word0;
#endif
        rx_pd->size = rx_desc.packet_length;
        cache_invalidate_len_outer_first((void *)rx_pd->data, rx_pd->size);

#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
        rx_pd->reason_data = rx_desc.flow_id;
        rx_desc.word1 = p_desc->word1;
        rx_desc.word1 = swap4bytes(rx_desc.word1);
        rx_pd->reason = (rdpa_cpu_reason)rx_desc.reason;
        rx_pd->dest_ssid = rx_desc.dst_ssid;
        rx_desc.word3 = p_desc->word3;
        rx_desc.word3 = swap4bytes(rx_desc.word3);
        rx_pd->wl_metadata = rx_desc.wl_metadata;
        rx_pd->ptp_index = p_desc->ip_sync_1588_idx;
        rx_pd->data_offset = 0;
#else
        rx_pd->reason = rdpa_cpu_rx_reason_ip_flow_miss;
#endif

#if defined(CONFIG_BCM_PKTRUNNER_CSUM_OFFLOAD)
        rx_pd->rx_csum_verified = rx_desc.is_chksum_verified;
#endif
        rx_pd->src_port = rdpa_cpu_rx_srcport_to_rdpa_if(rx_desc.source_port, rx_desc.flow_id);
        return 0;
    }

    return BDMF_ERR_NO_MORE;
}

/** Resets the descriptor with a new data pointer and sets descriptor ownership to the runner subsystem.
 *
 * \param[in]   raw_desc           Raw packet descriptor.
 * \param[out]  data               New data pointer.
 */
inline void rdpa_cpu_ring_rest_desc(volatile void *__restrict__ raw_desc, void *__restrict__ data)
{
    volatile CPU_RX_DESCRIPTOR *p_desc = (volatile CPU_RX_DESCRIPTOR *)raw_desc;

    p_desc->word2 = swap4bytes(VIRT_TO_PHYS(data)) & 0x7fffffff;
}

/** Check if there is a valid packet in descriptor
 *
 * \param[in]   raw_desc            Raw packet descriptor.
 * \return true if there is a valid packet in descriptor.
 */
const inline int rdpa_cpu_ring_not_empty(const void *raw_desc)
{
   CPU_RX_DESCRIPTOR *p_desc = (CPU_RX_DESCRIPTOR *)raw_desc;

#if defined(__ARMEL__) || defined(WL4908)
    return p_desc->word2 & 0x80;
#else
    return p_desc->word2 & 0x80000000;
#endif
}

/** Check if there is a valid packet in descriptor
 *
 * \param[in]   raw_desc            Raw packet descriptor.
 * \return      Data pointer.
 */
const inline uintptr_t rdpa_cpu_ring_get_data_ptr(const void *raw_desc)
{
    CPU_RX_DESCRIPTOR *rx_desc = (CPU_RX_DESCRIPTOR *)raw_desc;
    return rx_desc->host_buffer_data_pointer;
}

/** @} end of add to cpu_rx Doxygen group */

#endif
#endif
