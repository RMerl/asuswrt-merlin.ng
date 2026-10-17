// SPDX-License-Identifier: GPL-2.0+
/*
 * spi-synquacer.c - Socionext Synquacer SPI driver
 * Copyright 2021 Linaro Ltd.
 * Copyright 2021 Socionext, Inc.
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <log.h>
#include <time.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <spi.h>
#include <wait_bit.h>

#define MCTRL	0x0
#define MEN	0
#define CSEN	1
#define IPCLK	3
#define MES	4
#define SYNCON	5

#define PCC0		0x4
#define PCC(n)		(PCC0 + (n) * 4)
#define RTM		3
#define ACES		2
#define SAFESYNC	16
#define CPHA		0
#define CPOL		1
#define SSPOL		4
#define SDIR		7
#define SS2CD		5
#define SENDIAN		8
#define CDRS_SHIFT	9
#define CDRS_MASK	0x7f

#define TXF		0x14
#define TXE		0x18
#define TXC		0x1c
#define RXF		0x20
#define RXE		0x24
#define RXC		0x28
#define TFES		1
#define TFLETE		4
#define TSSRS		6
#define RFMTE		5
#define RSSRS		6

#define FAULTF		0x2c
#define FAULTC		0x30

#define DMCFG		0x34
#define SSDC		1
#define MSTARTEN	2

#define DMSTART		0x38
#define TRIGGER		0
#define DMSTOP		8
#define CS_MASK		3
#define CS_SHIFT	16
#define DATA_TXRX	0
#define DATA_RX		1
#define DATA_TX		2
#define DATA_MASK	3
#define DATA_SHIFT	26
#define BUS_WIDTH	24

#define DMBCC		0x3c
#define DMSTATUS	0x40
#define RX_DATA_MASK	0x1f
#define RX_DATA_SHIFT	8
#define TX_DATA_MASK	0x1f
#define TX_DATA_SHIFT	16

#define TXBITCNT	0x44

#define FIFOCFG		0x4c
#define BPW_MASK	0x3
#define BPW_SHIFT	8
#define RX_FLUSH	11
#define TX_FLUSH	12
#define RX_TRSHLD_MASK		0xf
#define RX_TRSHLD_SHIFT		0
#define TX_TRSHLD_MASK		0xf
#define TX_TRSHLD_SHIFT		4

#define TXFIFO		0x50
#define RXFIFO		0x90
#define MID		0xfc

#define FIFO_DEPTH	16
#define TX_TRSHLD	4
#define RX_TRSHLD	(FIFO_DEPTH - TX_TRSHLD)

#define TXBIT	1
#define RXBIT	2

DECLARE_GLOBAL_DATA_PTR;

struct synquacer_spi_plat {
	void __iomem *base;
	bool aces, rtm;
};

struct synquacer_spi_priv {
	void __iomem *base;
	bool aces, rtm;
	int speed, cs, mode, rwflag;
	void *rx_buf;
	const void *tx_buf;
	unsigned int tx_words, rx_words;
};

static void read_fifo(struct synquacer_spi_priv *priv)
{
	u32 len = readl(priv->base + DMSTATUS);
	u8 *buf = priv->rx_buf;
	int i;

	len = (len >> RX_DATA_SHIFT) & RX_DATA_MASK;
	len = min_t(unsigned int, len, priv->rx_words);

	for (i = 0; i < len; i++)
		*buf++ = readb(priv->base + RXFIFO);

	priv->rx_buf = buf;
	priv->rx_words -= len;
}

static void write_fifo(struct synquacer_spi_priv *priv)
{
	u32 len = readl(priv->base + DMSTATUS);
	const u8 *buf = priv->tx_buf;
	int i;

	len = (len >> TX_DATA_SHIFT) & TX_DATA_MASK;
	len = min_t(unsigned int, FIFO_DEPTH - len, priv->tx_words);

	for (i = 0; i < len; i++)
		writeb(*buf++, priv->base + TXFIFO);

	priv->tx_buf = buf;
	priv->tx_words -= len;
}

static void synquacer_cs_set(struct synquacer_spi_priv *priv, bool active)
{
	u32 val;

	val = readl(priv->base + DMSTART);
	val &= ~(CS_MASK << CS_SHIFT);
	val |= priv->cs << CS_SHIFT;

	if (active) {
		writel(val, priv->base + DMSTART);

		val = readl(priv->base + DMSTART);
		val &= ~BIT(DMSTOP);
		writel(val, priv->base + DMSTART);
	} else {
		val |= BIT(DMSTOP);
		writel(val, priv->base + DMSTART);

		if (priv->rx_buf) {
			u32 buf[16];

			priv->rx_buf = buf;
			priv->rx_words = 16;
			read_fifo(priv);
		}

		/* wait until slave is deselected */
		while (!(readl(priv->base + TXF) & BIT(TSSRS)) ||
		       !(readl(priv->base + RXF) & BIT(RSSRS)))
			;
	}
}

static void synquacer_spi_config(struct udevice *dev, void *rx, const void *tx)
{
	struct udevice *bus = dev->parent;
	struct synquacer_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	u32 val, div, bus_width;
	int rwflag;

	rwflag = (rx ? 1 : 0) | (tx ? 2 : 0);

	/* if nothing to do */
	if (slave_plat->mode == priv->mode &&
	    rwflag == priv->rwflag &&
	    slave_plat->cs == priv->cs &&
	    slave_plat->max_hz == priv->speed)
		return;

	priv->rwflag = rwflag;
	priv->cs = slave_plat->cs;
	priv->mode = slave_plat->mode;
	priv->speed = slave_plat->max_hz;

	if (priv->mode & SPI_TX_DUAL)
		bus_width = 2;
	else if (priv->mode & SPI_TX_QUAD)
		bus_width = 4;
	else if (priv->mode & SPI_TX_OCTAL)
		bus_width = 8;
	else
		bus_width = 1; /* default is single bit mode */

	div = DIV_ROUND_UP(125000000, priv->speed);

	val = readl(priv->base + PCC(priv->cs));
	val &= ~BIT(RTM);
	val &= ~BIT(ACES);
	val &= ~BIT(SAFESYNC);
	if ((priv->mode & (SPI_TX_DUAL | SPI_RX_DUAL)) && div < 3)
		val |= BIT(SAFESYNC);
	if ((priv->mode & (SPI_TX_QUAD | SPI_RX_QUAD)) && div < 6)
		val |= BIT(SAFESYNC);

	if (priv->mode & SPI_CPHA)
		val |= BIT(CPHA);
	else
		val &= ~BIT(CPHA);

	if (priv->mode & SPI_CPOL)
		val |= BIT(CPOL);
	else
		val &= ~BIT(CPOL);

	if (priv->mode & SPI_CS_HIGH)
		val |= BIT(SSPOL);
	else
		val &= ~BIT(SSPOL);

	if (priv->mode & SPI_LSB_FIRST)
		val |= BIT(SDIR);
	else
		val &= ~BIT(SDIR);

	if (priv->aces)
		val |= BIT(ACES);

	if (priv->rtm)
		val |= BIT(RTM);

	val |= (3 << SS2CD);
	val |= BIT(SENDIAN);

	val &= ~(CDRS_MASK << CDRS_SHIFT);
	val |= ((div >> 1) << CDRS_SHIFT);

	writel(val, priv->base + PCC(priv->cs));

	val = readl(priv->base + FIFOCFG);
	val &= ~(BPW_MASK << BPW_SHIFT);
	val |= (0 << BPW_SHIFT);
	writel(val, priv->base + FIFOCFG);

	val = readl(priv->base + DMSTART);
	val &= ~(DATA_MASK << DATA_SHIFT);

	if (tx && rx)
		val |= (DATA_TXRX << DATA_SHIFT);
	else if (rx)
		val |= (DATA_RX << DATA_SHIFT);
	else
		val |= (DATA_TX << DATA_SHIFT);

	val &= ~(3 << BUS_WIDTH);
	val |= ((bus_width >> 1) << BUS_WIDTH);
	writel(val, priv->base + DMSTART);
}

static int synquacer_spi_xfer(struct udevice *dev, unsigned int bitlen,
			      const void *tx_buf, void *rx_buf,
			      unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct synquacer_spi_priv *priv = dev_get_priv(bus);
	u32 val, words, busy = 0;

	val = readl(priv->base + FIFOCFG);
	val |= (1 << RX_FLUSH);
	val |= (1 << TX_FLUSH);
	writel(val, priv->base + FIFOCFG);

	synquacer_spi_config(dev, rx_buf, tx_buf);

	priv->tx_buf = tx_buf;
	priv->rx_buf = rx_buf;

	words = bitlen / 8;

	if (tx_buf) {
		busy |= BIT(TXBIT);
		priv->tx_words = words;
	} else {
		busy &= ~BIT(TXBIT);
		priv->tx_words = 0;
	}

	if (rx_buf) {
		busy |= BIT(RXBIT);
		priv->rx_words = words;
	} else {
		busy &= ~BIT(RXBIT);
		priv->rx_words = 0;
	}

	if (flags & SPI_XFER_BEGIN)
		synquacer_cs_set(priv, true);

	if (tx_buf)
		write_fifo(priv);

	if (rx_buf) {
		val = readl(priv->base + FIFOCFG);
		val &= ~(RX_TRSHLD_MASK << RX_TRSHLD_SHIFT);
		val |= ((priv->rx_words > FIFO_DEPTH ?
			RX_TRSHLD : priv->rx_words) << RX_TRSHLD_SHIFT);
		writel(val, priv->base + FIFOCFG);
	}

	writel(~0, priv->base + TXC);
	writel(~0, priv->base + RXC);

	/* Trigger */
	if (flags & SPI_XFER_BEGIN) {
		val = readl(priv->base + DMSTART);
		val |= BIT(TRIGGER);
		writel(val, priv->base + DMSTART);
	}

	while (busy & (BIT(RXBIT) | BIT(TXBIT))) {
		if (priv->rx_words)
			read_fifo(priv);
		else
			busy &= ~BIT(RXBIT);

		if (priv->tx_words) {
			write_fifo(priv);
		} else {
			/* wait for shifter to empty out */
			while (!(readl(priv->base + TXF) & BIT(TFES)))
				cpu_relax();

			busy &= ~BIT(TXBIT);
		}
	}

	if (flags & SPI_XFER_END)
		synquacer_cs_set(priv, false);

	return 0;
}

static int synquacer_spi_set_speed(struct udevice *bus, uint speed)
{
	return 0;
}

static int synquacer_spi_set_mode(struct udevice *bus, uint mode)
{
	return 0;
}

static int synquacer_spi_claim_bus(struct udevice *dev)
{
	return 0;
}

static int synquacer_spi_release_bus(struct udevice *dev)
{
	return 0;
}

static void synquacer_spi_disable_module(struct synquacer_spi_priv *priv)
{
	writel(0, priv->base + MCTRL);
	while (readl(priv->base + MCTRL) & BIT(MES))
		cpu_relax();
}

static void synquacer_spi_init(struct synquacer_spi_priv *priv)
{
	u32 val;

	synquacer_spi_disable_module(priv);

	writel(0, priv->base + TXE);
	writel(0, priv->base + RXE);
	val = readl(priv->base + TXF);
	writel(val, priv->base + TXC);
	val = readl(priv->base + RXF);
	writel(val, priv->base + RXC);
	val = readl(priv->base + FAULTF);
	writel(val, priv->base + FAULTC);

	val = readl(priv->base + DMCFG);
	val &= ~BIT(SSDC);
	val &= ~BIT(MSTARTEN);
	writel(val, priv->base + DMCFG);

	/* Enable module with direct mode */
	val = readl(priv->base + MCTRL);
	val &= ~BIT(IPCLK);
	val &= ~BIT(CSEN);
	val |= BIT(MEN);
	val |= BIT(SYNCON);
	writel(val, priv->base + MCTRL);
}

static void synquacer_spi_exit(struct synquacer_spi_priv *priv)
{
	u32 val;

	synquacer_spi_disable_module(priv);

	/* Enable module with command sequence mode */
	val = readl(priv->base + MCTRL);
	val &= ~BIT(IPCLK);
	val |= BIT(CSEN);
	val |= BIT(MEN);
	val |= BIT(SYNCON);
	writel(val, priv->base + MCTRL);

	while (!(readl(priv->base + MCTRL) & BIT(MES)))
		cpu_relax();
}

static int synquacer_spi_probe(struct udevice *bus)
{
	struct synquacer_spi_plat *plat = dev_get_plat(bus);
	struct synquacer_spi_priv *priv = dev_get_priv(bus);

	priv->base = plat->base;
	priv->aces = plat->aces;
	priv->rtm = plat->rtm;

	synquacer_spi_init(priv);
	return 0;
}

static int synquacer_spi_remove(struct udevice *bus)
{
	struct synquacer_spi_priv *priv = dev_get_priv(bus);

	synquacer_spi_exit(priv);
	return 0;
}

static int synquacer_spi_of_to_plat(struct udevice *bus)
{
	struct synquacer_spi_plat *plat = dev_get_plat(bus);
	struct clk clk;

	plat->base = dev_read_addr_ptr(bus);

	plat->aces = dev_read_bool(bus, "socionext,set-aces");
	plat->rtm = dev_read_bool(bus, "socionext,use-rtm");

	clk_get_by_name(bus, "iHCLK", &clk);
	clk_enable(&clk);

	return 0;
}

static const struct dm_spi_ops synquacer_spi_ops = {
	.claim_bus	= synquacer_spi_claim_bus,
	.release_bus	= synquacer_spi_release_bus,
	.xfer		= synquacer_spi_xfer,
	.set_speed	= synquacer_spi_set_speed,
	.set_mode	= synquacer_spi_set_mode,
};

static const struct udevice_id synquacer_spi_ids[] = {
	{ .compatible = "socionext,synquacer-spi" },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(synquacer_spi) = {
	.name		= "synquacer_spi",
	.id		= UCLASS_SPI,
	.of_match	= synquacer_spi_ids,
	.ops		= &synquacer_spi_ops,
	.of_to_plat	= synquacer_spi_of_to_plat,
	.plat_auto	= sizeof(struct synquacer_spi_plat),
	.priv_auto	= sizeof(struct synquacer_spi_priv),
	.probe		= synquacer_spi_probe,
	.flags		= DM_FLAG_OS_PREPARE,
	.remove		= synquacer_spi_remove,
};
