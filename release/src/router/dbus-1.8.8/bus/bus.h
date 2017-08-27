/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* bus.h  message bus context object
 *
 * Copyright (C) 2003 Red Hat, Inc.
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

#ifndef BUS_BUS_H
#define BUS_BUS_H

#include <dbus/dbus.h>
#include <dbus/dbus-string.h>
#include <dbus/dbus-mainloop.h>
#include <dbus/dbus-pipe.h>
#include <dbus/dbus-sysdeps.h>

typedef struct BusActivation    BusActivation;
typedef struct BusConnections   BusConnections;
typedef struct BusContext       BusContext;
typedef struct BusPolicy        BusPolicy;
typedef struct BusClientPolicy  BusClientPolicy;
typedef struct BusPolicyRule    BusPolicyRule;
typedef struct BusRegistry      BusRegistry;
typedef struct BusSELinuxID     BusSELinuxID;
typedef struct BusService       BusService;
typedef struct BusOwner		BusOwner;
typedef struct BusTransaction   BusTransaction;
typedef struct BusMatchmaker    BusMatchmaker;
typedef struct BusMatchRule     BusMatchRule;

typedef struct
{
  long max_incoming_bytes;          /**< How many incoming message bytes for a single connection */
  long max_incoming_unix_fds;       /**< How many incoming message unix fds for a single connection */
  long max_outgoing_bytes;          /**< How many outgoing bytes can be queued for a single connection */
  long max_outgoing_unix_fds;       /**< How many outgoing unix fds can be queued for a single connection */
  long max_message_size;            /**< Max size of a single message in bytes */
  long max_message_unix_fds;        /**< Max number of unix fds of a single message*/
  int activation_timeout;           /**< How long to wait for an activation to time out */
  int auth_timeout;                 /**< How long to wait for an authentication to time out */
  int pending_fd_timeout;           /**< How long to wait for a D-Bus message with a fd to time out */
  int max_completed_connections;    /**< Max number of authorized connections */
  int max_incomplete_connections;   /**< Max number of incomplete connections */
  int max_connections_per_user;     /**< Max number of connections auth'd as same user */
  int max_pending_activations;      /**< Max number of pending activations for the entire bus */
  int max_services_per_connection;  /**< Max number of owned services for a single connection */
  int max_match_rules_per_connection; /**< Max number of match rules for a single connection */
  int max_replies_per_connection;     /**< Max number of replies that can be pending for each connection */
  int reply_timeout;                  /**< How long to wait before timing out a reply */
} BusLimits;

typedef enum
{
  BUS_CONTEXT_FLAG_NONE = 0,
  BUS_CONTEXT_FLAG_FORK_ALWAYS = (1 << 1),
  BUS_CONTEXT_FLAG_FORK_NEVER = (1 << 2),
  BUS_CONTEXT_FLAG_WRITE_PID_FILE = (1 << 3),
  BUS_CONTEXT_FLAG_SYSTEMD_ACTIVATION = (1 << 4)
} BusContextFlags;

BusContext*       bus_context_new                                (const DBusString *config_file,
                                                                  BusContextFlags   flags,
                                                                  DBusPipe         *print_addr_pipe,
                                                                  DBusPipe         *print_pid_pipe,
                                                                  const DBusString *address,
                                                                  DBusError        *error);
dbus_bool_t       bus_context_reload_config                      (BusContext       *context,
								  DBusError        *error);
void              bus_context_shutdown                           (BusContext       *context);
BusContext*       bus_context_ref                                (BusContext       *context);
void              bus_context_unref                              (BusContext       *context);
dbus_bool_t       bus_context_get_id                             (BusContext       *context,
                                                                  DBusString       *uuid);
const char*       bus_context_get_type                           (BusContext       *context);
const char*       bus_context_get_address                        (BusContext       *context);
const char*       bus_context_get_servicehelper                  (BusContext       *context);
dbus_bool_t       bus_context_get_systemd_activation             (BusContext       *context);
BusRegistry*      bus_context_get_registry                       (BusContext       *context);
BusConnections*   bus_context_get_connections                    (BusContext       *context);
BusActivation*    bus_context_get_activation                     (BusContext       *context);
BusMatchmaker*    bus_context_get_matchmaker                     (BusContext       *context);
DBusLoop*         bus_context_get_loop                           (BusContext       *context);
dbus_bool_t       bus_context_allow_unix_user                    (BusContext       *context,
                                                                  unsigned long     uid);
dbus_bool_t       bus_context_allow_windows_user                 (BusContext       *context,
                                                                  const char       *windows_sid);
BusPolicy*        bus_context_get_policy                         (BusContext       *context);

BusClientPolicy*  bus_context_create_client_policy               (BusContext       *context,
                                                                  DBusConnection   *connection,
                                                                  DBusError        *error);
int               bus_context_get_activation_timeout             (BusContext       *context);
int               bus_context_get_auth_timeout                   (BusContext       *context);
int               bus_context_get_pending_fd_timeout             (BusContext       *context);
int               bus_context_get_max_completed_connections      (BusContext       *context);
int               bus_context_get_max_incomplete_connections     (BusContext       *context);
int               bus_context_get_max_connections_per_user       (BusContext       *context);
int               bus_context_get_max_pending_activations        (BusContext       *context);
int               bus_context_get_max_services_per_connection    (BusContext       *context);
int               bus_context_get_max_match_rules_per_connection (BusContext       *context);
int               bus_context_get_max_replies_per_connection     (BusContext       *context);
int               bus_context_get_reply_timeout                  (BusContext       *context);
void              bus_context_log                                (BusContext       *context,
                                                                  DBusSystemLogSeverity severity,
                                                                  const char       *msg,
                                                                  ...);
dbus_bool_t       bus_context_check_security_policy              (BusContext       *context,
                                                                  BusTransaction   *transaction,
                                                                  DBusConnection   *sender,
                                                                  DBusConnection   *addressed_recipient,
                                                                  DBusConnection   *proposed_recipient,
                                                                  DBusMessage      *message,
                                                                  DBusError        *error);
void              bus_context_check_all_watches                  (BusContext       *context);

#endif /* BUS_BUS_H */
