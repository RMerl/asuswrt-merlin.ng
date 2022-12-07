/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Nokia Corporation
 *
 *
 */

#include <dbus/dbus.h>

#define OBEXD_SERVICE  "org.bluez.obex"

struct obex_session;
struct obex_transfer;

void manager_register_session(struct obex_session *os);
void manager_unregister_session(struct obex_session *os);

struct obex_transfer *manager_register_transfer(struct obex_session *os);
void manager_unregister_transfer(struct obex_transfer *transfer);
void manager_emit_transfer_property(struct obex_transfer *transfer,
								char *name);
void manager_emit_transfer_started(struct obex_transfer *transfer);
void manager_emit_transfer_progress(struct obex_transfer *transfer);
void manager_emit_transfer_completed(struct obex_transfer *transfer);
int manager_request_authorization(struct obex_transfer *transfer,
					char **new_folder, char **new_name);

DBusConnection *manager_dbus_get_connection(void);
