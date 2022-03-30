// SPDX-License-Identifier: GPL-2.0+
/*
 * sunxi_emac.c -- Allwinner A10 ethernet driver
 *
 * (C) Copyright 2012, Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <linux/err.h>
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>

/* EMAC register  */
struct emac_regs {
	u32 ctl;	/* 0x00 */
	u32 tx_mode;	/* 0x04 */
	u32 tx_flow;	/* 0x08 */
	u32 tx_ctl0;	/* 0x0c */
	u32 tx_ctl1;	/* 0x10 */
	u32 tx_ins;	/* 0x14 */
	u32 tx_pl0;	/* 0x18 */
	u32 tx_pl1;	/* 0x1c */
	u32 tx_sta;	/* 0x20 */
	u32 tx_io_data;	/* 0x24 */
	u32 tx_io_data1;/* 0x28 */
	u32 tx_tsvl0;	/* 0x2c */
	u32 tx_tsvh0;	/* 0x30 */
	u32 tx_tsvl1;	/* 0x34 */
	u32 tx_tsvh1;	/* 0x38 */
	u32 rx_ctl;	/* 0x3c */
	u32 rx_hash0;	/* 0x40 */
	u32 rx_hash1;	/* 0x44 */
	u32 rx_sta;	/* 0x48 */
	u32 rx_io_data;	/* 0x4c */
	u32 rx_fbc;	/* 0x50 */
	u32 int_ctl;	/* 0x54 */
	u32 int_sta;	/* 0x58 */
	u32 mac_ctl0;	/* 0x5c */
	u32 mac_ctl1;	/* 0x60 */
	u32 mac_ipgt;	/* 0x64 */
	u32 mac_ipgr;	/* 0x68 */
	u32 mac_clrt;	/* 0x6c */
	u32 mac_maxf;	/* 0x70 */
	u32 mac_supp;	/* 0x74 */
	u32 mac_test;	/* 0x78 */
	u32 mac_mcfg;	/* 0x7c */
	u32 mac_mcmd;	/* 0x80 */
	u32 mac_madr;	/* 0x84 */
	u32 mac_mwtd;	/* 0x88 */
	u32 mac_mrdd;	/* 0x8c */
	u32 mac_mind;	/* 0x90 */
	u32 mac_ssrr;	/* 0x94 */
	u32 mac_a0;	/* 0x98 */
	u32 mac_a1;	/* 0x9c */
};

/* SRAMC register  */
struct sunxi_sramc_regs {
	u32 ctrl0;
	u32 ctrl1;
};

/* 0: Disable       1: Aborted frame enable(default) */
#define EMAC_TX_AB_M		(0x1 << 0)
/* 0: CPU           1: DMA(default) */
#define EMAC_TX_TM		(0x1 << 1)

#define EMAC_TX_SETUP		(0)

/* 0: DRQ asserted  1: DRQ automatically(default) */
#define EMAC_RX_DRQ_MODE	(0x1 << 1)
/* 0: CPU           1: DMA(default) */
#define EMAC_RX_TM		(0x1 << 2)
/* 0: Normal(default)        1: Pass all Frames */
#define EMAC_RX_PA		(0x1 << 4)
/* 0: Normal(default)        1: Pass Control Frames */
#define EMAC_RX_PCF		(0x1 << 5)
/* 0: Normal(default)        1: Pass Frames with CRC Error */
#define EMAC_RX_PCRCE		(0x1 << 6)
/* 0: Normal(default)        1: Pass Frames with Length Error */
#define EMAC_RX_PLE		(0x1 << 7)
/* 0: Normal                 1: Pass Frames length out of range(default) */
#define EMAC_RX_POR		(0x1 << 8)
/* 0: Not accept             1: Accept unicast Packets(default) */
#define EMAC_RX_UCAD		(0x1 << 16)
/* 0: Normal(default)        1: DA Filtering */
#define EMAC_RX_DAF		(0x1 << 17)
/* 0: Not accept             1: Accept multicast Packets(default) */
#define EMAC_RX_MCO		(0x1 << 20)
/* 0: Disable(default)       1: Enable Hash filter */
#define EMAC_RX_MHF		(0x1 << 21)
/* 0: Not accept             1: Accept Broadcast Packets(default) */
#define EMAC_RX_BCO		(0x1 << 22)
/* 0: Disable(default)       1: Enable SA Filtering */
#define EMAC_RX_SAF		(0x1 << 24)
/* 0: Normal(default)        1: Inverse Filtering */
#define EMAC_RX_SAIF		(0x1 << 25)

#define EMAC_RX_SETUP		(EMAC_RX_POR | EMAC_RX_UCAD | EMAC_RX_DAF | \
				 EMAC_RX_MCO | EMAC_RX_BCO)

/* 0: Disable                1: Enable Receive Flow Control(default) */
#define EMAC_MAC_CTL0_RFC	(0x1 << 2)
/* 0: Disable                1: Enable Transmit Flow Control(default) */
#define EMAC_MAC_CTL0_TFC	(0x1 << 3)

#define EMAC_MAC_CTL0_SETUP	(EMAC_MAC_CTL0_RFC | EMAC_MAC_CTL0_TFC)

/* 0: Disable                1: Enable MAC Frame Length Checking(default) */
#define EMAC_MAC_CTL1_FLC	(0x1 << 1)
/* 0: Disable(default)       1: Enable Huge Frame */
#define EMAC_MAC_CTL1_HF	(0x1 << 2)
/* 0: Disable(default)       1: Enable MAC Delayed CRC */
#define EMAC_MAC_CTL1_DCRC	(0x1 << 3)
/* 0: Disable                1: Enable MAC CRC(default) */
#define EMAC_MAC_CTL1_CRC	(0x1 << 4)
/* 0: Disable                1: Enable MAC PAD Short frames(default) */
#define EMAC_MAC_CTL1_PC	(0x1 << 5)
/* 0: Disable(default)       1: Enable MAC PAD Short frames and append CRC */
#define EMAC_MAC_CTL1_VC	(0x1 << 6)
/* 0: Disable(default)       1: Enable MAC auto detect Short frames */
#define EMAC_MAC_CTL1_ADP	(0x1 << 7)
/* 0: Disable(default)       1: Enable */
#define EMAC_MAC_CTL1_PRE	(0x1 << 8)
/* 0: Disable(default)       1: Enable */
#define EMAC_MAC_CTL1_LPE	(0x1 << 9)
/* 0: Disable(default)       1: Enable no back off */
#define EMAC_MAC_CTL1_NB	(0x1 << 12)
/* 0: Disable(default)       1: Enable */
#define EMAC_MAC_CTL1_BNB	(0x1 << 13)
/* 0: Disable(default)       1: Enable */
#define EMAC_MAC_CTL1_ED	(0x1 << 14)

#define EMAC_MAC_CTL1_SETUP	(EMAC_MAC_CTL1_FLC | EMAC_MAC_CTL1_CRC | \
				 EMAC_MAC_CTL1_PC)

#define EMAC_MAC_IPGT		0x15

#define EMAC_MAC_NBTB_IPG1	0xc
#define EMAC_MAC_NBTB_IPG2	0x12

#define EMAC_MAC_CW		0x37
#define EMAC_MAC_RM		0xf

#define EMAC_MAC_MFL		0x0600

/* Receive status */
#define EMAC_CRCERR		(0x1 << 4)
#define EMAC_LENERR		(0x3 << 5)

#define EMAC_RX_BUFSIZE		2000

struct emac_eth_dev {
	struct emac_regs *regs;
	struct clk clk;
	struct mii_dev *bus;
	struct phy_device *phydev;
	int link_printed;
#ifdef CONFIG_DM_ETH
	uchar rx_buf[EMAC_RX_BUFSIZE];
#endif
};

struct emac_rxhdr {
	s16 rx_len;
	u16 rx_status;
};

static void emac_inblk_32bit(void *reg, void *data, int count)
{
	int cnt = (count + 3) >> 2;

	if (cnt) {
		u32 *buf = data;

		do {
			u32 x = readl(reg);
			*buf++ = x;
		} while (--cnt);
	}
}

static void emac_outblk_32bit(void *reg, void *data, int count)
{
	int cnt = (count + 3) >> 2;

	if (cnt) {
		const u32 *buf = data;

		do {
			writel(*buf++, reg);
		} while (--cnt);
	}
}

/* Read a word from phyxcer */
static int emac_mdio_read(struct mii_dev *bus, int addr, int devad, int reg)
{
	struct emac_eth_dev *priv = bus->priv;
	struct emac_regs *regs = priv->regs;

	/* issue the phy address and reg */
	writel(addr << 8 | reg, &regs->mac_madr);

	/* pull up the phy io line */
	writel(0x1, &regs->mac_mcmd);

	/* Wait read complete */
	mdelay(1);

	/* push down the phy io line */
	writel(0x0, &regs->mac_mcmd);

	/* And read data */
	return readl(&regs->mac_mrdd);
}

/* Write a word to phyxcer */
static int emac_mdio_write(struct mii_dev *bus, int addr, int devad, int reg,
			  u16 value)
{
	struct emac_eth_dev *priv = bus->priv;
	struct emac_regs *regs = priv->regs;

	/* issue the phy address and reg */
	writel(addr << 8 | reg, &regs->mac_madr);

	/* pull up the phy io line */
	writel(0x1, &regs->mac_mcmd);

	/* Wait write complete */
	mdelay(1);

	/* push down the phy io line */
	writel(0x0, &regs->mac_mcmd);

	/* and write data */
	writel(value, &regs->mac_mwtd);

	return 0;
}

static int sunxi_emac_init_phy(struct emac_eth_dev *priv, void *dev)
{
	int ret, mask = 0xffffffff;

#ifdef CONFIG_PHY_ADDR
	mask = 1 << CONFIG_PHY_ADDR;
#endif

	priv->bus = mdio_alloc();
	if (!priv->bus) {
		printf("Failed to allocate MDIO bus\n");
		return -ENOMEM;
	}

	priv->bus->read = emac_mdio_read;
	priv->bus->write = emac_mdio_write;
	priv->bus->priv = priv;
	strcpy(priv->bus->name, "emac");

	ret = mdio_register(priv->bus);
	if (ret)
		return ret;

	priv->phydev = phy_find_by_mask(priv->bus, mask,
					PHY_INTERFACE_MODE_MII);
	if (!priv->phydev)
		return -ENODEV;

	phy_connect_dev(priv->phydev, dev);
	phy_config(priv->phydev);

	return 0;
}

static void emac_setup(struct emac_eth_dev *priv)
{
	struct emac_regs *regs = priv->regs;
	u32 reg_val;

	/* Set up TX */
	writel(EMAC_TX_SETUP, &regs->tx_mode);

	/* Set up RX */
	writel(EMAC_RX_SETUP, &regs->rx_ctl);

	/* Set MAC */
	/* Set MAC CTL0 */
	writel(EMAC_MAC_CTL0_SETUP, &regs->mac_ctl0);

	/* Set MAC CTL1 */
	reg_val = 0;
	if (priv->phydev->duplex == DUPLEX_FULL)
		reg_val = (0x1 << 0);
	writel(EMAC_MAC_CTL1_SETUP | reg_val, &regs->mac_ctl1);

	/* Set up IPGT */
	writel(EMAC_MAC_IPGT, &regs->mac_ipgt);

	/* Set up IPGR */
	writel(EMAC_MAC_NBTB_IPG2 | (EMAC_MAC_NBTB_IPG1 << 8), &regs->mac_ipgr);

	/* Set up Collison window */
	writel(EMAC_MAC_RM | (EMAC_MAC_CW << 8), &regs->mac_clrt);

	/* Set up Max Frame Length */
	writel(EMAC_MAC_MFL, &regs->mac_maxf);
}

static void emac_reset(struct emac_eth_dev *priv)
{
	struct emac_regs *regs = priv->regs;

	debug("resetting device\n");

	/* RESET device */
	writel(0, &regs->ctl);
	udelay(200);

	writel(1, &regs->ctl);
	udelay(200);
}

static int _sunxi_write_hwaddr(struct emac_eth_dev *priv, u8 *enetaddr)
{
	struct emac_regs *regs = priv->regs;
	u32 enetaddr_lo, enetaddr_hi;

	enetaddr_lo = enetaddr[2] | (enetaddr[1] << 8) | (enetaddr[0] << 16);
	enetaddr_hi = enetaddr[5] | (enetaddr[4] << 8) | (enetaddr[3] << 16);

	writel(enetaddr_hi, &regs->mac_a0);
	writel(enetaddr_lo, &regs->mac_a1);

	return 0;
}

static int _sunxi_emac_eth_init(struct emac_eth_dev *priv, u8 *enetaddr)
{
	struct emac_regs *regs = priv->regs;
	int ret;

	/* Init EMAC */

	/* Flush RX FIFO */
	setbits_le32(&regs->rx_ctl, 0x8);
	udelay(1);

	/* Init MAC */

	/* Soft reset MAC */
	clrbits_le32(&regs->mac_ctl0, 0x1 << 15);

	/* Clear RX counter */
	writel(0x0, &regs->rx_fbc);
	udelay(1);

	/* Set up EMAC */
	emac_setup(priv);

	_sunxi_write_hwaddr(priv, enetaddr);

	mdelay(1);

	emac_reset(priv);

	/* PHY POWER UP */
	ret = phy_startup(priv->phydev);
	if (ret) {
		printf("Could not initialize PHY %s\n",
		       priv->phydev->dev->name);
		return ret;
	}

	/* Print link status only once */
	if (!priv->link_printed) {
		printf("ENET Speed is %d Mbps - %s duplex connection\n",
		       priv->phydev->speed,
		       priv->phydev->duplex ? "FULL" : "HALF");
		priv->link_printed = 1;
	}

	/* Set EMAC SPEED depend on PHY */
	if (priv->phydev->speed == SPEED_100)
		setbits_le32(&regs->mac_supp, 1 << 8);
	else
		clrbits_le32(&regs->mac_supp, 1 << 8);

	/* Set duplex depend on phy */
	if (priv->phydev->duplex == DUPLEX_FULL)
		setbits_le32(&regs->mac_ctl1, 1 << 0);
	else
		clrbits_le32(&regs->mac_ctl1, 1 << 0);

	/* Enable RX/TX */
	setbits_le32(&regs->ctl, 0x7);

	return 0;
}

static int _sunxi_emac_eth_recv(struct emac_eth_dev *priv, void *packet)
{
	struct emac_regs *regs = priv->regs;
	struct emac_rxhdr rxhdr;
	u32 rxcount;
	u32 reg_val;
	int rx_len;
	int rx_status;
	int good_packet;

	/* Check packet ready or not */

	/* Race warning: The first packet might arrive with
	 * the interrupts disabled, but the second will fix
	 */
	rxcount = readl(&regs->rx_fbc);
	if (!rxcount) {
		/* Had one stuck? */
		rxcount = readl(&regs->rx_fbc);
		if (!rxcount)
			return -EAGAIN;
	}

	reg_val = readl(&regs->rx_io_data);
	if (reg_val != 0x0143414d) {
		/* Disable RX */
		clrbits_le32(&regs->ctl, 0x1 << 2);

		/* Flush RX FIFO */
		setbits_le32(&regs->rx_ctl, 0x1 << 3);
		while (readl(&regs->rx_ctl) & (0x1 << 3))
			;

		/* Enable RX */
		setbits_le32(&regs->ctl, 0x1 << 2);

		return -EAGAIN;
	}

	/* A packet ready now
	 * Get status/length
	 */
	good_packet = 1;

	emac_inblk_32bit(&regs->rx_io_data, &rxhdr, sizeof(rxhdr));

	rx_len = rxhdr.rx_len;
	rx_status = rxhdr.rx_status;

	/* Packet Status check */
	if (rx_len < 0x40) {
		good_packet = 0;
		debug("RX: Bad Packet (runt)\n");
	}

	/* rx_status is identical to RSR register. */
	if (0 & rx_status & (EMAC_CRCERR | EMAC_LENERR)) {
		good_packet = 0;
		if (rx_status & EMAC_CRCERR)
			printf("crc error\n");
		if (rx_status & EMAC_LENERR)
			printf("length error\n");
	}

	/* Move data from EMAC */
	if (good_packet) {
		if (rx_len > EMAC_RX_BUFSIZE) {
			printf("Received packet is too big (len=%d)\n", rx_len);
			return -EMSGSIZE;
		}
		emac_inblk_32bit((void *)&regs->rx_io_data, packet, rx_len);
		return rx_len;
	}

	return -EIO; /* Bad packet */
}

static int _sunxi_emac_eth_send(struct emac_eth_dev *priv, void *packet,
				int len)
{
	struct emac_regs *regs = priv->regs;

	/* Select channel 0 */
	writel(0, &regs->tx_ins);

	/* Write packet */
	emac_outblk_32bit((void *)&regs->tx_io_data, packet, len);

	/* Set TX len */
	writel(len, &regs->tx_pl0);

	/* Start translate from fifo to phy */
	setbits_le32(&regs->tx_ctl0, 1);

	return 0;
}

static int sunxi_emac_board_setup(struct emac_eth_dev *priv)
{
	struct sunxi_sramc_regs *sram =
		(struct sunxi_sramc_regs *)SUNXI_SRAMC_BASE;
	struct emac_regs *regs = priv->regs;
	int pin, ret;

	/* Map SRAM to EMAC */
	setbits_le32(&sram->ctrl1, 0x5 << 2);

	/* Configure pin mux settings for MII Ethernet */
	for (pin = SUNXI_GPA(0); pin <= SUNXI_GPA(17); pin++)
		sunxi_gpio_set_cfgpin(pin, SUNXI_GPA_EMAC);

	/* Set up clock gating */
	ret = clk_enable(&priv->clk);
	if (ret) {
		dev_err(dev, "failed to enable emac clock\n");
		return ret;
	}

	/* Set MII clock */
	clrsetbits_le32(&regs->mac_mcfg, 0xf << 2, 0xd << 2);

	return 0;
}

static int sunxi_emac_eth_start(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);

	return _sunxi_emac_eth_init(dev->priv, pdata->enetaddr);
}

static int sunxi_emac_eth_send(struct udevice *dev, void *packet, int length)
{
	struct emac_eth_dev *priv = dev_get_priv(dev);

	return _sunxi_emac_eth_send(priv, packet, length);
}

static int sunxi_emac_eth_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct emac_eth_dev *priv = dev_get_priv(dev);
	int rx_len;

	rx_len = _sunxi_emac_eth_recv(priv, priv->rx_buf);
	*packetp = priv->rx_buf;

	return rx_len;
}

static void sunxi_emac_eth_stop(struct udevice *dev)
{
	/* Nothing to do here */
}

static int sunxi_emac_eth_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct emac_eth_dev *priv = dev_get_priv(dev);
	int ret;

	priv->regs = (struct emac_regs *)pdata->iobase;

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret) {
		dev_err(dev, "failed to get emac clock\n");
		return ret;
	}

	ret = sunxi_emac_board_setup(priv);
	if (ret)
		return ret;

	return sunxi_emac_init_phy(priv, dev);
}

static const struct eth_ops sunxi_emac_eth_ops = {
	.start			= sunxi_emac_eth_start,
	.send			= sunxi_emac_eth_send,
	.recv			= sunxi_emac_eth_recv,
	.stop			= sunxi_emac_eth_stop,
};

static int sunxi_emac_eth_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);

	pdata->iobase = devfdt_get_addr(dev);

	return 0;
}

static const struct udevice_id sunxi_emac_eth_ids[] = {
	{ .compatible = "allwinner,sun4i-a10-emac" },
	{ }
};

U_BOOT_DRIVER(eth_sunxi_emac) = {
	.name	= "eth_sunxi_emac",
	.id	= UCLASS_ETH,
	.of_match = sunxi_emac_eth_ids,
	.ofdata_to_platdata = sunxi_emac_eth_ofdata_to_platdata,
	.probe	= sunxi_emac_eth_probe,
	.ops	= &sunxi_emac_eth_ops,
	.priv_auto_alloc_size = sizeof(struct emac_eth_dev),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
