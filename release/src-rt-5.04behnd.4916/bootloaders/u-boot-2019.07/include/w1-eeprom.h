/* SPDX-License-Identifier:	GPL-2.0+
 *
 * Copyright (c) 2015 Free Electrons
 * Copyright (c) 2015 NextThing Co
 * Copyright (c) 2018 Microchip Technology, Inc.
 *
 */

#ifndef __W1_EEPROM_H
#define __W1_EEPROM_H

struct udevice;

struct w1_eeprom_ops {
	/*
	 * Reads a buff from the given EEPROM memory, starting at
	 * given offset and place the results into the given buffer.
	 * Should read given count of bytes.
	 * Should return 0 on success, and normal error.h on error
	 */
	int	(*read_buf)(struct udevice *dev, unsigned int offset,
			    u8 *buf, unsigned int count);
};

int w1_eeprom_read_buf(struct udevice *dev, unsigned int offset,
		       u8 *buf, unsigned int count);

int w1_eeprom_dm_init(void);

int w1_eeprom_register_new_device(u64 id);

int w1_eeprom_get_id(struct udevice *dev, u64 *id);
#endif
