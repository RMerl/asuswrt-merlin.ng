// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
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
