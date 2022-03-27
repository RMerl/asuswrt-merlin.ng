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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <glib.h>
#include <dbus/dbus.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"

#include "gdbus/gdbus.h"

#include "src/log.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/service.h"
#include "src/error.h"
#include "src/dbus-common.h"
#include "src/shared/queue.h"

#include "avdtp.h"
#include "media.h"
#include "a2dp.h"
#include "sink.h"

#define STREAM_SETUP_RETRY_TIMER 2

struct sink {
	struct btd_service *service;
	struct avdtp *session;
	struct avdtp_stream *stream;
	unsigned int cb_id;
	avdtp_session_state_t session_state;
	avdtp_state_t stream_state;
	sink_state_t state;
	unsigned int connect_id;
	unsigned int disconnect_id;
	unsigned int avdtp_callback_id;
};

struct sink_state_callback {
	sink_state_cb cb;
	struct btd_service *service;
	void *user_data;
	unsigned int id;
};

static GSList *sink_callbacks = NULL;

static char *str_state[] = {
	"SINK_STATE_DISCONNECTED",
	"SINK_STATE_CONNECTING",
	"SINK_STATE_CONNECTED",
	"SINK_STATE_PLAYING",
};

static void sink_set_state(struct sink *sink, sink_state_t new_state)
{
	struct btd_service *service = sink->service;
	struct btd_device *dev = btd_service_get_device(service);
	sink_state_t old_state = sink->state;
	GSList *l;

	sink->state = new_state;

	DBG("State changed %s: %s -> %s", device_get_path(dev),
				str_state[old_state], str_state[new_state]);

	for (l = sink_callbacks; l != NULL; l = l->next) {
		struct sink_state_callback *cb = l->data;

		if (cb->service != service)
			continue;

		cb->cb(service, old_state, new_state, cb->user_data);
	}

	if (new_state != SINK_STATE_DISCONNECTED)
		return;

	if (sink->session) {
		avdtp_unref(sink->session);
		sink->session = NULL;
	}
}

static void avdtp_state_callback(struct btd_device *dev,
					struct avdtp *session,
					avdtp_session_state_t old_state,
					avdtp_session_state_t new_state,
					void *user_data)
{
	struct sink *sink = user_data;

	switch (new_state) {
	case AVDTP_SESSION_STATE_DISCONNECTED:
		sink_set_state(sink, SINK_STATE_DISCONNECTED);
		break;
	case AVDTP_SESSION_STATE_CONNECTING:
		sink_set_state(sink, SINK_STATE_CONNECTING);
		break;
	case AVDTP_SESSION_STATE_CONNECTED:
		break;
	}

	sink->session_state = new_state;
}

static void stream_state_changed(struct avdtp_stream *stream,
					avdtp_state_t old_state,
					avdtp_state_t new_state,
					struct avdtp_error *err,
					void *user_data)
{
	struct btd_service *service = user_data;
	struct sink *sink = btd_service_get_user_data(service);

	if (err)
		return;

	switch (new_state) {
	case AVDTP_STATE_IDLE:
		btd_service_disconnecting_complete(sink->service, 0);

		if (sink->disconnect_id > 0) {
			a2dp_cancel(sink->disconnect_id);
			sink->disconnect_id = 0;
		}

		if (sink->session) {
			avdtp_unref(sink->session);
			sink->session = NULL;
		}
		sink->stream = NULL;
		sink->cb_id = 0;
		break;
	case AVDTP_STATE_OPEN:
		btd_service_connecting_complete(sink->service, 0);
		sink_set_state(sink, SINK_STATE_CONNECTED);
		break;
	case AVDTP_STATE_STREAMING:
		sink_set_state(sink, SINK_STATE_PLAYING);
		break;
	case AVDTP_STATE_CONFIGURED:
	case AVDTP_STATE_CLOSING:
	case AVDTP_STATE_ABORTING:
	default:
		break;
	}

	sink->stream_state = new_state;
}

static void stream_setup_complete(struct avdtp *session, struct a2dp_sep *sep,
					struct avdtp_stream *stream, int err,
					void *user_data)
{
	struct sink *sink = user_data;

	sink->connect_id = 0;

	if (stream)
		return;

	avdtp_unref(sink->session);
	sink->session = NULL;
	btd_service_connecting_complete(sink->service, err);
}

static void select_complete(struct avdtp *session, struct a2dp_sep *sep,
			GSList *caps, void *user_data)
{
	struct sink *sink = user_data;
	int id;

	sink->connect_id = 0;

	id = a2dp_config(session, sep, stream_setup_complete, caps, sink);
	if (id == 0)
		goto failed;

	sink->connect_id = id;
	return;

failed:
	btd_service_connecting_complete(sink->service, -EIO);

	avdtp_unref(sink->session);
	sink->session = NULL;
}

static void discovery_complete(struct avdtp *session, GSList *seps, int err,
							void *user_data)
{
	struct sink *sink = user_data;
	int id;

	sink->connect_id = 0;

	if (err) {
		avdtp_unref(sink->session);
		sink->session = NULL;
		goto failed;
	}

	DBG("Discovery complete");

	id = a2dp_select_capabilities(sink->session, AVDTP_SEP_TYPE_SINK, NULL,
						select_complete, sink);
	if (id == 0) {
		err = -EIO;
		goto failed;
	}

	sink->connect_id = id;
	return;

failed:
	btd_service_connecting_complete(sink->service, err);
	avdtp_unref(sink->session);
	sink->session = NULL;
}

gboolean sink_setup_stream(struct btd_service *service, struct avdtp *session)
{
	struct sink *sink = btd_service_get_user_data(service);

	if (sink->connect_id > 0 || sink->disconnect_id > 0)
		return FALSE;

	if (session && !sink->session)
		sink->session = avdtp_ref(session);

	if (!sink->session)
		return FALSE;

	sink->connect_id = a2dp_discover(sink->session, discovery_complete,
								sink);
	if (sink->connect_id == 0)
		return FALSE;

	return TRUE;
}

int sink_connect(struct btd_service *service)
{
	struct sink *sink = btd_service_get_user_data(service);

	if (!sink->session)
		sink->session = a2dp_avdtp_get(btd_service_get_device(service));

	if (!sink->session) {
		DBG("Unable to get a session");
		return -EIO;
	}

	if (sink->connect_id > 0 || sink->disconnect_id > 0)
		return -EBUSY;

	if (sink->state == SINK_STATE_CONNECTING)
		return -EBUSY;

	if (sink->stream_state >= AVDTP_STATE_OPEN)
		return -EALREADY;

	if (!sink_setup_stream(service, NULL)) {
		DBG("Failed to create a stream");
		return -EIO;
	}

	DBG("stream creation in progress");

	return 0;
}

static void sink_free(struct btd_service *service)
{
	struct sink *sink = btd_service_get_user_data(service);

	if (sink->cb_id)
		avdtp_stream_remove_cb(sink->session, sink->stream,
					sink->cb_id);

	if (sink->session)
		avdtp_unref(sink->session);

	if (sink->connect_id > 0) {
		btd_service_connecting_complete(sink->service, -ECANCELED);
		a2dp_cancel(sink->connect_id);
		sink->connect_id = 0;
	}

	if (sink->disconnect_id > 0) {
		btd_service_disconnecting_complete(sink->service, -ECANCELED);
		a2dp_cancel(sink->disconnect_id);
		sink->disconnect_id = 0;
	}

	avdtp_remove_state_cb(sink->avdtp_callback_id);
	btd_service_unref(sink->service);

	g_free(sink);
}

void sink_unregister(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);

	DBG("%s", device_get_path(dev));

	sink_free(service);
}

int sink_init(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);
	struct sink *sink;

	DBG("%s", device_get_path(dev));

	sink = g_new0(struct sink, 1);

	sink->service = btd_service_ref(service);

	sink->avdtp_callback_id = avdtp_add_state_cb(dev, avdtp_state_callback,
									sink);

	btd_service_set_user_data(service, sink);

	return 0;
}

gboolean sink_is_active(struct btd_service *service)
{
	struct sink *sink = btd_service_get_user_data(service);

	if (sink->session)
		return TRUE;

	return FALSE;
}

gboolean sink_new_stream(struct btd_service *service, struct avdtp *session,
				struct avdtp_stream *stream)
{
	struct sink *sink = btd_service_get_user_data(service);

	if (sink->stream)
		return FALSE;

	if (!sink->session)
		sink->session = avdtp_ref(session);

	sink->stream = stream;

	sink->cb_id = avdtp_stream_add_cb(session, stream,
						stream_state_changed, service);

	return TRUE;
}

int sink_disconnect(struct btd_service *service)
{
	struct sink *sink = btd_service_get_user_data(service);

	if (!sink->session)
		return -ENOTCONN;

	/* cancel pending connect */
	if (sink->connect_id > 0) {
		avdtp_unref(sink->session);
		sink->session = NULL;

		a2dp_cancel(sink->connect_id);
		sink->connect_id = 0;
		btd_service_disconnecting_complete(sink->service, 0);

		return 0;
	}

	/* disconnect already ongoing */
	if (sink->disconnect_id > 0)
		return -EBUSY;

	if (!sink->stream)
		return -ENOTCONN;

	return avdtp_close(sink->session, sink->stream, FALSE);
}

unsigned int sink_add_state_cb(struct btd_service *service, sink_state_cb cb,
								void *user_data)
{
	struct sink_state_callback *state_cb;
	static unsigned int id = 0;

	state_cb = g_new(struct sink_state_callback, 1);
	state_cb->cb = cb;
	state_cb->service = service;
	state_cb->user_data = user_data;
	state_cb->id = ++id;

	sink_callbacks = g_slist_append(sink_callbacks, state_cb);

	return state_cb->id;
}

gboolean sink_remove_state_cb(unsigned int id)
{
	GSList *l;

	for (l = sink_callbacks; l != NULL; l = l->next) {
		struct sink_state_callback *cb = l->data;
		if (cb && cb->id == id) {
			sink_callbacks = g_slist_remove(sink_callbacks, cb);
			g_free(cb);
			return TRUE;
		}
	}

	return FALSE;
}
