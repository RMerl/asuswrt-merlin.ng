/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com
 *  Author: Peter Ujfalusi <peter.ujfalusi@ti.com>
 */

#ifndef __TI_UDMA_H
#define __TI_UDMA_H

/**
 * struct ti_udma_drv_packet_data - TI UDMA transfer specific data
 *
 * @pkt_type: Packet Type - specific for each DMA client HW
 * @dest_tag: Destination tag The source pointer.
 *
 * TI UDMA transfer specific data passed as part of DMA transfer to
 * the DMA client HW in UDMA descriptors.
 */
struct ti_udma_drv_packet_data {
	u32	pkt_type;
	u32	dest_tag;
};

/**
 * struct ti_udma_drv_chan_cfg_data - TI UDMA per channel specific
 *                                     configuration data
 *
 * @flow_id_base: Start index of flow ID allocated to this channel
 * @flow_id_cnt: Number of flows allocated for this channel starting at
 *               flow_id_base
 *
 * TI UDMA channel specific data returned as part of dma_get_cfg() call
 * from the DMA client driver.
 */
struct ti_udma_drv_chan_cfg_data {
	u32	flow_id_base;
	u32	flow_id_cnt;
};

/* TI UDMA specific flag IDs for dma_get_cfg() call */
#define TI_UDMA_CHAN_PRIV_INFO		0

#endif /* __TI_UDMA_H */
