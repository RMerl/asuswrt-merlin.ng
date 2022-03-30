/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 * Author: Mark Lee <mark-mc.lee@mediatek.com>
 */

#ifndef _MTK_ETH_H_
#define _MTK_ETH_H_

/* Frame Engine Register Bases */
#define PDMA_BASE			0x0800
#define GDMA1_BASE			0x0500
#define GDMA2_BASE			0x1500
#define GMAC_BASE			0x10000

/* Ethernet subsystem registers */

#define ETHSYS_SYSCFG0_REG		0x14
#define SYSCFG0_GE_MODE_S(n)		(12 + ((n) * 2))
#define SYSCFG0_GE_MODE_M		0x3

#define ETHSYS_CLKCFG0_REG		0x2c
#define ETHSYS_TRGMII_CLK_SEL362_5	BIT(11)

/* SYSCFG0_GE_MODE: GE Modes */
#define GE_MODE_RGMII			0
#define GE_MODE_MII			1
#define GE_MODE_MII_PHY			2
#define GE_MODE_RMII			3

/* Frame Engine Registers */

/* PDMA */
#define TX_BASE_PTR_REG(n)		(0x000 + (n) * 0x10)
#define TX_MAX_CNT_REG(n)		(0x004 + (n) * 0x10)
#define TX_CTX_IDX_REG(n)		(0x008 + (n) * 0x10)
#define TX_DTX_IDX_REG(n)		(0x00c + (n) * 0x10)

#define RX_BASE_PTR_REG(n)		(0x100 + (n) * 0x10)
#define RX_MAX_CNT_REG(n)		(0x104 + (n) * 0x10)
#define RX_CRX_IDX_REG(n)		(0x108 + (n) * 0x10)
#define RX_DRX_IDX_REG(n)		(0x10c + (n) * 0x10)

#define PDMA_GLO_CFG_REG		0x204
#define TX_WB_DDONE			BIT(6)
#define RX_DMA_BUSY			BIT(3)
#define RX_DMA_EN			BIT(2)
#define TX_DMA_BUSY			BIT(1)
#define TX_DMA_EN			BIT(0)

#define PDMA_RST_IDX_REG		0x208
#define RST_DRX_IDX0			BIT(16)
#define RST_DTX_IDX0			BIT(0)

/* GDMA */
#define GDMA_IG_CTRL_REG		0x000
#define GDM_ICS_EN			BIT(22)
#define GDM_TCS_EN			BIT(21)
#define GDM_UCS_EN			BIT(20)
#define STRP_CRC			BIT(16)
#define MYMAC_DP_S			12
#define MYMAC_DP_M			0xf000
#define BC_DP_S				8
#define BC_DP_M				0xf00
#define MC_DP_S				4
#define MC_DP_M				0xf0
#define UN_DP_S				0
#define UN_DP_M				0x0f

#define GDMA_MAC_LSB_REG		0x008

#define GDMA_MAC_MSB_REG		0x00c

/* MYMAC_DP/BC_DP/MC_DP/UN_DP: Destination ports */
#define DP_PDMA				0
#define DP_GDMA1			1
#define DP_GDMA2			2
#define DP_PPE				4
#define DP_QDMA				5
#define DP_DISCARD			7

/* GMAC Registers */

#define GMAC_PIAC_REG			0x0004
#define PHY_ACS_ST			BIT(31)
#define MDIO_REG_ADDR_S			25
#define MDIO_REG_ADDR_M			0x3e000000
#define MDIO_PHY_ADDR_S			20
#define MDIO_PHY_ADDR_M			0x1f00000
#define MDIO_CMD_S			18
#define MDIO_CMD_M			0xc0000
#define MDIO_ST_S			16
#define MDIO_ST_M			0x30000
#define MDIO_RW_DATA_S			0
#define MDIO_RW_DATA_M			0xffff

/* MDIO_CMD: MDIO commands */
#define MDIO_CMD_ADDR			0
#define MDIO_CMD_WRITE			1
#define MDIO_CMD_READ			2
#define MDIO_CMD_READ_C45		3

/* MDIO_ST: MDIO start field */
#define MDIO_ST_C45			0
#define MDIO_ST_C22			1

#define GMAC_PORT_MCR(p)		(0x0100 + (p) * 0x100)
#define MAC_RX_PKT_LEN_S		24
#define MAC_RX_PKT_LEN_M		0x3000000
#define IPG_CFG_S			18
#define IPG_CFG_M			0xc0000
#define MAC_MODE			BIT(16)
#define FORCE_MODE			BIT(15)
#define MAC_TX_EN			BIT(14)
#define MAC_RX_EN			BIT(13)
#define BKOFF_EN			BIT(9)
#define BACKPR_EN			BIT(8)
#define FORCE_RX_FC			BIT(5)
#define FORCE_TX_FC			BIT(4)
#define FORCE_SPD_S			2
#define FORCE_SPD_M			0x0c
#define FORCE_DPX			BIT(1)
#define FORCE_LINK			BIT(0)

/* MAC_RX_PKT_LEN: Max RX packet length */
#define MAC_RX_PKT_LEN_1518		0
#define MAC_RX_PKT_LEN_1536		1
#define MAC_RX_PKT_LEN_1552		2
#define MAC_RX_PKT_LEN_JUMBO		3

/* FORCE_SPD: Forced link speed */
#define SPEED_10M			0
#define SPEED_100M			1
#define SPEED_1000M			2

#define GMAC_TRGMII_RCK_CTRL		0x300
#define RX_RST				BIT(31)
#define RXC_DQSISEL			BIT(30)

#define GMAC_TRGMII_TD_ODT(n)		(0x354 + (n) * 8)
#define TD_DM_DRVN_S			4
#define TD_DM_DRVN_M			0xf0
#define TD_DM_DRVP_S			0
#define TD_DM_DRVP_M			0x0f

/* MT7530 Registers */

#define PCR_REG(p)			(0x2004 + (p) * 0x100)
#define PORT_MATRIX_S			16
#define PORT_MATRIX_M			0xff0000

#define PVC_REG(p)			(0x2010 + (p) * 0x100)
#define STAG_VPID_S			16
#define STAG_VPID_M			0xffff0000
#define VLAN_ATTR_S			6
#define VLAN_ATTR_M			0xc0

/* VLAN_ATTR: VLAN attributes */
#define VLAN_ATTR_USER			0
#define VLAN_ATTR_STACK			1
#define VLAN_ATTR_TRANSLATION		2
#define VLAN_ATTR_TRANSPARENT		3

#define PCMR_REG(p)			(0x3000 + (p) * 0x100)
/* XXX: all fields are defined under GMAC_PORT_MCR */

#define SYS_CTRL_REG			0x7000
#define SW_PHY_RST			BIT(2)
#define SW_SYS_RST			BIT(1)
#define SW_REG_RST			BIT(0)

#define NUM_TRGMII_CTRL			5

#define HWTRAP_REG			0x7800
#define MHWTRAP_REG			0x7804
#define CHG_TRAP			BIT(16)
#define LOOPDET_DIS			BIT(14)
#define P5_INTF_SEL_S			13
#define P5_INTF_SEL_M			0x2000
#define SMI_ADDR_S			11
#define SMI_ADDR_M			0x1800
#define XTAL_FSEL_S			9
#define XTAL_FSEL_M			0x600
#define P6_INTF_DIS			BIT(8)
#define P5_INTF_MODE_S			7
#define P5_INTF_MODE_M			0x80
#define P5_INTF_DIS			BIT(6)
#define C_MDIO_BPS			BIT(5)
#define CHIP_MODE_S			0
#define CHIP_MODE_M			0x0f

/* P5_INTF_SEL: Interface type of Port5 */
#define P5_INTF_SEL_GPHY		0
#define P5_INTF_SEL_GMAC5		1

/* P5_INTF_MODE: Interface mode of Port5 */
#define P5_INTF_MODE_GMII_MII		0
#define P5_INTF_MODE_RGMII		1

#define MT7530_P6ECR			0x7830
#define P6_INTF_MODE_M			0x3
#define P6_INTF_MODE_S			0

/* P6_INTF_MODE: Interface mode of Port6 */
#define P6_INTF_MODE_RGMII		0
#define P6_INTF_MODE_TRGMII		1

#define MT7530_TRGMII_RD(n)		(0x7a10 + (n) * 8)
#define RD_TAP_S			0
#define RD_TAP_M			0x7f

#define MT7530_TRGMII_TD_ODT(n)		(0x7a54 + (n) * 8)
/* XXX: all fields are defined under GMAC_TRGMII_TD_ODT */

/* MT7530 GPHY MDIO Indirect Access Registers */

#define MII_MMD_ACC_CTL_REG		0x0d
#define MMD_CMD_S			14
#define MMD_CMD_M			0xc000
#define MMD_DEVAD_S			0
#define MMD_DEVAD_M			0x1f

/* MMD_CMD: MMD commands */
#define MMD_ADDR			0
#define MMD_DATA			1
#define MMD_DATA_RW_POST_INC		2
#define MMD_DATA_W_POST_INC		3

#define MII_MMD_ADDR_DATA_REG		0x0e

/* MT7530 GPHY MDIO MMD Registers */

#define CORE_PLL_GROUP2			0x401
#define RG_SYSPLL_EN_NORMAL		BIT(15)
#define RG_SYSPLL_VODEN			BIT(14)
#define RG_SYSPLL_POSDIV_S		5
#define RG_SYSPLL_POSDIV_M		0x60

#define CORE_PLL_GROUP4			0x403
#define RG_SYSPLL_DDSFBK_EN		BIT(12)
#define RG_SYSPLL_BIAS_EN		BIT(11)
#define RG_SYSPLL_BIAS_LPF_EN		BIT(10)

#define CORE_PLL_GROUP5			0x404
#define RG_LCDDS_PCW_NCPO1_S		0
#define RG_LCDDS_PCW_NCPO1_M		0xffff

#define CORE_PLL_GROUP6			0x405
#define RG_LCDDS_PCW_NCPO0_S		0
#define RG_LCDDS_PCW_NCPO0_M		0xffff

#define CORE_PLL_GROUP7			0x406
#define RG_LCDDS_PWDB			BIT(15)
#define RG_LCDDS_ISO_EN			BIT(13)
#define RG_LCCDS_C_S			4
#define RG_LCCDS_C_M			0x70
#define RG_LCDDS_PCW_NCPO_CHG		BIT(3)

#define CORE_PLL_GROUP10		0x409
#define RG_LCDDS_SSC_DELTA_S		0
#define RG_LCDDS_SSC_DELTA_M		0xfff

#define CORE_PLL_GROUP11		0x40a
#define RG_LCDDS_SSC_DELTA1_S		0
#define RG_LCDDS_SSC_DELTA1_M		0xfff

#define CORE_GSWPLL_GRP1		0x40d
#define RG_GSWPLL_POSDIV_200M_S		12
#define RG_GSWPLL_POSDIV_200M_M		0x3000
#define RG_GSWPLL_EN_PRE		BIT(11)
#define RG_GSWPLL_FBKDIV_200M_S		0
#define RG_GSWPLL_FBKDIV_200M_M		0xff

#define CORE_GSWPLL_GRP2		0x40e
#define RG_GSWPLL_POSDIV_500M_S		8
#define RG_GSWPLL_POSDIV_500M_M		0x300
#define RG_GSWPLL_FBKDIV_500M_S		0
#define RG_GSWPLL_FBKDIV_500M_M		0xff

#define CORE_TRGMII_GSW_CLK_CG		0x410
#define REG_GSWCK_EN			BIT(0)
#define REG_TRGMIICK_EN			BIT(1)

#endif /* _MTK_ETH_H_ */
