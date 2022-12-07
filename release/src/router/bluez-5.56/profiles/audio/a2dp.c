// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2011  BMW Car IT GmbH. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <dbus/dbus.h>
#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "src/btd.h"
#include "src/plugin.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/dbus-common.h"
#include "src/error.h"
#include "src/profile.h"
#include "src/service.h"
#include "src/log.h"
#include "src/sdpd.h"
#include "src/shared/queue.h"
#include "src/shared/util.h"

#include "btio/btio.h"

#include "avdtp.h"
#include "sink.h"
#include "source.h"
#include "a2dp.h"
#include "a2dp-codecs.h"
#include "media.h"

/* The duration that streams without users are allowed to stay in
 * STREAMING state. */
#define SUSPEND_TIMEOUT 5
#define RECONFIGURE_TIMEOUT 500

#define AVDTP_PSM 25

#define MEDIA_ENDPOINT_INTERFACE "org.bluez.MediaEndpoint1"

struct a2dp_sep {
	struct a2dp_server *server;
	struct a2dp_endpoint *endpoint;
	uint8_t type;
	uint8_t codec;
	struct avdtp_local_sep *lsep;
	struct avdtp *session;
	struct avdtp_stream *stream;
	guint suspend_timer;
	gboolean delay_reporting;
	gboolean locked;
	gboolean suspending;
	gboolean starting;
	void *user_data;
	GDestroyNotify destroy;
};

struct a2dp_setup_cb {
	struct a2dp_setup *setup;
	a2dp_discover_cb_t discover_cb;
	a2dp_select_cb_t select_cb;
	a2dp_config_cb_t config_cb;
	a2dp_stream_cb_t resume_cb;
	a2dp_stream_cb_t suspend_cb;
	guint source_id;
	void *user_data;
	unsigned int id;
};

struct a2dp_setup {
	struct a2dp_channel *chan;
	struct avdtp *session;
	struct queue *eps;
	struct a2dp_sep *sep;
	struct a2dp_remote_sep *rsep;
	struct avdtp_stream *stream;
	struct avdtp_error *err;
	avdtp_set_configuration_cb setconf_cb;
	GSList *seps;
	GSList *caps;
	gboolean reconfigure;
	gboolean start;
	GSList *cb;
	GIOChannel *io;
	guint id;
	int ref;
};

struct a2dp_server {
	struct btd_adapter *adapter;
	GSList *sinks;
	GSList *sources;
	uint32_t source_record_id;
	uint32_t sink_record_id;
	gboolean sink_enabled;
	gboolean source_enabled;
	GIOChannel *io;
	struct queue *seps;
	struct queue *channels;
};

struct a2dp_remote_sep {
	struct a2dp_channel *chan;
	char *path;
	struct avdtp_remote_sep *sep;
};

struct a2dp_last_used {
	struct a2dp_sep *lsep;
	struct a2dp_remote_sep *rsep;
};

struct a2dp_channel {
	struct a2dp_server *server;
	struct btd_device *device;
	GIOChannel *io;
	guint io_id;
	unsigned int state_id;
	unsigned int auth_id;
	struct avdtp *session;
	struct queue *seps;
	struct a2dp_last_used *last_used;
};

static GSList *servers = NULL;
static GSList *setups = NULL;
static unsigned int cb_id = 0;

static struct a2dp_setup *setup_ref(struct a2dp_setup *setup)
{
	setup->ref++;

	DBG("%p: ref=%d", setup, setup->ref);

	return setup;
}

static bool match_by_session(const void *data, const void *user_data)
{
	const struct a2dp_channel *chan = data;
	const struct avdtp *session = user_data;

	return chan->session == session;
}

static struct a2dp_channel *find_channel(struct avdtp *session)
{
	GSList *l;

	for (l = servers; l; l = g_slist_next(l)) {
		struct a2dp_server *server = l->data;
		struct a2dp_channel *chan;

		chan = queue_find(server->channels, match_by_session, session);
		if (chan)
			return chan;
	}

	return NULL;
}

static struct a2dp_setup *setup_new(struct avdtp *session)
{
	struct a2dp_setup *setup;
	struct a2dp_channel *chan;

	chan = find_channel(session);
	if (!chan)
		return NULL;

	setup = g_new0(struct a2dp_setup, 1);
	setup->session = avdtp_ref(session);
	setup->chan = find_channel(session);
	setups = g_slist_append(setups, setup);

	return setup;
}

static void setup_free(struct a2dp_setup *s)
{
	DBG("%p", s);

	if (s->io) {
		g_io_channel_shutdown(s->io, TRUE, NULL);
		g_io_channel_unref(s->io);
	}

	if (s->id)
		g_source_remove(s->id);

	queue_destroy(s->eps, NULL);

	setups = g_slist_remove(setups, s);
	if (s->session)
		avdtp_unref(s->session);
	g_slist_free_full(s->cb, g_free);
	g_slist_free_full(s->caps, g_free);
	g_free(s);
}

static void setup_unref(struct a2dp_setup *setup)
{
	setup->ref--;

	DBG("%p: ref=%d", setup, setup->ref);

	if (setup->ref > 0)
		return;

	setup_free(setup);
}

static struct a2dp_setup_cb *setup_cb_new(struct a2dp_setup *setup)
{
	struct a2dp_setup_cb *cb;

	cb = g_new0(struct a2dp_setup_cb, 1);
	cb->setup = setup;
	cb->id = ++cb_id;

	setup->cb = g_slist_append(setup->cb, cb);
	return cb;
}

static void setup_cb_free(struct a2dp_setup_cb *cb)
{
	struct a2dp_setup *setup = cb->setup;

	if (cb->source_id)
		g_source_remove(cb->source_id);

	setup->cb = g_slist_remove(setup->cb, cb);
	setup_unref(cb->setup);
	g_free(cb);
}

static void finalize_setup_errno(struct a2dp_setup *s, int err,
					GSourceFunc cb1, ...)
{
	GSourceFunc finalize;
	va_list args;
	struct avdtp_error avdtp_err;

	if (err < 0) {
		avdtp_error_init(&avdtp_err, AVDTP_ERRNO, -err);
		s->err = &avdtp_err;
	}

	va_start(args, cb1);
	finalize = cb1;
	setup_ref(s);
	while (finalize != NULL) {
		finalize(s);
		finalize = va_arg(args, GSourceFunc);
	}
	setup_unref(s);
	va_end(args);
}

static int error_to_errno(struct avdtp_error *err)
{
	int perr;

	if (!err)
		return 0;

	if (avdtp_error_category(err) != AVDTP_ERRNO)
		return -EIO;

	perr = avdtp_error_posix_errno(err);
	switch (perr) {
	case EHOSTDOWN:
	case ECONNABORTED:
		return -perr;
	default:
		/*
		 * An unexpect error has occurred setup may be attempted again.
		 */
		return -EAGAIN;
	}
}

static gboolean finalize_config(gpointer data)
{
	struct a2dp_setup *s = data;
	GSList *l;
	struct avdtp_stream *stream = s->err ? NULL : s->stream;

	for (l = s->cb; l != NULL; ) {
		struct a2dp_setup_cb *cb = l->data;

		l = l->next;

		if (!cb->config_cb)
			continue;

		cb->config_cb(s->session, s->sep, stream,
				error_to_errno(s->err), cb->user_data);
		setup_cb_free(cb);
	}

	return FALSE;
}

static gboolean finalize_resume(gpointer data)
{
	struct a2dp_setup *s = data;
	GSList *l;

	for (l = s->cb; l != NULL; ) {
		struct a2dp_setup_cb *cb = l->data;

		l = l->next;

		if (!cb->resume_cb)
			continue;

		cb->resume_cb(s->session, error_to_errno(s->err),
							cb->user_data);
		setup_cb_free(cb);
	}

	return FALSE;
}

static gboolean finalize_suspend(gpointer data)
{
	struct a2dp_setup *s = data;
	GSList *l;

	for (l = s->cb; l != NULL; ) {
		struct a2dp_setup_cb *cb = l->data;

		l = l->next;

		if (!cb->suspend_cb)
			continue;

		cb->suspend_cb(s->session, error_to_errno(s->err),
							cb->user_data);
		setup_cb_free(cb);
	}

	return FALSE;
}

static void finalize_select(struct a2dp_setup *s)
{
	GSList *l;

	for (l = s->cb; l != NULL; ) {
		struct a2dp_setup_cb *cb = l->data;

		l = l->next;

		if (!cb->select_cb)
			continue;

		cb->select_cb(s->session, s->sep, s->caps,
					error_to_errno(s->err), cb->user_data);
		setup_cb_free(cb);
	}
}

static void finalize_discover(struct a2dp_setup *s)
{
	GSList *l;

	for (l = s->cb; l != NULL; ) {
		struct a2dp_setup_cb *cb = l->data;

		l = l->next;

		if (!cb->discover_cb)
			continue;

		cb->discover_cb(s->session, s->seps, error_to_errno(s->err),
								cb->user_data);
		setup_cb_free(cb);
	}
}

static struct a2dp_setup *find_setup_by_session(struct avdtp *session)
{
	GSList *l;

	for (l = setups; l != NULL; l = l->next) {
		struct a2dp_setup *setup = l->data;

		if (setup->session == session)
			return setup;
	}

	return NULL;
}

static struct a2dp_setup *a2dp_setup_get(struct avdtp *session)
{
	struct a2dp_setup *setup;

	setup = find_setup_by_session(session);
	if (!setup) {
		setup = setup_new(session);
		if (!setup)
			return NULL;
	}

	return setup_ref(setup);
}

static struct a2dp_setup *find_setup_by_stream(struct avdtp_stream *stream)
{
	GSList *l;

	for (l = setups; l != NULL; l = l->next) {
		struct a2dp_setup *setup = l->data;

		if (setup->stream == stream)
			return setup;
	}

	return NULL;
}

static void stream_state_changed(struct avdtp_stream *stream,
					avdtp_state_t old_state,
					avdtp_state_t new_state,
					struct avdtp_error *err,
					void *user_data)
{
	struct a2dp_sep *sep = user_data;

	if (new_state == AVDTP_STATE_OPEN) {
		struct a2dp_setup *setup;
		int err;

		setup = find_setup_by_stream(stream);
		if (!setup || !setup->start || setup->err)
			return;

		setup->start = FALSE;

		err = avdtp_start(setup->session, stream);
		if (err < 0 && err != -EINPROGRESS) {
			error("avdtp_start: %s (%d)", strerror(-err), -err);
			finalize_setup_errno(setup, err, finalize_resume,
									NULL);
			return;
		}

		sep->starting = TRUE;

		return;
	}

	if (new_state != AVDTP_STATE_IDLE)
		return;

	if (sep->suspend_timer) {
		g_source_remove(sep->suspend_timer);
		sep->suspend_timer = 0;
	}

	if (sep->session) {
		avdtp_unref(sep->session);
		sep->session = NULL;
	}

	sep->stream = NULL;

	if (sep->endpoint && sep->endpoint->clear_configuration)
		sep->endpoint->clear_configuration(sep, sep->user_data);
}

static gboolean auto_config(gpointer data)
{
	struct a2dp_setup *setup = data;
	struct btd_device *dev = NULL;
	struct btd_service *service;

	/* Check if configuration was aborted */
	if (setup->sep->stream == NULL)
		return FALSE;

	if (setup->err != NULL)
		goto done;

	dev = avdtp_get_device(setup->session);

	avdtp_stream_add_cb(setup->session, setup->stream,
				stream_state_changed, setup->sep);

	if (setup->sep->type == AVDTP_SEP_TYPE_SOURCE) {
		service = btd_device_get_service(dev, A2DP_SINK_UUID);
		sink_new_stream(service, setup->session, setup->stream);
	} else {
		service = btd_device_get_service(dev, A2DP_SOURCE_UUID);
		source_new_stream(service, setup->session, setup->stream);
	}

done:
	if (setup->setconf_cb)
		setup->setconf_cb(setup->session, setup->stream, setup->err);

	finalize_config(setup);

	if (setup->err) {
		g_free(setup->err);
		setup->err = NULL;
	}

	setup_unref(setup);

	return FALSE;
}

static void endpoint_setconf_cb(struct a2dp_setup *setup, gboolean ret)
{
	if (ret == FALSE) {
		setup->err = g_new(struct avdtp_error, 1);
		avdtp_error_init(setup->err, AVDTP_MEDIA_CODEC,
					AVDTP_UNSUPPORTED_CONFIGURATION);
	}

	auto_config(setup);
	setup_unref(setup);
}

static gboolean endpoint_match_codec_ind(struct avdtp *session,
				struct avdtp_media_codec_capability *codec,
				void *user_data)
{
	struct a2dp_sep *sep = user_data;
	a2dp_vendor_codec_t *remote_codec;
	a2dp_vendor_codec_t *local_codec;
	uint8_t *capabilities;
	size_t length;

	if (codec->media_codec_type != A2DP_CODEC_VENDOR)
		return TRUE;

	if (sep->endpoint == NULL)
		return FALSE;

	length = sep->endpoint->get_capabilities(sep, &capabilities,
							sep->user_data);
	if (length < sizeof(a2dp_vendor_codec_t))
		return FALSE;

	local_codec = (a2dp_vendor_codec_t *) capabilities;
	remote_codec = (a2dp_vendor_codec_t *) codec->data;

	if (A2DP_GET_VENDOR_ID(*remote_codec) !=
			A2DP_GET_VENDOR_ID(*local_codec))
		return FALSE;

	if (A2DP_GET_CODEC_ID(*remote_codec) != A2DP_GET_CODEC_ID(*local_codec))
		return FALSE;

	DBG("vendor 0x%08x codec 0x%04x", A2DP_GET_VENDOR_ID(*remote_codec),
					A2DP_GET_CODEC_ID(*remote_codec));
	return TRUE;
}

static void reverse_discover(struct avdtp *session, GSList *seps, int err,
							void *user_data)
{
	DBG("err %d", err);
}

static gboolean endpoint_setconf_ind(struct avdtp *session,
						struct avdtp_local_sep *sep,
						struct avdtp_stream *stream,
						GSList *caps,
						avdtp_set_configuration_cb cb,
						void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct a2dp_setup *setup;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Set_Configuration_Ind", sep);
	else
		DBG("Source %p: Set_Configuration_Ind", sep);

	setup = a2dp_setup_get(session);
	if (!setup)
		return FALSE;

	a2dp_sep->stream = stream;
	setup->sep = a2dp_sep;
	setup->stream = stream;
	setup->setconf_cb = cb;

	for (; caps != NULL; caps = g_slist_next(caps)) {
		struct avdtp_service_capability *cap = caps->data;
		struct avdtp_media_codec_capability *codec;
		gboolean ret;

		if (cap->category == AVDTP_DELAY_REPORTING &&
					!a2dp_sep->delay_reporting) {
			setup->err = g_new(struct avdtp_error, 1);
			avdtp_error_init(setup->err, AVDTP_DELAY_REPORTING,
					AVDTP_UNSUPPORTED_CONFIGURATION);
			goto done;
		}

		if (cap->category != AVDTP_MEDIA_CODEC)
			continue;

		codec = (struct avdtp_media_codec_capability *) cap->data;

		if (codec->media_codec_type != a2dp_sep->codec) {
			setup->err = g_new(struct avdtp_error, 1);
			avdtp_error_init(setup->err, AVDTP_MEDIA_CODEC,
					AVDTP_UNSUPPORTED_CONFIGURATION);
			goto done;
		}

		ret = a2dp_sep->endpoint->set_configuration(a2dp_sep,
						codec->data,
						cap->length - sizeof(*codec),
						setup_ref(setup),
						endpoint_setconf_cb,
						a2dp_sep->user_data);
		if (ret == 0) {
			/* Attempt to reverse discover if there are no remote
			 * SEPs.
			 */
			if (queue_isempty(setup->chan->seps))
				a2dp_discover(session, reverse_discover, NULL);
			return TRUE;
		}

		setup_unref(setup);
		setup->err = g_new(struct avdtp_error, 1);
		avdtp_error_init(setup->err, AVDTP_MEDIA_CODEC,
					AVDTP_UNSUPPORTED_CONFIGURATION);
		break;
	}

done:
	g_idle_add(auto_config, setup);
	return TRUE;
}

static gboolean endpoint_getcap_ind(struct avdtp *session,
					struct avdtp_local_sep *sep,
					gboolean get_all, GSList **caps,
					uint8_t *err, void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct avdtp_service_capability *media_transport, *media_codec;
	struct avdtp_media_codec_capability *codec_caps;
	uint8_t *capabilities;
	size_t length;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Get_Capability_Ind", sep);
	else
		DBG("Source %p: Get_Capability_Ind", sep);

	*caps = NULL;

	media_transport = avdtp_service_cap_new(AVDTP_MEDIA_TRANSPORT,
						NULL, 0);

	*caps = g_slist_append(*caps, media_transport);

	length = a2dp_sep->endpoint->get_capabilities(a2dp_sep, &capabilities,
							a2dp_sep->user_data);

	codec_caps = g_malloc0(sizeof(*codec_caps) + length);
	codec_caps->media_type = AVDTP_MEDIA_TYPE_AUDIO;
	codec_caps->media_codec_type = a2dp_sep->codec;
	memcpy(codec_caps->data, capabilities, length);

	media_codec = avdtp_service_cap_new(AVDTP_MEDIA_CODEC, codec_caps,
						sizeof(*codec_caps) + length);

	*caps = g_slist_append(*caps, media_codec);
	g_free(codec_caps);

	if (get_all) {
		struct avdtp_service_capability *delay_reporting;
		delay_reporting = avdtp_service_cap_new(AVDTP_DELAY_REPORTING,
								NULL, 0);
		*caps = g_slist_append(*caps, delay_reporting);
	}

	return TRUE;
}

static void endpoint_open_cb(struct a2dp_setup *setup, gboolean ret)
{
	int err = error_to_errno(setup->err);

	if (ret == FALSE) {
		setup->stream = NULL;
		finalize_setup_errno(setup, -EPERM, finalize_config, NULL);
		goto done;
	}

	if (err == 0)
		err = avdtp_open(setup->session, setup->stream);

	if (err == 0)
		goto done;

	error("avdtp_open %s (%d)", strerror(-err), -err);
	setup->stream = NULL;
	finalize_setup_errno(setup, err, finalize_config, NULL);
done:
	setup_unref(setup);
}

static void setconf_cfm(struct avdtp *session, struct avdtp_local_sep *sep,
				struct avdtp_stream *stream,
				struct avdtp_error *err, void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct a2dp_setup *setup;
	struct btd_device *dev;
	struct btd_service *service;
	int ret;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Set_Configuration_Cfm", sep);
	else
		DBG("Source %p: Set_Configuration_Cfm", sep);

	setup = find_setup_by_session(session);

	if (err) {
		if (setup) {
			setup_ref(setup);
			setup->err = err;
			finalize_config(setup);
			setup->err = NULL;
			setup_unref(setup);
		}
		return;
	}

	avdtp_stream_add_cb(session, stream, stream_state_changed, a2dp_sep);
	a2dp_sep->stream = stream;

	if (!setup)
		return;

	dev = avdtp_get_device(session);

	/* Notify D-Bus interface of the new stream */
	if (a2dp_sep->type == AVDTP_SEP_TYPE_SOURCE) {
		service = btd_device_get_service(dev, A2DP_SINK_UUID);
		sink_new_stream(service, session, setup->stream);
	} else {
		service = btd_device_get_service(dev, A2DP_SOURCE_UUID);
		source_new_stream(service, session, setup->stream);
	}

	/* Notify Endpoint */
	if (a2dp_sep->endpoint) {
		struct avdtp_service_capability *service;
		struct avdtp_media_codec_capability *codec;
		int err;

		service = avdtp_stream_get_codec(stream);
		codec = (struct avdtp_media_codec_capability *) service->data;

		err = a2dp_sep->endpoint->set_configuration(a2dp_sep,
						codec->data, service->length -
						sizeof(*codec),
						setup_ref(setup),
						endpoint_open_cb,
						a2dp_sep->user_data);
		if (err == 0)
			return;

		setup->stream = NULL;
		finalize_setup_errno(setup, -EPERM, finalize_config, NULL);
		setup_unref(setup);
		return;
	}

	ret = avdtp_open(session, stream);
	if (ret < 0) {
		error("avdtp_open %s (%d)", strerror(-ret), -ret);
		setup->stream = NULL;
		finalize_setup_errno(setup, ret, finalize_config, NULL);
	}
}

static gboolean getconf_ind(struct avdtp *session, struct avdtp_local_sep *sep,
				uint8_t *err, void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Get_Configuration_Ind", sep);
	else
		DBG("Source %p: Get_Configuration_Ind", sep);
	return TRUE;
}

static void getconf_cfm(struct avdtp *session, struct avdtp_local_sep *sep,
			struct avdtp_stream *stream, struct avdtp_error *err,
			void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Set_Configuration_Cfm", sep);
	else
		DBG("Source %p: Set_Configuration_Cfm", sep);
}

static bool match_remote_sep(const void *data, const void *user_data)
{
	const struct a2dp_remote_sep *sep = data;
	const struct avdtp_remote_sep *rsep = user_data;

	return sep->sep == rsep;
}

static void store_last_used(struct a2dp_channel *chan, uint8_t lseid,
							uint8_t rseid)
{
	GKeyFile *key_file;
	char filename[PATH_MAX];
	char dst_addr[18];
	char value[6];
	char *data;
	size_t len = 0;

	ba2str(device_get_address(chan->device), dst_addr);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/cache/%s",
		btd_adapter_get_storage_dir(device_get_adapter(chan->device)),
		dst_addr);
	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	sprintf(value, "%02hhx:%02hhx", lseid, rseid);

	g_key_file_set_string(key_file, "Endpoints", "LastUsed", value);

	data = g_key_file_to_data(key_file, &len, NULL);
	g_file_set_contents(filename, data, len, NULL);

	g_free(data);
	g_key_file_free(key_file);
}

static void add_last_used(struct a2dp_channel *chan, struct a2dp_sep *lsep,
				struct a2dp_remote_sep *rsep)
{
	if (!chan->last_used)
		chan->last_used = new0(struct a2dp_last_used, 1);

	chan->last_used->lsep = lsep;
	chan->last_used->rsep = rsep;
}

static void update_last_used(struct a2dp_channel *chan, struct a2dp_sep *lsep,
					struct avdtp_stream *stream)
{
	struct avdtp_remote_sep *rsep;
	struct a2dp_remote_sep *sep;

	rsep = avdtp_stream_get_remote_sep(stream);
	sep = queue_find(chan->seps, match_remote_sep, rsep);
	if (!sep) {
		error("Unable to find remote SEP");
		return;
	}

	/* Check if already stored then skip */
	if (chan->last_used && (chan->last_used->lsep == lsep &&
				chan->last_used->rsep == sep))
		return;

	add_last_used(chan, lsep, sep);

	store_last_used(chan, avdtp_sep_get_seid(lsep->lsep),
					avdtp_get_seid(rsep));
}

static gboolean open_ind(struct avdtp *session, struct avdtp_local_sep *sep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct a2dp_setup *setup;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Open_Ind", sep);
	else
		DBG("Source %p: Open_Ind", sep);

	setup = a2dp_setup_get(session);
	if (!setup)
		return FALSE;

	setup->stream = stream;

	if (!err && setup->chan)
		update_last_used(setup->chan, a2dp_sep, stream);

	if (setup->reconfigure)
		setup->reconfigure = FALSE;

	finalize_config(setup);

	return TRUE;
}

static void open_cfm(struct avdtp *session, struct avdtp_local_sep *sep,
			struct avdtp_stream *stream, struct avdtp_error *err,
			void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct a2dp_setup *setup;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Open_Cfm", sep);
	else
		DBG("Source %p: Open_Cfm", sep);

	setup = find_setup_by_session(session);
	if (!setup)
		return;

	if (setup->reconfigure)
		setup->reconfigure = FALSE;

	if (err) {
		setup->stream = NULL;
		setup->err = err;
		if (setup->start)
			finalize_resume(setup);
	} else if (setup->chan)
		update_last_used(setup->chan, a2dp_sep, stream);

	finalize_config(setup);

	return;
}

static gboolean suspend_timeout(struct a2dp_sep *sep)
{
	if (avdtp_suspend(sep->session, sep->stream) == 0)
		sep->suspending = TRUE;

	sep->suspend_timer = 0;

	avdtp_unref(sep->session);
	sep->session = NULL;

	return FALSE;
}

static gboolean start_ind(struct avdtp *session, struct avdtp_local_sep *sep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct a2dp_setup *setup;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Start_Ind", sep);
	else
		DBG("Source %p: Start_Ind", sep);

	if (!a2dp_sep->locked) {
		a2dp_sep->session = avdtp_ref(session);
		a2dp_sep->suspend_timer = g_timeout_add_seconds(SUSPEND_TIMEOUT,
						(GSourceFunc) suspend_timeout,
						a2dp_sep);
	}

	if (!a2dp_sep->starting)
		return TRUE;

	a2dp_sep->starting = FALSE;

	setup = find_setup_by_session(session);
	if (setup)
		finalize_resume(setup);

	return TRUE;
}

static void start_cfm(struct avdtp *session, struct avdtp_local_sep *sep,
			struct avdtp_stream *stream, struct avdtp_error *err,
			void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct a2dp_setup *setup;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Start_Cfm", sep);
	else
		DBG("Source %p: Start_Cfm", sep);

	a2dp_sep->starting = FALSE;

	setup = find_setup_by_session(session);
	if (!setup)
		return;

	if (err) {
		setup->stream = NULL;
		setup->err = err;
	}

	finalize_resume(setup);
}

static gboolean suspend_ind(struct avdtp *session, struct avdtp_local_sep *sep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct a2dp_setup *setup;
	gboolean start;
	int start_err;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Suspend_Ind", sep);
	else
		DBG("Source %p: Suspend_Ind", sep);

	if (a2dp_sep->suspend_timer) {
		g_source_remove(a2dp_sep->suspend_timer);
		a2dp_sep->suspend_timer = 0;
		avdtp_unref(a2dp_sep->session);
		a2dp_sep->session = NULL;
	}

	if (!a2dp_sep->suspending)
		return TRUE;

	a2dp_sep->suspending = FALSE;

	setup = find_setup_by_session(session);
	if (!setup)
		return TRUE;

	start = setup->start;
	setup->start = FALSE;

	finalize_suspend(setup);

	if (!start)
		return TRUE;

	start_err = avdtp_start(session, a2dp_sep->stream);
	if (start_err < 0 && start_err != -EINPROGRESS) {
		error("avdtp_start: %s (%d)", strerror(-start_err),
								-start_err);
		finalize_setup_errno(setup, start_err, finalize_resume, NULL);
	}

	return TRUE;
}

static void suspend_cfm(struct avdtp *session, struct avdtp_local_sep *sep,
			struct avdtp_stream *stream, struct avdtp_error *err,
			void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct a2dp_setup *setup;
	gboolean start;
	int start_err;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Suspend_Cfm", sep);
	else
		DBG("Source %p: Suspend_Cfm", sep);

	a2dp_sep->suspending = FALSE;

	setup = find_setup_by_session(session);
	if (!setup)
		return;

	start = setup->start;
	setup->start = FALSE;

	if (err) {
		setup->stream = NULL;
		setup->err = err;
	}

	finalize_suspend(setup);

	if (!start)
		return;

	if (err) {
		finalize_resume(setup);
		return;
	}

	start_err = avdtp_start(session, a2dp_sep->stream);
	if (start_err < 0 && start_err != -EINPROGRESS) {
		error("avdtp_start: %s (%d)", strerror(-start_err), -start_err);
		finalize_setup_errno(setup, start_err, finalize_suspend, NULL);
	}
}

static gboolean close_ind(struct avdtp *session, struct avdtp_local_sep *sep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct a2dp_setup *setup;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Close_Ind", sep);
	else
		DBG("Source %p: Close_Ind", sep);

	setup = find_setup_by_session(session);
	if (!setup)
		return TRUE;

	finalize_setup_errno(setup, -ECONNRESET, finalize_suspend,
							finalize_resume, NULL);

	return TRUE;
}

static struct a2dp_remote_sep *find_remote_sep(struct a2dp_channel *chan,
						struct a2dp_sep *sep)
{
	struct avdtp_remote_sep *rsep;

	rsep = avdtp_find_remote_sep(chan->session, sep->lsep);

	return queue_find(chan->seps, match_remote_sep, rsep);
}

static gboolean a2dp_reconfigure(gpointer data)
{
	struct a2dp_setup *setup = data;
	struct a2dp_sep *sep = setup->sep;
	int posix_err;
	struct avdtp_media_codec_capability *rsep_codec;
	struct avdtp_service_capability *cap;

	setup->id = 0;

	if (setup->err) {
		posix_err = error_to_errno(setup->err);
		goto failed;
	}

	if (!sep->lsep) {
		error("no valid local SEP");
		posix_err = -EINVAL;
		goto failed;
	}

	if (setup->rsep) {
		cap = avdtp_get_codec(setup->rsep->sep);
		rsep_codec = (struct avdtp_media_codec_capability *) cap->data;
	}

	if (!setup->rsep || sep->codec != rsep_codec->media_codec_type)
		setup->rsep = find_remote_sep(setup->chan, sep);

	if (!setup->rsep) {
		error("unable to find remote SEP");
		posix_err = -EINVAL;
		goto failed;
	}

	posix_err = avdtp_set_configuration(setup->session, setup->rsep->sep,
						sep->lsep,
						setup->caps,
						&setup->stream);
	if (posix_err < 0) {
		error("avdtp_set_configuration: %s", strerror(-posix_err));
		goto failed;
	}

	return FALSE;

failed:
	finalize_setup_errno(setup, posix_err, finalize_config, NULL);
	return FALSE;
}

static bool setup_reconfigure(struct a2dp_setup *setup)
{
	if (!setup->reconfigure || setup->id)
		return false;

	DBG("%p", setup);

	setup->id = g_timeout_add(RECONFIGURE_TIMEOUT, a2dp_reconfigure, setup);

	setup->reconfigure = FALSE;

	return true;
}

static struct a2dp_remote_sep *get_remote_sep(struct a2dp_channel *chan,
						struct avdtp_stream *stream)
{
	struct avdtp_remote_sep *rsep;

	rsep = avdtp_stream_get_remote_sep(stream);

	return queue_find(chan->seps, match_remote_sep, rsep);
}

static void close_cfm(struct avdtp *session, struct avdtp_local_sep *sep,
			struct avdtp_stream *stream, struct avdtp_error *err,
			void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct a2dp_setup *setup;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Close_Cfm", sep);
	else
		DBG("Source %p: Close_Cfm", sep);

	setup = find_setup_by_session(session);
	if (!setup)
		return;

	if (err) {
		setup->stream = NULL;
		setup->err = err;
		finalize_config(setup);
		return;
	}

	if (!setup->rsep)
		setup->rsep = get_remote_sep(setup->chan, stream);

	setup_reconfigure(setup);
}

static void abort_ind(struct avdtp *session, struct avdtp_local_sep *sep,
				struct avdtp_stream *stream, uint8_t *err,
				void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct a2dp_setup *setup;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Abort_Ind", sep);
	else
		DBG("Source %p: Abort_Ind", sep);

	a2dp_sep->stream = NULL;

	setup = find_setup_by_session(session);
	if (!setup)
		return;

	finalize_setup_errno(setup, -ECONNRESET, finalize_suspend,
							finalize_resume,
							finalize_config, NULL);

	return;
}

static void abort_cfm(struct avdtp *session, struct avdtp_local_sep *sep,
			struct avdtp_stream *stream, struct avdtp_error *err,
			void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct a2dp_setup *setup;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: Abort_Cfm", sep);
	else
		DBG("Source %p: Abort_Cfm", sep);

	setup = find_setup_by_session(session);
	if (!setup)
		return;

	if (setup_reconfigure(setup))
		return;

	setup_unref(setup);
}

static gboolean reconf_ind(struct avdtp *session, struct avdtp_local_sep *sep,
				uint8_t *err, void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: ReConfigure_Ind", sep);
	else
		DBG("Source %p: ReConfigure_Ind", sep);

	return TRUE;
}

static gboolean endpoint_delayreport_ind(struct avdtp *session,
						struct avdtp_local_sep *sep,
						uint8_t rseid, uint16_t delay,
						uint8_t *err, void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: DelayReport_Ind", sep);
	else
		DBG("Source %p: DelayReport_Ind", sep);

	if (a2dp_sep->endpoint == NULL ||
				a2dp_sep->endpoint->set_delay == NULL)
		return FALSE;

	a2dp_sep->endpoint->set_delay(a2dp_sep, delay, a2dp_sep->user_data);

	return TRUE;
}

static void reconf_cfm(struct avdtp *session, struct avdtp_local_sep *sep,
			struct avdtp_stream *stream, struct avdtp_error *err,
			void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;
	struct a2dp_setup *setup;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: ReConfigure_Cfm", sep);
	else
		DBG("Source %p: ReConfigure_Cfm", sep);

	setup = find_setup_by_session(session);
	if (!setup)
		return;

	if (err) {
		setup->stream = NULL;
		setup->err = err;
	}

	finalize_config(setup);
}

static void delay_report_cfm(struct avdtp *session, struct avdtp_local_sep *sep,
				struct avdtp_stream *stream,
				struct avdtp_error *err, void *user_data)
{
	struct a2dp_sep *a2dp_sep = user_data;

	if (a2dp_sep->type == AVDTP_SEP_TYPE_SINK)
		DBG("Sink %p: DelayReport_Cfm", sep);
	else
		DBG("Source %p: DelayReport_Cfm", sep);
}

static struct avdtp_sep_cfm cfm = {
	.set_configuration	= setconf_cfm,
	.get_configuration	= getconf_cfm,
	.open			= open_cfm,
	.start			= start_cfm,
	.suspend		= suspend_cfm,
	.close			= close_cfm,
	.abort			= abort_cfm,
	.reconfigure		= reconf_cfm,
	.delay_report		= delay_report_cfm,
};

static struct avdtp_sep_ind endpoint_ind = {
	.match_codec		= endpoint_match_codec_ind,
	.get_capability		= endpoint_getcap_ind,
	.set_configuration	= endpoint_setconf_ind,
	.get_configuration	= getconf_ind,
	.open			= open_ind,
	.start			= start_ind,
	.suspend		= suspend_ind,
	.close			= close_ind,
	.abort			= abort_ind,
	.reconfigure		= reconf_ind,
	.delayreport		= endpoint_delayreport_ind,
};

static sdp_record_t *a2dp_record(uint8_t type)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, l2cap_uuid, avdtp_uuid, a2dp_uuid;
	sdp_profile_desc_t profile[1];
	sdp_list_t *aproto, *proto[2];
	sdp_record_t *record;
	sdp_data_t *psm, *version, *features;
	uint16_t lp = AVDTP_UUID;
	uint16_t a2dp_ver = 0x0103, avdtp_ver = 0x0103, feat = 0x000f;

	record = sdp_record_alloc();
	if (!record)
		return NULL;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(0, &root_uuid);
	sdp_set_browse_groups(record, root);

	if (type == AVDTP_SEP_TYPE_SOURCE)
		sdp_uuid16_create(&a2dp_uuid, AUDIO_SOURCE_SVCLASS_ID);
	else
		sdp_uuid16_create(&a2dp_uuid, AUDIO_SINK_SVCLASS_ID);
	svclass_id = sdp_list_append(0, &a2dp_uuid);
	sdp_set_service_classes(record, svclass_id);

	sdp_uuid16_create(&profile[0].uuid, ADVANCED_AUDIO_PROFILE_ID);
	profile[0].version = a2dp_ver;
	pfseq = sdp_list_append(0, &profile[0]);
	sdp_set_profile_descs(record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(0, &l2cap_uuid);
	psm = sdp_data_alloc(SDP_UINT16, &lp);
	proto[0] = sdp_list_append(proto[0], psm);
	apseq = sdp_list_append(0, proto[0]);

	sdp_uuid16_create(&avdtp_uuid, AVDTP_UUID);
	proto[1] = sdp_list_append(0, &avdtp_uuid);
	version = sdp_data_alloc(SDP_UINT16, &avdtp_ver);
	proto[1] = sdp_list_append(proto[1], version);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(0, apseq);
	sdp_set_access_protos(record, aproto);

	features = sdp_data_alloc(SDP_UINT16, &feat);
	sdp_attr_add(record, SDP_ATTR_SUPPORTED_FEATURES, features);

	if (type == AVDTP_SEP_TYPE_SOURCE)
		sdp_set_info_attr(record, "Audio Source", 0, 0);
	else
		sdp_set_info_attr(record, "Audio Sink", 0, 0);

	free(psm);
	free(version);
	sdp_list_free(proto[0], 0);
	sdp_list_free(proto[1], 0);
	sdp_list_free(apseq, 0);
	sdp_list_free(pfseq, 0);
	sdp_list_free(aproto, 0);
	sdp_list_free(root, 0);
	sdp_list_free(svclass_id, 0);

	return record;
}

static struct a2dp_server *find_server(GSList *list, struct btd_adapter *a)
{

	for (; list; list = list->next) {
		struct a2dp_server *server = list->data;

		if (server->adapter == a)
			return server;
	}

	return NULL;
}

static void remote_sep_free(void *data)
{
	struct a2dp_remote_sep *sep = data;

	avdtp_remote_sep_set_destroy(sep->sep, NULL, NULL);

	free(sep->path);
	free(sep);
}

static void remove_remote_sep(void *data)
{
	struct a2dp_remote_sep *sep = data;

	if (!sep->path) {
		remote_sep_free(sep);
		return;
	}

	g_dbus_unregister_interface(btd_get_dbus_connection(), sep->path,
						MEDIA_ENDPOINT_INTERFACE);
}

static void channel_free(void *data)
{
	struct a2dp_channel *chan = data;
	struct a2dp_setup *setup;

	if (chan->auth_id > 0)
		btd_cancel_authorization(chan->auth_id);

	if (chan->io_id > 0)
		g_source_remove(chan->io_id);

	if (chan->io) {
		g_io_channel_shutdown(chan->io, TRUE, NULL);
		g_io_channel_unref(chan->io);
	}

	avdtp_remove_state_cb(chan->state_id);

	queue_destroy(chan->seps, remove_remote_sep);
	free(chan->last_used);

	setup = find_setup_by_session(chan->session);
	if (setup) {
		setup->chan = NULL;
		avdtp_unref(setup->session);
		setup->session = NULL;
		finalize_setup_errno(setup, -ENOTCONN, NULL);
	}

	g_free(chan);
}

static void channel_remove(struct a2dp_channel *chan)
{
	struct a2dp_server *server = chan->server;

	DBG("chan %p", chan);

	queue_remove(server->channels, chan);

	channel_free(chan);
}

static gboolean disconnect_cb(GIOChannel *io, GIOCondition cond, gpointer data)
{
	struct a2dp_channel *chan = data;

	DBG("chan %p", chan);

	chan->io_id = 0;

	channel_remove(chan);

	return FALSE;
}

static void caps_add_codec(GSList **l, uint8_t codec, uint8_t *caps,
							size_t size)
{
	struct avdtp_service_capability *media_transport, *media_codec;
	struct avdtp_media_codec_capability *cap;

	media_transport = avdtp_service_cap_new(AVDTP_MEDIA_TRANSPORT,
						NULL, 0);

	*l = g_slist_append(*l, media_transport);

	cap = g_malloc0(sizeof(*cap) + size);
	cap->media_type = AVDTP_MEDIA_TYPE_AUDIO;
	cap->media_codec_type = codec;
	memcpy(cap->data, caps, size);

	media_codec = avdtp_service_cap_new(AVDTP_MEDIA_CODEC, cap,
						sizeof(*cap) + size);

	*l = g_slist_append(*l, media_codec);
	g_free(cap);
}

struct client {
	const char *sender;
	const char *path;
};

static int match_client(const void *data, const void *user_data)
{
	struct a2dp_sep *sep = (void *) data;
	const struct a2dp_endpoint *endpoint = sep->endpoint;
	const struct client *client = user_data;

	if (strcmp(client->sender, endpoint->get_name(sep, sep->user_data)))
		return -1;

	return strcmp(client->path, endpoint->get_path(sep, sep->user_data));
}

static struct a2dp_sep *find_sep(struct a2dp_server *server, uint8_t type,
					const char *sender, const char *path)
{
	GSList *l;
	struct client client = { sender, path };

	l = type == AVDTP_SEP_TYPE_SINK ? server->sources : server->sinks;

	l = g_slist_find_custom(l, &client, match_client);
	if (l)
		return l->data;

	return NULL;
}

static int parse_properties(DBusMessageIter *props, uint8_t **caps, int *size)
{
	while (dbus_message_iter_get_arg_type(props) == DBUS_TYPE_DICT_ENTRY) {
		const char *key;
		DBusMessageIter value, entry;
		int var;

		dbus_message_iter_recurse(props, &entry);
		dbus_message_iter_get_basic(&entry, &key);

		dbus_message_iter_next(&entry);
		dbus_message_iter_recurse(&entry, &value);

		var = dbus_message_iter_get_arg_type(&value);
		if (strcasecmp(key, "Capabilities") == 0) {
			DBusMessageIter array;

			if (var != DBUS_TYPE_ARRAY)
				return -EINVAL;

			dbus_message_iter_recurse(&value, &array);
			dbus_message_iter_get_fixed_array(&array, caps, size);
			return 0;
		}

		dbus_message_iter_next(props);
	}

	return -EINVAL;
}

static void reconfig_cb(struct avdtp *session, struct a2dp_sep *sep,
			struct avdtp_stream *stream, int err, void *user_data)
{
	DBusMessage *msg = user_data;

	if (err)
		g_dbus_send_message(btd_get_dbus_connection(),
					btd_error_failed(msg, strerror(-err)));
	else
		g_dbus_send_reply(btd_get_dbus_connection(), msg,
					DBUS_TYPE_INVALID);

	dbus_message_unref(msg);
}

static int a2dp_reconfig(struct a2dp_channel *chan, const char *sender,
			struct a2dp_sep *lsep, struct a2dp_remote_sep *rsep,
			uint8_t *caps, int size, void *user_data)
{
	struct a2dp_setup *setup;
	struct a2dp_setup_cb *cb_data;
	GSList *l;
	int err;

	setup = a2dp_setup_get(chan->session);
	if (!setup)
		return -ENOMEM;

	cb_data = setup_cb_new(setup);
	cb_data->config_cb = reconfig_cb;
	cb_data->user_data = user_data;

	setup->sep = lsep;
	setup->rsep = rsep;

	g_slist_free_full(setup->caps, g_free);
	setup->caps = NULL;

	caps_add_codec(&setup->caps, setup->sep->codec, caps, size);

	l = avdtp_get_type(rsep->sep) == AVDTP_SEP_TYPE_SINK ?
					chan->server->sources :
					chan->server->sinks;

	/* Check for existing stream and close it */
	for (; l; l = g_slist_next(l)) {
		struct a2dp_sep *tmp = l->data;

		/* Attempt to reconfigure if a stream already exists */
		if (tmp->stream) {
			/* Only allow switching sep from the same sender */
			if (strcmp(sender, tmp->endpoint->get_name(tmp,
							tmp->user_data)))
				return -EPERM;

			/* Check if stream is for the channel */
			if (!avdtp_has_stream(chan->session, tmp->stream))
				continue;

			err = avdtp_close(chan->session, tmp->stream, FALSE);
			if (err < 0) {
				err = avdtp_abort(chan->session, tmp->stream);
				if (err < 0) {
					error("avdtp_abort: %s",
							strerror(-err));
					goto fail;
				}
			}

			setup->reconfigure = TRUE;

			return 0;
		}
	}

	err = avdtp_set_configuration(setup->session, setup->rsep->sep,
						lsep->lsep,
						setup->caps,
						&setup->stream);
	if (err < 0) {
		error("avdtp_set_configuration: %s", strerror(-err));
		goto fail;
	}

	return 0;

fail:
	setup_cb_free(cb_data);
	return err;
}

static DBusMessage *set_configuration(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	struct a2dp_remote_sep *rsep = data;
	struct a2dp_channel *chan = rsep->chan;
	struct a2dp_sep *lsep = NULL;
	struct avdtp_service_capability *service;
	struct avdtp_media_codec_capability *codec;
	DBusMessageIter args, props;
	const char *sender, *path;
	uint8_t *caps;
	int err, size = 0;

	sender = dbus_message_get_sender(msg);

	dbus_message_iter_init(msg, &args);

	dbus_message_iter_get_basic(&args, &path);
	dbus_message_iter_next(&args);

	lsep = find_sep(chan->server, avdtp_get_type(rsep->sep), sender, path);
	if (!lsep)
		return btd_error_invalid_args(msg);

	service = avdtp_get_codec(rsep->sep);
	codec = (struct avdtp_media_codec_capability *) service->data;

	/* Check if codec really matches */
	if (!endpoint_match_codec_ind(chan->session, codec, lsep))
		return btd_error_invalid_args(msg);

	dbus_message_iter_recurse(&args, &props);
	if (dbus_message_iter_get_arg_type(&props) != DBUS_TYPE_DICT_ENTRY)
		return btd_error_invalid_args(msg);

	if (parse_properties(&props, &caps, &size) < 0)
		return btd_error_invalid_args(msg);

	err = a2dp_reconfig(chan, sender, lsep, rsep, caps, size,
					dbus_message_ref(msg));
	if (err < 0) {
		dbus_message_unref(msg);
		return btd_error_failed(msg, strerror(-err));
	}

	return NULL;
}

static const GDBusMethodTable sep_methods[] = {
	{ GDBUS_ASYNC_METHOD("SetConfiguration",
					GDBUS_ARGS({ "endpoint", "o" },
						{ "properties", "a{sv}" } ),
					NULL, set_configuration) },
	{ },
};

static gboolean get_uuid(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct a2dp_remote_sep *sep = data;
	const char *uuid;

	switch (avdtp_get_type(sep->sep)) {
	case AVDTP_SEP_TYPE_SOURCE:
		uuid = A2DP_SOURCE_UUID;
		break;
	case AVDTP_SEP_TYPE_SINK:
		uuid = A2DP_SINK_UUID;
		break;
	default:
		uuid = "";
	}

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &uuid);

	return TRUE;
}

static gboolean get_codec(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct a2dp_remote_sep *sep = data;
	struct avdtp_service_capability *cap = avdtp_get_codec(sep->sep);
	struct avdtp_media_codec_capability *codec = (void *) cap->data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BYTE,
						&codec->media_codec_type);

	return TRUE;
}

static gboolean get_capabilities(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct a2dp_remote_sep *sep = data;
	struct avdtp_service_capability *service = avdtp_get_codec(sep->sep);
	struct avdtp_media_codec_capability *codec = (void *) service->data;
	uint8_t *caps = codec->data;
	DBusMessageIter array;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_BYTE_AS_STRING, &array);

	dbus_message_iter_append_fixed_array(&array, DBUS_TYPE_BYTE, &caps,
					service->length - sizeof(*codec));

	dbus_message_iter_close_container(iter, &array);

	return TRUE;
}

static gboolean get_device(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct a2dp_remote_sep *sep = data;
	const char *path;

	path = device_get_path(sep->chan->device);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);

	return TRUE;
}

static gboolean get_delay_reporting(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct a2dp_remote_sep *sep = data;
	dbus_bool_t delay_report;

	delay_report = avdtp_get_delay_reporting(sep->sep);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &delay_report);

	return TRUE;
}

static const GDBusPropertyTable sep_properties[] = {
	{ "UUID", "s", get_uuid, NULL, NULL },
	{ "Codec", "y", get_codec, NULL, NULL },
	{ "Capabilities", "ay", get_capabilities, NULL, NULL },
	{ "Device", "o", get_device, NULL, NULL },
	{ "DelayReporting", "b", get_delay_reporting, NULL, NULL },
	{ }
};

static void remote_sep_destroy(void *user_data)
{
	struct a2dp_remote_sep *sep = user_data;

	if (queue_remove(sep->chan->seps, sep))
		remove_remote_sep(sep);
}

static void register_remote_sep(void *data, void *user_data)
{
	struct avdtp_remote_sep *rsep = data;
	struct a2dp_channel *chan = user_data;
	struct a2dp_remote_sep *sep;

	sep = queue_find(chan->seps, match_remote_sep, rsep);
	if (sep)
		return;

	sep = new0(struct a2dp_remote_sep, 1);
	sep->chan = chan;
	sep->sep = rsep;

	if (asprintf(&sep->path, "%s/sep%d",
				device_get_path(chan->device),
				avdtp_get_seid(rsep)) < 0) {
		error("Could not allocate path for remote sep %s/sep%d",
				device_get_path(chan->device),
				avdtp_get_seid(rsep));
		sep->path = NULL;
		goto done;
	}

	if (g_dbus_register_interface(btd_get_dbus_connection(),
				sep->path, MEDIA_ENDPOINT_INTERFACE,
				sep_methods, NULL, sep_properties,
				sep, remote_sep_free) == FALSE) {
		error("Could not register remote sep %s", sep->path);
		free(sep->path);
		free(sep);
		return;
	}

	DBG("Found remote SEP: %s", sep->path);

	avdtp_remote_sep_set_destroy(rsep, sep, remote_sep_destroy);

done:
	queue_push_tail(chan->seps, sep);
}

static bool match_seid(const void *data, const void *user_data)
{
	const struct a2dp_remote_sep *sep = data;
	const uint8_t *seid = user_data;

	return avdtp_get_seid(sep->sep) == *seid;
}

static int match_sep(const void *data, const void *user_data)
{
	struct a2dp_sep *sep = (void *) data;
	const uint8_t *seid = user_data;

	return *seid - avdtp_sep_get_seid(sep->lsep);
}

static struct a2dp_sep *find_sep_by_seid(struct a2dp_server *server,
							uint8_t seid)
{
	GSList *l;

	l = g_slist_find_custom(server->sources, &seid, match_sep);
	if (l)
		return l->data;

	l = g_slist_find_custom(server->sinks, &seid, match_sep);
	if (l)
		return l->data;

	return NULL;
}

static void load_remote_sep(struct a2dp_channel *chan, GKeyFile *key_file,
								char **seids)
{
	struct a2dp_sep *lsep;
	struct a2dp_remote_sep *sep;
	struct avdtp_remote_sep *rsep;
	uint8_t lseid, rseid;
	char *value;

	if (!seids)
		return;

	for (; *seids; seids++) {
		uint8_t type;
		uint8_t codec;
		uint8_t delay_reporting;
		GSList *l = NULL;
		char caps[256];
		uint8_t data[128];
		int i, size;

		if (sscanf(*seids, "%02hhx", &rseid) != 1)
			continue;

		value = g_key_file_get_string(key_file, "Endpoints", *seids,
								NULL);
		if (!value)
			continue;

		/* Try loading with delay_reporting first */
		if (sscanf(value, "%02hhx:%02hhx:%02hhx:%s", &type, &codec,
					&delay_reporting, caps) != 4) {
			/* Try old format */
			if (sscanf(value, "%02hhx:%02hhx:%s", &type, &codec,
								caps) != 3) {
				warn("Unable to load Endpoint: seid %u", rseid);
				g_free(value);
				continue;
			}
			delay_reporting = false;
		}

		for (i = 0, size = strlen(caps); i < size; i += 2) {
			uint8_t *tmp = data + i / 2;

			if (sscanf(caps + i, "%02hhx", tmp) != 1) {
				warn("Unable to load Endpoint: seid %u", rseid);
				break;
			}
		}

		g_free(value);

		if (i != size)
			continue;

		caps_add_codec(&l, codec, data, size / 2);

		rsep = avdtp_register_remote_sep(chan->session, rseid, type, l,
							delay_reporting);
		if (!rsep) {
			warn("Unable to register Endpoint: seid %u", rseid);
			continue;
		}

		register_remote_sep(rsep, chan);
	}

	value = g_key_file_get_string(key_file, "Endpoints", "LastUsed", NULL);
	if (!value)
		return;

	if (sscanf(value, "%02hhx:%02hhx", &lseid, &rseid) != 2) {
		warn("Unable to load LastUsed");
		g_free(value);
		return;
	}

	g_free(value);

	lsep = find_sep_by_seid(chan->server, lseid);
	if (!lsep) {
		warn("Unable to load LastUsed: lseid %u not found", lseid);
		return;
	}

	sep = queue_find(chan->seps, match_seid, &rseid);
	if (!sep) {
		warn("Unable to load LastUsed: rseid %u not found", rseid);
		return;
	}

	add_last_used(chan, lsep, sep);
}

static void load_remote_seps(struct a2dp_channel *chan)
{
	struct btd_device *device = chan->device;
	char filename[PATH_MAX];
	char dst_addr[18];
	char **keys;
	GKeyFile *key_file;

	ba2str(device_get_address(device), dst_addr);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/cache/%s",
			btd_adapter_get_storage_dir(device_get_adapter(device)),
			dst_addr);
	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);
	keys = g_key_file_get_keys(key_file, "Endpoints", NULL, NULL);

	load_remote_sep(chan, key_file, keys);

	g_strfreev(keys);
	g_key_file_free(key_file);
}

static void avdtp_state_cb(struct btd_device *dev, struct avdtp *session,
					avdtp_session_state_t old_state,
					avdtp_session_state_t new_state,
					void *user_data)
{
	struct a2dp_channel *chan = user_data;

	switch (new_state) {
	case AVDTP_SESSION_STATE_DISCONNECTED:
		if (chan->session == session)
			channel_remove(chan);
		break;
	case AVDTP_SESSION_STATE_CONNECTING:
		break;
	case AVDTP_SESSION_STATE_CONNECTED:
		if (!chan->session)
			chan->session = session;
		load_remote_seps(chan);
		break;
	}
}

static struct a2dp_channel *channel_new(struct a2dp_server *server,
					struct btd_device *device,
					GIOChannel *io)
{
	struct a2dp_channel *chan;

	chan = g_new0(struct a2dp_channel, 1);
	chan->server = server;
	chan->device = device;
	chan->seps = queue_new();
	chan->state_id = avdtp_add_state_cb(device, avdtp_state_cb, chan);

	if (!queue_push_tail(server->channels, chan)) {
		g_free(chan);
		return NULL;
	}

	if (!io)
		return chan;

	chan->io = g_io_channel_ref(io);
	chan->io_id = g_io_add_watch(io, G_IO_ERR | G_IO_HUP | G_IO_NVAL,
					(GIOFunc) disconnect_cb, chan);

	return chan;
}

static bool match_by_device(const void *data, const void *user_data)
{
	const struct a2dp_channel *chan = data;
	const struct btd_device *device = user_data;

	return chan->device == device;
}

struct avdtp *a2dp_avdtp_get(struct btd_device *device)
{
	struct a2dp_server *server;
	struct a2dp_channel *chan;
	const struct queue_entry *entry;

	server = find_server(servers, device_get_adapter(device));
	if (server == NULL)
		return NULL;

	chan = queue_find(server->channels, match_by_device, device);
	if (!chan) {
		chan = channel_new(server, device, NULL);
		if (!chan)
			return NULL;
	}

	if (chan->session)
		return avdtp_ref(chan->session);

	/* Check if there is any SEP available */
	for (entry = queue_get_entries(server->seps); entry;
					entry = entry->next) {
		struct avdtp_local_sep *sep = entry->data;

		if (avdtp_sep_get_state(sep) == AVDTP_STATE_IDLE)
			goto found;
	}

	DBG("Unable to find any available SEP");

	return NULL;

found:
	chan->session = avdtp_new(chan->io, device, server->seps);
	if (!chan->session) {
		channel_remove(chan);
		return NULL;
	}

	return avdtp_ref(chan->session);
}

static void connect_cb(GIOChannel *io, GError *err, gpointer user_data)
{
	struct a2dp_channel *chan = user_data;

	if (err) {
		error("%s", err->message);
		goto fail;
	}

	if (!chan->session) {
		chan->session = avdtp_new(chan->io, chan->device,
							chan->server->seps);
		if (!chan->session) {
			error("Unable to create AVDTP session");
			goto fail;
		}
	}

	g_io_channel_unref(chan->io);
	chan->io = NULL;

	g_source_remove(chan->io_id);
	chan->io_id = 0;

	return;

fail:
	channel_remove(chan);
}

static void auth_cb(DBusError *derr, void *user_data)
{
	struct a2dp_channel *chan = user_data;
	GError *err = NULL;

	chan->auth_id = 0;

	if (derr && dbus_error_is_set(derr)) {
		error("Access denied: %s", derr->message);
		goto fail;
	}

	if (!bt_io_accept(chan->io, connect_cb, chan, NULL, &err)) {
		error("bt_io_accept: %s", err->message);
		g_error_free(err);
		goto fail;
	}

	return;

fail:
	channel_remove(chan);
}

static void transport_cb(GIOChannel *io, GError *err, gpointer user_data)
{
	struct a2dp_setup *setup = user_data;
	uint16_t omtu, imtu;

	if (!g_slist_find(setups, setup)) {
		warn("bt_io_accept: setup %p no longer valid", setup);
		g_io_channel_shutdown(io, TRUE, NULL);
		return;
	}

	if (err) {
		error("%s", err->message);
		goto drop;
	}

	bt_io_get(io, &err, BT_IO_OPT_OMTU, &omtu, BT_IO_OPT_IMTU, &imtu,
							BT_IO_OPT_INVALID);
	if (err) {
		error("%s", err->message);
		g_error_free(err);
		goto drop;
	}

	if (!avdtp_stream_set_transport(setup->stream,
					g_io_channel_unix_get_fd(io),
					imtu, omtu))
		goto drop;

	g_io_channel_set_close_on_unref(io, FALSE);

	g_io_channel_unref(setup->io);
	setup->io = NULL;

	setup_unref(setup);

	return;

drop:
	setup_unref(setup);
	g_io_channel_shutdown(io, TRUE, NULL);

}
static void confirm_cb(GIOChannel *io, gpointer data)
{
	struct a2dp_server *server = data;
	struct a2dp_channel *chan;
	char address[18];
	bdaddr_t src, dst;
	GError *err = NULL;
	struct btd_device *device;

	bt_io_get(io, &err,
			BT_IO_OPT_SOURCE_BDADDR, &src,
			BT_IO_OPT_DEST_BDADDR, &dst,
			BT_IO_OPT_DEST, address,
			BT_IO_OPT_INVALID);
	if (err) {
		error("%s", err->message);
		g_error_free(err);
		goto drop;
	}

	DBG("AVDTP: incoming connect from %s", address);

	device = btd_adapter_find_device(adapter_find(&src), &dst,
								BDADDR_BREDR);
	if (!device)
		goto drop;

	chan = queue_find(server->channels, match_by_device, device);
	if (chan) {
		struct a2dp_setup *setup;

		setup = find_setup_by_session(chan->session);
		if (!setup || !setup->stream)
			goto drop;

		if (setup->io) {
			error("transport channel already exists");
			goto drop;
		}

		if (!bt_io_accept(io, transport_cb, setup, NULL, &err)) {
			error("bt_io_accept: %s", err->message);
			g_error_free(err);
			goto drop;
		}

		/*
		 * Reference the channel so it can be shutdown properly
		 * stopping bt_io_accept from calling the callback with invalid
		 * setup pointer.
		 */
		setup->io = g_io_channel_ref(io);

		return;
	}

	chan = channel_new(server, device, io);
	if (!chan)
		goto drop;

	chan->auth_id = btd_request_authorization(&src, &dst,
							ADVANCED_AUDIO_UUID,
							auth_cb, chan);
	if (chan->auth_id == 0 && !chan->session)
		channel_remove(chan);

	return;

drop:
	g_io_channel_shutdown(io, TRUE, NULL);
}

static bool a2dp_server_listen(struct a2dp_server *server)
{
	GError *err = NULL;
	BtIOMode mode;

	if (server->io)
		return true;

	mode = btd_opts.avdtp.session_mode;

	server->io = bt_io_listen(NULL, confirm_cb, server, NULL, &err,
				BT_IO_OPT_SOURCE_BDADDR,
				btd_adapter_get_address(server->adapter),
				BT_IO_OPT_PSM, AVDTP_PSM,
				BT_IO_OPT_MODE, mode,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
				BT_IO_OPT_MASTER, true,
				BT_IO_OPT_INVALID);
	if (server->io)
		return true;

	error("%s", err->message);
	g_error_free(err);

	return false;
}

static struct a2dp_server *a2dp_server_register(struct btd_adapter *adapter)
{
	struct a2dp_server *server;

	server = g_new0(struct a2dp_server, 1);

	server->adapter = btd_adapter_ref(adapter);
	server->seps = queue_new();
	server->channels = queue_new();

	servers = g_slist_append(servers, server);

	return server;
}

static void a2dp_unregister_sep(struct a2dp_sep *sep)
{
	struct a2dp_server *server = sep->server;

	if (sep->destroy) {
		sep->destroy(sep->user_data);
		sep->endpoint = NULL;
	}

	avdtp_unregister_sep(server->seps, sep->lsep);

	g_free(sep);

	if (!queue_isempty(server->seps))
		return;

	if (server->io) {
		g_io_channel_shutdown(server->io, TRUE, NULL);
		g_io_channel_unref(server->io);
		server->io = NULL;
	}
}

static void a2dp_server_unregister(struct a2dp_server *server)
{
	servers = g_slist_remove(servers, server);
	queue_destroy(server->channels, channel_free);
	queue_destroy(server->seps, NULL);

	if (server->io) {
		g_io_channel_shutdown(server->io, TRUE, NULL);
		g_io_channel_unref(server->io);
	}

	btd_adapter_unref(server->adapter);
	g_free(server);
}

struct a2dp_sep *a2dp_add_sep(struct btd_adapter *adapter, uint8_t type,
				uint8_t codec, gboolean delay_reporting,
				struct a2dp_endpoint *endpoint,
				void *user_data, GDestroyNotify destroy,
				int *err)
{
	struct a2dp_server *server;
	struct a2dp_sep *sep;
	GSList **l;
	uint32_t *record_id;
	sdp_record_t *record;

	server = find_server(servers, adapter);
	if (server == NULL) {
		if (err)
			*err = -EPROTONOSUPPORT;
		return NULL;
	}

	if (type == AVDTP_SEP_TYPE_SINK && !server->sink_enabled) {
		if (err)
			*err = -EPROTONOSUPPORT;
		return NULL;
	}

	if (type == AVDTP_SEP_TYPE_SOURCE && !server->source_enabled) {
		if (err)
			*err = -EPROTONOSUPPORT;
		return NULL;
	}

	sep = g_new0(struct a2dp_sep, 1);

	sep->lsep = avdtp_register_sep(server->seps, type,
					AVDTP_MEDIA_TYPE_AUDIO, codec,
					delay_reporting, &endpoint_ind,
					&cfm, sep);

	if (sep->lsep == NULL) {
		g_free(sep);
		if (err)
			*err = -EINVAL;
		return NULL;
	}

	sep->server = server;
	sep->endpoint = endpoint;
	sep->codec = codec;
	sep->type = type;
	sep->delay_reporting = delay_reporting;
	sep->user_data = user_data;
	sep->destroy = destroy;

	if (type == AVDTP_SEP_TYPE_SOURCE) {
		l = &server->sources;
		record_id = &server->source_record_id;
	} else {
		l = &server->sinks;
		record_id = &server->sink_record_id;
	}

	if (*record_id != 0)
		goto add;

	record = a2dp_record(type);
	if (!record) {
		error("Unable to allocate new service record");
		a2dp_unregister_sep(sep);
		if (err)
			*err = -EINVAL;
		return NULL;
	}

	if (adapter_service_add(server->adapter, record) < 0) {
		error("Unable to register A2DP service record");
		sdp_record_free(record);
		a2dp_unregister_sep(sep);
		if (err)
			*err = -EINVAL;
		return NULL;
	}

	if (!a2dp_server_listen(server)) {
		sdp_record_free(record);
		a2dp_unregister_sep(sep);
		if (err)
			*err = -EINVAL;
		return NULL;
	}

	*record_id = record->handle;

add:
	*l = g_slist_append(*l, sep);

	if (err)
		*err = 0;
	return sep;
}

void a2dp_remove_sep(struct a2dp_sep *sep)
{
	struct a2dp_server *server = sep->server;

	if (sep->type == AVDTP_SEP_TYPE_SOURCE) {
		if (g_slist_find(server->sources, sep) == NULL)
			return;
		server->sources = g_slist_remove(server->sources, sep);
		if (server->sources == NULL && server->source_record_id) {
			adapter_service_remove(server->adapter,
						server->source_record_id);
			server->source_record_id = 0;
		}
	} else {
		if (g_slist_find(server->sinks, sep) == NULL)
			return;
		server->sinks = g_slist_remove(server->sinks, sep);
		if (server->sinks == NULL && server->sink_record_id) {
			adapter_service_remove(server->adapter,
						server->sink_record_id);
			server->sink_record_id = 0;
		}
	}

	if (sep->locked)
		return;

	a2dp_unregister_sep(sep);
}

static void select_cb(struct a2dp_setup *setup, void *ret, int size)
{
	struct avdtp_service_capability *service;
	struct avdtp_media_codec_capability *codec;
	int err;

	if (setup->err)
		goto done;

	if (size >= 0) {
		caps_add_codec(&setup->caps, setup->sep->codec, ret, size);
		goto done;
	}

	setup->sep = queue_pop_head(setup->eps);
	if (!setup->sep) {
		error("Unable to select a valid configuration");
		goto done;
	}

	setup->rsep = find_remote_sep(setup->chan, setup->sep);
	service = avdtp_get_codec(setup->rsep->sep);
	codec = (struct avdtp_media_codec_capability *) service->data;

	err = setup->sep->endpoint->select_configuration(setup->sep,
					codec->data,
					service->length - sizeof(*codec),
					setup,
					select_cb, setup->sep->user_data);
	if (err == 0)
		return;

done:
	finalize_select(setup);
	setup_unref(setup);
}

static struct queue *a2dp_find_eps(struct avdtp *session, GSList *list,
					const char *sender)
{
	struct a2dp_channel *chan = find_channel(session);
	struct queue *seps = NULL;

	for (; list; list = list->next) {
		struct a2dp_sep *sep = list->data;
		struct avdtp_remote_sep *rsep;

		/* Use sender's endpoint if available */
		if (sender) {
			const char *name;

			if (sep->endpoint == NULL)
				continue;

			name = sep->endpoint->get_name(sep, sep->user_data);
			if (g_strcmp0(sender, name) != 0)
				continue;
		}

		rsep = avdtp_find_remote_sep(session, sep->lsep);
		if (!rsep)
			continue;

		if (!seps)
			seps = queue_new();

		/* Prepend last used so it is preferred over others */
		if (chan->last_used && (chan->last_used->lsep == sep &&
					chan->last_used->rsep->sep == rsep))
			queue_push_head(seps, sep);
		else
			queue_push_tail(seps, sep);
	}

	return seps;
}

static struct queue *a2dp_select_eps(struct avdtp *session, uint8_t type,
					const char *sender)
{
	struct a2dp_server *server;
	struct queue *seps;
	GSList *l;

	server = find_server(servers, avdtp_get_adapter(session));
	if (!server)
		return NULL;

	l = type == AVDTP_SEP_TYPE_SINK ? server->sources : server->sinks;

	/* Check sender's seps first */
	seps = a2dp_find_eps(session, l, sender);
	if (seps != NULL)
		return seps;

	return a2dp_find_eps(session, l, NULL);
}

static void store_remote_sep(void *data, void *user_data)
{
	struct a2dp_remote_sep *sep = data;
	GKeyFile *key_file = user_data;
	char seid[4], value[256];
	struct avdtp_service_capability *service = avdtp_get_codec(sep->sep);
	struct avdtp_media_codec_capability *codec;
	unsigned int i;
	ssize_t offset;

	if (!service)
		return;

	codec = (void *) service->data;

	sprintf(seid, "%02hhx", avdtp_get_seid(sep->sep));

	offset = sprintf(value, "%02hhx:%02hhx:%02hhx:",
			avdtp_get_type(sep->sep), codec->media_codec_type,
			avdtp_get_delay_reporting(sep->sep));

	for (i = 0; i < service->length - sizeof(*codec); i++)
		offset += sprintf(value + offset, "%02hhx", codec->data[i]);

	g_key_file_set_string(key_file, "Endpoints", seid, value);
}

static void store_remote_seps(struct a2dp_channel *chan)
{
	struct btd_device *device = chan->device;
	char filename[PATH_MAX];
	char dst_addr[18];
	GKeyFile *key_file;
	char *data;
	gsize length = 0;

	if (queue_isempty(chan->seps))
		return;

	ba2str(device_get_address(device), dst_addr);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/cache/%s",
			btd_adapter_get_storage_dir(device_get_adapter(device)),
			dst_addr);
	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	data = g_key_file_get_string(key_file, "Endpoints", "LastUsed",
								NULL);

	/* Remove current endpoints since it might have changed */
	g_key_file_remove_group(key_file, "Endpoints", NULL);

	queue_foreach(chan->seps, store_remote_sep, key_file);

	if (data) {
		g_key_file_set_string(key_file, "Endpoints", "LastUsed",
						data);
		g_free(data);
	}

	data = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, data, length, NULL);

	g_free(data);
	g_key_file_free(key_file);
}

static void discover_cb(struct avdtp *session, GSList *seps,
				struct avdtp_error *err, void *user_data)
{
	struct a2dp_setup *setup = user_data;
	uint16_t version = avdtp_get_version(session);

	DBG("version 0x%04x err %p", version, err);

	setup->seps = seps;
	if (err)
		setup->err = err;

	if (!err) {
		g_slist_foreach(seps, register_remote_sep, setup->chan);

		/* Only store version has been initialized as features like
		 * Delay Reporting may not be queried if the version in
		 * unknown.
		 */
		if (version)
			store_remote_seps(setup->chan);
	}

	finalize_discover(setup);
}

unsigned int a2dp_discover(struct avdtp *session, a2dp_discover_cb_t cb,
							void *user_data)
{
	struct a2dp_setup *setup;
	struct a2dp_setup_cb *cb_data;

	setup = a2dp_setup_get(session);
	if (!setup)
		return 0;

	cb_data = setup_cb_new(setup);
	cb_data->discover_cb = cb;
	cb_data->user_data = user_data;

	if (avdtp_discover(session, discover_cb, setup) == 0)
		return cb_data->id;

	setup_cb_free(cb_data);
	return 0;
}

unsigned int a2dp_select_capabilities(struct avdtp *session,
					uint8_t type, const char *sender,
					a2dp_select_cb_t cb,
					void *user_data)
{
	struct a2dp_setup *setup;
	struct a2dp_setup_cb *cb_data;
	struct queue *eps;
	struct avdtp_service_capability *service;
	struct avdtp_media_codec_capability *codec;
	int err;

	eps = a2dp_select_eps(session, type, sender);
	if (!eps) {
		error("Unable to select SEP");
		return 0;
	}

	setup = a2dp_setup_get(session);
	if (!setup)
		return 0;

	cb_data = setup_cb_new(setup);
	cb_data->select_cb = cb;
	cb_data->user_data = user_data;

	setup->eps = eps;
	setup->sep = queue_pop_head(eps);
	setup->rsep = find_remote_sep(setup->chan, setup->sep);

	if (setup->rsep == NULL) {
		error("Could not find remote sep");
		goto fail;
	}

	service = avdtp_get_codec(setup->rsep->sep);
	codec = (struct avdtp_media_codec_capability *) service->data;

	err = setup->sep->endpoint->select_configuration(setup->sep,
							codec->data,
							service->length -
							sizeof(*codec),
							setup_ref(setup),
							select_cb,
							setup->sep->user_data);
	if (err == 0)
		return cb_data->id;

	setup_unref(setup);

fail:
	setup_cb_free(cb_data);
	return 0;

}

unsigned int a2dp_config(struct avdtp *session, struct a2dp_sep *sep,
				a2dp_config_cb_t cb, GSList *caps,
				void *user_data)
{
	struct a2dp_setup_cb *cb_data;
	GSList *l;
	struct a2dp_server *server;
	struct a2dp_setup *setup;
	struct a2dp_sep *tmp;
	struct avdtp_service_capability *cap;
	struct avdtp_media_codec_capability *codec_cap = NULL;
	int posix_err;

	server = find_server(servers, avdtp_get_adapter(session));
	if (!server)
		return 0;

	for (l = caps; l != NULL; l = l->next) {
		cap = l->data;

		if (cap->category != AVDTP_MEDIA_CODEC)
			continue;

		codec_cap = (void *) cap->data;
		break;
	}

	if (!codec_cap)
		return 0;

	if (sep->codec != codec_cap->media_codec_type)
		return 0;

	DBG("a2dp_config: selected SEP %p", sep->lsep);

	setup = a2dp_setup_get(session);
	if (!setup)
		return 0;

	cb_data = setup_cb_new(setup);
	cb_data->config_cb = cb;
	cb_data->user_data = user_data;

	setup->sep = sep;
	setup->stream = sep->stream;

	/* Copy given caps if they are different than current caps */
	if (setup->caps != caps) {
		g_slist_free_full(setup->caps, g_free);
		setup->caps = g_slist_copy(caps);
	}

	switch (avdtp_sep_get_state(sep->lsep)) {
	case AVDTP_STATE_IDLE:
		if (sep->type == AVDTP_SEP_TYPE_SOURCE)
			l = server->sources;
		else
			l = server->sinks;

		for (; l != NULL; l = l->next) {
			tmp = l->data;
			if (avdtp_has_stream(session, tmp->stream))
				break;
		}

		if (l != NULL) {
			if (tmp->locked)
				goto failed;
			setup->reconfigure = TRUE;
			if (avdtp_close(session, tmp->stream, FALSE) < 0) {
				error("avdtp_close failed");
				goto failed;
			}
			break;
		}

		setup->rsep = find_remote_sep(setup->chan, sep);
		if (setup->rsep == NULL) {
			error("No matching ACP and INT SEPs found");
			goto failed;
		}

		posix_err = avdtp_set_configuration(session, setup->rsep->sep,
							sep->lsep, caps,
							&setup->stream);
		if (posix_err < 0) {
			error("avdtp_set_configuration: %s",
				strerror(-posix_err));
			goto failed;
		}
		break;
	case AVDTP_STATE_OPEN:
	case AVDTP_STATE_STREAMING:
		if (avdtp_stream_has_capabilities(setup->stream, caps)) {
			DBG("Configuration match: resuming");
			cb_data->source_id = g_idle_add(finalize_config,
								setup);
		} else if (!setup->reconfigure) {
			setup->reconfigure = TRUE;
			if (avdtp_close(session, sep->stream, FALSE) < 0) {
				error("avdtp_close failed");
				goto failed;
			}
		}
		break;
	case AVDTP_STATE_CONFIGURED:
	case AVDTP_STATE_CLOSING:
	case AVDTP_STATE_ABORTING:
	default:
		error("SEP in bad state for requesting a new stream");
		goto failed;
	}

	return cb_data->id;

failed:
	setup_cb_free(cb_data);
	return 0;
}

unsigned int a2dp_resume(struct avdtp *session, struct a2dp_sep *sep,
				a2dp_stream_cb_t cb, void *user_data)
{
	struct a2dp_setup_cb *cb_data;
	struct a2dp_setup *setup;

	setup = a2dp_setup_get(session);
	if (!setup)
		return 0;

	cb_data = setup_cb_new(setup);
	cb_data->resume_cb = cb;
	cb_data->user_data = user_data;

	setup->sep = sep;
	setup->stream = sep->stream;

	switch (avdtp_sep_get_state(sep->lsep)) {
	case AVDTP_STATE_IDLE:
		goto failed;
		break;
	case AVDTP_STATE_CONFIGURED:
		setup->start = TRUE;
		break;
	case AVDTP_STATE_OPEN:
		if (avdtp_start(session, sep->stream) < 0) {
			error("avdtp_start failed");
			goto failed;
		}
		sep->starting = TRUE;
		break;
	case AVDTP_STATE_STREAMING:
		if (!sep->suspending && sep->suspend_timer) {
			g_source_remove(sep->suspend_timer);
			sep->suspend_timer = 0;
			avdtp_unref(sep->session);
			sep->session = NULL;
		}
		if (sep->suspending)
			setup->start = TRUE;
		else
			cb_data->source_id = g_idle_add(finalize_resume,
								setup);
		break;
	case AVDTP_STATE_CLOSING:
	case AVDTP_STATE_ABORTING:
	default:
		error("SEP in bad state for resume");
		goto failed;
	}

	return cb_data->id;

failed:
	setup_cb_free(cb_data);
	return 0;
}

unsigned int a2dp_suspend(struct avdtp *session, struct a2dp_sep *sep,
				a2dp_stream_cb_t cb, void *user_data)
{
	struct a2dp_setup_cb *cb_data;
	struct a2dp_setup *setup;

	setup = a2dp_setup_get(session);
	if (!setup)
		return 0;

	cb_data = setup_cb_new(setup);
	cb_data->suspend_cb = cb;
	cb_data->user_data = user_data;

	setup->sep = sep;
	setup->stream = sep->stream;

	switch (avdtp_sep_get_state(sep->lsep)) {
	case AVDTP_STATE_IDLE:
		error("a2dp_suspend: no stream to suspend");
		goto failed;
		break;
	case AVDTP_STATE_OPEN:
		cb_data->source_id = g_idle_add(finalize_suspend, setup);
		break;
	case AVDTP_STATE_STREAMING:
		if (avdtp_suspend(session, sep->stream) < 0) {
			error("avdtp_suspend failed");
			goto failed;
		}
		sep->suspending = TRUE;
		break;
	case AVDTP_STATE_CONFIGURED:
	case AVDTP_STATE_CLOSING:
	case AVDTP_STATE_ABORTING:
	default:
		error("SEP in bad state for suspend");
		goto failed;
	}

	return cb_data->id;

failed:
	setup_cb_free(cb_data);
	return 0;
}

gboolean a2dp_cancel(unsigned int id)
{
	GSList *ls;

	for (ls = setups; ls != NULL; ls = g_slist_next(ls)) {
		struct a2dp_setup *setup = ls->data;
		GSList *l;
		for (l = setup->cb; l != NULL; l = g_slist_next(l)) {
			struct a2dp_setup_cb *cb = l->data;

			if (cb->id != id)
				continue;

			setup_ref(setup);
			setup_cb_free(cb);

			if (!setup->cb) {
				DBG("aborting setup %p", setup);
				if (!avdtp_abort(setup->session, setup->stream))
					return TRUE;
			}

			setup_unref(setup);
			return TRUE;
		}
	}

	return FALSE;
}

gboolean a2dp_sep_lock(struct a2dp_sep *sep, struct avdtp *session)
{
	if (sep->locked)
		return FALSE;

	DBG("SEP %p locked", sep->lsep);
	sep->locked = TRUE;

	return TRUE;
}

gboolean a2dp_sep_unlock(struct a2dp_sep *sep, struct avdtp *session)
{
	struct a2dp_server *server = sep->server;
	avdtp_state_t state;
	GSList *l;

	state = avdtp_sep_get_state(sep->lsep);

	sep->locked = FALSE;

	DBG("SEP %p unlocked", sep->lsep);

	if (sep->type == AVDTP_SEP_TYPE_SOURCE)
		l = server->sources;
	else
		l = server->sinks;

	/* Unregister sep if it was removed */
	if (g_slist_find(l, sep) == NULL) {
		a2dp_unregister_sep(sep);
		return TRUE;
	}

	if (!sep->stream || state == AVDTP_STATE_IDLE)
		return TRUE;

	switch (state) {
	case AVDTP_STATE_OPEN:
		/* Set timer here */
		break;
	case AVDTP_STATE_STREAMING:
		if (avdtp_suspend(session, sep->stream) == 0)
			sep->suspending = TRUE;
		break;
	case AVDTP_STATE_IDLE:
	case AVDTP_STATE_CONFIGURED:
	case AVDTP_STATE_CLOSING:
	case AVDTP_STATE_ABORTING:
	default:
		break;
	}

	return TRUE;
}

struct avdtp_stream *a2dp_sep_get_stream(struct a2dp_sep *sep)
{
	return sep->stream;
}

struct btd_device *a2dp_setup_get_device(struct a2dp_setup *setup)
{
	if (setup->session == NULL)
		return NULL;

	return avdtp_get_device(setup->session);
}

const char *a2dp_setup_remote_path(struct a2dp_setup *setup)
{
	if (setup->rsep) {
		return setup->rsep->path;
	}

	return NULL;
}

static int a2dp_source_probe(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);

	DBG("path %s", device_get_path(dev));

	source_init(service);

	return 0;
}

static void a2dp_source_remove(struct btd_service *service)
{
	source_unregister(service);
}

static int a2dp_sink_probe(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);

	DBG("path %s", device_get_path(dev));

	return sink_init(service);
}

static void a2dp_sink_remove(struct btd_service *service)
{
	sink_unregister(service);
}

static int a2dp_source_connect(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);
	struct btd_adapter *adapter = device_get_adapter(dev);
	struct a2dp_server *server;
	const char *path = device_get_path(dev);

	DBG("path %s", path);

	server = find_server(servers, adapter);
	if (!server || !server->sink_enabled) {
		DBG("Unexpected error: cannot find server");
		return -EPROTONOSUPPORT;
	}

	/* Return protocol not available if no record/endpoint exists */
	if (server->sink_record_id == 0)
		return -ENOPROTOOPT;

	return source_connect(service);
}

static int a2dp_source_disconnect(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);
	const char *path = device_get_path(dev);

	DBG("path %s", path);

	return source_disconnect(service);
}

static int a2dp_sink_connect(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);
	struct btd_adapter *adapter = device_get_adapter(dev);
	struct a2dp_server *server;
	const char *path = device_get_path(dev);

	DBG("path %s", path);

	server = find_server(servers, adapter);
	if (!server || !server->source_enabled) {
		DBG("Unexpected error: cannot find server");
		return -EPROTONOSUPPORT;
	}

	/* Return protocol not available if no record/endpoint exists */
	if (server->source_record_id == 0)
		return -ENOPROTOOPT;

	return sink_connect(service);
}

static int a2dp_sink_disconnect(struct btd_service *service)
{
	struct btd_device *dev = btd_service_get_device(service);
	const char *path = device_get_path(dev);

	DBG("path %s", path);

	return sink_disconnect(service);
}

static int a2dp_source_server_probe(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	struct a2dp_server *server;

	DBG("path %s", adapter_get_path(adapter));

	server = find_server(servers, adapter);
	if (server != NULL)
		goto done;

	server = a2dp_server_register(adapter);
	if (server == NULL)
		return -EPROTONOSUPPORT;

done:
	server->source_enabled = TRUE;

	return 0;
}

static void a2dp_source_server_remove(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	struct a2dp_server *server;

	DBG("path %s", adapter_get_path(adapter));

	server = find_server(servers, adapter);
	if (!server)
		return;

	g_slist_free_full(server->sources,
					(GDestroyNotify) a2dp_unregister_sep);

	if (server->source_record_id) {
		adapter_service_remove(server->adapter,
					server->source_record_id);
		server->source_record_id = 0;
	}

	if (server->sink_record_id)
		return;

	a2dp_server_unregister(server);
}

static int a2dp_sink_server_probe(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	struct a2dp_server *server;

	DBG("path %s", adapter_get_path(adapter));

	server = find_server(servers, adapter);
	if (server != NULL)
		goto done;

	server = a2dp_server_register(adapter);
	if (server == NULL)
		return -EPROTONOSUPPORT;

done:
	server->sink_enabled = TRUE;

	return 0;
}

static void a2dp_sink_server_remove(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	struct a2dp_server *server;

	DBG("path %s", adapter_get_path(adapter));

	server = find_server(servers, adapter);
	if (!server)
		return;

	g_slist_free_full(server->sinks, (GDestroyNotify) a2dp_unregister_sep);

	if (server->sink_record_id) {
		adapter_service_remove(server->adapter, server->sink_record_id);
		server->sink_record_id = 0;
	}

	if (server->source_record_id)
		return;

	a2dp_server_unregister(server);
}

static int media_server_probe(struct btd_adapter *adapter)
{
	DBG("path %s", adapter_get_path(adapter));

	return media_register(adapter);
}

static void media_server_remove(struct btd_adapter *adapter)
{
	DBG("path %s", adapter_get_path(adapter));

	media_unregister(adapter);
}

static struct btd_profile a2dp_source_profile = {
	.name		= "a2dp-source",
	.priority	= BTD_PROFILE_PRIORITY_MEDIUM,

	.remote_uuid	= A2DP_SOURCE_UUID,
	.device_probe	= a2dp_source_probe,
	.device_remove	= a2dp_source_remove,

	.auto_connect	= true,
	.connect	= a2dp_source_connect,
	.disconnect	= a2dp_source_disconnect,

	.adapter_probe	= a2dp_sink_server_probe,
	.adapter_remove	= a2dp_sink_server_remove,
};

static struct btd_profile a2dp_sink_profile = {
	.name		= "a2dp-sink",
	.priority	= BTD_PROFILE_PRIORITY_MEDIUM,

	.remote_uuid	= A2DP_SINK_UUID,
	.device_probe	= a2dp_sink_probe,
	.device_remove	= a2dp_sink_remove,

	.auto_connect	= true,
	.connect	= a2dp_sink_connect,
	.disconnect	= a2dp_sink_disconnect,

	.adapter_probe	= a2dp_source_server_probe,
	.adapter_remove	= a2dp_source_server_remove,
};

static struct btd_adapter_driver media_driver = {
	.name		= "media",
	.probe		= media_server_probe,
	.remove		= media_server_remove,
};

static int a2dp_init(void)
{
	btd_register_adapter_driver(&media_driver);
	btd_profile_register(&a2dp_source_profile);
	btd_profile_register(&a2dp_sink_profile);

	return 0;
}

static void a2dp_exit(void)
{
	btd_unregister_adapter_driver(&media_driver);
	btd_profile_unregister(&a2dp_source_profile);
	btd_profile_unregister(&a2dp_sink_profile);
}

BLUETOOTH_PLUGIN_DEFINE(a2dp, VERSION, BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
		a2dp_init, a2dp_exit)
