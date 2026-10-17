// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2006 Ben Warren, Qstreams Networks Inc.
 * With help from the common/soft_spi and arch/powerpc/cpu/mpc8260 drivers
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <asm/mpc8xxx_spi.h>
#include <asm-generic/gpio.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <asm/arch/soc.h>

enum {
	SPI_EV_NE = BIT(31 - 22),	/* Receiver Not Empty */
	SPI_EV_NF = BIT(31 - 23),	/* Transmitter Not Full */
};

enum {
	SPI_MODE_LOOP  = BIT(31 - 1),	/* Loopback mode */
	SPI_MODE_CI    = BIT(31 - 2),	/* Clock invert */
	SPI_MODE_CP    = BIT(31 - 3),	/* Clock phase */
	SPI_MODE_DIV16 = BIT(31 - 4),	/* Divide clock source by 16 */
	SPI_MODE_REV   = BIT(31 - 5),	/* Reverse mode - MSB first */
	SPI_MODE_MS    = BIT(31 - 6),	/* Always master */
	SPI_MODE_EN    = BIT(31 - 7),	/* Enable interface */
	SPI_MODE_OP    = BIT(31 - 17),	/* CPU Mode, QE otherwise */

	SPI_MODE_LEN_MASK = 0xf00000,
	SPI_MODE_LEN_SHIFT = 20,
	SPI_MODE_PM_SHIFT = 16,
	SPI_MODE_PM_MASK = 0xf0000,

	SPI_COM_LST = BIT(31 - 9),
};

struct mpc8xxx_priv {
	spi8xxx_t *spi;
	struct gpio_desc gpios[16];
	int cs_count;
	ulong clk_rate;
};

#define SPI_TIMEOUT	1000

static int mpc8xxx_spi_of_to_plat(struct udevice *dev)
{
	struct mpc8xxx_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret;

	priv->spi = dev_read_addr_ptr(dev);

	ret = gpio_request_list_by_name(dev, "gpios", priv->gpios,
					ARRAY_SIZE(priv->gpios), GPIOD_IS_OUT | GPIOD_ACTIVE_LOW);
	if (ret < 0)
		return -EINVAL;

	priv->cs_count = ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret) {
		dev_err(dev, "%s: clock not defined\n", __func__);
		return ret;
	}

	priv->clk_rate = clk_get_rate(&clk);
	if (!priv->clk_rate) {
		dev_err(dev, "%s: failed to get clock rate\n", __func__);
		return -EINVAL;
	}

	return 0;
}

static int mpc8xxx_spi_probe(struct udevice *dev)
{
	struct mpc8xxx_priv *priv = dev_get_priv(dev);
	spi8xxx_t *spi = priv->spi;

	/*
	 * SPI pins on the MPC83xx are not muxed, so all we do is initialize
	 * some registers
	 */
	out_be32(&priv->spi->mode, SPI_MODE_REV | SPI_MODE_MS);

	if (dev_get_driver_data(dev) == SOC_MPC832X)
		setbits_be32(&priv->spi->mode, SPI_MODE_OP);

	/* set len to 8 bits */
	setbits_be32(&spi->mode, (8 - 1) << SPI_MODE_LEN_SHIFT);

	setbits_be32(&spi->mode, SPI_MODE_EN);

	/* Clear all SPI events */
	setbits_be32(&priv->spi->event, 0xffffffff);
	/* Mask  all SPI interrupts */
	clrbits_be32(&priv->spi->mask, 0xffffffff);
	/* LST bit doesn't do anything, so disregard */
	out_be32(&priv->spi->com, 0);

	return 0;
}

static void mpc8xxx_spi_cs_activate(struct udevice *dev)
{
	struct mpc8xxx_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_plat *plat = dev_get_parent_plat(dev);

	dm_gpio_set_value(&priv->gpios[plat->cs], 1);
}

static void mpc8xxx_spi_cs_deactivate(struct udevice *dev)
{
	struct mpc8xxx_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_plat *plat = dev_get_parent_plat(dev);

	dm_gpio_set_value(&priv->gpios[plat->cs], 0);
}

static int mpc8xxx_spi_xfer(struct udevice *dev, uint bitlen,
			    const void *dout, void *din, ulong flags)
{
	struct udevice *bus = dev->parent;
	struct mpc8xxx_priv *priv = dev_get_priv(bus);
	spi8xxx_t *spi = priv->spi;
	struct dm_spi_slave_plat *plat = dev_get_parent_plat(dev);
	u32 tmpdin = 0, tmpdout = 0, n;
	const u8 *cout = dout;
	u8 *cin = din;
	ulong type = dev_get_driver_data(bus);

	debug("%s: slave %s:%u dout %08X din %08X bitlen %u\n", __func__,
	      bus->name, plat->cs, (uint)dout, (uint)din, bitlen);
	if (plat->cs >= priv->cs_count) {
		dev_err(dev, "chip select index %d too large (cs_count=%d)\n",
			plat->cs, priv->cs_count);
		return -EINVAL;
	}
	if (bitlen % 8) {
		printf("*** spi_xfer: bitlen must be multiple of 8\n");
		return -ENOTSUPP;
	}

	if (flags & SPI_XFER_BEGIN)
		mpc8xxx_spi_cs_activate(dev);

	/* Clear all SPI events */
	setbits_be32(&spi->event, 0xffffffff);
	n = bitlen / 8;

	/* Handle data in 8-bit chunks */
	while (n--) {
		ulong start;

		if (cout)
			tmpdout = *cout++;

		if (type == SOC_MPC832X)
			tmpdout <<= 24;

		/* Write the data out */
		out_be32(&spi->tx, tmpdout);

		debug("*** %s: ... %08x written\n", __func__, tmpdout);

		/*
		 * Wait for SPI transmit to get out
		 * or time out (1 second = 1000 ms)
		 * The NE event must be read and cleared first
		 */
		start = get_timer(0);
		do {
			u32 event = in_be32(&spi->event);
			bool have_ne = event & SPI_EV_NE;
			bool have_nf = event & SPI_EV_NF;

			if (!have_ne)
				continue;

			tmpdin = in_be32(&spi->rx);
			setbits_be32(&spi->event, SPI_EV_NE);

			if (type == SOC_MPC832X)
				tmpdin >>= 16;

			if (cin)
				*cin++ = tmpdin;

			/*
			 * Only bail when we've had both NE and NF events.
			 * This will cause timeouts on RO devices, so maybe
			 * in the future put an arbitrary delay after writing
			 * the device.  Arbitrary delays suck, though...
			 */
			if (have_nf)
				break;

			mdelay(1);
		} while (get_timer(start) < SPI_TIMEOUT);

		if (get_timer(start) >= SPI_TIMEOUT) {
			debug("*** %s: Time out during SPI transfer\n",
			      __func__);
			return -ETIMEDOUT;
		}

		debug("*** %s: transfer ended. Value=%08x\n", __func__, tmpdin);
	}

	if (flags & SPI_XFER_END)
		mpc8xxx_spi_cs_deactivate(dev);

	return 0;
}

static int mpc8xxx_spi_set_speed(struct udevice *dev, uint speed)
{
	struct mpc8xxx_priv *priv = dev_get_priv(dev);
	spi8xxx_t *spi = priv->spi;
	u32 bits, mask, div16, pm;
	u32 mode;
	ulong clk;

	clk = priv->clk_rate;
	if (clk / 64 > speed) {
		div16 = SPI_MODE_DIV16;
		clk /= 16;
	} else {
		div16 = 0;
	}
	pm = (clk - 1)/(4*speed) + 1;
	if (pm > 16) {
		dev_err(dev, "requested speed %u too small\n", speed);
		return -EINVAL;
	}
	pm--;

	bits = div16 | (pm << SPI_MODE_PM_SHIFT);
	mask = SPI_MODE_DIV16 | SPI_MODE_PM_MASK;
	mode = in_be32(&spi->mode);
	if ((mode & mask) != bits) {
		/* Must clear mode[EN] while changing speed. */
		mode &= ~(mask | SPI_MODE_EN);
		out_be32(&spi->mode, mode);
		mode |= bits;
		out_be32(&spi->mode, mode);
		mode |= SPI_MODE_EN;
		out_be32(&spi->mode, mode);
	}

	debug("requested speed %u, set speed to %lu/(%s4*%u) == %lu\n",
	      speed, priv->clk_rate, div16 ? "16*" : "", pm + 1,
	      clk/(4*(pm + 1)));

	return 0;
}

static int mpc8xxx_spi_set_mode(struct udevice *dev, uint mode)
{
	/* TODO(mario.six@gdsys.cc): Using SPI_CPHA (for clock phase) and
	 * SPI_CPOL (for clock polarity) should work
	 */
	return 0;
}

static const struct dm_spi_ops mpc8xxx_spi_ops = {
	.xfer		= mpc8xxx_spi_xfer,
	.set_speed	= mpc8xxx_spi_set_speed,
	.set_mode	= mpc8xxx_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id mpc8xxx_spi_ids[] = {
	{ .compatible = "fsl,spi" },
	{ .compatible = "fsl,mpc832x-spi", .data = SOC_MPC832X },
	{ }
};

U_BOOT_DRIVER(mpc8xxx_spi) = {
	.name	= "mpc8xxx_spi",
	.id	= UCLASS_SPI,
	.of_match = mpc8xxx_spi_ids,
	.ops	= &mpc8xxx_spi_ops,
	.of_to_plat = mpc8xxx_spi_of_to_plat,
	.probe	= mpc8xxx_spi_probe,
	.priv_auto	= sizeof(struct mpc8xxx_priv),
};
