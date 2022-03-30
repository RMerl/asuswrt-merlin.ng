// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>

/*
 * i2c emulation works using an 'emul' node at the bus level. Each device in
 * that node is in the UCLASS_I2C_EMUL uclass, and emulates one i2c device. A
 * pointer to the device it emulates is in the 'dev' property of the emul device
 * uclass platdata (struct i2c_emul_platdata), put there by i2c_emul_find().
 * When sandbox wants an emulator for a device, it calls i2c_emul_find() which
 * searches for the emulator with the correct address. To find the device for an
 * emulator, call i2c_emul_get_device().
 *
 * The 'emul' node is in the UCLASS_I2C_EMUL_PARENT uclass. We use a separate
 * uclass so avoid having strange devices on the I2C bus.
 */

/**
 * struct i2c_emul_uc_platdata - information about the emulator for this device
 *
 * This is used by devices in UCLASS_I2C_EMUL to record information about the
 * device being emulated. It is accessible with dev_get_uclass_platdata()
 *
 * @dev: Device being emulated
 */
struct i2c_emul_uc_platdata {
	struct udevice *dev;
};

struct udevice *i2c_emul_get_device(struct udevice *emul)
{
	struct i2c_emul_uc_platdata *uc_plat = dev_get_uclass_platdata(emul);

	return uc_plat->dev;
}

int i2c_emul_find(struct udevice *dev, struct udevice **emulp)
{
	struct i2c_emul_uc_platdata *uc_plat;
	struct udevice *emul;
	int ret;

	ret = uclass_find_device_by_phandle(UCLASS_I2C_EMUL, dev,
					    "sandbox,emul", &emul);
	if (ret) {
		log_err("No emulators for device '%s'\n", dev->name);
		return ret;
	}
	uc_plat = dev_get_uclass_platdata(emul);
	uc_plat->dev = dev;
	*emulp = emul;

	return device_probe(emul);
}

UCLASS_DRIVER(i2c_emul) = {
	.id		= UCLASS_I2C_EMUL,
	.name		= "i2c_emul",
	.per_device_platdata_auto_alloc_size =
		 sizeof(struct i2c_emul_uc_platdata),
};

/*
 * This uclass is a child of the i2c bus. Its platdata is not defined here so
 * is defined by its parent, UCLASS_I2C, which uses struct dm_i2c_chip. See
 * per_child_platdata_auto_alloc_size in UCLASS_DRIVER(i2c).
 */
UCLASS_DRIVER(i2c_emul_parent) = {
	.id		= UCLASS_I2C_EMUL_PARENT,
	.name		= "i2c_emul_parent",
	.post_bind	= dm_scan_fdt_dev,
};

static const struct udevice_id i2c_emul_parent_ids[] = {
	{ .compatible = "sandbox,i2c-emul-parent" },
	{ }
};

U_BOOT_DRIVER(i2c_emul_parent_drv) = {
	.name		= "i2c_emul_parent_drv",
	.id		= UCLASS_I2C_EMUL_PARENT,
	.of_match	= i2c_emul_parent_ids,
};
