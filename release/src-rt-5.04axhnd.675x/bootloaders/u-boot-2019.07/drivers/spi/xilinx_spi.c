// SPDX-License-Identifier: GPL-2.0+
/*
 * Xilinx SPI driver
 *
 * Supports 8 bit SPI transfers only, with or w/o FIFO
 *
 * Based on bfin_spi.c, by way of altera_spi.c
 * Copyright (c) 2015 Jagan Teki <jteki@openedev.com>
 * Copyright (c) 2012 Stephan Linz <linz@li-pro.net>
 * Copyright (c) 2010 Graeme Smecher <graeme.smecher@mail.mcgill.ca>
 * Copyright (c) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Copyright (c) 2005-2008 Analog Devices Inc.
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include <wait_bit.h>

/*
 * [0]: http://www.xilinx.com/support/documentation
 *
 * Xilinx SPI Register Definitions
 * [1]:	[0]/ip_documentation/xps_spi.pdf
 *	page 8, Register Descriptions
 * [2]:	[0]/ip_documentation/axi_spi_ds742.pdf
 *	page 7, Register Overview Table
 */

/* SPI Control Register (spicr), [1] p9, [2] p8 */
#define SPICR_LSB_FIRST		BIT(9)
#define SPICR_MASTER_INHIBIT	BIT(8)
#define SPICR_MANUAL_SS		BIT(7)
#define SPICR_RXFIFO_RESEST	BIT(6)
#define SPICR_TXFIFO_RESEST	BIT(5)
#define SPICR_CPHA		BIT(4)
#define SPICR_CPOL		BIT(3)
#define SPICR_MASTER_MODE	BIT(2)
#define SPICR_SPE		BIT(1)
#define SPICR_LOOP		BIT(0)

/* SPI Status Register (spisr), [1] p11, [2] p10 */
#define SPISR_SLAVE_MODE_SELECT	BIT(5)
#define SPISR_MODF		BIT(4)
#define SPISR_TX_FULL		BIT(3)
#define SPISR_TX_EMPTY		BIT(2)
#define SPISR_RX_FULL		BIT(1)
#define SPISR_RX_EMPTY		BIT(0)

/* SPI Data Transmit Register (spidtr), [1] p12, [2] p12 */
#define SPIDTR_8BIT_MASK	GENMASK(7, 0)
#define SPIDTR_16BIT_MASK	GENMASK(15, 0)
#define SPIDTR_32BIT_MASK	GENMASK(31, 0)

/* SPI Data Receive Register (spidrr), [1] p12, [2] p12 */
#define SPIDRR_8BIT_MASK	GENMASK(7, 0)
#define SPIDRR_16BIT_MASK	GENMASK(15, 0)
#define SPIDRR_32BIT_MASK	GENMASK(31, 0)

/* SPI Slave Select Register (spissr), [1] p13, [2] p13 */
#define SPISSR_MASK(cs)		(1 << (cs))
#define SPISSR_ACT(cs)		~SPISSR_MASK(cs)
#define SPISSR_OFF		~0UL

/* SPI Software Reset Register (ssr) */
#define SPISSR_RESET_VALUE	0x0a

#define XILSPI_MAX_XFER_BITS	8
#define XILSPI_SPICR_DFLT_ON	(SPICR_MANUAL_SS | SPICR_MASTER_MODE | \
				SPICR_SPE)
#define XILSPI_SPICR_DFLT_OFF	(SPICR_MASTER_INHIBIT | SPICR_MANUAL_SS)

#ifndef CONFIG_XILINX_SPI_IDLE_VAL
#define CONFIG_XILINX_SPI_IDLE_VAL	GENMASK(7, 0)
#endif

#define XILINX_SPISR_TIMEOUT	10000 /* in milliseconds */

/* xilinx spi register set */
struct xilinx_spi_regs {
	u32 __space0__[7];
	u32 dgier;	/* Device Global Interrupt Enable Register (DGIER) */
	u32 ipisr;	/* IP Interrupt Status Register (IPISR) */
	u32 __space1__;
	u32 ipier;	/* IP Interrupt Enable Register (IPIER) */
	u32 __space2__[5];
	u32 srr;	/* Softare Reset Register (SRR) */
	u32 __space3__[7];
	u32 spicr;	/* SPI Control Register (SPICR) */
	u32 spisr;	/* SPI Status Register (SPISR) */
	u32 spidtr;	/* SPI Data Transmit Register (SPIDTR) */
	u32 spidrr;	/* SPI Data Receive Register (SPIDRR) */
	u32 spissr;	/* SPI Slave Select Register (SPISSR) */
	u32 spitfor;	/* SPI Transmit FIFO Occupancy Register (SPITFOR) */
	u32 spirfor;	/* SPI Receive FIFO Occupancy Register (SPIRFOR) */
};

/* xilinx spi priv */
struct xilinx_spi_priv {
	struct xilinx_spi_regs *regs;
	unsigned int freq;
	unsigned int mode;
	unsigned int fifo_depth;
	u8 startup;
};

static int xilinx_spi_probe(struct udevice *bus)
{
	struct xilinx_spi_priv *priv = dev_get_priv(bus);
	struct xilinx_spi_regs *regs = priv->regs;

	priv->regs = (struct xilinx_spi_regs *)dev_read_addr(bus);

	priv->fifo_depth = dev_read_u32_default(bus, "fifo-size", 0);

	writel(SPISSR_RESET_VALUE, &regs->srr);

	return 0;
}

static void spi_cs_activate(struct udevice *dev, uint cs)
{
	struct udevice *bus = dev_get_parent(dev);
	struct xilinx_spi_priv *priv = dev_get_priv(bus);
	struct xilinx_spi_regs *regs = priv->regs;

	writel(SPISSR_ACT(cs), &regs->spissr);
}

static void spi_cs_deactivate(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct xilinx_spi_priv *priv = dev_get_priv(bus);
	struct xilinx_spi_regs *regs = priv->regs;

	writel(SPISSR_OFF, &regs->spissr);
}

static int xilinx_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct xilinx_spi_priv *priv = dev_get_priv(bus);
	struct xilinx_spi_regs *regs = priv->regs;

	writel(SPISSR_OFF, &regs->spissr);
	writel(XILSPI_SPICR_DFLT_ON, &regs->spicr);

	return 0;
}

static int xilinx_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct xilinx_spi_priv *priv = dev_get_priv(bus);
	struct xilinx_spi_regs *regs = priv->regs;

	writel(SPISSR_OFF, &regs->spissr);
	writel(XILSPI_SPICR_DFLT_OFF, &regs->spicr);

	return 0;
}

static u32 xilinx_spi_fill_txfifo(struct udevice *bus, const u8 *txp,
				  u32 txbytes)
{
	struct xilinx_spi_priv *priv = dev_get_priv(bus);
	struct xilinx_spi_regs *regs = priv->regs;
	unsigned char d;
	u32 i = 0;

	while (txbytes && !(readl(&regs->spisr) & SPISR_TX_FULL) &&
	       i < priv->fifo_depth) {
		d = txp ? *txp++ : CONFIG_XILINX_SPI_IDLE_VAL;
		debug("spi_xfer: tx:%x ", d);
		/* write out and wait for processing (receive data) */
		writel(d & SPIDTR_8BIT_MASK, &regs->spidtr);
		txbytes--;
		i++;
	}

	return i;
}

static u32 xilinx_spi_read_rxfifo(struct udevice *bus, u8 *rxp, u32 rxbytes)
{
	struct xilinx_spi_priv *priv = dev_get_priv(bus);
	struct xilinx_spi_regs *regs = priv->regs;
	unsigned char d;
	unsigned int i = 0;

	while (rxbytes && !(readl(&regs->spisr) & SPISR_RX_EMPTY)) {
		d = readl(&regs->spidrr) & SPIDRR_8BIT_MASK;
		if (rxp)
			*rxp++ = d;
		debug("spi_xfer: rx:%x\n", d);
		rxbytes--;
		i++;
	}
	debug("Rx_done\n");

	return i;
}

static void xilinx_spi_startup_block(struct udevice *dev, unsigned int bytes,
				     const void *dout, void *din)
{
	struct udevice *bus = dev_get_parent(dev);
	struct xilinx_spi_priv *priv = dev_get_priv(bus);
	struct xilinx_spi_regs *regs = priv->regs;
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);
	const unsigned char *txp = dout;
	unsigned char *rxp = din;
	u32 reg, count;
	u32 txbytes = bytes;
	u32 rxbytes = bytes;

	/*
	 * This loop runs two times. First time to send the command.
	 * Second time to transfer data. After transferring data,
	 * it sets txp to the initial value for the normal operation.
	 */
	for ( ; priv->startup < 2; priv->startup++) {
		count = xilinx_spi_fill_txfifo(bus, txp, txbytes);
		reg = readl(&regs->spicr) & ~SPICR_MASTER_INHIBIT;
		writel(reg, &regs->spicr);
		count = xilinx_spi_read_rxfifo(bus, rxp, rxbytes);
		txp = din;

		if (priv->startup) {
			spi_cs_deactivate(dev);
			spi_cs_activate(dev, slave_plat->cs);
			txp = dout;
		}
	}
}

static int xilinx_spi_xfer(struct udevice *dev, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct xilinx_spi_priv *priv = dev_get_priv(bus);
	struct xilinx_spi_regs *regs = priv->regs;
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);
	/* assume spi core configured to do 8 bit transfers */
	unsigned int bytes = bitlen / XILSPI_MAX_XFER_BITS;
	const unsigned char *txp = dout;
	unsigned char *rxp = din;
	u32 txbytes = bytes;
	u32 rxbytes = bytes;
	u32 reg, count, timeout;
	int ret;

	debug("spi_xfer: bus:%i cs:%i bitlen:%i bytes:%i flags:%lx\n",
	      bus->seq, slave_plat->cs, bitlen, bytes, flags);

	if (bitlen == 0)
		goto done;

	if (bitlen % XILSPI_MAX_XFER_BITS) {
		printf("XILSPI warning: Not a multiple of %d bits\n",
		       XILSPI_MAX_XFER_BITS);
		flags |= SPI_XFER_END;
		goto done;
	}

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(dev, slave_plat->cs);

	/*
	 * This is the work around for the startup block issue in
	 * the spi controller. SPI clock is passing through STARTUP
	 * block to FLASH. STARTUP block don't provide clock as soon
	 * as QSPI provides command. So first command fails.
	 */
	xilinx_spi_startup_block(dev, bytes, dout, din);

	while (txbytes && rxbytes) {
		count = xilinx_spi_fill_txfifo(bus, txp, txbytes);
		reg = readl(&regs->spicr) & ~SPICR_MASTER_INHIBIT;
		writel(reg, &regs->spicr);
		txbytes -= count;
		if (txp)
			txp += count;

		ret = wait_for_bit_le32(&regs->spisr, SPISR_TX_EMPTY, true,
					XILINX_SPISR_TIMEOUT, false);
		if (ret < 0) {
			printf("XILSPI error: Xfer timeout\n");
			return ret;
		}

		debug("txbytes:0x%x,txp:0x%p\n", txbytes, txp);
		count = xilinx_spi_read_rxfifo(bus, rxp, rxbytes);
		rxbytes -= count;
		if (rxp)
			rxp += count;
		debug("rxbytes:0x%x rxp:0x%p\n", rxbytes, rxp);
	}

 done:
	if (flags & SPI_XFER_END)
		spi_cs_deactivate(dev);

	return 0;
}

static int xilinx_spi_set_speed(struct udevice *bus, uint speed)
{
	struct xilinx_spi_priv *priv = dev_get_priv(bus);

	priv->freq = speed;

	debug("xilinx_spi_set_speed: regs=%p, speed=%d\n", priv->regs,
	      priv->freq);

	return 0;
}

static int xilinx_spi_set_mode(struct udevice *bus, uint mode)
{
	struct xilinx_spi_priv *priv = dev_get_priv(bus);
	struct xilinx_spi_regs *regs = priv->regs;
	uint32_t spicr;

	spicr = readl(&regs->spicr);
	if (mode & SPI_LSB_FIRST)
		spicr |= SPICR_LSB_FIRST;
	if (mode & SPI_CPHA)
		spicr |= SPICR_CPHA;
	if (mode & SPI_CPOL)
		spicr |= SPICR_CPOL;
	if (mode & SPI_LOOP)
		spicr |= SPICR_LOOP;

	writel(spicr, &regs->spicr);
	priv->mode = mode;

	debug("xilinx_spi_set_mode: regs=%p, mode=%d\n", priv->regs,
	      priv->mode);

	return 0;
}

static const struct dm_spi_ops xilinx_spi_ops = {
	.claim_bus	= xilinx_spi_claim_bus,
	.release_bus	= xilinx_spi_release_bus,
	.xfer		= xilinx_spi_xfer,
	.set_speed	= xilinx_spi_set_speed,
	.set_mode	= xilinx_spi_set_mode,
};

static const struct udevice_id xilinx_spi_ids[] = {
	{ .compatible = "xlnx,xps-spi-2.00.a" },
	{ .compatible = "xlnx,xps-spi-2.00.b" },
	{ }
};

U_BOOT_DRIVER(xilinx_spi) = {
	.name	= "xilinx_spi",
	.id	= UCLASS_SPI,
	.of_match = xilinx_spi_ids,
	.ops	= &xilinx_spi_ops,
	.priv_auto_alloc_size = sizeof(struct xilinx_spi_priv),
	.probe	= xilinx_spi_probe,
};
