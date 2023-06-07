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

struct media_transport;

struct media_transport *media_transport_create(struct btd_device *device,
						const char *remote_endpoint,
						uint8_t *configuration,
						size_t size, void *data);

void media_transport_destroy(struct media_transport *transport);
const char *media_transport_get_path(struct media_transport *transport);
struct btd_device *media_transport_get_dev(struct media_transport *transport);
int8_t media_transport_get_volume(struct media_transport *transport);
void media_transport_update_delay(struct media_transport *transport,
							uint16_t delay);
void media_transport_update_volume(struct media_transport *transport,
								int8_t volume);
void transport_get_properties(struct media_transport *transport,
							DBusMessageIter *iter);

int8_t media_transport_get_device_volume(struct btd_device *dev);
void media_transport_update_device_volume(struct btd_device *dev,
								int8_t volume);
