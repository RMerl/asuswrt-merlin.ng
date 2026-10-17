// SPDX-License-Identifier: GPL-2.0+
/*
 * NVIDIA Tegra210 QSPI controller driver
 *
 * (C) Copyright 2015-2020 NVIDIA Corporation <www.nvidia.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <time.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch-tegra/clk_rst.h>
#include <spi.h>
#include <fdtdec.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include "tegra_spi.h"

DECLARE_GLOBAL_DATA_PTR;

/* COMMAND1 */
#define QSPI_CMD1_GO			BIT(31)
#define QSPI_CMD1_M_S			BIT(30)
#define QSPI_CMD1_MODE_MASK		GENMASK(1,0)
#define QSPI_CMD1_MODE_SHIFT		28
#define QSPI_CMD1_CS_SEL_MASK		GENMASK(1,0)
#define QSPI_CMD1_CS_SEL_SHIFT		26
#define QSPI_CMD1_CS_POL_INACTIVE0	BIT(22)
#define QSPI_CMD1_CS_SW_HW		BIT(21)
#define QSPI_CMD1_CS_SW_VAL		BIT(20)
#define QSPI_CMD1_IDLE_SDA_MASK		GENMASK(1,0)
#define QSPI_CMD1_IDLE_SDA_SHIFT	18
#define QSPI_CMD1_BIDIR			BIT(17)
#define QSPI_CMD1_LSBI_FE		BIT(16)
#define QSPI_CMD1_LSBY_FE		BIT(15)
#define QSPI_CMD1_BOTH_EN_BIT		BIT(14)
#define QSPI_CMD1_BOTH_EN_BYTE		BIT(13)
#define QSPI_CMD1_RX_EN			BIT(12)
#define QSPI_CMD1_TX_EN			BIT(11)
#define QSPI_CMD1_PACKED		BIT(5)
#define QSPI_CMD1_BITLEN_MASK		GENMASK(4,0)
#define QSPI_CMD1_BITLEN_SHIFT		0

/* COMMAND2 */
#define QSPI_CMD2_TX_CLK_TAP_DELAY_SHIFT	10
#define QSPI_CMD2_TX_CLK_TAP_DELAY_MASK	GENMASK(14,10)
#define QSPI_CMD2_RX_CLK_TAP_DELAY_SHIFT	0
#define QSPI_CMD2_RX_CLK_TAP_DELAY_MASK	GENMASK(7,0)

/* TRANSFER STATUS */
#define QSPI_XFER_STS_RDY		BIT(30)

/* FIFO STATUS */
#define QSPI_FIFO_STS_CS_INACTIVE	BIT(31)
#define QSPI_FIFO_STS_FRAME_END		BIT(30)
#define QSPI_FIFO_STS_RX_FIFO_FLUSH	BIT(15)
#define QSPI_FIFO_STS_TX_FIFO_FLUSH	BIT(14)
#define QSPI_FIFO_STS_ERR		BIT(8)
#define QSPI_FIFO_STS_TX_FIFO_OVF	BIT(7)
#define QSPI_FIFO_STS_TX_FIFO_UNR	BIT(6)
#define QSPI_FIFO_STS_RX_FIFO_OVF	BIT(5)
#define QSPI_FIFO_STS_RX_FIFO_UNR	BIT(4)
#define QSPI_FIFO_STS_TX_FIFO_FULL	BIT(3)
#define QSPI_FIFO_STS_TX_FIFO_EMPTY	BIT(2)
#define QSPI_FIFO_STS_RX_FIFO_FULL	BIT(1)
#define QSPI_FIFO_STS_RX_FIFO_EMPTY	BIT(0)

#define QSPI_TIMEOUT		1000

struct qspi_regs {
	u32 command1;	/* 000:QSPI_COMMAND1 register */
	u32 command2;	/* 004:QSPI_COMMAND2 register */
	u32 timing1;	/* 008:QSPI_CS_TIM1 register */
	u32 timing2;	/* 00c:QSPI_CS_TIM2 register */
	u32 xfer_status;/* 010:QSPI_TRANS_STATUS register */
	u32 fifo_status;/* 014:QSPI_FIFO_STATUS register */
	u32 tx_data;	/* 018:QSPI_TX_DATA register */
	u32 rx_data;	/* 01c:QSPI_RX_DATA register */
	u32 dma_ctl;	/* 020:QSPI_DMA_CTL register */
	u32 dma_blk;	/* 024:QSPI_DMA_BLK register */
	u32 rsvd[56];	/* 028-107 reserved */
	u32 tx_fifo;	/* 108:QSPI_FIFO1 register */
	u32 rsvd2[31];	/* 10c-187 reserved */
	u32 rx_fifo;	/* 188:QSPI_FIFO2 register */
	u32 spare_ctl;	/* 18c:QSPI_SPARE_CTRL register */
};

struct tegra210_qspi_priv {
	struct qspi_regs *regs;
	unsigned int freq;
	unsigned int mode;
	int periph_id;
	int valid;
	int last_transaction_us;
};

static int tegra210_qspi_of_to_plat(struct udevice *bus)
{
	struct tegra_spi_plat *plat = dev_get_plat(bus);

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
							 "spi-deactivate-delay",
							 0);
	debug("%s: base=%#08lx, periph_id=%d, max-frequency=%d, deactivate_delay=%d\n",
	      __func__, plat->base, plat->periph_id, plat->frequency,
	      plat->deactivate_delay_us);

	return 0;
}

static int tegra210_qspi_probe(struct udevice *bus)
{
	struct tegra_spi_plat *plat = dev_get_plat(bus);
	struct tegra210_qspi_priv *priv = dev_get_priv(bus);

	priv->regs = (struct qspi_regs *)plat->base;
	struct qspi_regs *regs = priv->regs;

	priv->last_transaction_us = timer_get_us();
	priv->freq = plat->frequency;
	priv->periph_id = plat->periph_id;

	debug("%s: Freq = %u, id = %d\n", __func__, priv->freq,
	      priv->periph_id);
	/* Change SPI clock to correct frequency, PLLP_OUT0 source */
	clock_start_periph_pll(priv->periph_id, CLOCK_ID_PERIPH, priv->freq);

	/* Set tap delays here, clock change above resets QSPI controller */
	u32 reg = (0x09 << QSPI_CMD2_TX_CLK_TAP_DELAY_SHIFT) |
		   (0x0C << QSPI_CMD2_RX_CLK_TAP_DELAY_SHIFT);
	writel(reg, &regs->command2);
	debug("%s: COMMAND2 = %08x\n", __func__, readl(&regs->command2));

	return 0;
}

static int tegra210_qspi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct tegra210_qspi_priv *priv = dev_get_priv(bus);
	struct qspi_regs *regs = priv->regs;

	debug("%s: FIFO STATUS = %08x\n", __func__, readl(&regs->fifo_status));

	/* Set master mode and sw controlled CS */
	setbits_le32(&regs->command1, QSPI_CMD1_M_S | QSPI_CMD1_CS_SW_HW |
		     (priv->mode << QSPI_CMD1_MODE_SHIFT));
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
	struct tegra_spi_plat *pdata = dev_get_plat(bus);
	struct tegra210_qspi_priv *priv = dev_get_priv(bus);

	/* If it's too soon to do another transaction, wait */
	if (pdata->deactivate_delay_us &&
	    priv->last_transaction_us) {
		ulong delay_us;		/* The delay completed so far */
		delay_us = timer_get_us() - priv->last_transaction_us;
		if (delay_us < pdata->deactivate_delay_us)
			udelay(pdata->deactivate_delay_us - delay_us);
	}

	clrbits_le32(&priv->regs->command1, QSPI_CMD1_CS_SW_VAL);
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
	struct tegra_spi_plat *pdata = dev_get_plat(bus);
	struct tegra210_qspi_priv *priv = dev_get_priv(bus);

	setbits_le32(&priv->regs->command1, QSPI_CMD1_CS_SW_VAL);

	/* Remember time of this transaction so we can honour the bus delay */
	if (pdata->deactivate_delay_us)
		priv->last_transaction_us = timer_get_us();

	debug("Deactivate CS, bus '%s'\n", bus->name);
}

static int tegra210_qspi_xfer(struct udevice *dev, unsigned int bitlen,
			     const void *data_out, void *data_in,
			     unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct tegra210_qspi_priv *priv = dev_get_priv(bus);
	struct qspi_regs *regs = priv->regs;
	u32 reg, tmpdout, tmpdin = 0;
	const u8 *dout = data_out;
	u8 *din = data_in;
	int num_bytes, tm, ret;

	debug("%s: slave %u:%u dout %p din %p bitlen %u\n",
	      __func__, dev_seq(bus), spi_chip_select(dev), dout, din, bitlen);
	if (bitlen % 8)
		return -1;
	num_bytes = bitlen / 8;

	ret = 0;

	/* clear all error status bits */
	reg = readl(&regs->fifo_status);
	writel(reg, &regs->fifo_status);

	/* flush RX/TX FIFOs */
	setbits_le32(&regs->fifo_status,
		     (QSPI_FIFO_STS_RX_FIFO_FLUSH |
		      QSPI_FIFO_STS_TX_FIFO_FLUSH));

	tm = QSPI_TIMEOUT;
	while ((tm && readl(&regs->fifo_status) &
		      (QSPI_FIFO_STS_RX_FIFO_FLUSH |
		       QSPI_FIFO_STS_TX_FIFO_FLUSH))) {
		tm--;
		udelay(1);
	}

	if (!tm) {
		printf("%s: timeout during QSPI FIFO flush!\n",
		       __func__);
		return -1;
	}

	/*
	 * Notes:
	 *   1. don't set LSBY_FE, so no need to swap bytes from/to TX/RX FIFOs;
	 *   2. don't set RX_EN and TX_EN yet.
	 *      (SW needs to make sure that while programming the blk_size,
	 *       tx_en and rx_en bits must be zero)
	 *      [TODO] I (Yen Lin) have problems when both RX/TX EN bits are set
	 *	       i.e., both dout and din are not NULL.
	 */
	clrsetbits_le32(&regs->command1,
			(QSPI_CMD1_LSBI_FE | QSPI_CMD1_LSBY_FE |
			 QSPI_CMD1_RX_EN | QSPI_CMD1_TX_EN),
			(spi_chip_select(dev) << QSPI_CMD1_CS_SEL_SHIFT));

	/* set xfer size to 1 block (32 bits) */
	writel(0, &regs->dma_blk);

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(dev);

	/* handle data in 32-bit chunks */
	while (num_bytes > 0) {
		int bytes;

		tmpdout = 0;
		bytes = (num_bytes > 4) ?  4 : num_bytes;

		if (dout != NULL) {
			memcpy((void *)&tmpdout, (void *)dout, bytes);
			dout += bytes;
			num_bytes -= bytes;
			writel(tmpdout, &regs->tx_fifo);
			setbits_le32(&regs->command1, QSPI_CMD1_TX_EN);
		}

		if (din != NULL)
			setbits_le32(&regs->command1, QSPI_CMD1_RX_EN);

		/* clear ready bit */
		setbits_le32(&regs->xfer_status, QSPI_XFER_STS_RDY);

		clrsetbits_le32(&regs->command1,
				QSPI_CMD1_BITLEN_MASK << QSPI_CMD1_BITLEN_SHIFT,
				(bytes * 8 - 1) << QSPI_CMD1_BITLEN_SHIFT);

		/* Need to stabilize other reg bits before GO bit set.
		 * As per the TRM:
		 * "For successful operation at various freq combinations,
		 * a minimum of 4-5 spi_clk cycle delay might be required
		 * before enabling the PIO or DMA bits. The worst case delay
		 * calculation can be done considering slowest qspi_clk as
		 * 1MHz. Based on that 1us delay should be enough before
		 * enabling PIO or DMA." Padded another 1us for safety.
		 */
		udelay(2);
		setbits_le32(&regs->command1, QSPI_CMD1_GO);
		udelay(1);

		/*
		 * Wait for SPI transmit FIFO to empty, or to time out.
		 * The RX FIFO status will be read and cleared last
		 */
		for (tm = 0; tm < QSPI_TIMEOUT; ++tm) {
			u32 fifo_status, xfer_status;

			xfer_status = readl(&regs->xfer_status);
			if (!(xfer_status & QSPI_XFER_STS_RDY))
				continue;

			fifo_status = readl(&regs->fifo_status);
			if (fifo_status & QSPI_FIFO_STS_ERR) {
				debug("%s: got a fifo error: ", __func__);
				if (fifo_status & QSPI_FIFO_STS_TX_FIFO_OVF)
					debug("tx FIFO overflow ");
				if (fifo_status & QSPI_FIFO_STS_TX_FIFO_UNR)
					debug("tx FIFO underrun ");
				if (fifo_status & QSPI_FIFO_STS_RX_FIFO_OVF)
					debug("rx FIFO overflow ");
				if (fifo_status & QSPI_FIFO_STS_RX_FIFO_UNR)
					debug("rx FIFO underrun ");
				if (fifo_status & QSPI_FIFO_STS_TX_FIFO_FULL)
					debug("tx FIFO full ");
				if (fifo_status & QSPI_FIFO_STS_TX_FIFO_EMPTY)
					debug("tx FIFO empty ");
				if (fifo_status & QSPI_FIFO_STS_RX_FIFO_FULL)
					debug("rx FIFO full ");
				if (fifo_status & QSPI_FIFO_STS_RX_FIFO_EMPTY)
					debug("rx FIFO empty ");
				debug("\n");
				break;
			}

			if (!(fifo_status & QSPI_FIFO_STS_RX_FIFO_EMPTY)) {
				tmpdin = readl(&regs->rx_fifo);
				if (din != NULL) {
					memcpy(din, &tmpdin, bytes);
					din += bytes;
					num_bytes -= bytes;
				}
			}
			break;
		}

		if (tm >= QSPI_TIMEOUT)
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

static int tegra210_qspi_set_speed(struct udevice *bus, uint speed)
{
	struct tegra_spi_plat *plat = dev_get_plat(bus);
	struct tegra210_qspi_priv *priv = dev_get_priv(bus);

	if (speed > plat->frequency)
		speed = plat->frequency;
	priv->freq = speed;
	debug("%s: regs=%p, speed=%d\n", __func__, priv->regs, priv->freq);

	return 0;
}

static int tegra210_qspi_set_mode(struct udevice *bus, uint mode)
{
	struct tegra210_qspi_priv *priv = dev_get_priv(bus);

	priv->mode = mode;
	debug("%s: regs=%p, mode=%d\n", __func__, priv->regs, priv->mode);

	return 0;
}

static const struct dm_spi_ops tegra210_qspi_ops = {
	.claim_bus	= tegra210_qspi_claim_bus,
	.xfer		= tegra210_qspi_xfer,
	.set_speed	= tegra210_qspi_set_speed,
	.set_mode	= tegra210_qspi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id tegra210_qspi_ids[] = {
	{ .compatible = "nvidia,tegra210-qspi" },
	{ }
};

U_BOOT_DRIVER(tegra210_qspi) = {
	.name = "tegra210-qspi",
	.id = UCLASS_SPI,
	.of_match = tegra210_qspi_ids,
	.ops = &tegra210_qspi_ops,
	.of_to_plat = tegra210_qspi_of_to_plat,
	.plat_auto	= sizeof(struct tegra_spi_plat),
	.priv_auto	= sizeof(struct tegra210_qspi_priv),
	.per_child_auto	= sizeof(struct spi_slave),
	.probe = tegra210_qspi_probe,
};
