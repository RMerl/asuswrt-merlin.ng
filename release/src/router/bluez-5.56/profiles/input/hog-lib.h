/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 */

struct bt_hog;

struct bt_hog *bt_hog_new_default(const char *name, uint16_t vendor,
					uint16_t product, uint16_t version,
					struct gatt_db *db);

struct bt_hog *bt_hog_new(int fd, const char *name, uint16_t vendor,
					uint16_t product, uint16_t version,
					struct gatt_db *db);

struct bt_hog *bt_hog_ref(struct bt_hog *hog);
void bt_hog_unref(struct bt_hog *hog);

bool bt_hog_attach(struct bt_hog *hog, void *gatt);
void bt_hog_detach(struct bt_hog *hog);

int bt_hog_set_control_point(struct bt_hog *hog, bool suspend);
int bt_hog_send_report(struct bt_hog *hog, void *data, size_t size, int type);
