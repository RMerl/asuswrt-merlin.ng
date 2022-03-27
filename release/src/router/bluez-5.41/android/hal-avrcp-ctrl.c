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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "hal-utils.h"
#include "hal-log.h"
#include "hal.h"
#include "hal-msg.h"
#include "ipc-common.h"
#include "hal-ipc.h"

static const btrc_ctrl_callbacks_t *cbs = NULL;

static bool interface_ready(void)
{
	return cbs != NULL;
}

static void handle_connection_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_avrcp_ctrl_conn_state *ev = buf;

	if (cbs->connection_state_cb)
		cbs->connection_state_cb(ev->state,
						(bt_bdaddr_t *) (ev->bdaddr));
}

static void handle_passthrough_rsp(void *buf, uint16_t len, int fd)
{
	struct hal_ev_avrcp_ctrl_passthrough_rsp *ev = buf;

	if (cbs->passthrough_rsp_cb)
		cbs->passthrough_rsp_cb(ev->id, ev->key_state);
}

/*
 * handlers will be called from notification thread context,
 * index in table equals to 'opcode - HAL_MINIMUM_EVENT'
 */
static const struct hal_ipc_handler ev_handlers[] = {
	/* HAL_EV_AVRCP_CTRL_CONN_STATE */
	{ handle_connection_state, false,
			sizeof(struct hal_ev_avrcp_ctrl_conn_state) },
	/* HAL_EV_AVRCP_CTRL_PASSTHROUGH_RSP */
	{ handle_passthrough_rsp, false,
			sizeof(struct hal_ev_avrcp_ctrl_passthrough_rsp) },
};

static bt_status_t init(btrc_ctrl_callbacks_t *callbacks)
{
	struct hal_cmd_register_module cmd;
	int ret;

	DBG("");

	if (interface_ready())
		return BT_STATUS_DONE;

	cbs = callbacks;

	hal_ipc_register(HAL_SERVICE_ID_AVRCP_CTRL, ev_handlers,
				sizeof(ev_handlers) / sizeof(ev_handlers[0]));

	cmd.service_id = HAL_SERVICE_ID_AVRCP_CTRL;
	cmd.mode = HAL_MODE_DEFAULT;
	cmd.max_clients = 1;

	ret = hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_REGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	if (ret != BT_STATUS_SUCCESS) {
		cbs = NULL;
		hal_ipc_unregister(HAL_SERVICE_ID_AVRCP_CTRL);
	}

	return ret;
}

static bt_status_t send_pass_through_cmd(bt_bdaddr_t *bd_addr, uint8_t key_code,
							uint8_t key_state)
{
	struct hal_cmd_avrcp_ctrl_send_passthrough cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));
	cmd.key_code = key_code;
	cmd.key_state = key_state;

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP_CTRL,
					HAL_OP_AVRCP_CTRL_SEND_PASSTHROUGH,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static void cleanup(void)
{
	struct hal_cmd_unregister_module cmd;

	DBG("");

	if (!interface_ready())
		return;

	cmd.service_id = HAL_SERVICE_ID_AVRCP_CTRL;

	hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_UNREGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	hal_ipc_unregister(HAL_SERVICE_ID_AVRCP_CTRL);

	cbs = NULL;
}

static btrc_ctrl_interface_t iface = {
	.size = sizeof(iface),
	.init = init,
	.send_pass_through_cmd = send_pass_through_cmd,
	.cleanup = cleanup
};

btrc_ctrl_interface_t *bt_get_avrcp_ctrl_interface(void)
{
	return &iface;
}
