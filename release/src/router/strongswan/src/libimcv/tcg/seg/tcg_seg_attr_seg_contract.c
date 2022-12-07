/*
 * Copyright (C) 2014-2022 Andreas Steffen
 *
 * Copyright (C) secunet Security Networks AG
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

#include "tcg_seg_attr_seg_contract.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_tcg_seg_attr_seg_contract_t private_tcg_seg_attr_seg_contract_t;

/**
 * Segmentation Contract Request/Response
 * see TCG IF-M Segmentation Specification Version 1.0 Rev. 5, 4 April 2016
 *
 *	                     1                   2				     3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                        Max Message Size                       |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                        Max Segment Size                       |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/**
 * Private data of an tcg_seg_attr_seg_contract_t object.
 */
struct private_tcg_seg_attr_seg_contract_t {

	/**
	 * Public members of tcg_seg_attr_seg_contract_t
	 */
	tcg_seg_attr_seg_contract_t public;

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
	 * Maximum IF-M message size in octets
	 */
	uint32_t max_msg_size;

	/**
	 * Maximum IF-M attribute segment size in octets
	 */
	uint32_t max_seg_size;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_tcg_seg_attr_seg_contract_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_tcg_seg_attr_seg_contract_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_tcg_seg_attr_seg_contract_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_tcg_seg_attr_seg_contract_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_seg_attr_seg_contract_t *this)
{
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(TCG_SEG_ATTR_SEG_CONTRACT_SIZE);
	writer->write_uint32(writer, this->max_msg_size);
	writer->write_uint32(writer, this->max_seg_size);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_seg_attr_seg_contract_t *this, uint32_t *offset)
{
	bio_reader_t *reader;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < TCG_SEG_ATTR_SEG_CONTRACT_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for %N", tcg_attr_names,
												  this->type.type);
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_uint32(reader, &this->max_msg_size);
	reader->read_uint32(reader, &this->max_seg_size);
	reader->destroy(reader);

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_tcg_seg_attr_seg_contract_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_tcg_seg_attr_seg_contract_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_tcg_seg_attr_seg_contract_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->value.ptr);
		free(this);
	}
}

METHOD(tcg_seg_attr_seg_contract_t, get_max_size, void,
	private_tcg_seg_attr_seg_contract_t *this, uint32_t *max_msg_size,
											   uint32_t *max_seg_size)
{
	if (max_msg_size)
	{
		*max_msg_size = this->max_msg_size;
	}
	if (max_seg_size)
	{
		*max_seg_size = this->max_seg_size;
	}
}

/**
 * Described in header.
 */
pa_tnc_attr_t* tcg_seg_attr_seg_contract_create(uint32_t max_msg_size,
												uint32_t max_seg_size,
												bool request)
{
	private_tcg_seg_attr_seg_contract_t *this;

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
			.get_max_size = _get_max_size,
		},
		.type = { PEN_TCG, request ? TCG_SEG_CONTRACT_REQ :
									 TCG_SEG_CONTRACT_RESP },
		.max_msg_size = max_msg_size,
		.max_seg_size = max_seg_size,
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_seg_attr_seg_contract_create_from_data(size_t length,
														  chunk_t data,
														  bool request)
{
	private_tcg_seg_attr_seg_contract_t *this;

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
			.get_max_size = _get_max_size,
		},
		.type = { PEN_TCG, request ? TCG_SEG_CONTRACT_REQ :
									 TCG_SEG_CONTRACT_RESP },
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}
