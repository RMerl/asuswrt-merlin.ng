/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009-2016 CompuLab, Ltd.
 *
 * Authors: Nikita Kiryanov <nikita@compulab.co.il>
 *	    Igor Grinberg <grinberg@compulab.co.il>
 */

#ifndef _FIELD_
#define _FIELD_

#define PRINT_FIELD_SEGMENT	"%-30s"

struct eeprom_field {
	char *name;
	int size;
	unsigned char *buf;

	void (*print)(const struct eeprom_field *eeprom_field);
	int (*update)(struct eeprom_field *eeprom_field, char *value);
};

void eeprom_field_print_bin(const struct eeprom_field *field);
int eeprom_field_update_bin(struct eeprom_field *field, char *value);

void eeprom_field_print_bin_rev(const struct eeprom_field *field);
int eeprom_field_update_bin_rev(struct eeprom_field *field, char *value);

void eeprom_field_print_mac(const struct eeprom_field *field);
int eeprom_field_update_mac(struct eeprom_field *field, char *value);

void eeprom_field_print_ascii(const struct eeprom_field *field);
int eeprom_field_update_ascii(struct eeprom_field *field, char *value);

void eeprom_field_print_reserved(const struct eeprom_field *field);
int eeprom_field_update_reserved(struct eeprom_field *field, char *value);

#endif
