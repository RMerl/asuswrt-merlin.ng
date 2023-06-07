// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
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
