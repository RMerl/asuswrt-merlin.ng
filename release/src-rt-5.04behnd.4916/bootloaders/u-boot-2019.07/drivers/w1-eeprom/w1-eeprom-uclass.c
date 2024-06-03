// SPDX-License-Identifier:	GPL-2.0+
/*
 *
 * Copyright (c) 2015 Free Electrons
 * Copyright (c) 2015 NextThing Co.
 * Copyright (c) 2018 Microchip Technology, Inc.
 *
 * Maxime Ripard <maxime.ripard@free-electrons.com>
 * Eugen Hristev <eugen.hristev@microchip.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <w1.h>
#include <w1-eeprom.h>

#include <dm/device-internal.h>

int w1_eeprom_read_buf(struct udevice *dev, unsigned int offset,
		       u8 *buf, unsigned int count)
{
	const struct w1_eeprom_ops *ops = device_get_ops(dev);
	u64 id = 0;
	int ret;

	if (!ops->read_buf)
		return -ENOSYS;

	ret = w1_eeprom_get_id(dev, &id);
	if (ret)
		return ret;
	if (!id)
		return -ENODEV;

	return ops->read_buf(dev, offset, buf, count);
}

int w1_eeprom_register_new_device(u64 id)
{
	u8 family = id & 0xff;
	int ret;
	struct udevice *dev;

	for (ret = uclass_first_device(UCLASS_W1_EEPROM, &dev);
	     !ret && dev;
	     uclass_next_device(&dev)) {
		if (ret || !dev) {
			debug("cannot find w1 eeprom dev\n");
			return ret;
		}
		if (dev_get_driver_data(dev) == family) {
			struct w1_device *w1;

			w1 = dev_get_parent_platdata(dev);
			if (w1->id) /* device already in use */
				continue;
			w1->id = id;
			debug("%s: Match found: %s:%s %llx\n", __func__,
			      dev->name, dev->driver->name, id);
			return 0;
		}
	}

	debug("%s: No matches found: error %d\n", __func__, ret);

	return ret;
}

int w1_eeprom_get_id(struct udevice *dev, u64 *id)
{
	struct w1_device *w1 = dev_get_parent_platdata(dev);

	if (!w1)
		return -ENODEV;
	*id = w1->id;

	return 0;
}

UCLASS_DRIVER(w1_eeprom) = {
	.name		= "w1_eeprom",
	.id		= UCLASS_W1_EEPROM,
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.post_bind	= dm_scan_fdt_dev,
#endif
};

int w1_eeprom_dm_init(void)
{
	struct udevice *dev;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_W1_EEPROM, &uc);
	if (ret) {
		debug("W1_EEPROM uclass not available\n");
		return ret;
	}

	uclass_foreach_dev(dev, uc) {
		ret = device_probe(dev);
		if (ret == -ENODEV) {	/* No such device. */
			debug("W1_EEPROM not available.\n");
			continue;
		}

		if (ret) {		/* Other error. */
			printf("W1_EEPROM probe failed, error %d\n", ret);
			continue;
		}
	}

	return 0;
}
