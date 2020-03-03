/*
 * This file is part of wl1271
 *
 * Copyright (C) 2008-2009 Nokia Corporation
 *
 * Contact: Luciano Coelho <luciano.coelho@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/swab.h>
#include <linux/crc7.h>
#include <linux/spi/spi.h>
#include <linux/wl12xx.h>
#include <linux/platform_device.h>

#include "wlcore.h"
#include "wl12xx_80211.h"
#include "io.h"

#define WSPI_CMD_READ                 0x40000000
#define WSPI_CMD_WRITE                0x00000000
#define WSPI_CMD_FIXED                0x20000000
#define WSPI_CMD_BYTE_LENGTH          0x1FFE0000
#define WSPI_CMD_BYTE_LENGTH_OFFSET   17
#define WSPI_CMD_BYTE_ADDR            0x0001FFFF

#define WSPI_INIT_CMD_CRC_LEN       5

#define WSPI_INIT_CMD_START         0x00
#define WSPI_INIT_CMD_TX            0x40
/* the extra bypass bit is sampled by the TNET as '1' */
#define WSPI_INIT_CMD_BYPASS_BIT    0x80
#define WSPI_INIT_CMD_FIXEDBUSY_LEN 0x07
#define WSPI_INIT_CMD_EN_FIXEDBUSY  0x80
#define WSPI_INIT_CMD_DIS_FIXEDBUSY 0x00
#define WSPI_INIT_CMD_IOD           0x40
#define WSPI_INIT_CMD_IP            0x20
#define WSPI_INIT_CMD_CS            0x10
#define WSPI_INIT_CMD_WS            0x08
#define WSPI_INIT_CMD_WSPI          0x01
#define WSPI_INIT_CMD_END           0x01

#define WSPI_INIT_CMD_LEN           8

#define HW_ACCESS_WSPI_FIXED_BUSY_LEN \
		((WL1271_BUSY_WORD_LEN - 4) / sizeof(u32))
#define HW_ACCESS_WSPI_INIT_CMD_MASK  0

/* HW limitation: maximum possible chunk size is 4095 bytes */
#define WSPI_MAX_CHUNK_SIZE    4092

/*
 * only support SPI for 12xx - this code should be reworked when 18xx
 * support is introduced
 */
#define SPI_AGGR_BUFFER_SIZE (4 * PAGE_SIZE)

/* Maximum number of SPI write chunks */
#define WSPI_MAX_NUM_OF_CHUNKS \
	((SPI_AGGR_BUFFER_SIZE / WSPI_MAX_CHUNK_SIZE) + 1)


struct wl12xx_spi_glue {
	struct device *dev;
	struct platform_device *core;
};

static void wl12xx_spi_reset(struct device *child)
{
	struct wl12xx_spi_glue *glue = dev_get_drvdata(child->parent);
	u8 *cmd;
	struct spi_transfer t;
	struct spi_message m;

	cmd = kzalloc(WSPI_INIT_CMD_LEN, GFP_KERNEL);
	if (!cmd) {
		dev_err(child->parent,
			"could not allocate cmd for spi reset\n");
		return;
	}

	memset(&t, 0, sizeof(t));
	spi_message_init(&m);

	memset(cmd, 0xff, WSPI_INIT_CMD_LEN);

	t.tx_buf = cmd;
	t.len = WSPI_INIT_CMD_LEN;
	spi_message_add_tail(&t, &m);

	spi_sync(to_spi_device(glue->dev), &m);

	kfree(cmd);
}

static void wl12xx_spi_init(struct device *child)
{
	struct wl12xx_spi_glue *glue = dev_get_drvdata(child->parent);
	struct spi_transfer t;
	struct spi_message m;
	u8 *cmd = kzalloc(WSPI_INIT_CMD_LEN, GFP_KERNEL);

	if (!cmd) {
		dev_err(child->parent,
			"could not allocate cmd for spi init\n");
		return;
	}

	memset(&t, 0, sizeof(t));
	spi_message_init(&m);

	/*
	 * Set WSPI_INIT_COMMAND
	 * the data is being send from the MSB to LSB
	 */
	cmd[0] = 0xff;
	cmd[1] = 0xff;
	cmd[2] = WSPI_INIT_CMD_START | WSPI_INIT_CMD_TX;
	cmd[3] = 0;
	cmd[4] = 0;
	cmd[5] = HW_ACCESS_WSPI_INIT_CMD_MASK << 3;
	cmd[5] |= HW_ACCESS_WSPI_FIXED_BUSY_LEN & WSPI_INIT_CMD_FIXEDBUSY_LEN;

	cmd[6] = WSPI_INIT_CMD_IOD | WSPI_INIT_CMD_IP | WSPI_INIT_CMD_CS
		| WSPI_INIT_CMD_WSPI | WSPI_INIT_CMD_WS;

	if (HW_ACCESS_WSPI_FIXED_BUSY_LEN == 0)
		cmd[6] |= WSPI_INIT_CMD_DIS_FIXEDBUSY;
	else
		cmd[6] |= WSPI_INIT_CMD_EN_FIXEDBUSY;

	cmd[7] = crc7_be(0, cmd+2, WSPI_INIT_CMD_CRC_LEN) | WSPI_INIT_CMD_END;
	/*
	 * The above is the logical order; it must actually be stored
	 * in the buffer byte-swapped.
	 */
	__swab32s((u32 *)cmd);
	__swab32s((u32 *)cmd+1);

	t.tx_buf = cmd;
	t.len = WSPI_INIT_CMD_LEN;
	spi_message_add_tail(&t, &m);

	spi_sync(to_spi_device(glue->dev), &m);
	kfree(cmd);
}

#define WL1271_BUSY_WORD_TIMEOUT 1000

static int wl12xx_spi_read_busy(struct device *child)
{
	struct wl12xx_spi_glue *glue = dev_get_drvdata(child->parent);
	struct wl1271 *wl = dev_get_drvdata(child);
	struct spi_transfer t[1];
	struct spi_message m;
	u32 *busy_buf;
	int num_busy_bytes = 0;

	/*
	 * Read further busy words from SPI until a non-busy word is
	 * encountered, then read the data itself into the buffer.
	 */

	num_busy_bytes = WL1271_BUSY_WORD_TIMEOUT;
	busy_buf = wl->buffer_busyword;
	while (num_busy_bytes) {
		num_busy_bytes--;
		spi_message_init(&m);
		memset(t, 0, sizeof(t));
		t[0].rx_buf = busy_buf;
		t[0].len = sizeof(u32);
		t[0].cs_change = true;
		spi_message_add_tail(&t[0], &m);
		spi_sync(to_spi_device(glue->dev), &m);

		if (*busy_buf & 0x1)
			return 0;
	}

	/* The SPI bus is unresponsive, the read failed. */
	dev_err(child->parent, "SPI read busy-word timeout!\n");
	return -ETIMEDOUT;
}

static int __must_check wl12xx_spi_raw_read(struct device *child, int addr,
					    void *buf, size_t len, bool fixed)
{
	struct wl12xx_spi_glue *glue = dev_get_drvdata(child->parent);
	struct wl1271 *wl = dev_get_drvdata(child);
	struct spi_transfer t[2];
	struct spi_message m;
	u32 *busy_buf;
	u32 *cmd;
	u32 chunk_len;

	while (len > 0) {
		chunk_len = min_t(size_t, WSPI_MAX_CHUNK_SIZE, len);

		cmd = &wl->buffer_cmd;
		busy_buf = wl->buffer_busyword;

		*cmd = 0;
		*cmd |= WSPI_CMD_READ;
		*cmd |= (chunk_len << WSPI_CMD_BYTE_LENGTH_OFFSET) &
			WSPI_CMD_BYTE_LENGTH;
		*cmd |= addr & WSPI_CMD_BYTE_ADDR;

		if (fixed)
			*cmd |= WSPI_CMD_FIXED;

		spi_message_init(&m);
		memset(t, 0, sizeof(t));

		t[0].tx_buf = cmd;
		t[0].len = 4;
		t[0].cs_change = true;
		spi_message_add_tail(&t[0], &m);

		/* Busy and non busy words read */
		t[1].rx_buf = busy_buf;
		t[1].len = WL1271_BUSY_WORD_LEN;
		t[1].cs_change = true;
		spi_message_add_tail(&t[1], &m);

		spi_sync(to_spi_device(glue->dev), &m);

		if (!(busy_buf[WL1271_BUSY_WORD_CNT - 1] & 0x1) &&
		    wl12xx_spi_read_busy(child)) {
			memset(buf, 0, chunk_len);
			return 0;
		}

		spi_message_init(&m);
		memset(t, 0, sizeof(t));

		t[0].rx_buf = buf;
		t[0].len = chunk_len;
		t[0].cs_change = true;
		spi_message_add_tail(&t[0], &m);

		spi_sync(to_spi_device(glue->dev), &m);

		if (!fixed)
			addr += chunk_len;
		buf += chunk_len;
		len -= chunk_len;
	}

	return 0;
}

static int __must_check wl12xx_spi_raw_write(struct device *child, int addr,
					     void *buf, size_t len, bool fixed)
{
	struct wl12xx_spi_glue *glue = dev_get_drvdata(child->parent);
	/* SPI write buffers - 2 for each chunk */
	struct spi_transfer t[2 * WSPI_MAX_NUM_OF_CHUNKS];
	struct spi_message m;
	u32 commands[WSPI_MAX_NUM_OF_CHUNKS]; /* 1 command per chunk */
	u32 *cmd;
	u32 chunk_len;
	int i;

	WARN_ON(len > SPI_AGGR_BUFFER_SIZE);

	spi_message_init(&m);
	memset(t, 0, sizeof(t));

	cmd = &commands[0];
	i = 0;
	while (len > 0) {
		chunk_len = min_t(size_t, WSPI_MAX_CHUNK_SIZE, len);

		*cmd = 0;
		*cmd |= WSPI_CMD_WRITE;
		*cmd |= (chunk_len << WSPI_CMD_BYTE_LENGTH_OFFSET) &
			WSPI_CMD_BYTE_LENGTH;
		*cmd |= addr & WSPI_CMD_BYTE_ADDR;

		if (fixed)
			*cmd |= WSPI_CMD_FIXED;

		t[i].tx_buf = cmd;
		t[i].len = sizeof(*cmd);
		spi_message_add_tail(&t[i++], &m);

		t[i].tx_buf = buf;
		t[i].len = chunk_len;
		spi_message_add_tail(&t[i++], &m);

		if (!fixed)
			addr += chunk_len;
		buf += chunk_len;
		len -= chunk_len;
		cmd++;
	}

	spi_sync(to_spi_device(glue->dev), &m);

	return 0;
}

static struct wl1271_if_operations spi_ops = {
	.read		= wl12xx_spi_raw_read,
	.write		= wl12xx_spi_raw_write,
	.reset		= wl12xx_spi_reset,
	.init		= wl12xx_spi_init,
	.set_block_size = NULL,
};

static int wl1271_probe(struct spi_device *spi)
{
	struct wl12xx_spi_glue *glue;
	struct wlcore_platdev_data pdev_data;
	struct resource res[1];
	int ret;

	memset(&pdev_data, 0x00, sizeof(pdev_data));

	/* TODO: add DT parsing when needed */

	pdev_data.if_ops = &spi_ops;

	glue = devm_kzalloc(&spi->dev, sizeof(*glue), GFP_KERNEL);
	if (!glue) {
		dev_err(&spi->dev, "can't allocate glue\n");
		return -ENOMEM;
	}

	glue->dev = &spi->dev;

	spi_set_drvdata(spi, glue);

	/* This is the only SPI value that we need to set here, the rest
	 * comes from the board-peripherals file */
	spi->bits_per_word = 32;

	ret = spi_setup(spi);
	if (ret < 0) {
		dev_err(glue->dev, "spi_setup failed\n");
		return ret;
	}

	glue->core = platform_device_alloc("wl12xx", PLATFORM_DEVID_AUTO);
	if (!glue->core) {
		dev_err(glue->dev, "can't allocate platform_device\n");
		return -ENOMEM;
	}

	glue->core->dev.parent = &spi->dev;

	memset(res, 0x00, sizeof(res));

	res[0].start = spi->irq;
	res[0].flags = IORESOURCE_IRQ;
	res[0].name = "irq";

	ret = platform_device_add_resources(glue->core, res, ARRAY_SIZE(res));
	if (ret) {
		dev_err(glue->dev, "can't add resources\n");
		goto out_dev_put;
	}

	ret = platform_device_add_data(glue->core, &pdev_data,
				       sizeof(pdev_data));
	if (ret) {
		dev_err(glue->dev, "can't add platform data\n");
		goto out_dev_put;
	}

	ret = platform_device_add(glue->core);
	if (ret) {
		dev_err(glue->dev, "can't register platform device\n");
		goto out_dev_put;
	}

	return 0;

out_dev_put:
	platform_device_put(glue->core);
	return ret;
}

static int wl1271_remove(struct spi_device *spi)
{
	struct wl12xx_spi_glue *glue = spi_get_drvdata(spi);

	platform_device_unregister(glue->core);

	return 0;
}


static struct spi_driver wl1271_spi_driver = {
	.driver = {
		.name		= "wl1271_spi",
		.owner		= THIS_MODULE,
	},

	.probe		= wl1271_probe,
	.remove		= wl1271_remove,
};

module_spi_driver(wl1271_spi_driver);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Luciano Coelho <coelho@ti.com>");
MODULE_AUTHOR("Juuso Oikarinen <juuso.oikarinen@nokia.com>");
MODULE_ALIAS("spi:wl1271");
