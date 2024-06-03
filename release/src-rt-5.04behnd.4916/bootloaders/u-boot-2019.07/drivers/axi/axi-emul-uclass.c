// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

#include <common.h>
#include <axi.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <asm/axi.h>

int axi_sandbox_get_emul(struct udevice *bus, ulong address,
			 enum axi_size_t size, struct udevice **emulp)
{
	struct udevice *dev;
	u32 reg[2];
	uint offset;

	switch (size) {
	case AXI_SIZE_8:
		offset = 1;
		break;
	case AXI_SIZE_16:
		offset = 2;
		break;
	case AXI_SIZE_32:
		offset = 4;
		break;
	default:
		debug("%s: Unknown AXI transfer size '%d'", bus->name, size);
		offset = 0;
	}

	/*
	 * Note: device_find_* don't activate the devices; they're activated
	 *	 as-needed below.
	 */
	for (device_find_first_child(bus, &dev);
	     dev;
	     device_find_next_child(&dev)) {
		int ret;

		ret = dev_read_u32_array(dev, "reg", reg, ARRAY_SIZE(reg));
		if (ret) {
			debug("%s: Could not read 'reg' property of %s\n",
			      bus->name, dev->name);
			continue;
		}

		/*
		 * Does the transfer's address fall into this device's address
		 * space?
		 */
		if (address >= reg[0] && address <= reg[0] + reg[1] - offset) {
			/* If yes, activate it... */
			if (device_probe(dev)) {
				debug("%s: Could not activate %s\n",
				      bus->name, dev->name);
				return -ENODEV;
			}

			/* ...and return it */
			*emulp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

int axi_get_store(struct udevice *dev, u8 **storep)
{
	struct axi_emul_ops *ops = axi_emul_get_ops(dev);

	if (!ops->get_store)
		return -ENOSYS;

	return ops->get_store(dev, storep);
}

UCLASS_DRIVER(axi_emul) = {
	.id		= UCLASS_AXI_EMUL,
	.name		= "axi_emul",
};
