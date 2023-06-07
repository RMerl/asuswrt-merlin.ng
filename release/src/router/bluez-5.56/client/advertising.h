/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2016  Intel Corporation. All rights reserved.
 *
 *
 */

void ad_register(DBusConnection *conn, GDBusProxy *manager, const char *type);
void ad_unregister(DBusConnection *conn, GDBusProxy *manager);

void ad_advertise_uuids(DBusConnection *conn, int argc, char *argv[]);
void ad_disable_uuids(DBusConnection *conn);
void ad_advertise_service(DBusConnection *conn, int argc, char *argv[]);
void ad_disable_service(DBusConnection *conn);
void ad_advertise_manufacturer(DBusConnection *conn, int argc, char *argv[]);
void ad_disable_manufacturer(DBusConnection *conn);
void ad_advertise_tx_power(DBusConnection *conn, dbus_bool_t *value);
void ad_advertise_name(DBusConnection *conn, bool value);
void ad_advertise_appearance(DBusConnection *conn, bool value);
void ad_advertise_local_name(DBusConnection *conn, const char *name);
void ad_advertise_local_appearance(DBusConnection *conn, long int *value);
void ad_advertise_duration(DBusConnection *conn, long int *value);
void ad_advertise_timeout(DBusConnection *conn, long int *value);
void ad_advertise_data(DBusConnection *conn, int argc, char *argv[]);
void ad_disable_data(DBusConnection *conn);
void ad_advertise_discoverable(DBusConnection *conn, dbus_bool_t *value);
void ad_advertise_discoverable_timeout(DBusConnection *conn, long int *value);
void ad_advertise_secondary(DBusConnection *conn, const char *value);
