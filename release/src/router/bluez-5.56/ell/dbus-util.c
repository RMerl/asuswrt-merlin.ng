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
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "dbus.h"
#include "private.h"
#include "dbus-private.h"
#include "string.h"
#include "queue.h"

#define DBUS_MAX_INTERFACE_LEN 255
#define DBUS_MAX_METHOD_LEN 255

static const char *simple_types = "sogybnqiuxtdh";

static int get_alignment(const char type)
{
	switch (type) {
	case 'b':
		return 4;
	case 'y':
		return 1;
	case 'n':
	case 'q':
		return 2;
	case 'u':
	case 'i':
		return 4;
	case 'x':
	case 't':
	case 'd':
		return 8;
	case 's':
	case 'o':
		return 4;
	case 'g':
		return 1;
	case 'a':
		return 4;
	case '(':
	case '{':
		return 8;
	case 'v':
		return 1;
	case 'h':
		return 4;
	default:
		return 0;
	}
}

static int get_basic_size(const char type)
{
	switch (type) {
	case 'b':
		return 4;
	case 'y':
		return 1;
	case 'n':
	case 'q':
		return 2;
	case 'i':
	case 'u':
		return 4;
	case 'x':
	case 't':
		return 8;
	case 'd':
		return 8;
	case 'h':
		return 4;
	default:
		return 0;
	}
}

static inline bool is_valid_character(const char c, bool bus_name)
{
	if (c >= 'a' && c <= 'z')
		return true;

	if (c >= 'A' && c <= 'Z')
		return true;

	if (c >= '0' && c <= '9')
		return true;

	if (c == '_')
		return true;

	if (c == '-' && bus_name)
		return true;

	return false;
}

bool _dbus_valid_object_path(const char *path)
{
	unsigned int i;
	char c = '\0';

	if (path == NULL)
		return false;

	if (path[0] == '\0')
		return false;

	if (path[0] && !path[1] && path[0] == '/')
		return true;

	if (path[0] != '/')
		return false;

	for (i = 0; path[i]; i++) {
		if (path[i] == '/' && c == '/')
			return false;

		c = path[i];

		if (is_valid_character(path[i], false) || path[i] == '/')
			continue;

		return false;
	}

	if (path[i-1] == '/')
		return false;

	return true;
}

static const char *validate_next_type(const char *sig)
{
	char s = *sig;

	if (s == '\0')
		return NULL;

	if (strchr(simple_types, s) || s == 'v')
		return sig + 1;

	switch (s) {
	case 'a':
		s = *++sig;

		if (s == '{') {
			s = *++sig;

			/* Dictionary keys can only be simple types */
			if (!strchr(simple_types, s))
				return NULL;

			sig = validate_next_type(sig + 1);

			if (!sig)
				return NULL;

			if (*sig != '}')
				return NULL;

			return sig + 1;
		}

		return validate_next_type(sig);

	case '(':
		sig++;

		do
			sig = validate_next_type(sig);
		while (sig && *sig != ')');

		if (!sig)
			return NULL;

		return sig + 1;
	}

	return NULL;
}

static bool valid_dict_signature(const char *sig)
{
	char s = *sig;

	if (s != '{')
		return false;

	s = *++sig;

	if (!strchr(simple_types, s))
		return false;

	sig = validate_next_type(sig + 1);
	if (!sig)
		return false;

	if (sig[0] != '}')
		return false;

	if (sig[1] != '\0')
		return false;

	return true;
}

bool _dbus_valid_signature(const char *sig)
{
	const char *s = sig;

	do {
		s = validate_next_type(s);

		if (!s)
			return false;
	} while (*s);

	return true;
}

int _dbus_num_children(const char *sig)
{
	const char *s = sig;
	int num_children = 0;

	do {
		s = validate_next_type(s);

		if (!s)
			return -1;

		num_children += 1;
	} while (*s);

	return num_children;
}

static bool valid_member_name(const char *start, const char *end,
				bool bus_name)
{
	const char *p;

	if ((end - start) < 1)
		return false;

	if (*start >= '0' && *start <= '9')
		return false;

	for (p = start; p < end; p++)
		if (!is_valid_character(*p, bus_name))
			return false;

	return true;
}

bool _dbus_valid_method(const char *method)
{
	unsigned int i;

	if (!method)
		return false;

	if (method[0] == '\0' || strlen(method) > DBUS_MAX_METHOD_LEN)
		return false;

	if (method[0] >= '0' && method[0] <= '9')
		return false;

	for (i = 0; method[i]; i++)
		if (!is_valid_character(method[i], false))
			return false;

	return true;
}

bool _dbus_valid_interface(const char *interface)
{
	const char *sep;

	if (!interface)
		return false;

	if (interface[0] == '\0' || strlen(interface) > DBUS_MAX_INTERFACE_LEN)
		return false;

	sep = strchrnul(interface, '.');
	if (*sep == '\0')
		return false;

	while (true) {
		if (!valid_member_name(interface, sep, false))
			return false;

		if (*sep == '\0')
			break;

		interface = sep + 1;
		sep = strchrnul(interface, '.');
	}

	return true;
}

bool _dbus_parse_unique_name(const char *name, uint64_t *out_id)
{
	char *endp = NULL;
	uint64_t r;

	if (!l_str_has_prefix(name, ":1."))
		return false;

	errno = 0;
	r = strtoull(name + 3, &endp, 10);
	if (!endp || endp == name || *endp || errno)
		return false;

	if (out_id)
		*out_id = r;

	return true;
}

bool _dbus_valid_bus_name(const char *bus_name)
{
	const char *sep;

	if (!bus_name)
		return false;

	if (bus_name[0] == '\0' || strlen(bus_name) > DBUS_MAX_INTERFACE_LEN)
		return false;

	if (_dbus_parse_unique_name(bus_name, NULL))
		return true;

	sep = strchrnul(bus_name, '.');
	if (*sep == '\0')
		return false;

	while (true) {
		if (!valid_member_name(bus_name, sep, true))
			return false;

		if (*sep == '\0')
			break;

		bus_name = sep + 1;
		sep = strchrnul(bus_name, '.');
	}

	return true;
}

const char *_dbus_signature_end(const char *signature)
{
	const char *ptr = signature;
	unsigned int indent = 0;
	char expect;

	switch (*signature) {
	case '(':
		expect = ')';
		break;
	case '{':
		expect = '}';
		break;
	case 'a':
		return _dbus_signature_end(signature + 1);
	default:
		return signature;
	}

	for (ptr = signature; *ptr != '\0'; ptr++) {
		if (*ptr == *signature)
			indent++;
		else if (*ptr == expect)
			if (!--indent)
				return ptr;
	}

	return NULL;
}

bool _dbus1_header_is_valid(void *data, size_t size)
{
	struct dbus_header *hdr;
	size_t header_len;

	if (size < sizeof(struct dbus_header))
		return false;

	hdr = data;

	if (hdr->endian != DBUS_NATIVE_ENDIAN)
		header_len = bswap_32(hdr->dbus1.field_length);
	else
		header_len = hdr->dbus1.field_length;

	header_len += sizeof(struct dbus_header);
	return size >= header_len;
}

static inline void dbus1_iter_init_internal(struct l_dbus_message_iter *iter,
						struct l_dbus_message *message,
						enum dbus_container_type type,
						const char *sig_start,
						const char *sig_end,
						const void *data, size_t len,
						size_t pos)
{
	size_t sig_len;

	iter->message = message;

	if (sig_end)
		sig_len = sig_end - sig_start;
	else
		sig_len = strlen(sig_start);

	iter->sig_start = sig_start;
	iter->sig_len = sig_len;
	iter->sig_pos = 0;
	iter->data = data;
	iter->len = pos + len;
	iter->pos = pos;
	iter->container_type = type;
}

void _dbus1_iter_init(struct l_dbus_message_iter *iter,
			struct l_dbus_message *message,
			const char *sig_start, const char *sig_end,
			const void *data, size_t len)
{
	dbus1_iter_init_internal(iter, message, DBUS_CONTAINER_TYPE_STRUCT,
					sig_start, sig_end, data, len, 0);
}

static const char *calc_len_next_item(const char *signature, const void *data,
					size_t data_pos, size_t data_len,
					size_t *out_len)
{
	unsigned int alignment;
	size_t pos;
	size_t len;
	const char *sig_end;
	const char *var_sig;

	alignment = get_alignment(*signature);
	if (alignment == 0)
		return NULL;

	pos = align_len(data_pos, alignment);
	if (pos > data_len)
		return NULL;

	switch (*signature) {
	case 'o':
	case 's':
		if (pos + 5 > data_len)
			return NULL;

		pos += l_get_u32(data + pos) + 5;
		break;
	case 'g':
		if (pos + 2 > data_len)
			return NULL;

		pos += l_get_u8(data + pos) + 2;
		break;
	case 'y':
		pos += 1;
		break;
	case 'n':
	case 'q':
		pos += 2;
		break;
	case 'b':
	case 'i':
	case 'u':
	case 'h':
		pos += 4;
		break;
	case 'x':
	case 't':
	case 'd':
		pos += 8;
		break;
	case 'a':
		if (pos + 4 > data_len)
			return NULL;

		len = l_get_u32(data + pos);
		pos += 4;

		alignment = get_alignment(signature[1]);
		pos = align_len(pos, alignment);
		pos += len;

		sig_end = _dbus_signature_end(signature) + 1;
		goto done;
	case '(':
		sig_end = signature + 1;

		while (*sig_end != ')') {
			sig_end = calc_len_next_item(sig_end, data, pos,
							data_len, &len);

			if (!sig_end)
				return NULL;

			pos += len;
		}

		sig_end += 1;
		goto done;
	case '{':
		sig_end = calc_len_next_item(signature + 1, data, pos,
						data_len, &len);

		if (!sig_end)
			return NULL;

		pos += len;

		sig_end = calc_len_next_item(sig_end, data, pos,
						data_len, &len);

		if (!sig_end)
			return NULL;

		pos += len;
		sig_end += 1;
		goto done;
	case 'v':
		if (!calc_len_next_item("g", data, pos, data_len, &len))
			return NULL;

		var_sig = data + pos + 1;
		pos += len;

		if (!calc_len_next_item(var_sig, data, pos, data_len, &len))
			return NULL;

		pos += len;
		break;
	default:
		return NULL;
	}

	sig_end = signature + 1;

done:
	if (pos > data_len)
		return NULL;

	*out_len = pos - data_pos;
	return sig_end;
}

bool _dbus1_iter_next_entry_basic(struct l_dbus_message_iter *iter,
					char type, void *out)
{
	const char *str_val;
	uint8_t uint8_val;
	uint16_t uint16_val;
	uint32_t uint32_val;
	uint64_t uint64_val;
	int16_t int16_val;
	int32_t int32_val;
	int64_t int64_val;
	size_t pos;

	if (iter->pos >= iter->len)
		return false;

	pos = align_len(iter->pos, get_alignment(type));

	switch (type) {
	case 'o':
	case 's':
		if (pos + 5 > iter->len)
			return false;
		uint32_val = l_get_u32(iter->data + pos);
		str_val = iter->data + pos + 4;
		*(const void **) out = str_val;
		iter->pos = pos + uint32_val + 5;
		break;
	case 'g':
		if (pos + 2 > iter->len)
			return false;
		uint8_val = l_get_u8(iter->data + pos);
		str_val = iter->data + pos + 1;
		*(const void **) out = str_val;
		iter->pos = pos + uint8_val + 2;
		break;
	case 'b':
		if (pos + 4 > iter->len)
			return false;
		uint32_val = l_get_u32(iter->data + pos);
		*(bool *) out = !!uint32_val;
		iter->pos = pos + 4;
		break;
	case 'y':
		if (pos + 1 > iter->len)
			return false;
		uint8_val = l_get_u8(iter->data + pos);
		*(uint8_t *) out = uint8_val;
		iter->pos = pos + 1;
		break;
	case 'n':
		if (pos + 2 > iter->len)
			return false;
		int16_val = l_get_s16(iter->data + pos);
		*(int16_t *) out = int16_val;
		iter->pos = pos + 2;
		break;
	case 'q':
		if (pos + 2 > iter->len)
			return false;
		uint16_val = l_get_u16(iter->data + pos);
		*(uint16_t *) out = uint16_val;
		iter->pos = pos + 2;
		break;
	case 'i':
		if (pos + 4 > iter->len)
			return false;
		int32_val = l_get_s32(iter->data + pos);
		*(int32_t *) out = int32_val;
		iter->pos = pos + 4;
		break;
	case 'u':
	case 'h':
		if (pos + 4 > iter->len)
			return false;
		uint32_val = l_get_u32(iter->data + pos);
		*(uint32_t *) out = uint32_val;
		iter->pos = pos + 4;
		break;
	case 'x':
		if (pos + 8 > iter->len)
			return false;
		int64_val = l_get_s64(iter->data + pos);
		*(int64_t *) out= int64_val;
		iter->pos = pos + 8;
		break;
	case 't':
	case 'd':
		if (pos + 8 > iter->len)
			return false;
		uint64_val = l_get_u64(iter->data + pos);
		*(uint64_t *) out = uint64_val;
		iter->pos = pos + 8;
		break;
	default:
		return false;
	}

	if (iter->container_type != DBUS_CONTAINER_TYPE_ARRAY)
		iter->sig_pos += 1;

	return true;
}

bool _dbus1_iter_get_fixed_array(struct l_dbus_message_iter *iter,
					void *out, uint32_t *n_elem)
{
	char type;
	uint32_t size;

	if (iter->container_type != DBUS_CONTAINER_TYPE_ARRAY)
		return false;

	type = iter->sig_start[iter->sig_pos];
	size = get_basic_size(type);

	/* Fail if the array is not a fixed size or contains file descriptors */
	if (!size || type == 'n')
		return false;

	/*
	 * enter_array should already align us to our container type, so
	 * there is no need to align pos here
	 */
	*(const void **) out = iter->data + iter->pos;
	*n_elem = (iter->len - iter->pos) / size;

	return true;
}

bool _dbus1_iter_enter_struct(struct l_dbus_message_iter *iter,
					struct l_dbus_message_iter *structure)
{
	size_t len;
	size_t pos;
	const char *sig_start;
	const char *sig_end;
	bool is_dict = iter->sig_start[iter->sig_pos] == '{';
	bool is_struct = iter->sig_start[iter->sig_pos] == '(';

	if (!is_dict && !is_struct)
		return false;

	pos = align_len(iter->pos, 8);
	if (pos >= iter->len)
		return false;

	sig_start = iter->sig_start + iter->sig_pos + 1;
	sig_end = _dbus_signature_end(iter->sig_start + iter->sig_pos);

	if (!calc_len_next_item(iter->sig_start + iter->sig_pos,
				iter->data, pos, iter->len, &len))
		return false;

	dbus1_iter_init_internal(structure, iter->message,
					DBUS_CONTAINER_TYPE_STRUCT,
					sig_start, sig_end, iter->data,
					len, pos);

	if (iter->container_type != DBUS_CONTAINER_TYPE_ARRAY)
		iter->sig_pos += sig_end - sig_start + 2;

	iter->pos = pos + len;

	return true;
}

bool _dbus1_iter_enter_variant(struct l_dbus_message_iter *iter,
					struct l_dbus_message_iter *variant)
{
	size_t pos;
	uint8_t sig_len;
	size_t len;
	const char *sig_start;

	if (iter->sig_start[iter->sig_pos] != 'v')
		return false;

	pos = align_len(iter->pos, 1);
	if (pos + 2 > iter->len)
		return false;

	sig_len = l_get_u8(iter->data + pos);
	sig_start = iter->data + pos + 1;

	if (!calc_len_next_item(sig_start, iter->data, pos + sig_len + 2,
					iter->len, &len))
		return false;

	dbus1_iter_init_internal(variant, iter->message,
					DBUS_CONTAINER_TYPE_VARIANT,
					sig_start, NULL, iter->data,
					len, pos + sig_len + 2);

	if (iter->container_type != DBUS_CONTAINER_TYPE_ARRAY)
		iter->sig_pos += 1;

	iter->pos = pos + sig_len + 2 + len;

	return true;
}

bool _dbus1_iter_enter_array(struct l_dbus_message_iter *iter,
					struct l_dbus_message_iter *array)
{
	size_t pos;
	size_t len;
	const char *sig_start;
	const char *sig_end;

	if (iter->sig_start[iter->sig_pos] != 'a')
		return false;

	sig_start = iter->sig_start + iter->sig_pos + 1;
	sig_end = _dbus_signature_end(sig_start) + 1;

	pos = align_len(iter->pos, 4);
	if (pos + 4 > iter->len)
		return false;

	len = l_get_u32(iter->data + pos);
	pos += 4;

	pos = align_len(pos, get_alignment(*sig_start));
	dbus1_iter_init_internal(array, iter->message,
					DBUS_CONTAINER_TYPE_ARRAY,
					sig_start, sig_end,
					iter->data, len, pos);

	if (iter->container_type != DBUS_CONTAINER_TYPE_ARRAY)
		iter->sig_pos += sig_end - sig_start + 1;

	iter->pos = pos + len;

	return true;
}

bool _dbus1_iter_skip_entry(struct l_dbus_message_iter *iter)
{
	size_t len;
	const char *sig_end;

	sig_end = calc_len_next_item(iter->sig_start + iter->sig_pos,
					iter->data, iter->pos, iter->len, &len);
	if (!sig_end)
		return false;

	iter->pos += len;
	iter->sig_pos = sig_end - iter->sig_start;

	return true;
}

struct dbus_builder {
	struct l_string *signature;
	void *body;
	size_t body_size;
	size_t body_pos;
	struct l_queue *containers;
	struct {
		struct container *container;
		int sig_end;
		size_t body_pos;
	} mark;
};

struct container {
	size_t start;
	enum dbus_container_type type;
	char signature[256];
	uint8_t sigindex;
};

static struct container *container_new(enum dbus_container_type type,
					const char *signature, size_t start)
{
	struct container *ret;

	ret = l_new(struct container, 1);

	ret->type = type;
	strcpy(ret->signature, signature);
	ret->start = start;

	return ret;
}

static void container_free(struct container *container)
{
	l_free(container);
}

static inline size_t grow_body(struct dbus_builder *builder,
					size_t len, unsigned int alignment)
{
	size_t size = align_len(builder->body_pos, alignment);

	if (size + len > builder->body_size) {
		builder->body = l_realloc(builder->body, size + len);
		builder->body_size = size + len;
	}

	if (size - builder->body_pos > 0)
		memset(builder->body + builder->body_pos, 0,
					size - builder->body_pos);

	builder->body_pos = size + len;

	return size;
}

struct dbus_builder *_dbus1_builder_new(void *body, size_t body_size)
{
	struct dbus_builder *builder;
	struct container *root;

	builder = l_new(struct dbus_builder, 1);
	builder->signature = l_string_new(63);

	builder->containers = l_queue_new();
	root = container_new(DBUS_CONTAINER_TYPE_STRUCT, "", 0);
	l_queue_push_head(builder->containers, root);

	builder->body = body;
	builder->body_size = body_size;
	builder->body_pos = body_size;

	builder->mark.container = root;
	builder->mark.sig_end = 0;
	builder->mark.body_pos = 0;

	return builder;
}

void _dbus1_builder_free(struct dbus_builder *builder)
{
	if (unlikely(!builder))
		return;

	l_string_free(builder->signature);
	l_queue_destroy(builder->containers,
				(l_queue_destroy_func_t) container_free);
	l_free(builder->body);

	l_free(builder);
}

bool _dbus1_builder_append_basic(struct dbus_builder *builder,
					char type, const void *value)
{
	struct container *container = l_queue_peek_head(builder->containers);
	size_t start;
	unsigned int alignment;
	size_t len;

	if (unlikely(!builder))
		return false;

	if (unlikely(!strchr(simple_types, type)))
		return false;

	alignment = get_alignment(type);
	if (!alignment)
		return false;

	if (l_queue_length(builder->containers) == 1)
		l_string_append_c(builder->signature, type);
	else if (container->signature[container->sigindex] != type)
		return false;

	len = get_basic_size(type);

	if (len) {
		uint32_t b;

		start = grow_body(builder, len, alignment);

		if (type == 'b') {
			b = *(bool *)value;
			memcpy(builder->body + start, &b, len);
		} else
			memcpy(builder->body + start, value, len);

		if (container->type != DBUS_CONTAINER_TYPE_ARRAY)
			container->sigindex += 1;

		return true;
	}

	len = strlen(value);

	if (type == 'g') {
		start = grow_body(builder, len + 2, 1);
		l_put_u8(len, builder->body + start);
		strcpy(builder->body + start + 1, value);
	} else {
		start = grow_body(builder, len + 5, 4);
		l_put_u32(len, builder->body + start);
		strcpy(builder->body + start + 4, value);
	}

	if (container->type != DBUS_CONTAINER_TYPE_ARRAY)
		container->sigindex += 1;

	return true;
}

static bool enter_struct_dict_common(struct dbus_builder *builder,
					const char *signature,
					enum dbus_container_type type,
					const char open,
					const char close)
{
	size_t qlen = l_queue_length(builder->containers);
	struct container *container = l_queue_peek_head(builder->containers);
	size_t start;

	if (qlen == 1) {
		if (l_string_length(builder->signature) +
				strlen(signature) + 2 > 255)
			return false;
	} else {
		/* Verify Signatures Match */
		char expect[256];
		const char *start;
		const char *end;

		start = container->signature + container->sigindex;
		end = _dbus_signature_end(start);

		if (*start != open || *end != close)
			return false;

		memcpy(expect, start + 1, end - start - 1);
		expect[end - start - 1] = '\0';

		if (strcmp(expect, signature))
			return false;
	}

	start = grow_body(builder, 0, 8);

	container = container_new(type, signature, start);
	l_queue_push_head(builder->containers, container);

	return true;
}

bool _dbus1_builder_enter_struct(struct dbus_builder *builder,
					const char *signature)
{
	if (!_dbus_valid_signature(signature))
		return false;

	return enter_struct_dict_common(builder, signature,
					DBUS_CONTAINER_TYPE_STRUCT, '(', ')');
}

bool _dbus1_builder_enter_dict(struct dbus_builder *builder,
					const char *signature)
{
	if (_dbus_num_children(signature) != 2)
		return false;

	if (!strchr(simple_types, signature[0]))
		return false;

	return enter_struct_dict_common(builder, signature,
					DBUS_CONTAINER_TYPE_DICT_ENTRY,
					'{', '}');
}

static bool leave_struct_dict_common(struct dbus_builder *builder,
					enum dbus_container_type type,
					const char open,
					const char close)
{
	struct container *container = l_queue_peek_head(builder->containers);
	size_t qlen = l_queue_length(builder->containers);
	struct container *parent;

	if (unlikely(qlen <= 1))
		return false;

	if (unlikely(container->type != type))
		return false;

	l_queue_pop_head(builder->containers);
	qlen -= 1;
	parent = l_queue_peek_head(builder->containers);

	if (qlen == 1)
		l_string_append_printf(builder->signature, "%c%s%c",
						open,
						container->signature,
						close);
	else if (parent->type != DBUS_CONTAINER_TYPE_ARRAY)
		parent->sigindex += strlen(container->signature) + 2;

	container_free(container);

	return true;
}

bool _dbus1_builder_leave_struct(struct dbus_builder *builder)
{
	return leave_struct_dict_common(builder, DBUS_CONTAINER_TYPE_STRUCT,
					'(', ')');
}

bool _dbus1_builder_leave_dict(struct dbus_builder *builder)
{
	return leave_struct_dict_common(builder,
					DBUS_CONTAINER_TYPE_DICT_ENTRY,
					'{', '}');
}

bool _dbus1_builder_enter_variant(struct dbus_builder *builder,
					const char *signature)
{
	size_t qlen = l_queue_length(builder->containers);
	struct container *container = l_queue_peek_head(builder->containers);
	size_t start;
	size_t siglen;

	if (_dbus_num_children(signature) != 1)
		return false;

	if (qlen == 1) {
		if (l_string_length(builder->signature) + 1 > 255)
			return false;
	} else if (container->signature[container->sigindex] != 'v')
		return false;

	siglen = strlen(signature);
	start = grow_body(builder, siglen + 2, 1);
	l_put_u8(siglen, builder->body + start);
	strcpy(builder->body + start + 1, signature);

	container = container_new(DBUS_CONTAINER_TYPE_VARIANT,
					signature, start);
	l_queue_push_head(builder->containers, container);

	return true;
}

bool _dbus1_builder_leave_variant(struct dbus_builder *builder)
{
	struct container *container = l_queue_peek_head(builder->containers);
	size_t qlen = l_queue_length(builder->containers);
	struct container *parent;

	if (unlikely(qlen <= 1))
		return false;

	if (unlikely(container->type != DBUS_CONTAINER_TYPE_VARIANT))
		return false;

	l_queue_pop_head(builder->containers);
	qlen -= 1;
	parent = l_queue_peek_head(builder->containers);

	if (qlen == 1)
		l_string_append_c(builder->signature, 'v');
	else if (parent->type != DBUS_CONTAINER_TYPE_ARRAY)
		parent->sigindex += 1;

	container_free(container);

	return true;
}

bool _dbus1_builder_enter_array(struct dbus_builder *builder,
					const char *signature)
{
	size_t qlen = l_queue_length(builder->containers);
	struct container *container = l_queue_peek_head(builder->containers);
	size_t start;
	int alignment;

	if (_dbus_num_children(signature) != 1 &&
			!valid_dict_signature(signature))
		return false;

	if (qlen == 1) {
		if (l_string_length(builder->signature) +
				strlen(signature) + 1 > 255)
			return false;
	} else {
		/* Verify Signatures Match */
		char expect[256];
		const char *start;
		const char *end;

		start = container->signature + container->sigindex;
		end = validate_next_type(start);

		if (*start != 'a')
			return false;

		memcpy(expect, start + 1, end - start - 1);
		expect[end - start - 1] = '\0';

		if (strcmp(expect, signature))
			return false;
	}

	/* First grow the body enough to cover preceding length */
	start = grow_body(builder, 4, 4);

	/* Now align to element alignment */
	alignment = get_alignment(*signature);
	grow_body(builder, 0, alignment);

	container = container_new(DBUS_CONTAINER_TYPE_ARRAY, signature, start);
	l_queue_push_head(builder->containers, container);

	return true;
}

bool _dbus1_builder_leave_array(struct dbus_builder *builder)
{
	struct container *container = l_queue_peek_head(builder->containers);
	size_t qlen = l_queue_length(builder->containers);
	struct container *parent;
	size_t alignment;
	size_t array_start;

	if (unlikely(qlen <= 1))
		return false;

	if (unlikely(container->type != DBUS_CONTAINER_TYPE_ARRAY))
		return false;

	l_queue_pop_head(builder->containers);
	qlen -= 1;
	parent = l_queue_peek_head(builder->containers);

	if (qlen == 1)
		l_string_append_printf(builder->signature, "a%s",
							container->signature);
	else if (parent->type != DBUS_CONTAINER_TYPE_ARRAY)
		parent->sigindex += strlen(container->signature) + 1;

	/* Update array length */
	alignment = get_alignment(container->signature[0]);
	array_start = align_len(container->start + 4, alignment);

	l_put_u32(builder->body_pos - array_start,
			builder->body + container->start);

	container_free(container);

	return true;
}

bool _dbus1_builder_mark(struct dbus_builder *builder)
{
	struct container *container = l_queue_peek_head(builder->containers);

	builder->mark.container = container;

	if (l_queue_length(builder->containers) == 1)
		builder->mark.sig_end = l_string_length(builder->signature);
	else
		builder->mark.sig_end = container->sigindex;

	builder->mark.body_pos = builder->body_pos;

	return true;
}

bool _dbus1_builder_rewind(struct dbus_builder *builder)
{
	struct container *container;

	while ((container = l_queue_peek_head(builder->containers)) !=
				builder->mark.container) {
		container_free(container);
		l_queue_pop_head(builder->containers);
	}

	builder->body_pos = builder->mark.body_pos;

	if (l_queue_length(builder->containers) == 1)
		l_string_truncate(builder->signature, builder->mark.sig_end);
	else
		container->sigindex = builder->mark.sig_end;

	return true;
}

char *_dbus1_builder_finish(struct dbus_builder *builder,
				void **body, size_t *body_size)
{
	char *signature;

	if (unlikely(!builder))
		return NULL;

	if (unlikely(l_queue_length(builder->containers) != 1))
		return NULL;

	signature = l_string_unwrap(builder->signature);
	builder->signature = NULL;

	*body = builder->body;
	*body_size = builder->body_pos;
	builder->body = NULL;
	builder->body_size = 0;

	return signature;
}
