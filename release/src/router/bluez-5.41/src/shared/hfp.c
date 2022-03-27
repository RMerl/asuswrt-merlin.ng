/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
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

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "src/shared/util.h"
#include "src/shared/ringbuf.h"
#include "src/shared/queue.h"
#include "src/shared/io.h"
#include "src/shared/hfp.h"

struct hfp_gw {
	int ref_count;
	int fd;
	bool close_on_unref;
	struct io *io;
	struct ringbuf *read_buf;
	struct ringbuf *write_buf;
	struct queue *cmd_handlers;
	bool writer_active;
	bool result_pending;
	hfp_command_func_t command_callback;
	hfp_destroy_func_t command_destroy;
	void *command_data;
	hfp_debug_func_t debug_callback;
	hfp_destroy_func_t debug_destroy;
	void *debug_data;

	hfp_disconnect_func_t disconnect_callback;
	hfp_destroy_func_t disconnect_destroy;
	void *disconnect_data;

	bool in_disconnect;
	bool destroyed;
};

struct hfp_hf {
	int ref_count;
	int fd;
	bool close_on_unref;
	struct io *io;
	struct ringbuf *read_buf;
	struct ringbuf *write_buf;

	bool writer_active;
	struct queue *cmd_queue;

	struct queue *event_handlers;

	hfp_debug_func_t debug_callback;
	hfp_destroy_func_t debug_destroy;
	void *debug_data;

	hfp_disconnect_func_t disconnect_callback;
	hfp_destroy_func_t disconnect_destroy;
	void *disconnect_data;

	bool in_disconnect;
	bool destroyed;
};

struct cmd_handler {
	char *prefix;
	void *user_data;
	hfp_destroy_func_t destroy;
	hfp_result_func_t callback;
};

struct hfp_context {
	const char *data;
	unsigned int offset;
};

struct cmd_response {
	hfp_response_func_t resp_cb;
	struct hfp_context *response;
	char *resp_data;
	void *user_data;
};

struct event_handler {
	char *prefix;
	void *user_data;
	hfp_destroy_func_t destroy;
	hfp_hf_result_func_t callback;
};

static void destroy_cmd_handler(void *data)
{
	struct cmd_handler *handler = data;

	if (handler->destroy)
		handler->destroy(handler->user_data);

	free(handler->prefix);

	free(handler);
}

static bool match_handler_prefix(const void *a, const void *b)
{
	const struct cmd_handler *handler = a;
	const char *prefix = b;

	if (strcmp(handler->prefix, prefix) != 0)
		return false;

	return true;
}

static void write_watch_destroy(void *user_data)
{
	struct hfp_gw *hfp = user_data;

	hfp->writer_active = false;
}

static bool can_write_data(struct io *io, void *user_data)
{
	struct hfp_gw *hfp = user_data;
	ssize_t bytes_written;

	bytes_written = ringbuf_write(hfp->write_buf, hfp->fd);
	if (bytes_written < 0)
		return false;

	if (ringbuf_len(hfp->write_buf) > 0)
		return true;

	return false;
}

static void wakeup_writer(struct hfp_gw *hfp)
{
	if (hfp->writer_active)
		return;

	if (!ringbuf_len(hfp->write_buf))
		return;

	if (!io_set_write_handler(hfp->io, can_write_data,
					hfp, write_watch_destroy))
		return;

	hfp->writer_active = true;
}

static void skip_whitespace(struct hfp_context *context)
{
	while (context->data[context->offset] == ' ')
		context->offset++;
}

static void handle_unknown_at_command(struct hfp_gw *hfp,
							const char *data)
{
	if (hfp->command_callback) {
		hfp->result_pending = true;
		hfp->command_callback(data, hfp->command_data);
	} else {
		hfp_gw_send_result(hfp, HFP_RESULT_ERROR);
	}
}

static bool handle_at_command(struct hfp_gw *hfp, const char *data)
{
	struct cmd_handler *handler;
	const char *separators = ";?=\0";
	struct hfp_context context;
	enum hfp_gw_cmd_type type;
	char lookup_prefix[18];
	uint8_t pref_len = 0;
	const char *prefix;
	int i;

	context.offset = 0;
	context.data = data;

	skip_whitespace(&context);

	if (strlen(data + context.offset) < 3)
		return false;

	if (strncmp(data + context.offset, "AT", 2))
		if (strncmp(data + context.offset, "at", 2))
			return false;

	context.offset += 2;
	prefix = data + context.offset;

	if (isalpha(prefix[0])) {
		lookup_prefix[pref_len++] = toupper(prefix[0]);
	} else {
		pref_len = strcspn(prefix, separators);
		if (pref_len > 17 || pref_len < 2)
			return false;

		for (i = 0; i < pref_len; i++)
			lookup_prefix[i] = toupper(prefix[i]);
	}

	lookup_prefix[pref_len] = '\0';
	context.offset += pref_len;

	if (lookup_prefix[0] == 'D') {
		type = HFP_GW_CMD_TYPE_SET;
		goto done;
	}

	if (data[context.offset] == '=') {
		context.offset++;
		if (data[context.offset] == '?') {
			context.offset++;
			type = HFP_GW_CMD_TYPE_TEST;
		} else {
			type = HFP_GW_CMD_TYPE_SET;
		}
		goto done;
	}

	if (data[context.offset] == '?') {
		context.offset++;
		type = HFP_GW_CMD_TYPE_READ;
		goto done;
	}

	type = HFP_GW_CMD_TYPE_COMMAND;

done:

	handler = queue_find(hfp->cmd_handlers, match_handler_prefix,
								lookup_prefix);
	if (!handler) {
		handle_unknown_at_command(hfp, data);
		return true;
	}

	hfp->result_pending = true;
	handler->callback(&context, type, handler->user_data);

	return true;
}

static void next_field(struct hfp_context *context)
{
	if (context->data[context->offset] == ',')
		context->offset++;
}

bool hfp_context_get_number_default(struct hfp_context *context,
						unsigned int *val,
						unsigned int default_val)
{
	skip_whitespace(context);

	if (context->data[context->offset] == ',') {
		if (val)
			*val = default_val;

		context->offset++;
		return true;
	}

	return hfp_context_get_number(context, val);
}

bool hfp_context_get_number(struct hfp_context *context,
							unsigned int *val)
{
	unsigned int i;
	int tmp = 0;

	skip_whitespace(context);

	i = context->offset;

	while (context->data[i] >= '0' && context->data[i] <= '9')
		tmp = tmp * 10 + context->data[i++] - '0';

	if (i == context->offset)
		return false;

	if (val)
		*val = tmp;
	context->offset = i;

	skip_whitespace(context);
	next_field(context);

	return true;
}

bool hfp_context_open_container(struct hfp_context *context)
{
	skip_whitespace(context);

	/* The list shall be preceded by a left parenthesis "(") */
	if (context->data[context->offset] != '(')
		return false;

	context->offset++;

	return true;
}

bool hfp_context_close_container(struct hfp_context *context)
{
	skip_whitespace(context);

	/* The list shall be followed by a right parenthesis (")" V250 5.7.3.1*/
	if (context->data[context->offset] != ')')
		return false;

	context->offset++;

	next_field(context);

	return true;
}

bool hfp_context_get_string(struct hfp_context *context, char *buf,
								uint8_t len)
{
	int i = 0;
	const char *data = context->data;
	unsigned int offset;

	skip_whitespace(context);

	if (data[context->offset] != '"')
		return false;

	offset = context->offset;
	offset++;

	while (data[offset] != '\0' && data[offset] != '"') {
		if (i == len)
			return false;

		buf[i++] = data[offset];
		offset++;
	}

	if (i == len)
		return false;

	buf[i] = '\0';

	if (data[offset] == '"')
		offset++;
	else
		return false;

	context->offset = offset;

	skip_whitespace(context);
	next_field(context);

	return true;
}

bool hfp_context_get_unquoted_string(struct hfp_context *context,
							char *buf, uint8_t len)
{
	const char *data = context->data;
	unsigned int offset;
	int i = 0;
	char c;

	skip_whitespace(context);

	c = data[context->offset];
	if (c == '"' || c == ')' || c == '(')
		return false;

	offset = context->offset;

	while (data[offset] != '\0' && data[offset] != ',' &&
							data[offset] != ')') {
		if (i == len)
			return false;

		buf[i++] = data[offset];
		offset++;
	}

	if (i == len)
		return false;

	buf[i] = '\0';

	context->offset = offset;

	next_field(context);

	return true;
}

bool hfp_context_has_next(struct hfp_context *context)
{
	return context->data[context->offset] != '\0';
}

void hfp_context_skip_field(struct hfp_context *context)
{
	const char *data = context->data;
	unsigned int offset = context->offset;

	while (data[offset] != '\0' && data[offset] != ',')
		offset++;

	context->offset = offset;
	next_field(context);
}

bool hfp_context_get_range(struct hfp_context *context, uint32_t *min,
								uint32_t *max)
{
	uint32_t l, h;
	uint32_t start;

	start = context->offset;

	if (!hfp_context_get_number(context, &l))
		goto failed;

	if (context->data[context->offset] != '-')
		goto failed;

	context->offset++;

	if (!hfp_context_get_number(context, &h))
		goto failed;

	*min = l;
	*max = h;

	next_field(context);

	return true;

failed:
	context->offset = start;
	return false;
}

static void process_input(struct hfp_gw *hfp)
{
	char *str, *ptr;
	size_t len, count;
	bool free_ptr = false;
	bool read_again;

	do {
		str = ringbuf_peek(hfp->read_buf, 0, &len);
		if (!str)
			return;

		ptr = memchr(str, '\r', len);
		if (!ptr) {
			char *str2;
			size_t len2;

			/*
			 * If there is no more data in ringbuffer,
			 * it's just an incomplete command.
			 */
			if (len == ringbuf_len(hfp->read_buf))
				return;

			str2 = ringbuf_peek(hfp->read_buf, len, &len2);
			if (!str2)
				return;

			ptr = memchr(str2, '\r', len2);
			if (!ptr)
				return;

			*ptr = '\0';

			count = len2 + len;
			ptr = malloc(count);
			if (!ptr)
				return;

			memcpy(ptr, str, len);
			memcpy(ptr + len, str2, len2);

			free_ptr = true;
			str = ptr;
		} else {
			count = ptr - str;
			*ptr = '\0';
		}

		if (!handle_at_command(hfp, str))
			/*
			 * Command is not handled that means that was some
			 * trash. Let's skip that and keep reading from ring
			 * buffer.
			 */
			read_again = true;
		else
			/*
			 * Command has been handled. If we are waiting for a
			 * result from upper layer, we can stop reading. If we
			 * already reply i.e. ERROR on unknown command, then we
			 * can keep reading ring buffer. Actually ring buffer
			 * should be empty but lets just look there.
			 */
			read_again = !hfp->result_pending;

		ringbuf_drain(hfp->read_buf, count + 1);

		if (free_ptr)
			free(ptr);

	} while (read_again);
}

static void read_watch_destroy(void *user_data)
{
}

static bool can_read_data(struct io *io, void *user_data)
{
	struct hfp_gw *hfp = user_data;
	ssize_t bytes_read;

	bytes_read = ringbuf_read(hfp->read_buf, hfp->fd);
	if (bytes_read < 0)
		return false;

	if (hfp->result_pending)
		return true;

	process_input(hfp);

	return true;
}

struct hfp_gw *hfp_gw_new(int fd)
{
	struct hfp_gw *hfp;

	if (fd < 0)
		return NULL;

	hfp = new0(struct hfp_gw, 1);
	hfp->fd = fd;
	hfp->close_on_unref = false;

	hfp->read_buf = ringbuf_new(4096);
	if (!hfp->read_buf) {
		free(hfp);
		return NULL;
	}

	hfp->write_buf = ringbuf_new(4096);
	if (!hfp->write_buf) {
		ringbuf_free(hfp->read_buf);
		free(hfp);
		return NULL;
	}

	hfp->io = io_new(fd);
	if (!hfp->io) {
		ringbuf_free(hfp->write_buf);
		ringbuf_free(hfp->read_buf);
		free(hfp);
		return NULL;
	}

	hfp->cmd_handlers = queue_new();

	if (!io_set_read_handler(hfp->io, can_read_data, hfp,
							read_watch_destroy)) {
		queue_destroy(hfp->cmd_handlers, destroy_cmd_handler);
		io_destroy(hfp->io);
		ringbuf_free(hfp->write_buf);
		ringbuf_free(hfp->read_buf);
		free(hfp);
		return NULL;
	}

	hfp->writer_active = false;
	hfp->result_pending = false;

	return hfp_gw_ref(hfp);
}

struct hfp_gw *hfp_gw_ref(struct hfp_gw *hfp)
{
	if (!hfp)
		return NULL;

	__sync_fetch_and_add(&hfp->ref_count, 1);

	return hfp;
}

void hfp_gw_unref(struct hfp_gw *hfp)
{
	if (!hfp)
		return;

	if (__sync_sub_and_fetch(&hfp->ref_count, 1))
		return;

	hfp_gw_set_command_handler(hfp, NULL, NULL, NULL);

	io_set_write_handler(hfp->io, NULL, NULL, NULL);
	io_set_read_handler(hfp->io, NULL, NULL, NULL);
	io_set_disconnect_handler(hfp->io, NULL, NULL, NULL);

	io_destroy(hfp->io);
	hfp->io = NULL;

	if (hfp->close_on_unref)
		close(hfp->fd);

	hfp_gw_set_debug(hfp, NULL, NULL, NULL);

	ringbuf_free(hfp->read_buf);
	hfp->read_buf = NULL;

	ringbuf_free(hfp->write_buf);
	hfp->write_buf = NULL;

	queue_destroy(hfp->cmd_handlers, destroy_cmd_handler);
	hfp->cmd_handlers = NULL;

	if (!hfp->in_disconnect) {
		free(hfp);
		return;
	}

	hfp->destroyed = true;
}

static void read_tracing(const void *buf, size_t count, void *user_data)
{
	struct hfp_gw *hfp = user_data;

	util_hexdump('>', buf, count, hfp->debug_callback, hfp->debug_data);
}

static void write_tracing(const void *buf, size_t count, void *user_data)
{
	struct hfp_gw *hfp = user_data;

	util_hexdump('<', buf, count, hfp->debug_callback, hfp->debug_data);
}

bool hfp_gw_set_debug(struct hfp_gw *hfp, hfp_debug_func_t callback,
				void *user_data, hfp_destroy_func_t destroy)
{
	if (!hfp)
		return false;

	if (hfp->debug_destroy)
		hfp->debug_destroy(hfp->debug_data);

	hfp->debug_callback = callback;
	hfp->debug_destroy = destroy;
	hfp->debug_data = user_data;

	if (hfp->debug_callback) {
		ringbuf_set_input_tracing(hfp->read_buf, read_tracing, hfp);
		ringbuf_set_input_tracing(hfp->write_buf, write_tracing, hfp);
	} else {
		ringbuf_set_input_tracing(hfp->read_buf, NULL, NULL);
		ringbuf_set_input_tracing(hfp->write_buf, NULL, NULL);
	}

	return true;
}

bool hfp_gw_set_close_on_unref(struct hfp_gw *hfp, bool do_close)
{
	if (!hfp)
		return false;

	hfp->close_on_unref = do_close;

	return true;
}

bool hfp_gw_send_result(struct hfp_gw *hfp, enum hfp_result result)
{
	const char *str;

	if (!hfp)
		return false;

	switch (result) {
	case HFP_RESULT_OK:
		str = "OK";
		break;
	case HFP_RESULT_ERROR:
		str = "ERROR";
		break;
	case HFP_RESULT_RING:
	case HFP_RESULT_NO_CARRIER:
	case HFP_RESULT_BUSY:
	case HFP_RESULT_NO_ANSWER:
	case HFP_RESULT_DELAYED:
	case HFP_RESULT_BLACKLISTED:
	case HFP_RESULT_CME_ERROR:
	case HFP_RESULT_NO_DIALTONE:
	case HFP_RESULT_CONNECT:
	default:
		return false;
	}

	if (ringbuf_printf(hfp->write_buf, "\r\n%s\r\n", str) < 0)
		return false;

	wakeup_writer(hfp);

	/*
	 * There might be already something to read in the ring buffer.
	 * If so, let's read it.
	 */
	if (hfp->result_pending) {
		hfp->result_pending = false;
		process_input(hfp);
	}

	return true;
}

bool hfp_gw_send_error(struct hfp_gw *hfp, enum hfp_error error)
{
	if (!hfp)
		return false;

	if (ringbuf_printf(hfp->write_buf, "\r\n+CME ERROR: %u\r\n", error) < 0)
		return false;

	wakeup_writer(hfp);

	/*
	 * There might be already something to read in the ring buffer.
	 * If so, let's read it.
	 */
	if (hfp->result_pending) {
		hfp->result_pending = false;
		process_input(hfp);
	}

	return true;
}

bool hfp_gw_send_info(struct hfp_gw *hfp, const char *format, ...)
{
	va_list ap;
	char *fmt;
	int len;

	if (!hfp || !format)
		return false;

	if (asprintf(&fmt, "\r\n%s\r\n", format) < 0)
		return false;

	va_start(ap, format);
	len = ringbuf_vprintf(hfp->write_buf, fmt, ap);
	va_end(ap);

	free(fmt);

	if (len < 0)
		return false;

	if (hfp->result_pending)
		return true;

	wakeup_writer(hfp);

	return true;
}

bool hfp_gw_set_command_handler(struct hfp_gw *hfp,
				hfp_command_func_t callback,
				void *user_data, hfp_destroy_func_t destroy)
{
	if (!hfp)
		return false;

	if (hfp->command_destroy)
		hfp->command_destroy(hfp->command_data);

	hfp->command_callback = callback;
	hfp->command_destroy = destroy;
	hfp->command_data = user_data;

	return true;
}

bool hfp_gw_register(struct hfp_gw *hfp, hfp_result_func_t callback,
						const char *prefix,
						void *user_data,
						hfp_destroy_func_t destroy)
{
	struct cmd_handler *handler;

	handler = new0(struct cmd_handler, 1);
	handler->callback = callback;
	handler->user_data = user_data;

	handler->prefix = strdup(prefix);
	if (!handler->prefix) {
		free(handler);
		return false;
	}

	if (queue_find(hfp->cmd_handlers, match_handler_prefix,
							handler->prefix)) {
		destroy_cmd_handler(handler);
		return false;
	}

	handler->destroy = destroy;

	return queue_push_tail(hfp->cmd_handlers, handler);
}

bool hfp_gw_unregister(struct hfp_gw *hfp, const char *prefix)
{
	struct cmd_handler *handler;
	char *lookup_prefix;

	lookup_prefix = strdup(prefix);
	if (!lookup_prefix)
		return false;

	handler = queue_remove_if(hfp->cmd_handlers, match_handler_prefix,
								lookup_prefix);
	free(lookup_prefix);

	if (!handler)
		return false;

	destroy_cmd_handler(handler);

	return true;
}

static void disconnect_watch_destroy(void *user_data)
{
	struct hfp_gw *hfp = user_data;

	if (hfp->disconnect_destroy)
		hfp->disconnect_destroy(hfp->disconnect_data);

	if (hfp->destroyed)
		free(hfp);
}

static bool io_disconnected(struct io *io, void *user_data)
{
	struct hfp_gw *hfp = user_data;

	hfp->in_disconnect = true;

	if (hfp->disconnect_callback)
		hfp->disconnect_callback(hfp->disconnect_data);

	hfp->in_disconnect = false;

	return false;
}

bool hfp_gw_set_disconnect_handler(struct hfp_gw *hfp,
					hfp_disconnect_func_t callback,
					void *user_data,
					hfp_destroy_func_t destroy)
{
	if (!hfp)
		return false;

	if (hfp->disconnect_destroy)
		hfp->disconnect_destroy(hfp->disconnect_data);

	if (!io_set_disconnect_handler(hfp->io, io_disconnected, hfp,
						disconnect_watch_destroy)) {
		hfp->disconnect_callback = NULL;
		hfp->disconnect_destroy = NULL;
		hfp->disconnect_data = NULL;
		return false;
	}

	hfp->disconnect_callback = callback;
	hfp->disconnect_destroy = destroy;
	hfp->disconnect_data = user_data;

	return true;
}

bool hfp_gw_disconnect(struct hfp_gw *hfp)
{
	if (!hfp)
		return false;

	return io_shutdown(hfp->io);
}

static bool match_handler_event_prefix(const void *a, const void *b)
{
	const struct event_handler *handler = a;
	const char *prefix = b;

	if (strcmp(handler->prefix, prefix) != 0)
		return false;

	return true;
}

static void destroy_event_handler(void *data)
{
	struct event_handler *handler = data;

	if (handler->destroy)
		handler->destroy(handler->user_data);

	free(handler->prefix);

	free(handler);
}

static bool hf_can_write_data(struct io *io, void *user_data)
{
	struct hfp_hf *hfp = user_data;
	ssize_t bytes_written;

	bytes_written = ringbuf_write(hfp->write_buf, hfp->fd);
	if (bytes_written < 0)
		return false;

	if (ringbuf_len(hfp->write_buf) > 0)
		return true;

	return false;
}

static void hf_write_watch_destroy(void *user_data)
{
	struct hfp_hf *hfp = user_data;

	hfp->writer_active = false;
}

static void hf_skip_whitespace(struct hfp_context *context)
{
	while (context->data[context->offset] == ' ')
		context->offset++;
}

static bool is_response(const char *prefix, enum hfp_result *result,
						enum hfp_error *cme_err,
						struct hfp_context *context)
{
	if (strcmp(prefix, "OK") == 0) {
		*result = HFP_RESULT_OK;
		/*
		 * Set cme_err to 0 as this is not valid when result is not
		 * CME ERROR
		 */
		*cme_err = 0;
		return true;
	}

	if (strcmp(prefix, "ERROR") == 0) {
		*result = HFP_RESULT_ERROR;
		*cme_err = 0;
		return true;
	}

	if (strcmp(prefix, "NO CARRIER") == 0) {
		*result = HFP_RESULT_NO_CARRIER;
		*cme_err = 0;
		return true;
	}

	if (strcmp(prefix, "NO ANSWER") == 0) {
		*result = HFP_RESULT_NO_ANSWER;
		*cme_err = 0;
		return true;
	}

	if (strcmp(prefix, "BUSY") == 0) {
		*result = HFP_RESULT_BUSY;
		*cme_err = 0;
		return true;
	}

	if (strcmp(prefix, "DELAYED") == 0) {
		*result = HFP_RESULT_DELAYED;
		*cme_err = 0;
		return true;
	}

	if (strcmp(prefix, "BLACKLISTED") == 0) {
		*result = HFP_RESULT_BLACKLISTED;
		*cme_err = 0;
		return true;
	}

	if (strcmp(prefix, "+CME ERROR") == 0) {
		uint32_t val;

		*result = HFP_RESULT_CME_ERROR;

		if (hfp_context_get_number(context, &val) &&
					val <= HFP_ERROR_NETWORK_NOT_ALLOWED)
			*cme_err = val;
		else
			*cme_err = HFP_ERROR_AG_FAILURE;

		return true;
	}

	return false;
}

static void hf_wakeup_writer(struct hfp_hf *hfp)
{
	if (hfp->writer_active)
		return;

	if (!ringbuf_len(hfp->write_buf))
		return;

	if (!io_set_write_handler(hfp->io, hf_can_write_data,
					hfp, hf_write_watch_destroy))
		return;

	hfp->writer_active = true;
}

static void hf_call_prefix_handler(struct hfp_hf *hfp, const char *data)
{
	struct event_handler *handler;
	const char *separators = ";:\0";
	struct hfp_context context;
	enum hfp_result result;
	enum hfp_error cme_err;
	char lookup_prefix[18];
	uint8_t pref_len = 0;
	const char *prefix;
	int i;

	context.offset = 0;
	context.data = data;

	hf_skip_whitespace(&context);

	if (strlen(data + context.offset) < 2)
		return;

	prefix = data + context.offset;

	pref_len = strcspn(prefix, separators);
	if (pref_len > 17 || pref_len < 2)
		return;

	for (i = 0; i < pref_len; i++)
		lookup_prefix[i] = toupper(prefix[i]);

	lookup_prefix[pref_len] = '\0';
	context.offset += pref_len + 1;

	if (is_response(lookup_prefix, &result, &cme_err, &context)) {
		struct cmd_response *cmd;

		cmd = queue_peek_head(hfp->cmd_queue);
		if (!cmd)
			return;

		cmd->resp_cb(result, cme_err, cmd->user_data);

		queue_remove(hfp->cmd_queue, cmd);
		free(cmd);

		hf_wakeup_writer(hfp);
		return;
	}

	handler = queue_find(hfp->event_handlers, match_handler_event_prefix,
								lookup_prefix);
	if (!handler)
		return;

	handler->callback(&context, handler->user_data);
}

static char *find_cr_lf(char *str, size_t len)
{
	char *ptr;
	size_t count, offset;

	offset = 0;

	ptr = memchr(str, '\r', len);
	while (ptr) {
		/*
		 * Check if there is more data after '\r'. If so check for
		 * '\n'
		 */
		count = ptr - str;
		if ((count < (len - 1)) && *(ptr + 1) == '\n')
			return ptr;

		/* There is only '\r'? Let's try to find next one */
		offset += count + 1;

		if (offset >= len)
			return NULL;

		ptr = memchr(str + offset, '\r', len - offset);
	}

	return NULL;
}

static void hf_process_input(struct hfp_hf *hfp)
{
	char *str, *ptr, *str2, *tmp;
	size_t len, count, offset, len2;
	bool free_tmp = false;

	str = ringbuf_peek(hfp->read_buf, 0, &len);
	if (!str)
		return;

	offset = 0;

	ptr = find_cr_lf(str, len);
	while (ptr) {
		count = ptr - (str + offset);
		if (count == 0) {
			/* 2 is for <cr><lf> */
			offset += 2;
		} else {
			*ptr = '\0';
			hf_call_prefix_handler(hfp, str + offset);
			offset += count + 2;
		}

		ptr = find_cr_lf(str + offset, len - offset);
	}

	/*
	 * Just check if there is no wrapped data in ring buffer.
	 * Should not happen too often
	 */
	if (len == ringbuf_len(hfp->read_buf))
		goto done;

	str2 = ringbuf_peek(hfp->read_buf, len, &len2);
	if (!str2)
		goto done;

	ptr = find_cr_lf(str2, len2);
	if (!ptr) {
		/* Might happen that we wrap between \r and \n */
		ptr = memchr(str2, '\n', len2);
		if (!ptr)
			goto done;
	}

	count = ptr - str2;

	if (count) {
		*ptr = '\0';

		tmp = malloc(len + count);
		if (!tmp)
			goto done;

		/* "str" here is not a string so we need to use memcpy */
		memcpy(tmp, str, len);
		memcpy(tmp + len, str2, count);

		free_tmp = true;
	} else {
		str[len-1] = '\0';
		tmp = str;
	}

	hf_call_prefix_handler(hfp, tmp);
	offset += count;

done:
	ringbuf_drain(hfp->read_buf, offset);

	if (free_tmp)
		free(tmp);
}

static bool hf_can_read_data(struct io *io, void *user_data)
{
	struct hfp_hf *hfp = user_data;
	ssize_t bytes_read;

	bytes_read = ringbuf_read(hfp->read_buf, hfp->fd);
	if (bytes_read < 0)
		return false;

	hf_process_input(hfp);

	return true;
}

struct hfp_hf *hfp_hf_new(int fd)
{
	struct hfp_hf *hfp;

	if (fd < 0)
		return NULL;

	hfp = new0(struct hfp_hf, 1);
	hfp->fd = fd;
	hfp->close_on_unref = false;

	hfp->read_buf = ringbuf_new(4096);
	if (!hfp->read_buf) {
		free(hfp);
		return NULL;
	}

	hfp->write_buf = ringbuf_new(4096);
	if (!hfp->write_buf) {
		ringbuf_free(hfp->read_buf);
		free(hfp);
		return NULL;
	}

	hfp->io = io_new(fd);
	if (!hfp->io) {
		ringbuf_free(hfp->write_buf);
		ringbuf_free(hfp->read_buf);
		free(hfp);
		return NULL;
	}

	hfp->event_handlers = queue_new();
	hfp->cmd_queue = queue_new();
	hfp->writer_active = false;

	if (!io_set_read_handler(hfp->io, hf_can_read_data, hfp,
							read_watch_destroy)) {
		queue_destroy(hfp->event_handlers,
						destroy_event_handler);
		io_destroy(hfp->io);
		ringbuf_free(hfp->write_buf);
		ringbuf_free(hfp->read_buf);
		free(hfp);
		return NULL;
	}

	return hfp_hf_ref(hfp);
}

struct hfp_hf *hfp_hf_ref(struct hfp_hf *hfp)
{
	if (!hfp)
		return NULL;

	__sync_fetch_and_add(&hfp->ref_count, 1);

	return hfp;
}

void hfp_hf_unref(struct hfp_hf *hfp)
{
	if (!hfp)
		return;

	if (__sync_sub_and_fetch(&hfp->ref_count, 1))
		return;

	io_set_write_handler(hfp->io, NULL, NULL, NULL);
	io_set_read_handler(hfp->io, NULL, NULL, NULL);
	io_set_disconnect_handler(hfp->io, NULL, NULL, NULL);

	io_destroy(hfp->io);
	hfp->io = NULL;

	if (hfp->close_on_unref)
		close(hfp->fd);

	hfp_hf_set_debug(hfp, NULL, NULL, NULL);

	ringbuf_free(hfp->read_buf);
	hfp->read_buf = NULL;

	ringbuf_free(hfp->write_buf);
	hfp->write_buf = NULL;

	queue_destroy(hfp->event_handlers, destroy_event_handler);
	hfp->event_handlers = NULL;

	queue_destroy(hfp->cmd_queue, free);
	hfp->cmd_queue = NULL;

	if (!hfp->in_disconnect) {
		free(hfp);
		return;
	}

	hfp->destroyed = true;
}

static void hf_read_tracing(const void *buf, size_t count,
							void *user_data)
{
	struct hfp_hf *hfp = user_data;

	util_hexdump('>', buf, count, hfp->debug_callback, hfp->debug_data);
}

static void hf_write_tracing(const void *buf, size_t count,
							void *user_data)
{
	struct hfp_hf *hfp = user_data;

	util_hexdump('<', buf, count, hfp->debug_callback, hfp->debug_data);
}

bool hfp_hf_set_debug(struct hfp_hf *hfp, hfp_debug_func_t callback,
				void *user_data, hfp_destroy_func_t destroy)
{
	if (!hfp)
		return false;

	if (hfp->debug_destroy)
		hfp->debug_destroy(hfp->debug_data);

	hfp->debug_callback = callback;
	hfp->debug_destroy = destroy;
	hfp->debug_data = user_data;

	if (hfp->debug_callback) {
		ringbuf_set_input_tracing(hfp->read_buf, hf_read_tracing, hfp);
		ringbuf_set_input_tracing(hfp->write_buf, hf_write_tracing,
									hfp);
	} else {
		ringbuf_set_input_tracing(hfp->read_buf, NULL, NULL);
		ringbuf_set_input_tracing(hfp->write_buf, NULL, NULL);
	}

	return true;
}

bool hfp_hf_set_close_on_unref(struct hfp_hf *hfp, bool do_close)
{
	if (!hfp)
		return false;

	hfp->close_on_unref = do_close;

	return true;
}

bool hfp_hf_send_command(struct hfp_hf *hfp, hfp_response_func_t resp_cb,
				void *user_data, const char *format, ...)
{
	va_list ap;
	char *fmt;
	int len;
	struct cmd_response *cmd;

	if (!hfp || !format || !resp_cb)
		return false;

	if (asprintf(&fmt, "%s\r", format) < 0)
		return false;

	cmd = new0(struct cmd_response, 1);

	va_start(ap, format);
	len = ringbuf_vprintf(hfp->write_buf, fmt, ap);
	va_end(ap);

	free(fmt);

	if (len < 0) {
		free(cmd);
		return false;
	}

	cmd->resp_cb = resp_cb;
	cmd->user_data = user_data;

	if (!queue_push_tail(hfp->cmd_queue, cmd)) {
		ringbuf_drain(hfp->write_buf, len);
		free(cmd);
		return false;
	}

	hf_wakeup_writer(hfp);

	return true;
}

bool hfp_hf_register(struct hfp_hf *hfp, hfp_hf_result_func_t callback,
						const char *prefix,
						void *user_data,
						hfp_destroy_func_t destroy)
{
	struct event_handler *handler;

	if (!callback)
		return false;

	handler = new0(struct event_handler, 1);
	handler->callback = callback;
	handler->user_data = user_data;

	handler->prefix = strdup(prefix);
	if (!handler->prefix) {
		free(handler);
		return false;
	}

	if (queue_find(hfp->event_handlers, match_handler_event_prefix,
							handler->prefix)) {
		destroy_event_handler(handler);
		return false;
	}

	handler->destroy = destroy;

	return queue_push_tail(hfp->event_handlers, handler);
}

bool hfp_hf_unregister(struct hfp_hf *hfp, const char *prefix)
{
	struct cmd_handler *handler;

	/* Cast to void as queue_remove needs that */
	handler = queue_remove_if(hfp->event_handlers,
						match_handler_event_prefix,
						(void *) prefix);

	if (!handler)
		return false;

	destroy_event_handler(handler);

	return true;
}

static void hf_disconnect_watch_destroy(void *user_data)
{
	struct hfp_hf *hfp = user_data;

	if (hfp->disconnect_destroy)
		hfp->disconnect_destroy(hfp->disconnect_data);

	if (hfp->destroyed)
		free(hfp);
}

static bool hf_io_disconnected(struct io *io, void *user_data)
{
	struct hfp_hf *hfp = user_data;

	hfp->in_disconnect = true;

	if (hfp->disconnect_callback)
		hfp->disconnect_callback(hfp->disconnect_data);

	hfp->in_disconnect = false;

	return false;
}

bool hfp_hf_set_disconnect_handler(struct hfp_hf *hfp,
						hfp_disconnect_func_t callback,
						void *user_data,
						hfp_destroy_func_t destroy)
{
	if (!hfp)
		return false;

	if (hfp->disconnect_destroy)
		hfp->disconnect_destroy(hfp->disconnect_data);

	if (!io_set_disconnect_handler(hfp->io, hf_io_disconnected, hfp,
						hf_disconnect_watch_destroy)) {
		hfp->disconnect_callback = NULL;
		hfp->disconnect_destroy = NULL;
		hfp->disconnect_data = NULL;
		return false;
	}

	hfp->disconnect_callback = callback;
	hfp->disconnect_destroy = destroy;
	hfp->disconnect_data = user_data;

	return true;
}

bool hfp_hf_disconnect(struct hfp_hf *hfp)
{
	if (!hfp)
		return false;

	return io_shutdown(hfp->io);
}
