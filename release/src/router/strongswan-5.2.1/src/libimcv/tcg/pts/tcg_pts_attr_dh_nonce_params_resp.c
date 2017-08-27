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

#include "tcg_pts_attr_dh_nonce_params_resp.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_tcg_pts_attr_dh_nonce_params_resp_t
					private_tcg_pts_attr_dh_nonce_params_resp_t;

/**
 * PTS DH Nonce Parameters Response
 * see section 3.8.2 of PTS Protocol: Binding to TNC IF-M Specification
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |					Reserved  					|   Nonce Len   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |		Selected D-H Group		|   	Hash Algorithm Set		|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |						D-H Responder Nonce ...					|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |					D-H Responder Public Value ...				|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

#define PTS_DH_NONCE_PARAMS_RESP_SIZE			16
#define PTS_DH_NONCE_PARAMS_RESP_RESERVED		0x0000

/**
 * Private data of an tcg_pts_attr_dh_nonce_params_resp_t object.
 */
struct private_tcg_pts_attr_dh_nonce_params_resp_t {

	/**
	 * Public members of tcg_pts_attr_dh_nonce_params_resp_t
	 */
	tcg_pts_attr_dh_nonce_params_resp_t public;

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
	 * Selected Diffie Hellman group
	 */
	pts_dh_group_t dh_group;

	/**
	 * Supported Hashing Algorithms
	 */
	pts_meas_algorithms_t hash_algo_set;

	/**
	 * DH Responder Nonce
	 */
	chunk_t responder_nonce;

	/**
	 * DH Responder Public Value
	 */
	chunk_t responder_value;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_tcg_pts_attr_dh_nonce_params_resp_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_tcg_pts_attr_dh_nonce_params_resp_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_tcg_pts_attr_dh_nonce_params_resp_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_tcg_pts_attr_dh_nonce_params_resp_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_pts_attr_dh_nonce_params_resp_t *this)
{
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(PTS_DH_NONCE_PARAMS_RESP_SIZE);
	writer->write_uint24(writer, PTS_DH_NONCE_PARAMS_RESP_RESERVED);
	writer->write_uint8 (writer, this->responder_nonce.len);
	writer->write_uint16(writer, this->dh_group);
	writer->write_uint16(writer, this->hash_algo_set);
	writer->write_data  (writer, this->responder_nonce);
	writer->write_data  (writer, this->responder_value);

	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_pts_attr_dh_nonce_params_resp_t *this, u_int32_t *offset)
{
	bio_reader_t *reader;
	u_int32_t reserved;
	u_int8_t nonce_len;
	u_int16_t dh_group, hash_algo_set;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < PTS_DH_NONCE_PARAMS_RESP_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for PTS DH Nonce Parameters Response");
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_uint24(reader, &reserved);
	reader->read_uint8 (reader, &nonce_len);
	reader->read_uint16(reader, &dh_group);
	reader->read_uint16(reader, &hash_algo_set);
	reader->read_data(reader, nonce_len, &this->responder_nonce);
	reader->read_data(reader, reader->remaining(reader), &this->responder_value);
	this->dh_group = dh_group;
	this->hash_algo_set = hash_algo_set;
	this->responder_nonce = chunk_clone(this->responder_nonce);
	this->responder_value = chunk_clone(this->responder_value);
	reader->destroy(reader);

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_tcg_pts_attr_dh_nonce_params_resp_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_tcg_pts_attr_dh_nonce_params_resp_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_tcg_pts_attr_dh_nonce_params_resp_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->value.ptr);
		free(this->responder_nonce.ptr);
		free(this->responder_value.ptr);
		free(this);
	}
}

METHOD(tcg_pts_attr_dh_nonce_params_resp_t, get_dh_group, pts_dh_group_t,
	private_tcg_pts_attr_dh_nonce_params_resp_t *this)
{
	return this->dh_group;
}

METHOD(tcg_pts_attr_dh_nonce_params_resp_t, get_hash_algo_set,
	   pts_meas_algorithms_t, private_tcg_pts_attr_dh_nonce_params_resp_t *this)
{
	return this->hash_algo_set;
}

METHOD(tcg_pts_attr_dh_nonce_params_resp_t, get_responder_nonce, chunk_t,
	private_tcg_pts_attr_dh_nonce_params_resp_t *this)
{
	return this->responder_nonce;
}

METHOD(tcg_pts_attr_dh_nonce_params_resp_t, get_responder_value, chunk_t,
	private_tcg_pts_attr_dh_nonce_params_resp_t *this)
{
	return this->responder_value;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_dh_nonce_params_resp_create(pts_dh_group_t dh_group,
											pts_meas_algorithms_t hash_algo_set,
   											chunk_t responder_nonce,
											chunk_t responder_value)
{
	private_tcg_pts_attr_dh_nonce_params_resp_t *this;

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
			.get_dh_group = _get_dh_group,
			.get_hash_algo_set = _get_hash_algo_set,
			.get_responder_nonce = _get_responder_nonce,
			.get_responder_value = _get_responder_value,
		},
		.type = { PEN_TCG, TCG_PTS_DH_NONCE_PARAMS_RESP },
		.dh_group = dh_group,
		.hash_algo_set = hash_algo_set,
		.responder_nonce = chunk_clone(responder_nonce),
		.responder_value = responder_value,
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_dh_nonce_params_resp_create_from_data(size_t length,
																  chunk_t value)
{
	private_tcg_pts_attr_dh_nonce_params_resp_t *this;

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
			.get_dh_group = _get_dh_group,
			.get_hash_algo_set = _get_hash_algo_set,
			.get_responder_nonce = _get_responder_nonce,
			.get_responder_value = _get_responder_value,
		},
		.type = { PEN_TCG, TCG_PTS_DH_NONCE_PARAMS_RESP },
		.length = length,
		.value = chunk_clone(value),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}
