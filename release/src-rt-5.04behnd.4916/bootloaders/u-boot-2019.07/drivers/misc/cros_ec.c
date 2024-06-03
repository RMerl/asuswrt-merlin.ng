// SPDX-License-Identifier: GPL-2.0+
/*
 * Chromium OS cros_ec driver
 *
 * Copyright (c) 2012 The Chromium OS Authors.
 */

/*
 * This is the interface to the Chrome OS EC. It provides keyboard functions,
 * power control and battery management. Quite a few other functions are
 * provided to enable the EC software to be updated, talk to the EC's I2C bus
 * and store a small amount of data in a memory which persists while the EC
 * is not reset.
 */

#define LOG_CATEGORY UCLASS_CROS_EC

#include <common.h>
#include <command.h>
#include <dm.h>
#include <i2c.h>
#include <cros_ec.h>
#include <fdtdec.h>
#include <malloc.h>
#include <spi.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <dm/device-internal.h>
#include <dm/of_extra.h>
#include <dm/uclass-internal.h>

#ifdef DEBUG_TRACE
#define debug_trace(fmt, b...)	debug(fmt, #b)
#else
#define debug_trace(fmt, b...)
#endif

enum {
	/* Timeout waiting for a flash erase command to complete */
	CROS_EC_CMD_TIMEOUT_MS	= 5000,
	/* Timeout waiting for a synchronous hash to be recomputed */
	CROS_EC_CMD_HASH_TIMEOUT_MS = 2000,
};

#define INVALID_HCMD 0xFF

/*
 * Map UHEPI masks to non UHEPI commands in order to support old EC FW
 * which does not support UHEPI command.
 */
static const struct {
	u8 set_cmd;
	u8 clear_cmd;
	u8 get_cmd;
} event_map[] = {
	[EC_HOST_EVENT_MAIN] = {
		INVALID_HCMD, EC_CMD_HOST_EVENT_CLEAR,
		INVALID_HCMD,
	},
	[EC_HOST_EVENT_B] = {
		INVALID_HCMD, EC_CMD_HOST_EVENT_CLEAR_B,
		EC_CMD_HOST_EVENT_GET_B,
	},
	[EC_HOST_EVENT_SCI_MASK] = {
		EC_CMD_HOST_EVENT_SET_SCI_MASK, INVALID_HCMD,
		EC_CMD_HOST_EVENT_GET_SCI_MASK,
	},
	[EC_HOST_EVENT_SMI_MASK] = {
		EC_CMD_HOST_EVENT_SET_SMI_MASK, INVALID_HCMD,
		EC_CMD_HOST_EVENT_GET_SMI_MASK,
	},
	[EC_HOST_EVENT_ALWAYS_REPORT_MASK] = {
		INVALID_HCMD, INVALID_HCMD, INVALID_HCMD,
	},
	[EC_HOST_EVENT_ACTIVE_WAKE_MASK] = {
		EC_CMD_HOST_EVENT_SET_WAKE_MASK, INVALID_HCMD,
		EC_CMD_HOST_EVENT_GET_WAKE_MASK,
	},
	[EC_HOST_EVENT_LAZY_WAKE_MASK_S0IX] = {
		EC_CMD_HOST_EVENT_SET_WAKE_MASK, INVALID_HCMD,
		EC_CMD_HOST_EVENT_GET_WAKE_MASK,
	},
	[EC_HOST_EVENT_LAZY_WAKE_MASK_S3] = {
		EC_CMD_HOST_EVENT_SET_WAKE_MASK, INVALID_HCMD,
		EC_CMD_HOST_EVENT_GET_WAKE_MASK,
	},
	[EC_HOST_EVENT_LAZY_WAKE_MASK_S5] = {
		EC_CMD_HOST_EVENT_SET_WAKE_MASK, INVALID_HCMD,
		EC_CMD_HOST_EVENT_GET_WAKE_MASK,
	},
};

void cros_ec_dump_data(const char *name, int cmd, const uint8_t *data, int len)
{
#ifdef DEBUG
	int i;

	printf("%s: ", name);
	if (cmd != -1)
		printf("cmd=%#x: ", cmd);
	for (i = 0; i < len; i++)
		printf("%02x ", data[i]);
	printf("\n");
#endif
}

/*
 * Calculate a simple 8-bit checksum of a data block
 *
 * @param data	Data block to checksum
 * @param size	Size of data block in bytes
 * @return checksum value (0 to 255)
 */
int cros_ec_calc_checksum(const uint8_t *data, int size)
{
	int csum, i;

	for (i = csum = 0; i < size; i++)
		csum += data[i];
	return csum & 0xff;
}

/**
 * Create a request packet for protocol version 3.
 *
 * The packet is stored in the device's internal output buffer.
 *
 * @param dev		CROS-EC device
 * @param cmd		Command to send (EC_CMD_...)
 * @param cmd_version	Version of command to send (EC_VER_...)
 * @param dout          Output data (may be NULL If dout_len=0)
 * @param dout_len      Size of output data in bytes
 * @return packet size in bytes, or <0 if error.
 */
static int create_proto3_request(struct cros_ec_dev *cdev,
				 int cmd, int cmd_version,
				 const void *dout, int dout_len)
{
	struct ec_host_request *rq = (struct ec_host_request *)cdev->dout;
	int out_bytes = dout_len + sizeof(*rq);

	/* Fail if output size is too big */
	if (out_bytes > (int)sizeof(cdev->dout)) {
		debug("%s: Cannot send %d bytes\n", __func__, dout_len);
		return -EC_RES_REQUEST_TRUNCATED;
	}

	/* Fill in request packet */
	rq->struct_version = EC_HOST_REQUEST_VERSION;
	rq->checksum = 0;
	rq->command = cmd;
	rq->command_version = cmd_version;
	rq->reserved = 0;
	rq->data_len = dout_len;

	/* Copy data after header */
	memcpy(rq + 1, dout, dout_len);

	/* Write checksum field so the entire packet sums to 0 */
	rq->checksum = (uint8_t)(-cros_ec_calc_checksum(cdev->dout, out_bytes));

	cros_ec_dump_data("out", cmd, cdev->dout, out_bytes);

	/* Return size of request packet */
	return out_bytes;
}

/**
 * Prepare the device to receive a protocol version 3 response.
 *
 * @param dev		CROS-EC device
 * @param din_len       Maximum size of response in bytes
 * @return maximum expected number of bytes in response, or <0 if error.
 */
static int prepare_proto3_response_buffer(struct cros_ec_dev *cdev, int din_len)
{
	int in_bytes = din_len + sizeof(struct ec_host_response);

	/* Fail if input size is too big */
	if (in_bytes > (int)sizeof(cdev->din)) {
		debug("%s: Cannot receive %d bytes\n", __func__, din_len);
		return -EC_RES_RESPONSE_TOO_BIG;
	}

	/* Return expected size of response packet */
	return in_bytes;
}

/**
 * Handle a protocol version 3 response packet.
 *
 * The packet must already be stored in the device's internal input buffer.
 *
 * @param dev		CROS-EC device
 * @param dinp          Returns pointer to response data
 * @param din_len       Maximum size of response in bytes
 * @return number of bytes of response data, or <0 if error. Note that error
 * codes can be from errno.h or -ve EC_RES_INVALID_CHECKSUM values (and they
 * overlap!)
 */
static int handle_proto3_response(struct cros_ec_dev *dev,
				  uint8_t **dinp, int din_len)
{
	struct ec_host_response *rs = (struct ec_host_response *)dev->din;
	int in_bytes;
	int csum;

	cros_ec_dump_data("in-header", -1, dev->din, sizeof(*rs));

	/* Check input data */
	if (rs->struct_version != EC_HOST_RESPONSE_VERSION) {
		debug("%s: EC response version mismatch\n", __func__);
		return -EC_RES_INVALID_RESPONSE;
	}

	if (rs->reserved) {
		debug("%s: EC response reserved != 0\n", __func__);
		return -EC_RES_INVALID_RESPONSE;
	}

	if (rs->data_len > din_len) {
		debug("%s: EC returned too much data\n", __func__);
		return -EC_RES_RESPONSE_TOO_BIG;
	}

	cros_ec_dump_data("in-data", -1, dev->din + sizeof(*rs), rs->data_len);

	/* Update in_bytes to actual data size */
	in_bytes = sizeof(*rs) + rs->data_len;

	/* Verify checksum */
	csum = cros_ec_calc_checksum(dev->din, in_bytes);
	if (csum) {
		debug("%s: EC response checksum invalid: 0x%02x\n", __func__,
		      csum);
		return -EC_RES_INVALID_CHECKSUM;
	}

	/* Return error result, if any */
	if (rs->result)
		return -(int)rs->result;

	/* If we're still here, set response data pointer and return length */
	*dinp = (uint8_t *)(rs + 1);

	return rs->data_len;
}

static int send_command_proto3(struct cros_ec_dev *cdev,
			       int cmd, int cmd_version,
			       const void *dout, int dout_len,
			       uint8_t **dinp, int din_len)
{
	struct dm_cros_ec_ops *ops;
	int out_bytes, in_bytes;
	int rv;

	/* Create request packet */
	out_bytes = create_proto3_request(cdev, cmd, cmd_version,
					  dout, dout_len);
	if (out_bytes < 0)
		return out_bytes;

	/* Prepare response buffer */
	in_bytes = prepare_proto3_response_buffer(cdev, din_len);
	if (in_bytes < 0)
		return in_bytes;

	ops = dm_cros_ec_get_ops(cdev->dev);
	rv = ops->packet ? ops->packet(cdev->dev, out_bytes, in_bytes) :
			-ENOSYS;
	if (rv < 0)
		return rv;

	/* Process the response */
	return handle_proto3_response(cdev, dinp, din_len);
}

static int send_command(struct cros_ec_dev *dev, uint cmd, int cmd_version,
			const void *dout, int dout_len,
			uint8_t **dinp, int din_len)
{
	struct dm_cros_ec_ops *ops;
	int ret = -1;

	/* Handle protocol version 3 support */
	if (dev->protocol_version == 3) {
		return send_command_proto3(dev, cmd, cmd_version,
					   dout, dout_len, dinp, din_len);
	}

	ops = dm_cros_ec_get_ops(dev->dev);
	ret = ops->command(dev->dev, cmd, cmd_version,
			   (const uint8_t *)dout, dout_len, dinp, din_len);

	return ret;
}

/**
 * Send a command to the CROS-EC device and return the reply.
 *
 * The device's internal input/output buffers are used.
 *
 * @param dev		CROS-EC device
 * @param cmd		Command to send (EC_CMD_...)
 * @param cmd_version	Version of command to send (EC_VER_...)
 * @param dout          Output data (may be NULL If dout_len=0)
 * @param dout_len      Size of output data in bytes
 * @param dinp          Response data (may be NULL If din_len=0).
 *			If not NULL, it will be updated to point to the data
 *			and will always be double word aligned (64-bits)
 * @param din_len       Maximum size of response in bytes
 * @return number of bytes in response, or -ve on error
 */
static int ec_command_inptr(struct udevice *dev, uint8_t cmd,
			    int cmd_version, const void *dout, int dout_len,
			    uint8_t **dinp, int din_len)
{
	struct cros_ec_dev *cdev = dev_get_uclass_priv(dev);
	uint8_t *din = NULL;
	int len;

	len = send_command(cdev, cmd, cmd_version, dout, dout_len, &din,
			   din_len);

	/* If the command doesn't complete, wait a while */
	if (len == -EC_RES_IN_PROGRESS) {
		struct ec_response_get_comms_status *resp = NULL;
		ulong start;

		/* Wait for command to complete */
		start = get_timer(0);
		do {
			int ret;

			mdelay(50);	/* Insert some reasonable delay */
			ret = send_command(cdev, EC_CMD_GET_COMMS_STATUS, 0,
					   NULL, 0,
					   (uint8_t **)&resp, sizeof(*resp));
			if (ret < 0)
				return ret;

			if (get_timer(start) > CROS_EC_CMD_TIMEOUT_MS) {
				debug("%s: Command %#02x timeout\n",
				      __func__, cmd);
				return -EC_RES_TIMEOUT;
			}
		} while (resp->flags & EC_COMMS_STATUS_PROCESSING);

		/* OK it completed, so read the status response */
		/* not sure why it was 0 for the last argument */
		len = send_command(cdev, EC_CMD_RESEND_RESPONSE, 0, NULL, 0,
				   &din, din_len);
	}

	debug("%s: len=%d, din=%p\n", __func__, len, din);
	if (dinp) {
		/* If we have any data to return, it must be 64bit-aligned */
		assert(len <= 0 || !((uintptr_t)din & 7));
		*dinp = din;
	}

	return len;
}

/**
 * Send a command to the CROS-EC device and return the reply.
 *
 * The device's internal input/output buffers are used.
 *
 * @param dev		CROS-EC device
 * @param cmd		Command to send (EC_CMD_...)
 * @param cmd_version	Version of command to send (EC_VER_...)
 * @param dout          Output data (may be NULL If dout_len=0)
 * @param dout_len      Size of output data in bytes
 * @param din           Response data (may be NULL If din_len=0).
 *			It not NULL, it is a place for ec_command() to copy the
 *      data to.
 * @param din_len       Maximum size of response in bytes
 * @return number of bytes in response, or -ve on error
 */
static int ec_command(struct udevice *dev, uint cmd, int cmd_version,
		      const void *dout, int dout_len,
		      void *din, int din_len)
{
	uint8_t *in_buffer;
	int len;

	assert((din_len == 0) || din);
	len = ec_command_inptr(dev, cmd, cmd_version, dout, dout_len,
			       &in_buffer, din_len);
	if (len > 0) {
		/*
		 * If we were asked to put it somewhere, do so, otherwise just
		 * disregard the result.
		 */
		if (din && in_buffer) {
			assert(len <= din_len);
			memmove(din, in_buffer, len);
		}
	}
	return len;
}

int cros_ec_scan_keyboard(struct udevice *dev, struct mbkp_keyscan *scan)
{
 	if (ec_command(dev, EC_CMD_MKBP_STATE, 0, NULL, 0, scan,
		       sizeof(scan->data)) != sizeof(scan->data))
		return -1;

	return 0;
}

int cros_ec_read_id(struct udevice *dev, char *id, int maxlen)
{
	struct ec_response_get_version *r;
	int ret;

	ret = ec_command_inptr(dev, EC_CMD_GET_VERSION, 0, NULL, 0,
			       (uint8_t **)&r, sizeof(*r));
	if (ret != sizeof(*r)) {
		log_err("Got rc %d, expected %u\n", ret, (uint)sizeof(*r));
		return -1;
	}

	if (maxlen > (int)sizeof(r->version_string_ro))
		maxlen = sizeof(r->version_string_ro);

	switch (r->current_image) {
	case EC_IMAGE_RO:
		memcpy(id, r->version_string_ro, maxlen);
		break;
	case EC_IMAGE_RW:
		memcpy(id, r->version_string_rw, maxlen);
		break;
	default:
		log_err("Invalid EC image %d\n", r->current_image);
		return -1;
	}

	id[maxlen - 1] = '\0';
	return 0;
}

int cros_ec_read_version(struct udevice *dev,
			 struct ec_response_get_version **versionp)
{
	if (ec_command_inptr(dev, EC_CMD_GET_VERSION, 0, NULL, 0,
			(uint8_t **)versionp, sizeof(**versionp))
			!= sizeof(**versionp))
		return -1;

	return 0;
}

int cros_ec_read_build_info(struct udevice *dev, char **strp)
{
	if (ec_command_inptr(dev, EC_CMD_GET_BUILD_INFO, 0, NULL, 0,
			(uint8_t **)strp, EC_PROTO2_MAX_PARAM_SIZE) < 0)
		return -1;

	return 0;
}

int cros_ec_read_current_image(struct udevice *dev,
			       enum ec_current_image *image)
{
	struct ec_response_get_version *r;

	if (ec_command_inptr(dev, EC_CMD_GET_VERSION, 0, NULL, 0,
			(uint8_t **)&r, sizeof(*r)) != sizeof(*r))
		return -1;

	*image = r->current_image;
	return 0;
}

static int cros_ec_wait_on_hash_done(struct udevice *dev,
				     struct ec_response_vboot_hash *hash)
{
	struct ec_params_vboot_hash p;
	ulong start;

	start = get_timer(0);
	while (hash->status == EC_VBOOT_HASH_STATUS_BUSY) {
		mdelay(50);	/* Insert some reasonable delay */

		p.cmd = EC_VBOOT_HASH_GET;
		if (ec_command(dev, EC_CMD_VBOOT_HASH, 0, &p, sizeof(p),
		       hash, sizeof(*hash)) < 0)
			return -1;

		if (get_timer(start) > CROS_EC_CMD_HASH_TIMEOUT_MS) {
			debug("%s: EC_VBOOT_HASH_GET timeout\n", __func__);
			return -EC_RES_TIMEOUT;
		}
	}
	return 0;
}

int cros_ec_read_hash(struct udevice *dev, uint hash_offset,
		      struct ec_response_vboot_hash *hash)
{
	struct ec_params_vboot_hash p;
	int rv;

	p.cmd = EC_VBOOT_HASH_GET;
	p.offset = hash_offset;
	if (ec_command(dev, EC_CMD_VBOOT_HASH, 0, &p, sizeof(p),
		       hash, sizeof(*hash)) < 0)
		return -1;

	/* If the EC is busy calculating the hash, fidget until it's done. */
	rv = cros_ec_wait_on_hash_done(dev, hash);
	if (rv)
		return rv;

	/* If the hash is valid, we're done. Otherwise, we have to kick it off
	 * again and wait for it to complete. Note that we explicitly assume
	 * that hashing zero bytes is always wrong, even though that would
	 * produce a valid hash value. */
	if (hash->status == EC_VBOOT_HASH_STATUS_DONE && hash->size)
		return 0;

	debug("%s: No valid hash (status=%d size=%d). Compute one...\n",
	      __func__, hash->status, hash->size);

	p.cmd = EC_VBOOT_HASH_START;
	p.hash_type = EC_VBOOT_HASH_TYPE_SHA256;
	p.nonce_size = 0;
	p.offset = hash_offset;

	if (ec_command(dev, EC_CMD_VBOOT_HASH, 0, &p, sizeof(p),
		       hash, sizeof(*hash)) < 0)
		return -1;

	rv = cros_ec_wait_on_hash_done(dev, hash);
	if (rv)
		return rv;

	debug("%s: hash done\n", __func__);

	return 0;
}

static int cros_ec_invalidate_hash(struct udevice *dev)
{
	struct ec_params_vboot_hash p;
	struct ec_response_vboot_hash *hash;

	/* We don't have an explict command for the EC to discard its current
	 * hash value, so we'll just tell it to calculate one that we know is
	 * wrong (we claim that hashing zero bytes is always invalid).
	 */
	p.cmd = EC_VBOOT_HASH_RECALC;
	p.hash_type = EC_VBOOT_HASH_TYPE_SHA256;
	p.nonce_size = 0;
	p.offset = 0;
	p.size = 0;

	debug("%s:\n", __func__);

	if (ec_command_inptr(dev, EC_CMD_VBOOT_HASH, 0, &p, sizeof(p),
		       (uint8_t **)&hash, sizeof(*hash)) < 0)
		return -1;

	/* No need to wait for it to finish */
	return 0;
}

int cros_ec_reboot(struct udevice *dev, enum ec_reboot_cmd cmd, uint8_t flags)
{
	struct ec_params_reboot_ec p;

	p.cmd = cmd;
	p.flags = flags;

	if (ec_command_inptr(dev, EC_CMD_REBOOT_EC, 0, &p, sizeof(p), NULL, 0)
			< 0)
		return -1;

	if (!(flags & EC_REBOOT_FLAG_ON_AP_SHUTDOWN)) {
		/*
		 * EC reboot will take place immediately so delay to allow it
		 * to complete.  Note that some reboot types (EC_REBOOT_COLD)
		 * will reboot the AP as well, in which case we won't actually
		 * get to this point.
		 */
		/*
		 * TODO(rspangler@chromium.org): Would be nice if we had a
		 * better way to determine when the reboot is complete.  Could
		 * we poll a memory-mapped LPC value?
		 */
		udelay(50000);
	}

	return 0;
}

int cros_ec_interrupt_pending(struct udevice *dev)
{
	struct cros_ec_dev *cdev = dev_get_uclass_priv(dev);

	/* no interrupt support : always poll */
	if (!dm_gpio_is_valid(&cdev->ec_int))
		return -ENOENT;

	return dm_gpio_get_value(&cdev->ec_int);
}

int cros_ec_info(struct udevice *dev, struct ec_response_mkbp_info *info)
{
	if (ec_command(dev, EC_CMD_MKBP_INFO, 0, NULL, 0, info,
		       sizeof(*info)) != sizeof(*info))
		return -1;

	return 0;
}

int cros_ec_get_event_mask(struct udevice *dev, uint type, uint32_t *mask)
{
	struct ec_response_host_event_mask rsp;
	int ret;

	ret = ec_command(dev, type, 0, NULL, 0, &rsp, sizeof(rsp));
	if (ret < 0)
		return ret;
	else if (ret != sizeof(rsp))
		return -EINVAL;

	*mask = rsp.mask;

	return 0;
}

int cros_ec_set_event_mask(struct udevice *dev, uint type, uint32_t mask)
{
	struct ec_params_host_event_mask req;
	int ret;

	req.mask = mask;

	ret = ec_command(dev, type, 0, &req, sizeof(req), NULL, 0);
	if (ret < 0)
		return ret;

	return 0;
}

int cros_ec_get_host_events(struct udevice *dev, uint32_t *events_ptr)
{
	struct ec_response_host_event_mask *resp;

	/*
	 * Use the B copy of the event flags, because the main copy is already
	 * used by ACPI/SMI.
	 */
	if (ec_command_inptr(dev, EC_CMD_HOST_EVENT_GET_B, 0, NULL, 0,
		       (uint8_t **)&resp, sizeof(*resp)) < (int)sizeof(*resp))
		return -1;

	if (resp->mask & EC_HOST_EVENT_MASK(EC_HOST_EVENT_INVALID))
		return -1;

	*events_ptr = resp->mask;
	return 0;
}

int cros_ec_clear_host_events(struct udevice *dev, uint32_t events)
{
	struct ec_params_host_event_mask params;

	params.mask = events;

	/*
	 * Use the B copy of the event flags, so it affects the data returned
	 * by cros_ec_get_host_events().
	 */
	if (ec_command_inptr(dev, EC_CMD_HOST_EVENT_CLEAR_B, 0,
		       &params, sizeof(params), NULL, 0) < 0)
		return -1;

	return 0;
}

int cros_ec_flash_protect(struct udevice *dev, uint32_t set_mask,
			  uint32_t set_flags,
			  struct ec_response_flash_protect *resp)
{
	struct ec_params_flash_protect params;

	params.mask = set_mask;
	params.flags = set_flags;

	if (ec_command(dev, EC_CMD_FLASH_PROTECT, EC_VER_FLASH_PROTECT,
		       &params, sizeof(params),
		       resp, sizeof(*resp)) != sizeof(*resp))
		return -1;

	return 0;
}

int cros_ec_entering_mode(struct udevice *dev, int mode)
{
	int rc;

	rc = ec_command(dev, EC_CMD_ENTERING_MODE, 0, &mode, sizeof(mode),
			NULL, 0);
	if (rc)
		return -1;
	return 0;
}

static int cros_ec_check_version(struct udevice *dev)
{
	struct cros_ec_dev *cdev = dev_get_uclass_priv(dev);
	struct ec_params_hello req;
	struct ec_response_hello *resp;

	struct dm_cros_ec_ops *ops;
	int ret;

	ops = dm_cros_ec_get_ops(dev);
	if (ops->check_version) {
		ret = ops->check_version(dev);
		if (ret)
			return ret;
	}

	/*
	 * TODO(sjg@chromium.org).
	 * There is a strange oddity here with the EC. We could just ignore
	 * the response, i.e. pass the last two parameters as NULL and 0.
	 * In this case we won't read back very many bytes from the EC.
	 * On the I2C bus the EC gets upset about this and will try to send
	 * the bytes anyway. This means that we will have to wait for that
	 * to complete before continuing with a new EC command.
	 *
	 * This problem is probably unique to the I2C bus.
	 *
	 * So for now, just read all the data anyway.
	 */

	/* Try sending a version 3 packet */
	cdev->protocol_version = 3;
	req.in_data = 0;
	if (ec_command_inptr(dev, EC_CMD_HELLO, 0, &req, sizeof(req),
			     (uint8_t **)&resp, sizeof(*resp)) > 0)
		return 0;

	/* Try sending a version 2 packet */
	cdev->protocol_version = 2;
	if (ec_command_inptr(dev, EC_CMD_HELLO, 0, &req, sizeof(req),
			     (uint8_t **)&resp, sizeof(*resp)) > 0)
		return 0;

	/*
	 * Fail if we're still here, since the EC doesn't understand any
	 * protcol version we speak.  Version 1 interface without command
	 * version is no longer supported, and we don't know about any new
	 * protocol versions.
	 */
	cdev->protocol_version = 0;
	printf("%s: ERROR: old EC interface not supported\n", __func__);
	return -1;
}

int cros_ec_test(struct udevice *dev)
{
	struct ec_params_hello req;
	struct ec_response_hello *resp;

	req.in_data = 0x12345678;
	if (ec_command_inptr(dev, EC_CMD_HELLO, 0, &req, sizeof(req),
		       (uint8_t **)&resp, sizeof(*resp)) < sizeof(*resp)) {
		printf("ec_command_inptr() returned error\n");
		return -1;
	}
	if (resp->out_data != req.in_data + 0x01020304) {
		printf("Received invalid handshake %x\n", resp->out_data);
		return -1;
	}

	return 0;
}

int cros_ec_flash_offset(struct udevice *dev, enum ec_flash_region region,
		      uint32_t *offset, uint32_t *size)
{
	struct ec_params_flash_region_info p;
	struct ec_response_flash_region_info *r;
	int ret;

	p.region = region;
	ret = ec_command_inptr(dev, EC_CMD_FLASH_REGION_INFO,
			 EC_VER_FLASH_REGION_INFO,
			 &p, sizeof(p), (uint8_t **)&r, sizeof(*r));
	if (ret != sizeof(*r))
		return -1;

	if (offset)
		*offset = r->offset;
	if (size)
		*size = r->size;

	return 0;
}

int cros_ec_flash_erase(struct udevice *dev, uint32_t offset, uint32_t size)
{
	struct ec_params_flash_erase p;

	p.offset = offset;
	p.size = size;
	return ec_command_inptr(dev, EC_CMD_FLASH_ERASE, 0, &p, sizeof(p),
			NULL, 0);
}

/**
 * Write a single block to the flash
 *
 * Write a block of data to the EC flash. The size must not exceed the flash
 * write block size which you can obtain from cros_ec_flash_write_burst_size().
 *
 * The offset starts at 0. You can obtain the region information from
 * cros_ec_flash_offset() to find out where to write for a particular region.
 *
 * Attempting to write to the region where the EC is currently running from
 * will result in an error.
 *
 * @param dev		CROS-EC device
 * @param data		Pointer to data buffer to write
 * @param offset	Offset within flash to write to.
 * @param size		Number of bytes to write
 * @return 0 if ok, -1 on error
 */
static int cros_ec_flash_write_block(struct udevice *dev, const uint8_t *data,
				     uint32_t offset, uint32_t size)
{
	struct ec_params_flash_write *p;
	int ret;

	p = malloc(sizeof(*p) + size);
	if (!p)
		return -ENOMEM;

	p->offset = offset;
	p->size = size;
	assert(data && p->size <= EC_FLASH_WRITE_VER0_SIZE);
	memcpy(p + 1, data, p->size);

	ret = ec_command_inptr(dev, EC_CMD_FLASH_WRITE, 0,
			  p, sizeof(*p) + size, NULL, 0) >= 0 ? 0 : -1;

	free(p);

	return ret;
}

/**
 * Return optimal flash write burst size
 */
static int cros_ec_flash_write_burst_size(struct udevice *dev)
{
	return EC_FLASH_WRITE_VER0_SIZE;
}

/**
 * Check if a block of data is erased (all 0xff)
 *
 * This function is useful when dealing with flash, for checking whether a
 * data block is erased and thus does not need to be programmed.
 *
 * @param data		Pointer to data to check (must be word-aligned)
 * @param size		Number of bytes to check (must be word-aligned)
 * @return 0 if erased, non-zero if any word is not erased
 */
static int cros_ec_data_is_erased(const uint32_t *data, int size)
{
	assert(!(size & 3));
	size /= sizeof(uint32_t);
	for (; size > 0; size -= 4, data++)
		if (*data != -1U)
			return 0;

	return 1;
}

/**
 * Read back flash parameters
 *
 * This function reads back parameters of the flash as reported by the EC
 *
 * @param dev  Pointer to device
 * @param info Pointer to output flash info struct
 */
int cros_ec_read_flashinfo(struct udevice *dev,
			   struct ec_response_flash_info *info)
{
	int ret;

	ret = ec_command(dev, EC_CMD_FLASH_INFO, 0,
			 NULL, 0, info, sizeof(*info));
	if (ret < 0)
		return ret;

	return ret < sizeof(*info) ? -1 : 0;
}

int cros_ec_flash_write(struct udevice *dev, const uint8_t *data,
			uint32_t offset, uint32_t size)
{
	struct cros_ec_dev *cdev = dev_get_uclass_priv(dev);
	uint32_t burst = cros_ec_flash_write_burst_size(dev);
	uint32_t end, off;
	int ret;

	if (!burst)
		return -EINVAL;

	/*
	 * TODO: round up to the nearest multiple of write size.  Can get away
	 * without that on link right now because its write size is 4 bytes.
	 */
	end = offset + size;
	for (off = offset; off < end; off += burst, data += burst) {
		uint32_t todo;

		/* If the data is empty, there is no point in programming it */
		todo = min(end - off, burst);
		if (cdev->optimise_flash_write &&
		    cros_ec_data_is_erased((uint32_t *)data, todo))
			continue;

		ret = cros_ec_flash_write_block(dev, data, off, todo);
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * Run verification on a slot
 *
 * @param me     CrosEc instance
 * @param region Region to run verification on
 * @return 0 if success or not applicable. Non-zero if verification failed.
 */
int cros_ec_efs_verify(struct udevice *dev, enum ec_flash_region region)
{
	struct ec_params_efs_verify p;
	int rv;

	log_info("EFS: EC is verifying updated image...\n");
	p.region = region;

	rv = ec_command(dev, EC_CMD_EFS_VERIFY, 0, &p, sizeof(p), NULL, 0);
	if (rv >= 0) {
		log_info("EFS: Verification success\n");
		return 0;
	}
	if (rv == -EC_RES_INVALID_COMMAND) {
		log_info("EFS: EC doesn't support EFS_VERIFY command\n");
		return 0;
	}
	log_info("EFS: Verification failed\n");

	return rv;
}

/**
 * Read a single block from the flash
 *
 * Read a block of data from the EC flash. The size must not exceed the flash
 * write block size which you can obtain from cros_ec_flash_write_burst_size().
 *
 * The offset starts at 0. You can obtain the region information from
 * cros_ec_flash_offset() to find out where to read for a particular region.
 *
 * @param dev		CROS-EC device
 * @param data		Pointer to data buffer to read into
 * @param offset	Offset within flash to read from
 * @param size		Number of bytes to read
 * @return 0 if ok, -1 on error
 */
static int cros_ec_flash_read_block(struct udevice *dev, uint8_t *data,
				    uint32_t offset, uint32_t size)
{
	struct ec_params_flash_read p;

	p.offset = offset;
	p.size = size;

	return ec_command(dev, EC_CMD_FLASH_READ, 0,
			  &p, sizeof(p), data, size) >= 0 ? 0 : -1;
}

int cros_ec_flash_read(struct udevice *dev, uint8_t *data, uint32_t offset,
		       uint32_t size)
{
	uint32_t burst = cros_ec_flash_write_burst_size(dev);
	uint32_t end, off;
	int ret;

	end = offset + size;
	for (off = offset; off < end; off += burst, data += burst) {
		ret = cros_ec_flash_read_block(dev, data, off,
					    min(end - off, burst));
		if (ret)
			return ret;
	}

	return 0;
}

int cros_ec_flash_update_rw(struct udevice *dev, const uint8_t *image,
			    int image_size)
{
	uint32_t rw_offset, rw_size;
	int ret;

	if (cros_ec_flash_offset(dev, EC_FLASH_REGION_ACTIVE, &rw_offset,
		&rw_size))
		return -1;
	if (image_size > (int)rw_size)
		return -1;

	/* Invalidate the existing hash, just in case the AP reboots
	 * unexpectedly during the update. If that happened, the EC RW firmware
	 * would be invalid, but the EC would still have the original hash.
	 */
	ret = cros_ec_invalidate_hash(dev);
	if (ret)
		return ret;

	/*
	 * Erase the entire RW section, so that the EC doesn't see any garbage
	 * past the new image if it's smaller than the current image.
	 *
	 * TODO: could optimize this to erase just the current image, since
	 * presumably everything past that is 0xff's.  But would still need to
	 * round up to the nearest multiple of erase size.
	 */
	ret = cros_ec_flash_erase(dev, rw_offset, rw_size);
	if (ret)
		return ret;

	/* Write the image */
	ret = cros_ec_flash_write(dev, image, rw_offset, image_size);
	if (ret)
		return ret;

	return 0;
}

int cros_ec_read_nvdata(struct udevice *dev, uint8_t *block, int size)
{
	struct ec_params_vbnvcontext p;
	int len;

	if (size != EC_VBNV_BLOCK_SIZE && size != EC_VBNV_BLOCK_SIZE_V2)
		return -EINVAL;

	p.op = EC_VBNV_CONTEXT_OP_READ;

	len = ec_command(dev, EC_CMD_VBNV_CONTEXT, EC_VER_VBNV_CONTEXT,
			 &p, sizeof(uint32_t) + size, block, size);
	if (len != size) {
		log_err("Expected %d bytes, got %d\n", size, len);
		return -EIO;
	}

	return 0;
}

int cros_ec_write_nvdata(struct udevice *dev, const uint8_t *block, int size)
{
	struct ec_params_vbnvcontext p;
	int len;

	if (size != EC_VBNV_BLOCK_SIZE && size != EC_VBNV_BLOCK_SIZE_V2)
		return -EINVAL;
	p.op = EC_VBNV_CONTEXT_OP_WRITE;
	memcpy(p.block, block, size);

	len = ec_command_inptr(dev, EC_CMD_VBNV_CONTEXT, EC_VER_VBNV_CONTEXT,
			&p, sizeof(uint32_t) + size, NULL, 0);
	if (len < 0)
		return -1;

	return 0;
}

int cros_ec_battery_cutoff(struct udevice *dev, uint8_t flags)
{
	struct ec_params_battery_cutoff p;
	int len;

	p.flags = flags;
	len = ec_command(dev, EC_CMD_BATTERY_CUT_OFF, 1, &p, sizeof(p),
			 NULL, 0);

	if (len < 0)
		return -1;
	return 0;
}

int cros_ec_set_ldo(struct udevice *dev, uint8_t index, uint8_t state)
{
	struct ec_params_ldo_set params;

	params.index = index;
	params.state = state;

	if (ec_command_inptr(dev, EC_CMD_LDO_SET, 0, &params, sizeof(params),
			     NULL, 0))
		return -1;

	return 0;
}

int cros_ec_get_ldo(struct udevice *dev, uint8_t index, uint8_t *state)
{
	struct ec_params_ldo_get params;
	struct ec_response_ldo_get *resp;

	params.index = index;

	if (ec_command_inptr(dev, EC_CMD_LDO_GET, 0, &params, sizeof(params),
			     (uint8_t **)&resp, sizeof(*resp)) !=
			     sizeof(*resp))
		return -1;

	*state = resp->state;

	return 0;
}

int cros_ec_register(struct udevice *dev)
{
	struct cros_ec_dev *cdev = dev_get_uclass_priv(dev);
	char id[MSG_BYTES];

	cdev->dev = dev;
	gpio_request_by_name(dev, "ec-interrupt", 0, &cdev->ec_int,
			     GPIOD_IS_IN);
	cdev->optimise_flash_write = dev_read_bool(dev, "optimise-flash-write");

	if (cros_ec_check_version(dev)) {
		debug("%s: Could not detect CROS-EC version\n", __func__);
		return -CROS_EC_ERR_CHECK_VERSION;
	}

	if (cros_ec_read_id(dev, id, sizeof(id))) {
		debug("%s: Could not read KBC ID\n", __func__);
		return -CROS_EC_ERR_READ_ID;
	}

	/* Remember this device for use by the cros_ec command */
	debug("Google Chrome EC v%d CROS-EC driver ready, id '%s'\n",
	      cdev->protocol_version, id);

	return 0;
}

int cros_ec_decode_ec_flash(struct udevice *dev, struct fdt_cros_ec *config)
{
	ofnode flash_node, node;

	flash_node = dev_read_subnode(dev, "flash");
	if (!ofnode_valid(flash_node)) {
		debug("Failed to find flash node\n");
		return -1;
	}

	if (ofnode_read_fmap_entry(flash_node,  &config->flash)) {
		debug("Failed to decode flash node in chrome-ec\n");
		return -1;
	}

	config->flash_erase_value = ofnode_read_s32_default(flash_node,
							    "erase-value", -1);
	ofnode_for_each_subnode(node, flash_node) {
		const char *name = ofnode_get_name(node);
		enum ec_flash_region region;

		if (0 == strcmp(name, "ro")) {
			region = EC_FLASH_REGION_RO;
		} else if (0 == strcmp(name, "rw")) {
			region = EC_FLASH_REGION_ACTIVE;
		} else if (0 == strcmp(name, "wp-ro")) {
			region = EC_FLASH_REGION_WP_RO;
		} else {
			debug("Unknown EC flash region name '%s'\n", name);
			return -1;
		}

		if (ofnode_read_fmap_entry(node, &config->region[region])) {
			debug("Failed to decode flash region in chrome-ec'\n");
			return -1;
		}
	}

	return 0;
}

int cros_ec_i2c_tunnel(struct udevice *dev, int port, struct i2c_msg *in,
		       int nmsgs)
{
	union {
		struct ec_params_i2c_passthru p;
		uint8_t outbuf[EC_PROTO2_MAX_PARAM_SIZE];
	} params;
	union {
		struct ec_response_i2c_passthru r;
		uint8_t inbuf[EC_PROTO2_MAX_PARAM_SIZE];
	} response;
	struct ec_params_i2c_passthru *p = &params.p;
	struct ec_response_i2c_passthru *r = &response.r;
	struct ec_params_i2c_passthru_msg *msg;
	uint8_t *pdata, *read_ptr = NULL;
	int read_len;
	int size;
	int rv;
	int i;

	p->port = port;

	p->num_msgs = nmsgs;
	size = sizeof(*p) + p->num_msgs * sizeof(*msg);

	/* Create a message to write the register address and optional data */
	pdata = (uint8_t *)p + size;

	read_len = 0;
	for (i = 0, msg = p->msg; i < nmsgs; i++, msg++, in++) {
		bool is_read = in->flags & I2C_M_RD;

		msg->addr_flags = in->addr;
		msg->len = in->len;
		if (is_read) {
			msg->addr_flags |= EC_I2C_FLAG_READ;
			read_len += in->len;
			read_ptr = in->buf;
			if (sizeof(*r) + read_len > sizeof(response)) {
				puts("Read length too big for buffer\n");
				return -1;
			}
		} else {
			if (pdata - (uint8_t *)p + in->len > sizeof(params)) {
				puts("Params too large for buffer\n");
				return -1;
			}
			memcpy(pdata, in->buf, in->len);
			pdata += in->len;
		}
	}

	rv = ec_command(dev, EC_CMD_I2C_PASSTHRU, 0, p, pdata - (uint8_t *)p,
			r, sizeof(*r) + read_len);
	if (rv < 0)
		return rv;

	/* Parse response */
	if (r->i2c_status & EC_I2C_STATUS_ERROR) {
		printf("Transfer failed with status=0x%x\n", r->i2c_status);
		return -1;
	}

	if (rv < sizeof(*r) + read_len) {
		puts("Truncated read response\n");
		return -1;
	}

	/* We only support a single read message for each transfer */
	if (read_len)
		memcpy(read_ptr, r->data, read_len);

	return 0;
}

int cros_ec_check_feature(struct udevice *dev, int feature)
{
	struct ec_response_get_features r;
	int rv;

	rv = ec_command(dev, EC_CMD_GET_FEATURES, 0, &r, sizeof(r), NULL, 0);
	if (rv)
		return rv;

	if (feature >= 8 * sizeof(r.flags))
		return -1;

	return r.flags[feature / 32] & EC_FEATURE_MASK_0(feature);
}

/*
 * Query the EC for specified mask indicating enabled events.
 * The EC maintains separate event masks for SMI, SCI and WAKE.
 */
static int cros_ec_uhepi_cmd(struct udevice *dev, uint mask, uint action,
			     uint64_t *value)
{
	int ret;
	struct ec_params_host_event req;
	struct ec_response_host_event rsp;

	req.action = action;
	req.mask_type = mask;
	if (action != EC_HOST_EVENT_GET)
		req.value = *value;
	else
		*value = 0;
	ret = ec_command(dev, EC_CMD_HOST_EVENT, 0, &req, sizeof(req), &rsp,
			 sizeof(rsp));

	if (action != EC_HOST_EVENT_GET)
		return ret;
	if (ret == 0)
		*value = rsp.value;

	return ret;
}

static int cros_ec_handle_non_uhepi_cmd(struct udevice *dev, uint hcmd,
					uint action, uint64_t *value)
{
	int ret = -1;
	struct ec_params_host_event_mask req;
	struct ec_response_host_event_mask rsp;

	if (hcmd == INVALID_HCMD)
		return ret;

	if (action != EC_HOST_EVENT_GET)
		req.mask = (uint32_t)*value;
	else
		*value = 0;

	ret = ec_command(dev, hcmd, 0, &req, sizeof(req), &rsp, sizeof(rsp));
	if (action != EC_HOST_EVENT_GET)
		return ret;
	if (ret == 0)
		*value = rsp.mask;

	return ret;
}

bool cros_ec_is_uhepi_supported(struct udevice *dev)
{
#define UHEPI_SUPPORTED 1
#define UHEPI_NOT_SUPPORTED 2
	static int uhepi_support;

	if (!uhepi_support) {
		uhepi_support = cros_ec_check_feature(dev,
			EC_FEATURE_UNIFIED_WAKE_MASKS) > 0 ? UHEPI_SUPPORTED :
			UHEPI_NOT_SUPPORTED;
		log_debug("Chrome EC: UHEPI %s\n",
			  uhepi_support == UHEPI_SUPPORTED ? "supported" :
			  "not supported");
	}
	return uhepi_support == UHEPI_SUPPORTED;
}

static int cros_ec_get_mask(struct udevice *dev, uint type)
{
	u64 value = 0;

	if (cros_ec_is_uhepi_supported(dev)) {
		cros_ec_uhepi_cmd(dev, type, EC_HOST_EVENT_GET, &value);
	} else {
		assert(type < ARRAY_SIZE(event_map));
		cros_ec_handle_non_uhepi_cmd(dev, event_map[type].get_cmd,
					     EC_HOST_EVENT_GET, &value);
	}
	return value;
}

static int cros_ec_clear_mask(struct udevice *dev, uint type, u64 mask)
{
	if (cros_ec_is_uhepi_supported(dev))
		return cros_ec_uhepi_cmd(dev, type, EC_HOST_EVENT_CLEAR, &mask);

	assert(type < ARRAY_SIZE(event_map));

	return cros_ec_handle_non_uhepi_cmd(dev, event_map[type].clear_cmd,
					    EC_HOST_EVENT_CLEAR, &mask);
}

uint64_t cros_ec_get_events_b(struct udevice *dev)
{
	return cros_ec_get_mask(dev, EC_HOST_EVENT_B);
}

int cros_ec_clear_events_b(struct udevice *dev, uint64_t mask)
{
	log_debug("Chrome EC: clear events_b mask to 0x%016llx\n", mask);

	return cros_ec_clear_mask(dev, EC_HOST_EVENT_B, mask);
}

int cros_ec_read_limit_power(struct udevice *dev, int *limit_powerp)
{
	struct ec_params_charge_state p;
	struct ec_response_charge_state r;
	int ret;

	p.cmd = CHARGE_STATE_CMD_GET_PARAM;
	p.get_param.param = CS_PARAM_LIMIT_POWER;
	ret = ec_command(dev, EC_CMD_CHARGE_STATE, 0, &p, sizeof(p),
			 &r, sizeof(r));

	/*
	 * If our EC doesn't support the LIMIT_POWER parameter, assume that
	 * LIMIT_POWER is not requested.
	 */
	if (ret == -EC_RES_INVALID_PARAM || ret == -EC_RES_INVALID_COMMAND) {
		log_warning("PARAM_LIMIT_POWER not supported by EC\n");
		return -ENOSYS;
	}

	if (ret != sizeof(r.get_param))
		return -EINVAL;

	*limit_powerp = r.get_param.value;
	return 0;
}

int cros_ec_config_powerbtn(struct udevice *dev, uint32_t flags)
{
	struct ec_params_config_power_button params;
	int ret;

	params.flags = flags;
	ret = ec_command(dev, EC_CMD_CONFIG_POWER_BUTTON, 0,
			 &params, sizeof(params), NULL, 0);
	if (ret < 0)
		return ret;

	return 0;
}

int cros_ec_get_lid_shutdown_mask(struct udevice *dev)
{
	u32 mask;
	int ret;

	ret = cros_ec_get_event_mask(dev, EC_CMD_HOST_EVENT_GET_SMI_MASK,
				     &mask);
	if (ret < 0)
		return ret;

	return !!(mask & EC_HOST_EVENT_MASK(EC_HOST_EVENT_LID_CLOSED));
}

int cros_ec_set_lid_shutdown_mask(struct udevice *dev, int enable)
{
	u32 mask;
	int ret;

	ret = cros_ec_get_event_mask(dev, EC_CMD_HOST_EVENT_GET_SMI_MASK,
				     &mask);
	if (ret < 0)
		return ret;

	/* Set lid close event state in the EC SMI event mask */
	if (enable)
		mask |= EC_HOST_EVENT_MASK(EC_HOST_EVENT_LID_CLOSED);
	else
		mask &= ~EC_HOST_EVENT_MASK(EC_HOST_EVENT_LID_CLOSED);

	ret = cros_ec_set_event_mask(dev, EC_CMD_HOST_EVENT_SET_SMI_MASK, mask);
	if (ret < 0)
		return ret;

	printf("EC: %sabled lid close event\n", enable ? "en" : "dis");
	return 0;
}

UCLASS_DRIVER(cros_ec) = {
	.id		= UCLASS_CROS_EC,
	.name		= "cros-ec",
	.per_device_auto_alloc_size = sizeof(struct cros_ec_dev),
	.post_bind	= dm_scan_fdt_dev,
	.flags		= DM_UC_FLAG_ALLOC_PRIV_DMA,
};
