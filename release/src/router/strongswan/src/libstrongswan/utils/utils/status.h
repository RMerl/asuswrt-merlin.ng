/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup status_i status
 * @{ @ingroup utils_i
 */

#ifndef STATUS_H_
#define STATUS_H_

typedef enum status_t status_t;

/**
 * Return values of function calls.
 */
enum status_t {
	/** Call succeeded */
	SUCCESS,
	/** Call failed */
	FAILED,
	/** Out of resources */
	OUT_OF_RES,
	/** The suggested operation is already done */
	ALREADY_DONE,
	/** Not supported */
	NOT_SUPPORTED,
	/** One of the arguments is invalid */
	INVALID_ARG,
	/** Something could not be found */
	NOT_FOUND,
	/** Error while parsing */
	PARSE_ERROR,
	/** Error while verifying */
	VERIFY_ERROR,
	/** Object in invalid state */
	INVALID_STATE,
	/** Destroy object which called method belongs to */
	DESTROY_ME,
	/** Another call to the method is required */
	NEED_MORE,
};

/**
 * enum_names for type status_t.
 */
extern enum_name_t *status_names;

/**
 * returns FAILED
 */
status_t return_failed();

/**
 * returns SUCCESS
 */
status_t return_success();

#endif /** STATUS_H_ @} */
