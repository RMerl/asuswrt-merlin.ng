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

#include "hal-log.h"
#include "hal.h"
#include "hal-msg.h"
#include "hal-ipc.h"

static const btav_callbacks_t *cbs = NULL;

static bool interface_ready(void)
{
	return cbs != NULL;
}

static void handle_conn_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_a2dp_conn_state *ev = buf;

	if (cbs->connection_state_cb)
		cbs->connection_state_cb(ev->state,
						(bt_bdaddr_t *) (ev->bdaddr));
}

static void handle_audio_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_a2dp_audio_state *ev = buf;

	if (cbs->audio_state_cb)
		cbs->audio_state_cb(ev->state, (bt_bdaddr_t *)(ev->bdaddr));
}

static void handle_audio_config(void *buf, uint16_t len, int fd)
{
	struct hal_ev_a2dp_audio_config *ev = buf;

	if (cbs->audio_config_cb)
		cbs->audio_config_cb((bt_bdaddr_t *)(ev->bdaddr),
					ev->sample_rate, ev->channel_count);
}

/*
 * handlers will be called from notification thread context,
 * index in table equals to 'opcode - HAL_MINIMUM_EVENT'
 */
static const struct hal_ipc_handler ev_handlers[] = {
	/* HAL_EV_A2DP_CONN_STATE */
	{ handle_conn_state, false, sizeof(struct hal_ev_a2dp_conn_state) },
	/* HAL_EV_A2DP_AUDIO_STATE */
	{ handle_audio_state, false, sizeof(struct hal_ev_a2dp_audio_state) },
	/* HAL_EV_A2DP_AUDIO_CONFIG */
	{ handle_audio_config, false, sizeof(struct hal_ev_a2dp_audio_config) },
};

static bt_status_t a2dp_connect(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_a2dp_connect cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_A2DP_SINK, HAL_OP_A2DP_CONNECT,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t disconnect(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_a2dp_disconnect cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_A2DP_SINK, HAL_OP_A2DP_DISCONNECT,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t init(btav_callbacks_t *callbacks)
{
	struct hal_cmd_register_module cmd;
	int ret;

	DBG("");

	if (interface_ready())
		return BT_STATUS_DONE;

	cbs = callbacks;

	hal_ipc_register(HAL_SERVICE_ID_A2DP_SINK, ev_handlers,
				sizeof(ev_handlers)/sizeof(ev_handlers[0]));

	cmd.service_id = HAL_SERVICE_ID_A2DP_SINK;
	cmd.mode = HAL_MODE_DEFAULT;
	cmd.max_clients = 1;

	ret = hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_REGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	if (ret != BT_STATUS_SUCCESS) {
		cbs = NULL;
		hal_ipc_unregister(HAL_SERVICE_ID_A2DP_SINK);
	}

	return ret;
}

static void cleanup(void)
{
	struct hal_cmd_unregister_module cmd;

	DBG("");

	if (!interface_ready())
		return;

	cmd.service_id = HAL_SERVICE_ID_A2DP_SINK;

	hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_UNREGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	hal_ipc_unregister(HAL_SERVICE_ID_A2DP_SINK);

	cbs = NULL;
}

static btav_interface_t iface = {
	.size = sizeof(iface),
	.init = init,
	.connect = a2dp_connect,
	.disconnect = disconnect,
	.cleanup = cleanup
};

btav_interface_t *bt_get_a2dp_sink_interface(void)
{
	return &iface;
}
