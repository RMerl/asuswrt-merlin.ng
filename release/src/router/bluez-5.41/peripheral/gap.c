/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Intel Corporation. All rights reserved.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include "lib/bluetooth.h"
#include "lib/mgmt.h"
#include "src/shared/util.h"
#include "src/shared/mgmt.h"
#include "peripheral/gatt.h"
#include "peripheral/gap.h"

static struct mgmt *mgmt = NULL;
static uint16_t mgmt_index = MGMT_INDEX_NONE;

static bool adv_features = false;
static bool adv_instances = false;
static bool require_connectable = true;

static uint8_t static_addr[6] = { 0x90, 0x78, 0x56, 0x34, 0x12, 0xc0 };
static uint8_t dev_name[260] = { 0x00, };
static uint8_t dev_name_len = 0;

void gap_set_static_address(uint8_t addr[6])
{
	memcpy(static_addr, addr, sizeof(static_addr));

	printf("Using static address %02x:%02x:%02x:%02x:%02x:%02x\n",
			static_addr[5], static_addr[4], static_addr[3],
			static_addr[2], static_addr[1], static_addr[0]);
}

static void clear_long_term_keys(uint16_t index)
{
	struct mgmt_cp_load_long_term_keys cp;

	memset(&cp, 0, sizeof(cp));
	cp.key_count = cpu_to_le16(0);

	mgmt_send(mgmt, MGMT_OP_LOAD_LONG_TERM_KEYS, index,
					sizeof(cp), &cp, NULL, NULL, NULL);
}

static void clear_identity_resolving_keys(uint16_t index)
{
	struct mgmt_cp_load_irks cp;

	memset(&cp, 0, sizeof(cp));
	cp.irk_count = cpu_to_le16(0);

	mgmt_send(mgmt, MGMT_OP_LOAD_IRKS, index,
					sizeof(cp), &cp, NULL, NULL, NULL);
}

static void add_advertising(uint16_t index)
{
	const char ad[] = { 0x11, 0x15,
			0xd0, 0x00, 0x2d, 0x12, 0x1e, 0x4b, 0x0f, 0xa4,
			0x99, 0x4e, 0xce, 0xb5, 0x31, 0xf4, 0x05, 0x79 };
	struct mgmt_cp_add_advertising *cp;
	void *buf;

	buf = malloc(sizeof(*cp) + sizeof(ad));
	if (!buf)
		return;

	memset(buf, 0, sizeof(*cp) + sizeof(ad));
	cp = buf;
	cp->instance = 0x01;
	cp->flags = cpu_to_le32((1 << 0) | (1 << 1) | (1 << 4));
	cp->duration = cpu_to_le16(0);
	cp->timeout = cpu_to_le16(0);
	cp->adv_data_len = sizeof(ad);
	cp->scan_rsp_len = 0;
	memcpy(cp->data, ad, sizeof(ad));

	mgmt_send(mgmt, MGMT_OP_ADD_ADVERTISING, index,
			sizeof(*cp) + sizeof(ad), buf, NULL, NULL, NULL);

	free(buf);
}

static void enable_advertising(uint16_t index)
{
	uint8_t val;

	val = require_connectable ? 0x01 : 0x00;
	mgmt_send(mgmt, MGMT_OP_SET_CONNECTABLE, index, 1, &val,
						NULL, NULL, NULL);

	val = 0x01;
	mgmt_send(mgmt, MGMT_OP_SET_POWERED, index, 1, &val,
						NULL, NULL, NULL);

	if (adv_instances) {
		add_advertising(index);
		return;
	}

	val = require_connectable ? 0x01 : 0x02;
	mgmt_send(mgmt, MGMT_OP_SET_ADVERTISING, index, 1, &val,
						NULL, NULL, NULL);
}

static void new_settings_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("New settings\n");
}

static void local_name_changed_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("Local name changed\n");
}

static void new_long_term_key_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("New long term key\n");
}

static void device_connected_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("Device connected\n");
}

static void device_disconnected_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("Device disconnected\n");
}

static void user_confirm_request_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("User confirm request\n");
}

static void user_passkey_request_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("User passkey request\n");
}

static void auth_failed_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("Authentication failed\n");
}

static void device_unpaired_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("Device unpaired\n");
}

static void passkey_notify_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("Passkey notification\n");
}

static void new_irk_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("New identify resolving key\n");
}

static void new_csrk_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("New connection signature resolving key\n");
}

static void new_conn_param_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("New connection parameter\n");
}

static void advertising_added_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("Advertising added\n");
}

static void advertising_removed_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("Advertising removed\n");
}

static void read_adv_features_complete(uint8_t status, uint16_t len,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_adv_features *rp = param;
	uint16_t index = PTR_TO_UINT(user_data);
	uint32_t flags;

	flags = le32_to_cpu(rp->supported_flags);

	if (rp->max_instances > 0) {
		adv_instances = true;

		if (flags & (1 << 0))
			require_connectable = false;
	} else
		require_connectable = false;

	enable_advertising(index);
}

static void read_info_complete(uint8_t status, uint16_t len,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_info *rp = param;
	uint16_t index = PTR_TO_UINT(user_data);
	uint32_t required_settings = MGMT_SETTING_LE |
					MGMT_SETTING_STATIC_ADDRESS;
	uint32_t supported_settings, current_settings;
	uint8_t val;

	required_settings = MGMT_SETTING_LE;

	if (status) {
		fprintf(stderr, "Reading info for index %u failed: %s\n",
						index, mgmt_errstr(status));
		return;
	}

	if (mgmt_index != MGMT_INDEX_NONE)
		return;

	supported_settings = le32_to_cpu(rp->supported_settings);
	current_settings = le32_to_cpu(rp->current_settings);

	if ((supported_settings & required_settings) != required_settings)
		return;

	printf("Selecting index %u\n", index);
	mgmt_index = index;

	mgmt_register(mgmt, MGMT_EV_NEW_SETTINGS, index,
					new_settings_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_LOCAL_NAME_CHANGED, index,
					local_name_changed_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_NEW_LONG_TERM_KEY, index,
					new_long_term_key_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_DEVICE_CONNECTED, index,
					device_connected_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_DEVICE_DISCONNECTED, index,
					device_disconnected_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_USER_CONFIRM_REQUEST, index,
					user_confirm_request_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_USER_PASSKEY_REQUEST, index,
					user_passkey_request_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_AUTH_FAILED, index,
					auth_failed_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_DEVICE_UNPAIRED, index,
					device_unpaired_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_PASSKEY_NOTIFY, index,
					passkey_notify_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_NEW_IRK, index,
					new_irk_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_NEW_CSRK, index,
					new_csrk_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_NEW_CONN_PARAM, index,
					new_conn_param_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_ADVERTISING_ADDED, index,
					advertising_added_event, NULL, NULL);
	mgmt_register(mgmt, MGMT_EV_ADVERTISING_REMOVED, index,
					advertising_removed_event, NULL, NULL);

	dev_name_len = snprintf((char *) dev_name, 26, "BlueZ Peripheral");

	if (current_settings & MGMT_SETTING_POWERED) {
		val = 0x00;
		mgmt_send(mgmt, MGMT_OP_SET_POWERED, index, 1, &val,
							NULL, NULL, NULL);
	}

	if (!(current_settings & MGMT_SETTING_LE)) {
		val = 0x01;
		mgmt_send(mgmt, MGMT_OP_SET_LE, index, 1, &val,
							NULL, NULL, NULL);
	}

	if (current_settings & MGMT_SETTING_BREDR) {
		val = 0x00;
		mgmt_send(mgmt, MGMT_OP_SET_BREDR, index, 1, &val,
							NULL, NULL, NULL);
	}

	if ((supported_settings & MGMT_SETTING_SECURE_CONN) &&
			!(current_settings & MGMT_SETTING_SECURE_CONN)) {
		val = 0x01;
		mgmt_send(mgmt, MGMT_OP_SET_SECURE_CONN, index, 1, &val,
							NULL, NULL, NULL);
	}

	if (current_settings & MGMT_SETTING_DEBUG_KEYS) {
		val = 0x00;
		mgmt_send(mgmt, MGMT_OP_SET_DEBUG_KEYS, index, 1, &val,
							NULL, NULL, NULL);
	}

	if (!(current_settings & MGMT_SETTING_BONDABLE)) {
		val = 0x01;
		mgmt_send(mgmt, MGMT_OP_SET_BONDABLE, index, 1, &val,
							NULL, NULL, NULL);
	}

	clear_long_term_keys(mgmt_index);
	clear_identity_resolving_keys(mgmt_index);

	mgmt_send(mgmt, MGMT_OP_SET_STATIC_ADDRESS, index,
					6, static_addr, NULL, NULL, NULL);

	mgmt_send(mgmt, MGMT_OP_SET_LOCAL_NAME, index,
					260, dev_name, NULL, NULL, NULL);

	gatt_set_static_address(static_addr);
	gatt_set_device_name(dev_name, dev_name_len);
	gatt_server_start();

	if (adv_features)
		mgmt_send(mgmt, MGMT_OP_READ_ADV_FEATURES, index, 0, NULL,
						read_adv_features_complete,
						UINT_TO_PTR(index), NULL);
	else
		enable_advertising(index);
}

static void read_index_list_complete(uint8_t status, uint16_t len,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_index_list *rp = param;
	uint16_t count;
	int i;

	if (status) {
		fprintf(stderr, "Reading index list failed: %s\n",
						mgmt_errstr(status));
		return;
	}

	count = le16_to_cpu(rp->num_controllers);

	printf("Index list: %u\n", count);

	for (i = 0; i < count; i++) {
		uint16_t index = cpu_to_le16(rp->index[i]);

		mgmt_send(mgmt, MGMT_OP_READ_INFO, index, 0, NULL,
				read_info_complete, UINT_TO_PTR(index), NULL);
	}
}

static void index_added_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("Index added\n");

	if (mgmt_index != MGMT_INDEX_NONE)
		return;

	mgmt_send(mgmt, MGMT_OP_READ_INFO, index, 0, NULL,
				read_info_complete, UINT_TO_PTR(index), NULL);
}

static void index_removed_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	printf("Index removed\n");

	if (mgmt_index != index)
		return;

	mgmt_index = MGMT_INDEX_NONE;
}

static void read_ext_index_list_complete(uint8_t status, uint16_t len,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_ext_index_list *rp = param;
	uint16_t count;
	int i;

	if (status) {
		fprintf(stderr, "Reading extended index list failed: %s\n",
						mgmt_errstr(status));
		return;
	}

	count = le16_to_cpu(rp->num_controllers);

	printf("Extended index list: %u\n", count);

	for (i = 0; i < count; i++) {
		uint16_t index = cpu_to_le16(rp->entry[i].index);

		if (rp->entry[i].type != 0x00)
			continue;

		mgmt_send(mgmt, MGMT_OP_READ_INFO, index, 0, NULL,
				read_info_complete, UINT_TO_PTR(index), NULL);
	}
}

static void ext_index_added_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_ext_index_added *ev = param;

	printf("Extended index added: %u\n", ev->type);

	if (mgmt_index != MGMT_INDEX_NONE)
		return;

	if (ev->type != 0x00)
		return;

	mgmt_send(mgmt, MGMT_OP_READ_INFO, index, 0, NULL,
				read_info_complete, UINT_TO_PTR(index), NULL);
}

static void ext_index_removed_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_ext_index_added *ev = param;

	printf("Extended index removed: %u\n", ev->type);

	if (mgmt_index != index)
		return;

	if (ev->type != 0x00)
		return;

	mgmt_index = MGMT_INDEX_NONE;
}

static void read_commands_complete(uint8_t status, uint16_t len,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_commands *rp = param;
	const uint16_t *opcode;
	uint16_t num_commands;
	bool ext_index_list = false;
	int i;

	if (status) {
		fprintf(stderr, "Reading index list failed: %s\n",
						mgmt_errstr(status));
		return;
	}

	num_commands = le16_to_cpu(rp->num_commands);
	opcode = rp->opcodes;

	for (i = 0; i < num_commands; i++) {
		uint16_t op = get_le16(opcode++);

		if (op == MGMT_OP_READ_EXT_INDEX_LIST)
			ext_index_list = true;
		else if (op == MGMT_OP_READ_ADV_FEATURES)
			adv_features = true;
	}

	if (ext_index_list) {
		mgmt_register(mgmt, MGMT_EV_EXT_INDEX_ADDED, MGMT_INDEX_NONE,
					ext_index_added_event, NULL, NULL);
		mgmt_register(mgmt, MGMT_EV_EXT_INDEX_REMOVED, MGMT_INDEX_NONE,
					ext_index_removed_event, NULL, NULL);

		if (!mgmt_send(mgmt, MGMT_OP_READ_EXT_INDEX_LIST,
				MGMT_INDEX_NONE, 0, NULL,
				read_ext_index_list_complete, NULL, NULL)) {
			fprintf(stderr, "Failed to read extended index list\n");
			return;
		}
	} else {
		mgmt_register(mgmt, MGMT_EV_INDEX_ADDED, MGMT_INDEX_NONE,
					index_added_event, NULL, NULL);
		mgmt_register(mgmt, MGMT_EV_INDEX_REMOVED, MGMT_INDEX_NONE,
					index_removed_event, NULL, NULL);

		if (!mgmt_send(mgmt, MGMT_OP_READ_INDEX_LIST,
				MGMT_INDEX_NONE, 0, NULL,
				read_index_list_complete, NULL, NULL)) {
			fprintf(stderr, "Failed to read index list\n");
			return;
		}
	}
}

void gap_start(void)
{
	mgmt = mgmt_new_default();
	if (!mgmt) {
		fprintf(stderr, "Failed to open management socket\n");
		return;
	}

	if (!mgmt_send(mgmt, MGMT_OP_READ_COMMANDS,
				MGMT_INDEX_NONE, 0, NULL,
				read_commands_complete, NULL, NULL)) {
		fprintf(stderr, "Failed to read supported commands\n");
		return;
	}
}

void gap_stop(void)
{
	if (!mgmt)
		return;

	gatt_server_stop();

        mgmt_unref(mgmt);
	mgmt = NULL;

	mgmt_index = MGMT_INDEX_NONE;
}
