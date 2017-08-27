/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-hash.h Generic hash table utility (internal to D-Bus implementation)
 * 
 * Copyright (C) 2002  Red Hat, Inc.
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

#ifndef DBUS_HASH_H
#define DBUS_HASH_H

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include <dbus/dbus-memory.h>
#include <dbus/dbus-types.h>
#include <dbus/dbus-sysdeps.h>

DBUS_BEGIN_DECLS

/**
 * @addtogroup DBusHashTable
 * @{
 */

/** Hash iterator object. The iterator is on the stack, but its real
 * fields are hidden privately.
 */
struct DBusHashIter
{
  void *dummy1; /**< Do not use. */
  void *dummy2; /**< Do not use. */
  void *dummy3; /**< Do not use. */
  void *dummy4; /**< Do not use. */
  int   dummy5; /**< Do not use. */
  int   dummy6; /**< Do not use. */
};

typedef struct DBusHashTable DBusHashTable;
typedef struct DBusHashIter  DBusHashIter;

/* Allowing an arbitrary function as with GLib
 * would be nicer for a public API, but for
 * an internal API this saves typing, we can add
 * more whenever we feel like it.
 */
typedef enum
{
  DBUS_HASH_STRING,        /**< Hash keys are strings. */
  DBUS_HASH_INT,           /**< Hash keys are integers. */
  DBUS_HASH_UINTPTR        /**< Hash keys are integer capable to hold a pointer. */
} DBusHashType;

DBusHashTable* _dbus_hash_table_new                (DBusHashType      type,
                                                    DBusFreeFunction  key_free_function,
                                                    DBusFreeFunction  value_free_function);
DBusHashTable* _dbus_hash_table_ref                (DBusHashTable    *table);
void           _dbus_hash_table_unref              (DBusHashTable    *table);
void           _dbus_hash_table_remove_all         (DBusHashTable    *table);
void           _dbus_hash_iter_init                (DBusHashTable    *table,
                                                    DBusHashIter     *iter);
dbus_bool_t    _dbus_hash_iter_next                (DBusHashIter     *iter);
void           _dbus_hash_iter_remove_entry        (DBusHashIter     *iter);
void*          _dbus_hash_iter_get_value           (DBusHashIter     *iter);
void           _dbus_hash_iter_set_value           (DBusHashIter     *iter,
                                                    void             *value);
int            _dbus_hash_iter_get_int_key         (DBusHashIter     *iter);
const char*    _dbus_hash_iter_get_string_key      (DBusHashIter     *iter);
uintptr_t      _dbus_hash_iter_get_uintptr_key     (DBusHashIter     *iter);
dbus_bool_t    _dbus_hash_iter_lookup              (DBusHashTable    *table,
                                                    void             *key,
                                                    dbus_bool_t       create_if_not_found,
                                                    DBusHashIter     *iter);
void*          _dbus_hash_table_lookup_string      (DBusHashTable    *table,
                                                    const char       *key);
void*          _dbus_hash_table_lookup_int         (DBusHashTable    *table,
                                                    int               key);
void*          _dbus_hash_table_lookup_uintptr     (DBusHashTable    *table,
                                                    uintptr_t         key);
dbus_bool_t    _dbus_hash_table_remove_string      (DBusHashTable    *table,
                                                    const char       *key);
dbus_bool_t    _dbus_hash_table_remove_int         (DBusHashTable    *table,
                                                    int               key);
dbus_bool_t    _dbus_hash_table_remove_uintptr     (DBusHashTable    *table,
                                                    uintptr_t         key);
dbus_bool_t    _dbus_hash_table_insert_string      (DBusHashTable    *table,
                                                    char             *key,
                                                    void             *value);
dbus_bool_t    _dbus_hash_table_insert_int         (DBusHashTable    *table,
                                                    int               key,
                                                    void             *value);
dbus_bool_t    _dbus_hash_table_insert_uintptr     (DBusHashTable    *table,
                                                    uintptr_t         key,
                                                    void             *value);
int            _dbus_hash_table_get_n_entries      (DBusHashTable    *table);

/* Preallocation */

/** A preallocated hash entry */
typedef struct DBusPreallocatedHash DBusPreallocatedHash;

DBusPreallocatedHash *_dbus_hash_table_preallocate_entry          (DBusHashTable        *table);
void                  _dbus_hash_table_free_preallocated_entry    (DBusHashTable        *table,
                                                                   DBusPreallocatedHash *preallocated);
void                  _dbus_hash_table_insert_string_preallocated (DBusHashTable        *table,
                                                                   DBusPreallocatedHash *preallocated,
                                                                   char                 *key,
                                                                   void                 *value);

/** @} */

DBUS_END_DECLS

#endif /* DBUS_HASH_H */
