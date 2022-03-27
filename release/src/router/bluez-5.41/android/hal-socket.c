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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "hal-ipc.h"
#include "hal-log.h"
#include "hal-msg.h"
#include "hal-utils.h"
#include "hal.h"

static bt_status_t socket_listen(btsock_type_t type, const char *service_name,
					const uint8_t *uuid, int chan,
					int *sock, int flags)
{
	struct hal_cmd_socket_listen cmd;

	if (!sock)
		return BT_STATUS_PARM_INVALID;

	DBG("uuid %s chan %d sock %p type %d service_name %s flags 0x%02x",
		btuuid2str(uuid), chan, sock, type, service_name, flags);

	memset(&cmd, 0, sizeof(cmd));

	/* type match IPC type */
	cmd.type = type;
	cmd.flags = flags;
	cmd.channel = chan;

	if (uuid)
		memcpy(cmd.uuid, uuid, sizeof(cmd.uuid));

	if (service_name)
		memcpy(cmd.name, service_name, strlen(service_name));

	return hal_ipc_cmd(HAL_SERVICE_ID_SOCKET, HAL_OP_SOCKET_LISTEN,
				sizeof(cmd), &cmd, NULL, NULL, sock);
}

static bt_status_t socket_connect(const bt_bdaddr_t *bdaddr, btsock_type_t type,
					const uint8_t *uuid, int chan,
					int *sock, int flags)
{
	struct hal_cmd_socket_connect cmd;

	if (!sock)
		return BT_STATUS_PARM_INVALID;

	DBG("bdaddr %s uuid %s chan %d sock %p type %d flags 0x%02x",
		bdaddr2str(bdaddr), btuuid2str(uuid), chan, sock, type, flags);

	memset(&cmd, 0, sizeof(cmd));

	/* type match IPC type */
	cmd.type = type;
	cmd.flags = flags;
	cmd.channel = chan;

	if (uuid)
		memcpy(cmd.uuid, uuid, sizeof(cmd.uuid));

	if (bdaddr)
		memcpy(cmd.bdaddr, bdaddr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_SOCKET, HAL_OP_SOCKET_CONNECT,
					sizeof(cmd), &cmd, NULL, NULL, sock);
}

static btsock_interface_t socket_if = {
	sizeof(socket_if),
	socket_listen,
	socket_connect
};

btsock_interface_t *bt_get_socket_interface(void)
{
	return &socket_if;
}
