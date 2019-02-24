/*
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

#include "ietf_attr_pa_tnc_error.h"

#include <pa_tnc/pa_tnc_msg.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

ENUM(pa_tnc_error_code_names, PA_ERROR_RESERVED,
							  PA_ERROR_SWIMA_SUBSCRIPTION_ID_REUSE,
	"Reserved",
	"Invalid Parameter",
	"Version Not Supported",
	"Attribute Type Not Supported",
	"SWIMA Error",
	"SWIMA Subscription Denied",
	"SWIMA Response Too Large",
	"SWIMA Subscription Fulfillment Error",
	"SWIMA Subscription ID Reuse"
);

typedef struct private_ietf_attr_pa_tnc_error_t private_ietf_attr_pa_tnc_error_t;

/**
 * PA-TNC Error Attribute Type  (see section 4.2.8 of RFC 5792)
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Reserved   |            PA-TNC Error Code Vendor ID        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                        PA-TNC Error Code                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                 Error Information (Variable Length)           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define PA_ERROR_HEADER_SIZE		8
#define PA_ERROR_RESERVED			0x00

/**
 * All IETF Error Types return the first 8 bytes of the erroneous PA-TNC message
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Version    |            Copy of Reserved                   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       Message Identifier                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define PA_ERROR_MSG_INFO_SIZE		8
#define PA_ERROR_MSG_INFO_MAX_SIZE	1024

/**
 * "Invalid Parameter" Error Code
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                             Offset                            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/**
 * "Version Not Supported" Error Code
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |  Max Version  |  Min Version  |            Reserved           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define PA_ERROR_VERSION_RESERVED	0x0000

/**
 * "Attribute Type Not Supported" Error Code
 *
 *                        1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Flags     |          PA-TNC Attribute Vendor ID           |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                     PA-TNC Attribute Type                     |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define PA_ERROR_ATTR_INFO_SIZE		8

/**
 * Private data of an ietf_attr_pa_tnc_error_t object.
 */
struct private_ietf_attr_pa_tnc_error_t {

	/**
	 * Public members of ietf_attr_pa_tnc_error_t
	 */
	ietf_attr_pa_tnc_error_t public;

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
	 * Vendor-specific error code
	 */
	pen_type_t error_code;

	/**
	 * First 8 bytes of erroneous PA-TNC message
	 */
	chunk_t msg_info;

	/**
	 * Flags of unsupported PA-TNC attribute
	 */
	uint8_t flags;

	/**
	 * Vendor ID and type of unsupported PA-TNC attribute
	 */
	pen_type_t unsupported_type;

	/**
	 * PA-TNC error offset
	 */
	uint32_t error_offset;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(pa_tnc_attr_t, get_type, pen_type_t,
	private_ietf_attr_pa_tnc_error_t *this)
{
	return this->type;
}

METHOD(pa_tnc_attr_t, get_value, chunk_t,
	private_ietf_attr_pa_tnc_error_t *this)
{
	return this->value;
}

METHOD(pa_tnc_attr_t, get_noskip_flag, bool,
	private_ietf_attr_pa_tnc_error_t *this)
{
	return this->noskip_flag;
}

METHOD(pa_tnc_attr_t, set_noskip_flag,void,
	private_ietf_attr_pa_tnc_error_t *this, bool noskip)
{
	this->noskip_flag = noskip;
}

METHOD(pa_tnc_attr_t, build, void,
	private_ietf_attr_pa_tnc_error_t *this)
{
	bio_writer_t *writer;

	if (this->value.ptr)
	{
		return;
	}
	writer = bio_writer_create(PA_ERROR_HEADER_SIZE + PA_ERROR_MSG_INFO_SIZE);
	writer->write_uint8 (writer, PA_ERROR_RESERVED);
	writer->write_uint24(writer, this->error_code.vendor_id);
	writer->write_uint32(writer, this->error_code.type);
	writer->write_data  (writer, this->msg_info);

	if (this->error_code.vendor_id == PEN_IETF)
	{
		switch (this->error_code.type)
		{
			case PA_ERROR_INVALID_PARAMETER:
				writer->write_uint32(writer, this->error_offset);
				break;
			case PA_ERROR_VERSION_NOT_SUPPORTED:
				writer->write_uint8 (writer, PA_TNC_VERSION);
				writer->write_uint8 (writer, PA_TNC_VERSION);
				writer->write_uint16(writer, PA_ERROR_VERSION_RESERVED);
				break;
			case PA_ERROR_ATTR_TYPE_NOT_SUPPORTED:
				writer->write_uint8 (writer, this->flags);
				writer->write_uint24(writer, this->unsupported_type.vendor_id);
				writer->write_uint32(writer, this->unsupported_type.type);
				break;
			default:
				break;
		}
	}
	this->value = writer->extract_buf(writer);
	this->length = this->value.len;
	writer->destroy(writer);
}

METHOD(pa_tnc_attr_t, process, status_t,
	private_ietf_attr_pa_tnc_error_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	uint8_t reserved;
	uint32_t vendor_id, type;

	*offset = 0;

	if (this->value.len < this->length)
	{
		return NEED_MORE;
	}
	if (this->value.len < PA_ERROR_HEADER_SIZE)
	{
		DBG1(DBG_TNC, "insufficient data for PA-TNC error header");
		return FAILED;
	}
	reader = bio_reader_create(this->value);
	reader->read_uint8 (reader, &reserved);
	reader->read_uint24(reader, &this->error_code.vendor_id);
	reader->read_uint32(reader, &this->error_code.type);

	if (this->error_code.vendor_id == PEN_IETF &&
		this->error_code.type <= PA_ERROR_PA_TNC_MSG_ROOF)
	{
		if (!reader->read_data(reader, PA_ERROR_MSG_INFO_SIZE, &this->msg_info))
		{
			reader->destroy(reader);
			DBG1(DBG_TNC, "insufficient data for IETF error information");
			*offset = PA_ERROR_HEADER_SIZE;
			return FAILED;
		}
		this->msg_info = chunk_clone(this->msg_info);

		switch (this->error_code.type)
		{
			case PA_ERROR_INVALID_PARAMETER:
				if (!reader->read_uint32(reader, &this->error_offset))
				{
					reader->destroy(reader);
					DBG1(DBG_TNC, "insufficient data for error offset field");
					*offset = PA_ERROR_HEADER_SIZE + PA_ERROR_MSG_INFO_SIZE;
					return FAILED;
				}
				break;
			case PA_ERROR_ATTR_TYPE_NOT_SUPPORTED:
				if (reader->remaining(reader) < PA_ERROR_ATTR_INFO_SIZE)
				{
					reader->destroy(reader);
					DBG1(DBG_TNC, "insufficient data for unsupported attribute "
								  "information");
					*offset = PA_ERROR_HEADER_SIZE + PA_ERROR_MSG_INFO_SIZE;
					return FAILED;
				}
				reader->read_uint8 (reader, &this->flags);
				reader->read_uint24(reader, &vendor_id);
				reader->read_uint32(reader, &type);
				this->unsupported_type = pen_type_create(vendor_id, type);
				break;
			default:
				break;
		}
	}
	else
	{
		reader->read_data(reader, reader->remaining(reader), &this->msg_info);
		this->msg_info = chunk_clone(this->msg_info);
	}
	reader->destroy(reader);

	return SUCCESS;
}

METHOD(pa_tnc_attr_t, add_segment, void,
	private_ietf_attr_pa_tnc_error_t *this, chunk_t segment)
{
	this->value = chunk_cat("mc", this->value, segment);
}

METHOD(pa_tnc_attr_t, get_ref, pa_tnc_attr_t*,
	private_ietf_attr_pa_tnc_error_t *this)
{
	ref_get(&this->ref);
	return &this->public.pa_tnc_attribute;
}

METHOD(pa_tnc_attr_t, destroy, void,
	private_ietf_attr_pa_tnc_error_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->value.ptr);
		free(this->msg_info.ptr);
		free(this);
	}
}

METHOD(ietf_attr_pa_tnc_error_t, get_error_code, pen_type_t,
	private_ietf_attr_pa_tnc_error_t *this)
{
	return this->error_code;
}

METHOD(ietf_attr_pa_tnc_error_t, get_msg_info, chunk_t,
	private_ietf_attr_pa_tnc_error_t *this)
{
	return this->msg_info;
}

METHOD(ietf_attr_pa_tnc_error_t, get_unsupported_attr, pen_type_t,
	private_ietf_attr_pa_tnc_error_t *this, uint8_t *flags)
{
	if (flags)
	{
		*flags = this->flags;
	}
	return this->unsupported_type;
}

METHOD(ietf_attr_pa_tnc_error_t, set_unsupported_attr, void,
	private_ietf_attr_pa_tnc_error_t *this, uint8_t flags, pen_type_t type)
{
	this->flags = flags;
	this->unsupported_type = type;
}

METHOD(ietf_attr_pa_tnc_error_t, get_offset, uint32_t,
	private_ietf_attr_pa_tnc_error_t *this)
{
	return this->error_offset;
}

/**
 * Generic constructor
 */
static private_ietf_attr_pa_tnc_error_t* create_generic()
{
	private_ietf_attr_pa_tnc_error_t *this;

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
			.get_error_code = _get_error_code,
			.get_msg_info = _get_msg_info,
			.get_unsupported_attr = _get_unsupported_attr,
			.set_unsupported_attr = _set_unsupported_attr,
			.get_offset = _get_offset,
		},
		.type = { PEN_IETF, IETF_ATTR_PA_TNC_ERROR },
		.ref = 1,
	);

	return this;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_pa_tnc_error_create(pen_type_t error_code,
											 chunk_t msg_info)
{
	private_ietf_attr_pa_tnc_error_t *this;

	if (error_code.vendor_id == PEN_IETF &&
		error_code.type <= PA_ERROR_PA_TNC_MSG_ROOF)
	{
		msg_info.len = PA_ERROR_MSG_INFO_SIZE;
	}
	else if (msg_info.len > PA_ERROR_MSG_INFO_MAX_SIZE)
	{
		msg_info.len = PA_ERROR_MSG_INFO_MAX_SIZE;
	}

	this = create_generic();
	this->error_code = error_code;
	this->msg_info = chunk_clone(msg_info);

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_pa_tnc_error_create_with_offset(pen_type_t error_code,
														 chunk_t msg_info,
														 uint32_t error_offset)
{
	private_ietf_attr_pa_tnc_error_t *this;

	/* the first 8 bytes of the erroneous PA-TNC message are sent back */
	msg_info.len = PA_ERROR_MSG_INFO_SIZE;

	this = create_generic();
	this->error_code = error_code;
	this->msg_info = chunk_clone(msg_info);
	this->error_offset = error_offset;

	return &this->public.pa_tnc_attribute;
}

/**
 * Described in header.
 */
pa_tnc_attr_t *ietf_attr_pa_tnc_error_create_from_data(size_t length,
													   chunk_t data)
{
	private_ietf_attr_pa_tnc_error_t *this;

	this = create_generic();
	this->length = length;
	this->value = chunk_clone(data);

	return &this->public.pa_tnc_attribute;
}
