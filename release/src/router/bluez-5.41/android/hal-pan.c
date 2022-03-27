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

#include "hal-utils.h"
#include "hal-log.h"
#include "hal.h"
#include "hal-msg.h"
#include "hal-ipc.h"

static const btpan_callbacks_t *cbs = NULL;

static bool interface_ready(void)
{
	return cbs != NULL;
}

static void handle_conn_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_pan_conn_state *ev = buf;

	if (cbs->connection_state_cb)
		cbs->connection_state_cb(ev->state, ev->status,
					(bt_bdaddr_t *) ev->bdaddr,
					ev->local_role, ev->remote_role);
}

static void handle_ctrl_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_pan_ctrl_state *ev = buf;

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	if (cbs->control_state_cb)
		cbs->control_state_cb(ev->state, ev->local_role, ev->status,
							(char *)ev->name);
#else
	/*
	 * Callback declared in bt_pan.h is 'typedef void
	 * (*btpan_control_state_callback)(btpan_control_state_t state,
	 * bt_status_t error, int local_role, const char* ifname);
	 * But PanService.Java defined it wrong way.
	 * private void onControlStateChanged(int local_role, int state,
	 * int error, String ifname).
	 * First and third parameters are misplaced, so sending data according
	 * to PanService.Java.
	 */
	if (cbs->control_state_cb)
		cbs->control_state_cb(ev->local_role, ev->state, ev->status,
							(char *)ev->name);
#endif
}

/*
 * handlers will be called from notification thread context,
 * index in table equals to 'opcode - HAL_MINIMUM_EVENT'
 */
static const struct hal_ipc_handler ev_handlers[] = {
	/* HAL_EV_PAN_CTRL_STATE */
	{ handle_ctrl_state, false, sizeof(struct hal_ev_pan_ctrl_state) },
	/* HAL_EV_PAN_CONN_STATE */
	{ handle_conn_state, false, sizeof(struct hal_ev_pan_conn_state) },
};

static bt_status_t pan_enable(int local_role)
{
	struct hal_cmd_pan_enable cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	cmd.local_role = local_role;

	return hal_ipc_cmd(HAL_SERVICE_ID_PAN, HAL_OP_PAN_ENABLE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static int pan_get_local_role(void)
{
	struct hal_rsp_pan_get_role rsp;
	size_t len = sizeof(rsp);
	bt_status_t status;

	DBG("");

	if (!interface_ready())
		return BTPAN_ROLE_NONE;

	status = hal_ipc_cmd(HAL_SERVICE_ID_PAN, HAL_OP_PAN_GET_ROLE, 0, NULL,
							&len, &rsp, NULL);
	if (status != BT_STATUS_SUCCESS)
		return BTPAN_ROLE_NONE;

	return rsp.local_role;
}

static bt_status_t pan_connect(const bt_bdaddr_t *bd_addr, int local_role,
					int remote_role)
{
	struct hal_cmd_pan_connect cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));
	cmd.local_role = local_role;
	cmd.remote_role = remote_role;

	return hal_ipc_cmd(HAL_SERVICE_ID_PAN, HAL_OP_PAN_CONNECT,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t pan_disconnect(const bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_pan_disconnect cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_PAN, HAL_OP_PAN_DISCONNECT,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t pan_init(const btpan_callbacks_t *callbacks)
{
	struct hal_cmd_register_module cmd;
	int ret;

	DBG("");

	if (interface_ready())
		return BT_STATUS_DONE;

	cbs = callbacks;

	hal_ipc_register(HAL_SERVICE_ID_PAN, ev_handlers,
				sizeof(ev_handlers)/sizeof(ev_handlers[0]));

	cmd.service_id = HAL_SERVICE_ID_PAN;
	cmd.mode = HAL_MODE_DEFAULT;
	cmd.max_clients = 1;

	ret = hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_REGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	if (ret != BT_STATUS_SUCCESS) {
		cbs = NULL;
		hal_ipc_unregister(HAL_SERVICE_ID_PAN);
	}

	return ret;
}

static void pan_cleanup(void)
{
	struct hal_cmd_unregister_module cmd;

	DBG("");

	if (!interface_ready())
		return;

	cmd.service_id = HAL_SERVICE_ID_PAN;

	hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_UNREGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	hal_ipc_unregister(HAL_SERVICE_ID_PAN);

	cbs = NULL;
}

static btpan_interface_t pan_if = {
	.size = sizeof(pan_if),
	.init = pan_init,
	.enable = pan_enable,
	.get_local_role = pan_get_local_role,
	.connect = pan_connect,
	.disconnect = pan_disconnect,
	.cleanup = pan_cleanup
};

btpan_interface_t *bt_get_pan_interface(void)
{
	return &pan_if;
}
