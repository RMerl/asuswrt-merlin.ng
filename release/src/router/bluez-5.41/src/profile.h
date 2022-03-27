/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
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

#define BTD_PROFILE_PRIORITY_LOW	0
#define BTD_PROFILE_PRIORITY_MEDIUM	1
#define BTD_PROFILE_PRIORITY_HIGH	2

struct btd_service;

struct btd_profile {
	const char *name;
	int priority;

	const char *local_uuid;
	const char *remote_uuid;

	bool auto_connect;
	bool external;

	int (*device_probe) (struct btd_service *service);
	void (*device_remove) (struct btd_service *service);

	int (*connect) (struct btd_service *service);
	int (*disconnect) (struct btd_service *service);

	int (*accept) (struct btd_service *service);

	int (*adapter_probe) (struct btd_profile *p,
						struct btd_adapter *adapter);
	void (*adapter_remove) (struct btd_profile *p,
						struct btd_adapter *adapter);
};

void btd_profile_foreach(void (*func)(struct btd_profile *p, void *data),
								void *data);

int btd_profile_register(struct btd_profile *profile);
void btd_profile_unregister(struct btd_profile *profile);

typedef bool (*btd_profile_prop_exists)(const char *uuid,
						struct btd_device *dev,
						void *user_data);

typedef bool (*btd_profile_prop_get)(const char *uuid,
						struct btd_device *dev,
						DBusMessageIter *iter,
						void *user_data);

bool btd_profile_add_custom_prop(const char *uuid, const char *type,
					const char *name,
					btd_profile_prop_exists exists,
					btd_profile_prop_get get,
					void *user_data);
bool btd_profile_remove_custom_prop(const char *uuid, const char *name);

void btd_profile_init(void);
void btd_profile_cleanup(void);
