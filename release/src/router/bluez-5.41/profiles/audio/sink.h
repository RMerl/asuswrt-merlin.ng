/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
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

typedef enum {
	SINK_STATE_DISCONNECTED,
	SINK_STATE_CONNECTING,
	SINK_STATE_CONNECTED,
	SINK_STATE_PLAYING,
} sink_state_t;

typedef void (*sink_state_cb) (struct btd_service *service,
				sink_state_t old_state,
				sink_state_t new_state,
				void *user_data);

struct btd_service;

unsigned int sink_add_state_cb(struct btd_service *service, sink_state_cb cb,
							void *user_data);
gboolean sink_remove_state_cb(unsigned int id);

int sink_init(struct btd_service *service);
void sink_unregister(struct btd_service *service);
gboolean sink_is_active(struct btd_service *service);
int sink_connect(struct btd_service *service);
gboolean sink_new_stream(struct btd_service *service, struct avdtp *session,
				struct avdtp_stream *stream);
gboolean sink_setup_stream(struct btd_service *service, struct avdtp *session);
int sink_disconnect(struct btd_service *service);
