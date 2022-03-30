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

#ifndef _RDP_CPU_RING_INLINE_H_
#define _RDP_CPU_RING_INLINE_H_

#ifndef RDP_SIM

#include "rdp_cpu_ring.h"

#if defined(__KERNEL__) || defined(__UBOOT__)

static inline void AssignPacketBuffertoRing(RING_DESCTIPTOR *pDescriptor, volatile CPU_RX_DESCRIPTOR *pTravel, void *pBuf)
{
   /* assign the buffer address to ring and set the ownership to runner
    * by clearing  bit 31 which is used as ownership flag */

   pTravel->word2 = swap4bytes(((VIRT_TO_PHYS(pBuf)) & 0x7fffffff));

   /* advance the head ptr, wrap around if needed*/
   if (++pDescriptor->head == pDescriptor->end) 
      pDescriptor->head = pDescriptor->base;
}

#if defined(BCM_DSL_RDP)
static inline int ReadPacketFromRing(RING_DESCTIPTOR *pDescriptor, volatile CPU_RX_DESCRIPTOR *pTravel, CPU_RX_PARAMS *rxParams)
{
   /* pTravel is in uncached mem so reading 32bits at a time into
      cached mem improves performance*/
   CPU_RX_DESCRIPTOR	    rxDesc;

   rxDesc.word2 = pTravel->word2;
   //printk("ReadPacketFromRing addr=%p ddr= %x\n",pTravel, rxDesc.word2);
   rxDesc.word2 = swap4bytes(rxDesc.word2);

   //printk("ReadPacketFromRing swapped bufaddr= %x\n", rxDesc.word2);
   if ((rxDesc.word2 & 0x80000000))
   {
      rxDesc.ownership = 0; /*clear the ownership bit */
      rxParams->data_ptr = (uint8_t *)PHYS_TO_CACHED(rxDesc.word2);

      rxDesc.word0 = pTravel->word0;
      rxDesc.word0 = swap4bytes(rxDesc.word0);

      rxParams->packet_size = rxDesc.packet_length;
      rxParams->src_bridge_port = (BL_LILAC_RDD_BRIDGE_PORT_DTE)rxDesc.source_port;
      rxParams->flow_id = rxDesc.flow_id;

#if defined(CONFIG_RUNNER_CSO)
      rxParams->is_csum_verified = rxDesc.is_chksum_verified;
#endif
      rxDesc.word1 = pTravel->word1;
      rxDesc.word1 = swap4bytes(rxDesc.word1);

      rxParams->dst_ssid = rxDesc.dst_ssid;

      rxDesc.word3 = pTravel->word3;
      rxDesc.word3 = swap4bytes(rxDesc.word3);

      if (rxDesc.is_rx_offload)
      {
          rxParams->reason            = rdpa_cpu_rx_reason_ipsec; /* hardcoded, in use by ipsec only */
          rxParams->free_index        = rxDesc.free_index;
          rxParams->is_rx_offload     = rxDesc.is_rx_offload;
          rxParams->is_ipsec_upstream = rxDesc.is_ipsec_upstream;
      }
      else
      {
          rxParams->reason      = (rdpa_cpu_reason)rxDesc.reason;
          rxParams->wl_metadata = rxDesc.wl_metadata;
          rxParams->ptp_index   = pTravel->ip_sync_1588_idx;
          rxParams->wl_metadata = 0;
      }

      return 0;
   }

   return BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_EMPTY;
}

#else
static inline int ReadPacketFromRing(RING_DESCTIPTOR *pDescriptor, volatile CPU_RX_DESCRIPTOR *pTravel, CPU_RX_PARAMS *rxParams)
{
   /* pTravel is in uncached mem so reading 32bits at a time into
      cached mem improves performance*/
   CPU_RX_DESCRIPTOR	    rxDesc;

   rxDesc.word2 = pTravel->word2;
   if ((rxDesc.ownership == OWNERSHIP_HOST))
   {
      rxParams->data_ptr = (uint8_t *)PHYS_TO_CACHED(rxDesc.word2);

      rxDesc.word0 = pTravel->word0;
      rxParams->packet_size = rxDesc.packet_length;
      rxParams->src_bridge_port = (BL_LILAC_RDD_BRIDGE_PORT_DTE)rxDesc.source_port;
      rxParams->flow_id = rxDesc.flow_id;

      rxDesc.word1 = pTravel->word1 ;
      rxParams->reason = (rdpa_cpu_reason)rxDesc.reason;
      rxParams->dst_ssid = rxDesc.dst_ssid;
      rxDesc.word3 = pTravel->word3 ;
      rxParams->wl_metadata = rxDesc.wl_metadata;
      rxParams->ptp_index = pTravel->ip_sync_1588_idx;

      return 0;
   }

   return BL_LILAC_RDD_ERROR_CPU_RX_QUEUE_EMPTY;
}
#endif

static inline int rdp_cpu_ring_buffers_free(RING_DESCTIPTOR *pDescriptor)
{
    volatile CPU_RX_DESCRIPTOR *pTravel;
    uint32_t i;

    for (pTravel =(volatile CPU_RX_DESCRIPTOR*)pDescriptor->base, i = 0 ; i < pDescriptor->num_of_entries;
        pTravel++, i++)
    {
        if (pTravel->word2)
        {
#ifdef _BYTE_ORDER_LITTLE_ENDIAN_
            // little-endian ownership is MSb of LSB
            pTravel->word2 = swap4bytes(pTravel->word2 | 0x80);
#else
            // big-endian ownership is MSb of MSB
            pTravel->ownership = OWNERSHIP_HOST;
#endif
            pDescriptor->databuf_free((void *)PHYS_TO_CACHED(pTravel->host_buffer_data_pointer), 0, pDescriptor);
            pTravel->word2 = 0;
        }
    }

    return 0;
}

static inline int rdp_cpu_ring_buffers_init(RING_DESCTIPTOR *pDescriptor, uint32_t ring_id)
{
	volatile CPU_RX_DESCRIPTOR *pTravel;
	void *dataPtr;
	uint32_t i;

	for (pTravel =(volatile CPU_RX_DESCRIPTOR*)pDescriptor->base, i = 0; i < pDescriptor->num_of_entries;
        pTravel++ ,i++)
	{
		memset((void*)pTravel,0,sizeof(*pTravel));

        /*allocate actual packet in DDR*/

		dataPtr = pDescriptor->databuf_alloc(pDescriptor);

		if (dataPtr)
		{
#ifdef _BYTE_ORDER_LITTLE_ENDIAN_
			/* since ARM is little-endian and runner is big-endian
			 * we need to byte-swap dataPtr and clear ownership
			 */
			pTravel->word2 = swap4bytes(VIRT_TO_PHYS(dataPtr)) & ~0x80;
#else
			pTravel->host_buffer_data_pointer = VIRT_TO_PHYS(dataPtr);
			pTravel->ownership = OWNERSHIP_RUNNER;
#endif        
		}
		else
		{
			pTravel->host_buffer_data_pointer = 0; /* NULL */
			printk("failed to allocate packet map entry=%d\n", i);
			rdp_cpu_ring_delete_ring(ring_id);
			return -1;
		}
	}
    return 0;
}

#endif /* defined(__KERNEL__) || defined(__UBOOT__) */
#endif /* RDP_SIM */

static inline int rdp_cpu_ring_is_ownership_host(volatile CPU_RX_DESCRIPTOR *pTravel)
{
#ifdef _BYTE_ORDER_LITTLE_ENDIAN_
	return pTravel->word2 & 0x80;
#else
	return pTravel->ownership == OWNERSHIP_HOST;
#endif    
}

static inline void rdp_cpu_ring_set_ownership_runner(volatile CPU_RX_DESCRIPTOR *pTravel)
{
#ifdef _BYTE_ORDER_LITTLE_ENDIAN_
    pTravel->word2 &= ~0x80;
#else
    pTravel->ownership = OWNERSHIP_RUNNER;
#endif    
}

#endif /* _RDP_CPU_RING_INLINE_H_ */
