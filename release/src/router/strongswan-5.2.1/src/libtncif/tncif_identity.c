/*
 * Copyright (C) 2013 Andreas Steffen
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

#include "tncif_identity.h"

#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <pen/pen.h>
#include <utils/debug.h>

typedef struct private_tncif_identity_t private_tncif_identity_t;

/**
 * TNC Identity List Attribute Format (TCG TNC IF-IMV 1.4 Draft)
 *
 *                      1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                        Identity Count                         |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   RESERVED    |            Identity Type Vendor ID            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                         Identity Type                         |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                     Identity Value Length                     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                                                               |
 *  ~                         Identity Value                        ~
 *  |                                                               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   RESERVED    |            Subject Type Vendor ID             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                         Subject Type                          |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |   RESERVED    |        Authentication Method Vendor ID        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                     Authentication Method                     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/**
 * Private data of a tncif_identity_t object.
 *
 */
struct private_tncif_identity_t {

	/**
	 * Public tncif_identity_t interface.
	 */
	tncif_identity_t public;

	/**
	 * Identity Type
	 */
	pen_type_t identity_type;

	/**
	 * Identity Value
	 */
	chunk_t identity_value;

	/**
	 * Subject Type
	 */
	pen_type_t subject_type;

	/**
	 * Authentication Type
	 */
	pen_type_t auth_type;
};

METHOD(tncif_identity_t, get_identity_type, pen_type_t,
	private_tncif_identity_t *this)
{
	return this->identity_type;
}

METHOD(tncif_identity_t, get_identity_value, chunk_t,
	private_tncif_identity_t *this)
{
	return this->identity_value;
}

METHOD(tncif_identity_t, get_subject_type, pen_type_t,
	private_tncif_identity_t *this)
{
	return this->subject_type;
}

METHOD(tncif_identity_t, get_auth_type, pen_type_t,
	private_tncif_identity_t *this)
{
	return this->auth_type;
}

METHOD(tncif_identity_t, build, void,
	private_tncif_identity_t *this, bio_writer_t *writer)
{
	writer->write_uint32(writer, this->identity_type.vendor_id);
	writer->write_uint32(writer, this->identity_type.type);
	writer->write_data32(writer, this->identity_value);
	writer->write_uint32(writer, this->subject_type.vendor_id);
	writer->write_uint32(writer, this->subject_type.type);
	writer->write_uint32(writer, this->auth_type.vendor_id);
	writer->write_uint32(writer, this->auth_type.type);
}

METHOD(tncif_identity_t, process, bool,
	private_tncif_identity_t *this, bio_reader_t *reader)
{
	u_int8_t reserved;
	u_int32_t vendor_id, type;
	chunk_t identity_value;

	if (reader->remaining(reader) < TNCIF_IDENTITY_MIN_SIZE)
	{
		return FALSE;
	}
	reader->read_uint8 (reader, &reserved);
	reader->read_uint24(reader, &vendor_id);
	reader->read_uint32(reader, &type);
	this->identity_type = pen_type_create(vendor_id, type);

	if (!reader->read_data32(reader, &identity_value) ||
		 reader->remaining(reader) < 16)
	{
		return FALSE;
	}
	this->identity_value = chunk_clone(identity_value);

	reader->read_uint8 (reader, &reserved);
	reader->read_uint24(reader, &vendor_id);
	reader->read_uint32(reader, &type);
	this->subject_type = pen_type_create(vendor_id, type);		

	reader->read_uint8 (reader, &reserved);
	reader->read_uint24(reader, &vendor_id);
	reader->read_uint32(reader, &type);
	this->auth_type = pen_type_create(vendor_id, type);

	return TRUE;
}

METHOD(tncif_identity_t, destroy, void,
	private_tncif_identity_t *this)
{
	free(this->identity_value.ptr);
	free(this);
}


/**
 * See header
 */
tncif_identity_t *tncif_identity_create_empty(void)
{
	private_tncif_identity_t *this;

	INIT(this,
		.public = {
			.get_identity_type = _get_identity_type,
			.get_identity_value = _get_identity_value,
			.get_subject_type = _get_subject_type,
			.get_auth_type = _get_auth_type,
			.build = _build,
			.process = _process,
			.destroy = _destroy,
		},
	);

	return &this->public;
}

/**
 * See header
 */
tncif_identity_t *tncif_identity_create(pen_type_t identity_type,
										chunk_t identity_value,
										pen_type_t subject_type,
										pen_type_t auth_type)
{
	private_tncif_identity_t *this;

	this = (private_tncif_identity_t*)tncif_identity_create_empty();
	this->identity_type = identity_type;
	this->identity_value = identity_value;
	this->subject_type = subject_type;
	this->auth_type = auth_type;

	return &this->public;
}

