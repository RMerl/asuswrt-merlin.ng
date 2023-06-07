/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
 *
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
