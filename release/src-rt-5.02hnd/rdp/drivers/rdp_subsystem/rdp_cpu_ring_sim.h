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
    uint16_t wl_metadata;
    uint16_t ptp_index;    
    uint16_t free_index;
    uint8_t  is_rx_offload;
    uint8_t  is_ipsec_upstream;
    uint8_t  is_ucast;
    uint8_t  is_exception;
} CPU_RX_PARAMS;

typedef struct
{
    void *buff_mem_context;
} RING_CB_FUNC;

static inline int rdp_cpu_ring_create_ring( uint32_t ringId, uint8_t type,
        uint32_t entries,  void *ic_cfg_p, uint32_t packetSize, RING_CB_FUNC *cbFunc )
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
