/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Xilinx Multirate Ethernet MAC(MRMAC) driver
 *
 * Author(s):   Ashok Reddy Soma <ashok.reddy.soma@xilinx.com>
 *              Michal Simek <michal.simek@amd.com>
 *
 * Copyright (C) 2021 Xilinx, Inc. All rights reserved.
 */

#ifndef __XILINX_AXI_MRMAC_H
#define __XILINX_AXI_MRMAC_H

#define MIN_PKT_SIZE	60

/* MRMAC needs atleast two buffer descriptors for Tx/Rx to work.
 * Otherwise MRMAC will drop the packets. So, have atleast two Tx and
 * two Rx bd's.
 */
#define TX_DESC		2
#define RX_DESC		2

/* MRMAC platform data structure */
struct axi_mrmac_plat {
	struct eth_pdata eth_pdata;
	struct mcdma_common_regs *mm2s_cmn;
	u32 mrmac_rate; /* Hold the value from DT property "mrmac-rate" */
};

/* MRMAC private driver structure */
struct axi_mrmac_priv {
	struct mrmac_regs *iobase;
	struct mcdma_common_regs *mm2s_cmn;
	struct mcdma_common_regs *s2mm_cmn;
	struct mcdma_chan_reg *mcdma_tx;
	struct mcdma_chan_reg *mcdma_rx;
	struct mcdma_bd *tx_bd[TX_DESC];
	struct mcdma_bd *rx_bd[RX_DESC];
	u8 *txminframe;		/* Pointer to hold min length Tx frame(60) */
	u32 mrmac_rate;		/* Speed to configure(Read from DT 10G/25G..) */
};

/* MRMAC Register Definitions */
struct mrmac_regs {
	u32 revision;	/* 0x0: Revision Register */
	u32 reset;	/* 0x4: Reset Register */
	u32 mode;	/* 0x8: Mode */
	u32 tx_config;	/* 0xc: Tx Configuration */
	u32 rx_config;	/* 0x10: Rx Configuration */
	u32 reserved[6];/* 0x14-0x28: Reserved */
	u32 tick_reg;	/* 0x2c: Tick Register */
};

#define TX_BD_TOTAL_SIZE		(TX_DESC * sizeof(struct mcdma_bd))
#define RX_BD_TOTAL_SIZE		(RX_DESC * sizeof(struct mcdma_bd))

#define RX_BUFF_TOTAL_SIZE		(RX_DESC * PKTSIZE_ALIGN)

/* Status Registers */
#define MRMAC_TX_STS_OFFSET		0x740
#define MRMAC_RX_STS_OFFSET		0x744
#define MRMAC_TX_RT_STS_OFFSET		0x748
#define MRMAC_RX_RT_STS_OFFSET		0x74c
#define MRMAC_STATRX_BLKLCK_OFFSET	0x754

/* Register bit masks */
#define MRMAC_RX_SERDES_RST_MASK	(BIT(3) | BIT(2) | BIT(1) | BIT(0))
#define MRMAC_TX_SERDES_RST_MASK	BIT(4)
#define MRMAC_RX_RST_MASK		BIT(5)
#define MRMAC_TX_RST_MASK		BIT(6)
#define MRMAC_RX_AXI_RST_MASK		BIT(8)
#define MRMAC_TX_AXI_RST_MASK		BIT(9)
#define MRMAC_STS_ALL_MASK		0xffffffff

#define MRMAC_RX_EN_MASK		BIT(0)
#define MRMAC_RX_DEL_FCS_MASK		BIT(1)

#define MRMAC_TX_EN_MASK		BIT(0)
#define MRMAC_TX_INS_FCS_MASK		BIT(1)

#define MRMAC_RX_BLKLCK_MASK		BIT(0)

#define MRMAC_TICK_TRIGGER		BIT(0)

#define MRMAC_RESET_DELAY		1   /* Delay in msecs */
#define MRMAC_BLKLCK_TIMEOUT		100 /* Block lock timeout in msecs */
#define MRMAC_DMARST_TIMEOUT		500 /* MCDMA reset timeout in msecs */

#define XMCDMA_RX_OFFSET		0x500
#define XMCDMA_CHAN_OFFSET		0x40

/* MCDMA Channel numbers are from 1-16 */
#define XMCDMA_CHANNEL_1	BIT(0)
#define XMCDMA_CHANNEL_2	BIT(1)

#define XMCDMA_CR_RUNSTOP	BIT(0)
#define XMCDMA_CR_RESET		BIT(2)

#define XMCDMA_BD_CTRL_TXSOF_MASK	BIT(31)		/* First tx packet */
#define XMCDMA_BD_CTRL_TXEOF_MASK	BIT(30)		/* Last tx packet */
#define XMCDMA_BD_CTRL_ALL_MASK		GENMASK(31, 30)	/* All control bits */
#define XMCDMA_BD_STS_ALL_MASK		GENMASK(31, 28)	/* All status bits */

/* MCDMA Mask registers */
#define XMCDMA_CR_RUNSTOP_MASK		BIT(0) /* Start/stop DMA channel */
#define XMCDMA_CR_RESET_MASK		BIT(2) /* Reset DMA engine */

#define XMCDMA_SR_HALTED_MASK		BIT(0)
#define XMCDMA_SR_IDLE_MASK		BIT(1)

#define XMCDMA_CH_IDLE			BIT(0)

#define XMCDMA_BD_STS_COMPLETE		BIT(31) /* Completed */
#define XMCDMA_BD_STS_DEC_ERR		BIT(20) /* Decode error */
#define XMCDMA_BD_STS_SLV_ERR		BIT(29) /* Slave error */
#define XMCDMA_BD_STS_INT_ERR		BIT(28) /* Internal err */
#define XMCDMA_BD_STS_ALL_ERR		GENMASK(30, 28) /* All errors */

#define XMCDMA_IRQ_ERRON_OTHERQ_MASK	BIT(3)
#define XMCDMA_IRQ_PKTDROP_MASK		BIT(4)
#define XMCDMA_IRQ_IOC_MASK		BIT(5)
#define XMCDMA_IRQ_DELAY_MASK		BIT(6)
#define XMCDMA_IRQ_ERR_MASK		BIT(7)
#define XMCDMA_IRQ_ALL_MASK		GENMASK(7, 5)
#define XMCDMA_PKTDROP_COALESCE_MASK	GENMASK(15, 8)
#define XMCDMA_COALESCE_MASK		GENMASK(23, 16)
#define XMCDMA_DELAY_MASK		GENMASK(31, 24)

#define MRMAC_CTL_DATA_RATE_MASK	GENMASK(2, 0)
#define MRMAC_CTL_DATA_RATE_10G		0
#define MRMAC_CTL_DATA_RATE_25G		1
#define MRMAC_CTL_DATA_RATE_40G		2
#define MRMAC_CTL_DATA_RATE_50G		3
#define MRMAC_CTL_DATA_RATE_100G	4

#define MRMAC_CTL_AXIS_CFG_MASK		GENMASK(11, 9)
#define MRMAC_CTL_AXIS_CFG_SHIFT	9
#define MRMAC_CTL_AXIS_CFG_10G_IND	1
#define MRMAC_CTL_AXIS_CFG_25G_IND	1

#define MRMAC_CTL_SERDES_WIDTH_MASK	GENMASK(6, 4)
#define MRMAC_CTL_SERDES_WIDTH_SHIFT	4
#define MRMAC_CTL_SERDES_WIDTH_10G	4
#define MRMAC_CTL_SERDES_WIDTH_25G	6

#define MRMAC_CTL_RATE_CFG_MASK		(MRMAC_CTL_DATA_RATE_MASK | \
					 MRMAC_CTL_AXIS_CFG_MASK | \
					 MRMAC_CTL_SERDES_WIDTH_MASK)

#define MRMAC_CTL_PM_TICK_MASK		BIT(30)
#define MRMAC_TICK_TRIGGER		BIT(0)

#define XMCDMA_BD_STS_ACTUAL_LEN_MASK  0x007fffff /* Actual length */

/* MCDMA common offsets */
struct mcdma_common_regs {
	u32 control;	/* Common control */
	u32 status;	/* Common status */
	u32 chen;	/* Channel enable/disable */
	u32 chser;	/* Channel in progress */
	u32 err;	/* Error */
	u32 ch_schd_type;	/* Channel Q scheduler type */
	u32 wrr_reg1;	/* Weight of each channel (ch1-8) */
	u32 wrr_reg2;	/* Weight of each channel (ch9-16) */
	u32 ch_serviced;	/* Channels completed */
	u32 arcache_aruser;	/* ARCACHE and ARUSER values for AXI4 read */
	u32 intr_status;	/* Interrupt monitor */
	u32 reserved[5];
};

/* MCDMA per-channel registers */
struct mcdma_chan_reg {
	u32 control;	/* Control */
	u32 status;	/* Status */
	u32 current;	/* Current descriptor */
	u32 current_hi;	/* Current descriptor high 32bit */
	u32 tail;	/* Tail descriptor */
	u32 tail_hi;	/* Tail descriptor high 32bit */
	u32 pktcnt;	/* Packet processed count */
};

/* MCDMA buffer descriptors */
struct mcdma_bd {
	u32 next_desc;	/* Next descriptor pointer */
	u32 next_desc_msb;
	u32 buf_addr;	/* Buffer address */
	u32 buf_addr_msb;
	u32 reserved1;
	u32 cntrl;	/* Control */
	u32 status;	/* Status */
	u32 sband_stats;
	u32 app0;
	u32 app1;	/* Tx start << 16 | insert */
	u32 app2;	/* Tx csum seed */
	u32 app3;
	u32 app4;
	u32 sw_id_offset;
	u32 reserved2;
	u32 reserved3;
	u32 reserved4[16];
};

#endif	/* __XILINX_AXI_MRMAC_H */
