/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-test.h  Declarations of test functions.
 *
 * Copyright (C) 2002  Red Hat Inc.
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

#ifndef DBUS_TEST_H
#define DBUS_TEST_H

#include <dbus/dbus-types.h>
#include <dbus/dbus-string.h>
#include <dbus/dbus-marshal-validate.h>

dbus_bool_t _dbus_hash_test              (void);
dbus_bool_t _dbus_list_test              (void);
dbus_bool_t _dbus_marshal_test           (void);
dbus_bool_t _dbus_marshal_recursive_test (void);
dbus_bool_t _dbus_marshal_byteswap_test  (void);
dbus_bool_t _dbus_marshal_validate_test  (void);
dbus_bool_t _dbus_misc_test              (void);
dbus_bool_t _dbus_signature_test         (void);
dbus_bool_t _dbus_mem_pool_test          (void);
dbus_bool_t _dbus_string_test            (void);
dbus_bool_t _dbus_address_test           (void);
dbus_bool_t _dbus_server_test            (void);
dbus_bool_t _dbus_message_test           (const char *test_data_dir);
dbus_bool_t _dbus_auth_test              (const char *test_data_dir);
dbus_bool_t _dbus_sha_test               (const char *test_data_dir);
dbus_bool_t _dbus_keyring_test           (void);
dbus_bool_t _dbus_data_slot_test         (void);
dbus_bool_t _dbus_sysdeps_test           (void);
dbus_bool_t _dbus_spawn_test             (const char *test_data_dir);
dbus_bool_t _dbus_userdb_test            (const char *test_data_dir);
dbus_bool_t _dbus_transport_unix_test    (void);
dbus_bool_t _dbus_memory_test            (void);
dbus_bool_t _dbus_object_tree_test       (void);
dbus_bool_t _dbus_credentials_test       (const char *test_data_dir);

void        dbus_internal_do_not_use_run_tests         (const char          *test_data_dir,
							const char          *specific_test);
dbus_bool_t dbus_internal_do_not_use_try_message_file  (const DBusString    *filename,
                                                        DBusValidity         expected_validity);
dbus_bool_t dbus_internal_do_not_use_try_message_data  (const DBusString    *data,
                                                        DBusValidity         expected_validity);
dbus_bool_t dbus_internal_do_not_use_load_message_file (const DBusString    *filename,
                                                        DBusString          *data);


/* returns FALSE on fatal failure */
typedef dbus_bool_t (* DBusForeachMessageFileFunc) (const DBusString   *filename,
                                                    DBusValidity        expected_validity,
                                                    void               *data);

dbus_bool_t dbus_internal_do_not_use_foreach_message_file (const char                 *test_data_dir,
                                                           DBusForeachMessageFileFunc  func,
                                                           void                       *user_data);
dbus_bool_t dbus_internal_do_not_use_generate_bodies    (int           sequence,
                                                         int           byte_order,
                                                         DBusString   *signature,
                                                         DBusString   *body);


#endif /* DBUS_TEST_H */
