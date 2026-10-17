/*
 * I2C Driver for Atmel ATSHA204 over I2C
 *
 * Copyright (C) 2014 Josh Datko, Cryptotronix, jbd@cryptotronix.com
 *		 2016 Tomas Hlavacek, CZ.NIC, tmshlvck@gmail.com
 *		 2017 Marek Behún, CZ.NIC, kabel@kernel.org
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <errno.h>
#include <atsha204a-i2c.h>
#include <log.h>
#include <asm/global_data.h>
#include <linux/delay.h>
#include <linux/bitrev.h>
#include <u-boot/crc.h>

#define ATSHA204A_TWLO_US		60
#define ATSHA204A_TWHI_US		2500
#define ATSHA204A_TRANSACTION_TIMEOUT	100000
#define ATSHA204A_TRANSACTION_RETRY	5
#define ATSHA204A_EXECTIME		5000

DECLARE_GLOBAL_DATA_PTR;

static inline u16 atsha204a_crc16(const u8 *buffer, size_t len)
{
	return bitrev16(crc16(0, buffer, len));
}

static int atsha204a_send(struct udevice *dev, const u8 *buf, u8 len)
{
	fdt_addr_t *priv = dev_get_priv(dev);
	struct i2c_msg msg;

	msg.addr = *priv;
	msg.flags = I2C_M_STOP;
	msg.len = len;
	msg.buf = (u8 *) buf;

	return dm_i2c_xfer(dev, &msg, 1);
}

static int atsha204a_recv(struct udevice *dev, u8 *buf, u8 len)
{
	fdt_addr_t *priv = dev_get_priv(dev);
	struct i2c_msg msg;

	msg.addr = *priv;
	msg.flags = I2C_M_RD | I2C_M_STOP;
	msg.len = len;
	msg.buf = (u8 *) buf;

	return dm_i2c_xfer(dev, &msg, 1);
}

static int atsha204a_recv_resp(struct udevice *dev,
			       struct atsha204a_resp *resp)
{
	int res;
	u16 resp_crc, computed_crc;
	u8 *p = (u8 *) resp;

	res = atsha204a_recv(dev, p, 4);
	if (res)
		return res;

	if (resp->length > 4) {
		if (resp->length > sizeof(*resp))
			return -EMSGSIZE;

		res = atsha204a_recv(dev, p + 4, resp->length - 4);
		if (res)
			return res;
	}

	resp_crc = (u16) p[resp->length - 2]
		   | (((u16) p[resp->length - 1]) << 8);
	computed_crc = atsha204a_crc16(p, resp->length - 2);

	if (resp_crc != computed_crc) {
		debug("Invalid checksum in ATSHA204A response\n");
		return -EBADMSG;
	}

	return 0;
}

int atsha204a_wakeup(struct udevice *dev)
{
	u8 req[4];
	struct atsha204a_resp resp;
	int try, res;

	debug("Waking up ATSHA204A\n");

	for (try = 1; try <= 10; ++try) {
		debug("Try %i... ", try);

		/*
		 * The device ignores any levels or transitions on the SCL pin
		 * when the device is idle, asleep or during waking up.
		 * Don't check for error when waking up the device.
		 */
		memset(req, 0, 4);
		atsha204a_send(dev, req, 4);

		udelay(ATSHA204A_TWLO_US + ATSHA204A_TWHI_US);

		res = atsha204a_recv_resp(dev, &resp);
		if (res) {
			debug("failed on receiving response, ending\n");
			return res;
		}

		if (resp.code != ATSHA204A_STATUS_AFTER_WAKE) {
			debug ("failed (responce code = %02x), ending\n",
			       resp.code);
			return -EBADMSG;
		}

		debug("success\n");
		return 0;
	}

	return -ETIMEDOUT;
}

int atsha204a_idle(struct udevice *dev)
{
	int res;
	u8 req = ATSHA204A_FUNC_IDLE;

	res = atsha204a_send(dev, &req, 1);
	if (res)
		debug("Failed putting ATSHA204A idle\n");
	return res;
}

int atsha204a_sleep(struct udevice *dev)
{
	int res;
	u8 req = ATSHA204A_FUNC_IDLE;

	res = atsha204a_send(dev, &req, 1);
	if (res)
		debug("Failed putting ATSHA204A to sleep\n");
	return res;
}

static int atsha204a_transaction(struct udevice *dev, struct atsha204a_req *req,
				struct atsha204a_resp *resp)
{
	int res, timeout = ATSHA204A_TRANSACTION_TIMEOUT;

	res = atsha204a_send(dev, (u8 *) req, req->length + 1);
	if (res) {
		debug("ATSHA204A transaction send failed\n");
		return -EBUSY;
	}

	do {
		udelay(ATSHA204A_EXECTIME);
		res = atsha204a_recv_resp(dev, resp);
		if (!res || res == -EMSGSIZE || res == -EBADMSG)
			break;

		debug("ATSHA204A transaction polling for response "
		      "(timeout = %d)\n", timeout);

		timeout -= ATSHA204A_EXECTIME;
	} while (timeout > 0);

	if (timeout <= 0) {
		debug("ATSHA204A transaction timed out\n");
		return -ETIMEDOUT;
	}

	return res;
}

static void atsha204a_req_crc32(struct atsha204a_req *req)
{
	u8 *p = (u8 *) req;
	u16 computed_crc;
	u16 *crc_ptr = (u16 *) &p[req->length - 1];

	/* The buffer to crc16 starts at byte 1, not 0 */
	computed_crc = atsha204a_crc16(p + 1, req->length - 2);

	*crc_ptr = cpu_to_le16(computed_crc);
}

int atsha204a_read(struct udevice *dev, enum atsha204a_zone zone, bool read32,
		  u16 addr, u8 *buffer)
{
	int res, retry = ATSHA204A_TRANSACTION_RETRY;
	struct atsha204a_req req;
	struct atsha204a_resp resp;

	req.function = ATSHA204A_FUNC_COMMAND;
	req.length = 7;
	req.command = ATSHA204A_CMD_READ;

	req.param1 = (u8) zone;
	if (read32)
		req.param1 |= 0x80;

	req.param2 = cpu_to_le16(addr);

	atsha204a_req_crc32(&req);

	do {
		res = atsha204a_transaction(dev, &req, &resp);
		if (!res)
			break;

		debug("ATSHA204A read retry (%d)\n", retry);
		retry--;
		atsha204a_wakeup(dev);
	} while (retry >= 0);

	if (res) {
		debug("ATSHA204A read failed\n");
		return res;
	}

	if (resp.length != (read32 ? 32 : 4) + 3) {
		debug("ATSHA204A read bad response length (%d)\n",
		      resp.length);
		return -EBADMSG;
	}

	memcpy(buffer, ((u8 *) &resp) + 1, read32 ? 32 : 4);

	return 0;
}

int atsha204a_get_random(struct udevice *dev, u8 *buffer, size_t max)
{
	int res;
	struct atsha204a_req req;
	struct atsha204a_resp resp;

	req.function = ATSHA204A_FUNC_COMMAND;
	req.length = 7;
	req.command = ATSHA204A_CMD_RANDOM;

	req.param1 = 1;
	req.param2 = 0;

	/* We do not have to compute the checksum dynamically */
	req.data[0] = 0x27;
	req.data[1] = 0x47;

	res = atsha204a_transaction(dev, &req, &resp);
	if (res) {
		debug("ATSHA204A random transaction failed\n");
		return res;
	}

	memcpy(buffer, ((u8 *) &resp) + 1, max >= 32 ? 32 : max);
	return 0;
}

static int atsha204a_of_to_plat(struct udevice *dev)
{
	fdt_addr_t *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE) {
		debug("Can't get ATSHA204A I2C base address\n");
		return -ENXIO;
	}

	*priv = addr;
	return 0;
}

static const struct udevice_id atsha204a_ids[] = {
	{ .compatible = "atmel,atsha204" },
	{ .compatible = "atmel,atsha204a" },
	{ }
};

U_BOOT_DRIVER(atsha204) = {
	.name			= "atsha204",
	.id			= UCLASS_MISC,
	.of_match		= atsha204a_ids,
	.of_to_plat	= atsha204a_of_to_plat,
	.priv_auto	= sizeof(fdt_addr_t),
};
