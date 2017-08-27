/*
 * Copyright (C) 2011 Sansar Choinyambuu
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

#include "swid_error.h"

#include <bio/bio_writer.h>
#include <ietf/ietf_attr_pa_tnc_error.h>

ENUM(swid_error_code_names, TCG_SWID_ERROR, TCG_SWID_RESPONSE_TOO_LARGE,
	"SWID Error",
	"SWID Subscription Denied",
	"SWID Response Too Large"
);

/**
 * Described in header.
 */
pa_tnc_attr_t* swid_error_create(swid_error_code_t code, u_int32_t request_id,
								 u_int32_t max_attr_size, char *description)
{
	bio_writer_t *writer;
	chunk_t msg_info;
	pa_tnc_attr_t *attr;
	pen_type_t error_code;

	error_code = pen_type_create( PEN_TCG, code);
	writer = bio_writer_create(4);
	writer->write_uint32(writer, request_id);
	if (code == TCG_SWID_RESPONSE_TOO_LARGE)
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

