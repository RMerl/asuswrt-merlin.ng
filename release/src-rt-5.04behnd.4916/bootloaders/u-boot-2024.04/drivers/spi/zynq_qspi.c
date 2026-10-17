// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013 Xilinx, Inc.
 * (C) Copyright 2015 Jagan Teki <jteki@openedev.com>
 *
 * Xilinx Zynq Quad-SPI(QSPI) controller driver (master mode only)
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <spi-mem.h>

DECLARE_GLOBAL_DATA_PTR;

/* zynq qspi register bit masks ZYNQ_QSPI_<REG>_<BIT>_MASK */
#define ZYNQ_QSPI_CR_IFMODE_MASK	BIT(31)	/* Flash intrface mode*/
#define ZYNQ_QSPI_CR_MSA_MASK		BIT(15)	/* Manual start enb */
#define ZYNQ_QSPI_CR_MCS_MASK		BIT(14)	/* Manual chip select */
#define ZYNQ_QSPI_CR_PCS_MASK		BIT(10)	/* Peri chip select */
#define ZYNQ_QSPI_CR_FW_MASK		GENMASK(7, 6)	/* FIFO width */
#define ZYNQ_QSPI_CR_SS_MASK		GENMASK(13, 10)	/* Slave Select */
#define ZYNQ_QSPI_CR_BAUD_MASK		GENMASK(5, 3)	/* Baud rate div */
#define ZYNQ_QSPI_CR_CPHA_MASK		BIT(2)	/* Clock phase */
#define ZYNQ_QSPI_CR_CPOL_MASK		BIT(1)	/* Clock polarity */
#define ZYNQ_QSPI_CR_MSTREN_MASK	BIT(0)	/* Mode select */
#define ZYNQ_QSPI_IXR_RXNEMPTY_MASK	BIT(4)	/* RX_FIFO_not_empty */
#define ZYNQ_QSPI_IXR_TXOW_MASK		BIT(2)	/* TX_FIFO_not_full */
#define ZYNQ_QSPI_IXR_ALL_MASK		GENMASK(6, 0)	/* All IXR bits */
#define ZYNQ_QSPI_ENR_SPI_EN_MASK	BIT(0)	/* SPI Enable */
#define ZYNQ_QSPI_LQSPICFG_LQMODE_MASK	BIT(31) /* Linear QSPI Mode */

/* zynq qspi Transmit Data Register */
#define ZYNQ_QSPI_TXD_00_00_OFFSET	0x1C	/* Transmit 4-byte inst */
#define ZYNQ_QSPI_TXD_00_01_OFFSET	0x80	/* Transmit 1-byte inst */
#define ZYNQ_QSPI_TXD_00_10_OFFSET	0x84	/* Transmit 2-byte inst */
#define ZYNQ_QSPI_TXD_00_11_OFFSET	0x88	/* Transmit 3-byte inst */

#define ZYNQ_QSPI_TXFIFO_THRESHOLD	1	/* Tx FIFO threshold level*/
#define ZYNQ_QSPI_RXFIFO_THRESHOLD	32	/* Rx FIFO threshold level */

#define ZYNQ_QSPI_CR_BAUD_MAX		8	/* Baud rate divisor max val */
#define ZYNQ_QSPI_CR_BAUD_SHIFT		3	/* Baud rate divisor shift */
#define ZYNQ_QSPI_CR_SS_SHIFT		10	/* Slave select shift */

#define ZYNQ_QSPI_MAX_BAUD_RATE		0x7
#define ZYNQ_QSPI_DEFAULT_BAUD_RATE	0x2

#define ZYNQ_QSPI_FIFO_DEPTH		63
#define ZYNQ_QSPI_WAIT			(CONFIG_SYS_HZ / 100)	/* 10 ms */

/* zynq qspi register set */
struct zynq_qspi_regs {
	u32 cr;		/* 0x00 */
	u32 isr;	/* 0x04 */
	u32 ier;	/* 0x08 */
	u32 idr;	/* 0x0C */
	u32 imr;	/* 0x10 */
	u32 enr;	/* 0x14 */
	u32 dr;		/* 0x18 */
	u32 txd0r;	/* 0x1C */
	u32 drxr;	/* 0x20 */
	u32 sicr;	/* 0x24 */
	u32 txftr;	/* 0x28 */
	u32 rxftr;	/* 0x2C */
	u32 gpior;	/* 0x30 */
	u32 reserved0[19];
	u32 txd1r;	/* 0x80 */
	u32 txd2r;	/* 0x84 */
	u32 txd3r;	/* 0x88 */
	u32 reserved1[5];
	u32 lqspicfg;	/* 0xA0 */
	u32 lqspists;	/* 0xA4 */
};

/* zynq qspi platform data */
struct zynq_qspi_plat {
	struct zynq_qspi_regs *regs;
	u32 frequency;          /* input frequency */
	u32 speed_hz;
};

/* zynq qspi priv */
struct zynq_qspi_priv {
	struct zynq_qspi_regs *regs;
	u8 cs;
	u8 mode;
	u8 fifo_depth;
	u32 freq;		/* required frequency */
	u32 max_hz;
	const void *tx_buf;
	void *rx_buf;
	unsigned len;
	int bytes_to_transfer;
	int bytes_to_receive;
	unsigned int is_inst;
	unsigned cs_change:1;
};

static int zynq_qspi_of_to_plat(struct udevice *bus)
{
	struct zynq_qspi_plat *plat = dev_get_plat(bus);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(bus);

	plat->regs = (struct zynq_qspi_regs *)fdtdec_get_addr(blob,
							      node, "reg");

	return 0;
}

/**
 * zynq_qspi_init_hw - Initialize the hardware
 * @priv:	Pointer to the zynq_qspi_priv structure
 *
 * The default settings of the QSPI controller's configurable parameters on
 * reset are
 *	- Master mode
 *	- Baud rate divisor is set to 2
 *	- Threshold value for TX FIFO not full interrupt is set to 1
 *	- Flash memory interface mode enabled
 *	- Size of the word to be transferred as 8 bit
 * This function performs the following actions
 *	- Disable and clear all the interrupts
 *	- Enable manual slave select
 *	- Enable auto start
 *	- Deselect all the chip select lines
 *	- Set the size of the word to be transferred as 32 bit
 *	- Set the little endian mode of TX FIFO and
 *	- Enable the QSPI controller
 */
static void zynq_qspi_init_hw(struct zynq_qspi_priv *priv)
{
	struct zynq_qspi_regs *regs = priv->regs;
	u32 confr;

	/* Disable QSPI */
	writel(~ZYNQ_QSPI_ENR_SPI_EN_MASK, &regs->enr);

	/* Disable Interrupts */
	writel(ZYNQ_QSPI_IXR_ALL_MASK, &regs->idr);

	/* Clear the TX and RX threshold reg */
	writel(ZYNQ_QSPI_TXFIFO_THRESHOLD, &regs->txftr);
	writel(ZYNQ_QSPI_RXFIFO_THRESHOLD, &regs->rxftr);

	/* Clear the RX FIFO */
	while (readl(&regs->isr) & ZYNQ_QSPI_IXR_RXNEMPTY_MASK)
		readl(&regs->drxr);

	/* Clear Interrupts */
	writel(ZYNQ_QSPI_IXR_ALL_MASK, &regs->isr);

	/* Manual slave select and Auto start */
	confr = readl(&regs->cr);
	confr &= ~ZYNQ_QSPI_CR_MSA_MASK;
	confr |= ZYNQ_QSPI_CR_IFMODE_MASK | ZYNQ_QSPI_CR_MCS_MASK |
		ZYNQ_QSPI_CR_PCS_MASK | ZYNQ_QSPI_CR_FW_MASK |
		ZYNQ_QSPI_CR_MSTREN_MASK;
	writel(confr, &regs->cr);

	/* Disable the LQSPI feature */
	confr = readl(&regs->lqspicfg);
	confr &= ~ZYNQ_QSPI_LQSPICFG_LQMODE_MASK;
	writel(confr, &regs->lqspicfg);

	/* Enable SPI */
	writel(ZYNQ_QSPI_ENR_SPI_EN_MASK, &regs->enr);
}

static int zynq_qspi_child_pre_probe(struct udevice *bus)
{
	struct spi_slave *slave = dev_get_parent_priv(bus);
	struct zynq_qspi_priv *priv = dev_get_priv(bus->parent);

	priv->max_hz = slave->max_hz;

	return 0;
}

static int zynq_qspi_probe(struct udevice *bus)
{
	struct zynq_qspi_plat *plat = dev_get_plat(bus);
	struct zynq_qspi_priv *priv = dev_get_priv(bus);
	struct clk clk;
	unsigned long clock;
	int ret;

	priv->regs = plat->regs;
	priv->fifo_depth = ZYNQ_QSPI_FIFO_DEPTH;

	ret = clk_get_by_name(bus, "ref_clk", &clk);
	if (ret < 0) {
		dev_err(bus, "failed to get clock\n");
		return ret;
	}

	clock = clk_get_rate(&clk);
	if (IS_ERR_VALUE(clock)) {
		dev_err(bus, "failed to get rate\n");
		return clock;
	}

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(bus, "failed to enable clock\n");
		return ret;
	}

	/* init the zynq spi hw */
	zynq_qspi_init_hw(priv);

	plat->frequency = clock;
	plat->speed_hz = plat->frequency / 2;

	debug("%s: max-frequency=%d\n", __func__, plat->speed_hz);

	return 0;
}

/**
 * zynq_qspi_read_data - Copy data to RX buffer
 * @priv:	Pointer to the zynq_qspi_priv structure
 * @data:	The 32 bit variable where data is stored
 * @size:	Number of bytes to be copied from data to RX buffer
 */
static void zynq_qspi_read_data(struct zynq_qspi_priv *priv, u32 data, u8 size)
{
	u8 byte3;

	debug("%s: data 0x%04x rx_buf addr: 0x%08x size %d\n", __func__ ,
	      data, (unsigned)(priv->rx_buf), size);

	if (priv->rx_buf) {
		switch (size) {
		case 1:
			*((u8 *)priv->rx_buf) = data;
			priv->rx_buf += 1;
			break;
		case 2:
			*((u8 *)priv->rx_buf) = data;
			priv->rx_buf += 1;
			*((u8 *)priv->rx_buf) = (u8)(data >> 8);
			priv->rx_buf += 1;
			break;
		case 3:
			*((u8 *)priv->rx_buf) = data;
			priv->rx_buf += 1;
			*((u8 *)priv->rx_buf) = (u8)(data >> 8);
			priv->rx_buf += 1;
			byte3 = (u8)(data >> 16);
			*((u8 *)priv->rx_buf) = byte3;
			priv->rx_buf += 1;
			break;
		case 4:
			/* Can not assume word aligned buffer */
			memcpy(priv->rx_buf, &data, size);
			priv->rx_buf += 4;
			break;
		default:
			/* This will never execute */
			break;
		}
	}
	priv->bytes_to_receive -= size;
	if (priv->bytes_to_receive < 0)
		priv->bytes_to_receive = 0;
}

/**
 * zynq_qspi_write_data - Copy data from TX buffer
 * @priv:	Pointer to the zynq_qspi_priv structure
 * @data:	Pointer to the 32 bit variable where data is to be copied
 * @size:	Number of bytes to be copied from TX buffer to data
 */
static void zynq_qspi_write_data(struct  zynq_qspi_priv *priv,
		u32 *data, u8 size)
{
	if (priv->tx_buf) {
		switch (size) {
		case 1:
			*data = *((u8 *)priv->tx_buf);
			priv->tx_buf += 1;
			*data |= 0xFFFFFF00;
			break;
		case 2:
			*data = *((u8 *)priv->tx_buf);
			priv->tx_buf += 1;
			*data |= (*((u8 *)priv->tx_buf) << 8);
			priv->tx_buf += 1;
			*data |= 0xFFFF0000;
			break;
		case 3:
			*data = *((u8 *)priv->tx_buf);
			priv->tx_buf += 1;
			*data |= (*((u8 *)priv->tx_buf) << 8);
			priv->tx_buf += 1;
			*data |= (*((u8 *)priv->tx_buf) << 16);
			priv->tx_buf += 1;
			*data |= 0xFF000000;
			break;
		case 4:
			/* Can not assume word aligned buffer */
			memcpy(data, priv->tx_buf, size);
			priv->tx_buf += 4;
			break;
		default:
			/* This will never execute */
			break;
		}
	} else {
		*data = 0;
	}

	debug("%s: data 0x%08x tx_buf addr: 0x%08x size %d\n", __func__,
	      *data, (u32)priv->tx_buf, size);

	priv->bytes_to_transfer -= size;
	if (priv->bytes_to_transfer < 0)
		priv->bytes_to_transfer = 0;
}

/**
 * zynq_qspi_chipselect - Select or deselect the chip select line
 * @priv:	Pointer to the zynq_qspi_priv structure
 * @is_on:	Select(1) or deselect (0) the chip select line
 */
static void zynq_qspi_chipselect(struct  zynq_qspi_priv *priv, int is_on)
{
	u32 confr;
	struct zynq_qspi_regs *regs = priv->regs;

	confr = readl(&regs->cr);

	if (is_on) {
		/* Select the slave */
		confr &= ~ZYNQ_QSPI_CR_SS_MASK;
		confr |= (~(1 << priv->cs) << ZYNQ_QSPI_CR_SS_SHIFT) &
					ZYNQ_QSPI_CR_SS_MASK;
	} else
		/* Deselect the slave */
		confr |= ZYNQ_QSPI_CR_SS_MASK;

	writel(confr, &regs->cr);
}

/**
 * zynq_qspi_fill_tx_fifo - Fills the TX FIFO with as many bytes as possible
 * @priv:	Pointer to the zynq_qspi_priv structure
 * @size:	Number of bytes to be copied to fifo
 */
static void zynq_qspi_fill_tx_fifo(struct zynq_qspi_priv *priv, u32 size)
{
	u32 data = 0;
	u32 fifocount = 0;
	unsigned len, offset;
	struct zynq_qspi_regs *regs = priv->regs;
	static const unsigned offsets[4] = {
		ZYNQ_QSPI_TXD_00_00_OFFSET, ZYNQ_QSPI_TXD_00_01_OFFSET,
		ZYNQ_QSPI_TXD_00_10_OFFSET, ZYNQ_QSPI_TXD_00_11_OFFSET };

	while ((fifocount < size) &&
			(priv->bytes_to_transfer > 0)) {
		if (priv->bytes_to_transfer >= 4) {
			if (priv->tx_buf) {
				memcpy(&data, priv->tx_buf, 4);
				priv->tx_buf += 4;
			} else {
				data = 0;
			}
			writel(data, &regs->txd0r);
			priv->bytes_to_transfer -= 4;
			fifocount++;
		} else {
			/* Write TXD1, TXD2, TXD3 only if TxFIFO is empty. */
			if (!(readl(&regs->isr)
					& ZYNQ_QSPI_IXR_TXOW_MASK) &&
					!priv->rx_buf)
				return;
			len = priv->bytes_to_transfer;
			zynq_qspi_write_data(priv, &data, len);
			offset = (priv->rx_buf) ? offsets[0] : offsets[len];
			writel(data, &regs->cr + (offset / 4));
		}
	}
}

/**
 * zynq_qspi_irq_poll - Interrupt service routine of the QSPI controller
 * @priv:	Pointer to the zynq_qspi structure
 *
 * This function handles TX empty and Mode Fault interrupts only.
 * On TX empty interrupt this function reads the received data from RX FIFO and
 * fills the TX FIFO if there is any data remaining to be transferred.
 * On Mode Fault interrupt this function indicates that transfer is completed,
 * the SPI subsystem will identify the error as the remaining bytes to be
 * transferred is non-zero.
 *
 * returns:	0 for poll timeout
 *		1 transfer operation complete
 */
static int zynq_qspi_irq_poll(struct zynq_qspi_priv *priv)
{
	struct zynq_qspi_regs *regs = priv->regs;
	u32 rxindex = 0;
	u32 rxcount;
	u32 status, timeout;

	/* Poll until any of the interrupt status bits are set */
	timeout = get_timer(0);
	do {
		status = readl(&regs->isr);
	} while ((status == 0) &&
		(get_timer(timeout) < ZYNQ_QSPI_WAIT));

	if (status == 0) {
		printf("zynq_qspi_irq_poll: Timeout!\n");
		return -ETIMEDOUT;
	}

	writel(status, &regs->isr);

	/* Disable all interrupts */
	writel(ZYNQ_QSPI_IXR_ALL_MASK, &regs->idr);
	if ((status & ZYNQ_QSPI_IXR_TXOW_MASK) ||
	    (status & ZYNQ_QSPI_IXR_RXNEMPTY_MASK)) {
		/*
		 * This bit is set when Tx FIFO has < THRESHOLD entries. We have
		 * the THRESHOLD value set to 1, so this bit indicates Tx FIFO
		 * is empty
		 */
		rxcount = priv->bytes_to_receive - priv->bytes_to_transfer;
		rxcount = (rxcount % 4) ? ((rxcount/4)+1) : (rxcount/4);
		while ((rxindex < rxcount) &&
				(rxindex < ZYNQ_QSPI_RXFIFO_THRESHOLD)) {
			/* Read out the data from the RX FIFO */
			u32 data;
			data = readl(&regs->drxr);

			if (priv->bytes_to_receive >= 4) {
				if (priv->rx_buf) {
					memcpy(priv->rx_buf, &data, 4);
					priv->rx_buf += 4;
				}
				priv->bytes_to_receive -= 4;
			} else {
				zynq_qspi_read_data(priv, data,
						    priv->bytes_to_receive);
			}
			rxindex++;
		}

		if (priv->bytes_to_transfer) {
			/* There is more data to send */
			zynq_qspi_fill_tx_fifo(priv,
					       ZYNQ_QSPI_RXFIFO_THRESHOLD);

			writel(ZYNQ_QSPI_IXR_ALL_MASK, &regs->ier);
		} else {
			/*
			 * If transfer and receive is completed then only send
			 * complete signal
			 */
			if (!priv->bytes_to_receive) {
				/* return operation complete */
				writel(ZYNQ_QSPI_IXR_ALL_MASK,
				       &regs->idr);
				return 1;
			}
		}
	}

	return 0;
}

/**
 * zynq_qspi_start_transfer - Initiates the QSPI transfer
 * @priv:	Pointer to the zynq_qspi_priv structure
 *
 * This function fills the TX FIFO, starts the QSPI transfer, and waits for the
 * transfer to be completed.
 *
 * returns:	Number of bytes transferred in the last transfer
 */
static int zynq_qspi_start_transfer(struct zynq_qspi_priv *priv)
{
	u32 data = 0;
	struct zynq_qspi_regs *regs = priv->regs;

	debug("%s: qspi: 0x%08x transfer: 0x%08x len: %d\n", __func__,
	      (u32)priv, (u32)priv, priv->len);

	priv->bytes_to_transfer = priv->len;
	priv->bytes_to_receive = priv->len;

	if (priv->len < 4)
		zynq_qspi_fill_tx_fifo(priv, priv->len);
	else
		zynq_qspi_fill_tx_fifo(priv, priv->fifo_depth);

	writel(ZYNQ_QSPI_IXR_ALL_MASK, &regs->ier);

	/* wait for completion */
	do {
		data = zynq_qspi_irq_poll(priv);
	} while (data == 0);

	return (priv->len) - (priv->bytes_to_transfer);
}

static int zynq_qspi_transfer(struct zynq_qspi_priv *priv)
{
	unsigned cs_change = 1;
	int status = 0;

	while (1) {
		/* Select the chip if required */
		if (cs_change)
			zynq_qspi_chipselect(priv, 1);

		cs_change = priv->cs_change;

		if (!priv->tx_buf && !priv->rx_buf && priv->len) {
			status = -1;
			break;
		}

		/* Request the transfer */
		if (priv->len) {
			status = zynq_qspi_start_transfer(priv);
			priv->is_inst = 0;
		}

		if (status != priv->len) {
			if (status > 0)
				status = -EMSGSIZE;
			debug("zynq_qspi_transfer:%d len:%d\n",
			      status, priv->len);
			break;
		}
		status = 0;

		if (cs_change)
			/* Deselect the chip */
			zynq_qspi_chipselect(priv, 0);

		break;
	}

	return status;
}

static int zynq_qspi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct zynq_qspi_priv *priv = dev_get_priv(bus);
	struct zynq_qspi_regs *regs = priv->regs;

	writel(ZYNQ_QSPI_ENR_SPI_EN_MASK, &regs->enr);

	return 0;
}

static int zynq_qspi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct zynq_qspi_priv *priv = dev_get_priv(bus);
	struct zynq_qspi_regs *regs = priv->regs;

	writel(~ZYNQ_QSPI_ENR_SPI_EN_MASK, &regs->enr);

	return 0;
}

static int zynq_qspi_xfer(struct udevice *dev, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct zynq_qspi_priv *priv = dev_get_priv(bus);
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(dev);

	priv->cs = slave_plat->cs;
	priv->tx_buf = dout;
	priv->rx_buf = din;
	priv->len = bitlen / 8;

	debug("zynq_qspi_xfer: bus:%i cs:%i bitlen:%i len:%i flags:%lx\n",
	      dev_seq(bus), slave_plat->cs, bitlen, priv->len, flags);

	/*
	 * Festering sore.
	 * Assume that the beginning of a transfer with bits to
	 * transmit must contain a device command.
	 */
	if (dout && flags & SPI_XFER_BEGIN)
		priv->is_inst = 1;
	else
		priv->is_inst = 0;

	if (flags & SPI_XFER_END)
		priv->cs_change = 1;
	else
		priv->cs_change = 0;

	zynq_qspi_transfer(priv);

	return 0;
}

static int zynq_qspi_set_speed(struct udevice *bus, uint speed)
{
	struct zynq_qspi_plat *plat = dev_get_plat(bus);
	struct zynq_qspi_priv *priv = dev_get_priv(bus);
	struct zynq_qspi_regs *regs = priv->regs;
	uint32_t confr;
	u8 baud_rate_val = 0;

	if (!speed || speed > priv->max_hz)
		speed = priv->max_hz;

	/* Set the clock frequency */
	confr = readl(&regs->cr);
	if (plat->speed_hz != speed) {
		while ((baud_rate_val < ZYNQ_QSPI_CR_BAUD_MAX) &&
		       ((plat->frequency /
		       (2 << baud_rate_val)) > speed))
			baud_rate_val++;

		if (baud_rate_val > ZYNQ_QSPI_MAX_BAUD_RATE)
			baud_rate_val = ZYNQ_QSPI_DEFAULT_BAUD_RATE;

		plat->speed_hz = speed / (2 << baud_rate_val);
	}
	confr &= ~ZYNQ_QSPI_CR_BAUD_MASK;
	confr |= (baud_rate_val << ZYNQ_QSPI_CR_BAUD_SHIFT);

	writel(confr, &regs->cr);
	priv->freq = speed;

	debug("%s: regs=%p, speed=%d\n", __func__, priv->regs, priv->freq);

	return 0;
}

static int zynq_qspi_set_mode(struct udevice *bus, uint mode)
{
	struct zynq_qspi_priv *priv = dev_get_priv(bus);
	struct zynq_qspi_regs *regs = priv->regs;
	uint32_t confr;

	/* Set the SPI Clock phase and polarities */
	confr = readl(&regs->cr);
	confr &= ~(ZYNQ_QSPI_CR_CPHA_MASK | ZYNQ_QSPI_CR_CPOL_MASK);

	if (mode & SPI_CPHA)
		confr |= ZYNQ_QSPI_CR_CPHA_MASK;
	if (mode & SPI_CPOL)
		confr |= ZYNQ_QSPI_CR_CPOL_MASK;

	writel(confr, &regs->cr);
	priv->mode = mode;

	debug("%s: regs=%p, mode=%d\n", __func__, priv->regs, priv->mode);

	return 0;
}

static int zynq_qspi_exec_op(struct spi_slave *slave,
			     const struct spi_mem_op *op)
{
	int op_len, pos = 0, ret, i;
	unsigned int flag = 0;
	const u8 *tx_buf = NULL;
	u8 *rx_buf = NULL;

	if (op->data.nbytes) {
		if (op->data.dir == SPI_MEM_DATA_IN)
			rx_buf = op->data.buf.in;
		else
			tx_buf = op->data.buf.out;
	}

	op_len = op->cmd.nbytes + op->addr.nbytes + op->dummy.nbytes;

	u8 op_buf[op_len];

	op_buf[pos++] = op->cmd.opcode;

	if (op->addr.nbytes) {
		for (i = 0; i < op->addr.nbytes; i++)
			op_buf[pos + i] = op->addr.val >>
			(8 * (op->addr.nbytes - i - 1));

		pos += op->addr.nbytes;
	}

	if (op->dummy.nbytes)
		memset(op_buf + pos, 0xff, op->dummy.nbytes);

	/* 1st transfer: opcode + address + dummy cycles */
	/* Make sure to set END bit if no tx or rx data messages follow */
	if (!tx_buf && !rx_buf)
		flag |= SPI_XFER_END;

	ret = zynq_qspi_xfer(slave->dev, op_len * 8, op_buf, NULL,
			     flag | SPI_XFER_BEGIN);
	if (ret)
		return ret;

	/* 2nd transfer: rx or tx data path */
	if (tx_buf || rx_buf) {
		ret = zynq_qspi_xfer(slave->dev, op->data.nbytes * 8, tx_buf,
				     rx_buf, flag | SPI_XFER_END);
		if (ret)
			return ret;
	}

	spi_release_bus(slave);

	return 0;
}

static int zynq_qspi_check_buswidth(struct spi_slave *slave, u8 width)
{
	u32 mode = slave->mode;

	switch (width) {
	case 1:
		return 0;
	case 2:
		if (mode & SPI_RX_DUAL)
			return 0;
		break;
	case 4:
		if (mode & SPI_RX_QUAD)
			return 0;
		break;
	}

	return -EOPNOTSUPP;
}

static bool zynq_qspi_mem_exec_op(struct spi_slave *slave,
				  const struct spi_mem_op *op)
{
	if (zynq_qspi_check_buswidth(slave, op->cmd.buswidth))
		return false;

	if (op->addr.nbytes &&
	    zynq_qspi_check_buswidth(slave, op->addr.buswidth))
		return false;

	if (op->dummy.nbytes &&
	    zynq_qspi_check_buswidth(slave, op->dummy.buswidth))
		return false;

	if (op->data.dir != SPI_MEM_NO_DATA &&
	    zynq_qspi_check_buswidth(slave, op->data.buswidth))
		return false;

	return true;
}

static const struct spi_controller_mem_ops zynq_qspi_mem_ops = {
	.exec_op = zynq_qspi_exec_op,
	.supports_op = zynq_qspi_mem_exec_op,
};

static const struct dm_spi_ops zynq_qspi_ops = {
	.claim_bus      = zynq_qspi_claim_bus,
	.release_bus    = zynq_qspi_release_bus,
	.xfer           = zynq_qspi_xfer,
	.set_speed      = zynq_qspi_set_speed,
	.set_mode       = zynq_qspi_set_mode,
	.mem_ops        = &zynq_qspi_mem_ops,
};

static const struct udevice_id zynq_qspi_ids[] = {
	{ .compatible = "xlnx,zynq-qspi-1.0" },
	{ }
};

U_BOOT_DRIVER(zynq_qspi) = {
	.name   = "zynq_qspi",
	.id     = UCLASS_SPI,
	.of_match = zynq_qspi_ids,
	.ops    = &zynq_qspi_ops,
	.of_to_plat = zynq_qspi_of_to_plat,
	.plat_auto	= sizeof(struct zynq_qspi_plat),
	.priv_auto	= sizeof(struct zynq_qspi_priv),
	.probe  = zynq_qspi_probe,
	.child_pre_probe = zynq_qspi_child_pre_probe,
};
