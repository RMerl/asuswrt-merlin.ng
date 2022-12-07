/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
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
