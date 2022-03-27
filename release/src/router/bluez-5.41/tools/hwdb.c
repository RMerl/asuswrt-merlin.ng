/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>

#include "lib/bluetooth.h"

static const struct {
	uint16_t vendor;
	uint16_t product;
	const char *str;
} product_table[] = {
	{ 0x0078, 0x0001, "Nike+ FuelBand"	},
	{ 0x0097, 0x0002, "COOKOO watch"	},
	{ }
};

int main(int argc, char *argv[])
{
	uint16_t id;

	printf("# This file is part of systemd.\n");
	printf("#\n");
	printf("# Data imported from:\n");
	printf("#  http://www.bluetooth.org/Technical/AssignedNumbers/identifiers.htm\n");

	for (id = 0;; id++) {
		const char *str;
		int i;

		str = bt_compidtostr(id);
		if (!str)
			break;

		if (!strcmp(str, "internal use"))
			break;

		if (!strcmp(str, "not assigned"))
			continue;

		printf("\n");
		printf("bluetooth:v%04X*\n", id);
		printf(" ID_VENDOR_FROM_DATABASE=%s\n", str);

		for (i = 0; product_table[i].str; i++) {
			if (product_table[i].vendor != id)
				continue;

			printf("\n");
			printf("bluetooth:v%04Xp%04X*\n",
						id, product_table[i].product);
			printf(" ID_PRODUCT_FROM_DATABASE=%s\n",
							product_table[i].str);
		}
	}

	return 0;
}
