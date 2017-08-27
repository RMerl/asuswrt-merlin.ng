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

#include "seg_env.h"

#include "imcv.h"
#include "pa_tnc/pa_tnc_msg.h"
#include "ietf/ietf_attr_pa_tnc_error.h"
#include "tcg/seg/tcg_seg_attr_seg_env.h"

#include <utils/debug.h>
#include <bio/bio_reader.h>
#include <bio/bio_writer.h>

#define BASE_ATTR_ID_PREFIX	0xFF

typedef struct private_seg_env_t private_seg_env_t;

/**
 * Private data of a seg_env_t object.
 */
struct private_seg_env_t {

	/**
	 * Public seg_env_t interface.
	 */
	seg_env_t public;

	/**
	 * Base Attribute ID
	 */
	uint32_t base_attr_id;

	/**
	 * Base Attribute
	 */
	pa_tnc_attr_t *base_attr;

	/**
	 * Base Attribute Info to be used for PA-TNC error messages
	 */
	u_char base_attr_info[8];

	/**
	 * Base Attribute needs more segment data
	 */
	bool need_more;

	/**
	 * Pointer to remaining attribute data to be sent
	 */
	chunk_t data;

	/**
	 * Maximum PA-TNC attribute segment size
	 */
	uint32_t max_seg_size;

};

METHOD(seg_env_t, get_base_attr_id, uint32_t,
	private_seg_env_t *this)
{
	return this->base_attr_id;
}

METHOD(seg_env_t, get_base_attr, pa_tnc_attr_t*,
	private_seg_env_t *this)
{
	return this->need_more ? NULL : this->base_attr->get_ref(this->base_attr);
}

METHOD(seg_env_t, get_base_attr_info, chunk_t,
	private_seg_env_t *this)
{
	return chunk_create(this->base_attr_info, 8);
}

METHOD(seg_env_t, first_segment, pa_tnc_attr_t*,
	private_seg_env_t *this)
{
	pa_tnc_attr_t *seg_env_attr;
	bio_writer_t *writer;
	pen_type_t type;
	chunk_t segment_data, value;
	uint8_t flags, seg_env_flags;

	/* get components of base attribute header and data */
	flags = this->base_attr->get_noskip_flag(this->base_attr) ?
				PA_TNC_ATTR_FLAG_NOSKIP : PA_TNC_ATTR_FLAG_NONE;
	type = this->base_attr->get_type(this->base_attr);

	/* attribute data going into the first segment */
	segment_data = this->data;
	segment_data.len = this->max_seg_size - PA_TNC_ATTR_HEADER_SIZE;

	/* build encoding of the base attribute header and first segment data */
	writer = bio_writer_create(this->max_seg_size);
	writer->write_uint8 (writer, flags);
	writer->write_uint24(writer, type.vendor_id);
	writer->write_uint32(writer, type.type);
	writer->write_uint32(writer, PA_TNC_ATTR_HEADER_SIZE + this->data.len);
	writer->write_data  (writer, segment_data);
	value = writer->extract_buf(writer);
	writer->destroy(writer);
	this->data = chunk_skip(this->data, segment_data.len);

	DBG2(DBG_TNC, "creating first segment for base attribute ID %d (%d bytes)",
		 this->base_attr_id, this->max_seg_size);

	seg_env_flags = SEG_ENV_FLAG_START | SEG_ENV_FLAG_MORE;
	seg_env_attr = tcg_seg_attr_seg_env_create(value, seg_env_flags,
											   this->base_attr_id);
	chunk_free(&value);

	return seg_env_attr;
}

METHOD(seg_env_t, next_segment, pa_tnc_attr_t*,
	private_seg_env_t *this, bool *last)
{
	pa_tnc_attr_t *seg_env_attr;
	chunk_t segment_data;
	uint8_t seg_env_flags;
	bool is_last_segment;

	if (this->data.len == 0)
	{
		/* no more attribute data to segment available */
		return NULL;
	}

	/* attribute data going into the next segment */
	segment_data = this->data;
	segment_data.len = min(this->max_seg_size, this->data.len);
	this->data = chunk_skip(this->data, segment_data.len);

	is_last_segment = (this->data.len == 0);
	if (last)
	{
		*last = is_last_segment;
	}
	DBG2(DBG_TNC, "creating %s segment for base attribute ID %d (%d bytes)",
				   is_last_segment ? "last" : "next", this->base_attr_id,
				   segment_data.len);

	seg_env_flags = is_last_segment ? SEG_ENV_FLAG_NONE : SEG_ENV_FLAG_MORE;
	seg_env_attr = tcg_seg_attr_seg_env_create(segment_data, seg_env_flags,
											   this->base_attr_id);

	return seg_env_attr;
}

METHOD(seg_env_t, add_segment, bool,
	private_seg_env_t *this, chunk_t segment, pa_tnc_attr_t **error)
{
	pen_type_t type, error_code;
	uint32_t attr_offset;
	chunk_t msg_info;
	status_t status;

	this->base_attr->add_segment(this->base_attr, segment);
	status = this->base_attr->process(this->base_attr, &attr_offset);

	if (status != SUCCESS && status != NEED_MORE)
	{
		type = this->base_attr->get_type(this->base_attr);
		if (type.vendor_id == PEN_IETF && type.type == IETF_ATTR_PA_TNC_ERROR)
		{
			/* error while processing a PA-TNC error attribute - abort */
			return FALSE;
		}
		error_code = pen_type_create(PEN_IETF, PA_ERROR_INVALID_PARAMETER);
		msg_info = get_base_attr_info(this);
		*error = ietf_attr_pa_tnc_error_create_with_offset(error_code,
					msg_info, PA_TNC_ATTR_HEADER_SIZE + attr_offset);
		return FALSE;
	}
	this->need_more = (status == NEED_MORE);

	return TRUE;
}

METHOD(seg_env_t, destroy, void,
	private_seg_env_t *this)
{
	DESTROY_IF(this->base_attr);
	free(this);
}

/**
 * See header
 */
seg_env_t *seg_env_create(uint32_t base_attr_id, pa_tnc_attr_t *base_attr,
						  uint32_t max_seg_size)
{
	private_seg_env_t *this;
	chunk_t value;

	base_attr->build(base_attr);
	value = base_attr->get_value(base_attr);

	/**
	 * The PA-TNC attribute header must not be segmented and
	 * there must be at least a first and one next segment
	 */
	if (max_seg_size <  PA_TNC_ATTR_HEADER_SIZE ||
		max_seg_size >= PA_TNC_ATTR_HEADER_SIZE + value.len)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.get_base_attr_id = _get_base_attr_id,
			.get_base_attr = _get_base_attr,
			.get_base_attr_info = _get_base_attr_info,
			.first_segment = _first_segment,
			.next_segment = _next_segment,
			.add_segment = _add_segment,
			.destroy = _destroy,
		},
		.base_attr_id = base_attr_id,
		.base_attr = base_attr->get_ref(base_attr),
		.max_seg_size = max_seg_size,
		.data = base_attr->get_value(base_attr),
	);

	return &this->public;
}

/**
 * See header
 */
seg_env_t *seg_env_create_from_data(uint32_t base_attr_id, chunk_t data,
									uint32_t max_seg_size, pa_tnc_attr_t** error)
{
	private_seg_env_t *this;
	pen_type_t type, error_code;
	bio_reader_t *reader;
	chunk_t msg_info;
	uint32_t offset = 0, attr_offset;
	status_t status;

	INIT(this,
		.public = {
			.get_base_attr_id = _get_base_attr_id,
			.get_base_attr = _get_base_attr,
			.get_base_attr_info = _get_base_attr_info,
			.first_segment = _first_segment,
			.next_segment = _next_segment,
			.add_segment = _add_segment,
			.destroy = _destroy,
		},
		.base_attr_id = base_attr_id,
		.max_seg_size = max_seg_size,
	);

	/* create info field to be used by PA-TNC error messages */
	memset(this->base_attr_info, 0xff, 4);
	htoun32(this->base_attr_info + 4, base_attr_id);
	msg_info = get_base_attr_info(this);

	/* extract from base attribute segment from data */
	reader = bio_reader_create(data);
	this->base_attr = imcv_pa_tnc_attributes->create(imcv_pa_tnc_attributes,
									 reader, TRUE, &offset, msg_info, error);
	reader->destroy(reader);

	if (!this->base_attr)
	{
		destroy(this);
		return NULL;
	}
	status = this->base_attr->process(this->base_attr, &attr_offset);

	if (status != SUCCESS && status != NEED_MORE)
	{
		type = this->base_attr->get_type(this->base_attr);
		if (!(type.vendor_id == PEN_IETF &&
			  type.type == IETF_ATTR_PA_TNC_ERROR))
		{
			error_code = pen_type_create(PEN_IETF, PA_ERROR_INVALID_PARAMETER);
			*error = ietf_attr_pa_tnc_error_create_with_offset(error_code,
						msg_info, PA_TNC_ATTR_HEADER_SIZE + attr_offset);
		}
		destroy(this);
		return NULL;
	}
	this->need_more = (status == NEED_MORE);

	return &this->public;
}

