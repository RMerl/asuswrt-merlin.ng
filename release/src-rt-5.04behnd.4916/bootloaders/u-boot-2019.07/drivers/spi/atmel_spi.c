// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2007 Atmel Corporation
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fdtdec.h>
#include <spi.h>
#include <malloc.h>
#include <wait_bit.h>

#include <asm/io.h>

#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#ifdef CONFIG_DM_SPI
#include <asm/arch/at91_spi.h>
#endif
#ifdef CONFIG_DM_GPIO
#include <asm/gpio.h>
#endif

#include "atmel_spi.h"

#ifndef CONFIG_DM_SPI

static int spi_has_wdrbt(struct atmel_spi_slave *slave)
{
	unsigned int ver;

	ver = spi_readl(slave, VERSION);

	return (ATMEL_SPI_VERSION_REV(ver) >= 0x210);
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
			unsigned int max_hz, unsigned int mode)
{
	struct atmel_spi_slave	*as;
	unsigned int		scbr;
	u32			csrx;
	void			*regs;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	switch (bus) {
	case 0:
		regs = (void *)ATMEL_BASE_SPI0;
		break;
#ifdef ATMEL_BASE_SPI1
	case 1:
		regs = (void *)ATMEL_BASE_SPI1;
		break;
#endif
#ifdef ATMEL_BASE_SPI2
	case 2:
		regs = (void *)ATMEL_BASE_SPI2;
		break;
#endif
#ifdef ATMEL_BASE_SPI3
	case 3:
		regs = (void *)ATMEL_BASE_SPI3;
		break;
#endif
	default:
		return NULL;
	}


	scbr = (get_spi_clk_rate(bus) + max_hz - 1) / max_hz;
	if (scbr > ATMEL_SPI_CSRx_SCBR_MAX)
		/* Too low max SCK rate */
		return NULL;
	if (scbr < 1)
		scbr = 1;

	csrx = ATMEL_SPI_CSRx_SCBR(scbr);
	csrx |= ATMEL_SPI_CSRx_BITS(ATMEL_SPI_BITS_8);
	if (!(mode & SPI_CPHA))
		csrx |= ATMEL_SPI_CSRx_NCPHA;
	if (mode & SPI_CPOL)
		csrx |= ATMEL_SPI_CSRx_CPOL;

	as = spi_alloc_slave(struct atmel_spi_slave, bus, cs);
	if (!as)
		return NULL;

	as->regs = regs;
	as->mr = ATMEL_SPI_MR_MSTR | ATMEL_SPI_MR_MODFDIS
			| ATMEL_SPI_MR_PCS(~(1 << cs) & 0xf);
	if (spi_has_wdrbt(as))
		as->mr |= ATMEL_SPI_MR_WDRBT;

	spi_writel(as, CSR(cs), csrx);

	return &as->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct atmel_spi_slave *as = to_atmel_spi(slave);

	free(as);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct atmel_spi_slave *as = to_atmel_spi(slave);

	/* Enable the SPI hardware */
	spi_writel(as, CR, ATMEL_SPI_CR_SPIEN);

	/*
	 * Select the slave. This should set SCK to the correct
	 * initial state, etc.
	 */
	spi_writel(as, MR, as->mr);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
	struct atmel_spi_slave *as = to_atmel_spi(slave);

	/* Disable the SPI hardware */
	spi_writel(as, CR, ATMEL_SPI_CR_SPIDIS);
}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct atmel_spi_slave *as = to_atmel_spi(slave);
	unsigned int	len_tx;
	unsigned int	len_rx;
	unsigned int	len;
	u32		status;
	const u8	*txp = dout;
	u8		*rxp = din;
	u8		value;

	if (bitlen == 0)
		/* Finish any previously submitted transfers */
		goto out;

	/*
	 * TODO: The controller can do non-multiple-of-8 bit
	 * transfers, but this driver currently doesn't support it.
	 *
	 * It's also not clear how such transfers are supposed to be
	 * represented as a stream of bytes...this is a limitation of
	 * the current SPI interface.
	 */
	if (bitlen % 8) {
		/* Errors always terminate an ongoing transfer */
		flags |= SPI_XFER_END;
		goto out;
	}

	len = bitlen / 8;

	/*
	 * The controller can do automatic CS control, but it is
	 * somewhat quirky, and it doesn't really buy us much anyway
	 * in the context of U-Boot.
	 */
	if (flags & SPI_XFER_BEGIN) {
		spi_cs_activate(slave);
		/*
		 * sometimes the RDR is not empty when we get here,
		 * in theory that should not happen, but it DOES happen.
		 * Read it here to be on the safe side.
		 * That also clears the OVRES flag. Required if the
		 * following loop exits due to OVRES!
		 */
		spi_readl(as, RDR);
	}

	for (len_tx = 0, len_rx = 0; len_rx < len; ) {
		status = spi_readl(as, SR);

		if (status & ATMEL_SPI_SR_OVRES)
			return -1;

		if (len_tx < len && (status & ATMEL_SPI_SR_TDRE)) {
			if (txp)
				value = *txp++;
			else
				value = 0;
			spi_writel(as, TDR, value);
			len_tx++;
		}
		if (status & ATMEL_SPI_SR_RDRF) {
			value = spi_readl(as, RDR);
			if (rxp)
				*rxp++ = value;
			len_rx++;
		}
	}

out:
	if (flags & SPI_XFER_END) {
		/*
		 * Wait until the transfer is completely done before
		 * we deactivate CS.
		 */
		do {
			status = spi_readl(as, SR);
		} while (!(status & ATMEL_SPI_SR_TXEMPTY));

		spi_cs_deactivate(slave);
	}

	return 0;
}

#else

#define MAX_CS_COUNT	4

struct atmel_spi_platdata {
	struct at91_spi *regs;
};

struct atmel_spi_priv {
	unsigned int freq;		/* Default frequency */
	unsigned int mode;
	ulong bus_clk_rate;
#ifdef CONFIG_DM_GPIO
	struct gpio_desc cs_gpios[MAX_CS_COUNT];
#endif
};

static int atmel_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct atmel_spi_platdata *bus_plat = dev_get_platdata(bus);
	struct atmel_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);
	struct at91_spi *reg_base = bus_plat->regs;
	u32 cs = slave_plat->cs;
	u32 freq = priv->freq;
	u32 scbr, csrx, mode;

	scbr = (priv->bus_clk_rate + freq - 1) / freq;
	if (scbr > ATMEL_SPI_CSRx_SCBR_MAX)
		return -EINVAL;

	if (scbr < 1)
		scbr = 1;

	csrx = ATMEL_SPI_CSRx_SCBR(scbr);
	csrx |= ATMEL_SPI_CSRx_BITS(ATMEL_SPI_BITS_8);

	if (!(priv->mode & SPI_CPHA))
		csrx |= ATMEL_SPI_CSRx_NCPHA;
	if (priv->mode & SPI_CPOL)
		csrx |= ATMEL_SPI_CSRx_CPOL;

	writel(csrx, &reg_base->csr[cs]);

	mode = ATMEL_SPI_MR_MSTR |
	       ATMEL_SPI_MR_MODFDIS |
	       ATMEL_SPI_MR_WDRBT |
	       ATMEL_SPI_MR_PCS(~(1 << cs));

	writel(mode, &reg_base->mr);

	writel(ATMEL_SPI_CR_SPIEN, &reg_base->cr);

	return 0;
}

static int atmel_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct atmel_spi_platdata *bus_plat = dev_get_platdata(bus);

	writel(ATMEL_SPI_CR_SPIDIS, &bus_plat->regs->cr);

	return 0;
}

static void atmel_spi_cs_activate(struct udevice *dev)
{
#ifdef CONFIG_DM_GPIO
	struct udevice *bus = dev_get_parent(dev);
	struct atmel_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);
	u32 cs = slave_plat->cs;

	if (!dm_gpio_is_valid(&priv->cs_gpios[cs]))
		return;

	dm_gpio_set_value(&priv->cs_gpios[cs], 0);
#endif
}

static void atmel_spi_cs_deactivate(struct udevice *dev)
{
#ifdef CONFIG_DM_GPIO
	struct udevice *bus = dev_get_parent(dev);
	struct atmel_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);
	u32 cs = slave_plat->cs;

	if (!dm_gpio_is_valid(&priv->cs_gpios[cs]))
		return;

	dm_gpio_set_value(&priv->cs_gpios[cs], 1);
#endif
}

static int atmel_spi_xfer(struct udevice *dev, unsigned int bitlen,
			  const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct atmel_spi_platdata *bus_plat = dev_get_platdata(bus);
	struct at91_spi *reg_base = bus_plat->regs;

	u32 len_tx, len_rx, len;
	u32 status;
	const u8 *txp = dout;
	u8 *rxp = din;
	u8 value;

	if (bitlen == 0)
		goto out;

	/*
	 * The controller can do non-multiple-of-8 bit
	 * transfers, but this driver currently doesn't support it.
	 *
	 * It's also not clear how such transfers are supposed to be
	 * represented as a stream of bytes...this is a limitation of
	 * the current SPI interface.
	 */
	if (bitlen % 8) {
		/* Errors always terminate an ongoing transfer */
		flags |= SPI_XFER_END;
		goto out;
	}

	len = bitlen / 8;

	/*
	 * The controller can do automatic CS control, but it is
	 * somewhat quirky, and it doesn't really buy us much anyway
	 * in the context of U-Boot.
	 */
	if (flags & SPI_XFER_BEGIN) {
		atmel_spi_cs_activate(dev);

		/*
		 * sometimes the RDR is not empty when we get here,
		 * in theory that should not happen, but it DOES happen.
		 * Read it here to be on the safe side.
		 * That also clears the OVRES flag. Required if the
		 * following loop exits due to OVRES!
		 */
		readl(&reg_base->rdr);
	}

	for (len_tx = 0, len_rx = 0; len_rx < len; ) {
		status = readl(&reg_base->sr);

		if (status & ATMEL_SPI_SR_OVRES)
			return -1;

		if ((len_tx < len) && (status & ATMEL_SPI_SR_TDRE)) {
			if (txp)
				value = *txp++;
			else
				value = 0;
			writel(value, &reg_base->tdr);
			len_tx++;
		}

		if (status & ATMEL_SPI_SR_RDRF) {
			value = readl(&reg_base->rdr);
			if (rxp)
				*rxp++ = value;
			len_rx++;
		}
	}

out:
	if (flags & SPI_XFER_END) {
		/*
		 * Wait until the transfer is completely done before
		 * we deactivate CS.
		 */
		wait_for_bit_le32(&reg_base->sr,
				  ATMEL_SPI_SR_TXEMPTY, true, 1000, false);

		atmel_spi_cs_deactivate(dev);
	}

	return 0;
}

static int atmel_spi_set_speed(struct udevice *bus, uint speed)
{
	struct atmel_spi_priv *priv = dev_get_priv(bus);

	priv->freq = speed;

	return 0;
}

static int atmel_spi_set_mode(struct udevice *bus, uint mode)
{
	struct atmel_spi_priv *priv = dev_get_priv(bus);

	priv->mode = mode;

	return 0;
}

static const struct dm_spi_ops atmel_spi_ops = {
	.claim_bus	= atmel_spi_claim_bus,
	.release_bus	= atmel_spi_release_bus,
	.xfer		= atmel_spi_xfer,
	.set_speed	= atmel_spi_set_speed,
	.set_mode	= atmel_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static int atmel_spi_enable_clk(struct udevice *bus)
{
	struct atmel_spi_priv *priv = dev_get_priv(bus);
	struct clk clk;
	ulong clk_rate;
	int ret;

	ret = clk_get_by_index(bus, 0, &clk);
	if (ret)
		return -EINVAL;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	clk_rate = clk_get_rate(&clk);
	if (!clk_rate)
		return -EINVAL;

	priv->bus_clk_rate = clk_rate;

	clk_free(&clk);

	return 0;
}

static int atmel_spi_probe(struct udevice *bus)
{
	struct atmel_spi_platdata *bus_plat = dev_get_platdata(bus);
	int ret;

	ret = atmel_spi_enable_clk(bus);
	if (ret)
		return ret;

	bus_plat->regs = (struct at91_spi *)devfdt_get_addr(bus);

#ifdef CONFIG_DM_GPIO
	struct atmel_spi_priv *priv = dev_get_priv(bus);
	int i;

	ret = gpio_request_list_by_name(bus, "cs-gpios", priv->cs_gpios,
					ARRAY_SIZE(priv->cs_gpios), 0);
	if (ret < 0) {
		pr_err("Can't get %s gpios! Error: %d", bus->name, ret);
		return ret;
	}

	for(i = 0; i < ARRAY_SIZE(priv->cs_gpios); i++) {
		if (!dm_gpio_is_valid(&priv->cs_gpios[i]))
			continue;

		dm_gpio_set_dir_flags(&priv->cs_gpios[i],
				      GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	}
#endif

	writel(ATMEL_SPI_CR_SWRST, &bus_plat->regs->cr);

	return 0;
}

static const struct udevice_id atmel_spi_ids[] = {
	{ .compatible = "atmel,at91rm9200-spi" },
	{ }
};

U_BOOT_DRIVER(atmel_spi) = {
	.name	= "atmel_spi",
	.id	= UCLASS_SPI,
	.of_match = atmel_spi_ids,
	.ops	= &atmel_spi_ops,
	.platdata_auto_alloc_size = sizeof(struct atmel_spi_platdata),
	.priv_auto_alloc_size = sizeof(struct atmel_spi_priv),
	.probe	= atmel_spi_probe,
};
#endif
