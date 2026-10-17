// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020, Linaro Limited
 *
 * Based on the Linux TIS core interface and U-Boot original SPI TPM driver
 */

#include <common.h>
#include <dm.h>
#include <tpm-v2.h>
#include <linux/delay.h>
#include <linux/unaligned/be_byteshift.h>
#include "tpm_tis.h"

int tpm_tis_get_desc(struct udevice *dev, char *buf, int size)
{
	struct tpm_chip *chip = dev_get_priv(dev);

	if (size < 80)
		return -ENOSPC;

	return snprintf(buf, size,
			"%s v2.0: VendorID 0x%04x, DeviceID 0x%04x, RevisionID 0x%02x [%s]",
			dev->name, chip->vend_dev & 0xFFFF,
			chip->vend_dev >> 16, chip->rid,
			(chip->is_open ? "open" : "closed"));
}

/**
 * tpm_tis_check_locality - Check the current TPM locality
 *
 * @dev: TPM device
 * @loc:  locality
 *
 * Return: True if the tested locality matches
 */
static bool tpm_tis_check_locality(struct udevice *dev, int loc)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	struct tpm_tis_phy_ops *phy_ops = chip->phy_ops;
	u8 locality;

	phy_ops->read_bytes(dev, TPM_ACCESS(loc), 1, &locality);
	if ((locality & (TPM_ACCESS_ACTIVE_LOCALITY | TPM_ACCESS_VALID |
	    TPM_ACCESS_REQUEST_USE)) ==
	    (TPM_ACCESS_ACTIVE_LOCALITY | TPM_ACCESS_VALID)) {
		chip->locality = loc;
		return true;
	}

	return false;
}

/**
 * tpm_tis_request_locality - Request a locality from the TPM
 *
 * @dev:  TPM device
 * @loc:  requested locality
 *
 * Return: 0 on success -1 on failure
 */
int tpm_tis_request_locality(struct udevice *dev, int loc)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	struct tpm_tis_phy_ops *phy_ops = chip->phy_ops;
	u8 buf = TPM_ACCESS_REQUEST_USE;
	unsigned long start, stop;

	if (tpm_tis_check_locality(dev, loc))
		return 0;

	phy_ops->write_bytes(dev, TPM_ACCESS(loc), 1, &buf);
	start = get_timer(0);
	stop = chip->timeout_a;
	do {
		if (tpm_tis_check_locality(dev, loc))
			return 0;
		mdelay(TPM_TIMEOUT_MS);
	} while (get_timer(start) < stop);

	return -1;
}

/**
 * tpm_tis_status - Check the current device status
 *
 * @dev:   TPM device
 * @status: return value of status
 *
 * Return: 0 on success, negative on failure
 */
static int tpm_tis_status(struct udevice *dev, u8 *status)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	struct tpm_tis_phy_ops *phy_ops = chip->phy_ops;

	if (chip->locality < 0)
		return -EINVAL;

	phy_ops->read_bytes(dev, TPM_STS(chip->locality), 1, status);

	if ((*status & TPM_STS_READ_ZERO)) {
		log_err("TPM returned invalid status\n");
		return -EINVAL;
	}

	return 0;
}

/**
 * tpm_tis_release_locality - Release the requested locality
 *
 * @dev: TPM device
 * @loc:  requested locality
 *
 * Return: 0 on success, negative on failure
 */
int tpm_tis_release_locality(struct udevice *dev, int loc)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	struct tpm_tis_phy_ops *phy_ops = chip->phy_ops;
	u8 buf = TPM_ACCESS_ACTIVE_LOCALITY;
	int ret;

	if (chip->locality < 0)
		return 0;

	ret = phy_ops->write_bytes(dev, TPM_ACCESS(loc), 1, &buf);
	chip->locality = -1;

	return ret;
}

/**
 * tpm_tis_wait_for_stat - Wait for TPM to become ready
 *
 * @dev:     TPM device
 * @mask:    mask to match
 * @timeout: timeout for retries
 * @status:  current status
 *
 * Return: 0 on success, negative on failure
 */
static int tpm_tis_wait_for_stat(struct udevice *dev, u8 mask,
				 unsigned long timeout, u8 *status)
{
	unsigned long start = get_timer(0);
	unsigned long stop = timeout;
	int ret;

	do {
		mdelay(TPM_TIMEOUT_MS);
		ret = tpm_tis_status(dev, status);
		if (ret)
			return ret;

		if ((*status & mask) == mask)
			return 0;
	} while (get_timer(start) < stop);

	return -ETIMEDOUT;
}

/**
 * tpm_tis_get_burstcount - Get the burstcount for the data FIFO
 *
 * @dev:        TPM device
 * @burstcount: current burstcount
 *
 * Return: 0 on success, negative on failure
 */
static int tpm_tis_get_burstcount(struct udevice *dev, size_t *burstcount)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	struct tpm_tis_phy_ops *phy_ops = chip->phy_ops;
	unsigned long start, stop;
	u32 burst;

	if (chip->locality < 0)
		return -EINVAL;

	/* wait for burstcount */
	start = get_timer(0);
	/*
	 * This is the TPMv2 defined timeout. Change this in case you want to
	 * make the driver compatile to TPMv1
	 */
	stop = chip->timeout_a;
	do {
		phy_ops->read32(dev, TPM_STS(chip->locality), &burst);
		*burstcount = (burst >> 8) & 0xFFFF;
		if (*burstcount)
			return 0;

		mdelay(TPM_TIMEOUT_MS);
	} while (get_timer(start) < stop);

	return -ETIMEDOUT;
}

/**
 * tpm_tis_ready - Cancel pending comands and get the device on a ready state
 *
 * @dev: TPM device
 *
 * Return: 0 on success, negative on failure
 */
static int tpm_tis_ready(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	struct tpm_tis_phy_ops *phy_ops = chip->phy_ops;
	u8 data = TPM_STS_COMMAND_READY;

	/* This will cancel any pending commands */
	return phy_ops->write_bytes(dev, TPM_STS(chip->locality), 1, &data);
}

int tpm_tis_send(struct udevice *dev, const u8 *buf, size_t len)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	struct tpm_tis_phy_ops *phy_ops = chip->phy_ops;
	size_t burstcnt, wr_size, sent = 0;
	u8 data = TPM_STS_GO;
	u8 status;
	int ret;

	ret = tpm_tis_request_locality(dev, 0);
	if (ret < 0)
		return -EBUSY;

	ret = tpm_tis_status(dev, &status);
	if (ret)
		goto release_locality;

	if (!(status & TPM_STS_COMMAND_READY)) {
		ret = tpm_tis_ready(dev);
		if (ret) {
			log_err("Can't cancel previous TPM operation\n");
			goto release_locality;
		}
		ret = tpm_tis_wait_for_stat(dev, TPM_STS_COMMAND_READY,
					    chip->timeout_b, &status);
		if (ret) {
			log_err("TPM not ready\n");
			goto release_locality;
		}
	}

	while (len > 0) {
		ret = tpm_tis_get_burstcount(dev, &burstcnt);
		if (ret)
			goto release_locality;

		wr_size = min(len, burstcnt);
		ret = phy_ops->write_bytes(dev, TPM_DATA_FIFO(chip->locality),
					   wr_size, buf + sent);
		if (ret < 0)
			goto release_locality;

		ret = tpm_tis_wait_for_stat(dev, TPM_STS_VALID,
					    chip->timeout_c, &status);
		if (ret)
			goto release_locality;

		sent += wr_size;
		len -= wr_size;
		/* make sure the TPM expects more data */
		if (len && !(status & TPM_STS_DATA_EXPECT)) {
			ret = -EIO;
			goto release_locality;
		}
	}

	/*
	 * Make a final check ensuring everything is ok and the TPM expects no
	 * more data
	 */
	ret = tpm_tis_wait_for_stat(dev, TPM_STS_VALID, chip->timeout_c,
				    &status);
	if (ret)
		goto release_locality;

	if (status & TPM_STS_DATA_EXPECT) {
		ret = -EIO;
		goto release_locality;
	}

	ret = phy_ops->write_bytes(dev, TPM_STS(chip->locality), 1, &data);
	if (ret)
		goto release_locality;

	return sent;

release_locality:
	tpm_tis_ready(dev);
	tpm_tis_release_locality(dev, chip->locality);

	return ret;
}

static int tpm_tis_recv_data(struct udevice *dev, u8 *buf, size_t count)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	struct tpm_tis_phy_ops *phy_ops = chip->phy_ops;
	int size = 0, len, ret;
	size_t burstcnt;
	u8 status;

	while (size < count &&
	       tpm_tis_wait_for_stat(dev, TPM_STS_DATA_AVAIL | TPM_STS_VALID,
				     chip->timeout_c, &status) == 0) {
		ret = tpm_tis_get_burstcount(dev, &burstcnt);
		if (ret)
			return ret;

		len = min_t(int, burstcnt, count - size);
		ret = phy_ops->read_bytes(dev, TPM_DATA_FIFO(chip->locality),
					  len, buf + size);
		if (ret < 0)
			return ret;

		size += len;
	}

	return size;
}

/**
 * tpm_tis_recv - Receive data from a device
 *
 * @dev:  TPM device
 * @buf:  buffer to copy data
 * @size: buffer size
 *
 * Return: bytes read or negative on failure
 */
int tpm_tis_recv(struct udevice *dev, u8 *buf, size_t count)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	int size, expected;

	if (count < TPM_HEADER_SIZE)
		return -E2BIG;

	size = tpm_tis_recv_data(dev, buf, TPM_HEADER_SIZE);
	if (size < TPM_HEADER_SIZE) {
		log_err("TPM error, unable to read header\n");
		goto out;
	}

	expected = get_unaligned_be32(buf + TPM_CMD_COUNT_OFFSET);
	if (expected > count) {
		size = -EIO;
		log_warning("Too much data: %d > %zu\n", expected, count);
		goto out;
	}

	size += tpm_tis_recv_data(dev, &buf[TPM_HEADER_SIZE],
				   expected - TPM_HEADER_SIZE);
	if (size < expected) {
		log(LOGC_NONE, LOGL_ERR,
		    "TPM error, unable to read remaining bytes of result\n");
		size = -EIO;
		goto out;
	}

out:
	tpm_tis_ready(dev);
	/* acquired in tpm_tis_send */
	tpm_tis_release_locality(dev, chip->locality);

	return size;
}

int tpm_tis_cleanup(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	int ret;

	ret = tpm_tis_request_locality(dev, 0);
	if (ret)
		return ret;

	tpm_tis_ready(dev);

	tpm_tis_release_locality(dev, chip->locality);

	return 0;
}

int tpm_tis_open(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	int ret;

	if (chip->is_open)
		return -EBUSY;

	ret = tpm_tis_request_locality(dev, 0);
	if (!ret)
		chip->is_open = 1;

	return ret;
}

void tpm_tis_ops_register(struct udevice *dev, struct tpm_tis_phy_ops *ops)
{
	struct tpm_chip *chip = dev_get_priv(dev);

	chip->phy_ops = ops;
}

static bool tis_check_ops(struct tpm_tis_phy_ops *phy_ops)
{
	if (!phy_ops || !phy_ops->read_bytes || !phy_ops->write_bytes ||
	    !phy_ops->read32 || !phy_ops->write32)
		return false;

	return true;
}

int tpm_tis_init(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	struct tpm_tis_phy_ops *phy_ops = chip->phy_ops;
	int ret;
	u32 tmp;

	if (!tis_check_ops(phy_ops)) {
		log_err("Driver bug. No bus ops defined\n");
		return -1;
	}

	chip->timeout_a = TIS_SHORT_TIMEOUT_MS;
	chip->timeout_b = TIS_LONG_TIMEOUT_MS;
	chip->timeout_c = TIS_SHORT_TIMEOUT_MS;
	chip->timeout_d = TIS_SHORT_TIMEOUT_MS;

	ret = tpm_tis_request_locality(dev, 0);
	if (ret)
		return ret;

	/* Disable interrupts */
	phy_ops->read32(dev, TPM_INT_ENABLE(chip->locality), &tmp);
	tmp |= TPM_INTF_CMD_READY_INT | TPM_INTF_LOCALITY_CHANGE_INT |
	       TPM_INTF_DATA_AVAIL_INT | TPM_INTF_STS_VALID_INT;
	tmp &= ~TPM_GLOBAL_INT_ENABLE;
	phy_ops->write32(dev, TPM_INT_ENABLE(chip->locality), tmp);

	phy_ops->read_bytes(dev, TPM_RID(chip->locality), 1, &chip->rid);
	phy_ops->read32(dev, TPM_DID_VID(chip->locality), &chip->vend_dev);

	return tpm_tis_release_locality(dev, chip->locality);
}

int tpm_tis_close(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	int ret = 0;

	if (chip->is_open) {
		ret = tpm_tis_release_locality(dev, chip->locality);
		chip->is_open = 0;
	}

	return ret;
}
