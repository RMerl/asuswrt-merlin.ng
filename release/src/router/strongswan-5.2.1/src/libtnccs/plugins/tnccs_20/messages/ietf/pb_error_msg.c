/*
 * Copyright (C) 2010 Sansar Choinyambuu
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

#include "pb_error_msg.h"

#include <tnc/tnccs/tnccs.h>

#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <pen/pen.h>
#include <utils/debug.h>

ENUM(pb_tnc_error_code_names, PB_ERROR_UNEXPECTED_BATCH_TYPE,
							  PB_ERROR_VERSION_NOT_SUPPORTED,
	"Unexpected Batch Type",
	"Invalid Parameter",
	"Local Error",
	"Unsupported Mandatory Message",
	"Version Not Supported"
);

typedef struct private_pb_error_msg_t private_pb_error_msg_t;

/**
 *   PB-Error message (see section 4.9 of RFC 5793)
 *
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |    Flags      |              Error Code Vendor ID             |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |           Error Code          |           Reserved            |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                Error Parameters (Variable Length)             |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define ERROR_FLAG_NONE		0x00
#define ERROR_FLAG_FATAL	(1<<7)
#define ERROR_RESERVED	 	0x0000
#define ERROR_HEADER_SIZE	8

/**
 * Private data of a pb_error_msg_t object.
 *
 */
struct private_pb_error_msg_t {
	/**
	 * Public pb_error_msg_t interface.
	 */
	pb_error_msg_t public;

	/**
	 * PB-TNC message type
	 */
	pen_type_t type;

	/**
	 * Fatal flag
	 */
	bool fatal;

	/**
	 * PB Error Code Vendor ID
	 */
	u_int32_t vendor_id;

	/**
	 * PB Error Code
	 */
	u_int16_t error_code;

	/**
	 * PB Error Offset
	 */
	u_int32_t error_offset;

	/**
	 * Bad PB-TNC version received
	 */
	u_int8_t bad_version;

	/**
	 * Encoded message
	 */
	chunk_t encoding;

	/**
	 * reference count
	 */
	refcount_t ref;
};

METHOD(pb_tnc_msg_t, get_type, pen_type_t,
	private_pb_error_msg_t *this)
{
	return this->type;
}

METHOD(pb_tnc_msg_t, get_encoding, chunk_t,
	private_pb_error_msg_t *this)
{
	return this->encoding;
}

METHOD(pb_tnc_msg_t, build, void,
	private_pb_error_msg_t *this)
{
	bio_writer_t *writer;

	if (this->encoding.ptr)
	{
		return;
	}

	/* build message header */
	writer = bio_writer_create(ERROR_HEADER_SIZE);
	writer->write_uint8 (writer, this->fatal ?
						 ERROR_FLAG_FATAL : ERROR_FLAG_NONE);
	writer->write_uint24(writer, this->vendor_id);
	writer->write_uint16(writer, this->error_code);
	writer->write_uint16(writer, ERROR_RESERVED);

	/* build message body */
	if (this->error_code == PB_ERROR_VERSION_NOT_SUPPORTED)
	{
		/* Bad version */
		writer->write_uint8(writer, this->bad_version);
		writer->write_uint8(writer, PB_TNC_VERSION); /* Max version */
		writer->write_uint8(writer, PB_TNC_VERSION); /* Min version */
		writer->write_uint8(writer, 0x00);           /* Reserved */
	}
	else
	{
		/* Error Offset */
		writer->write_uint32(writer, this->error_offset);
	}
	this->encoding = writer->get_buf(writer);
	this->encoding = chunk_clone(this->encoding);
	writer->destroy(writer);
}

METHOD(pb_tnc_msg_t, process, status_t,
	private_pb_error_msg_t *this, u_int32_t *offset)
{
	u_int8_t flags, max_version, min_version;
	u_int16_t reserved;
	bio_reader_t *reader;

	if (this->encoding.len < ERROR_HEADER_SIZE)
	{
		DBG1(DBG_TNC,"%N message is shorter than header size of %u bytes",
			 pb_tnc_msg_type_names, PB_MSG_ERROR, ERROR_HEADER_SIZE);
		*offset = 0;
		return FAILED;
	}

	/* process message header */
	reader = bio_reader_create(this->encoding);
	reader->read_uint8 (reader, &flags);
	reader->read_uint24(reader, &this->vendor_id);
	reader->read_uint16(reader, &this->error_code);
	reader->read_uint16(reader, &reserved);
	this->fatal = (flags & ERROR_FLAG_FATAL) != ERROR_FLAG_NONE;

	if (this->vendor_id == PEN_IETF && reader->remaining(reader) == 4)
	{
		if (this->error_code == PB_ERROR_VERSION_NOT_SUPPORTED)
		{
			reader->read_uint8(reader, &this->bad_version);
			reader->read_uint8(reader, &max_version);
			reader->read_uint8(reader, &min_version);
		}
		else
		{
			reader->read_uint32(reader, &this->error_offset);
		}
	}
	reader->destroy(reader);

	return SUCCESS;
}

METHOD(pb_tnc_msg_t, get_ref, pb_tnc_msg_t*,
	private_pb_error_msg_t *this)
{
	ref_get(&this->ref);
	return &this->public.pb_interface;
}

METHOD(pb_tnc_msg_t, destroy, void,
	private_pb_error_msg_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->encoding.ptr);
		free(this);
	}
}

METHOD(pb_error_msg_t, get_fatal_flag, bool,
	private_pb_error_msg_t *this)
{
	return this->fatal;
}

METHOD(pb_error_msg_t, get_vendor_id, u_int32_t,
	private_pb_error_msg_t *this)
{
	return this->vendor_id;
}

METHOD(pb_error_msg_t, get_error_code, u_int16_t,
	private_pb_error_msg_t *this)
{
	return this->error_code;
}

METHOD(pb_error_msg_t, get_offset, u_int32_t,
	private_pb_error_msg_t *this)
{
	return this->error_offset;
}

METHOD(pb_error_msg_t, get_bad_version, u_int8_t,
	private_pb_error_msg_t *this)
{
	return this->bad_version;
}

METHOD(pb_error_msg_t, set_bad_version, void,
	private_pb_error_msg_t *this, u_int8_t version)
{
	this->bad_version = version;
}

/**
 * See header
 */
pb_tnc_msg_t* pb_error_msg_create(bool fatal, u_int32_t vendor_id,
								  pb_tnc_error_code_t error_code)
{
	private_pb_error_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.get_fatal_flag = _get_fatal_flag,
			.get_vendor_id = _get_vendor_id,
			.get_error_code = _get_error_code,
			.get_offset = _get_offset,
			.get_bad_version = _get_bad_version,
			.set_bad_version = _set_bad_version,
		},
		.type = { PEN_IETF, PB_MSG_ERROR },
		.ref = 1,
		.fatal = fatal,
		.vendor_id = vendor_id,
		.error_code = error_code,
	);

	return &this->public.pb_interface;
}

/**
 * See header
 */
pb_tnc_msg_t* pb_error_msg_create_with_offset(bool fatal, u_int32_t vendor_id,
											  pb_tnc_error_code_t error_code,
											  u_int32_t error_offset)
{
	private_pb_error_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.get_fatal_flag = _get_fatal_flag,
			.get_vendor_id = _get_vendor_id,
			.get_error_code = _get_error_code,
			.get_offset = _get_offset,
			.get_bad_version = _get_bad_version,
			.set_bad_version = _set_bad_version,
		},
		.type = { PEN_IETF, PB_MSG_ERROR },
		.ref = 1,
		.fatal = fatal,
		.vendor_id = vendor_id,
		.error_code = error_code,
		.error_offset = error_offset,
	);

	return &this->public.pb_interface;
}

/**
 * See header
 */
pb_tnc_msg_t *pb_error_msg_create_from_data(chunk_t data)
{
	private_pb_error_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
			.get_fatal_flag = _get_fatal_flag,
			.get_vendor_id = _get_vendor_id,
			.get_error_code = _get_error_code,
			.get_offset = _get_offset,
			.get_bad_version = _get_bad_version,
			.set_bad_version = _set_bad_version,
		},
		.type = { PEN_IETF, PB_MSG_ERROR },
		.ref = 1,
		.encoding = chunk_clone(data),
	);

	return &this->public.pb_interface;
}

