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
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <spi-mem.h>
#include <asm/io.h>
#include <wait_bit.h>
#include <linux/bitops.h>

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
#define SPISSR_OFF		(~0U)

/* SPI Software Reset Register (ssr) */
#define SPISSR_RESET_VALUE	0x0a

#define XILSPI_MAX_XFER_BITS	8
#define XILSPI_SPICR_DFLT_ON	(SPICR_MANUAL_SS | SPICR_MASTER_MODE | \
				SPICR_SPE | SPICR_MASTER_INHIBIT)
#define XILSPI_SPICR_DFLT_OFF	(SPICR_MASTER_INHIBIT | SPICR_MANUAL_SS)

#define XILINX_SPI_IDLE_VAL	GENMASK(7, 0)

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

static int xilinx_spi_find_buffer_size(struct xilinx_spi_regs *regs)
{
	u8 sr;
	int n_words = 0;

	/*
	 * Before the buffer_size detection reset the core
	 * to make sure to start with a clean state.
	 */
	writel(SPISSR_RESET_VALUE, &regs->srr);

	/* Fill the Tx FIFO with as many words as possible */
	do {
		writel(0, &regs->spidtr);
		sr = readl(&regs->spisr);
		n_words++;
	} while (!(sr & SPISR_TX_FULL));

	return n_words;
}

static int xilinx_spi_probe(struct udevice *bus)
{
	struct xilinx_spi_priv *priv = dev_get_priv(bus);
	struct xilinx_spi_regs *regs;

	regs = priv->regs = dev_read_addr_ptr(bus);
	priv->fifo_depth = dev_read_u32_default(bus, "fifo-size", 0);
	if (!priv->fifo_depth)
		priv->fifo_depth = xilinx_spi_find_buffer_size(regs);

	writel(SPISSR_RESET_VALUE, &regs->srr);

	/*
	 * Reset RX & TX FIFO
	 * Enable Manual Slave Select Assertion,
	 * Set SPI controller into master mode, and enable it
	 */
	writel(SPICR_RXFIFO_RESEST | SPICR_TXFIFO_RESEST |
	       SPICR_MANUAL_SS | SPICR_MASTER_MODE | SPICR_SPE,
	       &regs->spicr);

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
	u32 reg;

	reg = readl(&regs->spicr) | SPICR_RXFIFO_RESEST | SPICR_TXFIFO_RESEST;
	writel(reg, &regs->spicr);
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
		d = txp ? *txp++ : XILINX_SPI_IDLE_VAL;
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

static int start_transfer(struct udevice *dev, const void *dout, void *din, u32 len)
{
	struct udevice *bus = dev->parent;
	struct xilinx_spi_priv *priv = dev_get_priv(bus);
	struct xilinx_spi_regs *regs = priv->regs;
	u32 count, txbytes, rxbytes;
	int reg, ret;
	const unsigned char *txp = (const unsigned char *)dout;
	unsigned char *rxp = (unsigned char *)din;

	txbytes = len;
	rxbytes = len;
	while (txbytes || rxbytes) {
		/* Disable master transaction */
		reg = readl(&regs->spicr) | SPICR_MASTER_INHIBIT;
		writel(reg, &regs->spicr);
		count = xilinx_spi_fill_txfifo(bus, txp, txbytes);
		/* Enable master transaction */
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

		reg = readl(&regs->spicr) | SPICR_MASTER_INHIBIT;
		writel(reg, &regs->spicr);
		count = xilinx_spi_read_rxfifo(bus, rxp, rxbytes);
		rxbytes -= count;
		if (rxp)
			rxp += count;
	}

	return 0;
}

static void xilinx_spi_startup_block(struct udevice *dev)
{
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	unsigned char txp;
	unsigned char rxp[8];

	/*
	 * Perform a dummy read as a work around for
	 * the startup block issue.
	 */
	spi_cs_activate(dev, slave_plat->cs);
	txp = 0x9f;
	start_transfer(dev, (void *)&txp, NULL, 1);

	start_transfer(dev, NULL, (void *)rxp, 6);

	spi_cs_deactivate(dev);
}

static int xilinx_spi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	int ret;

	spi_cs_activate(dev, slave_plat->cs);
	ret = start_transfer(dev, dout, din, bitlen / 8);
	spi_cs_deactivate(dev);
	return ret;
}

static int xilinx_spi_mem_exec_op(struct spi_slave *spi,
				  const struct spi_mem_op *op)
{
	struct dm_spi_slave_plat *slave_plat =
				dev_get_parent_plat(spi->dev);
	static u32 startup;
	u32 dummy_len, ret;

	/*
	 * This is the work around for the startup block issue in
	 * the spi controller. SPI clock is passing through STARTUP
	 * block to FLASH. STARTUP block don't provide clock as soon
	 * as QSPI provides command. So first command fails.
	 */
	if (!startup) {
		xilinx_spi_startup_block(spi->dev);
		startup++;
	}

	spi_cs_activate(spi->dev, slave_plat->cs);

	if (op->cmd.opcode) {
		ret = start_transfer(spi->dev, (void *)&op->cmd.opcode,
				     NULL, 1);
		if (ret)
			goto done;
	}
	if (op->addr.nbytes) {
		int i;
		u8 addr_buf[4];

		for (i = 0; i < op->addr.nbytes; i++)
			addr_buf[i] = op->addr.val >>
			(8 * (op->addr.nbytes - i - 1));

		ret = start_transfer(spi->dev, (void *)addr_buf, NULL,
				     op->addr.nbytes);
		if (ret)
			goto done;
	}
	if (op->dummy.nbytes) {
		dummy_len = (op->dummy.nbytes * op->data.buswidth) /
			     op->dummy.buswidth;

		ret = start_transfer(spi->dev, NULL, NULL, dummy_len);
		if (ret)
			goto done;
	}
	if (op->data.nbytes) {
		if (op->data.dir == SPI_MEM_DATA_IN) {
			ret = start_transfer(spi->dev, NULL,
					     op->data.buf.in, op->data.nbytes);
		} else {
			ret = start_transfer(spi->dev, op->data.buf.out,
					     NULL, op->data.nbytes);
		}
		if (ret)
			goto done;
	}
done:
	spi_cs_deactivate(spi->dev);

	return ret;
}

static int xilinx_qspi_check_buswidth(struct spi_slave *slave, u8 width)
{
	u32 mode = slave->mode;

	switch (width) {
	case 1:
		return 0;
	case 2:
		if (mode & SPI_RX_DUAL)
			return 0;
		break;
	case 4:
		if (mode & SPI_RX_QUAD)
			return 0;
		break;
	}

	return -EOPNOTSUPP;
}

static bool xilinx_qspi_mem_exec_op(struct spi_slave *slave,
				    const struct spi_mem_op *op)
{
	if (xilinx_qspi_check_buswidth(slave, op->cmd.buswidth))
		return false;

	if (op->addr.nbytes &&
	    xilinx_qspi_check_buswidth(slave, op->addr.buswidth))
		return false;

	if (op->dummy.nbytes &&
	    xilinx_qspi_check_buswidth(slave, op->dummy.buswidth))
		return false;

	if (op->data.dir != SPI_MEM_NO_DATA &&
	    xilinx_qspi_check_buswidth(slave, op->data.buswidth))
		return false;

	return true;
}

static int xilinx_spi_set_speed(struct udevice *bus, uint speed)
{
	struct xilinx_spi_priv *priv = dev_get_priv(bus);

	priv->freq = speed;

	debug("%s: regs=%p, speed=%d\n", __func__, priv->regs, priv->freq);

	return 0;
}

static int xilinx_spi_set_mode(struct udevice *bus, uint mode)
{
	struct xilinx_spi_priv *priv = dev_get_priv(bus);
	struct xilinx_spi_regs *regs = priv->regs;
	u32 spicr;

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

	debug("%s: regs=%p, mode=%d\n", __func__, priv->regs, priv->mode);

	return 0;
}

static const struct spi_controller_mem_ops xilinx_spi_mem_ops = {
	.exec_op = xilinx_spi_mem_exec_op,
	.supports_op = xilinx_qspi_mem_exec_op,
};

static const struct dm_spi_ops xilinx_spi_ops = {
	.claim_bus	= xilinx_spi_claim_bus,
	.release_bus	= xilinx_spi_release_bus,
	.xfer           = xilinx_spi_xfer,
	.set_speed	= xilinx_spi_set_speed,
	.set_mode	= xilinx_spi_set_mode,
	.mem_ops	= &xilinx_spi_mem_ops,
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
	.priv_auto	= sizeof(struct xilinx_spi_priv),
	.probe	= xilinx_spi_probe,
};
