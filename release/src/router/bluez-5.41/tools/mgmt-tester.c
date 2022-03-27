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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <sys/ioctl.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/mgmt.h"

#include "monitor/bt.h"
#include "emulator/bthost.h"
#include "emulator/hciemu.h"

#include "src/shared/util.h"
#include "src/shared/tester.h"
#include "src/shared/mgmt.h"

struct test_data {
	tester_data_func_t test_setup;
	const void *test_data;
	uint8_t expected_version;
	uint16_t expected_manufacturer;
	uint32_t expected_supported_settings;
	uint32_t initial_settings;
	struct mgmt *mgmt;
	struct mgmt *mgmt_alt;
	unsigned int mgmt_settings_id;
	unsigned int mgmt_alt_settings_id;
	unsigned int mgmt_alt_ev_id;
	unsigned int mgmt_discov_ev_id;
	uint8_t mgmt_version;
	uint16_t mgmt_revision;
	uint16_t mgmt_index;
	struct hciemu *hciemu;
	enum hciemu_type hciemu_type;
	int unmet_conditions;
};

static void mgmt_debug(const char *str, void *user_data)
{
	const char *prefix = user_data;

	tester_print("%s%s", prefix, str);
}

static void read_version_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();
	const struct mgmt_rp_read_version *rp = param;

	tester_print("Read Version callback");
	tester_print("  Status: %s (0x%02x)", mgmt_errstr(status), status);

	if (status || !param) {
		tester_pre_setup_failed();
		return;
	}

	data->mgmt_version = rp->version;
	data->mgmt_revision = btohs(rp->revision);

	tester_print("  Version %u.%u",
				data->mgmt_version, data->mgmt_revision);
}

static void read_commands_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	tester_print("Read Commands callback");
	tester_print("  Status: %s (0x%02x)", mgmt_errstr(status), status);

	if (status || !param) {
		tester_pre_setup_failed();
		return;
	}
}

static void read_info_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();
	const struct mgmt_rp_read_info *rp = param;
	char addr[18];
	uint16_t manufacturer;
	uint32_t supported_settings, current_settings;
	struct bthost *bthost;

	tester_print("Read Info callback");
	tester_print("  Status: %s (0x%02x)", mgmt_errstr(status), status);

	if (status || !param) {
		tester_pre_setup_failed();
		return;
	}

	ba2str(&rp->bdaddr, addr);
	manufacturer = btohs(rp->manufacturer);
	supported_settings = btohl(rp->supported_settings);
	current_settings = btohl(rp->current_settings);

	tester_print("  Address: %s", addr);
	tester_print("  Version: 0x%02x", rp->version);
	tester_print("  Manufacturer: 0x%04x", manufacturer);
	tester_print("  Supported settings: 0x%08x", supported_settings);
	tester_print("  Current settings: 0x%08x", current_settings);
	tester_print("  Class: 0x%02x%02x%02x",
			rp->dev_class[2], rp->dev_class[1], rp->dev_class[0]);
	tester_print("  Name: %s", rp->name);
	tester_print("  Short name: %s", rp->short_name);

	if (strcmp(hciemu_get_address(data->hciemu), addr)) {
		tester_pre_setup_failed();
		return;
	}

	if (rp->version != data->expected_version) {
		tester_pre_setup_failed();
		return;
	}

	if (manufacturer != data->expected_manufacturer) {
		tester_pre_setup_failed();
		return;
	}

	if (supported_settings != data->expected_supported_settings) {
		tester_pre_setup_failed();
		return;
	}

	if (current_settings != data->initial_settings) {
		tester_pre_setup_failed();
		return;
	}

	if (rp->dev_class[0] != 0x00 || rp->dev_class[1] != 0x00 ||
						rp->dev_class[2] != 0x00) {
		tester_pre_setup_failed();
		return;
	}

	bthost = hciemu_client_get_host(data->hciemu);
	bthost_notify_ready(bthost, tester_pre_setup_complete);
}

static void index_added_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	tester_print("Index Added callback");
	tester_print("  Index: 0x%04x", index);

	data->mgmt_index = index;

	mgmt_send(data->mgmt, MGMT_OP_READ_INFO, data->mgmt_index, 0, NULL,
					read_info_callback, NULL, NULL);
}

static void index_removed_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	tester_print("Index Removed callback");
	tester_print("  Index: 0x%04x", index);

	if (index != data->mgmt_index)
		return;

	mgmt_unregister_index(data->mgmt, data->mgmt_index);
	mgmt_unregister_index(data->mgmt_alt, data->mgmt_index);

	mgmt_unref(data->mgmt);
	data->mgmt = NULL;

	mgmt_unref(data->mgmt_alt);
	data->mgmt_alt = NULL;

	tester_post_teardown_complete();
}

static void read_index_list_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	tester_print("Read Index List callback");
	tester_print("  Status: %s (0x%02x)", mgmt_errstr(status), status);

	if (status || !param) {
		tester_pre_setup_failed();
		return;
	}

	mgmt_register(data->mgmt, MGMT_EV_INDEX_ADDED, MGMT_INDEX_NONE,
					index_added_callback, NULL, NULL);

	mgmt_register(data->mgmt, MGMT_EV_INDEX_REMOVED, MGMT_INDEX_NONE,
					index_removed_callback, NULL, NULL);

	data->hciemu = hciemu_new(data->hciemu_type);
	if (!data->hciemu) {
		tester_warn("Failed to setup HCI emulation");
		tester_pre_setup_failed();
	}
}

static void test_pre_setup(const void *test_data)
{
	struct test_data *data = tester_get_data();

	data->mgmt = mgmt_new_default();
	if (!data->mgmt) {
		tester_warn("Failed to setup management interface");
		tester_pre_setup_failed();
		return;
	}

	data->mgmt_alt = mgmt_new_default();
	if (!data->mgmt_alt) {
		tester_warn("Failed to setup alternate management interface");
		tester_pre_setup_failed();

		mgmt_unref(data->mgmt);
		data->mgmt = NULL;
		return;
	}

	if (tester_use_debug()) {
		mgmt_set_debug(data->mgmt, mgmt_debug, "mgmt: ", NULL);
		mgmt_set_debug(data->mgmt_alt, mgmt_debug, "mgmt-alt: ", NULL);
	}

	mgmt_send(data->mgmt, MGMT_OP_READ_VERSION, MGMT_INDEX_NONE, 0, NULL,
					read_version_callback, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_READ_COMMANDS, MGMT_INDEX_NONE, 0, NULL,
					read_commands_callback, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_READ_INDEX_LIST, MGMT_INDEX_NONE, 0, NULL,
					read_index_list_callback, NULL, NULL);
}

static void test_post_teardown(const void *test_data)
{
	struct test_data *data = tester_get_data();

	hciemu_unref(data->hciemu);
	data->hciemu = NULL;
}

static void test_add_condition(struct test_data *data)
{
	data->unmet_conditions++;

	tester_print("Test condition added, total %d", data->unmet_conditions);
}

static void test_condition_complete(struct test_data *data)
{
	data->unmet_conditions--;

	tester_print("Test condition complete, %d left",
						data->unmet_conditions);

	if (data->unmet_conditions > 0)
		return;

	tester_test_passed();
}

#define test_bredrle_full(name, data, setup, func, timeout) \
	do { \
		struct test_data *user; \
		user = malloc(sizeof(struct test_data)); \
		if (!user) \
			break; \
		user->hciemu_type = HCIEMU_TYPE_BREDRLE; \
		user->test_setup = setup; \
		user->test_data = data; \
		user->expected_version = 0x08; \
		user->expected_manufacturer = 0x003f; \
		user->expected_supported_settings = 0x0000bfff; \
		user->initial_settings = 0x00000080; \
		user->unmet_conditions = 0; \
		tester_add_full(name, data, \
				test_pre_setup, test_setup, func, NULL, \
				test_post_teardown, timeout, user, free); \
	} while (0)

#define test_bredrle(name, data, setup, func) \
	test_bredrle_full(name, data, setup, func, 2)

#define test_bredr20(name, data, setup, func) \
	do { \
		struct test_data *user; \
		user = malloc(sizeof(struct test_data)); \
		if (!user) \
			break; \
		user->hciemu_type = HCIEMU_TYPE_LEGACY; \
		user->test_setup = setup; \
		user->test_data = data; \
		user->expected_version = 0x03; \
		user->expected_manufacturer = 0x003f; \
		user->expected_supported_settings = 0x000010bf; \
		user->initial_settings = 0x00000080; \
		user->unmet_conditions = 0; \
		tester_add_full(name, data, \
				test_pre_setup, test_setup, func, NULL, \
				test_post_teardown, 2, user, free); \
	} while (0)

#define test_bredr(name, data, setup, func) \
	do { \
		struct test_data *user; \
		user = malloc(sizeof(struct test_data)); \
		if (!user) \
			break; \
		user->hciemu_type = HCIEMU_TYPE_BREDR; \
		user->test_setup = setup; \
		user->test_data = data; \
		user->expected_version = 0x05; \
		user->expected_manufacturer = 0x003f; \
		user->expected_supported_settings = 0x000011ff; \
		user->initial_settings = 0x00000080; \
		user->unmet_conditions = 0; \
		tester_add_full(name, data, \
				test_pre_setup, test_setup, func, NULL, \
				test_post_teardown, 2, user, free); \
	} while (0)

#define test_le(name, data, setup, func) \
	do { \
		struct test_data *user; \
		user = malloc(sizeof(struct test_data)); \
		if (!user) \
			break; \
		user->hciemu_type = HCIEMU_TYPE_LE; \
		user->test_setup = setup; \
		user->test_data = data; \
		user->expected_version = 0x08; \
		user->expected_manufacturer = 0x003f; \
		user->expected_supported_settings = 0x0000be1b; \
		user->initial_settings = 0x00000200; \
		user->unmet_conditions = 0; \
		tester_add_full(name, data, \
				test_pre_setup, test_setup, func, NULL, \
				test_post_teardown, 2, user, free); \
	} while (0)

static void controller_setup(const void *test_data)
{
	tester_test_passed();
}

struct generic_data {
	const uint16_t *setup_settings;
	bool setup_nobredr;
	bool setup_limited_discov;
	uint16_t setup_expect_hci_command;
	const void *setup_expect_hci_param;
	uint8_t setup_expect_hci_len;
	uint16_t setup_send_opcode;
	const void *setup_send_param;
	uint16_t setup_send_len;
	bool send_index_none;
	uint16_t send_opcode;
	const void *send_param;
	uint16_t send_len;
	const void * (*send_func)(uint16_t *len);
	uint8_t expect_status;
	bool expect_ignore_param;
	const void *expect_param;
	uint16_t expect_len;
	const void * (*expect_func)(uint16_t *len);
	uint32_t expect_settings_set;
	uint32_t expect_settings_unset;
	uint16_t expect_alt_ev;
	const void *expect_alt_ev_param;
	bool (*verify_alt_ev_func)(const void *param, uint16_t length);
	uint16_t expect_alt_ev_len;
	uint16_t expect_hci_command;
	const void *expect_hci_param;
	uint8_t expect_hci_len;
	const void * (*expect_hci_func)(uint8_t *len);
	bool expect_pin;
	uint8_t pin_len;
	const void *pin;
	uint8_t client_pin_len;
	const void *client_pin;
	bool client_enable_ssp;
	uint8_t io_cap;
	uint8_t client_io_cap;
	uint8_t client_auth_req;
	bool reject_confirm;
	bool client_reject_confirm;
	bool just_works;
	bool client_enable_le;
	bool client_enable_sc;
	bool client_enable_adv;
	bool expect_sc_key;
	bool force_power_off;
	bool addr_type_avail;
	uint8_t addr_type;
	bool set_adv;
	const uint8_t *adv_data;
	uint8_t adv_data_len;
};

static const char dummy_data[] = { 0x00 };

static const struct generic_data invalid_command_test = {
	.send_opcode = 0xffff,
	.expect_status = MGMT_STATUS_UNKNOWN_COMMAND,
};

static const struct generic_data read_version_success_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_READ_VERSION,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_len = 3,
};

static const struct generic_data read_version_invalid_param_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_READ_VERSION,
	.send_param = dummy_data,
	.send_len = sizeof(dummy_data),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data read_version_invalid_index_test = {
	.send_opcode = MGMT_OP_READ_VERSION,
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static const struct generic_data read_commands_invalid_param_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_READ_COMMANDS,
	.send_param = dummy_data,
	.send_len = sizeof(dummy_data),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data read_commands_invalid_index_test = {
	.send_opcode = MGMT_OP_READ_COMMANDS,
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static const struct generic_data read_index_list_invalid_param_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_READ_INDEX_LIST,
	.send_param = dummy_data,
	.send_len = sizeof(dummy_data),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data read_index_list_invalid_index_test = {
	.send_opcode = MGMT_OP_READ_INDEX_LIST,
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static const struct generic_data read_info_invalid_param_test = {
	.send_opcode = MGMT_OP_READ_INFO,
	.send_param = dummy_data,
	.send_len = sizeof(dummy_data),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data read_info_invalid_index_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_READ_INFO,
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static const struct generic_data read_unconf_index_list_invalid_param_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_READ_UNCONF_INDEX_LIST,
	.send_param = dummy_data,
	.send_len = sizeof(dummy_data),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data read_unconf_index_list_invalid_index_test = {
	.send_opcode = MGMT_OP_READ_UNCONF_INDEX_LIST,
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static const struct generic_data read_config_info_invalid_param_test = {
	.send_opcode = MGMT_OP_READ_CONFIG_INFO,
	.send_param = dummy_data,
	.send_len = sizeof(dummy_data),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data read_config_info_invalid_index_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_READ_CONFIG_INFO,
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static const struct generic_data read_ext_index_list_invalid_param_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_READ_EXT_INDEX_LIST,
	.send_param = dummy_data,
	.send_len = sizeof(dummy_data),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data read_ext_index_list_invalid_index_test = {
	.send_opcode = MGMT_OP_READ_EXT_INDEX_LIST,
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static const char set_powered_on_param[] = { 0x01 };
static const char set_powered_invalid_param[] = { 0x02 };
static const char set_powered_garbage_param[] = { 0x01, 0x00 };
static const char set_powered_settings_param[] = { 0x81, 0x00, 0x00, 0x00 };

static const struct generic_data set_powered_on_success_test = {
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_on_param,
	.send_len = sizeof(set_powered_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_powered_settings_param,
	.expect_len = sizeof(set_powered_settings_param),
	.expect_settings_set = MGMT_SETTING_POWERED,
};

static const struct generic_data set_powered_on_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_SET_POWERED,
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_powered_on_invalid_param_test_2 = {
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_invalid_param,
	.send_len = sizeof(set_powered_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_powered_on_invalid_param_test_3 = {
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_garbage_param,
	.send_len = sizeof(set_powered_garbage_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_powered_on_invalid_index_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_on_param,
	.send_len = sizeof(set_powered_on_param),
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static uint16_t settings_powered_advertising_privacy[] = {
						MGMT_OP_SET_PRIVACY,
						MGMT_OP_SET_ADVERTISING,
						MGMT_OP_SET_POWERED, 0 };

static const char set_adv_off_param[] = { 0x00 };

static const struct generic_data set_powered_on_privacy_adv_test = {
	.setup_settings = settings_powered_advertising_privacy,
	.send_opcode = MGMT_OP_SET_ADVERTISING,
	.send_param = set_adv_off_param,
	.send_len = sizeof(set_adv_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_ignore_param = true,
};

static const uint16_t settings_powered[] = { MGMT_OP_SET_POWERED, 0 };

static const char set_powered_off_param[] = { 0x00 };
static const char set_powered_off_settings_param[] = { 0x80, 0x00, 0x00, 0x00 };
static const char set_powered_off_class_of_dev[] = { 0x00, 0x00, 0x00 };

static const struct generic_data set_powered_off_success_test = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_off_param,
	.send_len = sizeof(set_powered_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_powered_off_settings_param,
	.expect_len = sizeof(set_powered_off_settings_param),
	.expect_settings_unset = MGMT_SETTING_POWERED,
};

static const struct generic_data set_powered_off_class_test = {
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_off_param,
	.send_len = sizeof(set_powered_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_powered_off_settings_param,
	.expect_len = sizeof(set_powered_off_settings_param),
	.expect_settings_unset = MGMT_SETTING_POWERED,
	.expect_alt_ev = MGMT_EV_CLASS_OF_DEV_CHANGED,
	.expect_alt_ev_param = set_powered_off_class_of_dev,
	.expect_alt_ev_len = sizeof(set_powered_off_class_of_dev),
};

static const struct generic_data set_powered_off_invalid_param_test_1 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_POWERED,
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_powered_off_invalid_param_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_invalid_param,
	.send_len = sizeof(set_powered_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_powered_off_invalid_param_test_3 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_garbage_param,
	.send_len = sizeof(set_powered_garbage_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const char set_connectable_on_param[] = { 0x01 };
static const char set_connectable_invalid_param[] = { 0x02 };
static const char set_connectable_garbage_param[] = { 0x01, 0x00 };
static const char set_connectable_settings_param_1[] = { 0x82, 0x00, 0x00, 0x00 };
static const char set_connectable_settings_param_2[] = { 0x83, 0x00, 0x00, 0x00 };
static const char set_connectable_scan_enable_param[] = { 0x02 };

static const struct generic_data set_connectable_on_success_test_1 = {
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_on_param,
	.send_len = sizeof(set_connectable_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_settings_param_1,
	.expect_len = sizeof(set_connectable_settings_param_1),
	.expect_settings_set = MGMT_SETTING_CONNECTABLE,
};

static const struct generic_data set_connectable_on_success_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_on_param,
	.send_len = sizeof(set_connectable_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_settings_param_2,
	.expect_len = sizeof(set_connectable_settings_param_2),
	.expect_settings_set = MGMT_SETTING_CONNECTABLE,
	.expect_hci_command = BT_HCI_CMD_WRITE_SCAN_ENABLE,
	.expect_hci_param = set_connectable_scan_enable_param,
	.expect_hci_len = sizeof(set_connectable_scan_enable_param),
};

static const struct generic_data set_connectable_on_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_connectable_on_invalid_param_test_2 = {
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_invalid_param,
	.send_len = sizeof(set_connectable_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_connectable_on_invalid_param_test_3 = {
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_garbage_param,
	.send_len = sizeof(set_connectable_garbage_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_connectable_on_invalid_index_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_on_param,
	.send_len = sizeof(set_connectable_on_param),
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static uint16_t settings_powered_advertising[] = { MGMT_OP_SET_ADVERTISING,
						MGMT_OP_SET_POWERED, 0 };

static const char set_connectable_le_settings_param_1[] = { 0x02, 0x02, 0x00, 0x00 };
static const char set_connectable_le_settings_param_2[] = { 0x03, 0x02, 0x00, 0x00 };
static const char set_connectable_le_settings_param_3[] = { 0x03, 0x06, 0x00, 0x00 };

static const struct generic_data set_connectable_on_le_test_1 = {
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_on_param,
	.send_len = sizeof(set_connectable_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_le_settings_param_1,
	.expect_len = sizeof(set_connectable_le_settings_param_1),
	.expect_settings_set = MGMT_SETTING_CONNECTABLE,
};

static const struct generic_data set_connectable_on_le_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_on_param,
	.send_len = sizeof(set_connectable_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_le_settings_param_2,
	.expect_len = sizeof(set_connectable_le_settings_param_2),
	.expect_settings_set = MGMT_SETTING_CONNECTABLE,
};

static uint8_t set_connectable_on_adv_param[] = {
		0x00, 0x08,				/* min_interval */
		0x00, 0x08,				/* max_interval */
		0x00,					/* type */
		0x00,					/* own_addr_type */
		0x00,					/* direct_addr_type */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* direct_addr */
		0x07,					/* channel_map */
		0x00,					/* filter_policy */
};

static const struct generic_data set_connectable_on_le_test_3 = {
	.setup_settings = settings_powered_advertising,
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_on_param,
	.send_len = sizeof(set_connectable_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_le_settings_param_3,
	.expect_len = sizeof(set_connectable_le_settings_param_3),
	.expect_settings_set = MGMT_SETTING_CONNECTABLE,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
	.expect_hci_param = set_connectable_on_adv_param,
	.expect_hci_len = sizeof(set_connectable_on_adv_param),
};

static const uint16_t settings_connectable[] = { MGMT_OP_SET_CONNECTABLE, 0 };
static const uint16_t settings_powered_connectable[] = {
						MGMT_OP_SET_CONNECTABLE,
						MGMT_OP_SET_POWERED, 0 };
static const uint16_t settings_powered_discoverable[] = {
						MGMT_OP_SET_CONNECTABLE,
						MGMT_OP_SET_DISCOVERABLE,
						MGMT_OP_SET_POWERED, 0 };

static const char set_connectable_off_param[] = { 0x00 };
static const char set_connectable_off_settings_1[] = { 0x80, 0x00, 0x00, 0x00 };
static const char set_connectable_off_settings_2[] = { 0x81, 0x00, 0x00, 0x00 };
static const char set_connectable_off_scan_enable_param[] = { 0x00 };

static const struct generic_data set_connectable_off_success_test_1 = {
	.setup_settings = settings_connectable,
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_off_param,
	.send_len = sizeof(set_connectable_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_off_settings_1,
	.expect_len = sizeof(set_connectable_off_settings_1),
	.expect_settings_unset = MGMT_SETTING_CONNECTABLE,
};

static const struct generic_data set_connectable_off_success_test_2 = {
	.setup_settings = settings_powered_connectable,
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_off_param,
	.send_len = sizeof(set_connectable_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_off_settings_2,
	.expect_len = sizeof(set_connectable_off_settings_2),
	.expect_settings_unset = MGMT_SETTING_CONNECTABLE,
	.expect_hci_command = BT_HCI_CMD_WRITE_SCAN_ENABLE,
	.expect_hci_param = set_connectable_off_scan_enable_param,
	.expect_hci_len = sizeof(set_connectable_off_scan_enable_param),
};

static const struct generic_data set_connectable_off_success_test_3 = {
	.setup_settings = settings_powered_discoverable,
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_off_param,
	.send_len = sizeof(set_connectable_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_off_settings_2,
	.expect_len = sizeof(set_connectable_off_settings_2),
	.expect_settings_unset = MGMT_SETTING_CONNECTABLE,
	.expect_hci_command = BT_HCI_CMD_WRITE_SCAN_ENABLE,
	.expect_hci_param = set_connectable_off_scan_enable_param,
	.expect_hci_len = sizeof(set_connectable_off_scan_enable_param),
};

static const struct generic_data set_connectable_off_success_test_4 = {
	.setup_settings = settings_powered_discoverable,
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_off_param,
	.send_len = sizeof(set_connectable_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_off_settings_2,
	.expect_len = sizeof(set_connectable_off_settings_2),
	.expect_settings_unset = MGMT_SETTING_CONNECTABLE,
	.expect_hci_command = BT_HCI_CMD_WRITE_SCAN_ENABLE,
	.expect_hci_param = set_connectable_scan_enable_param,
	.expect_hci_len = sizeof(set_connectable_scan_enable_param),
};

static const char set_connectable_off_le_settings_1[] = { 0x00, 0x02, 0x00, 0x00 };
static const char set_connectable_off_le_settings_2[] = { 0x01, 0x06, 0x00, 0x00 };

static uint16_t settings_le_connectable[] = { MGMT_OP_SET_LE,
						MGMT_OP_SET_CONNECTABLE, 0 };

static const struct generic_data set_connectable_off_le_test_1 = {
	.setup_settings = settings_le_connectable,
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_off_param,
	.send_len = sizeof(set_connectable_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_off_le_settings_1,
	.expect_len = sizeof(set_connectable_off_le_settings_1),
	.expect_settings_unset = MGMT_SETTING_CONNECTABLE,
};

static uint16_t settings_powered_le_connectable_advertising[] = {
					MGMT_OP_SET_LE,
					MGMT_OP_SET_CONNECTABLE,
					MGMT_OP_SET_ADVERTISING,
					MGMT_OP_SET_POWERED, 0 };

static uint8_t set_connectable_off_adv_param[] = {
		0x00, 0x08,				/* min_interval */
		0x00, 0x08,				/* max_interval */
		0x03,					/* type */
		0x01,					/* own_addr_type */
		0x00,					/* direct_addr_type */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* direct_addr */
		0x07,					/* channel_map */
		0x00,					/* filter_policy */
};

static const struct generic_data set_connectable_off_le_test_2 = {
	.setup_settings = settings_powered_le_connectable_advertising,
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_off_param,
	.send_len = sizeof(set_connectable_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_off_le_settings_2,
	.expect_len = sizeof(set_connectable_off_le_settings_2),
	.expect_settings_unset = MGMT_SETTING_CONNECTABLE,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
	.expect_hci_param = set_connectable_off_adv_param,
	.expect_hci_len = sizeof(set_connectable_off_adv_param),
};

static uint16_t settings_powered_le_discoverable[] = {
					MGMT_OP_SET_LE,
					MGMT_OP_SET_CONNECTABLE,
					MGMT_OP_SET_POWERED,
					MGMT_OP_SET_DISCOVERABLE, 0 };

static uint16_t settings_powered_le_discoverable_advertising[] = {
					MGMT_OP_SET_LE,
					MGMT_OP_SET_CONNECTABLE,
					MGMT_OP_SET_ADVERTISING,
					MGMT_OP_SET_POWERED,
					MGMT_OP_SET_DISCOVERABLE, 0 };

static const struct generic_data set_connectable_off_le_test_3 = {
	.setup_settings = settings_powered_le_discoverable_advertising,
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_off_param,
	.send_len = sizeof(set_connectable_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_off_le_settings_2,
	.expect_len = sizeof(set_connectable_off_le_settings_2),
	.expect_settings_unset = MGMT_SETTING_CONNECTABLE,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
	.expect_hci_param = set_connectable_off_adv_param,
	.expect_hci_len = sizeof(set_connectable_off_adv_param),
};

static const struct generic_data set_connectable_off_le_test_4 = {
	.setup_settings = settings_powered_le_discoverable_advertising,
	.setup_limited_discov = true,
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_off_param,
	.send_len = sizeof(set_connectable_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_off_le_settings_2,
	.expect_len = sizeof(set_connectable_off_le_settings_2),
	.expect_settings_unset = MGMT_SETTING_CONNECTABLE,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
	.expect_hci_param = set_connectable_off_adv_param,
	.expect_hci_len = sizeof(set_connectable_off_adv_param),
};

static const char set_fast_conn_on_param[] = { 0x01 };
static const char set_fast_conn_on_settings_1[] = { 0x87, 0x00, 0x00, 0x00 };
static const char set_fast_conn_on_settings_2[] = { 0x85, 0x00, 0x00, 0x00 };
static const char set_fast_conn_on_settings_3[] = { 0x84, 0x00, 0x00, 0x00 };

static const struct generic_data set_fast_conn_on_success_test_1 = {
	.setup_settings = settings_powered_connectable,
	.send_opcode = MGMT_OP_SET_FAST_CONNECTABLE,
	.send_param = set_fast_conn_on_param,
	.send_len = sizeof(set_fast_conn_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_fast_conn_on_settings_1,
	.expect_len = sizeof(set_fast_conn_on_settings_1),
	.expect_settings_set = MGMT_SETTING_FAST_CONNECTABLE,
};

static const struct generic_data set_fast_conn_on_success_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_FAST_CONNECTABLE,
	.send_param = set_fast_conn_on_param,
	.send_len = sizeof(set_fast_conn_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_fast_conn_on_settings_2,
	.expect_len = sizeof(set_fast_conn_on_settings_2),
	.expect_settings_set = MGMT_SETTING_FAST_CONNECTABLE,
};

static const struct generic_data set_fast_conn_on_success_test_3 = {
	.send_opcode = MGMT_OP_SET_FAST_CONNECTABLE,
	.send_param = set_fast_conn_on_param,
	.send_len = sizeof(set_fast_conn_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_fast_conn_on_settings_3,
	.expect_len = sizeof(set_fast_conn_on_settings_3),
	.expect_settings_set = MGMT_SETTING_FAST_CONNECTABLE,
};

static const struct generic_data set_fast_conn_on_not_supported_test_1 = {
	.setup_settings = settings_powered_connectable,
	.send_opcode = MGMT_OP_SET_FAST_CONNECTABLE,
	.send_param = set_fast_conn_on_param,
	.send_len = sizeof(set_fast_conn_on_param),
	.expect_status = MGMT_STATUS_NOT_SUPPORTED,
};

static const char set_fast_conn_nval_param[] = { 0xff };

static const struct generic_data set_fast_conn_nval_param_test_1 = {
	.send_opcode = MGMT_OP_SET_FAST_CONNECTABLE,
	.send_param = set_fast_conn_nval_param,
	.send_len = sizeof(set_fast_conn_nval_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const char set_bondable_on_param[] = { 0x01 };
static const char set_bondable_invalid_param[] = { 0x02 };
static const char set_bondable_garbage_param[] = { 0x01, 0x00 };
static const char set_bondable_settings_param[] = { 0x90, 0x00, 0x00, 0x00 };

static const struct generic_data set_bondable_on_success_test = {
	.send_opcode = MGMT_OP_SET_BONDABLE,
	.send_param = set_bondable_on_param,
	.send_len = sizeof(set_bondable_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_bondable_settings_param,
	.expect_len = sizeof(set_bondable_settings_param),
	.expect_settings_set = MGMT_SETTING_BONDABLE,
};

static const struct generic_data set_bondable_on_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_SET_BONDABLE,
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_bondable_on_invalid_param_test_2 = {
	.send_opcode = MGMT_OP_SET_BONDABLE,
	.send_param = set_bondable_invalid_param,
	.send_len = sizeof(set_bondable_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_bondable_on_invalid_param_test_3 = {
	.send_opcode = MGMT_OP_SET_BONDABLE,
	.send_param = set_bondable_garbage_param,
	.send_len = sizeof(set_bondable_garbage_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_bondable_on_invalid_index_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_SET_BONDABLE,
	.send_param = set_bondable_on_param,
	.send_len = sizeof(set_bondable_on_param),
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static const uint8_t set_discoverable_on_param[] = { 0x01, 0x00, 0x00 };
static const uint8_t set_discoverable_timeout_param[] = { 0x01, 0x0a, 0x00 };
static const uint8_t set_discoverable_invalid_param[] = { 0x02, 0x00, 0x00 };
static const uint8_t set_discoverable_off_param[] = { 0x00, 0x00, 0x00 };
static const uint8_t set_discoverable_offtimeout_param[] = { 0x00, 0x01, 0x00 };
static const uint8_t set_discoverable_garbage_param[] = { 0x01, 0x00, 0x00, 0x00 };
static const uint8_t set_discoverable_on_settings_param_1[] = { 0x8a, 0x00, 0x00, 0x00 };
static const uint8_t set_discoverable_on_settings_param_2[] = { 0x8b, 0x00, 0x00, 0x00 };
static const uint8_t set_discoverable_off_settings_param_1[] = { 0x82, 0x00, 0x00, 0x00 };
static const uint8_t set_discoverable_off_settings_param_2[] = { 0x83, 0x00, 0x00, 0x00 };
static const uint8_t set_discoverable_on_scan_enable_param[] = { 0x03 };
static const uint8_t set_discoverable_off_scan_enable_param[] = { 0x02 };

static const struct generic_data set_discoverable_on_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_discoverable_on_invalid_param_test_2 = {
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_discoverable_invalid_param,
	.send_len = sizeof(set_discoverable_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_discoverable_on_invalid_param_test_3 = {
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_discoverable_garbage_param,
	.send_len = sizeof(set_discoverable_garbage_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_discoverable_on_invalid_param_test_4 = {
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_discoverable_offtimeout_param,
	.send_len = sizeof(set_discoverable_offtimeout_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_discoverable_on_not_powered_test_1 = {
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_discoverable_timeout_param,
	.send_len = sizeof(set_discoverable_timeout_param),
	.expect_status = MGMT_STATUS_NOT_POWERED,
};

static const struct generic_data set_discoverable_on_not_powered_test_2 = {
	.setup_settings = settings_connectable,
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_discoverable_timeout_param,
	.send_len = sizeof(set_discoverable_timeout_param),
	.expect_status = MGMT_STATUS_NOT_POWERED,
};

static const struct generic_data set_discoverable_on_rejected_test_1 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_discoverable_on_param,
	.send_len = sizeof(set_discoverable_on_param),
	.expect_status = MGMT_STATUS_REJECTED,
};

static const struct generic_data set_discoverable_on_rejected_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_discoverable_on_param,
	.send_len = sizeof(set_discoverable_on_param),
	.expect_status = MGMT_STATUS_REJECTED,
};

static const struct generic_data set_discoverable_on_rejected_test_3 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_discoverable_timeout_param,
	.send_len = sizeof(set_discoverable_timeout_param),
	.expect_status = MGMT_STATUS_REJECTED,
};

static const struct generic_data set_discoverable_on_success_test_1 = {
	.setup_settings = settings_connectable,
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_discoverable_on_param,
	.send_len = sizeof(set_discoverable_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_discoverable_on_settings_param_1,
	.expect_len = sizeof(set_discoverable_on_settings_param_1),
	.expect_settings_set = MGMT_SETTING_DISCOVERABLE,
};

static const struct generic_data set_discoverable_on_success_test_2 = {
	.setup_settings = settings_powered_connectable,
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_discoverable_on_param,
	.send_len = sizeof(set_discoverable_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_discoverable_on_settings_param_2,
	.expect_len = sizeof(set_discoverable_on_settings_param_2),
	.expect_settings_set = MGMT_SETTING_DISCOVERABLE,
	.expect_hci_command = BT_HCI_CMD_WRITE_SCAN_ENABLE,
	.expect_hci_param = set_discoverable_on_scan_enable_param,
	.expect_hci_len = sizeof(set_discoverable_on_scan_enable_param),
};

static uint8_t set_discov_on_le_param[] = { 0x0b, 0x06, 0x00, 0x00 };
static uint8_t set_discov_adv_data[32] = { 0x06, 0x02, 0x01, 0x06,
								0x02, 0x0a, };

static const struct generic_data set_discov_on_le_success_1 = {
	.setup_settings = settings_powered_le_connectable_advertising,
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_discoverable_on_param,
	.send_len = sizeof(set_discoverable_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_discov_on_le_param,
	.expect_len = sizeof(set_discov_on_le_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_discov_adv_data,
	.expect_hci_len = sizeof(set_discov_adv_data),
};

static const struct generic_data set_discoverable_off_success_test_1 = {
	.setup_settings = settings_connectable,
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_discoverable_off_param,
	.send_len = sizeof(set_discoverable_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_discoverable_off_settings_param_1,
	.expect_len = sizeof(set_discoverable_off_settings_param_1),
};

static const struct generic_data set_discoverable_off_success_test_2 = {
	.setup_settings = settings_powered_discoverable,
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_discoverable_off_param,
	.send_len = sizeof(set_discoverable_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_discoverable_off_settings_param_2,
	.expect_len = sizeof(set_discoverable_off_settings_param_2),
	.expect_hci_command = BT_HCI_CMD_WRITE_SCAN_ENABLE,
	.expect_hci_param = set_discoverable_off_scan_enable_param,
	.expect_hci_len = sizeof(set_discoverable_off_scan_enable_param),
};

static const uint8_t set_limited_discov_on_param[] = { 0x02, 0x01, 0x00 };

static const struct generic_data set_limited_discov_on_success_1 = {
	.setup_settings = settings_powered_connectable,
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_limited_discov_on_param,
	.send_len = sizeof(set_limited_discov_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_discoverable_on_settings_param_2,
	.expect_len = sizeof(set_discoverable_on_settings_param_2),
	.expect_hci_command = BT_HCI_CMD_WRITE_SCAN_ENABLE,
	.expect_hci_param = set_discoverable_on_scan_enable_param,
	.expect_hci_len = sizeof(set_discoverable_on_scan_enable_param),
};

static uint8_t write_current_iac_lap_limited[] = { 0x01, 0x00, 0x8b, 0x9e };

static const struct generic_data set_limited_discov_on_success_2 = {
	.setup_settings = settings_powered_connectable,
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_limited_discov_on_param,
	.send_len = sizeof(set_limited_discov_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_discoverable_on_settings_param_2,
	.expect_len = sizeof(set_discoverable_on_settings_param_2),
	.expect_hci_command = BT_HCI_CMD_WRITE_CURRENT_IAC_LAP,
	.expect_hci_param = write_current_iac_lap_limited,
	.expect_hci_len = sizeof(write_current_iac_lap_limited),
};

static uint8_t write_cod_limited[] = { 0x00, 0x20, 0x00 };

static const struct generic_data set_limited_discov_on_success_3 = {
	.setup_settings = settings_powered_connectable,
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_limited_discov_on_param,
	.send_len = sizeof(set_limited_discov_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_discoverable_on_settings_param_2,
	.expect_len = sizeof(set_discoverable_on_settings_param_2),
	.expect_hci_command = BT_HCI_CMD_WRITE_CLASS_OF_DEV,
	.expect_hci_param = write_cod_limited,
	.expect_hci_len = sizeof(write_cod_limited),
};

static uint8_t set_limited_discov_adv_data[32] = { 0x06, 0x02, 0x01, 0x05,
								0x02, 0x0a, };

static const struct generic_data set_limited_discov_on_le_success_1 = {
	.setup_settings = settings_powered_le_connectable_advertising,
	.send_opcode = MGMT_OP_SET_DISCOVERABLE,
	.send_param = set_limited_discov_on_param,
	.send_len = sizeof(set_limited_discov_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_discov_on_le_param,
	.expect_len = sizeof(set_discov_on_le_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_limited_discov_adv_data,
	.expect_hci_len = sizeof(set_limited_discov_adv_data),
};

static uint16_t settings_link_sec[] = { MGMT_OP_SET_LINK_SECURITY, 0 };

static const char set_link_sec_on_param[] = { 0x01 };
static const char set_link_sec_invalid_param[] = { 0x02 };
static const char set_link_sec_garbage_param[] = { 0x01, 0x00 };
static const char set_link_sec_settings_param_1[] = { 0xa0, 0x00, 0x00, 0x00 };
static const char set_link_sec_settings_param_2[] = { 0xa1, 0x00, 0x00, 0x00 };
static const char set_link_sec_auth_enable_param[] = { 0x01 };

static const struct generic_data set_link_sec_on_success_test_1 = {
	.send_opcode = MGMT_OP_SET_LINK_SECURITY,
	.send_param = set_link_sec_on_param,
	.send_len = sizeof(set_link_sec_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_link_sec_settings_param_1,
	.expect_len = sizeof(set_link_sec_settings_param_1),
	.expect_settings_set = MGMT_SETTING_LINK_SECURITY,
};

static const struct generic_data set_link_sec_on_success_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_LINK_SECURITY,
	.send_param = set_link_sec_on_param,
	.send_len = sizeof(set_link_sec_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_link_sec_settings_param_2,
	.expect_len = sizeof(set_link_sec_settings_param_2),
	.expect_settings_set = MGMT_SETTING_LINK_SECURITY,
	.expect_hci_command = BT_HCI_CMD_WRITE_AUTH_ENABLE,
	.expect_hci_param = set_link_sec_auth_enable_param,
	.expect_hci_len = sizeof(set_link_sec_auth_enable_param),
};

static const struct generic_data set_link_sec_on_success_test_3 = {
	.setup_settings = settings_link_sec,
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_on_param,
	.send_len = sizeof(set_powered_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_link_sec_settings_param_2,
	.expect_len = sizeof(set_link_sec_settings_param_2),
	.expect_settings_set = MGMT_SETTING_LINK_SECURITY,
	.expect_hci_command = BT_HCI_CMD_WRITE_AUTH_ENABLE,
	.expect_hci_param = set_link_sec_auth_enable_param,
	.expect_hci_len = sizeof(set_link_sec_auth_enable_param),
};

static const struct generic_data set_link_sec_on_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_SET_LINK_SECURITY,
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_link_sec_on_invalid_param_test_2 = {
	.send_opcode = MGMT_OP_SET_LINK_SECURITY,
	.send_param = set_link_sec_invalid_param,
	.send_len = sizeof(set_link_sec_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_link_sec_on_invalid_param_test_3 = {
	.send_opcode = MGMT_OP_SET_LINK_SECURITY,
	.send_param = set_link_sec_garbage_param,
	.send_len = sizeof(set_link_sec_garbage_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_link_sec_on_invalid_index_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_SET_LINK_SECURITY,
	.send_param = set_link_sec_on_param,
	.send_len = sizeof(set_link_sec_on_param),
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static const uint16_t settings_powered_link_sec[] = {
						MGMT_OP_SET_LINK_SECURITY,
						MGMT_OP_SET_POWERED, 0 };

static const char set_link_sec_off_param[] = { 0x00 };
static const char set_link_sec_off_settings_1[] = { 0x80, 0x00, 0x00, 0x00 };
static const char set_link_sec_off_settings_2[] = { 0x81, 0x00, 0x00, 0x00 };
static const char set_link_sec_off_auth_enable_param[] = { 0x00 };

static const struct generic_data set_link_sec_off_success_test_1 = {
	.setup_settings = settings_link_sec,
	.send_opcode = MGMT_OP_SET_LINK_SECURITY,
	.send_param = set_link_sec_off_param,
	.send_len = sizeof(set_link_sec_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_link_sec_off_settings_1,
	.expect_len = sizeof(set_link_sec_off_settings_1),
	.expect_settings_unset = MGMT_SETTING_LINK_SECURITY,
};

static const struct generic_data set_link_sec_off_success_test_2 = {
	.setup_settings = settings_powered_link_sec,
	.send_opcode = MGMT_OP_SET_LINK_SECURITY,
	.send_param = set_link_sec_off_param,
	.send_len = sizeof(set_link_sec_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_link_sec_off_settings_2,
	.expect_len = sizeof(set_link_sec_off_settings_2),
	.expect_settings_unset = MGMT_SETTING_LINK_SECURITY,
	.expect_hci_command = BT_HCI_CMD_WRITE_AUTH_ENABLE,
	.expect_hci_param = set_link_sec_off_auth_enable_param,
	.expect_hci_len = sizeof(set_link_sec_off_auth_enable_param),
};

static uint16_t settings_ssp[] = { MGMT_OP_SET_SSP, 0 };

static const char set_ssp_on_param[] = { 0x01 };
static const char set_ssp_invalid_param[] = { 0x02 };
static const char set_ssp_garbage_param[] = { 0x01, 0x00 };
static const char set_ssp_settings_param_1[] = { 0xc0, 0x00, 0x00, 0x00 };
static const char set_ssp_settings_param_2[] = { 0xc1, 0x00, 0x00, 0x00 };
static const char set_ssp_on_write_ssp_mode_param[] = { 0x01 };

static const struct generic_data set_ssp_on_success_test_1 = {
	.send_opcode = MGMT_OP_SET_SSP,
	.send_param = set_ssp_on_param,
	.send_len = sizeof(set_ssp_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_ssp_settings_param_1,
	.expect_len = sizeof(set_ssp_settings_param_1),
	.expect_settings_set = MGMT_SETTING_SSP,
};

static const struct generic_data set_ssp_on_success_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_SSP,
	.send_param = set_ssp_on_param,
	.send_len = sizeof(set_ssp_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_ssp_settings_param_2,
	.expect_len = sizeof(set_ssp_settings_param_2),
	.expect_settings_set = MGMT_SETTING_SSP,
	.expect_hci_command = BT_HCI_CMD_WRITE_SIMPLE_PAIRING_MODE,
	.expect_hci_param = set_ssp_on_write_ssp_mode_param,
	.expect_hci_len = sizeof(set_ssp_on_write_ssp_mode_param),
};

static const struct generic_data set_ssp_on_success_test_3 = {
	.setup_settings = settings_ssp,
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_on_param,
	.send_len = sizeof(set_powered_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_ssp_settings_param_2,
	.expect_len = sizeof(set_ssp_settings_param_2),
	.expect_settings_set = MGMT_SETTING_SSP,
	.expect_hci_command = BT_HCI_CMD_WRITE_SIMPLE_PAIRING_MODE,
	.expect_hci_param = set_ssp_on_write_ssp_mode_param,
	.expect_hci_len = sizeof(set_ssp_on_write_ssp_mode_param),
};

static const struct generic_data set_ssp_on_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_SET_SSP,
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_ssp_on_invalid_param_test_2 = {
	.send_opcode = MGMT_OP_SET_SSP,
	.send_param = set_ssp_invalid_param,
	.send_len = sizeof(set_ssp_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_ssp_on_invalid_param_test_3 = {
	.send_opcode = MGMT_OP_SET_SSP,
	.send_param = set_ssp_garbage_param,
	.send_len = sizeof(set_ssp_garbage_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_ssp_on_invalid_index_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_SET_SSP,
	.send_param = set_ssp_on_param,
	.send_len = sizeof(set_ssp_on_param),
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static uint16_t settings_powered_ssp[] = { MGMT_OP_SET_SSP,
						MGMT_OP_SET_POWERED, 0 };

static uint16_t settings_powered_sc[] = { MGMT_OP_SET_SSP,
						MGMT_OP_SET_SECURE_CONN,
						MGMT_OP_SET_POWERED, 0 };

static const char set_sc_on_param[] = { 0x01 };
static const char set_sc_only_on_param[] = { 0x02 };
static const char set_sc_invalid_param[] = { 0x03 };
static const char set_sc_garbage_param[] = { 0x01, 0x00 };
static const char set_sc_settings_param_1[] = { 0xc0, 0x08, 0x00, 0x00 };
static const char set_sc_settings_param_2[] = { 0xc1, 0x08, 0x00, 0x00 };
static const char set_sc_on_write_sc_support_param[] = { 0x01 };

static const struct generic_data set_sc_on_success_test_1 = {
	.setup_settings = settings_ssp,
	.send_opcode = MGMT_OP_SET_SECURE_CONN,
	.send_param = set_sc_on_param,
	.send_len = sizeof(set_sc_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_sc_settings_param_1,
	.expect_len = sizeof(set_sc_settings_param_1),
	.expect_settings_set = MGMT_SETTING_SECURE_CONN,
};

static const struct generic_data set_sc_on_success_test_2 = {
	.setup_settings = settings_powered_ssp,
	.send_opcode = MGMT_OP_SET_SECURE_CONN,
	.send_param = set_sc_on_param,
	.send_len = sizeof(set_sc_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_sc_settings_param_2,
	.expect_len = sizeof(set_sc_settings_param_2),
	.expect_settings_set = MGMT_SETTING_SECURE_CONN,
	.expect_hci_command = BT_HCI_CMD_WRITE_SECURE_CONN_SUPPORT,
	.expect_hci_param = set_sc_on_write_sc_support_param,
	.expect_hci_len = sizeof(set_sc_on_write_sc_support_param),
};

static const struct generic_data set_sc_on_invalid_param_test_1 = {
	.setup_settings = settings_ssp,
	.send_opcode = MGMT_OP_SET_SECURE_CONN,
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_sc_on_invalid_param_test_2 = {
	.setup_settings = settings_ssp,
	.send_opcode = MGMT_OP_SET_SECURE_CONN,
	.send_param = set_sc_invalid_param,
	.send_len = sizeof(set_sc_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_sc_on_invalid_param_test_3 = {
	.setup_settings = settings_ssp,
	.send_opcode = MGMT_OP_SET_SECURE_CONN,
	.send_param = set_sc_garbage_param,
	.send_len = sizeof(set_sc_garbage_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_sc_on_invalid_index_test = {
	.setup_settings = settings_ssp,
	.send_index_none = true,
	.send_opcode = MGMT_OP_SET_SECURE_CONN,
	.send_param = set_sc_on_param,
	.send_len = sizeof(set_sc_on_param),
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static const struct generic_data set_sc_on_not_supported_test_1 = {
	.setup_settings = settings_ssp,
	.send_opcode = MGMT_OP_SET_SECURE_CONN,
	.send_param = set_sc_on_param,
	.send_len = sizeof(set_sc_on_param),
	.expect_status = MGMT_STATUS_NOT_SUPPORTED,
};

static const struct generic_data set_sc_on_not_supported_test_2 = {
	.setup_settings = settings_powered_ssp,
	.send_opcode = MGMT_OP_SET_SECURE_CONN,
	.send_param = set_sc_on_param,
	.send_len = sizeof(set_sc_on_param),
	.expect_status = MGMT_STATUS_NOT_SUPPORTED,
};

static const struct generic_data set_sc_only_on_success_test_1 = {
	.setup_settings = settings_ssp,
	.send_opcode = MGMT_OP_SET_SECURE_CONN,
	.send_param = set_sc_only_on_param,
	.send_len = sizeof(set_sc_only_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_sc_settings_param_1,
	.expect_len = sizeof(set_sc_settings_param_1),
	.expect_settings_set = MGMT_SETTING_SECURE_CONN,
};

static const struct generic_data set_sc_only_on_success_test_2 = {
	.setup_settings = settings_powered_ssp,
	.send_opcode = MGMT_OP_SET_SECURE_CONN,
	.send_param = set_sc_only_on_param,
	.send_len = sizeof(set_sc_only_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_sc_settings_param_2,
	.expect_len = sizeof(set_sc_settings_param_2),
	.expect_settings_set = MGMT_SETTING_SECURE_CONN,
	.expect_hci_command = BT_HCI_CMD_WRITE_SECURE_CONN_SUPPORT,
	.expect_hci_param = set_sc_on_write_sc_support_param,
	.expect_hci_len = sizeof(set_sc_on_write_sc_support_param),
};

static const char set_hs_on_param[] = { 0x01 };
static const char set_hs_invalid_param[] = { 0x02 };
static const char set_hs_garbage_param[] = { 0x01, 0x00 };
static const char set_hs_settings_param_1[] = { 0xc0, 0x01, 0x00, 0x00 };

static const struct generic_data set_hs_on_success_test = {
	.setup_settings = settings_ssp,
	.send_opcode = MGMT_OP_SET_HS,
	.send_param = set_hs_on_param,
	.send_len = sizeof(set_hs_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_hs_settings_param_1,
	.expect_len = sizeof(set_hs_settings_param_1),
	.expect_settings_set = MGMT_SETTING_HS,
};

static const struct generic_data set_hs_on_invalid_param_test_1 = {
	.setup_settings = settings_ssp,
	.send_opcode = MGMT_OP_SET_HS,
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_hs_on_invalid_param_test_2 = {
	.setup_settings = settings_ssp,
	.send_opcode = MGMT_OP_SET_HS,
	.send_param = set_hs_invalid_param,
	.send_len = sizeof(set_hs_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_hs_on_invalid_param_test_3 = {
	.setup_settings = settings_ssp,
	.send_opcode = MGMT_OP_SET_HS,
	.send_param = set_hs_garbage_param,
	.send_len = sizeof(set_hs_garbage_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_hs_on_invalid_index_test = {
	.setup_settings = settings_ssp,
	.send_index_none = true,
	.send_opcode = MGMT_OP_SET_HS,
	.send_param = set_hs_on_param,
	.send_len = sizeof(set_hs_on_param),
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static uint16_t settings_le[] = { MGMT_OP_SET_LE, 0 };

static const char set_le_on_param[] = { 0x01 };
static const char set_le_off_param[] = { 0x00 };
static const char set_le_invalid_param[] = { 0x02 };
static const char set_le_garbage_param[] = { 0x01, 0x00 };
static const char set_le_settings_param_1[] = { 0x80, 0x02, 0x00, 0x00 };
static const char set_le_settings_param_2[] = { 0x81, 0x02, 0x00, 0x00 };
static const char set_le_on_write_le_host_param[] = { 0x01, 0x00 };

static const struct generic_data set_le_on_success_test_1 = {
	.send_opcode = MGMT_OP_SET_LE,
	.send_param = set_le_on_param,
	.send_len = sizeof(set_le_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_le_settings_param_1,
	.expect_len = sizeof(set_le_settings_param_1),
	.expect_settings_set = MGMT_SETTING_LE,
};

static const struct generic_data set_le_on_success_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_LE,
	.send_param = set_le_on_param,
	.send_len = sizeof(set_le_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_le_settings_param_2,
	.expect_len = sizeof(set_le_settings_param_2),
	.expect_settings_set = MGMT_SETTING_LE,
	.expect_hci_command = BT_HCI_CMD_WRITE_LE_HOST_SUPPORTED,
	.expect_hci_param = set_le_on_write_le_host_param,
	.expect_hci_len = sizeof(set_le_on_write_le_host_param),
};

static const struct generic_data set_le_on_success_test_3 = {
	.setup_settings = settings_le,
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_on_param,
	.send_len = sizeof(set_powered_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_le_settings_param_2,
	.expect_len = sizeof(set_le_settings_param_2),
	.expect_settings_set = MGMT_SETTING_LE,
	.expect_hci_command = BT_HCI_CMD_WRITE_LE_HOST_SUPPORTED,
	.expect_hci_param = set_le_on_write_le_host_param,
	.expect_hci_len = sizeof(set_le_on_write_le_host_param),
};

static const struct generic_data set_le_on_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_SET_LE,
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_le_on_invalid_param_test_2 = {
	.send_opcode = MGMT_OP_SET_LE,
	.send_param = set_le_invalid_param,
	.send_len = sizeof(set_le_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_le_on_invalid_param_test_3 = {
	.send_opcode = MGMT_OP_SET_LE,
	.send_param = set_le_garbage_param,
	.send_len = sizeof(set_le_garbage_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data set_le_on_invalid_index_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_SET_LE,
	.send_param = set_le_on_param,
	.send_len = sizeof(set_le_on_param),
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static uint16_t settings_powered_le[] = { MGMT_OP_SET_LE,
					MGMT_OP_SET_POWERED, 0 };

static const char set_adv_on_param[] = { 0x01 };
static const char set_adv_settings_param_1[] = { 0x80, 0x06, 0x00, 0x00 };
static const char set_adv_settings_param_2[] = { 0x81, 0x06, 0x00, 0x00 };
static const char set_adv_on_set_adv_enable_param[] = { 0x01 };
static const char set_adv_on_set_adv_disable_param[] = { 0x00 };

static const struct generic_data set_adv_on_success_test_1 = {
	.setup_settings = settings_le,
	.send_opcode = MGMT_OP_SET_ADVERTISING,
	.send_param = set_adv_on_param,
	.send_len = sizeof(set_adv_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_adv_settings_param_1,
	.expect_len = sizeof(set_adv_settings_param_1),
	.expect_settings_set = MGMT_SETTING_ADVERTISING,
};

static const struct generic_data set_adv_on_success_test_2 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_SET_ADVERTISING,
	.send_param = set_adv_on_param,
	.send_len = sizeof(set_adv_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_adv_settings_param_2,
	.expect_len = sizeof(set_adv_settings_param_2),
	.expect_settings_set = MGMT_SETTING_ADVERTISING,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_ENABLE,
	.expect_hci_param = set_adv_on_set_adv_enable_param,
	.expect_hci_len = sizeof(set_adv_on_set_adv_enable_param),
};

static const struct generic_data set_adv_on_rejected_test_1 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_ADVERTISING,
	.send_param = set_adv_on_param,
	.send_len = sizeof(set_adv_on_param),
	.expect_status = MGMT_STATUS_REJECTED,
};

static const char set_bredr_off_param[] = { 0x00 };
static const char set_bredr_on_param[] = { 0x01 };
static const char set_bredr_invalid_param[] = { 0x02 };
static const char set_bredr_settings_param_1[] = { 0x00, 0x02, 0x00, 0x00 };
static const char set_bredr_settings_param_2[] = { 0x80, 0x02, 0x00, 0x00 };
static const char set_bredr_settings_param_3[] = { 0x81, 0x02, 0x00, 0x00 };

static const struct generic_data set_bredr_off_success_test_1 = {
	.setup_settings = settings_le,
	.send_opcode = MGMT_OP_SET_BREDR,
	.send_param = set_bredr_off_param,
	.send_len = sizeof(set_bredr_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_bredr_settings_param_1,
	.expect_len = sizeof(set_bredr_settings_param_1),
	.expect_settings_unset = MGMT_SETTING_BREDR,
};

static const struct generic_data set_bredr_on_success_test_1 = {
	.setup_settings = settings_le,
	.setup_nobredr = true,
	.send_opcode = MGMT_OP_SET_BREDR,
	.send_param = set_bredr_on_param,
	.send_len = sizeof(set_bredr_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_bredr_settings_param_2,
	.expect_len = sizeof(set_bredr_settings_param_2),
	.expect_settings_set = MGMT_SETTING_BREDR,
};

static const struct generic_data set_bredr_on_success_test_2 = {
	.setup_settings = settings_powered_le,
	.setup_nobredr = true,
	.send_opcode = MGMT_OP_SET_BREDR,
	.send_param = set_bredr_on_param,
	.send_len = sizeof(set_bredr_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_bredr_settings_param_3,
	.expect_len = sizeof(set_bredr_settings_param_3),
	.expect_settings_set = MGMT_SETTING_BREDR,
};

static const struct generic_data set_bredr_off_notsupp_test = {
	.send_opcode = MGMT_OP_SET_BREDR,
	.send_param = set_bredr_off_param,
	.send_len = sizeof(set_bredr_off_param),
	.expect_status = MGMT_STATUS_NOT_SUPPORTED,
};

static const struct generic_data set_bredr_off_failure_test_1 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_SET_BREDR,
	.send_param = set_bredr_off_param,
	.send_len = sizeof(set_bredr_off_param),
	.expect_status = MGMT_STATUS_REJECTED,
};

static const struct generic_data set_bredr_off_failure_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_BREDR,
	.send_param = set_bredr_off_param,
	.send_len = sizeof(set_bredr_off_param),
	.expect_status = MGMT_STATUS_REJECTED,
};

static const struct generic_data set_bredr_off_failure_test_3 = {
	.setup_settings = settings_le,
	.send_opcode = MGMT_OP_SET_BREDR,
	.send_param = set_bredr_invalid_param,
	.send_len = sizeof(set_bredr_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const char set_local_name_param[260] = { 'T', 'e', 's', 't', ' ',
						'n', 'a', 'm', 'e' };
static const char write_local_name_hci[248] = { 'T', 'e', 's', 't', ' ',
						'n', 'a', 'm', 'e' };
static const char write_eir_local_name_hci_1[241] = { 0x00,
		0x0a, 0x09, 'T', 'e', 's', 't', ' ', 'n', 'a', 'm', 'e',
		0x02, 0x0a, 0x00, };

static const struct generic_data set_local_name_test_1 = {
	.send_opcode = MGMT_OP_SET_LOCAL_NAME,
	.send_param = set_local_name_param,
	.send_len = sizeof(set_local_name_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_local_name_param,
	.expect_len = sizeof(set_local_name_param),
	.expect_alt_ev = MGMT_EV_LOCAL_NAME_CHANGED,
	.expect_alt_ev_param = set_local_name_param,
	.expect_alt_ev_len = sizeof(set_local_name_param),
};

static const struct generic_data set_local_name_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_LOCAL_NAME,
	.send_param = set_local_name_param,
	.send_len = sizeof(set_local_name_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_local_name_param,
	.expect_len = sizeof(set_local_name_param),
	.expect_hci_command = BT_HCI_CMD_WRITE_LOCAL_NAME,
	.expect_hci_param = write_local_name_hci,
	.expect_hci_len = sizeof(write_local_name_hci),
	.expect_alt_ev = MGMT_EV_LOCAL_NAME_CHANGED,
	.expect_alt_ev_param = set_local_name_param,
	.expect_alt_ev_len = sizeof(set_local_name_param),
};

static const struct generic_data set_local_name_test_3 = {
	.setup_settings = settings_powered_ssp,
	.send_opcode = MGMT_OP_SET_LOCAL_NAME,
	.send_param = set_local_name_param,
	.send_len = sizeof(set_local_name_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_local_name_param,
	.expect_len = sizeof(set_local_name_param),
	.expect_hci_command = BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE,
	.expect_hci_param = write_eir_local_name_hci_1,
	.expect_hci_len = sizeof(write_eir_local_name_hci_1),
	.expect_alt_ev = MGMT_EV_LOCAL_NAME_CHANGED,
	.expect_alt_ev_param = set_local_name_param,
	.expect_alt_ev_len = sizeof(set_local_name_param),
};

static const char start_discovery_invalid_param[] = { 0x00 };
static const char start_discovery_bredr_param[] = { 0x01 };
static const char start_discovery_le_param[] = { 0x06 };
static const char start_discovery_bredrle_param[] = { 0x07 };
static const char start_discovery_valid_hci[] = { 0x01, 0x01 };
static const char start_discovery_evt[] = { 0x07, 0x01 };
static const char start_discovery_le_evt[] = { 0x06, 0x01 };

static const struct generic_data start_discovery_not_powered_test_1 = {
	.send_opcode = MGMT_OP_START_DISCOVERY,
	.send_param = start_discovery_bredr_param,
	.send_len = sizeof(start_discovery_bredr_param),
	.expect_status = MGMT_STATUS_NOT_POWERED,
	.expect_param = start_discovery_bredr_param,
	.expect_len = sizeof(start_discovery_bredr_param),
};

static const struct generic_data start_discovery_invalid_param_test_1 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_START_DISCOVERY,
	.send_param = start_discovery_invalid_param,
	.send_len = sizeof(start_discovery_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
	.expect_param = start_discovery_invalid_param,
	.expect_len = sizeof(start_discovery_invalid_param),
};

static const struct generic_data start_discovery_not_supported_test_1 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_START_DISCOVERY,
	.send_param = start_discovery_le_param,
	.send_len = sizeof(start_discovery_le_param),
	.expect_status = MGMT_STATUS_REJECTED,
	.expect_param = start_discovery_le_param,
	.expect_len = sizeof(start_discovery_le_param),
};

static const struct generic_data start_discovery_valid_param_test_1 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_START_DISCOVERY,
	.send_param = start_discovery_bredrle_param,
	.send_len = sizeof(start_discovery_bredrle_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = start_discovery_bredrle_param,
	.expect_len = sizeof(start_discovery_bredrle_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_SCAN_ENABLE,
	.expect_hci_param = start_discovery_valid_hci,
	.expect_hci_len = sizeof(start_discovery_valid_hci),
	.expect_alt_ev = MGMT_EV_DISCOVERING,
	.expect_alt_ev_param = start_discovery_evt,
	.expect_alt_ev_len = sizeof(start_discovery_evt),
};

static const struct generic_data start_discovery_valid_param_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_START_DISCOVERY,
	.send_param = start_discovery_le_param,
	.send_len = sizeof(start_discovery_le_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = start_discovery_le_param,
	.expect_len = sizeof(start_discovery_le_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_SCAN_ENABLE,
	.expect_hci_param = start_discovery_valid_hci,
	.expect_hci_len = sizeof(start_discovery_valid_hci),
	.expect_alt_ev = MGMT_EV_DISCOVERING,
	.expect_alt_ev_param = start_discovery_le_evt,
	.expect_alt_ev_len = sizeof(start_discovery_le_evt),
};

static const struct generic_data start_discovery_valid_param_power_off_1 = {
	.setup_settings = settings_le,
	.send_opcode = MGMT_OP_START_DISCOVERY,
	.send_param = start_discovery_bredrle_param,
	.send_len = sizeof(start_discovery_bredrle_param),
	.expect_status = MGMT_STATUS_NOT_POWERED,
	.force_power_off = true,
	.expect_param = start_discovery_bredrle_param,
	.expect_len = sizeof(start_discovery_bredrle_param),
};

static const char stop_discovery_bredrle_param[] = { 0x07 };
static const char stop_discovery_bredrle_invalid_param[] = { 0x06 };
static const char stop_discovery_valid_hci[] = { 0x00, 0x00 };
static const char stop_discovery_evt[] = { 0x07, 0x00 };
static const char stop_discovery_bredr_param[] = { 0x01 };
static const char stop_discovery_bredr_discovering[] = { 0x01, 0x00 };

static const struct generic_data stop_discovery_success_test_1 = {
	.setup_settings = settings_powered_le,
	.setup_send_opcode = MGMT_OP_START_DISCOVERY,
	.setup_send_param = start_discovery_bredrle_param,
	.setup_send_len = sizeof(start_discovery_bredrle_param),
	.send_opcode = MGMT_OP_STOP_DISCOVERY,
	.send_param = stop_discovery_bredrle_param,
	.send_len = sizeof(stop_discovery_bredrle_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = stop_discovery_bredrle_param,
	.expect_len = sizeof(stop_discovery_bredrle_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_SCAN_ENABLE,
	.expect_hci_param = stop_discovery_valid_hci,
	.expect_hci_len = sizeof(stop_discovery_valid_hci),
	.expect_alt_ev = MGMT_EV_DISCOVERING,
	.expect_alt_ev_param = stop_discovery_evt,
	.expect_alt_ev_len = sizeof(stop_discovery_evt),
};

static const struct generic_data stop_discovery_bredr_success_test_1 = {
	.setup_settings = settings_powered,
	.setup_send_opcode = MGMT_OP_START_DISCOVERY,
	.setup_send_param = start_discovery_bredr_param,
	.setup_send_len = sizeof(start_discovery_bredr_param),
	.send_opcode = MGMT_OP_STOP_DISCOVERY,
	.send_param = stop_discovery_bredr_param,
	.send_len = sizeof(stop_discovery_bredr_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = stop_discovery_bredr_param,
	.expect_len = sizeof(stop_discovery_bredr_param),
	.expect_hci_command = BT_HCI_CMD_INQUIRY_CANCEL,
	.expect_alt_ev = MGMT_EV_DISCOVERING,
	.expect_alt_ev_param = stop_discovery_bredr_discovering,
	.expect_alt_ev_len = sizeof(stop_discovery_bredr_discovering),
};

static const struct generic_data stop_discovery_rejected_test_1 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_STOP_DISCOVERY,
	.send_param = stop_discovery_bredrle_param,
	.send_len = sizeof(stop_discovery_bredrle_param),
	.expect_status = MGMT_STATUS_REJECTED,
	.expect_param = stop_discovery_bredrle_param,
	.expect_len = sizeof(stop_discovery_bredrle_param),
};

static const struct generic_data stop_discovery_invalid_param_test_1 = {
	.setup_settings = settings_powered_le,
	.setup_send_opcode = MGMT_OP_START_DISCOVERY,
	.setup_send_param = start_discovery_bredrle_param,
	.setup_send_len = sizeof(start_discovery_bredrle_param),
	.send_opcode = MGMT_OP_STOP_DISCOVERY,
	.send_param = stop_discovery_bredrle_invalid_param,
	.send_len = sizeof(stop_discovery_bredrle_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
	.expect_param = stop_discovery_bredrle_invalid_param,
	.expect_len = sizeof(stop_discovery_bredrle_invalid_param),
};

static const char start_service_discovery_invalid_param[] = { 0x00, 0x00, 0x00, 0x00 };
static const char start_service_discovery_invalid_resp[] = { 0x00 };
static const char start_service_discovery_bredr_param[] = { 0x01, 0x00, 0x00, 0x00};
static const char start_service_discovery_bredr_resp[] = { 0x01 };
static const char start_service_discovery_le_param[] = { 0x06, 0x00, 0x01, 0x00,
			0xfa, 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00,
			0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const char start_service_discovery_le_resp[] = { 0x06 };
static const char start_service_discovery_bredrle_param[] = { 0x07, 0x00, 0x01, 0x00,
			0xfa, 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00,
			0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const char start_service_discovery_bredrle_resp[] = { 0x07 };
static const char start_service_discovery_valid_hci[] = { 0x01, 0x01 };
static const char start_service_discovery_evt[] = { 0x07, 0x01 };
static const char start_service_discovery_le_evt[] = { 0x06, 0x01 };

static const struct generic_data start_service_discovery_not_powered_test_1 = {
	.send_opcode = MGMT_OP_START_SERVICE_DISCOVERY,
	.send_param = start_service_discovery_bredr_param,
	.send_len = sizeof(start_service_discovery_bredr_param),
	.expect_status = MGMT_STATUS_NOT_POWERED,
	.expect_param = start_service_discovery_bredr_resp,
	.expect_len = sizeof(start_service_discovery_bredr_resp),
};

static const struct generic_data start_service_discovery_invalid_param_test_1 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_START_SERVICE_DISCOVERY,
	.send_param = start_service_discovery_invalid_param,
	.send_len = sizeof(start_service_discovery_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
	.expect_param = start_service_discovery_invalid_resp,
	.expect_len = sizeof(start_service_discovery_invalid_resp),
};

static const struct generic_data start_service_discovery_not_supported_test_1 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_START_SERVICE_DISCOVERY,
	.send_param = start_service_discovery_le_param,
	.send_len = sizeof(start_service_discovery_le_param),
	.expect_status = MGMT_STATUS_REJECTED,
	.expect_param = start_service_discovery_le_resp,
	.expect_len = sizeof(start_service_discovery_le_resp),
};

static const struct generic_data start_service_discovery_valid_param_test_1 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_START_SERVICE_DISCOVERY,
	.send_param = start_service_discovery_bredrle_param,
	.send_len = sizeof(start_service_discovery_bredrle_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = start_service_discovery_bredrle_resp,
	.expect_len = sizeof(start_service_discovery_bredrle_resp),
	.expect_hci_command = BT_HCI_CMD_LE_SET_SCAN_ENABLE,
	.expect_hci_param = start_service_discovery_valid_hci,
	.expect_hci_len = sizeof(start_service_discovery_valid_hci),
	.expect_alt_ev = MGMT_EV_DISCOVERING,
	.expect_alt_ev_param = start_service_discovery_evt,
	.expect_alt_ev_len = sizeof(start_service_discovery_evt),
};

static const struct generic_data start_service_discovery_valid_param_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_START_SERVICE_DISCOVERY,
	.send_param = start_service_discovery_le_param,
	.send_len = sizeof(start_service_discovery_le_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = start_service_discovery_le_resp,
	.expect_len = sizeof(start_service_discovery_le_resp),
	.expect_hci_command = BT_HCI_CMD_LE_SET_SCAN_ENABLE,
	.expect_hci_param = start_service_discovery_valid_hci,
	.expect_hci_len = sizeof(start_service_discovery_valid_hci),
	.expect_alt_ev = MGMT_EV_DISCOVERING,
	.expect_alt_ev_param = start_service_discovery_le_evt,
	.expect_alt_ev_len = sizeof(start_service_discovery_le_evt),
};

static const char set_dev_class_valid_param[] = { 0x01, 0x0c };
static const char set_dev_class_zero_rsp[] = { 0x00, 0x00, 0x00 };
static const char set_dev_class_valid_rsp[] = { 0x0c, 0x01, 0x00 };
static const char set_dev_class_valid_hci[] = { 0x0c, 0x01, 0x00 };
static const char set_dev_class_invalid_param[] = { 0x01, 0x01 };

static const struct generic_data set_dev_class_valid_param_test_1 = {
	.send_opcode = MGMT_OP_SET_DEV_CLASS,
	.send_param = set_dev_class_valid_param,
	.send_len = sizeof(set_dev_class_valid_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_dev_class_zero_rsp,
	.expect_len = sizeof(set_dev_class_zero_rsp),
};

static const struct generic_data set_dev_class_valid_param_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_DEV_CLASS,
	.send_param = set_dev_class_valid_param,
	.send_len = sizeof(set_dev_class_valid_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_dev_class_valid_rsp,
	.expect_len = sizeof(set_dev_class_valid_rsp),
	.expect_alt_ev = MGMT_EV_CLASS_OF_DEV_CHANGED,
	.expect_alt_ev_param = set_dev_class_valid_rsp,
	.expect_alt_ev_len = sizeof(set_dev_class_valid_rsp),
	.expect_hci_command = BT_HCI_CMD_WRITE_CLASS_OF_DEV,
	.expect_hci_param = set_dev_class_valid_hci,
	.expect_hci_len = sizeof(set_dev_class_valid_hci),
};

static const struct generic_data set_dev_class_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_SET_DEV_CLASS,
	.send_param = set_dev_class_invalid_param,
	.send_len = sizeof(set_dev_class_invalid_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const char add_spp_uuid_param[] = {
			0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00, 0x01, 0x11, 0x00, 0x00,
			0x00 };
static const char add_dun_uuid_param[] = {
			0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00, 0x03, 0x11, 0x00, 0x00,
			0x00 };
static const char add_sync_uuid_param[] = {
			0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00, 0x04, 0x11, 0x00, 0x00,
			0x00 };
static const char add_opp_uuid_param[] = {
			0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00, 0x05, 0x11, 0x00, 0x00,
			0x00 };
static const char write_eir_uuid16_hci[241] = { 0x00,
			0x02, 0x0a, 0x00, 0x03, 0x03, 0x01, 0x11 };
static const char write_eir_multi_uuid16_hci_1[241] = { 0x00,
			0x02, 0x0a, 0x00, 0x09, 0x03, 0x01, 0x11, 0x03,
			0x11, 0x04, 0x11, 0x05, 0x11 };
static const char write_eir_multi_uuid16_hci_2[241] = { 0x00,
			0x02, 0x0a, 0x00, 0xeb, 0x02, 0x00, 0x20, 0x01,
			0x20, 0x02, 0x20, 0x03, 0x20, 0x04, 0x20, 0x05,
			0x20, 0x06, 0x20, 0x07, 0x20, 0x08, 0x20, 0x09,
			0x20, 0x0a, 0x20, 0x0b, 0x20, 0x0c, 0x20, 0x0d,
			0x20, 0x0e, 0x20, 0x0f, 0x20, 0x10, 0x20, 0x11,
			0x20, 0x12, 0x20, 0x13, 0x20, 0x14, 0x20, 0x15,
			0x20, 0x16, 0x20, 0x17, 0x20, 0x18, 0x20, 0x19,
			0x20, 0x1a, 0x20, 0x1b, 0x20, 0x1c, 0x20, 0x1d,
			0x20, 0x1e, 0x20, 0x1f, 0x20, 0x20, 0x20, 0x21,
			0x20, 0x22, 0x20, 0x23, 0x20, 0x24, 0x20, 0x25,
			0x20, 0x26, 0x20, 0x27, 0x20, 0x28, 0x20, 0x29,
			0x20, 0x2a, 0x20, 0x2b, 0x20, 0x2c, 0x20, 0x2d,
			0x20, 0x2e, 0x20, 0x2f, 0x20, 0x30, 0x20, 0x31,
			0x20, 0x32, 0x20, 0x33, 0x20, 0x34, 0x20, 0x35,
			0x20, 0x36, 0x20, 0x37, 0x20, 0x38, 0x20, 0x39,
			0x20, 0x3a, 0x20, 0x3b, 0x20, 0x3c, 0x20, 0x3d,
			0x20, 0x3e, 0x20, 0x3f, 0x20, 0x40, 0x20, 0x41,
			0x20, 0x42, 0x20, 0x43, 0x20, 0x44, 0x20, 0x45,
			0x20, 0x46, 0x20, 0x47, 0x20, 0x48, 0x20, 0x49,
			0x20, 0x4a, 0x20, 0x4b, 0x20, 0x4c, 0x20, 0x4d,
			0x20, 0x4e, 0x20, 0x4f, 0x20, 0x50, 0x20, 0x51,
			0x20, 0x52, 0x20, 0x53, 0x20, 0x54, 0x20, 0x55,
			0x20, 0x56, 0x20, 0x57, 0x20, 0x58, 0x20, 0x59,
			0x20, 0x5a, 0x20, 0x5b, 0x20, 0x5c, 0x20, 0x5d,
			0x20, 0x5e, 0x20, 0x5f, 0x20, 0x60, 0x20, 0x61,
			0x20, 0x62, 0x20, 0x63, 0x20, 0x64, 0x20, 0x65,
			0x20, 0x66, 0x20, 0x67, 0x20, 0x68, 0x20, 0x69,
			0x20, 0x6a, 0x20, 0x6b, 0x20, 0x6c, 0x20, 0x6d,
			0x20, 0x6e, 0x20, 0x6f, 0x20, 0x70, 0x20, 0x71,
			0x20, 0x72, 0x20, 0x73, 0x20, 0x74, 0x20, 0x00 };
static const char add_uuid32_param_1[] = {
			0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00, 0x78, 0x56, 0x34, 0x12,
			0x00 };
static const char add_uuid32_param_2[] = {
			0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00, 0xef, 0xcd, 0xbc, 0x9a,
			0x00 };
static const char add_uuid32_param_3[] = {
			0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00, 0xff, 0xee, 0xdd, 0xcc,
			0x00 };
static const char add_uuid32_param_4[] = {
			0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44,
			0x00 };
static const char write_eir_uuid32_hci[241] = { 0x00,
			0x02, 0x0a, 0x00, 0x05, 0x05, 0x78, 0x56, 0x34,
			0x12 };
static const char write_eir_uuid32_multi_hci[241] = { 0x00,
			0x02, 0x0a, 0x00, 0x11, 0x05, 0x78, 0x56, 0x34,
			0x12, 0xef, 0xcd, 0xbc, 0x9a, 0xff, 0xee, 0xdd,
			0xcc, 0x11, 0x22, 0x33, 0x44 };
static const char write_eir_uuid32_multi_hci_2[] = { 0x00,
			0x02, 0x0a, 0x00, 0xe9, 0x04, 0xff, 0xff, 0xff,
			0xff, 0xfe, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff,
			0xff, 0xfc, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff,
			0xff, 0xfa, 0xff, 0xff, 0xff, 0xf9, 0xff, 0xff,
			0xff, 0xf8, 0xff, 0xff, 0xff, 0xf7, 0xff, 0xff,
			0xff, 0xf6, 0xff, 0xff, 0xff, 0xf5, 0xff, 0xff,
			0xff, 0xf4, 0xff, 0xff, 0xff, 0xf3, 0xff, 0xff,
			0xff, 0xf2, 0xff, 0xff, 0xff, 0xf1, 0xff, 0xff,
			0xff, 0xf0, 0xff, 0xff, 0xff, 0xef, 0xff, 0xff,
			0xff, 0xee, 0xff, 0xff, 0xff, 0xed, 0xff, 0xff,
			0xff, 0xec, 0xff, 0xff, 0xff, 0xeb, 0xff, 0xff,
			0xff, 0xea, 0xff, 0xff, 0xff, 0xe9, 0xff, 0xff,
			0xff, 0xe8, 0xff, 0xff, 0xff, 0xe7, 0xff, 0xff,
			0xff, 0xe6, 0xff, 0xff, 0xff, 0xe5, 0xff, 0xff,
			0xff, 0xe4, 0xff, 0xff, 0xff, 0xe3, 0xff, 0xff,
			0xff, 0xe2, 0xff, 0xff, 0xff, 0xe1, 0xff, 0xff,
			0xff, 0xe0, 0xff, 0xff, 0xff, 0xdf, 0xff, 0xff,
			0xff, 0xde, 0xff, 0xff, 0xff, 0xdd, 0xff, 0xff,
			0xff, 0xdc, 0xff, 0xff, 0xff, 0xdb, 0xff, 0xff,
			0xff, 0xda, 0xff, 0xff, 0xff, 0xd9, 0xff, 0xff,
			0xff, 0xd8, 0xff, 0xff, 0xff, 0xd7, 0xff, 0xff,
			0xff, 0xd6, 0xff, 0xff, 0xff, 0xd5, 0xff, 0xff,
			0xff, 0xd4, 0xff, 0xff, 0xff, 0xd3, 0xff, 0xff,
			0xff, 0xd2, 0xff, 0xff, 0xff, 0xd1, 0xff, 0xff,
			0xff, 0xd0, 0xff, 0xff, 0xff, 0xcf, 0xff, 0xff,
			0xff, 0xce, 0xff, 0xff, 0xff, 0xcd, 0xff, 0xff,
			0xff, 0xcc, 0xff, 0xff, 0xff, 0xcb, 0xff, 0xff,
			0xff, 0xca, 0xff, 0xff, 0xff, 0xc9, 0xff, 0xff,
			0xff, 0xc8, 0xff, 0xff, 0xff, 0xc7, 0xff, 0xff,
			0xff, 0xc6, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00 };
static const char add_uuid128_param_1[] = {
			0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
			0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
			0x00 };
static const char add_uuid128_param_2[] = {
			0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
			0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00,
			0x00 };
static const char write_eir_uuid128_hci[241] = { 0x00,
			0x02, 0x0a, 0x00, 0x11, 0x07, 0x00, 0x11, 0x22,
			0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
			0xbb, 0xcc, 0xdd, 0xee, 0xff };
static const char write_eir_uuid128_multi_hci[241] = { 0x00,
			0x02, 0x0a, 0x00, 0x21, 0x07, 0x00, 0x11, 0x22,
			0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
			0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xff, 0xee, 0xdd,
			0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55,
			0x44, 0x33, 0x22, 0x11 };
static const char write_eir_uuid128_multi_hci_2[] = { 0x00,
			0x02, 0x0a, 0x00, 0xe1, 0x07, 0x11, 0x22, 0x33,
			0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33,
			0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff, 0x01, 0x11, 0x22, 0x33,
			0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff, 0x02, 0x11, 0x22, 0x33,
			0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff, 0x03, 0x11, 0x22, 0x33,
			0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff, 0x04, 0x11, 0x22, 0x33,
			0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff, 0x05, 0x11, 0x22, 0x33,
			0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff, 0x06, 0x11, 0x22, 0x33,
			0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff, 0x07, 0x11, 0x22, 0x33,
			0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff, 0x08, 0x11, 0x22, 0x33,
			0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff, 0x09, 0x11, 0x22, 0x33,
			0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff, 0x0a, 0x11, 0x22, 0x33,
			0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff, 0x0b, 0x11, 0x22, 0x33,
			0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
			0xcc, 0xdd, 0xee, 0xff, 0x0c, 0xff, 0xee, 0xdd,
			0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55,
			0x44, 0x33, 0x22, 0x11, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static const char write_eir_uuid_mix_hci[241] = { 0x00,
			0x02, 0x0a, 0x00, 0x05, 0x03, 0x01, 0x11, 0x03,
			0x11, 0x09, 0x05, 0x78, 0x56, 0x34, 0x12, 0xef,
			0xcd, 0xbc, 0x9a, 0x21, 0x07, 0x00, 0x11, 0x22,
			0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa,
			0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xff, 0xee, 0xdd,
			0xcc, 0xbb, 0xaa, 0x99, 0x88, 0x77, 0x66, 0x55,
			0x44, 0x33, 0x22, 0x11 };

static const struct generic_data add_uuid16_test_1 = {
	.setup_settings = settings_powered_ssp,
	.send_opcode = MGMT_OP_ADD_UUID,
	.send_param = add_spp_uuid_param,
	.send_len = sizeof(add_spp_uuid_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_dev_class_zero_rsp,
	.expect_len = sizeof(set_dev_class_zero_rsp),
	.expect_hci_command = BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE,
	.expect_hci_param = write_eir_uuid16_hci,
	.expect_hci_len = sizeof(write_eir_uuid16_hci),
};

static const struct generic_data add_multi_uuid16_test_1 = {
	.send_opcode = MGMT_OP_ADD_UUID,
	.send_param = add_opp_uuid_param,
	.send_len = sizeof(add_opp_uuid_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_dev_class_zero_rsp,
	.expect_len = sizeof(set_dev_class_zero_rsp),
	.expect_hci_command = BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE,
	.expect_hci_param = write_eir_multi_uuid16_hci_1,
	.expect_hci_len = sizeof(write_eir_multi_uuid16_hci_1),
};

static const struct generic_data add_multi_uuid16_test_2 = {
	.send_opcode = MGMT_OP_ADD_UUID,
	.send_param = add_opp_uuid_param,
	.send_len = sizeof(add_opp_uuid_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_dev_class_zero_rsp,
	.expect_len = sizeof(set_dev_class_zero_rsp),
	.expect_hci_command = BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE,
	.expect_hci_param = write_eir_multi_uuid16_hci_2,
	.expect_hci_len = sizeof(write_eir_multi_uuid16_hci_2),
};

static const struct generic_data add_uuid32_test_1 = {
	.setup_settings = settings_powered_ssp,
	.send_opcode = MGMT_OP_ADD_UUID,
	.send_param = add_uuid32_param_1,
	.send_len = sizeof(add_uuid32_param_1),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_dev_class_zero_rsp,
	.expect_len = sizeof(set_dev_class_zero_rsp),
	.expect_hci_command = BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE,
	.expect_hci_param = write_eir_uuid32_hci,
	.expect_hci_len = sizeof(write_eir_uuid32_hci),
};

static const struct generic_data add_uuid32_multi_test_1 = {
	.send_opcode = MGMT_OP_ADD_UUID,
	.send_param = add_uuid32_param_4,
	.send_len = sizeof(add_uuid32_param_4),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_dev_class_zero_rsp,
	.expect_len = sizeof(set_dev_class_zero_rsp),
	.expect_hci_command = BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE,
	.expect_hci_param = write_eir_uuid32_multi_hci,
	.expect_hci_len = sizeof(write_eir_uuid32_multi_hci),
};

static const struct generic_data add_uuid32_multi_test_2 = {
	.send_opcode = MGMT_OP_ADD_UUID,
	.send_param = add_uuid32_param_4,
	.send_len = sizeof(add_uuid32_param_4),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_dev_class_zero_rsp,
	.expect_len = sizeof(set_dev_class_zero_rsp),
	.expect_hci_command = BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE,
	.expect_hci_param = write_eir_uuid32_multi_hci_2,
	.expect_hci_len = sizeof(write_eir_uuid32_multi_hci_2),
};

static const struct generic_data add_uuid128_test_1 = {
	.setup_settings = settings_powered_ssp,
	.send_opcode = MGMT_OP_ADD_UUID,
	.send_param = add_uuid128_param_1,
	.send_len = sizeof(add_uuid128_param_1),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_dev_class_zero_rsp,
	.expect_len = sizeof(set_dev_class_zero_rsp),
	.expect_hci_command = BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE,
	.expect_hci_param = write_eir_uuid128_hci,
	.expect_hci_len = sizeof(write_eir_uuid128_hci),
};

static const struct generic_data add_uuid128_multi_test_1 = {
	.send_opcode = MGMT_OP_ADD_UUID,
	.send_param = add_uuid128_param_2,
	.send_len = sizeof(add_uuid32_param_2),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_dev_class_zero_rsp,
	.expect_len = sizeof(set_dev_class_zero_rsp),
	.expect_hci_command = BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE,
	.expect_hci_param = write_eir_uuid128_multi_hci,
	.expect_hci_len = sizeof(write_eir_uuid128_multi_hci),
};

static const struct generic_data add_uuid128_multi_test_2 = {
	.send_opcode = MGMT_OP_ADD_UUID,
	.send_param = add_uuid128_param_2,
	.send_len = sizeof(add_uuid128_param_2),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_dev_class_zero_rsp,
	.expect_len = sizeof(set_dev_class_zero_rsp),
	.expect_hci_command = BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE,
	.expect_hci_param = write_eir_uuid128_multi_hci_2,
	.expect_hci_len = sizeof(write_eir_uuid128_multi_hci_2),
};

static const struct generic_data add_uuid_mix_test_1 = {
	.send_opcode = MGMT_OP_ADD_UUID,
	.send_param = add_uuid128_param_2,
	.send_len = sizeof(add_uuid128_param_2),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_dev_class_zero_rsp,
	.expect_len = sizeof(set_dev_class_zero_rsp),
	.expect_hci_command = BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE,
	.expect_hci_param = write_eir_uuid_mix_hci,
	.expect_hci_len = sizeof(write_eir_uuid_mix_hci),
};

static const char load_link_keys_valid_param_1[] = { 0x00, 0x00, 0x00 };
static const char load_link_keys_valid_param_2[] = { 0x01, 0x00, 0x00 };
static const char load_link_keys_invalid_param_1[] = { 0x02, 0x00, 0x00 };
static const char load_link_keys_invalid_param_2[] = { 0x00, 0x01, 0x00 };
/* Invalid bdaddr type */
static const char load_link_keys_invalid_param_3[] = { 0x00, 0x01, 0x00,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05,		/* addr */
	0x01,						/* addr type */
	0x00,						/* key type */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* value (1/2) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* value (2/2) */
	0x04,						/* PIN length */
};

static const struct generic_data load_link_keys_success_test_1 = {
	.send_opcode = MGMT_OP_LOAD_LINK_KEYS,
	.send_param = load_link_keys_valid_param_1,
	.send_len = sizeof(load_link_keys_valid_param_1),
	.expect_status = MGMT_STATUS_SUCCESS,
};

static const struct generic_data load_link_keys_success_test_2 = {
	.send_opcode = MGMT_OP_LOAD_LINK_KEYS,
	.send_param = load_link_keys_valid_param_2,
	.send_len = sizeof(load_link_keys_valid_param_2),
	.expect_status = MGMT_STATUS_SUCCESS,
};

static const struct generic_data load_link_keys_invalid_params_test_1 = {
	.send_opcode = MGMT_OP_LOAD_LINK_KEYS,
	.send_param = load_link_keys_invalid_param_1,
	.send_len = sizeof(load_link_keys_invalid_param_1),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data load_link_keys_invalid_params_test_2 = {
	.send_opcode = MGMT_OP_LOAD_LINK_KEYS,
	.send_param = load_link_keys_invalid_param_2,
	.send_len = sizeof(load_link_keys_invalid_param_2),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data load_link_keys_invalid_params_test_3 = {
	.send_opcode = MGMT_OP_LOAD_LINK_KEYS,
	.send_param = load_link_keys_invalid_param_3,
	.send_len = sizeof(load_link_keys_invalid_param_3),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const char load_ltks_valid_param_1[] = { 0x00, 0x00 };
/* Invalid key count */
static const char load_ltks_invalid_param_1[] = { 0x01, 0x00 };
/* Invalid addr type */
static const char load_ltks_invalid_param_2[] = {
	0x01, 0x00,					/* count */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05,		/* bdaddr */
	0x00,						/* addr type */
	0x00,						/* authenticated */
	0x00,						/* master */
	0x00,						/* encryption size */
	0x00, 0x00,					/* diversifier */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* rand */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* value (1/2) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* value (2/2) */
};
/* Invalid master value */
static const char load_ltks_invalid_param_3[] = {
	0x01, 0x00,					/* count */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05,		/* bdaddr */
	0x01,						/* addr type */
	0x00,						/* authenticated */
	0x02,						/* master */
	0x00,						/* encryption size */
	0x00, 0x00,					/* diversifier */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* rand */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* value (1/2) */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* value (2/2) */
};

static const struct generic_data load_ltks_success_test_1 = {
	.send_opcode = MGMT_OP_LOAD_LONG_TERM_KEYS,
	.send_param = load_ltks_valid_param_1,
	.send_len = sizeof(load_ltks_valid_param_1),
	.expect_status = MGMT_STATUS_SUCCESS,
};

static const struct generic_data load_ltks_invalid_params_test_1 = {
	.send_opcode = MGMT_OP_LOAD_LONG_TERM_KEYS,
	.send_param = load_ltks_invalid_param_1,
	.send_len = sizeof(load_ltks_invalid_param_1),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data load_ltks_invalid_params_test_2 = {
	.send_opcode = MGMT_OP_LOAD_LONG_TERM_KEYS,
	.send_param = load_ltks_invalid_param_2,
	.send_len = sizeof(load_ltks_invalid_param_2),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data load_ltks_invalid_params_test_3 = {
	.send_opcode = MGMT_OP_LOAD_LONG_TERM_KEYS,
	.send_param = load_ltks_invalid_param_3,
	.send_len = sizeof(load_ltks_invalid_param_3),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const char load_ltks_invalid_param_4[22] = { 0x1d, 0x07 };
static const struct generic_data load_ltks_invalid_params_test_4 = {
	.send_opcode = MGMT_OP_LOAD_LONG_TERM_KEYS,
	.send_param = load_ltks_invalid_param_4,
	.send_len = sizeof(load_ltks_invalid_param_4),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const char set_io_cap_invalid_param_1[] = { 0xff };

static const struct generic_data set_io_cap_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_SET_IO_CAPABILITY,
	.send_param = set_io_cap_invalid_param_1,
	.send_len = sizeof(set_io_cap_invalid_param_1),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const char pair_device_param[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00, 0x00 };
static const char pair_device_rsp[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00 };
static const char pair_device_invalid_param_1[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff, 0x00 };
static const char pair_device_invalid_param_rsp_1[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff };
static const char pair_device_invalid_param_2[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00, 0x05 };
static const char pair_device_invalid_param_rsp_2[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00 };

static const struct generic_data pair_device_not_powered_test_1 = {
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_param = pair_device_param,
	.send_len = sizeof(pair_device_param),
	.expect_status = MGMT_STATUS_NOT_POWERED,
	.expect_param = pair_device_rsp,
	.expect_len = sizeof(pair_device_rsp),
};

static const struct generic_data pair_device_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_param = pair_device_invalid_param_1,
	.send_len = sizeof(pair_device_invalid_param_1),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
	.expect_param = pair_device_invalid_param_rsp_1,
	.expect_len = sizeof(pair_device_invalid_param_rsp_1),
};

static const struct generic_data pair_device_invalid_param_test_2 = {
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_param = pair_device_invalid_param_2,
	.send_len = sizeof(pair_device_invalid_param_2),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
	.expect_param = pair_device_invalid_param_rsp_2,
	.expect_len = sizeof(pair_device_invalid_param_rsp_2),
};

static const void *pair_device_send_param_func(uint16_t *len)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	static uint8_t param[8];

	memcpy(param, hciemu_get_client_bdaddr(data->hciemu), 6);

	if (test->addr_type_avail)
		param[6] = test->addr_type;
	else if (data->hciemu_type == HCIEMU_TYPE_LE)
		param[6] = 0x01; /* Address type */
	else
		param[6] = 0x00; /* Address type */
	param[7] = test->io_cap;

	*len = sizeof(param);

	return param;
}

static const void *pair_device_expect_param_func(uint16_t *len)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	static uint8_t param[7];

	memcpy(param, hciemu_get_client_bdaddr(data->hciemu), 6);

	if (test->addr_type_avail)
		param[6] = test->addr_type;
	else if (data->hciemu_type == HCIEMU_TYPE_LE)
		param[6] = 0x01; /* Address type */
	else
		param[6] = 0x00; /* Address type */

	*len = sizeof(param);

	return param;
}

static uint16_t settings_powered_bondable[] = { MGMT_OP_SET_BONDABLE,
						MGMT_OP_SET_POWERED, 0 };
static uint8_t auth_req_param[] = { 0x2a, 0x00 };
static uint8_t pair_device_pin[] = { 0x30, 0x30, 0x30, 0x30 }; /* "0000" */

static const struct generic_data pair_device_success_test_1 = {
	.setup_settings = settings_powered_bondable,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.expect_hci_command = BT_HCI_CMD_AUTH_REQUESTED,
	.expect_hci_param = auth_req_param,
	.expect_hci_len = sizeof(auth_req_param),
	.pin = pair_device_pin,
	.pin_len = sizeof(pair_device_pin),
	.client_pin = pair_device_pin,
	.client_pin_len = sizeof(pair_device_pin),
};

static uint16_t settings_powered_bondable_linksec[] = { MGMT_OP_SET_BONDABLE,
							MGMT_OP_SET_POWERED,
							MGMT_OP_SET_LINK_SECURITY,
							0 };

static const struct generic_data pair_device_success_test_2 = {
	.setup_settings = settings_powered_bondable_linksec,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.expect_hci_command = BT_HCI_CMD_AUTH_REQUESTED,
	.expect_hci_param = auth_req_param,
	.expect_hci_len = sizeof(auth_req_param),
	.pin = pair_device_pin,
	.pin_len = sizeof(pair_device_pin),
	.client_pin = pair_device_pin,
	.client_pin_len = sizeof(pair_device_pin),
};

static const struct generic_data pair_device_legacy_nonbondable_1 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.pin = pair_device_pin,
	.pin_len = sizeof(pair_device_pin),
	.client_pin = pair_device_pin,
	.client_pin_len = sizeof(pair_device_pin),
};

static const struct generic_data pair_device_power_off_test_1 = {
	.setup_settings = settings_powered_bondable,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.force_power_off = true,
	.expect_status = MGMT_STATUS_NOT_POWERED,
	.expect_func = pair_device_expect_param_func,
};

static const void *client_bdaddr_param_func(uint8_t *len)
{
	struct test_data *data = tester_get_data();
	static uint8_t bdaddr[6];

	memcpy(bdaddr, hciemu_get_client_bdaddr(data->hciemu), 6);

	*len = sizeof(bdaddr);

	return bdaddr;
}

static const struct generic_data pair_device_not_supported_test_1 = {
	.setup_settings = settings_powered_bondable,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_NOT_SUPPORTED,
	.expect_func = pair_device_expect_param_func,
	.addr_type_avail = true,
	.addr_type = BDADDR_BREDR,
};

static const struct generic_data pair_device_not_supported_test_2 = {
	.setup_settings = settings_powered_bondable,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_NOT_SUPPORTED,
	.expect_func = pair_device_expect_param_func,
	.addr_type_avail = true,
	.addr_type = BDADDR_LE_PUBLIC,
};

static uint16_t settings_powered_bondable_le[] = { MGMT_OP_SET_LE,
							MGMT_OP_SET_BONDABLE,
							MGMT_OP_SET_POWERED,
							0 };

static const struct generic_data pair_device_reject_transport_not_enabled_1 = {
	.setup_settings = settings_powered_bondable_le,
	.setup_nobredr = true,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_REJECTED,
	.expect_func = pair_device_expect_param_func,
	.addr_type_avail = true,
	.addr_type = BDADDR_BREDR,
};

static const struct generic_data pair_device_reject_transport_not_enabled_2 = {
	.setup_settings = settings_powered_bondable,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_REJECTED,
	.expect_func = pair_device_expect_param_func,
	.addr_type_avail = true,
	.addr_type = BDADDR_LE_PUBLIC,
};

static const struct generic_data pair_device_reject_test_1 = {
	.setup_settings = settings_powered_bondable,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_AUTH_FAILED,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_AUTH_FAILED,
	.expect_alt_ev_len = 8,
	.expect_hci_command = BT_HCI_CMD_PIN_CODE_REQUEST_NEG_REPLY,
	.expect_hci_func = client_bdaddr_param_func,
	.expect_pin = true,
	.client_pin = pair_device_pin,
	.client_pin_len = sizeof(pair_device_pin),
};

static const struct generic_data pair_device_reject_test_2 = {
	.setup_settings = settings_powered_bondable,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_AUTH_FAILED,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_AUTH_FAILED,
	.expect_alt_ev_len = 8,
	.expect_hci_command = BT_HCI_CMD_AUTH_REQUESTED,
	.expect_hci_param = auth_req_param,
	.expect_hci_len = sizeof(auth_req_param),
	.pin = pair_device_pin,
	.pin_len = sizeof(pair_device_pin),
};

static const struct generic_data pair_device_reject_test_3 = {
	.setup_settings = settings_powered_bondable_linksec,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_AUTH_FAILED,
	.expect_func = pair_device_expect_param_func,
	.expect_hci_command = BT_HCI_CMD_PIN_CODE_REQUEST_NEG_REPLY,
	.expect_hci_func = client_bdaddr_param_func,
	.expect_pin = true,
	.client_pin = pair_device_pin,
	.client_pin_len = sizeof(pair_device_pin),
};

static const struct generic_data pair_device_reject_test_4 = {
	.setup_settings = settings_powered_bondable_linksec,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_AUTH_FAILED,
	.expect_func = pair_device_expect_param_func,
	.pin = pair_device_pin,
	.pin_len = sizeof(pair_device_pin),
};

static uint16_t settings_powered_bondable_ssp[] = {	MGMT_OP_SET_BONDABLE,
							MGMT_OP_SET_SSP,
							MGMT_OP_SET_POWERED,
							0 };

static const struct generic_data pair_device_ssp_test_1 = {
	.setup_settings = settings_powered_bondable_ssp,
	.client_enable_ssp = true,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.expect_hci_command = BT_HCI_CMD_USER_CONFIRM_REQUEST_REPLY,
	.expect_hci_func = client_bdaddr_param_func,
	.io_cap = 0x03, /* NoInputNoOutput */
	.client_io_cap = 0x03, /* NoInputNoOutput */
};

static const void *client_io_cap_param_func(uint8_t *len)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	static uint8_t param[9];

	memcpy(param, hciemu_get_client_bdaddr(data->hciemu), 6);
	memcpy(&param[6], test->expect_hci_param, 3);

	*len = sizeof(param);

	return param;
}

const uint8_t no_bonding_io_cap[] = { 0x03, 0x00, 0x00 };
static const struct generic_data pair_device_ssp_test_2 = {
	.setup_settings = settings_powered_ssp,
	.client_enable_ssp = true,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.expect_hci_command = BT_HCI_CMD_IO_CAPABILITY_REQUEST_REPLY,
	.expect_hci_func = client_io_cap_param_func,
	.expect_hci_param = no_bonding_io_cap,
	.expect_hci_len = sizeof(no_bonding_io_cap),
	.io_cap = 0x03, /* NoInputNoOutput */
	.client_io_cap = 0x03, /* NoInputNoOutput */
};

const uint8_t bonding_io_cap[] = { 0x03, 0x00, 0x02 };
static const struct generic_data pair_device_ssp_test_3 = {
	.setup_settings = settings_powered_bondable_ssp,
	.client_enable_ssp = true,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.expect_hci_command = BT_HCI_CMD_IO_CAPABILITY_REQUEST_REPLY,
	.expect_hci_func = client_io_cap_param_func,
	.expect_hci_param = bonding_io_cap,
	.expect_hci_len = sizeof(bonding_io_cap),
	.io_cap = 0x03, /* NoInputNoOutput */
	.client_io_cap = 0x03, /* NoInputNoOutput */
};

static const struct generic_data pair_device_ssp_test_4 = {
	.setup_settings = settings_powered_bondable_ssp,
	.client_enable_ssp = true,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.expect_hci_command = BT_HCI_CMD_USER_CONFIRM_REQUEST_REPLY,
	.expect_hci_func = client_bdaddr_param_func,
	.io_cap = 0x01, /* DisplayYesNo */
	.client_io_cap = 0x01, /* DisplayYesNo */
};

const uint8_t mitm_no_bonding_io_cap[] = { 0x01, 0x00, 0x01 };
static const struct generic_data pair_device_ssp_test_5 = {
	.setup_settings = settings_powered_ssp,
	.client_enable_ssp = true,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.expect_hci_command = BT_HCI_CMD_IO_CAPABILITY_REQUEST_REPLY,
	.expect_hci_func = client_io_cap_param_func,
	.expect_hci_param = mitm_no_bonding_io_cap,
	.expect_hci_len = sizeof(mitm_no_bonding_io_cap),
	.io_cap = 0x01, /* DisplayYesNo */
	.client_io_cap = 0x01, /* DisplayYesNo */
};

const uint8_t mitm_bonding_io_cap[] = { 0x01, 0x00, 0x03 };
static const struct generic_data pair_device_ssp_test_6 = {
	.setup_settings = settings_powered_bondable_ssp,
	.client_enable_ssp = true,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.expect_hci_command = BT_HCI_CMD_IO_CAPABILITY_REQUEST_REPLY,
	.expect_hci_func = client_io_cap_param_func,
	.expect_hci_param = mitm_bonding_io_cap,
	.expect_hci_len = sizeof(mitm_bonding_io_cap),
	.io_cap = 0x01, /* DisplayYesNo */
	.client_io_cap = 0x01, /* DisplayYesNo */
};

static const struct generic_data pair_device_ssp_reject_1 = {
	.setup_settings = settings_powered_bondable_ssp,
	.client_enable_ssp = true,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_AUTH_FAILED,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_AUTH_FAILED,
	.expect_alt_ev_len = 8,
	.expect_hci_command = BT_HCI_CMD_USER_CONFIRM_REQUEST_NEG_REPLY,
	.expect_hci_func = client_bdaddr_param_func,
	.io_cap = 0x01, /* DisplayYesNo */
	.client_io_cap = 0x01, /* DisplayYesNo */
	.client_auth_req = 0x01, /* No Bonding - MITM */
	.reject_confirm = true,
};

static const struct generic_data pair_device_ssp_reject_2 = {
	.setup_settings = settings_powered_bondable_ssp,
	.client_enable_ssp = true,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_AUTH_FAILED,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_AUTH_FAILED,
	.expect_alt_ev_len = 8,
	.expect_hci_command = BT_HCI_CMD_USER_CONFIRM_REQUEST_REPLY,
	.expect_hci_func = client_bdaddr_param_func,
	.io_cap = 0x01, /* DisplayYesNo */
	.client_io_cap = 0x01, /* DisplayYesNo */
	.client_reject_confirm = true,
};

static const struct generic_data pair_device_ssp_nonbondable_1 = {
	.setup_settings = settings_powered_ssp,
	.client_enable_ssp = true,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.expect_hci_command = BT_HCI_CMD_USER_CONFIRM_REQUEST_REPLY,
	.expect_hci_func = client_bdaddr_param_func,
	.io_cap = 0x01, /* DisplayYesNo */
	.client_io_cap = 0x01, /* DisplayYesNo */
};

static const struct generic_data pair_device_le_success_test_1 = {
	.setup_settings = settings_powered_bondable,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.just_works = true,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev =  MGMT_EV_NEW_LONG_TERM_KEY,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_new_long_term_key),
};

static bool ltk_is_authenticated(const struct mgmt_ltk_info *ltk)
{
	switch (ltk->type) {
	case 0x01:
	case 0x03:
		return true;
	default:
		return false;
	}
}

static bool ltk_is_sc(const struct mgmt_ltk_info *ltk)
{
	switch (ltk->type) {
	case 0x02:
	case 0x03:
	case 0x04:
		return true;
	default:
		return false;
	}
}

static bool verify_ltk(const void *param, uint16_t length)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	const struct mgmt_ev_new_long_term_key *ev = param;

	if (length != sizeof(struct mgmt_ev_new_long_term_key)) {
		tester_warn("Invalid new ltk length %u != %zu", length,
				sizeof(struct mgmt_ev_new_long_term_key));
		return false;
	}

	if (test->just_works && ltk_is_authenticated(&ev->key)) {
		tester_warn("Authenticated key for just-works");
		return false;
	}

	if (!test->just_works && !ltk_is_authenticated(&ev->key)) {
		tester_warn("Unauthenticated key for MITM");
		return false;
	}

	if (test->expect_sc_key && !ltk_is_sc(&ev->key)) {
		tester_warn("Non-LE SC key for SC pairing");
		return false;
	}

	if (!test->expect_sc_key && ltk_is_sc(&ev->key)) {
		tester_warn("SC key for Non-SC pairing");
		return false;
	}

	return true;
}

static const struct generic_data pair_device_le_success_test_2 = {
	.setup_settings = settings_powered_bondable,
	.io_cap = 0x02, /* KeyboardOnly */
	.client_io_cap = 0x04, /* KeyboardDisplay */
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev =  MGMT_EV_NEW_LONG_TERM_KEY,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_new_long_term_key),
	.verify_alt_ev_func = verify_ltk,
};

static uint16_t settings_powered_sc_bondable_le_ssp[] = {
						MGMT_OP_SET_BONDABLE,
						MGMT_OP_SET_LE,
						MGMT_OP_SET_SSP,
						MGMT_OP_SET_SECURE_CONN,
						MGMT_OP_SET_POWERED,
						0 };

static const struct generic_data pair_device_smp_bredr_test_1 = {
	.setup_settings = settings_powered_sc_bondable_le_ssp,
	.client_enable_ssp = true,
	.client_enable_le = true,
	.client_enable_sc = true,
	.expect_sc_key = true,
	.just_works = true,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev =  MGMT_EV_NEW_LONG_TERM_KEY,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_new_long_term_key),
	.verify_alt_ev_func = verify_ltk,
	.expect_hci_command = BT_HCI_CMD_USER_CONFIRM_REQUEST_REPLY,
	.expect_hci_func = client_bdaddr_param_func,
	.io_cap = 0x03, /* NoInputNoOutput */
	.client_io_cap = 0x03, /* NoInputNoOutput */
};

static const struct generic_data pair_device_smp_bredr_test_2 = {
	.setup_settings = settings_powered_sc_bondable_le_ssp,
	.client_enable_ssp = true,
	.client_enable_le = true,
	.client_enable_sc = true,
	.expect_sc_key = true,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev =  MGMT_EV_NEW_LONG_TERM_KEY,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_new_long_term_key),
	.verify_alt_ev_func = verify_ltk,
	.expect_hci_command = BT_HCI_CMD_USER_CONFIRM_REQUEST_REPLY,
	.expect_hci_func = client_bdaddr_param_func,
	.io_cap = 0x01, /* DisplayYesNo */
	.client_io_cap = 0x01, /* DisplayYesNo */
};

static const struct generic_data pair_device_le_reject_test_1 = {
	.setup_settings = settings_powered_bondable,
	.io_cap = 0x02, /* KeyboardOnly */
	.client_io_cap = 0x04, /* KeyboardDisplay */
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.expect_status = MGMT_STATUS_AUTH_FAILED,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev =  MGMT_EV_AUTH_FAILED,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_auth_failed),
	.reject_confirm = true,
};

static uint16_t settings_powered_sc_bondable[] = { MGMT_OP_SET_BONDABLE,
						MGMT_OP_SET_SECURE_CONN,
						MGMT_OP_SET_POWERED, 0 };

static const struct generic_data pair_device_le_sc_legacy_test_1 = {
	.setup_settings = settings_powered_sc_bondable,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.just_works = true,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev =  MGMT_EV_NEW_LONG_TERM_KEY,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_new_long_term_key),
	.verify_alt_ev_func = verify_ltk,
};

static const struct generic_data pair_device_le_sc_success_test_1 = {
	.setup_settings = settings_powered_sc_bondable,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.just_works = true,
	.client_enable_sc = true,
	.expect_sc_key = true,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev =  MGMT_EV_NEW_LONG_TERM_KEY,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_new_long_term_key),
	.verify_alt_ev_func = verify_ltk,
};

static const struct generic_data pair_device_le_sc_success_test_2 = {
	.setup_settings = settings_powered_sc_bondable,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.client_enable_sc = true,
	.expect_sc_key = true,
	.io_cap = 0x02, /* KeyboardOnly */
	.client_io_cap = 0x02, /* KeyboardOnly */
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev =  MGMT_EV_NEW_LONG_TERM_KEY,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_new_long_term_key),
	.verify_alt_ev_func = verify_ltk,
};

static bool lk_is_authenticated(const struct mgmt_link_key_info *lk)
{
	switch (lk->type) {
	case 0x00: /* Combination Key */
	case 0x01: /* Local Unit Key */
	case 0x02: /* Remote Unit Key */
	case 0x03: /* Debug Combination Key */
		if (lk->pin_len == 16)
			return true;
		return false;
	case 0x05: /* Authenticated Combination Key generated from P-192 */
	case 0x08: /* Authenticated Combination Key generated from P-256 */
		return true;
	default:
		return false;
	}
}

static bool lk_is_sc(const struct mgmt_link_key_info *lk)
{
	switch (lk->type) {
	case 0x07: /* Unauthenticated Combination Key generated from P-256 */
	case 0x08: /* Authenticated Combination Key generated from P-256 */
		return true;
	default:
		return false;
	}
}

static bool verify_link_key(const void *param, uint16_t length)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	const struct mgmt_ev_new_link_key *ev = param;

	if (length != sizeof(struct mgmt_ev_new_link_key)) {
		tester_warn("Invalid new Link Key length %u != %zu", length,
				sizeof(struct mgmt_ev_new_link_key));
		return false;
	}

	if (test->just_works && lk_is_authenticated(&ev->key)) {
		tester_warn("Authenticated key for just-works");
		return false;
	}

	if (!test->just_works && !lk_is_authenticated(&ev->key)) {
		tester_warn("Unauthenticated key for MITM");
		return false;
	}

	if (test->expect_sc_key && !lk_is_sc(&ev->key)) {
		tester_warn("Non-LE SC key for SC pairing");
		return false;
	}

	if (!test->expect_sc_key && lk_is_sc(&ev->key)) {
		tester_warn("SC key for Non-SC pairing");
		return false;
	}

	return true;
}

static uint16_t settings_powered_le_sc_bondable[] = {
						MGMT_OP_SET_LE,
						MGMT_OP_SET_SSP,
						MGMT_OP_SET_BONDABLE,
						MGMT_OP_SET_SECURE_CONN,
						MGMT_OP_SET_POWERED, 0 };

static const struct generic_data pair_device_le_sc_success_test_3 = {
	.setup_settings = settings_powered_le_sc_bondable,
	.send_opcode = MGMT_OP_PAIR_DEVICE,
	.send_func = pair_device_send_param_func,
	.addr_type_avail = true,
	.addr_type = 0x01,
	.client_enable_sc = true,
	.client_enable_ssp = true,
	.client_enable_adv = true,
	.expect_sc_key = true,
	.io_cap = 0x02, /* KeyboardOnly */
	.client_io_cap = 0x02, /* KeyboardOnly */
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = pair_device_expect_param_func,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.verify_alt_ev_func = verify_link_key,
};

static uint16_t settings_powered_connectable_bondable[] = {
						MGMT_OP_SET_BONDABLE,
						MGMT_OP_SET_CONNECTABLE,
						MGMT_OP_SET_POWERED, 0 };

static const struct generic_data pairing_acceptor_legacy_1 = {
	.setup_settings = settings_powered_connectable_bondable,
	.pin = pair_device_pin,
	.pin_len = sizeof(pair_device_pin),
	.client_pin = pair_device_pin,
	.client_pin_len = sizeof(pair_device_pin),
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
};

static const struct generic_data pairing_acceptor_legacy_2 = {
	.setup_settings = settings_powered_connectable_bondable,
	.expect_pin = true,
	.client_pin = pair_device_pin,
	.client_pin_len = sizeof(pair_device_pin),
	.expect_alt_ev = MGMT_EV_AUTH_FAILED,
	.expect_alt_ev_len = 8,
};

static const struct generic_data pairing_acceptor_legacy_3 = {
	.setup_settings = settings_powered_connectable,
	.client_pin = pair_device_pin,
	.client_pin_len = sizeof(pair_device_pin),
	.expect_alt_ev = MGMT_EV_AUTH_FAILED,
	.expect_alt_ev_len = 8,
	.expect_hci_command = BT_HCI_CMD_PIN_CODE_REQUEST_NEG_REPLY,
	.expect_hci_func = client_bdaddr_param_func,
};

static uint16_t settings_powered_connectable_bondable_linksec[] = {
						MGMT_OP_SET_BONDABLE,
						MGMT_OP_SET_CONNECTABLE,
						MGMT_OP_SET_LINK_SECURITY,
						MGMT_OP_SET_POWERED, 0 };

static const struct generic_data pairing_acceptor_linksec_1 = {
	.setup_settings = settings_powered_connectable_bondable_linksec,
	.pin = pair_device_pin,
	.pin_len = sizeof(pair_device_pin),
	.client_pin = pair_device_pin,
	.client_pin_len = sizeof(pair_device_pin),
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
};

static const struct generic_data pairing_acceptor_linksec_2 = {
	.setup_settings = settings_powered_connectable_bondable_linksec,
	.expect_pin = true,
	.client_pin = pair_device_pin,
	.client_pin_len = sizeof(pair_device_pin),
	.expect_alt_ev = MGMT_EV_CONNECT_FAILED,
	.expect_alt_ev_len = 8,
};

static uint16_t settings_powered_connectable_bondable_ssp[] = {
						MGMT_OP_SET_BONDABLE,
						MGMT_OP_SET_CONNECTABLE,
						MGMT_OP_SET_SSP,
						MGMT_OP_SET_POWERED, 0 };

static const struct generic_data pairing_acceptor_ssp_1 = {
	.setup_settings = settings_powered_connectable_bondable_ssp,
	.client_enable_ssp = true,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.expect_hci_command = BT_HCI_CMD_USER_CONFIRM_REQUEST_REPLY,
	.expect_hci_func = client_bdaddr_param_func,
	.io_cap = 0x03, /* NoInputNoOutput */
	.client_io_cap = 0x03, /* NoInputNoOutput */
	.just_works = true,
};

static const struct generic_data pairing_acceptor_ssp_2 = {
	.setup_settings = settings_powered_connectable_bondable_ssp,
	.client_enable_ssp = true,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.expect_hci_command = BT_HCI_CMD_USER_CONFIRM_REQUEST_REPLY,
	.expect_hci_func = client_bdaddr_param_func,
	.io_cap = 0x01, /* DisplayYesNo */
	.client_io_cap = 0x01, /* DisplayYesNo */
};

static const struct generic_data pairing_acceptor_ssp_3 = {
	.setup_settings = settings_powered_connectable_bondable_ssp,
	.client_enable_ssp = true,
	.expect_alt_ev = MGMT_EV_NEW_LINK_KEY,
	.expect_alt_ev_len = 26,
	.expect_hci_command = BT_HCI_CMD_USER_CONFIRM_REQUEST_REPLY,
	.expect_hci_func = client_bdaddr_param_func,
	.io_cap = 0x01, /* DisplayYesNo */
	.client_io_cap = 0x01, /* DisplayYesNo */
	.just_works = true,
};

static const void *client_io_cap_reject_param_func(uint8_t *len)
{
	struct test_data *data = tester_get_data();
	static uint8_t param[7];

	memcpy(param, hciemu_get_client_bdaddr(data->hciemu), 6);
	param[6] = 0x18; /* Pairing Not Allowed */

	*len = sizeof(param);

	return param;
}

static uint16_t settings_powered_connectable_ssp[] = {
						MGMT_OP_SET_CONNECTABLE,
						MGMT_OP_SET_SSP,
						MGMT_OP_SET_POWERED, 0 };

static const struct generic_data pairing_acceptor_ssp_4 = {
	.setup_settings = settings_powered_connectable_ssp,
	.client_enable_ssp = true,
	.expect_alt_ev = MGMT_EV_AUTH_FAILED,
	.expect_alt_ev_len = 8,
	.expect_hci_command = BT_HCI_CMD_IO_CAPABILITY_REQUEST_NEG_REPLY,
	.expect_hci_func = client_io_cap_reject_param_func,
	.io_cap = 0x01, /* DisplayYesNo */
	.client_io_cap = 0x01, /* DisplayYesNo */
	.client_auth_req = 0x02, /* Dedicated Bonding - No MITM */
};

static uint16_t settings_powered_sc_bondable_connectable_le_ssp[] = {
						MGMT_OP_SET_BONDABLE,
						MGMT_OP_SET_CONNECTABLE,
						MGMT_OP_SET_LE,
						MGMT_OP_SET_SSP,
						MGMT_OP_SET_SECURE_CONN,
						MGMT_OP_SET_POWERED,
						0 };

static const struct generic_data pairing_acceptor_smp_bredr_1 = {
	.setup_settings = settings_powered_sc_bondable_connectable_le_ssp,
	.client_enable_ssp = true,
	.client_enable_le = true,
	.client_enable_sc = true,
	.expect_sc_key = true,
	.expect_alt_ev =  MGMT_EV_NEW_LONG_TERM_KEY,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_new_long_term_key),
	.verify_alt_ev_func = verify_ltk,
	.just_works = true,
	.io_cap = 0x03, /* NoInputNoOutput */
	.client_io_cap = 0x03, /* No InputNoOutput */
	.client_auth_req = 0x00, /* No Bonding - No MITM */
};

static const struct generic_data pairing_acceptor_smp_bredr_2 = {
	.setup_settings = settings_powered_sc_bondable_connectable_le_ssp,
	.client_enable_ssp = true,
	.client_enable_le = true,
	.client_enable_sc = true,
	.expect_sc_key = true,
	.expect_alt_ev =  MGMT_EV_NEW_LONG_TERM_KEY,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_new_long_term_key),
	.verify_alt_ev_func = verify_ltk,
	.io_cap = 0x01, /* DisplayYesNo */
	.client_io_cap = 0x01, /* DisplayYesNo */
	.client_auth_req = 0x02, /* Dedicated Bonding - No MITM */
};

static uint16_t settings_powered_bondable_connectable_advertising[] = {
					MGMT_OP_SET_BONDABLE,
					MGMT_OP_SET_CONNECTABLE,
					MGMT_OP_SET_ADVERTISING,
					MGMT_OP_SET_POWERED, 0 };

static const struct generic_data pairing_acceptor_le_1 = {
	.setup_settings = settings_powered_bondable_connectable_advertising,
	.io_cap = 0x03, /* NoInputNoOutput */
	.client_io_cap = 0x03, /* NoInputNoOutput */
	.just_works = true,
	.expect_alt_ev =  MGMT_EV_NEW_LONG_TERM_KEY,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_new_long_term_key),
	.verify_alt_ev_func = verify_ltk,
};

static const struct generic_data pairing_acceptor_le_2 = {
	.setup_settings = settings_powered_bondable_connectable_advertising,
	.io_cap = 0x04, /* KeyboardDisplay */
	.client_io_cap = 0x04, /* KeyboardDisplay */
	.client_auth_req = 0x05, /* Bonding - MITM */
	.expect_alt_ev =  MGMT_EV_NEW_LONG_TERM_KEY,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_new_long_term_key),
	.verify_alt_ev_func = verify_ltk,
};

static const struct generic_data pairing_acceptor_le_3 = {
	.setup_settings = settings_powered_bondable_connectable_advertising,
	.io_cap = 0x04, /* KeyboardDisplay */
	.client_io_cap = 0x04, /* KeyboardDisplay */
	.expect_alt_ev =  MGMT_EV_AUTH_FAILED,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_auth_failed),
	.reject_confirm = true,
};

static const struct generic_data pairing_acceptor_le_4 = {
	.setup_settings = settings_powered_bondable_connectable_advertising,
	.io_cap = 0x02, /* KeyboardOnly */
	.client_io_cap = 0x04, /* KeyboardDisplay */
	.client_auth_req = 0x05, /* Bonding - MITM */
	.expect_alt_ev =  MGMT_EV_NEW_LONG_TERM_KEY,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_new_long_term_key),
	.verify_alt_ev_func = verify_ltk,
};

static const struct generic_data pairing_acceptor_le_5 = {
	.setup_settings = settings_powered_bondable_connectable_advertising,
	.io_cap = 0x02, /* KeyboardOnly */
	.client_io_cap = 0x04, /* KeyboardDisplay */
	.client_auth_req = 0x05, /* Bonding - MITM */
	.reject_confirm = true,
	.expect_alt_ev =  MGMT_EV_AUTH_FAILED,
	.expect_alt_ev_len = sizeof(struct mgmt_ev_auth_failed),
};

static const char unpair_device_param[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00, 0x00 };
static const char unpair_device_rsp[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00 };
static const char unpair_device_invalid_param_1[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff, 0x00 };
static const char unpair_device_invalid_param_rsp_1[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff };
static const char unpair_device_invalid_param_2[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00, 0x02 };
static const char unpair_device_invalid_param_rsp_2[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00 };

static const struct generic_data unpair_device_not_powered_test_1 = {
	.send_opcode = MGMT_OP_UNPAIR_DEVICE,
	.send_param = unpair_device_param,
	.send_len = sizeof(unpair_device_param),
	.expect_status = MGMT_STATUS_NOT_POWERED,
	.expect_param = unpair_device_rsp,
	.expect_len = sizeof(unpair_device_rsp),
};

static const struct generic_data unpair_device_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_UNPAIR_DEVICE,
	.send_param = unpair_device_invalid_param_1,
	.send_len = sizeof(unpair_device_invalid_param_1),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
	.expect_param = unpair_device_invalid_param_rsp_1,
	.expect_len = sizeof(unpair_device_invalid_param_rsp_1),
};

static const struct generic_data unpair_device_invalid_param_test_2 = {
	.send_opcode = MGMT_OP_UNPAIR_DEVICE,
	.send_param = unpair_device_invalid_param_2,
	.send_len = sizeof(unpair_device_invalid_param_2),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
	.expect_param = unpair_device_invalid_param_rsp_2,
	.expect_len = sizeof(unpair_device_invalid_param_rsp_2),
};

static const char disconnect_invalid_param_1[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff };
static const char disconnect_invalid_param_rsp_1[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff };

static const struct generic_data disconnect_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_DISCONNECT,
	.send_param = disconnect_invalid_param_1,
	.send_len = sizeof(disconnect_invalid_param_1),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
	.expect_param = disconnect_invalid_param_rsp_1,
	.expect_len = sizeof(disconnect_invalid_param_rsp_1),
};

static const char block_device_invalid_param_1[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff };
static const char block_device_invalid_param_rsp_1[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff };

static const struct generic_data block_device_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_BLOCK_DEVICE,
	.send_param = block_device_invalid_param_1,
	.send_len = sizeof(block_device_invalid_param_1),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
	.expect_param = block_device_invalid_param_rsp_1,
	.expect_len = sizeof(block_device_invalid_param_rsp_1),
};

static const char unblock_device_invalid_param_1[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff };
static const char unblock_device_invalid_param_rsp_1[] = {
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0xff };

static const struct generic_data unblock_device_invalid_param_test_1 = {
	.send_opcode = MGMT_OP_UNBLOCK_DEVICE,
	.send_param = unblock_device_invalid_param_1,
	.send_len = sizeof(unblock_device_invalid_param_1),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
	.expect_param = unblock_device_invalid_param_rsp_1,
	.expect_len = sizeof(unblock_device_invalid_param_rsp_1),
};

static const char set_static_addr_valid_param[] = {
			0x11, 0x22, 0x33, 0x44, 0x55, 0xc0 };
static const char set_static_addr_settings[] = { 0x00, 0x82, 0x00, 0x00 };

static const struct generic_data set_static_addr_success_test = {
	.send_opcode = MGMT_OP_SET_STATIC_ADDRESS,
	.send_param = set_static_addr_valid_param,
	.send_len = sizeof(set_static_addr_valid_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_static_addr_settings,
	.expect_len = sizeof(set_static_addr_settings),
	.expect_settings_set = MGMT_SETTING_STATIC_ADDRESS,
};

static const char set_static_addr_settings_dual[] = { 0x80, 0x00, 0x00, 0x00 };

static const struct generic_data set_static_addr_success_test_2 = {
	.send_opcode = MGMT_OP_SET_STATIC_ADDRESS,
	.send_param = set_static_addr_valid_param,
	.send_len = sizeof(set_static_addr_valid_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_static_addr_settings_dual,
	.expect_len = sizeof(set_static_addr_settings_dual),
};

static const struct generic_data set_static_addr_failure_test = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_STATIC_ADDRESS,
	.send_param = set_static_addr_valid_param,
	.send_len = sizeof(set_static_addr_valid_param),
	.expect_status = MGMT_STATUS_REJECTED,
};

static const struct generic_data set_static_addr_failure_test_2 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_STATIC_ADDRESS,
	.send_param = set_static_addr_valid_param,
	.send_len = sizeof(set_static_addr_valid_param),
	.expect_status = MGMT_STATUS_NOT_SUPPORTED,
};

static const char set_scan_params_valid_param[] = { 0x60, 0x00, 0x30, 0x00 };

static const struct generic_data set_scan_params_success_test = {
	.send_opcode = MGMT_OP_SET_SCAN_PARAMS,
	.send_param = set_scan_params_valid_param,
	.send_len = sizeof(set_scan_params_valid_param),
	.expect_status = MGMT_STATUS_SUCCESS,
};

static const char load_irks_empty_list[] = { 0x00, 0x00 };

static const struct generic_data load_irks_success1_test = {
	.send_opcode = MGMT_OP_LOAD_IRKS,
	.send_param = load_irks_empty_list,
	.send_len = sizeof(load_irks_empty_list),
	.expect_status = MGMT_STATUS_SUCCESS,
};

static const char load_irks_one_irk[] = { 0x01, 0x00,
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x01,
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

static const struct generic_data load_irks_success2_test = {
	.send_opcode = MGMT_OP_LOAD_IRKS,
	.send_param = load_irks_one_irk,
	.send_len = sizeof(load_irks_one_irk),
	.expect_status = MGMT_STATUS_SUCCESS,
};

static const char load_irks_nval_addr_type[] = { 0x01, 0x00,
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00,
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

static const struct generic_data load_irks_nval_param1_test = {
	.send_opcode = MGMT_OP_LOAD_IRKS,
	.send_param = load_irks_nval_addr_type,
	.send_len = sizeof(load_irks_nval_addr_type),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const char load_irks_nval_rand_addr[] = { 0x01, 0x00,
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x02,
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

static const struct generic_data load_irks_nval_param2_test = {
	.send_opcode = MGMT_OP_LOAD_IRKS,
	.send_param = load_irks_nval_rand_addr,
	.send_len = sizeof(load_irks_nval_rand_addr),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const char load_irks_nval_len[] = { 0x02, 0x00, 0xff, 0xff };

static const struct generic_data load_irks_nval_param3_test = {
	.send_opcode = MGMT_OP_LOAD_IRKS,
	.send_param = load_irks_nval_len,
	.send_len = sizeof(load_irks_nval_len),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data load_irks_not_supported_test = {
	.send_opcode = MGMT_OP_LOAD_IRKS,
	.send_param = load_irks_empty_list,
	.send_len = sizeof(load_irks_empty_list),
	.expect_status = MGMT_STATUS_NOT_SUPPORTED,
};

static const char set_privacy_valid_param[] = { 0x01,
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
static const char set_privacy_settings_param[] = { 0x80, 0x20, 0x00, 0x00 };

static const struct generic_data set_privacy_success_test = {
	.send_opcode = MGMT_OP_SET_PRIVACY,
	.send_param = set_privacy_valid_param,
	.send_len = sizeof(set_privacy_valid_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_privacy_settings_param,
	.expect_len = sizeof(set_privacy_settings_param),
	.expect_settings_set = MGMT_SETTING_PRIVACY,
};

static const struct generic_data set_privacy_powered_test = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_SET_PRIVACY,
	.send_param = set_privacy_valid_param,
	.send_len = sizeof(set_privacy_valid_param),
	.expect_status = MGMT_STATUS_REJECTED,
};

static const char set_privacy_nval_param[] = { 0xff,
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
static const struct generic_data set_privacy_nval_param_test = {
	.send_opcode = MGMT_OP_SET_PRIVACY,
	.send_param = set_privacy_nval_param,
	.send_len = sizeof(set_privacy_nval_param),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const void *get_conn_info_send_param_func(uint16_t *len)
{
	struct test_data *data = tester_get_data();
	static uint8_t param[7];

	memcpy(param, hciemu_get_client_bdaddr(data->hciemu), 6);
	param[6] = 0x00; /* Address type */

	*len = sizeof(param);

	return param;
}

static const void *get_conn_info_expect_param_func(uint16_t *len)
{
	struct test_data *data = tester_get_data();
	static uint8_t param[10];

	memcpy(param, hciemu_get_client_bdaddr(data->hciemu), 6);
	param[6] = 0x00; /* Address type */
	param[7] = 0xff; /* RSSI (= -1) */
	param[8] = 0xff; /* TX power (= -1) */
	param[9] = 0x04; /* max TX power */

	*len = sizeof(param);

	return param;
}

static const void *get_conn_info_error_expect_param_func(uint16_t *len)
{
	struct test_data *data = tester_get_data();
	static uint8_t param[10];

	/* All unset parameters shall be 0 in case of error */
	memset(param, 0, sizeof(param));

	memcpy(param, hciemu_get_client_bdaddr(data->hciemu), 6);
	param[6] = 0x00; /* Address type */

	*len = sizeof(param);

	return param;
}

static const struct generic_data get_conn_info_succes1_test = {
	.setup_settings = settings_powered_connectable_bondable_ssp,
	.send_opcode = MGMT_OP_GET_CONN_INFO,
	.send_func = get_conn_info_send_param_func,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_func = get_conn_info_expect_param_func,
};

static const struct generic_data get_conn_info_ncon_test = {
	.setup_settings = settings_powered_connectable_bondable_ssp,
	.send_opcode = MGMT_OP_GET_CONN_INFO,
	.send_func = get_conn_info_send_param_func,
	.expect_status = MGMT_STATUS_NOT_CONNECTED,
	.expect_func = get_conn_info_error_expect_param_func,
};

static const void *get_conn_info_expect_param_power_off_func(uint16_t *len)
{
	struct test_data *data = tester_get_data();
	static uint8_t param[10];

	memcpy(param, hciemu_get_client_bdaddr(data->hciemu), 6);
	param[6] = 0x00; /* Address type */
	param[7] = 127; /* RSSI */
	param[8] = 127; /* TX power */
	param[9] = 127; /* max TX power */

	*len = sizeof(param);

	return param;
}

static const struct generic_data get_conn_info_power_off_test = {
	.setup_settings = settings_powered_connectable_bondable_ssp,
	.send_opcode = MGMT_OP_GET_CONN_INFO,
	.send_func = get_conn_info_send_param_func,
	.force_power_off = true,
	.expect_status = MGMT_STATUS_NOT_POWERED,
	.expect_func = get_conn_info_expect_param_power_off_func,
};

static const uint8_t load_conn_param_nval_1[16] = { 0x12, 0x11 };
static const struct generic_data load_conn_params_fail_1 = {
	.send_opcode = MGMT_OP_LOAD_CONN_PARAM,
	.send_param = load_conn_param_nval_1,
	.send_len = sizeof(load_conn_param_nval_1),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const uint8_t add_device_nval_1[] = {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0x00,
					0x00,
};
static const uint8_t add_device_rsp[] =  {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0x00,
};
static const struct generic_data add_device_fail_1 = {
	.send_opcode = MGMT_OP_ADD_DEVICE,
	.send_param = add_device_nval_1,
	.send_len = sizeof(add_device_nval_1),
	.expect_param = add_device_rsp,
	.expect_len = sizeof(add_device_rsp),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const uint8_t add_device_nval_2[] = {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0x00,
					0x02,
};
static const struct generic_data add_device_fail_2 = {
	.send_opcode = MGMT_OP_ADD_DEVICE,
	.send_param = add_device_nval_2,
	.send_len = sizeof(add_device_nval_2),
	.expect_param = add_device_rsp,
	.expect_len = sizeof(add_device_rsp),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const uint8_t add_device_nval_3[] = {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0x00,
					0xff,
};
static const struct generic_data add_device_fail_3 = {
	.send_opcode = MGMT_OP_ADD_DEVICE,
	.send_param = add_device_nval_3,
	.send_len = sizeof(add_device_nval_3),
	.expect_param = add_device_rsp,
	.expect_len = sizeof(add_device_rsp),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const uint8_t add_device_nval_4[] = {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0x02,
					0x02,
};
static const uint8_t add_device_rsp_4[] =  {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0x02,
};
static const struct generic_data add_device_fail_4 = {
	.send_opcode = MGMT_OP_ADD_DEVICE,
	.send_param = add_device_nval_4,
	.send_len = sizeof(add_device_nval_4),
	.expect_param = add_device_rsp_4,
	.expect_len = sizeof(add_device_rsp_4),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const uint8_t add_device_success_param_1[] = {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0x00,
					0x01,
};
static const struct generic_data add_device_success_1 = {
	.send_opcode = MGMT_OP_ADD_DEVICE,
	.send_param = add_device_success_param_1,
	.send_len = sizeof(add_device_success_param_1),
	.expect_param = add_device_rsp,
	.expect_len = sizeof(add_device_rsp),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_DEVICE_ADDED,
	.expect_alt_ev_param = add_device_success_param_1,
	.expect_alt_ev_len = sizeof(add_device_success_param_1),
};

static const uint8_t add_device_success_param_2[] = {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0x01,
					0x00,
};
static const uint8_t add_device_rsp_le[] =  {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0x01,
};
static const struct generic_data add_device_success_2 = {
	.send_opcode = MGMT_OP_ADD_DEVICE,
	.send_param = add_device_success_param_2,
	.send_len = sizeof(add_device_success_param_2),
	.expect_param = add_device_rsp_le,
	.expect_len = sizeof(add_device_rsp_le),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_DEVICE_ADDED,
	.expect_alt_ev_param = add_device_success_param_2,
	.expect_alt_ev_len = sizeof(add_device_success_param_2),
};

static const uint8_t add_device_success_param_3[] = {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0x01,
					0x02,
};
static const struct generic_data add_device_success_3 = {
	.send_opcode = MGMT_OP_ADD_DEVICE,
	.send_param = add_device_success_param_3,
	.send_len = sizeof(add_device_success_param_3),
	.expect_param = add_device_rsp_le,
	.expect_len = sizeof(add_device_rsp_le),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_DEVICE_ADDED,
	.expect_alt_ev_param = add_device_success_param_3,
	.expect_alt_ev_len = sizeof(add_device_success_param_3),
};

static const struct generic_data add_device_success_4 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_ADD_DEVICE,
	.send_param = add_device_success_param_1,
	.send_len = sizeof(add_device_success_param_1),
	.expect_param = add_device_rsp,
	.expect_len = sizeof(add_device_rsp),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_DEVICE_ADDED,
	.expect_alt_ev_param = add_device_success_param_1,
	.expect_alt_ev_len = sizeof(add_device_success_param_1),
	.expect_hci_command = BT_HCI_CMD_WRITE_SCAN_ENABLE,
	.expect_hci_param = set_connectable_scan_enable_param,
	.expect_hci_len = sizeof(set_connectable_scan_enable_param),
};

static const uint8_t le_scan_enable[] = { 0x01, 0x01 };
static const struct generic_data add_device_success_5 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_DEVICE,
	.send_param = add_device_success_param_2,
	.send_len = sizeof(add_device_success_param_2),
	.expect_param = add_device_rsp_le,
	.expect_len = sizeof(add_device_rsp_le),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_DEVICE_ADDED,
	.expect_alt_ev_param = add_device_success_param_2,
	.expect_alt_ev_len = sizeof(add_device_success_param_2),
	.expect_hci_command = BT_HCI_CMD_LE_SET_SCAN_ENABLE,
	.expect_hci_param = le_scan_enable,
	.expect_hci_len = sizeof(le_scan_enable),
};

static const uint8_t remove_device_nval_1[] = {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0xff,
};
static const struct generic_data remove_device_fail_1 = {
	.send_opcode = MGMT_OP_REMOVE_DEVICE,
	.send_param = remove_device_nval_1,
	.send_len = sizeof(remove_device_nval_1),
	.expect_param = remove_device_nval_1,
	.expect_len = sizeof(remove_device_nval_1),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const uint8_t remove_device_param_1[] = {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0x00,
};
static const struct generic_data remove_device_fail_2 = {
	.send_opcode = MGMT_OP_REMOVE_DEVICE,
	.send_param = remove_device_param_1,
	.send_len = sizeof(remove_device_param_1),
	.expect_param = remove_device_param_1,
	.expect_len = sizeof(remove_device_param_1),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const uint8_t remove_device_param_3[] = {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0x02,
};
static const struct generic_data remove_device_fail_3 = {
	.send_opcode = MGMT_OP_REMOVE_DEVICE,
	.send_param = remove_device_param_3,
	.send_len = sizeof(remove_device_param_3),
	.expect_param = remove_device_param_3,
	.expect_len = sizeof(remove_device_param_3),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data remove_device_success_1 = {
	.send_opcode = MGMT_OP_REMOVE_DEVICE,
	.send_param = remove_device_param_1,
	.send_len = sizeof(remove_device_param_1),
	.expect_param = remove_device_param_1,
	.expect_len = sizeof(remove_device_param_1),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_DEVICE_REMOVED,
	.expect_alt_ev_param = remove_device_param_1,
	.expect_alt_ev_len = sizeof(remove_device_param_1),
};

static const struct generic_data remove_device_success_2 = {
	.send_opcode = MGMT_OP_REMOVE_DEVICE,
	.send_param = remove_device_param_1,
	.send_len = sizeof(remove_device_param_1),
	.expect_param = remove_device_param_1,
	.expect_len = sizeof(remove_device_param_1),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_DEVICE_REMOVED,
	.expect_alt_ev_param = remove_device_param_1,
	.expect_alt_ev_len = sizeof(remove_device_param_1),
	.expect_hci_command = BT_HCI_CMD_WRITE_SCAN_ENABLE,
	.expect_hci_param = set_connectable_off_scan_enable_param,
	.expect_hci_len = sizeof(set_connectable_off_scan_enable_param),
};

static const struct generic_data remove_device_success_3 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_REMOVE_DEVICE,
	.send_param = remove_device_param_1,
	.send_len = sizeof(remove_device_param_1),
	.expect_param = remove_device_param_1,
	.expect_len = sizeof(remove_device_param_1),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_DEVICE_REMOVED,
	.expect_alt_ev_param = remove_device_param_1,
	.expect_alt_ev_len = sizeof(remove_device_param_1),
};

static const uint8_t remove_device_param_2[] =  {
					0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc,
					0x01,
};
static const struct generic_data remove_device_success_4 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_REMOVE_DEVICE,
	.send_param = remove_device_param_2,
	.send_len = sizeof(remove_device_param_2),
	.expect_param = remove_device_param_2,
	.expect_len = sizeof(remove_device_param_2),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_DEVICE_REMOVED,
	.expect_alt_ev_param = remove_device_param_2,
	.expect_alt_ev_len = sizeof(remove_device_param_2),
};

static const struct generic_data remove_device_success_5 = {
	.send_opcode = MGMT_OP_REMOVE_DEVICE,
	.send_param = remove_device_param_2,
	.send_len = sizeof(remove_device_param_2),
	.expect_param = remove_device_param_2,
	.expect_len = sizeof(remove_device_param_2),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_DEVICE_REMOVED,
	.expect_alt_ev_param = remove_device_param_2,
	.expect_alt_ev_len = sizeof(remove_device_param_2),
};

static const struct generic_data read_adv_features_invalid_param_test = {
	.send_opcode = MGMT_OP_READ_ADV_FEATURES,
	.send_param = dummy_data,
	.send_len = sizeof(dummy_data),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data read_adv_features_invalid_index_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_READ_ADV_FEATURES,
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static const uint8_t read_adv_features_rsp_1[] =  {
	0x1f, 0x00, 0x00, 0x00,	/* supported flags */
	0x1f,			/* max_adv_data_len */
	0x1f,			/* max_scan_rsp_len */
	0x05,			/* max_instances */
	0x00,			/* num_instances */
};

static const struct generic_data read_adv_features_success_1 = {
	.send_opcode = MGMT_OP_READ_ADV_FEATURES,
	.expect_param = read_adv_features_rsp_1,
	.expect_len = sizeof(read_adv_features_rsp_1),
	.expect_status = MGMT_STATUS_SUCCESS,
};

static const uint8_t read_adv_features_rsp_2[] =  {
	0x1f, 0x00, 0x00, 0x00,	/* supported flags */
	0x1f,			/* max_adv_data_len */
	0x1f,			/* max_scan_rsp_len */
	0x05,			/* max_instances */
	0x01,			/* num_instances */
	0x01,			/* instance identifiers */
};

static const struct generic_data read_adv_features_success_2 = {
	.send_opcode = MGMT_OP_READ_ADV_FEATURES,
	.expect_param = read_adv_features_rsp_2,
	.expect_len = sizeof(read_adv_features_rsp_2),
	.expect_status = MGMT_STATUS_SUCCESS,
};

/* simple add advertising command */
static const uint8_t add_advertising_param_uuid[] = {
	0x01,			/* adv instance */
	0x00, 0x00, 0x00, 0x00,	/* flags: none */
	0x00, 0x00,		/* duration: default */
	0x00, 0x00,		/* timeout: none */
	0x09,			/* adv data len */
	0x00,			/* scan rsp len */
	/* adv data: */
	0x03,			/* AD len */
	0x02,			/* AD type: some 16 bit service class UUIDs */
	0x0d, 0x18,		/* heart rate monitor */
	0x04,			/* AD len */
	0xff,			/* AD type: manufacturer specific data */
	0x01, 0x02, 0x03,	/* custom advertising data */
};

/* add advertising with scan response data */
static const uint8_t add_advertising_param_scanrsp[] = {
	/* instance, flags, duration, timeout, adv data len: same as before */
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09,
	0x0a,			/* scan rsp len */
	/* adv data: same as before */
	0x03, 0x02, 0x0d, 0x18, 0x04, 0xff, 0x01, 0x02, 0x03,
	/* scan rsp data: */
	0x03,			/* AD len */
	0x19,			/* AD type: external appearance */
	0x40, 0x03,		/* some custom appearance */
	0x05,			/* AD len */
	0x03,			/* AD type: all 16 bit service class UUIDs */
	0x0d, 0x18,		/* heart rate monitor */
	0x0f, 0x18,		/* battery service */
};

/* add advertising with timeout */
static const uint8_t add_advertising_param_timeout[] = {
	/* instance, flags, duration: same as before */
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x05, 0x00,		/* timeout: 5 seconds */
	/* adv data: same as before */
	0x09, 0x00, 0x03, 0x02, 0x0d, 0x18, 0x04, 0xff, 0x01, 0x02, 0x03,
};

/* add advertising with connectable flag */
static const uint8_t add_advertising_param_connectable[] = {
	0x01,			/* adv instance */
	0x01, 0x00, 0x00, 0x00,	/* flags: connectable*/
	/* duration, timeout, adv/scan data: same as before */
	0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
	0x03, 0x02, 0x0d, 0x18, 0x04, 0xff, 0x01, 0x02, 0x03,
};

/* add advertising with general discoverable flag */
static const uint8_t add_advertising_param_general_discov[] = {
	0x01,			/* adv instance */
	0x02, 0x00, 0x00, 0x00,	/* flags: general discoverable*/
	/* duration, timeout, adv/scan data: same as before */
	0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
	0x03, 0x02, 0x0d, 0x18, 0x04, 0xff, 0x01, 0x02, 0x03,
};

/* add advertising with limited discoverable flag */
static const uint8_t add_advertising_param_limited_discov[] = {
	0x01,			/* adv instance */
	0x04, 0x00, 0x00, 0x00,	/* flags: limited discoverable */
	/* duration, timeout, adv/scan data: same as before */
	0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
	0x03, 0x02, 0x0d, 0x18, 0x04, 0xff, 0x01, 0x02, 0x03,
};

/* add advertising with managed flags */
static const uint8_t add_advertising_param_managed[] = {
	0x01,			/* adv instance */
	0x08, 0x00, 0x00, 0x00,	/* flags: managed flags */
	/* duration, timeout, adv/scan data: same as before */
	0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
	0x03, 0x02, 0x0d, 0x18, 0x04, 0xff, 0x01, 0x02, 0x03,
};

/* add advertising with tx power flag */
static const uint8_t add_advertising_param_txpwr[] = {
	0x01,			/* adv instance */
	0x10, 0x00, 0x00, 0x00,	/* flags: tx power */
	/* duration, timeout, adv/scan data: same as before */
	0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
	0x03, 0x02, 0x0d, 0x18, 0x04, 0xff, 0x01, 0x02, 0x03,
};

/* add advertising command for a second instance */
static const uint8_t add_advertising_param_test2[] = {
	0x02,				/* adv instance */
	0x00, 0x00, 0x00, 0x00,		/* flags: none */
	0x00, 0x00,			/* duration: default */
	0x01, 0x00,			/* timeout: 1 second */
	0x07,				/* adv data len */
	0x00,				/* scan rsp len */
	/* adv data: */
	0x06,				/* AD len */
	0x08,				/* AD type: shortened local name */
	0x74, 0x65, 0x73, 0x74, 0x32,	/* "test2" */
};

static const uint8_t advertising_instance1_param[] = {
	0x01,
};

static const uint8_t advertising_instance2_param[] = {
	0x02,
};

static const uint8_t set_adv_data_uuid[] = {
	/* adv data len */
	0x09,
	/* advertise heart rate monitor and manufacturer specific data */
	0x03, 0x02, 0x0d, 0x18, 0x04, 0xff, 0x01, 0x02, 0x03,
	/* padding */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,
};

static const uint8_t set_adv_data_test1[] = {
	0x07,				/* adv data len */
	0x06,				/* AD len */
	0x08,				/* AD type: shortened local name */
	0x74, 0x65, 0x73, 0x74, 0x31,	/* "test1" */
	/* padding */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
};

static const uint8_t set_adv_data_test2[] = {
	0x07,				/* adv data len */
	0x06,				/* AD len */
	0x08,				/* AD type: shortened local name */
	0x74, 0x65, 0x73, 0x74, 0x32,	/* "test2" */
	/* padding */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
};

static const uint8_t set_adv_data_txpwr[] = {
	0x03,			/* adv data len */
	0x02, 			/* AD len */
	0x0a,			/* AD type: tx power */
	0x00,			/* tx power */
	/* padding */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t set_adv_data_general_discov[] = {
	0x0c,			/* adv data len */
	0x02,			/* AD len */
	0x01,			/* AD type: flags */
	0x02,			/* general discoverable */
	0x03,			/* AD len */
	0x02,			/* AD type: some 16bit service class UUIDs */
	0x0d, 0x18,		/* heart rate monitor */
	0x04,			/* AD len */
	0xff,			/* AD type: manufacturer specific data */
	0x01, 0x02, 0x03,	/* custom advertising data */
	/* padding */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t set_adv_data_limited_discov[] = {
	0x0c,			/* adv data len */
	0x02,			/* AD len */
	0x01,			/* AD type: flags */
	0x01,			/* limited discoverable */
	/* rest: same as before */
	0x03, 0x02, 0x0d, 0x18, 0x04, 0xff, 0x01, 0x02, 0x03,
	/* padding */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t set_adv_data_uuid_txpwr[] = {
	0x0c,			/* adv data len */
	0x03,			/* AD len */
	0x02,			/* AD type: some 16bit service class UUIDs */
	0x0d, 0x18,		/* heart rate monitor */
	0x04,			/* AD len */
	0xff,			/* AD type: manufacturer specific data */
	0x01, 0x02, 0x03,	/* custom advertising data */
	0x02,			/* AD len */
	0x0a,			/* AD type: tx power */
	0x00,			/* tx power */
	/* padding */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t set_scan_rsp_uuid[] = {
	0x0a,			/* scan rsp data len */
	0x03,			/* AD len */
	0x19,			/* AD type: external appearance */
	0x40, 0x03,		/* some custom appearance */
	0x05,			/* AD len */
	0x03,			/* AD type: all 16 bit service class UUIDs */
	0x0d, 0x18, 0x0f, 0x18,	/* heart rate monitor, battery service */
	/* padding */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
};

static const uint8_t add_advertising_invalid_param_1[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00,
	0x03, 0x03, 0x0d, 0x18,
	0x19, 0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
	0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e,
};

static const uint8_t add_advertising_invalid_param_2[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
	0x04, 0x03, 0x0d, 0x18,
	0x04, 0xff, 0x01, 0x02, 0x03,
};

static const uint8_t add_advertising_invalid_param_3[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
	0x03, 0x03, 0x0d, 0x18,
	0x02, 0xff, 0x01, 0x02, 0x03,
};

static const uint8_t add_advertising_invalid_param_4[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00,
	0x03, 0x03, 0x0d, 0x18,
	0x05, 0xff, 0x01, 0x02, 0x03,
};

static const uint8_t add_advertising_invalid_param_5[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D, 0x00,
	0x03, 0x03, 0x0d, 0x18,
	0x19, 0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
	0x15, 0x16, 0x17, 0x18,
};

static const uint8_t add_advertising_invalid_param_6[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
	0x03, 0x03, 0x0d, 0x18,
	0x19, 0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
	0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e,
};

static const uint8_t add_advertising_invalid_param_7[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09,
	0x04, 0x03, 0x0d, 0x18,
	0x04, 0xff, 0x01, 0x02, 0x03,
};

static const uint8_t add_advertising_invalid_param_8[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09,
	0x03, 0x03, 0x0d, 0x18,
	0x02, 0xff, 0x01, 0x02, 0x03,
};

static const uint8_t add_advertising_invalid_param_9[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09,
	0x03, 0x03, 0x0d, 0x18,
	0x05, 0xff, 0x01, 0x02, 0x03,
};

static const uint8_t add_advertising_invalid_param_10[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D,
	0x03, 0x03, 0x0d, 0x18,
	0x19, 0xff, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
	0x15, 0x16, 0x17, 0x18,
};

static const struct generic_data add_advertising_fail_1 = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_uuid,
	.send_len = sizeof(add_advertising_param_uuid),
	.expect_status = MGMT_STATUS_REJECTED,
};

static const struct generic_data add_advertising_fail_2 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_invalid_param_1,
	.send_len = sizeof(add_advertising_invalid_param_1),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data add_advertising_fail_3 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_invalid_param_2,
	.send_len = sizeof(add_advertising_invalid_param_2),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data add_advertising_fail_4 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_invalid_param_3,
	.send_len = sizeof(add_advertising_invalid_param_3),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data add_advertising_fail_5 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_invalid_param_4,
	.send_len = sizeof(add_advertising_invalid_param_4),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data add_advertising_fail_6 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_invalid_param_5,
	.send_len = sizeof(add_advertising_invalid_param_5),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data add_advertising_fail_7 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_invalid_param_6,
	.send_len = sizeof(add_advertising_invalid_param_6),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data add_advertising_fail_8 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_invalid_param_7,
	.send_len = sizeof(add_advertising_invalid_param_7),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data add_advertising_fail_9 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_invalid_param_8,
	.send_len = sizeof(add_advertising_invalid_param_8),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data add_advertising_fail_10 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_invalid_param_9,
	.send_len = sizeof(add_advertising_invalid_param_9),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data add_advertising_fail_11 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_invalid_param_10,
	.send_len = sizeof(add_advertising_invalid_param_10),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data add_advertising_fail_12 = {
	.setup_settings = settings_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_timeout,
	.send_len = sizeof(add_advertising_param_timeout),
	.expect_status = MGMT_STATUS_REJECTED,
};

static const struct generic_data add_advertising_success_1 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_uuid,
	.send_len = sizeof(add_advertising_param_uuid),
	.expect_param = advertising_instance1_param,
	.expect_len = sizeof(advertising_instance1_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_ADVERTISING_ADDED,
	.expect_alt_ev_param = advertising_instance1_param,
	.expect_alt_ev_len = sizeof(advertising_instance1_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_adv_data_uuid,
	.expect_hci_len = sizeof(set_adv_data_uuid),
};

static const char set_powered_adv_instance_settings_param[] = {
	0x81, 0x02, 0x00, 0x00,
};

static const struct generic_data add_advertising_success_pwron_data = {
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_on_param,
	.send_len = sizeof(set_powered_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_powered_adv_instance_settings_param,
	.expect_len = sizeof(set_powered_adv_instance_settings_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_adv_data_test1,
	.expect_hci_len = sizeof(set_adv_data_test1),
};

static const struct generic_data add_advertising_success_pwron_enabled = {
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_on_param,
	.send_len = sizeof(set_powered_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_powered_adv_instance_settings_param,
	.expect_len = sizeof(set_powered_adv_instance_settings_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_ENABLE,
	.expect_hci_param = set_adv_on_set_adv_enable_param,
	.expect_hci_len = sizeof(set_adv_on_set_adv_enable_param),
};

static const struct generic_data add_advertising_success_4 = {
	.send_opcode = MGMT_OP_SET_ADVERTISING,
	.send_param = set_adv_on_param,
	.send_len = sizeof(set_adv_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_adv_settings_param_2,
	.expect_len = sizeof(set_adv_settings_param_2),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_adv_data_txpwr,
	.expect_hci_len = sizeof(set_adv_data_txpwr),
};

static const struct generic_data add_advertising_success_5 = {
	.send_opcode = MGMT_OP_SET_ADVERTISING,
	.send_param = set_adv_off_param,
	.send_len = sizeof(set_adv_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_powered_adv_instance_settings_param,
	.expect_len = sizeof(set_powered_adv_instance_settings_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_adv_data_test1,
	.expect_hci_len = sizeof(set_adv_data_test1),
};

static const struct generic_data add_advertising_success_6 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_scanrsp,
	.send_len = sizeof(add_advertising_param_scanrsp),
	.expect_param = advertising_instance1_param,
	.expect_len = sizeof(advertising_instance1_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_ADVERTISING_ADDED,
	.expect_alt_ev_param = advertising_instance1_param,
	.expect_alt_ev_len = sizeof(advertising_instance1_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_adv_data_uuid,
	.expect_hci_len = sizeof(set_adv_data_uuid),
};

static const struct generic_data add_advertising_success_7 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_scanrsp,
	.send_len = sizeof(add_advertising_param_scanrsp),
	.expect_param = advertising_instance1_param,
	.expect_len = sizeof(advertising_instance1_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_ADVERTISING_ADDED,
	.expect_alt_ev_param = advertising_instance1_param,
	.expect_alt_ev_len = sizeof(advertising_instance1_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_SCAN_RSP_DATA,
	.expect_hci_param = set_scan_rsp_uuid,
	.expect_hci_len = sizeof(set_scan_rsp_uuid),
};

static const struct generic_data add_advertising_success_8 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_connectable,
	.send_len = sizeof(add_advertising_param_connectable),
	.expect_param = advertising_instance1_param,
	.expect_len = sizeof(advertising_instance1_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
	.expect_hci_param = set_connectable_on_adv_param,
	.expect_hci_len = sizeof(set_connectable_on_adv_param),
};

static const struct generic_data add_advertising_success_9 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_general_discov,
	.send_len = sizeof(add_advertising_param_general_discov),
	.expect_param = advertising_instance1_param,
	.expect_len = sizeof(advertising_instance1_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_adv_data_general_discov,
	.expect_hci_len = sizeof(set_adv_data_general_discov),
};

static const struct generic_data add_advertising_success_10 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_limited_discov,
	.send_len = sizeof(add_advertising_param_limited_discov),
	.expect_param = advertising_instance1_param,
	.expect_len = sizeof(advertising_instance1_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_adv_data_limited_discov,
	.expect_hci_len = sizeof(set_adv_data_limited_discov),
};

static const struct generic_data add_advertising_success_11 = {
	.setup_settings = settings_powered_le_discoverable,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_managed,
	.send_len = sizeof(add_advertising_param_managed),
	.expect_param = advertising_instance1_param,
	.expect_len = sizeof(advertising_instance1_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_adv_data_general_discov,
	.expect_hci_len = sizeof(set_adv_data_general_discov),
};

static const struct generic_data add_advertising_success_12 = {
	.setup_settings = settings_powered_le_discoverable,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_txpwr,
	.send_len = sizeof(add_advertising_param_txpwr),
	.expect_param = advertising_instance1_param,
	.expect_len = sizeof(advertising_instance1_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_adv_data_uuid_txpwr,
	.expect_hci_len = sizeof(set_adv_data_uuid_txpwr),
};

static uint16_t settings_powered_le_connectable[] = {
						MGMT_OP_SET_POWERED,
						MGMT_OP_SET_LE,
						MGMT_OP_SET_CONNECTABLE, 0 };

static uint8_t set_connectable_off_scan_adv_param[] = {
		0x00, 0x08,				/* min_interval */
		0x00, 0x08,				/* max_interval */
		0x02,					/* type */
		0x01,					/* own_addr_type */
		0x00,					/* direct_addr_type */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* direct_addr */
		0x07,					/* channel_map */
		0x00,					/* filter_policy */
};

static const struct generic_data add_advertising_success_13 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_scanrsp,
	.send_len = sizeof(add_advertising_param_scanrsp),
	.expect_param = advertising_instance1_param,
	.expect_len = sizeof(advertising_instance1_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
	.expect_hci_param = set_connectable_off_scan_adv_param,
	.expect_hci_len = sizeof(set_connectable_off_scan_adv_param),
};

static const struct generic_data add_advertising_success_14 = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_uuid,
	.send_len = sizeof(add_advertising_param_uuid),
	.expect_param = advertising_instance1_param,
	.expect_len = sizeof(advertising_instance1_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
	.expect_hci_param = set_connectable_off_adv_param,
	.expect_hci_len = sizeof(set_connectable_off_adv_param),
};

static const struct generic_data add_advertising_success_15 = {
	.setup_settings = settings_powered_le_connectable,
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_uuid,
	.send_len = sizeof(add_advertising_param_uuid),
	.expect_param = advertising_instance1_param,
	.expect_len = sizeof(advertising_instance1_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
	.expect_hci_param = set_connectable_on_adv_param,
	.expect_hci_len = sizeof(set_connectable_on_adv_param),
};

static const char set_connectable_settings_param_3[] = {
						0x83, 0x02, 0x00, 0x00 };

static const struct generic_data add_advertising_success_16 = {
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_on_param,
	.send_len = sizeof(set_connectable_on_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_connectable_settings_param_3,
	.expect_len = sizeof(set_connectable_settings_param_3),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
	.expect_hci_param = set_connectable_on_adv_param,
	.expect_hci_len = sizeof(set_connectable_on_adv_param),
};

static const struct generic_data add_advertising_success_17 = {
	.send_opcode = MGMT_OP_SET_CONNECTABLE,
	.send_param = set_connectable_off_param,
	.send_len = sizeof(set_connectable_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_le_settings_param_2,
	.expect_len = sizeof(set_le_settings_param_2),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
	.expect_hci_param = set_connectable_off_adv_param,
	.expect_hci_len = sizeof(set_connectable_off_adv_param),
};

static const char set_powered_off_le_settings_param[] = {
	0x80, 0x02, 0x00, 0x00
};

static const struct generic_data add_advertising_power_off = {
	.send_opcode = MGMT_OP_SET_POWERED,
	.send_param = set_powered_off_param,
	.send_len = sizeof(set_powered_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_powered_off_le_settings_param,
	.expect_len = sizeof(set_powered_off_le_settings_param),
	.expect_alt_ev = MGMT_EV_ADVERTISING_REMOVED,
	.expect_alt_ev_param = advertising_instance1_param,
	.expect_alt_ev_len = sizeof(advertising_instance1_param),
};

static const char set_le_settings_param_off[] = { 0x81, 0x00, 0x00, 0x00 };

static const struct generic_data add_advertising_le_off = {
	.send_opcode = MGMT_OP_SET_LE,
	.send_param = set_le_off_param,
	.send_len = sizeof(set_le_off_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = set_le_settings_param_off,
	.expect_len = sizeof(set_le_settings_param_off),
	.expect_alt_ev = MGMT_EV_ADVERTISING_REMOVED,
	.expect_alt_ev_param = advertising_instance1_param,
	.expect_alt_ev_len = sizeof(advertising_instance1_param),
};

static const struct generic_data add_advertising_success_18 = {
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_uuid,
	.send_len = sizeof(add_advertising_param_uuid),
	.expect_param = advertising_instance1_param,
	.expect_len = sizeof(advertising_instance1_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_adv_data_uuid,
	.expect_hci_len = sizeof(set_adv_data_uuid),
};

static const struct generic_data add_advertising_timeout_expired = {
	.expect_alt_ev = MGMT_EV_ADVERTISING_REMOVED,
	.expect_alt_ev_param = advertising_instance1_param,
	.expect_alt_ev_len = sizeof(advertising_instance1_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_ENABLE,
	.expect_hci_param = set_adv_on_set_adv_disable_param,
	.expect_hci_len = sizeof(set_adv_on_set_adv_disable_param),
};

static const uint8_t remove_advertising_param_1[] = {
	0x01,
};

static const uint8_t remove_advertising_param_2[] = {
	0x00,
};

static const struct generic_data remove_advertising_fail_1 = {
	.send_opcode = MGMT_OP_REMOVE_ADVERTISING,
	.send_param = remove_advertising_param_1,
	.send_len = sizeof(remove_advertising_param_1),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data remove_advertising_success_1 = {
	.send_opcode = MGMT_OP_REMOVE_ADVERTISING,
	.send_param = remove_advertising_param_1,
	.send_len = sizeof(remove_advertising_param_1),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = remove_advertising_param_1,
	.expect_len = sizeof(remove_advertising_param_1),
	.expect_alt_ev = MGMT_EV_ADVERTISING_REMOVED,
	.expect_alt_ev_param = advertising_instance1_param,
	.expect_alt_ev_len = sizeof(advertising_instance1_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_ENABLE,
	.expect_hci_param = set_adv_off_param,
	.expect_hci_len = sizeof(set_adv_off_param),
};

static const struct generic_data remove_advertising_success_2 = {
	.send_opcode = MGMT_OP_REMOVE_ADVERTISING,
	.send_param = remove_advertising_param_2,
	.send_len = sizeof(remove_advertising_param_2),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = remove_advertising_param_2,
	.expect_len = sizeof(remove_advertising_param_2),
	.expect_alt_ev = MGMT_EV_ADVERTISING_REMOVED,
	.expect_alt_ev_param = advertising_instance1_param,
	.expect_alt_ev_len = sizeof(advertising_instance1_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_ENABLE,
	.expect_hci_param = set_adv_off_param,
	.expect_hci_len = sizeof(set_adv_off_param),
};

static const struct generic_data multi_advertising_switch = {
	.expect_alt_ev = MGMT_EV_ADVERTISING_REMOVED,
	.expect_alt_ev_param = advertising_instance1_param,
	.expect_alt_ev_len = sizeof(advertising_instance1_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_adv_data_test2,
	.expect_hci_len = sizeof(set_adv_data_test2),
};

static const struct generic_data multi_advertising_add_second = {
	.send_opcode = MGMT_OP_ADD_ADVERTISING,
	.send_param = add_advertising_param_test2,
	.send_len = sizeof(add_advertising_param_test2),
	.expect_param = advertising_instance2_param,
	.expect_len = sizeof(advertising_instance2_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_alt_ev = MGMT_EV_ADVERTISING_ADDED,
	.expect_alt_ev_param = advertising_instance2_param,
	.expect_alt_ev_len = sizeof(advertising_instance2_param),
	.expect_hci_command = BT_HCI_CMD_LE_SET_ADV_DATA,
	.expect_hci_param = set_adv_data_test2,
	.expect_hci_len = sizeof(set_adv_data_test2),
};

/* based on G-Tag ADV_DATA */
static const uint8_t adv_data_invalid_significant_len[] = { 0x02, 0x01, 0x06,
		0x0d, 0xff, 0x80, 0x01, 0x02, 0x15, 0x12, 0x34, 0x80, 0x91,
		0xd0, 0xf2, 0xbb, 0xc5, 0x03, 0x02, 0x0f, 0x18, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const char device_found_valid[] = { 0x00, 0x00, 0x01, 0x01, 0xaa, 0x00,
		0x01, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x02, 0x01,
		0x06, 0x0d, 0xff, 0x80, 0x01, 0x02, 0x15, 0x12, 0x34, 0x80,
		0x91, 0xd0, 0xf2, 0xbb, 0xc5, 0x03, 0x02, 0x0f, 0x18 };

static const struct generic_data device_found_gtag = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_START_DISCOVERY,
	.send_param = start_discovery_le_param,
	.send_len = sizeof(start_discovery_le_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = start_discovery_le_param,
	.expect_len = sizeof(start_discovery_le_param),
	.expect_alt_ev = MGMT_EV_DEVICE_FOUND,
	.expect_alt_ev_param = device_found_valid,
	.expect_alt_ev_len = sizeof(device_found_valid),
	.set_adv = true,
	.adv_data_len = sizeof(adv_data_invalid_significant_len),
	.adv_data = adv_data_invalid_significant_len,
};

static const uint8_t adv_data_invalid_field_len[] = { 0x02, 0x01, 0x01,
		0x05, 0x09, 0x74, 0x65, 0x73, 0x74,
		0xa0, 0xff, 0x01, 0x02, 0x03, 0x04, 0x05};

static const char device_found_valid2[] = { 0x00, 0x00, 0x01, 0x01, 0xaa, 0x00,
		0x01, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x09, 0x00, 0x02, 0x01,
		0x01, 0x05, 0x09, 0x74, 0x65, 0x73, 0x74};

static const struct generic_data device_found_invalid_field = {
	.setup_settings = settings_powered_le,
	.send_opcode = MGMT_OP_START_DISCOVERY,
	.send_param = start_discovery_le_param,
	.send_len = sizeof(start_discovery_le_param),
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_param = start_discovery_le_param,
	.expect_len = sizeof(start_discovery_le_param),
	.expect_alt_ev = MGMT_EV_DEVICE_FOUND,
	.expect_alt_ev_param = device_found_valid2,
	.expect_alt_ev_len = sizeof(device_found_valid2),
	.set_adv = true,
	.adv_data_len = sizeof(adv_data_invalid_field_len),
	.adv_data = adv_data_invalid_field_len,
};

static const struct generic_data read_local_oob_not_powered_test = {
	.send_opcode = MGMT_OP_READ_LOCAL_OOB_DATA,
	.expect_status = MGMT_STATUS_NOT_POWERED,
};

static const struct generic_data read_local_oob_invalid_param_test = {
	.send_opcode = MGMT_OP_READ_LOCAL_OOB_DATA,
	.send_param = dummy_data,
	.send_len = sizeof(dummy_data),
	.expect_status = MGMT_STATUS_INVALID_PARAMS,
};

static const struct generic_data read_local_oob_invalid_index_test = {
	.send_index_none = true,
	.send_opcode = MGMT_OP_READ_LOCAL_OOB_DATA,
	.expect_status = MGMT_STATUS_INVALID_INDEX,
};

static const struct generic_data read_local_oob_legacy_pairing_test = {
	.setup_settings = settings_powered,
	.send_opcode = MGMT_OP_READ_LOCAL_OOB_DATA,
	.expect_status = MGMT_STATUS_NOT_SUPPORTED,
};

static const struct generic_data read_local_oob_success_ssp_test = {
	.setup_settings = settings_powered_ssp,
	.send_opcode = MGMT_OP_READ_LOCAL_OOB_DATA,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_ignore_param = true,
	.expect_hci_command = BT_HCI_CMD_READ_LOCAL_OOB_DATA,
};

static const struct generic_data read_local_oob_success_sc_test = {
	.setup_settings = settings_powered_sc,
	.send_opcode = MGMT_OP_READ_LOCAL_OOB_DATA,
	.expect_status = MGMT_STATUS_SUCCESS,
	.expect_ignore_param = true,
	.expect_hci_command = BT_HCI_CMD_READ_LOCAL_OOB_EXT_DATA,
};

static void client_cmd_complete(uint16_t opcode, uint8_t status,
					const void *param, uint8_t len,
					void *user_data)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	struct bthost *bthost;

	bthost = hciemu_client_get_host(data->hciemu);

	switch (opcode) {
	case BT_HCI_CMD_WRITE_SCAN_ENABLE:
	case BT_HCI_CMD_LE_SET_ADV_ENABLE:
		tester_print("Client set connectable: %s (0x%02x)",
						mgmt_errstr(status), status);
		if (!status && test->client_enable_ssp) {
			bthost_write_ssp_mode(bthost, 0x01);
			return;
		}
		break;
	case BT_HCI_CMD_WRITE_SIMPLE_PAIRING_MODE:
		tester_print("Client enable SSP: %s (0x%02x)",
						mgmt_errstr(status), status);
		break;
	default:
		return;
	}

	if (status)
		tester_setup_failed();
	else
		tester_setup_complete();
}

static void setup_bthost(void)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	struct bthost *bthost;

	bthost = hciemu_client_get_host(data->hciemu);
	bthost_set_cmd_complete_cb(bthost, client_cmd_complete, data);
	if (data->hciemu_type == HCIEMU_TYPE_LE || test->client_enable_adv)
		bthost_set_adv_enable(bthost, 0x01);
	else
		bthost_write_scan_enable(bthost, 0x03);
}

static void setup_pairing_acceptor(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;

	if (!test->io_cap)
		return;

	mgmt_send(data->mgmt, MGMT_OP_SET_IO_CAPABILITY, data->mgmt_index,
					sizeof(test->io_cap), &test->io_cap,
					NULL, NULL, NULL);

	setup_bthost();
}

static void setup_powered_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	if (status != MGMT_STATUS_SUCCESS) {
		tester_setup_failed();
		return;
	}

	tester_print("Controller powered on");

	setup_bthost();
}

static void setup_class(const void *test_data)
{
	struct test_data *data = tester_get_data();
	unsigned char param[] = { 0x01 };
	unsigned char class_param[] = { 0x01, 0x0c };

	tester_print("Setting device class and powering on");

	mgmt_send(data->mgmt, MGMT_OP_SET_DEV_CLASS, data->mgmt_index,
				sizeof(class_param), class_param,
				NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
					sizeof(param), param,
					setup_powered_callback, NULL, NULL);
}

static void discovering_event(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();
	const struct mgmt_ev_discovering *ev = param;

	mgmt_unregister(data->mgmt, data->mgmt_discov_ev_id);

	if (length != sizeof(*ev)) {
		tester_warn("Incorrect discovering event length");
		tester_setup_failed();
		return;
	}

	if (!ev->discovering) {
		tester_warn("Unexpected discovery stopped event");
		tester_setup_failed();
		return;
	}

	tester_setup_complete();
}

static void setup_discovery_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	if (status != MGMT_STATUS_SUCCESS) {
		tester_setup_failed();
		return;
	}

	tester_print("Discovery started");
}

static void setup_start_discovery(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	const void *send_param = test->setup_send_param;
	uint16_t send_len = test->setup_send_len;
	unsigned int id;

	id = mgmt_register(data->mgmt, MGMT_EV_DISCOVERING, data->mgmt_index,
			   discovering_event, NULL, NULL);
	data->mgmt_discov_ev_id = id;

	mgmt_send(data->mgmt, test->setup_send_opcode, data->mgmt_index,
				send_len, send_param, setup_discovery_callback,
				NULL, NULL);
}

static void setup_multi_uuid32(const void *test_data)
{
	struct test_data *data = tester_get_data();
	unsigned char param[] = { 0x01 };

	tester_print("Powering on controller (with 32-bit UUID)");

	mgmt_send(data->mgmt, MGMT_OP_SET_SSP, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
				sizeof(add_uuid32_param_1), add_uuid32_param_1,
				NULL, NULL, NULL);
	mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
				sizeof(add_uuid32_param_2), add_uuid32_param_2,
				NULL, NULL, NULL);
	mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
				sizeof(add_uuid32_param_3), add_uuid32_param_3,
				NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
					sizeof(param), param,
					setup_powered_callback, NULL, NULL);
}

static void setup_multi_uuid32_2(const void *test_data)
{
	struct test_data *data = tester_get_data();
	unsigned char param[] = { 0x01 };
	unsigned char uuid_param[] = {
			0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00 };
	int i;

	tester_print("Powering on controller (with many 32-bit UUIDs)");

	mgmt_send(data->mgmt, MGMT_OP_SET_SSP, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);

	for (i = 0; i < 58; i++) {
		uint32_t val = htobl(0xffffffff - i);
		memcpy(&uuid_param[12], &val, sizeof(val));
		mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
				sizeof(uuid_param), uuid_param,
				NULL, NULL, NULL);
	}

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
					sizeof(param), param,
					setup_powered_callback, NULL, NULL);
}

static void setup_multi_uuid128(const void *test_data)
{
	struct test_data *data = tester_get_data();
	unsigned char param[] = { 0x01 };

	tester_print("Powering on controller (with 128-bit UUID)");

	mgmt_send(data->mgmt, MGMT_OP_SET_SSP, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
			sizeof(add_uuid128_param_1), add_uuid128_param_1,
			NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
					sizeof(param), param,
					setup_powered_callback, NULL, NULL);
}

static void setup_multi_uuid128_2(const void *test_data)
{
	struct test_data *data = tester_get_data();
	unsigned char param[] = { 0x01 };
	unsigned char uuid_param[] = {
			0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
			0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00,
			0x00 };
	int i;

	tester_print("Powering on controller (with many 128-bit UUIDs)");

	mgmt_send(data->mgmt, MGMT_OP_SET_SSP, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);

	for (i = 0; i < 13; i++) {
		uuid_param[15] = i;
		mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
				sizeof(uuid_param), uuid_param,
				NULL, NULL, NULL);
	}

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
					sizeof(param), param,
					setup_powered_callback, NULL, NULL);
}

static void setup_multi_uuid16(const void *test_data)
{
	struct test_data *data = tester_get_data();
	unsigned char param[] = { 0x01 };

	tester_print("Powering on controller (with SPP UUID)");

	mgmt_send(data->mgmt, MGMT_OP_SET_SSP, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
				sizeof(add_spp_uuid_param), add_spp_uuid_param,
				NULL, NULL, NULL);
	mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
				sizeof(add_dun_uuid_param), add_dun_uuid_param,
				NULL, NULL, NULL);
	mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
			sizeof(add_sync_uuid_param), add_sync_uuid_param,
			NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
					sizeof(param), param,
					setup_powered_callback, NULL, NULL);
}

static void setup_multi_uuid16_2(const void *test_data)
{
	struct test_data *data = tester_get_data();
	unsigned char param[] = { 0x01 };
	unsigned char uuid_param[] = {
			0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00 };
	int i;

	tester_print("Powering on controller (with many 16-bit UUIDs)");

	mgmt_send(data->mgmt, MGMT_OP_SET_SSP, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);

	for (i = 0; i < 117; i++) {
		uint16_t val = htobs(i + 0x2000);
		memcpy(&uuid_param[12], &val, sizeof(val));
		mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
				sizeof(uuid_param), uuid_param,
				NULL, NULL, NULL);
	}

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
					sizeof(param), param,
					setup_powered_callback, NULL, NULL);
}

static void setup_uuid_mix(const void *test_data)
{
	struct test_data *data = tester_get_data();
	unsigned char param[] = { 0x01 };

	tester_print("Powering on controller (with mixed UUIDs)");

	mgmt_send(data->mgmt, MGMT_OP_SET_SSP, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
				sizeof(add_spp_uuid_param), add_spp_uuid_param,
				NULL, NULL, NULL);
	mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
				sizeof(add_uuid32_param_1), add_uuid32_param_1,
				NULL, NULL, NULL);
	mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
			sizeof(add_uuid128_param_1), add_uuid128_param_1,
			NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
				sizeof(add_dun_uuid_param), add_dun_uuid_param,
				NULL, NULL, NULL);
	mgmt_send(data->mgmt, MGMT_OP_ADD_UUID, data->mgmt_index,
				sizeof(add_uuid32_param_2), add_uuid32_param_2,
				NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
					sizeof(param), param,
					setup_powered_callback, NULL, NULL);
}

static void setup_add_device(const void *test_data)
{
	struct test_data *data = tester_get_data();
	unsigned char param[] = { 0x01 };
	const unsigned char *add_param;
	size_t add_param_len;

	tester_print("Powering on controller (with added device)");

	if (data->hciemu_type == HCIEMU_TYPE_LE) {
		add_param = add_device_success_param_2;
		add_param_len = sizeof(add_device_success_param_2);
	} else {
		add_param = add_device_success_param_1;
		add_param_len = sizeof(add_device_success_param_1);
	}

	mgmt_send(data->mgmt, MGMT_OP_ADD_DEVICE, data->mgmt_index,
			add_param_len, add_param, NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
					sizeof(param), param,
					setup_powered_callback, NULL, NULL);
}

static void setup_add_advertising_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct mgmt_rp_add_advertising *rp =
				(struct mgmt_rp_add_advertising *) param;

	if (status != MGMT_STATUS_SUCCESS) {
		tester_setup_failed();
		return;
	}

	tester_print("Add Advertising setup complete (instance %d)",
								rp->instance);

	setup_bthost();
}

#define TESTER_ADD_ADV_DATA_LEN 7

static void setup_add_adv_param(struct mgmt_cp_add_advertising *cp,
							uint8_t instance)
{
	memset(cp, 0, sizeof(*cp));
	cp->instance = instance;
	cp->adv_data_len = TESTER_ADD_ADV_DATA_LEN;
	cp->data[0] = TESTER_ADD_ADV_DATA_LEN - 1; /* AD len */
	cp->data[1] = 0x08; /* AD type: shortened local name */
	cp->data[2] = 't';  /* adv data ... */
	cp->data[3] = 'e';
	cp->data[4] = 's';
	cp->data[5] = 't';
	cp->data[6] = '0' + instance;
}

static void setup_add_advertising_not_powered(const void *test_data)
{
	struct test_data *data = tester_get_data();
	struct mgmt_cp_add_advertising *cp;
	unsigned char adv_param[sizeof(*cp) + TESTER_ADD_ADV_DATA_LEN];
	unsigned char param[] = { 0x01 };

	tester_print("Adding advertising instance while unpowered");

	cp = (struct mgmt_cp_add_advertising *) adv_param;
	setup_add_adv_param(cp, 1);

	mgmt_send(data->mgmt, MGMT_OP_SET_LE, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_ADD_ADVERTISING, data->mgmt_index,
						sizeof(adv_param), adv_param,
						setup_add_advertising_callback,
						NULL, NULL);
}

static void setup_add_advertising(const void *test_data)
{
	struct test_data *data = tester_get_data();
	struct mgmt_cp_add_advertising *cp;
	unsigned char adv_param[sizeof(*cp) + TESTER_ADD_ADV_DATA_LEN];
	unsigned char param[] = { 0x01 };

	tester_print("Adding advertising instance while powered");

	cp = (struct mgmt_cp_add_advertising *) adv_param;
	setup_add_adv_param(cp, 1);

	mgmt_send(data->mgmt, MGMT_OP_SET_LE, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_ADD_ADVERTISING, data->mgmt_index,
						sizeof(adv_param), adv_param,
						setup_add_advertising_callback,
						NULL, NULL);
}

static void setup_add_advertising_connectable(const void *test_data)
{
	struct test_data *data = tester_get_data();
	struct mgmt_cp_add_advertising *cp;
	unsigned char adv_param[sizeof(*cp) + TESTER_ADD_ADV_DATA_LEN];
	unsigned char param[] = { 0x01 };

	tester_print("Adding advertising instance while connectable");

	cp = (struct mgmt_cp_add_advertising *) adv_param;
	setup_add_adv_param(cp, 1);

	mgmt_send(data->mgmt, MGMT_OP_SET_LE, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_CONNECTABLE, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_ADD_ADVERTISING, data->mgmt_index,
						sizeof(adv_param), adv_param,
						setup_add_advertising_callback,
						NULL, NULL);
}

static void setup_add_advertising_timeout(const void *test_data)
{
	struct test_data *data = tester_get_data();
	struct mgmt_cp_add_advertising *cp;
	unsigned char adv_param[sizeof(*cp) + TESTER_ADD_ADV_DATA_LEN];
	unsigned char param[] = { 0x01 };

	tester_print("Adding advertising instance with timeout");

	cp = (struct mgmt_cp_add_advertising *) adv_param;
	setup_add_adv_param(cp, 1);
	cp->timeout = 1;

	mgmt_send(data->mgmt, MGMT_OP_SET_LE, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_ADD_ADVERTISING, data->mgmt_index,
						sizeof(adv_param), adv_param,
						setup_add_advertising_callback,
						NULL, NULL);
}

static void setup_add_advertising_duration(const void *test_data)
{
	struct test_data *data = tester_get_data();
	struct mgmt_cp_add_advertising *cp;
	unsigned char adv_param[sizeof(*cp) + TESTER_ADD_ADV_DATA_LEN];
	unsigned char param[] = { 0x01 };

	tester_print("Adding instance with long timeout/short duration");

	cp = (struct mgmt_cp_add_advertising *) adv_param;
	setup_add_adv_param(cp, 1);
	cp->duration = 1;
	cp->timeout = 30;

	mgmt_send(data->mgmt, MGMT_OP_SET_LE, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_ADD_ADVERTISING, data->mgmt_index,
						sizeof(adv_param), adv_param,
						setup_add_advertising_callback,
						NULL, NULL);
}

static void setup_power_cycle_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();
	unsigned char param_off[] = { 0x00 };

	if (status != MGMT_STATUS_SUCCESS) {
		tester_setup_failed();
		return;
	}

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
						sizeof(param_off), &param_off,
						NULL, NULL, NULL);

	setup_bthost();
}

static void setup_add_advertising_power_cycle(const void *test_data)
{
	struct test_data *data = tester_get_data();
	struct mgmt_cp_add_advertising *cp;
	unsigned char adv_param[sizeof(*cp) + TESTER_ADD_ADV_DATA_LEN];
	unsigned char param_on[] = { 0x01 };

	tester_print("Adding instance without timeout and power cycle");

	cp = (struct mgmt_cp_add_advertising *) adv_param;
	setup_add_adv_param(cp, 1);

	mgmt_send(data->mgmt, MGMT_OP_SET_LE, data->mgmt_index,
						sizeof(param_on), &param_on,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
						sizeof(param_on), &param_on,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_ADD_ADVERTISING, data->mgmt_index,
						sizeof(adv_param), adv_param,
						setup_power_cycle_callback,
						NULL, NULL);
}

static void setup_set_and_add_advertising(const void *test_data)
{
	struct test_data *data = tester_get_data();
	struct mgmt_cp_add_advertising *cp;
	unsigned char adv_param[sizeof(*cp) + TESTER_ADD_ADV_DATA_LEN];
	unsigned char param[] = { 0x01 };

	tester_print("Set and add advertising instance");

	cp = (struct mgmt_cp_add_advertising *) adv_param;
	setup_add_adv_param(cp, 1);

	mgmt_send(data->mgmt, MGMT_OP_SET_LE, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_ADVERTISING, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_ADD_ADVERTISING, data->mgmt_index,
						sizeof(adv_param), adv_param,
						setup_add_advertising_callback,
						NULL, NULL);
}

static void setup_multi_adv_second_instance(uint8_t status, uint16_t length,
		const void *param, void *user_data) {
	struct mgmt_rp_add_advertising *rp =
				(struct mgmt_rp_add_advertising *) param;
	struct test_data *data = tester_get_data();
	struct mgmt_cp_add_advertising *cp;
	unsigned char adv_param[sizeof(*cp) + TESTER_ADD_ADV_DATA_LEN];

	if (status != MGMT_STATUS_SUCCESS) {
		tester_setup_failed();
		return;
	}

	tester_print("Add Advertising setup complete (instance %d)",
								rp->instance);

	cp = (struct mgmt_cp_add_advertising *) adv_param;
	setup_add_adv_param(cp, 2);
	cp->timeout = 1;
	cp->duration = 1;

	mgmt_send(data->mgmt, MGMT_OP_ADD_ADVERTISING, data->mgmt_index,
						sizeof(adv_param), adv_param,
						setup_add_advertising_callback,
						NULL, NULL);
}

static void setup_multi_adv(const void *test_data)
{
	struct test_data *data = tester_get_data();
	struct mgmt_cp_add_advertising *cp;
	unsigned char adv_param[sizeof(*cp) + TESTER_ADD_ADV_DATA_LEN];
	unsigned char param[] = { 0x01 };

	tester_print("Adding two instances with timeout 1 and duration 1");

	cp = (struct mgmt_cp_add_advertising *) adv_param;
	setup_add_adv_param(cp, 1);
	cp->timeout = 1;
	cp->duration = 1;

	mgmt_send(data->mgmt, MGMT_OP_SET_LE, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
						sizeof(param), &param,
						NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_ADD_ADVERTISING, data->mgmt_index,
						sizeof(adv_param), adv_param,
						setup_multi_adv_second_instance,
						NULL, NULL);
}

static void setup_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	if (status != MGMT_STATUS_SUCCESS) {
		tester_setup_failed();
		return;
	}

	tester_print("Initial settings completed");

	if (data->test_setup)
		data->test_setup(data);
	else
		setup_bthost();
}

static void pin_code_request_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_pin_code_request *ev = param;
	struct test_data *data = user_data;
	const struct generic_data *test = data->test_data;
	struct mgmt_cp_pin_code_reply cp;

	test_condition_complete(data);

	memset(&cp, 0, sizeof(cp));
	memcpy(&cp.addr, &ev->addr, sizeof(cp.addr));

	if (!test->pin) {
		mgmt_reply(data->mgmt, MGMT_OP_PIN_CODE_NEG_REPLY,
				data->mgmt_index, sizeof(cp.addr), &cp.addr,
				NULL, NULL, NULL);
		return;
	}

	cp.pin_len = test->pin_len;
	memcpy(cp.pin_code, test->pin, test->pin_len);

	mgmt_reply(data->mgmt, MGMT_OP_PIN_CODE_REPLY, data->mgmt_index,
			sizeof(cp), &cp, NULL, NULL, NULL);
}

static void user_confirm_request_callback(uint16_t index, uint16_t length,
							const void *param,
							void *user_data)
{
	const struct mgmt_ev_user_confirm_request *ev = param;
	struct test_data *data = user_data;
	const struct generic_data *test = data->test_data;
	struct mgmt_cp_user_confirm_reply cp;
	uint16_t opcode;

	if (test->just_works) {
		tester_warn("User Confirmation received for just-works case");
		tester_test_failed();
		return;
	}

	memset(&cp, 0, sizeof(cp));
	memcpy(&cp.addr, &ev->addr, sizeof(cp.addr));

	if (test->reject_confirm)
		opcode = MGMT_OP_USER_CONFIRM_NEG_REPLY;
	else
		opcode = MGMT_OP_USER_CONFIRM_REPLY;

	mgmt_reply(data->mgmt, opcode, data->mgmt_index, sizeof(cp), &cp,
							NULL, NULL, NULL);
}

static void user_passkey_request_callback(uint16_t index, uint16_t length,
							const void *param,
							void *user_data)
{
	const struct mgmt_ev_user_passkey_request *ev = param;
	struct test_data *data = user_data;
	const struct generic_data *test = data->test_data;
	struct mgmt_cp_user_passkey_reply cp;

	if (test->just_works) {
		tester_warn("User Passkey Request for just-works case");
		tester_test_failed();
		return;
	}

	memset(&cp, 0, sizeof(cp));
	memcpy(&cp.addr, &ev->addr, sizeof(cp.addr));

	if (test->reject_confirm) {
		mgmt_reply(data->mgmt, MGMT_OP_USER_PASSKEY_NEG_REPLY,
				data->mgmt_index, sizeof(cp.addr), &cp.addr,
				NULL, NULL, NULL);
		return;
	}

	mgmt_reply(data->mgmt, MGMT_OP_USER_PASSKEY_REPLY, data->mgmt_index,
					sizeof(cp), &cp, NULL, NULL, NULL);
}

static void test_setup(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	const uint16_t *cmd;

	if (!test)
		goto proceed;

	if (test->pin || test->expect_pin) {
		mgmt_register(data->mgmt, MGMT_EV_PIN_CODE_REQUEST,
				data->mgmt_index, pin_code_request_callback,
				data, NULL);
		test_add_condition(data);
	}

	mgmt_register(data->mgmt, MGMT_EV_USER_CONFIRM_REQUEST,
			data->mgmt_index, user_confirm_request_callback,
			data, NULL);

	mgmt_register(data->mgmt, MGMT_EV_USER_PASSKEY_REQUEST,
			data->mgmt_index, user_passkey_request_callback,
			data, NULL);

	if (test->client_pin)
		bthost_set_pin_code(bthost, test->client_pin,
							test->client_pin_len);

	if (test->client_io_cap)
		bthost_set_io_capability(bthost, test->client_io_cap);

	if (test->client_auth_req)
		bthost_set_auth_req(bthost, test->client_auth_req);
	else if (!test->just_works)
		bthost_set_auth_req(bthost, 0x01);

	if (test->client_reject_confirm)
		bthost_set_reject_user_confirm(bthost, true);

	if (test->client_enable_le)
		bthost_write_le_host_supported(bthost, 0x01);

	if (test->client_enable_sc)
		bthost_set_sc_support(bthost, 0x01);

proceed:
	if (!test || !test->setup_settings) {
		if (data->test_setup)
			data->test_setup(data);
		else
			tester_setup_complete();
		return;
	}

	for (cmd = test->setup_settings; *cmd; cmd++) {
		unsigned char simple_param[] = { 0x01 };
		unsigned char discov_param[] = { 0x01, 0x00, 0x00 };
		unsigned char privacy_param[] = { 0x01,
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
		unsigned char *param = simple_param;
		size_t param_size = sizeof(simple_param);
		mgmt_request_func_t func = NULL;

		/* If this is the last command (next one is 0) request
		 * for a callback. */
		if (!cmd[1])
			func = setup_complete;

		if (*cmd == MGMT_OP_SET_DISCOVERABLE) {
			if (test->setup_limited_discov) {
				discov_param[0] = 0x02;
				discov_param[1] = 0x01;
			}
			param_size = sizeof(discov_param);
			param = discov_param;
		}

		if (*cmd == MGMT_OP_SET_PRIVACY) {
			param_size = sizeof(privacy_param);
			param = privacy_param;
		}

		if (*cmd == MGMT_OP_SET_LE && test->setup_nobredr) {
			unsigned char off[] = { 0x00 };
			mgmt_send(data->mgmt, *cmd, data->mgmt_index,
					param_size, param, NULL, NULL, NULL);
			mgmt_send(data->mgmt, MGMT_OP_SET_BREDR,
					data->mgmt_index, sizeof(off), off,
					func, data, NULL);
		} else {
			mgmt_send(data->mgmt, *cmd, data->mgmt_index,
					param_size, param, func, data, NULL);
		}
	}
}

static void command_generic_new_settings(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	tester_print("New settings event received");

	mgmt_unregister(data->mgmt, data->mgmt_settings_id);

	tester_test_failed();
}

static void command_generic_new_settings_alt(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	uint32_t settings;

	if (length != 4) {
		tester_warn("Invalid parameter size for new settings event");
		tester_test_failed();
		return;
	}

	settings = get_le32(param);

	tester_print("New settings 0x%08x received", settings);

	if (test->expect_settings_unset) {
		if ((settings & test->expect_settings_unset) != 0)
			return;
		goto done;
	}

	if (!test->expect_settings_set)
		return;

	if ((settings & test->expect_settings_set) != test->expect_settings_set)
		return;

done:
	tester_print("Unregistering new settings notification");

	mgmt_unregister(data->mgmt_alt, data->mgmt_alt_settings_id);

	test_condition_complete(data);
}

static bool verify_alt_ev(const void *param, uint16_t length)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;

	if (length != test->expect_alt_ev_len) {
		tester_warn("Invalid length %u != %u", length,
						test->expect_alt_ev_len);
		return false;
	}

	if (test->expect_alt_ev_param &&
			memcmp(test->expect_alt_ev_param, param, length)) {
		tester_warn("Event parameters do not match");
		return false;
	}

	return true;
}

static void command_generic_event_alt(uint16_t index, uint16_t length,
							const void *param,
							void *user_data)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	bool (*verify)(const void *param, uint16_t length);

	tester_print("New %s event received", mgmt_evstr(test->expect_alt_ev));

	mgmt_unregister(data->mgmt_alt, data->mgmt_alt_ev_id);

	if (test->verify_alt_ev_func)
		verify = test->verify_alt_ev_func;
	else
		verify = verify_alt_ev;

	if (!verify(param, length)) {
		tester_warn("Incorrect %s event parameters",
					mgmt_evstr(test->expect_alt_ev));
		tester_test_failed();
		return;
	}

	test_condition_complete(data);
}

static void command_generic_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	const void *expect_param = test->expect_param;
	uint16_t expect_len = test->expect_len;

	tester_print("%s (0x%04x): %s (0x%02x)", mgmt_opstr(test->send_opcode),
			test->send_opcode, mgmt_errstr(status), status);

	if (status != test->expect_status) {
		tester_test_failed();
		return;
	}

	if (!test->expect_ignore_param) {
		if (test->expect_func)
			expect_param = test->expect_func(&expect_len);

		if (length != expect_len) {
			tester_warn("Invalid cmd response parameter size");
			tester_test_failed();
			return;
		}

		if (expect_param && expect_len > 0 &&
					memcmp(param, expect_param, length)) {
			tester_warn("Unexpected cmd response parameter value");
			tester_test_failed();
			return;
		}
	}

	test_condition_complete(data);
}

static void command_hci_callback(uint16_t opcode, const void *param,
					uint8_t length, void *user_data)
{
	struct test_data *data = user_data;
	const struct generic_data *test = data->test_data;
	const void *expect_hci_param = test->expect_hci_param;
	uint8_t expect_hci_len = test->expect_hci_len;

	tester_print("HCI Command 0x%04x length %u", opcode, length);

	if (opcode != test->expect_hci_command)
		return;

	if (test->expect_hci_func)
		expect_hci_param = test->expect_hci_func(&expect_hci_len);

	if (length != expect_hci_len) {
		tester_warn("Invalid parameter size for HCI command");
		tester_test_failed();
		return;
	}

	if (memcmp(param, expect_hci_param, length) != 0) {
		tester_warn("Unexpected HCI command parameter value");
		tester_test_failed();
		return;
	}

	test_condition_complete(data);
}

static bool power_off(uint16_t index)
{
	int sk, err;

	sk = hci_open_dev(index);
	if (sk < 0)
		return false;

	err = ioctl(sk, HCIDEVDOWN, index);

	hci_close_dev(sk);

	if (err < 0)
		return false;

	return true;
}

static void test_command_generic(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	const void *send_param = test->send_param;
	uint16_t send_len = test->send_len;
	unsigned int id;
	uint16_t index;

	index = test->send_index_none ? MGMT_INDEX_NONE : data->mgmt_index;

	if (test->expect_settings_set || test->expect_settings_unset) {
		tester_print("Registering new settings notification");

		id = mgmt_register(data->mgmt, MGMT_EV_NEW_SETTINGS, index,
				command_generic_new_settings, NULL, NULL);
		data->mgmt_settings_id = id;

		id = mgmt_register(data->mgmt_alt, MGMT_EV_NEW_SETTINGS, index,
				command_generic_new_settings_alt, NULL, NULL);
		data->mgmt_alt_settings_id = id;
		test_add_condition(data);
	}

	if (test->expect_alt_ev) {
		tester_print("Registering %s notification",
					mgmt_evstr(test->expect_alt_ev));
		id = mgmt_register(data->mgmt_alt, test->expect_alt_ev, index,
					command_generic_event_alt, NULL, NULL);
		data->mgmt_alt_ev_id = id;
		test_add_condition(data);
	}

	if (test->expect_hci_command) {
		tester_print("Registering HCI command callback");
		hciemu_add_master_post_command_hook(data->hciemu,
						command_hci_callback, data);
		test_add_condition(data);
	}

	if (test->send_opcode == 0x0000) {
		tester_print("Executing no-op test");
		return;
	}

	tester_print("Sending %s (0x%04x)", mgmt_opstr(test->send_opcode),
							test->send_opcode);

	if (test->send_func)
		send_param = test->send_func(&send_len);

	if (test->force_power_off) {
		mgmt_send_nowait(data->mgmt, test->send_opcode, index,
					send_len, send_param,
					command_generic_callback, NULL, NULL);
		power_off(data->mgmt_index);
	} else {
		mgmt_send(data->mgmt, test->send_opcode, index, send_len,
					send_param, command_generic_callback,
					NULL, NULL);
	}

	test_add_condition(data);
}

static void check_scan(void *user_data)
{
	struct test_data *data = tester_get_data();

	if (hciemu_get_master_le_scan_enable(data->hciemu)) {
		tester_warn("LE scan still enabled");
		tester_test_failed();
		return;
	}

	if (hciemu_get_master_scan_enable(data->hciemu)) {
		tester_warn("BR/EDR scan still enabled");
		tester_test_failed();
		return;
	}

	test_condition_complete(data);
}

static void test_remove_device(const void *test_data)
{
	struct test_data *data = tester_get_data();

	test_command_generic(test_data);
	tester_wait(1, check_scan, NULL);
	test_add_condition(data);
}

static void test_device_found(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	struct bthost *bthost;

	bthost = hciemu_client_get_host(data->hciemu);

	if ((data->hciemu_type == HCIEMU_TYPE_LE) ||
				(data->hciemu_type == HCIEMU_TYPE_BREDRLE)) {
		if (test->set_adv)
			bthost_set_adv_data(bthost, test->adv_data,
							test->adv_data_len);

		bthost_set_adv_enable(bthost, 0x01);
	}

	if (data->hciemu_type != HCIEMU_TYPE_LE)
		bthost_write_scan_enable(bthost, 0x03);

	test_command_generic(test_data);
}

static void pairing_new_conn(uint16_t handle, void *user_data)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost;

	tester_print("New connection with handle 0x%04x", handle);

	bthost = hciemu_client_get_host(data->hciemu);

	bthost_request_auth(bthost, handle);
}

static void test_pairing_acceptor(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	const uint8_t *master_bdaddr;
	struct bthost *bthost;
	uint8_t addr_type;

	if (test->expect_alt_ev) {
		unsigned int id;

		tester_print("Registering %s notification",
					mgmt_evstr(test->expect_alt_ev));
		id = mgmt_register(data->mgmt_alt, test->expect_alt_ev,
					data->mgmt_index,
					command_generic_event_alt, NULL, NULL);
		data->mgmt_alt_ev_id = id;
		test_add_condition(data);
	}

	master_bdaddr = hciemu_get_master_bdaddr(data->hciemu);
	if (!master_bdaddr) {
		tester_warn("No master bdaddr");
		tester_test_failed();
		return;
	}

	bthost = hciemu_client_get_host(data->hciemu);
	bthost_set_connect_cb(bthost, pairing_new_conn, data);

	if (data->hciemu_type == HCIEMU_TYPE_BREDRLE)
		addr_type = BDADDR_BREDR;
	else
		addr_type = BDADDR_LE_PUBLIC;

	bthost_hci_connect(bthost, master_bdaddr, addr_type);
}

static void connected_event(uint16_t index, uint16_t length, const void *param,
							void *user_data)
{
	struct test_data *data = tester_get_data();
	const struct generic_data *test = data->test_data;
	const void *send_param = test->send_param;
	uint16_t send_len = test->send_len;

	tester_print("Sending %s 0x%04x", mgmt_opstr(test->send_opcode),
							test->send_opcode);

	if (test->send_func)
		send_param = test->send_func(&send_len);

	if (test->force_power_off) {
		mgmt_send_nowait(data->mgmt, test->send_opcode, index,
					send_len, send_param,
					command_generic_callback, NULL, NULL);
		power_off(data->mgmt_index);
	} else {
		mgmt_send(data->mgmt, test->send_opcode, index, send_len,
					send_param, command_generic_callback,
					NULL, NULL);
	}

	test_add_condition(data);

	/* Complete MGMT_EV_DEVICE_CONNECTED *after* adding new one */
	test_condition_complete(data);
}

static void test_command_generic_connect(const void *test_data)
{
	struct test_data *data = tester_get_data();
	unsigned int id;
	const uint8_t *master_bdaddr;
	uint8_t addr_type;
	struct bthost *bthost;

	tester_print("Registering %s notification",
					mgmt_evstr(MGMT_EV_DEVICE_CONNECTED));
	id = mgmt_register(data->mgmt_alt, MGMT_EV_DEVICE_CONNECTED,
				data->mgmt_index, connected_event,
				NULL, NULL);
	data->mgmt_alt_ev_id = id;
	test_add_condition(data);

	master_bdaddr = hciemu_get_master_bdaddr(data->hciemu);
	if (!master_bdaddr) {
		tester_warn("No master bdaddr");
		tester_test_failed();
		return;
	}

	addr_type = data->hciemu_type == HCIEMU_TYPE_BREDRLE ? BDADDR_BREDR :
							BDADDR_LE_PUBLIC;

	bthost = hciemu_client_get_host(data->hciemu);
	bthost_hci_connect(bthost, master_bdaddr, addr_type);
}

int main(int argc, char *argv[])
{
	tester_init(&argc, &argv);

	test_bredrle("Controller setup",
				NULL, NULL, controller_setup);
	test_bredr("Controller setup (BR/EDR-only)",
				NULL, NULL, controller_setup);
	test_le("Controller setup (LE)",
				NULL, NULL, controller_setup);

	test_bredrle("Invalid command",
				&invalid_command_test,
				NULL, test_command_generic);

	test_bredrle("Read version - Success",
				&read_version_success_test,
				NULL, test_command_generic);
	test_bredrle("Read version - Invalid parameters",
				&read_version_invalid_param_test,
				NULL, test_command_generic);
	test_bredrle("Read version - Invalid index",
				&read_version_invalid_index_test,
				NULL, test_command_generic);
	test_bredrle("Read commands - Invalid parameters",
				&read_commands_invalid_param_test,
				NULL, test_command_generic);
	test_bredrle("Read commands - Invalid index",
				&read_commands_invalid_index_test,
				NULL, test_command_generic);
	test_bredrle("Read index list - Invalid parameters",
				&read_index_list_invalid_param_test,
				NULL, test_command_generic);
	test_bredrle("Read index list - Invalid index",
				&read_index_list_invalid_index_test,
				NULL, test_command_generic);
	test_bredrle("Read info - Invalid parameters",
				&read_info_invalid_param_test,
				NULL, test_command_generic);
	test_bredrle("Read info - Invalid index",
				&read_info_invalid_index_test,
				NULL, test_command_generic);
	test_bredrle("Read unconfigured index list - Invalid parameters",
				&read_unconf_index_list_invalid_param_test,
				NULL, test_command_generic);
	test_bredrle("Read unconfigured index list - Invalid index",
				&read_unconf_index_list_invalid_index_test,
				NULL, test_command_generic);
	test_bredrle("Read configuration info - Invalid parameters",
				&read_config_info_invalid_param_test,
				NULL, test_command_generic);
	test_bredrle("Read configuration info - Invalid index",
				&read_config_info_invalid_index_test,
				NULL, test_command_generic);
	test_bredrle("Read extended index list - Invalid parameters",
				&read_ext_index_list_invalid_param_test,
				NULL, test_command_generic);
	test_bredrle("Read extended index list - Invalid index",
				&read_ext_index_list_invalid_index_test,
				NULL, test_command_generic);

	test_bredrle("Set powered on - Success",
				&set_powered_on_success_test,
				NULL, test_command_generic);
	test_bredrle("Set powered on - Invalid parameters 1",
				&set_powered_on_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Set powered on - Invalid parameters 2",
				&set_powered_on_invalid_param_test_2,
				NULL, test_command_generic);
	test_bredrle("Set powered on - Invalid parameters 3",
				&set_powered_on_invalid_param_test_3,
				NULL, test_command_generic);
	test_bredrle("Set powered on - Invalid index",
				&set_powered_on_invalid_index_test,
				NULL, test_command_generic);
	test_le("Set powered on - Privacy and Advertising",
				&set_powered_on_privacy_adv_test,
				NULL, test_command_generic);

	test_bredrle("Set powered off - Success",
				&set_powered_off_success_test,
				NULL, test_command_generic);
	test_bredrle("Set powered off - Class of Device",
				&set_powered_off_class_test,
				setup_class, test_command_generic);
	test_bredrle("Set powered off - Invalid parameters 1",
				&set_powered_off_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Set powered off - Invalid parameters 2",
				&set_powered_off_invalid_param_test_2,
				NULL, test_command_generic);
	test_bredrle("Set powered off - Invalid parameters 3",
				&set_powered_off_invalid_param_test_3,
				NULL, test_command_generic);

	test_bredrle("Set connectable on - Success 1",
				&set_connectable_on_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set connectable on - Success 2",
				&set_connectable_on_success_test_2,
				NULL, test_command_generic);
	test_bredrle("Set connectable on - Invalid parameters 1",
				&set_connectable_on_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Set connectable on - Invalid parameters 2",
				&set_connectable_on_invalid_param_test_2,
				NULL, test_command_generic);
	test_bredrle("Set connectable on - Invalid parameters 3",
				&set_connectable_on_invalid_param_test_3,
				NULL, test_command_generic);
	test_bredrle("Set connectable on - Invalid index",
				&set_connectable_on_invalid_index_test,
				NULL, test_command_generic);

	test_le("Set connectable on (LE) - Success 1",
				&set_connectable_on_le_test_1,
				NULL, test_command_generic);
	test_le("Set connectable on (LE) - Success 2",
				&set_connectable_on_le_test_2,
				NULL, test_command_generic);
	test_le("Set connectable on (LE) - Success 3",
				&set_connectable_on_le_test_3,
				NULL, test_command_generic);

	test_bredrle("Set connectable off - Success 1",
				&set_connectable_off_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set connectable off - Success 2",
				&set_connectable_off_success_test_2,
				NULL, test_command_generic);
	test_bredrle("Set connectable off - Success 3",
				&set_connectable_off_success_test_3,
				NULL, test_command_generic);
	test_bredrle("Set connectable off - Success 4",
				&set_connectable_off_success_test_4,
				setup_add_device, test_command_generic);

	test_le("Set connectable off (LE) - Success 1",
				&set_connectable_off_le_test_1,
				NULL, test_command_generic);
	test_le("Set connectable off (LE) - Success 2",
				&set_connectable_off_le_test_2,
				NULL, test_command_generic);
	test_le("Set connectable off (LE) - Success 3",
				&set_connectable_off_le_test_3,
				NULL, test_command_generic);
	test_le("Set connectable off (LE) - Success 4",
				&set_connectable_off_le_test_4,
				NULL, test_command_generic);

	test_bredrle("Set fast connectable on - Success 1",
				&set_fast_conn_on_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set fast connectable on - Success 2",
				&set_fast_conn_on_success_test_2,
				NULL, test_command_generic);
	test_bredrle("Set fast connectable on - Success 3",
				&set_fast_conn_on_success_test_3,
				NULL, test_command_generic);
	test_bredrle("Set fast connectable on - Invalid Params 1",
				&set_fast_conn_nval_param_test_1,
				NULL, test_command_generic);
	test_le("Set fast connectable on - Not Supported 1",
				&set_fast_conn_on_not_supported_test_1,
				NULL, test_command_generic);

	test_bredrle("Set bondable on - Success",
				&set_bondable_on_success_test,
				NULL, test_command_generic);
	test_bredrle("Set bondable on - Invalid parameters 1",
				&set_bondable_on_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Set bondable on - Invalid parameters 2",
				&set_bondable_on_invalid_param_test_2,
				NULL, test_command_generic);
	test_bredrle("Set bondable on - Invalid parameters 3",
				&set_bondable_on_invalid_param_test_3,
				NULL, test_command_generic);
	test_bredrle("Set bondable on - Invalid index",
				&set_bondable_on_invalid_index_test,
				NULL, test_command_generic);

	test_bredrle("Set discoverable on - Invalid parameters 1",
				&set_discoverable_on_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Set discoverable on - Invalid parameters 2",
				&set_discoverable_on_invalid_param_test_2,
				NULL, test_command_generic);
	test_bredrle("Set discoverable on - Invalid parameters 3",
				&set_discoverable_on_invalid_param_test_3,
				NULL, test_command_generic);
	test_bredrle("Set discoverable on - Invalid parameters 4",
				&set_discoverable_on_invalid_param_test_4,
				NULL, test_command_generic);
	test_bredrle("Set discoverable on - Not powered 1",
				&set_discoverable_on_not_powered_test_1,
				NULL, test_command_generic);
	test_bredrle("Set discoverable on - Not powered 2",
				&set_discoverable_on_not_powered_test_2,
				NULL, test_command_generic);
	test_bredrle("Set discoverable on - Rejected 1",
				&set_discoverable_on_rejected_test_1,
				NULL, test_command_generic);
	test_bredrle("Set discoverable on - Rejected 2",
				&set_discoverable_on_rejected_test_2,
				NULL, test_command_generic);
	test_bredrle("Set discoverable on - Rejected 3",
				&set_discoverable_on_rejected_test_3,
				NULL, test_command_generic);
	test_bredrle("Set discoverable on - Success 1",
				&set_discoverable_on_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set discoverable on - Success 2",
				&set_discoverable_on_success_test_2,
				NULL, test_command_generic);
	test_le("Set discoverable on (LE) - Success 1",
				&set_discov_on_le_success_1,
				NULL, test_command_generic);
	test_bredrle("Set discoverable off - Success 1",
				&set_discoverable_off_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set discoverable off - Success 2",
				&set_discoverable_off_success_test_2,
				NULL, test_command_generic);
	test_bredrle("Set limited discoverable on - Success 1",
				&set_limited_discov_on_success_1,
				NULL, test_command_generic);
	test_bredrle("Set limited discoverable on - Success 2",
				&set_limited_discov_on_success_2,
				NULL, test_command_generic);
	test_bredrle("Set limited discoverable on - Success 3",
				&set_limited_discov_on_success_3,
				NULL, test_command_generic);
	test_le("Set limited discoverable on (LE) - Success 1",
				&set_limited_discov_on_le_success_1,
				NULL, test_command_generic);

	test_bredrle("Set link security on - Success 1",
				&set_link_sec_on_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set link security on - Success 2",
				&set_link_sec_on_success_test_2,
				NULL, test_command_generic);
	test_bredrle("Set link security on - Success 3",
				&set_link_sec_on_success_test_3,
				NULL, test_command_generic);
	test_bredrle("Set link security on - Invalid parameters 1",
				&set_link_sec_on_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Set link security on - Invalid parameters 2",
				&set_link_sec_on_invalid_param_test_2,
				NULL, test_command_generic);
	test_bredrle("Set link security on - Invalid parameters 3",
				&set_link_sec_on_invalid_param_test_3,
				NULL, test_command_generic);
	test_bredrle("Set link security on - Invalid index",
				&set_link_sec_on_invalid_index_test,
				NULL, test_command_generic);

	test_bredrle("Set link security off - Success 1",
				&set_link_sec_off_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set link security off - Success 2",
				&set_link_sec_off_success_test_2,
				NULL, test_command_generic);

	test_bredrle("Set SSP on - Success 1",
				&set_ssp_on_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set SSP on - Success 2",
				&set_ssp_on_success_test_2,
				NULL, test_command_generic);
	test_bredrle("Set SSP on - Success 3",
				&set_ssp_on_success_test_3,
				NULL, test_command_generic);
	test_bredrle("Set SSP on - Invalid parameters 1",
				&set_ssp_on_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Set SSP on - Invalid parameters 2",
				&set_ssp_on_invalid_param_test_2,
				NULL, test_command_generic);
	test_bredrle("Set SSP on - Invalid parameters 3",
				&set_ssp_on_invalid_param_test_3,
				NULL, test_command_generic);
	test_bredrle("Set SSP on - Invalid index",
				&set_ssp_on_invalid_index_test,
				NULL, test_command_generic);

	test_bredrle("Set Secure Connections on - Success 1",
				&set_sc_on_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set Secure Connections on - Success 2",
				&set_sc_on_success_test_2,
				NULL, test_command_generic);
	test_bredrle("Set Secure Connections on - Invalid params 1",
				&set_sc_on_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Set Secure Connections on - Invalid params 2",
				&set_sc_on_invalid_param_test_2,
				NULL, test_command_generic);
	test_bredrle("Set Secure Connections on - Invalid params 3",
				&set_sc_on_invalid_param_test_3,
				NULL, test_command_generic);
	test_bredrle("Set Secure Connections on - Invalid index",
				&set_sc_on_invalid_index_test,
				NULL, test_command_generic);
	test_bredr("Set Secure Connections on - Not supported 1",
				&set_sc_on_not_supported_test_1,
				NULL, test_command_generic);
	test_bredr("Set Secure Connections on - Not supported 2",
				&set_sc_on_not_supported_test_2,
				NULL, test_command_generic);

	test_bredrle("Set Secure Connections Only on - Success 1",
				&set_sc_only_on_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set Secure Connections Only on - Success 2",
				&set_sc_only_on_success_test_2,
				NULL, test_command_generic);

	test_bredrle("Set High Speed on - Success",
				&set_hs_on_success_test,
				NULL, test_command_generic);
	test_bredrle("Set High Speed on - Invalid parameters 1",
				&set_hs_on_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Set High Speed on - Invalid parameters 2",
				&set_hs_on_invalid_param_test_2,
				NULL, test_command_generic);
	test_bredrle("Set High Speed on - Invalid parameters 3",
				&set_hs_on_invalid_param_test_3,
				NULL, test_command_generic);
	test_bredrle("Set High Speed on - Invalid index",
				&set_hs_on_invalid_index_test,
				NULL, test_command_generic);

	test_bredrle("Set Low Energy on - Success 1",
				&set_le_on_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set Low Energy on - Success 2",
				&set_le_on_success_test_2,
				NULL, test_command_generic);
	test_bredrle("Set Low Energy on - Success 3",
				&set_le_on_success_test_3,
				NULL, test_command_generic);
	test_bredrle("Set Low Energy on - Invalid parameters 1",
				&set_le_on_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Set Low Energy on - Invalid parameters 2",
				&set_le_on_invalid_param_test_2,
				NULL, test_command_generic);
	test_bredrle("Set Low Energy on - Invalid parameters 3",
				&set_le_on_invalid_param_test_3,
				NULL, test_command_generic);
	test_bredrle("Set Low Energy on - Invalid index",
				&set_le_on_invalid_index_test,
				NULL, test_command_generic);

	test_bredrle("Set Advertising on - Success 1",
				&set_adv_on_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set Advertising on - Success 2",
				&set_adv_on_success_test_2,
				NULL, test_command_generic);
	test_bredrle("Set Advertising on - Rejected 1",
				&set_adv_on_rejected_test_1,
				NULL, test_command_generic);

	test_bredrle("Set BR/EDR off - Success 1",
				&set_bredr_off_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set BR/EDR on - Success 1",
				&set_bredr_on_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Set BR/EDR on - Success 2",
				&set_bredr_on_success_test_2,
				NULL, test_command_generic);
	test_bredr("Set BR/EDR off - Not Supported 1",
				&set_bredr_off_notsupp_test,
				NULL, test_command_generic);
	test_le("Set BR/EDR off - Not Supported 2",
				&set_bredr_off_notsupp_test,
				NULL, test_command_generic);
	test_bredrle("Set BR/EDR off - Rejected 1",
				&set_bredr_off_failure_test_1,
				NULL, test_command_generic);
	test_bredrle("Set BR/EDR off - Rejected 2",
				&set_bredr_off_failure_test_2,
				NULL, test_command_generic);
	test_bredrle("Set BR/EDR off - Invalid Parameters 1",
				&set_bredr_off_failure_test_3,
				NULL, test_command_generic);

	test_bredr("Set Local Name - Success 1",
				&set_local_name_test_1,
				NULL, test_command_generic);
	test_bredr("Set Local Name - Success 2",
				&set_local_name_test_2,
				NULL, test_command_generic);
	test_bredr("Set Local Name - Success 3",
				&set_local_name_test_3,
				NULL, test_command_generic);

	test_bredrle("Start Discovery - Not powered 1",
				&start_discovery_not_powered_test_1,
				NULL, test_command_generic);
	test_bredrle("Start Discovery - Invalid parameters 1",
				&start_discovery_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Start Discovery - Not supported 1",
				&start_discovery_not_supported_test_1,
				NULL, test_command_generic);
	test_bredrle("Start Discovery - Success 1",
				&start_discovery_valid_param_test_1,
				NULL, test_command_generic);
	test_le("Start Discovery - Success 2",
				&start_discovery_valid_param_test_2,
				NULL, test_command_generic);
	test_bredrle("Start Discovery - Power Off 1",
				&start_discovery_valid_param_power_off_1,
				NULL, test_command_generic);

	test_bredrle("Stop Discovery - Success 1",
				&stop_discovery_success_test_1,
				setup_start_discovery, test_command_generic);
	test_bredr("Stop Discovery - BR/EDR (Inquiry) Success 1",
				&stop_discovery_bredr_success_test_1,
				setup_start_discovery, test_command_generic);
	test_bredrle("Stop Discovery - Rejected 1",
				&stop_discovery_rejected_test_1,
				NULL, test_command_generic);
	test_bredrle("Stop Discovery - Invalid parameters 1",
				&stop_discovery_invalid_param_test_1,
				setup_start_discovery, test_command_generic);

	test_bredrle("Start Service Discovery - Not powered 1",
				&start_service_discovery_not_powered_test_1,
				NULL, test_command_generic);
	test_bredrle("Start Service Discovery - Invalid parameters 1",
				&start_service_discovery_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Start Service Discovery - Not supported 1",
				&start_service_discovery_not_supported_test_1,
				NULL, test_command_generic);
	test_bredrle("Start Service Discovery - Success 1",
				&start_service_discovery_valid_param_test_1,
				NULL, test_command_generic);
	test_le("Start Service Discovery - Success 2",
				&start_service_discovery_valid_param_test_2,
				NULL, test_command_generic);

	test_bredrle("Set Device Class - Success 1",
				&set_dev_class_valid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Set Device Class - Success 2",
				&set_dev_class_valid_param_test_2,
				NULL, test_command_generic);
	test_bredrle("Set Device Class - Invalid parameters 1",
				&set_dev_class_invalid_param_test_1,
				NULL, test_command_generic);

	test_bredrle("Add UUID - UUID-16 1",
				&add_uuid16_test_1,
				NULL, test_command_generic);
	test_bredrle("Add UUID - UUID-16 multiple 1",
				&add_multi_uuid16_test_1,
				setup_multi_uuid16, test_command_generic);
	test_bredrle("Add UUID - UUID-16 partial 1",
				&add_multi_uuid16_test_2,
				setup_multi_uuid16_2, test_command_generic);
	test_bredrle("Add UUID - UUID-32 1",
				&add_uuid32_test_1,
				NULL, test_command_generic);
	test_bredrle("Add UUID - UUID-32 multiple 1",
				&add_uuid32_multi_test_1,
				setup_multi_uuid32, test_command_generic);
	test_bredrle("Add UUID - UUID-32 partial 1",
				&add_uuid32_multi_test_2,
				setup_multi_uuid32_2, test_command_generic);
	test_bredrle("Add UUID - UUID-128 1",
				&add_uuid128_test_1,
				NULL, test_command_generic);
	test_bredrle("Add UUID - UUID-128 multiple 1",
				&add_uuid128_multi_test_1,
				setup_multi_uuid128, test_command_generic);
	test_bredrle("Add UUID - UUID-128 partial 1",
				&add_uuid128_multi_test_2,
				setup_multi_uuid128_2, test_command_generic);
	test_bredrle("Add UUID - UUID mix",
				&add_uuid_mix_test_1,
				setup_uuid_mix, test_command_generic);

	test_bredrle("Load Link Keys - Empty List Success 1",
				&load_link_keys_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Load Link Keys - Empty List Success 2",
				&load_link_keys_success_test_2,
				NULL, test_command_generic);
	test_bredrle("Load Link Keys - Invalid Parameters 1",
				&load_link_keys_invalid_params_test_1,
				NULL, test_command_generic);
	test_bredrle("Load Link Keys - Invalid Parameters 2",
				&load_link_keys_invalid_params_test_2,
				NULL, test_command_generic);
	test_bredrle("Load Link Keys - Invalid Parameters 3",
				&load_link_keys_invalid_params_test_3,
				NULL, test_command_generic);

	test_bredrle("Load Long Term Keys - Success 1",
				&load_ltks_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Load Long Term Keys - Invalid Parameters 1",
				&load_ltks_invalid_params_test_1,
				NULL, test_command_generic);
	test_bredrle("Load Long Term Keys - Invalid Parameters 2",
				&load_ltks_invalid_params_test_2,
				NULL, test_command_generic);
	test_bredrle("Load Long Term Keys - Invalid Parameters 3",
				&load_ltks_invalid_params_test_3,
				NULL, test_command_generic);
	test_bredrle("Load Long Term Keys - Invalid Parameters 4",
				&load_ltks_invalid_params_test_4,
				NULL, test_command_generic);

	test_bredrle("Set IO Capability - Invalid Params 1",
				&set_io_cap_invalid_param_test_1,
				NULL, test_command_generic);

	test_bredrle("Pair Device - Not Powered 1",
				&pair_device_not_powered_test_1,
				NULL, test_command_generic);
	test_bredrle("Pair Device - Power off 1",
				&pair_device_power_off_test_1,
				NULL, test_command_generic);
	test_le("Pair Device - Incorrect transport reject 1",
				&pair_device_not_supported_test_1,
				NULL, test_command_generic);
	test_bredr("Pair Device - Incorrect transport reject 2",
				&pair_device_not_supported_test_2,
				NULL, test_command_generic);
	test_bredrle("Pair Device - Reject on not enabled transport 1",
				&pair_device_reject_transport_not_enabled_1,
				NULL, test_command_generic);
	test_bredrle("Pair Device - Reject on not enabled transport 2",
				&pair_device_reject_transport_not_enabled_2,
				NULL, test_command_generic);
	test_bredrle("Pair Device - Invalid Parameters 1",
				&pair_device_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Pair Device - Invalid Parameters 2",
				&pair_device_invalid_param_test_2,
				NULL, test_command_generic);
	test_bredrle("Pair Device - Legacy Success 1",
				&pair_device_success_test_1,
				NULL, test_command_generic);
	test_bredrle("Pair Device - Legacy Non-bondable 1",
				&pair_device_legacy_nonbondable_1,
				NULL, test_command_generic);
	test_bredrle("Pair Device - Sec Mode 3 Success 1",
				&pair_device_success_test_2,
				NULL, test_command_generic);
	test_bredrle("Pair Device - Legacy Reject 1",
				&pair_device_reject_test_1,
				NULL, test_command_generic);
	test_bredrle("Pair Device - Legacy Reject 2",
				&pair_device_reject_test_2,
				NULL, test_command_generic);
	test_bredrle("Pair Device - Sec Mode 3 Reject 1",
				&pair_device_reject_test_3,
				NULL, test_command_generic);
	test_bredrle("Pair Device - Sec Mode 3 Reject 2",
				&pair_device_reject_test_4,
				NULL, test_command_generic);
	test_bredrle("Pair Device - SSP Just-Works Success 1",
				&pair_device_ssp_test_1,
				NULL, test_command_generic);
	test_bredrle("Pair Device - SSP Just-Works Success 2",
				&pair_device_ssp_test_2,
				NULL, test_command_generic);
	test_bredrle("Pair Device - SSP Just-Works Success 3",
				&pair_device_ssp_test_3,
				NULL, test_command_generic);
	test_bredrle("Pair Device - SSP Confirm Success 1",
				&pair_device_ssp_test_4,
				NULL, test_command_generic);
	test_bredrle("Pair Device - SSP Confirm Success 2",
				&pair_device_ssp_test_5,
				NULL, test_command_generic);
	test_bredrle("Pair Device - SSP Confirm Success 3",
				&pair_device_ssp_test_6,
				NULL, test_command_generic);
	test_bredrle("Pair Device - SSP Confirm Reject 1",
				&pair_device_ssp_reject_1,
				NULL, test_command_generic);
	test_bredrle("Pair Device - SSP Confirm Reject 2",
				&pair_device_ssp_reject_2,
				NULL, test_command_generic);
	test_bredrle("Pair Device - SSP Non-bondable 1",
				&pair_device_ssp_nonbondable_1,
				NULL, test_command_generic);
	test_bredrle("Pair Device - SMP over BR/EDR Success 1",
				&pair_device_smp_bredr_test_1,
				NULL, test_command_generic);
	test_bredrle("Pair Device - SMP over BR/EDR Success 2",
				&pair_device_smp_bredr_test_2,
				NULL, test_command_generic);
	test_le("Pair Device - LE Success 1",
				&pair_device_le_success_test_1,
				NULL, test_command_generic);
	test_le("Pair Device - LE Success 2",
				&pair_device_le_success_test_2,
				NULL, test_command_generic);
	test_le("Pair Device - LE Reject 1",
				&pair_device_le_reject_test_1,
				NULL, test_command_generic);
	test_le("Pair Device - LE SC Legacy 1",
				&pair_device_le_sc_legacy_test_1,
				NULL, test_command_generic);
	test_le("Pair Device - LE SC Success 1",
				&pair_device_le_sc_success_test_1,
				NULL, test_command_generic);
	test_le("Pair Device - LE SC Success 2",
				&pair_device_le_sc_success_test_2,
				NULL, test_command_generic);
	test_bredrle("Pair Device - LE SC Success 3",
				&pair_device_le_sc_success_test_3,
				NULL, test_command_generic);

	test_bredrle("Pairing Acceptor - Legacy 1",
				&pairing_acceptor_legacy_1, NULL,
				test_pairing_acceptor);
	test_bredrle("Pairing Acceptor - Legacy 2",
				&pairing_acceptor_legacy_2, NULL,
				test_pairing_acceptor);
	test_bredrle("Pairing Acceptor - Legacy 3",
				&pairing_acceptor_legacy_3, NULL,
				test_pairing_acceptor);
	test_bredrle("Pairing Acceptor - Link Sec 1",
				&pairing_acceptor_linksec_1, NULL,
				test_pairing_acceptor);
	test_bredrle("Pairing Acceptor - Link Sec 2",
				&pairing_acceptor_linksec_2, NULL,
				test_pairing_acceptor);
	test_bredrle("Pairing Acceptor - SSP 1",
				&pairing_acceptor_ssp_1, setup_pairing_acceptor,
				test_pairing_acceptor);
	test_bredrle("Pairing Acceptor - SSP 2",
				&pairing_acceptor_ssp_2, setup_pairing_acceptor,
				test_pairing_acceptor);
	test_bredrle("Pairing Acceptor - SSP 3",
				&pairing_acceptor_ssp_3, setup_pairing_acceptor,
				test_pairing_acceptor);
	test_bredrle("Pairing Acceptor - SSP 4",
				&pairing_acceptor_ssp_4, setup_pairing_acceptor,
				test_pairing_acceptor);
	test_bredrle("Pairing Acceptor - SMP over BR/EDR 1",
				&pairing_acceptor_smp_bredr_1,
				setup_pairing_acceptor, test_pairing_acceptor);
	test_bredrle("Pairing Acceptor - SMP over BR/EDR 2",
				&pairing_acceptor_smp_bredr_2,
				setup_pairing_acceptor, test_pairing_acceptor);
	test_le("Pairing Acceptor - LE 1",
				&pairing_acceptor_le_1, setup_pairing_acceptor,
				test_pairing_acceptor);
	test_le("Pairing Acceptor - LE 2",
				&pairing_acceptor_le_2, setup_pairing_acceptor,
				test_pairing_acceptor);
	test_le("Pairing Acceptor - LE 3",
				&pairing_acceptor_le_3, setup_pairing_acceptor,
				test_pairing_acceptor);
	test_le("Pairing Acceptor - LE 4",
				&pairing_acceptor_le_4, setup_pairing_acceptor,
				test_pairing_acceptor);
	test_le("Pairing Acceptor - LE 5",
				&pairing_acceptor_le_5, setup_pairing_acceptor,
				test_pairing_acceptor);

	test_bredrle("Unpair Device - Not Powered 1",
				&unpair_device_not_powered_test_1,
				NULL, test_command_generic);
	test_bredrle("Unpair Device - Invalid Parameters 1",
				&unpair_device_invalid_param_test_1,
				NULL, test_command_generic);
	test_bredrle("Unpair Device - Invalid Parameters 2",
				&unpair_device_invalid_param_test_2,
				NULL, test_command_generic);

	test_bredrle("Disconnect - Invalid Parameters 1",
				&disconnect_invalid_param_test_1,
				NULL, test_command_generic);

	test_bredrle("Block Device - Invalid Parameters 1",
				&block_device_invalid_param_test_1,
				NULL, test_command_generic);

	test_bredrle("Unblock Device - Invalid Parameters 1",
				&unblock_device_invalid_param_test_1,
				NULL, test_command_generic);

	test_le("Set Static Address - Success 1",
				&set_static_addr_success_test,
				NULL, test_command_generic);
	test_bredrle("Set Static Address - Success 2",
				&set_static_addr_success_test_2,
				NULL, test_command_generic);
	test_bredrle("Set Static Address - Failure 1",
				&set_static_addr_failure_test,
				NULL, test_command_generic);
	test_bredr("Set Static Address - Failure 2",
				&set_static_addr_failure_test_2,
				NULL, test_command_generic);

	test_bredrle("Set Scan Parameters - Success",
				&set_scan_params_success_test,
				NULL, test_command_generic);

	test_bredrle("Load IRKs - Success 1",
				&load_irks_success1_test,
				NULL, test_command_generic);
	test_bredrle("Load IRKs - Success 2",
				&load_irks_success2_test,
				NULL, test_command_generic);
	test_bredrle("Load IRKs - Invalid Parameters 1",
				&load_irks_nval_param1_test,
				NULL, test_command_generic);
	test_bredrle("Load IRKs - Invalid Parameters 2",
				&load_irks_nval_param2_test,
				NULL, test_command_generic);
	test_bredrle("Load IRKs - Invalid Parameters 3",
				&load_irks_nval_param3_test,
				NULL, test_command_generic);
	test_bredr("Load IRKs - Not Supported",
				&load_irks_not_supported_test,
				NULL, test_command_generic);

	test_bredrle("Set Privacy - Success",
				&set_privacy_success_test,
				NULL, test_command_generic);
	test_bredrle("Set Privacy - Rejected",
				&set_privacy_powered_test,
				NULL, test_command_generic);
	test_bredrle("Set Privacy - Invalid Parameters",
				&set_privacy_nval_param_test,
				NULL, test_command_generic);

	test_bredrle("Get Conn Info - Success",
				&get_conn_info_succes1_test, NULL,
				test_command_generic_connect);
	test_bredrle("Get Conn Info - Not Connected",
				&get_conn_info_ncon_test, NULL,
				test_command_generic);
	test_bredrle("Get Conn Info - Power off",
				&get_conn_info_power_off_test, NULL,
				test_command_generic_connect);

	test_bredrle("Load Connection Parameters - Invalid Params 1",
				&load_conn_params_fail_1,
				NULL, test_command_generic);

	test_bredrle("Add Device - Invalid Params 1",
				&add_device_fail_1,
				NULL, test_command_generic);
	test_bredrle("Add Device - Invalid Params 2",
				&add_device_fail_2,
				NULL, test_command_generic);
	test_bredrle("Add Device - Invalid Params 3",
				&add_device_fail_3,
				NULL, test_command_generic);
	test_bredrle("Add Device - Invalid Params 4",
				&add_device_fail_4,
				NULL, test_command_generic);
	test_bredrle("Add Device - Success 1",
				&add_device_success_1,
				NULL, test_command_generic);
	test_bredrle("Add Device - Success 2",
				&add_device_success_2,
				NULL, test_command_generic);
	test_bredrle("Add Device - Success 3",
				&add_device_success_3,
				NULL, test_command_generic);
	test_bredrle("Add Device - Success 4",
				&add_device_success_4,
				NULL, test_command_generic);
	test_bredrle("Add Device - Success 5",
				&add_device_success_5,
				NULL, test_command_generic);

	test_bredrle("Remove Device - Invalid Params 1",
				&remove_device_fail_1,
				NULL, test_command_generic);
	test_bredrle("Remove Device - Invalid Params 2",
				&remove_device_fail_2,
				NULL, test_command_generic);
	test_bredrle("Remove Device - Invalid Params 3",
				&remove_device_fail_3,
				NULL, test_command_generic);
	test_bredrle("Remove Device - Success 1",
				&remove_device_success_1,
				setup_add_device, test_command_generic);
	test_bredrle("Remove Device - Success 2",
				&remove_device_success_2,
				setup_add_device, test_command_generic);
	test_bredrle("Remove Device - Success 3",
				&remove_device_success_3,
				setup_add_device, test_remove_device);
	test_le("Remove Device - Success 4",
				&remove_device_success_4,
				setup_add_device, test_remove_device);
	test_le("Remove Device - Success 5",
				&remove_device_success_5,
				setup_add_device, test_remove_device);

	test_bredrle("Read Advertising Features - Invalid parameters",
				&read_adv_features_invalid_param_test,
				NULL, test_command_generic);
	test_bredrle("Read Advertising Features - Invalid index",
				&read_adv_features_invalid_index_test,
				NULL, test_command_generic);
	test_bredrle("Read Advertising Features - Success 1 (No instance)",
				&read_adv_features_success_1,
				NULL, test_command_generic);
	test_bredrle("Read Advertising Features - Success 2 (One instance)",
				&read_adv_features_success_2,
				setup_add_advertising,
				test_command_generic);

	test_bredrle("Add Advertising - Failure: LE off",
					&add_advertising_fail_1,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Invalid Params 1 (AD too long)",
					&add_advertising_fail_2,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Invalid Params 2 (Malformed len)",
					&add_advertising_fail_3,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Invalid Params 3 (Malformed len)",
					&add_advertising_fail_4,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Invalid Params 4 (Malformed len)",
					&add_advertising_fail_5,
					NULL, test_command_generic);
	test_le("Add Advertising - Invalid Params 5 (AD too long)",
					&add_advertising_fail_6,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Invalid Params 6 (ScRsp too long)",
					&add_advertising_fail_7,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Invalid Params 7 (Malformed len)",
					&add_advertising_fail_8,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Invalid Params 8 (Malformed len)",
					&add_advertising_fail_9,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Invalid Params 9 (Malformed len)",
					&add_advertising_fail_10,
					NULL, test_command_generic);
	test_le("Add Advertising - Invalid Params 10 (ScRsp too long)",
					&add_advertising_fail_11,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Rejected (Timeout, !Powered)",
					&add_advertising_fail_12,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Success 1 (Powered, Add Adv Inst)",
					&add_advertising_success_1,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Success 2 (!Powered, Add Adv Inst)",
					&add_advertising_success_pwron_data,
					setup_add_advertising_not_powered,
					test_command_generic);
	test_bredrle("Add Advertising - Success 3 (!Powered, Adv Enable)",
					&add_advertising_success_pwron_enabled,
					setup_add_advertising_not_powered,
					test_command_generic);
	test_bredrle("Add Advertising - Success 4 (Set Adv on override)",
					&add_advertising_success_4,
					setup_add_advertising,
					test_command_generic);
	test_bredrle("Add Advertising - Success 5 (Set Adv off override)",
					&add_advertising_success_5,
					setup_set_and_add_advertising,
					test_command_generic);
	test_bredrle("Add Advertising - Success 6 (Scan Rsp Dta, Adv ok)",
					&add_advertising_success_6,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Success 7 (Scan Rsp Dta, Scan ok) ",
					&add_advertising_success_7,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Success 8 (Connectable Flag)",
					&add_advertising_success_8,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Success 9 (General Discov Flag)",
					&add_advertising_success_9,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Success 10 (Limited Discov Flag)",
					&add_advertising_success_10,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Success 11 (Managed Flags)",
					&add_advertising_success_11,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Success 12 (TX Power Flag)",
					&add_advertising_success_12,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Success 13 (ADV_SCAN_IND)",
					&add_advertising_success_13,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Success 14 (ADV_NONCONN_IND)",
					&add_advertising_success_14,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Success 15 (ADV_IND)",
					&add_advertising_success_15,
					NULL, test_command_generic);
	test_bredrle("Add Advertising - Success 16 (Connectable -> on)",
					&add_advertising_success_16,
					setup_add_advertising,
					test_command_generic);
	test_bredrle("Add Advertising - Success 17 (Connectable -> off)",
					&add_advertising_success_17,
					setup_add_advertising_connectable,
					test_command_generic);
	/* Adv instances with a timeout do NOT survive a power cycle. */
	test_bredrle("Add Advertising - Success 18 (Power -> off, Remove)",
					&add_advertising_power_off,
					setup_add_advertising_timeout,
					test_command_generic);
	/* Adv instances without timeout survive a power cycle. */
	test_bredrle("Add Advertising - Success 19 (Power -> off, Keep)",
					&add_advertising_success_pwron_data,
					setup_add_advertising_power_cycle,
					test_command_generic);
	/* Changing an advertising instance while it is still being
	 * advertised will immediately update the advertised data if
	 * there is no other instance to switch to.
	 */
	test_bredrle("Add Advertising - Success 20 (Add Adv override)",
					&add_advertising_success_18,
					setup_add_advertising,
					test_command_generic);
	/* An instance should be removed when its timeout has been reached.
	 * Advertising will also be disabled if this was the last instance.
	 */
	test_bredrle_full("Add Advertising - Success 21 (Timeout expires)",
					&add_advertising_timeout_expired,
					setup_add_advertising_timeout,
					test_command_generic, 3);
	/* LE off will clear (remove) all instances. */
	test_bredrle("Add Advertising - Success 22 (LE -> off, Remove)",
					&add_advertising_le_off,
					setup_add_advertising,
					test_command_generic);


	test_bredrle("Remove Advertising - Invalid Params 1",
					&remove_advertising_fail_1,
					NULL, test_command_generic);
	test_bredrle("Remove Advertising - Success 1",
						&remove_advertising_success_1,
						setup_add_advertising,
						test_command_generic);
	test_bredrle("Remove Advertising - Success 2",
						&remove_advertising_success_2,
						setup_add_advertising,
						test_command_generic);

	/* When advertising two instances, the instances should be
	 * advertised in a round-robin fashion.
	 */
	test_bredrle("Multi Advertising - Success 1 (Instance Switch)",
					&multi_advertising_switch,
					setup_multi_adv,
					test_command_generic);
	/* Adding a new instance when one is already being advertised
	 * will switch to the new instance after the first has reached
	 * its duration. A long timeout has been set to
	 */
	test_bredrle_full("Multi Advertising - Success 2 (Add Second Inst)",
					&multi_advertising_add_second,
					setup_add_advertising_duration,
					test_command_generic, 3);

	test_bredrle("Read Local OOB Data - Not powered",
				&read_local_oob_not_powered_test,
				NULL, test_command_generic);
	test_bredrle("Read Local OOB Data - Invalid parameters",
				&read_local_oob_invalid_param_test,
				NULL, test_command_generic);
	test_bredrle("Read Local OOB Data - Invalid index",
				&read_local_oob_invalid_index_test,
				NULL, test_command_generic);
	test_bredr20("Read Local OOB Data - Legacy pairing",
				&read_local_oob_legacy_pairing_test,
				NULL, test_command_generic);
	test_bredrle("Read Local OOB Data - Success SSP",
				&read_local_oob_success_ssp_test,
				NULL, test_command_generic);
	test_bredrle("Read Local OOB Data - Success SC",
				&read_local_oob_success_sc_test,
				NULL, test_command_generic);

	test_bredrle("Device Found - Advertising data - Zero padded",
				&device_found_gtag,
				NULL, test_device_found);
	test_bredrle("Device Found - Advertising data - Invalid field",
				&device_found_invalid_field,
				NULL, test_device_found);

	return tester_run();
}
