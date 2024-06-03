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

#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fdt_support.h>
#include <linux/err.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <reset.h>
#include <dt-bindings/pinctrl/sun4i-a10.h>
#ifdef CONFIG_DM_GPIO
#include <asm-generic/gpio.h>
#endif

#define MDIO_CMD_MII_BUSY		BIT(0)
#define MDIO_CMD_MII_WRITE		BIT(1)

#define MDIO_CMD_MII_PHY_REG_ADDR_MASK	0x000001f0
#define MDIO_CMD_MII_PHY_REG_ADDR_SHIFT	4
#define MDIO_CMD_MII_PHY_ADDR_MASK	0x0001f000
#define MDIO_CMD_MII_PHY_ADDR_SHIFT	12

#define CONFIG_TX_DESCR_NUM	32
#define CONFIG_RX_DESCR_NUM	32
#define CONFIG_ETH_BUFSIZE	2048 /* Note must be dma aligned */

/*
 * The datasheet says that each descriptor can transfers up to 4096 bytes
 * But later, the register documentation reduces that value to 2048,
 * using 2048 cause strange behaviours and even BSP driver use 2047
 */
#define CONFIG_ETH_RXSIZE	2044 /* Note must fit in ETH_BUFSIZE */

#define TX_TOTAL_BUFSIZE	(CONFIG_ETH_BUFSIZE * CONFIG_TX_DESCR_NUM)
#define RX_TOTAL_BUFSIZE	(CONFIG_ETH_BUFSIZE * CONFIG_RX_DESCR_NUM)

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

#define CONFIG_MDIO_TIMEOUT	(3 * CONFIG_SYS_HZ)

#define AHB_GATE_OFFSET_EPHY	0

/* IO mux settings */
#define SUN8I_IOMUX_H3		2
#define SUN8I_IOMUX_R40	5
#define SUN8I_IOMUX		4

/* H3/A64 EMAC Register's offset */
#define EMAC_CTL0		0x00
#define EMAC_CTL1		0x04
#define EMAC_INT_STA		0x08
#define EMAC_INT_EN		0x0c
#define EMAC_TX_CTL0		0x10
#define EMAC_TX_CTL1		0x14
#define EMAC_TX_FLOW_CTL	0x1c
#define EMAC_TX_DMA_DESC	0x20
#define EMAC_RX_CTL0		0x24
#define EMAC_RX_CTL1		0x28
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

DECLARE_GLOBAL_DATA_PTR;

enum emac_variant {
	A83T_EMAC = 1,
	H3_EMAC,
	A64_EMAC,
	R40_GMAC,
};

struct emac_dma_desc {
	u32 status;
	u32 st;
	u32 buf_addr;
	u32 next;
} __aligned(ARCH_DMA_MINALIGN);

struct emac_eth_dev {
	struct emac_dma_desc rx_chain[CONFIG_TX_DESCR_NUM];
	struct emac_dma_desc tx_chain[CONFIG_RX_DESCR_NUM];
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

	enum emac_variant variant;
	void *mac_reg;
	phys_addr_t sysctl_reg;
	struct phy_device *phydev;
	struct mii_dev *bus;
	struct clk tx_clk;
	struct clk ephy_clk;
	struct reset_ctl tx_rst;
	struct reset_ctl ephy_rst;
#ifdef CONFIG_DM_GPIO
	struct gpio_desc reset_gpio;
#endif
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
	ulong start;
	u32 miiaddr = 0;
	int timeout = CONFIG_MDIO_TIMEOUT;

	miiaddr &= ~MDIO_CMD_MII_WRITE;
	miiaddr &= ~MDIO_CMD_MII_PHY_REG_ADDR_MASK;
	miiaddr |= (reg << MDIO_CMD_MII_PHY_REG_ADDR_SHIFT) &
		MDIO_CMD_MII_PHY_REG_ADDR_MASK;

	miiaddr &= ~MDIO_CMD_MII_PHY_ADDR_MASK;

	miiaddr |= (addr << MDIO_CMD_MII_PHY_ADDR_SHIFT) &
		MDIO_CMD_MII_PHY_ADDR_MASK;

	miiaddr |= MDIO_CMD_MII_BUSY;

	writel(miiaddr, priv->mac_reg + EMAC_MII_CMD);

	start = get_timer(0);
	while (get_timer(start) < timeout) {
		if (!(readl(priv->mac_reg + EMAC_MII_CMD) & MDIO_CMD_MII_BUSY))
			return readl(priv->mac_reg + EMAC_MII_DATA);
		udelay(10);
	};

	return -1;
}

static int sun8i_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			    u16 val)
{
	struct udevice *dev = bus->priv;
	struct emac_eth_dev *priv = dev_get_priv(dev);
	ulong start;
	u32 miiaddr = 0;
	int ret = -1, timeout = CONFIG_MDIO_TIMEOUT;

	miiaddr &= ~MDIO_CMD_MII_PHY_REG_ADDR_MASK;
	miiaddr |= (reg << MDIO_CMD_MII_PHY_REG_ADDR_SHIFT) &
		MDIO_CMD_MII_PHY_REG_ADDR_MASK;

	miiaddr &= ~MDIO_CMD_MII_PHY_ADDR_MASK;
	miiaddr |= (addr << MDIO_CMD_MII_PHY_ADDR_SHIFT) &
		MDIO_CMD_MII_PHY_ADDR_MASK;

	miiaddr |= MDIO_CMD_MII_WRITE;
	miiaddr |= MDIO_CMD_MII_BUSY;

	writel(val, priv->mac_reg + EMAC_MII_DATA);
	writel(miiaddr, priv->mac_reg + EMAC_MII_CMD);

	start = get_timer(0);
	while (get_timer(start) < timeout) {
		if (!(readl(priv->mac_reg + EMAC_MII_CMD) &
					MDIO_CMD_MII_BUSY)) {
			ret = 0;
			break;
		}
		udelay(10);
	};

	return ret;
}

static int _sun8i_write_hwaddr(struct emac_eth_dev *priv, u8 *mac_id)
{
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
		v |= BIT(0);
	else
		v &= ~BIT(0);

	v &= ~0x0C;

	switch (phydev->speed) {
	case 1000:
		break;
	case 100:
		v |= BIT(2);
		v |= BIT(3);
		break;
	case 10:
		v |= BIT(3);
		break;
	}
	writel(v, priv->mac_reg + EMAC_CTL0);
}

static int sun8i_emac_set_syscon_ephy(struct emac_eth_dev *priv, u32 *reg)
{
	if (priv->use_internal_phy) {
		/* H3 based SoC's that has an Internal 100MBit PHY
		 * needs to be configured and powered up before use
		*/
		*reg &= ~H3_EPHY_DEFAULT_MASK;
		*reg |=  H3_EPHY_DEFAULT_VALUE;
		*reg |= priv->phyaddr << H3_EPHY_ADDR_SHIFT;
		*reg &= ~H3_EPHY_SHUTDOWN;
		*reg |= H3_EPHY_SELECT;
	} else
		/* This is to select External Gigabit PHY on
		 * the boards with H3 SoC.
		*/
		*reg &= ~H3_EPHY_SELECT;

	return 0;
}

static int sun8i_emac_set_syscon(struct sun8i_eth_pdata *pdata,
				 struct emac_eth_dev *priv)
{
	int ret;
	u32 reg;

	if (priv->variant == R40_GMAC) {
		/* Select RGMII for R40 */
		reg = readl(priv->sysctl_reg + 0x164);
		reg |= CCM_GMAC_CTRL_TX_CLK_SRC_INT_RGMII |
		       CCM_GMAC_CTRL_GPIT_RGMII |
		       CCM_GMAC_CTRL_TX_CLK_DELAY(CONFIG_GMAC_TX_DELAY);

		writel(reg, priv->sysctl_reg + 0x164);
		return 0;
	}

	reg = readl(priv->sysctl_reg + 0x30);

	if (priv->variant == H3_EMAC) {
		ret = sun8i_emac_set_syscon_ephy(priv, &reg);
		if (ret)
			return ret;
	}

	reg &= ~(SC_ETCS_MASK | SC_EPIT);
	if (priv->variant == H3_EMAC || priv->variant == A64_EMAC)
		reg &= ~SC_RMII_EN;

	switch (priv->interface) {
	case PHY_INTERFACE_MODE_MII:
		/* default */
		break;
	case PHY_INTERFACE_MODE_RGMII:
		reg |= SC_EPIT | SC_ETCS_INT_GMII;
		break;
	case PHY_INTERFACE_MODE_RMII:
		if (priv->variant == H3_EMAC ||
		    priv->variant == A64_EMAC) {
			reg |= SC_RMII_EN | SC_ETCS_EXT_GMII;
		break;
		}
		/* RMII not supported on A83T */
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

	writel(reg, priv->sysctl_reg + 0x30);

	return 0;
}

static int sun8i_phy_init(struct emac_eth_dev *priv, void *dev)
{
	struct phy_device *phydev;

	phydev = phy_connect(priv->bus, priv->phyaddr, dev, priv->interface);
	if (!phydev)
		return -ENODEV;

	phy_connect_dev(phydev, dev);

	priv->phydev = phydev;
	phy_config(priv->phydev);

	return 0;
}

static void rx_descs_init(struct emac_eth_dev *priv)
{
	struct emac_dma_desc *desc_table_p = &priv->rx_chain[0];
	char *rxbuffs = &priv->rxbuffer[0];
	struct emac_dma_desc *desc_p;
	u32 idx;

	/* flush Rx buffers */
	flush_dcache_range((uintptr_t)rxbuffs, (ulong)rxbuffs +
			RX_TOTAL_BUFSIZE);

	for (idx = 0; idx < CONFIG_RX_DESCR_NUM; idx++) {
		desc_p = &desc_table_p[idx];
		desc_p->buf_addr = (uintptr_t)&rxbuffs[idx * CONFIG_ETH_BUFSIZE]
			;
		desc_p->next = (uintptr_t)&desc_table_p[idx + 1];
		desc_p->st |= CONFIG_ETH_RXSIZE;
		desc_p->status = BIT(31);
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
	u32 idx;

	for (idx = 0; idx < CONFIG_TX_DESCR_NUM; idx++) {
		desc_p = &desc_table_p[idx];
		desc_p->buf_addr = (uintptr_t)&txbuffs[idx * CONFIG_ETH_BUFSIZE]
			;
		desc_p->next = (uintptr_t)&desc_table_p[idx + 1];
		desc_p->status = (1 << 31);
		desc_p->st = 0;
	}

	/* Correcting the last pointer of the chain */
	desc_p->next =  (uintptr_t)&desc_table_p[0];

	/* Flush all Tx buffer descriptors */
	flush_dcache_range((uintptr_t)priv->tx_chain,
			   (uintptr_t)priv->tx_chain +
			sizeof(priv->tx_chain));

	writel((uintptr_t)&desc_table_p[0], priv->mac_reg + EMAC_TX_DMA_DESC);
	priv->tx_currdescnum = 0;
}

static int _sun8i_emac_eth_init(struct emac_eth_dev *priv, u8 *enetaddr)
{
	u32 reg, v;
	int timeout = 100;

	reg = readl((priv->mac_reg + EMAC_CTL1));

	if (!(reg & 0x1)) {
		/* Soft reset MAC */
		setbits_le32((priv->mac_reg + EMAC_CTL1), 0x1);
		do {
			reg = readl(priv->mac_reg + EMAC_CTL1);
		} while ((reg & 0x01) != 0 &&  (--timeout));
		if (!timeout) {
			printf("%s: Timeout\n", __func__);
			return -1;
		}
	}

	/* Rewrite mac address after reset */
	_sun8i_write_hwaddr(priv, enetaddr);

	v = readl(priv->mac_reg + EMAC_TX_CTL1);
	/* TX_MD Transmission starts after a full frame located in TX DMA FIFO*/
	v |= BIT(1);
	writel(v, priv->mac_reg + EMAC_TX_CTL1);

	v = readl(priv->mac_reg + EMAC_RX_CTL1);
	/* RX_MD RX DMA reads data from RX DMA FIFO to host memory after a
	 * complete frame has been written to RX DMA FIFO
	 */
	v |= BIT(1);
	writel(v, priv->mac_reg + EMAC_RX_CTL1);

	/* DMA */
	writel(8 << 24, priv->mac_reg + EMAC_CTL1);

	/* Initialize rx/tx descriptors */
	rx_descs_init(priv);
	tx_descs_init(priv);

	/* PHY Start Up */
	phy_startup(priv->phydev);

	sun8i_adjust_link(priv, priv->phydev);

	/* Start RX DMA */
	v = readl(priv->mac_reg + EMAC_RX_CTL1);
	v |= BIT(30);
	writel(v, priv->mac_reg + EMAC_RX_CTL1);
	/* Start TX DMA */
	v = readl(priv->mac_reg + EMAC_TX_CTL1);
	v |= BIT(30);
	writel(v, priv->mac_reg + EMAC_TX_CTL1);

	/* Enable RX/TX */
	setbits_le32(priv->mac_reg + EMAC_RX_CTL0, BIT(31));
	setbits_le32(priv->mac_reg + EMAC_TX_CTL0, BIT(31));

	return 0;
}

static int parse_phy_pins(struct udevice *dev)
{
	struct emac_eth_dev *priv = dev_get_priv(dev);
	int offset;
	const char *pin_name;
	int drive, pull = SUN4I_PINCTRL_NO_PULL, i;

	offset = fdtdec_lookup_phandle(gd->fdt_blob, dev_of_offset(dev),
				       "pinctrl-0");
	if (offset < 0) {
		printf("WARNING: emac: cannot find pinctrl-0 node\n");
		return offset;
	}

	drive = fdt_getprop_u32_default_node(gd->fdt_blob, offset, 0,
					     "drive-strength", ~0);
	if (drive != ~0) {
		if (drive <= 10)
			drive = SUN4I_PINCTRL_10_MA;
		else if (drive <= 20)
			drive = SUN4I_PINCTRL_20_MA;
		else if (drive <= 30)
			drive = SUN4I_PINCTRL_30_MA;
		else
			drive = SUN4I_PINCTRL_40_MA;
	}

	if (fdt_get_property(gd->fdt_blob, offset, "bias-pull-up", NULL))
		pull = SUN4I_PINCTRL_PULL_UP;
	else if (fdt_get_property(gd->fdt_blob, offset, "bias-pull-down", NULL))
		pull = SUN4I_PINCTRL_PULL_DOWN;

	for (i = 0; ; i++) {
		int pin;

		pin_name = fdt_stringlist_get(gd->fdt_blob, offset,
					      "pins", i, NULL);
		if (!pin_name)
			break;

		pin = sunxi_name_to_gpio(pin_name);
		if (pin < 0)
			continue;

		if (priv->variant == H3_EMAC)
			sunxi_gpio_set_cfgpin(pin, SUN8I_IOMUX_H3);
		else if (priv->variant == R40_GMAC)
			sunxi_gpio_set_cfgpin(pin, SUN8I_IOMUX_R40);
		else
			sunxi_gpio_set_cfgpin(pin, SUN8I_IOMUX);

		if (drive != ~0)
			sunxi_gpio_set_drv(pin, drive);
		if (pull != ~0)
			sunxi_gpio_set_pull(pin, pull);
	}

	if (!i) {
		printf("WARNING: emac: cannot find pins property\n");
		return -2;
	}

	return 0;
}

static int _sun8i_eth_recv(struct emac_eth_dev *priv, uchar **packetp)
{
	u32 status, desc_num = priv->rx_currdescnum;
	struct emac_dma_desc *desc_p = &priv->rx_chain[desc_num];
	int length = -EAGAIN;
	int good_packet = 1;
	uintptr_t desc_start = (uintptr_t)desc_p;
	uintptr_t desc_end = desc_start +
		roundup(sizeof(*desc_p), ARCH_DMA_MINALIGN);

	ulong data_start = (uintptr_t)desc_p->buf_addr;
	ulong data_end;

	/* Invalidate entire buffer descriptor */
	invalidate_dcache_range(desc_start, desc_end);

	status = desc_p->status;

	/* Check for DMA own bit */
	if (!(status & BIT(31))) {
		length = (desc_p->status >> 16) & 0x3FFF;

		if (length < 0x40) {
			good_packet = 0;
			debug("RX: Bad Packet (runt)\n");
		}

		data_end = data_start + length;
		/* Invalidate received data */
		invalidate_dcache_range(rounddown(data_start,
						  ARCH_DMA_MINALIGN),
					roundup(data_end,
						ARCH_DMA_MINALIGN));
		if (good_packet) {
			if (length > CONFIG_ETH_RXSIZE) {
				printf("Received packet is too big (len=%d)\n",
				       length);
				return -EMSGSIZE;
			}
			*packetp = (uchar *)(ulong)desc_p->buf_addr;
			return length;
		}
	}

	return length;
}

static int _sun8i_emac_eth_send(struct emac_eth_dev *priv, void *packet,
				int len)
{
	u32 v, desc_num = priv->tx_currdescnum;
	struct emac_dma_desc *desc_p = &priv->tx_chain[desc_num];
	uintptr_t desc_start = (uintptr_t)desc_p;
	uintptr_t desc_end = desc_start +
		roundup(sizeof(*desc_p), ARCH_DMA_MINALIGN);

	uintptr_t data_start = (uintptr_t)desc_p->buf_addr;
	uintptr_t data_end = data_start +
		roundup(len, ARCH_DMA_MINALIGN);

	/* Invalidate entire buffer descriptor */
	invalidate_dcache_range(desc_start, desc_end);

	desc_p->st = len;
	/* Mandatory undocumented bit */
	desc_p->st |= BIT(24);

	memcpy((void *)data_start, packet, len);

	/* Flush data to be sent */
	flush_dcache_range(data_start, data_end);

	/* frame end */
	desc_p->st |= BIT(30);
	desc_p->st |= BIT(31);

	/*frame begin */
	desc_p->st |= BIT(29);
	desc_p->status = BIT(31);

	/*Descriptors st and status field has changed, so FLUSH it */
	flush_dcache_range(desc_start, desc_end);

	/* Move to next Descriptor and wrap around */
	if (++desc_num >= CONFIG_TX_DESCR_NUM)
		desc_num = 0;
	priv->tx_currdescnum = desc_num;

	/* Start the DMA */
	v = readl(priv->mac_reg + EMAC_TX_CTL1);
	v |= BIT(31);/* mandatory */
	v |= BIT(30);/* mandatory */
	writel(v, priv->mac_reg + EMAC_TX_CTL1);

	return 0;
}

static int sun8i_eth_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct emac_eth_dev *priv = dev_get_priv(dev);

	return _sun8i_write_hwaddr(priv, pdata->enetaddr);
}

static int sun8i_emac_board_setup(struct emac_eth_dev *priv)
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

#if defined(CONFIG_DM_GPIO)
static int sun8i_mdio_reset(struct mii_dev *bus)
{
	struct udevice *dev = bus->priv;
	struct emac_eth_dev *priv = dev_get_priv(dev);
	struct sun8i_eth_pdata *pdata = dev_get_platdata(dev);
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
#endif

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
#if defined(CONFIG_DM_GPIO)
	bus->reset = sun8i_mdio_reset;
#endif

	return  mdio_register(bus);
}

static int sun8i_emac_eth_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);

	return _sun8i_emac_eth_init(dev->priv, pdata->enetaddr);
}

static int sun8i_emac_eth_send(struct udevice *dev, void *packet, int length)
{
	struct emac_eth_dev *priv = dev_get_priv(dev);

	return _sun8i_emac_eth_send(priv, packet, length);
}

static int sun8i_emac_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct emac_eth_dev *priv = dev_get_priv(dev);

	return _sun8i_eth_recv(priv, packetp);
}

static int _sun8i_free_pkt(struct emac_eth_dev *priv)
{
	u32 desc_num = priv->rx_currdescnum;
	struct emac_dma_desc *desc_p = &priv->rx_chain[desc_num];
	uintptr_t desc_start = (uintptr_t)desc_p;
	uintptr_t desc_end = desc_start +
		roundup(sizeof(u32), ARCH_DMA_MINALIGN);

	/* Make the current descriptor valid again */
	desc_p->status |= BIT(31);

	/* Flush Status field of descriptor */
	flush_dcache_range(desc_start, desc_end);

	/* Move to next desc and wrap-around condition. */
	if (++desc_num >= CONFIG_RX_DESCR_NUM)
		desc_num = 0;
	priv->rx_currdescnum = desc_num;

	return 0;
}

static int sun8i_eth_free_pkt(struct udevice *dev, uchar *packet,
			      int length)
{
	struct emac_eth_dev *priv = dev_get_priv(dev);

	return _sun8i_free_pkt(priv);
}

static void sun8i_emac_eth_stop(struct udevice *dev)
{
	struct emac_eth_dev *priv = dev_get_priv(dev);

	/* Stop Rx/Tx transmitter */
	clrbits_le32(priv->mac_reg + EMAC_RX_CTL0, BIT(31));
	clrbits_le32(priv->mac_reg + EMAC_TX_CTL0, BIT(31));

	/* Stop TX DMA */
	clrbits_le32(priv->mac_reg + EMAC_TX_CTL1, BIT(30));

	phy_shutdown(priv->phydev);
}

static int sun8i_emac_eth_probe(struct udevice *dev)
{
	struct sun8i_eth_pdata *sun8i_pdata = dev_get_platdata(dev);
	struct eth_pdata *pdata = &sun8i_pdata->eth_pdata;
	struct emac_eth_dev *priv = dev_get_priv(dev);
	int ret;

	priv->mac_reg = (void *)pdata->iobase;

	ret = sun8i_emac_board_setup(priv);
	if (ret)
		return ret;

	sun8i_emac_set_syscon(sun8i_pdata, priv);

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

static int sun8i_get_ephy_nodes(struct emac_eth_dev *priv)
{
	int node, ret;

	/* look for mdio-mux node for internal PHY node */
	node = fdt_path_offset(gd->fdt_blob,
			"/soc/ethernet@1c30000/mdio-mux/mdio@1/ethernet-phy@1");
	if (node < 0) {
		debug("failed to get mdio-mux with internal PHY\n");
		return node;
	}

	ret = fdt_node_check_compatible(gd->fdt_blob, node,
					"allwinner,sun8i-h3-mdio-internal");
	if (ret < 0) {
		debug("failed to find mdio-internal node\n");
		return ret;
	}

	ret = clk_get_by_index_nodev(offset_to_ofnode(node), 0,
				     &priv->ephy_clk);
	if (ret) {
		dev_err(dev, "failed to get EPHY TX clock\n");
		return ret;
	}

	ret = reset_get_by_index_nodev(offset_to_ofnode(node), 0,
				       &priv->ephy_rst);
	if (ret) {
		dev_err(dev, "failed to get EPHY TX reset\n");
		return ret;
	}

	priv->use_internal_phy = true;

	return 0;
}

static int sun8i_emac_eth_ofdata_to_platdata(struct udevice *dev)
{
	struct sun8i_eth_pdata *sun8i_pdata = dev_get_platdata(dev);
	struct eth_pdata *pdata = &sun8i_pdata->eth_pdata;
	struct emac_eth_dev *priv = dev_get_priv(dev);
	const char *phy_mode;
	const fdt32_t *reg;
	int node = dev_of_offset(dev);
	int offset = 0;
#ifdef CONFIG_DM_GPIO
	int reset_flags = GPIOD_IS_OUT;
#endif
	int ret;

	pdata->iobase = devfdt_get_addr(dev);
	if (pdata->iobase == FDT_ADDR_T_NONE) {
		debug("%s: Cannot find MAC base address\n", __func__);
		return -EINVAL;
	}

	priv->variant = dev_get_driver_data(dev);

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
	priv->sysctl_reg = fdt_translate_address((void *)gd->fdt_blob,
						 offset, reg);
	if (priv->sysctl_reg == FDT_ADDR_T_NONE) {
		debug("%s: Cannot find syscon base address\n", __func__);
		return -EINVAL;
	}

	pdata->phy_interface = -1;
	priv->phyaddr = -1;
	priv->use_internal_phy = false;

	offset = fdtdec_lookup_phandle(gd->fdt_blob, node, "phy-handle");
	if (offset < 0) {
		debug("%s: Cannot find PHY address\n", __func__);
		return -EINVAL;
	}
	priv->phyaddr = fdtdec_get_int(gd->fdt_blob, offset, "reg", -1);

	phy_mode = fdt_getprop(gd->fdt_blob, node, "phy-mode", NULL);

	if (phy_mode)
		pdata->phy_interface = phy_get_interface_by_name(phy_mode);
	printf("phy interface%d\n", pdata->phy_interface);

	if (pdata->phy_interface == -1) {
		debug("%s: Invalid PHY interface '%s'\n", __func__, phy_mode);
		return -EINVAL;
	}

	if (priv->variant == H3_EMAC) {
		ret = sun8i_get_ephy_nodes(priv);
		if (ret)
			return ret;
	}

	priv->interface = pdata->phy_interface;

	if (!priv->use_internal_phy)
		parse_phy_pins(dev);

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

#ifdef CONFIG_DM_GPIO
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
#endif

	return 0;
}

static const struct udevice_id sun8i_emac_eth_ids[] = {
	{.compatible = "allwinner,sun8i-h3-emac", .data = (uintptr_t)H3_EMAC },
	{.compatible = "allwinner,sun50i-a64-emac",
		.data = (uintptr_t)A64_EMAC },
	{.compatible = "allwinner,sun8i-a83t-emac",
		.data = (uintptr_t)A83T_EMAC },
	{.compatible = "allwinner,sun8i-r40-gmac",
		.data = (uintptr_t)R40_GMAC },
	{ }
};

U_BOOT_DRIVER(eth_sun8i_emac) = {
	.name   = "eth_sun8i_emac",
	.id     = UCLASS_ETH,
	.of_match = sun8i_emac_eth_ids,
	.ofdata_to_platdata = sun8i_emac_eth_ofdata_to_platdata,
	.probe  = sun8i_emac_eth_probe,
	.ops    = &sun8i_emac_eth_ops,
	.priv_auto_alloc_size = sizeof(struct emac_eth_dev),
	.platdata_auto_alloc_size = sizeof(struct sun8i_eth_pdata),
	.flags = DM_FLAG_ALLOC_PRIV_DMA,
};
