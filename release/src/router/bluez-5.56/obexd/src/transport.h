/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

struct obex_transport_driver {
	const char *name;
	uint16_t service;
	void *(*start) (struct obex_server *server, int *err);
	int (*getpeername) (GIOChannel *io, char **name);
	int (*getsockname) (GIOChannel *io, char **name);
	void (*stop) (void *data);
};

int obex_transport_driver_register(struct obex_transport_driver *driver);
void obex_transport_driver_unregister(struct obex_transport_driver *driver);
GSList *obex_transport_driver_list(void);
