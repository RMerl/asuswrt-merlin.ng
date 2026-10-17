/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2021 Michael Walle <michael@walle.cc>
 */

#ifndef __SL28CPLD_H
#define __SL28CPLD_H

#define SL28CPLD_VERSION	0x03

int sl28cpld_read(struct udevice *dev, uint offset);
int sl28cpld_write(struct udevice *dev, uint offset, uint8_t value);
int sl28cpld_update(struct udevice *dev, uint offset, uint8_t clear,
		    uint8_t set);

#endif
