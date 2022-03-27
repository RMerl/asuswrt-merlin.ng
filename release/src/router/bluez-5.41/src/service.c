/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2013  BMW Car IT GmbH. All rights reserved.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"

#include "log.h"
#include "backtrace.h"

#include "adapter.h"
#include "device.h"
#include "profile.h"
#include "service.h"

struct btd_service {
	int			ref;
	struct btd_device	*device;
	struct btd_profile	*profile;
	void			*user_data;
	btd_service_state_t	state;
	int			err;
};

struct service_state_callback {
	btd_service_state_cb	cb;
	void			*user_data;
	unsigned int		id;
};

static GSList *state_callbacks = NULL;

static const char *state2str(btd_service_state_t state)
{
	switch (state) {
	case BTD_SERVICE_STATE_UNAVAILABLE:
		return "unavailable";
	case BTD_SERVICE_STATE_DISCONNECTED:
		return "disconnected";
	case BTD_SERVICE_STATE_CONNECTING:
		return "connecting";
	case BTD_SERVICE_STATE_CONNECTED:
		return "connected";
	case BTD_SERVICE_STATE_DISCONNECTING:
		return "disconnecting";
	}

	return NULL;
}

static void change_state(struct btd_service *service, btd_service_state_t state,
									int err)
{
	btd_service_state_t old = service->state;
	char addr[18];
	GSList *l;

	if (state == old)
		return;

	btd_assert(service->device != NULL);
	btd_assert(service->profile != NULL);

	service->state = state;
	service->err = err;

	ba2str(device_get_address(service->device), addr);
	DBG("%p: device %s profile %s state changed: %s -> %s (%d)", service,
					addr, service->profile->name,
					state2str(old), state2str(state), err);

	for (l = state_callbacks; l != NULL; l = g_slist_next(l)) {
		struct service_state_callback *cb = l->data;

		cb->cb(service, old, state, cb->user_data);
	}
}

struct btd_service *btd_service_ref(struct btd_service *service)
{
	service->ref++;

	DBG("%p: ref=%d", service, service->ref);

	return service;
}

void btd_service_unref(struct btd_service *service)
{
	service->ref--;

	DBG("%p: ref=%d", service, service->ref);

	if (service->ref > 0)
		return;

	g_free(service);
}

struct btd_service *service_create(struct btd_device *device,
						struct btd_profile *profile)
{
	struct btd_service *service;

	service = g_try_new0(struct btd_service, 1);
	if (!service) {
		error("service_create: failed to alloc memory");
		return NULL;
	}

	service->ref = 1;
	service->device = device; /* Weak ref */
	service->profile = profile;
	service->state = BTD_SERVICE_STATE_UNAVAILABLE;

	return service;
}

int service_probe(struct btd_service *service)
{
	char addr[18];
	int err;

	btd_assert(service->state == BTD_SERVICE_STATE_UNAVAILABLE);

	err = service->profile->device_probe(service);
	if (err == 0) {
		change_state(service, BTD_SERVICE_STATE_DISCONNECTED, 0);
		return 0;
	}

	ba2str(device_get_address(service->device), addr);
	error("%s profile probe failed for %s", service->profile->name, addr);

	return err;
}

void service_remove(struct btd_service *service)
{
	change_state(service, BTD_SERVICE_STATE_DISCONNECTED, -ECONNABORTED);
	change_state(service, BTD_SERVICE_STATE_UNAVAILABLE, 0);
	service->profile->device_remove(service);
	service->device = NULL;
	service->profile = NULL;
	btd_service_unref(service);
}

int service_accept(struct btd_service *service)
{
	char addr[18];
	int err;

	switch (service->state) {
	case BTD_SERVICE_STATE_UNAVAILABLE:
		return -EINVAL;
	case BTD_SERVICE_STATE_DISCONNECTED:
		break;
	case BTD_SERVICE_STATE_CONNECTING:
	case BTD_SERVICE_STATE_CONNECTED:
		return 0;
	case BTD_SERVICE_STATE_DISCONNECTING:
		return -EBUSY;
	}

	if (!service->profile->accept)
		goto done;

	err = service->profile->accept(service);
	if (!err)
		goto done;

	ba2str(device_get_address(service->device), addr);
	error("%s profile accept failed for %s", service->profile->name, addr);

	return err;

done:
	change_state(service, BTD_SERVICE_STATE_CONNECTING, 0);
	return 0;
}

int btd_service_connect(struct btd_service *service)
{
	struct btd_profile *profile = service->profile;
	char addr[18];
	int err;

	if (!profile->connect)
		return -ENOTSUP;

	switch (service->state) {
	case BTD_SERVICE_STATE_UNAVAILABLE:
		return -EINVAL;
	case BTD_SERVICE_STATE_DISCONNECTED:
		break;
	case BTD_SERVICE_STATE_CONNECTING:
		return 0;
	case BTD_SERVICE_STATE_CONNECTED:
		return -EALREADY;
	case BTD_SERVICE_STATE_DISCONNECTING:
		return -EBUSY;
	}

	err = profile->connect(service);
	if (err == 0) {
		change_state(service, BTD_SERVICE_STATE_CONNECTING, 0);
		return 0;
	}

	ba2str(device_get_address(service->device), addr);
	error("%s profile connect failed for %s: %s", profile->name, addr,
								strerror(-err));

	return err;
}

int btd_service_disconnect(struct btd_service *service)
{
	struct btd_profile *profile = service->profile;
	char addr[18];
	int err;

	if (!profile->disconnect)
		return -ENOTSUP;

	switch (service->state) {
	case BTD_SERVICE_STATE_UNAVAILABLE:
		return -EINVAL;
	case BTD_SERVICE_STATE_DISCONNECTED:
	case BTD_SERVICE_STATE_DISCONNECTING:
		return -EALREADY;
	case BTD_SERVICE_STATE_CONNECTING:
	case BTD_SERVICE_STATE_CONNECTED:
		break;
	}

	change_state(service, BTD_SERVICE_STATE_DISCONNECTING, 0);

	err = profile->disconnect(service);
	if (err == 0)
		return 0;

	if (err == -ENOTCONN) {
		btd_service_disconnecting_complete(service, 0);
		return 0;
	}

	ba2str(device_get_address(service->device), addr);
	error("%s profile disconnect failed for %s: %s", profile->name, addr,
								strerror(-err));

	btd_service_disconnecting_complete(service, err);

	return err;
}

struct btd_device *btd_service_get_device(const struct btd_service *service)
{
	return service->device;
}

struct btd_profile *btd_service_get_profile(const struct btd_service *service)
{
	return service->profile;
}

void btd_service_set_user_data(struct btd_service *service, void *user_data)
{
	btd_assert(service->state == BTD_SERVICE_STATE_UNAVAILABLE);
	service->user_data = user_data;
}

void *btd_service_get_user_data(const struct btd_service *service)
{
	return service->user_data;
}

btd_service_state_t btd_service_get_state(const struct btd_service *service)
{
	return service->state;
}

int btd_service_get_error(const struct btd_service *service)
{
	return service->err;
}

unsigned int btd_service_add_state_cb(btd_service_state_cb cb, void *user_data)
{
	struct service_state_callback *state_cb;
	static unsigned int id = 0;

	state_cb = g_new0(struct service_state_callback, 1);
	state_cb->cb = cb;
	state_cb->user_data = user_data;
	state_cb->id = ++id;

	state_callbacks = g_slist_append(state_callbacks, state_cb);

	return state_cb->id;
}

bool btd_service_remove_state_cb(unsigned int id)
{
	GSList *l;

	for (l = state_callbacks; l != NULL; l = g_slist_next(l)) {
		struct service_state_callback *cb = l->data;

		if (cb && cb->id == id) {
			state_callbacks = g_slist_remove(state_callbacks, cb);
			g_free(cb);
			return true;
		}
	}

	return false;
}

void btd_service_connecting_complete(struct btd_service *service, int err)
{
	if (service->state != BTD_SERVICE_STATE_DISCONNECTED &&
			service->state != BTD_SERVICE_STATE_CONNECTING)
		return;

	if (err == 0)
		change_state(service, BTD_SERVICE_STATE_CONNECTED, 0);
	else
		change_state(service, BTD_SERVICE_STATE_DISCONNECTED, err);
}

void btd_service_disconnecting_complete(struct btd_service *service, int err)
{
	if (service->state != BTD_SERVICE_STATE_CONNECTED &&
			service->state != BTD_SERVICE_STATE_DISCONNECTING)
		return;

	if (err == 0)
		change_state(service, BTD_SERVICE_STATE_DISCONNECTED, 0);
	else /* If disconnect fails, we assume it remains connected */
		change_state(service, BTD_SERVICE_STATE_CONNECTED, err);
}
