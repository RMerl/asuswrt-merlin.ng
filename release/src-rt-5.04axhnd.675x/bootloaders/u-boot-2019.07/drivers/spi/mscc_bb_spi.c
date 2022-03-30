// SPDX-License-Identifier: (GPL-2.0 OR MIT)
/*
 * Microsemi SoCs spi driver
 *
 * Copyright (c) 2018 Microsemi Corporation
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <spi.h>
#include <dm.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/delay.h>

struct mscc_bb_priv {
	void __iomem *regs;
	u32 deactivate_delay_us;
	bool cs_active;   /* State flag as to whether CS is asserted */
	int cs_num;
	u32 svalue;			/* Value to start transfer with */
	u32 clk1;			/* Clock value start */
	u32 clk2;			/* Clock value 2nd phase */
};

/* Delay 24 instructions for this particular application */
#define hold_time_delay() mscc_vcoreiii_nop_delay(3)

static int mscc_bb_spi_cs_activate(struct mscc_bb_priv *priv, int mode, int cs)
{
	if (!priv->cs_active) {
		int cpha = mode & SPI_CPHA;
		u32 cs_value;

		priv->cs_num = cs;

		if (cpha) {
			/* Initial clock starts SCK=1 */
			priv->clk1 = ICPU_SW_MODE_SW_SPI_SCK;
			priv->clk2 = 0;
		} else {
			/* Initial clock starts SCK=0 */
			priv->clk1 = 0;
			priv->clk2 = ICPU_SW_MODE_SW_SPI_SCK;
		}

		/* Enable bitbang, SCK_OE, SDO_OE */
		priv->svalue = (ICPU_SW_MODE_SW_PIN_CTRL_MODE | /* Bitbang */
				ICPU_SW_MODE_SW_SPI_SCK_OE    | /* SCK_OE */
				ICPU_SW_MODE_SW_SPI_SDO_OE);   /* SDO OE */

		/* Add CS */
		if (cs >= 0) {
			cs_value =
				ICPU_SW_MODE_SW_SPI_CS_OE(BIT(cs)) |
				ICPU_SW_MODE_SW_SPI_CS(BIT(cs));
		} else {
			cs_value = 0;
		}

		priv->svalue |= cs_value;

		/* Enable the CS in HW, Initial clock value */
		writel(priv->svalue | priv->clk2, priv->regs);

		priv->cs_active = true;
		debug("Activated CS%d\n", priv->cs_num);
	}

	return 0;
}

static int mscc_bb_spi_cs_deactivate(struct mscc_bb_priv *priv, int deact_delay)
{
	if (priv->cs_active) {
		/* Keep driving the CLK to its current value while
		 * actively deselecting CS.
		 */
		u32 value = readl(priv->regs);

		value &= ~ICPU_SW_MODE_SW_SPI_CS_M;
		writel(value, priv->regs);
		hold_time_delay();

		/* Stop driving the clock, but keep CS with nCS == 1 */
		value &= ~ICPU_SW_MODE_SW_SPI_SCK_OE;
		writel(value, priv->regs);

		/* Deselect hold time delay */
		if (deact_delay)
			udelay(deact_delay);

		/* Drop everything */
		writel(0, priv->regs);

		priv->cs_active = false;
		debug("Deactivated CS%d\n", priv->cs_num);
	}

	return 0;
}

int mscc_bb_spi_claim_bus(struct udevice *dev)
{
	return 0;
}

int mscc_bb_spi_release_bus(struct udevice *dev)
{
	return 0;
}

int mscc_bb_spi_xfer(struct udevice *dev, unsigned int bitlen,
		     const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct dm_spi_slave_platdata *plat = dev_get_parent_platdata(dev);
	struct mscc_bb_priv *priv = dev_get_priv(bus);
	u32             i, count;
	const u8	*txd = dout;
	u8		*rxd = din;

	debug("spi_xfer: slave %s:%s cs%d mode %d, dout %p din %p bitlen %u\n",
	      dev->parent->name, dev->name, plat->cs,  plat->mode, dout,
	      din, bitlen);

	if (flags & SPI_XFER_BEGIN)
		mscc_bb_spi_cs_activate(priv, plat->mode, plat->cs);

	count = bitlen / 8;
	for (i = 0; i < count; i++) {
		u32 rx = 0, mask = 0x80, value;

		while (mask) {
			/* Initial condition: CLK is low. */
			value = priv->svalue;
			if (txd && txd[i] & mask)
				value |= ICPU_SW_MODE_SW_SPI_SDO;

			/* Drive data while taking CLK low. The device
			 * we're accessing will sample on the
			 * following rising edge and will output data
			 * on this edge for us to be sampled at the
			 * end of this loop.
			 */
			writel(value | priv->clk1, priv->regs);

			/* Wait for t_setup. All devices do have a
			 * setup-time, so we always insert some delay
			 * here. Some devices have a very long
			 * setup-time, which can be adjusted by the
			 * user through vcoreiii_device->delay.
			 */
			hold_time_delay();

			/* Drive the clock high. */
			writel(value | priv->clk2, priv->regs);

			/* Wait for t_hold. See comment about t_setup
			 * above.
			 */
			hold_time_delay();

			/* We sample as close to the next falling edge
			 * as possible.
			 */
			value = readl(priv->regs);
			if (value & ICPU_SW_MODE_SW_SPI_SDI)
				rx |= mask;
			mask >>= 1;
		}
		if (rxd) {
			debug("Read 0x%02x\n", rx);
			rxd[i] = (u8)rx;
		}
		debug("spi_xfer: byte %d/%d\n", i + 1, count);
	}

	debug("spi_xfer: done\n");

	if (flags & SPI_XFER_END)
		mscc_bb_spi_cs_deactivate(priv, priv->deactivate_delay_us);

	return 0;
}

int mscc_bb_spi_set_speed(struct udevice *dev, unsigned int speed)
{
	/* Accept any speed */
	return 0;
}

int mscc_bb_spi_set_mode(struct udevice *dev, unsigned int mode)
{
	return 0;
}

static const struct dm_spi_ops mscc_bb_ops = {
	.claim_bus	= mscc_bb_spi_claim_bus,
	.release_bus	= mscc_bb_spi_release_bus,
	.xfer		= mscc_bb_spi_xfer,
	.set_speed	= mscc_bb_spi_set_speed,
	.set_mode	= mscc_bb_spi_set_mode,
};

static const struct udevice_id mscc_bb_ids[] = {
	{ .compatible = "mscc,luton-bb-spi" },
	{ }
};

static int mscc_bb_spi_probe(struct udevice *bus)
{
	struct mscc_bb_priv *priv = dev_get_priv(bus);

	debug("%s: loaded, priv %p\n", __func__, priv);

	priv->regs = (void __iomem *)dev_read_addr(bus);

	priv->deactivate_delay_us =
		dev_read_u32_default(bus, "spi-deactivate-delay", 0);

	priv->cs_active = false;

	return 0;
}

U_BOOT_DRIVER(mscc_bb) = {
	.name	= "mscc_bb",
	.id	= UCLASS_SPI,
	.of_match = mscc_bb_ids,
	.ops	= &mscc_bb_ops,
	.priv_auto_alloc_size = sizeof(struct mscc_bb_priv),
	.probe	= mscc_bb_spi_probe,
};
