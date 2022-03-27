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

#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>
#include <poll.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#include <cutils/properties.h>

#include "hal.h"
#include "hal-msg.h"
#include "hal-log.h"
#include "ipc-common.h"
#include "hal-ipc.h"

#define CONNECT_TIMEOUT (10 * 1000)

static int listen_sk = -1;
static int cmd_sk = -1;
static int notif_sk = -1;

static pthread_mutex_t cmd_sk_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_t notif_th = 0;

struct service_handler {
	const struct hal_ipc_handler *handler;
	uint8_t size;
};

static struct service_handler services[HAL_SERVICE_ID_MAX + 1];

void hal_ipc_register(uint8_t service, const struct hal_ipc_handler *handlers,
								uint8_t size)
{
	services[service].handler = handlers;
	services[service].size = size;
}

void hal_ipc_unregister(uint8_t service)
{
	services[service].handler = NULL;
	services[service].size = 0;
}

static bool handle_msg(void *buf, ssize_t len, int fd)
{
	struct ipc_hdr *msg = buf;
	const struct hal_ipc_handler *handler;
	uint8_t opcode;

	if (len < (ssize_t) sizeof(*msg)) {
		error("IPC: message too small (%zd bytes)", len);
		return false;
	}

	if (len != (ssize_t) (sizeof(*msg) + msg->len)) {
		error("IPC: message malformed (%zd bytes)", len);
		return false;
	}

	/* if service is valid */
	if (msg->service_id > HAL_SERVICE_ID_MAX) {
		error("IPC: unknown service (0x%x)", msg->service_id);
		return false;
	}

	/* if service is registered */
	if (!services[msg->service_id].handler) {
		error("IPC: unregistered service (0x%x)", msg->service_id);
		return false;
	}

	/* if opcode fit valid range */
	if (msg->opcode < HAL_MINIMUM_EVENT) {
		error("IPC: invalid opcode for service 0x%x (0x%x)",
						msg->service_id, msg->opcode);
		return false;
	}

	/*
	 * opcode is used as table offset and must be adjusted as events start
	 * with HAL_MINIMUM_EVENT offset
	 */
	opcode = msg->opcode - HAL_MINIMUM_EVENT;

	/* if opcode is valid */
	if (opcode >= services[msg->service_id].size) {
		error("IPC: invalid opcode for service 0x%x (0x%x)",
						msg->service_id, msg->opcode);
		return false;
	}

	handler = &services[msg->service_id].handler[opcode];

	/* if payload size is valid */
	if ((handler->var_len && handler->data_len > msg->len) ||
			(!handler->var_len && handler->data_len != msg->len)) {
		error("IPC: message size invalid for service 0x%x opcode 0x%x "
				"(%u bytes)",
				msg->service_id, msg->opcode, msg->len);
		return false;
	}

	handler->handler(msg->payload, msg->len, fd);

	return true;
}

static void *notification_handler(void *data)
{
	struct msghdr msg;
	struct iovec iv;
	struct cmsghdr *cmsg;
	char cmsgbuf[CMSG_SPACE(sizeof(int))];
	char buf[IPC_MTU];
	ssize_t ret;
	int fd;

	bt_thread_associate();

	while (true) {
		memset(&msg, 0, sizeof(msg));
		memset(buf, 0, sizeof(buf));
		memset(cmsgbuf, 0, sizeof(cmsgbuf));

		iv.iov_base = buf;
		iv.iov_len = sizeof(buf);

		msg.msg_iov = &iv;
		msg.msg_iovlen = 1;

		msg.msg_control = cmsgbuf;
		msg.msg_controllen = sizeof(cmsgbuf);

		ret = recvmsg(notif_sk, &msg, 0);
		if (ret < 0) {
			error("Receiving notifications failed: %s",
							strerror(errno));
			goto failed;
		}

		/* socket was shutdown */
		if (ret == 0) {
			pthread_mutex_lock(&cmd_sk_mutex);
			if (cmd_sk == -1) {
				pthread_mutex_unlock(&cmd_sk_mutex);
				break;
			}
			pthread_mutex_unlock(&cmd_sk_mutex);

			error("Notification socket closed");
			goto failed;
		}

		fd = -1;

		/* Receive auxiliary data in msg */
		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg;
					cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_level == SOL_SOCKET
					&& cmsg->cmsg_type == SCM_RIGHTS) {
				memcpy(&fd, CMSG_DATA(cmsg), sizeof(int));
				break;
			}
		}

		if (!handle_msg(buf, ret, fd))
			goto failed;
	}

	close(notif_sk);
	notif_sk = -1;

	bt_thread_disassociate();

	DBG("exit");

	return NULL;

failed:
	exit(EXIT_FAILURE);
}

static int accept_connection(int sk)
{
	int err;
	struct pollfd pfd;
	int new_sk;

	memset(&pfd, 0 , sizeof(pfd));
	pfd.fd = sk;
	pfd.events = POLLIN;

	err = poll(&pfd, 1, CONNECT_TIMEOUT);
	if (err < 0) {
		err = errno;
		error("Failed to poll: %d (%s)", err, strerror(err));
		return -1;
	}

	if (err == 0) {
		error("bluetoothd connect timeout");
		return -1;
	}

	new_sk = accept(sk, NULL, NULL);
	if (new_sk < 0) {
		err = errno;
		error("Failed to accept socket: %d (%s)", err, strerror(err));
		return -1;
	}

	return new_sk;
}

bool hal_ipc_accept(void)
{
	int err;

	cmd_sk = accept_connection(listen_sk);
	if (cmd_sk < 0)
		return false;

	notif_sk = accept_connection(listen_sk);
	if (notif_sk < 0) {
		close(cmd_sk);
		cmd_sk = -1;
		return false;
	}

	err = pthread_create(&notif_th, NULL, notification_handler, NULL);
	if (err) {
		notif_th = 0;
		error("Failed to start notification thread: %d (%s)", err,
							strerror(err));
		close(cmd_sk);
		cmd_sk = -1;
		close(notif_sk);
		notif_sk = -1;
		return false;
	}

	info("IPC connected");

	return true;
}

bool hal_ipc_init(const char *path, size_t size)
{
	struct sockaddr_un addr;
	int sk;
	int err;

	sk = socket(AF_LOCAL, SOCK_SEQPACKET, 0);
	if (sk < 0) {
		err = errno;
		error("Failed to create socket: %d (%s)", err,
							strerror(err));
		return false;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	memcpy(addr.sun_path, path, size);

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		err = errno;
		error("Failed to bind socket: %d (%s)", err, strerror(err));
		close(sk);
		return false;
	}

	if (listen(sk, 2) < 0) {
		err = errno;
		error("Failed to listen on socket: %d (%s)", err,
								strerror(err));
		close(sk);
		return false;
	}

	listen_sk = sk;

	return true;
}

void hal_ipc_cleanup(void)
{
	close(listen_sk);
	listen_sk = -1;

	pthread_mutex_lock(&cmd_sk_mutex);
	if (cmd_sk >= 0) {
		close(cmd_sk);
		cmd_sk = -1;
	}
	pthread_mutex_unlock(&cmd_sk_mutex);

	if (notif_sk < 0)
		return;

	shutdown(notif_sk, SHUT_RD);

	pthread_join(notif_th, NULL);
	notif_th = 0;
}

int hal_ipc_cmd(uint8_t service_id, uint8_t opcode, uint16_t len, void *param,
					size_t *rsp_len, void *rsp, int *fd)
{
	ssize_t ret;
	struct msghdr msg;
	struct iovec iv[2];
	struct ipc_hdr cmd;
	char cmsgbuf[CMSG_SPACE(sizeof(int))];
	struct ipc_status s;
	size_t s_len = sizeof(s);

	if (cmd_sk < 0) {
		error("Invalid cmd socket passed to hal_ipc_cmd");
		goto failed;
	}

	if (!rsp || !rsp_len) {
		memset(&s, 0, s_len);
		rsp_len = &s_len;
		rsp = &s;
	}

	memset(&msg, 0, sizeof(msg));
	memset(&cmd, 0, sizeof(cmd));

	cmd.service_id = service_id;
	cmd.opcode = opcode;
	cmd.len = len;

	iv[0].iov_base = &cmd;
	iv[0].iov_len = sizeof(cmd);

	iv[1].iov_base = param;
	iv[1].iov_len = len;

	msg.msg_iov = iv;
	msg.msg_iovlen = 2;

	pthread_mutex_lock(&cmd_sk_mutex);

	ret = sendmsg(cmd_sk, &msg, 0);
	if (ret < 0) {
		error("Sending command failed:%s", strerror(errno));
		pthread_mutex_unlock(&cmd_sk_mutex);
		goto failed;
	}

	/* socket was shutdown */
	if (ret == 0) {
		error("Command socket closed");
		pthread_mutex_unlock(&cmd_sk_mutex);
		goto failed;
	}

	memset(&msg, 0, sizeof(msg));
	memset(&cmd, 0, sizeof(cmd));

	iv[0].iov_base = &cmd;
	iv[0].iov_len = sizeof(cmd);

	iv[1].iov_base = rsp;
	iv[1].iov_len = *rsp_len;

	msg.msg_iov = iv;
	msg.msg_iovlen = 2;

	if (fd) {
		memset(cmsgbuf, 0, sizeof(cmsgbuf));
		msg.msg_control = cmsgbuf;
		msg.msg_controllen = sizeof(cmsgbuf);
	}

	ret = recvmsg(cmd_sk, &msg, 0);

	pthread_mutex_unlock(&cmd_sk_mutex);

	if (ret < 0) {
		error("Receiving command response failed: %s", strerror(errno));
		goto failed;
	}


	if (ret < (ssize_t) sizeof(cmd)) {
		error("Too small response received(%zd bytes)", ret);
		goto failed;
	}

	if (cmd.service_id != service_id) {
		error("Invalid service id (0x%x vs 0x%x)",
						cmd.service_id, service_id);
		goto failed;
	}

	if (ret != (ssize_t) (sizeof(cmd) + cmd.len)) {
		error("Malformed response received(%zd bytes)", ret);
		goto failed;
	}

	if (cmd.opcode != opcode && cmd.opcode != HAL_OP_STATUS) {
		error("Invalid opcode received (0x%x vs 0x%x)",
						cmd.opcode, opcode);
		goto failed;
	}

	if (cmd.opcode == HAL_OP_STATUS) {
		struct ipc_status *s = rsp;

		if (sizeof(*s) != cmd.len) {
			error("Invalid status length");
			goto failed;
		}

		if (s->code == HAL_STATUS_SUCCESS) {
			error("Invalid success status response");
			goto failed;
		}

		return s->code;
	}

	/* Receive auxiliary data in msg */
	if (fd) {
		struct cmsghdr *cmsg;

		*fd = -1;

		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg;
					cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_level == SOL_SOCKET
					&& cmsg->cmsg_type == SCM_RIGHTS) {
				memcpy(fd, CMSG_DATA(cmsg), sizeof(int));
				break;
			}
		}
	}

	*rsp_len = cmd.len;

	return BT_STATUS_SUCCESS;

failed:
	exit(EXIT_FAILURE);
}
