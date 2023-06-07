/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2020  Google LLC
 *
 *
 */

struct btd_adapter;
struct btd_battery;
struct btd_battery_provider_manager;

struct btd_battery *btd_battery_register(const char *path, const char *source,
					 const char *provider_path);
bool btd_battery_unregister(struct btd_battery *battery);
bool btd_battery_update(struct btd_battery *battery, uint8_t percentage);

struct btd_battery_provider_manager *
btd_battery_provider_manager_create(struct btd_adapter *adapter);
void btd_battery_provider_manager_destroy(
	struct btd_battery_provider_manager *manager);
