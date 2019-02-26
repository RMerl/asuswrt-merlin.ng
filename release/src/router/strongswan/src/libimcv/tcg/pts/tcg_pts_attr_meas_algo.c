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

#include "tcg_pts_attr_meas_algo.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_tcg_pts_attr_meas_algo_t private_tcg_pts_attr_meas_algo_t;

/**
 * PTS Measurement Algorithm
 * see section 3.9.1 of PTS Protocol: Binding to TNC IF-M Specification
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |			Reserved			|	   Hash Algorithm Set	  	|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

#define PTS_MEAS_ALGO_SIZE		4
#define PTS_MEAS_ALGO_RESERVED	0x0000

/**
 * Private data of an tcg_pts_attr_meas_algo_t object.
 */
struct private_tcg_pts_attr_meas_algo_t {

	/**
	 * Public members of tcg_pts_attr_meas_algo_t
	 */
	tcg_pts_attr_meas_algo_t public;

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
	 * Set of algorithms
	 */
	pts_meas_algorithms_t algorithms;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_tcg_pts_attr_meas_algo_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_tcg_pts_attr_meas_algo_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_tcg_pts_attr_meas_algo_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_tcg_pts_attr_meas_algo_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_pts_attr_meas_algo_t *this)
{
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(PTS_MEAS_ALGO_SIZE);
	writer->write_uint16(writer, PTS_MEAS_ALGO_RESERVED);
	writer->write_uint16(writer, this->algorithms);
	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_pts_attr_meas_algo_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	uint16_t reserved, algorithms;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < PTS_MEAS_ALGO_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for PTS Measurement Algorithm");
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_uint16(reader, &reserved);
	reader->read_uint16(reader, &algorithms);
	this->algorithms = algorithms;
	reader->destroy(reader);

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_tcg_pts_attr_meas_algo_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_tcg_pts_attr_meas_algo_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_tcg_pts_attr_meas_algo_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->value.ptr);
		free(this);
	}
}

METHOD(tcg_pts_attr_meas_algo_t, get_algorithms, pts_meas_algorithms_t,
	private_tcg_pts_attr_meas_algo_t *this)
{
	return this->algorithms;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_meas_algo_create(pts_meas_algorithms_t algorithms,
											 bool selection)
{
	private_tcg_pts_attr_meas_algo_t *this;

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
			.get_algorithms = _get_algorithms,
		},
		.type = { PEN_TCG,
				  selection ? TCG_PTS_MEAS_ALGO_SELECTION : TCG_PTS_MEAS_ALGO },
		.algorithms = algorithms,
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}


/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_meas_algo_create_from_data(size_t length,
													   chunk_t data,
													   bool selection)
{
	private_tcg_pts_attr_meas_algo_t *this;

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
			.get_algorithms = _get_algorithms,
		},
		.type = { PEN_TCG,
				  selection ? TCG_PTS_MEAS_ALGO_SELECTION : TCG_PTS_MEAS_ALGO },
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}
