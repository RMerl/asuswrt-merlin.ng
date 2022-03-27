/*
 * Copyright (C) 2013 Intel Corporation
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

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <hardware/bluetooth.h>
#include <hardware/bt_hh.h>

#include "if-main.h"
#include "pollhandler.h"
#include "../hal-utils.h"

const bthh_interface_t *if_hh = NULL;

SINTMAP(bthh_protocol_mode_t, -1, "(unknown)")
	DELEMENT(BTHH_REPORT_MODE),
	DELEMENT(BTHH_BOOT_MODE),
	DELEMENT(BTHH_UNSUPPORTED_MODE),
ENDMAP

SINTMAP(bthh_report_type_t, -1, "(unknown)")
	DELEMENT(BTHH_INPUT_REPORT),
	DELEMENT(BTHH_OUTPUT_REPORT),
	DELEMENT(BTHH_FEATURE_REPORT),
ENDMAP

SINTMAP(bthh_connection_state_t, -1, "(unknown)")
	DELEMENT(BTHH_CONN_STATE_CONNECTED),
	DELEMENT(BTHH_CONN_STATE_CONNECTING),
	DELEMENT(BTHH_CONN_STATE_DISCONNECTED),
	DELEMENT(BTHH_CONN_STATE_DISCONNECTING),
	DELEMENT(BTHH_CONN_STATE_FAILED_MOUSE_FROM_HOST),
	DELEMENT(BTHH_CONN_STATE_FAILED_KBD_FROM_HOST),
	DELEMENT(BTHH_CONN_STATE_FAILED_TOO_MANY_DEVICES),
	DELEMENT(BTHH_CONN_STATE_FAILED_NO_BTHID_DRIVER),
	DELEMENT(BTHH_CONN_STATE_FAILED_GENERIC),
	DELEMENT(BTHH_CONN_STATE_UNKNOWN),
ENDMAP

SINTMAP(bthh_status_t, -1, "(unknown)")
	DELEMENT(BTHH_OK),
	DELEMENT(BTHH_HS_HID_NOT_READY),
	DELEMENT(BTHH_HS_INVALID_RPT_ID),
	DELEMENT(BTHH_HS_TRANS_NOT_SPT),
	DELEMENT(BTHH_HS_INVALID_PARAM),
	DELEMENT(BTHH_HS_ERROR),
	DELEMENT(BTHH_ERR),
	DELEMENT(BTHH_ERR_SDP),
	DELEMENT(BTHH_ERR_PROTO),
	DELEMENT(BTHH_ERR_DB_FULL),
	DELEMENT(BTHH_ERR_TOD_UNSPT),
	DELEMENT(BTHH_ERR_NO_RES),
	DELEMENT(BTHH_ERR_AUTH_FAILED),
	DELEMENT(BTHH_ERR_HDL),
ENDMAP

static char connected_device_addr[MAX_ADDR_STR_LEN];
/*
 * Callback for connection state change.
 * state will have one of the values from bthh_connection_state_t
 */
static void connection_state_cb(bt_bdaddr_t *bd_addr,
						bthh_connection_state_t state)
{
	char addr[MAX_ADDR_STR_LEN];

	haltest_info("%s: bd_addr=%s connection_state=%s\n", __func__,
					bt_bdaddr_t2str(bd_addr, addr),
					bthh_connection_state_t2str(state));
	if (state == BTHH_CONN_STATE_CONNECTED)
		strcpy(connected_device_addr, addr);
}

/*
 * Callback for virtual unplug api.
 * the status of the virtual unplug
 */
static void virtual_unplug_cb(bt_bdaddr_t *bd_addr, bthh_status_t hh_status)
{
	char addr[MAX_ADDR_STR_LEN];

	haltest_info("%s: bd_addr=%s hh_status=%s\n", __func__,
						bt_bdaddr_t2str(bd_addr, addr),
						bthh_status_t2str(hh_status));
}

/* Callback for Android 5.0 handshake api. */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void handshake_cb(bt_bdaddr_t *bd_addr, bthh_status_t hh_status)
{
	char addr[MAX_ADDR_STR_LEN];

	haltest_info("%s: bd_addr=%s hh_status=%s\n", __func__,
						bt_bdaddr_t2str(bd_addr, addr),
						bthh_status_t2str(hh_status));
}
#endif

/*
 * Callback for get hid info
 * hid_info will contain attr_mask, sub_class, app_id, vendor_id, product_id,
 * version, ctry_code, len
 */
static void hid_info_cb(bt_bdaddr_t *bd_addr, bthh_hid_info_t hid_info)
{
	char addr[MAX_ADDR_STR_LEN];

	/* TODO: bluedroid does not seem to ever call this callback */
	haltest_info("%s: bd_addr=%s\n", __func__,
						bt_bdaddr_t2str(bd_addr, addr));
}

/*
 * Callback for get/set protocol api.
 * the protocol mode is one of the value from bthh_protocol_mode_t
 */
static void protocol_mode_cb(bt_bdaddr_t *bd_addr, bthh_status_t hh_status,
						bthh_protocol_mode_t mode)
{
	char addr[MAX_ADDR_STR_LEN];

	haltest_info("%s: bd_addr=%s hh_status=%s mode=%s\n", __func__,
					bt_bdaddr_t2str(bd_addr, addr),
					bthh_status_t2str(hh_status),
					bthh_protocol_mode_t2str(mode));
}

/* Callback for get/set_idle_time api. */
static void idle_time_cb(bt_bdaddr_t *bd_addr, bthh_status_t hh_status,
								int idle_rate)
{
	char addr[MAX_ADDR_STR_LEN];

	haltest_info("%s: bd_addr=%s hh_status=%s idle_rate=%d\n", __func__,
				bt_bdaddr_t2str(bd_addr, addr),
				bthh_status_t2str(hh_status), idle_rate);
}


/*
 * Callback for get report api.
 * if status is ok rpt_data contains the report data
 */
static void get_report_cb(bt_bdaddr_t *bd_addr, bthh_status_t hh_status,
						uint8_t *rpt_data, int rpt_size)
{
	char addr[MAX_ADDR_STR_LEN];

	/* TODO: print actual report */
	haltest_info("%s: bd_addr=%s hh_status=%s rpt_size=%d\n", __func__,
					bt_bdaddr_t2str(bd_addr, addr),
					bthh_status_t2str(hh_status), rpt_size);
}

static bthh_callbacks_t bthh_callbacks = {
	.size = sizeof(bthh_callbacks),
	.connection_state_cb = connection_state_cb,
	.hid_info_cb = hid_info_cb,
	.protocol_mode_cb = protocol_mode_cb,
	.idle_time_cb = idle_time_cb,
	.get_report_cb = get_report_cb,
	.virtual_unplug_cb = virtual_unplug_cb,
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	.handshake_cb = handshake_cb
#endif
};

/* init */

static void init_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hh);

	EXEC(if_hh->init, &bthh_callbacks);
}

/* connect */

static void connect_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = (void *) connected_device_addr;
		*enum_func = enum_one_string;
	}
}

static void connect_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_hh);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_hh->connect, &addr);
}

/* disconnect */

/* Same completion as connect_c */
#define disconnect_c connect_c

static void disconnect_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_hh);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_hh->disconnect, &addr);
}

/* virtual_unplug */

/* Same completion as connect_c */
#define virtual_unplug_c connect_c

static void virtual_unplug_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_hh);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_hh->virtual_unplug, &addr);
}

/* set_info */

/* Same completion as connect_c */
#define set_info_c connect_c

static void set_info_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;
	bthh_hid_info_t hid_info;

	RETURN_IF_NULL(if_hh);
	VERIFY_ADDR_ARG(2, &addr);

	memset(&hid_info, 0, sizeof(hid_info));

	/*
	 * This command is intentionally not supported. See comment from
	 * bt_hid_info() in android/hidhost.c
	 */
	EXEC(if_hh->set_info, &addr, hid_info);
}

/* get_protocol */

static void get_protocol_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = connected_device_addr;
		*enum_func = enum_one_string;
	} else if (argc == 4) {
		*user = TYPE_ENUM(bthh_protocol_mode_t);
		*enum_func = enum_defines;
	}
}

static void get_protocol_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;
	bthh_protocol_mode_t protocolMode;

	RETURN_IF_NULL(if_hh);
	VERIFY_ADDR_ARG(2, &addr);

	if (argc < 4) {
		haltest_error("No protocol mode specified\n");
		return;
	}
	protocolMode = str2bthh_protocol_mode_t(argv[3]);

	EXEC(if_hh->get_protocol, &addr, protocolMode);
}

/* set_protocol */

/* Same completion as get_protocol_c */
#define set_protocol_c get_protocol_c

static void set_protocol_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;
	bthh_protocol_mode_t protocolMode;

	RETURN_IF_NULL(if_hh);
	VERIFY_ADDR_ARG(2, &addr);

	if (argc < 4) {
		haltest_error("No protocol mode specified\n");
		return;
	}
	protocolMode = str2bthh_protocol_mode_t(argv[3]);

	EXEC(if_hh->set_protocol, &addr, protocolMode);
}

/* get_report */

static void get_report_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = connected_device_addr;
		*enum_func = enum_one_string;
	} else if (argc == 4) {
		*user = TYPE_ENUM(bthh_report_type_t);
		*enum_func = enum_defines;
	}
}

static void get_report_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;
	bthh_report_type_t reportType;
	uint8_t reportId;
	int bufferSize;

	RETURN_IF_NULL(if_hh);
	VERIFY_ADDR_ARG(2, &addr);

	if (argc < 4) {
		haltest_error("No report type specified\n");
		return;
	}
	reportType = str2bthh_report_type_t(argv[3]);

	if (argc < 5) {
		haltest_error("No reportId specified\n");
		return;
	}
	reportId = (uint8_t) atoi(argv[4]);

	if (argc < 6) {
		haltest_error("No bufferSize specified\n");
		return;
	}
	bufferSize = atoi(argv[5]);

	EXEC(if_hh->get_report, &addr, reportType, reportId, bufferSize);
}

/* set_report */

static void set_report_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = connected_device_addr;
		*enum_func = enum_one_string;
	} else if (argc == 4) {
		*user = TYPE_ENUM(bthh_report_type_t);
		*enum_func = enum_defines;
	}
}

static void set_report_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;
	bthh_report_type_t reportType;

	RETURN_IF_NULL(if_hh);
	VERIFY_ADDR_ARG(2, &addr);

	if (argc <= 3) {
		haltest_error("No report type specified\n");
		return;
	}
	reportType = str2bthh_report_type_t(argv[3]);

	if (argc <= 4) {
		haltest_error("No report specified\n");
		return;
	}

	EXEC(if_hh->set_report, &addr, reportType, (char *) argv[4]);
}

/* send_data */

static void send_data_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = connected_device_addr;
		*enum_func = enum_one_string;
	}
}

static void send_data_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_hh);
	VERIFY_ADDR_ARG(2, &addr);

	if (argc <= 3) {
		haltest_error("No data to send specified\n");
		return;
	}

	EXEC(if_hh->send_data, &addr, (char *) argv[3]);
}

/* cleanup */

static void cleanup_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hh);

	EXECV(if_hh->cleanup);
}

/* Methods available in bthh_interface_t */
static struct method methods[] = {
	STD_METHOD(init),
	STD_METHODCH(connect, "<addr>"),
	STD_METHODCH(disconnect, "<addr>"),
	STD_METHODCH(virtual_unplug, "<addr>"),
	STD_METHODCH(set_info, "<addr>"),
	STD_METHODCH(get_protocol, "<addr> <mode>"),
	STD_METHODCH(set_protocol, "<addr> <mode>"),
	STD_METHODCH(get_report, "<addr> <type> <report_id> <size>"),
	STD_METHODCH(set_report, "<addr> <type> <hex_encoded_report>"),
	STD_METHODCH(send_data, "<addr> <hex_encoded_data>"),
	STD_METHOD(cleanup),
	END_METHOD
};

const struct interface hh_if = {
	.name = "hidhost",
	.methods = methods
};
