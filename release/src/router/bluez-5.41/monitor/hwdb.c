/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "hwdb.h"

#ifdef HAVE_UDEV_HWDB_NEW
#include <libudev.h>

bool hwdb_get_vendor_model(const char *modalias, char **vendor, char **model)
{
	struct udev *udev;
	struct udev_hwdb *hwdb;
	struct udev_list_entry *head, *entry;
	bool result;

	udev = udev_new();
	if (!udev)
		return false;

	hwdb = udev_hwdb_new(udev);
	if (!hwdb) {
		result = false;
		goto done;
	}

	*vendor = NULL;
	*model = NULL;

	head = udev_hwdb_get_properties_list_entry(hwdb, modalias, 0);

	udev_list_entry_foreach(entry, head) {
		const char *name = udev_list_entry_get_name(entry);

		if (!name)
			continue;

		if (!*vendor && !strcmp(name, "ID_VENDOR_FROM_DATABASE"))
			*vendor = strdup(udev_list_entry_get_value(entry));
		else if (!*model && !strcmp(name, "ID_MODEL_FROM_DATABASE"))
			*model = strdup(udev_list_entry_get_value(entry));
	}

	hwdb = udev_hwdb_unref(hwdb);

	result = true;

done:
	udev = udev_unref(udev);

	return result;
}

bool hwdb_get_company(const uint8_t *bdaddr, char **company)
{
	struct udev *udev;
	struct udev_hwdb *hwdb;
	struct udev_list_entry *head, *entry;
	char modalias[11];
	bool result;

	if (!bdaddr[2] && !bdaddr[1] && !bdaddr[0])
		return false;

	sprintf(modalias, "OUI:%2.2X%2.2X%2.2X",
				bdaddr[5], bdaddr[4], bdaddr[3]);

	udev = udev_new();
	if (!udev)
		return false;

	hwdb = udev_hwdb_new(udev);
	if (!hwdb) {
		result = false;
		goto done;
	}

	*company = NULL;

	head = udev_hwdb_get_properties_list_entry(hwdb, modalias, 0);

	udev_list_entry_foreach(entry, head) {
		const char *name = udev_list_entry_get_name(entry);

		if (name && !strcmp(name, "ID_OUI_FROM_DATABASE")) {
			*company = strdup(udev_list_entry_get_value(entry));
			break;
		}
	}

	hwdb = udev_hwdb_unref(hwdb);

	result = true;

done:
	udev = udev_unref(udev);

	return result;
}
#else
bool hwdb_get_vendor_model(const char *modalias, char **vendor, char **model)
{
	return false;
}

bool hwdb_get_company(const uint8_t *bdaddr, char **company)
{
	return false;
}
#endif
