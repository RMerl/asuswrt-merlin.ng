/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
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

#define OBEX_PORT_RANDOM UINT16_MAX

struct obex_service_driver {
	const char *name;
	uint16_t service;
	uint8_t channel;
	uint16_t port;
	gboolean secure;
	const uint8_t *target;
	unsigned int target_size;
	const uint8_t *who;
	unsigned int who_size;
	const char *record;
	void *(*connect) (struct obex_session *os, int *err);
	void (*progress) (struct obex_session *os, void *user_data);
	int (*get) (struct obex_session *os, void *user_data);
	int (*put) (struct obex_session *os, void *user_data);
	int (*chkput) (struct obex_session *os, void *user_data);
	int (*setpath) (struct obex_session *os, void *user_data);
	int (*action) (struct obex_session *os, void *user_data);
	void (*disconnect) (struct obex_session *os, void *user_data);
	void (*reset) (struct obex_session *os, void *user_data);
};

int obex_service_driver_register(struct obex_service_driver *driver);
void obex_service_driver_unregister(struct obex_service_driver *driver);
GSList *obex_service_driver_list(uint16_t services);
struct obex_service_driver *obex_service_driver_find(GSList *drivers,
			const uint8_t *target, unsigned int target_size,
			const uint8_t *who, unsigned int who_size);
