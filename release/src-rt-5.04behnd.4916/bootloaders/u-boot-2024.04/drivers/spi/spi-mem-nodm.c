// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <spi-mem.h>

int spi_mem_exec_op(struct spi_slave *slave,
		    const struct spi_mem_op *op)
{
	unsigned int pos = 0;
	const u8 *tx_buf = NULL;
	u8 *rx_buf = NULL;
	u8 *op_buf;
	int op_len;
	u32 flag;
	int ret;
	int i;

	if (op->data.nbytes) {
		if (op->data.dir == SPI_MEM_DATA_IN)
			rx_buf = op->data.buf.in;
		else
			tx_buf = op->data.buf.out;
	}

	op_len = op->cmd.nbytes + op->addr.nbytes + op->dummy.nbytes;
	op_buf = calloc(1, op_len);

	ret = spi_claim_bus(slave);
	if (ret < 0)
		return ret;

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
	flag = SPI_XFER_BEGIN;
	/* Make sure to set END bit if no tx or rx data messages follow */
	if (!tx_buf && !rx_buf)
		flag |= SPI_XFER_END;

	ret = spi_xfer(slave, op_len * 8, op_buf, NULL, flag);
	if (ret)
		return ret;

	/* 2nd transfer: rx or tx data path */
	if (tx_buf || rx_buf) {
		ret = spi_xfer(slave, op->data.nbytes * 8, tx_buf,
			       rx_buf, SPI_XFER_END);
		if (ret)
			return ret;
	}

	spi_release_bus(slave);

	for (i = 0; i < pos; i++)
		debug("%02x ", op_buf[i]);
	debug("| [%dB %s] ",
	      tx_buf || rx_buf ? op->data.nbytes : 0,
	      tx_buf || rx_buf ? (tx_buf ? "out" : "in") : "-");
	for (i = 0; i < op->data.nbytes; i++)
		debug("%02x ", tx_buf ? tx_buf[i] : rx_buf[i]);
	debug("[ret %d]\n", ret);

	free(op_buf);

	if (ret < 0)
		return ret;

	return 0;
}

int spi_mem_adjust_op_size(struct spi_slave *slave,
			   struct spi_mem_op *op)
{
	unsigned int len;

	len = op->cmd.nbytes + op->addr.nbytes + op->dummy.nbytes;
	if (slave->max_write_size && len > slave->max_write_size)
		return -EINVAL;

	if (op->data.dir == SPI_MEM_DATA_IN) {
		if (slave->max_read_size)
			op->data.nbytes = min(op->data.nbytes,
					      slave->max_read_size);
	} else if (slave->max_write_size) {
		op->data.nbytes = min(op->data.nbytes,
				      slave->max_write_size - len);
	}

	if (!op->data.nbytes)
		return -EINVAL;

	return 0;
}

static int spi_check_buswidth_req(struct spi_slave *slave, u8 buswidth, bool tx)
{
	u32 mode = slave->mode;

	switch (buswidth) {
	case 1:
		return 0;

	case 2:
		if ((tx && (mode & (SPI_TX_DUAL | SPI_TX_QUAD))) ||
		    (!tx && (mode & (SPI_RX_DUAL | SPI_RX_QUAD))))
			return 0;

		break;

	case 4:
		if ((tx && (mode & SPI_TX_QUAD)) ||
		    (!tx && (mode & SPI_RX_QUAD)))
			return 0;

		break;
	case 8:
		if ((tx && (mode & SPI_TX_OCTAL)) ||
		    (!tx && (mode & SPI_RX_OCTAL)))
			return 0;

		break;

	default:
		break;
	}

	return -ENOTSUPP;
}

bool spi_mem_supports_op(struct spi_slave *slave, const struct spi_mem_op *op)
{
	if (spi_check_buswidth_req(slave, op->cmd.buswidth, true))
		return false;

	if (op->addr.nbytes &&
	    spi_check_buswidth_req(slave, op->addr.buswidth, true))
		return false;

	if (op->dummy.nbytes &&
	    spi_check_buswidth_req(slave, op->dummy.buswidth, true))
		return false;

	if (op->data.nbytes &&
	    spi_check_buswidth_req(slave, op->data.buswidth,
				   op->data.dir == SPI_MEM_DATA_OUT))
		return false;

	if (op->cmd.dtr || op->addr.dtr || op->dummy.dtr || op->data.dtr)
		return false;

	if (op->cmd.nbytes != 1)
		return false;

	return true;
}
