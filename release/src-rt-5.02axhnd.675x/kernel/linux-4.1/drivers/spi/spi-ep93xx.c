/*
 * Driver for Cirrus Logic EP93xx SPI controller.
 *
 * Copyright (C) 2010-2011 Mika Westerberg
 *
 * Explicit FIFO handling code was inspired by amba-pl022 driver.
 *
 * Chip select support using other than built-in GPIOs by H. Hartley Sweeten.
 *
 * For more information about the SPI controller see documentation on Cirrus
 * Logic web site:
 *     http://www.cirrus.com/en/pubs/manual/EP93xx_Users_Guide_UM1.pdf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/io.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dmaengine.h>
#include <linux/bitops.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/scatterlist.h>
#include <linux/spi/spi.h>

#include <linux/platform_data/dma-ep93xx.h>
#include <linux/platform_data/spi-ep93xx.h>

#define SSPCR0			0x0000
#define SSPCR0_MODE_SHIFT	6
#define SSPCR0_SCR_SHIFT	8

#define SSPCR1			0x0004
#define SSPCR1_RIE		BIT(0)
#define SSPCR1_TIE		BIT(1)
#define SSPCR1_RORIE		BIT(2)
#define SSPCR1_LBM		BIT(3)
#define SSPCR1_SSE		BIT(4)
#define SSPCR1_MS		BIT(5)
#define SSPCR1_SOD		BIT(6)

#define SSPDR			0x0008

#define SSPSR			0x000c
#define SSPSR_TFE		BIT(0)
#define SSPSR_TNF		BIT(1)
#define SSPSR_RNE		BIT(2)
#define SSPSR_RFF		BIT(3)
#define SSPSR_BSY		BIT(4)
#define SSPCPSR			0x0010

#define SSPIIR			0x0014
#define SSPIIR_RIS		BIT(0)
#define SSPIIR_TIS		BIT(1)
#define SSPIIR_RORIS		BIT(2)
#define SSPICR			SSPIIR

/* timeout in milliseconds */
#define SPI_TIMEOUT		5
/* maximum depth of RX/TX FIFO */
#define SPI_FIFO_SIZE		8

/**
 * struct ep93xx_spi - EP93xx SPI controller structure
 * @pdev: pointer to platform device
 * @clk: clock for the controller
 * @regs_base: pointer to ioremap()'d registers
 * @sspdr_phys: physical address of the SSPDR register
 * @wait: wait here until given transfer is completed
 * @current_msg: message that is currently processed (or %NULL if none)
 * @tx: current byte in transfer to transmit
 * @rx: current byte in transfer to receive
 * @fifo_level: how full is FIFO (%0..%SPI_FIFO_SIZE - %1). Receiving one
 *              frame decreases this level and sending one frame increases it.
 * @dma_rx: RX DMA channel
 * @dma_tx: TX DMA channel
 * @dma_rx_data: RX parameters passed to the DMA engine
 * @dma_tx_data: TX parameters passed to the DMA engine
 * @rx_sgt: sg table for RX transfers
 * @tx_sgt: sg table for TX transfers
 * @zeropage: dummy page used as RX buffer when only TX buffer is passed in by
 *            the client
 */
struct ep93xx_spi {
	const struct platform_device	*pdev;
	struct clk			*clk;
	void __iomem			*regs_base;
	unsigned long			sspdr_phys;
	struct completion		wait;
	struct spi_message		*current_msg;
	size_t				tx;
	size_t				rx;
	size_t				fifo_level;
	struct dma_chan			*dma_rx;
	struct dma_chan			*dma_tx;
	struct ep93xx_dma_data		dma_rx_data;
	struct ep93xx_dma_data		dma_tx_data;
	struct sg_table			rx_sgt;
	struct sg_table			tx_sgt;
	void				*zeropage;
};

/**
 * struct ep93xx_spi_chip - SPI device hardware settings
 * @spi: back pointer to the SPI device
 * @ops: private chip operations
 */
struct ep93xx_spi_chip {
	const struct spi_device		*spi;
	struct ep93xx_spi_chip_ops	*ops;
};

/* converts bits per word to CR0.DSS value */
#define bits_per_word_to_dss(bpw)	((bpw) - 1)

static void ep93xx_spi_write_u8(const struct ep93xx_spi *espi,
				u16 reg, u8 value)
{
	writeb(value, espi->regs_base + reg);
}

static u8 ep93xx_spi_read_u8(const struct ep93xx_spi *spi, u16 reg)
{
	return readb(spi->regs_base + reg);
}

static void ep93xx_spi_write_u16(const struct ep93xx_spi *espi,
				 u16 reg, u16 value)
{
	writew(value, espi->regs_base + reg);
}

static u16 ep93xx_spi_read_u16(const struct ep93xx_spi *spi, u16 reg)
{
	return readw(spi->regs_base + reg);
}

static int ep93xx_spi_enable(const struct ep93xx_spi *espi)
{
	u8 regval;
	int err;

	err = clk_enable(espi->clk);
	if (err)
		return err;

	regval = ep93xx_spi_read_u8(espi, SSPCR1);
	regval |= SSPCR1_SSE;
	ep93xx_spi_write_u8(espi, SSPCR1, regval);

	return 0;
}

static void ep93xx_spi_disable(const struct ep93xx_spi *espi)
{
	u8 regval;

	regval = ep93xx_spi_read_u8(espi, SSPCR1);
	regval &= ~SSPCR1_SSE;
	ep93xx_spi_write_u8(espi, SSPCR1, regval);

	clk_disable(espi->clk);
}

static void ep93xx_spi_enable_interrupts(const struct ep93xx_spi *espi)
{
	u8 regval;

	regval = ep93xx_spi_read_u8(espi, SSPCR1);
	regval |= (SSPCR1_RORIE | SSPCR1_TIE | SSPCR1_RIE);
	ep93xx_spi_write_u8(espi, SSPCR1, regval);
}

static void ep93xx_spi_disable_interrupts(const struct ep93xx_spi *espi)
{
	u8 regval;

	regval = ep93xx_spi_read_u8(espi, SSPCR1);
	regval &= ~(SSPCR1_RORIE | SSPCR1_TIE | SSPCR1_RIE);
	ep93xx_spi_write_u8(espi, SSPCR1, regval);
}

/**
 * ep93xx_spi_calc_divisors() - calculates SPI clock divisors
 * @espi: ep93xx SPI controller struct
 * @rate: desired SPI output clock rate
 * @div_cpsr: pointer to return the cpsr (pre-scaler) divider
 * @div_scr: pointer to return the scr divider
 */
static int ep93xx_spi_calc_divisors(const struct ep93xx_spi *espi,
				    u32 rate, u8 *div_cpsr, u8 *div_scr)
{
	struct spi_master *master = platform_get_drvdata(espi->pdev);
	unsigned long spi_clk_rate = clk_get_rate(espi->clk);
	int cpsr, scr;

	/*
	 * Make sure that max value is between values supported by the
	 * controller. Note that minimum value is already checked in
	 * ep93xx_spi_transfer_one_message().
	 */
	rate = clamp(rate, master->min_speed_hz, master->max_speed_hz);

	/*
	 * Calculate divisors so that we can get speed according the
	 * following formula:
	 *	rate = spi_clock_rate / (cpsr * (1 + scr))
	 *
	 * cpsr must be even number and starts from 2, scr can be any number
	 * between 0 and 255.
	 */
	for (cpsr = 2; cpsr <= 254; cpsr += 2) {
		for (scr = 0; scr <= 255; scr++) {
			if ((spi_clk_rate / (cpsr * (scr + 1))) <= rate) {
				*div_scr = (u8)scr;
				*div_cpsr = (u8)cpsr;
				return 0;
			}
		}
	}

	return -EINVAL;
}

static void ep93xx_spi_cs_control(struct spi_device *spi, bool control)
{
	struct ep93xx_spi_chip *chip = spi_get_ctldata(spi);
	int value = (spi->mode & SPI_CS_HIGH) ? control : !control;

	if (chip->ops && chip->ops->cs_control)
		chip->ops->cs_control(spi, value);
}

/**
 * ep93xx_spi_setup() - setup an SPI device
 * @spi: SPI device to setup
 *
 * This function sets up SPI device mode, speed etc. Can be called multiple
 * times for a single device. Returns %0 in case of success, negative error in
 * case of failure. When this function returns success, the device is
 * deselected.
 */
static int ep93xx_spi_setup(struct spi_device *spi)
{
	struct ep93xx_spi *espi = spi_master_get_devdata(spi->master);
	struct ep93xx_spi_chip *chip;

	chip = spi_get_ctldata(spi);
	if (!chip) {
		dev_dbg(&espi->pdev->dev, "initial setup for %s\n",
			spi->modalias);

		chip = kzalloc(sizeof(*chip), GFP_KERNEL);
		if (!chip)
			return -ENOMEM;

		chip->spi = spi;
		chip->ops = spi->controller_data;

		if (chip->ops && chip->ops->setup) {
			int ret = chip->ops->setup(spi);

			if (ret) {
				kfree(chip);
				return ret;
			}
		}

		spi_set_ctldata(spi, chip);
	}

	ep93xx_spi_cs_control(spi, false);
	return 0;
}

/**
 * ep93xx_spi_cleanup() - cleans up master controller specific state
 * @spi: SPI device to cleanup
 *
 * This function releases master controller specific state for given @spi
 * device.
 */
static void ep93xx_spi_cleanup(struct spi_device *spi)
{
	struct ep93xx_spi_chip *chip;

	chip = spi_get_ctldata(spi);
	if (chip) {
		if (chip->ops && chip->ops->cleanup)
			chip->ops->cleanup(spi);
		spi_set_ctldata(spi, NULL);
		kfree(chip);
	}
}

/**
 * ep93xx_spi_chip_setup() - configures hardware according to given @chip
 * @espi: ep93xx SPI controller struct
 * @chip: chip specific settings
 * @speed_hz: transfer speed
 * @bits_per_word: transfer bits_per_word
 */
static int ep93xx_spi_chip_setup(const struct ep93xx_spi *espi,
				 const struct ep93xx_spi_chip *chip,
				 u32 speed_hz, u8 bits_per_word)
{
	u8 dss = bits_per_word_to_dss(bits_per_word);
	u8 div_cpsr = 0;
	u8 div_scr = 0;
	u16 cr0;
	int err;

	err = ep93xx_spi_calc_divisors(espi, speed_hz, &div_cpsr, &div_scr);
	if (err)
		return err;

	cr0 = div_scr << SSPCR0_SCR_SHIFT;
	cr0 |= (chip->spi->mode & (SPI_CPHA|SPI_CPOL)) << SSPCR0_MODE_SHIFT;
	cr0 |= dss;

	dev_dbg(&espi->pdev->dev, "setup: mode %d, cpsr %d, scr %d, dss %d\n",
		chip->spi->mode, div_cpsr, div_scr, dss);
	dev_dbg(&espi->pdev->dev, "setup: cr0 %#x\n", cr0);

	ep93xx_spi_write_u8(espi, SSPCPSR, div_cpsr);
	ep93xx_spi_write_u16(espi, SSPCR0, cr0);

	return 0;
}

static void ep93xx_do_write(struct ep93xx_spi *espi, struct spi_transfer *t)
{
	if (t->bits_per_word > 8) {
		u16 tx_val = 0;

		if (t->tx_buf)
			tx_val = ((u16 *)t->tx_buf)[espi->tx];
		ep93xx_spi_write_u16(espi, SSPDR, tx_val);
		espi->tx += sizeof(tx_val);
	} else {
		u8 tx_val = 0;

		if (t->tx_buf)
			tx_val = ((u8 *)t->tx_buf)[espi->tx];
		ep93xx_spi_write_u8(espi, SSPDR, tx_val);
		espi->tx += sizeof(tx_val);
	}
}

static void ep93xx_do_read(struct ep93xx_spi *espi, struct spi_transfer *t)
{
	if (t->bits_per_word > 8) {
		u16 rx_val;

		rx_val = ep93xx_spi_read_u16(espi, SSPDR);
		if (t->rx_buf)
			((u16 *)t->rx_buf)[espi->rx] = rx_val;
		espi->rx += sizeof(rx_val);
	} else {
		u8 rx_val;

		rx_val = ep93xx_spi_read_u8(espi, SSPDR);
		if (t->rx_buf)
			((u8 *)t->rx_buf)[espi->rx] = rx_val;
		espi->rx += sizeof(rx_val);
	}
}

/**
 * ep93xx_spi_read_write() - perform next RX/TX transfer
 * @espi: ep93xx SPI controller struct
 *
 * This function transfers next bytes (or half-words) to/from RX/TX FIFOs. If
 * called several times, the whole transfer will be completed. Returns
 * %-EINPROGRESS when current transfer was not yet completed otherwise %0.
 *
 * When this function is finished, RX FIFO should be empty and TX FIFO should be
 * full.
 */
static int ep93xx_spi_read_write(struct ep93xx_spi *espi)
{
	struct spi_message *msg = espi->current_msg;
	struct spi_transfer *t = msg->state;

	/* read as long as RX FIFO has frames in it */
	while ((ep93xx_spi_read_u8(espi, SSPSR) & SSPSR_RNE)) {
		ep93xx_do_read(espi, t);
		espi->fifo_level--;
	}

	/* write as long as TX FIFO has room */
	while (espi->fifo_level < SPI_FIFO_SIZE && espi->tx < t->len) {
		ep93xx_do_write(espi, t);
		espi->fifo_level++;
	}

	if (espi->rx == t->len)
		return 0;

	return -EINPROGRESS;
}

static void ep93xx_spi_pio_transfer(struct ep93xx_spi *espi)
{
	/*
	 * Now everything is set up for the current transfer. We prime the TX
	 * FIFO, enable interrupts, and wait for the transfer to complete.
	 */
	if (ep93xx_spi_read_write(espi)) {
		ep93xx_spi_enable_interrupts(espi);
		wait_for_completion(&espi->wait);
	}
}

/**
 * ep93xx_spi_dma_prepare() - prepares a DMA transfer
 * @espi: ep93xx SPI controller struct
 * @dir: DMA transfer direction
 *
 * Function configures the DMA, maps the buffer and prepares the DMA
 * descriptor. Returns a valid DMA descriptor in case of success and ERR_PTR
 * in case of failure.
 */
static struct dma_async_tx_descriptor *
ep93xx_spi_dma_prepare(struct ep93xx_spi *espi, enum dma_transfer_direction dir)
{
	struct spi_transfer *t = espi->current_msg->state;
	struct dma_async_tx_descriptor *txd;
	enum dma_slave_buswidth buswidth;
	struct dma_slave_config conf;
	struct scatterlist *sg;
	struct sg_table *sgt;
	struct dma_chan *chan;
	const void *buf, *pbuf;
	size_t len = t->len;
	int i, ret, nents;

	if (t->bits_per_word > 8)
		buswidth = DMA_SLAVE_BUSWIDTH_2_BYTES;
	else
		buswidth = DMA_SLAVE_BUSWIDTH_1_BYTE;

	memset(&conf, 0, sizeof(conf));
	conf.direction = dir;

	if (dir == DMA_DEV_TO_MEM) {
		chan = espi->dma_rx;
		buf = t->rx_buf;
		sgt = &espi->rx_sgt;

		conf.src_addr = espi->sspdr_phys;
		conf.src_addr_width = buswidth;
	} else {
		chan = espi->dma_tx;
		buf = t->tx_buf;
		sgt = &espi->tx_sgt;

		conf.dst_addr = espi->sspdr_phys;
		conf.dst_addr_width = buswidth;
	}

	ret = dmaengine_slave_config(chan, &conf);
	if (ret)
		return ERR_PTR(ret);

	/*
	 * We need to split the transfer into PAGE_SIZE'd chunks. This is
	 * because we are using @espi->zeropage to provide a zero RX buffer
	 * for the TX transfers and we have only allocated one page for that.
	 *
	 * For performance reasons we allocate a new sg_table only when
	 * needed. Otherwise we will re-use the current one. Eventually the
	 * last sg_table is released in ep93xx_spi_release_dma().
	 */

	nents = DIV_ROUND_UP(len, PAGE_SIZE);
	if (nents != sgt->nents) {
		sg_free_table(sgt);

		ret = sg_alloc_table(sgt, nents, GFP_KERNEL);
		if (ret)
			return ERR_PTR(ret);
	}

	pbuf = buf;
	for_each_sg(sgt->sgl, sg, sgt->nents, i) {
		size_t bytes = min_t(size_t, len, PAGE_SIZE);

		if (buf) {
			sg_set_page(sg, virt_to_page(pbuf), bytes,
				    offset_in_page(pbuf));
		} else {
			sg_set_page(sg, virt_to_page(espi->zeropage),
				    bytes, 0);
		}

		pbuf += bytes;
		len -= bytes;
	}

	if (WARN_ON(len)) {
		dev_warn(&espi->pdev->dev, "len = %zu expected 0!\n", len);
		return ERR_PTR(-EINVAL);
	}

	nents = dma_map_sg(chan->device->dev, sgt->sgl, sgt->nents, dir);
	if (!nents)
		return ERR_PTR(-ENOMEM);

	txd = dmaengine_prep_slave_sg(chan, sgt->sgl, nents, dir, DMA_CTRL_ACK);
	if (!txd) {
		dma_unmap_sg(chan->device->dev, sgt->sgl, sgt->nents, dir);
		return ERR_PTR(-ENOMEM);
	}
	return txd;
}

/**
 * ep93xx_spi_dma_finish() - finishes with a DMA transfer
 * @espi: ep93xx SPI controller struct
 * @dir: DMA transfer direction
 *
 * Function finishes with the DMA transfer. After this, the DMA buffer is
 * unmapped.
 */
static void ep93xx_spi_dma_finish(struct ep93xx_spi *espi,
				  enum dma_transfer_direction dir)
{
	struct dma_chan *chan;
	struct sg_table *sgt;

	if (dir == DMA_DEV_TO_MEM) {
		chan = espi->dma_rx;
		sgt = &espi->rx_sgt;
	} else {
		chan = espi->dma_tx;
		sgt = &espi->tx_sgt;
	}

	dma_unmap_sg(chan->device->dev, sgt->sgl, sgt->nents, dir);
}

static void ep93xx_spi_dma_callback(void *callback_param)
{
	complete(callback_param);
}

static void ep93xx_spi_dma_transfer(struct ep93xx_spi *espi)
{
	struct spi_message *msg = espi->current_msg;
	struct dma_async_tx_descriptor *rxd, *txd;

	rxd = ep93xx_spi_dma_prepare(espi, DMA_DEV_TO_MEM);
	if (IS_ERR(rxd)) {
		dev_err(&espi->pdev->dev, "DMA RX failed: %ld\n", PTR_ERR(rxd));
		msg->status = PTR_ERR(rxd);
		return;
	}

	txd = ep93xx_spi_dma_prepare(espi, DMA_MEM_TO_DEV);
	if (IS_ERR(txd)) {
		ep93xx_spi_dma_finish(espi, DMA_DEV_TO_MEM);
		dev_err(&espi->pdev->dev, "DMA TX failed: %ld\n", PTR_ERR(rxd));
		msg->status = PTR_ERR(txd);
		return;
	}

	/* We are ready when RX is done */
	rxd->callback = ep93xx_spi_dma_callback;
	rxd->callback_param = &espi->wait;

	/* Now submit both descriptors and wait while they finish */
	dmaengine_submit(rxd);
	dmaengine_submit(txd);

	dma_async_issue_pending(espi->dma_rx);
	dma_async_issue_pending(espi->dma_tx);

	wait_for_completion(&espi->wait);

	ep93xx_spi_dma_finish(espi, DMA_MEM_TO_DEV);
	ep93xx_spi_dma_finish(espi, DMA_DEV_TO_MEM);
}

/**
 * ep93xx_spi_process_transfer() - processes one SPI transfer
 * @espi: ep93xx SPI controller struct
 * @msg: current message
 * @t: transfer to process
 *
 * This function processes one SPI transfer given in @t. Function waits until
 * transfer is complete (may sleep) and updates @msg->status based on whether
 * transfer was successfully processed or not.
 */
static void ep93xx_spi_process_transfer(struct ep93xx_spi *espi,
					struct spi_message *msg,
					struct spi_transfer *t)
{
	struct ep93xx_spi_chip *chip = spi_get_ctldata(msg->spi);
	int err;

	msg->state = t;

	err = ep93xx_spi_chip_setup(espi, chip, t->speed_hz, t->bits_per_word);
	if (err) {
		dev_err(&espi->pdev->dev,
			"failed to setup chip for transfer\n");
		msg->status = err;
		return;
	}

	espi->rx = 0;
	espi->tx = 0;

	/*
	 * There is no point of setting up DMA for the transfers which will
	 * fit into the FIFO and can be transferred with a single interrupt.
	 * So in these cases we will be using PIO and don't bother for DMA.
	 */
	if (espi->dma_rx && t->len > SPI_FIFO_SIZE)
		ep93xx_spi_dma_transfer(espi);
	else
		ep93xx_spi_pio_transfer(espi);

	/*
	 * In case of error during transmit, we bail out from processing
	 * the message.
	 */
	if (msg->status)
		return;

	msg->actual_length += t->len;

	/*
	 * After this transfer is finished, perform any possible
	 * post-transfer actions requested by the protocol driver.
	 */
	if (t->delay_usecs) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout(usecs_to_jiffies(t->delay_usecs));
	}
	if (t->cs_change) {
		if (!list_is_last(&t->transfer_list, &msg->transfers)) {
			/*
			 * In case protocol driver is asking us to drop the
			 * chipselect briefly, we let the scheduler to handle
			 * any "delay" here.
			 */
			ep93xx_spi_cs_control(msg->spi, false);
			cond_resched();
			ep93xx_spi_cs_control(msg->spi, true);
		}
	}
}

/*
 * ep93xx_spi_process_message() - process one SPI message
 * @espi: ep93xx SPI controller struct
 * @msg: message to process
 *
 * This function processes a single SPI message. We go through all transfers in
 * the message and pass them to ep93xx_spi_process_transfer(). Chipselect is
 * asserted during the whole message (unless per transfer cs_change is set).
 *
 * @msg->status contains %0 in case of success or negative error code in case of
 * failure.
 */
static void ep93xx_spi_process_message(struct ep93xx_spi *espi,
				       struct spi_message *msg)
{
	unsigned long timeout;
	struct spi_transfer *t;
	int err;

	/*
	 * Enable the SPI controller and its clock.
	 */
	err = ep93xx_spi_enable(espi);
	if (err) {
		dev_err(&espi->pdev->dev, "failed to enable SPI controller\n");
		msg->status = err;
		return;
	}

	/*
	 * Just to be sure: flush any data from RX FIFO.
	 */
	timeout = jiffies + msecs_to_jiffies(SPI_TIMEOUT);
	while (ep93xx_spi_read_u16(espi, SSPSR) & SSPSR_RNE) {
		if (time_after(jiffies, timeout)) {
			dev_warn(&espi->pdev->dev,
				 "timeout while flushing RX FIFO\n");
			msg->status = -ETIMEDOUT;
			return;
		}
		ep93xx_spi_read_u16(espi, SSPDR);
	}

	/*
	 * We explicitly handle FIFO level. This way we don't have to check TX
	 * FIFO status using %SSPSR_TNF bit which may cause RX FIFO overruns.
	 */
	espi->fifo_level = 0;

	/*
	 * Assert the chipselect.
	 */
	ep93xx_spi_cs_control(msg->spi, true);

	list_for_each_entry(t, &msg->transfers, transfer_list) {
		ep93xx_spi_process_transfer(espi, msg, t);
		if (msg->status)
			break;
	}

	/*
	 * Now the whole message is transferred (or failed for some reason). We
	 * deselect the device and disable the SPI controller.
	 */
	ep93xx_spi_cs_control(msg->spi, false);
	ep93xx_spi_disable(espi);
}

static int ep93xx_spi_transfer_one_message(struct spi_master *master,
					   struct spi_message *msg)
{
	struct ep93xx_spi *espi = spi_master_get_devdata(master);

	msg->state = NULL;
	msg->status = 0;
	msg->actual_length = 0;

	espi->current_msg = msg;
	ep93xx_spi_process_message(espi, msg);
	espi->current_msg = NULL;

	spi_finalize_current_message(master);

	return 0;
}

static irqreturn_t ep93xx_spi_interrupt(int irq, void *dev_id)
{
	struct ep93xx_spi *espi = dev_id;
	u8 irq_status = ep93xx_spi_read_u8(espi, SSPIIR);

	/*
	 * If we got ROR (receive overrun) interrupt we know that something is
	 * wrong. Just abort the message.
	 */
	if (unlikely(irq_status & SSPIIR_RORIS)) {
		/* clear the overrun interrupt */
		ep93xx_spi_write_u8(espi, SSPICR, 0);
		dev_warn(&espi->pdev->dev,
			 "receive overrun, aborting the message\n");
		espi->current_msg->status = -EIO;
	} else {
		/*
		 * Interrupt is either RX (RIS) or TX (TIS). For both cases we
		 * simply execute next data transfer.
		 */
		if (ep93xx_spi_read_write(espi)) {
			/*
			 * In normal case, there still is some processing left
			 * for current transfer. Let's wait for the next
			 * interrupt then.
			 */
			return IRQ_HANDLED;
		}
	}

	/*
	 * Current transfer is finished, either with error or with success. In
	 * any case we disable interrupts and notify the worker to handle
	 * any post-processing of the message.
	 */
	ep93xx_spi_disable_interrupts(espi);
	complete(&espi->wait);
	return IRQ_HANDLED;
}

static bool ep93xx_spi_dma_filter(struct dma_chan *chan, void *filter_param)
{
	if (ep93xx_dma_chan_is_m2p(chan))
		return false;

	chan->private = filter_param;
	return true;
}

static int ep93xx_spi_setup_dma(struct ep93xx_spi *espi)
{
	dma_cap_mask_t mask;
	int ret;

	espi->zeropage = (void *)get_zeroed_page(GFP_KERNEL);
	if (!espi->zeropage)
		return -ENOMEM;

	dma_cap_zero(mask);
	dma_cap_set(DMA_SLAVE, mask);

	espi->dma_rx_data.port = EP93XX_DMA_SSP;
	espi->dma_rx_data.direction = DMA_DEV_TO_MEM;
	espi->dma_rx_data.name = "ep93xx-spi-rx";

	espi->dma_rx = dma_request_channel(mask, ep93xx_spi_dma_filter,
					   &espi->dma_rx_data);
	if (!espi->dma_rx) {
		ret = -ENODEV;
		goto fail_free_page;
	}

	espi->dma_tx_data.port = EP93XX_DMA_SSP;
	espi->dma_tx_data.direction = DMA_MEM_TO_DEV;
	espi->dma_tx_data.name = "ep93xx-spi-tx";

	espi->dma_tx = dma_request_channel(mask, ep93xx_spi_dma_filter,
					   &espi->dma_tx_data);
	if (!espi->dma_tx) {
		ret = -ENODEV;
		goto fail_release_rx;
	}

	return 0;

fail_release_rx:
	dma_release_channel(espi->dma_rx);
	espi->dma_rx = NULL;
fail_free_page:
	free_page((unsigned long)espi->zeropage);

	return ret;
}

static void ep93xx_spi_release_dma(struct ep93xx_spi *espi)
{
	if (espi->dma_rx) {
		dma_release_channel(espi->dma_rx);
		sg_free_table(&espi->rx_sgt);
	}
	if (espi->dma_tx) {
		dma_release_channel(espi->dma_tx);
		sg_free_table(&espi->tx_sgt);
	}

	if (espi->zeropage)
		free_page((unsigned long)espi->zeropage);
}

static int ep93xx_spi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct ep93xx_spi_info *info;
	struct ep93xx_spi *espi;
	struct resource *res;
	int irq;
	int error;

	info = dev_get_platdata(&pdev->dev);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "failed to get irq resources\n");
		return -EBUSY;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "unable to get iomem resource\n");
		return -ENODEV;
	}

	master = spi_alloc_master(&pdev->dev, sizeof(*espi));
	if (!master)
		return -ENOMEM;

	master->setup = ep93xx_spi_setup;
	master->transfer_one_message = ep93xx_spi_transfer_one_message;
	master->cleanup = ep93xx_spi_cleanup;
	master->bus_num = pdev->id;
	master->num_chipselect = info->num_chipselect;
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH;
	master->bits_per_word_mask = SPI_BPW_RANGE_MASK(4, 16);

	platform_set_drvdata(pdev, master);

	espi = spi_master_get_devdata(master);

	espi->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(espi->clk)) {
		dev_err(&pdev->dev, "unable to get spi clock\n");
		error = PTR_ERR(espi->clk);
		goto fail_release_master;
	}

	init_completion(&espi->wait);

	/*
	 * Calculate maximum and minimum supported clock rates
	 * for the controller.
	 */
	master->max_speed_hz = clk_get_rate(espi->clk) / 2;
	master->min_speed_hz = clk_get_rate(espi->clk) / (254 * 256);
	espi->pdev = pdev;

	espi->sspdr_phys = res->start + SSPDR;

	espi->regs_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(espi->regs_base)) {
		error = PTR_ERR(espi->regs_base);
		goto fail_release_master;
	}

	error = devm_request_irq(&pdev->dev, irq, ep93xx_spi_interrupt,
				0, "ep93xx-spi", espi);
	if (error) {
		dev_err(&pdev->dev, "failed to request irq\n");
		goto fail_release_master;
	}

	if (info->use_dma && ep93xx_spi_setup_dma(espi))
		dev_warn(&pdev->dev, "DMA setup failed. Falling back to PIO\n");

	/* make sure that the hardware is disabled */
	ep93xx_spi_write_u8(espi, SSPCR1, 0);

	error = devm_spi_register_master(&pdev->dev, master);
	if (error) {
		dev_err(&pdev->dev, "failed to register SPI master\n");
		goto fail_free_dma;
	}

	dev_info(&pdev->dev, "EP93xx SPI Controller at 0x%08lx irq %d\n",
		 (unsigned long)res->start, irq);

	return 0;

fail_free_dma:
	ep93xx_spi_release_dma(espi);
fail_release_master:
	spi_master_put(master);

	return error;
}

static int ep93xx_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master = platform_get_drvdata(pdev);
	struct ep93xx_spi *espi = spi_master_get_devdata(master);

	ep93xx_spi_release_dma(espi);

	return 0;
}

static struct platform_driver ep93xx_spi_driver = {
	.driver		= {
		.name	= "ep93xx-spi",
	},
	.probe		= ep93xx_spi_probe,
	.remove		= ep93xx_spi_remove,
};
module_platform_driver(ep93xx_spi_driver);

MODULE_DESCRIPTION("EP93xx SPI Controller driver");
MODULE_AUTHOR("Mika Westerberg <mika.westerberg@iki.fi>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:ep93xx-spi");
