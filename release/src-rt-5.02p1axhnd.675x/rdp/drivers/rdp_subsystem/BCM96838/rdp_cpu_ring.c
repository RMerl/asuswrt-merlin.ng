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

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#ifdef RDP_SIM
#define INTERN_PRINT bdmf_trace
#define ____cacheline_aligned 
#else
#define INTERN_PRINT printk
#endif

#if !defined(RDP_SIM) || defined(XRDP)
#include "rdp_cpu_ring.h"
#ifdef XRDP
#include "rdd_cpu_rx.h"
#include "rdp_drv_proj_cntr.h"
#if !defined(CFE) 
#include "rdd_runner_proj_defs.h"
#endif
#endif
#include "rdp_cpu_ring_inline.h"
#include "rdp_mm.h"


#ifndef _CFE_
#ifndef XRDP
#include "rdpa_cpu_helper.h"
#else /* XRDP */
#include "bdmf_system.h"
#endif /* XRDP */


static uint32_t init_shell = 0;
int stats_reason[2][rdpa_cpu_reason__num_of]  = {}; /* reason statistics for US/DS */
EXPORT_SYMBOL(stats_reason);

#define RW_INDEX_SIZE (sizeof(uint16_t))

#define shell_print(priv,format, ...) bdmf_session_print((bdmf_session_handle)priv,format,##__VA_ARGS__)
#else /* _CFE_ */
#define ____cacheline_aligned 
#define shell_print(dummy,format,...) xprintf(format, ##__VA_ARGS__)
#endif /* _CFE_ */

#if defined(XRDP)
#define SYNC_FIFO_ADDRESS (IMAGE_2_CPU_TX_SYNC_FIFO_TABLE_ADDRESS + CPU_TX_SYNC_FIFO_ENTRY_FIFO_WORD_OFFSET * 4)
#endif

#if !defined(XRDP) && !defined(__KERNEL__) && !defined(_CFE_)
#error "rdp_cpu_ring is supported only in CFE and Kernel modules or XRDP simulator"
#endif

RING_DESCTIPTOR ____cacheline_aligned host_ring[D_NUM_OF_RING_DESCRIPTORS]  = {};
EXPORT_SYMBOL(host_ring);
#ifdef XRDP
#if !defined(_CFE_) && !defined(RDP_SIM)
bdmf_fastlock feed_ring_lock;    
void rdp_recycle_buf_to_feed(void *pdata);
static uint32_t threshold_recycle = 0;
#define WRITE_IDX_UPDATE_THR (128)
#endif
#endif

int cpu_ring_shell_list_rings(void *shell_priv, int start_from)
{
    uint32_t cntr;
    uint32_t first = 0, last = D_NUM_OF_RING_DESCRIPTORS;
#ifdef XRDP
    uint16_t read_idx = 0, write_idx = 0;
#endif

    shell_print(shell_priv, "CPU RX Ring Descriptors \n" );
    shell_print(shell_priv, "------------------------------\n" );

    if (start_from != -1)
    {
        first = start_from;
        last = first + 1;
    }

    for (cntr = first; cntr < last; cntr++)
    {
        char *ring_type;
        if (!host_ring[cntr].num_of_entries) 
            continue;

        ring_type = "RX";
#ifdef XRDP
        rdp_cpu_get_read_idx(host_ring[cntr].ring_id, host_ring[cntr].type, &read_idx);
        rdp_cpu_get_write_idx(host_ring[cntr].ring_id, host_ring[cntr].type, &write_idx);

        if (host_ring[cntr].type == rdpa_ring_feed)
        {

            ring_type = "Feed";
#ifdef  CPU_RING_DEBUG
            shell_print(shell_priv, "Feed Ring allocation failures: = %d:\n", host_ring[cntr].stats_buff_err);
#endif            
        }

        else  if (host_ring[cntr].type == rdpa_ring_recycle)
            ring_type = "Recycle";
        else  if (host_ring[cntr].type == rdpa_ring_cpu_tx)
            ring_type = "Cpu_tx";
#endif

        shell_print(shell_priv, "CPU %s Ring Queue = %d:\n", ring_type, cntr );
        shell_print(shell_priv, "\tCPU %s Ring Queue id= %d\n",ring_type, host_ring[cntr].ring_id );
        shell_print(shell_priv, "\tNumber of entries = %d\n", host_ring[cntr].num_of_entries );
        shell_print(shell_priv, "\tSize of entry = %d bytes\n", host_ring[cntr].size_of_entry );
        shell_print(shell_priv, "\tAllocated Packet size = %d bytes\n", host_ring[cntr].packet_size );
        shell_print(shell_priv, "\tRing Base address = 0x%pK\n", host_ring[cntr].base );
#ifndef XRDP
        shell_print(shell_priv, "\tRing Head address = 0x%pK\n", host_ring[cntr].head );
        shell_print(shell_priv, "\tRing Head position = %ld\n",
            (long)(host_ring[cntr].head - (CPU_RX_DESCRIPTOR *)host_ring[cntr].base));
#else
        shell_print(shell_priv, "\tRing Write index = %d shadow = %d\n", write_idx, host_ring[cntr].shadow_write_idx);
        shell_print(shell_priv, "\tRing Read index = %d shadow = %d\n", read_idx, host_ring[cntr].shadow_read_idx);
#endif
        shell_print(shell_priv, "\tCurrently Queued = %d\n", rdp_cpu_ring_get_queued(cntr));

#ifdef XRDP
        shell_print(shell_priv, "\tLowest Filling Level = %d\n", host_ring[cntr].lowest_filling_level);
#endif /* XRDP */
#ifdef CONFIG_BCM_FEED_RING_DYNAMIC
        if (host_ring[cntr].type == rdpa_ring_feed)
        {
            const int alp = atomic_read(&allocated_packets);
            shell_print(shell_priv, "\tCurrent Allocations = %d\n", alp);
            shell_print(shell_priv, "\tMaximum Allocations = %d\n", max_allocations);
            shell_print(shell_priv, "\tAllocated %d out of %d checks\n", allocs_count, alloc_checks_count);
        }
#endif /* CONFIG_BCM_FEED_RING_DYNAMIC */
        shell_print(shell_priv, "-------------------------------\n" );
        shell_print(shell_priv, "\n\n" );
    }

    return 0;
}

/* XXX: Separate RDP/XRDP implementation, move XRDP to 6858 */
#ifdef XRDP
static void cpu_ring_print_phys_addr(void *shell_priv, uint32_t addr_hi, uint32_t addr_low)
{
   	bdmf_phys_addr_t phys_addr;
#ifdef PHYS_ADDR_64BIT
    phys_addr = ((bdmf_phys_addr_t)addr_hi << 32) | addr_low;
#else
    phys_addr = addr_low;
#endif
    shell_print(shell_priv, "\ttype: absolute address\n");
    shell_print(shell_priv, "\tpacket DDR phys low: 0x%x\n", addr_low);
    shell_print(shell_priv, "\tpacket DDR phys hi: 0x%x\n", addr_hi);
    shell_print(shell_priv, "\tpacket DDR phys: 0x%p\n", (uintptr_t *)phys_addr);
    shell_print(shell_priv, "\tpacket DDR virt uncached address: 0x%pK\n", (void *)PHYS_TO_UNCACHED(phys_addr));
}

static void cpu_ring_pd_print_fields(void *shell_priv, CPU_RX_DESCRIPTOR* pdPtr)
{
    shell_print(shell_priv, "descriptor fields:\n");
    if (pdPtr->abs.abs)
    {
        cpu_ring_print_phys_addr(shell_priv, pdPtr->abs.host_buffer_data_ptr_hi, pdPtr->abs.host_buffer_data_ptr_low);
        shell_print(shell_priv, "\tpacket len: %d\n", pdPtr->abs.packet_length);
    }
    else
    {
        shell_print(shell_priv, "\ttype: fpm\n");
        shell_print(shell_priv, "\tfpm_id: 0x%x\n", pdPtr->fpm.fpm_idx);
        shell_print(shell_priv, "\tpacket len: %d\n", pdPtr->fpm.packet_length);
    }

    if (pdPtr->cpu_vport.vport >= RDD_CPU_VPORT_FIRST && pdPtr->cpu_vport.vport <= RDD_CPU_VPORT_LAST)
    {
        shell_print(shell_priv, "\tsource: WLAN\n");
        shell_print(shell_priv, "\tdata offset: %d\n", pdPtr->cpu_vport.data_offset);
        shell_print(shell_priv, "\treason: %d\n", pdPtr->cpu_vport.reason);
        shell_print(shell_priv, "\tssid: %d\n", pdPtr->cpu_vport.ssid);
        shell_print(shell_priv, "\tvport: %d\n", pdPtr->cpu_vport.vport);

        shell_print(shell_priv, "\tis_rx_offload: %d\n", pdPtr->is_rx_offload);
        shell_print(shell_priv, "\tis_exception: %d\n", pdPtr->is_exception);
        shell_print(shell_priv, "\tis_ucast: %d\n", pdPtr->is_ucast);
        shell_print(shell_priv, "\ttx_prio: %d\n", pdPtr->tx_prio);
        shell_print(shell_priv, "\tcolor: %d\n", pdPtr->wan.color);
        shell_print(shell_priv, "\tdst_ssid_vector / metadata: 0x%x\n", pdPtr->dst_ssid_vector);
        shell_print(shell_priv, "\twl_metadata: 0x%x\n", pdPtr->wl_metadata);
    }
    else if (pdPtr->wan.is_src_lan)
    {
        shell_print(shell_priv, "\tsource: LAN\n");
        shell_print(shell_priv, "\tdata offset: %d\n", pdPtr->lan.data_offset);
        shell_print(shell_priv, "\treason: %d\n", pdPtr->lan.reason);
        shell_print(shell_priv, "\tsource port: %d\n", pdPtr->lan.source_port);
    }
    else
    {
        shell_print(shell_priv, "\tsource: WAN\n");
        shell_print(shell_priv, "\tdata offset: %d\n", pdPtr->wan.data_offset);
        shell_print(shell_priv, "\treason: %d\n", pdPtr->wan.reason);
        shell_print(shell_priv, "\tsource port: %d\n", pdPtr->wan.source_port);
        shell_print(shell_priv, "\tWAN flow id: %d\n", pdPtr->wan.wan_flow_id);
    }
}

static void cpu_feed_pd_print_fields(void *shell_priv, CPU_FEED_DESCRIPTOR* pdPtr)
{
    shell_print(shell_priv, "Feed descriptor fields:\n");
    cpu_ring_print_phys_addr(shell_priv, pdPtr->abs.host_buffer_data_ptr_hi, pdPtr->abs.host_buffer_data_ptr_low);

    shell_print(shell_priv, "\tpacket type: %d\n", pdPtr->abs.abs);
    shell_print(shell_priv, "\treserved: 0x%x\n", pdPtr->abs.reserved);
}

static void cpu_tx_pd_print_fields(void *shell_priv, RDD_RING_CPU_TX_DESCRIPTOR_DTS *pdPtr)
{
    shell_print(shell_priv, "Cpu TX descriptor fields:\n");

    shell_print(shell_priv, "\tis_egress %d\n", pdPtr->is_egress);
    shell_print(shell_priv, "\tfirst_level_q: %d\n", pdPtr->first_level_q);
    shell_print(shell_priv, "\tpacket_length: %d\n", pdPtr->packet_length);
    shell_print(shell_priv, "\tsk_buf_ptr address:\n");
    cpu_ring_print_phys_addr( shell_priv, pdPtr->sk_buf_ptr_high, pdPtr->sk_buf_ptr_low_or_data_1588);

    shell_print(shell_priv, "\tcolor: %d\n", pdPtr->color);
    shell_print(shell_priv, "\tdo_not_recycle: %d\n", pdPtr->do_not_recycle);
    shell_print(shell_priv, "\tflag_1588: %d\n", pdPtr->flag_1588);
    shell_print(shell_priv, "\tlan: %d\n", pdPtr->lan);
    shell_print(shell_priv, "\twan_flow_source_port: %d\n", pdPtr->wan_flow_source_port);
    shell_print(shell_priv, "\tfpm_fallback: %d\n", pdPtr->fpm_fallback);

    shell_print(shell_priv, "\tsbpm_copy: %d\n", pdPtr->sbpm_copy);
    shell_print(shell_priv, "\ttarget_mem_0: %d\n", pdPtr->target_mem_0);
    shell_print(shell_priv, "\tabs: %d\n", pdPtr->abs);
    shell_print(shell_priv, "\tlag_index: %d\n", pdPtr->lag_index);
    
    shell_print(shell_priv, "\tpkt_buf_ptr address\n");
    cpu_ring_print_phys_addr( shell_priv, pdPtr->pkt_buf_ptr_high, pdPtr->pkt_buf_ptr_low_or_fpm_bn0);
}

static void cpu_recycle_pd_print_fields(void *shell_priv, CPU_RECYCLE_DESCRIPTOR* pdPtr)
{
    shell_print(shell_priv, "Recycle descriptor fields:\n");
    cpu_ring_print_phys_addr(shell_priv, pdPtr->abs.host_buffer_data_ptr_hi, pdPtr->abs.host_buffer_data_ptr_low);

    shell_print(shell_priv, "\tpacket type: %d\n", pdPtr->abs.abs);
    shell_print(shell_priv, "\tfrom_feed_ring: 0x%x\n", pdPtr->abs.from_feed_ring);
    shell_print(shell_priv, "\treserved: 0x%x\n", pdPtr->abs.reserved);
}
#else

static void cpu_ring_pd_print_fields(void *shell_priv, CPU_RX_DESCRIPTOR* pdPtr)
{
    shell_print(shell_priv, "descriptor fields:\n");
    shell_print(shell_priv, "\townership: %s\n", pdPtr->ownership ? "Host":"Runner");
    shell_print(shell_priv, "\tpacket DDR uncached address: 0x%pK\n", (void*)PHYS_TO_UNCACHED(pdPtr->host_buffer_data_pointer));
    shell_print(shell_priv, "\tpacket len: %d\n", pdPtr->packet_length);

    if(pdPtr->ownership != OWNERSHIP_HOST)
        return;

    shell_print(shell_priv, "\tdescriptor type: %d\n", pdPtr->descriptor_type);
    shell_print(shell_priv, "\tsource port: %d\n", pdPtr->source_port );
    shell_print(shell_priv, "\tflow_id: %d\n", pdPtr->flow_id);
    shell_print(shell_priv, "\twl chain id: %d\n", pdPtr->wl_metadata & 0xFF);
    shell_print(shell_priv, "\twl priority: %d\n", (pdPtr->wl_metadata & 0xFF00) >> 8);
}
#endif

int cpu_ring_shell_print_pd(void *shell_priv, uint32_t ring_id, uint32_t pdIndex)
{
#ifdef XRDP
    if (host_ring[ring_id].type == rdpa_ring_data)
    {
#endif
        CPU_RX_DESCRIPTOR host_ring_desc;

#ifndef XRDP
        memcpy(&host_ring_desc, (CPU_RX_DESCRIPTOR *)host_ring[ring_id].base + pdIndex, sizeof(CPU_RX_DESCRIPTOR));
#else
        memcpy(&host_ring_desc, &((CPU_RX_DESCRIPTOR *)host_ring[ring_id].base)[pdIndex],
            sizeof(CPU_RX_DESCRIPTOR));
#endif

        shell_print(shell_priv, "descriptor unswapped: %08x %08x %08x %08x\n",
            host_ring_desc.word0, host_ring_desc.word1, host_ring_desc.word2,
            host_ring_desc.word3 );

        host_ring_desc.word0 = swap4bytes(host_ring_desc.word0);
        host_ring_desc.word1 = swap4bytes(host_ring_desc.word1);
        host_ring_desc.word2 = swap4bytes(host_ring_desc.word2);
        host_ring_desc.word3 = swap4bytes(host_ring_desc.word3);

        shell_print(shell_priv, "descriptor swapped  : %08x %08x %08x %08x\n", 
            host_ring_desc.word0, host_ring_desc.word1, host_ring_desc.word2,
            host_ring_desc.word3 );

        cpu_ring_pd_print_fields(shell_priv, &host_ring_desc);
#ifdef XRDP 
    }
    else if (host_ring[ring_id].type == rdpa_ring_recycle)
    {
        CPU_RECYCLE_DESCRIPTOR host_rc_desc;

        memcpy(&host_rc_desc, &((CPU_FEED_DESCRIPTOR *)host_ring[ring_id].base)[pdIndex],
            sizeof(CPU_FEED_DESCRIPTOR));

        shell_print(shell_priv, "feed descriptor unswapped: %08x %08x\n",
            host_rc_desc.word0, host_rc_desc.word1 );

        host_rc_desc.word0 = swap4bytes(host_rc_desc.word0);
        host_rc_desc.word1 = swap4bytes(host_rc_desc.word1);

        shell_print(shell_priv, "descriptor swapped  : %08x %08x\n", 
            host_rc_desc.word0, host_rc_desc.word1 );

        cpu_recycle_pd_print_fields(shell_priv, &host_rc_desc);
    }
    else if (host_ring[ring_id].type == rdpa_ring_feed )
    {
        CPU_FEED_DESCRIPTOR host_feed_desc;

        memcpy(&host_feed_desc, &((CPU_FEED_DESCRIPTOR *)host_ring[ring_id].base)[pdIndex],
            sizeof(CPU_FEED_DESCRIPTOR));
        shell_print(shell_priv, "feed descriptor unswapped: %08x %08x\n",
            host_feed_desc.word0, host_feed_desc.word1 );

        host_feed_desc.word0 = swap4bytes(host_feed_desc.word0);
        host_feed_desc.word1 = swap4bytes(host_feed_desc.word1);

        shell_print(shell_priv, "descriptor swapped  : %08x %08x\n", 
            host_feed_desc.word0, host_feed_desc.word1 );

        cpu_feed_pd_print_fields(shell_priv, &host_feed_desc);
    }
    else if (host_ring[ring_id].type == rdpa_ring_cpu_tx)
    {
        RDD_RING_CPU_TX_DESCRIPTOR_DTS cpu_tx_desc = {};
        union
        {
            uint32_t words[4];
            RDD_RING_CPU_TX_DESCRIPTOR_DTS cpu_tx_swapped;
        } swapped;

        shell_print(shell_priv, "host_ring[%d].base %p, size %d\n", ring_id, host_ring[ring_id].base, (int)sizeof(RDD_RING_CPU_TX_DESCRIPTOR_DTS));
        shell_print(shell_priv, "hostpd[%d].address %p\n", pdIndex, &((RDD_RING_CPU_TX_DESCRIPTOR_DTS *)host_ring[ring_id].base)[pdIndex]);
        memcpy(&cpu_tx_desc, &((RDD_RING_CPU_TX_DESCRIPTOR_DTS *)host_ring[ring_id].base)[pdIndex],
            sizeof(RDD_RING_CPU_TX_DESCRIPTOR_DTS));
        shell_print(shell_priv, "CPU TX descriptor unswapped: %08x %08x %08x %08x\n",
            ((uint32_t *)&cpu_tx_desc)[0], ((uint32_t *)&cpu_tx_desc)[1], ((uint32_t *)&cpu_tx_desc)[2], ((uint32_t *)&cpu_tx_desc)[3]);

        swapped.words[0] = swap4bytes(((uint32_t *)&cpu_tx_desc)[0]);
        swapped.words[1] = swap4bytes(((uint32_t *)&cpu_tx_desc)[1]);
        swapped.words[2] = swap4bytes(((uint32_t *)&cpu_tx_desc)[2]);
        swapped.words[3] = swap4bytes(((uint32_t *)&cpu_tx_desc)[3]);

        shell_print(shell_priv, "descriptor swapped  : %08x %08x %08x %08x\n", 
            swapped.words[0], swapped.words[1], swapped.words[2], swapped.words[3]);

        cpu_tx_pd_print_fields(shell_priv, &swapped.cpu_tx_swapped);
    }
#endif
    return 0;
}

/*bdmf shell is compiling only for RDPA and not CFE*/
#ifndef _CFE_
#define MAKE_BDMF_SHELL_CMD_NOPARM(dir, cmd, help, cb) \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, NULL)

#define MAKE_BDMF_SHELL_CMD(dir, cmd, help, cb, parms...)   \
{                                                           \
    static bdmfmon_cmd_parm_t cmd_parms[]={                 \
        parms,                                              \
        BDMFMON_PARM_LIST_TERMINATOR                        \
    };                                                      \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, cmd_parms); \
}

static int bdmf_cpu_ring_shell_list_rings( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
    int start_from = -1;

    if (n_parms == 1)
        start_from = (uint32_t)parm[0].value.unumber;

    return cpu_ring_shell_list_rings(session, start_from);
}

static int bdmf_cpu_ring_shell_print_pd(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    return cpu_ring_shell_print_pd(session, (uint32_t)parm[0].value.unumber, (uint32_t)parm[1].value.unumber);
}

#define MAKE_BDMF_SHELL_CMD_NOPARM(dir, cmd, help, cb) \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, NULL)

#define MAKE_BDMF_SHELL_CMD(dir, cmd, help, cb, parms...)   \
{                                                           \
    static bdmfmon_cmd_parm_t cmd_parms[]={                 \
        parms,                                              \
        BDMFMON_PARM_LIST_TERMINATOR                        \
    };                                                      \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, cmd_parms); \
}
int ring_make_shell_commands ( void )
{
    bdmfmon_handle_t driver_dir, cpu_dir;

    if ( !( driver_dir = bdmfmon_dir_find ( NULL, "driver" ) ) )
    {
        driver_dir = bdmfmon_dir_add ( NULL, "driver", "Device Drivers", BDMF_ACCESS_ADMIN, NULL );

        if ( !driver_dir )
            return ( 1 );
    }

    cpu_dir = bdmfmon_dir_add ( driver_dir, "cpur", "CPU Ring Interface Driver", BDMF_ACCESS_ADMIN, NULL );

    if ( !cpu_dir )
        return ( 1 );


    MAKE_BDMF_SHELL_CMD( cpu_dir, "sar",   "Show available rings", bdmf_cpu_ring_shell_list_rings,
        BDMFMON_MAKE_PARM_RANGE( "ring_id", "ring id", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL, 0, D_NUM_OF_RING_DESCRIPTORS) );

    MAKE_BDMF_SHELL_CMD( cpu_dir, "vrpd",     "View Ring packet descriptor", bdmf_cpu_ring_shell_print_pd,
        BDMFMON_MAKE_PARM_RANGE( "ring_id", "ring id", BDMFMON_PARM_NUMBER, 0, 0, D_NUM_OF_RING_DESCRIPTORS ),
#if defined(XRDP)
        BDMFMON_MAKE_PARM_RANGE( "descriptor", "packet descriptor index ", BDMFMON_PARM_NUMBER, 0, 0, FEED_RING_SIZE) );
#else
        BDMFMON_MAKE_PARM_RANGE( "descriptor", "packet descriptor index ", BDMFMON_PARM_NUMBER, 0, 0, RDPA_CPU_QUEUE_MAX_SIZE) );
#endif

    return 0;
}
#endif /* !_CFE_ */

/*delete a preallocated ring*/
int	rdp_cpu_ring_delete_ring(uint32_t ring_id)
{
    RING_DESCTIPTOR*			pDescriptor;
    int rc;

    pDescriptor = &host_ring[ring_id];
    if(!pDescriptor->num_of_entries)
    {
        INTERN_PRINT("ERROR:deleting ring_id %d which does not exists!",ring_id);
        return -1;
    }

    rc = rdp_cpu_ring_buffers_free(pDescriptor);
    if (rc)
    {
        INTERN_PRINT("ERROR: failed free ring buffers ring_id %d, err %d\n", ring_id, rc);
        return rc;
    }

    /* free any buffers in buff_cache */
    while(pDescriptor->buff_cache_cnt)
    {
        pDescriptor->databuf_free(pDescriptor->buff_cache[--pDescriptor->buff_cache_cnt], 0, pDescriptor);
    }

    /*free buff_cache */
    if(pDescriptor->buff_cache)
        CACHED_FREE(pDescriptor->buff_cache);

    /*delete the ring of descriptors in case of non-coherent*/
#ifndef RDP_SIM
    if (pDescriptor->base)
    {
        rdp_mm_aligned_free((void*)NONCACHE_TO_CACHE(pDescriptor->base),
            pDescriptor->num_of_entries * pDescriptor->size_of_entry);
    }
#endif
    pDescriptor->num_of_entries = 0;

    return 0;
}
EXPORT_SYMBOL(rdp_cpu_ring_delete_ring);

int	rdp_cpu_ring_create_ring(uint32_t ring_id,
    uint8_t ring_type,
    uint32_t entries,
    bdmf_phys_addr_t* ring_head,
    uint32_t packetSize,
    RING_CB_FUNC* ringCb,
    uint32_t ring_prio)
{
    return rdp_cpu_ring_create_ring_ex(ring_id, ring_type, entries, ring_head, NULL, packetSize, ringCb, ring_prio);
}
EXPORT_SYMBOL(rdp_cpu_ring_create_ring);
#if defined(XRDP)
static inline void __rdp_prepare_ptr_address(void *idx_address, uint32_t *addr_l, uint32_t *addr_h)
{
    uintptr_t phy_addr, address;
    address = (uintptr_t)idx_address;

    phy_addr = RDD_VIRT_TO_PHYS(address); 

    GET_ADDR_HIGH_LOW((*addr_h), (*addr_l), phy_addr);
}
#endif 

/* Using void * instead of (rdpa_cpu_rxq_ic_cfg_t *) to avoid CFE compile errors*/
int	rdp_cpu_ring_create_ring_ex(uint32_t ring_id,
    uint8_t ring_type,
    uint32_t entries,
    bdmf_phys_addr_t* ring_head,
    bdmf_phys_addr_t* rw_idx_addr,
    uint32_t packetSize,
    RING_CB_FUNC* ringCb,
    uint32_t ring_prio)
{
    RING_DESCTIPTOR* pDescriptor;
    bdmf_phys_addr_t phy_addr = 0;

#if defined(XRDP) && !defined(_CFE_)
    if (ring_id >= RING_ID_NUM_OF || (ring_type == rdpa_ring_data && ring_id > DATA_RING_ID_LAST))
#else
    if (ring_id >= RING_ID_NUM_OF)
#endif
    {
        INTERN_PRINT("ERROR: ring_id %d out of range(%d)", ring_id, RING_ID_NUM_OF);
        return -1;
    }

#ifdef XRDP
    if (ring_type == rdpa_ring_feed)
        pDescriptor = &host_ring[FEED_RING_ID];
    else
#endif
        pDescriptor = &host_ring[ring_id];

    if(pDescriptor->num_of_entries)
    {
        INTERN_PRINT("ERROR: ring_id %d already exists! must be deleted first",ring_id);
        return -1;
    }

    if(!entries)
    {
        INTERN_PRINT("ERROR: can't create ring with 0 packets\n");
        return -1;
    }

    /*set ring parameters*/
    pDescriptor->ring_id = ring_id;
    pDescriptor->num_of_entries = entries;
    pDescriptor->num_of_entries_mask = pDescriptor->num_of_entries - 1;
    pDescriptor->ring_prio = ring_prio;

#ifdef XRDP
    if (ring_type == rdpa_ring_feed)
        pDescriptor->size_of_entry		= sizeof(CPU_FEED_DESCRIPTOR);
    else if (ring_type == rdpa_ring_recycle)
        pDescriptor->size_of_entry		= sizeof(CPU_RECYCLE_DESCRIPTOR);
    else if (ring_type == rdpa_ring_cpu_tx)
        pDescriptor->size_of_entry		= sizeof(RDD_RING_CPU_TX_DESCRIPTOR_DTS);
    else if (ring_type == rdpa_ring_data)
#endif
        pDescriptor->size_of_entry		= sizeof(CPU_RX_DESCRIPTOR);

    INTERN_PRINT("Creating CPU ring for queue number %d with %d packets descriptor=0x%p, size_of_entry %d\n",
        ring_id, entries, pDescriptor, pDescriptor->size_of_entry);

    pDescriptor->buff_cache_cnt = 0;
    pDescriptor->packet_size = packetSize;
    pDescriptor->type = ring_type;

    pDescriptor->databuf_alloc  = rdp_databuf_alloc;
    pDescriptor->databuf_free   = rdp_databuf_free;
    pDescriptor->data_dump = rdp_packet_dump;
    
    if (ringCb) /* overwrite if needed */
    {
        pDescriptor->data_dump = ringCb->data_dump;
        pDescriptor->buff_mem_context = ringCb->buff_mem_context;
#ifndef XRDP
        pDescriptor->databuf_alloc  = ringCb->databuf_alloc;
        pDescriptor->databuf_free   = ringCb->databuf_free;
#endif
    }

    /*TODO:update the comment  allocate buff_cache which helps to reduce the overhead of when 
     * allocating data buffers to ring descriptor */
    pDescriptor->buff_cache = (uint8_t **)(CACHED_MALLOC_ATOMIC(sizeof(uint8_t *) * MAX_BUFS_IN_CACHE));
    if( pDescriptor->buff_cache == NULL )
    {
        INTERN_PRINT("failed to allocate memory for cache of data buffers \n");
        return -1;
    }

    /*allocate ring descriptors - must be non-cacheable memory*/
#if defined(XRDP) && !defined(_CFE_)
    if (ring_type == rdpa_ring_recycle)
    {
        pDescriptor->base =  rdp_mm_aligned_alloc((pDescriptor->size_of_entry * entries) + RW_INDEX_SIZE*2, &phy_addr);
        pDescriptor->read_idx = (uint16_t *)((uintptr_t)pDescriptor->base + (pDescriptor->size_of_entry * entries));
        pDescriptor->write_idx = (uint16_t *)((uintptr_t)pDescriptor->read_idx + RW_INDEX_SIZE);
        *pDescriptor->read_idx = 0;
        *pDescriptor->write_idx = 0;
        if (rw_idx_addr)
            *rw_idx_addr = phy_addr + (pDescriptor->size_of_entry * entries);
  
    }
    else
#endif  /* for rdpa_ring_cpu_tx rings read write indices will be allocated later in separate buffer*/
        pDescriptor->base =  rdp_mm_aligned_alloc((pDescriptor->size_of_entry * entries), &phy_addr);

    if( pDescriptor->base == NULL)
    {
        INTERN_PRINT("failed to allocate memory for ring descriptor\n");
        rdp_cpu_ring_delete_ring(ring_id);
        return -1;
    }

    if (rdp_cpu_ring_buffers_init(pDescriptor, ring_id))
        return -1;

#ifdef XRDP
    /*feed ring lowest filling level is the lowest number of entries seen during ring lifetime.*/
    pDescriptor->lowest_filling_level = (uint16_t)entries;
#endif /* XRDP */

#ifndef XRDP
    /*set the ring header to the first entry*/
    pDescriptor->head = (CPU_RX_DESCRIPTOR *)pDescriptor->base;

    /*using pointer arithmetics calculate the end of the ring*/
    pDescriptor->end = (CPU_RX_DESCRIPTOR *)pDescriptor->base + entries;
#endif

    *ring_head = phy_addr;

#ifndef XRDP
    INTERN_PRINT("Done initializing Ring %d Base=0x%pK End=0x%pK "
        "calculated entries= %ld RDD Base=%lxK descriptor=0x%p\n", ring_id,
        pDescriptor->base, pDescriptor->end, (long)(pDescriptor->end - (CPU_RX_DESCRIPTOR *)pDescriptor->base), 
        (unsigned long)phy_addr, pDescriptor);
#else
    INTERN_PRINT("Done initializing Ring %d Base=0x%p num of entries= %d RDD Base=%lx descriptor=0x%p\n",
        ring_id, pDescriptor->base, pDescriptor->num_of_entries, (unsigned long)phy_addr, pDescriptor);
#endif

#ifndef _CFE_
    {
        if(!init_shell)
        {
            if(ring_make_shell_commands())
            {	
                INTERN_PRINT("Failed to create ring bdmf shell commands\n");
                return 1;
            }

            init_shell = 1;
        }
    }
#endif

    return 0;
}
EXPORT_SYMBOL(rdp_cpu_ring_create_ring_ex);

#ifdef XRDP
/**********************************************************************
 *    rdp_cpu_tx_rings_indices_alloc
 *      allocate read write indices for rings  TX_HIGH_PRIO_RING_ID,
 *      TX_LOW_PRIO_RING_ID,
 *      also update theire addreces in runner firmware.
 *      allocated buffer structure:
 *      address[0]  TX_HIGH_PRIO_RING_ID -> read_idx
 *      address[1]  TX_HIGH_PRIO_RING_ID -> write_idx
 *      address[2]  TX_LOW_PRIO_RING_ID -> read_idx
 *      address[3]  TX_LOW_PRIO_RING_ID -> write_idx
 *
 **********************************************************************/
void rdp_cpu_tx_rings_indices_alloc(void)
{
    RING_DESCTIPTOR *pDescriptor_low = &host_ring[TX_LOW_PRIO_RING_ID];
    RING_DESCTIPTOR *pDescriptor_high = &host_ring[TX_HIGH_PRIO_RING_ID];
    uint16_t *buffer_ptr;
    bdmf_phys_addr_t phy_addr = 0;
    uint32_t addr_l, addr_h;

    buffer_ptr = rdp_mm_aligned_alloc((sizeof(uint16_t) * 4), &phy_addr);
    pDescriptor_high->read_idx = buffer_ptr;
    pDescriptor_high->write_idx = &buffer_ptr[1];
    pDescriptor_low->read_idx = &buffer_ptr[2];
    pDescriptor_low->write_idx = &buffer_ptr[3];

    /* indices initialization */
    *pDescriptor_high->read_idx = 0;
    *pDescriptor_high->write_idx = 0;
    *pDescriptor_low->read_idx = 0;
    *pDescriptor_low->write_idx = 0;

    /* update runner with indeces buffer address*/
    __rdp_prepare_ptr_address((void *)(buffer_ptr), &addr_l, &addr_h);

    RDD_CPU_TX_RING_INDICES_READ_IDX_WRITE_G(0, RDD_CPU_TX_RING_INDICES_VALUES_TABLE_ADDRESS_ARR, 0);
    RDD_CPU_TX_RING_INDICES_WRITE_IDX_WRITE_G(0, RDD_CPU_TX_RING_INDICES_VALUES_TABLE_ADDRESS_ARR, 0);
    RDD_CPU_TX_RING_INDICES_READ_IDX_WRITE_G(0, RDD_CPU_TX_RING_INDICES_VALUES_TABLE_ADDRESS_ARR, 1);
    RDD_CPU_TX_RING_INDICES_WRITE_IDX_WRITE_G(0, RDD_CPU_TX_RING_INDICES_VALUES_TABLE_ADDRESS_ARR, 1);

    /* initiate SYNC_FIFO_TABLE both write and read pointers should point FIFO head*/
    RDD_CPU_TX_SYNC_FIFO_ENTRY_WRITE_PTR_WRITE_G(SYNC_FIFO_ADDRESS, RDD_CPU_TX_SYNC_FIFO_TABLE_ADDRESS_ARR, 0);
    RDD_CPU_TX_SYNC_FIFO_ENTRY_READ_PTR_WRITE_G(SYNC_FIFO_ADDRESS, RDD_CPU_TX_SYNC_FIFO_TABLE_ADDRESS_ARR, 0);
    RDD_CPU_TX_SYNC_FIFO_ENTRY_WRITE_PTR_WRITE_G((SYNC_FIFO_ADDRESS + CPU_TX_SYNC_FIFO_ENTRY_BYTE_SIZE),
        RDD_CPU_TX_SYNC_FIFO_TABLE_ADDRESS_ARR, 1);
    RDD_CPU_TX_SYNC_FIFO_ENTRY_READ_PTR_WRITE_G((SYNC_FIFO_ADDRESS + CPU_TX_SYNC_FIFO_ENTRY_BYTE_SIZE),
        RDD_CPU_TX_SYNC_FIFO_TABLE_ADDRESS_ARR, 1);
}
EXPORT_SYMBOL(rdp_cpu_tx_rings_indices_alloc);
#endif

void rdp_cpu_ring_free_mem(uint32_t ringId, void *pBuf)
{
    RING_DESCTIPTOR *pDescriptor     = &host_ring[ringId];

    if ((pDescriptor == NULL) || (pDescriptor->databuf_free == NULL))
    {
        INTERN_PRINT("rdp_cpu_ring_free_mem: pDescriptor or free_cb is NULL, Memory is not freed.\n");
        return;
    }

    pDescriptor->databuf_free(pBuf, 0, pDescriptor);
}
EXPORT_SYMBOL(rdp_cpu_ring_free_mem);

#ifndef _CFE_
#ifdef XRDP
rdpa_if rdpa_port_vport_to_rdpa_if(rdd_vport_id_t rdd_vport);
#endif
inline int rdp_cpu_ring_get_packet(uint32_t ringId, rdpa_cpu_rx_info_t *info)
{
    int rc;
    CPU_RX_PARAMS params;
#ifndef XRDP
    RING_DESCTIPTOR *pDescriptor     = &host_ring[ringId];
    rdpa_traffic_dir dir;
#endif

    memset((void *)&params, 0, sizeof(CPU_RX_PARAMS));

    rc = rdp_cpu_ring_read_packet_refill(ringId, &params);
    if (rc)
    {
#ifdef LEGACY_RDP
        if (rc == BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_EMPTY)
            return BDMF_ERR_NO_MORE;
#else
        if (rc == BDMF_ERR_NO_MORE)
            return rc;
#endif
        return BDMF_ERR_INTERNAL;
    }

#ifndef XRDP
    info->src_port = rdpa_cpu_rx_srcport_to_rdpa_if(params.src_bridge_port,
        params.flow_id);
    if (info->src_port == rdpa_if_none)
    {
        DO_DEBUG(pDescriptor->stats_dropped++);

        pDescriptor->databuf_free(params.data_ptr, 0, pDescriptor);

        return BDMF_ERR_PERM;
    }
#else
    info->vport = params.src_bridge_port;
    info->src_port = rdpa_port_vport_to_rdpa_if(params.src_bridge_port);
#endif
    info->dest_ssid = params.dst_ssid;

    info->reason = (rdpa_cpu_reason)params.reason;
    info->reason_data = params.flow_id;
    info->ptp_index = params.ptp_index;
    info->data = (void*)params.data_ptr;
#ifdef XRDP
    info->data_offset = params.data_offset;
#endif
    info->size = params.packet_size;
#ifndef XRDP
    dir = rdpa_if_is_wan(info->src_port) ? rdpa_dir_ds : rdpa_dir_us;
    stats_reason[dir][info->reason]++;
    pDescriptor->stats_received++;
#endif

#ifdef CPU_RING_DEBUG    
    if (unlikely(pDescriptor->dump_enable))
        pDescriptor->data_dump(pDescriptor->ring_id, info);
#endif

    return rc;
}
EXPORT_SYMBOL(rdp_cpu_ring_get_packet);

#ifdef CPU_RING_DEBUG
void rdp_cpu_dump_data_cb(bdmf_index queue, bdmf_boolean enabled)
{
    host_ring[queue].dump_enable = enabled;
}
EXPORT_SYMBOL(rdp_cpu_dump_data_cb);
#endif

#ifndef XRDP
void rdp_cpu_rxq_stat_cb(int qid, extern_rxq_stat_t *stat, bdmf_boolean clear)
{
    RING_DESCTIPTOR *pDescriptor     = &host_ring[qid];

    if (!stat)
        return;

    stat->received = pDescriptor->stats_received;
    stat->dropped = pDescriptor->stats_dropped;
    stat->queued  = rdp_cpu_ring_get_queued(qid);

    if (clear)
        pDescriptor->stats_received = pDescriptor->stats_dropped = 0;
}
EXPORT_SYMBOL(rdp_cpu_rxq_stat_cb);

void rdp_cpu_reason_stat_cb(uint32_t *stat, rdpa_cpu_reason_index_t *rindex)
{
    if ((!stat) || (!rindex))
        return;

    *stat = stats_reason[rindex->dir][rindex->reason];
    stats_reason[rindex->dir][rindex->reason] = 0;
}
EXPORT_SYMBOL(rdp_cpu_reason_stat_cb);
#endif /* XRDP */

#endif /* (!_CFE_) */

#if defined(_CFE_) || !(defined(CONFIG_BCM963138) || defined(_BCM963138_) || defined(CONFIG_BCM963148) || defined(_BCM963148_))
/*this API copies the next available packet from ring to given pointer*/
int rdp_cpu_ring_read_packet_copy( uint32_t ring_id, CPU_RX_PARAMS* rxParams)
{
    RING_DESCTIPTOR* 					pDescriptor		= &host_ring[ ring_id ];
#ifndef XRDP
    volatile CPU_RX_DESCRIPTOR*			pTravel = (volatile CPU_RX_DESCRIPTOR*)pDescriptor->head;
#endif
    void* 	 						    client_pdata;
    uint32_t 							ret = 0;

    /* Data offset field is field ONLY in CFE driver on BCM6858 
     * To ensure correct work of another platforms the data offset field should be zeroed */
    rxParams->data_offset = 0;

    client_pdata 		= (void*)rxParams->data_ptr;

#ifndef XRDP
    ret = ReadPacketFromRing(pDescriptor,pTravel, rxParams);
#else
    ret = ReadPacketFromRing(pDescriptor, rxParams);
#endif
    if ( ret )
        goto exit;

    /*copy the data to user buffer*/
    /*TODO: investigate why INV_RANGE is needed before memcpy,*/  
    INV_RANGE((rxParams->data_ptr + rxParams->data_offset), rxParams->packet_size);
    memcpy(client_pdata,(void*)(rxParams->data_ptr + rxParams->data_offset), rxParams->packet_size);

    /*Assign the data buffer back to ring*/
    INV_RANGE((rxParams->data_ptr + rxParams->data_offset), rxParams->packet_size);
#ifndef XRDP
    AssignPacketBuffertoRing(pDescriptor, pTravel, rxParams->data_ptr);
#else
    AssignPacketBuffertoRing(pDescriptor, rxParams->data_ptr);
#endif

exit:
    rxParams->data_ptr = client_pdata;
    return ret;
}
#endif

/*this function if for debug purposes*/
int	rdp_cpu_ring_get_queue_size(uint32_t ring_idx)
{
    return host_ring[ ring_idx ].num_of_entries;
}

#ifdef XRDP
static uint32_t packets_count(RING_DESCTIPTOR *rd, uint16_t read_idx, uint16_t write_idx)
{
    uint32_t packets;
    if (read_idx <= write_idx)
        packets = write_idx - read_idx;
    else
        packets = (rd->num_of_entries - read_idx) + write_idx;
    return packets;
}
#endif /* XRDP */

/*this function if for debug purposes and should not be called during runtime*/
/*TODO:Add mutex to protect when reading while packets read from another context*/
int	rdp_cpu_ring_get_queued(uint32_t ring_idx)
{
    RING_DESCTIPTOR*			pDescriptor = &host_ring[ ring_idx ];
    uint32_t                    packets     = 0;
#ifndef XRDP
    volatile CPU_RX_DESCRIPTOR*	pTravel		= pDescriptor->base;
    volatile CPU_RX_DESCRIPTOR*	pEnd		= pDescriptor->end;
#else
    uint16_t read_idx = 0, write_idx = 0;
    rdp_cpu_get_read_idx(pDescriptor->ring_id, pDescriptor->type, &read_idx);
    rdp_cpu_get_write_idx(pDescriptor->ring_id, pDescriptor->type, &write_idx);
#endif

    if(pDescriptor->num_of_entries == 0)
        return 0;
#ifndef XRDP
    while (pTravel != pEnd)
    {
        if (rdp_cpu_ring_is_ownership_host(pTravel))
            packets++;
        pTravel++;
    }
#else
    packets = packets_count(pDescriptor, read_idx, write_idx);
#endif

    return packets;
}
 
#ifdef XRDP
static inline void update_lowest_filling_level(void)
{
    const uint16_t pkts = (uint16_t) rdp_cpu_ring_get_queued(FEED_RING_ID);
    RING_DESCTIPTOR* pDescriptor = &host_ring[FEED_RING_ID];

    if (pDescriptor->lowest_filling_level > pkts)
        pDescriptor->lowest_filling_level = pkts;
}
#endif /* XRDP */

int	rdp_cpu_ring_flush(uint32_t ring_id)
{
    RING_DESCTIPTOR*			pDescriptor = &host_ring[ ring_id ];
#ifndef XRDP
    volatile CPU_RX_DESCRIPTOR*	pTravel		= pDescriptor->base;
    volatile CPU_RX_DESCRIPTOR*	pEnd		= pDescriptor->end;


    while (pTravel != pEnd)
    {
        rdp_cpu_ring_set_ownership_runner(pTravel);
        pTravel++;
    }

    pDescriptor->head = (CPU_RX_DESCRIPTOR *)pDescriptor->base;
#else
    int rc;

    if (host_ring[ring_id].type != rdpa_ring_feed)
    {
        rc = rdp_cpu_ring_buffers_free(pDescriptor);
        if (rc)
        {
            INTERN_PRINT("ERROR: failed free ring buffers ring_id %d, err %d\n", ring_id, rc);
            return rc;
        }
    }
#endif
    INTERN_PRINT("cpu Ring %d has been flushed\n",ring_id);

    return 0;
}

int	rdp_cpu_ring_not_empty(uint32_t ring_id)
{
    RING_DESCTIPTOR*			pDescriptor = &host_ring[ ring_id ];
#ifndef XRDP
    CPU_RX_DESCRIPTOR*	pTravel		= (pDescriptor->head );

    return rdp_cpu_ring_is_ownership_host(pTravel);
#else
    uint32_t read_idx =0, write_idx = 0;

    read_idx = pDescriptor->shadow_read_idx;
    write_idx = pDescriptor->shadow_write_idx;
    /* igor :todo check this by unitest if code ok*/

    return read_idx != write_idx ? 1 : 0;
#endif
}

int rdp_cpu_ring_is_full(uint32_t ring_id)
{
#ifdef XRDP
    RING_DESCTIPTOR*          pDescriptor = &host_ring[ ring_id ];

    return (pDescriptor->num_of_entries - rdp_cpu_ring_get_queued(ring_id) < 10);

#else
    INTERN_PRINT("%s NOT IMPLEMENTED \n",__FUNCTION__);
    return 0;
#endif
}
#ifndef _CFE_
/*this API get the pointer of the next available packet and reallocate buffer in ring
 * in the descriptor is optimized to 16 bytes cache line, 6838 has 16 bytes cache line
 * while 68500 has 32 bytes cache line, so we don't prefetch the descriptor to cache
 * Also on ARM platform we are not sure of how to skip L2 cache, and use only L1 cache
 * so for now  always use uncached accesses to Packet Descriptor(pTravel)
 */

#ifdef XRDP
inline int rdp_cpu_ring_read_packet_refill(uint32_t ring_id, CPU_RX_PARAMS *rxParams)
{
    uint32_t ret;
    RING_DESCTIPTOR *pDescriptor = &host_ring[ring_id];

    ret = ReadPacketFromRing(pDescriptor, rxParams);
    if (ret)
        return ret;

#ifndef CONFIG_BCM_CACHE_COHERENCY
    bdmf_dcache_inv((unsigned long)(rxParams->data_ptr + rxParams->data_offset), rxParams->packet_size);
#endif

    return ret;
}

inline int rdp_cpu_ring_read_bulk(uint32_t ring_id, CPU_RX_PARAMS *rxParams, int max_count, int *count)
{
    uint32_t ret;
    RING_DESCTIPTOR *ring = &host_ring[ring_id];
    int read_count = 0;

    do 
    {
        if ((ret = ReadPacketFromRing(ring, rxParams)))
            goto Exit;

        bdmf_dcache_inv((unsigned long)(rxParams->data_ptr + rxParams->data_offset), rxParams->packet_size);
        rxParams++;
        read_count++;
    } while (read_count < max_count && read_count < CPU_RX_PACKETS_BULK_SIZE);

Exit:
    *count = read_count;
    return ret;
}
#else
inline int rdp_cpu_ring_read_packet_refill(uint32_t ring_id, CPU_RX_PARAMS *rxParams)
{
    uint32_t                     ret;
    RING_DESCTIPTOR *pDescriptor     = &host_ring[ring_id];
    volatile CPU_RX_DESCRIPTOR* pTravel = (volatile CPU_RX_DESCRIPTOR*)pDescriptor->head;

    void *pNewBuf = NULL;

    ret = ReadPacketFromRing(pDescriptor, pTravel, rxParams);
    if (ret)
    {
        return  ret;
    }

    /* A valid packet is recieved try to allocate a new data buffer and
     * refill the ring before giving the packet to upper layers
     */

    pNewBuf  = pDescriptor->databuf_alloc(pDescriptor);

    /*validate allocation*/
    if (unlikely(!pNewBuf))
    {
        //printk("ERROR:system buffer allocation failed!\n");
        /*assign old data buffer back to ring*/
        pNewBuf   = rxParams->data_ptr;
        rxParams->data_ptr = NULL;
        ret = 1;
    }

    AssignPacketBuffertoRing(pDescriptor, pTravel, pNewBuf);
    return ret;
}
#endif /* XRDP */

#ifdef XRDP
/* interrupt routine Recycle ring*/
static inline int _rdp_cpu_ring_recycle_free_host_buf(RING_DESCTIPTOR *ring_descr)
{
    volatile CPU_RECYCLE_DESCRIPTOR * cpu_recycle_descr;
    CPU_RECYCLE_DESCRIPTOR rx_desc;
    uintptr_t phys_ptr;
    void* data_ptr;
    uint32_t read_idx = ring_descr->shadow_read_idx;
    uint32_t write_idx =ring_descr->shadow_write_idx;
#if !defined(RDP_SIM) && defined(PHYS_ADDR_64BIT)
    register uint64_t dword0 asm ("x9");
    register uint64_t dword1 asm ("x10");
#endif

    if (unlikely(read_idx == write_idx))
    {
            return BDMF_ERR_NO_MORE;
    }

    cpu_recycle_descr = &((CPU_RECYCLE_DESCRIPTOR *)ring_descr->base)[read_idx];

#ifdef CONFIG_BCM_CACHE_COHERENCY
    /*Before accessing the descriptors must do barrier */
    dma_rmb();
#endif

#ifdef CONFIG_ARM64
#ifndef RDP_SIM
    __asm__("ldp   %1, %2,[%0]" \
         :  "=r" (cpu_recycle_descr), "=r" (dword0), "=r" (dword1) \
         : "0" (cpu_recycle_descr));

    /* Swap descriptor */
    *((uint64_t*)&rx_desc.word0) = swap4bytes64(dword0);
#else
    *((uint64_t*)&rx_desc.word0) = swap4bytes64(*((uint64_t*)&cpu_recycle_descr->word0));
#endif
#else
    rx_desc.word0 = swap4bytes(cpu_recycle_descr->word0);
    rx_desc.word1 = swap4bytes(cpu_recycle_descr->word1);
#endif

#ifdef PHYS_ADDR_64BIT
    phys_ptr = ((uintptr_t)rx_desc.abs.host_buffer_data_ptr_hi) << 32;
#else
    phys_ptr = 0;
#endif
    phys_ptr |= rx_desc.abs.host_buffer_data_ptr_low;
    data_ptr = (void *)RDD_PHYS_TO_VIRT(phys_ptr);
#if !defined(RDP_SIM)
    if (rx_desc.abs.from_feed_ring)
        rdp_recycle_buf_to_feed(PDATA_TO_PFKBUFF(data_ptr, BCM_PKT_HEADROOM));
    else
#endif

    bdmf_sysb_free(data_ptr);

    ring_descr->shadow_read_idx = (++read_idx) % ring_descr->num_of_entries;

    return BDMF_ERR_OK;
}

int rdp_cpu_ring_recycle_free_host_buf(int ring_id, int budget)
{
    RING_DESCTIPTOR *ring_descr;
    int rc = 0;
    int i;

    /*do not invoke SOFT_IRQTX before freeing budget*/
#ifndef RDP_SIM
    local_bh_disable();
#endif    
    ring_descr = &host_ring[ring_id];

    /* Update with real value*/
    rdp_cpu_get_write_idx(ring_descr->ring_id, rdpa_ring_recycle, &ring_descr->shadow_write_idx);
    for (i = 0; i < budget; i++)
    {
        rc = _rdp_cpu_ring_recycle_free_host_buf(ring_descr);
        if (rc)
            break;
    }
    rdp_cpu_inc_read_idx(ring_id, rdpa_ring_recycle, i);

#ifndef RDP_SIM    
    local_bh_enable();
#endif

    return i;
}
EXPORT_SYMBOL(rdp_cpu_ring_recycle_free_host_buf);

uint32_t rdp_cpu_feed_ring_get_queued(void)
{
    RING_DESCTIPTOR *rd = &host_ring[FEED_RING_ID];
    uint16_t read_idx = 0;

    rdp_cpu_get_read_idx(FEED_RING_ID, rdpa_ring_feed, &read_idx);
    rd->shadow_read_idx = read_idx;
    return packets_count(rd, read_idx, rd->shadow_write_idx);
}

int rdp_cpu_fill_feed_ring(int budget)
{
    RING_DESCTIPTOR *feed_ring_descr = &host_ring[FEED_RING_ID];
    int rc = 0;
    int i;

#ifndef RDP_SIM  
    bdmf_fastlock_lock(&feed_ring_lock);
#endif
    rdp_cpu_get_read_idx(FEED_RING_ID, rdpa_ring_feed, &feed_ring_descr->shadow_read_idx);
#ifndef RDP_SIM  
    bdmf_fastlock_unlock(&feed_ring_lock);
#endif

    for (i = 0; i < budget; i++)
    {
#ifndef RDP_SIM  
        bdmf_fastlock_lock(&feed_ring_lock);
#endif
        rc = alloc_and_assign_packet_to_feed_ring();
#ifndef RDP_SIM  
        bdmf_fastlock_unlock(&feed_ring_lock);
#endif
        if (rc)
            break;
    }

#ifdef XRDP
    update_lowest_filling_level();
#endif /* XRDP */

#ifndef RDP_SIM  
    bdmf_fastlock_lock(&feed_ring_lock);
#endif

#ifdef CONFIG_BCM_CACHE_COHERENCY
        dma_wmb();
#endif

    rdd_cpu_inc_feed_ring_write_idx(i);
#ifndef RDP_SIM  
    bdmf_fastlock_unlock(&feed_ring_lock);
#endif
    return i;
}
EXPORT_SYMBOL(rdp_cpu_fill_feed_ring);

#ifndef RDP_SIM
void rdp_recycle_buf_to_feed(void *pdata)
{
    int rc = 0;
    RING_DESCTIPTOR *feed_ring_descr = &host_ring[FEED_RING_ID];

#if defined(XRDP) && !(defined(CONFIG_BCM_BPM) || defined(CONFIG_BCM_BPM_MODULE))
    BUG_ON(virt_to_head_page(pdata)->slab_cache == 0);
#endif

    bdmf_fastlock_lock(&feed_ring_lock);
    rc = __rdp_recycle_buf_to_feed(feed_ring_descr ,(void*)(PFKBUFF_TO_PDATA(pdata, BCM_PKT_HEADROOM)));
    if (rc == 0)
       threshold_recycle++; 

#ifdef XRDP
    update_lowest_filling_level();
#endif /* XRDP */

    if (WRITE_IDX_UPDATE_THR == threshold_recycle) 
    {
#ifdef CONFIG_BCM_CACHE_COHERENCY
        dma_wmb();
#endif
        rdd_cpu_inc_feed_ring_write_idx(threshold_recycle);
        threshold_recycle = 0; 
    }
    bdmf_fastlock_unlock(&feed_ring_lock);
}
#endif /* RDP_SIM */
EXPORT_SYMBOL(rdp_recycle_buf_to_feed);

#endif  /* XRDP */
#endif /*ifndef _CFE_*/

/* Callback Functions */

void rdp_packet_dump(uint32_t ringId, rdpa_cpu_rx_info_t *info)
{
    char name[10];

    sprintf(name, "Queue-%d", ringId);
#ifdef __KERNEL__
    rdpa_cpu_rx_dump_packet(name, rdpa_cpu_host, ringId, info, 0);
#endif
}
EXPORT_SYMBOL(rdp_packet_dump);

#ifndef _CFE_

/* BPM */

void* rdp_databuf_alloc(RING_DESCTIPTOR *pDescriptor)
{
    if (likely(pDescriptor->buff_cache_cnt))
    {
        return (void *)(pDescriptor->buff_cache[--pDescriptor->buff_cache_cnt]);
    }
    else
    {
        uint32_t alloc_cnt = 0;

        /* refill the local cache from global pool */
        alloc_cnt = bdmf_sysb_databuf_alloc((void **)pDescriptor->buff_cache, MAX_BUFS_IN_CACHE, pDescriptor->ring_prio, 0);

        if (alloc_cnt)
        {
            pDescriptor->buff_cache_cnt = alloc_cnt;
            return (void *)(pDescriptor->buff_cache[--pDescriptor->buff_cache_cnt]);
        }
    }
    return NULL;
}
EXPORT_SYMBOL(rdp_databuf_alloc);

void rdp_databuf_free(void *pBuf, uint32_t context, RING_DESCTIPTOR *pDescriptor)
{
    bdmf_sysb_databuf_free(pBuf, context);
}
EXPORT_SYMBOL(rdp_databuf_free);

/* Kmem_Cache */

void* rdp_databuf_alloc_cache(RING_DESCTIPTOR *pDescriptor)
{
    if (likely(pDescriptor->buff_cache_cnt))
    {
        return (void *)(pDescriptor->buff_cache[--pDescriptor->buff_cache_cnt]);
    }
    else
    {
        uint32_t alloc_cnt = 0;
        int i;

        /* refill the local cache from global pool */
        for (i=0; i<MAX_BUFS_IN_CACHE; i++, alloc_cnt++)
        {
            uint8_t *datap;

            /* allocate from kernel directly */
            datap = kmem_cache_alloc((struct kmem_cache*)(pDescriptor->buff_mem_context), GFP_ATOMIC);

            /* do a cache invalidate of the buffer */
            bdmf_dcache_inv((unsigned long)datap, pDescriptor->packet_size );

            pDescriptor->buff_cache[i] = datap;
        }

        if (alloc_cnt)
        {
            pDescriptor->buff_cache_cnt = alloc_cnt;
            return (void *)(pDescriptor->buff_cache[--pDescriptor->buff_cache_cnt]);
        }
    }
    return NULL;
}
EXPORT_SYMBOL(rdp_databuf_alloc_cache);


void rdp_databuf_free_cache(void *pBuf, uint32_t context, RING_DESCTIPTOR *pDescriptor)
{
    kmem_cache_free((struct kmem_cache*)(pDescriptor->buff_mem_context), pBuf);
}
EXPORT_SYMBOL(rdp_databuf_free_cache);

#elif defined(_CFE_)

void* rdp_databuf_alloc(RING_DESCTIPTOR *pDescriptor)
{
    void *pBuf = KMALLOC(BCM_PKTBUF_SIZE, 64);

    if (pBuf)
    {
        INV_RANGE(pBuf, BCM_PKTBUF_SIZE);
        return pBuf;
    }
    return NULL;
}

void rdp_databuf_free(void *pBuf, uint32_t context, RING_DESCTIPTOR *pDescriptor)
{
    KFREE(pBuf);
}

#endif

void rdp_cpu_ring_read_idx_ddr_sync(uint32_t ring_id)
{
#ifdef XRDP
    RING_DESCTIPTOR *ring_descr;

    ring_descr = &host_ring[ring_id];
    if (ring_descr->accum_inc)
        rdp_cpu_ring_desc_read_idx_sync(ring_descr);
#endif
}


#endif
