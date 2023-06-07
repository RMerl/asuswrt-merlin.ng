// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include "lib/bluetooth.h"
#include "oui.h"

#ifdef HAVE_UDEV_HWDB_NEW
#include <libudev.h>

char *batocomp(const bdaddr_t *ba)
{
	struct udev *udev;
	struct udev_hwdb *hwdb;
	struct udev_list_entry *head, *entry;
	char modalias[11], *comp = NULL;

	sprintf(modalias, "OUI:%2.2X%2.2X%2.2X", ba->b[5], ba->b[4], ba->b[3]);

	udev = udev_new();
	if (!udev)
		return NULL;

	hwdb = udev_hwdb_new(udev);
	if (!hwdb)
		goto done;

	head = udev_hwdb_get_properties_list_entry(hwdb, modalias, 0);

	udev_list_entry_foreach(entry, head) {
		const char *name = udev_list_entry_get_name(entry);

		if (name && !strcmp(name, "ID_OUI_FROM_DATABASE")) {
			comp = strdup(udev_list_entry_get_value(entry));
			break;
		}
	}

	hwdb = udev_hwdb_unref(hwdb);

done:
	udev = udev_unref(udev);

	return comp;
}
#else
char *batocomp(const bdaddr_t *ba)
{
	return NULL;
}
#endif
