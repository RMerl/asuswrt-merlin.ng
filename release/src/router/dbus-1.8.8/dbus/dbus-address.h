/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-address.h  Server address parser.
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
#if !defined (DBUS_INSIDE_DBUS_H) && !defined (DBUS_COMPILATION)
#error "Only <dbus/dbus.h> can be included directly, this file may disappear or change contents."
#endif

#ifndef DBUS_ADDRESS_H
#define DBUS_ADDRESS_H

#include <dbus/dbus-types.h>
#include <dbus/dbus-errors.h>

DBUS_BEGIN_DECLS

/**
 * @addtogroup DBusAddress
 * @{
 */

/** Opaque type representing one of the semicolon-separated items in an address */
typedef struct DBusAddressEntry DBusAddressEntry;

DBUS_EXPORT
dbus_bool_t dbus_parse_address            (const char         *address,
					   DBusAddressEntry ***entry,
					   int                *array_len,
					   DBusError          *error);
DBUS_EXPORT
const char *dbus_address_entry_get_value  (DBusAddressEntry   *entry,
					   const char         *key);
DBUS_EXPORT
const char *dbus_address_entry_get_method (DBusAddressEntry   *entry);
DBUS_EXPORT
void        dbus_address_entries_free     (DBusAddressEntry  **entries);

DBUS_EXPORT
char* dbus_address_escape_value   (const char *value);
DBUS_EXPORT
char* dbus_address_unescape_value (const char *value,
                                   DBusError  *error);

/** @} */

DBUS_END_DECLS

#endif /* DBUS_ADDRESS_H */

