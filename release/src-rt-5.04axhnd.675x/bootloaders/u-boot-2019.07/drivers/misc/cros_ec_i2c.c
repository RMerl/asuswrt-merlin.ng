// SPDX-License-Identifier: GPL-2.0+
/*
 * Chromium OS cros_ec driver - I2C interface
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
#include <dm.h>
#include <i2c.h>
#include <cros_ec.h>

#ifdef DEBUG_TRACE
#define debug_trace(fmt, b...)	debug(fmt, #b)
#else
#define debug_trace(fmt, b...)
#endif

/**
 * Request format for protocol v3
 * byte 0	0xda (EC_COMMAND_PROTOCOL_3)
 * byte 1-8	struct ec_host_request
 * byte 10-	response data
 */
struct ec_host_request_i2c {
	/* Always 0xda to backward compatible with v2 struct */
	uint8_t  command_protocol;
	struct ec_host_request ec_request;
} __packed;

/*
 * Response format for protocol v3
 * byte 0	result code
 * byte 1	packet_length
 * byte 2-9	struct ec_host_response
 * byte 10-	response data
 */
struct ec_host_response_i2c {
	uint8_t result;
	uint8_t packet_length;
	struct ec_host_response ec_response;
} __packed;

static int cros_ec_i2c_packet(struct udevice *udev, int out_bytes, int in_bytes)
{
	struct cros_ec_dev *dev = dev_get_uclass_priv(udev);
	struct dm_i2c_chip *chip = dev_get_parent_platdata(udev);
	struct ec_host_request_i2c *ec_request_i2c =
		(struct ec_host_request_i2c *)dev->dout;
	struct ec_host_response_i2c *ec_response_i2c =
		(struct ec_host_response_i2c *)dev->din;
	struct i2c_msg i2c_msg[2];
	int ret;

	i2c_msg[0].addr = chip->chip_addr;
	i2c_msg[0].flags = 0;
	i2c_msg[1].addr = chip->chip_addr;
	i2c_msg[1].flags = I2C_M_RD;

	/* one extra byte, to indicate v3 */
	i2c_msg[0].len = out_bytes + 1;
	i2c_msg[0].buf = dev->dout;

	/* stitch on EC_COMMAND_PROTOCOL_3 */
	memmove(&ec_request_i2c->ec_request, dev->dout, out_bytes);
	ec_request_i2c->command_protocol = EC_COMMAND_PROTOCOL_3;

	/* two extra bytes for v3 */
	i2c_msg[1].len = in_bytes + 2;
	i2c_msg[1].buf = dev->din;

	ret = dm_i2c_xfer(udev, &i2c_msg[0], 2);
	if (ret) {
		printf("%s: Could not execute transfer: %d\n", __func__, ret);
		return ret;
	}

	/* When we send a v3 request to v2 ec, ec won't recognize the 0xda
	 * (EC_COMMAND_PROTOCOL_3) and will return with status
	 * EC_RES_INVALID_COMMAND with zero data length
	 *
	 * In case of invalid command for v3 protocol the data length
	 * will be at least sizeof(struct ec_host_response)
	 */
	if (ec_response_i2c->result == EC_RES_INVALID_COMMAND &&
	    ec_response_i2c->packet_length == 0)
		return -EPROTONOSUPPORT;

	if (ec_response_i2c->packet_length < sizeof(struct ec_host_response)) {
		printf("%s: response of %u bytes too short; not a full hdr\n",
		       __func__, ec_response_i2c->packet_length);
		return -EBADMSG;
	}


	/* drop result and packet_len */
	memmove(dev->din, &ec_response_i2c->ec_response, in_bytes);

	return in_bytes;
}

static int cros_ec_i2c_command(struct udevice *udev, uint8_t cmd,
			       int cmd_version, const uint8_t *dout,
			       int dout_len, uint8_t **dinp, int din_len)
{
	struct cros_ec_dev *dev = dev_get_uclass_priv(udev);
	struct dm_i2c_chip *chip = dev_get_parent_platdata(udev);
	struct i2c_msg i2c_msg[2];
	/* version8, cmd8, arglen8, out8[dout_len], csum8 */
	int out_bytes = dout_len + 4;
	/* response8, arglen8, in8[din_len], checksum8 */
	int in_bytes = din_len + 3;
	uint8_t *ptr;
	/* Receive input data, so that args will be dword aligned */
	uint8_t *in_ptr;
	int len, csum, ret;

	/*
	 * Sanity-check I/O sizes given transaction overhead in internal
	 * buffers.
	 */
	if (out_bytes > sizeof(dev->dout)) {
		debug("%s: Cannot send %d bytes\n", __func__, dout_len);
		return -1;
	}
	if (in_bytes > sizeof(dev->din)) {
		debug("%s: Cannot receive %d bytes\n", __func__, din_len);
		return -1;
	}
	assert(dout_len >= 0);
	assert(dinp);

	i2c_msg[0].addr = chip->chip_addr;
	i2c_msg[0].len = out_bytes;
	i2c_msg[0].buf = dev->dout;
	i2c_msg[0].flags = 0;

	/*
	 * Copy command and data into output buffer so we can do a single I2C
	 * burst transaction.
	 */
	ptr = dev->dout;

	/*
	 * in_ptr starts of pointing to a dword-aligned input data buffer.
	 * We decrement it back by the number of header bytes we expect to
	 * receive, so that the first parameter of the resulting input data
	 * will be dword aligned.
	 */
	in_ptr = dev->din + sizeof(int64_t);

	if (dev->protocol_version != 2) {
		/* Something we don't support */
		debug("%s: Protocol version %d unsupported\n",
		      __func__, dev->protocol_version);
		return -1;
	}

	*ptr++ = EC_CMD_VERSION0 + cmd_version;
	*ptr++ = cmd;
	*ptr++ = dout_len;
	in_ptr -= 2;	/* Expect status, length bytes */

	memcpy(ptr, dout, dout_len);
	ptr += dout_len;

	*ptr++ = (uint8_t)
		cros_ec_calc_checksum(dev->dout, dout_len + 3);

	i2c_msg[1].addr = chip->chip_addr;
	i2c_msg[1].len = in_bytes;
	i2c_msg[1].buf = in_ptr;
	i2c_msg[1].flags = I2C_M_RD;

	/* Send output data */
	cros_ec_dump_data("out", -1, dev->dout, out_bytes);

	ret = dm_i2c_xfer(udev, &i2c_msg[0], 2);
	if (ret) {
		debug("%s: Could not execute transfer to %s\n", __func__,
		      udev->name);
		ret = -1;
	}

	if (*in_ptr != EC_RES_SUCCESS) {
		debug("%s: Received bad result code %d\n", __func__, *in_ptr);
		return -(int)*in_ptr;
	}

	len = in_ptr[1];
	if (len + 3 > sizeof(dev->din)) {
		debug("%s: Received length %#02x too large\n",
		      __func__, len);
		return -1;
	}
	csum = cros_ec_calc_checksum(in_ptr, 2 + len);
	if (csum != in_ptr[2 + len]) {
		debug("%s: Invalid checksum rx %#02x, calced %#02x\n",
		      __func__, in_ptr[2 + din_len], csum);
		return -1;
	}
	din_len = min(din_len, len);
	cros_ec_dump_data("in", -1, in_ptr, din_len + 3);

	/* Return pointer to dword-aligned input data, if any */
	*dinp = dev->din + sizeof(int64_t);

	return din_len;
}

static int cros_ec_probe(struct udevice *dev)
{
	return cros_ec_register(dev);
}

static struct dm_cros_ec_ops cros_ec_ops = {
	.command = cros_ec_i2c_command,
	.packet = cros_ec_i2c_packet,
};

static const struct udevice_id cros_ec_ids[] = {
	{ .compatible = "google,cros-ec-i2c" },
	{ }
};

U_BOOT_DRIVER(cros_ec_i2c) = {
	.name		= "cros_ec_i2c",
	.id		= UCLASS_CROS_EC,
	.of_match	= cros_ec_ids,
	.probe		= cros_ec_probe,
	.ops		= &cros_ec_ops,
};
