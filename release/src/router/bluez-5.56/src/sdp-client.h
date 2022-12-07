/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

typedef void (*bt_callback_t) (sdp_list_t *recs, int err, gpointer user_data);
typedef void (*bt_destroy_t) (gpointer user_data);

int bt_search(const bdaddr_t *src, const bdaddr_t *dst,
			uuid_t *uuid, bt_callback_t cb, void *user_data,
			bt_destroy_t destroy, uint16_t flags);
int bt_search_service(const bdaddr_t *src, const bdaddr_t *dst,
			uuid_t *uuid, bt_callback_t cb, void *user_data,
			bt_destroy_t destroy, uint16_t flags);
int bt_cancel_discovery(const bdaddr_t *src, const bdaddr_t *dst);
void bt_clear_cached_session(const bdaddr_t *src, const bdaddr_t *dst);
