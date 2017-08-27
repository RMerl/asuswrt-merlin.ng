/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include "vici_message.h"
#include "vici_builder.h"

#include <bio/bio_reader.h>
#include <bio/bio_writer.h>

#include <errno.h>

typedef struct private_vici_message_t private_vici_message_t;

/**
 * Private data of an vici_message_t object.
 */
struct private_vici_message_t {

	/**
	 * Public vici_message_t interface.
	 */
	vici_message_t public;

	/**
	 * Message encoding
	 */
	chunk_t encoding;

	/**
	 * Free encoding during destruction?
	 */
	bool cleanup;

	/**
	 * Allocated strings we maintain for get_str()
	 */
	linked_list_t *strings;
};

ENUM(vici_type_names, VICI_START, VICI_END,
	"start",
	"section-start",
	"section-end",
	"key-value",
	"list-start",
	"list-item",
	"list-end",
	"end"
);

/**
 * See header.
 */
bool vici_stringify(chunk_t chunk, char *buf, size_t size)
{
	if (!chunk_printable(chunk, NULL, 0))
	{
		return FALSE;
	}
	snprintf(buf, size, "%.*s", (int)chunk.len, chunk.ptr);
	return TRUE;
}

/**
 * See header.
 */
bool vici_verify_type(vici_type_t type, u_int section, bool list)
{
	if (list)
	{
		if (type != VICI_LIST_END && type != VICI_LIST_ITEM)
		{
			DBG1(DBG_ENC, "'%N' within list", vici_type_names, type);
			return FALSE;
		}
	}
	else
	{
		if (type == VICI_LIST_ITEM || type == VICI_LIST_END)
		{
			DBG1(DBG_ENC, "'%N' outside list", vici_type_names, type);
			return FALSE;
		}
	}
	if (type == VICI_SECTION_END && section == 0)
	{
		DBG1(DBG_ENC, "'%N' outside of section", vici_type_names, type);
		return FALSE;
	}
	if (type == VICI_END)
	{
		if (section)
		{
			DBG1(DBG_ENC, "'%N' within section", vici_type_names, type);
			return FALSE;
		}
		if (list)
		{
			DBG1(DBG_ENC, "'%N' within list", vici_type_names, type);
			return FALSE;
		}
	}
	return TRUE;
}

/**
 * Enumerator parsing message
 */
typedef struct {
	/* implements enumerator */
	enumerator_t public;
	/** reader to parse from */
	bio_reader_t *reader;
	/** section nesting level */
	int section;
	/** currently parsing list? */
	bool list;
	/** string currently enumerating */
	char name[257];
} parse_enumerator_t;

METHOD(enumerator_t, parse_enumerate, bool,
	parse_enumerator_t *this, vici_type_t *out, char **name, chunk_t *value)
{
	u_int8_t type;
	chunk_t data;

	if (!this->reader->remaining(this->reader) ||
		!this->reader->read_uint8(this->reader, &type))
	{
		*out = VICI_END;
		return TRUE;
	}
	if (!vici_verify_type(type, this->section, this->list))
	{
		return FALSE;
	}

	switch (type)
	{
		case VICI_SECTION_START:
			if (!this->reader->read_data8(this->reader, &data) ||
				!vici_stringify(data, this->name, sizeof(this->name)))
			{
				DBG1(DBG_ENC, "invalid '%N' encoding", vici_type_names, type);
				return FALSE;
			}
			*name = this->name;
			this->section++;
			break;
		case VICI_SECTION_END:
			this->section--;
			break;
		case VICI_KEY_VALUE:
			if (!this->reader->read_data8(this->reader, &data) ||
				!vici_stringify(data, this->name, sizeof(this->name)) ||
				!this->reader->read_data16(this->reader, value))
			{
				DBG1(DBG_ENC, "invalid '%N' encoding", vici_type_names, type);
				return FALSE;
			}
			*name = this->name;
			break;
		case VICI_LIST_START:
			if (!this->reader->read_data8(this->reader, &data) ||
				!vici_stringify(data, this->name, sizeof(this->name)))
			{
				DBG1(DBG_ENC, "invalid '%N' encoding", vici_type_names, type);
				return FALSE;
			}
			*name = this->name;
			this->list = TRUE;
			break;
		case VICI_LIST_ITEM:
			this->reader->read_data16(this->reader, value);
			break;
		case VICI_LIST_END:
			this->list = FALSE;
			break;
		case VICI_END:
			return TRUE;
		default:
			DBG1(DBG_ENC, "unknown encoding type: %u", type);
			return FALSE;
	}

	*out = type;

	return TRUE;
}

METHOD(enumerator_t, parse_destroy, void,
	parse_enumerator_t *this)
{
	this->reader->destroy(this->reader);
	free(this);
}

METHOD(vici_message_t, create_enumerator, enumerator_t*,
	private_vici_message_t *this)
{
	parse_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = (void*)_parse_enumerate,
			.destroy = _parse_destroy,
		},
		.reader = bio_reader_create(this->encoding),
	);

	return &enumerator->public;
}

/**
 * Find a value for given vararg key
 */
static bool find_value(private_vici_message_t *this, chunk_t *value,
					   char *fmt, va_list args)
{
	enumerator_t *enumerator;
	char buf[128], *name, *key, *dot, *next;
	int section = 0, keysection = 0;
	bool found = FALSE;
	chunk_t current;
	vici_type_t type;

	vsnprintf(buf, sizeof(buf), fmt, args);
	next = buf;

	enumerator = create_enumerator(this);

	/* descent into section */
	while (TRUE)
	{
		dot = strchr(next, '.');
		if (!dot)
		{
			key = next;
			break;
		}
		*dot = '\0';
		key = next;
		next = dot + 1;
		keysection++;

		while (enumerator->enumerate(enumerator, &type, &name, &current))
		{
			switch (type)
			{
				case VICI_SECTION_START:
					section++;
					if (section == keysection && streq(name, key))
					{
						break;
					}
					continue;
				case VICI_SECTION_END:
					section--;
					continue;
				case VICI_END:
					break;
				default:
					continue;
			}
			break;
		}
	}

	/* find key/value in current section */
	while (enumerator->enumerate(enumerator, &type, &name, &current))
	{
		switch (type)
		{
			case VICI_KEY_VALUE:
				if (section == keysection && streq(key, name))
				{
					*value = current;
					found = TRUE;
					break;
				}
				continue;
			case VICI_SECTION_START:
				section++;
				continue;
			case VICI_SECTION_END:
				section--;
				continue;
			case VICI_END:
				break;
			default:
				continue;
		}
		break;
	}

	enumerator->destroy(enumerator);

	return found;
}

METHOD(vici_message_t, vget_str, char*,
	private_vici_message_t *this, char *def, char *fmt, va_list args)
{
	chunk_t value;
	bool found;
	char *str;

	found = find_value(this, &value, fmt, args);
	if (found)
	{
		if (chunk_printable(value, NULL, 0))
		{
			str = strndup(value.ptr, value.len);
			/* keep a reference to string, so caller doesn't have to care */
			this->strings->insert_last(this->strings, str);
			return str;
		}
	}
	return def;
}

METHOD(vici_message_t, get_str, char*,
	private_vici_message_t *this, char *def, char *fmt, ...)
{
	va_list args;
	char *str;

	va_start(args, fmt);
	str = vget_str(this, def, fmt, args);
	va_end(args);
	return str;
}

METHOD(vici_message_t, vget_int, int,
	private_vici_message_t *this, int def, char *fmt, va_list args)
{
	chunk_t value;
	bool found;
	char buf[32], *pos;
	int ret;

	found = find_value(this, &value, fmt, args);
	if (found)
	{
		if (value.len == 0)
		{
			return def;
		}
		if (chunk_printable(value, NULL, 0))
		{
			snprintf(buf, sizeof(buf), "%.*s", (int)value.len, value.ptr);
			errno = 0;
			ret = strtol(buf, &pos, 0);
			if (errno == 0 && pos == buf + strlen(buf))
			{
				return ret;
			}
		}
	}
	return def;
}

METHOD(vici_message_t, get_int, int,
	private_vici_message_t *this, int def, char *fmt, ...)
{
	va_list args;
	int val;

	va_start(args, fmt);
	val = vget_int(this, def, fmt, args);
	va_end(args);
	return val;
}

METHOD(vici_message_t, vget_value, chunk_t,
	private_vici_message_t *this, chunk_t def, char *fmt, va_list args)
{
	chunk_t value;
	bool found;

	found = find_value(this, &value, fmt, args);
	if (found)
	{
		return value;
	}
	return def;
}

METHOD(vici_message_t, get_value, chunk_t,
	private_vici_message_t *this, chunk_t def, char *fmt, ...)
{
	va_list args;
	chunk_t value;

	va_start(args, fmt);
	value = vget_value(this, def, fmt, args);
	va_end(args);
	return value;
}

METHOD(vici_message_t, get_encoding, chunk_t,
	private_vici_message_t *this)
{
	return this->encoding;
}

/**
 * Private parse context data
 */
struct vici_parse_context_t {
	/** current section nesting level */
	int level;
	/** parse enumerator */
	enumerator_t *e;
};

METHOD(vici_message_t, parse, bool,
	private_vici_message_t *this, vici_parse_context_t *ctx,
	vici_section_cb_t section, vici_value_cb_t kv, vici_value_cb_t li,
	void *user)
{
	vici_parse_context_t root = {};
	char *name, *list = NULL;
	vici_type_t type;
	chunk_t value;
	int base;
	bool ok = TRUE;

	if (!ctx)
	{
		ctx = &root;
		root.e = create_enumerator(this);
	}

	base = ctx->level;

	while (ok)
	{
		ok = ctx->e->enumerate(ctx->e, &type, &name, &value);
		if (ok)
		{
			switch (type)
			{
				case VICI_START:
					/* should never occur */
					continue;
				case VICI_KEY_VALUE:
					if (ctx->level == base && kv)
					{
						name = strdup(name);
						this->strings->insert_last(this->strings, name);
						ok = kv(user, &this->public, name, value);
					}
					continue;
				case VICI_LIST_START:
					if (ctx->level == base)
					{
						list = strdup(name);
						this->strings->insert_last(this->strings, list);
					}
					continue;
				case VICI_LIST_ITEM:
					if (list && li)
					{
						name = strdup(name);
						this->strings->insert_last(this->strings, name);
						ok = li(user, &this->public, list, value);
					}
					continue;
				case VICI_LIST_END:
					if (ctx->level == base)
					{
						list = NULL;
					}
					continue;
				case VICI_SECTION_START:
					if (ctx->level++ == base && section)
					{
						name = strdup(name);
						this->strings->insert_last(this->strings, name);
						ok = section(user, &this->public, ctx, name);
					}
					continue;
				case VICI_SECTION_END:
					if (ctx->level-- == base)
					{
						break;
					}
					continue;
				case VICI_END:
					break;
			}
		}
		break;
	}

	if (ctx == &root)
	{
		root.e->destroy(root.e);
	}
	return ok;
}

METHOD(vici_message_t, dump, bool,
	private_vici_message_t *this, char *label, bool pretty, FILE *out)
{
	enumerator_t *enumerator;
	int ident = 0, delta;
	vici_type_t type, last_type = VICI_START;
	char *name, *term, *sep, *separ, *assign;
	chunk_t value;

	/* pretty print uses indentation on multiple lines */
	if (pretty)
	{
		delta  = 2;
		term   = "\n";
		separ  = "";
		assign = " = ";
	}
	else
	{
		delta  = 0;
		term   = "";
		separ  = " ";
		assign = "=";
	}

	fprintf(out, "%s {%s", label, term);
	ident += delta;

	enumerator = create_enumerator(this);
	while (enumerator->enumerate(enumerator, &type, &name, &value))
	{
		switch (type)
		{
			case VICI_START:
				/* should never occur */
				break;
			case VICI_SECTION_START:
				sep = (last_type != VICI_SECTION_START &&
					   last_type != VICI_START) ? separ : "";
				fprintf(out, "%*s%s%s {%s", ident, "", sep, name, term);
				ident += delta;
				break;
			case VICI_SECTION_END:
				ident -= delta;
				fprintf(out, "%*s}%s", ident, "", term);
				break;
			case VICI_KEY_VALUE:
				sep = (last_type != VICI_SECTION_START &&
					   last_type != VICI_START) ? separ : "";
				if (chunk_printable(value, NULL, ' '))
				{
					fprintf(out, "%*s%s%s%s%.*s%s", ident, "", sep, name,
							assign, (int)value.len, value.ptr, term);
				}
				else
				{
					fprintf(out, "%*s%s%s%s0x%+#B%s", ident, "", sep, name,
							assign, &value, term);
				}
				break;
			case VICI_LIST_START:
				sep = (last_type != VICI_SECTION_START &&
					   last_type != VICI_START) ? separ : "";
				fprintf(out, "%*s%s%s%s[%s", ident, "", sep, name, assign, term);
				ident += delta;
				break;
			case VICI_LIST_END:
				ident -= delta;
				fprintf(out, "%*s]%s", ident, "", term);
				break;
			case VICI_LIST_ITEM:
				sep = (last_type != VICI_LIST_START) ? separ : "";
				if (chunk_printable(value, NULL, ' '))
				{
					fprintf(out, "%*s%s%.*s%s", ident, "", sep,
							(int)value.len, value.ptr, term);
				}
				else
				{
					fprintf(out, "%*s%s0x%+#B%s", ident, "", sep,
							&value, term);
				}
				break;
			case VICI_END:
				fprintf(out, "}\n");
				enumerator->destroy(enumerator);
				return TRUE;
		}
		last_type = type;
	}
	enumerator->destroy(enumerator);
	return FALSE;
}

METHOD(vici_message_t, destroy, void,
	private_vici_message_t *this)
{
	if (this->cleanup)
	{
		chunk_clear(&this->encoding);
	}
	this->strings->destroy_function(this->strings, free);
	free(this);
}

/**
 * See header
 */
vici_message_t *vici_message_create_from_data(chunk_t data, bool cleanup)
{
	private_vici_message_t *this;

	INIT(this,
		.public = {
			.create_enumerator = _create_enumerator,
			.get_str = _get_str,
			.vget_str = _vget_str,
			.get_int = _get_int,
			.vget_int = _vget_int,
			.get_value = _get_value,
			.vget_value = _vget_value,
			.get_encoding = _get_encoding,
			.parse = _parse,
			.dump = _dump,
			.destroy = _destroy,
		},
		.strings = linked_list_create(),
		.encoding = data,
		.cleanup = cleanup,
	);

	return &this->public;
}

/**
 * See header
 */
vici_message_t *vici_message_create_from_enumerator(enumerator_t *enumerator)
{
	vici_builder_t *builder;
	vici_type_t type;
	char *name;
	chunk_t value;

	builder = vici_builder_create();
	while (enumerator->enumerate(enumerator, &type, &name, &value))
	{
		switch (type)
		{
			case VICI_SECTION_START:
			case VICI_LIST_START:
				builder->add(builder, type, name);
				continue;
			case VICI_KEY_VALUE:
				builder->add(builder, type, name, value);
				continue;
			case VICI_LIST_ITEM:
				builder->add(builder, type, value);
				continue;
			case VICI_SECTION_END:
			case VICI_LIST_END:
			default:
				builder->add(builder, type);
				continue;
			case VICI_END:
				break;
		}
		break;
	}
	enumerator->destroy(enumerator);

	return builder->finalize(builder);
}

/**
 * See header
 */
vici_message_t *vici_message_create_from_args(vici_type_t type, ...)
{
	vici_builder_t *builder;
	va_list args;
	char *name;
	chunk_t value;

	builder = vici_builder_create();
	va_start(args, type);
	while (type != VICI_END)
	{
		switch (type)
		{
			case VICI_LIST_START:
			case VICI_SECTION_START:
				name = va_arg(args, char*);
				builder->add(builder, type, name);
				break;
			case VICI_KEY_VALUE:
				name = va_arg(args, char*);
				value = va_arg(args, chunk_t);
				builder->add(builder, type, name, value);
				break;
			case VICI_LIST_ITEM:
				value = va_arg(args, chunk_t);
				builder->add(builder, type, value);
				break;
			case VICI_SECTION_END:
			case VICI_LIST_END:
			default:
				builder->add(builder, type);
				break;
		}
		type = va_arg(args, vici_type_t);
	}
	va_end(args);
	return builder->finalize(builder);
}
