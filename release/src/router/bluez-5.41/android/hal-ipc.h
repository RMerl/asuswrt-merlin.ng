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
