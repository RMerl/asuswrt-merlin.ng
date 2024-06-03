/* SPDX-License-Identifier:	GPL-2.0+
 *
 * Copyright (c) 2015 Free Electrons
 * Copyright (c) 2015 NextThing Co
 *
 */

#ifndef __W1_H
#define __W1_H

#include <dm.h>

#define W1_FAMILY_DS24B33	0x23
#define W1_FAMILY_DS2431	0x2d
#define W1_FAMILY_DS2502	0x09
#define W1_FAMILY_EEP_SANDBOX	0xfe

struct w1_device {
	u64	id;
};

struct w1_ops {
	u8	(*read_byte)(struct udevice *dev);
	bool	(*reset)(struct udevice *dev);
	u8	(*triplet)(struct udevice *dev, bool bdir);
	void	(*write_byte)(struct udevice *dev, u8 byte);
};

int w1_get_bus(int busnum, struct udevice **busp);
u8 w1_get_device_family(struct udevice *dev);

int w1_read_buf(struct udevice *dev, u8 *buf, unsigned int count);
int w1_read_byte(struct udevice *dev);
int w1_reset_select(struct udevice *dev);
int w1_write_buf(struct udevice *dev, u8 *buf, unsigned int count);
int w1_write_byte(struct udevice *dev, u8 byte);

#endif
