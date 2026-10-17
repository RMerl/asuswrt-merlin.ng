// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <cpu_func.h>
#include <dm.h>
#include <clk.h>
#include <malloc.h>
#include <miiphy.h>
#include <misc.h>
#include <net.h>
#include <reset.h>
#include <asm/addrspace.h>
#include <asm/cache.h>
#include <asm/gpio.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/ethtool.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/mdio.h>
#include <linux/mii.h>
#include <mach/mt7620-sysc.h>

/* Frame Engine block */
#define GDMA_BASE			0x600
#define PDMA_BASE			0x800

/* GDMA registers */
#define GDMA_FWD_CFG			0x00
#define GDMA_DST_PORT			GENMASK(2, 0)
#define   GDMA_DST_PORT_CPU		0

#define GDMA_MAC_ADRL			0x08
#define GDMA_MAC_ADRH			0x0c

/* PDMA registers */
#define TX_BASE_PTR0			0x000
#define TX_MAX_CNT0			0x004
#define TX_CTX_IDX0			0x008
#define TX_DTX_IDX0			0x00c
#define RX_BASE_PTR0			0x100
#define RX_MAX_CNT0			0x104
#define RX_CALC_IDX0			0x108
#define RX_DRX_IDX0			0x10c

#define PDMA_GLO_CFG			0x204
#define TX_WB_DDONE			BIT(6)
#define PDMA_BT_SIZE			GENMASK(5, 4)
#define   PDMA_BT_SIZE_32B		1
#define RX_DMA_BUSY			BIT(3)
#define RX_DMA_EN			BIT(2)
#define TX_DMA_BUSY			BIT(1)
#define TX_DMA_EN			BIT(0)

#define PDMA_RST_IDX			0x208
#define RST_DRX_IDX0			BIT(16)
#define RST_DTX_IDX0			BIT(0)

/* Built-in giga ethernet switch block */

/* ARL registers */
#define GSW_MFC				0x0010
#define BC_FFP				GENMASK(31, 24)
#define UNM_FFP				GENMASK(23, 16)
#define UNU_FFP				GENMASK(15, 8)
#define CPU_EN				BIT(7)
#define CPU_PORT			GENMASK(6, 4)

/* Port registers */
#define GSW_PCR(p)			(0x2004 + (p) * 0x100)
#define PORT_MATRIX			GENMASK(23, 16)

#define GSW_PVC(p)			(0x2010 + (p) * 0x100)
#define STAG_VPID			GENMASK(31, 16)
#define VLAN_ATTR			GENMASK(7, 6)
#define   VLAN_ATTR_USER		0

/* MAC registers */
#define GSW_PMCR(p)			(0x3000 + (p) * 0x100)
#define IPG_CFG				GENMASK(19, 18)
#define IPG_96BIT_WITH_SHORT_IPG	1
#define MAC_MODE			BIT(16)
#define FORCE_MODE			BIT(15)
#define MAC_TX_EN			BIT(14)
#define MAC_RX_EN			BIT(13)
#define BKOFF_EN			BIT(9)
#define BACKPR_EN			BIT(8)
#define FORCE_EEE1G			BIT(7)
#define FORCE_EEE100			BIT(6)
#define FORCE_RX_FC			BIT(5)
#define FORCE_TX_FC			BIT(4)
#define FORCE_SPEED			GENMASK(3, 2)
#define   FORCE_SPEED_1000		2
#define   FORCE_SPEED_100		1
#define   FORCE_SPEED_10		0
#define FORCE_DUPLEX			BIT(1)
#define FORCE_LINK			BIT(0)

/* GMAC registers */
#define GSW_PPSC			0x7000
#define PHY_AP_EN			BIT(31)
#define PHY_PRE_EN			BIT(30)
#define PHY_MDC_CFG			GENMASK(29, 24)
#define EPHY_AP_EN			BIT(23)
#define EE_AN_EN			BIT(16)
#define PHY_AP_END_ADDR			GENMASK(12, 8)
#define PHY_AP_START_ADDR		GENMASK(4, 0)

#define GSW_PIAC			0x7004
#define PHY_ACS_ST			BIT(31)
#define MDIO_REG_ADDR			GENMASK(29, 25)
#define MDIO_PHY_ADDR			GENMASK(24, 20)
#define MDIO_CMD			GENMASK(19, 18)
#define   MDIO_CMD_WRITE		1
#define   MDIO_CMD_READ			2
#define MDIO_ST				GENMASK(17, 16)
#define MDIO_RW_DATA			GENMASK(15, 0)

#define GSW_GPC1			0x7014
#define PHY_DIS				GENMASK(28, 24)
#define PHY_BASE			GENMASK(20, 16)
#define TX_CLK_MODE			BIT(3)
#define RX_CLK_MODE			BIT(2)

/* MII Registers for MDIO clause 45 indirect access */
#define MII_MMD_ACC_CTL_REG		0x0d
#define MMD_OP_MODE			GENMASK(15, 14)
#define   MMD_ADDR			0
#define   MMD_DATA			1
#define   MMD_DATA_RW_POST_INC		2
#define   MMD_DATA_W_POST_INC		3
#define MMD_DEVAD			GENMASK(4, 0)

#define MII_MMD_ADDR_DATA_REG		0x0e

/* MT7530 internal register access */
#define MT7530_REG_PAGE_ADDR		GENMASK(15, 6)
#define MT7530_REG_ADDR			GENMASK(5, 2)

/* MT7530 system control registers*/
#define MT7530_SYS_CTRL			0x7000
#define SW_SYS_RST			BIT(1)
#define SW_REG_RST			BIT(0)

#define MT7530_MHWTRAP			0x7804
#define P5_INTF_SEL_GMAC5		BIT(13)
#define P5_INTF_DIS			BIT(6)

struct pdma_txd_info1 {
	u32 SDP0;
};

struct pdma_txd_info2 {
	u32 SDL1 : 14;
	u32 LS1 : 1;
	u32 BURST : 1;
	u32 SDL0 : 14;
	u32 LS0 : 1;
	u32 DDONE : 1;
};

struct pdma_txd_info3 {
	u32 SDP1;
};

struct pdma_txd_info4 {
	u32 VPRI_VIDX : 8;
	u32 SIDX : 4;
	u32 INSP : 1;
	u32 RESV : 2;
	u32 UDF : 5;
	u32 FP_BMAP : 8;
	u32 TSO : 1;
	u32 TUI_CO : 3;
};

struct pdma_tx_desc {
	struct pdma_txd_info1 txd_info1;
	struct pdma_txd_info2 txd_info2;
	struct pdma_txd_info3 txd_info3;
	struct pdma_txd_info4 txd_info4;
};

struct pdma_rxd_info1 {
	u32 PDP0;
};

struct pdma_rxd_info2 {
	u32 PLEN1 : 14;
	u32 LS1 : 1;
	u32 UN_USED : 1;
	u32 PLEN0 : 14;
	u32 LS0 : 1;
	u32 DDONE : 1;
};

struct pdma_rxd_info3 {
	u32 PDP1;
};

struct pdma_rxd_info4 {
	u32 FOE_ENTRY : 14;
	u32 CRSN : 5;
	u32 SP : 3;
	u32 L4F : 1;
	u32 L4VLD : 1;
	u32 TACK : 1;
	u32 IP4F : 1;
	u32 IP4 : 1;
	u32 IP6 : 1;
	u32 UN_USED : 4;
};

struct pdma_rx_desc {
	struct pdma_rxd_info1 rxd_info1;
	struct pdma_rxd_info2 rxd_info2;
	struct pdma_rxd_info3 rxd_info3;
	struct pdma_rxd_info4 rxd_info4;
};

struct mt7620_gsw_port_cfg {
	phy_interface_t mode;
	bool force_mode;
	bool duplex;
	u32 speed;
	int phy_addr;
};

struct mt7620_eth_priv {
	struct udevice *dev;

	void __iomem *fe_base;
	void __iomem *gsw_base;

	struct mii_dev *mdio_bus;

	struct pdma_tx_desc *tx_ring_noc;
	struct pdma_rx_desc *rx_ring_noc;

	int rx_dma_owner_idx0;
	int tx_cpu_owner_idx0;

	void *pkt_buf;
	void *tx_ring;
	void *rx_ring;

	struct reset_ctl_bulk rsts;
	struct clk_bulk clks;

	struct udevice *sysc;

	u32 ephy_num;
	bool port5_mt7530;
	struct gpio_desc gpio_swrst;
	struct mt7620_gsw_port_cfg port_cfg[3];
};

#define PDMA_TIMEOUT			100000

#define NUM_TX_DESC			64
#define NUM_RX_DESC			128
#define NUM_FE_PHYS			5
#define NUM_PORTS			7
#define CPU_PORT_NUM			6

#define NUM_MT7530_PHYS			5

static void pdma_write(struct mt7620_eth_priv *priv, u32 reg, u32 val)
{
	writel(val, priv->fe_base + PDMA_BASE + reg);
}

static void gdma_write(struct mt7620_eth_priv *priv, u32 reg, u32 val)
{
	writel(val, priv->fe_base + GDMA_BASE + reg);
}

static void gdma_rmw(struct mt7620_eth_priv *priv, u32 reg, u32 clr, u32 set)
{
	clrsetbits_le32(priv->fe_base + GDMA_BASE + reg, clr, set);
}

static u32 gsw_read(struct mt7620_eth_priv *priv, u32 reg)
{
	return readl(priv->gsw_base + reg);
}

static void gsw_write(struct mt7620_eth_priv *priv, u32 reg, u32 val)
{
	writel(val, priv->gsw_base + reg);
}

static void gsw_rmw(struct mt7620_eth_priv *priv, u32 reg, u32 clr, u32 set)
{
	clrsetbits_le32(priv->gsw_base + reg, clr, set);
}

static int mt7620_mdio_rw(struct mt7620_eth_priv *priv, u32 phy, u32 reg,
			  u32 data, u32 cmd)
{
	int ret;
	u32 val;

	val = FIELD_PREP(MDIO_ST, 1) | FIELD_PREP(MDIO_CMD, cmd) |
	      FIELD_PREP(MDIO_PHY_ADDR, phy) |
	      FIELD_PREP(MDIO_REG_ADDR, reg);

	if (cmd == MDIO_CMD_WRITE)
		val |= FIELD_PREP(MDIO_RW_DATA, data);

	gsw_write(priv, GSW_PIAC, val);
	gsw_write(priv, GSW_PIAC, val | PHY_ACS_ST);

	ret = readl_poll_timeout(priv->gsw_base + GSW_PIAC, val,
				 !(val & PHY_ACS_ST), 10000);
	if (ret) {
		dev_err(priv->dev, "mt7620_eth: MDIO access timeout\n");
		return ret;
	}

	if (cmd == MDIO_CMD_READ) {
		val = gsw_read(priv, GSW_PIAC);
		return FIELD_GET(MDIO_RW_DATA, val);
	}

	return 0;
}

static int mt7620_mii_read(struct mt7620_eth_priv *priv, u32 phy, u32 reg)
{
	return mt7620_mdio_rw(priv, phy, reg, 0, MDIO_CMD_READ);
}

static int mt7620_mii_write(struct mt7620_eth_priv *priv, u32 phy, u32 reg,
			    u16 val)
{
	return mt7620_mdio_rw(priv, phy, reg, val, MDIO_CMD_WRITE);
}

static int mt7620_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct mt7620_eth_priv *priv = bus->priv;
	int ret;

	if (devad < 0)
		return mt7620_mdio_rw(priv, addr, reg, 0, MDIO_CMD_READ);

	ret = mt7620_mdio_rw(priv, addr, MII_MMD_ACC_CTL_REG,
			     FIELD_PREP(MMD_OP_MODE, MMD_ADDR) |
			     FIELD_PREP(MMD_DEVAD, devad), MDIO_CMD_WRITE);
	if (ret)
		return ret;

	ret = mt7620_mdio_rw(priv, addr, MII_MMD_ADDR_DATA_REG, reg,
			     MDIO_CMD_WRITE);
	if (ret)
		return ret;

	ret = mt7620_mdio_rw(priv, addr, MII_MMD_ACC_CTL_REG,
			     FIELD_PREP(MMD_OP_MODE, MMD_DATA) |
			     FIELD_PREP(MMD_DEVAD, devad), MDIO_CMD_WRITE);
	if (ret)
		return ret;

	return mt7620_mdio_rw(priv, addr, MII_MMD_ADDR_DATA_REG, 0,
			      MDIO_CMD_READ);
}

static int mt7620_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			     u16 val)
{
	struct mt7620_eth_priv *priv = bus->priv;
	int ret;

	if (devad < 0)
		return mt7620_mdio_rw(priv, addr, reg, val, MDIO_CMD_WRITE);

	ret = mt7620_mdio_rw(priv, addr, MII_MMD_ACC_CTL_REG,
			     FIELD_PREP(MMD_OP_MODE, MMD_ADDR) |
			     FIELD_PREP(MMD_DEVAD, devad), MDIO_CMD_WRITE);
	if (ret)
		return ret;

	ret = mt7620_mdio_rw(priv, addr, MII_MMD_ADDR_DATA_REG, reg,
			     MDIO_CMD_WRITE);
	if (ret)
		return ret;

	ret = mt7620_mdio_rw(priv, addr, MII_MMD_ACC_CTL_REG,
			     FIELD_PREP(MMD_OP_MODE, MMD_DATA) |
			     FIELD_PREP(MMD_DEVAD, devad), MDIO_CMD_WRITE);
	if (ret)
		return ret;

	return mt7620_mdio_rw(priv, addr, MII_MMD_ADDR_DATA_REG, val,
			      MDIO_CMD_WRITE);
}

static int mt7620_mdio_register(struct udevice *dev)
{
	struct mt7620_eth_priv *priv = dev_get_priv(dev);
	struct mii_dev *mdio_bus = mdio_alloc();
	int ret;

	if (!mdio_bus)
		return -ENOMEM;

	mdio_bus->read = mt7620_mdio_read;
	mdio_bus->write = mt7620_mdio_write;
	snprintf(mdio_bus->name, sizeof(mdio_bus->name), dev->name);

	mdio_bus->priv = (void *)priv;

	ret = mdio_register(mdio_bus);

	if (ret)
		return ret;

	priv->mdio_bus = mdio_bus;

	return 0;
}

static int mt7530_reg_read(struct mt7620_eth_priv *priv, u32 reg, u32 *data)
{
	int ret, low_word, high_word;

	/* Write page address */
	ret = mt7620_mii_write(priv, 0x1f, 0x1f,
			       FIELD_GET(MT7530_REG_PAGE_ADDR, reg));
	if (ret)
		return ret;

	/* Read low word */
	low_word = mt7620_mii_read(priv, 0x1f, FIELD_GET(MT7530_REG_ADDR, reg));
	if (low_word < 0)
		return low_word;

	/* Read high word */
	high_word = mt7620_mii_read(priv, 0x1f, 0x10);
	if (high_word < 0)
		return high_word;

	if (data)
		*data = ((u32)high_word << 16) | ((u32)low_word & 0xffff);

	return 0;
}

static int mt7530_reg_write(struct mt7620_eth_priv *priv, u32 reg, u32 data)
{
	int ret;

	/* Write page address */
	ret = mt7620_mii_write(priv, 0x1f, 0x1f,
			       FIELD_GET(MT7530_REG_PAGE_ADDR, reg));
	if (ret)
		return ret;

	/* Write low word */
	ret = mt7620_mii_write(priv, 0x1f, FIELD_GET(MT7530_REG_ADDR, reg),
			       data & 0xffff);
	if (ret)
		return ret;

	/* Write high word */
	return mt7620_mii_write(priv, 0x1f, 0x10, data >> 16);
}

static void mt7620_phy_restart_an(struct mt7620_eth_priv *priv, u32 phy)
{
	u16 val;

	val = mt7620_mii_read(priv, phy, MII_BMCR);
	val |= BMCR_ANRESTART;
	mt7620_mii_write(priv, phy, MII_BMCR, val);
}

static void mt7620_gsw_ephy_init(struct mt7620_eth_priv *priv)
{
	struct mt7620_sysc_chip_rev chip_rev;
	int ret;
	u32 i;

	ret = misc_ioctl(priv->sysc, MT7620_SYSC_IOCTL_GET_CHIP_REV, &chip_rev);
	if (ret) {
		/* Assume MT7620A if misc_ioctl() failed */
		dev_warn(priv->dev, "mt7620_eth: failed to get chip rev\n");
		chip_rev.bga = 1;
	}

	 /* global, page 4 */
	mt7620_mii_write(priv, 1, 31, 0x4000);
	mt7620_mii_write(priv, 1, 17, 0x7444);

	if (chip_rev.bga)
		mt7620_mii_write(priv, 1, 19, 0x0114);
	else
		mt7620_mii_write(priv, 1, 19, 0x0117);

	mt7620_mii_write(priv, 1, 22, 0x10cf);
	mt7620_mii_write(priv, 1, 25, 0x6212);
	mt7620_mii_write(priv, 1, 26, 0x0777);
	mt7620_mii_write(priv, 1, 29, 0x4000);
	mt7620_mii_write(priv, 1, 28, 0xc077);
	mt7620_mii_write(priv, 1, 24, 0x0000);

	/* global, page 3 */
	mt7620_mii_write(priv, 1, 31, 0x3000);
	mt7620_mii_write(priv, 1, 17, 0x4838);

	/* global, page 2 */
	mt7620_mii_write(priv, 1, 31, 0x2000);

	if (chip_rev.bga) {
		mt7620_mii_write(priv, 1, 21, 0x0515);
		mt7620_mii_write(priv, 1, 22, 0x0053);
		mt7620_mii_write(priv, 1, 23, 0x00bf);
		mt7620_mii_write(priv, 1, 24, 0x0aaf);
		mt7620_mii_write(priv, 1, 25, 0x0fad);
		mt7620_mii_write(priv, 1, 26, 0x0fc1);
	} else {
		mt7620_mii_write(priv, 1, 21, 0x0517);
		mt7620_mii_write(priv, 1, 22, 0x0fd2);
		mt7620_mii_write(priv, 1, 23, 0x00bf);
		mt7620_mii_write(priv, 1, 24, 0x0aab);
		mt7620_mii_write(priv, 1, 25, 0x00ae);
		mt7620_mii_write(priv, 1, 26, 0x0fff);
	}

	/*  global, page 1 */
	mt7620_mii_write(priv, 1, 31, 0x1000);
	mt7620_mii_write(priv, 1, 17, 0xe7f8);

	/* local, page 0 */
	mt7620_mii_write(priv, 1, 31, 0x8000);
	for (i = 0; i < priv->ephy_num; i++)
		mt7620_mii_write(priv, i, 30, 0xa000);

	for (i = 0; i < priv->ephy_num; i++)
		mt7620_mii_write(priv, i, 4, 0x05e1);

	/* local, page 2 */
	mt7620_mii_write(priv, 1, 31, 0xa000);
	mt7620_mii_write(priv, 0, 16, 0x1111);
	mt7620_mii_write(priv, 1, 16, 0x1010);
	mt7620_mii_write(priv, 2, 16, 0x1515);
	mt7620_mii_write(priv, 3, 16, 0x0f0f);
	if (priv->ephy_num == NUM_FE_PHYS)
		mt7620_mii_write(priv, 4, 16, 0x1313);

	/* Restart auto-negotiation */
	for (i = 0; i < priv->ephy_num; i++)
		mt7620_phy_restart_an(priv, i);

	if (priv->port_cfg[0].phy_addr > 0)
		mt7620_phy_restart_an(priv, priv->port_cfg[0].phy_addr);

	if (priv->port_cfg[1].phy_addr > 0)
		mt7620_phy_restart_an(priv, priv->port_cfg[1].phy_addr);
}

static int mt7620_setup_gmac_mode(struct mt7620_eth_priv *priv, u32 gmac,
				  phy_interface_t mode)
{
	enum mt7620_sysc_ge_mode ge_mode;
	unsigned long req;
	int ret;

	switch (gmac) {
	case 1:
		req = MT7620_SYSC_IOCTL_SET_GE1_MODE;
		break;
	case 2:
		req = MT7620_SYSC_IOCTL_SET_GE2_MODE;
		break;
	default:
		/* Should not reach here */
		return -EINVAL;
	}

	switch (mode) {
	case PHY_INTERFACE_MODE_MII:
		ge_mode = MT7620_SYSC_GE_MII;
		break;
	case PHY_INTERFACE_MODE_RMII:
		ge_mode = MT7620_SYSC_GE_RMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
		ge_mode = MT7620_SYSC_GE_RGMII;
		break;
	case PHY_INTERFACE_MODE_NA:
		if (gmac == 2)
			ge_mode = MT7620_SYSC_GE_ESW_PHY;
		else
			ge_mode = MT7620_SYSC_GE_RGMII;
		break;
	default:
		/* Should not reach here */
		return -EINVAL;
	}

	ret = misc_ioctl(priv->sysc, req, &ge_mode);
	if (ret)
		dev_warn(priv->dev, "mt7620_eth: failed to set GE%u mode\n",
			 gmac);

	return 0;
}

static void mt7620_gsw_setup_port(struct mt7620_eth_priv *priv, u32 port,
				  struct mt7620_gsw_port_cfg *port_cfg)
{
	u32 pmcr;

	if (port_cfg->mode == PHY_INTERFACE_MODE_NA) {
		if (port == 5) {
			gsw_write(priv, GSW_PMCR(port), FORCE_MODE);
			return;
		}

		port_cfg->force_mode = port == CPU_PORT_NUM ? true : false;
	}

	pmcr = FIELD_PREP(IPG_CFG, IPG_96BIT_WITH_SHORT_IPG) | MAC_MODE |
	       MAC_TX_EN | MAC_RX_EN | BKOFF_EN | BACKPR_EN;

	if (port_cfg->force_mode) {
		pmcr |= FORCE_MODE | FORCE_RX_FC | FORCE_TX_FC |
			FIELD_PREP(FORCE_SPEED, port_cfg->speed) | FORCE_LINK;

		if (port_cfg->duplex)
			pmcr |= FORCE_DUPLEX;
	}

	gsw_write(priv, GSW_PMCR(port), pmcr);
}

static void mt7620_gsw_set_port_isolation(struct mt7620_eth_priv *priv)
{
	u32 i;

	for (i = 0; i < NUM_PORTS; i++) {
		/* Set port matrix mode */
		if (i != CPU_PORT_NUM)
			gsw_write(priv, GSW_PCR(i),
				  FIELD_PREP(PORT_MATRIX, 0x40));
		else
			gsw_write(priv, GSW_PCR(i),
				  FIELD_PREP(PORT_MATRIX, 0x3f));

		/* Set port mode to user port */
		gsw_write(priv, GSW_PVC(i), FIELD_PREP(STAG_VPID, 0x8100) |
			  FIELD_PREP(VLAN_ATTR, VLAN_ATTR_USER));
	}
}

static void mt7620_gsw_setup_phy_polling(struct mt7620_eth_priv *priv)
{
	int phy_addr_st, phy_addr_end;

	if (priv->port_cfg[0].mode == PHY_INTERFACE_MODE_NA)
		priv->ephy_num = NUM_FE_PHYS;
	else
		priv->ephy_num = NUM_FE_PHYS - 1;

	if (priv->port_cfg[0].phy_addr < 0 && priv->port_cfg[1].phy_addr < 0)
		return;

	if (priv->port_cfg[0].phy_addr > 0 && priv->port_cfg[1].phy_addr > 0) {
		phy_addr_st = priv->port_cfg[0].phy_addr;
		phy_addr_end = priv->port_cfg[1].phy_addr;
	} else if (priv->port_cfg[0].phy_addr > 0) {
		phy_addr_st = priv->port_cfg[0].phy_addr;
		phy_addr_end = priv->port_cfg[0].phy_addr + 1;
	} else {
		phy_addr_st = 4;
		phy_addr_end = priv->port_cfg[1].phy_addr;
	}

	gsw_rmw(priv, GSW_PPSC, PHY_AP_END_ADDR | PHY_AP_START_ADDR,
		PHY_AP_EN | FIELD_PREP(PHY_AP_START_ADDR, phy_addr_st) |
		FIELD_PREP(PHY_AP_END_ADDR, phy_addr_end));
}

static void mt7530_gsw_set_port_isolation(struct mt7620_eth_priv *priv)
{
	u32 i;

	for (i = 0; i < NUM_PORTS; i++) {
		/* Set port matrix mode */
		if (i != CPU_PORT_NUM)
			mt7530_reg_write(priv, GSW_PCR(i),
					 FIELD_PREP(PORT_MATRIX, 0x40));
		else
			mt7530_reg_write(priv, GSW_PCR(i),
					 FIELD_PREP(PORT_MATRIX, 0x3f));

		/* Set port mode to user port */
		mt7530_reg_write(priv, GSW_PVC(i),
				 FIELD_PREP(STAG_VPID, 0x8100) |
				 FIELD_PREP(VLAN_ATTR, VLAN_ATTR_USER));
	}
}

static void mt7620_gsw_config_mt7530(struct mt7620_eth_priv *priv)
{
	u16 phy_val;
	u32 i, val;

	/* Disable internal PHY, set PHY base to 12 */
	gsw_write(priv, GSW_GPC1, PHY_DIS | FIELD_PREP(PHY_BASE, 12) |
		  TX_CLK_MODE | RX_CLK_MODE);

	/* MT7530 reset deassert */
	dm_gpio_set_value(&priv->gpio_swrst, 1);
	mdelay(1000);

	/* Turn off PHYs */
	for (i = 0; i < NUM_MT7530_PHYS; i++) {
		phy_val = mt7620_mii_read(priv, i, MII_BMCR);
		phy_val |= BMCR_PDOWN;
		mt7620_mii_write(priv, i, MII_BMCR, phy_val);
	}

	/* Force MAC link down before reset */
	mt7530_reg_write(priv, GSW_PMCR(5), FORCE_MODE);
	mt7530_reg_write(priv, GSW_PMCR(6), FORCE_MODE);

	/* MT7530 soft reset */
	mt7530_reg_write(priv, MT7530_SYS_CTRL, SW_SYS_RST | SW_REG_RST);
	udelay(100);

	/* MT7530 port6 force to 1G (connects to MT7620 GSW port5) */
	mt7530_reg_write(priv, GSW_PMCR(6),
			 FIELD_PREP(IPG_CFG, IPG_96BIT_WITH_SHORT_IPG) |
			 MAC_MODE | FORCE_MODE | MAC_TX_EN | MAC_RX_EN |
			 BKOFF_EN | BACKPR_EN | FORCE_RX_FC | FORCE_TX_FC |
			 FIELD_PREP(FORCE_SPEED, FORCE_SPEED_1000) |
			 FORCE_DUPLEX | FORCE_LINK);

	/* Disable MT7530 port5 */
	mt7530_reg_read(priv, MT7530_MHWTRAP, &val);
	val |= P5_INTF_SEL_GMAC5 | P5_INTF_DIS;
	mt7530_reg_write(priv, MT7530_MHWTRAP, val);

	/* Isolate each ports */
	mt7530_gsw_set_port_isolation(priv);

	/* Turn on PHYs */
	for (i = 0; i < NUM_MT7530_PHYS; i++) {
		phy_val = mt7620_mii_read(priv, i, MII_BMCR);
		phy_val &= ~BMCR_PDOWN;
		mt7620_mii_write(priv, i, MII_BMCR, phy_val);
	}
	/* Restart auto-negotiation */
	for (i = 0; i < NUM_MT7530_PHYS; i++)
		mt7620_phy_restart_an(priv, i);
}

static void mt7620_gsw_init(struct mt7620_eth_priv *priv)
{
	/* If port5 connects to MT7530 Giga-switch, reset it first */
	if (priv->port5_mt7530)
		dm_gpio_set_value(&priv->gpio_swrst, 0);

	/* Set forward control  */
	gsw_write(priv, GSW_MFC, FIELD_PREP(BC_FFP, 0x7f) |
		  FIELD_PREP(UNM_FFP, 0x7f) | FIELD_PREP(UNU_FFP, 0x7f) |
		  CPU_EN | FIELD_PREP(CPU_PORT, CPU_PORT_NUM));

	/* Set GMAC mode (GMAC1 -> Port5, GMAC2 -> Port4) */
	mt7620_setup_gmac_mode(priv, 1, priv->port_cfg[1].mode);
	mt7620_setup_gmac_mode(priv, 2, priv->port_cfg[0].mode);

	/* port_cfg[2] is CPU port */
	priv->port_cfg[2].force_mode = true;
	priv->port_cfg[2].duplex = true;
	priv->port_cfg[2].speed = FORCE_SPEED_1000;

	/* Configure GSW MAC port */
	mt7620_gsw_setup_port(priv, 4, &priv->port_cfg[0]);
	mt7620_gsw_setup_port(priv, 5, &priv->port_cfg[1]);
	mt7620_gsw_setup_port(priv, 6, &priv->port_cfg[2]);

	/* Isolate each port */
	mt7620_gsw_set_port_isolation(priv);

	/* Polling external phy if exists */
	mt7620_gsw_setup_phy_polling(priv);

	/* Configure ephy */
	mt7620_gsw_ephy_init(priv);

	/* If port5 connects to MT7530 Giga-switch, do initialization */
	if (priv->port5_mt7530)
		mt7620_gsw_config_mt7530(priv);
}

static void mt7620_eth_fifo_init(struct mt7620_eth_priv *priv)
{
	uintptr_t pkt_base = (uintptr_t)priv->pkt_buf;
	int i;

	memset(priv->tx_ring, 0, NUM_TX_DESC * sizeof(struct pdma_tx_desc));
	memset(priv->rx_ring, 0, NUM_RX_DESC * sizeof(struct pdma_rx_desc));
	memset(priv->pkt_buf, 0, (NUM_TX_DESC + NUM_RX_DESC) * PKTSIZE_ALIGN);

	priv->tx_ring_noc = (void *)CKSEG1ADDR((uintptr_t)priv->tx_ring);
	priv->rx_ring_noc = (void *)CKSEG1ADDR((uintptr_t)priv->rx_ring);
	priv->rx_dma_owner_idx0 = 0;
	priv->tx_cpu_owner_idx0 = 0;

	for (i = 0; i < NUM_TX_DESC; i++) {
		priv->tx_ring_noc[i].txd_info2.LS0 = 1;
		priv->tx_ring_noc[i].txd_info2.DDONE = 1;
		priv->tx_ring_noc[i].txd_info4.FP_BMAP = GDMA_DST_PORT_CPU;
		priv->tx_ring_noc[i].txd_info1.SDP0 = CPHYSADDR(pkt_base);
		pkt_base += PKTSIZE_ALIGN;
	}

	for (i = 0; i < NUM_RX_DESC; i++) {
		priv->rx_ring_noc[i].rxd_info2.PLEN0 = PKTSIZE_ALIGN;
		priv->rx_ring_noc[i].rxd_info1.PDP0 = CPHYSADDR(pkt_base);
		pkt_base += PKTSIZE_ALIGN;
	}

	pdma_write(priv, TX_BASE_PTR0, CPHYSADDR(priv->tx_ring_noc));
	pdma_write(priv, TX_MAX_CNT0, NUM_TX_DESC);
	pdma_write(priv, TX_CTX_IDX0, priv->tx_cpu_owner_idx0);

	pdma_write(priv, RX_BASE_PTR0, CPHYSADDR(priv->rx_ring_noc));
	pdma_write(priv, RX_MAX_CNT0, NUM_RX_DESC);
	pdma_write(priv, RX_CALC_IDX0, NUM_RX_DESC - 1);

	pdma_write(priv, PDMA_RST_IDX, RST_DTX_IDX0 | RST_DRX_IDX0);
}

static int mt7620_eth_start(struct udevice *dev)
{
	struct mt7620_eth_priv *priv = dev_get_priv(dev);

	mt7620_eth_fifo_init(priv);

	gdma_rmw(priv, GDMA_FWD_CFG, GDMA_DST_PORT,
		 FIELD_PREP(GDMA_DST_PORT, GDMA_DST_PORT_CPU));

	pdma_write(priv, PDMA_GLO_CFG,
		   FIELD_PREP(PDMA_BT_SIZE, PDMA_BT_SIZE_32B) |
		   TX_WB_DDONE | RX_DMA_EN | TX_DMA_EN);
	udelay(500);

	return 0;
}

static void mt7620_eth_stop(struct udevice *dev)
{
	struct mt7620_eth_priv *priv = dev_get_priv(dev);
	u32 val;
	int ret;

	pdma_write(priv, PDMA_GLO_CFG,
		   FIELD_PREP(PDMA_BT_SIZE, PDMA_BT_SIZE_32B));
	udelay(500);

	ret = readl_poll_timeout(priv->fe_base + PDMA_BASE + PDMA_GLO_CFG,
				 val, !(val & (RX_DMA_BUSY | TX_DMA_BUSY)),
				 PDMA_TIMEOUT);
	if (ret)
		dev_warn(dev, "mt7620_eth: PDMA is still busy\n");
}

static int mt7620_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct mt7620_eth_priv *priv = dev_get_priv(dev);
	unsigned char *mac = pdata->enetaddr;
	u32 macaddr_lsb, macaddr_msb;

	macaddr_msb = ((u32)mac[0] << 8) | (u32)mac[1];
	macaddr_lsb = ((u32)mac[2] << 24) | ((u32)mac[3] << 16) |
		      ((u32)mac[4] << 8) | (u32)mac[5];

	gdma_write(priv, GDMA_MAC_ADRH, macaddr_msb);
	gdma_write(priv, GDMA_MAC_ADRL, macaddr_lsb);

	return 0;
}

static int mt7620_eth_send(struct udevice *dev, void *packet, int length)
{
	struct mt7620_eth_priv *priv = dev_get_priv(dev);
	u32 idx = priv->tx_cpu_owner_idx0;
	void *pkt_base;

	if (!priv->tx_ring_noc[idx].txd_info2.DDONE) {
		printf("mt7620_eth: TX DMA descriptor ring is full\n");
		return -EPERM;
	}

	pkt_base = (void *)CKSEG0ADDR(priv->tx_ring_noc[idx].txd_info1.SDP0);
	memcpy(pkt_base, packet, length);
	flush_dcache_range((ulong)pkt_base, (ulong)pkt_base + length);

	priv->tx_ring_noc[idx].txd_info2.SDL0 = length;
	priv->tx_ring_noc[idx].txd_info2.DDONE = 0;

	priv->tx_cpu_owner_idx0 = (priv->tx_cpu_owner_idx0 + 1) % NUM_TX_DESC;
	pdma_write(priv, TX_CTX_IDX0, priv->tx_cpu_owner_idx0);

	return 0;
}

static int mt7620_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct mt7620_eth_priv *priv = dev_get_priv(dev);
	u32 idx = priv->rx_dma_owner_idx0, length;
	uchar *pkt_base;

	if (!priv->rx_ring_noc[idx].rxd_info2.DDONE) {
		debug("mt7620_eth: RX DMA descriptor ring is empty\n");
		return -EAGAIN;
	}

	length = priv->rx_ring_noc[idx].rxd_info2.PLEN0;
	pkt_base = (void *)CKSEG0ADDR(priv->rx_ring_noc[idx].rxd_info1.PDP0);
	invalidate_dcache_range((ulong)pkt_base, (ulong)pkt_base + length);

	if (packetp)
		*packetp = pkt_base;

	return length;
}

static int mt7620_eth_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct mt7620_eth_priv *priv = dev_get_priv(dev);
	u32 idx = priv->rx_dma_owner_idx0;

	priv->rx_ring_noc[idx].rxd_info2.DDONE = 0;
	priv->rx_ring_noc[idx].rxd_info2.LS0 = 0;
	priv->rx_ring_noc[idx].rxd_info2.PLEN0 = PKTSIZE_ALIGN;

	pdma_write(priv, RX_CALC_IDX0, idx);
	priv->rx_dma_owner_idx0 = (priv->rx_dma_owner_idx0 + 1) % NUM_RX_DESC;

	return 0;
}

static const struct eth_ops mt7620_eth_ops = {
	.start = mt7620_eth_start,
	.stop = mt7620_eth_stop,
	.send = mt7620_eth_send,
	.recv = mt7620_eth_recv,
	.free_pkt = mt7620_eth_free_pkt,
	.write_hwaddr = mt7620_eth_write_hwaddr,
};

static int mt7620_eth_alloc_rings_pkts(struct mt7620_eth_priv *priv)
{
	priv->tx_ring = memalign(ARCH_DMA_MINALIGN,
				 NUM_TX_DESC * sizeof(struct pdma_tx_desc));
	if (!priv->tx_ring) {
		dev_err(priv->dev, "mt7620_eth: unable to alloc tx ring\n");
		return -ENOMEM;
	}

	priv->rx_ring = memalign(ARCH_DMA_MINALIGN,
				 NUM_RX_DESC * sizeof(struct pdma_rx_desc));
	if (!priv->rx_ring) {
		dev_err(priv->dev, "mt7620_eth: unable to alloc rx ring\n");
		goto cleanup;
	}

	priv->pkt_buf = memalign(ARCH_DMA_MINALIGN,
				 (NUM_TX_DESC + NUM_RX_DESC) * PKTSIZE_ALIGN);
	if (!priv->pkt_buf) {
		dev_err(priv->dev, "mt7620_eth: unable to alloc pkt buffer\n");
		goto cleanup;
	}

	return 0;

cleanup:
	if (priv->tx_ring)
		free(priv->tx_ring);

	if (priv->rx_ring)
		free(priv->rx_ring);

	return -ENOMEM;
}

static void mt7620_eth_free_rings_pkts(struct mt7620_eth_priv *priv)
{
	free(priv->tx_ring);
	free(priv->rx_ring);
	free(priv->pkt_buf);
}

static int mt7620_eth_probe(struct udevice *dev)
{
	struct mt7620_eth_priv *priv = dev_get_priv(dev);
	u32 pcie_mode = MT7620_SYSC_PCIE_RC_MODE;
	int ret;

	misc_ioctl(priv->sysc, MT7620_SYSC_IOCTL_SET_PCIE_MODE, &pcie_mode);

	clk_enable_bulk(&priv->clks);

	reset_assert_bulk(&priv->rsts);
	udelay(100);
	reset_deassert_bulk(&priv->rsts);
	udelay(1000);

	ret = mt7620_eth_alloc_rings_pkts(priv);
	if (ret)
		return ret;

	ret = mt7620_mdio_register(dev);
	if (ret)
		dev_warn(dev, "mt7620_eth: failed to register MDIO bus\n");

	mt7620_gsw_init(priv);

	return 0;
}

static int mt7620_eth_remove(struct udevice *dev)
{
	struct mt7620_eth_priv *priv = dev_get_priv(dev);

	mt7620_eth_stop(dev);

	mt7620_eth_free_rings_pkts(priv);

	return 0;
}

static int mt7620_eth_parse_gsw_port(struct mt7620_eth_priv *priv, u32 idx,
				     ofnode node)
{
	ofnode subnode;
	int speed, ret;
	u32 phy_addr;

	priv->port_cfg[idx].mode = ofnode_read_phy_mode(node);

	switch (priv->port_cfg[idx].mode) {
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_RMII:
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_NA:
		break;
	default:
		dev_err(priv->dev, "mt7620_eth: unsupported phy-mode\n");
		return -ENOTSUPP;
	}

	subnode = ofnode_find_subnode(node, "fixed-link");
	if (ofnode_valid(subnode)) {
		priv->port_cfg[idx].force_mode = 1;
		priv->port_cfg[idx].duplex = ofnode_read_bool(subnode,
							      "full-duplex");
		speed = ofnode_read_u32_default(subnode, "speed", 0);
		switch (speed) {
		case SPEED_10:
			priv->port_cfg[idx].speed = FORCE_SPEED_10;
			break;
		case SPEED_100:
			priv->port_cfg[idx].speed = FORCE_SPEED_100;
			break;
		case SPEED_1000:
			priv->port_cfg[idx].speed = FORCE_SPEED_1000;
			break;
		default:
			dev_err(priv->dev,
				"mt7620_eth: invalid speed for fixed-link\n");
			return -EINVAL;
		}

		if (idx == 1 && ofnode_read_bool(subnode, "mediatek,mt7530")) {
			priv->port5_mt7530 = true;

			ret = gpio_request_by_name_nodev(subnode,
				"mediatek,mt7530-reset", 0, &priv->gpio_swrst,
				GPIOD_IS_OUT);
			if (ret) {
				dev_err(priv->dev,
					"mt7620_eth: missing mt7530 reset gpio\n");
				return ret;
			}
		}
	}

	ret = ofnode_read_u32(node, "phy-addr", &phy_addr);
	if (!ret) {
		if (phy_addr > 31 || (idx == 0 && phy_addr < 3) ||
		    (idx == 1 && phy_addr < 4)) {
			dev_err(priv->dev, "mt7620_eth: invalid phy address\n");
			return -EINVAL;
		}

		priv->port_cfg[idx].phy_addr = phy_addr;
	} else {
		priv->port_cfg[idx].phy_addr = -1;
	}

	return 0;
}

static int mt7620_eth_parse_gsw_cfg(struct udevice *dev)
{
	struct mt7620_eth_priv *priv = dev_get_priv(dev);
	ofnode subnode;
	int ret;

	subnode = ofnode_find_subnode(dev_ofnode(dev), "port4");
	if (ofnode_valid(subnode)) {
		ret = mt7620_eth_parse_gsw_port(priv, 0, subnode);
		if (ret)
			return ret;
	} else {
		priv->port_cfg[0].mode = PHY_INTERFACE_MODE_NA;
	}

	subnode = ofnode_find_subnode(dev_ofnode(dev), "port5");
	if (ofnode_valid(subnode))
		return mt7620_eth_parse_gsw_port(priv, 1, subnode);

	priv->port_cfg[1].mode = PHY_INTERFACE_MODE_NA;
	return 0;
}

static int mt7620_eth_of_to_plat(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct mt7620_eth_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args sysc_args;
	int ret;

	pdata->iobase = dev_read_addr(dev);

	priv->dev = dev;

	ret = ofnode_parse_phandle_with_args(dev_ofnode(dev), "mediatek,sysc", NULL,
					     0, 0, &sysc_args);
	if (ret) {
		dev_err(dev, "mt7620_eth: sysc property not found\n");
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_MISC, sysc_args.node,
					  &priv->sysc);
	if (ret) {
		dev_err(dev, "mt7620_eth: failed to sysc device\n");
		return ret;
	}

	priv->fe_base = dev_remap_addr_name(dev, "fe");
	if (!priv->fe_base) {
		dev_err(dev, "mt7620_eth: failed to map fe registers\n");
		return -EINVAL;
	}

	priv->gsw_base = dev_remap_addr_name(dev, "esw");
	if (!priv->gsw_base) {
		dev_err(dev, "mt7620_eth: failed to map esw registers\n");
		return -EINVAL;
	}

	ret = reset_get_bulk(dev, &priv->rsts);
	if (ret) {
		dev_err(dev, "mt7620_eth: failed to get resetctl\n");
		return ret;
	}

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret) {
		dev_err(dev, "mt7620_eth: failed to get clocks\n");
		return ret;
	}

	return mt7620_eth_parse_gsw_cfg(dev);
}

static const struct udevice_id mt7620_eth_ids[] = {
	{ .compatible = "mediatek,mt7620-eth" },
	{}
};

U_BOOT_DRIVER(mt7620_eth) = {
	.name = "mt7620-eth",
	.id = UCLASS_ETH,
	.of_match = mt7620_eth_ids,
	.of_to_plat = mt7620_eth_of_to_plat,
	.plat_auto = sizeof(struct eth_pdata),
	.probe = mt7620_eth_probe,
	.remove = mt7620_eth_remove,
	.ops = &mt7620_eth_ops,
	.priv_auto = sizeof(struct mt7620_eth_priv),
};
