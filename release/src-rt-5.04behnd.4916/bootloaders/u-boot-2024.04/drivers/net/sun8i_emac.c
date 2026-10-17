// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 * Author: Amit Singh Tomar, amittomer25@gmail.com
 *
 * Ethernet driver for H3/A64/A83T based SoC's
 *
 * It is derived from the work done by
 * LABBE Corentin & Chen-Yu Tsai for Linux, THANKS!
 *
*/

#include <cpu_func.h>
#include <log.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fdt_support.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <reset.h>
#include <wait_bit.h>
#include <power/regulator.h>

#define MDIO_CMD_MII_BUSY		BIT(0)
#define MDIO_CMD_MII_WRITE		BIT(1)

#define MDIO_CMD_MII_PHY_REG_ADDR_MASK	0x000001f0
#define MDIO_CMD_MII_PHY_REG_ADDR_SHIFT	4
#define MDIO_CMD_MII_PHY_ADDR_MASK	0x0001f000
#define MDIO_CMD_MII_PHY_ADDR_SHIFT	12
#define MDIO_CMD_MII_CLK_CSR_DIV_16	0x0
#define MDIO_CMD_MII_CLK_CSR_DIV_32	0x1
#define MDIO_CMD_MII_CLK_CSR_DIV_64	0x2
#define MDIO_CMD_MII_CLK_CSR_DIV_128	0x3
#define MDIO_CMD_MII_CLK_CSR_SHIFT	20

#define CFG_TX_DESCR_NUM	32
#define CFG_RX_DESCR_NUM	32
#define CFG_ETH_BUFSIZE	2048 /* Note must be dma aligned */

/*
 * The datasheet says that each descriptor can transfers up to 4096 bytes
 * But later, the register documentation reduces that value to 2048,
 * using 2048 cause strange behaviours and even BSP driver use 2047
 */
#define CFG_ETH_RXSIZE	2044 /* Note must fit in ETH_BUFSIZE */

#define TX_TOTAL_BUFSIZE	(CFG_ETH_BUFSIZE * CFG_TX_DESCR_NUM)
#define RX_TOTAL_BUFSIZE	(CFG_ETH_BUFSIZE * CFG_RX_DESCR_NUM)

#define H3_EPHY_DEFAULT_VALUE	0x58000
#define H3_EPHY_DEFAULT_MASK	GENMASK(31, 15)
#define H3_EPHY_ADDR_SHIFT	20
#define REG_PHY_ADDR_MASK	GENMASK(4, 0)
#define H3_EPHY_LED_POL		BIT(17)	/* 1: active low, 0: active high */
#define H3_EPHY_SHUTDOWN	BIT(16)	/* 1: shutdown, 0: power up */
#define H3_EPHY_SELECT		BIT(15) /* 1: internal PHY, 0: external PHY */

#define SC_RMII_EN		BIT(13)
#define SC_EPIT			BIT(2) /* 1: RGMII, 0: MII */
#define SC_ETCS_MASK		GENMASK(1, 0)
#define SC_ETCS_EXT_GMII	0x1
#define SC_ETCS_INT_GMII	0x2
#define SC_ETXDC_MASK		GENMASK(12, 10)
#define SC_ETXDC_OFFSET		10
#define SC_ERXDC_MASK		GENMASK(9, 5)
#define SC_ERXDC_OFFSET		5

#define CFG_MDIO_TIMEOUT	(3 * CONFIG_SYS_HZ)

#define AHB_GATE_OFFSET_EPHY	0

/* H3/A64 EMAC Register's offset */
#define EMAC_CTL0		0x00
#define EMAC_CTL0_FULL_DUPLEX		BIT(0)
#define EMAC_CTL0_SPEED_MASK		GENMASK(3, 2)
#define EMAC_CTL0_SPEED_10		(0x2 << 2)
#define EMAC_CTL0_SPEED_100		(0x3 << 2)
#define EMAC_CTL0_SPEED_1000		(0x0 << 2)
#define EMAC_CTL1		0x04
#define EMAC_CTL1_SOFT_RST		BIT(0)
#define EMAC_CTL1_BURST_LEN_SHIFT	24
#define EMAC_INT_STA		0x08
#define EMAC_INT_EN		0x0c
#define EMAC_TX_CTL0		0x10
#define	EMAC_TX_CTL0_TX_EN		BIT(31)
#define EMAC_TX_CTL1		0x14
#define	EMAC_TX_CTL1_TX_MD		BIT(1)
#define	EMAC_TX_CTL1_TX_DMA_EN		BIT(30)
#define	EMAC_TX_CTL1_TX_DMA_START	BIT(31)
#define EMAC_TX_FLOW_CTL	0x1c
#define EMAC_TX_DMA_DESC	0x20
#define EMAC_RX_CTL0		0x24
#define	EMAC_RX_CTL0_RX_EN		BIT(31)
#define EMAC_RX_CTL1		0x28
#define	EMAC_RX_CTL1_RX_MD		BIT(1)
#define	EMAC_RX_CTL1_RX_RUNT_FRM	BIT(2)
#define	EMAC_RX_CTL1_RX_ERR_FRM		BIT(3)
#define	EMAC_RX_CTL1_RX_DMA_EN		BIT(30)
#define	EMAC_RX_CTL1_RX_DMA_START	BIT(31)
#define EMAC_RX_DMA_DESC	0x34
#define EMAC_MII_CMD		0x48
#define EMAC_MII_DATA		0x4c
#define EMAC_ADDR0_HIGH		0x50
#define EMAC_ADDR0_LOW		0x54
#define EMAC_TX_DMA_STA		0xb0
#define EMAC_TX_CUR_DESC	0xb4
#define EMAC_TX_CUR_BUF		0xb8
#define EMAC_RX_DMA_STA		0xc0
#define EMAC_RX_CUR_DESC	0xc4

#define EMAC_DESC_OWN_DMA	BIT(31)
#define EMAC_DESC_LAST_DESC	BIT(30)
#define EMAC_DESC_FIRST_DESC	BIT(29)
#define EMAC_DESC_CHAIN_SECOND	BIT(24)

#define EMAC_DESC_RX_ERROR_MASK	0x400068db

DECLARE_GLOBAL_DATA_PTR;

struct emac_variant {
	uint			syscon_offset;
	bool			soc_has_internal_phy;
	bool			support_rmii;
};

struct emac_dma_desc {
	u32 status;
	u32 ctl_size;
	u32 buf_addr;
	u32 next;
} __aligned(ARCH_DMA_MINALIGN);

struct emac_eth_dev {
	struct emac_dma_desc rx_chain[CFG_TX_DESCR_NUM];
	struct emac_dma_desc tx_chain[CFG_RX_DESCR_NUM];
	char rxbuffer[RX_TOTAL_BUFSIZE] __aligned(ARCH_DMA_MINALIGN);
	char txbuffer[TX_TOTAL_BUFSIZE] __aligned(ARCH_DMA_MINALIGN);

	u32 interface;
	u32 phyaddr;
	u32 link;
	u32 speed;
	u32 duplex;
	u32 phy_configured;
	u32 tx_currdescnum;
	u32 rx_currdescnum;
	u32 addr;
	u32 tx_slot;
	bool use_internal_phy;

	const struct emac_variant *variant;
	void *mac_reg;
	void *sysctl_reg;
	struct phy_device *phydev;
	struct mii_dev *bus;
	struct clk tx_clk;
	struct clk ephy_clk;
	struct reset_ctl tx_rst;
	struct reset_ctl ephy_rst;
	struct gpio_desc reset_gpio;
	struct udevice *phy_reg;
};


struct sun8i_eth_pdata {
	struct eth_pdata eth_pdata;
	u32 reset_delays[3];
	int tx_delay_ps;
	int rx_delay_ps;
};


static int sun8i_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct udevice *dev = bus->priv;
	struct emac_eth_dev *priv = dev_get_priv(dev);
	u32 mii_cmd;
	int ret;

	mii_cmd = (reg << MDIO_CMD_MII_PHY_REG_ADDR_SHIFT) &
		MDIO_CMD_MII_PHY_REG_ADDR_MASK;
	mii_cmd |= (addr << MDIO_CMD_MII_PHY_ADDR_SHIFT) &
		MDIO_CMD_MII_PHY_ADDR_MASK;

	/*
	 * The EMAC clock is either 200 or 300 MHz, so we need a divider
	 * of 128 to get the MDIO frequency below the required 2.5 MHz.
	 */
	if (!priv->use_internal_phy)
		mii_cmd |= MDIO_CMD_MII_CLK_CSR_DIV_128 <<
			   MDIO_CMD_MII_CLK_CSR_SHIFT;

	mii_cmd |= MDIO_CMD_MII_BUSY;

	writel(mii_cmd, priv->mac_reg + EMAC_MII_CMD);

	ret = wait_for_bit_le32(priv->mac_reg + EMAC_MII_CMD,
				MDIO_CMD_MII_BUSY, false,
				CFG_MDIO_TIMEOUT, true);
	if (ret < 0)
		return ret;

	return readl(priv->mac_reg + EMAC_MII_DATA);
}

static int sun8i_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			    u16 val)
{
	struct udevice *dev = bus->priv;
	struct emac_eth_dev *priv = dev_get_priv(dev);
	u32 mii_cmd;

	mii_cmd = (reg << MDIO_CMD_MII_PHY_REG_ADDR_SHIFT) &
		MDIO_CMD_MII_PHY_REG_ADDR_MASK;
	mii_cmd |= (addr << MDIO_CMD_MII_PHY_ADDR_SHIFT) &
		MDIO_CMD_MII_PHY_ADDR_MASK;

	/*
	 * The EMAC clock is either 200 or 300 MHz, so we need a divider
	 * of 128 to get the MDIO frequency below the required 2.5 MHz.
	 */
	if (!priv->use_internal_phy)
		mii_cmd |= MDIO_CMD_MII_CLK_CSR_DIV_128 <<
			   MDIO_CMD_MII_CLK_CSR_SHIFT;

	mii_cmd |= MDIO_CMD_MII_WRITE;
	mii_cmd |= MDIO_CMD_MII_BUSY;

	writel(val, priv->mac_reg + EMAC_MII_DATA);
	writel(mii_cmd, priv->mac_reg + EMAC_MII_CMD);

	return wait_for_bit_le32(priv->mac_reg + EMAC_MII_CMD,
				 MDIO_CMD_MII_BUSY, false,
				 CFG_MDIO_TIMEOUT, true);
}

static int sun8i_eth_write_hwaddr(struct udevice *dev)
{
	struct emac_eth_dev *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	uchar *mac_id = pdata->enetaddr;
	u32 macid_lo, macid_hi;

	macid_lo = mac_id[0] + (mac_id[1] << 8) + (mac_id[2] << 16) +
		(mac_id[3] << 24);
	macid_hi = mac_id[4] + (mac_id[5] << 8);

	writel(macid_hi, priv->mac_reg + EMAC_ADDR0_HIGH);
	writel(macid_lo, priv->mac_reg + EMAC_ADDR0_LOW);

	return 0;
}

static void sun8i_adjust_link(struct emac_eth_dev *priv,
			      struct phy_device *phydev)
{
	u32 v;

	v = readl(priv->mac_reg + EMAC_CTL0);

	if (phydev->duplex)
		v |= EMAC_CTL0_FULL_DUPLEX;
	else
		v &= ~EMAC_CTL0_FULL_DUPLEX;

	v &= ~EMAC_CTL0_SPEED_MASK;

	switch (phydev->speed) {
	case 1000:
		v |= EMAC_CTL0_SPEED_1000;
		break;
	case 100:
		v |= EMAC_CTL0_SPEED_100;
		break;
	case 10:
		v |= EMAC_CTL0_SPEED_10;
		break;
	}
	writel(v, priv->mac_reg + EMAC_CTL0);
}

static u32 sun8i_emac_set_syscon_ephy(struct emac_eth_dev *priv, u32 reg)
{
	if (priv->use_internal_phy) {
		/* H3 based SoC's that has an Internal 100MBit PHY
		 * needs to be configured and powered up before use
		*/
		reg &= ~H3_EPHY_DEFAULT_MASK;
		reg |=  H3_EPHY_DEFAULT_VALUE;
		reg |= priv->phyaddr << H3_EPHY_ADDR_SHIFT;
		reg &= ~H3_EPHY_SHUTDOWN;
		return reg | H3_EPHY_SELECT;
	}

	/* This is to select External Gigabit PHY on those boards with
	 * an internal PHY. Does not hurt on other SoCs. Linux does
	 * it as well.
	 */
	return reg & ~H3_EPHY_SELECT;
}

static int sun8i_emac_set_syscon(struct sun8i_eth_pdata *pdata,
				 struct emac_eth_dev *priv)
{
	u32 reg;

	reg = readl(priv->sysctl_reg);

	reg = sun8i_emac_set_syscon_ephy(priv, reg);

	reg &= ~(SC_ETCS_MASK | SC_EPIT);
	if (priv->variant->support_rmii)
		reg &= ~SC_RMII_EN;

	switch (priv->interface) {
	case PHY_INTERFACE_MODE_MII:
		/* default */
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		reg |= SC_EPIT | SC_ETCS_INT_GMII;
		break;
	case PHY_INTERFACE_MODE_RMII:
		if (priv->variant->support_rmii) {
			reg |= SC_RMII_EN | SC_ETCS_EXT_GMII;
			break;
		}
	default:
		debug("%s: Invalid PHY interface\n", __func__);
		return -EINVAL;
	}

	if (pdata->tx_delay_ps)
		reg |= ((pdata->tx_delay_ps / 100) << SC_ETXDC_OFFSET)
			 & SC_ETXDC_MASK;

	if (pdata->rx_delay_ps)
		reg |= ((pdata->rx_delay_ps / 100) << SC_ERXDC_OFFSET)
			 & SC_ERXDC_MASK;

	writel(reg, priv->sysctl_reg);

	return 0;
}

static int sun8i_phy_init(struct emac_eth_dev *priv, void *dev)
{
	struct phy_device *phydev;

	phydev = phy_connect(priv->bus, priv->phyaddr, dev, priv->interface);
	if (!phydev)
		return -ENODEV;

	priv->phydev = phydev;
	phy_config(priv->phydev);

	return 0;
}

#define cache_clean_descriptor(desc)					\
	flush_dcache_range((uintptr_t)(desc),				\
			   (uintptr_t)(desc) + sizeof(struct emac_dma_desc))

#define cache_inv_descriptor(desc)					\
	invalidate_dcache_range((uintptr_t)(desc),			\
			       (uintptr_t)(desc) + sizeof(struct emac_dma_desc))

static void rx_descs_init(struct emac_eth_dev *priv)
{
	struct emac_dma_desc *desc_table_p = &priv->rx_chain[0];
	char *rxbuffs = &priv->rxbuffer[0];
	struct emac_dma_desc *desc_p;
	int i;

	/*
	 * Make sure we don't have dirty cache lines around, which could
	 * be cleaned to DRAM *after* the MAC has already written data to it.
	 */
	invalidate_dcache_range((uintptr_t)desc_table_p,
			      (uintptr_t)desc_table_p + sizeof(priv->rx_chain));
	invalidate_dcache_range((uintptr_t)rxbuffs,
				(uintptr_t)rxbuffs + sizeof(priv->rxbuffer));

	for (i = 0; i < CFG_RX_DESCR_NUM; i++) {
		desc_p = &desc_table_p[i];
		desc_p->buf_addr = (uintptr_t)&rxbuffs[i * CFG_ETH_BUFSIZE];
		desc_p->next = (uintptr_t)&desc_table_p[i + 1];
		desc_p->ctl_size = CFG_ETH_RXSIZE;
		desc_p->status = EMAC_DESC_OWN_DMA;
	}

	/* Correcting the last pointer of the chain */
	desc_p->next = (uintptr_t)&desc_table_p[0];

	flush_dcache_range((uintptr_t)priv->rx_chain,
			   (uintptr_t)priv->rx_chain +
			sizeof(priv->rx_chain));

	writel((uintptr_t)&desc_table_p[0], (priv->mac_reg + EMAC_RX_DMA_DESC));
	priv->rx_currdescnum = 0;
}

static void tx_descs_init(struct emac_eth_dev *priv)
{
	struct emac_dma_desc *desc_table_p = &priv->tx_chain[0];
	char *txbuffs = &priv->txbuffer[0];
	struct emac_dma_desc *desc_p;
	int i;

	for (i = 0; i < CFG_TX_DESCR_NUM; i++) {
		desc_p = &desc_table_p[i];
		desc_p->buf_addr = (uintptr_t)&txbuffs[i * CFG_ETH_BUFSIZE];
		desc_p->next = (uintptr_t)&desc_table_p[i + 1];
		desc_p->ctl_size = 0;
		desc_p->status = 0;
	}

	/* Correcting the last pointer of the chain */
	desc_p->next =  (uintptr_t)&desc_table_p[0];

	/* Flush the first TX buffer descriptor we will tell the MAC about. */
	cache_clean_descriptor(desc_table_p);

	writel((uintptr_t)&desc_table_p[0], priv->mac_reg + EMAC_TX_DMA_DESC);
	priv->tx_currdescnum = 0;
}

static int sun8i_emac_eth_start(struct udevice *dev)
{
	struct emac_eth_dev *priv = dev_get_priv(dev);
	int ret;

	/* Soft reset MAC */
	writel(EMAC_CTL1_SOFT_RST, priv->mac_reg + EMAC_CTL1);
	ret = wait_for_bit_le32(priv->mac_reg + EMAC_CTL1,
				EMAC_CTL1_SOFT_RST, false, 10, true);
	if (ret) {
		printf("%s: Timeout\n", __func__);
		return ret;
	}

	/* Rewrite mac address after reset */
	sun8i_eth_write_hwaddr(dev);

	/* transmission starts after the full frame arrived in TX DMA FIFO */
	setbits_le32(priv->mac_reg + EMAC_TX_CTL1, EMAC_TX_CTL1_TX_MD);

	/*
	 * RX DMA reads data from RX DMA FIFO to host memory after a
	 * complete frame has been written to RX DMA FIFO
	 */
	setbits_le32(priv->mac_reg + EMAC_RX_CTL1, EMAC_RX_CTL1_RX_MD);

	/* DMA burst length */
	writel(8 << EMAC_CTL1_BURST_LEN_SHIFT, priv->mac_reg + EMAC_CTL1);

	/* Initialize rx/tx descriptors */
	rx_descs_init(priv);
	tx_descs_init(priv);

	/* PHY Start Up */
	ret = phy_startup(priv->phydev);
	if (ret)
		return ret;

	sun8i_adjust_link(priv, priv->phydev);

	/* Start RX/TX DMA */
	setbits_le32(priv->mac_reg + EMAC_RX_CTL1, EMAC_RX_CTL1_RX_DMA_EN |
		     EMAC_RX_CTL1_RX_ERR_FRM | EMAC_RX_CTL1_RX_RUNT_FRM);
	setbits_le32(priv->mac_reg + EMAC_TX_CTL1, EMAC_TX_CTL1_TX_DMA_EN);

	/* Enable RX/TX */
	setbits_le32(priv->mac_reg + EMAC_RX_CTL0, EMAC_RX_CTL0_RX_EN);
	setbits_le32(priv->mac_reg + EMAC_TX_CTL0, EMAC_TX_CTL0_TX_EN);

	return 0;
}

static int sun8i_emac_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct emac_eth_dev *priv = dev_get_priv(dev);
	u32 status, desc_num = priv->rx_currdescnum;
	struct emac_dma_desc *desc_p = &priv->rx_chain[desc_num];
	uintptr_t data_start = (uintptr_t)desc_p->buf_addr;
	int length;

	/* Invalidate entire buffer descriptor */
	cache_inv_descriptor(desc_p);

	status = desc_p->status;

	/* Check for DMA own bit */
	if (status & EMAC_DESC_OWN_DMA)
		return -EAGAIN;

	length = (status >> 16) & 0x3fff;

	/* make sure we read from DRAM, not our cache */
	invalidate_dcache_range(data_start,
				data_start + roundup(length, ARCH_DMA_MINALIGN));

	if (status & EMAC_DESC_RX_ERROR_MASK) {
		debug("RX: packet error: 0x%x\n",
		      status & EMAC_DESC_RX_ERROR_MASK);
		return 0;
	}
	if (length < 0x40) {
		debug("RX: Bad Packet (runt)\n");
		return 0;
	}

	if (length > CFG_ETH_RXSIZE) {
		debug("RX: Too large packet (%d bytes)\n", length);
		return 0;
	}

	*packetp = (uchar *)(ulong)desc_p->buf_addr;

	return length;
}

static int sun8i_emac_eth_send(struct udevice *dev, void *packet, int length)
{
	struct emac_eth_dev *priv = dev_get_priv(dev);
	u32 desc_num = priv->tx_currdescnum;
	struct emac_dma_desc *desc_p = &priv->tx_chain[desc_num];
	uintptr_t data_start = (uintptr_t)desc_p->buf_addr;
	uintptr_t data_end = data_start +
		roundup(length, ARCH_DMA_MINALIGN);

	desc_p->ctl_size = length | EMAC_DESC_CHAIN_SECOND;

	memcpy((void *)data_start, packet, length);

	/* Flush data to be sent */
	flush_dcache_range(data_start, data_end);

	/* frame begin and end */
	desc_p->ctl_size |= EMAC_DESC_LAST_DESC | EMAC_DESC_FIRST_DESC;
	desc_p->status = EMAC_DESC_OWN_DMA;

	/* make sure the MAC reads the actual data from DRAM */
	cache_clean_descriptor(desc_p);

	/* Move to next Descriptor and wrap around */
	if (++desc_num >= CFG_TX_DESCR_NUM)
		desc_num = 0;
	priv->tx_currdescnum = desc_num;

	/* Start the DMA */
	setbits_le32(priv->mac_reg + EMAC_TX_CTL1, EMAC_TX_CTL1_TX_DMA_START);

	/*
	 * Since we copied the data above, we return here without waiting
	 * for the packet to be actually send out.
	 */

	return 0;
}

static int sun8i_emac_board_setup(struct udevice *dev,
				  struct emac_eth_dev *priv)
{
	int ret;

	ret = clk_enable(&priv->tx_clk);
	if (ret) {
		dev_err(dev, "failed to enable TX clock\n");
		return ret;
	}

	if (reset_valid(&priv->tx_rst)) {
		ret = reset_deassert(&priv->tx_rst);
		if (ret) {
			dev_err(dev, "failed to deassert TX reset\n");
			goto err_tx_clk;
		}
	}

	/* Only H3/H5 have clock controls for internal EPHY */
	if (clk_valid(&priv->ephy_clk)) {
		ret = clk_enable(&priv->ephy_clk);
		if (ret) {
			dev_err(dev, "failed to enable EPHY TX clock\n");
			return ret;
		}
	}

	if (reset_valid(&priv->ephy_rst)) {
		ret = reset_deassert(&priv->ephy_rst);
		if (ret) {
			dev_err(dev, "failed to deassert EPHY TX clock\n");
			return ret;
		}
	}

	return 0;

err_tx_clk:
	clk_disable(&priv->tx_clk);
	return ret;
}

static int sun8i_mdio_reset(struct mii_dev *bus)
{
	struct udevice *dev = bus->priv;
	struct emac_eth_dev *priv = dev_get_priv(dev);
	struct sun8i_eth_pdata *pdata = dev_get_plat(dev);
	int ret;

	if (!dm_gpio_is_valid(&priv->reset_gpio))
		return 0;

	/* reset the phy */
	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret)
		return ret;

	udelay(pdata->reset_delays[0]);

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret)
		return ret;

	udelay(pdata->reset_delays[1]);

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret)
		return ret;

	udelay(pdata->reset_delays[2]);

	return 0;
}

static int sun8i_mdio_init(const char *name, struct udevice *priv)
{
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		debug("Failed to allocate MDIO bus\n");
		return -ENOMEM;
	}

	bus->read = sun8i_mdio_read;
	bus->write = sun8i_mdio_write;
	snprintf(bus->name, sizeof(bus->name), name);
	bus->priv = (void *)priv;
	bus->reset = sun8i_mdio_reset;

	return  mdio_register(bus);
}

static int sun8i_eth_free_pkt(struct udevice *dev, uchar *packet,
			      int length)
{
	struct emac_eth_dev *priv = dev_get_priv(dev);
	u32 desc_num = priv->rx_currdescnum;
	struct emac_dma_desc *desc_p = &priv->rx_chain[desc_num];

	/* give the current descriptor back to the MAC */
	desc_p->status |= EMAC_DESC_OWN_DMA;

	/* Flush Status field of descriptor */
	cache_clean_descriptor(desc_p);

	/* Move to next desc and wrap-around condition. */
	if (++desc_num >= CFG_RX_DESCR_NUM)
		desc_num = 0;
	priv->rx_currdescnum = desc_num;

	return 0;
}

static void sun8i_emac_eth_stop(struct udevice *dev)
{
	struct emac_eth_dev *priv = dev_get_priv(dev);

	/* Stop Rx/Tx transmitter */
	clrbits_le32(priv->mac_reg + EMAC_RX_CTL0, EMAC_RX_CTL0_RX_EN);
	clrbits_le32(priv->mac_reg + EMAC_TX_CTL0, EMAC_TX_CTL0_TX_EN);

	/* Stop RX/TX DMA */
	clrbits_le32(priv->mac_reg + EMAC_TX_CTL1, EMAC_TX_CTL1_TX_DMA_EN);
	clrbits_le32(priv->mac_reg + EMAC_RX_CTL1, EMAC_RX_CTL1_RX_DMA_EN);

	phy_shutdown(priv->phydev);
}

static int sun8i_emac_eth_probe(struct udevice *dev)
{
	struct sun8i_eth_pdata *sun8i_pdata = dev_get_plat(dev);
	struct eth_pdata *pdata = &sun8i_pdata->eth_pdata;
	struct emac_eth_dev *priv = dev_get_priv(dev);
	int ret;

	priv->mac_reg = (void *)pdata->iobase;

	ret = sun8i_emac_board_setup(dev, priv);
	if (ret)
		return ret;

	sun8i_emac_set_syscon(sun8i_pdata, priv);

	if (priv->phy_reg)
		regulator_set_enable(priv->phy_reg, true);

	sun8i_mdio_init(dev->name, dev);
	priv->bus = miiphy_get_dev_by_name(dev->name);

	return sun8i_phy_init(priv, dev);
}

static const struct eth_ops sun8i_emac_eth_ops = {
	.start                  = sun8i_emac_eth_start,
	.write_hwaddr           = sun8i_eth_write_hwaddr,
	.send                   = sun8i_emac_eth_send,
	.recv                   = sun8i_emac_eth_recv,
	.free_pkt               = sun8i_eth_free_pkt,
	.stop                   = sun8i_emac_eth_stop,
};

static int sun8i_handle_internal_phy(struct udevice *dev, struct emac_eth_dev *priv)
{
	struct ofnode_phandle_args phandle;
	int ret;

	ret = ofnode_parse_phandle_with_args(dev_ofnode(dev), "phy-handle",
					     NULL, 0, 0, &phandle);
	if (ret)
		return ret;

	/* If the PHY node is not a child of the internal MDIO bus, we are
	 * using some external PHY.
	 */
	if (!ofnode_device_is_compatible(ofnode_get_parent(phandle.node),
					 "allwinner,sun8i-h3-mdio-internal"))
		return 0;

	ret = clk_get_by_index_nodev(phandle.node, 0, &priv->ephy_clk);
	if (ret) {
		dev_err(dev, "failed to get EPHY TX clock\n");
		return ret;
	}

	ret = reset_get_by_index_nodev(phandle.node, 0, &priv->ephy_rst);
	if (ret) {
		dev_err(dev, "failed to get EPHY TX reset\n");
		return ret;
	}

	priv->use_internal_phy = true;

	return 0;
}

static int sun8i_emac_eth_of_to_plat(struct udevice *dev)
{
	struct sun8i_eth_pdata *sun8i_pdata = dev_get_plat(dev);
	struct eth_pdata *pdata = &sun8i_pdata->eth_pdata;
	struct emac_eth_dev *priv = dev_get_priv(dev);
	phys_addr_t syscon_base;
	const fdt32_t *reg;
	int node = dev_of_offset(dev);
	int offset = 0;
	int reset_flags = GPIOD_IS_OUT;
	int ret;

	pdata->iobase = dev_read_addr(dev);
	if (pdata->iobase == FDT_ADDR_T_NONE) {
		debug("%s: Cannot find MAC base address\n", __func__);
		return -EINVAL;
	}

	priv->variant = (const void *)dev_get_driver_data(dev);

	if (!priv->variant) {
		printf("%s: Missing variant\n", __func__);
		return -EINVAL;
	}

	ret = clk_get_by_name(dev, "stmmaceth", &priv->tx_clk);
	if (ret) {
		dev_err(dev, "failed to get TX clock\n");
		return ret;
	}

	ret = reset_get_by_name(dev, "stmmaceth", &priv->tx_rst);
	if (ret && ret != -ENOENT) {
		dev_err(dev, "failed to get TX reset\n");
		return ret;
	}

	offset = fdtdec_lookup_phandle(gd->fdt_blob, node, "syscon");
	if (offset < 0) {
		debug("%s: cannot find syscon node\n", __func__);
		return -EINVAL;
	}

	reg = fdt_getprop(gd->fdt_blob, offset, "reg", NULL);
	if (!reg) {
		debug("%s: cannot find reg property in syscon node\n",
		      __func__);
		return -EINVAL;
	}

	syscon_base = fdt_translate_address((void *)gd->fdt_blob, offset, reg);
	if (syscon_base == FDT_ADDR_T_NONE) {
		debug("%s: Cannot find syscon base address\n", __func__);
		return -EINVAL;
	}

	priv->sysctl_reg = (void *)syscon_base + priv->variant->syscon_offset;

	device_get_supply_regulator(dev, "phy-supply", &priv->phy_reg);

	pdata->phy_interface = -1;
	priv->phyaddr = -1;
	priv->use_internal_phy = false;

	offset = fdtdec_lookup_phandle(gd->fdt_blob, node, "phy-handle");
	if (offset >= 0)
		priv->phyaddr = fdtdec_get_int(gd->fdt_blob, offset, "reg", -1);

	pdata->phy_interface = dev_read_phy_mode(dev);
	debug("phy interface %d\n", pdata->phy_interface);
	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA)
		return -EINVAL;

	if (priv->variant->soc_has_internal_phy) {
		ret = sun8i_handle_internal_phy(dev, priv);
		if (ret)
			return ret;
	}

	priv->interface = pdata->phy_interface;

	sun8i_pdata->tx_delay_ps = fdtdec_get_int(gd->fdt_blob, node,
						  "allwinner,tx-delay-ps", 0);
	if (sun8i_pdata->tx_delay_ps < 0 || sun8i_pdata->tx_delay_ps > 700)
		printf("%s: Invalid TX delay value %d\n", __func__,
		       sun8i_pdata->tx_delay_ps);

	sun8i_pdata->rx_delay_ps = fdtdec_get_int(gd->fdt_blob, node,
						  "allwinner,rx-delay-ps", 0);
	if (sun8i_pdata->rx_delay_ps < 0 || sun8i_pdata->rx_delay_ps > 3100)
		printf("%s: Invalid RX delay value %d\n", __func__,
		       sun8i_pdata->rx_delay_ps);

	if (fdtdec_get_bool(gd->fdt_blob, dev_of_offset(dev),
			    "snps,reset-active-low"))
		reset_flags |= GPIOD_ACTIVE_LOW;

	ret = gpio_request_by_name(dev, "snps,reset-gpio", 0,
				   &priv->reset_gpio, reset_flags);

	if (ret == 0) {
		ret = fdtdec_get_int_array(gd->fdt_blob, dev_of_offset(dev),
					   "snps,reset-delays-us",
					   sun8i_pdata->reset_delays, 3);
	} else if (ret == -ENOENT) {
		ret = 0;
	}

	return 0;
}

static const struct emac_variant emac_variant_a83t = {
	.syscon_offset		= 0x30,
};

static const struct emac_variant emac_variant_h3 = {
	.syscon_offset		= 0x30,
	.soc_has_internal_phy	= true,
	.support_rmii		= true,
};

static const struct emac_variant emac_variant_r40 = {
	.syscon_offset		= 0x164,
};

static const struct emac_variant emac_variant_a64 = {
	.syscon_offset		= 0x30,
	.support_rmii		= true,
};

static const struct emac_variant emac_variant_h6 = {
	.syscon_offset		= 0x30,
	.support_rmii		= true,
};

static const struct udevice_id sun8i_emac_eth_ids[] = {
	{ .compatible = "allwinner,sun8i-a83t-emac",
	  .data = (ulong)&emac_variant_a83t },
	{ .compatible = "allwinner,sun8i-h3-emac",
	  .data = (ulong)&emac_variant_h3 },
	{ .compatible = "allwinner,sun8i-r40-gmac",
	  .data = (ulong)&emac_variant_r40 },
	{ .compatible = "allwinner,sun50i-a64-emac",
	  .data = (ulong)&emac_variant_a64 },
	{ .compatible = "allwinner,sun50i-h6-emac",
	  .data = (ulong)&emac_variant_h6 },
	{ }
};

U_BOOT_DRIVER(eth_sun8i_emac) = {
	.name   = "eth_sun8i_emac",
	.id     = UCLASS_ETH,
	.of_match = sun8i_emac_eth_ids,
	.of_to_plat = sun8i_emac_eth_of_to_plat,
	.probe  = sun8i_emac_eth_probe,
	.ops    = &sun8i_emac_eth_ops,
	.priv_auto	= sizeof(struct emac_eth_dev),
	.plat_auto	= sizeof(struct sun8i_eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
