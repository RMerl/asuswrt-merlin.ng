/*
 * Copyright (C) 2005 Stephen Street / StreetFire Sound Labs
 * Copyright (C) 2013, Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/spi/pxa2xx_spi.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/acpi.h>

#include "spi-pxa2xx.h"

MODULE_AUTHOR("Stephen Street");
MODULE_DESCRIPTION("PXA2xx SSP SPI Controller");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pxa2xx-spi");

#define TIMOUT_DFLT		1000

/*
 * for testing SSCR1 changes that require SSP restart, basically
 * everything except the service and interrupt enables, the pxa270 developer
 * manual says only SSCR1_SCFR, SSCR1_SPH, SSCR1_SPO need to be in this
 * list, but the PXA255 dev man says all bits without really meaning the
 * service and interrupt enables
 */
#define SSCR1_CHANGE_MASK (SSCR1_TTELP | SSCR1_TTE | SSCR1_SCFR \
				| SSCR1_ECRA | SSCR1_ECRB | SSCR1_SCLKDIR \
				| SSCR1_SFRMDIR | SSCR1_RWOT | SSCR1_TRAIL \
				| SSCR1_IFS | SSCR1_STRF | SSCR1_EFWR \
				| SSCR1_RFT | SSCR1_TFT | SSCR1_MWDS \
				| SSCR1_SPH | SSCR1_SPO | SSCR1_LBM)

#define QUARK_X1000_SSCR1_CHANGE_MASK (QUARK_X1000_SSCR1_STRF	\
				| QUARK_X1000_SSCR1_EFWR	\
				| QUARK_X1000_SSCR1_RFT		\
				| QUARK_X1000_SSCR1_TFT		\
				| SSCR1_SPH | SSCR1_SPO | SSCR1_LBM)

#define LPSS_RX_THRESH_DFLT	64
#define LPSS_TX_LOTHRESH_DFLT	160
#define LPSS_TX_HITHRESH_DFLT	224

/* Offset from drv_data->lpss_base */
#define GENERAL_REG		0x08
#define GENERAL_REG_RXTO_HOLDOFF_DISABLE BIT(24)
#define SSP_REG			0x0c
#define SPI_CS_CONTROL		0x18
#define SPI_CS_CONTROL_SW_MODE	BIT(0)
#define SPI_CS_CONTROL_CS_HIGH	BIT(1)

static bool is_lpss_ssp(const struct driver_data *drv_data)
{
	return drv_data->ssp_type == LPSS_SSP;
}

static bool is_quark_x1000_ssp(const struct driver_data *drv_data)
{
	return drv_data->ssp_type == QUARK_X1000_SSP;
}

static u32 pxa2xx_spi_get_ssrc1_change_mask(const struct driver_data *drv_data)
{
	switch (drv_data->ssp_type) {
	case QUARK_X1000_SSP:
		return QUARK_X1000_SSCR1_CHANGE_MASK;
	default:
		return SSCR1_CHANGE_MASK;
	}
}

static u32
pxa2xx_spi_get_rx_default_thre(const struct driver_data *drv_data)
{
	switch (drv_data->ssp_type) {
	case QUARK_X1000_SSP:
		return RX_THRESH_QUARK_X1000_DFLT;
	default:
		return RX_THRESH_DFLT;
	}
}

static bool pxa2xx_spi_txfifo_full(const struct driver_data *drv_data)
{
	u32 mask;

	switch (drv_data->ssp_type) {
	case QUARK_X1000_SSP:
		mask = QUARK_X1000_SSSR_TFL_MASK;
		break;
	default:
		mask = SSSR_TFL_MASK;
		break;
	}

	return (pxa2xx_spi_read(drv_data, SSSR) & mask) == mask;
}

static void pxa2xx_spi_clear_rx_thre(const struct driver_data *drv_data,
				     u32 *sccr1_reg)
{
	u32 mask;

	switch (drv_data->ssp_type) {
	case QUARK_X1000_SSP:
		mask = QUARK_X1000_SSCR1_RFT;
		break;
	default:
		mask = SSCR1_RFT;
		break;
	}
	*sccr1_reg &= ~mask;
}

static void pxa2xx_spi_set_rx_thre(const struct driver_data *drv_data,
				   u32 *sccr1_reg, u32 threshold)
{
	switch (drv_data->ssp_type) {
	case QUARK_X1000_SSP:
		*sccr1_reg |= QUARK_X1000_SSCR1_RxTresh(threshold);
		break;
	default:
		*sccr1_reg |= SSCR1_RxTresh(threshold);
		break;
	}
}

static u32 pxa2xx_configure_sscr0(const struct driver_data *drv_data,
				  u32 clk_div, u8 bits)
{
	switch (drv_data->ssp_type) {
	case QUARK_X1000_SSP:
		return clk_div
			| QUARK_X1000_SSCR0_Motorola
			| QUARK_X1000_SSCR0_DataSize(bits > 32 ? 8 : bits)
			| SSCR0_SSE;
	default:
		return clk_div
			| SSCR0_Motorola
			| SSCR0_DataSize(bits > 16 ? bits - 16 : bits)
			| SSCR0_SSE
			| (bits > 16 ? SSCR0_EDSS : 0);
	}
}

/*
 * Read and write LPSS SSP private registers. Caller must first check that
 * is_lpss_ssp() returns true before these can be called.
 */
static u32 __lpss_ssp_read_priv(struct driver_data *drv_data, unsigned offset)
{
	WARN_ON(!drv_data->lpss_base);
	return readl(drv_data->lpss_base + offset);
}

static void __lpss_ssp_write_priv(struct driver_data *drv_data,
				  unsigned offset, u32 value)
{
	WARN_ON(!drv_data->lpss_base);
	writel(value, drv_data->lpss_base + offset);
}

/*
 * lpss_ssp_setup - perform LPSS SSP specific setup
 * @drv_data: pointer to the driver private data
 *
 * Perform LPSS SSP specific setup. This function must be called first if
 * one is going to use LPSS SSP private registers.
 */
static void lpss_ssp_setup(struct driver_data *drv_data)
{
	unsigned offset = 0x400;
	u32 value, orig;

	/*
	 * Perform auto-detection of the LPSS SSP private registers. They
	 * can be either at 1k or 2k offset from the base address.
	 */
	orig = readl(drv_data->ioaddr + offset + SPI_CS_CONTROL);

	/* Test SPI_CS_CONTROL_SW_MODE bit enabling */
	value = orig | SPI_CS_CONTROL_SW_MODE;
	writel(value, drv_data->ioaddr + offset + SPI_CS_CONTROL);
	value = readl(drv_data->ioaddr + offset + SPI_CS_CONTROL);
	if (value != (orig | SPI_CS_CONTROL_SW_MODE)) {
		offset = 0x800;
		goto detection_done;
	}

	orig = readl(drv_data->ioaddr + offset + SPI_CS_CONTROL);

	/* Test SPI_CS_CONTROL_SW_MODE bit disabling */
	value = orig & ~SPI_CS_CONTROL_SW_MODE;
	writel(value, drv_data->ioaddr + offset + SPI_CS_CONTROL);
	value = readl(drv_data->ioaddr + offset + SPI_CS_CONTROL);
	if (value != (orig & ~SPI_CS_CONTROL_SW_MODE)) {
		offset = 0x800;
		goto detection_done;
	}

detection_done:
	/* Now set the LPSS base */
	drv_data->lpss_base = drv_data->ioaddr + offset;

	/* Enable software chip select control */
	value = SPI_CS_CONTROL_SW_MODE | SPI_CS_CONTROL_CS_HIGH;
	__lpss_ssp_write_priv(drv_data, SPI_CS_CONTROL, value);

	/* Enable multiblock DMA transfers */
	if (drv_data->master_info->enable_dma) {
		__lpss_ssp_write_priv(drv_data, SSP_REG, 1);

		value = __lpss_ssp_read_priv(drv_data, GENERAL_REG);
		value |= GENERAL_REG_RXTO_HOLDOFF_DISABLE;
		__lpss_ssp_write_priv(drv_data, GENERAL_REG, value);
	}
}

static void lpss_ssp_cs_control(struct driver_data *drv_data, bool enable)
{
	u32 value;

	value = __lpss_ssp_read_priv(drv_data, SPI_CS_CONTROL);
	if (enable)
		value &= ~SPI_CS_CONTROL_CS_HIGH;
	else
		value |= SPI_CS_CONTROL_CS_HIGH;
	__lpss_ssp_write_priv(drv_data, SPI_CS_CONTROL, value);
}

static void cs_assert(struct driver_data *drv_data)
{
	struct chip_data *chip = drv_data->cur_chip;

	if (drv_data->ssp_type == CE4100_SSP) {
		pxa2xx_spi_write(drv_data, SSSR, drv_data->cur_chip->frm);
		return;
	}

	if (chip->cs_control) {
		chip->cs_control(PXA2XX_CS_ASSERT);
		return;
	}

	if (gpio_is_valid(chip->gpio_cs)) {
		gpio_set_value(chip->gpio_cs, chip->gpio_cs_inverted);
		return;
	}

	if (is_lpss_ssp(drv_data))
		lpss_ssp_cs_control(drv_data, true);
}

static void cs_deassert(struct driver_data *drv_data)
{
	struct chip_data *chip = drv_data->cur_chip;

	if (drv_data->ssp_type == CE4100_SSP)
		return;

	if (chip->cs_control) {
		chip->cs_control(PXA2XX_CS_DEASSERT);
		return;
	}

	if (gpio_is_valid(chip->gpio_cs)) {
		gpio_set_value(chip->gpio_cs, !chip->gpio_cs_inverted);
		return;
	}

	if (is_lpss_ssp(drv_data))
		lpss_ssp_cs_control(drv_data, false);
}

int pxa2xx_spi_flush(struct driver_data *drv_data)
{
	unsigned long limit = loops_per_jiffy << 1;

	do {
		while (pxa2xx_spi_read(drv_data, SSSR) & SSSR_RNE)
			pxa2xx_spi_read(drv_data, SSDR);
	} while ((pxa2xx_spi_read(drv_data, SSSR) & SSSR_BSY) && --limit);
	write_SSSR_CS(drv_data, SSSR_ROR);

	return limit;
}

static int null_writer(struct driver_data *drv_data)
{
	u8 n_bytes = drv_data->n_bytes;

	if (pxa2xx_spi_txfifo_full(drv_data)
		|| (drv_data->tx == drv_data->tx_end))
		return 0;

	pxa2xx_spi_write(drv_data, SSDR, 0);
	drv_data->tx += n_bytes;

	return 1;
}

static int null_reader(struct driver_data *drv_data)
{
	u8 n_bytes = drv_data->n_bytes;

	while ((pxa2xx_spi_read(drv_data, SSSR) & SSSR_RNE)
	       && (drv_data->rx < drv_data->rx_end)) {
		pxa2xx_spi_read(drv_data, SSDR);
		drv_data->rx += n_bytes;
	}

	return drv_data->rx == drv_data->rx_end;
}

static int u8_writer(struct driver_data *drv_data)
{
	if (pxa2xx_spi_txfifo_full(drv_data)
		|| (drv_data->tx == drv_data->tx_end))
		return 0;

	pxa2xx_spi_write(drv_data, SSDR, *(u8 *)(drv_data->tx));
	++drv_data->tx;

	return 1;
}

static int u8_reader(struct driver_data *drv_data)
{
	while ((pxa2xx_spi_read(drv_data, SSSR) & SSSR_RNE)
	       && (drv_data->rx < drv_data->rx_end)) {
		*(u8 *)(drv_data->rx) = pxa2xx_spi_read(drv_data, SSDR);
		++drv_data->rx;
	}

	return drv_data->rx == drv_data->rx_end;
}

static int u16_writer(struct driver_data *drv_data)
{
	if (pxa2xx_spi_txfifo_full(drv_data)
		|| (drv_data->tx == drv_data->tx_end))
		return 0;

	pxa2xx_spi_write(drv_data, SSDR, *(u16 *)(drv_data->tx));
	drv_data->tx += 2;

	return 1;
}

static int u16_reader(struct driver_data *drv_data)
{
	while ((pxa2xx_spi_read(drv_data, SSSR) & SSSR_RNE)
	       && (drv_data->rx < drv_data->rx_end)) {
		*(u16 *)(drv_data->rx) = pxa2xx_spi_read(drv_data, SSDR);
		drv_data->rx += 2;
	}

	return drv_data->rx == drv_data->rx_end;
}

static int u32_writer(struct driver_data *drv_data)
{
	if (pxa2xx_spi_txfifo_full(drv_data)
		|| (drv_data->tx == drv_data->tx_end))
		return 0;

	pxa2xx_spi_write(drv_data, SSDR, *(u32 *)(drv_data->tx));
	drv_data->tx += 4;

	return 1;
}

static int u32_reader(struct driver_data *drv_data)
{
	while ((pxa2xx_spi_read(drv_data, SSSR) & SSSR_RNE)
	       && (drv_data->rx < drv_data->rx_end)) {
		*(u32 *)(drv_data->rx) = pxa2xx_spi_read(drv_data, SSDR);
		drv_data->rx += 4;
	}

	return drv_data->rx == drv_data->rx_end;
}

void *pxa2xx_spi_next_transfer(struct driver_data *drv_data)
{
	struct spi_message *msg = drv_data->cur_msg;
	struct spi_transfer *trans = drv_data->cur_transfer;

	/* Move to next transfer */
	if (trans->transfer_list.next != &msg->transfers) {
		drv_data->cur_transfer =
			list_entry(trans->transfer_list.next,
					struct spi_transfer,
					transfer_list);
		return RUNNING_STATE;
	} else
		return DONE_STATE;
}

/* caller already set message->status; dma and pio irqs are blocked */
static void giveback(struct driver_data *drv_data)
{
	struct spi_transfer* last_transfer;
	struct spi_message *msg;

	msg = drv_data->cur_msg;
	drv_data->cur_msg = NULL;
	drv_data->cur_transfer = NULL;

	last_transfer = list_last_entry(&msg->transfers, struct spi_transfer,
					transfer_list);

	/* Delay if requested before any change in chip select */
	if (last_transfer->delay_usecs)
		udelay(last_transfer->delay_usecs);

	/* Drop chip select UNLESS cs_change is true or we are returning
	 * a message with an error, or next message is for another chip
	 */
	if (!last_transfer->cs_change)
		cs_deassert(drv_data);
	else {
		struct spi_message *next_msg;

		/* Holding of cs was hinted, but we need to make sure
		 * the next message is for the same chip.  Don't waste
		 * time with the following tests unless this was hinted.
		 *
		 * We cannot postpone this until pump_messages, because
		 * after calling msg->complete (below) the driver that
		 * sent the current message could be unloaded, which
		 * could invalidate the cs_control() callback...
		 */

		/* get a pointer to the next message, if any */
		next_msg = spi_get_next_queued_message(drv_data->master);

		/* see if the next and current messages point
		 * to the same chip
		 */
		if (next_msg && next_msg->spi != msg->spi)
			next_msg = NULL;
		if (!next_msg || msg->state == ERROR_STATE)
			cs_deassert(drv_data);
	}

	drv_data->cur_chip = NULL;
	spi_finalize_current_message(drv_data->master);
}

static void reset_sccr1(struct driver_data *drv_data)
{
	struct chip_data *chip = drv_data->cur_chip;
	u32 sccr1_reg;

	sccr1_reg = pxa2xx_spi_read(drv_data, SSCR1) & ~drv_data->int_cr1;
	switch (drv_data->ssp_type) {
	case QUARK_X1000_SSP:
		sccr1_reg &= ~QUARK_X1000_SSCR1_RFT;
		break;
	default:
		sccr1_reg &= ~SSCR1_RFT;
		break;
	}
	sccr1_reg |= chip->threshold;
	pxa2xx_spi_write(drv_data, SSCR1, sccr1_reg);
}

static void int_error_stop(struct driver_data *drv_data, const char* msg)
{
	/* Stop and reset SSP */
	write_SSSR_CS(drv_data, drv_data->clear_sr);
	reset_sccr1(drv_data);
	if (!pxa25x_ssp_comp(drv_data))
		pxa2xx_spi_write(drv_data, SSTO, 0);
	pxa2xx_spi_flush(drv_data);
	pxa2xx_spi_write(drv_data, SSCR0,
			 pxa2xx_spi_read(drv_data, SSCR0) & ~SSCR0_SSE);

	dev_err(&drv_data->pdev->dev, "%s\n", msg);

	drv_data->cur_msg->state = ERROR_STATE;
	tasklet_schedule(&drv_data->pump_transfers);
}

static void int_transfer_complete(struct driver_data *drv_data)
{
	/* Stop SSP */
	write_SSSR_CS(drv_data, drv_data->clear_sr);
	reset_sccr1(drv_data);
	if (!pxa25x_ssp_comp(drv_data))
		pxa2xx_spi_write(drv_data, SSTO, 0);

	/* Update total byte transferred return count actual bytes read */
	drv_data->cur_msg->actual_length += drv_data->len -
				(drv_data->rx_end - drv_data->rx);

	/* Transfer delays and chip select release are
	 * handled in pump_transfers or giveback
	 */

	/* Move to next transfer */
	drv_data->cur_msg->state = pxa2xx_spi_next_transfer(drv_data);

	/* Schedule transfer tasklet */
	tasklet_schedule(&drv_data->pump_transfers);
}

static irqreturn_t interrupt_transfer(struct driver_data *drv_data)
{
	u32 irq_mask = (pxa2xx_spi_read(drv_data, SSCR1) & SSCR1_TIE) ?
		       drv_data->mask_sr : drv_data->mask_sr & ~SSSR_TFS;

	u32 irq_status = pxa2xx_spi_read(drv_data, SSSR) & irq_mask;

	if (irq_status & SSSR_ROR) {
		int_error_stop(drv_data, "interrupt_transfer: fifo overrun");
		return IRQ_HANDLED;
	}

	if (irq_status & SSSR_TINT) {
		pxa2xx_spi_write(drv_data, SSSR, SSSR_TINT);
		if (drv_data->read(drv_data)) {
			int_transfer_complete(drv_data);
			return IRQ_HANDLED;
		}
	}

	/* Drain rx fifo, Fill tx fifo and prevent overruns */
	do {
		if (drv_data->read(drv_data)) {
			int_transfer_complete(drv_data);
			return IRQ_HANDLED;
		}
	} while (drv_data->write(drv_data));

	if (drv_data->read(drv_data)) {
		int_transfer_complete(drv_data);
		return IRQ_HANDLED;
	}

	if (drv_data->tx == drv_data->tx_end) {
		u32 bytes_left;
		u32 sccr1_reg;

		sccr1_reg = pxa2xx_spi_read(drv_data, SSCR1);
		sccr1_reg &= ~SSCR1_TIE;

		/*
		 * PXA25x_SSP has no timeout, set up rx threshould for the
		 * remaining RX bytes.
		 */
		if (pxa25x_ssp_comp(drv_data)) {
			u32 rx_thre;

			pxa2xx_spi_clear_rx_thre(drv_data, &sccr1_reg);

			bytes_left = drv_data->rx_end - drv_data->rx;
			switch (drv_data->n_bytes) {
			case 4:
				bytes_left >>= 1;
			case 2:
				bytes_left >>= 1;
			}

			rx_thre = pxa2xx_spi_get_rx_default_thre(drv_data);
			if (rx_thre > bytes_left)
				rx_thre = bytes_left;

			pxa2xx_spi_set_rx_thre(drv_data, &sccr1_reg, rx_thre);
		}
		pxa2xx_spi_write(drv_data, SSCR1, sccr1_reg);
	}

	/* We did something */
	return IRQ_HANDLED;
}

static irqreturn_t ssp_int(int irq, void *dev_id)
{
	struct driver_data *drv_data = dev_id;
	u32 sccr1_reg;
	u32 mask = drv_data->mask_sr;
	u32 status;

	/*
	 * The IRQ might be shared with other peripherals so we must first
	 * check that are we RPM suspended or not. If we are we assume that
	 * the IRQ was not for us (we shouldn't be RPM suspended when the
	 * interrupt is enabled).
	 */
	if (pm_runtime_suspended(&drv_data->pdev->dev))
		return IRQ_NONE;

	/*
	 * If the device is not yet in RPM suspended state and we get an
	 * interrupt that is meant for another device, check if status bits
	 * are all set to one. That means that the device is already
	 * powered off.
	 */
	status = pxa2xx_spi_read(drv_data, SSSR);
	if (status == ~0)
		return IRQ_NONE;

	sccr1_reg = pxa2xx_spi_read(drv_data, SSCR1);

	/* Ignore possible writes if we don't need to write */
	if (!(sccr1_reg & SSCR1_TIE))
		mask &= ~SSSR_TFS;

	/* Ignore RX timeout interrupt if it is disabled */
	if (!(sccr1_reg & SSCR1_TINTE))
		mask &= ~SSSR_TINT;

	if (!(status & mask))
		return IRQ_NONE;

	if (!drv_data->cur_msg) {

		pxa2xx_spi_write(drv_data, SSCR0,
				 pxa2xx_spi_read(drv_data, SSCR0)
				 & ~SSCR0_SSE);
		pxa2xx_spi_write(drv_data, SSCR1,
				 pxa2xx_spi_read(drv_data, SSCR1)
				 & ~drv_data->int_cr1);
		if (!pxa25x_ssp_comp(drv_data))
			pxa2xx_spi_write(drv_data, SSTO, 0);
		write_SSSR_CS(drv_data, drv_data->clear_sr);

		dev_err(&drv_data->pdev->dev,
			"bad message state in interrupt handler\n");

		/* Never fail */
		return IRQ_HANDLED;
	}

	return drv_data->transfer_handler(drv_data);
}

/*
 * The Quark SPI has an additional 24 bit register (DDS_CLK_RATE) to multiply
 * input frequency by fractions of 2^24. It also has a divider by 5.
 *
 * There are formulas to get baud rate value for given input frequency and
 * divider parameters, such as DDS_CLK_RATE and SCR:
 *
 * Fsys = 200MHz
 *
 * Fssp = Fsys * DDS_CLK_RATE / 2^24			(1)
 * Baud rate = Fsclk = Fssp / (2 * (SCR + 1))		(2)
 *
 * DDS_CLK_RATE either 2^n or 2^n / 5.
 * SCR is in range 0 .. 255
 *
 * Divisor = 5^i * 2^j * 2 * k
 *       i = [0, 1]      i = 1 iff j = 0 or j > 3
 *       j = [0, 23]     j = 0 iff i = 1
 *       k = [1, 256]
 * Special case: j = 0, i = 1: Divisor = 2 / 5
 *
 * Accordingly to the specification the recommended values for DDS_CLK_RATE
 * are:
 *	Case 1:		2^n, n = [0, 23]
 *	Case 2:		2^24 * 2 / 5 (0x666666)
 *	Case 3:		less than or equal to 2^24 / 5 / 16 (0x33333)
 *
 * In all cases the lowest possible value is better.
 *
 * The function calculates parameters for all cases and chooses the one closest
 * to the asked baud rate.
 */
static unsigned int quark_x1000_get_clk_div(int rate, u32 *dds)
{
	unsigned long xtal = 200000000;
	unsigned long fref = xtal / 2;		/* mandatory division by 2,
						   see (2) */
						/* case 3 */
	unsigned long fref1 = fref / 2;		/* case 1 */
	unsigned long fref2 = fref * 2 / 5;	/* case 2 */
	unsigned long scale;
	unsigned long q, q1, q2;
	long r, r1, r2;
	u32 mul;

	/* Case 1 */

	/* Set initial value for DDS_CLK_RATE */
	mul = (1 << 24) >> 1;

	/* Calculate initial quot */
	q1 = DIV_ROUND_CLOSEST(fref1, rate);

	/* Scale q1 if it's too big */
	if (q1 > 256) {
		/* Scale q1 to range [1, 512] */
		scale = fls_long(q1 - 1);
		if (scale > 9) {
			q1 >>= scale - 9;
			mul >>= scale - 9;
		}

		/* Round the result if we have a remainder */
		q1 += q1 & 1;
	}

	/* Decrease DDS_CLK_RATE as much as we can without loss in precision */
	scale = __ffs(q1);
	q1 >>= scale;
	mul >>= scale;

	/* Get the remainder */
	r1 = abs(fref1 / (1 << (24 - fls_long(mul))) / q1 - rate);

	/* Case 2 */

	q2 = DIV_ROUND_CLOSEST(fref2, rate);
	r2 = abs(fref2 / q2 - rate);

	/*
	 * Choose the best between two: less remainder we have the better. We
	 * can't go case 2 if q2 is greater than 256 since SCR register can
	 * hold only values 0 .. 255.
	 */
	if (r2 >= r1 || q2 > 256) {
		/* case 1 is better */
		r = r1;
		q = q1;
	} else {
		/* case 2 is better */
		r = r2;
		q = q2;
		mul = (1 << 24) * 2 / 5;
	}

	/* Check case 3 only If the divisor is big enough */
	if (fref / rate >= 80) {
		u64 fssp;
		u32 m;

		/* Calculate initial quot */
		q1 = DIV_ROUND_CLOSEST(fref, rate);
		m = (1 << 24) / q1;

		/* Get the remainder */
		fssp = (u64)fref * m;
		do_div(fssp, 1 << 24);
		r1 = abs(fssp - rate);

		/* Choose this one if it suits better */
		if (r1 < r) {
			/* case 3 is better */
			q = 1;
			mul = m;
		}
	}

	*dds = mul;
	return q - 1;
}

static unsigned int ssp_get_clk_div(struct driver_data *drv_data, int rate)
{
	unsigned long ssp_clk = drv_data->max_clk_rate;
	const struct ssp_device *ssp = drv_data->ssp;

	rate = min_t(int, ssp_clk, rate);

	if (ssp->type == PXA25x_SSP || ssp->type == CE4100_SSP)
		return (ssp_clk / (2 * rate) - 1) & 0xff;
	else
		return (ssp_clk / rate - 1) & 0xfff;
}

static unsigned int pxa2xx_ssp_get_clk_div(struct driver_data *drv_data,
					   struct chip_data *chip, int rate)
{
	unsigned int clk_div;

	switch (drv_data->ssp_type) {
	case QUARK_X1000_SSP:
		clk_div = quark_x1000_get_clk_div(rate, &chip->dds_rate);
		break;
	default:
		clk_div = ssp_get_clk_div(drv_data, rate);
		break;
	}
	return clk_div << 8;
}

static void pump_transfers(unsigned long data)
{
	struct driver_data *drv_data = (struct driver_data *)data;
	struct spi_message *message = NULL;
	struct spi_transfer *transfer = NULL;
	struct spi_transfer *previous = NULL;
	struct chip_data *chip = NULL;
	u32 clk_div = 0;
	u8 bits = 0;
	u32 speed = 0;
	u32 cr0;
	u32 cr1;
	u32 dma_thresh = drv_data->cur_chip->dma_threshold;
	u32 dma_burst = drv_data->cur_chip->dma_burst_size;
	u32 change_mask = pxa2xx_spi_get_ssrc1_change_mask(drv_data);

	/* Get current state information */
	message = drv_data->cur_msg;
	transfer = drv_data->cur_transfer;
	chip = drv_data->cur_chip;

	/* Handle for abort */
	if (message->state == ERROR_STATE) {
		message->status = -EIO;
		giveback(drv_data);
		return;
	}

	/* Handle end of message */
	if (message->state == DONE_STATE) {
		message->status = 0;
		giveback(drv_data);
		return;
	}

	/* Delay if requested at end of transfer before CS change */
	if (message->state == RUNNING_STATE) {
		previous = list_entry(transfer->transfer_list.prev,
					struct spi_transfer,
					transfer_list);
		if (previous->delay_usecs)
			udelay(previous->delay_usecs);

		/* Drop chip select only if cs_change is requested */
		if (previous->cs_change)
			cs_deassert(drv_data);
	}

	/* Check if we can DMA this transfer */
	if (!pxa2xx_spi_dma_is_possible(transfer->len) && chip->enable_dma) {

		/* reject already-mapped transfers; PIO won't always work */
		if (message->is_dma_mapped
				|| transfer->rx_dma || transfer->tx_dma) {
			dev_err(&drv_data->pdev->dev,
				"pump_transfers: mapped transfer length of "
				"%u is greater than %d\n",
				transfer->len, MAX_DMA_LEN);
			message->status = -EINVAL;
			giveback(drv_data);
			return;
		}

		/* warn ... we force this to PIO mode */
		dev_warn_ratelimited(&message->spi->dev,
				     "pump_transfers: DMA disabled for transfer length %ld "
				     "greater than %d\n",
				     (long)drv_data->len, MAX_DMA_LEN);
	}

	/* Setup the transfer state based on the type of transfer */
	if (pxa2xx_spi_flush(drv_data) == 0) {
		dev_err(&drv_data->pdev->dev, "pump_transfers: flush failed\n");
		message->status = -EIO;
		giveback(drv_data);
		return;
	}
	drv_data->n_bytes = chip->n_bytes;
	drv_data->tx = (void *)transfer->tx_buf;
	drv_data->tx_end = drv_data->tx + transfer->len;
	drv_data->rx = transfer->rx_buf;
	drv_data->rx_end = drv_data->rx + transfer->len;
	drv_data->rx_dma = transfer->rx_dma;
	drv_data->tx_dma = transfer->tx_dma;
	drv_data->len = transfer->len;
	drv_data->write = drv_data->tx ? chip->write : null_writer;
	drv_data->read = drv_data->rx ? chip->read : null_reader;

	/* Change speed and bit per word on a per transfer */
	cr0 = chip->cr0;
	if (transfer->speed_hz || transfer->bits_per_word) {

		bits = chip->bits_per_word;
		speed = chip->speed_hz;

		if (transfer->speed_hz)
			speed = transfer->speed_hz;

		if (transfer->bits_per_word)
			bits = transfer->bits_per_word;

		clk_div = pxa2xx_ssp_get_clk_div(drv_data, chip, speed);

		if (bits <= 8) {
			drv_data->n_bytes = 1;
			drv_data->read = drv_data->read != null_reader ?
						u8_reader : null_reader;
			drv_data->write = drv_data->write != null_writer ?
						u8_writer : null_writer;
		} else if (bits <= 16) {
			drv_data->n_bytes = 2;
			drv_data->read = drv_data->read != null_reader ?
						u16_reader : null_reader;
			drv_data->write = drv_data->write != null_writer ?
						u16_writer : null_writer;
		} else if (bits <= 32) {
			drv_data->n_bytes = 4;
			drv_data->read = drv_data->read != null_reader ?
						u32_reader : null_reader;
			drv_data->write = drv_data->write != null_writer ?
						u32_writer : null_writer;
		}
		/* if bits/word is changed in dma mode, then must check the
		 * thresholds and burst also */
		if (chip->enable_dma) {
			if (pxa2xx_spi_set_dma_burst_and_threshold(chip,
							message->spi,
							bits, &dma_burst,
							&dma_thresh))
				dev_warn_ratelimited(&message->spi->dev,
						     "pump_transfers: DMA burst size reduced to match bits_per_word\n");
		}

		cr0 = pxa2xx_configure_sscr0(drv_data, clk_div, bits);
	}

	message->state = RUNNING_STATE;

	drv_data->dma_mapped = 0;
	if (pxa2xx_spi_dma_is_possible(drv_data->len))
		drv_data->dma_mapped = pxa2xx_spi_map_dma_buffers(drv_data);
	if (drv_data->dma_mapped) {

		/* Ensure we have the correct interrupt handler */
		drv_data->transfer_handler = pxa2xx_spi_dma_transfer;

		pxa2xx_spi_dma_prepare(drv_data, dma_burst);

		/* Clear status and start DMA engine */
		cr1 = chip->cr1 | dma_thresh | drv_data->dma_cr1;
		pxa2xx_spi_write(drv_data, SSSR, drv_data->clear_sr);

		pxa2xx_spi_dma_start(drv_data);
	} else {
		/* Ensure we have the correct interrupt handler	*/
		drv_data->transfer_handler = interrupt_transfer;

		/* Clear status  */
		cr1 = chip->cr1 | chip->threshold | drv_data->int_cr1;
		write_SSSR_CS(drv_data, drv_data->clear_sr);
	}

	if (is_lpss_ssp(drv_data)) {
		if ((pxa2xx_spi_read(drv_data, SSIRF) & 0xff)
		    != chip->lpss_rx_threshold)
			pxa2xx_spi_write(drv_data, SSIRF,
					 chip->lpss_rx_threshold);
		if ((pxa2xx_spi_read(drv_data, SSITF) & 0xffff)
		    != chip->lpss_tx_threshold)
			pxa2xx_spi_write(drv_data, SSITF,
					 chip->lpss_tx_threshold);
	}

	if (is_quark_x1000_ssp(drv_data) &&
	    (pxa2xx_spi_read(drv_data, DDS_RATE) != chip->dds_rate))
		pxa2xx_spi_write(drv_data, DDS_RATE, chip->dds_rate);

	/* see if we need to reload the config registers */
	if ((pxa2xx_spi_read(drv_data, SSCR0) != cr0)
	    || (pxa2xx_spi_read(drv_data, SSCR1) & change_mask)
	    != (cr1 & change_mask)) {
		/* stop the SSP, and update the other bits */
		pxa2xx_spi_write(drv_data, SSCR0, cr0 & ~SSCR0_SSE);
		if (!pxa25x_ssp_comp(drv_data))
			pxa2xx_spi_write(drv_data, SSTO, chip->timeout);
		/* first set CR1 without interrupt and service enables */
		pxa2xx_spi_write(drv_data, SSCR1, cr1 & change_mask);
		/* restart the SSP */
		pxa2xx_spi_write(drv_data, SSCR0, cr0);

	} else {
		if (!pxa25x_ssp_comp(drv_data))
			pxa2xx_spi_write(drv_data, SSTO, chip->timeout);
	}

	cs_assert(drv_data);

	/* after chip select, release the data by enabling service
	 * requests and interrupts, without changing any mode bits */
	pxa2xx_spi_write(drv_data, SSCR1, cr1);
}

static int pxa2xx_spi_transfer_one_message(struct spi_master *master,
					   struct spi_message *msg)
{
	struct driver_data *drv_data = spi_master_get_devdata(master);

	drv_data->cur_msg = msg;
	/* Initial message state*/
	drv_data->cur_msg->state = START_STATE;
	drv_data->cur_transfer = list_entry(drv_data->cur_msg->transfers.next,
						struct spi_transfer,
						transfer_list);

	/* prepare to setup the SSP, in pump_transfers, using the per
	 * chip configuration */
	drv_data->cur_chip = spi_get_ctldata(drv_data->cur_msg->spi);

	/* Mark as busy and launch transfers */
	tasklet_schedule(&drv_data->pump_transfers);
	return 0;
}

static int pxa2xx_spi_unprepare_transfer(struct spi_master *master)
{
	struct driver_data *drv_data = spi_master_get_devdata(master);

	/* Disable the SSP now */
	pxa2xx_spi_write(drv_data, SSCR0,
			 pxa2xx_spi_read(drv_data, SSCR0) & ~SSCR0_SSE);

	return 0;
}

static int setup_cs(struct spi_device *spi, struct chip_data *chip,
		    struct pxa2xx_spi_chip *chip_info)
{
	int err = 0;

	if (chip == NULL || chip_info == NULL)
		return 0;

	/* NOTE: setup() can be called multiple times, possibly with
	 * different chip_info, release previously requested GPIO
	 */
	if (gpio_is_valid(chip->gpio_cs))
		gpio_free(chip->gpio_cs);

	/* If (*cs_control) is provided, ignore GPIO chip select */
	if (chip_info->cs_control) {
		chip->cs_control = chip_info->cs_control;
		return 0;
	}

	if (gpio_is_valid(chip_info->gpio_cs)) {
		err = gpio_request(chip_info->gpio_cs, "SPI_CS");
		if (err) {
			dev_err(&spi->dev, "failed to request chip select GPIO%d\n",
				chip_info->gpio_cs);
			return err;
		}

		chip->gpio_cs = chip_info->gpio_cs;
		chip->gpio_cs_inverted = spi->mode & SPI_CS_HIGH;

		err = gpio_direction_output(chip->gpio_cs,
					!chip->gpio_cs_inverted);
	}

	return err;
}

static int setup(struct spi_device *spi)
{
	struct pxa2xx_spi_chip *chip_info = NULL;
	struct chip_data *chip;
	struct driver_data *drv_data = spi_master_get_devdata(spi->master);
	unsigned int clk_div;
	uint tx_thres, tx_hi_thres, rx_thres;

	switch (drv_data->ssp_type) {
	case QUARK_X1000_SSP:
		tx_thres = TX_THRESH_QUARK_X1000_DFLT;
		tx_hi_thres = 0;
		rx_thres = RX_THRESH_QUARK_X1000_DFLT;
		break;
	case LPSS_SSP:
		tx_thres = LPSS_TX_LOTHRESH_DFLT;
		tx_hi_thres = LPSS_TX_HITHRESH_DFLT;
		rx_thres = LPSS_RX_THRESH_DFLT;
		break;
	default:
		tx_thres = TX_THRESH_DFLT;
		tx_hi_thres = 0;
		rx_thres = RX_THRESH_DFLT;
		break;
	}

	/* Only alloc on first setup */
	chip = spi_get_ctldata(spi);
	if (!chip) {
		chip = kzalloc(sizeof(struct chip_data), GFP_KERNEL);
		if (!chip)
			return -ENOMEM;

		if (drv_data->ssp_type == CE4100_SSP) {
			if (spi->chip_select > 4) {
				dev_err(&spi->dev,
					"failed setup: cs number must not be > 4.\n");
				kfree(chip);
				return -EINVAL;
			}

			chip->frm = spi->chip_select;
		} else
			chip->gpio_cs = -1;
		chip->enable_dma = 0;
		chip->timeout = TIMOUT_DFLT;
	}

	/* protocol drivers may change the chip settings, so...
	 * if chip_info exists, use it */
	chip_info = spi->controller_data;

	/* chip_info isn't always needed */
	chip->cr1 = 0;
	if (chip_info) {
		if (chip_info->timeout)
			chip->timeout = chip_info->timeout;
		if (chip_info->tx_threshold)
			tx_thres = chip_info->tx_threshold;
		if (chip_info->tx_hi_threshold)
			tx_hi_thres = chip_info->tx_hi_threshold;
		if (chip_info->rx_threshold)
			rx_thres = chip_info->rx_threshold;
		chip->enable_dma = drv_data->master_info->enable_dma;
		chip->dma_threshold = 0;
		if (chip_info->enable_loopback)
			chip->cr1 = SSCR1_LBM;
	} else if (ACPI_HANDLE(&spi->dev)) {
		/*
		 * Slave devices enumerated from ACPI namespace don't
		 * usually have chip_info but we still might want to use
		 * DMA with them.
		 */
		chip->enable_dma = drv_data->master_info->enable_dma;
	}

	chip->lpss_rx_threshold = SSIRF_RxThresh(rx_thres);
	chip->lpss_tx_threshold = SSITF_TxLoThresh(tx_thres)
				| SSITF_TxHiThresh(tx_hi_thres);

	/* set dma burst and threshold outside of chip_info path so that if
	 * chip_info goes away after setting chip->enable_dma, the
	 * burst and threshold can still respond to changes in bits_per_word */
	if (chip->enable_dma) {
		/* set up legal burst and threshold for dma */
		if (pxa2xx_spi_set_dma_burst_and_threshold(chip, spi,
						spi->bits_per_word,
						&chip->dma_burst_size,
						&chip->dma_threshold)) {
			dev_warn(&spi->dev,
				 "in setup: DMA burst size reduced to match bits_per_word\n");
		}
	}

	clk_div = pxa2xx_ssp_get_clk_div(drv_data, chip, spi->max_speed_hz);
	chip->speed_hz = spi->max_speed_hz;

	chip->cr0 = pxa2xx_configure_sscr0(drv_data, clk_div,
					   spi->bits_per_word);
	switch (drv_data->ssp_type) {
	case QUARK_X1000_SSP:
		chip->threshold = (QUARK_X1000_SSCR1_RxTresh(rx_thres)
				   & QUARK_X1000_SSCR1_RFT)
				   | (QUARK_X1000_SSCR1_TxTresh(tx_thres)
				   & QUARK_X1000_SSCR1_TFT);
		break;
	default:
		chip->threshold = (SSCR1_RxTresh(rx_thres) & SSCR1_RFT) |
			(SSCR1_TxTresh(tx_thres) & SSCR1_TFT);
		break;
	}

	chip->cr1 &= ~(SSCR1_SPO | SSCR1_SPH);
	chip->cr1 |= (((spi->mode & SPI_CPHA) != 0) ? SSCR1_SPH : 0)
			| (((spi->mode & SPI_CPOL) != 0) ? SSCR1_SPO : 0);

	if (spi->mode & SPI_LOOP)
		chip->cr1 |= SSCR1_LBM;

	/* NOTE:  PXA25x_SSP _could_ use external clocking ... */
	if (!pxa25x_ssp_comp(drv_data))
		dev_dbg(&spi->dev, "%ld Hz actual, %s\n",
			drv_data->max_clk_rate
				/ (1 + ((chip->cr0 & SSCR0_SCR(0xfff)) >> 8)),
			chip->enable_dma ? "DMA" : "PIO");
	else
		dev_dbg(&spi->dev, "%ld Hz actual, %s\n",
			drv_data->max_clk_rate / 2
				/ (1 + ((chip->cr0 & SSCR0_SCR(0x0ff)) >> 8)),
			chip->enable_dma ? "DMA" : "PIO");

	if (spi->bits_per_word <= 8) {
		chip->n_bytes = 1;
		chip->read = u8_reader;
		chip->write = u8_writer;
	} else if (spi->bits_per_word <= 16) {
		chip->n_bytes = 2;
		chip->read = u16_reader;
		chip->write = u16_writer;
	} else if (spi->bits_per_word <= 32) {
		if (!is_quark_x1000_ssp(drv_data))
			chip->cr0 |= SSCR0_EDSS;
		chip->n_bytes = 4;
		chip->read = u32_reader;
		chip->write = u32_writer;
	}
	chip->bits_per_word = spi->bits_per_word;

	spi_set_ctldata(spi, chip);

	if (drv_data->ssp_type == CE4100_SSP)
		return 0;

	return setup_cs(spi, chip, chip_info);
}

static void cleanup(struct spi_device *spi)
{
	struct chip_data *chip = spi_get_ctldata(spi);
	struct driver_data *drv_data = spi_master_get_devdata(spi->master);

	if (!chip)
		return;

	if (drv_data->ssp_type != CE4100_SSP && gpio_is_valid(chip->gpio_cs))
		gpio_free(chip->gpio_cs);

	kfree(chip);
}

#ifdef CONFIG_ACPI
static struct pxa2xx_spi_master *
pxa2xx_spi_acpi_get_pdata(struct platform_device *pdev)
{
	struct pxa2xx_spi_master *pdata;
	struct acpi_device *adev;
	struct ssp_device *ssp;
	struct resource *res;
	int devid;

	if (!ACPI_HANDLE(&pdev->dev) ||
	    acpi_bus_get_device(ACPI_HANDLE(&pdev->dev), &adev))
		return NULL;

	pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return NULL;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return NULL;

	ssp = &pdata->ssp;

	ssp->phys_base = res->start;
	ssp->mmio_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(ssp->mmio_base))
		return NULL;

	ssp->clk = devm_clk_get(&pdev->dev, NULL);
	ssp->irq = platform_get_irq(pdev, 0);
	ssp->type = LPSS_SSP;
	ssp->pdev = pdev;

	ssp->port_id = -1;
	if (adev->pnp.unique_id && !kstrtoint(adev->pnp.unique_id, 0, &devid))
		ssp->port_id = devid;

	pdata->num_chipselect = 1;
	pdata->enable_dma = true;

	return pdata;
}

static struct acpi_device_id pxa2xx_spi_acpi_match[] = {
	{ "INT33C0", 0 },
	{ "INT33C1", 0 },
	{ "INT3430", 0 },
	{ "INT3431", 0 },
	{ "80860F0E", 0 },
	{ "8086228E", 0 },
	{ },
};
MODULE_DEVICE_TABLE(acpi, pxa2xx_spi_acpi_match);
#else
static inline struct pxa2xx_spi_master *
pxa2xx_spi_acpi_get_pdata(struct platform_device *pdev)
{
	return NULL;
}
#endif

static int pxa2xx_spi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct pxa2xx_spi_master *platform_info;
	struct spi_master *master;
	struct driver_data *drv_data;
	struct ssp_device *ssp;
	int status;
	u32 tmp;

	platform_info = dev_get_platdata(dev);
	if (!platform_info) {
		platform_info = pxa2xx_spi_acpi_get_pdata(pdev);
		if (!platform_info) {
			dev_err(&pdev->dev, "missing platform data\n");
			return -ENODEV;
		}
	}

	ssp = pxa_ssp_request(pdev->id, pdev->name);
	if (!ssp)
		ssp = &platform_info->ssp;

	if (!ssp->mmio_base) {
		dev_err(&pdev->dev, "failed to get ssp\n");
		return -ENODEV;
	}

	/* Allocate master with space for drv_data and null dma buffer */
	master = spi_alloc_master(dev, sizeof(struct driver_data) + 16);
	if (!master) {
		dev_err(&pdev->dev, "cannot alloc spi_master\n");
		pxa_ssp_free(ssp);
		return -ENOMEM;
	}
	drv_data = spi_master_get_devdata(master);
	drv_data->master = master;
	drv_data->master_info = platform_info;
	drv_data->pdev = pdev;
	drv_data->ssp = ssp;

	master->dev.parent = &pdev->dev;
	master->dev.of_node = pdev->dev.of_node;
	/* the spi->mode bits understood by this driver: */
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_CS_HIGH | SPI_LOOP;

	master->bus_num = ssp->port_id;
	master->num_chipselect = platform_info->num_chipselect;
	master->dma_alignment = DMA_ALIGNMENT;
	master->cleanup = cleanup;
	master->setup = setup;
	master->transfer_one_message = pxa2xx_spi_transfer_one_message;
	master->unprepare_transfer_hardware = pxa2xx_spi_unprepare_transfer;
	master->auto_runtime_pm = true;

	drv_data->ssp_type = ssp->type;
	drv_data->null_dma_buf = (u32 *)PTR_ALIGN(&drv_data[1], DMA_ALIGNMENT);

	drv_data->ioaddr = ssp->mmio_base;
	drv_data->ssdr_physical = ssp->phys_base + SSDR;
	if (pxa25x_ssp_comp(drv_data)) {
		switch (drv_data->ssp_type) {
		case QUARK_X1000_SSP:
			master->bits_per_word_mask = SPI_BPW_RANGE_MASK(4, 32);
			break;
		default:
			master->bits_per_word_mask = SPI_BPW_RANGE_MASK(4, 16);
			break;
		}

		drv_data->int_cr1 = SSCR1_TIE | SSCR1_RIE;
		drv_data->dma_cr1 = 0;
		drv_data->clear_sr = SSSR_ROR;
		drv_data->mask_sr = SSSR_RFS | SSSR_TFS | SSSR_ROR;
	} else {
		master->bits_per_word_mask = SPI_BPW_RANGE_MASK(4, 32);
		drv_data->int_cr1 = SSCR1_TIE | SSCR1_RIE | SSCR1_TINTE;
		drv_data->dma_cr1 = DEFAULT_DMA_CR1;
		drv_data->clear_sr = SSSR_ROR | SSSR_TINT;
		drv_data->mask_sr = SSSR_TINT | SSSR_RFS | SSSR_TFS | SSSR_ROR;
	}

	status = request_irq(ssp->irq, ssp_int, IRQF_SHARED, dev_name(dev),
			drv_data);
	if (status < 0) {
		dev_err(&pdev->dev, "cannot get IRQ %d\n", ssp->irq);
		goto out_error_master_alloc;
	}

	/* Setup DMA if requested */
	drv_data->tx_channel = -1;
	drv_data->rx_channel = -1;
	if (platform_info->enable_dma) {
		status = pxa2xx_spi_dma_setup(drv_data);
		if (status) {
			dev_dbg(dev, "no DMA channels available, using PIO\n");
			platform_info->enable_dma = false;
		}
	}

	/* Enable SOC clock */
	clk_prepare_enable(ssp->clk);

	drv_data->max_clk_rate = clk_get_rate(ssp->clk);

	/* Load default SSP configuration */
	pxa2xx_spi_write(drv_data, SSCR0, 0);
	switch (drv_data->ssp_type) {
	case QUARK_X1000_SSP:
		tmp = QUARK_X1000_SSCR1_RxTresh(RX_THRESH_QUARK_X1000_DFLT)
		      | QUARK_X1000_SSCR1_TxTresh(TX_THRESH_QUARK_X1000_DFLT);
		pxa2xx_spi_write(drv_data, SSCR1, tmp);

		/* using the Motorola SPI protocol and use 8 bit frame */
		pxa2xx_spi_write(drv_data, SSCR0,
				 QUARK_X1000_SSCR0_Motorola
				 | QUARK_X1000_SSCR0_DataSize(8));
		break;
	default:
		tmp = SSCR1_RxTresh(RX_THRESH_DFLT) |
		      SSCR1_TxTresh(TX_THRESH_DFLT);
		pxa2xx_spi_write(drv_data, SSCR1, tmp);
		tmp = SSCR0_SCR(2) | SSCR0_Motorola | SSCR0_DataSize(8);
		pxa2xx_spi_write(drv_data, SSCR0, tmp);
		break;
	}

	if (!pxa25x_ssp_comp(drv_data))
		pxa2xx_spi_write(drv_data, SSTO, 0);

	if (!is_quark_x1000_ssp(drv_data))
		pxa2xx_spi_write(drv_data, SSPSP, 0);

	if (is_lpss_ssp(drv_data))
		lpss_ssp_setup(drv_data);

	tasklet_init(&drv_data->pump_transfers, pump_transfers,
		     (unsigned long)drv_data);

	pm_runtime_set_autosuspend_delay(&pdev->dev, 50);
	pm_runtime_use_autosuspend(&pdev->dev);
	pm_runtime_set_active(&pdev->dev);
	pm_runtime_enable(&pdev->dev);

	/* Register with the SPI framework */
	platform_set_drvdata(pdev, drv_data);
	status = devm_spi_register_master(&pdev->dev, master);
	if (status != 0) {
		dev_err(&pdev->dev, "problem registering spi master\n");
		goto out_error_clock_enabled;
	}

	return status;

out_error_clock_enabled:
	clk_disable_unprepare(ssp->clk);
	pxa2xx_spi_dma_release(drv_data);
	free_irq(ssp->irq, drv_data);

out_error_master_alloc:
	spi_master_put(master);
	pxa_ssp_free(ssp);
	return status;
}

static int pxa2xx_spi_remove(struct platform_device *pdev)
{
	struct driver_data *drv_data = platform_get_drvdata(pdev);
	struct ssp_device *ssp;

	if (!drv_data)
		return 0;
	ssp = drv_data->ssp;

	pm_runtime_get_sync(&pdev->dev);

	/* Disable the SSP at the peripheral and SOC level */
	pxa2xx_spi_write(drv_data, SSCR0, 0);
	clk_disable_unprepare(ssp->clk);

	/* Release DMA */
	if (drv_data->master_info->enable_dma)
		pxa2xx_spi_dma_release(drv_data);

	pm_runtime_put_noidle(&pdev->dev);
	pm_runtime_disable(&pdev->dev);

	/* Release IRQ */
	free_irq(ssp->irq, drv_data);

	/* Release SSP */
	pxa_ssp_free(ssp);

	return 0;
}

static void pxa2xx_spi_shutdown(struct platform_device *pdev)
{
	int status = 0;

	if ((status = pxa2xx_spi_remove(pdev)) != 0)
		dev_err(&pdev->dev, "shutdown failed with %d\n", status);
}

#ifdef CONFIG_PM_SLEEP
static int pxa2xx_spi_suspend(struct device *dev)
{
	struct driver_data *drv_data = dev_get_drvdata(dev);
	struct ssp_device *ssp = drv_data->ssp;
	int status = 0;

	status = spi_master_suspend(drv_data->master);
	if (status != 0)
		return status;
	pxa2xx_spi_write(drv_data, SSCR0, 0);

	if (!pm_runtime_suspended(dev))
		clk_disable_unprepare(ssp->clk);

	return 0;
}

static int pxa2xx_spi_resume(struct device *dev)
{
	struct driver_data *drv_data = dev_get_drvdata(dev);
	struct ssp_device *ssp = drv_data->ssp;
	int status = 0;

	pxa2xx_spi_dma_resume(drv_data);

	/* Enable the SSP clock */
	if (!pm_runtime_suspended(dev))
		clk_prepare_enable(ssp->clk);

	/* Restore LPSS private register bits */
	if (is_lpss_ssp(drv_data))
		lpss_ssp_setup(drv_data);

	/* Start the queue running */
	status = spi_master_resume(drv_data->master);
	if (status != 0) {
		dev_err(dev, "problem starting queue (%d)\n", status);
		return status;
	}

	return 0;
}
#endif

#ifdef CONFIG_PM
static int pxa2xx_spi_runtime_suspend(struct device *dev)
{
	struct driver_data *drv_data = dev_get_drvdata(dev);

	clk_disable_unprepare(drv_data->ssp->clk);
	return 0;
}

static int pxa2xx_spi_runtime_resume(struct device *dev)
{
	struct driver_data *drv_data = dev_get_drvdata(dev);

	clk_prepare_enable(drv_data->ssp->clk);
	return 0;
}
#endif

static const struct dev_pm_ops pxa2xx_spi_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(pxa2xx_spi_suspend, pxa2xx_spi_resume)
	SET_RUNTIME_PM_OPS(pxa2xx_spi_runtime_suspend,
			   pxa2xx_spi_runtime_resume, NULL)
};

static struct platform_driver driver = {
	.driver = {
		.name	= "pxa2xx-spi",
		.pm	= &pxa2xx_spi_pm_ops,
		.acpi_match_table = ACPI_PTR(pxa2xx_spi_acpi_match),
	},
	.probe = pxa2xx_spi_probe,
	.remove = pxa2xx_spi_remove,
	.shutdown = pxa2xx_spi_shutdown,
};

static int __init pxa2xx_spi_init(void)
{
	return platform_driver_register(&driver);
}
subsys_initcall(pxa2xx_spi_init);

static void __exit pxa2xx_spi_exit(void)
{
	platform_driver_unregister(&driver);
}
module_exit(pxa2xx_spi_exit);
