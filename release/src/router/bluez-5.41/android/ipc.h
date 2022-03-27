/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
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

struct ipc_handler {
	void (*handler) (const void *buf, uint16_t len);
	bool var_len;
	size_t data_len;
};

struct ipc;

typedef void (*ipc_disconnect_cb) (void *data);

struct ipc *ipc_init(const char *path, size_t size, int max_service_id,
					bool notifications,
					ipc_disconnect_cb cb, void *cb_data);
void ipc_cleanup(struct ipc *ipc);

void ipc_send_rsp(struct ipc *ipc, uint8_t service_id, uint8_t opcode,
								uint8_t status);
void ipc_send_rsp_full(struct ipc *ipc, uint8_t service_id, uint8_t opcode,
					uint16_t len, void *param, int fd);
void ipc_send_notif(struct ipc *ipc, uint8_t service_id, uint8_t opcode,
						uint16_t len, void *param);
void ipc_send_notif_with_fd(struct ipc *ipc, uint8_t service_id, uint8_t opcode,
					uint16_t len, void *param, int fd);

void ipc_register(struct ipc *ipc, uint8_t service,
			const struct ipc_handler *handlers, uint8_t size);
void ipc_unregister(struct ipc *ipc, uint8_t service);
