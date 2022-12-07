/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
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

#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#include "util.h"
#include "io.h"
#include "idle.h"
#include "queue.h"
#include "hashmap.h"
#include "dbus.h"
#include "private.h"
#include "dbus-private.h"

#define DEFAULT_SYSTEM_BUS_ADDRESS "unix:path=/var/run/dbus/system_bus_socket"

#define DBUS_SERVICE_DBUS	"org.freedesktop.DBus"

#define DBUS_PATH_DBUS		"/org/freedesktop/DBus"

#define DBUS_MAXIMUM_MATCH_RULE_LENGTH	1024

enum auth_state {
	WAITING_FOR_OK,
	WAITING_FOR_AGREE_UNIX_FD,
	SETUP_DONE
};

struct l_dbus_ops {
	char version;
	bool (*send_message)(struct l_dbus *bus,
				struct l_dbus_message *message);
	struct l_dbus_message *(*recv_message)(struct l_dbus *bus);
	void (*free)(struct l_dbus *bus);
	struct _dbus_name_ops name_ops;
	struct _dbus_filter_ops filter_ops;
	uint32_t (*name_acquire)(struct l_dbus *dbus, const char *name,
				bool allow_replacement, bool replace_existing,
				bool queue, l_dbus_name_acquire_func_t callback,
				void *user_data);
};

struct l_dbus {
	struct l_io *io;
	char *guid;
	bool negotiate_unix_fd;
	bool support_unix_fd;
	bool is_ready;
	char *unique_name;
	unsigned int next_id;
	uint32_t next_serial;
	struct l_queue *message_queue;
	struct l_hashmap *message_list;
	struct l_hashmap *signal_list;
	l_dbus_ready_func_t ready_handler;
	l_dbus_destroy_func_t ready_destroy;
	void *ready_data;
	l_dbus_disconnect_func_t disconnect_handler;
	l_dbus_destroy_func_t disconnect_destroy;
	void *disconnect_data;
	l_dbus_debug_func_t debug_handler;
	l_dbus_destroy_func_t debug_destroy;
	void *debug_data;
	struct _dbus_object_tree *tree;
	struct _dbus_name_cache *name_cache;
	struct _dbus_filter *filter;
	bool name_notify_enabled;

	const struct l_dbus_ops *driver;
};

struct l_dbus_classic {
	struct l_dbus super;
	void *auth_command;
	enum auth_state auth_state;
	struct l_hashmap *match_strings;
	int *fd_buf;
	unsigned int num_fds;
};

struct message_callback {
	uint32_t serial;
	struct l_dbus_message *message;
	l_dbus_message_func_t callback;
	l_dbus_destroy_func_t destroy;
	void *user_data;
};

struct signal_callback {
	unsigned int id;
	l_dbus_message_func_t callback;
	l_dbus_destroy_func_t destroy;
	void *user_data;
};

static void message_queue_destroy(void *data)
{
	struct message_callback *callback = data;

	l_dbus_message_unref(callback->message);

	if (callback->destroy)
		callback->destroy(callback->user_data);

	l_free(callback);
}

static void message_list_destroy(void *value)
{
	message_queue_destroy(value);
}

static void signal_list_destroy(void *value)
{
	struct signal_callback *callback = value;

	if (callback->destroy)
		callback->destroy(callback->user_data);

	l_free(callback);
}

static bool message_write_handler(struct l_io *io, void *user_data)
{
	struct l_dbus *dbus = user_data;
	struct l_dbus_message *message;
	struct message_callback *callback;
	const void *header, *body;
	size_t header_size, body_size;

	callback = l_queue_pop_head(dbus->message_queue);
	if (!callback)
		return false;

	message = callback->message;
	if (_dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_METHOD_CALL &&
			callback->callback == NULL)
		l_dbus_message_set_no_reply(message, true);

	_dbus_message_set_serial(message, callback->serial);

	if (!dbus->driver->send_message(dbus, message)) {
		message_queue_destroy(callback);
		return false;
	}

	header = _dbus_message_get_header(message, &header_size);
	body = _dbus_message_get_body(message, &body_size);
	l_util_hexdump_two(false, header, header_size, body, body_size,
				dbus->debug_handler, dbus->debug_data);

	if (callback->callback == NULL) {
		message_queue_destroy(callback);
		goto done;
	}

	l_hashmap_insert(dbus->message_list,
				L_UINT_TO_PTR(callback->serial), callback);

done:
	if (l_queue_isempty(dbus->message_queue))
		return false;

	/* Only continue sending messges if the connection is ready */
	return dbus->is_ready;
}

static void handle_method_return(struct l_dbus *dbus,
					struct l_dbus_message *message)
{
	struct message_callback *callback;
	uint32_t reply_serial;

	reply_serial = _dbus_message_get_reply_serial(message);
	if (reply_serial == 0)
		return;

	callback = l_hashmap_remove(dbus->message_list,
					L_UINT_TO_PTR(reply_serial));
	if (!callback)
		return;

	if (callback->callback)
		callback->callback(message, callback->user_data);

	message_queue_destroy(callback);
}

static void handle_error(struct l_dbus *dbus, struct l_dbus_message *message)
{
	struct message_callback *callback;
	uint32_t reply_serial;

	reply_serial = _dbus_message_get_reply_serial(message);
	if (reply_serial == 0)
		return;

	callback = l_hashmap_remove(dbus->message_list,
					L_UINT_TO_PTR(reply_serial));
	if (!callback)
		return;

	if (callback->callback)
		callback->callback(message, callback->user_data);

	message_queue_destroy(callback);
}

static void process_signal(const void *key, void *value, void *user_data)
{
	struct signal_callback *callback = value;
	struct l_dbus_message *message = user_data;

	if (callback->callback)
		callback->callback(message, callback->user_data);
}

static void handle_signal(struct l_dbus *dbus, struct l_dbus_message *message)
{
	l_hashmap_foreach(dbus->signal_list, process_signal, message);
}

static bool message_read_handler(struct l_io *io, void *user_data)
{
	struct l_dbus *dbus = user_data;
	struct l_dbus_message *message;
	const void *header, *body;
	size_t header_size, body_size;
	enum dbus_message_type msgtype;

	message = dbus->driver->recv_message(dbus);
	if (!message)
		return true;

	header = _dbus_message_get_header(message, &header_size);
	body = _dbus_message_get_body(message, &body_size);
	l_util_hexdump_two(true, header, header_size, body, body_size,
				dbus->debug_handler, dbus->debug_data);

	msgtype = _dbus_message_get_type(message);

	switch (msgtype) {
	case DBUS_MESSAGE_TYPE_METHOD_RETURN:
		handle_method_return(dbus, message);
		break;
	case DBUS_MESSAGE_TYPE_ERROR:
		handle_error(dbus, message);
		break;
	case DBUS_MESSAGE_TYPE_SIGNAL:
		handle_signal(dbus, message);
		break;
	case DBUS_MESSAGE_TYPE_METHOD_CALL:
		if (!_dbus_object_tree_dispatch(dbus->tree, dbus, message)) {
			struct l_dbus_message *error;

			error = l_dbus_message_new_error(message,
					"org.freedesktop.DBus.Error.NotFound",
					"No matching method found");
			l_dbus_send(dbus, error);
		}

		break;
	}

	l_dbus_message_unref(message);

	return true;
}

static uint32_t send_message(struct l_dbus *dbus, bool priority,
				struct l_dbus_message *message,
				l_dbus_message_func_t function,
				void *user_data, l_dbus_destroy_func_t destroy)
{
	struct message_callback *callback;
	enum dbus_message_type type;
	const char *path;

	type = _dbus_message_get_type(message);

	if ((type == DBUS_MESSAGE_TYPE_METHOD_RETURN ||
				type == DBUS_MESSAGE_TYPE_ERROR) &&
			_dbus_message_get_reply_serial(message) == 0) {
		l_dbus_message_unref(message);
		return 0;
	}

	/* Default empty signature for method return messages */
	if (type == DBUS_MESSAGE_TYPE_METHOD_RETURN &&
			!l_dbus_message_get_signature(message))
		l_dbus_message_set_arguments(message, "");

	callback = l_new(struct message_callback, 1);

	callback->serial = dbus->next_serial++;
	callback->message = message;
	callback->callback = function;
	callback->destroy = destroy;
	callback->user_data = user_data;

	if (priority) {
		l_queue_push_head(dbus->message_queue, callback);

		l_io_set_write_handler(dbus->io, message_write_handler,
							dbus, NULL);

		return callback->serial;
	}

	path = l_dbus_message_get_path(message);
	if (path)
		_dbus_object_tree_signals_flush(dbus, path);

	l_queue_push_tail(dbus->message_queue, callback);

	if (dbus->is_ready)
		l_io_set_write_handler(dbus->io, message_write_handler,
							dbus, NULL);

	return callback->serial;
}

static void bus_ready(struct l_dbus *dbus)
{
	dbus->is_ready = true;

	if (dbus->ready_handler)
		dbus->ready_handler(dbus->ready_data);

	l_io_set_read_handler(dbus->io, message_read_handler, dbus, NULL);

	/* Check for messages added before the connection was ready */
	if (l_queue_isempty(dbus->message_queue))
		return;

	l_io_set_write_handler(dbus->io, message_write_handler, dbus, NULL);
}

static void hello_callback(struct l_dbus_message *message, void *user_data)
{
	struct l_dbus *dbus = user_data;
	const char *signature;
	const char *unique_name;

	signature = l_dbus_message_get_signature(message);
	if (!signature || strcmp(signature, "s")) {
		close(l_io_get_fd(dbus->io));
		return;
	}

	if (!l_dbus_message_get_arguments(message, "s", &unique_name)) {
		close(l_io_get_fd(dbus->io));
		return;
	}

	dbus->unique_name = l_strdup(unique_name);

	bus_ready(dbus);
}

static bool auth_write_handler(struct l_io *io, void *user_data)
{
	struct l_dbus_classic *classic = user_data;
	struct l_dbus *dbus = &classic->super;
	ssize_t written, len;
	int fd;

	fd = l_io_get_fd(io);

	if (!classic->auth_command)
		return false;

	len = strlen(classic->auth_command);
	if (!len)
		return false;

	written = L_TFR(send(fd, classic->auth_command, len, 0));
	if (written < 0)
		return false;

	l_util_hexdump(false, classic->auth_command, written,
					dbus->debug_handler, dbus->debug_data);

	if (written < len) {
		memmove(classic->auth_command, classic->auth_command + written,
				len + 1 - written);
		return true;
	}

	l_free(classic->auth_command);
	classic->auth_command = NULL;

	if (classic->auth_state == SETUP_DONE) {
		struct l_dbus_message *message;

		l_io_set_read_handler(dbus->io, message_read_handler,
							dbus, NULL);

		message = l_dbus_message_new_method_call(dbus,
							DBUS_SERVICE_DBUS,
							DBUS_PATH_DBUS,
							L_DBUS_INTERFACE_DBUS,
							"Hello");
		l_dbus_message_set_arguments(message, "");

		send_message(dbus, true, message, hello_callback, dbus, NULL);

		return true;
	}

	return false;
}

static bool auth_read_handler(struct l_io *io, void *user_data)
{
	struct l_dbus_classic *classic = user_data;
	struct l_dbus *dbus = &classic->super;
	char buffer[64];
	char *ptr, *end;
	ssize_t offset, len;
	int fd;

	fd = l_io_get_fd(io);

	ptr = buffer;
	offset = 0;

	while (1) {
		len = L_TFR(recv(fd, ptr + offset,
						sizeof(buffer) - offset,
						MSG_DONTWAIT));
		if (len < 0) {
			if (errno != EAGAIN)
				return false;

			break;
		}

		offset += len;
	}

	ptr = buffer;
	len = offset;

	if (!ptr || len < 3)
		return true;

	end = strstr(ptr, "\r\n");
	if (!end)
		return true;

	if (end - ptr + 2 != len)
		return true;

	l_util_hexdump(true, ptr, len, dbus->debug_handler, dbus->debug_data);

	*end = '\0';

	switch (classic->auth_state) {
	case WAITING_FOR_OK:
		if (!strncmp(ptr, "OK ", 3)) {
			enum auth_state state;
			const char *command;

			if (dbus->negotiate_unix_fd) {
				command = "NEGOTIATE_UNIX_FD\r\n";
				state = WAITING_FOR_AGREE_UNIX_FD;
			} else {
				command = "BEGIN\r\n";
				state = SETUP_DONE;
			}

			l_free(dbus->guid);
			dbus->guid = l_strdup(ptr + 3);

			classic->auth_command = l_strdup(command);
			classic->auth_state = state;
			break;
		} else if (!strncmp(ptr, "REJECTED ", 9)) {
			static const char *command = "AUTH ANONYMOUS\r\n";

			dbus->negotiate_unix_fd = true;

			classic->auth_command = l_strdup(command);
			classic->auth_state = WAITING_FOR_OK;
		}
		break;

	case WAITING_FOR_AGREE_UNIX_FD:
		if (!strncmp(ptr, "AGREE_UNIX_FD", 13)) {
			static const char *command = "BEGIN\r\n";

			dbus->support_unix_fd = true;

			classic->auth_command = l_strdup(command);
			classic->auth_state = SETUP_DONE;
			break;
		} else if (!strncmp(ptr, "ERROR", 5)) {
			static const char *command = "BEGIN\r\n";

			dbus->support_unix_fd = false;

			classic->auth_command = l_strdup(command);
			classic->auth_state = SETUP_DONE;
			break;
		}
		break;

	case SETUP_DONE:
		break;
	}

	l_io_set_write_handler(io, auth_write_handler, dbus, NULL);

	return true;
}

static void disconnect_handler(struct l_io *io, void *user_data)
{
	struct l_dbus *dbus = user_data;

	dbus->is_ready = false;

	l_util_debug(dbus->debug_handler, dbus->debug_data, "disconnect");

	if (dbus->disconnect_handler)
		dbus->disconnect_handler(dbus->disconnect_data);
}

static void dbus_init(struct l_dbus *dbus, int fd)
{
	dbus->io = l_io_new(fd);
	l_io_set_close_on_destroy(dbus->io, true);
	l_io_set_disconnect_handler(dbus->io, disconnect_handler, dbus, NULL);

	dbus->is_ready = false;
	dbus->next_id = 1;
	dbus->next_serial = 1;

	dbus->message_queue = l_queue_new();
	dbus->message_list = l_hashmap_new();
	dbus->signal_list = l_hashmap_new();

	dbus->tree = _dbus_object_tree_new();
}

static void classic_free(struct l_dbus *dbus)
{
	struct l_dbus_classic *classic =
		l_container_of(dbus, struct l_dbus_classic, super);
	unsigned int i;

	for (i = 0; i < classic->num_fds; i++)
		close(classic->fd_buf[i]);
	l_free(classic->fd_buf);

	l_free(classic->auth_command);
	l_hashmap_destroy(classic->match_strings, l_free);
	l_free(classic);
}

static bool classic_send_message(struct l_dbus *dbus,
					struct l_dbus_message *message)
{
	int fd = l_io_get_fd(dbus->io);
	struct msghdr msg;
	struct iovec iov[2], *iovpos;
	ssize_t r;
	int *fds = NULL;
	uint32_t num_fds = 0;
	struct cmsghdr *cmsg;
	int iovlen;

	iov[0].iov_base = _dbus_message_get_header(message, &iov[0].iov_len);
	iov[1].iov_base = _dbus_message_get_body(message, &iov[1].iov_len);

	if (dbus->support_unix_fd)
		fds = _dbus_message_get_fds(message, &num_fds);

	iovpos = iov;
	iovlen = 2;

	while (1) {
		memset(&msg, 0, sizeof(msg));
		msg.msg_iov = iovpos;
		msg.msg_iovlen = iovlen;

		if (num_fds) {
			msg.msg_control =
				alloca(CMSG_SPACE(num_fds * sizeof(int)));
			msg.msg_controllen = CMSG_LEN(num_fds * sizeof(int));

			cmsg = CMSG_FIRSTHDR(&msg);
			cmsg->cmsg_len = msg.msg_controllen;
			cmsg->cmsg_level = SOL_SOCKET;
			cmsg->cmsg_type = SCM_RIGHTS;
			memcpy(CMSG_DATA(cmsg), fds, num_fds * sizeof(int));
		}

		r = L_TFR(sendmsg(fd, &msg, 0));
		if (r < 0)
			return false;

		while ((size_t) r >= iovpos->iov_len) {
			r -= iovpos->iov_len;
			iovpos++;
			iovlen--;

			if (!iovlen)
				break;
		}

		if (!iovlen)
			break;

		iovpos->iov_base += r;
		iovpos->iov_len -= r;

		/* The FDs have been transmitted, don't retransmit */
		num_fds = 0;
	}

	return true;
}

static struct l_dbus_message *classic_recv_message(struct l_dbus *dbus)
{
	struct l_dbus_classic *classic =
		l_container_of(dbus, struct l_dbus_classic, super);
	int fd = l_io_get_fd(dbus->io);
	struct dbus_header hdr;
	struct msghdr msg;
	struct iovec iov[2], *iovpos;
	struct cmsghdr *cmsg;
	ssize_t len, r;
	void *header, *body;
	size_t header_size, body_size;
	union {
		uint8_t bytes[CMSG_SPACE(16 * sizeof(int))];
		struct cmsghdr align;
	} fd_buf;
	int *fds = NULL;
	uint32_t num_fds = 0;
	int iovlen;
	struct l_dbus_message *message;
	unsigned int i;

	len = recv(fd, &hdr, DBUS_HEADER_SIZE, MSG_PEEK | MSG_DONTWAIT);
	if (len != DBUS_HEADER_SIZE)
		return NULL;

	header_size = align_len(DBUS_HEADER_SIZE + hdr.dbus1.field_length, 8);
	header = l_malloc(header_size);

	body_size = hdr.dbus1.body_length;
	body = l_malloc(body_size);

	iov[0].iov_base = header;
	iov[0].iov_len  = header_size;
	iov[1].iov_base = body;
	iov[1].iov_len  = body_size;

	iovpos = iov;
	iovlen = 2;

	while (1) {
		memset(&msg, 0, sizeof(msg));
		msg.msg_iov = iovpos;
		msg.msg_iovlen = iovlen;
		msg.msg_control = &fd_buf;
		msg.msg_controllen = sizeof(fd_buf);

		r = L_TFR(recvmsg(fd, &msg,
					MSG_CMSG_CLOEXEC | MSG_WAITALL));
		if (r < 0)
			goto cmsg_fail;

		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg;
				cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_level != SOL_SOCKET ||
					cmsg->cmsg_type != SCM_RIGHTS)
				continue;

			num_fds = (cmsg->cmsg_len - CMSG_LEN(0)) / sizeof(int);
			fds = (void *) CMSG_DATA(cmsg);

			/* Set FD_CLOEXEC on all file descriptors */
			for (i = 0; i < num_fds; i++) {
				long flags;

				flags = fcntl(fds[i], F_GETFD, NULL);
				if (flags < 0)
					continue;

				if (!(flags & FD_CLOEXEC))
					fcntl(fds[i], F_SETFD,
						flags | FD_CLOEXEC);
			}

			classic->fd_buf = l_realloc(classic->fd_buf,
						(classic->num_fds + num_fds) *
						sizeof(int));
			memcpy(classic->fd_buf + classic->num_fds, fds,
				num_fds * sizeof(int));
			classic->num_fds += num_fds;
		}

		while ((size_t) r >= iovpos->iov_len) {
			r -= iovpos->iov_len;
			iovpos++;
			iovlen--;

			if (!iovlen)
				break;
		}

		if (!iovlen)
			break;

		iovpos->iov_base += r;
		iovpos->iov_len -= r;
	}

	if (hdr.endian != DBUS_NATIVE_ENDIAN) {
		l_util_debug(dbus->debug_handler,
				dbus->debug_data, "Endianness incorrect");
		goto bad_msg;
	}

	if (hdr.version != 1) {
		l_util_debug(dbus->debug_handler,
				dbus->debug_data, "Protocol version incorrect");
		goto bad_msg;
	}

	num_fds = _dbus_message_unix_fds_from_header(header, header_size);
	if (num_fds > classic->num_fds)
		goto bad_msg;

	message = dbus_message_build(header, header_size, body, body_size,
					classic->fd_buf, num_fds);

	if (message && num_fds) {
		if (classic->num_fds > num_fds) {
			memmove(classic->fd_buf, classic->fd_buf + num_fds,
				(classic->num_fds - num_fds) * sizeof(int));
			classic->num_fds -= num_fds;
		} else {
			l_free(classic->fd_buf);

			classic->fd_buf = NULL;
			classic->num_fds = 0;
		}
	}

	if (message)
		return message;

bad_msg:
cmsg_fail:
	for (i = 0; i < classic->num_fds; i++)
		close(classic->fd_buf[i]);

	l_free(classic->fd_buf);

	classic->fd_buf = NULL;
	classic->num_fds = 0;

	l_free(header);
	l_free(body);

	return NULL;
}

static bool classic_add_match(struct l_dbus *dbus, unsigned int id,
				const struct _dbus_filter_condition *rule,
				int rule_len)
{
	struct l_dbus_classic *classic =
		l_container_of(dbus, struct l_dbus_classic, super);
	char *match_str;
	struct l_dbus_message *message;

	match_str = _dbus_filter_rule_to_str(rule, rule_len);

	l_hashmap_insert(classic->match_strings, L_UINT_TO_PTR(id), match_str);

	message = l_dbus_message_new_method_call(dbus,
						DBUS_SERVICE_DBUS,
						DBUS_PATH_DBUS,
						L_DBUS_INTERFACE_DBUS,
						"AddMatch");

	l_dbus_message_set_arguments(message, "s", match_str);

	send_message(dbus, false, message, NULL, NULL, NULL);

	return true;
}

static bool classic_remove_match(struct l_dbus *dbus, unsigned int id)
{
	struct l_dbus_classic *classic =
		l_container_of(dbus, struct l_dbus_classic, super);
	char *match_str = l_hashmap_remove(classic->match_strings,
						L_UINT_TO_PTR(id));
	struct l_dbus_message *message;

	if (!match_str)
		return false;

	message = l_dbus_message_new_method_call(dbus,
						DBUS_SERVICE_DBUS,
						DBUS_PATH_DBUS,
						L_DBUS_INTERFACE_DBUS,
						"RemoveMatch");

	l_dbus_message_set_arguments(message, "s", match_str);

	send_message(dbus, false, message, NULL, NULL, NULL);

	l_free(match_str);

	return true;
}

static void name_owner_changed_cb(struct l_dbus_message *message,
					void *user_data)
{
	struct l_dbus *dbus = user_data;
	char *name, *old, *new;

	if (!l_dbus_message_get_arguments(message, "sss", &name, &old, &new))
		return;

	_dbus_name_cache_notify(dbus->name_cache, name, new);
}

struct get_name_owner_request {
	struct l_dbus_message *message;
	struct l_dbus *dbus;
};

static void get_name_owner_reply_cb(struct l_dbus_message *reply,
					void *user_data)
{
	struct get_name_owner_request *req = user_data;
	const char *name, *owner;

	/* No name owner yet */
	if (l_dbus_message_is_error(reply))
		return;

	/* Shouldn't happen */
	if (!l_dbus_message_get_arguments(reply, "s", &owner))
		return;

	/* Shouldn't happen */
	if (!l_dbus_message_get_arguments(req->message, "s", &name))
		return;

	_dbus_name_cache_notify(req->dbus->name_cache, name, owner);
}

static bool classic_get_name_owner(struct l_dbus *bus, const char *name)
{
	struct get_name_owner_request *req;

	/* Name resolution is not performed for DBUS_SERVICE_DBUS */
	if (!strcmp(name, DBUS_SERVICE_DBUS))
		return false;

	req = l_new(struct get_name_owner_request, 1);
	req->dbus = bus;
	req->message = l_dbus_message_new_method_call(bus,
							DBUS_SERVICE_DBUS,
							DBUS_PATH_DBUS,
							L_DBUS_INTERFACE_DBUS,
							"GetNameOwner");

	l_dbus_message_set_arguments(req->message, "s", name);

	send_message(bus, false, req->message, get_name_owner_reply_cb,
			req, l_free);

	if (!bus->name_notify_enabled) {
		static struct _dbus_filter_condition rule[] = {
			{ L_DBUS_MATCH_TYPE,		"signal" },
			{ L_DBUS_MATCH_SENDER,		DBUS_SERVICE_DBUS },
			{ L_DBUS_MATCH_PATH,		DBUS_PATH_DBUS },
			{ L_DBUS_MATCH_INTERFACE,	L_DBUS_INTERFACE_DBUS },
			{ L_DBUS_MATCH_MEMBER,		"NameOwnerChanged" },
		};

		if (!bus->filter)
			bus->filter = _dbus_filter_new(bus,
						&bus->driver->filter_ops,
						bus->name_cache);

		_dbus_filter_add_rule(bus->filter, rule, L_ARRAY_SIZE(rule),
					name_owner_changed_cb, bus);

		bus->name_notify_enabled = true;
	}

	return true;
}

struct name_request {
	l_dbus_name_acquire_func_t callback;
	void *user_data;
	struct l_dbus *dbus;
};

enum dbus_name_flag {
	DBUS_NAME_FLAG_ALLOW_REPLACEMENT	= 0x1,
	DBUS_NAME_FLAG_REPLACE_EXISTING		= 0x2,
	DBUS_NAME_FLAG_DO_NOT_QUEUE		= 0x4,
};

enum dbus_name_reply {
	DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER	= 1,
	DBUS_REQUEST_NAME_REPLY_IN_QUEUE	= 2,
	DBUS_REQUEST_NAME_REPLY_EXISTS		= 3,
	DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER	= 4,
};

static void request_name_reply_cb(struct l_dbus_message *reply, void *user_data)
{
	struct name_request *req = user_data;
	bool success = false, queued = false;
	uint32_t retval;

	if (!req->callback)
		return;

	/* No name owner yet */
	if (l_dbus_message_is_error(reply))
		goto call_back;

	/* Shouldn't happen */
	if (!l_dbus_message_get_arguments(reply, "u", &retval))
		goto call_back;

	success = (retval == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) ||
		(retval == DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER) ||
		(retval == DBUS_REQUEST_NAME_REPLY_IN_QUEUE);
	queued = (retval == DBUS_REQUEST_NAME_REPLY_IN_QUEUE);

call_back:
	req->callback(req->dbus, success, queued, req->user_data);
}

static uint32_t classic_name_acquire(struct l_dbus *dbus, const char *name,
					bool allow_replacement,
					bool replace_existing, bool queue,
					l_dbus_name_acquire_func_t callback,
					void *user_data)
{
	struct name_request *req;
	struct l_dbus_message *message;
	uint32_t flags = 0;

	req = l_new(struct name_request, 1);
	req->dbus = dbus;
	req->user_data = user_data;
	req->callback = callback;

	message = l_dbus_message_new_method_call(dbus, DBUS_SERVICE_DBUS,
							DBUS_PATH_DBUS,
							L_DBUS_INTERFACE_DBUS,
							"RequestName");

	if (allow_replacement)
		flags |= DBUS_NAME_FLAG_ALLOW_REPLACEMENT;

	if (replace_existing)
		flags |= DBUS_NAME_FLAG_REPLACE_EXISTING;

	if (!queue)
		flags |= DBUS_NAME_FLAG_DO_NOT_QUEUE;

	l_dbus_message_set_arguments(message, "su", name, flags);

	return send_message(dbus, false, message, request_name_reply_cb,
				req, free);
}

static const struct l_dbus_ops classic_ops = {
	.version = 1,
	.send_message = classic_send_message,
	.recv_message = classic_recv_message,
	.free = classic_free,
	.name_ops = {
		.get_name_owner = classic_get_name_owner,
	},
	.filter_ops = {
		.add_match = classic_add_match,
		.remove_match = classic_remove_match,
	},
	.name_acquire = classic_name_acquire,
};

static struct l_dbus *setup_dbus1(int fd, const char *guid)
{
	static const unsigned char creds = 0x00;
	char uid[6], hexuid[12], *ptr = hexuid;
	struct l_dbus *dbus;
	struct l_dbus_classic *classic;
	ssize_t written;
	unsigned int i;

	if (snprintf(uid, sizeof(uid), "%d", geteuid()) < 1) {
		close(fd);
		return NULL;
	}

	for (i = 0; i < strlen(uid); i++)
		ptr += sprintf(ptr, "%02x", uid[i]);

	/* Send special credentials-passing nul byte */
	written = L_TFR(send(fd, &creds, 1, 0));
	if (written < 1) {
		close(fd);
		return NULL;
	}

	classic = l_new(struct l_dbus_classic, 1);
	dbus = &classic->super;
	dbus->driver = &classic_ops;

	classic->match_strings = l_hashmap_new();

	dbus_init(dbus, fd);
	dbus->guid = l_strdup(guid);

	classic->auth_command = l_strdup_printf("AUTH EXTERNAL %s\r\n", hexuid);
	classic->auth_state = WAITING_FOR_OK;

	dbus->negotiate_unix_fd = true;
	dbus->support_unix_fd = false;

	l_io_set_read_handler(dbus->io, auth_read_handler, dbus, NULL);
	l_io_set_write_handler(dbus->io, auth_write_handler, dbus, NULL);

	return dbus;
}

static struct l_dbus *setup_unix(char *params)
{
	char *path = NULL, *guid = NULL;
	bool abstract = false;
	struct sockaddr_un addr;
	size_t len;
	int fd;

	while (params) {
		char *key = strsep(&params, ",");
		char *value;

		if (!key)
			break;

		value = strchr(key, '=');
		if (!value)
			continue;

		*value++ = '\0';

		if (!strcmp(key, "path")) {
			path = value;
			abstract = false;
		} else if (!strcmp(key, "abstract")) {
			path = value;
			abstract = true;
		} else if (!strcmp(key, "guid"))
			guid = value;
	}

	if (!path)
		return NULL;

	fd = socket(PF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (fd < 0)
		return NULL;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	len = strlen(path);

	if (abstract) {
		if (len > sizeof(addr.sun_path) - 1) {
			close(fd);
			return NULL;
		}

		addr.sun_path[0] = '\0';
		strncpy(addr.sun_path + 1, path, sizeof(addr.sun_path) - 2);
		len++;
	} else {
		if (len > sizeof(addr.sun_path)) {
			close(fd);
			return NULL;
		}

		strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
	}

	if (connect(fd, (struct sockaddr *) &addr,
				sizeof(addr.sun_family) + len) < 0) {
		close(fd);
		return NULL;
	}

	return setup_dbus1(fd, guid);
}

static struct l_dbus *setup_address(const char *address)
{
	struct l_dbus *dbus = NULL;
	char *address_copy;

	address_copy = strdupa(address);

	while (address_copy) {
		char *transport = strsep(&address_copy, ";");
		char *params;

		if (!transport)
			break;

		params = strchr(transport, ':');
		if (params)
			*params++ = '\0';

		if (!strcmp(transport, "unix")) {
			/* Function will modify params string */
			dbus = setup_unix(params);
			break;
		}
	}

	return dbus;
}

LIB_EXPORT struct l_dbus *l_dbus_new(const char *address)
{
	if (unlikely(!address))
		return NULL;

	return setup_address(address);
}

LIB_EXPORT struct l_dbus *l_dbus_new_default(enum l_dbus_bus bus)
{
	const char *address;

	switch (bus) {
	case L_DBUS_SYSTEM_BUS:
		address = getenv("DBUS_SYSTEM_BUS_ADDRESS");
		if (!address)
			address = DEFAULT_SYSTEM_BUS_ADDRESS;
		break;
	case L_DBUS_SESSION_BUS:
		address = getenv("DBUS_SESSION_BUS_ADDRESS");
		if (!address)
			return NULL;
		break;
	default:
		return NULL;
	}

	return setup_address(address);
}

LIB_EXPORT void l_dbus_destroy(struct l_dbus *dbus)
{
	if (unlikely(!dbus))
		return;

	if (dbus->ready_destroy)
		dbus->ready_destroy(dbus->ready_data);

	_dbus_filter_free(dbus->filter);

	_dbus_name_cache_free(dbus->name_cache);

	l_hashmap_destroy(dbus->signal_list, signal_list_destroy);
	l_hashmap_destroy(dbus->message_list, message_list_destroy);
	l_queue_destroy(dbus->message_queue, message_queue_destroy);

	l_io_destroy(dbus->io);

	if (dbus->disconnect_destroy)
		dbus->disconnect_destroy(dbus->disconnect_data);

	if (dbus->debug_destroy)
		dbus->debug_destroy(dbus->debug_data);

	l_free(dbus->guid);
	l_free(dbus->unique_name);

	_dbus_object_tree_free(dbus->tree);

	dbus->driver->free(dbus);
}

LIB_EXPORT bool l_dbus_set_ready_handler(struct l_dbus *dbus,
				l_dbus_ready_func_t function,
				void *user_data, l_dbus_destroy_func_t destroy)
{
	if (unlikely(!dbus))
		return false;

	if (dbus->ready_destroy)
		dbus->ready_destroy(dbus->ready_data);

	dbus->ready_handler = function;
	dbus->ready_destroy = destroy;
	dbus->ready_data = user_data;

	return true;
}

LIB_EXPORT bool l_dbus_set_disconnect_handler(struct l_dbus *dbus,
				l_dbus_disconnect_func_t function,
				void *user_data, l_dbus_destroy_func_t destroy)
{
	if (unlikely(!dbus))
		return false;

	if (dbus->disconnect_destroy)
		dbus->disconnect_destroy(dbus->disconnect_data);

	dbus->disconnect_handler = function;
	dbus->disconnect_destroy = destroy;
	dbus->disconnect_data = user_data;

	return true;
}

LIB_EXPORT bool l_dbus_set_debug(struct l_dbus *dbus,
				l_dbus_debug_func_t function,
				void *user_data, l_dbus_destroy_func_t destroy)
{
	if (unlikely(!dbus))
		return false;

	if (dbus->debug_destroy)
		dbus->debug_destroy(dbus->debug_data);

	dbus->debug_handler = function;
	dbus->debug_destroy = destroy;
	dbus->debug_data = user_data;

	/* l_io_set_debug(dbus->io, function, user_data, NULL); */

	return true;
}

LIB_EXPORT uint32_t l_dbus_send_with_reply(struct l_dbus *dbus,
						struct l_dbus_message *message,
						l_dbus_message_func_t function,
						void *user_data,
						l_dbus_destroy_func_t destroy)
{
	if (unlikely(!dbus || !message))
		return 0;

	return send_message(dbus, false, message, function, user_data, destroy);
}

LIB_EXPORT uint32_t l_dbus_send(struct l_dbus *dbus,
				struct l_dbus_message *message)
{
	if (unlikely(!dbus || !message))
		return 0;

	return send_message(dbus, false, message, NULL, NULL, NULL);
}

static bool remove_entry(void *data, void *user_data)
{
	struct message_callback *callback = data;
	uint32_t serial = L_PTR_TO_UINT(user_data);

	if (callback->serial == serial) {
		message_queue_destroy(callback);
		return true;
	}

	return false;
}

LIB_EXPORT bool l_dbus_cancel(struct l_dbus *dbus, uint32_t serial)
{
	struct message_callback *callback;
	unsigned int count;

	if (unlikely(!dbus || !serial))
		return false;

	callback = l_hashmap_remove(dbus->message_list, L_UINT_TO_PTR(serial));
        if (callback) {
		message_queue_destroy(callback);
		return true;
	}

	count = l_queue_foreach_remove(dbus->message_queue, remove_entry,
							L_UINT_TO_PTR(serial));
	if (!count)
		return false;

	return true;
}

LIB_EXPORT unsigned int l_dbus_register(struct l_dbus *dbus,
				l_dbus_message_func_t function,
				void *user_data, l_dbus_destroy_func_t destroy)
{
	struct signal_callback *callback;

	if (unlikely(!dbus))
		return 0;

	callback = l_new(struct signal_callback, 1);

	callback->id = dbus->next_id++;
	callback->callback = function;
	callback->destroy = destroy;
	callback->user_data = user_data;

	l_hashmap_insert(dbus->signal_list,
				L_UINT_TO_PTR(callback->id), callback);

	return callback->id;
}

LIB_EXPORT bool l_dbus_unregister(struct l_dbus *dbus, unsigned int id)
{
	struct signal_callback *callback;

	if (unlikely(!dbus || !id))
		return false;

	callback = l_hashmap_remove(dbus->signal_list, L_UINT_TO_PTR(id));
	if (!callback)
		return false;

	signal_list_destroy(callback);

	return true;
}

LIB_EXPORT uint32_t l_dbus_method_call(struct l_dbus *dbus,
				const char *destination, const char *path,
				const char *interface, const char *method,
				l_dbus_message_func_t setup,
				l_dbus_message_func_t function,
				void *user_data, l_dbus_destroy_func_t destroy)
{
	struct l_dbus_message *message;

	if (unlikely(!dbus))
		return 0;

	message = l_dbus_message_new_method_call(dbus, destination, path,
							interface, method);

	if (setup)
		setup(message, user_data);
	else
		l_dbus_message_set_arguments(message, "");

	return send_message(dbus, false, message, function, user_data, destroy);
}

uint8_t _dbus_get_version(struct l_dbus *dbus)
{
	return dbus->driver->version;
}

int _dbus_get_fd(struct l_dbus *dbus)
{
	return l_io_get_fd(dbus->io);
}

struct _dbus_object_tree *_dbus_get_tree(struct l_dbus *dbus)
{
	return dbus->tree;
}

/**
 * l_dbus_register_interface:
 * @dbus: D-Bus connection as returned by @l_dbus_new*
 * @interface: interface name string
 * @setup_func: function that sets up the methods, signals and properties by
 *              using the #dbus-service.h API.
 * @destroy: optional destructor to be called every time an instance of this
 *           interface is being removed from an object on this bus.
 * @handle_old_style_properties: whether to automatically handle SetProperty and
 *                               GetProperties for any properties registered by
 *                               @setup_func.
 *
 * Registers an interface.  If successful the interface can then be added
 * to any number of objects with @l_dbus_object_add_interface.
 *
 * Returns: whether the interface was successfully registered
 **/
LIB_EXPORT bool l_dbus_register_interface(struct l_dbus *dbus,
				const char *interface,
				l_dbus_interface_setup_func_t setup_func,
				l_dbus_destroy_func_t destroy,
				bool handle_old_style_properties)
{
	if (unlikely(!dbus))
		return false;

	if (unlikely(!dbus->tree))
		return false;

	return _dbus_object_tree_register_interface(dbus->tree, interface,
						setup_func, destroy,
						handle_old_style_properties);
}

LIB_EXPORT bool l_dbus_unregister_interface(struct l_dbus *dbus,
						const char *interface)
{
	if (unlikely(!dbus))
		return false;

	if (unlikely(!dbus->tree))
		return false;

	return _dbus_object_tree_unregister_interface(dbus->tree, interface);
}

/**
 * l_dbus_register_object:
 * @dbus: D-Bus connection
 * @path: new object path
 * @user_data: user pointer to be passed to @destroy if any
 * @destroy: optional destructor to be called when object dropped from the tree
 * @...: NULL-terminated list of 0 or more interfaces to be present on the
 *       object from the moment of creation.  For every interface the interface
 *       name string is expected followed by the @user_data pointer same as
 *       would be passed as @l_dbus_object_add_interface's last two parameters.
 *
 * Create a new D-Bus object on the tree visible to D-Bus peers.  For example:
 * 	success = l_dbus_register_object(bus, "/org/example/ExampleManager",
 * 						NULL, NULL,
 * 						"org.example.Manager",
 * 						manager_data,
 * 						NULL);
 *
 * Returns: whether the object path was successfully registered
 **/
LIB_EXPORT bool l_dbus_register_object(struct l_dbus *dbus, const char *path,
					void *user_data,
					l_dbus_destroy_func_t destroy, ...)
{
	va_list args;
	const char *interface;
	void *if_user_data;
	bool r = true;

	if (unlikely(!dbus))
		return false;

	if (unlikely(!dbus->tree))
		return false;

	if (!_dbus_object_tree_new_object(dbus->tree, path, user_data, destroy))
		return false;

	va_start(args, destroy);
	while ((interface = va_arg(args, const char *))) {
		if_user_data = va_arg(args, void *);

		if (!_dbus_object_tree_add_interface(dbus->tree, path,
							interface,
							if_user_data)) {
			_dbus_object_tree_object_destroy(dbus->tree, path);
			r = false;

			break;
		}
	}
	va_end(args);

	return r;
}

LIB_EXPORT bool l_dbus_unregister_object(struct l_dbus *dbus,
						const char *object)
{
	if (unlikely(!dbus))
		return false;

	if (unlikely(!dbus->tree))
		return false;

	return _dbus_object_tree_object_destroy(dbus->tree, object);
}

/**
 * l_dbus_object_add_interface:
 * @dbus: D-Bus connection
 * @object: object path as passed to @l_dbus_register_object
 * @interface: interface name as passed to @l_dbus_register_interface
 * @user_data: user data pointer to be passed to any method and property
 *             callbacks provided by the @setup_func and to the @destroy
 *             callback as passed to @l_dbus_register_interface
 *
 * Creates an instance of given interface at the given path in the
 * connection's object tree.  If no object was registered at this path
 * before @l_dbus_register_object gets called automatically.
 *
 * The addition of an interface to the object may trigger a query of
 * all the properties on this interface and
 * #org.freedesktop.DBus.ObjectManager.InterfacesAdded signals.
 *
 * Returns: whether the interface was successfully added.
 **/
LIB_EXPORT bool l_dbus_object_add_interface(struct l_dbus *dbus,
						const char *object,
						const char *interface,
						void *user_data)
{
	if (unlikely(!dbus))
		return false;

	if (unlikely(!dbus->tree))
		return false;

	return _dbus_object_tree_add_interface(dbus->tree, object, interface,
						user_data);
}

LIB_EXPORT bool l_dbus_object_remove_interface(struct l_dbus *dbus,
						const char *object,
						const char *interface)
{
	if (unlikely(!dbus))
		return false;

	if (unlikely(!dbus->tree))
		return false;

	return _dbus_object_tree_remove_interface(dbus->tree, object,
							interface);
}

LIB_EXPORT void *l_dbus_object_get_data(struct l_dbus *dbus, const char *object,
					const char *interface)
{
	if (unlikely(!dbus))
		return false;

	if (unlikely(!dbus->tree))
		return false;

	return _dbus_object_tree_get_interface_data(dbus->tree, object,
							interface);
}

LIB_EXPORT bool l_dbus_object_manager_enable(struct l_dbus *dbus,
						const char *root)
{
	if (unlikely(!dbus))
		return false;

	if (unlikely(!dbus->tree))
		return false;

	return _dbus_object_tree_add_interface(dbus->tree, root,
						L_DBUS_INTERFACE_OBJECT_MANAGER,
						dbus);
}

LIB_EXPORT unsigned int l_dbus_add_disconnect_watch(struct l_dbus *dbus,
					const char *name,
					l_dbus_watch_func_t disconnect_func,
					void *user_data,
					l_dbus_destroy_func_t destroy)
{
	return l_dbus_add_service_watch(dbus, name, NULL, disconnect_func,
					user_data, destroy);
}

LIB_EXPORT unsigned int l_dbus_add_service_watch(struct l_dbus *dbus,
					const char *name,
					l_dbus_watch_func_t connect_func,
					l_dbus_watch_func_t disconnect_func,
					void *user_data,
					l_dbus_destroy_func_t destroy)
{
	if (!name)
		return 0;

	if (!dbus->name_cache)
		dbus->name_cache = _dbus_name_cache_new(dbus,
						&dbus->driver->name_ops);

	return _dbus_name_cache_add_watch(dbus->name_cache, name, connect_func,
						disconnect_func, user_data,
						destroy);
}

LIB_EXPORT bool l_dbus_remove_watch(struct l_dbus *dbus, unsigned int id)
{
	if (!dbus->name_cache)
		return false;

	return _dbus_name_cache_remove_watch(dbus->name_cache, id);
}

/**
 * l_dbus_add_signal_watch:
 * @dbus: D-Bus connection
 * @sender: bus name to match the signal sender against or NULL to
 *          match any sender
 * @path: object path to match the signal path against or NULL to
 *        match any path
 * @interface: interface name to match the signal interface against
 *             or NULL to match any interface
 * @member: name to match the signal name against or NULL to match any
 *          signal
 * @...: a list of further conditions to be met by the signal followed
 *       by three more mandatory parameters:
 *       enum l_dbus_match_type list_end_marker,
 *       l_dbus_message_func callback,
 *       void *user_data,
 *       The value L_DBUS_MATCH_NONE must be passed as the end of list
 *       marker, followed by the signal match callback and user_data.
 *       In the list, every condition is a pair of parameters:
 *       enum l_dbus_match_type match_type, const char *value.
 *
 * Subscribe to a group of signals based on a set of conditions that
 * compare the signal's header fields and string arguments against given
 * values.  For example:
 * 	signal_id = l_dbus_add_signal_watch(bus, "org.example", "/"
 * 						"org.example.Manager",
 * 						"PropertyChanged",
 * 						L_DBUS_MATCH_ARGUMENT(0),
 * 						"ExampleProperty",
 * 						L_DBUS_MATCH_NONE
 * 						manager_property_change_cb,
 * 						NULL);
  *
 * Returns: a non-zero signal filter identifier that can be passed to
 *          l_dbus_remove_signal_watch to remove this filter rule, or
 *          zero on failure.
 **/
LIB_EXPORT unsigned int l_dbus_add_signal_watch(struct l_dbus *dbus,
						const char *sender,
						const char *path,
						const char *interface,
						const char *member, ...)
{
	struct _dbus_filter_condition *rule;
	int rule_len;
	va_list args;
	const char *value;
	l_dbus_message_func_t signal_func;
	enum l_dbus_match_type type;
	void *user_data;
	unsigned int id;

	va_start(args, member);

	rule_len = 0;
	while ((type = va_arg(args, enum l_dbus_match_type)) !=
			L_DBUS_MATCH_NONE)
		rule_len++;

	va_end(args);

	rule = l_new(struct _dbus_filter_condition, rule_len + 5);

	rule_len = 0;

	rule[rule_len].type = L_DBUS_MATCH_TYPE;
	rule[rule_len++].value = "signal";

	if (sender) {
		rule[rule_len].type = L_DBUS_MATCH_SENDER;
		rule[rule_len++].value = sender;
	}

	if (path) {
		rule[rule_len].type = L_DBUS_MATCH_PATH;
		rule[rule_len++].value = path;
	}

	if (interface) {
		rule[rule_len].type = L_DBUS_MATCH_INTERFACE;
		rule[rule_len++].value = interface;
	}

	if (member) {
		rule[rule_len].type = L_DBUS_MATCH_MEMBER;
		rule[rule_len++].value = member;
	}

	va_start(args, member);

	while (true) {
		type = va_arg(args, enum l_dbus_match_type);
		if (type == L_DBUS_MATCH_NONE)
			break;

		value = va_arg(args, const char *);

		rule[rule_len].type = type;
		rule[rule_len++].value = value;
	}

	signal_func = va_arg(args, l_dbus_message_func_t);
	user_data = va_arg(args, void *);

	va_end(args);

	if (!dbus->filter) {
		if (!dbus->name_cache)
			dbus->name_cache = _dbus_name_cache_new(dbus,
						&dbus->driver->name_ops);

		dbus->filter = _dbus_filter_new(dbus,
						&dbus->driver->filter_ops,
						dbus->name_cache);
	}

	id = _dbus_filter_add_rule(dbus->filter, rule, rule_len,
					signal_func, user_data);

	l_free(rule);

	return id;
}

LIB_EXPORT bool l_dbus_remove_signal_watch(struct l_dbus *dbus, unsigned int id)
{
	if (!dbus->filter)
		return false;

	return _dbus_filter_remove_rule(dbus->filter, id);
}

/**
 * l_dbus_name_acquire:
 * @dbus: D-Bus connection
 * @name: Well-known bus name to be acquired
 * @allow_replacement: Whether to allow another peer's name request to
 *                     take the name ownership away from this connection
 * @replace_existing: Whether to allow D-Bus to take the name's ownership
 *                    away from another peer in case the name is already
 *                    owned and allows replacement.  Ignored if name is
 *                    currently free.
 * @queue: Whether to allow the name request to be queued by D-Bus in
 *         case it cannot be acquired now, rather than to return a failure.
 * @callback: Callback to receive the request result when done.
 *
 * Acquire a well-known bus name (service name) on the bus.
 *
 * Returns: a non-zero request serial that can be passed to l_dbus_cancel
 *          while waiting for the callback or zero if the callback has
 *          has happened while l_dbus_name_acquire was running.
 **/
LIB_EXPORT uint32_t l_dbus_name_acquire(struct l_dbus *dbus, const char *name,
				bool allow_replacement, bool replace_existing,
				bool queue, l_dbus_name_acquire_func_t callback,
				void *user_data)
{
	return dbus->driver->name_acquire(dbus, name, allow_replacement,
						replace_existing, queue,
						callback, user_data);
}
