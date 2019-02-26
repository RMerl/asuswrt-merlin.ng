/*
 * Copyright (C) 2011-2014 Andreas Steffen
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
 * @defgroup attest_db_t attest_db
 * @{ @ingroup libimcv
 */

#ifndef ATTEST_DB_H_
#define ATTEST_DB_H_

#include <pts/pts_meas_algo.h>
#include <os_info/os_info.h>
#include <library.h>

typedef struct attest_db_t attest_db_t;

/**
 * Attestation database object
 */
struct attest_db_t {

	/**
	 * Set functional component to be queried
	 *
	 * @param comp			functional component
	 * @param create		if TRUE create database entry if it doesn't exist
	 * @return				TRUE if successful
	 */
	bool (*set_component)(attest_db_t *this, char *comp, bool create);

	/**
	 * Set primary key of the functional component to be queried
	 *
	 * @param fid			primary key of functional component
	 * @return				TRUE if successful
	 */
	bool (*set_cid)(attest_db_t *this, int fid);

	/**
	 * Set directory to be queried
	 *
	 * @param dir			directory
	 * @param create		if TRUE create database entry if it doesn't exist
	 * @return				TRUE if successful
	 */
	bool (*set_directory)(attest_db_t *this, char *dir, bool create);

	/**
	 * Set primary key of the directory to be queried
	 *
	 * @param did			primary key of directory
	 * @return				TRUE if successful
	 */
	bool (*set_did)(attest_db_t *this, int did);

	/**
	 * Set measurement file to be queried
	 *
	 * @param file			measurement file
	 * @param create		if TRUE create database entry if it doesn't exist
	 * @return				TRUE if successful
	 */
	bool (*set_file)(attest_db_t *this, char *file, bool create);

	/**
	 * Set primary key of the measurement file to be queried
	 *
	 * @param fid			primary key of measurement file
	 * @return				TRUE if successful
	 */
	bool (*set_fid)(attest_db_t *this, int fid);

	/**
	 * Set path to directory where file[s] are to be measured
	 *
	 * @param meas_dir		measurement directory
	 * @return				TRUE if successful
	 */
	bool (*set_meas_directory)(attest_db_t *this, char *dir);

	/**
	 * Set functional component to be queried
	 *
	 * @param key			AIK
	 * @param create		if TRUE create database entry if it doesn't exist
	 * @return				TRUE if successful
	 */
	bool (*set_key)(attest_db_t *this, chunk_t key, bool create);

	/**
	 * Set primary key of the AIK to be queried
	 *
	 * @param kid			primary key of AIK
	 * @return				TRUE if successful
	 */
	bool (*set_kid)(attest_db_t *this, int kid);

	/**
	 * Set software package to be queried
	 *
	 * @param product		software package
	 * @param create		if TRUE create database entry if it doesn't exist
	 * @return				TRUE if successful
	 */
	bool (*set_package)(attest_db_t *this, char *package, bool create);

	/**
	 * Set primary key of the software package to be queried
	 *
	 * @param gid			primary key of software package
	 * @return				TRUE if successful
	 */
	bool (*set_gid)(attest_db_t *this, int gid);

	/**
	 * Set software product to be queried
	 *
	 * @param product		software product
	 * @param create		if TRUE create database entry if it doesn't exist
	 * @return				TRUE if successful
	 */
	bool (*set_product)(attest_db_t *this, char *product, bool create);

	/**
	 * Set primary key of the software product to be queried
	 *
	 * @param pid			primary key of software product
	 * @return				TRUE if successful
	 */
	bool (*set_pid)(attest_db_t *this, int pid);

	/**
	 * Set software package version to be queried
	 *
	 * @param version		software package version
	 * @return				TRUE if successful
	 */
	bool (*set_version)(attest_db_t *this, char *version);

	/**
	 * Set measurement hash algorithm
	 *
	 * @param algo			hash algorithm
	 */
	void (*set_algo)(attest_db_t *this, pts_meas_algorithms_t algo);

	/**
	 * Set that the IMA-specific SHA-1 template hash be computed
	 */
	void (*set_ima)(attest_db_t *this);

	/**
	 * Set that relative filenames are to be used
	 */
	void (*set_relative)(attest_db_t *this);

	/**
	 * Set the package security or blacklist state
	 */
	void (*set_package_state)(attest_db_t *this, os_package_state_t package_state);

	/**
	 * Set the sequence number
	 */
	void (*set_sequence)(attest_db_t *this, int seq_no);

	/**
	 * Set owner [user/host] of an AIK
	 *
	 * @param owner			user/host name
	 * @return				TRUE if successful
	 */
	void (*set_owner)(attest_db_t *this, char *owner);

	/**
	 * Display all dates in UTC
	 */
	void (*set_utc)(attest_db_t *this);

	/**
	 * List all packages stored in the database
	 */
	void (*list_packages)(attest_db_t *this);

	/**
	 * List all products stored in the database
	 */
	void (*list_products)(attest_db_t *this);

	/**
	 * List all directories stored in the database
	 */
	void (*list_directories)(attest_db_t *this);

	/**
	 * List selected files stored in the database
	 */
	void (*list_files)(attest_db_t *this);

	/**
	 * List all components stored in the database
	 */
	void (*list_components)(attest_db_t *this);

	/**
	 * List all devices stored in the database
	 */
	void (*list_devices)(attest_db_t *this);

	/**
	 * List all AIKs stored in the database
	 */
	void (*list_keys)(attest_db_t *this);

	/**
	 * List selected measurement hashes stored in the database
	 */
	void (*list_hashes)(attest_db_t *this);

	/**
	 * List selected component measurement stored in the database
	 */
	void (*list_measurements)(attest_db_t *this);

	/**
	 * List sessions stored in the database
	 */
	void (*list_sessions)(attest_db_t *this);

	/**
	 * Add an entry to the database
	 */
	bool (*add)(attest_db_t *this);

	/**
	 * Delete an entry from the database
	 */
	bool (*delete)(attest_db_t *this);

	/**
	 * Destroy attest_db_t object
	 */
	void (*destroy)(attest_db_t *this);

};

/**
 * Create an attest_db_t instance
 *
 * @param uri				database URI
 */
attest_db_t* attest_db_create(char *uri);

#endif /** ATTEST_DB_H_ @}*/
