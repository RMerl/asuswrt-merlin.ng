/*
 * Copyright (C) 2010 Sansar Choinyanbuu
 * Copyright (C) 2010-2015 Andreas Steffen
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

#include "pb_tnc_batch.h"
#include "messages/ietf/pb_error_msg.h"
#include "messages/ietf/pb_pa_msg.h"
#include "state_machine/pb_tnc_state_machine.h"

#include <tnc/tnccs/tnccs.h>

#include <collections/linked_list.h>
#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <pen/pen.h>
#include <utils/debug.h>

ENUM(pb_tnc_batch_type_names, PB_BATCH_CDATA, PB_BATCH_CLOSE,
	"CDATA",
	"SDATA",
	"RESULT",
	"CRETRY",
	"SRETRY",
	"CLOSE"
);

typedef struct private_pb_tnc_batch_t private_pb_tnc_batch_t;

/**
 *   PB-Batch Header (see section 4.1 of RFC 5793)
 *
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |    Version    |D|     Reserved                        | B-Type|
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                       Batch Length                            |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define PB_TNC_BATCH_FLAG_NONE		0x00
#define PB_TNC_BATCH_FLAG_D			(1<<7)

/**
 *   PB-TNC Message (see section 4.2 of RFC 5793)
 *
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |     Flags     |               PB-TNC Vendor ID                |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                       PB-TNC Message Type                     |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |                      PB-TNC Message Length                    |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *     |               PB-TNC Message Value (Variable Length)          |
 *     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define PB_TNC_FLAG_NONE			0x00
#define PB_TNC_FLAG_NOSKIP			(1<<7)

#define PB_TNC_RESERVED_MSG_TYPE	0xffffffff

/**
 * Private data of a pb_tnc_batch_t object.
 *
 */
struct private_pb_tnc_batch_t {
	/**
	 * Public pb_pa_msg_t interface.
	 */
	pb_tnc_batch_t public;

	/**
	 * from TNC server if TRUE, from TNC client if FALSE
	 */
	bool is_server;

	/**
	 * PB-TNC Batch type
	 */
	pb_tnc_batch_type_t type;

	/**
	 * Current PB-TNC Batch size
	 */
	size_t batch_len;

	/**
	 * Maximum PB-TNC Batch size
	 */
	size_t max_batch_len;

	/**
	 * linked list of PB-TNC messages
	 */
	linked_list_t *messages;

	/**
	 * linked list of PB-TNC error messages
	 */
	linked_list_t *errors;

	/**
	 * Encoded message
	 */
	chunk_t encoding;

	/**
	 * Offset into encoding (used for error reporting)
	 */
	uint32_t offset;
};

METHOD(pb_tnc_batch_t, get_type, pb_tnc_batch_type_t,
	private_pb_tnc_batch_t *this)
{
	return this->type;
}

METHOD(pb_tnc_batch_t, get_encoding, chunk_t,
	private_pb_tnc_batch_t *this)
{
	return this->encoding;
}

METHOD(pb_tnc_batch_t, add_msg, bool,
	private_pb_tnc_batch_t *this, pb_tnc_msg_t* msg)
{
	enum_name_t *msg_type_names;
	chunk_t msg_value;
	pen_type_t msg_type;
	size_t msg_len;

	msg->build(msg);
	msg_value = msg->get_encoding(msg);
	msg_len = PB_TNC_MSG_HEADER_SIZE + msg_value.len;

	if (this->batch_len + msg_len > this->max_batch_len)
	{
		/* message just does not fit into this batch */
		return FALSE;
	}
	this->batch_len += msg_len;

	msg_type = msg->get_type(msg);
	switch (msg_type.vendor_id)
	{
		default:
		case PEN_IETF:
			msg_type_names = pb_tnc_msg_type_names;
			break;
		case PEN_TCG:
			msg_type_names = pb_tnc_tcg_msg_type_names;
			break;
		case PEN_ITA:
			msg_type_names = pb_tnc_ita_msg_type_names;
			break;
	}
	DBG2(DBG_TNC, "adding %N/%N message", pen_names, msg_type.vendor_id,
										  msg_type_names, msg_type.type);
	this->messages->insert_last(this->messages, msg);
	return TRUE;
}

METHOD(pb_tnc_batch_t, build, void,
	private_pb_tnc_batch_t *this)
{
	uint8_t version;
	uint32_t msg_len;
	chunk_t msg_value;
	enumerator_t *enumerator;
	pen_type_t msg_type;
	pb_tnc_msg_t *msg;
	pb_tnc_msg_info_t *msg_infos;
	bio_writer_t *writer;

	/* Set wrong PB-TNC version for testing purposes to force a PB-TNC error */
	version = lib->settings->get_int(lib->settings,
						"%s.plugins.tnccs-20.tests.pb_tnc_version",
						 PB_TNC_VERSION, lib->ns);

	/* build PB-TNC batch header */
	writer = bio_writer_create(this->batch_len);
	writer->write_uint8 (writer, version);
	writer->write_uint8 (writer, this->is_server ?
								 PB_TNC_BATCH_FLAG_D : PB_TNC_BATCH_FLAG_NONE);
	writer->write_uint16(writer, this->type);
	writer->write_uint32(writer, this->batch_len);

	/* build PB-TNC messages */
	enumerator = this->messages->create_enumerator(this->messages);
	while (enumerator->enumerate(enumerator, &msg))
	{
		uint8_t flags = PB_TNC_FLAG_NONE;

		/* build PB-TNC message */
		msg_value = msg->get_encoding(msg);
		msg_len = PB_TNC_MSG_HEADER_SIZE + msg_value.len;
		msg_type = msg->get_type(msg);
		switch (msg_type.vendor_id)
		{
			default:
			case PEN_IETF:
				msg_infos = pb_tnc_msg_infos;
				break;
			case PEN_TCG:
				msg_infos = pb_tnc_tcg_msg_infos;
				break;
			case PEN_ITA:
				msg_infos = pb_tnc_ita_msg_infos;
				break;
		}
		if (msg_infos[msg_type.type].has_noskip_flag)
		{
			flags |= PB_TNC_FLAG_NOSKIP;
		}
		writer->write_uint8 (writer, flags);
		writer->write_uint24(writer, msg_type.vendor_id);
		writer->write_uint32(writer, msg_type.type);
		writer->write_uint32(writer, msg_len);
		writer->write_data  (writer, msg_value);
	}
	enumerator->destroy(enumerator);

	this->encoding = writer->extract_buf(writer);
	writer->destroy(writer);
}

METHOD(pb_tnc_batch_t, process_header, status_t,
	private_pb_tnc_batch_t *this, bool directionality, bool is_server,
	bool *from_server)
{
	bio_reader_t *reader;
	pb_tnc_msg_t *msg;
	pb_error_msg_t *err_msg;
	uint8_t version, flags, reserved, type;
	uint32_t batch_len;

	if (this->encoding.len < PB_TNC_BATCH_HEADER_SIZE)
	{
		DBG1(DBG_TNC, "%u bytes insufficient to parse PB-TNC batch header",
					   this->encoding.len);
		msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
							PB_ERROR_INVALID_PARAMETER, 0);
		goto fatal;
	}

	reader = bio_reader_create(this->encoding);
	reader->read_uint8 (reader, &version);
	reader->read_uint8 (reader, &flags);
	reader->read_uint8 (reader, &reserved);
	reader->read_uint8 (reader, &type);
	reader->read_uint32(reader, &batch_len);
	reader->destroy(reader);

	/* Version */
	if (version != PB_TNC_VERSION)
	{
		DBG1(DBG_TNC, "unsupported TNCCS batch version 0x%02x", version);
		msg = pb_error_msg_create(TRUE, PEN_IETF,
								  PB_ERROR_VERSION_NOT_SUPPORTED);
		err_msg = (pb_error_msg_t*)msg;
		err_msg->set_bad_version(err_msg, version);
		goto fatal;
	}

	/* Directionality */
	*from_server = (flags & PB_TNC_BATCH_FLAG_D) != PB_TNC_BATCH_FLAG_NONE;

	if (directionality & (*from_server == is_server))
	{
		DBG1(DBG_TNC, "wrong Directionality: batch is from a PB-TNC %s",
					   is_server ? "server" : "client");
		msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
					   PB_ERROR_INVALID_PARAMETER, 1);
		goto fatal;
	}

	/* Batch Type */
	this->type = type & 0x0F;
	if (this->type > PB_BATCH_ROOF)
	{
		DBG1(DBG_TNC, "unknown PB-TNC batch type: %d", this->type);
		msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
							PB_ERROR_INVALID_PARAMETER, 3);
		goto fatal;
	}

	/* Batch Length */
	if (this->encoding.len != batch_len)
	{
		DBG1(DBG_TNC, "%u bytes of data is not equal to batch length of %u bytes",
					   this->encoding.len, batch_len);
		msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
							PB_ERROR_INVALID_PARAMETER, 4);
		goto fatal;
	}

	this->offset = PB_TNC_BATCH_HEADER_SIZE;

	return SUCCESS;

fatal:
	this->errors->insert_last(this->errors, msg);
	return FAILED;
}

static status_t process_tnc_msg(private_pb_tnc_batch_t *this)
{
	bio_reader_t *reader;
	pb_tnc_msg_t *pb_tnc_msg, *msg;
	pb_tnc_msg_info_t *msg_infos;
	uint8_t flags;
	uint32_t vendor_id, msg_type, msg_len, offset;
	chunk_t data, msg_value;
	bool noskip_flag;
	enum_name_t *msg_type_names;
	pen_type_t msg_pen_type;
	status_t status;

	data = chunk_skip(this->encoding, this->offset);

	if (data.len < PB_TNC_MSG_HEADER_SIZE)
	{
		DBG1(DBG_TNC, "%u bytes insufficient to parse PB-TNC message header",
					  data.len);
		msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
							PB_ERROR_INVALID_PARAMETER, this->offset);
		goto fatal;
	}

	reader = bio_reader_create(data);
	reader->read_uint8 (reader, &flags);
	reader->read_uint24(reader, &vendor_id);
	reader->read_uint32(reader, &msg_type);
	reader->read_uint32(reader, &msg_len);
	reader->destroy(reader);

	noskip_flag = (flags & PB_TNC_FLAG_NOSKIP) != PB_TNC_FLAG_NONE;

	if (msg_len > data.len)
	{
		DBG1(DBG_TNC, "%u bytes insufficient to parse PB-TNC message", data.len);
		msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
							PB_ERROR_INVALID_PARAMETER, this->offset + 8);
		goto fatal;
	}

	if (vendor_id == PEN_RESERVED)
	{
		DBG1(DBG_TNC, "Vendor ID 0x%06x is reserved", PEN_RESERVED);
		msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
							PB_ERROR_INVALID_PARAMETER, this->offset + 1);
		goto fatal;

	}

	if (msg_type == PB_TNC_RESERVED_MSG_TYPE)
	{
		DBG1(DBG_TNC, "PB-TNC message Type 0x%08x is reserved",
			 PB_TNC_RESERVED_MSG_TYPE);
		msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
							PB_ERROR_INVALID_PARAMETER, this->offset + 4);
		goto fatal;
	}

	if (vendor_id == PEN_IETF && msg_type <= PB_MSG_ROOF)
	{
		if (msg_type == PB_MSG_EXPERIMENTAL && noskip_flag)
		{
			DBG1(DBG_TNC, "reject IETF/PB-Experimental message with "
						  "NOSKIP flag set");
			msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
							PB_ERROR_UNSUPPORTED_MANDATORY_MSG, this->offset);
			goto fatal;
		}
		msg_type_names = pb_tnc_msg_type_names;
		msg_infos = pb_tnc_msg_infos;
	}
	else if (vendor_id == PEN_TCG && msg_type <= PB_TCG_MSG_ROOF &&
									 msg_type >  PB_TCG_MSG_RESERVED)
	{
		msg_type_names = pb_tnc_tcg_msg_type_names;
		msg_infos = pb_tnc_tcg_msg_infos;
	}
	else if (vendor_id == PEN_ITA && msg_type <= PB_ITA_MSG_ROOF &&
									 msg_type >  PB_ITA_MSG_NOSKIP_TEST)
	{
		msg_type_names = pb_tnc_ita_msg_type_names;
		msg_infos = pb_tnc_ita_msg_infos;
	}
	else
	{
		if (msg_len < PB_TNC_MSG_HEADER_SIZE)
		{
			DBG1(DBG_TNC, "%u bytes too small for PB-TNC message length",
						   msg_len);
			msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
							PB_ERROR_INVALID_PARAMETER, this->offset + 8);
			goto fatal;
		}

		if (noskip_flag)
		{
			DBG1(DBG_TNC, "reject PB-TNC message (0x%06x/0x%08x)",
						   vendor_id, msg_type);
			msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
							PB_ERROR_UNSUPPORTED_MANDATORY_MSG, this->offset);
			goto fatal;
		}
		else
		{
			DBG1(DBG_TNC, "ignore PB-TNC message (0x%06x/0x%08x)",
						   vendor_id, msg_type);
			this->offset += msg_len;
			return SUCCESS;
		}
	}

	if (msg_infos[msg_type].has_noskip_flag != TRUE_OR_FALSE &&
		msg_infos[msg_type].has_noskip_flag != noskip_flag)
	{
		DBG1(DBG_TNC, "%N/%N message must%s have NOSKIP flag set",
			 pen_names, vendor_id, msg_type_names, msg_type,
			 msg_infos[msg_type].has_noskip_flag ? "" : " not");
		msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
							PB_ERROR_INVALID_PARAMETER, this->offset);
		goto fatal;
	}

	if (msg_len < msg_infos[msg_type].min_size ||
	   (msg_infos[msg_type].exact_size && 
		msg_len != msg_infos[msg_type].min_size))
	{
		DBG1(DBG_TNC, "%N/%N message length must be %s %u bytes but is %u bytes",
			 pen_names, vendor_id, msg_type_names, msg_type,
			 msg_infos[msg_type].exact_size ? "exactly" : "at least",
			 msg_infos[msg_type].min_size, msg_len);
		msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
							PB_ERROR_INVALID_PARAMETER, this->offset);
		goto fatal;
	}

	if (msg_infos[msg_type].in_result_batch && this->type != PB_BATCH_RESULT)
	{
		if (this->is_server)
		{
			DBG1(DBG_TNC,"reject %N/%N message received from a PB-TNC client",
				 pen_names, vendor_id, msg_type_names, msg_type);
			msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
								PB_ERROR_INVALID_PARAMETER, this->offset);
			goto fatal;
		}
		else
		{
			DBG1(DBG_TNC,"ignore %N/%N message not received within RESULT batch",
				 pen_names, vendor_id, msg_type_names, msg_type);
			this->offset += msg_len;
			return SUCCESS;
		}
	}

	DBG2(DBG_TNC, "processing %N/%N message (%u bytes)", pen_names, vendor_id,
		 msg_type_names, msg_type, msg_len);
	data.len = msg_len;
	msg_value = chunk_skip(data, PB_TNC_MSG_HEADER_SIZE);
	msg_pen_type = pen_type_create(vendor_id, msg_type);
	pb_tnc_msg = pb_tnc_msg_create_from_data(msg_pen_type, msg_value);

	status = pb_tnc_msg->process(pb_tnc_msg, &offset);
	if (status == FAILED || status == VERIFY_ERROR)
	{
		msg = pb_error_msg_create_with_offset(TRUE, PEN_IETF,
							PB_ERROR_INVALID_PARAMETER, this->offset + offset);
		this->errors->insert_last(this->errors, msg);
	}
	if (status == FAILED)
	{
		pb_tnc_msg->destroy(pb_tnc_msg);
		return FAILED;
	}
	this->messages->insert_last(this->messages, pb_tnc_msg);
	this->offset += msg_len;
	return status;

fatal:
	this->errors->insert_last(this->errors, msg);
	return FAILED;
}

METHOD(pb_tnc_batch_t, process, status_t,
	private_pb_tnc_batch_t *this, pb_tnc_state_machine_t *state_machine)
{
	pb_tnc_msg_t *msg;
	status_t status = SUCCESS;

	if (!state_machine->receive_batch(state_machine, this->type))
	{
		DBG1(DBG_TNC, "unexpected PB-TNC batch type: %N",
					   pb_tnc_batch_type_names, this->type);
		msg = pb_error_msg_create(TRUE, PEN_IETF,
								  PB_ERROR_UNEXPECTED_BATCH_TYPE);
		this->errors->insert_last(this->errors, msg);
		return FAILED;
	}

	/* Register an empty CDATA batch with the state machine */
	if (this->type == PB_BATCH_CDATA)
	{
		state_machine->set_empty_cdata(state_machine,
									   this->offset == this->encoding.len);
	}

	while (this->offset < this->encoding.len)
	{
		switch (process_tnc_msg(this))
		{
			case FAILED:
				return FAILED;
			case VERIFY_ERROR:
				status = VERIFY_ERROR;
				break;
			case SUCCESS:
			default:
				break;
		}
	}
	return status;
}

METHOD(pb_tnc_batch_t, create_msg_enumerator, enumerator_t*,
	private_pb_tnc_batch_t *this)
{
	return this->messages->create_enumerator(this->messages);
}

METHOD(pb_tnc_batch_t, create_error_enumerator, enumerator_t*,
	private_pb_tnc_batch_t *this)
{
	return this->errors->create_enumerator(this->errors);
}

METHOD(pb_tnc_batch_t, destroy, void,
	private_pb_tnc_batch_t *this)
{
	this->messages->destroy_offset(this->messages,
								   offsetof(pb_tnc_msg_t, destroy));
	this->errors->destroy_offset(this->errors,
								   offsetof(pb_tnc_msg_t, destroy));
	free(this->encoding.ptr);
	free(this);
}

/**
 * See header
 */
pb_tnc_batch_t* pb_tnc_batch_create(bool is_server, pb_tnc_batch_type_t type,
									size_t max_batch_len)
{
	private_pb_tnc_batch_t *this;

	INIT(this,
		.public = {
			.get_type = _get_type,
			.get_encoding = _get_encoding,
			.add_msg = _add_msg,
			.build = _build,
			.process = _process,
			.create_msg_enumerator = _create_msg_enumerator,
			.create_error_enumerator = _create_error_enumerator,
			.destroy = _destroy,
		},
		.is_server = is_server,
		.type = type,
		.max_batch_len = max_batch_len,
		.batch_len = PB_TNC_BATCH_HEADER_SIZE,
		.messages = linked_list_create(),
		.errors = linked_list_create(),
	);

	DBG2(DBG_TNC, "creating PB-TNC %N batch", pb_tnc_batch_type_names, type);

	return &this->public;
}

/**
 * See header
 */
pb_tnc_batch_t* pb_tnc_batch_create_from_data(chunk_t data)
{
	private_pb_tnc_batch_t *this;

	INIT(this,
		.public = {
			.get_type = _get_type,
			.get_encoding = _get_encoding,
			.add_msg = _add_msg,
			.build = _build,
			.process_header = _process_header,
			.process = _process,
			.create_msg_enumerator = _create_msg_enumerator,
			.create_error_enumerator = _create_error_enumerator,
			.destroy = _destroy,
		},
		.messages = linked_list_create(),
		.errors = linked_list_create(),
		.encoding = chunk_clone(data),
	);

	return &this->public;
}

