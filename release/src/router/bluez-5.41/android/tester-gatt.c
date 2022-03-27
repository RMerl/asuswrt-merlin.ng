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
#include "lib/bluetooth.h"
#include "src/shared/util.h"
#include "src/shared/tester.h"
#include "src/shared/queue.h"
#include "tester-main.h"

#define ATT_HANDLE_SIZE	2

#define L2CAP_ATT_ERROR			0x01
#define L2CAP_ATT_EXCHANGE_MTU_REQ	0x02
#define L2CAP_ATT_EXCHANGE_MTU_RSP	0x03
#define L2CAP_ATT_FIND_BY_TYPE_REQ	0x06
#define L2CAP_ATT_READ_REQ		0x0a
#define L2CAP_ATT_READ_RSP		0x0b
#define L2CAP_ATT_WRITE_REQ		0x12
#define L2CAP_ATT_WRITE_RSP		0x13
#define L2CAP_ATT_HANDLE_VALUE_NOTIFY	0x1b
#define L2CAP_ATT_HANDLE_VALUE_IND	0x1d

#define GATT_STATUS_SUCCESS	0x00000000
#define GATT_STATUS_FAILURE	0x00000101
#define GATT_STATUS_INS_AUTH	0x08

#define GATT_ERR_INVAL_ATTR_VALUE_LEN	0x0D

#define GATT_SERVER_DISCONNECTED	0
#define GATT_SERVER_CONNECTED		1

#define APP1_ID	1
#define APP2_ID	2

#define CONN1_ID	1
#define CONN2_ID	2

#define TRANS1_ID	1

#define BT_TRANSPORT_UNKNOWN		0x00

#define GATT_SERVER_TRANSPORT_LE		0x01
#define GATT_SERVER_TRANSPORT_BREDR		0x02
#define GATT_SERVER_TRANSPORT_LE_BREDR		(0x01 | 0x02)

#define GATT_WRITE_TYPE_NO_RESPONSE	0x01
#define GATT_WRITE_TYPE_DEFAULT		0x02
#define GATT_WRITE_TYPE_PREPARE		0x03
#define GATT_WRITE_TYPE_SIGNED		0x04

#define CHAR_PROP_BROADCAST			0x01
#define CHAR_PROP_READ				0x02
#define CHAR_PROP_WRITE_WITHOUT_RESPONSE	0x04
#define CHAR_PROP_WRITE				0x08
#define CHAR_PROP_NOTIFY			0x10
#define CHAR_PROP_INDICATE			0x20
#define CHAR_PROP_AUTHENTICATED_SIGNED_WRITES	0x40
#define CHAR_PROP_EXTENDED_PROPERTIES		0x80

#define CHAR_PERM_READ			0x0001
#define CHAR_PERM_READ_ENCRYPTED	0x0002
#define CHAR_PERM_READ_ENCRYPTED_MITM	0x0004
#define CHAR_PERM_WRITE			0x0010
#define CHAR_PERM_WRITE_ENCRYPTED	0x0020
#define CHAR_PERM_WRITE_ENCRYPTED_MITM	0x0040
#define CHAR_PERM_WRITE_SIGNED		0x0080
#define CHAR_PERM_WRITE_SIGNED_MITM	0x0100

static struct queue *list; /* List of gatt test cases */

static uint16_t srvc1_handle;
static uint16_t inc_srvc1_handle;
static uint16_t char1_handle;

static struct iovec char1_handle_v = {
	.iov_base = &char1_handle,
	.iov_len = sizeof(char1_handle),
};

struct set_att_data {
	char *to;
	char *from;
	int len;
};

struct att_write_req_data {
	uint16_t *attr_handle;
	uint8_t *value;
};

static bt_uuid_t app1_uuid = {
	.uu = { 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
				0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 },
};

static bt_uuid_t app2_uuid = {
	.uu = { 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
				0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02 },
};

static uint8_t value_1[] = {0x01};

static uint8_t att_write_req_value_1[] = {0x00, 0x01, 0x02, 0x03};
static struct iovec att_write_req_value_1_v = {
	.iov_base = att_write_req_value_1,
	.iov_len = sizeof(att_write_req_value_1),
};

struct gatt_connect_data {
	const int app_id;
	const int conn_id;
};

struct gatt_search_service_data {
	const int conn_id;
	bt_uuid_t *filter_uuid;
};

struct get_char_data {
	const int conn_id;
	btgatt_srvc_id_t *service;
};

struct get_desc_data {
	const int conn_id;
	btgatt_srvc_id_t *service;
	btgatt_gatt_id_t *characteristic;
	btgatt_gatt_id_t *desc;
};

struct get_incl_data {
	const int conn_id;
	btgatt_srvc_id_t *service;
	btgatt_srvc_id_t *start_service;
};

struct read_char_data {
	const int conn_id;
	btgatt_srvc_id_t *service;
	btgatt_gatt_id_t *characteristic;
	int auth_req;
};

struct read_desc_data {
	const int conn_id;
	btgatt_srvc_id_t *service;
	btgatt_gatt_id_t *characteristic;
	btgatt_gatt_id_t *descriptor;
	int auth_req;
};

struct write_char_data {
	int conn_id;
	btgatt_srvc_id_t *service;
	btgatt_gatt_id_t *characteristic;
	int write_type;
	int len;
	int auth_req;
	char *p_value;
};

struct write_desc_data {
	int conn_id;
	btgatt_srvc_id_t *service;
	btgatt_gatt_id_t *characteristic;
	btgatt_gatt_id_t *descriptor;
	int write_type;
	int len;
	int auth_req;
	char *p_value;
};

struct notif_data {
	int conn_id;
	const bt_bdaddr_t *bdaddr;
	btgatt_srvc_id_t *service;
	btgatt_gatt_id_t *charac;
};

struct add_service_data {
	int app_id;
	btgatt_srvc_id_t *service;
	int num_handles;
};

struct add_included_service_data {
	int app_id;
	uint16_t *inc_srvc_handle;
	uint16_t *srvc_handle;
};
struct add_char_data {
	int app_id;
	uint16_t *srvc_handle;
	bt_uuid_t *uuid;
	int properties;
	int permissions;
};

struct add_desc_data {
	int app_id;
	uint16_t *srvc_handle;
	bt_uuid_t *uuid;
	int permissions;
};

struct start_srvc_data {
	int app_id;
	uint16_t *srvc_handle;
	int transport;
};

struct stop_srvc_data {
	int app_id;
	uint16_t *srvc_handle;
};

struct delete_srvc_data {
	int app_id;
	uint16_t *srvc_handle;
};

struct send_indication_data {
	int app_id;
	uint16_t *attr_handle;
	int conn_id;
	int len;
	int confirm;
	char *p_value;
};

struct send_resp_data {
	int conn_id;
	int trans_id;
	int status;
	btgatt_response_t *response;
};

static bt_bdaddr_t emu_remote_bdaddr_val = {
	.address = { 0x00, 0xaa, 0x01, 0x01, 0x00, 0x00 },
};
static bt_device_type_t emu_remote_ble_device_type = BT_DEVICE_DEVTYPE_BLE;

static bt_property_t prop_emu_remotes_default_set[] = {
	{ BT_PROPERTY_BDADDR, sizeof(emu_remote_bdaddr_val),
						&emu_remote_bdaddr_val },
};
static bt_property_t prop_emu_remotes_default_le_set[] = {
	{ BT_PROPERTY_BDADDR, sizeof(emu_remote_bdaddr_val),
						&emu_remote_bdaddr_val },
	{ BT_PROPERTY_TYPE_OF_DEVICE, sizeof(bt_device_type_t),
						&emu_remote_ble_device_type },
};

static struct bt_action_data prop_test_remote_ble_bdaddr_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_BDADDR,
	.prop = &prop_emu_remotes_default_set[0],
};

static bt_scan_mode_t setprop_scan_mode_conn_val =
					BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE;

static bt_property_t prop_test_scan_mode_conn = {
	.type = BT_PROPERTY_ADAPTER_SCAN_MODE,
	.val = &setprop_scan_mode_conn_val,
	.len = sizeof(setprop_scan_mode_conn_val),
};

static struct emu_l2cap_cid_data cid_data;

static struct gatt_connect_data app1_conn_req = {
	.app_id = APP1_ID,
	.conn_id = CONN1_ID,
};

static struct gatt_connect_data app1_conn2_req = {
	.app_id = APP1_ID,
	.conn_id = CONN2_ID,
};

static struct gatt_connect_data app2_conn_req = {
	.app_id = APP2_ID,
	.conn_id = CONN2_ID,
};

static struct gatt_search_service_data search_services_1 = {
	.conn_id = CONN1_ID,
	.filter_uuid = NULL,
};

static const struct iovec exchange_mtu_req_pdu = raw_pdu(0x02, 0xa0, 0x02);
static const struct iovec exchange_mtu_resp_pdu = raw_pdu(0x03, 0xa0, 0x02);

static struct bt_action_data bearer_type = {
	.bearer_type = BDADDR_LE_PUBLIC,
};

static btgatt_srvc_id_t service_1 = {
	.is_primary = true,
	.id = {
		.inst_id = 0,
		.uuid.uu = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00,  0x00, 0x18, 0x00, 0x00}
	}
};

static btgatt_srvc_id_t service_2 = {
	.is_primary = true,
	.id = {
		.inst_id = 1,
		.uuid.uu = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00,  0x01, 0x18, 0x00, 0x00},
	}
};

static btgatt_srvc_id_t service_add_1 = {
	.is_primary = true,
	.id = {
		.inst_id = 0,
		.uuid.uu = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00, 0xFF, 0xEF, 0x00, 0x00},
	}
};

static btgatt_srvc_id_t service_add_2 = {
	.is_primary = true,
	.id = {
		.inst_id = 1,
		.uuid.uu = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00, 0xFF, 0xDF, 0x00, 0x00},
	}
};

static btgatt_srvc_id_t service_add_3 = {
	.is_primary = true,
	.id = {
		.inst_id = 2,
		.uuid.uu = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00, 0xFF, 0xCF, 0x00, 0x00},
	}
};

static btgatt_srvc_id_t included_1 = {
	.is_primary = false,
	.id = {
		.inst_id = 1,
		.uuid.uu = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00,  0xff, 0xfe, 0x00, 0x00},
	}
};

static btgatt_srvc_id_t included_2 = {
	.is_primary = false,
	.id = {
		.inst_id = 1,
		.uuid.uu = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
				0x08, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10},
	}
};

static btgatt_gatt_id_t characteristic_1 = {
	.inst_id = 1,
	.uuid.uu = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00,  0x19, 0x00, 0x00, 0x00}
};

static btgatt_gatt_id_t desc_1 = {
	.inst_id = 1,
	.uuid.uu = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00,  0x00, 0x29, 0x00, 0x00}
};

static btgatt_gatt_id_t desc_2 = {
	.inst_id = 2,
	.uuid.uu = {0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
			0x00, 0x10, 0x00, 0x00,  0x01, 0x29, 0x00, 0x00}
};

static btgatt_read_params_t read_params_1;
static btgatt_write_params_t write_params_1;
static btgatt_notify_params_t notify_params_1;

static struct get_char_data get_char_data_1 = {
	.conn_id = CONN1_ID,
	.service = &service_1
};

static struct get_char_data get_char_data_2 = {
	.conn_id = CONN1_ID,
	.service = &service_2
};

static struct get_desc_data get_desc_data_1 = {
	.conn_id = CONN1_ID,
	.service = &service_1,
	.characteristic = &characteristic_1,
};

static struct get_desc_data get_desc_data_2 = {
	.conn_id = CONN1_ID,
	.service = &service_1,
	.characteristic = &characteristic_1,
	.desc = &desc_1,
};

static struct read_char_data read_char_data_1 = {
	.conn_id = CONN1_ID,
	.service = &service_1,
	.characteristic = &characteristic_1,
};

static struct read_char_data read_char_data_2 = {
	.conn_id = CONN1_ID,
	.service = &service_2,
	.characteristic = &characteristic_1,
};

static struct read_desc_data read_desc_data_1 = {
	.conn_id = CONN1_ID,
	.service = &service_1,
	.characteristic = &characteristic_1,
	.descriptor = &desc_1,
};

static struct read_desc_data read_desc_data_2 = {
	.conn_id = CONN1_ID,
	.service = &service_1,
	.characteristic = &characteristic_1,
	.descriptor = &desc_2,
};

static struct get_incl_data get_incl_data_1 = {
	.conn_id = CONN1_ID,
	.service = &service_1
};

static char value_2[] = {0x00, 0x01, 0x02, 0x03};

static struct write_char_data write_char_data_1 = {
	.conn_id = CONN1_ID,
	.service = &service_1,
	.characteristic = &characteristic_1,
	.write_type = GATT_WRITE_TYPE_NO_RESPONSE,
	.len = sizeof(value_2),
	.p_value = value_2,
	.auth_req = 0
};

static struct write_char_data write_char_data_2 = {
	.conn_id = CONN1_ID,
	.service = &service_1,
	.characteristic = &characteristic_1,
	.write_type = GATT_WRITE_TYPE_DEFAULT,
	.len = sizeof(value_2),
	.p_value = value_2,
	.auth_req = 0
};

static struct write_desc_data write_desc_data_1 = {
	.conn_id = CONN1_ID,
	.service = &service_1,
	.characteristic = &characteristic_1,
	.descriptor = &desc_1,
	.write_type = 2,
	.len = sizeof(value_2),
	.auth_req = 0,
	.p_value = value_2,
};

static struct write_desc_data write_desc_data_2 = {
	.conn_id = CONN1_ID,
	.service = &service_1,
	.characteristic = &characteristic_1,
	.descriptor = &desc_2,
	.write_type = 2,
	.len = sizeof(value_2),
	.auth_req = 0,
	.p_value = value_2,
};

static struct notif_data notif_data_1 = {
	.conn_id = CONN1_ID,
	.service = &service_1,
	.charac = &characteristic_1,
	.bdaddr = &emu_remote_bdaddr_val,
};

static struct add_service_data add_service_data_1 = {
	.app_id = APP1_ID,
	.service = &service_add_1,
	.num_handles = 1
};

static struct add_service_data add_service_data_2 = {
	.app_id = APP1_ID,
	.service = &service_add_2,
	.num_handles = 1
};

static struct add_service_data add_service_data_3 = {
	.app_id = APP1_ID,
	.service = &service_add_3,
	.num_handles = 1
};

static struct add_service_data add_service_data_4 = {
	.app_id = APP1_ID,
	.service = &service_add_1,
	.num_handles = 2
};

static struct add_service_data add_service_data_5 = {
	.app_id = APP1_ID,
	.service = &service_add_1,
	.num_handles = 3
};

static struct add_service_data add_service_data_6 = {
	.app_id = APP1_ID,
	.service = &service_add_1,
	.num_handles = 4
};

static struct add_service_data add_bad_service_data_1 = {
	.app_id = APP1_ID,
	.service = &service_add_1,
	.num_handles = 0
};

static struct add_service_data add_sec_service_data_1 = {
	.app_id = APP1_ID,
	.service = &included_1,
	.num_handles = 1
};

static uint16_t srvc_bad_handle = 0xffff;

static struct add_included_service_data add_inc_service_data_1 = {
	.app_id = APP1_ID,
	.inc_srvc_handle = &inc_srvc1_handle,
	.srvc_handle = &srvc1_handle
};

static struct add_included_service_data add_bad_inc_service_data_1 = {
	.app_id = APP1_ID,
	.inc_srvc_handle = &srvc_bad_handle,
	.srvc_handle = &srvc1_handle
};

static struct add_char_data add_char_data_1 = {
	.app_id = APP1_ID,
	.srvc_handle = &srvc1_handle,
	.uuid = &app1_uuid,
	.properties = 0,
	.permissions = 0
};

static struct add_char_data add_char_data_2 = {
	.app_id = APP1_ID,
	.srvc_handle = &srvc1_handle,
	.uuid = &app1_uuid,
	.properties = CHAR_PROP_WRITE,
	.permissions = CHAR_PERM_WRITE
};

static struct add_char_data add_bad_char_data_1 = {
	.app_id = APP1_ID,
	.srvc_handle = &srvc_bad_handle,
	.uuid = &app1_uuid,
	.properties = 0,
	.permissions = 0
};

static struct add_desc_data add_bad_desc_data_1 = {
	.app_id = APP1_ID,
	.srvc_handle = &srvc_bad_handle,
	.uuid = &app2_uuid,
	.permissions = 0
};

static struct add_desc_data add_bad_desc_data_2 = {
	.app_id = APP2_ID,
	.srvc_handle = &srvc1_handle,
	.uuid = &app2_uuid,
	.permissions = 0
};

static struct add_desc_data add_desc_data_1 = {
	.app_id = APP1_ID,
	.srvc_handle = &srvc1_handle,
	.uuid = &app2_uuid,
	.permissions = 0
};

static struct start_srvc_data start_srvc_data_1 = {
	.app_id = APP1_ID,
	.srvc_handle = &srvc1_handle,
	.transport = GATT_SERVER_TRANSPORT_LE_BREDR
};

static struct start_srvc_data start_srvc_data_2 = {
	.app_id = APP1_ID,
	.srvc_handle = &srvc1_handle,
	.transport = GATT_SERVER_TRANSPORT_LE
};

static struct start_srvc_data start_bad_srvc_data_1 = {
	.app_id = APP1_ID,
	.srvc_handle = &srvc_bad_handle,
	.transport = GATT_SERVER_TRANSPORT_LE
};

static struct start_srvc_data start_bad_srvc_data_2 = {
	.app_id = APP1_ID,
	.srvc_handle = &srvc1_handle,
	.transport = 0
};

static struct stop_srvc_data stop_srvc_data_1 = {
	.app_id = APP1_ID,
	.srvc_handle = &srvc1_handle
};

static struct stop_srvc_data stop_bad_srvc_data_1 = {
	.app_id = APP1_ID,
	.srvc_handle = &srvc_bad_handle
};

static struct delete_srvc_data delete_srvc_data_1 = {
	.app_id = APP1_ID,
	.srvc_handle = &srvc1_handle
};

static struct delete_srvc_data delete_bad_srvc_data_1 = {
	.app_id = APP1_ID,
	.srvc_handle = &srvc_bad_handle
};

static uint16_t srvc_indication_handle_1 = 0x01;

static struct send_indication_data send_indication_data_1 = {
	.app_id = APP1_ID,
	.attr_handle = &srvc_indication_handle_1,
	.conn_id = CONN1_ID,
	.len = sizeof(value_2),
	.p_value = value_2,
	.confirm = 1
};

static struct send_indication_data send_indication_data_2 = {
	.app_id = APP1_ID,
	.attr_handle = &srvc_indication_handle_1,
	.conn_id = CONN1_ID,
	.len = sizeof(value_2),
	.p_value = value_2,
	.confirm = 0
};

static struct send_indication_data send_bad_indication_data_1 = {
	.app_id = APP1_ID,
	.attr_handle = &srvc_indication_handle_1,
	.conn_id = CONN2_ID,
	.len = sizeof(value_2),
	.p_value = value_2,
	.confirm = 0
};

struct set_read_params {
	btgatt_read_params_t *params;
	btgatt_srvc_id_t *srvc_id;
	btgatt_gatt_id_t *char_id;
	btgatt_gatt_id_t *descr_id;
	uint8_t *value;
	uint16_t len;
	uint16_t value_type;
	uint8_t status;
};

struct set_write_params {
	btgatt_write_params_t *params;
	btgatt_srvc_id_t *srvc_id;
	btgatt_gatt_id_t *char_id;
	btgatt_gatt_id_t *descr_id;
	uint8_t status;
};

struct set_notify_params {
	btgatt_notify_params_t *params;
	uint8_t *value;
	uint16_t len;
	uint8_t is_notify;
	btgatt_srvc_id_t *srvc_id;
	btgatt_gatt_id_t *char_id;
	bt_bdaddr_t *bdaddr;
};

static struct set_read_params set_read_param_1 = {
	.params = &read_params_1,
	.srvc_id = &service_1,
	.char_id = &characteristic_1,
	.value = value_1,
	.len = sizeof(value_1),
	.status = BT_STATUS_SUCCESS
};

static struct set_read_params set_read_param_2 = {
	.params = &read_params_1,
	.srvc_id = &service_1,
	.char_id = &characteristic_1,
	.status = GATT_STATUS_INS_AUTH
};

static struct set_read_params set_read_param_3 = {
	.params = &read_params_1,
	.srvc_id = &service_2,
	.char_id = &characteristic_1,
	.status = BT_STATUS_FAIL
};

static struct set_read_params set_read_param_4 = {
	.params = &read_params_1,
	.srvc_id = &service_1,
	.char_id = &characteristic_1,
	.descr_id = &desc_1,
	.value = value_1,
	.len = sizeof(value_1),
	.status = BT_STATUS_SUCCESS
};

static struct set_read_params set_read_param_5 = {
	.params = &read_params_1,
	.srvc_id = &service_1,
	.char_id = &characteristic_1,
	.descr_id = &desc_1,
	.status = GATT_STATUS_INS_AUTH
};

static struct set_read_params set_read_param_6 = {
	.params = &read_params_1,
	.srvc_id = &service_1,
	.char_id = &characteristic_1,
	.descr_id = &desc_2,
	.status = BT_STATUS_FAIL
};

static struct set_write_params set_write_param_1 = {
	.params = &write_params_1,
	.srvc_id = &service_1,
	.char_id = &characteristic_1,
	.status = BT_STATUS_SUCCESS
};

static struct set_write_params set_write_param_2 = {
	.params = &write_params_1,
	.srvc_id = &service_1,
	.char_id = &characteristic_1,
	.status = GATT_STATUS_INS_AUTH
};

static struct set_write_params set_write_param_3 = {
	.params = &write_params_1,
	.srvc_id = &service_1,
	.char_id = &characteristic_1,
	.status = BT_STATUS_FAIL
};

static struct set_write_params set_write_param_4 = {
	.params = &write_params_1,
	.srvc_id = &service_1,
	.char_id = &characteristic_1,
	.descr_id = &desc_1,
	.status = BT_STATUS_SUCCESS
};

static struct set_write_params set_write_param_5 = {
	.params = &write_params_1,
	.srvc_id = &service_1,
	.char_id = &characteristic_1,
	.descr_id = &desc_2,
	.status = BT_STATUS_FAIL
};

static struct set_write_params set_write_param_6 = {
	.params = &write_params_1,
	.srvc_id = &service_1,
	.char_id = &characteristic_1,
	.descr_id = &desc_1,
	.status = GATT_STATUS_INS_AUTH
};

static struct set_notify_params set_notify_param_1 = {
	.params = &notify_params_1,
	.value = value_1,
	.len = sizeof(value_1),
	.is_notify = 0,
	.srvc_id = &service_1,
	.char_id = &characteristic_1,
	.bdaddr = &emu_remote_bdaddr_val
};

static struct set_notify_params set_notify_param_2 = {
	.params = &notify_params_1,
	.value = value_1,
	.len = sizeof(value_1),
	.is_notify = 1,
	.srvc_id = &service_1,
	.char_id = &characteristic_1,
	.bdaddr = &emu_remote_bdaddr_val
};

static btgatt_response_t response_1 = {
	.handle = 0x1c,
	.attr_value.auth_req = 0,
	.attr_value.handle = 0x1d,
	.attr_value.len = 0,
	.attr_value.offset = 0,
};

static btgatt_response_t response_2 = {
	.handle = 0x1c,
	.attr_value.auth_req = 0,
	.attr_value.handle = 0x1d,
	.attr_value.len = sizeof(att_write_req_value_1),
	.attr_value.offset = 0,
};

static struct send_resp_data send_resp_data_1 = {
	.conn_id = CONN1_ID,
	.trans_id = TRANS1_ID,
	.status = BT_STATUS_SUCCESS,
	.response = &response_1,
};

static struct send_resp_data send_resp_data_2 = {
	.conn_id = CONN1_ID,
	.trans_id = TRANS1_ID,
	.status = BT_STATUS_SUCCESS,
	.response = &response_2,
};

static struct send_resp_data send_resp_data_2_error = {
	.conn_id = CONN1_ID,
	.trans_id = TRANS1_ID,
	.status = GATT_ERR_INVAL_ATTR_VALUE_LEN,
	.response = &response_2,
};

#define SEARCH_SERVICE_SINGLE_SUCCESS_PDUS				\
	raw_pdu(0x10, 0x01, 0x00, 0xff, 0xff, 0x00, 0x28),		\
	raw_pdu(0x11, 0x06, 0x01, 0x00, 0x10, 0x00, 0x00, 0x18),	\
	raw_pdu(0x10, 0x11, 0x00, 0xff, 0xff, 0x00, 0x28),		\
	raw_pdu(0x01, 0x10, 0x11, 0x00, 0x0a)

#define READ_BY_TYPE_SINGLE_CHARACTERISTIC_PDUS				\
	raw_pdu(0x08, 0x01, 0x00, 0x10, 0x00, 0x03, 0x28),		\
	raw_pdu(0x09, 0x07, 0x02, 0x00, 0x04, 0x00, 0x00, 0x19, 0x00),	\
	raw_pdu(0x08, 0x03, 0x00, 0x10, 0x00, 0x03, 0x28),		\
	raw_pdu(0x01, 0x08, 0x03, 0x00, 0x0a)

static struct iovec search_service[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	end_pdu
};

static struct iovec search_service_2[] = {
	raw_pdu(0x10, 0x01, 0x00, 0xff, 0xff, 0x00, 0x28),
	raw_pdu(0x11, 0x06, 0x01, 0x00, 0x10, 0x00, 0x00, 0x18),
	raw_pdu(0x10, 0x11, 0x00, 0xff, 0xff, 0x00, 0x28),
	raw_pdu(0x11, 0x06, 0x11, 0x00, 0x20, 0x00, 0x01, 0x18),
	raw_pdu(0x10, 0x21, 0x00, 0xff, 0xff, 0x00, 0x28),
	raw_pdu(0x01, 0x10, 0x21, 0x00, 0x0a),
	end_pdu
};

static struct iovec search_service_3[] = {
	raw_pdu(0x10, 0x01, 0x00, 0xff, 0xff, 0x00, 0x28),
	raw_pdu(0x01, 0x08, 0x01, 0x00, 0x0a),
	end_pdu
};

static struct iovec search_service_4[] = {
	raw_pdu(0x10, 0x01, 0x00, 0xff, 0xff, 0x00, 0x28),
	raw_pdu(0x11, 0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x18),
	end_pdu
};

static struct iovec get_characteristic_1[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	READ_BY_TYPE_SINGLE_CHARACTERISTIC_PDUS,
	end_pdu
};

static struct iovec get_characteristic_2[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	raw_pdu(0x08, 0x01, 0x00, 0x10, 0x00, 0x03, 0x28),
	raw_pdu(0x09, 0x07, 0x00, 0x00, 0x04, 0x00, 0x00, 0x19, 0x00),
	end_pdu
};

static struct iovec get_descriptor_0[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	READ_BY_TYPE_SINGLE_CHARACTERISTIC_PDUS,
	raw_pdu(0x04, 0x01, 0x00, 0x10, 0x00),
	raw_pdu(0x05, 0x01, 0x00, 0x00, 0x00, 0x29),
	end_pdu
};

static struct iovec get_descriptor_1[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	READ_BY_TYPE_SINGLE_CHARACTERISTIC_PDUS,
	raw_pdu(0x04, 0x01, 0x00, 0x10, 0x00),
	raw_pdu(0x05, 0x01, 0x04, 0x00, 0x00, 0x29),
	raw_pdu(0x04, 0x05, 0x00, 0x10, 0x00),
	raw_pdu(0x01, 0x04, 0x05, 0x00, 0x0a),
	end_pdu
};

static struct iovec get_descriptor_2[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	READ_BY_TYPE_SINGLE_CHARACTERISTIC_PDUS,
	raw_pdu(0x04, 0x01, 0x00, 0x10, 0x00),
	raw_pdu(0x05, 0x01, 0x04, 0x00, 0x00, 0x29, 0x05, 0x00, 0x01, 0x29),
	raw_pdu(0x04, 0x06, 0x00, 0x10, 0x00),
	raw_pdu(0x01, 0x04, 0x06, 0x00, 0x0a),
	end_pdu
};

static struct iovec get_descriptor_3[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	READ_BY_TYPE_SINGLE_CHARACTERISTIC_PDUS,
	raw_pdu(0x04, 0x01, 0x00, 0x10, 0x00),
	raw_pdu(0x01, 0x04, 0x01, 0x00, 0x0a),
	end_pdu
};

static struct iovec get_included_0[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	raw_pdu(0x08, 0x01, 0x00, 0x10, 0x00, 0x02, 0x28),
	raw_pdu(0x09, 0x08, 0x00, 0x00, 0x15, 0x00, 0x19, 0x00, 0xff, 0xfe),
	end_pdu
};

static struct iovec get_included_1[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	raw_pdu(0x08, 0x01, 0x00, 0x10, 0x00, 0x02, 0x28),
	raw_pdu(0x09, 0x08, 0x02, 0x00, 0x15, 0x00, 0x19, 0x00, 0xff, 0xfe),
	raw_pdu(0x08, 0x03, 0x00, 0x10, 0x00, 0x02, 0x28),
	raw_pdu(0x01, 0x08, 0x03, 0x00, 0x0a),
	end_pdu
};

static struct iovec get_included_2[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	raw_pdu(0x08, 0x01, 0x00, 0x10, 0x00, 0x02, 0x28),
	raw_pdu(0x09, 0x06, 0x02, 0x00, 0x15, 0x00, 0x19, 0x00),
	raw_pdu(0x0a, 0x15, 0x00),
	raw_pdu(0x0b, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
				0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10),
	raw_pdu(0x08, 0x03, 0x00, 0x10, 0x00, 0x02, 0x28),
	raw_pdu(0x01, 0x08, 0x03, 0x00, 0x0a),
	end_pdu
};

static struct iovec get_included_3[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	raw_pdu(0x08, 0x01, 0x00, 0x10, 0x00, 0x02, 0x28),
	raw_pdu(0x01, 0x08, 0x01, 0x00, 0x0a),
	end_pdu
};

static struct iovec read_characteristic_1[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	raw_pdu(0x08, 0x01, 0x00, 0x10, 0x00, 0x03, 0x28),
	raw_pdu(0x09, 0x07, 0x02, 0x00, 0x04, 0x03, 0x00, 0x19, 0x00),
	raw_pdu(0x08, 0x03, 0x00, 0x10, 0x00, 0x03, 0x28),
	raw_pdu(0x01, 0x08, 0x03, 0x00, 0x0a),
	raw_pdu(0x0a, 0x03, 0x00),
	raw_pdu(0x0b, 0x01),
	end_pdu
};

static struct iovec read_characteristic_2[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	raw_pdu(0x08, 0x01, 0x00, 0x10, 0x00, 0x03, 0x28),
	raw_pdu(0x09, 0x07, 0x02, 0x00, 0x04, 0x03, 0x00, 0x19, 0x00),
	raw_pdu(0x08, 0x03, 0x00, 0x10, 0x00, 0x03, 0x28),
	raw_pdu(0x01, 0x08, 0x03, 0x00, 0x0a),
	raw_pdu(0x0a, 0x03, 0x00),
	raw_pdu(0x01, 0x0a, 0x03, 0x00, 0x08),
	end_pdu
};

static struct iovec read_descriptor_1[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	READ_BY_TYPE_SINGLE_CHARACTERISTIC_PDUS,
	raw_pdu(0x04, 0x01, 0x00, 0x10, 0x00),
	raw_pdu(0x05, 0x01, 0x04, 0x00, 0x00, 0x29),
	raw_pdu(0x04, 0x05, 0x00, 0x10, 0x00),
	raw_pdu(0x01, 0x04, 0x05, 0x00, 0x0a),
	raw_pdu(0x0a, 0x04, 0x00),
	raw_pdu(0x0b, 0x01),
	end_pdu
};

static struct iovec read_descriptor_2[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	READ_BY_TYPE_SINGLE_CHARACTERISTIC_PDUS,
	raw_pdu(0x04, 0x01, 0x00, 0x10, 0x00),
	raw_pdu(0x05, 0x01, 0x04, 0x00, 0x00, 0x29),
	raw_pdu(0x04, 0x05, 0x00, 0x10, 0x00),
	raw_pdu(0x01, 0x04, 0x05, 0x00, 0x0a),
	raw_pdu(0x0a, 0x04, 0x00),
	raw_pdu(0x01, 0x0a, 0x04, 0x00, 0x08),
	end_pdu
};

static struct iovec write_characteristic_1[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	raw_pdu(0x08, 0x01, 0x00, 0x10, 0x00, 0x03, 0x28),
	raw_pdu(0x09, 0x07, 0x02, 0x00, 0x04, 0x03, 0x00, 0x19, 0x00),
	raw_pdu(0x08, 0x03, 0x00, 0x10, 0x00, 0x03, 0x28),
	raw_pdu(0x01, 0x08, 0x03, 0x00, 0x0a),
	raw_pdu(0x52, 0x03, 0x00, 0x00, 0x01, 0x02, 0x03),
	end_pdu
};

static struct iovec write_characteristic_2[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	raw_pdu(0x08, 0x01, 0x00, 0x10, 0x00, 0x03, 0x28),
	raw_pdu(0x09, 0x07, 0x02, 0x00, 0x04, 0x03, 0x00, 0x19, 0x00),
	raw_pdu(0x08, 0x03, 0x00, 0x10, 0x00, 0x03, 0x28),
	raw_pdu(0x01, 0x08, 0x03, 0x00, 0x0a),
	raw_pdu(0x12, 0x03, 0x00, 0x00, 0x01, 0x02, 0x03),
	raw_pdu(0x13),
	end_pdu
};

static struct iovec write_characteristic_3[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	raw_pdu(0x08, 0x01, 0x00, 0x10, 0x00, 0x03, 0x28),
	raw_pdu(0x09, 0x07, 0x02, 0x00, 0x04, 0x03, 0x00, 0x19, 0x00),
	raw_pdu(0x08, 0x03, 0x00, 0x10, 0x00, 0x03, 0x28),
	raw_pdu(0x01, 0x08, 0x03, 0x00, 0x0a),
	raw_pdu(0x12, 0x03, 0x00, 0x00, 0x01, 0x02, 0x03),
	raw_pdu(0x01, 0x12, 0x03, 0x00, 0x08),
	end_pdu
};

static struct iovec write_descriptor_1[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	READ_BY_TYPE_SINGLE_CHARACTERISTIC_PDUS,
	raw_pdu(0x04, 0x01, 0x00, 0x10, 0x00),
	raw_pdu(0x05, 0x01, 0x04, 0x00, 0x00, 0x29),
	raw_pdu(0x04, 0x05, 0x00, 0x10, 0x00),
	raw_pdu(0x01, 0x04, 0x05, 0x00, 0x0a),
	raw_pdu(0x12, 0x04, 0x00, 0x00, 0x01, 0x02, 0x03),
	raw_pdu(0x13),
	end_pdu
};

static struct iovec write_descriptor_2[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	READ_BY_TYPE_SINGLE_CHARACTERISTIC_PDUS,
	raw_pdu(0x04, 0x01, 0x00, 0x10, 0x00),
	raw_pdu(0x05, 0x01, 0x04, 0x00, 0x00, 0x29),
	raw_pdu(0x04, 0x05, 0x00, 0x10, 0x00),
	raw_pdu(0x01, 0x04, 0x05, 0x00, 0x0a),
	raw_pdu(0x12, 0x04, 0x00, 0x00, 0x01, 0x02, 0x03),
	raw_pdu(0x01, 0x12, 0x04, 0x00, 0x08),
	end_pdu
};

static struct iovec notification_1[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	READ_BY_TYPE_SINGLE_CHARACTERISTIC_PDUS,
	end_pdu
};

static struct iovec notification_2[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	READ_BY_TYPE_SINGLE_CHARACTERISTIC_PDUS,
	raw_pdu(0x1d, 0x03, 0x00, 0x01),
	raw_pdu(0x1e),
	end_pdu
};

static struct iovec notification_3[] = {
	SEARCH_SERVICE_SINGLE_SUCCESS_PDUS,
	READ_BY_TYPE_SINGLE_CHARACTERISTIC_PDUS,
	raw_pdu(0x1b, 0x03, 0x00, 0x01),
	end_pdu
};

static struct iovec send_indication_1[] = {
	raw_pdu(0x1d, 0x01, 0x00, 0x00, 0x01, 0x02, 0x03),
	raw_pdu(0x1e),
	end_pdu
};

static struct iovec send_notification_1[] = {
	raw_pdu(0x1b, 0x01, 0x00, 0x00, 0x01, 0x02, 0x03),
	end_pdu
};

static struct iovec search_range_1[] = {
	raw_pdu(0x01, 0xff, 0xff, 0xff),
	end_pdu
};

static struct iovec primary_type = raw_pdu(0x00, 0x28);

/* att commands define raw pdus */
static struct iovec att_read_req_op_v = raw_pdu(L2CAP_ATT_READ_REQ);
static struct iovec att_write_req_op_v = raw_pdu(L2CAP_ATT_WRITE_REQ);
static struct iovec att_find_by_type_req_op_v =
					raw_pdu(L2CAP_ATT_FIND_BY_TYPE_REQ);

static struct iovec svc_change_ccc_handle_v = raw_pdu(0x1c, 0x00);
static struct iovec svc_change_ccc_value_v = raw_pdu(0x00, 0x01);

static void gatt_client_register_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	bt_uuid_t *app_uuid = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	if (!app_uuid) {
		tester_warn("No app uuid provided for register action.");
		return;
	}

	step->action_status = data->if_gatt->client->register_client(app_uuid);

	schedule_action_verification(step);
}

static void gatt_client_unregister_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	int32_t cl_id = PTR_TO_INT(current_data_step->set_data);
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->client->unregister_client(cl_id);

	schedule_action_verification(step);
}

static void gatt_client_start_scan_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->client->scan(TRUE);

	schedule_action_verification(step);
}

static void gatt_client_stop_scan_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->client->scan(FALSE);

	schedule_action_verification(step);
}

static void gatt_client_connect_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct gatt_connect_data *conn_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->client->connect(
						conn_data->app_id,
						&emu_remote_bdaddr_val, 0,
						BT_TRANSPORT_UNKNOWN);

	schedule_action_verification(step);
}

static void gatt_client_disconnect_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct gatt_connect_data *conn_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->client->disconnect(
							conn_data->app_id,
							&emu_remote_bdaddr_val,
							conn_data->conn_id);

	schedule_action_verification(step);
}

static void gatt_client_do_listen_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct gatt_connect_data *conn_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->client->listen(
							conn_data->app_id,
							1);

	schedule_action_verification(step);
}

static void gatt_client_stop_listen_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct gatt_connect_data *conn_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->client->listen(
							conn_data->app_id,
							0);

	schedule_action_verification(step);
}

static void gatt_client_get_characteristic_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct get_char_data *get_char = current_data_step->set_data;
	const btgatt_client_interface_t *client = data->if_gatt->client;
	struct step *step = g_new0(struct step, 1);
	int status;

	status = client->get_characteristic(get_char->conn_id,
						get_char->service, NULL);
	step->action_status = status;

	schedule_action_verification(step);
}

static void gatt_client_get_descriptor_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct get_desc_data *get_desc = current_data_step->set_data;
	const btgatt_client_interface_t *client = data->if_gatt->client;
	struct step *step = g_new0(struct step, 1);
	int status;

	status = client->get_descriptor(get_desc->conn_id, get_desc->service,
						get_desc->characteristic,
						get_desc->desc);
	step->action_status = status;

	schedule_action_verification(step);
}

static void gatt_client_get_included_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct get_incl_data *get_incl = current_data_step->set_data;
	const btgatt_client_interface_t *client = data->if_gatt->client;
	struct step *step = g_new0(struct step, 1);
	int status;

	status = client->get_included_service(get_incl->conn_id,
				get_incl->service, get_incl->start_service);

	step->action_status = status;

	schedule_action_verification(step);
}

static void gatt_client_read_characteristic_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct read_char_data *read_char_data = current_data_step->set_data;
	const btgatt_client_interface_t *client = data->if_gatt->client;
	struct step *step = g_new0(struct step, 1);
	int status;

	status = client->read_characteristic(read_char_data->conn_id,
			read_char_data->service, read_char_data->characteristic,
			read_char_data->auth_req);

	step->action_status = status;

	schedule_action_verification(step);
}

static void gatt_client_read_descriptor_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct read_desc_data *read_desc_data = current_data_step->set_data;
	const btgatt_client_interface_t *client = data->if_gatt->client;
	struct step *step = g_new0(struct step, 1);
	int status;

	status = client->read_descriptor(read_desc_data->conn_id,
			read_desc_data->service, read_desc_data->characteristic,
			read_desc_data->descriptor,
			read_desc_data->auth_req);

	step->action_status = status;

	schedule_action_verification(step);
}

static void gatt_client_write_characteristic_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct write_char_data *write_char_data = current_data_step->set_data;
	const btgatt_client_interface_t *client = data->if_gatt->client;
	struct step *step = g_new0(struct step, 1);
	int status;

	status = client->write_characteristic(write_char_data->conn_id,
						write_char_data->service,
						write_char_data->characteristic,
						write_char_data->write_type,
						write_char_data->len,
						write_char_data->auth_req,
						write_char_data->p_value);

	step->action_status = status;

	schedule_action_verification(step);
}

static void gatt_client_register_for_notification_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct notif_data *notif_data = current_data_step->set_data;
	const btgatt_client_interface_t *client = data->if_gatt->client;
	struct step *step = g_new0(struct step, 1);
	int status;

	status = client->register_for_notification(notif_data->conn_id,
							notif_data->bdaddr,
							notif_data->service,
							notif_data->charac);
	step->action_status = status;

	schedule_action_verification(step);
}

static void gatt_client_deregister_for_notification_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct notif_data *notif_data = current_data_step->set_data;
	const btgatt_client_interface_t *client = data->if_gatt->client;
	struct step *step = g_new0(struct step, 1);
	int status;

	status = client->deregister_for_notification(notif_data->conn_id,
							notif_data->bdaddr,
							notif_data->service,
							notif_data->charac);
	step->action_status = status;

	schedule_action_verification(step);
}

static void gatt_server_register_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	bt_uuid_t *app_uuid = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	if (!app_uuid) {
		tester_warn("No app uuid provided for register action.");
		return;
	}

	step->action_status = data->if_gatt->server->register_server(app_uuid);

	schedule_action_verification(step);
}

static void gatt_server_unregister_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	int32_t sr_id = PTR_TO_INT(current_data_step->set_data);
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->server->unregister_server(sr_id);

	schedule_action_verification(step);
}

static void gatt_server_connect_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct gatt_connect_data *conn_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->server->connect(
						conn_data->app_id,
						&emu_remote_bdaddr_val, 0,
						BT_TRANSPORT_UNKNOWN);

	schedule_action_verification(step);
}

static void gatt_server_disconnect_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct gatt_connect_data *conn_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->server->disconnect(
							conn_data->app_id,
							&emu_remote_bdaddr_val,
							conn_data->conn_id);

	schedule_action_verification(step);
}

static void gatt_server_add_service_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct add_service_data *add_srvc_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->server->add_service(
						add_srvc_data->app_id,
						add_srvc_data->service,
						add_srvc_data->num_handles);

	schedule_action_verification(step);
}

static void gatt_server_add_inc_service_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct add_included_service_data *add_inc_srvc_data =
						current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->server->add_included_service(
					add_inc_srvc_data->app_id,
					*add_inc_srvc_data->srvc_handle,
					*add_inc_srvc_data->inc_srvc_handle);

	schedule_action_verification(step);
}

static void gatt_server_add_char_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct add_char_data *add_char_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->server->add_characteristic(
						add_char_data->app_id,
						*add_char_data->srvc_handle,
						add_char_data->uuid,
						add_char_data->properties,
						add_char_data->permissions);

	schedule_action_verification(step);
}

static void gatt_server_add_desc_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct add_desc_data *add_desc_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->server->add_descriptor(
						add_desc_data->app_id,
						*add_desc_data->srvc_handle,
						add_desc_data->uuid,
						add_desc_data->permissions);

	schedule_action_verification(step);
}

static void gatt_client_write_descriptor_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct write_desc_data *write_desc_data = current_data_step->set_data;
	const btgatt_client_interface_t *client = data->if_gatt->client;
	struct step *step = g_new0(struct step, 1);
	int status;

	status = client->write_descriptor(write_desc_data->conn_id,
						write_desc_data->service,
						write_desc_data->characteristic,
						write_desc_data->descriptor,
						write_desc_data->write_type,
						write_desc_data->len,
						write_desc_data->auth_req,
						write_desc_data->p_value);

	step->action_status = status;

	schedule_action_verification(step);
}

static void gatt_server_start_srvc_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct start_srvc_data *start_srvc_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->server->start_service(
						start_srvc_data->app_id,
						*start_srvc_data->srvc_handle,
						start_srvc_data->transport);

	schedule_action_verification(step);
}

static void gatt_server_stop_srvc_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct stop_srvc_data *stop_srvc_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->server->stop_service(
						stop_srvc_data->app_id,
						*stop_srvc_data->srvc_handle);

	schedule_action_verification(step);
}

static void gatt_server_delete_srvc_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct delete_srvc_data *delete_srvc_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->server->delete_service(
						delete_srvc_data->app_id,
						*delete_srvc_data->srvc_handle);

	schedule_action_verification(step);
}

static void gatt_server_send_indication_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct send_indication_data *send_indication_data =
						current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->server->send_indication(
					send_indication_data->app_id,
					*send_indication_data->attr_handle,
					send_indication_data->conn_id,
					send_indication_data->len,
					send_indication_data->confirm,
					send_indication_data->p_value);

	schedule_action_verification(step);
}

static void gatt_server_send_response_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct send_resp_data *send_resp_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_gatt->server->send_response(
						send_resp_data->conn_id,
						send_resp_data->trans_id,
						send_resp_data->status,
						send_resp_data->response);

	schedule_action_verification(step);
}

static void gatt_cid_hook_cb(const void *data, uint16_t len, void *user_data)
{
	struct test_data *t_data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(t_data->hciemu);
	struct emu_l2cap_cid_data *cid_data = user_data;
	const uint8_t *pdu = data;
	struct iovec *gatt_pdu = queue_peek_head(t_data->pdus);
	struct step *step;

	tester_debug("Received att pdu with opcode 0x%02x", pdu[0]);

	switch (pdu[0]) {
	case L2CAP_ATT_ERROR:
		step = g_new0(struct step, 1);

		step->callback = CB_EMU_ATT_ERROR;
		step->callback_result.error = pdu[4];

		schedule_callback_verification(step);
		break;
	case L2CAP_ATT_EXCHANGE_MTU_REQ:
		tester_print("Exchange MTU request received.");

		if (!memcmp(exchange_mtu_req_pdu.iov_base, pdu, len))
			bthost_send_cid_v(bthost, cid_data->handle,
						cid_data->cid,
						&exchange_mtu_resp_pdu, 1);

		break;
	case L2CAP_ATT_EXCHANGE_MTU_RSP:
		tester_print("Exchange MTU response received.");

		break;
	case L2CAP_ATT_HANDLE_VALUE_IND:
		step = g_new0(struct step, 1);

		step->callback = CB_EMU_VALUE_INDICATION;

		schedule_callback_verification(step);
		goto respond;
	case L2CAP_ATT_HANDLE_VALUE_NOTIFY:
		step = g_new0(struct step, 1);

		step->callback = CB_EMU_VALUE_NOTIFICATION;

		schedule_callback_verification(step);
		break;
	case L2CAP_ATT_READ_RSP:
		/* TODO - More complicated cases should also verify pdu data */
		step = g_new0(struct step, 1);

		step->callback = CB_EMU_READ_RESPONSE;

		schedule_callback_verification(step);
		break;
	case L2CAP_ATT_WRITE_RSP:
		/* TODO - More complicated cases should also verify pdu data */
		step = g_new0(struct step, 1);

		step->callback = CB_EMU_WRITE_RESPONSE;

		schedule_callback_verification(step);
		break;
	default:
		if (!gatt_pdu || !gatt_pdu->iov_base) {
			tester_print("Unknown ATT packet.");
			break;
		}

		if (gatt_pdu->iov_len != len) {
			tester_print("Size of incoming frame is not valid");
			tester_print("Expected size = %zd incoming size = %d",
							gatt_pdu->iov_len, len);
			break;
		}

respond:
		if (memcmp(gatt_pdu->iov_base, data, len)) {
			tester_print("Incoming data mismatch");
			break;
		}
		queue_pop_head(t_data->pdus);
		gatt_pdu = queue_pop_head(t_data->pdus);
		if (!gatt_pdu || !gatt_pdu->iov_base)
			break;

		bthost_send_cid_v(bthost, cid_data->handle, cid_data->cid,
								gatt_pdu, 1);

		break;
	}
}

static void gatt_remote_send_frame_action(void)
{
	struct test_data *t_data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(t_data->hciemu);
	struct iovec *gatt_pdu = queue_pop_head(t_data->pdus);
	struct step *step = g_new0(struct step, 1);

	if (!gatt_pdu) {
		tester_print("No frame to send");
		step->action_status = BT_STATUS_FAIL;
	} else {
		bthost_send_cid_v(bthost, cid_data.handle, cid_data.cid,
								gatt_pdu, 1);
		step->action_status = BT_STATUS_SUCCESS;
	}

	schedule_action_verification(step);
}

static void gatt_remote_send_raw_pdu_action(void)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	struct step *current_data_step = queue_peek_head(data->steps);
	struct iovec *pdu = current_data_step->set_data;
	struct iovec *pdu2 = current_data_step->set_data_2;
	struct iovec *pdu3 = current_data_step->set_data_3;
	struct step *step = g_new0(struct step, 1);

	if (cid_data.handle && cid_data.cid) {
		struct iovec rsp[3];
		size_t len = 0;

		if (!pdu) {
			step->action_status = BT_STATUS_FAIL;
			goto done;
		}

		rsp[0].iov_base = pdu->iov_base;
		rsp[0].iov_len = pdu->iov_len;
		len++;

		if (pdu2) {
			rsp[1].iov_base = pdu2->iov_base;
			rsp[1].iov_len = pdu2->iov_len;
			len++;
		}

		if (pdu3) {
			rsp[2].iov_base = pdu3->iov_base;
			rsp[2].iov_len = pdu3->iov_len;
			len++;
		}

		bthost_send_cid_v(bthost, cid_data.handle, cid_data.cid, rsp,
									len);
		step->action_status = BT_STATUS_SUCCESS;
	} else {
		tester_debug("No connection set up");
		step->action_status = BT_STATUS_FAIL;
	}

done:
	schedule_action_verification(step);
}

static void gatt_conn_cb(uint16_t handle, void *user_data)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);

	tester_print("New connection with handle 0x%04x", handle);

	if (data->hciemu_type == HCIEMU_TYPE_BREDR) {
		tester_warn("Not handled device type.");
		return;
	}

	cid_data.cid = 0x0004;
	cid_data.handle = handle;

	bthost_add_cid_hook(bthost, handle, cid_data.cid, gatt_cid_hook_cb,
								&cid_data);
}

static void gatt_client_search_services(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct step *step = g_new0(struct step, 1);
	struct gatt_search_service_data *search_data;
	int status;

	search_data = current_data_step->set_data;

	status = data->if_gatt->client->search_service(search_data->conn_id,
						search_data->filter_uuid);
	step->action_status = status;

	schedule_action_verification(step);
}

static void init_pdus(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct step *step = g_new0(struct step, 1);
	struct iovec *pdu = current_data_step->set_data;

	while (pdu->iov_base) {
		queue_push_tail(data->pdus, pdu);
		pdu++;
	}

	step->action_status = BT_STATUS_SUCCESS;

	schedule_action_verification(step);
}

static void init_read_params_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct step *step = g_new0(struct step, 1);
	struct set_read_params *set_param_data = current_data_step->set_data;
	btgatt_read_params_t *param = set_param_data->params;

	memset(param, 0, sizeof(*param));

	if (set_param_data->srvc_id)
		memcpy(&param->srvc_id, set_param_data->srvc_id,
						sizeof(btgatt_srvc_id_t));

	if (set_param_data->char_id)
		memcpy(&param->char_id, set_param_data->char_id,
						sizeof(btgatt_gatt_id_t));

	if (set_param_data->descr_id)
		memcpy(&param->descr_id, set_param_data->descr_id,
						sizeof(btgatt_gatt_id_t));

	param->value_type = set_param_data->value_type;
	param->status = set_param_data->status;
	param->value.len = set_param_data->len;

	if (param->value.len != 0)
		memcpy(&param->value.value, set_param_data->value,
							param->value.len);

	step->action_status = BT_STATUS_SUCCESS;

	schedule_action_verification(step);
}

static void init_write_params_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct step *step = g_new0(struct step, 1);
	struct set_write_params *set_param_data = current_data_step->set_data;
	btgatt_write_params_t *param = set_param_data->params;

	memset(param, 0, sizeof(*param));

	if (set_param_data->srvc_id)
		memcpy(&param->srvc_id, set_param_data->srvc_id,
						sizeof(btgatt_srvc_id_t));

	if (set_param_data->char_id)
		memcpy(&param->char_id, set_param_data->char_id,
						sizeof(btgatt_gatt_id_t));

	if (set_param_data->descr_id)
		memcpy(&param->descr_id, set_param_data->descr_id,
						sizeof(btgatt_gatt_id_t));

	param->status = set_param_data->status;

	step->action_status = BT_STATUS_SUCCESS;

	schedule_action_verification(step);
}

static void init_notify_params_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct step *step = g_new0(struct step, 1);
	struct set_notify_params *set_param_data = current_data_step->set_data;
	btgatt_notify_params_t *param = set_param_data->params;

	memset(param, 0, sizeof(*param));

	if (set_param_data->srvc_id)
		memcpy(&param->srvc_id, set_param_data->srvc_id,
						sizeof(btgatt_srvc_id_t));

	if (set_param_data->char_id)
		memcpy(&param->char_id, set_param_data->char_id,
						sizeof(btgatt_gatt_id_t));

	param->len = set_param_data->len;
	param->is_notify = set_param_data->is_notify;

	memcpy(&param->bda, set_param_data->bdaddr, sizeof(bt_bdaddr_t));
	if (param->len != 0)
		memcpy(&param->value, set_param_data->value, param->len);

	step->action_status = BT_STATUS_SUCCESS;

	schedule_action_verification(step);
}

static struct test_case test_cases[] = {
	TEST_CASE_BREDRLE("Gatt Init",
		ACTION_SUCCESS(dummy_action, NULL),
	),
	TEST_CASE_BREDRLE("Gatt Client - Register",
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
	),
	TEST_CASE_BREDRLE("Gatt Client - Unregister",
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_unregister_action,
							INT_TO_PTR(APP1_ID)),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
	),
	TEST_CASE_BREDRLE("Gatt Client - Scan",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - LE Connect",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - LE Disconnect",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_disconnect_action,
							&app1_conn_req),
		CALLBACK_GATTC_DISCONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - LE Multiple Client Conn./Disc.",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_register_action, &app2_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_connect_action, &app2_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN2_ID, APP2_ID),
		ACTION_SUCCESS(gatt_client_disconnect_action,
							&app2_conn_req),
		CALLBACK_GATTC_DISCONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN2_ID, APP2_ID),
		ACTION_SUCCESS(gatt_client_disconnect_action,
							&app1_conn_req),
		CALLBACK_GATTC_DISCONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Listen and Disconnect",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(bt_set_property_action,
						&prop_test_scan_mode_conn),
		CALLBACK_ADAPTER_PROPS(&prop_test_scan_mode_conn, 1),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_do_listen_action, &app1_conn_req),
		CALLBACK_STATUS(CB_GATTC_LISTEN, GATT_STATUS_SUCCESS),
		ACTION_SUCCESS(emu_remote_connect_hci_action, &bearer_type),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_stop_listen_action,
							&app1_conn_req),
		CALLBACK_STATUS(CB_GATTC_LISTEN, GATT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_disconnect_action,
							&app1_conn_req),
		CALLBACK_GATTC_DISCONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Double Listen",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(bt_set_property_action,
						&prop_test_scan_mode_conn),
		CALLBACK_ADAPTER_PROPS(&prop_test_scan_mode_conn, 1),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_do_listen_action, &app1_conn_req),
		CALLBACK_STATUS(CB_GATTC_LISTEN, GATT_STATUS_SUCCESS),
		ACTION_SUCCESS(emu_remote_connect_hci_action, &bearer_type),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_stop_listen_action,
							&app1_conn_req),
		CALLBACK_STATUS(CB_GATTC_LISTEN, GATT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_disconnect_action,
							&app1_conn_req),
		CALLBACK_GATTC_DISCONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		/* Close ACL on emulated remotes side so it can reconnect */
		ACTION_SUCCESS(emu_remote_disconnect_hci_action,
							&cid_data.handle),
		CALLBACK_STATE(CB_BT_ACL_STATE_CHANGED,
						BT_ACL_STATE_DISCONNECTED),
		ACTION_SUCCESS(gatt_client_do_listen_action, &app1_conn_req),
		CALLBACK_STATUS(CB_GATTC_LISTEN, GATT_STATUS_SUCCESS),
		ACTION_SUCCESS(emu_remote_connect_hci_action, &bearer_type),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN2_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_disconnect_action,
							&app1_conn2_req),
		CALLBACK_GATTC_DISCONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN2_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_stop_listen_action,
							&app1_conn_req),
		CALLBACK_STATUS(CB_GATTC_LISTEN, GATT_STATUS_SUCCESS),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Search Service - Single",
		ACTION_SUCCESS(init_pdus, search_service),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_RESULT(CONN1_ID, &service_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Search Service - Multiple",
		ACTION_SUCCESS(init_pdus, search_service_2),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_RESULT(CONN1_ID, &service_1),
		CALLBACK_GATTC_SEARCH_RESULT(CONN1_ID, &service_2),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Search Service - None",
		ACTION_SUCCESS(init_pdus, search_service_3),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Search Service - Incorrect rsp",
		ACTION_SUCCESS(init_pdus, search_service_4),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Get Characteristic - Single",
		ACTION_SUCCESS(init_pdus, get_characteristic_1),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Get Characteristic - Incorrect rsp",
		ACTION_SUCCESS(init_pdus, get_characteristic_2),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_FAILURE,
				CONN1_ID, &service_1, NULL, 0),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Get Characteristic - None",
		ACTION_SUCCESS(init_pdus, get_characteristic_1),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_FAIL(gatt_client_get_characteristic_action,
							&get_char_data_2),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_FAILURE,
							CONN1_ID, &service_2,
							NULL, 0),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Get Descriptor - Incorrect rsp",
		ACTION_SUCCESS(init_pdus, get_descriptor_0),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_get_descriptor_action,
							&get_desc_data_1),
		CALLBACK_GATTC_GET_DESCRIPTOR(GATT_STATUS_FAILURE, CONN1_ID,
				&service_1, &characteristic_1, NULL),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Get Descriptor - Single",
		ACTION_SUCCESS(init_pdus, get_descriptor_1),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_get_descriptor_action,
							&get_desc_data_1),
		CALLBACK_GATTC_GET_DESCRIPTOR(GATT_STATUS_SUCCESS, CONN1_ID,
				&service_1, &characteristic_1, &desc_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Get Descriptor - Multiple",
		ACTION_SUCCESS(init_pdus, get_descriptor_2),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
							CONN1_ID, &service_1,
							&characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_get_descriptor_action,
							&get_desc_data_1),
		CALLBACK_GATTC_GET_DESCRIPTOR(GATT_STATUS_SUCCESS, CONN1_ID,
						&service_1, &characteristic_1,
						&desc_1),
		ACTION_SUCCESS(gatt_client_get_descriptor_action,
							&get_desc_data_2),
		CALLBACK_GATTC_GET_DESCRIPTOR(GATT_STATUS_SUCCESS, CONN1_ID,
						&service_1, &characteristic_1,
						&desc_2),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Get Descriptor - None",
		ACTION_SUCCESS(init_pdus, get_descriptor_3),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_get_descriptor_action,
							&get_desc_data_1),
		CALLBACK_GATTC_GET_DESCRIPTOR(GATT_STATUS_FAILURE, CONN1_ID,
				&service_1, &characteristic_1, NULL),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Get Included Services - Incorrect rsp",
		ACTION_SUCCESS(init_pdus, get_included_0),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_included_action,
							&get_incl_data_1),
		CALLBACK_GATTC_GET_INCLUDED(GATT_STATUS_FAILURE, CONN1_ID,
							&service_1, NULL),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
		),
	TEST_CASE_BREDRLE("Gatt Client - Get Included Service - 16 UUID",
		ACTION_SUCCESS(init_pdus, get_included_1),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_included_action,
							&get_incl_data_1),
		CALLBACK_GATTC_GET_INCLUDED(GATT_STATUS_SUCCESS, CONN1_ID,
						&service_1, &included_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Get Included Service - 128 UUID",
		ACTION_SUCCESS(init_pdus, get_included_2),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_included_action,
							&get_incl_data_1),
		CALLBACK_GATTC_GET_INCLUDED(GATT_STATUS_SUCCESS, CONN1_ID,
						&service_1, &included_2),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Get Included Service - None",
		ACTION_SUCCESS(init_pdus, get_included_3),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_included_action,
							&get_incl_data_1),
		CALLBACK_GATTC_GET_INCLUDED(GATT_STATUS_FAILURE, CONN1_ID,
							&service_1, NULL),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Read Characteristic - Success",
		ACTION_SUCCESS(init_pdus, read_characteristic_1),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(init_read_params_action, &set_read_param_1),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_read_characteristic_action,
							&read_char_data_1),
		CALLBACK_GATTC_READ_CHARACTERISTIC(GATT_STATUS_SUCCESS,
						CONN1_ID, &read_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),

	TEST_CASE_BREDRLE("Gatt Client - Read Characteristic - Insuf. Auth.",
		ACTION_SUCCESS(init_pdus, read_characteristic_2),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(init_read_params_action, &set_read_param_2),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_read_characteristic_action,
							&read_char_data_1),
		CALLBACK_GATTC_READ_CHARACTERISTIC(GATT_STATUS_INS_AUTH,
						CONN1_ID, &read_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Read Characteristic - Wrong params",
		ACTION_SUCCESS(init_pdus, read_characteristic_2),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(init_read_params_action, &set_read_param_3),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_FAIL(gatt_client_read_characteristic_action,
							&read_char_data_2),
		CALLBACK_GATTC_READ_CHARACTERISTIC(GATT_STATUS_FAILURE,
						CONN1_ID, &read_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Read Descriptor - Success",
		ACTION_SUCCESS(init_pdus, read_descriptor_1),
		ACTION_SUCCESS(init_read_params_action, &set_read_param_4),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_get_descriptor_action,
							&get_desc_data_1),
		CALLBACK_GATTC_GET_DESCRIPTOR(GATT_STATUS_SUCCESS, CONN1_ID,
				&service_1, &characteristic_1, &desc_1),
		ACTION_SUCCESS(gatt_client_read_descriptor_action,
							&read_desc_data_1),
		CALLBACK_GATTC_READ_DESCRIPTOR(GATT_STATUS_SUCCESS,
						CONN1_ID, &read_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Read Descriptor - Insuf. Auth.",
		ACTION_SUCCESS(init_pdus, read_descriptor_2),
		ACTION_SUCCESS(init_read_params_action, &set_read_param_5),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_get_descriptor_action,
							&get_desc_data_1),
		CALLBACK_GATTC_GET_DESCRIPTOR(GATT_STATUS_SUCCESS, CONN1_ID,
				&service_1, &characteristic_1, &desc_1),
		ACTION_SUCCESS(gatt_client_read_descriptor_action,
							&read_desc_data_1),
		CALLBACK_GATTC_READ_DESCRIPTOR(GATT_STATUS_INS_AUTH,
						CONN1_ID, &read_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Read Descriptor - Wrong params",
		ACTION_SUCCESS(init_pdus, read_descriptor_2),
		ACTION_SUCCESS(init_read_params_action, &set_read_param_6),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_get_descriptor_action,
							&get_desc_data_1),
		CALLBACK_GATTC_GET_DESCRIPTOR(GATT_STATUS_SUCCESS, CONN1_ID,
				&service_1, &characteristic_1, &desc_1),
		ACTION_FAIL(gatt_client_read_descriptor_action,
							&read_desc_data_2),
		CALLBACK_GATTC_READ_DESCRIPTOR(GATT_STATUS_FAILURE,
						CONN1_ID, &read_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Write Characteristic Cmd - Success",
		ACTION_SUCCESS(init_pdus, write_characteristic_1),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(init_write_params_action, &set_write_param_1),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_write_characteristic_action,
							&write_char_data_1),
		CALLBACK_GATTC_WRITE_CHARACTERISTIC(GATT_STATUS_SUCCESS,
						CONN1_ID, &write_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Write Characteristic Req - Success",
		ACTION_SUCCESS(init_pdus, write_characteristic_2),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(init_write_params_action, &set_write_param_1),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_write_characteristic_action,
							&write_char_data_2),
		CALLBACK_GATTC_WRITE_CHARACTERISTIC(GATT_STATUS_SUCCESS,
						CONN1_ID, &write_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Write Characteristic - Insuf. Auth.",
		ACTION_SUCCESS(init_pdus, write_characteristic_3),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(init_write_params_action, &set_write_param_2),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_write_characteristic_action,
							&write_char_data_2),
		CALLBACK_GATTC_WRITE_CHARACTERISTIC(GATT_STATUS_INS_AUTH,
						CONN1_ID, &write_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Write Characteristic - Wrong Params",
		ACTION_SUCCESS(init_pdus, write_characteristic_3),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(init_write_params_action, &set_write_param_3),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_FAIL(gatt_client_write_characteristic_action,
							&write_char_data_2),
		CALLBACK_GATTC_WRITE_CHARACTERISTIC(GATT_STATUS_FAILURE,
						CONN1_ID, &write_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Register For Notification - Success",
		ACTION_SUCCESS(init_pdus, notification_1),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_register_for_notification_action,
								&notif_data_1),
		CALLBACK_GATTC_REGISTER_FOR_NOTIF(GATT_STATUS_SUCCESS, CONN1_ID,
							&characteristic_1,
							&service_1, 1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Deregister For Notification - Success",
		ACTION_SUCCESS(init_pdus, notification_1),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_register_for_notification_action,
								&notif_data_1),
		CALLBACK_GATTC_REGISTER_FOR_NOTIF(GATT_STATUS_SUCCESS, CONN1_ID,
							&characteristic_1,
							&service_1, 1),
		ACTION_SUCCESS(gatt_client_deregister_for_notification_action,
								&notif_data_1),
		CALLBACK_GATTC_REGISTER_FOR_NOTIF(GATT_STATUS_SUCCESS, CONN1_ID,
							&characteristic_1,
							&service_1, 0),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Register For Notification - Indicate",
		ACTION_SUCCESS(init_pdus, notification_2),
		ACTION_SUCCESS(init_notify_params_action, &set_notify_param_1),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
							CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_register_for_notification_action,
								&notif_data_1),
		CALLBACK_GATTC_REGISTER_FOR_NOTIF(GATT_STATUS_SUCCESS, CONN1_ID,
							&characteristic_1,
							&service_1, 1),
		ACTION_SUCCESS(gatt_remote_send_frame_action, NULL),
		CALLBACK_GATTC_NOTIFY(CONN1_ID, &notify_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Register For Notification - Notify",
		ACTION_SUCCESS(init_pdus, notification_3),
		ACTION_SUCCESS(init_notify_params_action, &set_notify_param_2),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_register_for_notification_action,
								&notif_data_1),
		CALLBACK_GATTC_REGISTER_FOR_NOTIF(GATT_STATUS_SUCCESS, CONN1_ID,
							&characteristic_1,
							&service_1, 1),
		ACTION_SUCCESS(gatt_remote_send_frame_action, NULL),
		CALLBACK_GATTC_NOTIFY(CONN1_ID, &notify_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Write Descriptor - Success",
		ACTION_SUCCESS(init_pdus, write_descriptor_1),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(init_write_params_action, &set_write_param_4),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_get_descriptor_action,
							&get_desc_data_1),
		CALLBACK_GATTC_GET_DESCRIPTOR(GATT_STATUS_SUCCESS, CONN1_ID,
				&service_1, &characteristic_1, &desc_1),
		ACTION_SUCCESS(gatt_client_write_descriptor_action,
							&write_desc_data_1),
		CALLBACK_GATTC_WRITE_DESCRIPTOR(GATT_STATUS_SUCCESS,
						CONN1_ID, &write_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Write Descriptor - Insuf. Auth.",
		ACTION_SUCCESS(init_pdus, write_descriptor_2),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(init_write_params_action, &set_write_param_6),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_get_descriptor_action,
							&get_desc_data_1),
		CALLBACK_GATTC_GET_DESCRIPTOR(GATT_STATUS_SUCCESS, CONN1_ID,
				&service_1, &characteristic_1, &desc_1),
		ACTION_SUCCESS(gatt_client_write_descriptor_action,
							&write_desc_data_1),
		CALLBACK_GATTC_WRITE_DESCRIPTOR(GATT_STATUS_INS_AUTH,
						CONN1_ID, &write_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Client - Write Descriptor - Wrong Param",
		ACTION_SUCCESS(init_pdus, write_descriptor_1),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(init_write_params_action, &set_write_param_5),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_client_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTC_REGISTER_CLIENT, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_client_start_scan_action, NULL),
		CLLBACK_GATTC_SCAN_RES(prop_emu_remotes_default_set, 1, TRUE),
		ACTION_SUCCESS(gatt_client_stop_scan_action, NULL),
		ACTION_SUCCESS(gatt_client_connect_action, &app1_conn_req),
		CALLBACK_GATTC_CONNECT(GATT_STATUS_SUCCESS,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_client_search_services, &search_services_1),
		CALLBACK_GATTC_SEARCH_COMPLETE(GATT_STATUS_SUCCESS, CONN1_ID),
		ACTION_SUCCESS(gatt_client_get_characteristic_action,
							&get_char_data_1),
		CALLBACK_GATTC_GET_CHARACTERISTIC_CB(GATT_STATUS_SUCCESS,
				CONN1_ID, &service_1, &characteristic_1, 4),
		ACTION_SUCCESS(gatt_client_get_descriptor_action,
							&get_desc_data_1),
		CALLBACK_GATTC_GET_DESCRIPTOR(GATT_STATUS_SUCCESS, CONN1_ID,
				&service_1, &characteristic_1, &desc_1),
		ACTION_FAIL(gatt_client_write_descriptor_action,
							&write_desc_data_2),
		CALLBACK_GATTC_WRITE_DESCRIPTOR(GATT_STATUS_FAILURE,
						CONN1_ID, &write_params_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Server - Register",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
	),
	TEST_CASE_BREDRLE("Gatt Server - Unregister",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_unregister_action,
							INT_TO_PTR(APP1_ID)),
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
	),
	TEST_CASE_BREDRLE("Gatt Server - LE Connect",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		ACTION_SUCCESS(gatt_server_connect_action, &app1_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_CONNECTED,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Server - LE Disconnect",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		ACTION_SUCCESS(gatt_server_connect_action, &app1_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_CONNECTED,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_server_disconnect_action,
							&app1_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_DISCONNECTED,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Server - LE Multiple Server Conn./Disc",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_register_action, &app2_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		ACTION_SUCCESS(gatt_server_connect_action, &app1_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_CONNECTED,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_server_connect_action, &app2_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_CONNECTED,
						prop_emu_remotes_default_set,
						CONN2_ID, APP2_ID),
		ACTION_SUCCESS(gatt_server_disconnect_action, &app2_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_DISCONNECTED,
						prop_emu_remotes_default_set,
						CONN2_ID, APP2_ID),
		ACTION_SUCCESS(gatt_server_disconnect_action, &app1_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_DISCONNECTED,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Server - Add Single Service Successful",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_1),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
						&service_add_1, NULL, NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Add Multiple Services Successful",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_1),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
						&service_add_1, NULL, NULL),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_2),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
						&service_add_2, NULL, NULL),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_3),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
						&service_add_3, NULL, NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Add Service with 0 handles",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_FAIL(gatt_server_add_service_action,
						&add_bad_service_data_1),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_FAILURE, APP1_ID,
						&service_add_1, NULL, NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Add Secondary Service",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
						&add_sec_service_data_1),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
						&included_1, NULL, NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Add Included Service Successful",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_4),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_4),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&inc_srvc1_handle),
		ACTION_SUCCESS(gatt_server_add_inc_service_action,
						&add_inc_service_data_1),
		CALLBACK_GATTS_INC_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&srvc1_handle, NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Add Inc. Service with wrong handle",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_4),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_4),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
						&service_add_1, NULL, NULL),
		ACTION_FAIL(gatt_server_add_inc_service_action,
						&add_bad_inc_service_data_1),
		CALLBACK_GATTS_INC_SERVICE_ADDED(GATT_STATUS_FAILURE, APP1_ID,
							&srvc1_handle, NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Add Single Characteristic Successful",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_5),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_add_char_action, &add_char_data_1),
		CALLBACK_GATTS_CHARACTERISTIC_ADDED(GATT_STATUS_SUCCESS,
							APP1_ID, &app1_uuid,
							&srvc1_handle, NULL,
							NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Add Char. wrong service handle",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_5),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_FAIL(gatt_server_add_char_action, &add_bad_char_data_1),
		CALLBACK_GATTS_CHARACTERISTIC_ADDED(GATT_STATUS_FAILURE,
							APP1_ID, &app1_uuid,
							NULL, NULL, NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Add Single Descriptor Successful",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_6),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_add_char_action, &add_char_data_1),
		CALLBACK_GATTS_CHARACTERISTIC_ADDED(GATT_STATUS_SUCCESS,
							APP1_ID, &app1_uuid,
							&srvc1_handle, NULL,
							NULL),
		ACTION_SUCCESS(gatt_server_add_desc_action, &add_desc_data_1),
		CALLBACK_GATTS_DESCRIPTOR_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
						&app2_uuid, &srvc1_handle,
						NULL, NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Add Desc. wrong service handle",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_6),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_add_char_action, &add_char_data_1),
		CALLBACK_GATTS_CHARACTERISTIC_ADDED(GATT_STATUS_SUCCESS,
							APP1_ID, &app1_uuid,
							&srvc1_handle, NULL,
							NULL),
		ACTION_FAIL(gatt_server_add_desc_action, &add_bad_desc_data_1),
		CALLBACK_GATTS_DESCRIPTOR_ADDED(GATT_STATUS_FAILURE, APP1_ID,
						&app2_uuid, NULL, NULL, NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Add Desc. wrong app ID",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_6),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_add_char_action, &add_char_data_1),
		CALLBACK_GATTS_CHARACTERISTIC_ADDED(GATT_STATUS_SUCCESS,
							APP1_ID, &app1_uuid,
							&srvc1_handle, NULL,
							NULL),
		ACTION_FAIL(gatt_server_add_desc_action, &add_bad_desc_data_2),
		CALLBACK_GATTS_DESCRIPTOR_ADDED(GATT_STATUS_FAILURE, APP2_ID,
						&app2_uuid, NULL, NULL, NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Start Service Successful BREDRLE",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_1),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_start_srvc_action,
							&start_srvc_data_1),
		CALLBACK_GATTS_SERVICE_STARTED(GATT_STATUS_SUCCESS, APP1_ID,
								&srvc1_handle),
	),
	TEST_CASE_BREDRLE("Gatt Server - Start Service Successful LE",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_1),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_start_srvc_action,
							&start_srvc_data_2),
		CALLBACK_GATTS_SERVICE_STARTED(GATT_STATUS_SUCCESS, APP1_ID,
								&srvc1_handle),
	),
	TEST_CASE_BREDRLE("Gatt Server - Start Service wrong service handle",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_1),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
						&service_add_1, NULL, NULL),
		ACTION_FAIL(gatt_server_start_srvc_action,
							&start_bad_srvc_data_1),
		CALLBACK_GATTS_SERVICE_STARTED(GATT_STATUS_FAILURE, APP1_ID,
									NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Start Service wrong server transport",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_1),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_FAIL(gatt_server_start_srvc_action,
							&start_bad_srvc_data_2),
		CALLBACK_GATTS_SERVICE_STARTED(GATT_STATUS_FAILURE, APP1_ID,
								&srvc1_handle),
	),
	TEST_CASE_BREDRLE("Gatt Server - Stop Service Successful",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_1),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_start_srvc_action,
							&start_srvc_data_1),
		CALLBACK_GATTS_SERVICE_STARTED(GATT_STATUS_SUCCESS, APP1_ID,
								&srvc1_handle),
		ACTION_SUCCESS(gatt_server_stop_srvc_action, &stop_srvc_data_1),
		CALLBACK_GATTS_SERVICE_STOPPED(GATT_STATUS_SUCCESS, APP1_ID,
								&srvc1_handle),
	),
	TEST_CASE_BREDRLE("Gatt Server - Stop Service wrong service handle",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_1),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_start_srvc_action,
							&start_srvc_data_1),
		CALLBACK_GATTS_SERVICE_STARTED(GATT_STATUS_SUCCESS, APP1_ID,
								&srvc1_handle),
		ACTION_FAIL(gatt_server_stop_srvc_action,
							&stop_bad_srvc_data_1),
		CALLBACK_GATTS_SERVICE_STOPPED(GATT_STATUS_FAILURE, APP1_ID,
									NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Delete Service Successful",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_1),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_delete_srvc_action,
							&delete_srvc_data_1),
		CALLBACK_GATTS_SERVICE_DELETED(GATT_STATUS_SUCCESS, APP1_ID,
								&srvc1_handle),
	),
	TEST_CASE_BREDRLE("Gatt Server - Delete Service wrong handle",
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_1),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_FAIL(gatt_server_delete_srvc_action,
						&delete_bad_srvc_data_1),
		CALLBACK_GATTS_SERVICE_DELETED(GATT_STATUS_FAILURE, APP1_ID,
									NULL),
	),
	TEST_CASE_BREDRLE("Gatt Server - Send Indication",
		ACTION_SUCCESS(init_pdus, send_indication_1),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		ACTION_SUCCESS(gatt_server_connect_action, &app1_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_CONNECTED,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_server_send_indication_action,
						&send_indication_data_1),
		CALLBACK(CB_EMU_VALUE_INDICATION),
		CALLBACK_GATTS_NOTIF_CONF(CONN1_ID, GATT_STATUS_SUCCESS),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Server - Send Notification",
		ACTION_SUCCESS(init_pdus, send_notification_1),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		ACTION_SUCCESS(gatt_server_connect_action, &app1_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_CONNECTED,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_SUCCESS(gatt_server_send_indication_action,
						&send_indication_data_2),
		CALLBACK_GATTS_NOTIF_CONF(CONN1_ID, GATT_STATUS_SUCCESS),
		CALLBACK(CB_EMU_VALUE_NOTIFICATION),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Server - Send Notification, wrong conn id",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		ACTION_SUCCESS(gatt_server_connect_action, &app1_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_CONNECTED,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		ACTION_FAIL(gatt_server_send_indication_action,
						&send_bad_indication_data_1),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Server - Send response to read char request",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_5),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_add_char_action, &add_char_data_1),
		CALLBACK_GATTS_CHARACTERISTIC_ADDED(GATT_STATUS_SUCCESS,
							APP1_ID, &app1_uuid,
							&srvc1_handle, NULL,
							&char1_handle),
		ACTION_SUCCESS(gatt_server_start_srvc_action,
							&start_srvc_data_2),
		CALLBACK_GATTS_SERVICE_STARTED(GATT_STATUS_SUCCESS, APP1_ID,
								&srvc1_handle),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		ACTION_SUCCESS(gatt_server_connect_action, &app1_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_CONNECTED,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		PROCESS_DATA(GATT_STATUS_SUCCESS,
				gatt_remote_send_raw_pdu_action,
				&att_read_req_op_v, &char1_handle_v, NULL),
		CALLBACK_GATTS_REQUEST_READ(CONN1_ID, TRANS1_ID,
						prop_emu_remotes_default_set,
						&char1_handle, 0, false),
		ACTION_SUCCESS(gatt_server_send_response_action,
							&send_resp_data_1),
		CALLBACK(CB_EMU_READ_RESPONSE),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Server - Send response to write char request",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_5),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_add_char_action, &add_char_data_2),
		CALLBACK_GATTS_CHARACTERISTIC_ADDED(GATT_STATUS_SUCCESS,
							APP1_ID, &app1_uuid,
							&srvc1_handle, NULL,
							&char1_handle),
		ACTION_SUCCESS(gatt_server_start_srvc_action,
							&start_srvc_data_2),
		CALLBACK_GATTS_SERVICE_STARTED(GATT_STATUS_SUCCESS, APP1_ID,
								&srvc1_handle),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		ACTION_SUCCESS(gatt_server_connect_action, &app1_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_CONNECTED,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		PROCESS_DATA(GATT_STATUS_SUCCESS,
					gatt_remote_send_raw_pdu_action,
					&att_write_req_op_v, &char1_handle_v,
					&att_write_req_value_1_v),
		CALLBACK_GATTS_REQUEST_WRITE(CONN1_ID, TRANS1_ID,
						prop_emu_remotes_default_set,
						&char1_handle, 0,
						sizeof(att_write_req_value_1),
						true, false,
						att_write_req_value_1),
		ACTION_SUCCESS(gatt_server_send_response_action,
							&send_resp_data_2),
		CALLBACK(CB_EMU_WRITE_RESPONSE),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Server - Find By Type - Attribute not found",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_5),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_add_char_action, &add_char_data_2),
		CALLBACK_GATTS_CHARACTERISTIC_ADDED(GATT_STATUS_SUCCESS,
							APP1_ID, &app1_uuid,
							&srvc1_handle, NULL,
							&char1_handle),
		ACTION_SUCCESS(gatt_server_start_srvc_action,
							&start_srvc_data_2),
		CALLBACK_GATTS_SERVICE_STARTED(GATT_STATUS_SUCCESS, APP1_ID,
								&srvc1_handle),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		ACTION_SUCCESS(gatt_server_connect_action, &app1_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_CONNECTED,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		PROCESS_DATA(GATT_STATUS_SUCCESS,
						gatt_remote_send_raw_pdu_action,
						&att_find_by_type_req_op_v,
						&search_range_1,
						&primary_type),
		CALLBACK_ERROR(CB_EMU_ATT_ERROR, 0x0a),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	/* This tests embeded ccc */
	TEST_CASE_BREDRLE("Gatt Server - Srvc change write req. success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		ACTION_SUCCESS(gatt_server_connect_action, &app1_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_CONNECTED,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		/* For CCC we need to be bonded */
		ACTION_SUCCESS(bt_create_bond_action,
					&prop_test_remote_ble_bdaddr_req),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDED,
					&prop_emu_remotes_default_set[0], 1),
		/* Write and receive confirmation */
		PROCESS_DATA(GATT_STATUS_SUCCESS,
				gatt_remote_send_raw_pdu_action,
				&att_write_req_op_v, &svc_change_ccc_handle_v,
				&svc_change_ccc_value_v),
		CALLBACK(CB_EMU_WRITE_RESPONSE),
		/* Shutdown */
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Gatt Server - Send error resp to write char request",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_set_connect_cb_action, gatt_conn_cb),
		ACTION_SUCCESS(gatt_server_register_action, &app1_uuid),
		CALLBACK_STATUS(CB_GATTS_REGISTER_SERVER, BT_STATUS_SUCCESS),
		ACTION_SUCCESS(gatt_server_add_service_action,
							&add_service_data_5),
		CALLBACK_GATTS_SERVICE_ADDED(GATT_STATUS_SUCCESS, APP1_ID,
							&service_add_1, NULL,
							&srvc1_handle),
		ACTION_SUCCESS(gatt_server_add_char_action, &add_char_data_2),
		CALLBACK_GATTS_CHARACTERISTIC_ADDED(GATT_STATUS_SUCCESS,
							APP1_ID, &app1_uuid,
							&srvc1_handle, NULL,
							&char1_handle),
		ACTION_SUCCESS(gatt_server_start_srvc_action,
							&start_srvc_data_2),
		CALLBACK_GATTS_SERVICE_STARTED(GATT_STATUS_SUCCESS, APP1_ID,
								&srvc1_handle),
		ACTION_SUCCESS(bt_start_discovery_action, NULL),
		CALLBACK_STATE(CB_BT_DISCOVERY_STATE_CHANGED,
							BT_DISCOVERY_STARTED),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_le_set, 2),
		ACTION_SUCCESS(bt_cancel_discovery_action, NULL),
		ACTION_SUCCESS(gatt_server_connect_action, &app1_conn_req),
		CALLBACK_GATTS_CONNECTION(GATT_SERVER_CONNECTED,
						prop_emu_remotes_default_set,
						CONN1_ID, APP1_ID),
		PROCESS_DATA(GATT_STATUS_SUCCESS,
					gatt_remote_send_raw_pdu_action,
					&att_write_req_op_v, &char1_handle_v,
					&att_write_req_value_1_v),
		CALLBACK_GATTS_REQUEST_WRITE(CONN1_ID, TRANS1_ID,
						prop_emu_remotes_default_set,
						&char1_handle, 0,
						sizeof(att_write_req_value_1),
						true, false,
						att_write_req_value_1),
		ACTION_SUCCESS(gatt_server_send_response_action,
						&send_resp_data_2_error),
		CALLBACK_ERROR(CB_EMU_ATT_ERROR, GATT_ERR_INVAL_ATTR_VALUE_LEN),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
		),
};

struct queue *get_gatt_tests(void)
{
	uint16_t i = 0;

	list = queue_new();

	for (; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i)
		queue_push_tail(list, &test_cases[i]);

	return list;
}

void remove_gatt_tests(void)
{
	queue_destroy(list, NULL);
}
