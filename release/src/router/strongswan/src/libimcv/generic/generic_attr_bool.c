/*
 * Copyright (C) 2015 Andreas Steffen
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

#include "generic_attr_bool.h"

#include <imcv.h>
#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_generic_attr_bool_t private_generic_attr_bool_t;

/**
 * Generic PA-TNC attribute containing boolean status value in 32 bit encoding 
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                        Boolean Value                          |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define ATTR_BOOL_SIZE	4

/**
 * Private data of an generic_attr_bool_t object.
 */
struct private_generic_attr_bool_t {

	/**
	 * Public members of generic_attr_bool_t
	 */
	generic_attr_bool_t public;

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
	 * Boolean status value
	 */
	bool status;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_generic_attr_bool_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_generic_attr_bool_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_generic_attr_bool_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_generic_attr_bool_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_generic_attr_bool_t *this)
{
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(ATTR_BOOL_SIZE);
	writer->write_uint32(writer, this->status);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_generic_attr_bool_t *this, uint32_t *offset)
{
	enum_name_t *pa_attr_names;
	bio_reader_t *reader;
	uint32_t status;
  
	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	pa_attr_names = imcv_pa_tnc_attributes->get_names(imcv_pa_tnc_attributes,
													  this->type.vendor_id);

	if (this->value.len != ATTR_BOOL_SIZE)
	{
		DBG1(DBG_TNC, "incorrect attribute size for %N/%N",
			 pen_names, this->type.vendor_id, pa_attr_names, this->type.type);
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_uint32(reader, &status);
	reader->destroy(reader);

	if (status > 1)
	{
		DBG1(DBG_TNC, "%N/%N attribute contains invalid non-boolean value %u",
			 pen_names, this->type.vendor_id, pa_attr_names, this->type.type,
			 status);
		return FAILED;
	}
	this->status = status;

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_generic_attr_bool_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_generic_attr_bool_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_generic_attr_bool_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->value.ptr);
		free(this);
	}
}

METHOD(generic_attr_bool_t, get_status, bool,
	private_generic_attr_bool_t *this)
{
	return this->status;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *generic_attr_bool_create(bool status, pen_type_t type)
{
	private_generic_attr_bool_t *this;

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
			.get_status = _get_status,
		},
		.type = type,
		.status = status,
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *generic_attr_bool_create_from_data(size_t length, chunk_t data,
												  pen_type_t type)
{
	private_generic_attr_bool_t *this;

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
			.get_status = _get_status,
		},
		.type = type,
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

