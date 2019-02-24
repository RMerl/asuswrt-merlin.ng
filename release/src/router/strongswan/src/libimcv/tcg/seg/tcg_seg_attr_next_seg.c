/*
 * Copyright (C) 2014 Andreas Steffen
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

#include "tcg_seg_attr_next_seg.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_tcg_seg_attr_next_seg_t private_tcg_seg_attr_next_seg_t;

typedef enum {
	NEXT_SEG_FLAG_NONE =	0,
	NEXT_SEG_FLAG_CANCEL =	1
} next_seg_flags_t;

/**
 * Next Segment
 * see TCG IF-M Segmentation Specification
 *
 *	                     1                   2				     3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |C|   Reserved  |              Base Attribute ID                |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/**
 * Private data of an tcg_seg_attr_next_seg_t object.
 */
struct private_tcg_seg_attr_next_seg_t {

	/**
	 * Public members of tcg_seg_attr_next_seg_t
	 */
	tcg_seg_attr_next_seg_t public;

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
	 * Cancel flag
	 */
	bool cancel_flag;

	/**
	 * Base Attribute ID
	 */
	uint32_t base_attr_id;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_tcg_seg_attr_next_seg_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_tcg_seg_attr_next_seg_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_tcg_seg_attr_next_seg_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_tcg_seg_attr_next_seg_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_seg_attr_next_seg_t *this)
{
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(TCG_SEG_ATTR_NEXT_SEG_SIZE);
	writer->write_uint8 (writer, this->cancel_flag ? NEXT_SEG_FLAG_CANCEL :
													 NEXT_SEG_FLAG_NONE);
	writer->write_uint24(writer, this->base_attr_id);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_seg_attr_next_seg_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	uint8_t flags;

	*offset = 0;

	if (this->value.len < this->length)
	{
		DBG1(DBG_TNC, "segmentation not allowed for %N", tcg_attr_names,
														 this->type.type);
		return FAILED;
	}
	if (this->value.len < TCG_SEG_ATTR_NEXT_SEG_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for %N", tcg_attr_names,
												  this->type.type);
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_uint8 (reader, &flags);
	reader->read_uint24(reader, &this->base_attr_id);
	reader->destroy(reader);

	this->cancel_flag = (flags & NEXT_SEG_FLAG_CANCEL);

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_tcg_seg_attr_next_seg_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_tcg_seg_attr_next_seg_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_tcg_seg_attr_next_seg_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->value.ptr);
		free(this);
	}
}

METHOD(tcg_seg_attr_next_seg_t, get_base_attr_id, uint32_t,
	private_tcg_seg_attr_next_seg_t *this)
{
	return this->base_attr_id;
}

METHOD(tcg_seg_attr_next_seg_t, get_cancel_flag, bool,
	private_tcg_seg_attr_next_seg_t *this)
{
	return this->cancel_flag;
}

/**
 * Described in header.
 */
pa_tnc_attr_t* tcg_seg_attr_next_seg_create(uint32_t base_attr_id, bool cancel)
{
	private_tcg_seg_attr_next_seg_t *this;

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
			.get_base_attr_id = _get_base_attr_id,
			.get_cancel_flag = _get_cancel_flag,
		},
		.type = { PEN_TCG, TCG_SEG_NEXT_SEG_REQ },
		.base_attr_id = base_attr_id,
		.cancel_flag = cancel,
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_seg_attr_next_seg_create_from_data(size_t length,
													  chunk_t data)
{
	private_tcg_seg_attr_next_seg_t *this;

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
			.get_base_attr_id = _get_base_attr_id,
			.get_cancel_flag = _get_cancel_flag,
		},
		.type = { PEN_TCG, TCG_SEG_NEXT_SEG_REQ },
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}
