// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2011 Infineon Technologies
 *
 * Authors:
 * Peter Huewe <huewe.external@infineon.com>
 *
 * Description:
 * Device driver for TCG/TCPA TPM (trusted platform module).
 * Specifications at www.trustedcomputinggroup.org
 *
 * This device driver implements the TPM interface as defined in
 * the TCG TPM Interface Spec version 1.2, revision 1.0 and the
 * Infineon I2C Protocol Stack Specification v0.20.
 *
 * It is based on the Linux kernel driver tpm.c from Leendert van
 * Dorn, Dave Safford, Reiner Sailer, and Kyleen Hall.
 *
 * Version: 2.1.1
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <i2c.h>
#include <tpm-v1.h>
#include <linux/errno.h>
#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/unaligned/be_byteshift.h>

#include "tpm_tis.h"
#include "tpm_internal.h"

enum i2c_chip_type {
	SLB9635,
	SLB9645,
	UNKNOWN,
};

/* expected value for DIDVID register */
#define TPM_TIS_I2C_DID_VID_9635 0x000b15d1L
#define TPM_TIS_I2C_DID_VID_9645 0x001a15d1L

static const char * const chip_name[] = {
	[SLB9635] = "slb9635tt",
	[SLB9645] = "slb9645tt",
	[UNKNOWN] = "unknown/fallback to slb9635",
};

#define	TPM_ACCESS(l)			(0x0000 | ((l) << 4))
#define	TPM_STS(l)			(0x0001 | ((l) << 4))
#define	TPM_DATA_FIFO(l)		(0x0005 | ((l) << 4))
#define	TPM_DID_VID(l)			(0x0006 | ((l) << 4))

/*
 * tpm_tis_i2c_read() - read from TPM register
 * @addr: register address to read from
 * @buffer: provided by caller
 * @len: number of bytes to read
 *
 * Read len bytes from TPM register and put them into
 * buffer (little-endian format, i.e. first byte is put into buffer[0]).
 *
 * NOTE: TPM is big-endian for multi-byte values. Multi-byte
 * values have to be swapped.
 *
 * Return -EIO on error, 0 on success.
 */
static int tpm_tis_i2c_read(struct udevice *dev, u8 addr, u8 *buffer,
			    size_t len)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	int rc;
	int count;
	uint32_t addrbuf = addr;

	if ((chip->chip_type == SLB9635) || (chip->chip_type == UNKNOWN)) {
		/* slb9635 protocol should work in both cases */
		for (count = 0; count < MAX_COUNT; count++) {
			rc = dm_i2c_write(dev, 0, (uchar *)&addrbuf, 1);
			if (rc == 0)
				break;  /* Success, break to skip sleep */
			udelay(SLEEP_DURATION_US);
		}
		if (rc)
			return rc;

		/* After the TPM has successfully received the register address
		 * it needs some time, thus we're sleeping here again, before
		 * retrieving the data
		 */
		for (count = 0; count < MAX_COUNT; count++) {
			udelay(SLEEP_DURATION_US);
			rc = dm_i2c_read(dev, 0, buffer, len);
			if (rc == 0)
				break;  /* success, break to skip sleep */
		}
	} else {
		/*
		 * Use a combined read for newer chips.
		 * Unfortunately the smbus functions are not suitable due to
		 * the 32 byte limit of the smbus.
		 * Retries should usually not be needed, but are kept just to
		 * be safe on the safe side.
		 */
		for (count = 0; count < MAX_COUNT; count++) {
			rc = dm_i2c_read(dev, addr, buffer, len);
			if (rc == 0)
				break;  /* break here to skip sleep */
			udelay(SLEEP_DURATION_US);
		}
	}

	/* Take care of 'guard time' */
	udelay(SLEEP_DURATION_US);
	if (rc)
		return rc;

	return 0;
}

static int tpm_tis_i2c_write_generic(struct udevice *dev, u8 addr,
				     const u8 *buffer, size_t len,
				     unsigned int sleep_time_us, u8 max_count)
{
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);
	struct tpm_chip *chip = dev_get_priv(dev);
	int rc = 0;
	int count;

	if (chip->chip_type == SLB9635) {
		/* Prepare send buffer to include the address */
		priv->buf[0] = addr;
		memcpy(&(priv->buf[1]), buffer, len);
		buffer = priv->buf;
		len++;
		addr = 0;
	}

	for (count = 0; count < max_count; count++) {
		rc = dm_i2c_write(dev, addr, buffer, len);
		if (rc == 0)
			break;  /* Success, break to skip sleep */
		udelay(sleep_time_us);
	}

	/* take care of 'guard time' */
	udelay(sleep_time_us);
	if (rc)
		return rc;

	return 0;
}

/*
 * tpm_tis_i2c_write() - write to TPM register
 * @addr: register address to write to
 * @buffer: containing data to be written
 * @len: number of bytes to write
 *
 * Write len bytes from provided buffer to TPM register (little
 * endian format, i.e. buffer[0] is written as first byte).
 *
 * NOTE: TPM is big-endian for multi-byte values. Multi-byte
 * values have to be swapped.
 *
 * NOTE: use this function instead of the tpm_tis_i2c_write_generic function.
 *
 * Return -EIO on error, 0 on success
 */
static int tpm_tis_i2c_write(struct udevice *dev, u8 addr, const u8 *buffer,
			     size_t len)
{
	return tpm_tis_i2c_write_generic(dev, addr, buffer, len,
					 SLEEP_DURATION_US, MAX_COUNT);
}

/*
 * This function is needed especially for the cleanup situation after
 * sending TPM_READY
 */
static int tpm_tis_i2c_write_long(struct udevice *dev, u8 addr, u8 *buffer,
				  size_t len)
{
	return tpm_tis_i2c_write_generic(dev, addr, buffer, len,
					 SLEEP_DURATION_LONG_US,
					 MAX_COUNT_LONG);
}

static int tpm_tis_i2c_check_locality(struct udevice *dev, int loc)
{
	const u8 mask = TPM_ACCESS_ACTIVE_LOCALITY | TPM_ACCESS_VALID;
	struct tpm_chip *chip = dev_get_priv(dev);
	u8 buf;
	int rc;

	rc = tpm_tis_i2c_read(dev, TPM_ACCESS(loc), &buf, 1);
	if (rc < 0)
		return rc;

	if ((buf & mask) == mask) {
		chip->locality = loc;
		return loc;
	}

	return -ENOENT;
}

static void tpm_tis_i2c_release_locality(struct udevice *dev, int loc,
					 int force)
{
	const u8 mask = TPM_ACCESS_REQUEST_PENDING | TPM_ACCESS_VALID;
	u8 buf;

	if (tpm_tis_i2c_read(dev, TPM_ACCESS(loc), &buf, 1) < 0)
		return;

	if (force || (buf & mask) == mask) {
		buf = TPM_ACCESS_ACTIVE_LOCALITY;
		tpm_tis_i2c_write(dev, TPM_ACCESS(loc), &buf, 1);
	}
}

static int tpm_tis_i2c_request_locality(struct udevice *dev, int loc)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	unsigned long start, stop;
	u8 buf = TPM_ACCESS_REQUEST_USE;
	int rc;

	rc = tpm_tis_i2c_check_locality(dev, loc);
	if (rc >= 0) {
		debug("%s: Already have locality\n", __func__);
		return loc;  /* We already have the locality */
	} else if (rc != -ENOENT) {
		debug("%s: Failed to get locality: %d\n", __func__, rc);
		return rc;
	}

	rc = tpm_tis_i2c_write(dev, TPM_ACCESS(loc), &buf, 1);
	if (rc) {
		debug("%s: Failed to write to TPM: %d\n", __func__, rc);
		return rc;
	}

	/* Wait for burstcount */
	start = get_timer(0);
	stop = chip->timeout_a;
	do {
		rc = tpm_tis_i2c_check_locality(dev, loc);
		if (rc >= 0) {
			debug("%s: Have locality\n", __func__);
			return loc;
		} else if (rc != -ENOENT) {
			debug("%s: Failed to get locality: %d\n", __func__, rc);
			return rc;
		}
		mdelay(TPM_TIMEOUT_MS);
	} while (get_timer(start) < stop);
	debug("%s: Timeout getting locality: %d\n", __func__, rc);

	return rc;
}

static u8 tpm_tis_i2c_status(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	/* NOTE: Since i2c read may fail, return 0 in this case --> time-out */
	u8 buf;

	if (tpm_tis_i2c_read(dev, TPM_STS(chip->locality), &buf, 1) < 0)
		return 0;
	else
		return buf;
}

static int tpm_tis_i2c_ready(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	int rc;

	/* This causes the current command to be aborted */
	u8 buf = TPM_STS_COMMAND_READY;

	debug("%s\n", __func__);
	rc = tpm_tis_i2c_write_long(dev, TPM_STS(chip->locality), &buf, 1);
	if (rc)
		debug("%s: rc=%d\n", __func__, rc);

	return rc;
}

static ssize_t tpm_tis_i2c_get_burstcount(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	unsigned long start, stop;
	ssize_t burstcnt;
	u8 addr, buf[3];

	/* Wait for burstcount */
	/* XXX: Which timeout value? Spec has 2 answers (c & d) */
	start = get_timer(0);
	stop = chip->timeout_d;
	do {
		/* Note: STS is little endian */
		addr = TPM_STS(chip->locality) + 1;
		if (tpm_tis_i2c_read(dev, addr, buf, 3) < 0)
			burstcnt = 0;
		else
			burstcnt = (buf[2] << 16) + (buf[1] << 8) + buf[0];

		if (burstcnt)
			return burstcnt;
		mdelay(TPM_TIMEOUT_MS);
	} while (get_timer(start) < stop);

	return -EBUSY;
}

static int tpm_tis_i2c_wait_for_stat(struct udevice *dev, u8 mask,
				     unsigned long timeout, int *status)
{
	unsigned long start, stop;

	/* Check current status */
	*status = tpm_tis_i2c_status(dev);
	if ((*status & mask) == mask)
		return 0;

	start = get_timer(0);
	stop = timeout;
	do {
		mdelay(TPM_TIMEOUT_MS);
		*status = tpm_tis_i2c_status(dev);
		if ((*status & mask) == mask)
			return 0;
	} while (get_timer(start) < stop);

	return -ETIMEDOUT;
}

static int tpm_tis_i2c_recv_data(struct udevice *dev, u8 *buf, size_t count)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	size_t size = 0;
	ssize_t burstcnt;
	int rc;

	while (size < count) {
		burstcnt = tpm_tis_i2c_get_burstcount(dev);

		/* burstcount < 0 -> tpm is busy */
		if (burstcnt < 0)
			return burstcnt;

		/* Limit received data to max left */
		if (burstcnt > (count - size))
			burstcnt = count - size;

		rc = tpm_tis_i2c_read(dev, TPM_DATA_FIFO(chip->locality),
				      &(buf[size]), burstcnt);
		if (rc == 0)
			size += burstcnt;
	}

	return size;
}

static int tpm_tis_i2c_recv(struct udevice *dev, u8 *buf, size_t count)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	int size = 0;
	int status;
	unsigned int expected;
	int rc;

	status = tpm_tis_i2c_status(dev);
	if (status == TPM_STS_COMMAND_READY)
		return -EINTR;
	if ((status & (TPM_STS_DATA_AVAIL | TPM_STS_VALID)) !=
	    (TPM_STS_DATA_AVAIL | TPM_STS_VALID))
		return -EAGAIN;

	debug("...got it;\n");

	/* Read first 10 bytes, including tag, paramsize, and result */
	size = tpm_tis_i2c_recv_data(dev, buf, TPM_HEADER_SIZE);
	if (size < TPM_HEADER_SIZE) {
		debug("Unable to read header\n");
		return size < 0 ? size : -EIO;
	}

	expected = get_unaligned_be32(buf + TPM_RSP_SIZE_BYTE);
	if ((size_t)expected > count || (size_t)expected < TPM_HEADER_SIZE) {
		debug("Error size=%x, expected=%x, count=%x\n", size, expected,
		      count);
		return -ENOSPC;
	}

	size += tpm_tis_i2c_recv_data(dev, &buf[TPM_HEADER_SIZE],
				      expected - TPM_HEADER_SIZE);
	if (size < expected) {
		debug("Unable to read remainder of result\n");
		return -ETIMEDOUT;
	}

	rc = tpm_tis_i2c_wait_for_stat(dev, TPM_STS_VALID, chip->timeout_c,
				       &status);
	if (rc)
		return rc;
	if (status & TPM_STS_DATA_AVAIL) {  /* Retry? */
		debug("Error left over data\n");
		return -EIO;
	}

	return size;
}

static int tpm_tis_i2c_send(struct udevice *dev, const u8 *buf, size_t len)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	int rc, status;
	size_t burstcnt;
	size_t count = 0;
	int retry = 0;
	u8 sts = TPM_STS_GO;

	debug("%s: len=%d\n", __func__, len);
	if (len > TPM_DEV_BUFSIZE)
		return -E2BIG;  /* Command is too long for our tpm, sorry */

	if (tpm_tis_i2c_request_locality(dev, 0) < 0)
		return -EBUSY;

	status = tpm_tis_i2c_status(dev);
	if ((status & TPM_STS_COMMAND_READY) == 0) {
		rc = tpm_tis_i2c_ready(dev);
		if (rc)
			return rc;
		rc = tpm_tis_i2c_wait_for_stat(dev, TPM_STS_COMMAND_READY,
					       chip->timeout_b, &status);
		if (rc)
			return rc;
	}

	burstcnt = tpm_tis_i2c_get_burstcount(dev);

	/* burstcount < 0 -> tpm is busy */
	if (burstcnt < 0)
		return burstcnt;

	while (count < len) {
		udelay(300);
		if (burstcnt > len - count)
			burstcnt = len - count;

#ifdef CONFIG_TPM_TIS_I2C_BURST_LIMITATION
		if (retry && burstcnt > CONFIG_TPM_TIS_I2C_BURST_LIMITATION_LEN)
			burstcnt = CONFIG_TPM_TIS_I2C_BURST_LIMITATION_LEN;
#endif /* CONFIG_TPM_TIS_I2C_BURST_LIMITATION */

		rc = tpm_tis_i2c_write(dev, TPM_DATA_FIFO(chip->locality),
				       &(buf[count]), burstcnt);
		if (rc == 0)
			count += burstcnt;
		else {
			debug("%s: error\n", __func__);
			if (retry++ > 10)
				return -EIO;
			rc = tpm_tis_i2c_wait_for_stat(dev, TPM_STS_VALID,
						       chip->timeout_c,
						       &status);
			if (rc)
				return rc;

			if ((status & TPM_STS_DATA_EXPECT) == 0)
				return -EIO;
		}
	}

	/* Go and do it */
	rc = tpm_tis_i2c_write(dev, TPM_STS(chip->locality), &sts, 1);
	if (rc < 0)
		return rc;
	debug("%s: done, rc=%d\n", __func__, rc);

	return len;
}

static int tpm_tis_i2c_cleanup(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);

	tpm_tis_i2c_ready(dev);
	/*
	 * The TPM needs some time to clean up here,
	 * so we sleep rather than keeping the bus busy
	 */
	mdelay(2);
	tpm_tis_i2c_release_locality(dev, chip->locality, 0);

	return 0;
}

static int tpm_tis_i2c_init(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	u32 vendor;
	u32 expected_did_vid;
	int rc;

	chip->is_open = 1;

	/* Default timeouts - these could move to the device tree */
	chip->timeout_a = TIS_SHORT_TIMEOUT_MS;
	chip->timeout_b = TIS_LONG_TIMEOUT_MS;
	chip->timeout_c = TIS_SHORT_TIMEOUT_MS;
	chip->timeout_d = TIS_SHORT_TIMEOUT_MS;

	rc = tpm_tis_i2c_request_locality(dev, 0);
	if (rc < 0)
		return rc;

	/* Read four bytes from DID_VID register */
	if (tpm_tis_i2c_read(dev, TPM_DID_VID(0), (uchar *)&vendor, 4) < 0) {
		tpm_tis_i2c_release_locality(dev, 0, 1);
		return -EIO;
	}

	if (chip->chip_type == SLB9635) {
		vendor = be32_to_cpu(vendor);
		expected_did_vid = TPM_TIS_I2C_DID_VID_9635;
	} else {
		/* device id and byte order has changed for newer i2c tpms */
		expected_did_vid = TPM_TIS_I2C_DID_VID_9645;
	}

	if (chip->chip_type != UNKNOWN && vendor != expected_did_vid) {
		pr_err("Vendor id did not match! ID was %08x\n", vendor);
		return -ENODEV;
	}

	chip->vend_dev = vendor;
	debug("1.2 TPM (chip type %s device-id 0x%X)\n",
	      chip_name[chip->chip_type], vendor >> 16);

	/*
	 * A timeout query to TPM can be placed here.
	 * Standard timeout values are used so far
	 */

	return 0;
}

static int tpm_tis_i2c_open(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	int rc;

	debug("%s: start\n", __func__);
	if (chip->is_open)
		return -EBUSY;
	rc = tpm_tis_i2c_init(dev);
	if (rc < 0)
		chip->is_open = 0;

	return rc;
}

static int tpm_tis_i2c_close(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);

	if (chip->is_open) {
		tpm_tis_i2c_release_locality(dev, chip->locality, 1);
		chip->is_open = 0;
		chip->vend_dev = 0;
	}

	return 0;
}

static int tpm_tis_get_desc(struct udevice *dev, char *buf, int size)
{
	struct tpm_chip *chip = dev_get_priv(dev);

	if (size < 50)
		return -ENOSPC;

	return snprintf(buf, size, "1.2 TPM (%s, chip type %s device-id 0x%x)",
			chip->is_open ? "open" : "closed",
			chip_name[chip->chip_type],
			chip->vend_dev >> 16);
}

static int tpm_tis_i2c_probe(struct udevice *dev)
{
	struct tpm_chip_priv *uc_priv = dev_get_uclass_priv(dev);
	struct tpm_chip *chip = dev_get_priv(dev);

	chip->chip_type = dev_get_driver_data(dev);

	/* TODO: These need to be checked and tuned */
	uc_priv->duration_ms[TPM_SHORT] = TIS_SHORT_TIMEOUT_MS;
	uc_priv->duration_ms[TPM_MEDIUM] = TIS_LONG_TIMEOUT_MS;
	uc_priv->duration_ms[TPM_LONG] = TIS_LONG_TIMEOUT_MS;
	uc_priv->retry_time_ms = TPM_TIMEOUT_MS;

	return 0;
}

static const struct tpm_ops tpm_tis_i2c_ops = {
	.open		= tpm_tis_i2c_open,
	.close		= tpm_tis_i2c_close,
	.get_desc	= tpm_tis_get_desc,
	.send		= tpm_tis_i2c_send,
	.recv		= tpm_tis_i2c_recv,
	.cleanup	= tpm_tis_i2c_cleanup,
};

static const struct udevice_id tpm_tis_i2c_ids[] = {
	{ .compatible = "infineon,slb9635tt", .data = SLB9635 },
	{ .compatible = "infineon,slb9645tt", .data = SLB9645 },
	{ }
};

U_BOOT_DRIVER(tpm_tis_i2c) = {
	.name   = "tpm_tis_infineon",
	.id     = UCLASS_TPM,
	.of_match = tpm_tis_i2c_ids,
	.ops    = &tpm_tis_i2c_ops,
	.probe	= tpm_tis_i2c_probe,
	.priv_auto_alloc_size = sizeof(struct tpm_chip),
};
