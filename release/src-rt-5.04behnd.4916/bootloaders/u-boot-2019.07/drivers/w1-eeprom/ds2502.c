// SPDX-License-Identifier: GPL-2.0+
/*
 * Driver for DS-2502 One wire "Add only Memory".
 *
 * The chip has 4 pages of 32 bytes.
 * In addition it has 8 out of band status bytes that are used, by software,
 * as page redirection bytes by an algorithm described in the data sheet.
 * This is useful since data cannot be erased once written but it can be
 * "patched" up to four times by switching pages.
 *
 * So, when a read request is entirely in the first page automatically
 * apply the page redirection bytes (which allows the device to be seen as
 * a 32 byte PROM, writable 4 times).
 *
 * If the read request is outside of or larger than the first page then read
 * the raw data (which allows the device to be seen as a 128 byte PROM,
 * writable once).
 *
 * Copyright (c) 2018 Flowbird
 * Martin Fuzzey <martin.fuzzey@flowbird.group>
 */

#include <common.h>
#include <dm.h>
#include <linux/err.h>
#include <w1-eeprom.h>
#include <w1.h>

#define DS2502_PAGE_SIZE	32
#define DS2502_PAGE_COUNT	4
#define DS2502_STATUS_SIZE	8

#define DS2502_CMD_READ_STATUS	0xAA
#define DS2502_CMD_READ_GEN_CRC	0xC3

/* u-boot crc8() is CCITT CRC8, we need x^8 + x^5 + x^4 + 1 LSB first */
static unsigned int ds2502_crc8(const u8 *buf, int len)
{
	static const u8 poly = 0x8C;  /* (1 + x^4 + x^5) + x^8 */
	u8 crc = 0;
	int i;

	for (i = 0; i < len; i++) {
		u8 data = buf[i];
		int j;

		for (j = 0; j < 8; j++) {
			u8 mix = (crc ^ data) & 1;

			crc >>= 1;
			if (mix)
				crc ^= poly;
			data >>= 1;
		}
	}
	return crc;
}

static int ds2502_read(struct udevice *dev, u8 cmd,
		       int bytes_in_page, int pos,
		       u8 *buf, int bytes_for_user)
{
	int retry;
	int ret = 0;

	for (retry = 0; retry < 3; retry++) {
		u8 pagebuf[DS2502_PAGE_SIZE + 1]; /* 1 byte for CRC8 */
		u8 crc;
		int i;

		ret = w1_reset_select(dev);
		if (ret)
			return ret;

		/* send read to end of page and generate CRC command */
		pagebuf[0] = cmd;
		pagebuf[1] = pos & 0xff;
		pagebuf[2] = pos >> 8;
		crc = ds2502_crc8(pagebuf, 3);
		for (i = 0; i < 3; i++)
			w1_write_byte(dev, pagebuf[i]);

		/* Check command CRC */
		ret = w1_read_byte(dev);
		if (ret < 0) {
			dev_dbg(dev, "Error %d reading command CRC\n", ret);
			continue;
		}

		if (ret != crc) {
			dev_dbg(dev,
				"bad CRC8 for cmd %02x got=%02X exp=%02X\n",
				cmd, ret, crc);
			ret = -EIO;
			continue;
		}

		/* read data and check CRC */
		ret = w1_read_buf(dev, pagebuf, bytes_in_page + 1);
		if (ret < 0) {
			dev_dbg(dev, "Error %d reading data\n", ret);
			continue;
		}

		crc = ds2502_crc8(pagebuf, bytes_in_page);
		if (crc == pagebuf[bytes_in_page]) {
			memcpy(buf, pagebuf, bytes_for_user);
			ret = 0;
			break;
		}
		dev_dbg(dev, "Bad CRC8 got=%02X exp=%02X pos=%04X\n",
			pagebuf[bytes_in_page], crc, pos);
		ret = -EIO;
	}

	return ret;
}

static inline int ds2502_read_status_bytes(struct udevice *dev, u8 *buf)
{
	return ds2502_read(dev, DS2502_CMD_READ_STATUS,
				DS2502_STATUS_SIZE, 0,
				buf, DS2502_STATUS_SIZE);
}

/*
 * Status bytes (from index 1) contain 1's complement page indirection
 * So for N writes:
 * N=1: ff ff ff ff ff ff ff 00
 * N=2: ff fe ff ff ff ff ff 00
 * N=3: ff fe fd ff ff ff ff 00
 * N=4: ff fe fd fc ff ff ff 00
 */
static int ds2502_indirect_page(struct udevice *dev, u8 *status, int page)
{
	int page_seen = 0;

	do {
		u8 sb = status[page + 1];

		if (sb == 0xff)
			break;

		page = ~sb & 0xff;

		if (page >= DS2502_PAGE_COUNT) {
			dev_err(dev,
				"Illegal page redirection status byte %02x\n",
				sb);
			return -EINVAL;
		}

		if (page_seen & (1 << page)) {
			dev_err(dev, "Infinite loop in page redirection\n");
			return -EINVAL;
		}

		page_seen |= (1 << page);
	} while (1);

	return page;
}

static int ds2502_read_buf(struct udevice *dev, unsigned int offset,
			   u8 *buf, unsigned int count)
{
	unsigned int min_page = offset / DS2502_PAGE_SIZE;
	unsigned int max_page = (offset + count - 1) / DS2502_PAGE_SIZE;
	int xfered = 0;
	u8 status_bytes[DS2502_STATUS_SIZE];
	int i;
	int ret;

	if (min_page >= DS2502_PAGE_COUNT || max_page >= DS2502_PAGE_COUNT)
		return -EINVAL;

	if (min_page == 0 && max_page == 0) {
		ret = ds2502_read_status_bytes(dev, status_bytes);
		if (ret)
			return ret;
	} else {
		/* Dummy one to one page redirection */
		memset(status_bytes, 0xff, sizeof(status_bytes));
	}

	for (i = min_page; i <= max_page; i++) {
		int page;
		int pos;
		int bytes_in_page;
		int bytes_for_user;

		page = ds2502_indirect_page(dev, status_bytes, i);
		if (page < 0)
			return page;
		dev_dbg(dev, "page logical %d => physical %d\n", i, page);

		pos = page * DS2502_PAGE_SIZE;
		if (i == min_page)
			pos += offset % DS2502_PAGE_SIZE;

		bytes_in_page = DS2502_PAGE_SIZE - (pos % DS2502_PAGE_SIZE);

		if (i == max_page)
			bytes_for_user = count - xfered;
		else
			bytes_for_user = bytes_in_page;

		ret = ds2502_read(dev, DS2502_CMD_READ_GEN_CRC,
				  bytes_in_page, pos,
				  &buf[xfered], bytes_for_user);
		if (ret < 0)
			return ret;

		xfered += bytes_for_user;
	}

	return 0;
}

static int ds2502_probe(struct udevice *dev)
{
	struct w1_device *w1;

	w1 = dev_get_parent_platdata(dev);
	w1->id = 0;
	return 0;
}

static const struct w1_eeprom_ops ds2502_ops = {
	.read_buf	= ds2502_read_buf,
};

static const struct udevice_id ds2502_id[] = {
	{ .compatible = "maxim,ds2502", .data = W1_FAMILY_DS2502 },
	{ },
};

U_BOOT_DRIVER(ds2502) = {
	.name		= "ds2502",
	.id		= UCLASS_W1_EEPROM,
	.of_match	= ds2502_id,
	.ops		= &ds2502_ops,
	.probe		= ds2502_probe,
};
