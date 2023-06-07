// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Nokia Corporation
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <inttypes.h>

#include <glib.h>

#include "gobex/gobex.h"

#include "log.h"
#include "obex.h"
#include "obex-priv.h"
#include "server.h"
#include "service.h"
#include "transport.h"

static GSList *servers = NULL;

static void init_server(uint16_t service, GSList *transports)
{
	GSList *l;

	for (l = transports; l; l = l->next) {
		struct obex_transport_driver *transport = l->data;
		struct obex_server *server;
		int err;

		if (transport->service != 0 &&
				(transport->service & service) == FALSE)
			continue;

		server = g_new0(struct obex_server, 1);
		server->transport = transport;
		server->drivers = obex_service_driver_list(service);

		server->transport_data = transport->start(server, &err);
		if (server->transport_data == NULL) {
			DBG("Unable to start %s transport: %s (%d)",
					transport->name, strerror(err), err);
			g_free(server);
			continue;
		}

		servers = g_slist_prepend(servers, server);
	}
}

int obex_server_init(void)
{
	GSList *drivers;
	GSList *transports;
	GSList *l;

	drivers = obex_service_driver_list(0);
	if (drivers == NULL) {
		DBG("No service driver registered");
		return -EINVAL;
	}

	transports = obex_transport_driver_list();
	if (transports == NULL) {
		DBG("No transport driver registered");
		return -EINVAL;
	}

	for (l = drivers; l; l = l->next) {
		struct obex_service_driver *driver = l->data;

		init_server(driver->service, transports);
	}

	return 0;
}

void obex_server_exit(void)
{
	GSList *l;

	for (l = servers; l; l = l->next) {
		struct obex_server *server = l->data;

		server->transport->stop(server->transport_data);
		g_slist_free(server->drivers);
		g_free(server);
	}

	g_slist_free(servers);

	return;
}

int obex_server_new_connection(struct obex_server *server, GIOChannel *io,
					uint16_t tx_mtu, uint16_t rx_mtu,
					gboolean stream)
{
	return obex_session_start(io, tx_mtu, rx_mtu, stream, server);
}
