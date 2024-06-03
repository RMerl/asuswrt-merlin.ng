/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <contact@8051projects.net>
 *
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 */

#ifndef __ARMADA100_SPI_H_
#define __ARMADA100_SPI_H_

#include <asm/arch/armada100.h>

#define CAT_BASE_ADDR(x)	ARMD1_SSP ## x ## _BASE
#define SSP_REG_BASE(x)		CAT_BASE_ADDR(x)

/*
 * SSP Serial Port Registers
 * refer Appendix A.26
 */
struct ssp_reg {
	u32 sscr0;	/* SSP Control Register 0 - 0x000 */
	u32 sscr1;	/* SSP Control Register 1 - 0x004 */
	u32 sssr;	/* SSP Status Register - 0x008 */
	u32 ssitr;	/* SSP Interrupt Test Register - 0x00C */
	u32 ssdr;	/* SSP Data Register - 0x010 */
	u32 pad1[5];
	u32 ssto;	/* SSP Timeout Register - 0x028 */
	u32 sspsp;	/* SSP Programmable Serial Protocol Register - 0x02C */
	u32 sstsa;	/* SSP TX Timeslot Active Register - 0x030 */
	u32 ssrsa;	/* SSP RX Timeslot Active Register - 0x034 */
	u32 sstss;	/* SSP Timeslot Status Register - 0x038 */
};

#define DEFAULT_WORD_LEN	8
#define SSP_FLUSH_NUM		0x2000
#define RX_THRESH_DEF		8
#define TX_THRESH_DEF		8
#define TIMEOUT_DEF		1000

#define SSCR1_RIE	(1 << 0)	/* Receive FIFO Interrupt Enable */
#define SSCR1_TIE	(1 << 1)	/* Transmit FIFO Interrupt Enable */
#define SSCR1_LBM	(1 << 2)	/* Loop-Back Mode */
#define SSCR1_SPO	(1 << 3)	/* Motorola SPI SSPSCLK polarity
					   setting */
#define SSCR1_SPH	(1 << 4)	/* Motorola SPI SSPSCLK phase setting */
#define SSCR1_MWDS	(1 << 5)	/* Microwire Transmit Data Size */
#define SSCR1_TFT	0x03c0		/* Transmit FIFO Threshold (mask) */
#define SSCR1_RFT	0x3c00		/* Receive FIFO Threshold (mask) */

#define SSCR1_TXTRESH(x)	((x - 1) << 6)	/* level [1..16] */
#define SSCR1_RXTRESH(x)	((x - 1) << 10)	/* level [1..16] */
#define SSCR1_TINTE		(1 << 19)	/* Receiver Time-out
						   Interrupt enable */

#define SSCR0_DSS		0x0f		/* Data Size Select (mask) */
#define SSCR0_DATASIZE(x)	(x - 1)		/* Data Size Select [4..16] */
#define SSCR0_FRF		0x30		/* FRame Format (mask) */
#define SSCR0_MOTO		(0x0 << 4)	/* Motorola's Serial
						   Peripheral Interface */
#define SSCR0_TI		(0x1 << 4)	/* TI's Synchronous
						   Serial Protocol (SSP) */
#define SSCR0_NATIONAL		(0x2 << 4)	/* National Microwire */
#define SSCR0_ECS		(1 << 6)	/* External clock select */
#define SSCR0_SSE		(1 << 7)	/* Synchronous Serial Port
						   Enable */

#define SSSR_TNF	(1 << 2)	/* Transmit FIFO Not Full */
#define SSSR_RNE	(1 << 3)	/* Receive FIFO Not Empty */
#define SSSR_BSY	(1 << 4)	/* SSP Busy */
#define SSSR_TFS	(1 << 5)	/* Transmit FIFO Service Request */
#define SSSR_RFS	(1 << 6)	/* Receive FIFO Service Request */
#define SSSR_ROR	(1 << 7)	/* Receive FIFO Overrun */
#define SSSR_TINT	(1 << 19)	/* Receiver Time-out Interrupt */

#endif /* __ARMADA100_SPI_H_ */
