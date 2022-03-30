// SPDX-License-Identifier: GPL-2.0+
/*
 * Chromium OS cros_ec driver - SPI interface
 *
 * Copyright (c) 2012 The Chromium OS Authors.
 */

/*
 * The Matrix Keyboard Protocol driver handles talking to the keyboard
 * controller chip. Mostly this is for keyboard functions, but some other
 * things have slipped in, so we provide generic services to talk to the
 * KBC.
 */

#include <common.h>
#include <cros_ec.h>
#include <dm.h>
#include <errno.h>
#include <spi.h>

int cros_ec_spi_packet(struct udevice *udev, int out_bytes, int in_bytes)
{
	struct cros_ec_dev *dev = dev_get_uclass_priv(udev);
	struct spi_slave *slave = dev_get_parent_priv(dev->dev);
	ulong start;
	uint8_t byte;
	int rv;

	/* Do the transfer */
	if (spi_claim_bus(slave)) {
		debug("%s: Cannot claim SPI bus\n", __func__);
		return -1;
	}

	rv = spi_xfer(slave, out_bytes * 8, dev->dout, NULL, SPI_XFER_BEGIN);
	if (rv)
		goto done;
	start = get_timer(0);
	while (1) {
		rv = spi_xfer(slave, 8, NULL, &byte, 0);
		if (byte == SPI_PREAMBLE_END_BYTE)
			break;
		if (rv)
			goto done;
		if (get_timer(start) > 100) {
			rv = -ETIMEDOUT;
			goto done;
		}
	}

	rv = spi_xfer(slave, in_bytes * 8, NULL, dev->din, 0);
done:
	spi_xfer(slave, 0, NULL, NULL, SPI_XFER_END);
	spi_release_bus(slave);

	if (rv) {
		debug("%s: Cannot complete SPI transfer\n", __func__);
		return -1;
	}

	return in_bytes;
}

/**
 * Send a command to a LPC CROS_EC device and return the reply.
 *
 * The device's internal input/output buffers are used.
 *
 * @param dev		CROS_EC device
 * @param cmd		Command to send (EC_CMD_...)
 * @param cmd_version	Version of command to send (EC_VER_...)
 * @param dout		Output data (may be NULL If dout_len=0)
 * @param dout_len      Size of output data in bytes
 * @param dinp		Returns pointer to response data. This will be
 *			untouched unless we return a value > 0.
 * @param din_len	Maximum size of response in bytes
 * @return number of bytes in response, or -1 on error
 */
int cros_ec_spi_command(struct udevice *udev, uint8_t cmd, int cmd_version,
		     const uint8_t *dout, int dout_len,
		     uint8_t **dinp, int din_len)
{
	struct cros_ec_dev *dev = dev_get_uclass_priv(udev);
	struct spi_slave *slave = dev_get_parent_priv(dev->dev);
	int in_bytes = din_len + 4;	/* status, length, checksum, trailer */
	uint8_t *out;
	uint8_t *p;
	int csum, len;
	int rv;

	if (dev->protocol_version != 2) {
		debug("%s: Unsupported EC protcol version %d\n",
		      __func__, dev->protocol_version);
		return -1;
	}

	/*
	 * Sanity-check input size to make sure it plus transaction overhead
	 * fits in the internal device buffer.
	 */
	if (in_bytes > sizeof(dev->din)) {
		debug("%s: Cannot receive %d bytes\n", __func__, din_len);
		return -1;
	}

	/* We represent message length as a byte */
	if (dout_len > 0xff) {
		debug("%s: Cannot send %d bytes\n", __func__, dout_len);
		return -1;
	}

	/*
	 * Clear input buffer so we don't get false hits for MSG_HEADER
	 */
	memset(dev->din, '\0', in_bytes);

	if (spi_claim_bus(slave)) {
		debug("%s: Cannot claim SPI bus\n", __func__);
		return -1;
	}

	out = dev->dout;
	out[0] = EC_CMD_VERSION0 + cmd_version;
	out[1] = cmd;
	out[2] = (uint8_t)dout_len;
	memcpy(out + 3, dout, dout_len);
	csum = cros_ec_calc_checksum(out, 3)
	       + cros_ec_calc_checksum(dout, dout_len);
	out[3 + dout_len] = (uint8_t)csum;

	/*
	 * Send output data and receive input data starting such that the
	 * message body will be dword aligned.
	 */
	p = dev->din + sizeof(int64_t) - 2;
	len = dout_len + 4;
	cros_ec_dump_data("out", cmd, out, len);
	rv = spi_xfer(slave, max(len, in_bytes) * 8, out, p,
		      SPI_XFER_BEGIN | SPI_XFER_END);

	spi_release_bus(slave);

	if (rv) {
		debug("%s: Cannot complete SPI transfer\n", __func__);
		return -1;
	}

	len = min((int)p[1], din_len);
	cros_ec_dump_data("in", -1, p, len + 3);

	/* Response code is first byte of message */
	if (p[0] != EC_RES_SUCCESS) {
		printf("%s: Returned status %d\n", __func__, p[0]);
		return -(int)(p[0]);
	}

	/* Check checksum */
	csum = cros_ec_calc_checksum(p, len + 2);
	if (csum != p[len + 2]) {
		debug("%s: Invalid checksum rx %#02x, calced %#02x\n", __func__,
		      p[2 + len], csum);
		return -1;
	}

	/* Anything else is the response data */
	*dinp = p + 2;

	return len;
}

static int cros_ec_probe(struct udevice *dev)
{
	return cros_ec_register(dev);
}

static struct dm_cros_ec_ops cros_ec_ops = {
	.packet = cros_ec_spi_packet,
	.command = cros_ec_spi_command,
};

static const struct udevice_id cros_ec_ids[] = {
	{ .compatible = "google,cros-ec-spi" },
	{ }
};

U_BOOT_DRIVER(cros_ec_spi) = {
	.name		= "cros_ec_spi",
	.id		= UCLASS_CROS_EC,
	.of_match	= cros_ec_ids,
	.probe		= cros_ec_probe,
	.ops		= &cros_ec_ops,
};
