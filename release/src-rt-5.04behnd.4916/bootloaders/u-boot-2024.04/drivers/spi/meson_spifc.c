// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2014 Beniamino Galvani <b.galvani@gmail.com>
 * Copyright (C) 2018 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 *
 * Amlogic Meson SPI Flash Controller driver
 */

#include <common.h>
#include <log.h>
#include <spi.h>
#include <clk.h>
#include <dm.h>
#include <regmap.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>

/* register map */
#define REG_CMD			0x00
#define REG_ADDR		0x04
#define REG_CTRL		0x08
#define REG_CTRL1		0x0c
#define REG_STATUS		0x10
#define REG_CTRL2		0x14
#define REG_CLOCK		0x18
#define REG_USER		0x1c
#define REG_USER1		0x20
#define REG_USER2		0x24
#define REG_USER3		0x28
#define REG_USER4		0x2c
#define REG_SLAVE		0x30
#define REG_SLAVE1		0x34
#define REG_SLAVE2		0x38
#define REG_SLAVE3		0x3c
#define REG_C0			0x40
#define REG_B8			0x60
#define REG_MAX			0x7c

/* register fields */
#define CMD_USER		BIT(18)
#define CTRL_ENABLE_AHB		BIT(17)
#define CLOCK_SOURCE		BIT(31)
#define CLOCK_DIV_SHIFT		12
#define CLOCK_DIV_MASK		(0x3f << CLOCK_DIV_SHIFT)
#define CLOCK_CNT_HIGH_SHIFT	6
#define CLOCK_CNT_HIGH_MASK	(0x3f << CLOCK_CNT_HIGH_SHIFT)
#define CLOCK_CNT_LOW_SHIFT	0
#define CLOCK_CNT_LOW_MASK	(0x3f << CLOCK_CNT_LOW_SHIFT)
#define USER_DIN_EN_MS		BIT(0)
#define USER_CMP_MODE		BIT(2)
#define USER_CLK_NOT_INV	BIT(7)
#define USER_UC_DOUT_SEL	BIT(27)
#define USER_UC_DIN_SEL		BIT(28)
#define USER_UC_MASK		((BIT(5) - 1) << 27)
#define USER1_BN_UC_DOUT_SHIFT	17
#define USER1_BN_UC_DOUT_MASK	(0xff << 16)
#define USER1_BN_UC_DIN_SHIFT	8
#define USER1_BN_UC_DIN_MASK	(0xff << 8)
#define USER4_CS_POL_HIGH	BIT(23)
#define USER4_IDLE_CLK_HIGH	BIT(29)
#define USER4_CS_ACT		BIT(30)
#define SLAVE_TRST_DONE		BIT(4)
#define SLAVE_OP_MODE		BIT(30)
#define SLAVE_SW_RST		BIT(31)

#define SPIFC_BUFFER_SIZE	64

struct meson_spifc_priv {
	struct regmap			*regmap;
	struct clk			clk;
};

/**
 * meson_spifc_drain_buffer() - copy data from device buffer to memory
 * @spifc:	the Meson SPI device
 * @buf:	the destination buffer
 * @len:	number of bytes to copy
 */
static void meson_spifc_drain_buffer(struct meson_spifc_priv *spifc,
				     u8 *buf, int len)
{
	u32 data;
	int i = 0;

	while (i < len) {
		regmap_read(spifc->regmap, REG_C0 + i, &data);

		if (len - i >= 4) {
			*((u32 *)buf) = data;
			buf += 4;
		} else {
			memcpy(buf, &data, len - i);
			break;
		}
		i += 4;
	}
}

/**
 * meson_spifc_fill_buffer() - copy data from memory to device buffer
 * @spifc:	the Meson SPI device
 * @buf:	the source buffer
 * @len:	number of bytes to copy
 */
static void meson_spifc_fill_buffer(struct meson_spifc_priv *spifc,
				    const u8 *buf, int len)
{
	u32 data = 0;
	int i = 0;

	while (i < len) {
		if (len - i >= 4)
			data = *(u32 *)buf;
		else
			memcpy(&data, buf, len - i);

		regmap_write(spifc->regmap, REG_C0 + i, data);

		buf += 4;
		i += 4;
	}
}

/**
 * meson_spifc_txrx() - transfer a chunk of data
 * @spifc:	the Meson SPI device
 * @dout:	data buffer for TX
 * @din:	data buffer for RX
 * @offset:	offset of the data to transfer
 * @len:	length of the data to transfer
 * @last_xfer:	whether this is the last transfer of the message
 * @last_chunk:	whether this is the last chunk of the transfer
 * Return:	0 on success, a negative value on error
 */
static int meson_spifc_txrx(struct meson_spifc_priv *spifc,
			    const u8 *dout, u8 *din, int offset,
			    int len, bool last_xfer, bool last_chunk)
{
	bool keep_cs = true;
	u32 data;
	int ret;

	if (dout)
		meson_spifc_fill_buffer(spifc, dout + offset, len);

	/* enable DOUT stage */
	regmap_update_bits(spifc->regmap, REG_USER, USER_UC_MASK,
			   USER_UC_DOUT_SEL);
	regmap_write(spifc->regmap, REG_USER1,
		     (8 * len - 1) << USER1_BN_UC_DOUT_SHIFT);

	/* enable data input during DOUT */
	regmap_update_bits(spifc->regmap, REG_USER, USER_DIN_EN_MS,
			   USER_DIN_EN_MS);

	if (last_chunk && last_xfer)
		keep_cs = false;

	regmap_update_bits(spifc->regmap, REG_USER4, USER4_CS_ACT,
			   keep_cs ? USER4_CS_ACT : 0);

	/* clear transition done bit */
	regmap_update_bits(spifc->regmap, REG_SLAVE, SLAVE_TRST_DONE, 0);
	/* start transfer */
	regmap_update_bits(spifc->regmap, REG_CMD, CMD_USER, CMD_USER);

	/* wait for the current operation to terminate */
	ret = regmap_read_poll_timeout(spifc->regmap, REG_SLAVE, data,
				       (data & SLAVE_TRST_DONE),
				       0, 5 * CONFIG_SYS_HZ);

	if (!ret && din)
		meson_spifc_drain_buffer(spifc, din + offset, len);

	return ret;
}

/**
 * meson_spifc_xfer() - perform a single transfer
 * @dev:	the SPI controller device
 * @bitlen:	length of the transfer
 * @dout:	data buffer for TX
 * @din:	data buffer for RX
 * @flags:	transfer flags
 * Return:	0 on success, a negative value on error
 */
static int meson_spifc_xfer(struct udevice *slave, unsigned int bitlen,
			    const void *dout, void *din, unsigned long flags)
{
	struct meson_spifc_priv *spifc = dev_get_priv(slave->parent);
	int blen = bitlen / 8;
	int len, done = 0, ret = 0;

	if (bitlen % 8)
		return -EINVAL;

	debug("xfer len %d (%d) dout %p din %p\n", bitlen, blen, dout, din);

	regmap_update_bits(spifc->regmap, REG_CTRL, CTRL_ENABLE_AHB, 0);

	while (done < blen && !ret) {
		len = min_t(int, blen - done, SPIFC_BUFFER_SIZE);
		ret = meson_spifc_txrx(spifc, dout, din, done, len,
				       flags & SPI_XFER_END,
				       done + len >= blen);
		done += len;
	}

	regmap_update_bits(spifc->regmap, REG_CTRL, CTRL_ENABLE_AHB,
			   CTRL_ENABLE_AHB);

	return ret;
}

/**
 * meson_spifc_set_speed() - program the clock divider
 * @dev:	the SPI controller device
 * @speed:	desired speed in Hz
 */
static int meson_spifc_set_speed(struct udevice *dev, uint speed)
{
	struct meson_spifc_priv *spifc = dev_get_priv(dev);
	unsigned long parent, value;
	int n;

	parent = clk_get_rate(&spifc->clk);
	n = max_t(int, parent / speed - 1, 1);

	debug("parent %lu, speed %u, n %d\n", parent, speed, n);

	value = (n << CLOCK_DIV_SHIFT) & CLOCK_DIV_MASK;
	value |= (n << CLOCK_CNT_LOW_SHIFT) & CLOCK_CNT_LOW_MASK;
	value |= (((n + 1) / 2 - 1) << CLOCK_CNT_HIGH_SHIFT) &
		CLOCK_CNT_HIGH_MASK;

	regmap_write(spifc->regmap, REG_CLOCK, value);

	return 0;
}

/**
 * meson_spifc_set_mode() - setups the SPI bus mode
 * @dev:	the SPI controller device
 * @mode:	desired mode bitfield
 * Return:	0 on success, -ENODEV on error
 */
static int meson_spifc_set_mode(struct udevice *dev, uint mode)
{
	struct meson_spifc_priv *spifc = dev_get_priv(dev);

	if (mode & (SPI_CPHA | SPI_RX_QUAD | SPI_RX_DUAL |
		    SPI_TX_QUAD | SPI_TX_DUAL))
		return -ENODEV;

	regmap_update_bits(spifc->regmap, REG_USER, USER_CLK_NOT_INV,
			   mode & SPI_CPOL ? USER_CLK_NOT_INV : 0);

	regmap_update_bits(spifc->regmap, REG_USER4, USER4_CS_POL_HIGH,
			   mode & SPI_CS_HIGH ? USER4_CS_POL_HIGH : 0);

	return 0;
}

/**
 * meson_spifc_hw_init() - reset and initialize the SPI controller
 * @spifc:	the Meson SPI device
 */
static void meson_spifc_hw_init(struct meson_spifc_priv *spifc)
{
	/* reset device */
	regmap_update_bits(spifc->regmap, REG_SLAVE, SLAVE_SW_RST,
			   SLAVE_SW_RST);
	/* disable compatible mode */
	regmap_update_bits(spifc->regmap, REG_USER, USER_CMP_MODE, 0);
	/* set master mode */
	regmap_update_bits(spifc->regmap, REG_SLAVE, SLAVE_OP_MODE, 0);
}

static const struct dm_spi_ops meson_spifc_ops = {
	.xfer		= meson_spifc_xfer,
	.set_speed	= meson_spifc_set_speed,
	.set_mode	= meson_spifc_set_mode,
};

static int meson_spifc_probe(struct udevice *dev)
{
	struct meson_spifc_priv *priv = dev_get_priv(dev);
	int ret;

	ret = regmap_init_mem(dev_ofnode(dev), &priv->regmap);
	if (ret)
		return ret;

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret)
		return ret;

	meson_spifc_hw_init(priv);

	return 0;
}

static const struct udevice_id meson_spifc_ids[] = {
	{ .compatible = "amlogic,meson-gxbb-spifc", },
	{ }
};

U_BOOT_DRIVER(meson_spifc) = {
	.name		= "meson_spifc",
	.id		= UCLASS_SPI,
	.of_match	= meson_spifc_ids,
	.ops		= &meson_spifc_ops,
	.probe		= meson_spifc_probe,
	.priv_auto	= sizeof(struct meson_spifc_priv),
};
