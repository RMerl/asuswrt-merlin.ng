/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2020 Google LLC
 *
 *
 */

void adv_monitor_add_manager(DBusConnection *conn, GDBusProxy *proxy);
void adv_monitor_remove_manager(DBusConnection *conn);
void adv_monitor_register_app(DBusConnection *conn);
void adv_monitor_unregister_app(DBusConnection *conn);
void adv_monitor_set_rssi_threshold(int16_t low_threshold,
							int16_t high_threshold);
void adv_monitor_set_rssi_timeout(uint16_t low_timeout, uint16_t high_timeout);
void adv_monitor_set_rssi_sampling_period(uint16_t sampling);
void adv_monitor_add_monitor(DBusConnection *conn, char *type,
							int argc, char *argv[]);
void adv_monitor_print_monitor(DBusConnection *conn, int monitor_idx);
void adv_monitor_remove_monitor(DBusConnection *conn, int monitor_idx);
void adv_monitor_get_supported_info(void);
