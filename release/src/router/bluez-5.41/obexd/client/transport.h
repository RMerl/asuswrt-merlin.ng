/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2012 Intel Corporation
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
