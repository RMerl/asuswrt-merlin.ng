/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2009  Joao Paulo Rechi Vita
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
	SOURCE_STATE_DISCONNECTED,
	SOURCE_STATE_CONNECTING,
	SOURCE_STATE_CONNECTED,
	SOURCE_STATE_PLAYING,
} source_state_t;

typedef void (*source_state_cb) (struct btd_service *service,
				source_state_t old_state,
				source_state_t new_state,
				void *user_data);

struct btd_service;

unsigned int source_add_state_cb(struct btd_service *service,
					source_state_cb cb, void *user_data);
gboolean source_remove_state_cb(unsigned int id);

int source_init(struct btd_service *service);
void source_unregister(struct btd_service *service);
int source_connect(struct btd_service *service);
gboolean source_new_stream(struct btd_service *service, struct avdtp *session,
				struct avdtp_stream *stream);
gboolean source_setup_stream(struct btd_service *service,
							struct avdtp *session);
int source_disconnect(struct btd_service *service);
