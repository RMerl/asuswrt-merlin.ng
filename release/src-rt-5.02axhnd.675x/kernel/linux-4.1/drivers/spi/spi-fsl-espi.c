/*
 * Freescale eSPI controller driver.
 *
 * Copyright 2010 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/fsl_devices.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <sysdev/fsl_soc.h>

#include "spi-fsl-lib.h"

/* eSPI Controller registers */
struct fsl_espi_reg {
	__be32 mode;		/* 0x000 - eSPI mode register */
	__be32 event;		/* 0x004 - eSPI event register */
	__be32 mask;		/* 0x008 - eSPI mask register */
	__be32 command;		/* 0x00c - eSPI command register */
	__be32 transmit;	/* 0x010 - eSPI transmit FIFO access register*/
	__be32 receive;		/* 0x014 - eSPI receive FIFO access register*/
	u8 res[8];		/* 0x018 - 0x01c reserved */
	__be32 csmode[4];	/* 0x020 - 0x02c eSPI cs mode register */
};

struct fsl_espi_transfer {
	const void *tx_buf;
	void *rx_buf;
	unsigned len;
	unsigned n_tx;
	unsigned n_rx;
	unsigned actual_length;
	int status;
};

/* eSPI Controller mode register definitions */
#define SPMODE_ENABLE		(1 << 31)
#define SPMODE_LOOP		(1 << 30)
#define SPMODE_TXTHR(x)		((x) << 8)
#define SPMODE_RXTHR(x)		((x) << 0)

/* eSPI Controller CS mode register definitions */
#define CSMODE_CI_INACTIVEHIGH	(1 << 31)
#define CSMODE_CP_BEGIN_EDGECLK	(1 << 30)
#define CSMODE_REV		(1 << 29)
#define CSMODE_DIV16		(1 << 28)
#define CSMODE_PM(x)		((x) << 24)
#define CSMODE_POL_1		(1 << 20)
#define CSMODE_LEN(x)		((x) << 16)
#define CSMODE_BEF(x)		((x) << 12)
#define CSMODE_AFT(x)		((x) << 8)
#define CSMODE_CG(x)		((x) << 3)

/* Default mode/csmode for eSPI controller */
#define SPMODE_INIT_VAL (SPMODE_TXTHR(4) | SPMODE_RXTHR(3))
#define CSMODE_INIT_VAL (CSMODE_POL_1 | CSMODE_BEF(0) \
		| CSMODE_AFT(0) | CSMODE_CG(1))

/* SPIE register values */
#define	SPIE_NE		0x00000200	/* Not empty */
#define	SPIE_NF		0x00000100	/* Not full */

/* SPIM register values */
#define	SPIM_NE		0x00000200	/* Not empty */
#define	SPIM_NF		0x00000100	/* Not full */
#define SPIE_RXCNT(reg)     ((reg >> 24) & 0x3F)
#define SPIE_TXCNT(reg)     ((reg >> 16) & 0x3F)

/* SPCOM register values */
#define SPCOM_CS(x)		((x) << 30)
#define SPCOM_TRANLEN(x)	((x) << 0)
#define	SPCOM_TRANLEN_MAX	0xFFFF	/* Max transaction length */

static void fsl_espi_change_mode(struct spi_device *spi)
{
	struct mpc8xxx_spi *mspi = spi_master_get_devdata(spi->master);
	struct spi_mpc8xxx_cs *cs = spi->controller_state;
	struct fsl_espi_reg *reg_base = mspi->reg_base;
	__be32 __iomem *mode = &reg_base->csmode[spi->chip_select];
	__be32 __iomem *espi_mode = &reg_base->mode;
	u32 tmp;
	unsigned long flags;

	/* Turn off IRQs locally to minimize time that SPI is disabled. */
	local_irq_save(flags);

	/* Turn off SPI unit prior changing mode */
	tmp = mpc8xxx_spi_read_reg(espi_mode);
	mpc8xxx_spi_write_reg(espi_mode, tmp & ~SPMODE_ENABLE);
	mpc8xxx_spi_write_reg(mode, cs->hw_mode);
	mpc8xxx_spi_write_reg(espi_mode, tmp);

	local_irq_restore(flags);
}

static u32 fsl_espi_tx_buf_lsb(struct mpc8xxx_spi *mpc8xxx_spi)
{
	u32 data;
	u16 data_h;
	u16 data_l;
	const u32 *tx = mpc8xxx_spi->tx;

	if (!tx)
		return 0;

	data = *tx++ << mpc8xxx_spi->tx_shift;
	data_l = data & 0xffff;
	data_h = (data >> 16) & 0xffff;
	swab16s(&data_l);
	swab16s(&data_h);
	data = data_h | data_l;

	mpc8xxx_spi->tx = tx;
	return data;
}

static int fsl_espi_setup_transfer(struct spi_device *spi,
					struct spi_transfer *t)
{
	struct mpc8xxx_spi *mpc8xxx_spi = spi_master_get_devdata(spi->master);
	int bits_per_word = 0;
	u8 pm;
	u32 hz = 0;
	struct spi_mpc8xxx_cs *cs = spi->controller_state;

	if (t) {
		bits_per_word = t->bits_per_word;
		hz = t->speed_hz;
	}

	/* spi_transfer level calls that work per-word */
	if (!bits_per_word)
		bits_per_word = spi->bits_per_word;

	if (!hz)
		hz = spi->max_speed_hz;

	cs->rx_shift = 0;
	cs->tx_shift = 0;
	cs->get_rx = mpc8xxx_spi_rx_buf_u32;
	cs->get_tx = mpc8xxx_spi_tx_buf_u32;
	if (bits_per_word <= 8) {
		cs->rx_shift = 8 - bits_per_word;
	} else {
		cs->rx_shift = 16 - bits_per_word;
		if (spi->mode & SPI_LSB_FIRST)
			cs->get_tx = fsl_espi_tx_buf_lsb;
	}

	mpc8xxx_spi->rx_shift = cs->rx_shift;
	mpc8xxx_spi->tx_shift = cs->tx_shift;
	mpc8xxx_spi->get_rx = cs->get_rx;
	mpc8xxx_spi->get_tx = cs->get_tx;

	bits_per_word = bits_per_word - 1;

	/* mask out bits we are going to set */
	cs->hw_mode &= ~(CSMODE_LEN(0xF) | CSMODE_DIV16 | CSMODE_PM(0xF));

	cs->hw_mode |= CSMODE_LEN(bits_per_word);

	if ((mpc8xxx_spi->spibrg / hz) > 64) {
		cs->hw_mode |= CSMODE_DIV16;
		pm = DIV_ROUND_UP(mpc8xxx_spi->spibrg, hz * 16 * 4);

		WARN_ONCE(pm > 33, "%s: Requested speed is too low: %d Hz. "
			  "Will use %d Hz instead.\n", dev_name(&spi->dev),
				hz, mpc8xxx_spi->spibrg / (4 * 16 * (32 + 1)));
		if (pm > 33)
			pm = 33;
	} else {
		pm = DIV_ROUND_UP(mpc8xxx_spi->spibrg, hz * 4);
	}
	if (pm)
		pm--;
	if (pm < 2)
		pm = 2;

	cs->hw_mode |= CSMODE_PM(pm);

	fsl_espi_change_mode(spi);
	return 0;
}

static int fsl_espi_cpu_bufs(struct mpc8xxx_spi *mspi, struct spi_transfer *t,
		unsigned int len)
{
	u32 word;
	struct fsl_espi_reg *reg_base = mspi->reg_base;

	mspi->count = len;

	/* enable rx ints */
	mpc8xxx_spi_write_reg(&reg_base->mask, SPIM_NE);

	/* transmit word */
	word = mspi->get_tx(mspi);
	mpc8xxx_spi_write_reg(&reg_base->transmit, word);

	return 0;
}

static int fsl_espi_bufs(struct spi_device *spi, struct spi_transfer *t)
{
	struct mpc8xxx_spi *mpc8xxx_spi = spi_master_get_devdata(spi->master);
	struct fsl_espi_reg *reg_base = mpc8xxx_spi->reg_base;
	unsigned int len = t->len;
	int ret;

	mpc8xxx_spi->len = t->len;
	len = roundup(len, 4) / 4;

	mpc8xxx_spi->tx = t->tx_buf;
	mpc8xxx_spi->rx = t->rx_buf;

	reinit_completion(&mpc8xxx_spi->done);

	/* Set SPCOM[CS] and SPCOM[TRANLEN] field */
	if ((t->len - 1) > SPCOM_TRANLEN_MAX) {
		dev_err(mpc8xxx_spi->dev, "Transaction length (%d)"
				" beyond the SPCOM[TRANLEN] field\n", t->len);
		return -EINVAL;
	}
	mpc8xxx_spi_write_reg(&reg_base->command,
		(SPCOM_CS(spi->chip_select) | SPCOM_TRANLEN(t->len - 1)));

	ret = fsl_espi_cpu_bufs(mpc8xxx_spi, t, len);
	if (ret)
		return ret;

	wait_for_completion(&mpc8xxx_spi->done);

	/* disable rx ints */
	mpc8xxx_spi_write_reg(&reg_base->mask, 0);

	return mpc8xxx_spi->count;
}

static inline void fsl_espi_addr2cmd(unsigned int addr, u8 *cmd)
{
	if (cmd) {
		cmd[1] = (u8)(addr >> 16);
		cmd[2] = (u8)(addr >> 8);
		cmd[3] = (u8)(addr >> 0);
	}
}

static inline unsigned int fsl_espi_cmd2addr(u8 *cmd)
{
	if (cmd)
		return cmd[1] << 16 | cmd[2] << 8 | cmd[3] << 0;

	return 0;
}

static void fsl_espi_do_trans(struct spi_message *m,
				struct fsl_espi_transfer *tr)
{
	struct spi_device *spi = m->spi;
	struct mpc8xxx_spi *mspi = spi_master_get_devdata(spi->master);
	struct fsl_espi_transfer *espi_trans = tr;
	struct spi_message message;
	struct spi_transfer *t, *first, trans;
	int status = 0;

	spi_message_init(&message);
	memset(&trans, 0, sizeof(trans));

	first = list_first_entry(&m->transfers, struct spi_transfer,
			transfer_list);
	list_for_each_entry(t, &m->transfers, transfer_list) {
		if ((first->bits_per_word != t->bits_per_word) ||
			(first->speed_hz != t->speed_hz)) {
			espi_trans->status = -EINVAL;
			dev_err(mspi->dev,
				"bits_per_word/speed_hz should be same for the same SPI transfer\n");
			return;
		}

		trans.speed_hz = t->speed_hz;
		trans.bits_per_word = t->bits_per_word;
		trans.delay_usecs = max(first->delay_usecs, t->delay_usecs);
	}

	trans.len = espi_trans->len;
	trans.tx_buf = espi_trans->tx_buf;
	trans.rx_buf = espi_trans->rx_buf;
	spi_message_add_tail(&trans, &message);

	list_for_each_entry(t, &message.transfers, transfer_list) {
		if (t->bits_per_word || t->speed_hz) {
			status = -EINVAL;

			status = fsl_espi_setup_transfer(spi, t);
			if (status < 0)
				break;
		}

		if (t->len)
			status = fsl_espi_bufs(spi, t);

		if (status) {
			status = -EMSGSIZE;
			break;
		}

		if (t->delay_usecs)
			udelay(t->delay_usecs);
	}

	espi_trans->status = status;
	fsl_espi_setup_transfer(spi, NULL);
}

static void fsl_espi_cmd_trans(struct spi_message *m,
				struct fsl_espi_transfer *trans, u8 *rx_buff)
{
	struct spi_transfer *t;
	u8 *local_buf;
	int i = 0;
	struct fsl_espi_transfer *espi_trans = trans;

	local_buf = kzalloc(SPCOM_TRANLEN_MAX, GFP_KERNEL);
	if (!local_buf) {
		espi_trans->status = -ENOMEM;
		return;
	}

	list_for_each_entry(t, &m->transfers, transfer_list) {
		if (t->tx_buf) {
			memcpy(local_buf + i, t->tx_buf, t->len);
			i += t->len;
		}
	}

	espi_trans->tx_buf = local_buf;
	espi_trans->rx_buf = local_buf;
	fsl_espi_do_trans(m, espi_trans);

	espi_trans->actual_length = espi_trans->len;
	kfree(local_buf);
}

static void fsl_espi_rw_trans(struct spi_message *m,
				struct fsl_espi_transfer *trans, u8 *rx_buff)
{
	struct fsl_espi_transfer *espi_trans = trans;
	unsigned int total_len = espi_trans->len;
	struct spi_transfer *t;
	u8 *local_buf;
	u8 *rx_buf = rx_buff;
	unsigned int trans_len;
	unsigned int addr;
	unsigned int tx_only;
	unsigned int rx_pos = 0;
	unsigned int pos;
	int i, loop;

	local_buf = kzalloc(SPCOM_TRANLEN_MAX, GFP_KERNEL);
	if (!local_buf) {
		espi_trans->status = -ENOMEM;
		return;
	}

	for (pos = 0, loop = 0; pos < total_len; pos += trans_len, loop++) {
		trans_len = total_len - pos;

		i = 0;
		tx_only = 0;
		list_for_each_entry(t, &m->transfers, transfer_list) {
			if (t->tx_buf) {
				memcpy(local_buf + i, t->tx_buf, t->len);
				i += t->len;
				if (!t->rx_buf)
					tx_only += t->len;
			}
		}

		/* Add additional TX bytes to compensate SPCOM_TRANLEN_MAX */
		if (loop > 0)
			trans_len += tx_only;

		if (trans_len > SPCOM_TRANLEN_MAX)
			trans_len = SPCOM_TRANLEN_MAX;

		/* Update device offset */
		if (pos > 0) {
			addr = fsl_espi_cmd2addr(local_buf);
			addr += rx_pos;
			fsl_espi_addr2cmd(addr, local_buf);
		}

		espi_trans->len = trans_len;
		espi_trans->tx_buf = local_buf;
		espi_trans->rx_buf = local_buf;
		fsl_espi_do_trans(m, espi_trans);

		/* If there is at least one RX byte then copy it to rx_buf */
		if (tx_only < SPCOM_TRANLEN_MAX)
			memcpy(rx_buf + rx_pos, espi_trans->rx_buf + tx_only,
					trans_len - tx_only);

		rx_pos += trans_len - tx_only;

		if (loop > 0)
			espi_trans->actual_length += espi_trans->len - tx_only;
		else
			espi_trans->actual_length += espi_trans->len;
	}

	kfree(local_buf);
}

static int fsl_espi_do_one_msg(struct spi_master *master,
			       struct spi_message *m)
{
	struct spi_transfer *t;
	u8 *rx_buf = NULL;
	unsigned int n_tx = 0;
	unsigned int n_rx = 0;
	unsigned int xfer_len = 0;
	struct fsl_espi_transfer espi_trans;

	list_for_each_entry(t, &m->transfers, transfer_list) {
		if (t->tx_buf)
			n_tx += t->len;
		if (t->rx_buf) {
			n_rx += t->len;
			rx_buf = t->rx_buf;
		}
		if ((t->tx_buf) || (t->rx_buf))
			xfer_len += t->len;
	}

	espi_trans.n_tx = n_tx;
	espi_trans.n_rx = n_rx;
	espi_trans.len = xfer_len;
	espi_trans.actual_length = 0;
	espi_trans.status = 0;

	if (!rx_buf)
		fsl_espi_cmd_trans(m, &espi_trans, NULL);
	else
		fsl_espi_rw_trans(m, &espi_trans, rx_buf);

	m->actual_length = espi_trans.actual_length;
	m->status = espi_trans.status;
	spi_finalize_current_message(master);
	return 0;
}

static int fsl_espi_setup(struct spi_device *spi)
{
	struct mpc8xxx_spi *mpc8xxx_spi;
	struct fsl_espi_reg *reg_base;
	int retval;
	u32 hw_mode;
	u32 loop_mode;
	struct spi_mpc8xxx_cs *cs = spi_get_ctldata(spi);

	if (!spi->max_speed_hz)
		return -EINVAL;

	if (!cs) {
		cs = kzalloc(sizeof(*cs), GFP_KERNEL);
		if (!cs)
			return -ENOMEM;
		spi_set_ctldata(spi, cs);
	}

	mpc8xxx_spi = spi_master_get_devdata(spi->master);
	reg_base = mpc8xxx_spi->reg_base;

	hw_mode = cs->hw_mode; /* Save original settings */
	cs->hw_mode = mpc8xxx_spi_read_reg(
			&reg_base->csmode[spi->chip_select]);
	/* mask out bits we are going to set */
	cs->hw_mode &= ~(CSMODE_CP_BEGIN_EDGECLK | CSMODE_CI_INACTIVEHIGH
			 | CSMODE_REV);

	if (spi->mode & SPI_CPHA)
		cs->hw_mode |= CSMODE_CP_BEGIN_EDGECLK;
	if (spi->mode & SPI_CPOL)
		cs->hw_mode |= CSMODE_CI_INACTIVEHIGH;
	if (!(spi->mode & SPI_LSB_FIRST))
		cs->hw_mode |= CSMODE_REV;

	/* Handle the loop mode */
	loop_mode = mpc8xxx_spi_read_reg(&reg_base->mode);
	loop_mode &= ~SPMODE_LOOP;
	if (spi->mode & SPI_LOOP)
		loop_mode |= SPMODE_LOOP;
	mpc8xxx_spi_write_reg(&reg_base->mode, loop_mode);

	retval = fsl_espi_setup_transfer(spi, NULL);
	if (retval < 0) {
		cs->hw_mode = hw_mode; /* Restore settings */
		return retval;
	}
	return 0;
}

static void fsl_espi_cleanup(struct spi_device *spi)
{
	struct spi_mpc8xxx_cs *cs = spi_get_ctldata(spi);

	kfree(cs);
	spi_set_ctldata(spi, NULL);
}

void fsl_espi_cpu_irq(struct mpc8xxx_spi *mspi, u32 events)
{
	struct fsl_espi_reg *reg_base = mspi->reg_base;

	/* We need handle RX first */
	if (events & SPIE_NE) {
		u32 rx_data, tmp;
		u8 rx_data_8;

		/* Spin until RX is done */
		while (SPIE_RXCNT(events) < min(4, mspi->len)) {
			cpu_relax();
			events = mpc8xxx_spi_read_reg(&reg_base->event);
		}

		if (mspi->len >= 4) {
			rx_data = mpc8xxx_spi_read_reg(&reg_base->receive);
		} else {
			tmp = mspi->len;
			rx_data = 0;
			while (tmp--) {
				rx_data_8 = in_8((u8 *)&reg_base->receive);
				rx_data |= (rx_data_8 << (tmp * 8));
			}

			rx_data <<= (4 - mspi->len) * 8;
		}

		mspi->len -= 4;

		if (mspi->rx)
			mspi->get_rx(rx_data, mspi);
	}

	if (!(events & SPIE_NF)) {
		int ret;

		/* spin until TX is done */
		ret = spin_event_timeout(((events = mpc8xxx_spi_read_reg(
				&reg_base->event)) & SPIE_NF) == 0, 1000, 0);
		if (!ret) {
			dev_err(mspi->dev, "tired waiting for SPIE_NF\n");
			return;
		}
	}

	/* Clear the events */
	mpc8xxx_spi_write_reg(&reg_base->event, events);

	mspi->count -= 1;
	if (mspi->count) {
		u32 word = mspi->get_tx(mspi);

		mpc8xxx_spi_write_reg(&reg_base->transmit, word);
	} else {
		complete(&mspi->done);
	}
}

static irqreturn_t fsl_espi_irq(s32 irq, void *context_data)
{
	struct mpc8xxx_spi *mspi = context_data;
	struct fsl_espi_reg *reg_base = mspi->reg_base;
	irqreturn_t ret = IRQ_NONE;
	u32 events;

	/* Get interrupt events(tx/rx) */
	events = mpc8xxx_spi_read_reg(&reg_base->event);
	if (events)
		ret = IRQ_HANDLED;

	dev_vdbg(mspi->dev, "%s: events %x\n", __func__, events);

	fsl_espi_cpu_irq(mspi, events);

	return ret;
}

static void fsl_espi_remove(struct mpc8xxx_spi *mspi)
{
	iounmap(mspi->reg_base);
}

static int fsl_espi_suspend(struct spi_master *master)
{
	struct mpc8xxx_spi *mpc8xxx_spi;
	struct fsl_espi_reg *reg_base;
	u32 regval;

	mpc8xxx_spi = spi_master_get_devdata(master);
	reg_base = mpc8xxx_spi->reg_base;

	regval = mpc8xxx_spi_read_reg(&reg_base->mode);
	regval &= ~SPMODE_ENABLE;
	mpc8xxx_spi_write_reg(&reg_base->mode, regval);

	return 0;
}

static int fsl_espi_resume(struct spi_master *master)
{
	struct mpc8xxx_spi *mpc8xxx_spi;
	struct fsl_espi_reg *reg_base;
	u32 regval;

	mpc8xxx_spi = spi_master_get_devdata(master);
	reg_base = mpc8xxx_spi->reg_base;

	regval = mpc8xxx_spi_read_reg(&reg_base->mode);
	regval |= SPMODE_ENABLE;
	mpc8xxx_spi_write_reg(&reg_base->mode, regval);

	return 0;
}

static struct spi_master * fsl_espi_probe(struct device *dev,
		struct resource *mem, unsigned int irq)
{
	struct fsl_spi_platform_data *pdata = dev_get_platdata(dev);
	struct spi_master *master;
	struct mpc8xxx_spi *mpc8xxx_spi;
	struct fsl_espi_reg *reg_base;
	struct device_node *nc;
	const __be32 *prop;
	u32 regval, csmode;
	int i, len, ret = 0;

	master = spi_alloc_master(dev, sizeof(struct mpc8xxx_spi));
	if (!master) {
		ret = -ENOMEM;
		goto err;
	}

	dev_set_drvdata(dev, master);

	mpc8xxx_spi_probe(dev, mem, irq);

	master->bits_per_word_mask = SPI_BPW_RANGE_MASK(4, 16);
	master->setup = fsl_espi_setup;
	master->cleanup = fsl_espi_cleanup;
	master->transfer_one_message = fsl_espi_do_one_msg;
	master->prepare_transfer_hardware = fsl_espi_resume;
	master->unprepare_transfer_hardware = fsl_espi_suspend;

	mpc8xxx_spi = spi_master_get_devdata(master);
	mpc8xxx_spi->spi_remove = fsl_espi_remove;

	mpc8xxx_spi->reg_base = ioremap(mem->start, resource_size(mem));
	if (!mpc8xxx_spi->reg_base) {
		ret = -ENOMEM;
		goto err_probe;
	}

	reg_base = mpc8xxx_spi->reg_base;

	/* Register for SPI Interrupt */
	ret = request_irq(mpc8xxx_spi->irq, fsl_espi_irq,
			  0, "fsl_espi", mpc8xxx_spi);
	if (ret)
		goto free_irq;

	if (mpc8xxx_spi->flags & SPI_QE_CPU_MODE) {
		mpc8xxx_spi->rx_shift = 16;
		mpc8xxx_spi->tx_shift = 24;
	}

	/* SPI controller initializations */
	mpc8xxx_spi_write_reg(&reg_base->mode, 0);
	mpc8xxx_spi_write_reg(&reg_base->mask, 0);
	mpc8xxx_spi_write_reg(&reg_base->command, 0);
	mpc8xxx_spi_write_reg(&reg_base->event, 0xffffffff);

	/* Init eSPI CS mode register */
	for_each_available_child_of_node(master->dev.of_node, nc) {
		/* get chip select */
		prop = of_get_property(nc, "reg", &len);
		if (!prop || len < sizeof(*prop))
			continue;
		i = be32_to_cpup(prop);
		if (i < 0 || i >= pdata->max_chipselect)
			continue;

		csmode = CSMODE_INIT_VAL;
		/* check if CSBEF is set in device tree */
		prop = of_get_property(nc, "fsl,csbef", &len);
		if (prop && len >= sizeof(*prop)) {
			csmode &= ~(CSMODE_BEF(0xf));
			csmode |= CSMODE_BEF(be32_to_cpup(prop));
		}
		/* check if CSAFT is set in device tree */
		prop = of_get_property(nc, "fsl,csaft", &len);
		if (prop && len >= sizeof(*prop)) {
			csmode &= ~(CSMODE_AFT(0xf));
			csmode |= CSMODE_AFT(be32_to_cpup(prop));
		}
		mpc8xxx_spi_write_reg(&reg_base->csmode[i], csmode);

		dev_info(dev, "cs=%d, init_csmode=0x%x\n", i, csmode);
	}

	/* Enable SPI interface */
	regval = pdata->initial_spmode | SPMODE_INIT_VAL | SPMODE_ENABLE;

	mpc8xxx_spi_write_reg(&reg_base->mode, regval);

	ret = spi_register_master(master);
	if (ret < 0)
		goto unreg_master;

	dev_info(dev, "at 0x%p (irq = %d)\n", reg_base, mpc8xxx_spi->irq);

	return master;

unreg_master:
	free_irq(mpc8xxx_spi->irq, mpc8xxx_spi);
free_irq:
	iounmap(mpc8xxx_spi->reg_base);
err_probe:
	spi_master_put(master);
err:
	return ERR_PTR(ret);
}

static int of_fsl_espi_get_chipselects(struct device *dev)
{
	struct device_node *np = dev->of_node;
	struct fsl_spi_platform_data *pdata = dev_get_platdata(dev);
	const u32 *prop;
	int len;

	prop = of_get_property(np, "fsl,espi-num-chipselects", &len);
	if (!prop || len < sizeof(*prop)) {
		dev_err(dev, "No 'fsl,espi-num-chipselects' property\n");
		return -EINVAL;
	}

	pdata->max_chipselect = *prop;
	pdata->cs_control = NULL;

	return 0;
}

static int of_fsl_espi_probe(struct platform_device *ofdev)
{
	struct device *dev = &ofdev->dev;
	struct device_node *np = ofdev->dev.of_node;
	struct spi_master *master;
	struct resource mem;
	unsigned int irq;
	int ret = -ENOMEM;

	ret = of_mpc8xxx_spi_probe(ofdev);
	if (ret)
		return ret;

	ret = of_fsl_espi_get_chipselects(dev);
	if (ret)
		goto err;

	ret = of_address_to_resource(np, 0, &mem);
	if (ret)
		goto err;

	irq = irq_of_parse_and_map(np, 0);
	if (!irq) {
		ret = -EINVAL;
		goto err;
	}

	master = fsl_espi_probe(dev, &mem, irq);
	if (IS_ERR(master)) {
		ret = PTR_ERR(master);
		goto err;
	}

	return 0;

err:
	return ret;
}

static int of_fsl_espi_remove(struct platform_device *dev)
{
	return mpc8xxx_spi_remove(&dev->dev);
}

#ifdef CONFIG_PM_SLEEP
static int of_fsl_espi_suspend(struct device *dev)
{
	struct spi_master *master = dev_get_drvdata(dev);
	int ret;

	ret = spi_master_suspend(master);
	if (ret) {
		dev_warn(dev, "cannot suspend master\n");
		return ret;
	}

	return fsl_espi_suspend(master);
}

static int of_fsl_espi_resume(struct device *dev)
{
	struct fsl_spi_platform_data *pdata = dev_get_platdata(dev);
	struct spi_master *master = dev_get_drvdata(dev);
	struct mpc8xxx_spi *mpc8xxx_spi;
	struct fsl_espi_reg *reg_base;
	u32 regval;
	int i;

	mpc8xxx_spi = spi_master_get_devdata(master);
	reg_base = mpc8xxx_spi->reg_base;

	/* SPI controller initializations */
	mpc8xxx_spi_write_reg(&reg_base->mode, 0);
	mpc8xxx_spi_write_reg(&reg_base->mask, 0);
	mpc8xxx_spi_write_reg(&reg_base->command, 0);
	mpc8xxx_spi_write_reg(&reg_base->event, 0xffffffff);

	/* Init eSPI CS mode register */
	for (i = 0; i < pdata->max_chipselect; i++)
		mpc8xxx_spi_write_reg(&reg_base->csmode[i], CSMODE_INIT_VAL);

	/* Enable SPI interface */
	regval = pdata->initial_spmode | SPMODE_INIT_VAL | SPMODE_ENABLE;

	mpc8xxx_spi_write_reg(&reg_base->mode, regval);

	return spi_master_resume(master);
}
#endif /* CONFIG_PM_SLEEP */

static const struct dev_pm_ops espi_pm = {
	SET_SYSTEM_SLEEP_PM_OPS(of_fsl_espi_suspend, of_fsl_espi_resume)
};

static const struct of_device_id of_fsl_espi_match[] = {
	{ .compatible = "fsl,mpc8536-espi" },
	{}
};
MODULE_DEVICE_TABLE(of, of_fsl_espi_match);

static struct platform_driver fsl_espi_driver = {
	.driver = {
		.name = "fsl_espi",
		.of_match_table = of_fsl_espi_match,
		.pm = &espi_pm,
	},
	.probe		= of_fsl_espi_probe,
	.remove		= of_fsl_espi_remove,
};
module_platform_driver(fsl_espi_driver);

MODULE_AUTHOR("Mingkai Hu");
MODULE_DESCRIPTION("Enhanced Freescale SPI Driver");
MODULE_LICENSE("GPL");
