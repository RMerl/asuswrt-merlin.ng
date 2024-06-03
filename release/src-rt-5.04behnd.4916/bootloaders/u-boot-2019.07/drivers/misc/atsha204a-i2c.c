/*
 * I2C Driver for Atmel ATSHA204 over I2C
 *
 * Copyright (C) 2014 Josh Datko, Cryptotronix, jbd@cryptotronix.com
 * 		 2016 Tomas Hlavacek, CZ.NIC, tmshlvck@gmail.com
 * 		 2017 Marek Behun, CZ.NIC, marek.behun@nic.cz
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

#define ATSHA204A_TWLO			60
#define ATSHA204A_TRANSACTION_TIMEOUT	100000
#define ATSHA204A_TRANSACTION_RETRY	5
#define ATSHA204A_EXECTIME		5000

DECLARE_GLOBAL_DATA_PTR;

/*
 * The ATSHA204A uses an (to me) unknown CRC-16 algorithm.
 * The Reveng CRC-16 catalogue does not contain it.
 *
 * Because in Atmel's documentation only a primitive implementation
 * can be found, I have implemented this one with lookup table.
 */

/*
 * This is the code that computes the table below:
 *
 * int i, j;
 * for (i = 0; i < 256; ++i) {
 * 	u8 c = 0;
 * 	for (j = 0; j < 8; ++j) {
 * 		c = (c << 1) | ((i >> j) & 1);
 * 	}
 * 	bitreverse_table[i] = c;
 * }
 */

static u8 const bitreverse_table[256] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};

/*
 * This is the code that computes the table below:
 *
 * int i, j;
 * for (i = 0; i < 256; ++i) {
 * 	u16 c = i << 8;
 * 	for (j = 0; j < 8; ++j) {
 * 		int b = c >> 15;
 * 		c <<= 1;
 * 		if (b)
 * 			c ^= 0x8005;
 * 	}
 * 	crc16_table[i] = c;
 * }
 */
static u16 const crc16_table[256] = {
	0x0000, 0x8005, 0x800f, 0x000a, 0x801b, 0x001e, 0x0014, 0x8011,
	0x8033, 0x0036, 0x003c, 0x8039, 0x0028, 0x802d, 0x8027, 0x0022,
	0x8063, 0x0066, 0x006c, 0x8069, 0x0078, 0x807d, 0x8077, 0x0072,
	0x0050, 0x8055, 0x805f, 0x005a, 0x804b, 0x004e, 0x0044, 0x8041,
	0x80c3, 0x00c6, 0x00cc, 0x80c9, 0x00d8, 0x80dd, 0x80d7, 0x00d2,
	0x00f0, 0x80f5, 0x80ff, 0x00fa, 0x80eb, 0x00ee, 0x00e4, 0x80e1,
	0x00a0, 0x80a5, 0x80af, 0x00aa, 0x80bb, 0x00be, 0x00b4, 0x80b1,
	0x8093, 0x0096, 0x009c, 0x8099, 0x0088, 0x808d, 0x8087, 0x0082,
	0x8183, 0x0186, 0x018c, 0x8189, 0x0198, 0x819d, 0x8197, 0x0192,
	0x01b0, 0x81b5, 0x81bf, 0x01ba, 0x81ab, 0x01ae, 0x01a4, 0x81a1,
	0x01e0, 0x81e5, 0x81ef, 0x01ea, 0x81fb, 0x01fe, 0x01f4, 0x81f1,
	0x81d3, 0x01d6, 0x01dc, 0x81d9, 0x01c8, 0x81cd, 0x81c7, 0x01c2,
	0x0140, 0x8145, 0x814f, 0x014a, 0x815b, 0x015e, 0x0154, 0x8151,
	0x8173, 0x0176, 0x017c, 0x8179, 0x0168, 0x816d, 0x8167, 0x0162,
	0x8123, 0x0126, 0x012c, 0x8129, 0x0138, 0x813d, 0x8137, 0x0132,
	0x0110, 0x8115, 0x811f, 0x011a, 0x810b, 0x010e, 0x0104, 0x8101,
	0x8303, 0x0306, 0x030c, 0x8309, 0x0318, 0x831d, 0x8317, 0x0312,
	0x0330, 0x8335, 0x833f, 0x033a, 0x832b, 0x032e, 0x0324, 0x8321,
	0x0360, 0x8365, 0x836f, 0x036a, 0x837b, 0x037e, 0x0374, 0x8371,
	0x8353, 0x0356, 0x035c, 0x8359, 0x0348, 0x834d, 0x8347, 0x0342,
	0x03c0, 0x83c5, 0x83cf, 0x03ca, 0x83db, 0x03de, 0x03d4, 0x83d1,
	0x83f3, 0x03f6, 0x03fc, 0x83f9, 0x03e8, 0x83ed, 0x83e7, 0x03e2,
	0x83a3, 0x03a6, 0x03ac, 0x83a9, 0x03b8, 0x83bd, 0x83b7, 0x03b2,
	0x0390, 0x8395, 0x839f, 0x039a, 0x838b, 0x038e, 0x0384, 0x8381,
	0x0280, 0x8285, 0x828f, 0x028a, 0x829b, 0x029e, 0x0294, 0x8291,
	0x82b3, 0x02b6, 0x02bc, 0x82b9, 0x02a8, 0x82ad, 0x82a7, 0x02a2,
	0x82e3, 0x02e6, 0x02ec, 0x82e9, 0x02f8, 0x82fd, 0x82f7, 0x02f2,
	0x02d0, 0x82d5, 0x82df, 0x02da, 0x82cb, 0x02ce, 0x02c4, 0x82c1,
	0x8243, 0x0246, 0x024c, 0x8249, 0x0258, 0x825d, 0x8257, 0x0252,
	0x0270, 0x8275, 0x827f, 0x027a, 0x826b, 0x026e, 0x0264, 0x8261,
	0x0220, 0x8225, 0x822f, 0x022a, 0x823b, 0x023e, 0x0234, 0x8231,
	0x8213, 0x0216, 0x021c, 0x8219, 0x0208, 0x820d, 0x8207, 0x0202,
};

static inline u16 crc16_byte(u16 crc, const u8 data)
{
	u16 t = crc16_table[((crc >> 8) ^ bitreverse_table[data]) & 0xff];
	return ((crc << 8) ^ t);
}

static u16 atsha204a_crc16(const u8 *buffer, size_t len)
{
	u16 crc = 0;

	while (len--)
		crc = crc16_byte(crc, *buffer++);

	return cpu_to_le16(crc);
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

		memset(req, 0, 4);
		res = atsha204a_send(dev, req, 4);
		if (res) {
			debug("failed on I2C send, trying again\n");
			continue;
		}

		udelay(ATSHA204A_TWLO);

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
		break;
	}

	return 0;
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
		res = atsha204a_recv_resp(dev, resp);
		if (!res || res == -EMSGSIZE || res == -EBADMSG)
			break;

		debug("ATSHA204A transaction polling for response "
		      "(timeout = %d)\n", timeout);

		udelay(ATSHA204A_EXECTIME);
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

static int atsha204a_ofdata_to_platdata(struct udevice *dev)
{
	fdt_addr_t *priv = dev_get_priv(dev);
	fdt_addr_t addr;

	addr = fdtdec_get_addr(gd->fdt_blob, dev_of_offset(dev), "reg");
	if (addr == FDT_ADDR_T_NONE) {
		debug("Can't get ATSHA204A I2C base address\n");
		return -ENXIO;
	}

	*priv = addr;
	return 0;
}

static const struct udevice_id atsha204a_ids[] = {
	{ .compatible = "atmel,atsha204a" },
	{ }
};

U_BOOT_DRIVER(atsha204) = {
	.name			= "atsha204",
	.id			= UCLASS_MISC,
	.of_match		= atsha204a_ids,
	.ofdata_to_platdata	= atsha204a_ofdata_to_platdata,
	.priv_auto_alloc_size	= sizeof(fdt_addr_t),
};
