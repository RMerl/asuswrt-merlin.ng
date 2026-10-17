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

#include "rdp_mm.h"
#include "linux/nbuff.h"
#include "bcm_prefetch.h"

#define CPU_RX_DESCRIPTOR_STRUCT CPU_RX_DESCRIPTOR

#define SIZE_OF_RING_DESCRIPTOR sizeof(CPU_RX_DESCRIPTOR_STRUCT)

/** \addtogroup cpu_rx
 * @{
 */
#define RDPA_MAX_BUFFERS_IN_RING_CACHE   32

#if defined(CONFIG_BCM963138)
#define PACKET_PREFETCH_CACHE_LINES 3
#else
#define PACKET_PREFETCH_CACHE_LINES 2
#endif

typedef struct
{
    uint32_t ring_size;
    uint32_t descriptor_size;
    CPU_RX_DESCRIPTOR_STRUCT *head;
    CPU_RX_DESCRIPTOR_STRUCT *base;
    CPU_RX_DESCRIPTOR_STRUCT *end;
    uint32_t buff_cache_cnt;
    void **buff_cache;
    uint32_t ring_prio;
} RING_DESCRIPTOR_S;

static inline int rdpa_cpu_rx_pd_get(void *raw_desc, rdpa_cpu_rx_info_t *rx_pd)
{
    CPU_RX_DESCRIPTOR_STRUCT rx_desc;

#if defined(__ARMEL__)
    register uint32_t w0 __asm__("r8");
    register uint32_t w1 __asm__("r9");
    register uint32_t w2 __asm__("r10");

    READ_RX_DESC(raw_desc, w0, w1, w2);
#elif defined(__AARCH64EL__)
    register uint64_t dw0 __asm__("x9");
    register uint64_t dw1 __asm__("x10");

    READ_RX_DESC(raw_desc, dw0, dw1);
#else
    CPU_RX_DESCRIPTOR_STRUCT *p_desc = (CPU_RX_DESCRIPTOR_STRUCT *)raw_desc;
#endif

    /* p_desc is in uncached memory, so reading 32bits at a time into
     cached memory improves performance. Is changed to BurstBank read later*/
#if defined(__ARMEL__)
    rx_desc.word2 = swap4bytes(w2);
#elif defined(__AARCH64EL__)
    *((uint64_t *)&rx_desc.word2) = swap4bytes64(dw1);
#else
    rx_desc.word2 = p_desc->word2;
#endif
    if (rx_desc.word2 & 0x80000000)
    {
        rx_desc.word2 &= ~0x80000000;
        rx_pd->data = (void *)PHYS_TO_CACHED(rx_desc.word2);

#if defined(__ARMEL__)
        rx_desc.word0 = swap4bytes(w0);
        rx_desc.word1 = swap4bytes(w1);
#elif defined(__AARCH64EL__)
        *((uint64_t *)&rx_desc.word0) = swap4bytes64(dw0);
#else
        rx_desc.word0 = p_desc->word0;
#endif
        rx_pd->size = rx_desc.packet_length;
        rx_pd->reason_data = rx_desc.flow_id;
        cache_invalidate_len_outer_first((void *)rx_pd->data, rx_pd->size);

#if !defined(__AARCH64EL__)
        rx_desc.word1 = p_desc->word1;
        rx_desc.word1 = swap4bytes(rx_desc.word1);
        rx_desc.word3 = p_desc->word3;
        rx_desc.word3 = swap4bytes(rx_desc.word3);
#endif
        rx_pd->reason = (rdpa_cpu_reason)rx_desc.reason;
        rx_pd->dest_ssid = rx_desc.dst_ssid;
        rx_pd->wl_metadata = rx_desc.wl_metadata;
        rx_pd->data_offset = 0;

#if defined(CONFIG_RUNNER_CSO)
        rx_pd->rx_csum_verified = rx_desc.is_chksum_verified;
#endif
        rx_pd->vport = rx_desc.src_port;
        rx_pd->handle = rdpa_cpu_get_port_handle(rx_pd->vport, &rx_pd->data);

        return 0;
    }

    return BDMF_ERR_NO_MORE;
}

/** Resets the descriptor with a new data pointer and sets descriptor ownership to the runner subsystem.
 *
 * \param[in]   raw_desc           Raw packet descriptor.
 * \param[out]  data               New data pointer.
 */
static inline void rdpa_cpu_ring_rest_desc(volatile void *__restrict__ raw_desc, void *__restrict__ data)
{
    volatile CPU_RX_DESCRIPTOR_STRUCT *p_desc = (volatile CPU_RX_DESCRIPTOR_STRUCT *)raw_desc;

    p_desc->word2 = swap4bytes(((VIRT_TO_PHYS(data)) & 0x7fffffff));
}

/** Check if there is a valid packet in descriptor
 *
 * \param[in]   raw_desc            Raw packet descriptor.
 * \return true if there is a valid packet in descriptor.
 */
static const inline int rdpa_cpu_ring_not_empty(const void *raw_desc)
{
   CPU_RX_DESCRIPTOR_STRUCT *p_desc = (CPU_RX_DESCRIPTOR_STRUCT *)raw_desc;

#if defined(__ARMEL__) || defined(__AARCH64EL__)
    return p_desc->word2 & 0x80;
#else
    return p_desc->word2 & 0x80000000;
#endif
}

#endif


