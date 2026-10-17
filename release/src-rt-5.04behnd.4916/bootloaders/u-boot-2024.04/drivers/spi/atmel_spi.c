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
#include <asm/arch/at91_spi.h>
#if CONFIG_IS_ENABLED(DM_GPIO)
#include <asm/gpio.h>
#endif
#include <linux/bitops.h>
#include <linux/printk.h>

/*
 * Register definitions for the Atmel AT32/AT91 SPI Controller
 */
/* Register offsets */
#define ATMEL_SPI_CR			0x0000
#define ATMEL_SPI_MR			0x0004
#define ATMEL_SPI_RDR			0x0008
#define ATMEL_SPI_TDR			0x000c
#define ATMEL_SPI_SR			0x0010
#define ATMEL_SPI_IER			0x0014
#define ATMEL_SPI_IDR			0x0018
#define ATMEL_SPI_IMR			0x001c
#define ATMEL_SPI_CSR(x)		(0x0030 + 4 * (x))
#define ATMEL_SPI_VERSION		0x00fc

/* Bits in CR */
#define ATMEL_SPI_CR_SPIEN		BIT(0)
#define ATMEL_SPI_CR_SPIDIS		BIT(1)
#define ATMEL_SPI_CR_SWRST		BIT(7)
#define ATMEL_SPI_CR_LASTXFER		BIT(24)

/* Bits in MR */
#define ATMEL_SPI_MR_MSTR		BIT(0)
#define ATMEL_SPI_MR_PS			BIT(1)
#define ATMEL_SPI_MR_PCSDEC		BIT(2)
#define ATMEL_SPI_MR_FDIV		BIT(3)
#define ATMEL_SPI_MR_MODFDIS		BIT(4)
#define ATMEL_SPI_MR_WDRBT		BIT(5)
#define ATMEL_SPI_MR_LLB		BIT(7)
#define ATMEL_SPI_MR_PCS(x)		(((x) & 15) << 16)
#define ATMEL_SPI_MR_DLYBCS(x)		((x) << 24)

/* Bits in RDR */
#define ATMEL_SPI_RDR_RD(x)		(x)
#define ATMEL_SPI_RDR_PCS(x)		((x) << 16)

/* Bits in TDR */
#define ATMEL_SPI_TDR_TD(x)		(x)
#define ATMEL_SPI_TDR_PCS(x)		((x) << 16)
#define ATMEL_SPI_TDR_LASTXFER		BIT(24)

/* Bits in SR/IER/IDR/IMR */
#define ATMEL_SPI_SR_RDRF		BIT(0)
#define ATMEL_SPI_SR_TDRE		BIT(1)
#define ATMEL_SPI_SR_MODF		BIT(2)
#define ATMEL_SPI_SR_OVRES		BIT(3)
#define ATMEL_SPI_SR_ENDRX		BIT(4)
#define ATMEL_SPI_SR_ENDTX		BIT(5)
#define ATMEL_SPI_SR_RXBUFF		BIT(6)
#define ATMEL_SPI_SR_TXBUFE		BIT(7)
#define ATMEL_SPI_SR_NSSR		BIT(8)
#define ATMEL_SPI_SR_TXEMPTY		BIT(9)
#define ATMEL_SPI_SR_SPIENS		BIT(16)

/* Bits in CSRx */
#define ATMEL_SPI_CSRx_CPOL		BIT(0)
#define ATMEL_SPI_CSRx_NCPHA		BIT(1)
#define ATMEL_SPI_CSRx_CSAAT		BIT(3)
#define ATMEL_SPI_CSRx_BITS(x)		((x) << 4)
#define ATMEL_SPI_CSRx_SCBR(x)		((x) << 8)
#define ATMEL_SPI_CSRx_SCBR_MAX		GENMASK(7, 0)
#define ATMEL_SPI_CSRx_DLYBS(x)		((x) << 16)
#define ATMEL_SPI_CSRx_DLYBCT(x)	((x) << 24)

/* Bits in VERSION */
#define ATMEL_SPI_VERSION_REV(x)	((x) & 0xfff)
#define ATMEL_SPI_VERSION_MFN(x)	((x) << 16)

/* Constants for CSRx:BITS */
#define ATMEL_SPI_BITS_8		0
#define ATMEL_SPI_BITS_9		1
#define ATMEL_SPI_BITS_10		2
#define ATMEL_SPI_BITS_11		3
#define ATMEL_SPI_BITS_12		4
#define ATMEL_SPI_BITS_13		5
#define ATMEL_SPI_BITS_14		6
#define ATMEL_SPI_BITS_15		7
#define ATMEL_SPI_BITS_16		8

#define MAX_CS_COUNT	4

/* Register access macros */
#define spi_readl(as, reg)					\
	readl(as->regs + ATMEL_SPI_##reg)
#define spi_writel(as, reg, value)				\
	writel(value, as->regs + ATMEL_SPI_##reg)

struct atmel_spi_plat {
	struct at91_spi *regs;
};

struct atmel_spi_priv {
	unsigned int freq;		/* Default frequency */
	unsigned int mode;
	ulong bus_clk_rate;
#if CONFIG_IS_ENABLED(DM_GPIO)
	struct gpio_desc cs_gpios[MAX_CS_COUNT];
#endif
};

static int atmel_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct atmel_spi_plat *bus_plat = dev_get_plat(bus);
	struct atmel_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
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
	struct atmel_spi_plat *bus_plat = dev_get_plat(bus);

	writel(ATMEL_SPI_CR_SPIDIS, &bus_plat->regs->cr);

	return 0;
}

static void atmel_spi_cs_activate(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(DM_GPIO)
	struct udevice *bus = dev_get_parent(dev);
	struct atmel_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
	u32 cs = slave_plat->cs;

	if (!dm_gpio_is_valid(&priv->cs_gpios[cs]))
		return;

	dm_gpio_set_value(&priv->cs_gpios[cs], 0);
#endif
}

static void atmel_spi_cs_deactivate(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(DM_GPIO)
	struct udevice *bus = dev_get_parent(dev);
	struct atmel_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);
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
	struct atmel_spi_plat *bus_plat = dev_get_plat(bus);
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

	return 0;
}

static int atmel_spi_probe(struct udevice *bus)
{
	struct atmel_spi_plat *bus_plat = dev_get_plat(bus);
	int ret;

	ret = atmel_spi_enable_clk(bus);
	if (ret)
		return ret;

	bus_plat->regs = dev_read_addr_ptr(bus);

#if CONFIG_IS_ENABLED(DM_GPIO)
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
	.plat_auto	= sizeof(struct atmel_spi_plat),
	.priv_auto	= sizeof(struct atmel_spi_priv),
	.probe	= atmel_spi_probe,
};
