// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Padmavathi Venna <padma.v@samsung.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <spi.h>
#include <fdtdec.h>
#include <asm/arch/clk.h>
#include <asm/arch/clock.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/spi.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

struct exynos_spi_platdata {
	enum periph_id periph_id;
	s32 frequency;		/* Default clock frequency, -1 for none */
	struct exynos_spi *regs;
	uint deactivate_delay_us;	/* Delay to wait after deactivate */
};

struct exynos_spi_priv {
	struct exynos_spi *regs;
	unsigned int freq;		/* Default frequency */
	unsigned int mode;
	enum periph_id periph_id;	/* Peripheral ID for this device */
	unsigned int fifo_size;
	int skip_preamble;
	ulong last_transaction_us;	/* Time of last transaction end */
};

/**
 * Flush spi tx, rx fifos and reset the SPI controller
 *
 * @param regs	Pointer to SPI registers
 */
static void spi_flush_fifo(struct exynos_spi *regs)
{
	clrsetbits_le32(&regs->ch_cfg, SPI_CH_HS_EN, SPI_CH_RST);
	clrbits_le32(&regs->ch_cfg, SPI_CH_RST);
	setbits_le32(&regs->ch_cfg, SPI_TX_CH_ON | SPI_RX_CH_ON);
}

static void spi_get_fifo_levels(struct exynos_spi *regs,
	int *rx_lvl, int *tx_lvl)
{
	uint32_t spi_sts = readl(&regs->spi_sts);

	*rx_lvl = (spi_sts >> SPI_RX_LVL_OFFSET) & SPI_FIFO_LVL_MASK;
	*tx_lvl = (spi_sts >> SPI_TX_LVL_OFFSET) & SPI_FIFO_LVL_MASK;
}

/**
 * If there's something to transfer, do a software reset and set a
 * transaction size.
 *
 * @param regs	SPI peripheral registers
 * @param count	Number of bytes to transfer
 * @param step	Number of bytes to transfer in each packet (1 or 4)
 */
static void spi_request_bytes(struct exynos_spi *regs, int count, int step)
{
	debug("%s: regs=%p, count=%d, step=%d\n", __func__, regs, count, step);

	/* For word address we need to swap bytes */
	if (step == 4) {
		setbits_le32(&regs->mode_cfg,
			     SPI_MODE_CH_WIDTH_WORD | SPI_MODE_BUS_WIDTH_WORD);
		count /= 4;
		setbits_le32(&regs->swap_cfg, SPI_TX_SWAP_EN | SPI_RX_SWAP_EN |
			SPI_TX_BYTE_SWAP | SPI_RX_BYTE_SWAP |
			SPI_TX_HWORD_SWAP | SPI_RX_HWORD_SWAP);
	} else {
		/* Select byte access and clear the swap configuration */
		clrbits_le32(&regs->mode_cfg,
			     SPI_MODE_CH_WIDTH_WORD | SPI_MODE_BUS_WIDTH_WORD);
		writel(0, &regs->swap_cfg);
	}

	assert(count && count < (1 << 16));
	setbits_le32(&regs->ch_cfg, SPI_CH_RST);
	clrbits_le32(&regs->ch_cfg, SPI_CH_RST);

	writel(count | SPI_PACKET_CNT_EN, &regs->pkt_cnt);
}

static int spi_rx_tx(struct exynos_spi_priv *priv, int todo,
			void **dinp, void const **doutp, unsigned long flags)
{
	struct exynos_spi *regs = priv->regs;
	uchar *rxp = *dinp;
	const uchar *txp = *doutp;
	int rx_lvl, tx_lvl;
	uint out_bytes, in_bytes;
	int toread;
	unsigned start = get_timer(0);
	int stopping;
	int step;

	out_bytes = in_bytes = todo;

	stopping = priv->skip_preamble && (flags & SPI_XFER_END) &&
					!(priv->mode & SPI_SLAVE);

	/*
	 * Try to transfer words if we can. This helps read performance at
	 * SPI clock speeds above about 20MHz.
	 */
	step = 1;
	if (!((todo | (uintptr_t)rxp | (uintptr_t)txp) & 3) &&
	    !priv->skip_preamble)
		step = 4;

	/*
	 * If there's something to send, do a software reset and set a
	 * transaction size.
	 */
	spi_request_bytes(regs, todo, step);

	/*
	 * Bytes are transmitted/received in pairs. Wait to receive all the
	 * data because then transmission will be done as well.
	 */
	toread = in_bytes;

	while (in_bytes) {
		int temp;

		/* Keep the fifos full/empty. */
		spi_get_fifo_levels(regs, &rx_lvl, &tx_lvl);

		/*
		 * Don't completely fill the txfifo, since we don't want our
		 * rxfifo to overflow, and it may already contain data.
		 */
		while (tx_lvl < priv->fifo_size/2 && out_bytes) {
			if (!txp)
				temp = -1;
			else if (step == 4)
				temp = *(uint32_t *)txp;
			else
				temp = *txp;
			writel(temp, &regs->tx_data);
			out_bytes -= step;
			if (txp)
				txp += step;
			tx_lvl += step;
		}
		if (rx_lvl >= step) {
			while (rx_lvl >= step) {
				temp = readl(&regs->rx_data);
				if (priv->skip_preamble) {
					if (temp == SPI_PREAMBLE_END_BYTE) {
						priv->skip_preamble = 0;
						stopping = 0;
					}
				} else {
					if (rxp || stopping) {
						if (step == 4)
							*(uint32_t *)rxp = temp;
						else
							*rxp = temp;
						rxp += step;
					}
					in_bytes -= step;
				}
				toread -= step;
				rx_lvl -= step;
			}
		} else if (!toread) {
			/*
			 * We have run out of input data, but haven't read
			 * enough bytes after the preamble yet. Read some more,
			 * and make sure that we transmit dummy bytes too, to
			 * keep things going.
			 */
			assert(!out_bytes);
			out_bytes = in_bytes;
			toread = in_bytes;
			txp = NULL;
			spi_request_bytes(regs, toread, step);
		}
		if (priv->skip_preamble && get_timer(start) > 100) {
			debug("SPI timeout: in_bytes=%d, out_bytes=%d, ",
			      in_bytes, out_bytes);
			return -ETIMEDOUT;
		}
	}

	*dinp = rxp;
	*doutp = txp;

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
	struct exynos_spi_platdata *pdata = dev_get_platdata(bus);
	struct exynos_spi_priv *priv = dev_get_priv(bus);

	/* If it's too soon to do another transaction, wait */
	if (pdata->deactivate_delay_us &&
	    priv->last_transaction_us) {
		ulong delay_us;		/* The delay completed so far */
		delay_us = timer_get_us() - priv->last_transaction_us;
		if (delay_us < pdata->deactivate_delay_us)
			udelay(pdata->deactivate_delay_us - delay_us);
	}

	clrbits_le32(&priv->regs->cs_reg, SPI_SLAVE_SIG_INACT);
	debug("Activate CS, bus '%s'\n", bus->name);
	priv->skip_preamble = priv->mode & SPI_PREAMBLE;
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
	struct exynos_spi_platdata *pdata = dev_get_platdata(bus);
	struct exynos_spi_priv *priv = dev_get_priv(bus);

	setbits_le32(&priv->regs->cs_reg, SPI_SLAVE_SIG_INACT);

	/* Remember time of this transaction so we can honour the bus delay */
	if (pdata->deactivate_delay_us)
		priv->last_transaction_us = timer_get_us();

	debug("Deactivate CS, bus '%s'\n", bus->name);
}

static int exynos_spi_ofdata_to_platdata(struct udevice *bus)
{
	struct exynos_spi_platdata *plat = bus->platdata;
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(bus);

	plat->regs = (struct exynos_spi *)devfdt_get_addr(bus);
	plat->periph_id = pinmux_decode_periph_id(blob, node);

	if (plat->periph_id == PERIPH_ID_NONE) {
		debug("%s: Invalid peripheral ID %d\n", __func__,
			plat->periph_id);
		return -FDT_ERR_NOTFOUND;
	}

	/* Use 500KHz as a suitable default */
	plat->frequency = fdtdec_get_int(blob, node, "spi-max-frequency",
					500000);
	plat->deactivate_delay_us = fdtdec_get_int(blob, node,
					"spi-deactivate-delay", 0);
	debug("%s: regs=%p, periph_id=%d, max-frequency=%d, deactivate_delay=%d\n",
	      __func__, plat->regs, plat->periph_id, plat->frequency,
	      plat->deactivate_delay_us);

	return 0;
}

static int exynos_spi_probe(struct udevice *bus)
{
	struct exynos_spi_platdata *plat = dev_get_platdata(bus);
	struct exynos_spi_priv *priv = dev_get_priv(bus);

	priv->regs = plat->regs;
	if (plat->periph_id == PERIPH_ID_SPI1 ||
	    plat->periph_id == PERIPH_ID_SPI2)
		priv->fifo_size = 64;
	else
		priv->fifo_size = 256;

	priv->skip_preamble = 0;
	priv->last_transaction_us = timer_get_us();
	priv->freq = plat->frequency;
	priv->periph_id = plat->periph_id;

	return 0;
}

static int exynos_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct exynos_spi_priv *priv = dev_get_priv(bus);

	exynos_pinmux_config(priv->periph_id, PINMUX_FLAG_NONE);
	spi_flush_fifo(priv->regs);

	writel(SPI_FB_DELAY_180, &priv->regs->fb_clk);

	return 0;
}

static int exynos_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct exynos_spi_priv *priv = dev_get_priv(bus);

	spi_flush_fifo(priv->regs);

	return 0;
}

static int exynos_spi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct exynos_spi_priv *priv = dev_get_priv(bus);
	int upto, todo;
	int bytelen;
	int ret = 0;

	/* spi core configured to do 8 bit transfers */
	if (bitlen % 8) {
		debug("Non byte aligned SPI transfer.\n");
		return -1;
	}

	/* Start the transaction, if necessary. */
	if ((flags & SPI_XFER_BEGIN))
		spi_cs_activate(dev);

	/*
	 * Exynos SPI limits each transfer to 65535 transfers. To keep
	 * things simple, allow a maximum of 65532 bytes. We could allow
	 * more in word mode, but the performance difference is small.
	 */
	bytelen = bitlen / 8;
	for (upto = 0; !ret && upto < bytelen; upto += todo) {
		todo = min(bytelen - upto, (1 << 16) - 4);
		ret = spi_rx_tx(priv, todo, &din, &dout, flags);
		if (ret)
			break;
	}

	/* Stop the transaction, if necessary. */
	if ((flags & SPI_XFER_END) && !(priv->mode & SPI_SLAVE)) {
		spi_cs_deactivate(dev);
		if (priv->skip_preamble) {
			assert(!priv->skip_preamble);
			debug("Failed to complete premable transaction\n");
			ret = -1;
		}
	}

	return ret;
}

static int exynos_spi_set_speed(struct udevice *bus, uint speed)
{
	struct exynos_spi_platdata *plat = bus->platdata;
	struct exynos_spi_priv *priv = dev_get_priv(bus);
	int ret;

	if (speed > plat->frequency)
		speed = plat->frequency;
	ret = set_spi_clk(priv->periph_id, speed);
	if (ret)
		return ret;
	priv->freq = speed;
	debug("%s: regs=%p, speed=%d\n", __func__, priv->regs, priv->freq);

	return 0;
}

static int exynos_spi_set_mode(struct udevice *bus, uint mode)
{
	struct exynos_spi_priv *priv = dev_get_priv(bus);
	uint32_t reg;

	reg = readl(&priv->regs->ch_cfg);
	reg &= ~(SPI_CH_CPHA_B | SPI_CH_CPOL_L);

	if (mode & SPI_CPHA)
		reg |= SPI_CH_CPHA_B;

	if (mode & SPI_CPOL)
		reg |= SPI_CH_CPOL_L;

	writel(reg, &priv->regs->ch_cfg);
	priv->mode = mode;
	debug("%s: regs=%p, mode=%d\n", __func__, priv->regs, priv->mode);

	return 0;
}

static const struct dm_spi_ops exynos_spi_ops = {
	.claim_bus	= exynos_spi_claim_bus,
	.release_bus	= exynos_spi_release_bus,
	.xfer		= exynos_spi_xfer,
	.set_speed	= exynos_spi_set_speed,
	.set_mode	= exynos_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

static const struct udevice_id exynos_spi_ids[] = {
	{ .compatible = "samsung,exynos-spi" },
	{ }
};

U_BOOT_DRIVER(exynos_spi) = {
	.name	= "exynos_spi",
	.id	= UCLASS_SPI,
	.of_match = exynos_spi_ids,
	.ops	= &exynos_spi_ops,
	.ofdata_to_platdata = exynos_spi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct exynos_spi_platdata),
	.priv_auto_alloc_size = sizeof(struct exynos_spi_priv),
	.probe	= exynos_spi_probe,
};
