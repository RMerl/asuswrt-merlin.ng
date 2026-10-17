// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2010-2013 NVIDIA Corporation
 * With help from the mpc8xxx SPI driver
 * With more help from omap3_spi SPI driver
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <time.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <asm/arch/pinmux.h>
#include <asm/arch-tegra/clk_rst.h>
#include <spi.h>
#include <fdtdec.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include "tegra_spi.h"

DECLARE_GLOBAL_DATA_PTR;

#define SPI_CMD_GO			BIT(30)
#define SPI_CMD_ACTIVE_SCLK_SHIFT	26
#define SPI_CMD_ACTIVE_SCLK_MASK	(3 << SPI_CMD_ACTIVE_SCLK_SHIFT)
#define SPI_CMD_CK_SDA			BIT(21)
#define SPI_CMD_ACTIVE_SDA_SHIFT	18
#define SPI_CMD_ACTIVE_SDA_MASK		(3 << SPI_CMD_ACTIVE_SDA_SHIFT)
#define SPI_CMD_CS_POL			BIT(16)
#define SPI_CMD_TXEN			BIT(15)
#define SPI_CMD_RXEN			BIT(14)
#define SPI_CMD_CS_VAL			BIT(13)
#define SPI_CMD_CS_SOFT			BIT(12)
#define SPI_CMD_CS_DELAY		BIT(9)
#define SPI_CMD_CS3_EN			BIT(8)
#define SPI_CMD_CS2_EN			BIT(7)
#define SPI_CMD_CS1_EN			BIT(6)
#define SPI_CMD_CS0_EN			BIT(5)
#define SPI_CMD_BIT_LENGTH		BIT(4)
#define SPI_CMD_BIT_LENGTH_MASK		GENMASK(4, 0)

#define SPI_STAT_BSY			BIT(31)
#define SPI_STAT_RDY			BIT(30)
#define SPI_STAT_RXF_FLUSH		BIT(29)
#define SPI_STAT_TXF_FLUSH		BIT(28)
#define SPI_STAT_RXF_UNR		BIT(27)
#define SPI_STAT_TXF_OVF		BIT(26)
#define SPI_STAT_RXF_EMPTY		BIT(25)
#define SPI_STAT_RXF_FULL		BIT(24)
#define SPI_STAT_TXF_EMPTY		BIT(23)
#define SPI_STAT_TXF_FULL		BIT(22)
#define SPI_STAT_SEL_TXRX_N		BIT(16)
#define SPI_STAT_CUR_BLKCNT		BIT(15)

#define SPI_TIMEOUT		1000
#define TEGRA_SPI_MAX_FREQ	52000000

struct spi_regs {
	u32 command;	/* SPI_COMMAND_0 register  */
	u32 status;	/* SPI_STATUS_0 register */
	u32 rx_cmp;	/* SPI_RX_CMP_0 register  */
	u32 dma_ctl;	/* SPI_DMA_CTL_0 register */
	u32 tx_fifo;	/* SPI_TX_FIFO_0 register */
	u32 rsvd[3];	/* offsets 0x14 to 0x1F reserved */
	u32 rx_fifo;	/* SPI_RX_FIFO_0 register */
};

struct tegra20_sflash_priv {
	struct spi_regs *regs;
	unsigned int freq;
	unsigned int mode;
	int periph_id;
	int valid;
	int last_transaction_us;
};

int tegra20_sflash_cs_info(struct udevice *bus, unsigned int cs,
			   struct spi_cs_info *info)
{
	/* Tegra20 SPI-Flash - only 1 device ('bus/cs') */
	if (cs != 0)
		return -EINVAL;
	else
		return 0;
}

static int tegra20_sflash_of_to_plat(struct udevice *bus)
{
	struct tegra_spi_plat *plat = dev_get_plat(bus);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(bus);

	plat->base = dev_read_addr(bus);
	plat->periph_id = clock_decode_periph_id(bus);

	if (plat->periph_id == PERIPH_ID_NONE) {
		debug("%s: could not decode periph id %d\n", __func__,
		      plat->periph_id);
		return -FDT_ERR_NOTFOUND;
	}

	/* Use 500KHz as a suitable default */
	plat->frequency = fdtdec_get_int(blob, node, "spi-max-frequency",
					500000);
	plat->deactivate_delay_us = fdtdec_get_int(blob, node,
					"spi-deactivate-delay", 0);
	debug("%s: base=%#08lx, periph_id=%d, max-frequency=%d, deactivate_delay=%d\n",
	      __func__, plat->base, plat->periph_id, plat->frequency,
	      plat->deactivate_delay_us);

	return 0;
}

static int tegra20_sflash_probe(struct udevice *bus)
{
	struct tegra_spi_plat *plat = dev_get_plat(bus);
	struct tegra20_sflash_priv *priv = dev_get_priv(bus);

	priv->regs = (struct spi_regs *)plat->base;

	priv->last_transaction_us = timer_get_us();
	priv->freq = plat->frequency;
	priv->periph_id = plat->periph_id;

	/* Change SPI clock to correct frequency, PLLP_OUT0 source */
	clock_start_periph_pll(priv->periph_id, CLOCK_ID_PERIPH,
			       priv->freq);

	return 0;
}

static int tegra20_sflash_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct tegra20_sflash_priv *priv = dev_get_priv(bus);
	struct spi_regs *regs = priv->regs;
	u32 reg;

	/* Change SPI clock to correct frequency, PLLP_OUT0 source */
	clock_start_periph_pll(priv->periph_id, CLOCK_ID_PERIPH,
			       priv->freq);

	/* Clear stale status here */
	reg = SPI_STAT_RDY | SPI_STAT_RXF_FLUSH | SPI_STAT_TXF_FLUSH | \
		SPI_STAT_RXF_UNR | SPI_STAT_TXF_OVF;
	writel(reg, &regs->status);
	debug("%s: STATUS = %08x\n", __func__, readl(&regs->status));

	/*
	 * Use sw-controlled CS, so we can clock in data after ReadID, etc.
	 */
	reg = (priv->mode & 1) << SPI_CMD_ACTIVE_SDA_SHIFT;
	if (priv->mode & 2)
		reg |= 1 << SPI_CMD_ACTIVE_SCLK_SHIFT;
	clrsetbits_le32(&regs->command, SPI_CMD_ACTIVE_SCLK_MASK |
		SPI_CMD_ACTIVE_SDA_MASK, SPI_CMD_CS_SOFT | reg);
	debug("%s: COMMAND = %08x\n", __func__, readl(&regs->command));

	/*
	 * SPI pins on Tegra20 are muxed - change pinmux later due to UART
	 * issue.
	 */
	pinmux_set_func(PMUX_PINGRP_GMD, PMUX_FUNC_SFLASH);
	pinmux_tristate_disable(PMUX_PINGRP_LSPI);
	pinmux_set_func(PMUX_PINGRP_GMC, PMUX_FUNC_SFLASH);

	return 0;
}

static void spi_cs_activate(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct tegra_spi_plat *pdata = dev_get_plat(bus);
	struct tegra20_sflash_priv *priv = dev_get_priv(bus);

	/* If it's too soon to do another transaction, wait */
	if (pdata->deactivate_delay_us &&
	    priv->last_transaction_us) {
		ulong delay_us;		/* The delay completed so far */
		delay_us = timer_get_us() - priv->last_transaction_us;
		if (delay_us < pdata->deactivate_delay_us)
			udelay(pdata->deactivate_delay_us - delay_us);
	}

	/* CS is negated on Tegra, so drive a 1 to get a 0 */
	setbits_le32(&priv->regs->command, SPI_CMD_CS_VAL);
}

static void spi_cs_deactivate(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct tegra_spi_plat *pdata = dev_get_plat(bus);
	struct tegra20_sflash_priv *priv = dev_get_priv(bus);

	/* CS is negated on Tegra, so drive a 0 to get a 1 */
	clrbits_le32(&priv->regs->command, SPI_CMD_CS_VAL);

	/* Remember time of this transaction so we can honour the bus delay */
	if (pdata->deactivate_delay_us)
		priv->last_transaction_us = timer_get_us();
}

static int tegra20_sflash_xfer(struct udevice *dev, unsigned int bitlen,
			     const void *data_out, void *data_in,
			     unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct tegra20_sflash_priv *priv = dev_get_priv(bus);
	struct spi_regs *regs = priv->regs;
	u32 reg, tmpdout, tmpdin = 0;
	const u8 *dout = data_out;
	u8 *din = data_in;
	int num_bytes;
	int ret;

	debug("%s: slave %u:%u dout %p din %p bitlen %u\n",
	      __func__, dev_seq(bus), spi_chip_select(dev), dout, din, bitlen);
	if (bitlen % 8)
		return -1;
	num_bytes = bitlen / 8;

	ret = 0;

	reg = readl(&regs->status);
	writel(reg, &regs->status);	/* Clear all SPI events via R/W */
	debug("spi_xfer entry: STATUS = %08x\n", reg);

	reg = readl(&regs->command);
	reg |= SPI_CMD_TXEN | SPI_CMD_RXEN;
	writel(reg, &regs->command);
	debug("spi_xfer: COMMAND = %08x\n", readl(&regs->command));

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(dev);

	/* handle data in 32-bit chunks */
	while (num_bytes > 0) {
		int bytes;
		int is_read = 0;
		int tm, i;

		tmpdout = 0;
		bytes = (num_bytes > 4) ?  4 : num_bytes;

		if (dout != NULL) {
			for (i = 0; i < bytes; ++i)
				tmpdout = (tmpdout << 8) | dout[i];
		}

		num_bytes -= bytes;
		if (dout)
			dout += bytes;

		clrsetbits_le32(&regs->command, SPI_CMD_BIT_LENGTH_MASK,
				bytes * 8 - 1);
		writel(tmpdout, &regs->tx_fifo);
		setbits_le32(&regs->command, SPI_CMD_GO);

		/*
		 * Wait for SPI transmit FIFO to empty, or to time out.
		 * The RX FIFO status will be read and cleared last
		 */
		for (tm = 0, is_read = 0; tm < SPI_TIMEOUT; ++tm) {
			u32 status;

			status = readl(&regs->status);

			/* We can exit when we've had both RX and TX activity */
			if (is_read && (status & SPI_STAT_TXF_EMPTY))
				break;

			if ((status & (SPI_STAT_BSY | SPI_STAT_RDY)) !=
					SPI_STAT_RDY)
				tm++;

			else if (!(status & SPI_STAT_RXF_EMPTY)) {
				tmpdin = readl(&regs->rx_fifo);
				is_read = 1;

				/* swap bytes read in */
				if (din != NULL) {
					for (i = bytes - 1; i >= 0; --i) {
						din[i] = tmpdin & 0xff;
						tmpdin >>= 8;
					}
					din += bytes;
				}
			}
		}

		if (tm >= SPI_TIMEOUT)
			ret = tm;

		/* clear ACK RDY, etc. bits */
		writel(readl(&regs->status), &regs->status);
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(dev);

	debug("spi_xfer: transfer ended. Value=%08x, status = %08x\n",
		tmpdin, readl(&regs->status));

	if (ret) {
		printf("spi_xfer: timeout during SPI transfer, tm %d\n", ret);
		return -1;
	}

	return 0;
}

static int tegra20_sflash_set_speed(struct udevice *bus, uint speed)
{
	struct tegra_spi_plat *plat = dev_get_plat(bus);
	struct tegra20_sflash_priv *priv = dev_get_priv(bus);

	if (speed > plat->frequency)
		speed = plat->frequency;
	priv->freq = speed;
	debug("%s: regs=%p, speed=%d\n", __func__, priv->regs, priv->freq);

	return 0;
}

static int tegra20_sflash_set_mode(struct udevice *bus, uint mode)
{
	struct tegra20_sflash_priv *priv = dev_get_priv(bus);

	priv->mode = mode;
	debug("%s: regs=%p, mode=%d\n", __func__, priv->regs, priv->mode);

	return 0;
}

static const struct dm_spi_ops tegra20_sflash_ops = {
	.claim_bus	= tegra20_sflash_claim_bus,
	.xfer		= tegra20_sflash_xfer,
	.set_speed	= tegra20_sflash_set_speed,
	.set_mode	= tegra20_sflash_set_mode,
	.cs_info	= tegra20_sflash_cs_info,
};

static const struct udevice_id tegra20_sflash_ids[] = {
	{ .compatible = "nvidia,tegra20-sflash" },
	{ }
};

U_BOOT_DRIVER(tegra20_sflash) = {
	.name	= "tegra20_sflash",
	.id	= UCLASS_SPI,
	.of_match = tegra20_sflash_ids,
	.ops	= &tegra20_sflash_ops,
	.of_to_plat = tegra20_sflash_of_to_plat,
	.plat_auto	= sizeof(struct tegra_spi_plat),
	.priv_auto	= sizeof(struct tegra20_sflash_priv),
	.probe	= tegra20_sflash_probe,
};
