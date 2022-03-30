// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Marvell International Ltd.
 *
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <spi.h>
#include <clk.h>
#include <wait_bit.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define MVEBU_SPI_A3700_XFER_RDY		BIT(1)
#define MVEBU_SPI_A3700_FIFO_FLUSH		BIT(9)
#define MVEBU_SPI_A3700_BYTE_LEN		BIT(5)
#define MVEBU_SPI_A3700_CLK_PHA			BIT(6)
#define MVEBU_SPI_A3700_CLK_POL			BIT(7)
#define MVEBU_SPI_A3700_FIFO_EN			BIT(17)
#define MVEBU_SPI_A3700_SPI_EN_0		BIT(16)
#define MVEBU_SPI_A3700_CLK_PRESCALE_MASK	0x1f


/* SPI registers */
struct spi_reg {
	u32 ctrl;	/* 0x10600 */
	u32 cfg;	/* 0x10604 */
	u32 dout;	/* 0x10608 */
	u32 din;	/* 0x1060c */
};

struct mvebu_spi_platdata {
	struct spi_reg *spireg;
	struct clk clk;
};

static void spi_cs_activate(struct spi_reg *reg, int cs)
{
	setbits_le32(&reg->ctrl, MVEBU_SPI_A3700_SPI_EN_0 << cs);
}

static void spi_cs_deactivate(struct spi_reg *reg, int cs)
{
	clrbits_le32(&reg->ctrl, MVEBU_SPI_A3700_SPI_EN_0 << cs);
}

/**
 * spi_legacy_shift_byte() - triggers the real SPI transfer
 * @bytelen:	Indicate how many bytes to transfer.
 * @dout:	Buffer address of what to send.
 * @din:	Buffer address of where to receive.
 *
 * This function triggers the real SPI transfer in legacy mode. It
 * will shift out char buffer from @dout, and shift in char buffer to
 * @din, if necessary.
 *
 * This function assumes that only one byte is shifted at one time.
 * However, it is not its responisbility to set the transfer type to
 * one-byte. Also, it does not guarantee that it will work if transfer
 * type becomes two-byte. See spi_set_legacy() for details.
 *
 * In legacy mode, simply write to the SPI_DOUT register will trigger
 * the transfer.
 *
 * If @dout == NULL, which means no actual data needs to be sent out,
 * then the function will shift out 0x00 in order to shift in data.
 * The XFER_RDY flag is checked every time before accessing SPI_DOUT
 * and SPI_DIN register.
 *
 * The number of transfers to be triggerred is decided by @bytelen.
 *
 * Return:	0 - cool
 *		-ETIMEDOUT - XFER_RDY flag timeout
 */
static int spi_legacy_shift_byte(struct spi_reg *reg, unsigned int bytelen,
				 const void *dout, void *din)
{
	const u8 *dout_8;
	u8 *din_8;
	int ret;

	/* Use 0x00 as dummy dout */
	const u8 dummy_dout = 0x0;
	u32 pending_dout = 0x0;

	/* dout_8: pointer of current dout */
	dout_8 = dout;
	/* din_8: pointer of current din */
	din_8 = din;

	while (bytelen) {
		ret = wait_for_bit_le32(&reg->ctrl,
					MVEBU_SPI_A3700_XFER_RDY,
					true,100, false);
		if (ret)
			return ret;

		if (dout)
			pending_dout = (u32)*dout_8;
		else
			pending_dout = (u32)dummy_dout;

		/* Trigger the xfer */
		writel(pending_dout, &reg->dout);

		if (din) {
			ret = wait_for_bit_le32(&reg->ctrl,
						MVEBU_SPI_A3700_XFER_RDY,
						true, 100, false);
			if (ret)
				return ret;

			/* Read what is transferred in */
			*din_8 = (u8)readl(&reg->din);
		}

		/* Don't increment the current pointer if NULL */
		if (dout)
			dout_8++;
		if (din)
			din_8++;

		bytelen--;
	}

	return 0;
}

static int mvebu_spi_xfer(struct udevice *dev, unsigned int bitlen,
			  const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct mvebu_spi_platdata *plat = dev_get_platdata(bus);
	struct spi_reg *reg = plat->spireg;
	unsigned int bytelen;
	int ret;

	bytelen = bitlen / 8;

	if (dout && din)
		debug("This is a duplex transfer.\n");

	/* Activate CS */
	if (flags & SPI_XFER_BEGIN) {
		debug("SPI: activate cs.\n");
		spi_cs_activate(reg, spi_chip_select(dev));
	}

	/* Send and/or receive */
	if (dout || din) {
		ret = spi_legacy_shift_byte(reg, bytelen, dout, din);
		if (ret)
			return ret;
	}

	/* Deactivate CS */
	if (flags & SPI_XFER_END) {
		ret = wait_for_bit_le32(&reg->ctrl,
					MVEBU_SPI_A3700_XFER_RDY,
					true, 100, false);
		if (ret)
			return ret;

		debug("SPI: deactivate cs.\n");
		spi_cs_deactivate(reg, spi_chip_select(dev));
	}

	return 0;
}

static int mvebu_spi_set_speed(struct udevice *bus, uint hz)
{
	struct mvebu_spi_platdata *plat = dev_get_platdata(bus);
	struct spi_reg *reg = plat->spireg;
	u32 data, prescale;

	data = readl(&reg->cfg);

	prescale = DIV_ROUND_UP(clk_get_rate(&plat->clk), hz);
	if (prescale > 0x1f)
		prescale = 0x1f;
	else if (prescale > 0xf)
		prescale = 0x10 + (prescale + 1) / 2;

	data &= ~MVEBU_SPI_A3700_CLK_PRESCALE_MASK;
	data |= prescale & MVEBU_SPI_A3700_CLK_PRESCALE_MASK;

	writel(data, &reg->cfg);

	return 0;
}

static int mvebu_spi_set_mode(struct udevice *bus, uint mode)
{
	struct mvebu_spi_platdata *plat = dev_get_platdata(bus);
	struct spi_reg *reg = plat->spireg;

	/*
	 * Set SPI polarity
	 * 0: Serial interface clock is low when inactive
	 * 1: Serial interface clock is high when inactive
	 */
	if (mode & SPI_CPOL)
		setbits_le32(&reg->cfg, MVEBU_SPI_A3700_CLK_POL);
	else
		clrbits_le32(&reg->cfg, MVEBU_SPI_A3700_CLK_POL);
	if (mode & SPI_CPHA)
		setbits_le32(&reg->cfg, MVEBU_SPI_A3700_CLK_PHA);
	else
		clrbits_le32(&reg->cfg, MVEBU_SPI_A3700_CLK_PHA);

	return 0;
}

static int mvebu_spi_probe(struct udevice *bus)
{
	struct mvebu_spi_platdata *plat = dev_get_platdata(bus);
	struct spi_reg *reg = plat->spireg;
	u32 data;
	int ret;

	/*
	 * Settings SPI controller to be working in legacy mode, which
	 * means use only DO pin (I/O 1) for Data Out, and DI pin (I/O 0)
	 * for Data In.
	 */

	/* Flush read/write FIFO */
	data = readl(&reg->cfg);
	writel(data | MVEBU_SPI_A3700_FIFO_FLUSH, &reg->cfg);
	ret = wait_for_bit_le32(&reg->cfg, MVEBU_SPI_A3700_FIFO_FLUSH,
				false, 1000, false);
	if (ret)
		return ret;

	/* Disable FIFO mode */
	data &= ~MVEBU_SPI_A3700_FIFO_EN;

	/* Always shift 1 byte at a time */
	data &= ~MVEBU_SPI_A3700_BYTE_LEN;

	writel(data, &reg->cfg);

	return 0;
}

static int mvebu_spi_ofdata_to_platdata(struct udevice *bus)
{
	struct mvebu_spi_platdata *plat = dev_get_platdata(bus);
	int ret;

	plat->spireg = (struct spi_reg *)devfdt_get_addr(bus);

	ret = clk_get_by_index(bus, 0, &plat->clk);
	if (ret) {
		dev_err(bus, "cannot get clock\n");
		return ret;
	}

	return 0;
}

static int mvebu_spi_remove(struct udevice *bus)
{
	struct mvebu_spi_platdata *plat = dev_get_platdata(bus);

	clk_free(&plat->clk);

	return 0;
}

static const struct dm_spi_ops mvebu_spi_ops = {
	.xfer		= mvebu_spi_xfer,
	.set_speed	= mvebu_spi_set_speed,
	.set_mode	= mvebu_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id mvebu_spi_ids[] = {
	{ .compatible = "marvell,armada-3700-spi" },
	{ }
};

U_BOOT_DRIVER(mvebu_spi) = {
	.name = "mvebu_spi",
	.id = UCLASS_SPI,
	.of_match = mvebu_spi_ids,
	.ops = &mvebu_spi_ops,
	.ofdata_to_platdata = mvebu_spi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct mvebu_spi_platdata),
	.probe = mvebu_spi_probe,
	.remove = mvebu_spi_remove,
};
