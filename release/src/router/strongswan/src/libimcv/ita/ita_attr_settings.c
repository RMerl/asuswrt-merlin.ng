/*
 * Copyright (C) 2012-2014 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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

#define _GNU_SOURCE /* for stdndup() */
#include <string.h>

#include "ita_attr.h"
#include "ita_attr_settings.h"

#include <bio/bio_reader.h>
#include <bio/bio_writer.h>
#include <collections/linked_list.h>
#include <pen/pen.h>
#include <utils/debug.h>

typedef struct private_ita_attr_settings_t private_ita_attr_settings_t;
typedef struct entry_t entry_t;

/**
 * Contains a settings name/value pair
 */
struct entry_t {
	char *name;
	chunk_t value;
};

/**
 * Free an entry_t object
 */
static void free_entry(entry_t *this)
{
	free(this->name);
	free(this->value.ptr);
	free(this);
}

/**
 * ITA Settings
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                         Settings Count                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |        Name Length            |  Name (Variable Length)       ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~                      Name (Variable Length)                   ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |        Value Length           |  Value (Variable Length)      ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~                      Value (Variable Length)                  ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |        Name Length            |  Name (Variable Length)       ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~                      Name (Variable Length)                   ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |        Value Length           |  Value (Variable Length)      ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~                      Value (Variable Length)                  ~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *					 ...........................
 */

#define ITA_SETTINGS_MIN_SIZE	4

/**
 * Private data of an ita_attr_settings_t object.
 */
struct private_ita_attr_settings_t {

	/**
	 * Public members of ita_attr_settings_t
	 */
	ita_attr_settings_t public;

	/**
	 * Vendor-specific attribute type
	 */
	pen_type_t type;

	/**
	 * Length of attribute value
	 */
	size_t length;

	/**
	 * Attribute value or segment
	 */
	chunk_t value;

	/**
	 * Noskip flag
	 */
	bool noskip_flag;

	/**
	 * List of settings
	 */
	linked_list_t *list;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_ita_attr_settings_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_ita_attr_settings_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_ita_attr_settings_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_ita_attr_settings_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_ita_attr_settings_t *this)
{
	bio_writer_t *writer;
	enumerator_t *enumerator;
	entry_t *entry;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(ITA_SETTINGS_MIN_SIZE);
	writer->write_uint32(writer, this->list->get_count(this->list));

	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &entry))
	{
		writer->write_data16(writer, chunk_create(entry->name,
												  strlen(entry->name)));
		writer->write_data16(writer, entry->value);
	}
	enumerator->destroy(enumerator);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_ita_attr_settings_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	uint32_t count;
	chunk_t name, value;
	entry_t *entry;
	status_t status = FAILED;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < ITA_SETTINGS_MIN_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for ITA Settings attribute");
		return FAILED;
	}

	reader = bio_reader_create(this->value);
	reader->read_uint32(reader, &count);

	*offset = ITA_SETTINGS_MIN_SIZE;

	while (count--)
	{
		if (!reader->read_data16(reader, &name))
		{
			DBG1(DBG_TNC, "insufficient data for setting name");
			goto end;
		}
		*offset += 2 + name.len;

		if (!reader->read_data16(reader, &value))
		{
			DBG1(DBG_TNC, "insufficient data for setting value");
			goto end;
		}
		*offset += 2 + value.len;

		/* remove a terminating newline character */
		if (value.len && value.ptr[value.len - 1] == '\n')
		{
			value.len--;
		}
		entry = malloc_thing(entry_t);
		entry->name = strndup(name.ptr, name.len);
		entry->value = chunk_clone(value);
		this->list->insert_last(this->list, entry);
	}
	status = SUCCESS;

end:
	reader->destroy(reader);
	return status;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_ita_attr_settings_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_ita_attr_settings_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_ita_attr_settings_t *this)
{
	if (ref_put(&this->ref))
	{
		this->list->destroy_function(this->list, (void*)free_entry);
		free(this->value.ptr);
		free(this);
	}
}

METHOD(ita_attr_settings_t, add, void,
	private_ita_attr_settings_t *this, char *name, chunk_t value)
{
	entry_t *entry;

	entry = malloc_thing(entry_t);
	entry->name = strdup(name);
	entry->value = chunk_clone(value);
	this->list->insert_last(this->list, entry);
}

CALLBACK(entry_filter, bool,
	void *null, enumerator_t *orig, va_list args)
{
	entry_t *entry;
	chunk_t *value;
	char **name;

	VA_ARGS_VGET(args, name, value);

	while (orig->enumerate(orig, &entry))
	{
		*name = entry->name;
		*value = entry->value;
		return TRUE;
	}
	return FALSE;
}

METHOD(ita_attr_settings_t, create_enumerator, enumerator_t*,
	private_ita_attr_settings_t *this)
{
	return enumerator_create_filter(this->list->create_enumerator(this->list),
									entry_filter, NULL, NULL);
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ita_attr_settings_create(void)
{
	private_ita_attr_settings_t *this;

	INIT(this,
		.public = {
			.pa_tnc_attribute = {
				.get_type = _get_type,
				.get_value = _get_value,
				.get_noskip_flag = _get_noskip_flag,
				.set_noskip_flag = _set_noskip_flag,
				.build = _build,
				.process = _process,
				.add_segment = _add_segment,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.add = _add,
			.create_enumerator = _create_enumerator,
		},
		.type = { PEN_ITA, ITA_ATTR_SETTINGS },
		.list = linked_list_create(),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ita_attr_settings_create_from_data(size_t length, chunk_t data)
{
	private_ita_attr_settings_t *this;

	INIT(this,
		.public = {
			.pa_tnc_attribute = {
				.get_type = _get_type,
				.get_value = _get_value,
				.get_noskip_flag = _get_noskip_flag,
				.set_noskip_flag = _set_noskip_flag,
				.build = _build,
				.process = _process,
				.add_segment = _add_segment,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.add = _add,
			.create_enumerator = _create_enumerator,
		},
		.type = { PEN_ITA, ITA_ATTR_SETTINGS },
		.length = length,
		.value = chunk_clone(data),
		.list = linked_list_create(),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}


