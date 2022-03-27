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

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "hal-log.h"
#include "hal.h"
#include "hal-msg.h"
#include "ipc-common.h"
#include "hal-ipc.h"

static const bthh_callbacks_t *cbacks;

static bool interface_ready(void)
{
	return cbacks != NULL;
}

static void handle_conn_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hidhost_conn_state *ev = buf;

	if (cbacks->connection_state_cb)
		cbacks->connection_state_cb((bt_bdaddr_t *) ev->bdaddr,
								ev->state);
}

static void handle_info(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hidhost_info *ev = buf;
	bthh_hid_info_t info;

	info.attr_mask = ev->attr;
	info.sub_class = ev->subclass;
	info.app_id = ev->app_id;
	info.vendor_id = ev->vendor;
	info.product_id = ev->product;
	info.version = ev->version;
	info.ctry_code = ev->country;
	info.dl_len = ev->descr_len;
	memcpy(info.dsc_list, ev->descr, info.dl_len);

	if (cbacks->hid_info_cb)
		cbacks->hid_info_cb((bt_bdaddr_t *) ev->bdaddr, info);
}

static void handle_proto_mode(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hidhost_proto_mode *ev = buf;

	if (cbacks->protocol_mode_cb)
		cbacks->protocol_mode_cb((bt_bdaddr_t *) ev->bdaddr,
							ev->status, ev->mode);
}

static void handle_idle_time(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hidhost_idle_time *ev = buf;

	if (cbacks->idle_time_cb)
		cbacks->idle_time_cb((bt_bdaddr_t *) ev->bdaddr, ev->status,
								ev->idle_rate);
}

static void handle_get_report(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hidhost_get_report *ev = buf;

	if (len != sizeof(*ev) + ev->len) {
		error("invalid get report event, aborting");
		exit(EXIT_FAILURE);
	}

	if (cbacks->get_report_cb)
		cbacks->get_report_cb((bt_bdaddr_t *) ev->bdaddr, ev->status,
							ev->data, ev->len);
}

static void handle_virtual_unplug(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hidhost_virtual_unplug *ev = buf;

	if (cbacks->virtual_unplug_cb)
		cbacks->virtual_unplug_cb((bt_bdaddr_t *) ev->bdaddr,
								ev->status);
}

static void handle_handshake(void *buf, uint16_t len, int fd)
{
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	struct hal_ev_hidhost_handshake *ev = buf;

	if (cbacks->handshake_cb)
		cbacks->handshake_cb((bt_bdaddr_t *) ev->bdaddr, ev->status);
#endif
}

/*
 * handlers will be called from notification thread context,
 * index in table equals to 'opcode - HAL_MINIMUM_EVENT'
 */
static const struct hal_ipc_handler ev_handlers[] = {
	/* HAL_EV_HIDHOST_CONN_STATE */
	{ handle_conn_state, false, sizeof(struct hal_ev_hidhost_conn_state) },
	/* HAL_EV_HIDHOST_INFO */
	{ handle_info, false, sizeof(struct hal_ev_hidhost_info) },
	/* HAL_EV_HIDHOST_PROTO_MODE */
	{ handle_proto_mode, false, sizeof(struct hal_ev_hidhost_proto_mode) },
	/* HAL_EV_HIDHOST_IDLE_TIME */
	{ handle_idle_time, false, sizeof(struct hal_ev_hidhost_idle_time) },
	/* HAL_EV_HIDHOST_GET_REPORT */
	{ handle_get_report, true, sizeof(struct hal_ev_hidhost_get_report) },
	/* HAL_EV_HIDHOST_VIRTUAL_UNPLUG */
	{ handle_virtual_unplug, false,
				sizeof(struct hal_ev_hidhost_virtual_unplug) },
	{ handle_handshake, false, sizeof(struct hal_ev_hidhost_handshake) },
};

static bt_status_t hidhost_connect(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_hidhost_connect cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HIDHOST, HAL_OP_HIDHOST_CONNECT,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t disconnect(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_hidhost_disconnect cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HIDHOST, HAL_OP_HIDHOST_DISCONNECT,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t virtual_unplug(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_hidhost_virtual_unplug cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HIDHOST,
					HAL_OP_HIDHOST_VIRTUAL_UNPLUG,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t set_info(bt_bdaddr_t *bd_addr, bthh_hid_info_t hid_info)
{
	struct hal_cmd_hidhost_set_info cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));
	cmd.attr = hid_info.attr_mask;
	cmd.subclass = hid_info.sub_class;
	cmd.app_id = hid_info.app_id;
	cmd.vendor = hid_info.vendor_id;
	cmd.product = hid_info.product_id;
	cmd.country = hid_info.ctry_code;
	cmd.descr_len = hid_info.dl_len;
	memcpy(cmd.descr, hid_info.dsc_list, cmd.descr_len);

	return hal_ipc_cmd(HAL_SERVICE_ID_HIDHOST, HAL_OP_HIDHOST_SET_INFO,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t get_protocol(bt_bdaddr_t *bd_addr,
					bthh_protocol_mode_t protocol_mode)
{
	struct hal_cmd_hidhost_get_protocol cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	/* type match IPC type */
	cmd.mode = protocol_mode;

	return hal_ipc_cmd(HAL_SERVICE_ID_HIDHOST,
				HAL_OP_HIDHOST_GET_PROTOCOL,
				sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t set_protocol(bt_bdaddr_t *bd_addr,
					bthh_protocol_mode_t protocol_mode)
{
	struct hal_cmd_hidhost_set_protocol cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	/* type match IPC type */
	cmd.mode = protocol_mode;

	return hal_ipc_cmd(HAL_SERVICE_ID_HIDHOST,
				HAL_OP_HIDHOST_SET_PROTOCOL,
				sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t get_report(bt_bdaddr_t *bd_addr,
						bthh_report_type_t report_type,
						uint8_t report_id,
						int buffer_size)
{
	struct hal_cmd_hidhost_get_report cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));
	cmd.id = report_id;
	cmd.buf_size = buffer_size;

	/* type match IPC type */
	cmd.type = report_type;

	return hal_ipc_cmd(HAL_SERVICE_ID_HIDHOST, HAL_OP_HIDHOST_GET_REPORT,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t set_report(bt_bdaddr_t *bd_addr,
						bthh_report_type_t report_type,
						char *report)
{
	uint8_t buf[IPC_MTU];
	struct hal_cmd_hidhost_set_report *cmd = (void *) buf;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr || !report)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd->bdaddr, bd_addr, sizeof(cmd->bdaddr));
	cmd->len = strlen(report);
	memcpy(cmd->data, report, cmd->len);

	/* type match IPC type */
	cmd->type = report_type;

	return hal_ipc_cmd(HAL_SERVICE_ID_HIDHOST, HAL_OP_HIDHOST_SET_REPORT,
				sizeof(*cmd) + cmd->len, buf, NULL, NULL, NULL);
}

static bt_status_t send_data(bt_bdaddr_t *bd_addr, char *data)
{
	uint8_t buf[IPC_MTU];
	struct hal_cmd_hidhost_send_data *cmd = (void *) buf;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr || !data)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd->bdaddr, bd_addr, sizeof(cmd->bdaddr));
	cmd->len = strlen(data);
	memcpy(cmd->data, data, cmd->len);

	return hal_ipc_cmd(HAL_SERVICE_ID_HIDHOST, HAL_OP_HIDHOST_SEND_DATA,
			sizeof(*cmd) + cmd->len, buf, NULL, NULL, NULL);
}

static bt_status_t init(bthh_callbacks_t *callbacks)
{
	struct hal_cmd_register_module cmd;
	int ret;

	DBG("");

	if (interface_ready())
		return BT_STATUS_DONE;

	/* store reference to user callbacks */
	cbacks = callbacks;

	hal_ipc_register(HAL_SERVICE_ID_HIDHOST, ev_handlers,
				sizeof(ev_handlers)/sizeof(ev_handlers[0]));

	cmd.service_id = HAL_SERVICE_ID_HIDHOST;
	cmd.mode = HAL_MODE_DEFAULT;
	cmd.max_clients = 1;

	ret = hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_REGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	if (ret != BT_STATUS_SUCCESS) {
		cbacks = NULL;
		hal_ipc_unregister(HAL_SERVICE_ID_HIDHOST);
	}

	return ret;
}

static void cleanup(void)
{
	struct hal_cmd_unregister_module cmd;

	DBG("");

	if (!interface_ready())
		return;

	cmd.service_id = HAL_SERVICE_ID_HIDHOST;

	hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_UNREGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	hal_ipc_unregister(HAL_SERVICE_ID_HIDHOST);

	cbacks = NULL;
}

static bthh_interface_t hidhost_if = {
	.size = sizeof(hidhost_if),
	.init = init,
	.connect = hidhost_connect,
	.disconnect = disconnect,
	.virtual_unplug = virtual_unplug,
	.set_info = set_info,
	.get_protocol = get_protocol,
	.set_protocol = set_protocol,
	.get_report = get_report,
	.set_report = set_report,
	.send_data = send_data,
	.cleanup = cleanup
};

bthh_interface_t *bt_get_hidhost_interface(void)
{
	return &hidhost_if;
}
