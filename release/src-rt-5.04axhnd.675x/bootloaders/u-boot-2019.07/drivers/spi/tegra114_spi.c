// SPDX-License-Identifier: GPL-2.0
/*
 * NVIDIA Tegra SPI controller (T114 and later)
 *
 * Copyright (c) 2010-2013 NVIDIA Corporation
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch-tegra/clk_rst.h>
#include <spi.h>
#include "tegra_spi.h"

/* COMMAND1 */
#define SPI_CMD1_GO			BIT(31)
#define SPI_CMD1_M_S			BIT(30)
#define SPI_CMD1_MODE_MASK		GENMASK(1, 0)
#define SPI_CMD1_MODE_SHIFT		28
#define SPI_CMD1_CS_SEL_MASK		GENMASK(1, 0)
#define SPI_CMD1_CS_SEL_SHIFT		26
#define SPI_CMD1_CS_POL_INACTIVE3	BIT(25)
#define SPI_CMD1_CS_POL_INACTIVE2	BIT(24)
#define SPI_CMD1_CS_POL_INACTIVE1	BIT(23)
#define SPI_CMD1_CS_POL_INACTIVE0	BIT(22)
#define SPI_CMD1_CS_SW_HW		BIT(21)
#define SPI_CMD1_CS_SW_VAL		BIT(20)
#define SPI_CMD1_IDLE_SDA_MASK		GENMASK(1, 0)
#define SPI_CMD1_IDLE_SDA_SHIFT		18
#define SPI_CMD1_BIDIR			BIT(17)
#define SPI_CMD1_LSBI_FE		BIT(16)
#define SPI_CMD1_LSBY_FE		BIT(15)
#define SPI_CMD1_BOTH_EN_BIT		BIT(14)
#define SPI_CMD1_BOTH_EN_BYTE		BIT(13)
#define SPI_CMD1_RX_EN			BIT(12)
#define SPI_CMD1_TX_EN			BIT(11)
#define SPI_CMD1_PACKED			BIT(5)
#define SPI_CMD1_BIT_LEN_MASK		GENMASK(4, 0)
#define SPI_CMD1_BIT_LEN_SHIFT		0

/* COMMAND2 */
#define SPI_CMD2_TX_CLK_TAP_DELAY	BIT(6)
#define SPI_CMD2_TX_CLK_TAP_DELAY_MASK	GENMASK(11, 6)
#define SPI_CMD2_RX_CLK_TAP_DELAY	BIT(0)
#define SPI_CMD2_RX_CLK_TAP_DELAY_MASK	GENMASK(5, 0)

/* TRANSFER STATUS */
#define SPI_XFER_STS_RDY		BIT(30)

/* FIFO STATUS */
#define SPI_FIFO_STS_CS_INACTIVE	BIT(31)
#define SPI_FIFO_STS_FRAME_END		BIT(30)
#define SPI_FIFO_STS_RX_FIFO_FLUSH	BIT(15)
#define SPI_FIFO_STS_TX_FIFO_FLUSH	BIT(14)
#define SPI_FIFO_STS_ERR		BIT(8)
#define SPI_FIFO_STS_TX_FIFO_OVF	BIT(7)
#define SPI_FIFO_STS_TX_FIFO_UNR	BIT(6)
#define SPI_FIFO_STS_RX_FIFO_OVF	BIT(5)
#define SPI_FIFO_STS_RX_FIFO_UNR	BIT(4)
#define SPI_FIFO_STS_TX_FIFO_FULL	BIT(3)
#define SPI_FIFO_STS_TX_FIFO_EMPTY	BIT(2)
#define SPI_FIFO_STS_RX_FIFO_FULL	BIT(1)
#define SPI_FIFO_STS_RX_FIFO_EMPTY	BIT(0)

#define SPI_TIMEOUT		1000
#define TEGRA_SPI_MAX_FREQ	52000000

struct spi_regs {
	u32 command1;	/* 000:SPI_COMMAND1 register */
	u32 command2;	/* 004:SPI_COMMAND2 register */
	u32 timing1;	/* 008:SPI_CS_TIM1 register */
	u32 timing2;	/* 00c:SPI_CS_TIM2 register */
	u32 xfer_status;/* 010:SPI_TRANS_STATUS register */
	u32 fifo_status;/* 014:SPI_FIFO_STATUS register */
	u32 tx_data;	/* 018:SPI_TX_DATA register */
	u32 rx_data;	/* 01c:SPI_RX_DATA register */
	u32 dma_ctl;	/* 020:SPI_DMA_CTL register */
	u32 dma_blk;	/* 024:SPI_DMA_BLK register */
	u32 rsvd[56];	/* 028-107 reserved */
	u32 tx_fifo;	/* 108:SPI_FIFO1 register */
	u32 rsvd2[31];	/* 10c-187 reserved */
	u32 rx_fifo;	/* 188:SPI_FIFO2 register */
	u32 spare_ctl;	/* 18c:SPI_SPARE_CTRL register */
};

struct tegra114_spi_priv {
	struct spi_regs *regs;
	unsigned int freq;
	unsigned int mode;
	int periph_id;
	int valid;
	int last_transaction_us;
};

static int tegra114_spi_ofdata_to_platdata(struct udevice *bus)
{
	struct tegra_spi_platdata *plat = bus->platdata;

	plat->base = dev_read_addr(bus);
	plat->periph_id = clock_decode_periph_id(bus);

	if (plat->periph_id == PERIPH_ID_NONE) {
		debug("%s: could not decode periph id %d\n", __func__,
		      plat->periph_id);
		return -FDT_ERR_NOTFOUND;
	}

	/* Use 500KHz as a suitable default */
	plat->frequency = dev_read_u32_default(bus, "spi-max-frequency",
					       500000);
	plat->deactivate_delay_us = dev_read_u32_default(bus,
						"spi-deactivate-delay", 0);
	debug("%s: base=%#08lx, periph_id=%d, max-frequency=%d, deactivate_delay=%d\n",
	      __func__, plat->base, plat->periph_id, plat->frequency,
	      plat->deactivate_delay_us);

	return 0;
}

static int tegra114_spi_probe(struct udevice *bus)
{
	struct tegra_spi_platdata *plat = dev_get_platdata(bus);
	struct tegra114_spi_priv *priv = dev_get_priv(bus);
	struct spi_regs *regs;
	ulong rate;

	priv->regs = (struct spi_regs *)plat->base;
	regs = priv->regs;

	priv->last_transaction_us = timer_get_us();
	priv->freq = plat->frequency;
	priv->periph_id = plat->periph_id;

	/*
	 * Change SPI clock to correct frequency, PLLP_OUT0 source, falling
	 * back to the oscillator if that is too fast.
	 */
	rate = clock_start_periph_pll(priv->periph_id, CLOCK_ID_PERIPH,
				      priv->freq);
	if (rate > priv->freq + 100000) {
		rate = clock_start_periph_pll(priv->periph_id, CLOCK_ID_OSC,
					      priv->freq);
		if (rate != priv->freq) {
			printf("Warning: SPI '%s' requested clock %u, actual clock %lu\n",
			       bus->name, priv->freq, rate);
		}
	}
	udelay(plat->deactivate_delay_us);

	/* Clear stale status here */
	setbits_le32(&regs->fifo_status,
		     SPI_FIFO_STS_ERR		|
		     SPI_FIFO_STS_TX_FIFO_OVF	|
		     SPI_FIFO_STS_TX_FIFO_UNR	|
		     SPI_FIFO_STS_RX_FIFO_OVF	|
		     SPI_FIFO_STS_RX_FIFO_UNR	|
		     SPI_FIFO_STS_TX_FIFO_FULL	|
		     SPI_FIFO_STS_TX_FIFO_EMPTY	|
		     SPI_FIFO_STS_RX_FIFO_FULL	|
		     SPI_FIFO_STS_RX_FIFO_EMPTY);
	debug("%s: FIFO STATUS = %08x\n", __func__, readl(&regs->fifo_status));

	setbits_le32(&priv->regs->command1, SPI_CMD1_M_S | SPI_CMD1_CS_SW_HW |
		     (priv->mode << SPI_CMD1_MODE_SHIFT) | SPI_CMD1_CS_SW_VAL);
	debug("%s: COMMAND1 = %08x\n", __func__, readl(&regs->command1));

	return 0;
}

/**
 * Activate the CS by driving it LOW
 *
 * @param slave	Pointer to spi_slave to which controller has to
 *		communicate with
 */
static void spi_cs_activate(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct tegra_spi_platdata *pdata = dev_get_platdata(bus);
	struct tegra114_spi_priv *priv = dev_get_priv(bus);

	/* If it's too soon to do another transaction, wait */
	if (pdata->deactivate_delay_us &&
	    priv->last_transaction_us) {
		ulong delay_us;		/* The delay completed so far */
		delay_us = timer_get_us() - priv->last_transaction_us;
		if (delay_us < pdata->deactivate_delay_us)
			udelay(pdata->deactivate_delay_us - delay_us);
	}

	clrbits_le32(&priv->regs->command1, SPI_CMD1_CS_SW_VAL);
}

/**
 * Deactivate the CS by driving it HIGH
 *
 * @param slave	Pointer to spi_slave to which controller has to
 *		communicate with
 */
static void spi_cs_deactivate(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct tegra_spi_platdata *pdata = dev_get_platdata(bus);
	struct tegra114_spi_priv *priv = dev_get_priv(bus);

	setbits_le32(&priv->regs->command1, SPI_CMD1_CS_SW_VAL);

	/* Remember time of this transaction so we can honour the bus delay */
	if (pdata->deactivate_delay_us)
		priv->last_transaction_us = timer_get_us();

	debug("Deactivate CS, bus '%s'\n", bus->name);
}

static int tegra114_spi_xfer(struct udevice *dev, unsigned int bitlen,
			     const void *data_out, void *data_in,
			     unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct tegra114_spi_priv *priv = dev_get_priv(bus);
	struct spi_regs *regs = priv->regs;
	u32 reg, tmpdout, tmpdin = 0;
	const u8 *dout = data_out;
	u8 *din = data_in;
	int num_bytes;
	int ret;

	debug("%s: slave %u:%u dout %p din %p bitlen %u\n",
	      __func__, bus->seq, spi_chip_select(dev), dout, din, bitlen);
	if (bitlen % 8)
		return -1;
	num_bytes = bitlen / 8;

	ret = 0;

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(dev);

	/* clear all error status bits */
	reg = readl(&regs->fifo_status);
	writel(reg, &regs->fifo_status);

	clrsetbits_le32(&regs->command1, SPI_CMD1_CS_SW_VAL,
			SPI_CMD1_RX_EN | SPI_CMD1_TX_EN | SPI_CMD1_LSBY_FE |
			(spi_chip_select(dev) << SPI_CMD1_CS_SEL_SHIFT));

	/* set xfer size to 1 block (32 bits) */
	writel(0, &regs->dma_blk);

	/* handle data in 32-bit chunks */
	while (num_bytes > 0) {
		int bytes;
		int tm, i;

		tmpdout = 0;
		bytes = (num_bytes > 4) ?  4 : num_bytes;

		if (dout != NULL) {
			for (i = 0; i < bytes; ++i)
				tmpdout = (tmpdout << 8) | dout[i];
			dout += bytes;
		}

		num_bytes -= bytes;

		/* clear ready bit */
		setbits_le32(&regs->xfer_status, SPI_XFER_STS_RDY);

		clrsetbits_le32(&regs->command1,
				SPI_CMD1_BIT_LEN_MASK << SPI_CMD1_BIT_LEN_SHIFT,
				(bytes * 8 - 1) << SPI_CMD1_BIT_LEN_SHIFT);
		writel(tmpdout, &regs->tx_fifo);
		setbits_le32(&regs->command1, SPI_CMD1_GO);

		/*
		 * Wait for SPI transmit FIFO to empty, or to time out.
		 * The RX FIFO status will be read and cleared last
		 */
		for (tm = 0; tm < SPI_TIMEOUT; ++tm) {
			u32 fifo_status, xfer_status;

			xfer_status = readl(&regs->xfer_status);
			if (!(xfer_status & SPI_XFER_STS_RDY))
				continue;

			fifo_status = readl(&regs->fifo_status);
			if (fifo_status & SPI_FIFO_STS_ERR) {
				debug("%s: got a fifo error: ", __func__);
				if (fifo_status & SPI_FIFO_STS_TX_FIFO_OVF)
					debug("tx FIFO overflow ");
				if (fifo_status & SPI_FIFO_STS_TX_FIFO_UNR)
					debug("tx FIFO underrun ");
				if (fifo_status & SPI_FIFO_STS_RX_FIFO_OVF)
					debug("rx FIFO overflow ");
				if (fifo_status & SPI_FIFO_STS_RX_FIFO_UNR)
					debug("rx FIFO underrun ");
				if (fifo_status & SPI_FIFO_STS_TX_FIFO_FULL)
					debug("tx FIFO full ");
				if (fifo_status & SPI_FIFO_STS_TX_FIFO_EMPTY)
					debug("tx FIFO empty ");
				if (fifo_status & SPI_FIFO_STS_RX_FIFO_FULL)
					debug("rx FIFO full ");
				if (fifo_status & SPI_FIFO_STS_RX_FIFO_EMPTY)
					debug("rx FIFO empty ");
				debug("\n");
				break;
			}

			if (!(fifo_status & SPI_FIFO_STS_RX_FIFO_EMPTY)) {
				tmpdin = readl(&regs->rx_fifo);

				/* swap bytes read in */
				if (din != NULL) {
					for (i = bytes - 1; i >= 0; --i) {
						din[i] = tmpdin & 0xff;
						tmpdin >>= 8;
					}
					din += bytes;
				}

				/* We can exit when we've had both RX and TX */
				break;
			}
		}

		if (tm >= SPI_TIMEOUT)
			ret = tm;

		/* clear ACK RDY, etc. bits */
		writel(readl(&regs->fifo_status), &regs->fifo_status);
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(dev);

	debug("%s: transfer ended. Value=%08x, fifo_status = %08x\n",
	      __func__, tmpdin, readl(&regs->fifo_status));

	if (ret) {
		printf("%s: timeout during SPI transfer, tm %d\n",
		       __func__, ret);
		return -1;
	}

	return ret;
}

static int tegra114_spi_set_speed(struct udevice *bus, uint speed)
{
	struct tegra_spi_platdata *plat = bus->platdata;
	struct tegra114_spi_priv *priv = dev_get_priv(bus);

	if (speed > plat->frequency)
		speed = plat->frequency;
	priv->freq = speed;
	debug("%s: regs=%p, speed=%d\n", __func__, priv->regs, priv->freq);

	return 0;
}

static int tegra114_spi_set_mode(struct udevice *bus, uint mode)
{
	struct tegra114_spi_priv *priv = dev_get_priv(bus);

	priv->mode = mode;
	debug("%s: regs=%p, mode=%d\n", __func__, priv->regs, priv->mode);

	return 0;
}

static const struct dm_spi_ops tegra114_spi_ops = {
	.xfer		= tegra114_spi_xfer,
	.set_speed	= tegra114_spi_set_speed,
	.set_mode	= tegra114_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id tegra114_spi_ids[] = {
	{ .compatible = "nvidia,tegra114-spi" },
	{ }
};

U_BOOT_DRIVER(tegra114_spi) = {
	.name	= "tegra114_spi",
	.id	= UCLASS_SPI,
	.of_match = tegra114_spi_ids,
	.ops	= &tegra114_spi_ops,
	.ofdata_to_platdata = tegra114_spi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct tegra_spi_platdata),
	.priv_auto_alloc_size = sizeof(struct tegra114_spi_priv),
	.probe	= tegra114_spi_probe,
};
