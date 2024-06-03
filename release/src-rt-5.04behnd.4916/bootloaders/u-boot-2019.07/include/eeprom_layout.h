/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#ifndef _LAYOUT_
#define _LAYOUT_

#define RESERVED_FIELDS			NULL
#define LAYOUT_VERSION_UNRECOGNIZED	-1
#define LAYOUT_VERSION_AUTODETECT	-2

struct eeprom_layout {
	struct eeprom_field *fields;
	int num_of_fields;
	int layout_version;
	unsigned char *data;
	int data_size;
	void (*print)(const struct eeprom_layout *eeprom_layout);
	int (*update)(struct eeprom_layout *eeprom_layout, char *field_name,
		      char *new_data);
};

void eeprom_layout_setup(struct eeprom_layout *layout, unsigned char *buf,
			 unsigned int buf_size, int layout_version);
__weak void __eeprom_layout_assign(struct eeprom_layout *layout,
				   int layout_version);

#endif
