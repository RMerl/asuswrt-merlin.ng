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
#include "source.h"

struct source {
	struct btd_service *service;
	struct avdtp *session;
	struct avdtp_stream *stream;
	unsigned int cb_id;
	avdtp_session_state_t session_state;
	avdtp_state_t stream_state;
	source_state_t state;
	unsigned int connect_id;
	unsigned int disconnect_id;
	unsigned int avdtp_callback_id;
};

struct source_state_callback {
	source_state_cb cb;
	struct btd_service *service;
	void *user_data;
	unsigned int id;
};

static GSList *source_callbacks = NULL;

static char *str_state[] = {
	"SOURCE_STATE_DISCONNECTED",
	"SOURCE_STATE_CONNECTING",
	"SOURCE_STATE_CONNECTED",
	"SOURCE_STATE_PLAYING",
};

static void source_set_state(struct source *source, source_state_t new_state)
{
	struct btd_device *dev = btd_service_get_device(source->service);
	source_state_t old_state = source->state;
	GSList *l;

	source->state = new_state;

	DBG("State changed %s: %s -> %s", device_get_path(dev),
				str_state[old_state], str_state[new_state]);

	for (l = source_callbacks; l != NULL; l = l->next) {
		struct source_state_callback *cb = l->data;

		if (cb->service != source->service)
			continue;

		cb->cb(source->service, old_state, new_state, cb->user_data);
	}

	if (new_state != SOURCE_STATE_DISCONNECTED)
		return;

	if (source->session) {
		avdtp_unref(source->session);
		source->session = NULL;
	}
}

static void avdtp_state_callback(struct btd_device *dev, struct avdtp *session,
					avdtp_session_state_t old_state,
					avdtp_session_state_t new_state,
					void *user_data)
{
	struct source *source = user_data;

	switch (new_state) {
	case AVDTP_SESSION_STATE_DISCONNECTED:
		source_set_state(source, SOURCE_STATE_DISCONNECTED);
		break;
	case AVDTP_SESSION_STATE_CONNECTING:
		source_set_state(source, SOURCE_STATE_CONNECTING);
		break;
	case AVDTP_SESSION_STATE_CONNECTED:
		break;
	}

	source->session_state = new_state;
}

static void stream_state_changed(struct avdtp_stream *stream,
					avdtp_state_t old_state,
					avdtp_state_t new_state,
					struct avdtp_error *err,
					void *user_data)
{
	struct btd_service *service = user_data;
	struct source *source = btd_service_get_user_data(service);

	if (err)
		return;

	switch (new_state) {
	case AVDTP_STATE_IDLE:
		btd_service_disconnecting_complete(source->service, 0);

		if (source->disconnect_id > 0) {
			a2dp_cancel(source->disconnect_id);
			source->disconnect_id = 0;
		}

		if (source->session) {
			avdtp_unref(source->session);
			source->session = NULL;
		}
		source->stream = NULL;
		source->cb_id = 0;
		break;
	case AVDTP_STATE_OPEN:
		btd_service_connecting_complete(source->service, 0);
		source_set_state(source, SOURCE_STATE_CONNECTED);
		break;
	case AVDTP_STATE_STREAMING:
		source_set_state(source, SOURCE_STATE_PLAYING);
		break;
	case AVDTP_STATE_CONFIGURED:
	case AVDTP_STATE_CLOSING:
	case AVDTP_STATE_ABORTING:
	default:
		break;
	}

	source->stream_state = new_state;
}

static void stream_setup_complete(struct avdtp *session, struct a2dp_sep *sep,
					struct avdtp_stream *stream, int err,
					void *user_data)
{
	struct source *source = user_data;

	source->connect_id = 0;

	if (stream)
		return;

	avdtp_unref(source->session);
	source->session = NULL;
	btd_service_connecting_complete(source->service, err);
}

static void select_complete(struct avdtp *session, struct a2dp_sep *sep,
			GSList *caps, void *user_data)
{
	struct source *source = user_data;
	int id;

	source->connect_id = 0;

	if (caps == NULL)
		goto failed;

	id = a2dp_config(session, sep, stream_setup_complete, caps, source);
	if (id == 0)
		goto failed;

	source->connect_id = id;
	return;

failed:
	btd_service_connecting_complete(source->service, -EIO);

	avdtp_unref(source->session);
	source->session = NULL;
}

static void discovery_complete(struct avdtp *session, GSList *seps, int err,
							void *user_data)
{
	struct source *source = user_data;
	int id;

	source->connect_id = 0;

	if (err) {
		avdtp_unref(source->session);
		source->session = NULL;
		goto failed;
	}

	DBG("Discovery complete");

	id = a2dp_select_capabilities(source->session, AVDTP_SEP_TYPE_SOURCE,
					NULL, select_complete, source);
	if (id == 0) {
		err = -EIO;
		goto failed;
	}

	source->connect_id = id;
	return;

failed:
	btd_service_connecting_complete(source->service, err);
	avdtp_unref(source->session);
	source->session = NULL;
}

gboolean source_setup_stream(struct btd_service *service,
							struct avdtp *session)
{
	struct source *source = btd_service_get_user_data(service);

	if (source->connect_id > 0 || source->disconnect_id > 0)
		return FALSE;

	if (session && !source->session)
		source->session = avdtp_ref(session);

	if (!source->session)
		return FALSE;

	source->connect_id = a2dp_discover(source->session, discovery_complete,
								source);
	if (source->connect_id == 0)
		return FALSE;

	return TRUE;
}

int source_connect(struct btd_service *service)
{
	struct source *source = btd_service_get_user_data(service);

	if (!source->session)
		source->session = a2dp_avdtp_get(btd_service_get_device(service));

	if (!source->session) {
		DBG("Unable to get a session");
		return -EIO;
	}

	if (source->connect_id > 0 || source->disconnect_id > 0)
		return -EBUSY;

	if (source->state == SOURCE_STATE_CONNECTING)
		return -EBUSY;

	if (source->stream_state >= AVDTP_STATE_OPEN)
		return -EALREADY;

	if (!source_setup_stream(service, NULL)) {
		DBG("Failed to create a stream");
		return -EIO;
	}

	DBG("stream creation in progress");

	return 0;
}

static void source_free(struct btd_service *service)
{
	struct source *source = btd_service_get_user_data(service);

	if (source->cb_id)
		avdtp_stream_remove_cb(source->session, source->stream,
					source->cb_id);

	if (source->session)
		avdtp_unref(source->session);

	if (source->connect_id > 0) {
		btd_service_connecting_complete(source->service, -ECANCELED);
		a2dp_cancel(source->connect_id);
		source->connect_id = 0;
	}

	if (source->disconnect_id > 0) {
		btd_service_disconnecting_complete(source->service, -ECANCELED);
		a2dp_cancel(source->disconnect_id);
		source->disconnect_id = 0;
	}

	avdtp_remove_state_cb(source->avdtp_callback_id);
	btd_service_unref(source->service);

	g_free(source);
}

void source_unregister(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);

	DBG("%s", device_get_path(dev));

	source_free(service);
}

int source_init(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);
	struct source *source;

	DBG("%s", device_get_path(dev));

	source = g_new0(struct source, 1);

	source->service = btd_service_ref(service);

	source->avdtp_callback_id = avdtp_add_state_cb(dev,
							avdtp_state_callback,
							source);

	btd_service_set_user_data(service, source);

	return 0;
}

gboolean source_new_stream(struct btd_service *service, struct avdtp *session,
				struct avdtp_stream *stream)
{
	struct source *source = btd_service_get_user_data(service);

	if (source->stream)
		return FALSE;

	if (!source->session)
		source->session = avdtp_ref(session);

	source->stream = stream;

	source->cb_id = avdtp_stream_add_cb(session, stream,
						stream_state_changed, service);

	return TRUE;
}

int source_disconnect(struct btd_service *service)
{
	struct source *source = btd_service_get_user_data(service);

	if (!source->session)
		return -ENOTCONN;

	/* cancel pending connect */
	if (source->connect_id > 0) {
		avdtp_unref(source->session);
		source->session = NULL;

		a2dp_cancel(source->connect_id);
		source->connect_id = 0;
		btd_service_disconnecting_complete(source->service, 0);

		return 0;
	}

	/* disconnect already ongoing */
	if (source->disconnect_id > 0)
		return -EBUSY;

	if (!source->stream)
		return -ENOTCONN;

	return avdtp_close(source->session, source->stream, FALSE);
}

unsigned int source_add_state_cb(struct btd_service *service,
					source_state_cb cb, void *user_data)
{
	struct source_state_callback *state_cb;
	static unsigned int id = 0;

	state_cb = g_new(struct source_state_callback, 1);
	state_cb->cb = cb;
	state_cb->service = service;
	state_cb->user_data = user_data;
	state_cb->id = ++id;

	source_callbacks = g_slist_append(source_callbacks, state_cb);

	return state_cb->id;
}

gboolean source_remove_state_cb(unsigned int id)
{
	GSList *l;

	for (l = source_callbacks; l != NULL; l = l->next) {
		struct source_state_callback *cb = l->data;
		if (cb && cb->id == id) {
			source_callbacks = g_slist_remove(source_callbacks, cb);
			g_free(cb);
			return TRUE;
		}
	}

	return FALSE;
}
