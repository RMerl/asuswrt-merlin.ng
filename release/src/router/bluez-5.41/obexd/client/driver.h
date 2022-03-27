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

struct obc_driver {
	const char *service;
	const char *uuid;
	void *target;
	gsize target_len;
	void *(*supported_features) (struct obc_session *session);
	int (*probe) (struct obc_session *session);
	void (*remove) (struct obc_session *session);
};

int obc_driver_register(struct obc_driver *driver);
void obc_driver_unregister(struct obc_driver *driver);
struct obc_driver *obc_driver_find(const char *pattern);
