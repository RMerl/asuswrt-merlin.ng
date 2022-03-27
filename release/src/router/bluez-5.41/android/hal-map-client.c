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

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "hal-log.h"
#include "hal.h"
#include "hal-msg.h"
#include "hal-ipc.h"

static const btmce_callbacks_t *cbs = NULL;

static bool interface_ready(void)
{
	return cbs != NULL;
}

/* Event Handlers */

static void remote_mas_instances_to_hal(btmce_mas_instance_t *send_instance,
				struct hal_map_client_mas_instance *instance,
				int num_instances, uint16_t len)
{
	void *buf = instance;
	char *name;
	int i;

	DBG("");

	for (i = 0; i < num_instances; i++) {
		name = (char *) instance->name;
		if (sizeof(*instance) + instance->name_len > len ||
					(instance->name_len != 0 &&
					name[instance->name_len - 1] != '\0')) {
			error("invalid remote mas instance %d, aborting", i);
			exit(EXIT_FAILURE);
		}

		send_instance[i].id = instance->id;
		send_instance[i].msg_types = instance->msg_types;
		send_instance[i].scn = instance->scn;
		send_instance[i].p_name = name;

		len -= sizeof(*instance) + instance->name_len;
		buf += sizeof(*instance) + instance->name_len;
		instance = buf;
	}

	if (!len)
		return;

	error("invalid remote mas instances (%u bytes left), aborting", len);
	exit(EXIT_FAILURE);
}

static void handle_remote_mas_instances(void *buf, uint16_t len, int fd)
{
	struct hal_ev_map_client_remote_mas_instances *ev = buf;
	btmce_mas_instance_t instances[ev->num_instances];

	DBG("");

	len -= sizeof(*ev);
	remote_mas_instances_to_hal(instances, ev->instances, ev->num_instances,
									len);

	if (cbs->remote_mas_instances_cb)
		cbs->remote_mas_instances_cb(ev->status,
						(bt_bdaddr_t *) ev->bdaddr,
						ev->num_instances, instances);
}

/*
 * handlers will be called from notification thread context,
 * index in table equals to 'opcode - HAL_MINIMUM_EVENT'
 */
static const struct hal_ipc_handler ev_handlers[] = {
	/* HAL_EV_MCE_REMOTE_MAS_INSTANCES */
	{ handle_remote_mas_instances, true,
			sizeof(struct hal_ev_map_client_remote_mas_instances) }
};

/* API */

static bt_status_t get_remote_mas_instances(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_map_client_get_instances cmd;

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	memcpy(cmd.bdaddr, bd_addr, sizeof(*bd_addr));

	return hal_ipc_cmd(HAL_SERVICE_ID_MAP_CLIENT,
				HAL_OP_MAP_CLIENT_GET_INSTANCES, sizeof(cmd),
				&cmd, NULL, NULL, NULL);
}

static bt_status_t init(btmce_callbacks_t *callbacks)
{
	struct hal_cmd_register_module cmd;
	int ret;

	DBG("");

	/*
	 * Interface ready check was removed because there is no cleanup
	 * function to unregister and clear callbacks. MAP client testers may
	 * restart bluetooth, unregister this profile and try to reuse it.
	 * This situation make service unregistered but callbacks are still
	 * set - interface is ready. On android devices there is no need to
	 * re-init MAP client profile while bluetooth is loaded.
	 */

	cbs = callbacks;

	hal_ipc_register(HAL_SERVICE_ID_MAP_CLIENT, ev_handlers,
				sizeof(ev_handlers)/sizeof(ev_handlers[0]));

	cmd.service_id = HAL_SERVICE_ID_MAP_CLIENT;
	cmd.mode = HAL_MODE_DEFAULT;
	cmd.max_clients = 1;

	ret = hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_REGISTER_MODULE,
					sizeof(cmd), &cmd, 0, NULL, NULL);

	if (ret != BT_STATUS_SUCCESS) {
		cbs = NULL;
		hal_ipc_unregister(HAL_SERVICE_ID_MAP_CLIENT);
	}

	return ret;
}

static btmce_interface_t iface = {
	.size = sizeof(iface),
	.init = init,
	.get_remote_mas_instances = get_remote_mas_instances
};

btmce_interface_t *bt_get_map_client_interface(void)
{
	return &iface;
}
