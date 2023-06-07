/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 */

void gatt_add_service(GDBusProxy *proxy);
void gatt_remove_service(GDBusProxy *proxy);

void gatt_add_characteristic(GDBusProxy *proxy);
void gatt_remove_characteristic(GDBusProxy *proxy);

void gatt_add_descriptor(GDBusProxy *proxy);
void gatt_remove_descriptor(GDBusProxy *proxy);

void gatt_list_attributes(const char *device);
GDBusProxy *gatt_select_attribute(GDBusProxy *parent, const char *path);
char *gatt_attribute_generator(const char *text, int state);

void gatt_read_attribute(GDBusProxy *proxy, int argc, char *argv[]);
void gatt_write_attribute(GDBusProxy *proxy, int argc, char *argv[]);
void gatt_notify_attribute(GDBusProxy *proxy, bool enable);
void gatt_clone_attribute(GDBusProxy *proxy, int argc, char *argv[]);

void gatt_acquire_write(GDBusProxy *proxy, const char *arg);
void gatt_release_write(GDBusProxy *proxy, const char *arg);

void gatt_acquire_notify(GDBusProxy *proxy, const char *arg);
void gatt_release_notify(GDBusProxy *proxy, const char *arg);

void gatt_add_manager(GDBusProxy *proxy);
void gatt_remove_manager(GDBusProxy *proxy);

void gatt_register_app(DBusConnection *conn, GDBusProxy *proxy,
						int argc, char *argv[]);
void gatt_unregister_app(DBusConnection *conn, GDBusProxy *proxy);

void gatt_register_service(DBusConnection *conn, GDBusProxy *proxy,
						int argc, char *argv[]);
void gatt_unregister_service(DBusConnection *conn, GDBusProxy *proxy,
						int argc, char *argv[]);

void gatt_register_chrc(DBusConnection *conn, GDBusProxy *proxy,
						int argc, char *argv[]);
void gatt_unregister_chrc(DBusConnection *conn, GDBusProxy *proxy,
						int argc, char *argv[]);

void gatt_register_desc(DBusConnection *conn, GDBusProxy *proxy,
						int argc, char *argv[]);
void gatt_unregister_desc(DBusConnection *conn, GDBusProxy *proxy,
						int argc, char *argv[]);

void gatt_register_include(DBusConnection *conn, GDBusProxy *proxy,
					int argc, char *argv[]);
void gatt_unregister_include(DBusConnection *conn, GDBusProxy *proxy,
						int argc, char *argv[]);
