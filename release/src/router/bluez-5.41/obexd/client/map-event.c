/*
 *
 *  OBEX
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

#include <glib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#include "gdbus/gdbus.h"

#include "obexd/src/log.h"
#include "map-event.h"

#include "transfer.h"
#include "session.h"

static GSList *handlers;

struct mns_handler {
	int mas_id;
	struct obc_session *session;
	map_event_cb cb;
	void *user_data;
};

static struct mns_handler *find_handler(int mas_id, const char *device)
{
	GSList *list;

	for (list = handlers; list; list = list->next) {
		struct mns_handler *handler = list->data;

		if (mas_id != handler->mas_id)
			continue;

		if (g_str_equal(device,
				obc_session_get_destination(handler->session)))
			return handler;
	}

	return NULL;
}

bool map_register_event_handler(struct obc_session *session,
					int mas_id, map_event_cb cb,
					void *user_data)
{
	struct mns_handler *handler;

	handler = find_handler(mas_id, obc_session_get_destination(session));
	if (handler != NULL)
		return FALSE;

	handler = g_new0(struct mns_handler, 1);
	handler->mas_id = mas_id;
	handler->session = session;
	handler->cb = cb;
	handler->user_data = user_data;

	handlers = g_slist_prepend(handlers, handler);
	DBG("Handler %p for %s:%d registered", handler,
			obc_session_get_destination(session), mas_id);

	return TRUE;
}

void map_unregister_event_handler(struct obc_session *session, int mas_id)
{
	struct mns_handler *handler;

	handler = find_handler(mas_id, obc_session_get_destination(session));
	if (handler == NULL)
		return;

	handlers = g_slist_remove(handlers, handler);
	DBG("Handler %p for %s:%d unregistered", handler,
			obc_session_get_destination(session), mas_id);
	g_free(handler);
}

void map_dispatch_event(int mas_id, const char *device,
						struct map_event *event)
{
	struct mns_handler *handler;

	handler = find_handler(mas_id, device);
	if (handler)
		handler->cb(event, handler->user_data);
}
