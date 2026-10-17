// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 *
 * Driver for STMicroelectronics Serial peripheral interface (SPI)
 */

#define LOG_CATEGORY UCLASS_SPI

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <reset.h>
#include <spi.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/printk.h>

#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/bitfield.h>
#include <linux/iopoll.h>

/* STM32 SPI registers */
#define STM32_SPI_CR1		0x00
#define STM32_SPI_CR2		0x04
#define STM32_SPI_CFG1		0x08
#define STM32_SPI_CFG2		0x0C
#define STM32_SPI_SR		0x14
#define STM32_SPI_IFCR		0x18
#define STM32_SPI_TXDR		0x20
#define STM32_SPI_RXDR		0x30
#define STM32_SPI_I2SCFGR	0x50

/* STM32_SPI_CR1 bit fields */
#define SPI_CR1_SPE		BIT(0)
#define SPI_CR1_MASRX		BIT(8)
#define SPI_CR1_CSTART		BIT(9)
#define SPI_CR1_CSUSP		BIT(10)
#define SPI_CR1_HDDIR		BIT(11)
#define SPI_CR1_SSI		BIT(12)

/* STM32_SPI_CR2 bit fields */
#define SPI_CR2_TSIZE		GENMASK(15, 0)

/* STM32_SPI_CFG1 bit fields */
#define SPI_CFG1_DSIZE		GENMASK(4, 0)
#define SPI_CFG1_DSIZE_MIN	3
#define SPI_CFG1_FTHLV_SHIFT	5
#define SPI_CFG1_FTHLV		GENMASK(8, 5)
#define SPI_CFG1_MBR_SHIFT	28
#define SPI_CFG1_MBR		GENMASK(30, 28)
#define SPI_CFG1_MBR_MIN	0
#define SPI_CFG1_MBR_MAX	FIELD_GET(SPI_CFG1_MBR, SPI_CFG1_MBR)

/* STM32_SPI_CFG2 bit fields */
#define SPI_CFG2_COMM_SHIFT	17
#define SPI_CFG2_COMM		GENMASK(18, 17)
#define SPI_CFG2_MASTER		BIT(22)
#define SPI_CFG2_LSBFRST	BIT(23)
#define SPI_CFG2_CPHA		BIT(24)
#define SPI_CFG2_CPOL		BIT(25)
#define SPI_CFG2_SSM		BIT(26)
#define SPI_CFG2_AFCNTR		BIT(31)

/* STM32_SPI_SR bit fields */
#define SPI_SR_RXP		BIT(0)
#define SPI_SR_TXP		BIT(1)
#define SPI_SR_EOT		BIT(3)
#define SPI_SR_TXTF		BIT(4)
#define SPI_SR_OVR		BIT(6)
#define SPI_SR_SUSP		BIT(11)
#define SPI_SR_RXPLVL_SHIFT	13
#define SPI_SR_RXPLVL		GENMASK(14, 13)
#define SPI_SR_RXWNE		BIT(15)

/* STM32_SPI_IFCR bit fields */
#define SPI_IFCR_ALL		GENMASK(11, 3)

/* STM32_SPI_I2SCFGR bit fields */
#define SPI_I2SCFGR_I2SMOD	BIT(0)

#define MAX_CS_COUNT	4

/* SPI Master Baud Rate min/max divisor */
#define STM32_MBR_DIV_MIN	(2 << SPI_CFG1_MBR_MIN)
#define STM32_MBR_DIV_MAX	(2 << SPI_CFG1_MBR_MAX)

#define STM32_SPI_TIMEOUT_US	100000

/* SPI Communication mode */
#define SPI_FULL_DUPLEX		0
#define SPI_SIMPLEX_TX		1
#define SPI_SIMPLEX_RX		2
#define SPI_HALF_DUPLEX		3

struct stm32_spi_plat {
	void __iomem *base;
	struct clk clk;
	struct reset_ctl rst_ctl;
	struct gpio_desc cs_gpios[MAX_CS_COUNT];
};

struct stm32_spi_priv {
	ulong bus_clk_rate;
	unsigned int fifo_size;
	unsigned int cur_bpw;
	unsigned int cur_hz;
	unsigned int cur_xferlen; /* current transfer length in bytes */
	unsigned int tx_len;	  /* number of data to be written in bytes */
	unsigned int rx_len;	  /* number of data to be read in bytes */
	const void *tx_buf;	  /* data to be written, or NULL */
	void *rx_buf;		  /* data to be read, or NULL */
	u32 cur_mode;
	bool cs_high;
};

static void stm32_spi_write_txfifo(struct udevice *bus)
{
	struct stm32_spi_priv *priv = dev_get_priv(bus);
	struct stm32_spi_plat *plat = dev_get_plat(bus);
	void __iomem *base = plat->base;

	while ((priv->tx_len > 0) &&
	       (readl(base + STM32_SPI_SR) & SPI_SR_TXP)) {
		u32 offs = priv->cur_xferlen - priv->tx_len;

		if (priv->tx_len >= sizeof(u32) &&
		    IS_ALIGNED((uintptr_t)(priv->tx_buf + offs), sizeof(u32))) {
			const u32 *tx_buf32 = (const u32 *)(priv->tx_buf + offs);

			writel(*tx_buf32, base + STM32_SPI_TXDR);
			priv->tx_len -= sizeof(u32);
		} else if (priv->tx_len >= sizeof(u16) &&
			   IS_ALIGNED((uintptr_t)(priv->tx_buf + offs), sizeof(u16))) {
			const u16 *tx_buf16 = (const u16 *)(priv->tx_buf + offs);

			writew(*tx_buf16, base + STM32_SPI_TXDR);
			priv->tx_len -= sizeof(u16);
		} else {
			const u8 *tx_buf8 = (const u8 *)(priv->tx_buf + offs);

			writeb(*tx_buf8, base + STM32_SPI_TXDR);
			priv->tx_len -= sizeof(u8);
		}
	}

	log_debug("%d bytes left\n", priv->tx_len);
}

static void stm32_spi_read_rxfifo(struct udevice *bus)
{
	struct stm32_spi_priv *priv = dev_get_priv(bus);
	struct stm32_spi_plat *plat = dev_get_plat(bus);
	void __iomem *base = plat->base;
	u32 sr = readl(base + STM32_SPI_SR);
	u32 rxplvl = (sr & SPI_SR_RXPLVL) >> SPI_SR_RXPLVL_SHIFT;

	while ((priv->rx_len > 0) &&
	       ((sr & SPI_SR_RXP) ||
	       ((sr & SPI_SR_EOT) && ((sr & SPI_SR_RXWNE) || (rxplvl > 0))))) {
		u32 offs = priv->cur_xferlen - priv->rx_len;

		if (IS_ALIGNED((uintptr_t)(priv->rx_buf + offs), sizeof(u32)) &&
		    (priv->rx_len >= sizeof(u32) || (sr & SPI_SR_RXWNE))) {
			u32 *rx_buf32 = (u32 *)(priv->rx_buf + offs);

			*rx_buf32 = readl(base + STM32_SPI_RXDR);
			priv->rx_len -= sizeof(u32);
		} else if (IS_ALIGNED((uintptr_t)(priv->rx_buf + offs), sizeof(u16)) &&
			   (priv->rx_len >= sizeof(u16) ||
			    (!(sr & SPI_SR_RXWNE) &&
			    (rxplvl >= 2 || priv->cur_bpw > 8)))) {
			u16 *rx_buf16 = (u16 *)(priv->rx_buf + offs);

			*rx_buf16 = readw(base + STM32_SPI_RXDR);
			priv->rx_len -= sizeof(u16);
		} else {
			u8 *rx_buf8 = (u8 *)(priv->rx_buf + offs);

			*rx_buf8 = readb(base + STM32_SPI_RXDR);
			priv->rx_len -= sizeof(u8);
		}

		sr = readl(base + STM32_SPI_SR);
		rxplvl = (sr & SPI_SR_RXPLVL) >> SPI_SR_RXPLVL_SHIFT;
	}

	log_debug("%d bytes left\n", priv->rx_len);
}

static int stm32_spi_enable(void __iomem *base)
{
	log_debug("\n");

	/* Enable the SPI hardware */
	setbits_le32(base + STM32_SPI_CR1, SPI_CR1_SPE);

	return 0;
}

static int stm32_spi_disable(void __iomem *base)
{
	log_debug("\n");

	/* Disable the SPI hardware */
	clrbits_le32(base + STM32_SPI_CR1, SPI_CR1_SPE);

	return 0;
}

static int stm32_spi_claim_bus(struct udevice *slave)
{
	struct udevice *bus = dev_get_parent(slave);
	struct stm32_spi_plat *plat = dev_get_plat(bus);
	void __iomem *base = plat->base;

	dev_dbg(slave, "\n");

	/* Enable the SPI hardware */
	return stm32_spi_enable(base);
}

static int stm32_spi_release_bus(struct udevice *slave)
{
	struct udevice *bus = dev_get_parent(slave);
	struct stm32_spi_plat *plat = dev_get_plat(bus);
	void __iomem *base = plat->base;

	dev_dbg(slave, "\n");

	/* Disable the SPI hardware */
	return stm32_spi_disable(base);
}

static void stm32_spi_stopxfer(struct udevice *dev)
{
	struct stm32_spi_plat *plat = dev_get_plat(dev);
	void __iomem *base = plat->base;
	u32 cr1, sr;
	int ret;

	dev_dbg(dev, "\n");

	cr1 = readl(base + STM32_SPI_CR1);

	if (!(cr1 & SPI_CR1_SPE))
		return;

	/* Wait on EOT or suspend the flow */
	ret = readl_poll_timeout(base + STM32_SPI_SR, sr,
				 !(sr & SPI_SR_EOT), 100000);
	if (ret < 0) {
		if (cr1 & SPI_CR1_CSTART) {
			writel(cr1 | SPI_CR1_CSUSP, base + STM32_SPI_CR1);
			if (readl_poll_timeout(base + STM32_SPI_SR,
					       sr, !(sr & SPI_SR_SUSP),
					       100000) < 0)
				dev_err(dev, "Suspend request timeout\n");
		}
	}

	/* clear status flags */
	setbits_le32(base + STM32_SPI_IFCR, SPI_IFCR_ALL);
}

static int stm32_spi_set_cs(struct udevice *dev, unsigned int cs, bool enable)
{
	struct stm32_spi_plat *plat = dev_get_plat(dev);
	struct stm32_spi_priv *priv = dev_get_priv(dev);

	dev_dbg(dev, "cs=%d enable=%d\n", cs, enable);

	if (cs >= MAX_CS_COUNT)
		return -ENODEV;

	if (!dm_gpio_is_valid(&plat->cs_gpios[cs]))
		return -EINVAL;

	if (priv->cs_high)
		enable = !enable;

	return dm_gpio_set_value(&plat->cs_gpios[cs], enable ? 1 : 0);
}

static int stm32_spi_set_mode(struct udevice *bus, uint mode)
{
	struct stm32_spi_priv *priv = dev_get_priv(bus);
	struct stm32_spi_plat *plat = dev_get_plat(bus);
	void __iomem *base = plat->base;
	u32 cfg2_clrb = 0, cfg2_setb = 0;

	dev_dbg(bus, "mode=%d\n", mode);

	if (mode & SPI_CPOL)
		cfg2_setb |= SPI_CFG2_CPOL;
	else
		cfg2_clrb |= SPI_CFG2_CPOL;

	if (mode & SPI_CPHA)
		cfg2_setb |= SPI_CFG2_CPHA;
	else
		cfg2_clrb |= SPI_CFG2_CPHA;

	if (mode & SPI_LSB_FIRST)
		cfg2_setb |= SPI_CFG2_LSBFRST;
	else
		cfg2_clrb |= SPI_CFG2_LSBFRST;

	if (cfg2_clrb || cfg2_setb)
		clrsetbits_le32(base + STM32_SPI_CFG2,
				cfg2_clrb, cfg2_setb);

	if (mode & SPI_CS_HIGH)
		priv->cs_high = true;
	else
		priv->cs_high = false;
	return 0;
}

static int stm32_spi_set_fthlv(struct udevice *dev, u32 xfer_len)
{
	struct stm32_spi_priv *priv = dev_get_priv(dev);
	struct stm32_spi_plat *plat = dev_get_plat(dev);
	void __iomem *base = plat->base;
	u32 fthlv, half_fifo;

	/* data packet should not exceed 1/2 of fifo space */
	half_fifo = (priv->fifo_size / 2);

	/* data_packet should not exceed transfer length */
	fthlv = (half_fifo > xfer_len) ? xfer_len : half_fifo;

	/* align packet size with data registers access */
	fthlv -= (fthlv % 4);

	if (!fthlv)
		fthlv = 1;
	clrsetbits_le32(base + STM32_SPI_CFG1, SPI_CFG1_FTHLV,
			(fthlv - 1) << SPI_CFG1_FTHLV_SHIFT);

	return 0;
}

static int stm32_spi_set_speed(struct udevice *bus, uint hz)
{
	struct stm32_spi_priv *priv = dev_get_priv(bus);
	struct stm32_spi_plat *plat = dev_get_plat(bus);
	void __iomem *base = plat->base;
	u32 mbrdiv;
	long div;

	dev_dbg(bus, "hz=%d\n", hz);

	if (priv->cur_hz == hz)
		return 0;

	div = DIV_ROUND_UP(priv->bus_clk_rate, hz);

	if (div < STM32_MBR_DIV_MIN ||
	    div > STM32_MBR_DIV_MAX)
		return -EINVAL;

	/* Determine the first power of 2 greater than or equal to div */
	if (div & (div - 1))
		mbrdiv = fls(div);
	else
		mbrdiv = fls(div) - 1;

	if (!mbrdiv)
		return -EINVAL;

	clrsetbits_le32(base + STM32_SPI_CFG1, SPI_CFG1_MBR,
			(mbrdiv - 1) << SPI_CFG1_MBR_SHIFT);

	priv->cur_hz = hz;

	return 0;
}

static int stm32_spi_xfer(struct udevice *slave, unsigned int bitlen,
			  const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(slave);
	struct dm_spi_slave_plat *slave_plat;
	struct stm32_spi_priv *priv = dev_get_priv(bus);
	struct stm32_spi_plat *plat = dev_get_plat(bus);
	void __iomem *base = plat->base;
	u32 sr;
	u32 ifcr = 0;
	u32 xferlen;
	u32 mode;
	int xfer_status = 0;

	xferlen = bitlen / 8;

	if (xferlen <= SPI_CR2_TSIZE)
		writel(xferlen, base + STM32_SPI_CR2);
	else
		return -EMSGSIZE;

	priv->tx_buf = dout;
	priv->rx_buf = din;
	priv->tx_len = priv->tx_buf ? bitlen / 8 : 0;
	priv->rx_len = priv->rx_buf ? bitlen / 8 : 0;

	mode = SPI_FULL_DUPLEX;
	if (!priv->tx_buf)
		mode = SPI_SIMPLEX_RX;
	else if (!priv->rx_buf)
		mode = SPI_SIMPLEX_TX;

	if (priv->cur_xferlen != xferlen || priv->cur_mode != mode) {
		priv->cur_mode = mode;
		priv->cur_xferlen = xferlen;

		/* Disable the SPI hardware to unlock CFG1/CFG2 registers */
		stm32_spi_disable(base);

		clrsetbits_le32(base + STM32_SPI_CFG2, SPI_CFG2_COMM,
				mode << SPI_CFG2_COMM_SHIFT);

		stm32_spi_set_fthlv(bus, xferlen);

		/* Enable the SPI hardware */
		stm32_spi_enable(base);
	}

	dev_dbg(bus, "priv->tx_len=%d priv->rx_len=%d\n",
		priv->tx_len, priv->rx_len);

	slave_plat = dev_get_parent_plat(slave);
	if (flags & SPI_XFER_BEGIN)
		stm32_spi_set_cs(bus, slave_plat->cs, false);

	/* Be sure to have data in fifo before starting data transfer */
	if (priv->tx_buf)
		stm32_spi_write_txfifo(bus);

	setbits_le32(base + STM32_SPI_CR1, SPI_CR1_CSTART);

	while (1) {
		sr = readl(base + STM32_SPI_SR);

		if (sr & SPI_SR_OVR) {
			dev_err(bus, "Overrun: RX data lost\n");
			xfer_status = -EIO;
			break;
		}

		if (sr & SPI_SR_SUSP) {
			dev_warn(bus, "System too slow is limiting data throughput\n");

			if (priv->rx_buf && priv->rx_len > 0)
				stm32_spi_read_rxfifo(bus);

			ifcr |= SPI_SR_SUSP;
		}

		if (sr & SPI_SR_TXTF)
			ifcr |= SPI_SR_TXTF;

		if (sr & SPI_SR_TXP)
			if (priv->tx_buf && priv->tx_len > 0)
				stm32_spi_write_txfifo(bus);

		if (sr & SPI_SR_RXP)
			if (priv->rx_buf && priv->rx_len > 0)
				stm32_spi_read_rxfifo(bus);

		if (sr & SPI_SR_EOT) {
			if (priv->rx_buf && priv->rx_len > 0)
				stm32_spi_read_rxfifo(bus);
			break;
		}

		writel(ifcr, base + STM32_SPI_IFCR);
	}

	/* clear status flags */
	setbits_le32(base + STM32_SPI_IFCR, SPI_IFCR_ALL);
	stm32_spi_stopxfer(bus);

	if (flags & SPI_XFER_END)
		stm32_spi_set_cs(bus, slave_plat->cs, true);

	return xfer_status;
}

static int stm32_spi_get_fifo_size(struct udevice *dev)
{
	struct stm32_spi_plat *plat = dev_get_plat(dev);
	void __iomem *base = plat->base;
	u32 count = 0;

	stm32_spi_enable(base);

	while (readl(base + STM32_SPI_SR) & SPI_SR_TXP)
		writeb(++count, base + STM32_SPI_TXDR);

	stm32_spi_disable(base);

	dev_dbg(dev, "%d x 8-bit fifo size\n", count);

	return count;
}

static int stm32_spi_of_to_plat(struct udevice *dev)
{
	struct stm32_spi_plat *plat = dev_get_plat(dev);
	int ret;

	plat->base = dev_read_addr_ptr(dev);
	if (!plat->base) {
		dev_err(dev, "can't get registers base address\n");
		return -ENOENT;
	}

	ret = clk_get_by_index(dev, 0, &plat->clk);
	if (ret < 0)
		return ret;

	ret = reset_get_by_index(dev, 0, &plat->rst_ctl);
	if (ret < 0)
		return ret;

	ret = gpio_request_list_by_name(dev, "cs-gpios", plat->cs_gpios,
					ARRAY_SIZE(plat->cs_gpios), 0);
	if (ret < 0) {
		dev_err(dev, "Can't get %s cs gpios: %d", dev->name, ret);
		return -ENOENT;
	}

	return 0;
}

static int stm32_spi_probe(struct udevice *dev)
{
	struct stm32_spi_plat *plat = dev_get_plat(dev);
	struct stm32_spi_priv *priv = dev_get_priv(dev);
	void __iomem *base = plat->base;
	unsigned long clk_rate;
	int ret;
	unsigned int i;

	/* enable clock */
	ret = clk_enable(&plat->clk);
	if (ret < 0)
		return ret;

	clk_rate = clk_get_rate(&plat->clk);
	if (!clk_rate) {
		ret = -EINVAL;
		goto clk_err;
	}

	priv->bus_clk_rate = clk_rate;

	/* perform reset */
	reset_assert(&plat->rst_ctl);
	udelay(2);
	reset_deassert(&plat->rst_ctl);

	priv->fifo_size = stm32_spi_get_fifo_size(dev);
	priv->cur_mode = SPI_FULL_DUPLEX;
	priv->cur_xferlen = 0;
	priv->cur_bpw = SPI_DEFAULT_WORDLEN;
	clrsetbits_le32(base + STM32_SPI_CFG1, SPI_CFG1_DSIZE,
			priv->cur_bpw - 1);

	for (i = 0; i < ARRAY_SIZE(plat->cs_gpios); i++) {
		if (!dm_gpio_is_valid(&plat->cs_gpios[i]))
			continue;

		dm_gpio_set_dir_flags(&plat->cs_gpios[i],
				      GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	}

	/* Ensure I2SMOD bit is kept cleared */
	clrbits_le32(base + STM32_SPI_I2SCFGR, SPI_I2SCFGR_I2SMOD);

	/*
	 * - SS input value high
	 * - transmitter half duplex direction
	 * - automatic communication suspend when RX-Fifo is full
	 */
	setbits_le32(base + STM32_SPI_CR1,
		     SPI_CR1_SSI | SPI_CR1_HDDIR | SPI_CR1_MASRX);

	/*
	 * - Set the master mode (default Motorola mode)
	 * - Consider 1 master/n slaves configuration and
	 *   SS input value is determined by the SSI bit
	 * - keep control of all associated GPIOs
	 */
	setbits_le32(base + STM32_SPI_CFG2,
		     SPI_CFG2_MASTER | SPI_CFG2_SSM | SPI_CFG2_AFCNTR);

	return 0;

clk_err:
	clk_disable(&plat->clk);

	return ret;
};

static int stm32_spi_remove(struct udevice *dev)
{
	struct stm32_spi_plat *plat = dev_get_plat(dev);
	void __iomem *base = plat->base;
	int ret;

	stm32_spi_stopxfer(dev);
	stm32_spi_disable(base);

	ret = reset_assert(&plat->rst_ctl);
	if (ret < 0)
		return ret;

	reset_free(&plat->rst_ctl);

	return clk_disable(&plat->clk);
};

static const struct dm_spi_ops stm32_spi_ops = {
	.claim_bus	= stm32_spi_claim_bus,
	.release_bus	= stm32_spi_release_bus,
	.set_mode	= stm32_spi_set_mode,
	.set_speed	= stm32_spi_set_speed,
	.xfer		= stm32_spi_xfer,
};

static const struct udevice_id stm32_spi_ids[] = {
	{ .compatible = "st,stm32h7-spi", },
	{ }
};

U_BOOT_DRIVER(stm32_spi) = {
	.name			= "stm32_spi",
	.id			= UCLASS_SPI,
	.of_match		= stm32_spi_ids,
	.ops			= &stm32_spi_ops,
	.of_to_plat		= stm32_spi_of_to_plat,
	.plat_auto		= sizeof(struct stm32_spi_plat),
	.priv_auto		= sizeof(struct stm32_spi_priv),
	.probe			= stm32_spi_probe,
	.remove			= stm32_spi_remove,
};
