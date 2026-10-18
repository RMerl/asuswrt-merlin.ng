/*
 * Copyright (C) 2017 Broadcom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#define pr_fmt(fmt)            KBUILD_MODNAME ": " fmt

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/if.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/bbsi.h>

/***************************************************************************
* File Name  : bbsi.c
*
* Description: This file contains the functions for communicating between a brcm
*              cpe chip(eg 7366) to another brcm chip(6802) which is connected
*              as a spi slave device. This protocol used to communicate is BBSI.
*
***************************************************************************/

#define BBSI_COMMAND_BYTE	0x80

#define STATUS_REGISTER_ADDR	0x6
#define CONFIG_REGISTER_ADDR	0x7
#define DATA0_REGISTER_ADDR	0xC

#define CPU_RUNNING_SHIFT	0x6
#define CPU_RUNNING_MASK	0x1

#define HAB_REQ_SHIFT		0x5
#define HAB_REQ_MASK		0x1

#define BUSY_SHIFT		0x4
#define BUSY_MASK		0x1

#define RBUS_UNEXP_TX_SHIFT	0x3
#define RBUS_UNEXP_TX_MASK	0x1

#define RBUS_TIMEOUT_SHIFT	0x2
#define RBUS_TIMEOUT_MASK	0x1

#define RBUS_ERR_ACK_SHIFT	0x1
#define RBUS_ERR_ACK_MASK	0x1

#define ERROR_SHIFT		0x0
#define ERROR_MASK		0x1

#define XFER_MODE_SHIFT		0x3
#define XFER_MODE_MASK		0x3

#define NO_RBUS_ADDR_INC_SHIFT	0x2
#define NO_RBUS_ADDR_INC_MASK	0x1

#define SPECULATIVE_READ_EN_SHIFT	0x1
#define SPECULATIVE_READ_EN_MASK	0x1

#define READ_RBUS_SHIFT		0x0
#define READ_RBUS_MASK		0x1

#define MAX_STATUS_RETRY	5

static int bbsi_is_done(struct spi_device *spi_device)
{
	u8 read_status[2] = {
		BBSI_COMMAND_BYTE,	/* | 0x1,  Do a Read */
		STATUS_REGISTER_ADDR
	};
	u8 read_rx[1];
	int status;
	int i;
	int ret = 0;

	for (i = 0; i < MAX_STATUS_RETRY; i++) {
		status =
		    spi_write_then_read(spi_device,
					read_status, sizeof(read_status),
					read_rx, sizeof(read_rx));

		if (0 != status) {
			dev_err(&spi_device->dev,
				"bbsi_is_done: spi returned error\n");
			ret = 0;
			break;
		}

		if (read_rx[0] & 0xF) {
			dev_err(&spi_device->dev,
				"bbsi_is_done: BBSI transaction error, status=0x%02X\n",
				read_rx[0]);
			ret = 0;
			break;
		} else if ((read_rx[0] & (1 << BUSY_SHIFT)) == 0) {
			ret = 1;
			break;
		}
	}

	return ret;
}

int bbsi_read(struct spi_device *spi_device, u32 addr,
	      u32 *data, unsigned long nbits)
{
	u8 buf[12];
	int status;

	buf[0] = BBSI_COMMAND_BYTE | 0x1;
	buf[1] = CONFIG_REGISTER_ADDR;	/* Start the writes from this addr */

	/* Indicates the transaction is 32bit, 24bit, 16bit or 8bit, 1..4 */
	buf[2] = ((4 - nbits) << XFER_MODE_SHIFT) | 0x1;

	buf[3] = (addr >> 24) & 0xFF;
	buf[4] = (addr >> 16) & 0xFF;
	buf[5] = (addr >> 8) & 0xFF;
	buf[6] = (addr >> 0) & 0xFF;

	status = spi_write(spi_device, buf, 7);

	if (0 != status) {
		dev_err(&spi_device->dev,
			"bbsi_read: Spi returned error\n");
		return (-1);
	}

	if (!bbsi_is_done(spi_device)) {
		dev_err(&spi_device->dev,
			"bbsi_read: read to addr:0x%x failed\n", addr);
		return (-1);
	}

	buf[0] = BBSI_COMMAND_BYTE;	/*read */
	buf[1] = DATA0_REGISTER_ADDR;

	status = spi_write_then_read(spi_device, buf, 2, buf, 4);

	if (0 != status) {
		dev_err(&spi_device->dev,
			"bbsi_read: spi_write_then_read returned error\n");
		return (-1);
	}

	*data = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];

	return 0;
}
EXPORT_SYMBOL(bbsi_read);

int bbsi_write(struct spi_device *spi_device, u32 addr,
	       u32 data, unsigned long nbits)
{
	u8 buf[12];
	int status;

	data <<= (8 * (4 - nbits));

	buf[0] = BBSI_COMMAND_BYTE | 0x1;	/* Write signal is 1 */
	buf[1] = CONFIG_REGISTER_ADDR;	/* Start the writes from this addr */

	/* Indicates the transaction is 32bit, 24bit, 16bit or 8bit. 1..4 */
	buf[2] = (4 - nbits) << XFER_MODE_SHIFT;

	buf[3] = (addr >> 24) & 0xFF;
	buf[4] = (addr >> 16) & 0xFF;
	buf[5] = (addr >> 8) & 0xFF;
	buf[6] = (addr >> 0) & 0xFF;

	buf[7] = (data >> 24) & 0xFF;
	buf[8] = (data >> 16) & 0xFF;
	buf[9] = (data >> 8) & 0xFF;
	buf[10] = (data >> 0) & 0xFF;

	status = spi_write(spi_device, buf, 11);
	if (0 != status) {
		dev_err(&spi_device->dev,
			"bbsi_write: Spi returned error\n");
		return (-1);
	}

	if (!bbsi_is_done(spi_device)) {
		dev_err(&spi_device->dev,
			"bbsi_write: write to addr:0x%x failed\n", addr);
		return (-1);
	}


	return 0;
}
EXPORT_SYMBOL(bbsi_write);

static int bbsi_do_read_buffer(struct spi_device *spi_device,
			       u32 addr, u32 *data,
			       unsigned long len)
{
	u8 buf[12];
	int status;

	buf[0] = BBSI_COMMAND_BYTE | 0x1;
	buf[1] = CONFIG_REGISTER_ADDR;	/* Start the writes from this addr */

	/* Indicates the transaction is 32bit, 24bit, 16bit or 8bit. 1..4 */
	buf[2] = 0x3;

	buf[3] = (addr >> 24) & 0xFF;
	buf[4] = (addr >> 16) & 0xFF;
	buf[5] = (addr >> 8) & 0xFF;
	buf[6] = (addr >> 0) & 0xFF;

	status = spi_write(spi_device, buf, 7);
	if (0 != status) {
		dev_err(&spi_device->dev,
			"bbsi_do_read_buffer: Spi returned error\n");
		return (-1);
	}

	if (0 != status) {
		dev_err(&spi_device->dev,
			"SPI Slave Read: BcmSpiSyncTrans returned error\n");
		return (-1);
	}

	if (!bbsi_is_done(spi_device)) {
		dev_err(&spi_device->dev,
			"SPI Slave Read: read to addr:0x%x failed\n", addr);
		return (-1);
	}

	buf[0] = BBSI_COMMAND_BYTE;	/*read */
	buf[1] = DATA0_REGISTER_ADDR;

	while (len) {
		unsigned int count;

		count = (len > 4 ? 4 : len);

		status = spi_write_then_read(spi_device, buf, 2,
					     data, count);
		if (0 != status) {
			dev_err(&spi_device->dev,
				"bbsi_do_read_buffer: spi_write_then_read returned error\n");
			return (-1);
		}

		if (!bbsi_is_done(spi_device)) {
			dev_err(&spi_device->dev,
				"SPI Slave Read: read to addr:0x%x failed\n",
				addr);
			return (-1);
		}

		len -= count;
		data += count / 4;
	}

	return 0;
}

static int bbsi_do_write_buffer(struct spi_device *spi_device,
				u32 addr, u32 *data,
				unsigned long len)
{
	u8 buf[512 + 7];
	int status;

	/* 7 bytes are used for addressing and BBSI protocol */
	if (len > sizeof(buf) - 7) {
		dev_err(&spi_device->dev,
			"SPI Slave Write: write to addr:0x%x failed. Len (%ld) too long.\n",
			addr, len);
		return (-1);
	}

	buf[0] = BBSI_COMMAND_BYTE | 0x1;	/* Write signal is 1 */
	buf[1] = CONFIG_REGISTER_ADDR;	/* Start the writes from this addr */
	buf[2] = 0;			/* Transactions are 32-bits */
	buf[3] = (addr >> 24) & 0xFF;
	buf[4] = (addr >> 16) & 0xFF;
	buf[5] = (addr >> 8) & 0xFF;
	buf[6] = (addr >> 0) & 0xFF;

	memcpy(&buf[7], data, len);

	status = spi_write(spi_device, buf, 7 + len);
	if (0 != status) {
		dev_err(&spi_device->dev,
			"SPI Slave Write: spi_write returned error\n");
		return (-1);
	}

	if (!bbsi_is_done(spi_device)) {
		dev_err(&spi_device->dev,
			"SPI Slave Write: write to addr:0x%x failed\n", addr);
		return (-1);
	}
	return 0;
}

int bbsi_readbuf(struct spi_device *spi_device, u32 addr,
		 u32 *data, unsigned long len)
{
	int ret = -1;

	addr &= 0x1fffffff;

	ret = bbsi_do_read_buffer(spi_device, addr, data, len);

	return ret;
}
EXPORT_SYMBOL(bbsi_readbuf);

int bbsi_writebuf(struct spi_device *spi_device, u32 addr,
		  u32 *data, unsigned long len)
{
	int ret = -1;
	int count = 0;

	addr &= 0x1fffffff;

	while (len) {
		count = (len > 500 ? 500 : len);

		ret = bbsi_do_write_buffer(spi_device, addr, data, count);
		if (ret)
			break;

		len -= count;
		addr += count;
		data += count / sizeof(*data);
	}

	return ret;
}
EXPORT_SYMBOL(bbsi_writebuf);

u32 bbsi_read32(struct spi_device *spi_device, u32 addr)
{
	u32 data = 0;

	addr &= 0x1fffffff;

	if (bbsi_read(spi_device, addr, &data, 4) < 0) {
		dev_err(&spi_device->dev,
			"bbsi_read32: can't read %08x\n", addr);
	}

	return data;
}
EXPORT_SYMBOL(bbsi_read32);

void bbsi_write32(struct spi_device *spi_device, u32 addr,
		  u32 data)
{
	addr &= 0x1fffffff;

	if (bbsi_write(spi_device, addr, data, 4) < 0) {
		dev_err(&spi_device->dev,
			"bbsi_write32: can't write %08x (data %08x)\n",
			addr, data);
	}
}
EXPORT_SYMBOL(bbsi_write32);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("BBSI library driver");
