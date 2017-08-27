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
 * @defgroup pts_file_meas pts_file_meas
 * @{ @ingroup pts
 */

#ifndef PTS_FILE_MEAS_H_
#define PTS_FILE_MEAS_H_

#include "pts/pts_database.h"

#include <library.h>

typedef struct pts_file_meas_t pts_file_meas_t;

/**
 * Class storing PTS File Measurements
 */
struct pts_file_meas_t {

	/**
	 * Get the ID of the PTS File Measurement Request
	 *
	 * @return				ID of PTS File Measurement Request
	 */
	u_int16_t (*get_request_id)(pts_file_meas_t *this);

	/**
	 * Get the number of measured files
	 *
	 * @return				Number of measured files
	 */
	int (*get_file_count)(pts_file_meas_t *this);

	/**
	 * Add a PTS File Measurement
	 *
	 * @param filename		Name of measured file or directory
	 * @param measurement	PTS Measurement hash
	 */
	void (*add)(pts_file_meas_t *this, char *filename, chunk_t measurement);

	/**
	  * Create a PTS File Measurement enumerator
	  *
	  * @return				Enumerator returning filename and measurement
	  */
	enumerator_t* (*create_enumerator)(pts_file_meas_t *this);

	/**
	 * Check PTS File Measurements against reference value in the database
	 *
	 * @param db			PTS Measurement database
	 * @param pid			Primary key of software product in database
	 * @param algo			PTS Measurement algorithm used
	 * @return				TRUE if all measurements agreed
	 */
	bool (*check)(pts_file_meas_t *this, pts_database_t *db, int pid,
				  pts_meas_algorithms_t algo);

	/**
	 * Verify stored hashes against PTS File Measurements
	 *
	 * @param e_hash		Hash enumerator
	 * @param is_dir		TRUE for directory contents hashes
	 * @return				TRUE if all hashes match a measurement
	 */
	bool (*verify)(pts_file_meas_t *this, enumerator_t *e_hash, bool is_dir);

	/**
	 * Destroys a pts_file_meas_t object.
	 */
	void (*destroy)(pts_file_meas_t *this);

};

/**
 * Creates a pts_file_meas_t object
 *
 * @param request_id		ID of PTS File Measurement Request
 */
pts_file_meas_t* pts_file_meas_create(u_int16_t request_id);

/**
 * Creates a pts_file_meas_t object measuring a file/directory
 *
 * @param request_id		ID of PTS File Measurement Request
 * @param pathname			Absolute file or directory pathname
 * @param is_dir			TRUE if directory path
 * @param use_rel_name		TRUE if relative filenames are to be used
 * @param alg				PTS hash measurement algorithm to be used
 */
pts_file_meas_t* pts_file_meas_create_from_path(u_int16_t request_id,
							char* pathname, bool is_dir, bool use_rel_name,
							pts_meas_algorithms_t alg);

#endif /** PTS_FILE_MEAS_H_ @}*/
