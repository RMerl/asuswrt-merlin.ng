/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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

#include "pt_tls.h"

#include <utils/debug.h>
#include <pen/pen.h>

/**
 * Described in header.
 */
void libpttls_init(void)
{
	/* empty */
}

/*
 * PT-TNC Message format:
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |    Reserved   |           Message Type Vendor ID              |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          Message Type                         |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                         Message Length                        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                       Message Identifier                      |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                Message Value (e.g. PB-TNC Batch) . . .        |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

ENUM(pt_tls_message_type_names, PT_TLS_EXPERIMENTAL, PT_TLS_ERROR,
	"Experimental",
	"Version Request",
	"Version Response",
	"SASL Mechanisms",
	"SASL Mechanism Selection",
	"SASL Authentication Data",
	"SASL Result",
	"PB-TNC Batch",
	"PT-TLS Error"
);

ENUM(pt_tls_sasl_result_names, PT_TLS_SASL_RESULT_SUCCESS,
							   PT_TLS_SASL_RESULT_MECH_FAILURE,
	"Success",
	"Failure",
	"Abort",
	"Mechanism Failure"
);

/**
 * Read a chunk of data from TLS, returning a reader for it
 */
static bio_reader_t* read_tls(tls_socket_t *tls, size_t len)
{
	ssize_t got, total = 0;
	char *buf;

	buf = malloc(len);
	while (total < len)
	{
		got = tls->read(tls, buf + total, len - total, TRUE);
		if (got <= 0)
		{
			free(buf);
			return NULL;
		}
		total += got;
	}
	return bio_reader_create_own(chunk_create(buf, len));
}

/**
 * Read a PT-TLS message, return header data
 */
bio_reader_t* pt_tls_read(tls_socket_t *tls, uint32_t *vendor,
						  uint32_t *type, uint32_t *identifier)
{
	bio_reader_t *reader;
	uint32_t len;
	uint8_t reserved;

	reader = read_tls(tls, PT_TLS_HEADER_LEN);
	if (!reader)
	{
		return NULL;
	}
	if (!reader->read_uint8(reader, &reserved) ||
		!reader->read_uint24(reader, vendor) ||
		!reader->read_uint32(reader, type) ||
		!reader->read_uint32(reader, &len) ||
		!reader->read_uint32(reader, identifier))
	{
		reader->destroy(reader);
		return NULL;
	}
	reader->destroy(reader);

	if (len < PT_TLS_HEADER_LEN)
	{
		DBG1(DBG_TNC, "received short PT-TLS header (%d bytes)", len);
		return NULL;
	}

	if (*vendor == PEN_IETF)
	{
		DBG2(DBG_TNC, "received PT-TLS message #%d of type '%N' (%d bytes)",
					  *identifier, pt_tls_message_type_names, *type, len);
	}
	else
	{
		DBG2(DBG_TNC, "received PT-TLS message #%d of unknown type "
					  "0x%06x/0x%08x (%d bytes)",
					  *identifier, *vendor, *type, len);
	}

	return read_tls(tls, len - PT_TLS_HEADER_LEN);
}

/**
 * Prepend a PT-TLS header to a writer, send data, destroy writer
 */
bool pt_tls_write(tls_socket_t *tls, pt_tls_message_type_t type,
				  uint32_t identifier, chunk_t data)
{
	bio_writer_t *writer;
	chunk_t out;
	ssize_t len;

	len = PT_TLS_HEADER_LEN + data.len;
	writer = bio_writer_create(len);

	/* write PT-TLS header */
	writer->write_uint8 (writer, 0);
	writer->write_uint24(writer, 0);
	writer->write_uint32(writer, type);
	writer->write_uint32(writer, len);
	writer->write_uint32(writer, identifier);

	/* write PT-TLS body */
	writer->write_data(writer, data);

	DBG2(DBG_TNC, "sending PT-TLS message #%d of type '%N' (%d bytes)",
				   identifier, pt_tls_message_type_names, type, len);

	out = writer->get_buf(writer);
	len = tls->write(tls, out.ptr, out.len);
	writer->destroy(writer);

	return len == out.len;
}
