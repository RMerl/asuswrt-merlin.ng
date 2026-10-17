// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Wills Wang <wills.wang@live.com>
 */

#include <common.h>
#include <clock_legacy.h>
#include <spi.h>
#include <dm.h>
#include <div64.h>
#include <errno.h>
#include <time.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/types.h>
#include <dm/pinctrl.h>
#include <mach/ar71xx_regs.h>

/* CLOCK_DIVIDER = 3 (SPI clock = 200 / 8 ~ 25 MHz) */
#define ATH79_SPI_CLK_DIV(x)           (((x) >> 1) - 1)
#define ATH79_SPI_RRW_DELAY_FACTOR     12000
#define ATH79_SPI_MHZ                  (1000 * 1000)

struct ath79_spi_priv {
	void __iomem *regs;
	u32 rrw_delay;
};

static void spi_cs_activate(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct ath79_spi_priv *priv = dev_get_priv(bus);

	writel(AR71XX_SPI_FS_GPIO, priv->regs + AR71XX_SPI_REG_FS);
	writel(AR71XX_SPI_IOC_CS_ALL, priv->regs + AR71XX_SPI_REG_IOC);
}

static void spi_cs_deactivate(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct ath79_spi_priv *priv = dev_get_priv(bus);

	writel(AR71XX_SPI_IOC_CS_ALL, priv->regs + AR71XX_SPI_REG_IOC);
	writel(0, priv->regs + AR71XX_SPI_REG_FS);
}

static int ath79_spi_claim_bus(struct udevice *dev)
{
	return 0;
}

static int ath79_spi_release_bus(struct udevice *dev)
{
	return 0;
}

static int ath79_spi_xfer(struct udevice *dev, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct ath79_spi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave = dev_get_parent_plat(dev);
	u8 *rx = din;
	const u8 *tx = dout;
	u8 curbyte, curbitlen, restbits;
	u32 bytes = bitlen / 8;
	u32 out, in;
	u64 tick;

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(dev);

	restbits = (bitlen % 8);
	if (restbits)
		bytes++;

	out = AR71XX_SPI_IOC_CS_ALL & ~(AR71XX_SPI_IOC_CS(slave->cs));
	while (bytes > 0) {
		bytes--;
		curbyte = 0;
		if (tx)
			curbyte = *tx++;

		if (restbits && !bytes) {
			curbitlen = restbits;
			curbyte <<= 8 - restbits;
		} else {
			curbitlen = 8;
		}

		for (curbyte <<= (8 - curbitlen); curbitlen; curbitlen--) {
			if (curbyte & 0x80)
				out |= AR71XX_SPI_IOC_DO;
			else
				out &= ~(AR71XX_SPI_IOC_DO);

			writel(out, priv->regs + AR71XX_SPI_REG_IOC);

			/* delay for low level */
			if (priv->rrw_delay) {
				tick = get_ticks() + priv->rrw_delay;
				while (get_ticks() < tick)
					/*NOP*/;
			}

			writel(out | AR71XX_SPI_IOC_CLK,
			       priv->regs + AR71XX_SPI_REG_IOC);

			/* delay for high level */
			if (priv->rrw_delay) {
				tick = get_ticks() + priv->rrw_delay;
				while (get_ticks() < tick)
					/*NOP*/;
			}

			curbyte <<= 1;
		}

		if (!bytes)
			writel(out, priv->regs + AR71XX_SPI_REG_IOC);

		in = readl(priv->regs + AR71XX_SPI_REG_RDS);
		if (rx) {
			if (restbits && !bytes)
				*rx++ = (in << (8 - restbits));
			else
				*rx++ = in;
		}
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(dev);

	return 0;
}


static int ath79_spi_set_speed(struct udevice *bus, uint speed)
{
	struct ath79_spi_priv *priv = dev_get_priv(bus);
	u32 val, div = 0;
	u64 time;

	if (speed)
		div = get_bus_freq(0) / speed;

	if (div > 63)
		div = 63;

	if (div < 5)
		div = 5;

	/* calculate delay */
	time = get_tbclk();
	do_div(time, speed / 2);
	val = get_bus_freq(0) / ATH79_SPI_MHZ;
	val = ATH79_SPI_RRW_DELAY_FACTOR / val;
	if (time > val)
		priv->rrw_delay = time - val + 1;
	else
		priv->rrw_delay = 0;

	writel(AR71XX_SPI_FS_GPIO, priv->regs + AR71XX_SPI_REG_FS);
	clrsetbits_be32(priv->regs + AR71XX_SPI_REG_CTRL,
			AR71XX_SPI_CTRL_DIV_MASK,
			ATH79_SPI_CLK_DIV(div));
	writel(0, priv->regs + AR71XX_SPI_REG_FS);
	return 0;
}

static int ath79_spi_set_mode(struct udevice *bus, uint mode)
{
	return 0;
}

static int ath79_spi_probe(struct udevice *bus)
{
	struct ath79_spi_priv *priv = dev_get_priv(bus);
	fdt_addr_t addr;

	addr = dev_read_addr(bus);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = map_physmem(addr,
				 AR71XX_SPI_SIZE,
				 MAP_NOCACHE);

	/* Init SPI Hardware, disable remap, set clock */
	writel(AR71XX_SPI_FS_GPIO, priv->regs + AR71XX_SPI_REG_FS);
	writel(AR71XX_SPI_CTRL_RD | ATH79_SPI_CLK_DIV(8),
	       priv->regs + AR71XX_SPI_REG_CTRL);
	writel(0, priv->regs + AR71XX_SPI_REG_FS);

	return 0;
}

static int ath79_cs_info(struct udevice *bus, uint cs,
			   struct spi_cs_info *info)
{
	/* Always allow activity on CS 0/1/2 */
	if (cs >= 3)
		return -EINVAL;

	return 0;
}

static const struct dm_spi_ops ath79_spi_ops = {
	.claim_bus  = ath79_spi_claim_bus,
	.release_bus    = ath79_spi_release_bus,
	.xfer       = ath79_spi_xfer,
	.set_speed  = ath79_spi_set_speed,
	.set_mode   = ath79_spi_set_mode,
	.cs_info    = ath79_cs_info,
};

static const struct udevice_id ath79_spi_ids[] = {
	{ .compatible = "qca,ar7100-spi" },
	{}
};

U_BOOT_DRIVER(ath79_spi) = {
	.name   = "ath79_spi",
	.id = UCLASS_SPI,
	.of_match = ath79_spi_ids,
	.ops    = &ath79_spi_ops,
	.priv_auto	= sizeof(struct ath79_spi_priv),
	.probe  = ath79_spi_probe,
};
