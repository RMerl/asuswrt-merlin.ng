/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Google Inc.
 *
 *
 */

struct btd_adapter;
struct btd_adv_manager;

struct btd_adv_manager *btd_adv_manager_new(struct btd_adapter *adapter,
							struct mgmt *mgmt);
void btd_adv_manager_destroy(struct btd_adv_manager *manager);
void btd_adv_manager_refresh(struct btd_adv_manager *manager);
