/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2007  Nokia Corporation
 *  Copyright (C) 2004-2009  Marcel Holtmann <marcel@holtmann.org>
 *
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

int8_t media_player_get_device_volume(struct btd_device *device);
