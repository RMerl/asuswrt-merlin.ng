// SPDX-License-Identifier: GPL-2.0+
/*
 * Multicore Navigator driver for TI Keystone 2 devices.
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#include <asm/ti-common/keystone_nav.h>

#ifdef CONFIG_KSNAV_PKTDMA_NETCP
/* NETCP Pktdma */
struct pktdma_cfg netcp_pktdma = {
	.global		= (void *)CONFIG_KSNAV_NETCP_PDMA_CTRL_BASE,
	.tx_ch		= (void *)CONFIG_KSNAV_NETCP_PDMA_TX_BASE,
	.tx_ch_num	= CONFIG_KSNAV_NETCP_PDMA_TX_CH_NUM,
	.rx_ch		= (void *)CONFIG_KSNAV_NETCP_PDMA_RX_BASE,
	.rx_ch_num	= CONFIG_KSNAV_NETCP_PDMA_RX_CH_NUM,
	.tx_sched	= (u32 *)CONFIG_KSNAV_NETCP_PDMA_SCHED_BASE,
	.rx_flows	= (void *)CONFIG_KSNAV_NETCP_PDMA_RX_FLOW_BASE,
	.rx_flow_num	= CONFIG_KSNAV_NETCP_PDMA_RX_FLOW_NUM,
	.rx_free_q	= CONFIG_KSNAV_NETCP_PDMA_RX_FREE_QUEUE,
	.rx_rcv_q	= CONFIG_KSNAV_NETCP_PDMA_RX_RCV_QUEUE,
	.tx_snd_q	= CONFIG_KSNAV_NETCP_PDMA_TX_SND_QUEUE,
};
#endif
