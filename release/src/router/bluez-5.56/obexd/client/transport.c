// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2012 Intel Corporation
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <inttypes.h>

#include "obexd/src/log.h"
#include "transport.h"

static GSList *transports = NULL;

struct obc_transport *obc_transport_find(const char *name)
{
	GSList *l;

	for (l = transports; l; l = l->next) {
		struct obc_transport *transport = l->data;

		if (strcasecmp(name, transport->name) == 0)
			return transport;
	}

	return NULL;
}

int obc_transport_register(struct obc_transport *transport)
{
	if (!transport) {
		error("Invalid transport");
		return -EINVAL;
	}

	if (obc_transport_find(transport->name)) {
		error("Permission denied: transport %s already registered",
							transport->name);
		return -EPERM;
	}

	DBG("transport %p name %s registered", transport, transport->name);

	transports = g_slist_append(transports, transport);

	return 0;
}

void obc_transport_unregister(struct obc_transport *transport)
{
	if (!g_slist_find(transports, transport)) {
		error("Unable to unregister: No such transport %p", transport);
		return;
	}

	DBG("transport %p name %s unregistered", transport, transport->name);

	transports = g_slist_remove(transports, transport);
}
