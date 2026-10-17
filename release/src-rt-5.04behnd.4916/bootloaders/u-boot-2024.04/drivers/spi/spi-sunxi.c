/*
 * (C) Copyright 2017 Whitebox Systems / Northend Systems B.V.
 * S.J.R. van Schaik <stephan@whiteboxsystems.nl>
 * M.B.W. Wajer <merlijn@whiteboxsystems.nl>
 *
 * (C) Copyright 2017 Olimex Ltd..
 * Stefan Mavrodiev <stefan@olimex.com>
 *
 * Based on linux spi driver. Original copyright follows:
 * linux/drivers/spi/spi-sun4i.c
 *
 * Copyright (C) 2012 - 2014 Allwinner Tech
 * Pan Nan <pannan@allwinnertech.com>
 *
 * Copyright (C) 2014 Maxime Ripard
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <spi.h>
#include <errno.h>
#include <fdt_support.h>
#include <reset.h>
#include <wait_bit.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>

#include <asm/bitops.h>
#include <asm/io.h>

#include <linux/iopoll.h>

DECLARE_GLOBAL_DATA_PTR;

/* sun4i spi registers */
#define SUN4I_RXDATA_REG		0x00
#define SUN4I_TXDATA_REG		0x04
#define SUN4I_CTL_REG			0x08
#define SUN4I_CLK_CTL_REG		0x1c
#define SUN4I_BURST_CNT_REG		0x20
#define SUN4I_XMIT_CNT_REG		0x24
#define SUN4I_FIFO_STA_REG		0x28

/* sun6i spi registers */
#define SUN6I_GBL_CTL_REG		0x04
#define SUN6I_TFR_CTL_REG		0x08
#define SUN6I_FIFO_CTL_REG		0x18
#define SUN6I_FIFO_STA_REG		0x1c
#define SUN6I_CLK_CTL_REG		0x24
#define SUN6I_BURST_CNT_REG		0x30
#define SUN6I_XMIT_CNT_REG		0x34
#define SUN6I_BURST_CTL_REG		0x38
#define SUN6I_TXDATA_REG		0x200
#define SUN6I_RXDATA_REG		0x300

/* sun spi bits */
#define SUN4I_CTL_ENABLE		BIT(0)
#define SUN4I_CTL_MASTER		BIT(1)
#define SUN4I_CLK_CTL_CDR2_MASK		0xff
#define SUN4I_CLK_CTL_CDR2(div)		((div) & SUN4I_CLK_CTL_CDR2_MASK)
#define SUN4I_CLK_CTL_CDR1_MASK		0xf
#define SUN4I_CLK_CTL_CDR1(div)		(((div) & SUN4I_CLK_CTL_CDR1_MASK) << 8)
#define SUN4I_CLK_CTL_DRS		BIT(12)
#define SUN4I_MAX_XFER_SIZE		0xffffff
#define SUN4I_BURST_CNT(cnt)		((cnt) & SUN4I_MAX_XFER_SIZE)
#define SUN4I_XMIT_CNT(cnt)		((cnt) & SUN4I_MAX_XFER_SIZE)
#define SUN4I_FIFO_STA_RF_CNT_BITS	0

#ifdef CONFIG_MACH_SUNIV
/* the AHB clock, which we programmed to be 1/3 of PLL_PERIPH@600MHz */
#define SUNXI_INPUT_CLOCK		200000000	/* 200 MHz */
#define SUN4I_SPI_MAX_RATE		(SUNXI_INPUT_CLOCK / 2)
#else
/* the SPI mod clock, defaulting to be 1/1 of the HOSC@24MHz */
#define SUNXI_INPUT_CLOCK		24000000	/* 24 MHz */
#define SUN4I_SPI_MAX_RATE		SUNXI_INPUT_CLOCK
#endif
#define SUN4I_SPI_MIN_RATE		3000
#define SUN4I_SPI_DEFAULT_RATE		1000000
#define SUN4I_SPI_TIMEOUT_MS		1000

#define SPI_REG(priv, reg)		((priv)->base + \
					(priv)->variant->regs[reg])
#define SPI_BIT(priv, bit)		((priv)->variant->bits[bit])
#define SPI_CS(priv, cs)		(((cs) << SPI_BIT(priv, SPI_TCR_CS_SEL)) & \
					SPI_BIT(priv, SPI_TCR_CS_MASK))

/* sun spi register set */
enum sun4i_spi_regs {
	SPI_GCR,
	SPI_TCR,
	SPI_FCR,
	SPI_FSR,
	SPI_CCR,
	SPI_BC,
	SPI_TC,
	SPI_BCTL,
	SPI_TXD,
	SPI_RXD,
};

/* sun spi register bits */
enum sun4i_spi_bits {
	SPI_GCR_TP,
	SPI_GCR_SRST,
	SPI_TCR_CPHA,
	SPI_TCR_CPOL,
	SPI_TCR_CS_ACTIVE_LOW,
	SPI_TCR_CS_SEL,
	SPI_TCR_CS_MASK,
	SPI_TCR_XCH,
	SPI_TCR_CS_MANUAL,
	SPI_TCR_CS_LEVEL,
	SPI_TCR_SDC,
	SPI_TCR_SDM,
	SPI_FCR_TF_RST,
	SPI_FCR_RF_RST,
	SPI_FSR_RF_CNT_MASK,
};

struct sun4i_spi_variant {
	const unsigned long *regs;
	const u32 *bits;
	u32 fifo_depth;
	bool has_soft_reset;
	bool has_burst_ctl;
	bool has_clk_ctl;
};

struct sun4i_spi_plat {
	struct sun4i_spi_variant *variant;
	u32 base;
	u32 max_hz;
};

struct sun4i_spi_priv {
	struct sun4i_spi_variant *variant;
	struct clk clk_ahb, clk_mod;
	struct reset_ctl reset;
	u32 base;
	u32 freq;
	u32 mode;

	const u8 *tx_buf;
	u8 *rx_buf;
};

static inline void sun4i_spi_drain_fifo(struct sun4i_spi_priv *priv, int len)
{
	u8 byte;

	while (len--) {
		byte = readb(SPI_REG(priv, SPI_RXD));
		if (priv->rx_buf)
			*priv->rx_buf++ = byte;
	}
}

static inline void sun4i_spi_fill_fifo(struct sun4i_spi_priv *priv, int len)
{
	u8 byte;

	while (len--) {
		byte = priv->tx_buf ? *priv->tx_buf++ : 0;
		writeb(byte, SPI_REG(priv, SPI_TXD));
	}
}

static void sun4i_spi_set_cs(struct udevice *bus, u8 cs, bool enable)
{
	struct sun4i_spi_priv *priv = dev_get_priv(bus);
	u32 reg;

	reg = readl(SPI_REG(priv, SPI_TCR));

	reg &= ~SPI_BIT(priv, SPI_TCR_CS_MASK);
	reg |= SPI_CS(priv, cs);

	if (enable)
		reg &= ~SPI_BIT(priv, SPI_TCR_CS_LEVEL);
	else
		reg |= SPI_BIT(priv, SPI_TCR_CS_LEVEL);

	writel(reg, SPI_REG(priv, SPI_TCR));
}

static inline int sun4i_spi_set_clock(struct udevice *dev, bool enable)
{
	struct sun4i_spi_priv *priv = dev_get_priv(dev);
	int ret;

	if (!enable) {
		clk_disable(&priv->clk_ahb);
		clk_disable(&priv->clk_mod);
		if (reset_valid(&priv->reset))
			reset_assert(&priv->reset);
		return 0;
	}

	ret = clk_enable(&priv->clk_ahb);
	if (ret) {
		dev_err(dev, "failed to enable ahb clock (ret=%d)\n", ret);
		return ret;
	}

	ret = clk_enable(&priv->clk_mod);
	if (ret) {
		dev_err(dev, "failed to enable mod clock (ret=%d)\n", ret);
		goto err_ahb;
	}

	if (reset_valid(&priv->reset)) {
		ret = reset_deassert(&priv->reset);
		if (ret) {
			dev_err(dev, "failed to deassert reset\n");
			goto err_mod;
		}
	}

	return 0;

err_mod:
	clk_disable(&priv->clk_mod);
err_ahb:
	clk_disable(&priv->clk_ahb);
	return ret;
}

static void sun4i_spi_set_speed_mode(struct udevice *dev)
{
	struct sun4i_spi_priv *priv = dev_get_priv(dev);
	unsigned int div;
	u32 reg;

	/*
	 * Setup clock divider.
	 *
	 * We have two choices there. Either we can use the clock
	 * divide rate 1, which is calculated thanks to this formula:
	 * SPI_CLK = MOD_CLK / (2 ^ (cdr + 1))
	 * Or we can use CDR2, which is calculated with the formula:
	 * SPI_CLK = MOD_CLK / (2 * (cdr + 1))
	 * Whether we use the former or the latter is set through the
	 * DRS bit.
	 *
	 * First try CDR2, and if we can't reach the expected
	 * frequency, fall back to CDR1.
	 */

	div = DIV_ROUND_UP(SUNXI_INPUT_CLOCK, priv->freq);
	reg = readl(SPI_REG(priv, SPI_CCR));

	if ((div / 2) <= (SUN4I_CLK_CTL_CDR2_MASK + 1)) {
		div /= 2;
		if (div > 0)
			div--;

		reg &= ~(SUN4I_CLK_CTL_CDR2_MASK | SUN4I_CLK_CTL_DRS);
		reg |= SUN4I_CLK_CTL_CDR2(div) | SUN4I_CLK_CTL_DRS;
	} else {
		div = fls(div - 1);
		/* The F1C100s encodes the divider as 2^(n+1) */
		if (IS_ENABLED(CONFIG_MACH_SUNIV))
			div--;
		reg &= ~((SUN4I_CLK_CTL_CDR1_MASK << 8) | SUN4I_CLK_CTL_DRS);
		reg |= SUN4I_CLK_CTL_CDR1(div);
	}

	writel(reg, SPI_REG(priv, SPI_CCR));

	reg = readl(SPI_REG(priv, SPI_TCR));
	reg &= ~(SPI_BIT(priv, SPI_TCR_CPOL) | SPI_BIT(priv, SPI_TCR_CPHA));

	if (priv->mode & SPI_CPOL)
		reg |= SPI_BIT(priv, SPI_TCR_CPOL);

	if (priv->mode & SPI_CPHA)
		reg |= SPI_BIT(priv, SPI_TCR_CPHA);

	writel(reg, SPI_REG(priv, SPI_TCR));
}

static int sun4i_spi_claim_bus(struct udevice *dev)
{
	struct sun4i_spi_priv *priv = dev_get_priv(dev->parent);
	int ret;

	ret = sun4i_spi_set_clock(dev->parent, true);
	if (ret)
		return ret;

	setbits_le32(SPI_REG(priv, SPI_GCR), SUN4I_CTL_ENABLE |
		     SUN4I_CTL_MASTER | SPI_BIT(priv, SPI_GCR_TP));

	if (priv->variant->has_soft_reset)
		setbits_le32(SPI_REG(priv, SPI_GCR),
			     SPI_BIT(priv, SPI_GCR_SRST));

	setbits_le32(SPI_REG(priv, SPI_TCR), SPI_BIT(priv, SPI_TCR_CS_MANUAL) |
		     SPI_BIT(priv, SPI_TCR_CS_ACTIVE_LOW));

	if (priv->variant->has_clk_ctl) {
		sun4i_spi_set_speed_mode(dev->parent);
	} else {
		/*
		 * At this moment there is no ability to change input clock.
		 * Therefore, we can only use default HOSC@24MHz clock and
		 * set SPI sampling mode to normal
		 */
		clrsetbits_le32(SPI_REG(priv, SPI_TCR),
				SPI_BIT(priv, SPI_TCR_SDC) |
				SPI_BIT(priv, SPI_TCR_SDM),
				SPI_BIT(priv, SPI_TCR_SDM));
	}

	return 0;
}

static int sun4i_spi_release_bus(struct udevice *dev)
{
	struct sun4i_spi_priv *priv = dev_get_priv(dev->parent);

	clrbits_le32(SPI_REG(priv, SPI_GCR), SUN4I_CTL_ENABLE);

	sun4i_spi_set_clock(dev->parent, false);

	return 0;
}

static int sun4i_spi_xfer(struct udevice *dev, unsigned int bitlen,
			  const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct sun4i_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);

	u32 len = bitlen / 8;
	u8 nbytes;
	int ret;

	priv->tx_buf = dout;
	priv->rx_buf = din;

	if (bitlen % 8) {
		debug("%s: non byte-aligned SPI transfer.\n", __func__);
		return -ENAVAIL;
	}

	if (flags & SPI_XFER_BEGIN)
		sun4i_spi_set_cs(bus, slave_plat->cs, true);

	/* Reset FIFOs */
	setbits_le32(SPI_REG(priv, SPI_FCR), SPI_BIT(priv, SPI_FCR_RF_RST) |
		     SPI_BIT(priv, SPI_FCR_TF_RST));

	while (len) {
		/* Setup the transfer now... */
		nbytes = min(len, (priv->variant->fifo_depth - 1));

		/* Setup the counters */
		writel(SUN4I_BURST_CNT(nbytes), SPI_REG(priv, SPI_BC));
		writel(SUN4I_XMIT_CNT(nbytes), SPI_REG(priv, SPI_TC));

		if (priv->variant->has_burst_ctl)
			writel(SUN4I_BURST_CNT(nbytes),
			       SPI_REG(priv, SPI_BCTL));

		/* Fill the TX FIFO */
		sun4i_spi_fill_fifo(priv, nbytes);

		/* Start the transfer */
		setbits_le32(SPI_REG(priv, SPI_TCR),
			     SPI_BIT(priv, SPI_TCR_XCH));

		/* Wait for the transfer to be done */
		ret = wait_for_bit_le32((const void *)SPI_REG(priv, SPI_TCR),
					SPI_BIT(priv, SPI_TCR_XCH),
					false, SUN4I_SPI_TIMEOUT_MS, false);
		if (ret < 0) {
			printf("ERROR: sun4i_spi: Timeout transferring data\n");
			sun4i_spi_set_cs(bus, slave_plat->cs, false);
			return ret;
		}

		/* Drain the RX FIFO */
		sun4i_spi_drain_fifo(priv, nbytes);

		len -= nbytes;
	}

	if (flags & SPI_XFER_END)
		sun4i_spi_set_cs(bus, slave_plat->cs, false);

	return 0;
}

static int sun4i_spi_set_speed(struct udevice *dev, uint speed)
{
	struct sun4i_spi_plat *plat = dev_get_plat(dev);
	struct sun4i_spi_priv *priv = dev_get_priv(dev);

	if (speed > plat->max_hz)
		speed = plat->max_hz;

	if (speed < SUN4I_SPI_MIN_RATE)
		speed = SUN4I_SPI_MIN_RATE;

	priv->freq = speed;

	return 0;
}

static int sun4i_spi_set_mode(struct udevice *dev, uint mode)
{
	struct sun4i_spi_priv *priv = dev_get_priv(dev);

	priv->mode = mode;

	return 0;
}

static const struct dm_spi_ops sun4i_spi_ops = {
	.claim_bus		= sun4i_spi_claim_bus,
	.release_bus		= sun4i_spi_release_bus,
	.xfer			= sun4i_spi_xfer,
	.set_speed		= sun4i_spi_set_speed,
	.set_mode		= sun4i_spi_set_mode,
};

static int sun4i_spi_probe(struct udevice *bus)
{
	struct sun4i_spi_plat *plat = dev_get_plat(bus);
	struct sun4i_spi_priv *priv = dev_get_priv(bus);
	int ret;

	ret = clk_get_by_name(bus, "ahb", &priv->clk_ahb);
	if (ret) {
		dev_err(bus, "failed to get ahb clock\n");
		return ret;
	}

	ret = clk_get_by_name(bus, "mod", &priv->clk_mod);
	if (ret) {
		dev_err(bus, "failed to get mod clock\n");
		return ret;
	}

	ret = reset_get_by_index(bus, 0, &priv->reset);
	if (ret && ret != -ENOENT) {
		dev_err(bus, "failed to get reset\n");
		return ret;
	}

	priv->variant = plat->variant;
	priv->base = plat->base;
	priv->freq = plat->max_hz;

	return 0;
}

static int sun4i_spi_of_to_plat(struct udevice *bus)
{
	struct sun4i_spi_plat *plat = dev_get_plat(bus);
	int node = dev_of_offset(bus);

	plat->base = dev_read_addr(bus);
	plat->variant = (struct sun4i_spi_variant *)dev_get_driver_data(bus);
	plat->max_hz = fdtdec_get_int(gd->fdt_blob, node,
				      "spi-max-frequency",
				      SUN4I_SPI_DEFAULT_RATE);

	if (plat->max_hz > SUN4I_SPI_MAX_RATE)
		plat->max_hz = SUN4I_SPI_MAX_RATE;

	return 0;
}

static const unsigned long sun4i_spi_regs[] = {
	[SPI_GCR]		= SUN4I_CTL_REG,
	[SPI_TCR]		= SUN4I_CTL_REG,
	[SPI_FCR]		= SUN4I_CTL_REG,
	[SPI_FSR]		= SUN4I_FIFO_STA_REG,
	[SPI_CCR]		= SUN4I_CLK_CTL_REG,
	[SPI_BC]		= SUN4I_BURST_CNT_REG,
	[SPI_TC]		= SUN4I_XMIT_CNT_REG,
	[SPI_TXD]		= SUN4I_TXDATA_REG,
	[SPI_RXD]		= SUN4I_RXDATA_REG,
};

static const u32 sun4i_spi_bits[] = {
	[SPI_GCR_TP]		= BIT(18),
	[SPI_TCR_CPHA]		= BIT(2),
	[SPI_TCR_CPOL]		= BIT(3),
	[SPI_TCR_CS_ACTIVE_LOW] = BIT(4),
	[SPI_TCR_XCH]		= BIT(10),
	[SPI_TCR_CS_SEL]	= 12,
	[SPI_TCR_CS_MASK]	= 0x3000,
	[SPI_TCR_CS_MANUAL]	= BIT(16),
	[SPI_TCR_CS_LEVEL]	= BIT(17),
	[SPI_FCR_TF_RST]	= BIT(8),
	[SPI_FCR_RF_RST]	= BIT(9),
	[SPI_FSR_RF_CNT_MASK]	= GENMASK(6, 0),
};

static const unsigned long sun6i_spi_regs[] = {
	[SPI_GCR]		= SUN6I_GBL_CTL_REG,
	[SPI_TCR]		= SUN6I_TFR_CTL_REG,
	[SPI_FCR]		= SUN6I_FIFO_CTL_REG,
	[SPI_FSR]		= SUN6I_FIFO_STA_REG,
	[SPI_CCR]		= SUN6I_CLK_CTL_REG,
	[SPI_BC]		= SUN6I_BURST_CNT_REG,
	[SPI_TC]		= SUN6I_XMIT_CNT_REG,
	[SPI_BCTL]		= SUN6I_BURST_CTL_REG,
	[SPI_TXD]		= SUN6I_TXDATA_REG,
	[SPI_RXD]		= SUN6I_RXDATA_REG,
};

static const u32 sun6i_spi_bits[] = {
	[SPI_GCR_TP]		= BIT(7),
	[SPI_GCR_SRST]		= BIT(31),
	[SPI_TCR_CPHA]		= BIT(0),
	[SPI_TCR_CPOL]		= BIT(1),
	[SPI_TCR_CS_ACTIVE_LOW] = BIT(2),
	[SPI_TCR_CS_SEL]	= 4,
	[SPI_TCR_CS_MASK]	= 0x30,
	[SPI_TCR_CS_MANUAL]	= BIT(6),
	[SPI_TCR_CS_LEVEL]	= BIT(7),
	[SPI_TCR_SDC]		= BIT(11),
	[SPI_TCR_SDM]		= BIT(13),
	[SPI_TCR_XCH]		= BIT(31),
	[SPI_FCR_RF_RST]	= BIT(15),
	[SPI_FCR_TF_RST]	= BIT(31),
	[SPI_FSR_RF_CNT_MASK]	= GENMASK(7, 0),
};

static const struct sun4i_spi_variant sun4i_a10_spi_variant = {
	.regs			= sun4i_spi_regs,
	.bits			= sun4i_spi_bits,
	.fifo_depth		= 64,
	.has_clk_ctl		= true,
};

static const struct sun4i_spi_variant sun6i_a31_spi_variant = {
	.regs			= sun6i_spi_regs,
	.bits			= sun6i_spi_bits,
	.fifo_depth		= 128,
	.has_soft_reset		= true,
	.has_burst_ctl		= true,
	.has_clk_ctl		= true,
};

static const struct sun4i_spi_variant sun8i_h3_spi_variant = {
	.regs			= sun6i_spi_regs,
	.bits			= sun6i_spi_bits,
	.fifo_depth		= 64,
	.has_soft_reset		= true,
	.has_burst_ctl		= true,
	.has_clk_ctl		= true,
};

static const struct sun4i_spi_variant sun50i_r329_spi_variant = {
	.regs			= sun6i_spi_regs,
	.bits			= sun6i_spi_bits,
	.fifo_depth		= 64,
	.has_soft_reset		= true,
	.has_burst_ctl		= true,
};

static const struct udevice_id sun4i_spi_ids[] = {
	{
	  .compatible = "allwinner,sun4i-a10-spi",
	  .data = (ulong)&sun4i_a10_spi_variant,
	},
	{
	  .compatible = "allwinner,sun6i-a31-spi",
	  .data = (ulong)&sun6i_a31_spi_variant,
	},
	{
	  .compatible = "allwinner,sun8i-h3-spi",
	  .data = (ulong)&sun8i_h3_spi_variant,
	},
	{
	  .compatible = "allwinner,sun50i-r329-spi",
	  .data = (ulong)&sun50i_r329_spi_variant,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sun4i_spi) = {
	.name	= "sun4i_spi",
	.id	= UCLASS_SPI,
	.of_match	= sun4i_spi_ids,
	.ops	= &sun4i_spi_ops,
	.of_to_plat	= sun4i_spi_of_to_plat,
	.plat_auto	= sizeof(struct sun4i_spi_plat),
	.priv_auto	= sizeof(struct sun4i_spi_priv),
	.probe	= sun4i_spi_probe,
};
