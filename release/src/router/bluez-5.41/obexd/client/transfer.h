/*
 *
 *  OBEX Client
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2011-2012  BMW Car IT GmbH. All rights reserved.
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

struct obc_transfer;

typedef void (*transfer_callback_t) (struct obc_transfer *transfer,
					GError *err, void *user_data);

struct obc_transfer *obc_transfer_get(const char *type, const char *name,
					const char *filename, GError **err);
struct obc_transfer *obc_transfer_put(const char *type, const char *name,
					const char *filename,
					const void *contents, size_t size,
					GError **err);

gboolean obc_transfer_register(struct obc_transfer *transfer,
					DBusConnection *conn,
					const char *session,
					const char *owner,
					GError **err);

void obc_transfer_unregister(struct obc_transfer *transfer);

gboolean obc_transfer_set_callback(struct obc_transfer *transfer,
					transfer_callback_t func,
					void *user_data);

gboolean obc_transfer_start(struct obc_transfer *transfer, void *obex,
								GError **err);
guint8 obc_transfer_get_operation(struct obc_transfer *transfer);

void obc_transfer_set_apparam(struct obc_transfer *transfer, void *data);
void *obc_transfer_get_apparam(struct obc_transfer *transfer);
int obc_transfer_get_contents(struct obc_transfer *transfer, char **contents,
								size_t *size);

const char *obc_transfer_get_path(struct obc_transfer *transfer);
gint64 obc_transfer_get_size(struct obc_transfer *transfer);

DBusMessage *obc_transfer_create_dbus_reply(struct obc_transfer *transfer,
							DBusMessage *message);
