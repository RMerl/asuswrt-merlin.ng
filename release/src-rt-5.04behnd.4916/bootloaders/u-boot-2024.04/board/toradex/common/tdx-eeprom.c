// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020 Toradex
 */

#include <dm.h>
#include <i2c_eeprom.h>
#include <asm/global_data.h>
#include <linux/errno.h>

DECLARE_GLOBAL_DATA_PTR;

static int get_tdx_eeprom(u32 eeprom_id, struct udevice **devp)
{
	int ret = 0;
	int node;
	ofnode eeprom;
	char eeprom_str[16];
	const char *path;

	if (!gd->fdt_blob) {
		printf("%s: don't have a valid gd->fdt_blob!\n", __func__);
		return -EFAULT;
	}

	node = fdt_path_offset(gd->fdt_blob, "/aliases");
	if (node < 0)
		return -ENODEV;

	sprintf(eeprom_str, "eeprom%d", eeprom_id);

	path = fdt_getprop(gd->fdt_blob, node, eeprom_str, NULL);
	if (!path) {
		printf("%s: no alias for %s\n", __func__, eeprom_str);
		return -ENODEV;
	}

	eeprom = ofnode_path(path);
	if (!ofnode_valid(eeprom)) {
		printf("%s: invalid hardware path to EEPROM\n", __func__);
		return -ENODEV;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_I2C_EEPROM, eeprom, devp);
	if (ret) {
		printf("%s: cannot find EEPROM by node\n", __func__);
		return ret;
	}

	return ret;
}

int read_tdx_eeprom_data(u32 eeprom_id, int offset, u8 *buf,
			 int size)
{
	struct udevice *dev;
	int ret;

	ret = get_tdx_eeprom(eeprom_id, &dev);
	if (ret)
		return ret;

	ret = i2c_eeprom_read(dev, 0x0, buf, size);
	if (ret) {
		printf("%s: error reading data from EEPROM id: %d!, ret = %d\n",
		       __func__, eeprom_id, ret);
		return ret;
	}

	return ret;
}

int write_tdx_eeprom_data(u32 eeprom_id, int offset, u8 *buf,
			  int size)
{
	struct udevice *dev;
	int ret;

	ret = get_tdx_eeprom(eeprom_id, &dev);
	if (ret)
		return ret;

	ret = i2c_eeprom_write(dev, 0x0, buf, size);
	if (ret) {
		printf("%s: error writing data to EEPROM id: %d, ret = %d\n",
		       __func__, eeprom_id, ret);
		return ret;
	}

	return ret;
}
