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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>
#include <glib.h>

#include "lib/bluetooth.h"
#include "src/log.h"
#include "hal-msg.h"
#include "ipc.h"
#include "a2dp-sink.h"

static struct ipc *hal_ipc = NULL;

static void bt_a2dp_sink_connect(const void *buf, uint16_t len)
{
	/* TODO */

	DBG("");

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_A2DP_SINK, HAL_OP_A2DP_CONNECT,
							HAL_STATUS_UNSUPPORTED);
}

static void bt_a2dp_sink_disconnect(const void *buf, uint16_t len)
{
	/* TODO */

	DBG("");

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_A2DP_SINK, HAL_OP_A2DP_DISCONNECT,
							HAL_STATUS_UNSUPPORTED);
}

static const struct ipc_handler cmd_handlers[] = {
	/* HAL_OP_A2DP_CONNECT */
	{ bt_a2dp_sink_connect, false, sizeof(struct hal_cmd_a2dp_connect) },
	/* HAL_OP_A2DP_DISCONNECT */
	{ bt_a2dp_sink_disconnect, false,
				sizeof(struct hal_cmd_a2dp_disconnect) },
};

bool bt_a2dp_sink_register(struct ipc *ipc, const bdaddr_t *addr, uint8_t mode)
{
	DBG("");

	hal_ipc = ipc;
	ipc_register(hal_ipc, HAL_SERVICE_ID_A2DP_SINK, cmd_handlers,
						G_N_ELEMENTS(cmd_handlers));

	return true;
}

void bt_a2dp_sink_unregister(void)
{
	DBG("");

	ipc_unregister(hal_ipc, HAL_SERVICE_ID_A2DP_SINK);
	hal_ipc = NULL;
}
