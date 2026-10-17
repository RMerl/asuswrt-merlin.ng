// SPDX-License-Identifier: GPL-2.0
/*
 * Author:
 * Miquel Raynal <miquel.raynal@bootlin.com>
 *
 * Description:
 * SPI-level driver for TCG/TIS TPM (trusted platform module).
 * Specifications at www.trustedcomputinggroup.org
 *
 * This device driver implements the TPM interface as defined in
 * the TCG SPI protocol stack version 2.0.
 *
 * It is based on the U-Boot driver tpm_tis_infineon_i2c.c.
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <log.h>
#include <spi.h>
#include <tpm-v2.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/unaligned/be_byteshift.h>
#include <asm-generic/gpio.h>

#include "tpm_tis.h"
#include "tpm_internal.h"

#define MAX_SPI_FRAMESIZE 64

/* Number of wait states to wait for */
#define TPM_WAIT_STATES 100

/**
 * struct tpm_tis_chip_data - Non-discoverable TPM information
 *
 * @pcr_count:		Number of PCR per bank
 * @pcr_select_min:	Size in octets of the pcrSelect array
 */
struct tpm_tis_chip_data {
	unsigned int pcr_count;
	unsigned int pcr_select_min;
	unsigned int time_before_first_cmd_ms;
};

/**
 * tpm_tis_spi_read() - Read from TPM register
 *
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
 * Return: -EIO on error, 0 on success.
 */
static int tpm_tis_spi_xfer(struct udevice *dev, u32 addr, const u8 *out,
			    u8 *in, u16 len)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);
	int transfer_len, ret;
	u8 tx_buf[MAX_SPI_FRAMESIZE];
	u8 rx_buf[MAX_SPI_FRAMESIZE];

	if (in && out) {
		log(LOGC_NONE, LOGL_ERR, "%s: can't do full duplex\n",
		    __func__);
		return -EINVAL;
	}

	ret = spi_claim_bus(slave);
	if (ret < 0) {
		log(LOGC_NONE, LOGL_ERR, "%s: could not claim bus\n", __func__);
		return ret;
	}

	while (len) {
		/* Request */
		transfer_len = min_t(u16, len, MAX_SPI_FRAMESIZE);
		tx_buf[0] = (in ? BIT(7) : 0) | (transfer_len - 1);
		tx_buf[1] = 0xD4;
		tx_buf[2] = addr >> 8;
		tx_buf[3] = addr;

		ret = spi_xfer(slave, 4 * 8, tx_buf, rx_buf, SPI_XFER_BEGIN);
		if (ret < 0) {
			log(LOGC_NONE, LOGL_ERR,
			    "%s: spi request transfer failed (err: %d)\n",
			    __func__, ret);
			goto release_bus;
		}

		/* Wait state */
		if (!(rx_buf[3] & 0x1)) {
			int i;

			for (i = 0; i < TPM_WAIT_STATES; i++) {
				ret = spi_xfer(slave, 1 * 8, NULL, rx_buf, 0);
				if (ret) {
					log(LOGC_NONE, LOGL_ERR,
					    "%s: wait state failed: %d\n",
					    __func__, ret);
					goto release_bus;
				}

				if (rx_buf[0] & 0x1)
					break;
			}

			if (i == TPM_WAIT_STATES) {
				log(LOGC_NONE, LOGL_ERR,
				    "%s: timeout on wait state\n", __func__);
				ret = -ETIMEDOUT;
				goto release_bus;
			}
		}

		/* Read/Write */
		if (out) {
			memcpy(tx_buf, out, transfer_len);
			out += transfer_len;
		}

		ret = spi_xfer(slave, transfer_len * 8,
			       out ? tx_buf : NULL,
			       in ? rx_buf : NULL,
			       SPI_XFER_END);
		if (ret) {
			log(LOGC_NONE, LOGL_ERR,
			    "%s: spi read transfer failed (err: %d)\n",
			    __func__, ret);
			goto release_bus;
		}

		if (in) {
			memcpy(in, rx_buf, transfer_len);
			in += transfer_len;
		}

		len -= transfer_len;
	}

release_bus:
	/* If an error occurred, release the chip by deasserting the CS */
	if (ret < 0)
		spi_xfer(slave, 0, NULL, NULL, SPI_XFER_END);

	spi_release_bus(slave);

	return ret;
}

static int tpm_tis_spi_read(struct udevice *dev, u32 addr, u16 len, u8 *in)
{
	return tpm_tis_spi_xfer(dev, addr, NULL, in, len);
}

static int tpm_tis_spi_read32(struct udevice *dev, u32 addr, u32 *result)
{
	__le32 result_le;
	int ret;

	ret = tpm_tis_spi_read(dev, addr, sizeof(u32), (u8 *)&result_le);
	if (!ret)
		*result = le32_to_cpu(result_le);

	return ret;
}


static int tpm_tis_spi_write(struct udevice *dev, u32 addr, u16 len, const u8 *out)
{
	return tpm_tis_spi_xfer(dev, addr, out, NULL, len);
}

static int tpm_tis_spi_write32(struct udevice *dev, u32 addr, u32 value)
{
	__le32 value_le = cpu_to_le32(value);

	return tpm_tis_spi_write(dev, addr, sizeof(value), (u8 *)&value_le);
}

static int tpm_tis_wait_init(struct udevice *dev, int loc)
{
	struct tpm_chip *chip = dev_get_priv(dev);
	unsigned long start, stop;
	u8 status;
	int ret;

	start = get_timer(0);
	stop = chip->timeout_b;
	do {
		mdelay(TPM_TIMEOUT_MS);

		ret = tpm_tis_spi_read(dev, TPM_ACCESS(loc), 1, &status);
		if (ret)
			break;

		if (status & TPM_ACCESS_VALID)
			return 0;
	} while (get_timer(start) < stop);

	return -EIO;
}

static struct tpm_tis_phy_ops phy_ops = {
	.read_bytes = tpm_tis_spi_read,
	.write_bytes = tpm_tis_spi_write,
	.read32 = tpm_tis_spi_read32,
	.write32 = tpm_tis_spi_write32,
};

static int tpm_tis_spi_probe(struct udevice *dev)
{
	struct tpm_tis_chip_data *drv_data = (void *)dev_get_driver_data(dev);
	struct tpm_chip_priv *priv = dev_get_uclass_priv(dev);
	struct tpm_chip *chip = dev_get_priv(dev);
	int ret;

	/* Use the TPM v2 stack */
	priv->version = TPM_V2;

	if (CONFIG_IS_ENABLED(DM_GPIO)) {
		struct gpio_desc reset_gpio;

		ret = gpio_request_by_name(dev, "reset-gpios", 0,
					   &reset_gpio, GPIOD_IS_OUT);
		if (ret) {
			/* legacy reset */
			ret = gpio_request_by_name(dev, "gpio-reset", 0,
						   &reset_gpio, GPIOD_IS_OUT);
			if (ret) {
				log(LOGC_NONE, LOGL_NOTICE,
				    "%s: missing reset GPIO\n", __func__);
				goto init;
			}
			log(LOGC_NONE, LOGL_NOTICE,
			    "%s: gpio-reset is deprecated\n", __func__);
		}
		dm_gpio_set_value(&reset_gpio, 1);
		mdelay(1);
		dm_gpio_set_value(&reset_gpio, 0);
	}
init:
	/* Ensure a minimum amount of time elapsed since reset of the TPM */
	mdelay(drv_data->time_before_first_cmd_ms);

	ret = tpm_tis_wait_init(dev, chip->locality);
	if (ret) {
		log(LOGC_DM, LOGL_ERR, "%s: no device found\n", __func__);
		return ret;
	}

	tpm_tis_ops_register(dev, &phy_ops);
	ret = tpm_tis_init(dev);
	if (ret)
		goto err;

	priv->pcr_count = drv_data->pcr_count;
	priv->pcr_select_min = drv_data->pcr_select_min;
	priv->version = TPM_V2;

	return 0;
err:
	return -EINVAL;
}

static int tpm_tis_spi_remove(struct udevice *udev)
{
	return tpm_tis_cleanup(udev);
}

static const struct tpm_ops tpm_tis_spi_ops = {
	.open		= tpm_tis_open,
	.close		= tpm_tis_close,
	.get_desc	= tpm_tis_get_desc,
	.send		= tpm_tis_send,
	.recv		= tpm_tis_recv,
	.cleanup	= tpm_tis_cleanup,
};

static const struct tpm_tis_chip_data tpm_tis_std_chip_data = {
	.pcr_count = 24,
	.pcr_select_min = 3,
	.time_before_first_cmd_ms = 30,
};

static const struct udevice_id tpm_tis_spi_ids[] = {
	{
		.compatible = "tcg,tpm_tis-spi",
		.data = (ulong)&tpm_tis_std_chip_data,
	},
	{ }
};

U_BOOT_DRIVER(tpm_tis_spi) = {
	.name   = "tpm_tis_spi",
	.id     = UCLASS_TPM,
	.of_match = tpm_tis_spi_ids,
	.ops    = &tpm_tis_spi_ops,
	.probe	= tpm_tis_spi_probe,
	.remove	= tpm_tis_spi_remove,
	.priv_auto	= sizeof(struct tpm_chip),
};
