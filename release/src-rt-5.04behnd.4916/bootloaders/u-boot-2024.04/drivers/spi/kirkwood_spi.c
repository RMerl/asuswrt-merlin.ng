// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * Derived from drivers/spi/mpc8xxx_spi.c
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#ifdef CONFIG_ARCH_KIRKWOOD
#include <asm/arch/mpp.h>
#endif
#include <asm/arch-mvebu/spi.h>

struct mvebu_spi_dev {
	bool			is_errata_50mhz_ac;
};

struct mvebu_spi_plat {
	struct kwspi_registers *spireg;
	bool is_errata_50mhz_ac;
};

struct mvebu_spi_priv {
	struct kwspi_registers *spireg;
};

static void _spi_cs_activate(struct kwspi_registers *reg)
{
	setbits_le32(&reg->ctrl, KWSPI_CSN_ACT);
}

static void _spi_cs_deactivate(struct kwspi_registers *reg)
{
	clrbits_le32(&reg->ctrl, KWSPI_CSN_ACT);
}

static int _spi_xfer(struct kwspi_registers *reg, unsigned int bitlen,
		     const void *dout, void *din, unsigned long flags)
{
	unsigned int tmpdout, tmpdin;
	int tm, isread = 0;

	debug("spi_xfer: dout %p din %p bitlen %u\n", dout, din, bitlen);

	if (flags & SPI_XFER_BEGIN)
		_spi_cs_activate(reg);

	/*
	 * handle data in 8-bit chunks
	 * TBD: 2byte xfer mode to be enabled
	 */
	clrsetbits_le32(&reg->cfg, KWSPI_XFERLEN_MASK, KWSPI_XFERLEN_1BYTE);

	while (bitlen > 4) {
		debug("loopstart bitlen %d\n", bitlen);
		tmpdout = 0;

		/* Shift data so it's msb-justified */
		if (dout)
			tmpdout = *(u32 *)dout & 0xff;

		clrbits_le32(&reg->irq_cause, KWSPI_SMEMRDIRQ);
		writel(tmpdout, &reg->dout);	/* Write the data out */
		debug("*** spi_xfer: ... %08x written, bitlen %d\n",
		      tmpdout, bitlen);

		/*
		 * Wait for SPI transmit to get out
		 * or time out (1 second = 1000 ms)
		 * The NE event must be read and cleared first
		 */
		for (tm = 0, isread = 0; tm < KWSPI_TIMEOUT; ++tm) {
			if (readl(&reg->irq_cause) & KWSPI_SMEMRDIRQ) {
				isread = 1;
				tmpdin = readl(&reg->din);
				debug("spi_xfer: din %p..%08x read\n",
				      din, tmpdin);

				if (din) {
					*((u8 *)din) = (u8)tmpdin;
					din += 1;
				}
				if (dout)
					dout += 1;
				bitlen -= 8;
			}
			if (isread)
				break;
		}
		if (tm >= KWSPI_TIMEOUT)
			printf("*** spi_xfer: Time out during SPI transfer\n");

		debug("loopend bitlen %d\n", bitlen);
	}

	if (flags & SPI_XFER_END)
		_spi_cs_deactivate(reg);

	return 0;
}

static int mvebu_spi_set_speed(struct udevice *bus, uint hz)
{
	struct mvebu_spi_plat *plat = dev_get_plat(bus);
	struct dm_spi_bus *spi = dev_get_uclass_priv(bus);
	struct kwspi_registers *reg = plat->spireg;
	u32 data, divider;
	unsigned int spr, sppr;

	if (spi->max_hz && (hz > spi->max_hz)) {
		debug("%s: limit speed to the max_hz of the bus %d\n",
		      __func__, spi->max_hz);
		hz = spi->max_hz;
	}

	/*
	 * Calculate spi clock prescaller using max_hz.
	 * SPPR is SPI Baud Rate Pre-selection, it holds bits 5 and 7:6 in
	 * SPI Interface Configuration Register;
	 * SPR is SPI Baud Rate Selection, it holds bits 3:0 in SPI Interface
	 * Configuration Register.
	 * The SPR together with the SPPR define the SPI CLK frequency as
	 * follows:
	 * SPI actual frequency = core_clk / (SPR * (2 ^ SPPR))
	 */
	divider = DIV_ROUND_UP(CFG_SYS_TCLK, hz);
	if (divider < 16) {
		/* This is the easy case, divider is less than 16 */
		spr = divider;
		sppr = 0;

	} else {
		unsigned int two_pow_sppr;
		/*
		 * Find the highest bit set in divider. This and the
		 * three next bits define SPR (apart from rounding).
		 * SPPR is then the number of zero bits that must be
		 * appended:
		 */
		sppr = fls(divider) - 4;

		/*
		 * As SPR only has 4 bits, we have to round divider up
		 * to the next multiple of 2 ** sppr.
		 */
		two_pow_sppr = 1 << sppr;
		divider = (divider + two_pow_sppr - 1) & -two_pow_sppr;

		/*
		 * recalculate sppr as rounding up divider might have
		 * increased it enough to change the position of the
		 * highest set bit. In this case the bit that now
		 * doesn't make it into SPR is 0, so there is no need to
		 * round again.
		 */
		sppr = fls(divider) - 4;
		spr = divider >> sppr;

		/*
		 * Now do range checking. SPR is constructed to have a
		 * width of 4 bits, so this is fine for sure. So we
		 * still need to check for sppr to fit into 3 bits:
		 */
		if (sppr > 7)
			return -EINVAL;
	}

	data = ((sppr & 0x6) << 5) | ((sppr & 0x1) << 4) | spr;

	/* program spi clock prescaler using max_hz */
	writel(KWSPI_ADRLEN_3BYTE | data, &reg->cfg);
	debug("data = 0x%08x\n", data);

	return 0;
}

static void mvebu_spi_50mhz_ac_timing_erratum(struct udevice *bus, uint mode)
{
	struct mvebu_spi_plat *plat = dev_get_plat(bus);
	struct kwspi_registers *reg = plat->spireg;
	u32 data;

	/*
	 * Erratum description: (Erratum NO. FE-9144572) The device
	 * SPI interface supports frequencies of up to 50 MHz.
	 * However, due to this erratum, when the device core clock is
	 * 250 MHz and the SPI interfaces is configured for 50MHz SPI
	 * clock and CPOL=CPHA=1 there might occur data corruption on
	 * reads from the SPI device.
	 * Erratum Workaround:
	 * Work in one of the following configurations:
	 * 1. Set CPOL=CPHA=0 in "SPI Interface Configuration
	 * Register".
	 * 2. Set TMISO_SAMPLE value to 0x2 in "SPI Timing Parameters 1
	 * Register" before setting the interface.
	 */
	data = readl(&reg->timing1);
	data &= ~KW_SPI_TMISO_SAMPLE_MASK;

	if (CFG_SYS_TCLK == 250000000 &&
	    mode & SPI_CPOL &&
	    mode & SPI_CPHA)
		data |= KW_SPI_TMISO_SAMPLE_2;
	else
		data |= KW_SPI_TMISO_SAMPLE_1;

	writel(data, &reg->timing1);
}

static int mvebu_spi_set_mode(struct udevice *bus, uint mode)
{
	struct mvebu_spi_plat *plat = dev_get_plat(bus);
	struct kwspi_registers *reg = plat->spireg;
	u32 data = readl(&reg->cfg);

	data &= ~(KWSPI_CPHA | KWSPI_CPOL | KWSPI_RXLSBF | KWSPI_TXLSBF);

	if (mode & SPI_CPHA)
		data |= KWSPI_CPHA;
	if (mode & SPI_CPOL)
		data |= KWSPI_CPOL;
	if (mode & SPI_LSB_FIRST)
		data |= (KWSPI_RXLSBF | KWSPI_TXLSBF);

	writel(data, &reg->cfg);

	if (plat->is_errata_50mhz_ac)
		mvebu_spi_50mhz_ac_timing_erratum(bus, mode);

	return 0;
}

static int mvebu_spi_xfer(struct udevice *dev, unsigned int bitlen,
			  const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct mvebu_spi_plat *plat = dev_get_plat(bus);

	return _spi_xfer(plat->spireg, bitlen, dout, din, flags);
}

__attribute__((weak)) int mvebu_board_spi_claim_bus(struct udevice *dev)
{
	return 0;
}

static int mvebu_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct mvebu_spi_plat *plat = dev_get_plat(bus);

	/* Configure the chip-select in the CTRL register */
	clrsetbits_le32(&plat->spireg->ctrl,
			KWSPI_CS_MASK << KWSPI_CS_SHIFT,
			spi_chip_select(dev) << KWSPI_CS_SHIFT);

	return mvebu_board_spi_claim_bus(dev);
}

__attribute__((weak)) int mvebu_board_spi_release_bus(struct udevice *dev)
{
	return 0;
}

static int mvebu_spi_release_bus(struct udevice *dev)
{
	return mvebu_board_spi_release_bus(dev);
}

static int mvebu_spi_probe(struct udevice *bus)
{
	struct mvebu_spi_plat *plat = dev_get_plat(bus);
	struct kwspi_registers *reg = plat->spireg;

	writel(KWSPI_SMEMRDY, &reg->ctrl);
	writel(KWSPI_SMEMRDIRQ, &reg->irq_cause);
	writel(KWSPI_IRQMASK, &reg->irq_mask);

	return 0;
}

static int mvebu_spi_of_to_plat(struct udevice *bus)
{
	struct mvebu_spi_plat *plat = dev_get_plat(bus);
	const struct mvebu_spi_dev *drvdata =
		(struct mvebu_spi_dev *)dev_get_driver_data(bus);

	plat->spireg = dev_read_addr_ptr(bus);
	plat->is_errata_50mhz_ac = drvdata->is_errata_50mhz_ac;

	return 0;
}

static const struct dm_spi_ops mvebu_spi_ops = {
	.claim_bus	= mvebu_spi_claim_bus,
	.release_bus	= mvebu_spi_release_bus,
	.xfer		= mvebu_spi_xfer,
	.set_speed	= mvebu_spi_set_speed,
	.set_mode	= mvebu_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct mvebu_spi_dev armada_spi_dev_data = {
	.is_errata_50mhz_ac = false,
};

static const struct mvebu_spi_dev armada_xp_spi_dev_data = {
	.is_errata_50mhz_ac = false,
};

static const struct mvebu_spi_dev armada_375_spi_dev_data = {
	.is_errata_50mhz_ac = false,
};

static const struct mvebu_spi_dev armada_380_spi_dev_data = {
	.is_errata_50mhz_ac = true,
};

static const struct udevice_id mvebu_spi_ids[] = {
	{
		.compatible = "marvell,orion-spi",
		.data = (ulong)&armada_spi_dev_data,
	},
	{
		.compatible = "marvell,armada-375-spi",
		.data = (ulong)&armada_375_spi_dev_data
	},
	{
		.compatible = "marvell,armada-380-spi",
		.data = (ulong)&armada_380_spi_dev_data
	},
	{
		.compatible = "marvell,armada-xp-spi",
		.data = (ulong)&armada_xp_spi_dev_data
	},
	{ }
};

U_BOOT_DRIVER(mvebu_spi) = {
	.name = "mvebu_spi",
	.id = UCLASS_SPI,
	.of_match = mvebu_spi_ids,
	.ops = &mvebu_spi_ops,
	.of_to_plat = mvebu_spi_of_to_plat,
	.plat_auto	= sizeof(struct mvebu_spi_plat),
	.priv_auto	= sizeof(struct mvebu_spi_priv),
	.probe = mvebu_spi_probe,
};
