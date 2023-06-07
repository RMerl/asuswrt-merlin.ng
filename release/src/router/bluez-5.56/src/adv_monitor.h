/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2020 Google LLC
 *
 *
 */

#ifndef __ADV_MONITOR_H
#define __ADV_MONITOR_H

#include <glib.h>

#include "src/shared/ad.h"

struct mgmt;
struct queue;
struct btd_device;
struct btd_adapter;
struct btd_adv_monitor_manager;
struct btd_adv_monitor_pattern;

struct btd_adv_monitor_manager *btd_adv_monitor_manager_create(
						struct btd_adapter *adapter,
						struct mgmt *mgmt);
void btd_adv_monitor_manager_destroy(struct btd_adv_monitor_manager *manager);

struct queue *btd_adv_monitor_content_filter(
				struct btd_adv_monitor_manager *manager,
				struct bt_ad *ad);

void btd_adv_monitor_notify_monitors(struct btd_adv_monitor_manager *manager,
					struct btd_device *device, int8_t rssi,
					struct queue *matched_monitors);

void btd_adv_monitor_device_remove(struct btd_adv_monitor_manager *manager,
				   struct btd_device *device);

#endif /* __ADV_MONITOR_H */
