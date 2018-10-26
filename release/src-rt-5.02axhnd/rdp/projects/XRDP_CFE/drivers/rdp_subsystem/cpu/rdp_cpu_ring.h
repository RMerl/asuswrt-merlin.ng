/*
    <:copyright-BRCM:2013:DUAL/GPL:standard

       Copyright (c) 2013 Broadcom
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

/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation of the Runner CPU ring interface     */
/*                                                                            */
/******************************************************************************/

#ifndef _RDP_CPU_RING_H_
#define _RDP_CPU_RING_H_

#if !defined(RDP_SIM) || defined(XRDP)

#if defined(__KERNEL__) || defined(_CFE_)
#include<bcm_pkt_lengths.h>
#include <bcm_mm.h>
#if defined(_CFE_) && defined(XRDP)
#include "rdp_subsystem_common.h"
#endif
#endif

#include "rdpa_types.h"
#include "rdpa_cpu_basic.h"
#include "rdd.h"
#include "rdp_cpu_ring_defs.h"

#ifdef _CFE_

#include "lib_malloc.h"
#include "cfe_iocb.h"
#ifndef XRDP
#define RDP_CPU_RING_MAX_QUEUES	1
#define RDP_WLAN_MAX_QUEUES		0
#endif
#define rdpa_cpu_rx_info_t int

#else

#include "rdpa_cpu.h"
#include "bdmf_system.h"
#include "bdmf_shell.h"
#include "bdmf_dev.h"

#ifdef XRDP
#include "rdd_cpu_rx.h"
#define RDPA_PKT_MIN_ALIGN 128
#ifndef RDP_SIM
#define DEF_DATA_RING_SIZE  1024
#else
#define DEF_DATA_RING_SIZE  128
#endif

#define RECYCLE_RING_SIZE  (DEF_DATA_RING_SIZE * RDD_CPU_RING_DESCRIPTORS_TABLE_SIZE)
#ifdef CONFIG_BCM96836
#define FEED_RING_SIZE     (DEF_DATA_RING_SIZE * 8)
#else
#define FEED_RING_SIZE     (DEF_DATA_RING_SIZE * RDD_CPU_RING_DESCRIPTORS_TABLE_SIZE)
#endif
#else
#define RDPA_PKT_MIN_ALIGN 0
#define RDP_CPU_RING_MAX_QUEUES		RDPA_CPU_MAX_QUEUES
#define RDP_WLAN_MAX_QUEUES		RDPA_WLAN_MAX_QUEUES
#endif

/* extern const bdmf_attr_enum_table_t rdpa_cpu_reason_enum_table; */
#endif

#ifdef RDP_SIM
#include "rdp_cpu_sim.h"
#define BCM_PKTBUF_SIZE 2048
#endif

typedef struct
{
   uint8_t* data_ptr;
   uint8_t data_offset;
   uint16_t packet_size;
   uint16_t flow_id;
   uint16_t reason;
   uint16_t src_bridge_port;
   uint16_t dst_ssid;
   uint16_t wl_metadata;
   uint16_t ptp_index;
   uint16_t free_index;
   uint8_t  is_rx_offload;
   uint8_t  is_ipsec_upstream;
   uint8_t  is_ucast;
   uint8_t  is_exception;
   uint8_t  is_csum_verified;
#ifdef XRDP
   uint8_t  mcast_tx_prio;
#endif
#ifdef CONFIG_CPU_REDIRECT_MODE_SUPPORT
   uint8_t  cpu_redirect_egress_queue;
   uint8_t  cpu_redirect_wan_flow;
#endif
}
CPU_RX_PARAMS;

#define MAX_BUFS_IN_CACHE 32
typedef struct RING_DESCTIPTOR RING_DESCTIPTOR;
typedef void* (*databuf_alloc_func)(RING_DESCTIPTOR *pDescriptor);
typedef void (*databuf_free_func)(void *pBuf, uint32_t context, RING_DESCTIPTOR *pDescriptor);
typedef void (*data_dump_func)(uint32_t rindId, rdpa_cpu_rx_info_t *info);
typedef void* (*memory_create_func)(RING_DESCTIPTOR *pDescriptor);
typedef void (*memory_delete_func)(void *buffMem);

struct RING_DESCTIPTOR
{
	uint32_t	   ring_id;
	uint32_t	   admin_status;
	uint32_t	   num_of_entries;
	uint32_t	   size_of_entry;
	uint32_t	   packet_size;
    rdpa_ring_type_t type;
    void* base;
#ifndef XRDP
	CPU_RX_DESCRIPTOR* head;
	CPU_RX_DESCRIPTOR* end;
#endif
	uint32_t           buff_cache_cnt;
    uint8_t **buff_cache;
	void *buff_mem_context;
	databuf_alloc_func databuf_alloc;
	databuf_free_func databuf_free;
	data_dump_func data_dump;
	memory_create_func memory_create;
	memory_delete_func memory_delete;
    uint32_t stats_received; /* for every queue */
    uint32_t stats_dropped;  /* for every queue */
    uint32_t stats_buff_err; /* buffer allocation failure */
    int dump_enable;
#ifdef XRDP
    uint32_t shadow_read_idx;
    uint32_t shadow_write_idx;
    uint32_t accum_inc;
    uint16_t *read_idx;
    uint16_t *write_idx;
#endif
};

typedef struct
{
#ifndef XRDP
    databuf_alloc_func databuf_alloc;
    databuf_free_func databuf_free;
#endif
    data_dump_func data_dump;
    void *buff_mem_context;
} RING_CB_FUNC;

#ifndef _CFE_

int rdp_cpu_ring_get_packet(uint32_t ringId, rdpa_cpu_rx_info_t *rxParams);

void rdp_cpu_reason_stat_cb(uint32_t *stat, rdpa_cpu_reason_index_t *rindex);

void rdp_cpu_rxq_stat_cb(int qid, extern_rxq_stat_t *stat, bdmf_boolean clear);

void rdp_cpu_dump_data_cb(bdmf_index queue, bdmf_boolean enabled);

int rdp_cpu_ring_read_packet_refill(uint32_t ring_id, CPU_RX_PARAMS *rxParams);

int rdp_cpu_ring_read_bulk(uint32_t ring_id, CPU_RX_PARAMS *rxParams, int max_count, int *count);

#ifdef XRDP
int rdp_cpu_ring_recycle_free_host_buf(int ring_id, int budget);
int rdp_cpu_fill_feed_ring(int budget);
#endif

#else /* !_CFE_*/

int rdp_cpu_ring_read_packet_copy(uint32_t ringId, CPU_RX_PARAMS* rxParams);

#endif

int rdp_cpu_ring_create_ring(uint32_t ring_id,
                             uint8_t ring_type,
                             uint32_t entries,
                             bdmf_phys_addr_t *ring_head, uint32_t packetSize,
                             RING_CB_FUNC *cbFunc);

int	rdp_cpu_ring_create_ring_ex(uint32_t ring_id,
                                uint8_t ring_type,
                                uint32_t entries,
                                bdmf_phys_addr_t* ring_head,
                                bdmf_phys_addr_t* rw_idx_addr,
                                uint32_t packetSize,
                                RING_CB_FUNC* ringCb);

int rdp_cpu_ring_delete_ring(uint32_t ringId);

void rdp_cpu_ring_free_mem(uint32_t ringId, void *pBuf);

int rdp_cpu_ring_get_queue_size(uint32_t ringId);

int rdp_cpu_ring_get_queued(uint32_t ringId);

int rdp_cpu_ring_flush(uint32_t ringId);

int rdp_cpu_ring_not_empty(uint32_t ringId);

int rdp_cpu_ring_is_full(uint32_t ringId);

int cpu_ring_shell_list_rings(void *shell_priv, int start_from);

int cpu_ring_shell_print_pd(void *shell_priv, uint32_t ring_id, uint32_t pdIndex);

int cpu_ring_shell_admin_ring(void *shell_priv, uint32_t ring_id, uint32_t admin_status);

/* Callback Functions */

void rdp_packet_dump(uint32_t ringId, rdpa_cpu_rx_info_t *info);

/* BPM (or CFE)*/

void* rdp_databuf_alloc(RING_DESCTIPTOR *pDescriptor);

void rdp_databuf_free(void *pBuf, uint32_t context, RING_DESCTIPTOR *pDescriptor);

/* Kmem_Cache */

void* rdp_databuf_alloc_cache(RING_DESCTIPTOR *pDescriptor);

void rdp_databuf_free_cache(void *pBuf, uint32_t context, RING_DESCTIPTOR *pDescriptor);

void rdp_cpu_ring_read_idx_ddr_sync(uint32_t ring_id);

#else /* !defined(RDP_SIM) || defined(XRDP) */
#include "rdp_cpu_ring_sim.h"
#define RDP_CPU_RING_MAX_QUEUES        RDPA_CPU_MAX_QUEUES
#define RDP_WLAN_MAX_QUEUES        RDPA_WLAN_MAX_QUEUES
#endif /* !defined(RDP_SIM) || defined(XRDP) */

/*array of possible rings private data*/
#ifndef XRDP
#define RING_ID_NUM_OF (RDP_CPU_RING_MAX_QUEUES + RDP_WLAN_MAX_QUEUES)
#endif
#define D_NUM_OF_RING_DESCRIPTORS RING_ID_NUM_OF
#endif /* _RDP_CPU_RING_H_ */
