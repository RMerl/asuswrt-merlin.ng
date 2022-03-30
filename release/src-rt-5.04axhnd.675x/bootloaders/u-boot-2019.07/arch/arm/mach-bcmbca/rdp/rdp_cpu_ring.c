// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
   
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
#define INTERN_PRINT printf

#if !defined(RDP_SIM) || defined(XRDP)
#include "rdp_cpu_ring.h"
#include "rdp_cpu_ring_inline.h"
#include "rdp_mm.h"

#define ____cacheline_aligned
#define shell_print(dummy,format,...) xprintf(format, ##__VA_ARGS__)

#if !defined(XRDP) && !defined(__KERNEL__) && !defined(__UBOOT__)
#error "rdp_cpu_ring is supported only in CFE and Kernel modules or XRDP simulator"
#endif

RING_DESCTIPTOR ____cacheline_aligned host_ring[D_NUM_OF_RING_DESCRIPTORS] =
    { };
EXPORT_SYMBOL(host_ring);

/*delete a preallocated ring*/
int rdp_cpu_ring_delete_ring(uint32_t ring_id)
{
	RING_DESCTIPTOR *pDescriptor;
	int rc;

	pDescriptor = &host_ring[ring_id];
	if (!pDescriptor->num_of_entries) {
		INTERN_PRINT("ERROR:deleting ring_id %d which does not exists!",
			     ring_id);
		return -1;
	}

	rc = rdp_cpu_ring_buffers_free(pDescriptor);
	if (rc) {
		INTERN_PRINT
		    ("ERROR: failed free ring buffers ring_id %d, err %d\n",
		     ring_id, rc);
		return rc;
	}

	/* free any buffers in buff_cache */
	while (pDescriptor->buff_cache_cnt) {
		pDescriptor->databuf_free(pDescriptor->
					  buff_cache[--pDescriptor->
						     buff_cache_cnt], 0,
					  pDescriptor);
	}

	/*free buff_cache */
	if (pDescriptor->buff_cache)
		CACHED_FREE(pDescriptor->buff_cache);

	/*delete the ring of descriptors in case of non-coherent */
#ifndef RDP_SIM
	if (pDescriptor->base) {
		rdp_mm_aligned_free((void *)(pDescriptor->base),
				    pDescriptor->num_of_entries *
				    pDescriptor->size_of_entry);
	}
#endif
	pDescriptor->num_of_entries = 0;

	return 0;
}

EXPORT_SYMBOL(rdp_cpu_ring_delete_ring);

int rdp_cpu_ring_create_ring(uint32_t ring_id,
			     uint8_t ring_type,
			     uint32_t entries,
			     bdmf_phys_addr_t * ring_head,
			     uint32_t packetSize,
			     RING_CB_FUNC * ringCb, uint32_t ring_prio)
{
	return rdp_cpu_ring_create_ring_ex(ring_id, ring_type, entries,
					   ring_head, NULL, packetSize, ringCb,
					   ring_prio);
}

EXPORT_SYMBOL(rdp_cpu_ring_create_ring);

/* Using void * instead of (rdpa_cpu_rxq_ic_cfg_t *) to avoid CFE compile errors*/
int rdp_cpu_ring_create_ring_ex(uint32_t ring_id,
				uint8_t ring_type,
				uint32_t entries,
				bdmf_phys_addr_t * ring_head,
				bdmf_phys_addr_t * rw_idx_addr,
				uint32_t packetSize,
				RING_CB_FUNC * ringCb, uint32_t ring_prio)
{
	RING_DESCTIPTOR *pDescriptor;
	bdmf_phys_addr_t phy_addr = 0;

	if (ring_id >= RING_ID_NUM_OF) {
		INTERN_PRINT("ERROR: ring_id %d out of range(%d)", ring_id,
			     RING_ID_NUM_OF);
		return -1;
	}

	pDescriptor = &host_ring[ring_id];

	if (pDescriptor->num_of_entries) {
		INTERN_PRINT
		    ("ERROR: ring_id %d already exists! must be deleted first",
		     ring_id);
		return -1;
	}

	if (!entries) {
		INTERN_PRINT("ERROR: can't create ring with 0 packets\n");
		return -1;
	}

	/*set ring parameters */
	pDescriptor->ring_id = ring_id;
	pDescriptor->num_of_entries = entries;
	pDescriptor->num_of_entries_mask = pDescriptor->num_of_entries - 1;
	pDescriptor->ring_prio = ring_prio;

#ifdef XRDP
	if (ring_type == rdpa_ring_feed)
		pDescriptor->size_of_entry = sizeof(CPU_FEED_DESCRIPTOR);
	else if (ring_type == rdpa_ring_recycle)
		pDescriptor->size_of_entry = sizeof(CPU_RECYCLE_DESCRIPTOR);
	else if (ring_type == rdpa_ring_cpu_tx)
		pDescriptor->size_of_entry =
		    sizeof(RDD_RING_CPU_TX_DESCRIPTOR_DTS);
	else if (ring_type == rdpa_ring_data)
#endif
		pDescriptor->size_of_entry = sizeof(CPU_RX_DESCRIPTOR);

	INTERN_PRINT
	    ("Creating CPU ring for queue number %d with %d packets descriptor=0x%p, size_of_entry %d\n",
	     ring_id, entries, pDescriptor, pDescriptor->size_of_entry);

	pDescriptor->buff_cache_cnt = 0;
	pDescriptor->packet_size = packetSize;
	pDescriptor->type = ring_type;

	pDescriptor->databuf_alloc = rdp_databuf_alloc;
	pDescriptor->databuf_free = rdp_databuf_free;
	pDescriptor->data_dump = rdp_packet_dump;

	if (ringCb) {		/* overwrite if needed */
		pDescriptor->data_dump = ringCb->data_dump;
		pDescriptor->buff_mem_context = ringCb->buff_mem_context;
#ifndef XRDP
		pDescriptor->databuf_alloc = ringCb->databuf_alloc;
		pDescriptor->databuf_free = ringCb->databuf_free;
#endif
	}

	/*TODO:update the comment  allocate buff_cache which helps to reduce the overhead of when 
	 * allocating data buffers to ring descriptor */
	pDescriptor->buff_cache =
	    (uint8_t
	     **) (CACHED_MALLOC_ATOMIC(sizeof(uint8_t *) * MAX_BUFS_IN_CACHE));
	if (pDescriptor->buff_cache == NULL) {
		INTERN_PRINT
		    ("failed to allocate memory for cache of data buffers \n");
		return -1;
	}

	/*allocate ring descriptors - must be non-cacheable memory */
	pDescriptor->base =
	    rdp_mm_aligned_alloc((pDescriptor->size_of_entry * entries),
				 &phy_addr);
	if (pDescriptor->base == NULL) {
		INTERN_PRINT("failed to allocate memory for ring descriptor\n");
		rdp_cpu_ring_delete_ring(ring_id);
		return -1;
	}

	if (rdp_cpu_ring_buffers_init(pDescriptor, ring_id))
		return -1;

#ifndef XRDP
	/*set the ring header to the first entry */
	pDescriptor->head = (CPU_RX_DESCRIPTOR *) pDescriptor->base;

	/*using pointer arithmetics calculate the end of the ring */
	pDescriptor->end = (CPU_RX_DESCRIPTOR *) pDescriptor->base + entries;
#endif

	*ring_head = phy_addr;

#ifndef XRDP
	INTERN_PRINT("Done initializing Ring %d Base=0x%pK End=0x%pK "
		     "calculated entries= %ld RDD Base=%lxK descriptor=0x%p\n",
		     ring_id, pDescriptor->base, pDescriptor->end,
		     (long)(pDescriptor->end -
			    (CPU_RX_DESCRIPTOR *) pDescriptor->base),
		     (unsigned long)phy_addr, pDescriptor);
#else
	INTERN_PRINT
	    ("Done initializing Ring %d Base=0x%p num of entries= %d RDD Base=%lx descriptor=0x%p\n",
	     ring_id, pDescriptor->base, pDescriptor->num_of_entries,
	     (unsigned long)phy_addr, pDescriptor);
#endif

	return 0;
}

EXPORT_SYMBOL(rdp_cpu_ring_create_ring_ex);

#if defined(__UBOOT__) || !(defined(CONFIG_BCM63138) || defined(CONFIG_BCM63148))
/*this API copies the next available packet from ring to given pointer*/
int rdp_cpu_ring_read_packet_copy(uint32_t ring_id, CPU_RX_PARAMS * rxParams)
{
	RING_DESCTIPTOR *pDescriptor = &host_ring[ring_id];
#ifndef XRDP
	volatile CPU_RX_DESCRIPTOR *pTravel =
	    (volatile CPU_RX_DESCRIPTOR *)pDescriptor->head;
#endif
	void *client_pdata;
	uint32_t ret = 0;

	/* Data offset field is field ONLY in CFE driver on BCM6858 
	 * To ensure correct work of another platforms the data offset field should be zeroed */
	rxParams->data_offset = 0;

	client_pdata = (void *)rxParams->data_ptr;

#ifndef XRDP
	ret = ReadPacketFromRing(pDescriptor, pTravel, rxParams);
#else
	ret = ReadPacketFromRing(pDescriptor, rxParams);
#endif
	if (ret)
		goto exit;

	/*copy the data to user buffer */
	/*TODO: investigate why INV_RANGE is needed before memcpy, */
	INV_RANGE((rxParams->data_ptr + rxParams->data_offset),
		  rxParams->packet_size);
	memcpy(client_pdata,
	       (void *)(rxParams->data_ptr + rxParams->data_offset),
	       rxParams->packet_size);

	/*Assign the data buffer back to ring */
	INV_RANGE((rxParams->data_ptr + rxParams->data_offset),
		  rxParams->packet_size);
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

/* Callback Functions */

void rdp_packet_dump(uint32_t ringId, rdpa_cpu_rx_info_t * info)
{
	char name[10];

	sprintf(name, "Queue-%d", ringId);
#if !defined( __UBOOT__) && defined(__KERNEL__)
	rdpa_cpu_rx_dump_packet(name, rdpa_cpu_host, ringId, info, 0);
#endif
}

EXPORT_SYMBOL(rdp_packet_dump);

void *rdp_databuf_alloc(RING_DESCTIPTOR * pDescriptor)
{
	void *pBuf = KMALLOC(BCM_PKTBUF_SIZE, 64);

	if (pBuf) {
		INV_RANGE(pBuf, BCM_PKTBUF_SIZE);
		return pBuf;
	}
	return NULL;
}

void rdp_databuf_free(void *pBuf, uint32_t context,
		      RING_DESCTIPTOR * pDescriptor)
{
	KFREE(pBuf);
}

#endif
