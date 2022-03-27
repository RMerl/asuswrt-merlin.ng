/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
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

#include <glib.h>
#include <hardware/audio.h>
#include <hardware/bluetooth.h>
#include <hardware/bt_sock.h>
#include <hardware/bt_hh.h>
#include <hardware/bt_pan.h>
#include <hardware/bt_hl.h>
#include <hardware/bt_av.h>
#include <hardware/bt_rc.h>
#include <hardware/bt_gatt.h>

#include "emulator/hciemu.h"
#include <hardware/bt_mce.h>

struct pdu_set {
	struct iovec req;
	struct iovec rsp;
};

#define raw_data(args...) ((unsigned char[]) { args })

#define raw_pdu(args...)					\
	{							\
		.iov_base = raw_data(args),			\
		.iov_len = sizeof(raw_data(args)),		\
	}

#define end_pdu { .iov_base = NULL }

#define TEST_CASE_BREDR(text, ...) { \
		HCIEMU_TYPE_BREDR, \
		text, \
		sizeof((struct step[]) {__VA_ARGS__}) / sizeof(struct step), \
		(struct step[]) {__VA_ARGS__}, \
	}

#define TEST_CASE_BREDRLE(text, ...) { \
		HCIEMU_TYPE_BREDRLE, \
		text, \
		sizeof((struct step[]) {__VA_ARGS__}) / sizeof(struct step), \
		(struct step[]) {__VA_ARGS__}, \
	}

#define MODIFY_DATA(status, modif_fun, from, to, len) { \
		.action_status = status, \
		.action = modif_fun, \
		.set_data = from, \
		.set_data_2 = to, \
		.set_data_len = len, \
	}

#define PROCESS_DATA(status, proc_fun, data1, data2, data3) { \
		.action_status = status, \
		.action = proc_fun, \
		.set_data = data1, \
		.set_data_2 = data2, \
		.set_data_3 = data3, \
	}

#define ACTION(status, act_fun, data_set) { \
		.action_status = status, \
		.action = act_fun, \
		.set_data = data_set, \
	}

#define ACTION_FAIL(act_fun, data_set) \
		ACTION(BT_STATUS_FAIL, act_fun, data_set)

#define ACTION_SUCCESS(act_fun, data_set) \
		ACTION(BT_STATUS_SUCCESS, act_fun, data_set)

#define CALLBACK(cb) { \
		.callback = cb, \
	}

#define CALLBACK_STATE(cb, cb_res) { \
		.callback = cb, \
		.callback_result.state = cb_res, \
	}

#define CALLBACK_STATUS(cb, cb_res) { \
		.callback = cb, \
		.callback_result.status = cb_res, \
	}

#define CALLBACK_ERROR(cb, cb_err) { \
		.callback = cb, \
		.callback_result.error = cb_err, \
	}

#define CALLBACK_ADAPTER_PROPS(props, prop_cnt) { \
		.callback = CB_BT_ADAPTER_PROPERTIES, \
		.callback_result.properties = props, \
		.callback_result.num_properties = prop_cnt, \
	}

#define CALLBACK_PROPS(cb, props, prop_cnt) { \
		.callback = cb, \
		.callback_result.properties = props, \
		.callback_result.num_properties = prop_cnt, \
	}

#define CALLBACK_HH_MODE(cb, cb_res, cb_mode) { \
		.callback = cb, \
		.callback_result.status = cb_res, \
		.callback_result.mode = cb_mode, \
	}

#define CALLBACK_HHREPORT(cb, cb_res, cb_rep_size) { \
		.callback = cb, \
		.callback_result.status = cb_res, \
		.callback_result.report_size = cb_rep_size, \
	}

#define CLLBACK_GATTC_SCAN_RES(props, prop_cnt, cb_adv_data) {\
		.callback = CB_GATTC_SCAN_RESULT, \
		.callback_result.properties = props, \
		.callback_result.num_properties = prop_cnt, \
		.callback_result.adv_data = cb_adv_data, \
	}

#define CALLBACK_GATTC_CONNECT(cb_res, cb_prop, cb_conn_id, cb_client_id) { \
		.callback = CB_GATTC_OPEN, \
		.callback_result.status = cb_res, \
		.callback_result.properties = cb_prop, \
		.callback_result.num_properties = 1, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.gatt_app_id = cb_client_id, \
	}

#define CALLBACK_GATTC_SEARCH_RESULT(cb_conn_id, cb_service) { \
		.callback = CB_GATTC_SEARCH_RESULT, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.service = cb_service \
	}

#define CALLBACK_GATTC_SEARCH_COMPLETE(cb_res, cb_conn_id) { \
		.callback = CB_GATTC_SEARCH_COMPLETE, \
		.callback_result.conn_id = cb_conn_id \
	}
#define CALLBACK_GATTC_GET_CHARACTERISTIC_CB(cb_res, cb_conn_id, cb_service, \
						cb_char, cb_char_prop) { \
		.callback = CB_GATTC_GET_CHARACTERISTIC, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.status = cb_res, \
		.callback_result.service = cb_service, \
		.callback_result.characteristic = cb_char, \
		.callback_result.char_prop = cb_char_prop \
	}

#define CALLBACK_GATTC_GET_DESCRIPTOR(cb_res, cb_conn_id, cb_service, \
						cb_char, cb_desc) { \
		.callback = CB_GATTC_GET_DESCRIPTOR, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.status = cb_res, \
		.callback_result.service = cb_service, \
		.callback_result.characteristic = cb_char, \
		.callback_result.descriptor = cb_desc \
	}

#define CALLBACK_GATTC_GET_INCLUDED(cb_res, cb_conn_id, cb_service, \
							cb_incl) { \
		.callback = CB_GATTC_GET_INCLUDED_SERVICE, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.status = cb_res, \
		.callback_result.service = cb_service, \
		.callback_result.included = cb_incl, \
	}

#define CALLBACK_GATTC_READ_CHARACTERISTIC(cb_res, cb_conn_id, cb_read_data) { \
		.callback = CB_GATTC_READ_CHARACTERISTIC, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.status = cb_res, \
		.callback_result.read_params = cb_read_data, \
	}

#define CALLBACK_GATTC_READ_DESCRIPTOR(cb_res, cb_conn_id, cb_read_data) { \
		.callback = CB_GATTC_READ_DESCRIPTOR, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.status = cb_res, \
		.callback_result.read_params = cb_read_data, \
	}

#define CALLBACK_GATTC_WRITE_DESCRIPTOR(cb_res, cb_conn_id, cb_write_data) { \
		.callback = CB_GATTC_WRITE_DESCRIPTOR, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.status = cb_res, \
		.callback_result.write_params = cb_write_data, \
	}

#define CALLBACK_GATTC_WRITE_CHARACTERISTIC(cb_res, cb_conn_id, \
							cb_write_data) { \
		.callback = CB_GATTC_WRITE_CHARACTERISTIC, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.status = cb_res, \
		.callback_result.write_params = cb_write_data, \
	}

#define CALLBACK_GATTC_REGISTER_FOR_NOTIF(cb_res, cb_conn_id, cb_char,\
						cb_service, cb_registered) { \
		.callback = CB_GATTC_REGISTER_FOR_NOTIFICATION, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.status = cb_res, \
		.callback_result.service = cb_service, \
		.callback_result.characteristic = cb_char, \
		.callback_result.notification_registered = cb_registered \
	}

#define CALLBACK_GATTC_NOTIFY(cb_conn_id, cb_notify) { \
		.callback = CB_GATTC_NOTIFY, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.notify_params = cb_notify \
	}

#define CALLBACK_GATTC_DISCONNECT(cb_res, cb_prop, cb_conn_id, cb_client_id) { \
		.callback = CB_GATTC_CLOSE, \
		.callback_result.status = cb_res, \
		.callback_result.properties = cb_prop, \
		.callback_result.num_properties = 1, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.gatt_app_id = cb_client_id, \
	}

#define CALLBACK_GATTS_CONNECTION(cb_res, cb_prop, cb_conn_id, cb_server_id) { \
		.callback = CB_GATTS_CONNECTION, \
		.callback_result.connected = cb_res, \
		.callback_result.properties = cb_prop, \
		.callback_result.num_properties = 1, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.gatt_app_id = cb_server_id, \
	}

#define CALLBACK_GATTS_NOTIF_CONF(cb_conn_id, cb_status) { \
		.callback = CB_GATTS_INDICATION_SEND, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.status = cb_status, \
	}

#define CALLBACK_GATTS_SERVICE_ADDED(cb_res, cb_server_id, cb_service, \
						cb_srvc_handle, \
						cb_store_srvc_handle) { \
		.callback = CB_GATTS_SERVICE_ADDED, \
		.callback_result.status = cb_res, \
		.callback_result.gatt_app_id = cb_server_id, \
		.callback_result.service = cb_service, \
		.callback_result.srvc_handle = cb_srvc_handle, \
		.store_srvc_handle = cb_store_srvc_handle, \
	}

#define CALLBACK_GATTS_INC_SERVICE_ADDED(cb_res, cb_server_id, cb_srvc_handle, \
							cb_inc_srvc_handle) { \
		.callback = CB_GATTS_INCLUDED_SERVICE_ADDED, \
		.callback_result.status = cb_res, \
		.callback_result.gatt_app_id = cb_server_id, \
		.callback_result.srvc_handle = cb_srvc_handle, \
		.callback_result.inc_srvc_handle = cb_inc_srvc_handle, \
	}

#define CALLBACK_GATTS_CHARACTERISTIC_ADDED(cb_res, cb_server_id, cb_uuid, \
						cb_srvc_handle, \
						cb_char_handle, \
						cb_store_char_handle) { \
		.callback = CB_GATTS_CHARACTERISTIC_ADDED, \
		.callback_result.status = cb_res, \
		.callback_result.gatt_app_id = cb_server_id, \
		.callback_result.uuid = cb_uuid, \
		.callback_result.srvc_handle = cb_srvc_handle, \
		.callback_result.char_handle = cb_char_handle, \
		.store_char_handle = cb_store_char_handle, \
	}

#define CALLBACK_GATTS_DESCRIPTOR_ADDED(cb_res, cb_server_id, cb_uuid, \
					cb_srvc_handle, cb_desc_handle, \
					cb_store_desc_handle) { \
		.callback = CB_GATTS_DESCRIPTOR_ADDED, \
		.callback_result.status = cb_res, \
		.callback_result.gatt_app_id = cb_server_id, \
		.callback_result.uuid = cb_uuid, \
		.callback_result.srvc_handle = cb_srvc_handle, \
		.callback_result.desc_handle = cb_desc_handle, \
		.store_desc_handle = cb_store_desc_handle, \
	}

#define CALLBACK_GATTS_SERVICE_STARTED(cb_res, cb_server_id, cb_srvc_handle) { \
		.callback = CB_GATTS_SERVICE_STARTED, \
		.callback_result.status = cb_res, \
		.callback_result.gatt_app_id = cb_server_id, \
		.callback_result.srvc_handle = cb_srvc_handle, \
	}

#define CALLBACK_GATTS_SERVICE_STOPPED(cb_res, cb_server_id, cb_srvc_handle) { \
		.callback = CB_GATTS_SERVICE_STOPPED, \
		.callback_result.status = cb_res, \
		.callback_result.gatt_app_id = cb_server_id, \
		.callback_result.srvc_handle = cb_srvc_handle, \
	}

#define CALLBACK_GATTS_SERVICE_DELETED(cb_res, cb_server_id, cb_srvc_handle) { \
		.callback = CB_GATTS_SERVICE_DELETED, \
		.callback_result.status = cb_res, \
		.callback_result.gatt_app_id = cb_server_id, \
		.callback_result.srvc_handle = cb_srvc_handle, \
	}

#define CALLBACK_GATTS_REQUEST_READ(cb_conn_id, cb_trans_id, cb_prop, \
						cb_attr_handle, cb_offset, \
						cb_is_long) { \
		.callback = CB_GATTS_REQUEST_READ, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.trans_id = cb_trans_id, \
		.callback_result.properties = cb_prop, \
		.callback_result.num_properties = 1, \
		.callback_result.attr_handle = cb_attr_handle, \
		.callback_result.offset = cb_offset, \
		.callback_result.is_long = cb_is_long, \
	}

#define CALLBACK_GATTS_REQUEST_WRITE(cb_conn_id, cb_trans_id, cb_prop, \
						cb_attr_handle, cb_offset, \
						cb_length, cb_need_rsp, \
						cb_is_prep, cb_value) { \
		.callback = CB_GATTS_REQUEST_WRITE, \
		.callback_result.conn_id = cb_conn_id, \
		.callback_result.trans_id = cb_trans_id, \
		.callback_result.properties = cb_prop, \
		.callback_result.num_properties = 1, \
		.callback_result.attr_handle = cb_attr_handle, \
		.callback_result.offset = cb_offset, \
		.callback_result.length = cb_length, \
		.callback_result.need_rsp = cb_need_rsp, \
		.callback_result.is_prep = cb_is_prep, \
		.callback_result.value = cb_value, \
	}

#define CALLBACK_MAP_CLIENT_REMOTE_MAS_INSTANCE(cb_status, cb_prop, \
						cb_num_inst, cb_instances) { \
		.callback = CB_MAP_CLIENT_REMOTE_MAS_INSTANCES, \
		.callback_result.properties = cb_prop, \
		.callback_result.num_properties = 1, \
		.callback_result.status = cb_status, \
		.callback_result.num_mas_instances = cb_num_inst, \
		.callback_result.mas_instances = cb_instances, \
	}

#define CALLBACK_PAN_CTRL_STATE(cb, cb_res, cb_state, cb_local_role) { \
		.callback = cb, \
		.callback_result.status = cb_res, \
		.callback_result.ctrl_state = cb_state, \
		.callback_result.local_role = cb_local_role, \
	}

#define CALLBACK_PAN_CONN_STATE(cb, cb_res, cb_state, cb_local_role, \
							cb_remote_role) { \
		.callback = cb, \
		.callback_result.status = cb_res, \
		.callback_result.conn_state = cb_state, \
		.callback_result.local_role = cb_local_role, \
		.callback_result.remote_role = cb_remote_role, \
	}

#define CALLBACK_HDP_APP_REG_STATE(cb, cb_app_id, cb_state) { \
		.callback = cb, \
		.callback_result.app_id = cb_app_id, \
		.callback_result.app_state = cb_state, \
	}

#define CALLBACK_HDP_CHANNEL_STATE(cb, cb_app_id, cb_channel_id, \
					cb_mdep_cfg_index, cb_state) { \
		.callback = cb, \
		.callback_result.app_id = cb_app_id, \
		.callback_result.channel_id = cb_channel_id, \
		.callback_result.mdep_cfg_index = cb_mdep_cfg_index, \
		.callback_result.channel_state = cb_state, \
	}

#define CALLBACK_AV_CONN_STATE(cb, cb_av_conn_state) { \
		.callback = cb, \
		.callback_result.av_conn_state = cb_av_conn_state, \
	}

#define CALLBACK_AV_AUDIO_STATE(cb, cb_av_audio_state) { \
		.callback = cb, \
		.callback_result.av_audio_state = cb_av_audio_state, \
	}

#define CALLBACK_RC_PLAY_STATUS(cb, cb_length, cb_position, cb_status) { \
		.callback = cb, \
		.callback_result.song_length = cb_length, \
		.callback_result.song_position = cb_position, \
		.callback_result.play_status = cb_status, \
	}

#define CALLBACK_RC_REG_NOTIF_TRACK_CHANGED(cb, cb_index) { \
		.callback = cb, \
		.callback_result.rc_index = cb_index, \
	}

#define CALLBACK_RC_REG_NOTIF_POSITION_CHANGED(cb, cb_position) { \
		.callback = cb, \
		.callback_result.song_position = cb_position, \
	}

#define CALLBACK_RC_REG_NOTIF_STATUS_CHANGED(cb, cb_status) { \
		.callback = cb, \
		.callback_result.play_status = cb_status, \
	}

#define CALLBACK_RC_GET_ELEMENT_ATTRIBUTES(cb, cb_num_of_attrs, cb_attrs) { \
		.callback = cb, \
		.callback_result.num_of_attrs = cb_num_of_attrs, \
		.callback_result.attrs = cb_attrs, \
	}

#define CALLBACK_DEVICE_PROPS(props, prop_cnt) \
	CALLBACK_PROPS(CB_BT_REMOTE_DEVICE_PROPERTIES, props, prop_cnt)

#define CALLBACK_DEVICE_FOUND(props, prop_cnt) \
	CALLBACK_PROPS(CB_BT_DEVICE_FOUND, props, prop_cnt)

#define CALLBACK_BOND_STATE(cb_res, props, prop_cnt) { \
		.callback = CB_BT_BOND_STATE_CHANGED, \
		.callback_result.state = cb_res, \
		.callback_result.properties = props, \
		.callback_result.num_properties = prop_cnt, \
	}

#define CALLBACK_BOND_STATE_FAILED(cb_res, props, prop_cnt, reason) { \
		.callback = CB_BT_BOND_STATE_CHANGED, \
		.callback_result.state = cb_res, \
		.callback_result.status = reason, \
		.callback_result.properties = props, \
		.callback_result.num_properties = prop_cnt, \
	}

#define CALLBACK_SSP_REQ(pair_var, props, prop_cnt) { \
		.callback = CB_BT_SSP_REQUEST, \
		.callback_result.pairing_variant = pair_var, \
		.callback_result.properties = props, \
		.callback_result.num_properties = prop_cnt, \
	}

#define DBG_CB(cb) { cb, #cb }

/*
 * NOTICE:
 * Callback enum sections should be
 * updated while adding new HAL to tester.
 */
typedef enum {
	CB_BT_NONE,
	CB_BT_ADAPTER_STATE_CHANGED,
	CB_BT_ADAPTER_PROPERTIES,
	CB_BT_REMOTE_DEVICE_PROPERTIES,
	CB_BT_DEVICE_FOUND,
	CB_BT_DISCOVERY_STATE_CHANGED,
	CB_BT_PIN_REQUEST,
	CB_BT_SSP_REQUEST,
	CB_BT_BOND_STATE_CHANGED,
	CB_BT_ACL_STATE_CHANGED,
	CB_BT_THREAD_EVT,
	CB_BT_DUT_MODE_RECV,
	CB_BT_LE_TEST_MODE,

	/* Hidhost cb */
	CB_HH_CONNECTION_STATE,
	CB_HH_HID_INFO,
	CB_HH_PROTOCOL_MODE,
	CB_HH_IDLE_TIME,
	CB_HH_GET_REPORT,
	CB_HH_VIRTUAL_UNPLUG,

	/* PAN cb */
	CB_PAN_CONTROL_STATE,
	CB_PAN_CONNECTION_STATE,

	/* HDP cb */
	CB_HDP_APP_REG_STATE,
	CB_HDP_CHANNEL_STATE,

	/* A2DP cb */
	CB_A2DP_CONN_STATE,
	CB_A2DP_AUDIO_STATE,

	/* AVRCP */
	CB_AVRCP_PLAY_STATUS_REQ,
	CB_AVRCP_PLAY_STATUS_RSP,
	CB_AVRCP_REG_NOTIF_REQ,
	CB_AVRCP_REG_NOTIF_RSP,
	CB_AVRCP_GET_ATTR_REQ,
	CB_AVRCP_GET_ATTR_RSP,

	/* Gatt client */
	CB_GATTC_REGISTER_CLIENT,
	CB_GATTC_SCAN_RESULT,
	CB_GATTC_OPEN,
	CB_GATTC_CLOSE,
	CB_GATTC_SEARCH_COMPLETE,
	CB_GATTC_SEARCH_RESULT,
	CB_GATTC_GET_CHARACTERISTIC,
	CB_GATTC_GET_DESCRIPTOR,
	CB_GATTC_GET_INCLUDED_SERVICE,
	CB_GATTC_REGISTER_FOR_NOTIFICATION,
	CB_GATTC_NOTIFY,
	CB_GATTC_READ_CHARACTERISTIC,
	CB_GATTC_WRITE_CHARACTERISTIC,
	CB_GATTC_READ_DESCRIPTOR,
	CB_GATTC_WRITE_DESCRIPTOR,
	CB_GATTC_EXECUTE_WRITE,
	CB_GATTC_READ_REMOTE_RSSI,
	CB_GATTC_LISTEN,

	/* Gatt server */
	CB_GATTS_REGISTER_SERVER,
	CB_GATTS_CONNECTION,
	CB_GATTS_SERVICE_ADDED,
	CB_GATTS_INCLUDED_SERVICE_ADDED,
	CB_GATTS_CHARACTERISTIC_ADDED,
	CB_GATTS_DESCRIPTOR_ADDED,
	CB_GATTS_SERVICE_STARTED,
	CB_GATTS_SERVICE_STOPPED,
	CB_GATTS_SERVICE_DELETED,
	CB_GATTS_REQUEST_READ,
	CB_GATTS_REQUEST_WRITE,
	CB_GATTS_REQUEST_EXEC_WRITE,
	CB_GATTS_RESPONSE_CONFIRMATION,
	CB_GATTS_INDICATION_SEND,

	/* Map client */
	CB_MAP_CLIENT_REMOTE_MAS_INSTANCES,

	/* Emulator callbacks */
	CB_EMU_CONFIRM_SEND_DATA,
	CB_EMU_ENCRYPTION_ENABLED,
	CB_EMU_ENCRYPTION_DISABLED,
	CB_EMU_CONNECTION_REJECTED,
	CB_EMU_VALUE_INDICATION,
	CB_EMU_VALUE_NOTIFICATION,
	CB_EMU_READ_RESPONSE,
	CB_EMU_WRITE_RESPONSE,
	CB_EMU_ATT_ERROR,
} expected_bt_callback_t;

struct test_data {
	struct mgmt *mgmt;
	audio_hw_device_t *audio;
	struct hw_device_t *device;
	struct hciemu *hciemu;
	enum hciemu_type hciemu_type;

	const bt_interface_t *if_bluetooth;
	const btsock_interface_t *if_sock;
	const bthh_interface_t *if_hid;
	const btpan_interface_t *if_pan;
	const bthl_interface_t *if_hdp;
	const btav_interface_t *if_a2dp;
	struct audio_stream_out *if_stream;
	const btrc_interface_t *if_avrcp;
	const btgatt_interface_t *if_gatt;
	const btmce_interface_t *if_map_client;

	const void *test_data;
	struct queue *steps;

	guint signalfd;
	uint16_t mgmt_index;
	pid_t bluetoothd_pid;

	struct queue *pdus;
};

/*
 * Struct holding bluetooth HAL action parameters
 */
struct bt_action_data {
	bt_bdaddr_t *addr;

	/* Remote props action arguments */
	const int prop_type;
	const bt_property_t *prop;

	/* Bonding requests parameters */
	bt_pin_code_t *pin;
	const uint8_t pin_len;
	const uint8_t ssp_variant;
	const bool accept;
	const uint16_t io_cap;

	/* Socket HAL specific params */
	const btsock_type_t sock_type;
	const int channel;
	const uint8_t *service_uuid;
	const char *service_name;
	const int flags;
	int *fd;

	/* HidHost params */
	const int report_size;

	/*Connection params*/
	const uint8_t bearer_type;
	const uint8_t transport_type;
};

/* bthost's l2cap server setup parameters */
struct emu_set_l2cap_data {
	const uint16_t psm;
	const bthost_l2cap_connect_cb func;
	void *user_data;
};

struct emu_l2cap_cid_data {
	const struct pdu_set *pdu;

	uint16_t handle;
	uint16_t cid;
	bool is_sdp;
};

struct map_inst_data {
	int32_t id;
	int32_t scn;
	int32_t msg_types;
	int32_t name_len;
	uint8_t *name;
};

/*
 * Callback data structure should be enhanced with data
 * returned by callbacks. It's used for test case step
 * matching with expected step data.
 */
struct bt_callback_data {
	bt_state_t state;
	bt_status_t status;
	int num_properties;
	bt_property_t *properties;
	bt_uuid_t *uuid;

	bt_ssp_variant_t pairing_variant;

	bthh_protocol_mode_t mode;
	int report_size;

	bool adv_data;

	int gatt_app_id;
	int conn_id;
	int trans_id;
	int offset;
	bool is_long;
	int connected;
	uint16_t *attr_handle;
	uint16_t *srvc_handle;
	uint16_t *inc_srvc_handle;
	uint16_t *char_handle;
	uint16_t *desc_handle;
	btgatt_srvc_id_t *service;
	btgatt_gatt_id_t *characteristic;
	btgatt_gatt_id_t *descriptor;
	btgatt_srvc_id_t *included;
	btgatt_read_params_t *read_params;
	btgatt_write_params_t *write_params;
	btgatt_notify_params_t *notify_params;
	int notification_registered;
	int char_prop;
	int length;
	uint8_t *value;
	bool need_rsp;
	bool is_prep;
	uint8_t error;

	btpan_control_state_t ctrl_state;
	btpan_connection_state_t conn_state;
	int local_role;
	int remote_role;

	int app_id;
	int channel_id;
	int mdep_cfg_index;
	bthl_app_reg_state_t app_state;
	bthl_channel_state_t channel_state;

	btav_connection_state_t av_conn_state;
	btav_audio_state_t av_audio_state;
	uint32_t song_length;
	uint32_t song_position;
	btrc_play_status_t play_status;
	uint64_t rc_index;
	uint8_t num_of_attrs;
	btrc_element_attr_val_t *attrs;

	int num_mas_instances;
	btmce_mas_instance_t *mas_instances;
};

/*
 * Step structure contains expected step data and step
 * action, which should be performed before step check.
 */
struct step {
	void (*action)(void);
	int action_status;

	expected_bt_callback_t callback;
	struct bt_callback_data callback_result;

	void *set_data;
	void *set_data_2;
	void *set_data_3;
	int set_data_len;

	uint16_t *store_srvc_handle;
	uint16_t *store_char_handle;
	uint16_t *store_desc_handle;
};

struct test_case {
	const uint8_t emu_type;
	const char *title;
	const uint16_t step_num;
	const struct step *step;
};

void tester_handle_l2cap_data_exchange(struct emu_l2cap_cid_data *cid_data);
void tester_generic_connect_cb(uint16_t handle, uint16_t cid, void *user_data);

/* Get, remove test cases API */
struct queue *get_bluetooth_tests(void);
void remove_bluetooth_tests(void);
struct queue *get_socket_tests(void);
void remove_socket_tests(void);
struct queue *get_hidhost_tests(void);
void remove_hidhost_tests(void);
struct queue *get_pan_tests(void);
void remove_pan_tests(void);
struct queue *get_hdp_tests(void);
void remove_hdp_tests(void);
struct queue *get_a2dp_tests(void);
void remove_a2dp_tests(void);
struct queue *get_avrcp_tests(void);
void remove_avrcp_tests(void);
struct queue *get_gatt_tests(void);
void remove_gatt_tests(void);
struct queue *get_map_client_tests(void);
void remove_map_client_tests(void);

/* Generic tester API */
void schedule_action_verification(struct step *step);
void schedule_callback_verification(struct step *step);

/* Emulator actions */
void emu_setup_powered_remote_action(void);
void emu_set_pin_code_action(void);
void emu_set_ssp_mode_action(void);
void emu_set_connect_cb_action(void);
void emu_remote_connect_hci_action(void);
void emu_remote_disconnect_hci_action(void);
void emu_set_io_cap(void);
void emu_add_l2cap_server_action(void);
void emu_add_rfcomm_server_action(void);

/* Actions */
void dummy_action(void);
void bluetooth_enable_action(void);
void bluetooth_disable_action(void);
void bt_set_property_action(void);
void bt_get_property_action(void);
void bt_start_discovery_action(void);
void bt_cancel_discovery_action(void);
void bt_get_device_props_action(void);
void bt_get_device_prop_action(void);
void bt_set_device_prop_action(void);
void bt_create_bond_action(void);
void bt_pin_reply_accept_action(void);
void bt_ssp_reply_accept_action(void);
void bt_cancel_bond_action(void);
void bt_remove_bond_action(void);
void set_default_ssp_request_handler(void);
