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
 * @defgroup tcg_pts_attr_req_file_meta tcg_pts_attr_req_file_meta
 * @{ @ingroup tcg_attr
 */

#ifndef TCG_PTS_ATTR_REQ_FILE_META_H_
#define TCG_PTS_ATTR_REQ_FILE_META_H_

typedef struct tcg_pts_attr_req_file_meta_t tcg_pts_attr_req_file_meta_t;

#include "tcg/tcg_attr.h"
#include "pa_tnc/pa_tnc_attr.h"

/**
 * Class implementing the TCG PTS Request File Metadata attribute
 *
 */
struct tcg_pts_attr_req_file_meta_t {

	/**
	 * Public PA-TNC attribute interface
	 */
	pa_tnc_attr_t pa_tnc_attribute;

	/**
	 * Get directory flag for PTS Request File Metadata
	 *
	 * @return				Directory Contents flag
	 */
	bool (*get_directory_flag)(tcg_pts_attr_req_file_meta_t *this);

	/**
	 * Get Delimiter
	 *
	 * @return				UTF-8 encoding of a Delimiter Character
	 */
	u_int8_t (*get_delimiter)(tcg_pts_attr_req_file_meta_t *this);

	/**
	 * Get Fully Qualified File Pathname
	 *
	 * @return				Pathname
	 */
	char* (*get_pathname)(tcg_pts_attr_req_file_meta_t *this);

};

/**
 * Creates an tcg_pts_attr_req_file_meta_t object
 *
 * @param directory_flag	Directory Contents Flag
 * @param delimiter			Delimiter Character
 * @param pathname			File Pathname
 */
pa_tnc_attr_t* tcg_pts_attr_req_file_meta_create(bool directory_flag,
												 u_int8_t delimiter,
												 char *pathname);

/**
 * Creates an tcg_pts_attr_req_file_meta_t object from received data
 *
 * @param length			Total length of attribute value
 * @param value				Unparsed attribute value (might be a segment)
 */
pa_tnc_attr_t* tcg_pts_attr_req_file_meta_create_from_data(size_t length,
														   chunk_t value);

#endif /** TCG_PTS_ATTR_REQ_FILE_META_H_ @}*/
