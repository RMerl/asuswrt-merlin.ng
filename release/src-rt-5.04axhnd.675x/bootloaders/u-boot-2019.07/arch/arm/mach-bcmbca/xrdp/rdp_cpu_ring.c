// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
   
 */

/* This file contains the implementation of the Runner CPU ring interface */

#include "rdp_cpu_ring.h"
#include "rdd_cpu_rx.h"
#include "rdp_cpu_ring_inline.h"
#include "rdp_mm.h"
#include "bdmf_system.h"

/* reason statistics for US/DS */
int stats_reason[2][rdpa_cpu_reason__num_of] = {};
EXPORT_SYMBOL(stats_reason);

#define RW_INDEX_SIZE (sizeof(uint16_t))

#ifndef CONFIG_BCMBCA_XRDP_GPL
#include "rdd_ag_cpu_rx.h"
#include "rdp_drv_sbpm.h"
#else
#include "rdpa_gpl_sbpm.h"
#endif

static int GetPdFromRamFifo(uint32_t *word0, uint32_t *word1, uint32_t *word2,
			    uint32_t *word3)
{
	static int idx = 0;
#if defined(CONFIG_BCM63146)
	static int prev_idx = 0;
	volatile uint32_t *desc_addr = (uint32_t *)RDD_SRAM_PD_FIFO_PTR(0);
	volatile uint32_t *prev_desc_addr = (uint32_t *)RDD_SRAM_PD_FIFO_PTR(0);
	RDD_CPU_RX_LAST_READ_INDEX_DTS *last_read_idx_ptr = RDD_CPU_RX_LAST_READ_INDEX_PTR(0);
	CPU_RX_DESCRIPTOR prev_rx_desc;

	desc_addr += idx;
	*word2 = swap4bytes(*(desc_addr + 2));
	*word3 = swap4bytes(*(desc_addr + 3));
	*word1 = swap4bytes(*(desc_addr + 1));
	*word0 = swap4bytes(*(desc_addr));

	if ((*word2 == 0) || (*word1 == 0))
		return BDMF_ERR_NO_MORE;

	/* need to invalidate cache for the previous buffer, or else
	 * HW cache eviction might corrupt the next RX packet that uses
	 * the same buffer */
	if (likely(prev_idx != idx)) {
		prev_desc_addr += prev_idx;
		prev_rx_desc.word0 = swap4bytes(*(prev_desc_addr));
		prev_rx_desc.word1 = swap4bytes(*(prev_desc_addr + 1));
		INV_RANGE(prev_rx_desc.word0, prev_rx_desc.fpm.packet_length);
	}
	prev_idx = idx;

	MWRITE_16(last_read_idx_ptr, ((uintptr_t)desc_addr & 0xffff));
	idx = (idx + 4) % (RDD_SRAM_PD_FIFO_SIZE * 4);
#else
	static int total_rx_packets = 0;

	INV_RANGE((uintptr_t)RDD_SRAM_PD_FIFO_ADDRESS_ARR[0] , (64 * 16));

	GROUP_MREAD_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR, (idx + 0)*sizeof(uint32_t),
			(*word0));
	GROUP_MREAD_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR, (idx + 1)*sizeof(uint32_t),
			(*word1));
	GROUP_MREAD_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR, (idx + 2)*sizeof(uint32_t),
			(*word2));
	GROUP_MREAD_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR, (idx + 3)*sizeof(uint32_t),
			(*word3));

	if (*word1 == 0)
		return BDMF_ERR_NO_MORE;

	total_rx_packets++;
	GROUP_MWRITE_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR,
			(idx + 3)*sizeof(uint32_t), 0);
	GROUP_MWRITE_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR,
			(idx + 2)*sizeof(uint32_t), 0);
	/* for 63146, we need first 2 PDs to free buffer back */
	GROUP_MWRITE_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR,
			(idx + 1)*sizeof(uint32_t), 0);
	GROUP_MWRITE_32(RDD_SRAM_PD_FIFO_ADDRESS_ARR,
			(idx + 0)*sizeof(uint32_t), 0);

	idx = (idx + 4) % (64 * 4);
#endif
	return 0;
}

static inline int ReadPacketFromFpmFwDirect(CPU_RX_PARAMS *rx_params)
{
	CPU_RX_DESCRIPTOR rx_desc;
	int ret;
#if !defined(CONFIG_BCM63146)
	uint32_t bn0;
#endif

#ifdef CONFIG_BCM_CACHE_COHERENCY
	/* Before accessing the descriptors must do barrier */
	dma_rmb();
#endif

	ret = GetPdFromRamFifo(&rx_desc.word0, &rx_desc.word1, &rx_desc.word2,
			       &rx_desc.word3);
	if (ret == BDMF_ERR_NO_MORE)
		return BDMF_ERR_NO_MORE;

	rx_params->packet_size = rx_desc.fpm.packet_length;

#if defined(CONFIG_BCM63146)
	rx_params->data_ptr = (uint8_t *)((uintptr_t)rx_desc.word0);
#else
	bn0 = rx_desc.sbpm.bn0;

	ret = drv_sbpm_copy_list(bn0, &rx_params->data_ptr[0]);
	if (ret != 0) {
		printf("copy sbpm failed\n");
		return BDMF_ERR_NO_MORE;
	}
	ret = drv_sbpm_free_list(bn0);
	if (ret != 0) {
		printf("free sbpm failed\n");
		return BDMF_ERR_NO_MORE;
	}
#endif

	/* The place of data_ofset is the same in all structures in this union
	 * we could use any.*/
	rx_params->data_offset = rx_desc.wan.data_offset;
	rx_params->src_bridge_port = rx_desc.wan.source_port;

	if (rx_desc.cpu_vport.vport >= RDD_CPU_VPORT_FIRST &&
	    rx_desc.cpu_vport.vport <= RDD_CPU_VPORT_LAST)
		rx_params->dst_ssid = rx_desc.cpu_vport.ssid;
	else if (!rx_desc.wan.is_src_lan)
		rx_params->flow_id = rx_desc.wan.wan_flow_id;
	rx_params->reason = (rdpa_cpu_reason)rx_desc.wan.reason;
#ifdef CONFIG_CPU_REDIRECT_MODE_SUPPORT
	if (rx_params->reason == rdpa_cpu_rx_reason_cpu_redirect) {
		rx_params->cpu_redirect_egress_queue = rx_desc.cpu_redirect.egress_queue;
		rx_params->cpu_redirect_wan_flow = rx_desc.cpu_redirect.wan_flow;
	}
#endif
	rx_params->is_ucast = rx_desc.is_ucast;
	rx_params->is_exception = rx_desc.is_exception;
	rx_params->is_rx_offload = rx_desc.is_rx_offload;
	rx_params->mcast_tx_prio = rx_desc.mcast_tx_prio;

	if (rx_desc.wl_nic.is_chain) {
		/* Re-construct metadata to comply rdd_fc_context_t for wl_nic. */
		uint16_t metadata_1 = rx_desc.wl_nic.iq_prio << 8 | rx_desc.wl_nic.chain_id;
		uint8_t metadata_0 = (1 << 3) | rx_desc.wl_nic.tx_prio;

		rx_params->wl_metadata = metadata_0 << 10 | metadata_1;
	} else
		rx_params->wl_metadata = rx_desc.wl_metadata;

	return 0;
}

/* this API copies the next available packet from ring to given pointer */
int rdp_cpu_ring_read_packet_copy(uint32_t ring_id, CPU_RX_PARAMS *rxParams)
{
#if !defined(CONFIG_BCM63146)
	void *client_pdata;
#endif
	uint32_t ret = 0;

	/* Data offset field is field ONLY in CFE driver on BCM6858
	 * To ensure correct work of another platforms the data offset field
	 * should be zeroed */
	rxParams->data_offset = 0;

#if !defined(CONFIG_BCM63146)
	client_pdata = (void*)rxParams->data_ptr;
#endif

	ret = ReadPacketFromFpmFwDirect(rxParams);
	if (ret)
		goto exit;

	/* Assign the data buffer back to ring */
	INV_RANGE((rxParams->data_ptr + rxParams->data_offset), rxParams->packet_size);

exit:

#if !defined(CONFIG_BCM63146)
	rxParams->data_ptr = client_pdata;
#endif
	return ret;
}

