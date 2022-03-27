/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2013  BMW Car IT GmbH. All rights reserved.
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

#include <string.h>
#include <errno.h>
#include <glib.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#include "gobex/gobex.h"
#include "gobex/gobex-apparam.h"

#include "obexd/src/obexd.h"
#include "obexd/src/plugin.h"
#include "obexd/src/log.h"
#include "obexd/src/obex.h"
#include "obexd/src/service.h"
#include "obexd/src/mimetype.h"
#include "obexd/src/map_ap.h"
#include "map-event.h"

#include "obexd/src/manager.h"

struct mns_session {
	GString *buffer;
	GObexApparam *inparams;
	char *remote_address;
	uint8_t mas_instance_id;
};

static const uint8_t MNS_TARGET[TARGET_SIZE] = {
			0xbb, 0x58, 0x2b, 0x41, 0x42, 0x0c, 0x11, 0xdb,
			0xb0, 0xde, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66  };

static int get_params(struct obex_session *os, struct mns_session *mns)
{
	const uint8_t *buffer;
	ssize_t size;

	size = obex_get_apparam(os, &buffer);
	if (size < 0)
		size = 0;

	mns->inparams = g_obex_apparam_decode(buffer, size);
	if (mns->inparams == NULL) {
		DBG("Error when parsing parameters!");
		return -EBADR;
	}

	return 0;
}

static void reset_request(struct mns_session *mns)
{
	if (mns->buffer) {
		g_string_free(mns->buffer, TRUE);
		mns->buffer = NULL;
	}

	if (mns->inparams) {
		g_obex_apparam_free(mns->inparams);
		mns->inparams = NULL;
	}
}

static void mns_session_free(struct mns_session *mns)
{
	reset_request(mns);

	if (mns->remote_address)
		g_free(mns->remote_address);

	g_free(mns);
}

static void *mns_connect(struct obex_session *os, int *err)
{
	struct mns_session *mns;
	char *address;

	manager_register_session(os);

	mns = g_new0(struct mns_session, 1);

	if (obex_getpeername(os, &address) == 0) {
		mns->remote_address = g_strdup(address);
		g_free(address);
	}

	DBG("MNS connected to %s", mns->remote_address);

	if (err)
		*err = 0;

	return mns;
}

static void mns_disconnect(struct obex_session *os, void *user_data)
{
	struct mns_session *mns = user_data;

	DBG("MNS disconnected from %s", mns->remote_address);

	manager_unregister_session(os);

	mns_session_free(mns);
}

static int mns_put(struct obex_session *os, void *user_data)
{
	struct mns_session *mns = user_data;
	const char *type = obex_get_type(os);
	const char *name = obex_get_name(os);
	int ret;

	DBG("PUT: name %s type %s mns %p", name, type, mns);

	if (type == NULL)
		return -EBADR;

	ret = get_params(os, mns);
	if (ret < 0)
		goto failed;

	ret = obex_put_stream_start(os, name);
	if (ret < 0)
		goto failed;

	return 0;

failed:
	reset_request(mns);

	return ret;
}

static void parse_event_report_type(struct map_event *event, const char *value)
{
	if (!g_ascii_strcasecmp(value, "NewMessage"))
		event->type = MAP_ET_NEW_MESSAGE;
	else if (!g_ascii_strcasecmp(value, "DeliverySuccess"))
		event->type = MAP_ET_DELIVERY_SUCCESS;
	else if (!g_ascii_strcasecmp(value, "SendingSuccess"))
		event->type = MAP_ET_SENDING_SUCCESS;
	else if (!g_ascii_strcasecmp(value, "DeliveryFailure"))
		event->type = MAP_ET_DELIVERY_FAILURE;
	else if (!g_ascii_strcasecmp(value, "SendingFailure"))
		event->type = MAP_ET_SENDING_FAILURE;
	else if (!g_ascii_strcasecmp(value, "MemoryFull"))
		event->type = MAP_ET_MEMORY_FULL;
	else if (!g_ascii_strcasecmp(value, "MemoryAvailable"))
		event->type = MAP_ET_MEMORY_AVAILABLE;
	else if (!g_ascii_strcasecmp(value, "MessageDeleted"))
		event->type = MAP_ET_MESSAGE_DELETED;
	else if (!g_ascii_strcasecmp(value, "MessageShift"))
		event->type = MAP_ET_MESSAGE_SHIFT;
}

static void parse_event_report_handle(struct map_event *event,
							const char *value)
{
	event->handle = strtoull(value, NULL, 16);
}

static void parse_event_report_folder(struct map_event *event,
							const char *value)
{
	g_free(event->folder);

	if (g_str_has_prefix(value, "/"))
		event->folder = g_strdup(value);
	else
		event->folder = g_strconcat("/", value, NULL);
}

static void parse_event_report_old_folder(struct map_event *event,
							const char *value)
{
	g_free(event->old_folder);

	if (g_str_has_prefix(value, "/"))
		event->old_folder = g_strdup(value);
	else
		event->old_folder = g_strconcat("/", value, NULL);
}

static void parse_event_report_msg_type(struct map_event *event,
							const char *value)
{
	g_free(event->msg_type);
	event->msg_type = g_strdup(value);
}

static void parse_event_report_date_time(struct map_event *event,
							const char *value)
{
	g_free(event->datetime);
	event->datetime = g_strdup(value);
}

static void parse_event_report_subject(struct map_event *event,
							const char *value)
{
	g_free(event->subject);
	event->subject = g_strdup(value);
}

static void parse_event_report_sender_name(struct map_event *event,
							const char *value)
{
	g_free(event->sender_name);
	event->sender_name = g_strdup(value);
}

static void parse_event_report_priority(struct map_event *event,
							const char *value)
{
	g_free(event->priority);
	event->priority = g_strdup(value);
}

static struct map_event_report_parser {
	const char *name;
	void (*func) (struct map_event *event, const char *value);
} event_report_parsers[] = {
		{ "type", parse_event_report_type },
		{ "handle", parse_event_report_handle },
		{ "folder", parse_event_report_folder },
		{ "old_folder", parse_event_report_old_folder },
		{ "msg_type", parse_event_report_msg_type },
		{ "datetime", parse_event_report_date_time },
		{ "subject", parse_event_report_subject },
		{ "sender_name", parse_event_report_sender_name },
		{ "priority", parse_event_report_priority },
		{ }
};

static void event_report_element(GMarkupParseContext *ctxt,
				const char *element, const char **names,
				const char **values, gpointer user_data,
								GError **gerr)
{
	struct map_event *event = user_data;
	const char *key;
	int i;

	if (strcasecmp("event", element) != 0)
		return;

	for (i = 0, key = names[i]; key; key = names[++i]) {
		struct map_event_report_parser *parser;

		for (parser = event_report_parsers; parser && parser->name;
								parser++) {
			if (strcasecmp(key, parser->name) == 0) {
				if (values[i])
					parser->func(event, values[i]);
				break;
			}
		}
	}
}

static const GMarkupParser event_report_parser = {
	event_report_element,
	NULL,
	NULL,
	NULL,
	NULL
};

static void map_event_free(struct map_event *event)
{
	g_free(event->folder);
	g_free(event->old_folder);
	g_free(event->msg_type);
	g_free(event->datetime);
	g_free(event->subject);
	g_free(event->sender_name);
	g_free(event->priority);
	g_free(event);
}

static void *event_report_open(const char *name, int oflag, mode_t mode,
				void *driver_data, size_t *size, int *err)
{
	struct mns_session *mns = driver_data;

	DBG("");

	g_obex_apparam_get_uint8(mns->inparams, MAP_AP_MASINSTANCEID,
							&mns->mas_instance_id);

	mns->buffer = g_string_new("");

	if (err != NULL)
		*err = 0;

	return mns;
}

static int event_report_close(void *obj)
{
	struct mns_session *mns = obj;
	GMarkupParseContext *ctxt;
	struct map_event *event;

	DBG("");

	event = g_new0(struct map_event, 1);
	ctxt = g_markup_parse_context_new(&event_report_parser, 0, event,
									NULL);
	g_markup_parse_context_parse(ctxt, mns->buffer->str, mns->buffer->len,
									NULL);
	g_markup_parse_context_free(ctxt);

	map_dispatch_event(mns->mas_instance_id, mns->remote_address, event);
	map_event_free(event);

	reset_request(mns);

	return 0;
}

static ssize_t event_report_write(void *obj, const void *buf, size_t count)
{
	struct mns_session *mns = obj;

	DBG("");

	g_string_append_len(mns->buffer, buf, count);
	return count;
}

static struct obex_service_driver mns = {
	.name = "Message Notification server",
	.service = OBEX_MNS,
	.target = MNS_TARGET,
	.target_size = TARGET_SIZE,
	.connect = mns_connect,
	.put = mns_put,
	.disconnect = mns_disconnect,
};

static struct obex_mime_type_driver mime_event_report = {
	.target = MNS_TARGET,
	.target_size = TARGET_SIZE,
	.mimetype = "x-bt/MAP-event-report",
	.open = event_report_open,
	.close = event_report_close,
	.write = event_report_write,
};

static int mns_init(void)
{
	int err;

	err = obex_mime_type_driver_register(&mime_event_report);
	if (err < 0)
		goto fail_mime_event;

	err = obex_service_driver_register(&mns);
	if (err < 0)
		goto fail_mns_reg;

	return 0;

fail_mns_reg:
	obex_mime_type_driver_unregister(&mime_event_report);
fail_mime_event:
	return err;
}

static void mns_exit(void)
{
	obex_service_driver_unregister(&mns);
	obex_mime_type_driver_unregister(&mime_event_report);
}

OBEX_PLUGIN_DEFINE(mns, mns_init, mns_exit)
