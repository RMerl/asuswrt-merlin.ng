/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2012 Intel Corporation
 *
 *
 */

typedef void (*obc_transport_func)(GIOChannel *io, GError *err,
							gpointer user_data);

struct obc_transport {
	const char *name;
	guint (*connect) (const char *source, const char *destination,
				const char *service, uint16_t port,
				obc_transport_func func, void *user_data);
	int (*getpacketopt) (GIOChannel *io, int *tx_mtu, int *rx_mtu);
	void (*disconnect) (guint id);
	const void *(*getattribute) (guint id, int attribute_id);
};

int obc_transport_register(struct obc_transport *transport);
void obc_transport_unregister(struct obc_transport *transport);
struct obc_transport *obc_transport_find(const char *name);
