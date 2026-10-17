// SPDX-License-Identifier: GPL-2.0+
/*
 * STMicroelectronics TPM ST33ZP24 I2C UBOOT driver
 *
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author(s): Christophe Ricard <christophe-h.ricard@st.com> for STMicroelectronics.
 *
 * Description: Device driver for ST33ZP24 I2C TPM TCG.
 *
 * This device driver implements the TPM interface as defined in
 * the TCG TPM Interface Spec version 1.21, revision 1.0 and the
 * STMicroelectronics Protocol Stack Specification version 1.2.0.
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <i2c.h>
#include <log.h>
#include <tpm-v1.h>
#include <errno.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <asm/unaligned.h>

#include "tpm_tis.h"
#include "tpm_internal.h"

#define TPM_ACCESS			0x0
#define TPM_STS				0x18
#define TPM_DATA_FIFO			0x24

#define LOCALITY0			0

#define TPM_DUMMY_BYTE			0xAA
#define TPM_ST33ZP24_I2C_SLAVE_ADDR	0x13

#define TPM_WRITE_DIRECTION             0x80

/*
 * st33zp24_i2c_write8_reg
 * Send byte to the TIS register according to the ST33ZP24 I2C protocol.
 * @param: tpm_register, the tpm tis register where the data should be written
 * @param: tpm_data, the tpm_data to write inside the tpm_register
 * @param: tpm_size, The length of the data
 * @return: Number of byte written successfully else an error code.
 */
static int st33zp24_i2c_write8_reg(struct udevice *dev, u8 tpm_register,
				   const u8 *tpm_data, size_t tpm_size)
{
	struct tpm_chip_priv *chip_priv = dev_get_uclass_priv(dev);

	chip_priv->buf[0] = tpm_register;
	memcpy(chip_priv->buf + 1, tpm_data, tpm_size);

	return dm_i2c_write(dev, 0, chip_priv->buf, tpm_size + 1);
}

/*
* st33zp24_i2c_read8_reg
* Recv byte from the TIS register according to the ST33ZP24 I2C protocol.
* @param: tpm_register, the tpm tis register where the data should be read
* @param: tpm_data, the TPM response
* @param: tpm_size, tpm TPM response size to read.
* @return: Number of byte read successfully else an error code.
*/
static int st33zp24_i2c_read8_reg(struct udevice *dev, u8 tpm_register,
				  u8 *tpm_data, size_t tpm_size)
{
	int status;
	u8 data;

	data = TPM_DUMMY_BYTE;
	status = st33zp24_i2c_write8_reg(dev, tpm_register, &data, 1);
	if (status < 0)
		return status;

	return dm_i2c_read(dev, 0, tpm_data, tpm_size);
}

/*
 * st33zp24_i2c_write
 * Send byte to the TIS register according to the ST33ZP24 I2C protocol.
 * @param: phy_id, the phy description
 * @param: tpm_register, the tpm tis register where the data should be written
 * @param: tpm_data, the tpm_data to write inside the tpm_register
 * @param: tpm_size, the length of the data
 * @return: number of byte written successfully: should be one if success.
 */
static int st33zp24_i2c_write(struct udevice *dev, u8 tpm_register,
			      const u8 *tpm_data, size_t tpm_size)
{
	return st33zp24_i2c_write8_reg(dev, tpm_register | TPM_WRITE_DIRECTION,
				       tpm_data, tpm_size);
}

/*
 * st33zp24_i2c_read
 * Recv byte from the TIS register according to the ST33ZP24 I2C protocol.
 * @param: phy_id, the phy description
 * @param: tpm_register, the tpm tis register where the data should be read
 * @param: tpm_data, the TPM response
 * @param: tpm_size, tpm TPM response size to read.
 * @return: number of byte read successfully: should be one if success.
 */
static int st33zp24_i2c_read(struct udevice *dev, u8 tpm_register,
			     u8 *tpm_data, size_t tpm_size)
{
	return st33zp24_i2c_read8_reg(dev, tpm_register, tpm_data, tpm_size);
}

/*
 * st33zp24_i2c_release_locality release the active locality
 * @param: chip, the tpm chip description.
 */
static void st33zp24_i2c_release_locality(struct udevice *dev)
{
	u8 data = TPM_ACCESS_ACTIVE_LOCALITY;

	st33zp24_i2c_write(dev, TPM_ACCESS, &data, 1);
}

/*
 * st33zp24_i2c_check_locality if the locality is active
 * @param: chip, the tpm chip description
 * @return: the active locality or -EACCES.
 */
static int st33zp24_i2c_check_locality(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	u8 data;
	u8 status;

	status = st33zp24_i2c_read(dev, TPM_ACCESS, &data, 1);
	if (!status && (data &
		(TPM_ACCESS_ACTIVE_LOCALITY | TPM_ACCESS_VALID)) ==
		(TPM_ACCESS_ACTIVE_LOCALITY | TPM_ACCESS_VALID))
		return chip->locality;

	return -EACCES;
}

/*
 * st33zp24_i2c_request_locality request the TPM locality
 * @param: chip, the chip description
 * @return: the active locality or negative value.
 */
static int st33zp24_i2c_request_locality(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	unsigned long start, stop;
	long ret;
	u8 data;

	if (st33zp24_i2c_check_locality(dev) == chip->locality)
		return chip->locality;

	data = TPM_ACCESS_REQUEST_USE;
	ret = st33zp24_i2c_write(dev, TPM_ACCESS, &data, 1);
	if (ret < 0)
		return ret;

	/* wait for locality activated */
	start = get_timer(0);
	stop = chip->timeout_a;
	do {
		if (st33zp24_i2c_check_locality(dev) >= 0)
			return chip->locality;
		udelay(TPM_TIMEOUT_MS * 1000);
	} while	 (get_timer(start) < stop);

	return -EACCES;
}

/*
 * st33zp24_i2c_status return the TPM_STS register
 * @param: chip, the tpm chip description
 * @return: the TPM_STS register value.
 */
static u8 st33zp24_i2c_status(struct udevice *dev)
{
	u8 data;

	st33zp24_i2c_read(dev, TPM_STS, &data, 1);

	return data;
}

/*
 * st33zp24_i2c_get_burstcount return the burstcount address 0x19 0x1A
 * @param: chip, the chip description
 * return: the burstcount or -TPM_DRIVER_ERR in case of error.
 */
static int st33zp24_i2c_get_burstcount(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	unsigned long start, stop;
	int burstcnt, status;
	u8 tpm_reg, temp;

	/* wait for burstcount */
	start = get_timer(0);
	stop = chip->timeout_d;
	do {
		tpm_reg = TPM_STS + 1;
		status = st33zp24_i2c_read(dev, tpm_reg, &temp, 1);
		if (status < 0)
			return -EBUSY;

		tpm_reg = TPM_STS + 2;
		burstcnt = temp;
		status = st33zp24_i2c_read(dev, tpm_reg, &temp, 1);
		if (status < 0)
			return -EBUSY;

		burstcnt |= temp << 8;
		if (burstcnt)
			return burstcnt;
		udelay(TIS_SHORT_TIMEOUT_MS * 1000);
	} while (get_timer(start) < stop);

	return -EBUSY;
}

/*
 * st33zp24_i2c_cancel, cancel the current command execution or
 * set STS to COMMAND READY.
 * @param: chip, tpm_chip description.
 */
static void st33zp24_i2c_cancel(struct udevice *dev)
{
	u8 data;

	data = TPM_STS_COMMAND_READY;
	st33zp24_i2c_write(dev, TPM_STS, &data, 1);
}

/*
 * st33zp24_i2c_wait_for_stat wait for a TPM_STS value
 * @param: chip, the tpm chip description
 * @param: mask, the value mask to wait
 * @param: timeout, the timeout
 * @param: status,
 * @return: the tpm status, 0 if success, -ETIME if timeout is reached.
 */
static int st33zp24_i2c_wait_for_stat(struct udevice *dev, u8 mask,
				      unsigned long timeout, int *status)
{
	unsigned long start, stop;

	/* Check current status */
	*status = st33zp24_i2c_status(dev);
	if ((*status & mask) == mask)
		return 0;

	start = get_timer(0);
	stop = timeout;
	do {
		udelay(TPM_TIMEOUT_MS * 1000);
		*status = st33zp24_i2c_status(dev);
		if ((*status & mask) == mask)
			return 0;
	} while (get_timer(start) < stop);

	return -ETIME;
}

/*
 * st33zp24_i2c_recv_data receive data
 * @param: chip, the tpm chip description
 * @param: buf, the buffer where the data are received
 * @param: count, the number of data to receive
 * @return: the number of bytes read from TPM FIFO.
 */
static int st33zp24_i2c_recv_data(struct udevice *dev, u8 *buf, size_t count)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	int size = 0, burstcnt, len, ret, status;

	while (size < count &&
	       st33zp24_i2c_wait_for_stat(dev, TPM_STS_DATA_AVAIL | TPM_STS_VALID,
				chip->timeout_c, &status) == 0) {
		burstcnt = st33zp24_i2c_get_burstcount(dev);
		if (burstcnt < 0)
			return burstcnt;
		len = min_t(int, burstcnt, count - size);
		ret = st33zp24_i2c_read(dev, TPM_DATA_FIFO, buf + size, len);
		if (ret < 0)
			return ret;

		size += len;
	}

	return size;
}

/*
 * st33zp24_i2c_recv received TPM response through TPM phy.
 * @param: chip, tpm_chip description.
 * @param: buf,	the buffer to store data.
 * @param: count, the number of bytes that can received (sizeof buf).
 * @return: Returns zero in case of success else -EIO.
 */
static int st33zp24_i2c_recv(struct udevice *dev, u8 *buf, size_t count)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	int size;
	unsigned int expected;

	if (!chip)
		return -ENODEV;

	if (count < TPM_HEADER_SIZE) {
		size = -EIO;
		goto out;
	}

	size = st33zp24_i2c_recv_data(dev, buf, TPM_HEADER_SIZE);
	if (size < TPM_HEADER_SIZE) {
		debug("TPM error, unable to read header\n");
		goto out;
	}

	expected = get_unaligned_be32(buf + 2);
	if (expected > count || expected < TPM_HEADER_SIZE) {
		size = -EIO;
		goto out;
	}

	size += st33zp24_i2c_recv_data(dev, &buf[TPM_HEADER_SIZE],
				   expected - TPM_HEADER_SIZE);
	if (size < expected) {
		debug("TPM error, unable to read remaining bytes of result\n");
		size = -EIO;
		goto out;
	}

out:
	st33zp24_i2c_cancel(dev);
	st33zp24_i2c_release_locality(dev);

	return size;
}

/*
 * st33zp24_i2c_send send TPM commands through TPM phy.
 * @param: chip, tpm_chip description.
 * @param: buf,	the buffer to send.
 * @param: len, the number of bytes to send.
 * @return: Returns zero in case of success else the negative error code.
 */
static int st33zp24_i2c_send(struct udevice *dev, const u8 *buf, size_t len)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	u32 i, size;
	int burstcnt, ret, status;
	u8 data, tpm_stat;

	if (!chip)
		return -ENODEV;
	if (len < TPM_HEADER_SIZE)
		return -EIO;

	ret = st33zp24_i2c_request_locality(dev);
	if (ret < 0)
		return ret;

	tpm_stat = st33zp24_i2c_status(dev);
	if ((tpm_stat & TPM_STS_COMMAND_READY) == 0) {
		st33zp24_i2c_cancel(dev);
		if (st33zp24_i2c_wait_for_stat(dev, TPM_STS_COMMAND_READY,
					       chip->timeout_b, &status) < 0) {
			ret = -ETIME;
			goto out_err;
		}
	}

	for (i = 0; i < len - 1;) {
		burstcnt = st33zp24_i2c_get_burstcount(dev);
		if (burstcnt < 0)
			return burstcnt;

		size = min_t(int, len - i - 1, burstcnt);
		ret = st33zp24_i2c_write(dev, TPM_DATA_FIFO, buf + i, size);
		if (ret < 0)
			goto out_err;

		i += size;
	}

	tpm_stat = st33zp24_i2c_status(dev);
	if ((tpm_stat & TPM_STS_DATA_EXPECT) == 0) {
		ret = -EIO;
		goto out_err;
	}

	ret = st33zp24_i2c_write(dev, TPM_DATA_FIFO, buf + len - 1, 1);
	if (ret < 0)
		goto out_err;

	tpm_stat = st33zp24_i2c_status(dev);
	if ((tpm_stat & TPM_STS_DATA_EXPECT) != 0) {
		ret = -EIO;
		goto out_err;
	}

	data = TPM_STS_GO;
	ret = st33zp24_i2c_write(dev, TPM_STS, &data, 1);
	if (ret < 0)
		goto out_err;

	return len;

out_err:
	st33zp24_i2c_cancel(dev);
	st33zp24_i2c_release_locality(dev);

	return ret;
}

static int st33zp24_i2c_cleanup(struct udevice *dev)
{
	st33zp24_i2c_cancel(dev);
	/*
	 * The TPM needs some time to clean up here,
	 * so we sleep rather than keeping the bus busy
	 */
	mdelay(2);
	st33zp24_i2c_release_locality(dev);

	return 0;
}

static int st33zp24_i2c_init(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);

	chip->is_open = 1;

	/* Default timeouts - these could move to the device tree */
	chip->timeout_a = TIS_SHORT_TIMEOUT_MS;
	chip->timeout_b = TIS_LONG_TIMEOUT_MS;
	chip->timeout_c = TIS_SHORT_TIMEOUT_MS;
	chip->timeout_d = TIS_SHORT_TIMEOUT_MS;

	chip->locality = LOCALITY0;

	/*
	 * A timeout query to TPM can be placed here.
	 * Standard timeout values are used so far
	 */

	return 0;
}

static int st33zp24_i2c_open(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	int rc;

	debug("%s: start\n", __func__);
	if (chip->is_open)
		return -EBUSY;

	rc = st33zp24_i2c_init(dev);
	if (rc < 0)
		chip->is_open = 0;

	return rc;
}

static int st33zp24_i2c_close(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);

	if (chip->is_open) {
		st33zp24_i2c_release_locality(dev);
		chip->is_open = 0;
		chip->vend_dev = 0;
	}

	return 0;
}

static int st33zp24_i2c_get_desc(struct udevice *dev, char *buf, int size)
{
	struct tpm_chip *chip = dev_get_priv(dev);

	if (size < 50)
		return -ENOSPC;

	return snprintf(buf, size, "1.2 TPM (%s, chip type %s device-id 0x%x)",
			chip->is_open ? "open" : "closed",
			dev->name,
			chip->vend_dev >> 16);
}

static const struct tpm_ops st33zp24_i2c_tpm_ops = {
	.open = st33zp24_i2c_open,
	.close = st33zp24_i2c_close,
	.recv = st33zp24_i2c_recv,
	.send = st33zp24_i2c_send,
	.cleanup = st33zp24_i2c_cleanup,
	.get_desc = st33zp24_i2c_get_desc,
};

static int st33zp24_i2c_probe(struct udevice *dev)
{
	struct tpm_chip *chip = dev_get_priv(dev);

	/* Default timeouts */
	chip->timeout_a = TIS_SHORT_TIMEOUT_MS;
	chip->timeout_b = TIS_LONG_TIMEOUT_MS;
	chip->timeout_c = TIS_SHORT_TIMEOUT_MS;
	chip->timeout_d = TIS_SHORT_TIMEOUT_MS;

	chip->locality = LOCALITY0;

	i2c_set_chip_offset_len(dev, 0);

	debug("ST33ZP24 I2C TPM from STMicroelectronics found\n");

	return 0;
}

static int st33zp24_i2c_remove(struct udevice *dev)
{
	st33zp24_i2c_release_locality(dev);

	return 0;
}

static const struct udevice_id st33zp24_i2c_ids[] = {
	{ .compatible = "st,st33zp24-i2c" },
	{ }
};

U_BOOT_DRIVER(st33zp24_i2c) = {
	.name   = "st33zp24-i2c",
	.id     = UCLASS_TPM,
	.of_match = of_match_ptr(st33zp24_i2c_ids),
	.probe  = st33zp24_i2c_probe,
	.remove = st33zp24_i2c_remove,
	.ops = &st33zp24_i2c_tpm_ops,
	.priv_auto	= sizeof(struct tpm_chip),
};
