/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014 Broadcom Corporation.
 */

#ifndef _BCM_SF2_ETH_GMAC_H_
#define _BCM_SF2_ETH_GMAC_H_

#define BCM_SF2_ETH_MAC_NAME	"gmac"

#ifndef ETHHW_PORT_INT
#define ETHHW_PORT_INT		8
#endif

#define GMAC0_REG_BASE			0x18042000
#define GMAC0_DEV_CTRL_ADDR		GMAC0_REG_BASE
#define GMAC0_INT_STATUS_ADDR		(GMAC0_REG_BASE + 0x020)
#define GMAC0_INTR_RECV_LAZY_ADDR	(GMAC0_REG_BASE + 0x100)
#define GMAC0_PHY_CTRL_ADDR		(GMAC0_REG_BASE + 0x188)


#define GMAC_DMA_PTR_OFFSET		0x04
#define GMAC_DMA_ADDR_LOW_OFFSET	0x08
#define GMAC_DMA_ADDR_HIGH_OFFSET	0x0c
#define GMAC_DMA_STATUS0_OFFSET		0x10
#define GMAC_DMA_STATUS1_OFFSET		0x14

#define GMAC0_DMA_TX_CTRL_ADDR		(GMAC0_REG_BASE + 0x200)
#define GMAC0_DMA_TX_PTR_ADDR \
		(GMAC0_DMA_TX_CTRL_ADDR + GMAC_DMA_PTR_OFFSET)
#define GMAC0_DMA_TX_ADDR_LOW_ADDR \
		(GMAC0_DMA_TX_CTRL_ADDR + GMAC_DMA_ADDR_LOW_OFFSET)
#define GMAC0_DMA_TX_ADDR_HIGH_ADDR \
		(GMAC0_DMA_TX_CTRL_ADDR + GMAC_DMA_ADDR_HIGH_OFFSET)
#define GMAC0_DMA_TX_STATUS0_ADDR \
		(GMAC0_DMA_TX_CTRL_ADDR + GMAC_DMA_STATUS0_OFFSET)
#define GMAC0_DMA_TX_STATUS1_ADDR \
		(GMAC0_DMA_TX_CTRL_ADDR + GMAC_DMA_STATUS1_OFFSET)

#define GMAC0_DMA_RX_CTRL_ADDR		(GMAC0_REG_BASE + 0x220)
#define GMAC0_DMA_RX_PTR_ADDR \
		(GMAC0_DMA_RX_CTRL_ADDR + GMAC_DMA_PTR_OFFSET)
#define GMAC0_DMA_RX_ADDR_LOW_ADDR \
		(GMAC0_DMA_RX_CTRL_ADDR + GMAC_DMA_ADDR_LOW_OFFSET)
#define GMAC0_DMA_RX_ADDR_HIGH_ADDR \
		(GMAC0_DMA_RX_CTRL_ADDR + GMAC_DMA_ADDR_HIGH_OFFSET)
#define GMAC0_DMA_RX_STATUS0_ADDR \
		(GMAC0_DMA_RX_CTRL_ADDR + GMAC_DMA_STATUS0_OFFSET)
#define GMAC0_DMA_RX_STATUS1_ADDR \
		(GMAC0_DMA_RX_CTRL_ADDR + GMAC_DMA_STATUS1_OFFSET)

#define UNIMAC0_CMD_CFG_ADDR		(GMAC0_REG_BASE + 0x808)
#define UNIMAC0_MAC_MSB_ADDR		(GMAC0_REG_BASE + 0x80c)
#define UNIMAC0_MAC_LSB_ADDR		(GMAC0_REG_BASE + 0x810)
#define UNIMAC0_FRM_LENGTH_ADDR		(GMAC0_REG_BASE + 0x814)

#define GMAC0_IRL_FRAMECOUNT_SHIFT	24

/* transmit channel control */
/* transmit enable */
#define D64_XC_XE		0x00000001
/* transmit suspend request */
#define D64_XC_SE		0x00000002
/* parity check disable */
#define D64_XC_PD		0x00000800
/* BurstLen bits */
#define D64_XC_BL_MASK		0x001C0000
#define D64_XC_BL_SHIFT		18

/* transmit descriptor table pointer */
/* last valid descriptor */
#define D64_XP_LD_MASK		0x00001fff

/* transmit channel status */
/* transmit state */
#define D64_XS0_XS_MASK		0xf0000000
#define D64_XS0_XS_SHIFT	28
#define D64_XS0_XS_DISABLED	0x00000000
#define D64_XS0_XS_ACTIVE	0x10000000
#define D64_XS0_XS_IDLE		0x20000000
#define D64_XS0_XS_STOPPED	0x30000000
#define D64_XS0_XS_SUSP		0x40000000

/* receive channel control */
/* receive enable */
#define D64_RC_RE		0x00000001
/* address extension bits */
#define D64_RC_AE		0x00030000
/* overflow continue */
#define D64_RC_OC		0x00000400
/* parity check disable */
#define D64_RC_PD		0x00000800
/* receive frame offset */
#define D64_RC_RO_MASK		0x000000fe
#define D64_RC_RO_SHIFT		1
/* BurstLen bits */
#define D64_RC_BL_MASK		0x001C0000
#define D64_RC_BL_SHIFT		18

/* flags for dma controller */
/* partity enable */
#define DMA_CTRL_PEN		(1 << 0)
/* rx overflow continue */
#define DMA_CTRL_ROC		(1 << 1)

/* receive descriptor table pointer */
/* last valid descriptor */
#define D64_RP_LD_MASK		0x00001fff

/* receive channel status */
/* current descriptor pointer */
#define D64_RS0_CD_MASK		0x00001fff
/* receive state */
#define D64_RS0_RS_MASK		0xf0000000
#define D64_RS0_RS_SHIFT	28
#define D64_RS0_RS_DISABLED	0x00000000
#define D64_RS0_RS_ACTIVE	0x10000000
#define D64_RS0_RS_IDLE		0x20000000
#define D64_RS0_RS_STOPPED	0x30000000
#define D64_RS0_RS_SUSP		0x40000000

/* descriptor control flags 1 */
/* core specific flags */
#define D64_CTRL_COREFLAGS	0x0ff00000
/* end of descriptor table */
#define D64_CTRL1_EOT		((uint32_t)1 << 28)
/* interrupt on completion */
#define D64_CTRL1_IOC		((uint32_t)1 << 29)
/* end of frame */
#define D64_CTRL1_EOF		((uint32_t)1 << 30)
/* start of frame */
#define D64_CTRL1_SOF		((uint32_t)1 << 31)

/* descriptor control flags 2 */
/* buffer byte count. real data len must <= 16KB */
#define D64_CTRL2_BC_MASK	0x00007fff
/* address extension bits */
#define D64_CTRL2_AE		0x00030000
#define D64_CTRL2_AE_SHIFT	16
/* parity bit */
#define D64_CTRL2_PARITY	0x00040000
/* control flags in the range [27:20] are core-specific and not defined here */
#define D64_CTRL_CORE_MASK	0x0ff00000

#define DC_MROR		0x00000010
#define PC_MTE		0x00800000

/* command config */
#define CC_TE		0x00000001
#define CC_RE		0x00000002
#define CC_ES_MASK	0x0000000c
#define CC_ES_SHIFT	2
#define CC_PROM		0x00000010
#define CC_PAD_EN	0x00000020
#define CC_CF		0x00000040
#define CC_PF		0x00000080
#define CC_RPI		0x00000100
#define CC_TAI		0x00000200
#define CC_HD		0x00000400
#define CC_HD_SHIFT	10
#define CC_SR		0x00002000
#define CC_ML		0x00008000
#define CC_AE		0x00400000
#define CC_CFE		0x00800000
#define CC_NLC		0x01000000
#define CC_RL		0x02000000
#define CC_RED		0x04000000
#define CC_PE		0x08000000
#define CC_TPI		0x10000000
#define CC_AT		0x20000000

#define I_PDEE		0x00000400
#define I_PDE		0x00000800
#define I_DE		0x00001000
#define I_RDU		0x00002000
#define I_RFO		0x00004000
#define I_XFU		0x00008000
#define I_RI		0x00010000
#define I_XI0		0x01000000
#define I_XI1		0x02000000
#define I_XI2		0x04000000
#define I_XI3		0x08000000
#define I_ERRORS	(I_PDEE | I_PDE | I_DE | I_RDU | I_RFO | I_XFU)
#define DEF_INTMASK	(I_XI0 | I_XI1 | I_XI2 | I_XI3 | I_RI | I_ERRORS)

#define I_INTMASK	0x0f01fcff

#define CHIP_DRU_BASE				0x0301d000
#define CRMU_CHIP_IO_PAD_CONTROL_ADDR		(CHIP_DRU_BASE + 0x0bc)
#define SWITCH_GLOBAL_CONFIG_ADDR		(CHIP_DRU_BASE + 0x194)

#define CDRU_IOMUX_FORCE_PAD_IN_SHIFT		0
#define CDRU_SWITCH_BYPASS_SWITCH_SHIFT		13

#define AMAC0_IDM_RESET_ADDR			0x18110800
#define AMAC0_IO_CTRL_DIRECT_ADDR		0x18110408
#define AMAC0_IO_CTRL_CLK_250_SEL_SHIFT		6
#define AMAC0_IO_CTRL_GMII_MODE_SHIFT		5
#define AMAC0_IO_CTRL_DEST_SYNC_MODE_EN_SHIFT	3

#define CHIPA_CHIP_ID_ADDR			0x18000000
#define CHIPID		(readl(CHIPA_CHIP_ID_ADDR) & 0xFFFF)
#define CHIPREV		(((readl(CHIPA_CHIP_ID_ADDR) >> 16) & 0xF)
#define CHIPSKU		(((readl(CHIPA_CHIP_ID_ADDR) >> 20) & 0xF)

#define GMAC_MII_CTRL_ADDR		0x18002000
#define GMAC_MII_CTRL_BYP_SHIFT		10
#define GMAC_MII_CTRL_EXT_SHIFT		9
#define GMAC_MII_DATA_ADDR		0x18002004
#define GMAC_MII_DATA_READ_CMD		0x60020000
#define GMAC_MII_DATA_WRITE_CMD		0x50020000
#define GMAC_MII_BUSY_SHIFT		8
#define GMAC_MII_PHY_ADDR_SHIFT		23
#define GMAC_MII_PHY_REG_SHIFT		18

#define GMAC_RESET_DELAY		2
#define HWRXOFF				30
#define MAXNAMEL			8
#define NUMTXQ				4

int gmac_add(struct eth_device *dev);

#endif /* _BCM_SF2_ETH_GMAC_H_ */
