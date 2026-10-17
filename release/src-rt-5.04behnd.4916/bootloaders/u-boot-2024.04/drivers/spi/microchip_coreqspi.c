// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Microchip Technology Inc.
 * Padmarao Begari <padmarao.begari@microchip.com>
 * Naga Sureshkumar Relli <nagasuresh.relli@microchip.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <spi-mem.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * QSPI Control register mask defines
 */
#define CONTROL_ENABLE         BIT(0)
#define CONTROL_MASTER         BIT(1)
#define CONTROL_XIP            BIT(2)
#define CONTROL_XIPADDR        BIT(3)
#define CONTROL_CLKIDLE        BIT(10)
#define CONTROL_SAMPLE_MASK    GENMASK(12, 11)
#define CONTROL_MODE0          BIT(13)
#define CONTROL_MODE12_MASK    GENMASK(15, 14)
#define CONTROL_MODE12_EX_RO   BIT(14)
#define CONTROL_MODE12_EX_RW   BIT(15)
#define CONTROL_MODE12_FULL    GENMASK(15, 14)
#define CONTROL_FLAGSX4        BIT(16)
#define CONTROL_CLKRATE_MASK   GENMASK(27, 24)
#define CONTROL_CLKRATE_SHIFT  24

/*
 * QSPI Frames register mask defines
 */
#define FRAMES_TOTALBYTES_MASK GENMASK(15, 0)
#define FRAMES_CMDBYTES_MASK   GENMASK(24, 16)
#define FRAMES_CMDBYTES_SHIFT  16
#define FRAMES_SHIFT           25
#define FRAMES_IDLE_MASK       GENMASK(29, 26)
#define FRAMES_IDLE_SHIFT      26
#define FRAMES_FLAGBYTE        BIT(30)
#define FRAMES_FLAGWORD        BIT(31)

/*
 * QSPI Interrupt Enable register mask defines
 */
#define IEN_TXDONE             BIT(0)
#define IEN_RXDONE             BIT(1)
#define IEN_RXAVAILABLE        BIT(2)
#define IEN_TXAVAILABLE        BIT(3)
#define IEN_RXFIFOEMPTY        BIT(4)
#define IEN_TXFIFOFULL         BIT(5)

/*
 * QSPI Status register mask defines
 */
#define STATUS_TXDONE          BIT(0)
#define STATUS_RXDONE          BIT(1)
#define STATUS_RXAVAILABLE     BIT(2)
#define STATUS_TXAVAILABLE     BIT(3)
#define STATUS_RXFIFOEMPTY     BIT(4)
#define STATUS_TXFIFOFULL      BIT(5)
#define STATUS_READY           BIT(7)
#define STATUS_FLAGSX4         BIT(8)
#define STATUS_MASK            GENMASK(8, 0)

#define BYTESUPPER_MASK        GENMASK(31, 16)
#define BYTESLOWER_MASK        GENMASK(15, 0)

#define MAX_DIVIDER            16
#define MIN_DIVIDER            0
#define MAX_DATA_CMD_LEN       256

/* QSPI ready time out value */
#define TIMEOUT_MS             (1000 * 500)

/*
 * QSPI Register offsets.
 */
#define REG_CONTROL            (0x00)
#define REG_FRAMES             (0x04)
#define REG_IEN                (0x0c)
#define REG_STATUS             (0x10)
#define REG_DIRECT_ACCESS      (0x14)
#define REG_UPPER_ACCESS       (0x18)
#define REG_RX_DATA            (0x40)
#define REG_TX_DATA            (0x44)
#define REG_X4_RX_DATA         (0x48)
#define REG_X4_TX_DATA         (0x4c)
#define REG_FRAMESUP           (0x50)

/**
 * struct mchp_coreqspi - Defines qspi driver instance
 * @regs:              Address of the QSPI controller registers
 * @freq:              QSPI Input frequency
 * @txbuf:             TX buffer
 * @rxbuf:             RX buffer
 * @tx_len:            Number of bytes left to transfer
 * @rx_len:            Number of bytes left to receive
 */
struct mchp_coreqspi {
	void __iomem *regs;
	u32 freq;
	u8 *txbuf;
	u8 *rxbuf;
	int tx_len;
	int rx_len;
};

static void mchp_coreqspi_init_hw(struct mchp_coreqspi *qspi)
{
	u32 control;

	control = CONTROL_CLKIDLE | CONTROL_ENABLE;

	writel(control, qspi->regs + REG_CONTROL);
	writel(0, qspi->regs + REG_IEN);
}

static inline void mchp_coreqspi_read_op(struct mchp_coreqspi *qspi)
{
	u32 control, data;

	if (!qspi->rx_len)
		return;

	control = readl(qspi->regs + REG_CONTROL);

	/*
	 * Read 4-bytes from the SPI FIFO in single transaction and then read
	 * the reamaining data byte wise.
	 */
	control |= CONTROL_FLAGSX4;
	writel(control, qspi->regs + REG_CONTROL);

	while (qspi->rx_len >= 4) {
		while (readl(qspi->regs + REG_STATUS) & STATUS_RXFIFOEMPTY)
			;
		data = readl(qspi->regs + REG_X4_RX_DATA);
		*(u32 *)qspi->rxbuf = data;
		qspi->rxbuf += 4;
		qspi->rx_len -= 4;
	}

	control &= ~CONTROL_FLAGSX4;
	writel(control, qspi->regs + REG_CONTROL);

	while (qspi->rx_len--) {
		while (readl(qspi->regs + REG_STATUS) & STATUS_RXFIFOEMPTY)
			;
		data = readl(qspi->regs + REG_RX_DATA);
		*qspi->rxbuf++ = (data & 0xFF);
	}
}

static inline void mchp_coreqspi_write_op(struct mchp_coreqspi *qspi, bool word)
{
	u32 control, data;

	control = readl(qspi->regs + REG_CONTROL);
	control |= CONTROL_FLAGSX4;
	writel(control, qspi->regs + REG_CONTROL);

	while (qspi->tx_len >= 4) {
		while (readl(qspi->regs + REG_STATUS) & STATUS_TXFIFOFULL)
			;
		data = *(u32 *)qspi->txbuf;
		qspi->txbuf += 4;
		qspi->tx_len -= 4;
		writel(data, qspi->regs + REG_X4_TX_DATA);
	}

	control &= ~CONTROL_FLAGSX4;
	writel(control, qspi->regs + REG_CONTROL);

	while (qspi->tx_len--) {
		while (readl(qspi->regs + REG_STATUS) & STATUS_TXFIFOFULL)
			;
		data =  *qspi->txbuf++;
		writel(data, qspi->regs + REG_TX_DATA);
	}
}

static inline void mchp_coreqspi_config_op(struct mchp_coreqspi *qspi,
					   const struct spi_mem_op *op)
{
	u32 idle_cycles = 0;
	int total_bytes, cmd_bytes, frames, ctrl;

	cmd_bytes = op->cmd.nbytes + op->addr.nbytes;
	total_bytes = cmd_bytes + op->data.nbytes;

	/*
	 * As per the coreQSPI IP spec,the number of command and data bytes are
	 * controlled by the frames register for each SPI sequence. This supports
	 * the SPI flash memory read and writes sequences as below. so configure
	 * the cmd and total bytes accordingly.
	 * ---------------------------------------------------------------------
	 * TOTAL BYTES  |  CMD BYTES | What happens                             |
	 * ______________________________________________________________________
	 *              |            |                                          |
	 *     1        |   1        | The SPI core will transmit a single byte |
	 *              |            | and receive data is discarded            |
	 *              |            |                                          |
	 *     1        |   0        | The SPI core will transmit a single byte |
	 *              |            | and return a single byte                 |
	 *              |            |                                          |
	 *     10       |   4        | The SPI core will transmit 4 command     |
	 *              |            | bytes discarding the receive data and    |
	 *              |            | transmits 6 dummy bytes returning the 6  |
	 *              |            | received bytes and return a single byte  |
	 *              |            |                                          |
	 *     10       |   10       | The SPI core will transmit 10 command    |
	 *              |            |                                          |
	 *     10       |    0       | The SPI core will transmit 10 command    |
	 *              |            | bytes and returning 10 received bytes    |
	 * ______________________________________________________________________
	 */

	if (!(op->data.dir == SPI_MEM_DATA_IN))
		cmd_bytes = total_bytes;

	frames = total_bytes & BYTESUPPER_MASK;
	writel(frames, qspi->regs + REG_FRAMESUP);
	frames = total_bytes & BYTESLOWER_MASK;
	frames |= cmd_bytes << FRAMES_CMDBYTES_SHIFT;

	if (op->dummy.buswidth)
		idle_cycles = op->dummy.nbytes * 8 / op->dummy.buswidth;

	frames |= idle_cycles << FRAMES_IDLE_SHIFT;
	ctrl = readl(qspi->regs + REG_CONTROL);

	if (ctrl & CONTROL_MODE12_MASK)
		frames |= (1 << FRAMES_SHIFT);

	frames |= FRAMES_FLAGWORD;
	writel(frames, qspi->regs + REG_FRAMES);
}

static int mchp_coreqspi_wait_for_ready(struct spi_slave *slave)
{
	struct mchp_coreqspi *qspi = dev_get_priv(slave->dev->parent);
	unsigned long count = 0;

	while (1) {
		if (readl(qspi->regs + REG_STATUS) & STATUS_READY)
			return 0;

		udelay(1);
		count += 1;
		if (count == TIMEOUT_MS)
			return -ETIMEDOUT;
	}
}

static int mchp_coreqspi_set_operate_mode(struct mchp_coreqspi *qspi,
					  const struct spi_mem_op *op)
{
	u32 control = readl(qspi->regs + REG_CONTROL);

	/*
	 * The operating mode can be configured based on the command that needs
	 * to be send.
	 * bits[15:14]: Sets whether multiple bit SPI operates in normal,
	 *              extended or full modes.
	 *               00: Normal (single DQ0 TX and single DQ1 RX lines)
	 *               01: Extended RO (command and address bytes on DQ0 only)
	 *               10: Extended RW (command byte on DQ0 only)
	 *               11: Full. (command and address are on all DQ lines)
	 * bit[13]:     Sets whether multiple bit SPI uses 2 or 4 bits of data
	 *               0: 2-bits (BSPI)
	 *               1: 4-bits (QSPI)
	 */
	if (op->data.buswidth == 4 || op->data.buswidth == 2) {
		control &= ~CONTROL_MODE12_MASK;
		if (op->cmd.buswidth == 1 && (op->addr.buswidth == 1 ||
					      op->addr.buswidth == 0))
			control |= CONTROL_MODE12_EX_RO;
		else if (op->cmd.buswidth == 1)
			control |= CONTROL_MODE12_EX_RW;
		else
			control |= CONTROL_MODE12_FULL;

		control |= CONTROL_MODE0;
	} else {
		control &= ~(CONTROL_MODE12_MASK | CONTROL_MODE0);
	}

	writel(control, qspi->regs + REG_CONTROL);

	return 0;
}

static int mchp_coreqspi_exec_op(struct spi_slave *slave,
				 const struct spi_mem_op *op)
{
	struct mchp_coreqspi *qspi = dev_get_priv(slave->dev->parent);

	u32 address = op->addr.val;
	u8 opcode = op->cmd.opcode;
	u8 opaddr[5];
	int err = 0, i;

	err = mchp_coreqspi_wait_for_ready(slave);
	if (err)
		return err;

	err = mchp_coreqspi_set_operate_mode(qspi, op);
	if (err)
		return err;

	mchp_coreqspi_config_op(qspi, op);
	if (op->cmd.opcode) {
		qspi->txbuf = &opcode;
		qspi->rxbuf = NULL;
		qspi->tx_len = op->cmd.nbytes;
		qspi->rx_len = 0;
		mchp_coreqspi_write_op(qspi, false);
	}

	qspi->txbuf = &opaddr[0];
	if (op->addr.nbytes) {
		for (i = 0; i < op->addr.nbytes; i++)
			qspi->txbuf[i] = address >> (8 * (op->addr.nbytes - i - 1));

		qspi->rxbuf = NULL;
		qspi->tx_len = op->addr.nbytes;
		qspi->rx_len = 0;
		mchp_coreqspi_write_op(qspi, false);
	}

	if (op->data.nbytes) {
		if (op->data.dir == SPI_MEM_DATA_OUT) {
			qspi->txbuf = (u8 *)op->data.buf.out;
			qspi->rxbuf = NULL;
			qspi->rx_len = 0;
			qspi->tx_len = op->data.nbytes;
			mchp_coreqspi_write_op(qspi, true);
		} else {
			qspi->txbuf = NULL;
			qspi->rxbuf = (u8 *)op->data.buf.in;
			qspi->rx_len = op->data.nbytes;
			qspi->tx_len = 0;
			mchp_coreqspi_read_op(qspi);
		}
	}

	return 0;
}

static bool mchp_coreqspi_supports_op(struct spi_slave *slave,
				      const struct spi_mem_op *op)
{
	if (!spi_mem_default_supports_op(slave, op))
		return false;

	if ((op->data.buswidth == 4 || op->data.buswidth == 2) &&
	    (op->cmd.buswidth == 1 && (op->addr.buswidth == 1 ||
	     op->addr.buswidth == 0))) {
		/*
		 * If the command and address are on DQ0 only, then this
		 * controller doesn't support sending data on dual and
		 * quad lines. but it supports reading data on dual and
		 * quad lines with same configuration as command and
		 * address on DQ0.
		 * i.e. The control register[15:13] :EX_RO(read only) is
		 * meant only for the command and address are on DQ0 but
		 * not to write data, it is just to read.
		 * Ex: 0x34h is Quad Load Program Data which is not
		 * supported. Then the spi-mem layer will iterate over
		 * each command and it will chose the supported one.
		 */
		if (op->data.dir == SPI_MEM_DATA_OUT)
			return false;
	}

	return true;
}

static int mchp_coreqspi_adjust_op_size(struct spi_slave *slave,
					struct spi_mem_op *op)
{
	if (op->data.dir == SPI_MEM_DATA_OUT) {
		if (op->data.nbytes > MAX_DATA_CMD_LEN)
			op->data.nbytes = MAX_DATA_CMD_LEN;
	}

	return 0;
}

static int mchp_coreqspi_set_speed(struct udevice *dev, uint speed)
{
	struct mchp_coreqspi *qspi = dev_get_priv(dev);
	u32 control, baud_rate_val = 0;

	if (speed > (qspi->freq / 2))
		speed = qspi->freq / 2;

	baud_rate_val = DIV_ROUND_UP(qspi->freq, 2 * speed);
	if (baud_rate_val >= MAX_DIVIDER || baud_rate_val <= MIN_DIVIDER)
		return -EINVAL;

	control = readl(qspi->regs + REG_CONTROL);
	control &= ~CONTROL_CLKRATE_MASK;
	control |= baud_rate_val << CONTROL_CLKRATE_SHIFT;
	writel(control, qspi->regs + REG_CONTROL);

	return 0;
}

static int mchp_coreqspi_set_mode(struct udevice *dev, uint mode)
{
	struct mchp_coreqspi *qspi = dev_get_priv(dev);
	u32 control;

	control = readl(qspi->regs + REG_CONTROL);

	if ((mode & SPI_CPOL) && (mode & SPI_CPHA))
		control |= CONTROL_CLKIDLE;
	else
		control &= ~CONTROL_CLKIDLE;

	writel(control, qspi->regs + REG_CONTROL);

	return 0;
}

static int mchp_coreqspi_claim_bus(struct udevice *dev)
{
	return 0;
}

static int mchp_coreqspi_release_bus(struct udevice *dev)
{
	return 0;
}

static int mchp_coreqspi_probe(struct udevice *dev)
{
	struct mchp_coreqspi *qspi = dev_get_priv(dev);
	struct clk clk;
	ulong clk_rate;
	int ret;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return -EINVAL;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	clk_rate = clk_get_rate(&clk);
	if (!clk_rate)
		return -EINVAL;
	qspi->freq = clk_rate;

	qspi->regs = dev_read_addr_ptr(dev);
	if (!qspi->regs)
		return -EINVAL;

	/* Init the mpfs qspi hw */
	mchp_coreqspi_init_hw(qspi);

	return 0;
}

static const struct spi_controller_mem_ops mchp_coreqspi_mem_ops = {
	.adjust_op_size = mchp_coreqspi_adjust_op_size,
	.supports_op = mchp_coreqspi_supports_op,
	.exec_op = mchp_coreqspi_exec_op,
};

static const struct dm_spi_ops mchp_coreqspi_ops = {
	.claim_bus      = mchp_coreqspi_claim_bus,
	.release_bus    = mchp_coreqspi_release_bus,
	.set_speed      = mchp_coreqspi_set_speed,
	.set_mode       = mchp_coreqspi_set_mode,
	.mem_ops        = &mchp_coreqspi_mem_ops,
};

static const struct udevice_id mchp_coreqspi_ids[] = {
	{ .compatible = "microchip,mpfs-coreqspi-rtl-v2" },
	{ .compatible = "microchip,mpfs-qspi" },
	{ }
};

U_BOOT_DRIVER(mchp_coreqspi) = {
	.name   = "mchp_coreqspi",
	.id     = UCLASS_SPI,
	.of_match = mchp_coreqspi_ids,
	.ops    = &mchp_coreqspi_ops,
	.priv_auto = sizeof(struct mchp_coreqspi),
	.probe  = mchp_coreqspi_probe,
};
