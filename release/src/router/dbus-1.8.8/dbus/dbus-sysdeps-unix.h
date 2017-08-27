/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-sysdeps-unix.h UNIX-specific wrappers around system/libc features (internal to D-Bus implementation)
 *
 * Copyright (C) 2002, 2003, 2006  Red Hat, Inc.
 * Copyright (C) 2003 CodeFactory AB
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

#ifndef DBUS_SYSDEPS_UNIX_H
#define DBUS_SYSDEPS_UNIX_H

#include <dbus/dbus-sysdeps.h>

#ifdef DBUS_WIN
#error "Don't include this on Windows"
#endif

DBUS_BEGIN_DECLS

/**
 * @defgroup DBusSysdepsUnix UNIX-specific internal API
 * @ingroup DBusInternals
 * @brief Internal system-dependent API available on UNIX only
 * @{
 */

dbus_bool_t
_dbus_close     (int               fd,
                 DBusError        *error);
int _dbus_dup   (int               fd,
                 DBusError        *error);
int
_dbus_read      (int               fd,
                 DBusString       *buffer,
                 int               count);
int
_dbus_write     (int               fd,
                 const DBusString *buffer,
                 int               start,
                 int               len);
int
_dbus_write_two (int               fd,
                 const DBusString *buffer1,
                 int               start1,
                 int               len1,
                 const DBusString *buffer2,
                 int               start2,
                 int               len2);

int _dbus_connect_unix_socket (const char     *path,
                               dbus_bool_t     abstract,
                               DBusError      *error);
int _dbus_listen_unix_socket  (const char     *path,
                               dbus_bool_t     abstract,
                               DBusError      *error);

int _dbus_connect_exec (const char     *path,
                        char *const    argv[],
                        DBusError      *error);

int _dbus_listen_systemd_sockets (int       **fd,
                                 DBusError *error);

dbus_bool_t _dbus_read_credentials (int               client_fd,
                                    DBusCredentials  *credentials,
                                    DBusError        *error);
dbus_bool_t _dbus_send_credentials (int              server_fd,
                                    DBusError       *error);

dbus_bool_t _dbus_lookup_launchd_socket (DBusString *socket_path,
                                         const char *launchd_env_var,
                                         DBusError  *error);

/** Information about a UNIX user */
typedef struct DBusUserInfo  DBusUserInfo;
/** Information about a UNIX group */
typedef struct DBusGroupInfo DBusGroupInfo;

/**
 * Information about a UNIX user
 */
struct DBusUserInfo
{
  dbus_uid_t  uid;            /**< UID */
  dbus_gid_t  primary_gid;    /**< GID */
  dbus_gid_t *group_ids;      /**< Groups IDs, *including* above primary group */
  int         n_group_ids;    /**< Size of group IDs array */
  char       *username;       /**< Username */
  char       *homedir;        /**< Home directory */
};

/**
 * Information about a UNIX group
 */
struct DBusGroupInfo
{
  dbus_gid_t  gid;            /**< GID */
  char       *groupname;      /**< Group name */
};

dbus_bool_t _dbus_user_info_fill     (DBusUserInfo     *info,
                                      const DBusString *username,
                                      DBusError        *error);
dbus_bool_t _dbus_user_info_fill_uid (DBusUserInfo     *info,
                                      dbus_uid_t        uid,
                                      DBusError        *error);
void        _dbus_user_info_free     (DBusUserInfo     *info);

dbus_bool_t _dbus_group_info_fill     (DBusGroupInfo    *info,
                                       const DBusString *groupname,
                                       DBusError        *error);
dbus_bool_t _dbus_group_info_fill_gid (DBusGroupInfo    *info,
                                       dbus_gid_t        gid,
                                       DBusError        *error);
void        _dbus_group_info_free     (DBusGroupInfo    *info);

dbus_uid_t    _dbus_getuid (void);
dbus_uid_t    _dbus_geteuid (void);

dbus_bool_t _dbus_parse_uid (const DBusString  *uid_str,
                             dbus_uid_t        *uid);

void _dbus_close_all (void);

dbus_bool_t _dbus_append_address_from_socket (int         fd,
                                              DBusString *address,
                                              DBusError  *error);

/** @} */

DBUS_END_DECLS

#endif /* DBUS_SYSDEPS_UNIX_H */
