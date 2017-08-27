/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* driver.h  Bus client (driver)
 *
 * Copyright (C) 2003  CodeFactory AB
 *
 * Licensed under the Academic Free License version 2.1
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef BUS_DRIVER_H
#define BUS_DRIVER_H

#include <dbus/dbus.h>
#include "connection.h"

void        bus_driver_remove_connection     (DBusConnection *connection);
dbus_bool_t bus_driver_handle_message        (DBusConnection *connection,
                                              BusTransaction *transaction,
                                              DBusMessage    *message,
                                              DBusError      *error);
dbus_bool_t bus_driver_send_service_lost     (DBusConnection *connection,
                                              const char     *service_name,
                                              BusTransaction *transaction,
                                              DBusError      *error);
dbus_bool_t bus_driver_send_service_acquired (DBusConnection *connection,
                                              const char     *service_name,
                                              BusTransaction *transaction,
                                              DBusError      *error);
dbus_bool_t bus_driver_send_service_owner_changed  (const char     *service_name,
						    const char     *old_owner,
						    const char     *new_owner,
						    BusTransaction *transaction,
						    DBusError      *error);
dbus_bool_t bus_driver_generate_introspect_string  (DBusString *xml);



#endif /* BUS_DRIVER_H */
