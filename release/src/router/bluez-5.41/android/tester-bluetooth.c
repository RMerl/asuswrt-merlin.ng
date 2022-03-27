/*
 * Copyright (C) 2014 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <stdbool.h>

#include "emulator/bthost.h"
#include "src/shared/tester.h"
#include "src/shared/queue.h"
#include "tester-main.h"

static struct queue *list; /* List of bluetooth test cases */

static bt_bdaddr_t emu_bdaddr_val = {
	.address = { 0x00, 0xaa, 0x01, 0x00, 0x00, 0x00 },
};
static bt_property_t prop_emu_bdaddr = {
	.type = BT_PROPERTY_BDADDR,
	.val = &emu_bdaddr_val,
	.len = sizeof(emu_bdaddr_val),
};

static char emu_bdname_val[] = "BlueZ for Android";
static bt_property_t prop_emu_bdname = {
	.type = BT_PROPERTY_BDNAME,
	.val = &emu_bdname_val,
	.len = sizeof(emu_bdname_val) - 1,
};

static char emu_uuids_val[] = {
	/* Multi profile UUID */
	0x00, 0x00, 0x11, 0x3b, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00,
					0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB,
	/* Device identification profile UUID */
	0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00,
					0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB,
};
static bt_property_t prop_emu_uuids = {
	.type = BT_PROPERTY_UUIDS,
	.val = &emu_uuids_val,
	.len = sizeof(emu_uuids_val),
};

static uint32_t emu_cod_val = 0x00020c;
static bt_property_t prop_emu_cod = {
	.type = BT_PROPERTY_CLASS_OF_DEVICE,
	.val = &emu_cod_val,
	.len = sizeof(emu_cod_val),
};

static bt_device_type_t emu_tod_dual_val = BT_DEVICE_DEVTYPE_DUAL;
static bt_property_t prop_emu_dual_tod = {
	.type = BT_PROPERTY_TYPE_OF_DEVICE,
	.val = &emu_tod_dual_val,
	.len = sizeof(emu_tod_dual_val),
};

static bt_scan_mode_t emu_scan_mode_val = BT_SCAN_MODE_NONE;
static bt_property_t prop_emu_scan_mode = {
	.type = BT_PROPERTY_ADAPTER_SCAN_MODE,
	.val = &emu_scan_mode_val,
	.len = sizeof(emu_scan_mode_val),
};

static uint32_t emu_disc_timeout_val = 120;
static bt_property_t prop_emu_disc_timeout = {
	.type = BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT,
	.val = &emu_disc_timeout_val,
	.len = sizeof(emu_disc_timeout_val),
};

static bt_property_t prop_emu_bonded_devs = {
	.type = BT_PROPERTY_ADAPTER_BONDED_DEVICES,
	.val = NULL,
	.len = 0,
};

static bt_bdaddr_t emu_remote_bdaddr_val = {
	.address = { 0x00, 0xaa, 0x01, 0x01, 0x00, 0x00 },
};
static bt_property_t prop_emu_remote_bdadr = {
	.type = BT_PROPERTY_BDADDR,
	.val = &emu_remote_bdaddr_val,
	.len = sizeof(emu_remote_bdaddr_val),
};
static struct bt_action_data prop_emu_remote_ble_bdaddr_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_BDADDR,
};

static uint32_t emu_remote_type_val = BT_DEVICE_DEVTYPE_BREDR;

static uint32_t emu_remote_tod_ble_val = BT_DEVICE_DEVTYPE_BLE;
static bt_property_t prop_emu_remote_ble_tod_prop = {
	.type = BT_PROPERTY_TYPE_OF_DEVICE,
	.val = &emu_remote_tod_ble_val,
	.len = sizeof(emu_remote_tod_ble_val),
};
static struct bt_action_data prop_emu_remote_ble_tod_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_TYPE_OF_DEVICE,
};

static int32_t emu_remote_rssi_val = -60;

static int32_t emu_remote_ble_rssi_val = 127;
static bt_property_t prop_emu_remote_ble_rssi_prop = {
	.type = BT_PROPERTY_REMOTE_RSSI,
	.val = &emu_remote_ble_rssi_val,
	.len = sizeof(emu_remote_ble_rssi_val),
};
static struct bt_action_data prop_emu_remote_ble_rssi_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_REMOTE_RSSI,
};

static char emu_remote_bdname_val[] = "00:AA:01:01:00:00";
static bt_property_t prop_emu_remote_ble_bdname_prop = {
	.type = BT_PROPERTY_BDNAME,
	.val = &emu_remote_bdname_val,
	.len = sizeof(emu_remote_bdname_val) - 1,
};
static struct bt_action_data prop_emu_remote_ble_bdname_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_BDNAME,
};

static uint32_t emu_remote_cod_val = 0;
static bt_property_t prop_emu_remote_ble_cod_prop = {
	.type = BT_PROPERTY_CLASS_OF_DEVICE,
	.val = &emu_remote_cod_val,
	.len = sizeof(emu_remote_cod_val),
};
static struct bt_action_data prop_emu_remote_ble_cod_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_CLASS_OF_DEVICE,
};

static bt_property_t prop_emu_remote_ble_uuids_prop = {
	.type = BT_PROPERTY_UUIDS,
	.val = NULL,
	.len = 0,
};
static struct bt_action_data prop_emu_remote_ble_uuids_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_UUIDS,
};

static bt_property_t prop_emu_remote_ble_timestamp_prop = {
	.type = BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP,
	.val = NULL,
	.len = 4,
};
static struct bt_action_data prop_emu_remote_ble_timestamp_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP,
};

static struct bt_action_data prop_emu_remote_ble_scan_mode_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_ADAPTER_SCAN_MODE,
};

static struct bt_action_data prop_emu_remote_ble_bondeddev_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_ADAPTER_BONDED_DEVICES,
};

static struct bt_action_data prop_emu_remote_ble_disctimeout_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT,
};

static struct bt_action_data prop_emu_remote_ble_verinfo_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_REMOTE_VERSION_INFO,
};

static char prop_test_fname_val[] = "FriendlyTestName";
static bt_property_t prop_emu_remote_ble_fname_prop = {
	.type = BT_PROPERTY_REMOTE_FRIENDLY_NAME,
	.val = &prop_test_fname_val,
	.len = sizeof(prop_test_fname_val) - 1,
};
static struct bt_action_data prop_emu_remote_ble_fname_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_REMOTE_FRIENDLY_NAME,
	.prop = &prop_emu_remote_ble_fname_prop,
};

static bt_pin_code_t emu_pin_value = {
	.pin = { 0x30, 0x30, 0x30, 0x30 },
};
static bt_pin_code_t emu_pin_invalid_value = {
	.pin = { 0x30, 0x10, 0x30, 0x30 },
};
static struct bt_action_data emu_pin_set_req = {
	.addr = &emu_remote_bdaddr_val,
	.pin = &emu_pin_value,
	.pin_len = 4,
};
static struct bt_action_data emu_pin_set_invalid_req = {
	.addr = &emu_remote_bdaddr_val,
	.pin = &emu_pin_invalid_value,
	.pin_len = 4,
};

static bt_property_t prop_emu_default_set[] = {
	{ BT_PROPERTY_BDADDR, sizeof(emu_bdaddr_val), NULL },
	{ BT_PROPERTY_BDNAME, sizeof(emu_bdname_val) - 1, &emu_bdname_val },
	{ BT_PROPERTY_CLASS_OF_DEVICE, sizeof(uint32_t), NULL },
	{ BT_PROPERTY_TYPE_OF_DEVICE, sizeof(emu_tod_dual_val),
							&emu_tod_dual_val },
	{ BT_PROPERTY_ADAPTER_SCAN_MODE, sizeof(emu_scan_mode_val),
							&emu_scan_mode_val },
	{ BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT, sizeof(emu_disc_timeout_val),
							&emu_disc_timeout_val},
	{ BT_PROPERTY_ADAPTER_BONDED_DEVICES, 0, NULL },
	{ BT_PROPERTY_UUIDS, sizeof(emu_uuids_val), &emu_uuids_val },
};

static bt_property_t prop_emu_remote_bles_default_set[] = {
	{ BT_PROPERTY_BDADDR, sizeof(emu_remote_bdaddr_val),
						&emu_remote_bdaddr_val },
	{ BT_PROPERTY_TYPE_OF_DEVICE, sizeof(emu_remote_tod_ble_val),
						&emu_remote_tod_ble_val },
	{ BT_PROPERTY_REMOTE_RSSI, sizeof(emu_remote_ble_rssi_val),
						&emu_remote_ble_rssi_val },
};

static bt_property_t prop_emu_remotes_default_set[] = {
	{ BT_PROPERTY_BDADDR, sizeof(emu_remote_bdaddr_val),
						&emu_remote_bdaddr_val },
	{ BT_PROPERTY_TYPE_OF_DEVICE, sizeof(emu_remote_type_val),
						&emu_remote_type_val },
	{ BT_PROPERTY_REMOTE_RSSI, sizeof(emu_remote_rssi_val),
						&emu_remote_rssi_val },
};

static bt_property_t prop_emu_remote_bles_query_set[] = {
	{ BT_PROPERTY_TYPE_OF_DEVICE, sizeof(emu_remote_tod_ble_val),
						&emu_remote_tod_ble_val },
	{ BT_PROPERTY_CLASS_OF_DEVICE, sizeof(emu_remote_cod_val),
							&emu_remote_cod_val },
	{ BT_PROPERTY_REMOTE_RSSI, sizeof(emu_remote_ble_rssi_val),
						&emu_remote_ble_rssi_val },
	{ BT_PROPERTY_BDNAME, sizeof(emu_remote_bdname_val) - 1,
						&emu_remote_bdname_val },
	{ BT_PROPERTY_UUIDS, 0, NULL },
	{ BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP, 4, NULL },
};

static bt_property_t prop_emu_remotes_pin_req_set[] = {
	{ BT_PROPERTY_BDADDR, sizeof(emu_remote_bdaddr_val),
						&emu_remote_bdaddr_val },
	{ BT_PROPERTY_CLASS_OF_DEVICE, sizeof(emu_remote_cod_val),
						&emu_remote_cod_val },
	{ BT_PROPERTY_BDNAME, sizeof(emu_remote_bdname_val) - 1,
						&emu_remote_bdname_val },
};

static char test_bdname[] = "test_bdname";
static bt_property_t prop_test_bdname = {
	.type = BT_PROPERTY_BDNAME,
	.val = test_bdname,
	.len = sizeof(test_bdname) - 1,
};
static struct bt_action_data prop_test_remote_ble_bdname_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_BDNAME,
	.prop = &prop_test_bdname,
};

static bt_scan_mode_t test_scan_mode_connectable_discoverable =
					BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE;
static bt_property_t prop_test_scanmode_conn_discov = {
	.type = BT_PROPERTY_ADAPTER_SCAN_MODE,
	.val = &test_scan_mode_connectable_discoverable,
	.len = sizeof(bt_scan_mode_t),
};

static uint32_t test_disctimeout_val = 600;
static bt_property_t prop_test_disctimeout = {
	.type = BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT,
	.val = &test_disctimeout_val,
	.len = sizeof(test_disctimeout_val),
};
static struct bt_action_data prop_test_remote_ble_disc_timeout_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT,
	.prop = &prop_test_disctimeout,
};

static unsigned char test_uuids_val[] = { 0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00,
			0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00 };
static bt_property_t prop_test_uuid = {
	.type = BT_PROPERTY_UUIDS,
	.val = &test_uuids_val,
	.len = sizeof(test_uuids_val),
};
static struct bt_action_data prop_test_remote_ble_uuids_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_UUIDS,
	.prop = &prop_test_uuid,
};

static uint32_t test_cod_val = 0;
static bt_property_t prop_test_cod = {
	.type = BT_PROPERTY_CLASS_OF_DEVICE,
	.val = &test_cod_val,
	.len = sizeof(test_cod_val),
};
static struct bt_action_data prop_test_remote_ble_cod_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_CLASS_OF_DEVICE,
	.prop = &prop_test_cod,
};

static uint32_t test_tod_val = BT_DEVICE_DEVTYPE_BLE;
static bt_property_t prop_test_tod = {
	.type = BT_PROPERTY_TYPE_OF_DEVICE,
	.val = &test_tod_val,
	.len = sizeof(test_tod_val),
};
static struct bt_action_data prop_test_remote_ble_tod_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_TYPE_OF_DEVICE,
	.prop = &prop_test_tod,
};

static int32_t test_remote_rssi_val = -9;
static bt_property_t prop_test_remote_rssi = {
	.type = BT_PROPERTY_REMOTE_RSSI,
	.val = &test_remote_rssi_val,
	.len = sizeof(test_remote_rssi_val),
};
static struct bt_action_data prop_test_remote_ble_rssi_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_REMOTE_RSSI,
	.prop = &prop_test_remote_rssi,
};

static bt_service_record_t test_srvc_record_val =  {
	.uuid = { {0x00} },
	.channel = 12,
	.name = "bt_name",
};
static bt_property_t prop_test_srvc_record = {
	.type = BT_PROPERTY_SERVICE_RECORD,
	.val = &test_srvc_record_val,
	.len = sizeof(test_srvc_record_val),
};
static struct bt_action_data prop_test_remote_ble_srvc_record_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_SERVICE_RECORD,
	.prop = &prop_test_srvc_record,
};

static bt_bdaddr_t test_bdaddr_val = {
	.address = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
};
static bt_property_t prop_test_bdaddr = {
	.type = BT_PROPERTY_BDADDR,
	.val = &test_bdaddr_val,
	.len = sizeof(test_bdaddr_val),
};
static struct bt_action_data prop_test_remote_ble_bdaddr_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_BDADDR,
	.prop = &prop_test_bdaddr,
};
static struct bt_action_data prop_test_bdaddr_req = {
	.addr = &test_bdaddr_val,
	.prop_type = BT_PROPERTY_BDADDR,
	.prop = &prop_test_bdaddr,
};

static bt_scan_mode_t setprop_scan_mode_conn_val = BT_SCAN_MODE_CONNECTABLE;

static bt_property_t prop_test_scan_mode_conn = {
	.type = BT_PROPERTY_ADAPTER_SCAN_MODE,
	.val = &setprop_scan_mode_conn_val,
	.len = sizeof(setprop_scan_mode_conn_val),
};

static bt_scan_mode_t test_scan_mode_none_val = BT_SCAN_MODE_NONE;
static bt_property_t prop_test_scan_mode_none = {
	.type = BT_PROPERTY_ADAPTER_SCAN_MODE,
	.val = &test_scan_mode_none_val,
	.len = sizeof(test_scan_mode_none_val),
};
static struct bt_action_data prop_test_remote_ble_scanmode_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_ADAPTER_SCAN_MODE,
	.prop = &prop_test_scan_mode_none,
};

static bt_bdaddr_t test_bonded_dev_addr_val = {
	.address = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 },
};
static bt_property_t prop_test_bonded_dev_addr = {
	.type = BT_PROPERTY_ADAPTER_BONDED_DEVICES,
	.val = &test_bonded_dev_addr_val,
	.len = sizeof(test_bonded_dev_addr_val),
};
static struct bt_action_data prop_test_ble_bonded_dev_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_ADAPTER_BONDED_DEVICES,
	.prop = &prop_test_bonded_dev_addr,
};

static uint32_t test_remote_timestamp_val = 42;
static bt_property_t prop_test_remote_ble_timestamp_prop = {
	.type = BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP,
	.val = &test_remote_timestamp_val,
	.len = sizeof(test_remote_timestamp_val),
};
static struct bt_action_data prop_test_remote_ble_timestamp_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP,
	.prop = &prop_test_remote_ble_timestamp_prop,
};

static struct bt_action_data ssp_confirm_accept_reply = {
	.addr = &emu_remote_bdaddr_val,
	.ssp_variant = BT_SSP_VARIANT_PASSKEY_CONFIRMATION,
	.accept = TRUE,
};

static struct bt_action_data ssp_confirm_reject_reply = {
	.addr = &emu_remote_bdaddr_val,
	.ssp_variant = BT_SSP_VARIANT_PASSKEY_CONFIRMATION,
	.accept = FALSE,
};

static  struct bt_action_data no_input_no_output_io_cap = {
	.io_cap = 0x03,
};

static  struct bt_action_data display_yes_no_io_cap = {
	.io_cap = 0x01,
};

static uint16_t test_conn_handle = 0;

static void conn_cb(uint16_t handle, void *user_data)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);

	tester_print("New connection with handle 0x%04x", handle);

	test_conn_handle = handle;

	bthost_request_auth(bthost, handle);
}

static struct test_case test_cases[] = {
	TEST_CASE_BREDRLE("Bluetooth Init",
		ACTION_SUCCESS(dummy_action, NULL),
	),
	TEST_CASE_BREDRLE("Bluetooth Enable - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_ADAPTER_PROPS(prop_emu_default_set, 8),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
	),
	TEST_CASE_BREDRLE("Bluetooth Enable - Success 2",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_ADAPTER_PROPS(prop_emu_default_set, 8),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
	),
	TEST_CASE_BREDRLE("Bluetooth Disable - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Bluetooth Set BDNAME - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_set_property_action, &prop_test_bdname),
		CALLBACK_ADAPTER_PROPS(&prop_test_bdname, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Set SCAN_MODE - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_set_property_action,
					&prop_test_scanmode_conn_discov),
		CALLBACK_ADAPTER_PROPS(&prop_test_scanmode_conn_discov, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Set DISCOVERY_TIMEOUT - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_set_property_action, &prop_test_disctimeout),
		CALLBACK_ADAPTER_PROPS(&prop_test_disctimeout, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Get BDADDR - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_get_property_action, &prop_emu_bdaddr),
		CALLBACK_ADAPTER_PROPS(&prop_emu_bdaddr, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Get BDNAME - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_get_property_action, &prop_emu_bdname),
		CALLBACK_ADAPTER_PROPS(&prop_emu_bdname, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Set UUID - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_FAIL(bt_set_property_action, &prop_test_uuid),
	),
	TEST_CASE_BREDRLE("Bluetooth Set CLASS_OF_DEVICE - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_FAIL(bt_set_property_action, &prop_test_cod),
	),
	TEST_CASE_BREDRLE("Bluetooth Set TYPE_OF_DEVICE - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_FAIL(bt_set_property_action, &prop_test_tod),
	),
	TEST_CASE_BREDRLE("Bluetooth Set REMOTE_RSSI - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_FAIL(bt_set_property_action, &prop_test_remote_rssi),
	),
	TEST_CASE_BREDRLE("Bluetooth Set SERVICE_RECORD - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_FAIL(bt_set_property_action, &prop_test_srvc_record),
	),
	TEST_CASE_BREDRLE("Bluetooth Set BDADDR - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_FAIL(bt_set_property_action, &prop_test_bdaddr),
	),
	TEST_CASE_BREDRLE("Bluetooth Set SCAN_MODE_CONNECTABLE - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_set_property_action,
						&prop_test_scan_mode_conn),
		CALLBACK_ADAPTER_PROPS(&prop_test_scan_mode_conn, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Set BONDED_DEVICES - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_FAIL(bt_set_property_action, &prop_test_bonded_dev_addr),
	),
	TEST_CASE_BREDRLE("Bluetooth Get CLASS_OF_DEVICE - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_get_property_action, &prop_emu_cod),
		CALLBACK_ADAPTER_PROPS(&prop_emu_cod, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Get TYPE_OF_DEVICE - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_get_property_action, &prop_emu_dual_tod),
		CALLBACK_ADAPTER_PROPS(&prop_emu_dual_tod, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Get SCAN_MODE - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_get_property_action, &prop_emu_scan_mode),
		CALLBACK_ADAPTER_PROPS(&prop_emu_scan_mode, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Get DISCOVERY_TIMEOUT - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_get_property_action, &prop_emu_disc_timeout),
		CALLBACK_ADAPTER_PROPS(&prop_emu_disc_timeout, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Get UUIDS - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_get_property_action, &prop_emu_uuids),
		CALLBACK_ADAPTER_PROPS(&prop_emu_uuids, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Get BONDED_DEVICES - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_get_property_action, &prop_emu_bonded_devs),
		CALLBACK_ADAPTER_PROPS(&prop_emu_bonded_devs, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Set SCAN_MODE - Success 2",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_set_property_action,
						&prop_test_scan_mode_none),
		CALLBACK_ADAPTER_PROPS(&prop_test_scan_mode_none, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Discovery Start - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
	),
	TEST_CASE_BREDRLE("Bluetooth Discovery Start - Done",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
	),
	TEST_CASE_BREDRLE("Bluetooth Discovery Stop - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
	),
	TEST_CASE_BREDRLE("Bluetooth Discovery Stop - Done",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
	),
	TEST_CASE_BREDRLE("Bluetooth Discovery Device Found",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remote_bles_default_set, 3),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Get Props - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_get_device_props_action,
							&emu_remote_bdaddr_val),
		CALLBACK_DEVICE_PROPS(prop_emu_remote_bles_query_set, 6),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Get BDNAME - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_get_device_prop_action,
					&prop_emu_remote_ble_bdname_req),
		CALLBACK_DEVICE_PROPS(&prop_emu_remote_ble_bdname_prop, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Get UUIDS - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_get_device_prop_action,
						&prop_emu_remote_ble_uuids_req),
		CALLBACK_DEVICE_PROPS(&prop_emu_remote_ble_uuids_prop, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Get COD - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_get_device_prop_action,
						&prop_emu_remote_ble_cod_req),
		CALLBACK_DEVICE_PROPS(&prop_emu_remote_ble_cod_prop, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Get TOD - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_get_device_prop_action,
						&prop_emu_remote_ble_tod_req),
		CALLBACK_DEVICE_PROPS(&prop_emu_remote_ble_tod_prop, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Get RSSI - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_get_device_prop_action,
						&prop_emu_remote_ble_rssi_req),
		CALLBACK_DEVICE_PROPS(&prop_emu_remote_ble_rssi_prop, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Get TIMESTAMP - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_get_device_prop_action,
					&prop_emu_remote_ble_timestamp_req),
		CALLBACK_DEVICE_PROPS(&prop_emu_remote_ble_timestamp_prop, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Get BDADDR - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_get_device_prop_action,
					&prop_emu_remote_ble_bdaddr_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Get SCAN_MODE - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_get_device_prop_action,
					&prop_emu_remote_ble_scan_mode_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Get BONDED_DEVICES - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_get_device_prop_action,
					&prop_emu_remote_ble_bondeddev_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Get DISCOVERY_TIMEOUT - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_get_device_prop_action,
					&prop_emu_remote_ble_disctimeout_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Get VERSION_INFO - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_get_device_prop_action,
					&prop_emu_remote_ble_verinfo_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Get FRIENDLY_NAME - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_get_device_prop_action,
						&prop_emu_remote_ble_fname_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Set FRIENDLY_NAME - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_set_device_prop_action,
						&prop_emu_remote_ble_fname_req),
		ACTION_SUCCESS(bt_get_device_prop_action,
						&prop_emu_remote_ble_fname_req),
		CALLBACK_DEVICE_PROPS(&prop_emu_remote_ble_fname_prop, 1),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Set BDNAME - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_set_device_prop_action,
					&prop_test_remote_ble_bdname_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Set UUIDS - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_set_device_prop_action,
					&prop_test_remote_ble_uuids_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Set COD - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_set_device_prop_action,
						&prop_test_remote_ble_cod_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Set TOD - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_set_device_prop_action,
						&prop_test_remote_ble_tod_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Set RSSI - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_set_device_prop_action,
						&prop_test_remote_ble_rssi_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Set TIMESTAMP - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_set_device_prop_action,
					&prop_test_remote_ble_timestamp_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Set BDADDR - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_set_device_prop_action,
					&prop_test_remote_ble_bdaddr_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Set SERVICE_RECORD - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_set_device_prop_action,
					&prop_test_remote_ble_srvc_record_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Set SCAN_MODE - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_set_device_prop_action,
					&prop_test_remote_ble_scanmode_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Set BONDED_DEVICES - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_set_device_prop_action,
						&prop_test_ble_bonded_dev_req),
	),
	TEST_CASE_BREDRLE("Bluetooth Device Set DISCOVERY_TIMEOUT - Fail",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_FAIL(bt_set_device_prop_action,
					&prop_test_remote_ble_disc_timeout_req),
	),
	TEST_CASE_BREDR("Bluetooth Create Bond PIN - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_pin_code_action, &emu_pin_set_req),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_set, 3),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_create_bond_action,
					&prop_test_remote_ble_bdaddr_req),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDING,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_PROPS(CB_BT_PIN_REQUEST, prop_emu_remotes_pin_req_set,
									2),
		ACTION_SUCCESS(bt_pin_reply_accept_action,
							&emu_pin_set_req),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDED,
						&prop_emu_remote_bdadr, 1),
	),
	TEST_CASE_BREDR("Bluetooth Create Bond PIN - Bad PIN",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_pin_code_action, &emu_pin_set_req),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_set, 3),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_create_bond_action,
					&prop_test_remote_ble_bdaddr_req),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDING,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_PROPS(CB_BT_PIN_REQUEST, prop_emu_remotes_pin_req_set,
									2),
		ACTION_SUCCESS(bt_pin_reply_accept_action,
						&emu_pin_set_invalid_req),
		CALLBACK_BOND_STATE_FAILED(BT_BOND_STATE_NONE,
						&prop_emu_remote_bdadr, 1,
						BT_STATUS_AUTH_FAILURE),
	),
	TEST_CASE_BREDR("Bluetooth Create Bond SSP -Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_io_cap, &display_yes_no_io_cap),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_set, 3),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_create_bond_action,
					&prop_test_remote_ble_bdaddr_req),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDING,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_SSP_REQ(BT_SSP_VARIANT_PASSKEY_CONFIRMATION,
					prop_emu_remotes_pin_req_set, 2),
		ACTION_SUCCESS(bt_ssp_reply_accept_action,
						&ssp_confirm_accept_reply),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDED,
						&prop_emu_remote_bdadr, 1),
	),
	TEST_CASE_BREDR("Bluetooth Create Bond SSP - Negative reply",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_io_cap, &display_yes_no_io_cap),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_set, 3),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_create_bond_action,
					&prop_test_remote_ble_bdaddr_req),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDING,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_SSP_REQ(BT_SSP_VARIANT_PASSKEY_CONFIRMATION,
					prop_emu_remotes_pin_req_set, 2),
		ACTION_SUCCESS(bt_ssp_reply_accept_action,
						&ssp_confirm_reject_reply),
		CALLBACK_BOND_STATE_FAILED(BT_BOND_STATE_NONE,
						&prop_emu_remote_bdadr, 1,
						BT_STATUS_AUTH_FAILURE),
	),
	TEST_CASE_BREDR("Bluetooth Create Bond - No Discovery",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_io_cap, &display_yes_no_io_cap),
		ACTION_SUCCESS(bt_create_bond_action,
					&prop_test_remote_ble_bdaddr_req),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDING,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_SSP_REQ(BT_SSP_VARIANT_PASSKEY_CONFIRMATION,
					prop_emu_remotes_pin_req_set, 2),
		ACTION_SUCCESS(bt_ssp_reply_accept_action,
						&ssp_confirm_accept_reply),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDED,
						&prop_emu_remote_bdadr, 1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDR("Bluetooth Create Bond - Bad Address",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(bt_create_bond_action, &prop_test_bdaddr_req),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDING,
							&prop_test_bdaddr, 1),
		CALLBACK_BOND_STATE_FAILED(BT_BOND_STATE_NONE,
							&prop_test_bdaddr, 1,
							BT_STATUS_FAIL),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDR("Bluetooth Cancel Bonding - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_io_cap, &display_yes_no_io_cap),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_set, 3),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_create_bond_action,
					&prop_test_remote_ble_bdaddr_req),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDING,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_SSP_REQ(BT_SSP_VARIANT_PASSKEY_CONFIRMATION,
					prop_emu_remotes_pin_req_set, 2),
		ACTION_SUCCESS(bt_cancel_bond_action, &emu_remote_bdaddr_val),
		CALLBACK_BOND_STATE_FAILED(BT_BOND_STATE_NONE,
						&prop_emu_remote_bdadr, 1,
						BT_STATUS_FAIL),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDR("Bluetooth Remove Bond - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_io_cap, &display_yes_no_io_cap),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_set, 3),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STOPPED),
		ACTION_SUCCESS(bt_create_bond_action,
					&prop_test_remote_ble_bdaddr_req),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDING,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_SSP_REQ(BT_SSP_VARIANT_PASSKEY_CONFIRMATION,
					prop_emu_remotes_pin_req_set, 2),
		ACTION_SUCCESS(bt_ssp_reply_accept_action,
						&ssp_confirm_accept_reply),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDED,
						&prop_emu_remote_bdadr, 1),
		ACTION_SUCCESS(bt_remove_bond_action, &emu_remote_bdaddr_val),
		CALLBACK_BOND_STATE(BT_BOND_STATE_NONE,
						&prop_emu_remote_bdadr, 1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDR("Bluetooth Accept Bond - Just Works - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_set_property_action,
					&prop_test_scanmode_conn_discov),
		CALLBACK_ADAPTER_PROPS(&prop_test_scanmode_conn_discov, 1),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_io_cap, &no_input_no_output_io_cap),
		ACTION_SUCCESS(emu_set_connect_cb_action, conn_cb),
		ACTION_SUCCESS(emu_remote_connect_hci_action, NULL),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDING,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDED,
						&prop_emu_remote_bdadr, 1),
		ACTION_SUCCESS(bt_remove_bond_action, &emu_remote_bdaddr_val),
		CALLBACK_BOND_STATE(BT_BOND_STATE_NONE,
						&prop_emu_remote_bdadr, 1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDR("Bluetooth Accept Bond - No Bond - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_set_property_action,
					&prop_test_scanmode_conn_discov),
		CALLBACK_ADAPTER_PROPS(&prop_test_scanmode_conn_discov, 1),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_io_cap, &no_input_no_output_io_cap),
		ACTION_SUCCESS(emu_set_connect_cb_action, conn_cb),
		ACTION_SUCCESS(emu_remote_connect_hci_action, NULL),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDING,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDED,
						&prop_emu_remote_bdadr, 1),
		ACTION_SUCCESS(emu_remote_disconnect_hci_action,
							&test_conn_handle),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDING,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_BOND_STATE(BT_BOND_STATE_NONE,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
};

struct queue *get_bluetooth_tests(void)
{
	uint16_t i = 0;

	list = queue_new();

	for (; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i)
		queue_push_tail(list, &test_cases[i]);

	return list;
}

void remove_bluetooth_tests(void)
{
	queue_destroy(list, NULL);
}
