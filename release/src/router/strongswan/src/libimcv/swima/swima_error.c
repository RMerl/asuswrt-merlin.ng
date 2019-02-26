/*
 * Copyright (C) 2017 Andreas Steffen
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

#include "swima_error.h"

#include <bio/bio_writer.h>
#include <ietf/ietf_attr_pa_tnc_error.h>

/**
 * SW_ERROR, SW_SUBSCRIPTION_DENIED_ERROR and SW_SUBSCRIPTION_ID_REUSE_ERROR
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |            Copy of Request ID / Subscription ID               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                Description (variable length)                  |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/**
 * SW_RESPONSE_TOO_LARGE_ERROR 
 *
 *                       1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |            Copy of Request ID / Subscription ID               |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                    Maximum Allowed Size                       |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                Description (variable length)                  |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

/**
 * Described in header.
 */
pa_tnc_attr_t* swima_error_create(pa_tnc_error_code_t code, uint32_t request_id,
								  uint32_t max_attr_size, char *description)
{
	bio_writer_t *writer;
	chunk_t msg_info;
	pa_tnc_attr_t *attr;
	pen_type_t error_code;

	error_code = pen_type_create( PEN_IETF, code);
	writer = bio_writer_create(4);
	writer->write_uint32(writer, request_id);

	if (code == PA_ERROR_SWIMA_RESPONSE_TOO_LARGE)
	{
		writer->write_uint32(writer, max_attr_size);
	}

	if (description)
	{
		writer->write_data(writer, chunk_from_str(description));
	}
	msg_info = writer->get_buf(writer);
	attr = ietf_attr_pa_tnc_error_create(error_code, msg_info);
	writer->destroy(writer);

	return attr;
}

