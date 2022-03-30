/* SPDX-License-Identifier: GPL-2.0 */
/*
 *  Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com
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

#endif /* __TI_UDMA_H */
