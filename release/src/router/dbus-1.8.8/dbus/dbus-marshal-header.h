/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-marshal-header.h  Managing marshaling/demarshaling of message headers
 *
 * Copyright (C) 2005  Red Hat, Inc.
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

#ifndef DBUS_MARSHAL_HEADER_H
#define DBUS_MARSHAL_HEADER_H

#include <dbus/dbus-marshal-basic.h>
#include <dbus/dbus-marshal-validate.h>

typedef struct DBusHeader      DBusHeader;
typedef struct DBusHeaderField DBusHeaderField;

#define _DBUS_HEADER_FIELD_VALUE_UNKNOWN -1
#define _DBUS_HEADER_FIELD_VALUE_NONEXISTENT -2

/**
 * Cached information about a header field in the message
 */
struct DBusHeaderField
{
  int            value_pos; /**< Position of field value, or -1/-2 */
};

/**
 * Message header data and some cached details of it.
 */
struct DBusHeader
{
  DBusString data; /**< Header network data, stored
                    * separately from body so we can
                    * independently realloc it.
                    */

  DBusHeaderField fields[DBUS_HEADER_FIELD_LAST + 1]; /**< Track the location
                                                       * of each field in header
                                                       */

  dbus_uint32_t padding : 3;        /**< bytes of alignment in header */
  dbus_uint32_t byte_order : 8;     /**< byte order of header */
};

dbus_bool_t   _dbus_header_init                   (DBusHeader        *header);
void          _dbus_header_free                   (DBusHeader        *header);
void          _dbus_header_reinit                 (DBusHeader        *header);
dbus_bool_t   _dbus_header_create                 (DBusHeader        *header,
                                                   int                byte_order,
                                                   int                type,
                                                   const char        *destination,
                                                   const char        *path,
                                                   const char        *interface,
                                                   const char        *member,
                                                   const char        *error_name);
dbus_bool_t   _dbus_header_copy                   (const DBusHeader  *header,
                                                   DBusHeader        *dest);
int           _dbus_header_get_message_type       (DBusHeader        *header);
void          _dbus_header_set_serial             (DBusHeader        *header,
                                                   dbus_uint32_t      serial);
dbus_uint32_t _dbus_header_get_serial             (DBusHeader        *header);
void          _dbus_header_update_lengths         (DBusHeader        *header,
                                                   int                body_len);
dbus_bool_t   _dbus_header_set_field_basic        (DBusHeader        *header,
                                                   int                field,
                                                   int                type,
                                                   const void        *value);
dbus_bool_t   _dbus_header_get_field_basic        (DBusHeader        *header,
                                                   int                field,
                                                   int                type,
                                                   void              *value);
dbus_bool_t   _dbus_header_get_field_raw          (DBusHeader        *header,
                                                   int                field,
                                                   const DBusString **str,
                                                   int               *pos);
dbus_bool_t   _dbus_header_delete_field           (DBusHeader        *header,
                                                   int                field);
void          _dbus_header_toggle_flag            (DBusHeader        *header,
                                                   dbus_uint32_t      flag,
                                                   dbus_bool_t        value);
dbus_bool_t   _dbus_header_get_flag               (DBusHeader        *header,
                                                   dbus_uint32_t      flag);
dbus_bool_t   _dbus_header_ensure_signature       (DBusHeader        *header,
                                                   DBusString       **type_str,
                                                   int               *type_pos);
dbus_bool_t   _dbus_header_have_message_untrusted (int                max_message_length,
                                                   DBusValidity      *validity,
                                                   int               *byte_order,
                                                   int               *fields_array_len,
                                                   int               *header_len,
                                                   int               *body_len,
                                                   const DBusString  *str,
                                                   int                start,
                                                   int                len);
dbus_bool_t   _dbus_header_load                   (DBusHeader        *header,
                                                   DBusValidationMode mode,
                                                   DBusValidity      *validity,
                                                   int                byte_order,
                                                   int                fields_array_len,
                                                   int                header_len,
                                                   int                body_len,
                                                   const DBusString  *str,
                                                   int                start,
                                                   int                len);
void          _dbus_header_byteswap               (DBusHeader        *header,
                                                   int                new_order);
char          _dbus_header_get_byte_order         (const DBusHeader  *header);



#endif /* DBUS_MARSHAL_HEADER_H */
