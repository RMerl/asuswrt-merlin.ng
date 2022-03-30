// SPDX-License-Identifier: GPL-2.0+
/*
 * drivers/net/ravb.c
 *     This file is driver for Renesas Ethernet AVB.
 *
 * Copyright (C) 2015-2017  Renesas Electronics Corporation
 *
 * Based on the SuperH Ethernet driver.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <miiphy.h>
#include <malloc.h>
#include <linux/mii.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <asm/gpio.h>

/* Registers */
#define RAVB_REG_CCC		0x000
#define RAVB_REG_DBAT		0x004
#define RAVB_REG_CSR		0x00C
#define RAVB_REG_APSR		0x08C
#define RAVB_REG_RCR		0x090
#define RAVB_REG_TGC		0x300
#define RAVB_REG_TCCR		0x304
#define RAVB_REG_RIC0		0x360
#define RAVB_REG_RIC1		0x368
#define RAVB_REG_RIC2		0x370
#define RAVB_REG_TIC		0x378
#define RAVB_REG_ECMR		0x500
#define RAVB_REG_RFLR		0x508
#define RAVB_REG_ECSIPR		0x518
#define RAVB_REG_PIR		0x520
#define RAVB_REG_GECMR		0x5b0
#define RAVB_REG_MAHR		0x5c0
#define RAVB_REG_MALR		0x5c8

#define CCC_OPC_CONFIG		BIT(0)
#define CCC_OPC_OPERATION	BIT(1)
#define CCC_BOC			BIT(20)

#define CSR_OPS			0x0000000F
#define CSR_OPS_CONFIG		BIT(1)

#define APSR_TDM		BIT(14)

#define TCCR_TSRQ0		BIT(0)

#define RFLR_RFL_MIN		0x05EE

#define PIR_MDI			BIT(3)
#define PIR_MDO			BIT(2)
#define PIR_MMD			BIT(1)
#define PIR_MDC			BIT(0)

#define ECMR_TRCCM		BIT(26)
#define ECMR_RZPF		BIT(20)
#define ECMR_PFR		BIT(18)
#define ECMR_RXF		BIT(17)
#define ECMR_RE			BIT(6)
#define ECMR_TE			BIT(5)
#define ECMR_DM			BIT(1)
#define ECMR_CHG_DM		(ECMR_TRCCM | ECMR_RZPF | ECMR_PFR | ECMR_RXF)

/* DMA Descriptors */
#define RAVB_NUM_BASE_DESC		16
#define RAVB_NUM_TX_DESC		8
#define RAVB_NUM_RX_DESC		8

#define RAVB_TX_QUEUE_OFFSET		0
#define RAVB_RX_QUEUE_OFFSET		4

#define RAVB_DESC_DT(n)			((n) << 28)
#define RAVB_DESC_DT_FSINGLE		RAVB_DESC_DT(0x7)
#define RAVB_DESC_DT_LINKFIX		RAVB_DESC_DT(0x9)
#define RAVB_DESC_DT_EOS		RAVB_DESC_DT(0xa)
#define RAVB_DESC_DT_FEMPTY		RAVB_DESC_DT(0xc)
#define RAVB_DESC_DT_EEMPTY		RAVB_DESC_DT(0x3)
#define RAVB_DESC_DT_MASK		RAVB_DESC_DT(0xf)

#define RAVB_DESC_DS(n)			(((n) & 0xfff) << 0)
#define RAVB_DESC_DS_MASK		0xfff

#define RAVB_RX_DESC_MSC_MC		BIT(23)
#define RAVB_RX_DESC_MSC_CEEF		BIT(22)
#define RAVB_RX_DESC_MSC_CRL		BIT(21)
#define RAVB_RX_DESC_MSC_FRE		BIT(20)
#define RAVB_RX_DESC_MSC_RTLF		BIT(19)
#define RAVB_RX_DESC_MSC_RTSF		BIT(18)
#define RAVB_RX_DESC_MSC_RFE		BIT(17)
#define RAVB_RX_DESC_MSC_CRC		BIT(16)
#define RAVB_RX_DESC_MSC_MASK		(0xff << 16)

#define RAVB_RX_DESC_MSC_RX_ERR_MASK \
	(RAVB_RX_DESC_MSC_CRC | RAVB_RX_DESC_MSC_RFE | RAVB_RX_DESC_MSC_RTLF | \
	 RAVB_RX_DESC_MSC_RTSF | RAVB_RX_DESC_MSC_CEEF)

#define RAVB_TX_TIMEOUT_MS		1000

struct ravb_desc {
	u32	ctrl;
	u32	dptr;
};

struct ravb_rxdesc {
	struct ravb_desc	data;
	struct ravb_desc	link;
	u8			__pad[48];
	u8			packet[PKTSIZE_ALIGN];
};

struct ravb_priv {
	struct ravb_desc	base_desc[RAVB_NUM_BASE_DESC];
	struct ravb_desc	tx_desc[RAVB_NUM_TX_DESC];
	struct ravb_rxdesc	rx_desc[RAVB_NUM_RX_DESC];
	u32			rx_desc_idx;
	u32			tx_desc_idx;

	struct phy_device	*phydev;
	struct mii_dev		*bus;
	void __iomem		*iobase;
	struct clk		clk;
	struct gpio_desc	reset_gpio;
};

static inline void ravb_flush_dcache(u32 addr, u32 len)
{
	flush_dcache_range(addr, addr + len);
}

static inline void ravb_invalidate_dcache(u32 addr, u32 len)
{
	u32 start = addr & ~((uintptr_t)ARCH_DMA_MINALIGN - 1);
	u32 end = roundup(addr + len, ARCH_DMA_MINALIGN);
	invalidate_dcache_range(start, end);
}

static int ravb_send(struct udevice *dev, void *packet, int len)
{
	struct ravb_priv *eth = dev_get_priv(dev);
	struct ravb_desc *desc = &eth->tx_desc[eth->tx_desc_idx];
	unsigned int start;

	/* Update TX descriptor */
	ravb_flush_dcache((uintptr_t)packet, len);
	memset(desc, 0x0, sizeof(*desc));
	desc->ctrl = RAVB_DESC_DT_FSINGLE | RAVB_DESC_DS(len);
	desc->dptr = (uintptr_t)packet;
	ravb_flush_dcache((uintptr_t)desc, sizeof(*desc));

	/* Restart the transmitter if disabled */
	if (!(readl(eth->iobase + RAVB_REG_TCCR) & TCCR_TSRQ0))
		setbits_le32(eth->iobase + RAVB_REG_TCCR, TCCR_TSRQ0);

	/* Wait until packet is transmitted */
	start = get_timer(0);
	while (get_timer(start) < RAVB_TX_TIMEOUT_MS) {
		ravb_invalidate_dcache((uintptr_t)desc, sizeof(*desc));
		if ((desc->ctrl & RAVB_DESC_DT_MASK) != RAVB_DESC_DT_FSINGLE)
			break;
		udelay(10);
	};

	if (get_timer(start) >= RAVB_TX_TIMEOUT_MS)
		return -ETIMEDOUT;

	eth->tx_desc_idx = (eth->tx_desc_idx + 1) % (RAVB_NUM_TX_DESC - 1);
	return 0;
}

static int ravb_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct ravb_priv *eth = dev_get_priv(dev);
	struct ravb_rxdesc *desc = &eth->rx_desc[eth->rx_desc_idx];
	int len;
	u8 *packet;

	/* Check if the rx descriptor is ready */
	ravb_invalidate_dcache((uintptr_t)desc, sizeof(*desc));
	if ((desc->data.ctrl & RAVB_DESC_DT_MASK) == RAVB_DESC_DT_FEMPTY)
		return -EAGAIN;

	/* Check for errors */
	if (desc->data.ctrl & RAVB_RX_DESC_MSC_RX_ERR_MASK) {
		desc->data.ctrl &= ~RAVB_RX_DESC_MSC_MASK;
		return -EAGAIN;
	}

	len = desc->data.ctrl & RAVB_DESC_DS_MASK;
	packet = (u8 *)(uintptr_t)desc->data.dptr;
	ravb_invalidate_dcache((uintptr_t)packet, len);

	*packetp = packet;
	return len;
}

static int ravb_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct ravb_priv *eth = dev_get_priv(dev);
	struct ravb_rxdesc *desc = &eth->rx_desc[eth->rx_desc_idx];

	/* Make current descriptor available again */
	desc->data.ctrl = RAVB_DESC_DT_FEMPTY | RAVB_DESC_DS(PKTSIZE_ALIGN);
	ravb_flush_dcache((uintptr_t)desc, sizeof(*desc));

	/* Point to the next descriptor */
	eth->rx_desc_idx = (eth->rx_desc_idx + 1) % RAVB_NUM_RX_DESC;
	desc = &eth->rx_desc[eth->rx_desc_idx];
	ravb_invalidate_dcache((uintptr_t)desc, sizeof(*desc));

	return 0;
}

static int ravb_reset(struct udevice *dev)
{
	struct ravb_priv *eth = dev_get_priv(dev);

	/* Set config mode */
	writel(CCC_OPC_CONFIG, eth->iobase + RAVB_REG_CCC);

	/* Check the operating mode is changed to the config mode. */
	return wait_for_bit_le32(eth->iobase + RAVB_REG_CSR,
				 CSR_OPS_CONFIG, true, 100, true);
}

static void ravb_base_desc_init(struct ravb_priv *eth)
{
	const u32 desc_size = RAVB_NUM_BASE_DESC * sizeof(struct ravb_desc);
	int i;

	/* Initialize all descriptors */
	memset(eth->base_desc, 0x0, desc_size);

	for (i = 0; i < RAVB_NUM_BASE_DESC; i++)
		eth->base_desc[i].ctrl = RAVB_DESC_DT_EOS;

	ravb_flush_dcache((uintptr_t)eth->base_desc, desc_size);

	/* Register the descriptor base address table */
	writel((uintptr_t)eth->base_desc, eth->iobase + RAVB_REG_DBAT);
}

static void ravb_tx_desc_init(struct ravb_priv *eth)
{
	const u32 desc_size = RAVB_NUM_TX_DESC * sizeof(struct ravb_desc);
	int i;

	/* Initialize all descriptors */
	memset(eth->tx_desc, 0x0, desc_size);
	eth->tx_desc_idx = 0;

	for (i = 0; i < RAVB_NUM_TX_DESC; i++)
		eth->tx_desc[i].ctrl = RAVB_DESC_DT_EEMPTY;

	/* Mark the end of the descriptors */
	eth->tx_desc[RAVB_NUM_TX_DESC - 1].ctrl = RAVB_DESC_DT_LINKFIX;
	eth->tx_desc[RAVB_NUM_TX_DESC - 1].dptr = (uintptr_t)eth->tx_desc;
	ravb_flush_dcache((uintptr_t)eth->tx_desc, desc_size);

	/* Point the controller to the TX descriptor list. */
	eth->base_desc[RAVB_TX_QUEUE_OFFSET].ctrl = RAVB_DESC_DT_LINKFIX;
	eth->base_desc[RAVB_TX_QUEUE_OFFSET].dptr = (uintptr_t)eth->tx_desc;
	ravb_flush_dcache((uintptr_t)&eth->base_desc[RAVB_TX_QUEUE_OFFSET],
			  sizeof(struct ravb_desc));
}

static void ravb_rx_desc_init(struct ravb_priv *eth)
{
	const u32 desc_size = RAVB_NUM_RX_DESC * sizeof(struct ravb_rxdesc);
	int i;

	/* Initialize all descriptors */
	memset(eth->rx_desc, 0x0, desc_size);
	eth->rx_desc_idx = 0;

	for (i = 0; i < RAVB_NUM_RX_DESC; i++) {
		eth->rx_desc[i].data.ctrl = RAVB_DESC_DT_EEMPTY |
					    RAVB_DESC_DS(PKTSIZE_ALIGN);
		eth->rx_desc[i].data.dptr = (uintptr_t)eth->rx_desc[i].packet;

		eth->rx_desc[i].link.ctrl = RAVB_DESC_DT_LINKFIX;
		eth->rx_desc[i].link.dptr = (uintptr_t)&eth->rx_desc[i + 1];
	}

	/* Mark the end of the descriptors */
	eth->rx_desc[RAVB_NUM_RX_DESC - 1].link.ctrl = RAVB_DESC_DT_LINKFIX;
	eth->rx_desc[RAVB_NUM_RX_DESC - 1].link.dptr = (uintptr_t)eth->rx_desc;
	ravb_flush_dcache((uintptr_t)eth->rx_desc, desc_size);

	/* Point the controller to the rx descriptor list */
	eth->base_desc[RAVB_RX_QUEUE_OFFSET].ctrl = RAVB_DESC_DT_LINKFIX;
	eth->base_desc[RAVB_RX_QUEUE_OFFSET].dptr = (uintptr_t)eth->rx_desc;
	ravb_flush_dcache((uintptr_t)&eth->base_desc[RAVB_RX_QUEUE_OFFSET],
			  sizeof(struct ravb_desc));
}

static int ravb_phy_config(struct udevice *dev)
{
	struct ravb_priv *eth = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct phy_device *phydev;
	int mask = 0xffffffff, reg;

	if (dm_gpio_is_valid(&eth->reset_gpio)) {
		dm_gpio_set_value(&eth->reset_gpio, 1);
		mdelay(20);
		dm_gpio_set_value(&eth->reset_gpio, 0);
		mdelay(1);
	}

	phydev = phy_find_by_mask(eth->bus, mask, pdata->phy_interface);
	if (!phydev)
		return -ENODEV;

	phy_connect_dev(phydev, dev);

	eth->phydev = phydev;

	phydev->supported &= SUPPORTED_100baseT_Full |
			     SUPPORTED_1000baseT_Full | SUPPORTED_Autoneg |
			     SUPPORTED_TP | SUPPORTED_MII | SUPPORTED_Pause |
			     SUPPORTED_Asym_Pause;

	if (pdata->max_speed != 1000) {
		phydev->supported &= ~SUPPORTED_1000baseT_Full;
		reg = phy_read(phydev, -1, MII_CTRL1000);
		reg &= ~(BIT(9) | BIT(8));
		phy_write(phydev, -1, MII_CTRL1000, reg);
	}

	phy_config(phydev);

	return 0;
}

/* Set Mac address */
static int ravb_write_hwaddr(struct udevice *dev)
{
	struct ravb_priv *eth = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	unsigned char *mac = pdata->enetaddr;

	writel((mac[0] << 24) | (mac[1] << 16) | (mac[2] << 8) | mac[3],
	       eth->iobase + RAVB_REG_MAHR);

	writel((mac[4] << 8) | mac[5], eth->iobase + RAVB_REG_MALR);

	return 0;
}

/* E-MAC init function */
static int ravb_mac_init(struct ravb_priv *eth)
{
	/* Disable MAC Interrupt */
	writel(0, eth->iobase + RAVB_REG_ECSIPR);

	/* Recv frame limit set register */
	writel(RFLR_RFL_MIN, eth->iobase + RAVB_REG_RFLR);

	return 0;
}

/* AVB-DMAC init function */
static int ravb_dmac_init(struct udevice *dev)
{
	struct ravb_priv *eth = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_platdata(dev);
	int ret = 0;

	/* Set CONFIG mode */
	ret = ravb_reset(dev);
	if (ret)
		return ret;

	/* Disable all interrupts */
	writel(0, eth->iobase + RAVB_REG_RIC0);
	writel(0, eth->iobase + RAVB_REG_RIC1);
	writel(0, eth->iobase + RAVB_REG_RIC2);
	writel(0, eth->iobase + RAVB_REG_TIC);

	/* Set little endian */
	clrbits_le32(eth->iobase + RAVB_REG_CCC, CCC_BOC);

	/* AVB rx set */
	writel(0x18000001, eth->iobase + RAVB_REG_RCR);

	/* FIFO size set */
	writel(0x00222210, eth->iobase + RAVB_REG_TGC);

	/* Delay CLK: 2ns (not applicable on R-Car E3/D3) */
	if ((rmobile_get_cpu_type() == RMOBILE_CPU_TYPE_R8A77990) ||
	    (rmobile_get_cpu_type() == RMOBILE_CPU_TYPE_R8A77995))
		return 0;

	if ((pdata->phy_interface == PHY_INTERFACE_MODE_RGMII_ID) ||
	    (pdata->phy_interface == PHY_INTERFACE_MODE_RGMII_TXID))
		writel(APSR_TDM, eth->iobase + RAVB_REG_APSR);

	return 0;
}

static int ravb_config(struct udevice *dev)
{
	struct ravb_priv *eth = dev_get_priv(dev);
	struct phy_device *phy = eth->phydev;
	u32 mask = ECMR_CHG_DM | ECMR_RE | ECMR_TE;
	int ret;

	/* Configure AVB-DMAC register */
	ravb_dmac_init(dev);

	/* Configure E-MAC registers */
	ravb_mac_init(eth);
	ravb_write_hwaddr(dev);

	ret = phy_startup(phy);
	if (ret)
		return ret;

	/* Set the transfer speed */
	if (phy->speed == 100)
		writel(0, eth->iobase + RAVB_REG_GECMR);
	else if (phy->speed == 1000)
		writel(1, eth->iobase + RAVB_REG_GECMR);

	/* Check if full duplex mode is supported by the phy */
	if (phy->duplex)
		mask |= ECMR_DM;

	writel(mask, eth->iobase + RAVB_REG_ECMR);

	phy->drv->writeext(phy, -1, 0x02, 0x08, (0x0f << 5) | 0x19);

	return 0;
}

static int ravb_start(struct udevice *dev)
{
	struct ravb_priv *eth = dev_get_priv(dev);
	int ret;

	ret = ravb_reset(dev);
	if (ret)
		return ret;

	ravb_base_desc_init(eth);
	ravb_tx_desc_init(eth);
	ravb_rx_desc_init(eth);

	ret = ravb_config(dev);
	if (ret)
		return ret;

	/* Setting the control will start the AVB-DMAC process. */
	writel(CCC_OPC_OPERATION, eth->iobase + RAVB_REG_CCC);

	return 0;
}

static void ravb_stop(struct udevice *dev)
{
	struct ravb_priv *eth = dev_get_priv(dev);

	phy_shutdown(eth->phydev);
	ravb_reset(dev);
}

static int ravb_probe(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	struct ravb_priv *eth = dev_get_priv(dev);
	struct ofnode_phandle_args phandle_args;
	struct mii_dev *mdiodev;
	void __iomem *iobase;
	int ret;

	iobase = map_physmem(pdata->iobase, 0x1000, MAP_NOCACHE);
	eth->iobase = iobase;

	ret = clk_get_by_index(dev, 0, &eth->clk);
	if (ret < 0)
		goto err_mdio_alloc;

	ret = dev_read_phandle_with_args(dev, "phy-handle", NULL, 0, 0, &phandle_args);
	if (!ret) {
		gpio_request_by_name_nodev(phandle_args.node, "reset-gpios", 0,
					   &eth->reset_gpio, GPIOD_IS_OUT);
	}

	if (!dm_gpio_is_valid(&eth->reset_gpio)) {
		gpio_request_by_name(dev, "reset-gpios", 0, &eth->reset_gpio,
				     GPIOD_IS_OUT);
	}

	mdiodev = mdio_alloc();
	if (!mdiodev) {
		ret = -ENOMEM;
		goto err_mdio_alloc;
	}

	mdiodev->read = bb_miiphy_read;
	mdiodev->write = bb_miiphy_write;
	bb_miiphy_buses[0].priv = eth;
	snprintf(mdiodev->name, sizeof(mdiodev->name), dev->name);

	ret = mdio_register(mdiodev);
	if (ret < 0)
		goto err_mdio_register;

	eth->bus = miiphy_get_dev_by_name(dev->name);

	/* Bring up PHY */
	ret = clk_enable(&eth->clk);
	if (ret)
		goto err_mdio_register;

	ret = ravb_reset(dev);
	if (ret)
		goto err_mdio_reset;

	ret = ravb_phy_config(dev);
	if (ret)
		goto err_mdio_reset;

	return 0;

err_mdio_reset:
	clk_disable(&eth->clk);
err_mdio_register:
	mdio_free(mdiodev);
err_mdio_alloc:
	unmap_physmem(eth->iobase, MAP_NOCACHE);
	return ret;
}

static int ravb_remove(struct udevice *dev)
{
	struct ravb_priv *eth = dev_get_priv(dev);

	clk_disable(&eth->clk);

	free(eth->phydev);
	mdio_unregister(eth->bus);
	mdio_free(eth->bus);
	if (dm_gpio_is_valid(&eth->reset_gpio))
		dm_gpio_free(dev, &eth->reset_gpio);
	unmap_physmem(eth->iobase, MAP_NOCACHE);

	return 0;
}

int ravb_bb_init(struct bb_miiphy_bus *bus)
{
	return 0;
}

int ravb_bb_mdio_active(struct bb_miiphy_bus *bus)
{
	struct ravb_priv *eth = bus->priv;

	setbits_le32(eth->iobase + RAVB_REG_PIR, PIR_MMD);

	return 0;
}

int ravb_bb_mdio_tristate(struct bb_miiphy_bus *bus)
{
	struct ravb_priv *eth = bus->priv;

	clrbits_le32(eth->iobase + RAVB_REG_PIR, PIR_MMD);

	return 0;
}

int ravb_bb_set_mdio(struct bb_miiphy_bus *bus, int v)
{
	struct ravb_priv *eth = bus->priv;

	if (v)
		setbits_le32(eth->iobase + RAVB_REG_PIR, PIR_MDO);
	else
		clrbits_le32(eth->iobase + RAVB_REG_PIR, PIR_MDO);

	return 0;
}

int ravb_bb_get_mdio(struct bb_miiphy_bus *bus, int *v)
{
	struct ravb_priv *eth = bus->priv;

	*v = (readl(eth->iobase + RAVB_REG_PIR) & PIR_MDI) >> 3;

	return 0;
}

int ravb_bb_set_mdc(struct bb_miiphy_bus *bus, int v)
{
	struct ravb_priv *eth = bus->priv;

	if (v)
		setbits_le32(eth->iobase + RAVB_REG_PIR, PIR_MDC);
	else
		clrbits_le32(eth->iobase + RAVB_REG_PIR, PIR_MDC);

	return 0;
}

int ravb_bb_delay(struct bb_miiphy_bus *bus)
{
	udelay(10);

	return 0;
}

struct bb_miiphy_bus bb_miiphy_buses[] = {
	{
		.name		= "ravb",
		.init		= ravb_bb_init,
		.mdio_active	= ravb_bb_mdio_active,
		.mdio_tristate	= ravb_bb_mdio_tristate,
		.set_mdio	= ravb_bb_set_mdio,
		.get_mdio	= ravb_bb_get_mdio,
		.set_mdc	= ravb_bb_set_mdc,
		.delay		= ravb_bb_delay,
	},
};
int bb_miiphy_buses_num = ARRAY_SIZE(bb_miiphy_buses);

static const struct eth_ops ravb_ops = {
	.start			= ravb_start,
	.send			= ravb_send,
	.recv			= ravb_recv,
	.free_pkt		= ravb_free_pkt,
	.stop			= ravb_stop,
	.write_hwaddr		= ravb_write_hwaddr,
};

int ravb_ofdata_to_platdata(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_platdata(dev);
	const char *phy_mode;
	const fdt32_t *cell;
	int ret = 0;

	pdata->iobase = devfdt_get_addr(dev);
	pdata->phy_interface = -1;
	phy_mode = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "phy-mode",
			       NULL);
	if (phy_mode)
		pdata->phy_interface = phy_get_interface_by_name(phy_mode);
	if (pdata->phy_interface == -1) {
		debug("%s: Invalid PHY interface '%s'\n", __func__, phy_mode);
		return -EINVAL;
	}

	pdata->max_speed = 1000;
	cell = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "max-speed", NULL);
	if (cell)
		pdata->max_speed = fdt32_to_cpu(*cell);

	sprintf(bb_miiphy_buses[0].name, dev->name);

	return ret;
}

static const struct udevice_id ravb_ids[] = {
	{ .compatible = "renesas,etheravb-r8a7795" },
	{ .compatible = "renesas,etheravb-r8a7796" },
	{ .compatible = "renesas,etheravb-r8a77965" },
	{ .compatible = "renesas,etheravb-r8a77970" },
	{ .compatible = "renesas,etheravb-r8a77990" },
	{ .compatible = "renesas,etheravb-r8a77995" },
	{ .compatible = "renesas,etheravb-rcar-gen3" },
	{ }
};

U_BOOT_DRIVER(eth_ravb) = {
	.name		= "ravb",
	.id		= UCLASS_ETH,
	.of_match	= ravb_ids,
	.ofdata_to_platdata = ravb_ofdata_to_platdata,
	.probe		= ravb_probe,
	.remove		= ravb_remove,
	.ops		= &ravb_ops,
	.priv_auto_alloc_size = sizeof(struct ravb_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA,
};
