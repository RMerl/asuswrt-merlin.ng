/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Nokia Corporation
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

struct obex_server {
	struct obex_transport_driver *transport;
	void *transport_data;
	GSList *drivers;
};

int obex_server_init(void);

void obex_server_exit(void);

int obex_server_new_connection(struct obex_server *server, GIOChannel *io,
					uint16_t tx_mtu, uint16_t rx_mtu,
					gboolean stream);
