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

struct obex_session {
	GIOChannel *io;
	uint32_t id;
	uint8_t cmd;
	uint8_t action_id;
	char *src;
	char *dst;
	char *name;
	char *destname;
	char *type;
	char *path;
	time_t time;
	uint8_t *apparam;
	size_t apparam_len;
	const void *nonhdr;
	size_t nonhdr_len;
	guint get_rsp;
	uint8_t *buf;
	int64_t pending;
	int64_t offset;
	int64_t size;
	void *object;
	gboolean aborted;
	int err;
	struct obex_service_driver *service;
	void *service_data;
	struct obex_server *server;
	gboolean checked;
	GObex *obex;
	struct obex_mime_type_driver *driver;
	gboolean headers_sent;
};

int obex_session_start(GIOChannel *io, uint16_t tx_mtu, uint16_t rx_mtu,
				gboolean stream, struct obex_server *server);
