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

#define W1_MATCH_ROM	0x55
#define W1_SKIP_ROM	0xcc
#define W1_SEARCH	0xf0

struct w1_bus {
	u64	search_id;
};

static int w1_enumerate(struct udevice *bus)
{
	const struct w1_ops *ops = device_get_ops(bus);
	struct w1_bus *w1 = dev_get_uclass_priv(bus);
	u64 last_rn, rn = w1->search_id, tmp64;
	bool last_device = false;
	int search_bit, desc_bit = 64;
	int last_zero = -1;
	u8 triplet_ret = 0;
	int i;

	if (!ops->reset || !ops->write_byte || !ops->triplet)
		return -ENOSYS;

	while (!last_device) {
		last_rn = rn;
		rn = 0;

		/*
		 * Reset bus and all 1-wire device state machines
		 * so they can respond to our requests.
		 *
		 * Return 0 - device(s) present, 1 - no devices present.
		 */
		if (ops->reset(bus)) {
			debug("%s: No devices present on the wire.\n",
			      __func__);
			break;
		}

		/* Start the search */
		ops->write_byte(bus, W1_SEARCH);
		for (i = 0; i < 64; ++i) {
			/* Determine the direction/search bit */
			if (i == desc_bit)
				/* took the 0 path last time, so take the 1 path */
				search_bit = 1;
			else if (i > desc_bit)
				/* take the 0 path on the next branch */
				search_bit = 0;
			else
				search_bit = ((last_rn >> i) & 0x1);

			/* Read two bits and write one bit */
			triplet_ret = ops->triplet(bus, search_bit);

			/* quit if no device responded */
			if ((triplet_ret & 0x03) == 0x03)
				break;

			/* If both directions were valid, and we took the 0 path... */
			if (triplet_ret == 0)
				last_zero = i;

			/* extract the direction taken & update the device number */
			tmp64 = (triplet_ret >> 2);
			rn |= (tmp64 << i);
		}

		if ((triplet_ret & 0x03) != 0x03) {
			if (desc_bit == last_zero || last_zero < 0) {
				last_device = 1;
				w1->search_id = 0;
			} else {
				w1->search_id = rn;
			}
			desc_bit = last_zero;

			debug("%s: Detected new device 0x%llx (family 0x%x)\n",
			      bus->name, rn, (u8)(rn & 0xff));

			/* attempt to register as w1-eeprom device */
			w1_eeprom_register_new_device(rn);
		}
	}

	return 0;
}

int w1_get_bus(int busnum, struct udevice **busp)
{
	int ret, i = 0;

	struct udevice *dev;

	for (ret = uclass_first_device(UCLASS_W1, &dev);
	     dev && !ret;
	     ret = uclass_next_device(&dev), i++) {
		if (i == busnum) {
			*busp = dev;
			return 0;
		}
	}

	if (!ret) {
		debug("Cannot find w1 bus %d\n", busnum);
		ret = -ENODEV;
	}

	return ret;
}

u8 w1_get_device_family(struct udevice *dev)
{
	struct w1_device *w1 = dev_get_parent_platdata(dev);

	return w1->id & 0xff;
}

int w1_reset_select(struct udevice *dev)
{
	struct w1_device *w1 = dev_get_parent_platdata(dev);
	struct udevice *bus = dev_get_parent(dev);
	const struct w1_ops *ops = device_get_ops(bus);
	int i;

	if (!ops->reset || !ops->write_byte)
		return -ENOSYS;

	ops->reset(bus);

	ops->write_byte(bus, W1_MATCH_ROM);

	for (i = 0; i < sizeof(w1->id); i++)
		ops->write_byte(bus, (w1->id >> (i * 8)) & 0xff);

	return 0;
}

int w1_read_byte(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	const struct w1_ops *ops = device_get_ops(bus);

	if (!ops->read_byte)
		return -ENOSYS;

	return ops->read_byte(bus);
}

int w1_read_buf(struct udevice *dev, u8 *buf, unsigned int count)
{
	int i, ret;

	for (i = 0; i < count; i++) {
		ret = w1_read_byte(dev);
		if (ret < 0)
			return ret;

		buf[i] = ret & 0xff;
	}

	return 0;
}

int w1_write_byte(struct udevice *dev, u8 byte)
{
	struct udevice *bus = dev_get_parent(dev);
	const struct w1_ops *ops = device_get_ops(bus);

	if (!ops->write_byte)
		return -ENOSYS;

	ops->write_byte(bus, byte);

	return 0;
}

static int w1_post_probe(struct udevice *bus)
{
	w1_enumerate(bus);

	return 0;
}

int w1_init(void)
{
	struct udevice *bus;
	struct uclass *uc;
	int ret;

	ret = uclass_get(UCLASS_W1, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(bus, uc) {
		ret = device_probe(bus);
		if (ret == -ENODEV) {	/* No such device. */
			printf("W1 controller not available.\n");
			continue;
		}

		if (ret) {		/* Other error. */
			printf("W1 controller probe failed.\n");
			continue;
		}
	}
	return 0;
}

UCLASS_DRIVER(w1) = {
	.name		= "w1",
	.id		= UCLASS_W1,
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.per_device_auto_alloc_size	= sizeof(struct w1_bus),
	.post_probe	= w1_post_probe,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.post_bind	= dm_scan_fdt_dev,
#endif
	.per_child_platdata_auto_alloc_size     = sizeof(struct w1_device),
};
