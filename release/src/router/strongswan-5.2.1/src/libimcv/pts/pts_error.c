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

#include "pts_error.h"

#include <bio/bio_writer.h>
#include <ietf/ietf_attr_pa_tnc_error.h>

ENUM(pts_error_code_names, TCG_PTS_RESERVED_ERROR, TCG_PTS_UNABLE_DET_PCR,
	"Reserved Error",
	"Hash Algorithm Not Supported",
	"Invalid Path",
	"File Not Found",
	"Registry Not Supported",
	"Registry Key Not Found",
	"D-H Group Not Supported",
	"DH-PN Nonce Not Acceptable",
	"Invalid Functional Name Family",
	"TPM Version Information Unavailable",
	"Invalid File Pathname Delimiter",
	"PTS Operation Not Supported",
	"Unable To Update Reference Manifest",
	"Unable To Perform Local Validation",
	"Unable To Collect Current Evidence",
	"Unable To Determine Transitive Trust Chain",
	"Unable To Determine PCR"
);

/**
 * Described in header.
 */
pa_tnc_attr_t* pts_hash_alg_error_create(pts_meas_algorithms_t algorithms)
{
	bio_writer_t *writer;
	chunk_t msg_info;
	pa_tnc_attr_t *attr;
	pen_type_t error_code = { PEN_TCG, TCG_PTS_HASH_ALG_NOT_SUPPORTED };

	writer = bio_writer_create(4);
	writer->write_uint16(writer, 0x0000);
	writer->write_uint16(writer, algorithms);
	msg_info = writer->get_buf(writer);
	attr = ietf_attr_pa_tnc_error_create(error_code, msg_info);
	writer->destroy(writer);

	return attr;
}

/**
 * Described in header.
 */
pa_tnc_attr_t* pts_dh_group_error_create(pts_dh_group_t dh_groups)
{
	bio_writer_t *writer;
	chunk_t msg_info;
	pa_tnc_attr_t *attr;
	pen_type_t error_code = { PEN_TCG, TCG_PTS_DH_GRPS_NOT_SUPPORTED };

	writer = bio_writer_create(4);
	writer->write_uint16(writer, 0x0000);
	writer->write_uint16(writer, dh_groups);
	msg_info = writer->get_buf(writer);
	attr = ietf_attr_pa_tnc_error_create(error_code, msg_info);
	writer->destroy(writer);

	return attr;
}

/**
 * Described in header.
 */
pa_tnc_attr_t* pts_dh_nonce_error_create(int min_nonce_len, int max_nonce_len)
{
	bio_writer_t *writer;
	chunk_t msg_info;
	pa_tnc_attr_t *attr;
	pen_type_t error_code = { PEN_TCG, TCG_PTS_BAD_NONCE_LENGTH };

	writer = bio_writer_create(4);
	writer->write_uint16(writer, min_nonce_len);
	writer->write_uint16(writer, max_nonce_len);
	msg_info = writer->get_buf(writer);
	attr = ietf_attr_pa_tnc_error_create(error_code, msg_info);
	writer->destroy(writer);

	return attr;
}
