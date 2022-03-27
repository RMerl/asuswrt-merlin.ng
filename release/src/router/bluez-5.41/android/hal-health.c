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
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "hal-log.h"
#include "hal.h"
#include "hal-msg.h"
#include "ipc-common.h"
#include "hal-ipc.h"

static const bthl_callbacks_t *cbacks = NULL;

static bool interface_ready(void)
{
	return cbacks != NULL;
}

static void handle_app_registration_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_health_app_reg_state *ev = buf;

	if (cbacks->app_reg_state_cb)
		cbacks->app_reg_state_cb(ev->id, ev->state);
}

static void handle_channel_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_health_channel_state *ev = buf;
	int flags;

	if (fd < 0)
		goto end;

	flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) {
		error("health: fcntl GETFL error: %s", strerror(errno));
		return;
	}

	/* Clean O_NONBLOCK fd flag as Android Java layer expects */
	if (fcntl(fd, F_SETFL, flags & ~O_NONBLOCK) < 0) {
		error("health: fcntl SETFL error: %s", strerror(errno));
		return;
	}

end:
	if (cbacks->channel_state_cb)
		cbacks->channel_state_cb(ev->app_id, (bt_bdaddr_t *) ev->bdaddr,
						ev->mdep_index, ev->channel_id,
						ev->channel_state, fd);
}

/*
 * handlers will be called from notification thread context,
 * index in table equals to 'opcode - HAL_MINIMUM_EVENT'
 */
static const struct hal_ipc_handler ev_handlers[] = {
	/* HAL_EV_HEALTH_APP_REG_STATE */
	{ handle_app_registration_state, false,
				sizeof(struct hal_ev_health_app_reg_state) },
	/* HAL_EV_HEALTH_CHANNEL_STATE */
	{ handle_channel_state, false,
				sizeof(struct hal_ev_health_channel_state) },
};

static bt_status_t register_application(bthl_reg_param_t *reg, int *app_id)
{
	uint8_t buf[IPC_MTU];
	struct hal_cmd_health_reg_app *cmd = (void *) buf;
	struct hal_rsp_health_reg_app rsp;
	size_t rsp_len = sizeof(rsp);
	bt_status_t status;
	uint16_t off, len;
	int i;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!reg || !app_id || !reg->application_name)
		return BT_STATUS_PARM_INVALID;

	*app_id = -1;
	memset(buf, 0, IPC_MTU);

	cmd->num_of_mdep = reg->number_of_mdeps;

	off = 0;
	cmd->app_name_off = off;
	len = strlen(reg->application_name) + 1;
	memcpy(cmd->data, reg->application_name, len);
	off += len;

	cmd->provider_name_off = off;
	if (reg->provider_name) {
		len = strlen(reg->provider_name) + 1;
		memcpy(cmd->data + off, reg->provider_name, len);
		off += len;
	}

	cmd->service_name_off = off;
	if (reg->srv_name) {
		len = strlen(reg->srv_name) + 1;
		memcpy(cmd->data + off, reg->srv_name, len);
		off += len;
	}

	cmd->service_descr_off = off;
	if (reg->srv_desp) {
		len = strlen(reg->srv_desp) + 1;
		memcpy(cmd->data + off, reg->srv_desp, len);
		off += len;
	}

	cmd->len = off;
	status = hal_ipc_cmd(HAL_SERVICE_ID_HEALTH, HAL_OP_HEALTH_REG_APP,
						sizeof(*cmd) + cmd->len, buf,
						&rsp_len, &rsp, NULL);
	if (status != BT_STATUS_SUCCESS)
		return status;

	for (i = 0; i < reg->number_of_mdeps; i++) {
		struct hal_cmd_health_mdep *mdep = (void *) buf;

		memset(buf, 0, IPC_MTU);
		mdep->app_id = rsp.app_id;
		mdep->role = reg->mdep_cfg[i].mdep_role;
		mdep->data_type = reg->mdep_cfg[i].data_type;
		mdep->channel_type = reg->mdep_cfg[i].channel_type;

		if (reg->mdep_cfg[i].mdep_description) {
			mdep->descr_len =
				strlen(reg->mdep_cfg[i].mdep_description) + 1;
			memcpy(mdep->descr, reg->mdep_cfg[i].mdep_description,
							mdep->descr_len);
		}

		status = hal_ipc_cmd(HAL_SERVICE_ID_HEALTH, HAL_OP_HEALTH_MDEP,
						sizeof(*mdep) + mdep->descr_len,
						buf, NULL, NULL, NULL);

		if (status != BT_STATUS_SUCCESS)
			return status;
	}

	*app_id = rsp.app_id;

	return status;
}

static bt_status_t unregister_application(int app_id)
{
	struct hal_cmd_health_unreg_app cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	cmd.app_id = app_id;

	return hal_ipc_cmd(HAL_SERVICE_ID_HEALTH, HAL_OP_HEALTH_UNREG_APP,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t connect_channel(int app_id, bt_bdaddr_t *bd_addr,
					int mdep_cfg_index, int *channel_id)
{
	struct hal_cmd_health_connect_channel cmd;
	struct hal_rsp_health_connect_channel rsp;
	size_t len = sizeof(rsp);
	bt_status_t status;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr || !channel_id)
		return BT_STATUS_PARM_INVALID;

	*channel_id = -1;
	cmd.app_id = app_id;
	cmd.mdep_index = mdep_cfg_index;
	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	status = hal_ipc_cmd(HAL_SERVICE_ID_HEALTH,
					HAL_OP_HEALTH_CONNECT_CHANNEL,
					sizeof(cmd), &cmd, &len, &rsp, NULL);

	if (status == BT_STATUS_SUCCESS)
		*channel_id = rsp.channel_id;

	return status;
}

static bt_status_t destroy_channel(int channel_id)
{
	struct hal_cmd_health_destroy_channel cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	cmd.channel_id = channel_id;

	return hal_ipc_cmd(HAL_SERVICE_ID_HEALTH, HAL_OP_HEALTH_DESTROY_CHANNEL,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t init(bthl_callbacks_t *callbacks)
{
	struct hal_cmd_register_module cmd;
	int ret;

	DBG("");

	if (interface_ready())
		return BT_STATUS_DONE;

	/* store reference to user callbacks */
	cbacks = callbacks;

	hal_ipc_register(HAL_SERVICE_ID_HEALTH, ev_handlers,
				sizeof(ev_handlers)/sizeof(ev_handlers[0]));

	cmd.service_id = HAL_SERVICE_ID_HEALTH;
	cmd.mode = HAL_MODE_DEFAULT;
	cmd.max_clients = 1;

	ret = hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_REGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	if (ret != BT_STATUS_SUCCESS) {
		cbacks = NULL;
		hal_ipc_unregister(HAL_SERVICE_ID_HEALTH);
	}

	return ret;
}

static void cleanup(void)
{
	struct hal_cmd_unregister_module cmd;

	DBG("");

	if (!interface_ready())
		return;

	cmd.service_id = HAL_SERVICE_ID_HEALTH;

	hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_UNREGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	hal_ipc_unregister(HAL_SERVICE_ID_HEALTH);

	cbacks = NULL;
}

static bthl_interface_t health_if = {
	.size = sizeof(health_if),
	.init = init,
	.register_application = register_application,
	.unregister_application = unregister_application,
	.connect_channel = connect_channel,
	.destroy_channel = destroy_channel,
	.cleanup = cleanup
};

bthl_interface_t *bt_get_health_interface(void)
{
	return &health_if;
}
