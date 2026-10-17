// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX28 SPI driver
 *
 * Copyright (C) 2019 DENX Software Engineering
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * NOTE: This driver only supports the SPI-controller chipselects,
 *       GPIO driven chipselects are not supported.
 */

#include <common.h>
#include <dm.h>
#include <dt-structs.h>
#include <cpu_func.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <memalign.h>
#include <spi.h>
#include <asm/cache.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/dma.h>

#define	MXS_SPI_MAX_TIMEOUT	1000000
#define	MXS_SPI_PORT_OFFSET	0x2000
#define MXS_SSP_CHIPSELECT_MASK		0x00300000
#define MXS_SSP_CHIPSELECT_SHIFT	20

#define MXSSSP_SMALL_TRANSFER	512

/* Base numbers of i.MX2[38] clk for ssp0 IP block */
#define MXS_SSP_IMX23_CLKID_SSP0 33
#define MXS_SSP_IMX28_CLKID_SSP0 46

struct mxs_spi_plat {
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_fsl_imx23_spi dtplat;
#endif
	s32 frequency;		/* Default clock frequency, -1 for none */
	fdt_addr_t base;        /* SPI IP block base address */
	int num_cs;             /* Number of CSes supported */
	int dma_id;             /* ID of the DMA channel */
	int clk_id;             /* ID of the SSP clock */
};

struct mxs_spi_priv {
	struct mxs_ssp_regs *regs;
	unsigned int dma_channel;
	unsigned int max_freq;
	unsigned int clk_id;
	unsigned int mode;
};

static void mxs_spi_start_xfer(struct mxs_ssp_regs *ssp_regs)
{
	writel(SSP_CTRL0_LOCK_CS, &ssp_regs->hw_ssp_ctrl0_set);
	writel(SSP_CTRL0_IGNORE_CRC, &ssp_regs->hw_ssp_ctrl0_clr);
}

static void mxs_spi_end_xfer(struct mxs_ssp_regs *ssp_regs)
{
	writel(SSP_CTRL0_LOCK_CS, &ssp_regs->hw_ssp_ctrl0_clr);
	writel(SSP_CTRL0_IGNORE_CRC, &ssp_regs->hw_ssp_ctrl0_set);
}

static int mxs_spi_xfer_pio(struct mxs_spi_priv *priv,
			    char *data, int length, int write,
			    unsigned long flags)
{
	struct mxs_ssp_regs *ssp_regs = priv->regs;

	if (flags & SPI_XFER_BEGIN)
		mxs_spi_start_xfer(ssp_regs);

	while (length--) {
		/* We transfer 1 byte */
#if defined(CONFIG_MX23)
		writel(SSP_CTRL0_XFER_COUNT_MASK, &ssp_regs->hw_ssp_ctrl0_clr);
		writel(1, &ssp_regs->hw_ssp_ctrl0_set);
#elif defined(CONFIG_MX28)
		writel(1, &ssp_regs->hw_ssp_xfer_size);
#endif

		if ((flags & SPI_XFER_END) && !length)
			mxs_spi_end_xfer(ssp_regs);

		if (write)
			writel(SSP_CTRL0_READ, &ssp_regs->hw_ssp_ctrl0_clr);
		else
			writel(SSP_CTRL0_READ, &ssp_regs->hw_ssp_ctrl0_set);

		writel(SSP_CTRL0_RUN, &ssp_regs->hw_ssp_ctrl0_set);

		if (mxs_wait_mask_set(&ssp_regs->hw_ssp_ctrl0_reg,
			SSP_CTRL0_RUN, MXS_SPI_MAX_TIMEOUT)) {
			printf("MXS SPI: Timeout waiting for start\n");
			return -ETIMEDOUT;
		}

		if (write)
			writel(*data++, &ssp_regs->hw_ssp_data);

		writel(SSP_CTRL0_DATA_XFER, &ssp_regs->hw_ssp_ctrl0_set);

		if (!write) {
			if (mxs_wait_mask_clr(&ssp_regs->hw_ssp_status_reg,
				SSP_STATUS_FIFO_EMPTY, MXS_SPI_MAX_TIMEOUT)) {
				printf("MXS SPI: Timeout waiting for data\n");
				return -ETIMEDOUT;
			}

			*data = readl(&ssp_regs->hw_ssp_data);
			data++;
		}

		if (mxs_wait_mask_clr(&ssp_regs->hw_ssp_ctrl0_reg,
			SSP_CTRL0_RUN, MXS_SPI_MAX_TIMEOUT)) {
			printf("MXS SPI: Timeout waiting for finish\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int mxs_spi_xfer_dma(struct mxs_spi_priv *priv,
			    char *data, int length, int write,
			    unsigned long flags)
{	struct mxs_ssp_regs *ssp_regs = priv->regs;
	const int xfer_max_sz = 0xff00;
	const int desc_count = DIV_ROUND_UP(length, xfer_max_sz) + 1;
	struct mxs_dma_desc *dp;
	uint32_t ctrl0;
	uint32_t cache_data_count;
	const uint32_t dstart = (uint32_t)data;
	int dmach;
	int tl;
	int ret = 0;

#if defined(CONFIG_MX23)
	const int mxs_spi_pio_words = 1;
#elif defined(CONFIG_MX28)
	const int mxs_spi_pio_words = 4;
#endif

	ALLOC_CACHE_ALIGN_BUFFER(struct mxs_dma_desc, desc, desc_count);

	memset(desc, 0, sizeof(struct mxs_dma_desc) * desc_count);

	ctrl0 = readl(&ssp_regs->hw_ssp_ctrl0);
	ctrl0 |= SSP_CTRL0_DATA_XFER;

	if (flags & SPI_XFER_BEGIN)
		ctrl0 |= SSP_CTRL0_LOCK_CS;
	if (!write)
		ctrl0 |= SSP_CTRL0_READ;

	if (length % ARCH_DMA_MINALIGN)
		cache_data_count = roundup(length, ARCH_DMA_MINALIGN);
	else
		cache_data_count = length;

	/* Flush data to DRAM so DMA can pick them up */
	if (write)
		flush_dcache_range(dstart, dstart + cache_data_count);

	/* Invalidate the area, so no writeback into the RAM races with DMA */
	invalidate_dcache_range(dstart, dstart + cache_data_count);

	dmach = priv->dma_channel;

	dp = desc;
	while (length) {
		dp->address = (dma_addr_t)dp;
		dp->cmd.address = (dma_addr_t)data;

		/*
		 * This is correct, even though it does indeed look insane.
		 * I hereby have to, wholeheartedly, thank Freescale Inc.,
		 * for always inventing insane hardware and keeping me busy
		 * and employed ;-)
		 */
		if (write)
			dp->cmd.data = MXS_DMA_DESC_COMMAND_DMA_READ;
		else
			dp->cmd.data = MXS_DMA_DESC_COMMAND_DMA_WRITE;

		/*
		 * The DMA controller can transfer large chunks (64kB) at
		 * time by setting the transfer length to 0. Setting tl to
		 * 0x10000 will overflow below and make .data contain 0.
		 * Otherwise, 0xff00 is the transfer maximum.
		 */
		if (length >= 0x10000)
			tl = 0x10000;
		else
			tl = min(length, xfer_max_sz);

		dp->cmd.data |=
			((tl & 0xffff) << MXS_DMA_DESC_BYTES_OFFSET) |
			(mxs_spi_pio_words << MXS_DMA_DESC_PIO_WORDS_OFFSET) |
			MXS_DMA_DESC_HALT_ON_TERMINATE |
			MXS_DMA_DESC_TERMINATE_FLUSH;

		data += tl;
		length -= tl;

		if (!length) {
			dp->cmd.data |= MXS_DMA_DESC_IRQ | MXS_DMA_DESC_DEC_SEM;

			if (flags & SPI_XFER_END) {
				ctrl0 &= ~SSP_CTRL0_LOCK_CS;
				ctrl0 |= SSP_CTRL0_IGNORE_CRC;
			}
		}

		/*
		 * Write CTRL0, CMD0, CMD1 and XFER_SIZE registers in
		 * case of MX28, write only CTRL0 in case of MX23 due
		 * to the difference in register layout. It is utterly
		 * essential that the XFER_SIZE register is written on
		 * a per-descriptor basis with the same size as is the
		 * descriptor!
		 */
		dp->cmd.pio_words[0] = ctrl0;
#ifdef CONFIG_MX28
		dp->cmd.pio_words[1] = 0;
		dp->cmd.pio_words[2] = 0;
		dp->cmd.pio_words[3] = tl;
#endif

		mxs_dma_desc_append(dmach, dp);

		dp++;
	}

	if (mxs_dma_go(dmach))
		ret = -EINVAL;

	/* The data arrived into DRAM, invalidate cache over them */
	if (!write)
		invalidate_dcache_range(dstart, dstart + cache_data_count);

	return ret;
}

int mxs_spi_xfer(struct udevice *dev, unsigned int bitlen,
		 const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct mxs_spi_priv *priv = dev_get_priv(bus);
	struct mxs_ssp_regs *ssp_regs = priv->regs;
	int len = bitlen / 8;
	char dummy;
	int write = 0;
	char *data = NULL;
	int dma = 1;

	if (bitlen == 0) {
		if (flags & SPI_XFER_END) {
			din = (void *)&dummy;
			len = 1;
		} else
			return 0;
	}

	/* Half-duplex only */
	if (din && dout)
		return -EINVAL;
	/* No data */
	if (!din && !dout)
		return 0;

	if (dout) {
		data = (char *)dout;
		write = 1;
	} else if (din) {
		data = (char *)din;
		write = 0;
	}

	/*
	 * Check for alignment, if the buffer is aligned, do DMA transfer,
	 * PIO otherwise. This is a temporary workaround until proper bounce
	 * buffer is in place.
	 */
	if (dma) {
		if (((uint32_t)data) & (ARCH_DMA_MINALIGN - 1))
			dma = 0;
		if (((uint32_t)len) & (ARCH_DMA_MINALIGN - 1))
			dma = 0;
	}

	if (!dma || (len < MXSSSP_SMALL_TRANSFER)) {
		writel(SSP_CTRL1_DMA_ENABLE, &ssp_regs->hw_ssp_ctrl1_clr);
		return mxs_spi_xfer_pio(priv, data, len, write, flags);
	} else {
		writel(SSP_CTRL1_DMA_ENABLE, &ssp_regs->hw_ssp_ctrl1_set);
		return mxs_spi_xfer_dma(priv, data, len, write, flags);
	}
}

static int mxs_spi_probe(struct udevice *bus)
{
	struct mxs_spi_plat *plat = dev_get_plat(bus);
	struct mxs_spi_priv *priv = dev_get_priv(bus);
	int ret;

	debug("%s: probe\n", __func__);

#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct dtd_fsl_imx23_spi *dtplat = &plat->dtplat;
	struct phandle_1_arg *p1a = &dtplat->clocks[0];

	priv->regs = (struct mxs_ssp_regs *)dtplat->reg[0];
	priv->dma_channel = dtplat->dmas[1];
	priv->clk_id = p1a->arg[0];
	priv->max_freq = dtplat->spi_max_frequency;
	plat->num_cs = dtplat->num_cs;

	debug("OF_PLATDATA: regs: 0x%x max freq: %d clkid: %d\n",
	      (unsigned int)priv->regs, priv->max_freq, priv->clk_id);
#else
	priv->regs = (struct mxs_ssp_regs *)plat->base;
	priv->max_freq = plat->frequency;

	priv->dma_channel = plat->dma_id;
	priv->clk_id = plat->clk_id;
#endif

	mxs_reset_block(&priv->regs->hw_ssp_ctrl0_reg);

	ret = mxs_dma_init_channel(priv->dma_channel);
	if (ret) {
		printf("%s: DMA init channel error %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

static int mxs_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct mxs_spi_priv *priv = dev_get_priv(bus);
	struct mxs_ssp_regs *ssp_regs = priv->regs;
	int cs = spi_chip_select(dev);

	/*
	 * i.MX28 supports up to 3 CS (SSn0, SSn1, SSn2)
	 * To set them it uses following tuple (WAIT_FOR_IRQ,WAIT_FOR_CMD),
	 * where:
	 *
	 * WAIT_FOR_IRQ is bit 21 of HW_SSP_CTRL0
	 * WAIT_FOR_CMD is bit 20 (#defined as MXS_SSP_CHIPSELECT_SHIFT here) of
	 *                        HW_SSP_CTRL0
	 * SSn0 b00
	 * SSn1 b01
	 * SSn2 b10 (which require setting WAIT_FOR_IRQ)
	 *
	 * However, for now i.MX28 SPI driver will support up till 2 CSes
	 * (SSn0, and SSn1).
	 */

	/* Ungate SSP clock and set active CS */
	clrsetbits_le32(&ssp_regs->hw_ssp_ctrl0,
			BIT(MXS_SSP_CHIPSELECT_SHIFT) |
			SSP_CTRL0_CLKGATE, (cs << MXS_SSP_CHIPSELECT_SHIFT));

	return 0;
}

static int mxs_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct mxs_spi_priv *priv = dev_get_priv(bus);
	struct mxs_ssp_regs *ssp_regs = priv->regs;

	/* Gate SSP clock */
	setbits_le32(&ssp_regs->hw_ssp_ctrl0, SSP_CTRL0_CLKGATE);

	return 0;
}

static int mxs_spi_set_speed(struct udevice *bus, uint speed)
{
	struct mxs_spi_priv *priv = dev_get_priv(bus);
#ifdef CONFIG_MX28
	int clkid = priv->clk_id - MXS_SSP_IMX28_CLKID_SSP0;
#else /* CONFIG_MX23 */
	int clkid = priv->clk_id - MXS_SSP_IMX23_CLKID_SSP0;
#endif
	if (speed > priv->max_freq)
		speed = priv->max_freq;

	debug("%s speed: %u [Hz] clkid: %d\n", __func__, speed, clkid);
	mxs_set_ssp_busclock(clkid, speed / 1000);

	return 0;
}

static int mxs_spi_set_mode(struct udevice *bus, uint mode)
{
	struct mxs_spi_priv *priv = dev_get_priv(bus);
	struct mxs_ssp_regs *ssp_regs = priv->regs;
	u32 reg;

	priv->mode = mode;
	debug("%s: mode 0x%x\n", __func__, mode);

	reg = SSP_CTRL1_SSP_MODE_SPI | SSP_CTRL1_WORD_LENGTH_EIGHT_BITS;
	reg |= (priv->mode & SPI_CPOL) ? SSP_CTRL1_POLARITY : 0;
	reg |= (priv->mode & SPI_CPHA) ? SSP_CTRL1_PHASE : 0;
	writel(reg, &ssp_regs->hw_ssp_ctrl1);

	/* Single bit SPI support */
	writel(SSP_CTRL0_BUS_WIDTH_ONE_BIT, &ssp_regs->hw_ssp_ctrl0);

	return 0;
}

static const struct dm_spi_ops mxs_spi_ops = {
	.claim_bus	= mxs_spi_claim_bus,
	.release_bus    = mxs_spi_release_bus,
	.xfer		= mxs_spi_xfer,
	.set_speed	= mxs_spi_set_speed,
	.set_mode	= mxs_spi_set_mode,
	/*
	 * cs_info is not needed, since we require all chip selects to be
	 * in the device tree explicitly
	 */
};

#if CONFIG_IS_ENABLED(OF_REAL)
static int mxs_of_to_plat(struct udevice *bus)
{
	struct mxs_spi_plat *plat = dev_get_plat(bus);
	u32 prop[2];
	int ret;

	plat->base = dev_read_addr(bus);
	plat->frequency =
		dev_read_u32_default(bus, "spi-max-frequency", 40000000);
	plat->num_cs = dev_read_u32_default(bus, "num-cs", 2);

	ret = dev_read_u32_array(bus, "dmas", prop, ARRAY_SIZE(prop));
	if (ret) {
		printf("%s: Reading 'dmas' property failed!\n", __func__);
		return ret;
	}
	plat->dma_id = prop[1];

	ret = dev_read_u32_array(bus, "clocks", prop, ARRAY_SIZE(prop));
	if (ret) {
		printf("%s: Reading 'clocks' property failed!\n", __func__);
		return ret;
	}
	plat->clk_id = prop[1];

	debug("%s: base=0x%x, max-frequency=%d num-cs=%d dma_id=%d clk_id=%d\n",
	      __func__, (uint)plat->base, plat->frequency, plat->num_cs,
	      plat->dma_id, plat->clk_id);

	return 0;
}

static const struct udevice_id mxs_spi_ids[] = {
	{ .compatible = "fsl,imx23-spi" },
	{ .compatible = "fsl,imx28-spi" },
	{ }
};
#endif

U_BOOT_DRIVER(fsl_imx23_spi) = {
	.name = "fsl_imx23_spi",
	.id	= UCLASS_SPI,
#if CONFIG_IS_ENABLED(OF_REAL)
	.of_match = mxs_spi_ids,
	.of_to_plat = mxs_of_to_plat,
#endif
	.plat_auto	= sizeof(struct mxs_spi_plat),
	.ops	= &mxs_spi_ops,
	.priv_auto	= sizeof(struct mxs_spi_priv),
	.probe	= mxs_spi_probe,
};

DM_DRIVER_ALIAS(fsl_imx23_spi, fsl_imx28_spi)
