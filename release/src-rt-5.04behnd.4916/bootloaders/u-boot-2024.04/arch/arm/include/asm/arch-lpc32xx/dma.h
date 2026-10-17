/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * LPC32xx DMA Controller Interface
 *
 * Copyright (C) 2008 by NXP Semiconductors
 * @Author: Kevin Wells
 * @Descr: Definitions for LPC3250 chip
 * @References: NXP LPC3250 User's Guide
 */

#ifndef _LPC32XX_DMA_H
#define _LPC32XX_DMA_H

/*
 * DMA linked list structure used with a channel's LLI register;
 * refer to UM10326, "LPC32x0 and LPC32x0/01 User manual" - Rev. 3
 * tables 84, 85, 86 & 87 for details.
 */
struct lpc32xx_dmac_ll {
	u32 dma_src;
	u32 dma_dest;
	u32 next_lli;
	u32 next_ctrl;
};

/* control register definitions */
#define DMAC_CHAN_INT_TC_EN	(1 << 31) /* channel terminal count interrupt */
#define DMAC_CHAN_DEST_AUTOINC	(1 << 27) /* automatic destination increment */
#define DMAC_CHAN_SRC_AUTOINC	(1 << 26) /* automatic source increment */
#define DMAC_CHAN_DEST_AHB1	(1 << 25) /* AHB1 master for dest. transfer */
#define DMAC_CHAN_DEST_WIDTH_32	(1 << 22) /* Destination data width selection */
#define DMAC_CHAN_SRC_WIDTH_32	(1 << 19) /* Source data width selection */
#define DMAC_CHAN_DEST_BURST_1	0
#define DMAC_CHAN_DEST_BURST_4	(1 << 15) /* Destination data burst size */
#define DMAC_CHAN_SRC_BURST_1	0
#define DMAC_CHAN_SRC_BURST_4	(1 << 12) /* Source data burst size */

/*
 * config_ch register definitions
 * DMAC_CHAN_FLOW_D_xxx: flow control with DMA as the controller
 * DMAC_DEST_PERIP: Macro for loading destination peripheral
 * DMAC_SRC_PERIP: Macro for loading source peripheral
 */
#define DMAC_CHAN_FLOW_D_M2P	(0x1 << 11)
#define DMAC_CHAN_FLOW_D_P2M	(0x2 << 11)
#define DMAC_DEST_PERIP(n)	(((n) & 0x1F) << 6)
#define DMAC_SRC_PERIP(n)	(((n) & 0x1F) << 1)

/*
 * config_ch register definitions
 * (source and destination peripheral ID numbers).
 * These can be used with the DMAC_DEST_PERIP and DMAC_SRC_PERIP macros.
 */
#define DMA_PERID_NAND1		1

/* Channel enable bit */
#define DMAC_CHAN_ENABLE	(1 << 0)

int lpc32xx_dma_get_channel(void);
int lpc32xx_dma_start_xfer(unsigned int channel,
			   const struct lpc32xx_dmac_ll *desc, u32 config);
int lpc32xx_dma_wait_status(unsigned int channel);

#endif /* _LPC32XX_DMA_H */
