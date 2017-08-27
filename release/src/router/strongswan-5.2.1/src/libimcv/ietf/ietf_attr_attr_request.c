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

#include "imcv.h"
#include "ietf_attr_attr_request.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <collections/linked_list.h>

#include <utils/debug.h>

typedef struct private_ietf_attr_attr_request_t private_ietf_attr_attr_request_t;

/**
 * PA-TNC Attribute Request type  (see section 4.2.1 of RFC 5792)
 *
 *                      1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Reserved    |           PA-TNC Attribute Vendor ID          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      PA-TNC Attribute Type                    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Reserved    |           PA-TNC Attribute Vendor ID          |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                      PA-TNC Attribute Type                    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define ATTR_REQUEST_ENTRY_SIZE		8

/**
 * Private data of an ietf_attr_attr_request_t object.
 */
struct private_ietf_attr_attr_request_t {

	/**
	 * Public members of ietf_attr_attr_request_t
	 */
	ietf_attr_attr_request_t public;

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
	 * List of requested attribute types
	 */
	linked_list_t *list;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_ietf_attr_attr_request_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_ietf_attr_attr_request_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_ietf_attr_attr_request_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_ietf_attr_attr_request_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_ietf_attr_attr_request_t *this)
{
	bio_writer_t *writer;
	enumerator_t *enumerator;
	pen_type_t *entry;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(ATTR_REQUEST_ENTRY_SIZE *
							   this->list->get_count(this->list));

	enumerator = this->list->create_enumerator(this->list);
	while (enumerator->enumerate(enumerator, &entry))
	{
		writer->write_uint32(writer, entry->vendor_id);
		writer->write_uint32(writer, entry->type);
	}
	enumerator->destroy(enumerator);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(ietf_attr_attr_request_t, add, void,
	private_ietf_attr_attr_request_t *this, pen_t vendor_id, u_int32_t type)
{
	pen_type_t *entry;

	entry = malloc_thing(pen_type_t);
	entry->vendor_id = vendor_id;
	entry->type = type;
	this->list->insert_last(this->list, entry);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_ietf_attr_attr_request_t *this, u_int32_t *offset)
{
	bio_reader_t *reader;
	enum_name_t *pa_attr_names;
	pen_t vendor_id;
	u_int32_t type;
	u_int8_t reserved;
	int count;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}

	count = this->value.len / ATTR_REQUEST_ENTRY_SIZE;
	if (this->value.len != ATTR_REQUEST_ENTRY_SIZE * count)
	{
		DBG1(DBG_TNC, "incorrect attribute length for IETF attribute request");
		return FAILED;
	}

	reader = bio_reader_create(this->value);
	while (count--)
	{
		reader->read_uint8 (reader, &reserved);
		reader->read_uint24(reader, &vendor_id);
		reader->read_uint32(reader, &type);

		pa_attr_names = imcv_pa_tnc_attributes->get_names(imcv_pa_tnc_attributes,
														  vendor_id);
		if (pa_attr_names)
		{
			DBG2(DBG_TNC, "  0x%06x/0x%08x '%N/%N'", vendor_id, type,
							 pen_names, vendor_id, pa_attr_names, type);
		}
		else
		{
			DBG2(DBG_TNC, "  0x%06x/0x%08x '%N'", vendor_id, type,
							 pen_names, vendor_id);
		}
		add(this, vendor_id, type);
	}
	reader->destroy(reader);

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_ietf_attr_attr_request_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_ietf_attr_attr_request_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_ietf_attr_attr_request_t *this)
{
	if (ref_put(&this->ref))
	{
		this->list->destroy_function(this->list, free);
		free(this->value.ptr);
		free(this);
	}
}

METHOD(ietf_attr_attr_request_t, create_enumerator, enumerator_t*,
	private_ietf_attr_attr_request_t *this)
{
	return this->list->create_enumerator(this->list);
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_attr_request_create(pen_t vendor_id, u_int32_t type)
{
	private_ietf_attr_attr_request_t *this;

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
		.type = { PEN_IETF, IETF_ATTR_ATTRIBUTE_REQUEST },
		.list = linked_list_create(),
		.ref = 1,
	);

	if (vendor_id != PEN_RESERVED)
	{
		add(this, vendor_id, type);
	}

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_attr_request_create_from_data(size_t length,
													   chunk_t data)
{
	private_ietf_attr_attr_request_t *this;

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
		.type = { PEN_IETF, IETF_ATTR_ATTRIBUTE_REQUEST },
		.length = length,
		.value = chunk_clone(data),
		.list = linked_list_create(),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

