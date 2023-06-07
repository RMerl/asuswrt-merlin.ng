/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
 *
 *
 */

void agent_register(DBusConnection *conn, GDBusProxy *manager,
						const char *capability);
void agent_unregister(DBusConnection *conn, GDBusProxy *manager);
void agent_default(DBusConnection *conn, GDBusProxy *manager);

dbus_bool_t agent_completion(void);
