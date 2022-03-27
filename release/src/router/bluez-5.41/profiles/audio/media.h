/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2007  Nokia Corporation
 *  Copyright (C) 2004-2009  Marcel Holtmann <marcel@holtmann.org>
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

struct media_endpoint;

typedef void (*media_endpoint_cb_t) (struct media_endpoint *endpoint,
					void *ret, int size, void *user_data);

int media_register(struct btd_adapter *btd_adapter);
void media_unregister(struct btd_adapter *btd_adapter);

struct a2dp_sep *media_endpoint_get_sep(struct media_endpoint *endpoint);
const char *media_endpoint_get_uuid(struct media_endpoint *endpoint);
uint8_t media_endpoint_get_codec(struct media_endpoint *endpoint);
