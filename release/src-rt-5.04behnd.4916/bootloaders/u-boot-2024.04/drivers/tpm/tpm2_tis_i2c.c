// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2022 IBM Corp.
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <i2c.h>
#include <log.h>
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

struct tpm_tis_chip_data {
	unsigned int pcr_count;
	unsigned int pcr_select_min;
};

static uint tpm_tis_i2c_address_to_register(u32 addr)
{
	addr &= 0xFFF;

	/*
	 * Adapt register addresses that have changed compared to older TIS
	 * version.
	 */
	switch (addr) {
	case TPM_ACCESS(0):
		return 0x04;
	case TPM_DID_VID(0):
		return 0x48;
	case TPM_RID(0):
		return 0x4C;
	default:
		return addr;
	}
}

static int tpm_tis_i2c_read(struct udevice *dev, u32 addr, u16 len, u8 *in)
{
	int rc;
	int count = 0;
	uint reg = tpm_tis_i2c_address_to_register(addr);

	do {
		rc = dm_i2c_read(dev, reg, in, len);
		udelay(SLEEP_DURATION_US);
	} while (rc && count++ < MAX_COUNT);

	return rc;
}

static int tpm_tis_i2c_write(struct udevice *dev, u32 addr, u16 len,
			     const u8 *out)
{
	int rc;
	int count = 0;
	uint reg = tpm_tis_i2c_address_to_register(addr);

	do {
		rc = dm_i2c_write(dev, reg, out, len);
		udelay(SLEEP_DURATION_US);
	} while (rc && count++ < MAX_COUNT);

	return rc;
}

static int tpm_tis_i2c_read32(struct udevice *dev, u32 addr, u32 *result)
{
	__le32 result_le;
	int rc;

	rc = tpm_tis_i2c_read(dev, addr, sizeof(u32), (u8 *)&result_le);
	if (!rc)
		*result = le32_to_cpu(result_le);

	return rc;
}

static int tpm_tis_i2c_write32(struct udevice *dev, u32 addr, u32 value)
{
	__le32 value_le = cpu_to_le32(value);

	return tpm_tis_i2c_write(dev, addr, sizeof(value), (u8 *)&value_le);
}

static struct tpm_tis_phy_ops phy_ops = {
	.read_bytes = tpm_tis_i2c_read,
	.write_bytes = tpm_tis_i2c_write,
	.read32 = tpm_tis_i2c_read32,
	.write32 = tpm_tis_i2c_write32,
};

static int tpm_tis_i2c_probe(struct udevice *udev)
{
	struct tpm_tis_chip_data *drv_data = (void *)dev_get_driver_data(udev);
	struct tpm_chip_priv *priv = dev_get_uclass_priv(udev);
	int rc;
	u8 loc = 0;

	tpm_tis_ops_register(udev, &phy_ops);

	/*
	 * Force locality 0. The core driver doesn't actually write the
	 * locality register and instead just reads/writes various access
	 * bits of the selected locality.
	 */
	rc = dm_i2c_write(udev, 0, &loc, 1);
	if (rc)
		return rc;

	rc = tpm_tis_init(udev);
	if (rc)
		return rc;

	priv->pcr_count = drv_data->pcr_count;
	priv->pcr_select_min = drv_data->pcr_select_min;
	priv->version = TPM_V2;

	return 0;
}

static int tpm_tis_i2c_remove(struct udevice *udev)
{
	return tpm_tis_cleanup(udev);
}

static const struct tpm_ops tpm_tis_i2c_ops = {
	.open = tpm_tis_open,
	.close = tpm_tis_close,
	.get_desc = tpm_tis_get_desc,
	.send = tpm_tis_send,
	.recv = tpm_tis_recv,
	.cleanup = tpm_tis_cleanup,
};

static const struct tpm_tis_chip_data tpm_tis_std_chip_data = {
	.pcr_count = 24,
	.pcr_select_min = 3,
};

static const struct udevice_id tpm_tis_i2c_ids[] = {
	{
		.compatible = "nuvoton,npct75x",
		.data = (ulong)&tpm_tis_std_chip_data,
	},
	{
		.compatible = "tcg,tpm-tis-i2c",
		.data = (ulong)&tpm_tis_std_chip_data,
	},
	{ }
};

U_BOOT_DRIVER(tpm_tis_i2c) = {
	.name = "tpm_tis_i2c",
	.id = UCLASS_TPM,
	.of_match = tpm_tis_i2c_ids,
	.ops = &tpm_tis_i2c_ops,
	.probe = tpm_tis_i2c_probe,
	.remove = tpm_tis_i2c_remove,
	.priv_auto = sizeof(struct tpm_chip),
};
