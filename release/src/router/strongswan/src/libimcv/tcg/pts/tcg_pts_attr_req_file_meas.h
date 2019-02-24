/*
 * Copyright (C) 2011 Sansar Choinyambuu
 * Copyright (C) 2014 Andreas Steffen
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

/**
 * @defgroup tcg_pts_attr_req_file_meas tcg_pts_attr_req_file_meas
 * @{ @ingroup tcg_attr
 */

#ifndef TCG_PTS_ATTR_REQ_FILE_MEAS_H_
#define TCG_PTS_ATTR_REQ_FILE_MEAS_H_

typedef struct tcg_pts_attr_req_file_meas_t tcg_pts_attr_req_file_meas_t;

#include "tcg/tcg_attr.h"
#include "pa_tnc/pa_tnc_attr.h"

/**
 * Class implementing the TCG PTS Request File Measurement attribute
 *
 */
struct tcg_pts_attr_req_file_meas_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get flag for PTS Request File Measurement
	 *
	 * @return				Directory Contents flag
	 */
	bool (*get_directory_flag)(tcg_pts_attr_req_file_meas_t *this);

	/**
	 * Get Request ID
	 *
	 * @return				Request ID
	 */
	uint16_t (*get_request_id)(tcg_pts_attr_req_file_meas_t *this);

	/**
	 * Get Delimiter
	 *
	 * @return				UTF-8 encoding of a Delimiter Character
	 */
	uint32_t (*get_delimiter)(tcg_pts_attr_req_file_meas_t *this);

	/**
	 * Get Fully Qualified File Pathname
	 *
	 * @return				Pathname
	 */
	char* (*get_pathname)(tcg_pts_attr_req_file_meas_t *this);

};

/**
 * Creates an tcg_pts_attr_req_file_meas_t object
 *
 * @param directory_flag	Directory Contents Flag
 * @param request_id		Request ID
 * @param delimiter			Delimiter Character
 * @param pathname			File Pathname
 */
pa_tnc_attr_t* tcg_pts_attr_req_file_meas_create(bool directory_flag,
												 uint16_t request_id,
												 uint32_t delimiter,
												 char *pathname);

/**
 * Creates an tcg_pts_attr_req_file_meas_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* tcg_pts_attr_req_file_meas_create_from_data(size_t length,
														   chunk_t value);

#endif /** TCG_PTS_ATTR_REQ_FILE_MEAS_H_ @}*/
