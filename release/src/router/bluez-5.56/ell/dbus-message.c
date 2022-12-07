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

#include "util.h"
#include "private.h"
#include "dbus.h"
#include "dbus-private.h"
#include "gvariant-private.h"

#define DBUS_MESSAGE_LITTLE_ENDIAN	('l')
#define DBUS_MESSAGE_BIG_ENDIAN		('B')

#define DBUS_MESSAGE_PROTOCOL_VERSION	1

#define DBUS_MESSAGE_FLAG_NO_REPLY_EXPECTED	0x01
#define DBUS_MESSAGE_FLAG_NO_AUTO_START		0x02

#define DBUS_MESSAGE_FIELD_PATH		1
#define DBUS_MESSAGE_FIELD_INTERFACE	2
#define DBUS_MESSAGE_FIELD_MEMBER	3
#define DBUS_MESSAGE_FIELD_ERROR_NAME	4
#define DBUS_MESSAGE_FIELD_REPLY_SERIAL	5
#define DBUS_MESSAGE_FIELD_DESTINATION	6
#define DBUS_MESSAGE_FIELD_SENDER	7
#define DBUS_MESSAGE_FIELD_SIGNATURE	8
#define DBUS_MESSAGE_FIELD_UNIX_FDS	9

#define DBUS_MAX_NESTING	32

struct l_dbus_message {
	int refcount;
	void *header;
	size_t header_size;
	size_t header_end;
	char *signature;
	void *body;
	size_t body_size;
	char *path;
	char *interface;
	char *member;
	char *error_name;
	uint32_t reply_serial;
	char *destination;
	char *sender;
	int fds[16];
	uint32_t num_fds;

	bool sealed : 1;
	bool signature_free : 1;
};

struct l_dbus_message_builder {
	struct l_dbus_message *message;
	struct dbus_builder *builder;
	struct builder_driver *driver;
};

static inline bool _dbus_message_is_gvariant(struct l_dbus_message *msg)
{
	struct dbus_header *hdr = msg->header;

	return hdr->version == 2;
}

void *_dbus_message_get_header(struct l_dbus_message *msg, size_t *out_size)
{
	if (out_size)
		*out_size = msg->header_size;

	return msg->header;
}

void *_dbus_message_get_body(struct l_dbus_message *msg, size_t *out_size)
{
	if (out_size)
		*out_size = msg->body_size;

	return msg->body;
}

/* Get a buffer containing the final message contents except the header */
void *_dbus_message_get_footer(struct l_dbus_message *msg, size_t *out_size)
{
	size_t size;

	if (_dbus_message_is_gvariant(msg)) {
		size = _gvariant_message_finalize(msg->header_end,
						msg->body, msg->body_size,
						msg->signature);
		size -= msg->header_size;
	} else
		size = msg->body_size;

	if (out_size)
		*out_size = size;

	return msg->body;
}

int *_dbus_message_get_fds(struct l_dbus_message *msg, uint32_t *num_fds)
{
	*num_fds = msg->num_fds;

	return msg->fds;
}

void _dbus_message_set_serial(struct l_dbus_message *msg, uint32_t serial)
{
	struct dbus_header *hdr = msg->header;

	hdr->dbus1.serial = serial;
}

uint32_t _dbus_message_get_serial(struct l_dbus_message *msg)
{
	struct dbus_header *hdr = msg->header;

	return hdr->dbus1.serial;
}

LIB_EXPORT bool l_dbus_message_set_no_reply(struct l_dbus_message *msg, bool on)
{
	struct dbus_header *hdr;

	if (unlikely(!msg))
		return false;

	hdr = msg->header;

	if (on)
		hdr->flags |= DBUS_MESSAGE_FLAG_NO_REPLY_EXPECTED;
	else
		hdr->flags &= ~DBUS_MESSAGE_FLAG_NO_REPLY_EXPECTED;

	return true;
}

LIB_EXPORT bool l_dbus_message_get_no_reply(struct l_dbus_message *msg)
{
	struct dbus_header *hdr;

	if (unlikely(!msg))
		return false;

	hdr = msg->header;

	if (hdr->flags & DBUS_MESSAGE_FLAG_NO_REPLY_EXPECTED)
		return true;

	return false;
}

LIB_EXPORT bool l_dbus_message_set_no_autostart(struct l_dbus_message *msg,
							bool on)
{
	struct dbus_header *hdr;

	if (unlikely(!msg))
		return false;

	hdr = msg->header;

	if (on)
		hdr->flags |= DBUS_MESSAGE_FLAG_NO_AUTO_START;
	else
		hdr->flags &= ~DBUS_MESSAGE_FLAG_NO_AUTO_START;

	return true;
}

LIB_EXPORT bool l_dbus_message_get_no_autostart(struct l_dbus_message *msg)
{
	struct dbus_header *hdr;

	if (unlikely(!msg))
		return false;

	hdr = msg->header;

	if (hdr->flags & DBUS_MESSAGE_FLAG_NO_AUTO_START)
		return true;

	return false;

}

static struct l_dbus_message *message_new_common(uint8_t type, uint8_t flags,
						uint8_t version)
{
	struct l_dbus_message *message;
	struct dbus_header *hdr;

	message = l_new(struct l_dbus_message, 1);
	message->refcount = 1;

	/*
	 * We allocate the header with the initial 12 bytes (up to the field
	 * length) so that we can store the basic information here.  For
	 * GVariant we need 16 bytes.
	 */
	message->header_size = version == 1 ? 12 : 16;
	message->header_end = message->header_size;
	message->header = l_realloc(NULL, message->header_size);

	hdr = message->header;
	hdr->endian = DBUS_NATIVE_ENDIAN;
	hdr->message_type = type;
	hdr->flags = flags;
	hdr->version = version;

	return message;
}

struct l_dbus_message *_dbus_message_new_method_call(uint8_t version,
							const char *destination,
							const char *path,
							const char *interface,
							const char *method)
{
	struct l_dbus_message *message;

	message = message_new_common(DBUS_MESSAGE_TYPE_METHOD_CALL, 0, version);

	message->destination = l_strdup(destination);
	message->path = l_strdup(path);
	message->interface = l_strdup(interface);
	message->member = l_strdup(method);

	return message;
}

LIB_EXPORT struct l_dbus_message *l_dbus_message_new_method_call(
							struct l_dbus *dbus,
							const char *destination,
							const char *path,
							const char *interface,
							const char *method)
{
	uint8_t version;

	if (unlikely(!dbus))
		return NULL;

	version = _dbus_get_version(dbus);

	return _dbus_message_new_method_call(version, destination, path,
						interface, method);
}

struct l_dbus_message *_dbus_message_new_signal(uint8_t version,
						const char *path,
						const char *interface,
						const char *name)
{
	struct l_dbus_message *message;

	message = message_new_common(DBUS_MESSAGE_TYPE_SIGNAL,
					DBUS_MESSAGE_FLAG_NO_REPLY_EXPECTED,
					version);

	message->path = l_strdup(path);
	message->interface = l_strdup(interface);
	message->member = l_strdup(name);

	return message;
}

LIB_EXPORT struct l_dbus_message *l_dbus_message_new_signal(struct l_dbus *dbus,
							const char *path,
							const char *interface,
							const char *name)
{
	uint8_t version;

	if (unlikely(!dbus))
		return NULL;

	version = _dbus_get_version(dbus);

	return _dbus_message_new_signal(version, path, interface, name);
}

LIB_EXPORT struct l_dbus_message *l_dbus_message_new_method_return(
					struct l_dbus_message *method_call)
{
	struct l_dbus_message *message;
	struct dbus_header *hdr = method_call->header;
	const char *sender;

	message = message_new_common(DBUS_MESSAGE_TYPE_METHOD_RETURN,
					DBUS_MESSAGE_FLAG_NO_REPLY_EXPECTED,
					hdr->version);

	if (!l_dbus_message_get_no_reply(method_call))
		message->reply_serial = _dbus_message_get_serial(method_call);

	sender = l_dbus_message_get_sender(method_call);
	if (sender)
		message->destination = l_strdup(sender);

	return message;
}

struct l_dbus_message *_dbus_message_new_error(uint8_t version,
						uint32_t reply_serial,
						const char *destination,
						const char *name,
						const char *error)
{
	struct l_dbus_message *reply;

	if (!_dbus_valid_interface(name))
		return NULL;

	reply = message_new_common(DBUS_MESSAGE_TYPE_ERROR,
					DBUS_MESSAGE_FLAG_NO_REPLY_EXPECTED,
					version);

	reply->error_name = l_strdup(name);
	reply->destination = l_strdup(destination);
	reply->reply_serial = reply_serial;

	if (!l_dbus_message_set_arguments(reply, "s", error)) {
		l_dbus_message_unref(reply);
		return NULL;
	}

	return reply;
}

LIB_EXPORT struct l_dbus_message *l_dbus_message_new_error_valist(
					struct l_dbus_message *method_call,
					const char *name,
					const char *format, va_list args)
{
	char str[1024];
	struct dbus_header *hdr = method_call->header;
	uint32_t reply_serial = 0;

	vsnprintf(str, sizeof(str), format, args);

	if (!l_dbus_message_get_no_reply(method_call))
		reply_serial = _dbus_message_get_serial(method_call);

	return _dbus_message_new_error(hdr->version, reply_serial,
					l_dbus_message_get_sender(method_call),
					name, str);
}

LIB_EXPORT struct l_dbus_message *l_dbus_message_new_error(
					struct l_dbus_message *method_call,
					const char *name,
					const char *format, ...)
{
	va_list args;
	struct l_dbus_message *reply;

	va_start(args, format);
	reply = l_dbus_message_new_error_valist(method_call, name,
								format, args);
	va_end(args);

	return reply;
}

LIB_EXPORT struct l_dbus_message *l_dbus_message_ref(struct l_dbus_message *message)
{
	if (unlikely(!message))
		return NULL;

	__atomic_fetch_add(&message->refcount, 1, __ATOMIC_SEQ_CST);

	return message;
}

LIB_EXPORT void l_dbus_message_unref(struct l_dbus_message *message)
{
	unsigned int i;

	if (unlikely(!message))
		return;

	if (__atomic_sub_fetch(&message->refcount, 1, __ATOMIC_SEQ_CST))
		return;

	for (i = 0; i < message->num_fds; i++)
		close(message->fds[i]);

	if (!message->sealed) {
		l_free(message->destination);
		l_free(message->path);
		l_free(message->interface);
		l_free(message->member);
		l_free(message->error_name);
		l_free(message->sender);
	}

	if (message->signature_free)
		l_free(message->signature);

	l_free(message->header);
	l_free(message->body);
	l_free(message);
}

const char *_dbus_message_get_nth_string_argument(
					struct l_dbus_message *message, int n)
{
	struct l_dbus_message_iter iter;
	const char *signature, *value;
	void *body;
	size_t size;
	char type;
	bool (*skip_entry)(struct l_dbus_message_iter *);
	bool (*get_basic)(struct l_dbus_message_iter *, char, void *);

	signature = l_dbus_message_get_signature(message);
	body = _dbus_message_get_body(message, &size);

	if (!signature)
		return NULL;

	if (_dbus_message_is_gvariant(message)) {
		if (!_gvariant_iter_init(&iter, message, signature, NULL,
						body, size))
			return NULL;

		skip_entry = _gvariant_iter_skip_entry;
		get_basic = _gvariant_iter_next_entry_basic;
	} else {
		_dbus1_iter_init(&iter, message, signature, NULL, body, size);

		skip_entry = _dbus1_iter_skip_entry;
		get_basic = _dbus1_iter_next_entry_basic;
	}

	while (n--)
		if (!skip_entry(&iter))
			return NULL;

	if (!iter.sig_start)
		return NULL;

	type = iter.sig_start[iter.sig_pos];
	if (!strchr("sog", type))
		return NULL;

	if (!get_basic(&iter, type, &value))
		return NULL;

	return value;
}

static bool message_iter_next_entry_valist(struct l_dbus_message_iter *orig,
						va_list args)
{
	static const char *simple_types = "sogybnqiuxtd";
	struct l_dbus_message_iter *iter = orig;
	const char *signature = orig->sig_start + orig->sig_pos;
	const char *end;
	struct l_dbus_message_iter *sub_iter;
	struct l_dbus_message_iter stack[DBUS_MAX_NESTING];
	unsigned int indent = 0;
	uint32_t uint32_val;
	int fd;
	void *arg;
	bool (*get_basic)(struct l_dbus_message_iter *, char ,void *);
	bool (*enter_struct)(struct l_dbus_message_iter *,
				struct l_dbus_message_iter *);
	bool (*enter_array)(struct l_dbus_message_iter *,
				struct l_dbus_message_iter *);
	bool (*enter_variant)(struct l_dbus_message_iter *,
				struct l_dbus_message_iter *);

	if (_dbus_message_is_gvariant(orig->message)) {
		get_basic = _gvariant_iter_next_entry_basic;
		enter_struct = _gvariant_iter_enter_struct;
		enter_array = _gvariant_iter_enter_array;
		enter_variant = _gvariant_iter_enter_variant;
	} else {
		get_basic = _dbus1_iter_next_entry_basic;
		enter_struct = _dbus1_iter_enter_struct;
		enter_array = _dbus1_iter_enter_array;
		enter_variant = _dbus1_iter_enter_variant;
	}

	while (signature < orig->sig_start + orig->sig_len) {
		if (strchr(simple_types, *signature)) {
			arg = va_arg(args, void *);
			if (!get_basic(iter, *signature, arg))
				return false;

			signature += 1;
			continue;
		}

		switch (*signature) {
		case 'h':
			if (!get_basic(iter, 'h', &uint32_val))
				return false;

			if (uint32_val < iter->message->num_fds)
				fd = fcntl(iter->message->fds[uint32_val],
						F_DUPFD_CLOEXEC, 3);
			else
				fd = -1;

			*va_arg(args, int *) = fd;
			signature += 1;
			break;
		case '(':
		case '{':
			signature += 1;
			indent += 1;

			if (indent > DBUS_MAX_NESTING)
				return false;

			if (!enter_struct(iter, &stack[indent - 1]))
				return false;

			iter = &stack[indent - 1];

			break;
		case ')':
		case '}':
			/*
			 * Sanity check in case of an unmatched paren/brace
			 * that isn't caught elsewhere.
			 */
			if (unlikely(indent == 0))
				return false;

			signature += 1;
			indent -= 1;

			if (indent == 0)
				iter = orig;
			else
				iter = &stack[indent - 1];
			break;
		case 'a':
			sub_iter = va_arg(args, void *);

			if (!enter_array(iter, sub_iter))
				return false;

			end = _dbus_signature_end(signature + 1);
			signature = end + 1;
			break;
		case 'v':
			sub_iter = va_arg(args, void *);

			if (!enter_variant(iter, sub_iter))
				return false;

			signature += 1;
			break;
		default:
			return false;
		}
	}

	return true;
}

static inline bool message_iter_next_entry(struct l_dbus_message_iter *iter,
						...)
{
	va_list args;
	bool result;

        va_start(args, iter);
	result = message_iter_next_entry_valist(iter, args);
	va_end(args);

	return result;
}

static bool get_header_field_from_iter_valist(struct l_dbus_message *message,
						uint8_t type, char data_type,
						va_list args)
{
	struct l_dbus_message_iter header;
	struct l_dbus_message_iter array, iter;
	uint8_t endian, message_type, flags, version;
	uint32_t body_length, serial;
	bool found;

	if (!message->sealed)
		return false;

	if (_dbus_message_is_gvariant(message)) {
		uint64_t field_type;

		if (!_gvariant_iter_init(&header, message, "a(tv)", NULL,
						message->header + 16,
						message->header_end - 16))
			return false;

		if (!_gvariant_iter_enter_array(&header, &array))
			return false;

		while ((found = message_iter_next_entry(&array,
							&field_type, &iter)))
			if (field_type == type)
				break;
	} else {
		uint8_t field_type;

		_dbus1_iter_init(&header, message, "yyyyuua(yv)", NULL,
				message->header, message->header_size);

		if (!message_iter_next_entry(&header, &endian,
						&message_type, &flags, &version,
						&body_length, &serial, &array))
			return false;

		while ((found = message_iter_next_entry(&array,
							&field_type, &iter)))
			if (field_type == type)
				break;
	}

	if (!found)
		return false;

	if (iter.sig_start[iter.sig_pos] != data_type)
		return false;

	return message_iter_next_entry_valist(&iter, args);
}

static inline bool get_header_field(struct l_dbus_message *message,
					uint8_t type, int data_type, ...)
{
	va_list args;
	bool result;

	va_start(args, data_type);
	result = get_header_field_from_iter_valist(message, type, data_type,
							args);
	va_end(args);

	return result;
}

static bool valid_header(const struct dbus_header *hdr)
{
	if (hdr->endian != DBUS_MESSAGE_LITTLE_ENDIAN &&
			hdr->endian != DBUS_MESSAGE_BIG_ENDIAN)
		return false;

	if (hdr->message_type < DBUS_MESSAGE_TYPE_METHOD_CALL ||
			hdr->message_type > DBUS_MESSAGE_TYPE_SIGNAL)
		return false;

	if (hdr->version != 1 && hdr->version != 2)
		return false;

	if (hdr->version == 1) {
		if (hdr->dbus1.serial == 0)
			return false;
	}

	return true;
}

unsigned int _dbus_message_unix_fds_from_header(const void *data, size_t size)
{
	struct l_dbus_message message;
	uint32_t unix_fds;

	message.header = (uint8_t *) data;
	message.header_size = size;
	message.body_size = 0;
	message.sealed = true;

	if (!get_header_field(&message, DBUS_MESSAGE_FIELD_UNIX_FDS,
				'u', &unix_fds))
		return 0;

	return unix_fds;
}

struct l_dbus_message *dbus_message_from_blob(const void *data, size_t size,
						int fds[], uint32_t num_fds)
{
	const struct dbus_header *hdr = data;
	struct l_dbus_message *message;
	size_t body_pos;
	unsigned int i;

	if (unlikely(size < DBUS_HEADER_SIZE))
		return NULL;

	message = l_new(struct l_dbus_message, 1);

	message->refcount = 1;

	if (hdr->version == 1) {
		message->header_size = align_len(DBUS_HEADER_SIZE +
						hdr->dbus1.field_length, 8);
		message->body_size = hdr->dbus1.body_length;

		if (message->header_size + message->body_size < size)
			goto free;

		body_pos = message->header_size;
	} else {
		struct l_dbus_message_iter iter;
		struct l_dbus_message_iter header, variant, body;

		/*
		 * GVariant message structure as per
		 * https://wiki.gnome.org/Projects/GLib/GDBus/Version2
		 * is "(yyyyuta{tv}v)".  As noted this is equivalent to
		 * some other types, this one lets us get iterators for
		 * the header and the body in the fewest steps.
		 */
		if (!_gvariant_iter_init(&iter, message, "(yyyyuta{tv})v",
						NULL, data, size))
			goto free;

		if (!_gvariant_iter_enter_struct(&iter, &header))
			goto free;

		if (!_gvariant_iter_enter_variant(&iter, &variant))
			goto free;

		if (!_gvariant_iter_enter_struct(&variant, &body))
			goto free;

		message->header_size = align_len(header.len - header.pos, 8);
		message->body_size = body.len - body.pos;
		message->signature = l_strndup(body.sig_start + body.sig_pos,
						body.sig_len - body.sig_pos);
		message->signature_free = true;
		message->header_end = header.len;
		body_pos = body.data + body.pos - data;
	}

	message->header = l_malloc(message->header_size);
	message->body = l_malloc(message->body_size);

	memcpy(message->header, data, message->header_size);
	memcpy(message->body, data + body_pos, message->body_size);

	message->sealed = true;

	/* If the field is absent message->signature will remain NULL */
	if (hdr->version == 1)
		get_header_field(message, DBUS_MESSAGE_FIELD_SIGNATURE,
					'g', &message->signature);

	if (num_fds) {
		uint32_t unix_fds, orig_fds = num_fds;

		if (!get_header_field(message, DBUS_MESSAGE_FIELD_UNIX_FDS,
					'u', &unix_fds))
			goto free;

		if (num_fds > unix_fds)
			num_fds = unix_fds;

		if (num_fds > L_ARRAY_SIZE(message->fds))
			num_fds = L_ARRAY_SIZE(message->fds);

		for (i = num_fds; i < orig_fds; i++)
			close(fds[i]);

		message->num_fds = num_fds;
		memcpy(message->fds, fds, num_fds * sizeof(int));
	}

	return message;

free:
	l_dbus_message_unref(message);

	return NULL;
}

struct l_dbus_message *dbus_message_build(void *header, size_t header_size,
						void *body, size_t body_size,
						int fds[], uint32_t num_fds)
{
	const struct dbus_header *hdr = header;
	struct l_dbus_message *message;
	unsigned int i;

	if (unlikely(header_size < DBUS_HEADER_SIZE))
		return NULL;

	if (unlikely(!valid_header(hdr)))
		return NULL;

	/*
	 * With GVariant we need to know the signature, use
	 * dbus_message_from_blob instead.
	 */
	if (unlikely(hdr->version != 1))
		return NULL;

	message = l_new(struct l_dbus_message, 1);

	message->refcount = 1;
	message->header_size = header_size;
	message->header = header;
	message->body_size = body_size;
	message->body = body;
	message->sealed = true;

	if (num_fds) {
		uint32_t unix_fds, orig_fds = num_fds;

		if (!get_header_field(message, DBUS_MESSAGE_FIELD_UNIX_FDS,
					'u', &unix_fds)) {
			l_free(message);
			return NULL;
		}

		if (num_fds > unix_fds)
			num_fds = unix_fds;

		if (num_fds > L_ARRAY_SIZE(message->fds))
			num_fds = L_ARRAY_SIZE(message->fds);

		for (i = num_fds; i < orig_fds; i++)
			close(fds[i]);

		message->num_fds = num_fds;
		memcpy(message->fds, fds, num_fds * sizeof(int));
	}

	/* If the field is absent message->signature will remain NULL */
	get_header_field(message, DBUS_MESSAGE_FIELD_SIGNATURE, 'g',
						&message->signature);

	return message;
}

bool dbus_message_compare(struct l_dbus_message *message,
					const void *data, size_t size)
{
	struct l_dbus_message *other;
	bool ret = false;

	other = dbus_message_from_blob(data, size, NULL, 0);

	if (message->signature) {
		if (!other->signature)
			goto done;

		if (strcmp(message->signature, other->signature))
			goto done;
	} else {
		if (other->signature)
			goto done;
	}

	if (message->body_size != other->body_size)
		goto done;

	if (message->header_size != other->header_size)
		goto done;

	ret = !memcmp(message->body, other->body, message->body_size);

done:
	l_dbus_message_unref(other);

	return ret;
}

struct builder_driver {
	bool (*append_basic)(struct dbus_builder *, char, const void *);
	bool (*enter_struct)(struct dbus_builder *, const char *);
	bool (*leave_struct)(struct dbus_builder *);
	bool (*enter_dict)(struct dbus_builder *, const char *);
	bool (*leave_dict)(struct dbus_builder *);
	bool (*enter_array)(struct dbus_builder *, const char *);
	bool (*leave_array)(struct dbus_builder *);
	bool (*enter_variant)(struct dbus_builder *, const char *);
	bool (*leave_variant)(struct dbus_builder *);
	char *(*finish)(struct dbus_builder *, void **, size_t *);
	bool (*mark)(struct dbus_builder *);
	bool (*rewind)(struct dbus_builder *);
	struct dbus_builder *(*new)(void *, size_t);
	void (*free)(struct dbus_builder *);
};

static struct builder_driver dbus1_driver = {
	.append_basic = _dbus1_builder_append_basic,
	.enter_struct = _dbus1_builder_enter_struct,
	.leave_struct = _dbus1_builder_leave_struct,
	.enter_dict = _dbus1_builder_enter_dict,
	.leave_dict = _dbus1_builder_leave_dict,
	.enter_variant = _dbus1_builder_enter_variant,
	.leave_variant = _dbus1_builder_leave_variant,
	.enter_array = _dbus1_builder_enter_array,
	.leave_array = _dbus1_builder_leave_array,
	.finish = _dbus1_builder_finish,
	.mark = _dbus1_builder_mark,
	.rewind = _dbus1_builder_rewind,
	.new = _dbus1_builder_new,
	.free = _dbus1_builder_free,
};

static struct builder_driver gvariant_driver = {
	.append_basic = _gvariant_builder_append_basic,
	.enter_struct = _gvariant_builder_enter_struct,
	.leave_struct = _gvariant_builder_leave_struct,
	.enter_dict = _gvariant_builder_enter_dict,
	.leave_dict = _gvariant_builder_leave_dict,
	.enter_variant = _gvariant_builder_enter_variant,
	.leave_variant = _gvariant_builder_leave_variant,
	.enter_array = _gvariant_builder_enter_array,
	.leave_array = _gvariant_builder_leave_array,
	.finish = _gvariant_builder_finish,
	.mark = _gvariant_builder_mark,
	.rewind = _gvariant_builder_rewind,
	.new = _gvariant_builder_new,
	.free = _gvariant_builder_free,
};

static void add_field(struct dbus_builder *builder,
			struct builder_driver *driver,
			uint8_t field, const char *type, const void *value)
{
	if (driver == &gvariant_driver) {
		uint64_t long_field = field;

		driver->enter_struct(builder, "tv");
		driver->append_basic(builder, 't', &long_field);
	} else {
		driver->enter_struct(builder, "yv");
		driver->append_basic(builder, 'y', &field);
	}
	driver->enter_variant(builder, type);
	driver->append_basic(builder, type[0], value);
	driver->leave_variant(builder);
	driver->leave_struct(builder);
}

static void build_header(struct l_dbus_message *message, const char *signature)
{
	struct dbus_builder *builder;
	struct builder_driver *driver;
	char *generated_signature;
	size_t header_size;
	bool gvariant;

	gvariant = _dbus_message_is_gvariant(message);

	if (gvariant)
		driver = &gvariant_driver;
	else
		driver = &dbus1_driver;

	builder = driver->new(message->header, message->header_size);

	driver->enter_array(builder, gvariant ? "(tv)" : "(yv)");

	if (message->path) {
		add_field(builder, driver, DBUS_MESSAGE_FIELD_PATH,
					"o", message->path);
		l_free(message->path);
		message->path = NULL;
	}

	if (message->member) {
		add_field(builder, driver, DBUS_MESSAGE_FIELD_MEMBER,
					"s", message->member);
		l_free(message->member);
		message->member = NULL;
	}

	if (message->interface) {
		add_field(builder, driver, DBUS_MESSAGE_FIELD_INTERFACE,
					"s", message->interface);
		l_free(message->interface);
		message->interface = NULL;
	}

	if (message->destination) {
		add_field(builder, driver, DBUS_MESSAGE_FIELD_DESTINATION,
					"s", message->destination);
		l_free(message->destination);
		message->destination = NULL;
	}

	if (message->error_name != 0) {
		add_field(builder, driver, DBUS_MESSAGE_FIELD_ERROR_NAME,
					"s", message->error_name);
		l_free(message->error_name);
		message->error_name = NULL;
	}

	if (message->reply_serial != 0) {
		if (gvariant) {
			uint64_t reply_serial = message->reply_serial;

			add_field(builder, driver,
					DBUS_MESSAGE_FIELD_REPLY_SERIAL,
					"t", &reply_serial);
		} else {
			add_field(builder, driver,
					DBUS_MESSAGE_FIELD_REPLY_SERIAL,
					"u", &message->reply_serial);
		}

		message->reply_serial = 0;
	}

	if (message->sender) {
		add_field(builder, driver, DBUS_MESSAGE_FIELD_SENDER,
					"s", message->sender);
		l_free(message->sender);
		message->sender = NULL;
	}

	if (signature[0] != '\0' && !gvariant)
		add_field(builder, driver, DBUS_MESSAGE_FIELD_SIGNATURE,
				"g", signature);

	if (message->num_fds)
		add_field(builder, driver, DBUS_MESSAGE_FIELD_UNIX_FDS,
					"u", &message->num_fds);

	driver->leave_array(builder);

	generated_signature = driver->finish(builder, &message->header,
						&header_size);
	l_free(generated_signature);

	driver->free(builder);

	if (!_dbus_message_is_gvariant(message)) {
		struct dbus_header *hdr = message->header;

		hdr->dbus1.body_length = message->body_size;
	}

	/* We must align the end of the header to an 8-byte boundary */
	message->header_size = align_len(header_size, 8);
	message->header = l_realloc(message->header, message->header_size);
	memset(message->header + header_size, 0,
			message->header_size - header_size);
	message->header_end = header_size;
}

struct container {
	char type;
	const char *sig_start;
	const char *sig_end;
	unsigned int n_items;
};

static bool append_arguments(struct l_dbus_message *message,
					const char *signature, va_list args)
{
	struct l_dbus_message_builder *builder;
	bool ret;

	builder = l_dbus_message_builder_new(message);
	if (!builder)
		return false;

	if (!l_dbus_message_builder_append_from_valist(builder, signature,
									args)) {
		l_dbus_message_builder_destroy(builder);
		return false;
	}

	l_dbus_message_builder_finalize(builder);

	ret = strcmp(signature, builder->message->signature) == 0;

	l_dbus_message_builder_destroy(builder);

	return ret;
}

LIB_EXPORT bool l_dbus_message_get_error(struct l_dbus_message *message,
					const char **name, const char **text)
{
	struct dbus_header *hdr;
	const char *str;

	if (unlikely(!message))
		return false;

	hdr = message->header;

	if (hdr->message_type != DBUS_MESSAGE_TYPE_ERROR)
		return false;

	if (!message->signature)
		return false;

	if (message->signature[0] != 's')
		return false;

	str = _dbus_message_get_nth_string_argument(message, 0);
	if (!str)
		return false;

	if (!message->error_name)
		get_header_field(message, DBUS_MESSAGE_FIELD_ERROR_NAME, 's',
					&message->error_name);

	if (name)
		*name = message->error_name;

	if (text)
		*text = str;

	return true;
}

LIB_EXPORT bool l_dbus_message_is_error(struct l_dbus_message *message)
{
	struct dbus_header *hdr;

	if (unlikely(!message))
		return false;

	hdr = message->header;
	return hdr->message_type == DBUS_MESSAGE_TYPE_ERROR;
}

LIB_EXPORT bool l_dbus_message_get_arguments_valist(
					struct l_dbus_message *message,
					const char *signature, va_list args)
{
	struct l_dbus_message_iter iter;

	if (unlikely(!message))
		return false;

	if (!message->signature) {
		/* An empty signature is valid */
		if (!signature || *signature == '\0')
			return true;

		return false;
	}

	if (!signature || strcmp(message->signature, signature))
		return false;

	if (_dbus_message_is_gvariant(message)) {
		if (!_gvariant_iter_init(&iter, message, message->signature,
						NULL, message->body,
						message->body_size))
			return false;
	} else
		_dbus1_iter_init(&iter, message, message->signature, NULL,
				message->body, message->body_size);

	return message_iter_next_entry_valist(&iter, args);
}

LIB_EXPORT bool l_dbus_message_get_arguments(struct l_dbus_message *message,
						const char *signature, ...)
{
	va_list args;
	bool result;

	va_start(args, signature);
	result = l_dbus_message_get_arguments_valist(message, signature, args);
	va_end(args);

	return result;
}

LIB_EXPORT bool l_dbus_message_set_arguments(struct l_dbus_message *message,
						const char *signature, ...)
{
	va_list args;
	bool result;

	if (unlikely(!message))
		return false;

	if (unlikely(message->sealed))
		return false;

	if (!signature)
		return true;

	va_start(args, signature);
	result = append_arguments(message, signature, args);
	va_end(args);

	return result;
}

LIB_EXPORT bool l_dbus_message_set_arguments_valist(
					struct l_dbus_message *message,
					const char *signature, va_list args)
{
	bool result;

	if (unlikely(!message))
		return false;

	if (!signature)
		return true;

	result = append_arguments(message, signature, args);

	return result;
}

LIB_EXPORT const char *l_dbus_message_get_path(struct l_dbus_message *message)
{
	if (unlikely(!message))
		return NULL;

	if (!message->path && message->sealed)
		get_header_field(message, DBUS_MESSAGE_FIELD_PATH, 'o',
					&message->path);

	return message->path;
}

LIB_EXPORT const char *l_dbus_message_get_interface(struct l_dbus_message *message)
{
	if (unlikely(!message))
		return NULL;

	if (!message->interface && message->sealed)
		get_header_field(message, DBUS_MESSAGE_FIELD_INTERFACE, 's',
					&message->interface);

	return message->interface;
}

LIB_EXPORT const char *l_dbus_message_get_member(struct l_dbus_message *message)
{
	if (unlikely(!message))
		return NULL;

	if (!message->member && message->sealed)
		get_header_field(message, DBUS_MESSAGE_FIELD_MEMBER, 's',
					&message->member);

	return message->member;
}

LIB_EXPORT const char *l_dbus_message_get_destination(struct l_dbus_message *message)
{
	if (unlikely(!message))
		return NULL;

	if (!message->destination && message->sealed)
		get_header_field(message, DBUS_MESSAGE_FIELD_DESTINATION, 's',
							&message->destination);

	return message->destination;
}

LIB_EXPORT const char *l_dbus_message_get_sender(struct l_dbus_message *message)
{
	if (unlikely(!message))
		return NULL;

	if (!message->sender && message->sealed)
		get_header_field(message, DBUS_MESSAGE_FIELD_SENDER, 's',
					&message->sender);

	return message->sender;
}

LIB_EXPORT const char *l_dbus_message_get_signature(
						struct l_dbus_message *message)
{
	if (unlikely(!message))
		return NULL;

	return message->signature;
}

uint32_t _dbus_message_get_reply_serial(struct l_dbus_message *message)
{
	if (unlikely(!message))
		return 0;

	if (message->reply_serial == 0 && message->sealed) {
		if (_dbus_message_is_gvariant(message)) {
			uint64_t reply_serial = 0;

			get_header_field(message,
					DBUS_MESSAGE_FIELD_REPLY_SERIAL, 't',
					&reply_serial);

			message->reply_serial = reply_serial;
		} else
			get_header_field(message,
					DBUS_MESSAGE_FIELD_REPLY_SERIAL, 'u',
					&message->reply_serial);
	}

	return message->reply_serial;
}

enum dbus_message_type _dbus_message_get_type(struct l_dbus_message *message)
{
	struct dbus_header *header;

	header = message->header;
	return header->message_type;
}

const char * _dbus_message_get_type_as_string(struct l_dbus_message *message)
{
	struct dbus_header *header;

	header = message->header;

	switch (header->message_type) {
	case DBUS_MESSAGE_TYPE_METHOD_CALL:
		return "method_call";
	case DBUS_MESSAGE_TYPE_METHOD_RETURN:
		return "method_return";
	case DBUS_MESSAGE_TYPE_ERROR:
		return "error";
	case DBUS_MESSAGE_TYPE_SIGNAL:
		return "signal";
	}

	return NULL;
}

uint8_t _dbus_message_get_endian(struct l_dbus_message *message)
{
	struct dbus_header *header = message->header;

	return header->endian;
}

uint8_t _dbus_message_get_version(struct l_dbus_message *message)
{
	struct dbus_header *header = message->header;

	return header->version;
}

LIB_EXPORT bool l_dbus_message_iter_next_entry(struct l_dbus_message_iter *iter,
									...)
{
	va_list args;
	bool result;

	if (unlikely(!iter))
		return false;

	va_start(args, iter);
	result = message_iter_next_entry_valist(iter, args);
	va_end(args);

	return result;
}

LIB_EXPORT bool l_dbus_message_iter_get_variant(
					struct l_dbus_message_iter *iter,
					const char *signature, ...)
{
	va_list args;
	bool result;

	if (unlikely(!iter))
		return false;

	if (!iter->sig_start || strlen(signature) != iter->sig_len ||
			memcmp(iter->sig_start, signature, iter->sig_len))
		return false;

	va_start(args, signature);
	result = message_iter_next_entry_valist(iter, args);
	va_end(args);

	return result;
}

LIB_EXPORT bool l_dbus_message_iter_get_fixed_array(
					struct l_dbus_message_iter *iter,
					void *out, uint32_t *n_elem)
{
	if (unlikely(!iter))
		return false;

	if (_dbus_message_is_gvariant(iter->message))
		return false;

	return _dbus1_iter_get_fixed_array(iter, out, n_elem);
}

void _dbus_message_set_sender(struct l_dbus_message *message,
					const char *sender)
{
	if (!_dbus_message_is_gvariant(message))
		return;

	l_free(message->sender);

	message->sender = l_strdup(sender);
}

void _dbus_message_set_destination(struct l_dbus_message *message,
					const char *destination)
{
	if (!_dbus_message_is_gvariant(message))
		return;

	l_free(message->destination);

	message->destination = l_strdup(destination);
}

LIB_EXPORT struct l_dbus_message_builder *l_dbus_message_builder_new(
						struct l_dbus_message *message)
{
	struct l_dbus_message_builder *ret;

	if (unlikely(!message))
		return NULL;

	if (message->sealed)
		return NULL;

	ret = l_new(struct l_dbus_message_builder, 1);
	ret->message = l_dbus_message_ref(message);

	if (_dbus_message_is_gvariant(message))
		ret->driver = &gvariant_driver;
	else
		ret->driver = &dbus1_driver;

	ret->builder = ret->driver->new(NULL, 0);

	return ret;
}

LIB_EXPORT void l_dbus_message_builder_destroy(
					struct l_dbus_message_builder *builder)
{
	if (unlikely(!builder))
		return;

	builder->driver->free(builder->builder);
	l_dbus_message_unref(builder->message);

	l_free(builder);
}

LIB_EXPORT bool l_dbus_message_builder_append_basic(
					struct l_dbus_message_builder *builder,
					char type, const void *value)
{
	if (unlikely(!builder))
		return false;

	return builder->driver->append_basic(builder->builder, type, value);
}

LIB_EXPORT bool l_dbus_message_builder_enter_container(
					struct l_dbus_message_builder *builder,
					char container_type,
					const char *signature)
{
	if (unlikely(!builder))
		return false;

	switch (container_type) {
	case DBUS_CONTAINER_TYPE_ARRAY:
		return builder->driver->enter_array(builder->builder,
								signature);
	case DBUS_CONTAINER_TYPE_DICT_ENTRY:
		return builder->driver->enter_dict(builder->builder, signature);
	case DBUS_CONTAINER_TYPE_STRUCT:
		return builder->driver->enter_struct(builder->builder,
								signature);
	case DBUS_CONTAINER_TYPE_VARIANT:
		return builder->driver->enter_variant(builder->builder,
								signature);
	default:
		break;
	}

	return false;
}

LIB_EXPORT bool l_dbus_message_builder_leave_container(
					struct l_dbus_message_builder *builder,
					char container_type)
{
	if (unlikely(!builder))
		return false;

	switch (container_type) {
	case DBUS_CONTAINER_TYPE_ARRAY:
		return builder->driver->leave_array(builder->builder);
	case DBUS_CONTAINER_TYPE_DICT_ENTRY:
		return builder->driver->leave_dict(builder->builder);
	case DBUS_CONTAINER_TYPE_STRUCT:
		return builder->driver->leave_struct(builder->builder);
	case DBUS_CONTAINER_TYPE_VARIANT:
		return builder->driver->leave_variant(builder->builder);
	default:
		break;
	}

	return false;
}

LIB_EXPORT bool l_dbus_message_builder_enter_struct(
					struct l_dbus_message_builder *builder,
					const char *signature)
{
	return l_dbus_message_builder_enter_container(builder, 'r', signature);
}

LIB_EXPORT bool l_dbus_message_builder_leave_struct(
					struct l_dbus_message_builder *builder)
{
	return l_dbus_message_builder_leave_container(builder, 'r');
}

LIB_EXPORT bool l_dbus_message_builder_enter_array(
					struct l_dbus_message_builder *builder,
					const char *signature)
{
	return l_dbus_message_builder_enter_container(builder, 'a', signature);
}

LIB_EXPORT bool l_dbus_message_builder_leave_array(
					struct l_dbus_message_builder *builder)
{
	return l_dbus_message_builder_leave_container(builder, 'a');
}

LIB_EXPORT bool l_dbus_message_builder_enter_dict(
					struct l_dbus_message_builder *builder,
					const char *signature)
{
	return l_dbus_message_builder_enter_container(builder, 'e', signature);
}

LIB_EXPORT bool l_dbus_message_builder_leave_dict(
					struct l_dbus_message_builder *builder)
{
	return l_dbus_message_builder_leave_container(builder, 'e');
}

LIB_EXPORT bool l_dbus_message_builder_enter_variant(
					struct l_dbus_message_builder *builder,
					const char *signature)
{
	return l_dbus_message_builder_enter_container(builder, 'v', signature);
}

LIB_EXPORT bool l_dbus_message_builder_leave_variant(
					struct l_dbus_message_builder *builder)
{
	return l_dbus_message_builder_leave_container(builder, 'v');
}

/**
 * l_dbus_message_builder_append_from_iter:
 * @builder: message builder to receive a new value
 * @from: message iterator to have its position moved by one value
 *
 * Copy one value from a message iterator onto a message builder.  The
 * value's signature is also copied.
 *
 * Returns: whether the value was correctly copied.  On failure both
 *          the @from iterator and the @builder may have their positions
 *          moved to somewhere within the new value if it's of a
 *          container type.
 **/
LIB_EXPORT bool l_dbus_message_builder_append_from_iter(
					struct l_dbus_message_builder *builder,
					struct l_dbus_message_iter *from)
{
	static const char *simple_types = "sogybnqiuxtd";
	char type = from->sig_start[from->sig_pos];
	char container_type;
	char signature[256];
	struct l_dbus_message_iter iter;
	void *basic_ptr;
	uint64_t basic;
	uint32_t uint32_val;
	bool (*get_basic)(struct l_dbus_message_iter *, char, void *);
	bool (*enter_func)(struct l_dbus_message_iter *,
				struct l_dbus_message_iter *);
	bool (*enter_struct)(struct l_dbus_message_iter *,
				struct l_dbus_message_iter *);
	bool (*enter_array)(struct l_dbus_message_iter *,
				struct l_dbus_message_iter *);
	bool (*enter_variant)(struct l_dbus_message_iter *,
				struct l_dbus_message_iter *);

	if (_dbus_message_is_gvariant(from->message)) {
		get_basic = _gvariant_iter_next_entry_basic;
		enter_struct = _gvariant_iter_enter_struct;
		enter_array = _gvariant_iter_enter_array;
		enter_variant = _gvariant_iter_enter_variant;
	} else {
		get_basic = _dbus1_iter_next_entry_basic;
		enter_struct = _dbus1_iter_enter_struct;
		enter_array = _dbus1_iter_enter_array;
		enter_variant = _dbus1_iter_enter_variant;
	}

	if (strchr(simple_types, type)) {
		if (strchr("sog", type)) {
			if (!get_basic(from, type, &basic_ptr))
				return false;
		} else {
			basic_ptr = &basic;

			if (!get_basic(from, type, basic_ptr))
				return false;
		}

		if (!l_dbus_message_builder_append_basic(builder, type,
								basic_ptr))
			return false;

		return true;
	}

	switch (type) {
	case 'h':
		if (!get_basic(from, type, &uint32_val))
			return false;

		if (!l_dbus_message_builder_append_basic(builder, type,
						&builder->message->num_fds))
			return false;

		if (builder->message->num_fds <
				L_ARRAY_SIZE(builder->message->fds)) {
			int fd;

			if (uint32_val < from->message->num_fds)
				fd = fcntl(from->message->fds[uint32_val],
						F_DUPFD_CLOEXEC, 3);
			else
				fd = -1;

			builder->message->fds[builder->message->num_fds++] = fd;
		}

		return true;
	case '(':
		enter_func = enter_struct;
		container_type = DBUS_CONTAINER_TYPE_STRUCT;
		break;
	case '{':
		enter_func = enter_struct;
		container_type = DBUS_CONTAINER_TYPE_DICT_ENTRY;
		break;
	case 'a':
		enter_func = enter_array;
		container_type = DBUS_CONTAINER_TYPE_ARRAY;
		break;
	case 'v':
		enter_func = enter_variant;
		container_type = DBUS_CONTAINER_TYPE_VARIANT;
		break;
	default:
		return false;
	}

	if (!enter_func(from, &iter))
		return false;

	memcpy(signature, iter.sig_start, iter.sig_len);
	signature[iter.sig_len] = '\0';

	if (!l_dbus_message_builder_enter_container(builder,
						container_type, signature))
		return false;

	if (container_type == DBUS_CONTAINER_TYPE_ARRAY)
		while(l_dbus_message_builder_append_from_iter(builder, &iter));
	else
		while (iter.sig_pos < iter.sig_len)
			if (!l_dbus_message_builder_append_from_iter(builder,
									&iter))
				return false;

	if (!l_dbus_message_builder_leave_container(builder,
						container_type))
		return false;

	return true;
}

LIB_EXPORT bool l_dbus_message_builder_append_from_valist(
					struct l_dbus_message_builder *builder,
					const char *signature, va_list args)
{
	struct builder_driver *driver;
	char subsig[256];
	const char *sigend;
	/* Nesting requires an extra stack entry for the base level */
	struct container stack[DBUS_MAX_NESTING + 1];
	unsigned int stack_index = 0;

	if (unlikely(!builder))
		return false;

	driver = builder->driver;

	stack[stack_index].type = DBUS_CONTAINER_TYPE_STRUCT;
	stack[stack_index].sig_start = signature;
	stack[stack_index].sig_end = signature + strlen(signature);
	stack[stack_index].n_items = 0;

	while (stack_index != 0 || stack[0].sig_start != stack[0].sig_end) {
		const char *s;
		const char *str;

		if (stack[stack_index].type == DBUS_CONTAINER_TYPE_ARRAY &&
				stack[stack_index].n_items == 0)
			stack[stack_index].sig_start =
				stack[stack_index].sig_end;

		if (stack[stack_index].sig_start ==
				stack[stack_index].sig_end) {
			bool ret;

			/*
			 * Sanity check in case of an invalid signature that
			 * isn't caught elsewhere.
			 */
			if (unlikely(stack_index == 0))
				return false;

			switch (stack[stack_index].type) {
			case DBUS_CONTAINER_TYPE_STRUCT:
				ret = driver->leave_struct(builder->builder);
				break;
			case DBUS_CONTAINER_TYPE_DICT_ENTRY:
				ret = driver->leave_dict(builder->builder);
				break;
			case DBUS_CONTAINER_TYPE_VARIANT:
				ret = driver->leave_variant(builder->builder);
				break;
			case DBUS_CONTAINER_TYPE_ARRAY:
				ret = driver->leave_array(builder->builder);
				break;
			default:
				ret = false;
			}

			if (!ret)
				return false;

			stack_index -= 1;
			continue;
		}

		s = stack[stack_index].sig_start;

		if (stack[stack_index].type != DBUS_CONTAINER_TYPE_ARRAY)
			stack[stack_index].sig_start += 1;
		else
			stack[stack_index].n_items -= 1;

		switch (*s) {
		case 'o':
		case 's':
		case 'g':
			str = va_arg(args, const char *);

			if (!driver->append_basic(builder->builder, *s, str))
				return false;
			break;
		case 'b':
		case 'y':
		{
			uint8_t y = (uint8_t) va_arg(args, int);

			if (!driver->append_basic(builder->builder, *s, &y))
				return false;

			break;
		}
		case 'n':
		case 'q':
		{
			uint16_t n = (uint16_t) va_arg(args, int);

			if (!driver->append_basic(builder->builder, *s, &n))
				return false;

			break;
		}
		case 'i':
		case 'u':
		{
			uint32_t u = va_arg(args, uint32_t);

			if (!driver->append_basic(builder->builder, *s, &u))
				return false;

			break;
		}
		case 'h':
		{
			int fd = va_arg(args, int);
			struct l_dbus_message *message = builder->message;

			if (!driver->append_basic(builder->builder, *s,
							&message->num_fds))
				return false;

			if (message->num_fds < L_ARRAY_SIZE(message->fds))
				message->fds[message->num_fds++] =
					fcntl(fd, F_DUPFD_CLOEXEC, 3);

			break;
		}
		case 'x':
		case 't':
		{
			uint64_t x = va_arg(args, uint64_t);

			if (!driver->append_basic(builder->builder, *s, &x))
				return false;
			break;
		}
		case 'd':
		{
			double d = va_arg(args, double);

			if (!driver->append_basic(builder->builder, *s, &d))
				return false;
			break;
		}
		case '(':
		case '{':
			if (stack_index == DBUS_MAX_NESTING)
				return false;

			sigend = _dbus_signature_end(s);
			memcpy(subsig, s + 1, sigend - s - 1);
			subsig[sigend - s - 1] = '\0';

			if (*s == '(' && !driver->enter_struct(builder->builder,
									subsig))
				return false;

			if (*s == '{' && !driver->enter_dict(builder->builder,
									subsig))
				return false;

			if (stack[stack_index].type !=
					DBUS_CONTAINER_TYPE_ARRAY)
				stack[stack_index].sig_start = sigend + 1;

			stack_index += 1;
			stack[stack_index].sig_start = s + 1;
			stack[stack_index].sig_end = sigend;
			stack[stack_index].n_items = 0;
			stack[stack_index].type = *s == '(' ?
						DBUS_CONTAINER_TYPE_STRUCT :
						DBUS_CONTAINER_TYPE_DICT_ENTRY;

			break;
		case 'v':
			if (stack_index == DBUS_MAX_NESTING)
				return false;

			str = va_arg(args, const char *);

			if (!str)
				return false;

			if (!driver->enter_variant(builder->builder, str))
				return false;

			stack_index += 1;
			stack[stack_index].type = DBUS_CONTAINER_TYPE_VARIANT;
			stack[stack_index].sig_start = str;
			stack[stack_index].sig_end = str + strlen(str);
			stack[stack_index].n_items = 0;

			break;
		case 'a':
			if (stack_index == DBUS_MAX_NESTING)
				return false;

			sigend = _dbus_signature_end(s + 1) + 1;
			memcpy(subsig, s + 1, sigend - s - 1);
			subsig[sigend - s - 1] = '\0';

			if (!driver->enter_array(builder->builder, subsig))
				return false;

			if (stack[stack_index].type !=
					DBUS_CONTAINER_TYPE_ARRAY)
				stack[stack_index].sig_start = sigend;

			stack_index += 1;
			stack[stack_index].sig_start = s + 1;
			stack[stack_index].sig_end = sigend;
			stack[stack_index].n_items = va_arg(args, unsigned int);
			stack[stack_index].type = DBUS_CONTAINER_TYPE_ARRAY;

			break;
		default:
			return false;
		}
	}

	return true;
}

LIB_EXPORT struct l_dbus_message *l_dbus_message_builder_finalize(
					struct l_dbus_message_builder *builder)
{
	char *generated_signature;

	if (unlikely(!builder))
		return NULL;

	generated_signature = builder->driver->finish(builder->builder,
						&builder->message->body,
						&builder->message->body_size);

	build_header(builder->message, generated_signature);
	builder->message->sealed = true;
	builder->message->signature = generated_signature;
	builder->message->signature_free = true;

	return builder->message;
}

bool _dbus_message_builder_mark(struct l_dbus_message_builder *builder)
{
	if (unlikely(!builder))
		return false;

	return builder->driver->mark(builder->builder);
}

bool _dbus_message_builder_rewind(struct l_dbus_message_builder *builder)
{
	if (unlikely(!builder))
		return false;

	return builder->driver->rewind(builder->builder);
}
