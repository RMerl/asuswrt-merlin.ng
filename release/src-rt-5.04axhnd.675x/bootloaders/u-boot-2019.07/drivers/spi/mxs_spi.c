// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale i.MX28 SPI driver
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * NOTE: This driver only supports the SPI-controller chipselects,
 *       GPIO driven chipselects are not supported.
 */

#include <common.h>
#include <malloc.h>
#include <memalign.h>
#include <spi.h>
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

struct mxs_spi_slave {
	struct spi_slave	slave;
	uint32_t		max_khz;
	uint32_t		mode;
	struct mxs_ssp_regs	*regs;
};

static inline struct mxs_spi_slave *to_mxs_slave(struct spi_slave *slave)
{
	return container_of(slave, struct mxs_spi_slave, slave);
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	/* MXS SPI: 4 ports and 3 chip selects maximum */
	if (!mxs_ssp_bus_id_valid(bus) || cs > 2)
		return 0;
	else
		return 1;
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
				  unsigned int max_hz, unsigned int mode)
{
	struct mxs_spi_slave *mxs_slave;

	if (!spi_cs_is_valid(bus, cs)) {
		printf("mxs_spi: invalid bus %d / chip select %d\n", bus, cs);
		return NULL;
	}

	mxs_slave = spi_alloc_slave(struct mxs_spi_slave, bus, cs);
	if (!mxs_slave)
		return NULL;

	if (mxs_dma_init_channel(MXS_DMA_CHANNEL_AHB_APBH_SSP0 + bus))
		goto err_init;

	mxs_slave->max_khz = max_hz / 1000;
	mxs_slave->mode = mode;
	mxs_slave->regs = mxs_ssp_regs_by_bus(bus);

	return &mxs_slave->slave;

err_init:
	free(mxs_slave);
	return NULL;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct mxs_spi_slave *mxs_slave = to_mxs_slave(slave);
	free(mxs_slave);
}

int spi_claim_bus(struct spi_slave *slave)
{
	struct mxs_spi_slave *mxs_slave = to_mxs_slave(slave);
	struct mxs_ssp_regs *ssp_regs = mxs_slave->regs;
	uint32_t reg = 0;

	mxs_reset_block(&ssp_regs->hw_ssp_ctrl0_reg);

	writel((slave->cs << MXS_SSP_CHIPSELECT_SHIFT) |
	       SSP_CTRL0_BUS_WIDTH_ONE_BIT,
	       &ssp_regs->hw_ssp_ctrl0);

	reg = SSP_CTRL1_SSP_MODE_SPI | SSP_CTRL1_WORD_LENGTH_EIGHT_BITS;
	reg |= (mxs_slave->mode & SPI_CPOL) ? SSP_CTRL1_POLARITY : 0;
	reg |= (mxs_slave->mode & SPI_CPHA) ? SSP_CTRL1_PHASE : 0;
	writel(reg, &ssp_regs->hw_ssp_ctrl1);

	writel(0, &ssp_regs->hw_ssp_cmd0);

	mxs_set_ssp_busclock(slave->bus, mxs_slave->max_khz);

	return 0;
}

void spi_release_bus(struct spi_slave *slave)
{
}

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

static int mxs_spi_xfer_pio(struct mxs_spi_slave *slave,
			char *data, int length, int write, unsigned long flags)
{
	struct mxs_ssp_regs *ssp_regs = slave->regs;

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

static int mxs_spi_xfer_dma(struct mxs_spi_slave *slave,
			char *data, int length, int write, unsigned long flags)
{
	const int xfer_max_sz = 0xff00;
	const int desc_count = DIV_ROUND_UP(length, xfer_max_sz) + 1;
	struct mxs_ssp_regs *ssp_regs = slave->regs;
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

	dmach = MXS_DMA_CHANNEL_AHB_APBH_SSP0 + slave->slave.bus;

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

int spi_xfer(struct spi_slave *slave, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct mxs_spi_slave *mxs_slave = to_mxs_slave(slave);
	struct mxs_ssp_regs *ssp_regs = mxs_slave->regs;
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
		return mxs_spi_xfer_pio(mxs_slave, data, len, write, flags);
	} else {
		writel(SSP_CTRL1_DMA_ENABLE, &ssp_regs->hw_ssp_ctrl1_set);
		return mxs_spi_xfer_dma(mxs_slave, data, len, write, flags);
	}
}
