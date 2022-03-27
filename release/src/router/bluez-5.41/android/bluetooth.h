/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

typedef void (*bt_bluetooth_ready)(int err, const bdaddr_t *addr);
bool bt_bluetooth_start(int index, bool mgmt_dbg, bt_bluetooth_ready cb);

typedef void (*bt_bluetooth_stopped)(void);
bool bt_bluetooth_stop(bt_bluetooth_stopped cb);

void bt_bluetooth_cleanup(void);

bool bt_bluetooth_register(struct ipc *ipc, uint8_t mode);
void bt_bluetooth_unregister(void);

int bt_adapter_add_record(sdp_record_t *rec, uint8_t svc_hint);
void bt_adapter_remove_record(uint32_t handle);

typedef void (*bt_le_device_found)(const bdaddr_t *addr, int rssi,
					uint16_t eir_len, const void *eir,
					bool connectable, bool bonded);
bool bt_le_register(bt_le_device_found cb);
void bt_le_unregister(void);

bool bt_le_discovery_start(void);

typedef void (*bt_le_discovery_stopped)(void);
bool bt_le_discovery_stop(bt_le_discovery_stopped cb);

typedef void (*bt_le_set_advertising_done)(uint8_t status, void *user_data);
bool bt_le_set_advertising(bool advertising, bt_le_set_advertising_done cb,
							void *user_data);

uint8_t bt_get_device_android_type(const bdaddr_t *addr);
bool bt_is_device_le(const bdaddr_t *addr);
uint8_t bt_device_last_seen_bearer(const bdaddr_t *bdaddr);

const char *bt_get_adapter_name(void);
bool bt_device_is_bonded(const bdaddr_t *bdaddr);
bool bt_device_set_uuids(const bdaddr_t *bdaddr, GSList *uuids);

typedef void (*bt_read_device_rssi_done)(uint8_t status, const bdaddr_t *addr,
						int8_t rssi, void *user_data);
bool bt_read_device_rssi(const bdaddr_t *addr, bt_read_device_rssi_done cb,
							void *user_data);

bool bt_get_csrk(const bdaddr_t *addr, bool local, uint8_t key[16],
				uint32_t *sign_cnt, bool *authenticated);

void bt_update_sign_counter(const bdaddr_t *addr, bool local, uint32_t val);

void bt_store_gatt_ccc(const bdaddr_t *addr, uint16_t value);

uint16_t bt_get_gatt_ccc(const bdaddr_t *addr);

const bdaddr_t *bt_get_id_addr(const bdaddr_t *addr, uint8_t *type);

bool bt_kernel_conn_control(void);

bool bt_auto_connect_add(const bdaddr_t *addr);

void bt_auto_connect_remove(const bdaddr_t *addr);

typedef void (*bt_unpaired_device_cb)(const bdaddr_t *addr);
bool bt_unpaired_register(bt_unpaired_device_cb cb);
void bt_unpaired_unregister(bt_unpaired_device_cb cb);

typedef void (*bt_paired_device_cb)(const bdaddr_t *addr);
bool bt_paired_register(bt_paired_device_cb cb);
void bt_paired_unregister(bt_paired_device_cb cb);
bool bt_is_pairing(const bdaddr_t *addr);
