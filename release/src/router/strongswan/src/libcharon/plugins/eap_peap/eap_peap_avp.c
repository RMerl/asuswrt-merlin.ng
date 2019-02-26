/*
 * Copyright (C) 2011 Andreas Steffen
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

#include "eap_peap_avp.h"

#include <eap/eap.h>
#include <utils/debug.h>

/**
 * Microsoft Success and Failure Result AVPs
 */
static const chunk_t MS_AVP_Success = chunk_from_chars(
											0x80, 0x03, 0x00, 0x02, 0x00, 0x01);
static const chunk_t MS_AVP_Failure = chunk_from_chars(
											0x80, 0x03, 0x00, 0x02, 0x00, 0x02);

typedef struct private_eap_peap_avp_t private_eap_peap_avp_t;

/**
 * Private data of an eap_peap_avp_t object.
 */
struct private_eap_peap_avp_t {

	/**
	 * Public eap_peap_avp_t interface.
	 */
	eap_peap_avp_t public;

	/**
	 * EAP server or peer
	 */
	bool is_server;
};

METHOD(eap_peap_avp_t, build, void,
	private_eap_peap_avp_t *this, bio_writer_t *writer, chunk_t data)
{
	uint8_t code;
	eap_packet_t *pkt;
	chunk_t avp_data;

	pkt = (eap_packet_t*)data.ptr;

	if (pkt->code == EAP_SUCCESS || pkt->code == EAP_FAILURE)
	{
		code = (this->is_server) ? EAP_REQUEST : EAP_RESPONSE;
		writer->write_uint8(writer, code);
		writer->write_uint8(writer, pkt->identifier);
		writer->write_uint16(writer, 11);
		writer->write_uint8(writer, EAP_MSTLV);
		avp_data = (pkt->code == EAP_SUCCESS) ? MS_AVP_Success : MS_AVP_Failure;
	}
	else
	{
		avp_data = chunk_skip(data, 4);
	}
	writer->write_data(writer, avp_data);
}

METHOD(eap_peap_avp_t, process, status_t,
	private_eap_peap_avp_t* this, bio_reader_t *reader, chunk_t *data,
	uint8_t identifier)
{
	uint8_t code;
	uint16_t len;
	eap_packet_t *pkt;
	chunk_t avp_data;

	code = (this->is_server) ? EAP_RESPONSE : EAP_REQUEST;
	len = reader->remaining(reader);
	if (!reader->read_data(reader, len, &avp_data))
	{
		return FAILED;
	}
	pkt = (eap_packet_t*)avp_data.ptr;

	if (len > 4 && pkt->code == code && untoh16(&pkt->length) == len)
	{
		if (len == 5 && pkt->type == EAP_IDENTITY)
		{
			DBG2(DBG_IKE, "uncompressed EAP Identity request");
			*data = chunk_clone(avp_data);
			return SUCCESS;
		}
		else if (len == 11 && pkt->type == EAP_MSTLV)
		{
			if (memeq(&pkt->data, MS_AVP_Success.ptr, MS_AVP_Success.len))
			{
				DBG2(DBG_IKE, "MS Success Result AVP");
				code = EAP_SUCCESS;
			}
			else if (memeq(&pkt->data, MS_AVP_Failure.ptr, MS_AVP_Failure.len))
			{
				DBG2(DBG_IKE, "MS Failure Result AVP");
				code = EAP_FAILURE;
			}
			else
			{
				DBG1(DBG_IKE, "unknown MS AVP message");
				return FAILED;
			}
			identifier = pkt->identifier;
			len = 0;
		}
	}

	*data = chunk_alloc(4 + len);
	pkt = (eap_packet_t*)data->ptr;
	pkt->code = code;
	pkt->identifier = identifier;
	htoun16(&pkt->length, data->len);
	memcpy(data->ptr + 4, avp_data.ptr, len);

	return SUCCESS;
}

METHOD(eap_peap_avp_t, destroy, void,
	private_eap_peap_avp_t *this)
{
	free(this);
}

/**
 * See header
 */
eap_peap_avp_t *eap_peap_avp_create(bool is_server)
{
	private_eap_peap_avp_t *this;

	INIT(this,
		.public= {
			.process = _process,
			.build = _build,
			.destroy = _destroy,
		},
		.is_server = is_server,
	);

	return &this->public;
}
