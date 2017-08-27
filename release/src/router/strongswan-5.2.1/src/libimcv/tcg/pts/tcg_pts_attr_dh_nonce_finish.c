/*
 * Copyright (C) 2011-2012 Sansar Choinyambuu
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

#include "tcg_pts_attr_dh_nonce_finish.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_tcg_pts_attr_dh_nonce_finish_t
					private_tcg_pts_attr_dh_nonce_finish_t;

/**
 * PTS DH Nonce Finish
 * see section 3.8.3 of PTS Protocol: Binding to TNC IF-M Specification
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |	Reserved  	|   Nonce Len   |   Selected Hash Algorithm		|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |					D-H Initiator Public Value ...				|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |						D-H Initiator Nonce ...					|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

#define PTS_DH_NONCE_FINISH_SIZE			12
#define PTS_DH_NONCE_FINISH_RESERVED		0x00

/**
 * Private data of an tcg_pts_attr_dh_nonce_finish_t object.
 */
struct private_tcg_pts_attr_dh_nonce_finish_t {

	/**
	 * Public members of tcg_pts_attr_dh_nonce_finish_t
	 */
	tcg_pts_attr_dh_nonce_finish_t public;

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
	 * Selected Hashing Algorithm
	 */
	pts_meas_algorithms_t hash_algo;

	/**
	 * DH Initiator Public Value
	 */
	chunk_t initiator_value;

	/**
	 * DH Initiator Nonce
	 */
	chunk_t initiator_nonce;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_tcg_pts_attr_dh_nonce_finish_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_tcg_pts_attr_dh_nonce_finish_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_tcg_pts_attr_dh_nonce_finish_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_tcg_pts_attr_dh_nonce_finish_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_pts_attr_dh_nonce_finish_t *this)
{
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(PTS_DH_NONCE_FINISH_SIZE);
	writer->write_uint8 (writer, PTS_DH_NONCE_FINISH_RESERVED);
	writer->write_uint8 (writer, this->initiator_nonce.len);
	writer->write_uint16(writer, this->hash_algo);
	writer->write_data  (writer, this->initiator_value);
	writer->write_data  (writer, this->initiator_nonce);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_pts_attr_dh_nonce_finish_t *this, u_int32_t *offset)
{
	bio_reader_t *reader;
	u_int8_t reserved, nonce_len;
	u_int16_t hash_algo;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < PTS_DH_NONCE_FINISH_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for PTS DH Nonce Finish");
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_uint8 (reader, &reserved);
	reader->read_uint8 (reader, &nonce_len);
	reader->read_uint16(reader, &hash_algo);
	reader->read_data(reader, reader->remaining(reader) - nonce_len,
							  &this->initiator_value);
	reader->read_data(reader, nonce_len, &this->initiator_nonce);
	this->hash_algo = hash_algo;
	this->initiator_value = chunk_clone(this->initiator_value);
	this->initiator_nonce = chunk_clone(this->initiator_nonce);
	reader->destroy(reader);

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_tcg_pts_attr_dh_nonce_finish_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_tcg_pts_attr_dh_nonce_finish_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_tcg_pts_attr_dh_nonce_finish_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->value.ptr);
		free(this->initiator_value.ptr);
		free(this->initiator_nonce.ptr);
		free(this);
	}
}

METHOD(tcg_pts_attr_dh_nonce_finish_t, get_hash_algo, pts_meas_algorithms_t,
	private_tcg_pts_attr_dh_nonce_finish_t *this)
{
	return this->hash_algo;
}

METHOD(tcg_pts_attr_dh_nonce_finish_t, get_initiator_value, chunk_t,
	private_tcg_pts_attr_dh_nonce_finish_t *this)
{
	return this->initiator_value;
}

METHOD(tcg_pts_attr_dh_nonce_finish_t, get_initiator_nonce, chunk_t,
	private_tcg_pts_attr_dh_nonce_finish_t *this)
{
	return this->initiator_nonce;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_dh_nonce_finish_create(
											pts_meas_algorithms_t hash_algo,
											chunk_t initiator_value,
   											chunk_t initiator_nonce)
{
	private_tcg_pts_attr_dh_nonce_finish_t *this;

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
			.get_hash_algo = _get_hash_algo,
			.get_initiator_nonce = _get_initiator_nonce,
			.get_initiator_value = _get_initiator_value,
		},
		.type = { PEN_TCG, TCG_PTS_DH_NONCE_FINISH },
		.hash_algo = hash_algo,
		.initiator_value = initiator_value,
		.initiator_nonce = chunk_clone(initiator_nonce),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_dh_nonce_finish_create_from_data(size_t length,
															 chunk_t value)
{
	private_tcg_pts_attr_dh_nonce_finish_t *this;

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
			.get_hash_algo = _get_hash_algo,
			.get_initiator_nonce = _get_initiator_nonce,
			.get_initiator_value = _get_initiator_value,
		},
		.type = { PEN_TCG, TCG_PTS_DH_NONCE_FINISH },
		.length = length,
		.value = chunk_clone(value),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}
