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

#include "pb_pdp_referral_msg.h"

#include <bio/bio_writer.h>
#include <bio/bio_reader.h>
#include <utils/debug.h>

ENUM(pb_tnc_pdp_identifier_type_names, PB_PDP_ID_FQDN, PB_PDP_ID_IPV6,
	"PDP FQDN ID",
	"PDP IPv4 ID",
	"PDP IPv6 ID"
);

typedef struct private_pb_pdp_referral_msg_t private_pb_pdp_referral_msg_t;

/**
 *   PB-PDP-Referral message (see section 3.1.1.1 of
 *   TCG TNC PDP Discovery and Validation Specification 1.0
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |   Reserved    |           PDP Identifier Vendor ID            |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                      PDP Identifier Type                      |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                 PDP Identifier (Variable Length)              |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *   Section 3.1.1.2.1 FQDN Identifier
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |   Reserved    |   Protocol    |        Port Number            |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                     FQDN (Variable Length)                    |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *   Section 3.1.1.2.2 IPv4 Identifier
 *
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |   Reserved    |   Protocol    |        Port Number            |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                          IPv4 Address                         |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *   Section 3.1.1.2.3 IPv6 Identifier
 * 
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |   Reserved    |   Protocol    |        Port Number            |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                    IPv6 Address (octets 1-4)                  |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                    IPv6 Address (octets 5-8)                  |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                    IPv6 Address (octets 9-12)                 |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                    IPv6 Address (octets 13-16)                |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

# define PDP_REFERRAL_HEADER_SIZE		8
# define PDP_REFERRAL_ID_HEADER_SIZE	4
# define PDP_REFERRAL_RESERVED			0x00
# define PDP_REFERRAL_PROTOCOL			0x00

/**
 * Private data of a pb_pdp_referral_msg_t object.
 *
 */
struct private_pb_pdp_referral_msg_t {
	/**
	 * Public pb_pdp_referral_msg_t interface.
	 */
	pb_pdp_referral_msg_t public;

	/**
	 * PB-TNC message type
	 */
	pen_type_t type;

	/**
	 * PDP Identifier Type
	 */
	pen_type_t identifier_type;

	/**
	 * PDP Identifier Value
	 */
	chunk_t identifier;

	/**
	 * PDP FQDN Identifier
	 */
	chunk_t fqdn;

	/**
	 * PT protocol the PDP is using
	 */
	uint8_t protocol;

	/**
	 * PT port the PDP is using
	 */
	uint16_t port;

	/**
	 * Encoded message
	 */
	chunk_t encoding;
};

METHOD(pb_tnc_msg_t, get_type, pen_type_t,
	private_pb_pdp_referral_msg_t *this)
{
	return this->type;
}

METHOD(pb_tnc_msg_t, get_encoding, chunk_t,
	private_pb_pdp_referral_msg_t *this)
{
	return this->encoding;
}

METHOD(pb_tnc_msg_t, build, void,
	private_pb_pdp_referral_msg_t *this)
{
	bio_writer_t *writer;

	if (this->encoding.ptr)
	{
		return;
	}
	writer = bio_writer_create(PDP_REFERRAL_HEADER_SIZE + this->identifier.len);
	writer->write_uint8 (writer, PDP_REFERRAL_RESERVED);
	writer->write_uint24(writer, this->identifier_type.vendor_id);
	writer->write_uint32(writer, this->identifier_type.type);
	writer->write_data  (writer, this->identifier);

	this->encoding = writer->get_buf(writer);
	this->encoding = chunk_clone(this->encoding);
	writer->destroy(writer);
}

METHOD(pb_tnc_msg_t, process, status_t,
	private_pb_pdp_referral_msg_t *this, uint32_t *offset)
{
	bio_reader_t *reader;
	uint8_t reserved;

	*offset = 0;

	/* process message */
	reader = bio_reader_create(this->encoding);
	reader->read_uint8 (reader, &reserved);
	reader->read_uint24(reader, &this->identifier_type.vendor_id);
	reader->read_uint32(reader, &this->identifier_type.type);
	reader->read_data  (reader, reader->remaining(reader), &this->identifier);

	this->identifier = chunk_clone(this->identifier);
	reader->destroy(reader);

	if (this->identifier_type.vendor_id == PEN_TCG &&
		this->identifier_type.type == PB_PDP_ID_FQDN)
	{
		reader = bio_reader_create(this->identifier);
		*offset += PDP_REFERRAL_HEADER_SIZE;

		if (this->identifier.len <= PDP_REFERRAL_ID_HEADER_SIZE)
		{
			reader->destroy(reader);
			return FAILED;
		}
		reader->read_uint8 (reader, &reserved);
		reader->read_uint8 (reader, &this->protocol);
		reader->read_uint16(reader, &this->port);
		reader->read_data  (reader, reader->remaining(reader), &this->fqdn);
		this->fqdn = chunk_clone(this->fqdn);
		reader->destroy(reader);
	}
	return SUCCESS;
}

METHOD(pb_tnc_msg_t, destroy, void,
	private_pb_pdp_referral_msg_t *this)
{
	free(this->encoding.ptr);
	free(this->identifier.ptr);
	free(this->fqdn.ptr);
	free(this);
}

METHOD(pb_pdp_referral_msg_t, get_identifier_type, pen_type_t,
	private_pb_pdp_referral_msg_t *this)
{
	return this->identifier_type;
}

METHOD(pb_pdp_referral_msg_t, get_identifier, chunk_t,
	private_pb_pdp_referral_msg_t *this)
{
	return this->identifier;
}

METHOD(pb_pdp_referral_msg_t, get_fqdn, chunk_t,
	private_pb_pdp_referral_msg_t *this, uint8_t *protocol, uint16_t *port)
{
	if (protocol)
	{
		*protocol = this->protocol;
	}
	if (port)
	{
		*port = this->port;
	}
	return this->fqdn;
}

/**
 * See header
 */
pb_tnc_msg_t* pb_pdp_referral_msg_create(pen_type_t identifier_type,
										 chunk_t identifier)
{
	private_pb_pdp_referral_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_identifier_type = _get_identifier_type,
			.get_identifier = _get_identifier,
		},
		.type = { PEN_TCG, PB_TCG_MSG_PDP_REFERRAL },
		.identifier_type = identifier_type,
		.identifier = chunk_clone(identifier),
	);

	return &this->public.pb_interface;
}

/**
 * See header
 */
pb_tnc_msg_t* pb_pdp_referral_msg_create_from_fqdn(chunk_t fqdn, uint16_t port)
{
	pb_tnc_msg_t *msg;
	bio_writer_t *writer;
	pen_type_t type = { PEN_TCG, PB_PDP_ID_FQDN };

	writer = bio_writer_create(PDP_REFERRAL_ID_HEADER_SIZE + fqdn.len);
	writer->write_uint8 (writer, PDP_REFERRAL_RESERVED);
	writer->write_uint8 (writer, PDP_REFERRAL_PROTOCOL);
	writer->write_uint16(writer, port);
	writer->write_data  (writer, fqdn);

	msg = pb_pdp_referral_msg_create(type, writer->get_buf(writer));
	writer->destroy(writer);

	return msg;
}

/**
 * See header
 */
pb_tnc_msg_t *pb_pdp_referral_msg_create_from_data(chunk_t data)
{
	private_pb_pdp_referral_msg_t *this;

	INIT(this,
		.public = {
			.pb_interface = {
				.get_type = _get_type,
				.get_encoding = _get_encoding,
				.build = _build,
				.process = _process,
				.destroy = _destroy,
			},
			.get_identifier_type = _get_identifier_type,
			.get_identifier = _get_identifier,
			.get_fqdn = _get_fqdn,
		},
		.type = { PEN_TCG, PB_TCG_MSG_PDP_REFERRAL },
		.encoding = chunk_clone(data),
	);

	return &this->public.pb_interface;
}

