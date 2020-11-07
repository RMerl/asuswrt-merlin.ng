/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


#ifndef _RDP_CPU_RING_SIM_H
#define _RDP_CPU_RING_SIM_H

#include "rdpa_types.h"
#include "rdd.h"
#include "rdd_defs.h"
#ifdef LEGACY_RDP
#include "rdd_legacy_conv.h"
#endif

#define BCM_PKTBUF_SIZE 1532

typedef enum
{
    sysb_type_skb,
    sysb_type_fkb,
    sysb_type_raw,
} cpu_ring_sysb_type;

typedef enum
{
    type_cpu_rx,
    type_pci_tx
} cpu_ring_type;

typedef struct
{
    uint8_t *sysb_ptr;
    uint8_t *data_ptr;
    uint32_t packet_size;
#ifdef XRDP
    rdd_vport_id_t src_bridge_port;
    uint8_t data_offset;
#else
    rdd_bridge_port_t src_bridge_port;
#endif
    uint32_t flow_id;
    rdpa_cpu_reason reason;
    uint16_t dst_ssid;
    uint32_t wl_metadata;
    uint16_t ptp_index;    
    uint16_t free_index;
    uint8_t  is_rx_offload;
    uint8_t  is_ipsec_upstream;
    uint8_t  is_ucast;
    uint8_t  is_exception;
    uint8_t  mcast_tx_prio;
    uint8_t  color;
} CPU_RX_PARAMS;

typedef struct
{
    void *buff_mem_context;
} RING_CB_FUNC;

static inline int rdp_cpu_ring_create_ring( uint32_t ringId, uint8_t type,
    uint32_t entries,  void *ic_cfg_p, uint32_t packetSize, RING_CB_FUNC *cbFunc, uint32_t prio )
{
    return 0;
}

static inline int rdp_cpu_ring_create_ring_ex(uint32_t ring_id, uint8_t ring_type,
    uint32_t entries, bdmf_phys_addr_t* ring_head, bdmf_phys_addr_t* rw_idx_addr, uint32_t packetSize, RING_CB_FUNC* ringCb, uint32_t prio)
{
    return 0;
}

static inline int rdp_cpu_ring_delete_ring( uint32_t ringId )
{
    return 0;
}

static inline int rdp_cpu_ring_read_packet_refill(
    uint32_t ringId, CPU_RX_PARAMS* rxParams)
{
    return 0;
}

#if defined(CONFIG_BCM963138) || defined(_BCM963138_) || defined(CONFIG_BCM963148) || defined(_BCM963148_)
typedef struct
{
    uint32_t packet_length;
    uint32_t source_port;
    uint8_t *data_ptr;
    rdpa_cpu_reason reason;
} CPU_RX_DESCRIPTOR;

static inline int rdp_cpu_ring_read_packet_refill2(uint32_t ring_id, CPU_RX_DESCRIPTOR* rxDesc)
{
    return 0;
}
#endif

static inline int rdp_cpu_ring_bulk_skb_get(
    uint32_t ring_id, unsigned int budget, void ** rx_pkts)
{
    return 0;
}

static inline int rdp_cpu_ring_bulk_fkb_get(
    uint32_t ring_id, unsigned int budget, void ** rx_pkts)
{
    return 0;
}

static inline int rdp_cpu_ring_read_packet_copy( uint32_t ringId,CPU_RX_PARAMS* rxParams)
{
    return 0;
}

static inline int	rdp_cpu_ring_get_queue_size( uint32_t ringId)
{
    return 0;
}

static inline int	rdp_cpu_ring_get_queued( uint32_t ringId)
{
    return 0;
}

static inline int rdp_cpu_ring_flush(uint32_t ringId)
{
    return 0;
}
static inline int rdp_cpu_ring_not_empty(uint32_t ringId)
{
    return 0;
}
static inline int rdp_cpu_ring_is_full(uint32_t ringId)
{
    return 0;
}
#endif
