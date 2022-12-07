/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#include <stdint.h>

enum server_type {
	SERVER_TYPE_BREDRLE,
	SERVER_TYPE_BREDR,
	SERVER_TYPE_LE,
	SERVER_TYPE_AMP,
	SERVER_TYPE_MONITOR,
};

struct server;

struct server *server_open_unix(enum server_type type, const char *path);
struct server *server_open_tcp(enum server_type type);
void server_close(struct server *server);
