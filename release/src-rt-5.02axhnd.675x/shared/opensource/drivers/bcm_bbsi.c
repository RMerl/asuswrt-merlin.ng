/*
 * <:copyright-BRCM:2017:DUAL/GPL:standard
 * 
 *    Copyright (c) 2017 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

#include "boardparms.h"
#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bcm_map.h"
#define printk printf
#endif
#include "bcm_map_part.h"
#ifndef _CFE_
#include <linux/module.h>
#endif
#include "bcm_gpio.h"
#include "bcm_bbsi.h"

static int
spi_write(struct bbsi_t *bbsi, uint8 *buf, uint32 buflen)
{
	uint32 spi_clk = 1 << bbsi->spi_clk;
	uint32 spi_cs = 1 << bbsi->spi_cs;
	uint32 spi_mosi = 1 << bbsi->spi_mosi;
	uint32 i, j, output;
	uint8 data;

	/* set CS low (activate) */
	bcm_multi_gpio_set_data(spi_cs, 0);

	for (i = 0; i < buflen; i++) {
		data = buf[i];

		/* send data from MSB to LSB */
		for (j = 0; j < 8; j++) {
			output = (data & 0x80) ? spi_mosi : 0;

			/* clock falling and output data (mode 3) */
			bcm_multi_gpio_set_data(spi_clk | spi_mosi, 0 | output);
			/* clock rising */
			bcm_multi_gpio_set_data(spi_clk, spi_clk);
			/* shift to next bit to be send */
			data <<= 1;
		}
	}

	/* Set CS high (deactivate) */
	bcm_multi_gpio_set_data(spi_cs, spi_cs);

	return 0;
}

static int
spi_write_then_read(struct bbsi_t *bbsi, uint8 *buf, uint32 buflen, uint8 *rbuf, uint32 rbuflen)
{
	uint32 spi_clk = 1 << bbsi->spi_clk;
	uint32 spi_cs = 1 << bbsi->spi_cs;
	uint32 spi_mosi = 1 << bbsi->spi_mosi;
	uint32 i, j, output;
	uint8 data;

	/* set CS low (activate) */
	bcm_multi_gpio_set_data(spi_cs, 0);

	for (i = 0; i < buflen; i++) {
		data = buf[i];

		/* send data from MSB to LSB */
		for (j = 0; j < 8; j++) {
			output = (data & 0x80) ? spi_mosi : 0;

			/* clock falling and output data (mode 3) */
			bcm_multi_gpio_set_data((spi_clk | spi_mosi), output);
			/* clock rising */
			bcm_multi_gpio_set_data(spi_clk, spi_clk);
			/* shift to next bit to be send */
			data <<= 1;
		}
	}

	for (i = 0; i < rbuflen; i++) {
		/* receive data from MSB to LSB */
		for (j = 0, data = 0; j < 8; j++) {
			/* shift to next bit */
			data <<= 1;

			/* clock falling */
			bcm_multi_gpio_set_data(spi_clk, 0);
			/* sample input data */
			data |= bcm_gpio_get_data(bbsi->spi_miso);
			/* clock rising */
			bcm_multi_gpio_set_data(spi_clk, spi_clk);
		}

		rbuf[i] = data;
	}

	/* Set CS high (deactivate) */
	bcm_multi_gpio_set_data(spi_cs, spi_cs);

	return 0;
}

/* Poll for command completion. Returns zero when complete. */
int is_bbsi_done(struct bbsi_t *bbsi)
{
	uint8 read_status[2] = {BBSI_CMD_READ, BBSI_STATUS_REG_ADDR};
	uint8 read_rx;
	int status, i, ret = -1;

	for (i = 0; i < BSSI_STATUS_RETRY; i++) {
		/* issue command to spi */
		status = spi_write_then_read(bbsi, read_status, 2, &read_rx, 1);

		/* status return from spi operation */
		if (status != 0) {
			printk("%s: spi returned error\n", __func__);
			break;
		}

		/* transaction error reported by BBSI slave device */
		if (read_rx & BBSI_STATUS_FAILED) {
			printk("%s: BBSI transaction error, status=0x%x\n", __func__, read_rx);
			break;
		} else if ((read_rx & (1 << BBSI_STATUS_BUSY_SHIFT)) == 0) {
			/* transaction completed */
			ret = 0;
			break;
		}
	}

	return ret;
}

int bbsi_read(struct bbsi_t *bbsi, uint32_t addr, uint32_t readlen, uint32_t *data)
{
	uint8 buf[7] = {0}, rbuf[4] = {0};
	int status, ret = -1;

	if (readlen > 4) {
		printk("%s: incorrect read length: %d\n", __func__, readlen);
		return ret;
	}

	buf[0] = BBSI_CMD_WRITE;
	buf[1] = BBSI_CONFIG_REG_ADDR;
	buf[2] = ((4 - readlen) << BBSI_CONFIG_XFER_MODE_SHIFT) |
		(BBSI_CONFIG_READ_RBUS_MASK << BBSI_CONFIG_READ_RBUS_SHIFT);
	buf[3] = (addr >> 24) & 0xFF;	/* Assuming MSB bytes are always sent first */
	buf[4] = (addr >> 16) & 0xFF;
	buf[5] = (addr >> 8) & 0xFF;
	buf[6] = (addr >> 0) & 0xFF;

	status = spi_write(bbsi, buf, 7);

	if (status != 0) {
		printk("%s: Spi returned error\n", __func__);
		return ret;
	}

	if (is_bbsi_done(bbsi) != 0) {
		printk("%s: read to addr:0x%08x failed\n", __func__, addr);
		return ret;
	}

	buf[0] = BBSI_CMD_READ;
	buf[1] = BBSI_DATA0_REG_ADDR;

	status = spi_write_then_read(bbsi, buf, 2, rbuf, 4);

	if (status != 0) {
		printk("%s: spi returned error\n", __func__);
		return ret;
	}

	*data = (rbuf[0] << 24) | (rbuf[1] << 16) | (rbuf[2] << 8) | rbuf[3];
	ret = 0;

	return ret;
}

int bbsi_write(struct bbsi_t *bbsi, uint32 addr, uint32 writelen, uint32 data)
{
	uint8 buf[12];
	int status, ret = -1;

	if (writelen > 4) {
		printk("%s: incorrect write length: %d\n", __func__, writelen);
		return ret;
	}

	data <<= (8 * (4 - writelen));

	buf[0] = BBSI_CMD_WRITE;
	buf[1] = BBSI_CONFIG_REG_ADDR;
	buf[2] = (4 - writelen) << BBSI_CONFIG_XFER_MODE_SHIFT;
	buf[3] = (addr >> 24) & 0xFF;	/* Assuming MSB bytes are always sent first */
	buf[4] = (addr >> 16) & 0xFF;
	buf[5] = (addr >> 8) & 0xFF;
	buf[6] = (addr >> 0) & 0xFF;
	buf[7] = (data >> 24) & 0xFF;
	buf[8] = (data >> 16) & 0xFF;
	buf[9] = (data >> 8) & 0xFF;
	buf[10] = (data >> 0) & 0xFF;

	status = spi_write(bbsi, buf, 11);

	if (status != 0) {
		printk("%s: Spi returned error\n", __func__);
		return ret;
	}

	if (is_bbsi_done(bbsi) != 0) {
		printk("%s: write to addr:0x%08x failed\n", __func__, addr);
		return ret;
	}

	ret = 0;
	return ret;
}


void bbsi_init(struct bbsi_t *bbsi)
{
    uint32_t mask, val;

    /* BCM6802 need spi on mode3.  set clk, cs to 1, mosi to 0 */
    mask = (1 << bbsi->spi_clk) | (1 << bbsi->spi_cs) | (1 << bbsi->spi_mosi);
    val = (1 << bbsi->spi_clk) | (1 << bbsi->spi_cs);
    bcm_multi_gpio_set_data(mask, val);

    /* Setup GPIO output */
    mask = (1 << bbsi->spi_clk) | (1 << bbsi->spi_mosi) | (1 << bbsi->spi_cs);
    val = (1 << bbsi->spi_clk) | (1 << bbsi->spi_mosi) | (1 << bbsi->spi_cs);
    bcm_multi_gpio_set_dir(mask, val);
}

#ifndef _CFE_
EXPORT_SYMBOL(bbsi_init);
EXPORT_SYMBOL(bbsi_write);
EXPORT_SYMBOL(bbsi_read);
EXPORT_SYMBOL(is_bbsi_done);
#endif

