// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Nuvoton Technology.
 */

#include <common.h>
#include <dm.h>
#include <spi.h>
#include <clk.h>
#include <asm/gpio.h>
#include <linux/iopoll.h>

#define MAX_DIV	127

/* Register offsets */
#define PSPI_DATA		0
#define PSPI_CTL1		2
#define PSPI_STAT		4

/* PSPI_CTL1 fields */
#define PSPI_CTL1_SPIEN		BIT(0)
#define PSPI_CTL1_SCM		BIT(7)
#define PSPI_CTL1_SCIDL		BIT(8)
#define PSPI_CTL1_SCDV_MASK	GENMASK(15, 9)
#define PSPI_CTL1_SCDV_SHIFT	9

/* PSPI_STAT fields */
#define PSPI_STAT_BSY		BIT(0)
#define PSPI_STAT_RBF		BIT(1)

struct npcm_pspi_priv {
	void __iomem *base;
	struct clk clk;
	struct gpio_desc cs_gpio;
	u32 max_hz;
};

static inline void spi_cs_activate(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct npcm_pspi_priv *priv = dev_get_priv(bus);

	dm_gpio_set_value(&priv->cs_gpio, 1);
}

static inline void spi_cs_deactivate(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct npcm_pspi_priv *priv = dev_get_priv(bus);

	dm_gpio_set_value(&priv->cs_gpio, 0);
}

static inline void npcm_pspi_enable(struct npcm_pspi_priv *priv)
{
	u16 val;

	val = readw(priv->base + PSPI_CTL1);
	val |= PSPI_CTL1_SPIEN;
	writew(val, priv->base + PSPI_CTL1);
}

static inline void npcm_pspi_disable(struct npcm_pspi_priv *priv)
{
	u16 val;

	val = readw(priv->base + PSPI_CTL1);
	val &= ~PSPI_CTL1_SPIEN;
	writew(val, priv->base + PSPI_CTL1);
}

static int npcm_pspi_xfer(struct udevice *dev, unsigned int bitlen,
			  const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct npcm_pspi_priv *priv = dev_get_priv(bus);
	void __iomem *base = priv->base;
	const u8 *tx = dout;
	u8 *rx = din;
	u32 bytes = bitlen / 8;
	u8 tmp;
	u32 val;
	int i, ret = 0;

	npcm_pspi_enable(priv);

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(dev);

	for (i = 0; i < bytes; i++) {
		/* Making sure we can write */
		ret = readb_poll_timeout(base + PSPI_STAT, val,
					 !(val & PSPI_STAT_BSY),
					 1000000);
		if (ret < 0)
			break;

		if (tx)
			writeb(*tx++, base + PSPI_DATA);
		else
			writeb(0, base + PSPI_DATA);

		/* Wait till write completed */
		ret = readb_poll_timeout(base + PSPI_STAT, val,
					 !(val & PSPI_STAT_BSY),
					 1000000);
		if (ret < 0)
			break;

		/* Wait till read buffer full */
		ret = readb_poll_timeout(base + PSPI_STAT, val,
					 (val & PSPI_STAT_RBF),
					 1000000);
		if (ret < 0)
			break;

		tmp = readb(base + PSPI_DATA);
		if (rx)
			*rx++ = tmp;
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(dev);

	debug("npcm_pspi_xfer: slave %s:%s dout %08X din %08X bitlen %u\n",
	      dev->parent->name, dev->name, *(uint *)tx, *(uint *)rx, bitlen);

	npcm_pspi_disable(priv);

	return ret;
}

static int npcm_pspi_set_speed(struct udevice *bus, uint speed)
{
	struct npcm_pspi_priv *priv = dev_get_priv(bus);
	ulong apb_clock;
	u32 divisor;
	u16 val;

	apb_clock = clk_get_rate(&priv->clk);
	if (!apb_clock)
		return -EINVAL;

	if (speed > priv->max_hz)
		speed = priv->max_hz;

	divisor = DIV_ROUND_CLOSEST(apb_clock, (2 * speed)) - 1;
	if (divisor > MAX_DIV)
		divisor = MAX_DIV;

	val = readw(priv->base + PSPI_CTL1);
	val &= ~PSPI_CTL1_SCDV_MASK;
	val |= divisor << PSPI_CTL1_SCDV_SHIFT;
	writew(val, priv->base + PSPI_CTL1);

	debug("%s: apb_clock=%lu speed=%d divisor=%u\n",
	      __func__, apb_clock, speed, divisor);

	return 0;
}

static int npcm_pspi_set_mode(struct udevice *bus, uint mode)
{
	struct npcm_pspi_priv *priv = dev_get_priv(bus);
	u16 pspi_mode, val;

	switch (mode & (SPI_CPOL | SPI_CPHA)) {
	case SPI_MODE_0:
		pspi_mode = 0;
		break;
	case SPI_MODE_1:
		pspi_mode = PSPI_CTL1_SCM;
		break;
	case SPI_MODE_2:
		pspi_mode = PSPI_CTL1_SCIDL;
		break;
	case SPI_MODE_3:
		pspi_mode = PSPI_CTL1_SCIDL | PSPI_CTL1_SCM;
		break;
	default:
		break;
	}

	val = readw(priv->base + PSPI_CTL1);
	val &= ~(PSPI_CTL1_SCIDL | PSPI_CTL1_SCM);
	val |= pspi_mode;
	writew(val, priv->base + PSPI_CTL1);

	debug("%s: mode=%u\n", __func__, mode);
	return 0;
}

static int npcm_pspi_probe(struct udevice *bus)
{
	struct npcm_pspi_priv *priv = dev_get_priv(bus);
	int node = dev_of_offset(bus);
	int ret;

	ret = clk_get_by_index(bus, 0, &priv->clk);
	if (ret < 0)
		return ret;

	priv->base = dev_read_addr_ptr(bus);
	priv->max_hz = dev_read_u32_default(bus, "spi-max-frequency", 1000000);
	gpio_request_by_name_nodev(offset_to_ofnode(node), "cs-gpios", 0,
				   &priv->cs_gpio, GPIOD_IS_OUT| GPIOD_ACTIVE_LOW);

	return 0;
}

static const struct dm_spi_ops npcm_pspi_ops = {
	.xfer           = npcm_pspi_xfer,
	.set_speed      = npcm_pspi_set_speed,
	.set_mode       = npcm_pspi_set_mode,
};

static const struct udevice_id npcm_pspi_ids[] = {
	{ .compatible = "nuvoton,npcm845-pspi"},
	{ .compatible = "nuvoton,npcm750-pspi"},
	{ }
};

U_BOOT_DRIVER(npcm_pspi) = {
	.name   = "npcm_pspi",
	.id     = UCLASS_SPI,
	.of_match = npcm_pspi_ids,
	.ops    = &npcm_pspi_ops,
	.priv_auto = sizeof(struct npcm_pspi_priv),
	.probe  = npcm_pspi_probe,
};
