// SPDX-License-Identifier: GPL-2.0+
/**
 * sni_ave.c - Socionext UniPhier AVE ethernet driver
 * Copyright 2016-2018 Socionext inc.
 */

#include <clk.h>
#include <dm.h>
#include <fdt_support.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <miiphy.h>
#include <net.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>

#define AVE_GRST_DELAY_MSEC	40
#define AVE_MIN_XMITSIZE	60
#define AVE_SEND_TIMEOUT_COUNT	1000
#define AVE_MDIO_TIMEOUT_USEC	10000
#define AVE_HALT_TIMEOUT_USEC	10000

/* General Register Group */
#define AVE_IDR			0x000	/* ID */
#define AVE_VR			0x004	/* Version */
#define AVE_GRR			0x008	/* Global Reset */
#define AVE_CFGR		0x00c	/* Configuration */

/* Interrupt Register Group */
#define AVE_GIMR		0x100	/* Global Interrupt Mask */
#define AVE_GISR		0x104	/* Global Interrupt Status */

/* MAC Register Group */
#define AVE_TXCR		0x200	/* TX Setup */
#define AVE_RXCR		0x204	/* RX Setup */
#define AVE_RXMAC1R		0x208	/* MAC address (lower) */
#define AVE_RXMAC2R		0x20c	/* MAC address (upper) */
#define AVE_MDIOCTR		0x214	/* MDIO Control */
#define AVE_MDIOAR		0x218	/* MDIO Address */
#define AVE_MDIOWDR		0x21c	/* MDIO Data */
#define AVE_MDIOSR		0x220	/* MDIO Status */
#define AVE_MDIORDR		0x224	/* MDIO Rd Data */

/* Descriptor Control Register Group */
#define AVE_DESCC		0x300	/* Descriptor Control */
#define AVE_TXDC		0x304	/* TX Descriptor Configuration */
#define AVE_RXDC		0x308	/* RX Descriptor Ring0 Configuration */
#define AVE_IIRQC		0x34c	/* Interval IRQ Control */

/* 64bit descriptor memory */
#define AVE_DESC_SIZE_64	12	/* Descriptor Size */
#define AVE_TXDM_64		0x1000	/* Tx Descriptor Memory */
#define AVE_RXDM_64		0x1c00	/* Rx Descriptor Memory */

/* 32bit descriptor memory */
#define AVE_DESC_SIZE_32	8	/* Descriptor Size */
#define AVE_TXDM_32		0x1000	/* Tx Descriptor Memory */
#define AVE_RXDM_32		0x1800	/* Rx Descriptor Memory */

/* RMII Bridge Register Group */
#define AVE_RSTCTRL		0x8028	/* Reset control */
#define AVE_RSTCTRL_RMIIRST	BIT(16)
#define AVE_LINKSEL		0x8034	/* Link speed setting */
#define AVE_LINKSEL_100M	BIT(0)

/* AVE_GRR */
#define AVE_GRR_PHYRST		BIT(4)	/* Reset external PHY */
#define AVE_GRR_GRST		BIT(0)	/* Reset all MAC */

/* AVE_CFGR */
#define AVE_CFGR_MII		BIT(27)	/* Func mode (1:MII/RMII, 0:RGMII) */

/* AVE_GISR (common with GIMR) */
#define AVE_GIMR_CLR		0
#define AVE_GISR_CLR		GENMASK(31, 0)

/* AVE_TXCR */
#define AVE_TXCR_FLOCTR		BIT(18)	/* Flow control */
#define AVE_TXCR_TXSPD_1G	BIT(17)
#define AVE_TXCR_TXSPD_100	BIT(16)

/* AVE_RXCR */
#define AVE_RXCR_RXEN		BIT(30)	/* Rx enable */
#define AVE_RXCR_FDUPEN		BIT(22)	/* Interface mode */
#define AVE_RXCR_FLOCTR		BIT(21)	/* Flow control */

/* AVE_MDIOCTR */
#define AVE_MDIOCTR_RREQ	BIT(3)	/* Read request */
#define AVE_MDIOCTR_WREQ	BIT(2)	/* Write request */

/* AVE_MDIOSR */
#define AVE_MDIOSR_STS		BIT(0)	/* access status */

/* AVE_DESCC */
#define AVE_DESCC_RXDSTPSTS	BIT(20)
#define AVE_DESCC_RD0		BIT(8)	/* Enable Rx descriptor Ring0 */
#define AVE_DESCC_RXDSTP	BIT(4)	/* Pause Rx descriptor */
#define AVE_DESCC_TD		BIT(0)	/* Enable Tx descriptor */

/* AVE_TXDC/RXDC */
#define AVE_DESC_SIZE(priv, num) \
	((num) * ((priv)->data->is_desc_64bit ? AVE_DESC_SIZE_64 :	\
		  AVE_DESC_SIZE_32))

/* Command status for descriptor */
#define AVE_STS_OWN		BIT(31)	/* Descriptor ownership */
#define AVE_STS_OK		BIT(27)	/* Normal transmit */
#define AVE_STS_1ST		BIT(26)	/* Head of buffer chain */
#define AVE_STS_LAST		BIT(25)	/* Tail of buffer chain */
#define AVE_STS_PKTLEN_TX_MASK	GENMASK(15, 0)
#define AVE_STS_PKTLEN_RX_MASK	GENMASK(10, 0)

#define AVE_DESC_OFS_CMDSTS	0
#define AVE_DESC_OFS_ADDRL	4
#define AVE_DESC_OFS_ADDRU	8

/* Parameter for ethernet frame */
#define AVE_RXCR_MTU		1518

/* SG */
#define SG_ETPINMODE		0x540
#define SG_ETPINMODE_EXTPHY	BIT(1)	/* for LD11 */
#define SG_ETPINMODE_RMII(ins)	BIT(ins)

#define AVE_MAX_CLKS		4
#define AVE_MAX_RSTS		2

enum desc_id {
	AVE_DESCID_TX,
	AVE_DESCID_RX,
};

struct ave_private {
	phys_addr_t iobase;
	unsigned int nclks;
	struct clk clk[AVE_MAX_CLKS];
	unsigned int nrsts;
	struct reset_ctl rst[AVE_MAX_RSTS];
	struct regmap *regmap;
	unsigned int regmap_arg;

	struct mii_dev *bus;
	struct phy_device *phydev;
	int phy_mode;
	int max_speed;

	int rx_pos;
	int rx_siz;
	int rx_off;
	int tx_num;

	u8 tx_adj_packetbuf[PKTSIZE_ALIGN + PKTALIGN];
	void *tx_adj_buf;

	const struct ave_soc_data *data;
};

struct ave_soc_data {
	bool	is_desc_64bit;
	const char	*clock_names[AVE_MAX_CLKS];
	const char	*reset_names[AVE_MAX_RSTS];
	int	(*get_pinmode)(struct ave_private *priv);
};

static u32 ave_desc_read(struct ave_private *priv, enum desc_id id, int entry,
			 int offset)
{
	int desc_size;
	u32 addr;

	if (priv->data->is_desc_64bit) {
		desc_size = AVE_DESC_SIZE_64;
		addr = (id == AVE_DESCID_TX) ? AVE_TXDM_64 : AVE_RXDM_64;
	} else {
		desc_size = AVE_DESC_SIZE_32;
		addr = (id == AVE_DESCID_TX) ? AVE_TXDM_32 : AVE_RXDM_32;
	}

	addr += entry * desc_size + offset;

	return readl(priv->iobase + addr);
}

static u32 ave_desc_read_cmdsts(struct ave_private *priv, enum desc_id id,
				int entry)
{
	return ave_desc_read(priv, id, entry, AVE_DESC_OFS_CMDSTS);
}

static void ave_desc_write(struct ave_private *priv, enum desc_id id,
			   int entry, int offset, u32 val)
{
	int desc_size;
	u32 addr;

	if (priv->data->is_desc_64bit) {
		desc_size = AVE_DESC_SIZE_64;
		addr = (id == AVE_DESCID_TX) ? AVE_TXDM_64 : AVE_RXDM_64;
	} else {
		desc_size = AVE_DESC_SIZE_32;
		addr = (id == AVE_DESCID_TX) ? AVE_TXDM_32 : AVE_RXDM_32;
	}

	addr += entry * desc_size + offset;
	writel(val, priv->iobase + addr);
}

static void ave_desc_write_cmdsts(struct ave_private *priv, enum desc_id id,
				  int entry, u32 val)
{
	ave_desc_write(priv, id, entry, AVE_DESC_OFS_CMDSTS, val);
}

static void ave_desc_write_addr(struct ave_private *priv, enum desc_id id,
				int entry, uintptr_t paddr)
{
	ave_desc_write(priv, id, entry,
		       AVE_DESC_OFS_ADDRL, lower_32_bits(paddr));
	if (priv->data->is_desc_64bit)
		ave_desc_write(priv, id, entry,
			       AVE_DESC_OFS_ADDRU, upper_32_bits(paddr));
}

static void ave_cache_invalidate(uintptr_t vaddr, int len)
{
	invalidate_dcache_range(rounddown(vaddr, ARCH_DMA_MINALIGN),
				roundup(vaddr + len, ARCH_DMA_MINALIGN));
}

static void ave_cache_flush(uintptr_t vaddr, int len)
{
	flush_dcache_range(rounddown(vaddr, ARCH_DMA_MINALIGN),
			   roundup(vaddr + len, ARCH_DMA_MINALIGN));
}

static int ave_mdiobus_read(struct mii_dev *bus,
			    int phyid, int devad, int regnum)
{
	struct ave_private *priv = bus->priv;
	u32 mdioctl, mdiosr;
	int ret;

	/* write address */
	writel((phyid << 8) | regnum, priv->iobase + AVE_MDIOAR);

	/* read request */
	mdioctl = readl(priv->iobase + AVE_MDIOCTR);
	writel(mdioctl | AVE_MDIOCTR_RREQ, priv->iobase + AVE_MDIOCTR);

	ret = readl_poll_timeout(priv->iobase + AVE_MDIOSR, mdiosr,
				 !(mdiosr & AVE_MDIOSR_STS),
				 AVE_MDIO_TIMEOUT_USEC);
	if (ret) {
		pr_err("%s: failed to read from mdio (phy:%d reg:%x)\n",
		       priv->phydev->dev->name, phyid, regnum);
		return ret;
	}

	return readl(priv->iobase + AVE_MDIORDR) & GENMASK(15, 0);
}

static int ave_mdiobus_write(struct mii_dev *bus,
			     int phyid, int devad, int regnum, u16 val)
{
	struct ave_private *priv = bus->priv;
	u32 mdioctl, mdiosr;
	int ret;

	/* write address */
	writel((phyid << 8) | regnum, priv->iobase + AVE_MDIOAR);

	/* write data */
	writel(val, priv->iobase + AVE_MDIOWDR);

	/* write request */
	mdioctl = readl(priv->iobase + AVE_MDIOCTR);
	writel((mdioctl | AVE_MDIOCTR_WREQ) & ~AVE_MDIOCTR_RREQ,
	       priv->iobase + AVE_MDIOCTR);

	ret = readl_poll_timeout(priv->iobase + AVE_MDIOSR, mdiosr,
				 !(mdiosr & AVE_MDIOSR_STS),
				 AVE_MDIO_TIMEOUT_USEC);
	if (ret)
		pr_err("%s: failed to write to mdio (phy:%d reg:%x)\n",
		       priv->phydev->dev->name, phyid, regnum);

	return ret;
}

static int ave_adjust_link(struct ave_private *priv)
{
	struct phy_device *phydev = priv->phydev;
	struct eth_pdata *pdata = dev_get_platdata(phydev->dev);
	u32 val, txcr, rxcr, rxcr_org;
	u16 rmt_adv = 0, lcl_adv = 0;
	u8 cap;

	/* set RGMII speed */
	val = readl(priv->iobase + AVE_TXCR);
	val &= ~(AVE_TXCR_TXSPD_100 | AVE_TXCR_TXSPD_1G);

	if (phy_interface_is_rgmii(phydev) && phydev->speed == SPEED_1000)
		val |= AVE_TXCR_TXSPD_1G;
	else if (phydev->speed == SPEED_100)
		val |= AVE_TXCR_TXSPD_100;

	writel(val, priv->iobase + AVE_TXCR);

	/* set RMII speed (100M/10M only)  */
	if (!phy_interface_is_rgmii(phydev)) {
		val = readl(priv->iobase + AVE_LINKSEL);
		if (phydev->speed == SPEED_10)
			val &= ~AVE_LINKSEL_100M;
		else
			val |= AVE_LINKSEL_100M;
		writel(val, priv->iobase + AVE_LINKSEL);
	}

	/* check current RXCR/TXCR */
	rxcr = readl(priv->iobase + AVE_RXCR);
	txcr = readl(priv->iobase + AVE_TXCR);
	rxcr_org = rxcr;

	if (phydev->duplex) {
		rxcr |= AVE_RXCR_FDUPEN;

		if (phydev->pause)
			rmt_adv |= LPA_PAUSE_CAP;
		if (phydev->asym_pause)
			rmt_adv |= LPA_PAUSE_ASYM;
		if (phydev->advertising & ADVERTISED_Pause)
			lcl_adv |= ADVERTISE_PAUSE_CAP;
		if (phydev->advertising & ADVERTISED_Asym_Pause)
			lcl_adv |= ADVERTISE_PAUSE_ASYM;

		cap = mii_resolve_flowctrl_fdx(lcl_adv, rmt_adv);
		if (cap & FLOW_CTRL_TX)
			txcr |= AVE_TXCR_FLOCTR;
		else
			txcr &= ~AVE_TXCR_FLOCTR;
		if (cap & FLOW_CTRL_RX)
			rxcr |= AVE_RXCR_FLOCTR;
		else
			rxcr &= ~AVE_RXCR_FLOCTR;
	} else {
		rxcr &= ~AVE_RXCR_FDUPEN;
		rxcr &= ~AVE_RXCR_FLOCTR;
		txcr &= ~AVE_TXCR_FLOCTR;
	}

	if (rxcr_org != rxcr) {
		/* disable Rx mac */
		writel(rxcr & ~AVE_RXCR_RXEN, priv->iobase + AVE_RXCR);
		/* change and enable TX/Rx mac */
		writel(txcr, priv->iobase + AVE_TXCR);
		writel(rxcr, priv->iobase + AVE_RXCR);
	}

	pr_notice("%s: phy:%s speed:%d mac:%pM\n",
		  phydev->dev->name, phydev->drv->name, phydev->speed,
		  pdata->enetaddr);

	return phydev->link;
}

static int ave_mdiobus_init(struct ave_private *priv, const char *name)
{
	struct mii_dev *bus = mdio_alloc();

	if (!bus)
		return -ENOMEM;

	bus->read = ave_mdiobus_read;
	bus->write = ave_mdiobus_write;
	snprintf(bus->name, sizeof(bus->name), "%s", name);
	bus->priv = priv;

	return mdio_register(bus);
}

static int ave_phy_init(struct ave_private *priv, void *dev)
{
	struct phy_device *phydev;
	int mask = GENMASK(31, 0), ret;

	phydev = phy_find_by_mask(priv->bus, mask, priv->phy_mode);
	if (!phydev)
		return -ENODEV;

	phy_connect_dev(phydev, dev);

	phydev->supported &= PHY_GBIT_FEATURES;
	if (priv->max_speed) {
		ret = phy_set_supported(phydev, priv->max_speed);
		if (ret)
			return ret;
	}
	phydev->advertising = phydev->supported;

	priv->phydev = phydev;
	phy_config(phydev);

	return 0;
}

static void ave_stop(struct udevice *dev)
{
	struct ave_private *priv = dev_get_priv(dev);
	u32 val;
	int ret;

	val = readl(priv->iobase + AVE_GRR);
	if (val)
		return;

	val = readl(priv->iobase + AVE_RXCR);
	val &= ~AVE_RXCR_RXEN;
	writel(val, priv->iobase + AVE_RXCR);

	writel(0, priv->iobase + AVE_DESCC);
	ret = readl_poll_timeout(priv->iobase + AVE_DESCC, val, !val,
				 AVE_HALT_TIMEOUT_USEC);
	if (ret)
		pr_warn("%s: halt timeout\n", priv->phydev->dev->name);

	writel(AVE_GRR_GRST, priv->iobase + AVE_GRR);

	phy_shutdown(priv->phydev);
}

static void ave_reset(struct ave_private *priv)
{
	u32 val;

	/* reset RMII register */
	val = readl(priv->iobase + AVE_RSTCTRL);
	val &= ~AVE_RSTCTRL_RMIIRST;
	writel(val, priv->iobase + AVE_RSTCTRL);

	/* assert reset */
	writel(AVE_GRR_GRST | AVE_GRR_PHYRST, priv->iobase + AVE_GRR);
	mdelay(AVE_GRST_DELAY_MSEC);

	/* 1st, negate PHY reset only */
	writel(AVE_GRR_GRST, priv->iobase + AVE_GRR);
	mdelay(AVE_GRST_DELAY_MSEC);

	/* negate reset */
	writel(0, priv->iobase + AVE_GRR);
	mdelay(AVE_GRST_DELAY_MSEC);

	/* negate RMII register */
	val = readl(priv->iobase + AVE_RSTCTRL);
	val |= AVE_RSTCTRL_RMIIRST;
	writel(val, priv->iobase + AVE_RSTCTRL);
}

static int ave_start(struct udevice *dev)
{
	struct ave_private *priv = dev_get_priv(dev);
	uintptr_t paddr;
	u32 val;
	int i;

	ave_reset(priv);

	priv->rx_pos = 0;
	priv->rx_off = 2; /* RX data has 2byte offsets */
	priv->tx_num = 0;
	priv->tx_adj_buf =
		(void *)roundup((uintptr_t)&priv->tx_adj_packetbuf[0],
				PKTALIGN);
	priv->rx_siz = (PKTSIZE_ALIGN - priv->rx_off);

	val = 0;
	if (priv->phy_mode != PHY_INTERFACE_MODE_RGMII)
		val |= AVE_CFGR_MII;
	writel(val, priv->iobase + AVE_CFGR);

	/* use one descriptor for Tx */
	writel(AVE_DESC_SIZE(priv, 1) << 16, priv->iobase + AVE_TXDC);
	ave_desc_write_cmdsts(priv, AVE_DESCID_TX, 0, 0);
	ave_desc_write_addr(priv, AVE_DESCID_TX, 0, 0);

	/* use PKTBUFSRX descriptors for Rx */
	writel(AVE_DESC_SIZE(priv, PKTBUFSRX) << 16, priv->iobase + AVE_RXDC);
	for (i = 0; i < PKTBUFSRX; i++) {
		paddr = (uintptr_t)net_rx_packets[i];
		ave_cache_flush(paddr, priv->rx_siz + priv->rx_off);
		ave_desc_write_addr(priv, AVE_DESCID_RX, i, paddr);
		ave_desc_write_cmdsts(priv, AVE_DESCID_RX, i, priv->rx_siz);
	}

	writel(AVE_GISR_CLR, priv->iobase + AVE_GISR);
	writel(AVE_GIMR_CLR, priv->iobase + AVE_GIMR);

	writel(AVE_RXCR_RXEN | AVE_RXCR_FDUPEN | AVE_RXCR_FLOCTR | AVE_RXCR_MTU,
	       priv->iobase + AVE_RXCR);
	writel(AVE_DESCC_RD0 | AVE_DESCC_TD, priv->iobase + AVE_DESCC);

	phy_startup(priv->phydev);
	ave_adjust_link(priv);

	return 0;
}

static int ave_write_hwaddr(struct udevice *dev)
{
	struct ave_private *priv = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	u8 *mac = pdata->enetaddr;

	writel(mac[0] | mac[1] << 8 | mac[2] << 16 | mac[3] << 24,
	       priv->iobase + AVE_RXMAC1R);
	writel(mac[4] | mac[5] << 8, priv->iobase + AVE_RXMAC2R);

	return 0;
}

static int ave_send(struct udevice *dev, void *packet, int length)
{
	struct ave_private *priv = dev_get_priv(dev);
	u32 val;
	void *ptr = packet;
	int count;

	/* adjust alignment for descriptor */
	if ((uintptr_t)ptr & 0x3) {
		memcpy(priv->tx_adj_buf, (const void *)ptr, length);
		ptr = priv->tx_adj_buf;
	}

	/* padding for minimum length */
	if (length < AVE_MIN_XMITSIZE) {
		memset(ptr + length, 0, AVE_MIN_XMITSIZE - length);
		length = AVE_MIN_XMITSIZE;
	}

	/* check ownership and wait for previous xmit done */
	count = AVE_SEND_TIMEOUT_COUNT;
	do {
		val = ave_desc_read_cmdsts(priv, AVE_DESCID_TX, 0);
	} while ((val & AVE_STS_OWN) && --count);
	if (!count)
		return -ETIMEDOUT;

	ave_cache_flush((uintptr_t)ptr, length);
	ave_desc_write_addr(priv, AVE_DESCID_TX, 0, (uintptr_t)ptr);

	val = AVE_STS_OWN | AVE_STS_1ST | AVE_STS_LAST |
		(length & AVE_STS_PKTLEN_TX_MASK);
	ave_desc_write_cmdsts(priv, AVE_DESCID_TX, 0, val);
	priv->tx_num++;

	count = AVE_SEND_TIMEOUT_COUNT;
	do {
		val = ave_desc_read_cmdsts(priv, AVE_DESCID_TX, 0);
	} while ((val & AVE_STS_OWN) && --count);
	if (!count)
		return -ETIMEDOUT;

	if (!(val & AVE_STS_OK))
		pr_warn("%s: bad send packet status:%08x\n",
			priv->phydev->dev->name, le32_to_cpu(val));

	return 0;
}

static int ave_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct ave_private *priv = dev_get_priv(dev);
	uchar *ptr;
	int length = 0;
	u32 cmdsts;

	while (1) {
		cmdsts = ave_desc_read_cmdsts(priv, AVE_DESCID_RX,
					      priv->rx_pos);
		if (!(cmdsts & AVE_STS_OWN))
			/* hardware ownership, no received packets */
			return -EAGAIN;

		ptr = net_rx_packets[priv->rx_pos] + priv->rx_off;
		if (cmdsts & AVE_STS_OK)
			break;

		pr_warn("%s: bad packet[%d] status:%08x ptr:%p\n",
			priv->phydev->dev->name, priv->rx_pos,
			le32_to_cpu(cmdsts), ptr);
	}

	length = cmdsts & AVE_STS_PKTLEN_RX_MASK;

	/* invalidate after DMA is done */
	ave_cache_invalidate((uintptr_t)ptr, length);
	*packetp = ptr;

	return length;
}

static int ave_free_packet(struct udevice *dev, uchar *packet, int length)
{
	struct ave_private *priv = dev_get_priv(dev);

	ave_cache_flush((uintptr_t)net_rx_packets[priv->rx_pos],
			priv->rx_siz + priv->rx_off);

	ave_desc_write_cmdsts(priv, AVE_DESCID_RX,
			      priv->rx_pos, priv->rx_siz);

	if (++priv->rx_pos >= PKTBUFSRX)
		priv->rx_pos = 0;

	return 0;
}

static int ave_pro4_get_pinmode(struct ave_private *priv)
{
	u32 reg, mask, val = 0;

	if (priv->regmap_arg > 0)
		return -EINVAL;

	mask = SG_ETPINMODE_RMII(0);

	switch (priv->phy_mode) {
	case PHY_INTERFACE_MODE_RMII:
		val = SG_ETPINMODE_RMII(0);
		break;
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_RGMII:
		break;
	default:
		return -EINVAL;
	}

	regmap_read(priv->regmap, SG_ETPINMODE, &reg);
	reg &= ~mask;
	reg |= val;
	regmap_write(priv->regmap, SG_ETPINMODE, reg);

	return 0;
}

static int ave_ld11_get_pinmode(struct ave_private *priv)
{
	u32 reg, mask, val = 0;

	if (priv->regmap_arg > 0)
		return -EINVAL;

	mask = SG_ETPINMODE_EXTPHY | SG_ETPINMODE_RMII(0);

	switch (priv->phy_mode) {
	case PHY_INTERFACE_MODE_INTERNAL:
		break;
	case PHY_INTERFACE_MODE_RMII:
		val = SG_ETPINMODE_EXTPHY | SG_ETPINMODE_RMII(0);
		break;
	default:
		return -EINVAL;
	}

	regmap_read(priv->regmap, SG_ETPINMODE, &reg);
	reg &= ~mask;
	reg |= val;
	regmap_write(priv->regmap, SG_ETPINMODE, reg);

	return 0;
}

static int ave_ld20_get_pinmode(struct ave_private *priv)
{
	u32 reg, mask, val = 0;

	if (priv->regmap_arg > 0)
		return -EINVAL;

	mask = SG_ETPINMODE_RMII(0);

	switch (priv->phy_mode) {
	case PHY_INTERFACE_MODE_RMII:
		val  = SG_ETPINMODE_RMII(0);
		break;
	case PHY_INTERFACE_MODE_RGMII:
		break;
	default:
		return -EINVAL;
	}

	regmap_read(priv->regmap, SG_ETPINMODE, &reg);
	reg &= ~mask;
	reg |= val;
	regmap_write(priv->regmap, SG_ETPINMODE, reg);

	return 0;
}

static int ave_pxs3_get_pinmode(struct ave_private *priv)
{
	u32 reg, mask, val = 0;

	if (priv->regmap_arg > 1)
		return -EINVAL;

	mask = SG_ETPINMODE_RMII(priv->regmap_arg);

	switch (priv->phy_mode) {
	case PHY_INTERFACE_MODE_RMII:
		val = SG_ETPINMODE_RMII(priv->regmap_arg);
		break;
	case PHY_INTERFACE_MODE_RGMII:
		break;
	default:
		return -EINVAL;
	}

	regmap_read(priv->regmap, SG_ETPINMODE, &reg);
	reg &= ~mask;
	reg |= val;
	regmap_write(priv->regmap, SG_ETPINMODE, reg);

	return 0;
}

static int ave_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct ave_private *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	const char *phy_mode;
	const u32 *valp;
	int ret, nc, nr;
	const char *name;

	priv->data = (const struct ave_soc_data *)dev_get_driver_data(dev);
	if (!priv->data)
		return -EINVAL;

	pdata->iobase = devfdt_get_addr(dev);
	pdata->phy_interface = -1;
	phy_mode = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "phy-mode",
			       NULL);
	if (phy_mode)
		pdata->phy_interface = phy_get_interface_by_name(phy_mode);
	if (pdata->phy_interface == -1) {
		dev_err(dev, "Invalid PHY interface '%s'\n", phy_mode);
		return -EINVAL;
	}

	pdata->max_speed = 0;
	valp = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "max-speed",
			   NULL);
	if (valp)
		pdata->max_speed = fdt32_to_cpu(*valp);

	for (nc = 0; nc < AVE_MAX_CLKS; nc++) {
		name = priv->data->clock_names[nc];
		if (!name)
			break;
		ret = clk_get_by_name(dev, name, &priv->clk[nc]);
		if (ret) {
			dev_err(dev, "Failed to get clocks property: %d\n",
				ret);
			goto out_clk_free;
		}
		priv->nclks++;
	}

	for (nr = 0; nr < AVE_MAX_RSTS; nr++) {
		name = priv->data->reset_names[nr];
		if (!name)
			break;
		ret = reset_get_by_name(dev, name, &priv->rst[nr]);
		if (ret) {
			dev_err(dev, "Failed to get resets property: %d\n",
				ret);
			goto out_reset_free;
		}
		priv->nrsts++;
	}

	ret = dev_read_phandle_with_args(dev, "socionext,syscon-phy-mode",
					 NULL, 1, 0, &args);
	if (ret) {
		dev_err(dev, "Failed to get syscon-phy-mode property: %d\n",
			ret);
		goto out_reset_free;
	}

	priv->regmap = syscon_node_to_regmap(args.node);
	if (IS_ERR(priv->regmap)) {
		ret = PTR_ERR(priv->regmap);
		dev_err(dev, "can't get syscon: %d\n", ret);
		goto out_reset_free;
	}

	if (args.args_count != 1) {
		ret = -EINVAL;
		dev_err(dev, "Invalid argument of syscon-phy-mode\n");
		goto out_reset_free;
	}

	priv->regmap_arg = args.args[0];

	return 0;

out_reset_free:
	while (--nr >= 0)
		reset_free(&priv->rst[nr]);
out_clk_free:
	while (--nc >= 0)
		clk_free(&priv->clk[nc]);

	return ret;
}

static int ave_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct ave_private *priv = dev_get_priv(dev);
	int ret, nc, nr;

	priv->data = (const struct ave_soc_data *)dev_get_driver_data(dev);
	if (!priv->data)
		return -EINVAL;

	priv->iobase = pdata->iobase;
	priv->phy_mode = pdata->phy_interface;
	priv->max_speed = pdata->max_speed;

	ret = priv->data->get_pinmode(priv);
	if (ret) {
		dev_err(dev, "Invalid phy-mode\n");
		return -EINVAL;
	}

	for (nc = 0; nc < priv->nclks; nc++) {
		ret = clk_enable(&priv->clk[nc]);
		if (ret) {
			dev_err(dev, "Failed to enable clk: %d\n", ret);
			goto out_clk_release;
		}
	}

	for (nr = 0; nr < priv->nrsts; nr++) {
		ret = reset_deassert(&priv->rst[nr]);
		if (ret) {
			dev_err(dev, "Failed to deassert reset: %d\n", ret);
			goto out_reset_release;
		}
	}

	ave_reset(priv);

	ret = ave_mdiobus_init(priv, dev->name);
	if (ret) {
		dev_err(dev, "Failed to initialize mdiobus: %d\n", ret);
		goto out_reset_release;
	}

	priv->bus = miiphy_get_dev_by_name(dev->name);

	ret = ave_phy_init(priv, dev);
	if (ret) {
		dev_err(dev, "Failed to initialize phy: %d\n", ret);
		goto out_mdiobus_release;
	}

	return 0;

out_mdiobus_release:
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);
out_reset_release:
	reset_release_all(priv->rst, nr);
out_clk_release:
	clk_release_all(priv->clk, nc);

	return ret;
}

static int ave_remove(struct udevice *dev)
{
	struct ave_private *priv = dev_get_priv(dev);

	free(priv->phydev);
	mdio_unregister(priv->bus);
	mdio_free(priv->bus);
	reset_release_all(priv->rst, priv->nrsts);
	clk_release_all(priv->clk, priv->nclks);

	return 0;
}

static const struct eth_ops ave_ops = {
	.start        = ave_start,
	.stop         = ave_stop,
	.send         = ave_send,
	.recv         = ave_recv,
	.free_pkt     = ave_free_packet,
	.write_hwaddr = ave_write_hwaddr,
};

static const struct ave_soc_data ave_pro4_data = {
	.is_desc_64bit = false,
	.clock_names = {
		"gio", "ether", "ether-gb", "ether-phy",
	},
	.reset_names = {
		"gio", "ether",
	},
	.get_pinmode = ave_pro4_get_pinmode,
};

static const struct ave_soc_data ave_pxs2_data = {
	.is_desc_64bit = false,
	.clock_names = {
		"ether",
	},
	.reset_names = {
		"ether",
	},
	.get_pinmode = ave_pro4_get_pinmode,
};

static const struct ave_soc_data ave_ld11_data = {
	.is_desc_64bit = false,
	.clock_names = {
		"ether",
	},
	.reset_names = {
		"ether",
	},
	.get_pinmode = ave_ld11_get_pinmode,
};

static const struct ave_soc_data ave_ld20_data = {
	.is_desc_64bit = true,
	.clock_names = {
		"ether",
	},
	.reset_names = {
		"ether",
	},
	.get_pinmode = ave_ld20_get_pinmode,
};

static const struct ave_soc_data ave_pxs3_data = {
	.is_desc_64bit = false,
	.clock_names = {
		"ether",
	},
	.reset_names = {
		"ether",
	},
	.get_pinmode = ave_pxs3_get_pinmode,
};

static const struct udevice_id ave_ids[] = {
	{
		.compatible = "socionext,uniphier-pro4-ave4",
		.data = (ulong)&ave_pro4_data,
	},
	{
		.compatible = "socionext,uniphier-pxs2-ave4",
		.data = (ulong)&ave_pxs2_data,
	},
	{
		.compatible = "socionext,uniphier-ld11-ave4",
		.data = (ulong)&ave_ld11_data,
	},
	{
		.compatible = "socionext,uniphier-ld20-ave4",
		.data = (ulong)&ave_ld20_data,
	},
	{
		.compatible = "socionext,uniphier-pxs3-ave4",
		.data = (ulong)&ave_pxs3_data,
	},
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(ave) = {
	.name     = "ave",
	.id       = UCLASS_ETH,
	.of_match = ave_ids,
	.probe	  = ave_probe,
	.remove	  = ave_remove,
	.ofdata_to_platdata = ave_ofdata_to_platdata,
	.ops	  = &ave_ops,
	.priv_auto_alloc_size = sizeof(struct ave_private),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
