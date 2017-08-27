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

#include "tcg_pts_attr_file_meas.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <collections/linked_list.h>
#include <utils/debug.h>

typedef struct private_tcg_pts_attr_file_meas_t private_tcg_pts_attr_file_meas_t;

/**
 * File Measurement
 * see section 3.19.2 of PTS Protocol: Binding to TNC IF-M Specification
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |				   Number of Files included						|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |				   Number of Files included						|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |		  Request ID		   |	  Measurement Length	    |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |				   Measurement #1 (Variable Length)				|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |	   Filename Length		 | Filename (Variable Length)		~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~					Filename (Variable Length)					~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |				   Measurement #2 (Variable Length)				|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |	   Filename Length		 | Filename (Variable Length)		~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~					Filename (Variable Length)					~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *					 ...........................
 */

#define PTS_FILE_MEAS_SIZE		12

/**
 * Private data of an tcg_pts_attr_file_meas_t object.
 */
struct private_tcg_pts_attr_file_meas_t {

	/**
	 * Public members of tcg_pts_attr_file_meas_t
	 */
	tcg_pts_attr_file_meas_t public;

	/**
	 * Vendor-specific attribute type
	 */
	pen_type_t type;

	/**
	 * Length of attribute value
	 */
	size_t length;

	/**
	 * Offset up to which attribute value has been processed
	 */
	size_t offset;

	/**
	 * Current position of attribute value pointer
	 */
	chunk_t value;

	/**
	 * Contains complete attribute or current segment
	 */
	chunk_t segment;

	/**
	 * Noskip flag
	 */
	bool noskip_flag;

	/**
	 * Request ID
	 */
	uint16_t request_id;

	/**
	 * Measurement Length
	 */
	uint16_t meas_len;

	/**
	 * Number of Files in attribute
	 */
	uint64_t count;

	/**
	 * PTS File Measurements
	 */
	pts_file_meas_t *measurements;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_tcg_pts_attr_file_meas_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_tcg_pts_attr_file_meas_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_tcg_pts_attr_file_meas_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_tcg_pts_attr_file_meas_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_pts_attr_file_meas_t *this)
{
	bio_writer_t *writer;
	enumerator_t *enumerator;
	u_int64_t count;
	u_int16_t request_id;
	char *filename;
	chunk_t measurement;
	bool first = TRUE;

	if (this->value.ptr)
	{
		return;
	}
	count = this->measurements->get_file_count(this->measurements);
	request_id = this->measurements->get_request_id(this->measurements);

	writer = bio_writer_create(PTS_FILE_MEAS_SIZE);
	writer->write_uint64(writer, count);
	writer->write_uint16(writer, request_id);

	enumerator = this->measurements->create_enumerator(this->measurements);
	while (enumerator->enumerate(enumerator, &filename, &measurement))
	{
		if (first)
		{
			writer->write_uint16(writer, measurement.len);
			first = FALSE;
		}
		writer->write_data  (writer, measurement);
		writer->write_data16(writer, chunk_create(filename, strlen(filename)));
	}
	enumerator->destroy(enumerator);

	if (first)
	{
		/* no attached measurements */
		writer->write_uint16(writer, 0);
	}

	this->value = writer->extract_buf(writer);
	this->segment = this->value;
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_pts_attr_file_meas_t *this, u_int32_t *offset)
{
	bio_reader_t *reader;
	chunk_t measurement, filename;
	status_t status = NEED_MORE;
	char buf[BUF_LEN];
	size_t len;

	if (this->offset == 0)
	{
		if (this->length < PTS_FILE_MEAS_SIZE)
		{
			DBG1(DBG_TNC, "insufficient data for %N/%N", pen_names, PEN_TCG,
						   tcg_attr_names, this->type.type);
			*offset = this->offset;
			return FAILED;
		}
		if (this->value.len < PTS_FILE_MEAS_SIZE)
		{
			return NEED_MORE;
		}
		reader = bio_reader_create(this->value);
		reader->read_uint64(reader, &this->count);
		reader->read_uint16(reader, &this->request_id);
		reader->read_uint16(reader, &this->meas_len);
		this->offset = PTS_FILE_MEAS_SIZE;
		this->value = reader->peek(reader);
		reader->destroy(reader);
	}

	this->measurements = pts_file_meas_create(this->request_id);
	reader = bio_reader_create(this->value);

	while (this->count)
	{
		if (!reader->read_data(reader, this->meas_len, &measurement) ||
			!reader->read_data16(reader, &filename))
		{
			goto end;
		}
		this->offset += this->value.len - reader->remaining(reader);
		this->value = reader->peek(reader);

		len = min(filename.len, BUF_LEN-1);
		memcpy(buf, filename.ptr, len);
		buf[len] = '\0';
		this->measurements->add(this->measurements, buf, measurement);
		this->count--;
	}

	if (this->length != this->offset)
	{
		DBG1(DBG_TNC, "inconsistent length for %N/%N", pen_names, PEN_TCG,
					   tcg_attr_names, this->type.type);
		*offset = this->offset;
		status = FAILED;
	}
	status = SUCCESS;

end:
	reader->destroy(reader);
	return status;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_tcg_pts_attr_file_meas_t *this, chunk_t segment)
{
	this->value = chunk_cat("cc", this->value, segment);
	chunk_free(&this->segment);
	this->segment = this->value;
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_tcg_pts_attr_file_meas_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}
METHOD(pa_tnc_attr_t, destroy, void,
	private_tcg_pts_attr_file_meas_t *this)
{
	if (ref_put(&this->ref))
	{
		DESTROY_IF(this->measurements);
		free(this->segment.ptr);
		free(this);
	}
}

METHOD(tcg_pts_attr_file_meas_t, get_measurements, pts_file_meas_t*,
	private_tcg_pts_attr_file_meas_t *this)
{
	return this->measurements;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_file_meas_create(pts_file_meas_t *measurements)
{
	private_tcg_pts_attr_file_meas_t *this;

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
			.get_measurements = _get_measurements,
		},
		.type = { PEN_TCG, TCG_PTS_FILE_MEAS },
		.request_id = measurements->get_request_id(measurements),
		.count = measurements->get_file_count(measurements),
		.measurements = measurements,
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}


/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_file_meas_create_from_data(size_t length,
													   chunk_t data)
{
	private_tcg_pts_attr_file_meas_t *this;

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
			.get_measurements = _get_measurements,
		},
		.type = { PEN_TCG, TCG_PTS_FILE_MEAS },
		.length = length,
		.segment = chunk_clone(data),
		.ref = 1,
	);

	/* received either complete attribute value or first segment */
	this->value = this->segment;

	return &this->public.pa_tnc_attribute;
}
