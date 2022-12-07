/* SPDX-License-Identifier: GPL-2.0-or-later */
/* *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

void dict_append_entry(DBusMessageIter *dict,
			const char *key, int type, void *val);
void dict_append_array(DBusMessageIter *dict, const char *key, int type,
			void *val, int n_elements);

void set_dbus_connection(DBusConnection *conn);
DBusConnection *btd_get_dbus_connection(void);

const char *class_to_icon(uint32_t class);
const char *gap_appearance_to_icon(uint16_t appearance);
