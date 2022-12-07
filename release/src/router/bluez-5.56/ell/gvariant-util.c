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
#include <unistd.h>
#include <string.h>
#include <endian.h>
#include <limits.h>

#include "private.h"
#include "util.h"
#include "queue.h"
#include "string.h"
#include "log.h"
#include "dbus.h"
#include "dbus-private.h"
#include "gvariant-private.h"

static const char *simple_types = "sogybnqiuxtdh";
static const char *variable_types = "sogav";
static const char *fixed_types = "bynqhiuxtd";

/*
 * The alignment of a container type is equal to the largest alignment of
 * any potential child of that container. This means that, even if an array
 * of 32-bit integers is empty, it still must be aligned to the nearest
 * multiple of 4 bytes. It also means that the variant type (described below)
 * has an alignment of 8 (since it could potentially contain a value of any
 * other type and the maximum alignment is 8).
 */
static int get_basic_alignment(const char type)
{
	switch (type) {
	case 'b':
		return 1;
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
	case 'd':
		return 8;
	case 's':
	case 'g':
	case 'o':
		return 1;
	case 'h':
		return 4;
	case 'v':
		return 8;
	default:
		return 0;
	}
}

static int get_basic_fixed_size(const char type)
{
	switch (type) {
	case 'b':
		return 1;
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
	case 'd':
		return 8;
	case 'h':
		return 4;
	default:
		return 0;
	}
}

static const char *validate_next_type(const char *sig, int *out_alignment)
{
	char s = *sig;
	int alignment;

	if (s == '\0')
		return NULL;

	if (strchr(simple_types, s) || s == 'v') {
		*out_alignment = get_basic_alignment(s);
		return sig + 1;
	}

	switch (s) {
	case 'a':
		return validate_next_type(++sig, out_alignment);

	case '{':
		s = *++sig;

		/* Dictionary keys can only be simple types */
		if (!strchr(simple_types, s))
			return NULL;

		alignment = get_basic_alignment(s);

		sig = validate_next_type(sig + 1, out_alignment);

		if (!sig)
			return NULL;

		if (*sig != '}')
			return NULL;

		if (alignment > *out_alignment)
			*out_alignment = alignment;

		return sig + 1;

	case '(':
	{
		int max_alignment = 1, alignment;

		sig++;

		while (sig && *sig != ')') {
			sig = validate_next_type(sig, &alignment);

			if (alignment > max_alignment)
				max_alignment = alignment;
		}

		if (!sig)
			return NULL;

		if (*sig != ')')
			return NULL;

		*out_alignment = max_alignment;

		return sig + 1;
	}
	}

	return NULL;
}

bool _gvariant_valid_signature(const char *sig)
{
	const char *s = sig;
	int a;

	if (strlen(sig) > 255)
		return false;

	do {
		s = validate_next_type(s, &a);

		if (!s)
			return false;
	} while (*s);

	return true;
}

int _gvariant_num_children(const char *sig)
{
	const char *s = sig;
	int a;
	int num_children = 0;

	if (strlen(sig) > 255)
		return false;

	do {
		s = validate_next_type(s, &a);

		if (!s)
			return -1;

		num_children += 1;
	} while (*s);

	return num_children;
}

int _gvariant_get_alignment(const char *sig)
{
	int max_alignment = 1, alignment;
	const char *s = sig;

	/* 8 is the largest alignment possible, so quit if we reach it */
	while (*s && max_alignment != 8) {
		s = validate_next_type(s, &alignment);
		if (!s)
			return 0;

		if (alignment > max_alignment)
			max_alignment = alignment;
	}

	return max_alignment;
}

bool _gvariant_is_fixed_size(const char *sig)
{
	while (*sig != 0) {
		if (strchr(variable_types, sig[0]))
			return false;

		sig += 1;
	}

	return true;
}

int _gvariant_get_fixed_size(const char *sig)
{
	const char *s = sig;
	const char *p;
	int size = 0;
	int alignment;
	int max_alignment = 1;
	int r;

	while (*s) {
		if (strchr(variable_types, *s))
			return 0;

		if (strchr(fixed_types, *s)) {
			alignment = get_basic_alignment(*s);

			if (alignment > max_alignment)
				max_alignment = alignment;

			size = align_len(size, alignment);
			size += get_basic_fixed_size(*s);
			s++;
			continue;
		}

		if (*s == '}' || *s == ')')
			break;

		p = validate_next_type(s, &alignment);

		if (!p)
			return 0;

		if (alignment > max_alignment)
			max_alignment = alignment;

		size = align_len(size, alignment);

		/* Handle special case of unit type */
		if (s[0] == '(' && s[1] == ')')
			r = 1;
		else
			r = _gvariant_get_fixed_size(s + 1);

		if (r == 0)
			return 0;

		size += r;
		s = p;
	}

	size = align_len(size, max_alignment);

	return size;
}

static inline size_t offset_length(size_t size, size_t n_offsets)
{
	if (size + n_offsets <= 0xff)
		return 1;
	if (size + n_offsets * 2 <= 0xffff)
		return 2;
	if (size + n_offsets * 4 <= 0xffffffff)
		return 4;
	return 8;
}

static inline size_t read_word_le(const void *p, size_t sz) {
	union {
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
	} x;

	if (sz == 1)
		return *(uint8_t *) p;

	memcpy(&x, p, sz);

	if (sz == 2)
		return le16toh(x.u16);
	if (sz == 4)
		return le32toh(x.u32);
	return le64toh(x.u64);
}

static inline void write_word_le(void *p, size_t value, size_t sz) {
	union {
		uint16_t u16;
		uint32_t u32;
		uint64_t u64;
	} x;

	if (sz == 1) {
		*(uint8_t *) p = value;
		return;
	}

	if (sz == 2)
		x.u16 = htole16((uint16_t) value);
	else if (sz == 4)
		x.u32 = htole32((uint32_t) value);
	else
		x.u64 = htole64((uint64_t) value);

	memcpy(p, &x, sz);
}

static bool gvariant_iter_init_internal(struct l_dbus_message_iter *iter,
					struct l_dbus_message *message,
					enum dbus_container_type type,
					const char *sig_start,
					const char *sig_end, const void *data,
					size_t len)
{
	const char *p;
	int i;
	int v;
	char subsig[256];
	unsigned int num_variable = 0;
	unsigned int offset_len = offset_length(len, 0);
	size_t last_offset;
	struct gvariant_type_info {
		uint8_t sig_start;
		uint8_t sig_end;
		bool fixed_size : 1;
		unsigned int alignment : 4;
		size_t end;		/* Index past the end of the type */
	} *children;
	int n_children;

	if (sig_end) {
		size_t len = sig_end - sig_start;
		memcpy(subsig, sig_start, len);
		subsig[len] = '\0';
	} else
		strcpy(subsig, sig_start);

	iter->message = message;
	iter->sig_start = sig_start;
	iter->sig_len = strlen(subsig);
	iter->sig_pos = 0;
	iter->data = data;
	iter->len = len;
	iter->pos = 0;

	if (subsig[0] != '\0') {
		n_children = _gvariant_num_children(subsig);
		if (n_children < 0)
			return false;

		children = l_new(struct gvariant_type_info, n_children);
	} else {
		n_children = 0;

		children = NULL;
	}

	for (p = sig_start, i = 0; i < n_children; i++) {
		int alignment;
		size_t size;
		size_t len;

		children[i].sig_start = p - sig_start;
		p = validate_next_type(p, &alignment);
		children[i].sig_end = p - sig_start;

		len = children[i].sig_end - children[i].sig_start;
		memcpy(subsig, sig_start + children[i].sig_start, len);
		subsig[len] = '\0';

		children[i].alignment = alignment;
		children[i].fixed_size = _gvariant_is_fixed_size(subsig);

		if (children[i].fixed_size) {
			size = _gvariant_get_fixed_size(subsig);
			children[i].end = size;
		} else if (i + 1 < n_children)
			num_variable += 1;
	}

	if (len < num_variable * offset_len)
		goto fail;

	last_offset = len - num_variable * offset_len;

	if (num_variable > 0)
		iter->offsets = iter->data + len - offset_len;
	else
		iter->offsets = NULL;

	for (i = 0, v = 0; i < n_children; i++) {
		size_t o;

		if (children[i].fixed_size) {
			if (i == 0)
				continue;

			o = align_len(children[i-1].end,
					children[i].alignment);
			children[i].end += o;

			if (children[i].end > len)
				goto fail;

			continue;
		}

		if (num_variable == 0) {
			children[i].end = last_offset;
			continue;
		}

		v += 1;
		children[i].end = read_word_le(data + len - offset_len * v,
						offset_len);
		num_variable -= 1;

		if (children[i].end > len)
			goto fail;
	}

	iter->container_type = type;

	if (type == DBUS_CONTAINER_TYPE_ARRAY &&
			!children[0].fixed_size) {
		size_t offset = read_word_le(iter->data + iter->len -
						offset_len, offset_len);
		iter->offsets = iter->data + offset;
	}

	l_free(children);

	return true;

fail:
	l_free(children);
	return false;
}

bool _gvariant_iter_init(struct l_dbus_message_iter *iter,
				struct l_dbus_message *message,
				const char *sig_start, const char *sig_end,
				const void *data, size_t len)
{
	return gvariant_iter_init_internal(iter, message,
						DBUS_CONTAINER_TYPE_STRUCT,
						sig_start, sig_end, data, len);
}

static const void *next_item(struct l_dbus_message_iter *iter,
							size_t *out_item_size)
{
	const void *start;
	const char *p;
	char sig[256];
	int alignment;
	bool fixed_size;
	bool last_member;
	unsigned int sig_len;
	unsigned int offset_len;

	memcpy(sig, iter->sig_start + iter->sig_pos,
			iter->sig_len - iter->sig_pos);
	sig[iter->sig_len - iter->sig_pos] = '\0';

	/*
	 * Find the next type and make a note whether it is the last in the
	 * structure.  Arrays will always have a single complete type, so
	 * last_member will always be true.
	 */
	p = validate_next_type(sig, &alignment);
	if (!p)
		return NULL;

	sig_len = p - sig;

	last_member = *p == '\0';
	sig[sig_len] = '\0';

	fixed_size = _gvariant_is_fixed_size(sig);

	if (iter->container_type != DBUS_CONTAINER_TYPE_ARRAY)
		iter->sig_pos += sig_len;

	iter->pos = align_len(iter->pos, alignment);

	if (fixed_size) {
		*out_item_size = _gvariant_get_fixed_size(sig);
		goto done;
	}

	if (iter->container_type != DBUS_CONTAINER_TYPE_ARRAY && last_member) {
		unsigned int len = iter->len;

		offset_len = offset_length(iter->len, 0);

		if (iter->offsets && iter->offsets + offset_len <
				iter->data + len)
			len = iter->offsets + offset_len - iter->data;

		*out_item_size = len - iter->pos;
		goto done;
	}

	if (iter->offsets >= iter->data + iter->len)
			return NULL;

	offset_len = offset_length(iter->len, 0);
	*out_item_size = read_word_le(iter->offsets, offset_len) - iter->pos;

	/* In structures the offsets are in reverse order */
	if (iter->container_type == DBUS_CONTAINER_TYPE_ARRAY)
		iter->offsets += offset_len;
	else
		iter->offsets -= offset_len;

done:
	start = iter->data + iter->pos;

	if (start >= iter->data + iter->len)
		return NULL;

	iter->pos += *out_item_size;

	return start;
}

bool _gvariant_iter_next_entry_basic(struct l_dbus_message_iter *iter,
					char type, void *out)
{
	size_t item_size = 0;
	const void *start;
	uint8_t uint8_val;
	uint16_t uint16_val;
	uint32_t uint32_val;
	uint64_t uint64_val;
	int16_t int16_val;
	int32_t int32_val;
	int64_t int64_val;

	if (iter->pos >= iter->len)
		return false;

	if (iter->sig_start[iter->sig_pos] != type)
		return false;

	start = next_item(iter, &item_size);
	if (!start)
		return false;

	switch (type) {
	case 'o':
	case 's':
	case 'g':
	{
		const void *end = memchr(start, 0, item_size);

		if (!end)
			return false;

		*(const char**) out = start;
		break;
	}
	case 'b':
		uint8_val = l_get_u8(start);
		*(bool *) out = !!uint8_val;
		break;
	case 'y':
		uint8_val = l_get_u8(start);
		*(uint8_t *) out = uint8_val;
		break;
	case 'n':
		int16_val = l_get_s16(start);
		*(int16_t *) out = int16_val;
		break;
	case 'q':
		uint16_val = l_get_u16(start);
		*(uint16_t *) out = uint16_val;
		break;
	case 'i':
		int32_val = l_get_s32(start);
		*(int32_t *) out = int32_val;
		break;
	case 'h':
	case 'u':
		uint32_val = l_get_u32(start);
		*(uint32_t *) out = uint32_val;
		break;
	case 'x':
		int64_val = l_get_s64(start);
		*(int64_t *) out = int64_val;
		break;
	case 't':
		uint64_val = l_get_u64(start);
		*(uint64_t *) out = uint64_val;
		break;
	case 'd':
		uint64_val = l_get_u64(start);
		*(uint64_t *) out = uint64_val;
		break;
	}

	return true;
}

bool _gvariant_iter_enter_struct(struct l_dbus_message_iter *iter,
					struct l_dbus_message_iter *structure)
{
	bool is_dict = iter->sig_start[iter->sig_pos] == '{';
	bool is_struct = iter->sig_start[iter->sig_pos] == '(';
	const char *sig_start = iter->sig_start + iter->sig_pos + 1;
	const char *sig_end;
	const void *start;
	size_t item_size;
	enum dbus_container_type type;

	if (!is_dict && !is_struct)
		return false;

	start = next_item(iter, &item_size);
	if (!start)
		return false;

	/* For ARRAY containers the sig_pos is never incremented */
	if (iter->container_type == DBUS_CONTAINER_TYPE_ARRAY)
		sig_end = iter->sig_start + iter->sig_len - 1;
	else
		sig_end = iter->sig_start + iter->sig_pos - 1;

	type = is_dict ? DBUS_CONTAINER_TYPE_DICT_ENTRY :
			DBUS_CONTAINER_TYPE_STRUCT;

	return gvariant_iter_init_internal(structure, iter->message,
						type, sig_start, sig_end,
						start, item_size);
}

bool _gvariant_iter_enter_variant(struct l_dbus_message_iter *iter,
					struct l_dbus_message_iter *variant)
{
	size_t item_size;
	const void *start, *end, *nul;
	char signature[256];

	if (iter->sig_start[iter->sig_pos] != 'v')
		return false;

	start = next_item(iter, &item_size);
	if (!start)
		return false;

	/* Find the signature */
	end = start + item_size;
	nul = memrchr(start, 0, end - start);

	if (!nul)
		return false;

	if (end - nul - 1 > 255)
		return false;

	memcpy(signature, nul + 1, end - nul - 1);
	signature[end - nul - 1] = '\0';

	if (_gvariant_num_children(signature) != 1)
		return false;

	return gvariant_iter_init_internal(variant, iter->message,
						DBUS_CONTAINER_TYPE_VARIANT,
						nul + 1, end,
						start, nul - start);
}

bool _gvariant_iter_enter_array(struct l_dbus_message_iter *iter,
					struct l_dbus_message_iter *array)
{
	const char *sig_start;
	const char *sig_end;
	size_t item_size;
	const void *start;

	if (iter->sig_start[iter->sig_pos] != 'a')
		return false;

	sig_start = iter->sig_start + iter->sig_pos + 1;

	start = next_item(iter, &item_size);
	if (!start)
		return false;

	/* For ARRAY containers the sig_pos is never incremented */
	if (iter->container_type == DBUS_CONTAINER_TYPE_ARRAY)
		sig_end = iter->sig_start + iter->sig_len;
	else
		sig_end = iter->sig_start + iter->sig_pos;

	return gvariant_iter_init_internal(array, iter->message,
						DBUS_CONTAINER_TYPE_ARRAY,
						sig_start, sig_end,
						start, item_size);
}

bool _gvariant_iter_skip_entry(struct l_dbus_message_iter *iter)
{
	size_t size;

	if (!next_item(iter, &size))
		return false;

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
		size_t offset_index;
		bool variable_is_last : 1;
	} mark;
};

struct container {
	size_t *offsets;
	size_t offsets_size;
	size_t offset_index;
	size_t start;
	bool variable_is_last : 1;
	enum dbus_container_type type;
	char signature[256];
	uint8_t sigindex;
};

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

static inline bool grow_offsets(struct container *container)
{
	size_t needed;

	if (container->offset_index < container->offsets_size)
		return true;

	needed = container->offsets_size * 2;

	if (needed > USHRT_MAX)
		return false;

	if (needed == 0)
		needed = 8;

	container->offsets = l_realloc(container->offsets,
						needed * sizeof(size_t));
	container->offsets_size = needed;

	return true;
}

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
	l_free(container->offsets);
	l_free(container);
}

static void container_append_struct_offsets(struct container *container,
					struct dbus_builder *builder)
{
	size_t offset_size;
	int i;
	size_t start;

	if (container->variable_is_last)
		container->offset_index -= 1;

	if (container->offset_index == 0)
		return;

	offset_size = offset_length(builder->body_pos,
						container->offset_index);
	i = container->offset_index - 1;

	start = grow_body(builder, offset_size * container->offset_index, 1);

	for (i = container->offset_index - 1; i >= 0; i--) {
		write_word_le(builder->body + start,
				container->offsets[i], offset_size);
		start += offset_size;
	}
}

static void container_append_array_offsets(struct container *container,
					struct dbus_builder *builder)
{
	size_t offset_size;
	unsigned int i;
	size_t start;

	if (container->offset_index == 0)
		return;

	offset_size = offset_length(builder->body_pos,
						container->offset_index);
	start = grow_body(builder, offset_size * container->offset_index, 1);

	for (i = 0; i < container->offset_index; i++) {
		write_word_le(builder->body + start,
				container->offsets[i], offset_size);
		start += offset_size;
	}
}

struct dbus_builder *_gvariant_builder_new(void *body, size_t body_size)
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

	return builder;
}

void _gvariant_builder_free(struct dbus_builder *builder)
{
	if (unlikely(!builder))
		return;

	l_string_free(builder->signature);
	l_queue_destroy(builder->containers,
				(l_queue_destroy_func_t) container_free);
	l_free(builder->body);

	l_free(builder);
}

static bool enter_struct_dict_common(struct dbus_builder *builder,
					const char *signature,
					enum dbus_container_type type,
					const char open,
					const char close)
{
	size_t qlen = l_queue_length(builder->containers);
	struct container *container = l_queue_peek_head(builder->containers);
	int alignment;
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
		end = validate_next_type(start, &alignment) - 1;

		if (*start != open || *end != close)
			return false;

		memcpy(expect, start + 1, end - start - 1);
		expect[end - start - 1] = '\0';

		if (strcmp(expect, signature))
			return false;
	}

	alignment = _gvariant_get_alignment(signature);
	start = grow_body(builder, 0, alignment);

	container = container_new(type, signature, start);
	l_queue_push_head(builder->containers, container);

	return true;
}

bool _gvariant_builder_enter_struct(struct dbus_builder *builder,
					const char *signature)
{
	if (signature[0] && !_gvariant_valid_signature(signature))
		return false;

	return enter_struct_dict_common(builder, signature,
					DBUS_CONTAINER_TYPE_STRUCT, '(', ')');
}

bool _gvariant_builder_enter_dict(struct dbus_builder *builder,
					const char *signature)
{
	if (_gvariant_num_children(signature) != 2)
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

	if (_gvariant_is_fixed_size(container->signature)) {
		int alignment = _gvariant_get_alignment(container->signature);
		grow_body(builder, 0, alignment);

		/* Empty struct or "unit type" is encoded as a zero byte */
		if (container->signature[0] == '\0') {
			size_t start = grow_body(builder, 1, 1);

			memset(builder->body + start, 0, 1);
		}

		parent->variable_is_last = false;
	} else {
		size_t offset;

		if (!grow_offsets(parent))
			return false;

		container_append_struct_offsets(container, builder);
		offset = builder->body_pos - parent->start;
		parent->offsets[parent->offset_index++] = offset;
		parent->variable_is_last = true;
	}

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

bool _gvariant_builder_leave_struct(struct dbus_builder *builder)
{
	return leave_struct_dict_common(builder, DBUS_CONTAINER_TYPE_STRUCT,
					'(', ')');
}

bool _gvariant_builder_leave_dict(struct dbus_builder *builder)
{
	return leave_struct_dict_common(builder,
					DBUS_CONTAINER_TYPE_DICT_ENTRY,
					'{', '}');
}

bool _gvariant_builder_enter_variant(struct dbus_builder *builder,
					const char *signature)
{
	size_t qlen = l_queue_length(builder->containers);
	struct container *container = l_queue_peek_head(builder->containers);
	size_t start;

	if (_gvariant_num_children(signature) != 1)
		return false;

	if (qlen == 1) {
		if (l_string_length(builder->signature) + 1 > 255)
			return false;
	} else if (container->signature[container->sigindex] != 'v')
		return false;

	start = grow_body(builder, 0, 8);

	container = container_new(DBUS_CONTAINER_TYPE_VARIANT,
					signature, start);
	l_queue_push_head(builder->containers, container);

	return true;
}

bool _gvariant_builder_leave_variant(struct dbus_builder *builder)
{
	struct container *container = l_queue_peek_head(builder->containers);
	size_t qlen = l_queue_length(builder->containers);
	struct container *parent;
	size_t start;
	size_t siglen;
	size_t offset;

	if (unlikely(qlen <= 1))
		return false;

	if (unlikely(container->type != DBUS_CONTAINER_TYPE_VARIANT))
		return false;

	l_queue_pop_head(builder->containers);
	qlen -= 1;
	parent = l_queue_peek_head(builder->containers);

	siglen = strlen(container->signature);
	start = grow_body(builder, siglen + 1, 1);
	memset(builder->body + start, 0, 1);
	memcpy(builder->body + start + 1, container->signature, siglen);

	if (!grow_offsets(parent))
		return false;

	offset = builder->body_pos - parent->start;
	parent->offsets[parent->offset_index++] = offset;
	parent->variable_is_last = true;

	if (qlen == 1)
		l_string_append_c(builder->signature, 'v');
	else if (parent->type != DBUS_CONTAINER_TYPE_ARRAY)
		parent->sigindex += 1;

	container_free(container);

	return true;
}

bool _gvariant_builder_enter_array(struct dbus_builder *builder,
					const char *signature)
{
	size_t qlen = l_queue_length(builder->containers);
	struct container *container = l_queue_peek_head(builder->containers);
	size_t start;
	int alignment;

	if (_gvariant_num_children(signature) != 1)
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
		end = validate_next_type(start, &alignment);

		if (*start != 'a')
			return false;

		memcpy(expect, start + 1, end - start - 1);
		expect[end - start - 1] = '\0';

		if (strcmp(expect, signature))
			return false;
	}

	alignment = _gvariant_get_alignment(signature);
	start = grow_body(builder, 0, alignment);

	container = container_new(DBUS_CONTAINER_TYPE_ARRAY, signature, start);
	l_queue_push_head(builder->containers, container);

	return true;
}

bool _gvariant_builder_leave_array(struct dbus_builder *builder)
{
	struct container *container = l_queue_peek_head(builder->containers);
	size_t qlen = l_queue_length(builder->containers);
	struct container *parent;
	size_t offset;

	if (unlikely(qlen <= 1))
		return false;

	if (unlikely(container->type != DBUS_CONTAINER_TYPE_ARRAY))
		return false;

	l_queue_pop_head(builder->containers);
	qlen -= 1;
	parent = l_queue_peek_head(builder->containers);

	if (!_gvariant_is_fixed_size(container->signature))
		container_append_array_offsets(container, builder);

	if (!grow_offsets(parent))
		return false;

	offset = builder->body_pos - parent->start;
	parent->offsets[parent->offset_index++] = offset;
	parent->variable_is_last = true;

	if (qlen == 1)
		l_string_append_printf(builder->signature, "a%s",
							container->signature);
	else if (parent->type != DBUS_CONTAINER_TYPE_ARRAY)
		parent->sigindex += strlen(container->signature) + 1;

	container_free(container);

	return true;
}

bool _gvariant_builder_append_basic(struct dbus_builder *builder,
					char type, const void *value)
{
	struct container *container = l_queue_peek_head(builder->containers);
	size_t start;
	unsigned int alignment;
	size_t len;
	size_t offset;

	if (unlikely(!builder))
		return false;

	if (unlikely(!strchr(simple_types, type)))
		return false;

	alignment = get_basic_alignment(type);
	if (!alignment)
		return false;

	if (l_queue_length(builder->containers) == 1)
		l_string_append_c(builder->signature, type);
	else if (container->signature[container->sigindex] != type)
		return false;

	len = get_basic_fixed_size(type);

	if (len) {
		start = grow_body(builder, len, alignment);
		memcpy(builder->body + start, value, len);
		container->variable_is_last = false;

		if (container->type != DBUS_CONTAINER_TYPE_ARRAY)
			container->sigindex += 1;

		return true;
	}

	if (!grow_offsets(container))
		return false;

	len = strlen(value) + 1;
	start = grow_body(builder, len, alignment);
	memcpy(builder->body + start, value, len);

	offset = builder->body_pos - container->start;
	container->offsets[container->offset_index++] = offset;
	container->variable_is_last = true;

	if (container->type != DBUS_CONTAINER_TYPE_ARRAY)
		container->sigindex += 1;

	return true;
}

bool _gvariant_builder_mark(struct dbus_builder *builder)
{
	struct container *container = l_queue_peek_head(builder->containers);

	builder->mark.container = container;

	if (l_queue_length(builder->containers) == 1)
		builder->mark.sig_end = l_string_length(builder->signature);
	else
		builder->mark.sig_end = container->sigindex;

	builder->mark.body_pos = builder->body_pos;
	builder->mark.offset_index = container->offset_index;
	builder->mark.variable_is_last = container->variable_is_last;

	return true;
}

bool _gvariant_builder_rewind(struct dbus_builder *builder)
{
	struct container *container;

	while ((container = l_queue_peek_head(builder->containers)) !=
				builder->mark.container) {
		container_free(container);
		l_queue_pop_head(builder->containers);
	}

	builder->body_pos = builder->mark.body_pos;
	container->offset_index = builder->mark.offset_index;
	container->variable_is_last = builder->mark.variable_is_last;

	if (l_queue_length(builder->containers) == 1)
		l_string_truncate(builder->signature, builder->mark.sig_end);
	else
		container->sigindex = builder->mark.sig_end;

	return true;
}

char *_gvariant_builder_finish(struct dbus_builder *builder,
				void **body, size_t *body_size)
{
	char *signature;
	struct container *root;
	uint8_t *variant_buf;
	size_t size;

	if (unlikely(!builder))
		return NULL;

	if (unlikely(l_queue_length(builder->containers) != 1))
		return NULL;

	root = l_queue_peek_head(builder->containers);

	signature = l_string_unwrap(builder->signature);
	builder->signature = NULL;

	if (_gvariant_is_fixed_size(signature)) {
		int alignment = _gvariant_get_alignment(signature);
		grow_body(builder, 0, alignment);

		/* Empty struct or "unit type" is encoded as a zero byte */
		if (signature[0] == '\0') {
			size_t start = grow_body(builder, 1, 1);

			memset(builder->body + start, 0, 1);
		}
	} else
		container_append_struct_offsets(root, builder);

	/*
	 * Make sure there's enough space after the body for the variant
	 * signature written here but not included in the body size and
	 * one framing offset value to be written in
	 * _gvariant_message_finalize.
	 */
	size = 3 + strlen(signature) + 8;
	if (builder->body_pos + size > builder->body_size)
		builder->body = l_realloc(builder->body,
						builder->body_pos + size);

	variant_buf = builder->body + builder->body_pos;
	*variant_buf++ = 0;
	*variant_buf++ = '(';
	variant_buf = mempcpy(variant_buf, signature, strlen(signature));
	*variant_buf++ = ')';

	*body = builder->body;
	*body_size = builder->body_pos;
	builder->body = NULL;
	builder->body_size = 0;

	return signature;
}

/*
 * Write the header's framing offset after the body variant which is the
 * last piece of data in the message after the header, the padding and
 * the builder has written the message body.
 */
size_t _gvariant_message_finalize(size_t header_end,
					void *body, size_t body_size,
					const char *signature)
{
	size_t offset_start;
	size_t offset_size;

	offset_start = body_size + 3 + strlen(signature);

	offset_size = offset_length(align_len(header_end, 8) + offset_start, 1);

	write_word_le(body + offset_start, header_end, offset_size);

	return align_len(header_end, 8) + offset_start + offset_size;
}
