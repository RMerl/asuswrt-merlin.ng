/*
 * Copyright (C) 2011-2014 Andreas Steffen
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

#include "ietf_attr_product_info.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_ietf_attr_product_info_t private_ietf_attr_product_info_t;

/**
 * PA-TNC Product Information type  (see section 4.2.2 of RFC 5792)
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |               Product Vendor ID               |  Product ID   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  Product ID   |         Product Name (Variable Length)        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define PRODUCT_INFO_MIN_SIZE	5

/**
 * Private data of an ietf_attr_product_info_t object.
 */
struct private_ietf_attr_product_info_t {

	/**
	 * Public members of ietf_attr_product_info_t
	 */
	ietf_attr_product_info_t public;

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
	 * Product vendor ID
	 */
	pen_t product_vendor_id;

	/**
	 * Product ID
	 */
	uint16_t product_id;

	/**
	 * Product Name
	 */
	chunk_t product_name;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_ietf_attr_product_info_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_ietf_attr_product_info_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_ietf_attr_product_info_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_ietf_attr_product_info_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_ietf_attr_product_info_t *this)
{
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(PRODUCT_INFO_MIN_SIZE);
	writer->write_uint24(writer, this->product_vendor_id);
	writer->write_uint16(writer, this->product_id);
	writer->write_data  (writer, this->product_name);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_ietf_attr_product_info_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	chunk_t product_name;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < PRODUCT_INFO_MIN_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for IETF product information");
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_uint24(reader, &this->product_vendor_id);
	reader->read_uint16(reader, &this->product_id);
	reader->read_data  (reader, reader->remaining(reader), &product_name);
	reader->destroy(reader);

	if (!this->product_vendor_id && this->product_id)
	{
		DBG1(DBG_TNC, "IETF product information vendor ID is 0 "
					  "but product ID is not 0");
		*offset = 3;
		return FAILED;
	}
	this->product_name = chunk_clone(product_name);

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_ietf_attr_product_info_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_ietf_attr_product_info_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_ietf_attr_product_info_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->product_name.ptr);
		free(this->value.ptr);
		free(this);
	}
}

METHOD(ietf_attr_product_info_t, get_info, chunk_t,
	private_ietf_attr_product_info_t *this, pen_t *vendor_id, uint16_t *id)
{
	if (vendor_id)
	{
		*vendor_id = this->product_vendor_id;
	}
	if (id)
	{
		*id = this->product_id;
	}
	return this->product_name;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_product_info_create(pen_t vendor_id, uint16_t id,
											 chunk_t name)
{
	private_ietf_attr_product_info_t *this;

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
			.get_info = _get_info,
		},
		.type = { PEN_IETF, IETF_ATTR_PRODUCT_INFORMATION },
		.product_vendor_id = vendor_id,
		.product_id = id,
		.product_name = chunk_clone(name),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_product_info_create_from_data(size_t length,
													   chunk_t data)
{
	private_ietf_attr_product_info_t *this;

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
			.get_info = _get_info,
		},
		.type = { PEN_IETF, IETF_ATTR_PRODUCT_INFORMATION },
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

