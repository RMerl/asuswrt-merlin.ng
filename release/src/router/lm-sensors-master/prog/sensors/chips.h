/*
    chips.h - Part of sensors, a user-space program for hardware monitoring
    Copyright (c) 1998, 1999  Frodo Looijaard <frodol@dds.nl>
    Copyright (C) 2007        Jean Delvare <jdelvare@suse.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.
*/

#ifndef PROG_SENSORS_CHIPS_H
#define PROG_SENSORS_CHIPS_H

#include "lib/sensors.h"

/*
 * Retrieved subfeatures
 */
struct sensor_subfeature_data {
	double value;		/* Subfeature value. Not used for alarms. */
	const char *name;	/* Subfeature name */
	const char *unit;	/* Unit to be displayed for this subfeature.
				   This field is optional. */
};

/*
 * Subfeature data structure. Used to create a table of implemented subfeatures
 * for a given feature.
 */
struct sensor_subfeature_list {
	int subfeature;
	const struct sensor_subfeature_list *exists;
				/* Complementary subfeatures to be displayed
				   if subfeature exists */
	int alarm;		/* true if this is an alarm */
	const char *name;	/* subfeature name to be printed */
};

void print_chip_raw(const sensors_chip_name *name);
void print_chip(const sensors_chip_name *name);

#endif /* def PROG_SENSORS_CHIPS_H */
