// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Guntermann & Drunck, GmbH
 *
 * Written by Dirk Eibach <dirk.eibach@gdsys.cc>
 */

#include <common.h>
#include <dm.h>
#include <tpm-v1.h>
#include <i2c.h>
#include <asm/unaligned.h>

#include "tpm_internal.h"

#define ATMEL_TPM_TIMEOUT_MS 5000 /* sufficient for anything but
				     generating/exporting keys */

/*
 * tpm_atmel_twi_open()
 *
 * Requests access to locality 0 for the caller. After all commands have been
 * completed the caller is supposed to call tis_close().
 *
 * Returns 0 on success, -1 on failure.
 */
static int tpm_atmel_twi_open(struct udevice *dev)
{
	return 0;
}

/*
 * tpm_atmel_twi_close()
 *
 * terminate the currect session with the TPM by releasing the locked
 * locality. Returns 0 on success of -1 on failure (in case lock
 * removal did not succeed).
 */
static int tpm_atmel_twi_close(struct udevice *dev)
{
	return 0;
}

/*
 * tpm_atmel_twi_get_desc()
 *
 * @dev:        Device to check
 * @buf:        Buffer to put the string
 * @size:       Maximum size of buffer
 * @return length of string, or -ENOSPC it no space
 */
static int tpm_atmel_twi_get_desc(struct udevice *dev, char *buf, int size)
{
	return 0;
}

/*
 * tpm_atmel_twi_xfer()
 *
 * Send the requested data to the TPM and then try to get its response
 *
 * @sendbuf - buffer of the data to send
 * @send_size size of the data to send
 * @recvbuf - memory to save the response to
 * @recv_len - pointer to the size of the response buffer
 *
 * Returns 0 on success (and places the number of response bytes at recv_len)
 * or -1 on failure.
 */
static int tpm_atmel_twi_xfer(struct udevice *dev,
			      const uint8_t *sendbuf, size_t send_size,
			      uint8_t *recvbuf, size_t *recv_len)
{
	int res;
	unsigned long start;

#ifdef DEBUG
	memset(recvbuf, 0xcc, *recv_len);
	printf("send to TPM (%d bytes, recv_len=%d):\n", send_size, *recv_len);
	print_buffer(0, (void *)sendbuf, 1, send_size, 0);
#endif

#ifndef CONFIG_DM_I2C
	res = i2c_write(0x29, 0, 0, (uchar *)sendbuf, send_size);
#else
	res = dm_i2c_write(dev, 0, sendbuf, send_size);
#endif
	if (res) {
		printf("i2c_write returned %d\n", res);
		return -1;
	}

	start = get_timer(0);
#ifndef CONFIG_DM_I2C
	while ((res = i2c_read(0x29, 0, 0, recvbuf, 10)))
#else
	while ((res = dm_i2c_read(dev, 0, recvbuf, 10)))
#endif
	{
		/* TODO Use TIS_TIMEOUT from tpm_tis_infineon.h */
		if (get_timer(start) > ATMEL_TPM_TIMEOUT_MS) {
			puts("tpm timed out\n");
			return -1;
		}
		udelay(100);
	}
	if (!res) {
		unsigned int hdr_recv_len;
		hdr_recv_len = get_unaligned_be32(recvbuf + 2);
		if (hdr_recv_len < 10) {
			puts("tpm response header too small\n");
			return -1;
		} else if (hdr_recv_len > *recv_len) {
			puts("tpm response length is bigger than receive buffer\n");
			return -1;
		} else {
			*recv_len = hdr_recv_len;
#ifndef CONFIG_DM_I2C
			res = i2c_read(0x29, 0, 0, recvbuf, *recv_len);
#else
			res = dm_i2c_read(dev, 0, recvbuf, *recv_len);
#endif

		}
	}
	if (res) {
		printf("i2c_read returned %d (rlen=%d)\n", res, *recv_len);
#ifdef DEBUG
		print_buffer(0, recvbuf, 1, *recv_len, 0);
#endif
	}

#ifdef DEBUG
	if (!res) {
		printf("read from TPM (%d bytes):\n", *recv_len);
		print_buffer(0, recvbuf, 1, *recv_len, 0);
	}
#endif

	return res;
}

static int tpm_atmel_twi_probe(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id tpm_atmel_twi_ids[] = {
	{ .compatible = "atmel,at97sc3204t"},
	{ }
};

static const struct tpm_ops tpm_atmel_twi_ops = {
	.open = tpm_atmel_twi_open,
	.close = tpm_atmel_twi_close,
	.xfer = tpm_atmel_twi_xfer,
	.get_desc = tpm_atmel_twi_get_desc,
};

U_BOOT_DRIVER(tpm_atmel_twi) = {
	.name = "tpm_atmel_twi",
	.id = UCLASS_TPM,
	.of_match = tpm_atmel_twi_ids,
	.ops = &tpm_atmel_twi_ops,
	.probe = tpm_atmel_twi_probe,
};
