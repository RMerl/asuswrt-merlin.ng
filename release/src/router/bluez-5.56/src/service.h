/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2013  BMW Car IT GmbH. All rights reserved.
 *
 *
 */

typedef enum {
	BTD_SERVICE_STATE_UNAVAILABLE, /* Not probed */
	BTD_SERVICE_STATE_DISCONNECTED,
	BTD_SERVICE_STATE_CONNECTING,
	BTD_SERVICE_STATE_CONNECTED,
	BTD_SERVICE_STATE_DISCONNECTING,
} btd_service_state_t;

struct btd_service;
struct btd_device;
struct btd_profile;

typedef void (*btd_service_state_cb) (struct btd_service *service,
						btd_service_state_t old_state,
						btd_service_state_t new_state,
						void *user_data);

struct btd_service *btd_service_ref(struct btd_service *service);
void btd_service_unref(struct btd_service *service);

/* Service management functions used by the core */
struct btd_service *service_create(struct btd_device *device,
						struct btd_profile *profile);

int service_probe(struct btd_service *service);
void service_remove(struct btd_service *service);

int service_accept(struct btd_service *service);
int service_set_connecting(struct btd_service *service);

/* Connection control API */
int btd_service_connect(struct btd_service *service);
int btd_service_disconnect(struct btd_service *service);

/* Public member access */
struct btd_device *btd_service_get_device(const struct btd_service *service);
struct btd_profile *btd_service_get_profile(const struct btd_service *service);
btd_service_state_t btd_service_get_state(const struct btd_service *service);
int btd_service_get_error(const struct btd_service *service);

unsigned int btd_service_add_state_cb(btd_service_state_cb cb,
							void *user_data);
bool btd_service_remove_state_cb(unsigned int id);

/* Functions used by profile implementation */
void btd_service_connecting_complete(struct btd_service *service, int err);
void btd_service_disconnecting_complete(struct btd_service *service, int err);
void btd_service_set_user_data(struct btd_service *service, void *user_data);
void *btd_service_get_user_data(const struct btd_service *service);
