// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2006 Ben Warren, Qstreams Networks Inc.
 * With help from the common/soft_spi and arch/powerpc/cpu/mpc8260 drivers
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <spi.h>
#include <asm/mpc8xxx_spi.h>
#include <asm-generic/gpio.h>

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

	SPI_MODE_LEN_MASK = 0xf00000,
	SPI_MODE_PM_MASK = 0xf0000,

	SPI_COM_LST = BIT(31 - 9),
};

struct mpc8xxx_priv {
	spi8xxx_t *spi;
	struct gpio_desc gpios[16];
	int max_cs;
};

static inline u32 to_prescale_mod(u32 val)
{
	return (min(val, (u32)15) << 16);
}

static void set_char_len(spi8xxx_t *spi, u32 val)
{
	clrsetbits_be32(&spi->mode, SPI_MODE_LEN_MASK, (val << 20));
}

#define SPI_TIMEOUT	1000

static int __spi_set_speed(spi8xxx_t *spi, uint speed)
{
	/* TODO(mario.six@gdsys.cc): This only ever sets one fixed speed */

	/* Use SYSCLK / 8 (16.67MHz typ.) */
	clrsetbits_be32(&spi->mode, SPI_MODE_PM_MASK, to_prescale_mod(1));

	return 0;
}

static int mpc8xxx_spi_ofdata_to_platdata(struct udevice *dev)
{
	struct mpc8xxx_priv *priv = dev_get_priv(dev);
	int ret;

	priv->spi = (spi8xxx_t *)dev_read_addr(dev);

	/* TODO(mario.six@gdsys.cc): Read clock and save the value */

	ret = gpio_request_list_by_name(dev, "gpios", priv->gpios,
					ARRAY_SIZE(priv->gpios), GPIOD_IS_OUT | GPIOD_ACTIVE_LOW);
	if (ret < 0)
		return -EINVAL;

	priv->max_cs = ret;

	return 0;
}

static int mpc8xxx_spi_probe(struct udevice *dev)
{
	struct mpc8xxx_priv *priv = dev_get_priv(dev);

	/*
	 * SPI pins on the MPC83xx are not muxed, so all we do is initialize
	 * some registers
	 */
	out_be32(&priv->spi->mode, SPI_MODE_REV | SPI_MODE_MS | SPI_MODE_EN);

	__spi_set_speed(priv->spi, 16666667);

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
	struct dm_spi_slave_platdata *platdata = dev_get_parent_platdata(dev);

	dm_gpio_set_dir_flags(&priv->gpios[platdata->cs], GPIOD_IS_OUT);
	dm_gpio_set_value(&priv->gpios[platdata->cs], 0);
}

static void mpc8xxx_spi_cs_deactivate(struct udevice *dev)
{
	struct mpc8xxx_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_platdata *platdata = dev_get_parent_platdata(dev);

	dm_gpio_set_dir_flags(&priv->gpios[platdata->cs], GPIOD_IS_OUT);
	dm_gpio_set_value(&priv->gpios[platdata->cs], 1);
}

static int mpc8xxx_spi_xfer(struct udevice *dev, uint bitlen,
			    const void *dout, void *din, ulong flags)
{
	struct udevice *bus = dev->parent;
	struct mpc8xxx_priv *priv = dev_get_priv(bus);
	spi8xxx_t *spi = priv->spi;
	struct dm_spi_slave_platdata *platdata = dev_get_parent_platdata(dev);
	u32 tmpdin = 0;
	int num_blks = DIV_ROUND_UP(bitlen, 32);

	debug("%s: slave %s:%u dout %08X din %08X bitlen %u\n", __func__,
	      bus->name, platdata->cs, *(uint *)dout, *(uint *)din, bitlen);

	if (flags & SPI_XFER_BEGIN)
		mpc8xxx_spi_cs_activate(dev);

	/* Clear all SPI events */
	setbits_be32(&spi->event, 0xffffffff);

	/* Handle data in 32-bit chunks */
	while (num_blks--) {
		u32 tmpdout = 0;
		uchar xfer_bitlen = (bitlen >= 32 ? 32 : bitlen);
		ulong start;

		clrbits_be32(&spi->mode, SPI_MODE_EN);

		/* Set up length for this transfer */

		if (bitlen <= 4) /* 4 bits or less */
			set_char_len(spi, 3);
		else if (bitlen <= 16) /* at most 16 bits */
			set_char_len(spi, bitlen - 1);
		else /* more than 16 bits -> full 32 bit transfer */
			set_char_len(spi, 0);

		setbits_be32(&spi->mode, SPI_MODE_EN);

		/* Shift data so it's msb-justified */
		tmpdout = *(u32 *)dout >> (32 - xfer_bitlen);

		if (bitlen > 32) {
			/* Set up the next iteration if sending > 32 bits */
			bitlen -= 32;
			dout += 4;
		}

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

			*(u32 *)din = (tmpdin << (32 - xfer_bitlen));
			if (xfer_bitlen == 32) {
				/* Advance output buffer by 32 bits */
				din += 4;
			}

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

	return __spi_set_speed(priv->spi, speed);
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
	{ }
};

U_BOOT_DRIVER(mpc8xxx_spi) = {
	.name	= "mpc8xxx_spi",
	.id	= UCLASS_SPI,
	.of_match = mpc8xxx_spi_ids,
	.ops	= &mpc8xxx_spi_ops,
	.ofdata_to_platdata = mpc8xxx_spi_ofdata_to_platdata,
	.probe	= mpc8xxx_spi_probe,
	.priv_auto_alloc_size = sizeof(struct mpc8xxx_priv),
};
