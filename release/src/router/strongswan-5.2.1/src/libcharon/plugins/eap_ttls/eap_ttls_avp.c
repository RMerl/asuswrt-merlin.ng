/*
 * Copyright (C) 2010 Andreas Steffen
 * Copyright (C) 2010 HSR Hochschule fuer Technik Rapperswil
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

#include "eap_ttls_avp.h"

#include <utils/debug.h>

#define AVP_EAP_MESSAGE		79
#define AVP_HEADER_LEN		 8

typedef struct private_eap_ttls_avp_t private_eap_ttls_avp_t;

/**
 * Private data of an eap_ttls_avp_t object.
 */
struct private_eap_ttls_avp_t {

	/**
	 * Public eap_ttls_avp_t interface.
	 */
	eap_ttls_avp_t public;

	/**
	 * AVP input buffer
	 */
	chunk_t input;

	/**
	 * Position in input buffer
	 */
	size_t inpos;

	/**
	 * process header (TRUE) or body (FALSE)
	 */
	bool process_header;

	/**
	 * Size of AVP data
	 */
	size_t data_len;
};

METHOD(eap_ttls_avp_t, build, void,
	private_eap_ttls_avp_t *this, bio_writer_t *writer, chunk_t data)
{
	char zero_padding[] = { 0x00, 0x00, 0x00 };
	chunk_t   avp_padding;
	u_int8_t  avp_flags;
	u_int32_t avp_len;

	avp_flags = 0x40;
	avp_len = 8 + data.len;
	avp_padding = chunk_create(zero_padding, (4 - data.len) % 4);

	writer->write_uint32(writer, AVP_EAP_MESSAGE);
	writer->write_uint8(writer, avp_flags);
	writer->write_uint24(writer, avp_len);
	writer->write_data(writer, data);
	writer->write_data(writer, avp_padding);
}

METHOD(eap_ttls_avp_t, process, status_t,
	private_eap_ttls_avp_t* this, bio_reader_t *reader, chunk_t *data)
{
	size_t len;
	chunk_t buf;

	if (this->process_header)
	{
		bio_reader_t *header;
		u_int32_t avp_code;
		u_int8_t  avp_flags;
		u_int32_t avp_len;
		bool success;

		len = min(reader->remaining(reader), AVP_HEADER_LEN - this->inpos);
		if (!reader->read_data(reader, len, &buf))
		{
			return FAILED;
		}
		if (this->input.len == 0)
		{
			/* start of a new AVP header */
			this->input = chunk_alloc(AVP_HEADER_LEN);
			memcpy(this->input.ptr, buf.ptr, len);
			this->inpos = len;
		}
		else
		{
			memcpy(this->input.ptr + this->inpos, buf.ptr, len);
			this->inpos += len;
		}

		if (this->inpos < AVP_HEADER_LEN)
		{
			return NEED_MORE;
		}

		/* parse AVP header */
		header = bio_reader_create(this->input);
		success = header->read_uint32(header, &avp_code) &&
				  header->read_uint8(header, &avp_flags) &&
				  header->read_uint24(header, &avp_len);
		header->destroy(header);
		chunk_free(&this->input);
		this->inpos = 0;

		if (!success)
		{
			DBG1(DBG_IKE, "received invalid AVP header");
			return FAILED;
		}
 		if (avp_code != AVP_EAP_MESSAGE)
		{
			DBG1(DBG_IKE, "expected AVP_EAP_MESSAGE but received %u", avp_code);
			return FAILED;
		}
		this->process_header = FALSE;
		this->data_len = avp_len - 8;
		this->input = chunk_alloc(this->data_len + (4 - avp_len) % 4);
	}

	/* process AVP data */
	len = min(reader->remaining(reader), this->input.len - this->inpos);
	if (!reader->read_data(reader, len, &buf))
	{
		return FAILED;
	}
	memcpy(this->input.ptr + this->inpos, buf.ptr, len);
	this->inpos += len;
	if (this->inpos < this->input.len)
	{
		return NEED_MORE;
	}

	*data = this->input;
	data->len = this->data_len;

	/* preparing for next AVP */
	this->input = chunk_empty;
	this->inpos = 0;
	this->process_header = TRUE;

	return SUCCESS;
}

METHOD(eap_ttls_avp_t, destroy, void,
	private_eap_ttls_avp_t *this)
{
	chunk_free(&this->input);
	free(this);
}

/**
 * See header
 */
eap_ttls_avp_t *eap_ttls_avp_create(void)
{
	private_eap_ttls_avp_t *this;

	INIT(this,
		.public= {
			.process = _process,
			.build = _build,
			.destroy = _destroy,
		},
		.input = chunk_empty,
		.inpos = 0,
		.process_header = TRUE,
		.data_len = 0,
	);

	return &this->public;
}
