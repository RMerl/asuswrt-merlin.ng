// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 Xilinx, Inc.
 * (C) Copyright 2015 Jagan Teki <jteki@openedev.com>
 *
 * Xilinx Zynq PS SPI controller driver (master mode only)
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <time.h>
#include <clk.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

/* zynq spi register bit masks ZYNQ_SPI_<REG>_<BIT>_MASK */
#define ZYNQ_SPI_CR_MSA_MASK		BIT(15)	/* Manual start enb */
#define ZYNQ_SPI_CR_MCS_MASK		BIT(14)	/* Manual chip select */
#define ZYNQ_SPI_CR_CS_MASK		GENMASK(13, 10)	/* Chip select */
#define ZYNQ_SPI_CR_BAUD_MASK		GENMASK(5, 3)	/* Baud rate div */
#define ZYNQ_SPI_CR_CPHA_MASK		BIT(2)	/* Clock phase */
#define ZYNQ_SPI_CR_CPOL_MASK		BIT(1)	/* Clock polarity */
#define ZYNQ_SPI_CR_MSTREN_MASK		BIT(0)	/* Mode select */
#define ZYNQ_SPI_IXR_RXNEMPTY_MASK	BIT(4)	/* RX_FIFO_not_empty */
#define ZYNQ_SPI_IXR_TXOW_MASK		BIT(2)	/* TX_FIFO_not_full */
#define ZYNQ_SPI_IXR_ALL_MASK		GENMASK(6, 0)	/* All IXR bits */
#define ZYNQ_SPI_ENR_SPI_EN_MASK	BIT(0)	/* SPI Enable */

#define ZYNQ_SPI_CR_BAUD_MAX		8	/* Baud rate divisor max val */
#define ZYNQ_SPI_CR_BAUD_SHIFT		3	/* Baud rate divisor shift */
#define ZYNQ_SPI_CR_SS_SHIFT		10	/* Slave select shift */

#define ZYNQ_SPI_FIFO_DEPTH		128
#define ZYNQ_SPI_WAIT			(CONFIG_SYS_HZ / 100)	/* 10 ms */

/* zynq spi register set */
struct zynq_spi_regs {
	u32 cr;		/* 0x00 */
	u32 isr;	/* 0x04 */
	u32 ier;	/* 0x08 */
	u32 idr;	/* 0x0C */
	u32 imr;	/* 0x10 */
	u32 enr;	/* 0x14 */
	u32 dr;		/* 0x18 */
	u32 txdr;	/* 0x1C */
	u32 rxdr;	/* 0x20 */
};


/* zynq spi platform data */
struct zynq_spi_plat {
	struct zynq_spi_regs *regs;
	u32 frequency;		/* input frequency */
	u32 speed_hz;
	uint deactivate_delay_us;	/* Delay to wait after deactivate */
	uint activate_delay_us;		/* Delay to wait after activate */
};

/* zynq spi priv */
struct zynq_spi_priv {
	struct zynq_spi_regs *regs;
	u8 cs;
	u8 mode;
	ulong last_transaction_us;	/* Time of last transaction end */
	u8 fifo_depth;
	u32 freq;		/* required frequency */
};

static int zynq_spi_of_to_plat(struct udevice *bus)
{
	struct zynq_spi_plat *plat = dev_get_plat(bus);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(bus);

	plat->regs = dev_read_addr_ptr(bus);

	plat->deactivate_delay_us = fdtdec_get_int(blob, node,
					"spi-deactivate-delay", 0);
	plat->activate_delay_us = fdtdec_get_int(blob, node,
						 "spi-activate-delay", 0);

	return 0;
}

static void zynq_spi_init_hw(struct zynq_spi_priv *priv)
{
	struct zynq_spi_regs *regs = priv->regs;
	u32 confr;

	/* Disable SPI */
	confr = ZYNQ_SPI_ENR_SPI_EN_MASK;
	writel(~confr, &regs->enr);

	/* Disable Interrupts */
	writel(ZYNQ_SPI_IXR_ALL_MASK, &regs->idr);

	/* Clear RX FIFO */
	while (readl(&regs->isr) &
			ZYNQ_SPI_IXR_RXNEMPTY_MASK)
		readl(&regs->rxdr);

	/* Clear Interrupts */
	writel(ZYNQ_SPI_IXR_ALL_MASK, &regs->isr);

	/* Manual slave select and Auto start */
	confr = ZYNQ_SPI_CR_MCS_MASK | ZYNQ_SPI_CR_CS_MASK |
		ZYNQ_SPI_CR_MSTREN_MASK;
	confr &= ~ZYNQ_SPI_CR_MSA_MASK;
	writel(confr, &regs->cr);

	/* Enable SPI */
	writel(ZYNQ_SPI_ENR_SPI_EN_MASK, &regs->enr);
}

static int zynq_spi_probe(struct udevice *bus)
{
	struct zynq_spi_plat *plat = dev_get_plat(bus);
	struct zynq_spi_priv *priv = dev_get_priv(bus);
	struct clk clk;
	unsigned long clock;
	int ret;

	priv->regs = plat->regs;
	priv->fifo_depth = ZYNQ_SPI_FIFO_DEPTH;

	ret = clk_get_by_name(bus, "ref_clk", &clk);
	if (ret < 0) {
		dev_err(bus, "failed to get clock\n");
		return ret;
	}

	clock = clk_get_rate(&clk);
	if (IS_ERR_VALUE(clock)) {
		dev_err(bus, "failed to get rate\n");
		return clock;
	}

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(bus, "failed to enable clock\n");
		return ret;
	}

	/* init the zynq spi hw */
	zynq_spi_init_hw(priv);

	plat->frequency = clock;
	plat->speed_hz = plat->frequency / 2;

	debug("%s: max-frequency=%d\n", __func__, plat->speed_hz);

	return 0;
}

static void spi_cs_activate(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct zynq_spi_plat *plat = dev_get_plat(bus);
	struct zynq_spi_priv *priv = dev_get_priv(bus);
	struct zynq_spi_regs *regs = priv->regs;
	u32 cr;

	/* If it's too soon to do another transaction, wait */
	if (plat->deactivate_delay_us && priv->last_transaction_us) {
		ulong delay_us;		/* The delay completed so far */
		delay_us = timer_get_us() - priv->last_transaction_us;
		if (delay_us < plat->deactivate_delay_us)
			udelay(plat->deactivate_delay_us - delay_us);
	}

	clrbits_le32(&regs->cr, ZYNQ_SPI_CR_CS_MASK);
	cr = readl(&regs->cr);
	/*
	 * CS cal logic: CS[13:10]
	 * xxx0	- cs0
	 * xx01	- cs1
	 * x011 - cs2
	 */
	cr |= (~(1 << priv->cs) << ZYNQ_SPI_CR_SS_SHIFT) & ZYNQ_SPI_CR_CS_MASK;
	writel(cr, &regs->cr);

	if (plat->activate_delay_us)
		udelay(plat->activate_delay_us);
}

static void spi_cs_deactivate(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct zynq_spi_plat *plat = dev_get_plat(bus);
	struct zynq_spi_priv *priv = dev_get_priv(bus);
	struct zynq_spi_regs *regs = priv->regs;

	setbits_le32(&regs->cr, ZYNQ_SPI_CR_CS_MASK);

	/* Remember time of this transaction so we can honour the bus delay */
	if (plat->deactivate_delay_us)
		priv->last_transaction_us = timer_get_us();
}

static int zynq_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct zynq_spi_priv *priv = dev_get_priv(bus);
	struct zynq_spi_regs *regs = priv->regs;

	writel(ZYNQ_SPI_ENR_SPI_EN_MASK, &regs->enr);

	return 0;
}

static int zynq_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct zynq_spi_priv *priv = dev_get_priv(bus);
	struct zynq_spi_regs *regs = priv->regs;
	u32 confr;

	confr = ZYNQ_SPI_ENR_SPI_EN_MASK;
	writel(~confr, &regs->enr);

	return 0;
}

static int zynq_spi_xfer(struct udevice *dev, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct zynq_spi_priv *priv = dev_get_priv(bus);
	struct zynq_spi_regs *regs = priv->regs;
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	u32 len = bitlen / 8;
	u32 tx_len = len, rx_len = len, tx_tvl;
	const u8 *tx_buf = dout;
	u8 *rx_buf = din, buf;
	u32 ts, status;

	debug("spi_xfer: bus:%i cs:%i bitlen:%i len:%i flags:%lx\n",
	      dev_seq(bus), slave_plat->cs, bitlen, len, flags);

	if (bitlen % 8) {
		debug("spi_xfer: Non byte aligned SPI transfer\n");
		return -1;
	}

	priv->cs = slave_plat->cs;
	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(dev);

	while (rx_len > 0) {
		/* Write the data into TX FIFO - tx threshold is fifo_depth */
		tx_tvl = 0;
		while ((tx_tvl < priv->fifo_depth) && tx_len) {
			if (tx_buf)
				buf = *tx_buf++;
			else
				buf = 0;
			writel(buf, &regs->txdr);
			tx_len--;
			tx_tvl++;
		}

		/* Check TX FIFO completion */
		ts = get_timer(0);
		status = readl(&regs->isr);
		while (!(status & ZYNQ_SPI_IXR_TXOW_MASK)) {
			if (get_timer(ts) > ZYNQ_SPI_WAIT) {
				printf("spi_xfer: Timeout! TX FIFO not full\n");
				return -1;
			}
			status = readl(&regs->isr);
		}

		/* Read the data from RX FIFO */
		status = readl(&regs->isr);
		while ((status & ZYNQ_SPI_IXR_RXNEMPTY_MASK) && rx_len) {
			buf = readl(&regs->rxdr);
			if (rx_buf)
				*rx_buf++ = buf;
			status = readl(&regs->isr);
			rx_len--;
		}
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(dev);

	return 0;
}

static int zynq_spi_set_speed(struct udevice *bus, uint speed)
{
	struct zynq_spi_plat *plat = dev_get_plat(bus);
	struct zynq_spi_priv *priv = dev_get_priv(bus);
	struct zynq_spi_regs *regs = priv->regs;
	uint32_t confr;
	u8 baud_rate_val = 0;

	if (speed > plat->frequency)
		speed = plat->frequency;

	/* Set the clock frequency */
	confr = readl(&regs->cr);
	if (speed == 0) {
		/* Set baudrate x8, if the freq is 0 */
		baud_rate_val = 0x2;
	} else if (plat->speed_hz != speed) {
		while ((baud_rate_val < ZYNQ_SPI_CR_BAUD_MAX) &&
				((plat->frequency /
				(2 << baud_rate_val)) > speed))
			baud_rate_val++;
		plat->speed_hz = speed / (2 << baud_rate_val);
	}
	confr &= ~ZYNQ_SPI_CR_BAUD_MASK;
	confr |= (baud_rate_val << ZYNQ_SPI_CR_BAUD_SHIFT);

	writel(confr, &regs->cr);
	priv->freq = speed;

	debug("zynq_spi_set_speed: regs=%p, speed=%d\n",
	      priv->regs, priv->freq);

	return 0;
}

static int zynq_spi_set_mode(struct udevice *bus, uint mode)
{
	struct zynq_spi_priv *priv = dev_get_priv(bus);
	struct zynq_spi_regs *regs = priv->regs;
	uint32_t confr;

	/* Set the SPI Clock phase and polarities */
	confr = readl(&regs->cr);
	confr &= ~(ZYNQ_SPI_CR_CPHA_MASK | ZYNQ_SPI_CR_CPOL_MASK);

	if (mode & SPI_CPHA)
		confr |= ZYNQ_SPI_CR_CPHA_MASK;
	if (mode & SPI_CPOL)
		confr |= ZYNQ_SPI_CR_CPOL_MASK;

	writel(confr, &regs->cr);
	priv->mode = mode;

	debug("zynq_spi_set_mode: regs=%p, mode=%d\n", priv->regs, priv->mode);

	return 0;
}

static const struct dm_spi_ops zynq_spi_ops = {
	.claim_bus	= zynq_spi_claim_bus,
	.release_bus	= zynq_spi_release_bus,
	.xfer		= zynq_spi_xfer,
	.set_speed	= zynq_spi_set_speed,
	.set_mode	= zynq_spi_set_mode,
};

static const struct udevice_id zynq_spi_ids[] = {
	{ .compatible = "xlnx,zynq-spi-r1p6" },
	{ .compatible = "cdns,spi-r1p6" },
	{ }
};

U_BOOT_DRIVER(zynq_spi) = {
	.name	= "zynq_spi",
	.id	= UCLASS_SPI,
	.of_match = zynq_spi_ids,
	.ops	= &zynq_spi_ops,
	.of_to_plat = zynq_spi_of_to_plat,
	.plat_auto	= sizeof(struct zynq_spi_plat),
	.priv_auto	= sizeof(struct zynq_spi_priv),
	.probe	= zynq_spi_probe,
};
