// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#include <common.h>
#include <i2c_eeprom.h>
#include <linker_lists.h>
#include <misc.h>
#include <nvmem.h>
#include <rtc.h>
#include <dm/device_compat.h>
#include <dm/ofnode.h>
#include <dm/read.h>
#include <dm/uclass.h>

int nvmem_cell_read(struct nvmem_cell *cell, void *buf, size_t size)
{
	dev_dbg(cell->nvmem, "%s: off=%u size=%zu\n", __func__, cell->offset, size);
	if (size != cell->size)
		return -EINVAL;

	switch (cell->nvmem->driver->id) {
	case UCLASS_I2C_EEPROM:
		return i2c_eeprom_read(cell->nvmem, cell->offset, buf, size);
	case UCLASS_MISC: {
		int ret = misc_read(cell->nvmem, cell->offset, buf, size);

		if (ret < 0)
			return ret;
		if (ret != size)
			return -EIO;
		return 0;
	}
	case UCLASS_RTC:
		return dm_rtc_read(cell->nvmem, cell->offset, buf, size);
	default:
		return -ENOSYS;
	}
}

int nvmem_cell_write(struct nvmem_cell *cell, const void *buf, size_t size)
{
	dev_dbg(cell->nvmem, "%s: off=%u size=%zu\n", __func__, cell->offset, size);
	if (size != cell->size)
		return -EINVAL;

	switch (cell->nvmem->driver->id) {
	case UCLASS_I2C_EEPROM:
		return i2c_eeprom_write(cell->nvmem, cell->offset, buf, size);
	case UCLASS_MISC: {
		int ret = misc_write(cell->nvmem, cell->offset, buf, size);

		if (ret < 0)
			return ret;
		if (ret != size)
			return -EIO;
		return 0;
	}
	case UCLASS_RTC:
		return dm_rtc_write(cell->nvmem, cell->offset, buf, size);
	default:
		return -ENOSYS;
	}
}

/**
 * nvmem_get_device() - Get an nvmem device for a cell
 * @node: ofnode of the nvmem device
 * @cell: Cell to look up
 *
 * Try to find a nvmem-compatible device by going through the nvmem interfaces.
 *
 * Return:
 * * 0 on success
 * * -ENODEV if we didn't find anything
 * * A negative error if there was a problem looking up the device
 */
static int nvmem_get_device(ofnode node, struct nvmem_cell *cell)
{
	int i, ret;
	enum uclass_id ids[] = {
		UCLASS_I2C_EEPROM,
		UCLASS_MISC,
		UCLASS_RTC,
	};

	for (i = 0; i < ARRAY_SIZE(ids); i++) {
		ret = uclass_get_device_by_ofnode(ids[i], node, &cell->nvmem);
		if (!ret)
			return 0;
		if (ret != -ENODEV && ret != -EPFNOSUPPORT)
			return ret;
	}

	return -ENODEV;
}

int nvmem_cell_get_by_index(struct udevice *dev, int index,
			    struct nvmem_cell *cell)
{
	fdt_addr_t offset;
	fdt_size_t size = FDT_SIZE_T_NONE;
	int ret;
	struct ofnode_phandle_args args;

	dev_dbg(dev, "%s: index=%d\n", __func__, index);

	ret = dev_read_phandle_with_args(dev, "nvmem-cells", NULL, 0, index,
					 &args);
	if (ret)
		return ret;

	ret = nvmem_get_device(ofnode_get_parent(args.node), cell);
	if (ret)
		return ret;

	offset = ofnode_get_addr_size_index_notrans(args.node, 0, &size);
	if (offset == FDT_ADDR_T_NONE || size == FDT_SIZE_T_NONE) {
		dev_dbg(cell->nvmem, "missing address or size for %s\n",
			ofnode_get_name(args.node));
		return -EINVAL;
	}

	cell->offset = offset;
	cell->size = size;
	return 0;
}

int nvmem_cell_get_by_name(struct udevice *dev, const char *name,
			   struct nvmem_cell *cell)
{
	int index;

	dev_dbg(dev, "%s, name=%s\n", __func__, name);

	index = dev_read_stringlist_search(dev, "nvmem-cell-names", name);
	if (index < 0)
		return index;

	return nvmem_cell_get_by_index(dev, index, cell);
}
