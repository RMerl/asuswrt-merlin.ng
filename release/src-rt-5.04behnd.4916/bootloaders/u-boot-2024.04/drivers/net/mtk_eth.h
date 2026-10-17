/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 * Author: Mark Lee <mark-mc.lee@mediatek.com>
 */

#ifndef _MTK_ETH_H_
#define _MTK_ETH_H_

#include <linux/bitops.h>
#include <linux/bitfield.h>

enum mkt_eth_capabilities {
	MTK_TRGMII_BIT,
	MTK_TRGMII_MT7621_CLK_BIT,
	MTK_U3_COPHY_V2_BIT,
	MTK_INFRA_BIT,
	MTK_NETSYS_V2_BIT,
	MTK_NETSYS_V3_BIT,

	/* PATH BITS */
	MTK_ETH_PATH_GMAC1_TRGMII_BIT,
	MTK_ETH_PATH_GMAC2_SGMII_BIT,
};

#define MTK_TRGMII			BIT(MTK_TRGMII_BIT)
#define MTK_TRGMII_MT7621_CLK		BIT(MTK_TRGMII_MT7621_CLK_BIT)
#define MTK_U3_COPHY_V2			BIT(MTK_U3_COPHY_V2_BIT)
#define MTK_INFRA			BIT(MTK_INFRA_BIT)
#define MTK_NETSYS_V2			BIT(MTK_NETSYS_V2_BIT)
#define MTK_NETSYS_V3			BIT(MTK_NETSYS_V3_BIT)

/* Supported path present on SoCs */
#define MTK_ETH_PATH_GMAC1_TRGMII	BIT(MTK_ETH_PATH_GMAC1_TRGMII_BIT)

#define MTK_ETH_PATH_GMAC2_SGMII	BIT(MTK_ETH_PATH_GMAC2_SGMII_BIT)

#define MTK_GMAC1_TRGMII	(MTK_ETH_PATH_GMAC1_TRGMII | MTK_TRGMII)

#define MTK_GMAC2_U3_QPHY	(MTK_ETH_PATH_GMAC2_SGMII | MTK_U3_COPHY_V2 | MTK_INFRA)

#define MTK_HAS_CAPS(caps, _x)		(((caps) & (_x)) == (_x))

#define MT7621_CAPS  (MTK_GMAC1_TRGMII | MTK_TRGMII_MT7621_CLK)

#define MT7623_CAPS  (MTK_GMAC1_TRGMII)

#define MT7981_CAPS  (MTK_GMAC2_U3_QPHY | MTK_NETSYS_V2)

#define MT7986_CAPS  (MTK_NETSYS_V2)

#define MT7988_CAPS  (MTK_NETSYS_V3 | MTK_INFRA)

/* Frame Engine Register Bases */
#define PDMA_V1_BASE			0x0800
#define PDMA_V2_BASE			0x6000
#define PDMA_V3_BASE			0x6800
#define GDMA1_BASE			0x0500
#define GDMA2_BASE			0x1500
#define GDMA3_BASE			0x0540
#define GMAC_BASE			0x10000
#define GSW_BASE			0x20000

/* Ethernet subsystem registers */

#define ETHSYS_SYSCFG0_REG		0x14
#define SYSCFG0_GE_MODE_S(n)		(12 + ((n) * 2))
#define SYSCFG0_GE_MODE_M		0x3
#define SYSCFG0_SGMII_SEL_M		(0x3 << 8)
#define SYSCFG0_SGMII_SEL(gmac)		((!(gmac)) ? BIT(9) : BIT(8))

#define ETHSYS_CLKCFG0_REG		0x2c
#define ETHSYS_TRGMII_CLK_SEL362_5	BIT(11)

/* Top misc registers */
#define TOPMISC_NETSYS_PCS_MUX		0x84
#define NETSYS_PCS_MUX_MASK		GENMASK(1, 0)
#define MUX_G2_USXGMII_SEL		BIT(1)
#define MUX_HSGMII1_G1_SEL		BIT(0)

#define USB_PHY_SWITCH_REG		0x218
#define QPHY_SEL_MASK			0x3
#define SGMII_QPHY_SEL			0x2

/* SYSCFG0_GE_MODE: GE Modes */
#define GE_MODE_RGMII			0
#define GE_MODE_MII			1
#define GE_MODE_MII_PHY			2
#define GE_MODE_RMII			3

/* SGMII subsystem config registers */
#define SGMSYS_PCS_CONTROL_1		0x0
#define SGMII_LINK_STATUS		BIT(18)
#define SGMII_AN_ENABLE			BIT(12)
#define SGMII_AN_RESTART		BIT(9)

#define SGMSYS_SGMII_MODE		0x20
#define SGMII_AN_MODE			0x31120103
#define SGMII_FORCE_MODE		0x31120019

#define SGMSYS_QPHY_PWR_STATE_CTRL	0xe8
#define SGMII_PHYA_PWD			BIT(4)

#define SGMSYS_QPHY_WRAP_CTRL		0xec
#define SGMII_PN_SWAP_TX_RX		0x03

#define SGMSYS_GEN2_SPEED		0x2028
#define SGMSYS_GEN2_SPEED_V2		0x128
#define SGMSYS_SPEED_2500		BIT(2)

/* USXGMII subsystem config registers */
/* Register to control USXGMII XFI PLL digital */
#define XFI_PLL_DIG_GLB8		0x08
#define RG_XFI_PLL_EN			BIT(31)

/* Register to control USXGMII XFI PLL analog */
#define XFI_PLL_ANA_GLB8		0x108
#define RG_XFI_PLL_ANA_SWWA		0x02283248

/* Frame Engine Registers */
#define PSE_NO_DROP_CFG_REG		0x108
#define PSE_NO_DROP_GDM1		BIT(1)

#define FE_GLO_MISC_REG			0x124
#define PDMA_VER_V2			BIT(4)

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

#define GDMA_EG_CTRL_REG		0x004
#define GDMA_CPU_BRIDGE_EN		BIT(31)

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

#define GMAC_XGMAC_STS_REG		0x000c
#define P1_XGMAC_FORCE_LINK		BIT(15)

#define GMAC_MAC_MISC_REG		0x0010

#define GMAC_GSW_CFG_REG		0x0080
#define GSWTX_IPG_M			0xF0000
#define GSWTX_IPG_S			16
#define GSWRX_IPG_M			0xF
#define GSWRX_IPG_S			0

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
#define DEL_RXFIFO_CLR			BIT(12)
#define BKOFF_EN			BIT(9)
#define BACKPR_EN			BIT(8)
#define FORCE_RX_FC			BIT(5)
#define FORCE_TX_FC			BIT(4)
#define FORCE_SPD_S			2
#define FORCE_SPD_M			0x0c
#define FORCE_DPX			BIT(1)
#define FORCE_LINK			BIT(0)

/* Values of IPG_CFG */
#define IPG_96BIT			0
#define IPG_96BIT_WITH_SHORT_IPG	1
#define IPG_64BIT			2

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

/* XGMAC Status Registers */
#define XGMAC_STS(x)			(((x) == 2) ? 0x001C : 0x000C)
#define XGMAC_FORCE_LINK		BIT(15)

/* XGMAC Registers */
#define XGMAC_PORT_MCR(x)		(0x2000 + (((x) - 1) * 0x1000))
#define XGMAC_TRX_DISABLE		0xf
#define XGMAC_FORCE_TX_FC		BIT(5)
#define XGMAC_FORCE_RX_FC		BIT(4)

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

#define PMCR_REG(p)			(0x3000 + (p) * 0x100)
/* XXX: all fields of MT7530 are defined under GMAC_PORT_MCR
 * MT7531 specific fields are defined below
 */
#define FORCE_MODE_EEE1G		BIT(25)
#define FORCE_MODE_EEE100		BIT(26)
#define FORCE_MODE_TX_FC		BIT(27)
#define FORCE_MODE_RX_FC		BIT(28)
#define FORCE_MODE_DPX			BIT(29)
#define FORCE_MODE_SPD			BIT(30)
#define FORCE_MODE_LNK			BIT(31)
#define MT7531_FORCE_MODE		FORCE_MODE_EEE1G | FORCE_MODE_EEE100 |\
					FORCE_MODE_TX_FC | FORCE_MODE_RX_FC | \
					FORCE_MODE_DPX   | FORCE_MODE_SPD | \
					FORCE_MODE_LNK
#define MT7988_FORCE_MODE		FORCE_MODE_TX_FC | FORCE_MODE_RX_FC | \
					FORCE_MODE_DPX   | FORCE_MODE_SPD | \
					FORCE_MODE_LNK

/* MT7531 SGMII Registers */
#define MT7531_SGMII_REG_BASE		0x5000
#define MT7531_SGMII_REG_PORT_BASE	0x1000
#define MT7531_SGMII_REG(p, r)		(MT7531_SGMII_REG_BASE + \
					(p) * MT7531_SGMII_REG_PORT_BASE + (r))
#define MT7531_PCS_CONTROL_1(p)		MT7531_SGMII_REG(((p) - 5), 0x00)
#define MT7531_SGMII_MODE(p)		MT7531_SGMII_REG(((p) - 5), 0x20)
#define MT7531_QPHY_PWR_STATE_CTRL(p)	MT7531_SGMII_REG(((p) - 5), 0xe8)
#define MT7531_PHYA_CTRL_SIGNAL3(p)	MT7531_SGMII_REG(((p) - 5), 0x128)
/* XXX: all fields of MT7531 SGMII  are defined under SGMSYS */

/* MT753x System Control Register */
#define SYS_CTRL_REG			0x7000
#define SW_PHY_RST			BIT(2)
#define SW_SYS_RST			BIT(1)
#define SW_REG_RST			BIT(0)

/* MT7531  */
#define MT7531_PHY_IAC			0x701c
/* XXX: all fields are defined under GMAC_PIAC_REG */

#define MT7531_CLKGEN_CTRL		0x7500
#define CLK_SKEW_OUT_S			8
#define CLK_SKEW_OUT_M			0x300
#define CLK_SKEW_IN_S			6
#define CLK_SKEW_IN_M			0xc0
#define RXCLK_NO_DELAY			BIT(5)
#define TXCLK_NO_REVERSE		BIT(4)
#define GP_MODE_S			1
#define GP_MODE_M			0x06
#define GP_CLK_EN			BIT(0)

/* Values of GP_MODE */
#define GP_MODE_RGMII			0
#define GP_MODE_MII			1
#define GP_MODE_REV_MII			2

/* Values of CLK_SKEW_IN */
#define CLK_SKEW_IN_NO_CHANGE		0
#define CLK_SKEW_IN_DELAY_100PPS	1
#define CLK_SKEW_IN_DELAY_200PPS	2
#define CLK_SKEW_IN_REVERSE		3

/* Values of CLK_SKEW_OUT */
#define CLK_SKEW_OUT_NO_CHANGE		0
#define CLK_SKEW_OUT_DELAY_100PPS	1
#define CLK_SKEW_OUT_DELAY_200PPS	2
#define CLK_SKEW_OUT_REVERSE		3

#define HWTRAP_REG			0x7800
/* MT7530 Modified Hardware Trap Status Registers */
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

#define NUM_TRGMII_CTRL			5

#define MT7530_TRGMII_RD(n)		(0x7a10 + (n) * 8)
#define RD_TAP_S			0
#define RD_TAP_M			0x7f

#define MT7530_TRGMII_TD_ODT(n)		(0x7a54 + (n) * 8)
/* XXX: all fields are defined under GMAC_TRGMII_TD_ODT */

/* TOP Signals Status Register */
#define MT7531_TOP_SIG_SR		0x780c
#define PAD_MCM_SMI_EN			BIT(0)
#define PAD_DUAL_SGMII_EN		BIT(1)

/* MT7531 PLLGP Registers */
#define MT7531_PLLGP_EN			0x7820
#define EN_COREPLL			BIT(2)
#define SW_CLKSW			BIT(1)
#define SW_PLLGP			BIT(0)

#define MT7531_PLLGP_CR0		0x78a8
#define RG_COREPLL_EN			BIT(22)
#define RG_COREPLL_POSDIV_S		23
#define RG_COREPLL_POSDIV_M		0x3800000
#define RG_COREPLL_SDM_PCW_S		1
#define RG_COREPLL_SDM_PCW_M		0x3ffffe
#define RG_COREPLL_SDM_PCW_CHG		BIT(0)

/* MT7531 RGMII and SGMII PLL clock */
#define MT7531_ANA_PLLGP_CR2		0x78b0
#define MT7531_ANA_PLLGP_CR5		0x78bc

/* MT7531 GPIO GROUP IOLB SMT0 Control */
#define MT7531_SMT0_IOLB		0x7f04
#define SMT_IOLB_5_SMI_MDC_EN		BIT(5)

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
#define MT7531_BYPASS_MODE		BIT(4)
#define MT7531_POWER_ON_OFF		BIT(5)
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

/* Extend PHY Control Register 3 */
#define PHY_EXT_REG_14			0x14

/* Fields of PHY_EXT_REG_14 */
#define PHY_EN_DOWN_SHFIT		BIT(4)

/* Extend PHY Control Register 4 */
#define PHY_EXT_REG_17			0x17

/* Fields of PHY_EXT_REG_17 */
#define PHY_LINKDOWN_POWER_SAVING_EN	BIT(4)

/* PHY RXADC Control Register 7 */
#define PHY_DEV1E_REG_0C6		0x0c6

/* Fields of PHY_DEV1E_REG_0C6 */
#define PHY_POWER_SAVING_S		8
#define PHY_POWER_SAVING_M		0x300
#define PHY_POWER_SAVING_TX		0x0

/* PDMA descriptors */
struct mtk_rx_dma {
	unsigned int rxd1;
	unsigned int rxd2;
	unsigned int rxd3;
	unsigned int rxd4;
} __packed __aligned(4);

struct mtk_rx_dma_v2 {
	unsigned int rxd1;
	unsigned int rxd2;
	unsigned int rxd3;
	unsigned int rxd4;
	unsigned int rxd5;
	unsigned int rxd6;
	unsigned int rxd7;
	unsigned int rxd8;
} __packed __aligned(4);

struct mtk_tx_dma {
	unsigned int txd1;
	unsigned int txd2;
	unsigned int txd3;
	unsigned int txd4;
} __packed __aligned(4);

struct mtk_tx_dma_v2 {
	unsigned int txd1;
	unsigned int txd2;
	unsigned int txd3;
	unsigned int txd4;
	unsigned int txd5;
	unsigned int txd6;
	unsigned int txd7;
	unsigned int txd8;
} __packed __aligned(4);

/* PDMA TXD fields */
#define PDMA_TXD2_DDONE			BIT(31)
#define PDMA_TXD2_LS0			BIT(30)
#define PDMA_V1_TXD2_SDL0_M		GENMASK(29, 16)
#define PDMA_V1_TXD2_SDL0_SET(_v)	FIELD_PREP(PDMA_V1_TXD2_SDL0_M, (_v))
#define PDMA_V2_TXD2_SDL0_M		GENMASK(23, 8)
#define PDMA_V2_TXD2_SDL0_SET(_v)	FIELD_PREP(PDMA_V2_TXD2_SDL0_M, (_v))

#define PDMA_V1_TXD4_FPORT_M		GENMASK(27, 25)
#define PDMA_V1_TXD4_FPORT_SET(_v)	FIELD_PREP(PDMA_V1_TXD4_FPORT_M, (_v))
#define PDMA_V2_TXD4_FPORT_M		GENMASK(27, 24)
#define PDMA_V2_TXD4_FPORT_SET(_v)	FIELD_PREP(PDMA_V2_TXD4_FPORT_M, (_v))

#define PDMA_V2_TXD5_FPORT_M		GENMASK(19, 16)
#define PDMA_V2_TXD5_FPORT_SET(_v)	FIELD_PREP(PDMA_V2_TXD5_FPORT_M, (_v))

/* PDMA RXD fields */
#define PDMA_RXD2_DDONE			BIT(31)
#define PDMA_RXD2_LS0			BIT(30)
#define PDMA_V1_RXD2_PLEN0_M		GENMASK(29, 16)
#define PDMA_V1_RXD2_PLEN0_GET(_v)	FIELD_GET(PDMA_V1_RXD2_PLEN0_M, (_v))
#define PDMA_V1_RXD2_PLEN0_SET(_v)	FIELD_PREP(PDMA_V1_RXD2_PLEN0_M, (_v))
#define PDMA_V2_RXD2_PLEN0_M		GENMASK(23, 8)
#define PDMA_V2_RXD2_PLEN0_GET(_v)	FIELD_GET(PDMA_V2_RXD2_PLEN0_M, (_v))
#define PDMA_V2_RXD2_PLEN0_SET(_v)	FIELD_PREP(PDMA_V2_RXD2_PLEN0_M, (_v))

#endif /* _MTK_ETH_H_ */
