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
#ifndef RDP_SIM
#include "rdp_cpu_ring.h"
#include "rdp_cpu_ring_inline.h"
#include "rdp_mm.h"

#ifndef _CFE_
#include "rdpa_cpu_helper.h"
#endif

#if defined(__KERNEL__)
static uint32_t		init_shell = 0;
int stats_reason[2][rdpa_cpu_reason__num_of]  = {}; /* reason statistics for US/DS */
EXPORT_SYMBOL(stats_reason);
#endif

#if !defined(__KERNEL__) && !defined(_CFE_)
	#error "rdp_cpu_ring is supported only in CFE and Kernel modules"
#endif

RING_DESCTIPTOR host_ring[D_NUM_OF_RING_DESCRIPTORS] = {};
EXPORT_SYMBOL(host_ring);


/*bdmf shell is compiling only for RDPA and not CFE*/
#ifdef __KERNEL__
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


static int cpu_ring_shell_list_rings(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t cntr;
    uint32_t first = 0 ,last = D_NUM_OF_RING_DESCRIPTORS;


    bdmf_session_print(session, "CPU RX Ring Descriptors \n");
    bdmf_session_print(session, "------------------------------\n");

    if (n_parms == 1 )
    {
        first = (uint32_t)parm[0].value.unumber;
        last = first + 1;
    }

    for (cntr = first; cntr < last; cntr++)
    {
        if(!host_ring[cntr].num_of_entries)
            continue;

        bdmf_session_print(session, "CPU RX Ring Queue = %d:\n", cntr);
        bdmf_session_print(session, "\tCPU RX Ring Queue = %d \n", cntr);
        bdmf_session_print(session, "\tNumber of entries = %d\n", host_ring[cntr].num_of_entries);
        bdmf_session_print(session, "\tSize of entry = %d bytes\n", host_ring[cntr].size_of_entry);
        bdmf_session_print(session, "\tAllocated Packet size = %d bytes\n", host_ring[cntr].packet_size);
        bdmf_session_print(session, "\tRing Base address = 0x%pK \n", host_ring[cntr].base);
        bdmf_session_print(session, "\tRing Head address = 0x%pK \n", host_ring[cntr].head);
        bdmf_session_print(session, "\tRing Head position = %ld \n", host_ring[cntr].head - host_ring[cntr].base);
        bdmf_session_print(session, "\tCurrently Queued = %d \n", rdp_cpu_ring_get_queued(cntr));
        bdmf_session_print(session, "-------------------------------\n");
        bdmf_session_print(session, "\n\n" );
    }

    return (0);
}

static int cpu_ring_shell_print_pd(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
	uint32_t 					ring_id;
	uint32_t					pdIndex;
	volatile CPU_RX_DESCRIPTOR*	pdPtr;
    CPU_RX_DESCRIPTOR             host_ring_desc;

	ring_id  = ( uint32_t )parm[ 0 ].value.unumber;
	pdIndex = ( uint32_t )parm[ 1].value.unumber;
	pdPtr = &host_ring_desc;

    memcpy(&host_ring_desc, host_ring[ring_id].base + pdIndex, sizeof(CPU_RX_DESCRIPTOR));

	bdmf_session_print ( session, "descriptor unswapped: %08x %08x %08x %08x\n", 
                                    pdPtr->word0, pdPtr->word1, pdPtr->word2, pdPtr->word3 );

    host_ring_desc.word0 = swap4bytes(host_ring_desc.word0);
    host_ring_desc.word2 = swap4bytes(host_ring_desc.word2);
    host_ring_desc.word3 = swap4bytes(host_ring_desc.word3);

	bdmf_session_print ( session, "descriptor swapped  : %08x %08x %08x %08x\n", 
                                    pdPtr->word0, pdPtr->word1, pdPtr->word2, pdPtr->word3 );
	bdmf_session_print ( session, "descriptor ownership is:%s\n",pdPtr->ownership ? "Host":"Runner");
	bdmf_session_print ( session, "descriptor packet DDR uncached address is:0x%pK\n",(void*)PHYS_TO_UNCACHED(pdPtr->host_buffer_data_pointer));
	bdmf_session_print ( session, "descriptor packet len is:%d\n",pdPtr->packet_length);
	if(pdPtr->ownership == OWNERSHIP_HOST)
	{
		bdmf_session_print ( session, "descriptor type is: %d\n",pdPtr->descriptor_type );
		bdmf_session_print ( session, "descriptor source port is: %d\n",pdPtr->source_port );
		//bdmf_session_print ( session, "descriptor packet reason is: %s\n",bdmf_attr_get_enum_text_hlp(&rdpa_cpu_reason_enum_table, pdPtr->reason) );
		bdmf_session_print ( session, "descriptor packet flow_id is: %d\n",pdPtr->flow_id );
		bdmf_session_print ( session, "descriptor packet wl chain id is: %d\n",pdPtr->wl_metadata & 0xFF);
		bdmf_session_print ( session, "descriptor packet wl priority is: %d\n",(pdPtr->wl_metadata & 0xFF00) >> 8);
	}
	return 0;
}

static int cpu_ring_shell_admin_ring ( bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms )
{
	uint32_t 					ring_id;
	ring_id  						= ( uint32_t )parm[ 0 ].value.unumber;
	host_ring[ring_id].admin_status 	= ( uint32_t )parm[ 1 ].value.unumber;

	bdmf_session_print ( session, "ring_id %d admin status is set to :%s\n",ring_id,host_ring[ring_id].admin_status ? "Up" : "Down");

	return 0;

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


    MAKE_BDMF_SHELL_CMD( cpu_dir, "sar",   "Show available rings", cpu_ring_shell_list_rings,
        					 BDMFMON_MAKE_PARM_RANGE( "ring_id", "ring id", BDMFMON_PARM_NUMBER, BDMFMON_PARM_FLAG_OPTIONAL, 0, D_NUM_OF_RING_DESCRIPTORS) );

    MAKE_BDMF_SHELL_CMD( cpu_dir, "vrpd",     "View Ring packet descriptor", cpu_ring_shell_print_pd,
    					 BDMFMON_MAKE_PARM_RANGE( "ring_id", "ring id", BDMFMON_PARM_NUMBER, 0, 0, D_NUM_OF_RING_DESCRIPTORS ),
                         BDMFMON_MAKE_PARM_RANGE( "descriptor", "packet descriptor index ", BDMFMON_PARM_NUMBER, 0, 0, RDPA_CPU_QUEUE_MAX_SIZE) );

    MAKE_BDMF_SHELL_CMD( cpu_dir, "cras",     "configure ring admin status", cpu_ring_shell_admin_ring,
        					 BDMFMON_MAKE_PARM_RANGE( "ring_id", "ring id", BDMFMON_PARM_NUMBER, 0, 0, D_NUM_OF_RING_DESCRIPTORS ),
                             BDMFMON_MAKE_PARM_RANGE( "admin", "ring admin status ", BDMFMON_PARM_NUMBER, 0, 0, 1) );

    return 0;
}
#endif /*__KERNEL__*/

/*delete a preallocated ring*/
int	rdp_cpu_ring_delete_ring(uint32_t ring_id)
{
	RING_DESCTIPTOR*			pDescriptor;

	pDescriptor = &host_ring[ring_id];
	if(!pDescriptor->num_of_entries)
	{
		printk("ERROR:deleting ring_id %d which does not exists!",ring_id);
		return -1;
	}

    rdp_cpu_ring_buffers_free(pDescriptor);

	/* free any buffers in buff_cache */
	while(pDescriptor->buff_cache_cnt) 
	{
		pDescriptor->databuf_free(pDescriptor->buff_cache[--pDescriptor->buff_cache_cnt], 0, pDescriptor);
	}

	/*free buff_cache */
	if(pDescriptor->buff_cache)
		CACHED_FREE(pDescriptor->buff_cache);

	/*delete the ring of descriptors*/
	if(pDescriptor->base)
		rdp_mm_aligned_free((void*)NONCACHE_TO_CACHE(pDescriptor->base),
				pDescriptor->num_of_entries * sizeof(CPU_RX_DESCRIPTOR));

	pDescriptor->num_of_entries = 0;

	return 0;
}
EXPORT_SYMBOL(rdp_cpu_ring_delete_ring);

/* Using void * instead of (rdpa_cpu_rxq_ic_cfg_t *) to avoid CFE compile errors*/
int rdp_cpu_ring_create_ring(uint32_t ring_id,
                             uint8_t ring_type,
                             uint32_t entries,
                             bdmf_phys_addr_t *ring_head,
                             uint32_t packetSize,
                             RING_CB_FUNC* ringCb)
{
	RING_DESCTIPTOR *pDescriptor;
        bdmf_phys_addr_t phy_addr;

	if( ring_id >= (RDP_CPU_RING_MAX_QUEUES + RDP_WLAN_MAX_QUEUES))
	{
		printk("ERROR: ring_id %d out of range(%d)",ring_id,RDP_CPU_RING_MAX_QUEUES);
		return -1;
	}

	pDescriptor = &host_ring[ ring_id ];
	if(pDescriptor->num_of_entries)
	{
		printk("ERROR: ring_id %d already exists! must be deleted first",ring_id);
		return -1;
	}

	if(!entries)
	{
		printk("ERROR: can't create ring with 0 packets\n");
		return -1;
	}

	printk("Creating CPU ring for queue number %d with %d packets descriptor=0x%p\n ",ring_id,entries,pDescriptor);

	/*set ring parameters*/
	pDescriptor->ring_id 			= ring_id;
	pDescriptor->admin_status		= 1;
	pDescriptor->num_of_entries		= entries;
	pDescriptor->size_of_entry		= sizeof(CPU_RX_DESCRIPTOR);
	pDescriptor->buff_cache_cnt = 0;
        pDescriptor->packet_size    = packetSize;

    if (ringCb == NULL)
    {
        pDescriptor->databuf_alloc  = rdp_databuf_alloc;
        pDescriptor->databuf_free   = rdp_databuf_free;
        pDescriptor->data_dump = rdp_packet_dump;
    }
    else
    {
        pDescriptor->databuf_alloc  = ringCb->databuf_alloc;
        pDescriptor->databuf_free   = ringCb->databuf_free;
        pDescriptor->data_dump = ringCb->data_dump;

        pDescriptor->buff_mem_context = ringCb->buff_mem_context;
    }

	/*TODO:update the comment  allocate buff_cache which helps to reduce the overhead of when 
	 * allocating data buffers to ring descriptor */
	pDescriptor->buff_cache = (uint8_t **)(CACHED_MALLOC_ATOMIC(sizeof(uint8_t*) * MAX_BUFS_IN_CACHE));
	if( pDescriptor->buff_cache == NULL )
	{
		printk("failed to allocate memory for cache of data buffers \n");
		return -1;
	}

	/*allocate ring descriptors - must be non-cacheable memory*/
	pDescriptor->base = (CPU_RX_DESCRIPTOR*)rdp_mm_aligned_alloc(sizeof(CPU_RX_DESCRIPTOR) * entries, &phy_addr);
	if( pDescriptor->base == NULL)
	{
		printk("failed to allocate memory for ring descriptor\n");
		rdp_cpu_ring_delete_ring(ring_id);
		return -1;
	}

    if (rdp_cpu_ring_buffers_init(pDescriptor, ring_id))
        return -1;

	/*set the ring header to the first entry*/
	pDescriptor->head = pDescriptor->base;

	/*using pointer arithmetics calculate the end of the ring*/
	pDescriptor->end  = pDescriptor->base + entries;

	*ring_head = phy_addr; 

	printk("Done initializing Ring %d Base=0x%pK End=0x%pK "
           "calculated entries= %ld RDD Base=0x%xK descriptor=0x%p\n",ring_id,
            pDescriptor->base, pDescriptor->end, (pDescriptor->end - pDescriptor->base), 
           phy_addr, pDescriptor);

#ifdef __KERNEL__
    {
        if(!init_shell)
        {
            if(ring_make_shell_commands())
            {	printk("Failed to create ring bdmf shell commands\n");
                return 1;
            }

            init_shell = 1;
        }
    }
#endif
	return 0;
}
EXPORT_SYMBOL(rdp_cpu_ring_create_ring);

void rdp_cpu_ring_free_mem(uint32_t ringId, void *pBuf)
{
    RING_DESCTIPTOR *pDescriptor     = &host_ring[ringId];

    if ((pDescriptor == NULL) || (pDescriptor->databuf_free == NULL))
    {
        printk("rdp_cpu_ring_free_mem: pDescriptor or free_cb is NULL, Memory is not freed.\n");
        return;
    }

    pDescriptor->databuf_free(pBuf, 0, pDescriptor);
}
EXPORT_SYMBOL(rdp_cpu_ring_free_mem);

#ifdef __KERNEL__

inline int rdp_cpu_ring_get_packet(uint32_t ringId, rdpa_cpu_rx_info_t *info)
{
    int rc;
    CPU_RX_PARAMS params;
    rdpa_traffic_dir dir;
    RING_DESCTIPTOR *pDescriptor     = &host_ring[ringId];

    //Check ringId range

    if (pDescriptor == NULL)
    {
        printk("rdp_cpu_ring_get_packet: pDescriptor is NULL\n");
        return BDMF_ERR_INTERNAL;
    }

    memset((void *)&params, 0, sizeof(CPU_RX_PARAMS));

    rc = rdp_cpu_ring_read_packet_refill(ringId, &params);
    if (rc)
    {
#ifdef LEGACY_RDP
        if (rc == RDD_ERROR_CPU_RX_QUEUE_EMPTY)
            return BDMF_ERR_NO_MORE;
#else
        if (rc == BDMF_ERR_NO_MORE)
            return rc;
#endif
        return BDMF_ERR_INTERNAL;
    }

    info->src_port = rdpa_cpu_rx_srcport_to_rdpa_if(params.src_bridge_port,
        params.flow_id);

    if (info->src_port == rdpa_if_none)
    {
        pDescriptor->stats_dropped++;

        pDescriptor->databuf_free(params.data_ptr, 0, pDescriptor);

        return BDMF_ERR_PERM;
    }
    info->reason = (rdpa_cpu_reason)params.reason;
    info->reason_data = params.flow_id;
    info->ptp_index = params.ptp_index;
    info->data = (void*)params.data_ptr;
    info->size = params.packet_size;
    dir = rdpa_if_is_wan(info->src_port) ? rdpa_dir_ds : rdpa_dir_us;
    stats_reason[dir][info->reason]++;

    pDescriptor->stats_received++;
    if (unlikely(pDescriptor->dump_enable))
        pDescriptor->data_dump(pDescriptor->ring_id, info);

    return rc;
}
EXPORT_SYMBOL(rdp_cpu_ring_get_packet);

void rdp_cpu_dump_data_cb(bdmf_index queue, bdmf_boolean enabled)
{
    host_ring[queue].dump_enable = enabled;
}
EXPORT_SYMBOL(rdp_cpu_dump_data_cb);

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

#endif //(__KERNEL__)

/*this function if for debug purposes*/
int	rdp_cpu_ring_get_queue_size(uint32_t ring_id)
{
	return host_ring[ ring_id ].num_of_entries;
}


/*this function if for debug purposes and should not be called during runtime*/
/*TODO:Add mutex to protect when reading while packets read from another context*/
int	rdp_cpu_ring_get_queued(uint32_t ring_id)
{
	RING_DESCTIPTOR*			pDescriptor = &host_ring[ ring_id ];
	volatile CPU_RX_DESCRIPTOR*	pTravel		= pDescriptor->base;
	volatile CPU_RX_DESCRIPTOR*	pEnd		= pDescriptor->end;
	uint32_t					packets		= 0;

	if(!pDescriptor->num_of_entries)
		return 0;

	while (pTravel != pEnd)
	{
        if (rdp_cpu_ring_is_ownership_host(pTravel))
            packets++;
		pTravel++;
	}

	return packets;
}

int	rdp_cpu_ring_flush(uint32_t ring_id)
{
	RING_DESCTIPTOR*			pDescriptor = &host_ring[ ring_id ];
	volatile CPU_RX_DESCRIPTOR*	pTravel		= pDescriptor->base;
	volatile CPU_RX_DESCRIPTOR*	pEnd		= pDescriptor->end;

	while (pTravel != pEnd)
	{
	    rdp_cpu_ring_set_ownership_runner(pTravel);
		pTravel++;
	}

	pDescriptor->head = pDescriptor->base;

	printk("cpu Ring %d has been flushed\n",ring_id);

	return 0;
}

int	rdp_cpu_ring_not_empty(uint32_t ring_id)
{
	RING_DESCTIPTOR*			pDescriptor = &host_ring[ ring_id ];
	CPU_RX_DESCRIPTOR*	pTravel		= (pDescriptor->head );

	return rdp_cpu_ring_is_ownership_host(pTravel);
}

int	rdp_cpu_ring_is_full(uint32_t ring_id)
{
    printk("NOT implemented\n");
    return 0;
}
#ifndef _CFE_
/*this API get the pointer of the next available packet and reallocate buffer in ring
 * in the descriptor is optimized to 16 bytes cache line, 6838 has 16 bytes cache line
 * while 68500 has 32 bytes cache line, so we don't prefetch the descriptor to cache
 * Also on ARM platform we are not sure of how to skip L2 cache, and use only L1 cache
 * so for now  always use uncached accesses to Packet Descriptor(pTravel)
 */

inline int rdp_cpu_ring_read_packet_refill(uint32_t ring_id, CPU_RX_PARAMS *rxParams)
{
   uint32_t                     ret;
   RING_DESCTIPTOR *pDescriptor     = &host_ring[ring_id];
   CPU_RX_DESCRIPTOR *pTravel       = pDescriptor->head;
   void *pNewBuf;

#ifdef __KERNEL__
   if (unlikely(pDescriptor->admin_status == 0))
   {
#ifndef LEGACY_RDP
       return BDMF_ERR_NO_MORE;
#else
       return RDD_ERROR_CPU_RX_QUEUE_EMPTY;
#endif
   }
#endif

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

#if defined(__KERNEL__)

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
        alloc_cnt = bdmf_sysb_databuf_alloc((void**)pDescriptor->buff_cache, MAX_BUFS_IN_CACHE, 0);

        if (alloc_cnt)
        {
            pDescriptor->buff_cache_cnt = alloc_cnt;
            return pDescriptor->buff_cache[--pDescriptor->buff_cache_cnt];
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
        return pDescriptor->buff_cache[--pDescriptor->buff_cache_cnt];
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
            return pDescriptor->buff_cache[--pDescriptor->buff_cache_cnt];
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
   void *pBuf = KMALLOC(BCM_PKTBUF_SIZE, 16);

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

#endif
