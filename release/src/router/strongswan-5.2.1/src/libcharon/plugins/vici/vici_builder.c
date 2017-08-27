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

#include "vici_builder.h"

#include <bio/bio_writer.h>

typedef struct private_vici_builder_t private_vici_builder_t;

/**
 * Private data of an vici_builder_t object.
 */
struct private_vici_builder_t {

	/**
	 * Public vici_builder_t interface.
	 */
	vici_builder_t public;

	/**
	 * Writer for elements
	 */
	bio_writer_t *writer;

	/**
	 * Errors encountered
	 */
	u_int error;

	/**
	 * Section nesting level
	 */
	u_int section;

	/**
	 * In list element?
	 */
	bool list;
};

METHOD(vici_builder_t, add, void,
	private_vici_builder_t *this, vici_type_t type, ...)
{
	va_list args;
	char *name = NULL;
	chunk_t value = chunk_empty;

	va_start(args, type);
	switch (type)
	{
		case VICI_SECTION_END:
		case VICI_LIST_END:
		case VICI_END:
			break;
		case VICI_LIST_START:
		case VICI_SECTION_START:
			name = va_arg(args, char*);
			break;
		case VICI_KEY_VALUE:
			name = va_arg(args, char*);
			value = va_arg(args, chunk_t);
			break;
		case VICI_LIST_ITEM:
			value = va_arg(args, chunk_t);
			break;
		default:
			va_end(args);
			this->error++;
			return;
	}
	va_end(args);

	if (value.len > 0xffff)
	{
		this->error++;
		return;
	}
	if (!vici_verify_type(type, this->section, this->list))
	{
		this->error++;
		return;
	}
	if (type != VICI_END)
	{
		this->writer->write_uint8(this->writer, type);
	}
	switch (type)
	{
		case VICI_SECTION_START:
			this->writer->write_data8(this->writer, chunk_from_str(name));
			this->section++;
			break;
		case VICI_SECTION_END:
			this->section--;
			break;
		case VICI_KEY_VALUE:
			this->writer->write_data8(this->writer, chunk_from_str(name));
			this->writer->write_data16(this->writer, value);
			break;
		case VICI_LIST_START:
			this->writer->write_data8(this->writer, chunk_from_str(name));
			this->list = TRUE;
			break;
		case VICI_LIST_ITEM:
			this->writer->write_data16(this->writer, value);
			break;
		case VICI_LIST_END:
			this->list = FALSE;
			break;
		default:
			this->error++;
			break;
	}
}

METHOD(vici_builder_t, vadd_kv, void,
	private_vici_builder_t *this, char *key, char *fmt, va_list args)
{
	char buf[2048];
	ssize_t len;

	len = vsnprintf(buf, sizeof(buf), fmt, args);
	if (len < 0 || len >= sizeof(buf))
	{
		DBG1(DBG_ENC, "vici builder format buffer exceeds limit");
		this->error++;
	}
	else
	{
		add(this, VICI_KEY_VALUE, key, chunk_create(buf, len));
	}
}

METHOD(vici_builder_t, add_kv, void,
	private_vici_builder_t *this, char *key, char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vadd_kv(this, key, fmt, args);
	va_end(args);
}


METHOD(vici_builder_t, vadd_li, void,
	private_vici_builder_t *this, char *fmt, va_list args)
{
	char buf[2048];
	ssize_t len;

	len = vsnprintf(buf, sizeof(buf), fmt, args);
	if (len < 0 || len >= sizeof(buf))
	{
		DBG1(DBG_ENC, "vici builder format buffer exceeds limit");
		this->error++;
	}
	else
	{
		add(this, VICI_LIST_ITEM, chunk_create(buf, len));
	}
}

METHOD(vici_builder_t, add_li, void,
	private_vici_builder_t *this, char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vadd_li(this, fmt, args);
	va_end(args);
}

METHOD(vici_builder_t, begin_section, void,
	private_vici_builder_t *this, char *name)
{
	add(this, VICI_SECTION_START, name);
}

METHOD(vici_builder_t, end_section, void,
	private_vici_builder_t *this)
{
	add(this, VICI_SECTION_END);
}

METHOD(vici_builder_t, begin_list, void,
	private_vici_builder_t *this, char *name)
{
	add(this, VICI_LIST_START, name);
}

METHOD(vici_builder_t, end_list, void,
	private_vici_builder_t *this)
{
	add(this, VICI_LIST_END);
}

METHOD(vici_builder_t, finalize, vici_message_t*,
	private_vici_builder_t *this)
{
	vici_message_t *product;

	if (this->error || this->section || this->list)
	{
		DBG1(DBG_ENC, "vici builder error: %u errors (section: %u, list %u)",
			 this->error, this->section, this->list);
		this->writer->destroy(this->writer);
		free(this);
		return NULL;
	}
	product = vici_message_create_from_data(
								this->writer->extract_buf(this->writer), TRUE);
	this->writer->destroy(this->writer);
	free(this);
	return product;
}

/**
 * See header
 */
vici_builder_t *vici_builder_create()
{
	private_vici_builder_t *this;

	INIT(this,
		.public = {
			.add = _add,
			.add_kv = _add_kv,
			.vadd_kv = _vadd_kv,
			.add_li = _add_li,
			.vadd_li = _vadd_li,
			.begin_section = _begin_section,
			.end_section = _end_section,
			.begin_list = _begin_list,
			.end_list = _end_list,
			.finalize = _finalize,
		},
		.writer = bio_writer_create(0),
	);

	return &this->public;
}
