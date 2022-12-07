/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010  Nokia Corporation
 *  Copyright (C) 2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

uint16_t attrib_db_find_avail(struct btd_adapter *adapter, bt_uuid_t *svc_uuid,
							uint16_t nitems);
struct attribute *attrib_db_add(struct btd_adapter *adapter, uint16_t handle,
				bt_uuid_t *uuid, int read_req,
				int write_req, const uint8_t *value,
				size_t len);
int attrib_db_update(struct btd_adapter *adapter, uint16_t handle,
					bt_uuid_t *uuid, const uint8_t *value,
					size_t len, struct attribute **attr);
int attrib_db_del(struct btd_adapter *adapter, uint16_t handle);
int attrib_gap_set(struct btd_adapter *adapter, uint16_t uuid,
					const uint8_t *value, size_t len);
uint32_t attrib_create_sdp(struct btd_adapter *adapter, uint16_t handle,
							const char *name);
void attrib_free_sdp(struct btd_adapter *adapter, uint32_t sdp_handle);
GAttrib *attrib_from_device(struct btd_device *device);
guint attrib_channel_attach(GAttrib *attrib);
gboolean attrib_channel_detach(GAttrib *attrib, guint id);
