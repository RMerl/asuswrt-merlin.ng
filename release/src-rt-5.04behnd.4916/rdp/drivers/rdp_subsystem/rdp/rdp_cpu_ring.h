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

#if !defined(RDP_SIM)

#if defined(__KERNEL__)
#include<bcm_pkt_lengths.h>
#include <bcm_mm.h>
#endif

#include "rdpa_types.h"
#include "rdpa_cpu_basic.h"
#include "rdd.h"
#include "rdp_cpu_ring_defs.h"

#include "rdpa_cpu.h"
#include "bdmf_system.h"
#include "bdmf_shell.h"
#include "bdmf_dev.h"

#define WRITE_IDX_UPDATE_THR (128)

#define RDPA_PKT_MIN_ALIGN 0
#define RDP_CPU_RING_MAX_QUEUES		RDPA_CPU_MAX_QUEUES
#define RDP_WLAN_MAX_QUEUES		RDPA_WLAN_MAX_QUEUES

#ifdef RDP_SIM
#include "rdp_cpu_sim.h"
#define BCM_PKTBUF_SIZE 2048
#endif


#define CPU_RING_DEBUG
#ifdef CPU_RING_DEBUG
    #define DO_DEBUG(a_) a_
#else
    #define DO_DEBUG(a_) 
#endif


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
    uint32_t       ring_prio; 
    uint32_t	   num_of_entries;
    uint32_t	   num_of_entries_mask;
    uint32_t	   size_of_entry;
    uint32_t	   packet_size;
    rdpa_ring_type_t type;
    void* base;
    CPU_RX_DESCRIPTOR* head;
    CPU_RX_DESCRIPTOR* end;
    uint32_t           buff_cache_cnt;
    uint8_t **buff_cache;
    void *buff_mem_context;
    databuf_alloc_func databuf_alloc;
    databuf_free_func databuf_free;
    data_dump_func data_dump;
    memory_create_func memory_create;
    memory_delete_func memory_delete;
#ifdef CPU_RING_DEBUG
    uint32_t stats_received; /* for every queue */
    uint32_t stats_dropped;  /* for every queue */
    uint32_t stats_buff_err; /* buffer allocation failure */
    int dump_enable;
#endif /* CPU_RING_DEBUG */
};

typedef struct
{
    databuf_alloc_func databuf_alloc;
    databuf_free_func databuf_free;
    data_dump_func data_dump;
    void *buff_mem_context;
} RING_CB_FUNC;


void rdp_cpu_reason_stat_cb(uint32_t *stat, rdpa_cpu_reason_index_t *rindex);

void rdp_cpu_rxq_stat_cb(int qid, extern_rxq_stat_t *stat, bdmf_boolean clear);

void rdp_cpu_dump_data_cb(bdmf_index queue, bdmf_boolean enabled);

int rdp_cpu_ring_read_packet_refill(uint32_t ring_id, rdpa_cpu_rx_info_t *info);

int rdp_cpu_ring_create_ring(uint32_t ring_id,
                             uint8_t ring_type,
                             uint32_t entries,
                             bdmf_phys_addr_t *ring_head, uint32_t packetSize,
                             RING_CB_FUNC *cbFunc,
                             uint32_t prio);

int	rdp_cpu_ring_create_ring_ex(uint32_t ring_id,
                                uint8_t ring_type,
                                uint32_t entries,
                                bdmf_phys_addr_t* ring_head,
                                bdmf_phys_addr_t* rw_idx_addr,
                                uint32_t packetSize,
                                RING_CB_FUNC* ringCb,
                                uint32_t prio);

int rdp_cpu_ring_delete_ring(uint32_t ringId);

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

/* BPM */

void* rdp_databuf_alloc(RING_DESCTIPTOR *pDescriptor);

void rdp_databuf_free(void *pBuf, uint32_t context, RING_DESCTIPTOR *pDescriptor);

/* Kmem_Cache */

void* rdp_databuf_alloc_cache(RING_DESCTIPTOR *pDescriptor);

void rdp_databuf_free_cache(void *pBuf, uint32_t context, RING_DESCTIPTOR *pDescriptor);

void rdp_cpu_ring_read_idx_ddr_sync(uint32_t ring_id);

extern bdmf_fastlock feed_ring_lock;

uint32_t rdp_cpu_feed_ring_get_queued(void);

#if defined(CONFIG_BCM_RUNNER_FEED_RING_DYNAMIC)
extern int feed_ring_low_threshold;
extern int feed_min_buffers_in_ring;
extern atomic_t allocated_buffers;
#endif

#else /* !defined(RDP_SIM) */
#include "rdp_cpu_ring_sim.h"
#define RDP_CPU_RING_MAX_QUEUES        RDPA_CPU_MAX_QUEUES
#define RDP_WLAN_MAX_QUEUES        RDPA_WLAN_MAX_QUEUES
#endif /* !defined(RDP_SIM) */

/*array of possible rings private data*/
#define RING_ID_NUM_OF (RDP_CPU_RING_MAX_QUEUES + RDP_WLAN_MAX_QUEUES)
#define D_NUM_OF_RING_DESCRIPTORS RING_ID_NUM_OF

#endif /* _RDP_CPU_RING_H_ */
