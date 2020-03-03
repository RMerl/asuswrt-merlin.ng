/*
 * Analog Devices SPI3 controller driver
 *
 * Copyright (c) 2014 Analog Devices Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/spi/adi_spi3.h>
#include <linux/types.h>

#include <asm/dma.h>
#include <asm/portmux.h>

enum adi_spi_state {
	START_STATE,
	RUNNING_STATE,
	DONE_STATE,
	ERROR_STATE
};

struct adi_spi_master;

struct adi_spi_transfer_ops {
	void (*write) (struct adi_spi_master *);
	void (*read) (struct adi_spi_master *);
	void (*duplex) (struct adi_spi_master *);
};

/* runtime info for spi master */
struct adi_spi_master {
	/* SPI framework hookup */
	struct spi_master *master;

	/* Regs base of SPI controller */
	struct adi_spi_regs __iomem *regs;

	/* Pin request list */
	u16 *pin_req;

	/* Message Transfer pump */
	struct tasklet_struct pump_transfers;

	/* Current message transfer state info */
	struct spi_message *cur_msg;
	struct spi_transfer *cur_transfer;
	struct adi_spi_device *cur_chip;
	unsigned transfer_len;

	/* transfer buffer */
	void *tx;
	void *tx_end;
	void *rx;
	void *rx_end;

	/* dma info */
	unsigned int tx_dma;
	unsigned int rx_dma;
	dma_addr_t tx_dma_addr;
	dma_addr_t rx_dma_addr;
	unsigned long dummy_buffer; /* used in unidirectional transfer */
	unsigned long tx_dma_size;
	unsigned long rx_dma_size;
	int tx_num;
	int rx_num;

	/* store register value for suspend/resume */
	u32 control;
	u32 ssel;

	unsigned long sclk;
	enum adi_spi_state state;

	const struct adi_spi_transfer_ops *ops;
};

struct adi_spi_device {
	u32 control;
	u32 clock;
	u32 ssel;

	u8 cs;
	u16 cs_chg_udelay; /* Some devices require > 255usec delay */
	u32 cs_gpio;
	u32 tx_dummy_val; /* tx value for rx only transfer */
	bool enable_dma;
	const struct adi_spi_transfer_ops *ops;
};

static void adi_spi_enable(struct adi_spi_master *drv_data)
{
	u32 ctl;

	ctl = ioread32(&drv_data->regs->control);
	ctl |= SPI_CTL_EN;
	iowrite32(ctl, &drv_data->regs->control);
}

static void adi_spi_disable(struct adi_spi_master *drv_data)
{
	u32 ctl;

	ctl = ioread32(&drv_data->regs->control);
	ctl &= ~SPI_CTL_EN;
	iowrite32(ctl, &drv_data->regs->control);
}

/* Caculate the SPI_CLOCK register value based on input HZ */
static u32 hz_to_spi_clock(u32 sclk, u32 speed_hz)
{
	u32 spi_clock = sclk / speed_hz;

	if (spi_clock)
		spi_clock--;
	return spi_clock;
}

static int adi_spi_flush(struct adi_spi_master *drv_data)
{
	unsigned long limit = loops_per_jiffy << 1;

	/* wait for stop and clear stat */
	while (!(ioread32(&drv_data->regs->status) & SPI_STAT_SPIF) && --limit)
		cpu_relax();

	iowrite32(0xFFFFFFFF, &drv_data->regs->status);

	return limit;
}

/* Chip select operation functions for cs_change flag */
static void adi_spi_cs_active(struct adi_spi_master *drv_data, struct adi_spi_device *chip)
{
	if (likely(chip->cs < MAX_CTRL_CS)) {
		u32 reg;
		reg = ioread32(&drv_data->regs->ssel);
		reg &= ~chip->ssel;
		iowrite32(reg, &drv_data->regs->ssel);
	} else {
		gpio_set_value(chip->cs_gpio, 0);
	}
}

static void adi_spi_cs_deactive(struct adi_spi_master *drv_data,
				struct adi_spi_device *chip)
{
	if (likely(chip->cs < MAX_CTRL_CS)) {
		u32 reg;
		reg = ioread32(&drv_data->regs->ssel);
		reg |= chip->ssel;
		iowrite32(reg, &drv_data->regs->ssel);
	} else {
		gpio_set_value(chip->cs_gpio, 1);
	}

	/* Move delay here for consistency */
	if (chip->cs_chg_udelay)
		udelay(chip->cs_chg_udelay);
}

/* enable or disable the pin muxed by GPIO and SPI CS to work as SPI CS */
static inline void adi_spi_cs_enable(struct adi_spi_master *drv_data,
					struct adi_spi_device *chip)
{
	if (chip->cs < MAX_CTRL_CS) {
		u32 reg;
		reg = ioread32(&drv_data->regs->ssel);
		reg |= chip->ssel >> 8;
		iowrite32(reg, &drv_data->regs->ssel);
	}
}

static inline void adi_spi_cs_disable(struct adi_spi_master *drv_data,
					struct adi_spi_device *chip)
{
	if (chip->cs < MAX_CTRL_CS) {
		u32 reg;
		reg = ioread32(&drv_data->regs->ssel);
		reg &= ~(chip->ssel >> 8);
		iowrite32(reg, &drv_data->regs->ssel);
	}
}

/* stop controller and re-config current chip*/
static void adi_spi_restore_state(struct adi_spi_master *drv_data)
{
	struct adi_spi_device *chip = drv_data->cur_chip;

	/* Clear status and disable clock */
	iowrite32(0xFFFFFFFF, &drv_data->regs->status);
	iowrite32(0x0, &drv_data->regs->rx_control);
	iowrite32(0x0, &drv_data->regs->tx_control);
	adi_spi_disable(drv_data);

	/* Load the registers */
	iowrite32(chip->control, &drv_data->regs->control);
	iowrite32(chip->clock, &drv_data->regs->clock);

	adi_spi_enable(drv_data);
	drv_data->tx_num = drv_data->rx_num = 0;
	/* we always choose tx transfer initiate */
	iowrite32(SPI_RXCTL_REN, &drv_data->regs->rx_control);
	iowrite32(SPI_TXCTL_TEN | SPI_TXCTL_TTI, &drv_data->regs->tx_control);
	adi_spi_cs_active(drv_data, chip);
}

/* discard invalid rx data and empty rfifo */
static inline void dummy_read(struct adi_spi_master *drv_data)
{
	while (!(ioread32(&drv_data->regs->status) & SPI_STAT_RFE))
		ioread32(&drv_data->regs->rfifo);
}

static void adi_spi_u8_write(struct adi_spi_master *drv_data)
{
	dummy_read(drv_data);
	while (drv_data->tx < drv_data->tx_end) {
		iowrite32(*(u8 *)(drv_data->tx++), &drv_data->regs->tfifo);
		while (ioread32(&drv_data->regs->status) & SPI_STAT_RFE)
			cpu_relax();
		ioread32(&drv_data->regs->rfifo);
	}
}

static void adi_spi_u8_read(struct adi_spi_master *drv_data)
{
	u32 tx_val = drv_data->cur_chip->tx_dummy_val;

	dummy_read(drv_data);
	while (drv_data->rx < drv_data->rx_end) {
		iowrite32(tx_val, &drv_data->regs->tfifo);
		while (ioread32(&drv_data->regs->status) & SPI_STAT_RFE)
			cpu_relax();
		*(u8 *)(drv_data->rx++) = ioread32(&drv_data->regs->rfifo);
	}
}

static void adi_spi_u8_duplex(struct adi_spi_master *drv_data)
{
	dummy_read(drv_data);
	while (drv_data->rx < drv_data->rx_end) {
		iowrite32(*(u8 *)(drv_data->tx++), &drv_data->regs->tfifo);
		while (ioread32(&drv_data->regs->status) & SPI_STAT_RFE)
			cpu_relax();
		*(u8 *)(drv_data->rx++) = ioread32(&drv_data->regs->rfifo);
	}
}

static const struct adi_spi_transfer_ops adi_spi_transfer_ops_u8 = {
	.write  = adi_spi_u8_write,
	.read   = adi_spi_u8_read,
	.duplex = adi_spi_u8_duplex,
};

static void adi_spi_u16_write(struct adi_spi_master *drv_data)
{
	dummy_read(drv_data);
	while (drv_data->tx < drv_data->tx_end) {
		iowrite32(*(u16 *)drv_data->tx, &drv_data->regs->tfifo);
		drv_data->tx += 2;
		while (ioread32(&drv_data->regs->status) & SPI_STAT_RFE)
			cpu_relax();
		ioread32(&drv_data->regs->rfifo);
	}
}

static void adi_spi_u16_read(struct adi_spi_master *drv_data)
{
	u32 tx_val = drv_data->cur_chip->tx_dummy_val;

	dummy_read(drv_data);
	while (drv_data->rx < drv_data->rx_end) {
		iowrite32(tx_val, &drv_data->regs->tfifo);
		while (ioread32(&drv_data->regs->status) & SPI_STAT_RFE)
			cpu_relax();
		*(u16 *)drv_data->rx = ioread32(&drv_data->regs->rfifo);
		drv_data->rx += 2;
	}
}

static void adi_spi_u16_duplex(struct adi_spi_master *drv_data)
{
	dummy_read(drv_data);
	while (drv_data->rx < drv_data->rx_end) {
		iowrite32(*(u16 *)drv_data->tx, &drv_data->regs->tfifo);
		drv_data->tx += 2;
		while (ioread32(&drv_data->regs->status) & SPI_STAT_RFE)
			cpu_relax();
		*(u16 *)drv_data->rx = ioread32(&drv_data->regs->rfifo);
		drv_data->rx += 2;
	}
}

static const struct adi_spi_transfer_ops adi_spi_transfer_ops_u16 = {
	.write  = adi_spi_u16_write,
	.read   = adi_spi_u16_read,
	.duplex = adi_spi_u16_duplex,
};

static void adi_spi_u32_write(struct adi_spi_master *drv_data)
{
	dummy_read(drv_data);
	while (drv_data->tx < drv_data->tx_end) {
		iowrite32(*(u32 *)drv_data->tx, &drv_data->regs->tfifo);
		drv_data->tx += 4;
		while (ioread32(&drv_data->regs->status) & SPI_STAT_RFE)
			cpu_relax();
		ioread32(&drv_data->regs->rfifo);
	}
}

static void adi_spi_u32_read(struct adi_spi_master *drv_data)
{
	u32 tx_val = drv_data->cur_chip->tx_dummy_val;

	dummy_read(drv_data);
	while (drv_data->rx < drv_data->rx_end) {
		iowrite32(tx_val, &drv_data->regs->tfifo);
		while (ioread32(&drv_data->regs->status) & SPI_STAT_RFE)
			cpu_relax();
		*(u32 *)drv_data->rx = ioread32(&drv_data->regs->rfifo);
		drv_data->rx += 4;
	}
}

static void adi_spi_u32_duplex(struct adi_spi_master *drv_data)
{
	dummy_read(drv_data);
	while (drv_data->rx < drv_data->rx_end) {
		iowrite32(*(u32 *)drv_data->tx, &drv_data->regs->tfifo);
		drv_data->tx += 4;
		while (ioread32(&drv_data->regs->status) & SPI_STAT_RFE)
			cpu_relax();
		*(u32 *)drv_data->rx = ioread32(&drv_data->regs->rfifo);
		drv_data->rx += 4;
	}
}

static const struct adi_spi_transfer_ops adi_spi_transfer_ops_u32 = {
	.write  = adi_spi_u32_write,
	.read   = adi_spi_u32_read,
	.duplex = adi_spi_u32_duplex,
};


/* test if there is more transfer to be done */
static void adi_spi_next_transfer(struct adi_spi_master *drv)
{
	struct spi_message *msg = drv->cur_msg;
	struct spi_transfer *t = drv->cur_transfer;

	/* Move to next transfer */
	if (t->transfer_list.next != &msg->transfers) {
		drv->cur_transfer = list_entry(t->transfer_list.next,
			       struct spi_transfer, transfer_list);
		drv->state = RUNNING_STATE;
	} else {
		drv->state = DONE_STATE;
		drv->cur_transfer = NULL;
	}
}

static void adi_spi_giveback(struct adi_spi_master *drv_data)
{
	struct adi_spi_device *chip = drv_data->cur_chip;

	adi_spi_cs_deactive(drv_data, chip);
	spi_finalize_current_message(drv_data->master);
}

static int adi_spi_setup_transfer(struct adi_spi_master *drv)
{
	struct spi_transfer *t = drv->cur_transfer;
	u32 cr, cr_width;

	if (t->tx_buf) {
		drv->tx = (void *)t->tx_buf;
		drv->tx_end = drv->tx + t->len;
	} else {
		drv->tx = NULL;
	}

	if (t->rx_buf) {
		drv->rx = t->rx_buf;
		drv->rx_end = drv->rx + t->len;
	} else {
		drv->rx = NULL;
	}

	drv->transfer_len = t->len;

	/* bits per word setup */
	switch (t->bits_per_word) {
	case 8:
		cr_width = SPI_CTL_SIZE08;
		drv->ops = &adi_spi_transfer_ops_u8;
		break;
	case 16:
		cr_width = SPI_CTL_SIZE16;
		drv->ops = &adi_spi_transfer_ops_u16;
		break;
	case 32:
		cr_width = SPI_CTL_SIZE32;
		drv->ops = &adi_spi_transfer_ops_u32;
		break;
	default:
		return -EINVAL;
	}
	cr = ioread32(&drv->regs->control) & ~SPI_CTL_SIZE;
	cr |= cr_width;
	iowrite32(cr, &drv->regs->control);

	/* speed setup */
	iowrite32(hz_to_spi_clock(drv->sclk, t->speed_hz), &drv->regs->clock);
	return 0;
}

static int adi_spi_dma_xfer(struct adi_spi_master *drv_data)
{
	struct spi_transfer *t = drv_data->cur_transfer;
	struct spi_message *msg = drv_data->cur_msg;
	struct adi_spi_device *chip = drv_data->cur_chip;
	u32 dma_config;
	unsigned long word_count, word_size;
	void *tx_buf, *rx_buf;

	switch (t->bits_per_word) {
	case 8:
		dma_config = WDSIZE_8 | PSIZE_8;
		word_count = drv_data->transfer_len;
		word_size = 1;
		break;
	case 16:
		dma_config = WDSIZE_16 | PSIZE_16;
		word_count = drv_data->transfer_len / 2;
		word_size = 2;
		break;
	default:
		dma_config = WDSIZE_32 | PSIZE_32;
		word_count = drv_data->transfer_len / 4;
		word_size = 4;
		break;
	}

	if (!drv_data->rx) {
		tx_buf = drv_data->tx;
		rx_buf = &drv_data->dummy_buffer;
		drv_data->tx_dma_size = drv_data->transfer_len;
		drv_data->rx_dma_size = sizeof(drv_data->dummy_buffer);
		set_dma_x_modify(drv_data->tx_dma, word_size);
		set_dma_x_modify(drv_data->rx_dma, 0);
	} else if (!drv_data->tx) {
		drv_data->dummy_buffer = chip->tx_dummy_val;
		tx_buf = &drv_data->dummy_buffer;
		rx_buf = drv_data->rx;
		drv_data->tx_dma_size = sizeof(drv_data->dummy_buffer);
		drv_data->rx_dma_size = drv_data->transfer_len;
		set_dma_x_modify(drv_data->tx_dma, 0);
		set_dma_x_modify(drv_data->rx_dma, word_size);
	} else {
		tx_buf = drv_data->tx;
		rx_buf = drv_data->rx;
		drv_data->tx_dma_size = drv_data->rx_dma_size
					= drv_data->transfer_len;
		set_dma_x_modify(drv_data->tx_dma, word_size);
		set_dma_x_modify(drv_data->rx_dma, word_size);
	}

	drv_data->tx_dma_addr = dma_map_single(&msg->spi->dev,
				(void *)tx_buf,
				drv_data->tx_dma_size,
				DMA_TO_DEVICE);
	if (dma_mapping_error(&msg->spi->dev,
				drv_data->tx_dma_addr))
		return -ENOMEM;

	drv_data->rx_dma_addr = dma_map_single(&msg->spi->dev,
				(void *)rx_buf,
				drv_data->rx_dma_size,
				DMA_FROM_DEVICE);
	if (dma_mapping_error(&msg->spi->dev,
				drv_data->rx_dma_addr)) {
		dma_unmap_single(&msg->spi->dev,
				drv_data->tx_dma_addr,
				drv_data->tx_dma_size,
				DMA_TO_DEVICE);
		return -ENOMEM;
	}

	dummy_read(drv_data);
	set_dma_x_count(drv_data->tx_dma, word_count);
	set_dma_x_count(drv_data->rx_dma, word_count);
	set_dma_start_addr(drv_data->tx_dma, drv_data->tx_dma_addr);
	set_dma_start_addr(drv_data->rx_dma, drv_data->rx_dma_addr);
	dma_config |= DMAFLOW_STOP | RESTART | DI_EN;
	set_dma_config(drv_data->tx_dma, dma_config);
	set_dma_config(drv_data->rx_dma, dma_config | WNR);
	enable_dma(drv_data->tx_dma);
	enable_dma(drv_data->rx_dma);

	iowrite32(SPI_RXCTL_REN | SPI_RXCTL_RDR_NE,
			&drv_data->regs->rx_control);
	iowrite32(SPI_TXCTL_TEN | SPI_TXCTL_TTI | SPI_TXCTL_TDR_NF,
			&drv_data->regs->tx_control);

	return 0;
}

static int adi_spi_pio_xfer(struct adi_spi_master *drv_data)
{
	struct spi_message *msg = drv_data->cur_msg;

	if (!drv_data->rx) {
		/* write only half duplex */
		drv_data->ops->write(drv_data);
		if (drv_data->tx != drv_data->tx_end)
			return -EIO;
	} else if (!drv_data->tx) {
		/* read only half duplex */
		drv_data->ops->read(drv_data);
		if (drv_data->rx != drv_data->rx_end)
			return -EIO;
	} else {
		/* full duplex mode */
		drv_data->ops->duplex(drv_data);
		if (drv_data->tx != drv_data->tx_end)
			return -EIO;
	}

	if (!adi_spi_flush(drv_data))
		return -EIO;
	msg->actual_length += drv_data->transfer_len;
	tasklet_schedule(&drv_data->pump_transfers);
	return 0;
}

static void adi_spi_pump_transfers(unsigned long data)
{
	struct adi_spi_master *drv_data = (struct adi_spi_master *)data;
	struct spi_message *msg = NULL;
	struct spi_transfer *t = NULL;
	struct adi_spi_device *chip = NULL;
	int ret;

	/* Get current state information */
	msg = drv_data->cur_msg;
	t = drv_data->cur_transfer;
	chip = drv_data->cur_chip;

	/* Handle for abort */
	if (drv_data->state == ERROR_STATE) {
		msg->status = -EIO;
		adi_spi_giveback(drv_data);
		return;
	}

	if (drv_data->state == RUNNING_STATE) {
		if (t->delay_usecs)
			udelay(t->delay_usecs);
		if (t->cs_change)
			adi_spi_cs_deactive(drv_data, chip);
		adi_spi_next_transfer(drv_data);
		t = drv_data->cur_transfer;
	}
	/* Handle end of message */
	if (drv_data->state == DONE_STATE) {
		msg->status = 0;
		adi_spi_giveback(drv_data);
		return;
	}

	if ((t->len == 0) || (t->tx_buf == NULL && t->rx_buf == NULL)) {
		/* Schedule next transfer tasklet */
		tasklet_schedule(&drv_data->pump_transfers);
		return;
	}

	ret = adi_spi_setup_transfer(drv_data);
	if (ret) {
		msg->status = ret;
		adi_spi_giveback(drv_data);
	}

	iowrite32(0xFFFFFFFF, &drv_data->regs->status);
	adi_spi_cs_active(drv_data, chip);
	drv_data->state = RUNNING_STATE;

	if (chip->enable_dma)
		ret = adi_spi_dma_xfer(drv_data);
	else
		ret = adi_spi_pio_xfer(drv_data);
	if (ret) {
		msg->status = ret;
		adi_spi_giveback(drv_data);
	}
}

static int adi_spi_transfer_one_message(struct spi_master *master,
					struct spi_message *m)
{
	struct adi_spi_master *drv_data = spi_master_get_devdata(master);

	drv_data->cur_msg = m;
	drv_data->cur_chip = spi_get_ctldata(drv_data->cur_msg->spi);
	adi_spi_restore_state(drv_data);

	drv_data->state = START_STATE;
	drv_data->cur_transfer = list_entry(drv_data->cur_msg->transfers.next,
					    struct spi_transfer, transfer_list);

	tasklet_schedule(&drv_data->pump_transfers);
	return 0;
}

#define MAX_SPI_SSEL	7

static const u16 ssel[][MAX_SPI_SSEL] = {
	{P_SPI0_SSEL1, P_SPI0_SSEL2, P_SPI0_SSEL3,
	P_SPI0_SSEL4, P_SPI0_SSEL5,
	P_SPI0_SSEL6, P_SPI0_SSEL7},

	{P_SPI1_SSEL1, P_SPI1_SSEL2, P_SPI1_SSEL3,
	P_SPI1_SSEL4, P_SPI1_SSEL5,
	P_SPI1_SSEL6, P_SPI1_SSEL7},

	{P_SPI2_SSEL1, P_SPI2_SSEL2, P_SPI2_SSEL3,
	P_SPI2_SSEL4, P_SPI2_SSEL5,
	P_SPI2_SSEL6, P_SPI2_SSEL7},
};

static int adi_spi_setup(struct spi_device *spi)
{
	struct adi_spi_master *drv_data = spi_master_get_devdata(spi->master);
	struct adi_spi_device *chip = spi_get_ctldata(spi);
	u32 ctl_reg = SPI_CTL_ODM | SPI_CTL_PSSE;
	int ret = -EINVAL;

	if (!chip) {
		struct adi_spi3_chip *chip_info = spi->controller_data;

		chip = kzalloc(sizeof(*chip), GFP_KERNEL);
		if (!chip)
			return -ENOMEM;

		if (chip_info) {
			if (chip_info->control & ~ctl_reg) {
				dev_err(&spi->dev,
					"do not set bits that the SPI framework manages\n");
				goto error;
			}
			chip->control = chip_info->control;
			chip->cs_chg_udelay = chip_info->cs_chg_udelay;
			chip->tx_dummy_val = chip_info->tx_dummy_val;
			chip->enable_dma = chip_info->enable_dma;
		}
		chip->cs = spi->chip_select;

		if (chip->cs < MAX_CTRL_CS) {
			chip->ssel = (1 << chip->cs) << 8;
			ret = peripheral_request(ssel[spi->master->bus_num]
					[chip->cs-1], dev_name(&spi->dev));
			if (ret) {
				dev_err(&spi->dev, "peripheral_request() error\n");
				goto error;
			}
		} else {
			chip->cs_gpio = chip->cs - MAX_CTRL_CS;
			ret = gpio_request_one(chip->cs_gpio, GPIOF_OUT_INIT_HIGH,
						dev_name(&spi->dev));
			if (ret) {
				dev_err(&spi->dev, "gpio_request_one() error\n");
				goto error;
			}
		}
		spi_set_ctldata(spi, chip);
	}

	/* force a default base state */
	chip->control &= ctl_reg;

	if (spi->mode & SPI_CPOL)
		chip->control |= SPI_CTL_CPOL;
	if (spi->mode & SPI_CPHA)
		chip->control |= SPI_CTL_CPHA;
	if (spi->mode & SPI_LSB_FIRST)
		chip->control |= SPI_CTL_LSBF;
	chip->control |= SPI_CTL_MSTR;
	/* we choose software to controll cs */
	chip->control &= ~SPI_CTL_ASSEL;

	chip->clock = hz_to_spi_clock(drv_data->sclk, spi->max_speed_hz);

	adi_spi_cs_enable(drv_data, chip);
	adi_spi_cs_deactive(drv_data, chip);

	return 0;
error:
	if (chip) {
		kfree(chip);
		spi_set_ctldata(spi, NULL);
	}

	return ret;
}

static void adi_spi_cleanup(struct spi_device *spi)
{
	struct adi_spi_device *chip = spi_get_ctldata(spi);
	struct adi_spi_master *drv_data = spi_master_get_devdata(spi->master);

	if (!chip)
		return;

	if (chip->cs < MAX_CTRL_CS) {
		peripheral_free(ssel[spi->master->bus_num]
					[chip->cs-1]);
		adi_spi_cs_disable(drv_data, chip);
	} else {
		gpio_free(chip->cs_gpio);
	}

	kfree(chip);
	spi_set_ctldata(spi, NULL);
}

static irqreturn_t adi_spi_tx_dma_isr(int irq, void *dev_id)
{
	struct adi_spi_master *drv_data = dev_id;
	u32 dma_stat = get_dma_curr_irqstat(drv_data->tx_dma);
	u32 tx_ctl;

	clear_dma_irqstat(drv_data->tx_dma);
	if (dma_stat & DMA_DONE) {
		drv_data->tx_num++;
	} else {
		dev_err(&drv_data->master->dev,
				"spi tx dma error: %d\n", dma_stat);
		if (drv_data->tx)
			drv_data->state = ERROR_STATE;
	}
	tx_ctl = ioread32(&drv_data->regs->tx_control);
	tx_ctl &= ~SPI_TXCTL_TDR_NF;
	iowrite32(tx_ctl, &drv_data->regs->tx_control);
	return IRQ_HANDLED;
}

static irqreturn_t adi_spi_rx_dma_isr(int irq, void *dev_id)
{
	struct adi_spi_master *drv_data = dev_id;
	struct spi_message *msg = drv_data->cur_msg;
	u32 dma_stat = get_dma_curr_irqstat(drv_data->rx_dma);

	clear_dma_irqstat(drv_data->rx_dma);
	if (dma_stat & DMA_DONE) {
		drv_data->rx_num++;
		/* we may fail on tx dma */
		if (drv_data->state != ERROR_STATE)
			msg->actual_length += drv_data->transfer_len;
	} else {
		drv_data->state = ERROR_STATE;
		dev_err(&drv_data->master->dev,
				"spi rx dma error: %d\n", dma_stat);
	}
	iowrite32(0, &drv_data->regs->tx_control);
	iowrite32(0, &drv_data->regs->rx_control);
	if (drv_data->rx_num != drv_data->tx_num)
		dev_dbg(&drv_data->master->dev,
				"dma interrupt missing: tx=%d,rx=%d\n",
				drv_data->tx_num, drv_data->rx_num);
	tasklet_schedule(&drv_data->pump_transfers);
	return IRQ_HANDLED;
}

static int adi_spi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct adi_spi3_master *info = dev_get_platdata(dev);
	struct spi_master *master;
	struct adi_spi_master *drv_data;
	struct resource *mem, *res;
	unsigned int tx_dma, rx_dma;
	struct clk *sclk;
	int ret;

	if (!info) {
		dev_err(dev, "platform data missing!\n");
		return -ENODEV;
	}

	sclk = devm_clk_get(dev, "spi");
	if (IS_ERR(sclk)) {
		dev_err(dev, "can not get spi clock\n");
		return PTR_ERR(sclk);
	}

	res = platform_get_resource(pdev, IORESOURCE_DMA, 0);
	if (!res) {
		dev_err(dev, "can not get tx dma resource\n");
		return -ENXIO;
	}
	tx_dma = res->start;

	res = platform_get_resource(pdev, IORESOURCE_DMA, 1);
	if (!res) {
		dev_err(dev, "can not get rx dma resource\n");
		return -ENXIO;
	}
	rx_dma = res->start;

	/* allocate master with space for drv_data */
	master = spi_alloc_master(dev, sizeof(*drv_data));
	if (!master) {
		dev_err(dev, "can not alloc spi_master\n");
		return -ENOMEM;
	}
	platform_set_drvdata(pdev, master);

	/* the mode bits supported by this driver */
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_LSB_FIRST;

	master->bus_num = pdev->id;
	master->num_chipselect = info->num_chipselect;
	master->cleanup = adi_spi_cleanup;
	master->setup = adi_spi_setup;
	master->transfer_one_message = adi_spi_transfer_one_message;
	master->bits_per_word_mask = SPI_BPW_MASK(32) | SPI_BPW_MASK(16) |
				     SPI_BPW_MASK(8);

	drv_data = spi_master_get_devdata(master);
	drv_data->master = master;
	drv_data->tx_dma = tx_dma;
	drv_data->rx_dma = rx_dma;
	drv_data->pin_req = info->pin_req;
	drv_data->sclk = clk_get_rate(sclk);

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	drv_data->regs = devm_ioremap_resource(dev, mem);
	if (IS_ERR(drv_data->regs)) {
		ret = PTR_ERR(drv_data->regs);
		goto err_put_master;
	}

	/* request tx and rx dma */
	ret = request_dma(tx_dma, "SPI_TX_DMA");
	if (ret) {
		dev_err(dev, "can not request SPI TX DMA channel\n");
		goto err_put_master;
	}
	set_dma_callback(tx_dma, adi_spi_tx_dma_isr, drv_data);

	ret = request_dma(rx_dma, "SPI_RX_DMA");
	if (ret) {
		dev_err(dev, "can not request SPI RX DMA channel\n");
		goto err_free_tx_dma;
	}
	set_dma_callback(drv_data->rx_dma, adi_spi_rx_dma_isr, drv_data);

	/* request CLK, MOSI and MISO */
	ret = peripheral_request_list(drv_data->pin_req, "adi-spi3");
	if (ret < 0) {
		dev_err(dev, "can not request spi pins\n");
		goto err_free_rx_dma;
	}

	iowrite32(SPI_CTL_MSTR | SPI_CTL_CPHA, &drv_data->regs->control);
	iowrite32(0x0000FE00, &drv_data->regs->ssel);
	iowrite32(0x0, &drv_data->regs->delay);

	tasklet_init(&drv_data->pump_transfers,
			adi_spi_pump_transfers, (unsigned long)drv_data);
	/* register with the SPI framework */
	ret = devm_spi_register_master(dev, master);
	if (ret) {
		dev_err(dev, "can not  register spi master\n");
		goto err_free_peripheral;
	}

	return ret;

err_free_peripheral:
	peripheral_free_list(drv_data->pin_req);
err_free_rx_dma:
	free_dma(rx_dma);
err_free_tx_dma:
	free_dma(tx_dma);
err_put_master:
	spi_master_put(master);

	return ret;
}

static int adi_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master = platform_get_drvdata(pdev);
	struct adi_spi_master *drv_data = spi_master_get_devdata(master);

	adi_spi_disable(drv_data);
	peripheral_free_list(drv_data->pin_req);
	free_dma(drv_data->rx_dma);
	free_dma(drv_data->tx_dma);
	return 0;
}

#ifdef CONFIG_PM
static int adi_spi_suspend(struct device *dev)
{
	struct spi_master *master = dev_get_drvdata(dev);
	struct adi_spi_master *drv_data = spi_master_get_devdata(master);

	spi_master_suspend(master);

	drv_data->control = ioread32(&drv_data->regs->control);
	drv_data->ssel = ioread32(&drv_data->regs->ssel);

	iowrite32(SPI_CTL_MSTR | SPI_CTL_CPHA, &drv_data->regs->control);
	iowrite32(0x0000FE00, &drv_data->regs->ssel);
	dma_disable_irq(drv_data->rx_dma);
	dma_disable_irq(drv_data->tx_dma);

	return 0;
}

static int adi_spi_resume(struct device *dev)
{
	struct spi_master *master = dev_get_drvdata(dev);
	struct adi_spi_master *drv_data = spi_master_get_devdata(master);
	int ret = 0;

	/* bootrom may modify spi and dma status when resume in spi boot mode */
	disable_dma(drv_data->rx_dma);

	dma_enable_irq(drv_data->rx_dma);
	dma_enable_irq(drv_data->tx_dma);
	iowrite32(drv_data->control, &drv_data->regs->control);
	iowrite32(drv_data->ssel, &drv_data->regs->ssel);

	ret = spi_master_resume(master);
	if (ret) {
		free_dma(drv_data->rx_dma);
		free_dma(drv_data->tx_dma);
	}

	return ret;
}
#endif
static const struct dev_pm_ops adi_spi_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(adi_spi_suspend, adi_spi_resume)
};

MODULE_ALIAS("platform:adi-spi3");
static struct platform_driver adi_spi_driver = {
	.driver	= {
		.name	= "adi-spi3",
		.pm     = &adi_spi_pm_ops,
	},
	.remove		= adi_spi_remove,
};

module_platform_driver_probe(adi_spi_driver, adi_spi_probe);

MODULE_DESCRIPTION("Analog Devices SPI3 controller driver");
MODULE_AUTHOR("Scott Jiang <Scott.Jiang.Linux@gmail.com>");
MODULE_LICENSE("GPL v2");
