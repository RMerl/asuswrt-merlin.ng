// SPDX-License-Identifier: GPL-2.0+
/*
 * uniphier_spi.c - Socionext UniPhier SPI driver
 * Copyright 2019 Socionext, Inc.
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <log.h>
#include <time.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <spi.h>
#include <wait_bit.h>
#include <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

#define SSI_CTL			0x00
#define   SSI_CTL_EN		BIT(0)

#define SSI_CKS			0x04
#define   SSI_CKS_CKRAT_MASK	GENMASK(7, 0)
#define   SSI_CKS_CKPHS		BIT(14)
#define   SSI_CKS_CKINIT	BIT(13)
#define   SSI_CKS_CKDLY		BIT(12)

#define SSI_TXWDS		0x08
#define   SSI_TXWDS_WDLEN_MASK	GENMASK(13, 8)
#define   SSI_TXWDS_TDTF_MASK	GENMASK(7, 6)
#define   SSI_TXWDS_DTLEN_MASK	GENMASK(5, 0)

#define SSI_RXWDS		0x0c
#define   SSI_RXWDS_RDTF_MASK	GENMASK(7, 6)
#define   SSI_RXWDS_DTLEN_MASK	GENMASK(5, 0)

#define SSI_FPS			0x10
#define   SSI_FPS_FSPOL		BIT(15)
#define   SSI_FPS_FSTRT		BIT(14)

#define SSI_SR			0x14
#define   SSI_SR_BUSY		BIT(7)
#define   SSI_SR_TNF		BIT(5)
#define   SSI_SR_RNE		BIT(0)

#define SSI_IE			0x18

#define SSI_IC			0x1c
#define   SSI_IC_TCIC		BIT(4)
#define   SSI_IC_RCIC		BIT(3)
#define   SSI_IC_RORIC		BIT(0)

#define SSI_FC			0x20
#define   SSI_FC_TXFFL		BIT(12)
#define   SSI_FC_TXFTH_MASK	GENMASK(11, 8)
#define   SSI_FC_RXFFL		BIT(4)
#define   SSI_FC_RXFTH_MASK	GENMASK(3, 0)

#define SSI_XDR			0x24	/* TXDR for write, RXDR for read */

#define SSI_FIFO_DEPTH		8U

#define SSI_REG_TIMEOUT		(CONFIG_SYS_HZ / 100)	/* 10 ms */
#define SSI_XFER_TIMEOUT	(CONFIG_SYS_HZ)		/* 1 sec */

#define SSI_CLK			50000000	/* internal I/O clock: 50MHz */

struct uniphier_spi_plat {
	void __iomem *base;
	u32 frequency;			/* input frequency */
	u32 speed_hz;
	uint deactivate_delay_us;	/* Delay to wait after deactivate */
	uint activate_delay_us;		/* Delay to wait after activate */
};

struct uniphier_spi_priv {
	void __iomem *base;
	u8 mode;
	u8 fifo_depth;
	u8 bits_per_word;
	ulong last_transaction_us;	/* Time of last transaction end */
};

static void uniphier_spi_enable(struct uniphier_spi_priv *priv, int enable)
{
	u32 val;

	val = readl(priv->base + SSI_CTL);
	if (enable)
		val |= SSI_CTL_EN;
	else
		val &= ~SSI_CTL_EN;
	writel(val, priv->base + SSI_CTL);
}

static void uniphier_spi_regdump(struct uniphier_spi_priv *priv)
{
	pr_debug("CTL   %08x\n", readl(priv->base + SSI_CTL));
	pr_debug("CKS   %08x\n", readl(priv->base + SSI_CKS));
	pr_debug("TXWDS %08x\n", readl(priv->base + SSI_TXWDS));
	pr_debug("RXWDS %08x\n", readl(priv->base + SSI_RXWDS));
	pr_debug("FPS   %08x\n", readl(priv->base + SSI_FPS));
	pr_debug("SR    %08x\n", readl(priv->base + SSI_SR));
	pr_debug("IE    %08x\n", readl(priv->base + SSI_IE));
	pr_debug("IC    %08x\n", readl(priv->base + SSI_IC));
	pr_debug("FC    %08x\n", readl(priv->base + SSI_FC));
	pr_debug("XDR   %08x\n", readl(priv->base + SSI_XDR));
}

static void spi_cs_activate(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct uniphier_spi_plat *plat = dev_get_plat(bus);
	struct uniphier_spi_priv *priv = dev_get_priv(bus);
	ulong delay_us;		/* The delay completed so far */
	u32 val;

	/* If it's too soon to do another transaction, wait */
	if (plat->deactivate_delay_us && priv->last_transaction_us) {
		delay_us = timer_get_us() - priv->last_transaction_us;
		if (delay_us < plat->deactivate_delay_us)
			udelay(plat->deactivate_delay_us - delay_us);
	}

	val = readl(priv->base + SSI_FPS);
	if (priv->mode & SPI_CS_HIGH)
		val |= SSI_FPS_FSPOL;
	else
		val &= ~SSI_FPS_FSPOL;
	writel(val, priv->base + SSI_FPS);

	if (plat->activate_delay_us)
		udelay(plat->activate_delay_us);
}

static void spi_cs_deactivate(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct uniphier_spi_plat *plat = dev_get_plat(bus);
	struct uniphier_spi_priv *priv = dev_get_priv(bus);
	u32 val;

	val = readl(priv->base + SSI_FPS);
	if (priv->mode & SPI_CS_HIGH)
		val &= ~SSI_FPS_FSPOL;
	else
		val |= SSI_FPS_FSPOL;
	writel(val, priv->base + SSI_FPS);

	/* Remember time of this transaction so we can honour the bus delay */
	if (plat->deactivate_delay_us)
		priv->last_transaction_us = timer_get_us();
}

static int uniphier_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct uniphier_spi_priv *priv = dev_get_priv(bus);
	u32 val, size;

	uniphier_spi_enable(priv, false);

	/* disable interrupts */
	writel(0, priv->base + SSI_IE);

	/* bits_per_word */
	size = priv->bits_per_word;
	val = readl(priv->base + SSI_TXWDS);
	val &= ~(SSI_TXWDS_WDLEN_MASK | SSI_TXWDS_DTLEN_MASK);
	val |= FIELD_PREP(SSI_TXWDS_WDLEN_MASK, size);
	val |= FIELD_PREP(SSI_TXWDS_DTLEN_MASK, size);
	writel(val, priv->base + SSI_TXWDS);

	val = readl(priv->base + SSI_RXWDS);
	val &= ~SSI_RXWDS_DTLEN_MASK;
	val |= FIELD_PREP(SSI_RXWDS_DTLEN_MASK, size);
	writel(val, priv->base + SSI_RXWDS);

	/* reset FIFOs */
	val = SSI_FC_TXFFL | SSI_FC_RXFFL;
	writel(val, priv->base + SSI_FC);

	/* FIFO threthold */
	val = readl(priv->base + SSI_FC);
	val &= ~(SSI_FC_TXFTH_MASK | SSI_FC_RXFTH_MASK);
	val |= FIELD_PREP(SSI_FC_TXFTH_MASK, priv->fifo_depth);
	val |= FIELD_PREP(SSI_FC_RXFTH_MASK, priv->fifo_depth);
	writel(val, priv->base + SSI_FC);

	/* clear interrupts */
	writel(SSI_IC_TCIC | SSI_IC_RCIC | SSI_IC_RORIC,
	       priv->base + SSI_IC);

	uniphier_spi_enable(priv, true);

	return 0;
}

static int uniphier_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct uniphier_spi_priv *priv = dev_get_priv(bus);

	uniphier_spi_enable(priv, false);

	return 0;
}

static int uniphier_spi_xfer(struct udevice *dev, unsigned int bitlen,
			     const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct uniphier_spi_priv *priv = dev_get_priv(bus);
	const u8 *tx_buf = dout;
	u8 *rx_buf = din, buf;
	u32 len = bitlen / 8;
	u32 tx_len, rx_len;
	u32 ts, status;
	int ret = 0;

	if (bitlen % 8) {
		dev_err(dev, "Non byte aligned SPI transfer\n");
		return -EINVAL;
	}

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(dev);

	uniphier_spi_enable(priv, true);

	ts = get_timer(0);
	tx_len = len;
	rx_len = len;

	uniphier_spi_regdump(priv);

	while (tx_len || rx_len) {
		ret = wait_for_bit_le32(priv->base + SSI_SR, SSI_SR_BUSY, false,
					SSI_REG_TIMEOUT * 1000, false);
		if (ret) {
			if (ret == -ETIMEDOUT)
				dev_err(dev, "access timeout\n");
			break;
		}

		status = readl(priv->base + SSI_SR);
		/* write the data into TX */
		if (tx_len && (status & SSI_SR_TNF)) {
			buf = tx_buf ? *tx_buf++ : 0;
			writel(buf, priv->base + SSI_XDR);
			tx_len--;
		}

		/* read the data from RX */
		if (rx_len && (status & SSI_SR_RNE)) {
			buf = readl(priv->base + SSI_XDR);
			if (rx_buf)
				*rx_buf++ = buf;
			rx_len--;
		}

		if (get_timer(ts) >= SSI_XFER_TIMEOUT) {
			dev_err(dev, "transfer timeout\n");
			ret = -ETIMEDOUT;
			break;
		}
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(dev);

	uniphier_spi_enable(priv, false);

	return ret;
}

static int uniphier_spi_set_speed(struct udevice *bus, uint speed)
{
	struct uniphier_spi_plat *plat = dev_get_plat(bus);
	struct uniphier_spi_priv *priv = dev_get_priv(bus);
	u32 val, ckdiv;

	if (speed > plat->frequency)
		speed = plat->frequency;

	/* baudrate */
	ckdiv = DIV_ROUND_UP(SSI_CLK, speed);
	ckdiv = round_up(ckdiv, 2);

	val = readl(priv->base + SSI_CKS);
	val &= ~SSI_CKS_CKRAT_MASK;
	val |= ckdiv & SSI_CKS_CKRAT_MASK;
	writel(val, priv->base + SSI_CKS);

	return 0;
}

static int uniphier_spi_set_mode(struct udevice *bus, uint mode)
{
	struct uniphier_spi_priv *priv = dev_get_priv(bus);
	u32 val1, val2;

	/*
	 * clock setting
	 * CKPHS    capture timing. 0:rising edge, 1:falling edge
	 * CKINIT   clock initial level. 0:low, 1:high
	 * CKDLY    clock delay. 0:no delay, 1:delay depending on FSTRT
	 *          (FSTRT=0: 1 clock, FSTRT=1: 0.5 clock)
	 *
	 * frame setting
	 * FSPOL    frame signal porarity. 0: low, 1: high
	 * FSTRT    start frame timing
	 *          0: rising edge of clock, 1: falling edge of clock
	 */
	val1 = readl(priv->base + SSI_CKS);
	val2 = readl(priv->base + SSI_FPS);

	switch (mode & (SPI_CPOL | SPI_CPHA)) {
	case SPI_MODE_0:
		/* CKPHS=1, CKINIT=0, CKDLY=1, FSTRT=0 */
		val1 |= SSI_CKS_CKPHS | SSI_CKS_CKDLY;
		val1 &= ~SSI_CKS_CKINIT;
		val2 &= ~SSI_FPS_FSTRT;
		break;
	case SPI_MODE_1:
		/* CKPHS=0, CKINIT=0, CKDLY=0, FSTRT=1 */
		val1 &= ~(SSI_CKS_CKPHS | SSI_CKS_CKINIT | SSI_CKS_CKDLY);
		val2 |= SSI_FPS_FSTRT;
		break;
	case SPI_MODE_2:
		/* CKPHS=0, CKINIT=1, CKDLY=1, FSTRT=1 */
		val1 |= SSI_CKS_CKINIT | SSI_CKS_CKDLY;
		val1 &= ~SSI_CKS_CKPHS;
		val2 |= SSI_FPS_FSTRT;
		break;
	case SPI_MODE_3:
		/* CKPHS=1, CKINIT=1, CKDLY=0, FSTRT=0 */
		val1 |= SSI_CKS_CKPHS | SSI_CKS_CKINIT;
		val1 &= ~SSI_CKS_CKDLY;
		val2 &= ~SSI_FPS_FSTRT;
		break;
	}

	writel(val1, priv->base + SSI_CKS);
	writel(val2, priv->base + SSI_FPS);

	/* format */
	val1 = readl(priv->base + SSI_TXWDS);
	val2 = readl(priv->base + SSI_RXWDS);
	if (mode & SPI_LSB_FIRST) {
		val1 |= FIELD_PREP(SSI_TXWDS_TDTF_MASK, 1);
		val2 |= FIELD_PREP(SSI_RXWDS_RDTF_MASK, 1);
	}
	writel(val1, priv->base + SSI_TXWDS);
	writel(val2, priv->base + SSI_RXWDS);

	priv->mode = mode;

	return 0;
}

static int uniphier_spi_of_to_plat(struct udevice *bus)
{
	struct uniphier_spi_plat *plat = dev_get_plat(bus);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(bus);

	plat->base = dev_read_addr_ptr(bus);

	plat->frequency =
		fdtdec_get_int(blob, node, "spi-max-frequency", 12500000);
	plat->deactivate_delay_us =
		fdtdec_get_int(blob, node, "spi-deactivate-delay", 0);
	plat->activate_delay_us =
		fdtdec_get_int(blob, node, "spi-activate-delay", 0);
	plat->speed_hz = plat->frequency / 2;

	return 0;
}

static int uniphier_spi_probe(struct udevice *bus)
{
	struct uniphier_spi_plat *plat = dev_get_plat(bus);
	struct uniphier_spi_priv *priv = dev_get_priv(bus);

	priv->base = plat->base;
	priv->fifo_depth = SSI_FIFO_DEPTH;
	priv->bits_per_word = 8;

	return 0;
}

static const struct dm_spi_ops uniphier_spi_ops = {
	.claim_bus	= uniphier_spi_claim_bus,
	.release_bus	= uniphier_spi_release_bus,
	.xfer		= uniphier_spi_xfer,
	.set_speed	= uniphier_spi_set_speed,
	.set_mode	= uniphier_spi_set_mode,
};

static const struct udevice_id uniphier_spi_ids[] = {
	{ .compatible = "socionext,uniphier-scssi" },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(uniphier_spi) = {
	.name	= "uniphier_spi",
	.id	= UCLASS_SPI,
	.of_match = uniphier_spi_ids,
	.ops	= &uniphier_spi_ops,
	.of_to_plat = uniphier_spi_of_to_plat,
	.plat_auto	= sizeof(struct uniphier_spi_plat),
	.priv_auto	= sizeof(struct uniphier_spi_priv),
	.probe	= uniphier_spi_probe,
};
