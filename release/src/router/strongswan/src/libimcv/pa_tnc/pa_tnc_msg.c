/*
 * Copyright (C) 2011-2015 Andreas Steffen
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

#include "imcv.h"
#include "pa_tnc_msg.h"
#include "ietf/ietf_attr_pa_tnc_error.h"

#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <collections/linked_list.h>
#include <pen/pen.h>
#include <utils/debug.h>

typedef struct private_pa_tnc_msg_t private_pa_tnc_msg_t;

/**
 *   PA-TNC message header
 *
 *                        1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Version    |                    Reserved                   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       Message Identifier                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define PA_TNC_RESERVED		0x000000

/**
 * Private data of a pa_tnc_msg_t object.
 *
 */
struct private_pa_tnc_msg_t {

	/**
	 * Public pa_tnc_msg_t interface.
	 */
	pa_tnc_msg_t public;

	/**
	 * List of PA-TNC attributes
	 */
	linked_list_t *attributes;

	/**
	 * linked list of PA-TNC error messages
	 */
	linked_list_t *errors;

	/**
	 * Message identifier
	 */
	uint32_t identifier;

	/**
	 * Current PA-TNC Message size
	 */
	size_t msg_len;

	/**
	 * Maximum PA-TNC Message size
	 */
	size_t max_msg_len;

	/**
	 * TRUE if attribute was extracted from data
	 */
	bool from_data;

	/**
	 * Encoded message
	 */
	chunk_t encoding;
};

METHOD(pa_tnc_msg_t, get_encoding, chunk_t,
	private_pa_tnc_msg_t *this)
{
	return this->encoding;
}

METHOD(pa_tnc_msg_t, get_space, size_t,
	private_pa_tnc_msg_t *this)
{
	return this->max_msg_len ? this->max_msg_len - this->msg_len : 0;
}

METHOD(pa_tnc_msg_t, add_attribute, bool,
	private_pa_tnc_msg_t *this, pa_tnc_attr_t *attr)
{
	chunk_t attr_value;
	size_t attr_len;

	if (!this->from_data)
	{
		attr->build(attr);
		attr_value = attr->get_value(attr);
		attr_len = PA_TNC_ATTR_HEADER_SIZE + attr_value.len;

		if (this->max_msg_len && this->msg_len + attr_len > this->max_msg_len)
		{
			/* attribute just does not fit into this message */
			return FALSE;
		}
		this->msg_len += attr_len;
	}
	this->attributes->insert_last(this->attributes, attr);
	return TRUE;
}

METHOD(pa_tnc_msg_t, build, bool,
	private_pa_tnc_msg_t *this)
{
	bio_writer_t *writer;
	enumerator_t *enumerator;
	pa_tnc_attr_t *attr;
	enum_name_t *pa_attr_names;
	pen_type_t type;
	uint8_t flags;
	chunk_t value;
	nonce_gen_t *ng;

	/* generate a nonce as a message identifier */
	ng = lib->crypto->create_nonce_gen(lib->crypto);
	if (!ng || !ng->get_nonce(ng, 4, (uint8_t*)&this->identifier))
	{
		DBG1(DBG_TNC, "failed to generate random PA-TNC message identifier");
		DESTROY_IF(ng);
		return FALSE;
	}
	ng->destroy(ng);
	DBG1(DBG_TNC, "creating PA-TNC message with ID 0x%08x", this->identifier);

	/* build message header */
	writer = bio_writer_create(this->msg_len);
	writer->write_uint8 (writer, PA_TNC_VERSION);
	writer->write_uint24(writer, PA_TNC_RESERVED);
	writer->write_uint32(writer, this->identifier);

	/* append encoded value of PA-TNC attributes */
	enumerator = this->attributes->create_enumerator(this->attributes);
	while (enumerator->enumerate(enumerator, &attr))
	{
		type  = attr->get_type(attr);
		value = attr->get_value(attr);
		flags = attr->get_noskip_flag(attr) ? PA_TNC_ATTR_FLAG_NOSKIP :
											  PA_TNC_ATTR_FLAG_NONE;

		pa_attr_names = imcv_pa_tnc_attributes->get_names(imcv_pa_tnc_attributes,
														  type.vendor_id);
		if (pa_attr_names)
		{
			DBG2(DBG_TNC, "creating PA-TNC attribute type '%N/%N' "
						  "0x%06x/0x%08x", pen_names, type.vendor_id,
						   pa_attr_names, type.type, type.vendor_id, type.type);
		}
		else
		{
			DBG2(DBG_TNC, "creating PA-TNC attribute type '%N' "
						  "0x%06x/0x%08x", pen_names, type.vendor_id,
						   type.vendor_id, type.type);
		}
		DBG3(DBG_TNC, "%B", &value);

		writer->write_uint8 (writer, flags);
		writer->write_uint24(writer, type.vendor_id);
		writer->write_uint32(writer, type.type);
		writer->write_uint32(writer, PA_TNC_ATTR_HEADER_SIZE + value.len);
		writer->write_data  (writer, value);
	}
	enumerator->destroy(enumerator);

	free(this->encoding.ptr);
	this->encoding = writer->extract_buf(writer);
	writer->destroy(writer);

	return TRUE;
}

METHOD(pa_tnc_msg_t, process, status_t,
	private_pa_tnc_msg_t *this)
{
	bio_reader_t *reader;
	pa_tnc_attr_t *attr, *error;
	pen_type_t attr_type;
	chunk_t attr_value;
	uint8_t version;
	uint32_t reserved, offset, attr_offset;
	pen_type_t error_code = { PEN_IETF, PA_ERROR_INVALID_PARAMETER };

	/* process message header */
	if (this->encoding.len < PA_TNC_HEADER_SIZE)
	{
		DBG1(DBG_TNC, "%u bytes insufficient to parse PA-TNC message header",
					   this->encoding.len);
		return FAILED;
	}
	reader = bio_reader_create(this->encoding);
	reader->read_uint8 (reader, &version);
	reader->read_uint24(reader, &reserved);
	reader->read_uint32(reader, &this->identifier);
	DBG1(DBG_TNC, "processing PA-TNC message with ID 0x%08x", this->identifier);

	if (version != PA_TNC_VERSION)
	{
		DBG1(DBG_TNC, "PA-TNC version %u not supported", version);
		error_code = pen_type_create(PEN_IETF, PA_ERROR_VERSION_NOT_SUPPORTED);
		error = ietf_attr_pa_tnc_error_create(error_code, this->encoding);
		goto err;
	}

	/* offset of the first PA-TNC attribute in the PA-TNC message */
	offset = PA_TNC_HEADER_SIZE;

	/* pre-process PA-TNC attributes */
	while (reader->remaining(reader) > 0)
	{
		attr = imcv_pa_tnc_attributes->create(imcv_pa_tnc_attributes,
							reader, FALSE, &offset, this->encoding, &error);
		if (!attr)
		{
			if (error)
			{
				goto err;
			}
			else
			{
				continue;
			}
		}
		attr_value = attr->get_value(attr);
		attr_type  = attr->get_type(attr);

		if (attr->process(attr, &attr_offset) != SUCCESS)
		{
			attr->destroy(attr);

			if (attr_type.vendor_id == PEN_IETF &&
				attr_type.type == IETF_ATTR_PA_TNC_ERROR)
			{
				/* suppress error while processing a PA-TNC error attribute */
				offset += attr_value.len;
				continue;
			}
			error_code = pen_type_create(PEN_IETF, PA_ERROR_INVALID_PARAMETER);
			error = ietf_attr_pa_tnc_error_create_with_offset(error_code,
									this->encoding, offset + attr_offset);
			goto err;
		}
		offset += attr_value.len;
		this->attributes->insert_last(this->attributes, attr);
	}
	reader->destroy(reader);
	return SUCCESS;

err:
	reader->destroy(reader);
	this->errors->insert_last(this->errors, error);
	return VERIFY_ERROR;
}

METHOD(pa_tnc_msg_t, process_ietf_std_errors, bool,
	private_pa_tnc_msg_t *this, linked_list_t *non_fatal_types)
{
	enumerator_t *e1, *e2;
	enum_name_t *pa_attr_names;
	pa_tnc_attr_t *attr;
	pen_type_t type, unsupported_type;
	uint8_t flags;
	bool fatal_error = FALSE;

	e1 = this->attributes->create_enumerator(this->attributes);
	while (e1->enumerate(e1, &attr))
	{
		type = attr->get_type(attr);

		if (type.vendor_id == PEN_IETF && type.type == IETF_ATTR_PA_TNC_ERROR)
		{
			ietf_attr_pa_tnc_error_t *error_attr;
			pen_type_t error_code, *non_fatal_type;
			chunk_t msg_info;
			uint32_t offset;
			bool fatal_current_error = TRUE;

			error_attr = (ietf_attr_pa_tnc_error_t*)attr;
			error_code = error_attr->get_error_code(error_attr);
			msg_info = error_attr->get_msg_info(error_attr);

			/* skip errors from non-IETF namespaces and non PA-TNC msg errors */
			if (error_code.vendor_id != PEN_IETF ||
				error_code.type > PA_ERROR_PA_TNC_MSG_ROOF)
			{
				continue;
			}
			DBG1(DBG_TNC, "received PA-TNC error '%N' concerning message "
				 "0x%08x/0x%08x", pa_tnc_error_code_names, error_code.type,
				 untoh32(msg_info.ptr), untoh32(msg_info.ptr + 4));

			switch (error_code.type)
			{
				case PA_ERROR_INVALID_PARAMETER:
					offset = error_attr->get_offset(error_attr);
					DBG1(DBG_TNC, "  occurred at offset of %u bytes", offset);
					break;
				case PA_ERROR_ATTR_TYPE_NOT_SUPPORTED:
					unsupported_type =
						error_attr->get_unsupported_attr(error_attr, &flags);
					pa_attr_names =
						imcv_pa_tnc_attributes->get_names(imcv_pa_tnc_attributes,
													unsupported_type.vendor_id);
					if (pa_attr_names)
					{
						DBG1(DBG_TNC, "  unsupported attribute type '%N/%N' "
							 "0x%06x/0x%08x, flags 0x%02x",
							 pen_names, unsupported_type.vendor_id,
							 pa_attr_names, unsupported_type.type,
							 unsupported_type.vendor_id, unsupported_type.type,
							 flags);
					}
					else
					{
						DBG1(DBG_TNC, "  unsupported attribute type '%N' "
							 "0x%06x/0x%08x, flags 0x%02x",
							 pen_names, unsupported_type.vendor_id,
							 unsupported_type.vendor_id, unsupported_type.type,
							 flags);
					}
					e2 = non_fatal_types->create_enumerator(non_fatal_types);
					while (e2->enumerate(e2, &non_fatal_type))
					{
						if (pen_type_equals(unsupported_type, *non_fatal_type))
						{
							fatal_current_error = FALSE;
							break;
						}
					}
					e2->destroy(e2);
					break;
				default:
					break;
			}
			if (fatal_current_error)
			{
				fatal_error = TRUE;
			}
		}
	}
	e1->destroy(e1);

	return fatal_error;
}

METHOD(pa_tnc_msg_t, create_attribute_enumerator, enumerator_t*,
	private_pa_tnc_msg_t *this)
{
	return this->attributes->create_enumerator(this->attributes);
}

METHOD(pa_tnc_msg_t, create_error_enumerator, enumerator_t*,
	private_pa_tnc_msg_t *this)
{
	return this->errors->create_enumerator(this->errors);
}

METHOD(pa_tnc_msg_t, destroy, void,
	private_pa_tnc_msg_t *this)
{
	this->attributes->destroy_offset(this->attributes,
									 offsetof(pa_tnc_attr_t, destroy));
	this->errors->destroy_offset(this->errors,
									 offsetof(pa_tnc_attr_t, destroy));
	free(this->encoding.ptr);
	free(this);
}

/**
 * See header
 */
pa_tnc_msg_t *pa_tnc_msg_create(size_t max_msg_len)
{
	private_pa_tnc_msg_t *this;

	INIT(this,
		.public = {
			.get_encoding = _get_encoding,
			.get_space = _get_space,
			.add_attribute = _add_attribute,
			.build = _build,
			.process = _process,
			.process_ietf_std_errors = _process_ietf_std_errors,
			.create_attribute_enumerator = _create_attribute_enumerator,
			.create_error_enumerator = _create_error_enumerator,
			.destroy = _destroy,
		},
		.attributes = linked_list_create(),
		.errors = linked_list_create(),
		.msg_len = PA_TNC_HEADER_SIZE,
		.max_msg_len = max_msg_len,
	);

	return &this->public;
}

/**
 * See header
 */
pa_tnc_msg_t *pa_tnc_msg_create_from_data(chunk_t data)
{
	private_pa_tnc_msg_t *this;

	INIT(this,
		.public = {
			.get_encoding = _get_encoding,
			.get_space = _get_space,
			.add_attribute = _add_attribute,
			.build = _build,
			.process = _process,
			.process_ietf_std_errors = _process_ietf_std_errors,
			.create_attribute_enumerator = _create_attribute_enumerator,
			.create_error_enumerator = _create_error_enumerator,
			.destroy = _destroy,
		},
		.encoding = chunk_clone(data),
		.attributes = linked_list_create(),
		.errors = linked_list_create(),
		.from_data = TRUE,
	);

	return &this->public;
}

