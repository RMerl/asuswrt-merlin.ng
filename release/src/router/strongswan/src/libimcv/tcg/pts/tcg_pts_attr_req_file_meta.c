/*
 * Copyright (C) 2011-2012 Sansar Choinyambuu
 * Copyright (C) 2011-2018 Andreas Steffen
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

#define _GNU_SOURCE /* for stdndup() */
#include <string.h>

#include "tcg_pts_attr_req_file_meta.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

typedef struct private_tcg_pts_attr_req_file_meta_t private_tcg_pts_attr_req_file_meta_t;

/**
 * Request File Metadata
 * see section 3.17.1 of PTS Protocol: Binding to TNC IF-M Specification
 *
 *					   1				   2				   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |	 Flags	 |   Delimiter	|		  Reserved					|
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  ~	   Fully Qualified File Pathname (Variable Length)			~
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define PTS_REQ_FILE_META_SIZE			4
#define PTS_REQ_FILE_META_RESERVED		0x00
#define PTS_REQ_FILE_META_NO_FLAGS		0x00

#define DIRECTORY_CONTENTS_FLAG			(1<<7)

/**
 * Private data of an tcg_pts_attr_req_file_meta_t object.
 */
struct private_tcg_pts_attr_req_file_meta_t {

	/**
	 * Public members of tcg_pts_attr_req_file_meta_t
	 */
	tcg_pts_attr_req_file_meta_t public;

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
	 * Directory Contents flag
	 */
	bool directory_flag;

	/**
	 * UTF8 Encoding of Delimiter Character
	 */
	uint8_t delimiter;

	/**
	 * Fully Qualified File Pathname
	 */
	char *pathname;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_tcg_pts_attr_req_file_meta_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_tcg_pts_attr_req_file_meta_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_tcg_pts_attr_req_file_meta_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_tcg_pts_attr_req_file_meta_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_tcg_pts_attr_req_file_meta_t *this)
{
	uint8_t flags = PTS_REQ_FILE_META_NO_FLAGS;
	chunk_t pathname;
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}
	if (this->directory_flag)
	{
		flags |= DIRECTORY_CONTENTS_FLAG;
	}
	pathname = chunk_create(this->pathname, strlen(this->pathname));

	writer = bio_writer_create(PTS_REQ_FILE_META_SIZE);
	writer->write_uint8 (writer, flags);
	writer->write_uint8 (writer, this->delimiter);
	writer->write_uint16(writer, PTS_REQ_FILE_META_RESERVED);

	writer->write_data  (writer, pathname);
	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_tcg_pts_attr_req_file_meta_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	uint8_t flags;
	uint16_t reserved;
	chunk_t pathname;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < PTS_REQ_FILE_META_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for Request File Metadata");
		return FAILED;
	}

	reader = bio_reader_create(this->value);
	reader->read_uint8 (reader, &flags);
	reader->read_uint8 (reader, &this->delimiter);
	reader->read_uint16(reader, &reserved);

	reader->read_data  (reader, reader->remaining(reader), &pathname);

	this->directory_flag = (flags & DIRECTORY_CONTENTS_FLAG) !=
							PTS_REQ_FILE_META_NO_FLAGS;
	this->pathname = strndup(pathname.ptr, pathname.len);

	reader->destroy(reader);
	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_tcg_pts_attr_req_file_meta_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_tcg_pts_attr_req_file_meta_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_tcg_pts_attr_req_file_meta_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->pathname);
		free(this->value.ptr);
		free(this);
	}
}

METHOD(tcg_pts_attr_req_file_meta_t, get_directory_flag, bool,
	private_tcg_pts_attr_req_file_meta_t *this)
{
	return this->directory_flag;
}

METHOD(tcg_pts_attr_req_file_meta_t, get_delimiter, uint8_t,
	private_tcg_pts_attr_req_file_meta_t *this)
{
	return this->delimiter;
}

METHOD(tcg_pts_attr_req_file_meta_t, get_pathname, char*,
	private_tcg_pts_attr_req_file_meta_t *this)
{
	return this->pathname;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_req_file_meta_create(bool directory_flag,
												 uint8_t delimiter,
												 char *pathname)
{
	private_tcg_pts_attr_req_file_meta_t *this;

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
			.get_directory_flag = _get_directory_flag,
			.get_delimiter = _get_delimiter,
			.get_pathname = _get_pathname,
		},
		.type = { PEN_TCG, TCG_PTS_REQ_FILE_META },
		.directory_flag = directory_flag,
		.delimiter = delimiter,
		.pathname = strdup(pathname),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}


/**
 * Described in header.
 */
pa_tnc_attr_t *tcg_pts_attr_req_file_meta_create_from_data(size_t length,
														   chunk_t data)
{
	private_tcg_pts_attr_req_file_meta_t *this;

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
			.get_directory_flag = _get_directory_flag,
			.get_delimiter = _get_delimiter,
			.get_pathname = _get_pathname,
		},
		.type = { PEN_TCG, TCG_PTS_REQ_FILE_META },
		.length = length,
		.value = chunk_clone(data),
		.ref = 1,
	);

	return &this->public.pa_tnc_attribute;
}
