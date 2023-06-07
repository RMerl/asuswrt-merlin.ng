/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (C) 2013 Intel Corporation
 *
 */

struct hal_ipc_handler {
	void (*handler) (void *buf, uint16_t len, int fd);
	bool var_len;
	size_t data_len;
};

bool hal_ipc_init(const char *path, size_t size);
bool hal_ipc_accept(void);
void hal_ipc_cleanup(void);

int hal_ipc_cmd(uint8_t service_id, uint8_t opcode, uint16_t len, void *param,
					size_t *rsp_len, void *rsp, int *fd);

void hal_ipc_register(uint8_t service, const struct hal_ipc_handler *handlers,
								uint8_t size);
void hal_ipc_unregister(uint8_t service);
