// SPDX-License-Identifier: GPL-2.0+
/*
 * Chromium OS cros_ec driver - LPC interface
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
#include <command.h>
#include <cros_ec.h>
#include <log.h>
#include <asm/io.h>

#ifdef DEBUG_TRACE
#define debug_trace(fmt, b...)	debug(fmt, ##b)
#else
#define debug_trace(fmt, b...)
#endif

/* Timeout waiting for a flash erase command to complete */
static const int CROS_EC_CMD_TIMEOUT_MS = 5000;

static int wait_for_sync(struct cros_ec_dev *dev)
{
	unsigned long start;

	start = get_timer(0);
	while (inb(EC_LPC_ADDR_HOST_CMD) & EC_LPC_STATUS_BUSY_MASK) {
		if (get_timer(start) > CROS_EC_CMD_TIMEOUT_MS) {
			debug("%s: Timeout waiting for CROS_EC sync\n",
			      __func__);
			return -1;
		}
	}

	return 0;
}

int cros_ec_lpc_packet(struct udevice *udev, int out_bytes, int in_bytes)
{
	struct cros_ec_dev *dev = dev_get_uclass_priv(udev);
	uint8_t *d;
	int i;

	if (out_bytes > EC_LPC_HOST_PACKET_SIZE)
		return log_msg_ret("Cannot send that many bytes\n", -E2BIG);

	if (in_bytes > EC_LPC_HOST_PACKET_SIZE)
		return log_msg_ret("Cannot receive that many bytes\n", -E2BIG);

	if (wait_for_sync(dev))
		return log_msg_ret("Timeout waiting ready\n", -ETIMEDOUT);

	/* Write data */
	for (i = 0, d = (uint8_t *)dev->dout; i < out_bytes; i++, d++)
		outb(*d, EC_LPC_ADDR_HOST_PACKET + i);

	/* Start the command */
	outb(EC_COMMAND_PROTOCOL_3, EC_LPC_ADDR_HOST_CMD);

	if (wait_for_sync(dev))
		return log_msg_ret("Timeout waiting ready\n", -ETIMEDOUT);

	/* Read back args */
	for (i = 0, d = dev->din; i < in_bytes; i++, d++)
		*d = inb(EC_LPC_ADDR_HOST_PACKET + i);

	return in_bytes;
}

int cros_ec_lpc_command(struct udevice *udev, uint8_t cmd, int cmd_version,
		     const uint8_t *dout, int dout_len,
		     uint8_t **dinp, int din_len)
{
	struct cros_ec_dev *dev = dev_get_uclass_priv(udev);
	const int cmd_addr = EC_LPC_ADDR_HOST_CMD;
	const int data_addr = EC_LPC_ADDR_HOST_DATA;
	const int args_addr = EC_LPC_ADDR_HOST_ARGS;
	const int param_addr = EC_LPC_ADDR_HOST_PARAM;

	struct ec_lpc_host_args args;
	uint8_t *d;
	int csum;
	int i;

	if (dout_len > EC_PROTO2_MAX_PARAM_SIZE) {
		debug("%s: Cannot send %d bytes\n", __func__, dout_len);
		return -1;
	}

	/* Fill in args */
	args.flags = EC_HOST_ARGS_FLAG_FROM_HOST;
	args.command_version = cmd_version;
	args.data_size = dout_len;

	/* Calculate checksum */
	csum = cmd + args.flags + args.command_version + args.data_size;
	for (i = 0, d = (uint8_t *)dout; i < dout_len; i++, d++)
		csum += *d;

	args.checksum = (uint8_t)csum;

	if (wait_for_sync(dev)) {
		debug("%s: Timeout waiting ready\n", __func__);
		return -1;
	}

	/* Write args */
	for (i = 0, d = (uint8_t *)&args; i < sizeof(args); i++, d++)
		outb(*d, args_addr + i);

	/* Write data, if any */
	debug_trace("cmd: %02x, ver: %02x", cmd, cmd_version);
	for (i = 0, d = (uint8_t *)dout; i < dout_len; i++, d++) {
		outb(*d, param_addr + i);
		debug_trace("%02x ", *d);
	}

	outb(cmd, cmd_addr);
	debug_trace("\n");

	if (wait_for_sync(dev)) {
		debug("%s: Timeout waiting for response\n", __func__);
		return -1;
	}

	/* Check result */
	i = inb(data_addr);
	if (i) {
		debug("%s: CROS_EC result code %d\n", __func__, i);
		return -i;
	}

	/* Read back args */
	for (i = 0, d = (uint8_t *)&args; i < sizeof(args); i++, d++)
		*d = inb(args_addr + i);

	/*
	 * If EC didn't modify args flags, then somehow we sent a new-style
	 * command to an old EC, which means it would have read its params
	 * from the wrong place.
	 */
	if (!(args.flags & EC_HOST_ARGS_FLAG_TO_HOST)) {
		debug("%s: CROS_EC protocol mismatch\n", __func__);
		return -EC_RES_INVALID_RESPONSE;
	}

	if (args.data_size > din_len) {
		debug("%s: CROS_EC returned too much data %d > %d\n",
		      __func__, args.data_size, din_len);
		return -EC_RES_INVALID_RESPONSE;
	}

	/* Read data, if any */
	for (i = 0, d = (uint8_t *)dev->din; i < args.data_size; i++, d++) {
		*d = inb(param_addr + i);
		debug_trace("%02x ", *d);
	}
	debug_trace("\n");

	/* Verify checksum */
	csum = cmd + args.flags + args.command_version + args.data_size;
	for (i = 0, d = (uint8_t *)dev->din; i < args.data_size; i++, d++)
		csum += *d;

	if (args.checksum != (uint8_t)csum) {
		debug("%s: CROS_EC response has invalid checksum\n", __func__);
		return -EC_RES_INVALID_CHECKSUM;
	}
	*dinp = dev->din;

	/* Return actual amount of data received */
	return args.data_size;
}

/**
 * Initialize LPC protocol.
 *
 * @param dev		CROS_EC device
 * @param blob		Device tree blob
 * Return: 0 if ok, -1 on error
 */
int cros_ec_lpc_init(struct cros_ec_dev *dev, const void *blob)
{
	int byte, i;

	/* See if we can find an EC at the other end */
	byte = 0xff;
	byte &= inb(EC_LPC_ADDR_HOST_CMD);
	byte &= inb(EC_LPC_ADDR_HOST_DATA);
	for (i = 0; i < EC_PROTO2_MAX_PARAM_SIZE && (byte == 0xff); i++)
		byte &= inb(EC_LPC_ADDR_HOST_PARAM + i);
	if (byte == 0xff) {
		debug("%s: CROS_EC device not found on LPC bus\n",
			__func__);
		return -1;
	}

	return 0;
}

/* Return the byte of EC switch states */
static int cros_ec_lpc_get_switches(struct udevice *dev)
{
	return inb(EC_LPC_ADDR_MEMMAP + EC_MEMMAP_SWITCHES);
}

/*
 * Test if LPC command args are supported.
 *
 * The cheapest way to do this is by looking for the memory-mapped
 * flag.  This is faster than sending a new-style 'hello' command and
 * seeing whether the EC sets the EC_HOST_ARGS_FLAG_FROM_HOST flag
 * in args when it responds.
 */
static int cros_ec_lpc_check_version(struct udevice *dev)
{
	if (inb(EC_LPC_ADDR_MEMMAP + EC_MEMMAP_ID) == 'E' &&
			inb(EC_LPC_ADDR_MEMMAP + EC_MEMMAP_ID + 1)
				== 'C' &&
			(inb(EC_LPC_ADDR_MEMMAP +
				EC_MEMMAP_HOST_CMD_FLAGS) &
				EC_HOST_CMD_FLAG_LPC_ARGS_SUPPORTED)) {
		return 0;
	}

	printf("%s: ERROR: old EC interface not supported\n", __func__);
	return -1;
}

static int cros_ec_probe(struct udevice *dev)
{
	return cros_ec_register(dev);
}

static struct dm_cros_ec_ops cros_ec_ops = {
	.packet = cros_ec_lpc_packet,
	.command = cros_ec_lpc_command,
	.check_version = cros_ec_lpc_check_version,
	.get_switches = cros_ec_lpc_get_switches,
};

static const struct udevice_id cros_ec_ids[] = {
	{ .compatible = "google,cros-ec-lpc" },
	{ }
};

U_BOOT_DRIVER(google_cros_ec_lpc) = {
	.name		= "google_cros_ec_lpc",
	.id		= UCLASS_CROS_EC,
	.of_match	= cros_ec_ids,
	.probe		= cros_ec_probe,
	.ops		= &cros_ec_ops,
};
